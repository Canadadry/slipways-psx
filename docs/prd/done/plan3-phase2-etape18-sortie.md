---
title: "Plan 3, étape 2.18 : revue de sortie de la phase 2 — Sortie constatée, `warn-review` de fin de phase, `CLAUDE.md` au présent, plan clos ; six restes hors Sortie posés à l'humain — un commit, aucune ligne compilée touchée"
description: "Constat, commande par commande, des quatre critères de Sortie de la phase 2 sur le commit de 2.17f (pied `résidu 2 : 0 ligne(s), 0 fichier(s)`, garde de 14 catégories égale à `SORTIE` et `CATEGORIES`, aucune paire d'usage, 26 archétypes et 14 tables dispensées, test ponctuel passé pour les 26 classes puis retiré, filets verts) ; revue `warn-review` de la phase entière ; clôtures : phrase de `CLAUDE.md` au présent, note « Découvert à 2.18 » qui clôt la phase. Liste mesurée des restes hors Sortie (`void *` d'entité hors périmètre de `void-ptr`, deux cycles du graphe d'usage, 72 includes à 0 nom, noms provisoires, mutations équivalentes, trous de test), avec six questions à l'humain (98 mots au total) : phase 2 prolongée, ou nettoyage du code C annoncé pour plus tard."
status: needs-triage
---

## Problem Statement

**Écrit sur `b9b8fd2`** (branche `claude/plan-pilot-3-7zdft0`). Départ réel attendu : **le commit de 2.17f**, dont l'implémentation était en cours dans l'arbre de travail du dépôt pendant l'écriture (fichiers non commités, non lus). L'arbre des mesures est `b9b8fd2` (arbres de 2.17d2 : `src` `b5a5cb59…`, `tests` `ea2eb691…`, `Makefile` `422546ca…`) + `git apply` des deux diffs de l'annexe B du PRD 2.17e–f (`docs/prd/triage/plan3-phase2-etape17ef-fin-demelage.md`), rejoués sur une copie : **`src` `452f895c…`, `tests` `d78e7efb…` puis `9f857991…`, `Makefile` `2f295599…` puis `0b084174…`, `tools` `f5677a38…`** — exactement les arbres que ce PRD annonce pour `e` et `f`. Notation : `f` = le commit de 2.17f (départ) ; `x` = le commit de ce PRD. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier ; aucun `make` lancé dans le dépôt. Essais sur une seule copie jetable (scratchpad `prd218/`), supprimée depuis.

Vu du mainteneur du port : la phase 2 a remplacé l'enchâssement des classes par un noyau commun, des archétypes `static const` et des composants par pointeur. Son compteur est vide à `f`. Il reste à **constater** la Sortie telle que le plan l'écrit — chaque critère par une commande et son attendu —, à faire la **revue de fin de phase** que le plan prévoit (`warn-review`), à fermer ce que la Sortie exige encore hors du code (la phrase de `CLAUDE.md` qui dit « en cours », la note du plan), et à dire **honnêtement ce qui reste** hors Sortie, pour que l'humain choisisse : le traiter dans une phase 2 prolongée, ou le laisser au nettoyage du code C qu'il a annoncé pour plus tard.

### Texte du plan, recopié (`docs/plan-3.md` à `b9b8fd2`)

> - **Sortie** : aucune struct d'entité n'enchâsse plus la struct de sa parente (tout type = noyau + archétype + composants) ; chaque ancienne classe a son archétype `static const` ; le test ponctuel est passé pour chacune avant d'être supprimé ; `make composition-residue` rend `résidu 2 : 0 ligne(s), 0 fichier(s)`, toutes catégories à zéro, `inclusion-mutuelle` et `nu` compris (Q1, Q2, Q4).

> - 2.18 — sortie de phase : `make composition-residue` rend `résidu 2 : 0 ligne(s), 0 fichier(s)`, les treize catégories dans `COMPOSITION_RESIDUE_GUARD` (dont `registres`, 2.10) (2.11 : quatorze, avec `usage-mutuel`, 2.17f) ; aucun test ponctuel restant (2.9 : `test_pico8_archetype_chain.c` retiré en 2.13e2) ; 26 archétypes, les 14 autres tables dispensées pour les raisons de l'étude, §2 ; `class-coverage-net` (identique à `tools/class_coverage_net_expected.txt`) et `sanitize-net` verts ; revue `warn-review` de sortie. Budget ≈ 50 min.

§7 : `class-coverage-net` et `sanitize-net` « toujours à la sortie de phase ». Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte changée.

Décisions de l'humain qui fixent la Sortie : **Q1** (les catégories ajoutées à la revue de sortie de D sont bloquantes), **Q2** (`inclusion-mutuelle` démêlée, dans `SORTIE`), **Q4** (`nu` renommé, catégorie à 0), **planètes A** (un seul type planète, un seul type sonde), **Q-2.10** (liens de composant au noyau ; « la taille du code C est reprise plus tard, hors phase 2 »), **Q-2.14d**. Et, transmis par le pilote pour ce PRD : l'humain veut dans `CLAUDE.md` « pas d'historique, que du présent », et a annoncé un **nettoyage du code C pour plus tard**.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que fait 2.18 |
|---|---|
| table « Ce que 2.18 aura à constater » du PRD 2.17e–f (8 lignes, mesurées à `f`) | chaque ligne devient un constat C1 à C9 (table « Critères de Sortie »), **remesuré sur la copie : identique** |
| « garde complète » (pilote) : `COMPOSITION_RESIDUE_GUARD` = toutes les catégories de `SORTIE` | mesuré déjà vrai à `f` (14 = 14 = 14, C3) ; 2.18 ne touche le `Makefile` que si le départ le contredit (repli 2) |
| message de `f938bfb` (2.16) : « `void-ptr` à 0 ne couvre que les prototypes de premier niveau des `.h` ; des `void *` d'entité restent hors périmètre (13 autres membres de la table de classe et `pico8_state_fn`, 16 `*_dispatch`, trois fonctions `static`, rappels de `collect.h`/`ptrs.h`, locale `draw` d'`entity.c`) » | inventaire remesuré à `f` (reste 1) ; **19** `*_dispatch` publics et 37 `static` (le vérificateur de 2.16 en comptait 16 publics ; 2.17a en a déclaré 4 dans les `.h`) ; coût de typer la table mesuré (231 erreurs) |
| étude 2.11, §3 et PRD 2.17e–f : « le cycle de 25 modules, hors périmètre » | remesuré : à `f`, **deux** composantes, 4 et 19 modules (2.17e a scindé la composante de 25) (reste 2) |
| PRD 2.17e–f, restes : includes préexistants à 0 nom, noms provisoires `pico8_kr`, `pico8_jo_kv`, `pico8_fm.h`, `pico8_probe_comp.h` | `deadinc.py` sur tout `src/` : **72** lignes (reste 3) ; noms (reste 4) |
| messages « vérifiée et notée » de 2.4 à 2.17d et notes du plan : mutations survivantes déclarées, trous de test | relus un par un ; ce qui reste ouvert à `f` est dans les restes 5 et 6 ; ce qui a été fermé depuis est écrit comme tel |
| sortie de D (`docs/prd/done/plan3-phaseD-etape21*`, `warn-review-phaseD-sortie.md` du scratchpad) | même forme de revue (gravité, lieu, constat, risque, remède) ; **différence voulue** : les remèdes ne deviennent pas d'office des étapes 2.19+ (comme D.21) — l'humain tranche par les questions |

