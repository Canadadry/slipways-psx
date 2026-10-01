---
title: "Plan 3, étape 2.11 : étude C en lecture seule, les 17 paires d'inclusion mutuelle. Pour chaque paire, le sens coupé, la technique, l'étape de 2.14/2.15 qui la fait tomber d'elle-même s'il y en a une, et l'essai sur copie qui prouve qu'aucune coupe ne crée de paire nouvelle, ni au compteur ni dans le graphe d'usage ; liste ordonnée de 2.17 réécrite dans le plan. Deux sous-étapes, aucun code changé"
description: "L'étude qui fixe, avant 2.12, comment les paires de modules qui s'incluent l'un l'autre seront démêlées (décision de l'humain : toutes). Mesuré sur `ced58a6` (2.8 finie) : 17 paires / 43 lignes au compteur, 20 paires dans le graphe d'usage (3 cachées par des includes de détour), 14 des 34 lignes `#include` de `.c` retirables sans erreur de compilation (détour) ; un essai de coupe coûte ≈ 3 s (compilation `-fsyntax-only` de l'arbre + compteur), ≈ 10 s avec `make test` et `make e2e`. Produit `docs/etude-2-11-inclusions.md` et réécrit la ligne 2.17 du plan (annotations sur 2.14, 2.15, 2.18)."
status: needs-triage
---

## Problem Statement

**Écrit sur `ced58a6`** (branche `claude/plan-pilot-3-7zdft0`, 2.8c commise : 2.8 est finie). Commencé sur `25787e1` (2.8b), remesuré sur `ced58a6` : seuls des numéros de ligne de `ui.c`, `wormhole.c`, `wormhole.h` ont bougé, les 43 lignes et les symboles qu'elles portent sont les mêmes. Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les essais ont été faits sur une seule copie jetable (`git clone` local dans le scratchpad), supprimée depuis. **Les numéros de ligne sont ceux de `ced58a6`** ; 2.9 et 2.10 ne changent aucun code, mais l'agent retrouve chaque site par son texte si une étape s'est glissée avant.

Du point de vue du mainteneur du port : `make composition-residue` compte 17 paires de modules qui s'incluent l'un l'autre (catégorie `inclusion-mutuelle`, 43 lignes `#include` au pied). L'humain a tranché (Q2 de 2.0, 2026-09-30) : **on démêle**, toutes les paires ; la catégorie est une condition de sortie de la phase, à zéro, gardée. Une partie des paires naît de la couche objet (tests de famille suivis d'un cast, descentes `pico8_X_of`, comparaisons de classe) : 2.14 (accesseurs) et 2.15 (aplatissement, lien parent) en feront tomber certaines d'elles-mêmes. L'étape 2.17, qui coupe le reste, est **provisoire** dans le plan : ses sous-étapes et ses comptes attendent cette étude.

Le compteur ne voit que les lignes `#include` directes. Or, mesuré sur `ced58a6`, **14 des 34 lignes `#include` de `.c` qui réalisent une paire peuvent être supprimées sans aucune erreur de compilation** : l'en-tête arrive par un détour (par exemple `planet.c:21`, `pico8_probe.h`, qui revient par `planet_family.h:9`). Supprimer une telle ligne fait tomber la paire au compteur (essai : `planet` ↔ `probe` disparaît, pied 1073 → 1071) alors que `planet.c` appelle toujours `pico8_fm_hy`. Ce n'est pas une coupe. Inversement, trois paires de modules s'emploient l'un l'autre **sans inclusion directe** dans un sens, et le compteur ne les voit pas du tout. L'étude doit donc raisonner sur les **symboles employés**, pas sur les lignes.

### Texte du plan, recopié (`docs/plan-3.md` à `ced58a6`, ligne 2.11)

> - 2.11 — **étude C, lecture seule : les 17 paires d'inclusion.** Pour chaque paire : le sens coupé, la technique (déclaration déplacée, table de classe rangée avec ses méthodes d'affichage — `planet` ↔ `planet_display`, `slipway` ↔ `slipway_display`, `wormhole` ↔ `wormhole_display`, `score_panel` ↔ `score_panel_display`, `planet` ↔ `planet_trade` —, test de classe par le noyau…), l'étape de 2.14/2.15 qui la fait tomber d'elle-même s'il y en a une, et la preuve qu'aucune coupe ne crée de paire nouvelle (compteur relancé sur copie). Paires sans lien avec la couche à trancher aussi : `economy` ↔ `planet` (`planet.c:417` → `pico8_hy_nx`), `economy_formulas` ↔ `slipway` (`economy_formulas.c:133`, `slipway.c:140`), `entity_geom` ↔ `planet` (`planet.c:36`, `:471`), `planet` ↔ `slipway` (`planet.c:509`, `pico8_jc_new`). Sortie : `docs/prd/done/plan3-phase2-etape11-etude-inclusions.md`, liste ordonnée de 2.17. Compte : aucun. Budget ≈ 50 min.

Et la dépendance du plan qui fixe l'ordre (recopiée) :

> - **Paires d'inclusion de classe après la composition (2.17 après 2.14 et 2.15).** Les arêtes vers `planet` viennent du test de famille suivi d'un cast pour lire `p`/`ra`/`ma` (`slipway.c:56`, `:65`, `entity_geom.c:79`), celles vers `planet_family` et `probe` des descentes `pico8_X_of` (`planet.c:285`, `planet_display.c:429`, `:436`, `particle.c:19`), celles vers `slipway`, `wormhole`, `probe` des comparaisons de classe (`ui.c:333`, `probe.c:60`, `:98`, `:109`) : 2.14 et 2.15 changent ces lectures ; les couper avant ferait le travail deux fois. Les paires sans lien avec la couche (includes morts, 2.1 ; `entity` ↔ `ke`, 2.3a) passent en tête.

Les numéros de ligne cités par le plan datent de `ffd0a63` : `planet.c:417` est `:412` à `ced58a6`, `economy_formulas.c:133` et `slipway.c:139`, `planet.c:36` et `:466`, `planet.c:504`.

