---
title: "Plan 3, étape 1b.0 : recensement des membres `pico8_value_t` des structs d'entité (outil seul), rejeu ASan+UBSan mesuré, script de sortie de 1b"
description: "La phase 1b va donner un type C à chacun des ~270 membres `pico8_value_t` des structs d'entité. Mais on ne sait pas, membre par membre, quels genres de valeur y passent, ni si un lecteur y voit nil. Sans ce savoir, on ne peut écrire ni la liste des étapes 1b, ni choisir la struct pilote, ni décider où poser un indicateur de présence. Cette étape livre des outils et aucun type : un build à drapeau qui recense les membres et produit une table versionnée, le rejeu ASan+UBSan du filet avec sa durée mesurée, et le script de sortie de 1b avec son compte initial."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.0 — recensement des membres (outil seul, aucune struct typée) ; table versionnée, durée du rejeu ASan+UBSan mesurée, script de sortie (puce **Sortie**) écrit et son compte initial noté.
>
> La suite de la liste est écrite après 1b.0, à partir de sa table.

Puces du §5 (« Phase 1b ») qui cadrent l'étape :

> - **Nil** : un indicateur de présence **uniquement** là où le code observe nil (test de vérité, `mv` qui rend 0 sur champ absent, affectation `= nil`). Rappel : en Lua `0` est vrai, donc un fixe à 0 n'est pas nil. Pas de valeur sentinelle.
> - **Recensement (1b.0)** : outil seul, aucune struct typée, sur le modèle de l'étape 0.2 et du build `-DPICO8_CLASS_COVERAGE`. Un build à drapeau recense, par (struct, membre), l'ensemble des genres de `pico8_value_t` écrits et si nil est observé par un lecteur (test de vérité, `mv` qui rend 0, `= nil`). Crochets : init des structs, écrivains `set_field`, lecteurs `*_truthy` et par nom, les ~24 sites à table d'offsets `*(pico8_value_t *)((char *)self + offset)`. Passé sur les rejeux de `FRAMES_EVENTS` et `make test`. Sortie : une table versionnée, et la durée du rejeu ASan+UBSan (§7). Un membre sans genre observé est signalé : son PRD ajoute une caractérisation dirigée avant de le typer.
> - **Pilote (1b.1)** : la plus petite struct dont la ligne de recensement mêle au moins un fixe simple, un membre à nil observé et un membre REF.
> - **Sortie** : aucune struct d'entité ne contient de `pico8_value_t` — constaté par une commande nommée : un script qui parcourt les typedefs de struct d'entité (premier membre `pico8_struct_header_t` ou struct d'entité enchâssée) et liste tout membre `pico8_value_t` ; sortie vide. Le même script sert de compteur de progrès à chaque étape.

Et le §7 :

> - **ASan+UBSan, en 1b** : chaque étape ajoute un build ASan+UBSan du binaire de rejeu, passé sur `FRAMES_EVENTS` ; critère : sortie propre, sans comparaison d'empreinte. Si la durée mesurée à 1b.0 est trop longue, seulement aux étapes qui touchent une table de membres ou une boucle d'init, et à la sortie de phase.
> - Chaque commit de 1a passe `make class-coverage-net` […]. En 1b, seulement les étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]` ; toujours à la sortie de phase.

### État de départ

HEAD 7b6f0e8. La phase 1a est sortie à 1a.42e (02705f2), note de sortie du §5 :

- toute entité est une struct à en-tête `pico8_struct_header_t` ;
- elle est lue par nom (`pico8_entity_get_field`/`set_field`) sur des membres `pico8_value_t` ;
- `make class-coverage-net` est identique à `tools/class_coverage_net_expected.txt` ;
- `make frames-check` donne 15 « conforme ».

Le compteur de couverture vit dans `src/pico8_coverage.c/.h`. C'est une unité vide hors `-DPICO8_CLASS_COVERAGE` : H11 de 1a.42, vérifiée sans avertissement sous les `CFLAGS` du projet. C'est le modèle direct du build à drapeau de cette étape.

**Rien n'a été compilé ni lancé pour ce PRD.** Les numéros de ligne et les comptes viennent de `grep`/`awk` sur 7b6f0e8. Tout fait d'exécution est une hypothèse (H1 à H22).

### Inventaire

#### 1. Les structs d'entité et leurs membres `pico8_value_t`

Mesure grossière (awk sur `typedef struct { … } pico8_X_t;` dans `src/*.h` et `src/*.c`, entité = premier membre `pico8_struct_header_t` ou struct d'entité enchâssée) : **~270 membres `pico8_value_t` dans ~41 typedefs** (H3).

| Typedef | Membres | Typedef | Membres |
|---|---|---|---|
| `pico8_jo_t` | 31 | `pico8_g__t` | 23 |
| `pico8_js_t` | 19 | `pico8_hp_t`, `pico8_mz_t` | 17 chacun |
| `pico8_jc_t` | 16 | `pico8_hj_t` | 13 |
| `pico8_hy_t`, `pico8_ga_t` | 11 chacun | `pico8_qh_t`, `pico8_fv_t`, `pico8_ui_t` | 10 chacun |
| `pico8_ie_t` | 8 | `pico8_iv_t` | 7 |
| `pico8_lq_t`, `pico8_pj_t` | 6 chacun | `pico8_bg_t`, `pico8_mh_t`, `pico8_qb_t`, `pico8_fq_t` | 5 chacun |
| `pico8_ie_ke_t`, `pico8_pj_cr_t` | 4 chacun | `pico8_nl_t`, `pico8_trade_t`, `pico8_ha_pz_t` | 3 chacun |
| `pico8_ke_t`, `pico8_pm_t`, `pico8_nr_t`, `pico8_fm_placeholder_t`, `pico8_jc_too_far_t`, `pico8_jc_mouse_point_t` | 2 chacun | `pico8_jb_t`, `pico8_nf_t`, `pico8_fr_t`, `pico8_mx_t`, `pico8_mz_jj_t`, `pico8_bv_et_t` | 1 chacun |

S'y ajoutent des typedefs sans membre `pico8_value_t` : `pico8_os_t` (= `{ pico8_qh_t qh; }`), les placeholders `mo`/`je`, `pico8_nr_ldown_empty_t`.

Pièges du parseur relevés à la mesure :

- `pico8_jo_fe_ctx_t` (`planet_display.c:357`) a pour premier membre `pico8_jo_t *self`. Un **pointeur** n'est pas un enchâssement : ce n'est pas une entité ;
- quatre typedefs à membres sont déclarés dans un `.c` : `pico8_bv_et_t` (`probe.c:50`), `pico8_jc_too_far_t` (`slipway.c:30`), `pico8_jc_mouse_point_t` (`slipway.c:75`) et `pico8_ha_pz_t` (`menu.c:32`), soit 8 membres. `offsetof` ne les voit pas hors de leur fichier ;
- `pico8_lq_t` et `pico8_mh_t` ont un membre `pico8_list_t items` (liste de valeurs). La puce **Sortie** ne compte que `pico8_value_t` : ce membre n'est pas compté (voir Further Notes) ;
- aucun déclarateur multiple (`pico8_value_t a, b;`) ni tableau `pico8_value_t x[N]` dans une struct d'entité aujourd'hui. Le script les accepte quand même (H3).

#### 2. Descripteurs et en-têtes

- 40 descripteurs `static const pico8_struct_class_t` (`g_X_struct_class`) dans 23 fichiers (`g_lq_struct_class` est déclaré deux fois, `layout.c:33,394`). 36 ont un accesseur public `pico8_X_struct_class()`. `jb`, `mh`, `iv` et `nr_ldown_empty` n'en ont pas ; `ke` n'a pas de descripteur (jamais construite).
- Correspondance descripteur → typedef : presque toujours `g_X_struct_class` ↔ `pico8_X_t`. Exceptions : `g_ie_ke_view_struct_class` ↔ `pico8_ie_ke_t` ; `g_os_struct_class` ↔ `pico8_os_t`, qui n'a pas de membre propre.
- **29 constructeurs** dans 19 fichiers, repérés par `pico8_class_coverage_record(` : `bg`, `economy`, `help`, `layout` ×2, `mouse_reader`, `msgbox` ×2, `planet`, `planet_family` ×6, `planet_family_trade` ×3, `probe`, `research`, `research_icon`, `score_panel`, `scroll`, `slipway`, `trade`, `ui`, `ui_button` ×2, `wormhole`. La 30ᵉ occurrence est la définition dans `coverage.c`.
- **11 objets hors constructeur** reçoivent le tag `PICO8_TYPE_TAG_STRUCT` :
  - statiques : `g_mo_placeholder` (`entity.c:55`), l'enveloppe `ha_pz` (`menu.c:68`), `g_nr_ldown_empty` (`planet_family_trade.c:103`, `const`), le `fm` provisoire (`probe.c:48`), `g_bv_et` (`probe.c:83`), l'enveloppe `pj_cr` (`score_panel.c:149`), `g_jc_too_far` (`slipway.c:69`), `g_je_placeholder` (`ui.c:66`), `mz_jj` (`wormhole.c:43`) ;
  - par appel : la vue `ie_ke` (`research_icon.c:101`, variable locale) et le `{ra,p}` de `jc:hq` (`slipway.c:190`, `malloc` à chaque appel).

#### 3. Lecture et écriture par nom

- `pico8_entity_get_field` (`entity.c:173`) et `pico8_entity_set_field` (`entity.c:189`) sont les deux seuls points d'entrée génériques. Les appels `->field(`/`->set_field(` hors d'`entity.c` sont tous des chaînages **à l'intérieur** du lecteur ou de l'écrivain d'une fille (`help.c:165,174`, `planet_family.c:106,132,236,245,469,479,746,756`, `planet_family_trade.c:263,278,541,550`). De même, les appels à `pico8_ke_field` sont tous dans le corps d'un lecteur. Seule exception : `research_icon.c:108-110` appelle `kind->field(…)`, qui lit les défauts d'un **genre** (`pico8_entity_kind_t`) et non une entité (H2).
- Passent donc tous par `pico8_entity_get_field` :
  - `pico8_mv` (`entity.c:265`), qui rend 0 quand `get_field` rend 0 ;
  - les tests de présence de `lt`/`he` (`if(e[p])`) ;
  - les quatre `field_truthy(inst, name)` (`economy_formulas.c:17`, `economy_score.c:37`, `economy_tiers.c:26`, `planet_display.c:37`) et `pico8_probe_field_truthy` (`probe.c:107`) ;
  - 105 appels directs dans `src/`.
- 18 appels à `pico8_entity_set_field` dans `src/`.
- Un lecteur de struct rend 0 quand le membre vaut nil (`REF` `NULL`) ; exemple, `pico8_jo_field`, `planet.c:199-209`.

#### 4. Sites à table d'offsets

Le plan en annonçait ~24. La mesure en trouve **39 dans 14 fichiers** (`grep '\.offset' src/*.c`, hors `offsetof`) :

| Rôle | Sites |
|---|---|
| lecture d'un membre par la table (lecteur ou `pico8_X_member`) | 17 |
| mise à nil de tous les membres à l'init | 7 |
| recopie des défauts à l'init (`g_X_defaults[i].value`) | 13 |
| clés de construction (`overrides->entries[k].value`) | 2 (`probe.c:553`, `planet_family.c:955`) |

Fichiers : `score_panel`, `planet`, `probe`, `research_icon`, `layout`, `ui_button`, `help`, `planet_family_trade`, `planet_family`, `ui`, `research`, `bg`, `wormhole`, `slipway`. Les tables de noms (`g_X_members`, `g_X_defaults`, `keys[]`) portent `{"nom", offsetof(pico8_X_t, m)}` (H1).

#### 5. Accès directs, hors du nom

Depuis 1a, le code des méthodes lit et écrit surtout les membres **directement** :

- ~225 écritures `->m = pico8_value_…(` ou `.m = pico8_value_…(` dans 27 fichiers ;
- plus les écritures d'une variable (`->m = v;`, ~45 sur les seuls noms `self`/`jo`/`qh`/`g_`/`e`) ;
- 30 appels `X_truthy(…->m)` sur un membre, dans 14 fichiers (les `*_truthy(pico8_value_t v)` sont des fonctions de valeur, qui ignorent le membre) ;
- ~50 tests en ligne `->m.as.ref == NULL` / `!= NULL`, dans 22 fichiers.

Aucun de ces sites ne passe par un point central.

#### 6. Build, filet et arbres d'objets

- L'outil de couverture est le modèle direct :
  - arbre `obj-coverage` compilé avec `COVERAGE_CFLAGS := -DPICO8_CLASS_COVERAGE` (`Makefile:83,120`) ;
  - binaire `$(TMP_DIR)/pico8_class_coverage` construit depuis `main_class_coverage.c` et `ENGINE_SRCS` (`:821`) ;
  - cibles `class-coverage`/`class-coverage-net` hors suite (`:830-836`).
- `ref-audit` (`:774-817`, `tools/ref_audit_report.sh`) est le modèle de la jonction d'une moitié statique (grep des sites) et d'une moitié dynamique (tallies par processus) : un site jamais atteint sort NOT COVERED au lieu de manquer en silence.
- Sept arbres d'objets aujourd'hui (`:64-73`, « Ne pas fusionner les sept »). Arbres existants : ASan (`obj-asan`, `test_pico8_planet_gh_row_lq`, lancé avec `ASAN_OPTIONS=detect_leaks=0`, `:664`) et UBSan (`obj-ubsan`, `test_pico8_fixed_ubsan`). Aucun ne combine les deux.
- Le binaire de rejeu :
  - `$(TMP_DIR)/pico8_replay` (`-O0`) et `pico8_replay_e2e` (`-O2`), depuis `REPLAY_SRCS` (`:487-491`) ;
  - `main.c` appelle `pico8__update60()` puis `pico8__draw()` à chaque frame (`main.c:95-96`) ;
  - argv : événements, capture, dump (`main.c:43-45`).
- 55 TU de `UNIT_TESTS` lient `ENGINE_SRCS` ou `DATA_ENGINE_SRCS`. 8 d'entre eux sont dans `UNIT_TESTS_SLOW`, restent **47**. `test_pico8_layout_nil_item_draws_nothing` lie une liste explicite sans `entity.c` (`:553`).
- `pico8_coverage.c` est dans `ENGINE_SRCS`, donc `CORE_SRCS` : `float-grep` et `float-guard` le lisent (`:139`).

## Solution

Quatre livrables, sans aucun membre typé ni changement de comportement du build normal :

1. **Le script de sortie de 1b**, `tools/entity_value_members.sh`, et la cible `make value-members`.
   - Il parcourt les typedefs de struct d'entité de `src/` et imprime une ligne par membre `pico8_value_t`, puis le compte.
   - Son compte initial est noté.
   - Le même parseur produit, dans un second mode, la table de membres qu'utilise le recensement : une seule définition de « quels membres existent ».
2. **Le build à drapeau `-DPICO8_MEMBER_CENSUS`** : un module `pico8_member_census.c/.h`, vide sans le drapeau comme `pico8_coverage.c`. Il observe les membres **à des points d'observation** au lieu de crocheter chaque écriture :
   - fin de chaque constructeur et de chaque objet statique ;
   - chaque lecture et écriture par nom (deux crochets centraux dans `entity.c`) ;
   - fin de chaque frame, sur toutes les entités de `hv`.

   Une moitié statique (grep) complète ce que l'exécution ne voit pas : les tests de nil et les `= nil` faits directement sur un membre.
3. **La cible `make member-census`** :
   - rejoue les 15 `FRAMES_EVENTS` et les 47 TU de `make unit` qui lient le moteur, dans un arbre à drapeau ;
   - replie les observations avec la moitié statique ;
   - vérifie l'intégrité de l'outil (preuves P1 à P4) ;
   - réécrit la table versionnée `tools/member_census.txt`.

   La table est une **entrée** de la liste 1b, pas un oracle.
4. **La cible `make sanitize-net`** : le binaire de rejeu construit sous ASan+UBSan, passé sur `FRAMES_EVENTS`, critère « sortie propre ». Sa durée est mesurée et elle tranche le mode du §7 pour la suite de 1b.

## User Stories

1. En tant qu'auteur de la liste 1b, je veux, pour chaque (struct, membre `pico8_value_t`), l'ensemble des genres de valeur qu'il a tenus, afin de choisir son type C.
2. En tant qu'auteur de la liste 1b, je veux savoir, pour chaque membre, si un lecteur a observé nil (test de vérité, `mv` qui rend 0, `= nil`), distinct du simple fait de tenir nil, afin de ne poser un indicateur de présence que là où le §5 le demande.
3. En tant qu'auteur de la liste 1b, je veux, pour un membre REF, la forme de la cible (liste, table ob, struct de tel typedef, autre), afin de savoir vers quel pointeur typé il va.
4. En tant qu'auteur de la liste 1b, je veux les membres sans aucun genre observé signalés, afin que leur PRD ajoute une caractérisation dirigée (§5).
5. En tant qu'auteur de la liste 1b, je veux savoir quels membres sont lus ou écrits par nom, afin de fixer la règle de coexistence sur des faits.
6. En tant que `plan-pilot`, je veux pouvoir désigner la struct pilote (§5 : un fixe simple, un membre à nil observé, un membre REF) par simple lecture de la table.
7. En tant que mainteneur, je veux une table versionnée au format ligne par ligne, stable d'une régénération à l'autre, afin que son diff soit lisible.
8. En tant que mainteneur, je veux régénérer la table par une seule cible `make`, hors de `make test` et `make e2e`.
9. En tant que mainteneur, je veux une passe réduite (un rejeu, sans TU, table écrite ailleurs), afin de vérifier une mutation de l'outil en moins d'une minute.
10. En tant que gardien de l'invariant §3, je veux que le build normal soit inchangé : `make test` vert, 15 conformes, aucun `*.sha256` touché, le drapeau absent de tous les arbres existants.
11. En tant que gardien du filet de couverture, je veux `make class-coverage-net` identique à sa référence après l'ajout des crochets dans les constructeurs.
12. En tant que vérificateur, je veux que l'outil prouve qu'il a vu chaque écrivain par nom, chaque constructeur, chaque objet statique et chaque site à table d'offsets, afin qu'un trou d'instrumentation fasse échouer la cible au lieu d'appauvrir la table en silence.
13. En tant que vérificateur, je veux que chaque ligne de la table corresponde exactement à une ligne du script de sortie (même parseur), afin qu'aucun membre n'échappe au recensement.
14. En tant que vérificateur, je veux qu'un descripteur rencontré au rejeu sans être rattaché à un typedef fasse échouer la passe des rejeux.
15. En tant qu'auteur de chaque étape 1b, je veux un script qui liste les membres `pico8_value_t` restants et leur compte, afin de mesurer le progrès et de constater la sortie de phase (sortie vide).
16. En tant qu'auteur de chaque étape 1b, je veux une cible `make sanitize-net` qui rejoue le filet sous ASan+UBSan, afin d'appliquer le critère « sortie propre » du §7.
17. En tant que `plan-pilot`, je veux la durée de `make sanitize-net` mesurée et le mode du §7 tranché (à chaque étape, ou seulement aux étapes qui touchent une table de membres ou une boucle d'init), afin que les PRD suivants n'aient pas à le redécider.
18. En tant que vérificateur, je veux que la fuite des entités jamais libérées (TROU-7) ne compte pas comme une sortie sale, afin que le critère ASan porte sur les erreurs mémoire.
19. En tant que lecteur du code, je veux que les crochets tiennent chacun sur une ligne qui ne compile à rien sans le drapeau, et que le module ne porte que ses contrats (règle des quatre destinations, CLAUDE.md).
20. En tant que lecteur du `Makefile`, je veux les trois nouvelles cibles dans la légende de tête.
21. En tant que lecteur du plan, je veux une note « Découvert à 1b.0 » qui donne le nom du script de sortie et son compte initial, le chemin de la table et le mode ASan retenu.
22. En tant qu'autopilote, je veux toute condition d'arrêt du §7 écrite comme critère `FAILED`, et aucune écriture de la liste 1b ni de la règle de coexistence dans cette étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur ; ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués (G).

### Ce qu'on observe, et où

- **(G) Observer les valeurs tenues, pas chaque écriture.** Les genres d'un membre sont ceux qu'il **tient** aux points d'observation O1 à O4 ci-dessous. Crocheter chaque écriture demanderait ~270 sites directs dans ~27 fichiers (inventaire 5), en plus des 39 sites à offsets. Ces sites seront tous réécrits par la phase 1b, parce que le compilateur refusera `self->m.as…` sur un membre typé.
  - L'observation lit la mémoire du membre, là où l'écrivent les sites à offsets, les écrivains par nom et les écritures directes. Aucun de ces écrivains n'a donc besoin d'un crochet propre.
  - Angle mort accepté et écrit en tête de la table : un genre écrit directement puis écrasé avant l'observation suivante (dans la même frame) n'est pas vu. Le PRD de chaque étape 1b relit ses sites, et le compilateur les lui désigne tous.
  - Rejeté : un macro d'écriture sur chaque site direct, qui touche ~30 fichiers pour un code que 1b réécrit.
- **Points d'observation** :
  - **O1**, *vu* (instantané complet de l'objet et de ses parents enchâssés) :
    - à la fin de chacun des 29 constructeurs, juste avant le `return` final, donc après les défauts, les clés de construction et `pq` ;
    - pour chacun des 11 objets hors constructeur, une fois ses membres posés : dans l'init de son module pour un statique, à chaque construction pour la vue `ie_ke` et le `{ra,p}` de `jc:hq`.

    Le crochet porte le nom du typedef : `PICO8_MEMBER_CENSUS_SEE(self, pico8_jo_t)`. Il apprend ainsi la correspondance descripteur → typedef à l'exécution, sans table tenue à la main ni lien vers les accesseurs des modules.
  - **O2**, *lecture par nom* : à la sortie de `pico8_entity_get_field`, après le lecteur de la struct, quand le nom n'est pas une méthode et que c'est un membre de la chaîne du typedef :
    - le lecteur a rendu 1 : le genre tenu est noté, ainsi que le drapeau « lu par nom » ;
    - le lecteur a rendu 0 et le membre vaut nil : **nil observé, preuve `lu`** ;
    - le lecteur a rendu 0 et le membre ne vaut pas nil : « lecteur muet », décompté dans le rapport.
  - **O3**, *écriture par nom* : à la sortie de `pico8_entity_set_field`. Le genre écrit est noté, ainsi que le drapeau « écrit par nom ». Une valeur nil vaut **nil observé, preuve `écrit`** : c'est le `= nil` du cart. Aucun constructeur ne passe par `set_field`.
  - **O4**, *fin de frame* : à la fin de `pico8__update60` et de `pico8__draw` (`cart.c`), un instantané de chaque entité de `hv`. `cart.c` passe `pico8_hv()` au module, qui ne dépend ainsi d'aucun autre module.
- **(G) Les sites à table d'offsets ne reçoivent pas de crochet.**
  - Leurs écritures (nil à l'init, défauts, clés de construction) précèdent toujours O1 dans le même constructeur.
  - Leurs lectures sont celles des lecteurs de struct, donc vues par O2.
  - La preuve qu'aucun n'échappe au recensement est P3 (ci-dessous). Ce PRD lit ainsi la puce « crochets : […] les ~24 sites à table d'offsets » du §5 ; la mesure trouve 39 sites (H1).
- **(G) Moitié statique pour ce que l'exécution ne voit pas.** Un test de vérité fait sur un membre lu directement ne passe par aucun point central : 30 `X_truthy(…->m)` et ~50 `->m.as.ref ==/!= NULL`. Il en va de même d'un `->m = pico8_value_ref(NULL)` direct. Le repli les trouve par grep dans `src/*.c` :
  - `_truthy\([^()]*(->|\.)m\)` ;
  - `(->|\.)m\.as\.ref (==|!=) NULL` ;
  - `(->|\.)m = pico8_value_ref\(NULL\)` hors d'une fonction dont le nom finit par `_struct_init`, `_new` ou `_struct_new`. Une mise à nil à l'init n'est pas un `= nil` : c'est « tenir nil ».

  Le site est rattaché **par nom** à chaque typedef qui déclare un membre `pico8_value_t` de ce nom : **nil observé, preuve `stat`**, ou `stat?` quand plusieurs typedefs portent ce nom (`ra`, `p`, `s`, `fo`, `me`, `od`, `nu`…). C'est volontairement une **sur-approximation**. Un indicateur de présence en trop coûte un booléen ; un indicateur manquant change un comportement. La liste des sites, par ligne de table, va dans le rapport non versionné.

  Pas de moitié statique pour les genres : l'angle mort ci-dessus est accepté.

### Définitions de la table (fixées ici, reprises en tête du fichier)

- **Genres** : `fixe`, `chaine`, `bool`, `fonction`, `ref:liste`, `ref:table`, `ref:<typedef>` (cible struct, typedef résolu par son descripteur), `ref:autre` (REF non NULL dont `pico8_type_tag_of` rend `UNKNOWN`, par exemple un descripteur de genre `pico8_entity_kind_t`). Nil n'est pas un genre.
- **Nil** : une valeur parmi trois.
  - `observé` : au moins une preuve `lu`, `écrit`, `stat` ou `stat?` ;
  - `tenu` : le membre a valu nil à un point d'observation, sans aucune preuve ;
  - `jamais`.

  **Tenir nil n'est pas observer nil** (consigne du §5) : les mises à nil de l'init donnent `tenu`, jamais `observé`.
- **Sans genre** : le champ genres est `-`. Le membre est signalé ; son PRD ajoute une caractérisation dirigée (§5).

### Table versionnée

- **(G) Emplacement** : `tools/member_census.txt`, à côté de `tools/class_coverage_net_expected.txt` et `tools/float_audit_c1.txt`, les autres données produites par un outil et versionnées.
- **Format** : texte, une ligne par (typedef déclarant, membre), colonnes séparées par une tabulation, dans l'ordre du script de sortie (fichier, puis ordre de déclaration). Lignes d'en-tête et de pied préfixées par `#` :

  ```
  # Recensement des membres pico8_value_t des structs d'entité (plan 3, 1b.0).
  # Produit par make member-census ; ne pas éditer à la main.
  # <rappel des définitions genres / nil / par-nom / accès, et de l'angle mort>
  # struct	membre	accès	genres	nil	preuves-nil	par-nom
  pico8_ke_t	me	offset	chaine	jamais	-	l
  pico8_jo_t	ep	offset	ref:table	observé	lu,stat	le
  pico8_bv_et_t	fo	nom	ref:pico8_jo_t	tenu	-	le
  …
  # lignes 270 ; sans genre N ; nil observé N ; nil tenu seulement N ; lecteurs muets N ; rejeux 15 ; TU 47
  ```

  (Valeurs de l'exemple illustratives.)
  - `accès` : `offset` si le typedef est déclaré dans un en-tête, donc instantané possible ; `nom` s'il est déclaré dans un `.c` (4 typedefs, 8 membres), donc observé seulement par O2/O3.
  - `genres` : ensemble trié, séparé par des virgules, ou `-`.
  - `preuves-nil` : sous-ensemble trié de `lu`, `écrit`, `stat`, `stat?`, ou `-`.
  - `par-nom` : `l`, `e`, `le` ou `-`.
- Un membre déclaré par un parent enchâssé est **une seule ligne**, celle du typedef qui le déclare. `pico8_jo_t.nu` agrège ce que tiennent les `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr` et `mx`. C'est l'unité de retypage de 1b : un membre est typé une fois pour toutes les descendantes.
- Les observations sont des **ensembles**, pas des comptes. L'union est indépendante de l'ordre et du parallélisme, et une double sortie d'un processus fils est sans effet. La table ne dépend que des rejeux et des TU, qui sont déterministes (H16).
- Le rapport détaillé, non versionné, va dans `tmp/census/report.txt` : sites statiques par ligne, attributions ambiguës, lecteurs muets, typedefs jamais vus dans `hv`, descripteurs inconnus de la passe des TU, durées.

### (G) La table n'est pas un oracle

La table est une **entrée** de l'écriture de la liste 1b (tailles, pilote, indicateurs de présence, membres à caractériser), rien de plus :

- elle n'entre ni dans `make test` ni dans `make e2e` ;
- aucune étape suivante n'a pour critère de la garder identique ;
- `make member-census` la **réécrit**, et son diff est informatif ;
- un PRD de 1b peut relancer la cible pour information ; les membres qu'il a typés disparaissent de la table d'eux-mêmes, puisque le parseur ne les voit plus.

La cible n'échoue que sur l'**intégrité de l'outil** (P1 à P4 et les runs), jamais sur un contenu. Rejeté : en faire un filet (`--expect`) comme `class-coverage-net`. La table change par construction à chaque étape 1b.

### Preuve que l'outil voit tout (vérifiée par le repli, échec de la cible sinon)

- **P1, écrivains et lecteurs par nom centralisés** (grep) :
  - tout appel à un `*_set_field` de struct ou à un pointeur `->set_field(` dans `src/` est soit dans `pico8_entity_set_field`, soit dans le corps d'une autre fonction `*_set_field` (chaînage) ;
  - même règle pour `*_field`/`->field(` et `pico8_entity_get_field`, avec `kind->field` (`pico8_entity_kind_t`, défauts d'un genre) pour seule exception nommée.

  Toute lecture ou écriture par nom passe donc par O2/O3. Cela couvre `mv`, `lt`/`he`, les `field_truthy` et les 18 `set_field`.
- **P2, constructeurs et objets statiques vus** :
  - (statique) chaque fonction qui contient `pico8_class_coverage_record(` contient aussi `PICO8_MEMBER_CENSUS_SEE(` après lui ;
  - (statique) chaque site hors assert qui pose `PICO8_TYPE_TAG_STRUCT` (les 17 lignes de l'inventaire 2, constructeurs de `ke`, `hy`, `lq`, `jb`, `hp`, `trade` compris) a un `SEE` pour son objet, dans la même fonction ou dans l'init de son module pour un statique de portée fichier ;
  - (dynamique) la passe des rejeux ne rencontre **aucun descripteur inconnu**. Dans la passe des TU, un descripteur inconnu est celui d'une struct locale à un test (`test_pico8_mo`…) : il est décompté dans le rapport et n'est pas une erreur.
- **P3, sites à table d'offsets** :
  - chaque entrée `{"n", offsetof(pico8_X_t, m)}` d'une table de `src/*.c` nomme un (typedef, membre) présent dans la table, directement ou par un parent enchâssé ;
  - le nombre de sites `.offset` (39) est imprimé dans le rapport, pour qu'une nouvelle forme se voie.
- **P4, lignes = script de sortie** : l'ensemble (typedef, membre) de la table est exactement la sortie de `tools/entity_value_members.sh` (même nombre, mêmes paires).
- **Runs** : chaque rejeu et chaque TU de la passe sort avec le code 0. Un TU rouge sous le drapeau veut dire que l'outil a changé un comportement.
- Rejeté : `gcov` comme preuve de crochetage. Les crochets sont en trois points centraux plus 40 lignes `SEE`, ce qu'un grep établit exactement, et P2 dynamique couvre le reste. Pas de `gcov` pour 1b.0 : aucune conversion (§7 « Preuve des tests » porte sur les lignes d'une conversion).

### Module et build à drapeau

- **Nouveau module `src/pico8_member_census.c/.h`**, sur le modèle de `pico8_coverage.c/.h` :
  - unité vide hors `-DPICO8_MEMBER_CENSUS` ;
  - dans `ENGINE_SRCS` et `DATA_ENGINE_SRCS` (donc `CORE_SRCS` : pas de flottant, pas de `double`, même dans le rapport) ;
  - l'en-tête est inclus par `pico8_struct.h`, comme `pico8_coverage.h`, pour que les 19 fichiers de constructeurs le voient sans nouvel `#include`.
- **Crochets en macros d'une ligne**, qui se développent en `((void)0)` sans le drapeau : `PICO8_MEMBER_CENSUS_SEE(obj, typedef)`, `PICO8_MEMBER_CENSUS_WALK(list)`, et deux appels dans `entity.c`. Contrairement aux `#ifdef` de trois lignes de la couverture, ils ne sont jamais dans un chemin compilé du build normal. Le build normal, `obj-coverage` et `obj-e2e` ne changent donc pas de comportement.
- **Stockage statique seulement** : tableaux dimensionnés par la table générée, aucun `malloc` dans les chemins instrumentés. La disposition du tas du jeu reste celle du build normal (H18). La sortie se fait par un `atexit` enregistré à la première observation, dans le fichier `$PICO8_MEMBER_CENSUS_OUT/<pid>.txt` ouvert en ajout. Sans cette variable, rien n'est écrit. Un processus qui finit par `abort` ou `_exit` (fils de `must_abort`) n'écrit rien, sans perte pour l'union.
- **Table de membres générée** : `tools/entity_value_members.sh --c-table` écrit un en-tête C dans l'arbre du recensement (sous `tmp/census/`), jamais dans `src/`. Il contient :
  - les `#include` des en-têtes qui déclarent un typedef d'entité ;
  - une ligne par typedef (nom, parent enchâssé ou aucun) ;
  - une ligne par membre (typedef, nom, `offsetof`, ou la marque « par nom » pour un typedef déclaré dans un `.c`).

  Le build normal ne lance jamais le générateur. Seul l'objet de `pico8_member_census.c` de l'arbre à drapeau en dépend.
- **Résolution d'un nom** : on remonte la chaîne du typedef, du plus dérivé vers `ke`, et le premier qui déclare le nom l'emporte (H19). Un premier membre enchâssé est à l'offset 0, donc `offsetof(parent, m)` vaut sur un pointeur de la fille. Un descripteur vu avec deux typedefs différents est une erreur.

### Cibles `make`

- **`make value-members`** : lance `tools/entity_value_members.sh`.
  - Il imprime `fichier:ligne: typedef membre` par membre, puis `membres pico8_value_t : N` ;
  - il sort toujours 0 : c'est un compteur. La sortie de phase constate « aucune ligne de membre ».
  - Règles du parseur :
    - `typedef struct { … } pico8_X_t;` dans `src/*.h` et `src/*.c` ;
    - entité si le premier membre est `pico8_struct_header_t` ou un typedef d'entité **non pointeur** (point fixe) ;
    - membres `pico8_value_t a;`, déclarateurs multiples et tableaux acceptés (un tableau est noté `a[]`) ;
    - commentaires et lignes vides ignorés.
- **`make member-census`** (hors suite) :
  1. génère la table de membres ;
  2. lance un `make` récursif avec `TMP_DIR=tmp/census` et `CFLAGS` augmenté de `-DPICO8_MEMBER_CENSUS` (et de ce qu'il faut pour trouver l'en-tête généré), avec `PICO8_MEMBER_CENSUS_OUT` posé. Ce `make` construit et lance :
     - (a) `pico8_replay_e2e` de l'arbre à drapeau sur chacun des 15 `FRAMES_EVENTS`, une cible par rejeu pour que `-j` les parallélise (capture `/dev/null`, dump dans `tmp/census/`, `--from-title` pour `RECORDINGS`), sorties dans `raw/replay/` ;
     - (b) les `unit-run-*` de `CENSUS_UNIT_TESTS`, sorties dans `raw/unit/` ;
  3. lance `tools/member_census.sh`, qui fait la moitié statique, le repli, P1 à P4, et écrit la table et le rapport.

  Variables :
  - `CENSUS_EVENTS` (défaut `$(FRAMES_EVENTS)`) ;
  - `CENSUS_UNIT` (défaut `$(CENSUS_UNIT_TESTS)`, vide pour sauter les TU) ;
  - `CENSUS_TABLE` (défaut `tools/member_census.txt`).

  Une passe réduite pour une mutation s'écrit `make member-census CENSUS_EVENTS=tests/fixtures/slipway_drag_events.json CENSUS_UNIT= CENSUS_TABLE=tmp/census/m.txt`. En passe réduite, P4 reste vérifiée.
- **`CENSUS_UNIT_TESTS`** : liste explicite versionnée dans le `Makefile`, les **47 TU** de `UNIT_TESTS` dont la règle lie `ENGINE_SRCS` ou `DATA_ENGINE_SRCS`, moins `UNIT_TESTS_SLOW`. C'est la lecture de « et `make test` » du §5 : les autres TU ne construisent pas d'entité (outils, `fixed`, `list`, `ob`…) ou lancent des binaires par un chemin `tmp/` en dur (`demo`, `frames_tool`, `headless_capture`). Les 8 TU lents sont hors de `make test`. Un TU de la liste qui ne se construit pas ou ne tourne pas sous le drapeau en est retiré, avec sa raison dans le commit (H7).
- **`make sanitize-net`** (hors suite) :
  - nouvel arbre `obj-sanitize` compilé avec `$(ASAN_CFLAGS) $(UBSAN_CFLAGS) $(E2E_CFLAGS)`, donc le `-O2` des rejeux du filet ;
  - le commentaire des arbres passe à « huit » ;
  - binaire `$(TMP_DIR)/pico8_replay_sanitize` depuis `REPLAY_SRCS` ;
  - une cible par rejeu de `FRAMES_EVENTS`, lancée avec `ASAN_OPTIONS=detect_leaks=0`, capture `/dev/null`, dump dans `tmp/sanitize/` ;
  - sortie : `sanitize: 15 propre(s)`, ou le rejeu et le rapport du sanitizer, code non nul.
  - **(G) `detect_leaks=0`** : les entités ne sont jamais libérées (TROU-7, Plan 2 §8 hors périmètre). Même réglage que `unit-run-test_pico8_planet_gh_row_lq`. Le critère porte sur les erreurs mémoire et l'UB.
  - **(G) `-O2`** : c'est le niveau des rejeux de `make e2e`, et il est déjà propre sous `-Werror` sans sanitizer (H12).
- **Légende de tête du `Makefile`** : `value-members`, `member-census` et `sanitize-net`, chacun marqué « hors suite ».

### Mode ASan du §7 pour la suite de 1b

- **(G) Seuil de « trop longue »** : la durée murale de `make sanitize-net` sur un arbre déjà construit (rejeux seulement, `-j` par défaut), **au-delà de 5 minutes**, le quart du budget de tests d'un passage.
  - Au-dessous : chaque étape 1b lance `make sanitize-net`.
  - Au-dessus : seulement les étapes qui touchent une table de membres ou une boucle d'init, et la sortie de phase.
- La décision et les deux durées (construction à froid, rejeux) vont dans la note « Découvert à 1b.0 ».

### Ce qui ne change pas

- Aucun membre, aucun lecteur, aucun écrivain, aucun descripteur ne change.
- Aucun `KNOWN[]`, `tools/class_coverage_*`, `*.sha256`, `FRAMES_EVENTS`/`RECORDINGS` ne change.
- Les seules lignes ajoutées dans `src/` hors du nouveau module sont :
  - les 40 `SEE` ;
  - les deux crochets d'`entity.c` ;
  - les deux `WALK` de `cart.c` ;
  - l'inclusion dans `pico8_struct.h`.
- **(G) `make class-coverage-net` est lancé** : le diff touche les 29 constructeurs (§7, « En 1b, seulement les étapes dont le diff touche un constructeur »). Le crochet ne compile à rien dans l'arbre de couverture, donc la sortie doit être identique.
- **TROU** : aucun écart port/cart n'est touché. `TROU-7` justifie `detect_leaks=0`.
- **Commentaires** (CLAUDE.md) :
  - contrats seulement dans `pico8_member_census.h` : ce que fait `SEE` et quand l'appeler (membres posés, un typedef par descripteur), la variable d'environnement, l'ajout par pid, stockage statique sans libération ;
  - l'aide à la lecture de chaque recette dans le `Makefile` ;
  - le reste va dans le message de commit.

### Hypothèses à vérifier à l'exécution (rien n'a été lancé pour ce PRD)

- **H1** : les sites à table d'offsets sont exactement les 39 de l'inventaire 4 (aucune autre graphie, par exemple une conversion sur deux lignes), et toutes les tables de noms ont la forme `{"n", offsetof(pico8_X_t, m)}` que P3 lit.
- **H2** : P1 tient sur 7b6f0e8 : aucun appel de lecteur ou d'écrivain de struct hors d'`entity.c` et des corps de chaînage, `kind->field` excepté.
- **H3** : le compte initial du script de sortie est proche de **270 membres dans ~41 typedefs**. La mesure grossière comptait `pico8_jo_fe_ctx_t` comme entité à tort (premier membre pointeur, 0 membre). L'implémenteur note le compte exact. Un écart de plus de ~10 % demande une ligne d'explication dans le commit.
- **H4** : les 29 constructeurs et les 11 objets de l'inventaire 2 sont tous les endroits où une struct d'entité reçoit son en-tête : P2 dynamique trouve 0 descripteur inconnu dans les rejeux.
- **H5** : les 4 typedefs déclarés dans un `.c` ne sont atteints que par nom. Leurs 8 lignes `nom` ont des genres observés par O2/O3.
- **H6** : les typedefs jamais vus dans `hv` (attendus : `hp`, `trade`, `lq`, `hy`, les statiques) ne sont observés qu'à leur construction et par nom. Le rapport les liste. Leurs genres écrits directement après la construction tombent dans l'angle mort, à noter pour leur PRD.
- **H7** : le `make` récursif avec `TMP_DIR` et `CFLAGS` en ligne de commande construit et lance les 47 TU et `pico8_replay_e2e` dans `tmp/census/` sans toucher `tmp/obj*`. Les TU spéciaux (`score_panel_stars` à deux arguments, `planet_gh_row_lq` en arbre ASan avec `detect_leaks=0`) gardent leur recette. Un TU qui ne lie pas sous le drapeau sort de `CENSUS_UNIT_TESTS`, avec sa raison.
- **H8** : `-I`/`-D` sur une ligne d'édition de liens sont sans effet (précédent : `REFAUDIT_CFLAGS` sur les liens de `ref-audit`).
- **H9** : `pico8_member_census.c` vide hors drapeau compile sans avertissement sous `-std=c99 -Wall -Wextra -Werror`, et passe `float-grep`/`float-guard` (précédent : H11 de 1a.42).
- **H10** : `main.c` accepte `/dev/null` comme capture et un dump par rejeu (`main.c:43-45`).
- **H11** : sans `detect_leaks=0`, LeakSanitizer signale les entités jamais libérées (TROU-7) ; avec, les 15 rejeux sortent propres. À vérifier une fois, sur le rejeu le plus court, dans les deux réglages.
- **H12** : l'arbre `obj-sanitize` (`-O2` + ASan + UBSan) compile sous `-Werror`. Si un avertissement n'apparaît qu'avec l'instrumentation, l'arbre seul reçoit `-Wno-error=<cet avertissement>`, avec une ligne dans le commit. S'il révèle un défaut réel, c'est H13.
- **H13** : les 15 rejeux sont propres sous ASan+UBSan sur l'arbre de 1b.0. Sinon : critère `FAILED` (le critère ASan de chaque étape 1b deviendrait inatteignable), avec le rapport, sans correction dans cette étape.
- **H14** : durée de `make sanitize-net`, rejeux seuls, de l'ordre de 2 à 3 fois le plus long rejeu `-O2` de `make e2e`. Le seuil de 5 min tranche.
- **H15** : durée de `make member-census`, construction de l'arbre comprise : quelques minutes. Le surcoût d'O2 (recherche linéaire sur ≤ 40 descripteurs et ≤ ~60 noms par appel) et d'O4 (toutes les entités de `hv` à chaque frame, deux fois) reste modéré.
- **H16** : deux passes complètes sur le même arbre donnent la même table. Pas de relance exigée dans le budget ; à vérifier si le temps le permet, sinon noté comme non vérifié.
- **H17** : `pico8__update60` et `pico8__draw` sont appelés une fois chacun par frame, par `main.c` et par les TU qui font tourner des frames.
- **H18** : aucun comportement du jeu ne dépend des adresses du tas. Le module n'alloue rien avant `atexit`, et les TU donnent le même verdict sous le drapeau.
- **H19** : aucun membre ne masque un membre de même nom d'un parent enchâssé. S'il y en a un, le plus dérivé l'emporte et le rapport le liste.
- **H20** : les motifs de la moitié statique trouvent ~30 `_truthy` et ~50 `as.ref` sur des noms de membres. Le rapport donne leur nombre et la part `stat?` (ambiguë).
- **H21** : toute cible de REF tenue par un membre est un objet vivant d'au moins 4 octets, que `pico8_type_tag_of` peut lire. Un descripteur de genre donne `ref:autre`.
- **H22** : la note de sortie de 1a (02705f2, 7b6f0e8) est l'état de départ ; aucun changement non commité n'est en cours.

## Implementation Chunks

**Emprise estimée** :

- 5 fichiers nouveaux : `pico8_member_census.c/.h`, `tools/entity_value_members.sh`, `tools/member_census.sh`, `tools/member_census.txt` ;
- 23 fichiers de `src/` modifiés : `pico8_struct.h`, `entity.c`, `cart.c`, les 19 fichiers de constructeurs et `menu.c` (statique `ha_pz`) ;
- `Makefile` et `docs/plan-3.md`.

Soit **~30 fichiers**, sous le seuil de ~40 : **pas de découpage**. Un seul commit, un seul passage. Ordre de travail conseillé, chacun vérifiable seul :

### A. Script de sortie (`tools/entity_value_members.sh`, `make value-members`)

- Mode liste et mode `--c-table`.
- À vérifier à la main :
  - `pico8_jo_fe_ctx_t` n'est pas une entité ;
  - `pico8_os_t` l'est, sans membre ;
  - les 4 typedefs de `.c` sont marqués « par nom ».
- Compte initial noté (H3).

### B. `make sanitize-net`, puis la mesure

- Arbre, binaire, cibles par rejeu, légende.
- Construire d'abord le binaire (durée à froid notée), puis `time make sanitize-net` (durée des rejeux) ; H11 sur le rejeu le plus court.
- Peut tourner en arrière-plan pendant C.

### C. Module et crochets

- `pico8_member_census.c/.h`, inclusion par `pico8_struct.h`, `ENGINE_SRCS`/`DATA_ENGINE_SRCS`.
- O2/O3 dans `entity.c`, O4 dans `cart.c`, les 40 `SEE`.
- `make test` vert : le drapeau est absent, rien ne change.

### D. `make member-census`

- `CENSUS_UNIT_TESTS`, `make` récursif, `tools/member_census.sh` (moitié statique, repli, P1 à P4).
- Première passe complète, qui écrit la table versionnée.
- Mutations en passe réduite.

### E. Docs

- Légende du `Makefile`.
- Note « Découvert à 1b.0 » dans `docs/plan-3.md` : nom et commande du script de sortie avec son compte initial ; chemin et format de la table ; `make member-census` ; `make sanitize-net` et le mode ASan retenu.

**Coupe de repli**, seulement si le budget est épuisé avant D : A + B forment un commit « 1b.0a » (script de sortie, rejeu ASan+UBSan, mesure), outillage pur sans ligne de `src/`. C + D + E font « 1b.0b ». L'implémenteur note la coupe et s'arrête ; `plan-pilot` reprend avec le reste de ce PRD.

## Testing Decisions

Un bon test, ici, juge l'outil par ce qu'il produit : la table, le rapport, le code de sortie de la cible. Il ne juge pas la disposition interne du module. L'étape n'ajoute **aucun TU** :

- le build normal ne compile aucune ligne de l'outil ;
- l'intégrité de l'outil est vérifiée à chaque passe par P1 à P4 et par les runs ;
- les mutations prouvent que ces contrôles mordent.

Prior art :

- `make class-coverage-net` et `test_class_coverage` : build à drapeau, compteur vide hors drapeau, script de repli ;
- `make ref-audit` : jonction d'un grep des sites et des tallies par processus, NOT COVERED plutôt que silence ;
- `unit-run-test_pico8_planet_gh_row_lq` : ASan avec `detect_leaks=0`.

Règles :

- **Aucun test existant ne change.** Aucune empreinte, aucun `#define …_HASH`, aucune valeur attendue. Un TU qui rougit sous le drapeau est un défaut de l'outil, à corriger dans l'outil.
- **Mutations** (§7 : 2 à 3), faites dans une copie jetable du scratchpad, restaurées, puis `cmp` contre l'arbre. Chacune en passe réduite (un rejeu, sans TU, table dans `tmp/`) :
  1. **O2 retiré** (le crochet de `pico8_entity_get_field` ne note plus rien) : sur le rejeu choisi, au moins une ligne à preuve `lu` seule passe de `observé` à `tenu` ou `jamais`, et sa colonne `par-nom` perd `l`. Preuve que le crochet central alimente la colonne nil.
  2. **Un `SEE` de constructeur retiré** (celui de `hp`, jamais dans `hv` d'après H6) : la cible échoue par P2 statique. Avec le contrôle statique neutralisé dans la copie, la table perd les genres de `pico8_hp_t`. Preuve que O1 est la seule source pour un typedef hors `hv`.
  3. **Le mode liste du script de sortie oublie les parents enchâssés** (seuls les typedefs à `pico8_struct_header_t` en tête) : la cible échoue par P4. Preuve que table et script de sortie ne peuvent pas diverger.

  Facultative, seulement si le budget le permet : une écriture d'un octet hors d'un `malloc` dans une copie, sur le rejeu le plus court, doit rendre `make sanitize-net` rouge.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 minutes de temps total de tests** pour le passage. Ordres de grandeur (1a.39 à 1a.42) :

| Cible | Durée |
|---|---|
| `make test` | de 20 s à 1 min |
| `make e2e` | 4 à 6 min |
| `make class-coverage-net` | 3 à 4 min |
| `make member-census` | à mesurer (H15) |
| `make sanitize-net` | à mesurer (H14) |

Répartition indicative :

- `make value-members` (quelques secondes) ;
- `make test` 2 fois (après C, puis en fin) ;
- construction puis passe de `make sanitize-net` une fois (B), en arrière-plan de C si possible ;
- `make member-census` complet une fois, plus 3 passes réduites pour les mutations ;
- `make e2e` et `make class-coverage-net` une fois en fin. Ils peuvent tourner dans la même invocation : arbres distincts.

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, ou applique la coupe de repli, plutôt que de relancer.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + « chunk X appliqué » / « mutation N, copie jetable ») | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Exemples de ligne clé : `membres pico8_value_t : 270`, `sanitize: 15 propre(s)`, `# lignes 270 ; sans genre … ; rejeux 15 ; TU 47`, `frames-check : 15 conformes`, `couverture: OK -- … (15 rejeu(x))`, ou la ligne P1 à P4 qui échoue.

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, toutes vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : `frames-check` rend **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make class-coverage-net` : sortie identique à `tools/class_coverage_net_expected.txt`, qui ne change pas ;
4. `timeout 15m make member-census` : code 0, P1 à P4 tenues, 15 rejeux et 47 TU (ou la liste réduite justifiée) à code 0, 0 descripteur inconnu dans les rejeux, `tools/member_census.txt` réécrit et commité ;
5. `timeout 15m make sanitize-net` : `sanitize: 15 propre(s)`, durées notées ;
6. `make value-members` : compte initial noté.

Jamais l'appli raylib. `make raylib` n'est pas concerné : `main_raylib` n'est pas touché, et `pico8_member_census.c` y est vide.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

Conditions d'arrêt du §7 :

- le filet reste rouge après **3 tentatives** de correction sur le même commit ;
- le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS`/`RECORDINGS` ;
- un champ ne tombe sous aucune règle du §5. Lecture en 1b.0 : l'outil rencontre une écriture par nom acceptée par l'écrivain d'une struct sur un nom qui n'est la ligne d'aucun membre `pico8_value_t` de sa chaîne. Cet écrivain écrit alors ailleurs que dans un membre, et le §5 n'a pas de règle pour ce cas ;
- un test de régression de bug devrait changer **d'intention** pour passer (ici, n'importe quelle modification d'un test existant) ;
- la porte de couverture échoue : `make class-coverage-net` n'est pas identique à `tools/class_coverage_net_expected.txt`, un compte change, `KNOWN[]` change, ou le nombre de rejeux lus n'est pas 15.

Critères propres à l'étape :

- un membre d'une struct d'entité change de type, ou un lecteur, un écrivain, un descripteur ou une table de membres change (l'étape est « outil seul, aucune struct typée ») ;
- une ligne ajoutée dans `src/` hors de `pico8_member_census.c/.h` fait autre chose que les crochets listés (`SEE`, `WALK`, les deux crochets d'`entity.c`, l'inclusion dans `pico8_struct.h`), ou un crochet compile à autre chose que rien sans `-DPICO8_MEMBER_CENSUS` ;
- `make member-census` ou `make sanitize-net` entre dans `make test` ou `make e2e` ;
- `make sanitize-net` n'est pas propre sur l'arbre de l'étape (H13) : arrêt, le rapport du sanitizer joint, rien de corrigé ;
- la cible `make member-census` finit en succès alors que P1, P2, P3 ou P4 est fausse (contrôle neutralisé), ou alors qu'un rejeu ou un TU de la passe sort non nul ;
- `tools/member_census.txt` est édité à la main, absent du commit, ou ses lignes ne sont pas exactement la sortie de `make value-members` ;
- le module alloue sur le tas dans un chemin instrumenté (H18) ;
- le diff écrit la liste des étapes 1b.1 et suivantes, la règle de coexistence ou le choix du pilote : hors périmètre, c'est l'écriture de la liste qui les fait à partir de la table ;
- manque l'un de ces éléments : la note « Découvert à 1b.0 » (script, compte initial, table, mode ASan), les deux durées de `sanitize-net`, la durée de `member-census`, les trois mutations, le tableau « Runs ».

## Out of Scope

- Typer un membre, poser un indicateur de présence, emballer un lecteur : 1b.1 et suivantes.
- Écrire la liste des étapes 1b, mesurer l'empreinte en fichiers de chaque struct (puce **Liste** du §5), choisir le pilote, fixer la règle de coexistence. C'est l'écriture de la liste, par un agent `guideline`, à partir de la table.
- Les membres `pico8_list_t items` de `lq` et `mh`, que le script de sortie ne compte pas (Further Notes).
- Crocheter chaque écriture directe, ou un banc de mutations propre à l'outil.
- Les TU lents (`UNIT_TESTS_SLOW`) dans la passe du recensement : ils ne sont pas dans `make test`.
- Libérer les entités (TROU-7, Plan 2 §8), donc chercher les fuites sous ASan.
- Corriger un rapport du sanitizer.
- Tout changement de rendu ou de comportement, et l'audio.

## Further Notes

- Numéros de ligne et comptes relevés sur 7b6f0e8 par `grep`/`awk`. Rien n'a été compilé ni lancé.
- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules et tests avec l'utilisateur), tranchées en `guideline` :
  - modules : un module de recensement, deux scripts `tools/`, trois cibles ;
  - tests : aucun nouveau TU, contrôles d'intégrité P1 à P4 et trois mutations ;
  - pas de découpage.

  Les rejets sont notés à chaque décision (G).
- **Pour l'écriture de la liste** :
  - la colonne `accès` dit quels membres ne sont observés que par nom ;
  - le rapport dit quels typedefs ne passent jamais dans `hv` (angle mort des écritures directes après construction) ;
  - `lq.items` et `mh.items` (`pico8_list_t` de valeurs) échappent au script de sortie tel que le §5 le définit. Si 1b doit les typer, c'est une lecture nouvelle de la puce **Sortie**, à trancher à l'écriture de la liste.
- **Le script de sortie** sert ensuite de compteur de progrès à chaque étape 1b (§5). Son compte avant et après va dans chaque message de commit.
- **Contrôle d'étape** : un sous-agent frais relit le diff contre ce PRD avant le commit (hors budget de tests).
- Le message de commit porte :
  - le périmètre et les fichiers touchés ;
  - les décisions (G) et leurs rejets ;
  - le tableau « Runs » résumé ;
  - les durées et le mode ASan ;
  - le compte initial du script de sortie ;
  - le résumé de la table (lignes, sans genre, nil observé, lecteurs muets) ;
  - les mutations tuées ;
  - les réponses à H1 à H22 ;
  - la note « Découvert à 1b.0 ».
