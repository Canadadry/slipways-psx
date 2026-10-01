---
title: "Plan 4, étape 4b — États de pico8_gen.c : jo.oc, jo.ir et leur source en zone partie, oh de bg:pq en zone frame"
description: "Les générateurs jf/gn prennent encore leur état à malloc et le rendent par free ; 4b leur fait prendre la zone désignée par l'appelant, met jo.oc, jo.ir et la source de jo.oc dans la génération de partie ouverte par eu, et oh de bg:pq dans la zone frame. pico8_gen.c sort de la liste du verrou, et c'est la première famille dont on mesure l'empreinte en zone partie."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. L'étape 4 met les objets de partie dans la **zone partie**. Le texte de l'étape, recopié du plan tel qu'il était quand 4b a été rédigée (`HEAD` a182a7f). Le texte à jour, après la clôture de 4a, est cité plus bas :

> 4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement, et si son remplissage ne monte pas pendant une phase sans action d'un e2e.
>    Elle se fait en sous-étapes, une PRD chacune :
>    - **4a. Socle.** La zone partie, vide, avec sa remise à zéro et son empoisonnement sur nouvelle partie, son accesseur de remplissage (octets pris, et maximum, §3) écrit par le rejeu, et l'inventaire du §4 des pointeurs qui survivent à une nouvelle partie. Aucun propriétaire converti.
>    - **4b et suivantes.** Une famille de propriétaires par PRD (tableaux croissants et registres ; planètes et relais ; slipways et routes ; recherche ; états de `pico8_gen.c` ; trous de ver), dans l'ordre que fixe la PRD 4a. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
>    - *Reçu de l'étape 1 :* même chose pour les boîtes `vec` des objets de partie (`p`/`v`/`ra` de `qb`, `ra` de `nf`, `mo->v`, `re`/`p` de `jc:hq`, `p` de `pico8_bv`, `ie.gy`) ; `qb:ph` seul en fait trois par frame. L'état `oh` de `pico8_bg_pq` est un objet de frame fait par `pico8_jf`, qui ne prend pas de zone : l'étape 4, qui donne une zone aux états de `pico8_gen.c`, le range en zone frame.
>    - *Reçu de l'étape 2 :* restent à cette étape `ok.qa` (gardé par les hôtes de `bv` et le point souris de `jc:hq` : objet relu par des objets de partie), la structure `ok` et ses calques `ic`, la liste statique de départ, `ie.jb` (fait une fois par partie), `fv`, la structure `pj`, `hp.i`. Les structures de planète ne sont toujours pas libérées (seule leur rangée l'est).
>    - *Reçu de l'étape 3 :* la structure `g_` (et `fv`) et sa boîte `p` de `bv` restent à cette étape (`/* TROU-7 */`) ; `oe` d'une planète ordinaire passe par `pico8_jo_pq_dans`, qui prend déjà sa zone.

4a est finie (10d368e, 646f7fc ; clôture b46601e). Sa clôture a réécrit la puce « 4b et suivantes », dans l'ordre fixé par la PRD 4a :

> - **4b et suivantes.** Une famille de propriétaires par PRD dans l'ordre fixé par la PRD 4a : 4b états de `pico8_gen.c` ; 4c tableaux croissants et registres (crée la sous-zone chargement) ; 4d planètes et relais ; 4e slipways et routes ; 4f trous de ver ; 4g recherche et économie ; 4h singletons de `jl`/`eu` et objets du joueur (famille ajoutée par 4a : les « Reçu » des étapes 1 à 3 leur reviennent). N'entre en zone partie qu'une allocation faite une fois par objet de partie ; un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.

La clôture a aussi ajouté une note « *Découvert (4a)* » et une seule ligne « *Reçu de l'étape 4a* » (les traînées `ts` vont à 4e). Elle a corrigé les lignes « Partie » et « Particules » du §2. La ligne « Partie » dit désormais : « une génération par partie, de `eu` à `eu` ; rendue d'un bloc au vidage de la file qui suit le `eu` suivant (zone à deux bouts) ; `_init` ne la rend pas ».

La PRD 4a a été retirée de `docs/prd/` par b46601e. On la lit par `git show a182a7f:docs/prd/triage/phase1-etape4a-socle-zone-partie.md`. Sa ligne pour 4b, recopiée :

> | **4b** | **États de `pico8_gen.c`** | `pico8_jf` (9), `pico8_jf_free` (26), `pico8_gn` (50), `pico8_gn_free` (124) ; leurs appelants `bw`, `jo:pq` et `bg:pq` choisissent la zone | `gen` 4 → sort | *Reçu 1* : `oh` de `pico8_bg_pq` en zone frame |

et la raison de cet ordre :

> **4b** est la plus petite famille (4 relocations, moins de 2 Ko). Ses objets sont faits dans `eu` et rendus par `bw` au `eu` suivant. Elle éprouve la bascule sur de vrais objets avec le seul motif délicat de l'inventaire : un ancien objet rendu par son remplaçant, encore vivant.

Cette PRD ne traite que **4b**. Le seul *Reçu* qui lui revient est la deuxième phrase du *Reçu de l'étape 1* (`oh` en zone frame). Les autres lignes « Reçu » vont à 4c–4h, selon la table de 4a.

Les règles du plan qui la cadrent :

- §2 : la zone partie contient les objets de partie ; « taille fixe ; vidée entièrement à chaque nouvelle partie ». La zone frame contient les « listes et tableaux de travail jetés dans la frame ».
- §3 :
  - limites au doigt mouillé ;
  - somme des zones ≤ 1,5 Mo ; un relèvement qui la dépasserait arrête l'étape, qui le signale ;
  - invariant de chaque commit : `make test-all`, `make sanitize-net`, `*.sha256` identiques ;
  - « Croissance par frame » : le remplissage d'une zone qui n'est pas remise à chaque frame est égal à deux frames séparées par une phase sans action.
- §4 :
  - un site douteux va dans la zone la plus longue qui le couvre, et seule la lecture le fait passer dans une zone plus courte ;
  - avant de faire finir une durée de vie, la PRD inventorie mécaniquement les pointeurs du cœur qui survivent.
- §5 : fin d'étape = chaque site de l'inventaire converti, `alloc-guard` à égalité stricte, aucun compte n'a monté.

**Ce qui se passe aujourd'hui** (relu à `HEAD` b46601e, 4a finie ; `pico8_gen.*`, `pico8_galaxy.*`, `pico8_planet.*`, `pico8_bg.c` et `test_pico8_map_tables.c` n'ont pas changé depuis a182a7f, les numéros de ligne restent donc valables) :

