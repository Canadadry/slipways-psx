---
title: "Plan 4, étape 3 — Zone particules : particules, listes qy et éclats ga rendent leur place"
description: "Les particules faites par `g_:mm` et `ga:mm`, les listes `qy` faites par `f_` et les éclats `ga` sont pris à la libc ou à la transitoire et jamais rendus (TROU-7) : l'étape 3 leur donne d'abord un point de libération, sur la transitoire (3a), puis les range dans des zones à places de taille fixe (3b)."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. Le texte de l'étape, recopié du plan (relu à `HEAD` ecbf87c) :

> 3. **Zone particules.**
>    - *Reçu de l'étape 2 :* la file de sortie (entités ou arènes), le membre d'archétype `rendre`, les arènes, la poignée sur zone à places et son accesseur de places prises sont prêts ; l'en-tête d'arène coûte 96 o par place.
>
>    Ces deux étapes créent un point de libération qui n'existe pas aujourd'hui (TROU-7 : panneaux, messages et particules fuient). Elles ne sont finies que si chaque sorte convertie rend sa place, ce qui se montre par le remplissage du §3 : après une session scriptée, le nombre de places vivantes revient à son niveau de départ, et il ne monte pas pendant une phase sans action. Dans une zone à places, un `pico8_fatal` est d'abord lu comme une place jamais rendue ; la limite n'est relevée qu'une fois ce cas écarté, et la note « Découvert » dit lequel des deux c'était.
>
>    Chacune se fait en deux temps, chacun sa PRD ou son lot de commits :
>    - **a. Points de libération, sur la transitoire.** Chaque sorte de l'étape (et chaque champ réaffecté qui lui revient, TROU-7) gagne sa libération, la transitoire encore derrière : sous ASan, la quarantaine de la libc attrape tout usage après libération. Le critère des places vivantes est montré ici, sur un compte de vivants par sorte.
>    - **b. Conversion.** Les mêmes sites passent à la zone à places, sans ajouter ni déplacer de point de libération : un `sanitize-net` rouge en b vient de la zone, pas d'une libération manquante.

Cette PRD ne traite que l'étape 3. Les règles qui la cadrent sont celles du plan :

- §2 : la zone particules contient « `g_:mm`, `qy`, éclats `ga` » ; elle a un « nombre maximum de particules vivantes ; place rendue à leur mort » (ligne inchangée par l'étape 2, qui n'a corrigé que la ligne Interface) ;
- §3 : limites au doigt mouillé ; somme des zones ≤ 1,5 Mo, et si un relèvement la dépasse, l'étape s'arrête et le signale (zone, limite voulue, somme actuelle) sans rogner une autre zone ; limite atteinte = `pico8_fatal` ; invariant de chaque commit (`make test-all`, `make sanitize-net`, `*.sha256` identiques) ; empoisonnement ASan d'une place rendue ; contrôle « Croissance par frame » : le rejeu écrit le remplissage **et le maximum atteint par sorte** (places vivantes ; octets pris pour une place-arène) ; le temps a relève ce maximum sur tous les e2e, le temps b ne fixe aucune taille de place ni aucun nombre en dessous, et « Découvert » donne, par sorte, maximum relevé et limite retenue ; le remplissage est égal à deux frames d'un e2e séparées par une phase sans action ; une hausse se corrige à l'étape qui l'a fait entrer, pas en relevant la limite ;
- §4 : un site douteux va dans la zone la plus longue qui le couvre ; ASan ne fait que rétrograder ; il ne voit ni un pointeur gardé à travers deux remises à zéro, ni une place rendue un tour complet plus tôt ; **avant de faire finir une durée de vie, la PRD inventorie mécaniquement les pointeurs du cœur qui survivent à la frame** (`static … *` de portée fichier, champs pointeurs des structures statiques et des singletons `ok`, `pj`, …) et dit pour chacun : remis à nil, repointé, ou ne peut pas viser la sorte rendue ; cet inventaire, pas la lecture des sites, fixe la liste des références oubliées (`oublier`) ;
- §5 : fin d'étape = chaque site de l'inventaire converti (vérifié sur la source), `alloc-guard` à égalité stricte, aucun compte n'a monté ; la baisse prévue n'est qu'une estimation.

L'étape 2 est finie (`HEAD` 3b28708 ; le plan a été amendé ensuite en ecbf87c, sans toucher `src/`). Ce que cette PRD en reprend est relevé dans « Vérifié contre l'étape 2 ».

**Ce qui se passe aujourd'hui.** Aucune particule n'est jamais rendue :

- `ez` ajoute à `self.qy` une particule `{p,v,a,l}` faite par `self:mm()` : `g_:mm` (`pico8_g__mm`, pour `g_`, `fv`, `hj`, `pm`, `mx`) ou `ga:mm` (`pico8_ga_mm`). Chacune est un `malloc` ;
- `ei` la retire de `qy` quand `l <= 1` (`pico8_ptrs_del`), et l'abandonne ;
- `f_` fait la liste `qy` d'un émetteur (`malloc` de la struct, tableau sur la transitoire). Elle n'est jamais rendue, ni elle ni les particules qu'elle tient encore quand son hôte meurt : sonde `g_` posée, écran d'aide `fv` quitté, éclat `ga` éteint, planète abandonnée par `jl()`. Depuis l'étape 2, `ga`, `hj`, `pm` et `mx` passent déjà par la file de sortie, mais leur `rendre` (`pico8_jo_rendre_dispatch`) ne rend que leur rangée d'interface ;
- l'éclat `ga` lui-même (struct, son `oe`, la boîte `ra` que `pm:pq` lui fait) n'est jamais rendu après son `p_`.

Le débit est fort : l'écran titre (`fv`, `fc=5`) fait 5 particules par frame ; un éclat en fait 140 d'un coup ; une sonde ou un protostar une par frame. Une partie longue croît donc sans borne en mémoire de particules, ce que la PSX ne peut pas tenir.

## Solution

L'étape 3 donne à chaque **sorte de particules** une place dans une zone à places de taille fixe, et un **point de libération** qui rend cette place à la mort de l'objet. Elle reprend les outils de l'étape 2 (file de sortie, membre `rendre`, arène, poignée sur zone à places et accesseur de places prises, TU de rejeu des places vivantes, écriture du remplissage par `PICO8_REMPLISSAGE`) au lieu d'en inventer d'autres. Elle se fait en deux lots de commits, dans l'ordre imposé par le plan.

**3a — Points de libération, sur la transitoire.**

- **Une particule meurt dans `ei`.** Quand `ei` la retire de `qy` (`l <= 1`), il rend aussi sa place, sur-le-champ : la liste est sa seule propriétaire.
- **Une liste `qy` meurt avec son hôte.** Quand un émetteur (`g_`, `fv`, `hj`, `pm`, `mx`, `ga`) sort par la file de sortie (retiré par `he`, ou abandonné par `jl()`), son `rendre` rend chaque particule encore dans `qy`, puis la liste.
- **Un éclat `ga` meurt quand son `p_` le fait sortir.** Sa place (struct, `oe`, boîte `ra`) est rendue au vidage de la file, après sa liste et sa rangée d'interface.
- **Avant toute remise**, l'inventaire mécanique du §4 des pointeurs qui survivent à la frame : aucun ne peut viser une particule, une liste ou un éclat sans être remis à nil, donc rien à ajouter à `oublier`.
- **Un compte de vivants et un maximum par sorte**, lus par un accesseur de remplissage et écrits par le rejeu ; le maximum sur les 26 e2e devient le plancher des limites de 3b. Le compte montre le critère du plan sur `pause_restart`, `tech_geoharvesting_burst`, `message_box_hold`, et un invariant plus fort sur tous les e2e (voir Testing Decisions).

**3b — Conversion.**

- Un module de zone particules possède une zone à places par sorte, de taille et de nombre fixés à la compilation, dont la somme est vérifiée contre `PICO8_ZONE_PARTICLES_BYTES`.
- Les poignées passent de la transitoire à ces zones, sans ajouter ni déplacer de point de libération.
- `pico8_particle.c` sort de la liste d'exceptions du verrou.

**Pas de changement visible.** Les 26 `*.sha256` restent identiques à chaque commit, et `make test-all` et `make sanitize-net` passent.

## User Stories

