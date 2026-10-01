---
title: "Plan 4, étape 4f — Trous de ver dans l'arène de prise partagée, liste ox réservée, rendus à leur mort"
description: "Un trou de ver mz naît à chaque pression sur le vide quand une sonde vole, et meurt au plus 28 frames après son relâcher ; sa structure et sa liste ox sont encore prises à la libc et à la transitoire, et ne sont jamais rendues. 4f construit chaque mz, ox compris (capacité réservée exacte), dans l'arène de prise de 4d/4e, et lui donne un rendre qui remet cette arène dans les prises libres, sans condition d'état."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. L'étape 4 range les objets de partie dans la **zone partie**. Cette PRD traite la sous-étape **4f, trous de ver**.

Elle a été rédigée pendant 4e, avec des hypothèses H-4e-1 à H-4e-10. **4e est finie** : morceau 1 d0fd7e7, morceau 2 d819338, morceau 3 aae9612, clôture 4093382. Toutes les hypothèses ont été revérifiées à `HEAD` 4093382 (section « Vérifié contre 4e »). Les sections qui en dépendaient sont réécrites sur les faits, et le prototype a été refait sur 4093382. Tout a été lu sur le contenu commité (`git archive HEAD`), jamais sur l'arbre de travail.

**Le texte de l'étape**, recopié du plan à 4093382 (entrée 4, extraits qui concernent 4f) :

> 4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement, et si son remplissage ne monte pas pendant une phase sans action d'un e2e.
>    Elle se fait en sous-étapes, une PRD chacune :
>    - **4a. Socle.** […]
>    - **4b et suivantes.** Une famille de propriétaires par PRD dans l'ordre fixé par la PRD 4a : 4b états de `pico8_gen.c` ; 4c tableaux croissants et registres (crée la sous-zone chargement) ; 4d planètes et relais ; 4e slipways et routes ; 4f trous de ver ; 4g recherche et économie ; 4h singletons de `jl`/`eu` et objets du joueur (famille ajoutée par 4a : les « Reçu » des étapes 1 à 3 leur reviennent). N'entre en zone partie qu'une allocation faite une fois par objet de partie ; un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
>    - *Découvert (4a) :* la partie bascule en tête de `pico8_eu`, avant son `jl()` ; `_init` (titre, « restart », « again? ») ne bascule pas. L'ancienne génération est rendue au vidage suivant (en jeu, l'update qui suit le clic), après un point d'oubli de partie, vide à ce jour […]. Zone à deux bouts, un seul stockage de 768 Ko ; une double bascule sans vidage est refusée ; `cart_load`/`shutdown` remettent tout à zéro. […] Angle mort ASan : un pointeur gardé à travers deux bascules ne plante plus. […] Les 81 relocations de l'étape 4 sont attribuées aux familles 4b–4h ; la seule marge est de 49 Ko sur la somme des zones […]. Le rejeu écrit `partie` et `max_octets partie`.
>    - *Découvert (4c) :* la zone partie se partage en génération (704 Ko, deux bouts) et chargement (64 Ko, remis à zéro au seul chargement) […]. Blocs morts des agrandissements bornés (prise = G + R exact). […]
>    - *Découvert (4d) :* planètes, `oe`, `p`/`ra`/« p… », listes de `jo:lz` et `jo:cn` en génération ; […]. Une prise d'icône refusée faisait grandir la génération (+616 o) : elle prend une arène de 1536 o recyclée par une liste de 4 prises libres. Trou de l'inventaire du §4 : le `fo` d'une planète hors écran pouvait viser l'objet rendu ; `oublier` le coupe (TROU-26). […]
>    - *Reçu de l'étape 4d :* pour 4e, la route `lw.t` proposée à chaque `bd` ne doit pas entrer en génération ; l'arène de prise et sa liste de prises libres sont réutilisables pour le slipway tiré. […]
>    - *Découvert (4e) :* routes candidates en zone frame, seule la route que `gu` accepte est copiée en génération (`pico8_trade_copy`) ; un `jc` est fait à chaque pression sur une planète (pas une fois par slipway) : il prend l'arène de prise de 4d, portée à 2048 o, et la rend s'il est refusé ; son point souris est réécrit en place ; ses traînées `ts` forment une nouvelle sorte de la zone particules (24 o × 680, zone particules 130 822 o sur 131 072, sans relèvement). Mesure : maximum 130 720 o inchangé, `trade_rich_mine` 95 312 o, au plus 13 traînées ; projection ≈ 147 Ko ≤ 704 : on continue. Comptes : 16 / 47 → 13 / 36.
>    - *Reçu de l'étape 4e :* pour 4h, le point souris d'un slipway tenu garde `ok.qa` (`ra = mp`) : la zone de `ok.qa` doit le couvrir.
>    - *Reçu de l'étape 3 :* la structure `g_` (et `fv`) et sa boîte `p` de `bv` restent à cette étape (`/* TROU-7 */`) ; […]

Aucune ligne « *Reçu* » du plan ne vise 4f à 4093382.

**La ligne « Partie » du §2**, recopiée à 4093382 (4e y a écrit « slipways posés » et « routes `trade` gardées dans `by` ») :

> | **Partie** | planètes, relais, slipways posés, routes `trade` gardées dans `by`, recherche, trous de ver ; les tableaux croissants (`ents`, `ptrs`, registres) qui les contiennent ; les objets faits au chargement (`gb`, `hp.i`, listes statiques) dans une sous-zone chargement, prise dans la zone partie, remise à zéro par le seul chargement | une génération par partie, de `eu` à `eu` ; rendue d'un bloc au vidage de la file qui suit le `eu` suivant (zone à deux bouts) ; `_init` ne la rend pas |

**La ligne de 4f dans la table des familles de la PRD 4a**, retirée par b46601e (`git show b46601e~1:docs/prd/triage/phase1-etape4a-socle-zone-partie.md`), recopiée :

> | Ordre | Famille | Sites (fonction, ligne) | Relocations | Reçus traités |
> |---|---|---|---|---|
> | **4f** | **Trous de ver** | `pico8_mz_struct_new` (204), `pico8_mz_pq` (39, 40 : `ox`) | `wormhole` 3 → sort | — |

La raison de l'ordre, recopiée de la même PRD :

> - **4f** est petite, avec les mêmes motifs que 4e.

**L'estimation de 4a pour 4f** : « 4f trous de ver | < 1 Ko | ~1 Ko » (par génération, pic à la bascule), avec `pico8_mz_t` à 224 o.

**La règle des familles**, recopiée de la PRD 4a (section « Sites qui allouent par tick ou par action ») :

> - n'entre en génération de partie qu'une allocation faite une fois par objet de partie créé (planète posée, slipway, route, icône, singleton de `jl`/`eu`) ;
> - un champ réaffecté par tick ou par action doit, au choix de la PRD de sa famille et site par site :
>   - être **réécrit en place**, quand la lecture prouve qu'aucun autre objet ne garde l'ancienne valeur ;
>   - devenir une **valeur** au lieu d'une boîte, quand la boîte n'est jamais modifiée en place. […]
>   - aller en **zone frame**, s'il n'est relu que dans la frame ;
>   - prendre une **place** dans une zone qui rend.

**La méthode de suivi du budget** est celle de la PRD 4a, reprise par 4c, 4d et 4e : deux comptes (génération ≤ 704 Ko, chargement ≤ 64 Ko), projection `P_k = M_k + Σ estimations restantes × r`, `r = max(1, mesuré / estimé)`, marge de 49 Ko sur la somme des zones.

