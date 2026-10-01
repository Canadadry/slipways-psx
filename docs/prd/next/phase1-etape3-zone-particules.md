---
title: "Plan 4, étape 3 — Zone particules : particules, listes qy et éclats ga rendent leur place"
description: "Les particules faites par `g_:mm` et `ga:mm`, les listes `qy` faites par `f_` et les éclats `ga` sont pris à la libc ou à la transitoire et jamais rendus (TROU-7) : l'étape 3 leur donne d'abord un point de libération, sur la transitoire (3a), puis les range dans des zones à places de taille fixe (3b)."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. Le texte de l'étape, recopié du plan (relu à `HEAD` 021cfa3) :

> 3. **Zone particules.**
>
>    Ces deux étapes créent un point de libération qui n'existe pas aujourd'hui (TROU-7 : panneaux, messages et particules fuient). Elles ne sont finies que si chaque sorte convertie rend sa place, ce qui se montre par le remplissage du §3 : après une session scriptée, le nombre de places vivantes revient à son niveau de départ, et il ne monte pas pendant une phase sans action. Dans une zone à places, un `pico8_fatal` est d'abord lu comme une place jamais rendue ; la limite n'est relevée qu'une fois ce cas écarté, et la note « Découvert » dit lequel des deux c'était.
>
>    Chacune se fait en deux temps, chacun sa PRD ou son lot de commits :
>    - **a. Points de libération, sur la transitoire.** Chaque sorte de l'étape (et chaque champ réaffecté qui lui revient, TROU-7) gagne sa libération, la transitoire encore derrière : sous ASan, la quarantaine de la libc attrape tout usage après libération. Le critère des places vivantes est montré ici, sur un compte de vivants par sorte.
>    - **b. Conversion.** Les mêmes sites passent à la zone à places, sans ajouter ni déplacer de point de libération : un `sanitize-net` rouge en b vient de la zone, pas d'une libération manquante.

Cette PRD ne traite que l'étape 3. Les règles qui la cadrent sont celles du plan :

- §2 : la zone particules contient « `g_:mm`, `qy`, éclats `ga` » ; elle a un « nombre maximum de particules vivantes ; place rendue à leur mort » ;
- §3 : limites au doigt mouillé ; somme des zones ≤ 1,5 Mo, et si un relèvement la dépasse, l'étape s'arrête et le signale (zone, limite voulue, somme actuelle) sans rogner une autre zone ; limite atteinte = `pico8_fatal` ; invariant de chaque commit (`make test-all`, `make sanitize-net`, `*.sha256` identiques) ; empoisonnement ASan d'une place rendue ; contrôle « Croissance par frame » (remplissage égal à deux frames d'un e2e séparées par une phase sans action ; une hausse se corrige à l'étape qui l'a fait entrer, pas en relevant la limite) ;
- §4 : un site douteux va dans la zone la plus longue qui le couvre ; ASan ne fait que rétrograder ; il ne voit ni un pointeur gardé à travers deux remises à zéro, ni une place rendue un tour complet plus tôt ;
- §5 : fin d'étape = chaque site de l'inventaire converti (vérifié sur la source), `alloc-guard` à égalité stricte, aucun compte n'a monté ; la baisse prévue n'est qu'une estimation.

**L'étape 2 n'est pas finie** au moment où cette PRD s'écrit : elle est en cours, d'après `docs/prd/triage/phase1-etape2-zone-interface.md`. Tout ce que cette PRD suppose de son résultat est écrit en hypothèse H-2-n (section « Hypothèses sur l'étape 2 »), à revérifier avant de commencer.

**Ce qui se passe aujourd'hui.** Aucune particule n'est jamais rendue :

- `ez` ajoute à `self.qy` une particule `{p,v,a,l}` faite par `self:mm()` : `g_:mm` (`pico8_g__mm`, pour `g_`, `fv`, `hj`, `pm`, `mx`) ou `ga:mm` (`pico8_ga_mm`). Chacune est un `malloc` ;
- `ei` la retire de `qy` quand `l <= 1` (`pico8_ptrs_del`), et l'abandonne ;
- `f_` fait la liste `qy` d'un émetteur (`malloc` de la struct, tableau sur la transitoire) ; elle n'est jamais rendue, ni elle ni les particules qu'elle tient encore quand son hôte meurt (sonde `g_` posée, écran d'aide `fv` quitté, éclat `ga` éteint, planète abandonnée par `jl()`) ;
- l'éclat `ga` lui-même (struct, son `oe`, la boîte `ra` que `pm:pq` lui fait) n'est jamais rendu après son `p_`.

Le débit est fort : l'écran titre (`fv`, `fc=5`) fait 5 particules par frame ; un éclat en fait 140 d'un coup ; une sonde ou un protostar une par frame. Une partie longue croît donc sans borne en mémoire de particules, ce que la PSX ne peut pas tenir.

## Solution

L'étape 3 donne à chaque **sorte de particules** une place dans une zone à places de taille fixe, et un **point de libération** qui rend cette place à la mort de l'objet. Elle reprend les outils de l'étape 2 (file de sortie, membre d'archétype, place-arène, module de zone à sortes, compte de vivants, écriture du remplissage par le rejeu : H-2-1 à H-2-6) au lieu d'en inventer d'autres. Elle se fait en deux lots de commits, dans l'ordre imposé par le plan.

**3a — Points de libération, sur la transitoire.**

- **Une particule meurt dans `ei`.** Quand `ei` la retire de `qy` (`l <= 1`), il rend aussi sa place, sur-le-champ : la liste est sa seule propriétaire.
- **Une liste `qy` meurt avec son hôte.** Quand un émetteur (`g_`, `fv`, `hj`, `pm`, `mx`, `ga`) sort par la file de sortie de l'étape 2 (retiré par `he`, ou abandonné par `jl()`), son membre d'archétype rend chaque particule encore dans `qy`, puis la liste.
- **Un éclat `ga` meurt quand son `p_` le fait sortir.** Sa place (struct, `oe`, boîte `ra`) est rendue au vidage de la file, avec sa liste et sa rangée d'interface (étape 2).
- **Un compte de vivants par sorte**, lu par l'accesseur de remplissage : il montre le critère du plan sur `pause_restart`, `tech_geoharvesting_burst`, `message_box_hold`, et un invariant plus fort sur tous les e2e (voir Testing Decisions).

**3b — Conversion.**

