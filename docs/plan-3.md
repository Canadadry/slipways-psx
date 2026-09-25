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

**Après chaque commit, `make test` passe et tous les `*.sha256` du filet sont identiques (`make frames-check` conforme).** Aucun re-baseline dans ce plan. Un changement de rendu voulu n'appartient pas à ce plan.

Le filet compare l'écran, pas l'état : une erreur sur une valeur jamais affichée pendant les frames enregistrées lui échappe. C'est accepté (décision de cadrage) ; la couverture de l'étape 0 réduit le risque, elle ne l'annule pas.

## 4. Étape 0 — Prérequis, avant tout code de ce plan

Dans cet ordre, chacun bloquant le suivant :

1. **Code de triche** — PRD [`docs/prd/done/code-triche-boutons-ox.md`](prd/done/code-triche-boutons-ox.md) : boutons O/X, séquence A A B B A A B B → 99 science, 200 crédits.
2. **Outil de couverture des classes** — `make class-coverage DIR=<dossier>` :
   - rejoue **chaque** `*events.json` du dossier ; un fichier qui ne se charge pas est signalé « illisible » et ignoré, sans arrêter l'outil ;
   - affiche une ligne par classe : nom, nombre d'instanciations, nombre de rejeux qui l'instancient ;
   - sort en erreur si une classe est à 0 sans être exemptée ;
   - compte par un crochet dans l'instanciation, compilé seulement dans un binaire dédié (`-DPICO8_CLASS_COVERAGE`) — le binaire de rejeu normal n'est pas touché ;
   - liste d'exemptions versionnée, une raison par ligne. Contenu initial : `ke` (abstraite, jamais instanciée directement). **`pm` et `ga` ne sont pas exemptés** ;
   - **hors de `make test`** : lancé à la main, et par l'autopilote aux points prévus ci-dessous ;
   - le contrat de l'outil est « chaque type d'entité est construit au moins une fois », indépendant de la représentation : en 1a, le crochet migre vers les constructeurs de struct, dans le commit de la classe concernée.
   - avant le premier commit de 1a (étape 1a.0) : le crochet compte par **nom** passé au constructeur, plus par pointeur de classe ; `KNOWN[]` devient la seule liste des noms, et l'outil sort en erreur si un nom compté n'y est pas, ou si un nom des registres `fz`/nommé n'y est pas. Ce commit ne change aucun compte (sortie identique à la porte 0.4).
3. **Enregistrement humain** — une session raylib qui construit `qh`, `os`, `nl`, `hj`, `nr`, `fr`, `mx` et sonde une protostar (`pm`, qui engendre `ga`). Déposée par l'humain dans le dossier de fixtures de son choix, ajoutée à la liste `FRAMES_EVENTS` du filet, avec son `.sha256` posé par `make frames-baseline`.
4. **Porte** — `make class-coverage` sur ce dossier : aucune classe à 0 hors exemption, sur un dossier qui ne contient que les rejeux du filet (dans `FRAMES_EVENTS`, `.sha256` versionné) — un rejeu hors filet n'en protège aucune, et `tests/fixtures/` en contient. Le nombre de rejeux lus par l'outil doit égaler le nombre de rejeux du filet dans le dossier : sinon c'est « enregistrement non rejoué » (l'enregistreur nomme `events_recorded_<horodatage>.json`, que le motif `*events.json` de l'outil ne prend pas), que l'autopilote corrige lui-même sans redemander de session. Le code de sortie tranche ensuite. Même règle pour la relance en sortie de 1a. Sinon, l'autopilote s'arrête et demande un nouvel enregistrement.