**Ce qui se passe aujourd'hui** (lu à 4093382, cart et port ; `pico8_wormhole.{h,c}` n'a changé dans aucun commit de 4e) :

- **Quand naît un trou de ver.** `bg:ldown` (cart l. 376-377, `pico8_bg_ldown`) : une pression du bouton gauche sur le vide fait un `mz` **si `cg.g_` n'est pas vide**, sinon une sonde `g_`. Une sonde posée reste toute la partie : dès la deuxième pression sur le vide, **chaque pression fait un `mz`**. C'est donc un objet **par action**, sans borne par partie, et non « une fois par partie » comme le laissait lire l'estimation de 4a.
- **Combien de temps il vit** (cart l. 752-797, `pico8_wormhole.c`).
  - Tenu (`ok.lheld`), il est en état « kz » ; `mz:hq` (`bv`, puis la recherche de l'ancrage le plus proche dans `ox`) tourne à chaque frame.
  - Au relâcher, `mz:ey` : avant 15 ticks, ou si `d_` échoue (`et` posé, ou pas d'argent), `p_` ; il est **retiré par le `dg` suivant**. Sinon `d_` le passe en « pr » (`dq`), et `mz:pr` met `p_` à `t > 25` : il meurt **26 ticks après le relâcher**.
  - Le prototype le confirme : relâché ouvert, il est rendu 28 frames plus tard (f160 → f188 sur la fixture de pliages) ; sur `slipway_build` et sur la fixture de refus de 4e, le `mz` de f4, ouvert à f20, est rendu à f48 ; sur `trade_rich_mine`, les `mz` de f26 et f51 sont rendus vers f70 et f95 ; sur une fixture « f0-20 de `slipway_build`, puis f2-17 de `pause_restart` », le `mz` ouvert est abandonné par l'`_init` de f26 et rendu au vidage de f27.
- **Ce qu'il alloue.**
  - Sa structure, `pico8_mz_t`, **224 o** sur x86-64 (mesuré par `sizeof`), par `malloc` (`pico8_mz_struct_new`, l. 204).
  - Sa liste `ox` (`mz:pq`, cart l. 764 : `jp(hv, function(p) return p.fx end)`) : le conteneur `pico8_ents_t` (24 o) par `malloc` (l. 39), et ses éléments par la transitoire (l. 40), grandis par doublement depuis 4. `ox` tient **toutes les entités de `hv` dont `fx` est vrai** : les planètes de la famille `jo` colonisées ou reliées (`fx` posé par `jo:ly` et le commerce), et les sondes `g_`/`fv` (`fx = 1` dans leur description). Un éclat `ga` a `fx = 0` et n'y entre jamais. **`ox` n'est écrite que par `mz:pq`**, une fois, à la création ; elle est relue par `mz:hq` à chaque frame tenue (`ij`).
  - `mz.p` (la boîte `p` de `bv`) est faite par `pico8_bv` (`probe` 162) à chaque frame tenue : c'est un site de **4h** (table de 4a), que 4f ne touche pas. `mz.ra` est `mp`, c'est-à-dire `ok.qa` (4h aussi).
- **Qui la rend.** Personne : `mz` n'a pas de `rendre`. Retiré de `hv` par `dg`/`he`, il **n'entre pas dans la file de sortie** et fuit (TROU-7), structure et `ox` compris. Il ne passe donc jamais par `oublier`.
- **L'arène de prise, après 4e** (`pico8_planet.{h,c}`) :
  - `PICO8_JO_PRISE_OCTETS` vaut **2048**, contrat « arène où une icône tenue ou un slipway tiré construit son objet » ;
  - une seule liste de prises libres, de **4** places, partagée par icônes et slipways ;
  - `pico8_jo_prise_zone_rendue(zone, me)` est la fonction publique de retour : elle ne fait rien si `zone` n'est pas une arène nommée « prise » ou si `me` n'est pas « kz », sinon elle met l'arène dans la liste ; `pico8_jo_prise_rendue` (planète) et le `rendre` du `jc` l'appellent ;
  - `pico8_jo_oublier_prises()` vide la liste au point d'oubli de partie, au chargement et à l'arrêt.
- **Le verrou** à 4093382 : **13 fichiers, 36 appels** (`make alloc-guard` sur une copie `git archive HEAD` : « verrou objets 65 allouent 13 appels 36 »). Les 3 relocations de 4f sont aux lignes de la table de 4a : `wormhole` 39, 40, 204.

**Le problème.** Le `mz` est une allocation **par action**. La règle des familles lui interdit la génération telle quelle : une pression ratée sur le vide la ferait grandir de 224 + 24 + 32 = **280 o** au moins, à chaque clic, toute la partie, que le §6 interdit d'enregistrer. Il lui faut une **place dans une zone qui rend** : une place rendue à sa mort, réutilisée par la pression suivante. Ce mécanisme existe : c'est l'arène de prise de 4d, partagée par 4e avec les slipways.

## Solution

- **Le trou de ver dans l'arène de prise partagée.**
  - Chaque `mz` est construit, avec sa liste `ox`, dans une arène de prise ouverte par `pico8_jo_prise_ouvrir()` et prise par `pico8_jo_zone_construction()`, comme le `jc` de 4e et la structure d'icône de 4d. Même liste de prises libres, même taille (2048 o).
  - Le lot d'un `mz` fait 96 (en-tête d'arène) + 224 (`pico8_mz_t`) + 24 (conteneur `ox`) + 8 × n, où n est le nombre d'entités `fx` de `hv` à sa création. Il tient dans 2048 o jusqu'à **n = 213**. Sur les 26 e2e, n vaut **3 au plus**.
- **`ox` réservée exactement.** `mz:pq` compte d'abord les entités `fx` de `hv`, puis réserve `ox` à ce compte, puis la remplit dans le même ordre. Elle ne grandit jamais : `ox` n'est écrite qu'une fois. Sans réservation, la croissance par doublement laisserait des blocs morts dans l'arène (jusqu'à ~2 Ko d'éléments pour n ≈ 120), et la ferait déborder en fin de partie.
  - Une fonction nouvelle de `pico8_ents`, sœur de `pico8_ptrs_init_reserve` : 4e n'en a pas ajouté.
- **Un `rendre` pour le `mz`**, qui le fait entrer dans la file de sortie à sa mort (`he`) ou à l'abandon de sa partie (`jl()` d'`_init`/`eu`), donc dans `oublier`. Il remet son arène dans les prises libres **sans condition d'état** : un `mz` meurt en « kz » (refusé) comme en « pr » (ouvert), et rien n'emprunte sa mémoire après sa mort (voir « Le `rendre` du trou de ver »). `pico8_jo_prise_zone_rendue` garde son test « kz » ; 4f en extrait le cœur sans état, que le `mz` appelle.
- **Liste des prises libres : capacité 4 → 8.** Un `mz` ouvert garde son arène 28 frames après son relâcher, quand un `jc` ou une icône refusés la rendent en deux. La lecture borne à 4 les arènes recyclables en vol à la fois, soit exactement la capacité actuelle ; 4f prend de la marge (32 o statiques, hors zones).
- **Deux TU de prise réécrits.** Une arène laissée libre par un trou de ver mort se fait reprendre par la prise suivante. Sur leurs fixtures, la structure d'icône posée de 4d et le slipway posé de 4e ne prennent donc plus d'arène neuve : le prototype les voit rouges sur « + `PICO8_JO_PRISE_OCTETS` », verts sans.
- **Inventaire du §4** refait pour le `mz`, son arène et sa liste : aucune entrée nouvelle au point d'oubli de partie ni à `oublier`. Une ligne « *Reçu de l'étape 4f* » pour 4h : `ox` et `ij` d'un `mz` désignent des sondes `g_`.
- **Budget** : `M_4f` = `M_4e` = **130 720 o** mesuré par le prototype (`pause_restart` n'a aucun `mz`) ; `trade_rich_mine` reste à 95 312 o. Projection **≈ 146 Ko** sous 704 Ko : **on continue**.
- **Verrou** : `wormhole` sort. Mesuré par le prototype à 4093382 : **13 / 36 → 12 / 33**.

Aucun pixel ne change, aucun tirage `rnd` n'est déplacé : le prototype à 4093382 garde les 26 `*.sha256`, et `make sanitize-net` répond « 26 propre(s) ».

## User Stories

1. En tant que mainteneur du plan 4, je veux que `pico8_wormhole.c` n'appelle plus ni `malloc` ni la transitoire, pour qu'il sorte de la liste d'exceptions du verrou.
2. En tant que développeur, je veux que chaque `mz` soit construit dans l'arène de prise de 4d/4e, `ox` comprise, pour qu'un trou de ver et tout ce qu'il possède vivent et meurent d'un bloc, sans second mécanisme de recyclage.
3. En tant que développeur, je veux que `pico8_mz_struct_new` ouvre la prise avant d'allouer sa structure et la ferme après `mz:pq`, avant l'inscription dans les registres, pour que les registres gardent leur propre poignée.
4. En tant que développeur, je veux que la liste `ox` soit réservée exactement au nombre d'entités `fx` de `hv`, pour qu'elle ne laisse aucun bloc mort dans l'arène et n'y grandisse jamais.
5. En tant que développeur, je veux que l'ordre des éléments de `ox` reste celui de `hv`, pour que `ij` choisisse le même ancrage qu'aujourd'hui en cas d'égalité de distance.
6. En tant que développeur, je veux une fonction de `pico8_ents` qui réserve une capacité à l'initialisation, sœur de celle de `pico8_ptrs`, pour ne pas écrire à la main dans les champs du conteneur.
7. En tant que développeur, je veux que `mz` gagne un `rendre`, pour qu'il entre dans la file de sortie à sa mort comme à l'abandon de sa partie.
8. En tant que développeur, je veux que le `rendre` du `mz` remette son arène dans les prises libres quel que soit son état, pour qu'un trou de ver ouvert (« pr ») soit recyclé comme un refusé (« kz »).
9. En tant que développeur, je veux que la remise d'une arène sans condition d'état soit le cœur de `pico8_jo_prise_zone_rendue`, appelé après son test « kz », pour qu'aucun appelant ne passe un faux état.
10. En tant que mainteneur, je veux que la liste des prises libres passe de 4 à 8 places, parce que deux `mz` ouverts, un objet tenu et un refusé en attente de vidage peuvent être en vol à la fois, pour qu'aucun `assert` ne tombe en partie longue.
11. En tant que mainteneur, je veux qu'une suite de vingt trous de ver refusés laisse la génération constante, pour prouver que le `mz` n'entre pas en génération à chaque pression.
12. En tant que mainteneur, je veux que la génération ne bouge pas pendant les frames où un `mz` est tenu, pour prouver que `mz:hq` n'alloue rien dans la zone partie (la boîte `p` de `bv` reste à 4h).
13. En tant que mainteneur, je veux qu'un trou de ver ouvert, mort 26 ticks après son relâcher, rende son arène à la pression suivante, pour prouver que l'état « pr » est recyclé.
14. En tant que mainteneur, je veux qu'un `mz` vivant quand la partie est abandonnée se relise pendant la frame du clic qui lance la nouvelle partie, et fasse planter un enfant ASan quand il est relu après la frame suivante, pour prouver qu'il vit dans la génération et meurt avec elle.
15. En tant que mainteneur, je veux qu'un `mz` refusé, puis « restart », puis une nouvelle partie et une pression sur le vide, ne fasse pas mourir l'enfant ASan, pour prouver que les prises libres de l'ancienne génération sont oubliées avant d'être reprises.
16. En tant que mainteneur, je veux que le contrôle `message_box_hold` f45 = f90 reste vert, pour que la croissance par tick reste nulle.
17. En tant que mainteneur, je veux que `une_prise_posee_garde_son_arene` (4d) et `un_slipway_pose_garde_son_arene` (4e) attendent ce que leur fixture rend vraiment, puisque le `mz` de f4 laisse une arène libre que la pose reprend, pour qu'ils ne mentent pas.
18. En tant que gardien de l'invariant, je veux que les 26 `*.sha256` restent identiques, pour prouver que la réservation de `ox` et le recyclage ne changent ni un pixel ni un tirage `rnd`.
19. En tant que gardien de l'invariant, je veux que `make sanitize-net` reste vert sur les 26 e2e, pour que rien ne lise une arène de trou de ver recyclée.
20. En tant que mainteneur du plan 4, je veux que l'inventaire du §4 soit refait mécaniquement pour le `mz` (pointeurs `static`, champs des structures `static`, écritures de `fo` et d'`et`), pour que la liste des références oubliées vienne du `grep` et non de la lecture des seuls sites.
21. En tant que mainteneur du plan 4, je veux que le relevé « éclats `ga` rendus en cours de partie » soit refait pour le `mz`, pour savoir si `ox`, `ij` ou `et` peuvent désigner un éclat rendu.
22. En tant que mainteneur du plan 4, je veux le maximum de la zone partie et de la sous-zone chargement relevé sur les 26 e2e après conversion, avec la projection et la décision, pour que le budget repose sur une mesure.
23. En tant que mainteneur du plan 4, je veux que la note « Découvert (4f) » corrige l'estimation de 4a (« < 1 Ko par génération ») : le `mz` est par action, et son coût est un nombre d'arènes borné par les prises en vol, pour que les familles suivantes projettent sur un modèle juste.
24. En tant que mainteneur de 4h, je veux une ligne « *Reçu de l'étape 4f* » qui dise que `ox` et `ij` d'un `mz` désignent des sondes `g_`, et que `mz.p`/`mz.ra` restent les boîtes de `bv`/`ok.qa`, pour que 4h ne range pas la sonde dans une place recyclée sans revoir ces pointeurs.
25. En tant que mainteneur, je veux que les contrats devenus faux du module trou de ver (« nothing frees the struct », `TROU-7` sur `mz:pq`, « never frees any `mz` ») soient réécrits, et que `<stdlib.h>` et l'inclusion de la transitoire en sortent, pour que les commentaires restent vrais (règle du `CLAUDE.md`).
26. En tant que mainteneur, je veux que le contrat de `PICO8_JO_PRISE_OCTETS` nomme aussi le trou de ver, pour qu'un relèvement futur sache qui la partage.

