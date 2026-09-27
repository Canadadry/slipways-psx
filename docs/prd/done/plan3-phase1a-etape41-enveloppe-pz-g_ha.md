---
title: "Plan 3, étape 1a.41 : l'enveloppe de `pz()` sur `g_ha` devient une struct statique sans parent à trois membres `hi`/`gq`/`ca`, et `pz` passe à la forme struct seule"
description: "`pico8_ix4_my_dispatch` passe à `pz` une enveloppe sur la pile (tag `INSTANCE`, `cls` NULL, `fields` = la table `g_ha`). C'est la dernière instance sans classe construite dans `src/`. Elle devient une struct statique sans parent avec trois membres empruntés à `g_ha` à l'init. Elle a un lecteur, aucun écrivain et aucune méthode. `g_ha` reste une table ob (phase D), et `pz`, qui ne reçoit plus que des structs, refuse désormais une instance."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.41 — enveloppe de `pz()` sur `g_ha` : sans classe (`menu.c:413`).

Le numéro de ligne n'est plus le bon. À HEAD 5577b96, l'enveloppe est dans `pico8_ix4_my_dispatch`, `src/pico8_menu.c:368-404` :

- `:371` : déclaration de `pico8_instance_t wrapper` sur la pile ;
- `:377-392` : installation des quatre `ha[i].my` sur les lignes de `g_ha` ;
- `:394-400` : si `g_ha` n'est pas NULL, `bs` reçoit les positionnels de `g_ha`, puis `wrapper.tag = INSTANCE`, `wrapper.fields = g_ha`, `wrapper.cls = NULL`, puis `pico8_pz(&wrapper, &bs)`.

Le commentaire `:365-367` décrit ce montage.

C'est le port littéral de `pico8/lua.lua:986-997` :

```lua
ha=ob([[
o(lq=o(31,"restart     ")),
o(lq=o(42,"toggle music")),
o(lq=o(40,"toggle sfx  ")),
o(lq=o(26,"how to play?"),fg=5,),
hi=v(0,107),gq=v(0,-11),
ca=14,
]])ix[4].my=function()for i,fn in pairs({_init,…})do
ha[i].my=fn
end
pz(ha,ha)end
```