Décisions de l'humain qui cadrent l'étude : **Q2** (les paires sont démêlées, `inclusion-mutuelle` à zéro, gardée) ; **question A du 2026-09-30** (un seul type « planète » pour les dix sortes, un seul type pour la sonde et sa variante d'aide). A pèse ici : après 2.15, le type planète existe toujours, dans `planet.h` ; un module qui lit un champ de planète (`economy`, `entity_geom`, `slipway`, `probe`, `particle`) inclura toujours `planet.h`. L'aplatissement ne fait donc tomber une arête vers `planet` que si les lectures qui la portent passent par le noyau ou un composant.

### État de départ attendu — hypothèses

L'étude part de l'arbre d'après 2.10 (2.9 et 2.10 sont des études : aucun code ne change entre `ced58a6` et son départ). Chaque hypothèse est revérifiée en 2.11a ; si l'une tombe, l'étude le dit et refait les mesures touchées.

- **H-1** : `make composition-residue` rend `parente` 10, `lien-parent` 111, `accesseurs-q6` 237, `défauts` 669, `void-ptr` 8 / 4, `inclusion-mutuelle` **17 / 16**, les six autres à 0 et gardées, pied **`1073 / 85`**. `--check inclusion-mutuelle` rend **43 lignes**. Mesuré à `ced58a6`.
- **H-2.9** : `docs/etude-2-9-archetypes.md` existe, avec la **place** des archétypes et le **chemin de lecture** retenus (§2 de 2.9 : struct de classe par famille, pointeur d'archétype dans l'instance, ou accesseur `static`), et la ligne 2.12–2.13 du plan réécrite. L'étude C en dépend parce que la technique « table de classe rangée avec ses méthodes d'affichage » déplace la table de classe, que 2.9 peut avoir typée par famille ou fait porter l'archétype ; et parce qu'un archétype posé dans `game_data` ou le module de la classe ajoute des arêtes (2.9 a vérifié « aucune paire neuve » sur deux classes, `nl` et `bg`, pas sur toutes).
- **H-2.10** : `docs/etude-2-10-composants.md` existe, avec la liste ordonnée de 2.14, 2.15 et 2.16 réécrite dans le plan : pour chaque accesseur Q6, lecture d'archétype ou composant ; la forme de l'arbre `jo` et de `g_`/`fv` aplatis ; le sort de `pico8_X_of`, `parent()`, `emitter_of`/`probe_of` ; le typage des cibles `void *` ; l'extension du compteur aux registres `hv`, `cv`, `cg`, `ui.ic` (note « Découvert à 2.8c »). L'étude C lit ces décisions pour dire quelle arête tombe à quelle sous-étape. **Si l'un des deux livrables manque, l'étude s'arrête** : elle ne devine pas 2.14/2.15.

À revoir si une hypothèse tombe : « Mesures », le tableau de pré-classement, le tableau « Runs ».

## Solution

Une étude en deux sous-étapes, un commit de documentation chacune. Elle produit :

1. **`docs/etude-2-11-inclusions.md`**, le livrable (voir G1 pour le chemin), qui contient :
   - §1, le **graphe** : les 43 lignes du compteur et les arêtes d'usage (qui emploie quoi, symbole par symbole), les 17 paires du compteur et les paires d'usage hors compteur (3 à `ced58a6`) ;
   - §2, une **fiche par paire** : sens coupé, technique, sort de chaque symbole de l'arête coupée (tombe à 2.14x/2.15x selon 2.10, déplacé, réécrit), essai sur copie ;
   - §3, l'**essai cumulé** : toutes les coupes de 2.17 appliquées ensemble sur la copie, compteur et graphe d'usage relancés ;
   - §4, la **liste ordonnée de 2.17** : sous-étapes à ≈ 50 min, comptes d'arrivée, preuves ;
   - §5, les questions à l'humain, s'il y en a ;
   - en annexe, les scripts et le tableau « Runs ».
2. **`docs/plan-3.md`** mis à jour : ligne 2.17 réécrite ; 2.14 et 2.15 annotées des paires qu'elles font tomber (« (2.11 : fait tomber `particle` ↔ `probe`…) ») ; 2.18 annotée si l'étude étend la preuve de sortie (G4) ; note « Découvert à 2.11 » sur la ligne 2.11 ; ligne d'estimation mise à jour.

Aucun fichier de `src/`, `tests/`, `tools/` ou du `Makefile` ne change. `make composition-residue` rend le même pied avant et après.

## User Stories

1. En tant que pilote de 2.17, je veux, pour chaque paire qui reste, le sens coupé, la technique et le texte exact des déclarations qui bougent, pour couper sans relire le graphe.
2. En tant que pilote de 2.14 et 2.15, je veux savoir quelles paires mes sous-étapes font tomber, pour vérifier au commit que le compte `inclusion-mutuelle` baisse comme prévu, et ne pas couper une arête à la main qui tomberait seule.
3. En tant que vérificateur, je veux que chaque coupe soit une coupe d'usage (plus aucun symbole de l'autre module employé dans ce sens), pas une ligne retirée qui compile par un détour.
4. En tant que vérificateur, je veux la preuve sur copie qu'aucune coupe, seule ou avec les autres, ne crée de paire neuve, au compteur comme dans le graphe d'usage.
5. En tant qu'humain, je veux au plus quelques questions, en français simple, seulement là où aucun critère ne tranche.

## Implementation Decisions

### Ce qu'est une coupe (lecture de Q2, à écrire dans la note « Découvert à 2.11 »)

Une paire {A, B} est **démêlée** quand, dans un des deux sens, disons A → B, **aucun fichier du module A** (`A.c`, `A.h`) **n'emploie un nom que `B.h` déclare**, et que `A.c`/`A.h` n'incluent plus `B.h`. Le module est le nom de fichier sans extension, comme au compteur (`composition_residue.sh`, `read_inclusions`) : `.c` et `.h` d'un même nom forment un module, `tests/` est hors graphe.