- Le module de zone particules possède une zone à places par sorte, de taille et de nombre fixés à la compilation, dont la somme est vérifiée contre `PICO8_ZONE_PARTICLES_BYTES`.
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
7. En tant que développeur, je veux que `f_` prenne la liste `qy` dans une place, avec une capacité réservée d'un coup, pour que la liste ne s'agrandisse jamais par copie et que son débordement sorte par un `pico8_fatal` qui la nomme.
8. En tant que développeur, je veux que la liste de l'écran d'aide `fv` prenne une place plus grande que celle des autres émetteurs, pour que ses ~600 particules vivantes (5 par frame pendant ~115 frames) ne fixent pas la taille de toutes les listes.
9. En tant que développeur, je veux qu'un `f_` appelé sur un hôte qui a déjà une liste rende l'ancienne, pour reproduire la réaffectation `self.qy={}` du cart sans abandonner de place (TROU-7, champ réaffecté).
10. En tant que développeur, je veux que la sortie d'une sonde `g_` (posée par `d_`, ou abandonnée par `jl()`) rende ses particules et sa liste, pour que chaque sonde lancée ne laisse rien en zone particules.
11. En tant que développeur, je veux que la sortie de l'écran d'aide `fv` (au `jl()` de `eu`) rende ses particules et sa liste, pour qu'une nouvelle partie démarre sans les ~600 particules du titre.
12. En tant que développeur, je veux que la sortie d'une planète émettrice (`hj`, `pm`, `mx`) rende ses particules et sa liste en plus de sa rangée d'interface (étape 2), pour que « restart » et une nouvelle partie rendent toute la zone particules.
13. En tant que développeur, je veux que la sortie d'un éclat `ga` rende ses particules restantes, sa liste, puis sa place, pour qu'un éclat éteint ne laisse rien.
14. En tant que développeur, je veux qu'un éclat `ga` prenne une place qui contient sa struct, son `oe` et la boîte `ra` que `pm:pq` lui donne, pour que le rendre soit un seul appel.
15. En tant que développeur, je veux que `pm:pq` passe à l'éclat une **valeur** de position, recopiée dans la place de l'éclat, au lieu d'allouer une boîte qu'il abandonne.
16. En tant que développeur, je veux qu'un éclat né par `jo:hu` (« harvest ») garde le `ra` et le `p` empruntés à la planète qu'il remplace, comme dans le cart, pour ne changer ni un pixel ni un objet de partie.
17. En tant que développeur, je veux que `jo:pq` puisse prendre la zone de son `oe` en paramètre, pour que l'éclat range son `oe` dans sa place sans que les autres planètes changent de zone avant l'étape 4.
18. En tant que développeur, je veux que la libération des particules et des éclats passe par la file de sortie et le membre d'archétype de l'étape 2, pour qu'il n'y ait qu'un seul chemin de sortie et que l'éclat, que `ui:jz`, `ok.o_` ou `ok.gj` peuvent encore désigner dans la frame, ne soit rendu qu'en tête de la suivante.
19. En tant que développeur, je veux que le membre d'archétype d'une classe émettrice de la famille `jo` fasse, dans l'ordre, ce que l'étape 2 y a mis (rendre `jo.lq`) puis ce que l'étape 3 y ajoute, pour qu'aucune des deux étapes n'écrase l'autre.
20. En tant que développeur, je veux qu'un objet de partie émetteur (planète, sonde, aide) ne soit **pas** rendu par l'étape 3 — seulement ce qu'il tient en zone particules —, pour que l'étape 4 garde la main sur les objets de partie.
21. En tant que mainteneur, je veux un compte de places vivantes par sorte (`particule`, `qy`, `qy_fv`, `ga`), lisible par un accesseur, pour montrer en 3a comme en 3b qu'une session revient à son niveau de départ.
22. En tant que mainteneur, je veux un TU de rejeu qui vérifie, à chaque frame de chaque e2e de `tests/e2e/`, que le nombre de places `particule` vivantes est égal à la somme des `#qy` des émetteurs inscrits (plus ceux en file), pour prouver frame par frame qu'aucune particule n'est perdue.
23. En tant que mainteneur, je veux un TU qui rejoue `pause_restart` et compare les comptes `qy`, `qy_fv` et `ga` à l'écran titre avant la partie et après « restart », et le compte `particule` à ce que l'aide a émis depuis son `_init`, pour prouver que `jl()` rend tout ce qu'il abandonne.
24. En tant que mainteneur, je veux un TU qui rejoue `tech_geoharvesting_burst` et montre que les comptes `ga` et `qy` reviennent, après l'extinction de l'éclat, au niveau d'avant le « harvest », pour prouver que l'éclat rend tout.
25. En tant que mainteneur, je veux un TU qui rejoue `message_box_hold` et compare tous les comptes aux frames 45 et 90 de sa phase sans action, pour satisfaire le contrôle « Croissance par frame » du §3.
26. En tant que mainteneur, je veux un TU qui laisse tourner l'écran titre sans entrée bien au-delà de la vie d'une particule de l'aide, et montre que le compte `particule` reste borné et que `qy_fv` reste à 1, pour prouver qu'un émetteur actif atteint un régime sans croissance.
27. En tant que mainteneur, je veux que le rejeu écrive le remplissage des sortes de particules en fin de session, avec le même mécanisme que l'étape 2, pour lire un taux sans écrire de test.
28. En tant que développeur qui chasse un accès hors durée de vie, je veux qu'en 3a la place rendue soit un bloc libéré par la transitoire, pour que la quarantaine d'ASan attrape toute lecture d'une particule morte.
29. En tant que développeur, je veux qu'en 3b une place rendue soit empoisonnée par la zone à places, pour que `make sanitize-net` attrape une lecture tardive, dans la limite d'un tour complet du curseur.
30. En tant que développeur, je veux qu'un `pico8_fatal` « zone particules <sorte> pleine » soit d'abord lu comme une place jamais rendue, pour ne relever un nombre de places qu'une fois ce cas écarté.
31. En tant que développeur, je veux qu'un `pico8_fatal` « zone place qy pleine » (ou `qy_fv`) dise qu'une liste a dépassé sa capacité réservée, pour relever cette capacité en un chiffre.
32. En tant que mainteneur du budget, je veux une vérification à la compilation que la somme des zones à places de particules tient dans `PICO8_ZONE_PARTICLES_BYTES`.
33. En tant que mainteneur du budget, je veux que tout relèvement de `PICO8_ZONE_PARTICLES_BYTES` au-delà de ce que permettent les 1,5 Mo arrête l'étape et le signale (zone, limite voulue, somme actuelle), pour ne jamais rogner une autre zone.
34. En tant que gardien du cliquet, je veux qu'aucun commit de l'étape ne fasse monter un compte, pour respecter « plus aucun compte ne monte après 0b ».
35. En tant que gardien du cliquet, je veux qu'en fin d'étape `pico8_particle.c` ait quitté la liste d'exceptions, et que `probe`, `planet_family` et `planet` aient baissé, pour que le verrou interdise toute nouvelle allocation de particule.
36. En tant que gardien de l'invariant, je veux que les `*.sha256` restent identiques à chaque commit, pour prouver que rendre une particule ne change pas un pixel ni un tirage `rnd`.
37. En tant que développeur des TU existants, je veux que les tests qui glissent une particule statique dans `qy` la prennent désormais à la poignée de la sorte `particule`, pour que `ei` puisse la rendre sans rendre de la mémoire statique.
38. En tant que lecteur du plan, je veux une note « Découvert » qui dise les sites classés autrement que dans le §2, les tailles et nombres de places retenus et pourquoi, chaque `pico8_fatal` rencontré, et les comptes laissés.
39. En tant que développeur de l'étape 4, je veux des lignes « *Reçu de l'étape 3* » sous l'entrée 4 (struct `g_`, struct `fv`, `oe` des planètes par `jo:pq` à zone passée), pour savoir ce que l'étape 3 m'a laissé et pourquoi.
40. En tant que mainteneur des commentaires, je veux que les `/* TROU-7 */` des fuites refermées (`f_`, `g_:mm`) soient retirés et que les contrats « nothing frees it » de `pico8_particle.h` disent désormais qui rend quoi, pour que les commentaires restent vrais.