Mesure de cadrage (12 rejeux du filet, avant l'étape 0) : à 0 → `ke`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` ; rares → `qh` 4, `ga` 1, `fq` 1 ; les 19 autres entre 15 et 1 350.

## 5. Phases

### Phase 1a — Entités en structs, champs `pico8_value_t`

Transformation mécanique. Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`, comme aujourd'hui.

- **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
- **Clés construites à l'exécution** : `self[bt.."held"]` (`pico8_ui.c:328`) → deux champs `lheld`/`rheld` ; `ldown`/`rdown` côté méthodes. Tout autre site de clé dynamique trouvé est énuméré de même, ou déclenche un arrêt (§7) si l'ensemble des clés n'est pas fini.
- **Défauts mutés sur la classe** : `ie.gy`, `ie.id` (`pico8_research_icon.c:158,182`, `pico8_cart.c:158`) deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** (connus : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`) ; la liste des étapes les nomme.
- **Instances sans classe** : converties comme des classes à part entière, sans parent.
- **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
- **Ordre** : une classe par commit, la parente avant ses filles (`ke` → `jo` → `qh` → …). Les racines sans parent dans n'importe quel ordre.
- **Premier commit** : une vraie racine sans parent ni fille (`jb`), qui prouve la coexistence. **Deuxième** : `ke` **dans le même commit** que sa fille directe sans descendance de plus petite emprise (mesurée par le PRD de l'étape), pour que la struct de `ke` soit construite et vue par le filet dès son introduction. Ensuite les autres filles directes sans descendance de `ke`, par emprise croissante ; l'arbre `jo` et `g_`→`fv` après elles ; `ui` et `pj` (clés dynamiques, défauts mutés) en fin de liste.
- **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a, qui les supprime toutes avec `pico8_class.c`. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe. Les descendants non convertis trouvent leurs champs hérités en remontant `parent`, qui reste valide.
- **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` (grep `->defaults`, ~100 sites dans 22 fichiers) migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde qu'une copie en lecture pour les descendants non convertis.
- **Sortie** : plus aucune `pico8_class_t`/`pico8_instance_t` à l'exécution ; `pico8_class.c` supprimé ; `make class-coverage-net` toujours sans zéro — vérifié par `grep -rn 'pico8_class_t\|pico8_instance_t\|->defaults\|lookup_named' src/` vide.

#### Étapes de la phase 1a

Chemins relatifs à `src/`. « Tests » = tests de régression (§6) réécrits dans l'étape. L'emprise de 1a.3 à 1a.14 est estimée ; chaque PRD la mesure, et une feuille de plus petite emprise passe devant sans être un écart.

- 1a.0 — crochet de couverture par nom (§4.2) ; comptes de la porte 0.4 versionnés. Tests : `test_class_coverage`.
- 1a.1 — `jb` : racine sans fille (`layout.c:357`), champ `ls`. Tests : aucun.
- 1a.2 — `ke` (`ke.c:19`) **avec** la feuille directe de `ke` de plus petite emprise (mesurée ; candidates : `iv`, `mh`, `fq`, `nf`, `qb`, `mz`, `bg`, `jc`, `ie`, `js`), qui sort alors de la suite de la liste. Tests : `built_planet_panel_nil_row`, `empty_planet_cleared` (`e.me`).
- 1a.3 — ~~`iv` (`msgbox.c:260`). Tests : aucun.~~ Sans objet : `iv` a été mesurée feuille de plus petite emprise et convertie dans 1a.2.
- 1a.4 — `mh` (`msgbox.c:146`), construite avec le défaut muté `pj.ea` (`score_panel_display.c:115`). Tests : aucun.
- 1a.5 — `fq` (`ui_button.c:297`), méthode `ldown`. Tests : `score_panel_close` Découvert à 1a.5 : boucle de survol `gw` (`ui.c`, lecture `fa`, écritures `m_`), `jc:hq` (`slipway.c:107`, `ok.o_.gv`).
- 1a.6 — `nf` (`mouse_reader.c:35`), clé dynamique `je[bt]` (`ui.c:60`) → `l`/`r`. Tests : aucun. Découvert à 1a.6 : `l`/`r` défauts modifiés en place (`bu`, partagés par tous les `nf`) → état statique.
- 1a.7 — `je` : sans classe (`ui.c:108`), provisoire, remplacée par un `nf` (`bootstrap.c:49`).
- 1a.8 — `qb` (`scroll.c:50`). Tests : `planet_kj_dispatch` (`md`).
- 1a.9 — `mo` : sans classe (`entity.c:129`), provisoire, remplacée par un `qb` (`bootstrap.c:48`). Tests : `planet_kj_dispatch`.
- 1a.10 — `mz` (`wormhole.c:270`), `jj` lu sur la classe. Tests : aucun.
- 1a.11 — enveloppe `mz.jj` : sans classe (`wormhole.c:293`).
- 1a.12 — `bg` (`bg.c:69`), `pl` lu sur la classe, `ldown`/`rdown`, `lookup_named("mz"|"g_")` (`bg.c:185-191`). Tests : aucun.
- 1a.13 — `jc` (`slipway.c:392`), instanciée par `lookup_named("jc")` (`planet.c:482,491`, `planet_family_trade.c:104,110`). Tests : aucun.
- 1a.14 — `ie` (`research_icon.c:281`), `gy`/`id` → état statique (`research_icon.c:158,182`, `cart.c:158`), instancie `ie.ke` (`:198`). Tests : aucun.
- 1a.15 — enveloppe `ie.ke` (`research_icon.c:68`, `cls` non nul), lit `rr`/`s`.
- 1a.16 — `js` (`ui_button.c:116`), `ldown`, construite par les lignes de `pz()` (`msgbox.c:317`). Tests : `nil_row`, `ga_export_panel_by`, `planet_gh_row_lq`, `rich_planet_exports` (`lq`), `pause_menu_music` (`my`) ; harnais `pico8_js_new` de 13 tests.
- 1a.17 — `jo` : `ke` (`planet.c:140`), défauts lus `planet_display.c:184,296,532`, `research_display.c:265-266`. Tests : `nil_row`, `empty_planet_cleared`, `planet_gh_row_lq`, `planet_kj_dispatch`, `playability`, `rich_planet_exports`, `slipway_drag`.
- 1a.18 — `qh` : `jo` (`planet_family.c:306`), `gx[0]`. Tests : `empty_planet_cleared`.
- 1a.19 — `nl` : `qh` (`planet_family.c:322`), `gx[4]`.
- 1a.20 — `os` : `qh` (`planet_family.c:331`), `gx[1]`.
- 1a.21 — `hj` : `qh` (`planet_family.c:339`), `gx[6]`.
- 1a.22 — `pm` : `hj` (`planet_family.c:358`), via `kv[hp.cw]` (`planet.c:337,351`).
- 1a.23 — `mx` : `hj` (`planet_family_trade.c:333`), `gx[5]`.
- 1a.24 — `nr` : `qh` (`planet_family_trade.c:312`), `ldown`, `gx[2]`.
- 1a.25 — `fr` : `nr` (`planet_family_trade.c:326`), `gx[3]`.
- 1a.26 — `ga` : `jo` (`planet_family.c:378`), `by` réécrit (`:401-406`), `k_` lu (`:271`), via `kv[hp.cw]`. Tests : `ga_export_panel_by`, `geoharvest_burst`.
- 1a.27 — `g_` : `ke` (`probe.c:372`). Tests : `probe_mm_rnd_order`.
- 1a.28 — `fv` : `g_` (`help.c:129`).
- 1a.29 — `pj` : `ke` (`score_panel.c:348`), défauts mutés en place `ou`/`ea`/`cr` (`score_panel.c:245`, `score_panel_display.c:103-109`, `menu.c:241`). Tests : `game_end`, `score_panel_close`.
- 1a.30 — enveloppe de `pz()` sur `pj.cr` : sans classe (`menu.c:243`).
- 1a.31 — `ui` : `ke` (`ui.c:93`), `bt.."held"` → `lheld`/`rheld`, `bt.."down"` → `ldown`/`rdown` (`ui.c:316-352`), `le` réécrit (`:102`), `hw` lu (`probe.c:112`). Tests : `nil_row`, `ga_export_panel_by`, `pause_menu_music`, `planet_gh_row_lq`, `rich_planet_exports`, `tech_screen`, `slipway_drag`.
- 1a.32 — `trade` : racine (`trade.c:14`).
- 1a.33 — `hp` : racine (`research.c:75`), `mr`/`gp`/`fk` lus sur la classe, surcharges `ri[p]`, `p` ∈ `hp.mr` (8 clés, `collect.c:215`).
- 1a.34 — `lq` : racine (`layout.c:354`), champs positionnels → membre liste. Tests : `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`, `rich_planet_exports`.
- 1a.35 — `hy` : racine (`economy.c:60`). Tests : `cheat_code`, `empty_planet_cleared`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`.
- 1a.36 — `fm` : sans classe (`probe.c:385`), provisoire, remplacée par un `hy` (`cart.c:207`). Tests : ceux de 1a.35.
- 1a.37 — `{}` passé à `nr:gv` : sans classe (`planet_family_trade.c:94`).
- 1a.38 — `et={}` : sans classe (`probe.c:169`).
- 1a.39 — `{qs="-too far-"}` : sans classe (`slipway.c:255`).
- 1a.40 — `re={ra,p}` : sans classe (`slipway.c:112`).
- 1a.41 — enveloppe de `pz()` sur `g_ha` : sans classe (`menu.c:413`).
- 1a.42 — suppression de toutes les `pico8_class_t`, de `pico8_class.c` et de `test_pico8_entity_smoke` ; sortie de phase (grep ci-dessus vide, `make class-coverage-net`).

Écarts relevés avec le §2 : `bg`, `g_`, `pj` descendent de `ke` ; `fv` de `g_` ; « research » est `hp` ; vraies racines sans fille : `trade`, `lq`, `jb`, `hy`, `hp` ; 30 classes dans `src/` ; `mo`, `je`, `fm` sont provisoires ; 19 fichiers de test de régression lisent la couche, pas 17.

### Phase 1b — Types C réels

Struct par struct, chaque membre `pico8_value_t` prend son type C (`pico8_fixed_t`, `pico8_vec_t`, pointeur de struct typé, `const char *`, booléen…).

- **Nil** : un indicateur de présence **uniquement** là où le code observe nil (test de vérité, `mv` qui rend 0 sur champ absent, affectation `= nil`). Rappel : en Lua `0` est vrai, donc un fixe à 0 n'est pas nil. Pas de valeur sentinelle.
- Les REF deviennent des pointeurs typés : les sites de `pico8_ref_audit.h` disparaissent au fil de l'eau.
- **Sortie** : aucune struct d'entité ne contient de `pico8_value_t`.

### Phase D — Données

- Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
- Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
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

- **Sortie** : aucune struct d'entité n'enchâsse plus la struct de sa parente (tout type = noyau + archétype + composants) ; chaque ancienne classe a son archétype `static const` ; le test ponctuel est passé pour chacune avant d'être supprimé.

### Phase 3 — Renommage

Un champ est renommé **seulement** si `docs/glossaire-noms.md` en donne le sens. Sinon, il garde son nom minifié. Aucun sens n'est inventé. Un commit par groupe de champs renommés ; le glossaire est mis à jour en conséquence (l'entrée pointe du nom minifié vers le nouveau nom).

