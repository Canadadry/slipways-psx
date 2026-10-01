---
title: "Plan 4, étape 4f — Trous de ver dans l'arène de prise partagée, liste ox réservée, rendus à leur mort"
description: "Un trou de ver mz naît à chaque pression sur le vide quand une sonde vole, et meurt au plus 28 frames après son relâcher ; sa structure et sa liste ox sont encore prises à la libc et à la transitoire, et ne sont jamais rendues. 4f construit chaque mz, ox compris (capacité réservée exacte), dans l'arène de prise de 4d/4e, et lui donne un rendre qui remet cette arène dans les prises libres, sans condition d'état."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. L'étape 4 range les objets de partie dans la **zone partie**. Cette PRD traite la sous-étape **4f, trous de ver**.

**Elle est rédigée pendant 4e.** La PRD de 4e (`docs/prd/triage/phase1-etape4e-slipways-et-routes.md`) est en cours d'implémentation. Tout ce que cette PRD suppose de l'issue de 4e est écrit comme hypothèse `H-4e-n` (section « Hypothèses sur l'étape 4e »). Ce qu'il faudra relire une fois 4e finie est listé dans « À revérifier une fois 4e finie ». Tout le reste a été lu sur le contenu commité à `HEAD` 7fa10bb (`git archive HEAD`), jamais sur l'arbre de travail.

**Le texte de l'étape**, recopié du plan à 7fa10bb (entrée 4, extraits qui concernent 4f) :

> 4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement, et si son remplissage ne monte pas pendant une phase sans action d'un e2e.
>    Elle se fait en sous-étapes, une PRD chacune :
>    - **4a. Socle.** […]
>    - **4b et suivantes.** Une famille de propriétaires par PRD dans l'ordre fixé par la PRD 4a : 4b états de `pico8_gen.c` ; 4c tableaux croissants et registres (crée la sous-zone chargement) ; 4d planètes et relais ; 4e slipways et routes ; 4f trous de ver ; 4g recherche et économie ; 4h singletons de `jl`/`eu` et objets du joueur (famille ajoutée par 4a : les « Reçu » des étapes 1 à 3 leur reviennent). N'entre en zone partie qu'une allocation faite une fois par objet de partie ; un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
>    - *Découvert (4a) :* la partie bascule en tête de `pico8_eu`, avant son `jl()` ; `_init` (titre, « restart », « again? ») ne bascule pas. L'ancienne génération est rendue au vidage suivant (en jeu, l'update qui suit le clic), après un point d'oubli de partie, vide à ce jour […]. Zone à deux bouts, un seul stockage de 768 Ko ; une double bascule sans vidage est refusée ; `cart_load`/`shutdown` remettent tout à zéro. […] Angle mort ASan : un pointeur gardé à travers deux bascules ne plante plus. […] Les 81 relocations de l'étape 4 sont attribuées aux familles 4b–4h ; la seule marge est de 49 Ko sur la somme des zones […]. Le rejeu écrit `partie` et `max_octets partie`.
>    - *Découvert (4c) :* la zone partie se partage en génération (704 Ko, deux bouts) et chargement (64 Ko, remis à zéro au seul chargement) […]. Blocs morts des agrandissements bornés (prise = G + R exact). […]
>    - *Découvert (4d) :* planètes, `oe`, `p`/`ra`/« p… », listes de `jo:lz` et `jo:cn` en génération ; […]. Une prise d'icône refusée faisait grandir la génération (+616 o) : elle prend une arène de 1536 o recyclée par une liste de 4 prises libres. Trou de l'inventaire du §4 : le `fo` d'une planète hors écran pouvait viser l'objet rendu ; `oublier` le coupe (TROU-26). Mesure : 90 planètes × 536 o = 48 240 o par partie, maximum 130 720 o au clic de `pause_restart` (prévision exacte), 80 464 sur `game_end_bankrupt` ; projection ≈ 177 Ko ≤ 704, chargement 0 : on continue. Comptes : 22 / 75 → 16 / 47.
>    - *Reçu de l'étape 4d :* pour 4e, la route `lw.t` proposée à chaque `bd` ne doit pas entrer en génération ; l'arène de prise et sa liste de prises libres sont réutilisables pour le slipway tiré. […]
>    - *Reçu de l'étape 3 :* la structure `g_` (et `fv`) et sa boîte `p` de `bv` restent à cette étape (`/* TROU-7 */`) ; […]

Aucune ligne « *Reçu* » du plan ne vise 4f à 7fa10bb (voir H-4e-7).

**Les lignes du §2**, recopiées :

> | **Partie** | planètes, relais, slipways, routes `trade`, recherche, trous de ver ; les tableaux croissants (`ents`, `ptrs`, registres) qui les contiennent ; les objets faits au chargement (`gb`, `hp.i`, listes statiques) dans une sous-zone chargement, prise dans la zone partie, remise à zéro par le seul chargement | une génération par partie, de `eu` à `eu` ; rendue d'un bloc au vidage de la file qui suit le `eu` suivant (zone à deux bouts) ; `_init` ne la rend pas |

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

**Ce qui se passe aujourd'hui** (lu à 7fa10bb, cart et port) :

