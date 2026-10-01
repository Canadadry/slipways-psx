---
title: "Plan 4, étape 4g — Recherche en sous-zone chargement, économie et icônes en génération"
description: "Les projets de recherche gb (avec leurs listes hp.i), l'économie fm, les sept icônes ie et leurs bulles sont encore pris à la libc ou à la transitoire ; fm est libéré par free au eu suivant et gb ne rend jamais hp.i. 4g range gb et hp.i dans la sous-zone chargement, fm, les icônes, leurs bulles et leurs boîtes ie.gy dans la génération, l'identifiant « s… » d'une icône prise dans son arène de prise, et supprime le free de fm."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. L'étape 4 range les objets de partie dans la **zone partie**. Cette PRD traite la sous-étape **4g, recherche et économie**.

**Elle a été rédigée pendant 4f, puis revérifiée contre 4f finie** (section « Vérifié contre 4f ») : les sept hypothèses `H-4f-n` du brouillon tiennent toutes, et le prototype a été refait à `HEAD` 88d9567. Tout a été lu sur le contenu commité (`git archive HEAD`), jamais sur l'arbre de travail. Les fichiers de 4g n'ont pas bougé depuis 4093382, où le brouillon avait été écrit : les lignes citées valent aux deux.

**Le texte de l'étape**, recopié du plan à 88d9567 (entrée 4, extraits qui concernent 4g) :

> 4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement, et si son remplissage ne monte pas pendant une phase sans action d'un e2e.
>    - **4b et suivantes.** Une famille de propriétaires par PRD dans l'ordre fixé par la PRD 4a : […] 4g recherche et économie ; 4h singletons de `jl`/`eu` et objets du joueur […]. N'entre en zone partie qu'une allocation faite une fois par objet de partie ; un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
>    - *Découvert (4c) :* la zone partie se partage en génération (704 Ko, deux bouts) et chargement (64 Ko, remis à zéro au seul chargement), somme toujours 768 Ko ; `g_shared_le` et `g_ga_by_shared` vont en chargement (0 o mesuré : rien ne les fait grandir en jeu). […] Piège : la remise à zéro faisait relire `hv` par le `jl()` suivant — `cart_load`/`shutdown` abandonnent `hv` avant le vidage et oublient les registres sans les lire après. […] projection P ≈ 33 + 209 × 1,52 ≈ 351 Ko ≤ 704, Q ≈ 49 Ko ≤ 64 : on continue.
>    - *Reçu de l'étape 4c :* pour 4g, la sous-zone chargement existe (`pico8_zone_chargement()`, 64 Ko, `chargement`/`max_octets chargement` écrits par le rejeu) : `gb` et `hp.i` y entrent sans la recréer.
>    - *Découvert (4d) :* […] Une prise d'icône refusée faisait grandir la génération (+616 o) : elle prend une arène de 1536 o recyclée par une liste de 4 prises libres. […]
>    - *Reçu de l'étape 4d :* […] Pour 4g, l'identifiant « s… » de `ie:ldown` est fait à chaque prise : il va dans l'arène de prise.
>    - *Découvert (4e) :* […] un `jc` est fait à chaque pression sur une planète (pas une fois par slipway) : il prend l'arène de prise de 4d, portée à 2048 o, et la rend s'il est refusé ; […] Mesure : maximum 130 720 o inchangé, `trade_rich_mine` 95 312 o, au plus 13 traînées ; projection ≈ 147 Ko ≤ 704 : on continue. Comptes : 16 / 47 → 13 / 36.
>    - *Découvert (4f) :* un trou de ver `mz` naît à chaque pression dans le vide (pas une fois par partie) : il prend l'arène de prise et la rend à sa mort, quel que soit son état ; son `ox` est réservé à sa taille exacte (`pico8_ents_init_reserve`). Liste des prises libres 4 → 8. Rien de nouveau à oublier (`dw = 0` : un `mz` n'accroche jamais `fo`). Mesure : maximum 130 720 o inchangé, chargement 0. Comptes : 13 / 36 → 12 / 33.
>    - *Reçu de l'étape 1 :* même chose pour les boîtes `vec` des objets de partie (`p`/`v`/`ra` de `qb`, `ra` de `nf`, `mo->v`, `re`/`p` de `jc:hq`, `p` de `pico8_bv`, `ie.gy`) ; […]
>    - *Reçu de l'étape 2 :* restent à cette étape `ok.qa` […], la structure `ok` et ses calques `ic`, la liste statique de départ, `ie.jb` (fait une fois par partie), `fv`, la structure `pj`, `hp.i`. […]

La seule ligne « *Reçu de l'étape 4f* » vise 4h (`ox`/`ij` d'un `mz` et les sondes `g_`, `mz.p`/`mz.ra`) : rien à traiter ici.

**La ligne « Partie » du §2**, recopiée à 88d9567 :

> | **Partie** | planètes, relais, slipways posés, routes `trade` gardées dans `by`, recherche ; dans l'arène de prise recyclée, l'objet tenu (icône, slipway, trou de ver) ; les tableaux croissants (`ents`, `ptrs`, registres) qui les contiennent ; les objets faits au chargement (`gb`, `hp.i`, listes statiques) dans une sous-zone chargement, prise dans la zone partie, remise à zéro par le seul chargement | une génération par partie, de `eu` à `eu` ; rendue d'un bloc au vidage de la file qui suit le `eu` suivant (zone à deux bouts) ; `_init` ne la rend pas |

**La ligne de 4g dans la table des familles de la PRD 4a**, retirée par b46601e (`git show b46601e~1:docs/prd/triage/phase1-etape4a-socle-zone-partie.md`), recopiée :

> | Ordre | Famille | Sites (fonction, ligne) | Relocations | Reçus traités |
> |---|---|---|---|---|
> | **4g** | **Recherche** (et économie) | `pico8_hp_struct_new` (133, 200, 201), `pico8_jy_new` (119), `pico8_research_free_gb_instances` (33), en sous-zone chargement ; `pico8_ie_struct_new` (203), `pico8_ie_hb` (100), `pico8_ie_ldown` (130) ; `pico8_eu` (`cart` 222 : `ie.jb`) ; `pico8_hy_struct_new` (`economy` 36, 62, 63) ; `fm_free` (`probe` 453) | `research` 5 → sort ; `research_icon` 3 → sort ; `cart` 1 → sort ; `economy` 3 → sort ; `probe` 3 → 2 (13) | *Reçu 1* : `ie.gy` ; *Reçu 2* : `ie.jb`, `hp.i` |

Ses deux lignes de la table « Sites qui allouent par tick ou par action », recopiées :

> | `pico8_ie_hb` (boîte de `ie.gy`) | à chaque `hb`, donc chaque `hy:lj` | 4g |
> | `pico8_ie_ldown` (identifiant « s… ») | à chaque clic d'icône | 4g |

La raison de l'ordre, recopiée de la même PRD :

> - **4g** touche le chargement et `fm`, que l'inventaire désigne comme vivant entre `_init` et `eu`.

L'estimation de 4a : « 4g recherche | `hy`, 7 `ie` et leurs `jb` ≈ 3 Ko ; chargement : ~36 `hp` × 576 o + `hp.i` ≈ 32 Ko, hors bascule | ~38 Ko ».