## 6. Tests existants

- Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
- Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés**. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.

## 7. Exécution

`plan-pilot`, phase après phase, un commit par classe (1a, 1b) ou par groupe (D, 2, 3).

- **Une étape `plan-pilot`** = une classe en 1a/1b, un groupe nommé en D, 2 et 3. La liste ordonnée des étapes d'une phase (classes et instances sans classe en 1a/1b, dans l'ordre du §5 ; groupes en D, 2 et 3) est écrite dans ce plan à l'entrée de la phase, avant son premier commit ; elle fait foi sur le §2 quand ils divergent, et chaque PRD d'étape reprend sa ligne telle quelle.
- Chaque commit de 1a passe `make class-coverage-net` (critère du PRD, hors `make test`), avec, pour chaque classe, un compte d'instanciations **égal** à celui de la porte 0.4 (valeurs versionnées à côté des exemptions à l'étape 1a.0).
- **Nommage des PRD** : `plan3-phase<X>-etape<M>-*.md` (X ∈ `0`, `1a`, `1b`, `D`, `2`, `3`). « La plus haute » s'entend dans l'ordre du plan — `0` < `1a` < `1b` < `D` < `2` < `3`, puis `M` numérique —, jamais dans l'ordre texte des noms. Seuls ces fichiers de `docs/prd/done/` comptent pour trouver l'étape suivante — les `phaseD-*`/`phaseE-*` de Plan 2 n'en sont pas. Les étapes 0.1 et 0.2 sont livrées, 0.3 est humaine : l'autopilote démarre à la porte 0.4, puis au premier commit de 1a.

