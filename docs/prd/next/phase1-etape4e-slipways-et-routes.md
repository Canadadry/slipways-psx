---
title: "Plan 4, étape 4e — Slipways en arène de prise recyclée, routes gardées recopiées en génération, traînées ts en zone particules"
description: "Le slipway jc, sa liste ts, son point souris, ses traînées et les routes trade sont encore pris à la libc ou à la transitoire. 4e met chaque slipway et sa liste dans une arène prise à la génération et recyclée quand il est refusé, réécrit le point souris en place, fait toutes les routes candidates de bd en zone frame et ne recopie en génération que celles que by garde, et range les traînées ts dans une nouvelle sorte de la zone particules, rendue à leur mort et au rendu du slipway."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. L'étape 4 range les objets de partie dans la **zone partie**. Cette PRD traite la sous-étape **4e, slipways et routes**.

**Elle est rédigée pendant 4d.** Lu à `HEAD` **24057b4** : le morceau 1 de 4d est versé (`cc` vidée en place, `ib` en génération, `lp`/`rl` des propositions en zone frame ; verrou 21 fichiers / 66 appels) ; les morceaux 2 (structures de planète en génération), 3 (arène de prise) et 4 (clôture) ne le sont pas. Tout ce que 4e suppose de la fin de 4d est écrit en hypothèses H-4d-1 à H-4d-8 (« Hypothèses sur l'étape 4d ») et repris dans « À revérifier une fois 4d finie ». `HEAD` a bougé pendant la rédaction (6fe9958 → 24057b4) : tout ce qui suit est lu et prototypé à 24057b4.

**Le texte de l'étape**, recopié du plan à 24057b4 (extraits qui concernent 4e) :

> - **4b et suivantes.** Une famille de propriétaires par PRD dans l'ordre fixé par la PRD 4a : 4b états de `pico8_gen.c` ; 4c tableaux croissants et registres (crée la sous-zone chargement) ; 4d planètes et relais ; 4e slipways et routes ; 4f trous de ver ; 4g recherche et économie ; 4h singletons de `jl`/`eu` et objets du joueur (famille ajoutée par 4a : les « Reçu » des étapes 1 à 3 leur reviennent). N'entre en zone partie qu'une allocation faite une fois par objet de partie ; un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
> - *Découvert (4a) :* […] Les 81 relocations de l'étape 4 sont attribuées aux familles 4b–4h ; la seule marge est de 49 Ko sur la somme des zones, la zone particules gardant ses ~17 Ko libres pour les traînées `ts`. Le rejeu écrit `partie` et `max_octets partie`.
> - *Reçu de l'étape 4a :* les traînées `ts` de `jc:ch` (par frame) vont à 4e, en zone particules, places de 24 o.
> - *Découvert (4c) :* […] Mesure : génération 34 240 o au plus (clic de `pause_restart`), chargement 0 ; projection P ≈ 33 + 209 × 1,52 ≈ 351 Ko ≤ 704, Q ≈ 49 Ko ≤ 64 : on continue. Comptes : 23 / 80 → 22 / 75.
> - *Reçu de l'étape 1 :* même chose pour les boîtes `vec` des objets de partie (`p`/`v`/`ra` de `qb`, `ra` de `nf`, `mo->v`, `re`/`p` de `jc:hq`, `p` de `pico8_bv`, `ie.gy`) ; `qb:ph` seul en fait trois par frame. […]
> - *Reçu de l'étape 2 :* restent à cette étape `ok.qa` (gardé par les hôtes de `bv` et le point souris de `jc:hq` : objet relu par des objets de partie), […]

**Les lignes du §2**, recopiées :

> | **Particules** | particules de `g_:mm`/`ga:mm`/aide, listes `qy` de leurs émetteurs (`g_`, `fv`, `hj`, `pm`, `mx`, `ga`), éclats `ga` entiers, traînées `ts` de `jc:ch` | nombre maximum de particules vivantes ; place rendue à leur mort |
> | **Partie** | planètes, relais, slipways, routes `trade`, recherche, trous de ver ; les tableaux croissants (`ents`, `ptrs`, registres) qui les contiennent ; les objets faits au chargement (`gb`, `hp.i`, listes statiques) dans une sous-zone chargement, prise dans la zone partie, remise à zéro par le seul chargement | une génération par partie, de `eu` à `eu` ; rendue d'un bloc au vidage de la file qui suit le `eu` suivant (zone à deux bouts) ; `_init` ne la rend pas |

**La ligne de 4e dans la table des familles de la PRD 4a**, retirée par b46601e (`git show b46601e~1:docs/prd/triage/phase1-etape4a-socle-zone-partie.md`), recopiée :

> | Ordre | Famille | Sites (fonction, ligne) | Relocations | Reçus traités |
> |---|---|---|---|---|
> | **4e** | **Slipways et routes** | `pico8_jc_struct_new` (344), `pico8_jc_pq` (89, 90 : `ts`), `pico8_jc_hq` (111, 112) ; `pico8_jc_ch` (`slipway_display` 36 : traînée, vers la zone particules) ; `pico8_trade_struct_new` (29), `pico8_trade_kg` (135-138) ; refait le relevé « éclats `ga` » pour `jc:hq`/`re`/`et` | `slipway` 5 → sort ; `slipway_display` 1 → sort ; `trade` 5 → sort (11) | *Reçu 1* : `re`/`p` de `jc:hq` ; *Reçu 4a* : traînées `ts` |

La raison de l'ordre, recopiée de la même PRD :

> - **4e** dépend de 4d en lecture seulement (un `jc` désigne ses planètes). Elle apporte la seule sorte de particules nouvelle, à ranger dans les ~17 Ko libres de la zone particules (`ts` : 24 o par place, de l'ordre de quelques centaines de traînées vivantes, donc quelques Ko).
> - **4f** est petite, avec les mêmes motifs que 4e.

**Les lignes de 4e dans la table « Sites qui allouent par tick ou par action » de la PRD 4a**, recopiées avec la règle qui les précède :

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
> | `pico8_jc_hq` (point souris et sa boîte `p`) | par frame où un slipway est tenu | 4e |
> | `pico8_jc_ch` (traînée `ts`, `pico8_ts_t` de 20 o, place de 24) | par frame, tirage `rnd` 0,08 par sens | 4e, en zone particules |
> | `pico8_trade_kg` (`lp`, `rl` concaténées) | à chaque `kg` | 4e |

**L'estimation de 4a pour 4e** : « ≤ 10 slipways × ~1,1 Ko + routes ≈ 15 Ko » par génération, pic **~30 Ko**.

**La ligne « *Reçu de l'étape 4d* » pour 4e**, telle que la PRD 4d (`docs/prd/triage/phase1-etape4d-planetes-et-relais.md` à 24057b4) prévoit de l'écrire sous l'entrée 4 à sa clôture (elle n'est pas encore dans le plan, H-4d-1) :

> - **pour 4e** :
>   - la proposition `lw.t` de `jo:ib` est faite à chaque `bd`, pour chaque slipway de chaque planète ;
>   - ses `lp`/`rl` sont en zone frame, et sa structure `trade` (`pico8_trade_new`) ne doit pas entrer en génération (zone frame par l'appelant, ou valeur) ;
>   - seules les routes gardées dans `by`/`cc` sont des objets de partie ;
>   - les TU de `trade_rich_mine` et `game_end_final_score` tiennent cette règle pour 4e ;

**La méthode de suivi du budget** est celle de la PRD 4a, reprise par 4c et 4d (deux comptes depuis 4c : génération ≤ 704 Ko, chargement ≤ 64 Ko ; `P_k = M_k + Σ estimations restantes × r`, `r = max(1, mesuré / estimé)` ; marge de 49 Ko sur la somme des zones, d'où se prend aussi un relèvement de la zone particules pour `ts`).

**Ce qui se passe aujourd'hui** (lu à 24057b4) :

- **Le slipway.** `jc` naît d'une **pression** du bouton gauche sur une planète « ly » (`jo:ldown`, cart l. 412, `pico8_jo_ldown`) ou sur un relais (`nr:ldown`, l. 1112). Chaque pression en fait un, par `malloc` (`pico8_jc_struct_new`, 344), avec sa liste `ts` (`malloc` + transitoire, `jc:pq`, 89-90). `pico8_jc_t` fait **960 o** sur x86-64, dont 728 pour `hg_own` (90 identifiants au plus). `ui:bh` le tient (`ok.lheld`) ; au relâcher, `jc:ey` le **pose** (`d_` → `ks("ph")`, `qu:kf`, `re:kf`, `fm:nx`) ou le **refuse** (`re` nil, ou `d_` qui échoue sur `et` ou sur le coût) : `p_`, l'état reste « kz ». Un simple clic sur une planète colonisée fait donc un `jc` refusé. Le `jc` n'a pas de `rendre` : un refusé sort de `hv` par `dg`/`he` et fuit (TROU-7).
- **Le point souris.** Tant que le slipway est tenu au-dessus du vide, `jc:hq` (l. 622) fait **à chaque frame** un `{ra=mp,p=mp/128}` : une structure `pico8_jc_mouse_point_t` (144 o) et sa boîte `p` (`malloc`, 111-112), jamais rendues. `ra` est `mp`, c'est-à-dire `ok.qa` lui-même.
- **Les traînées.** Dans le `kj` d'un slipway visible, `jc:ch` (l. 657) tire `rnd()<0.08` pour chaque sens et, si `hg[t.id]`, ajoute un `pico8_ts_t` de 20 o (`malloc`, `slipway_display` 36). `kj` le fait avancer, décrémente `l`, le retire par `del(self.ts,t)` quand `l<=0`, sans le rendre.
- **Les routes.** `trade:kg` (l. 606) fait une route neuve : structure (`malloc`, `trade` 29), `lp` et `rl` concaténées (`malloc` + transitoire, 135-138). Ses appelants, **à chaque `bd`** :
  - `jo:ib` fait la proposition `lw.t` par `pico8_trade_new` (structure par `malloc` ; ses `lp`/`rl` sont déjà en zone frame depuis 24057b4) ;
  - `jo:cn` fait `t:kg(self)` et le passe à `lw:kf`, la fermeture de `jo:ib`, qui ne l'ajoute à `ib` (= `by`) que si `gu(ib,a)` est faux (égalité de route, `trade.ns.__eq`) ;
  - `nr:cn` fait `t:kg(self,w)` pour chaque slipway du relais et le passe, dans une copie `gf` de la proposition, au `cn` de l'autre bout.

  `by` n'est jamais vidée : une route connue est **reproposée et refusée par `gu` à chaque `bd`**. Le prototype compte, sur `trade_rich_mine`, **32 `kg` pour 7 routes gardées**.
- **Le verrou** à 24057b4 : **21 fichiers, 66 appels**. Les 11 relocations de 4e sont aux mêmes lignes qu'à e88df7a (4d n'a pas touché ces fichiers) : `slipway` 5 (89, 90, 111, 112, 344), `slipway_display` 1 (36), `trade` 5 (29, 135, 136, 137, 138).

**Le problème.** Trois fréquences se mêlent dans la famille, et une seule a sa place telle quelle en génération :

1. **Une fois par objet de partie** : le slipway posé (et sa liste `ts`), la route gardée dans `by`.
2. **Par frame** : le point souris d'un slipway tenu, les traînées.
3. **Par action ou par `bd`** :
   - **le slipway refusé** : chaque clic ou glissé raté sur une planète fait un `jc`. Mis tel quel en génération, il la fait grandir de **1 648 o par refus** (mesuré, 20 cycles) ;
   - **les routes candidates** : `lw.t`, chaque `kg` de `jo:cn` et de `nr:cn`. Mises telles quelles en génération, les routes reproposées à chaque `bd` la feraient grandir à chaque action, comme `cc` avant 4d.

## Solution

- **Slipway en arène de prise recyclée.** Chaque `jc` est construit, avec sa liste `ts` (capacité réservée de **64**), dans une **arène de 2 048 o** prise à la génération. Posé, il la garde jusqu'au rendu de la génération : c'est un objet de partie, fait une fois. Refusé, son `rendre` (nouveau) remet l'arène dans une courte liste de **prises jc libres**, que la pression suivante vide et réutilise. C'est le mécanisme de l'arène de prise de 4d (H-4d-3), appliqué à une seconde sorte.
- **Point souris réécrit en place.** Le `{ra,p}` de `jc:hq` devient un membre du `jc` (le point et sa boîte `p` en valeur), réécrit à chaque appel. Plus aucune allocation par frame tenue.
- **Routes candidates en zone frame, routes gardées recopiées en génération.** `pico8_trade_new` et `trade:kg` prennent la zone de l'appelant. `jo:ib` (`lw.t`), `jo:cn` et `nr:cn` passent la zone frame. La fermeture `kf` de `jo:ib`, au moment où `gu` accepte une route, en fait une **copie profonde en génération** (structure, `lp`, `rl`) et ajoute la copie à `ib`. Une route gardée l'est une fois ; les reproposées meurent avec la frame.
- **Traînées en zone particules.** Une sorte nouvelle, `ts`, de places nues de 24 o, **680 places**, dans les ~17 Ko libres des 128 Ko : la zone particules passe de 113 822 à 130 822 o, sans relèvement, somme des zones inchangée (1487 Ko sur 1536). `jc:ch` prend une place, `kj` la rend au `del`, le `rendre` du slipway rend celles qui vivent encore.
- **`fo` coupé au vidage.** Lecture faite pour l'inventaire : un `jo.fo` accroché par un `jc` (ou une structure tenue) n'est **pas** remis à nil par le `kj` d'une planète hors écran ou « ll » (cart l. 541-549 : `return` avant `self.fo=nil`), puis relu (`fo.dw`) quand elle revient à l'écran. Recycler l'arène d'un `jc` refusé ferait relire une arène vidée. `oublier` coupe désormais le `fo` des entités de `cg.np` qui désigne l'entité rendue.
- **Inventaire du §4** refait pour les slipways, les routes, les traînées et l'arène de prise jc, relevé « éclats `ga` » compris.
- **Budget.** Le maximum sur les e2e reste celui de `pause_restart`, sans slipway : `M_4e = M_4d` (130 720 o, H-4d-5). Projection **≈ 147 Ko** sous 704 Ko : **on continue**.
- **Verrou.** `slipway`, `slipway_display` et `trade` sortent : **21 / 66 → 18 / 55** mesuré par le prototype à 24057b4 ; **16 / 47 → 13 / 36** une fois 4d finie (H-4d-2).

Aucun pixel ne change, aucun tirage `rnd` n'est déplacé : le prototype à 24057b4 garde les 26 `*.sha256` et `make sanitize-net` (« 26 propre(s) »).

## User Stories

1. En tant que mainteneur du plan, je veux que le slipway, sa liste `ts`, son point souris, ses traînées et les routes quittent la libc et la transitoire, pour que `slipway`, `slipway_display` et `trade` sortent du verrou.
2. En tant que développeur, je veux que chaque `jc` soit construit dans une arène prise à la génération, avec sa liste `ts` réservée dedans, pour qu'un slipway et tout ce qu'il possède vivent et meurent d'un bloc.
3. En tant que développeur, je veux qu'un slipway posé garde son arène jusqu'au rendu de sa génération, pour que les planètes (`oe`), les routes (`rl`), les registres et les sondes qui le désignent lisent toujours un objet vivant.
4. En tant que développeur, je veux qu'un slipway refusé remette son arène dans la liste des prises jc libres à son `rendre`, et pas avant, pour qu'aucun `dg`, `he`, `oublier` ni `kj` ne lise une arène déjà réutilisée.
5. En tant que développeur, je veux que la pression suivante vide et réutilise une arène jc libre avant d'en prendre une nouvelle, pour que des clics répétés sur une planète ne fassent plus grandir la génération.
6. En tant que développeur, je veux que le point d'oubli de partie, le chargement et l'arrêt vident la liste des prises jc libres, pour qu'une pression de la partie suivante ne reprenne jamais une arène de la génération rendue.
7. En tant que développeur, je veux que `jc` ait un `rendre`, pour que ses traînées vivantes reviennent à la zone particules quand il quitte `hv`, en fin de partie comme au refus.
8. En tant que développeur, je veux que le point souris de `jc:hq` soit un membre du slipway réécrit à chaque appel, parce que seul `self.re` le désigne, pour qu'un glissé tenu ne fasse plus aucune allocation par frame.
9. En tant que développeur, je veux que `pico8_trade_new` et `trade:kg` prennent la zone de l'appelant, pour que la règle de 0b (une zone par propriétaire ou par appel) s'applique aux routes.
10. En tant que développeur, je veux que la proposition `lw.t` de `jo:ib` et toutes les routes candidates de `jo:cn` et `nr:cn` soient faites en zone frame, parce qu'elles ne sont désignées que par des variables locales de `bd`, pour qu'un `bd` qui repropose des routes connues ne fasse plus grandir la génération.
11. En tant que développeur, je veux que la fermeture `kf` de `jo:ib` recopie en génération la route qu'elle garde, structure et listes comprises, pour qu'une route gardée dans `by` et `cc` survive à la remise à zéro de la frame.
12. En tant que développeur, je veux une fonction de `pico8_trade` qui recopie une route dans une zone donnée, pour que la copie profonde ne soit écrite qu'une fois et ne pique pas dans les champs depuis un autre module.
13. En tant que développeur, je veux une sorte `ts` de la zone particules, à places nues de 24 o, pour que les traînées soient prises et rendues une à une comme les particules de `qy`.
14. En tant que développeur, je veux que `kj` rende la place d'une traînée au moment où il la retire de `ts`, en gardant le dessin final de la traînée morte (pas de `else` dans le cart), pour que les traînées ne fuient plus.
15. En tant que développeur, je veux que la liste `ts` réserve 64 éléments dans l'arène du slipway, parce que la lecture borne une moyenne de ~21 traînées vivantes par slipway, pour qu'elle ne grandisse jamais en jeu.
16. En tant que développeur, je veux qu'au vidage `oublier` coupe le `fo` de toute entité de `cg.np` qui désigne l'entité rendue, parce qu'une planète hors écran ne remet pas son `fo` à nil, pour qu'une planète revenue à l'écran ne relise pas un slipway recyclé.
17. En tant que mainteneur, je veux que la génération ne bouge pas pendant les frames où un slipway est tenu au-dessus du vide, pour prouver que `jc:hq` n'alloue plus par frame.
18. En tant que mainteneur, je veux un rejeu qui enchaîne vingt pressions refusées sur une planète et dont la génération ne bouge plus après la première, pour prouver le recyclage de l'arène jc.
19. En tant que mainteneur, je veux qu'un slipway posé après des refus prenne exactement une arène jc de plus, pour prouver qu'un slipway posé n'est jamais recyclé.
20. En tant que mainteneur, je veux qu'une pression refusée, puis une nouvelle partie, puis une pression ne fassent pas planter ASan, pour prouver que la liste des prises jc libres est vidée au point d'oubli.
21. En tant que mainteneur, je veux que sur `trade_rich_mine` la génération grandisse d'exactement une arène jc entre f139 et f151 (saut d'année de f140 et deux clics sur des planètes à f142 et f148), pour prouver à la fois que `bd` ne garde pas les routes reproposées et que le second slipway refusé reprend l'arène du premier.
22. En tant que mainteneur, je veux qu'une route gardée à f139 de `trade_rich_mine` se relise intacte sous ASan après douze remises à zéro de la frame, pour prouver que la copie est en génération et pas en zone frame.
23. En tant que mainteneur, je veux qu'à chaque frame de `trade_rich_mine` les places `ts` vivantes égalent la somme des `ts->count` des slipways de `cg.jc`, pour prouver qu'aucune traînée n'est perdue ni rendue deux fois.
24. En tant que mainteneur, je veux qu'après `trade_rich_mine` puis un rechargement les places `ts` vivantes reviennent à 0, pour prouver que le `rendre` du slipway rend ses traînées.
25. En tant que mainteneur, je veux que le rejeu écrive `ts`, `max_vivantes ts` et `max_elements ts`, pour relever le pire des traînées sans écrire de test.
26. En tant que gardien du budget, je veux que la sorte `ts` tienne dans les 128 Ko de la zone particules par le contrôle à la compilation existant, sans relèvement, pour ne rien prendre aux 49 Ko de marge.
27. En tant que gardien du cliquet, je veux que `slipway`, `slipway_display` et `trade` sortent de la liste sans qu'aucun compte ne monte, pour que le verrou reste à égalité stricte.
28. En tant que gardien de l'invariant, je veux les 26 `*.sha256` inchangés et `make sanitize-net` vert, pour prouver que l'arène, la copie des routes, le point en place et les traînées en places ne changent rien au jeu.
29. En tant que mainteneur du budget, je veux `M_4e` relevé sur les 26 e2e et comparé à `M_4d`, avec la projection et la décision écrites, pour appliquer l'arrêt anticipé du §3.
30. En tant que développeur des familles suivantes, je veux l'inventaire du §4 refait pour les slipways, les routes, les traînées et l'arène jc, relevé « éclats `ga` » compris, pour savoir ce que le point d'oubli et `oublier` coupent désormais.
31. En tant que développeur de 4h, je veux une ligne « *Reçu de l'étape 4e* » qui dise que le point souris d'un slipway tenu garde `ok.qa` (`ra = mp`), pour que 4h, qui choisit la zone de `ok.qa`, sache qui le relit.
32. En tant que lecteur du plan, je veux que le §2 dise que seules les routes gardées dans `by` sont en zone partie et que les routes candidates de `bd` sont en zone frame, pour que le tableau colle au code.
33. En tant que mainteneur des commentaires, je veux que les seuls commentaires touchés soient les contrats C devenus faux (« Nothing frees the struct: TROU-7 » de `pico8_jc_t`, « a fresh one per call, never freed » du point souris, « which owns it from then on » de `jc:ch`, le `/* TROU-7 */` de `kg`, « read, not kept » des clés de route), les contrats des fonctions nouvelles et les `assert` porteurs de message, pour respecter les quatre destinations du `CLAUDE.md`.

## Implementation Decisions

### Inventaire des sites de 4e, et zone retenue

Chaque site par frame, par action ou par `bd` est tranché par lecture : « qui d'autre garde l'ancienne valeur ? ».

| Site | Ligne à 24057b4 | Fréquence (lecture) | Qui garde l'objet | Zone retenue |
|---|---|---|---|---|
| structure `jc` | `slipway` 344 | **une par pression** sur une planète « ly » ou un relais (`jo:ldown`, `nr:ldown`) ; posée ou refusée par `jc:ey` | posé : `oe` des deux bouts, `rl` des routes gardées, registres, `et` d'un autre slipway tenu, cible `et` d'une sonde (`bv`) ; refusé : plus rien après son `rendre` (voir « Arène jc ») | **arène jc**, prise à la génération, gardée si posé, recyclée si refusé |
| liste `ts` et son conteneur | `slipway` 89, 90 | une par `jc` | le `jc` seul | **dans l'arène jc**, capacité réservée 64 |
| point souris et sa boîte `p` | `slipway` 111, 112 | **par frame** où le slipway est tenu hors d'une planète | `self.re` seulement (voir plus bas) | **réécrit en place** : membres du `jc` |
| traînée `pico8_ts_t` | `slipway_display` 36 | **par frame** où le slipway est visible, tirage 0,08 par sens | la liste `ts` seule | **place de la sorte `ts`**, zone particules, rendue au `del` et au `rendre` du `jc` |
| structure `trade` de `lw.t` | `trade` 29, appelée par `planet_trade` 224 | par `bd`, par planète, par slipway de `oe` | la proposition, variable locale de `jo:ib` | **zone frame** |
| route de `kg` (structure, `lp`, `rl`) | `trade` 29, 135-138 | par `bd`, par chemin parcouru par `jo:cn` / `nr:cn` | candidate : `gf.t` (locale de `nr:cn`), l'argument de `kf` ; seule `kf` la garde, et seulement si `gu` est faux | **zone frame** ; la copie gardée par `kf` en **génération** |

- **Pourquoi pas « valeur » pour les routes.** Une route est désignée par pointeur depuis `by`, `cc`, la proposition et `gf` ; `pico8_trade_eq` compare des extrémités, pas des adresses, mais `by`/`cc` sont des `pico8_ents_t` d'en-têtes. La rendre valeur toucherait `by`, `cc`, `dh`, `db`, les panneaux : hors de proportion.
- **Pourquoi la zone frame pour les candidates.** `bd` tourne dans un `hy:lj`, dans un update, entre deux remises de la zone frame. `lw` et `gf` sont des locales sur la pile ; les commentaires existants (`lw` « on the stack », `&gf` « safe only while no `cn` implementation stores its proposal ») le disent déjà. La seule fonction qui range une route est la fermeture `kf` de `jo:ib` (`grep` de `->kf(` et de `.kf =` sur `pico8_jo_trade_proposal_t` : un seul appel, une seule affectation). Toutes les autres lectures (`gu_trade`, `nr:cn` sur `t->rl`, `jo:cn` sur `t->qe`) sont dans le même `bd`.
- **Pourquoi « copier à la garde » et pas « faire `kg` dans la zone de l'appelant ».** `jo:cn` ne sait pas, quand il fait `kg`, si `gu` va accepter : la plupart des routes qu'il fait sont reproposées et refusées (32 `kg` pour 7 routes gardées sur `trade_rich_mine`). Faire `kg` en génération dans `jo:cn` ferait grandir la génération à chaque `bd`. La copie a lieu une fois par route gardée.
- **Pourquoi la copie est exacte.** Le cart fait `add(ib,a)` sur l'objet `a` lui-même. Une copie ne se distingue de `a` que si quelqu'un garde `a` et le compare par adresse ou le modifie après `kf`. Après `lw:kf(t:kg(self))`, `jo:cn` fait `return` ; `nr:cn` ne relit que `gf.mu`. Rien ne modifie une route après sa construction (`lp`/`rl` sont faites par `la`, jamais écrites ensuite). `cc` désigne les routes de `by` (par `bd_route_visit`), donc les copies. Le prototype garde les 26 `*.sha256`.
- **Pourquoi réécrire le point souris en place.** Le cart fait un `{ra=mp,p=mp/128}` neuf à chaque `hq` et le range dans `self.re`. Qui garde l'ancien ? Le `grep` des lecteurs de `re` d'un `jc` donne `jc:kj`/`jc:lg` (dessin), `jc:ce` (compare `eb(w,qu)==re`, jamais vrai pour un point), `jc:ey` (remplace `re` ou garde le point d'un refusé), `jc_end_p`/`jc_end_ra`, et `oublier` (compare `lheld->re == e`, sans déréférencer). Aucun ne garde le point ailleurs que dans `self.re`. Un membre du `jc` réécrit à chaque appel a la même valeur à chaque lecture. Coût : 144 + 8 o par `jc` (`pico8_jc_t` passe de 960 à 1 112 o).
  - Écarté, une statique unique : un `jc` refusé (frame N) et un nouveau `jc` tenu (frame N+1) la partageraient, et c'est un pointeur de plus à l'inventaire. Écarté, la zone frame : sûr par lecture (un refusé n'est plus lu après le dessin de la frame N), mais une durée plus courte que celle de l'objet qui le désigne, que le §4 ne permet que prouvée ; le membre ne demande aucune preuve.
- **Pourquoi l'arène jc.** C'est le cas de la prise d'icône refusée de 4d (H-4d-3), sur une autre sorte :
  - *Le `jc` refusé est-il gardé ?* **Non**, voir l'inventaire. Après son `rendre`, plus rien ne le lit, une fois le `fo` coupé (voir plus bas).
  - *Une place dans une zone qui rend ?* **Non** : un slipway posé vit toute la partie, et le nombre de slipways posés n'a pas de borne (§7).
  - *Recycler le `jc` abandonné ?* **Oui, par une arène**, comme 4d.

### Arène jc : recycler le slipway refusé

- **Toujours une arène.** Tous les `jc` du jeu naissent d'une pression (`jo:ldown`, `nr:ldown`) : `pico8_jc_struct_new` prend toujours son arène, sans « zone de construction » à poser autour de l'appel (c'est plus simple que 4d, où `qh`… naissent aussi hors prise). Un `jc` fait par un TU en a une aussi.
- **Taille au doigt mouillé : 2 048 o**, nom « prise jc » pour `pico8_fatal`. Le lot fait 96 (en-tête d'arène) + 1 112 (`pico8_jc_t`) + 24 (conteneur `ts`) + 64 × 8 (éléments réservés) = **1 744 o**. Une liste `ts` qui dépasserait 64 grandirait dans l'arène, copie comprise, et la remplirait : `pico8_fatal` « zone prise jc pleine », qui se lit comme un slipway à plus de 64 traînées vivantes et se relève d'un chiffre.
- **Prises jc libres.** Une liste statique d'arènes libres, capacité 4, `assert` porteur de message. Même forme et mêmes points d'oubli que les prises libres de 4d (H-4d-3). La forme exacte est laissée à l'implémenteur : seconde instance d'un petit pool générique « arènes de prise (taille, nom) » si la forme de 4d s'y prête, ou liste propre au module slipway.
  - **Ajout** : au `rendre` du `jc`, **en dernier** (après ses traînées), si `p_` est vrai et l'état encore « kz » (un `jc` posé est passé « ph » par `d_` → `ks("ph")`, `jc` n'ayant pas de `dq`).
  - **Retrait** : `pico8_jc_struct_new` prend une arène libre et la vide (opération « vider une arène » de 4d, H-4d-3), sinon il en prend une nouvelle à la génération.
  - **Borne** : un `jc` refusé à la frame N (dans `ok:lj`, après `dg`) est retiré par le `dg` de N+1 et rendu au vidage de N+2. Une pression ne peut tomber qu'au plus tôt en N+1 (`b.jn` et `b.hm` de la même frame font naître et relâcher le même `jc`). Au plus deux arènes en vol, plus une par slipway posé.
- **Oubli.** La liste des prises jc libres est vidée au point d'oubli de partie (`oublier_partie()`, après le vidage de la file, avant le rendu) et après la remise à zéro du chargement et de l'arrêt, aux côtés des prises libres de 4d.
- **Angle mort ASan**, comme 4d : la pression suivante réalloue aussitôt les mêmes adresses. Un pointeur oublié vers un `jc` refusé lirait le nouveau sans planter. L'inventaire ci-dessous, pas ASan, garantit qu'il n'y en a pas.
- **Ce que l'arène coûte.** 2 048 o par slipway posé au lieu de son lot (1 648 o) ; une arène en vol au plus pour les refus. Sur `trade_rich_mine` (sept posés, deux refusés), ≈ 8 × 2 048 o.

### `fo` coupé au vidage : un trou de lecture

- **Lecture** (cart l. 541-549, `pico8_jo_kj`) : `jo:kj` fait `if (not gd(self))return` puis, pour un « ll », un second `return` possible, **avant** `self.fo=nil`. `bv` (sonde, structure tenue) et `jc:hq` écrivent `et.fo = self` à chaque update de la prise. Pour `jc:hq`, `et` vient de `jc:ce` : une planète de `cg.np` près du **segment** `qu`→souris, qui peut sortir de l'écran (caméra déplacée au pavé pendant le glissé, `qu` hors écran). Une telle planète garde `fo` sur le `jc` après sa mort et, revenue à l'écran, lit `fo->probe->def->dw` (`pico8_planet_display.c`).
- **Aujourd'hui** le `jc` fuit (TROU-7) : la lecture est saine. **Avec l'arène jc**, elle porterait sur une arène vidée (empoisonnée sous ASan) ou réutilisée.
- **Décision** : `oublier(e)` (`pico8_cart.c`), appelé pour chaque entité au vidage avant son `rendre`, remet à nil le `fo` de toute entité de `cg.np` dont `fo == e`. Il couvre aussi les structures de prise de 4d accrochées par `bv` (H-4d-6). `jc.fo` n'en a pas besoin : `jc:kj` le remet à nil sans condition (l. 680). `g_jc_too_far.fo` et `g_bv_et.fo` sont écrits seulement (4a).
- C'est un **trou de l'inventaire de 4d** si 4d ne l'a pas déjà comblé (« À revérifier une fois 4d finie ») : la PRD 4d écrit « lus et remis à nil par le `kj` de la même frame », ce qui n'est vrai que pour une planète dessinée.

### Traînées : la sorte `ts` de la zone particules

- **La sorte.** `PICO8_PARTICULES_TS`, places nues (pas d'arène) de 24 o (`pico8_ts_t` fait 20 o, trois cellules de 8 o), nom « particules ts ». `pico8_jc_ch` prend une place par la poignée de la sorte ; `jc:kj` la rend juste après `pico8_ptrs_del`, avant le dessin qui ne lit plus que des copies locales (`new_ra`) ; le `rendre` du `jc` rend les places encore dans `ts`. `pico8_zone_particules_noter_elements` accepte `ts`, et le rejeu écrit `ts`, `max_vivantes ts`, `max_elements ts`.
- **Combien de places — lecture du pire.**
  - Une traînée naît avec `l = |d|` en pixels (`d = t.ra - f.ra`, `v` unitaire) et perd 1 à chaque dessin. `jc:ce` refuse un slipway dont `#(sp-dp)` (carré de la longueur en unités `p` = `ra/128`) dépasse `ov("space folding",0.5,1.08)`. Donc `|d| ≤ 128 × √1,08 ≈ 133` pixels (≈ 90,5 sans « space folding ») : une traînée vit **au plus 134 dessins**.
  - Au plus une naissance par sens et par dessin, et seulement vers une planète de `hg` : **268 vivantes au pire absolu** par slipway, **≈ 21,3 en moyenne** (2 × 0,08 × 133), écart-type ≈ 4,4.
  - Hors écran, un slipway n'est ni dessiné ni vieilli : ses traînées gèlent, comme les éclats `ga` (étape 3).
  - Un `jc` tenu n'a jamais de traînée : `hg_own` est vide (`jc:pq`) et aucun `bd` ne tombe pendant la prise (4d).
- **Liste par slipway : 64 éléments réservés**, à 9,7 écarts-types de la moyenne. Le pire absolu (268) n'est pas atteignable en pratique ; le dépasser sort par `pico8_fatal` (« zone prise jc pleine »).
- **Nombre de places : 680.** La zone particules occupe 113 822 o sur 131 072 ; une place de 24 o coûte 25 o dans le contrôle (`SORTE_BYTES`, octet de prise compris) ; 690 places la rempliraient exactement. 680 places la portent à **130 822 o**, contrôle à la compilation tenu, **sans relèvement** : la somme des zones reste 1487 Ko et la marge de 49 Ko reste entière.
- **Risque accepté** (comme `qy` 24 et `ga` 8 à l'étape 3) : à ~21 traînées par slipway desservi dans les deux sens, 680 places se dépassent vers **une trentaine de tels slipways visibles à la fois**. Le relevé des e2e (13 au plus, sur `trade_rich_mine`) est loin dessous. Un `pico8_fatal` « zone particules ts pleine » se lit d'abord comme une traînée jamais rendue (l'invariant « places vivantes = Σ des listes » le tranche), puis comme une limite trop basse, relevée sur les 49 Ko (règle de 4a), ce que la projection compte alors.

### Le `rendre` du slipway

- `jc` gagne `rendre`. Il entre donc dans la file de sortie quand `he` (refus) ou le `jl()` d'`eu`/`_init` (fin de partie) le retirent de `hv`.
- Ordre : rendre chaque place de `ts` (morceau 3 ; avant, les traînées fuient encore, TROU-7), remettre `ts` à vide, puis, s'il est refusé, remettre son arène dans les prises jc libres, **en dernier**.
- **File de sortie.** Les slipways s'ajoutent aux planètes et aux autres entités à `rendre` que le `jl()` d'`eu` met en file d'un coup (`PICO8_FILE_SORTIE_CAP` 256). Aucun e2e ne recommence une partie qui a des slipways ; la borne se lit « 90 planètes + slipways + structures posées + interface ≤ 256 ». Un `pico8_fatal` « file pleine » se relève d'un chiffre : la file est un tableau statique de pointeurs, hors des zones.

### Choix de la zone par le propriétaire

- `pico8_trade_new` et `trade:kg` prennent la zone en premier argument (forme de `pico8_jo_pq_dans`, `pico8_jf`, `pico8_gn`). Leurs appelants du jeu passent `pico8_zone_frame()`. Les TU passent la zone qu'ils veulent (la génération si la route doit survivre à une frame).
- Une fonction nouvelle de `pico8_trade` (nom laissé à l'implémenteur) recopie une route dans une zone : structure, `lp` et `rl` neuves (`pico8_ents_cat` avec une liste vide), `qe` recopié. Contrat à écrire : la copie ne partage rien avec l'original.
- La fermeture `kf` de `jo:ib` l'appelle avec `pico8_zone_partie()` juste avant `pico8_ents_add(ib, …)`.
- `pico8_jc_struct_new` prend son arène elle-même (voir « Arène jc »).
- Les contrats devenus faux sont réécrits : « Nothing frees the struct: TROU-7 » (`pico8_jc_t`), « a fresh one per call, never freed, nor its `p` » (point souris), « `/* TROU-7 */` Appends one particle to `self.ts`, which owns it from then on » (`jc:ch`), le `/* TROU-7 */` de `pico8_trade_kg`, « The keys of `pico8_trade_new`, read, not kept » (la structure garde `lp`/`rl`).
- `<stdlib.h>` et l'inclusion de la transitoire sortent des trois fichiers.

### Relevé « éclats `ga` rendus en cours de partie », refait pour 4e

Lu à 24057b4. Les objets de 4e vivent plus longtemps qu'un éclat : peuvent-ils en désigner un après le rendu de sa place ?

| Objet de 4e | Peut-il désigner un éclat ? | Pourquoi |
|---|---|---|
| `re` du `jc` tenu (`ok.lheld`) | oui, inatteignable | `jc:hq` prend `ok.o_` quand `o_.gv` existe, et `ga` hérite `gv` de `jo` ; **remis à nil** par `oublier` (étape 3), réécrit au `hq` suivant |
| `qu`, `re` d'un `jc` posé | non | `jc:ey` ne pose que si `d_` réussit sur un `re` que `ui:jz("gv")` a accepté : `jo:gv` ne rend vrai que pour `me=="ly"`, l'éclat reste « unknown » ; `qu` vient de `jo:ldown`/`nr:ldown`, sur « ly » ou un relais |
| `et` d'un `jc` | non | `jc:ce` ne cherche que dans `qu.oe`, `cg.np` et `cg.jc` ; l'éclat a `.tags = 0` ; le « too far » est statique |
| `fo` accroché par un `jc` | non (sens inverse) | `jc:hq` écrit `et.fo` sur une planète de `cg.np` ou un slipway, jamais sur un éclat |
| point souris | non | `ra` est `ok.qa`, `p` est un membre du `jc` |
| `lp`/`rl` d'une route, `hg` | non | une route ne passe que par des slipways posés et leurs planètes ; `hg` tient des identifiants (un éclat emprunte celui de la planète remplacée, chaîne de génération, comparée par `strcmp`) |
| traînées | non | positions en valeur |

### Inventaire du §4 pour 4e

Quatre durées de vie finissent sur des objets de 4e :

- **la génération** (slipways posés et leurs arènes, routes gardées) : rendue au vidage qui suit le `eu` suivant, ou remise à zéro au chargement et à l'arrêt ;
- **l'arène jc d'un slipway refusé** : réutilisée par la pression suivante, après son `rendre` ;
- **la place d'une traînée** : rendue au `del` dans `kj` ou au `rendre` du slipway ;
- **la frame** (routes candidates) : remise à zéro en tête de `update60`.

**Méthode.** Refaire **mécaniquement**, sur le code du commit, les deux `grep` de 4a sur les 65 fichiers de `CORE_SRCS` (pointeurs `static`, puis champs pointeurs des structures `static`), plus :

- le `grep` des écritures de `fo` (`->fo = `, `pico8_probe_et_hang_fo`) et de leurs lectures ;
- le `grep` des lecteurs de `->re` d'un `jc` ;
- le `grep` des appels de `->kf(` et des lecteurs de `->by`/`->cc`.

**Relevé fait à 24057b4** (18 pointeurs `static`, 37 structures `static` non constantes), avec ce qu'ajoute 4e :

| Pointeur | Peut viser | Statut |
|---|---|---|
| prises jc libres (ajouté par 4e) | des arènes de la génération | n'y entrent que des arènes de `jc` refusés **déjà rendus** ; **vidées au point d'oubli de partie** et après la remise à zéro du chargement et de l'arrêt |
| `g_arenes[ts]` (`zone_particules`) | — | `ts` est une sorte de places nues : pas d'arène tenue |
| `g_ok` → `ok.lheld` | le `jc` tenu | remis à nil par `ui:bh` après `ey` ; le nouvel `ok` après la bascule |
| `g_ok` → `ok.o_`, `ok.gj` | — | un `jc` n'a pas `hd` : jamais dans `cv.hd`, jamais survolé |
| `g_hv`, `g_cv[].list`, `g_cg[].list` (`entity`) | des `jc` | un refusé est retiré par `he` (`dg` de N+1) avant d'entrer en file ; refaits au `jl()` d'`eu` |
| `g_file[]` (`file_sortie`) | un `jc` refusé ou abandonné | vidé avant le rendu de la génération et avant tout recyclage d'arène (le recyclage est fait **par** le `rendre`) |
| `jo.fo` (planètes de `cg.np`) | un `jc` tenu, une structure tenue | **coupé par `oublier`** au vidage (trou de lecture ci-dessus) |
| `jc.fo` | un `jc` tenu, une sonde, un `mz` | remis à nil sans condition par `jc:kj` (l. 680) |
| `g_jc_too_far.fo` (`slipway`), `g_bv_et.fo` (`probe`) | un `jc`, un hôte de `bv` | **écrits seulement, jamais lus** (4a) |
| `et` d'un autre `jc` tenu, d'une sonde | un `jc` | un refusé n'est plus dans `cg.jc` quand une autre prise commence (N+1, après `dg`) ; un posé vit la génération |
| `oe` des planètes, `rl` des routes gardées | des `jc` posés | ne visent jamais un refusé (`kf` seulement au succès de `ey` ; `bd` jamais pendant une prise) |
| `by`, `cc` | des routes | ne tiennent que des copies en génération |
| `lw.t`, `gf.t` | des routes candidates | locales de `bd`, mortes avant la fin de l'update |
| `g_mo`, `g_je`, `g_kc`, `g_ju`, `g_dk`, `g_fm`, `g_gb[]`, `g_ie_gy`, `g_jo_oc`/`g_jo_ir`, zones et poignées | — | ne peuvent pas viser un objet de 4e, comme à 4a, 4c et 4d |

**Objets d'autres zones qui désignent un objet de 4e** : les panneaux de planète (`jo:gh`, zone interface) ne lisent que `by->count` ; la sonde `mz`/`g_` qui vise un slipway posé (`bv`, `et`) est une entité rendue par la file avant le rendu de la génération.

**Conclusion** : le point d'oubli de partie reçoit une entrée de plus (les prises jc libres), et `oublier` coupe `jo.fo`. Un pointeur trouvé ensuite par ASan y est ajouté et noté dans « Découvert » comme un **trou de l'inventaire**.

### Verrou

- `tools/alloc_guard_exceptions.txt` : retirer `slipway` 5, `slipway_display` 1, `trade` 5.
- **Prévision : 21 / 66 → 18 / 55** à 24057b4, mesurée par `make alloc-guard` sur le prototype (« verrou objets 65 allouent 18 appels 55 »). Une fois 4d finie, **16 / 47 → 13 / 36** (H-4d-2). L'arène jc ne change aucun compte : elle est prise à la génération.

### Budget : mesure, projection, décision

- **Mesure par prototype** à 24057b4 (copie `git archive`, hors dépôt) : `jc` et `ts` en génération sans arène, point en place, traînées en sorte `ts` (680 places), routes candidates en frame et copiées à la garde. 26 e2e rejoués avec `PICO8_REMPLISSAGE`, contre la même mesure sur 24057b4 :
  - 26 `*.sha256` identiques ; `sanitize-net` « 26 propre(s) » ; le rejeu de vingt refus propre sous ASan ;
  - `max_octets partie` : **42 992 o** (`trade_rich_mine`, contre 26 256 sans 4e : neuf `jc` à 1 648 o, sept routes, la croissance de `by`/`cc`), contre **42 880** pour `pause_restart`, inchangé (aucun slipway) ;
  - par route gardée de deux planètes : 160 + 2 × (24 + 32) = **272 o** (+ 32 o de `by`/`cc` qui grandissent) ;
  - par `jc` : **1 648 o** (struct 1 112, conteneur 24, 64 × 8) ;
  - **vingt refus** (rejeu synthétique : f0-34 de `slipway_build`, puis vingt cycles « pression sur « p27 » (88,37), dix frames tenues en (40,100), relâché, trois frames ») : la génération ne bouge pas pendant les dix frames tenues, et grandit de 1 648 o par cycle sans recyclage ;
  - `trade_rich_mine` : génération identique de f139 à f141 (saut d'année et `bd` de f140 : routes reproposées, aucune gardée), puis +1 648 à f142 et à f148 (deux clics sur des planètes, deux `jc` refusés) ;
  - zone frame, maximum par frame : 32 464 → **37 344 o** sur `trade_rich_mine` (32 `kg`), inchangé ailleurs ; zone de 256 Ko ;
  - traînées : **13 vivantes au plus** (`trade_rich_mine`), 4 sur `synthesizer_second_link`.
- **Projection sous H-4d.** `M_4d` = 130 720 o (`pause_restart`, H-4d-5). Avec l'arène, `trade_rich_mine` vaut ≈ 75 488 (mesure de 4d avec son arène) + 8 × 2 048 + 7 × ~300 ≈ **93,8 Ko**, sous `pause_restart`. Donc **`M_4e` = `M_4d`** sur les e2e. Estimations restantes (4a), génération : 4f ~1, 4g ~6, 4h ~12, soit **19 Ko** ; chargement : 4g ~32 Ko. `r = max(1, M_4e / (2 + 20 + 160 + 30 Ko))` = **1**. `P_4e` ≈ 127,7 + 19 ≈ **147 Ko** ≤ 704 ; `Q_4e` = 32 Ko ≤ 64. Zone particules : 130 822 ≤ 131 072. **On continue.**
- **Hors e2e** (§6, §7) : chaque slipway posé coûte une arène (2 Ko) et ses routes ~0,3 Ko ; cent slipways ≈ 230 Ko, encore sous les 704 Ko moins `M_4e`. Les routes candidates d'un `bd` vont en zone frame : un réseau dense de relais (`nr:cn` parcourt tous les chemins sans slipway répété) pourrait en faire assez pour remplir les 256 Ko ; `pico8_fatal` « zone frame pleine » le dirait.
- **Mesure à refaire** après conversion : `M_4e`, `K_4e`, `max_vivantes ts`, `max_elements ts`, le maximum de la file de sortie si l'accesseur existe ; dans le commit et dans « Découvert », comparés au prototype. Un écart s'explique avant la projection.
- **Décision**, règle de 4c : `P_4e ≤ 704` et `Q_4e ≤ 64` : on continue ; sinon déplacer la frontière, puis relever d'un chiffre sur les 49 Ko, puis **arrêt de l'étape 4 à 4e** et signalement à l'humain (zone, limite voulue, somme actuelle 1487 Ko sur 1536).
- Un `pico8_fatal` « zone partie pleine » pendant 4e se lit d'abord comme un `jc` refusé ou une route candidate entrés tels quels en génération (le rejeu de refus et la fenêtre f139-f151 le montrent), puis comme une limite trop basse.

### Note « Découvert » et clôture

À la clôture, sous l'entrée 4 de `docs/plan-4.md`, une note « *Découvert (4e) :* » de quelques phrases :

- `jc` et sa liste `ts` en arène de prise jc (2 048 o, recyclée au refus), point souris réécrit en place, routes candidates en zone frame et copiées en génération à la garde, traînées en sorte `ts` (680 places de 24 o, zone particules à 130 822 o) ;
- correction de la table de 4a : `jc` est par action (chaque pression), pas une fois par slipway ; `kg` est par `bd` et ses routes ne sont gardées qu'une fois ;
- le trou de lecture de `jo.fo` et sa coupe dans `oublier` ;
- l'inventaire et le relevé « éclats `ga` » refaits ;
- `M_4e`, `K_4e`, `P_4e`, `Q_4e`, `r`, la décision ; l'estimation de 4a (~30 Ko) contre la mesure ;
- les comptes du verrou.

Le §2 est corrigé : ligne « Frame », ajouter « routes `trade` candidates de `bd` (propositions, `kg`) » ; ligne « Partie », « routes `trade` » devient « routes `trade` gardées dans `by` ».

Sous l'entrée 4, une ligne « *Reçu de l'étape 4e :* » **pour 4h** : le point souris d'un slipway tenu garde `ok.qa` (`ra = mp`) ; il est réécrit à chaque `hq` et lu par le `kj` de la même frame ; un `jc` refusé le garde jusqu'à son `rendre` sans le relire après le dessin de sa dernière frame.

La PRD est retirée de `docs/prd/` dans le commit de clôture.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 : `make test-all`, `make sanitize-net`, 26 `*.sha256` identiques, `alloc-guard` à égalité stricte. Aucun ne passe la transitoire à un nouvel appelant.

1. **Routes : candidates en zone frame, copie en génération à la garde.**
   - Les tests d'abord :
     - TU de `pico8_trade` : `kg` et `trade_new` font la route dans la zone passée ; la copie ne partage ni structure ni listes avec l'original, et se relit après une remise à zéro de la zone frame (enfant ASan) ;
     - la fenêtre de `trade_rich_mine` passe de f137-f151 à **f139-f151**, égalité stricte de `prise − R` (le slipway posé à f136 fait garder une route au `bd` de f139 : légitime) ; rouge si `kg` est fait en génération ;
     - une route de `by` gardée à f139 se relit intacte à f151 sous ASan.
   - `pico8_trade_new` et `kg` prennent la zone ; `jo:ib`, `jo:cn`, `nr:cn` passent la zone frame ; la fermeture `kf` recopie en génération.
   - Le verrou : `trade` sort. **21 / 66 → 20 / 61** (à 24057b4).
   - Ce morceau vient d'abord : il ne dépend pas de 4d au-delà de son morceau 1, déjà versé.
2. **Slipway en arène jc, point en place, `rendre`, coupe de `fo`.**
   - Les tests d'abord :
     - `test_pico8_jc` : le point souris est le même membre à chaque `hq`, ses valeurs suivent `mp`, et la génération ne bouge pas sur dix `hq` ;
     - le rejeu de refus (fixture) : génération constante pendant les frames tenues ; `prise − R` égal du premier au vingtième refus (rouge sans recyclage : +1 648 o par cycle) ; un slipway posé après les refus, puis un refus, prennent exactement une arène jc de plus ;
     - un refus, « restart », nouvelle partie, une pression : l'enfant ne meurt pas ;
     - `trade_rich_mine` : `prise − R` à f151 = `prise − R` à f139 + **une arène jc** exactement (le `jc` de f148 reprend l'arène de celui de f142, rendu à f145) ;
     - `jo.fo` d'une planète hors écran accroché par un `jc` refusé : nil après le vidage (rouge sans la coupe : relecture d'une arène vidée dans l'enfant).
   - Arène jc et prises jc libres, sur l'opération « vider une arène » de 4d ; `rendre` du `jc` ; coupe de `fo` dans `oublier` ; vidage des prises jc libres au point d'oubli et après le reset du chargement et de l'arrêt.
   - Point souris en membres du `jc`.
   - Le verrou : `slipway` sort. **20 / 61 → 19 / 56**.
   - **Prérequis : 4d finie** (H-4d-3 : « vider une arène », point d'oubli, prises libres).
3. **Traînées en zone particules.**
   - Les tests d'abord :
     - TU de la zone particules : la sorte `ts` prend et rend ses places ; 681 prises sortent par `pico8_fatal` « zone particules ts pleine » ;
     - à chaque frame de `trade_rich_mine` depuis f112, places `ts` vivantes = Σ des `ts->count` de `cg.jc` ;
     - après `trade_rich_mine` puis `pico8_cart_load()`, places `ts` vivantes = 0.
   - La sorte `ts` et son contrôle de taille ; `jc:ch` prend une place, `kj` la rend au `del`, le `rendre` du `jc` rend les vivantes ; le rejeu écrit les lignes `ts`.
   - Le verrou : `slipway_display` sort. **19 / 56 → 18 / 55**.
4. **Mesure et clôture.**
   - Le relevé sur les 26 e2e, `M_4e`, `K_4e`, les maxima `ts`, la comparaison au prototype, la projection et la décision, dans le message du commit.
   - L'inventaire refait par `grep`.
   - La note « Découvert (4e) », la correction du §2, la ligne « *Reçu de l'étape 4e* » pour 4h, le retrait de cette PRD.

Le morceau 1 est indépendant. Le 2 attend 4d. Le 3 dépend du 2 (le `rendre` du `jc` rend ses traînées). Le 4 dépend des trois.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris et le maximum de la zone partie, les places vivantes et maxima de la zone particules, par les accesseurs ou par les lignes de `PICO8_REMPLISSAGE` ;
  - les champs publics d'un slipway (`re`, `ts`, `ts->count`, `p_`, `header.me`), d'une route (`lp`, `rl`, `qe`), d'une planète (`by`, `cc`, `fo`) ;
  - un rapport ASan dans un processus enfant ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne lit ni les bouts de la zone ni une adresse, sauf pour la relire après sa mort. Les tailles attendues se calculent dans le TU (`sizeof`, `PICO8_ZONE_CELLS`, taille de l'arène jc), jamais en dur : 1 648 et 2 048 ne valent que sur x86-64.
- **`tests/test_pico8_places_vivantes.c`** (ASan, un enfant par cas, `demarrer`/`jouer_jusqu_a`, `partie_sans_registres`) :
  - **`trade_rich_mine_bd_sur_des_routes_ne_fait_pas_grandir_la_partie`** (4d) : fenêtre f139-f151. Au morceau 1, égalité stricte ; au morceau 2, différence d'exactement une arène jc. Rouge si le `bd` de f140 garde des routes reproposées, ou si le second refus ne recycle pas ;
  - **route gardée survivant à la frame** (cas nouveau) : un pointeur vers une route de `by` de « p27 » pris après f139 se relit à f151 sans crash ;
  - **slipway tenu sans allocation** et **refus répétés** (cas nouveaux) sur une fixture de `tests/fixtures/` (pas un e2e : elle n'ajoute pas de ligne couverte) : f0-34 de `slipway_build`, puis vingt cycles « pression sur (88,37), dix frames en (40,100), relâché, trois frames » ; puis un glissé posé de « p27 » à « p48 » (f35-38 de `slipway_build`) et un refus ;
  - **refus à travers une nouvelle partie** (cas nouveau, même fixture prolongée par « restart » et une pression) ;
  - **`fo` coupé** (cas nouveau) : planète de `cg.np` hors de l'écran (`gd` faux) dont `fo` désigne un `jc` refusé ; après le vidage qui rend le `jc`, `fo` est nil ;
  - **invariant des traînées** et **traînées rendues au rechargement** (cas nouveaux, morceau 3) ;
  - **inchangés** : `message_box_hold` f45 = f90, `game_end_final_score` f2 = f51 (aucun slipway), les égalités exactes de 4c/4d sur `pause_restart` (aucun slipway ni route), la sentinelle de 4a.
- **`tests/test_pico8_jc.c`** : `test_hq_mouse_point` attendait un point neuf à chaque appel (`r2 != r1`) ; il attend désormais le même membre, `ra == mp`, `p == mp/128`, et la génération inchangée. Rouge au prototype, comme prévu.
- **`tests/test_pico8_trade.c`** (et les TU qui appellent `pico8_trade_new`/`kg` : 33 appels dans sept fichiers de `tests/` à 24057b4) : argument de zone ajouté ; une route qui doit survivre à un `update60` est faite en génération.
- **`tests/test_pico8_zone_particules.c`** : la sorte `ts` comme les autres sortes nues (prise, rendu, plein).
- **Verrou** : `make alloc-guard` à égalité stricte, liste sans les trois fichiers.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. Le prototype à 24057b4 passe `make test` sauf exactement les deux cas que 4e réécrit (`test_hq_mouse_point`, la fenêtre de `trade_rich_mine`).
- **Exemples dans le dépôt** :
  - `tests/test_pico8_places_vivantes.c` : rejeu par enfant, `octets_des_registres()`, les cas de 4c et 4d ;
  - `tests/test_pico8_zone_particules.c` : sortes nues et arènes ;
  - les TU de l'arène de prise de 4d (fixture de prises refusées) : forme du rejeu de refus ;
  - `tests/fixtures/` : rejeux qui ne sont pas des e2e.

## Out of Scope

- Les structures de planète, `oe`, `by`/`cc` en tant que listes, l'arène de prise d'icône et l'opération « vider une arène » : 4d. 4e les utilise.
- Les trous de ver `mz` (`mz:pq`, `ox`) : 4f.
- `ok.qa`, les boîtes par frame des singletons, la sonde `g_` et `bv` : 4h. 4e ne fait que garder `ra = mp` dans le point souris et le signaler.
- Un budget de slipways affiché et bloquant (§7).
- Relever `PICO8_ZONE_PARTICLES_BYTES` ou `PICO8_ZONE_GAME_BYTES`, sauf si la mesure l'exige.
- Retirer TROU-7 : étape 5.
- Lancer l'application raylib.

## Hypothèses sur l'étape 4d

4d est en cours : seul son morceau 1 est versé (24057b4). Chaque hypothèse dit ce que 4e en tire.

- **H-4d-1 (Reçu).** La clôture de 4d écrit sous l'entrée 4 la ligne « *Reçu de l'étape 4d* » pour 4e telle que recopiée plus haut (proposition `lw.t` hors génération, routes de `by`/`cc` seules objets de partie, TU `trade_rich_mine` et `game_end_final_score`). *Vérifié en partie à 24057b4* : `lp`/`rl` de `lw.t` sont en zone frame, `ib` en génération, `cc` vidée en place (`pico8_ents_clear`) ; la fenêtre du TU `trade_rich_mine` est f137-f151 (le fichier n'a que 152 frames). 4e déplace cette fenêtre à f139.
- **H-4d-2 (verrou).** Les morceaux 2 à 4 de 4d ne touchent ni `slipway`, ni `slipway_display`, ni `trade` ; le verrou de 4d finie est **16 / 47** ; les 11 relocations de 4e restent aux lignes de 24057b4. 4e prévoit alors **13 / 36**.
- **H-4d-3 (arène de prise).** 4d livre, comme sa PRD le décide : une opération de la zone générique qui vide une arène sur son bloc (empoisonné sous ASan) ; une liste de prises libres vidée par `oublier_partie()` (sa première entrée) et après la remise à zéro du chargement et de l'arrêt ; le recyclage au `rendre`, en dernier, sur « état kz + arène de prise ». 4e réutilise l'opération et les deux points de vidage pour sa liste de prises jc, et, si la forme s'y prête, le pool lui-même avec une autre taille.
- **H-4d-4 (planètes en génération).** Les planètes, leurs `p`/`ra`/identifiants et leurs `oe` sont en génération (ou dans l'arène d'une structure posée). `qu`/`re` d'un slipway, `lp` d'une route et les chaînes de `hg` désignent donc des objets de même durée de vie que le slipway et la route.
- **H-4d-5 (budget).** `M_4d` = 130 720 o (`pause_restart`), `K_4d` = 0, `trade_rich_mine` ≈ 75 488 o avec l'arène de prise, `r` = 1, décision « on continue ». La projection de 4e en part.
- **H-4d-6 (inventaire).** 4d ne coupe pas `jo.fo` au vidage : sa PRD tient `fo` pour « remis à nil par le `kj` de la même frame ». Si 4d l'a fait (trou trouvé par ASan ou à la relecture), 4e réutilise sa coupe et ne fait que vérifier qu'elle couvre un `jc`.
- **H-4d-7 (zone de construction).** La « zone de construction » de 4d n'est posée que par `ie:ldown` autour de `new_entity`. `jo:ldown` et `nr:ldown` ne la posent pas : un `jc` n'y tombe pas et prend sa propre arène.
- **H-4d-8 (TU).** Les égalités exactes de 4c complétées par `P` (4d) portent sur `pause_restart`, sans slipway ni route : 4e ne les change pas.

## À revérifier une fois 4d finie

À relire sur le `HEAD` de clôture de 4d, avant de lancer le morceau 2, et à corriger ici si besoin :

- la ligne « *Reçu de l'étape 4d* » réellement écrite, et toute autre ligne « Reçu » qui vise 4e (H-4d-1) ;
- `tools/alloc_guard_exceptions.txt` et `make alloc-guard` : 16 / 47, et `slipway` 5, `slipway_display` 1, `trade` 5 aux mêmes lignes (`objdump -r` + `addr2line`) (H-4d-2) ;
- la forme livrée de l'arène de prise : nom de l'opération « vider », de la liste des prises libres, de ses points de vidage ; possibilité d'en faire une seconde instance de taille 2 048 (H-4d-3) ;
- la fenêtre et l'assertion du TU `trade_rich_mine` de 4d, et le rejeu de prises refusées (fixture) que 4e prend pour modèle (H-4d-1) ;
- `M_4d`, `K_4d`, le maximum de `trade_rich_mine` avec arène, la décision écrite dans « Découvert (4d) » ; refaire la projection de 4e avec ces valeurs (H-4d-5) ;
- `oublier` et `oublier_partie()` : la coupe de `jo.fo` existe-t-elle ? sinon, la signaler comme trou de l'inventaire de 4d dans « Découvert (4e) » (H-4d-6) ;
- `ie:ldown` et la zone de construction : ne couvre pas `jo:ldown`/`nr:ldown` (H-4d-7) ;
- les égalités exactes de `test_pico8_places_vivantes.c` et `test_pico8_zone_partie.c` (H-4d-8) ;
- refaire le prototype de 4e sur ce `HEAD` (avec l'arène jc cette fois) et comparer : 26 `*.sha256`, `sanitize-net`, verrou 13 / 36, `trade_rich_mine` ≈ 93,8 Ko, `M_4e` = `M_4d`.

## Further Notes

- **Pas d'humain disponible, et pas d'outil `Agent` dans cette session pour lancer un agent `guideline` neuf.** J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage) :
  - **Hypothèses explicites** : tout ce qui dépend de 4d est en H-4d-1 à H-4d-8. Les fréquences ont été lues dans le cart (`jo:ldown` l. 412, `nr:ldown` l. 1112, `ui:bh` l. 225-236, `_update60` l. 1187, `d_` l. 569, `jc:pq`/`hq`/`ey`/`ce`/`ch`/`kj` l. 620-681, `trade:kg` l. 606, `jo:cn` l. 495, `jo:ib` l. 501-512, `nr:cn` l. 1116-1119, `jo:kj` l. 541-549, `bv` l. 551-558) et dans le code. Il en sort deux corrections de la table de 4a (`jc` est par pression ; les routes de `kg` sont par `bd` et presque toutes refusées) et un trou de lecture (`jo.fo` d'une planète non dessinée).
  - **Simplicité** :
    - une seule fonction nouvelle côté routes (la copie) et un argument de zone ;
    - le point souris en deux membres, sans allocation ni statique ;
    - une sorte de particules de plus, sans relèvement ;
    - l'arène jc réutilise le mécanisme de 4d, sans zone de construction ;
    - pas de valeur pour les routes (hors de proportion).
  - **Changements chirurgicaux** : 11 relocations dans trois fichiers ; en plus, la fermeture `kf` (un appel), `jo:ib`/`jo:cn`/`nr:cn` (l'argument de zone), `oublier` (la coupe de `fo`), `oublier_partie()` et le reset (un appel chacun), la zone particules (une sorte), le rejeu (une colonne de noms).
  - **Critères vérifiables** :
    - 18 / 55 à 24057b4, 13 / 36 après 4d ;
    - génération constante pendant un slipway tenu et sur vingt refus ;
    - +1 arène jc exactement sur `trade_rich_mine` f139-f151 ;
    - places `ts` vivantes = Σ des listes, puis 0 après rechargement ;
    - `fo` nil après le vidage ;
    - 26 `*.sha256` et `sanitize-net` ;
    - `M_4e` = `M_4d`, `P_4e` ≤ 704 Ko.
- **Le prototype.** Fait dans une copie `git archive` de 24057b4, hors dépôt. Il met `jc` et `ts` directement en génération (sans arène : l'opération « vider » de 4d n'existe pas encore), le point souris en membres, la sorte `ts` (680 places), un `rendre` de `jc` qui rend les traînées, `trade_new`/`kg` en zone frame et la copie en génération dans `kf`. Il ne fait pas la coupe de `fo`.
  - Résultats : `alloc-guard` 18 / 55 ; 26 `*.sha256` identiques ; `sanitize-net` « 26 propre(s) » ; `make test` vert sauf `test_hq_mouse_point` et la fenêtre f137-f151 de `trade_rich_mine` ; les mesures de « Budget ».
  - Il n'est pas versé : c'est au cycle `tdd` de 4e de refaire le code, tests d'abord.
- **`hg_own` pèse 728 des 1 112 o d'un `jc`** (90 identifiants). Le réduire serait une économie PSX (§7 du plan), pas un sujet de 4e.
- **4f** (trous de ver) a « les mêmes motifs que 4e » (4a) : `mz` accroche aussi `fo` sur un slipway (`pico8_probe_et_hang_fo`), que `jc:kj` remet à nil sans condition ; la coupe de `jo.fo` dans `oublier` la couvre déjà côté planètes.