1. En tant que mainteneur du plan, je veux que chaque particule rende sa mémoire à sa mort, pour qu'une partie longue ne croisse plus en mémoire de particules.
2. En tant que futur porteur PSX, je veux que particules, listes `qy` et éclats `ga` vivent dans des zones à places de taille connue à la compilation, pour que leur plafond se lise dans un en-tête et compte dans les 1,5 Mo.
3. En tant que développeur, je veux que `ei` rende la place d'une particule au moment même où il la retire de `qy`, pour qu'un émetteur actif (5 particules par frame à l'écran titre) ne laisse aucune place morte.
4. En tant que développeur, je veux que la particule retirée ne soit plus lue après sa remise, pour que la quarantaine d'ASan (3a) ou l'empoisonnement de la place (3b) ne trouve rien.
5. En tant que développeur, je veux que `ga:kj` ne garde que le fait « la troisième place de `qy` était occupée », et non le pointeur de la particule, pour que `self.p_ = not pt` ne compare jamais un pointeur vers une place que `ei` vient de rendre.
6. En tant que développeur, je veux que `g_:mm` et `ga:mm` prennent leur particule à la poignée de la sorte `particule`, pour que toutes les particules du jeu soient comptées au même endroit.
7. En tant que développeur, je veux que `f_` prenne la liste `qy` dans une arène, avec une capacité réservée d'un coup, pour que la liste ne s'agrandisse jamais par copie et que son débordement sorte par un `pico8_fatal` qui la nomme.
8. En tant que développeur, je veux que la liste de l'écran d'aide `fv` prenne une place plus grande que celle des autres émetteurs, pour que ses ~580 particules vivantes (5 par frame pendant ~115 frames) ne fixent pas la taille de toutes les listes.
9. En tant que développeur, je veux qu'un `f_` appelé sur un hôte qui a déjà une liste rende l'ancienne, pour reproduire la réaffectation `self.qy={}` du cart sans abandonner de place (TROU-7, champ réaffecté).
10. En tant que développeur, je veux que la sortie d'une sonde `g_` (posée par `d_`, ou abandonnée par `jl()`) rende ses particules et sa liste, pour que chaque sonde lancée ne laisse rien en zone particules.
11. En tant que développeur, je veux que la sortie de l'écran d'aide `fv` (au `jl()` de `eu`) rende ses particules et sa liste, pour qu'une nouvelle partie démarre sans les ~580 particules du titre.
12. En tant que développeur, je veux que la sortie d'une planète émettrice (`hj`, `pm`, `mx`) rende ses particules et sa liste en plus de sa rangée d'interface, pour que « restart » et une nouvelle partie rendent toute la zone particules.
13. En tant que développeur, je veux que la sortie d'un éclat `ga` rende sa rangée, ses particules restantes, sa liste, puis sa place, pour qu'un éclat éteint ne laisse rien.
14. En tant que développeur, je veux qu'un éclat `ga` prenne une arène qui contient sa struct, son `oe` et la boîte `ra` que `pm:pq` lui donne, pour que le rendre soit un seul appel.
15. En tant que développeur, je veux que `pm:pq` passe à l'éclat une **valeur** de position, recopiée dans la place de l'éclat, au lieu d'allouer une boîte qu'il abandonne.
16. En tant que développeur, je veux qu'un éclat né par `jo:hu` (« harvest ») garde le `ra` et le `p` empruntés à la planète qu'il remplace, comme dans le cart, pour ne changer ni un pixel ni un objet de partie.
17. En tant que développeur, je veux que `jo:pq` puisse prendre la zone de son `oe` en paramètre, pour que l'éclat range son `oe` dans sa place sans que les autres planètes changent de zone avant l'étape 4.
18. En tant que développeur, je veux que la libération des listes et des éclats passe par la file de sortie et le membre `rendre` de l'étape 2, pour qu'il n'y ait qu'un seul chemin de sortie, et que l'éclat ne soit rendu qu'en tête de la frame suivante (`dg` écrit son `t` après `he`).
19. En tant que développeur, je veux que le `rendre` d'une classe émettrice de la famille `jo` appelle d'abord `pico8_jo_rendre_dispatch` (la rangée) puis rende `qy`, pour qu'aucune des deux étapes n'écrase l'autre.
20. En tant que développeur, je veux qu'un objet de partie émetteur (planète, sonde, aide) ne soit **pas** rendu par l'étape 3 — seulement ce qu'il tient en zone particules —, pour que l'étape 4 garde la main sur les objets de partie.
21. En tant que développeur, je veux un inventaire mécanique (§4) des pointeurs du cœur qui survivent à la frame, chacun classé « remis à nil », « repointé » ou « ne peut pas viser » une particule, une liste `qy` ou un éclat, refait par `grep` dans chaque morceau qui rend une sorte, pour ne pas refaire l'erreur de `dk` (référence absente de la PRD de l'étape 2, trouvée par un use-after-free ASan).
22. En tant que mainteneur, je veux un compte de places vivantes par sorte (`particule`, `qy`, `qy_fv`, `ga`), lisible par un accesseur, pour montrer en 3a comme en 3b qu'une session revient à son niveau de départ.
23. En tant que mainteneur, je veux un TU de rejeu qui vérifie, à chaque frame de chaque e2e de `tests/e2e/`, que le nombre de places `particule` vivantes est égal à la somme des `#qy` des émetteurs inscrits, pour prouver frame par frame qu'aucune particule n'est perdue.
24. En tant que mainteneur, je veux que le TU des places vivantes rejoue `pause_restart` et compare les comptes `qy`, `qy_fv` et `ga` à l'écran titre avant la partie et après « restart », et le compte `particule` à ce que l'aide a émis depuis son `_init`, pour prouver que `jl()` rend tout ce qu'il abandonne.
25. En tant que mainteneur, je veux un relevé sur `tech_geoharvesting_burst` qui montre que les comptes `ga` et `qy` reviennent, après l'extinction de l'éclat, au niveau d'avant le « harvest », pour prouver que l'éclat rend tout.
26. En tant que mainteneur, je veux un relevé sur `message_box_hold` qui compare tous les comptes aux frames 45 et 90 de sa phase sans action, pour satisfaire le contrôle « Croissance par frame » du §3.
27. En tant que mainteneur, je veux un TU qui laisse tourner l'écran titre sans entrée bien au-delà de la vie d'une particule de l'aide, et montre que le compte `particule` reste borné et que `qy_fv` reste à 1, pour prouver qu'un émetteur actif atteint un régime sans croissance.
28. En tant que mainteneur, je veux que le rejeu écrive aussi le remplissage des sortes de particules dans le fichier nommé par `PICO8_REMPLISSAGE`, avec, par sorte, le maximum atteint pendant la session (places vivantes, octets pris dans une place-arène, éléments d'une liste `qy`), pour lire un taux et un pic sans écrire de test.
29. En tant que mainteneur du budget, je veux qu'en 3a ce maximum soit relevé sur les 26 e2e et noté, et qu'en 3b aucune taille de place, capacité de liste ni nombre de places ne soit fixé en dessous, pour qu'une limite au doigt mouillé ne soit jamais plus basse que ce que les e2e montrent déjà.
30. En tant que développeur qui chasse un accès hors durée de vie, je veux qu'en 3a la place rendue soit un bloc libéré par la transitoire, pour que la quarantaine d'ASan attrape toute lecture d'une particule morte.
31. En tant que développeur, je veux qu'en 3b une place rendue soit empoisonnée par la zone à places, pour que `make sanitize-net` attrape une lecture tardive, dans la limite d'un tour complet du curseur.
32. En tant que développeur, je veux qu'un `pico8_fatal` « zone particules <sorte> pleine » soit d'abord lu comme une place jamais rendue, pour ne relever un nombre de places qu'une fois ce cas écarté.
33. En tant que développeur, je veux qu'un `pico8_fatal` « zone place qy pleine » (ou `qy_fv`, `ga`) dise qu'une liste a dépassé sa capacité réservée ou qu'une arène est trop petite, pour relever cette capacité en un chiffre.
34. En tant que développeur, je veux, pour chaque sorte, un TU qui construit le plus gros objet que le jeu met dans sa place (liste pleine à sa capacité, éclat avec son `oe` et sa boîte) et montre qu'il y tient, pour qu'une taille de place sans marge se voie dans `make test` et pas dans une partie.
35. En tant que mainteneur du budget, je veux une vérification à la compilation que la somme des zones à places de particules tient dans `PICO8_ZONE_PARTICLES_BYTES`.
36. En tant que mainteneur du budget, je veux que tout relèvement de `PICO8_ZONE_PARTICLES_BYTES` au-delà de ce que permettent les 1,5 Mo (49 Ko de marge) arrête l'étape et le signale (zone, limite voulue, somme actuelle), pour ne jamais rogner une autre zone.
37. En tant que gardien du cliquet, je veux qu'aucun commit de l'étape ne fasse monter un compte, pour respecter « plus aucun compte ne monte après 0b ».
38. En tant que gardien du cliquet, je veux qu'en fin d'étape `pico8_particle.c` ait quitté la liste d'exceptions, et que `probe`, `planet_family` et `planet` aient baissé, pour que le verrou interdise toute nouvelle allocation de particule.
39. En tant que gardien de l'invariant, je veux que les `*.sha256` restent identiques à chaque commit, pour prouver que rendre une particule ne change pas un pixel ni un tirage `rnd`.
40. En tant que développeur des TU existants, je veux que les tests qui glissent une particule statique dans `qy` la prennent désormais à la poignée de la sorte `particule`, et que les tests qui enchaînent `jl()` sans frame vident la file de sortie, pour qu'aucune place ne s'épuise dans un binaire de test.
41. En tant que lecteur du plan, je veux une note « Découvert » qui dise les sites classés autrement que dans le §2, les tailles et nombres de places retenus et pourquoi, chaque `pico8_fatal` rencontré, et les comptes laissés.
42. En tant que développeur de l'étape 4, je veux des lignes « *Reçu de l'étape 3* » sous l'entrée 4 (struct `g_`, struct `fv`, `oe` des planètes par `jo:pq` à zone passée), pour savoir ce que l'étape 3 m'a laissé et pourquoi.
43. En tant que mainteneur des commentaires, je veux que les `/* TROU-7 */` des fuites refermées (`f_`, `g_:mm`) soient retirés, que les contrats « nothing frees it » de `pico8_particle.h` disent qui rend quoi, et que le commentaire de `rendre` dans `pico8_struct.h` ne parle plus de la seule zone interface, pour que les commentaires restent vrais.

## Implementation Decisions

### Ce qu'est une sorte de particules

| Sorte | Objet | Créé par | Meurt (point de libération) |
|---|---|---|---|
| `particule` | un élément de `qy`, `pico8_qy_t` (32 octets) | `ez` → `self:mm()` : `g_:mm` (`g_`, `fv`, `hj`, `pm`, `mx`), `ga:mm` | `ei`, quand `l <= 1` : rendue juste après `pico8_ptrs_del` ; ou avec sa liste |
| `qy` | la liste `qy` d'un émetteur autre que `fv` (en-tête `pico8_ptrs_t` et tableau réservé), dans une arène | `f_` : `g_.pq`, `hj`/`pm`/`mx` à leur construction, `ga:pq` | `rendre` de l'hôte, au vidage de la file (hôte retiré par `he` ou abandonné par `jl()`) ; ou nouveau `f_` sur le même hôte |
| `qy_fv` | la liste `qy` de l'écran d'aide, dans une arène | `f_` dans la construction de `fv` | `rendre` de `fv` (le `jl()` de `eu`) |
| `ga` | l'éclat : struct `pico8_jo_t`, son `oe`, et sa boîte `ra` quand `pm:pq` la fournit, dans une arène | `pm:pq` (protostar), `jo:hu` (`kv["ga"]`, « harvest ») | `rendre` de `ga`, au vidage qui suit son `p_` (posé par `ga:kj` quand la troisième place de `qy` est vide → `dg` → `he`) ; ou `jl()` |

- **Hôtes émetteurs** (classes qui portent un `emitter`) : `g_`, `fv`, `hj`, `pm`, `mx`, `ga`.
- **Ce que l'étape ne rend pas.** La struct d'une sonde `g_`, celle de `fv`, et les planètes `hj`/`pm`/`mx` ne sont pas des particules (§2). Seul ce qu'elles tiennent en zone particules est rendu. Les structs `g_` et `fv` vont à l'étape 4 (voir « Sites écartés »).

### Mort d'une particule : dans `ei`, sur-le-champ

- `ei` retire la particule de `qy` puis rend sa place à la poignée `particule`. La particule n'est plus lue ensuite : la branche `else` qui la dessine n'est pas prise.
- **Pourquoi pas par la file.** La liste est la seule propriétaire d'une particule. Les seuls autres lecteurs sont `ga:kj`, qui lit `qy[1..3]` **avant** d'appeler `ei`, et les TU. Rendre sur-le-champ évite des centaines d'entrées de file par frame, que la file (256) ne tiendrait pas. Cette preuve est une lecture ; elle se vérifie en 3a par la quarantaine d'ASan.
- **Piège à commenter (`ga:kj`).** Le cart fait `self.p_ = not pt` après `ei(self)`. En C, `pt` peut viser une particule que `ei` vient de rendre. On garde un booléen calculé avant `ei` (« la troisième place était occupée ») au lieu du pointeur : la valeur de `p_` est la même, et aucune place rendue n'est comparée. ASan ne verrait pas cette comparaison (§4).
- **Ordre des tirages inchangé.** Rendre une place ne tire aucun `rnd` ; l'ordre de `ez` et de `mm` ne bouge pas. Le saut d'un élément après `del` (boucle `pairs`, TROU-14) est conservé tel quel.

### Liste `qy` : une arène à capacité réservée

- `f_` prend une arène de la sorte `qy` ou `qy_fv`, de la même façon que `pico8_zone_interface_place` (`pico8_zone_arene_prendre` sur la poignée de la sorte, nommée « place qy » ou « place qy_fv », de la taille entière de la place). Il y range l'en-tête `pico8_ptrs_t` et un tableau d'éléments réservé **en une fois** à la capacité de la sorte.
- `pico8_ptrs` gagne une initialisation à capacité réservée (nom laissé à l'implémenteur). Au-delà de cette capacité, l'agrandissement passe par la poignée de l'arène, qui est pleine : `pico8_fatal` « zone place qy pleine ». C'est le signal de relever la capacité de la sorte. Sans réserve, les doublements successifs (4, 8, …, 256) laisseraient dans l'arène des copies mortes aussi grosses que le tableau final (une arène « agrandit en copiant et laisse l'ancien bloc mort », `pico8_zone_choisie.h`).
- **Choix de la sorte par l'hôte** : `fv` prend `qy_fv`, tous les autres `qy`. La forme du choix (paramètre de `f_`, variante, champ de l'émetteur) est laissée à l'implémenteur ; aucune donnée générée du cart ne change.
- **`f_` sur un hôte qui a déjà une liste** : il rend ses particules et l'ancienne arène avant d'en prendre une nouvelle. Aujourd'hui aucun chemin n'appelle `f_` deux fois sur le même hôte ; la règle couvre la réaffectation `self.qy={}` du cart (TROU-7, champ réaffecté) au lieu d'abandonner la place.
- **Rendre une liste** = rendre chaque particule encore dans `qy`, puis l'arène (`pico8_zone_arene_rendre`), puis remettre `emitter->qy` à NULL. Une seule fonction de `pico8_particle`, appelée par les `rendre` des émetteurs et par `f_`.
- **Pas d'entrée de file pour une liste.** Une liste n'est rendue qu'au vidage, depuis le `rendre` de son hôte, ou dans `f_` quand l'hôte se refait une liste (aucun lecteur de l'ancienne). `pico8_file_sortie_mettre_arene` n'est pas utilisé par l'étape 3.

### Éclat `ga` : une arène

- `pico8_ga_struct_new` prend une arène de la sorte `ga`. Il y range sa struct, son `oe` et, quand elle est fournie par valeur, sa boîte `ra`.
- **`jo:pq` à zone passée.** `pico8_jo_pq` reçoit une variante qui prend la zone de `oe` en paramètre, sur le modèle des fonctions de 0b. L'éclat passe la poignée de son arène ; toutes les autres planètes passent encore la transitoire, par la forme actuelle de `jo:pq` qui délègue à la variante. Le compte de `pico8_planet.c` baisse d'une référence (le `malloc` de `oe` devient une allocation par la poignée) et aucun autre fichier ne monte.
- **Position de l'éclat.**
  - `pm:pq` passe une **valeur** (`ra + ng`) par les clés de `ga` ; l'éclat la recopie dans une boîte de son arène. Personne d'autre ne tenait la boîte que `pm:pq` allouait : rien ne change à l'écran.
  - Par `jo:hu` (`kv["ga"]({p=…,ra=…,id=…})`), l'éclat **emprunte** le `p`, le `ra` et l'`id` de la planète remplacée, comme dans le cart. Ce sont des objets de partie (étape 4), jamais rendus par l'étape 3.
- `by` reste la liste partagée `g_ga_by_shared` : statique, jamais rendue.
- **`rendre` de `ga`**, dans l'ordre : `pico8_jo_rendre_dispatch` (rangée `lq_place`, étape 2), puis la liste `qy` et ses particules, puis l'arène de l'éclat. L'arène n'est rendue qu'au vidage, en tête du `pico8__update60()` suivant, après `oublier` : `ga:kj` pose `p_` pendant le `draw`, `dg` fait `he` à l'`update` suivant puis écrit encore `t`.

### Inventaire des pointeurs qui survivent à la frame (§4)

Fait **mécaniquement** à `HEAD` ecbf87c, sur les 63 fichiers de `CORE_SRCS` :

- `grep -nE '^\s*static\s[^(]*\*\s*[a-zA-Z_][a-zA-Z0-9_]*(\[[^]]*\])?\s*(;|=)'`, hors `const char *` et tables `const … *const` ;
- puis `grep -nE '^static\s+pico8_[a-z0-9_]+_t\s+…'` pour les structures statiques (non `const`), dont on lit les champs pointeurs ;
- puis les champs pointeurs des singletons atteints par ces pointeurs (`ok`, `pj`, `mo`, `je`, `fm`).

Les sortes que l'étape 3 rend sont : une **particule** (`pico8_qy_t *`), une **liste `qy`** (`pico8_ptrs_t *` d'un émetteur), un **éclat `ga`** (`pico8_jo_t *` ou `pico8_struct_header_t *`). Un pointeur dont le type ne peut désigner aucune des trois est classé d'office « ne peut pas viser ».

