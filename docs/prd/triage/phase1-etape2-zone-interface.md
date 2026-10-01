---
title: "Plan 4, étape 2 — Zone interface : boutons, panneaux, boîtes et toasts rendent leur place"
description: "Les boutons js, fonds fq, boîtes mh, toasts iv, panneaux lq/jb et listes le de l'interface sont pris à la libc ou à la transitoire et jamais rendus (TROU-7) : l'étape 2 leur donne d'abord un point de libération, sur la transitoire (2a), puis les range dans des zones à places de taille fixe (2b)."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. Le texte de l'étape, recopié du plan :

> 2. **Zone interface.**
>
> 3. **Zone particules.**
>
>    Ces deux étapes créent un point de libération qui n'existe pas aujourd'hui (TROU-7 : panneaux, messages et particules fuient). Elles ne sont finies que si chaque sorte convertie rend sa place, ce qui se montre par le remplissage du §3 : après une session scriptée, le nombre de places vivantes revient à son niveau de départ, et il ne monte pas pendant une phase sans action. Dans une zone à places, un `pico8_fatal` est d'abord lu comme une place jamais rendue ; la limite n'est relevée qu'une fois ce cas écarté, et la note « Découvert » dit lequel des deux c'était.
>
>    Chacune se fait en deux temps, chacun sa PRD ou son lot de commits :
>    - **a. Points de libération, sur la transitoire.** Chaque sorte de l'étape (et chaque champ réaffecté qui lui revient, TROU-7) gagne sa libération, la transitoire encore derrière : sous ASan, la quarantaine de la libc attrape tout usage après libération. Le critère des places vivantes est montré ici, sur un compte de vivants par sorte.
>    - **b. Conversion.** Les mêmes sites passent à la zone à places, sans ajouter ni déplacer de point de libération : un `sanitize-net` rouge en b vient de la zone, pas d'une libération manquante.

Cette PRD ne traite que l'étape 2. Les règles qui la cadrent sont celles du plan :

- §2 : la zone interface contient « panneaux `lq`, messages `iv`/`mh`, menus, boutons » ; elle a un « nombre maximum d'objets vivants ; une place est rendue quand l'objet quitte l'écran » ;
- §3 : limites au doigt mouillé, somme des zones ≤ 1,5 Mo, limite atteinte = `pico8_fatal`, invariant de chaque commit (`make test-all`, `make sanitize-net`, `*.sha256` identiques), empoisonnement ASan d'une place rendue, et contrôle « Croissance par frame » (remplissage égal à deux frames d'un e2e séparées par une phase sans action) ;
- §4 : un site douteux va dans la zone la plus longue qui le couvre ; ASan ne fait que rétrograder, il ne voit ni un pointeur gardé à travers deux remises à zéro ni une place rendue un tour complet plus tôt ;
- §5 : fin d'étape = chaque site de l'inventaire converti (vérifié sur la source), `alloc-guard` à égalité stricte, aucun compte n'a monté ; la baisse prévue n'est qu'une estimation.

**Ce qui se passe aujourd'hui.** Aucun objet d'interface n'est jamais rendu :

- un bouton `js` cliqué disparaît de l'écran par `ui:cj` (`he`), un toast `iv` ou une boîte `mh` par `p_` (`dg` → `he`), un fond `fq` par `p_` ; leur struct reste allouée ;
- leurs morceaux aussi : le `lq`, le `h_`, la bulle `jb` et la boîte `kq` d'un bouton, les tableaux d'un panneau, les textes et tuples que ses cases désignent ;
- à chaque tick, `js:ph` et `iv:ph` allouent une nouvelle boîte `ra` sans rendre l'ancienne ;
- à chaque `hy:lj`, `jo:hb` refait la rangée `lq` de chacune des ~90 planètes et abandonne l'ancienne ;
- chaque `ui:cj` et chaque `pz` abandonnent la liste `ok.le` précédente ;
- `jl()` (nouvelle partie, « restart ») abandonne d'un coup tout ce qui vivait.

Une partie longue croît donc sans borne en mémoire d'interface. Sur PSX, où la RAM est bornée, cela ne peut pas tenir.

## Solution

L'étape 2 donne à chaque **sorte d'interface** une place dans une zone à places de taille fixe, et un **point de libération** qui rend cette place quand l'objet quitte l'écran. Elle se fait en deux lots de commits, dans l'ordre imposé par le plan.

**2a — Points de libération, sur la transitoire.**

- **Une place par objet.** Chaque objet d'interface (« racine ») prend **une place** de taille fixe propre à sa sorte. Cette place est une petite **arène** : l'objet y range sa struct et tout ce qu'il possède (boîtes, panneaux fixés à sa construction, tableaux, copies de textes). Le rendre, c'est rendre la place, d'un seul geste. En 2a, la place est un bloc pris à la transitoire.
- **Une file de sortie.** Un objet qui quitte l'écran (retiré par `he`, ou abandonné par `jl`) n'est pas rendu sur-le-champ, parce que la frame en cours le relit encore (`dg` écrit son `t` après `he`, `ui:cj` et `ui:jz` le parcourent, `ok.gj` le désigne). Il entre dans une **file de sortie**, vidée en tête du `pico8__update60()` suivant.
- **Des objets qui possèdent ce qu'ils montrent.** Un panneau recopie dans sa zone les textes de ses cases. Une boîte `mh` recopie ses articles. Un toast `iv` garde une copie de son texte. Un bouton et un toast tiennent leur boîte `ra` dans leur struct au lieu d'en allouer une par tick. Les textes, tuples et rangées que les constructeurs de panneaux fabriquent pour un seul usage passent donc en **zone frame** (étape 1).
- **Un compte de vivants par sorte**, lu par un accesseur de remplissage. Il montre le critère du plan : après `pause_restart`, les places vivantes reviennent au niveau de l'écran titre ; pendant la phase sans action de `message_box_hold`, elles ne bougent pas.

**2b — Conversion.**

- Un module de zone interface possède une zone à places par sorte (taille et nombre fixés à la compilation, somme vérifiée).
- Les poignées des sortes passent de la transitoire à ces zones, sans ajouter ni déplacer de point de libération.
- Les fichiers d'interface sortent de la liste d'exceptions du verrou.

**Pas de changement visible.** Les 26 `*.sha256` restent identiques à chaque commit, et `make test-all` et `make sanitize-net` passent.

## User Stories

