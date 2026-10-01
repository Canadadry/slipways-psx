---
title: "Plan 4, étape 4a — Socle de la zone partie : bascule sur nouvelle partie, remplissage, inventaire, ordre des familles"
description: "La zone partie n'existe que comme une taille dans pico8_zone_sizes.h : 4a lui donne un module vide, une remise sur nouvelle partie qui ne tue rien de ce que la frame relit encore, son empoisonnement ASan, un accesseur de remplissage écrit par le rejeu, l'inventaire du §4 des pointeurs qui survivent à une nouvelle partie, et l'ordre des familles 4b et suivantes, sans convertir aucun propriétaire."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. Le texte de l'étape, recopié du plan (relu à `HEAD` 7255c72) :

> 4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement, et si son remplissage ne monte pas pendant une phase sans action d'un e2e.
>    Elle se fait en sous-étapes, une PRD chacune :
>    - **4a. Socle.** La zone partie, vide, avec sa remise à zéro et son empoisonnement sur nouvelle partie, son accesseur de remplissage (octets pris, et maximum, §3) écrit par le rejeu, et l'inventaire du §4 des pointeurs qui survivent à une nouvelle partie. Aucun propriétaire converti.
>    - **4b et suivantes.** Une famille de propriétaires par PRD (tableaux croissants et registres ; planètes et relais ; slipways et routes ; recherche ; états de `pico8_gen.c` ; trous de ver), dans l'ordre que fixe la PRD 4a. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
>    - *Reçu de l'étape 1 :* même chose pour les boîtes `vec` des objets de partie (`p`/`v`/`ra` de `qb`, `ra` de `nf`, `mo->v`, `re`/`p` de `jc:hq`, `p` de `pico8_bv`, `ie.gy`) ; `qb:ph` seul en fait trois par frame. L'état `oh` de `pico8_bg_pq` est un objet de frame fait par `pico8_jf`, qui ne prend pas de zone : l'étape 4, qui donne une zone aux états de `pico8_gen.c`, le range en zone frame.
>    - *Reçu de l'étape 2 :* restent à cette étape `ok.qa` (gardé par les hôtes de `bv` et le point souris de `jc:hq` : objet relu par des objets de partie), la structure `ok` et ses calques `ic`, la liste statique de départ, `ie.jb` (fait une fois par partie), `fv`, la structure `pj`, `hp.i`. Les structures de planète ne sont toujours pas libérées (seule leur rangée l'est).

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

