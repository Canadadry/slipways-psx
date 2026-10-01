---
title: "Plan 4, étape 4d — Planètes et relais en génération de partie, sites par bd réécrits en place ou en zone frame, arène de prise recyclée"
description: "Les structures de planète (jo, qh, nl, os, hj, pm, nr, fr, mx), leurs boîtes p/ra, leur identifiant « p… », leurs listes oe/by/cc/jy et les copies de besoins sont encore prises à la libc ou à la transitoire. 4d les met dans la génération de partie, sauf ce qui est refait par action : cc est vidée en place, les listes lp/rl des propositions de jo:ib vont en zone frame, et la structure d'une prise d'icône refusée rend son arène à la prise suivante. Elle mesure l'empreinte de la plus grosse famille et refait la projection du budget."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. L'étape 4 range les objets de partie dans la **zone partie**. Cette PRD traite la sous-étape **4d, planètes et relais**.

Elle a été rédigée pendant 4c, avec des hypothèses H-4c-1 à H-4c-6. **4c est finie** : morceau 1 8e75265, morceau 2 4530656, clôture f87d7a8. Toutes les hypothèses ont été vérifiées à `HEAD` f87d7a8 (section « Vérifié contre 4c »), et les sections qui en dépendaient sont réécrites sur les faits. Le prototype de mesure a été refait sur f87d7a8.

**Le texte de l'étape**, recopié du plan à f87d7a8 :

> 4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement, et si son remplissage ne monte pas pendant une phase sans action d'un e2e.
>    Elle se fait en sous-étapes, une PRD chacune :
>    - **4a. Socle.** La zone partie, vide, avec sa remise à zéro et son empoisonnement sur nouvelle partie, son accesseur de remplissage (octets pris, et maximum, §3) écrit par le rejeu, et l'inventaire du §4 des pointeurs qui survivent à une nouvelle partie. Aucun propriétaire converti.
>    - **4b et suivantes.** Une famille de propriétaires par PRD dans l'ordre fixé par la PRD 4a : 4b états de `pico8_gen.c` ; 4c tableaux croissants et registres (crée la sous-zone chargement) ; 4d planètes et relais ; 4e slipways et routes ; 4f trous de ver ; 4g recherche et économie ; 4h singletons de `jl`/`eu` et objets du joueur (famille ajoutée par 4a : les « Reçu » des étapes 1 à 3 leur reviennent). N'entre en zone partie qu'une allocation faite une fois par objet de partie ; un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
>    - *Découvert (4a) :* la partie bascule en tête de `pico8_eu`, avant son `jl()` ; `_init` (titre, « restart », « again? ») ne bascule pas. L'ancienne génération est rendue au vidage suivant (en jeu, l'update qui suit le clic), après un point d'oubli de partie, vide à ce jour : rendre dans `jl()` laissait lire `ok:lj` sur l'ancien `ok`, la file, `fm`/`ju`. Zone à deux bouts, un seul stockage de 768 Ko ; une double bascule sans vidage est refusée ; `cart_load`/`shutdown` remettent tout à zéro. Un champ de plus dans `pico8_zone_choisie_t` grossissait chaque en-tête d'arène et faisait tomber `qy_fv` : la poignée de partie vit à part. Angle mort ASan : un pointeur gardé à travers deux bascules ne plante plus. Inventaire du §4 refait (éclats `ga` compris) : rien ne lit l'ancienne génération après son rendu. Les 81 relocations de l'étape 4 sont attribuées aux familles 4b–4h ; la seule marge est de 49 Ko sur la somme des zones, la zone particules gardant ses ~17 Ko libres pour les traînées `ts`. Le rejeu écrit `partie` et `max_octets partie`.
>    - *Reçu de l'étape 4a :* les traînées `ts` de `jc:ch` (par frame) vont à 4e, en zone particules, places de 24 o.
>    - *Découvert (4b) :* `pico8_jf`/`pico8_gn` prennent la zone de l'appelant ; `bw` fait ses états en zone partie après la bascule, `oh` de `bg:pq` va en zone frame. Le libérateur n'était pas `jo:pq` (comme le disait la table de 4a) mais `jo_set_oc`/`_ir`/`_shutdown`. Seuls `g_jo_oc`/`g_jo_ir` visent un état, repointés à chaque `eu` : point d'oubli toujours vide. Mesure : 608 o par partie, maximum 1216 au clic de `pause_restart` (deux générations) ; projection ≈ 262 Ko sous 768, on continue. Comptes : 24 / 84 → 23 / 80.
>    - *Découvert (4c) :* la zone partie se partage en génération (704 Ko, deux bouts) et chargement (64 Ko, remis à zéro au seul chargement), somme toujours 768 Ko ; `g_shared_le` et `g_ga_by_shared` vont en chargement (0 o mesuré : rien ne les fait grandir en jeu). Les registres `hv`/`cv`/`cg` vivent une génération (refaits par `jl`), pas le chargement. Piège : la remise à zéro faisait relire `hv` par le `jl()` suivant — `cart_load`/`shutdown` abandonnent `hv` avant le vidage et oublient les registres sans les lire après. Blocs morts des agrandissements bornés (prise = G + R exact). Mesure : génération 34 240 o au plus (clic de `pause_restart`), chargement 0 ; projection P ≈ 33 + 209 × 1,52 ≈ 351 Ko ≤ 704, Q ≈ 49 Ko ≤ 64 : on continue. Comptes : 23 / 80 → 22 / 75.
>    - *Reçu de l'étape 4c :* pour 4g, la sous-zone chargement existe (`pico8_zone_chargement()`, 64 Ko, `chargement`/`max_octets chargement` écrits par le rejeu) : `gb` et `hp.i` y entrent sans la recréer.
>    - *Reçu de l'étape 1 :* même chose pour les boîtes `vec` des objets de partie (`p`/`v`/`ra` de `qb`, `ra` de `nf`, `mo->v`, `re`/`p` de `jc:hq`, `p` de `pico8_bv`, `ie.gy`) ; `qb:ph` seul en fait trois par frame. L'état `oh` de `pico8_bg_pq` est un objet de frame fait par `pico8_jf`, qui ne prend pas de zone : l'étape 4, qui donne une zone aux états de `pico8_gen.c`, le range en zone frame.
>    - *Reçu de l'étape 2 :* restent à cette étape `ok.qa` (gardé par les hôtes de `bv` et le point souris de `jc:hq` : objet relu par des objets de partie), la structure `ok` et ses calques `ic`, la liste statique de départ, `ie.jb` (fait une fois par partie), `fv`, la structure `pj`, `hp.i`. Les structures de planète ne sont toujours pas libérées (seule leur rangée l'est).
>    - *Reçu de l'étape 3 :* la structure `g_` (et `fv`) et sa boîte `p` de `bv` restent à cette étape (`/* TROU-7 */`) ; `oe` d'une planète ordinaire passe par `pico8_jo_pq_dans`, qui prend déjà sa zone.

La ligne « *Reçu de l'étape 4c* » vise 4g, pas 4d.

**La ligne « Partie » du §2**, recopiée :

> | **Partie** | planètes, relais, slipways, routes `trade`, recherche, trous de ver ; les tableaux croissants (`ents`, `ptrs`, registres) qui les contiennent ; les objets faits au chargement (`gb`, `hp.i`, listes statiques) dans une sous-zone chargement, prise dans la zone partie, remise à zéro par le seul chargement | une génération par partie, de `eu` à `eu` ; rendue d'un bloc au vidage de la file qui suit le `eu` suivant (zone à deux bouts) ; `_init` ne la rend pas |

**La ligne de 4d dans la table des familles de la PRD 4a**, retirée par b46601e (`git show b46601e~1:docs/prd/triage/phase1-etape4a-socle-zone-partie.md`), recopiée :

> | Ordre | Famille | Sites (fonction, ligne) | Relocations | Reçus traités |
> |---|---|---|---|---|
> | **4d** | **Planètes et relais** | `pico8_jo_struct_new` (102), `pico8_jo_pq` (192 : la transitoire passée à `pico8_jo_pq_dans`), `pico8_jo_lz_mw_step` (219), `pico8_jo_lz` (242, 244, 246) ; `pico8_qh/nl/os/hj/pm_struct_new` (62, 123, 167, 247, 316) ; `pico8_nr/fr/mx_struct_new` (`planet_family_trade` 191, 237, 331) ; `pico8_jo_cn` (127, 129), `pico8_jo_ib` (188, 189, 215, 216, 218, 219) ; `pico8_bw` (63, 65, 71 : `p`, `ra`, identifiant « p… ») ; `bd_reset_visit` (48, 51, 52) ; refait le relevé « éclats `ga` rendus en cours de partie » pour `jo:hu` | `planet` 6 → sort ; `planet_family` 5 → sort ; `planet_family_trade` 3 → sort ; `planet_trade` 8 → sort ; `galaxy` 3 → sort ; `economy_tiers` 3 → sort (28) | *Reçu 2* : les structures de planète ; *Reçu 3* : `oe` d'une planète ordinaire par `pico8_jo_pq_dans` (il suffit de lui passer la zone partie) |

La raison de l'ordre, recopiée de la même PRD :

> **4d** est le plus gros poste (planètes générées par `bw(90)`, `bd` par action). On le mesure tôt, pour que la projection du budget repose sur une mesure et non sur une estimation.

**Les lignes de 4d dans la table « Sites qui allouent par tick ou par action » de la PRD 4a**, recopiées avec la règle qui les précède :

