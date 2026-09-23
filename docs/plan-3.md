# Plan 3 — Suppression de la couche objet Lua

Document distinct de [`docs/plan.md`](plan.md) (le portage) et de [`docs/plan-2.md`](plan-2.md) (flottant, puis `malloc`). Celui-ci remplace la couche d'émulation Lua — tables dynamiques, classes à chaîne de prototypes, valeurs taguées — par des structs C plates.

Ce document est **le plan** ; aucune implémentation n'est engagée par sa rédaction. Exécution prévue par `plan-pilot`, sans supervision humaine entre les étapes (§7).

---

## 1. Motivation, et la décision qu'elle renverse

`docs/plan.md` a acté : mécanisme d'entités « porté littéralement, pas réécrit en C idiomatique (pas de struct-par-type-d'entité + enum + switch) ». C'était juste pour **porter** — le diff Lua/C était l'outil de vérification. Le port marche maintenant ; ce plan renverse la décision, délibérément, pour quatre raisons qui pèsent ensemble :

1. **Types.** La confusion REF `ob_table`/`instance` a produit trois crashes et un audit de 74 sites (`docs/audit-ref-cast-reverse.md`). Des structs typées suppriment la classe de bug au lieu de la garder.
2. **`malloc`.** Plan 2 §8 constate que les allocations *sont* cette couche. Des formes fixes rendent les pools possibles.
3. **Lisibilité et performance.** Un champ de struct au lieu d'une recherche par chaîne sur chaque accès.
4. **PSX.** Mémoire et coût des recherches par chaîne sur un R3000A à 33 MHz.

La traçabilité par nom minifié vers `pico8/lua.lua` est conservée jusqu'à la phase 3 (§6), puis abandonnée champ par champ là où le glossaire donne un sens.

## 2. Ce qu'est la couche

| Module | Rôle |
|---|---|
| `pico8_value` | valeur taguée (fixe, chaîne, booléen, REF, fonction) |
| `pico8_ob` | table ordonnée clé/valeur + parseur de spec `ob()`/`fi()` |
| `pico8_class` | classes à chaîne `parent`, instances, registres par `fz` et par nom |
| `pico8_entity` | `hv`/`cv`/`cg`, `lt`/`he`, `dc`/`mv`, `ci`, `mo` |
| `pico8_type_tag.h`, `pico8_ref_audit.h` | désambiguïsation des REF |
| `pico8_list` | tableau Lua |

Emprise mesurée au cadrage : ~1 300 sites dans ~40 fichiers (218 `instance_get_field`, 408 `ob_table_get/set_name`, 141 accès indexés, 323 `value_ref`, 90 `value_function`, 31 déclarations de classe, 36 instanciations).

Arbre des classes :

```
ke (me, t, pq→lt)
├─ jo (planète) ─┬─ qh ─┬─ nl
│                │      ├─ os
│                │      ├─ hj ─┬─ pm
│                │      │      └─ mx
│                │      └─ nr ── fr
│                └─ ga
├─ js, fq, nf, qb, mz, mh, iv, jc, ui, ie
racines : trade, lq, jb, hy, pj, g_ (sonde), fv, bg, research
```

Plus des **instances sans classe** construites à la main : `mo`, `je`, le `fm` provisoire, les enveloppes `mz.jj`/`ie.ke`, les enveloppes de lignes de `pz()`.

## 3. Invariant de toutes les phases

**Après chaque commit, `make test` passe et tous les `*.frames.hash` sont identiques octet pour octet.** Aucun re-baseline dans ce plan. Un changement de rendu voulu n'appartient pas à ce plan.

Le filet compare l'écran, pas l'état : une erreur sur une valeur jamais affichée pendant les frames enregistrées lui échappe. C'est accepté (décision de cadrage) ; la couverture de l'étape 0 réduit le risque, elle ne l'annule pas.

## 4. Étape 0 — Prérequis, avant tout code de ce plan

Dans cet ordre, chacun bloquant le suivant :

1. **Code de triche** — PRD [`docs/prd/triage/01-code-triche-boutons-ox.md`](prd/triage/01-code-triche-boutons-ox.md) : boutons O/X, séquence A A B B A A B B → 99 science, 200 crédits.
2. **Outil de couverture des classes** — `make class-coverage DIR=<dossier>` :
   - rejoue **chaque** `*events.json` du dossier ; un fichier qui ne se charge pas est signalé « illisible » et ignoré, sans arrêter l'outil ;
   - affiche une ligne par classe : nom, nombre d'instanciations, nombre de rejeux qui l'instancient ;
   - sort en erreur si une classe est à 0 sans être exemptée ;
   - compte par un crochet dans l'instanciation, compilé seulement dans un binaire dédié (`-DPICO8_CLASS_COVERAGE`) — le binaire de rejeu normal n'est pas touché ;
   - liste d'exemptions versionnée, une raison par ligne. Contenu initial : `ke` (abstraite, jamais instanciée directement). **`pm` et `ga` ne sont pas exemptés** ;
   - **hors de `make test`** : lancé à la main, et par l'autopilote aux points prévus ci-dessous ;
   - le contrat de l'outil est « chaque type d'entité est construit au moins une fois », indépendant de la représentation : en 1a, le crochet migre vers les constructeurs de struct, dans le commit de la classe concernée.