L'autopilote **s'arrête et demande à l'humain** quand :

- le filet reste rouge après **3 tentatives** de correction sur le même commit ;
- un champ ne tombe sous aucune règle (§5 : clé dynamique à ensemble non fini ; en phase 2, champ ni archétype ni état, ou groupe au classement ambigu) ;
- un test de régression de bug devrait être modifié **dans son intention** (pas seulement dans sa façon de lire l'état) pour passer ;
- la porte de l'étape 0 échoue ;
- le diff modifie ou supprime un `.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` (seule exception : l'ajout de l'enregistrement de l'étape 0.3) — `make test` resterait vert.

Un site ou une instance sans classe découvert en cours d'étape et **couvert par une règle existante du §5** n'est pas un arrêt : l'autopilote l'ajoute à la liste des étapes dans le commit qui le traite, avec une ligne « découvert à 1a.M » dans le message de commit. Seul un site qu'aucune règle ne couvre déclenche l'arrêt.

Chacune de ces conditions est écrite dans le PRD de l'étape concernée comme critère que le vérificateur de `prd-autopilot` traduit en `FAILED:` — c'est le seul arrêt que `plan-pilot` honore.

## 8. Hors périmètre

- Le `malloc` des entités (pools, arènes, épuisement) : Plan 2 §8, à cadrer sur la base typée que ce plan produit.
- Toute comparaison de l'état des entités frame à frame.
- Tout changement de rendu ou de comportement.
- L'audio.