> **Règle pour toutes les familles :**
>
> - n'entre en génération de partie qu'une allocation faite une fois par objet de partie créé (planète posée, slipway, route, icône, singleton de `jl`/`eu`) ;
> - un champ réaffecté par tick ou par action doit, au choix de la PRD de sa famille et site par site :
>   - être **réécrit en place**, quand la lecture prouve qu'aucun autre objet ne garde l'ancienne valeur ;
>   - devenir une **valeur** au lieu d'une boîte, quand la boîte n'est jamais modifiée en place. Le cart en modifie trois en place : `self.ra.y+=` (l. 578), `p.v.x=`/`p.v.y=` (l. 699-700), `self.p.x=`/`self.p.y=` (l. 1141) ;
>   - aller en **zone frame**, s'il n'est relu que dans la frame ;
>   - prendre une **place** dans une zone qui rend.
>
> | Site | Fréquence | Famille |
> |---|---|---|
> | `bd_reset_visit` (`pt.cc={}` pour chaque planète, `free`+`malloc`) | à chaque `bd`, donc chaque `hy:lj` (action, année, invention), en boucle tant que `bb()` | 4d |
> | `pico8_jo_ib` (`ib`, `lp`, `rl`) | à chaque `bd`, pour chaque planète | 4d |
> | `pico8_jo_cn` (liste fusionnée) | à chaque calcul de route | 4d |
> | `pico8_jo_lz`, `pico8_jo_lz_mw_step` (copies de `hp.i`) | à la pose et à chaque amélioration d'une planète | 4d |

**L'estimation de 4a pour 4d** : « 90 planètes × ~0,9 Ko (struct, `p`/`ra`/id, `oe`, copies `jy`, `cc`/`by`/`ib` une fois les sites par action réglés) ≈ 80 Ko ; les éclats `ga` n'y sont plus (zone particules) », pic estimé à la bascule **~160 Ko**.

**La méthode de suivi du budget**, recopiée de la PRD 4a :

> 1. Après sa conversion, rejouer les 26 e2e avec `PICO8_REMPLISSAGE`. Relever le maximum de la zone partie sur l'ensemble (`M_k`), et celui de la sous-zone chargement à partir de 4c. Le noter dans le message du commit et dans « Découvert ». La forme est laissée à l'implémenteur : boucle de shell dans le commit, ou cible `make` hors suite comme `class-coverage`.
> 2. **Projection** : `P_k = M_k + Σ` (estimations des familles restantes) `× r`. `r` vaut le rapport « mesuré / estimé » des familles déjà faites, s'il dépasse 1, et 1 sinon. Une famille dont la mesure dépasse son estimation fait monter la projection des suivantes.
> 3. **Décision** :
>    - `P_k ≤ PICO8_ZONE_GAME_BYTES` (768 Ko, sous-zone chargement comprise) : on continue ;
>    - `768 Ko < P_k ≤ 768 Ko + marge`, avec marge = 1536 − (frame + interface + particules) = **49 Ko** à e88df7a : la famille relève `PICO8_ZONE_GAME_BYTES` d'un chiffre, dans son commit, et le dit. Les ~17 Ko libres dans les 128 Ko de la zone particules n'entrent pas dans cette marge : ils restent aux particules (traînées `ts` de 4e), et les reprendre serait rogner une autre zone (§3). Si 4e a besoin de relever la zone particules pour `ts`, ce relèvement est pris sur les 49 Ko, et la projection des familles suivantes le compte ;
>    - `P_k` au-delà : **arrêt** de l'étape 4 à la famille k, et signalement à l'humain (zone partie, limite voulue = `P_k` arrondi, somme actuelle des zones). On ne rogne ni la frame, ni l'interface, ni les particules (§3), et on n'attend pas la dernière famille.
> 4. **Croissance** : à chaque famille, le TU des places vivantes vérifie que les octets de la zone partie sont égaux aux frames 45 et 90 de `message_box_hold`. Une hausse est une allocation par tick de la famille. Elle se corrige dans la famille, pas en relevant la limite (§3).
> 5. Un `pico8_fatal` « zone partie pleine » en cours de famille se lit d'abord comme un site par tick ou par action entré tel quel, puis comme une limite trop basse. La note « Découvert » dit lequel.
>
> La sous-zone chargement suit la même méthode à part. Son maximum ne dépend pas de la bascule.

Depuis 4c, génération (704 Ko) et chargement (64 Ko) se projettent à part. Faire passer la frontière de l'une à l'autre est un découpage interne à la zone partie, pas un relèvement.

**Ce qui se passe aujourd'hui** (lu à f87d7a8) :

- **Les structures.** Neuf constructeurs font une structure `pico8_jo_t` (440 o sur x86-64) par `malloc` : `jo` (planètes de `bw`), `qh`, `nl`, `os`, `hj`, `pm` (`pico8_planet_family.c`), `nr`, `fr`, `mx` (`pico8_planet_family_trade.c`). Ils appellent tous `pico8_jo_pq`, qui passe la transitoire à `pico8_jo_pq_dans` pour faire `oe`. L'éclat `ga` est déjà entier dans une place de la zone particules (étape 3) et passe sa place à `pico8_jo_pq_dans`.
- **Qui les crée, et quand.**
  - `bw(90)`, dans `eu` après la bascule : 90 planètes `jo`, chacune avec une boîte `p`, une boîte `ra` (`p*128`) et un identifiant « p… » (`pico8_concat2` sur la transitoire).
  - `ie:ldown` (cart l. 932, `pico8_ie_ldown`) : une structure (`qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`) **à chaque pression du bouton sur une icône**, avec un identifiant « s… » (famille 4g). `ui:bh` la tient (`ok.lheld`), et, au relâcher, appelle son `ey`, c'est-à-dire `d_` (cart l. 569, `pico8_d_`). `d_` la pose (`ks(dq)`, donc « ly », et `fm:nx`) si elle n'a pas de cible `et` et si `fm:lu` la paie. Sinon il la refuse : son 57 et `self.p_=true`, l'état reste « kz ». La structure refusée passe ensuite par `dg`, `he`, la file, et son `rendre`.
  - `jo:hu` avec `hp.cw` : la planète remplacée reçoit `p_`, une nouvelle structure (planète, ou éclat `ga`) **emprunte** `p`, `ra` et `id` de l'ancienne.