## Implementation Decisions

### Inventaire des sites de 4f, et zone retenue

Chaque site est tranché par lecture : « à quelle fréquence, et qui d'autre garde l'objet ? ».

| Site | Ligne à 4093382 | Fréquence (lecture) | Qui garde l'objet | Zone retenue |
|---|---|---|---|---|
| structure `mz` | `wormhole` 204 | **une par pression sur le vide** quand `cg.g_` n'est pas vide (`bg:ldown`) ; meurt au `dg` qui suit son relâcher (refusé) ou 26 ticks plus tard (ouvert) | `ok.lheld` tant qu'il est tenu ; registres ; `fo` d'un `jc` qu'il survole ; `g_bv_et.fo` (écrit, jamais lu) | **arène de prise**, recyclée à sa mort |
| conteneur `ox` | `wormhole` 39 | une par `mz` | le `mz` seul | **dans l'arène** |
| éléments de `ox` | `wormhole` 40 | une fois par `mz`, jamais réécrits | le `mz` seul | **dans l'arène**, capacité réservée exacte |

- **Pourquoi pas la génération.** C'est une allocation par action. Mis tel quel en génération, chaque pression ratée sur le vide la ferait grandir d'au moins 280 o.
- **Pourquoi pas une sorte nouvelle de la zone particules ou interface.** Les particules sont à 130 822 o sur 131 072 depuis 4e, l'interface à 335 Ko sans marge. Une arène de prise existe déjà, recyclée, prise à la génération et remise à zéro avec elle. Elle ne demande ni sorte, ni taille, ni contrôle de somme nouveaux.
- **Pourquoi pas la zone frame pour `ox`.** `ox` est relue à chaque frame tenue, à travers plusieurs remises de la zone frame.
- **Pourquoi pas une valeur pour `ox`.** Sa taille dépend de la partie (n de 1 à ~120).

