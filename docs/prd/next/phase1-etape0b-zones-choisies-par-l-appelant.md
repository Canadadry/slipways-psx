---
title: "Plan 4, étape 0b — Socle : zones choisies par l'appelant"
description: "Les tableaux croissants partagés et les fonctions partagées qui fabriquent textes et boîtes allouent eux-mêmes avec la libc, alors que leurs résultats vivent tantôt une frame, tantôt toute la partie : tant que la zone n'est pas un paramètre, aucune étape de conversion ne peut choisir la bonne."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) range toute la mémoire du cœur dans des zones de taille fixe (frame, interface, particules, partie). L'étape 0a pose les outils : `pico8_fatal`, le module de zone, le plafond de 1,5 Mo et le verrou à cliquet. Elle ne convertit aucun site.

Les étapes 1 à 4 doivent ensuite déplacer chaque allocation dans sa zone. Mais une partie des allocations ne se fait pas au site qui sait combien de temps l'objet vit :

- les **tableaux croissants partagés** (`pico8_ents_t`, `pico8_ptrs_t`) font leur `realloc` eux-mêmes. Pourtant, un même type sert à la fois au registre `hv` de toute la partie, aux listes `oe`/`cc` d'une planète et à une liste filtrée jetée à la fin de la frame ;
- les **listes fabriquées** (`jp`, `la`, `nv`, la concaténation et le filtre d'entités) rendent une liste neuve, que l'appelant libère ;
- les **textes fabriqués** (`tostr`, `sub`, `ne`, les concaténations) font leur `malloc` dans la fonction partagée. Or l'identifiant `"p"..id` d'une planète, qui vit toute la partie, est fait par les mêmes fonctions qu'un texte de menu jeté à la frame suivante ;
- la **boîte** `qz` est fabriquée par une fonction partagée ;
- les tableaux des **panneaux `lq`** (`items`, `fs`, et `ls` d'un `jb`) sont alloués et agrandis au fond du module de mise en page. Leur durée de vie est pourtant celle du panneau, que seul son créateur connaît.

Cinq fichiers recopient en plus la même concaténation statique (`concat2`), avec un `malloc` chacun. Une étape qui convertirait un seul appelant devrait aujourd'hui modifier la fonction partagée, donc changer la zone de **tous** ses appelants. Cela casse l'invariant du plan (un objet de partie logé en zone frame plante sous ASan), ou bien oblige à dupliquer chaque fonction par zone.

## Solution

L'étape 0b fait de la zone un **paramètre choisi par l'appelant**. Elle ne convertit encore aucun site.

- Une notion de **zone désignée** : une poignée qu'un propriétaire ou un appelant passe aux fonctions partagées. Elle sait allouer, agrandir et rendre. Deux sortes existent en 0b :
  - une poignée sur une **zone à pointeur montant** de 0a. Agrandir y alloue un nouveau bloc et recopie. Rendre n'y fait rien, puisque la mémoire repart à la remise à zéro ;
  - la **zone transitoire**, adossée à la libc. Allouer y appelle `malloc`, agrandir `realloc`, rendre `free`. Elle reproduit exactement le comportement actuel.
- Les **tableaux croissants partagés** (`ents`, `ptrs`) reçoivent leur zone à l'initialisation, de leur propriétaire, et la gardent : chaque croissance et la libération passent par elle.
- Les **fonctions partagées qui fabriquent** une liste, un texte ou une boîte reçoivent la zone de l'appelant : `jp`, `la`, `nv`, la concaténation et le filtre d'entités, `tostr`, `sub`, `ne`, `qz`, et une concaténation de textes partagée qui remplace les cinq copies statiques.
- Les **panneaux `lq` et `jb`** reçoivent de leur créateur la zone de leurs tableaux et la gardent pour `lq_add`.
- **Tous les appelants actuels passent la zone transitoire.** L'appelant libère encore comme aujourd'hui. Les captures restent identiques au pixel près, et tous les `*.sha256` de `tests/e2e/` restent inchangés.
- Le **verrou** compte chaque référence à la zone transitoire comme une allocation. Le compte d'un fichier mesure alors le travail qui lui reste : les étapes 1 à 4 le font baisser appel par appel, et l'étape 5 supprime la zone transitoire.

## User Stories

1. En tant que développeur de l'étape 1, je veux passer la zone frame à `tostr` pour un texte de menu et la zone transitoire ailleurs, pour convertir un appelant sans toucher aux autres.
2. En tant que développeur de l'étape 4, je veux passer la zone partie aux fonctions qui fabriquent l'identifiant d'une planète, pour qu'il vive toute la partie alors que les mêmes fonctions servent aux textes de frame.
3. En tant que développeur des étapes 1 à 4, je veux qu'un propriétaire de liste `ents` ou `ptrs` désigne sa zone une fois, à l'initialisation, pour que chaque `add` ultérieur grandisse dans la bonne zone sans repasser la zone à chaque appel.
4. En tant que développeur des étapes 1 à 4, je veux que la libération d'une liste passe par la zone qu'elle a gardée, pour que la même ligne de libération soit correcte en zone transitoire (vraie libération) et en zone à pointeur montant (rien à faire).
5. En tant que développeur de l'étape 1, je veux qu'une liste filtrée (`jp`) ou concaténée (`la`, concaténation d'entités) soit fabriquée dans la zone que je passe, pour la mettre en zone frame quand elle ne sert qu'à la frame.
6. En tant que développeur de l'étape 1, je veux que `nv` et le filtre d'entités prennent aussi une zone, pour qu'aucune liste fabriquée par le cœur n'échappe au choix de l'appelant.
7. En tant que développeur des étapes 1 et 4, je veux une seule concaténation de textes partagée, qui prend une zone, à la place des cinq copies statiques identiques, pour convertir chaque appel de concaténation à son propre rythme et dans sa propre zone.
8. En tant que développeur de l'étape 1, je veux que `ne` fabrique son résultat dans la zone de l'appelant et rende son texte intermédiaire par cette même zone, pour que `ne` n'ait plus aucun appel libc à lui.
9. En tant que développeur de l'étape 2, je veux que les tableaux d'un panneau `lq` (cellules, mise en page) et les lignes d'un `jb` prennent la zone désignée par le créateur du panneau, pour que leur durée de vie suive celle du panneau.
10. En tant que développeur de l'étape 2, je veux que `lq_add` agrandisse les cellules dans la zone gardée par le panneau, pour qu'un ajout tardif ne retombe pas dans la libc.
11. En tant que développeur, je veux que la boîte `qz` soit fabriquée dans la zone de l'appelant, pour que la seule boîte fabriquée par une fonction partagée suive la règle commune.
12. En tant que développeur, je veux qu'agrandir un tableau dans une zone à pointeur montant garde ses éléments dans l'ordre, pour que `add`, `del` et l'itération se comportent comme en libc.
13. En tant que développeur, je veux qu'un itérateur `all()` survive à la croissance en zone à pointeur montant comme en libc, pour que la tolérance à la suppression pendant le parcours ne dépende pas de la zone.
14. En tant que développeur qui chasse un accès hors durée de vie, je veux qu'une liste grandie dans une zone à pointeur montant, puis lue après la remise à zéro de cette zone, plante sous ASan, pour que l'oracle du plan couvre aussi les tableaux croissants.
15. En tant que développeur, je veux qu'une zone pleine atteinte par une croissance appelle `pico8_fatal` avec le nom de la zone, pour qu'une liste trop longue se lise comme une limite à relever.
16. En tant que développeur, je veux qu'une liste initialisée sans zone soit refusée par un `assert` porteur de message, pour qu'un oubli ne retombe pas en silence dans une zone par défaut.
17. En tant que mainteneur de l'invariant, je veux que tous les appelants actuels passent la zone transitoire, pour que l'étape ne change ni un pixel ni un `*.sha256`.
18. En tant que mainteneur de l'invariant, je veux que la zone transitoire se comporte exactement comme `malloc`/`realloc`/`free`, pour que `make sanitize-net` voie le même programme qu'avant, fuites comprises.
19. En tant que développeur, je veux que l'appelant d'un texte fabriqué en zone transitoire le libère encore avec `free`, comme aujourd'hui, pour que l'étape ne touche à aucune ligne de libération des sites du jeu.
20. En tant que gardien du cliquet, je veux que chaque référence à la zone transitoire dans un objet du cœur compte comme un appel d'allocation, pour que le compte d'un fichier dise combien d'appels il reste à convertir.
21. En tant que gardien du cliquet, je veux que le fichier de la zone transitoire figure dans la liste d'exceptions avec son compte, pour que ses propres appels libc soient visibles jusqu'à sa suppression à l'étape 5.
22. En tant que relecteur, je veux que la liste d'exceptions soit remesurée et réécrite à la main dans les commits de l'étape, avec le pourquoi des hausses dans le message de commit, pour que le déplacement des comptes (des fonctions partagées vers leurs appelants) soit lisible dans le diff.
23. En tant que développeur des étapes 1 à 4, je veux que la note « Découvert » de 0b donne, fichier par fichier, les comptes laissés par 0b, pour savoir combien d'appels chaque étape doit retirer.
24. En tant que développeur, je veux que le module de zone de 0a reste hors de la liste d'exceptions, pour que la zone transitoire soit le seul code de zone qui touche la libc.
25. En tant que futur porteur PSX, je veux que la suppression de la zone transitoire à l'étape 5 se résume à supprimer un fichier et sa ligne d'exception, pour qu'il ne reste aucun appel libc caché dans le module de zone désignée.
26. En tant que développeur, je veux que les fonctions de fabrication locales à un seul fichier (`jo_dollar`, `concat_percent`, la jointure de `hp`, le texte de `pj`) et les boîtes `vec` écrites en ligne restent des sites ordinaires, pour qu'elles soient converties par leur étape comme n'importe quel `malloc` du jeu.
27. En tant que développeur, je veux que les tests unitaires des listes, des textes et des panneaux prouvent le même comportement dans une zone à pointeur montant et dans la zone transitoire, pour que le choix de zone ne change que la durée de vie.
28. En tant que développeur, je veux qu'aucune zone du jeu (frame, interface, particules, partie) ne soit encore instanciée en 0b, pour que le BSS ne grossisse pas et que l'étape reste un socle.

## Implementation Decisions

- **Zone désignée.** Un petit module du cœur, distinct du module de zone de 0a, définit la poignée et ses trois opérations :
  - **allouer** `n` octets ;
  - **agrandir** un bloc de l'ancienne à la nouvelle taille, en gardant le contenu ;
  - **rendre** un bloc.

  La poignée porte ses opérations par pointeurs de fonction. Le module ne référence donc jamais la zone transitoire, et la supprimer à l'étape 5 ne touche pas ce module. Forme indicative :

  ```c
  typedef struct pico8_zone_choisie {
      void *(*alloc)(const struct pico8_zone_choisie *z, size_t n);
      void *(*grow)(const struct pico8_zone_choisie *z, void *p, size_t old_n, size_t new_n);
      void  (*release)(const struct pico8_zone_choisie *z, void *p);
      void *impl; /* la zone de 0a, ou NULL pour la transitoire */
  } pico8_zone_choisie_t;
  ```

  Le nom définitif reste au choix de l'implémenteur. Il s'aligne sur le vocabulaire du module de 0a (H-0a-1).
- **Poignée sur zone à pointeur montant.** On l'obtient à partir d'une zone de 0a.
  - Allouer délègue à l'allocation de 0a, qui appelle `pico8_fatal` quand la zone est pleine.
  - Agrandir alloue `new_n` octets puis recopie `old_n` octets. L'ancien bloc reste jusqu'à la remise à zéro.
  - Rendre ne fait rien.

  Aucune zone du jeu n'est instanciée en 0b : seuls les tests créent de telles poignées, sur un stockage local.
- **Zone transitoire.** C'est un fichier du cœur à part, dans `CORE_SRCS`, qui expose une seule poignée globale constante. Ses opérations sont `malloc`, `realloc` et `free`, avec `abort` sur `NULL` comme aujourd'hui. Elle n'entre pas dans le budget de 1,5 Mo : elle n'a pas de taille, et elle disparaît à l'étape 5 (H-0a-6).
- **Le cliquet compte la zone transitoire.** Le verrou de 0a ajoute le symbole de la poignée transitoire à la liste des symboles comptés. Chaque relocation vers elle dans un objet du cœur compte pour une allocation, dans le même compte par fichier que `malloc`, `calloc`, `realloc` et `free` (H-0a-4).
  - Le fichier de la zone transitoire entre dans la liste d'exceptions avec ses appels libc.
  - Le module de zone désignée n'y entre pas.
  - Conséquence assumée : les comptes **se déplacent**. Ils baissent dans les fonctions partagées, qui n'appellent plus la libc, et montent chez leurs appelants, d'une unité par référence à la zone transitoire. 0b est la seule étape où un compte peut monter. Le message de chaque commit en donne la raison, et la liste reste à égalité stricte.
- **`ents` et `ptrs`.**
  - L'initialisation prend la zone et la garde dans le conteneur ; une zone `NULL` est refusée par un `assert` porteur de message. La croissance passe par **agrandir**, la libération par **rendre**.
  - Le conteneur garde son type, sa forme (`items`, `count`, `capacity`, doublement à partir de 4) et ses contrats d'itération.
  - Les fonctions de libération gardent leur nom et leur contrat (« libère `items` seulement, laisse le conteneur vide ») ; seul leur mécanisme change.
  - Les trois conteneurs statiques (`hv`, le `by` partagé des `ga`, le `le` partagé de `ui`) reçoivent la transitoire à leur initialisation, comme tous les autres.
- **Listes fabriquées.** `jp_ptrs`, `la_ptrs`, `nv_ptrs`, `ents_cat` et `jp_ents` prennent la zone de la liste qu'elles rendent. L'appelant la libère toujours avec la fonction de libération du conteneur, qui passe par la zone gardée.
- **Textes.**
  - `tostr` et `sub` prennent une zone.
  - Une concaténation de deux textes partagée, dans le module des chaînes, prend une zone. Elle remplace les cinq copies statiques identiques (galaxie, menu, icône de recherche, tableau des scores, affichage du tableau des scores) : un déplacement de code à comportement identique.
  - `ne` prend une zone. Elle y fabrique son résultat, et y fabrique puis rend son texte intermédiaire par **rendre**.
  - L'appelant d'un texte fabriqué en zone transitoire le libère encore avec `free` : aucune ligne de libération d'un site du jeu ne change en 0b.
  - Les fabricants locaux à un fichier (`jo_dollar`, `concat_percent`, la jointure de `hp`, le texte de `pj`) ne sont pas partagés. Ils restent des sites, que l'étape 1 convertira.
- **Boîtes.**
  - `qz` prend une zone.
  - Les boîtes `vec` et les listes enveloppées s'écrivent aujourd'hui en ligne (`malloc(sizeof *heap)` au site), sans fonction partagée. Ce sont des sites ordinaires : chaque site choisira sa zone à son étape. 0b ne crée pas de fonction de boîte `vec`, puisque sans site converti elle n'aurait aucun appelant. Cet écart de lecture avec la puce du plan (« boîtes `vec` ») va dans la note « Découvert ».
- **Panneaux `lq` et `jb`.**
  - Les fonctions de création (`lq_new`, `lq_copy`, `jb_new`) prennent la zone des tableaux du panneau : `items`, `fs`, `ls`. Le panneau la garde, et `lq_add` agrandit `items` par **agrandir**.
  - Les lignes qu'un `jb` crée héritent de sa zone.
  - L'objet panneau lui-même reste alloué comme aujourd'hui. C'est un site de l'étape 2 (zone interface), pas un tableau croissant.
  - Le choix de la zone des tableaux d'un panneau qui vivra dans une zone à places est l'affaire de l'étape 2.
- **Appelants.** Tous les sites du cœur et des tests qui appellent une fonction modifiée passent la zone transitoire, sans autre changement. Ordre de grandeur relevé sur `HEAD` : environ 150 appels dans `src/` et autant dans `tests/`.
- **Commentaires.** Ils suivent les quatre destinations de `CLAUDE.md`. Le contrat « qui libère » des en-têtes modifiés est réécrit : « dans la zone passée ; rendu par cette zone », et « l'appelant libère avec `free` » ne reste vrai que pour la transitoire. Le pourquoi de la copie à la croissance en zone à pointeur montant est un piège qu'on réintroduirait (un `realloc` direct) : il tient en un commentaire court.
- **Invariant.** À chaque commit, `make test-all` et `make sanitize-net` passent, et tous les `*.sha256` restent identiques.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant et remet la liste d'exceptions à égalité stricte.

1. **Zone désignée et zone transitoire.** Le module de poignée, la poignée sur zone à pointeur montant, le fichier de la zone transitoire, l'ajout de son symbole au verrou et la ligne d'exception de ce fichier. Les tests du module et du verrou. Aucun appelant du jeu ne change.
2. **Listes.** `ents`, `ptrs` et les cinq fabricants de listes prennent leur zone ; tous leurs appelants passent la transitoire. La liste d'exceptions est remesurée : `ents` et `ptrs` en sortent, les appelants montent.
3. **Textes et boîte.** `tostr`, `sub`, `ne`, la concaténation partagée (les cinq copies supprimées) et `qz` ; tous leurs appelants passent la transitoire. Remesure.
4. **Panneaux.** `lq_new`, `lq_copy`, `jb_new` et `lq_add` ; tous leurs appelants passent la transitoire. Remesure.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe le comportement externe : le contenu et l'ordre d'une liste, le texte rendu, un code de sortie, un rapport ASan, la sortie du verrou. Il ne regarde ni la capacité interne ni l'adresse d'un bloc.
- **Zone désignée** : testée dans `make test`, dans la règle ASan de 0a (H-0a-5).
  - Une liste grandie bien au-delà de 4 éléments dans une poignée sur zone à pointeur montant garde tous ses éléments, dans l'ordre.
  - Une liste grandie puis lue **après** la remise à zéro de sa zone, et après de nouvelles allocations, plante (`use-after-poison`), dans un processus enfant.
  - Cas de contrôle : la même liste lue avant la remise à zéro ne plante pas.
  - Une croissance qui dépasse la zone sort avec le code de `pico8_fatal`, et le message nomme la zone (H-0a-3).
  - La zone transitoire alloue, agrandit et rend sans rapport ASan.
- **`ents` et `ptrs`** : les tests existants (`test_pico8_ents`, `test_pico8_ptrs`) passent la transitoire et restent verts sans autre changement. On y ajoute les cas d'itération avec suppression pendant le parcours, et la libération, rejoués sur une poignée à pointeur montant.
- **Textes** : `test_pico8_str` reçoit les mêmes témoins dans les deux sortes de zones. La concaténation partagée a son propre cas : deux textes, un texte vide de chaque côté. `ne` est vérifié sur un signe positif et un négatif, avec et sans suffixe, en zone à pointeur montant ; son texte intermédiaire n'y fuit pas.
- **Panneaux** : `test_pico8_lq` et `test_pico8_jb_ol` passent la transitoire. On ajoute un cas `lq_add` sur un panneau en zone à pointeur montant, dont le dessin reste le même qu'en transitoire.
- **Verrou** : sur des sources fabriquées puis compilées, comme `test_float_guard` :
  - un fichier qui référence la poignée transitoire sans être listé échoue ;
  - deux références à la poignée transitoire et un `free` comptent 3 ;
  - la forme Darwin préfixée `_` du symbole est reconnue.
- **Invariant** : `make e2e` et `make sanitize-net`, avec des `*.sha256` inchangés, prouvent que le passage par la transitoire ne change rien.
- **Exemples dans le dépôt** :
  - `tests/test_float_guard.c` (garde testée sur sources fabriquées) ;
  - `tests/test_pico8_replay_usage.c` (processus enfant, statut et `stderr`) ;
  - les tests du module de zone de 0a (règle ASan, `detect_leaks=0`).

## Out of Scope

- Toute conversion d'un site du jeu vers une zone du jeu : en 0b, chaque appelant passe la transitoire. C'est le travail des étapes 1 à 4.
- L'instanciation des zones frame, interface, particules et partie.
- Les `malloc` écrits en ligne aux sites : objets du jeu, boîtes `vec`, listes enveloppées, objets panneau et `jb` eux-mêmes, fabricants de texte locaux à un fichier.
- Une variante de poignée sur zone à places. L'étape 2 l'ajoutera si les tableaux d'un panneau doivent y vivre.
- Le remplacement des `free` d'appelants par **rendre** : l'appelant libère encore, comme le dit le plan.
- La suppression de la zone transitoire et de `TROU-7`, qui revient à l'étape 5.
- Les autres appels libc du cœur (§7 du plan).
- Lancer l'application raylib.

## Hypothèses sur l'étape 0a

Au moment où cette PRD est écrite, 0a est en cours. Ces hypothèses viennent de sa PRD (`docs/prd/triage/phase1-etape0a-socle-outils.md`) et du plan ; elles ne sont pas encore vérifiées.

- **H-0a-1** — Le module de zone de 0a expose une zone à pointeur montant avec une initialisation sur stockage fourni et nommé, une allocation de `n` octets qui appelle `pico8_fatal` quand la zone est pleine, et une remise à zéro. Il n'offre ni libération individuelle ni `realloc` : **agrandir** doit donc allouer puis recopier. Les noms exacts des types et fonctions sont inconnus.
- **H-0a-2** — Les blocs rendus par la zone à pointeur montant de 0a sont alignés pour un pointeur et un `pico8_fixed_t`. Sinon, la poignée devra aligner elle-même.
- **H-0a-3** — `pico8_fatal` sort avec le code 3, et le message composé par le module nomme la zone et sa limite. Toute ligne de link qui tire le module de zone lie aussi l'implémentation PC : tests, rejeu, raylib.
- **H-0a-4** — Le verrou est un script sous `tools/` avec une liste d'exceptions d'une ligne `fichier compte` par fichier, à égalité stricte. Il compte par objet les relocations vers une liste de symboles fixée dans le script, préfixe `_` de Darwin compris. On peut ajouter un symbole à cette liste sans changer le format de la liste d'exceptions.
- **H-0a-5** — `make test` contient une règle qui compile et lance des tests du cœur sous ASan (`detect_leaks=0`), et un test de 0b peut s'y ajouter.
- **H-0a-6** — L'en-tête central des tailles ne déclare que les quatre zones du §2. La transitoire n'y a pas sa place, et la vérification de 1,5 Mo ne la voit pas.
- **H-0a-7** — 0a n'a changé ni les signatures ni les fichiers de `ents`, `ptrs`, des chaînes, des fabricants de listes, de `qz` ni de la mise en page. Les fichiers qui allouent sont dans la liste initiale avec leurs comptes mesurés (environ 36 fichiers ; par exemple `pico8_ents.c` et `pico8_ptrs.c` à 1, `pico8_str.c` à 3, `pico8_layout.c` autour de 6).
- **H-0a-8** — Le module de zone de 0a est dans `CORE_SRCS` et hors de la liste d'exceptions. `pico8_input.c` a été coupé, et `pico8_coverage.c` est sorti du cœur, sans effet sur les fichiers que touche 0b.

## À revérifier une fois 0a finie

Quand la note « Découvert » de 0a existe sous son entrée du plan, il faut relire :

- **Solution** et **Implementation Decisions — Zone désignée / Poignée sur zone à pointeur montant** : contre l'API réelle du module de zone (H-0a-1, H-0a-2). Si 0a fournit déjà une poignée commune ou un agrandissement en place, il faut simplifier la poignée.
- **Implementation Decisions — Le cliquet compte la zone transitoire** et le morceau 1 : contre le format réel du verrou et de sa liste (H-0a-4). Si le script ne prend pas un symbole de plus sans changer de format, il faut décider ici comment compter.
- **Implementation Decisions — Zone transitoire** : contre l'en-tête des tailles (H-0a-6).
- **Testing Decisions — Zone désignée** : contre la règle ASan et le code de sortie réels (H-0a-3, H-0a-5).
- **Implementation Chunks 2 à 4** et **User Stories 20–23** : contre les comptes de la liste initiale (H-0a-7), pour annoncer les déplacements de comptes attendus.
- **Out of Scope** : vérifier que 0a n'a rien tiré de 0b (H-0a-7, H-0a-8).

## Further Notes

- Les points que `to-prd` fait valider par un humain (modules attendus, modules à tester, découpage) ont été tranchés en appliquant les principes du skill `guideline`. Aucun agent séparé n'était disponible dans cette session.
  - *Simplicité* : pas de fonction de boîte `vec` sans appelant, et pas de variante sur zone à places.
  - *Changements chirurgicaux* : les appelants passent la transitoire et gardent leur `free`.
  - *Hypothèses explicites* : voir ci-dessus.
  - Le seul regroupement de code retenu est la fusion des cinq `concat2` identiques. C'est la puce « concaténations » du plan, et sans cette fusion l'étape 1 devrait convertir cinq copies dont les appelants ont des durées de vie mêlées (l'identifiant de planète de la galaxie est une donnée de partie).
- Mesure faite sur `HEAD` pour cette PRD, par grep sur les sources, donc à remesurer sur objets :
  - `tostr` : 28 appels dans 12 fichiers ;
  - `ents_init` : 24 appels ;
  - `ents_free` : 16 ;
  - `ptrs_init` : 11 ;
  - `lq_new` : 10 ;
  - les cinq `concat2` : 23 appels.
- À la fin de l'étape, `plan-pilot` ajoute sous l'entrée 0b du plan sa note « Découvert ». Elle porte au minimum :
  - les comptes du cliquet laissés par 0b, fichier par fichier ;
  - la lecture retenue pour les boîtes `vec` ;
  - le sort des hypothèses H-0a-*.
