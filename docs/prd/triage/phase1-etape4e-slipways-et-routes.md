---
title: "Plan 4, étape 4e — Slipways dans l'arène de prise de 4d, routes gardées recopiées en génération, traînées ts en zone particules"
description: "Le slipway jc, sa liste ts, son point souris, ses traînées et les routes trade sont encore pris à la libc ou à la transitoire. 4e construit chaque slipway dans l'arène de prise de 4d (portée à 2048 o et partagée), recyclée quand il est refusé ; elle réécrit le point souris en place, fait toutes les routes candidates de bd en zone frame et ne recopie en génération que celles que by garde, et range les traînées ts dans une nouvelle sorte de la zone particules, rendue à leur mort et au rendu du slipway."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. L'étape 4 range les objets de partie dans la **zone partie**. Cette PRD traite la sous-étape **4e, slipways et routes**.

Elle a été rédigée pendant 4d, avec des hypothèses H-4d-1 à H-4d-8. **4d est finie** : morceau 1 24057b4, morceau 2 54c2624, morceau 3 cbe29df, clôture 4567120. Toutes les hypothèses ont été revérifiées à `HEAD` 4567120 (section « Vérifié contre 4d »). Les sections qui en dépendaient sont réécrites sur les faits, et le prototype a été refait sur 4567120.

**Le texte de l'étape**, recopié du plan à 4567120 (extraits qui concernent 4e) :

> - **4b et suivantes.** Une famille de propriétaires par PRD dans l'ordre fixé par la PRD 4a : 4b états de `pico8_gen.c` ; 4c tableaux croissants et registres (crée la sous-zone chargement) ; 4d planètes et relais ; 4e slipways et routes ; 4f trous de ver ; 4g recherche et économie ; 4h singletons de `jl`/`eu` et objets du joueur (famille ajoutée par 4a : les « Reçu » des étapes 1 à 3 leur reviennent). N'entre en zone partie qu'une allocation faite une fois par objet de partie ; un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
> - *Découvert (4a) :* […] Les 81 relocations de l'étape 4 sont attribuées aux familles 4b–4h ; la seule marge est de 49 Ko sur la somme des zones, la zone particules gardant ses ~17 Ko libres pour les traînées `ts`. Le rejeu écrit `partie` et `max_octets partie`.
> - *Reçu de l'étape 4a :* les traînées `ts` de `jc:ch` (par frame) vont à 4e, en zone particules, places de 24 o.
> - *Découvert (4d) :* planètes, `oe`, `p`/`ra`/« p… », listes de `jo:lz` et `jo:cn` en génération ; `cc` vidée sur place à chaque `bd` (`pico8_ents_clear`), `ib` une fois par planète (pas par `bd`, contrairement à la table de 4a), `lp`/`rl` des propositions en zone frame. Une prise d'icône refusée faisait grandir la génération (+616 o) : elle prend une arène de 1536 o recyclée par une liste de 4 prises libres. Trou de l'inventaire du §4 : le `fo` d'une planète hors écran pouvait viser l'objet rendu ; `oublier` le coupe (TROU-26). Mesure : 90 planètes × 536 o = 48 240 o par partie, maximum 130 720 o au clic de `pause_restart` (prévision exacte), 80 464 sur `game_end_bankrupt` ; projection ≈ 177 Ko ≤ 704, chargement 0 : on continue. Comptes : 22 / 75 → 16 / 47.
> - *Reçu de l'étape 4d :* pour 4e, la route `lw.t` proposée à chaque `bd` ne doit pas entrer en génération ; l'arène de prise et sa liste de prises libres sont réutilisables pour le slipway tiré. Pour 4g, l'identifiant « s… » de `ie:ldown` est fait à chaque prise : il va dans l'arène de prise.
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

**La méthode de suivi du budget** est celle de la PRD 4a, reprise par 4c et 4d. Depuis 4c, il y a deux comptes : génération ≤ 704 Ko, chargement ≤ 64 Ko. La projection est `P_k = M_k + Σ estimations restantes × r`, avec `r = max(1, mesuré / estimé)`. La marge est de 49 Ko sur la somme des zones, et un relèvement de la zone particules pour `ts` se prend aussi sur elle.

**Ce qui se passe aujourd'hui** (lu à 4567120) :

- **Le slipway.** `jc` naît d'une **pression** du bouton gauche sur une planète « ly » (`jo:ldown`, cart l. 412, `pico8_jo_ldown`) ou sur un relais (`nr:ldown`, l. 1112). Chaque pression en fait un, par `malloc` (`pico8_jc_struct_new`, 344), avec sa liste `ts` (`malloc` + transitoire, `jc:pq`, 89-90).
  - `pico8_jc_t` fait **960 o** sur x86-64, dont 728 pour `hg_own` (90 identifiants au plus).
  - `ui:bh` le tient (`ok.lheld`). Au relâcher, `jc:ey` le **pose** (`d_` → `ks("ph")`, `qu:kf`, `re:kf`, `fm:nx`) ou le **refuse** (`re` nil, ou `d_` qui échoue sur `et` ou sur le coût) : `p_`, et l'état reste « kz ». Un simple clic sur une planète colonisée fait donc un `jc` refusé.
  - Le `jc` n'a pas de `rendre` : un refusé sort de `hv` par `dg`/`he`, **n'entre pas dans la file de sortie**, et fuit (TROU-7). Il ne passe donc jamais par `oublier`, ni par la coupe de `fo` de 4d (`pico8_jo_oublier_fo`).
- **Le point souris.** Tant que le slipway est tenu au-dessus du vide, `jc:hq` (l. 622) fait **à chaque frame** un `{ra=mp,p=mp/128}`. C'est une structure `pico8_jc_mouse_point_t` (144 o) et sa boîte `p`, par `malloc` (111-112), jamais rendues. `ra` est `mp`, c'est-à-dire `ok.qa` lui-même.
- **Les traînées.** Dans le `kj` d'un slipway visible, `jc:ch` (l. 657) tire `rnd()<0.08` pour chaque sens. Si `hg[t.id]`, il ajoute un `pico8_ts_t` de 20 o (`malloc`, `slipway_display` 36). `kj` le fait avancer, décrémente `l`, et le retire par `del(self.ts,t)` quand `l<=0`, sans le rendre.
- **Les routes.** `trade:kg` (l. 606) fait une route neuve : une structure (`malloc`, `trade` 29), avec `lp` et `rl` concaténées (`malloc` + transitoire, 135-138). Ses appelants, **à chaque `bd`** :
  - `jo:ib` fait la proposition `lw.t` par `pico8_trade_new` : structure par `malloc`, `lp`/`rl` en zone frame (4d) ;
  - `jo:cn` fait `t:kg(self)` et le passe à `lw:kf`, la fermeture de `jo:ib`, qui ne l'ajoute à `ib` (= `by`, en génération depuis 4d) que si `gu(ib,a)` est faux (égalité de route, `trade.ns.__eq`) ;
  - `nr:cn` fait `t:kg(self,w)` pour chaque slipway du relais et le passe, dans une copie `gf` de la proposition, au `cn` de l'autre bout.

  `by` n'est jamais vidée : une route connue est **reproposée et refusée par `gu` à chaque `bd`**. Le prototype compte, sur `trade_rich_mine`, **32 `kg` pour 7 routes gardées**.
- **L'arène de prise de 4d** (`pico8_planet.{h,c}`), l'outil que 4e réutilise :
  - `PICO8_JO_PRISE_OCTETS` vaut 1536 ;
  - `pico8_jo_prise_ouvrir()` reprend une arène de la liste des prises libres (4 au plus) et la vide (`pico8_zone_arene_vider`), ou en prend une neuve à la génération ;
  - `pico8_jo_zone_construction()` rend cette arène tant qu'elle est ouverte, sinon la génération ;
  - `pico8_jo_prise_fermer()` la referme ;
  - au `rendre` d'une planète, `pico8_jo_prise_rendue()` remet l'arène dans la liste si l'arène de son `oe` porte le nom « prise » et si l'état est encore « kz » ;
  - `pico8_jo_oublier_prises()` vide la liste : au point d'oubli de partie, au chargement, à l'arrêt.