- La zone partie n'est qu'une taille : `PICO8_ZONE_GAME_BYTES` vaut 768 Ko dans `pico8_zone_sizes.h`. Elle compte dans le contrôle de somme : frame 256 + interface 335 + particules 128 + partie 768 = **1487 Ko sur 1536**, soit **49 Ko de marge**. Aucun module ne la possède et rien n'y alloue.
- Les objets de partie sont pris à la libc ou à la transitoire et jamais rendus (TROU-7).
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
2. **Le vidage de la file**, en tête du `pico8__update60()` suivant. Il appelle `rendre` sur les entités abandonnées par `jl()`. `pico8_jo_rendre_dispatch` lit la struct de la planète (`lq_place`) ; après l'étape 3, il lit aussi son `qy` (H-3-4). `oublier` lit `ju->ed`.
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
- **Une durée de vie absente du §2 : le chargement.** Les projets de recherche `gb` (avec `hp.i` et ses `jy`) et deux listes statiques (`g_shared_le`, `g_ga_by_shared`) sont faits par `pico8_cart_load()` et vivent d'une partie à l'autre. Ils ne peuvent pas entrer dans une génération. 4a corrige la ligne « Partie » du §2. La famille qui en a besoin la première crée une sous-zone **chargement**, prise dans `PICO8_ZONE_GAME_BYTES` et remise à zéro seulement par le chargement du cart.
- **Une sorte de particules absente du §2 : les traînées `ts` de `jc:ch`.** Elles naissent par tirage `rnd` à chaque frame et meurent par `del(self.ts,t)`. Ni le §2 ni la PRD de l'étape 3 ne les nomment. Dans une zone partie montante, elles feraient croître le remplissage à chaque tick. 4a corrige la ligne « Particules » du §2 et les confie à la famille slipways, qui les range en zone particules avec un point de libération.
- **L'ordre des familles 4b à 4h**, l'attribution de chacune des 81 relocations du cliquet qui restent à l'étape 4, et la **méthode de suivi du budget** (relevé, projection, arrêt anticipé).

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
21. En tant que mainteneur du budget, je veux que `PICO8_ZONE_GAME_BYTES` reste à 768 Ko en 4a et que le contrôle de somme reste vert, pour que la marge de 49 Ko (H-3-2) reste aux familles.
22. En tant que développeur des familles 4b+, je veux l'inventaire du §4 des pointeurs du cœur qui survivent à une nouvelle partie, chacun classé, pour savoir lesquels mon commit doit remettre à nil ou repointer, et ne pas refaire l'erreur de `dk`.
23. En tant que développeur des familles 4b+, je veux que chaque famille refasse le même `grep` sur le code de son commit et classe tout pointeur nouveau, pour qu'un pointeur trouvé ensuite par ASan soit noté comme un trou de l'inventaire.
24. En tant que développeur des familles 4b+, je veux un point d'oubli de partie, appelé juste avant le rendu de l'ancienne génération, où une famille ajoute la remise à nil d'un pointeur que son inventaire désigne, pour qu'il n'existe qu'un seul endroit où une référence à l'ancienne partie est coupée.
25. En tant que lecteur du plan, je veux que la ligne « Partie » du §2 dise le vrai point de rendu (le vidage qui suit `eu`) et la durée de vie « chargement » de `gb` et des listes statiques, pour que le tableau ne contredise plus le code.
26. En tant que lecteur du plan, je veux que la ligne « Particules » du §2 nomme les traînées `ts` de `jc:ch`, pour qu'aucune famille ne les range en zone partie, où elles croîtraient par tick.
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
- **Double bascule sans vidage.** Une bascule demandée alors qu'une génération attend encore est refusée par `assert`. Le message dit qu'il faut vider la file entre deux `eu`. Le jeu ne peut pas déclencher ce cas : un clic par frame, et `eu` n'est atteint que depuis l'écran titre. Les TU qui enchaînent `_init`/`eu` sans frame vident déjà la file depuis les étapes 2 et 3 (H-3-6). Ceux qui enchaînent deux `eu` sans vidage sont corrigés dans le commit de 4a.
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
- **Taille** : `PICO8_ZONE_GAME_BYTES` reste à **768 Ko**. Le contrôle de somme de `pico8_zone_sizes.h` ne change pas : 1487 Ko sur 1536 (H-3-2).
- **Sous-zone chargement : décidée ici, créée par la famille 4c.** C'est la première famille qui en a besoin (`g_shared_le`, `g_ga_by_shared`). Ce sera une zone montante à une moitié, remise à zéro seulement par `pico8_cart_load()`/`pico8_cart_shutdown()`, prise **dans** `PICO8_ZONE_GAME_BYTES`, avec une vérification à la compilation que chargement + partie ≤ 768 Ko. Taille de départ au doigt mouillé : 64 Ko, prise sur les 768 (partie 704 Ko). Elle ne reçoit que ce que fait le chargement. Ce qui y grandirait ensuite (`g_shared_le`, où `ok.le` ajoute des boutons) doit y réserver sa capacité au chargement, comme la liste `qy` de l'étape 3, ou aller ailleurs. 4a ne la crée pas : aucune poignée sans utilisateur.
- **Makefile** : le nouvel objet entre dans toutes les listes de sources qui lient `pico8_zone_frame.c` (`ENGINE_SRCS`, `DATA_ENGINE_SRCS` si `pico8_cart.c` y est, ligne de lien de `main_raylib`) et dans la règle ASan du TU du module.