3. **Enregistrement humain** — une session raylib qui construit `qh`, `os`, `nl`, `hj`, `nr`, `fr`, `mx` et sonde une protostar (`pm`, qui engendre `ga`). Déposée par l'humain dans le dossier de fixtures de son choix, avec son `.frames.hash` posé par `make frames-baseline`.
4. **Porte** — `make class-coverage` sur ce dossier : aucune classe à 0 hors exemption. Sinon, l'autopilote s'arrête et demande un nouvel enregistrement.

Mesure de cadrage (12 rejeux du filet, avant l'étape 0) : à 0 → `ke`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` ; rares → `qh` 4, `ga` 1, `fq` 1 ; les 19 autres entre 15 et 1 350.

## 5. Phases

### Phase 1a — Entités en structs, champs `pico8_value_t`

Transformation mécanique. Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`, comme aujourd'hui.

- **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
- **Clés construites à l'exécution** : `self[bt.."held"]` (`pico8_ui.c:328`) → deux champs `lheld`/`rheld` ; `ldown`/`rdown` côté méthodes. Tout autre site de clé dynamique trouvé est énuméré de même, ou déclenche un arrêt (§7) si l'ensemble des clés n'est pas fini.
- **Défauts mutés sur la classe** : `ie.gy`, `ie.id` (`pico8_research_icon.c:158,182`, `pico8_cart.c:158`) deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut écrit après l'initialisation.
- **Instances sans classe** : converties comme des classes à part entière, sans parent.
- **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
- **Ordre** : une classe par commit, la parente avant ses filles (`ke` → `jo` → `qh` → …). Les racines sans parent dans n'importe quel ordre.
- **Sortie** : plus aucune `pico8_class_t`/`pico8_instance_t` à l'exécution ; `pico8_class.c` supprimé ; `make class-coverage` relancé sur le dossier de l'étape 0, toujours sans zéro.

### Phase 1b — Types C réels

Struct par struct, chaque membre `pico8_value_t` prend son type C (`pico8_fixed_t`, `pico8_vec_t`, pointeur de struct typé, `const char *`, booléen…).

- **Nil** : un indicateur de présence **uniquement** là où le code observe nil (test de vérité, `mv` qui rend 0 sur champ absent, affectation `= nil`). Rappel : en Lua `0` est vrai, donc un fixe à 0 n'est pas nil. Pas de valeur sentinelle.
- Les REF deviennent des pointeurs typés : les sites de `pico8_ref_audit.h` disparaissent au fil de l'eau.
- **Sortie** : aucune struct d'entité ne contient de `pico8_value_t`.

### Phase D — Données

- Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
- Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
- **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

### Phase 2 — Composition

La hiérarchie par enchâssement de 1a devient :

- un **noyau** `entity` commun (ce que `ke` apporte : table de méthodes, `me`, `t`, tags `nu` en masque de bits) ;
- des **archétypes** `static const` : les défauts de chaque classe, résolus une fois le long de sa chaîne (`NL_DEF = { .hp=6, .s=68, … }`) ;
- des **composants** optionnels portés par pointeur.

Règles, appliquées sans supervision :

1. **Archétype ou état** : un champ va dans l'archétype si aucun code ne l'écrit sur une instance (prouvé par grep) ; sinon dans l'état mutable de la struct. L'état partagé de classe (`ie.gy`, `ie.id`) reste statique.
2. **Composant** : un groupe de champs devient un composant s'il apparaît dans **deux branches distinctes** de l'arbre (exemple connu : `py`/`fc`/`k_`/`qo`/`ro`/`qg`/`q_`/`du` dans `hj`, `pm`, `mx` et `ga`, alors que `ga` descend de `jo`). Sinon il reste dans la struct du type.
3. Test ponctuel, supprimé en fin de phase : pour chaque ancienne classe, chaque champ résolu par sa chaîne est égal à la valeur de l'archétype correspondant (piège connu : le `nu=o()` vide de `ga` masque celui de `jo`).

### Phase 3 — Renommage

Un champ est renommé **seulement** si `docs/glossaire-noms.md` en donne le sens. Sinon, il garde son nom minifié. Aucun sens n'est inventé. Un commit par groupe de champs renommés ; le glossaire est mis à jour en conséquence (l'entrée pointe du nom minifié vers le nouveau nom).

## 6. Tests existants

- Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
- Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés**. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit.

## 7. Exécution

`plan-pilot`, phase après phase, un commit par classe (1a, 1b) ou par groupe (D, 2, 3).

L'autopilote **s'arrête et demande à l'humain** quand :

- le filet reste rouge après **3 tentatives** de correction sur le même commit ;
- un champ ne tombe sous aucune règle (§5 : clé dynamique à ensemble non fini ; en phase 2, champ ni archétype ni état, ou groupe au classement ambigu) ;
- un test de régression de bug devrait être modifié **dans son intention** (pas seulement dans sa façon de lire l'état) pour passer ;
- la porte de l'étape 0 échoue.

## 8. Hors périmètre

- Le `malloc` des entités (pools, arènes, épuisement) : Plan 2 §8, à cadrer sur la base typée que ce plan produit.
- Toute comparaison de l'état des entités frame à frame.
- Tout changement de rendu ou de comportement.
- L'audio.