- **Quand naît un trou de ver.** `bg:ldown` (cart l. 376-377, `pico8_bg_ldown`) : une pression du bouton gauche sur le vide fait un `mz` **si `cg.g_` n'est pas vide**, sinon une sonde `g_`. Une sonde posée reste toute la partie : dès la deuxième pression sur le vide, **chaque pression fait un `mz`**. C'est donc un objet **par action**, sans borne par partie, et non « une fois par partie » comme le laissait lire l'estimation de 4a.
- **Combien de temps il vit** (cart l. 752-797, `pico8_wormhole.c`).
  - Tenu (`ok.lheld`), il est en état « kz » ; `mz:hq` (`bv`, puis la recherche de l'ancrage le plus proche dans `ox`) tourne à chaque frame.
  - Au relâcher, `mz:ey` : avant 15 ticks, ou si `d_` échoue (`et` posé, ou pas d'argent), `p_` ; il est **retiré par le `dg` suivant**. Sinon `d_` le passe en « pr » (`dq`), et `mz:pr` met `p_` à `t > 25` : il meurt **26 ticks après le relâcher**.
  - Le prototype le confirme : sur `slipway_build`, relâché ouvert à f20, il est encore vivant à f45 ; sur `trade_rich_mine`, les `mz` de f26 et f51 sont retirés vers f70 et f95 ; sur une fixture « f0-20 de `slipway_build`, puis f2-17 de `pause_restart` », le `mz` ouvert est abandonné par l'`_init` de f26.
- **Ce qu'il alloue.**
  - Sa structure, `pico8_mz_t`, **224 o** sur x86-64 (mesuré par `sizeof` à 7fa10bb), par `malloc` (`pico8_mz_struct_new`, l. 204).
  - Sa liste `ox` (`mz:pq`, cart l. 764 : `jp(hv, function(p) return p.fx end)`) : le conteneur `pico8_ents_t` (24 o) par `malloc` (l. 39), et ses éléments par la transitoire (l. 40), grandis par doublement depuis 4. `ox` tient **toutes les entités de `hv` dont `fx` est vrai** : les planètes de la famille `jo` colonisées ou reliées (`fx` posé par `jo:ly` et le commerce), et les sondes `g_`/`fv` (`fx = 1` dans leur description). Un éclat `ga` a `fx = 0` et n'y entre jamais. **`ox` n'est écrite que par `mz:pq`**, une fois, à la création ; elle est relue par `mz:hq` à chaque frame tenue (`ij`).
  - `mz.p` (la boîte `p` de `bv`) est faite par `pico8_bv` (`probe` 162) à chaque frame tenue : c'est un site de **4h** (table de 4a), que 4f ne touche pas. `mz.ra` est `mp`, c'est-à-dire `ok.qa` (4h aussi).
- **Qui la rend.** Personne : `mz` n'a pas de `rendre`. Retiré de `hv` par `dg`/`he`, il **n'entre pas dans la file de sortie** et fuit (TROU-7), structure et `ox` compris. Il ne passe donc jamais par `oublier`.
- **Le verrou** à 7fa10bb : **16 fichiers, 47 appels** (`make alloc-guard` sur une copie `git archive HEAD` : « verrou objets 65 allouent 16 appels 47 »). Les 3 relocations de 4f sont aux lignes de la table de 4a : `wormhole` 39, 40, 204.

**Le problème.** Le `mz` est une allocation **par action**. La règle des familles lui interdit la génération telle quelle : une pression ratée sur le vide la ferait grandir de 224 + 24 + 32 = **280 o** au moins, à chaque clic, toute la partie, que le §6 interdit d'enregistrer. Il lui faut une **place dans une zone qui rend** : une place rendue à sa mort, réutilisée par la pression suivante. Ce mécanisme existe déjà : c'est l'arène de prise de 4d, que 4e partage avec les slipways (H-4e-1, H-4e-2).

## Solution

- **Le trou de ver dans l'arène de prise partagée.**
  - Chaque `mz` est construit, avec sa liste `ox`, dans une arène de prise ouverte par `pico8_jo_prise_ouvrir()` et prise par `pico8_jo_zone_construction()`, comme le `jc` de 4e et la structure d'icône de 4d. Même liste de prises libres, même taille (`PICO8_JO_PRISE_OCTETS`, 2048 après 4e : H-4e-1).
  - Le lot d'un `mz` fait 96 (en-tête d'arène) + 224 (`pico8_mz_t`) + 24 (conteneur `ox`) + 8 × n, où n est le nombre d'entités `fx` de `hv` à sa création. Il tient dans 2048 o jusqu'à **n = 213** (et dans 1536 o jusqu'à n = 149). Sur les 26 e2e, n vaut **3 au plus**.
- **`ox` réservée exactement.** `mz:pq` compte d'abord les entités `fx` de `hv`, puis réserve `ox` à ce compte, puis la remplit dans le même ordre. Elle ne grandit jamais : `ox` n'est écrite qu'une fois. Sans réservation, la croissance par doublement laisserait des blocs morts dans l'arène (jusqu'à ~2 Ko d'éléments pour n ≈ 120), et la ferait déborder en fin de partie.
  - Une fonction nouvelle de `pico8_ents`, sœur de `pico8_ptrs_init_reserve` (sauf si 4e en a déjà fait une, H-4e-9).
- **Un `rendre` pour le `mz`**, qui le fait entrer dans la file de sortie à sa mort (`he`) ou à l'abandon de sa partie (`jl()` d'`_init`/`eu`), donc dans `oublier`. Il remet son arène dans les prises libres **sans condition d'état** : un `mz` meurt en « kz » (refusé) comme en « pr » (ouvert), et rien n'emprunte sa mémoire après sa mort (voir « Le `rendre` du trou de ver »).
- **Liste des prises libres : capacité 4 → 8.** Un `mz` ouvert garde son arène 28 frames après son relâcher, quand un `jc` ou une icône refusés la rendent en deux. La lecture borne à 4 les arènes recyclables en vol à la fois, soit exactement la capacité actuelle ; 4f prend de la marge (32 o statiques, hors zones).
- **Inventaire du §4** refait pour le `mz`, son arène et sa liste : aucune entrée nouvelle au point d'oubli de partie ni à `oublier`. Une ligne « *Reçu de l'étape 4f* » pour 4h : `ox` et `ij` d'un `mz` désignent des sondes `g_`.
- **Budget** : `M_4f` = `M_4e` = **130 720 o** prévu (`pause_restart` n'a aucun `mz`). Projection **≈ 146 Ko** sous 704 Ko : **on continue**.
- **Verrou** : `wormhole` sort. Mesuré par le prototype à 7fa10bb : **16 / 47 → 15 / 44**. Après 4e (H-4e-4) : **13 / 36 → 12 / 33**.

Aucun pixel ne change, aucun tirage `rnd` n'est déplacé : le prototype à 7fa10bb garde les 26 `*.sha256`, et `make sanitize-net` répond « 26 propre(s) ».

## User Stories

1. En tant que mainteneur du plan 4, je veux que `pico8_wormhole.c` n'appelle plus ni `malloc` ni la transitoire, pour qu'il sorte de la liste d'exceptions du verrou.
2. En tant que développeur, je veux que chaque `mz` soit construit dans l'arène de prise de 4d/4e, `ox` comprise, pour qu'un trou de ver et tout ce qu'il possède vivent et meurent d'un bloc, sans second mécanisme de recyclage.
3. En tant que développeur, je veux que `pico8_mz_struct_new` ouvre la prise avant d'allouer sa structure et la ferme après `mz:pq`, avant l'inscription dans les registres, pour que les registres gardent leur propre poignée.
4. En tant que développeur, je veux que la liste `ox` soit réservée exactement au nombre d'entités `fx` de `hv`, pour qu'elle ne laisse aucun bloc mort dans l'arène et n'y grandisse jamais.
5. En tant que développeur, je veux que l'ordre des éléments de `ox` reste celui de `hv`, pour que `ij` choisisse le même ancrage qu'aujourd'hui en cas d'égalité de distance.
6. En tant que développeur, je veux une fonction de `pico8_ents` qui réserve une capacité à l'initialisation, sœur de celle de `pico8_ptrs`, pour ne pas écrire à la main dans les champs du conteneur.
7. En tant que développeur, je veux que `mz` gagne un `rendre`, pour qu'il entre dans la file de sortie à sa mort comme à l'abandon de sa partie.
8. En tant que développeur, je veux que le `rendre` du `mz` remette son arène dans les prises libres quel que soit son état, pour qu'un trou de ver ouvert (« pr ») soit recyclé comme un refusé (« kz »).
9. En tant que développeur, je veux que la remise d'une arène sans condition d'état soit une fonction du module planète distincte du test « encore kz » des planètes et des slipways, pour qu'aucun appelant ne passe un faux état.
10. En tant que mainteneur, je veux que la liste des prises libres passe de 4 à 8 places, parce que deux `mz` ouverts, un objet tenu et un refusé en attente de vidage peuvent être en vol à la fois, pour qu'aucun `assert` ne tombe en partie longue.
11. En tant que mainteneur, je veux qu'une suite de vingt trous de ver refusés laisse la génération constante, pour prouver que le `mz` n'entre pas en génération à chaque pression.
12. En tant que mainteneur, je veux que la génération ne bouge pas pendant les frames où un `mz` est tenu, pour prouver que `mz:hq` n'alloue rien dans la zone partie (la boîte `p` de `bv` reste à 4h).
13. En tant que mainteneur, je veux qu'un trou de ver ouvert, mort 26 ticks après son relâcher, rende son arène à la pression suivante, pour prouver que l'état « pr » est recyclé.
14. En tant que mainteneur, je veux qu'un `mz` vivant quand la partie est abandonnée se relise pendant la frame du clic qui lance la nouvelle partie, et fasse planter un enfant ASan quand il est relu après la frame suivante, pour prouver qu'il vit dans la génération et meurt avec elle.
15. En tant que mainteneur, je veux qu'un `mz` refusé, puis « restart », puis une nouvelle partie et une pression sur le vide, ne fasse pas mourir l'enfant ASan, pour prouver que les prises libres de l'ancienne génération sont oubliées avant d'être reprises.
16. En tant que mainteneur, je veux que le contrôle `message_box_hold` f45 = f90 reste vert, pour que la croissance par tick reste nulle.
17. En tant que mainteneur, je veux que les tests de prise de 4d et de 4e qui comptent « exactement une arène de plus » soient réécrits sur ce que leur fixture rend vraiment, pour qu'une arène laissée libre par un trou de ver ne les fasse pas mentir.
18. En tant que gardien de l'invariant, je veux que les 26 `*.sha256` restent identiques, pour prouver que la réservation de `ox` et le recyclage ne changent ni un pixel ni un tirage `rnd`.
19. En tant que gardien de l'invariant, je veux que `make sanitize-net` reste vert sur les 26 e2e, pour que rien ne lise une arène de trou de ver recyclée.
20. En tant que mainteneur du plan 4, je veux que l'inventaire du §4 soit refait mécaniquement pour le `mz` (pointeurs `static`, champs des structures `static`, écritures de `fo` et d'`et`), pour que la liste des références oubliées vienne du `grep` et non de la lecture des seuls sites.
21. En tant que mainteneur du plan 4, je veux que le relevé « éclats `ga` rendus en cours de partie » soit refait pour le `mz`, pour savoir si `ox`, `ij` ou `et` peuvent désigner un éclat rendu.
22. En tant que mainteneur du plan 4, je veux le maximum de la zone partie et de la sous-zone chargement relevé sur les 26 e2e après conversion, avec la projection et la décision, pour que le budget repose sur une mesure.
23. En tant que mainteneur du plan 4, je veux que la note « Découvert (4f) » corrige l'estimation de 4a (« < 1 Ko par génération ») : le `mz` est par action, et son coût est un nombre d'arènes borné par les prises en vol, pour que les familles suivantes projettent sur un modèle juste.
24. En tant que mainteneur de 4h, je veux une ligne « *Reçu de l'étape 4f* » qui dise que `ox` et `ij` d'un `mz` désignent des sondes `g_`, et que `mz.p`/`mz.ra` restent les boîtes de `bv`/`ok.qa`, pour que 4h ne range pas la sonde dans une place recyclée sans revoir ces pointeurs.
25. En tant que mainteneur, je veux que les contrats devenus faux du module trou de ver (« nothing frees the struct », `TROU-7` sur `mz:pq`, « never frees any `mz` ») soient réécrits, et que `<stdlib.h>` et l'inclusion de la transitoire en sortent, pour que les commentaires restent vrais (règle du `CLAUDE.md`).
26. En tant que mainteneur, je veux que le contrat de `PICO8_JO_PRISE_OCTETS` nomme aussi le trou de ver, pour qu'un relèvement futur sache qui la partage.
27. En tant qu'agent sans humain, je veux que toute hypothèse sur l'issue de 4e soit numérotée et revérifiée avant l'implémentation, pour ne pas bâtir sur une conception que 4e aurait changée.

## Implementation Decisions

### Inventaire des sites de 4f, et zone retenue

Chaque site est tranché par lecture : « à quelle fréquence, et qui d'autre garde l'objet ? ».

| Site | Ligne à 7fa10bb | Fréquence (lecture) | Qui garde l'objet | Zone retenue |
|---|---|---|---|---|
| structure `mz` | `wormhole` 204 | **une par pression sur le vide** quand `cg.g_` n'est pas vide (`bg:ldown`) ; meurt au `dg` qui suit son relâcher (refusé) ou 26 ticks plus tard (ouvert) | `ok.lheld` tant qu'il est tenu ; registres ; `fo` d'un `jc` qu'il survole ; `g_bv_et.fo` (écrit, jamais lu) | **arène de prise**, recyclée à sa mort |
| conteneur `ox` | `wormhole` 39 | une par `mz` | le `mz` seul | **dans l'arène** |
| éléments de `ox` | `wormhole` 40 | une fois par `mz`, jamais réécrits | le `mz` seul | **dans l'arène**, capacité réservée exacte |

- **Pourquoi pas la génération.** C'est une allocation par action. Mis tel quel en génération, chaque pression ratée sur le vide la ferait grandir d'au moins 280 o.
- **Pourquoi pas une sorte nouvelle de la zone particules ou interface.** Les particules sont à 130 822 o sur 131 072 après 4e (H-4e-5), l'interface à 335 Ko sans marge. Une arène de prise existe déjà, recyclée, prise à la génération et remise à zéro avec elle. Elle ne demande ni sorte, ni taille, ni contrôle de somme nouveaux.
- **Pourquoi pas la zone frame pour `ox`.** `ox` est relue à chaque frame tenue, à travers plusieurs remises de la zone frame.
- **Pourquoi pas une valeur pour `ox`.** Sa taille dépend de la partie (n de 1 à ~120).

### Construction dans l'arène de prise

- `pico8_mz_struct_new` appelle `pico8_jo_prise_ouvrir()`, alloue la structure par `pico8_jo_zone_construction()`, initialise comme aujourd'hui, appelle `mz:pq` (qui alloue `ox` par la même zone), puis `pico8_jo_prise_fermer()`, **avant** `pico8_ke_struct_pq` : les registres ont leur propre poignée.
- **Aucune prise ouverte ne peut déjà l'être.** `bg:ldown` est une pression sur le vide ; la prise d'icône n'est ouverte qu'autour du `new_entity` d'`ie:ldown`, et celle du `jc` qu'autour de `pico8_jc_struct_new` (H-4e-2). L'`assert` « une prise d'icone est deja ouverte » le garde.
- Un `mz` fait par un TU (`test_pico8_mz`, `test_pico8_jc`, `test_pico8_struct_header`) prend aussi une arène. Le prototype garde ces TU verts sans changement.
- **Taille** : le lot est de 344 + 8 × n o ; aucun relèvement de `PICO8_JO_PRISE_OCTETS`. Le dépasser (n > 213 à 2048 o) sort par `pico8_fatal` « zone prise pleine », qui se lit comme une galaxie à plus de 213 ancrages possibles et se relève d'un chiffre.
- **`ox` réservée** : `mz:pq` fait deux passes sur `hv` avec le même prédicat (`fx` non nul) : la première compte, la seconde remplit une liste réservée à ce compte. Le conteneur `ox` est alloué dans l'arène (il reste un pointeur de la structure : aucune forme de `pico8_mz_t` ne change). Avec n = 0, la liste reste vide sans éléments alloués, comme aujourd'hui avant le premier ajout.
- **La fonction de réservation** de `pico8_ents` (nom laissé à l'implémenteur, sur le modèle de `pico8_ptrs_init_reserve`) : prend la zone et une capacité ≥ 0, alloue les éléments d'un coup ; ajouter au-delà fait grandir dans la zone, comme `ents_add`. Contrat à écrire. Si 4e en a déjà fait une, 4f la réutilise (H-4e-9).

### Le `rendre` du trou de ver

- `mz` gagne `rendre` dans sa classe. Il entre donc dans la file de sortie quand `he` (mort) ou le `jl()` d'`eu`/`_init` (abandon) le retirent de `hv`. Au vidage, il passe d'abord par `oublier`, puis par son `rendre`.
- Le `rendre` ne fait qu'une chose : **remettre l'arène de sa liste `ox` (`ox->zone`) dans les prises libres, sans condition d'état.** Il n'a ni liste `qy`, ni rangée, ni traînée à rendre.
- **Pourquoi sans condition.** Le test « encore kz » de 4d existe parce qu'une structure **posée** prête sa mémoire après sa mort : un éclat `ga` né par `jo:hu` emprunte son `p`, son `ra` et son `id`. Rien n'emprunte la mémoire d'un `mz` :
  - `ij` et les éléments de `ox` désignent d'autres entités, pas lui ;
  - `et` désigne un `jc`, la cible jetable statique ou `mz.jj` (statique) ;
  - `mz.p` est une boîte de `bv` (hors arène, 4h), `mz.ra` est `ok.qa` ;
  - aucun objet ne garde un pointeur dans un `mz` (inventaire ci-dessous).

  Un `mz` ouvert (« pr ») passerait le test « kz » en échec, et son arène resterait prise jusqu'à la fin de la génération : +1 arène par trou de ver ouvert, soit une croissance par action.
- **La fonction de remise.** 4e sépare le retour d'arène en une fonction publique du module planète « zone, état » (H-4e-2). 4f en extrait le cœur sans condition d'état : reconnaître l'arène (nom « prise ») et la remettre dans la liste. La fonction « zone, état » de 4e l'appelle après son test « kz ». Le `rendre` du `mz` appelle le cœur directement. Aucun appelant ne passe un état qu'il n'a pas. La forme exacte (deux fonctions, ou le test d'état déplacé chez les appelants) est laissée à l'implémenteur.
- **Oubli** : rien à ajouter. `pico8_jo_oublier_prises()` vide déjà la liste partagée au point d'oubli de partie, au chargement et à l'arrêt.
- **Borne de recyclage.** Un `mz` refusé à la frame N (dans `ok:lj`, `ey`) est retiré par le `dg` de N+1 et rendu au vidage de N+2. Un `mz` ouvert est retiré au `dg` qui suit `t > 25`, rendu au vidage suivant : le prototype le voit rendu **28 frames après son relâcher** (relâché f160, rendu f188).
- **Angle mort ASan**, comme en 4d et 4e : la pression suivante réalloue aussitôt les mêmes adresses. Un pointeur oublié vers un `mz` rendu lirait le nouveau sans planter. C'est l'inventaire, pas ASan, qui garantit qu'il n'y en a pas.

### Liste des prises libres : capacité 4 → 8

- **La lecture.** Les arènes recyclables en vol à un instant sont celles des objets non posés et pas encore rendus :
  - un objet tenu (`ok.lheld` : icône, `jc` ou `mz`), au plus un ;
  - un objet refusé qui attend son vidage (deux frames), au plus un de plus, puisqu'une pression tombe au plus tôt la frame suivante ;
  - des `mz` ouverts : chacun tient son arène 28 frames après son relâcher, et un `mz` ne s'ouvre que tenu 15 ticks. Deux au plus sont donc ouverts à la fois.

  Cela fait **4** au pire, quand tous meurent avant toute nouvelle prise : exactement la capacité de 4d. Les e2e montrent 2 au plus (prototype : `empty_planet_fade`, `trade_rich_mine`).
- **Décision** : `PRISES_LIBRES_MAX` passe à **8**, avec un message d'`assert` inchangé. Coût : 32 o de tableau statique, hors zones. Ce n'est pas une zone : rien à compter dans la somme des 1,5 Mo.

### La coupe de `fo`, vérifiée pour le trou de ver

`mz:hq` appelle `bv`, qui accroche `fo` sur sa cible `et` (`pico8_probe_et_hang_fo`).

| Champ qui peut tenir un `mz` | Qui l'écrit | Couvert par |
|---|---|---|
| `fo` d'une planète de `cg.np` | — | **ne peut pas** : la description du `mz` a `dw = 0`, donc `bv` ne prend jamais une planète pour `et` (`nq < 0` est faux) |
| `fo` d'un `jc` | `bv` quand `et` est un slipway survolé | `jc:kj` le remet à nil **sans condition**, en fin de dessin (`slipway_display` 171 à 7fa10bb), dans la frame même ; un `mz` refusé n'est recyclé que deux updates plus tard, un `mz` ouvert n'est plus tenu donc plus `hq` (H-4e-3) |
| `g_bv_et.fo` (`probe`) | `bv` quand `et` est la cible jetable | **écrit seulement, jamais lu** (4a) |
| `mz.jj` (`g_mz_jj_wrapper`) | — | `mz:hq` y fait pointer `et` **après** `bv`, et `pico8_probe_et_hang_fo` n'écrit `fo` que sur une planète, un `jc` ou la cible jetable : jamais sur `jj` |
| `ok.lheld` | `ui:bh` | remis à nil par `ui:bh` après `ey` |

Une planète hors écran ne peut donc pas garder un `fo` vers un `mz` : le cas TROU-26 de 4d n'a pas d'analogue ici. Le `rendre` fait tout de même passer le `mz` par `oublier`, qui couvrirait une cible planète si `dw` changeait.

### Relevé « éclats `ga` rendus en cours de partie », refait pour 4f

Lu à 7fa10bb. Un `mz` vit au plus ~45 frames ; peut-il désigner un éclat dont la place est rendue pendant ce temps ?

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

**Relevé fait à 7fa10bb** (20 pointeurs `static` dans `CORE_SRCS`, 37 lignes de structures `static`) :

| Pointeur | Peut viser | Statut |
|---|---|---|
| `g_jo_prise` (`planet`) | l'arène d'un `mz` en construction | non nul seulement entre `ouvrir` et `fermer`, que `pico8_mz_struct_new` appelle tous deux |
| `g_jo_prises_libres[]` (`planet`) | arènes de `mz` rendus | n'y entrent que des arènes déjà rendues ; vidée au point d'oubli de partie, au chargement, à l'arrêt. **Fenêtre nouvelle** : après l'`_init` qui abandonne un `mz`, la liste tient une arène de l'ancienne génération jusqu'au vidage qui suit l'`eu`. Aucune prise ne s'ouvre dans cette fenêtre : l'écran titre n'a pas de `bg`, et le clic qui lance `eu` est pris par le bouton du menu. Le cas « refusé, restart, nouvelle partie, pression » le teste |
| `g_ok` → `ok.lheld` | le `mz` tenu | remis à nil par `ui:bh` après `ey` ; c'est le nouvel `ok` après la bascule |
| `g_ok` → `ok.o_`, `ok.gj` | — | `mz` n'a pas `hd` : jamais dans `cv.hd`, jamais survolé |
| `g_hv`, `g_cv[].list`, `g_cg[].list` (`entity`) | des `mz` | retirés par `he` avant l'entrée en file ; refaits au `jl()` d'`eu` |
| `g_file[]` (`file_sortie`) | un `mz` mort ou abandonné | vidé avant le rendu de la génération, et avant tout recyclage (le recyclage est fait **par** le `rendre`) |
| `g_bv_et.fo` (`probe`) | un `mz` | écrit seulement, jamais lu (4a) |
| `fo` des `jc` | un `mz` | voir « La coupe de `fo` » |
| `g_mz_jj_wrapper` (`wormhole`) | — | statique ; son en-tête n'a aucun champ écrit (voir « La coupe de `fo` ») |
| `g_jc_too_far.fo` (`slipway`) | — | n'est écrit que par un `jc` |
| `g_mo`, `g_je`, `g_kc`, `g_ju`, `g_dk`, `g_fm`, `g_gb[]`, `g_ie_gy`, `g_jo_oc`/`g_jo_ir`, `g_arenes[][]`, `partie_choisie`, zones et poignées, dispatch | — | ne peuvent pas viser un `mz` ni son arène, comme à 4a, 4c, 4d et 4e |

**Objets d'autres zones qui désignent un `mz`** : aucun. Un `mz` n'a ni rangée (zone interface), ni liste `qy` (zone particules), ni étiquette autre que `mz` (`cg.mz` n'est lu que par les TU). `ox` d'un autre `mz` ne le contient pas : `mz` n'a pas `fx`.

**Conclusion** : ni le point d'oubli de partie ni `oublier` ne gagnent d'entrée. Un pointeur trouvé ensuite par ASan sera ajouté au point d'oubli et noté dans « Découvert » comme un **trou de l'inventaire** (§4). L'inventaire est refait sur le code du commit au morceau de clôture ; s'il diffère de ce relevé (par exemple parce que 4e a ajouté un pointeur), la différence est classée avec la même question.

### Verrou

- `tools/alloc_guard_exceptions.txt` : retirer `src/pico8_wormhole.c 3`.
- **Prévision** : mesurée par `make alloc-guard` sur le prototype à 7fa10bb, « verrou objets 65 allouent 15 appels 44 » (16 / 47 → 15 / 44). Après 4e (H-4e-4) : **13 / 36 → 12 / 33**. L'arène ne change aucun compte : elle est prise à la génération.

### Budget : mesure, projection, décision

- **Mesure par prototype** à 7fa10bb, sur une copie `git archive` hors dépôt. Le prototype met en œuvre la conception de cette PRD **sur l'arène de 4d (1536 o)**, puisque 4e n'est pas finie : construction dans la prise, `ox` réservée exactement, `rendre` sans condition d'état, fonction de réservation de `pico8_ents`. Les 26 e2e ont été rejoués avec `PICO8_REMPLISSAGE`, contre la même mesure à 7fa10bb :
  - les 26 `*.sha256` sont identiques ; `sanitize-net` répond « 26 propre(s) » ; `make test` est vert une fois réécrit le cas `une_prise_posee_garde_son_arene` (voir Testing Decisions) ;
  - **`max_octets partie` sur les 26 e2e : 130 720 o** (`pause_restart`, sans `mz`, inchangé) ;
  - vingt `mz` construits sur les 26 e2e (un par e2e au plus, deux sur `slipway_build` et `trade_rich_mine`) ; `ox` compte **3 éléments au plus** ;
  - génération finale : **+1 536 o** (une arène) sur les onze e2e qui ont un `mz` et aucune icône posée après sa mort ; **+3 072 o** sur `slipway_build` (le `mz` ouvert à f20 vit encore quand celui de f39 naît) ; **+0** sur `slipgate_outgoing_first`, `synthesizer_second_link`, `tech_infraspace_infragate`, `tech_slipgates_relay`, `tech_trade_league_hub` et `trade_rich_mine`, où une icône posée reprend l'arène du `mz` mort ;
  - prises libres : **2 au plus** (`empty_planet_fade`, `trade_rich_mine`) ;
  - fixture synthétique de pliages (f0-3 de `slipway_build`, puis vingt cycles « pression en (80,55), quatre frames tenues, relâché, trois frames », puis un `mz` tenu seize frames et ouvert, trente-deux frames d'attente, une pression) : génération **66 672 o** après un refus, après vingt refus, et après l'ouvert et la pression qui suit ; le `mz` ouvert est rendu à f188, 28 frames après son relâcher ;
  - fixture d'abandon (f0-20 de `slipway_build`, puis f2-17 de `pause_restart`) : le `mz` ouvert à f20 est abandonné par l'`_init` de f26 et rendu au vidage de f27 en état « pr » ; le rejeu est propre sous ASan.
- **Prévision après 4e** (arène 2048, H-4e-1) : les +1 536 deviennent +2 048 ; là où un `jc` refusé ou une icône posée reprend l'arène du `mz`, rien. `trade_rich_mine` reste à la valeur de 4e (95 312 o, H-4e-5) : ses deux `mz` (rendus vers f70 et f95) laissent deux arènes que les labos posés de f100-111 reprennent, comme ils en auraient pris deux neuves. **`M_4f` = `M_4e` = 130 720 o** ; `K_4f` = 0.
- **Projection.**
  - Estimations restantes (4e, recopiées de sa PRD) : génération 4g ~6, 4h ~12, soit **18 Ko** ; chargement 4g ~32 Ko.
  - `r` = **1** (H-4e-5 ; la mesure de 4f ne dépasse pas son estimation au pic, puisque le pic ne bouge pas).
  - `P_4f` ≈ 127,7 + 18 ≈ **146 Ko** ≤ 704 ; `Q_4f` ≈ 32 Ko ≤ 64.
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

1. **Réservation de `pico8_ents`.** (Sauté si 4e l'a faite, H-4e-9.)
   - Les tests d'abord, dans le TU de `pico8_ents` : la liste réservée a la capacité demandée ; n ajouts ne font pas bouger les octets pris de sa zone ; un ajout de plus grandit ; une capacité 0 ne prend rien.
   - La fonction et son contrat. Aucun compte du verrou ne bouge.
2. **Trou de ver dans l'arène de prise, `rendre`, prises libres à 8.**
   - Les tests d'abord :
     - `test_pico8_mz` : un `mz` neuf a `ox->count == ox->capacity` ; la structure et `ox` sont dans une même arène de prise (la génération prend exactement une arène, ou rien si une prise libre existe) ; l'ordre de `ox` suit `hv` ;
     - la fixture de pliages : génération constante pendant les frames tenues, égale du premier au vingtième refus, et inchangée par l'ouvert et la pression qui suit (rouge si le `mz` entre en génération, ou si l'ouvert n'est pas recyclé) ;
     - la fixture d'abandon : le `mz` se relit à la frame du clic « challenging », et l'enfant meurt par ASan quand il est relu après la frame suivante (rouge aujourd'hui : le `mz` de la libc n'est jamais libéré) ;
     - refus, « restart », nouvelle partie, pression : l'enfant ne meurt pas ;
     - les cas de prise de 4d/4e qui comptent « une arène de plus » réécrits sur leur fixture (voir Testing Decisions).
   - Le cœur de remise sans condition d'état dans le module planète, appelé par la fonction « zone, état » de 4e et par le `rendre` du `mz`.
   - `pico8_mz_struct_new` entre `pico8_jo_prise_ouvrir`/`_fermer` ; `mz:pq` en deux passes, `ox` réservée ; `rendre` dans la classe ; `PRISES_LIBRES_MAX` à 8 ; contrats réécrits (`PICO8_JO_PRISE_OCTETS`, module trou de ver).
   - Le verrou : `wormhole` sort, **13 / 36 → 12 / 33** (H-4e-4 ; 16 / 47 → 15 / 44 si 4e ne l'avait pas changé).
3. **Mesure et clôture.**
   - Le relevé sur les 26 e2e, `M_4f`, `K_4f`, le maximum de `ox->count` et des prises libres, la comparaison au prototype, la projection et la décision, dans le message du commit.
   - L'inventaire refait par `grep`.
   - La note « Découvert (4f) », la correction du §2, la ligne « *Reçu de l'étape 4f* » pour 4h, le retrait de cette PRD.

Le morceau 2 dépend du 1. Le 3 dépend des deux. **Aucun ne commence avant la clôture de 4e** et la revérification de la section « À revérifier une fois 4e finie ».

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris de la zone partie (`partie_sans_registres`, les lignes de `PICO8_REMPLISSAGE`) ;
  - les champs publics d'un `mz` (`ox`, `ox->count`, `ox->capacity`, `ij`, `header.me`, `p_`) ;
  - un rapport ASan dans un processus enfant ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne lit ni les bouts de la zone ni une adresse, sauf pour la relire après sa mort. Les tailles attendues se calculent dans le TU (`sizeof`, `PICO8_JO_PRISE_OCTETS`), jamais en dur : 344 et 2048 ne valent que sur x86-64.
- **`tests/test_pico8_places_vivantes.c`** (ASan, un enfant par cas, `demarrer`/`jouer_jusqu_a`, `partie_sans_registres`, `dans_un_enfant_qui_meurt_par_asan`).
  - **Pliages refusés et ouverts** (cas nouveaux), sur une fixture nouvelle `tests/fixtures/pliages.json`, construite comme celle du prototype : f0-3 de `slipway_build` (sonde `g_` à f2) ; vingt cycles « pression en (80,55), quatre frames tenues, relâché, trois frames » ; un `mz` tenu seize frames en (80,55), relâché et ouvert ; trente-deux frames d'attente ; une pression ; « restart » (f2-17 de `pause_restart`), puis une pression sur le vide. Ce n'est pas un e2e : elle n'ajoute pas de ligne couverte. Les cas :
    - génération constante pendant les frames tenues d'un cycle ;
    - égale après le premier et après le vingtième refus ;
    - égale avant l'ouvert et après la pression qui suit sa mort ;
    - la traversée « restart », nouvelle partie, pression ne fait pas mourir l'enfant.
  - **Trou de ver abandonné** (cas nouveau), sur une fixture `tests/fixtures/pliage_abandonne.json` (f0-20 de `slipway_build`, puis f2-17 de `pause_restart`, 37 frames) : on prend le `mz` de `cg.mz` à f22 ; il se relit à f31 (clic « challenging », bascule) ; l'enfant meurt par ASan quand il est relu à f32 ou après. Sur le modèle de `pause_restart_garde_les_planetes_de_la_partie_quittee_pendant_le_clic` et `pause_restart_rend_les_planetes_de_la_partie_quittee`.
  - **`une_prise_posee_garde_son_arene`** (4d) **est réécrit.** Sa fixture `prises_refusees.json` commence par f0-20 d'`empty_planet_fade`, dont le `mz` (f4) est rendu à f48 : à f161, deux arènes sont libres, et la structure posée en reprend une. Le prototype le voit rouge sur l'attente « `PICO8_JO_PRISE_OCTETS` + listes », vert sur « listes » seules. Le nouveau test dit pourquoi dans son nom ou son message, pas dans un commentaire de récit.
  - **Les cas de 4e qui comptent « exactement une arène de plus »** sont relus (À revérifier) : sur la fixture de refus de 4e (f0-34 de `slipway_build`), le `mz` de f4, ouvert à f20, meurt pendant les cycles de refus et laisse une arène libre ; « un slipway posé après les refus, puis un refus, prennent exactement une arène de plus » devient « aucune arène de plus ». La fenêtre f139-f151 de `trade_rich_mine` ne devrait pas bouger (les arènes des `mz` sont reprises par les labos de f100-111 avant f139).
  - **Inchangés** : `message_box_hold` f45 = f90 ; `game_end_final_score` f2 = f51 ; les égalités exactes de 4c/4d sur `pause_restart` (aucun `mz`) ; `prises_refusees_ne_font_pas_grandir_la_partie` (l'arène du `mz` est prise à f4, avant f27) ; `le_fo_d_une_planete_hors_ecran_oublie_la_prise_refusee` (le `mz` vit encore à f28, la prise de f28 reprend bien l'arène de la refusée) ; `empty_planet_fade_ne_monte_pas` ; la sentinelle de 4a. Tous verts dans le prototype.
- **`tests/test_pico8_mz.c`** : réservation et arène (voir morceau 2). Les attentes existantes sur `ox` (entrées, sonde et planète éclairée listées, `mz` absent) restent vertes dans le prototype.
- **`tests/test_pico8_ents.c`** : la réservation (morceau 1), sur le modèle du cas de réservation de `pico8_ptrs`.
- **Verrou** : `make alloc-guard` à égalité stricte, avec la liste sans `wormhole`.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`.
- **Exemples dans le dépôt** :
  - `tests/test_pico8_places_vivantes.c` : rejeu par enfant, les cas de prise de 4d (`prises_refusees_*`, `une_prise_posee_garde_son_arene`, `une_prise_refusee_traverse_une_nouvelle_partie`), les cas « planètes de la partie quittée » de 4d et la sentinelle de 4a ;
  - `tests/fixtures/prises_refusees.json` : la forme d'une fixture de prises ;
  - `tests/test_pico8_ptrs.c` : le cas de `pico8_ptrs_init_reserve`.

## Out of Scope

- La boîte `p` de `bv` faite à chaque frame tenue (`probe` 162), `mz.ra` = `ok.qa`, la sonde `g_` et `fv` : 4h.
- L'arène de prise elle-même, sa taille, `pico8_zone_arene_vider`, la coupe de `fo` (TROU-26), le `rendre` du `jc` : 4d et 4e, versés. 4f n'y touche que pour extraire le cœur de remise sans condition d'état et porter la liste libre à 8.
- `mz.jj` (`g_mz_jj_wrapper`) : statique, sans écrivain, ne change pas.
- L'audio (`rn(53)` de `mz:pq`) : hors périmètre du portage.
- Relever `PICO8_ZONE_GAME_BYTES` ou `PICO8_JO_PRISE_OCTETS`, sauf si la mesure l'exige.
- Retirer TROU-7 : étape 5.
- Lancer l'application raylib.

## Hypothèses sur l'étape 4e

Chacune est ce que cette PRD suppose de 4e, lue dans sa PRD (`docs/prd/triage/phase1-etape4e-slipways-et-routes.md` à 7fa10bb), et ce qui change ici si elle tombe.

- **H-4e-1 (taille de l'arène).** `PICO8_JO_PRISE_OCTETS` vaut **2048**, et son contrat dit « arène où une icône tenue ou un slipway tiré construit son objet ». *Si elle tombe* (arène restée à 1536) : le lot du `mz` tient jusqu'à n = 149, encore très au-dessus des e2e ; les prévisions « +2 048 » redeviennent « +1 536 », mesurées par le prototype.
- **H-4e-2 (retour d'arène et liste partagée).** Une seule liste de prises libres, partagée par icônes et slipways, de capacité 4 ; le retour d'arène est une fonction publique du module planète « zone, état », qui teste le nom « prise » et l'état « kz », appelée par le `rendre` des planètes et du `jc` ; `pico8_jc_struct_new` ouvre et ferme la prise ; `pico8_jo_oublier_prises()` reste appelée au point d'oubli de partie, au chargement et à l'arrêt. *Si elle tombe* (seconde liste propre au slipway, ou retour non séparé) : 4f se branche sur la liste des icônes de 4d et sépare elle-même le cœur sans état de `pico8_jo_prise_rendue`.
- **H-4e-3 (`jc` et `fo`).** Le `jc` a un `rendre` et passe par `oublier` ; `pico8_jo_oublier_fo` n'a pas changé ; `jc:kj` remet toujours `fo` à nil sans condition en fin de dessin. *Si elle tombe* (`jc.fo` remis à nil sous condition) : un `mz` rendu pourrait rester dans le `fo` d'un slipway hors écran ; `oublier` doit alors couper aussi les `fo` de `cg.jc` qui visent l'entité rendue.
- **H-4e-4 (verrou).** Après 4e : **13 fichiers, 36 appels** ; `pico8_wormhole.c` n'est pas touché par 4e, ses 3 relocations restent aux lignes 39, 40 et 204. *Si elle tombe* : 4f repart de la mesure à son `HEAD` et retire 3 appels.
- **H-4e-5 (budget).** `M_4e` = 130 720 o (`pause_restart`), `K_4e` = 0, `r` = 1, `trade_rich_mine` à 95 312 o, `game_end_bankrupt` à 84 048 o, zone particules à 130 822 o sur 131 072 (sorte `ts` de 680 places), somme des zones 1487 Ko sur 1536, décision « on continue ». *Si elle tombe* : la projection de 4f se refait sur la mesure de 4e ; seul un `M_4e` au-dessus de ~686 Ko changerait la décision.
- **H-4e-6 (TU de 4e).** La fixture de refus de 4e commence par f0-34 de `slipway_build` ; elle attend « un slipway posé après les refus, puis un refus, prennent exactement une arène de plus », et la fenêtre f139-f151 de `trade_rich_mine` attend « exactement `PICO8_JO_PRISE_OCTETS` ». *Conséquence* : avec 4f, le `mz` de f4 de `slipway_build` laisse une arène libre pendant les refus, et la première attente devient « aucune arène de plus » ; la seconde ne change pas.
- **H-4e-7 (« Reçu »).** La clôture de 4e n'écrit de « *Reçu de l'étape 4e* » que pour 4h (le point souris garde `ok.qa`). *Si elle tombe* (un « Reçu » pour 4f) : cette PRD le traite ou dit pourquoi non, avant l'implémentation.
- **H-4e-8 (fréquences).** 4e confirme que tous les `jc` naissent d'une pression dans `jo:ldown`/`nr:ldown`, jamais pendant une prise ouverte. *Si elle tombe* : l'`assert` de prise déjà ouverte pourrait tomber entre `mz` et `jc` ; on le verrait au premier rejeu.
- **H-4e-9 (réservation).** 4e réserve `ts` par `pico8_ptrs_init_reserve` et n'ajoute rien à `pico8_ents`. *Si elle tombe* (4e a ajouté une réservation à `pico8_ents`) : le morceau 1 est sauté.
- **H-4e-10 (outillage de test).** `demarrer`, `jouer_jusqu_a`, `partie_sans_registres`, `dans_un_enfant_qui_meurt_par_asan` et la forme des fixtures de `tests/fixtures/` ne changent pas.

## À revérifier une fois 4e finie

Relire à `HEAD` après la clôture de 4e, sans toucher git ni les sources, et réécrire les sections qui en dépendent :

- la note « *Découvert (4e)* », toute ligne « *Reçu de l'étape 4e* » et la ligne « Partie » du §2 dans `docs/plan-4.md` (H-4e-5, H-4e-7) ;
- `git log --stat` des commits de 4e : aucun ne doit toucher `pico8_wormhole.{h,c}` (H-4e-4) ;
- `pico8_planet.{h,c}` : `PICO8_JO_PRISE_OCTETS`, la fonction de retour « zone, état », la capacité et l'unicité de la liste des prises libres, les appels de `pico8_jo_oublier_prises()` (H-4e-1, H-4e-2) ;
- `pico8_slipway_display.c` : `jc:kj` remet `fo` à nil sans condition ; `pico8_cart.c` : `oublier` et `oublier_partie` (H-4e-3) ;
- `pico8_ents.h` : une réservation existe-t-elle déjà (H-4e-9) ;
- `make alloc-guard` sur une copie `git archive HEAD` (H-4e-4) ;
- les cas de 4e dans `tests/test_pico8_places_vivantes.c` et sa fixture de refus : les attentes « une arène de plus » (H-4e-6, H-4e-10) ;
- **refaire le prototype** sur ce `HEAD`, à l'arène de 4e : `sha256`, `sanitize-net`, `make test` (pour lister exactement les cas de 4d/4e à réécrire), `alloc-guard`, `max_octets partie` sur les 26 e2e et sur `trade_rich_mine`, prises libres au plus, et les deux fixtures de 4f ;
- refaire l'inventaire du §4 par `grep` : 4e a pu ajouter des pointeurs `static` (fonction de retour, sorte `ts`).

## Further Notes

- **Pas d'humain disponible, et pas d'outil `Agent` dans cette session pour lancer un agent `guideline` neuf.** J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage).
  - **Hypothèses explicites** : celles sur 4e sont numérotées (H-4e-1 à H-4e-10) et listées à revérifier. Les fréquences ont été lues dans le cart : `bg:ldown` l. 376-377, `mz` l. 752-797 (`pq` 764, `hq` 767, `ey` 774, `pr` 776-781), `bv` l. 552-558, `d_` l. 569-575, `dg`/`he` l. 163-182, `jp` l. 31-36, `jo:hu` l. 451-455. Il en sort une correction de la table de 4a : le `mz` est par action.
  - **Simplicité** : aucun mécanisme neuf. L'arène de prise de 4d/4e, un `rendre` d'une ligne, une réservation sœur de celle de `pico8_ptrs`, une constante relevée. Écartés : une sorte de particules (zone pleine après 4e), une arène propre aux trous de ver (même code en double), `ox` en zone frame (relue sur plusieurs frames), un faux état « kz » passé au retour d'arène.
  - **Changements chirurgicaux** : 3 relocations dans un fichier, plus le cœur de remise du module planète, la capacité de la liste libre et une fonction de `pico8_ents`. La forme de `pico8_mz_t` ne change pas.
  - **Critères vérifiables** : chaque morceau a ses tests rouges d'abord, et le prototype à 7fa10bb a montré les attentes (sha256, ASan, verrou, génération constante sur vingt refus, abandon propre).
- Le prototype à 7fa10bb (copie `git archive` hors dépôt) a aussi instrumenté les créations et rendus de `mz` (frames, état, `ox->count`) et la longueur de la liste libre ; ces mesures sont reprises dans « Budget ». Rien de ce prototype n'est dans le dépôt.