- **Le verrou** à 4567120 : **16 fichiers, 47 appels**. Les 11 relocations de 4e sont aux mêmes lignes qu'à e88df7a, car aucun commit de 4d n'a touché ces fichiers : `slipway` 5 (89, 90, 111, 112, 344), `slipway_display` 1 (36), `trade` 5 (29, 135, 136, 137, 138).

**Le problème.** Trois fréquences se mêlent dans la famille, et une seule a sa place telle quelle en génération :

1. **Une fois par objet de partie** : le slipway posé (et sa liste `ts`), la route gardée dans `by`.
2. **Par frame** : le point souris d'un slipway tenu, les traînées.
3. **Par action ou par `bd`** :
   - **le slipway refusé** : chaque clic ou glissé raté sur une planète fait un `jc`. Mis tel quel en génération, il la fait grandir de **1 648 o par refus** (mesuré sur 20 cycles à 24057b4) ;
   - **les routes candidates** : `lw.t`, et chaque `kg` de `jo:cn` et de `nr:cn`. Mises telles quelles en génération, les routes reproposées à chaque `bd` la feraient grandir à chaque action, comme `cc` avant 4d.

## Solution

- **Le slipway dans l'arène de prise de 4d, partagée et portée à 2 048 o.**
  - Chaque `jc` est construit, avec sa liste `ts` (capacité réservée de **64**), dans une arène de prise ouverte par `pico8_jo_prise_ouvrir()` et prise par `pico8_jo_zone_construction()`. C'est le même mécanisme et la même liste de prises libres que la prise d'icône, comme le propose le *Reçu de l'étape 4d*.
  - `PICO8_JO_PRISE_OCTETS` passe de **1536 à 2048**, parce que le lot d'un `jc` fait 1 744 o.
  - Posé, le slipway garde son arène jusqu'au rendu de la génération. Refusé, son `rendre` (nouveau) la remet dans les prises libres.
- **Point souris réécrit en place.** Le `{ra,p}` de `jc:hq` devient un membre du `jc` (le point, et sa boîte `p` en valeur), réécrit à chaque appel. Plus aucune allocation par frame tenue.
- **Routes candidates en zone frame, routes gardées recopiées en génération.**
  - `pico8_trade_new` et `trade:kg` prennent la zone de l'appelant. `jo:ib` (`lw.t`), `jo:cn` et `nr:cn` passent la zone frame.
  - La fermeture `kf` de `jo:ib`, au moment où `gu` accepte une route, en fait une **copie profonde en génération** (structure, `lp`, `rl`) et ajoute la copie à `ib`.
  - Une route gardée l'est une fois ; les reproposées meurent avec la frame.
- **Traînées en zone particules.**
  - Une sorte nouvelle, `ts` : **680 places** nues de 24 o, dans les ~17 Ko libres des 128 Ko. La zone particules passe de 113 822 à 130 822 o, sans relèvement, et la somme des zones reste 1487 Ko sur 1536.
  - `jc:ch` prend une place. `kj` la rend au `del`. Le `rendre` du slipway rend celles qui vivent encore.
- **La coupe de `fo` n'est plus à faire** : 4d l'a versée (`pico8_jo_oublier_fo`, TROU-26), pour toute entité de `cg.np`. C'est le `rendre` neuf du `jc` qui fait entrer un slipway dans la file de sortie, donc dans `oublier`, et donc sous cette coupe. Les autres champs qui peuvent tenir un `jc` sont vérifiés dans l'inventaire.
- **Inventaire du §4** refait pour les slipways, les routes, les traînées et l'arène partagée, relevé « éclats `ga` » compris.
- **Budget** : `M_4e` = `M_4d` = **130 720 o** (`pause_restart`, sans slipway), mesuré par le prototype sur 4567120. Projection **≈ 147 Ko** sous 704 Ko : **on continue**.
- **Verrou** : `slipway`, `slipway_display` et `trade` sortent. **16 / 47 → 13 / 36**, mesuré par le prototype.

Aucun pixel ne change, aucun tirage `rnd` n'est déplacé : le prototype à 4567120 garde les 26 `*.sha256`, et `make sanitize-net` répond « 26 propre(s) ».

## User Stories

1. En tant que mainteneur du plan, je veux que le slipway, sa liste `ts`, son point souris, ses traînées et les routes quittent la libc et la transitoire, pour que `slipway`, `slipway_display` et `trade` sortent du verrou.
2. En tant que développeur, je veux que chaque `jc` soit construit dans l'arène de prise de 4d, avec sa liste `ts` réservée dedans, pour qu'un slipway et tout ce qu'il possède vivent et meurent d'un bloc, sans second mécanisme de recyclage.
3. En tant que développeur, je veux que `PICO8_JO_PRISE_OCTETS` passe à 2048, parce que le lot d'un `jc` fait 1 744 o, pour qu'une seule taille d'arène serve les icônes et les slipways et qu'une arène libre serve l'un ou l'autre.
4. En tant que développeur, je veux qu'un slipway posé garde son arène jusqu'au rendu de sa génération, pour que les planètes (`oe`), les routes (`rl`), les registres et les sondes qui le désignent lisent toujours un objet vivant.
5. En tant que développeur, je veux qu'un slipway refusé remette son arène dans les prises libres à son `rendre`, et pas avant, pour qu'aucun `dg`, `he`, `oublier` ni `kj` ne lise une arène déjà réutilisée.
6. En tant que développeur, je veux que la reconnaissance d'une arène de prise et son retour dans la liste soient une fonction du module planète qui prend la zone et l'état, appelée par le `rendre` de planète comme par celui du slipway, pour ne pas lire la liste de prises libres depuis un autre module.
7. En tant que développeur, je veux que `jc` ait un `rendre`, pour qu'il passe par la file de sortie, donc par `oublier` et la coupe de `fo` de 4d, et que ses traînées vivantes reviennent à la zone particules en fin de partie comme au refus.
8. En tant que développeur, je veux que le point souris de `jc:hq` soit un membre du slipway réécrit à chaque appel, parce que seul `self.re` le désigne, pour qu'un glissé tenu ne fasse plus aucune allocation par frame.
9. En tant que développeur, je veux que `pico8_trade_new` et `trade:kg` prennent la zone de l'appelant, pour que la règle de 0b (une zone par propriétaire ou par appel) s'applique aux routes.
10. En tant que développeur, je veux que la proposition `lw.t` de `jo:ib` et toutes les routes candidates de `jo:cn` et `nr:cn` soient faites en zone frame, parce qu'elles ne sont désignées que par des variables locales de `bd`, pour qu'un `bd` qui repropose des routes connues ne fasse plus grandir la génération.
11. En tant que développeur, je veux que la fermeture `kf` de `jo:ib` recopie en génération la route qu'elle garde, structure et listes comprises, pour qu'une route gardée dans `by` et `cc` survive à la remise à zéro de la frame.
12. En tant que développeur, je veux une fonction de `pico8_trade` qui recopie une route dans une zone donnée, pour que la copie profonde ne soit écrite qu'une fois et ne pique pas dans les champs depuis un autre module.
13. En tant que développeur, je veux une sorte `ts` de la zone particules, à places nues de 24 o, pour que les traînées soient prises et rendues une à une comme les particules de `qy`.
14. En tant que développeur, je veux que `kj` rende la place d'une traînée au moment où il la retire de `ts`, en gardant le dessin final de la traînée morte (pas de `else` dans le cart), pour que les traînées ne fuient plus.
15. En tant que développeur, je veux que la liste `ts` réserve 64 éléments dans l'arène du slipway, parce que la lecture borne une moyenne de ~21 traînées vivantes par slipway, pour qu'elle ne grandisse jamais en jeu.
16. En tant que mainteneur, je veux que la génération ne bouge pas pendant les frames où un slipway est tenu au-dessus du vide, pour prouver que `jc:hq` n'alloue plus par frame.
17. En tant que mainteneur, je veux un rejeu qui enchaîne vingt pressions refusées sur une planète et dont la génération ne bouge plus après la première, pour prouver le recyclage de l'arène.
18. En tant que mainteneur, je veux qu'un slipway posé après des refus prenne exactement une arène de plus, pour prouver qu'un slipway posé n'est jamais recyclé.
19. En tant que mainteneur, je veux qu'une pression refusée sur une planète, puis une nouvelle partie, puis une pression ne fassent pas planter ASan, pour prouver que la liste partagée est vidée au point d'oubli aussi pour les slipways.
20. En tant que mainteneur, je veux que sur `trade_rich_mine` la génération grandisse d'exactement une arène entre f139 et f151, pour prouver deux choses à la fois : que `bd` ne garde pas les routes reproposées au saut d'année de f140, et que le second slipway refusé (clic de f148) reprend l'arène du premier (clic de f142).
21. En tant que mainteneur, je veux qu'une route gardée à f139 de `trade_rich_mine` se relise intacte sous ASan après douze remises à zéro de la frame, pour prouver que la copie est en génération et pas en zone frame.
22. En tant que mainteneur, je veux qu'à chaque frame de `trade_rich_mine` les places `ts` vivantes égalent la somme des `ts->count` des slipways de `cg.jc`, pour prouver qu'aucune traînée n'est perdue ni rendue deux fois.
23. En tant que mainteneur, je veux qu'après `trade_rich_mine` puis un rechargement les places `ts` vivantes reviennent à 0, pour prouver que le `rendre` du slipway rend ses traînées.
24. En tant que mainteneur, je veux qu'un slipway refusé, dont la cible `et` est une planète hors écran, laisse cette planète avec un `fo` nil après son rendu, pour prouver que la coupe de 4d s'applique bien aux slipways depuis qu'ils ont un `rendre`.
25. En tant que mainteneur, je veux que le rejeu écrive `ts`, `max_vivantes ts` et `max_elements ts`, pour relever le pire des traînées sans écrire de test.
26. En tant que gardien du budget, je veux que la sorte `ts` tienne dans les 128 Ko de la zone particules par le contrôle à la compilation existant, sans relèvement, pour ne rien prendre aux 49 Ko de marge.
27. En tant que gardien du cliquet, je veux que `slipway`, `slipway_display` et `trade` sortent de la liste sans qu'aucun compte ne monte, pour que le verrou reste à égalité stricte.
28. En tant que gardien de l'invariant, je veux les 26 `*.sha256` inchangés et `make sanitize-net` vert, pour prouver que l'arène, la copie des routes, le point en place et les traînées en places ne changent rien au jeu.
29. En tant que mainteneur du budget, je veux `M_4e` relevé sur les 26 e2e et comparé à `M_4d`, avec le coût de l'arène portée à 2048 sur les prises d'icône, la projection et la décision écrites, pour appliquer l'arrêt anticipé du §3.
30. En tant que développeur des familles suivantes, je veux l'inventaire du §4 refait pour les slipways, les routes, les traînées et l'arène partagée, relevé « éclats `ga` » compris, pour savoir ce que le point d'oubli et `oublier` coupent désormais.
31. En tant que développeur de 4h, je veux une ligne « *Reçu de l'étape 4e* » qui dise que le point souris d'un slipway tenu garde `ok.qa` (`ra = mp`), pour que 4h, qui choisit la zone de `ok.qa`, sache qui le relit.
32. En tant que lecteur du plan, je veux que le §2 dise que seules les routes gardées dans `by` sont en zone partie et que les routes candidates de `bd` sont en zone frame, pour que le tableau colle au code.
33. En tant que mainteneur des commentaires, je veux que les seuls commentaires touchés soient les contrats C devenus faux, les contrats des fonctions nouvelles et les `assert` porteurs de message, pour respecter les quatre destinations du `CLAUDE.md`. Les contrats devenus faux sont : « Nothing frees the struct: TROU-7 » de `pico8_jc_t`, « a fresh one per call, never freed » du point souris, « which owns it from then on » de `jc:ch`, le `/* TROU-7 */` de `kg`, « read, not kept » des clés de route, et « Bytes of the arena a held icon builds its structure in » de `PICO8_JO_PRISE_OCTETS`.