**Variables de portée fichier.**

| Pointeur | Fichier | Type | Statut |
|---|---|---|---|
| `g_gfx`, `g_mem`, `g_input` | `dispatch_context` | gfx, mem, entrées | ne peut pas viser (type) |
| `g_pi_gfx`, `g_pi_mem` | `pi` | gfx, mem | ne peut pas viser (type) |
| `g_dk` | `economy_score` | `pico8_js_t *` | ne peut pas viser (type) ; repointé par `oublier` pour les boutons (étape 2) |
| `g_mo` | `entity` | `pico8_struct_header_t *` | ne peut pas viser : n'est écrit qu'avec le `qb` de `jl()` ou son tenant-lieu |
| `g_bw_oc_r_ctx` | `galaxy` | `pico8_jf_state_t *` | ne peut pas viser (type) |
| `g_jo_oc`, `g_jo_ir` | `planet` | états de `pico8_gen` | ne peut pas viser (type) |
| `g_kc` | `msgbox` | `pico8_mh_t *` | ne peut pas viser (type) ; remis à nil par `oublier` |
| `g_fm` | `probe` | `pico8_struct_header_t *` | ne peut pas viser : un `hy` ou son tenant-lieu (`assert` de `fm_free`) |
| `g_gb[]` | `research` | `pico8_hp_t *` | ne peut pas viser (type) |
| `g_ie_gy` | `research_icon` | `const pico8_vec_t *` | ne peut pas viser (type) |
| `g_ju` | `score_panel` | `pico8_pj_t *` | ne peut pas viser (type) |
| `g_je` | `ui` | `pico8_struct_header_t *` | ne peut pas viser : n'est écrit qu'avec le `nf` de `jl()` ou son tenant-lieu |
| `g_ok` | `ui` | `pico8_ui_t *` | ne peut pas viser (type) ; ses champs plus bas |