1. En tant que mainteneur du plan, je veux que chaque bouton, fond modal, boîte de message, toast et panneau d'interface rende sa mémoire quand il quitte l'écran, pour qu'une partie longue ne croisse plus en mémoire d'interface.
2. En tant que futur porteur PSX, je veux que la mémoire d'interface vive dans des zones à places de taille connue à la compilation, pour que son plafond se lise dans un en-tête et compte dans les 1,5 Mo.
3. En tant que développeur, je veux qu'un objet d'interface et tout ce qu'il possède tiennent dans une seule place, pour que le rendre soit un seul appel et qu'aucun morceau ne fuie.
4. En tant que développeur, je veux qu'un objet retiré par `he` ne soit rendu qu'en tête de la frame suivante, pour que `dg`, `ui:cj`, `ui:jz` et `ok.gj`, qui le relisent encore pendant la frame en cours, ne lisent pas une place rendue.
5. En tant que développeur, je veux que `jl()` mette dans la file de sortie tout objet encore inscrit dans `hv` qui possède de la mémoire d'interface, pour que « restart » et une nouvelle partie rendent les boutons de titre, de pause, de `ix` et du score.
6. En tant que développeur, je veux que `jl()` appelé au milieu d'un clic (« restart » du menu pause) ne rende rien avant la fin de la frame, pour que l'ancien `ok`, qui fait encore son `cj` sur ses anciens boutons, ne lise pas une place rendue.
7. En tant que développeur, je veux que `he` ne mette un objet dans la file que s'il était dans `hv`, pour qu'un `cj` de l'ancien `ok` après `jl()` ne rende pas deux fois le même bouton.
8. En tant que développeur, je veux qu'à la sortie d'un objet, les références globales qui le désignent encore (`ok.o_`, `ok.gj`, `kc`, `ju.ed`) soient remises à nil, pour qu'une nouvelle place à la même adresse ne soit jamais prise pour l'ancienne (`ju.ed == kc`) ni écrite (`kc.p_`, `gj.m_`).
9. En tant que développeur, je veux que la file de sortie ait une capacité fixe et sorte par `pico8_fatal` en la nommant si elle déborde, pour qu'elle n'appelle jamais la libc.
10. En tant que développeur, je veux qu'un bouton `js` prenne une place qui contient sa struct, sa boîte `kq`, son `lq`, son `h_` et sa bulle `jb` avec ses lignes, pour que `ui:cj` rende le tout.
11. En tant que développeur, je veux que `js:ph` écrive la position glissée dans une boîte tenue par le bouton, au lieu d'allouer une boîte par tick, pour que la croissance par frame d'un menu ouvert soit nulle.
12. En tant que développeur, je veux qu'un bouton recopie à sa construction la valeur de son `ra` et de son `qf`, pour que `pz` et `pj:ph` n'allouent plus de boîte partagée ni de boîte par bouton.
13. En tant que développeur, je veux qu'un toast `iv` garde son `ra` emprunté jusqu'à son premier `ph`, puis écrive dans une boîte qu'il tient, pour qu'un toast posé sur une colonie qui flotte (`na`) bouge exactement comme aujourd'hui.
14. En tant que développeur, je veux qu'un toast `iv` recopie son texte dans sa place, pour que le `ne` de `hy:lu` et de `jo:hk` soit fait en zone frame et que `"-no money-"`, littéral, ne soit jamais libéré.
15. En tant que développeur, je veux qu'un fond `fq` prenne une place et la rende quand son `my` lève `p_`, pour que fermer le panneau de score rende son fond.
16. En tant que développeur, je veux qu'une boîte `mh` recopie ses articles (textes, sous-rangées, et le `lq` d'un article `ld`) dans sa place, pour que les textes de `pj:hk` et de `pj:ph` soient faits en zone frame et que deux boîtes vivantes à la même frame gardent chacune leurs textes.
17. En tant que développeur, je veux que `lq_new`, `lq_copy`, `lq_add` et `jb_new` prennent leur struct dans la zone passée et y recopient les textes de leurs cases, pour qu'un panneau ne dépende plus d'un texte que son créateur devrait garder vivant.
18. En tant que développeur des panneaux, je veux que les textes et tuples fabriqués pour une case (`gi_str`, `label` et `ep` de `cr`, `jo_dollar`, `combined` et `mu_str` de `jo:gh`, la jointure de `hp:jb`, les tuples et `jq_str` de `jo.ex`, `jo_kw_tuple`) soient faits en zone frame, pour supprimer leur fuite TROU-7.
19. En tant que développeur, je veux que `jo.ex` prenne la zone de son résultat en paramètre, pour que `jo:hb` le fasse dans une place de rangée et que `hp:lq`, recopié aussitôt par `js:pq`, le fasse en zone frame.
20. En tant que développeur, je veux que la rangée `jo.lq` d'une planète prenne sa propre place, rendue dès que `jo:hb` la remplace, pour que les ~90 rangées refaites à chaque `hy:lj` ne s'accumulent pas.
21. En tant que développeur, je veux que la rangée `jo.lq` d'une planète soit aussi rendue quand la planète quitte `hv` (`jo:ll`, `jl`), pour que la planète, objet de partie, ne garde pas une place d'interface.
22. En tant que développeur du menu, je veux que le `lq` refait par `ix[2].hb` prenne sa propre place, rendue au remplacement suivant, pour que l'icône de recherche ne fuie plus à chaque `hy:lj`.
23. En tant que développeur, je veux que chaque liste `ok.le` prenne une place, et que la liste remplacée par `cj` ou `pz` passe par la file de sortie, pour que la comparaison d'identité `self.le == le` de `ui:lj` ne soit jamais faussée par une adresse réutilisée dans la même frame.
24. En tant que développeur, je veux que la liste statique de départ de `ok.le` ne soit jamais rendue, pour qu'un `cj` sur un `ok` neuf ne rende pas de la mémoire statique.
25. En tant que développeur du panneau de score, je veux que les textes de `pj:hb` (crédits, revenu, `+po`, bonheur) soient recopiés dans l'état de `pj`, pour qu'ils soient faits en zone frame au lieu de fuir à chaque `hy:lj`.
26. En tant que mainteneur, je veux un compte de places vivantes par sorte, lisible par un accesseur, pour montrer en 2a comme en 2b qu'une session revient à son niveau de départ.
27. En tant que mainteneur, je veux un TU qui rejoue `pause_restart` et compare les places vivantes à l'écran titre avant la partie et après « restart », pour prouver que `jl()` rend tout ce qu'il abandonne.
28. En tant que mainteneur, je veux un TU qui rejoue `message_box_hold` et compare les places vivantes à deux frames de sa phase sans action, pour prouver qu'aucune sorte n'alloue par tick.
29. En tant que mainteneur, je veux que le rejeu puisse écrire le remplissage de chaque sorte en fin de session, sans changer ni la capture ni le dump, pour lire un taux sans écrire de test.
30. En tant que développeur qui chasse un accès hors durée de vie, je veux qu'en 2a la place rendue soit un bloc libéré par la transitoire, pour que la quarantaine d'ASan attrape toute lecture d'un objet sorti.
31. En tant que développeur, je veux qu'en 2b une place rendue soit empoisonnée par la zone à places, pour que `make sanitize-net` attrape une lecture tardive, dans la limite d'un tour complet du curseur.
32. En tant que développeur, je veux qu'un `pico8_fatal` « zone interface <sorte> pleine » soit d'abord lu comme une place jamais rendue, pour ne relever un nombre de places qu'une fois ce cas écarté.
33. En tant que développeur, je veux qu'un `pico8_fatal` « zone place <sorte> pleine » nomme l'arène trop petite, pour relever la taille d'une place en un chiffre.
34. En tant que mainteneur du budget, je veux une vérification à la compilation que la somme des zones à places d'interface tient dans `PICO8_ZONE_INTERFACE_BYTES`, pour que le plafond de 1,5 Mo reste le seul garde-fou.
35. En tant que mainteneur du budget, je veux que relever `PICO8_ZONE_INTERFACE_BYTES` au-delà de ce que permet la somme de 1,5 Mo arrête l'étape et le signale (zone, limite voulue, somme actuelle), pour ne jamais rogner une autre zone.
36. En tant que gardien du cliquet, je veux qu'aucun commit de l'étape ne fasse monter un compte, pour respecter « plus aucun compte ne monte après 0b ».
37. En tant que gardien du cliquet, je veux qu'en fin d'étape `ui_button`, `msgbox`, `layout`, `menu`, `planet_display`, `research_display` et `score_panel_display` aient quitté la liste d'exceptions, pour que le verrou y interdise toute nouvelle allocation.
38. En tant que gardien de l'invariant, je veux que les `*.sha256` restent identiques à chaque commit, pour prouver que rendre la mémoire ne change pas un pixel.
39. En tant que lecteur du plan, je veux une note « Découvert » qui dise les sites classés autrement que dans le §2, les tailles et nombres de places finalement retenus et pourquoi, et les comptes laissés, pour que les étapes 3 et 4 partent du réel.
40. En tant que développeur de l'étape 4, je veux des lignes « *Reçu de l'étape 2* » sous l'entrée 4 (`ok.qa`, `ok` et ses calques `ic`, `ie.jb`, `fv`, la struct `pj`), pour savoir ce que l'étape 2 m'a laissé et pourquoi.
41. En tant que développeur de l'étape 3, je veux réutiliser la file de sortie et le membre d'archétype qui rend les morceaux d'une instance, pour que les particules rendent leur place par le même chemin.
42. En tant que mainteneur des commentaires, je veux que les `/* TROU-7 */` des fuites refermées soient retirés et que les contrats « borrowed, never owned » des panneaux disent désormais qui possède quoi, pour que les commentaires restent vrais.