## Implementation Decisions

### Inventaire des sites de 4e, et zone retenue

Chaque site par frame, par action ou par `bd` est tranché par lecture : « qui d'autre garde l'ancienne valeur ? ».

| Site | Ligne à 4567120 | Fréquence (lecture) | Qui garde l'objet | Zone retenue |
|---|---|---|---|---|
| structure `jc` | `slipway` 344 | **une par pression** sur une planète « ly » ou un relais (`jo:ldown`, `nr:ldown`) ; posée ou refusée par `jc:ey` | posé : `oe` des deux bouts, `rl` des routes gardées, registres, `et` d'un autre slipway tenu, cible `et` d'une sonde (`bv`) ; refusé : plus rien après son `rendre` (voir « Arène de prise partagée ») | **arène de prise** de 4d, gardée si posé, recyclée si refusé |
| liste `ts` et son conteneur | `slipway` 89, 90 | une par `jc` | le `jc` seul | **dans l'arène**, capacité réservée 64 |
| point souris et sa boîte `p` | `slipway` 111, 112 | **par frame** où le slipway est tenu hors d'une planète | `self.re` seulement (voir plus bas) | **réécrit en place** : membres du `jc` |
| traînée `pico8_ts_t` | `slipway_display` 36 | **par frame** où le slipway est visible, tirage 0,08 par sens | la liste `ts` seule | **place de la sorte `ts`**, zone particules, rendue au `del` et au `rendre` du `jc` |
| structure `trade` de `lw.t` | `trade` 29, appelée par `planet_trade` 224 | par `bd`, par planète, par slipway de `oe` | la proposition, variable locale de `jo:ib` | **zone frame** |
| route de `kg` (structure, `lp`, `rl`) | `trade` 29, 135-138 | par `bd`, par chemin parcouru par `jo:cn` / `nr:cn` | candidate : `gf.t` (locale de `nr:cn`), l'argument de `kf` ; seule `kf` la garde, et seulement si `gu` est faux | **zone frame** ; la copie gardée par `kf` en **génération** |

- **Pourquoi pas « valeur » pour les routes.** `by` et `cc` sont des `pico8_ents_t` d'en-têtes, qui désignent les routes par pointeur. Les rendre valeurs toucherait `by`, `cc`, `dh`, `db` et les panneaux, ce qui est hors de proportion.
- **Pourquoi la zone frame pour les candidates.**
  - `bd` tourne dans un `hy:lj`, donc dans un update, entre deux remises de la zone frame.
  - `lw` et `gf` sont des locales sur la pile, ce que disent déjà les commentaires existants.
  - La seule fonction qui range une route est la fermeture `kf` de `jo:ib` : le `grep` de `->kf(` et de `lw.kf =` donne un seul appel et une seule affectation.
  - Toutes les autres lectures (`gu_trade`, `nr:cn` sur `t->rl`, `jo:cn` sur `t->qe`) ont lieu dans le même `bd`.
- **Pourquoi « copier à la garde » et pas « faire `kg` dans la zone de l'appelant ».** `jo:cn` ne sait pas, quand il fait `kg`, si `gu` va accepter, et la plupart des routes qu'il fait sont reproposées puis refusées. Faire `kg` en génération ferait grandir la génération à chaque `bd`. La copie a lieu une fois par route gardée.
- **Pourquoi la copie est exacte.**
  - Le cart fait `add(ib,a)` sur l'objet `a` lui-même. Une copie ne s'en distingue que si quelqu'un garde `a` et le compare par adresse, ou le modifie après `kf`. Or après `lw:kf(t:kg(self))`, `jo:cn` fait `return`, et `nr:cn` ne relit que `gf.mu`.
  - Rien ne modifie une route après sa construction : `lp`/`rl` sont faites par `la` et jamais écrites ensuite.
  - `cc` désigne les routes de `by` (par `bd_route_visit`), donc les copies.
  - Le prototype garde les 26 `*.sha256`.
- **Pourquoi réécrire le point souris en place.**
  - Le cart fait un `{ra=mp,p=mp/128}` neuf à chaque `hq` et le range dans `self.re`. Qui garde l'ancien ? Le `grep` des lecteurs de `re` d'un `jc` donne :
    - `jc:kj`/`jc:lg` (dessin), `jc_end_p`/`jc_end_ra` ;
    - `jc:ce`, qui compare `eb(w,qu)==re`, jamais vrai pour un point ;
    - `jc:ey`, qui remplace `re` ou garde le point d'un refusé ;
    - `oublier`, qui compare `lheld->re == e` sans déréférencer.
  - Aucun ne garde le point ailleurs que dans `self.re`. Un membre du `jc` réécrit à chaque appel a donc la même valeur à chaque lecture. Coût : 144 + 8 o par `jc`, qui passe de 960 à 1 112 o.
  - **Écarté, une statique unique** : un `jc` refusé (frame N) et un nouveau `jc` tenu (frame N+1) la partageraient, et c'est un pointeur de plus à l'inventaire.
  - **Écartée, la zone frame** : elle est sûre par lecture, mais c'est une durée de vie plus courte que celle de l'objet qui la désigne, ce que le §4 n'autorise que prouvé. Le membre ne demande aucune preuve.