## Solution

Un commit `docs(2.18)`, sans ligne compilée touchée, précédé d'une revue.

1. **Constater la Sortie** sur l'arbre de `f` (propre) : `make test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e`, `make class-coverage-net`, `make sanitize-net`, puis `constat218.sh` (annexe A) ; chaque ligne rend l'attendu de la table « Critères de Sortie ». Un écart est un arrêt (critères d'échec), pas une correction dans 2.18.
2. **Revue `warn-review` de fin de phase** (lancée par le pilote, comme à la sortie de D) sur `eb18618..f` (`eb18618` = note de D.21d–f, phase D close) : `src/ tests/ tools/ Makefile` changent dans 123 fichiers (+6083 / −4998, mesuré). Sortie écrite dans le scratchpad, `warn-review-phase2-sortie.md`, jamais dans le dépôt. Format de la sortie de D : par gravité, lieu, constat, risque, remède (« maintenant » ou « au nettoyage »). Chaque avertissement est ensuite **classé** (section « Revue ») ; aucun n'est corrigé dans 2.18.
3. **Clôtures** (les seules modifications) :
   - `CLAUDE.md:5` : la phrase « Les objets du jeu sont des structs C plates ; leur composition (phase 2) est en cours — voir `docs/plan-3.md`. » devient « Les objets du jeu sont des structs C plates : un noyau commun (`pico8_struct_header_t`), un archétype `static const` par sorte, des composants par pointeur ; le plan actif est `docs/plan-3.md`. » (au présent : ni « phase 2 », ni « en cours » de chantier ; le renvoi au plan reste, il porte la phase 3) ;
   - `docs/plan-3.md` : note « Découvert à 2.18 » au bout de la ligne 2.18 (constats C1–C9 en une phrase, classement de la revue, « la phase 2 est close ») ; puis, sous les décisions de l'humain, un bloc **« Questions de 2.18 (en attente) »** qui recopie les six questions de ce PRD, chacune avec le reste qu'elle vise ;
   - `Makefile` : **rien** si C3 rend `GARDE = SORTIE = CATEGORIES (14)` (mesuré) ; sinon repli 2.
4. **Arrêt du pilote** après le commit : `plan-pilot` pose les six questions à l'humain et n'ouvre ni la phase 3 ni une étape 2.19 avant la réponse. Une réponse « maintenant » ajoute une ligne à la liste de la phase 2 (phase prolongée) ; « au nettoyage » la laisse au chantier annoncé, avec la mesure de ce PRD comme point de départ.

## Critères de Sortie et comment les constater

Mesurés sur la copie à `f` (Runs 3 à 9). « Commande » est lancée à la racine d'un arbre propre au commit de `f`, puis de `x`.

| # | Critère (texte du plan) | Commande | Attendu (mesuré à `f`) |
|---|---|---|---|
| C1 | `make composition-residue` rend `résidu 2 : 0 ligne(s), 0 fichier(s)`, toutes catégories à zéro | `make -s composition-residue` | **une seule ligne** : `résidu 2 : 0 ligne(s), 0 fichier(s)` (aucune ligne de catégorie) |
| C2 | chaque catégorie vidée gardée par `make test` | `make -s composition-guard` | `résidu gardé (noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr,inclusion-mutuelle,usage-mutuel) : 0 ligne(s)`, code 0 ; la même ligne dans le journal de `make test` |
| C3 | « les treize (quatorze) catégories dans `COMPOSITION_RESIDUE_GUARD` » ; garde complète | comparaison triée des trois listes (`constat218.sh`) | `GARDE = SORTIE = CATEGORIES (14)` |
| C4 | `inclusion-mutuelle` à 0 et aucune paire d'usage (Q2, 2.17f) | `python3 usegraph.py .` (annexe A du PRD 2.11) ; `sh tools/composition_residue.sh --detail src tests` | `paires 0` ; `--detail` sans ligne `inclusion-mutuelle` ni `usage-mutuel` (11 lignes `void-ptr-r2`, la ligne `info  src/pico8_game_data.h  429 lignes`, le pied) |
| C5 | aucune struct d'entité n'enchâsse sa parente ; tout type = noyau + archétype + composants | C1/C2 (`parente`, `lien-parent`, `accesseurs-q6`, `défauts` à 0 et gardés) ; `--champs \| tail -1` | `champs : 238 ; 0 écriture : 14 ; 0 hors constructeur : 19` |
| C6 | chaque ancienne classe a son archétype `static const` ; 26 archétypes, 14 tables dispensées (étude 2.9, §2) | noms des 40 tables `g_X_struct_class` contre les `extern const pico8_X_def_t pico8_X_def;` des `.h` (`constat218.sh`) | tables **40**, archétypes **26** (25 dans `game_data.h`, `pico8_hy_def` dans `economy.h` ; `pico8_probe_def_none` n'est pas un archétype de classe), archétype sans table **0**, tables sans archétype : `bv_et fm_placeholder ha_pz ie_ke_view jb jc_mouse_point jc_too_far je_placeholder lq mo_placeholder mz_jj nr_ldown_empty pj_cr trade` — **exactement les 14 « non » de l'étude 2.9, §2** |
| C7 | le test ponctuel est passé pour chacune avant d'être supprimé | `ls tests \| grep -c archetype_chain` ; `git show 5922ab8:tests/test_pico8_archetype_chain.c \| grep -c '^    test_[a-z0-9_]*();'` ; `git log --diff-filter=D -1 --format=%h -- tests/test_pico8_archetype_chain.c` | `0` ; **26** familles appelées à 2.12e2 (`5922ab8`, `g_compared == 438`, vert à ce commit : note « Découvert à 2.12e2 ») ; retiré par `4f81553` (2.13e2c), après le basculement de chaque famille |
| C8 | filets de sortie de phase (§7) | `make class-coverage-net` ; `make sanitize-net` | `couverture: OK -- chaque classe non exemptée est instanciée (26 rejeu(x))` (sortie égale à `tools/class_coverage_net_expected.txt`, que la cible compare) ; `sanitize: 26 propre(s)` |
| C9 | §3 : aucun re-baseline pendant la phase | `git diff --stat eb18618 f -- '*.sha256' tests/e2e` ; `make test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | vide ; vert ; **26 conformes** ; **26 conformes**. (Les huit constantes d'empreinte de TU changées à 2.14d1–d2 l'ont été par décision **Q-2.14d**, refaites au parent au seul pli : ce n'est pas un re-baseline du filet.) |

`constat218.sh` rend, à `f` (2,7 s) :

```
C1 pied        : résidu 2 : 0 ligne(s), 0 fichier(s)|
C2 garde       : résidu gardé (…) : 0 ligne(s)
C3 listes      : GARDE = SORTIE = CATEGORIES (14)
C4 usage       : paires 0
C5 cycles      : aretes 623 ; [4, 19] modules en cycle : 23
C6 archetypes  : tables 40, archetypes 26, sans table 0, dispensees : bv_et fm_placeholder ha_pz ie_ke_view jb jc_mouse_point jc_too_far je_placeholder lq mo_placeholder mz_jj nr_ldown_empty pj_cr trade
C7 ponctuel    : present 0 ; familles a 2.12e2 26 ; retire par 4f81553
C8 champs      : champs : 238 ; 0 écriture : 14 ; 0 hors constructeur : 19
C9 filet       : 0 ligne(s) de diff sha256/e2e depuis eb18618
```

(La ligne `C5` n'est pas un critère : elle mesure le reste 2. La ligne `C8` du script est le `--champs` du critère C5 de la table.)

À `x`, les mêmes lignes, octet pour octet : 2.18 ne touche ni `src/`, ni `tests/`, ni `tools/`, ni le `Makefile` (sauf repli 2).

## Revue `warn-review` de fin de phase

- **Périmètre** : `eb18618..f`, `src/ tests/ tools/ Makefile` (123 fichiers). La revue de sortie de D couvrait `0a74e49..39c4e23` ; rien entre `39c4e23` et `eb18618` ne touche ces dossiers hors D.21, déjà revu par ses vérificateurs.
- **Méthode** : skill `warn-review`, lancée par le pilote (le plan la prévoit à chaque fin de phase, comme en 1b et D). Sa question d'ouverture à l'humain est remplacée par les pistes ci-dessous, comme à la sortie de D. Écrite dans le scratchpad (`warn-review-phase2-sortie.md`), en lecture seule sur le dépôt.
- **Pistes** (relevées en mesurant ce PRD, à vérifier, pas à recopier) : `tools/composition_r2_sites.txt` cite des numéros de ligne datés (`probe.c:349` pour `pico8_d__dispatch`, aujourd'hui `:324` ; `particle.c:152`, `slipway.c:341`…) que `--detail` réimprime ; `pico8_game_data.h` 429 lignes (tous les types d'archétype) ; les `void *self` de la table de classe (91 lignes dans 32 fichiers de `src/`) ; les avertissements de la sortie de D encore ouverts (`check()`/`fnv` recopiés dans les TU, `src/` à plat, préfixes de fichier et d'export différents, français et anglais mêlés) ; les réserves de forme des vérificateurs (commentaires de `planet_family.h`/`planet_family_trade.h` séparés de leur `_ready`, deux commentaires de TU qui paraphrasent, le repère `TROU-7` parti avec `pico8_fv_t`, l'`assert(ed_fixed(other) == …)` de `test_pico8_pj.c:411`, le cast de `fold_pk` de `test_pico8_hj.c`).
- **Classement de chaque avertissement** (dans la revue, puis résumé dans la note du plan) :
  - (a) contredit un critère C1–C9 → **arrêt** (critère d'échec) ; le pilote n'en attend aucun ;
  - (b) relève d'un des six restes → rattaché à sa question, sans question neuve ;
  - (c) neuf, hors restes → ajouté à la liste des restes de la note du plan ; s'il demande une décision, il est **fondu dans une des six questions** (la plus proche) sans dépasser 100 mots au total — sinon il part au nettoyage, dit tel quel ;
  - aucun remède appliqué dans 2.18.

## Restes hors Sortie, mesurés à `f`, et questions à l'humain

Aucun n'est compté par une catégorie de `SORTIE` ; aucun ne bloque 2.18. Mesures de `restes218.sh` (annexe A, 6 s), sauf mention.

| # | Reste | Mesure à `f` | Ce qu'il en coûterait (ordre de grandeur) |
|---|---|---|---|
| 1 | **`void *` d'entité hors périmètre de `void-ptr`** (constat de `f938bfb`). `void-ptr` compte les prototypes de premier niveau des `.h` ; restent : | table de classe : **13** membres (`kj`, `ev`, `hk`, `hb`, `has_hb`, `ldown`, `rdown`, `gv`, `hq`, `ey`, `kf`, `cn`, `ol`, `struct.h:145-158`) + `pico8_state_fn` (`:132`, tableau `states`) ; `*_dispatch` : **56** définitions, **19** publiques dans 8 `.h`, 37 `static` ; **3** types de rappel (`collect.h:11`, `:22`, `ptrs.h:35`) et **3** fonctions `static` qui les servent (`collect.c:42`, `planet.c:213`, `planet_display.c:64`) ; la locale `draw` (`entity.c:227`) ; en tout 91 lignes `void *self` dans 32 fichiers de `src/`. Détail R2 : **6** des 11 sites de D.20b ont encore un `*_dispatch` en amont (`pico8_ke_ks`, `pico8_ez`, `pico8_ei`, `pico8_bv`, `pico8_dl`, `pico8_d_`) ; et `tools/composition_r2_sites.txt` cite des lignes datées | typer les 14 emplacements de la table (`void *self` → `pico8_struct_header_t *self`, essai sur copie) : **231 erreurs** de compilation dans 41 fichiers (18 de `src/`, 23 TU) ; l'étude 2.10 en mesurait 418 avant 2.14–2.16. Les dispatchers deviendraient des casts typés, un par cible |
| 2 | **Cycles du graphe d'usage** (étude 2.11, §3 ; signalé par 2.17e) | 623 arêtes orientées ; composantes fortement connexes : **4** (`entity`, `entity_geom`, `ke`, `scroll`) et **19** (`bg`, `economy`, `economy_formulas`, `economy_score`, `economy_tiers`, `fm`, `msgbox`, `planet`, `planet_display`, `planet_family`, `planet_trade`, `probe`, `research`, `research_display`, `slipway`, `tech_gate`, `ui`, `ui_button`, `wormhole`) ; 23 modules en cycle. Ce ne sont plus des paires : ni `inclusion-mutuelle` ni `usage-mutuel` ne les voient | non estimé : aucune étude ; chaque retour à couper est une coupe d'usage comme en 2.17 |
| 3 | **Includes préexistants à 0 nom** | `deadinc.py` (annexe A du PRD 2.17a–b) sur les 148 `.c`/`.h` de `src/` hors raylib : **72** lignes `#include` dont le fichier n'emploie aucun nom propre à l'en-tête (dont 12 × `pico8_ke.h`, tous dans des `.h`, et 12 × `pico8_entity.h`). **Majorant** : un include d'en-tête peut relayer des noms à ses propres includeurs (détours) ; aucun retrait n'a été essayé | par ligne : retrait, `-fsyntax-only` de `src/` et `tests/`, include direct ajouté là où un détour cassait (règle de 2.17d) |
| 4 | **Noms provisoires** (étude 2.11 ; PRD 2.17c–f) | en-têtes `pico8_fm.h` (71 lignes, sans `.c`) et `pico8_probe_comp.h` (64 lignes, sans `.c`) ; fonctions `pico8_kr` (`economy_score.c`) et `pico8_jo_kv` (`planet_family.c`) | renommage : un commit par nom, includes compris ; la phase 3 du plan ne renomme que selon le glossaire |
| 5 | **Mutations équivalentes déclarées** (jumelles vertes aux deux bouts, sans filet possible à l'écran) | PJ-init (2.17a, remise à 0 de `g_pj_hk_lh_t` inobservable) ; DJ-sw (2.17c, distance au carré, égale au bit près) ; GD-p (2.17e, `ra` ≠ `p` seulement pendant une secousse, marge ±76) ; M6, M7, M8 (2.6, bras inaccessible de `pj:hb`) ; N3, N4 (2.13e2b, `def` de sonde de `pb` sans lecteur) ; K-mp (2.14a, vu du grep seul) : **9** | aucun, si acceptées ; sinon un TU par mutation qui observe l'état, pas l'écran |
| 6 | **Trous de test signalés, encore ouverts** | M10 (2.11r-b3) : l'ordre de `la` dans `jc:ce` n'est observé par aucun test ; `pj.ed` (1) n'est lu par aucun code du jeu, seulement par `ed_fixed` du TU `pj` (2.13a2, P34) ; `emitter.pn`/`du`/`ki`/`pk` de `ga`, présents dans son archétype, ne sont jamais lus pour `ga` (2.12e2 C4 ; 2.13e2a D7, D8, D10, D11). **Fermés depuis leur signalement** (relu, à ne pas reposer) : `has_ik` de l'arbre `qh`, `kq` de `ga`/`nr`/`fr`, `ui.hw`, `iv.fa`, le lien `fa` de `jc`, l'ordre de `bv` sur `cg("jc")` (2.11r-d), la construction de `mz`, `g_`, `iv`, `mh` par `test_pico8_struct_header` | trois TU ciblés, ou le retrait des valeurs mortes |

**Questions à l'humain** (98 mots au total, comptés par `wc -w` ; sans nom minifié). Pour chacune, « maintenant » = une étape de plus en phase 2, avant la phase 3 ; « au nettoyage » = le chantier de code C annoncé pour plus tard, qui part de la mesure ci-dessus.

1. (reste 1) Des fonctions reçoivent encore un objet du jeu sans son type (environ 230 corrections) : maintenant ou au nettoyage ?
2. (reste 2) Deux boucles de dépendances entre modules (4 et 19) : les casser maintenant ou au nettoyage ?
3. (reste 3) Environ 70 inclusions inutiles : les retirer maintenant ou au nettoyage ?
4. (reste 4) Deux fichiers et deux fonctions aux noms provisoires : renommer maintenant ou au nettoyage ?
5. (reste 5) Une dizaine d'erreurs glissées exprès ne font échouer aucun test (effet invisible) : on l'accepte ?
6. (reste 6) Trois comportements sans test (un ordre de liste, deux valeurs jamais lues) : maintenant ou au nettoyage ?

## User Stories

1. En tant que mainteneur, je veux voir chaque critère de Sortie de la phase 2 rendu par une commande, afin de savoir que la phase est close sans relire 57 notes.
2. En tant que lecteur de `CLAUDE.md`, je veux une phrase qui dise ce que sont les objets du jeu aujourd'hui, sans le récit du chantier.
3. En tant qu'humain qui décide, je veux la liste mesurée de ce qui reste hors Sortie et une question courte par point, afin de choisir entre une phase 2 prolongée et le nettoyage annoncé.
4. En tant que pilote, je veux une revue de fin de phase classée, afin qu'aucun avertissement ne devienne une étape sans décision.

## Implementation Decisions

### Modules

`CLAUDE.md` (ligne 5), `docs/plan-3.md` (note « Découvert à 2.18 », bloc « Questions de 2.18 (en attente) »). Rien d'autre ; `Makefile` seulement sous le repli 2.

### Ce qui change

| Fichier | Avant | Après |
|---|---|---|
| `CLAUDE.md:5` | « … Les objets du jeu sont des structs C plates ; leur composition (phase 2) est en cours — voir `docs/plan-3.md`. » | « … Les objets du jeu sont des structs C plates : un noyau commun (`pico8_struct_header_t`), un archétype `static const` par sorte, des composants par pointeur ; le plan actif est `docs/plan-3.md`. » (le début de la ligne ne change pas) |
| `docs/plan-3.md`, fin de la ligne 2.18 | — | « Découvert à 2.18 (clôt la phase 2) : … » : C1–C9 tenus (valeurs de la table, une phrase) ; revue `eb18618..f` : N avertissements, classés (a) 0 / (b) … / (c) … ; restes 1 à 6 nommés avec leur mesure (une ligne chacun) ; « **La phase 2 est close** ; le prochain pas attend les réponses aux questions de 2.18 » |
| `docs/plan-3.md`, après « Décisions de l'humain » | — | « **Questions de 2.18 (en attente)** » : les six questions, telles quelles, chacune suivie de « (reste N) » |

La note ne garde que ce qui change une étape suivante (§7) : le détail des mesures va dans le message de commit.

### Anti-tassement

**Interdit** :
- corriger un reste ou un avertissement de la revue dans 2.18 (y compris les numéros datés de `tools/composition_r2_sites.txt`, les includes morts, un commentaire) : 2.18 constate, classe et demande ;
- toucher `src/`, `tests/`, `tools/`, un `*.sha256`, `tests/e2e/`, une constante d'empreinte, `docs/glossaire-noms.md`, `docs/trous-de-portage.md` ; toucher le `Makefile` hors repli 2 ;
- dans la phrase neuve de `CLAUDE.md`, écrire un mot d’historique (« phase 2 », « désormais », « depuis », « a été », « en cours ») ; toucher le reste de la ligne 5 ou une autre ligne ;
- reformuler les questions au-delà de 100 mots au total, y mettre un nom minifié, un numéro de TROU ou d'étape ; en ajouter une septième (une trouvaille neuve de la revue se fond dans une des six) ;
- ouvrir 2.19 ou la phase 3 avant la réponse de l'humain.

## Implementation Chunks

Un commit, **≈ 50 min** : filets et `constat218.sh` à `f` (≈ 25 s de calcul : `make -j4 test` 12 s, `make e2e` ×2 2 s, `class-coverage-net` 2 s, `sanitize-net` 4 s, script 3 s) ; `restes218.sh` (6 s, sur une copie jetable : il modifie puis restaure `struct.h`) ; revue `warn-review` (≈ 25 min) et son classement ; deux éditions de texte ; les mêmes constats à `x` ; note, message.

**Replis écrits**, dans l'ordre :
1. départ autre que le commit de 2.17f annoncé (arbres `src` `452f895c…`, `tests` `9f857991…`, `tools` `f5677a38…`, `Makefile` `0b084174…`, ou un descendant qui ne touche que `docs/`) : lancer les constats quand même ; tout écart à la table « Critères de Sortie » est rapporté dans la note, et un écart sur C1–C4, C6–C9 est un arrêt ;
2. C3 rend `ECART` (une catégorie de `SORTIE` absente de `COMPOSITION_RESIDUE_GUARD`) et `make composition-residue` est vide : ajouter la catégorie manquante en fin de `COMPOSITION_RESIDUE_GUARD`, prouver la garde mordante sur une copie (une ligne de la catégorie réinjectée → `make composition-guard` code 2), le dire dans la note ; si c'est `CATEGORIES` ≠ `SORTIE`, arrêter (l'outil a changé hors plan) ;
3. la revue dépasse 30 min : la limiter à `src/` et au `Makefile`, et le dire ;
4. un avertissement de classe (a) : ne pas commiter ; arrêter et rapporter.

## Testing Decisions

- **Pas de test neuf.** 2.18 ne change aucune ligne compilée ; le bon test est la commande qui rend le critère, lancée aux deux bouts.
- **Prior art** : sortie de D (`docs/prd/done/plan3-phaseD-etape21*`, revue `warn-review-phaseD-sortie.md`) ; sortie de 1a (`plan3-phase1a-etape42-suppression-pico8-class-sortie-1a.md`) ; PRD 2.17e–f (contrat de 2.18, `usegraph.py`) ; PRD 2.17a–b (`deadinc.py`) ; étude 2.11 (`scc.py`).

### Preuves (parent de `x` = `f`)

1. **Communes** : à `f` et à `x`, `make -j4 test` vert, `make e2e` **26 conformes**, `MALLOC_PERTURB_=165 make e2e` **26 conformes** ; `git diff f x -- src tests tools '*.sha256' tests/e2e` vide ; `git diff f x -- Makefile` vide (hors repli 2).
2. **Constats** : `constat218.sh` à `f` et à `x` rend les lignes de la table, identiques entre `f` et `x`.
3. **Filets de sortie** : `make class-coverage-net` et `make sanitize-net` verts à `f` (`src/` identique à `x`).
4. **Restes** : `restes218.sh` sur une copie de `f` rend R1 à R8 comme dans la table des restes (écart : la table est mise à jour dans la note, pas un arrêt).
5. **Questions** : les six lignes extraites du plan à `x` dans un fichier, `wc -w` ≤ 100 ; `grep -cE 'pico8_|TROU|_' ` sur ce fichier → 0 ; relecture : aucun nom minifié ni sigle.
6. **`CLAUDE.md`** : `git diff f x -- CLAUDE.md` = une ligne ; `grep -c 'phase 2\|en cours\|désormais' CLAUDE.md` → 0 ; `grep -c 'pico8_struct_header_t' CLAUDE.md` → 1.

### Greps de Sortie (à `x`)

1. `make -s composition-residue` → une ligne, `résidu 2 : 0 ligne(s), 0 fichier(s)`.
2. `grep -o 'inclusion-mutuelle,usage-mutuel$' Makefile` → 1 ligne ; `grep -c '^COMPOSITION_RESIDUE_GUARD' Makefile` → 1.
3. `ls tests | grep -c archetype_chain` → 0.
4. `grep -c 'Découvert à 2.18' docs/plan-3.md` → 1 ; `grep -c 'Questions de 2.18' docs/plan-3.md` → 1 ; `grep -c 'La phase 2 est close' docs/plan-3.md` → 1.
5. `grep -c 'leur composition (phase 2) est en cours' CLAUDE.md` → 0.
6. `git diff --stat f x` : `CLAUDE.md`, `docs/plan-3.md`, rien d'autre (hors repli 2 : `Makefile`).

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd218/copie`, journaux dans `prd218/logs`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout b9b8fd2` ; `git rev-parse` des arbres | `d2` | 0 | `b5a5cb59…`, `ea2eb691…`, `422546ca…` (= 2.17d2) | 3 s |
| 2 | annexe B du PRD 2.17e–f extraite (lignes 663-872, 878-1142) ; `git apply` e puis f, commit de chacun ; `rev-parse` | `e`, `f` | 0 | `452f895c…`, `d78e7efb…`, `2f295599…` ; puis `9f857991…`, `f5677a38…`, `0b084174…` (ceux du PRD) | 1 s |
| 3 | `make -j4 test` | `f` | 0 | `résidu gardé (…,inclusion-mutuelle,usage-mutuel) : 0 ligne(s)` ; `test_composition_residue: all tests passed` | 12 s |
| 4 | `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | `f` | 0 ; 0 | 26 conformes ; 26 conformes | 1 s ; 1 s |
| 5 | `make class-coverage-net` ; `make sanitize-net` | `f` | 0 ; 0 | `couverture: OK … (26 rejeu(x))` ; `sanitize: 26 propre(s)` | 2 s ; 4 s |
| 6 | `make composition-residue`, `composition-guard`, `--detail`, `--champs` ; trois listes triées | `f` | 0 | pied seul `0 / 0` ; 14 = 14 = 14 ; `238 ; 14 ; 19` ; 11 lignes `void-ptr-r2`, dont 6 avec un `*_dispatch` en amont | 2 s |
| 7 | `usegraph.py` (PRD 2.11), `scc.py` (étude 2.11), `usegraph_e.py` (dérivé, annexe A) | `f` | 0 | `paires 0` ; 623 arêtes ; [4, 19], 23 modules | 2 s |
| 8 | tables contre archétypes ; historique de `test_pico8_archetype_chain.c` | `f` | 0 | 40 / 26 / 0, 14 dispensées = étude §2 ; 26 familles, 438 comparaisons à `5922ab8` ; retiré par `4f81553` | < 1 s |
| 9 | `git diff --stat eb18618 HEAD -- '*.sha256' tests/e2e` ; `git diff --shortstat eb18618 HEAD -- src tests tools Makefile` | `f` | 0 | vide ; 123 fichiers, +6083 / −4998 | < 1 s |
| 10 | inventaire des `void *` d'entité (greps de `restes218.sh`) ; `*_dispatch` à `9794a11` (2.16) et `f7913b1` (2.17d2) par `git grep` | `f` | 0 | 13 + 1 ; 19 publics / 37 `static` / 56 ; 3 rappels, 3 `static`, `entity.c:227` ; à 2.16 : 15 publics dans les `.h`, 56 définitions | < 1 s |
| 11 | `syn.sh` (base) ; 14 emplacements de la table typés par `sed` ; `syn.sh` ; `git checkout` | `f` | 0 | base : 1 erreur (`raylib.h` absent, préexistante) ; typée : 232 → **231** de plus, 41 fichiers ; restauré, 1 | 25 s |
| 12 | `deadinc.py` sur les 148 `.c`/`.h` de `src/` hors raylib | `f` | 0 | 72 `MORT` | 2 s |
| 13 | `constat218.sh` premier jet | `f` | 0 | C2 tronqué (`sed` glouton sur `ligne(s)`) → motif `(.*) :` | 3 s |
| 14 | `constat218.sh` ; `restes218.sh` | `f` | 0 ; 0 | lignes des tables ; arbre propre après R6 | 3 s ; 6 s |
| 15 | questions : trois jets, `wc -w` | — | 0 | 116 → 103 → **98** | < 1 s |
| 16 | diff de l'annexe B tiré du PRD, `git apply` sur la copie ; greps de la preuve 6 ; `git checkout` | `f` | 0 | premier jet : `corrupt patch` (en-tête de bloc à 7 lignes, contexte final vide perdu) → `@@ -2,6 +2,6 @@` ; puis appliqué, 0 mot d'historique, `pico8_struct_header_t` 1 | < 1 s |
| 17 | `rm -rf` de la copie, des journaux et des fichiers d'essai | — | 0 | supprimés | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit de 2.17f (arbres ci-dessus, ou un descendant qui ne touche que `docs/`) sans que l'écart soit rapporté ; si un constat C1–C4, C6–C9 ne rend pas l'attendu à `f` ou à `x` (arrêt, pas correction) ; si C5 change sans explication.
- `FAILED:` si `make test` n'est pas vert, si `make e2e` ou `MALLOC_PERTURB_=165 make e2e` ne rend pas 26 conformes, si `class-coverage-net` ou `sanitize-net` est rouge, à `f` ou à `x`.
- `FAILED:` si le diff `f..x` touche autre chose que `CLAUDE.md` (une ligne) et `docs/plan-3.md` (la note de 2.18 et le bloc des questions), hors repli 2 rapporté ; si `src/`, `tests/`, `tools/`, un `*.sha256`, `tests/e2e/` ou une constante d'empreinte change.
- `FAILED:` si la phrase de `CLAUDE.md` garde un mot d'historique ou n'est pas celle de « Ce qui change » (au mot près, sauf retouche de ponctuation déclarée).
- `FAILED:` si la revue `warn-review` n'a pas été faite sur `eb18618..f`, si son fichier n'est pas dans le scratchpad, ou si un de ses avertissements n'est pas classé (a)/(b)/(c) dans la note ; si un avertissement (a) est commité ; si un remède est appliqué.
- `FAILED:` si la liste des restes de la note omet un des six, ou si une question manque, dépasse le total de 100 mots (`wc -w`), nomme un symbole minifié, un TROU ou une étape.
- `FAILED:` si la note « Découvert à 2.18 » manque, ne dit pas « La phase 2 est close », ou si le plan change ailleurs qu'à la ligne 2.18 et au bloc des questions.
- `FAILED:` si le pilote ouvre 2.19 ou la phase 3 sans réponse de l'humain.
- `FAILED:` si le message de commit omet : les constats C1–C9 avec leurs valeurs, le périmètre et le classement de la revue, les six restes avec leurs mesures, les questions, le tableau « Runs ».
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Tout traitement d'un reste (1 à 6) ou d'un avertissement de la revue : il attend la réponse de l'humain.
- La phase 3 (renommage selon le glossaire) ; la taille du code C (Q-2.10) ; le `malloc` des entités ; l'appli raylib ; l'audio ; PSX.
- `make e2e-gap` (hors suite, déjà rouge avant la phase 2 ; 2837 lignes à 2.6) et `tools/line_coverage_reference.txt`.
- Les autres lignes de `CLAUDE.md`.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Un commit.** Rien de compilé ne change ; les deux textes se relisent ensemble. La revue n'est pas commitée (scratchpad, comme en D).
- **G2. La garde n'est pas réécrite** : elle est déjà complète à `f` (C3). Remplacer la liste littérale par une lecture de `SORTIE` changerait l'outil sans nécessité ; la forme de `DATA_RESIDUE_GUARD` est la même.
- **G3. Les remèdes de la revue ne deviennent pas des étapes d'office** (écart voulu avec D.21) : l'humain a annoncé un nettoyage du code C pour plus tard ; une étape 2.19 n'existe que sur sa réponse « maintenant ».
- **G4. La phrase de `CLAUDE.md`** nomme le noyau par son type (`pico8_struct_header_t`, que le lecteur retrouvera par grep) et garde le renvoi à `docs/plan-3.md` (il porte la phase 3, au présent : « le plan actif », comme « Cible active ») ; les mots « archétype » et « composant » sont ceux du plan.
- **G5. Les restes 5 et 6 séparent** ce qui ne peut pas avoir de filet à l'écran (mutations équivalentes : accepter ou non) de ce qui pourrait en avoir un (trous de test : quand) ; les trous fermés depuis leur signalement sont listés pour ne pas être reposés.
- **G6. Le reste 3 est un majorant** : `deadinc.py` ne voit pas le relais d'un en-tête à ses includeurs ; la question dit « environ 70 », pas « 72 ».
- **G7. Les numéros datés de `tools/composition_r2_sites.txt`** vont au reste 1 (c'est le détail de `void-ptr`), pas à une correction dans 2.18.

### Surprises de mesure, pour `plan-pilot`

1. **Le cycle n'est plus de 25 modules** : 2.17e l'a scindé en 4 + 19 (23 en cycle) ; la question le dit.
2. **Les `*_dispatch` publics sont 19**, pas 16 : 2.17a en a déclaré quatre dans des `.h` (`git grep` : 15 à `9794a11`, 19 à `f7913b1`).
3. **Typer toute la table coûte 231 erreurs**, pas les 418 de l'étude 2.10 : 2.14–2.16 ont retiré les accesseurs et typé `hd`/`mm`.
4. **Six sites R2 sur onze** gardent un `void *` en amont, tous par un `*_dispatch` ; `--detail` les imprime avec des numéros de ligne datés.
5. **Le test ponctuel a vu les 26 classes en même temps** une seule fois, à `5922ab8` (438 comparaisons) ; C7 s'appuie sur ce commit.

### Questions pour l'humain

Les six de la section « Restes » (98 mots), posées par `plan-pilot` après le commit de 2.18 ; aucune ne bloque ce commit.

### Annexe A — scripts (texte exact)

Hors dépôt, dans un dossier d'essais `O` qui contient aussi : `usegraph.py` (annexe A du PRD 2.11, `docs/prd/done/plan3-phase2-etape11-etude-inclusions.md`, bloc après la ligne 289), `scc.py` (annexe A de l'étude 2.11, `docs/etude-2-11-inclusions.md`, bloc après la ligne 432), `deadinc.py` (annexe A du PRD 2.17a–b, `docs/prd/done/plan3-phase2-etape17ab-affichage.md`, bloc après la ligne 494). Extraction : `awk -v start=N 'NR>start && /^```/ {c++; if(c==2) exit; next} NR>start && c==1' FICHIER`.

`usegraph_e.py` (arêtes orientées dans `$EDGES`, au format que `scc.py` lit) :

```sh
python3 - <<'EOF'
s=open('usegraph.py').read()
s=s.replace("pairs=[(a,b)", "open(os.environ['EDGES'],'w').write(''.join(f'E\\t{a}\\t{b}\\t{len(use[a][b])}\\n' for a in sorted(use) for b in sorted(use[a])))\npairs=[(a,b)",1)
open('usegraph_e.py','w').write(s)
EOF
```

`syn.sh` :

```sh
#!/bin/sh
# compile every .c of src/ and tests/ with -fsyntax-only; print error lines
cd "$1"
for f in src/*.c tests/*.c; do
  case $f in *raylib*) continue;; esac
  fl=""; case $f in *class_coverage*) fl="-DPICO8_CLASS_COVERAGE";; esac
  cc -std=c99 -Wall -Wextra -Werror -Isrc $fl -fsyntax-only "$f" 2>&1 | grep -E "error:" 
done
```

`constat218.sh` (lancé `sh constat218.sh <arbre> <O>`) :

```sh
#!/bin/sh
# constat218.sh ARBRE OUTILS : les constats de Sortie de la phase 2 sur ARBRE
# (arbre de travail propre au commit de 2.17f ou de 2.18) ; OUTILS contient
# usegraph.py, usegraph_e.py, scc.py (annexe A). Une ligne par constat.
T=$1; O=$2; cd "$T" || exit 2
echo "C1 pied        : $(make -s composition-residue | tr '\n' '|')"
echo "C2 garde       : $(make -s composition-guard | sed 's/(.*) :/(…) :/')"
l() { grep "^$1=" tools/composition_residue.sh | sed 's/.*="//;s/"$//' | tr ' ' '\n' | sort; }
g=$(grep '^COMPOSITION_RESIDUE_GUARD' Makefile | sed 's/.*:= //' | tr ',' '\n' | sort)
[ "$g" = "$(l SORTIE)" ] && [ "$(l SORTIE)" = "$(l CATEGORIES)" ] \
  && echo "C3 listes      : GARDE = SORTIE = CATEGORIES ($(l SORTIE | wc -l))" \
  || echo "C3 listes      : ECART"
echo "C4 usage       : $(python3 "$O/usegraph.py" . 2>&1 >/dev/null)"
EDGES=$O/edges.txt python3 "$O/usegraph_e.py" . >/dev/null 2>&1
echo "C5 cycles      : aretes $(wc -l < "$O/edges.txt") ; $(python3 "$O/scc.py" "$O/edges.txt" | sed 's/.*: \[/[/')"
grep -hoE "pico8_struct_class_t g_[a-z0-9_]+_struct_class =" src/*.c | sed -E 's/.*t g_(.*)_struct_class =/\1/' | sort > "$O/tables.txt"
grep -hoE "^extern const pico8_[a-z0-9_]+_def_t pico8_[a-z0-9_]+_def;" src/*.h | sed -E 's/.* pico8_(.*)_def;/\1/' | sort > "$O/defs.txt"
echo "C6 archetypes  : tables $(wc -l < "$O/tables.txt"), archetypes $(wc -l < "$O/defs.txt"), sans table $(comm -13 "$O/tables.txt" "$O/defs.txt" | wc -l), dispensees : $(comm -23 "$O/tables.txt" "$O/defs.txt" | tr '\n' ' ')"
echo "C7 ponctuel    : present $(ls tests | grep -c archetype_chain) ; familles a 2.12e2 $(git show 5922ab8:tests/test_pico8_archetype_chain.c | grep -c '^    test_[a-z0-9_]*();') ; retire par $(git log --format=%h --diff-filter=D -1 -- tests/test_pico8_archetype_chain.c)"
echo "C8 champs      : $(sh tools/composition_residue.sh --champs src tests | tail -1)"
echo "C9 filet       : $(git diff --stat eb18618 HEAD -- '*.sha256' tests/e2e | wc -l) ligne(s) de diff sha256/e2e depuis eb18618"
```

`restes218.sh` (sur une copie jetable : R6 modifie puis restaure `src/pico8_struct.h`) :

```sh
#!/bin/sh
# restes218.sh ARBRE OUTILS : les restes hors Sortie, mesures sur ARBRE
# (arbre de travail propre, jetable : R6 modifie puis restaure struct.h).
T=$1; O=$2; cd "$T" || exit 2
echo "R1 table       : $(sed -n '130,160p' src/pico8_struct.h | grep -c 'void \*self') membres void *self (dont pico8_state_fn)"
echo "R2 dispatch    : $(grep -cE '_dispatch\((const )?void \*' src/*.h | awk -F: '{s+=$2} END{print s}') publics (.h), $(grep -E '^static [a-z].*_dispatch\((const )?void \*self' src/*.c | wc -l) static, $(grep -E '^(static )?[a-z].*_dispatch\((const )?void \*self' src/*.c | wc -l) definitions"
echo "R3 rappels     : $(grep -cE '\(void \*e, void \*ctx' src/pico8_collect.h src/pico8_ptrs.h | awk -F: '{s+=$2} END{print s}') types de rappel ; static : $(grep -lE '^static int pico8_[a-z_]+_step\(void \*e|^static void pico8_la_ptrs_append\(void \*e' src/*.c | tr '\n' ' ')"
echo "R4 locale      : $(grep -n 'void (\*draw)(void \*self' src/pico8_entity.c | cut -d: -f1 | sed 's/^/entity.c:/')"
echo "R5 R2 amont    : $(sh tools/composition_residue.sh --detail src tests | grep '^void-ptr-r2' | grep -vc 'aucun') site(s) R2 sur 11 avec un *_dispatch en amont"
sh "$O/syn.sh" . > "$O/syn_base.txt"
sed -i '132,158s/(const void \*self/(const pico8_struct_header_t *self/; 132,158s/(void \*self/(pico8_struct_header_t *self/' src/pico8_struct.h
sh "$O/syn.sh" . > "$O/syn_table.txt"; git checkout -q src/pico8_struct.h
echo "R6 typer table : $(($(wc -l < "$O/syn_table.txt") - $(wc -l < "$O/syn_base.txt"))) erreur(s) dans $(cut -d: -f1 "$O/syn_table.txt" | sort -u | grep -vc raylib) fichier(s)"
python3 "$O/deadinc.py" . $(ls src/*.c src/*.h | grep -v raylib) > "$O/deadinc_all.txt"
echo "R7 includes    : $(grep -c MORT "$O/deadinc_all.txt") includes a 0 nom (majorant)"
echo "R8 lignes R2   : $(grep -c 'probe.c:349\|particle.c:152' tools/composition_r2_sites.txt) renvoi(s) date(s) dans composition_r2_sites.txt"
```

Sortie à `f` :

```
R1 table       : 14 membres void *self (dont pico8_state_fn)
R2 dispatch    : 19 publics (.h), 37 static, 56 definitions
R3 rappels     : 3 types de rappel ; static : src/pico8_collect.c src/pico8_planet.c src/pico8_planet_display.c
R4 locale      : entity.c:227
R5 R2 amont    : 6 site(s) R2 sur 11 avec un *_dispatch en amont
R6 typer table : 231 erreur(s) dans 41 fichier(s)
R7 includes    : 72 includes a 0 nom (majorant)
R8 lignes R2   : 4 renvoi(s) date(s) dans composition_r2_sites.txt
```

### Annexe B — diff de `CLAUDE.md` (texte exact)

```diff
--- a/CLAUDE.md
+++ b/CLAUDE.md
@@ -2,6 +2,6 @@
 
 ## Le portage
 
-- Cible active : **PC via raylib, rendu logiciel** — voir `docs/plan.md`. Portage littéral, fonction par fonction, de `pico8/lua.lua` (la source décompilée du cart), pas une réécriture depuis le document de conception. Les objets du jeu sont des structs C plates ; leur composition (phase 2) est en cours — voir `docs/plan-3.md`.
+- Cible active : **PC via raylib, rendu logiciel** — voir `docs/plan.md`. Portage littéral, fonction par fonction, de `pico8/lua.lua` (la source décompilée du cart), pas une réécriture depuis le document de conception. Les objets du jeu sont des structs C plates : un noyau commun (`pico8_struct_header_t`), un archétype `static const` par sorte, des composants par pointeur ; le plan actif est `docs/plan-3.md`.
 - PSX est **en pause**, pas abandonné : on y revient quand une version PC/C est jouable de bout en bout. `psx-hello/` est un projet de test du toolchain — voir `docs/psx-hello-world.md`.
 - L'audio (`sfx`, `music`) est **hors périmètre**, pas reporté.
 