### Remplissage écrit par le rejeu

- Sous `PICO8_REMPLISSAGE`, `main.c` écrit, après le vidage qu'il fait déjà, une ligne pour la zone partie : nom de zone, octets pris, maximum de la session. Elle suit la forme qu'auront prise les lignes de l'étape 3 (H-3-3). La capture, le dump et le code de sortie ne changent pas.
- Les octets pris après le vidage final ne comptent que la génération courante : l'ancienne vient d'être rendue. Le maximum, lui, garde le pic de la frame de bascule.

### Inventaire du §4 : pointeurs qui survivent à une nouvelle partie

Fait **mécaniquement** à `HEAD` 7255c72, sur les 63 fichiers de `CORE_SRCS` (liste tirée du `Makefile`) :

- `grep -nE '^\s*static\s[^(]*\*\s*[a-zA-Z_][a-zA-Z0-9_]*(\[[^]]*\])?\s*(;|=)'`, hors `const char *` et tables `* const` : 17 pointeurs ;
- `grep -nE '^static\s+(struct\s+)?[a-z0-9_]+(_t)?\s+[a-zA-Z_][a-zA-Z0-9_]*\s*(\[[^]]*\])*\s*(;|=)'` hors `const` et scalaires, dont on lit les champs pointeurs : 31 structures, plus un `static` de bloc (`copie`, `pico8_economy_dk_quitte`) ;
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
| zones et poignées (`frame`, `frame_handle`, `g_zones`, `g_handles`) | ne peuvent pas viser (type) | — |
| `ok.o_`, `ok.gj`, `ok.lheld`, `ok.rheld`, `ok.ic`, `ok.le`, `ok.qa` | champs du **nouvel** `ok`, faits à nil (ou `&g_shared_le`) par sa construction après la bascule ; l'ancien `ok` n'est plus atteint par aucun pointeur une fois `ok:lj` fini | 4h |
| `mo.v`/`.p`/`.ra`, `je.ra` | champs des nouveaux `mo`/`je` | 4h |
| `fm.invented`, `fm.po`/`.fu` | champs du nouvel `hy` | 4g |
| `ju.ed` | `mh` (zone interface) ; remis à nil par `oublier` | — |

**Objets d'autres zones qui désignent des objets de partie** (hors périmètre mécanique, relevés par lecture) :

- rangées, panneaux et boutons de la zone interface ;
- listes `qy` et éclats `ga` de la zone particules (H-3-4). Un éclat né par `jo:hu` emprunte le `p`, le `ra` et l'`id` de la planète remplacée.

Tous sont abandonnés par le `jl()` d'`eu` et rendus par le vidage, **avant** le rendu de la génération. Aucun ne survit au rendu.

**Conclusion : le point d'oubli de partie est vide en 4a.** Aucun pointeur ne lit l'ancienne génération après son rendu. Le seul cas « vivant mais désigné » est celui de `fm`/`ju` entre `_init` et `eu`. Il est réglé par le choix du point de bascule, pas par une remise à nil. Remettre `fm` au tenant-lieu changerait ce que lit la triche au titre.

**Dans chaque famille 4b+**, le même `grep` est refait sur le code du commit, avec la même question. Un pointeur nouveau y est classé. Un pointeur trouvé ensuite par ASan est ajouté au point d'oubli de partie et noté dans « Découvert » comme un **trou de l'inventaire** (§4).

### Corrections du §2 (dans le commit de clôture de 4a)