**Les lignes de l'inventaire du §4 de 4a qui renvoient à 4g**, recopiées :

> | `g_fm` | `probe` | **repointé** par `eu` (`pico8_probe_set_fm`), après la bascule ; entre `_init` et `eu` il désigne l'ancien `hy`, encore vivant (triche, `fm_free`) | 4g |
> | `g_gb[]` | `research` | ne doit pas viser la génération : les `hp` sont faits au chargement et vivent d'une partie à l'autre (sous-zone chargement) | 4g |
> | `g_ie_gy` | `research_icon` | **repointé** par `_init` sur `ie.eh` (statique), puis avancé par les `ie` d'`eu` sur des boîtes de la nouvelle génération | 4g |
> | `fm.invented`, `fm.po`/`.fu` | champs du nouvel `hy` | 4g |

**La règle des familles**, recopiée de la PRD 4a :

> - n'entre en génération de partie qu'une allocation faite une fois par objet de partie créé (planète posée, slipway, route, icône, singleton de `jl`/`eu`) ;
> - un champ réaffecté par tick ou par action doit, au choix de la PRD de sa famille et site par site :
>   - être **réécrit en place**, quand la lecture prouve qu'aucun autre objet ne garde l'ancienne valeur ;
>   - devenir une **valeur** au lieu d'une boîte, quand la boîte n'est jamais modifiée en place. […]
>   - aller en **zone frame**, s'il n'est relu que dans la frame ;
>   - prendre une **place** dans une zone qui rend.

**Ce qui se passe aujourd'hui** (lu à 88d9567, cart et port) :

- **Le verrou** : `make alloc-guard` sur une copie `git archive HEAD` répond « verrou objets 65 allouent 12 appels 33 » (`wormhole` est sorti avec 4f). Les 13 relocations de 4g, rattachées par `objdump -r` et `addr2line` sur les objets `-g` d'`alloc-guard` (les lignes de 4a ont bougé) :

  | Fichier | Fonction (ligne à 88d9567) | Appel |
  |---|---|---|
  | `research` | `pico8_research_free_gb_instances` (33) | `free` d'un `hp` |
  | `research` | `pico8_jy_new` (119) | `calloc` d'un besoin `jy` |
  | `research` | `pico8_hp_struct_new` (133, 200, 201) | `malloc` du `hp`, `malloc` du conteneur `i`, transitoire pour ses éléments |
  | `research_icon` | `pico8_ie_hb` (101) | `malloc` de la boîte de `ie.gy` |
  | `research_icon` | `pico8_ie_ldown` (131) | transitoire de `pico8_concat2` pour « s… » |
  | `research_icon` | `pico8_ie_struct_new` (206) | `malloc` de l'icône |
  | `cart` | `pico8_eu` (243) | transitoire passée à `pico8_jb_new` (`ie.jb`) |
  | `economy` | `pico8_hy_struct_new` (36, 62, 63) | `malloc` du `hy`, `malloc` du conteneur `invented`, transitoire pour ses éléments |
  | `probe` | `fm_free` (453) | `free` de l'ancien `hy` |

  Restent dans `probe` les deux sites de 4h : `pico8_bv` (162) et `pico8_g__struct_new` (413).