### Construction dans l'arène de prise

- `pico8_mz_struct_new` appelle `pico8_jo_prise_ouvrir()`, alloue la structure par `pico8_jo_zone_construction()`, initialise comme aujourd'hui, appelle `mz:pq` (qui alloue `ox` par la même zone), puis `pico8_jo_prise_fermer()`, **avant** `pico8_ke_struct_pq` : les registres ont leur propre poignée. C'est la forme de `pico8_jc_struct_new` à 4093382.
- **Aucune prise ne peut déjà être ouverte.** `bg:ldown` est une pression sur le vide ; la prise d'icône n'est ouverte qu'autour du `new_entity` d'`ie:ldown`, et celle du `jc` qu'à l'intérieur de `pico8_jc_struct_new`, qui ne naît que de `jo:ldown`/`nr:ldown`. L'`assert` « une prise d'icone est deja ouverte » le garde.
- Un `mz` fait par un TU (`test_pico8_mz`, `test_pico8_jc`, `test_pico8_struct_header`) prend aussi une arène. Le prototype garde ces TU verts sans changement.
- **Taille** : le lot est de 344 + 8 × n o ; aucun relèvement de `PICO8_JO_PRISE_OCTETS`. Le dépasser (n > 213) sort par `pico8_fatal` « zone prise pleine », qui se lit comme une galaxie à plus de 213 ancrages possibles et se relève d'un chiffre. Le contrat de la constante devient « arène où une icône tenue, un slipway tiré ou un trou de ver construit son objet ».
- **`ox` réservée** : `mz:pq` fait deux passes sur `hv` avec le même prédicat (`fx` non nul) : la première compte, la seconde remplit une liste réservée à ce compte. Le conteneur `ox` est alloué dans l'arène (il reste un pointeur de la structure : aucune forme de `pico8_mz_t` ne change). Avec n = 0, la liste reste vide sans éléments alloués, comme aujourd'hui avant le premier ajout.
- **La fonction de réservation** de `pico8_ents` (nom laissé à l'implémenteur, sur le modèle de `pico8_ptrs_init_reserve`) : prend la zone et une capacité ≥ 0, alloue les éléments d'un coup ; ajouter au-delà fait grandir dans la zone, comme `ents_add`. Contrat à écrire.

### Le `rendre` du trou de ver

- `mz` gagne `rendre` dans sa classe. Il entre donc dans la file de sortie quand `he` (mort) ou le `jl()` d'`eu`/`_init` (abandon) le retirent de `hv`. Au vidage, il passe d'abord par `oublier`, puis par son `rendre`.
- Le `rendre` ne fait qu'une chose : **remettre l'arène de sa liste `ox` (`ox->zone`) dans les prises libres, sans condition d'état.** Il n'a ni liste `qy`, ni rangée, ni traînée à rendre.
- **Pourquoi sans condition.** Le test « encore kz » de 4d existe parce qu'une structure **posée** prête sa mémoire après sa mort : un éclat `ga` né par `jo:hu` emprunte son `p`, son `ra` et son `id`. Rien n'emprunte la mémoire d'un `mz` :
  - `ij` et les éléments de `ox` désignent d'autres entités, pas lui ;
  - `et` désigne un `jc`, la cible jetable statique ou `mz.jj` (statique) ;
  - `mz.p` est une boîte de `bv` (hors arène, 4h), `mz.ra` est `ok.qa` ;
  - aucun objet ne garde un pointeur dans un `mz` (inventaire ci-dessous).

  Un `mz` ouvert (« pr ») échouerait au test « kz », et son arène resterait prise jusqu'à la fin de la génération : +1 arène par trou de ver ouvert, soit une croissance par action.
- **La fonction de remise.** `pico8_jo_prise_zone_rendue(zone, me)` est séparée en deux : un cœur public qui ne prend que la zone (reconnaître une arène nommée « prise », la mettre dans la liste, avec l'`assert` de capacité), et la fonction existante qui teste `me == "kz"` puis appelle ce cœur. Les appelants actuels (`pico8_jo_prise_rendue`, `rendre` du `jc`) ne changent pas. Le `rendre` du `mz` appelle le cœur. **Écarté** : passer `"kz"` à `pico8_jo_prise_zone_rendue` depuis le `mz`, un faux état qui ment au lecteur. Le nom du cœur est laissé à l'implémenteur.
- **Oubli** : rien à ajouter. `pico8_jo_oublier_prises()` vide déjà la liste partagée au point d'oubli de partie, au chargement et à l'arrêt.
- **Borne de recyclage.** Un `mz` refusé à la frame N (dans `ok:lj`, `ey`) est retiré par le `dg` de N+1 et rendu au vidage de N+2. Un `mz` ouvert est retiré au `dg` qui suit `t > 25`, rendu au vidage suivant : **28 frames après son relâcher**.
- **Angle mort ASan**, comme en 4d et 4e : la pression suivante réalloue aussitôt les mêmes adresses. Un pointeur oublié vers un `mz` rendu lirait le nouveau sans planter. C'est l'inventaire, pas ASan, qui garantit qu'il n'y en a pas.

### Liste des prises libres : capacité 4 → 8

- **La lecture.** Les arènes recyclables en vol à un instant sont celles des objets non posés et pas encore rendus :
  - un objet tenu (`ok.lheld` : icône, `jc` ou `mz`), au plus un ;
  - un objet refusé qui attend son vidage (deux frames), au plus un de plus, puisqu'une pression tombe au plus tôt la frame suivante ;
  - des `mz` ouverts : chacun tient son arène 28 frames après son relâcher, et un `mz` ne s'ouvre que tenu 15 ticks. Deux au plus sont donc ouverts à la fois.

  Cela fait **4** au pire, quand tous meurent avant toute nouvelle prise : exactement la capacité actuelle. Les e2e et les fixtures en montrent 2 au plus (prototype).
- **Décision** : `PRISES_LIBRES_MAX` passe à **8**, avec un message d'`assert` inchangé. Coût : 32 o de tableau statique, hors zones. Ce n'est pas une zone : rien à compter dans la somme des 1,5 Mo.

### La coupe de `fo`, vérifiée pour le trou de ver

`mz:hq` appelle `bv`, qui accroche `fo` sur sa cible `et` (`pico8_probe_et_hang_fo`).

| Champ qui peut tenir un `mz` | Qui l'écrit | Couvert par |
|---|---|---|
| `fo` d'une planète de `cg.np` | — | **ne peut pas** : la description du `mz` a `dw = 0`, donc `bv` ne prend jamais une planète pour `et` (`nq < 0` est faux) |
| `fo` d'un `jc` | `bv` quand `et` est un slipway survolé | `jc:kj` le remet à nil **sans condition**, en fin de dessin (`slipway_display` 175 à 4093382), dans la frame même ; un `mz` refusé n'est recyclé que deux updates plus tard, un `mz` ouvert n'est plus tenu donc plus `hq` |
| `g_bv_et.fo` (`probe`) | `bv` quand `et` est la cible jetable | **écrit seulement, jamais lu** (4a) |
| `mz.jj` (`g_mz_jj_wrapper`) | — | `mz:hq` y fait pointer `et` **après** `bv`, et `pico8_probe_et_hang_fo` n'écrit `fo` que sur une planète, un `jc` ou la cible jetable : jamais sur `jj` |
| `ok.lheld` | `ui:bh` | remis à nil par `ui:bh` après `ey` |

Une planète hors écran ne peut donc pas garder un `fo` vers un `mz` : le cas TROU-26 de 4d n'a pas d'analogue ici. Le `rendre` fait tout de même passer le `mz` par `oublier`, qui couvrirait une cible planète si `dw` changeait.

### Relevé « éclats `ga` rendus en cours de partie », refait pour 4f

Lu à 4093382. Un `mz` vit au plus ~45 frames ; peut-il désigner un éclat dont la place est rendue pendant ce temps ?

| Champ du `mz` | Peut-il désigner un éclat ? | Pourquoi |
|---|---|---|
| éléments de `ox`, `ij` | non | `ox` ne tient que des entités `fx` ; un éclat est fait par `pico8_jo_struct_init` avec `fx = 0` et ne le gagne jamais (pas de `ly`, pas de slipway). `ij` est un élément de `ox` |
| `et` | non | `dw = 0` : jamais une planète de `cg.np` ; sinon un `jc` (`cg.jc`) ou une cible statique |
| `fo` accroché par le `mz` | non (sens inverse) | jamais sur un éclat (ni dans `cg.np` comme cible, ni `jc`) |

**Éléments de `ox` qui meurent pendant la vie du `mz`.** Une planète de `ox` remplacée par `jo:hu` (posée sur une structure, « harvest ») est rendue, mais sa mémoire reste vivante : elle est en génération, ou dans l'arène d'une structure posée (état ≠ « kz », jamais recyclée). La sonde `g_` est encore prise à la libc et jamais libérée (4h). `ij` et `ox` ne lisent donc jamais de mémoire rendue en 4f. C'est l'objet du « Reçu » pour 4h.

### Inventaire du §4 pour 4f

Trois durées de vie finissent sur des objets de 4f :

- **l'arène d'un `mz` mort** : remise dans les prises libres par son `rendre`, puis vidée et réutilisée par la prise suivante (icône, `jc` ou `mz`) ;
- **l'arène d'un `mz` abandonné** : remise dans les prises libres au vidage qui suit l'`_init` ou l'`eu` qui l'abandonne, puis oubliée par `pico8_jo_oublier_prises()` au point d'oubli de partie, et rendue avec sa génération ;
- **la génération** : comme en 4a.

**Méthode.** Refaire **mécaniquement**, sur le code du commit, les deux `grep` de 4a sur les fichiers de `CORE_SRCS` :

- pointeurs `static` : `grep -nE '^\s*static\s[^(]*\*\s*[a-zA-Z_][a-zA-Z0-9_]*(\[[^]]*\])*\s*(;|=)'`, hors `const char *` et `* const` ;
- structures `static` dont on lit les champs pointeurs : `grep -nE '^static\s+(struct\s+)?[a-z0-9_]+(_t)?\s+[a-zA-Z_][a-zA-Z0-9_]*\s*(\[[^]]*\])*\s*(;|=)'`, hors `const` et scalaires.

Y ajouter le `grep` des écritures de `fo` et d'`et` (`->fo = `, `pico8_probe_et_hang_fo`, `->et = `), des lecteurs de `->ox` et `->ij`, et des appels de `pico8_mz_new`.

**Relevé fait à 4093382** : 20 pointeurs `static` dans `CORE_SRCS`, 37 lignes de structures `static`, exactement les mêmes qu'à 7fa10bb (4e n'a ajouté ni pointeur ni structure de portée fichier ; son point souris est un membre du `jc`).

| Pointeur | Peut viser | Statut |
|---|---|---|
| `g_jo_prise` (`planet`) | l'arène d'un `mz` en construction | non nul seulement entre `ouvrir` et `fermer`, que `pico8_mz_struct_new` appelle tous deux |
| `g_jo_prises_libres[]` (`planet`) | arènes de `mz` rendus | n'y entrent que des arènes déjà rendues ; vidée au point d'oubli de partie, au chargement, à l'arrêt. **Fenêtre nouvelle** : après l'`_init` qui abandonne un `mz`, la liste tient une arène de l'ancienne génération jusqu'au vidage qui suit l'`eu` (prototype : rendue à f27, oubliée à f32). Aucune prise ne s'ouvre dans cette fenêtre : l'écran titre n'a pas de `bg`, et le clic qui lance `eu` est pris par le bouton du menu. Le cas « refusé, restart, nouvelle partie, pression » le teste |
| `g_ok` → `ok.lheld` | le `mz` tenu | remis à nil par `ui:bh` après `ey` ; c'est le nouvel `ok` après la bascule |
| `g_ok` → `ok.o_`, `ok.gj` | — | `mz` n'a pas `hd` : jamais dans `cv.hd`, jamais survolé |
| `g_hv`, `g_cv[].list`, `g_cg[].list` (`entity`) | des `mz` | retirés par `he` avant l'entrée en file ; refaits au `jl()` d'`eu` |
| `g_file[]` (`file_sortie`) | un `mz` mort ou abandonné | vidé avant le rendu de la génération, et avant tout recyclage (le recyclage est fait **par** le `rendre`) |
| `g_bv_et.fo` (`probe`) | un `mz` | écrit seulement, jamais lu (4a) |
| `fo` des `jc` | un `mz` | voir « La coupe de `fo` » |
| `g_mz_jj_wrapper` (`wormhole`) | — | statique ; son en-tête n'a aucun champ écrit (voir « La coupe de `fo` ») |
| `g_jc_too_far.fo` (`slipway`) | — | n'est écrit que par un `jc` |
| `g_mo`, `g_je`, `g_kc`, `g_ju`, `g_dk`, `g_fm`, `g_gb[]`, `g_ie_gy`, `g_jo_oc`/`g_jo_ir`, `g_arenes[][]`, `partie_choisie`, zones et poignées, dispatch | — | ne peuvent pas viser un `mz` ni son arène, comme à 4a, 4c, 4d et 4e |

**Objets d'autres zones qui désignent un `mz`** : aucun. Un `mz` n'a ni rangée (zone interface), ni liste `qy` ou traînée (zone particules), ni étiquette autre que `mz` (`cg.mz` n'est lu que par les TU). `ox` d'un autre `mz` ne le contient pas : `mz` n'a pas `fx`.

**Conclusion** : ni le point d'oubli de partie ni `oublier` ne gagnent d'entrée. Un pointeur trouvé ensuite par ASan sera ajouté au point d'oubli et noté dans « Découvert » comme un **trou de l'inventaire** (§4). L'inventaire est refait sur le code du commit au morceau de clôture.

### Verrou

- `tools/alloc_guard_exceptions.txt` : retirer `src/pico8_wormhole.c 3`.
- **Prévision : 13 / 36 → 12 / 33**, mesurée par `make alloc-guard` sur le prototype à 4093382 (« verrou objets 65 allouent 12 appels 33 »). L'arène ne change aucun compte : elle est prise à la génération.

### Budget : mesure, projection, décision

- **Mesure par prototype** à 4093382, sur une copie `git archive` hors dépôt. Le prototype met en œuvre la conception de cette PRD : construction dans la prise (2048 o), `ox` réservée exactement, cœur de remise sans état extrait de `pico8_jo_prise_zone_rendue`, `rendre` du `mz`, liste libre à 8, fonction de réservation de `pico8_ents`. Les 26 e2e ont été rejoués avec `PICO8_REMPLISSAGE`, contre la même mesure sur 4093382 non modifié :
  - les 26 `*.sha256` sont identiques ; `sanitize-net` répond « 26 propre(s) » ; `make test` est vert une fois réécrits les deux cas de prise (voir Testing Decisions) ;
  - **`max_octets partie` sur les 26 e2e : 130 720 o** (`pause_restart`, sans `mz`, inchangé), donc **`M_4f` = `M_4e`** ; `K_4f` = 0 ;
  - vingt `mz` construits sur 18 e2e (deux sur `slipway_build` et `trade_rich_mine`) ; `ox` compte **3 éléments au plus** ;
  - génération finale : **+2 048 o** (une arène) sur dix e2e (`colony_income_offscreen`, `colony_panel`, `empty_planet_fade`, `project_tooltip_inputs`, `slipway_refused`, `space_fold_same_spot`, `tech_geoharvesting_burst`, `tech_nanomaterials_discount`, `tech_replication_mechanize`, `tech_xenofoods_recipes`) ; **+4 096 o** sur `slipway_build` (le `mz` ouvert à f20 vit encore quand celui de f39 naît) ; **+0** sur les sept autres (`slipgate_outgoing_first`, `synthesizer_second_link`, `tech_infraspace_infragate`, `tech_slipgates_relay`, `tech_starbirth_protostar`, `tech_trade_league_hub`, `trade_rich_mine`), où une prise posée ensuite reprend l'arène du `mz` mort ;
  - **`trade_rich_mine` reste à 95 312 o** : ses deux `mz` laissent deux arènes que les labos posés de f100-111 reprennent, comme ils en auraient pris deux neuves ; sa fenêtre f139-f151 (4e) reste « + `PICO8_JO_PRISE_OCTETS` » ;
  - prises libres : **2 au plus** ;
  - fixture de pliages (f0-3 de `slipway_build`, puis vingt cycles « pression en (80,55), quatre frames tenues, relâché, trois frames », puis un `mz` tenu seize frames et ouvert, trente-deux frames d'attente, une pression) : génération **67 184 o** après un refus, après vingt refus, et après l'ouvert et la pression qui suit ; le `mz` ouvert est rendu à f188, 28 frames après son relâcher ;
  - fixture d'abandon (f0-20 de `slipway_build`, puis f2-17 de `pause_restart`) : le `mz` ouvert à f20 est abandonné par l'`_init` de f26 et rendu au vidage de f27 en état « pr » ; dans un enfant ASan, il se relit à f31 (clic « challenging ») et sa relecture à f32 meurt par `use-after-poison`. Les deux rejeux sont propres sous ASan.
- **Projection.**
  - Estimations restantes (recopiées de la PRD 4e) : génération 4g ~6, 4h ~12, soit **18 Ko** ; chargement 4g ~32 Ko.
  - `r` = **1** : le pic ne bouge pas.
  - `P_4f` ≈ 127,7 + 18 ≈ **146 Ko** ≤ 704 ; `Q_4f` ≈ 32 Ko ≤ 64.
  - Zone particules : inchangée (130 822 ≤ 131 072) ; somme des zones 1487 Ko sur 1536.
  - **On continue.**
- **Correction de l'estimation de 4a** (« < 1 Ko par génération ») : le `mz` est par action ; ce qu'il coûte à une génération est le nombre d'arènes de prise en vol à la fois, partagées avec les icônes et les slipways, soit au plus 4 × 2 Ko = **8 Ko** par partie au-delà de ce que les icônes et slipways prennent déjà, quel que soit le nombre de pressions.
- **Mesure à refaire** après conversion : `M_4f`, `K_4f`, le maximum de `ox->count` et des prises libres. Les noter dans le commit et dans « Découvert », comparés au prototype ; un écart s'explique avant la projection.
- **Décision**, règle de 4c, dans l'ordre :
  - si `P_4f ≤ 704` et `Q_4f ≤ 64` : on continue ;
  - sinon, déplacer la frontière entre génération et chargement ;
  - sinon, relever d'un chiffre sur les 49 Ko ;
  - sinon, **arrêt de l'étape 4 à 4f** et signalement à l'humain (zone, limite voulue, somme actuelle 1487 Ko sur 1536).
- Un `pico8_fatal` « zone partie pleine » pendant 4f se lit d'abord comme un `mz` entré tel quel en génération (que montre la fixture de pliages), ensuite seulement comme une limite trop basse. Un « zone prise pleine » se lit comme un `ox` plus grand que l'arène.

### Note « Découvert » et clôture

À la clôture, sous l'entrée 4 de `docs/plan-4.md`, une note « *Découvert (4f) :* » de quelques phrases :

- placement : `mz` et `ox` (réservée exactement) dans l'arène de prise partagée, recyclée à sa mort sans condition d'état ; prises libres portées à 8 ;
- correction de la table de 4a : `mz` est par action (chaque pression sur le vide quand une sonde vole), pas une fois par partie ; il meurt au plus 28 frames après son relâcher ;
- le `rendre` du `mz` ; l'inventaire et le relevé « éclats `ga` » refaits (`dw = 0` : jamais de `fo` sur une planète) ;
- les deux TU de prise réécrits (une arène libérée par un `mz` est reprise par la pose) ;
- les mesures et la décision : `M_4f`, `K_4f`, `P_4f`, `Q_4f`, `r` ; l'estimation de 4a contre la mesure ;
- les comptes du verrou.

Le §2 est corrigé : ligne « Partie », « trous de ver » devient « trous de ver (dans une arène de prise, recyclée à leur mort) ».

Sous l'entrée 4, une ligne « *Reçu de l'étape 4f :* » **pour 4h** :

- `ox` et `ij` d'un `mz` désignent les sondes `g_` (et `fv`), qui ont `fx` ; si 4h range une sonde dans une place recyclée, un `mz` vivant peut la garder dans `ox` jusqu'à sa mort (~45 frames) ;
- `mz.p` est la boîte de `bv` refaite à chaque frame tenue, `mz.ra` est `ok.qa`.

La PRD est retirée de `docs/prd/` dans le commit de clôture.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 :

- `make test-all` et `make sanitize-net` passent ;
- les 26 `*.sha256` sont identiques ;
- `alloc-guard` est à égalité stricte.

Aucun morceau ne passe la transitoire à un nouvel appelant.

1. **Réservation de `pico8_ents`.**
   - Les tests d'abord, dans le TU de `pico8_ents` : la liste réservée a la capacité demandée ; n ajouts ne font pas bouger les octets pris de sa zone ; un ajout de plus grandit ; une capacité 0 ne prend rien.
   - La fonction et son contrat. Aucun compte du verrou ne bouge.
2. **Trou de ver dans l'arène de prise, `rendre`, prises libres à 8.**
   - Les tests d'abord :
     - `test_pico8_mz` : un `mz` neuf a `ox->count == ox->capacity` ; la structure et `ox` sont dans une même arène de prise (la génération prend exactement une arène, ou rien si une prise libre existe) ; l'ordre de `ox` suit `hv` ;
     - la fixture de pliages : génération constante pendant les frames tenues, égale du premier au vingtième refus, et inchangée par l'ouvert et la pression qui suit (rouge si le `mz` entre en génération, ou si l'ouvert n'est pas recyclé) ;
     - la fixture d'abandon : le `mz` se relit à f31, et l'enfant meurt par ASan quand il est relu à f32 (rouge aujourd'hui : le `mz` de la libc n'est jamais libéré) ;
     - refus, « restart », nouvelle partie, pression : l'enfant ne meurt pas ;
     - `une_prise_posee_garde_son_arene` et `un_slipway_pose_garde_son_arene` réécrits (voir Testing Decisions) : ils passent au rouge dès que le `mz` recycle, et ne repassent au vert qu'avec leur nouvelle attente.
   - Le cœur de remise sans état extrait de `pico8_jo_prise_zone_rendue`, appelé par elle et par le `rendre` du `mz`.
   - `pico8_mz_struct_new` entre `pico8_jo_prise_ouvrir`/`_fermer` ; `mz:pq` en deux passes, `ox` réservée ; `rendre` dans la classe ; `PRISES_LIBRES_MAX` à 8 ; contrats réécrits (`PICO8_JO_PRISE_OCTETS`, module trou de ver).
   - Le verrou : `wormhole` sort, **13 / 36 → 12 / 33**.
3. **Mesure et clôture.**
   - Le relevé sur les 26 e2e, `M_4f`, `K_4f`, le maximum de `ox->count` et des prises libres, la comparaison au prototype, la projection et la décision, dans le message du commit.
   - L'inventaire refait par `grep`.
   - La note « Découvert (4f) », la correction du §2, la ligne « *Reçu de l'étape 4f* » pour 4h, le retrait de cette PRD.

Le morceau 2 dépend du 1. Le 3 dépend des deux.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris de la zone partie (`partie_sans_registres`, `pico8_zone_partie_prise`, les lignes de `PICO8_REMPLISSAGE`) ;
  - les champs publics d'un `mz` (`ox`, `ox->count`, `ox->capacity`, `ij`, `oi`, `header.me`, `p_`) ;
  - un rapport ASan dans un processus enfant ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne lit ni les bouts de la zone ni une adresse, sauf pour la relire après sa mort. Les tailles attendues se calculent dans le TU (`sizeof`, `PICO8_JO_PRISE_OCTETS`), jamais en dur : 344 et 2048 ne valent que sur x86-64.
- **`tests/test_pico8_places_vivantes.c`** (ASan, un enfant par cas, `demarrer`/`jouer_jusqu_a`, `partie_sans_registres`, `dans_un_enfant`, `dans_un_enfant_qui_meurt_par_asan`).
  - **Pliages refusés et ouverts** (cas nouveaux), sur une fixture nouvelle `tests/fixtures/pliages.json`, construite comme celle du prototype : f0-3 de `slipway_build` (sonde `g_` à f2) ; vingt cycles « pression en (80,55), quatre frames tenues, relâché, trois frames » ; un `mz` tenu seize frames en (80,55), relâché et ouvert ; trente-deux frames d'attente ; une pression ; « restart » (f2-17 de `pause_restart`), puis une pression sur le vide. Ce n'est pas un e2e : elle n'ajoute pas de ligne couverte. Les cas :
    - génération constante pendant les frames tenues d'un cycle ;
    - égale après le premier et après le vingtième refus ;
    - égale avant l'ouvert et après la pression qui suit sa mort ;
    - la traversée « restart », nouvelle partie, pression ne fait pas mourir l'enfant.
  - **Trou de ver abandonné** (cas nouveaux), sur une fixture `tests/fixtures/pliage_abandonne.json` (f0-20 de `slipway_build`, puis f2-17 de `pause_restart`, 37 frames) : on prend le `mz` de `cg.mz` à f22 ; il se relit à f31 (clic « challenging », bascule) dans `dans_un_enfant` ; l'enfant meurt par `use-after-poison` quand il est relu à f32, dans `dans_un_enfant_qui_meurt_par_asan`. Sur le modèle de `pause_restart_garde_les_planetes_de_la_partie_quittee_pendant_le_clic` et `pause_restart_rend_les_planetes_de_la_partie_quittee`. Les deux cas ont tourné dans le prototype.
  - **`une_prise_posee_garde_son_arene`** (4d) **est réécrit.** Sa fixture `prises_refusees.json` commence par f0-20 d'`empty_planet_fade`, dont le `mz` (f4, ouvert à f20) est rendu à f48 : à f161, deux arènes sont libres, et la structure posée en reprend une. Le prototype mesure 52 944 → 52 992 o, soit « by/cc 48 » seulement : l'attente devient « + listes ».
  - **`un_slipway_pose_garde_son_arene`** (4e) **est réécrit.** Sa fixture `slipways_refuses.json` commence par f0-34 de `slipway_build`, dont le `mz` de f4 est rendu à f48, pendant les refus : à f335, deux arènes sont libres, le slipway posé en reprend une et le refus suivant l'autre. Le prototype mesure 53 152 → 53 216 o, soit « oe 64 » seulement : l'attente devient « + `oe` ». Le nom et le message disent que la pose reprend une arène libre.
  - Le nom ou le message de ces deux tests dit pourquoi, pas un commentaire de récit (règle du `CLAUDE.md`).
  - **Inchangés, verts dans le prototype** : `message_box_hold` f45 = f90 ; `game_end_final_score` f2 = f51 ; les égalités exactes de 4c/4d sur `pause_restart` (aucun `mz`) ; `trade_rich_mine_bd_sur_des_routes_ne_fait_pas_grandir_la_partie` (f139-f151 = `PICO8_JO_PRISE_OCTETS`) et les autres cas `trade_rich_mine` de 4e ; `prises_refusees_ne_font_pas_grandir_la_partie` (l'arène du `mz` est prise à f4, avant f27) ; `le_fo_d_une_planete_hors_ecran_oublie_la_prise_refusee` (le `mz` vit encore à f28) ; `un_slipway_tenu_ne_fait_pas_grandir_la_partie`, `slipways_refuses_ne_font_pas_grandir_la_partie` (mesuré à f49, après le rendu du `mz` à f48 : 53 152 = 53 152), `le_fo_d_une_planete_hors_ecran_oublie_le_slipway_refuse`, `un_slipway_refuse_traverse_une_nouvelle_partie` ; `empty_planet_fade_ne_monte_pas` ; la sentinelle de 4a.
- **`tests/test_pico8_mz.c`** : réservation et arène (voir morceau 2). Les attentes existantes sur `ox` (entrées, sonde et planète éclairée listées, `mz` absent) restent vertes dans le prototype.
- **`tests/test_pico8_ents.c`** : la réservation (morceau 1), sur le modèle du cas de réservation de `pico8_ptrs`.
- **Verrou** : `make alloc-guard` à égalité stricte, avec la liste sans `wormhole`.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. Dans le prototype à 4093382, `make test` échoue exactement sur les deux cas de prise que 4f réécrit.
- **Exemples dans le dépôt** :
  - `tests/test_pico8_places_vivantes.c` : rejeu par enfant, les cas de prise de 4d et de slipway de 4e, les cas « planètes de la partie quittée » de 4d et la sentinelle de 4a ;
  - `tests/fixtures/prises_refusees.json`, `tests/fixtures/slipways_refuses.json` : la forme d'une fixture de prises ;
  - `tests/test_pico8_ptrs.c` : le cas de `pico8_ptrs_init_reserve`.

## Out of Scope

- La boîte `p` de `bv` faite à chaque frame tenue (`probe` 162), `mz.ra` = `ok.qa`, la sonde `g_` et `fv` : 4h.
- L'arène de prise elle-même, sa taille, `pico8_zone_arene_vider`, la coupe de `fo` (TROU-26), le `rendre` du `jc` : 4d et 4e, versés. 4f n'y touche que pour extraire le cœur de remise sans état de `pico8_jo_prise_zone_rendue` et porter la liste libre à 8.
- `mz.jj` (`g_mz_jj_wrapper`) : statique, sans écrivain, ne change pas.
- L'audio (`rn(53)` de `mz:pq`) : hors périmètre du portage.
- Relever `PICO8_ZONE_GAME_BYTES` ou `PICO8_JO_PRISE_OCTETS`, sauf si la mesure l'exige.
- Retirer TROU-7 : étape 5.
- Lancer l'application raylib.

## Vérifié contre 4e

Relu à `HEAD` 4093382, sans toucher git ni les sources :

- la note « *Découvert (4e)* », la ligne « *Reçu de l'étape 4e* » et les lignes « Frame » et « Partie » du §2 de `docs/plan-4.md` ;
- `git log -7 --stat` (d0fd7e7, d819338, aae9612, 4093382) ;
- `src/pico8_planet.{h,c}` : `PICO8_JO_PRISE_OCTETS`, `pico8_jo_prise_zone_rendue`, `pico8_jo_prise_rendue`, liste de 4 prises libres, `pico8_jo_oublier_prises` ;
- `src/pico8_slipway.c` (`rendre` du `jc`, ouverture et fermeture de la prise), `src/pico8_slipway_display.c` (`fo` remis à nil) ;
- `src/pico8_ents.h`, `src/pico8_cart.c`, `src/pico8_wormhole.{h,c}` : aucun changement depuis 7fa10bb ;
- `tools/alloc_guard_exceptions.txt` et `make alloc-guard` sur une copie `git archive HEAD` : 65 objets, 13 allouent, 36 appels ;
- `tests/fixtures/slipways_refuses.json` et les cas de 4e de `tests/test_pico8_places_vivantes.c`.

Les hypothèses du brouillon, une par une :

- **H-4e-1 (taille de l'arène) — tenue.** `PICO8_JO_PRISE_OCTETS` vaut 2048, avec le contrat « a held icon or a drawn slipway ». 4f y ajoute le trou de ver.
- **H-4e-2 (retour d'arène et liste partagée) — tenue.** Une seule liste, de 4 places ; la fonction « zone, état » est `pico8_jo_prise_zone_rendue(zone, me)`, appelée par `pico8_jo_prise_rendue` et par le `rendre` du `jc` ; `pico8_jc_struct_new` ouvre et ferme la prise ; `pico8_jo_oublier_prises()` reste aux trois points d'oubli. 4f en extrait le cœur sans état.
- **H-4e-3 (`jc` et `fo`) — tenue.** Le `jc` a un `rendre` ; `pico8_jo_oublier_fo` n'a pas changé ; `jc:kj` remet `fo` à nil sans condition (ligne 175).
- **H-4e-4 (verrou) — tenue.** 13 / 36 ; `wormhole` 3, aux lignes 39, 40, 204 (fichier inchangé). Le prototype mesure **12 / 33**.
- **H-4e-5 (budget) — tenue.** `M_4e` = 130 720, `trade_rich_mine` 95 312, zone particules 130 822, projection ≈ 147 Ko, « on continue ». `game_end_bankrupt` vaut 84 048 o, comme prévu par la PRD 4e.
- **H-4e-6 (TU de 4e) — tenue, et une conséquence de plus.** La fixture de refus commence bien par f0-34 de `slipway_build`. Comme prévu, `un_slipway_pose_garde_son_arene` passe au rouge avec le recyclage du `mz` (attente « + arène + `oe` », mesure « + `oe` ») et la fenêtre f139-f151 de `trade_rich_mine` ne bouge pas. Le prototype refait montre aussi que le test « 1/20 refus » reste vert : il mesure à f49, après le rendu du `mz` à f48.
- **H-4e-7 (« Reçu ») — tenue.** Le seul « *Reçu de l'étape 4e* » vise 4h (`ok.qa`) ; rien pour 4f.
- **H-4e-8 (fréquences) — tenue.** Tous les `jc` naissent de `pico8_jc_struct_new`, qui ouvre sa propre prise ; aucune prise n'est ouverte sur une pression sur le vide.
- **H-4e-9 (réservation) — tenue.** `pico8_ents.h` n'a pas de réservation ; `ts` est réservée par `pico8_ptrs_init_reserve`. Le morceau 1 reste.
- **H-4e-10 (outillage de test) — tenue.** `demarrer`, `jouer_jusqu_a`, `partie_sans_registres`, `dans_un_enfant`, `dans_un_enfant_qui_meurt_par_asan` sont inchangés ; les fixtures gardent leur forme.

**Ce que la vérification change dans cette PRD** :

- la fonction de remise s'appuie sur `pico8_jo_prise_zone_rendue`, dont 4f extrait le cœur sans état ;
- le second TU réécrit est nommé (`un_slipway_pose_garde_son_arene`), avec ses mesures ;
- le prototype est refait à 4093382 avec l'arène de 2048 o : +2 048 / +4 096 / +0 par e2e, `trade_rich_mine` inchangé, pliages à 67 184 o constants, abandon vérifié par un enfant ASan ;
- le verrou part de 13 / 36 ;
- les morceaux n'attendent plus 4e.

## Further Notes

- **Pas d'humain disponible, et pas d'outil `Agent` dans cette session pour lancer un agent `guideline` neuf.** J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage).
  - **Hypothèses explicites** : celles sur 4e sont vérifiées (« Vérifié contre 4e »). Les fréquences ont été lues dans le cart : `bg:ldown` l. 376-377, `mz` l. 752-797 (`pq` 764, `hq` 767, `ey` 774, `pr` 776-781), `bv` l. 552-558, `d_` l. 569-575, `dg`/`he` l. 163-182, `jp` l. 31-36, `jo:hu` l. 451-455. Il en sort une correction de la table de 4a : le `mz` est par action.
  - **Simplicité** : aucun mécanisme neuf. L'arène de prise de 4d/4e, un `rendre` d'une ligne, un cœur de remise extrait d'une fonction existante, une réservation sœur de celle de `pico8_ptrs`, une constante relevée. Écartés : une sorte de particules (zone pleine depuis 4e), une arène propre aux trous de ver (même code en double), `ox` en zone frame (relue sur plusieurs frames), un faux état « kz » passé au retour d'arène.
  - **Changements chirurgicaux** : 3 relocations dans un fichier, plus le cœur de remise du module planète, la capacité de la liste libre, une fonction de `pico8_ents` et deux attentes de TU. La forme de `pico8_mz_t` ne change pas.
  - **Critères vérifiables** : chaque morceau a ses tests rouges d'abord, et le prototype à 4093382 a montré les attentes (sha256, ASan, verrou, génération constante sur vingt refus, abandon qui meurt par `use-after-poison` à f32).
- Le prototype à 4093382 (copie `git archive` hors dépôt) a aussi instrumenté les créations et rendus de `mz` (frames, état, `ox->count`) et la longueur de la liste libre ; ces mesures sont reprises dans « Budget ». Rien de ce prototype n'est dans le dépôt.