En Lua, `ha` est à la fois `p` et `bs`. `pz(p,bs)` (`lua.lua:280-288`) lit sur `p` les champs `ep`, `eo`, `hi` (sinon `ra`), `ca`, `gq` et `ff`. Il écrit `ra`/`qf`/`fa` sur chaque ligne `b` de `bs`, jamais sur `p`. Le C sépare déjà `bs` (la liste des positionnels) de `p` (l'enveloppe) : l'enveloppe n'existe que parce que `pz` prend une instance et que `ha` est une table.

Règles du §5 qui s'appliquent :

> Chaque classe devient un `struct` […] ; chaque champ de sa chaîne […] devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui.

La phase D (§5) nomme explicitement « menus `g_ix`/`g_ha` » parmi les tables constantes qui deviendront des données `static const`. **La table `g_ha` n'est donc pas l'objet de cette étape** : seule l'enveloppe l'est.

§6 : la ligne du plan ne nomme aucun test de régression. `pause_menu_music` lit `g_ha` et doit passer avec un diff vide. §7 : `make class-coverage-net` rend une sortie identique à `tools/class_coverage_net_expected.txt`. La preuve des tests passe par `gcov` et 2 ou 3 mutations. La note « Découvert à 1a.41 » suit la forme courte.

### Ce que le plan attend aussi de cette étape

- 1a.28 : « restent à double forme : […] `pz` (1a.30, 1a.41) ».
- 1a.30 : « `pz` reste à double forme pour 1a.41 ».

Après cette étape, plus aucun appelant ne passe d'instance à `pz`. Il passe donc à la forme struct seule, comme `ke_ks`/`dg` à 1a.31 (`test_ke_closed`) et les fonctions de `g_` à 1a.28 (`test_g__closed`). Le type `const pico8_instance_t *` de son paramètre reste jusqu'à 1a.42, comme pour `bv`/`dl`/`jc:hq` (1a.38 à 1a.40).

### Précédents réutilisés

- **1a.30 (c1da235), enveloppe de `pz` sur `pj.cr`** : c'est le jumeau direct. Il donne la struct à en-tête sans `pico8_ke_t`, le descripteur `static const` sans méthode, l'écrivain NULL, le lecteur limité aux membres (`eo`/`ra`/`ff` absents : `rn(62)`, départ sur `hi`, `ok.le` non vidé), l'accesseur public du descripteur et un cast vers `const pico8_instance_t *` à l'appel. **Une différence** : à 1a.30, la table `cr` était un défaut de classe modifié en place (§5, « défauts mutés »), donc la table et l'enveloppe ont fusionné. Ici, `g_ha` est une globale de module, sans classe. Ses seules mutations en place visent ses **lignes** (`ha[i].my`, puis `ra`/`qf`/`fa` écrits par `pz_step`), jamais `hi`/`gq`/`ca` ni ses emplacements indexés. La règle des défauts mutés ne s'applique donc pas, et la phase D garde la table.
- **1a.16 (`js` construit par les lignes de `pz`)** : les lignes restent des tables ob (assert `pz_step`, `msgbox.c:421`). Rien ne change de ce côté.
- **1a.34 (`lq`, positionnels → membre liste)** : cette règle ne s'applique pas ici. Les positionnels de `ha` arrivent à `pz` comme `bs`, le second argument, et `pz` ne les lit jamais sur `p`. Voir la décision (G) plus bas.
- **1a.37 à 1a.40** : objet jetable devenu struct sans parent, avec accesseur public du descripteur, tests adaptés dans leur seule façon de reconnaître l'objet, et journal de lancements.

### Emprise (HEAD 5577b96, lecture du code et grep ; rien n'a été lancé)

| Aspect | Constat |
|---|---|
| Construction | 1 site : `pico8_ix4_my_dispatch`, `menu.c:371,396-399`, sur la pile, à chaque clic sur `ix[4]` (bouton pause). Ni classe, ni `pico8_class_instantiate`, ni `lt`, donc jamais dans `hv`/`cv`/`cg`. Pas de nom dans `KNOWN[]`, rien au crochet de couverture. |
| Vie | la durée de l'appel. `g_ha` (dont l'enveloppe emprunte les champs) vit de `pico8_menu_init` (`:437`) à `pico8_menu_shutdown` (`:470-473`). Un init re-parse `g_ha` (`menu.h:25-27`). |
| Champs présents | `hi` = REF `v(0,107)`, `gq` = REF `v(0,-11)`, `ca` = 14, plus les positionnels `[1..4]` (lignes). |
| Champs lus sur `p` par `pz`/`pz_step` | `ep` (absent : pas de ligne d'en-tête), `eo` (absent : `rn(62)`), `hi` (présent : `ra` = sa valeur, copiée dans `ctx.ra_heap`), `ra` (non lu, puisque `hi` est présent), `ca` (présent : `fa` de chaque ligne, TROU-9), `gq` (présent : pas de `qf`, valeur copiée), `ff` (absent : `ok.le` non vidé). `pz` ne lit jamais les positionnels sur `p`. |
| Écrivains | aucun. `pz`/`pz_step` n'écrivent que sur les lignes et sur `ok`. `ix4` écrit `my` sur les **lignes** de `g_ha`, pas sur l'enveloppe. |
| Méthodes | aucune. Aucun des noms lus n'est une méthode de l'enum (`pico8_method_of`). |
| Lecteurs de `g_ha` hors de l'enveloppe | `pico8_ha()` (`menu.c:410`, `menu.h:21`), lu seulement par `test_pico8_pause_menu_music` (`:52`, `ha[2].my`). `pico8_menu_positional_values(g_ha)` pour `bs`. Personne ne lit `hi`/`gq`/`ca` de la table hors de `pz`. |
| Autres appelants de `pz` | `help.c:79` (`fv`, struct), `menu.c:229` (`pj.cr`, struct depuis 1a.30), `planet_display.c:334,462` (`self` de `jo:gh`/`jo:fe`, planète struct depuis 1a.26). Aucun test n'appelle `pico8_pz` directement. |
| Tests qui traversent le chemin | `test_pico8_js` : `test_pause` (`:500-510`, `my` de `ix[4]` appelé, `check_shared_ra`, `fold_le` → `PAUSE_HASH` 0xF715AAC0 : nombre de lignes, et par bouton `fa` = `ca`, `ra` = `hi`, `qf` = `hi` + i×`gq`, `lq`, `my`) et `test_replay_pause` (`PAUSE_REPLAY_HASH`). `test_pico8_ui` : `call_row_my(pico8_ix(), 4)` à `:351` (`SHARED_HASH`) et `:563` (`CJ_HASH`). `test_pico8_pause_menu_music` (régression). Le rejeu `pause_menu_music_events.json` du filet (`FRAMES_EVENTS`). |
| Clé dynamique | aucune. |

## Solution

1. **Struct de l'enveloppe**, privée au module du menu (`pico8_menu.c`), avec son type exposé dans `pico8_menu.h` seulement si le test de forme en a besoin : un `pico8_struct_header_t`, puis trois membres `pico8_value_t` au nom minifié conservé, `hi`, `gq`, `ca`. Pas de `pico8_ke_t` : c'est une instance sans classe, sans parent, jamais une entité. Nom C au choix de l'implémenteur, rattaché à `ha`/`pz`, sans nom minifié inventé.
2. **Un seul objet statique**, rempli à l'init du menu à partir de `g_ha` et vidé au shutdown :
   - à `pico8_menu_init`, juste après le parse de `g_ha`, `hi`, `gq` et `ca` reçoivent **les mêmes valeurs que la table** (même REF de vecteur pour `hi`/`gq`, même fixe pour `ca`) : l'enveloppe emprunte les champs de `g_ha`, comme `wrapper.fields = g_ha` aujourd'hui ;
   - des asserts porteurs de message vérifient que la spec donne bien `hi`/`gq` en REF de vecteur non NULL et `ca` en fixe ;
   - à `pico8_menu_shutdown`, avant la libération de `g_ha`, les trois membres reviennent à nil, pour ne laisser aucune REF pendante ;
   - la spec de `g_ha` **ne change pas**. Ses `hi`/`gq`/`ca` restent dans la table (phase D) et sont la seule source des valeurs.
3. **Descripteur `static const`** : aucune méthode, écrivain NULL (aucun écrivain générique n'atteint l'enveloppe). Le lecteur rend `hi`, `gq`, `ca` quand ils ne sont pas nil, et 0 pour tout autre nom (`ep`, `eo`, `ra`, `ff`, `fo`, `nu`, `my`…). Il n'a pas d'assert sur un nom inconnu. Un accesseur public du descripteur sert aux tests (modèle `pico8_pj_cr_struct_class()`, `pico8_jc_too_far_struct_class()`).
4. **Accesseur de l'objet** : il rend l'objet quand `g_ha` n'est pas NULL, et NULL sinon, avec le même contrat de vie que `pico8_ha()`. Il sert au test de forme.
5. **`pico8_ix4_my_dispatch`** : les installations `ha[i].my` et leur ordre ne changent pas. La garde `if (g_ha != NULL)` et `bs` = positionnels de `g_ha` non plus. `pz` reçoit l'objet par un cast vers `const pico8_instance_t *` (comme `help.c:79` et `menu.c:229`). `pico8_instance_t wrapper` disparaît, et le commentaire `:365-367` est corrigé (au plus un contrat : `ha` donne ses lignes comme `bs`, l'enveloppe emprunte ses `hi`/`gq`/`ca`).
6. **`pz` passe à la forme struct seule** : juste après la garde `p == NULL`, un assert porteur de message exige le tag `STRUCT`. Les lectures restent par `pico8_entity_get_field`/`pico8_msgbox_field_vec`, parce que `p` est hétérogène (`fv`, planète, `pj.cr`, enveloppe de `ha`) et que `pico8_entity_get_field` sur une struct passe par le lecteur du descripteur. Le type du paramètre reste `const pico8_instance_t *` jusqu'à 1a.42. Le contrat de `pico8_msgbox.h:86-91` (« `p` may be a classless field-bag … `pz(ha,ha)` ») devient faux : il est corrigé (`p` est une struct dont le descripteur a un lecteur).
7. **`pico8_ha()`, `g_ha`, les lignes, `pico8_menu_positional_values`** : inchangés.
8. **Plan** : corriger la référence de ligne de 1a.41 (`menu.c:413` → le site réel) et ajouter la note « Découvert à 1a.41 : enveloppe de `g_ha` en struct statique sans parent à trois membres `hi`/`gq`/`ca` empruntés à `g_ha` à l'init, sans écrivain ni méthode ; `g_ha` reste une table ob (phase D) ; `pz` struct seule, le type `pico8_instance_t *` de son paramètre tombe à 1a.42 ; plus aucune instance sans classe construite dans `src/` ; enum inchangée. » Tout site découvert en cours d'étape qui change une étape suivante s'ajoute à cette note.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que l'enveloppe de `pz` sur `g_ha` soit la seule instance convertie par ce commit, afin de respecter « une classe par commit ».
2. En tant que lecteur du code, je veux que l'enveloppe soit une struct à en-tête avec les membres `hi`, `gq`, `ca`, afin que la traçabilité vers `hi=v(0,107),gq=v(0,-11),ca=14` de `lua.lua:991-992` tienne.
3. En tant que lecteur du code, je veux que la struct n'ait pas de `pico8_ke_t`, afin que « sans parent » se lise dans le type.
4. En tant que mainteneur, je veux que les valeurs de l'enveloppe viennent de la table `g_ha` parsée, et non de constantes recopiées en C, afin qu'il n'y ait qu'une source pour `hi`/`gq`/`ca` tant que la phase D n'a pas remplacé la table.
5. En tant qu'auteur de la phase D, je veux que `g_ha` reste une table ob avec sa spec inchangée, afin de générer ses données à partir de ce que `ob()` produit aujourd'hui.
6. En tant que gardien du test de régression `pause_menu_music`, je veux que `pico8_ha()` et les lignes de `g_ha` ne changent pas, afin que ce test passe avec un diff vide.
7. En tant que joueur, je veux que le menu pause affiche ses quatre lignes aux mêmes positions (depuis `v(0,107)`, pas de `v(0,-11)`), avec le même `fa` = 14 et le même clic sur « toggle music », afin que le rejeu du filet ne bouge pas.
8. En tant que gardien du filet, je veux que `ep`, `eo`, `ra` et `ff` restent absents de l'enveloppe, afin que `pz` n'ajoute pas de ligne d'en-tête, joue `rn(62)`, parte de `hi` et ne vide pas `ok.le`.
9. En tant que mainteneur, je veux que l'enveloppe n'ait pas d'écrivain, afin qu'une écriture jamais mesurée arrête l'exécution.
10. En tant que mainteneur, je veux que l'enveloppe n'ait aucune méthode, afin qu'un `mv` sur elle rende 0 comme aujourd'hui.
11. En tant que gardien de la mémoire, je veux que l'enveloppe ne soit ni allouée ni libérée et qu'elle ne garde aucune REF après le shutdown du menu, afin qu'ASan reste propre et que les cycles init/shutdown ne laissent rien de pendant.
12. En tant que mainteneur, je veux que `pz` refuse une instance, afin que la double forme ouverte à 1a.28 et reportée à 1a.30 soit fermée là où le plan l'a prévu.
13. En tant que mainteneur, je veux que le contrat d'en-tête de `pz` dise ce qu'il reçoit vraiment, une struct avec lecteur, afin de ne laisser aucun contrat mensonger.
14. En tant qu'auteur de 1a.42, je veux que le type `const pico8_instance_t *` du paramètre de `pz` reste tel quel, afin que le retypage des paramètres reste une étape à part.
15. En tant qu'auteur de 1a.42, je veux une liste des constructions d'instances et des asserts `INSTANCE || STRUCT` qui restent, afin de savoir ce que la suppression de `pico8_class.c` va casser.
16. En tant que gardien de la couverture, je veux qu'aucun nom ne s'ajoute à `KNOWN[]`, afin que `make class-coverage-net` rende une sortie identique.
17. En tant que vérificateur, je veux la preuve qu'un test existant (`PAUSE_HASH`) pinne déjà ce chemin, par une mutation rouge sur l'ancien code, afin de pouvoir m'appuyer sur lui comme caractérisation.
18. En tant que vérificateur, je veux un test de forme de l'enveloppe et un test qui montre que `pz` refuse une instance, afin que la nouvelle forme soit elle-même gardée.
19. En tant que vérificateur, je veux un rapport `gcov` et 2 ou 3 mutations sur le nouveau code, chacune rouge, afin de voir que chaque ligne ajoutée est exécutée et que les valeurs sont gardées.
20. En tant que vérificateur, je veux un journal de chaque lancement de test (commande, état de l'arbre, code de sortie, ligne clé, durée), afin de réutiliser les résultats au lieu de tout relancer.
21. En tant que lecteur du plan, je veux la note « Découvert à 1a.41 » sous forme courte, avec la ligne du site corrigée, afin de savoir ce qui change pour 1a.42.
22. En tant que lecteur du code, je veux que les commentaires rendus faux soient corrigés, et que les ajouts ne portent qu'un contrat C ou un piège (CLAUDE.md), afin de n'avoir ni récit de chantier ni contrat mensonger.

## Implementation Decisions

Pas d'humain dans la boucle. J'ai appliqué le skill `guideline` moi-même (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : le menu (struct, objet, descripteur, accesseurs, init/shutdown, `ix[4].my`), `msgbox` (assert struct seule dans `pz` et contrat d'en-tête), `test_pico8_js` (tests ajoutés), la ligne 1a.41 du plan. Rien d'autre dans `src/` : `help`, `score_panel`, `planet_display`, `ui`, `entity`, `pico8_struct.h` (enum), `cart` et `bootstrap` restent tels quels.
- **(G) Convertir l'enveloppe, pas la table.** Deux solutions sont rejetées :
  - **fusionner `g_ha` et l'enveloppe** (modèle 1a.30). La règle qui justifiait la fusion à 1a.30 (défaut de classe modifié en place) ne vise pas `g_ha`. La fusion changerait le type de `pico8_ha()` et forcerait la réécriture de `pause_menu_music`. Elle ferait aussi des lignes un membre liste d'une struct, ce qui est une conversion de données réservée à la phase D ;
  - **lire les positionnels comme champs de l'enveloppe** (règle `lq` de 1a.34). `pz` ne les lit jamais sur `p` : ils sont son second argument `bs`, déjà séparé en C. Un membre liste serait un champ que personne ne lit.
- **(G) Objet statique rempli à l'init depuis `g_ha`, plutôt que des constantes à la compilation (1a.30) ou une struct sur la pile par appel.**
  - Les constantes en C dupliqueraient la spec, qui reste en place. À 1a.30, la spec `cr` était devenue une copie morte, alors qu'ici `g_ha` reste la table vivante que `pico8_ha()` rend.
  - Une struct par appel ne serait pas atteignable par un test de forme. Elle relirait aussi la table à chaque clic, sans rien gagner : aucun champ de l'enveloppe ne porte de valeur propre à l'appel (contraste avec 1a.40), et aucun n'est jamais écrit.
  - Même REF que la table pour `hi`/`gq` : c'est le « REF prise […] à l'init » du §5 appliqué à une table de module. Observablement, c'est neutre, puisque `pz` copie les valeurs (`ctx.ra_heap`, `bp`).
- **(G) Vie liée à celle de `g_ha`** : l'accesseur rend NULL quand `g_ha` est NULL, et les membres reviennent à nil au shutdown. C'est le contrat de `pico8_ha()`, et celui de `pico8_pj_cr()` à 1a.30.
- **(G) `pz` struct seule** : un seul assert à l'entrée, sans rien changer aux lectures. Justification : le plan attribue la double forme de `pz` à 1a.30 **et 1a.41** (lignes 1a.28, 1a.30), et 1a.41 retire le dernier appelant à instance. **Repli** : si H1 (ci-dessous) montre qu'un appelant passe encore une instance à l'exécution, l'implémenteur **retire l'assert et son test** et consigne l'appelant comme constat pour 1a.42 dans le message de commit et la note du plan. Ce n'est ni un arrêt ni un `FAILED`, et cet appelant n'est pas converti ici.
- **Pas d'écrivain** (modèle `mz.jj`, `pj.cr`, `{ra,p}`) : `set_field` NULL. Si une écriture générique sur l'enveloppe apparaissait (l'assert de `pico8_entity_set_field` partirait), ce serait un champ sans règle mesurée : arrêt §7 « champ sans règle », pas un membre ajouté en silence.
- **Lecteur** : jamais NULL (`pico8_entity_get_field` l'exige), et un membre nil est absent (convention de toutes les structs converties).
- **Signatures** : `pico8_pz` garde `const pico8_instance_t *p`. `pico8_ha()` garde son type et son contrat. Aucune entrée d'enum n'est ajoutée.
- **TROU** : aucun écart port/cart n'est connu sur ce chemin. `TROU-9` (le `fa` n'est posé que si `ca` est présent) et `TROU-7` (fuites de `pz`) restent aux sites, sans changement. Un écart découvert en cours d'étape est préservé, avec une nouvelle entrée `TROU-N` et un `/* TROU-N */` nu au site, jamais corrigé.
- **Commentaires** (CLAUDE.md) : au plus un contrat C sur l'objet (emprunte à `g_ha` ses `hi`/`gq`/`ca` à l'init, vidé au shutdown, jamais alloué). Les asserts de l'init et de `pz` portent leur message. Le commentaire `menu.c:365-367` et le contrat `msgbox.h:86-91` sont corrigés. Tout le reste va dans le message de commit.

### Hypothèses à vérifier à l'exécution (rien n'a été lancé pour ce PRD)

- **H1** : tout appelant de `pz` passe une struct à l'exécution (`fv`, `pj.cr`, planètes, enveloppe de `ha`). Vérifié par `make test` + `make e2e` verts avec l'assert. Sinon, appliquer le repli ci-dessus.
- **H2** : `PAUSE_HASH` pinne `hi`/`gq`/`ca`. Vérifié sur l'**ancien code**, dans une copie jetable : `ca=14` → `ca=15` dans la spec de `g_ha` rend `test_pause` rouge ; une seconde mutation `hi=v(0,107)` → `v(0,108)` le rend aussi rouge (facultative).
- **H3** : la spec de `g_ha` parse `hi`/`gq` en REF de vecteur et `ca` en fixe (les asserts de l'init le montrent au premier `make test`).
- **H4** : `test_pico8_js` reste sous 30 s après ajout des tests (sinon, il passe dans `UNIT_TESTS_SLOW`). Durée relevée dans le journal.
- **H5** : aucun code de `src/` ne lit `hi`/`gq`/`ca` sur la table `g_ha` hors de `pz` (grep à refaire ; si un tel lecteur existe, il reste sur la table, sans changement).

## Testing Decisions

Un bon test lit ce que `pz` produit (les boutons de `ok.le`, leurs `fa`/`ra`/`qf`/`lq`/`my`) et ce que l'API à double forme rend. Il ne regarde pas la disposition mémoire. Seule la reconnaissance de l'objet passe par l'accesseur du descripteur. Prior art : `test_pico8_pj` `test_cr_is_a_struct` (1a.30), `test_pico8_jc` (1a.39/1a.40), `test_ke_closed` de `test_pico8_ui` et `test_g__closed` de `test_pico8_fv` (fonctions fermées aux instances, `must_abort`).

### Caractérisation : un test existant pinne déjà le chemin

`test_pico8_js` `test_pause` (`PAUSE_HASH`) plie, pour chaque bouton de `ok.le` après `ix[4].my` : `fa` (= `ca`), `ra` (= `hi`, REF partagée par `check_shared_ra`), `qf` (= `hi` + i×`gq`), `lq`, `my`, ainsi que le nombre de lignes (4 : `ep` absent, `ff` absent). S'y ajoutent `test_replay_pause` (`PAUSE_REPLAY_HASH`), `test_pico8_ui` (`SHARED_HASH`, `CJ_HASH`), `pause_menu_music` et le rejeu du filet. **Aucune nouvelle caractérisation n'est écrite** : la preuve que ce filet mord est H2, une mutation rouge sur l'ancien code (copie jetable), notée dans le journal. Aucune valeur figée, aucun `*_HASH`, aucun `check(...)` attendu n'est édité.

### Ajouté après conversion, dans `test_pico8_js` (rapide, `make test`)

1. **Forme de l'enveloppe** (nom au choix, par exemple `test_ha_pz_is_a_struct`), lancée par `in_fresh_child` après `start_game(PAUSE)` :
   - l'accesseur rend un objet de tag `STRUCT`, dont le descripteur est celui de l'accesseur public ;
   - aucune méthode (`pico8_mv(o, "ph"/"ldown"/"my", …)` rend 0, ou le descripteur n'a que des NULL) ;
   - `hi`, `gq`, `ca` présents et **identiques** à ceux de `pico8_ha()` (même REF pour `hi`/`gq`, même fixe pour `ca`) ;
   - `ep`, `eo`, `ra`, `ff`, `fo`, `nu`, `my` absents ;
   - `must_abort` sur une écriture générique de `hi`, de `ca` et de `ep` ;
   - après `pico8_menu_shutdown()`, l'accesseur rend NULL. Après un nouveau `pico8_menu_init()`, `hi` est la REF de la **nouvelle** table.
2. **`pz` fermé aux instances** (par exemple `test_pz_closed`) : `must_abort` sur `pico8_pz` avec une instance faite à la main (`{PICO8_TYPE_TAG_INSTANCE, …, NULL}`, comme `test_g__closed`) et une liste vide. `pico8_pz(NULL, …)` ne fait toujours rien.

`test_pause` et tous les autres tests existants passent **sans aucune modification**. `test_pico8_pause_menu_music` a un diff vide : il lit `pico8_ha()`, qui ne change pas, donc aucune preuve rouge du §6 n'est requise. Sa preuve rouge connue, « `pz` avant les installations `ha[i].my` », sert de mutation 3 ci-dessous.

### `gcov`

Dans une copie jetable du scratchpad, supprimée ensuite, compiler en `--coverage -O0` `test_pico8_js` et `test_pico8_pause_menu_music` (précédents 1a.38 à 1a.40 : `__gcov_dump` avant `_exit` par `-include`, parce que les tests tournent dans des fils), puis lancer `gcov` sur `pico8_menu.c` et `pico8_msgbox.c`. Chaque ligne exécutable ajoutée ou modifiée doit être exécutée : remplissage à l'init, remise à nil au shutdown, lecteur (membre servi, nom inconnu → 0), les deux accesseurs, l'appel à `pz` avec l'objet, et l'assert de `pz`. Une ligne à 0 est couverte par un test ajouté ou justifiée dans le message de commit.

### Mutations sur le nouveau code (au moins trois, copie jetable, restaurées, `cmp` contre l'arbre)

1. Le lecteur ne sert plus `ca` → `test_pause` rouge (`fa` des boutons, `PAUSE_HASH`).
2. L'init remplit `gq` avec la REF de `hi` → `test_pause` rouge (`qf`, `PAUSE_HASH`) et test de forme rouge (identité avec la table).
3. Dans `ix[4].my`, `pz` est appelé avant les installations `ha[i].my` → `test_pico8_pause_menu_music` rouge (sa preuve connue) et `PAUSE_HASH` rouge (`my`).
4. (facultative) Le lecteur rend `ff` présent → `test_pause` rouge (`ok.le` vidé).

Elles sont rapportées dans le message de commit avec la ligne qui rougit.

### Budget et journal des lancements

Budget : **environ 20 minutes de temps total de tests**. Ordres de grandeur (CLAUDE.md, 1a.39/1a.40) : `make test` de 20 s à une minute, `make e2e` de 4 à 6 min, `make class-coverage-net` de 3 à 4 min. Répartition indicative :

- la boucle se fait sur `test_pico8_js` seul (H2 sur l'ancien code, tests ajoutés, mutations) et sur `test_pico8_pause_menu_music` pour la mutation 3 ;
- `gcov` une fois ;
- `make test` 2 à 3 fois ;
- `make e2e` une fois en fin ;
- `make class-coverage-net` une fois en fin.

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque plutôt que de relancer.

**Chaque lancement est journalisé dans le journal de progression** avec :

- la commande exacte ;
- l'état de l'arbre (HEAD + « ancien code » / « converti » / « mutation N, copie jetable ») ;
- le code de sortie ;
- la ligne clé (par exemple `test_pico8_js: all tests passed`, `frames-check : N conformes`, ou la ligne d'assert qui rougit) ;
- la durée.

Le vérificateur réutilise ce journal et ne relance que ce qui est postérieur au dernier changement de l'arbre.

Commandes : `timeout 10m make test` ; `timeout 15m make e2e` ; `timeout 15m make class-coverage-net`. Ne jamais lancer l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- le filet reste rouge après 3 tentatives de correction sur le même commit ;
- le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS`/`RECORDINGS` ;
- `make class-coverage-net` n'est pas identique à `tools/class_coverage_net_expected.txt`, ou un nom est ajouté à `KNOWN[]` ;
- un champ ne tombe sous aucune règle du §5 (par exemple, un écrivain découvert sur l'enveloppe) ;
- un test de régression (`pause_menu_music` compris) devrait changer d'intention pour passer, ou changerait sans preuve rouge du §6 (attendu ici : diff vide) ;
- une autre classe ou instance sans classe est convertie ou modifiée (`g_ha` elle-même, `g_ix`, les lignes, `pj.cr`, `fv`, la cible de `bv`, `{qs}`, `{ra,p}`…) ;
- une valeur figée ou une empreinte (`#define …_HASH`, `check(...)` attendu) est éditée ;
- un écart port/cart est corrigé au lieu d'être préservé ;
- l'enveloppe a un parent, une méthode ou un écrivain, ou elle est allouée par appel ;
- la note « Découvert à 1a.41 », le rapport `gcov`, les mutations ou le journal manquent.

## Out of Scope

- La table `g_ha` (et `g_ix`) comme données : phase D.
- Le type `const pico8_instance_t *` du paramètre de `pz` et des autres fonctions, et la suppression des `pico8_class_t`, de `pico8_class.c` et de `test_pico8_entity_smoke` : 1a.42.
- Les fuites de `pz` (`ra_heap`, `qf`, `le`, TROU-7) : Plan 2 §8.
- Tout changement de rendu ou de comportement, et l'audio (`rn`, `music`).

## Further Notes

- Étape d'un seul commit, sans morceaux : deux fichiers de `src/` (menu, msgbox), leurs en-têtes, un fichier de test, une ligne du plan.
- Numéros de ligne relevés à HEAD 5577b96. Rien n'a été compilé ni lancé pour ce PRD.
- **Constat pour 1a.42 (sans élargir cette étape)**. Une fois l'enveloppe convertie, `grep` ne trouve plus aucune instance sans classe construite dans `src/` (le dernier `.cls = NULL` / `tag = PICO8_TYPE_TAG_INSTANCE` hors de `pico8_class.c` est `menu.c:396-398`). Il reste pour 1a.42 :
  - **constructions par classe-valeur** (`pico8_class_instantiate`, qui délègue au constructeur de struct) : `planet.c:515` (`kv[hp.cw]`), `planet.c:656` et `planet_family_trade.c:126` (`jc` par `lookup_named`), `research_icon.c:199` (`ie.ke`), `bg.c:246` (`lookup_named("mz"|"g_")`) ;
  - **asserts qui acceptent encore `INSTANCE || STRUCT`** : `score_panel.c:215`, `probe.c:234`, `slipway.c:211`, `entity_geom.c:103`, `entity.c:304`, `wormhole_display.c:49` ;
  - **aiguillages sur le tag** dans `pico8_entity_get_field`/`set_field` (`entity.c:177,194`), et l'accesseur `pico8_instance_fields` ;
  - 17 sites `->defaults`/`lookup_named` dans 13 fichiers, et `pico8_instance_t` cité dans 50 fichiers de `src/` ;
  - **tests** qui construisent des instances : `test_pico8_mo:100-101` (instance faite à la main pour `gd`), `test_pico8_fv:662`, `test_pico8_ui:739`, `test_pico8_jc:410,873,935`, `test_pico8_bg:291,645`, `test_pico8_trade:551`, `test_pico8_g_:483`, `test_pico8_class_chain`, `test_class_coverage`, `test_pico8_entity_smoke` (supprimé à 1a.42) ;
  - **empreintes** : `test_pico8_js:244,277` et `test_pico8_g_:186` plient la **valeur** `PICO8_TYPE_TAG_INSTANCE`. Si la macro disparaît, il faut garder la constante numérique `0x9E5C1451u` pour que les `*_HASH` ne bougent pas ;
  - le test `test_pz_closed` ajouté ici utilisera lui aussi une instance faite à la main, à traiter comme `test_g__closed`.
- Le message de commit porte :
  - le site réel et la ligne corrigée du plan ;
  - la forme choisie (statique, empruntée à `g_ha` à l'init, sans écrivain) et pourquoi ni la fusion ni une struct par appel ;
  - `pz` struct seule, ou le repli H1 ;
  - le journal résumé des lancements et la preuve H2 sur l'ancien code ;
  - le rapport `gcov` et les mutations tuées ;
  - les réponses à H1-H5 ;
  - le constat pour 1a.42 ;
  - « Découvert à 1a.41 ».