## Implementation Decisions

### Ce qu'est une sorte de particules

| Sorte | Objet | Créé par | Meurt (point de libération) |
|---|---|---|---|
| `particule` | un élément de `qy`, `pico8_qy_t` (32 octets) | `ez` → `self:mm()` : `g_:mm` (`g_`, `fv`, `hj`, `pm`, `mx`), `ga:mm` | `ei`, quand `l <= 1` : rendue juste après `pico8_ptrs_del` ; ou avec sa liste |
| `qy` | la liste `qy` d'un émetteur autre que `fv` (en-tête `pico8_ptrs_t` et tableau réservé) | `f_` : `g_.pq`, `hj`/`pm`/`mx` à leur construction, `ga:pq` | sortie de l'hôte par la file de sortie (`he` ou `jl()`) ; ou nouveau `f_` sur le même hôte |
| `qy_fv` | la liste `qy` de l'écran d'aide | `f_` dans la construction de `fv` | sortie de `fv` (le `jl()` de `eu`, ou `he`) |
| `ga` | l'éclat : struct `pico8_jo_t`, son `oe`, et sa boîte `ra` quand `pm:pq` la fournit | `pm:pq` (protostar), `jo:hu` (`kv["ga"]`, « harvest ») | `p_` posé par `ga:kj` quand la troisième place de `qy` est vide → `dg` → `he` → file ; ou `jl()` |

- **Hôtes émetteurs** (classes qui portent un `emitter`) : `g_`, `fv`, `hj`, `pm`, `mx`, `ga`. Ce sont eux qui gagnent (ou complètent) un membre d'archétype.
- **Ce que l'étape ne rend pas.** La struct d'une sonde `g_`, celle de `fv`, et les planètes `hj`/`pm`/`mx` ne sont pas des particules (§2). Seul ce qu'elles tiennent en zone particules est rendu. Les structs `g_` et `fv` vont à l'étape 4 (voir « Sites écartés »).

### Mort d'une particule : dans `ei`, sur-le-champ

- `ei` retire la particule de `qy` puis rend sa place à la poignée `particule`. La particule n'est plus lue ensuite : la branche `else` qui la dessine n'est pas prise.
- **Pourquoi pas par la file.** La liste est la seule propriétaire d'une particule. Les seuls autres lecteurs sont `ga:kj`, qui lit `qy[1..3]` **avant** d'appeler `ei`, et les TU. Rendre sur-le-champ évite des centaines d'entrées de file par frame. Cette preuve est une lecture ; elle se vérifie en 3a par la quarantaine d'ASan.
- **Piège à commenter (`ga:kj`).** Le cart fait `self.p_ = not pt` après `ei(self)`. En C, `pt` peut viser une particule que `ei` vient de rendre. On garde un booléen calculé avant `ei` (« la troisième place était occupée ») au lieu du pointeur : la valeur de `p_` est la même, et aucune place rendue n'est comparée. ASan ne verrait pas cette comparaison (§4).
- **Ordre des tirages inchangé.** Rendre une place ne tire aucun `rnd` ; l'ordre de `ez` et de `mm` ne bouge pas. Le saut d'un élément après `del` (boucle `pairs`, TROU-14) est conservé tel quel.

### Liste `qy` : une place-arène à capacité réservée

