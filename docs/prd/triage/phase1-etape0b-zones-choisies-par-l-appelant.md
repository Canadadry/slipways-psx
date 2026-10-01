---
title: "Plan 4, étape 0b — Socle : zones choisies par l'appelant"
description: "Les tableaux croissants partagés et les fonctions partagées qui fabriquent textes et boîtes allouent eux-mêmes avec la libc, alors que leurs résultats vivent tantôt une frame, tantôt toute la partie : tant que la zone n'est pas un paramètre, aucune étape de conversion ne peut choisir la bonne."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) range toute la mémoire du cœur dans des zones de taille fixe : frame, interface, particules, partie. L'étape 0a a posé les outils :

- `pico8_fatal` ;
- le module de zone : zone à pointeur montant et zone à places, allocateurs en `unsigned char *` ;
- le plafond de 1,5 Mo, vérifié à la compilation ;
- le verrou à cliquet `alloc-guard` : 36 fichiers, 148 appels, égalité stricte.

Elle n'a converti aucun site.

Les étapes 1 à 4 doivent déplacer chaque allocation dans sa zone. Mais une partie des allocations ne se fait pas au site qui sait combien de temps l'objet vit :

- les **tableaux croissants partagés** (`pico8_ents_t`, `pico8_ptrs_t`) font eux-mêmes leur `realloc` et leur `free`. Or un même type sert au registre `hv` de toute la partie, aux listes `oe`/`cc` d'une planète, et à une liste filtrée jetée à la fin de la frame ;
- les **listes fabriquées** (`jp`, `la`, `nv`, la concaténation et le filtre d'entités) rendent une liste neuve, que l'appelant libère ;
- les **textes fabriqués** (`tostr`, `sub`, `ne`, les concaténations) font leur `malloc` dans la fonction partagée. Or l'identifiant `"p"..id` d'une planète, qui vit toute la partie, est fait par les mêmes fonctions qu'un texte de menu jeté à la frame suivante ;
- la **boîte** `qz` est fabriquée par une fonction partagée ;
- les tableaux des **panneaux `lq`** (`items`, `fs`, et `ls` d'un `jb`) sont alloués et agrandis au fond du module de mise en page. Leur durée de vie est pourtant celle du panneau, que seul son créateur connaît.

De plus, cinq fichiers recopient la même concaténation statique (`concat2`), avec un `malloc` chacun.

Aujourd'hui, convertir un seul appelant obligerait à modifier la fonction partagée, et donc à changer la zone de **tous** ses appelants. Soit on casse l'invariant du plan (un objet de partie logé en zone frame plante sous ASan), soit on duplique chaque fonction par zone.

## Solution

L'étape 0b fait de la zone un **paramètre choisi par l'appelant**. Elle ne convertit encore aucun site.

- Une **zone désignée** est une poignée qu'un propriétaire ou un appelant passe aux fonctions partagées. Elle sait **allouer**, **agrandir** et **rendre**. Deux sortes existent en 0b :
  - une poignée sur une **zone à pointeur montant** de 0a. Agrandir alloue un nouveau bloc et recopie. Rendre ne fait rien : la mémoire repart à la remise à zéro.
  - la **zone transitoire**, adossée à la libc. Allouer y fait `malloc`, agrandir `realloc`, rendre `free`. Elle reproduit exactement le comportement actuel.
- Les **tableaux croissants partagés** (`ents`, `ptrs`) reçoivent leur zone de leur propriétaire, à l'initialisation, et la gardent. Chaque croissance et la libération passent par elle.
- Les **fonctions partagées qui fabriquent** une liste, un texte ou une boîte reçoivent la zone de l'appelant. Ce sont `jp`, `la`, `nv`, la concaténation et le filtre d'entités, `tostr`, `sub`, `ne` et `qz`. S'y ajoute une concaténation de textes partagée, qui remplace les cinq copies statiques.
- Les **panneaux `lq` et `jb`** reçoivent de leur créateur la zone de leurs tableaux, et la gardent pour `lq_add`.
- **Tous les appelants actuels passent la zone transitoire.** L'appelant d'un texte libère encore comme aujourd'hui. Les captures restent identiques au pixel près, et tous les `*.sha256` de `tests/e2e/` restent inchangés.
- Le **verrou** compte chaque référence à la zone transitoire comme une allocation.
  - Les comptes se déplacent une seule fois, dans 0b : ils baissent dans les fonctions partagées et montent chez leurs appelants.
  - Le compte d'un fichier mesure alors le travail qui lui reste. Les étapes 1 à 4 le font baisser appel par appel, et l'étape 5 supprime la zone transitoire.
  - Après 0b, aucun compte ne peut plus monter (voir « Le cliquet pendant 0b »).

## User Stories

1. En tant que développeur de l'étape 1, je veux passer la zone frame à `tostr` pour un texte de menu, et la zone transitoire ailleurs, pour convertir un appelant sans toucher aux autres.
2. En tant que développeur de l'étape 4, je veux passer la zone partie aux fonctions qui fabriquent l'identifiant d'une planète, pour qu'il vive toute la partie alors que les mêmes fonctions servent aux textes de frame.
3. En tant que développeur des étapes 1 à 4, je veux qu'un propriétaire de liste `ents` ou `ptrs` désigne sa zone une seule fois, à l'initialisation, pour que chaque `add` ultérieur grandisse dans la bonne zone sans repasser la zone à chaque appel.
4. En tant que développeur des étapes 1 à 4, je veux que la libération d'une liste passe par la zone qu'elle a gardée, pour que la même ligne de libération soit correcte en zone transitoire (vraie libération) et en zone à pointeur montant (rien à faire).
5. En tant que développeur de l'étape 1, je veux qu'une liste filtrée (`jp`) ou concaténée (`la`, concaténation d'entités) soit fabriquée dans la zone que je passe, pour la mettre en zone frame quand elle ne sert qu'à la frame.
6. En tant que développeur de l'étape 1, je veux que `nv` et le filtre d'entités prennent aussi une zone, pour qu'aucune liste fabriquée par le cœur n'échappe au choix de l'appelant.
7. En tant que développeur des étapes 1 et 4, je veux une seule concaténation de textes partagée, qui prend une zone, à la place des cinq copies statiques identiques, pour convertir chaque appel de concaténation à son rythme et dans sa zone.
8. En tant que développeur de l'étape 1, je veux que `ne` fabrique son résultat dans la zone de l'appelant et y rende son texte intermédiaire, pour que `ne` n'ait plus aucun appel libc à lui.
9. En tant que développeur de l'étape 2, je veux que les tableaux d'un panneau `lq` et les lignes d'un `jb` prennent la zone désignée par le créateur du panneau, pour que leur durée de vie suive celle du panneau.
10. En tant que développeur de l'étape 2, je veux que `lq_add` agrandisse les cellules dans la zone gardée par le panneau, pour qu'un ajout tardif ne retombe pas dans la libc.
11. En tant que développeur, je veux que la boîte `qz` soit fabriquée dans la zone de l'appelant, pour que la seule boîte fabriquée par une fonction partagée suive la règle commune.
12. En tant que développeur, je veux qu'agrandir un tableau dans une zone à pointeur montant garde ses éléments dans l'ordre, pour que `add`, `del` et l'itération se comportent comme en libc.
13. En tant que développeur, je veux qu'un itérateur `all()` survive à la croissance en zone à pointeur montant comme en libc, pour que la tolérance à la suppression pendant le parcours ne dépende pas de la zone.
14. En tant que développeur qui chasse un accès hors durée de vie, je veux qu'une liste grandie dans une zone à pointeur montant, puis lue après la remise à zéro de cette zone, plante sous ASan, pour que l'oracle du plan couvre aussi les tableaux croissants.
15. En tant que développeur, je veux qu'une croissance qui remplit sa zone sorte par `pico8_fatal` avec le message de 0a (« zone <nom> pleine, limite <n> octets », code 3), pour qu'une liste trop longue se lise comme une limite à relever.
16. En tant que développeur, je veux qu'une liste initialisée sans zone soit refusée par un `assert` porteur de message, pour qu'un oubli ne retombe pas en silence dans une zone par défaut.
17. En tant que mainteneur de l'invariant, je veux que tous les appelants actuels passent la zone transitoire, pour que l'étape ne change ni un pixel ni un `*.sha256`.
18. En tant que mainteneur de l'invariant, je veux que la zone transitoire se comporte exactement comme `malloc`/`realloc`/`free`, pour que `make sanitize-net` voie le même programme qu'avant.
19. En tant que développeur, je veux que l'appelant d'un texte fabriqué en zone transitoire le libère encore avec `free`, comme aujourd'hui, pour que l'étape ne touche à aucune ligne de libération de texte des sites du jeu.
20. En tant que gardien du cliquet, je veux que chaque référence à la zone transitoire dans un objet du cœur compte comme un appel d'allocation, pour que le compte d'un fichier dise combien d'appels il lui reste à convertir.
21. En tant que gardien du cliquet, je veux que le fichier de la zone transitoire figure dans la liste d'exceptions avec son compte, pour que ses propres appels libc restent visibles jusqu'à sa suppression à l'étape 5.
22. En tant que relecteur, je veux que chaque commit de 0b remette la liste d'exceptions à égalité stricte et dise, dans son message, quels comptes montent, lesquels baissent, et pourquoi, pour que le déplacement des comptes soit lisible dans le diff.
23. En tant que gardien du cliquet, je veux qu'après le dernier commit de 0b aucun compte de la liste ne puisse plus monter, pour que la règle « ne peut que baisser » de 0a reprenne sans exception.
24. En tant que développeur des étapes 1 à 4, je veux que la note « Découvert » de 0b donne, fichier par fichier, les comptes laissés par 0b, pour savoir combien d'appels chaque étape doit retirer.
25. En tant que développeur, je veux que le module de zone de 0a et le module de zone désignée restent hors de la liste d'exceptions, pour que la zone transitoire soit le seul code de zone qui touche la libc.
26. En tant que futur porteur PSX, je veux que supprimer la zone transitoire à l'étape 5 revienne à supprimer un fichier, sa ligne d'exception et le symbole compté par le verrou, pour qu'aucun appel libc ne reste caché dans le module de zone désignée.
27. En tant que développeur, je veux que les fonctions de fabrication locales à un fichier (`jo_dollar`, `concat_percent`, la jointure de `hp`, le texte de `pj`) et les boîtes `vec` écrites en ligne restent des sites ordinaires, pour qu'elles soient converties par leur étape comme n'importe quel `malloc` du jeu.
28. En tant que développeur, je veux que les tests unitaires des listes, des textes et des panneaux prouvent le même comportement dans une zone à pointeur montant et dans la zone transitoire, pour que le choix de zone ne change que la durée de vie.
29. En tant que développeur, je veux qu'aucune zone du jeu (frame, interface, particules, partie) ne soit encore instanciée en 0b, pour que le BSS ne grossisse pas et que l'étape reste un socle.
30. En tant que gardien de `composition-guard`, je veux que la poignée et ses opérations n'utilisent pas `void *`, mais `unsigned char *` comme les allocateurs de 0a, pour que les catégories `void-ptr` et `registres` restent à 0.

## Implementation Decisions

- **Zone désignée.** C'est un petit module du cœur, distinct de `pico8_zone`, qui définit la poignée et ses trois opérations.
  - Les opérations sont **allouer** `n` octets, **agrandir** un bloc de l'ancienne taille à la nouvelle en gardant son contenu, et **rendre** un bloc.
  - La poignée porte ses opérations par pointeurs de fonction. Le module ne référence donc jamais la zone transitoire, et la supprimer à l'étape 5 ne le touche pas.
  - Tout passe en `unsigned char *`, comme les allocateurs de 0a. `composition-guard` compte en effet, sous `registres`, un membre qui rend `void *` par pointeur de fonction, et sous `void-ptr`, un prototype `pico8_*` d'en-tête qui prend `void *`.
  - Seul l'appelant final convertit le bloc vers son type : `items` d'une liste, `char *` d'un texte.
  - Forme indicative :

  ```c
  typedef struct pico8_zone_choisie {
      unsigned char *(*alloc)(const struct pico8_zone_choisie *z, size_t n);
      unsigned char *(*grow)(const struct pico8_zone_choisie *z, unsigned char *p,
                             size_t old_n, size_t new_n);
      void (*release)(const struct pico8_zone_choisie *z, unsigned char *p);
      pico8_bump_zone_t *bump; /* NULL pour la transitoire */
  } pico8_zone_choisie_t;
  ```

  Le nom définitif est laissé à l'implémenteur.
- **Poignée sur zone à pointeur montant.** Elle se construit à partir d'un `pico8_bump_zone_t` déjà initialisé.
  - **Allouer** délègue à `pico8_bump_zone_alloc`, qui aligne sur une cellule `pico8_zone_cell_t` et appelle `pico8_fatal` quand la zone est pleine.
  - **Agrandir** alloue `new_n` octets puis recopie `old_n` octets, puisque 0a n'a ni `realloc` ni libération par bloc. L'ancien bloc reste en place jusqu'à la remise à zéro.
  - **Rendre** ne fait rien.

  Aucune zone du jeu n'est instanciée en 0b : seuls les tests créent de telles poignées, sur un stockage `PICO8_BUMP_ZONE_STORAGE` local.
- **Zone transitoire.** C'est un fichier du cœur à part, dans `CORE_SRCS`, qui expose une seule poignée globale constante.
  - Ses opérations font `malloc`, `realloc` et `free`, avec `abort` sur `NULL` comme aujourd'hui.
  - Elle n'a pas de taille : elle n'entre pas dans `pico8_zone_sizes.h` ni dans le plafond de 1,5 Mo.
  - Son fichier entre dans `tools/alloc_guard_exceptions.txt` avec ses 3 appels libc. Il en sort à l'étape 5, quand on le supprime.
- **Le verrou compte la zone transitoire.** `tools/alloc_guard.sh` ajoute le symbole de la poignée transitoire aux quatre noms qu'il compte : la condition `awk` sur `malloc`, `calloc`, `realloc` et `free`, avec le préfixe `_` de Darwin.
  - Chaque relocation vers ce symbole, dans une section de code ou de données d'un objet du cœur, compte pour une allocation, dans le même compte par fichier.
  - Le format de la liste ne change pas : `<source> <appels>`, égalité stricte, messages « ligne à écrire ». L'en-tête du script et celui de la liste nomment le symbole ajouté.
  - Le module de zone désignée et `pico8_zone.c` restent hors de la liste.
- **Le cliquet pendant 0b.** C'est la tension avec la règle de 0a, « ce nombre ne peut que baisser ». Elle est tranchée ainsi : **les comptes des appelants montent pendant 0b, et nulle part ensuite.**
  - **Pourquoi c'est autorisé.**
    - Le texte de 0b dit de la zone transitoire : « Ses allocations sont comptées par le cliquet comme les autres ; les étapes 1 à 4 en retirent leurs appelants. »
    - Compter une allocation faite pour un appelant, chez cet appelant, fait monter son compte au moment où l'allocation quitte la fonction partagée.
    - La règle de 0a vise la réintroduction dans un fichier à moitié converti. En 0b, aucun fichier n'est converti, donc il n'y a rien à réintroduire.
  - **La variante écartée.** On aurait pu ne faire monter aucun compte en gardant les anciennes signatures comme enveloppes qui passent la transitoire. Le diff serait plus petit : aucun site d'appel ne change en 0b. Mais une allocation faite pour un appelant ne serait pas comptée chez lui, ce que le texte de 0b exclut. Et un fichier déjà converti pourrait rappeler l'enveloppe sans que le verrou le voie, ce qui lui ôte sa raison d'être. Le changement retenu est donc le plus sûr, même s'il touche plus de lignes.
  - **La règle.**
    - Dans un commit de 0b, un compte ne monte que du nombre de références à la poignée transitoire que ce commit ajoute au fichier.
    - Le message de commit liste les fichiers qui montent et ceux qui baissent ou sortent.
    - Après le dernier commit de 0b, aucun compte ne monte plus, et la règle de 0a reprend sans exception. La note « Découvert » de 0b le redit, à côté des comptes qu'elle laisse.
  - **Ordre de grandeur, à remesurer.**
    - Sortent de la liste : `pico8_ents.c` (2), `pico8_ptrs.c` (2), `pico8_str.c` (3), `pico8_collect.c` (3) et `pico8_geom.c` (1).
    - `pico8_layout.c` passe de 6 à 2 : il garde les objets `lq` et `jb` eux-mêmes.
    - Les cinq fichiers à `concat2` perdent chacun 1.
    - Les appelants gagnent une unité par référence à la transitoire, soit environ 150 appels relevés dans `src/`.
    - Le total de la liste monte donc nettement au-dessus de 148. C'est le reste à faire réel, jusque-là caché dans les fonctions partagées.
- **`ents` et `ptrs`.**
  - L'initialisation prend la zone et la garde dans le conteneur. Une zone `NULL` est refusée par un `assert` porteur de message.
  - La croissance passe par **agrandir**, la libération par **rendre**.
  - Le conteneur garde son type, sa forme (`items`, `count`, `capacity`, doublement à partir de 4) et ses contrats d'itération.
  - Les fonctions de libération gardent leur nom et leur contrat (« libère `items` seulement, laisse le conteneur vide ») ; seul le mécanisme change.
  - Les trois conteneurs statiques (`hv`, le `by` partagé des `ga`, le `le` partagé de `ui`) reçoivent la transitoire à leur initialisation, comme tous les autres.
- **Listes fabriquées.** `jp_ptrs`, `la_ptrs`, `nv_ptrs`, `ents_cat` et `jp_ents` prennent la zone de la liste qu'elles rendent. L'appelant la libère toujours avec la fonction de libération du conteneur, qui passe par la zone gardée. `jp_ptrs`, `la_ptrs` et `nv_ptrs` gardent leur nom, qui est leur exemption dans `REGISTRES_NON_ENTITE` de `composition-guard`.
- **Textes.**
  - `tostr` et `sub` prennent une zone.
  - Une concaténation de deux textes partagée, dans le module des chaînes, prend une zone. Elle remplace les cinq copies statiques identiques (galaxie, menu, icône de recherche, tableau des scores, affichage du tableau des scores) : c'est un déplacement de code à comportement identique.
  - `ne` prend une zone. Elle y fabrique son résultat, y fabrique son texte intermédiaire, et le rend par **rendre**.
  - L'appelant d'un texte fabriqué en zone transitoire le libère encore avec `free` : aucune ligne de libération de texte d'un site du jeu ne change en 0b.
  - Les fabricants locaux à un fichier (`jo_dollar`, `concat_percent`, la jointure de `hp`, le texte de `pj`) ne sont pas partagés. Ils restent des sites, que l'étape 1 convertira.
- **Boîtes.**
  - `qz` prend une zone.
  - Les boîtes `vec` et les listes enveloppées s'écrivent aujourd'hui en ligne (`malloc(sizeof *heap)` au site), sans fonction partagée. Ce sont des sites ordinaires : chaque site choisira sa zone à son étape.
  - 0b ne crée pas de fonction de boîte `vec`, car sans site converti elle n'aurait aucun appelant. Cette lecture de la puce du plan (« boîtes `vec` ») va dans la note « Découvert ».
- **Panneaux `lq` et `jb`.**
  - Les fonctions de création (`lq_new`, `lq_copy`, `jb_new`) prennent la zone des tableaux du panneau : `items`, `fs`, `ls`. Le panneau la garde, et `lq_add` agrandit `items` par **agrandir**.
  - Les lignes qu'un `jb` crée héritent de sa zone.
  - L'objet panneau lui-même reste alloué comme aujourd'hui. C'est un site de l'étape 2 (zone interface), pas un tableau croissant.
  - Le choix de la zone des tableaux d'un panneau qui vivra dans une zone à places revient à l'étape 2.
- **Appelants.** Tous les sites du cœur et des tests qui appellent une fonction modifiée passent la zone transitoire, sans autre changement. Ordre de grandeur relevé : environ 150 appels dans `src/` et autant dans `tests/`.
- **Commentaires.** Ils suivent les quatre destinations de `CLAUDE.md`.
  - Le contrat « qui libère » des en-têtes modifiés est réécrit : « dans la zone passée ; rendu par cette zone ».
  - « L'appelant libère avec `free` » ne reste vrai que pour la transitoire.
  - Le pourquoi de la copie à la croissance en zone à pointeur montant est un piège qu'on réintroduirait (un `realloc` direct). Il tient en un commentaire court.
- **Invariant.** À chaque commit, `make test-all` et `make sanitize-net` passent, et tous les `*.sha256` restent identiques.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant et remet la liste d'exceptions à égalité stricte, avec les hausses et baisses écrites dans son message.

1. **Zone désignée et zone transitoire.** Le module de poignée, la poignée sur zone à pointeur montant, le fichier de la zone transitoire et sa ligne d'exception (3). S'y ajoutent le symbole transitoire compté par `alloc_guard.sh`, et les tests du module et du verrou. Aucun appelant du jeu ne change.
2. **Listes.** `ents`, `ptrs` et les cinq fabricants de listes prennent leur zone, et tous leurs appelants passent la transitoire. Après remesure, `pico8_ents.c` et `pico8_ptrs.c` sortent de la liste, et les appelants montent.
3. **Textes et boîte.** `tostr`, `sub`, `ne`, la concaténation partagée (qui supprime les cinq copies) et `qz` ; tous leurs appelants passent la transitoire. On remesure : `pico8_str.c`, `pico8_collect.c` et `pico8_geom.c` sortent de la liste.
4. **Panneaux.** `lq_new`, `lq_copy`, `jb_new` et `lq_add` ; tous leurs appelants passent la transitoire. On remesure : `pico8_layout.c` descend. C'est le dernier commit où un compte peut monter.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe le comportement externe : le contenu et l'ordre d'une liste, le texte rendu, un code de sortie, un rapport ASan, la sortie du verrou. Il ne regarde ni la capacité interne ni l'adresse d'un bloc.
- **Zone désignée** : un TU sous ASan, construit comme `tmp/test_pico8_zone`.
  - Il a sa règle avec `asan_objs`, et il lie le module de poignée, `pico8_zone.c`, la zone transitoire, `pico8_fatal_pc.c`, et `ents`, `ptrs` et les chaînes pour les cas de liste et de texte.
  - Il entre dans `UNIT_RUNS_SPECIAL` et est lancé avec `ASAN_OPTIONS=detect_leaks=0`.
  - Cas obligatoires :
    - une liste grandie bien au-delà de 4 éléments dans une poignée sur zone à pointeur montant garde tous ses éléments, dans l'ordre ;
    - une liste grandie, puis lue **après une** remise à zéro de sa zone et de nouvelles allocations, plante (`use-after-poison`), dans un processus enfant. Le test se limite à une seule remise à zéro : 0a a établi qu'après deux, la moitié empoisonnée redevient valide et rien n'est plus attrapé ;
    - cas de contrôle : la même liste lue avant la remise à zéro ne plante pas ;
    - une croissance qui dépasse la zone sort avec le code 3, et `stderr` contient « zone <nom> pleine » ;
    - la zone transitoire alloue, agrandit et rend sans rapport ASan.
- **`ents` et `ptrs`** : les tests existants (`test_pico8_ents`, `test_pico8_ptrs`) passent la transitoire et restent verts sans autre changement. Les cas d'itération avec suppression pendant le parcours, et la libération, sont rejoués sur une poignée à pointeur montant dans le TU ci-dessus.
- **Textes** : `test_pico8_str` passe la transitoire. Les mêmes témoins sont rejoués en zone à pointeur montant dans le TU ASan, avec :
  - pour la concaténation partagée : deux textes, puis un texte vide de chaque côté ;
  - pour `ne` : un signe positif et un négatif, avec et sans suffixe.
- **Panneaux** : `test_pico8_lq` et `test_pico8_jb_ol` passent la transitoire. On ajoute un cas `lq_add` sur un panneau en zone à pointeur montant, dont le dessin reste le même qu'en transitoire.
- **Verrou** : `tests/test_alloc_guard.c` reçoit des objets fabriqués de plus, avec ces cas :
  - un fichier qui référence la poignée transitoire sans être listé échoue, avec la « ligne à ajouter » ;
  - deux références à la poignée et un `free` comptent 3 ;
  - la forme Darwin du symbole, préfixée `_`, est reconnue (via `$OBJDUMP` simulé, comme les cas Darwin existants).
- **Invariant** : `make e2e` et `make sanitize-net`, avec des `*.sha256` inchangés, prouvent que le passage par la transitoire ne change rien.
- **Exemples dans le dépôt** :
  - `tests/test_pico8_zone.c` et sa règle ASan ;
  - `tests/test_alloc_guard.c` ;
  - `tests/test_pico8_replay_usage.c` (processus enfant, statut et `stderr`).

## Out of Scope

- Toute conversion d'un site du jeu vers une zone du jeu : en 0b, chaque appelant passe la transitoire. C'est le travail des étapes 1 à 4.
- L'instanciation des zones frame, interface, particules et partie. Les tailles de `pico8_zone_sizes.h` ne changent pas.
- Les `malloc` écrits en ligne aux sites : objets du jeu, boîtes `vec`, listes enveloppées, objets panneau et `jb` eux-mêmes, fabricants de texte locaux à un fichier.
- Une poignée sur zone à places. L'étape 2 l'ajoutera si les tableaux d'un panneau doivent y vivre.
- Le remplacement des `free` de texte des appelants par **rendre** : l'appelant libère encore, comme le dit le plan.
- La suppression de la zone transitoire, de son symbole dans le verrou et de `TROU-7`, qui revient à l'étape 5.
- Les autres appels libc du cœur (§7 du plan).
- Lancer l'application raylib.

## Vérifié contre 0a

Hypothèses relues contre `HEAD` b5668f7 : note « Découvert » de 0a, `pico8_zone.[ch]`, `pico8_zone_sizes.h`, `pico8_fatal.h`, `pico8_fatal_pc.c`, `tools/alloc_guard.sh`, la liste d'exceptions, la règle `tmp/test_pico8_zone` et `tools/composition_residue.sh`.

- **H-0a-1 (API de la zone montante) — tenue, avec un écart de type.**
  - L'API est `pico8_bump_zone_init(zone, nom, stockage, capacité)` / `_alloc(zone, taille)` / `_reset(zone)`, sans `realloc` ni libération par bloc. **Agrandir** copie donc.
  - Les blocs sont en `unsigned char *`, parce que `composition-guard` refuse `void *`. La poignée a été réécrite en conséquence : plus de `void *`, un membre `pico8_bump_zone_t *` à la place de `void *impl`.
- **H-0a-2 (alignement) — tenue.** Chaque allocation commence sur une cellule `pico8_zone_cell_t` (union `unsigned long long` / `char *`).
- **H-0a-3 (`pico8_fatal`) — tenue.** Code 3 sur PC, message `pico8_fatal: zone <nom> pleine, limite <n> octets`. `pico8_fatal_pc.c` est dans `ENGINE_SRCS`, hors `CORE_SRCS`, et lié par raylib.
- **H-0a-4 (verrou) — tenue, avec une précision.** Le verrou compte les relocations par `objdump -r`, hors `.debug*`/`.eh_frame`, avec égalité stricte et une liste `<source> <appels>`. Les noms comptés sont écrits dans la condition `awk` du script : ajouter le symbole transitoire est une modification du script et de son test, sans changement de format.
- **H-0a-5 (règle ASan) — tenue en partie.** Il n'existe pas de règle ASan générique, mais une règle par TU (`asan_objs`, entrée dans `UNIT_RUNS_SPECIAL`, `detect_leaks=0`). Le TU de 0b en ajoute une sur ce modèle.
- **H-0a-6 (budget) — tenue.** `pico8_zone_sizes.h` ne déclare que les quatre zones (frame 256 Ko, interface 256 Ko, particules 128 Ko, partie 768 Ko : 1408 Ko sur 1536). La transitoire n'y entre pas.
- **H-0a-7 (signatures et comptes initiaux) — tenue sur les signatures, en échec sur les comptes cités.**
  - Aucun en-tête touché par 0b n'a changé.
  - Comptes réels : `pico8_ents.c` 2 et `pico8_ptrs.c` 2 (`realloc` + `free`, et non 1), `pico8_str.c` 3, `pico8_collect.c` 3, `pico8_geom.c` 1, `pico8_layout.c` 6. Liste : 36 fichiers, 148 appels.
  - Les ordres de grandeur du « Cliquet pendant 0b » et les morceaux 2 à 4 sont réécrits sur ces chiffres.
- **H-0a-8 (classement des fichiers) — tenue.**
  - `pico8_zone.c` est dans `CORE_SRCS` et hors liste.
  - `pico8_input.c` est coupé : les E/S JSON sont dans `pico8_input_json.c`, hors cœur.
  - `pico8_coverage.c` est sorti du cœur.

  Rien de cela ne touche les fichiers de 0b.
- **Constat ajouté.** Sous ASan, un pointeur gardé à travers **deux** remises à zéro n'est plus attrapé. Les tests de 0b s'en tiennent donc à une seule remise à zéro.

## Further Notes

- Les points que `to-prd` fait valider par un humain ont été tranchés en appliquant les principes du skill `guideline` : modules attendus, modules à tester, découpage, et ici la tension du cliquet. Aucun outil `Agent` n'était disponible dans cette session.
  - **Simplicité** : pas de fonction de boîte `vec` sans appelant, et pas de poignée sur zone à places.
  - **Changements chirurgicaux** : les appelants passent la transitoire et gardent leur `free` de texte.
  - **Sûreté avant taille du diff** : le comptage chez l'appelant est préféré aux enveloppes (voir « Le cliquet pendant 0b »).
  - Le seul regroupement de code retenu est la fusion des cinq `concat2` identiques. C'est la puce « concaténations » du plan. Sans cette fusion, l'étape 1 devrait convertir cinq copies dont les appelants ont des durées de vie mêlées : l'identifiant de planète de la galaxie est une donnée de partie.
- Mesures faites sur les sources pour cette PRD, à remesurer sur objets :
  - `tostr` : 28 appels dans 12 fichiers ;
  - `ents_init` : 24 ;
  - `ents_free` : 16 ;
  - `ptrs_init` : 11 ;
  - `lq_new` : 10 ;
  - les cinq `concat2` : 23.
- À la fin de l'étape, `plan-pilot` ajoute sous l'entrée 0b du plan sa note « Découvert ». Elle porte au minimum :
  - les comptes du cliquet laissés par 0b, fichier par fichier, et leur total ;
  - la règle « plus aucune hausse après 0b » ;
  - la lecture retenue pour les boîtes `vec`.
