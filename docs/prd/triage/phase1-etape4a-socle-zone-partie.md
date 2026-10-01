---
title: "Plan 4, étape 4a — Socle de la zone partie : bascule sur nouvelle partie, remplissage, inventaire, ordre des familles"
description: "La zone partie n'existe que comme une taille dans pico8_zone_sizes.h : 4a lui donne un module vide, une remise sur nouvelle partie qui ne tue rien de ce que la frame relit encore, son empoisonnement ASan, un accesseur de remplissage écrit par le rejeu, l'inventaire du §4 des pointeurs qui survivent à une nouvelle partie, et l'ordre des familles 4b et suivantes, sans convertir aucun propriétaire."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. Le texte de l'étape, recopié du plan (relu à `HEAD` e88df7a, étape 3 finie) :

> 4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement, et si son remplissage ne monte pas pendant une phase sans action d'un e2e.
>    Elle se fait en sous-étapes, une PRD chacune :
>    - **4a. Socle.** La zone partie, vide, avec sa remise à zéro et son empoisonnement sur nouvelle partie, son accesseur de remplissage (octets pris, et maximum, §3) écrit par le rejeu, et l'inventaire du §4 des pointeurs qui survivent à une nouvelle partie. Aucun propriétaire converti.
>    - **4b et suivantes.** Une famille de propriétaires par PRD (tableaux croissants et registres ; planètes et relais ; slipways et routes ; recherche ; états de `pico8_gen.c` ; trous de ver), dans l'ordre que fixe la PRD 4a. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
>    - *Reçu de l'étape 1 :* même chose pour les boîtes `vec` des objets de partie (`p`/`v`/`ra` de `qb`, `ra` de `nf`, `mo->v`, `re`/`p` de `jc:hq`, `p` de `pico8_bv`, `ie.gy`) ; `qb:ph` seul en fait trois par frame. L'état `oh` de `pico8_bg_pq` est un objet de frame fait par `pico8_jf`, qui ne prend pas de zone : l'étape 4, qui donne une zone aux états de `pico8_gen.c`, le range en zone frame.
>    - *Reçu de l'étape 2 :* restent à cette étape `ok.qa` (gardé par les hôtes de `bv` et le point souris de `jc:hq` : objet relu par des objets de partie), la structure `ok` et ses calques `ic`, la liste statique de départ, `ie.jb` (fait une fois par partie), `fv`, la structure `pj`, `hp.i`. Les structures de planète ne sont toujours pas libérées (seule leur rangée l'est).
>    - *Reçu de l'étape 3 :* la structure `g_` (et `fv`) et sa boîte `p` de `bv` restent à cette étape (`/* TROU-7 */`) ; `oe` d'une planète ordinaire passe par `pico8_jo_pq_dans`, qui prend déjà sa zone.

Cette PRD ne traite que **4a**. Elle fixe aussi, parce que le plan le lui demande, l'ordre des familles 4b et suivantes et la méthode de suivi du budget qu'elles appliqueront.

Les règles du plan qui la cadrent :

- §2 : la zone partie contient « planètes, relais, slipways, routes `trade`, recherche, trous de ver ; les tableaux croissants (`ents`, `ptrs`, registres) qui les contiennent ». Elle est de « taille fixe ; vidée entièrement à chaque nouvelle partie ».
- §3 :
  - limites au doigt mouillé ;
  - somme des zones ≤ 1,5 Mo ; un relèvement qui la dépasserait arrête l'étape et la fait signaler (zone, limite voulue, somme actuelle), sans rogner une autre zone ;
  - limite atteinte = `pico8_fatal` ;
  - invariant de chaque commit : `make test-all`, `make sanitize-net`, `*.sha256` identiques ;
  - sous ASan, la remise à zéro de la zone partie à chaque nouvelle partie empoisonne la mémoire rendue ;
  - « Croissance par frame » : chaque zone expose son remplissage et le maximum atteint pendant la session (octets pris pour une zone montante), et le rejeu les écrit. Le remplissage d'une zone qui n'est pas remise à zéro à chaque frame est égal à deux frames séparées par une phase sans action. Une hausse se corrige à l'étape qui l'a fait entrer, pas en relevant la limite.
- §4 :
  - un site douteux va dans la zone la plus longue qui le couvre ; ASan ne fait que rétrograder ;
  - ASan ne voit pas un pointeur gardé à travers deux remises à zéro ;
  - **avant de faire finir une durée de vie, la PRD inventorie mécaniquement les pointeurs du cœur qui survivent** : `static … *` de portée fichier, et champs pointeurs des structures statiques et des singletons. Pour chacun, elle dit s'il est remis à nil, repointé, ou s'il ne peut pas viser la sorte rendue.
- §5 : fin d'étape = chaque site de l'inventaire converti, `alloc-guard` à égalité stricte, aucun compte n'a monté. 4a ne convertit aucun site : ses comptes ne bougent pas.
- §6 : pas de partie longue enregistrée.

**Ce qui se passe aujourd'hui.**

- La zone partie n'est qu'une taille : `PICO8_ZONE_GAME_BYTES` vaut 768 Ko dans `pico8_zone_sizes.h`. Elle compte dans le contrôle de somme : frame 256 + interface 335 + particules 128 + partie 768 = **1487 Ko sur 1536**, soit **49 Ko de marge** sur le total. Aucun module ne la possède et rien n'y alloue.
- La zone particules n'occupe que **113 822 o (111 Ko) de ses 128 Ko** (contrôle à la compilation de `pico8_zone_particules.c`), non relevée. Les ~17 Ko qui restent dedans ne sont pas de la marge pour la zone partie : les reprendre, ce serait rogner une autre zone (§3). Ils restent aux particules, et c'est là que tiendront les traînées `ts` (voir plus bas).
- Les objets de partie sont pris à la libc ou à la transitoire et jamais rendus (TROU-7). Seule exception depuis l'étape 3 : l'éclat `ga`. Sa structure `pico8_jo_t`, son `oe` et sa copie de position tiennent dans une place de la zone particules, rendue par la file de sortie à sa mort, **en cours de partie**. C'est la première structure de la famille planète vraiment libérée. Un éclat né par `jo:hu` emprunte encore `p`, `ra` et `id` à la planète qu'il remplace, qui est un objet de partie.
- Aucune « nouvelle partie » n'est nommée dans le code. Il y a quatre entrées, toutes appelées par le `my` d'un bouton, donc depuis `pico8_js_ldown`, au milieu de `ok:lj` (`pico8_ui_jz`, puis `pico8_ui_bh`) :

  | Entrée | Bouton | Appelle |
  |---|---|---|
  | `eu` (choix de difficulté) | boutons `bo` de l'aide `fv`, à l'écran titre | `dk=nj`, `jl()`, `bg()`, `bw(90)`, `fm,ju=hy(),pj()`, menus, sept `ie`, `fm:lj()` |
  | `_init` par « restart » | `ix[4]`, première entrée du menu pause | `cu()`, `jl()`, `ie.gy=ie.eh`, `kc=nil`, `fv()` |
  | `_init` par « again? » | bouton de `pj:ph`, écran de fin | idem |
  | `_init` au démarrage | `main.c`, après `pico8_cart_load()` | idem |

  `jl()` abandonne tout ce qui est inscrit dans `hv` : chaque entité qui a `rendre` est mise en file de sortie. Puis il refait `hv`/`cv`/`cg` et les singletons `mo`, `je`, `ok`.

**Ce qui est encore lu après `jl()`.** Une remise à zéro faite *dans* `jl()` ferait relire de la mémoire rendue, en quatre endroits (relevés par lecture à `HEAD`) :

1. **La fin de `ok:lj` sur l'ancien `ok`.**
   - Après le retour du `my`, `pico8_ui_bh(self, "l")` termine sur l'ancien `ok` : `self->qa`, `self->lheld`, `hq` sur l'objet tenu.
   - Puis `pico8_ui_bh(self, "r")` : `hq` sur l'ancien `rheld` (le `bg` de l'ancienne partie, si le bouton droit est tenu), et, au relâcher, `pico8_ui_jz(GV)` sur les calques `ic` de l'ancien `ok`, donc `gv` sur des planètes de l'ancienne partie.
   - Puis `pico8_ui_cj` fait `he` sur l'ancien `ok.le`, et `self->gj = self->o_`.
2. **Le vidage de la file**, en tête du `pico8__update60()` suivant. Il appelle `rendre` sur les entités abandonnées par `jl()`. `pico8_jo_rendre_dispatch` lit la struct de la planète (`lq_place`). Les `rendre` des émetteurs (`hj`, `pm`, `mx`, `g_`, `fv`) lisent aussi `self->emitter` pour rendre leur `qy`. Celui de `ga` lit `oe->zone` puis rend sa place. `oublier` lit `ok->o_`, `ok->gj`, `ok->lheld` (et le `re` d'un `jc` tenu, ajouté par l'étape 3), `kc`, `ju->ed` et `dk`.
3. **Entre « restart »/« again? » et `eu`**, à l'écran titre :
   - `fm` et `ju` désignent encore le `hy` et le `pj` de la partie quittée ; le cart ne les change que dans `eu` ;
   - la triche (`pico8_cheat.c`, tapée au titre) écrit dans `pico8_fm_hy()` ;
   - `oublier` lit `ju->ed` à chaque vidage ;
   - à `eu`, `pico8_probe_set_fm` passe l'ancien `hy` à `fm_free`, qui lit son `cls` ;
   - à `eu`, `bw` rend les anciens états de `pico8_gen.c` (`g_bw_oc_r_ctx`, `jo.oc`, `jo.ir`).