### Arène de prise partagée : recycler le slipway refusé

**Le choix : réutiliser le mécanisme de 4d, avec une seule liste et une seule taille.** Trois formes ont été pesées :

- **Liste et taille partagées, arène portée à 2048** (retenue).
  - `jc` et prise d'icône ouvrent la même arène et rendent à la même liste.
  - Une arène libérée par un `jc` refusé sert la prochaine icône, et inversement : les prises refusées en vol restent au plus deux (un seul `ok.lheld` à la fois), sous la capacité 4 de la liste.
  - Coût : 512 o de plus par structure d'icône posée. Le prototype mesure +3 584 o sur `game_end_bankrupt` (7 prises) et rien sur `pause_restart`.
  - Les TU de 4d écrivent leurs attentes avec `PICO8_JO_PRISE_OCTETS`, et restent verts à 2048 dans le prototype : 50 896 o constants sur vingt prises refusées, et une prise posée qui coûte « arène 2048 + by/cc 48 ».
- **Écarté, faire tenir le `jc` dans 1536 o.** Il resterait 1536 − 96 − 1112 − 24 = 304 o, soit 38 éléments réservés pour `ts`. C'est à 3,8 écarts-types de la moyenne de ~21, donc atteint en partie longue.
- **Écarté, une seconde liste à 2048 propre au slipway.** C'est le même code en double, avec deux appels de plus aux points d'oubli, pour économiser 512 o par icône posée.

**Décision.**

- **Taille** : `PICO8_JO_PRISE_OCTETS` 1536 → **2048**. Le lot d'un `jc` fait 96 (en-tête d'arène) + 1 112 (`pico8_jc_t`) + 24 (conteneur `ts`) + 64 × 8 (éléments réservés) = **1 744 o**. Le plus gros lot d'icône mesuré par 4d fait 744 o.
  - Une liste `ts` qui dépasserait 64 grandirait dans l'arène, copie comprise, et la remplirait. `pico8_fatal` « zone prise pleine » se lit alors comme un slipway à plus de 64 traînées vivantes, et se relève d'un chiffre.
  - Le contrat de `PICO8_JO_PRISE_OCTETS` devient « arène où une icône tenue ou un slipway tiré construit son objet ».
- **Construction** : `pico8_jc_struct_new` appelle `pico8_jo_prise_ouvrir()`, alloue la structure et la liste `ts` (`jc:pq`) par `pico8_jo_zone_construction()`, puis appelle `pico8_jo_prise_fermer()` avant `pico8_ke_struct_pq` (les registres ont leur propre poignée).
  - Tous les `jc` du jeu naissent d'une pression, dans `jo:ldown` ou `nr:ldown`. Ces deux appels ne tombent jamais pendant une prise d'icône ouverte : celle-ci n'est ouverte qu'autour du `new_entity` d'`ie:ldown`, et l'`assert` « une prise d'icone est deja ouverte » le garde.
  - Un `jc` fait par un TU prend aussi une arène.