## Implementation Decisions

### Ce qu'est une sorte d'interface

Une **sorte** est un genre d'objet qui prend une place à lui. Les sortes de l'étape :

| Sorte | Objet racine | Créé par | Quitte l'écran (point de libération) |
|---|---|---|---|
| `js` | bouton | `pz` (menus, panneaux de planète et de recherche, titre, pause), `ix` dans `eu`, « again? »/retour de `pj:ph` | `ui:cj` → `he` ; ou abandon par `jl()` (boutons hors `le` : `ix`, titre `ff`, `pj:ph`) |
| `fq` | fond modal | `ix[3].my`, `pj:ph` | `p_` levé par son `my` → `dg` → `he` ; ou `jl()` |
| `mh` | boîte de message | `pj:hk`, `pj:ph` | `p_` (fin de course ou remplacée : `kc.p_`) → `dg` → `he` ; ou `jl()` |
| `iv` | toast | `hy:lu`, `jo:hk` | `p_` après 50 ticks → `dg` → `he` ; ou `jl()` |
| `rangée` | `lq` réaffecté dans un objet qui lui survit : `jo.lq` des planètes, `js.lq` refait par `ix[2].hb` | `jo:hb` (par `jo.ex`), `ix[2].hb` | remplacement par le `hb` suivant ; sortie du propriétaire (`he` d'une planète, `jl()`) |
| `le` | liste `ok.le` | `ui:cj`, `pz` (et sa branche `ff`) | remplacement par le `cj` ou le `pz` suivant |

Les panneaux `lq`/`jb` fixés à la construction d'un objet ne sont pas des sortes : ils vivent dans la place de leur propriétaire (le `lq`, le `h_` et le `jb` d'un bouton ; le `lq` d'un article `ld` dans une boîte `mh` ; les lignes d'une bulle dans la bulle).