4. **Sur PSX, sans ASan**, une zone remise à zéro au milieu de la frame redonnerait aussitôt ses adresses aux objets de la nouvelle partie, alors que les trois lectures ci-dessus portent encore sur les anciens. Ce serait une corruption silencieuse, pas un crash.

Une zone partie « vidée à chaque nouvelle partie » ne peut donc pas être vidée à l'endroit où le cart crée la nouvelle partie. C'est ce que 4a doit trancher avant qu'un seul propriétaire y entre.

## Solution

4a crée la **zone partie**, vide, et son contrat de durée de vie. Aucun propriétaire n'y entre. Aucun pixel ne change. Aucun compte du cliquet ne bouge.

- **Nouvelle partie = `eu`.** La zone partie bascule en tête de `pico8_eu`, juste avant son `jl()`. `_init` (« restart », « again? », démarrage) ne bascule pas : l'écran titre vit dans la génération de la partie qu'il suit, comme `fm` et `ju` qui la désignent encore.
- **Zone à deux bouts (« bascule »).** Une seule réserve de `PICO8_ZONE_GAME_BYTES` octets. La partie courante alloue depuis un bout, la suivante depuis l'autre. La bascule ne rend rien : elle change de bout, et l'ancienne génération reste valide.
- **Rendu différé au vidage suivant.** L'ancienne génération est rendue d'un bloc, et empoisonnée sous ASan, juste après le vidage de la file de sortie qui suit la bascule. C'est-à-dire en tête du `pico8__update60()` suivant, après la remise de la zone frame et le vidage de la file, avant `dg`. Les quatre lectures ci-dessus tombent toutes avant ce point.
- **Remplissage et maximum.** Le module expose les octets pris (les deux générations tant que l'ancienne attend) et leur maximum sur la session. Le rejeu les écrit sous `PICO8_REMPLISSAGE`.
- **Inventaire du §4** des pointeurs qui survivent à une nouvelle partie, refait mécaniquement à `HEAD` (voir Implementation Decisions). Conclusion : avec la bascule à `eu` et le rendu au vidage suivant, aucun pointeur du cœur ne lit l'ancienne génération après son rendu. Rien n'est à remettre à nil en 4a.
- **Une durée de vie absente du §2 : le chargement.** Les projets de recherche `gb` (avec `hp.i` et ses `jy`) et deux listes statiques (`g_shared_le`, `g_ga_by_shared`) sont faits par `pico8_cart_load()` et vivent d'une partie à l'autre. Ils ne peuvent pas entrer dans une génération. 4a décide une sous-zone **chargement**, prise dans `PICO8_ZONE_GAME_BYTES` et remise à zéro seulement par le chargement du cart. La première famille qui en a besoin (4c) la crée.
- **Une sorte de particules absente du §2 : les traînées `ts` de `jc:ch`.** Elles naissent par tirage `rnd` à chaque frame et meurent par `del(self.ts,t)`. L'étape 3 ne les a pas prises : la ligne « Particules » qu'elle a corrigée ne les nomme pas, et `pico8_jc_ch` fait toujours son `malloc`. Dans une zone partie montante, elles feraient croître le remplissage à chaque tick. 4a les confie à la famille slipways, qui les range en zone particules, dans les ~17 Ko restés libres, avec un point de libération.
- **L'ordre des familles 4b à 4h**, l'attribution de chacune des 81 relocations du cliquet qui restent à l'étape 4 (84 mesurées, moins les 3 de `pico8_zone_transitoire.c`, qui vont à l'étape 5), et la **méthode de suivi du budget** (relevé, projection, arrêt anticipé).
- **Changements du plan, à écrire dans `docs/plan-4.md` à la clôture de 4a** (voir « Changements du plan ») :
  - la ligne « Partie » du §2 (point de rendu, sous-zone chargement) ;
  - la ligne « Particules » du §2 (traînées `ts`) ;
  - la famille 4h ajoutée à la liste de l'entrée 4, avec l'ordre des familles.

## User Stories

1. En tant que mainteneur du plan, je veux une zone partie qui existe comme module, vide, avant tout propriétaire, pour que chaque famille 4b+ n'ait plus qu'à choisir sa poignée.
2. En tant que futur porteur PSX, je veux que la zone partie soit une seule réserve de taille fixe, sans moitié supplémentaire en build normal, pour que sa taille lue dans `pico8_zone_sizes.h` soit ce que la PSX paiera.
3. En tant que développeur, je veux que la nouvelle partie soit nommée en un seul point, en tête de `pico8_eu`, pour qu'aucune famille n'ait à se demander si « restart » ou « again? » vide la zone.
4. En tant que développeur, je veux que `_init` ne fasse pas basculer la zone partie, pour que `fm` et `ju`, que le cart garde jusqu'à `eu`, désignent encore des objets vivants à l'écran titre (triche, `fm_free`, `oublier`).
5. En tant que développeur, je veux que la bascule laisse l'ancienne génération valide jusqu'au vidage suivant, pour que la fin de `ok:lj` sur l'ancien `ok` et les `rendre` de la file lisent des objets vivants.
6. En tant que développeur, je veux que l'ancienne génération soit rendue juste après le vidage de la file qui suit la bascule, et pas avant, pour qu'aucun `rendre` ne lise une struct de planète déjà rendue.
7. En tant que développeur PSX, je veux que la nouvelle génération ne réutilise aucune adresse de l'ancienne tant que celle-ci attend, pour qu'aucune corruption silencieuse ne remplace le crash qu'ASan montre sur PC.
8. En tant que développeur qui chasse un accès hors durée de vie, je veux que l'ancienne génération soit empoisonnée sous ASan au moment même de son rendu, pour qu'un pointeur gardé vers elle fasse planter `make sanitize-net`.
9. En tant que développeur, je veux qu'une bascule demandée alors qu'une génération attend encore son rendu échoue par un `assert` porteur de message, pour qu'un TU qui enchaîne deux `eu` sans frame vide la file au lieu de rendre une génération que la file relira.
10. En tant que développeur, je veux que `pico8_cart_load()` et `pico8_cart_shutdown()` remettent toute la zone partie à zéro (les deux bouts), pour qu'un binaire de TU qui recharge le cart reparte d'une zone vide.
11. En tant que développeur, je veux qu'une zone partie pleine (les deux bouts se rejoignent) sorte par `pico8_fatal` « zone partie pleine, limite N octets », pour qu'une limite trop basse se lise et se relève en un chiffre.
12. En tant que développeur, je veux une poignée `pico8_zone_choisie_t` sur la génération courante, avec `alloc`, `grow` qui copie et un `release` qui ne fait rien, pour que les fonctions partagées de 0b (`ents`, `ptrs`, `concat2`, `tostr`…) y allouent sans changer de forme.
13. En tant que mainteneur, je veux un accesseur qui rend les octets pris dans la zone partie et leur maximum sur la session, pour lire un taux et un pic sans écrire de test.
14. En tant que mainteneur, je veux que le maximum compte les deux générations à la fois pendant la frame de bascule, pour que le pic mesuré couvre le pire moment réel : partie quittée, titre et première frame de la nouvelle partie.
15. En tant que mainteneur, je veux que le rejeu écrive le remplissage et le maximum de la zone partie dans le fichier nommé par `PICO8_REMPLISSAGE`, dans la forme déjà utilisée par les zones interface et particules, pour que chaque famille relève son maximum sur tous les e2e sans code nouveau.
16. En tant que mainteneur, je veux un TU de rejeu qui pose une sentinelle dans la zone partie avant le clic « challenging » de `pause_restart`, la relise sans crash pendant la frame de ce clic, puis plante (enfant, ASan) en la relisant après la frame suivante, pour prouver le point exact de la bascule et du rendu.
17. En tant que mainteneur, je veux que ce TU montre aussi que « restart » (f7) ne fait pas basculer la zone, pour que la règle « nouvelle partie = `eu` » soit tenue par un test et pas seulement par une lecture.
18. En tant que mainteneur, je veux que le TU des places vivantes relève les octets de la zone partie aux frames 45 et 90 de `message_box_hold` et les trouve égaux, pour que le contrôle « Croissance par frame » soit en place avant que la première famille y alloue.
19. En tant que gardien du cliquet, je veux que 4a ne change aucun compte d'`alloc-guard` et que le module de zone partie ne référence ni la libc ni `pico8_zone_transitoire`, pour que le verrou reste à égalité stricte.
20. En tant que gardien de l'invariant, je veux que les 26 `*.sha256` restent identiques, pour prouver qu'une zone vide et sa bascule ne changent ni un pixel ni un tirage `rnd`.
21. En tant que mainteneur du budget, je veux que `PICO8_ZONE_GAME_BYTES` reste à 768 Ko en 4a et que le contrôle de somme reste vert, pour que la marge de 49 Ko sur le total reste aux familles.
22. En tant que développeur des familles 4b+, je veux l'inventaire du §4 des pointeurs du cœur qui survivent à une nouvelle partie, chacun classé, pour savoir lesquels mon commit doit remettre à nil ou repointer, et ne pas refaire l'erreur de `dk`.
23. En tant que développeur des familles 4b+, je veux que chaque famille refasse le même `grep` sur le code de son commit et classe tout pointeur nouveau, pour qu'un pointeur trouvé ensuite par ASan soit noté comme un trou de l'inventaire.
24. En tant que développeur des familles 4b+, je veux un point d'oubli de partie, appelé juste avant le rendu de l'ancienne génération, où une famille ajoute la remise à nil d'un pointeur que son inventaire désigne, pour qu'il n'existe qu'un seul endroit où une référence à l'ancienne partie est coupée.
25. En tant que lecteur du plan, je veux que la ligne « Partie » du §2 dise le vrai point de rendu (le vidage qui suit `eu`) et la durée de vie « chargement » de `gb` et des listes statiques, pour que le tableau ne contredise plus le code.
26. En tant que lecteur du plan, je veux que la ligne « Particules » du §2, déjà corrigée par l'étape 3, nomme aussi les traînées `ts` de `jc:ch`, pour qu'aucune famille ne les range en zone partie, où elles croîtraient par tick.
33. En tant que développeur des familles 4b+, je veux que l'inventaire tienne compte des éclats `ga`, rendus en cours de partie depuis l'étape 3, pour savoir quels objets de partie peuvent désigner un objet déjà rendu avant même toute nouvelle partie.
27. En tant que développeur des familles 4b+, je veux l'ordre des familles, chacune avec ses fichiers, ses fonctions, ses relocations et ses lignes « Reçu », pour que la somme des baisses prévues tombe exactement sur les comptes laissés par l'étape 3.
28. En tant que développeur des familles 4b+, je veux la liste des sites qui allouent par tick ou par action, rattachés à leur famille, pour qu'aucun n'entre tel quel dans une zone qui ne rend rien avant la partie suivante.
29. En tant que mainteneur du budget, je veux une estimation par lecture de l'empreinte de chaque famille, à remplacer par la mesure au fil des familles, pour pouvoir projeter le total avant la dernière famille.
30. En tant que mainteneur du budget, je veux une règle de projection et d'arrêt (relevé cumulé + estimations restantes, comparées à 768 Ko puis à 768 Ko + marge), pour arrêter l'étape et la signaler dès qu'elle ne peut plus tenir, comme le veut le §3.
31. En tant que mainteneur, je veux une note « Découvert » sous 4a, qui dise le point de bascule et de rendu, l'inventaire, l'ordre des familles et les corrections du §2, pour que les PRD 4b+ partent d'un plan à jour.
32. En tant que mainteneur des commentaires, je veux que le seul commentaire ajouté par 4a soit l'`assert` porteur de message de la double bascule et le contrat de la poignée (ce que rend `release`, qui rend la génération), pour respecter les quatre destinations du `CLAUDE.md`.