- **Reconnaissance et retour** : la fonction de `pico8_jo_prise_rendue` qui reconnaît l'arène et la remet dans la liste est séparée en une fonction publique du module planète, qui prend une **zone** et un **état** (nom laissé à l'implémenteur).
  - Le `rendre` de planète l'appelle avec l'arène de son `oe`. Celui du `jc` l'appelle avec l'arène de sa liste `ts` (`ts->zone`), **en dernier**, après ses traînées.
  - Le test est inchangé : l'arène porte le nom « prise » et l'état est encore « kz ». Un `jc` posé est passé « ph » par `d_` → `ks("ph")` (`jc` n'a pas de `dq`).
- **Oubli** : rien à ajouter. `pico8_jo_oublier_prises()` vide déjà la liste partagée au point d'oubli de partie, au chargement et à l'arrêt.
- **Borne** : un `jc` refusé à la frame N (dans `ok:lj`, après `dg`) est retiré par le `dg` de N+1 et rendu au vidage de N+2. Une pression ne peut tomber qu'au plus tôt en N+1 : si `b.jn` et `b.hm` tombent dans la même frame, c'est le même `jc` qui naît et qui est relâché.
- **Angle mort ASan**, comme en 4d : la pression suivante réalloue aussitôt les mêmes adresses. Un pointeur oublié vers un `jc` refusé lirait le nouveau sans planter. C'est l'inventaire ci-dessous, pas ASan, qui garantit qu'il n'y en a pas.

### Traînées : la sorte `ts` de la zone particules

- **La sorte.** `PICO8_PARTICULES_TS` : places nues (pas d'arène) de 24 o, nom « particules ts ». `pico8_ts_t` fait 20 o, soit trois cellules de 8 o.
  - `pico8_jc_ch` prend une place par la poignée de la sorte.
  - `jc:kj` la rend juste après `pico8_ptrs_del`, avant le dessin, qui ne lit plus que des copies locales (`new_ra`).
  - Le `rendre` du `jc` rend les places encore dans `ts`.
  - `pico8_zone_particules_noter_elements` accepte `ts`, et le rejeu écrit `ts`, `max_vivantes ts` et `max_elements ts`.
- **Combien de traînées vivent à la fois — lecture du pire.**
  - Une traînée naît avec `l = |d|` en pixels (`d = t.ra - f.ra`, `v` unitaire) et perd 1 à chaque dessin.
  - `jc:ce` refuse un slipway dont `#(sp-dp)` dépasse `ov("space folding",0.5,1.08)` ; c'est le carré de la longueur, en unités `p` = `ra/128`. Donc `|d| ≤ 128 × √1,08 ≈ 133` pixels (≈ 90,5 sans « space folding »), et une traînée vit **au plus 134 dessins**.
  - Il naît au plus une traînée par sens et par dessin, et seulement vers une planète de `hg`. Cela donne **268 vivantes au pire absolu** par slipway, et **≈ 21,3 en moyenne** (2 × 0,08 × 133), avec un écart-type ≈ 4,4.
  - Hors écran, un slipway n'est ni dessiné ni vieilli : ses traînées gèlent, comme les éclats `ga` de l'étape 3.
  - Un `jc` tenu n'a jamais de traînée : `hg_own` est vide après `jc:pq`, et aucun `bd` ne tombe pendant la prise (4d).
- **Liste par slipway : 64 éléments réservés**, à 9,7 écarts-types de la moyenne. Le pire absolu (268) n'est pas atteignable en pratique ; le dépasser sort par `pico8_fatal` (« zone prise pleine »).
- **Nombre de places : 680.**
  - La zone particules occupe 113 822 o sur 131 072. Une place de 24 o coûte 25 o dans le contrôle (`SORTE_BYTES`, octet de prise compris), donc 690 places la rempliraient exactement.
  - 680 places la portent à **130 822 o**. Le contrôle à la compilation tient **sans relèvement** : la somme des zones reste 1487 Ko, et la marge de 49 Ko reste entière.
- **Risque accepté**, comme pour `qy` 24 et `ga` 8 à l'étape 3.
  - À ~21 traînées par slipway desservi dans les deux sens, 680 places se dépassent vers **une trentaine de tels slipways visibles à la fois**. Le relevé des e2e est loin dessous : 13 au plus, sur `trade_rich_mine`.
  - Un `pico8_fatal` « zone particules ts pleine » se lit d'abord comme une traînée jamais rendue, ce que tranche l'invariant « places vivantes = Σ des listes ».
  - Si l'invariant tient, c'est une limite trop basse : on la relève sur les 49 Ko (règle de 4a), et la projection en tient compte.

### Le `rendre` du slipway

- `jc` gagne `rendre`. Il entre donc dans la file de sortie quand `he` (refus) ou le `jl()` d'`eu`/`_init` (fin de partie) le retirent de `hv`. Au vidage, il passe d'abord par `oublier`, qui comprend `pico8_jo_oublier_fo` (4d), puis par son `rendre`.
- Ordre du `rendre` :
  1. rendre chaque place de `ts` (à partir du morceau 3 ; avant, les traînées fuient encore, TROU-7) ;
  2. remettre `ts` à vide ;
  3. appeler le retour d'arène de prise du module planète, **en dernier**.
- **File de sortie.** Les slipways s'ajoutent aux planètes et aux autres entités à `rendre` que le `jl()` d'`eu` met en file d'un coup (`PICO8_FILE_SORTIE_CAP` 256).
  - Aucun e2e ne recommence une partie qui a des slipways ; la borne se lit « 90 planètes + slipways + structures posées + interface ≤ 256 ».
  - Un `pico8_fatal` « file pleine » se relève d'un chiffre : la file est un tableau statique de pointeurs, hors des zones.

### La coupe de `fo` (versée par 4d), vérifiée pour les slipways

4d a versé `pico8_jo_oublier_fo(e)` (TROU-26). Appelée par `oublier` pour chaque entité rendue, elle remet à nil le `fo` de toute entité de `cg.np` qui désigne `e`. **4e ne la change pas.** Reste à vérifier que tous les champs qui peuvent tenir un `jc` sont couverts :

| Champ qui peut tenir un `jc` | Qui l'écrit | Couvert par |
|---|---|---|
| `fo` d'une planète de `cg.np` | `jc:hq` via `pico8_probe_et_hang_fo` quand `et` est une planète | `pico8_jo_oublier_fo`, **à condition que le `jc` passe par la file** : c'est le `rendre` neuf de 4e qui l'y fait entrer |
| `fo` d'un autre `jc` (posé) | `jc:hq` quand `et` est un slipway croisé ou déjà posé entre les mêmes bouts | `jc:kj` le remet à nil **sans condition**, hors du test `gd` (`slipway_display` 171), dans le dessin de la frame même ; le `jc` refusé n'est recyclé que deux updates plus tard |
| `g_jc_too_far.fo` (`slipway`) | `jc:hq` quand `et` est le « too far » | **écrit seulement, jamais lu** (4a) |
| `ok.lheld` | `ui:bh` | remis à nil par `ui:bh` après `ey` |
| `et` d'un autre `jc` tenu, d'une sonde (`bv`) | `jc:ce`, `bv` (cherchent dans `cg.jc`) | un refusé sort de `cg.jc` au `dg` de N+1, avant toute autre prise ; un posé vit la génération |
| `oe` des planètes, `rl` des routes gardées | `jo:kf` au succès de `jc:ey`, `kg` | ne visent jamais un refusé (`kf` seulement au succès ; aucun `bd` pendant une prise) |

`jc:ce` ne rend qu'une planète de `cg.np`, un slipway (`qu.oe`, `cg.jc`) ou le « too far » statique. `pico8_probe_et_hang_fo` n'écrit `fo` que sur une planète ou un `jc`. Aucun autre champ ne reçoit un `jc`.

### Choix de la zone par le propriétaire

- `pico8_trade_new` et `trade:kg` prennent la zone en premier argument, sur le modèle de `pico8_jo_pq_dans`, `pico8_jf` et `pico8_gn`.
  - Leurs appelants du jeu passent `pico8_zone_frame()`.
  - Les TU passent la zone qu'ils veulent (la génération si la route doit survivre à une frame) : 33 appels dans sept fichiers de `tests/`.
- Une fonction nouvelle de `pico8_trade` (nom laissé à l'implémenteur) recopie une route dans une zone : structure, `lp` et `rl` neuves (`pico8_ents_cat` avec une liste vide), `qe` recopié. Contrat à écrire : la copie ne partage rien avec l'original.
- La fermeture `kf` de `jo:ib` l'appelle avec `pico8_zone_partie()`, juste avant `pico8_ents_add(ib, …)`.
- `pico8_jc_struct_new` ouvre et ferme la prise elle-même (voir « Arène de prise partagée »).
- Les contrats devenus faux sont réécrits (story 33). `<stdlib.h>` et l'inclusion de la transitoire sortent des trois fichiers.

### Relevé « éclats `ga` rendus en cours de partie », refait pour 4e

Lu à 4567120. Les objets de 4e vivent plus longtemps qu'un éclat : peuvent-ils en désigner un après le rendu de sa place ?

| Objet de 4e | Peut-il désigner un éclat ? | Pourquoi |
|---|---|---|
| `re` du `jc` tenu (`ok.lheld`) | oui, inatteignable | `jc:hq` prend `ok.o_` quand `o_.gv` existe, et `ga` hérite `gv` de `jo` ; **remis à nil** par `oublier` (étape 3), réécrit au `hq` suivant |
| `qu`, `re` d'un `jc` posé | non | `jc:ey` ne pose que si `d_` réussit sur un `re` que `ui:jz("gv")` a accepté. `jo:gv` ne rend vrai que pour `me=="ly"`, et l'éclat reste « unknown ». `qu` vient de `jo:ldown`/`nr:ldown`, sur « ly » ou un relais |
| `et` d'un `jc` | non | `jc:ce` ne cherche que dans `qu.oe`, `cg.np` et `cg.jc` ; l'éclat a `.tags = 0` ; le « too far » est statique |
| `fo` accroché par un `jc` | non (sens inverse) | `jc:hq` écrit `et.fo` sur une planète de `cg.np` ou un slipway, jamais sur un éclat |
| point souris | non | `ra` est `ok.qa`, `p` est un membre du `jc` |
| `lp`/`rl` d'une route, `hg` | non | une route ne passe que par des slipways posés et leurs planètes ; `hg` tient des identifiants (un éclat emprunte celui de la planète remplacée, chaîne de génération, comparée par `strcmp`) |
| traînées | non | positions en valeur |

### Inventaire du §4 pour 4e

Quatre durées de vie finissent sur des objets de 4e :

- **la génération** (slipways posés et leurs arènes, routes gardées) : rendue au vidage qui suit le `eu` suivant, ou remise à zéro au chargement et à l'arrêt ;
- **l'arène de prise d'un slipway refusé** : réutilisée par la prise suivante (icône ou slipway), après son `rendre` ;
- **la place d'une traînée** : rendue au `del` dans `kj` ou au `rendre` du slipway ;
- **la frame** (routes candidates) : remise à zéro en tête de `update60`.

**Méthode.** Refaire **mécaniquement**, sur le code du commit, les deux `grep` de 4a sur les 65 fichiers de `CORE_SRCS` (pointeurs `static`, puis champs pointeurs des structures `static`). Y ajouter :

- le `grep` des écritures de `fo` (`->fo = `, `pico8_probe_et_hang_fo`) et de leurs lectures ;
- le `grep` des lecteurs de `->re` d'un `jc` ;
- le `grep` des appels de `->kf(` et des lecteurs de `->by`/`->cc`.

**Relevé fait à 4567120**, avec ce qu'ajoute 4e :

| Pointeur | Peut viser | Statut |
|---|---|---|
| `g_jo_prise`, `g_jo_prises_libres` (`planet`, 4d) | des arènes de la génération, désormais aussi celles des `jc` | `g_jo_prise` n'est non nul qu'entre `ouvrir` et `fermer`, et `pico8_jc_struct_new` les appelle tous deux. Seules les arènes de `jc` refusés **déjà rendus** entrent dans la liste. `pico8_jo_oublier_prises()` vide la liste au point d'oubli, au chargement et à l'arrêt |
| `g_arenes[ts]` (`zone_particules`) | — | `ts` est une sorte de places nues : pas d'arène tenue |
| `g_ok` → `ok.lheld` | le `jc` tenu | remis à nil par `ui:bh` après `ey` ; c'est le nouvel `ok` après la bascule |
| `g_ok` → `ok.o_`, `ok.gj` | — | un `jc` n'a pas `hd` : jamais dans `cv.hd`, jamais survolé |
| `g_hv`, `g_cv[].list`, `g_cg[].list` (`entity`) | des `jc` | un refusé est retiré par `he` (`dg` de N+1) avant d'entrer en file ; refaits au `jl()` d'`eu` |
| `g_file[]` (`file_sortie`) | un `jc` refusé ou abandonné | vidé avant le rendu de la génération, et avant tout recyclage d'arène (le recyclage est fait **par** le `rendre`) |
| `fo` des planètes, des `jc` ; `g_jc_too_far.fo` ; `g_bv_et.fo` | un `jc` tenu | voir la table « La coupe de `fo` » |
| `oe`, `by`, `cc`, `lw.t`, `gf.t` | des `jc` posés, des routes | voir l'inventaire des sites |
| `g_mo`, `g_je`, `g_kc`, `g_ju`, `g_dk`, `g_fm`, `g_gb[]`, `g_ie_gy`, `g_jo_oc`/`g_jo_ir`, zones et poignées | — | ne peuvent pas viser un objet de 4e, comme à 4a, 4c et 4d |

**Objets d'autres zones qui désignent un objet de 4e.**

- Les panneaux de planète (`jo:gh`, zone interface) ne lisent que `by->count`.
- La sonde `mz`/`g_` qui vise un slipway posé (`bv`, `et`) est une entité rendue par la file avant le rendu de la génération.

**Conclusion** : ni le point d'oubli de partie ni `oublier` ne gagnent d'entrée. La liste partagée et la coupe de `fo` de 4d couvrent les slipways dès qu'ils ont un `rendre`. Un pointeur trouvé ensuite par ASan sera ajouté au point d'oubli et noté dans « Découvert » comme un **trou de l'inventaire**.

### Verrou

- `tools/alloc_guard_exceptions.txt` : retirer `slipway` 5, `slipway_display` 1, `trade` 5.
- **Prévision : 16 / 47 → 13 / 36**, mesurée par `make alloc-guard` sur le prototype à 4567120 (« verrou objets 65 allouent 13 appels 36 »). L'arène ne change aucun compte, puisqu'elle est prise à la génération.

### Budget : mesure, projection, décision

- **Mesure par prototype** à 4567120, sur une copie `git archive` hors dépôt. Le prototype met en œuvre la conception de cette PRD :
  - arène partagée à 2048 ;
  - retour d'arène séparé en une fonction « zone, état » ;
  - `rendre` du `jc` ;
  - point souris en membres ;
  - sorte `ts` à 680 places ;
  - routes candidates en frame, copiées à la garde.

  Les 26 e2e ont été rejoués avec `PICO8_REMPLISSAGE`, contre la même mesure sur 4567120. Résultats :
  - les 26 `*.sha256` sont identiques ; `sanitize-net` répond « 26 propre(s) » ; le rejeu de vingt refus est propre sous ASan ;
  - **`max_octets partie` sur les 26 e2e : 130 720 o** (`pause_restart`, sans slipway, inchangé), donc **`M_4e` = `M_4d`** ;
  - `trade_rich_mine` : 75 488 → **95 312 o**. Le surcoût vient de trois structures d'icône posées à +512, de huit arènes à 2048 (sept slipways posés, plus un refusé en vol), de sept routes et de la croissance de `by`/`cc` ;
  - `game_end_bankrupt` : 80 464 → **84 048 o** (sept prises d'icône × 512) ;
  - par route gardée de deux planètes : 160 + 2 × (24 + 32) = **272 o**, plus 32 o quand `by`/`cc` grandissent ;
  - **vingt refus** (rejeu synthétique : f0-34 de `slipway_build`, puis vingt cycles « pression sur « p27 » (88,37), dix frames tenues en (40,100), relâché, trois frames ») : la génération ne bouge pas pendant les dix frames tenues ; elle vaut 67 552 o après le premier refus comme après le vingtième (sans arène à 24057b4 : +1 648 o par cycle) ;
  - `trade_rich_mine` : génération identique de f139 à f141, ce qui couvre le saut d'année et le `bd` de f140 (routes reproposées, aucune gardée). Elle prend +2 048 au clic de f142 et rien au clic de f148, qui reprend l'arène rendue à f145 : **f151 − f139 = une arène exactement** ;
  - zone frame, maximum par frame : 32 464 → **37 344 o** sur `trade_rich_mine` (32 `kg`), inchangé ailleurs, pour une zone de 256 Ko ;
  - traînées : **13 vivantes au plus** (`trade_rich_mine`), 4 sur `synthesizer_second_link`.
- **Projection.**
  - Estimations restantes (4a), génération : 4f ~1, 4g ~6, 4h ~12, soit **19 Ko** ; chargement : 4g ~32 Ko.
  - `r = max(1, M_4e / (2 + 20 + 160 + 30 Ko))` = **1**.
  - `P_4e` ≈ 127,7 + 19 ≈ **147 Ko** ≤ 704 ; `Q_4e` = 32 Ko ≤ 64.
  - Zone particules : 130 822 ≤ 131 072.
  - **On continue.**
- **Hors e2e** (§6, §7).
  - Chaque slipway posé coûte une arène (2 Ko) et ses routes ~0,3 Ko. Cent slipways font ≈ 230 Ko, encore sous les 704 Ko moins `M_4e`.
  - Les routes candidates d'un `bd` vont en zone frame. `nr:cn` parcourt tous les chemins sans slipway répété : un réseau dense de relais pourrait en faire assez pour remplir les 256 Ko, et `pico8_fatal` « zone frame pleine » le dirait.
- **Mesure à refaire** après conversion : `M_4e`, `K_4e`, `max_vivantes ts`, `max_elements ts`. Les noter dans le commit et dans « Découvert », comparés au prototype ; un écart s'explique avant la projection.
- **Décision**, règle de 4c, dans l'ordre :
  - si `P_4e ≤ 704` et `Q_4e ≤ 64` : on continue ;
  - sinon, déplacer la frontière entre génération et chargement ;
  - sinon, relever d'un chiffre sur les 49 Ko ;
  - sinon, **arrêt de l'étape 4 à 4e** et signalement à l'humain (zone, limite voulue, somme actuelle 1487 Ko sur 1536).
- Un `pico8_fatal` « zone partie pleine » pendant 4e se lit d'abord comme un `jc` refusé ou une route candidate entrés tels quels en génération, ce que montrent le rejeu de refus et la fenêtre f139-f151. Ensuite seulement, comme une limite trop basse.

### Note « Découvert » et clôture

À la clôture, sous l'entrée 4 de `docs/plan-4.md`, une note « *Découvert (4e) :* » de quelques phrases :

- placement :
  - `jc` et sa liste `ts` dans l'arène de prise de 4d, portée à 2048 et partagée avec les icônes ;
  - point souris réécrit en place ;
  - routes candidates en zone frame, copiées en génération à la garde ;
  - traînées en sorte `ts` (680 places de 24 o, zone particules à 130 822 o) ;
- correction de la table de 4a : `jc` est par action (chaque pression), pas une fois par slipway ; `kg` est par `bd`, et ses routes ne sont gardées qu'une fois ;
- le `rendre` du `jc`, qui le fait passer sous la coupe de `fo` de 4d ;
- l'inventaire et le relevé « éclats `ga` » refaits ;
- les mesures et la décision : `M_4e`, `K_4e`, `P_4e`, `Q_4e`, `r`, et le coût de l'arène à 2048 sur les icônes ; l'estimation de 4a (~30 Ko) contre la mesure ;
- les comptes du verrou.

Le §2 est corrigé :

- ligne « Frame » : ajouter « routes `trade` candidates de `bd` (propositions, `kg`) » ;
- ligne « Partie » : « routes `trade` » devient « routes `trade` gardées dans `by` ».

Sous l'entrée 4, une ligne « *Reçu de l'étape 4e :* » **pour 4h** :

- le point souris d'un slipway tenu garde `ok.qa` (`ra = mp`) ;
- il est réécrit à chaque `hq` et lu par le `kj` de la même frame ;
- un `jc` refusé le garde jusqu'à son `rendre`, sans le relire après le dessin de sa dernière frame.

La PRD est retirée de `docs/prd/` dans le commit de clôture.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 :

- `make test-all` et `make sanitize-net` passent ;
- les 26 `*.sha256` sont identiques ;
- `alloc-guard` est à égalité stricte.

Aucun morceau ne passe la transitoire à un nouvel appelant.

1. **Routes : candidates en zone frame, copie en génération à la garde.**
   - Les tests d'abord :
     - TU de `pico8_trade` : `kg` et `trade_new` font la route dans la zone passée ; la copie ne partage ni structure ni listes avec l'original, et se relit après une remise à zéro de la zone frame (enfant ASan) ;
     - la fenêtre de `trade_rich_mine_bd_sur_des_routes_ne_fait_pas_grandir_la_partie` passe de f137-f151 à **f139-f151**, en égalité stricte de `prise − R` : le slipway posé à f136 fait garder une route au `bd` de f139, ce qui est légitime. Le test est rouge si `kg` est fait en génération ;
     - une route de `by` gardée à f139 se relit intacte à f151 sous ASan.
   - `pico8_trade_new` et `kg` prennent la zone ; `jo:ib`, `jo:cn` et `nr:cn` passent la zone frame ; la fermeture `kf` recopie en génération.
   - Le verrou : `trade` sort, **16 / 47 → 15 / 42**.
2. **Slipway dans l'arène de prise, point en place, `rendre`.**
   - Les tests d'abord :
     - `test_pico8_jc` : le point souris est le même membre à chaque `hq`, ses valeurs suivent `mp`, et la génération ne bouge pas sur dix `hq` ;
     - le rejeu de refus (fixture) : génération constante pendant les frames tenues ; `prise − R` égal du premier au vingtième refus (rouge sans le `rendre` du `jc` : +1 648 o par cycle) ; un slipway posé après les refus, puis un refus, prennent exactement une arène de plus ;
     - un refus, « restart », nouvelle partie, une pression : l'enfant ne meurt pas ;
     - `trade_rich_mine` : `prise − R` à f151 = `prise − R` à f139 + **`PICO8_JO_PRISE_OCTETS`** exactement ;
     - un `jc` refusé dont l'`et` est une planète hors écran laisse son `fo` nil après le vidage. Le test est rouge sans le `rendre` du `jc`, qui le fait passer par `oublier`.
   - `PICO8_JO_PRISE_OCTETS` → 2048, avec son contrat.
   - Le retour d'arène séparé en une fonction « zone, état » du module planète ; `pico8_jo_prise_rendue` l'appelle.
   - `pico8_jc_struct_new` entre `pico8_jo_prise_ouvrir`/`_fermer` ; `rendre` du `jc` ; le point souris en membres.
   - Les TU de prise d'icône de 4d restent verts sans changement d'attente : ils lisent la constante.
   - Le verrou : `slipway` sort, **15 / 42 → 14 / 37**.
3. **Traînées en zone particules.**
   - Les tests d'abord :
     - TU de la zone particules : la sorte `ts` prend et rend ses places ; 681 prises sortent par `pico8_fatal` « zone particules ts pleine » ;
     - à chaque frame de `trade_rich_mine` depuis f112, places `ts` vivantes = Σ des `ts->count` de `cg.jc` ;
     - après `trade_rich_mine` puis `pico8_cart_load()`, places `ts` vivantes = 0 ;
     - `le_rejeu_ecrit_le_remplissage_en_fin_de_session` gagne la sorte `ts` dans ses lignes attendues (`ts`, `max_vivantes ts`, `max_octets ts`, `max_elements ts`).
   - La sorte `ts` et son contrôle de taille ; `jc:ch` prend une place, `kj` la rend au `del`, le `rendre` du `jc` rend les vivantes ; le rejeu écrit les lignes `ts`.
   - Le verrou : `slipway_display` sort, **14 / 37 → 13 / 36**.
4. **Mesure et clôture.**
   - Le relevé sur les 26 e2e, `M_4e`, `K_4e`, les maxima `ts`, la comparaison au prototype, la projection et la décision, dans le message du commit.
   - L'inventaire refait par `grep`.
   - La note « Découvert (4e) », la correction du §2, la ligne « *Reçu de l'étape 4e* » pour 4h, le retrait de cette PRD.

Le morceau 1 est indépendant. Le 3 dépend du 2, parce que le `rendre` du `jc` rend ses traînées. Le 4 dépend des trois.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris et le maximum de la zone partie, les places vivantes et les maxima de la zone particules, par les accesseurs ou par les lignes de `PICO8_REMPLISSAGE` ;
  - les champs publics d'un slipway (`re`, `ts`, `ts->count`, `p_`, `header.me`), d'une route (`lp`, `rl`, `qe`), d'une planète (`by`, `cc`, `fo`) ;
  - un rapport ASan dans un processus enfant ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne lit ni les bouts de la zone ni une adresse, sauf pour la relire après sa mort. Les tailles attendues se calculent dans le TU (`sizeof`, `PICO8_ZONE_CELLS`, `PICO8_JO_PRISE_OCTETS`), jamais en dur : 1 744 et 2 048 ne valent que sur x86-64.
- **`tests/test_pico8_places_vivantes.c`** (ASan, un enfant par cas, `demarrer`/`jouer_jusqu_a`, `partie_sans_registres`).
  - **`trade_rich_mine_bd_sur_des_routes_ne_fait_pas_grandir_la_partie`** (4d) : fenêtre f139-f151. Au morceau 1, égalité stricte ; au morceau 2, différence d'exactement `PICO8_JO_PRISE_OCTETS`. Le test est rouge si le `bd` de f140 garde des routes reproposées, ou si le second refus ne recycle pas. Le prototype échoue sur la fenêtre f137-f151 de 4d (+272 o de route, +2 048 d'arène), comme prévu.
  - **Route gardée qui survit à la frame** (cas nouveau) : un pointeur vers une route de `by` de « p27 », pris après f139, se relit à f151 sans crash.
  - **Slipway tenu sans allocation**, **refus répétés** et **refus à travers une nouvelle partie** (cas nouveaux), sur le modèle de la fixture `tests/fixtures/prises_refusees.json` de 4d. La nouvelle fixture n'est pas un e2e, puisqu'elle n'ajoute pas de ligne couverte. Elle contient :
    - f0-34 de `slipway_build` ;
    - vingt cycles « pression sur (88,37), dix frames en (40,100), relâché, trois frames » ;
    - un glissé posé de « p27 » à « p48 » (f35-38 de `slipway_build`), puis un refus ;
    - « restart », puis une pression.
  - **`fo` d'une planète hors écran après un `jc` refusé** (cas nouveau), sur le modèle du cas `fo` de 4d.
  - **Invariant des traînées** et **traînées rendues au rechargement** (cas nouveaux, morceau 3).
  - **`le_rejeu_ecrit_le_remplissage_en_fin_de_session`** : la liste des sortes de particules gagne `ts`, et la boucle `max_elements` la couvre.
  - **Inchangés** :
    - `message_box_hold` f45 = f90 ;
    - `game_end_final_score` f2 = f51 (aucun slipway) ;
    - les égalités exactes de 4c/4d sur `pause_restart` (aucun slipway ni route) ;
    - les prises refusées de 4d (50 896 o constants à 2048 dans le prototype) ;
    - la sentinelle de 4a.
- **`tests/test_pico8_jc.c`** : `test_hq_mouse_point` attendait un point neuf à chaque appel (`r2 != r1`). Il attend désormais le même membre, avec `ra == mp`, `p == mp/128`, et la génération inchangée. Il est rouge dans le prototype, comme prévu.
- **`tests/test_pico8_trade.c`** et les autres TU qui appellent `pico8_trade_new`/`kg` (33 appels dans sept fichiers) : l'argument de zone est ajouté, et une route qui doit survivre à un `update60` est faite en génération.
- **`tests/test_pico8_zone_particules.c`** : la sorte `ts` est testée comme les autres sortes nues (prise, rendu, plein).
- **Verrou** : `make alloc-guard` à égalité stricte, avec la liste sans les trois fichiers.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. Dans le prototype à 4567120, `make test` échoue exactement sur les cas que 4e réécrit : `test_hq_mouse_point`, la fenêtre de `trade_rich_mine`, et `le_rejeu_ecrit_le_remplissage_en_fin_de_session` (lignes `ts`).
- **Exemples dans le dépôt** :
  - `tests/test_pico8_places_vivantes.c` : rejeu par enfant, `octets_des_registres()`, les cas de 4c et 4d, dont les prises refusées et le `fo` hors écran ;
  - `tests/fixtures/prises_refusees.json` : la forme du rejeu de refus ;
  - `tests/test_pico8_zone_particules.c` : sortes nues et arènes.

## Out of Scope

- Les structures de planète, `oe`, `by`/`cc` en tant que listes, l'arène de prise, `pico8_zone_arene_vider`, la coupe de `fo` (TROU-26) : 4d, versés. 4e ne touche à l'arène que pour sa taille et pour séparer le retour d'arène en une fonction « zone, état ».
- Les trous de ver `mz` (`mz:pq`, `ox`) : 4f.
- `ok.qa`, les boîtes par frame des singletons, la sonde `g_` et `bv` : 4h. 4e ne fait que garder `ra = mp` dans le point souris et le signaler.
- Un budget de slipways affiché et bloquant (§7).
- Relever `PICO8_ZONE_PARTICLES_BYTES` ou `PICO8_ZONE_GAME_BYTES`, sauf si la mesure l'exige.
- Retirer TROU-7 : étape 5.
- Lancer l'application raylib.

## Vérifié contre 4d

Relu à `HEAD` 4567120, sans toucher git ni les sources :

- la note « *Découvert (4d)* » et la ligne « *Reçu de l'étape 4d* » de `docs/plan-4.md` ;
- `git log -5 --stat` (24057b4, 54c2624, cbe29df, 4567120) ;
- `src/pico8_planet.{h,c}` : `pico8_jo_prise_ouvrir`/`_fermer`/`_rendue`, `pico8_jo_zone_construction`, `pico8_jo_oublier_prises`, `pico8_jo_oublier_fo`, `PICO8_JO_PRISE_OCTETS`, liste de 4 prises libres ;
- `src/pico8_zone_choisie.h` (`pico8_zone_arene_vider`) ;
- `src/pico8_cart.c` (`oublier`, `oublier_partie`, chargement, arrêt) ;
- `docs/trous-de-portage.md` (TROU-26) ;
- `tools/alloc_guard_exceptions.txt` et `make alloc-guard` sur une copie `git archive HEAD` : 65 objets, 16 allouent, 47 appels ;
- `git diff --stat 24057b4 HEAD` sur `slipway`, `slipway_display`, `trade` : aucun changement.

Les hypothèses du brouillon, une par une :

- **H-4d-1 (Reçu) — tenue, et plus favorable.** La ligne écrite est plus courte que celle que prévoyait la PRD 4d (`lw.t` hors génération), et elle ajoute que « l'arène de prise et sa liste de prises libres sont réutilisables pour le slipway tiré ». La fenêtre f137-f151 du TU `trade_rich_mine` est toujours là ; 4e la déplace à f139.
- **H-4d-2 (verrou) — tenue.** 16 / 47 ; les 11 relocations de 4e sont aux lignes de 24057b4. Le prototype mesure **13 / 36**.
- **H-4d-3 (arène de prise) — tenue, avec une limite de forme.** `pico8_zone_arene_vider`, une liste de 4 prises libres et `pico8_jo_oublier_prises()` aux trois points d'oubli existent. Mais le pool a une seule taille (1536, trop petite pour un `jc`) et reconnaît l'arène par l'`oe` d'une planète. D'où la décision « Arène de prise partagée » : arène portée à 2048, et retour d'arène séparé en une fonction « zone, état ».
- **H-4d-4 (planètes en génération) — tenue.** Planètes, `p`/`ra`/« p… », `oe` en génération, ou dans l'arène d'une structure posée.
- **H-4d-5 (budget) — tenue.** `M_4d` = 130 720, `trade_rich_mine` = 75 488, `game_end_bankrupt` = 80 464, décision « on continue ».
- **H-4d-6 (inventaire) — tombée.** 4d a versé la coupe de `fo` (`pico8_jo_oublier_fo`, TROU-26), générique pour `cg.np`, appelée par `oublier`. Ce n'est donc plus à 4e de l'écrire. 4e la sort de son périmètre, vérifie que les champs qui tiennent un `jc` sont couverts (table « La coupe de `fo` »), et note que c'est son `rendre` qui y fait passer les slipways.
- **H-4d-7 (zone de construction) — tenue, et utilisée.** La zone de construction n'est ouverte qu'autour du `new_entity` d'`ie:ldown`. 4e l'ouvre aussi autour de `pico8_jc_struct_new`, au lieu d'une arène propre.
- **H-4d-8 (TU) — tenue.** Les égalités exactes de `pause_restart` restent vertes dans le prototype. Les TU de prise d'icône lisent `PICO8_JO_PRISE_OCTETS` et restent verts à 2048.

**Ce que la vérification change dans cette PRD** :

- l'arène propre aux slipways (« prise jc », liste à part) est remplacée par l'arène de prise de 4d, partagée et portée à 2048 ;
- la coupe de `fo` sort du périmètre et devient une vérification de couverture ;
- le point d'oubli de partie ne gagne plus d'entrée ;
- le prototype est refait à 4567120 avec l'arène, ce qui donne les mesures de « Budget » et le verrou 13 / 36 ;
- les comptes des morceaux partent de 16 / 47 ;
- le morceau 2 n'attend plus 4d.

## Further Notes

- **Pas d'humain disponible, et pas d'outil `Agent` dans cette session pour lancer un agent `guideline` neuf.** J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage).
  - **Hypothèses explicites** : celles sur 4d sont vérifiées (« Vérifié contre 4d »). Les fréquences ont été lues dans le cart :
    - `jo:ldown` l. 412, `nr:ldown` l. 1112, `ui:bh` l. 225-236, `_update60` l. 1187, `d_` l. 569 ;
    - `jc:pq`/`hq`/`ey`/`ce`/`ch`/`kj` l. 620-681, `trade:kg` l. 606 ;
    - `jo:cn` l. 495, `jo:ib` l. 501-512, `nr:cn` l. 1116-1119, `jo:kj` l. 541-549, `bv` l. 551-558.

    Il en sort deux corrections de la table de 4a : `jc` est par pression, et les routes de `kg` sont par `bd` et presque toutes refusées.
  - **Simplicité** :
    - une fonction nouvelle côté routes (la copie) et un argument de zone ;
    - le point souris en deux membres, sans allocation ni statique ;
    - une sorte de particules de plus, sans relèvement ;
    - aucun second mécanisme d'arène : celui de 4d, une constante relevée et une fonction séparée ;
    - pas de valeur pour les routes, hors de proportion.
  - **Changements chirurgicaux** : 11 relocations dans trois fichiers. S'y ajoutent :
    - la fermeture `kf` (un appel) ;
    - `jo:ib`/`jo:cn`/`nr:cn` (l'argument de zone) ;
    - `PICO8_JO_PRISE_OCTETS` et la séparation de `pico8_jo_prise_rendue` ;
    - la zone particules (une sorte) ;
    - le rejeu (une colonne de noms).
  - **Critères vérifiables** :
    - 13 / 36 au verrou ;
    - génération constante pendant un slipway tenu et sur vingt refus ;
    - +`PICO8_JO_PRISE_OCTETS` exactement sur `trade_rich_mine` f139-f151 ;
    - places `ts` vivantes = Σ des listes, puis 0 après rechargement ;
    - `fo` nil après le rendu d'un `jc` refusé ;
    - 26 `*.sha256` et `sanitize-net` ;
    - `M_4e` = `M_4d` = 130 720 o, `P_4e` ≤ 704 Ko.
- **Le prototype.** Il a d'abord été fait à 24057b4 (`jc` en génération sans arène), puis refait à 4567120 avec la conception de cette PRD, dans des copies `git archive` hors dépôt.
  - Résultats à 4567120 :
    - `alloc-guard` 13 / 36 ;
    - 26 `*.sha256` identiques, `sanitize-net` « 26 propre(s) », rejeu de vingt refus propre sous ASan ;
    - `make test` vert, sauf les trois cas que 4e réécrit ;
    - les TU de prises refusées de 4d verts à 2048 ;
    - les mesures de « Budget ».
  - Il n'est pas versé : c'est au cycle `tdd` de 4e de refaire le code, tests d'abord.
- **`hg_own` pèse 728 des 1 112 o d'un `jc`** (90 identifiants). Le réduire serait une économie PSX (§7 du plan), pas un sujet de 4e. Le faire ramènerait le lot sous 1536, et rendrait inutile le relèvement de l'arène.
- **4f** (trous de ver) a « les mêmes motifs que 4e » (4a). `mz` accroche aussi `fo` sur un slipway (`pico8_probe_et_hang_fo`) ; `jc:kj` le remet à nil sans condition, et la coupe de 4d couvre déjà les planètes.
