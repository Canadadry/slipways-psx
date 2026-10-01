---
title: "Plan 3, étape 1a.42 : suppression de toutes les `pico8_class_t`, de `pico8_class.c` et de `test_pico8_entity_smoke` ; sortie de la phase 1a (à découper en 1a.42a à 1a.42e)"
description: "Toutes les entités sont des structs depuis 1a.41, mais la couche de classes reste : 30 `pico8_class_t` déclarées pour leurs défauts, leur rôle de classe-valeur (`gx[]`, `ie.ke`, `kv[hp.cw]`), leurs registres et leur rôle de témoin d'init, et `pico8_instance_t` sert de type générique d'entité dans 50 fichiers de `src/`. La sortie de phase exige un grep vide sur 427 lignes de 70 fichiers. L'étape dépasse un seul passage : elle est découpée en cinq commits, chacun avec `make test` vert."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.42 — suppression de toutes les `pico8_class_t`, de `pico8_class.c` et de `test_pico8_entity_smoke` ; sortie de phase (grep ci-dessus vide, `make class-coverage-net`).

Critère de sortie de la phase 1a (§5, « Sortie ») :

> plus aucune `pico8_class_t`/`pico8_instance_t` à l'exécution ; `pico8_class.c` supprimé ; `make class-coverage-net` toujours sans zéro — vérifié par `grep -rn 'pico8_class_t\|pico8_instance_t\|->defaults\|lookup_named' src/` vide.

Le §5 (« Héritage entre formes et classes-valeurs ») annonce déjà ce que fait cette étape : « une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a, qui les supprime toutes avec `pico8_class.c` ».

Le §6 ajoute : « Les tests **de la couche** (`test_pico8_entity_smoke`, …) sont supprimés dans le commit qui supprime le code testé ». `test_pico8_class_chain` ne teste que `pico8_class.c` : il tombe donc sous la même règle, même si la ligne de l'étape ne le nomme pas.

### État de départ

HEAD b46e45b, plus les changements non commités de 1a.41 (`pico8_menu.c/.h`, `pico8_msgbox.c/.h`, `test_pico8_js.c`, `docs/plan-3.md`), tenus pour acquis. Conséquences de 1a.41 :

- `pz` reçoit la struct statique `g_ha_pz` et asserte le tag `STRUCT` ;
- le type de son paramètre reste `const pico8_instance_t *` (`pico8_msgbox.h:91`), à retyper ici ;
- `test_pz_closed` (`test_pico8_js.c:1153`) construit une instance à la main.

Tous les numéros de ligne ci-dessous sont relevés sur cet arbre. **Rien n'a été compilé ni lancé pour ce PRD** : les faits d'exécution sont des hypothèses (H1 à H14).

### Pourquoi l'étape doit être découpée

- le grep de sortie rend aujourd'hui **427 lignes dans 70 fichiers de `src/`** ;
- 46 fichiers de `tests/` nomment l'API de classe ou `pico8_instance_t` ;
- en ajoutant le `Makefile`, `tools/` et la doc, un seul passage toucherait plus de 110 fichiers, pour ~20 minutes de tests.

C'est bien au-delà du seuil d'un implémenteur (~40 fichiers). La section « Implementation Chunks » propose cinq sous-étapes, **1a.42a à 1a.42e**, dans un ordre imposé. Chacune est un commit et garde `make test` vert, les `*.sha256` identiques et `make class-coverage-net` identique à la référence. Précédent de nommage : `plan3-phase1a-etape16b-…`.

**Recommandation à `plan-pilot`** : couper ce PRD en cinq PRD `plan3-phase1a-etape42a-…` à `…42e-…`. Chaque section « Chunk » ci-dessous est écrite pour être découpée telle quelle. Chaque PRD reçoit son propre budget d'environ 20 minutes et son propre tableau « Runs ». Si ce PRD est exécuté d'un bloc, l'implémenteur fait **un seul chunk par passage**, puis s'arrête avec le chunk suivant noté comme restant.

### Inventaire

#### 1. Références du grep de sortie (427 lignes, 70 fichiers)

Comptes par fichier (`grep -c`) : `slipway.c` 29, `probe.c` 25, `class.c` 24, `planet_family.c` 23, `planet_family_trade.c` 22, `ui.c` 21, `economy_tiers.c` 19, `class.h` 17, `planet.c` 14, `economy_score.c` 14, `wormhole.c` 9, `research_icon.c` 9, `entity.c` 9, `economy_formulas.c` 9, `planet_display.c` 8, `particle.c` 8, `msgbox.c` 8, `entity_geom.c` 8, `ui_button.c` 7, `slipway_display.c` 7, `score_panel.c` 7, `probe.h` 7, `planet_family.h` 7, `help.c` 7, `entity.h` 6, `bg.c` 6, et 44 fichiers à 5 ou moins (en-têtes surtout). Elles se rangent en six familles.

**a) `lookup_named` : 5 appels, 6 inscriptions, 5 commentaires.**

Appels :

| Site | Rôle |
|---|---|
| `planet.c:500` | `kv[hp.cw]` |
| `planet.c:647` | `jc` de `jo:ldown` |
| `planet_family_trade.c:120` | `jc` de `nr:ldown` |
| `bg.c:240` | `"mz"` ou `"g_"` de `bg:ldown` |

Inscriptions `pico8_class_register_named` : `planet.c:312` (`jo`, jamais cherché dans `src/`), `planet_family.c:1059` (`pm`), `:1102` (`ga`), `probe.c:575` (`g_`), `slipway.c:579` (`jc`), `wormhole.c:401` (`mz`).

Commentaires qui tombent sous le grep : `planet.h:69`, `planet_family.h:175`, `wormhole.h:55`, `slipway.h:45`, `probe.h:64`.

Dans les tests : `jc` ×4 (`test_pico8_jc:404,873,935`, `test_pico8_mx:534`, `test_pico8_g_:483`), `pm` ×3, `g_` ×1, `mz` ×1. Fichiers concernés : `test_pico8_jc`, `mx`, `ga`, `pm`, `mz`, `g_`, `hj`.

**b) `->defaults` : 6 sites de lecture ou d'écriture directe.**

| Site | Défaut |
|---|---|
| `score_panel.c:518` | `pj`, boucle des défauts |
| `help.c:239` | `fv` |
| `planet_family.c:1093,1098` | `ga.by`, échange table → liste |
| `ui.c:212,217` | `ui.le`, échange table → liste |
| `ui.c:222` | défauts de `ui` |
| `research.c:273` | défauts de `hp` |

**c) `pico8_class_t` comme classe déclarée.** Voir le tableau 2.

**d) `pico8_class_t` comme classe-valeur**, hors registres :

- `cart.c:190,223-229,244` : `gx[7]` = `qh`, `os`, `nr`, `fr`, `nl`, `mx`, `hj`, rangé comme clé `ke` du `ie` ;
- `research_icon.c:60-67` (`ie_self_ke_class`, qui asserte le tag `UNKNOWN`), `:97-111` (`pico8_ie_ke_view` lit `rr`/`s`/`me`/`od` par `pico8_class_get_field` sur la chaîne) et `:172-199` (`ie:ldown` instancie la classe) ;
- `planet.c:494-515` (`kv[hp.cw]`).

**e) `pico8_class_t` comme témoin d'init** (asserts ou gardes sur `… != NULL`) :

