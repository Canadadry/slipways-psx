---
title: "Plan 4, étape 0a — Socle : outils des zones mémoire"
description: "Il manque au cœur une erreur fatale de plateforme, un module de zone éprouvé sous ASan, un plafond de 1,5 Mo vérifié et un verrou qui empêche malloc de revenir : sans eux, aucune conversion des étapes suivantes n'est sûre."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc`, `free` dans le cœur (`CORE_SRCS`), pour que toute la mémoire du jeu tienne dans des zones de taille fixe connue à la compilation, en vue de la reprise PSX. Aujourd'hui, rien ne permet de commencer :

- le cœur n'a aucun moyen de signaler qu'une limite est atteinte. Le plan exige une erreur fatale fournie par la plateforme, sans refus silencieux ni changement du jeu ;
- il n'existe aucun allocateur de zone. Rien ne garantit non plus qu'un accès après la durée de vie d'un objet fera planter le rejeu sous ASan, alors que c'est l'oracle du §4 du plan. Or une zone qui réutilise tout de suite les mêmes adresses masque ce bug : le pointeur périmé retombe sur de la mémoire redevenue valide ;
- rien ne vérifie que la somme des zones tient dans 1,5 Mo, le seul garde-fou du §3 ;
- rien n'empêche un commit de réintroduire une allocation libc dans le cœur, ni dans un fichier à moitié converti. Environ 37 fichiers du cœur allouent aujourd'hui ;
- deux fichiers d'hôte sont classés dans le cœur. `pico8_input.c` mêle les primitives d'entrée du jeu (`btn`, `stat`, pas de frame) et la lecture/écriture d'`events.json`, qui alloue. `pico8_coverage.c` est de l'instrumentation qui n'alloue que sous `-DPICO8_CLASS_COVERAGE`. Sans décision sur ces deux fichiers, le verrou serait aveugle ou vert par accident.

## Solution

L'étape 0a pose les outils, et seulement eux. **Aucun site du jeu n'est converti** : les captures restent identiques au pixel près et tous les `*.sha256` de `tests/e2e/` restent inchangés.

- `pico8_fatal(message)` est déclarée pour le cœur. Son implémentation PC écrit sur `stderr` un message qui nomme la zone et sa limite, puis sort avec le code 3. Le pilote de rejeu et le binaire raylib la lient tous les deux.
- Un module de zone générique couvre deux sortes de zones :
  - la zone à pointeur montant, remise à zéro d'un coup ;
  - la zone à places de taille fixe, dont chaque place est rendue une à une.

  Le module empoisonne la mémoire sous ASan. Il est conçu pour qu'un pointeur périmé fasse encore planter le programme **après** que la zone a servi de nouvelles allocations.
- Un en-tête central déclare la taille de chaque zone. La compilation échoue si leur somme dépasse 1,5 Mo.
- Le **verrou**, dans `make test`, compte les appels d'allocation libc de chaque objet du cœur. Il échoue sur tout fichier qui alloue sans être dans la liste d'exceptions, et sur tout écart avec le compte inscrit pour ce fichier : c'est un cliquet.
- `pico8_input.c` est coupé en deux. Les primitives restent dans le cœur, les entrées-sorties JSON partent dans un fichier d'hôte. `pico8_coverage.c` sort de `CORE_SRCS`.

## User Stories

1. En tant que développeur du portage, je veux que le cœur appelle `pico8_fatal(message)` quand une zone est pleine, pour qu'une limite trop basse arrête le programme au lieu de corrompre la mémoire ou de modifier le jeu.
2. En tant que développeur du portage, je veux que le message de `pico8_fatal` sur PC nomme la zone et sa limite, pour savoir quel chiffre relever sans déboguer.
3. En tant que lecteur d'un e2e en échec, je veux que `pico8_fatal` sorte avec un code distinct de ceux du pilote (1, 2) et d'ASan (1), pour reconnaître une limite atteinte à son seul code de sortie.
4. En tant que mainteneur du backend raylib, je veux que le binaire interactif lie la même implémentation PC de `pico8_fatal` que le rejeu, pour qu'une limite atteinte en jouant produise le même message qu'en e2e.
5. En tant que futur porteur PSX, je veux que `pico8_fatal` soit seulement déclarée par le cœur et fournie par la plateforme, pour brancher plus tard une implémentation PSX sans toucher au cœur.
6. En tant que développeur des étapes 1 et 4, je veux une zone à pointeur montant qui alloue en avançant un pointeur et se remet à zéro d'un coup, pour loger les textes et boîtes de frame, puis les objets de partie, sans `free`.
7. En tant que développeur des étapes 2 et 3, je veux une zone à places de taille fixe où chaque place se rend individuellement, pour loger panneaux, messages et particules avec un nombre maximum d'objets vivants.
8. En tant que développeur, je veux que le module de zone travaille sur un stockage fourni par le propriétaire de la zone, pour que la mémoire soit un tableau statique de taille connue à la compilation, sans aucune allocation dynamique.
9. En tant que développeur, je veux qu'une zone pleine appelle `pico8_fatal`, avec la même limite dans les builds avec et sans ASan, pour qu'un e2e échoue de la même façon partout.
10. En tant que développeur qui chasse un accès hors durée de vie, je veux qu'une remise à zéro empoisonne toute la mémoire de la zone sous ASan, pour qu'un pointeur gardé de la frame ou de la partie précédente plante.
11. En tant que développeur, je veux qu'un pointeur gardé d'avant une remise à zéro plante encore **après** de nouvelles allocations dans la zone, pour que la réutilisation des adresses ne masque pas le bug.
12. En tant que développeur, je veux qu'une place rendue soit empoisonnée et ne soit pas réattribuée tout de suite, pour qu'un pointeur vers elle plante encore après d'autres allocations dans la zone.
13. En tant que développeur, je veux que chaque allocation ne désempoisonne que ses propres octets, pour qu'un débordement au-delà d'un objet soit lui aussi détecté.
14. En tant que développeur, je veux qu'un build sans ASan garde un comportement simple (remise à zéro qui ramène le pointeur au début), pour que le rejeu normal et le futur PSX ne paient pas la machinerie de diagnostic.
15. En tant que gardien du budget PSX, je veux que la taille de chaque zone soit déclarée à un seul endroit, pour lire d'un coup d'œil la somme des zones.
16. En tant que gardien du budget PSX, je veux que la compilation échoue si la somme des zones dépasse 1,5 Mo, pour qu'aucune étape ne puisse relever une limite au-delà du plafond sans s'en apercevoir.
17. En tant que gardien du budget PSX, je veux que le budget compte les tailles réelles (hors doublement ASan), pour que le plafond mesure ce que la PSX paiera.
18. En tant que développeur des étapes 1 à 4, je veux que `make test` échoue si un objet du cœur référence `malloc`, `calloc`, `realloc` ou `free` hors de la liste d'exceptions, pour qu'aucune allocation ne revienne dans un fichier déjà propre.
19. En tant que développeur des étapes 1 à 4, je veux que la liste d'exceptions inscrive pour chaque fichier son nombre d'appels d'allocation restants, pour qu'un fichier à moitié converti ne puisse pas regagner un appel.
20. En tant que relecteur d'un commit de conversion, je veux que `make test` échoue aussi quand le compte réel passe sous le compte inscrit, pour que chaque baisse soit écrite dans le commit qui la produit et visible dans son diff.
21. En tant que développeur, je veux que `make test` échoue quand un fichier listé n'alloue plus, ou n'est plus dans `CORE_SRCS`, pour que la liste d'exceptions dise toujours la vérité.
22. En tant que développeur, je veux que le message d'échec du verrou donne la ligne exacte à écrire dans la liste, pour la corriger sans deviner le format.
23. En tant que développeur, je veux que le verrou compte les appels sur les objets compilés et non sur le texte source, pour que commentaires et identifiants ne faussent pas le compte.
24. En tant que développeur sous macOS, je veux que le verrou reconnaisse les symboles préfixés d'un `_`, pour que la garde ne passe pas au vert par erreur sur Darwin.
25. En tant que développeur, je veux que le verrou échoue s'il ne reçoit aucun objet, pour qu'une garde sur rien ne passe jamais pour verte (même règle que `float-guard`).
26. En tant que développeur, je veux que la liste d'exceptions initiale contienne exactement les fichiers du cœur qui allouent aujourd'hui, avec leurs comptes mesurés, pour que le verrou soit vert le jour où il est posé.
27. En tant que futur porteur PSX, je veux que `btn`, `stat` et le pas de frame restent dans le cœur, pour que le jeu garde ses primitives d'entrée sur PSX.
28. En tant que développeur, je veux que la lecture et l'écriture d'`events.json` (`load`, `save`, `append`, `free`) quittent le cœur pour un fichier d'hôte, pour que leurs allocations ne rendent pas le verrou aveugle sur un fichier que le cœur utilise vraiment.
29. En tant que développeur, je veux que `pico8_coverage.c` sorte de `CORE_SRCS` comme `pico8_assets.c` et `pico8_dump.c`, pour que l'instrumentation d'hôte soit classée comme telle plutôt que verte par accident.
30. En tant que mainteneur de l'invariant du plan, je veux que la coupe de `pico8_input.c` et les changements de listes de sources gardent `make test-all`, `make sanitize-net` et tous les `*.sha256` identiques, pour prouver que cette étape ne change pas un pixel.
31. En tant que développeur, je veux que le module de zone naisse dans `CORE_SRCS`, sous le verrou déjà posé, pour que le verrou prouve d'emblée que le module n'alloue pas.
32. En tant que développeur des étapes suivantes, je veux qu'aucune zone du jeu ne soit encore instanciée ni utilisée en 0a, pour que cette étape reste un socle sans effet sur le jeu ni sur sa mémoire.

## Implementation Decisions

- **`pico8_fatal`.**
  - Un en-tête du cœur déclare `pico8_fatal(message)` comme fonction de plateforme qui ne rend pas la main.
  - L'implémentation PC est un fichier d'hôte, hors de `CORE_SRCS`, comme `pico8_assets.c`. Elle écrit le message sur `stderr` puis sort avec le **code 3**, distinct des codes 1 et 2 du pilote et du code 1 d'ASan.
  - Le message nomme la zone et sa limite. C'est le module de zone qui le compose, à partir du nom de la zone et de sa capacité.
  - Toute ligne de link qui tire le module de zone lie aussi l'implémentation PC : le rejeu, les binaires de test qui lient la closure du moteur, et le binaire raylib. Ce dernier énumère ses sources à la main dans le `Makefile` et doit l'y recevoir explicitement. Il est seulement compilé et lié, jamais lancé (`CLAUDE.md`).
- **Module de zone générique**, dans `CORE_SRCS`.
  - Il opère sur un stockage fourni par le propriétaire de la zone, déclaré par une macro de stockage. Il ne crée aucune instance en 0a et ne réserve aucun tableau statique : le BSS ne grossit pas.
  - Chaque zone porte un nom, qui sert au message de `pico8_fatal`.
  - **Zone à pointeur montant** : on y alloue et on la remet à zéro.
    - Sans ASan, la remise à zéro ramène simplement le pointeur au début.
    - Sous `__SANITIZE_ADDRESS__` seulement, la macro de stockage réserve deux fois la capacité, et chaque remise à zéro alterne entre les deux moitiés. La moitié quittée est empoisonnée en entier (`ASAN_POISON_MEMORY_REGION`), et chaque allocation ne désempoisonne que ses propres octets.
    - Un pointeur gardé d'avant une remise à zéro tombe donc dans la moitié empoisonnée pendant toute la durée de vie suivante.
    - La limite et le déclenchement de `pico8_fatal` restent ceux de la capacité réelle, identiques dans les deux builds.
  - **Zone à places de taille fixe** : on y prend une place et on la rend.
    - L'allocation cherche la place libre suivante à partir d'un curseur tournant, pas dans une liste libre LIFO. Une place rendue n'est donc réattribuée qu'après un tour complet du curseur.
    - Le comportement est le même dans les deux builds. Le seul code propre à ASan est l'empoisonnement d'une place rendue et le désempoisonnement d'une place prise.
    - `pico8_fatal` se déclenche quand un tour entier ne trouve aucune place libre.
  - Les macros d'empoisonnement ne coûtent rien hors ASan.
- **Budget de 1,5 Mo.**
  - Un en-tête central déclare la taille de chacune des zones du §2 du plan (frame, interface, particules, partie). Les valeurs sont provisoires, « au doigt mouillé » (§3 du plan) ; les étapes 1 à 4 les ajusteront en changeant un chiffre.
  - Une vérification à la compilation (typedef de tableau à taille négative, C99) fait échouer le build si la somme dépasse 1,5 Mo.
  - Le budget compte les tailles sans le doublement ASan.
  - Cet en-tête ne réserve aucune mémoire.
- **Verrou et cliquet.**
  - Ils suivent le modèle de `float-guard` : un script sous `tools/` et un fichier de liste d'exceptions, branchés dans `make test`.
  - Le script reçoit les objets de `CORE_SRCS`, compilés avec les flags par défaut. Il échoue sans objet, comme `float_guard.sh` qui rend 2 sans source.
  - Pour chaque objet, il compte les relocations vers `malloc`, `calloc`, `realloc` et `free`, en reconnaissant aussi la forme préfixée `_` de Darwin. Ce compte sur objet est fiable, alors que le grep de source en compte trop à cause des commentaires.
  - Chaque ligne de la liste porte un fichier et son nombre d'appels d'allocation restants.
  - `make test` exige l'**égalité stricte**. Il échoue dans chacun de ces cas :
    - un fichier alloue sans être listé ;
    - un compte réel diffère du compte inscrit, plus haut ou plus bas ;
    - un fichier listé n'alloue plus ;
    - un fichier listé n'est plus dans `CORE_SRCS`.
  - Le message d'échec donne la ligne exacte à écrire.
  - La liste se met à jour à la main, sans cible de réécriture, pour que chaque baisse apparaisse dans le diff du commit qui convertit.
  - La liste initiale est mesurée après la coupe de `pico8_input.c`. Elle contient tous les fichiers du cœur qui allouent, environ 36 d'après la mesure actuelle hors `pico8_input.c`, chacun avec son compte.
  - Elle ne contient aucune exception permanente : les deux fichiers d'hôte sont tranchés par sortie du cœur, pas par exception.
- **Sort des fichiers d'hôte classés dans le cœur.**
  - `pico8_input.c` est coupé en deux par simple déplacement de code :
    - `init`, `step`, `btn` et `stat` restent dans le cœur, car `pico8_menu.c`, `pico8_slipway.c`, la souris et le défilement en dépendent ;
    - `load`, `save`, `append`, `free` et la lecture JSON partent dans un nouveau fichier d'hôte, hors de `CORE_SRCS`, qui emporte les 17 appels d'allocation.

    Les contrats décrits dans l'en-tête restent inchangés. Les règles du `Makefile` qui lient `pico8_input.c` (tests d'entrée, de rejeu, d'enregistrement, `minimize`, `gif`, raylib, visualiseur) lient aussi le fichier d'hôte quand elles en ont besoin.
  - `pico8_coverage.c` rejoint le `filter-out` de `CORE_SRCS`, à côté de `pico8_assets.c` et `pico8_dump.c`.
  - Ces sorties retirent aussi ces fichiers des autres gardes qui partent de `CORE_SRCS` (`float-guard`, `float-grep`, `constants-check`). C'est voulu : ce sont des outils d'hôte (§1 du plan).
- **Commentaires.** Ils suivent les quatre destinations de `CLAUDE.md`. Le pourquoi du curseur tournant et des deux moitiés sous ASan est un piège qu'on réintroduirait : il tient en un commentaire court sur chacune des deux zones. Le reste va dans le message de commit, dont la vérification manuelle du budget (voir Testing Decisions).
- **Invariant.** À chaque commit, `make test-all` et `make sanitize-net` passent, et tous les `*.sha256` de `tests/e2e/` restent identiques.

## Implementation Chunks

1. **Préparation des listes de sources.** Couper `pico8_input.c` (primitives du cœur et entrées-sorties JSON d'hôte), et ajouter `pico8_coverage.c` au `filter-out` de `CORE_SRCS`. C'est un pur déplacement : `make test-all` et `make sanitize-net` restent verts, les `.sha256` identiques.
2. **Verrou et cliquet.** Le script, ses tests sur objets fabriqués, la liste d'exceptions initiale mesurée sur les objets après le morceau 1, puis le branchement dans `make test` et la légende du `Makefile`.
3. **Zones et erreur fatale.** `pico8_fatal` (en-tête du cœur et implémentation PC liée par le rejeu, les tests et raylib), le module de zone dans `CORE_SRCS`, ses tests sous ASan, et l'en-tête central des tailles avec sa vérification de 1,5 Mo à la compilation. Le module naît sous le verrou, qui prouve tout de suite qu'il n'alloue pas.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe le comportement externe : un code de sortie, un rapport ASan, la sortie d'un script. Il ne regarde jamais la disposition interne de la zone (curseur, moitié courante).
- **Module de zone : testé, compilé sous ASan, dans `make test`.**
  - Il faut une règle qui prend les objets de l'arbre instrumenté, puisque `make test` compile sans ASan par défaut. Le lancement se fait avec `detect_leaks=0`, comme `test_pico8_planet_gh_row_lq`.
  - Chaque cas de crash tourne dans un processus enfant. Le parent vérifie un statut non nul et `use-after-poison` dans `stderr`.
  - Cas obligatoires :
    - un pointeur gardé d'avant une remise à zéro de la zone à pointeur montant, lu **après** que la zone a servi de nouvelles allocations, plante ;
    - un pointeur vers une place rendue, lu **après** que la zone à places a servi de nouvelles allocations, plante ;
    - le dépassement de la zone à pointeur montant sort avec le code 3, et le message nomme la zone et sa limite ;
    - une zone à places pleine sort avec le code 3, avec le même message.
  - Cas de contrôle, pour qu'un test qui plante toujours ne passe pas pour une preuve : une allocation valide, lue et écrite dans sa durée de vie, ne plante pas.
  - Ce critère est celui du plan : sans lui, la réutilisation masque le bug et l'oracle du §4 ne tranche rien.
- **`pico8_fatal`** : pas de test séparé. Les deux cas « zone pleine » vérifient son code de sortie et son message dans un processus enfant.
- **Script du verrou : testé sur des sources fabriquées puis compilées**, comme `test_float_guard`. Cas :
  - un fichier qui appelle `malloc` sans être listé → échec ;
  - un compte inscrit plus haut que le réel → échec ;
  - un compte inscrit plus bas que le réel → échec ;
  - un fichier listé qui n'alloue plus → échec ;
  - un fichier listé absent des objets reçus → échec ;
  - des comptes exacts → succès ;
  - aucun objet → échec distinct ;
  - un symbole préfixé `_` à la Darwin → reconnu (par exemple via une liste de symboles simulée, sans dépendre de la plateforme).
- **Budget de 1,5 Mo** : pas de test unitaire. La vérification est une assertion à la compilation sur un seul en-tête. On vérifie une fois à la main qu'un dépassement casse le build, et on le dit dans le message de commit.
- **Coupe de `pico8_input.c`** : couverte par les tests existants (`test_pico8_input`, `test_pico8_replay_source`, `test_pico8_recording_continuity`, `test_pico8_marker_coherence`, `test_pico8_minimize`), par `make e2e` et par `make sanitize-net`. Aucun nouveau test.
- **Exemples à suivre dans le dépôt** :
  - `tests/test_float_guard.c` : un script de garde testé sur des sources fabriquées, avec son code de sortie relu ;
  - `tests/test_pico8_replay_usage.c` : un processus enfant, avec son statut et son `stderr` vérifiés ;
  - `test_pico8_planet_gh_row_lq` : un test lié sous ASan, lancé avec `detect_leaks=0`.

## Out of Scope

- Toute conversion d'un site du jeu : aucune allocation du jeu ne passe dans une zone en 0a, et aucune zone n'est instanciée ni utilisée par le jeu. C'est le travail des étapes 1 à 4.
- Les tableaux croissants partagés (`ents`, `ptrs`, panneaux `lq`), et les fonctions de textes et de boîtes, qui prennent leur zone de l'appelant. C'est l'étape 0b.
- La zone transitoire adossée à la libc, qui est aussi l'étape 0b.
- L'implémentation PSX de `pico8_fatal`.
- Le choix définitif des limites de chaque zone : les valeurs de l'en-tête central sont provisoires.
- Le budget de slipways et de relais affiché dans le jeu (§7 du plan).
- Les économies de taille pour la PSX (`pico8_quarter_wave`, chaînes d'`assert`, assets en binaire), §7 du plan.
- Les autres appels libc du cœur (`fopen`, `snprintf`, `__divdi3`…), et toute fonction d'allocation autre que les quatre que vise le verrou, §7 du plan.
- Le retrait de `TROU-7` et de l'entrée `malloc` du `README`, qui est l'étape 5.
- Lancer l'application raylib.

## Further Notes

- Mesure faite pour cette PRD : sur des objets compilés `-O0` avec les flags par défaut, `objdump -r` donne un compte de relocations par fichier. Par exemple 15 pour `pico8_menu.c`, 11 pour `pico8_planet_display.c` et 17 pour `pico8_input.c` avant la coupe. Le grep de source surcompte (`pico8_galaxy.c` : 5 lignes pour 4 appels). La liste initiale est remesurée au morceau 2 ; elle n'est pas recopiée d'ici.
- Les réponses aux points que `to-prd` fait valider par un humain (modules attendus, modules à tester, découpage) viennent d'un agent `guideline` consulté à la place de l'humain. Elles sont intégrées ci-dessus.
- À la fin de l'étape, `plan-pilot` ajoute sous l'entrée 0a du plan sa note « Découvert ». Elle porte au minimum les comptes du cliquet laissés par la liste initiale et le sort des deux fichiers d'hôte.