**Champs des structures statiques et des singletons.**

| Champ | Type | Statut |
|---|---|---|
| `g_hv`, `g_cv[].list` (`entity`) | listes d'entités | peuvent tenir un éclat, une sonde, `fv`, une planète émettrice : **retiré** par `pico8_he` avant l'entrée en file ; listes refaites par `jl()` avant tout vidage |
| `g_cg[].list` (`entity`) | listes d'entités par étiquette | un éclat n'a **aucune** étiquette (`.tags = 0`) : ne peut pas le viser ; `g_`, `fv` et les planètes y sont retirés par `he` ou refaits par `jl()`, et leur struct n'est pas rendue |
| `ok.o_`, `ok.gj` | `pico8_struct_header_t *` | **remis à nil** par `oublier` |
| `ok.lheld` | `pico8_struct_header_t *` | ne peut pas viser un éclat : ce que rend un `ldown` est un `jc`, une sonde `g_` ou un `mz` de `bg`, ou une planète `qh`/`os`/`nr`/`fr`/`nl`/`mx` (`assert` de `ui:cj`) ; une sonde tenue garde sa struct (non rendue), seul son `qy` l'est, que `bv` ne lit pas |
| `ok.rheld` | `pico8_bg_t *` | ne peut pas viser (type) |
| `ok.ic` → `ic_own.layers[]` | listes d'entités survolées | ne peut pas viser un éclat : sa boîte `kq=b(0,0,-1,-1)` est vide, il n'est jamais survolé ; vidées et refaites à chaque `gw` |
| `ok.le`, `g_shared_le` | listes de boutons | ne peut pas viser (n'y entrent que des `js`) |
| `ok.qa` | `pico8_vec_t *` | ne peut pas viser (type) |
| `ju.ed` | `pico8_mh_t *` | ne peut pas viser (type) ; remis à nil par `oublier` |
| `g_bv_et.fo` (`probe`) | `pico8_struct_header_t *` | ne peut pas viser : c'est la planète trouvée par `pico8_ij_ents` dans `cg("np")`, où un éclat n'est jamais |
| `g_jc_too_far.fo` (`slipway`) | `pico8_jc_t *` | ne peut pas viser (type) |
| `g_ga_by_shared` (`planet_family`) | liste d'entités (routes `by`) | ne peut pas viser : liste vide partagée, où n'entrent que des routes |
| `fm.invented` (`hy`) | `pico8_ptrs_t *` de projets | ne peut pas viser : liste propre à `hy`, ce n'est pas un `qy` |
| `mo.v`, `mo.p`, `mo.ra` ; `je.ra` | `pico8_vec_t *` | ne peut pas viser (type) |
| `g_ha_pz`, `g_pj_cr`, `g_bz[]`, `g_mo_placeholder`, `g_je_placeholder`, `g_fm_placeholder`, `g_mz_jj_wrapper`, `g_pj_ou`, `g_pj_ea`, `g_pi_dispatch[]`, zones et poignées | vecteurs, textes, entrées constantes, ou aucun champ pointeur | ne peut pas viser (type) |

**Conclusion : aucune référence à ajouter à `oublier`.** Particules et listes ne sont désignées par aucun pointeur de cet inventaire : une particule n'est tenue que par sa liste (et par la variable locale `pt` de `ga:kj` pendant la frame, traitée plus haut), une liste par `emitter->qy` de son hôte, remis à NULL au rendu. Hors du périmètre mécanique du §4, la lecture confirme pour l'éclat : `me="unknown"` pour toujours (pas dans `cg.np`, jamais découvert), donc `jo:ldown` ne crée aucun `jc` qui le tiendrait en `qu`.

**Dans chaque morceau qui rend une sorte (2, 3, 4)**, le même `grep` est refait sur le code du moment ; un pointeur nouveau est classé dans le commit. Un pointeur trouvé ensuite par ASan est ajouté à `oublier` et noté dans « Découvert » comme un **trou de l'inventaire** (§4).

### Maximum de remplissage (§3)

- **Ce qui est relevé, par sorte de particules :**
  - le maximum de places vivantes atteint pendant la session (relevé à chaque prise, pas en fin de frame) ;
  - pour `qy`, `qy_fv` et `ga`, le maximum d'octets pris dans une place-arène (en-tête compris), relevé quand la place est rendue et, pour celles encore vivantes, à l'écriture ;
  - pour `qy` et `qy_fv`, le maximum d'éléments d'une liste : avec la capacité réservée, les octets d'une arène de liste sont constants, et c'est ce nombre-là que la capacité borne.
- **Où.** Le module de zone particules tient ces maxima, et l'accesseur de remplissage les rend à côté des places vivantes. Avec `PICO8_REMPLISSAGE`, `main.c` les écrit en fin de session à côté du remplissage, sans changer la capture, le dump ni le code de sortie.
- **3a : relevé sur tous les e2e.** Les 26 e2e de `tests/e2e/` sont rejoués avec `PICO8_REMPLISSAGE`, et le maximum de chaque sorte sur l'ensemble est noté. La forme est laissée à l'implémenteur : boucle de shell dans le commit, ou cible `make` hors suite comme `class-coverage`. Le résultat va dans le message du commit du morceau 5.
- **3b : plancher.** Aucune taille de place, capacité de liste ni nombre de places n'est fixé sous le maximum relevé en 3a. Si la table des tailles plus bas est sous un maximum, on relève la limite, dans la marge de 49 Ko, ou on s'arrête et on signale (§3).
- **« Découvert »** donne, par sorte, le maximum relevé et la limite retenue.
- Les sortes d'interface n'ont pas encore ce maximum. L'étape 2 était close avant l'ajout au §3, et l'y ajouter n'est pas de cette étape (Out of Scope).

### Membres `rendre` des émetteurs

| Classe | `rendre` après l'étape 3 |
|---|---|
| `g_` | rendre `qy` (nouveau membre) |
| `fv` | rendre `qy_fv` (nouveau membre) |
| `hj`, `pm`, `mx` | `pico8_jo_rendre_dispatch`, puis rendre `qy` |
| `ga` | `pico8_jo_rendre_dispatch`, puis rendre `qy`, puis rendre l'arène `ga` |

- Les autres classes de la famille `jo` gardent `pico8_jo_rendre_dispatch` sans changement.
- `pico8_he` et `pico8_jl` enfilent déjà toute entité dont la classe a `rendre` : l'étape 3 n'ajoute aucun site d'entrée.
- **File de sortie (capacité 256, pire frame estimée par l'étape 2 à ~210 entrées).** `ga`, `hj`, `pm` et `mx` y entraient déjà par l'étape 2 (ils ont `rendre`) : ils sont dans les ~210. Les seules entrées nouvelles sont `g_` et `fv`. Au pire d'une frame (le `jl()` de « restart » ou de « again? ») : au plus une sonde en vol (`bg:ldown` fait un `mz`, pas une sonde, quand `cg.g_` n'est pas vide) et au plus une aide (elle ne vit qu'à l'écran titre, et seul un bouton de l'aide quitte le titre). **Recompte : ≤ ~212 sur 256**, sans relèvement. Les listes et les particules n'entrent jamais dans la file.

### Module de zone particules et poignées des sortes

Il suit la forme de `pico8_zone_interface.{h,c}` (énumération des sortes, `SORTE_BYTES`, `PICO8_SLOT_ZONE_STORAGE` par sorte, initialisation paresseuse, poignée `pico8_zone_choisie_slot`, accesseur `pico8_slot_zone_taken`), dans un module à lui :

- **Module du cœur**, hors liste d'exceptions : il ne référence ni la libc ni `pico8_zone_transitoire`. Pour chaque sorte, il expose la poignée de la sorte, une fonction qui prend une place en arène (pour `qy`, `qy_fv`, `ga` ; `particule` n'a pas d'arène, une place de 32 octets) et le nombre de places vivantes.
- **3a : adossé à la transitoire**, comme l'a fait l'étape 2 en 2a (`pico8_zone_interface_adosser(&pico8_zone_transitoire)` depuis `pico8_ui_button.c`, retiré en 2b par 26852b1). Le module compte les places prises et rendues et délègue à une poignée d'adossement. Cette poignée lui est donnée par `pico8_particle.c` — **une seule référence comptée à la transitoire** — dans le commit qui retire déjà de ce fichier le `malloc` et la référence de `f_` (2 → 1). Aucun compte ne monte. En 3a, une place est donc un bloc `malloc` de la taille de la sorte : les tailles de place sont éprouvées dès 3a.
- **3b : zones à places.** Un `PICO8_SLOT_ZONE_STORAGE` par sorte, la poignée `pico8_zone_choisie_slot` (`alloc(n)` exige `n` ≤ taille de place, `grow` refusé, `release` rend la place). L'adossement disparaît : `pico8_particle.c` sort de la liste. Aucun point de libération ne bouge.
- **Messages de `pico8_fatal`.** « zone particules <sorte> pleine » (nom de la zone à places) : place jamais rendue d'abord, limite trop basse ensuite. « zone place qy pleine » (nom de l'arène, comme « zone place js pleine » à l'étape 2) : capacité réservée ou place trop petite.

### Tailles et nombres de places (au doigt mouillé, justifiés par lecture)

Ces valeurs ne sont pas mesurées (§3). Leçon de l'étape 2 : une taille de place compte l'**en-tête d'arène (96 o)**, et un nombre de places se justifie en lisant la **pire frame**, pas par le pic d'un e2e. `sizeof` relevés à `HEAD` 3b28708 (x86-64) : `pico8_qy_t` 32, `pico8_ptrs_t` 24, `pico8_ents_t` 24, `pico8_jo_t` 432 (l'étape 2 a ajouté `lq_place`), `pico8_g__t` 256. Une zone à places coûte en plus un octet par place (`SORTE_BYTES`).

Vies, lues dans le cart (`l` perd 0,14 par frame où l'émetteur est à l'écran ; mort à `l <= 1`) :

- `fv` : `fc=5` (exactement 5 par `draw` : `rnd()<n` est toujours vrai pour `n >= 1`), `ki=17` → 115 frames, **~575 vivantes** en régime, ~580 avec les sauts de TROU-14 ;
- `g_`, `pm` : `fc=1`, `ki=9` → ~58 vivantes ;
- `hj` : `fc=0.5` → ~29 ; `mx` : `fc=0.3` → ~17 ;
- `ga` : exactement 140 à la naissance (`ez(self,140)`), `l` dans [6,12[ → au plus ~79 frames, `fc=0`.

Un émetteur hors de l'écran (`fa < 10` et pas `gd`) ne fait ni naître ni vieillir ses particules : son compte est figé, il ne croît pas.

| Sorte | Place | Nombre | Octets (`SORTE_BYTES`) | Pire frame, par lecture |
|---|---|---|---|---|
| `particule` | 32 o | 2048 | 67 584 | titre : ~580 (aide seule). En partie : une sonde au plus (~58) + les planètes émettrices posées par le joueur (au pire 58 chacune) + les éclats vivants (140 chacun). Avec 16 planètes émettrices et 3 éclats : ~1410. Frame de « restart » : tout cela encore pris jusqu'au vidage, plus les 5 premières de la nouvelle aide. |
| `qy` | 1400 o (en-tête 96 + `pico8_ptrs_t` 24 + 160 × 8) | 24 | 33 624 | capacité : un éclat (140) et une sonde ou un `pm` (~58, plus les sauts). Nombre : une sonde + les planètes émettrices + les éclats, tous encore pris à la frame de « restart » jusqu'au vidage ; 24 admet 23 planètes émettrices et éclats. |
| `qy_fv` | 6264 o (96 + 24 + 768 × 8) | 1 | 6 265 | capacité : ~580, avec marge. Nombre : l'aide ne vit qu'à l'écran titre ; elle sort par le `jl()` de `eu` sans qu'une autre naisse, et un `_init` (« restart », « again? ») part d'une partie, où il n'y a pas d'aide. Une seule à la fois. |
| `ga` | 640 o (96 + struct 432 + `oe` 24 + boîte 8 + 80 de marge pour les éléments de `oe`) | 8 | 5 128 | un éclat par « harvest » ou « collapse », chacun un clic de menu, éteint en ≤ 79 frames visibles ; figé hors écran. 8 au doigt mouillé ; un `pico8_fatal` est d'abord lu comme un éclat jamais rendu. |
| **Total** | | | **112 601 (≈ 110 Ko)** | |

- **Le total tient dans `PICO8_ZONE_PARTICLES_BYTES` (128 Ko) sans relèvement**, avec ~18 Ko de marge. Somme des zones à `HEAD` : frame 256 + interface 335 + particules 128 + partie 768 = **1487 Ko sur 1536**. L'étape 3 ne la change pas.
- **Marge pour un relèvement : 49 Ko.** La zone particules peut monter jusqu'à 177 Ko sans dépasser 1,5 Mo. Au-delà, l'étape s'arrête et signale à l'humain : zone particules, limite voulue, somme actuelle (§3). Elle ne rogne ni la zone partie ni l'interface. Tout relèvement consomme la marge dont l'étape 4 dispose : le message de commit le dit.
- **Plancher mesuré (§3).** Ces tailles et nombres sont des valeurs de départ pour 3a. En 3b, chacun est comparé au maximum relevé sur les 26 e2e (« Maximum de remplissage ») et relevé s'il est en dessous ; « Découvert » donne les deux chiffres par sorte.
- Une vérification à la compilation, comme celle de `pico8_zone_interface.c`, exige que la somme des `SORTE_BYTES` des sortes de particules tienne dans `PICO8_ZONE_PARTICLES_BYTES`.
- **Lecture d'un `pico8_fatal`** : voir « Module ». La note « Découvert » dit, pour chacun, lequel des deux cas c'était (à l'étape 2 : trois `pico8_fatal`, tous « limite trop basse », dont deux par oubli de l'en-tête d'arène).

### Inventaire des sites — convertis à l'étape 3

Relevé à `HEAD` 3b28708 sur une copie de `HEAD` : `make alloc-guard` y donne « 63 objets, 25 allouent, 91 appels », égal à `tools/alloc_guard_exceptions.txt`. Les relocations ont été rattachées à leur fonction et à leur ligne par `objdump -r` et `addr2line` sur les objets `-O0 -g` d'`alloc-guard`. L'étape 2 n'a touché aucun de ces sites ; seuls des numéros de ligne ont bougé (`planet_family` +4 à +6, `planet` +3).

« T » désigne une référence à `pico8_zone_transitoire`, « libc » un appel `malloc`/`calloc`/`realloc`/`free`.

| Fichier | Fonction (ligne) | Objet | Devient | Baisse |
|---|---|---|---|---|
| `pico8_particle.c` | `pico8_f_` (37, 38) | struct `qy` (libc), tableau (T) | arène `qy`/`qy_fv`, capacité réservée | 2 (1 libc + 1 T) |
| | (adossement du module, 3a) | référence ajoutée en 3a, compensée par les deux ci-dessus (2 → 1) | retirée en 3b | 1 T en 3b |
| `pico8_probe.c` | `pico8_g__mm` (354) | particule | place `particule` | 1 libc |
| `pico8_planet_family.c` | `pico8_ga_mm` (379) | particule | place `particule` | 1 libc |
| | `pico8_ga_struct_new` (458) | struct `ga` | arène `ga` | 1 libc |
| | `pico8_pm_pq` (277) | boîte `ra` de l'éclat | valeur recopiée dans l'arène `ga` | 1 libc |
| `pico8_planet.c` | `pico8_jo_pq` (198, 199) | `oe` (libc + T) | variante à zone passée ; la forme actuelle garde une référence T pour les planètes | 1 (2 → 1) |

### Baisse attendue du cliquet, par fichier

| Fichier | À `HEAD` 3b28708 (mesuré) | Après 3a | Après 3b |
|---|---|---|---|
| `pico8_particle.c` | 2 | 1 | sort |
| `pico8_probe.c` | 4 | 3 | 3 |
| `pico8_planet_family.c` | 9 | 6 | 6 |
| `pico8_planet.c` | 7 | 6 | 6 |
| **Liste** | 25 fichiers, 91 appels (mesuré) | 25 fichiers, 85 | 24 fichiers, 84 |

Ces chiffres sont des **prévisions** : le verrou compte des relocations, qui dépendent du compilateur. La règle de fin est celle du §5 :

- chaque site de l'inventaire est converti, vérifié sur la source : la fonction ne référence plus la transitoire ni la libc pour ces objets ;
- `alloc-guard` est à égalité stricte sur les comptes mesurés ;
- aucun compte n'a monté.

L'écart se note dans le message de commit et dans « Découvert ». Il n'est jamais compensé ailleurs.

### Sites écartés de l'étape 3 (et où ils vont)

Ils sont listés pour la note « Découvert » et pour les lignes « *Reçu de l'étape 3* » sous l'entrée 4.

- **La struct d'une sonde `g_`** (`pico8_g__struct_new`, `pico8_probe.c` 409). Le §2 range `g_:mm` en zone particules, pas la sonde. L'interface la tient d'une frame à l'autre (`out.held`) et lui envoie `hq` : objet relu au-delà de sa sortie, zone la plus longue (§4), étape 4. Une ligne « Reçu » le signale : la sonde a désormais `rendre` et passe par la file ; en zone partie, chaque lancer coûterait une struct jusqu'à la nouvelle partie (croissance par action, pas par tick).
- **La struct de `fv`** (`pico8_help.c` 116). Le *Reçu de l'étape 2* sous l'entrée 4 la range déjà à l'étape 4. L'étape 3 ne prend que sa liste et ses particules.
- **Le `oe` des planètes** (`pico8_jo_pq` par sa forme actuelle). Il reste sur la transitoire ; l'étape 4 n'a qu'à passer la zone partie à la variante. Ligne « Reçu ».
- **`p` de `pico8_bv`** (`pico8_probe.c` 162) et `fm_free` (449) : déjà reçus par l'étape 4 (*Reçu de l'étape 1*), non touchés.
- **Les structs `hj`, `pm`, `mx`** : planètes, étape 4 (« les structures de planète ne sont toujours pas libérées », *Reçu de l'étape 2*).

## Implementation Chunks

Chaque morceau est un commit (ou une courte suite) qui respecte l'invariant du §3 et remet `alloc-guard` à égalité stricte. Son message dit les baisses mesurées par fichier et leur écart à la prévision. **Aucun morceau ne fait monter un compte.** Un site dont la conversion fait planter `make sanitize-net` n'est pas commité ; il est corrigé (sortie manquante, référence oubliée — à ajouter à `oublier` comme `dk`) ou noté pour « Découvert ».

Les tables partent de la liste mesurée à `HEAD` 3b28708 (25 fichiers, 91 appels).

**3a — Points de libération, sur la transitoire**

1. **Socle.**
   - Le module de zone particules, ses quatre sortes, leurs tailles de place, leurs compteurs et l'entrée d'adossement ; encore sans adossement ni utilisateur.
   - L'initialisation à capacité réservée de `pico8_ptrs`, et ses TU.
   - La fonction qui rend une liste `qy` (particules, puis arène).
   - Les maxima par sorte (places vivantes, octets d'une place-arène, éléments d'une liste) dans le module, et leur écriture par `main.c` sous `PICO8_REMPLISSAGE`.

   Comptes inchangés.
2. **Listes `qy`, et adossement du module.**
   - `f_` prend une arène `qy` ou `qy_fv` à capacité réservée ; un second `f_` rend l'ancienne.
   - Les `rendre` de `g_` et `fv`, et ceux de `hj`, `pm`, `mx`, `ga` qui appellent `pico8_jo_rendre_dispatch` puis rendent la liste. Les particules sont encore des `malloc` : la fonction qui rend une liste les rend par la poignée qui les a données, ce qui impose de passer `g_:mm` et `ga:mm` dans le même commit, ou de faire ce morceau et le suivant en un seul commit. L'implémenteur choisit ; aucun compte ne monte dans les deux cas.
   - L'adossement du module, dans `pico8_particle.c`, qui retire dans le même commit le `malloc` et la référence de `f_`.
   - Les TU qui enchaînent `jl()` sans frame (`start_title`, `start_game` et semblables) vident la file : `g_` et `fv` y entrent désormais.
   - L'inventaire des pointeurs (§4) refait par `grep` sur le code du commit pour les listes ; un pointeur nouveau y est classé.

   | Fichier | Avant | Après |
   |---|---|---|
   | `particle` | 2 | 1 (l'adossement) |

   Liste : 25 fichiers, 90 appels.
3. **Particules.**
   - `g_:mm` et `ga:mm` prennent leur particule à la poignée `particule`.
   - `ei` rend la particule retirée ; `ga:kj` garde un booléen au lieu du pointeur.
   - Les TU qui glissent une particule statique dans `qy` (`test_pico8_fv`, `test_pico8_ga`) la prennent à la poignée.
   - L'inventaire des pointeurs refait pour les particules.

   | Fichier | Avant | Après |
   |---|---|---|
   | `probe` | 4 | 3 |
   | `planet_family` | 9 | 8 |

   Liste : 25 fichiers, 88 appels.
4. **Éclats `ga`.**
   - L'arène `ga` (struct, `oe` par la variante de `jo:pq`, boîte `ra` recopiée depuis la valeur de `pm:pq`) ; l'emprunt de `jo:hu` est conservé.
   - Le `rendre` de `ga` rend son arène en dernier.
   - L'inventaire des pointeurs refait pour l'éclat.

   | Fichier | Avant | Après |
   |---|---|---|
   | `planet_family` | 8 | 6 |
   | `planet` | 7 | 6 |

   Liste : 25 fichiers, 85 appels.
5. **Preuve des places vivantes, et relevé du maximum.** L'extension de `test_pico8_places_vivantes` aux sortes de particules, l'invariant par frame, et le TU de régime à l'écran titre (voir Testing Decisions).
   - Le relevé du maximum de chaque sorte sur les 26 e2e (§3), écrit dans le message du commit. C'est le plancher des limites de 3b.
   - Si un compte ne revient pas à son niveau, ou si l'invariant `particule` = Σ `#qy` casse, c'est une sortie manquante : on la trouve avant de passer à 3b.

**3b — Conversion**

6. **Zones à places de particules.**
   - Le stockage par sorte dans le module, et la vérification de somme contre `PICO8_ZONE_PARTICLES_BYTES`.
   - Chaque taille de place, capacité de liste et nombre de places au moins égal au maximum relevé au morceau 5 ; sinon relevé dans la marge, ou arrêt et signalement (§3).
   - `PICO8_ZONE_PARTICLES_BYTES` relevé seulement si la somme l'exige et si les 1,5 Mo le permettent (aucun relèvement prévu ; plafond permis 177 Ko). Sinon : arrêt et signalement (§3).
   - Le retrait de l'adossement : `pico8_particle.c` sort de la liste. Liste : 24 fichiers, 84 appels.
   - Aucun point de libération n'est ajouté ni déplacé.
7. **Clôture de l'étape.**
   - La note « Découvert » sous l'entrée 3 du plan : sites classés autrement que dans le §2, par sorte le maximum relevé sur les e2e et la limite retenue (taille, capacité, nombre), chaque `pico8_fatal` rencontré (« place jamais rendue » ou « limite trop basse »), `PICO8_ZONE_PARTICLES_BYTES` et la somme des zones, comptes laissés et écarts à la prévision, et tout pointeur trouvé par ASan hors de l'inventaire du §4 (un trou de l'inventaire, ajouté à `oublier`).
   - La correction du §2 si un constat le contredit.
   - Les lignes « *Reçu de l'étape 3* » sous l'entrée 4 : struct `g_`, `oe` des planètes par la variante de `jo:pq`.
   - Le retrait des `/* TROU-7 */` des fuites refermées, la mise à jour des contrats de `pico8_particle.h` et du commentaire de `rendre` (`pico8_struct.h`).

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - un dessin de particule ou d'éclat ;
  - un compte de places vivantes rendu par l'accesseur ;
  - la longueur de `qy` d'un émetteur ;
  - un code de sortie, un message de `pico8_fatal`, un rapport ASan ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne regarde ni une adresse ni l'intérieur d'une arène.
- **TU de `pico8_ptrs` à capacité réservée** : on ajoute jusqu'à la capacité sans agrandissement ; un élément de plus sur une arène pleine sort en 3 avec « zone place <nom> pleine » (processus enfant).
- **TU du module de zone particules**, sur le modèle de `tests/test_pico8_zone_interface.c` et `tests/test_pico8_zone_choisie.c` (règle `asan_objs`, `UNIT_RUNS_SPECIAL`, `ASAN_OPTIONS=detect_leaks=0`) :
  - le compte de chaque sorte suit prises et rendus ;
  - une place `particule` rendue puis lue plante (enfant) ;
  - une zone pleine sort en 3 avec « zone particules <sorte> pleine ».
- **« Le plus gros tient dans sa place », un par sorte**, sur le modèle des tests de l'étape 2 (`test_pico8_js`, `test_pico8_mh`, `test_pico8_lq`) :
  - `qy` : une liste remplie à 160 éléments tient dans son arène ; le 161ᵉ sort par « zone place qy pleine » (enfant) ;
  - `qy_fv` : de même à 768 ;
  - `ga` : un éclat de protostar (boîte `ra` recopiée) et un éclat de « harvest », chacun avec son `oe`, tiennent dans leur arène ;
  - `particule` : `sizeof(pico8_qy_t)` ≤ taille de place (contrôle à la compilation ou `assert`).
- **TU par sorte**, dans les TU existants (passés en règle ASan si besoin) : `test_pico8_g_`, `test_pico8_fv`, `test_pico8_ga`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`.
  - Une particule dont `l` tombe sous 1 dans `ei` est rendue : le compte `particule` baisse d'un, et la lire ensuite plante (enfant).
  - Un émetteur qui sort (`he`, puis vidage de la file ; ou `jl()`, puis vidage) rend toutes ses particules et sa liste : les comptes `particule` et `qy` (ou `qy_fv`) reviennent à leur niveau d'avant sa naissance.
  - Une planète émettrice qui sort rend aussi sa rangée : le compte `rangee` de la zone interface baisse comme avant l'étape 3.
  - Un éclat qui s'éteint (`p_` par `ga:kj`, `dg`, vidage) rend son arène : le compte `ga` revient à 0.
  - Un éclat né d'un `pm` a la même position qu'aujourd'hui (`ra + ng`), et un éclat né par `jo:hu` suit le `ra` de la planète remplacée.
  - Les empreintes `fold_particle` existantes (`test_pico8_g_`, `test_pico8_ga`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`) ne changent pas : rendre une place ne tire aucun `rnd`.
- **Places vivantes, sur rejeu** : `tests/test_pico8_places_vivantes.c` (un rejeu par processus enfant, piloté comme `main.c`, comptes lus après `pico8_cart_vider_file_sortie()`) gagne les sortes de particules.
  - **Invariant sur tous les e2e.** À chaque frame de chacun des 26 e2e, après le `draw` et un vidage : `particule` = Σ `#qy` des émetteurs inscrits dans `hv` ; `qy` + `qy_fv` = nombre d'émetteurs inscrits qui ont une liste ; `ga` = nombre d'éclats inscrits. C'est la forme forte du critère : une particule perdue le casse à la frame même où elle l'est. Le compte `particule` varie naturellement d'une frame à l'autre (naissances `rnd`, morts au fil de `l`) : l'égalité brute du §3 ne s'y applique qu'à travers cet invariant.
  - **`pause_restart`** : `qy_fv` = 1, `qy` = 0, `ga` = 0 avant la frame 0 (écran titre) et après la frame 11 (écran titre après « restart » en f7). `particule` est, aux deux relevés, égal à `#qy` de l'aide courante, soit 5 par `draw` depuis son `_init` (aucune particule de l'aide ne meurt avant 115 frames). Les sortes d'interface y gardent leurs relevés de l'étape 2.
  - **`tech_geoharvesting_burst`** : `ga` passe à 1 au « harvest » (f56) et revient à 0 quand l'éclat s'éteint (vers f139) ; `qy` revient alors au niveau d'avant f56. Si l'éclat n'est pas éteint à la dernière frame, on le constate et « Découvert » le dit.
  - **`message_box_hold`** (contrôle « Croissance par frame » du §3) : tous les comptes égaux aux frames 45 et 90 de la phase sans action f42–90, comme à l'étape 2. Aucun émetteur n'y est actif si la partie n'a ni sonde ni planète émettrice : le relevé est alors trivial, et le TU de régime ci-dessous porte la preuve sur un émetteur actif.
  - **Régime sans entrée à l'écran titre** : le cart, chargé et laissé sans entrée 400 frames (au-delà des 115 frames de vie d'une particule de l'aide), garde `qy_fv` = 1 ; `particule` reste égal à `#qy` de l'aide et ne dépasse pas un plafond relevé entre les frames 200 et 400 (aucune hausse d'une fenêtre de 100 frames à la suivante).

  Le même TU sert en 3a (comptes adossés à la transitoire) et en 3b (zones à places).
- **Remplissage et maximum écrits par le rejeu** : avec `PICO8_REMPLISSAGE`, `main.c` écrit aussi, par sorte de particules, les places vivantes après le vidage qu'il fait déjà, et les maxima de la session. La capture, le dump et le code de sortie ne changent pas. Un TU (sur le modèle de `tests/test_pico8_replay_usage.c`) rejoue `tech_geoharvesting_burst` et vérifie que le maximum de `ga` vaut au moins 1 et celui de `particule` au moins 140, alors que le remplissage final de `ga` vaut 0 : un maximum qui ne serait relevé qu'en fin de session échoue à ce test.
- **Tests existants.** Ils restent verts sans changement d'attente. Seules adaptations permises :
  - prendre à la poignée `particule` une particule glissée à la main dans `qy` (`test_pico8_fv`, `test_pico8_ga`), et ne plus vider `qy` par `qy->count = 0` sans rendre ses particules ;
  - vider la file de sortie dans les aides de TU qui enchaînent `jl()` sans frame : sans quoi la file (256) ou la place unique `qy_fv` s'épuise dans un binaire de test (leçon de l'étape 2).
- **Verrou.** `make alloc-guard` (dans `make test`) à égalité stricte après chaque morceau. Pas de nouveau cas dans `tests/test_alloc_guard.c`.
- **Invariant.** `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. En 3a, la quarantaine de la libc attrape un usage après libération ; en 3b, l'empoisonnement des places rendues l'attrape, dans la limite d'un tour complet du curseur (§4). Le curseur de la sorte `particule` tourne vite (2048 places ; 5 prises par frame au titre, jusqu'à ~20 en partie) : un tour complet prend 100 à 400 frames, ce qui laisse à ASan une fenêtre plus courte que pour l'interface. Ce n'est pas une raison de promouvoir un site douteux (§4).
- **Exemples dans le dépôt** : `tests/test_pico8_zone_interface.c`, `tests/test_pico8_zone_choisie.c` (lecture périmée dans un enfant, zone pleine en 3) ; `tests/test_pico8_places_vivantes.c` ; `tests/test_pico8_file_sortie.c` ; `src/main.c` (boucle de rejeu, `PICO8_REMPLISSAGE`) ; `test_pico8_ga`, `test_pico8_g_` (TU qui bouclent `update60`/`draw` et replient les particules).

## Out of Scope

- Les structs `g_`, `fv`, `hj`, `pm`, `mx`, et le `oe` des planètes autres que l'éclat : étape 4.
- La file de sortie, le membre `rendre`, l'arène, la poignée sur zone à places et le module de zone interface eux-mêmes : faits à l'étape 2, réutilisés tels quels (seul le commentaire de `rendre` est élargi).
- L'accesseur de remplissage d'une zone montante : étape 4.
- Le maximum de remplissage des sortes d'interface (§3, ajouté après la clôture de l'étape 2) : non rétrofité ici.
- Retirer l'entrée TROU-7 de `docs/trous-de-portage.md` et des sources qui la citent encore pour d'autres sortes : étape 5. L'étape 3 ne retire que les `/* TROU-7 */` des fuites qu'elle referme.
- Changer le comportement du jeu : la boucle `pairs` qui saute un élément (TROU-14), un émetteur hors écran dont les particules sont figées, la palette d'un éclat de protoétoile (TROU-21).
- Compacter `pico8_qy_t` (`has_a` et `a` absents pour `g_`) : économie PSX (§7).
- Lancer l'application raylib.

## Vérifié contre l'étape 2

Relu contre `HEAD` 3b28708 :

- la note « Découvert » de l'étape 2 ;
- les lignes « *Reçu de l'étape 2* » sous les entrées 3 et 4 de `docs/plan-4.md` ;
- la ligne « Interface » corrigée du §2 (la ligne « Particules » est inchangée) ;
- `src/pico8_zone.h`, `pico8_zone_choisie.h`, `pico8_zone_interface.{h,c}`, `pico8_file_sortie.{h,c}`, `pico8_zone_sizes.h`, `pico8_struct.h`, `pico8_cart.c` (`oublier`, `pico8__update60`), `pico8_bootstrap.c` (`pico8_jl`), `pico8_entity.c` (`pico8_he`), `pico8_planet_display.c` (`pico8_jo_rendre_dispatch`), `src/main.c`, `tests/test_pico8_places_vivantes.c` ;
- `tools/alloc_guard_exceptions.txt` (`make alloc-guard` sur une copie de `HEAD` : 63 objets, 25 allouent, 91 appels) ;
- `git log -12 --stat` (489054d à 3b28708) ;
- l'amendement du plan ecbf87c (§3 : maximum par sorte écrit par le rejeu, relevé en a, plancher en b ; §4 : inventaire mécanique des pointeurs qui survivent à la frame), qui ne change pas `src/` ;
- `git diff 021cfa3 HEAD` sur les fichiers de l'inventaire.

Statut de chaque hypothèse du brouillon :

- **H-2-1 (file de sortie) — tenue, élargie.** `pico8_file_sortie` : tableau statique de 256 entrées, `pico8_fatal` « file de sortie pleine, limite 256 ». Entrée par `pico8_he` si l'entité était dans `hv` et que sa classe a `rendre`, et par `pico8_jl` pour chaque entité de `hv` qui a `rendre`, avant `pico8_entity_init`. Vidage en tête de `pico8__update60()`, après `pico8_zone_frame_reset()` et avant `pico8_dg()`, ainsi qu'à `pico8_cart_load()`, `pico8_cart_shutdown()` et dans `main.c` avant l'écriture du remplissage. Élargie : une entrée peut aussi être une **arène** (`pico8_file_sortie_mettre_arene`, pour les listes `le`) ; l'étape 3 n'en a pas besoin. **Recompte de la pire frame** fait ci-dessus : ~210 → ≤ ~212.
- **H-2-2 (membre d'archétype) — tenue.** Le membre s'appelle `rendre` ; c'est un seul pointeur de fonction, appelé au vidage après `oublier(e)`. Toutes les classes de la famille `jo`, dont `ga`, `hj`, `pm` et `mx`, ont `pico8_jo_rendre_dispatch`, qui rend `lq_place` et remet `lq`/`lq_place` à NULL. Le commentaire de `rendre` dit « in the interface zone » : l'étape 3 l'élargit.
- **H-2-3 (place-arène) — tenue, taille corrigée.** `pico8_zone_arene_prendre(from, name, size)` et `pico8_zone_arene_rendre(arene)`. L'arène n'est jamais remise à zéro et n'a qu'une moitié (`pico8_bump_zone_init_single`). Elle sort par « zone <name> pleine », avec un `name` de la forme « place js ». **L'en-tête coûte 96 o** : les tailles de place de cette PRD le comptent désormais (le brouillon disait « moins de 100 o » et le comptait à peine).
- **H-2-4 (module de zone à sortes) — tenue, avec un écart de forme.** Le module de l'étape 2 est propre à l'interface : énumération `pico8_sorte_t`, `pico8_zone_interface(sorte)`, `pico8_zone_interface_place(sorte)`, `pico8_zone_interface_vivantes(sorte)`. L'étape 3 en fait un pareil pour ses sortes au lieu d'y ajouter les siennes. `pico8_zone_choisie_slot` et `pico8_slot_zone_taken` existent. L'adossement de 2a (`pico8_zone_interface_adosser`, appelé depuis `pico8_ui_button.c`) a été retiré en 2b, et il ne reste aucun adossement à la transitoire dans le module d'interface. L'étape 3 refait le même schéma depuis `pico8_particle.c`.
- **H-2-5 (rejeu et remplissage) — tenue.**
  - `tests/test_pico8_places_vivantes.c` lance un rejeu par enfant et relève les comptes après le `draw` d'une frame, juste après un vidage.
  - `main.c` écrit le remplissage par sorte d'interface si `PICO8_REMPLISSAGE` nomme un fichier.
  - L'étape 3 étend les deux à ses sortes.
- **H-2-6 (limites) — tombée.** `PICO8_ZONE_INTERFACE_BYTES` vaut **335 Ko**, pas 320 Ko. La somme est de **1487 Ko sur 1536**, et la marge de **49 Ko**, pas 64 Ko. La zone particules reste à 128 Ko.
  - Conséquence : le plafond de relèvement de la zone particules passe de 192 Ko à **177 Ko**.
  - Le total prévu (≈ 110 Ko, recalculé) n'exige aucun relèvement.
- **H-2-7 (comptes laissés) — tenue.** La liste compte 25 fichiers et 91 appels, comme prévu. Fichiers de l'étape 3 : `particle` 2, `probe` 4, `planet_family` 9, `planet` 7. Les tables partent de cette mesure ; la colonne « après l'étape 2 » du brouillon est retirée.
- **H-2-8 (sites non touchés) — tenue.** `pico8_particle.c` et `pico8_probe.c` sont inchangés depuis 021cfa3. `pico8_planet_family.c` et `pico8_planet_family_trade.c` n'ont gagné que `.rendre`, et `pico8_planet.c` que `.rendre`, `lq_place` et le `ne` de `jo:hk` en zone frame. Les lignes de l'inventaire sont relevées de nouveau.
- **H-2-9 (références remises à nil) — tenue, élargie.** `oublier` (`pico8_cart.c`) vise toute entité rendue, quelle que soit sa sorte : `ok.o_`, `ok.gj`, `kc`, `ju.ed`. Il fait aussi `pico8_economy_dk_quitte`, qui repointe `dk` sur une copie statique. Cette référence manquait à la PRD de l'étape 2 et a été trouvée par un use-after-free ASan. Le plan en a tiré l'inventaire mécanique du §4 (ecbf87c), fait dans la section « Inventaire des pointeurs qui survivent à la frame » : aucune référence à ajouter à `oublier`.
- **H-2-10 (`fv`) — tenue.** Ni `g_` ni `fv` n'ont `rendre` : l'étape 3 les crée. Le *Reçu de l'étape 2* sous l'entrée 4 range la struct `fv` à l'étape 4, ce qui confirme le partage.
- **H-2-11 (« Reçu de l'étape 2 » sous l'entrée 3) — tenue.** La ligne dit : file de sortie (entités ou arènes), `rendre`, arènes, poignée sur zone à places et son accesseur prêts ; en-tête d'arène de 96 o par place. Elle est recopiée en tête de cette PRD.

Les autres leçons de l'étape 2 sont reprises dans les décisions :

- tests qui enchaînent `jl()` sans frame : ils doivent vider la file (morceau 2, Testing Decisions) ;
- nombres de places justifiés par la pire frame lue, pas par un pic d'e2e (table des tailles) ;
- TU « le plus gros tient dans sa place » par sorte (Testing Decisions), parce que `js` et `mh` sont restés sans marge à l'étape 2.

## Further Notes

- **Pas d'humain disponible, pas d'outil `Agent` dans cette session.** Les points que `to-prd` fait valider par un humain ont été tranchés en appliquant moi-même le skill `guideline` :
  - **Hypothèses explicites** : la section « Vérifié contre l'étape 2 », l'inventaire mécanique des pointeurs (§4), et les preuves par lecture marquées « se vérifie en 3a » (particule sans autre lecteur).
  - **Simplicité** : aucun outil nouveau quand l'étape 2 en fournit un (file, `rendre`, arène, poignée sur zone à places, TU de places vivantes). Un seul ajout partagé, la capacité réservée de `pico8_ptrs`, sans quoi une liste `qy` n'a pas de taille fixe. Deux sortes de liste seulement (`qy`, `qy_fv`), parce que l'aide fait vivre dix fois plus de particules que les autres émetteurs.
  - **Changements chirurgicaux** : aucun objet de partie converti. `jo:pq` gagne une variante à zone passée, et sa forme actuelle délègue sans changer ses appelants.
  - **Critères vérifiables** :
    - comptes du cliquet par morceau ;
    - `*.sha256` inchangés et `sanitize-net` ;
    - un invariant par frame sur les 26 e2e ;
    - quatre relevés de comptes, et le maximum par sorte sur tous les e2e ;
    - un test « le plus gros tient » par sorte.
- **Pourquoi une particule meurt sur-le-champ, et l'éclat par la file.** Une particule n'a qu'un propriétaire, sa liste, et personne ne la relit après `del`. L'éclat est une entité : `dg` écrit son `t` après `he`, et la frame en cours peut encore le parcourir. C'est le cas que la file de l'étape 2 résout.
- **Pourquoi l'égalité brute du §3 ne suffit pas pour `particule`.** Un émetteur actif fait varier le compte d'une frame à l'autre sans rien perdre : les naissances sont tirées par `rnd`, et les particules meurent au fil de `l`. L'invariant `particule` = Σ `#qy` est plus fort que l'égalité à deux frames, car il casse à la frame même d'une fuite. Les sortes `qy`, `qy_fv` et `ga` suivent la règle du §3 telle quelle. La note « Découvert » le reprend.
- **Risque de budget.** Le poste le plus lourd est la sorte `particule` (66 Ko). Si un e2e ou une partie la remplit :
  - on lit d'abord une particule jamais rendue (l'invariant est-il cassé ?) ;
  - puis on relève le nombre de places : passer de 2048 à 3072 places coûterait 33 Ko, dans la marge de 49 Ko, mais en la mangeant presque entière pour l'étape 4. Un tel relèvement se signale dans le message de commit.

  `qy_fv` à une seule place n'a aucune marge de nombre : la lecture dit qu'une seule aide vit à la fois. Un `pico8_fatal` sur elle désigne donc d'abord une aide jamais rendue, ou un TU qui ne vide pas la file.
- À la fin de l'étape, la note « Découvert » sous l'entrée 3 du plan porte au minimum :
  - les sites classés autrement que dans le §2, et pourquoi (structs `g_`, `fv`) ;
  - les tailles et nombres de places retenus par sorte, et les capacités réservées des listes ;
  - chaque `pico8_fatal` rencontré, lu comme « place jamais rendue » ou « limite trop basse » ;
  - `PICO8_ZONE_PARTICLES_BYTES` et la somme des zones ;
  - par sorte, le maximum relevé sur les 26 e2e et la limite retenue ;
  - les références ajoutées à `oublier`, s'il y en a, comme trous de l'inventaire du §4 ;
  - les comptes du cliquet laissés, fichier par fichier, et leur total ;
  - les écarts entre les prévisions de cette PRD et la mesure.