- **Au chargement** (`pico8_cart_load` → `pico8_data_init` → `pico8_research_init`, `pico8_research_build_gb`) : 34 projets `hp` (`pico8_hp_t`, 576 o), chacun avec sa liste `i` (`pico8_ptrs_t`, un besoin `jy` de 48 o par caractère de sa chaîne `i`). `free_gb_instances` ne libère que les structures : `hp.i` et ses `jy` fuient (TROU-7). Rien d'autre n'appelle `pico8_hp_new` en jeu ; seuls des TU en font d'autres.
- **Les besoins `jy`** : `pico8_jy_new` a trois appelants. `hp:pq` (chargement) ; `economy_tiers` (`bb`, quand `mi` d'une planète quitte 1 : une fois par planète, `mi` ne fait que monter) ; `planet_trade` (sous le verrou `has_jh` : une fois par planète). `jo:lz` copie les `jy` de `hp.i` par valeur (`pico8_jo_lz_mw_step`, 4d) : un `jy` de `hp.i` n'est jamais écrit après le chargement.
- **À chaque `eu`** (cart l. 1182-1187) : `fm,ju=hy(),pj()`, puis sept `ie({ke=b,jb=jb(dr[i])})`, puis `fm:lj()`. `pico8_probe_set_fm` passe l'ancien `hy` à `fm_free`, qui lit son `cls` et le libère : à ce moment, l'ancien `hy` est dans l'ancienne génération, encore vivante jusqu'au vidage qui suit. `hy.invented` grandit à chaque invention (`hy:kt`), jamais rendu.
- **`ie:hb`** (cart l. 926-930) : `if not self.kh and oj(self.ke.rr)` ; il ne fait sa boîte qu'**une fois par icône**, `kh` reste vrai et n'est remis par personne (`grep` : seul `research_icon` écrit `kh`). Au plus sept boîtes par partie. La table de 4a disait « à chaque `hb` » : c'est faux. `qh` et `os` n'ont pas de `rr`, donc `oj` répond vrai et ils se révèlent dès le `fm:lj()` d'`eu`.
- **`ie:ldown`** (cart l. 932-935) : `ie.id+=1`, puis `self.ke({id="s"..ie.id})`. L'identifiant est fait **avant** `pico8_jo_prise_ouvrir()`, par la transitoire : il fuit à la libc à chaque prise, refusée ou posée. La planète tenue garde ce pointeur comme `id`.
- **À l'arrêt et au rechargement** : `pico8_cart_load` et `pico8_cart_shutdown` font `pico8_zone_partie_reset()` (génération et chargement) **avant** `pico8_probe_init`/`_shutdown` et `pico8_data_init`/`_shutdown`. Aujourd'hui sans effet, puisque rien de 4g n'est en zone.

**Le problème.** Treize relocations restent au verrou ; `hp.i`, `jy`, `invented`, les icônes, leurs bulles et leurs boîtes fuient (TROU-7) ; l'identifiant « s… » fuit à chaque prise. Si on se contente de remplacer `malloc` par la zone, `fm_free` lirait le `cls` d'un `hy` dans une zone remise à zéro (arrêt, rechargement : `probe_shutdown` passe après le reset), et un `free` sur un bloc de zone.

## Solution

- **Sous-zone chargement** : chaque `hp`, son conteneur `i` et ses besoins `jy` y sont pris. `i` est **réservée exactement** à la longueur de sa chaîne (`pico8_ptrs_init_reserve`), sauf une chaîne vide, laissée sans éléments (`pico8_ents_init_reserve` de 4f, qui accepte 0, ne convient pas : `i` est une `pico8_ptrs_t` de besoins, pas une liste d'entités). `free_gb_instances` ne libère plus rien : il oublie `gb`. Mesuré par prototype : **K_4g = 22 752 o** sur 65 536.
- **`pico8_jy_new` prend une zone.** `hp:pq` passe le chargement ; `economy_tiers` et `planet_trade` passent la génération (une fois par planète, règle des familles tenue).
- **Génération** : la structure `hy` et son conteneur `invented` (agrandi dans la génération à chaque invention, borné par le nombre de techs) ; chaque `ie` ; la bulle `ie.jb` de chaque icône ; la boîte qu'une icône avance sur `ie.gy`, une fois par icône. Mesuré : **+10 184 o par partie**.
- **L'identifiant « s… » dans l'arène de prise** : `ie:ldown` ouvre la prise, puis fait l'identifiant dans `pico8_jo_zone_construction()`, puis la structure. Une prise refusée rend son identifiant avec son arène ; une prise posée le garde avec la sienne.
- **Plus de `free` de `fm`** : `pico8_probe_set_fm` et `pico8_probe_shutdown` ne font que rebrancher ; l'ancien `hy` meurt avec sa génération, que le vidage qui suit `eu` rend.
- **Inventaire du §4** refait : rien à ajouter au point d'oubli de partie ni à `oublier`.
- **Budget** : `M_4g` = **151 088 o** (`pause_restart`, au clic), `K_4g` = 22 752 o ; projection ≈ 188 Ko au pire ≤ 704, chargement 22,2 Ko ≤ 64 : **on continue**.
- **Verrou** : `research`, `research_icon`, `cart`, `economy` sortent, `probe` 3 → 2. Mesuré par le prototype à 88d9567 : **12 / 33 → 8 / 20**.

Aucun pixel ne change : le prototype garde les 26 `*.sha256`, `make test` est vert une fois les TU de « Testing Decisions » réécrits, et `make sanitize-net` répond « 26 propre(s) ».

## User Stories

1. En tant que mainteneur du plan 4, je veux que `pico8_research.c`, `pico8_research_icon.c`, `pico8_cart.c` et `pico8_economy.c` n'appellent plus ni la libc ni la transitoire, pour qu'ils sortent de la liste d'exceptions du verrou.
2. En tant que mainteneur du plan 4, je veux que `pico8_probe.c` n'ait plus que ses deux sites de 4h, pour que 4h n'y trouve que sa famille.
3. En tant que développeur, je veux que les projets de `gb` soient faits dans la sous-zone chargement, pour qu'ils vivent d'une partie à l'autre sans jamais être libérés.
4. En tant que développeur, je veux que la liste `i` d'un projet et ses besoins `jy` vivent dans la sous-zone chargement avec leur projet, pour que `hp.i` ne fuie plus.
5. En tant que développeur, je veux que la liste `i` soit réservée exactement à la longueur de la chaîne `i`, pour qu'aucun agrandissement ne laisse de bloc mort dans le chargement.
6. En tant que développeur, je veux qu'une chaîne `i` vide donne une liste sans éléments, pour respecter le contrat de `pico8_ptrs_init_reserve` (capacité strictement positive).
7. En tant que développeur, je veux que `pico8_jy_new` prenne la zone de son appelant, pour qu'un besoin de projet aille au chargement et un besoin de planète à la génération.
8. En tant que développeur, je veux que `free_gb_instances` oublie `gb` sans rien libérer, pour qu'un rechargement reparte de `gb` vide sans toucher une zone remise à zéro.
9. En tant que développeur, je veux que l'économie `fm` et sa liste `invented` soient faites dans la génération de la partie, pour qu'elles meurent avec elle.
10. En tant que développeur, je veux que `pico8_probe_set_fm` et `pico8_probe_shutdown` ne libèrent plus l'ancien `hy`, pour qu'aucun code ne lise le `cls` d'un `hy` dans une zone remise à zéro.
11. En tant que développeur, je veux que l'écran titre lise encore l'économie de la partie quittée (triche, panneau), pour que `fm` reste valide entre `_init` et `eu` comme dans le cart.
12. En tant que développeur, je veux que chaque icône `ie` soit faite dans la génération, pour qu'elle meure avec sa partie.
13. En tant que développeur, je veux que la bulle `ie.jb` de chaque icône soit faite dans la génération, pour qu'elle ne fuie plus à chaque partie.
14. En tant que développeur, je veux que la boîte qu'une icône avance sur `ie.gy` soit faite dans la génération, pour que l'icône et la suivante gardent leur position toute la partie.
15. En tant que développeur, je veux que l'identifiant « s… » d'une icône prise soit fait dans l'arène de prise de sa planète, pour qu'une prise refusée le rende avec elle et qu'une prise posée le garde.
16. En tant que développeur, je veux que la prise soit ouverte avant l'identifiant, pour que l'identifiant et la structure partagent l'arène.
17. En tant que mainteneur, je veux que la sous-zone chargement tienne exactement `gb` après un chargement, et la même valeur après un second chargement, pour prouver que seul le chargement la remet à zéro.
18. En tant que mainteneur, je veux que la sous-zone chargement ne bouge pas pendant une partie, pour prouver que rien du jeu n'y entre après le chargement.
19. En tant que mainteneur, je veux que l'économie de la partie quittée se relise pendant la frame du clic qui lance la nouvelle partie, et fasse planter un enfant ASan quand elle est relue après la frame suivante, pour prouver qu'elle vit dans la génération et meurt avec elle.
20. En tant que mainteneur, je veux la même preuve pour une icône de la partie quittée, pour que les icônes suivent la même durée de vie.
21. En tant que mainteneur, je veux que la génération, juste après le rendu de `pause_restart`, soit exactement « états de `bw` + registres + planètes + recherche et économie », pour qu'aucune allocation de 4g n'y entre sans être comptée.
22. En tant que mainteneur, je veux que le contrôle `message_box_hold` f45 = f90 et `game_end_final_score` f2 = f51 restent verts, pour prouver qu'aucun site de 4g n'alloue par tick ni par saut d'année.
23. En tant que mainteneur, je veux que les prises refusées ne fassent toujours pas grandir la génération, pour prouver que l'identifiant « s… » ne s'y ajoute pas.
24. En tant que gardien de l'invariant, je veux que les 26 `*.sha256` restent identiques, pour prouver que ni la zone ni l'ordre de construction dans l'arène ne changent un pixel ou un tirage `rnd`.
25. En tant que gardien de l'invariant, je veux que `make sanitize-net` reste vert sur les 26 e2e, pour que rien ne lise un `hy`, une icône ou un identifiant rendus.
26. En tant que mainteneur du plan 4, je veux que l'inventaire du §4 soit refait mécaniquement pour les trois durées de vie qui finissent (chargement remis à zéro, génération rendue, arène de prise recyclée), pour que la liste des références oubliées vienne du `grep`.
27. En tant que mainteneur du plan 4, je veux les maxima de la génération et du chargement relevés sur les 26 e2e, avec la projection et la décision, pour que le budget repose sur une mesure.
28. En tant que mainteneur du plan 4, je veux que la note « Découvert (4g) » corrige la table de 4a (la boîte de `ie.gy` est faite une fois par icône, pas à chaque `hb`) et son estimation (≈ 10 Ko par partie, pas 3), pour que 4h projette sur un modèle juste.
29. En tant que mainteneur, je veux que les contrats devenus faux (« Freed only by pico8_research_shutdown », « never freed (TROU-7) », « Nothing frees the struct », « the caller owns the struct until pico8_probe_set_fm() takes it », les `/* TROU-7 */` de ces sites) soient réécrits, et que `<stdlib.h>` et l'inclusion de la transitoire sortent des fichiers qui n'en ont plus besoin, pour que les commentaires restent vrais (règle du `CLAUDE.md`).
30. En tant qu'agent sans humain, je veux que chaque hypothèse faite sur l'issue de 4f soit marquée tenue ou tombée contre 4f finie, pour ne pas bâtir sur une conception que 4f aurait changée.

## Implementation Decisions

### Inventaire des sites de 4g, et zone retenue

Chaque site est tranché par lecture : « à quelle fréquence, et qui d'autre garde l'objet ? ».

| Site | Fréquence (lecture) | Qui garde l'objet | Zone retenue |
|---|---|---|---|
| structure `hp` | au chargement, 34 fois | `g_gb[]`, `g_gb_px[]`, `hn` des planètes, boutons et rangées de l'interface | **chargement** |
| conteneur et éléments de `hp.i` | au chargement, un par `hp` | le `hp` ; `jo:lz` en fait des copies par valeur | **chargement**, capacité réservée exacte |
| `jy` de `hp.i` | au chargement | `hp.i` | **chargement** |
| `jy` de `economy_tiers` (`bb`) | une fois par planète (`mi` quitte 1) | `jo.jy` | **génération** |
| `jy` de `planet_trade` | une fois par planète (`has_jh`) | `jo.jy` | **génération** |
| `free_gb_instances` | chargement, arrêt | — | **supprimé** : oublie `gb` |
| structure `hy` | une fois par `eu` | `g_fm` | **génération** |
| `hy.invented` (conteneur et éléments) | une fois par `eu`, puis agrandi à chaque invention (≤ nombre de techs) | le `hy` | **génération** |
| `fm_free` | à chaque `eu`, à l'arrêt | — | **supprimé** : l'ancien `hy` meurt avec sa génération |
| structure `ie` | sept par `eu` | `hv`/`cv`, `ok.o_`/`ok.gj` (survol) | **génération** |
| bulle `ie.jb` | sept par `eu` | l'`ie` | **génération** |
| boîte de `ie.gy` | **une fois par icône** (`kh`) | `g_ie_gy`, puis le `ra` de l'icône suivante | **génération** |
| identifiant « s… » | à chaque prise d'icône | la planète tenue, puis posée (`id`), un éclat `ga` né de `jo:hu` | **arène de prise** de la planète |

- **Pourquoi la génération pour la boîte de `ie.gy`.** C'est une allocation par objet de partie (une icône révélée), pas par tick. La règle la permet. En faire une **valeur** demanderait un champ de plus dans `pico8_ie_t` et un `g_ie_gy` par valeur, pour 56 o par partie : écarté (simplicité).
- **Pourquoi pas la génération pour l'identifiant.** Une prise refusée le ferait grandir à chaque clic. L'arène de prise est déjà recyclée à la mort d'une prise refusée et gardée par une prise posée : l'identifiant a exactement sa durée de vie (*Reçu de l'étape 4d*).
- **Pourquoi le chargement pour `jy` de `hp.i`** : ils sont faits par `hp:pq` au chargement et lus seulement ; `jo:lz` en copie la valeur dans la génération.

### Sous-zone chargement : `gb` et `hp.i`

- `pico8_hp_struct_new` prend sa structure, le conteneur `i` et ses `jy` dans `pico8_zone_chargement()`. Le conteneur `i` est réservé à `strlen` de la chaîne `i` par `pico8_ptrs_init_reserve` ; une chaîne vide passe par `pico8_ptrs_init` (le prototype a d'abord heurté l'`assert` « capacity > 0 » de `pico8_ptrs_init_reserve`). `pico8_ents_init_reserve` (4f) accepte 0, mais vise des listes d'entités (`pico8_ents_t`) : `hp.i` reste une `pico8_ptrs_t`. Aligner `pico8_ptrs_init_reserve` sur sa sœur (capacité 0 : rien n'est pris) est permis à l'implémenteur à la place du test, avec un cas de TU ; ce n'est pas exigé.
- Un `hp` fait par un TU (`pico8_hp_new` hors `build_gb`) prend aussi le chargement. Il ne grandit qu'avec les TU, chacun dans son enfant.
- `free_gb_instances` remet les compteurs à zéro sans rien libérer. Le nom peut changer (il ne libère plus) ; laissé à l'implémenteur.
- **Remise à zéro** : par `pico8_zone_partie_reset()` seul, dans `pico8_cart_load()`/`pico8_cart_shutdown()`, avant `pico8_data_init` qui refait `gb`. Aucune remise ajoutée.
- **Reconstruire `gb` sans recharger** (`pico8_research_init` + `build_gb` hors `pico8_cart_load`) n'arrive que dans des TU. Chaque reconstruction ajoute K au chargement. Le prototype l'a vu : `test_pico8_hp`, cas `test_rebuild_and_shutdown`, fait trois `gb` sans remise (3 × 22 752 = 68 256 > 65 536) et sort par « zone chargement pleine ». Ce cas est réécrit (voir Testing Decisions). Deux `gb` sans remise (`test_pico8_js` refait `pico8_data_init` après `pico8_cart_load`) tiennent : 45 504 o.
- **Pas de relèvement** de `PICO8_ZONE_CHARGEMENT_BYTES` ni déplacement de la frontière : K_4g = 22 752 o, et aucune famille restante ne prévoit de chargement.

### `pico8_jy_new` prend une zone

- Signature : la zone d'abord, puis `qe` (NULL : nil). Contrat : « un besoin neuf dans `zone`, jamais libéré ici ».
- Appelants : `hp:pq` → chargement ; `economy_tiers` et `planet_trade` → `pico8_zone_partie()` ; TU → la génération (voir Testing Decisions pour `test_pico8_hy`).

### Économie : `hy` en génération, plus de `free`

- `pico8_hy_struct_new` prend la structure et le conteneur `invented` dans `pico8_zone_partie()`, et `invented` grandit dans la génération (`pico8_ptrs_init` sur la génération). La croissance par doublement laisse des blocs morts bornés par le nombre de techs ; aucune réservation, `invented` ne grandit qu'à une invention.
- `fm_free` disparaît. `pico8_probe_set_fm` rebranche `g_fm` ; `pico8_probe_shutdown` le met à NULL. Personne ne lit plus le `cls` de l'ancien `hy` (l'`assert` qui le faisait disparaît avec `fm_free`).
- **Pourquoi c'est sûr** : `set_fm` est appelé dans `eu` **après** la bascule. L'ancien `hy` est alors dans l'ancienne génération, vivante jusqu'au vidage suivant ; après, `g_fm` désigne déjà le nouveau. À l'arrêt et au rechargement, la zone est remise à zéro avant `pico8_probe_shutdown` : sans `fm_free`, ce dernier ne lit plus rien.

### Icônes : structure, bulle, boîte de `ie.gy`

- `pico8_ie_struct_new` prend sa structure dans `pico8_zone_partie()`.
- `pico8_eu` passe `pico8_zone_partie()` à `pico8_jb_new` pour `ie.jb`. La bulle est mise en page une fois à sa construction (`pico8_lq_pq`) : rien ne l'agrandit ensuite.
- `pico8_ie_hb` prend sa boîte dans `pico8_zone_partie()`. Elle reste rebranchée, jamais écrite en place.

### L'identifiant « s… » dans l'arène de prise

- `pico8_ie_ldown` : après le test `ke_kind == NULL` (qui ne prend rien), **ouvrir la prise**, faire le texte du nombre en zone frame, faire l'identifiant par `pico8_concat2` dans `pico8_jo_zone_construction()`, construire la planète, fermer la prise.
- Ordre dans l'arène : l'identifiant, puis la structure. Rien ne suppose que la structure est le premier bloc de l'arène : le retour d'arène part de `oe->zone` (`pico8_jo_prise_rendue`). Le prototype le confirme (sha256, ASan, cas de prise de 4d/4e verts).
- Coût : un bloc de 8 à 16 o de plus par arène de prise, dans 2048 o.

### Inventaire du §4 pour 4g

Trois durées de vie finissent sur des objets de 4g :

- **le chargement**, remis à zéro par `pico8_cart_load`/`_shutdown` : `gb`, `hp.i`, leurs `jy` ;
- **la génération**, rendue au vidage qui suit le `eu` suivant : `hy`, `invented`, les `ie`, leurs bulles, les boîtes de `ie.gy`, les `jy` de planètes ;
- **l'arène de prise** d'une icône refusée, recyclée : son identifiant.

**Méthode.** Refaire **mécaniquement**, sur le code du commit, les deux `grep` de 4a sur les fichiers de `CORE_SRCS` (65 fichiers à 88d9567) :

- pointeurs `static` : `grep -nE '^\s*static\s[^(]*\*\s*[a-zA-Z_][a-zA-Z0-9_]*(\[[^]]*\])*\s*(;|=)'`, hors `const char *` et `* const` ;
- structures `static` dont on lit les champs pointeurs : `grep -nE '^static\s+(struct\s+)?[a-z0-9_]+(_t)?\s+[a-zA-Z_][a-zA-Z0-9_]*\s*(\[[^]]*\])*\s*(;|=)'`, hors `const` et scalaires.

**Trou du motif** : le second `grep` ne voit pas une structure anonyme (`static struct { … } g_gb_px[…]`). Ajouter `grep -nE '^static\s+struct\s*\{'` et classer ce qu'il trouve. À 88d9567, il ne trouve que `g_gb_px`.

Y ajouter le `grep` des lecteurs de `hn`, `->i`, `pico8_gb_at`/`_find`, `pico8_fm`/`pico8_fm_hy`, `pico8_ie_gy`, `->jb` d'une icône, et des copies de `id`.

**Relevé fait à 88d9567** (20 pointeurs `static`, 38 lignes de structures `static` hors `const` et scalaires, plus `g_gb_px` ; identique à 4093382 : 4f n'a ajouté ni pointeur ni structure `static`) :

| Pointeur | Peut viser | Statut |
|---|---|---|
| `g_gb[]`, `g_gb_px[].hp` (`research`) | chargement | **refaits** par `build_gb` dans le même `pico8_cart_load`, après la remise ; **oubliés** par `pico8_research_shutdown` à l'arrêt, avant d'être relus. Tout pilote pose `mem` avant `pico8_cart_load`, donc `pico8_data_init` tourne toujours (`grep` : aucun appelant de `pico8_cart_load` sans `pico8_dispatch_set_mem`) |
| `g_gb_px[].px`, `g_rr` | — | textes de `pico8_fd_table`, statiques |
| `g_fm` (`probe`) | génération | **repointé** par `eu` après la bascule ; entre `_init` et `eu`, il désigne l'ancien `hy`, encore vivant ; à l'arrêt et au rechargement, mis à NULL ou au tenant-lieu sans lecture |
| `g_ie_gy` (`research_icon`) | génération | **repointé** par `_init` sur `ie.eh` (statique), puis, dans `eu`, par les `hb` de `qh` et `os` (sans `rr`, révélées d'office par `fm:lj()`) sur des boîtes de la nouvelle génération. Après le vidage qui suit un `eu`, il ne désigne jamais la génération rendue, même sans `_init` entre deux `eu` (cas de TU) |
| `g_jo_prise`, `g_jo_prises_libres[]` (`planet`, 8 places depuis 4f) | arène d'une icône refusée (et son identifiant) | comme en 4d : ouverte seulement autour de la construction, liste oubliée au point d'oubli de partie, au chargement, à l'arrêt. Une arène rendue par un `mz` (4f) peut être reprise par une icône : son identifiant y est fait après la remise à vide |
| `g_ok` → `ok.o_`, `ok.gj` | une `ie` survolée | nouvel `ok` refait par le `jl()` d'`eu` ; l'ancien n'est plus lu une fois `ok:lj` fini (4a). Une `ie` n'a pas de `rendre` : elle n'entre pas en file et ne meurt qu'avec sa génération |
| `g_ok` → `ok.lheld` | la planète tenue, et son identifiant | comme en 4d |
| `g_hv`, `g_cv[].list`, `g_cg[].list` | des `ie` | refaits par le `jl()` d'`eu` (4c) |
| `g_mo`, `g_je`, `g_kc`, `g_ju`, `g_dk`, `g_jo_oc`/`g_jo_ir`, `g_arenes[][]`, `partie_choisie`, zones et poignées, dispatch, `g_shared_le`, `g_ga_by_shared`, `g_bv_et`, `g_jc_too_far`, `g_mz_jj_wrapper`, tables du panneau de score | — | ne peuvent pas viser un objet de 4g, comme à 4a–4e |

**Objets d'autres zones qui désignent des objets de 4g** (lecture) :

- boutons (`_jo_fe_b`) et rangées de la zone interface désignent des `hp` : ce sont des entités ou leurs parties, abandonnées par `pico8_entity_abandonner_hv` et rendues par le vidage **avant** la remise à zéro du chargement ;
- une planète (`hn`) désigne un `hp` : génération, rendue ou remise avant le chargement ;
- `jo.jy` : des copies par valeur de `hp.i` (4d), ou des `jy` de génération ;
- un éclat `ga` emprunte l'`id` d'une planète remplacée par `jo:hu` : une planète **posée**, dont l'arène n'est jamais recyclée (4d).

**Conclusion** : ni le point d'oubli de partie ni `oublier` ne gagnent d'entrée. Un pointeur trouvé ensuite par ASan sera ajouté au point d'oubli et noté dans « Découvert » comme un **trou de l'inventaire** (§4). L'inventaire est refait sur le code du commit au morceau de clôture ; s'il diffère de ce relevé, la différence est classée avec la même question.

### Verrou

- `tools/alloc_guard_exceptions.txt` : retirer `src/pico8_research.c 5`, `src/pico8_research_icon.c 3`, `src/pico8_cart.c 1`, `src/pico8_economy.c 3` ; `src/pico8_probe.c 3` → `2`.
- **Mesuré** par `make alloc-guard` sur le prototype à 88d9567 : « verrou objets 65 allouent 8 appels 20 » (**12 / 33 → 8 / 20**). Le brouillon avait mesuré 13 / 36 → 9 / 23 à 4093382 : même baisse de 13.
- `<stdlib.h>` et `pico8_zone_transitoire.h` sortent des fichiers qui n'en ont plus besoin (`cart` garde peut-être `<stdlib.h>` pour autre chose : à vérifier à la compilation).

### Budget : mesure, projection, décision

- **Mesure par prototype** à 88d9567 (refaite après 4f), sur une copie `git archive` hors dépôt, avec toute la conception de cette PRD. Les 26 e2e rejoués avec `PICO8_REMPLISSAGE`, contre la même mesure à 88d9567 (maximum 130 720 o sur `pause_restart`, chargement 0) :
  - les 26 `*.sha256` sont identiques ; `sanitize-net` répond « 26 propre(s) » ; `make e2e` est vert ; `make test` est vert une fois réécrits les TU de « Testing Decisions », cas de 4f compris (`pliages.json`, `pliage_abandonne.json`, prises qui reprennent l'arène du `mz` mort) ;
  - **chargement : 22 752 o** sur les 26 e2e, constant du début à la fin de chaque e2e (contre 0) ;
  - **génération : +10 184 o par partie** au maximum de 17 e2e (2 × 10 184 sur `pause_restart`), jusqu'à +10 344 sur ceux qui inventent ou font des besoins de planète (`synthesizer_second_link`, `tech_starbirth_protostar`) ;
  - **`max_octets partie` : 151 088 o** sur `pause_restart` (130 720 + 2 × 10 184, deux générations au clic) ; `trade_rich_mine` 105 528 o ; `game_end_bankrupt` 94 232 o ;
  - les écarts sont exactement ceux du brouillon à 4093382, e2e par e2e : 4f a décalé la base de certains e2e (+2 048, une arène de `mz`), pas le coût de 4g.
- **Ce que coûtent les 10 184 o** (tailles x86-64) : `hy` 472 o + conteneur `invented` 24 o ; 7 `ie` × 176 o = 1 232 o ; les boîtes de `ie.gy` des icônes révélées par `eu` (8 o chacune) ; le reste, **≈ 8,4 Ko, sont les sept bulles `jb`** (lignes et textes recopiés). L'estimation de 4a (≈ 3 Ko) oubliait les bulles.
- **Chargement** : 34 `hp` × 576 o = 19 584 o, plus 3 168 o de listes `i` et de `jy`. L'estimation de 4a (≈ 32 Ko) comptait ~36 `hp` et des listes plus grosses.
- **Projection.**
  - `M_4g` = 151 088 o ≈ 147,5 Ko (`M_4f` = 130 720 o mesuré par 4f, + 2 × 10 184).
  - Reste : 4h, estimée ~12 Ko (4a).
  - `r` : le rapport « mesuré / estimé » des familles faites, cumulé (4b–4g : 147,5 mesurés contre ~219 Ko estimés au pic), vaut moins de 1, donc **r = 1** ; avec le seul rapport de 4g (20,4 / 6 ≈ 3,4), par prudence, `P_4g` ≈ 147,5 + 12 × 3,4 ≈ **188 Ko**. Dans les deux cas ≤ 704.
  - `Q_4g` = 22,2 Ko ≤ 64 ; aucune famille restante n'a de chargement.
  - Somme des zones inchangée : 1487 Ko sur 1536 (`pico8_zone_sizes.h` identique à 4093382).
  - **On continue.**
- **Mesure à refaire** après conversion : `M_4g`, `K_4g`, le surcoût par partie, et la génération de `pause_restart` à f13 contre le terme exact (voir Testing Decisions). Les noter dans le commit et dans « Découvert », comparés au prototype ; un écart s'explique avant la projection.
- **Décision**, règle de 4c, dans l'ordre : si `P_4g ≤ 704` et `Q_4g ≤ 64`, on continue ; sinon déplacer la frontière entre génération et chargement ; sinon relever d'un chiffre sur les 49 Ko ; sinon **arrêt de l'étape 4 à 4g** et signalement à l'humain (zone, limite voulue, somme actuelle).
- Un `pico8_fatal` « zone chargement pleine » se lit d'abord comme un `gb` refait sans rechargement (TU), puis comme une limite trop basse. Un « zone partie pleine » se lit d'abord comme un site de 4g entré par tick ou par action.

### Note « Découvert » et clôture

À la clôture, sous l'entrée 4 de `docs/plan-4.md`, une note « *Découvert (4g) :* » de quelques phrases :

- placement : `gb`, `hp.i` (réservée) et leurs `jy` en chargement ; `fm`, `invented`, `ie`, `ie.jb`, boîtes de `ie.gy`, `jy` de planètes en génération ; « s… » dans l'arène de prise ; `pico8_jy_new` prend une zone ; `fm_free` supprimé ;
- corrections de la table de 4a : la boîte de `ie.gy` est faite une fois par icône (`kh`), pas à chaque `hb` ; l'estimation par partie oubliait les bulles `jb` ;
- les TU réécrits : `test_rebuild_and_shutdown` (trois `gb` sans recharger dépassaient le chargement) ; le besoin de `test_pico8_hy` fait avant la dernière nouvelle partie ;
- les mesures et la décision : `M_4g`, `K_4g`, `P_4g`, `Q_4g`, `r` ; les comptes du verrou.

Le §2 est corrigé : ligne « Partie », « recherche » devient « économie `fm`, icônes de recherche et leurs bulles » ; les objets du chargement deviennent « `gb` avec `hp.i` et ses besoins ».

Sous l'entrée 4, une ligne « *Reçu de l'étape 4g :* » **pour 4h** : `pico8_probe.c` n'a plus que `pico8_bv` et `pico8_g__struct_new` ; `fm` n'est plus libéré par `set_fm`, et `g_fm` désigne l'ancien `hy` (génération) jusqu'à `eu`.

La PRD est retirée de `docs/prd/` dans le commit de clôture.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 :

- `make test-all` et `make sanitize-net` passent ;
- les 26 `*.sha256` sont identiques ;
- `alloc-guard` est à égalité stricte.

Aucun morceau ne passe la transitoire à un nouvel appelant. Les comptes partent de 88d9567 (12 / 33).

1. **Recherche en sous-zone chargement.**
   - Les tests d'abord : chargement exact après `pico8_cart_load` et égal après un second ; chargement constant sur `pause_restart` ; `pico8_jy_new` dans la zone passée ; `test_rebuild_and_shutdown` réécrit.
   - `pico8_jy_new` prend une zone, ses trois appelants et les TU passent la leur ; `hp:pq` en chargement, `i` réservée ; `free_gb_instances` sans `free` ; contrats de `pico8_research.h` réécrits.
   - Le verrou : `research` sort, **12 / 33 → 11 / 28**.
2. **Économie en génération, plus de `free` de `fm`.**
   - Les tests d'abord : le `fm` de la partie quittée se relit pendant le clic de `pause_restart` et fait mourir l'enfant ASan après (rouge aujourd'hui : `set_fm` le libère dans `eu`) ; terme `hy` dans l'égalité exacte de la génération ; besoin de `test_pico8_hy` déplacé.
   - `hy` et `invented` en génération ; `fm_free` supprimé ; contrats de `pico8_economy.h` et `pico8_fm.h` réécrits.
   - Le verrou : `economy` sort, `probe` 3 → 2, **11 / 28 → 10 / 24**.
3. **Icônes en génération, identifiant dans l'arène de prise.**
   - Les tests d'abord : terme « icônes, bulles, boîtes de `ie.gy` » dans l'égalité exacte de la génération ; une icône de la partie quittée se relit pendant le clic et fait mourir l'enfant ASan après.
   - `ie` et sa boîte en génération ; `ie.jb` en génération dans `eu` ; `ie:ldown` ouvre la prise avant l'identifiant ; contrats de `pico8_research_icon.h` réécrits.
   - Le verrou : `research_icon` et `cart` sortent, **10 / 24 → 8 / 20**.
4. **Mesure et clôture.**
   - Le relevé sur les 26 e2e, `M_4g`, `K_4g`, la comparaison au prototype, la projection et la décision, dans le message du commit.
   - L'inventaire refait par `grep` (motif de structure anonyme compris).
   - La note « Découvert (4g) », la correction du §2, la ligne « *Reçu de l'étape 4g* » pour 4h, le retrait de cette PRD.

Les morceaux 1 à 3 sont indépendants entre eux (fichiers et sites disjoints, sauf l'égalité exacte des TU, que chacun complète). Le 4 dépend des trois. 4f est finie et vérifiée : rien n'attend plus.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris de la génération et du chargement (`pico8_zone_partie_prise`, `pico8_zone_chargement_prise`, les lignes de `PICO8_REMPLISSAGE`) ;
  - les accesseurs publics (`pico8_gb_count`/`_at`/`_find`, `pico8_fm_hy`, `pico8_ie_gy`, les champs d'un `hp`, d'un `hy`, d'une `ie`) ;
  - un rapport ASan dans un processus enfant ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne lit ni les bouts de la zone ni une adresse, sauf pour la relire après sa mort. Les tailles attendues se calculent dans le TU (`sizeof`, `PICO8_ZONE_CELLS`, une bulle refaite dans une arène de mesure), jamais en dur : 22 752 et 10 184 ne valent que sur x86-64.
- **Chargement exact** (morceau 1, `tests/test_pico8_hp.c` ou `tests/test_pico8_places_vivantes.c`) : après `pico8_cart_load`, `pico8_zone_chargement_prise()` vaut la somme, sur `gb`, des blocs du `hp`, du conteneur `i`, de ses éléments (si la chaîne n'est pas vide) et de ses `jy`, plus les listes statiques de 4c (vides). Rouge aujourd'hui (0).
- **`test_rebuild_and_shutdown` réécrit** : `build_gb`, `test_gb`, arrêt, `pico8_research_init`, puis `pico8_cart_load` (qui remet le chargement et refait `gb`) : `test_gb` passe, et un second `pico8_cart_load` laisse `pico8_zone_chargement_prise()` à la même valeur. Le prototype l'a vu vert. Le contrat « rebuilt from `fd` frees the projects it held » devient « oublie les projets qu'il tenait ; leur mémoire repart au chargement suivant ».
- **Chargement constant pendant une partie** : sur `pause_restart` (deux parties), le chargement est égal à f1 et à la dernière frame ; `max_octets chargement` égale `chargement`.
- **`pico8_jy_new`** : un besoin pris dans la zone passée, sur le modèle des cas de zone de `tests/test_pico8_ptrs.c`.
- **Égalité exacte de la génération** (morceaux 2 et 3), dans `tests/test_pico8_zone_partie.c` (`case_prise_and_max`) et `tests/test_pico8_places_vivantes.c` (`pause_restart_borne_les_blocs_morts_des_registres`, `pause_restart_compte_les_etats_de_bw`) : un terme « recherche et économie » s'ajoute aux états de `bw`, registres et planètes :
  - bloc du `hy`, bloc du conteneur `invented` (l'assertion « aucune invention » garde la liste vide) ;
  - pour chaque `ie` de `hv` (reconnue par `pico8_ie_struct_class()`, sept, dans l'ordre de `pico8_dr_table`) : bloc de l'`ie`, octets de sa bulle **mesurés en la refaisant** par `pico8_jb_new` dans une arène de la zone frame (`pico8_zone_arene_prendre`/`_prise`), et un bloc de `pico8_vec_t` si elle est révélée.

  Le prototype a vu ce terme exact (`pause_restart` f13 : génération 75 256 = 608 + 16 224 + 48 240 + 10 184).
- **`fm` de la partie quittée** (morceau 2, `tests/test_pico8_places_vivantes.c`, sur le modèle de `pause_restart_garde_les_planetes_de_la_partie_quittee_pendant_le_clic` et `pause_restart_rend_les_planetes_de_la_partie_quittee`) : pris à f11 (`pico8_fm_hy()`), il se relit à la fin de f12 (clic « challenging », bascule) ; l'enfant meurt par ASan quand il est relu à f13. Le premier cas est rouge aujourd'hui (`set_fm` le libère dans `eu`).
- **Icône de la partie quittée** (morceau 3) : même forme, sur une `ie` de `hv` à f11.
- **`test_pico8_hy`, cas `test_lj`** : le besoin `need` était fait avant `built_world()` et `new_game()`, donc dans une génération rendue avant qu'il serve. Le prototype a vu l'attente sur `fu` tomber (lecture d'une génération rendue, sans ASan dans ce binaire). Il est fait juste avant `pico8_qh_new`, après la dernière nouvelle partie.
- **Inchangés** et verts dans le prototype refait à 88d9567 : `message_box_hold` f45 = f90, `game_end_final_score` f2 = f51, `prises_refusees_ne_font_pas_grandir_la_partie`, `une_prise_posee_reprend_l_arene_du_mz_mort`, `un_slipway_pose_reprend_l_arene_du_mz_mort`, les cas de trous de ver de 4f (`un_mz_tenu_ne_fait_pas_grandir_la_partie`, `mz_refuses_ne_font_pas_grandir_la_partie`, `un_mz_ouvert_rend_son_arene_a_sa_mort`, `un_mz_refuse_traverse_une_nouvelle_partie`, `pliage_abandonne_*`) et la sentinelle de 4a. Ils comparent des générations à l'intérieur d'une partie, ou des prises après la nouvelle partie : les 10 184 o faits par `eu` n'y entrent pas ; `tests/test_pico8_ie.c`, `tests/test_pico8_hp.c` (hors le cas réécrit), `tests/test_pico8_hy.c` (hors `need`).
- **Verrou** : `make alloc-guard` à égalité stricte avec la liste de chaque morceau.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`.
- **Exemples dans le dépôt** : `tests/test_pico8_places_vivantes.c` (rejeu par enfant, `dans_un_enfant_qui_meurt_par_asan`, cas « planètes de la partie quittée » de 4d, `octets_des_*`) ; `tests/test_pico8_zone_partie.c` (`case_prise_and_max`) ; `tests/test_pico8_ptrs.c` (réservation).

## Out of Scope

- `pico8_bv` (`probe` 162) et `pico8_g__struct_new` (`probe` 413), `ok.qa`, `pj`, `fv`, `bg`, `qb`, `nf`, `ui` : 4h.
- Le score `pj`, fait dans `eu` à côté de `fm` (`pico8_pj_new`, `score_panel` 263) : 4h.
- L'arène de prise elle-même, sa taille (2048 o), ses prises libres (8), son retour (`pico8_jo_prise_zone_rendue`, `pico8_jo_prise_arene_rendue`) : 4d, 4e, 4f. 4g n'y ajoute qu'un bloc (l'identifiant).
- Faire de la boîte de `ie.gy` une valeur : écarté (voir Implementation Decisions).
- Relever `PICO8_ZONE_CHARGEMENT_BYTES`, `PICO8_ZONE_GAME_BYTES` ou `PICO8_JO_PRISE_OCTETS`, sauf si la mesure l'exige.
- Retirer TROU-7 : étape 5.
- L'audio : hors périmètre du portage.
- Lancer l'application raylib.

## Vérifié contre 4f

Relu à `HEAD` 88d9567 après la clôture de 4f (`git log -4 --stat` : 1d7bfa3 `pico8_ents_init_reserve`, d1e5da0 trous de ver dans l'arène de prise, 88d9567 clôture), sans toucher git ni les sources. Chaque hypothèse du brouillon, et ce qui a été réécrit.

| Hypothèse | Tenue ? | Constat à 88d9567 | Sections réécrites |
|---|---|---|---|
| **H-4f-1** (verrou et fichiers) | **tenue** | 4f n'a touché que `pico8_ents`, `pico8_planet`, `pico8_wormhole`, `test_pico8_mz`, `test_pico8_places_vivantes`, deux fixtures et la liste du verrou. `alloc-guard` : 12 / 33. Les fichiers de 4g (`research*`, `economy*`, `economy_tiers`, `planet_trade`, `cart`, `probe`) et leurs TU sont identiques à 4093382 : les 13 relocations sont aux mêmes lignes | Problem Statement (verrou), Verrou, Implementation Chunks (paliers 12 / 33 → 11 / 28 → 10 / 24 → 8 / 20) |
| **H-4f-2** (arène de prise) | **tenue** | `pico8_jo_prise_ouvrir`/`_fermer`, `pico8_jo_zone_construction` inchangés ; `PICO8_JO_PRISE_OCTETS` 2048 (contrat : « a held icon, a drawn slipway or a wormhole ») ; `pico8_jo_prise_zone_rendue` garde son test « kz » et appelle le cœur sans condition `pico8_jo_prise_arene_rendue` ; `PRISES_LIBRES_MAX` 8 | Inventaire (`g_jo_prises_libres`), Out of Scope |
| **H-4f-3** (budget) | **tenue** | « Découvert (4f) » : maximum 130 720 o inchangé, chargement 0, 12 / 33 ; `pico8_zone_sizes.h` inchangé (somme 1487 Ko) | Budget (mesure refaite, `M_4g`, somme des zones) |
| **H-4f-4** (outillage de test) | **tenue** | `demarrer`, `jouer_jusqu_a`, `partie_sans_registres`, `octets_des_registres`, `octets_des_planetes`, `etats_de_bw`, `dans_un_enfant_qui_meurt_par_asan` et l'égalité exacte de `pause_restart` f13 sont là. Les cas « une arène de plus » de 4d/4e sont devenus `*_reprend_l_arene_du_mz_mort`. Le cas « refus, restart, nouvelle partie, pression » (`un_mz_refuse_traverse_une_nouvelle_partie`) ne compare pas de tailles : il vérifie que l'enfant ne meurt pas ; vert dans le prototype | Testing Decisions (liste des cas inchangés) |
| **H-4f-5** (« Reçu ») | **tenue** | La seule ligne « *Reçu de l'étape 4f* » vise 4h | Problem Statement |
| **H-4f-6** (inventaire) | **tenue** | `oublier` et `oublier_partie` inchangés (`pico8_cart.c` identique) ; les deux `grep` de 4a et celui des structures anonymes donnent les mêmes lignes qu'à 4093382 (20 pointeurs, 38 structures, `g_gb_px`) | Inventaire (relevé daté de 88d9567) |
| **H-4f-7** (ordre de construction) | **tenue** | Le `rendre` du `mz` rend `ox->zone`, celui des planètes `oe->zone` : aucun retour ne suppose la structure en tête d'arène | Aucune : l'ordre « ouvrir, identifiant, structure » reste |

**Ce que 4f apporte en plus des hypothèses.** `pico8_ents_init_reserve` (capacité 0 acceptée) : elle ne convient pas à `hp.i`, qui est une `pico8_ptrs_t` ; l'implémenteur peut aligner `pico8_ptrs_init_reserve` sur elle (voir « Sous-zone chargement »).

**Prototype refait à 88d9567**, même conception, mêmes réécritures de TU (l'égalité exacte de `tests/test_pico8_places_vivantes.c` reprise sur sa version de 4f) :

- `make test` vert, `make e2e` vert (26 `*.sha256` identiques), `make sanitize-net` « 26 propre(s) » ;
- `alloc-guard` : « verrou objets 65 allouent 8 appels 20 » ;
- chargement 22 752 o, constant sur les 26 e2e ; génération +10 184 o par partie, 151 088 o au plus (`pause_restart`) ; écarts identiques, e2e par e2e, à ceux du brouillon ;
- `pause_restart` f13 : génération 75 256 = 608 + 16 224 + 48 240 + 10 184, comme au brouillon.

## Further Notes

- **Pas d'humain disponible, et pas d'outil `Agent` dans cette session pour lancer un agent `guideline` neuf.** J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage).
  - **Hypothèses explicites** : celles sur 4f (H-4f-1 à H-4f-7) ont été posées au brouillon puis toutes vérifiées contre 4f finie (section « Vérifié contre 4f »). Les fréquences ont été lues dans le cart : `ie` l. 919-939 (`hb` 926, `ldown` 932), `_init` l. 1178, `eu` l. 1180-1187, `hy` l. 1198-1230 et `hy:kt` l. 1446. Il en sort deux corrections de 4a : la boîte de `ie.gy` est faite une fois par icône ; l'estimation par partie oubliait les bulles.
  - **Simplicité** : aucun mécanisme neuf. La sous-zone chargement de 4c, la génération, l'arène de prise de 4d/4e, `pico8_ptrs_init_reserve`, un paramètre de zone de plus à `pico8_jy_new`, une fonction supprimée (`fm_free`). Écartés : une remise du chargement par `build_gb` (la sous-zone tient aussi les listes de 4c, faites avant `pico8_data_init`), une arène propre à `gb` (seul un TU refait `gb` sans recharger), une valeur pour `ie.gy` (forme de `pico8_ie_t` changée pour 56 o), un relèvement du chargement pour un chemin de TU.
  - **Changements chirurgicaux** : 13 relocations dans 5 fichiers, plus deux appelants de `pico8_jy_new` et l'ordre de trois lignes dans `ie:ldown`. Aucune forme de structure ne change.
  - **Critères vérifiables** : chaque morceau a ses tests rouges d'abord ; le prototype, refait à 88d9567, a montré les attentes (sha256, ASan, verrou 8 / 20, chargement 22 752, génération +10 184 exacte).
- Le prototype (copie `git archive HEAD` hors dépôt, à 4093382 puis à 88d9567) a aussi refait les TU cassés, pour savoir lesquels le seraient : `test_pico8_zone_partie` et `test_pico8_places_vivantes` (égalités exactes), `test_pico8_hp` (« zone chargement pleine » au troisième `gb`), `test_pico8_hy` (besoin fait dans une génération rendue). Un `test_pico8_hy` compilé sous ASan pour l'occasion passe : le second `eu` sans `_init` du cas « construction » lit `ie.gy` dans la génération précédente, encore vivante. Rien de ce prototype n'est dans le dépôt.