- **La zone partie existe** (`pico8_zone_partie.{h,c}`) :
  - **poignée** : `pico8_zone_partie()` rend la poignée de la génération courante, `const pico8_zone_choisie_t *` ; `alloc` prend au bout courant, `grow` copie, `release` ne fait rien ;
  - **bascule** : `pico8_zone_partie_basculer()` est appelée en tête de `pico8_eu`, après `pico8_economy_set_dk(nj)` et avant `pico8_jl()` ;
  - **rendu** : `pico8_cart_vider_file_sortie()` vide la file, puis, si `pico8_zone_partie_en_attente()`, appelle le point d'oubli `oublier_partie()` (statique, vide) et `pico8_zone_partie_rendre()` ;
  - **remise à zéro** : `pico8_cart_load()` et `pico8_cart_shutdown()` appellent `pico8_zone_partie_reset()` juste après leur vidage ;
  - **remplissage** : `pico8_zone_partie_prise()`/`_max()` comptent les deux bouts, arrondis à la cellule de 8 o ; le maximum survit au reset. Le rejeu écrit `partie <n>` et `max_octets partie <n>`. Aucun objet du jeu n'y est encore.

- `pico8_gen.c` porte les deux fabriques de fermetures du cart, `jf(n_,b)` (marche aléatoire bornée) et `gn(hr,og,r)` (tirage pondéré avec fenêtre anti-répétition). C n'a pas de fermetures : chacune devient un **état** (`pico8_jf_state_t`, 8 o ; `pico8_gn_state_t`, 592 o) pris à `malloc` et un `_next`. L'appelant le rend par `pico8_jf_free`/`pico8_gn_free`, qui font `free`.
- Trois états vivent toute une partie. Ils sont faits par `bw` (`pico8_bw`, appelé une seule fois, par `pico8_eu`, après son `jl()`), à la fin de la boucle des 90 planètes, comme dans le cart (`jo.oc,jo.ir=gn(df,2,jf(0.5,dk.en)),jf(0.8,0.5)`) :
  - `jo.oc`, l'état `gn`, gardé par `g_jo_oc` (`pico8_planet.c`) ;
  - sa source aléatoire, un état `jf` passé à `gn` comme `r_ctx`. `gn` ne le possède pas, alors `pico8_galaxy.c` le garde dans `g_bw_oc_r_ctx` pour pouvoir le libérer ;
  - `jo.ir`, un état `jf`, gardé par `g_jo_ir`.
- **Qui les remplace.** Au `bw` suivant, `pico8_jo_set_oc`/`pico8_jo_set_ir` libèrent l'ancien état avant de garder le nouveau, et `bw` libère l'ancien `g_bw_oc_r_ctx`. `pico8_jo_shutdown` (appelé par `pico8_jo_init` au chargement, et par `pico8_cart_shutdown`) libère `jo.oc`/`jo.ir` et les remet à nil. `pico8_galaxy_shutdown` libérerait `g_bw_oc_r_ctx`, mais **rien ne l'appelle**.
- **Qui les lit.** Seul `pico8_jo_hh` (`jo:hh`, quand une sonde `mz:pr` dévoile une planète « unknown ») lit `jo.ir`, puis `jo.oc` si `df.ll` n'est pas pris. `jo:pq` ne les touche pas : la ligne de 4a nomme `jo:pq` là où les sites de `pico8_planet.c` sont `pico8_jo_set_oc`, `pico8_jo_set_ir` et `pico8_jo_shutdown` (qui libèrent) et `pico8_jo_hh` (qui lit).
- **Un état de frame.** `pico8_bg_pq` (`bg:pq`, peinture des 32×32 tuiles du fond) fait un `jf(0.1,0)` local, `oh`, et le libère en sortant. `bg:pq` n'est appelé que par la construction de `bg`, dans `eu` (et par les TU qui construisent un `bg`).
- **Le verrou**, remesuré par `make alloc-guard` sur une copie `git archive HEAD` à b46601e : **65 objets** (`pico8_zone_partie.o` en plus, qui n'alloue pas), **24 allouent, 84 appels**, liste inchangée. Les 4 relocations de `pico8_gen.o`, rattachées par `objdump -r` et `addr2line` (relevé à a182a7f ; fichier identique depuis) :

  | Fonction | Ligne | Appel |
  |---|---|---|
  | `pico8_jf` | 9 | `malloc` |
  | `pico8_jf_free` | 26 | `free` |
  | `pico8_gn` | 50 | `malloc` |
  | `pico8_gn_free` | 124 | `free` |

  Les appelants ne comptent pas : `pico8_galaxy.c` 3 (`p`, `ra`, identifiant « p… » de `bw`, famille 4d), `pico8_bg.c` 2 (`bg_struct_new`, `bg_hq`, famille 4h), `pico8_planet.c` 6 (famille 4d). Aucun de ces comptes ne vient d'un état de `pico8_gen.c`.

Le problème : ces quatre appels sont les seuls de `pico8_gen.c`, et ses états n'ont pas de zone. Tant que `jf`/`gn` prennent `malloc`, aucun appelant ne peut choisir la zone de l'objet qu'il fait.

## Solution

`pico8_jf` et `pico8_gn` prennent leur état dans une zone **désignée par l'appelant** (`pico8_zone_choisie_t`), comme `pico8_tostr_fixed` ou `pico8_concat2` depuis 0b. Personne ne libère plus un état : il vit autant que sa zone.

- `bw` passe la **poignée de la génération de partie courante** (`pico8_zone_partie()`) pour les trois états qui vivent toute la partie. `bw` est appelé dans `eu` *après* la bascule, donc ses états entrent dans la génération de la nouvelle partie.
- `bg:pq` passe la **zone frame** pour `oh`, qui ne sort pas de l'appel (*Reçu de l'étape 1*).
- Les remplacements ne libèrent plus rien. Au `eu` suivant, `bw` repointe `jo.oc`/`jo.ir` sur les nouveaux états. Les anciens restent dans l'ancienne génération, valides jusqu'à son rendu au vidage qui suit, puis empoisonnés sous ASan. Le « motif délicat » de 4a (un ancien objet rendu par son remplaçant, encore vivant) disparaît : le remplaçant n'a plus rien à rendre, c'est la génération qui rend.
- `pico8_jf_free`, `pico8_gn_free`, `g_bw_oc_r_ctx` et `pico8_galaxy_shutdown` n'avaient d'autre rôle que de libérer : ils disparaissent.
- `pico8_gen.c` n'appelle plus la libc et **sort de la liste du verrou** : 24 fichiers / 84 appels → **23 / 80** (prévision).
- Première mesure réelle de la zone partie : le maximum relevé sur les 26 e2e est **prévu exactement** (608 o par génération, 1216 o pendant la bascule de `pause_restart`, sur x86-64). La projection de 4a est refaite avec cette mesure, et l'étape continue ou s'arrête selon sa règle.