- **Ligne « Partie »**, durée de vie : « une génération par partie, de `eu` à `eu` ; rendue d'un bloc au vidage de la file qui suit le `eu` suivant (zone à deux bouts) ; `_init` ne la rend pas ». Ajout : « les objets faits au chargement (`gb`, `hp.i`, listes statiques) vivent dans une sous-zone chargement, prise dans la zone partie, remise à zéro par le seul chargement ».
- **Ligne « Particules »**, contenu : ajouter « traînées `ts` de `jc:ch` ». Une ligne « *Reçu de l'étape 4a* » sous l'entrée 4 les confie à la famille slipways (4e).

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
| `pico8_jc_ch` (traînée `ts`) | par frame, tirage `rnd` 0,08 par sens | 4e, en zone particules |
| `bd_reset_visit` (`pt.cc={}` pour chaque planète, `free`+`malloc`) | à chaque `bd`, donc chaque `hy:lj` (action, année, invention), en boucle tant que `bb()` | 4d |
| `pico8_jo_ib` (`ib`, `lp`, `rl`) | à chaque `bd`, pour chaque planète | 4d |
| `pico8_jo_cn` (liste fusionnée) | à chaque calcul de route | 4d |
| `pico8_trade_kg` (`lp`, `rl` concaténées) | à chaque `kg` | 4e |
| `pico8_ie_hb` (boîte de `ie.gy`) | à chaque `hb`, donc chaque `hy:lj` | 4g |
| `pico8_ie_ldown` (identifiant « s… ») | à chaque clic d'icône | 4g |
| `pico8_jo_lz`, `pico8_jo_lz_mw_step` (copies de `hp.i`) | à la pose et à chaque amélioration d'une planète | 4d |

### Ordre des familles 4b et suivantes

Le cliquet compte à `HEAD` 7255c72, mesuré par `make alloc-guard` sur une copie `git archive HEAD` : **63 objets, 25 allouent, 91 appels**, égal à `tools/alloc_guard_exceptions.txt`. Les relocations ont été rattachées à leur fonction et à leur ligne par `objdump -r` et `addr2line` sur les objets `-g` d'`alloc-guard`.

L'étape 3 doit en retirer 7 (H-3-1) :

- `pico8_particle.c` 2 ;
- `pico8_g__mm` 1 ;
- `pico8_ga_mm`, `pico8_ga_struct_new`, `pico8_pm_pq` 3 ;
- l'une des deux de `pico8_jo_pq` 1.

Il en restera 84. Les 3 de `pico8_zone_transitoire.c` vont à l'étape 5. **L'étape 4 porte donc 81 relocations**, toutes attribuées ci-dessous.