Conséquences :
- retirer une ligne `#include` qui compile encore par un détour **n'est pas** une coupe (14 lignes sur 34 sont dans ce cas à `ced58a6`, liste en « Mesures ») ;
- une coupe qui laisse A employer un nom de B par un détour est un `FAILED:` ;
- un nom **déplacé** (déclaration qui change d'en-tête) compte pour le module qui le déclare après le déplacement.

### Le graphe d'usage (§1, exécuté à 2.11a)

Deux relevés sur la copie, tous deux en annexe A (texte exact essayé) :

- **arêtes du compteur** (`cross.py`) : pour chacune des 43 lignes de `--check inclusion-mutuelle`, les noms que l'en-tête inclus déclare et que le fichier inclueur emploie ;
- **graphe d'usage** (`usegraph.py`) : A → B quand un fichier de A emploie un nom que `B.h` est seul à déclarer ; paires mutuelles. Il retrouve les 17 paires du compteur et en ajoute 3 (voir « Mesures »).

Les deux scripts lisent les noms par motif (macros, prototypes, `typedef`, enums, `extern`) après retrait des commentaires et des chaînes ; ils ignorent un nom déclaré par deux en-têtes. Ce sont des relevés, pas des preuves : chaque arête coupée est **confirmée par le compilateur** (voir « Preuve d'une coupe »). L'étude corrige un faux positif ou un oubli à la lecture, et le dit.

### Fiche par paire (§2)

Pour chaque paire du compteur et chaque paire d'usage hors compteur :

1. **Les deux arêtes**, avec leurs symboles et leurs sites (fichier:ligne, fonction).
2. **Sort de chaque symbole**, d'après les livrables 2.9 et 2.10 et les lignes 2.12–2.16 réécrites : disparaît à une sous-étape nommée (par exemple `pico8_qh_of` à 2.15x, `get_ra` à 2.14x), reste, ou change de module. Une arête **tombe d'elle-même** à l'étape où son dernier symbole disparaît ; l'étude le nomme, et la paire sort de 2.17.
3. **Sens coupé** pour les paires qui restent, et la raison : le sens qui porte le moins de symboles n'est pas toujours le bon (exemple : couper `planet_family` → `planet`, 21 symboles, est exclu ; `planet` → `planet_family`, 4 symboles, dont `pico8_qh_of` que 2.15 retire).
4. **Technique**, dans le catalogue ci-dessous ou une autre justifiée ;
5. **Essai sur copie** (voir « Preuve d'une coupe »), avec le compte après ;
6. **Preuve pour 2.17** : refactor pur, ou corps identiques à l'adresse près, ou chemin touché (voir plus bas).

### Catalogue des techniques (point de départ, l'étude le complète)

- **T1 — dispatcher ou table de classe rangé avec l'affichage.** Mesuré sur `wormhole` ↔ `wormhole_display` : `wormhole.c` n'emploie de `wormhole_display.h` que `pico8_mz_kj`, appelé par le dispatcher `static pico8_mz_kj_dispatch` (`wormhole.c:190`) que la table `g_mz_struct_class` pointe. Le dispatcher déplacé dans `wormhole_display.c`, déclaré dans `wormhole.h`, `#include "pico8_wormhole_display.h"` retiré de `wormhole.c` : paire tombée, 43 → 40 lignes, pied 1073 → 1070 / 84, graphe d'usage 20 → 19, `make test` et `make e2e` (26) verts. Vaut aussi, a priori, pour `slipway` ↔ `slipway_display` (seul `pico8_jc_kj`). Pour `planet` ↔ `planet_display`, `planet.c` appelle aussi `pico8_jo_fe` directement (`planet.c:495`, dans le chemin de `ldown`) : le dispatcher ne suffit pas. Pour `planet` ↔ `planet_trade` (`pico8_jo_kf`, `pico8_jo_cn` derrière `pico8_jo_kf_dispatch`, `pico8_jo_cn_dispatch`, `planet.c:68-72`) et `score_panel` ↔ `score_panel_display` (`pico8_pj_hk`, `pico8_pj_ph`, `pico8_score_panel_display_init`/`_shutdown`), l'étude vérifie. Si 2.9 a changé la forme des tables de classe (struct par famille), la technique suit cette forme.
- **T2 — déclaration déplacée.** Un nom déclaré dans le mauvais module : `pico8_fm_hy` (le `hy` global) est déclaré dans `probe.h:128` ; c'est le seul nom de `probe.h` que `planet.c` emploie (`planet.c:412`), et `economy_score.c`, `tech_gate.c` l'emploient avec `pico8_hy_t` de `economy.h` qu'ils n'incluent pas. Le déplacer (vers `economy`, où vit `pico8_hy_t`, ou ailleurs) touche `planet` ↔ `probe`, `probe` ↔ `slipway` (`slipway.c:195`) et deux paires cachées. L'étude vérifie que le déplacement ne crée pas `economy` ↔ `probe` ou autre.
- **T3 — signature qui prend des valeurs au lieu d'entités.** Mesuré sur `economy_formulas` ↔ `slipway` : `pico8_dj` prend deux `pico8_vec_t` au lieu de deux entités, l'appelant `slipway.c:139` passe `pico8_jc_end_p(self->qu)`, `pico8_jc_end_p(self->re)` ; `economy_formulas.c` n'inclut plus `slipway.h`. Paire tombée, pied −2 lignes, `make test` et `make e2e` (26) verts. L'ordre d'évaluation des deux `pico8_jc_end_p` était et reste non spécifié (deux arguments d'un même appel) : l'étude vérifie qu'ils sont sans effet de bord.
- **T4 — test de classe par le noyau.** Les comparaisons `cls == pico8_X_struct_class()` (`ui.c:333` pour `jc`, `g_`, `mz` ; `probe.c:60`, `:98`, `:108-129`, `:215-218`, `:299`, `:305`) remplacées par une question au noyau : étiquette du masque `tags` (2.7 : l'enum a `jc`, `mz`, `g_`, `jo`…), ou un membre de la table de classe. L'étude vérifie que la question est **équivalente** (une étiquette peut être portée par plusieurs classes : `jc` est-il seul à porter l'étiquette `jc` ? `g_` et `fv` ?) ; sinon elle nomme un autre moyen.
- **T5 — arête qui tombe avec 2.14/2.15.** Descentes `pico8_qh_of`, `pico8_nl_of` (`planet_display.c`), `pico8_g__of` (`particle.c:19`), tests de famille suivis d'un cast pour lire `ra`, `ma`, `p`, `fa` (`entity_geom.c:61-62`, `:77-79`, `slipway.c:55`, `:64`, `particle.c:25-29`). À trancher selon 2.10 : si `ra`/`ma`/`p`/`fa` deviennent un composant lu par le noyau, l'arête vers `planet` tombe ; si elles restent des champs du type planète (décision A), l'arête reste et la paire se coupe dans l'autre sens.

### Pièges mesurés (chacun a produit un faux résultat pendant les essais)

1. **Le détour.** Une ligne retirée qui compile encore n'est pas une coupe (voir plus haut). 14 lignes de `.c` sur 34, liste en « Mesures ». Le contrôle est le graphe d'usage, pas la compilation.
2. **Le module fusionne `.c` et `.h`.** Couper `A.c` → `B.h` ne suffit pas si `A.h` inclut `B.h` (exemples : `probe.h:9` → `particle.h`, `slipway.h:7` → `probe.h`, `wormhole.h:11` → `probe.h`, les trois `*_display.h` → leur module).
3. **Les types de planète sont des `typedef struct { … }` sans étiquette** (`planet.h:26-65`) : une déclaration anticipée demande d'abord une étiquette. Essai : `typedef struct pico8_jo {` ne change ni le compteur (1073 / 85, `parente` 10) ni `--champs` (282 / 15 / 27).
4. **Pas de `typedef` répété.** `typedef struct a a_t;` deux fois passe sous `gcc -std=c99 -Wall -Wextra -Werror` (13.3) mais **échoue sous `clang`** (`-Wtypedef-redefinition`, « a C11 feature ») ; `CC ?= cc` et la cible PSX sont d'autres compilateurs. Une déclaration anticipée passe par `struct pico8_X *` dans le prototype, ou par la liste de `struct.h:99-104` (`struct pico8_qy;` …), comme 2.2.
5. **Déplacer une définition change les objets.** Essai T1 : le dispatcher déplacé passe de `t` à `T` dans `nm`, et son corps désassemblé est identique **à l'adresse près** (seules les cibles de `je`/`jmp` relatives au début de la fonction diffèrent). « Objets sans `-g` identiques » ne vaut donc que pour une déclaration seule ; pour un corps déplacé, la preuve est « corps identiques à l'adresse près, `nm` ne diffère que par la liaison du symbole déplacé » ; pour une signature changée (T3), chemin touché.
6. **Le compteur ne voit que `src/`.** Une coupe qui obligerait un TU à inclure un en-tête de plus est permise ; elle est notée.

### Preuve d'une coupe, et « aucune paire nouvelle » (§2 et §3)

- **Copie** : une seule copie jetable de l'arbre de départ dans le scratchpad (`git clone` local), jamais l'arbre de travail : un implémenteur peut y travailler en parallèle. Supprimée à la fin de 2.11b ; journaux dans le scratchpad.
- **Chaque coupe, seule** (2.11a) : sur la copie remise à l'état de départ, appliquer la coupe, puis (1) compiler tous les `.c` de `src/` et `tests/` en `cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only` (les deux `class_coverage` sous `-DPICO8_CLASS_COVERAGE`, les trois fichiers raylib exclus comme en 2.9), (2) relancer `sh tools/composition_residue.sh --detail` et comparer la liste des paires à celle du départ, (3) relancer `usegraph.py`. Critère : la paire visée sort des deux listes, **aucune paire n'y entre**, aucune catégorie ne monte (en particulier `void-ptr` : un dispatcher déplacé garde son nom en `_dispatch`). Coût mesuré : ≈ 3 s.
- **Coupes qui dépendent de 2.14/2.15** : l'étude ne simule pas l'aplatissement. La preuve est la table des symboles de l'arête : chaque symbole a son étape de disparition, lue dans le livrable 2.10. Elle est rejouée par 2.17a au départ (la note le dit).
- **Essai cumulé** (2.11b, §3) : toutes les coupes que 2.17 fera sans attendre 2.14/2.15, appliquées ensemble sur la copie. Critères : compilation propre, compteur à `17 − n` paires (n = coupes appliquées), graphe d'usage à `20 − n` moins les paires cachées coupées, aucune paire nouvelle, `make test` et `make e2e` (26 conformes) verts sur la copie (≈ 10 s). Le diff cumulé est versé en annexe de l'étude, pour 2.17.

### Liste ordonnée de 2.17 (§4 et plan)

- Sous-étapes à ≈ 50 min, regroupées par technique ou par module, sur le modèle des lignes 2.4 à 2.8 : modules, compte `inclusion-mutuelle` d'arrivée (paires et lignes, au compteur) et paires d'usage d'arrivée, preuves (piège 5), `class-coverage-net` (oui si une table de classe ou un constructeur bouge), `sanitize-net`, budget.
- **Ordre** : une coupe dont aucun symbole n'est touché par 2.12 à 2.16 (T1, T2, T3 à `ced58a6`) peut être placée en tête de 2.17 ou avant 2.12 ; l'étude choisit, et le plan le dit. Les coupes qui dépendent de 2.14/2.15 restent après.
- **Paires cachées** (hors compteur) : décision G3 ci-dessous.
- **Garde** : 2.17 ajoute `inclusion-mutuelle` à `COMPOSITION_RESIDUE_GUARD`. Si l'étude recommande que la garde voie aussi l'usage (G4), elle chiffre le travail dans `tools/` et le place dans 2.17 ou 2.18.

### Mesures faites pour ce PRD (sur `ced58a6`)

Pré-classement **automatique** par les deux scripts de l'annexe A et une lecture rapide des sites. Il chiffre le travail ; il ne remplace pas la lecture de l'étude.

| # | Paire (lignes au compteur) | A → B : symboles | B → A : symboles | Pré-classement |
|---|---|---|---|---|
| 1 | `economy` ↔ `planet` (2) | `economy.c:221-224` : `pico8_jo_is_family`, `pico8_jo_t` (hy:lj, cast pour lire la planète) | `planet.c:412` : `pico8_hy_nx` | sans lien avec la couche ; même instruction que la paire 8 (`pico8_hy_nx(pico8_fm_hy(), fp)`) |
| 2 | `economy_formulas` ↔ `slipway` (2) | `economy_formulas.c:133` : `pico8_jc_end_p` | `slipway.c:139` : `pico8_dj` | **T3, essayé** : −1 paire, −2 lignes |
| 3 | `entity_geom` ↔ `planet` (2) | `entity_geom.c:61-62`, `:77-79` : `pico8_jo_is_family`, `pico8_jo_t` (`ra`, `ma`) | `planet.c:36`, `:466` : `pico8_gk`, `pico8_gd` | T5 selon 2.10 (`ra`, `ma` : composant ?) ; même site que la paire cachée `entity_geom` ↔ `scroll` |
| 4 | `particle` ↔ `probe` (3) | `particle.c:19` : `pico8_g__of`, `pico8_g__t` (`fa`, `ra`) | `probe.c`, `probe.h:9` : `pico8_emitter_t`, `pico8_qy_t`, `pico8_f_`, `pico8_ei_dispatch` | T5 : `pico8_g__of` part à 2.15, `fa`/`ra` à 2.14 ? |
| 5 | `planet` ↔ `planet_display` (3) | `planet.c:40-54`, `:495` : `pico8_jo_hb`, `_gv`, `_kj`, `_fe` | `planet_display.c`, `.h` : `pico8_jo_cq`, `_dz`, `_gg`, `_hu`, `_ka`, `pico8_jo_t` | T1, plus l'appel direct `pico8_jo_fe` |
| 6 | `planet` ↔ `planet_family` (3) | `planet.c:269-280`, `:362-363` : `pico8_qh_of`, `pico8_qh_t`, `pico8_pm_kind`, `pico8_ga_kind` | 21 symboles (dispatchers, `get_*`, `pq`, table…) | T5 (`pico8_qh_of`, 2.15) + registre des sortes déplacé ; décision A peut fondre les deux modules |
| 7 | `planet` ↔ `planet_trade` (3) | `planet.c:68-72` : `pico8_jo_kf`, `pico8_jo_cn` | `planet_trade.c`, `.h` : `pico8_jo_is_family`, `pico8_jo_kd`, `pico8_jo_t` | T1 |
| 8 | `planet` ↔ `probe` (2) | `planet.c:412` : `pico8_fm_hy` | `probe.c:84-117`, `:284-294` : `pico8_jo_is_family`, `pico8_jo_struct_class`, `pico8_jo_t` | **T2** (`pico8_fm_hy` déplacé) ; `planet.c:21` est un détour |
| 9 | `planet` ↔ `slipway` (2) | `planet.c:500-504` : `pico8_jc_keys_t`, `pico8_jc_new`, `pico8_jc_ready` | `slipway.c:55`, `:64`, `:267-268` : `pico8_jo_is_family`, `pico8_jo_struct_class`, `pico8_jo_t` (`p`, `ra`, `oe`) | T5 pour `p`/`ra` ; `oe` reste un champ de planète |
| 10 | `planet_display` ↔ `planet_family` (2) | `planet_display.c` : `pico8_qh_of`, `pico8_nl_of`, `pico8_qh_t`, `pico8_nl_t` | `planet_family.c:17` : `pico8_jo_kj` | T5 (2.15) ou T1 |
| 11 | `probe` ↔ `slipway` (3) | `probe.c` : `pico8_jc_end_p`, `pico8_jc_struct_class`, `pico8_jc_t`, `pico8_jc_too_far_*` | `slipway.c`, `slipway.h:7` : `pico8_probe_t`, `pico8_d_`, `pico8_dl`, `pico8_fm_hy`, `pico8_probe_et_hang_fo` | T4 + T2 ; la plus chargée |
| 12 | `probe` ↔ `ui` (2) | `probe.c:72`, `:243` : `pico8_ui_hw`, `pico8_je_nf` | `ui.c:333` : `pico8_g__struct_class` | T4 |
| 13 | `probe` ↔ `wormhole` (3) | `probe.c:34`, `:98`, `:122`, `:215`, `:305` : `pico8_mz_struct_class`, `pico8_mz_jj_*`, `pico8_mz_t` | `wormhole.c`, `wormhole.h:11` : `pico8_probe_t`, `pico8_bv`, `pico8_d_`, `pico8_dl_dispatch` | T4 |
| 14 | `score_panel` ↔ `score_panel_display` (3) | `score_panel.c` : `pico8_pj_hk`, `pico8_pj_ph`, `pico8_score_panel_display_init`/`_shutdown` | 8 symboles | T1 (init/shutdown en plus) |
| 15 | `slipway` ↔ `slipway_display` (3) | `slipway.c` : `pico8_jc_kj` | `pico8_jc_end_ra`, `pico8_jc_hg_has`, `pico8_jc_t` | T1 (comme la paire 17) |
| 16 | `slipway` ↔ `ui` (2) | `slipway.c:116-123` : `pico8_ok`, `pico8_ui_t` | `ui.c:333` : `pico8_jc_struct_class` | T4 |
| 17 | `wormhole` ↔ `wormhole_display` (3) | `wormhole.c:190-192` : `pico8_mz_kj` | `pico8_mz_ej`, `pico8_mz_t` | **T1, essayé** : −1 paire, −3 lignes |
| c1 | `economy` ↔ `economy_score` (0) | `pico8_bf`, `_cz`, `_da`, `_db` | `economy_score.c:189`, `:232` : `pico8_hy_t` (sans inclure `economy.h`) | cachée ; T2 (`pico8_fm_hy`) |
| c2 | `economy` ↔ `tech_gate` (0) | `pico8_gi_nb`, `pico8_ov` | `tech_gate.c:16`, `:38` : `pico8_hy_t` | cachée ; T2 |
| c3 | `entity_geom` ↔ `scroll` (0) | `entity_geom.c:49`, `:77-79` : `pico8_qb_t`, `pico8_qb_struct_class` (par `entity.h` → `scroll.h`) | `scroll.c` : `pico8_ma` | cachée ; liée à la paire 3 (`ma` partagé planète/`qb`) |

Lignes retirables sans erreur (détours, piège 1) : `economy.c:15`, `planet.c:11`, `:21`, `planet_display.c:17`, `planet_family.c:16`, `planet_trade.c:10`, `probe.c:18`, `score_panel_display.c:14`, `slipway.c:11`, `:17`, `slipway_display.c:10`, `ui.c:13`, `wormhole.c:13`, `wormhole_display.c:9` (14 sur 34 lignes de `.c` ; les 9 lignes de `.h` non essayées).

Lecture du pré-classement : 3 paires ont une coupe essayée ou évidente sans lien avec la couche (2, 17, et 15 par analogie) ; T2 sur `pico8_fm_hy` touche 2 paires du compteur (8, et une arête de 11) et les 2 premières cachées ; 5 à 6 paires sont candidates à tomber avec 2.14/2.15 (3, 4, 6, 9, 10, c3), à confirmer par 2.10 ; le reste relève de T1 (5, 7, 14) et T4 (11, 12, 13, 16). Le plan provisoire donne ≈ 5 sous-étapes à 2.17 ; ce pré-classement est cohérent avec 4 à 6.

## Implementation Chunks

Deux sous-étapes, un commit chacune, ≈ 50 min. Le plan donnait ≈ 50 min pour toute l'étude : **écart déclaré**, voir « Surprises ».

1. **2.11a — graphe, fiches, essais seuls (§1, §2).** Revérifier H-1, H-2.9, H-2.10. Monter la copie, lancer `cross.py` et `usegraph.py` (annexe A), corriger à la lecture. Pour chaque paire (17 + cachées) : fiche complète ; sort des symboles d'après 2.10 ; pour chaque coupe que 2.17 fera, l'essai seul sur copie (≈ 3 s chacun). Commit : `docs(etude): 2.11a …`.
2. **2.11b — essai cumulé, liste de 2.17, plan (§3, §4, §5).** Essai cumulé avec `make test` et `make e2e` sur la copie ; diff cumulé en annexe ; liste ordonnée de 2.17 ; réécriture de la ligne 2.17, incises sur 2.14, 2.15 (paires qu'elles font tomber, avec la sous-étape) et 2.18 (G4), note « Découvert à 2.11 », estimation ; questions à l'humain ; tableau « Runs » ; copie supprimée. Commit : `docs(plan): 2.11b …`.

**Repli** (si une sous-étape dépasse 1,5 fois son budget avant son commit) : 2.11a en deux, **2.11a1** (graphe et fiches des paires sans lien avec 2.14/2.15, avec leurs essais) et **2.11a2** (fiches des paires qui dépendent de 2.10). Le repli est écrit dans la note « Découvert à 2.11 ».

## Testing Decisions

Aucun code ne change, donc ni `gcov` ni mutation. Les preuves portent sur la complétude du graphe et sur les essais.

### Contrôles de complétude (écrits dans l'étude, avec leur sortie)

1. **Lignes** : chacune des lignes de `--check inclusion-mutuelle` (43, H-1) apparaît dans une fiche, avec ses symboles. `comm -3` entre la liste du compteur et celle des fiches → **vide**.
2. **Paires** : les paires de `--detail` (17) et celles de `usegraph.py` (20 à `ced58a6`) ont chacune une fiche ; une paire d'usage absente du compteur est une paire cachée (G3).
3. **Symboles** : chaque symbole d'une arête coupée a un sort (disparaît à 2.1Nx, déplacé vers tel module, remplacé par tel appel). Aucune arête n'est déclarée coupée tant qu'un de ses symboles reste employé.
4. **Tombées d'elles-mêmes** : chaque paire rangée en T5 nomme la sous-étape de 2.14/2.15 qui retire son dernier symbole, telle que la ligne réécrite par 2.10 la nomme. Si la ligne ne la nomme pas, la paire reste dans 2.17.
5. **Essais seuls** : chaque coupe de 2.17 a son essai sur copie, avec compilation propre, et la liste des paires du compteur et de l'usage avant/après ; aucune paire entrante.
6. **Essai cumulé** : toutes les coupes de 2.17 qui n'attendent pas 2.14/2.15, ensemble : compilation propre, aucune paire entrante, `make test` vert, `make e2e` 26 conformes, sur la copie.
7. **Arbre réel intact** : `git diff --stat <parent>` ne montre que `docs/etude-2-11-inclusions.md` et, à 2.11b, `docs/plan-3.md`. `make composition-residue` rend le même pied qu'au départ (1073 / 85).

### Budget et tableau « Runs »

Budget : **2 × ≈ 50 min**. Au-delà de 1,5 fois, repli. Jamais l'appli raylib ; `make raylib` n'est pas exigé. `timeout` sur toute commande longue (`make test`, `make e2e` : `timeout 900`). Pour attendre un processus de fond, attendre son PID, jamais `pgrep -f`.

L'agent de l'étude tient un tableau « Runs » en annexe de l'étude. Voici celui des essais de ce PRD, sur une copie de `25787e1` (lignes 1 à 8) puis de `ced58a6` (lignes 9 à 12) :

| # | Commande | Code | Ligne clé | Durée |
|---|---|---|---|---|
| 1 | `sh tools/composition_residue.sh` (`25787e1`, 2.8b) | 0 | `inclusion-mutuelle 17 16` ; `résidu 2 : 1077 ligne(s), 87 fichier(s)` | 0,7 s |
| 2 | `--check inclusion-mutuelle` ; `cross.py` | 0 | 43 lignes ; symboles par ligne (table « Mesures ») | < 1 s |
| 3 | T3 sur `pico8_dj` ; `-fsyntax-only` de `src/` et `tests/` ; compteur | 0 | aucune erreur ; `inclusion-mutuelle 16 15` ; pied 1075 / 86 | 3,6 s |
| 4 | `make -j4 test` puis `make e2e` sur l'essai 3 | 0 | `test_composition_residue: all tests passed` ; `e2e: 26 e2e` | 8,8 s + 0,8 s |
| 5 | `planet.c:21` (`pico8_probe.h`) supprimée seule | 0 | compile ; `inclusion-mutuelle 16 16`, pied 1075 / 87 : **détour** (`planet_family.h:9` → `probe.h`) | < 1 s |
| 6 | chaque ligne de `.c` des 43 supprimée seule, `-fsyntax-only` du fichier | 0 | 14 sur 34 compilent (liste en « Mesures ») | 5 s |
| 7 | `usegraph.py` | 0 | `paires 20` : les 17 + `economy` ↔ `economy_score`, `economy` ↔ `tech_gate`, `entity_geom` ↔ `scroll` | < 1 s |
| 8 | `git pull` → `ced58a6` (2.8c) ; compteur ; `cross.py`, `usegraph.py` | 0 | pied **1073 / 85**, `inclusion-mutuelle 17 16`, `void-ptr 8 4` ; mêmes symboles, lignes `ui.c`, `wormhole.c`, `wormhole.h` décalées de 1 ; `paires 20` | < 2 s |
| 9 | T1 sur `pico8_mz_kj_dispatch` ; `-fsyntax-only` ; compteur ; `usegraph.py` | 0 | aucune erreur ; `inclusion-mutuelle 16 15` ; pied 1070 / 84 ; `paires 19` | 3,1 s |
| 10 | objets sans `-g` de `wormhole`, `wormhole_display` avant/après ; `make -j4 test` ; `make e2e` | 0 | corps du dispatcher identique à l'adresse près ; `nm` : `t` → `T` ; tests verts ; `e2e: 26 e2e` | ≈ 10 s |
| 11 | étiquette `pico8_jo` ajoutée à `planet.h:26` ; compteur ; `--champs` | 0 | inchangés : 1073 / 85, `parente 10 3` ; `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27` | 1 s |
| 12 | `typedef struct a a_t;` répété, `gcc` puis `clang` en `-std=c99 -Wall -Wextra -Werror` | 0 / 1 | gcc 13.3 l'accepte ; clang : `redefinition of typedef 'a_t' is a C11 feature` | < 1 s |

Chaque essai a été remis à zéro (`git checkout -- .`) avant le suivant ; `git status --short` de la copie vide à la fin.

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas vérifié (H-1 : pied 1073 / 85, `inclusion-mutuelle` 17 / 16, 43 lignes) et que l'étude ne l'explique pas ; ou si `docs/etude-2-9-archetypes.md` ou `docs/etude-2-10-composants.md` manque et que l'étude n'est pas arrêtée.
- `FAILED:` si un fichier hors de `docs/etude-2-11-inclusions.md` et `docs/plan-3.md` change (le déplacement de ce PRD par le pilote excepté), ou si un essai a été fait dans l'arbre de travail plutôt que dans la copie.
- `FAILED:` si le contrôle 1 ou 2 n'est pas vide (une ligne ou une paire sans fiche).
- `FAILED:` si une coupe est une ligne retirée qui compile par un détour : un symbole de l'autre module reste employé dans le sens coupé (contrôle 3, piège 1).
- `FAILED:` si une paire est rangée « tombe à 2.14/2.15 » sans la sous-étape qui retire son dernier symbole (contrôle 4).
- `FAILED:` si une coupe n'a pas son essai seul, ou si un essai (seul ou cumulé) fait entrer une paire, au compteur ou dans le graphe d'usage, ou fait monter une autre catégorie (contrôles 5, 6).
- `FAILED:` si une technique répète un `typedef` (piège 4), ou si une preuve de 2.17 dit « objets identiques » pour un corps déplacé ou une signature changée (piège 5).
- `FAILED:` si la ligne 2.17 n'a pas, dans le plan, ses sous-étapes, ses comptes d'arrivée (paires et lignes), ses preuves et ses budgets, ou si le plan change ailleurs que sur 2.11 (note), 2.14, 2.15, 2.17, 2.18 et la ligne d'estimation.
- `FAILED:` si une question à l'humain dépasse 100 mots, emploie un nom minifié ou un numéro de TROU, ou porte sur un point qu'un critère de ce PRD tranche.
- `FAILED:` si le tableau « Runs » manque, ou si la copie n'est pas supprimée à la fin de 2.11b.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Toute modification de code, de TU, d'outil ou du `Makefile`. Aucune coupe n'est commise : c'est 2.17 (ou, pour les coupes que l'étude avance, la sous-étape qu'elle nomme).
- Les décisions de 2.9 (archétypes) et de 2.10 (accesseurs, aplatissement, lien parent) : 2.11 les lit, ne les rouvre pas. Si une coupe demandait de les changer, l'étude le signale en « Découvert à 2.11 » et range la paire dans 2.17 sans la couper autrement.
- Les inclusions de `tests/`, les cycles de plus de deux modules (le compteur ne compte que des paires ; l'étude peut les signaler), les includes superflus qui ne réalisent aucune paire.
- Renommer un module ou un symbole (phase 3). L'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Le livrable est `docs/etude-2-11-inclusions.md`, pas `docs/prd/done/plan3-phase2-etape11-etude-inclusions.md`.** Même conflit et même règle que 2.9 (son G1) : ce PRD prendra ce chemin dans `done/`, et `plan-pilot` le lit pour trouver l'étape suivante. La note « Découvert à 2.11 » corrige la sortie de la ligne 2.11.
- **G2. Une coupe est une coupe d'usage.** Le compteur compte des lignes ; 14 lignes sur 34 tombent sans rien changer au code. Q2 veut les modules démêlés, pas les lignes cachées.
- **G3. Les paires cachées entrent dans l'étude.** Elles sont des inclusions mutuelles réelles que le compteur ne voit que par un détour ; « on démêle » les couvre. L'étude les coupe dans 2.17 quand une coupe du compteur les touche déjà (T2 sur `pico8_fm_hy`, paire 3 pour `scroll`), sinon elle les liste avec leur coût et les range dans 2.17.
- **G4. Garde de l'usage : à l'étude de chiffrer.** La garde de 2.17 (`inclusion-mutuelle` dans `COMPOSITION_RESIDUE_GUARD`) ne voit pas un détour réintroduit. L'étude chiffre l'ajout d'un contrôle d'usage au compteur (catégorie ou mode `--check`) et le place dans 2.17 ou 2.18, ou dit pourquoi le graphe d'usage relancé à chaque sous-étape suffit.
- **G5. Deux sous-étapes.** Le relevé est scripté (≈ 1 s), un essai coûte ≈ 3 s ; ce qui prend du temps est la lecture de 2.10 pour le sort des symboles et la réécriture de 2.17.

### Surprises de mesure, pour `plan-pilot`

1. **Budget** : le plan donne ≈ 50 min pour 2.11 ; ce PRD en compte **2 × 50 min**. La ligne « Estimation » gagne 1 sous-étape.
2. **Détours** : 14 des 34 lignes `#include` de `.c` qui réalisent une paire sont retirables sans erreur. Sans G2, 2.17 pourrait « vider » le compteur sans démêler.
3. **Trois paires cachées** : `economy` ↔ `economy_score`, `economy` ↔ `tech_gate` (par `pico8_fm_hy` de `probe.h`, qui rend un `pico8_hy_t` de `economy.h`), `entity_geom` ↔ `scroll` (par `entity.h` → `scroll.h`). La Sortie de la phase (`inclusion-mutuelle` à 0) serait tenue avec elles en place.
4. **`pico8_fm_hy` est déclaré dans `probe.h`** alors qu'il rend le `hy` d'`economy` : une seule déclaration déplacée touche 4 paires (2 du compteur, 2 cachées).
5. **Les numéros de ligne de la ligne 2.11 du plan datent de `ffd0a63`** (`planet.c:417` → `:412`, `slipway.c:140` → `:139`, `planet.c:471` → `:466`, `planet.c:509` → `:504`).
6. **Clang refuse le `typedef` répété** que gcc accepte en C99 ; aucune technique ne doit en dépendre.

### Questions pour l'humain

Aucune. Celles que l'étude trouvera sont posées par elle, dans son §5.

## Annexe A — les deux scripts essayés (texte exact)

Lancés depuis le dossier du script, avec le chemin de la copie en argument : `python3 cross.py tree > cross.tsv`, `python3 usegraph.py tree > usepairs.tsv`. Sorties : `cross.tsv` (fichier:ligne, en-tête, nombre de symboles, symboles) ; `usepairs.tsv` (A, B, symboles A → B, symboles B → A) et `paires N` sur la sortie d'erreur.

`cross.py` :

```python
#!/usr/bin/env python3
# Pour chaque ligne #include d'une paire mutuelle : identifiants declares par
# l'en-tete inclus et employes par le fichier qui l'inclut.
import re, sys, os, subprocess
T = sys.argv[1]; os.chdir(T)
lines = subprocess.run(['sh','tools/composition_residue.sh','--check','inclusion-mutuelle'],capture_output=True,text=True).stdout
edges=[]
for l in lines.split('\n'):
    m=re.match(r'inclusion-mutuelle: (\S+):(\d+):#include "(\S+)"',l)
    if m: edges.append((m.group(1),int(m.group(2)),m.group(3)))
def strip(s):
    s=re.sub(r'/\*.*?\*/',' ',s,flags=re.S); s=re.sub(r'//[^\n]*','',s)
    s=re.sub(r'"(\\.|[^"\\])*"','""',s); return s
decl_cache={}
KW=set('if for while return sizeof switch'.split())
def decls(h):
    if h in decl_cache: return decl_cache[h]
    s=strip(open('src/'+h).read()); n=set()
    n|=set(re.findall(r'#define\s+(\w+)',s))
    depth=0; buf=''
    for ch in s:
        if ch=='{': depth+=1
        if depth<=1: buf+=ch
        if ch=='}': depth-=1
    n|=set(x for x in re.findall(r'(\w+)\s*\(',buf) if x not in KW and not x.startswith('__'))
    n|=set(re.findall(r'\}\s*(\w+)\s*;',s))
    n|=set(re.findall(r'typedef\s[^;{]*?(\w+)\s*;',s))
    n|=set(re.findall(r'struct\s+(\w+)',s))
    n|=set(re.findall(r'extern\s[^;(]*?(\w+)\s*(?:\[[^\]]*\])?\s*;',s))
    for body in re.findall(r'enum\s*\w*\s*\{([^}]*)\}',s):
        n|=set(re.findall(r'(\w+)\s*(?:=[^,]*)?,?',body))
    n={x for x in n if x and not x[0].isdigit() and x not in ('void','int','char','const','unsigned','bool','PICO8')}
    decl_cache[h]=n; return n
for f,ln,h in edges:
    toks=set(re.findall(r'\b\w+\b',strip(open(f).read())))
    use=sorted(decls(h)&toks)
    print(f"{f}:{ln}\t{h}\t{len(use)}\t{' '.join(use)}")
```

`usegraph.py` :

```python
#!/usr/bin/env python3
# Graphe d'usage entre modules de src/ : a -> b si un fichier de a emploie un
# nom que b.h declare (et que b.h est seul a declarer). Paires mutuelles.
import re, sys, os, glob, collections
os.chdir(sys.argv[1])
def strip(s):
    s=re.sub(r'/\*.*?\*/',' ',s,flags=re.S); s=re.sub(r'//[^\n]*','',s)
    return re.sub(r'"(\\.|[^"\\])*"','""',s)
KW=set('if for while return sizeof switch void int char const unsigned'.split())
def top(s):
    d=0; out=''
    for ch in s:
        if ch=='{': d+=1
        if d==0 or ch=='}': out+=ch if d==0 else ''
        if ch=='}': d-=1
    return out
decl={}
for h in glob.glob('src/*.h'):
    s=strip(open(h).read()); t=top(s); n=set()
    n|=set(re.findall(r'#define\s+(\w+)',s))
    n|=set(x for x in re.findall(r'(\w+)\s*\([^()]*(?:\([^()]*\)[^()]*)*\)\s*;',t) if x not in KW)
    n|=set(re.findall(r'\(\s*\*\s*(\w+)\s*\)',t))
    n|=set(re.findall(r'\}\s*(\w+)\s*;',t))
    n|=set(re.findall(r'typedef\s[^;{}]*?(\w+)\s*;',t))
    n|=set(re.findall(r'extern\s[^;(]*?(\w+)\s*(?:\[[^\]]*\])?\s*;',t))
    for body in re.findall(r'enum\s*\w*\s*\{([^}]*)\}',s): n|=set(re.findall(r'(\w+)\s*(?:=[^,]*)?(?:,|$)',body.strip()))
    n-= {'','PICO8'}; n={x for x in n if not x.startswith(('PICO8_','__')) or re.match(r'PICO8_\w+',x)}
    decl[os.path.basename(h)[:-2]]=n
owner={}
for m,ns in decl.items():
    for x in ns: owner.setdefault(x,set()).add(m)
files=collections.defaultdict(list)
for f in glob.glob('src/*.[ch]'): files[os.path.basename(f)[:-2]].append(f)
use=collections.defaultdict(lambda: collections.defaultdict(set))
for a,fs in files.items():
    toks=set()
    for f in fs: toks|=set(re.findall(r'\b[A-Za-z_]\w*\b',strip(open(f).read())))
    for x in toks:
        for b in owner.get(x,()):
            if b!=a and len(owner[x])==1: use[a][b].add(x)
pairs=[(a,b) for a in use for b in use[a] if a<b and a in use.get(b,{})]
for a,b in sorted(pairs): print(f"{a}\t{b}\t{len(use[a][b])}:{' '.join(sorted(use[a][b]))}\t{len(use[b][a])}:{' '.join(sorted(use[b][a]))}")
print('paires', len(pairs), file=sys.stderr)
```

Limites connues, à traiter à la lecture : les deux scripts lisent les déclarations par motif (un prototype sur plusieurs lignes, une macro à paramètres ou un nom déclaré dans deux en-têtes peuvent être manqués) ; `cross.py` compte aussi les noms que l'en-tête reçoit d'une déclaration anticipée (`struct X`) ; un nom employé seulement dans un commentaire est retiré avec lui, mais un nom employé dans une macro d'un autre en-tête n'est pas suivi. La confirmation d'une coupe est toujours le compilateur plus le graphe d'usage relancé.