- asserts « init d'abord » : `score_panel.c:450`, `planet.c:249`, `probe.c:539`, `research_icon.c:145,180`, `help.c:194`, `planet_family_trade.c:324,409,584`, `planet_family.c:173,300,373,521,651,934`, `ui.c:176` ;
- asserts d'ordre entre parente et fille : `cart.c:59-61` (`->parent == pico8_ke_class()`), `cart.c:143,193`, `help.c:219`, `planet_family_trade.c:611`, `planet_family.c:968`, `mouse_reader.c:81`, `scroll.c:116` ;
- **gardes de comportement** :
  - `bootstrap.c:35-46` : `ke`/`qb`/`nf`/`ui` construits une seule fois, parce qu'un second `jl()` (appelé par `eu`) ne doit pas les reconstruire ;
  - `research_display.c:272` : `bj` rend NULL si `pico8_hp_class() == NULL`.

**f) `pico8_instance_t` comme type générique d'entité** : ~271 lignes dans 50 fichiers de `src/`, hors `class.c/h`. Aucun de ces sites ne lit `->fields` ou `->cls` d'une instance (grep) : ils ne font que typer et convertir. Les en-têtes publics concernés :

- `entity.h:43,44,64,80,88` (`lt`, `he`, `mv`, `mo`, `set_mo`) ;
- `entity_geom.h:16,18,23,25` ; `particle.h:24,30,35` ; `probe.h:96,97,109,119,125` (`fm`, `bv`, `dl`, `d_`) ; `ke.h:37` ;
- `msgbox.h:91` (`pz`) ; `slipway.h:69,79` ; `slipway_display.h:17` ; `ui.h:64,71,98` ;
- `economy_formulas.h:28` ; `economy_score.h:15,22` ; `economy_tiers.h:22` ;
- `planet_display.h:26` ; `planet_family_trade.h:80,93` ; `planet_trade.h:31` ; `research_display.h:36`.

Commentaires sous le grep : `value.h:31` et `type_tag.h:8`.

#### 2. Les `pico8_class_t` encore définies, et qui les crée

Il y en a 30, une par classe. Chacune est créée par `pico8_class_declare` dans l'init de son module. Toutes sauf `ke` reçoivent `pico8_class_set_struct_new`, donc `pico8_class_instantiate` délègue au constructeur de struct. Aucune ne construit plus d'instance à l'exécution (H9).

| Classe | Parente | Déclaration | `set_struct_new` | Accesseur public |
|---|---|---|---|---|
| `ke` | — | `ke.c:21` (+ `set_pq` `:23`, pour `test_pico8_entity_smoke` seul, 1a.31) | — | `ke.h:33` |
| `jo` | `ke` | `planet.c:291` | `:311` | `planet.h:71` |
| `qh`, `nl`, `os`, `hj`, `pm`, `ga` | `jo`, puis `qh`, `qh`, `qh`, `hj`, `jo` | `planet_family.c:972,991,1006,1021,1044,1069` | `:984,999,1014,1037,1060,1083` | `planet_family.h:127,140,148,156,177,186` |
| `nr`, `fr`, `mx` | `qh`, `nr`, `hj` | `planet_family_trade.c:616,633,647` | `:626,640,662` | `planet_family_trade.h:53,62,70` |
| `g_` | `ke` | `probe.c:569` | `:574` | `probe.h:65` |
| `fv` | `g_` | `help.c:229` | `:237` | `help.h:41` |
| `jc` | `ke` | `slipway.c:569` | `:578` | `slipway.h:46` |
| `mz` | `ke` | `wormhole.c:396` | `:400` | `wormhole.h:56` |
| `bg` | `ke` | `bg.c:126` | `:129` | `bg.h:36` |
| `js`, `fq` | `ke` | `ui_button.c:210,452` | `:212,454` | `ui_button.h:46,79` |
| `nf` | `ke` | `mouse_reader.c:82` | `:84` | `mouse_reader.h:34` |
| `qb` | `ke` | `scroll.c:117` | `:121` | `scroll.h:41` |
| `mh`, `iv` | `ke` | `msgbox.c:190,383` | `:192,385` | `msgbox.h:42,74` |
| `ui` | `ke` | `ui.c:207` | `:220` | `ui.h:48` |
| `ie` | `ke` | `research_icon.c:373` | `:375` | `research_icon.h:55` |
| `pj` | `ke` | `score_panel.c:502` | `:515` | `score_panel.h:46` |
| `trade`, `lq`, `jb`, `hy`, `hp` | — | `trade.c:77`, `layout.c:482,485`, `economy.c:124`, `research.c:266` | `:79`, `:484,487`, `:126`, `:270` | `trade.h:32`, `layout.h:43,44`, `economy.h:38`, `research.h:49` |

Ce que la classe apporte encore :

1. **La résolution des défauts de struct** :
   - `pico8_class_get_field` sur la chaîne remplit les tableaux `g_*_defaults` : `planet.c:314`, `probe.c:577`, `planet_family_trade.c:628,642,664`, `planet_family.c:986,1001,1016,1039,1062,1104` ;
   - s'y ajoutent les lectures directes de `->defaults` (famille b).

   Ces défauts sont des **REF dans la table parsée de la spec**, libérée par `pico8_class_free`. Plusieurs tests comparent leur identité (`class_ref(...)`).
2. **La classe-valeur** (famille d).
3. **Les registres** : `fz`, par `pico8_class_declare`, n'est lu que par `pico8_class_coverage_first_unknown` et `test_class_coverage` ; le registre nommé est la famille a.
4. **Le témoin d'init** (famille e).
5. **Le nom de couverture** (`PICO8_CLASS_COVERAGE_NAME`, 29 sites). Il ne sert que dans `pico8_class_instantiate` pour une classe sans `struct_new`, ce qui n'arrive jamais à l'exécution (H9).

Dans les tests (`grep -c`) :

- `pico8_class_get_field` : 43 appels dans 15 fichiers (`class_chain` 9, `mx` 5, `nr` 3, `nl` 3, puis `fv`, `fr`, `os`, `ga`, `pm`, `ui`, `g_`, `pj`, `jo`, `hj`, `qh` à 2) ;
- `pico8_class_instantiate` : 40 appels dans 20 fichiers (`entity_smoke` 15, `jc` 3, `class_coverage` 3, `ie` 2, `bg` 2, `trade`, `mx`, `jb_ol`, `mh`, `fq`, `ga`, `pm`, `mz`, `iv`, `ui`, `g_`, `nf`, `hj`, `qb`, `qh`) ;
- accesseurs `pico8_*_class()` : 29 fichiers, en tête `jo` 41, `qh` 25, `hj` 20, `g_` 20, `nr` 18, `mx` 17.

#### 3. `PICO8_TYPE_TAG_INSTANCE` et `pico8_class_set_struct_new`

`PICO8_TYPE_TAG_INSTANCE` dans `src/` :

| Site | Usage |
|---|---|
| `type_tag.h:40` | définition |
| `type_tag.h:60` | reconnu par `pico8_type_tag_of` |
| `class.c:275` | posé par `instantiate` |
| `class.h:40` | commentaire |
| `ref_audit.h:166` | case `n_instance`, relayé par `tools/ref_audit_report.sh:122,144`, hors suite |
| `score_panel.c:215`, `probe.c:234`, `entity_geom.c:103`, `entity.c:304`, `wormhole_display.c:49`, `slipway.c:211` | six asserts `INSTANCE \|\| STRUCT` |

Dans les tests :