Aucun pixel ne change, aucun tirage `rnd` n'est déplacé.

## User Stories

1. En tant que mainteneur du plan, je veux que `pico8_jf` et `pico8_gn` prennent leur état dans une zone désignée par l'appelant, pour que chaque appelant choisisse la durée de vie de ce qu'il fait, comme pour `tostr` et `concat2` depuis 0b.
2. En tant que gardien du cliquet, je veux que `pico8_gen.c` n'appelle plus ni `malloc` ni `free`, pour qu'il sorte de la liste d'exceptions d'`alloc-guard`.
3. En tant que gardien du cliquet, je veux que la liste passe de 24 fichiers / 84 appels à 23 / 80, et qu'aucun autre compte ne bouge, pour que le verrou reste à égalité stricte sans hausse.
4. En tant que gardien du cliquet, je veux qu'aucun appelant ne passe la transitoire à `jf` ou `gn`, pour qu'aucun compte ne monte (plus aucun ne monte après 0b).
5. En tant que développeur, je veux que `jo.oc`, `jo.ir` et la source aléatoire de `jo.oc` soient pris dans la génération de partie ouverte par `eu`, pour qu'ils vivent exactement une partie, comme dans le cart.
6. En tant que développeur, je veux que la source aléatoire de `jo.oc` et l'état `gn` qui la lit soient pris par la même poignée, dans le même `bw`, pour qu'ils appartiennent à la même génération et qu'aucun ne survive à l'autre.
7. En tant que développeur, je veux que `oh` de `bg:pq` soit pris en zone frame, pour qu'un état qui ne sort pas de l'appel ne coûte rien à la zone partie (*Reçu de l'étape 1*).
8. En tant que développeur, je veux que remplacer `jo.oc` ou `jo.ir` ne libère plus l'ancien état, pour que l'ancien reste valide jusqu'au rendu de sa génération et qu'aucun `free` ne vise une zone.
9. En tant que développeur, je veux que `pico8_jo_shutdown` remette `jo.oc`/`jo.ir` à nil sans les lire, pour qu'un chargement qui vient de remettre la zone partie à zéro ne touche pas de mémoire empoisonnée.
10. En tant que développeur, je veux que `g_bw_oc_r_ctx` disparaisse, pour qu'il n'existe plus de pointeur statique qui ne servait qu'à libérer et qui pendrait après chaque chargement.
11. En tant que développeur, je veux que `pico8_galaxy_shutdown`, déjà jamais appelé et désormais sans contenu, disparaisse avec ce pointeur, pour ne pas laisser une fonction vide qui laisse croire à un rendu.
12. En tant que développeur, je veux que `pico8_jf_free` et `pico8_gn_free` disparaissent, pour qu'aucun appelant futur ne libère un état pris dans une zone.
13. En tant que lecteur de `pico8_gen.h`, je veux que le contrat des deux fabriques dise que l'état est pris dans `zone` et vit autant qu'elle, sans rien à rendre, et que `r_ctx` doit vivre autant que l'état, pour que le contrat C remplace « l'appelant libère ».
14. En tant que lecteur de `pico8_planet.h`, je veux que le contrat de `pico8_jo_set_oc`/`_ir` dise que l'ancien état n'est pas libéré, pour que personne ne réintroduise un `free`.
15. En tant que développeur des familles suivantes, je veux l'inventaire du §4 refait sur le code de 4b, avec chaque pointeur qui peut viser un état de `pico8_gen.c` classé (repointé, remis à nil, ne peut pas viser), pour que le premier propriétaire en zone partie n'y laisse pas de référence oubliée.
16. En tant que développeur, je veux un TU sous ASan qui joue `pause_restart` jusqu'après le rendu de f13, puis fait dévoiler des planètes de la nouvelle partie par `jo:hh`, pour prouver que `jo.ir` et `jo.oc` désignent la nouvelle génération et pas l'ancienne, empoisonnée.
17. En tant que développeur, je veux que ce TU vérifie que la voie `jo.oc()` a bien été prise au moins une fois (une planète dévoilée dont le type n'est pas `df.ll`), pour qu'il lise réellement l'état `gn` et pas seulement `jo.ir`.
18. En tant que mainteneur du budget, je veux que les octets de la zone partie valent une génération d'états après f11 de `pause_restart`, deux après f12 et une après f13, avec un maximum de deux, calculés par `sizeof` dans le TU, pour que le pic de la bascule soit prouvé sur de vrais objets par un test qui ne dépend pas de l'architecture (le rejeu écrit déjà ces mêmes accesseurs).
19. En tant que développeur, je veux que le cas `prise_and_max` de `test_pico8_zone_partie.c` compte les états faits par le `bw` de son `eu`, pour que le TU du socle reste exact maintenant que `eu` met de vrais objets dans la zone.
20. En tant que mainteneur, je veux que le relevé f45 = f90 de `message_box_hold` reste vert avec des octets non nuls, pour que le contrôle « Croissance par frame » porte enfin sur une zone qui contient quelque chose.
21. En tant que mainteneur du budget, je veux le maximum de la zone partie relevé sur les 26 e2e après conversion, noté dans le commit et dans « Découvert », pour remplacer l'estimation de 4b par une mesure.
22. En tant que mainteneur du budget, je veux la projection de 4a refaite avec cette mesure (`P_4b`), et la décision (continuer, relever, s'arrêter et signaler) écrite, pour que l'arrêt anticipé du §3 soit appliqué dès la première famille.
23. En tant que mainteneur du budget, je veux qu'un maximum différent de la prévision exacte soit expliqué avant d'entrer dans la projection, pour qu'un objet entré par erreur en zone partie ne passe pas pour un écart d'estimation.
24. En tant que gardien de l'invariant, je veux les 26 `*.sha256` inchangés et `make sanitize-net` vert, pour prouver que changer la zone d'un état ne déplace aucun tirage `rnd` ni aucun pixel.
25. En tant que mainteneur de la fin de l'étape 4, je veux que `pause_restart` passe `make sanitize-net` avec, pour la première fois, des objets du jeu dans la génération rendue à f13, pour éprouver le critère de fin de l'étape 4 sur de vrais objets.
26. En tant que développeur de TU, je veux que `test_pico8_map_tables` construise son état `gn` dans la zone frame et ne le libère plus, pour qu'aucun TU ne garde un appel à une fonction disparue.
27. En tant que lecteur du plan, je veux une note « Découvert » sous l'entrée 4 qui dise ce que 4b a converti, sa zone par propriétaire, le maximum relevé, la projection, les comptes du verrou et l'écart à la ligne de 4a (`jo:pq`), pour que 4c parte d'un plan à jour.
28. En tant que mainteneur des commentaires, je veux que les seuls commentaires touchés soient les contrats C devenus faux (qui libère, ce que vit l'état), pour respecter les quatre destinations du `CLAUDE.md`.

## Implementation Decisions

### Inventaire des sites de 4b

| Site | Fonction, ligne à a182a7f | Aujourd'hui | Durée de vie (lecture) | Zone retenue |
|---|---|---|---|---|
| État `jf` | `pico8_jf` (`gen` 9) | `malloc` | celle que choisit l'appelant | désignée par l'appelant |
| Rendu `jf` | `pico8_jf_free` (`gen` 26) | `free` | — | supprimé |
| État `gn` | `pico8_gn` (`gen` 50) | `malloc` | celle que choisit l'appelant | désignée par l'appelant |
| Rendu `gn` | `pico8_gn_free` (`gen` 124) | `free` | — | supprimé |
| `jo.oc` (`gn(df,2,…)`) | `pico8_bw` (`galaxy` 91) | par `pico8_gn` | de `eu` au `eu` suivant (remplacé par `bw`) | **partie** (génération courante) |
| Source de `jo.oc` (`jf(0.5,dk.en)`) | `pico8_bw` (`galaxy` 90) | par `pico8_jf` ; gardée par `g_bw_oc_r_ctx` | autant que `jo.oc`, qui la lit à chaque tirage | **partie**, même poignée que `jo.oc` |
| `jo.ir` (`jf(0.8,0.5)`) | `pico8_bw` (`galaxy` 104) | par `pico8_jf` | de `eu` au `eu` suivant | **partie** |
| Remplacement de `jo.oc`/`jo.ir` | `pico8_jo_set_oc`/`_ir` (`planet` 143-157) | libère l'ancien | — | n'en libère plus aucun |
| Remise à nil | `pico8_jo_shutdown` (`planet` 131-141) | libère puis nil | — | nil seulement |
| Rendu de la source | `pico8_bw` (`galaxy` 96-99), `pico8_galaxy_shutdown` (`galaxy` 108-113) | `pico8_jf_free` | — | supprimés, avec `g_bw_oc_r_ctx` |
| `oh` (`jf(0.1,0)`) | `pico8_bg_pq` (`bg` 109, 138) | par `pico8_jf`, libéré en sortant | l'appel de `bg:pq` : local, jamais rangé ni retourné | **frame** |

- **Classement par lecture (§4).** Les trois états de `bw` sont relus par `jo:hh` à n'importe quel moment de la partie (sonde qui dévoile) : la zone frame est exclue, la zone partie les couvre. `oh` n'est lu que par la boucle de `bg:pq` et ne sort pas de l'appel : la lecture prouve la frame, ce qui permet de le mettre dans la zone la plus courte. Aucun site n'est douteux.
- **Pas de nouvelle sorte de particules, pas de site par tick ou par action** : les trois états sont faits une fois par `eu`, et `oh` une fois par `bg` construit. La règle de 4a sur les sites par tick ou par action ne s'applique pas.

### Interface de `pico8_gen`

- `pico8_jf` et `pico8_gn` prennent en **premier** argument la zone, `const pico8_zone_choisie_t *`, comme `pico8_tostr_fixed(zone, …)` et `pico8_concat2(zone, …)`. L'état est pris par `pico8_zone_alloc`. Le reste des arguments et les `_next` ne changent pas.
- `pico8_jf_free` et `pico8_gn_free` sont supprimés. Aucun appelant ne rend un état. Avec une zone à places, l'état ne serait jamais rendu, mais aucun appelant n'en passe une, et 4b n'en prévoit pas.
- **Contrats C**, seuls commentaires réécrits dans `pico8_gen.h` (ils remplacent « The state is the caller's, to release with … ») :
  - l'état est pris dans `zone` et vit autant qu'elle ; il n'y a rien à rendre ;
  - pour `gn` : `hr` et `r_ctx` doivent vivre au moins autant que l'état.

  Le commentaire d'en-tête sur les fermetures (« each … becomes a heap state struct ») dit « heap » : il devient « un état pris dans la zone de l'appelant ».
- `pico8_gen.c` n'inclut plus `<stdlib.h>`. Il inclut l'en-tête de zone choisie.

### Appelants

- **`pico8_bw`** (`pico8_galaxy.c`) :
  - fait `jf(0.5,dk.en)`, `gn(df,2,…)` et `jf(0.8,0.5)` avec `pico8_zone_partie()`, dans le même ordre qu'aujourd'hui (ordre des tirages inchangé : `jf`/`gn` ne tirent rien à la construction) ;
  - passe l'état `gn` et l'état `jo.ir` à `pico8_jo_set_oc`/`_ir`, comme aujourd'hui ;
  - ne garde plus la source de `jo.oc` dans un statique : l'état `gn` la tient par son `r_ctx`, et les deux sont dans la même génération.

  `g_bw_oc_r_ctx` et `pico8_galaxy_shutdown` (déclaration et définition) sont supprimés : la conversion les rend orphelins. `pico8_galaxy_shutdown` n'avait déjà aucun appelant à a182a7f. Le commentaire de `pico8_galaxy.h` qui renvoie à `pico8_jo_shutdown()` pour libérer `jo.oc`/`jo.ir` part avec la fonction.
- **`pico8_jo_set_oc` / `pico8_jo_set_ir`** (`pico8_planet.c`) : affectation seule. Le test d'égalité, qui ne servait qu'à éviter un double `free`, part avec le `free`. Le contrat de `pico8_planet.h` (« Both take ownership and free the previous generator … ») devient : l'état passé doit vivre jusqu'au prochain appel, l'ancien n'est pas libéré, NULL reste le retour valide à « aucun », et seul `bw` les pose.
- **`pico8_jo_shutdown`** : remet `g_jo_oc`/`g_jo_ir` à nil sans les lire. C'est nécessaire, pas seulement plus simple : au chargement, `pico8_jo_init()` → `pico8_jo_shutdown()` passe après `pico8_zone_partie_reset()`. Libérer, ou même lire, l'ancien état y serait un accès à de la mémoire empoisonnée.
- **`pico8_bg_pq`** (`pico8_bg.c`) : `oh = pico8_jf(pico8_zone_frame(), …)`, et l'appel à `pico8_jf_free(oh)` est supprimé. Le commentaire de tête (« `oh` is local to the call and freed here ») perd « and freed here ».
  - *Alternative écartée* : un `pico8_jf_state_t` sur la pile, rempli par une variante d'initialisation. Ce serait plus court, mais cela ajouterait une deuxième fabrique pour un seul appelant et s'écarterait du *Reçu*, qui range `oh` en zone frame.
- **`pico8_cart.c` n'est pas touché.** `pico8_eu` appelle `bw` comme aujourd'hui. La bascule qui précède est de 4a.

### Inventaire du §4 pour 4b

4b est la première famille à mettre des objets dans une génération de partie. La durée de vie qui finit est celle de 4a : une génération est rendue par `pico8_zone_partie_rendre()` au vidage qui suit le `eu` suivant, après `oublier_partie()`, ou remise à zéro par `pico8_zone_partie_reset()` au chargement et à l'arrêt.

**Méthode.** Refaire **mécaniquement**, sur le code du commit de 4b, les deux `grep` de 4a sur les fichiers de `CORE_SRCS` :

- `grep -nE '^\s*static\s[^(]*\*\s*[a-zA-Z_][a-zA-Z0-9_]*(\[[^]]*\])*\s*(;|=)'` (hors `const char *` et tables `* const`) ;
- `grep -nE '^static\s+(struct\s+)?[a-z0-9_]+(_t)?\s+[a-zA-Z_][a-zA-Z0-9_]*\s*(\[[^]]*\])*\s*(;|=)'` (hors `const` et scalaires), dont on lit les champs pointeurs.

Ajouter ensuite un `grep` des types `pico8_jf_state_t` et `pico8_gn_state_t` dans `src/` : il trouve tout champ ou local qui peut garder un état. Comparer avec la table de la PRD 4a et avec les statiques que 4a a ajoutés (`pico8_zone_partie.c` : `partie`, `partie_handle`, `partie_choisie`). Tout pointeur nouveau est classé dans « Découvert ».

**Relevé attendu** (fait à a182a7f et refait à b46601e : les types n'apparaissent que dans `pico8_gen.*`, `pico8_planet.*`, `pico8_galaxy.*` et `pico8_bg.c`) :

| Pointeur | Peut viser | Statut après le rendu de la génération |
|---|---|---|
| `g_jo_oc`, `g_jo_ir` (`planet`) | l'état `gn` / `jf` de la génération où `bw` les a faits | **repointés** par `bw` dans `eu`, après `pico8_zone_partie_basculer()` et avant le rendu du vidage suivant. Entre la bascule et `bw`, seuls `jl()` et la construction de `bg` s'exécutent, et aucun ne lit `jo.oc`/`jo.ir`. La boucle des planètes de `bw` ne les lit pas non plus (dans le cart, `jo.oc` vaut nil à la première partie, donc `jo:hh` ne peut pas y être appelé). Au chargement et à l'arrêt : **remis à nil** par `pico8_jo_shutdown`, sans lecture. |
| `r_ctx` de l'état `gn` | la source `jf` | ne peut pas viser une autre génération : faite par le même `bw` avec la même poignée ; elle est rendue en même temps que l'état qui la lit |
| `g_bw_oc_r_ctx` (`galaxy`) | — | **supprimé** |
| `partie_choisie`, `partie_handle`, `partie` (`zone_partie`, ajoutés par 4a) | la poignée et la zone elles-mêmes | ne peuvent pas viser un état (type) |
| `oh` (local de `bg:pq`) | état de la zone frame | ne survit pas à l'appel |

- Entre `_init` et `eu` (écran titre), `jo.oc`/`jo.ir` désignent les états de la partie quittée. Ils restent vivants, car `_init` ne bascule pas. Rien ne les lit au titre : `jl()` a vidé `hv`, il n'y a pas de sonde. C'est le même cas que `fm`/`ju` dans 4a.
- Angle mort ASan (4a) : un pointeur gardé à travers deux nouvelles parties ne planterait pas. Il ne peut pas exister ici, puisque `bw` réécrit les deux pointeurs à chaque `eu`.
- **Conclusion : le point d'oubli de partie (`oublier_partie()`) reste vide.** `pico8_jo_shutdown` couvre le chargement et l'arrêt, et `bw` couvre la nouvelle partie. Un pointeur trouvé ensuite par ASan est un trou de cet inventaire, noté dans « Découvert » et coupé au point d'oubli.

### Verrou

- `tools/alloc_guard_exceptions.txt` : la ligne `src/pico8_gen.c 4` est retirée. Les lignes `galaxy` 3, `bg` 2 et `planet` 6 ne bougent pas.
- **Prévision : 24 fichiers / 84 appels → 23 / 80.** La baisse est une estimation (§5). Elle part de la mesure faite au `HEAD` de 4b avant la conversion, qui redonne 24 / 84 à b46601e (65 objets). Un écart se note dans « Découvert ».

### Budget : mesure, projection, décision

- **Prévision exacte, x86-64** : `CELLS(8) + CELLS(592) + CELLS(8)` = 8 + 592 + 8 = **608 o par génération**.
  - Sur les 25 e2e à un seul `eu` : `max_octets partie 608`, `partie 608`.
  - Sur `pause_restart` (deux `eu`, f0 et f12) : les deux générations sont présentes de f12 au rendu de f13, donc `max_octets partie 1216` et `partie 608` en fin de session.
  - `game_end_final_score` finit sur `_init` (« again? ») sans second `eu` : 608 / 608.
- **Mesure.** Après conversion, rejouer les 26 e2e avec `PICO8_REMPLISSAGE` (méthode de 4a, étape 1 ; la forme est laissée à l'implémenteur : boucle de shell citée dans le commit, ou cible hors suite) et relever `M_4b` = maximum des `max_octets partie`. Si `M_4b` ≠ 1216, l'écart est expliqué **avant** la projection : objet entré par erreur, cellule différente, en-tête. Un écart inexpliqué bloque l'étape.
- **Projection** (règle de la PRD 4a, relue à a182a7f ; la note « Découvert (4a) » ne l'a pas changée) : `P_4b = M_4b + Σ estimations restantes × r`.
  - Estimations de 4a pour les familles restantes : 4c ~20, 4d ~160, 4e ~30, 4f ~1, 4g ~38, 4h ~12, soit **261 Ko**.
  - `r` = max(1, mesuré / estimé) = max(1, 1216 o / ~2 Ko) = **1**.
  - `P_4b` ≈ 1,2 + 261 ≈ **262 Ko**.
- **Décision**, comparée à `PICO8_ZONE_GAME_BYTES` (768 Ko) et à la marge (49 Ko, `pico8_zone_sizes.h` à b46601e) :
  - `P_4b ≤ 768 Ko` : on continue. C'est la prévision ;
  - entre 768 et 817 Ko : 4b relève `PICO8_ZONE_GAME_BYTES` d'un chiffre dans son commit ;
  - au-delà : **arrêt de l'étape 4 à 4b**, et signalement à l'humain (zone partie, limite voulue = `P_4b` arrondi, somme actuelle des zones 1487 Ko sur 1536). On ne rogne aucune autre zone (§3).
- `PICO8_ZONE_GAME_BYTES` n'est pas touché si la prévision tient.

### Note « Découvert » et clôture

À la clôture, sous l'entrée 4 de `docs/plan-4.md`, une note « *Découvert (4b) :* » d'une poignée de phrases. Elle dit :

- `jf`/`gn` prennent la zone de l'appelant ;
- `jo.oc`, `jo.ir` et la source de `jo.oc` sont en zone partie, `oh` en zone frame ;
- `g_bw_oc_r_ctx`, `pico8_galaxy_shutdown`, `pico8_jf_free` et `pico8_gn_free` sont supprimés ;
- l'écart de la ligne de 4a (`jo:pq` pour `set_oc`/`set_ir`/`shutdown`/`hh`) ;
- l'inventaire et le point d'oubli resté vide ;
- `M_4b`, par e2e s'il diffère de la prévision, puis `P_4b` et la décision ;
- les comptes du verrou laissés (prévision 23 / 80).

Le tableau du §2 n'est pas contredit : rien à corriger. La PRD est retirée de `docs/prd/` dans le commit de clôture, comme pour les étapes précédentes (`docs/` ne garde que ce qui est vrai ou à faire).

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 : `make test-all`, `make sanitize-net`, 26 `*.sha256` identiques, `alloc-guard` à égalité stricte. La conversion ne peut pas être coupée : passer temporairement la transitoire à `bw` ferait monter `galaxy`, ce que le cliquet interdit après 0b.

1. **Conversion, tests rouges d'abord.**
   - Les attentes rouges : octets de la zone partie à f12 et f13 de `pause_restart`, et maximum (2G / G / 2G, `G` calculé par `sizeof`) ; valeurs exactes du cas `prise_and_max` de `test_pico8_zone_partie.c`, qui comptent désormais `G` par `eu`.
   - Le cas `jo:hh` après f13, sous ASan : vert avant la conversion, puisque `malloc` ne s'empoisonne pas, mais il garde la régression.
   - La nouvelle signature de `pico8_jf`/`pico8_gn`. La suppression de `pico8_jf_free`/`pico8_gn_free`, de `g_bw_oc_r_ctx` et de `pico8_galaxy_shutdown`.
   - Les appelants : `bw` sur la poignée de partie, `bg:pq` sur la zone frame, `set_oc`/`set_ir`/`shutdown` sans `free`.
   - `test_pico8_map_tables` adapté. Les contrats C réécrits.
   - `tools/alloc_guard_exceptions.txt` sans `pico8_gen.c`.
2. **Mesure et clôture.**
   - Le relevé sur les 26 e2e, `M_4b`, `P_4b` et la décision, dans le message du commit.
   - L'inventaire refait par `grep`.
   - La note « Découvert » dans `docs/plan-4.md`, et le retrait de cette PRD.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris et le maximum de la zone partie, par son accesseur ou par les lignes écrites sous `PICO8_REMPLISSAGE` ;
  - le type d'une planète dévoilée ;
  - un rapport ASan dans un processus enfant ;
  - la sortie d'`alloc-guard` ;
  - l'empreinte d'un e2e.

  Il ne lit ni `g_jo_oc` ni les bouts de la zone. Les tailles attendues se calculent dans le TU à partir de `sizeof(pico8_jf_state_t)`, `sizeof(pico8_gn_state_t)` et de la cellule de zone, jamais en dur.
- **`tests/test_pico8_places_vivantes.c`** (ASan, un enfant par cas, piloté comme `main.c`) :
  - **remplissage écrit par le rejeu** : le cas `le_rejeu_ecrit_le_remplissage_en_fin_de_session` reste **inchangé**. Il compare le fichier écrit aux accesseurs lus dans le même processus, sans valeur en dur. La valeur exacte se prouve par le cas suivant, sur les accesseurs ;
  - **bascule et rendu sur de vrais objets** (cas nouveau). Soit `G` = `2 × CELLS(sizeof(pico8_jf_state_t)) + CELLS(sizeof(pico8_gn_state_t))` cellules de `sizeof(pico8_zone_cell_t)` octets. Sur `pause_restart`, avec les aides `demarrer` et `jouer_jusqu_a` de 4a :
    - après f11, `pico8_zone_partie_prise()` vaut `G` ;
    - après f12 (bascule et `bw` faits, rendu pas encore), elle vaut `2G` ;
    - après f13, elle vaut `G`, et `pico8_zone_partie_max()` vaut `2G`.

    Comme pour la sentinelle de 4a, on joue sans passer par `relever`, dont le vidage rendrait la génération lui-même. Sur x86-64, cela donne 608 / 1216 / 608 o ;
  - **`jo:hh` après une nouvelle partie** : après f13 de `pause_restart`, dévoiler par `pico8_jo_hh` quelques planètes « unknown » de `cg.jo` (cinq au plus : `oc` n'a que 66 tickets), puis vérifier qu'au moins une a un type autre que `df.ll`, ce qui prouve que `jo.oc()` a été lu. L'enfant ne doit pas mourir : `jo.ir` et `jo.oc` désignent la nouvelle génération. Si `bw` ne les repointait pas, ASan planterait sur la génération rendue en f13 ;
  - **sentinelle de 4a** : les trois cas restent inchangés. Ils n'imposent que `prise >= prise d'avant`, la survie en f12, la mort en f13 et la survie à « restart ». En f13, la génération rendue contient désormais, en plus de la sentinelle, les états de la première partie ;
  - **croissance par frame** : le relevé f45 = f90 sur `message_box_hold` reste égal, désormais à une génération d'états au lieu de zéro.
- **`tests/test_pico8_zone_partie.c`**, cas `prise_and_max` : c'est la **seule attente existante que 4b change**. Le `pico8_eu(NULL)` du cas fait désormais un `bw`, qui prend `G` octets dans la nouvelle génération. Les attentes deviennent :
  - 0 après chargement et `_init` ;
  - 64 après le premier bloc ;
  - `128 + G` après `eu` et le second bloc ;
  - `64 + G` après l'update qui rend l'ancienne génération ;
  - maximum `128 + G` ;
  - après `cart_shutdown`, 0, et le maximum reste `128 + G`.

  `G` est calculé par `sizeof`, comme ci-dessus. Les autres cas de ce TU (vivant à travers `eu`, mort après l'update, `_init` sans bascule, mort après `cart_load`, double `eu` refusé) restent inchangés : ils ne comptent pas d'octets.
- **`tests/test_pico8_map_tables.c`** (`test_gn_pools_df_entries_by_their_r`) : `pico8_gn(pico8_zone_frame(), …)`, sans `pico8_gn_free`. L'attente (66 tickets, répartis selon `r`) ne change pas.
- **`tests/test_pico8_bg.c`** et les TU qui construisent un `bg` : verts sans changement. `oh` passe en zone frame, et les tuiles peintes sont les mêmes.
- **Verrou** : `make alloc-guard` à égalité stricte, liste sans `pico8_gen.c`. Pas de nouveau cas dans `tests/test_alloc_guard.c`.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. Sur `pause_restart`, la génération rendue à f13 contient pour la première fois des objets du jeu (les trois états de la première partie).
- **Pas de TU propre à `pico8_gen`** : il n'en existe pas, et la prise dans la zone est observée par le remplissage de la zone partie, sur rejeu. En ajouter un ne montrerait que ce qu'`alloc-guard` et le remplissage montrent déjà.
- **Exemples dans le dépôt** :
  - `tests/test_pico8_places_vivantes.c` : rejeu par enfant, relevés, remplissage écrit par le rejeu, et la sentinelle de 4a avec `demarrer`/`jouer_jusqu_a` ;
  - `tests/test_pico8_zone_partie.c` : cas en processus enfant relancé par nom, `use-after-poison` attendu ;
  - `tests/test_pico8_zone_particules.c` : lecture périmée dans un enfant ;
  - `tests/test_pico8_map_tables.c`.

## Out of Scope

- Les autres boîtes et identifiants faits par `bw` (`p`, `ra`, « p… », `galaxy` 3) : famille 4d. `galaxy` reste dans la liste.
- La structure `bg` et sa boîte `v` de `bg_hq` (`bg` 2) : famille 4h.
- Les structures de planète et `jo:pq` (`planet` 6) : famille 4d.
- Toute modification de la zone partie, de sa bascule, de son rendu, de son point d'oubli ou de son remplissage : 4a.
- Relever `PICO8_ZONE_GAME_BYTES`, sauf si la projection l'exige (voir « Budget »).
- Retirer TROU-7 : étape 5. Aucun site de 4b ne le cite.
- Les sorties et le glossaire : `jf`/`gn` gardent leur entrée dans `docs/glossaire-noms.md`, dont le sens ne change pas.
- Lancer l'application raylib.

## Vérifié contre 4a

Relu à `HEAD` b46601e, sans toucher git :

- `git log -6 --stat` : bd372fe (zone à deux bouts et sa poignée), 10d368e (module, bascule, rendu, reset), 646f7fc (remplissage écrit, preuves sur `pause_restart`), b46601e (clôture) ;
- dans `docs/plan-4.md` : la note « *Découvert (4a)* », la ligne « *Reçu de l'étape 4a* », les lignes « Partie » et « Particules » du §2, et la liste ordonnée des familles dans la puce « 4b et suivantes » ;
- les sources et les TU : `src/pico8_zone_partie.{h,c}`, `src/pico8_zone.{h,c}` (zone à deux bouts), `src/pico8_zone_choisie.{h,c}`, `src/pico8_cart.c`, `src/main.c`, `tests/test_pico8_places_vivantes.c`, `tests/test_pico8_zone_partie.c`, `tests/test_pico8_hy.c` ;
- `make alloc-guard` sur une copie `git archive HEAD`.

Les hypothèses du brouillon, une par une :

- **H-4a-1 (poignée de partie) — tenue.**
  - Le module est `pico8_zone_partie.{h,c}`, et `pico8_zone_partie()` rend un `const pico8_zone_choisie_t *` sur la génération courante.
  - Avec la poignée, le module exporte `_basculer`, `_en_attente`, `_rendre`, `_reset`, `_prise` et `_max`.
  - Comportement de la poignée : `alloc` prend au bout courant, `grow` copie, `release` ne fait rien (`bump_release`).
  - Le module est hors liste d'exceptions.
- **H-4a-2 (point de bascule) — tenue.** `pico8_zone_partie_basculer()` est appelée juste après `pico8_economy_set_dk(nj)`, avant `pico8_jl()`, `pico8_bg_new()` et `pico8_bw()`. `_init` ne bascule pas (cas `init_keeps_generation`, et variante « restart » de `pause_restart`).
- **H-4a-3 (point de rendu) — tenue.**
  - `pico8_cart_vider_file_sortie()` vide la file, puis, seulement si une génération attend, appelle `oublier_partie()` et `pico8_zone_partie_rendre()`.
  - Sur `pause_restart`, le rendu tombe en tête de l'update de f13 : la sentinelle de 4a y meurt par `use-after-poison`.
- **H-4a-4 (point d'oubli) — tenue.** `oublier_partie()`, statique de `pico8_cart.c`, est vide.
- **H-4a-5 (chargement et arrêt) — tenue.**
  - `pico8_cart_load()` fait : vidage, `pico8_zone_partie_reset()`, puis plus loin `pico8_jo_init()` (donc `pico8_jo_shutdown()`).
  - `pico8_cart_shutdown()` fait : vidage, reset, puis plus loin `pico8_jo_shutdown()`.
  - On alloue sans bascule préalable : après `cart_load` + `_init`, une allocation va au bout bas (cas `prise_and_max`). La première bascule exige seulement que le bout d'arrivée soit vide.
- **H-4a-6 (comptage) — tenue.**
  - `pico8_bascule_zone_alloc` arrondit à `PICO8_ZONE_CELLS(size) * sizeof(pico8_zone_cell_t)` (8 o), sans en-tête.
  - `_prise` vaut `bas + haut`, et le maximum ne se remet qu'avec l'init.
  - Aucun appel à `pico8_zone_partie()` dans `src/` hors le module lui-même : la zone ne contient aucun objet du jeu.
- **H-4a-7 (remplissage écrit) — tenue.** `write_remplissage` finit par `partie %lu` et `max_octets partie %lu`.
- **H-4a-8 (TU de 4a) — tombée, sur la forme.** Ce qu'on avait supposé :
  - *le cas du remplissage attend `partie 0`* : non. `le_rejeu_ecrit_le_remplissage_en_fin_de_session` rejoue jusqu'à f17 et compare le fichier écrit aux accesseurs lus dans le même processus. Il reste vert sans changement : il ne prouve aucune valeur, seulement la forme ;
  - *la sentinelle compare des octets exacts* : non. Elle n'impose que `prise >= prise d'avant`, et 4b ne la touche pas ;
  - *aucun autre TU ne compte les octets exacts* : faux. **`tests/test_pico8_zone_partie.c`, cas `prise_and_max`, attend des valeurs exactes à travers un `pico8_eu(NULL)`** : 0, 64, 128, 64, max 128, puis 0 et max 128 après `cart_shutdown`. Après 4b, le `bw` de cet `eu` ajoute une génération d'états. Ce cas casse, et Testing Decisions le reprend.
  - Le relevé f45 = f90 de `message_box_hold` existe (`r[0].partie == r[1].partie`).
  - Les aides `jouer_jusqu_a` et `demarrer` de `test_pico8_places_vivantes.c` rejouent frame à frame sans passer par `relever` : 4b les réutilise.
- **H-4a-9 (double bascule) — tenue.** L'`assert` dit « … vider la file de sortie entre deux eu ». `test_pico8_hy` vide la file dans `test_construction` et `test_lj`.
- **H-4a-10 (budget) — tenue, précisée.**
  - `PICO8_ZONE_GAME_BYTES` vaut 768 Ko, la somme 1487 Ko sur 1536, la marge 49 Ko (`pico8_zone_sizes.h` n'a pas changé).
  - La note « *Découvert (4a)* » ne recopie ni les estimations par famille ni la règle de projection. Elles restent dans la PRD 4a retirée (`git show a182a7f:docs/prd/triage/phase1-etape4a-socle-zone-partie.md`) et rien ne les a changées. `P_4b` s'en sert telles quelles, et la note « Découvert » de 4b cite leur source.
- **H-4a-11 (verrou) — tenue.** Mesure : **65 objets, 24 allouent, 84 appels**. Il y a un objet de plus (`pico8_zone_partie.o`, qui n'alloue pas), et la liste est identique. `pico8_gen.c` n'a pas changé : 4 appels aux lignes 9, 26, 50 et 124.
- **H-4a-12 (Makefile) — tenue.** `pico8_zone_partie.c` est dans `ENGINE_SRCS` et sur les deux lignes de `main_raylib`. `pico8_galaxy.c` n'est pas dans `DATA_ENGINE_SRCS`.
- **H-4a-13 (Reçu) — tenue.** La seule ligne « *Reçu de l'étape 4a* » confie les traînées `ts` à 4e. La liste ordonnée met « 4b états de `pico8_gen.c` » en tête.

**Ce que la vérification change dans cette PRD** : Testing Decisions (le cas `prise_and_max` de `test_pico8_zone_partie.c` à adapter ; le cas du remplissage inchangé, avec une valeur exacte prouvée ailleurs), le morceau 1, et les renvois aux hypothèses, remplacés par les faits. L'inventaire ajoute les statiques du module de zone partie. La prévision de 608 / 1216 o tient.

## Further Notes

- **Pas d'humain disponible, pas d'outil `Agent` dans cette session.** J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage) :
  - **Hypothèses explicites** : rédigée pendant 4a, la PRD posait ce qu'elle supposait de 4a en H-4a-1 à H-4a-13. Toutes sont vérifiées à b46601e (section « Vérifié contre 4a ») : 12 tiennent, H-4a-8 tombe sur la forme (TU exacts dans `test_pico8_zone_partie.c`). L'écart de la ligne de 4a (`jo:pq`) est relevé, pas corrigé en silence.
  - **Simplicité** :
    - un seul argument ajouté à deux fonctions ;
    - quatre fonctions ou statiques supprimés parce que la conversion les rend orphelins ;
    - pas de variante sur la pile pour `oh` ;
    - pas de TU propre à `pico8_gen`.
  - **Changements chirurgicaux** :
    - quatre fichiers de source (`gen`, `galaxy`, `planet`, `bg`) et leurs en-têtes ;
    - deux TU adaptés (`test_pico8_map_tables`, `prise_and_max` de `test_pico8_zone_partie`), deux cas ajoutés dans `test_pico8_places_vivantes.c` ;
    - une ligne retirée de la liste du verrou ;
    - `pico8_cart.c` intact.
  - **Critères vérifiables** :
    - 23 / 80 au verrou ;
    - `G` / `2G` / `G` octets (608 / 1216 / 608 sur x86-64) après f11, f12 et f13 de `pause_restart`, et `M_4b` = 1216 relevé sur les 26 e2e ;
    - deux générations après f12, une après f13 ;
    - `jo:hh` sans crash après f13 ;
    - 26 `*.sha256` inchangés et `sanitize-net` vert ;
    - `P_4b` ≈ 262 Ko ≤ 768 Ko.
- **Ce que 4b éprouve pour la suite.** C'est la première génération rendue qui contient des objets du jeu. Si l'ordre bascule → `bw` → vidage → rendu de 4a avait un trou, ASan le montrerait ici sur `pause_restart` (le TU de `jo:hh`), sur trois objets faciles à suivre, avant que 4c n'y mette les registres de toutes les entités.
- **Pourquoi `r` reste à 1.** 4b mesure 1216 o pour une estimation de ~2 Ko. Une famille si petite ne dit rien de l'erreur des autres estimations : la règle de 4a (`r` = max(1, …)) l'empêche de faire baisser la projection, et c'est voulu.