| Ordre | Famille | Sites (fonction, ligne à `HEAD`) | Relocations | Reçus traités |
|---|---|---|---|---|
| **4b** | **États de `pico8_gen.c`** | `pico8_jf` (9), `pico8_jf_free` (26), `pico8_gn` (50), `pico8_gn_free` (124) ; leurs appelants `bw`, `jo:pq` et `bg:pq` choisissent la zone | `gen` 4 → sort | *Reçu 1* : `oh` de `pico8_bg_pq` en zone frame |
| **4c** | **Tableaux croissants et registres** | `pico8_entity_init` (78, 82, 86 : `hv`, `cv`, `cg`) ; `pico8_ui_init` (145 : `g_shared_le`) ; `pico8_planet_family_init` (510 : `g_ga_by_shared`) ; crée la sous-zone chargement | `entity` 3 → sort ; `ui` 6 → 5 ; `planet_family` 6 → 5 (H-3-1) | *Reçu 2* : la liste statique de départ |
| **4d** | **Planètes et relais** | `pico8_jo_struct_new` (102), `pico8_jo_pq` (199, forme qui reste après l'étape 3), `pico8_jo_lz_mw_step` (215), `pico8_jo_lz` (238, 240, 242) ; `pico8_qh/nl/os/hj/pm_struct_new` (62, 123, 167, 242, 313) ; `pico8_nr/fr/mx_struct_new` (191, 237, 331) ; `pico8_jo_cn` (127, 129), `pico8_jo_ib` (188, 189, 215, 216, 218, 219) ; `pico8_bw` (63, 65, 71 : `p`, `ra`, identifiant « p… ») ; `bd_reset_visit` (48, 51, 52) | `planet` 6 → sort ; `planet_family` 5 → sort ; `planet_family_trade` 3 → sort ; `planet_trade` 8 → sort ; `galaxy` 3 → sort ; `economy_tiers` 3 → sort (28) | *Reçu 2* : les structures de planète ; *Reçu 3* : le `oe` des planètes par la variante de `jo:pq` à zone passée (H-3-4) |
| **4e** | **Slipways et routes** | `pico8_jc_struct_new` (344), `pico8_jc_pq` (89, 90 : `ts`), `pico8_jc_hq` (111, 112) ; `pico8_jc_ch` (`slipway_display` 36 : traînée, vers la zone particules) ; `pico8_trade_struct_new` (29), `pico8_trade_kg` (135-138) | `slipway` 5 → sort ; `slipway_display` 1 → sort ; `trade` 5 → sort (11) | *Reçu 1* : `re`/`p` de `jc:hq` ; *Reçu 4a* : traînées `ts` |
| **4f** | **Trous de ver** | `pico8_mz_struct_new` (204), `pico8_mz_pq` (39, 40 : `ox`) | `wormhole` 3 → sort | — |
| **4g** | **Recherche** (et économie) | `pico8_hp_struct_new` (133, 200, 201), `pico8_jy_new` (119), `pico8_research_free_gb_instances` (33), en sous-zone chargement ; `pico8_ie_struct_new` (203), `pico8_ie_hb` (100), `pico8_ie_ldown` (130) ; `pico8_eu` (`cart` 218 : `ie.jb`) ; `pico8_hy_struct_new` (`economy` 36, 62, 63) ; `fm_free` (`probe` 449) | `research` 5 → sort ; `research_icon` 3 → sort ; `cart` 1 → sort ; `economy` 3 → sort ; `probe` 3 → 2 (13) | *Reçu 1* : `ie.gy` ; *Reçu 2* : `ie.jb`, `hp.i` |
| **4h** | **Singletons de `jl`/`eu` et objets du joueur** | `pico8_qb_struct_new` (41), `pico8_qb_ph` (122, 124, 129) ; `pico8_nf_struct_new` (39), `pico8_nf_ph` (96) ; `pico8_ui_struct_new` (119, 129), `pico8_ui_shutdown` (156), `pico8_ui_lj` (235), `pico8_ui_gw` (295) ; `pico8_bg_struct_new` (54), `pico8_bg_hq` (208) ; `pico8_fv_struct_new` (`help` 116) ; `pico8_pj_struct_new` (`score_panel` 263) ; `pico8_g__struct_new` (`probe` 409), `pico8_bv` (`probe` 162) | `scroll` 4 → sort ; `mouse_reader` 2 → sort ; `ui` 5 → sort ; `bg` 2 → sort ; `help` 1 → sort ; `score_panel` 1 → sort ; `probe` 2 → sort (17) | *Reçu 1* : `p`/`v`/`ra` de `qb`, `mo->v`, `ra` de `nf`, `p` de `pico8_bv` ; *Reçu 2* : `ok.qa`, la structure `ok` et ses calques `ic`, `fv`, la structure `pj` ; *Reçu 3* : la struct `g_`, la struct `fv` (H-3-4) |
| | **Total** | | **4 + 5 + 28 + 11 + 3 + 13 + 17 = 81** | |

- **Pourquoi cet ordre.**
  - **4b** est la plus petite famille (4 relocations, moins de 2 Ko). Ses objets sont faits dans `eu` et rendus par `bw` au `eu` suivant. Elle éprouve la bascule sur de vrais objets avec le seul motif délicat de l'inventaire : un ancien objet rendu par son remplaçant, encore vivant.
  - **4c** vient ensuite parce que toutes les autres familles inscrivent leurs objets dans `hv`/`cv`/`cg`. Elle fixe aussi la tenue des tableaux croissants dans une zone qui ne rend pas : un agrandissement laisse un bloc mort jusqu'à la génération suivante, ce qui coûte au plus le double de la taille finale. Enfin, elle crée la sous-zone chargement, dont 4g aura besoin.
  - **4d** est le plus gros poste (planètes générées par `bw(90)`, `bd` par action). On le mesure tôt, pour que la projection du budget repose sur une mesure et non sur une estimation.
  - **4e** dépend de 4d en lecture seulement (un `jc` désigne ses planètes). Elle apporte la seule sorte de particules nouvelle.
  - **4f** est petite, avec les mêmes motifs que 4e.
  - **4g** touche le chargement et `fm`, que l'inventaire désigne comme vivant entre `_init` et `eu`.
  - **4h** est la plus difficile : boîtes par frame partagées entre objets (`ok.qa` gardé par les hôtes de `bv`), `ok` encore lu après `jl()`, sondes tenues. Elle passe en dernier. Elle donne aussi le critère de fin de l'étape 4 : `pause_restart` sous empoisonnement avec tous les singletons en zone.
- **Une famille de plus que la liste du plan : 4h.** Le plan énumère six familles, et aucune ne couvre les singletons `mo`/`je`/`ok`, `bg`, `fv`, `pj` ni les sondes `g_`. Ce sont pourtant ceux que les lignes « Reçu » des étapes 1, 2 et 3 renvoient à l'étape 4. Les ranger dans « tableaux croissants et registres » mêlerait les boîtes par frame, le cas le plus dur, à la famille qui doit ouvrir la voie. La note « Découvert » de 4a le dit, et le commit de clôture ajoute 4h à la liste de l'entrée 4.
- **Fichiers partagés entre familles.**
  - `ui` : 4c puis 4h.
  - `planet_family` : 4c puis 4d.
  - `probe` : 4g puis 4h.

  Chacun ne sort de la liste qu'avec sa dernière famille.
- Ces comptes sont des **prévisions**. Le verrou compte des relocations, qui dépendent du compilateur, et l'étape 3 n'est pas finie (H-3-1). Chaque famille repart de la mesure à son `HEAD` et note l'écart dans « Découvert ».

### Suivi du budget, famille par famille

**Estimation par lecture**, x86-64, pic d'une session e2e. La bascule est comprise : partie quittée + titre + première frame de la nouvelle partie, soit environ deux parties à la fois. Les tailles viennent de `sizeof` relevés à `HEAD` : `pico8_jo_t` 432, `pico8_jc_t` 960, `pico8_trade_t` 160, `pico8_hp_t` 576, `pico8_hy_t` 472, `pico8_ui_t` 584, `pico8_qb_t` 168, `pico8_nf_t` 144, `pico8_bg_t` 152, `pico8_g__t` 256, `pico8_pj_t` 168, `pico8_ie_t` 176, `pico8_mz_t` 224, `pico8_gn_state_t` 592, `pico8_jf_state_t` 8, `pico8_jy_t` 48, `pico8_ents_t`/`pico8_ptrs_t` 24, `pico8_vec_t` 8. Une allocation est arrondie à 8 o.

| Famille | Par génération | Pic estimé (bascule) |
|---|---|---|
| 4b états | ~0,6 Ko | ~2 Ko |
| 4c registres | `hv` ~200 entrées, 5 `cv`, les `cg`, blocs morts des agrandissements : ~10 Ko | ~20 Ko |
| 4d planètes | 90 planètes × ~0,9 Ko (struct, `p`/`ra`/id, `oe`, copies `jy`, `cc`/`by`/`ib` une fois les sites par action réglés) ≈ 80 Ko | ~160 Ko |
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
   - `768 Ko < P_k ≤ 768 Ko + marge`, avec marge = 1536 − (frame + interface + particules), soit 49 Ko si l'étape 3 n'a rien relevé (H-3-2) : la famille relève `PICO8_ZONE_GAME_BYTES` d'un chiffre, dans son commit, et le dit ;
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
   - Les aides de TU qui enchaînent deux `eu` sans vidage, corrigées pour vider la file.
3. **Remplissage écrit par le rejeu, et preuves sur rejeu.**
   - La ligne de la zone partie sous `PICO8_REMPLISSAGE`.
   - Le TU de la sentinelle sur `pause_restart`.
   - Le relevé des octets sur `message_box_hold` (f45 = f90).
   - Le contrôle du maximum par un rejeu.
4. **Clôture de 4a.**
   - La note « Découvert » sous 4a : point de bascule et de rendu, et pourquoi pas `jl()` ni `_init` ; inventaire et conclusion ; ordre des familles et attribution des 81 relocations ; méthode de budget ; écarts avec les hypothèses H-3-x.
   - Les corrections des lignes « Partie » et « Particules » du §2.
   - L'ajout de la famille 4h à la liste de l'entrée 4.
   - La ligne « *Reçu de l'étape 4a* » sur les traînées `ts`.
   - La vérification des points « À revérifier ».

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
- **Croissance par frame** : le même TU relève les octets de la zone partie aux frames 45 et 90 de `message_box_hold` et les trouve égaux. En 4a, la zone est vide et le relevé est trivial. Il est posé ici pour que chaque famille hérite du contrôle.
- **Remplissage écrit par le rejeu** : sur le modèle de `tests/test_pico8_replay_usage.c`, un rejeu avec `PICO8_REMPLISSAGE` écrit la ligne de la zone partie (0 octet pris, maximum 0 en 4a) sans changer la capture ni le code de sortie. Le cas « maximum > octets finaux » est porté par le TU de la zone à deux bouts, puisqu'aucun objet du jeu n'y entre encore.
- **Tests existants** : verts, sans changement d'attente. Seule adaptation permise : vider la file entre deux `eu` dans les aides de TU qui ne jouent pas de frame. L'`assert` les désigne.
- **Verrou** : `make alloc-guard` à égalité stricte, comptes inchangés. Pas de nouveau cas dans `tests/test_alloc_guard.c`.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. En 4a, rien n'est empoisonné en jeu, et `sanitize-net` vert ne prouve que l'absence de régression. La preuve du point de rendu est le TU de la sentinelle.
- **Exemples dans le dépôt** : `tests/test_pico8_zone.c`, `tests/test_pico8_zone_frame.c`, `tests/test_pico8_zone_choisie.c` (lecture périmée dans un enfant, zone pleine en 3) ; `tests/test_pico8_places_vivantes.c` ; `tests/test_pico8_replay_usage.c` ; `src/main.c` (`write_remplissage`).

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

## Hypothèses sur l'étape 3

L'étape 3 est en cours (PRD `docs/prd/triage/phase1-etape3-zone-particules.md`). Cette PRD suppose ce qui suit. Chaque point est à revérifier quand l'étape 3 sera finie.

- **H-3-1 (comptes laissés).** L'étape 3 se clôt sur sa prévision : **24 fichiers, 84 appels**.
  - `pico8_particle.c` sort.
  - `probe` 4 → 3 (`g__mm`).
  - `planet_family` 9 → 6 (`ga_mm`, `ga_struct_new`, `pm_pq`).
  - `planet` 7 → 6 (une des deux de `jo_pq`).

  L'attribution des 81 relocations de l'étape 4 en dépend.
- **H-3-2 (budget).** `PICO8_ZONE_PARTICLES_BYTES` reste à 128 Ko : somme 1487 Ko, marge 49 Ko. Tout relèvement de l'étape 3 (permis jusqu'à 177 Ko) retire autant à la marge de l'étape 4.
- **H-3-3 (format du remplissage).** `main.c` écrit, sous `PICO8_REMPLISSAGE`, par sorte, les places vivantes et le maximum de la session. 4a ajoute sa ligne dans la même forme (nom, valeur courante, maximum).
- **H-3-4 (sorties de l'étape 3 vers l'étape 4).**
  - `g_` et `fv` ont `rendre` et passent par la file de sortie.
  - Les `rendre` des planètes émettrices lisent la struct (rangée, puis `qy`) au vidage.
  - `jo:pq` a une variante à zone passée, et sa forme actuelle garde une référence à la transitoire.
  - Les lignes « *Reçu de l'étape 3* » sous l'entrée 4 nomment la struct `g_`, la struct `fv` et le `oe` des planètes.
  - L'éclat `ga` emprunte `p`/`ra`/`id` à la planète remplacée par `jo:hu`.