- **empreintes** qui plient la valeur `0x9E5C1451u` : `test_pico8_js:245,278`, `test_pico8_os:186`, `test_pico8_g_:186`, `test_pico8_hj:198`, `test_pico8_qh:198` ;
- **instances faites à la main** : `test_pico8_js:1154` (`test_pz_closed`), `test_pico8_fv:662` (`test_g__closed`), `test_pico8_mo:100-101` (`gd`, la double forme laissée « pour 1a.42 » à 1a.28), `test_pico8_entity_smoke:805` ;
- **branches mortes** (le tag n'est plus jamais `INSTANCE`) : `test_pico8_ie:481,558,564`, `test_pico8_qh:544`, `test_pico8_ui:740` (`test_ke_closed`, qui instancie la classe `ke`).

`pico8_class_set_struct_new` : les 29 sites du tableau 2, plus `test_pico8_entity_smoke`.

#### 4. Les accesseurs à double forme `pico8_entity_get_field`/`set_field` (`entity.c:173-204`)

- la branche instance lit `pico8_instance_get_field` (`:177-178`) ou écrit dans `pico8_instance_fields(e)` (`:194-195`). Ce sont les seuls usages de ces deux fonctions hors de `class.c` ;
- la branche struct passe par le descripteur.

**Elles peuvent se réduire à la forme struct seule** : plus rien ne construit d'instance dans `src/` depuis 1a.41 (H4). Mais elles **ne disparaissent pas** : ce sont les lecteurs par nom que gardent `mv`, `lt`/`he`, `dc` et les lecteurs hétérogènes (`dl`, `pz`, `gd`…). Les remplacer par des lectures de membre est l'affaire de la phase 1b.

Même réduction pour les autres sites :

- les six asserts `INSTANCE || STRUCT` deviennent `STRUCT`. `entity_geom.c:103` garde sa tolérance à NULL ;
- `gd`, `ke_ks`, `dg` et `pz` sont déjà en forme struct seule, ou le deviennent par cette réduction.

#### 5. `Makefile`

`src/pico8_class.c` apparaît :

- `:138` (`ENGINE_SRCS`, donc `CORE_SRCS`, `float-grep`, `float-guard`, l'outil de couverture) ;
- `:207` (`DATA_ENGINE_SRCS`) ;
- `:325` (`test_class_coverage`) ;
- `:364` (`test_pico8_entity_smoke`, qui lie aussi `pico8_ke.c`) ;
- `:367` (`test_pico8_class_chain`) ;
- `:546` (`test_pico8_layout_nil_item_draws_nothing`, liste explicite) ;
- `:911-912` (`main_raylib`, prérequis et commande).

`UNIT_TESTS` : `:563` (`test_pico8_entity_smoke`), `:587` (`test_class_coverage`), `:616` (`test_pico8_class_chain`).

Le commentaire `:812` (« la struct de classe n'y a pas la même forme ») porte sur le membre `coverage_name` : il devient faux.

`pico8_class.h` est inclus par 39 en-têtes ou sources de `src/` et par 45 fichiers de `tests/` : `entity.h:4` le propage à presque tout.

#### 6. `tools/` et le filet de couverture

**Ce que mesure le filet.** `make class-coverage-net` (`Makefile:825-826`) lance `tools/class_coverage.sh --expect tools/class_coverage_net_expected.txt` sur `$(TMP_DIR)/pico8_class_coverage`, construit depuis `src/main_class_coverage.c` et `ENGINE_SRCS` en `-DPICO8_CLASS_COVERAGE`.

- Le compte vient de `pico8_class_coverage_record("<nom>")`, appelé **dans chaque constructeur de struct** : 29 sites, par exemple `ui_button.c:200-202`. Il ne vient plus de l'instanciation de classe.
- `main_class_coverage.c:82-94` imprime une ligne par nom de `KNOWN[]` (30 noms, `ke` compris). Il rend 3 si `pico8_class_coverage_first_unknown` trouve un nom compté hors `KNOWN[]`, ou un nom des registres `fz`/nommé hors `KNOWN[]` (`class.c:249-268`).
- Le compteur, `record`/`count`/`first_unknown`, **vit dans `pico8_class.c:178-270`**.

**Une fois les classes supprimées**, le filet mesure exactement ce qu'il mesure déjà : combien de fois chaque constructeur de struct tourne sur les 15 rejeux. Seul le contrôle « nom d'un registre hors `KNOWN[]` » perd son objet, puisqu'il n'y a plus de registre. Le plan le prévoyait (§4.2) :

> le contrat de l'outil est « chaque type d'entité est construit au moins une fois », indépendant de la représentation : en 1a, le crochet migre vers les constructeurs de struct, dans le commit de la classe concernée

Il demande aussi, pour la sortie de 1a, « `make class-coverage-net` toujours sans zéro », et « Même règle pour la relance en sortie de 1a » (§4.4).

**Décision : le filet n'est pas retiré, et `tools/class_coverage_net_expected.txt` ne change pas.**

- `ke 0 … exemptée` reste une ligne, parce que `ke` reste dans `KNOWN[]` et dans `tools/class_coverage_exempt.txt` (« abstraite, jamais instanciée directement », toujours vrai : aucun constructeur ne compte `ke`).
- Les 29 autres comptes sont produits par les mêmes appels `record`.
- Seul le compteur **déménage** hors de `pico8_class.c` (chunk e).
- Les noms `pico8_class_coverage_*`, `PICO8_CLASS_COVERAGE`, `pico8_class_coverage` (binaire), `class-coverage(-net)` et `tools/class_coverage*` peuvent rester : aucun ne tombe sous le grep de sortie. Les renommer n'est pas demandé.

Autres fichiers de `tools/` : aucune mention de `pico8_class`, sauf l'usage de `tools/class_coverage.sh:3` (« outil pico8_class_coverage »), qui reste vrai. `tools/ref_audit_report.sh` garde sa colonne `instance`, qui vaudra 0 (hors suite, rien à changer).

#### 7. Documentation

Deux documents ont un texte qui devient faux :

- `docs/glossaire-noms.md:16,17,20` : `kv` → `pico8_class_*`, `jr` → `pico8_class_declare`, `pq` → `pico8_class_set_pq` ;
- `docs/plan-3.md` : le §2 cite `pico8_class` comme module de la couche.

`docs/audit-ref-cast-reverse.md` et `docs/plan-2.md:675` sont historiques : ils ne changent pas.

## Solution

Cinq commits, dans cet ordre, chacun vert :

1. **1a.42a, les classes-valeurs deviennent des descripteurs de genre, et le registre nommé tombe.**
   - Un descripteur `static const` par classe utilisée comme valeur (les 7 de `gx[]`, plus `pm` et `ga` pour `kv[hp.cw]`) porte son constructeur (`pico8_X_new`) et un lecteur de défauts par nom.
   - `gx[]`, `ie.ke`, `ie:ldown`, `pico8_ie_ke_view` et `kv[hp.cw]` utilisent le descripteur. `jc`, `mz` et `g_` sont construits par appel direct de `pico8_jc_new`/`pico8_mz_new`/`pico8_g__new`.
   - Les 6 `pico8_class_register_named` sont retirés.
2. **1a.42b, les défauts sans classe hors de l'arbre `jo`.**
   - Les 19 classes concernées : `trade`, `lq`, `jb`, `hy`, `hp`, `pj`, `js`, `fq`, `nf`, `qb`, `mh`, `iv`, `ui`, `ie`, `bg`, `mz`, `jc`, `g_`, `fv`.
   - Chaque module garde la **table parsée de sa spec** (même appel `pico8_ob(spec, NULL)`, même moment de l'init, même libération au shutdown). Il résout ses défauts par une fonction de lecture explicite : sa spec, puis la fonction de sa parente.
   - Chaque témoin d'init devient un prédicat « prêt » du module, avec la même table de vérité.
   - L'accesseur `pico8_X_class()` et `pico8_class_set_struct_new` disparaissent.
3. **1a.42c, les défauts sans classe de l'arbre `jo`, plus `ke`.**
   - Même traitement pour `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, puis `ke`. `ke` garde sa spec parsée pour que `me`/`t` résolus par la chaîne restent la même REF.
   - Suppression de `test_pico8_entity_smoke` (§6, ligne du plan) : il dépend de la classe `ke`.
   - Après ce commit, plus aucune `pico8_class_t` n'est déclarée dans `src/`.
4. **1a.42d, `src/` en forme struct seule, puis retypage.** Deux temps dans un seul commit, ou deux commits d1 et d2 si l'implémenteur le préfère :
   - **d1, sémantique** : `pico8_entity_get_field`/`set_field` en forme struct seule ; les six asserts `INSTANCE || STRUCT` en `STRUCT` ; les instances faites à la main des tests remplacées ; branches mortes des tests retirées ; `PICO8_TYPE_TAG_INSTANCE` retiré de `src/` (les tests plient la constante littérale) ;
   - **d2, mécanique** : chaque `pico8_instance_t` de `src/` et `tests/` devient `pico8_struct_header_t`, const-ness conservée.
5. **1a.42e, suppression de `pico8_class.c/.h` et de `test_pico8_class_chain`.**
   - Le compteur de couverture déménage.
   - `test_class_coverage` est adapté.
   - `Makefile`, glossaire, §2 du plan et note de sortie.
   - Le grep de sortie est vide et `make class-coverage-net` est identique.

## User Stories

1. En tant qu'autopilote, je veux l'étape 1a.42 découpée en sous-étapes d'au plus ~40 fichiers chacune, afin de tenir dans un passage d'implémenteur et son budget de tests.
2. En tant qu'autopilote, je veux un ordre imposé entre les sous-étapes, afin que chaque commit compile et garde `make test` vert.
3. En tant qu'autopilote, je veux que chaque sous-étape reprenne l'inventaire de ce PRD avec ses numéros de ligne, afin de ne pas refaire la recherche.
4. En tant que gardien du filet, je veux que chaque sous-étape laisse tous les `*.sha256` identiques et `make frames-check` conforme (15 conformes), afin de respecter l'invariant du §3.
5. En tant que gardien de la couverture, je veux que `make class-coverage-net` rende, après chaque sous-étape, une sortie identique à `tools/class_coverage_net_expected.txt`, afin que la porte 0.4 tienne jusqu'à la sortie de phase.
6. En tant que gardien de la couverture, je veux que le filet survive à la suppression des classes, parce que son contrat porte sur les types d'entité et pas sur leur représentation (§4.2).
7. En tant que lecteur du cart, je veux que `gx[]`, `ie.ke` et `kv[hp.cw]` gardent un objet « classe-valeur » qui sait construire son entité et lire ses défauts, afin que `ie:ldown`, `ie:hb` et `jo:hu` restent des ports littéraux de `lua.lua:454` et du `gx` du cart.
8. En tant que joueur, je veux que les icônes de recherche affichent les mêmes `rr`/`s`/`me`/`od` et construisent la même planète, afin que l'écran technologique ne change pas.
9. En tant que joueur, je veux qu'une recherche à `cw` (protoétoile, éclat) transforme la planète comme aujourd'hui.
10. En tant que joueur, je veux que cliquer une planète ou un fond construise le même slipway, la même sonde ou le même trou de ver.
11. En tant que mainteneur, je veux que chaque struct garde **la même REF** de défaut qu'aujourd'hui (table parsée de sa spec), afin que les tests d'identité de défauts partagés restent verts sans changer d'intention.
12. En tant que mainteneur, je veux que la résolution d'un défaut suive la même chaîne qu'aujourd'hui (la spec propre, puis la parente, jusqu'à `ke`), afin qu'aucun défaut ne change de valeur.
13. En tant que mainteneur, je veux que chaque module remplace « sa classe n'est pas NULL » par un prédicat « prêt » à la même table de vérité, afin que `jl()` ne reconstruise toujours pas `ke`/`qb`/`nf`/`ui` et que `bj` rende toujours NULL avant l'init de `hp`.
14. En tant que mainteneur, je veux `pico8_entity_get_field`/`set_field` en forme struct seule, avec un assert porteur de message, afin qu'une REF de table ob ou de liste qui y arrive arrête le programme au lieu d'être lue comme une instance.
15. En tant que mainteneur, je veux un seul type générique d'entité dans les signatures (`pico8_struct_header_t`), afin que le retypage de la phase 1b parte d'un type réel et non d'une instance fantôme.
16. En tant que mainteneur, je veux `PICO8_TYPE_TAG_INSTANCE` retiré de `src/`, afin que `pico8_type_tag_of` ne reconnaisse plus une forme qui n'existe plus.
17. En tant que gardien des empreintes, je veux que les tests qui pliaient la valeur de ce tag plient la même constante littérale, afin que leurs `*_HASH` ne bougent pas.
18. En tant que gardien des tests de régression (§6), je veux que leur diff se limite à l'orthographe d'un type, afin qu'ils n'aient besoin ni de changer d'intention ni d'une preuve rouge.
19. En tant que gardien de `test_g__closed`, `test_pz_closed` et `test_ke_closed`, je veux qu'ils refusent toujours un objet qui n'est pas une struct (une table ob), afin que les fonctions fermées restent gardées.
20. En tant qu'auteur du §6, je veux `test_pico8_entity_smoke` et `test_pico8_class_chain` supprimés dans le commit qui supprime le code qu'ils testent.
21. En tant que vérificateur, je veux la liste des lignes de `pico8_entity.c` que seul `test_pico8_entity_smoke` exécutait (mesure `gcov`), afin de savoir ce que sa suppression découvre.
22. En tant que vérificateur, pour chaque sous-étape, je veux une caractérisation figée sur l'ancien code de ce que la sous-étape touche et que rien ne pinne, un rapport `gcov` des lignes ajoutées et 2 à 3 mutations, afin de juger la preuve sans banc propre.
23. En tant que vérificateur, je veux un tableau « Runs » par sous-étape (commande, état de l'arbre, sortie, ligne clé, durée), afin de ne relancer que ce qui suit le dernier changement.
24. En tant que lecteur du code, je veux que chaque commentaire ou contrat rendu faux soit corrigé, et que rien de nouveau ne raconte le chantier (règle des quatre destinations de CLAUDE.md).
25. En tant que lecteur du glossaire, je veux que `kv`/`jr`/`pq` pointent vers ce qui les porte après la suppression.
26. En tant que lecteur du plan, je veux la ligne 1a.42 remplacée par les lignes 1a.42a à 1a.42e, chacune avec sa note « Découvert à », et la sortie de phase constatée.
27. En tant qu'autopilote, je veux que toute condition d'arrêt du §7 soit un critère `FAILED`, afin de m'arrêter au bon endroit.
28. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé par un `TROU-N`, jamais corrigé.

## Implementation Decisions

Pas d'humain dans la boucle. Les décisions tranchées par le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) sont marquées (G).

- **(G) Découpage en cinq sous-étapes, dans l'ordre a → b → c → d → e.**
  - **a** précède **b/c** parce que la classe-valeur est le seul usage de `pico8_class_get_field` qui ne soit pas un remplissage de défauts. Une fois isolée derrière un descripteur, b/c peuvent changer l'implémentation du lecteur sans toucher aux appelants.
  - **b** (hors arbre `jo`) précède **c** (arbre `jo` et `ke`), parce que toute classe déclarée garde un pointeur `parent` vers `ke`. `ke` ne peut tomber qu'après la dernière fille.
  - Dans **c**, l'ordre interne va des feuilles vers la racine : `mx`, `fr`, `nr`, `ga`, `pm`, `hj`, `os`, `nl`, `qh`, `jo`, `ke`. Une fille privée de classe résout encore sa parente par la classe de celle-ci tant que la parente n'est pas traitée.
  - **d** vient après **c** : tant qu'une classe existe, `pico8_class_instantiate` peut rendre une instance.
  - **e** vient en dernier : `pico8_class.h` définit `pico8_instance_t`, que d2 retire.
- **(G) Le descripteur de genre (classe-valeur)** remplace une `pico8_class_t` partout où le cart manipule une classe comme valeur :
  - c'est un `static const` qui porte deux pointeurs : un constructeur de même signature que `pico8_X_new`, et un lecteur de défaut par nom qui rend 0 quand le nom est absent de la chaîne ;
  - son **premier membre n'est pas un tag magique** : `pico8_type_tag_of` le lit `UNKNOWN`, comme une classe aujourd'hui, et l'assert de `ie_self_ke_class` garde son sens ;
  - pas de parent, pas de registre, pas de `pq` ;
  - le type va dans `pico8_struct.h` ou à côté, avec un nom au choix, sans nom minifié inventé ;
  - dans **a**, le lecteur délègue encore à `pico8_class_get_field(g_X_class, …)` ; **b/c** le branchent sur la chaîne de défauts sans classe.

  Rejeté : garder un registre par nom renommé. Ce serait `lookup_named` sous un autre nom, et le cart indexe `kv` par `fz`, un ensemble fini connu (H2).
- **`kv[hp.cw]`** : une table `static const` de deux entrées `"pm"`/`"ga"` → descripteur, dans le module de `jo:hu`. Un `cw` inconnu donne NULL, donc `working_self` NULL, comme aujourd'hui (H2).
- **(G) `jc`, `mz`, `g_` par appel direct** de `pico8_jc_new`/`pico8_mz_new`/`pico8_g__new`.
  - Le registre nommé ne servait qu'à éviter l'inclusion d'un en-tête.
  - Tous les tests qui atteignent ces sites lient `ENGINE_SRCS` ou `DATA_ENGINE_SRCS` (inventaire du `Makefile` : 59 cibles). L'édition de liens ne change donc pas.
  - La branche « classe absente → nil » de `planet.c:651`, `planet_family_trade.c:121` et `bg.c:242` devient morte si H3 tient. Si H3 échoue, cette branche lit le prédicat « prêt » du module cible.
- **(G) Défauts sans classe (b, c).** Chaque module :
  - parse sa spec avec le même appel, au même point de son init, et garde la table (`static`), libérée à son shutdown là où `pico8_class_free` la libérait ;
  - expose, si une fille ou un descripteur en a besoin, une fonction « défaut par nom » : sa spec, puis la fonction de sa parente. C'est la chaîne de `pico8_class_get_field`, écrite à la main, sur les mêmes tables, donc avec les mêmes REF ;
  - remplit ses tableaux `g_*_defaults` par cette fonction, avec les mêmes asserts « trouvé » ;
  - fait lire aux 6 sites `->defaults` la table du module. Les échanges `ga.by` et `ui.le` (table → liste) se font sur cette table, au même moment.

  Pas de `fz` à inscrire : le registre `fz` n'a plus de lecteur hors couverture. Rejeté :
  - recopier les défauts en constantes C, qui relève de la phase D et rompt l'identité des REF ;
  - fusionner les spécifications, qui relève de la phase 2 (archétypes).
- **(G) Témoins d'init** : chaque `pico8_X_class() != NULL` (tableau e) devient un prédicat « prêt » du module, vrai de la fin de l'init au début du shutdown.
  - Les **gardes de comportement** (`bootstrap.c:35-46`, `research_display.c:272`) gardent exactement leur table de vérité.
  - Les asserts `->parent == pico8_ke_class()` de `cart.c:59-61` perdent leur objet : ils sont remplacés par l'assert que `ke` est prêt avant les trois inits.
  - Les messages des asserts ne changent pas s'ils restent vrais.
- **`ke`** (c) :
  - garde sa spec parsée (`me="ph"`, `t=0`) comme fin de chaîne des défauts, et son prédicat « prêt » pour `jl()` ;
  - `pico8_ke_class()`, le pont `pico8_ke_pq_lt` et `set_pq` disparaissent ;
  - `pico8_ke_struct_init` (valeurs en dur) reste tel quel. Si une empreinte montrait que la REF de chaîne de `me` compte (H5), ce serait à préserver, pas à corriger ;
  - le contrat de `ke.h:17-32` est réécrit.
- **(G) Type générique d'entité : `pico8_struct_header_t`**, const-ness conservée, en remplacement exact de `pico8_instance_t` dans les signatures, les variables et les conversions. Tout objet qui passe par ces sites commence par cet en-tête. Rejetés :
  - `void *`, qui perd le seul contrôle statique restant (motivation 1 du plan) ;
  - `pico8_ke_t *`, faux pour `trade`, `lq`, `jb`, `hy`, `hp` et les jetables sans parent (`et`, `{qs}`, `{ra,p}`, `g_ha_pz`…).

  `-Werror` interdit un mélange des deux types entre appelant et appelé : **d2 se fait d'un seul coup**, par substitution mécanique, puis la vérification `grep` et `make test`.
- **(G) Forme struct seule (d1)** : `pico8_entity_get_field`/`set_field` perdent la branche instance et assertent `STRUCT`, avec un message qui nomme le piège (une REF de table ob ou de liste passée comme entité). Les six asserts `INSTANCE || STRUCT` deviennent `STRUCT`, avec un message inchangé ou raccourci. Le contrat d'`entity.h:23-58` est corrigé.
- **`PICO8_TYPE_TAG_INSTANCE`** (d1) :
  - retiré de `type_tag.h` (définition, reconnaissance, commentaire `:8`) et de `ref_audit.h:166` ;
  - les tests qui pliaient sa valeur plient `0x9E5C1451u` sous un nom local, avec une ligne qui dit pourquoi (piège : sans elle, les `*_HASH` changent) ;
  - `tools/ref_audit_report.sh` ne change pas.
- **Tests « fermés »** (d1) : `test_g__closed`, `test_pz_closed` et `test_ke_closed` (ce dernier dès c, où la classe `ke` disparaît, avec une instance faite à la main jusqu'à d1) passent une table ob (`pico8_ob_table_new()`) au lieu d'une instance. Si un assert de fonction fermée ne refuse pas une table ob, c'est H10.
- **`test_pico8_mo` `test_gd_on_placeholder`** (d1) : les deux instances faites à la main deviennent deux structs locales au test (en-tête, membre `ra`, lecteur `ra`). `GD_NEAR`/`GD_FAR` ne changent pas.
- **Compteur de couverture** (e) :
  - `pico8_class_coverage_record`/`count`/`first_unknown` déménagent, sans la partie « registres », dans un petit module compilé avec le moteur, sous `PICO8_CLASS_COVERAGE` ;
  - ses déclarations vont dans un en-tête que les 29 constructeurs voient déjà (par exemple `pico8_struct.h`), pour ne pas toucher les 29 sites ;
  - `main_class_coverage.c` inclut ce nouvel en-tête ;
  - `KNOWN[]` et `tools/class_coverage_net_expected.txt` ne changent pas ;
  - le code de sortie 3 garde le cas « nom compté hors `KNOWN[]` », y compris `""`.

  Si un module vide hors `PICO8_CLASS_COVERAGE` pose un problème de compilation, c'est H11.
- **`test_class_coverage`** (e) : `test_coverage_hook_first_unknown_name` teste le compteur directement (`record` deux fois, `count`, `first_unknown` sur connu, inconnu et `""`). Les cas `register_named`/`fz`/`lookup("")` disparaissent avec les registres. Les tests du script (`:53-89`) ne changent pas.
- **Signatures et API conservées** : `pico8_X_new`, `pico8_X_struct_class()`, les descripteurs des jetables, `pico8_ha()`, `pico8_pj_cr()`, et tous les lecteurs par nom. Aucune entrée de `pico8_method_t` n'est ajoutée.
- **TROU** : aucun écart port/cart n'est connu sur ces chemins. `TROU-7` (constructeurs jamais libérés) reste à ses sites.
- **Commentaires** (CLAUDE.md) :
  - un contrat C sur le descripteur de genre (non libéré, lecteur rend 0 sur absent) et sur la fonction de défaut (REF empruntée à la spec du module, valable jusqu'à son shutdown) ;
  - les contrats rendus faux sont corrigés : `ke.h`, `entity.h`, `research_icon.h:13-18`, `planet.h:69`, `planet_family.h:175`, `wormhole.h:55`, `slipway.h:45`, `probe.h:64`, `ui.h:47`, `layout.h:41`, `mouse_reader.h:33`, `bg.h:35`, `trade.h:30`, `scroll.h:40`, `value.h:31`, `type_tag.h:8`, `Makefile:812` ;
  - tout le reste va dans les messages de commit.

### Hypothèses à vérifier à l'exécution (rien n'a été lancé pour ce PRD)

- **H1** (a) : pour les 7 classes de `gx[]` et pour `pm`/`ga`, le lecteur du descripteur rend pour `rr`, `s`, `me`, `od` exactement ce que rend `pico8_class_get_field` sur la classe (valeur et REF, absent compris : `qh`/`os` n'ont pas `rr`). À prouver par la caractérisation de a, écrite sur l'ancien code, puis gardée inchangée jusqu'à c.
- **H2** (a) : les `cw` des données de recherche ne valent que `"pm"` ou `"ga"`, les seuls `fz` du port. À vérifier en loggant `cw` pendant un `make e2e` dans une copie jetable, ou par lecture de `research.c` et du stash. Un autre nom resterait NULL, comme aujourd'hui.
- **H3** (a) : partout où `jo:ldown`, `nr:ldown` et `bg:ldown` sont atteints dans les tests et le filet, les modules `slipway`, `wormhole` et `probe` sont initialisés, donc la branche « classe absente » est morte. Vérifié par un assert temporaire sur cette branche pendant un `make test` et un `make e2e`, dans une copie jetable.
- **H4** (d1) : aucun appelant ne passe à l'exécution autre chose qu'une struct à `pico8_entity_get_field`/`set_field`. Vérifié par `make test` et `make e2e` verts avec l'assert.
- **H5** (b, c) : parser chaque spec hors de `pico8_class_declare`, au même point de l'init, donne les mêmes REF que les tests comparent (`class_ref`). Aucune empreinte ne plie un pointeur de classe ni l'ordre du registre `fz`.
- **H6** (b, c) : chaque tableau `g_*_defaults` rempli par la chaîne écrite à la main est identique, entrée par entrée (valeur et REF), à celui rempli par `pico8_class_get_field`. À prouver sur l'ancien code par une caractérisation qui compare, pour chaque classe du chunk, un struct neuf à la chaîne de classe.
- **H7** (b, c) : les gardes `bootstrap.c:35-46` restent vraies une seule fois par partie (second `jl()` de `eu`). Vérifié par un compteur temporaire d'inits dans une copie jetable, ou par un test qui lance deux `jl()` et compare la REF `ui.le`.
- **H8** (c) : les lignes de `pico8_entity.c` que seul `test_pico8_entity_smoke` exécutait sont mesurées par `gcov` (`make test` sans lui, contre avec lui) et listées dans le message de commit. On ne les reporte pas dans un autre test, sauf si l'une porte un `TROU`.
- **H9** (e) : aucun compte de couverture ne vient aujourd'hui de `pico8_class_instantiate` (toutes les classes sauf `ke` ont `struct_new`, et `ke` n'est jamais instanciée à l'exécution). Le déménagement du compteur ne change donc aucune ligne de `make class-coverage-net`.
- **H10** (c, d1) : les asserts des fonctions fermées (`g_`, `pz`, `ke_ks`, `dg`) refusent une table ob. Sinon le test fermé passe l'objet que l'assert refuse, sans affaiblir l'assert.
- **H11** (e) : le module de couverture compile sans avertissement hors `-DPICO8_CLASS_COVERAGE` (unité vide) sous `-std=c99 -Wall -Wextra -Werror`. Sinon, il n'est ajouté qu'à l'arbre de couverture.
- **H12** : chaque test modifié reste sous 30 s. Sinon il va dans `UNIT_TESTS_SLOW`. `test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx` et `nr` y sont déjà, donc ne tournent que par `make e2e`.
- **H13** (d2) : le diff des tests de régression (§6 ; parmi eux `game_end`, `slipway_drag`, `playability`, `rich_planet_exports`, `geoharvest_burst`, `tech_screen`, `empty_planet_cleared`, `planet_kj_dispatch`, `probe_mm_rnd_order`) se limite à l'orthographe du type. Toute autre ligne changée demande la preuve rouge du §6.
- **H14** : l'état 1a.41 du working tree est bien vérifié et commité avant 1a.42a. Si 1a.41 est rejetée, 1a.42 attend.

## Implementation Chunks

Chaque chunk est un commit et un passage d'implémenteur, avec son propre budget d'environ 20 minutes de tests et son propre tableau « Runs ». Les comptes de fichiers sont estimés par grep.

### 1a.42a : classes-valeurs et registre nommé (~12 `src/`, ~10 `tests/`)

- **Descripteur de genre** : son type, puis un descripteur par classe de `gx[]` (`qh`, `os`, `nl`, `hj` dans `planet_family`, `nr`, `fr`, `mx` dans `planet_family_trade`) et pour `pm`/`ga`, chacun avec son accesseur public.
- **Sites de code** : `cart.c:190-244` (`gx[]` de descripteurs), `research_icon.c:60-111,172-199` (`ie.ke`, la vue et `ie:ldown` par le descripteur), `planet.c:494-515` (table `cw`), `planet.c:647-658`, `planet_family_trade.c:106-127` et `bg.c:232-247` (appels directs).
- **Registre nommé** : les 6 `register_named` sont retirés. `pico8_class_lookup_named` n'est plus appelé que par `test_pico8_entity_smoke`/`test_class_coverage`, puis plus du tout.
- **Commentaires** : les 5 commentaires de famille a et `research_icon.h:13-18` sont corrigés.
- **Tests** :
  - `test_pico8_jc`, `mx`, `ga`, `pm`, `mz`, `g_`, `hj` : `lookup_named` + `instantiate` → `pico8_X_new` ;
  - `test_pico8_ie` et `test_pico8_qh:838` (`gx`, `ie.ke` : identité par descripteur).
- **Caractérisation** figée sur l'ancien code (H1) : pour les 9 descripteurs, les valeurs lues de `rr`/`s`/`me`/`od`, repliées dans une empreinte, et le genre du struct construit. Elle est gardée inchangée jusqu'à c.
- **Mutations** :
  1. le descripteur `gx[2]` pointe sur `fr` : `test_pico8_ie` rouge ;
  2. la table `cw` échange `pm` et `ga` : `test_pico8_pm`/`ga` rouge ;
  3. le lecteur ne sert plus `od` : caractérisation rouge.
- **Sortie** : `grep -rn lookup_named src/` ne rend que `pico8_class.c/.h`.
- **Note du plan** : « Découvert à 1a.42a ».

### 1a.42b : défauts sans classe hors de l'arbre `jo` (~17 `src/`, ~16 `tests/`)

- **Modules** : `trade`, `layout` (`lq`, `jb`), `economy` (`hy`), `research` (`hp`), `score_panel` (`pj`), `ui_button` (`js`, `fq`), `mouse_reader` (`nf`), `scroll` (`qb`), `msgbox` (`mh`, `iv`), `ui`, `research_icon` (`ie`), `bg`, `wormhole` (`mz`), `slipway` (`jc`), `probe` (`g_`), `help` (`fv`) :
  - spec gardée ;
  - fonction de défaut quand une fille (`fv` → `g_`) en a besoin ;
  - `->defaults` de `score_panel.c:518`, `help.c:239`, `ui.c:212-222` et `research.c:273` ;
  - prédicats « prêt » (famille e pour ces modules, dont `bootstrap.c:38-46` et `research_display.c:272`) ;
  - accesseurs `pico8_X_class()` retirés (`.c` et `.h`) ;
  - `PICO8_CLASS_COVERAGE_NAME`/`set_struct_new` retirés.
- **Classes restantes** : les parentes restent `pico8_ke_class()` jusqu'à c. Ces modules ne déclarent plus rien contre elle.
- **Makefile** : `pico8_class.c` sort de la liste explicite de `test_pico8_layout_nil_item_draws_nothing` (`:546`) si `layout.c` n'en dépend plus.
- **Tests** : `trade`, `lq`, `hy`, `jb_ol`, `mh`, `fq`, `iv`, `nf`, `qb`, `bg`, `hp`, `pj`, `ui` (`class_get_field`), `fv`, `g_`, `jc`. Ils lisent les défauts par la fonction de défaut ou un struct neuf, et construisent par `pico8_X_new`.
- **Caractérisation** figée sur l'ancien code (H6) : pour chaque classe du chunk, défauts d'un struct neuf égaux (valeur et REF) à `pico8_class_get_field` sur sa classe. La référence est prise avant, puis figée en empreinte.
- **Mutations** :
  1. la fonction de défaut de `fv` lit la parente avant la spec propre : caractérisation rouge ;
  2. le prédicat « prêt » de `ui` est toujours faux : un second `jl()` reconstruit, et le test de H7 est rouge ;
  3. l'échange `ui.le` est fait sur une copie : `test_pico8_ui` rouge.
- **Note du plan** : « Découvert à 1a.42b ».

### 1a.42c : défauts sans classe de l'arbre `jo`, `ke`, suppression de `test_pico8_entity_smoke` (~9 `src/`, ~14 `tests/`, `Makefile`)

- **Modules** : `planet` (`jo`), `planet_family` (`qh`, `nl`, `os`, `hj`, `pm`, `ga`, dont l'échange `ga.by` `:1087-1100`), `planet_family_trade` (`nr`, `fr`, `mx`), `ke`, `bootstrap.c:35-37`, `cart.c:55-61,193`.
  - Ordre interne : des feuilles vers la racine.
  - Les descripteurs de genre de a passent sur les fonctions de défaut.
- **Suppression** de `tests/test_pico8_entity_smoke.c`, de `Makefile:364-365` et de `:563`.
- **Tests** :
  - `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, `ie` : `pico8_X_class()`/`class_get_field` → fonction de défaut ou descripteur ;
  - `test_pico8_ui` `test_ke_closed` : instance faite à la main.
- **Caractérisations** : celle de a (H1) et une caractérisation H6 pour ces 10 classes, figées sur l'ancien code.
- **Mesure** `gcov` H8.
- **Mutations** :
  1. `qh` résout `me` sur `jo` avant sa spec : caractérisation rouge ;
  2. `pm` perd `ng` : caractérisation rouge, `test_pico8_pm` rouge ;
  3. l'échange `ga.by` est oublié : `test_pico8_ga`/`ga_export_panel_by` rouge (preuve rouge §6 si ce test de régression est touché).
- **Sortie** : `grep -rn 'pico8_class_t\|->defaults' src/` ne rend que `pico8_class.c/.h`, `entity.h` (inclusion) et les sites de d.
- **Note du plan** : « Découvert à 1a.42c ».

### 1a.42d : forme struct seule, puis retypage (d1 ~12 fichiers ; d2 mécanique, ~50 `src/` et ~19 `tests/`)

- **d1** :
  - `entity.c:173-204` (réduction et contrat `entity.h`) et les six asserts (inventaire 3) ;
  - `type_tag.h:8,40,60` et `ref_audit.h:166` ;
  - tests : `js:245,278,1154`, `os:186`, `g_:186`, `hj:198`, `qh:198,544-549`, `fv:662`, `mo:100-113`, `ie:481-483,558-565`, `ui:735-744`.
- **d2** : `pico8_instance_t` → `pico8_struct_header_t` dans `src/` et `tests/` ; `#include "pico8_class.h"` → `pico8_struct.h` là où il ne servait qu'au type. Aucun autre changement.
  - Vérification : `grep -rn pico8_instance_t src/ tests/` ne rend que `pico8_class.c/.h`, `test_pico8_class_chain` et `test_class_coverage`, puis `make test`.
  - H13 est vérifiée sur le diff.
- **Mutations** (d1) :
  1. `pico8_entity_get_field` sans assert : un `must_abort` ajouté (table ob passée à `mv`) devient vert, donc rouge pour le test ;
  2. le littéral de `test_pico8_js:245` passe à `0x9E5C1450u` : `PAUSE_HASH` rouge ;
  3. le lecteur local de `test_pico8_mo` sert `ra` décalé : `gd` rouge.
- d1 et d2 peuvent faire deux commits (1a.42d1, 1a.42d2) si l'implémenteur le préfère. Chacun est vert.
- **Note du plan** : « Découvert à 1a.42d ».

### 1a.42e : suppression de `pico8_class.c/.h`, sortie de phase (~6 `src/`, ~2 `tests/`, `Makefile`, docs)

- **Couverture** : module du compteur (`record`/`count`/`first_unknown` sans registres) et ses déclarations. `main_class_coverage.c` perd `pico8_class.h`.
- **Suppressions** : `src/pico8_class.c`, `src/pico8_class.h`, `tests/test_pico8_class_chain.c`.
- **`test_class_coverage`** est adapté (voir les décisions).
- **`Makefile`** :
  - `:138`, `:207`, `:325`, `:367-368`, `:546`, `:616` et `:911-912` : `pico8_class.c` retiré, et le module du compteur ajouté là où le compteur est lié (au moins `ENGINE_SRCS` pour l'outil de couverture, et `:325`) ;
  - `:812` : commentaire corrigé.
- **Docs** :
  - `docs/glossaire-noms.md:16,17,20` ;
  - `docs/plan-3.md` §2 (ligne `pico8_class`) ;
  - lignes 1a.42a à e (« Découvert à ») ;
  - constat de sortie de 1a.
- **Sortie** :
  - `grep -rn 'pico8_class_t\|pico8_instance_t\|->defaults\|lookup_named' src/` vide ;
  - `make class-coverage-net` identique à la référence, sans zéro hors `ke` exemptée, 15 rejeux lus.
- **Mutations** :
  1. `record` ne compte plus : `test_class_coverage` rouge ;
  2. `first_unknown` ignore `""` : `test_class_coverage` rouge.

## Testing Decisions

Un bon test lit ce que le jeu produit (champs par `pico8_entity_get_field`, écran, identité des REF que le cart partage, comptes de couverture), jamais la disposition mémoire. Seule la reconnaissance d'un objet passe par un accesseur de descripteur.

Prior art :

- caractérisations figées de 1a.17 à 1a.26 (`test_pico8_qh`, `jo`, `hj`, `pm`, `mx` : `class_value`/`class_ref` contre struct neuf) ;
- tests « fermés » : `test_g__closed`, `test_ke_closed`, `test_pz_closed` ;
- tests de forme de 1a.38 à 1a.41 ;
- `test_class_coverage` pour le compteur.

Règles communes à chaque sous-étape :

- **La caractérisation est écrite avant le changement**, vue verte sur l'ancien code, puis gardée inchangée : diff vide sur la fonction. Seuls les lecteurs partagés du test peuvent changer. La caractérisation de a reste jusqu'à c, et celles de b/c restent après leur commit.
- **Les tests existants ne changent que dans leur façon de lire** : `pico8_X_class()` → fonction de défaut, descripteur ou `pico8_X_new` ; type renommé ; instance faite à la main → table ob ou struct locale. Ils ne changent jamais dans leurs empreintes, leurs `#define …_HASH` ou leurs valeurs `check(...)` attendues.
- **Tests de régression du §6** : diff limité au type (H13) ou à la façon de lire. Si une autre ligne doit changer : preuve rouge (bug remis localement, test rouge, correctif restauré), notée dans le tableau « Runs ».
- **`gcov`** (§7, dans une copie jetable du scratchpad, supprimée ensuite ; `__gcov_dump` avant `_exit` par `-include`, précédents 1a.38 à 1a.41) : chaque ligne exécutable ajoutée ou modifiée est exécutée (descripteurs, fonctions de défaut, prédicats « prêt », réduction de `entity.c`, compteur). Une ligne à 0 est couverte par un test ajouté ou justifiée dans le message de commit.
- **Mutations** : 2 à 3 par sous-étape (listées par chunk), faites dans une copie jetable, restaurées, puis `cmp` contre l'arbre.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 minutes de temps total de tests par passage d'implémenteur**, donc par sous-étape. Ordres de grandeur (CLAUDE.md, 1a.39 à 1a.41) :

| Cible | Durée |
|---|---|
| `make test` | de 20 s à 1 min |
| `make e2e` | 4 à 6 min |
| `make class-coverage-net` | 3 à 4 min |

Répartition indicative par sous-étape :

- boucle sur les binaires de test touchés, lancés seuls ;
- caractérisation sur l'ancien code, puis mutations ;
- `gcov` une fois ;
- `make test` 2 à 3 fois ;
- `make e2e` une fois en fin ;
- `make class-coverage-net` une fois en fin.

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + « ancien code » / « chunk X appliqué » / « mutation N, copie jetable ») | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Exemples de ligne clé : `test_pico8_ie: all tests passed`, `frames-check : 15 conformes`, `couverture: OK -- … (15 rejeu(x))`, ou la ligne d'assert qui rougit.

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales de chaque sous-étape**, toutes vertes :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : `frames-check` rend **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make class-coverage-net` : sortie identique à `tools/class_coverage_net_expected.txt`. Ce fichier ne change pas ; le filet n'est ni retiré ni remplacé (inventaire 6).

En fin de 1a.42e, s'ajoute le grep de sortie, vide.

Jamais l'appli raylib. `make raylib` peut être compilé (sans l'exécuter) en e, parce que `:911-912` change, seulement si `third_party/raylib` est déjà en place ; sinon, la vérification est notée comme non faite.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

Conditions d'arrêt du §7 :

- le filet reste rouge après **3 tentatives** de correction sur le même commit ;
- le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS`/`RECORDINGS` ;
- un champ ne tombe sous aucune règle du §5 (par exemple, un défaut qu'aucune spec de la chaîne ne porte, ou un écrivain générique qui atteint un descripteur de genre) ;
- un test de régression de bug devrait changer **d'intention** pour passer ;
- la porte de couverture échoue : `make class-coverage-net` n'est pas identique à `tools/class_coverage_net_expected.txt`, un compte change, un nom est ajouté à `KNOWN[]` ou en est retiré, ou le nombre de rejeux lus n'est pas 15.

Critères propres à l'étape :

- une valeur figée ou une empreinte (`#define …_HASH`, `check(...)` attendu) est éditée, ou une caractérisation est modifiée après avoir été vue verte sur l'ancien code ;
- `tools/class_coverage_net_expected.txt` ou `tools/class_coverage_exempt.txt` est modifié ;
- un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N` ;
- une sous-étape est faite hors de l'ordre a → b → c → d → e, ou deux sous-étapes tiennent dans un même commit (d1 et d2 exceptés) ;
- un registre par nom, une chaîne `parent` générique ou une struct de classe renommée réapparaît (la suppression serait nominale) ;
- une garde de `bootstrap.c` ou de `research_display.c` change de table de vérité ;
- en fin de 1a.42e, le grep de sortie n'est pas vide, `pico8_class.c`/`.h` existent encore, ou `test_pico8_entity_smoke`/`test_pico8_class_chain` existent encore ;
- une note « Découvert à 1a.42x », un rapport `gcov`, les mutations ou le tableau « Runs » manquent.

## Out of Scope

- Remplacer les lectures par nom (`pico8_entity_get_field`) par des lectures de membre, typer les membres ou les REF : phase 1b.
- Les tables parsées (`ob()`) des spécifications et des menus comme données `static const` : phase D. Les specs restent parsées à l'init.
- Les archétypes et la fusion des chaînes de défauts : phase 2. Les fonctions de défaut chaînées sont un état de 1a.
- Renommer l'outil, la cible, le binaire, le drapeau ou les fichiers `class_coverage*`.
- `tools/ref_audit_report.sh` (colonne `instance` à 0).
- Libérer les structs (TROU-7, Plan 2 §8).
- Tout changement de rendu ou de comportement, et l'audio.

## Further Notes

- Numéros de ligne relevés sur HEAD b46e45b, plus le working tree de 1a.41. Rien n'a été compilé ni lancé pour ce PRD. Chaque sous-étape recale ses numéros.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque sous-étape contre ce PRD avant le commit (hors budget de tests).
- Le message de commit de chaque sous-étape porte :
  - le périmètre et les fichiers touchés ;
  - les décisions tranchées (G) et leurs rejets ;
  - le tableau « Runs » résumé ;
  - la caractérisation et sa preuve sur l'ancien code ;
  - le rapport `gcov` et les mutations tuées ;
  - les réponses aux hypothèses du chunk (H1 à H14) ;
  - la note « Découvert à 1a.42x ».
- Le message de 1a.42e constate la sortie de phase : grep vide, `class-coverage-net` identique, 15 conformes. C'est le point d'entrée de la phase 1b.