- **`jo:lz`** fait `jy` : une liste `pico8_ptrs_t` par `malloc`, ses éléments sur la transitoire, et une copie `pico8_jy_t` (48 o) par besoin de `hp.i` (`lz_mw_step`, `malloc`). Appelants : `jo:pq` (structure de lignée `qh` dont l'archétype a `hp`) et `jo:hu` (planète « known » qui devient « ly »).
- **`jo:cn`** fait, sous `if t.qe==r.e and not self.jh`, une liste fusionnée (`pico8_la_ptrs` sur la transitoire, conteneur par `malloc`) qui remplace `self.jy`, puis pose `jh`. `has_jh` n'est remis à 0 que par l'init de la structure : **une fois par planète**, pas « à chaque calcul de route » comme le disait la table de 4a.
- **`jo:ib`** rend `self.by or {}` : le conteneur `ib` (`malloc`, transitoire) n'est fait que si `by` est nil, et `bd_route_visit` le range aussitôt dans `jo->by`. **Une fois par planète.** En revanche, pour **chaque** slipway de `oe`, à chaque `bd`, il fait deux listes `lp` et `rl` (`malloc` + transitoire) pour la proposition `lw.t`.
- **`bd_reset_visit`** fait `pt.cc={}` pour chaque planète de `cg.jo`, à chaque `bd` : `pico8_ents_free` + `free` de l'ancienne liste, puis `malloc` + transitoire.
- **`bw`** : `p`, `ra` par `malloc`, l'identifiant sur la transitoire.
- **Ce que 4c a laissé dans la génération** : les états de `bw` (`G` = 608 o) et les 16 registres (`R`, blocs morts compris). Après f13 de `pause_restart`, `prise = G + R = 608 + 16 224` exactement. `pico8_entity_abandonner_hv()` et `pico8_entity_oublier_registres()` encadrent le reset de `cart_load`/`cart_shutdown`. `oublier_partie()` (`pico8_cart.c`) est toujours vide.
- **Le verrou** à f87d7a8 : **22 fichiers, 75 appels**. Les 28 relocations de 4d, rattachées par `objdump -r` et `addr2line` sur une copie `git archive HEAD` (lignes identiques à 8e75265 : 4c n'a touché aucun de ces fichiers après son morceau 1) :

  | Fichier | Fonction (ligne à f87d7a8) | Relocations |
  |---|---|---|
  | `planet` | `pico8_jo_struct_new` (102), `pico8_jo_pq` (178), `pico8_jo_lz_mw_step` (205), `pico8_jo_lz` (228, 230, 232) | 6 |
  | `planet_family` | `pico8_qh/nl/os/hj/pm_struct_new` (63, 124, 168, 248, 317) | 5 |
  | `planet_family_trade` | `pico8_nr/fr/mx_struct_new` (191, 237, 331) | 3 |
  | `planet_trade` | `pico8_jo_cn` (127, 129) ; `pico8_jo_ib` (188, 189 : `ib` ; 215, 216, 218, 219 : `lp`, `rl`) | 8 |
  | `galaxy` | `pico8_bw` (59, 61, 67) | 3 |
  | `economy_tiers` | `bd_reset_visit` (48, 51, 52) | 3 |
  | | | **28** |

**Le problème.** 4d est la plus grosse famille, et la seule qui mélange trois fréquences :

1. Des objets faits **une fois par objet de partie** : structures de `bw` et de `jo:hu`, structures posées, boîtes et identifiant de `bw`, `oe`, `by`, `jy`, copies de besoins. Ils entrent tels quels en génération.
2. Deux sites **par `bd`**, donc par action, en boucle tant que `bb()` : `cc` et les `lp`/`rl` des propositions. Mis tels quels en génération, ils la font grandir à chaque action. Le prototype de cette PRD le mesure : `cc` refaite en génération à chaque `bd` porte `game_end_final_score` de 48 848 à 105 008 o, et `trade_rich_mine` de 51 896 à 136 832 o (mesure à 8e75265, sans les registres).
3. Un site **par action** que la table de 4a ne nommait pas : la **prise d'icône refusée**. Chaque pression sur une icône fait une structure et ses listes (488 à 744 o mesurés selon la sorte), et un refus la laisse morte. Mise telle quelle en génération, elle la ferait grandir à chaque refus : **+616 o par prise refusée** d'un labo, mesuré sur 20 cycles. C'est contraire à la règle de la famille : l'allocation est faite par action, pas une fois par objet de partie qui vit.

## Solution

- **Toutes les structures de planète en génération** : les neuf constructeurs prennent la zone de construction, qui est la génération courante hors d'une prise d'icône. `pico8_jo_pq` passe cette zone à `pico8_jo_pq_dans` (Reçu 3). L'éclat `ga` ne change pas (zone particules). Une planète remplacée par `jo:hu` reste allouée et morte dans la génération, comme aujourd'hui dans la libc (TROU-7) : son remplacement est une action payée, qui crée un objet de partie.
- **`bw`** : `p`, `ra` et l'identifiant « p… » en génération.
- **`jo:lz`** : la liste `jy`, son conteneur et les copies de besoins dans la zone de construction.
- **`jo:cn`** : la liste fusionnée et son conteneur en génération (une fois par planète, prouvé par lecture).
- **`jo:ib`** : `ib` en génération (une fois par planète). `lp` et `rl` des propositions **en zone frame** : seule la proposition les désigne, et `trade:kg` en recopie les éléments.
- **`bd_reset_visit` réécrit en place** : `cc` est faite une fois par planète, en génération, puis **vidée sans rendre sa capacité** à chaque `bd`. Une opération « vider » est ajoutée à `pico8_ents`.
- **Arène de prise, recyclée** : `ie:ldown` construit sa structure, son `oe`, son `jy` et ses copies dans une **arène** prise à la génération. Posée, la structure garde son arène jusqu'au rendu de la génération : c'est un objet de partie, fait une fois. Refusée, son `rendre` remet l'arène dans une courte liste de **prises libres**, et la prise suivante la vide et la réutilise. Le point d'oubli de partie vide cette liste : c'est sa première entrée.
- **Inventaire du §4** refait pour les planètes et l'arène de prise.
- **Budget** : mesuré par le prototype refait à f87d7a8 sur les 26 e2e. `M_4d` = **130 720 o = `M_4c` + 96 480 o** exactement, au clic de `pause_restart`. L'estimation de 4a (~160 Ko pour 4d) était deux fois trop haute. Projection **≈ 177 Ko** sous 704 : **on continue**.
- **Verrou** : six fichiers sortent. Prévision **22 / 75 → 16 / 47**, mesurée par le prototype.

Aucun pixel ne change, aucun tirage `rnd` n'est déplacé : le prototype garde les 26 `*.sha256` et `make sanitize-net` vert.

## User Stories

1. En tant que mainteneur du plan, je veux que les neuf structures de planète (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`) soient prises dans la génération de partie, pour que la plus grosse famille quitte la libc d'un bloc.
2. En tant que développeur, je veux que `oe` d'une planète ordinaire passe la zone de construction à `pico8_jo_pq_dans`, et que l'éclat `ga` garde sa place de la zone particules, pour que le *Reçu de l'étape 3* soit traité sans toucher à l'étape 3.
3. En tant que développeur, je veux que `bw` fasse `p`, `ra` et l'identifiant « p… » dans la génération ouverte par `eu`, pour qu'ils vivent exactement autant que la planète qui les garde.
4. En tant que développeur, je veux que `jo:lz` fasse `jy`, son conteneur et ses copies de besoins dans la zone de construction, parce que `lz` n'est appelé qu'une fois par création (`jo:pq`) et une fois par colonisation (`jo:hu` sur une planète « known »).
5. En tant que développeur, je veux que la liste fusionnée de `jo:cn` aille en génération, parce que `has_jh` la limite à une fois par planète, pour ne pas inventer une place pour un site qui n'est pas par action.
6. En tant que développeur, je veux que `jo:ib` ne fasse `ib` qu'une fois par planète, en génération, parce que `bd` range le résultat dans `by` et que `jo:ib` le réutilise ensuite.
7. En tant que développeur, je veux que les listes `lp` et `rl` de chaque proposition de `jo:ib` aillent en zone frame, parce qu'elles ne sont désignées que par la proposition, morte à la fin de l'itération, et que `trade:kg` en recopie les éléments.
8. En tant que développeur, je veux que `bd` vide `cc` en place au lieu de la refaire, parce que seul `jo->cc` la désigne, pour que les années et les actions ne fassent plus grandir la génération.
9. En tant que développeur, je veux une opération de `pico8_ents` qui vide une liste en gardant ses éléments alloués et sa capacité, pour que « réécrire en place » ne pique pas dans le champ `count` depuis un autre module.
10. En tant que développeur, je veux que la structure, le `oe`, le `jy` et les copies d'une prise d'icône soient faits dans une arène prise à la génération, pour qu'un refus puisse rendre tout l'ensemble d'un coup à la prise suivante.
11. En tant que développeur, je veux qu'une structure refusée remette son arène dans la liste des prises libres à son `rendre`, et pas avant, pour qu'aucun `dg`, `he`, `rendre` ni `oublier` ne lise une structure déjà réutilisée.
12. En tant que développeur, je veux que la prise suivante vide et réutilise une arène libre avant d'en prendre une nouvelle, pour que des prises refusées répétées ne fassent plus grandir la génération.
13. En tant que développeur, je veux qu'une structure posée garde son arène jusqu'au rendu de sa génération, et qu'un `rendre` de structure posée (état « ly », abandon par `jl()`) ne recycle rien, pour qu'un objet de partie vivant ne soit jamais réutilisé.
14. En tant que développeur, je veux que le point d'oubli de partie et la remise à zéro du chargement et de l'arrêt vident la liste des prises libres, pour qu'une prise de la nouvelle partie ne reprenne jamais une arène de la génération rendue.
15. En tant que développeur, je veux une opération de la zone générique qui vide une arène en place (son bloc empoisonné sous ASan), pour recycler une arène sans primitive de zone nouvelle autre que celle-là.
16. En tant que développeur, je veux que l'ancienne planète remplacée par `jo:hu` reste allouée dans la génération, pour que l'éclat ou la planète qui emprunte ses `p`, `ra` et `id` lise de la mémoire vivante jusqu'au rendu.
17. En tant que développeur qui chasse un accès hors durée de vie, je veux qu'une planète de la partie quittée meure par ASan quand on la relit après le rendu de sa génération, pour prouver qu'elle est bien dans la génération.
18. En tant que développeur, je veux que la planète de la partie quittée se relise encore pendant la frame du clic qui ouvre la nouvelle partie, pour que la fin de `ok:lj` et les `rendre` de la file lisent des planètes vivantes.
19. En tant que mainteneur, je veux qu'après f13 de `pause_restart` les octets de la génération valent exactement `G + R + P`, avec `P` l'empreinte des planètes calculée à partir de `sizeof` et des capacités publiques de leurs listes, pour qu'aucun autre objet n'y soit entré par erreur.
20. En tant que mainteneur, je veux que les octets de la génération, registres déduits, soient égaux avant et après les 25 sauts d'année de `game_end_final_score`, pour prouver que `bd` ne fait plus grandir la zone.
21. En tant que mainteneur, je veux la même égalité autour du saut d'année de `trade_rich_mine`, où `bd` parcourt de vraies routes (`jo:ib`, `jo:cn`, `lp`/`rl`), pour prouver que les sites par `bd` ne grandissent pas non plus quand des routes existent.
22. En tant que mainteneur, je veux un rejeu qui enchaîne vingt prises refusées et dont la génération, registres déduits, ne bouge plus après la première, pour prouver que la prise d'icône respecte la règle de la famille.
23. En tant que mainteneur, je veux qu'une prise posée après des refus prenne exactement une arène de plus, pour prouver qu'une structure posée n'est pas recyclée.
24. En tant que mainteneur, je veux que le relevé f45 = f90 de `message_box_hold` reste égal avec les planètes dans la zone, pour prouver qu'aucune allocation de planète n'est faite par tick.
25. En tant que gardien du cliquet, je veux que `planet`, `planet_family`, `planet_family_trade`, `planet_trade`, `galaxy` et `economy_tiers` sortent de la liste sans qu'aucun compte ne monte, pour que le verrou reste à égalité stricte.
26. En tant que gardien de l'invariant, je veux les 26 `*.sha256` inchangés et `make sanitize-net` vert, pour prouver que vider `cc` en place, mettre `lp`/`rl` en zone frame et recycler l'arène de prise ne changent rien au jeu.
27. En tant que mainteneur du budget, je veux `M_4d` relevé sur les 26 e2e et comparé à la prévision du prototype (`M_4c` + 96 480 o sur `pause_restart`), pour remplacer la plus grosse estimation de 4a par une mesure.
28. En tant que mainteneur du budget, je veux la projection refaite en deux comptes (génération ≤ 704 Ko, chargement ≤ 64 Ko) avec la décision écrite, pour appliquer l'arrêt anticipé du §3 sur la plus grosse famille.
29. En tant que développeur des familles suivantes, je veux l'inventaire du §4 refait pour les planètes et l'arène de prise, le relevé « éclats `ga` » compris, pour savoir ce que le point d'oubli de partie coupe désormais et pourquoi.
30. En tant que développeur de 4e, je veux une ligne « *Reçu de l'étape 4d* » qui dise que la proposition `lw.t` de `jo:ib` a ses `lp`/`rl` en zone frame et que sa structure `trade`, faite à chaque `bd`, ne doit pas entrer en génération, pour que 4e ne la range pas avec les routes gardées dans `by`/`cc`.
31. En tant que développeur de 4g, je veux une ligne « *Reçu de l'étape 4d* » qui dise que l'identifiant « s… » de `ie:ldown` est fait à chaque prise et doit aller dans l'arène de prise, pour qu'il suive la structure au lieu de grandir la génération à chaque refus.
32. En tant que lecteur du plan, je veux que la note « Découvert (4d) » corrige la table de 4a (`jo:cn` et `ib` sont une fois par planète ; seuls `cc` et `lp`/`rl` sont par `bd` ; la prise refusée est par action), pour que les familles suivantes partent des fréquences vraies.
33. En tant que mainteneur des commentaires, je veux que les seuls commentaires touchés soient les contrats C devenus faux (« a fresh heap container » de `pico8_jo_ib`, « not freed here » de `bw`, « nothing ever frees it » de `pico8_jo_new`), les contrats des opérations nouvelles et les `assert` porteurs de message, pour respecter les quatre destinations du `CLAUDE.md`.

## Implementation Decisions

### Inventaire des sites de 4d, et zone retenue

Chaque site par `bd` ou par action est tranché par lecture : « qui d'autre garde l'ancienne valeur ? ».

| Site | Ligne à f87d7a8 | Fréquence (lecture) | Qui garde l'objet | Zone retenue |
|---|---|---|---|---|
| structure `jo` | `planet` 102 | une par planète de `bw` | registres, `ok`, panneaux, slipways, routes, file | **génération** |
| structures `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` faites par `jo:hu` (`cw`) | `planet_family` 63, 124, 168, 248, 317 ; `planet_family_trade` 191, 237, 331 | une par remplacement payé | idem | **génération** |
| les mêmes, faites par `ie:ldown` | idem | **une par pression sur une icône** ; posée ou refusée par `d_` | idem, puis plus rien après le `rendre` d'une refusée (voir « Arène de prise ») | **arène de prise**, prise à la génération, gardée si posée, recyclée si refusée |
| `oe` | `planet` 178 (`pico8_jo_pq`) | une par structure | la structure | **zone de construction** (génération, ou arène de prise) |
| `p`, `ra`, identifiant « p… » | `galaxy` 59, 61, 67 | une par planète de `bw` | la planète ; empruntés par la remplaçante de `jo:hu`, par un éclat `ga`, par les `iv`/rangées qui la désignent | **génération** |
| `jy`, conteneur, copies de besoins | `planet` 205, 228, 230, 232 | `jo:pq` (une par structure de lignée `qh` avec `hp`) et `jo:hu` (une par colonisation) | la planète ; une rangée de panneau peut désigner une copie | **zone de construction** |
| liste fusionnée de `jo:cn` et son conteneur | `planet_trade` 127, 129 | **une par planète** : `has_jh` n'est remis à 0 que par l'init de la structure | la planète (`self->jy`) | **génération** |
| `ib` de `jo:ib` | `planet_trade` 188, 189 | **une par planète** : fait si `by` est nil, rangé dans `jo->by` par `bd_route_visit` | la planète (`by`) | **génération** |
| `lp`, `rl` de la proposition `lw.t` | `planet_trade` 215, 216, 218, 219 | par `bd`, par planète, par slipway de `oe` | la seule proposition `lw.t`, locale à l'itération ; `trade:kg` recopie ses éléments par `pico8_ents_cat` ; `jo:cn` et `nr:cn` ne gardent que le résultat de `kg` | **zone frame** |
| `cc` | `economy_tiers` 48, 51, 52 | par `bd`, par planète | seulement `jo->cc` : lu par `dh` et `bd_route_visit`, jamais recopié dans un champ (`grep` de `->cc`) | **réécrite en place** : faite une fois en génération, vidée à chaque `bd` |

- **Pourquoi pas « valeur ».** Aucun site de 4d n'est une boîte `vec` réaffectée. Le seul champ modifié en place, `ra.y` par `na` (l. 578), est celui d'une boîte de génération qui ne change jamais d'adresse.
- **Pourquoi la zone frame pour `lp`/`rl`.** `bd` tourne dans un `hy:lj`, donc dans un update, entre deux remises de la zone frame. La proposition est une variable locale de `jo:ib`. Son `t` (structure `trade`, famille 4e) n'est rangé nulle part : `pico8_jo_ib_kf_closure` et `nr:cn` ne gardent que `t:kg(…)`, qui recopie `lp`/`rl`. La zone frame couvre leur durée de vie (§4) ; ASan la tranche si la lecture se trompe.
- **Pourquoi vider `cc` en place est exact.** Le cart fait `pt.cc={}` dans une première boucle sur `cg.jo`, puis remplit les `cc` dans une seconde. Une table neuve ou une table vidée ne se distinguent que si un autre objet garde l'ancienne. Le `grep` de `->cc` ne trouve que l'init (`NULL`), `dh` et `bd_route_visit`, qui relisent toujours `jo->cc`. Le prototype garde les 26 `*.sha256`.
- **`by`/`cc` d'une structure tenue.** `bd` ne suit que `hy:lj`, appelé par `hy:nx` et `hy:hk` après une action terminée (pose, colonisation, slipway, année, invention). La souris tient l'icône pendant la prise : aucun `bd` ne tombe entre la pression et le relâcher. Une structure refusée n'a donc jamais de `by` ni de `cc`. Une structure posée les reçoit au `bd` que déclenche le `fm:nx` de sa pose, en génération (une fois).

### `pico8_ents` : vider sans rendre

- Une opération ajoutée au module `pico8_ents` (nom laissé à l'implémenteur) : le compte passe à 0, `items`, la capacité et la poignée ne changent pas. Les ajouts suivants réutilisent la capacité ; `pico8_ents_grow` ne copie qu'au-delà du pic déjà atteint.
- `pico8_ents_free` ne convient pas : sur une poignée de génération, `release` ne rend rien et la liste repartirait de 4, ce qui laisserait un bloc mort à chaque `bd`.
- Contrat à écrire dans l'en-tête : la liste vidée garde ses éléments alloués ; elle ne rend rien à sa zone.
- `bd_reset_visit` : si `jo->cc` est NULL, il la fait en génération (conteneur et poignée), sinon il la vide. L'inclusion de la transitoire et `<stdlib.h>` sortent de `pico8_economy_tiers.c` s'ils ne servent plus.

### Arène de prise : recycler la structure d'une prise refusée

**Lecture.** Chaque pression sur une icône construit une structure et son lot (struct, `oe`, et pour une structure à besoins `jy`, son conteneur et une copie de 48 o par besoin). Le prototype mesure ce lot à **488 à 744 o** selon la sorte (labo 616–648, `synthesizer` 488, `ascension` 744), identifiant « s… » non compris. `ui:bh` la tient puis appelle `d_` au relâcher. `d_` la pose (« kz » → « ly », `fm:nx`) ou la refuse (`p_`, l'état reste « kz »). Le jeu n'interdit pas de presser et refuser autant de fois qu'on veut.

**Les trois options demandées, tranchées par lecture :**

- *La structure est-elle en fait gardée ?* **Non.** Refusée, elle a `p_`. Le `dg` suivant la retire (`he`) et la met en file, le vidage suivant appelle son `rendre`, et `oublier` coupe `ok.o_`/`ok.gj`/le `re` d'un `jc` tenu. `ok.lheld` est déjà remis à nil par `ui:bh` après `ey`. Les seuls autres pointeurs vers elle sont des champs `fo` :
  - `bv` (hôte tenu) écrit `et.fo = self` sur la planète ou le slipway visé, à chaque update de la prise ;
  - `jo:kj` et `jc:kj` (cart l. 549, 680) lisent `fo` puis le remettent à nil **dans le draw de la même frame** ;
  - `g_bv_et.fo`/`g_jc_too_far.fo` sont écrits seulement ;
  - la cible `et` est choisie sous la souris (`ij` sur `cg.np` à moins de `dw`, ou un slipway près du point), donc à l'écran et dessinée.

  Plus rien ne lit la structure après son `rendre`.
- *Lui donner une place dans une zone qui rend ?* **Non.** Une structure posée vit toute la partie : sa place ne serait jamais rendue en cours de partie, et le nombre de places devrait borner le nombre de structures posées. On ne peut pas non plus la déplacer à la pose, puisque `ok`, la file et les registres la désignent déjà.
- *Recycler la structure abandonnée ?* **Oui, par une arène.** C'est la décision.

**Décision.**

- **Zone de construction.** Les constructeurs de planète, `pico8_jo_pq` et `jo:lz` allouent dans une zone de construction (nom laissé à l'implémenteur). C'est la génération courante, sauf pendant l'appel `new_entity` de `ie:ldown`, où c'est l'arène de prise. `jo:hu`, `bw` et `jo:cn` restent en génération. La forme est laissée à l'implémenteur : statique du module planète posée et reposée par `ie:ldown` autour de `new_entity` (forme du prototype), ou champ de zone dans `pico8_jo_keys_t`. Dans ce second cas, chaque site qui remplit des clés à la main (`jo:hu`, `bw`, l'enveloppe de `ga`, les TU) doit le poser.
- **L'arène.** Un bloc de taille fixe pris à la génération par `pico8_zone_arene_prendre`, avec son nom pour `pico8_fatal` (« prise »). Taille au doigt mouillé : **1536 o**. Le lot mesuré le plus gros fait 744 o, et l'en-tête d'arène 96 o. La structure posée y fait grandir son `oe` (4, 8, 16, 32 slipways : 480 o, blocs morts compris) et son `jy` (un besoin ajouté par `bb`). Ses `by`/`cc` et la liste fusionnée de `jo:cn` vont en génération. Une arène pleine sort par `pico8_fatal` « zone prise pleine, limite N octets » et se relève d'un chiffre.
- **Prises libres.** Le module planète tient une courte liste statique d'arènes libres. Capacité 4, avec un `assert` porteur de message.
  - **Ajout** : au `rendre` d'une structure, **en dernier** (après sa rangée et, pour `hj`/`pm`, sa liste `qy`), si elle est encore « kz » et que son `oe` a été fait dans une arène de prise, son arène entre dans la liste. Le test « arène de prise » se fait par la poignée de son `oe` (comme `ga`, qui retrouve sa place par `oe->zone`) ou par un champ explicite, au choix de l'implémenteur. Une structure « kz » faite par `jo:hu` est en génération et n'entre pas.
  - **Retrait** : `ie:ldown` prend une arène libre et la vide (opération « vider » de l'arène), sinon il en prend une nouvelle à la génération.
  - **Borne.** Une structure refusée en frame N n'est rendue qu'au vidage de N+2. Une prise faite entre-temps prend une autre arène. On a donc au plus deux ou trois arènes en vol, plus une par structure posée. Chaque prise retire une arène libre si elle en trouve une : la liste ne peut pas grandir sans borne.
- **Oubli.** Les arènes libres sont des blocs de la génération. La liste est vidée :
  - **au point d'oubli de partie** (`oublier_partie()`), qui tombe après le vidage de la file et avant le rendu de la génération. Les structures en vol, abandonnées par le `jl()` d'`eu` et rendues à ce vidage, y entrent d'abord, puis sont oubliées ;
  - **après la remise à zéro du chargement et de l'arrêt**, à côté de `pico8_entity_oublier_registres()`.

  Aucune prise ne tombe entre la bascule d'`eu` (frame du clic) et ce vidage (frame suivante, avant `ui:lj`). C'est **la première entrée du point d'oubli de partie**.
- **Vider une arène.** Une opération de la zone générique, à côté de `pico8_zone_arene_prendre`/`_rendre`, remet à vide la zone montante privée d'une arène sur son propre bloc, et l'empoisonne sous ASan. Le prototype la fait en réinitialisant la zone à une moitié sur le même stockage (`pico8_bump_zone_init_single`). La remise à zéro ordinaire reste refusée sur une arène (`jamais_remise`).
- **Angle mort ASan**, à noter comme pour 0a et 4a : la prise suivante réalloue aussitôt les mêmes adresses. Un pointeur oublié vers la structure refusée lirait la nouvelle sans planter. L'inventaire ci-dessous, pas ASan, garantit qu'il n'y en a pas.
- **Ce que l'arène coûte.** Chaque structure posée prend 1536 o au lieu de son lot (~650 o). Sur `game_end_bankrupt` (sept prises, une refusée), le maximum passe de 74 024 à 80 464 o. Le maximum sur les 26 e2e (`pause_restart`, sans prise) ne change pas.
- **Prototype**, refait à f87d7a8 avec l'arène. Rejeu synthétique : les frames 0 à 20 d'`empty_planet_fade`, puis N cycles « pression sur l'icône (122,124), glissé sur « p34 » (31,84), relâché, trois frames d'attente ». Le refus vient de la cible `et`.
  - Sans arène : 65 784, 66 400 et 77 488 o après 1, 2 et 20 cycles ;
  - avec arène : **66 704 o après 1, 2 et 20 cycles** ;
  - 26 `*.sha256` identiques, `sanitize-net` propre, le rejeu synthétique propre sous ASan (`detect_leaks=0`).

### Choix de la zone par le propriétaire

- Les fonctions de planète appellent la génération (`pico8_zone_partie()`) ou la zone de construction elles-mêmes, comme `bw` l'a fait en 4b. 0b demande « une zone par propriétaire » : hors de la prise, le seul propriétaire possible est la partie.
- `pico8_jo_pq` reste la forme courte : elle passe la zone de construction à `pico8_jo_pq_dans`. `ga` garde son appel direct avec sa place.
- `lz_mw_step` alloue sa copie dans la zone de construction. Le `ctx` peut porter la poignée, au choix de l'implémenteur.
- Les contrats devenus faux sont réécrits :
  - `pico8_planet_trade.h` : « Returns `self.by` itself when it is set, else a fresh heap container » devient : un conteneur fait dans la génération courante, jamais rendu avant elle ;
  - `pico8_planet.h` : « nothing ever frees it » devient : vit jusqu'au rendu de sa génération ; une structure de prise refusée rend son arène à la prise suivante ;
  - les commentaires de `bw` sur `p`/`ra`/`id` « not freed here » : à retirer ou à dire « vivent avec la génération ».

### `jo:hu` : la planète remplacée, et le relevé « éclats `ga` »

- **La planète remplacée** (`hp.cw`) reçoit `p_`, sort des registres et rend sa rangée par la file. Sa structure reste allouée dans la génération : elle n'est pas « kz » faite par une prise, donc jamais recyclée. La remplaçante (planète de génération, ou éclat `ga` de la zone particules) emprunte ses `p`, `ra` et `id`, qui sont en génération. Ils vivent plus longtemps que l'éclat, et autant que la planète remplaçante.
- **Relevé « éclats `ga` rendus en cours de partie », refait pour 4d** (lu à f87d7a8). Les objets de 4d vivent plus longtemps qu'un éclat. Peuvent-ils en désigner un après le rendu de sa place ?

  | Objet de 4d | Peut-il désigner un éclat ? | Pourquoi |
  |---|---|---|
  | `oe` d'une planète | non | `jo:kf` n'y ajoute que des slipways (`assert`) ; aucun slipway n'aboutit à un éclat (`jo:gv` ne rend vrai que pour `me=="ly"`, l'éclat reste « unknown ») |
  | `by`, `cc` | non | ne tiennent que des routes (`assert`) ; une route ne passe que par des slipways ; `ga.by` est `&g_ga_by_shared`, jamais parcouru par `bd` (un éclat n'est pas dans `cg.jo`) |
  | `jy` et ses copies | non | des `pico8_jy_t`, pas des entités |
  | `p`, `ra`, `id` | non (sens inverse) | c'est l'éclat qui les emprunte ; `pico8_ga_rendre_dispatch` ne lit que `oe->zone`, sa rangée et sa liste `qy`, jamais `p`/`ra`/`id` |
  | la structure remplacée | non | `jo:hu` n'écrit pas l'éclat dans l'ancienne planète ; `jo_kd(working_self)` ne touche l'éclat qu'au moment où il est vivant |
  | une structure `pm` (prise ou posée) | non | `pm:pq` crée son éclat avec une copie de position dans sa place (`ra_copie`) et ne le garde pas ; une prise de `pm` refusée fait donc naître un éclat qui vit sa vie de particule |
  | la liste des prises libres | non | elle ne tient que des arènes de prise ; `pico8_ga_rendre_dispatch` ne passe pas par `pico8_jo_rendre_dispatch` |

  Le sens « éclat qui survit à la génération » ne peut pas arriver : l'éclat est une entité de `hv` avec `rendre`, abandonnée par le `jl()` d'`eu` et rendue au vidage, **avant** le rendu de la génération (4a).

### Inventaire du §4 pour 4d

Deux durées de vie finissent sur des objets de 4d :

- **la génération** : rendue au vidage qui suit le `eu` suivant, ou remise à zéro au chargement et à l'arrêt ;
- **l'arène de prise d'une structure refusée** : réutilisée par la prise suivante, après son `rendre`.

Rien de 4d n'entre dans la sous-zone chargement.

**Méthode.** Refaire **mécaniquement**, sur le code du commit de 4d, les deux `grep` de 4a sur les 65 fichiers de `CORE_SRCS` :

- `grep -nE '^\s*static\s[^(]*\*\s*[a-zA-Z_][a-zA-Z0-9_]*(\[[^]]*\])*\s*(;|=)'` (hors `const char *` et tables `* const`) ;
- `grep -nE '^static\s+(struct\s+)?[a-z0-9_]+(_t)?\s+[a-zA-Z_][a-zA-Z0-9_]*\s*(\[[^]]*\])*\s*(;|=)'` (hors `const` et scalaires), dont on lit les champs pointeurs.

Ajouter :

- le `grep` des champs qui reçoivent un `p`, un `ra` ou un `id` de planète (`= self->ra`, `keys.ra = `, `keys.p = `, `keys.id = `) ;
- le `grep` des écritures de `fo` (`->fo = `) pour l'arène de prise.

**Relevé fait à f87d7a8**, avec, en tête, ce qu'ajoute le prototype :

| Pointeur | Peut viser | Statut |
|---|---|---|
| zone de construction (`planet`, ajouté par 4d) | une arène de prise | non NULL **seulement pendant** l'appel `new_entity` de `ie:ldown` |
| prises libres (`planet`, ajouté par 4d) | des arènes de la génération | **vidées au point d'oubli de partie** (avant le rendu) et après la remise à zéro du chargement et de l'arrêt ; n'y entrent que des arènes de structures refusées déjà rendues |
| `g_hv`, `g_cv[].list`, `g_cg[].list` (`entity`) | des planètes | **refaits** par le `jl()` d'`eu` ; **vidés sans lecture** par `pico8_entity_oublier_registres()` après le reset |
| `g_file[]` (`file_sortie`) | des planètes abandonnées, une structure refusée | **vidé avant le rendu** et avant tout recyclage : `pico8_jo_rendre_dispatch` (et celui de `hj`/`pm`, qui lit `emitter`) lit une structure vivante |
| `ok.o_`, `ok.gj`, `ok.ic`, `ok.lheld` | une planète survolée ou tenue, ses calques, un `jc` dont `qu`/`re` est une planète | champs du **nouvel** `ok` après la bascule ; `lheld` remis à nil par `ui:bh` après `ey` ; `o_`/`gj`/`re` d'un `jc` tenu remis à nil par `oublier` au `rendre` d'une structure refusée, avant son recyclage ; `ic` refaits par `gw` |
| `jo.fo`, `jc.fo` | une structure tenue (hôte de `bv`) | écrits par `bv` pendant la prise, lus et **remis à nil par le `kj` de la même frame** ; la structure n'est recyclée que deux updates plus tard |
| `g_bv_et.fo` (`probe`), `g_jc_too_far.fo` (`slipway`) | un hôte de `bv`, un `jc` | **écrits seulement, jamais lus** (4a) |
| `g_fm` (`probe`) | un `hy` | ne peut pas viser une planète : `invented`, `po`, `fu` ne tiennent ni planète ni boîte de planète |
| `g_ie_gy` (`research_icon`) | une boîte | ne peut pas viser une boîte de planète : `ie.eh` statique, puis les boîtes de `ie:hb` (4g) |
| `g_jo_oc`, `g_jo_ir` (`planet`) | des états de `bw` | repointés à chaque `eu` (4b) |
| `g_jo_kw_shared` (`planet`) | — | statique sans pointeur ; les planètes le désignent, pas l'inverse |
| `g_ga_by_shared` (`planet_family`) | — | liste toujours vide, sous-zone chargement (4c) |
| `g_mo`, `g_je`, `g_ok`, `g_kc`, `g_ju`, `g_dk`, `g_gb[]`, `g_arenes`, zones et poignées | — | ne peuvent pas viser une planète, comme à 4a et 4c |

**Objets d'autres zones qui empruntent un objet de 4d** (relevés par lecture) :

- les `iv` de `jo:hk` (`keys.ra = self->ra`), les rangées de panneau (`frame.ra = self->ra` dans `jo_pz_frame`, `lq.ra`), les rangées qui désignent une copie de besoin (`jo:gh`) : **zone interface** ;
- l'éclat `ga` né de `jo:hu` (`p`, `ra`, `id`) : **zone particules** ;
- les slipways (`qu`, `re`, `et`) et les routes (`lp`) : **transitoire** jusqu'à 4e. Une structure refusée n'a jamais de slipway (`jc:hq` ne la vise que si `gv` est vrai, donc « ly »), ni de route (pas de `bd` pendant la prise).

Tous sont des entités, ou appartiennent à une entité, inscrites dans `hv` avec un `rendre`, ou ne sont lus qu'à travers elles. Le `jl()` d'`eu` les met en file, et le vidage les rend **avant** le rendu de la génération. Une rangée de panneau ouverte sur une structure tenue n'existe pas : `jo:gh` n'est appelé que par `jo:gv` sur une planète « ly ».

**Conclusion : le point d'oubli de partie reçoit sa première entrée, la liste des prises libres.** Un pointeur trouvé ensuite par ASan y est ajouté et noté dans « Découvert » comme un **trou de l'inventaire**.

### Verrou

- `tools/alloc_guard_exceptions.txt` : retirer `planet` 6, `planet_family` 5, `planet_family_trade` 3, `planet_trade` 8, `galaxy` 3, `economy_tiers` 3.
- **Prévision : 22 / 75 → 16 / 47**, mesurée par `make alloc-guard` sur le prototype à f87d7a8 (« verrou objets 65 allouent 16 appels 47 »). L'arène de prise ne change aucun compte : elle est prise à la génération, et `pico8_research_icon.c` garde sa référence à la transitoire pour l'identifiant « s… » (4g).
- Les fichiers qui n'incluent plus la transitoire ni `<stdlib.h>` les retirent. Aucun compte ne monte.

### Budget : mesure, projection, décision

- **Mesure par prototype**, refaite à f87d7a8 (copie `git archive`, hors dépôt). Les 28 sites ont été convertis comme décidé ci-dessus, puis les 26 e2e rejoués avec `tmp/pico8_replay` et `PICO8_REMPLISSAGE`, à côté de la même mesure sur `HEAD` sans 4d :
  - les 26 `*.sha256` identiques ; `make sanitize-net` : « 26 propre(s) » ; aucun `pico8_fatal` ;
  - **`M_4d` = 130 720 o au clic de `pause_restart` = `M_4c` (34 240) + 96 480 o**, exactement la prévision faite à 8e75265 ;
  - par génération, 4d apporte **48 240 o = 90 × 536 o** juste après `eu`, soit par planète : 440 (structure) + 8 (`p`) + 8 (`ra`) + 8 (« pN ») + 3 × 24 (`oe`, `by`, `cc`, faits par le `bd` d'`eu`) ;
  - après f13 de `pause_restart` : **65 072 o = `G` + `R` + `P` = 608 + 16 224 + 48 240** ;
  - sur une seule partie, le plus gros est `game_end_bankrupt` (74 024 o, 80 464 avec l'arène de prise), puis `trade_rich_mine` (72 824 o ; 75 488 avec l'arène) ;
  - `K_4d` = 0 sur les 26 e2e (rien de 4d en chargement) ;
  - `message_box_hold` : 65 104 o à f45 comme à f90 ; `game_end_final_score` : 65 104 o après f2 comme après f51 ; `trade_rich_mine` : 72 824 o à f137 comme à f152 ;
  - **contre-épreuve** (à 8e75265, sans registres) : `cc` refaite en génération à chaque `bd` donne 105 008 o sur `game_end_final_score` (au lieu de 48 848), 82 752 sur `game_end_bankrupt`, 102 016 sur `pause_restart` et 136 832 sur `trade_rich_mine`. La réécriture en place est nécessaire.
- **Mesure à refaire après conversion** : `M_4d` = maximum des `max_octets partie`, `K_4d` = maximum des `max_octets chargement`, notés dans le commit et dans « Découvert » avec la comparaison au prototype. Un écart s'explique avant la projection.
- **Projection**, deux comptes (4c) :
  - estimations de 4a pour les familles restantes, génération : 4e ~30, 4f ~1, 4g ~6, 4h ~12, soit **49 Ko** ; chargement : 4g ~32 Ko ;
  - `r = max(1, M_4d / (2 + 20 + 160 Ko))`, rapport « mesuré / estimé » des familles faites (estimations de pic de 4b, 4c, 4d). `M_4d` ≈ 127,7 Ko, soit un rapport de 0,70 : **`r` = 1**. 4c avait trouvé `r` ≈ 1,52 sur 4b et 4c seules ; la mesure de 4d, deux fois sous son estimation, le ramène à 1 ;
  - `P_4d = M_4d + 49 Ko × r` ≈ 127,7 + 49 ≈ **177 Ko**, sous 704 ; `Q_4d = K_4d + 32 Ko × r` = **32 Ko**, sous 64.
- **Décision, dans l'ordre** (règle de 4c) :
  - `P_4d ≤ 704 Ko` et `Q_4d ≤ 64 Ko` : on continue. **C'est le cas mesuré par le prototype** ;
  - `P_4d + Q_4d ≤ 768 Ko`, mais un compte déborde : déplacer la frontière (`PICO8_ZONE_CHARGEMENT_BYTES`) dans le commit, et le dire ;
  - `768 < P_4d + Q_4d ≤ 768 + 49 Ko` : relever `PICO8_ZONE_GAME_BYTES` d'un chiffre ;
  - au-delà : **arrêt de l'étape 4 à 4d** et signalement à l'humain : zone partie, limite voulue `P_4d + Q_4d` arrondie, somme actuelle des zones (1487 Ko sur 1536 à f87d7a8). On ne rogne ni la frame, ni l'interface, ni les particules.
- Un `pico8_fatal` « zone partie pleine » pendant 4d se lit d'abord comme un site par `bd` ou par prise entré tel quel (la contre-épreuve et le rejeu de prises montrent lequel), puis comme une limite trop basse. Un « zone prise pleine » se lit comme une structure posée qui a dépassé l'arène, et se relève d'un chiffre. Dans un TU qui crée des planètes en boucle sans recharger, la correction est un `cart_load` dans son aide (précédent de 4a et 4c).

### Note « Découvert » et clôture

À la clôture, sous l'entrée 4 de `docs/plan-4.md`, une note « *Découvert (4d) :* » de quelques phrases :

- structures, `oe`, `p`/`ra`/id de `bw`, `jy` et copies, liste fusionnée de `jo:cn`, `ib` : en génération ; `cc` vidée en place ; `lp`/`rl` des propositions en zone frame ;
- correction de la table de 4a :
  - `jo:cn` et `ib` sont une fois par planète, seuls `cc` et `lp`/`rl` sont par `bd` ;
  - `jo:lz` est une fois par création et par colonisation ;
  - la prise d'icône refusée est un site par action ;
- l'arène de prise : sa taille, son coût par structure posée, la liste des prises libres, première entrée du point d'oubli de partie, et l'angle mort ASan du recyclage ;
- la contre-épreuve de `cc` et le rejeu de vingt prises refusées ;
- l'inventaire et le relevé « éclats `ga` » refait ;
- `M_4d`, `K_4d`, `P_4d`, `Q_4d`, `r`, la décision ; l'estimation de 4a pour 4d (~160 Ko) contre la mesure ;
- les comptes du verrou.

Sous l'entrée 4, deux lignes « *Reçu de l'étape 4d :* » :

- **pour 4e** :
  - la proposition `lw.t` de `jo:ib` est faite à chaque `bd`, pour chaque slipway de chaque planète ;
  - ses `lp`/`rl` sont en zone frame, et sa structure `trade` (`pico8_trade_new`) ne doit pas entrer en génération (zone frame par l'appelant, ou valeur) ;
  - seules les routes gardées dans `by`/`cc` sont des objets de partie ;
  - les TU de `trade_rich_mine` et `game_end_final_score` tiennent cette règle pour 4e ;
- **pour 4g** :
  - l'identifiant « s… » de `ie:ldown` est fait à chaque prise ;
  - il va dans l'arène de prise, la zone de construction ouverte par `ie:ldown`, pour être recyclé avec la structure ;
  - le rejeu de vingt prises refusées tient cette règle pour 4g.

La ligne « Partie » du §2 dit déjà « planètes, relais » : rien n'est à corriger, sauf si la mesure contredit la lecture. La PRD est retirée de `docs/prd/` dans le commit de clôture.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 : `make test-all`, `make sanitize-net`, 26 `*.sha256` identiques, `alloc-guard` à égalité stricte. Aucun ne passe la transitoire à un nouvel appelant.

1. **Sites par `bd` : `cc` en place, `lp`/`rl` en zone frame, `ib` en génération.**
   - Les tests d'abord :
     - TU de l'opération « vider » de `pico8_ents` ;
     - TU `game_end_final_score` et `trade_rich_mine` (octets de la génération moins registres, constants). Le premier est rouge sans la réécriture en place dès que `cc` est en génération.
   - L'opération « vider » de `pico8_ents`, avec son contrat.
   - `bd_reset_visit` : faite une fois en génération, vidée ensuite.
   - `pico8_jo_ib` : `ib` en génération, `lp`/`rl` en zone frame ; le contrat de `pico8_planet_trade.h`.
   - Le verrou : `economy_tiers` sort, `planet_trade` 8 → 2.
   - Ce morceau vient d'abord : il borne la croissance par `bd` **avant** que les structures n'entrent en génération.
2. **Structures, boîtes et listes de planète en génération.**
   - Les tests d'abord :
     - l'égalité exacte `G + R + P` après f13 de `pause_restart` ;
     - la mort par ASan d'une planète de la partie quittée après f13, et sa lecture saine pendant f12 ;
     - les trois égalités exactes de 4c complétées par `P` (voir Testing Decisions).
   - Les neuf constructeurs, `pico8_jo_pq`, `jo:lz` et `lz_mw_step` passent par la zone de construction, qui n'est encore que la génération. `jo:cn` et `bw` (`p`, `ra`, identifiant) prennent la génération.
   - Les commentaires de contrat devenus faux.
   - Le verrou : `planet`, `planet_family`, `planet_family_trade`, `planet_trade`, `galaxy` sortent. **22 / 75 → 16 / 47.**
   - Entre ce morceau et le suivant, une prise refusée laisse son lot dans la génération (au plus un refus par e2e, +616 o). Les invariants du commit tiennent ; la règle de la famille est rétablie par le morceau 3 et vérifiée à la clôture.
3. **Arène de prise.**
   - Les tests d'abord :
     - TU de l'opération « vider » d'une arène (prise revenue à l'en-tête ; un bloc d'avant le vidage meurt par ASan dans un enfant, avant toute nouvelle allocation) ;
     - le rejeu de prises refusées : la génération, registres déduits, est constante de 1 à 20 cycles (rouge à ce stade : +616 o par cycle), et une prise posée après les refus prend exactement une arène de plus ;
     - la prise refusée à travers une nouvelle partie.
   - L'opération « vider » d'une arène dans la zone générique.
   - `ie:ldown` ouvre l'arène de prise comme zone de construction autour de `new_entity`. La liste des prises libres est alimentée au `rendre` (en dernier), et remise à vide dans `oublier_partie()` et après le reset du chargement et de l'arrêt.
   - Aucun compte du verrou ne bouge : l'arène est prise à la génération.
4. **Mesure et clôture.**
   - Le relevé sur les 26 e2e, `M_4d`, `K_4d`, la comparaison au prototype (`M_4c` + 96 480 o), la projection et la décision, dans le message du commit.
   - L'inventaire refait par `grep`.
   - La note « Découvert (4d) », les deux lignes « *Reçu de l'étape 4d* » (4e, 4g), le retrait de cette PRD.

Le morceau 1 vient avant le 2 pour qu'aucun site par `bd` n'entre en génération sans sa règle. Le 3 dépend du 2 (zone de construction, structures en génération). Le 4 dépend des trois.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris et le maximum, par les accesseurs ou par les lignes écrites sous `PICO8_REMPLISSAGE` ;
  - les capacités et les comptes publics des listes (`pico8_ents_t`, `pico8_ptrs_t`) et les champs publics d'une planète (`oe`, `by`, `cc`, `jy`, `id`) ;
  - un rapport ASan dans un processus enfant ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne lit ni les bouts de la zone ni une adresse, sauf pour la relire après son rendu. Les tailles attendues se calculent dans le TU (`sizeof`, cellule de zone `PICO8_ZONE_CELLS`, `strlen` de l'identifiant), jamais en dur : les 536 o par planète ne valent que sur x86-64.
- **Empreinte d'une planète**, aide de TU, à côté de `octets_d_un_registre`/`octets_des_registres` de 4c : pour une planète de `cg.jo`, compter les cellules de sa structure, de `p`, de `ra` et de son identifiant (`strlen + 1`). Ajouter, pour chacune de ses listes non nulles (`oe`, `by`, `cc`, `jy`), le conteneur et les blocs pris depuis la capacité 0 (capacités 4, 8, …, `C`, comme `octets_d_un_registre`). `P` est la somme sur `cg.jo`. L'aide sert après f13 de `pause_restart`, où `cg.jo` ne tient que les 90 planètes de `bw`, sans structure ni arène de prise.
- **`tests/test_pico8_places_vivantes.c`** (ASan, un enfant par cas, `demarrer`/`jouer_jusqu_a`, sans `relever`, dont le vidage rendrait la génération lui-même) :
  - **`pause_restart_borne_les_blocs_morts_des_registres`** (4c) : `prise == G + R` devient `prise == G + R + P`. Le cas prouve désormais aussi que les planètes sont dans la génération et que chacune ne coûte que ce qu'elle garde. Le prototype donne 65 072 = 608 + 16 224 + 48 240 ;
  - **`pause_restart_compte_les_etats_de_bw`** (4c) : son égalité après f13 (`g + octets_des_registres()`) gagne `P` ; ses inégalités de f11 et f12 restent ;
  - **une planète de la partie quittée meurt après le rendu** (cas nouveau, sur le modèle de `pause_restart_devoile_sur_les_etats_de_la_nouvelle_partie`) : un pointeur vers une planète de `cg.jo` pris après f6 (première partie) se relit sans crash après f12 (frame du clic, bascule). L'enfant meurt par `use-after-poison` quand on le relit après f13 ;
  - **`bd` ne fait pas grandir la génération** (cas nouveau) : sur `game_end_final_score`, `prise − R` après f2 est égal à `prise − R` après f51 (25 sauts d'année, donc 25 `bd` au moins). Rouge si `cc` est refaite à chaque `bd` ;
  - **`bd` sur de vraies routes** (cas nouveau) : sur `trade_rich_mine`, `prise − R` après f137 est égal à `prise − R` après f152, saut d'année de f140 compris (`jo:ib` sur les sept slipways posés de f112 à f136, `jo:cn`, `lp`/`rl`). `R` est déduit parce qu'un saut d'année inscrit des `iv` qui peuvent faire grandir `hv` ;
  - **prises refusées répétées** (cas nouveau), sur un rejeu de fixture `tests/fixtures/` (pas un e2e : il n'ajoute pas de ligne de code couverte). Il reprend les frames 0 à 20 d'`empty_planet_fade`, puis vingt cycles « pression sur (122,124), glissé et relâché sur « p34 » (31,84), trois frames d'attente ».
    - `prise − R` après le premier cycle est égal à `prise − R` après le vingtième. Rouge sans le recyclage (+616 o par cycle) ;
    - une prise **posée** ajoutée en fin de fixture (le labo de f95-98 d'`empty_planet_fade`), puis une prise refusée : la génération grandit d'exactement une arène (taille de l'arène) entre avant la pose et après le refus suivant ;
  - **prise refusée à travers une nouvelle partie** (cas nouveau, même fixture) : une prise refusée, « restart », nouvelle partie, puis une prise. L'enfant ne meurt pas. Sans la remise à vide au point d'oubli, la prise reprendrait une arène de la génération rendue et planterait par `use-after-poison` ;
  - **croissance par frame** : f45 = f90 sur `message_box_hold`, inchangé ;
  - **sentinelle de 4a**, **`pause_restart_recharge_apres_le_titre`** (4c) : inchangés.
- **`tests/test_pico8_zone_partie.c`** : `case_prise_and_max` (« `g + octets_des_registres() + 64` » après l'update qui rend) gagne `P`, puisque le `eu` du cas fait `bw(90)`. Mêmes aides.
- **`tests/test_pico8_zone_choisie.c`** (cas nouveaux) : vider une arène ramène sa prise à l'en-tête. Un bloc pris avant le vidage meurt par ASan dans un enfant. Une nouvelle allocation après le vidage reprend la même adresse : c'est l'angle mort, que le test documente par une égalité d'adresse plutôt que par un crash.
- **`tests/test_pico8_ents.c`** (cas nouveau) : après des ajouts qui ont porté la capacité à `C`, l'opération « vider » laisse un compte de 0, la même capacité et le même `items`. Des ajouts jusqu'à `C` ne prennent plus rien à la zone (prise d'une zone montante de TU inchangée) ; l'ajout `C + 1` grandit comme avant.
- **Tests de planète existants** (`test_pico8_jo`, `test_pico8_qh`, `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_nr`, `test_pico8_fr`, `test_pico8_mx`, `test_pico8_ga`) : verts sans changement d'attente. Le prototype à f87d7a8 passe tout `make test`, sauf les égalités exactes de 4c (`case_prise_and_max`, `pause_restart_compte_les_etats_de_bw`, et `pause_restart_borne_les_blocs_morts_des_registres`, non atteint parce que l'enfant précédent arrête le binaire), que 4d complète par `P`.
- **Verrou** : `make alloc-guard` à égalité stricte, liste sans les six fichiers. Pas de nouveau cas dans `tests/test_alloc_guard.c`.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. Sur `pause_restart`, la génération rendue à f13 contient désormais les 90 planètes de la première partie. `empty_planet_fade` exerce une prise refusée suivie d'une prise posée qui reprend l'arène recyclée.
- **Exemples dans le dépôt** :
  - `tests/test_pico8_places_vivantes.c` : rejeu par enfant, `demarrer`/`jouer_jusqu_a`, `etats_de_bw()`, `octets_des_registres()`, les cas de 4b et 4c ;
  - `tests/test_pico8_zone_partie.c` : cas en enfant relancé par nom, `use-after-poison` attendu ;
  - `tests/test_pico8_zone_choisie.c` : poignées et arènes ;
  - `tests/test_pico8_ents.c` : la forme des listes ;
  - `tests/fixtures/` : rejeux qui ne sont pas des e2e.

## Out of Scope

- Les routes `trade` (structures, `trade:kg`, la proposition `lw.t` elle-même), les slipways, `jc:hq`, les traînées `ts` : famille 4e. 4d ne fait que poser `lp`/`rl` de la proposition en zone frame.
- L'identifiant « s… » de `ie:ldown` (`research_icon` 130), `pico8_jy_new` (appelé par `jo:cn` et `bb`), `hp`, `hp.i`, `gb` : famille 4g. 4d ouvre l'arène de prise où l'identifiant ira.
- Les boîtes par frame des singletons (`qb`, `nf`, `bv`, `ok.qa`) : famille 4h.
- L'éclat `ga` : déjà en zone particules (étape 3). Celui que fait naître une prise de `pm` refusée y vit et y meurt comme avant.
- Toute modification de la zone à deux bouts, de sa bascule, de son rendu, de la sous-zone chargement ou des registres : 4a et 4c. 4d n'ajoute à la zone générique que « vider une arène ».
- Relever `PICO8_ZONE_GAME_BYTES`, sauf si la projection l'exige.
- Retirer TROU-7 : étape 5.
- Lancer l'application raylib.

## Vérifié contre 4c

Relu à `HEAD` f87d7a8, sans toucher git :

- la note « *Découvert (4c)* » et la ligne « *Reçu de l'étape 4c* » sous l'entrée 4 de `docs/plan-4.md` ;
- `git show 4530656 --stat` (registres, morceau 2) et `git show 8e75265 --stat` (sous-zone chargement, morceau 1), puis le code : `src/pico8_entity.{h,c}` (`pico8_entity_abandonner_hv`, `pico8_entity_oublier_registres`), `src/pico8_cart.c` (`oublier_partie`, `pico8_cart_load`, `pico8_cart_shutdown`), `tests/test_pico8_places_vivantes.c`, `tests/test_pico8_zone_partie.c` ;
- `git diff --stat 8e75265 f87d7a8` sur les six fichiers de 4d, `pico8_ents.{h,c}` : aucun changement ;
- `tools/alloc_guard_exceptions.txt` et `make alloc-guard` sur une copie `git archive HEAD` : 65 objets, 22 allouent, 75 appels ; les 28 relocations de 4d par `objdump -r` + `addr2line`.

Les hypothèses du brouillon, une par une :

- **H-4c-1 (verrou) — tenue.** 22 fichiers, 75 appels, `entity` sorti. Les six fichiers de 4d sont à 6, 5, 3, 8, 3, 3, aux mêmes lignes qu'à 8e75265. Le prototype refait mesure 16 / 47.
- **H-4c-2 (reset sans relecture) — tenue.** `pico8_cart_load()` et `pico8_cart_shutdown()` appellent `pico8_entity_abandonner_hv()` avant le vidage et `pico8_entity_oublier_registres()` après `pico8_zone_partie_reset()`. 4d y ajoute la remise à vide des prises libres, à côté du second.
- **H-4c-3 (TU de 4c) — tenue, précisée.**
  - Les aides s'appellent `octets_d_un_registre` et `octets_des_registres`, et sont dupliquées dans les deux fichiers de TU.
  - Les égalités exactes sont **trois** et non deux : `pause_restart_borne_les_blocs_morts_des_registres` (`G + R`) et `pause_restart_compte_les_etats_de_bw` (`G + R` après f13) dans `test_pico8_places_vivantes.c`, et `case_prise_and_max` (`g + R + 64`) dans `test_pico8_zone_partie.c`. Testing Decisions les nomme et leur ajoute `P`.
- **H-4c-4 (budget) — tombée sur les chiffres, tenue sur la décision.**
  - `M_4c` = 34 240 o (et non 20 à 30 Ko), `K_4c` = 0, frontière inchangée (704 / 64), décision « on continue ».
  - 4c a projeté avec `r` ≈ 1,52 (P ≈ 351 Ko, Q ≈ 49 Ko). La projection de 4d est refaite avec la mesure : `r` revient à 1, `P_4d` ≈ 177 Ko, `Q_4d` = 32 Ko.
- **H-4c-5 (inventaire) — tenue.** `oublier_partie()` est vide à f87d7a8, et aucun statique nouveau ne vise une planète. 4d lui donne sa première entrée (prises libres).
- **H-4c-6 (plan) — tenue.** « *Découvert (4c)* » est écrit. La ligne « *Reçu de l'étape 4c* » vise 4g (sous-zone chargement pour `gb` et `hp.i`), pas 4d.

**Ce que la vérification change dans cette PRD** :

- les renvois aux hypothèses sont remplacés par les faits ;
- le prototype est refait à f87d7a8 : `M_4d` = 130 720 o, exactement la prévision `M_4c` + 96 480 ;
- la projection est refaite avec le vrai `M_4c` et `r` = 1 ;
- Testing Decisions nomme les trois égalités exactes de 4c et ses aides ;
- les morceaux n'ont plus de prérequis 4c.

**Décision révisée à la même relecture** : la prise d'icône refusée n'est plus laissée en génération avec un risque d'épuisement. Elle était contraire à la règle de la famille (« un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place »). Elle est désormais recyclée par une arène de prise (« Arène de prise »), prouvée par le prototype et tenue par un TU.

## Further Notes

- **Pas d'humain disponible, et pas d'outil `Agent` dans cette session pour lancer un agent `guideline` neuf.** J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage) :
  - **Hypothèses explicites** : celles sur 4c sont vérifiées (« Vérifié contre 4c »). Les fréquences des sites ont été lues dans le code et dans le cart (`jo:ib` l. 500, `jo:cn` l. 479, `jo:lz` l. 461, `jo:hu` l. 451, `bd` l. 1265, `hy:nx`/`hy:hk` l. 1204-1216, `ie:ldown` l. 932, `ui:bh` l. 237, `d_` l. 569, `fo` l. 549/558/680). Il en sort deux corrections de la table de 4a (`jo:cn`, `ib`) et un site par action qu'elle ne nommait pas (prise refusée).
  - **Simplicité** :
    - deux opérations nouvelles (« vider » de `pico8_ents`, « vider » d'une arène) ;
    - une zone de construction et une liste de quatre arènes ;
    - pas de paramètre de zone ajouté aux fonctions partagées ;
    - pas de nouvelle zone dans le contrôle de somme ;
    - `ga` intact.
  - **Changements chirurgicaux** : 28 relocations dans six fichiers. Le recyclage touche en plus `ie:ldown` (deux appels), le `rendre` de planète (un appel), `oublier_partie()` et le chargement (un appel chacun), et la zone générique (une opération).
  - **Critères vérifiables** :
    - 16 / 47 au verrou ;
    - `G + R + P` exact après f13 de `pause_restart` ;
    - `prise − R` constant sur `game_end_final_score`, `trade_rich_mine` et vingt prises refusées ;
    - f45 = f90 sur `message_box_hold` ;
    - la planète de la partie quittée qui meurt après f13 ;
    - 26 `*.sha256` et `sanitize-net` ;
    - `M_4d` = `M_4c` + 96 480 o ;
    - `P_4d` ≤ 704 Ko.
- **Le prototype.** Fait dans des copies `git archive`, hors dépôt : d'abord à 8e75265, puis refait à f87d7a8. Il convertit les 28 sites comme décidé ici. Il vide `cc` en écrivant le compte directement, au lieu de l'opération de `pico8_ents`. L'arène de prise passe par une statique « zone de construction » posée par `ie:ldown`, et la structure refusée est reconnue par son état « kz » et la poignée de son `oe`.
  - Résultats : `alloc-guard` 16 / 47 ; 26 `*.sha256` identiques ; `sanitize-net` propre ; `make test` vert sauf les trois égalités exactes de 4c ; les mesures de « Budget » et d'« Arène de prise ».
  - Il n'est pas versé : c'est au cycle `tdd` de 4d de refaire le code, tests d'abord.
- **Ce que 4d change à la projection.** La plus grosse estimation de 4a (~160 Ko) est remplacée par une mesure deux fois plus basse (~94 Ko). Le budget de la zone partie n'est plus un risque pour l'étape 4 sur les e2e. Le risque restant est celui des parties longues, que le §6 accepte de découvrir par `pico8_fatal` : slipways et routes sans budget (§7, 4e). La prise d'icône n'en est plus un.
- **`nr` est le relais** (slipgate, glossaire) : il n'a pas l'étiquette `jo`, n'est pas dans `cg.jo`, et n'a donc ni `cc` ni `by`. Sa structure et son `oe` sont à 4d, et, comme les autres structures d'icône, il passe par l'arène de prise.