- **H-3-5 (inventaire).** L'étape 3 n'a rien ajouté à `oublier`. Les pointeurs statiques sont ceux relevés à `HEAD` 7255c72.
- **H-3-6 (TU).** Les aides de TU qui enchaînent `_init`/`eu` sans frame vident la file de sortie (leçon des étapes 2 et 3). `tests/test_pico8_places_vivantes.c` sait jouer un e2e frame par frame dans un enfant et lire des comptes entre deux frames.
- **H-3-7 (traînées `ts`).** L'étape 3 ne touche pas `pico8_jc_ch` : la traînée reste un `malloc` dans `pico8_slipway_display.c` (1).
- **H-3-8 (file de sortie).** Capacité inchangée (256). Le vidage reste en tête de `pico8__update60()`, après la remise de la zone frame et avant `pico8_dg()`. Il est aussi appelé par `pico8_cart_load()`, `pico8_cart_shutdown()` et `main.c`.

## À revérifier une fois l'étape 3 finie

- `make alloc-guard` sur le `HEAD` d'alors : liste et comptes par fichier. Recalculer la colonne « Relocations » de l'ordre des familles et le total de 81. Un écart va dans « Découvert » de 4a.
- La note « Découvert » de l'étape 3 :
  - sites classés autrement que prévu (une struct `g_` ou `fv` gardée par l'étape 3 change 4h) ;
  - `PICO8_ZONE_PARTICLES_BYTES` et la somme des zones (marge de l'étape 4) ;
  - toute référence ajoutée à `oublier`.
- Les lignes « *Reçu de l'étape 3* » sous l'entrée 4 du plan, à rattacher à leur famille dans la table d'ordre.
- Le format des lignes de `PICO8_REMPLISSAGE` (H-3-3), pour que la ligne de la zone partie suive le même.
- Le `grep` de l'inventaire refait sur le code d'alors : tout pointeur statique nouveau (par exemple un pointeur de module de particules) est classé avec la question de 4a.
- Le vidage de la file et son ordre dans `pico8__update60()` (H-3-8). Le point de rendu de la zone partie s'y accroche.
- Si l'étape 3 a ajouté la sorte `ts` à son module, contre H-3-7 : retirer `pico8_jc_ch` de 4e et la ligne « Reçu de l'étape 4a ».
- Si le module de particules ne permet pas d'ajouter une sorte sans le refaire : le dire dans la ligne « Reçu de l'étape 4a ».

## Further Notes

- **Pas d'humain disponible, pas d'outil `Agent` dans cette session.** Les points que `to-prd` fait valider par un humain (modules attendus, tests voulus, découpage) ont été tranchés en appliquant moi-même le skill `guideline` :
  - **Hypothèses explicites** : H-3-1 à H-3-8 et la liste « À revérifier » ; l'inventaire du §4 refait par `grep` ; les quatre lectures après `jl()`, relevées dans le code et dans le cart (l. 217-247, 1178-1186).
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
  - la conclusion de l'inventaire (point d'oubli vide) ;
  - l'ordre 4b → 4h, la famille 4h ajoutée et pourquoi ;
  - l'attribution des 81 relocations et l'écart à H-3-1 ;
  - la règle des sites par tick ou par action ;
  - la méthode de budget et l'estimation de départ (~265 Ko) ;
  - les corrections du §2 (Partie, chargement, `ts`).