## Implementation Decisions

### Le point de nouvelle partie : bascule dans `eu`, rendu au vidage suivant

- **Bascule.** Elle est faite en tête de `pico8_eu`, après `pico8_economy_set_dk(nj)` (qui n'alloue rien) et avant `pico8_jl()`. Tout ce que `eu` construit ensuite va dans la nouvelle génération : listes `hv`/`cv`/`cg`, `mo`/`je`/`ok`, `bg`, la galaxie de `bw(90)`, `hy`, `pj`, les menus, `ie`.
- **Pas de bascule dans `_init`.**
  - Le cart garde `fm` et `ju` de la partie quittée jusqu'à `eu`. La triche les écrit au titre, et `oublier` lit `ju->ed`.
  - L'écran titre (`mo`/`je`/`ok`, `fv`) s'ajoute à la génération courante. Il est rendu avec elle au vidage qui suit le prochain `eu`.
  - La croissance d'un `_init` est bornée : un seul `_init` peut précéder un `eu`. Le menu pause n'existe qu'en partie.
- **Pas de bascule dans `jl()`.** Elle tomberait dans `_init` (voir ci-dessus), et la fin de `ok:lj` lit encore l'ancien `ok` après le retour du `my`.
- **Rendu.** Il est fait par `pico8_cart_vider_file_sortie()`, juste après `pico8_file_sortie_vider(oublier)`, si une génération attend : d'abord le point d'oubli de partie, puis le rendu et l'empoisonnement du bout ancien.
  - En jeu, cela tombe en tête du `pico8__update60()` qui suit la frame du clic, après `pico8_zone_frame_reset()` et avant `pico8_dg()`.
  - Les autres appelants du vidage font aussi le rendu : `pico8_cart_load()`, `pico8_cart_shutdown()`, et `main.c` avant d'écrire le remplissage.
  - L'ordre vidage puis rendu est le seul qui laisse les `rendre` lire des structs vivantes. Un TU le prouve (Testing Decisions).
- **Point d'oubli de partie.** Une fonction de `pico8_cart.c`, sœur d'`oublier`, appelée avant le rendu. En 4a elle est vide : l'inventaire ne désigne aucun pointeur à couper. Une famille y ajoute une remise à nil seulement si son propre inventaire en trouve un. Elle ne remplace pas `oublier`, qui vise une entité, pas une génération.
- **Double bascule sans vidage.** Une bascule demandée alors qu'une génération attend encore est refusée par `assert`. Le message dit qu'il faut vider la file entre deux `eu`. Le jeu ne peut pas déclencher ce cas : un clic par frame, et `eu` n'est atteint que depuis l'écran titre. Les étapes 2 et 3 ont fait vider la file aux aides de TU qui en avaient besoin, mais pas à toutes. `tests/test_pico8_hy.c` (cas « construction ») enchaîne deux `new_game()`, donc deux `pico8_eu`, sans vidage ni frame. Ce TU et tout autre que l'`assert` désignera au premier `make test` sont corrigés dans le commit de 4a, par un `pico8_cart_vider_file_sortie()` entre les deux `eu`.
- **Chargement et arrêt.** `pico8_cart_load()` et `pico8_cart_shutdown()` remettent toute la zone à zéro, après leur vidage : les deux bouts, maximum de session non compris. Le maximum ne se remet qu'avec le processus.

### Primitive : la zone à deux bouts

- Ajoutée au module de zone générique, à côté de la zone montante et de la zone à places. Elle a les mêmes conventions : stockage déclaré par le propriétaire, nom pour le message de `pico8_fatal`, allocateurs en `unsigned char *` (composition-guard refuse `void *`).
- **État** : un bout bas qui monte, un bout haut qui descend, une capacité partagée, le bout courant, un drapeau « ancienne génération en attente », et le maximum des octets pris.
- **Opérations** (noms laissés à l'implémenteur) :
  - allouer au bout courant ;
  - basculer : change de bout, marque l'autre en attente, exige qu'aucun ne l'était ;
  - rendre l'ancienne : remet le bout en attente à vide et l'empoisonne ;
  - tout remettre à zéro : les deux bouts, empoisonnés ;
  - lire les octets pris et le maximum.
- **Plein** : quand les deux bouts se rejoignent, `pico8_fatal` « zone <nom> pleine, limite N octets », dans la forme de `zone_full`.
- **ASan** : pas de seconde moitié. Les générations alternent déjà entre les deux bouts, et une génération n'est empoisonnée qu'à son rendu. Angle mort, à noter comme pour 0a : un pointeur gardé à travers **deux** nouvelles parties retombe sur le bout réutilisé et ne plante pas.
- **Poignée** : un troisième constructeur de `pico8_zone_choisie_t` sur cette zone. `alloc` prend au bout courant, `grow` copie (l'ancien bloc reste mort jusqu'au rendu de sa génération), `release` ne fait rien. La forme exacte (champ ajouté à la poignée, ou poignée qui pointe sur la zone) est laissée à l'implémenteur, à condition que la poignée soit statique et vive autant que la zone : une liste `ents`/`ptrs` garde un pointeur vers sa poignée.

### Module de zone partie

- Un module du cœur, sur le modèle de `pico8_zone_frame.{h,c}` (stockage statique, initialisation paresseuse qui empoisonne). Il est **hors liste d'exceptions** : ni libc ni transitoire.
- **Interface** :
  - la poignée de la génération courante ;
  - la bascule (appelée par `pico8_eu`) ;
  - le rendu de l'ancienne génération s'il y en a une (appelé par le vidage) ;
  - la remise à zéro complète (chargement, arrêt) ;
  - les octets pris et leur maximum.
- **Taille** : `PICO8_ZONE_GAME_BYTES` reste à **768 Ko**. Le contrôle de somme de `pico8_zone_sizes.h` ne change pas : 1487 Ko sur 1536, marge de 49 Ko sur le total (zone particules à 128 Ko, dont 111 Ko occupés).
- **Sous-zone chargement : décidée ici, créée par la famille 4c.** C'est un changement du plan : le §2 n'a pas cette durée de vie, et la clôture de 4a l'écrit dans la ligne « Partie ». 4c est la première famille qui en a besoin (`g_shared_le`, `g_ga_by_shared`). Ce sera une zone montante à une moitié, remise à zéro seulement par `pico8_cart_load()`/`pico8_cart_shutdown()`, prise **dans** `PICO8_ZONE_GAME_BYTES`, avec une vérification à la compilation que chargement + partie ≤ 768 Ko. Taille de départ au doigt mouillé : 64 Ko, prise sur les 768 (partie 704 Ko). Elle ne reçoit que ce que fait le chargement. Ce qui y grandirait ensuite (`g_shared_le`, où `ok.le` ajoute des boutons) doit y réserver sa capacité au chargement, comme la liste `qy` de l'étape 3, ou aller ailleurs. 4a ne la crée pas : aucune poignée sans utilisateur.
- **Makefile** : le nouvel objet entre dans toutes les listes de sources qui lient `pico8_zone_frame.c` (`ENGINE_SRCS`, `DATA_ENGINE_SRCS` si `pico8_cart.c` y est, ligne de lien de `main_raylib`) et dans la règle ASan du TU du module.

### Remplissage écrit par le rejeu

- Sous `PICO8_REMPLISSAGE`, `main.c` écrit, après le vidage qu'il fait déjà, deux lignes pour la zone partie, dans la forme que `write_remplissage` a prise à l'étape 3 : la valeur courante sur une ligne `<nom> <n>`, le maximum sur une ligne préfixée. Les particules écrivent `<sorte> <vivantes>`, puis `max_vivantes <sorte> <n>`, `max_octets <sorte> <n>`, `max_elements <sorte> <n>`. La zone partie écrit donc `partie <octets pris>` et `max_octets partie <n>`. La capture, le dump et le code de sortie ne changent pas.
- Les octets pris après le vidage final ne comptent que la génération courante : l'ancienne vient d'être rendue. Le maximum, lui, garde le pic de la frame de bascule.

### Inventaire du §4 : pointeurs qui survivent à une nouvelle partie

Fait **mécaniquement** à `HEAD` e88df7a, sur les 64 fichiers de `CORE_SRCS` (liste tirée du `Makefile` ; l'étape 3 a ajouté `pico8_zone_particules.c`) :

- `grep -nE '^\s*static\s[^(]*\*\s*[a-zA-Z_][a-zA-Z0-9_]*(\[[^]]*\])*\s*(;|=)'`, hors `const char *` et tables `* const` : 18 pointeurs. Le motif accepte désormais plusieurs dimensions : le motif de l'étape 3, à une seule dimension, ne voyait pas `g_arenes` ;
- `grep -nE '^static\s+(struct\s+)?[a-z0-9_]+(_t)?\s+[a-zA-Z_][a-zA-Z0-9_]*\s*(\[[^]]*\])*\s*(;|=)'` hors `const` et scalaires, dont on lit les champs pointeurs : 33 structures (les 31 d'avant l'étape 3, plus `g_zones` et `g_slots` de `pico8_zone_particules.c`), plus un `static` de bloc (`copie`, `pico8_economy_dk_quitte`) ;
- les champs pointeurs des singletons atteints : `ok`, `mo`, `je`, `fm`, `ju`.

**La sorte rendue** est une génération de la zone partie : tout objet alloué par la poignée de partie entre deux bascules. En 4a, aucun objet n'y est, et le classement dit ce qui se passera quand les familles y mettront leurs objets. La question posée à chaque pointeur : **après le rendu (le vidage qui suit `eu`), peut-il encore désigner un objet de la génération rendue, et est-il lu ?**

**Variables de portée fichier.**

| Pointeur | Fichier | Statut après le rendu | Famille qui le revérifie |
|---|---|---|---|
| `g_gfx`, `g_mem`, `g_input` | `dispatch_context` | ne peut pas viser (type) | — |
| `g_pi_gfx`, `g_pi_mem` | `pi` | ne peut pas viser (type) | — |
| `g_dk` | `economy_score` | ne peut pas viser : un `js` (zone interface) ; repointé sur `copie` par `oublier` | — |
| `g_mo` | `entity` | **repointé** par le `jl()` d'`eu`, après la bascule | 4h |
| `g_je`, `g_ok` | `ui` | **repointés** par le `jl()` d'`eu`, après la bascule ; l'ancien `ok` n'est plus lu une fois `ok:lj` fini | 4h |
| `g_bw_oc_r_ctx` | `galaxy` | **repointé** par `bw` dans `eu`, après la bascule ; l'ancien est rendu par `bw` à ce moment, encore vivant | 4b |
| `g_jo_oc`, `g_jo_ir` | `planet` | **repointés** par `bw` (`pico8_jo_set_oc`/`_ir`), idem ; `pico8_gn_free` lit l'ancien encore vivant | 4b |
| `g_kc` | `msgbox` | ne peut pas viser : un `mh` (zone interface) ; remis à nil par `_init` et `oublier` | — |
| `g_fm` | `probe` | **repointé** par `eu` (`pico8_probe_set_fm`), après la bascule ; entre `_init` et `eu` il désigne l'ancien `hy`, encore vivant (triche, `fm_free`) | 4g |
| `g_gb[]` | `research` | ne doit pas viser la génération : les `hp` sont faits au chargement et vivent d'une partie à l'autre (sous-zone chargement) | 4g |
| `g_ie_gy` | `research_icon` | **repointé** par `_init` sur `ie.eh` (statique), puis avancé par les `ie` d'`eu` sur des boîtes de la nouvelle génération | 4g |
| `g_ju` | `score_panel` | **repointé** par `eu`, après la bascule ; lu par `oublier` (`ju->ed`) au vidage qui précède le rendu, encore vivant | 4h |
| `g_arenes[][]` | `zone_particules` | ne peut pas viser : des places de la zone particules (`qy`, `qy_fv`, `ga`) | — |

**Champs des structures statiques et des singletons.**

| Champ | Statut après le rendu | Famille |
|---|---|---|
| `g_hv`, `g_cv[].list`, `g_cg[].list` (`entity`) | **refaits** par le `jl()` d'`eu` (tableaux de la nouvelle génération) ; les anciennes entités n'y sont plus | 4c |
| `g_file[]` (`file_sortie`) | tient les entités abandonnées par `jl()` ; **vidé avant le rendu**, par construction du point de rendu | — |
| `g_mo_placeholder`, `g_je_placeholder`, `g_fm_placeholder` | statiques, sans pointeur vers un objet de partie | — |
| `g_bv_et.fo` (`probe`) | peut garder un hôte de `bv` d'une ancienne partie ; **écrit seulement, jamais lu** (seuls `jo.fo`, `jc.fo` sont lus) : ne peut pas être lu | 4h |
| `g_jc_too_far.fo` (`slipway`) | peut garder un `jc` d'une ancienne partie ; **écrit seulement, jamais lu** | 4e |
| `g_ga_by_shared` (`planet_family`) | liste partagée toujours vide ; faite au chargement | 4c |
| `g_shared_le` (`ui`) | liste de départ de `ok.le`, faite au chargement ; n'y entrent que des `js` (zone interface) | 4c |
| `g_ha_pz.hi`/`.gq` (`menu`) | pointent sur `g_ha_hi`/`g_ha_gq`, statiques | — |
| `g_pj_cr` (`score_panel`) | `hi`/`gq` littéraux constants, `ep` entrée de `pico8_nb_table` | — |
| `g_bz[].e`, `g_pj_ou`, `g_pj_ea`, `g_jo_kw_shared`, `g_mz_jj_wrapper`, `g_nf_l`/`g_nf_r`/`g_nf_ra`, `g_pi_dispatch[]`, `g_ix_ra`, `g_ha_hi`/`g_ha_gq` | tables constantes, textes statiques, ou aucun champ pointeur | — |
| `copie` (`economy_score`) | copie d'un `js` : textes et nombres, aucun objet de partie | — |
| zones et poignées (`frame`, `frame_handle`, `g_zones`/`g_handles` de l'interface, `g_zones`/`g_slots` des particules) | ne peuvent pas viser (type) | — |
| `ok.o_`, `ok.gj`, `ok.lheld`, `ok.rheld`, `ok.ic`, `ok.le`, `ok.qa` | champs du **nouvel** `ok`, faits à nil (ou `&g_shared_le`) par sa construction après la bascule ; l'ancien `ok` n'est plus atteint par aucun pointeur une fois `ok:lj` fini. `oublier` lit `ok.lheld` et le `re` d'un `jc` tenu au vidage qui précède le rendu : c'est le nouvel `ok`, dont `lheld` est nil | 4h |
| `mo.v`/`.p`/`.ra`, `je.ra` | champs des nouveaux `mo`/`je` | 4h |
| `fm.invented`, `fm.po`/`.fu` | champs du nouvel `hy` | 4g |
| `ju.ed` | `mh` (zone interface) ; remis à nil par `oublier` | — |

**Objets d'autres zones qui désignent des objets de partie** (hors périmètre mécanique, relevés par lecture) :

- rangées, panneaux et boutons de la zone interface ;
- listes `qy` et éclats `ga` de la zone particules. Un éclat né par `jo:hu` emprunte le `p`, le `ra` et l'`id` de la planète remplacée ; celui de `pm:pq` a sa copie de position dans sa place.

Tous sont des entités, ou appartiennent à une entité, inscrites dans `hv` avec un `rendre`. Le `jl()` d'`eu` les met en file, et le vidage les rend **avant** le rendu de la génération. Aucun ne survit au rendu. `pico8_ga_rendre_dispatch` ne lit que la place de l'éclat (`oe->zone`), sa rangée et sa liste `qy` : jamais les `p`/`ra`/`id` empruntés.

**Éclats `ga` rendus en cours de partie (depuis l'étape 3).** C'est le sens inverse : un objet *de partie* qui désigne un éclat déjà rendu, sans nouvelle partie. Le rendu de la génération n'y est pour rien, mais chaque famille 4b+ doit en tenir compte, car ses objets passent en zone partie et vivent plus longtemps que l'éclat. Relevé par lecture à `HEAD` e88df7a :

| Qui peut désigner un éclat | Pourquoi | Statut |
|---|---|---|
| `hv`, `cv`, `cg` | l'éclat y est inscrit ; aucune étiquette (`.tags = 0`), donc absent de tous les `cg` | **retiré** par `pico8_he` avant l'entrée en file |
| `ok.o_`, `ok.gj` | sa boîte `kq` se survole (`qz` d'un `b(0,0,-1,-1)` est un carré 2×2) | **remis à nil** par `oublier` |
| `re` du `jc` tenu (`ok.lheld`) | `jc:hq` prend `ok.o_` comme `re` quand `o_.gv` existe, et `ga` hérite `gv` de `jo` | **remis à nil** par `oublier` (ajouté par l'étape 3) ; inatteignable aujourd'hui, `hq` réécrit `re` à chaque `bh` |
| `ok.ic` | calques du survol | **refaits** par `gw` à chaque `ok:lj` ; rien ne les lit entre le vidage et `gw` (`dg` ne les lit pas) |
| `qu`/`re` d'un slipway posé, `jc.et` | — | ne peut pas viser : `jo:gv` ne rend vrai que pour `me=="ly"`, et un éclat reste « unknown » ; `jc:ce` ne cherche que dans `qu.oe`, `cg.np` et `cg.jc` |
| `jo.fo`, `jc.fo`, `g_bv_et.fo` | — | ne peut pas viser : `ga` n'a pas `hq` (pas hôte de `bv`) et n'est pas dans `cg.np` |
| `oe` d'une planète, routes `by` | — | ne peut pas viser : `ga` a `by = g_ga_by_shared`, toujours vide ; aucun slipway n'aboutit à lui |

Les familles **4d** (planètes, dont `jo:hu` qui crée l'éclat) et **4e** (slipways : `jc:hq`, `re`, `et`) refont ce relevé sur leur code : leurs objets deviennent ceux qui survivent à l'éclat.

**Conclusion : le point d'oubli de partie est vide en 4a.** Aucun pointeur ne lit l'ancienne génération après son rendu. Le seul cas « vivant mais désigné » est celui de `fm`/`ju` entre `_init` et `eu`. Il est réglé par le choix du point de bascule, pas par une remise à nil. Remettre `fm` au tenant-lieu changerait ce que lit la triche au titre. Les éclats rendus en cours de partie restent l'affaire d'`oublier`, qui les couvre déjà.

**Dans chaque famille 4b+**, le même `grep` est refait sur le code du commit, avec la même question. Un pointeur nouveau y est classé. Un pointeur trouvé ensuite par ASan est ajouté au point d'oubli de partie et noté dans « Découvert » comme un **trou de l'inventaire** (§4).

### Changements du plan (écrits dans `docs/plan-4.md` par le commit de clôture de 4a)

Ce sont des décisions de 4a qui changent le texte du plan, pas seulement des constats. Elles sont écrites dans le plan à la clôture de 4a, dans le même commit que la note « Découvert ».

- **§2, ligne « Partie », durée de vie** : « une génération par partie, de `eu` à `eu` ; rendue d'un bloc au vidage de la file qui suit le `eu` suivant (zone à deux bouts) ; `_init` ne la rend pas ».
- **§2, ligne « Partie », contenu** : « les objets faits au chargement (`gb`, `hp.i`, listes statiques) vivent dans une sous-zone chargement, prise dans la zone partie, remise à zéro par le seul chargement ».
- **§2, ligne « Particules », contenu** : la ligne corrigée par l'étape 3 (« particules de `g_:mm`/`ga:mm`/aide, listes `qy` de leurs émetteurs (`g_`, `fv`, `hj`, `pm`, `mx`, `ga`), éclats `ga` entiers ») gagne « traînées `ts` de `jc:ch` ».
- **Entrée 4, puce « 4b et suivantes »** : la liste des familles devient la liste ordonnée de 4a (4b à 4h), avec la famille **4h** « singletons de `jl`/`eu` et objets du joueur » ajoutée.
- **Entrée 4** : une ligne « *Reçu de l'étape 4a* » confie les traînées `ts` à 4e, en zone particules, dans les ~17 Ko libres de ses 128 Ko.

### Sites qui allouent par tick ou par action

Une zone partie ne rend rien avant la génération suivante. Un site qui alloue à chaque frame, ou à chaque action du joueur, la ferait croître sans borne : par tick, le §3 le voit sur une phase sans action ; par action, il épuise la zone au fil d'une partie longue, que le §6 interdit d'enregistrer. **Règle pour toutes les familles :**

- n'entre en génération de partie qu'une allocation faite une fois par objet de partie créé (planète posée, slipway, route, icône, singleton de `jl`/`eu`) ;
- un champ réaffecté par tick ou par action doit, au choix de la PRD de sa famille et site par site :
  - être **réécrit en place**, quand la lecture prouve qu'aucun autre objet ne garde l'ancienne valeur ;
  - devenir une **valeur** au lieu d'une boîte, quand la boîte n'est jamais modifiée en place. Le cart en modifie trois en place : `self.ra.y+=` (l. 578), `p.v.x=`/`p.v.y=` (l. 699-700), `self.p.x=`/`self.p.y=` (l. 1141) ;
  - aller en **zone frame**, s'il n'est relu que dans la frame ;
  - prendre une **place** dans une zone qui rend.

Le classement de chaque site est l'affaire de sa famille. 4a ne fait que les rattacher :

| Site | Fréquence | Famille |
|---|---|---|
| `pico8_qb_ph` (`p`, `v`, `ra` de `mo`, trois boîtes) | par frame | 4h |
| `pico8_nf_ph` (`ra` de `je`) | par frame | 4h |
| `pico8_ui_lj` (`ok.qa`, gardé par les hôtes de `bv` et le point souris de `jc:hq`) | par frame | 4h |
| `pico8_ui_gw` (16 calques `ic` libérés puis refaits) | par frame | 4h |
| `pico8_bv` (`p` de l'hôte) | par frame où une sonde ou un hôte est tenu | 4h |
| `pico8_bg_hq` (boîte `v`) | par frame de glissé | 4h |
| `pico8_jc_hq` (point souris et sa boîte `p`) | par frame où un slipway est tenu | 4e |
| `pico8_jc_ch` (traînée `ts`, `pico8_ts_t` de 20 o, place de 24) | par frame, tirage `rnd` 0,08 par sens | 4e, en zone particules |
| `bd_reset_visit` (`pt.cc={}` pour chaque planète, `free`+`malloc`) | à chaque `bd`, donc chaque `hy:lj` (action, année, invention), en boucle tant que `bb()` | 4d |
| `pico8_jo_ib` (`ib`, `lp`, `rl`) | à chaque `bd`, pour chaque planète | 4d |
| `pico8_jo_cn` (liste fusionnée) | à chaque calcul de route | 4d |
| `pico8_trade_kg` (`lp`, `rl` concaténées) | à chaque `kg` | 4e |
| `pico8_ie_hb` (boîte de `ie.gy`) | à chaque `hb`, donc chaque `hy:lj` | 4g |
| `pico8_ie_ldown` (identifiant « s… ») | à chaque clic d'icône | 4g |
| `pico8_jo_lz`, `pico8_jo_lz_mw_step` (copies de `hp.i`) | à la pose et à chaque amélioration d'une planète | 4d |

### Ordre des familles 4b et suivantes

Le cliquet, mesuré par `make alloc-guard` sur une copie `git archive HEAD` à e88df7a : **64 objets, 24 allouent, 84 appels**, égal à `tools/alloc_guard_exceptions.txt`. Les relocations ont été rattachées à leur fonction et à leur ligne par `objdump -r` et `addr2line` sur les objets `-g` d'`alloc-guard`. Par rapport à 7255c72 (91 appels), il manque exactement les 7 que l'étape 3 devait retirer :

- `pico8_f_` 2 ;
- `pico8_g__mm` 1 ;
- `pico8_ga_mm`, `pico8_ga_struct_new`, `pico8_pm_pq` 3 ;
- le `malloc` de `pico8_jo_pq`, devenu `pico8_jo_pq_dans`. Reste la référence à la transitoire de la forme `pico8_jo_pq`, qui délègue (`planet` 192).

Les 3 de `pico8_zone_transitoire.c` vont à l'étape 5. **L'étape 4 porte donc 81 relocations**, toutes attribuées ci-dessous (lignes à `HEAD` e88df7a).

| Ordre | Famille | Sites (fonction, ligne) | Relocations | Reçus traités |
|---|---|---|---|---|
| **4b** | **États de `pico8_gen.c`** | `pico8_jf` (9), `pico8_jf_free` (26), `pico8_gn` (50), `pico8_gn_free` (124) ; leurs appelants `bw`, `jo:pq` et `bg:pq` choisissent la zone | `gen` 4 → sort | *Reçu 1* : `oh` de `pico8_bg_pq` en zone frame |
| **4c** | **Tableaux croissants et registres** | `pico8_entity_init` (78, 82, 86 : `hv`, `cv`, `cg`) ; `pico8_ui_init` (145 : `g_shared_le`) ; `pico8_planet_family_init` (531 : `g_ga_by_shared`) ; crée la sous-zone chargement | `entity` 3 → sort ; `ui` 6 → 5 ; `planet_family` 6 → 5 | *Reçu 2* : la liste statique de départ |
| **4d** | **Planètes et relais** | `pico8_jo_struct_new` (102), `pico8_jo_pq` (192 : la transitoire passée à `pico8_jo_pq_dans`), `pico8_jo_lz_mw_step` (219), `pico8_jo_lz` (242, 244, 246) ; `pico8_qh/nl/os/hj/pm_struct_new` (62, 123, 167, 247, 316) ; `pico8_nr/fr/mx_struct_new` (`planet_family_trade` 191, 237, 331) ; `pico8_jo_cn` (127, 129), `pico8_jo_ib` (188, 189, 215, 216, 218, 219) ; `pico8_bw` (63, 65, 71 : `p`, `ra`, identifiant « p… ») ; `bd_reset_visit` (48, 51, 52) ; refait le relevé « éclats `ga` rendus en cours de partie » pour `jo:hu` | `planet` 6 → sort ; `planet_family` 5 → sort ; `planet_family_trade` 3 → sort ; `planet_trade` 8 → sort ; `galaxy` 3 → sort ; `economy_tiers` 3 → sort (28) | *Reçu 2* : les structures de planète ; *Reçu 3* : `oe` d'une planète ordinaire par `pico8_jo_pq_dans` (il suffit de lui passer la zone partie) |
| **4e** | **Slipways et routes** | `pico8_jc_struct_new` (344), `pico8_jc_pq` (89, 90 : `ts`), `pico8_jc_hq` (111, 112) ; `pico8_jc_ch` (`slipway_display` 36 : traînée, vers la zone particules) ; `pico8_trade_struct_new` (29), `pico8_trade_kg` (135-138) ; refait le relevé « éclats `ga` » pour `jc:hq`/`re`/`et` | `slipway` 5 → sort ; `slipway_display` 1 → sort ; `trade` 5 → sort (11) | *Reçu 1* : `re`/`p` de `jc:hq` ; *Reçu 4a* : traînées `ts` |
| **4f** | **Trous de ver** | `pico8_mz_struct_new` (204), `pico8_mz_pq` (39, 40 : `ox`) | `wormhole` 3 → sort | — |
| **4g** | **Recherche** (et économie) | `pico8_hp_struct_new` (133, 200, 201), `pico8_jy_new` (119), `pico8_research_free_gb_instances` (33), en sous-zone chargement ; `pico8_ie_struct_new` (203), `pico8_ie_hb` (100), `pico8_ie_ldown` (130) ; `pico8_eu` (`cart` 222 : `ie.jb`) ; `pico8_hy_struct_new` (`economy` 36, 62, 63) ; `fm_free` (`probe` 453) | `research` 5 → sort ; `research_icon` 3 → sort ; `cart` 1 → sort ; `economy` 3 → sort ; `probe` 3 → 2 (13) | *Reçu 1* : `ie.gy` ; *Reçu 2* : `ie.jb`, `hp.i` |
| **4h** | **Singletons de `jl`/`eu` et objets du joueur** | `pico8_qb_struct_new` (41), `pico8_qb_ph` (122, 124, 129) ; `pico8_nf_struct_new` (39), `pico8_nf_ph` (96) ; `pico8_ui_struct_new` (119, 129), `pico8_ui_shutdown` (156), `pico8_ui_lj` (235), `pico8_ui_gw` (295) ; `pico8_bg_struct_new` (54), `pico8_bg_hq` (208) ; `pico8_fv_struct_new` (`help` 117) ; `pico8_pj_struct_new` (`score_panel` 263) ; `pico8_g__struct_new` (`probe` 413), `pico8_bv` (`probe` 162) | `scroll` 4 → sort ; `mouse_reader` 2 → sort ; `ui` 5 → sort ; `bg` 2 → sort ; `help` 1 → sort ; `score_panel` 1 → sort ; `probe` 2 → sort (17) | *Reçu 1* : `p`/`v`/`ra` de `qb`, `mo->v`, `ra` de `nf`, `p` de `pico8_bv` ; *Reçu 2* : `ok.qa`, la structure `ok` et ses calques `ic`, `fv`, la structure `pj` ; *Reçu 3* : la structure `g_` (et `fv`) et sa boîte `p` de `bv` (les `rendre` de `g_` et `fv` existent déjà et ne rendent que leur `qy`) |
| | **Total** | | **4 + 5 + 28 + 11 + 3 + 13 + 17 = 81** | |

- **Pourquoi cet ordre.**
  - **4b** est la plus petite famille (4 relocations, moins de 2 Ko). Ses objets sont faits dans `eu` et rendus par `bw` au `eu` suivant. Elle éprouve la bascule sur de vrais objets avec le seul motif délicat de l'inventaire : un ancien objet rendu par son remplaçant, encore vivant.
  - **4c** vient ensuite parce que toutes les autres familles inscrivent leurs objets dans `hv`/`cv`/`cg`. Elle fixe aussi la tenue des tableaux croissants dans une zone qui ne rend pas : un agrandissement laisse un bloc mort jusqu'à la génération suivante, ce qui coûte au plus le double de la taille finale. Enfin, elle crée la sous-zone chargement, dont 4g aura besoin.
  - **4d** est le plus gros poste (planètes générées par `bw(90)`, `bd` par action). On le mesure tôt, pour que la projection du budget repose sur une mesure et non sur une estimation.
  - **4e** dépend de 4d en lecture seulement (un `jc` désigne ses planètes). Elle apporte la seule sorte de particules nouvelle, à ranger dans les ~17 Ko libres de la zone particules (`ts` : 24 o par place, de l'ordre de quelques centaines de traînées vivantes, donc quelques Ko).
  - **4f** est petite, avec les mêmes motifs que 4e.
  - **4g** touche le chargement et `fm`, que l'inventaire désigne comme vivant entre `_init` et `eu`.
  - **4h** est la plus difficile : boîtes par frame partagées entre objets (`ok.qa` gardé par les hôtes de `bv`), `ok` encore lu après `jl()`, sondes tenues. Elle passe en dernier. Elle donne aussi le critère de fin de l'étape 4 : `pause_restart` sous empoisonnement avec tous les singletons en zone.
- **Une famille de plus que la liste du plan : 4h. C'est un changement du plan**, écrit dans l'entrée 4 à la clôture de 4a. Le plan énumère six familles, et aucune ne couvre les singletons `mo`/`je`/`ok`, `bg`, `fv`, `pj` ni les sondes `g_`. Ce sont pourtant ceux que les lignes « Reçu » des étapes 1, 2 et 3 renvoient à l'étape 4. Le *Reçu de l'étape 3* (structure `g_` et `fv`, boîte `p` de `bv`) n'a d'autre place. Les ranger dans « tableaux croissants et registres » mêlerait les boîtes par frame, le cas le plus dur, à la famille qui doit ouvrir la voie.
- **Fichiers partagés entre familles.**
  - `ui` : 4c puis 4h.
  - `planet_family` : 4c puis 4d.
  - `probe` : 4g puis 4h.

  Chacun ne sort de la liste qu'avec sa dernière famille.
- La colonne « Relocations » est **mesurée** à e88df7a. Les baisses par famille restent des prévisions, puisque le verrou compte des relocations qui dépendent du compilateur. Chaque famille repart de la mesure à son `HEAD` et note l'écart dans « Découvert ».

### Suivi du budget, famille par famille

**Estimation par lecture**, x86-64, pic d'une session e2e. La bascule est comprise : partie quittée + titre + première frame de la nouvelle partie, soit environ deux parties à la fois. Les tailles viennent de `sizeof` relevés à `HEAD` e88df7a : `pico8_jo_t` 440 (432 avant l'étape 3), `pico8_g__t` 264 (256), `pico8_jc_t` 960, `pico8_trade_t` 160, `pico8_hp_t` 576, `pico8_hy_t` 472, `pico8_ui_t` 584, `pico8_qb_t` 168, `pico8_nf_t` 144, `pico8_bg_t` 152, `pico8_pj_t` 168, `pico8_ie_t` 176, `pico8_mz_t` 224, `pico8_gn_state_t` 592, `pico8_jf_state_t` 8, `pico8_jy_t` 48, `pico8_ents_t`/`pico8_ptrs_t` 24, `pico8_vec_t` 8. Une allocation est arrondie à 8 o.

| Famille | Par génération | Pic estimé (bascule) |
|---|---|---|
| 4b états | ~0,6 Ko | ~2 Ko |
| 4c registres | `hv` ~200 entrées, 5 `cv`, les `cg`, blocs morts des agrandissements : ~10 Ko | ~20 Ko |
| 4d planètes | 90 planètes × ~0,9 Ko (struct, `p`/`ra`/id, `oe`, copies `jy`, `cc`/`by`/`ib` une fois les sites par action réglés) ≈ 80 Ko ; les éclats `ga` n'y sont plus (zone particules) | ~160 Ko |
| 4e slipways et routes | ≤ 10 slipways × ~1,1 Ko + routes ≈ 15 Ko | ~30 Ko |
| 4f trous de ver | < 1 Ko | ~1 Ko |
| 4g recherche | `hy`, 7 `ie` et leurs `jb` ≈ 3 Ko ; chargement : ~36 `hp` × 576 o + `hp.i` ≈ 32 Ko, hors bascule | ~38 Ko |
| 4h singletons | `qb`, `nf`, `ui` et ses 16 calques, `bg`, `fv`, `pj`, sondes ≈ 4 Ko, une fois de plus par `_init` | ~12 Ko |
| **Total** | | **~265 Ko** sur 768 |

Cette table est un ordre de grandeur et une base de projection, pas une limite (§3). Les sites par action sont supposés réglés selon la règle ci-dessus : s'ils ne l'étaient pas, la mesure le montrerait sur les e2e qui enchaînent des actions (`game_end_final_score` saute 25 années).

**Méthode, appliquée par chaque famille k (4b à 4h) :**

1. Après sa conversion, rejouer les 26 e2e avec `PICO8_REMPLISSAGE`. Relever le maximum de la zone partie sur l'ensemble (`M_k`), et celui de la sous-zone chargement à partir de 4c. Le noter dans le message du commit et dans « Découvert ». La forme est laissée à l'implémenteur : boucle de shell dans le commit, ou cible `make` hors suite comme `class-coverage`.
2. **Projection** : `P_k = M_k + Σ` (estimations des familles restantes) `× r`. `r` vaut le rapport « mesuré / estimé » des familles déjà faites, s'il dépasse 1, et 1 sinon. Une famille dont la mesure dépasse son estimation fait monter la projection des suivantes.
3. **Décision** :
   - `P_k ≤ PICO8_ZONE_GAME_BYTES` (768 Ko, sous-zone chargement comprise) : on continue ;
   - `768 Ko < P_k ≤ 768 Ko + marge`, avec marge = 1536 − (frame + interface + particules) = **49 Ko** à e88df7a : la famille relève `PICO8_ZONE_GAME_BYTES` d'un chiffre, dans son commit, et le dit. Les ~17 Ko libres dans les 128 Ko de la zone particules n'entrent pas dans cette marge : ils restent aux particules (traînées `ts` de 4e), et les reprendre serait rogner une autre zone (§3). Si 4e a besoin de relever la zone particules pour `ts`, ce relèvement est pris sur les 49 Ko, et la projection des familles suivantes le compte ;
   - `P_k` au-delà : **arrêt** de l'étape 4 à la famille k, et signalement à l'humain (zone partie, limite voulue = `P_k` arrondi, somme actuelle des zones). On ne rogne ni la frame, ni l'interface, ni les particules (§3), et on n'attend pas la dernière famille.
4. **Croissance** : à chaque famille, le TU des places vivantes vérifie que les octets de la zone partie sont égaux aux frames 45 et 90 de `message_box_hold`. Une hausse est une allocation par tick de la famille. Elle se corrige dans la famille, pas en relevant la limite (§3).
5. Un `pico8_fatal` « zone partie pleine » en cours de famille se lit d'abord comme un site par tick ou par action entré tel quel, puis comme une limite trop basse. La note « Découvert » dit lequel.

La sous-zone chargement suit la même méthode à part. Son maximum ne dépend pas de la bascule.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 : `make test-all`, `make sanitize-net`, 26 `*.sha256` identiques. `alloc-guard` reste à égalité stricte, avec des comptes inchangés.

1. **Zone à deux bouts.**
   - La primitive dans le module de zone générique : allouer au bout courant, basculer, rendre l'ancienne, remise complète, octets pris et maximum, plein → `pico8_fatal`, empoisonnement sous ASan.
   - Sa poignée `pico8_zone_choisie_t`.
   - Leurs TU, sous ASan.
2. **Module de zone partie, bascule dans `eu`, rendu au vidage.**
   - Le module, son stockage de `PICO8_ZONE_GAME_BYTES`, son initialisation paresseuse.
   - La bascule en tête de `pico8_eu` et le rendu dans `pico8_cart_vider_file_sortie()`, après le vidage de la file, avec le point d'oubli de partie (vide).
   - La remise complète dans `pico8_cart_load()`/`pico8_cart_shutdown()`.
   - Les listes de sources du `Makefile`.
   - Les aides de TU qui enchaînent deux `eu` sans vidage, corrigées pour vider la file (au moins `test_pico8_hy`, cas « construction »).
3. **Remplissage écrit par le rejeu, et preuves sur rejeu.**
   - La ligne de la zone partie sous `PICO8_REMPLISSAGE`.
   - Le TU de la sentinelle sur `pause_restart`.
   - Le relevé des octets sur `message_box_hold` (f45 = f90).
   - Le contrôle du maximum par un rejeu.
4. **Clôture de 4a.**
   - La note « Découvert » sous 4a : point de bascule et de rendu, et pourquoi pas `jl()` ni `_init` ; inventaire et conclusion, éclats `ga` compris ; ordre des familles et attribution des 81 relocations ; méthode de budget.
   - Les changements du plan (voir « Changements du plan ») : lignes « Partie » et « Particules » du §2, famille 4h et ordre des familles dans l'entrée 4, ligne « *Reçu de l'étape 4a* » sur les traînées `ts`.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris et le maximum rendus par l'accesseur ;
  - un code de sortie, un message de `pico8_fatal`, un rapport ASan dans un processus enfant ;
  - la ligne écrite sous `PICO8_REMPLISSAGE` ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne lit ni les bouts internes de la zone ni une adresse, sauf pour la relire après son rendu.
- **TU de la zone à deux bouts**, dans `tests/test_pico8_zone.c` (règle ASan, `UNIT_RUNS_SPECIAL`, `ASAN_OPTIONS=detect_leaks=0`, sur le modèle des cas existants de zone montante et de zone à places) :
  - les octets pris suivent les allocations, arrondies à la cellule ;
  - après une bascule, un bloc de l'ancienne génération se lit encore et un nouveau bloc ne le recouvre pas ;
  - après le rendu de l'ancienne, ce bloc relu plante, **et plante encore après que la nouvelle génération a servi de nouvelles allocations** (critère de 0a) ;
  - une seconde bascule sans rendu échoue par l'`assert` (enfant) ;
  - les deux bouts qui se rejoignent sortent en 3 avec « zone <nom> pleine, limite N octets » (enfant) ;
  - le maximum garde le pic de la frame de bascule après le rendu ;
  - la remise complète rend tout et empoisonne tout.
- **TU de la poignée** dans `tests/test_pico8_zone_choisie.c` : `alloc`, `grow` (copie, l'ancien bloc reste lisible jusqu'au rendu de sa génération), `release` sans effet.
- **TU de la sentinelle, sur rejeu**, dans `tests/test_pico8_places_vivantes.c`, piloté comme `main.c`, un processus enfant par cas, sur `pause_restart` :
  - après la frame 11 (titre, après « restart » en f7), une sentinelle est prise dans la zone partie ;
  - la frame 12 (clic « challenging », donc `eu`) est jouée : la sentinelle se relit sans crash, et les octets pris l'incluent encore ;
  - la frame 13 est jouée, puis la sentinelle est relue : l'enfant doit mourir par ASan ;
  - une variante montre que « restart » ne bascule pas : une sentinelle prise après la frame 6 se relit encore après la frame 11.

  **Piège** : la fonction `relever` de ce TU vide la file avant de lire les comptes (`pico8_cart_vider_file_sortie()`), et ce vidage devient le point de rendu. Entre f12 et f13, la sentinelle doit donc être relue **sans** passer par `relever`, sinon le TU rend la génération lui-même et prouve un autre point que celui du jeu.
- **Croissance par frame** : le même TU relève les octets de la zone partie aux frames 45 et 90 de `message_box_hold` et les trouve égaux. En 4a, la zone est vide et le relevé est trivial. Il est posé ici pour que chaque famille hérite du contrôle.
- **Remplissage écrit par le rejeu** : sur le modèle des cas `le_rejeu_ecrit_le_remplissage_en_fin_de_session` et `le_rejeu_ecrit_le_pic_de_la_session` de `tests/test_pico8_places_vivantes.c`, un rejeu avec `PICO8_REMPLISSAGE` écrit `partie 0` et `max_octets partie 0` en 4a sans changer la capture ni le code de sortie. Le cas « maximum > octets finaux » est porté par le TU de la zone à deux bouts, puisqu'aucun objet du jeu n'y entre encore.
- **Tests existants** : verts, sans changement d'attente. Seule adaptation permise : vider la file entre deux `eu` dans les aides de TU qui ne jouent pas de frame. L'`assert` les désigne.
- **Verrou** : `make alloc-guard` à égalité stricte, comptes inchangés. Pas de nouveau cas dans `tests/test_alloc_guard.c`.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. En 4a, rien n'est empoisonné en jeu, et `sanitize-net` vert ne prouve que l'absence de régression. La preuve du point de rendu est le TU de la sentinelle.
- **Exemples dans le dépôt** : `tests/test_pico8_zone.c`, `tests/test_pico8_zone_frame.c`, `tests/test_pico8_zone_choisie.c`, `tests/test_pico8_zone_particules.c` (lecture périmée dans un enfant, zone pleine en 3) ; `tests/test_pico8_places_vivantes.c` (rejeu frame par frame dans un enfant, relevés, remplissage écrit par le rejeu) ; `src/main.c` (`write_remplissage`).

## Out of Scope

- Convertir un propriétaire, quel qu'il soit : c'est le travail de 4b à 4h. Aucun compte du cliquet ne bouge en 4a.
- Créer la sous-zone chargement : décidée ici, créée par 4c.
- Classer site par site les allocations par tick ou par action : chaque famille le fait pour les siens, selon la règle de 4a.
- Ajouter la sorte `ts` au module de particules : famille 4e.
- Relever `PICO8_ZONE_GAME_BYTES` : aucune mesure ne l'exige en 4a.
- Le maximum de remplissage des sortes d'interface : non rétrofité (comme à l'étape 3).
- Retirer TROU-7 : étape 5.
- Changer le comportement du jeu : `fm`/`ju` gardés au titre, triche au titre, `hq` sur l'ancien `rheld` après `jl()`.
- Afficher un budget de slipways (§7).
- Lancer l'application raylib.

## Vérifié contre l'étape 3

Relu contre `HEAD` e88df7a :

- la note « Découvert » de l'étape 3, la ligne « *Reçu de l'étape 3* » sous l'entrée 4 et la ligne « Particules » corrigée du §2 (`docs/plan-4.md`) ;
- `src/pico8_zone_particules.{h,c}`, `src/pico8_cart.c` (`oublier`, `pico8__update60`, `pico8_eu`), `src/pico8_planet_family.c` (`pico8_ga_rendre_dispatch`, classe `ga`), `src/pico8_planet.c` (`pico8_jo_pq`, `pico8_jo_pq_dans`), `src/pico8_probe.c` et `src/pico8_help.c` (`rendre` de `g_` et `fv`), `src/main.c` (`write_remplissage`), `tests/test_pico8_places_vivantes.c`, `tests/test_pico8_hy.c` ;
- `tools/alloc_guard_exceptions.txt`, et `make alloc-guard` sur une copie `git archive HEAD` : 64 objets, 24 allouent, 84 appels ;
- `git log -10 --stat` (5fb575c à e88df7a).

Statut de chaque hypothèse du brouillon :

- **H-3-1 (comptes laissés) — tenue.** 24 fichiers, 84 appels. `pico8_particle.c` est sorti ; `probe` 3, `planet_family` 6, `planet` 6. Les 84 relocations relevées sont exactement les 91 de 7255c72 moins les 7 prévues (`pico8_f_` ×2, `g__mm`, `ga_mm`, `ga_struct_new`, `pm_pq`, le `malloc` de `jo_pq`). Les numéros de ligne ont bougé : la table des familles donne ceux de e88df7a. L'attribution reste 4 + 5 + 28 + 11 + 3 + 13 + 17 = 81.
- **H-3-2 (budget) — tenue, précisée.**
  - `PICO8_ZONE_PARTICLES_BYTES` reste à 128 Ko : somme 1487 Ko sur 1536, marge de 49 Ko sur le total.
  - Précision : la zone particules n'occupe que 113 822 o (111 Ko). Ses ~17 Ko libres ne sont pas une marge pour la zone partie (§3 : ne pas rogner une autre zone). Ils servent aux traînées `ts` de 4e (voir « Suivi du budget »).
- **H-3-3 (format du remplissage) — tombée sur la forme.**
  - `write_remplissage` écrit `<sorte> <n>` pour les vivantes, puis le maximum sur des lignes préfixées : `max_vivantes <sorte> <n>`, `max_octets <sorte> <n>`, `max_elements <sorte> <n>`. Ce n'est pas une ligne « nom, courant, maximum ».
  - La zone partie suit cette forme : `partie <octets>` et `max_octets partie <n>` (« Remplissage écrit par le rejeu »).
- **H-3-4 (sorties de l'étape 3 vers l'étape 4) — tenue, avec un écart.**
  - `g_` et `fv` ont `rendre` (`pico8_g__rendre_dispatch`, qui ne rend que leur `qy`) et passent par la file.
  - Les `rendre` de `hj`, `pm`, `mx` lisent la struct (rangée, puis `qy`).
  - `jo:pq` a sa variante `pico8_jo_pq_dans`, et la forme `pico8_jo_pq` garde la référence à la transitoire (`planet` 192, attribuée à 4d).
  - L'éclat de `jo:hu` emprunte `p`/`ra`/`id`.
  - **Écart** : l'éclat `ga` est désormais **entièrement** en zone particules (structure, `oe`, copie de position) et **rendu en cours de partie**. Ce n'est plus un objet de partie : il sort de 4d. Les objets de partie qui pourraient le désigner sont relevés dans l'inventaire (« Éclats `ga` rendus en cours de partie »).
  - Le *Reçu de l'étape 3* dit « la structure `g_` (et `fv`) et sa boîte `p` de `bv` » et « `oe` d'une planète ordinaire passe par `pico8_jo_pq_dans` ». Les deux sont rattachés (4h et 4d).
- **H-3-5 (inventaire) — tombée.**
  - L'étape 3 a ajouté à `oublier` la remise à nil du `re` du `jc` tenu (`ok.lheld`), quand il vise l'objet rendu. C'est un trou de son inventaire, trouvé à la vérification : un éclat se survole.
  - Un pointeur statique est apparu (`g_arenes`), avec deux structures (`g_zones`, `g_slots` des particules). Le motif du `grep` a été élargi aux tableaux à plusieurs dimensions pour le voir.
  - Tous sont classés dans l'inventaire refait à e88df7a. Aucun ne change la conclusion : point d'oubli de partie vide.
- **H-3-6 (TU) — tenue en partie.**
  - `tests/test_pico8_places_vivantes.c` rejoue frame par frame dans un enfant, relève les comptes, et sait vérifier le remplissage et le pic écrits par le rejeu.
  - Mais toutes les aides de TU ne vident pas la file entre deux `eu` : `test_pico8_hy` (cas « construction ») enchaîne deux `pico8_eu` sans vidage. Le morceau 2 de 4a le corrige.
  - Nouveau piège pour le TU de la sentinelle : `relever` vide la file, et ce vidage deviendra le point de rendu (Testing Decisions).
- **H-3-7 (traînées `ts`) — tenue.** `pico8_jc_ch` fait toujours son `malloc` (`slipway_display` 36). La ligne « Particules » corrigée par l'étape 3 ne nomme pas `ts`. 4a l'ajoute aux changements du plan et confie `ts` à 4e.
- **H-3-8 (file de sortie) — tenue.**
  - Capacité 256.
  - `pico8__update60()` fait toujours : remise de la zone frame, vidage, `pico8_dg()`, `pico8_ui_lj()`.
  - Le vidage est aussi appelé par `pico8_cart_load()`, `pico8_cart_shutdown()` et `main.c`.
  - Le commentaire de `pico8_zone_particules.c` compte d'ailleurs déjà « les 5 de la nouvelle aide, nées au `_init` avant le vidage qui rend l'ancienne partie » : c'est l'ordre sur lequel s'appuie le rendu de la génération.

## Further Notes

- **Pas d'humain disponible, pas d'outil `Agent` dans cette session.** Les points que `to-prd` fait valider par un humain (modules attendus, tests voulus, découpage) ont été tranchés en appliquant moi-même le skill `guideline` :
  - **Hypothèses explicites** : celles du brouillon (H-3-1 à H-3-8) ont été vérifiées contre l'étape 3 finie (section « Vérifié contre l'étape 3 ») ; l'inventaire du §4 a été refait par `grep` à e88df7a ; les quatre lectures après `jl()` ont été relevées dans le code et dans le cart (l. 217-247, 1178-1186).
  - **Simplicité** :
    - une seule primitive nouvelle (la zone à deux bouts) et un seul module ;
    - pas de sous-zone chargement tant qu'aucun propriétaire n'y entre ;
    - un point d'oubli de partie vide, posé parce que les familles en auront besoin au même endroit que le rendu ;
    - pas de seconde moitié ASan.
  - **Changements chirurgicaux** :
    - deux appels dans `pico8_cart.c` (bascule dans `eu`, rendu dans le vidage) et la remise complète au chargement et à l'arrêt ;
    - une ligne de plus dans `write_remplissage` ;
    - aucun site du jeu touché.
  - **Critères vérifiables** :
    - comptes du cliquet inchangés ;
    - `*.sha256` inchangés et `sanitize-net` vert ;
    - la sentinelle de `pause_restart` (vivante en f12, morte après f13, vivante à travers « restart ») ;
    - f45 = f90 sur `message_box_hold` ;
    - la ligne du rejeu.
- **Alternatives écartées pour le point de rendu.**
  - *Remise dans `jl()`* : elle tue `fm`/`ju` au titre, la fin de `ok:lj` et les `rendre` de la file.
  - *Remise différée à la frame suivante, sur une zone à un bout* : les objets que `eu` construit dans la frame du clic seraient pris dans la génération qu'on va vider.
  - *Deux zones de 768 Ko en alternance* : 1,5 Mo à elles seules.
  - *Rendre tout de suite les entités abandonnées par `jl()`* : `ok.le` et les calques de l'ancien `ok` les relisent dans la même frame.

  La zone à deux bouts ne coûte rien de plus que la zone : seule la somme des deux générations doit tenir dans la frame de bascule, et le maximum la mesure.
- **Ce que la bascule à `eu` change au budget.** Le pire moment n'est pas une partie longue mais la frame qui suit une partie longue : partie quittée, titre et nouvelle galaxie ensemble. Le maximum relevé par le rejeu le couvre sur `pause_restart`. Pour une partie plus longue que les e2e, le §6 accepte de le découvrir par un `pico8_fatal` qui nomme la zone.
- **Le critère de fin de l'étape 4** (« `pause_restart` passe `make sanitize-net` sous cet empoisonnement ») tombe sur la bascule de f12 et le rendu de f13. Le TU de la sentinelle montre dès 4a que ce sont bien les frames où la génération de la première partie (et du titre de f7) est rendue.
- À la fin de 4a, la note « Découvert » sous l'entrée 4 du plan porte au minimum :
  - le point de bascule (`eu`) et de rendu (vidage suivant), et pourquoi ni `jl()` ni `_init` ;
  - l'angle mort ASan (deux nouvelles parties) ;
  - la conclusion de l'inventaire (point d'oubli vide), éclats `ga` rendus en cours de partie compris ;
  - l'ordre 4b → 4h, la famille 4h ajoutée et pourquoi ;
  - l'attribution des 81 relocations, mesurée à e88df7a, et l'écart à la mesure du `HEAD` de clôture ;
  - la règle des sites par tick ou par action ;
  - la méthode de budget et l'estimation de départ (~265 Ko) ;
  - les changements du plan écrits dans le même commit (§2 Partie et chargement, §2 Particules et `ts`, famille 4h et ordre des familles, *Reçu de l'étape 4a*).
