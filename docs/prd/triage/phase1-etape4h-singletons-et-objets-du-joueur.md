---
title: "Plan 4, étape 4h — Singletons de jl/eu et objets du joueur : boîtes par tick en place, singletons en génération, sonde dans l'arène de prise"
description: "Les singletons mo, je, ok, bg, ju et fv sont pris à malloc et jamais rendus, leurs boîtes vec sont refaites à chaque frame (qb:ph en fait trois, nf:ph, ok:lj, bv, bg:hq une chacun), les 16 calques ic de ok sont libérés puis refaits par la transitoire à chaque frame, et chaque pression dans le vide fait une sonde g_ jamais libérée. 4h réécrit ces boîtes en place dans leur objet, range les singletons dans la génération, les calques en zone frame et la sonde dans l'arène de prise : il ne reste au verrou que pico8_zone_transitoire.c, et l'étape 4 se ferme."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. L'étape 4 range les objets de partie dans la **zone partie**. Cette PRD traite la dernière sous-étape, **4h, singletons de `jl`/`eu` et objets du joueur**. Elle ferme l'étape 4.

**Elle est rédigée pendant 4g.** À la commande, 4g n'était pas finie : sa PRD était en file. Tout ce que cette PRD suppose de l'issue de 4g est écrit comme hypothèse `H-4g-n` (section « Hypothèses sur l'étape 4g »). Ce qu'il faut relire est listé dans « À revérifier une fois 4g finie ».

Le contenu lu est toujours du contenu commité (`git archive`), jamais l'arbre de travail :
- **88d9567** (4f close) : base de la lecture et du premier prototype ;
- **5c3ea7e** : 4g s'est close pendant la rédaction (commits bc94ff3, f16f654, 29bf950, clôture 5c3ea7e). Le prototype a été refait sur ce commit, et chaque hypothèse y a été confrontée. Le statut est donné hypothèse par hypothèse.

**Le texte de l'étape**, recopié du plan à 88d9567 (entrée 4, extraits qui concernent 4h) :

> 4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement, et si son remplissage ne monte pas pendant une phase sans action d'un e2e.
>    - **4b et suivantes.** Une famille de propriétaires par PRD dans l'ordre fixé par la PRD 4a : […] 4h singletons de `jl`/`eu` et objets du joueur (famille ajoutée par 4a : les « Reçu » des étapes 1 à 3 leur reviennent). N'entre en zone partie qu'une allocation faite une fois par objet de partie ; un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
>    - *Reçu de l'étape 4e :* pour 4h, le point souris d'un slipway tenu garde `ok.qa` (`ra = mp`) : la zone de `ok.qa` doit le couvrir.
>    - *Reçu de l'étape 4f :* pour 4h, `ox` et `ij` d'un `mz` désignent des sondes `g_`, et `mz.p`/`mz.ra` restent les boîtes de `bv`/`ok.qa` : ranger la sonde dans une place recyclée demande de revoir ces pointeurs.
>    - *Reçu de l'étape 1 :* même chose pour les boîtes `vec` des objets de partie (`p`/`v`/`ra` de `qb`, `ra` de `nf`, `mo->v`, `re`/`p` de `jc:hq`, `p` de `pico8_bv`, `ie.gy`) ; `qb:ph` seul en fait trois par frame. […]
>    - *Reçu de l'étape 2 :* restent à cette étape `ok.qa` (gardé par les hôtes de `bv` et le point souris de `jc:hq` : objet relu par des objets de partie), la structure `ok` et ses calques `ic`, la liste statique de départ, `ie.jb` (fait une fois par partie), `fv`, la structure `pj`, `hp.i`. […]
>    - *Reçu de l'étape 3 :* la structure `g_` (et `fv`) et sa boîte `p` de `bv` restent à cette étape (`/* TROU-7 */`) ; […]

Et, ajouté par la clôture de 4g (5c3ea7e) :

> - *Reçu de l'étape 4g :* pour 4h, `pico8_probe.c` n'a plus que `pico8_bv` et `pico8_g__struct_new` ; `fm` n'est plus libéré par `set_fm`, et `g_fm` désigne l'ancien `hy` (génération) jusqu'à `eu`.

**La ligne de 4h dans la table des familles de la PRD 4a**, retirée par b46601e (`git show b46601e~1:docs/prd/triage/phase1-etape4a-socle-zone-partie.md`), recopiée :

> | **4h** | **Singletons de `jl`/`eu` et objets du joueur** | `pico8_qb_struct_new` (41), `pico8_qb_ph` (122, 124, 129) ; `pico8_nf_struct_new` (39), `pico8_nf_ph` (96) ; `pico8_ui_struct_new` (119, 129), `pico8_ui_shutdown` (156), `pico8_ui_lj` (235), `pico8_ui_gw` (295) ; `pico8_bg_struct_new` (54), `pico8_bg_hq` (208) ; `pico8_fv_struct_new` (`help` 117) ; `pico8_pj_struct_new` (`score_panel` 263) ; `pico8_g__struct_new` (`probe` 413), `pico8_bv` (`probe` 162) | `scroll` 4 → sort ; `mouse_reader` 2 → sort ; `ui` 5 → sort ; `bg` 2 → sort ; `help` 1 → sort ; `score_panel` 1 → sort ; `probe` 2 → sort (17) | *Reçu 1* : `p`/`v`/`ra` de `qb`, `mo->v`, `ra` de `nf`, `p` de `pico8_bv` ; *Reçu 2* : `ok.qa`, la structure `ok` et ses calques `ic`, `fv`, la structure `pj` ; *Reçu 3* : la structure `g_` (et `fv`) et sa boîte `p` de `bv` (les `rendre` de `g_` et `fv` existent déjà et ne rendent que leur `qy`) |

Ses lignes de la table « Sites qui allouent par tick ou par action », recopiées :

> | `pico8_qb_ph` (`p`, `v`, `ra` de `mo`, trois boîtes) | par frame | 4h |
> | `pico8_nf_ph` (`ra` de `je`) | par frame | 4h |
> | `pico8_ui_lj` (`ok.qa`, gardé par les hôtes de `bv` et le point souris de `jc:hq`) | par frame | 4h |
> | `pico8_ui_gw` (16 calques `ic` libérés puis refaits) | par frame | 4h |
> | `pico8_bv` (`p` de l'hôte) | par frame où une sonde ou un hôte est tenu | 4h |
> | `pico8_bg_hq` (boîte `v`) | par frame de glissé | 4h |

La raison de l'ordre, recopiée de la même PRD :

> - **4h** est la plus difficile : boîtes par frame partagées entre objets (`ok.qa` gardé par les hôtes de `bv`), `ok` encore lu après `jl()`, sondes tenues. Elle passe en dernier. Elle donne aussi le critère de fin de l'étape 4 : `pause_restart` sous empoisonnement avec tous les singletons en zone.

L'estimation de 4a : « 4h singletons | `qb`, `nf`, `ui` et ses 16 calques, `bg`, `fv`, `pj`, sondes ≈ 4 Ko, une fois de plus par `_init` | ~12 Ko ».

**La règle des familles**, recopiée de la PRD 4a :

> - n'entre en génération de partie qu'une allocation faite une fois par objet de partie créé (planète posée, slipway, route, icône, singleton de `jl`/`eu`) ;
> - un champ réaffecté par tick ou par action doit, au choix de la PRD de sa famille et site par site :
>   - être **réécrit en place**, quand la lecture prouve qu'aucun autre objet ne garde l'ancienne valeur ;
>   - devenir une **valeur** au lieu d'une boîte, quand la boîte n'est jamais modifiée en place. Le cart en modifie trois en place : `self.ra.y+=` (l. 578), `p.v.x=`/`p.v.y=` (l. 699-700), `self.p.x=`/`self.p.y=` (l. 1141) ;
>   - aller en **zone frame**, s'il n'est relu que dans la frame ;
>   - prendre une **place** dans une zone qui rend.

**Les lignes de l'inventaire du §4 de 4a qui renvoient à 4h**, recopiées :

> | `g_mo` | `entity` | **repointé** par le `jl()` d'`eu`, après la bascule | 4h |
> | `g_je`, `g_ok` | `ui` | **repointés** par le `jl()` d'`eu`, après la bascule ; l'ancien `ok` n'est plus lu une fois `ok:lj` fini | 4h |
> | `g_ju` | `score_panel` | **repointé** par `eu`, après la bascule ; lu par `oublier` (`ju->ed`) au vidage qui précède le rendu, encore vivant | 4h |
> | `g_bv_et.fo` (`probe`) | peut garder un hôte de `bv` d'une ancienne partie ; **écrit seulement, jamais lu** (seuls `jo.fo`, `jc.fo` sont lus) : ne peut pas être lu | 4h |
> | `ok.o_`, `ok.gj`, `ok.lheld`, `ok.rheld`, `ok.ic`, `ok.le`, `ok.qa` | champs du **nouvel** `ok`, faits à nil (ou `&g_shared_le`) par sa construction après la bascule ; l'ancien `ok` n'est plus atteint par aucun pointeur une fois `ok:lj` fini. `oublier` lit `ok.lheld` et le `re` d'un `jc` tenu au vidage qui précède le rendu : c'est le nouvel `ok`, dont `lheld` est nil | 4h |
> | `mo.v`/`.p`/`.ra`, `je.ra` | champs des nouveaux `mo`/`je` | 4h |

**Ce qui se passe aujourd'hui** (lu à 88d9567 et à 5c3ea7e, cart et port) :

- **Le verrou.** `make alloc-guard` sur une copie `git archive` répond « verrou objets 65 allouent 12 appels 33 » à 88d9567, et « allouent 8 appels 20 » à 5c3ea7e. La prévision de 4g est tenue. Les 17 relocations de 4h, rattachées par `objdump -r -j .text` et `addr2line` sur les objets `-g` d'`alloc-guard` (lignes à 88d9567) :

  | Fichier | Fonction (ligne) | Appel |
  |---|---|---|
  | `scroll` | `pico8_qb_struct_new` (41) | `malloc` de `mo` |
  | `scroll` | `pico8_qb_ph` (122, 124, 129) | trois `malloc` : `p`, `v`, `ra` |
  | `mouse_reader` | `pico8_nf_struct_new` (39) | `malloc` de `je` |
  | `mouse_reader` | `pico8_nf_ph` (96) | `malloc` de `je.ra` |
  | `ui` | `pico8_ui_struct_new` (120, 130) | `malloc` de `ok`, transitoire pour les 16 calques `ic` |
  | `ui` | `pico8_ui_shutdown` (158) | `free` de `ok` |
  | `ui` | `pico8_ui_lj` (237) | `malloc` de `ok.qa` |
  | `ui` | `pico8_ui_gw` (297) | transitoire pour les 16 calques `ic`, à chaque frame |
  | `bg` | `pico8_bg_struct_new` (55) | `malloc` de `bg` |
  | `bg` | `pico8_bg_hq` (207) | `malloc` de `mo.v` |
  | `help` | `pico8_fv_struct_new` (117) | `malloc` de `fv` |
  | `score_panel` | `pico8_pj_struct_new` (263) | `malloc` de `ju` |
  | `probe` | `pico8_bv` (162) | `malloc` de la boîte `p` de l'hôte |
  | `probe` | `pico8_g__struct_new` (413) | `malloc` de la sonde |

  Sur l'objet `ui`, le motif `awk` du verrou voit aussi `pico8_ents_free` (le nom contient `free`). `alloc_guard.sh` compare le symbole entier : ce n'est pas un appel compté. Le compte de `ui` est bien 5.

  **L'attribution est complète.** 33 = 13 (4g : `research` 5, `research_icon` 3, `economy` 3, `cart` 1, `fm_free` 1) + 17 (4h) + 3 (`pico8_zone_transitoire.c`, étape 5). À 5c3ea7e : 20 = 17 + 3. Après 4h, la liste ne contient plus que `src/pico8_zone_transitoire.c 3`.
- **`jl()`** (cart l. 153-155) fait `mo,je,ok=qb(),nf(),ui()`. Il est appelé par `_init` (au chargement, après « restart », après « again? ») et par `eu`, après la bascule.
  - `_init` ne bascule pas : son `mo`/`je`/`ok`, et le `fv()` qui suit, entrent dans la génération courante, celle de la partie qu'on vient de quitter.
  - « restart » n'existe que dans le menu de jeu (`ix[4]`, fait par `eu`), et « again? » que sur le panneau de fin (`pj:ph`) : entre deux `eu`, il y a au plus un `_init`.
- **`eu`** (l. 1181-1187) fait `jl()`, `bg()`, `bw(90)`, `fm,ju=hy(),pj()`, puis les icônes.
- **`qb:ph`** (l. 1137-1141) fait `self.p+=self.v`, `self.v+=…`, `self.p.x=mid(…)`, `self.ra=ma(self.p,self)`. Ce sont trois boîtes neuves par frame. `self.p.x=` modifie la boîte neuve, pas celle de l'archétype. Le port les prend à `malloc` et ne les rend jamais.
- **`nf:ph`** (l. 203) fait `self.ra=v(stat(32),stat(33))`, une boîte par frame. Le port la prend à `malloc`.
- **`bg:rdown`/`bg:hq`** (l. 378-382) font `self.mq=rp` (c'est `je.ra`, **gardé d'une frame à l'autre**), puis `mo.v=self.mq-rp` (une boîte neuve) et `self.mq=rp`. Le glissé est la différence entre la boîte de la frame d'avant et celle de cette frame.
- **`ok:lj`** (l. 217-224) fait `self.qa=je.ra+mo.ra`, une boîte par frame. Elle est passée à `gw`, aux `ldown`/`rdown`/`hq` des objets survolés ou tenus, et gardée par :
  - **`bv`** (l. 552) : `self.ra,self.p=mp,mp/128`. L'hôte garde `ok.qa` comme `ra`, et une boîte neuve comme `p`. Les hôtes sont `g_`, `fv`, `mz` et les planètes faites par une icône (`qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`). Une sonde posée, une planète posée et un `mz` ouvert gardent donc pour toujours la boîte `ok.qa` de la frame du relâché, et leur `p`. Un `qh` posé la **modifie en place** à chaque frame par `na` (`self.ra.y+=`, l. 578).
  - **`jc:hq`** (l. 622) : `{ra=mp,p=mp/128}`. Le port réécrit `point_p` en place (4e), mais `point.ra` est `ok.qa` lui-même.
- **`ok:gw`** (l. 226) refait `self.ic={}` à chaque frame. Le port libère puis refait 16 listes par la transitoire. `ic` n'est lu que par `jz`, dans le même `lj` (`grep` : seul `pico8_ui.c` lit `->ic` et `->qa`).
- **`bg:ldown`** (l. 376-377) fait `(#cg.g_>0 and mz or g_)()`. Une sonde `g_` naît donc à **chaque pression dans le vide** tant qu'aucune n'est posée. Elle est refusée au relâché si `bv` lui a trouvé un `et`, c'est-à-dire au-dessus d'une planète, d'un slipway, ou hors du panneau `hw` (`d_`, `p_`). Le port la prend à `malloc` et ne la rend jamais. Une seule sonde est posée par partie : ensuite, `cg.g_` n'est plus vide et le vide fait des `mz`.
- **À l'arrêt et au rechargement**, `pico8_cart_load`/`pico8_cart_shutdown` font, dans l'ordre :
  - abandon de `hv` ;
  - vidage (le `rendre` de `ok` rend sa liste `le`) ;
  - `pico8_zone_partie_reset()` ;
  - puis `pico8_ui_init`/`pico8_ui_shutdown`.

  Or `pico8_ui_shutdown` **relit** `g_ok->le` et fait `free(g_ok)`. Une fois `ok` en zone, ce serait une lecture de zone remise à zéro et un `free` sur un bloc de zone.

**Le problème.** Dix-sept relocations restent au verrou. Les singletons fuient à chaque `jl`, et la sonde à chaque pression dans le vide (TROU-7). Six sites font une boîte par frame, et le glissé de caméra en fait une de plus par frame de glissé. On ne peut pas se contenter de mettre ces boîtes en zone frame : `ok.qa` est gardé par des objets posés pour toute la partie, et `je.ra` par `bg` d'une frame à l'autre. On ne peut pas non plus les mettre en génération : la génération grandirait de cinq boîtes par frame, et de plus à chaque sonde refusée.

## Solution

- **Plus aucune boîte par tick.** Chaque champ refait par tick devient une boîte **propre à son objet, réécrite en place**, et son pointeur la désigne :
  - `mo.p`/`mo.v`/`mo.ra` ;
  - `je.ra` ;
  - `ok.qa` ;
  - les `ra`/`p` qu'un hôte reçoit de `bv`, rangés dans le composant sonde de l'hôte ;
  - le `ra` du point souris d'un slipway tenu, copié dans le slipway comme `point_p`.

  `bg.mq` devient une **valeur**. Les calques `ic` de `ok` passent en **zone frame**.
- **Singletons en génération** : `mo`, `je`, `ok` (faits par `jl`), `bg` et `ju` (faits par `eu`), `fv` (fait par `_init`), dans `pico8_zone_partie()`. `pico8_ui_shutdown` oublie `ok` sans le relire.
- **La sonde `g_` dans l'arène de prise** de 4d–4f. Une sonde refusée la rend par son `rendre`, une sonde posée la garde.
- **Inventaire du §4** refait : rien à ajouter au point d'oubli de partie ni à `oublier`. Le `fo` d'une planète hors écran qui viserait une sonde refusée est déjà coupé par `pico8_jo_oublier_fo` (TROU-26).
- **Budget mesuré** par prototype à 5c3ea7e : `M_4h` = **157 704 o** (`pause_restart`, au clic), chargement 22 752 o, constant. C'est la dernière famille : `P_4h = M_4h` ≈ 154 Ko ≤ 704, et la somme des zones est inchangée (1487 Ko sur 1536). **On continue, et l'étape 4 se ferme.**
- **Verrou** : `scroll`, `mouse_reader`, `ui`, `bg`, `help`, `score_panel`, `probe` sortent. **8 / 20 → 1 / 3** (mesuré à 5c3ea7e), il ne reste que `pico8_zone_transitoire.c`.

Aucun pixel ne change. Le prototype garde les 26 `*.sha256`, `make test` est vert une fois les TU de « Testing Decisions » réécrits, `make sanitize-net` répond « 26 propre(s) », et aucune des 61 fenêtres sans action des 26 e2e ne fait monter la génération.

## User Stories

1. En tant que mainteneur du plan 4, je veux que `pico8_scroll.c`, `pico8_mouse_reader.c`, `pico8_ui.c`, `pico8_bg.c`, `pico8_help.c`, `pico8_score_panel.c` et `pico8_probe.c` n'appellent plus ni la libc ni la transitoire, pour que la liste du verrou ne contienne plus que `pico8_zone_transitoire.c`.
2. En tant que mainteneur du plan 4, je veux que l'étape 4 soit déclarée finie sur ses deux critères (`pause_restart` sous `sanitize-net` avec tous les singletons en zone ; remplissage constant pendant une phase sans action), pour que l'étape 5 puisse commencer.
3. En tant que développeur, je veux que `qb:ph` réécrive `p`, `v` et `ra` dans des boîtes de la caméra elle-même, pour qu'elle ne prenne plus trois blocs par frame.
4. En tant que développeur, je veux que la caméra garde ses boîtes de l'archétype jusqu'à son premier `ph`, pour que ce qui la lit entre sa construction et son premier tick lise la même valeur qu'aujourd'hui.
5. En tant que développeur, je veux que `bg:hq` écrive le `v` de la caméra dans sa boîte, par une fonction de la caméra, pour que le glissé ne prenne plus de bloc par frame.
6. En tant que développeur, je veux que `nf:ph` réécrive `je.ra` dans une boîte du lecteur de souris, pour qu'il ne prenne plus de bloc par frame.
7. En tant que développeur, je veux qu'un `je` neuf lise (0,0) jusqu'à son premier `ph`, pour que le curseur dessiné à la frame du clic qui lance une partie ne change pas.
8. En tant que développeur, je veux que `bg.mq` soit une copie de la position souris, et non la boîte `je.ra`, pour que le glissé reste la différence entre deux frames quand `je.ra` est réécrit en place.
9. En tant que développeur, je veux que `ok.qa` soit une boîte de `ok` réécrite au début de chaque `ok:lj`, pour qu'il ne prenne plus de bloc par frame.
10. En tant que développeur, je veux que `bv` copie la position souris dans le composant sonde de son hôte, et y range le `p` qu'il en déduit, pour qu'une sonde, un `mz` ou une planète posée ne gardent plus la boîte d'une frame passée.
11. En tant que développeur, je veux qu'un `qh` posé continue de flotter (`na` modifie son `ra` en place), pour que la boîte copiée par `bv` se comporte comme la boîte `ok.qa` du cart, que plus rien d'autre ne lit.
12. En tant que développeur, je veux que le point souris d'un slipway tenu copie `ra` dans le slipway, comme il copie déjà `p`, pour que `ok.qa` ne soit plus gardé par aucun objet.
13. En tant que développeur, je veux qu'un `mp` nil laisse `ra` à nil chez l'hôte de `bv` et dans le point souris, pour garder le comportement que les TU observent.
14. En tant que développeur, je veux que les 16 calques `ic` de `ok` soient refaits dans la zone frame à chaque `gw`, pour qu'ils ne passent plus par la transitoire et ne grandissent dans aucune zone qui dure.
15. En tant que développeur, je veux que `mo`, `je` et `ok` soient faits dans la génération courante, pour qu'ils meurent avec la partie (ou le titre) qui les a faits.
16. En tant que développeur, je veux que `bg` et `ju` soient faits dans la génération de la partie que `eu` ouvre, pour qu'ils meurent avec elle.
17. En tant que développeur, je veux que l'aide `fv` de l'écran titre soit faite dans la génération courante, pour qu'elle meure au rendu qui suit le `eu` suivant.
18. En tant que développeur, je veux que l'ancien `ok` reste lisible pendant toute la frame du clic qui lance une partie, pour que la fin de son `lj` (après `jl()`) lise un objet vivant.
19. En tant que développeur, je veux que `pico8_ui_shutdown` oublie `ok` sans le lire, pour qu'un rechargement ou un arrêt ne lise pas une zone remise à zéro.
20. En tant que développeur, je veux que la sonde `g_` faite par une pression dans le vide prenne l'arène de prise, et la rende si elle est refusée, pour que des refus répétés ne fassent pas grandir la génération.
21. En tant que développeur, je veux qu'une sonde posée garde son arène, pour que `mz.ox`/`mz.ij` et le dessin d'un `mz` lisent une sonde vivante.
22. En tant que développeur, je veux que l'aide `fv`, qui partage la classe de structure de `g_`, ne rende aucune arène, pour qu'elle meure avec sa génération.
23. En tant que mainteneur, je veux un TU qui montre qu'entre deux frames, les boîtes `mo.p`, `mo.v`, `mo.ra`, `je.ra`, `ok.qa` gardent leur adresse et changent de valeur, pour prouver qu'elles sont réécrites en place.
24. En tant que mainteneur, je veux que la génération, juste après le rendu de `pause_restart`, soit exactement « états de `bw` + registres + planètes + recherche et économie + singletons », pour qu'aucune allocation de 4h n'y entre sans être comptée.
25. En tant que mainteneur, je veux que `mo`, `je` et `ok` de la partie quittée se relisent pendant la frame du clic qui lance la nouvelle partie, et fassent planter un enfant ASan quand ils sont relus après la frame suivante, pour prouver qu'ils vivent dans la génération et meurent avec elle.
26. En tant que mainteneur, je veux la même preuve pour `bg` et `ju`, et pour l'aide `fv` du titre.
27. En tant que mainteneur, je veux que des sondes refusées à la suite ne fassent pas grandir la génération, et que chacune reprenne l'arène de la précédente, pour prouver qu'elles passent par l'arène de prise.
28. En tant que mainteneur, je veux que le `fo` d'une planète hors écran, visé par une sonde refusée au-dessus d'elle, soit coupé au vidage qui rend la sonde, pour que la planète ne lise pas une arène recyclée.
29. En tant que mainteneur, je veux qu'aucune fenêtre sans action de chacun des 26 e2e ne fasse monter la génération, pour tenir le second critère de fin de l'étape 4 sur tous les e2e et pas sur un seul.
30. En tant que gardien de l'invariant, je veux que les 26 `*.sha256` restent identiques, pour prouver qu'aucune boîte réécrite en place n'était gardée ailleurs (le glissé de caméra, le flottement des planètes posées, le curseur à la frame du clic).
31. En tant que gardien de l'invariant, je veux que `make sanitize-net` reste vert sur les 26 e2e, pour que rien ne lise un singleton, une sonde ou un calque rendus.
32. En tant que mainteneur du plan 4, je veux l'inventaire du §4 refait mécaniquement pour les trois durées de vie qui finissent sur des objets de 4h (génération rendue, arène de prise recyclée, zone frame remise à zéro), pour que la liste des références oubliées vienne du `grep`.
33. En tant que mainteneur du plan 4, je veux les maxima de la génération et du chargement relevés sur les 26 e2e, avec la décision, pour que le budget de l'étape 4 se ferme sur une mesure.
34. En tant que mainteneur, je veux que les contrats devenus faux et les `/* TROU-7 */` de ces sites soient réécrits ou retirés, et que `<stdlib.h>` et l'inclusion de la transitoire sortent des fichiers qui n'en ont plus besoin, pour que les commentaires restent vrais (règle du `CLAUDE.md`). Exemples de contrats faux : « Nothing frees the struct: TROU-7 », « Rebinds … to fresh heap vectors each tick », « Frees `ok` and gives back its `le` », « `ra` is the caller's `mp` itself, not a copy », « `mp` is stored into `ra` by REF identity », « nothing ever frees the probe itself ».
35. En tant qu'agent sans humain, je veux que toute hypothèse sur l'issue de 4g soit numérotée, confrontée au commit de clôture de 4g et revérifiée avant l'implémentation, pour ne pas bâtir sur une conception que 4g aurait changée.

## Implementation Decisions

### Inventaire des sites de 4h, et zone retenue

Chaque site est tranché par lecture : « à quelle fréquence, et qui d'autre garde l'objet ? ». La colonne « Preuve » dit pourquoi personne ne voit la différence.

| Site | Fréquence (lecture) | Qui garde l'objet | Décision | Preuve |
|---|---|---|---|---|
| `mo.p`, `mo.v`, `mo.ra` (`qb:ph`) | par frame | personne : `ui:lj`, `gd`, `iv:pq`, `dc`, `qb:md` lisent la valeur ; `grep` des écritures `= ra;`/`= mp;`/`= rp;` dans le port : aucune ne vise `mo` | **réécrit en place** dans trois boîtes de `qb` | `qb:ph` copie `p` et `v` avant d'écrire ; l'archétype n'est jamais écrit |
| `mo.v` (`bg:hq`) | par frame de glissé | idem | **réécrit en place**, par une fonction de `qb` qui écrit sa boîte `v` | idem |
| `je.ra` (`nf:ph`) | par frame | `bg.mq`, d'une frame à l'autre (`bg:rdown`, `bg:hq`) | **réécrit en place** dans une boîte de `nf`, **et** `bg.mq` devient une valeur | `mq` était la seule garde ; une copie de la valeur rend la même différence (sha256 de `camera_drag_dpad`, `colony_income_offscreen`, `tech_starbirth_protostar`, qui glissent au bouton droit) |
| `bg.mq` | par `rdown` et par frame de glissé | — | **valeur** (et un drapeau « nil » : `rp` nil, ou pas encore de `rdown`, se lit comme le vecteur nul, comme aujourd'hui) | — |
| `ok.qa` (`ok:lj`) | par frame | les hôtes de `bv` (`ra`) et le point souris d'un `jc` tenu (`ra`) | **réécrit en place** dans une boîte de `ok`, **et** ses deux gardiens copient | après les copies, `qa` n'est lu que par `gw`, `bh`, `jz` et les méthodes qu'ils appellent, dans le même `lj` (`grep` : seul `pico8_ui.c` lit `->qa`) |
| `ra`, `p` d'un hôte de `bv` | par frame où l'hôte est tenu | l'hôte, pour toute sa vie s'il est posé ; `iv` (`jo:hk`) et `ga` (`jo:hu`) empruntent le `ra`/`p` d'une planète **posée** | **réécrits en place** dans deux boîtes du **composant sonde** de l'hôte (`pico8_probe_t`), que l'hôte désigne | la boîte vit avec l'hôte ; `bv` ne tourne plus sur un hôte posé ; `na` modifie le `ra` en place comme le cart modifie la boîte `ok.qa` du relâché, qu'aucun autre lecteur ne lit entre le `dg` qui la modifie et le `ok:lj` qui la rebranche |
| `ra` du point souris de `jc:hq` | par frame où le slipway est tenu au-dessus du vide | le slipway tenu | **copié** dans le slipway (une boîte à côté de `point_p`, réécrite en place) | le `jc` réécrit `point` à chaque `hq` ; un slipway posé désigne une planète (`jc:ey`) |
| calques `ic` de `ok` (`ok:gw`) | par frame | `ok`, lus par `jz` dans le même `lj` | **zone frame** : chaque `gw` refait les 16 listes dans `pico8_zone_frame()`, sans libérer | 4a : « rien ne les lit entre le vidage et `gw` (`dg` ne les lit pas) » ; l'ancien `ok`, après le `jl()` d'un clic, ne les relit que dans la même frame |
| structures `mo`, `je`, `ok` | une fois par `jl` (au plus deux par génération : `eu`, puis `_init`) | `g_mo`, `g_je`, `g_ok`, `hv`/`cv` | **génération** | — |
| structures `bg`, `ju` | une fois par `eu` | `hv`/`cv`, `g_ju` | **génération** | — |
| structure `fv` | une fois par `_init` | `hv`/`cv`/`cg`, ses boutons (zone interface, rendus par la file avant la génération) | **génération** (courante, celle de la partie quittée) | — |
| sonde `g_` | **une par pression dans le vide**, tant qu'aucune n'est posée | `hv`/`cv`/`cg`, `ok.lheld`, `ok.o_`/`ok.gj`, le `fo` de ce qu'elle vise, `mz.ox`/`mz.ij` (si posée) | **arène de prise** : rendue par `rendre` si elle n'est pas posée (`me` ≠ `ly`), gardée sinon | voir l'inventaire du §4 ci-dessous |
| `free(g_ok)` de `pico8_ui_shutdown` | arrêt, rechargement, init | — | **supprimé** : `g_ok` est oublié sans lecture | voir « Arrêt et rechargement » |

- **Pourquoi pas la zone frame pour `ok.qa` et `je.ra`.** Le §2 du plan l'écarte : « pas les boîtes `vec` rangées dans un champ d'objet, qui vont avec leur objet ». Un champ qui désigne la zone frame pendrait entre la remise à zéro et le tick qui le refait. La boîte propre à l'objet n'alloue rien et ne pend jamais.
- **Pourquoi le composant sonde pour les boîtes de `bv`.** Un seul site écrit (`probe_host_set_ra_p`) et tous les hôtes (`g_`, `fv`, `mz`, la famille `jo`) ont déjà `probe_own`. Coût : 16 o par porteur du composant, dont les 90 planètes de `bw` : **1 440 o par partie**.
  - Écarté : des champs par sorte d'hôte (`g_`, `jo`, `mz`). C'est trois sites au lieu d'un, pour le même coût sur `jo`.
  - Écarté : des boîtes prises une fois dans l'arène des seules planètes d'icône. `bv` ne sait pas où son hôte a été fait.
  - Le composant grossit aussi dans `jc` (arène de 2048 o, `jc` 1112 → 1136 o) et dans l'éclat `ga`. Le `ga` mesuré passe de 560 à 576 o, dans des places de 640 o (zone particules inchangée).
- **Pourquoi pas la génération pour la sonde.** Une sonde naît à chaque pression dans le vide : une génération ferait grandir la partie à chaque refus. L'arène de prise a exactement sa durée de vie : recyclée à la mort d'une prise refusée, gardée par une prise posée. C'est le *Reçu de l'étape 4f*.
- **Pourquoi un champ `prise` dans `g_`.** `mz` retrouve son arène par `ox->zone`, `jo` par `oe->zone`. `g_` n'a aucun bloc de génération à lui (sa liste `qy` est en zone particules) : il garde l'arène dans un champ, NULL pour `fv`. La condition de rendu est « pas posée » : `me` ≠ `ly`. Une sonde tenue ou refusée est en `ph` ; `d_` ne la fait passer en `ly` que si elle est acceptée.

Forme retenue par le prototype (extraits) :

```c
typedef struct {            /* pico8_qb_t, en plus de l'existant */
    /* ... */ const pico8_vec_t *p; const pico8_vec_t *ra;
    pico8_vec_t p_own, v_own, ra_own;  /* p/v/ra les désignent après le premier ph */
} pico8_qb_t;
void pico8_qb_set_v(pico8_qb_t *self, pico8_vec_t v);   /* écrit v_own, y pointe v ; bg:hq l'appelle */

typedef struct pico8_probe {  /* composant sonde */
    /* ... */ pico8_vec_t bv_ra, bv_p;  /* l'hôte de bv y pointe ra (NULL si mp est nil) et p */
} pico8_probe_t;

/* nf : pico8_vec_t ra_own ; ui : pico8_vec_t qa_own ; bg : pico8_vec_t mq + int has_mq ;
 * jc : pico8_vec_t point_ra (point.ra y pointe, NULL si mp est nil) ;
 * g_ : const pico8_zone_choisie_t *prise (NULL pour fv) */
```

### Singletons en génération

- `pico8_qb_struct_new`, `pico8_nf_struct_new`, `pico8_ui_struct_new`, `pico8_bg_struct_new`, `pico8_pj_struct_new` et `pico8_fv_struct_new` prennent leur structure dans `pico8_zone_partie()`.
- **Pourquoi c'est sûr.**
  - `eu` bascule avant son `jl()` : le nouvel ensemble (`mo`, `je`, `ok`, `bg`, `ju`) entre dans la nouvelle génération.
  - L'ancien `ok` finit son `lj` dans l'ancienne, vivante jusqu'au vidage de la frame suivante (4a).
  - `_init` ne bascule pas : son ensemble (`mo`, `je`, `ok`, `fv`) entre dans la génération de la partie quittée, et meurt au rendu qui suit le `eu` suivant. Au chargement, c'est la génération de départ, vide.
  - Comme il y a au plus un `_init` entre deux `eu`, une génération tient au plus deux ensembles de `jl`.
- `pico8_ui_set_je`/`pico8_ui_set_ok` ne libèrent rien (inchangé). Leur raison, l'ancien `ok` encore lu après `jl()`, est exactement la durée de vie de la génération.

### Arrêt et rechargement

- `pico8_ui_shutdown` met `g_ok` à NULL **sans le lire**. La liste `le` de `ok` est déjà rendue : `ok` est dans `hv`, avec un `rendre`. L'abandon de `hv` puis le vidage qui précèdent `pico8_zone_partie_reset()`, dans `pico8_cart_load` comme dans `pico8_cart_shutdown`, la rendent.
- `g_mo` (`pico8_entity_init` : tenant-lieu ; `pico8_entity_shutdown` : NULL), `g_je` (`pico8_ui_init`/`_shutdown`) et `g_ju` (`pico8_score_panel_init`/`_shutdown` : NULL) sont déjà remis sans lecture.
- Entre la remise à zéro et leur remise, aucun module ne les lit. C'est vérifié par le prototype : les TU qui rechargent sous ASan passent, dont `pause_restart_recharge_apres_le_titre`.

### Inventaire du §4 pour 4h

Trois durées de vie finissent sur des objets de 4h :

- **la génération**, rendue au vidage qui suit le `eu` suivant : `mo`, `je`, `ok`, `bg`, `ju`, `fv`, et les boîtes propres qu'ils contiennent ;
- **l'arène de prise** d'une sonde refusée, recyclée ;
- **la zone frame**, remise à zéro en tête de `update60` : les tableaux des calques `ic`.

**Méthode.** Refaire **mécaniquement**, sur le code du commit, les trois `grep` de 4g sur les fichiers de `CORE_SRCS` (65 fichiers) :

- pointeurs `static`, hors `const char *` et `* const` ;
- structures `static` dont on lit les champs pointeurs ;
- structures anonymes `static struct {`.

Y ajouter le `grep` des écritures qui gardent un argument de méthode : `(->|\.)\w+ = (ra|qa|mp|rp|je_ra);`. C'est lui qui a trouvé `bg.mq`, `probe_host_set_ra_p` et `point.ra`, les seuls gardiens de `je.ra` et `ok.qa`.

**Relevé fait à 88d9567 sur le prototype** (20 pointeurs `static`, 38 lignes de structures `static`, `g_gb_px` comme seule structure anonyme) :

| Pointeur | Peut viser | Statut |
|---|---|---|
| `g_mo` (`entity`), `g_je`, `g_ok` (`ui`) | génération | **repointés** par `jl()` (après la bascule dans `eu`) ; remis au tenant-lieu ou à NULL sans lecture au chargement et à l'arrêt |
| `g_ju` (`score_panel`) | génération | **repointé** par `eu` après la bascule ; lu par `oublier` (`ju->ed`) au vidage qui précède le rendu, encore vivant ; mis à NULL sans lecture par `_init`/`_shutdown` du module |
| `ok.qa`, `je.ra`, `mo.p`/`.v`/`.ra` | la boîte de leur propre objet, ou l'archétype (`const`) | même durée de vie que l'objet |
| `ra`/`p` d'un hôte de `bv` | le composant sonde de l'hôte | même durée de vie que l'hôte ; `iv` et `ga` n'empruntent que ceux d'une planète **posée**, dont l'arène n'est jamais recyclée (4d), et meurent avant la génération (file de sortie) |
| `point.ra` d'un `jc` | `point_ra` du même `jc` | même durée de vie |
| `ok.ic` (16 calques) | zone frame | refaits par chaque `gw` avant toute lecture (`jz`, dans le même `lj`) |
| `g_jo_prise`, `g_jo_prises_libres[]` (`planet`) | l'arène d'une sonde refusée | comme en 4d : liste oubliée au point d'oubli de partie, au chargement et à l'arrêt |
| `ok.lheld` | la sonde tenue | remis à nil par `bh` au relâché, dans la frame du refus |
| `ok.o_`, `ok.gj` | une sonde survolée | remis à nil par `oublier` au vidage qui rend la sonde |
| `jo.fo` (planètes de `cg.np`) | une sonde refusée au-dessus d'elle | **déréférencé** au dessin (`fo->probe->def->dw`), puis remis à nil, mais seulement si la planète est à l'écran : **coupé** par `pico8_jo_oublier_fo` au vidage qui rend la sonde (TROU-26, mécanisme de 4d, sans entrée nouvelle) |
| `jc.fo` | une sonde refusée au-dessus de lui | **comparé à NULL seulement**, jamais déréférencé : un pointeur pendant y vaut le « vrai » du cart |
| `g_bv_et.fo` (`probe`) | une sonde refusée hors du panneau | **écrit seulement**, jamais lu (4a) |
| `mz.ox`, `mz.ij` | des sondes posées | **ne peut pas viser une sonde refusée**. La sonde refusée au relâché de la frame N sort de `hv` par `he` dans le `dg` de N+1. Le `mz` suivant naît au plus tôt dans le `lj` de N+1, car `jn` et `hm` ne sont jamais vrais dans la même frame, et `mz:pq` lit `hv`. Une sonde posée ne meurt jamais (aucun `p_` hors `d_`) |
| `g_fm`, `g_ie_gy`, `g_gb[]` | — | 4g |
| `g_dk`, `g_kc`, `g_arenes[][]`, `partie_choisie`, `g_jo_oc`/`g_jo_ir`, dispatch, `g_shared_le`, `g_ga_by_shared`, `g_jc_too_far`, `g_mz_jj_wrapper`, tables | — | ne peuvent pas viser un objet de 4h |

**Conclusion** : ni le point d'oubli de partie ni `oublier` ne gagnent d'entrée. Un pointeur trouvé ensuite par ASan est ajouté au point d'oubli et noté dans « Découvert » comme un **trou de l'inventaire** (§4). L'inventaire est refait sur le code du commit au morceau de clôture.

### Verrou

- `tools/alloc_guard_exceptions.txt` : retirer `src/pico8_scroll.c 4`, `src/pico8_mouse_reader.c 2`, `src/pico8_ui.c 5`, `src/pico8_bg.c 2`, `src/pico8_help.c 1`, `src/pico8_score_panel.c 1`, `src/pico8_probe.c 2`.
- **Mesuré** par `make alloc-guard` sur le prototype :
  - à 5c3ea7e : « verrou objets 65 allouent 1 appels 3 » (**8 / 20 → 1 / 3**) ;
  - à 88d9567, sans 4g : « allouent 6 appels 16 » (12 / 33 − 17).
- `<stdlib.h>` sort des sept fichiers s'il n'y sert plus (le prototype l'a retiré de `scroll`, `mouse_reader`, `ui`, `bg`, `help`, `score_panel` sans erreur). L'inclusion de la transitoire sort de `pico8_ui.c`.

### Budget : mesure, projection, décision

- **Mesure par prototype** à 5c3ea7e (et à 88d9567, mêmes écarts), sur une copie `git archive` hors dépôt. Les 26 e2e ont été rejoués avec `PICO8_REMPLISSAGE` :
  - les 26 `*.sha256` sont identiques ; `sanitize-net` répond « 26 propre(s) » ; `make test` est vert une fois réécrits les TU de « Testing Decisions » ;
  - **chargement : 22 752 o**, inchangé et constant ;
  - **génération : +2 696 o par partie**. Ce sont les singletons de `eu` (`qb` 192, `nf` 152, `ui` 592, `bg` 152, `pj` 168 = 1 256 o) et les boîtes de `bv` des 90 planètes (90 × 16 = 1 440 o). S'y ajoutent **+2 048 o** quand une sonde est posée (son arène), et **+1 224 o** par `_init` (`qb`, `nf`, `ui`, `fv` 288) dans la génération qu'il trouve ;
  - **`max_octets partie` : 157 704 o** sur `pause_restart` (151 088 + 2 × 2 696 + 1 224 : deux parties et un titre au clic) ; `trade_rich_mine` 110 272 o ; `game_end_bankrupt` 96 928 o.
- **Comparaison avec 4a** (≈ 4 Ko par génération, ~12 Ko au pic) : 6,5 Ko mesurés au pic. Le rapport mesuré / estimé est inférieur à 1.
- **Projection.** 4h est la dernière famille : `P_4h = M_4h` = 157 704 o ≈ 154 Ko ≤ 704 ; `Q_4h` = 22,2 Ko ≤ 64 ; somme des zones inchangée, 1487 Ko sur 1536. **On continue, et l'étape 4 se ferme sans relèvement.**
- **Mesure à refaire** après conversion : `M_4h`, `K_4h`, le surcoût par partie, et la génération de `pause_restart` à f13 contre le terme exact (voir Testing Decisions). Les noter dans le commit et dans « Découvert », comparés au prototype ; un écart s'explique avant la décision.
- **Décision**, règle de 4c, dans l'ordre :
  1. si `P_4h ≤ 704` et `Q_4h ≤ 64`, on continue ;
  2. sinon, déplacer la frontière entre génération et chargement ;
  3. sinon, relever d'un chiffre sur les 49 Ko ;
  4. sinon, **arrêt de l'étape 4 à 4h** et signalement à l'humain (zone, limite voulue, somme actuelle).
- **Lire un `pico8_fatal`.**
  - « zone partie pleine » : d'abord un site de 4h entré par tick ou par action (une boîte refaite, une sonde hors de l'arène), puis une limite trop basse.
  - « prises libres pleines » : d'abord une sonde refusée qui ne rend pas son arène, ou qui la rend deux fois.

### Note « Découvert », fin de l'étape 4 et clôture

À la clôture, sous l'entrée 4 de `docs/plan-4.md`, une note « *Découvert (4h) :* » de quelques phrases :

- placement :
  - boîtes par tick réécrites en place (`mo`, `je`, `ok.qa`, boîtes de `bv` dans le composant sonde, `ra` du point souris copié) ;
  - `bg.mq` valeur ; calques `ic` en zone frame ;
  - `mo`/`je`/`ok`/`bg`/`ju`/`fv` en génération ;
  - sonde dans l'arène de prise ;
  - `free(g_ok)` supprimé ;
- les gardiens trouvés par lecture et absents de la table de 4a : `bg.mq` garde `je.ra`, et `na` écrit en place le `ra` qu'un `qh` posé a reçu de `bv` ;
- les TU réécrits : égalités d'adresse `ra == mp` devenues égalités de valeur (`jc`, `g_`, famille `qh`), `bg->mq` (valeur), égalités exactes de la génération (terme singletons) ;
- les mesures et la décision : `M_4h`, `K_4h`, le surcoût par partie, le `ga` à 576 o sur 640 ; les comptes du verrou.

**Fin de l'étape 4.** Les deux critères de l'entrée 4 sont constatés et écrits dans la note :
- `pause_restart` passe `make sanitize-net` avec tous les singletons en zone ;
- aucune fenêtre sans action des 26 e2e ne fait monter la génération (le TU de « Testing Decisions »).

Le §2 est corrigé dans le même commit :
- ligne « Partie » : ajouter « singletons de `jl`/`eu` (`mo`, `je`, `ok`, `bg`, `ju`) et l'aide `fv` » et « la sonde tenue » dans l'arène de prise ;
- ligne « Frame » : ajouter « calques `ic` de `ok` ».

Sous l'entrée 5, une ligne « *Reçu de l'étape 4 :* » pour l'étape 5 :
- la liste du verrou ne contient plus que `pico8_zone_transitoire.c 3`, et aucun fichier du cœur ne référence plus la transitoire ;
- les `/* TROU-7 */` qui restent sont ceux des autres familles (`pico8_jo_lz`, `pico8_jo_ldown`, `pico8_jo_fe`, `pico8_nr_ldown`, `pico8_jo_cn`, la boucle de `planet_trade`) : à relire un par un avant de retirer TROU-7.

La PRD est retirée de `docs/prd/` dans le commit de clôture.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 :

- `make test-all` et `make sanitize-net` passent ;
- les 26 `*.sha256` sont identiques ;
- `alloc-guard` est à égalité stricte.

Aucun morceau ne passe la transitoire à un nouvel appelant. Les comptes partent de 5c3ea7e (8 / 20) et sont des prévisions, car le verrou compte des relocations.

1. **Boîtes par tick réécrites en place, calques en zone frame.**
   - Les tests d'abord :
     - les boîtes `mo.p`, `mo.v`, `mo.ra`, `je.ra`, `ok.qa` gardent leur adresse entre deux frames (rouge aujourd'hui : 0 / 5 stables) ;
     - le TU des fenêtres sans action sur les 26 e2e (vert aujourd'hui : c'est la garde) ;
     - la réécriture des TU d'identité (`ra == mp` → valeur, `bg->mq`).
   - `qb` : trois boîtes propres et une fonction d'écriture de `v`. `nf` : `ra_own`. `bg.mq` : valeur. `ok.qa` : `qa_own`. Calques `ic` en zone frame, à la construction et à chaque `gw`. Boîtes de `bv` dans le composant sonde. `point_ra` dans `jc`.
   - Le verrou : `scroll` 4 → 1, `mouse_reader` 2 → 1, `ui` 5 → 2, `bg` 2 → 1, `probe` 2 → 1, **8 / 20 → 8 / 11**.
2. **Singletons en génération.**
   - Les tests d'abord :
     - terme « singletons » dans l'égalité exacte de la génération (rouge aujourd'hui) ;
     - `mo`, `je`, `ok`, `bg`, `ju` de la partie quittée relus pendant le clic, puis enfant mort par ASan après (rouge aujourd'hui : l'ancien `mo` se relit à f13) ;
     - l'aide `fv` du titre, même forme.
   - `qb`, `nf`, `ui`, `bg`, `pj`, `fv` en génération ; `pico8_ui_shutdown` sans lecture ni `free` ; contrats réécrits.
   - Le verrou : `scroll`, `mouse_reader`, `ui`, `bg`, `help`, `score_panel` sortent, **8 / 11 → 2 / 4**.
3. **Sonde dans l'arène de prise.**
   - Les tests d'abord, sur une fixture de sondes refusées :
     - des refus à la suite ne font pas grandir la génération, et chaque sonde reprend l'arène de la précédente (rouge aujourd'hui : chaque sonde a une adresse neuve) ;
     - le `fo` d'une planète hors écran est coupé au vidage qui rend la sonde refusée au-dessus d'elle ;
     - une sonde refusée traverse une nouvelle partie.
   - `pico8_g__struct_new` ouvre la prise, garde l'arène dans `prise`, la ferme ; le `rendre` de `g_` la rend si la sonde n'est pas posée ; contrats de `pico8_probe.h` réécrits.
   - Le verrou : `probe` sort, **2 / 4 → 1 / 3**.
4. **Mesure et clôture de 4h et de l'étape 4.**
   - Le relevé sur les 26 e2e, `M_4h`, `K_4h`, la comparaison au prototype et la décision, dans le message du commit.
   - L'inventaire refait par `grep` (les trois motifs, plus celui des arguments gardés).
   - La note « Découvert (4h) », la fin de l'étape 4, la correction du §2, la ligne « *Reçu de l'étape 4* » pour l'étape 5, le retrait de cette PRD.

Ordre et dépendances :
- les morceaux 2 et 3 ne dépendent pas l'un de l'autre ;
- tous deux viennent après le 1. Le 1 retire les gardiens de `ok.qa` et `je.ra`, ce qui rend la mise en génération de `ok` et la sonde recyclée sûres à lire ;
- le 4 dépend des trois ;
- **aucun ne commence avant la revérification de « À revérifier une fois 4g finie ».**

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris de la génération et du chargement (`pico8_zone_partie_prise`, les lignes de `PICO8_REMPLISSAGE`) ;
  - les accesseurs publics (`pico8_mo_qb`, `pico8_je_nf`, `pico8_ok`, `pico8_ju`, `pico8_cg`, `pico8_hv`, les champs d'un objet) ;
  - un rapport ASan dans un processus enfant ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne lit les adresses que pour montrer qu'une boîte est la même d'une frame à l'autre, ou pour relire un objet après sa mort. Les tailles attendues se calculent dans le TU (`sizeof`, `PICO8_ZONE_CELLS`, `PICO8_JO_PRISE_OCTETS`), jamais en dur : 2 696, 1 224 et 157 704 ne valent que sur x86-64.
- **Boîtes réécrites en place** (morceau 1, `tests/test_pico8_places_vivantes.c`, rejeu de `message_box_hold`) :
  - à f44, on prend `pico8_mo_qb()->p`, `->v`, `->ra`, `pico8_je_nf()->ra`, `pico8_ok()->qa` ;
  - à f45, les cinq pointeurs sont les mêmes ;
  - et la génération ne bouge pas.

  Le prototype l'a vu : HEAD 0 / 5 stables, prototype 5 / 5.
- **Fenêtres sans action** (morceau 1, même fichier, sur le modèle d'`invariant_sur_tous_les_e2e`, un enfant par e2e) :
  - une fenêtre est une suite d'au moins deux frames sans bouton ni D-pad, à souris immobile, la frame d'avant comprise ;
  - la génération est égale à sa deuxième frame (la première peut encore vider ce que l'action d'avant a laissé) et à sa dernière ;
  - le relevé se fait sans `relever`, dont le vidage rendrait la génération en avance.

  Le prototype en a trouvé **61** sur les 26 e2e (41 d'au moins quatre frames), aucune ne monte, à 88d9567 comme sur le prototype. Ce TU remplace la seule paire `message_box_hold` f45 = f90, qui reste.
- **Égalité exacte de la génération** (morceau 2, `case_prise_and_max` de `tests/test_pico8_zone_partie.c` ; `pause_restart_borne_les_blocs_morts_des_registres` et `pause_restart_compte_les_etats_de_bw` de `tests/test_pico8_places_vivantes.c`). Un terme « singletons » s'ajoute aux états de `bw`, registres, planètes, recherche et économie : un bloc pour `qb`, `nf`, `ui`, `pj`, et pour le seul `bg` de `hv` (asserté unique).
  - Le prototype a vu ce terme exact dans les trois cas, à 88d9567 et à 5c3ea7e.
  - Le terme « planètes » n'a pas à changer : il compte `sizeof(pico8_jo_t)`, qui porte les 16 o du composant sonde.
- **Singletons de la partie quittée** (morceau 2, sur le modèle de `pause_restart_garde_l_economie_de_la_partie_quittee_pendant_le_clic` et `pause_restart_rend_l_economie_de_la_partie_quittee`) :
  - `mo`, `je`, `ok`, `bg`, `ju`, pris à f11, se relisent à la fin de f12 (clic « challenging », bascule) ;
  - l'enfant meurt par ASan (« use-after-poison ») quand ils sont relus à f13.

  Le prototype l'a vu : `ok` se relit à f12, et la relecture de `mo` à f13 meurt par « use-after-poison ». Le second cas est rouge aujourd'hui : l'ancien `mo` se relit à f13. Même forme pour l'aide `fv` du titre de f7.
- **Sondes refusées** (morceau 3). Une fixture `tests/fixtures/sondes_refusees.json`, dans la forme de `prises_refusees.json` et `pliages.json` :
  - clic « reasonable » (60,74) ;
  - des cycles « pression 4 frames, relâché 3 frames » hors du panneau `hw`, en (5,100) : la sonde y est refusée (`et` = le `{}` jetable) ;
  - un cycle relâché au-dessus d'une planète ;
  - une sonde posée en (55,50).

  Le prototype a joué la partie hors panneau (dix refus) : la génération reste à la même valeur du premier au dixième, une sonde vit et meurt à chaque cycle, et l'enfant ASan+UBSan est propre. Les cas :
  - *refus sans croissance* : génération égale après le premier et le dernier refus ;
  - *reprise d'arène* : la sonde du cycle k+1 a l'adresse de celle du cycle k. Comme `une_prise_posee_reprend_l_arene_du_mz_mort`, c'est la façon de montrer le recyclage ;
  - *`fo` coupé* : sur le modèle de `le_fo_d_une_planete_hors_ecran_oublie_la_prise_refusee`, la planète visée est poussée hors écran, et son `fo` est NULL au vidage qui rend la sonde ;
  - *nouvelle partie* : sur le modèle d'`un_mz_refuse_traverse_une_nouvelle_partie`.
- **TU d'identité à réécrire** (morceau 1), relevés par le prototype. Le cart garde la boîte de `mp`, le port la copie : l'assertion devient « autre adresse, même valeur ».
  - `test_pico8_jc` (`test_hq_mouse_point` : `mouse_point_ra(r) == m`) ;
  - `test_pico8_g_` (`fold_hq`) ;
  - `test_pico8_qh` (`fold_held`), `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_mx`, `test_pico8_nr` (`test_life`), `test_pico8_fr` (`place`) ;
  - `test_pico8_bg` : `mq` devient une valeur, et les empreintes (`DRAG_HASH`) sont inchangées.

  `test_mouse_point_is_a_struct` garde son cas `mp` nil → `ra` nil : c'est pourquoi la copie laisse `ra` à NULL quand `mp` l'est.
- **Inchangés** et verts dans le prototype :
  - `message_box_hold` f45 = f90, `game_end_final_score` f2 = f51 ;
  - les cas de prises, slipways et `mz` de 4d–4f, les cas d'économie et d'icônes de 4g, la sentinelle de 4a ;
  - `test_pico8_qb`, `test_pico8_nf`, `test_pico8_mo`, `test_pico8_je`, `test_pico8_ui`, `test_pico8_fv`, `test_pico8_pj`.
- **Verrou** : `make alloc-guard` à égalité stricte avec la liste de chaque morceau.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. Trois e2e glissent la caméra au bouton droit (`camera_drag_dpad`, `colony_income_offscreen`, `tech_starbirth_protostar`) : ils gardent `bg.mq` honnête.
- **Exemples dans le dépôt** :
  - `tests/test_pico8_places_vivantes.c` : rejeu par enfant, `dans_un_enfant_qui_meurt_par_asan`, cas « partie quittée » de 4d et 4g, cas de prises de 4d–4f, `invariant_sur_tous_les_e2e` ;
  - `tests/test_pico8_zone_partie.c` : `case_prise_and_max`.

## Out of Scope

- L'étape 5 : supprimer `pico8_zone_transitoire.c`, retirer TROU-7 de `docs/trous-de-portage.md`, du `README` et des sources, dont les `/* TROU-7 */` d'autres familles (`planet`, `planet_display`, `planet_family_trade`, `planet_trade`). 4h ne retire que les `/* TROU-7 */` et contrats devenus faux sur ses propres sites : `scroll`, `mouse_reader`, `bg`, `ui`, `probe`, `probe_comp`, `score_panel`, `entity` (`mo` « dropped »), et `cart` devant `pj_new`.
- La recherche, l'économie, les icônes : 4g.
- L'arène de prise elle-même, sa taille, sa liste de prises libres : 4d–4f. 4h n'y ajoute qu'une sorte (la sonde).
- Réduire `PICO8_ZONE_GAME_BYTES`, très au-dessus de la mesure (154 Ko sur 704) : c'est une économie PSX (§7 du plan), pas un relèvement.
- Ranger les boîtes de `bv` ailleurs que dans le composant sonde : écarté (voir Implementation Decisions).
- Un TU de fuites LSan sur les rejeux. Le prototype montre qu'après 4h, les 26 e2e rejoués ne laissent fuir **aucun** bloc du cœur (seul `pico8_input_load` de l'outil de mesure) ; c'est le terrain de l'étape 5.
- L'audio : hors périmètre du portage.
- Lancer l'application raylib.

## Hypothèses sur l'étape 4g

Chacune est ce que cette PRD suppose de 4g, lue dans sa PRD (`docs/prd/next/phase1-etape4g-recherche-et-economie.md` à 88d9567). Pour chacune : ce qui change ici si elle tombe, et son statut, constaté à 5c3ea7e (clôture de 4g, survenue pendant la rédaction).

- **H-4g-1 (verrou et fichiers).** 4g ne touche que `research`, `research_icon`, `economy`, `cart` et le `fm_free` de `probe`. Après 4g : **8 fichiers, 20 appels**, et les 17 relocations de 4h restent dans les fonctions du tableau ci-dessus. *Si elle tombe* : 4h repart de la mesure à son `HEAD` et retire 17 appels dans les fichiers qui les portent. *Statut à 5c3ea7e* : **tenue** (« allouent 8 appels 20 », liste exacte : `bg` 2, `help` 1, `mouse_reader` 2, `probe` 2, `score_panel` 1, `scroll` 4, `ui` 5, `zone_transitoire` 3).
- **H-4g-2 (`fm_free` supprimé, arrêt sans lecture).** `pico8_probe_shutdown` et `pico8_probe_set_fm` ne lisent ni ne libèrent plus le `hy`. Après la remise à zéro, plus rien de `probe` ne lit la génération : seul `pico8_ui_shutdown` reste à corriger, et c'est 4h. *Si elle tombe* : `fm_free` s'ajoute aux sites de 4h, avec la même correction que `g_ok`. *Statut* : **tenue** (f16f654, « fm_free supprimé »).
- **H-4g-3 (budget).** `M_4g` = 151 088 o (`pause_restart`), `K_4g` = 22 752 o, `r` ≤ 1, somme des zones 1487 Ko, décision « on continue ». *Si elle tombe* : `M_4h = M_4g + 2 × 2 696 + 1 224` au clic de `pause_restart`, et la décision se refait ; seul un `M_4g` au-delà de ~690 Ko la changerait. *Statut* : **tenue** (note « Découvert (4g) » ; le prototype mesure 151 088 → 157 704).
- **H-4g-4 (arène de prise).** `pico8_jo_prise_ouvrir`/`_fermer`, `pico8_jo_zone_construction` et `pico8_jo_prise_arene_rendue` gardent leur forme. `PICO8_JO_PRISE_OCTETS` vaut 2048 et `PRISES_LIBRES_MAX` 8. 4g n'y ajoute que l'identifiant « s… » d'une icône. *Si elle tombe* : la sonde suit la prise là où elle s'ouvre. *Statut* : **tenue** (29bf950 ne touche pas `pico8_planet.c`).
- **H-4g-5 (outillage de test).** Les égalités exactes de la génération (`case_prise_and_max`, `pause_restart_borne_les_blocs_morts_des_registres`, `pause_restart_compte_les_etats_de_bw`) ont un terme « recherche et économie » (`octets_de_la_recherche_et_de_l_economie`). Le terme « singletons » s'y ajoute. `demarrer`, `jouer_jusqu_a` et `dans_un_enfant_qui_meurt_par_asan` existent sous ces formes. *Si elle tombe* : on ajoute le terme à la forme que 4g a laissée. *Statut* : **tenue** (le prototype y a ajouté le terme, vert).
- **H-4g-6 (inventaire).** 4g n'ajoute ni pointeur `static` qui puisse viser un objet de 4h, ni entrée au point d'oubli de partie ou à `oublier`. *Si elle tombe* : le nouveau pointeur est classé avec la question de l'inventaire. *Statut* : **à refaire par `grep`** au morceau de clôture. La lecture des diffs de 4g n'en montre pas : 29bf950 ne touche que `cart`, `research_icon` et des TU.
- **H-4g-7 (« Reçu »).** La clôture de 4g n'écrit de « *Reçu de l'étape 4g* » que pour 4h, et seulement ce que 4h sait déjà (`probe` n'a plus que `pico8_bv` et `pico8_g__struct_new` ; `g_fm` désigne l'ancien `hy` jusqu'à `eu`). *Si elle tombe* : cette PRD traite le nouveau « Reçu » ou dit pourquoi non. *Statut* : **tenue**. Le « Reçu » de 5c3ea7e est celui-là. Il ne demande rien à 4h : `g_fm` n'est lu ni par `ui` ni par la sonde après la remise à zéro.
- **H-4g-8 (`cart`).** 4g retire de `pico8_eu` la transitoire de `ie.jb` sans déplacer `jl()`, `bg()`, `pj()` ni la bascule. Le `/* TROU-7 */` devant `set_fm`/`pj_new` subsiste, et 4h le retire (`pj` en génération). *Si elle tombe* : relire l'ordre de `eu` et refaire le point « Singletons en génération ». *Statut* : **tenue** (diff de `pico8_cart.c` : 4 lignes, la bulle seule).

## À revérifier une fois 4g finie

4g est close à 5c3ea7e, et le prototype y a été refait. Il reste à relire, à `HEAD` au moment d'implémenter, sans toucher git ni les sources :

- que `HEAD` n'a pas bougé depuis 5c3ea7e sur les fichiers de 4h (`git log --stat 5c3ea7e..HEAD`) ; sinon, refaire tout ce qui suit ;
- la note « *Découvert (4g)* » et la ligne « *Reçu de l'étape 4g* » de `docs/plan-4.md` (H-4g-3, H-4g-7) ;
- `make alloc-guard` sur une copie `git archive HEAD`, et le rattachement des 17 relocations par `objdump -r`/`addr2line` (H-4g-1) ;
- `pico8_probe.c` : pas de `fm_free`, `pico8_probe_shutdown` sans lecture (H-4g-2) ;
- `pico8_planet.{h,c}` : la forme de la prise (H-4g-4) ;
- les égalités exactes et leurs aides dans `tests/test_pico8_places_vivantes.c` et `tests/test_pico8_zone_partie.c` (H-4g-5) ;
- `pico8_cart.c` : l'ordre de `eu`, `oublier`, `oublier_partie` (H-4g-6, H-4g-8) ;
- **refaire le prototype** sur ce `HEAD` si le point 1 montre un écart : `sha256`, `sanitize-net`, `make test` (pour lister les TU à réécrire), `alloc-guard`, `max_octets partie` et `chargement` sur les 26 e2e, fenêtres sans action ;
- refaire l'inventaire du §4 par `grep` (H-4g-6).

## Further Notes

- **Pas d'humain disponible, et pas d'outil `Agent` dans cette session pour lancer un agent `guideline` neuf.** J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage).
  - **Hypothèses explicites.** Celles sur 4g sont numérotées (H-4g-1 à H-4g-8), confrontées à 5c3ea7e et listées à revérifier. Les gardiens ont été lus dans le cart :
    - `bv` l. 552, `jc:hq` l. 622, `bg:rdown`/`hq` l. 378-382, `na` l. 577-578 ;
    - `jo:hk` l. 515-516 (`iv` emprunte `ra`), `mz:pq`/`hq`/`kj` l. 764-792 ;
    - `ui:lj`/`gw` l. 217-236, `qb:ph` l. 1137-1141, `nf:ph` l. 203.

    Ils l'ont aussi été dans le port, par le `grep` des écritures `= ra|qa|mp|rp;`. Deux gardiens manquaient à la table de 4a : `bg.mq` garde `je.ra`, et `na` modifie en place le `ra` reçu de `bv`.
  - **Simplicité.** Aucun mécanisme neuf : une boîte propre là où le cart rebranche un champ, la génération de 4a, l'arène de prise de 4d–4f, la zone frame de l'étape 1. Écartés :
    - la zone frame pour `ok.qa`/`je.ra` (le §2 l'interdit pour une boîte de champ, et `bg.mq`/les hôtes la gardent) ;
    - une place recyclée propre à la sonde (l'arène de prise existe) ;
    - des boîtes de `bv` par sorte d'hôte (trois sites au lieu d'un) ;
    - des calques `ic` vidés en place dans la génération (des blocs morts suivraient l'historique du survol).
  - **Changements chirurgicaux.**
    - Dans le cœur : 17 relocations dans 7 fichiers, plus le point souris de `pico8_slipway.c` ;
    - formes : six champs valeur (`p_own`, `v_own`, `ra_own`, `ra_own`, `qa_own`, `point_ra`), `mq` pointeur → valeur, deux boîtes dans le composant sonde, un champ `prise` ;
    - une fonction ajoutée (`pico8_qb_set_v`).

    Le diff du prototype fait 127 lignes `±` dans `src/` et 78 dans `tests/`.
  - **Critères vérifiables.** Chaque morceau a ses tests rouges d'abord. Le prototype a montré :
    - sha256 26 / 26, `sanitize-net` 26 propres ;
    - verrou 1 / 3 ;
    - génération +2 696 par partie et terme exact ;
    - 61 fenêtres sans action, 0 montée ;
    - boîtes stables 5 / 5 ;
    - singleton quitté mort par ASan à f13.
- **Le prototype** (copies `git archive` de 88d9567 puis de 5c3ea7e, hors dépôt) a aussi refait les TU cassés, pour savoir lesquels le seraient : neuf TU d'identité de `ra`, `test_pico8_bg` (`mq`), et les deux fichiers d'égalité exacte. Rien de ce prototype n'est dans le dépôt.
- **Rejeux sous LSan, pour mémoire.** À 5c3ea7e, les 26 e2e rejoués d'affilée font fuir 227 224 o en 10 155 allocations. 89 856 o viennent de l'outil de mesure (`pico8_input_load`, hôte). Les ~137 Ko restants sont tous des sites de 4h : `qb:ph` 5 535 boîtes, `nf:ph` 1 845, `ok:lj` 1 835, `bv` 600, plus les structures. Sur le prototype : 0 bloc du cœur.