- `f_` prend une **place-arène** (primitive de l'étape 2, H-2-3) de la sorte `qy` ou `qy_fv`. Il y range l'en-tête `pico8_ptrs_t` et un tableau d'éléments réservé **en une fois** à la capacité de la sorte.
- `pico8_ptrs` gagne une initialisation à capacité réservée (nom laissé à l'implémenteur). Au-delà de cette capacité, l'agrandissement passe par la poignée de l'arène, qui est pleine : `pico8_fatal` « zone place qy pleine, limite N octets ». C'est le signal de relever la capacité de la sorte. Sans réserve, les doublements successifs (4, 8, …, 256) laisseraient dans l'arène des copies mortes aussi grosses que le tableau final.
- **Choix de la sorte par l'hôte** : `fv` prend `qy_fv`, tous les autres `qy`. La forme du choix (paramètre de `f_`, variante, champ de l'émetteur) est laissée à l'implémenteur ; aucune donnée générée du cart ne change.
- **`f_` sur un hôte qui a déjà une liste** : il rend ses particules et l'ancienne place avant d'en prendre une nouvelle. Aujourd'hui aucun chemin n'appelle `f_` deux fois sur le même hôte ; la règle couvre la réaffectation `self.qy={}` du cart (TROU-7, champ réaffecté) au lieu d'abandonner la place.
- **Rendre une liste** = rendre chaque particule encore dans `qy`, puis la place-arène, puis remettre `emitter->qy` à NULL. Une seule fonction de `pico8_particle`, appelée par les membres d'archétype.

### Éclat `ga` : une place-arène

- `pico8_ga_struct_new` prend une place-arène de la sorte `ga`. Il y range sa struct, son `oe` et, quand elle est fournie par valeur, sa boîte `ra`.
- **`jo:pq` à zone passée.** `pico8_jo_pq` reçoit une variante qui prend la zone de `oe` en paramètre, sur le modèle des fonctions de 0b. L'éclat passe la poignée de sa place ; toutes les autres planètes passent encore la transitoire, par la forme actuelle de `jo:pq` qui délègue à la variante. Le compte de `pico8_planet.c` baisse d'une référence (le `malloc` de `oe` devient une allocation par la poignée) et aucun autre fichier ne monte.
- **Position de l'éclat.**
  - `pm:pq` passe une **valeur** (`ra + ng`) par les clés de `ga` ; l'éclat la recopie dans une boîte de sa place. Personne d'autre ne tenait la boîte que `pm:pq` allouait : rien ne change à l'écran.
  - Par `jo:hu` (`kv["ga"]({p=…,ra=…,id=…})`), l'éclat **emprunte** le `p`, le `ra` et l'`id` de la planète remplacée, comme dans le cart. Ce sont des objets de partie (étape 4), jamais rendus par l'étape 3.
- `by` reste la liste partagée `g_ga_by_shared` : statique, jamais rendue.
- **Membre d'archétype de `ga`**, dans l'ordre : ce que l'étape 2 y met (rendre la place de `jo.lq`, H-2-2), puis rendre la liste `qy` et ses particules, puis rendre la place de l'éclat. La place n'est rendue qu'au vidage de la file, en tête du `pico8__update60()` suivant : `ga:kj` pose `p_` pendant le `draw`, `dg` fait `he` à l'`update` suivant, et `ui:jz`, `ok.o_` ou `ok.gj` peuvent encore désigner l'éclat survolé dans cette frame (`ga` a un `hd`). La remise à nil de ces références au vidage est celle de l'étape 2 (H-2-9).

### Membres d'archétype des émetteurs

| Classe | Membre après l'étape 3 |
|---|---|
| `g_` | rendre `qy` (nouveau membre) |
| `fv` | rendre `qy` (nouveau membre) |
| `hj`, `pm`, `mx` | rendre `jo.lq` (étape 2), puis rendre `qy` |
| `ga` | rendre `jo.lq` (étape 2), rendre `qy`, rendre la place `ga` |

- Les autres classes de la famille `jo` gardent le membre de l'étape 2 sans changement.
- La file de sortie (H-2-1) les fait entrer par `he` (s'ils étaient dans `hv`) et par `jl()`. L'étape 3 n'ajoute aucune entrée de file.
- La file de sortie de l'étape 2 a une capacité de 256 entrées (H-2-1). L'étape 3 y ajoute au plus quelques entités par frame (`g_`, `fv`, `ga`), et les planètes émettrices y entraient déjà par l'étape 2 : pas de relèvement prévu.

### Module de zone particules et poignées des sortes

Il suit la forme du module de zone interface de l'étape 2 (H-2-4), sans en partager le code s'il n'y a rien à partager :

- **Module du cœur**, hors liste d'exceptions : il ne référence ni la libc ni `pico8_zone_transitoire`. Pour chaque sorte, il expose une poignée `const pico8_zone_choisie_t *` (une place entière ; pour `particule`, une place de 32 octets, sans arène) et le nombre de places vivantes.
- **3a : adossé à la transitoire.** Le module compte les places prises et rendues, et délègue à une poignée d'adossement. Cette poignée lui est donnée par `pico8_particle.c`, qui référence déjà la transitoire, dans le commit qui retire déjà le `malloc` et la référence à la transitoire de `f_` (2 → 1). Aucun compte ne monte.
- **3b : zones à places.** Un `PICO8_SLOT_ZONE_STORAGE` par sorte. La poignée devient la poignée sur zone à places de l'étape 2 (H-2-4) : `alloc(n)` exige `n` ≤ taille de place (`assert` porteur de message), `grow` est refusé, `release` rend la place. L'adossement disparaît : `pico8_particle.c` sort de la liste. Aucun point de libération ne bouge.
- **Messages de `pico8_fatal`.** « zone particules <sorte> pleine, limite N places » : place jamais rendue d'abord, limite trop basse ensuite. « zone place qy pleine, limite N octets » (arène de liste ou d'éclat) : capacité réservée ou place trop petite.

### Tailles et nombres de places (au doigt mouillé)

Ces valeurs ne sont pas mesurées (§3). `sizeof` relevés à `HEAD` 021cfa3 (x86-64, `-O0`) : `pico8_qy_t` 32, `pico8_ptrs_t` 24, `pico8_ents_t` 24, `pico8_jo_t` 424, `pico8_g__t` 256, `pico8_vec_t` 8. L'en-tête d'une arène (zone montante privée et sa poignée) est celui de l'étape 2, estimé à moins de 100 octets (H-2-3).

Vies, lues dans le cart (`l` perd 0,14 par frame où l'émetteur est à l'écran ; mort à `l <= 1`) :

- `fv` : `fc=5`, `ki=17` → ~115 frames, **~575 vivantes** en régime, un peu plus par les sauts de TROU-14 ;
- `g_`, `pm` : `fc=1`, `ki=9` → ~58 vivantes ;
- `hj` : `fc=0.5` → ~29 ; `mx` : `fc=0.3` → ~17 ;
- `ga` : 140 à la naissance (`ez(self,140)`), `l` dans [6,12[ → au plus ~79 frames, `fc=0`.

Un émetteur hors de l'écran (`fa < 10` et pas `gd`) ne fait ni naître ni vieillir ses particules : son compte est figé, il ne croît pas.

| Sorte | Place | Nombre | Total | Raisonnement |
|---|---|---|---|---|
| `particule` | 32 o | 2048 | 64 Ko | titre : ~600 (aide seule) ; en partie : une sonde (~60) + ≤ 16 planètes émettrices (~30 à ~60 chacune) + 2–3 éclats (140) ≈ 1000 ; le double en marge |
| `qy` | 1,5 Ko | 24 | 36 Ko | en-tête d'arène + `pico8_ptrs_t` + 160 éléments (8 o) : couvre un éclat (140) et une sonde ou un `pm` (~60, avec les sauts) ; vivantes : sonde + planètes émettrices + éclats + entrées de file |
| `qy_fv` | 6,5 Ko | 2 | 13 Ko | 768 éléments ; l'aide courante et celle d'un `_init` refait dans la frame (« restart ») en file |
| `ga` | 768 o | 8 | 6 Ko | en-tête d'arène + struct 424 + `oe` 24 + boîte 8 + marge pour les éléments de `oe` ; vivants : 2–3 éclats + file |
| **Total** | | | **≈ 119 Ko** | |

- **Le total tient dans `PICO8_ZONE_PARTICLES_BYTES` (128 Ko) sans relèvement.** Somme prévue des zones après l'étape 2 : frame 256 + interface 320 (H-2-6) + particules 128 + partie 768 = **1472 Ko sur 1536**. L'étape 3 ne la change pas.
- **Marge pour un relèvement.** La zone particules peut monter jusqu'à 128 + 64 = **192 Ko** sans dépasser 1,5 Mo (si H-2-6 tient). Au-delà, l'étape s'arrête et signale à l'humain : zone particules, limite voulue, somme actuelle (§3). Elle ne rogne ni la zone partie ni l'interface. Ce relèvement consommerait aussi la marge dont l'étape 4 dispose : le message de commit le dit.
- Une vérification à la compilation, comme celle de `pico8_zone_sizes.h`, exige que la somme des zones à places de particules (tailles réelles, hors ASan) tienne dans `PICO8_ZONE_PARTICLES_BYTES`.
- **Lecture d'un `pico8_fatal`** : voir « Module ». La note « Découvert » dit, pour chacun, lequel des deux cas c'était.

### Inventaire des sites — convertis à l'étape 3

Relevé à `HEAD` 021cfa3 (= 522364f pour `src/`, l'étape 1 finie) sur une copie de `HEAD` : `make alloc-guard` y donne « 61 objets, 32 allouent, 147 appels », égal à `tools/alloc_guard_exceptions.txt`. Les relocations ont été rattachées à leur fonction et à leur ligne par `objdump -r` et `addr2line` sur les objets `-O0 -g` d'`alloc-guard`. L'étape 2 ne touche aucun de ces sites (H-2-8) ; seuls les numéros de ligne peuvent bouger.

« T » désigne une référence à `pico8_zone_transitoire`, « libc » un appel `malloc`/`calloc`/`realloc`/`free`.

| Fichier | Fonction (ligne) | Objet | Devient | Baisse |
|---|---|---|---|---|
| `pico8_particle.c` | `pico8_f_` (37, 38) | struct `qy` (libc), tableau (T) | place-arène `qy`/`qy_fv`, capacité réservée | 2 (1 libc + 1 T) |
| | (adossement du module, 3a) | référence ajoutée en 3a, compensée par les deux ci-dessus (2 → 1) | retirée en 3b | 1 T en 3b |
| `pico8_probe.c` | `pico8_g__mm` (354) | particule | place `particule` | 1 libc |
| `pico8_planet_family.c` | `pico8_ga_mm` (374) | particule | place `particule` | 1 libc |
| | `pico8_ga_struct_new` (452) | struct `ga` | place-arène `ga` | 1 libc |
| | `pico8_pm_pq` (273) | boîte `ra` de l'éclat | valeur recopiée dans la place `ga` | 1 libc |
| `pico8_planet.c` | `pico8_jo_pq` (195, 196) | `oe` (libc + T) | variante à zone passée ; la forme actuelle garde une référence T pour les planètes | 1 (2 → 1) |

### Baisse attendue du cliquet, par fichier

| Fichier | À `HEAD` 021cfa3 (mesuré) | Après l'étape 2 (H-2-7, prévu) | Après 3a | Après 3b |
|---|---|---|---|---|
| `pico8_particle.c` | 2 | 2 | 1 | sort |
| `pico8_probe.c` | 4 | 4 | 3 | 3 |
| `pico8_planet_family.c` | 9 | 9 | 6 | 6 |
| `pico8_planet.c` | 8 | 7 | 6 | 6 |
| **Liste** | 32 fichiers, 147 appels (mesuré) | 25 fichiers, 91 (prévu par l'étape 2) | 25 fichiers, 85 | 24 fichiers, 84 |

Ces chiffres sont des **prévisions** : le verrou compte des relocations, qui dépendent du compilateur, et la colonne « après l'étape 2 » est elle-même une prévision (H-2-7). Le point de départ réel est la liste mesurée quand l'étape 3 commence. La règle de fin est celle du §5 :

- chaque site de l'inventaire est converti, vérifié sur la source : la fonction ne référence plus la transitoire ni la libc pour ces objets ;
- `alloc-guard` est à égalité stricte sur les comptes mesurés ;
- aucun compte n'a monté.

L'écart se note dans le message de commit et dans « Découvert ». Il n'est jamais compensé ailleurs.

### Sites écartés de l'étape 3 (et où ils vont)

Ils sont listés pour la note « Découvert » et pour les lignes « *Reçu de l'étape 3* » sous l'entrée 4.

- **La struct d'une sonde `g_`** (`pico8_g__struct_new`, `pico8_probe.c` 409). Le §2 range `g_:mm` en zone particules, pas la sonde. `bg:ldown` la rend comme objet tenu (`out.held`) à l'interface, qui la garde d'une frame à l'autre et lui envoie `hq` (`bv`). C'est un objet relu au-delà de sa sortie : zone la plus longue (§4), étape 4. Une ligne « Reçu » le signale : la sonde meurt par `he` et la file de l'étape 2 sait déjà la faire sortir ; en zone partie, chaque lancer coûterait une struct jusqu'à la nouvelle partie (croissance par action, pas par tick).
- **La struct de `fv`** (`pico8_help.c` 116). L'étape 2 l'a renvoyée « aux étapes 3 et 4 ». Elle vit de `_init` au `jl()` suivant, comme les objets de partie : étape 4. L'étape 3 ne prend que sa liste et ses particules.
- **Le `oe` des planètes** (`pico8_jo_pq` à travers sa forme actuelle). Il reste sur la transitoire ; l'étape 4 n'a qu'à passer la zone partie à la variante. Ligne « Reçu ».
- **`p` de `pico8_bv`** (`pico8_probe.c` 162) et `fm_free` (449) : déjà reçus par l'étape 4 (*Reçu de l'étape 1*), non touchés.
- **Les structs `hj`, `pm`, `mx`** : planètes, étape 4.

## Implementation Chunks

Chaque morceau est un commit (ou une courte suite) qui respecte l'invariant du §3 et remet `alloc-guard` à égalité stricte. Son message dit les baisses mesurées par fichier et leur écart à la prévision. **Aucun morceau ne fait monter un compte.** Un site dont la conversion fait planter `make sanitize-net` n'est pas commité ; il est corrigé (sortie manquante, référence oubliée) ou noté pour « Découvert ».

Les tables partent de la liste prévue après l'étape 2 (H-2-7) ; le commit écrit la liste mesurée et note l'écart.

**3a — Points de libération, sur la transitoire**

1. **Socle.**
   - Le module de zone particules, ses quatre sortes, leurs tailles de place, leurs compteurs ; encore sans adossement ni utilisateur.
   - L'initialisation à capacité réservée de `pico8_ptrs`, et ses TU.
   - La fonction qui rend une liste `qy` (particules, puis place).

   Comptes inchangés.
2. **Listes `qy`, et adossement du module.**
   - `f_` prend une place-arène `qy` ou `qy_fv` à capacité réservée ; un second `f_` rend l'ancienne.
   - Les membres d'archétype de `g_` et `fv`, et le complément de ceux de `hj`, `pm`, `mx`, `ga` (après ce que l'étape 2 y a mis) : ils rendent la liste. Les particules sont encore des `malloc` : la fonction qui rend une liste les rend par la poignée qui les a données, ce qui impose de passer `g_:mm` et `ga:mm` dans le même commit, ou de faire ce morceau et le suivant en un seul commit. L'implémenteur choisit ; aucun compte ne monte dans les deux cas.
   - L'adossement du module, dans `pico8_particle.c`, qui retire dans le même commit le `malloc` et la référence de `f_`.

   | Fichier | Avant | Après |
   |---|---|---|
   | `particle` | 2 | 1 (l'adossement) |

   Liste : 25 fichiers, 90 appels.
3. **Particules.**
   - `g_:mm` et `ga:mm` prennent leur particule à la poignée `particule`.
   - `ei` rend la particule retirée ; `ga:kj` garde un booléen au lieu du pointeur.
   - Les TU qui glissent une particule statique dans `qy` (`test_pico8_fv`, `test_pico8_ga`) la prennent à la poignée.

   | Fichier | Avant | Après |
   |---|---|---|
   | `probe` | 4 | 3 |
   | `planet_family` | 9 | 8 |

   Liste : 25 fichiers, 88 appels.
4. **Éclats `ga`.**
   - La place-arène `ga` (struct, `oe` par la variante de `jo:pq`, boîte `ra` recopiée depuis la valeur de `pm:pq`) ; l'emprunt de `jo:hu` est conservé.
   - Le membre de `ga` rend sa place en dernier.

   | Fichier | Avant | Après |
   |---|---|---|
   | `planet_family` | 8 | 6 |
   | `planet` | 7 | 6 |

   Liste : 25 fichiers, 85 appels.
5. **Preuve des places vivantes.** Les TU de rejeu (voir Testing Decisions) et l'écriture du remplissage des sortes de particules par le rejeu.
   - Si un compte ne revient pas à son niveau, ou si l'invariant `particule` = Σ `#qy` casse, c'est une sortie manquante : on la trouve avant de passer à 3b.

**3b — Conversion**

6. **Zones à places de particules.**
   - Le stockage par sorte dans le module, et la vérification de somme contre `PICO8_ZONE_PARTICLES_BYTES`.
   - `PICO8_ZONE_PARTICLES_BYTES` relevé seulement si la somme l'exige et si les 1,5 Mo le permettent (aucun relèvement prévu ; plafond permis 192 Ko si H-2-6 tient). Sinon : arrêt et signalement (§3).
   - Le retrait de l'adossement : `pico8_particle.c` sort de la liste. Liste : 24 fichiers, 84 appels.
   - Aucun point de libération n'est ajouté ni déplacé.
7. **Clôture de l'étape.**
   - La note « Découvert » sous l'entrée 3 du plan : sites classés autrement que dans le §2, tailles et nombres de places retenus (avec, pour chaque `pico8_fatal` rencontré, « place jamais rendue » ou « limite trop basse »), `PICO8_ZONE_PARTICLES_BYTES` et la somme des zones, comptes laissés et écarts à la prévision.
   - La correction du §2 si un constat le contredit.
   - Les lignes « *Reçu de l'étape 3* » sous l'entrée 4 : struct `g_`, struct `fv`, `oe` des planètes par la variante de `jo:pq`.
   - Le retrait des `/* TROU-7 */` des fuites refermées et la mise à jour des contrats de `pico8_particle.h`.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - un dessin de particule ou d'éclat ;
  - un compte de places vivantes rendu par l'accesseur ;
  - la longueur de `qy` d'un émetteur ;
  - un code de sortie, un message de `pico8_fatal`, un rapport ASan ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne regarde ni une adresse ni l'intérieur d'une arène.
- **TU de `pico8_ptrs` à capacité réservée** : on ajoute jusqu'à la capacité sans agrandissement ; un élément de plus sur une arène pleine sort en 3 avec « zone place <nom> pleine » (processus enfant).
- **TU du module de zone particules**, sur le modèle de `tests/test_pico8_zone_choisie.c` (règle `asan_objs`, `UNIT_RUNS_SPECIAL`, `ASAN_OPTIONS=detect_leaks=0`) :
  - le compte de chaque sorte suit prises et rendus ;
  - une place `particule` rendue puis lue plante (enfant) ;
  - une zone pleine sort en 3 avec « zone particules <sorte> pleine ».
- **TU par sorte**, dans les TU existants (passés en règle ASan si besoin) : `test_pico8_g_`, `test_pico8_fv`, `test_pico8_ga`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`.
  - Une particule dont `l` tombe sous 1 dans `ei` est rendue : le compte `particule` baisse d'un, et la lire ensuite plante (enfant).
  - Un émetteur qui sort (`he`, puis vidage de la file ; ou `jl()`, puis vidage) rend toutes ses particules et sa liste : les comptes `particule` et `qy` (ou `qy_fv`) reviennent à leur niveau d'avant sa naissance.
  - Un éclat qui s'éteint (`p_` par `ga:kj`, `dg`, vidage) rend sa place : le compte `ga` revient à 0.
  - Un éclat né d'un `pm` a la même position qu'aujourd'hui (`ra + ng`), et un éclat né par `jo:hu` suit le `ra` de la planète remplacée.
  - Les empreintes `fold_particle` existantes (`test_pico8_g_`, `test_pico8_ga`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`) ne changent pas : rendre une place ne tire aucun `rnd`.
- **Places vivantes, sur rejeu** : le TU de rejeu de l'étape 2 (H-2-5), qui pilote le cart frame par frame depuis un `events.json` de `tests/e2e/` comme la boucle de `main.c`, et lit les comptes **après un vidage de la file**.
  - **Invariant sur tous les e2e.** À chaque frame de chacun des 26 e2e : `particule` = Σ `#qy` des émetteurs inscrits dans `hv` ou en file ; `qy` + `qy_fv` = nombre d'émetteurs qui ont une liste ; `ga` = nombre d'éclats vivants ou en file. C'est la forme forte du critère : une particule perdue le casse à la frame même où elle l'est. Le compte `particule` varie naturellement d'une frame à l'autre (naissances `rnd`, morts) : l'égalité brute du §3 ne s'y applique qu'à travers cet invariant.
  - **`pause_restart`** : `qy_fv` = 1, `qy` = 0, `ga` = 0 avant la frame 0 (écran titre) et après la frame 11 (écran titre après « restart » en f7). `particule` est, aux deux relevés, égal à `#qy` de l'aide courante, soit 5 par `draw` depuis son `_init` (aucune particule de l'aide ne meurt avant ~115 frames). Cela montre que `jl()` rend tout ce qu'il abandonne, l'aide d'avant comprise.
  - **`tech_geoharvesting_burst`** : `ga` passe à 1 au « harvest » (f56) et revient à 0 quand l'éclat s'éteint (vers f139) ; `qy` revient alors au niveau d'avant f56. Si l'éclat n'est pas éteint à la dernière frame, on le constate et « Découvert » le dit.
  - **`message_box_hold`** (contrôle « Croissance par frame » du §3) : tous les comptes égaux aux frames 45 et 90 de la phase sans action f42–90. Aucun émetteur n'y est actif si la partie n'a ni sonde ni planète émettrice : le relevé est alors trivial, et le TU de régime ci-dessous porte la preuve sur un émetteur actif.
  - **Régime sans entrée à l'écran titre** : le cart, chargé et laissé sans entrée 400 frames (au-delà des ~115 frames de vie d'une particule de l'aide), garde `qy_fv` = 1 ; `particule` reste égal à `#qy` de l'aide et ne dépasse pas un plafond relevé entre les frames 200 et 400 (aucune hausse d'une fenêtre de 100 frames à la suivante).

  Le même TU sert en 3a (comptes adossés à la transitoire) et en 3b (zones à places).
- **Tests existants.** Ils restent verts sans changement d'attente. Seules adaptations permises : prendre à la poignée `particule` une particule glissée à la main dans `qy` (`test_pico8_fv` 484, `test_pico8_ga` 637), et ne plus vider `qy` par `qy->count = 0` sans rendre ses particules quand un test lit les comptes.
- **Verrou.** `make alloc-guard` (dans `make test`) à égalité stricte après chaque morceau. Pas de nouveau cas dans `tests/test_alloc_guard.c`.
- **Invariant.** `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. En 3a, la quarantaine de la libc attrape un usage après libération ; en 3b, l'empoisonnement des places rendues l'attrape, dans la limite d'un tour complet du curseur (§4). Le curseur de la sorte `particule` tourne vite (2048 places, ~5 à 20 prises par frame) : un tour complet prend une centaine de frames, ce qui laisse à ASan une fenêtre plus courte que pour l'interface. Ce n'est pas une raison de promouvoir un site douteux (§4).
- **Exemples dans le dépôt** : `tests/test_pico8_zone.c`, `tests/test_pico8_zone_choisie.c` (lecture périmée dans un enfant, zone pleine en 3) ; `tests/test_pico8_replay_usage.c` (enfant, statut, `stderr`) ; `src/main.c` (boucle de rejeu) ; `test_pico8_ga`, `test_pico8_g_` (TU qui bouclent `update60`/`draw` et replient les particules).

## Out of Scope

- Les structs `g_`, `fv`, `hj`, `pm`, `mx`, et le `oe` des planètes autres que l'éclat : étape 4.
- Les objets d'interface, la file de sortie, le membre d'archétype, la place-arène et le module de zone interface eux-mêmes : étape 2, que l'étape 3 réutilise.
- L'accesseur de remplissage d'une zone montante : étape 4.
- Retirer l'entrée TROU-7 de `docs/trous-de-portage.md` et des sources qui la citent encore pour d'autres sortes : étape 5. L'étape 3 ne retire que les `/* TROU-7 */` des fuites qu'elle referme.
- Changer le comportement du jeu : la boucle `pairs` qui saute un élément (TROU-14), un émetteur hors écran dont les particules sont figées, la palette d'un éclat de protoétoile (TROU-21).
- Compacter `pico8_qy_t` (`has_a` et `a` absents pour `g_`) : économie PSX (§7).
- Lancer l'application raylib.

## Hypothèses sur l'étape 2

Tirées de `docs/prd/triage/phase1-etape2-zone-interface.md` (commit 021cfa3). Chacune dit ce que l'étape 3 en fait si elle tombe.

- **H-2-1 (file de sortie).** Un module du cœur, tableau statique de capacité fixe (256), `pico8_fatal` « file de sortie pleine ». Entrée par `pico8_he` si l'entité était dans `hv` et que son archétype a le membre ; entrée par `jl()` de chaque entité encore inscrite qui a le membre, avant que `hv` ne soit vidé. Vidage en tête de `pico8__update60()`, après `pico8_zone_frame_reset()` et avant `pico8_dg()`, et à `pico8_cart_load()`. *Si elle tombe* (pas de file, ou rendu dans `he`) : l'étape 3 ne peut pas rendre un éclat dans `he` (`ui:jz`, `ok.gj` le relisent) ; elle devrait construire la file elle-même, et cette PRD repasse en brouillon.
- **H-2-2 (membre d'archétype).** `pico8_struct_class_t` a un membre, un pointeur de fonction (NULL : rien à rendre), appelé au vidage ; chaque classe de la famille `jo`, `ga`, `hj`, `pm`, `mx` compris, l'a, pour rendre `jo.lq`. *Si la forme diffère* (plusieurs membres, ou une table de rendus) : l'étape 3 ajoute son travail dans la forme retenue ; l'ordre « interface, puis `qy`, puis place `ga` » reste.
- **H-2-3 (place-arène).** Une primitive prend un bloc de taille fixe à une poignée `pico8_zone_choisie_t`, y initialise une zone montante privée et sa poignée dans l'en-tête, se rend d'un seul appel, n'est jamais remise à zéro, n'a qu'une moitié même sous ASan, et sort par « zone place <nom> pleine, limite N octets ». *Si elle n'existe pas* : la liste `qy` et l'éclat prennent un bloc simple de taille fixe, et `pico8_ptrs` à capacité réservée vise ce bloc ; les tailles de place baissent de l'en-tête.
- **H-2-4 (module de zone à sortes).** Le module de zone interface expose par sorte une poignée qui donne et reprend une place entière, et un compte de vivants ; en 2a il est adossé à la transitoire par une poignée donnée par un fichier qui la référence déjà ; en 2b, `pico8_zone_choisie_t` gagne une construction sur `pico8_slot_zone_t` (`alloc(n)` ≤ taille de place par `assert`, `grow` refusé, `release` rend la place) et `pico8_zone.h` l'accesseur du nombre de places prises. *Si elle tombe* : l'étape 3 ajoute elle-même la construction et l'accesseur, dans son morceau 1 (3a pour l'accesseur, 3b pour la construction).
- **H-2-5 (rejeu et remplissage).** Il existe un TU de rejeu qui pilote un `events.json` frame par frame et lit les comptes après un vidage, et le rejeu (`main.c`) peut écrire le remplissage par sorte en fin de session (option ou variable d'environnement). *Si elle tombe* : l'étape 3 écrit ce TU dans son morceau 5, sur le modèle de `src/main.c`.
- **H-2-6 (limites).** `PICO8_ZONE_INTERFACE_BYTES` est relevé à 320 Ko, aucune autre limite ne bouge : somme 1472 Ko sur 1536, marge 64 Ko, zone particules à 128 Ko. *Si la marge est plus petite* : le plafond de relèvement de la zone particules baisse d'autant ; le total prévu (≈ 119 Ko) n'a pas besoin de relèvement tant que la zone reste à 128 Ko.
- **H-2-7 (comptes laissés).** Liste prévue après l'étape 2 : 25 fichiers, 91 appels ; parmi les fichiers de l'étape 3 : `particle` 2, `probe` 4, `planet_family` 9 (inchangés), `planet` 7 (le `ne` de `jo:hk` est passé en zone frame). *Si l'écart est autre* : les tables de cette PRD se récrivent sur la liste mesurée ; les baisses par site restent.
- **H-2-8 (sites non touchés).** L'étape 2 ne touche ni `f_`, ni `g_:mm`, ni `ga:mm`, ni `pico8_ga_struct_new`, ni `pm:pq`, ni `jo:pq` (hors-périmètre de sa PRD : « Particules … étape 3 »). *Si l'un l'a été* : il sort de l'inventaire, et « Découvert » le dit.
- **H-2-9 (références remises à nil).** Au vidage, `o_` et `gj` de l'`ok` courant, `kc` et `ju.ed` sont remis à nil s'ils désignent **l'entité rendue, quelle qu'en soit la sorte**, pas seulement une sorte d'interface. *Si la remise ne vise que les sortes d'interface* : l'étape 3 l'étend à `ga` (survolable par son `hd`) dans son morceau 4.
- **H-2-10 (`fv`).** L'étape 2 laisse la struct `fv` « aux étapes 3 et 4 » sans la toucher, et ne lui donne pas de membre. *Si elle lui en a donné un* : l'étape 3 le complète au lieu de le créer.
- **H-2-11 (« Reçu de l'étape 2 » sous l'entrée 3).** La note de clôture de l'étape 2 ajoute sous l'entrée 3 du plan que la file et le membre sont prêts pour les particules. *Si elle ajoute autre chose* (une sorte renvoyée à l'étape 3, un `pico8_fatal` d'interface lu comme limite), cette PRD le traite ou dit pourquoi non.

## À revérifier une fois l'étape 2 finie

Avant le morceau 1, contre le `HEAD` où l'étape 2 est close :

- la note « Découvert » de l'étape 2 et toute ligne « *Reçu de l'étape 2* » sous l'entrée 3 de `docs/plan-4.md` (H-2-11) ;
- la ligne « Particules » du §2, au cas où l'étape 2 l'aurait corrigée ;
- `tools/alloc_guard_exceptions.txt` et `make alloc-guard` : point de départ réel des tables (H-2-7) ;
- `pico8_zone_sizes.h` : `PICO8_ZONE_INTERFACE_BYTES` et la somme ; recalculer la marge et le plafond de la zone particules (H-2-6) ;
- `pico8_struct_class_t`, le module de file de sortie, la primitive d'arène, le module de zone interface et `pico8_zone.h` : noms et formes réels (H-2-1 à H-2-4) ;
- le membre de la famille `jo` et ce qu'il fait pour `ga`, `hj`, `pm`, `mx` (H-2-2) ; l'absence de membre pour `g_` et `fv` (H-2-10) ;
- la remise à nil au vidage, et pour quelles entités (H-2-9) ;
- le TU de rejeu et l'écriture du remplissage par le rejeu (H-2-5) ;
- `git log -p` des commits de l'étape 2 sur `pico8_particle.c`, `pico8_probe.c`, `pico8_planet_family.c`, `pico8_planet.c`, `pico8_help.c` (H-2-8) ;
- les numéros de ligne de l'inventaire, relevés de nouveau par `objdump -r` et `addr2line`.

Chaque hypothèse tombée se note dans une section « Vérifié contre l'étape 2 » ajoutée à cette PRD, comme l'a fait la PRD de l'étape 2 pour l'étape 1, avant de passer la PRD en `triage`.

## Further Notes

- **Pas d'humain disponible, pas d'outil `Agent` dans cette session.** Les points que `to-prd` fait valider par un humain ont été tranchés en appliquant moi-même le skill `guideline` :
  - **Hypothèses explicites** : les H-2-n ci-dessus, et les preuves par lecture marquées « se vérifie en 3a ».
  - **Simplicité** : pas d'outil nouveau quand l'étape 2 en fournit un (file, membre, arène, module à sortes) ; un seul ajout partagé, la capacité réservée de `pico8_ptrs`, sans quoi une liste `qy` n'a pas de taille fixe. Deux sortes de liste seulement (`qy`, `qy_fv`), parce que l'aide vit dix fois plus de particules que les autres émetteurs.
  - **Changements chirurgicaux** : aucun objet de partie converti ; `jo:pq` gagne une variante à zone passée, sa forme actuelle délègue sans changer ses appelants.
  - **Critères vérifiables** : comptes du cliquet par morceau, `*.sha256` inchangés, `sanitize-net`, un invariant par frame sur les 26 e2e et quatre relevés de comptes.
- **Pourquoi une particule meurt sur-le-champ, et l'éclat par la file.** Une particule n'a qu'un propriétaire, sa liste, et personne ne la relit après `del`. L'éclat est une entité : `ui:jz` parcourt `ic`, `ok.gj` peut le désigner, `dg` écrit son `t` après `he`. C'est exactement le cas que la file de l'étape 2 résout.
- **Pourquoi l'égalité brute du §3 ne suffit pas pour `particule`.** Un émetteur actif fait varier le compte d'une frame à l'autre sans rien perdre (naissances tirées par `rnd`, morts au fil de `l`). L'invariant `particule` = Σ `#qy` est plus fort que l'égalité à deux frames : il casse à la frame même d'une fuite. Les sortes `qy`, `qy_fv` et `ga`, elles, suivent la règle du §3 telle quelle. La note « Découvert » le reprend.
- **Risque de budget.** Le poste le plus lourd est la sorte `particule` (64 Ko). Si un e2e ou une partie la remplit, on lit d'abord une particule jamais rendue (invariant cassé ?), puis on relève le nombre de places : 2048 → 3072 ferait 32 Ko de plus, dans la marge. La liste de l'aide est le seul émetteur dont la capacité est connue à l'avance (5 par frame, vie fixe).
- À la fin de l'étape, la note « Découvert » sous l'entrée 3 du plan porte au minimum :
  - les sites classés autrement que dans le §2, et pourquoi (structs `g_`, `fv`) ;
  - les tailles et nombres de places retenus par sorte, et les capacités réservées des listes ;
  - chaque `pico8_fatal` rencontré, lu comme « place jamais rendue » ou « limite trop basse » ;
  - `PICO8_ZONE_PARTICLES_BYTES` et la somme des zones ;
  - les comptes du cliquet laissés, fichier par fichier, et leur total ;
  - les écarts entre les prévisions de cette PRD et la mesure, et les hypothèses H-2-n tombées.