**Règle de rangement.** Un morceau fixé pour toute la vie de son propriétaire vit dans la place du propriétaire. Un morceau **réaffecté** pendant la vie du propriétaire (TROU-7 : « une réaffectation de champ abandonne l'ancienne valeur ») prend sa propre place, rendue à la réaffectation suivante. Sinon, la place du propriétaire se remplirait de versions mortes.

### Place-arène

- **Primitive.** Le module de zone gagne une **arène** : un bloc d'une taille fixe pris par une poignée `pico8_zone_choisie_t`, dans lequel on initialise une zone à pointeur montant privée et une poignée `pico8_zone_choisie_bump` sur elle. Les deux sont rangées dans l'en-tête du bloc.
  - Prendre une arène rend la poignée privée. Les panneaux gardent un pointeur vers leur zone : ce pointeur vise l'intérieur du bloc et vit autant que lui.
  - Rendre l'arène rend le bloc à la poignée qui l'a donné. Rien d'autre n'est à rendre.
  - Le nom de l'arène compose le message de zone pleine : « zone place js pleine, limite N octets ». Le nom de la zone à places de 2b compose l'autre : « zone interface js pleine, limite N places ». Les deux ne se confondent pas.
  - **Piège à commenter :** une arène ne se remet jamais à zéro. Sous ASan, `PICO8_BUMP_ZONE_HALVES` vaut 2 et `pico8_bump_zone_init` empoisonne deux moitiés : initialiser l'arène comme une zone frame empoisonnerait au-delà du bloc. L'arène n'a qu'une moitié, quel que soit le build.
- **Pas de rendu partiel.** Un morceau rangé dans l'arène de son propriétaire n'est jamais rendu seul : `pico8_zone_release` y ne fait rien (zone montante). Agrandir y copie (0b) : un `lq_add` laisse un bloc mort dans l'arène. Les tailles de place en tiennent compte.

### File de sortie et membre d'archétype

- **Membre d'archétype.** `pico8_struct_class_t` gagne un membre (nom laissé à l'implémenteur) qui rend ce que l'instance tient en zone interface. NULL : la sorte n'a rien à rendre.
  - Pour `js`, `fq`, `mh` et `iv`, il rend la place de l'instance.
  - Pour chaque classe de la famille `jo` (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`), il rend la place de `jo.lq` et remet `jo.lq` à nil. La planète, objet de partie (étape 4), n'est pas rendue.
  - L'étape 3 pourra y ajouter les particules ; l'étape 2 ne le fait pas.
- **File de sortie.** C'est un petit module du cœur, sans libc ni transitoire : un tableau statique de capacité fixe (au doigt mouillé, 256 entrées). Le débordement sort par `pico8_fatal` « file de sortie pleine, limite N ».
  - **Entrée par `he`.** `pico8_he` met l'entité dans la file si elle était dans `hv` (`pico8_ents_del` rend 1) et si son archétype a le membre. Cela couvre `dg` (`p_`) et `ui:cj`.
  - **Entrée par `jl()`.** Avant que `pico8_entity_init` ne vide `hv`, chaque entité encore inscrite dont l'archétype a le membre entre dans la file.
  - **Vidage** en tête de `pico8__update60()`, juste après `pico8_zone_frame_reset()` et avant `pico8_dg()` (l'ordre actuel de `pico8__update60()` : remise à zéro, `pico8_dg()`, `pico8_ui_lj()`). Le vidage appelle le membre de chaque entrée.
  - **Vidage aussi à `pico8_cart_load()`**, pour qu'un TU qui recharge le cart sans boucler `update60` ne remplisse pas la file.
  - **Références remises à nil au vidage**, quand elles désignent l'entité rendue : `o_` et `gj` de l'`ok` courant, `kc`, `ju.ed`. Écrire `m_` ou `p_` sur un objet mort n'a aucun effet visible dans le cart. Comparer `kc` à un `ed` mort y est toujours faux : remettre à nil garde ces deux vérités, alors qu'une adresse réutilisée les fausserait. ASan ne voit pas ce cas, qui est une comparaison et non une lecture (§4).
- **Pourquoi pas sur-le-champ.** Le code relit l'entité après `he` dans la même frame :
  - `dg` écrit `qc->t` juste après `pico8_he(qc)` ;
  - `ui:cj` parcourt `le` ;
  - `ui:jz` (le `bh("r")` qui suit un clic) parcourt `ic`, qui contient encore le bouton retiré ;
  - `ui:lj` pose `gj = o_` ;
  - un `jl()` lancé par « restart » laisse l'ancien `ok` faire son `cj` sur des boutons déjà abandonnés.

  `pico8_ui_set_ok` le dit déjà pour l'`ok` sortant.
- **Champs réaffectés : rendus sur-le-champ.** L'ancien `jo.lq` (`jo:hb`) et l'ancien `js.lq` d'`ix[2]` (`hb`) ne sont relus par personne après leur remplacement. `jo:kj` et `js:kj` ne dessinent qu'au `draw`, avec la nouvelle valeur. Les rendre sur-le-champ évite d'avoir ~180 places de rangée vivantes à chaque `hy:lj`. Cette preuve est une lecture : elle se vérifie en 2a.
- **Listes `le` : par la file.** `ui:lj` compare `self.le` au `le` du début de frame. Une liste rendue puis reprise à la même adresse dans la même frame fausserait ce test sans qu'ASan le voie. La liste remplacée passe donc par la file : une entrée de file peut viser une entité ou une liste. La liste statique de départ (`pico8_ui_le_shared()`) n'y entre jamais.

### Objets qui possèdent ce qu'ils montrent

Aucun de ces changements ne change un pixel. Chacun se vérifie par lecture : personne n'écrit en place dans la valeur recopiée.

- **Panneaux.** `pico8_lq_new`, `pico8_lq_copy` et `pico8_jb_new` prennent aussi leur **struct** dans la zone passée, au lieu de `malloc`.
  - Ils recopient dans cette zone le texte de chaque case `STRING`. Les cases et `fs` visent la copie. `pico8_lq_add` fait de même.
  - Une case `ROW` (tuple, `{qe=…}`, sous-rangée) reste empruntée. Elle n'est lue qu'à la mise en page, et par une copie faite dans la même frame (`hp:lq` → `js:pq`, `lq` d'un article `ld` → `mh`). Le contrat de `pico8_layout.h` le dit.
- **`jo.ex`** prend la zone de son résultat en premier argument, comme les fonctions de 0b. `jo:hb` passe la poignée d'une place de rangée ; `hp:lq` passe la zone frame. Ses tuples et `jq_str` passent en zone frame.
- **Bouton `js`.**
  - Sa place contient la struct, la boîte `kq` (`pico8_qz` sur la poignée de la place), le `lq`, le `h_` et la bulle `jb`.
  - Le bouton tient deux boîtes `ra` et `qf`. À la construction, il recopie la **valeur** de `keys->ra` et de `keys->qf` : la boîte partagée de `pz` et ses boîtes `qf`, les boîtes de `pj:ph` et `g_ix_ra` ne sont écrites par personne après la construction.
  - `js:ph` écrit la position glissée dans sa boîte `ra`.
  - `pz` et `pj:ph` ne font plus de `malloc` de boîte : des `pico8_vec_t` locales suffisent.
- **Toast `iv`.**
  - Il tient une boîte `ra` et une copie de son texte, d'une capacité fixe (au doigt mouillé, 16 caractères, avec un `assert` porteur de message).
  - **Piège à commenter :** `iv.ra` reste **emprunté** jusqu'au premier `iv:ph`. `jo:hk` lui passe le `ra` de la planète, que `na` déplace en place pendant le `dg` suivant, avant le `ph` du toast. Le cart lit ce `ra` déplacé. Une copie à la construction décalerait le toast d'une fraction de pixel, et pourrait changer un arrondi.
  - Dans `iv:pq`, le `ra` calculé quand la clé est absente va dans la boîte tenue.
  - Le `ne` de `hy:lu` et de `jo:hk` passe en zone frame.
- **Boîte `mh`.**
  - Sa place contient la struct et une copie de chaque article : la rangée, ses textes, ses sous-rangées. Un article `ld` y voit son `lq` recopié par `pico8_lq_copy` sur la poignée de la place, et son vecteur recopié par valeur.
  - `pj:hk` (`year_label`) et `pj:ph` (les textes de `pico8_pj_ph_s_str`, `label`, le `lq` et l'article `wrapped` de `pj:ph_map`) fabriquent en zone frame.
  - **Piège à commenter :** l'état statique de `pj` (`second_items`, `st->items`) garde un pointeur vers ces textes de frame. Il n'est relu que dans la fonction qui vient de l'écrire. Cela se vérifie par lecture en 2a ; sinon, l'état de `pj` en tient une copie.
- **Textes de `pj:hb`.** L'état `ou` de `pj` tient une copie de ses quatre textes (crédits `$`, revenu `ne`, `+po`, bonheur `%`) dans des tampons de capacité fixe. Leurs `pico8_ne`/`pico8_concat2` passent en zone frame. La struct `pj` reste à l'étape 4.

### Module de zone interface et poignées des sortes

- **Module.** C'est un module du cœur, hors de la liste d'exceptions : il ne référence ni la libc ni `pico8_zone_transitoire`. Il expose, pour chaque sorte :
  - une poignée `const pico8_zone_choisie_t *` qui donne et reprend **une place entière** ;
  - le nombre de places vivantes (accesseur de remplissage, §3).

  La place est prise et rendue par la primitive d'arène.
- **2a : adossé à la transitoire.** Le module compte les places prises et rendues, et délègue à une poignée d'adossement.
  - Cette poignée lui est donnée par un fichier qui référence déjà la transitoire : `pico8_ui_button.c`, une seule référence pour toutes les sortes.
  - L'adossement entre dans le commit qui retire déjà des références de ce fichier, pour qu'aucun compte ne monte.
  - Une place est alors un bloc `malloc` de la taille de la sorte. Les tailles de place sont donc éprouvées dès 2a.
- **2b : zones à places.** Le module possède un `PICO8_SLOT_ZONE_STORAGE` par sorte. La poignée devient une poignée sur zone à places :
  - `pico8_zone_choisie_t` gagne une construction sur `pico8_slot_zone_t` ;
  - `alloc(n)` exige `n` ≤ taille de place, par un `assert` porteur de message ;
  - `grow` est refusé ;
  - `release` rend la place.

  L'adossement à la transitoire disparaît. Aucun point de libération ne bouge.
- **Remplissage (§3).** `pico8_zone.h` gagne l'accesseur du nombre de places prises d'une zone à places. Celui d'une zone montante (octets pris) n'est pas ajouté : aucune zone montante non remise à chaque frame n'existe avant l'étape 4.
  - Le module de zone interface expose le compte par sorte, en 2a comme en 2b : le même test sert aux deux.
  - Le rejeu (`main.c`, hôte) peut écrire ce compte en fin de session, sous une option ou une variable d'environnement dont la forme est laissée à l'implémenteur. La capture, le dump et le code de sortie ne changent pas.

### Tailles et nombres de places (au doigt mouillé)

Ces valeurs ne sont pas mesurées (§3). Une place est la struct de la sorte plus ce qu'elle possède. Les tailles viennent de `sizeof` relevés à `HEAD` :

- `lq` 200 octets, `jb` 152 ;
- `js` 288, `mh` 264, `iv` 176, `fq` 152 ;
- une case 16, un élément `fs` 40 ;
- un `pico8_jb_item_t` 48, un `pico8_jb_row_t` 40.

| Sorte | Place | Nombre | Total | Raisonnement |
|---|---|---|---|---|
| `js` | 5 Ko | 32 | 160 Ko | struct + `kq` + `lq` et `h_` de 2–3 cases + une bulle de ≤ 6 lignes de ≤ 7 éléments (~3,6 Ko) ; vivants : 4 `ix` + ≤ 10 dans `le` + ≤ 10 en file + 2 de `pj:ph` (ou 4 du titre) |
| `rangée` | 768 o | 128 | 96 Ko | un `jo.ex` de ~6–9 cases (~550 o) + textes ; ~90 planètes (`bw(90)`) et quelques-unes en plus, + `ix[2]` |
| `mh` | 12 Ko | 4 | 48 Ko | `pj:ph` : 12 articles, chacun avec un `lq` de ≤ 5 cases ; vivantes : la courante, la remplacée, une en file |
| `iv` | 256 o | 48 | 12 Ko | struct + boîte + texte de 16 ; un saut d'année fait un toast par colonie visible |
| `fq` | 256 o | 4 | 1 Ko | au plus un fond ouvert, plus la file |
| `le` | 512 o | 4 | 2 Ko | `ents_t` + ≤ 16 pointeurs, avec les copies d'agrandissement |
| **Total** | | | **319 Ko** | |

- **Le total dépasse `PICO8_ZONE_INTERFACE_BYTES` (256 Ko).** 2b le relève à **320 Ko**. L'étape 1 n'a relevé aucune limite (`pico8_zone_sizes.h` à `HEAD` : frame 256, interface 256, particules 128, partie 768 Ko). La somme passe donc de 1408 Ko à **1472 Ko sur 1536**. **Ce relèvement est permis** : le §3 n'arrête une étape que si la somme dépasserait 1,5 Mo. Ici elle reste 64 Ko en dessous, sans rogner aucune autre zone. Le commit qui relève la limite l'écrit dans son message avec la somme, et la note « Découvert » la reprend.
- Une vérification à la compilation, comme celle de `pico8_zone_sizes.h`, exige que la somme des zones à places d'interface (tailles réelles, hors ASan) tienne dans `PICO8_ZONE_INTERFACE_BYTES`.
- **Lecture d'un `pico8_fatal`.**
  - « zone interface X pleine, limite N places » se lit d'abord comme une place jamais rendue. On cherche le point de sortie manquant. On ne relève N qu'une fois ce cas écarté, et la note « Découvert » dit lequel des deux c'était.
  - « zone place X pleine, limite N octets » dit qu'une place est trop petite. On la relève.
  - Si un relèvement ferait dépasser 1,5 Mo, l'étape s'arrête et le signale : zone, limite voulue, somme actuelle (§3).

### Inventaire des sites — convertis à l'étape 2

Il est relevé à `HEAD` 522364f (étape 1 finie) par `objdump -r` sur les objets d'`alloc-guard` à `-O0 -g`, chaque relocation rattachée à sa fonction et à sa ligne par `addr2line`. `make alloc-guard` y donne « 61 objets, 32 allouent, 147 appels », égal à `tools/alloc_guard_exceptions.txt`. Les 56 relocations ci-dessous sont celles de ce relevé ; aucune n'a été convertie par l'étape 1.

« T » désigne une référence à `pico8_zone_transitoire`. « libc » désigne un appel `malloc`/`calloc`/`realloc`/`free`.

| Fichier | Fonction (ligne) | Objet | Devient | Baisse |
|---|---|---|---|---|
| `pico8_ui_button.c` | `pico8_js_struct_new` (73) | struct `js` | place `js` | 1 libc |
| | `pico8_js_lq` (160, 163, 165) | `lq` du bouton | arène de la place | 3 T |
| | `pico8_js_pq` (180, 184, 191) | `kq`, `h_`, `jb` | arène de la place | 3 T |
| | `pico8_js_ph` (207) | boîte `ra` par tick | boîte tenue | 1 libc |
| | `pico8_fq_struct_new` (301) | struct `fq` | place `fq` | 1 libc |
| | (adossement du module, 2a) | référence ajoutée en 2a, compensée par les 9 ci-dessus (9 → 1) | retirée en 2b | 1 T en 2b |
| `pico8_msgbox.c` | `pico8_mh_struct_new` (91) | struct `mh` | place `mh` | 1 libc |
| | `pico8_iv_pq` (159), `pico8_iv_ph` (176) | boîtes `ra` | boîte tenue | 2 libc |
| | `pico8_iv_struct_new` (221) | struct `iv` | place `iv` | 1 libc |
| | `pico8_pz_step` (280) | boîte `qf` par bouton | valeur recopiée par `js` | 1 libc |
| | `pico8_pz` (338) | boîte `ra` partagée | valeur recopiée par `js` | 1 libc |
| | `pico8_pz` (345, 346, 357, 364, 365) | listes `le` et `ff`, `free` de la branche sans `ok` | place `le`, rendue par la file | 5 (3 libc + 2 T) |
| `pico8_layout.c` | `pico8_lq_struct_alloc` (296), `pico8_jb_struct_alloc` (402) | struct du panneau | zone passée | 2 libc |
| `pico8_ui.c` | `pico8_ui_cj` (441, 442) | liste `le` | place `le`, rendue par la file | 2 (1 libc + 1 T) |
| `pico8_menu.c` | `pico8_cr_step` (98) | `gi_str` | zone frame | 1 T |
| | `pico8_cr` (`label` 175, `ep` 214) | textes de case | zone frame | 2 T |
| | `pico8_ix2_hb_dispatch` (261) | `lq` réaffecté | place `rangée` | 1 T |
| `pico8_planet_display.c` | `jo_kw_tuple` (31) | tuple de case | zone frame | 1 libc |
| | `jo_dollar` (107) | texte de case (l'étape 1 lui a déjà retiré le `free` de son argument, qu'elle reçoit `const`) | zone frame | 1 libc |
| | `pico8_jo_gh` (`mu_str` de « makes » 240, `combined` 250) | textes de case | zone frame | 2 (1 T + 1 libc) |
| `pico8_research_display.c` | `pico8_hp_jb_join` (23, 55) | jointure de `hp:jb` | zone frame | 2 libc |
| | `pico8_jo_ex_tuple` (116) | tuples | zone frame | 1 libc |
| | `pico8_jo_ex` (217, 257) | `jq_str` ; résultat | frame ; zone passée | 2 T |
| `pico8_score_panel_display.c` | `pico8_pj_hk` (50) | `year_label` | zone frame | 1 T |
| | `pico8_pj_ph_s_str` (117–130) | sept textes | zone frame | 7 T |
| | `pico8_pj_ph_map` (181, 206, 207) | `label`, `lq`, `wrapped` | zone frame | 3 (2 T + 1 libc) |
| | `pico8_pj_ph` (241, 255) | boîtes `spec*.ra` | `pico8_vec_t` locales | 2 libc |
| `pico8_score_panel.c` | `pico8_pj_hb` (161, 164, 179, 183) | `cf` et trois `pico8_concat2` | zone frame, copie dans `ou` | 4 T |
| `pico8_economy.c` | `pico8_hy_lu` (188) | `ne` du toast | zone frame | 1 T |
| `pico8_planet.c` | `pico8_jo_hk` (325) | `ne` du toast | zone frame | 1 T |

### Baisse attendue du cliquet, par fichier

| Fichier | À `HEAD` 522364f (mesuré) | Après 2a | Après 2b |
|---|---|---|---|
| `pico8_ui_button.c` | 9 | 1 | sort |
| `pico8_msgbox.c` | 11 | sort | — |
| `pico8_layout.c` | 2 | sort | — |
| `pico8_menu.c` | 4 | sort | — |
| `pico8_planet_display.c` | 4 | sort | — |
| `pico8_research_display.c` | 5 | sort | — |
| `pico8_score_panel_display.c` | 13 | sort | — |
| `pico8_score_panel.c` | 5 | 1 | 1 |
| `pico8_ui.c` | 8 | 6 | 6 |
| `pico8_economy.c` | 4 | 3 | 3 |
| `pico8_planet.c` | 8 | 7 | 7 |
| **Liste** | 32 fichiers, 147 appels (mesuré) | 26 fichiers, 92 | 25 fichiers, 91 |

Ces chiffres sont des **prévisions**. Le verrou compte des relocations, qui dépendent du compilateur. La règle de fin est celle du §5 :

- chaque site de l'inventaire est converti, vérifié sur la source : la fonction ne référence plus la transitoire ni la libc pour ces objets ;
- `alloc-guard` est à égalité stricte sur les comptes mesurés ;
- aucun compte n'a monté.

L'écart se note dans le message de commit et dans « Découvert ». Il n'est jamais compensé ailleurs.

### Sites écartés de l'étape 2 (et où ils vont)

Ils sont listés pour la note « Découvert » et pour les lignes « *Reçu de l'étape 2* » sous l'étape 4.

- **`ok.qa`** (`pico8_ui_lj`, 201 ; boîte remplacée à chaque tick). Le *Reçu de l'étape 1* la range sous l'étape 2. **Elle n'y est pas traitée.** Les hôtes de `bv` (`g_`, la famille `jo`, `mz`) et le point souris de `jc:hq` gardent le `qa` qu'on leur passe (`hq`, `ldown` peuvent garder leurs vecteurs : `pico8_struct.h`). Le rendre au tick suivant serait un usage après libération. C'est un objet relu par des objets de partie : zone la plus longue (§4). Il passe à l'étape 4, qui devra le faire tenir par ses hôtes pour ne pas croître par tick.
- **`ok` lui-même** (`pico8_ui_struct_new`, 86 ; `pico8_ui_shutdown`, 122), ses calques `ic` (96, 261) et la liste statique de départ (112). Ils vivent de `jl()` à `jl()` : étape 4.
- **`ie.jb`** (`pico8_eu`, `pico8_cart.c` 189). La bulle d'une icône de recherche est faite une fois par partie et ne quitte l'écran qu'à `jl()`. La zone partie la couvre exactement : étape 4.
- **`fv`** (`pico8_help.c` 116). L'écran d'aide vit de `_init` au `jl()` suivant, et descend de `g_` : étapes 3 et 4.
- **La struct `pj`** (`pico8_score_panel.c` 260). Singleton de partie : étape 4.
- **`hp.i`**, les besoins d'un projet (`pico8_research.c`). Ce sont des données chargées avec le cart : étape 4.

## Implementation Chunks

Chaque morceau est un commit (ou une courte suite) qui respecte l'invariant du §3 et remet `alloc-guard` à égalité stricte. Son message dit les baisses mesurées par fichier et leur écart à la prévision. **Aucun morceau ne fait monter un compte.** Un site dont la conversion fait planter `make sanitize-net` n'est pas commité ; il est corrigé (sortie manquante, référence oubliée) ou noté pour « Découvert ».

Les tables « Avant/Après » partent de la liste mesurée à `HEAD` 522364f (32 fichiers, 147 appels). Ce sont des prévisions : le commit écrit la liste mesurée, à égalité stricte, et note l'écart. Chaque morceau n'ajoute que des références à la zone frame ou au module de zone interface, que le verrou ne compte pas ; la seule référence nouvelle à la transitoire, l'adossement, entre dans un commit qui en retire huit du même fichier.

**2a — Points de libération, sur la transitoire**

1. **Socle.**
   - La primitive d'arène et ses TU.
   - Le module de zone interface, avec ses sortes, leurs tailles de place et leurs compteurs, encore sans adossement.
   - La file de sortie, le membre d'archétype, l'entrée par `he` et par `jl()`, et le vidage en tête de `pico8__update60()` et à `pico8_cart_load()`.
   - La remise à nil de `o_`, `gj`, `kc` et `ju.ed`.

   Aucune sorte n'a encore de membre : la file reste vide en jeu. Comptes inchangés.
2. **Panneaux qui possèdent leurs textes.**
   - `lq_new`/`lq_copy`/`jb_new` prennent leur struct dans la zone passée et recopient les textes de leurs cases.
   - Les textes et tuples de case passent en zone frame : `gi_str`, `label`, `ep` (`menu`) ; `jo_kw_tuple`, `jo_dollar`, `mu_str`, `combined` (`planet_display`) ; la jointure de `hp:jb`, les tuples et `jq_str` de `jo.ex` (`research_display`).
   - `jo.ex` garde encore son `pico8_lq_new` sur la transitoire, et `ix[2].hb` aussi : leur zone change au morceau 5. Donner sa zone à `jo.ex` ici ferait passer la transitoire par `jo:hb`, et monter `planet_display`.

   | Fichier | Avant | Après |
   |---|---|---|
   | `layout` | 2 | sort |
   | `menu` | 4 | 1 |
   | `planet_display` | 4 | sort |
   | `research_display` | 5 | 1 |

   Liste : 30 fichiers, 134 appels.
3. **Boutons `js` et fonds `fq`, et adossement du module.**
   - La place `js` (struct, `kq`, `lq`, `h_`, `jb`) et les boîtes `ra`/`qf` tenues par le bouton.
   - `pz` et `pj:ph` ne font plus de boîte.
   - La place `fq`.
   - Leur membre d'archétype.
   - L'adossement du module de zone interface à la transitoire, dans `pico8_ui_button.c`. Il entre dans ce commit, qui retire huit références de ce fichier : c'est le premier morceau qui prend une place, et aucun compte ne monte. Les morceaux 4 à 7, qui prennent aussi des places, viennent après.

   | Fichier | Avant | Après |
   |---|---|---|
   | `ui_button` | 9 | 1 (l'adossement) |
   | `msgbox` | 11 | 9 |
   | `score_panel_display` | 13 | 11 |

   Liste : 30 fichiers, 122 appels.
4. **Toasts `iv`.**
   - Le texte et la boîte `ra` sont tenus par le toast, avec l'emprunt jusqu'au premier `ph`.
   - Le toast a sa place et son membre.
   - Le `ne` de `hy:lu` et de `jo:hk` passe en zone frame.

   | Fichier | Avant | Après |
   |---|---|---|
   | `msgbox` | 9 | 6 |
   | `economy` | 4 | 3 |
   | `planet` | 8 | 7 |

   Liste : 30 fichiers, 117 appels.
5. **Rangées réaffectées.**
   - `jo.ex` prend la zone de son résultat en premier argument : `jo:hb` passe une place `rangée`, `hp:lq` la zone frame.
   - `jo.lq` et le `lq` d'`ix[2]` prennent une place `rangée`, rendue sur-le-champ au remplacement.
   - Le membre de la famille `jo` rend `jo.lq` à la sortie de la planète.

   | Fichier | Avant | Après |
   |---|---|---|
   | `menu` | 1 | sort |
   | `research_display` | 1 | sort |

   Liste : 28 fichiers, 115 appels.
6. **Listes `le`.** `ui:cj` et `pz` (et sa branche `ff`) prennent une place `le` et mettent la liste remplacée dans la file.

   | Fichier | Avant | Après |
   |---|---|---|
   | `msgbox` | 6 | 1 |
   | `ui` | 8 | 6 |

   Liste : 28 fichiers, 108 appels.
7. **Boîtes `mh` et textes de `pj`.**
   - La place `mh` recopie ses articles.
   - `pj:hk`, `pj:ph_map`, `pj:ph_s_str` et `pj:hb` fabriquent en zone frame, et l'état `ou` tient ses copies.
   - Le membre de `mh`.

   | Fichier | Avant | Après |
   |---|---|---|
   | `msgbox` | 1 | sort |
   | `score_panel_display` | 11 | sort |
   | `score_panel` | 5 | 1 |

   Liste : 26 fichiers, 92 appels.
8. **Preuve des places vivantes.** Les TU de rejeu `pause_restart` et `message_box_hold` (voir Testing Decisions), et l'écriture du remplissage par le rejeu.
   - Si un compte ne revient pas à son niveau, c'est une sortie manquante : on la trouve avant de passer à 2b.

**2b — Conversion**

9. **Zones à places d'interface.**
   - La construction de poignée sur zone à places, et l'accesseur de places prises de `pico8_zone.h`.
   - Le stockage par sorte dans le module de zone interface, et la vérification de somme.
   - `PICO8_ZONE_INTERFACE_BYTES` relevé si la somme l'exige (320 Ko prévus : la somme passe à 1472 Ko sur 1536, permis).
   - Le retrait de l'adossement à la transitoire : `pico8_ui_button.c` sort de la liste. Liste : 25 fichiers, 91 appels.
   - Aucun point de libération n'est ajouté ni déplacé.
10. **Clôture de l'étape.**
    - La note « Découvert » sous l'entrée 2 du plan : sites classés autrement que dans le §2, tailles et nombres de places retenus (avec, pour chaque `pico8_fatal` rencontré, « place jamais rendue » ou « limite trop basse »), comptes laissés et écarts à la prévision.
    - La correction du §2 si un constat le contredit.
    - Les lignes « *Reçu de l'étape 2* » sous l'entrée 4 (`ok.qa`, `ok` et `ic`, `ie.jb`, `fv`, la struct `pj`) et sous l'entrée 3 (la file de sortie et le membre d'archétype sont prêts pour les particules).

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - un dessin de panneau, de bouton, de boîte ou de toast ;
  - un compte de places vivantes rendu par l'accesseur ;
  - un code de sortie, un message de `pico8_fatal`, un rapport ASan ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne regarde ni une adresse ni l'intérieur d'une arène.
- **TU ASan de l'arène et de la zone à places**, sur le modèle de `tmp/test_pico8_zone_choisie` (règle `asan_objs`, `UNIT_RUNS_SPECIAL`, `ASAN_OPTIONS=detect_leaks=0`). Chaque plantage attendu est vérifié dans un processus enfant.
  - Ce qu'on alloue dans une arène se lit jusqu'à ce qu'elle soit rendue ; lu après, cela plante.
  - Une arène qui déborde sort en 3 avec « zone place <nom> pleine ».
  - Sous ASan, l'initialisation n'empoisonne rien au-delà du bloc : une place voisine reste lisible.
  - Une poignée sur zone à places refuse une taille plus grande que la place (`assert`). Une place rendue puis lue plante. Une zone pleine sort en 3 avec « zone interface <nom> pleine ». Le compte de places prises suit les prises et les rendus.
- **TU de la file de sortie.**
  - Une entité retirée par `he` reste lisible jusqu'au vidage. Elle est rendue au vidage (plantage dans l'enfant si on la lit après).
  - `he` d'une entité absente de `hv` n'enfile rien.
  - `jl()` enfile chaque entité encore inscrite qui a le membre.
  - Après vidage, `ok.gj`, `ok.o_`, `kc` et `ju.ed` qui désignaient l'entité valent nil.
  - Une file pleine sort en 3 avec « file de sortie pleine ».
- **TU par sorte**, dans les TU existants passés en règle ASan si besoin : `test_pico8_js`, `test_pico8_ui`, `test_pico8_mh`, `test_pico8_iv`, `test_pico8_lq`, `test_pico8_jb_ol`.
  - On construit l'objet, on le fait sortir (`cj`, `p_`, remplacement), on vide la file : le compte de la sorte revient à son niveau d'avant.
  - **Textes recopiés.** On construit un panneau, une boîte ou un toast avec des textes faits en zone frame, on remet la zone frame à zéro et on alloue de nouveau, puis on dessine sans rapport ASan.
  - **Emprunt du `ra` d'un toast.** Un toast fait avec le `ra` d'une planète suit le déplacement en place de ce `ra` jusqu'à son premier `ph`, comme dans le cart.
  - **Boutons qui glissent.** Un menu ouvert et laissé glisser N ticks garde le même compte de places `js`, en 2a comme en 2b.
- **Places vivantes, sur rejeu** : un TU qui pilote le cart frame par frame depuis un `events.json` de `tests/e2e/`, comme la boucle de `main.c`, et lit le compte de chaque sorte **après un vidage de la file**.
  - `pause_restart` : le compte relevé après `_init`, avant la frame 0 (écran titre), est égal à celui relevé après la frame 11 (écran titre après « restart » en f7, avant le clic de f12). Cela montre que `jl()` rend tout ce qu'il abandonne.
  - `message_box_hold` : comptes égaux aux frames 45 et 90, dans la phase sans action f42–90 où la boîte de l'année est retenue par son `lh`. Cela montre qu'aucune sorte n'alloue par tick (§3, « Croissance par frame »). Si la fenêtre n'est pas sans mouvement de places (la lecture de `jo:hk` dit qu'aucune colonie n'existe, donc aucun toast : à confirmer par le TU), on en choisit une autre dans le même e2e, et « Découvert » le dit.
  - `empty_planet_fade` : aucun compte ne monte entre f25 et f94, où « p40 » s'efface. La rangée de « p40 » est rendue : le compte `rangée` peut baisser d'une place.

  Le même TU sert en 2a (compteurs adossés à la transitoire) et en 2b (zones à places).
- **Tests existants.** Les TU de panneaux, de menu, de `pj`, de la sonde, des échanges, `test_pico8_replay_from_title` et `test_pico8_replay_usage` restent verts sans changement d'attente. La seule adaptation permise : appeler `jo.ex` avec sa zone.
- **Verrou.** `make alloc-guard` (dans `make test`) à égalité stricte après chaque morceau. Pas de nouveau cas dans `tests/test_alloc_guard.c`.
- **Invariant.** `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. En 2a, la quarantaine de la libc attrape un usage après libération. En 2b, l'empoisonnement des places rendues l'attrape, dans la limite d'un tour complet du curseur (§4).
- **Exemples dans le dépôt** :
  - `tests/test_pico8_zone.c` et `tests/test_pico8_zone_choisie.c` (lecture périmée dans un enfant, zone pleine en 3) ;
  - `tests/test_pico8_replay_usage.c` (processus enfant, statut, `stderr`) ;
  - `src/main.c` (boucle de rejeu : entrées d'une frame, `pico8__update60()`, `pico8__draw()`) ;
  - `test_pico8_bg`, `test_pico8_ga`, `test_pico8_lq` (TU qui bouclent `update60`/`draw`).

## Out of Scope

- Particules `g_:mm`, `qy`, éclats `ga`, et leur point de libération : étape 3, qui réutilisera la file de sortie.
- Planètes, relais, slipways, routes `trade`, recherche, trous de ver, `ok`, `qa`, `ie.jb`, `fv`, `pj` et leurs tableaux croissants : étape 4.
- La zone frame elle-même, son module et sa remise à zéro : étape 1.
- L'accesseur de remplissage d'une zone montante : étape 4, première à en instancier une qui n'est pas remise à chaque frame.
- Retirer l'entrée TROU-7 de `docs/trous-de-portage.md` et des sources qui la citent encore pour d'autres sortes : étape 5. L'étape 2 ne retire que les `/* TROU-7 */` des fuites qu'elle referme.
- Les autres appels libc du cœur (§7) et les économies de taille PSX.
- Changer le comportement du jeu, ou compacter la représentation des panneaux (`pico8_jb_item_t` à 48 octets).
- Lancer l'application raylib.

## Vérifié contre l'étape 1

Relu contre `HEAD` 522364f : la note « Découvert » de l'étape 1 et les lignes « *Reçu de l'étape 1* » sous les entrées 2 et 4 de `docs/plan-4.md`, la ligne « Frame » du §2, `tools/alloc_guard_exceptions.txt` (`make alloc-guard` : 61 objets, 32 allouent, 147 appels), `src/pico8_zone_frame.h`, `pico8_zone_sizes.h`, `pico8_zone.h`, `pico8__update60()`, et `git log -8 -p` (4b7ec43, c12cd07, 80e175f, fd2f883, aeb8dfd).

- **H-1-1 (poignée et remise à zéro) — tenue.** `const pico8_zone_choisie_t *pico8_zone_frame(void)` et `void pico8_zone_frame_reset(void)`, initialisation au premier appel de l'une ou l'autre. `pico8__update60()` appelle `pico8_zone_frame_reset()` puis `pico8_dg()`. Le vidage de la file de sortie se place entre les deux.
- **H-1-2 (comptes laissés) — tenue.** 32 fichiers, 147 appels. Chaque fichier de l'inventaire est au compte prévu : `menu` 4, `planet_display` 4, `research_display` 5, `score_panel` 5, `score_panel_display` 13, `ui` 8, `ui_button` 9, `msgbox` 11, `layout` 2, `economy` 4, `planet` 8, `cart` 1. Les tables de cette PRD partent de ces chiffres mesurés.
- **H-1-3 (sites laissés par l'étape 1) — tenue.** Les 56 relocations de l'inventaire sont toutes encore présentes : `jo_dollar` garde son `malloc` (il ne libère plus son argument, devenu `const`), `jo_kw_tuple` et les tuples de `jo.ex` leur `calloc`, la jointure de `hp:jb` son `malloc`/`realloc`, la boîte `pico8_qz` de `js` sa référence à la transitoire. Seuls les numéros de ligne ont bougé ; l'inventaire est récrit dessus.
- **H-1-4 (déjà en zone frame) — tenue.** `jo_gh_row_items`, `pico8_menu_invented_lines`, le bloc de `pico8_hp_jb` et `concat_percent` sont en zone frame, chacun avec son test de survie (aeb8dfd) ; `bs`/`rows` et `l` aussi (fd2f883).
- **H-1-5 (limites) — tenue.** Aucune limite relevée. Somme 1408 Ko ; 128 Ko de marge. Le relèvement de l'interface à 320 Ko la porte à 1472 Ko, permis.
- **H-1-6 (« Reçu de l'étape 1 » sous l'étape 2) — tenue.** La ligne dit exactement : boîtes `vec` remplacées à chaque tick dans un champ d'objet d'interface (`ra` de `iv`, de `js`, `qa` de `ui`, `qf`/`ra` de `pz`, `spec*.ra` de `pj:ph`), à traiter au point de libération (2a). Traitement : voir ci-dessous.
- **H-1-7 (pas de toast dans la phase sans action) — non touchée par l'étape 1.** C'est une lecture de `jo:hk`, pas une hypothèse sur l'étape 1 ; elle reste à confirmer par le TU de `message_box_hold` (Testing Decisions).
- **H-1-8 (pas d'accesseur de remplissage) — tenue.** `pico8_zone.h` n'en expose aucun : l'étape 2 ajoute celui des zones à places.
- **H-1-9 (§2 corrigé) — tenue.** Seule la ligne « Frame » a changé (« pas les boîtes `vec` rangées dans un champ d'objet, qui vont avec leur objet »). La ligne « Interface » est intacte : le tableau des sortes n'a pas à bouger.

Aucune hypothèse n'a échoué. Aucun site n'a été renvoyé à la transitoire par un crash ASan à l'étape 1.

### Traitement du « Reçu de l'étape 1 » sous l'étape 2

Chaque boîte de la ligne est traitée en 2a, sauf une :

| Boîte | Traitement | Morceau |
|---|---|---|
| `ra` de `iv` (`iv:pq`, `iv:ph`) | boîte tenue par le toast ; `ra` emprunté jusqu'au premier `ph` | 3 |
| `ra` de `js` (`js:ph`) | boîte tenue par le bouton | 4 |
| `qf`/`ra` de `pz` | valeurs recopiées par le bouton ; plus de `malloc` | 4 |
| `spec*.ra` de `pj:ph` | `pico8_vec_t` locales, recopiées par le bouton | 4 |
| `qa` de `ui` | **non traitée à l'étape 2** : les hôtes de `bv` (`g_`, la famille `jo`, `mz`) et le point souris de `jc:hq` gardent le `qa` qu'on leur passe ; la rendre au tick suivant serait un usage après libération. Objet relu par des objets de partie : zone la plus longue (§4), étape 4, par une ligne « *Reçu de l'étape 2* » | 10 |

## Further Notes

- **Pas d'humain disponible, pas d'outil `Agent` dans cette session.** Les points que `to-prd` fait valider par un humain ont été tranchés en appliquant moi-même le skill `guideline` :
  - **Hypothèses explicites** : la liste « Vérifié contre l'étape 1 », et les pièges marqués « vérifié par lecture en 2a ».
  - **Simplicité** : une seule forme de mémoire pour toutes les sortes, la place-arène. Rendre une place est un seul appel, et 2b se réduit à changer la poignée d'adossement.
  - **Changements chirurgicaux** : pas de nouvelle représentation de panneau, pas de compaction des rangées, aucun site de partie touché hors `jo.lq` et les textes de `pj`.
  - **Critères vérifiables** : comptes du cliquet par morceau, `*.sha256` inchangés, `sanitize-net`, et trois relevés de places vivantes sur rejeu.
- **Pourquoi une file plutôt qu'un rendu dans `he`.** `pico8_ui_set_ok` documente déjà que `jl()` refait `ok` au milieu d'un clic, et que l'ancien `ok` continue sa frame. Un rendu dans `he` ferait du `cj` de l'ancien `ok`, du `qc->t += 1` de `dg` et du `bh("r")` qui suit un clic des usages après libération. Les références globales remises à nil au vidage (`o_`, `gj`, `kc`, `ju.ed`) couvrent ce qu'une frame de retard ne couvre pas. Elles couvrent aussi ce qu'ASan ne voit pas : une comparaison de pointeurs vers une place réutilisée.
- **Pourquoi les textes de case passent en zone frame.** L'étape 1 les a écartés parce qu'un panneau les désignait par pointeur. Une fois que le panneau, la boîte ou le toast en tient une copie, ils ne vivent plus que le temps de construire l'objet. C'est l'étape 2 qui rend cette copie possible.
- **Risque de budget.** La somme prévue (≈ 319 Ko) dépasse la limite de départ de l'interface. Le relèvement à 320 Ko porte la somme à 1472 Ko sur 1536 : c'est permis, et la marge restante (64 Ko) borne tout relèvement ultérieur, ici ou aux étapes 3 et 4. Le poste le plus lourd est le bouton (une bulle de six lignes) ; le plus nombreux, la rangée de planète. Une compaction des panneaux, si elle devient nécessaire, est une décision de l'humain, pas de cette étape.
- À la fin de l'étape, la note « Découvert » sous l'entrée 2 du plan porte au minimum :
  - les sites classés autrement que dans le §2, et pourquoi (dont `ok.qa`, `ie.jb`, `fv`) ;
  - les tailles et nombres de places retenus par sorte ;
  - chaque `pico8_fatal` rencontré, lu comme « place jamais rendue » ou « limite trop basse » ;
  - `PICO8_ZONE_INTERFACE_BYTES` et la somme des zones ;
  - les comptes du cliquet laissés, fichier par fichier, et leur total ;
  - les écarts entre les prévisions de cette PRD et la mesure.
