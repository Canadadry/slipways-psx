---
title: "Verrou : full_game.json joué jusqu'au bout dans une cible dédiée"
description: "Rien ne casse aujourd'hui quand la partie pleine sature une zone : il faut une cible à part, hors de make test-all, qui échoue dès qu'un p8_fatal interrompt worst-case/full_game.json."
status: needs-triage
---

## Problem Statement

Une fois l'étape 1 du plan 7 posée, `worst-case/full_game.json` (une partie
pleine, 50 planètes, 192 slipways, 7685 frames) va jusqu'au bout avec les
tailles de zone actuelles. Mais rien ne le garde : `make e2e` ne rejoue que
`tests/e2e/`, des parties courtes (180 frames, 600 par exception) qui
n'approchent pas le pire cas mémoire, et `make state` n'est qu'un outil qu'on
lance à la main. L'étape 4 va ramener chaque zone à son pic plus une marge et
le budget de 1,5 Mo à 768 Ko : la moindre zone trop serrée, la moindre
nouvelle fuite dans un brouillon de frame, ne se verraient qu'en relançant
`make state` et en lisant `tmp/state.txt`. Le plan exige au contraire que,
dès l'étape 2, une cible verrou passe à chaque commit, au même titre que
`make test-all` et `make sanitize-net`.

La contrainte est double : la règle du projet interdit d'allonger `make test`
ou `make e2e` (« une vérification lente va dans une cible à part, hors de
`make test-all` »), et la partie pleine est, de loin, le rejeu le plus long du
dépôt.

## Solution

Une nouvelle cible du `Makefile`, **`make full-game`**, rejoue
`worst-case/full_game.json` depuis l'écran titre avec le binaire de rejeu
sans fenêtre déjà construit par `make test`, et :

- passe quand le rejeu sort proprement après sa dernière frame, en affichant
  une ligne courte (`full-game: worst-case/full_game.json clean`) ;
- échoue sinon (un `p8_fatal` de zone pleine, un `assert`, tout code de
  sortie non nul), en affichant ce que le rejeu a écrit (le message
  `p8_fatal: zone … full, limit … bytes`) et une ligne qui nomme le fichier
  (`full-game: worst-case/full_game.json stopped`).

La cible n'entre ni dans `make test`, ni dans `make e2e`, ni dans
`make test-all`. Elle est documentée dans la légende en tête du `Makefile`,
à côté de `sanitize-net`, et la CI la lance après `make sanitize-net`, pour
que l'invariant du plan (« `make test-all`, `make sanitize-net` et la cible
verrou passent à chaque commit ») soit vérifié là où les deux autres le sont
déjà.

Le jeu ne change pas : aucun fichier de `src/` ni de `tests/e2e/` n'est
touché.

## User Stories

1. En tant que mainteneur du portage, je veux une commande unique qui rejoue la partie pleine jusqu'au bout, pour savoir en une ligne si le pire cas mémoire tient dans les zones actuelles.
2. En tant que mainteneur, je veux que cette commande échoue (code de sortie non nul) dès qu'un `p8_fatal` interrompt le rejeu, pour qu'une zone trop petite ne passe pas inaperçue.
3. En tant que mainteneur, je veux que l'échec affiche le message du `p8_fatal` (la zone et sa limite), pour savoir tout de suite quelle zone a saturé sans relancer `make state`.
4. En tant que mainteneur, je veux que l'échec nomme le fichier rejoué, pour reconnaître la cible verrou dans une sortie groupée par `-j`.
5. En tant que mainteneur, je veux que le succès tienne en une ligne, pour que la sortie reste lisible à côté de `make sanitize-net`.
6. En tant que mainteneur, je veux que `make test` reste à environ 5 s et `make e2e` à environ 1 s, pour garder la boucle rapide imposée par le `CLAUDE.md`.
7. En tant que mainteneur, je veux que `make test-all` ne dépende pas de la cible verrou, pour que la suite complète ne ralentisse pas.
8. En tant que mainteneur de l'étape 4 (redimensionner), je veux lancer la cible verrou après chaque réduction de zone, pour voir aussitôt si la marge choisie suffit au pire cas.
9. En tant que mainteneur de l'étape 4, je veux que la cible échoue si `ZONE_BUDGET_BYTES` à 768 Ko laisse une zone sous son pic réel, pour que le passage à 768 Ko soit prouvé par une partie pleine et non par une lecture de chiffres.
10. En tant que développeur du jeu, je veux qu'une fuite réintroduite dans un brouillon de la zone frame (comme celle de `planet_refresh` corrigée à l'étape 1) fasse échouer la cible verrou, pour qu'elle ne revienne pas silencieusement.
11. En tant que développeur du jeu, je veux que la cible réutilise le binaire de rejeu de `make test` (même compilation, `-O0`), pour ne pas construire un second arbre d'objets et pour pouvoir la déboguer sous gdb avec le même binaire.
12. En tant que développeur, je veux que la cible ne lise ni n'écrive le dump par défaut `tmp/replay_dump.txt`, pour qu'elle puisse tourner en parallèle de `make test`, dont deux tests relisent ce fichier.
13. En tant que développeur, je veux que la cible n'écrive que sous `tmp/`, pour ne jamais salir l'arbre versionné.
14. En tant que développeur, je veux que la cible ne capture pas les frames, pour ne pas écrire des dizaines de Mo à chaque passage.
15. En tant que développeur, je veux que la cible soit déclarée `.PHONY`, pour qu'elle rejoue à chaque appel même si rien n'a changé.
16. En tant que développeur, je veux que la cible reconstruise le binaire de rejeu si une source a changé, pour ne jamais verrouiller un binaire périmé.
17. En tant que relecteur d'une PR, je veux que la CI lance la cible verrou, pour qu'une régression mémoire de la partie pleine bloque la fusion comme une régression e2e.
18. En tant que relecteur, je veux que l'étape CI de la cible verrou soit distincte de celles de `make test-all` et de `make sanitize-net`, pour lire d'un coup d'œil lequel des trois invariants a cassé.
19. En tant que lecteur du `Makefile`, je veux trouver la cible dans la légende en tête du fichier, avec ce qu'elle rejoue et le fait qu'elle est hors de la suite, pour ne pas avoir à lire sa recette.
20. En tant que lecteur du `README`, je veux que la phrase qui présente `worst-case/full_game.json` nomme aussi la cible verrou, pour savoir que ce fichier est gardé et pas seulement mesuré.
21. En tant qu'agent sans fenêtre, je veux une vérification entièrement non interactive (pas de raylib, pas de serveur X), pour respecter la règle « ne jamais lancer l'appli raylib ».
22. En tant que pilote du plan, je veux que la cible verrou passe au commit qui l'introduit, pour que l'invariant « dès l'étape 2, la cible verrou aussi » soit vrai dès ce commit.
23. En tant que pilote du plan, je veux une preuve que la cible sait échouer (rouge sur le code d'avant l'étape 1), pour ne pas faire confiance à une cible qui passerait toujours.
24. En tant que mainteneur de l'étape 3 (mesurer), je veux que la cible verrou ne produise pas de mesure, pour que la mesure des pics reste l'affaire de `make state` et de `tmp/state.txt`.

## Implementation Decisions

- **Une seule cible, nommée `full-game`.** Elle ne rejoue que
  `worst-case/full_game.json` : pas de boucle sur tout `worst-case/`, qui ne
  contient qu'un fichier (pas de généralité spéculative). Nom anglais, comme
  les autres cibles (`sanitize-net`, `e2e-ref`).
- **Binaire : le rejeu `-O0` de `make test`**, prérequis de la cible comme il
  l'est de `replay` et `state`. Pas d'ASan : la régression visée est une zone
  pleine, que `p8_fatal` signale sans instrumentation, et `make sanitize-net`
  couvre déjà ASan+UBSan sur les e2e. Un rejeu sanitizé de la partie pleine
  serait bien plus lent pour un gain que l'étape ne demande pas.
- **Critère de passage : le code de sortie du rejeu, et lui seul.** Le pilote
  de rejeu ne rend 0 qu'après avoir joué toutes les frames du fichier et
  écrit son dump ; un `p8_fatal` sort en 3, un `assert` avorte, une écriture
  ratée rend 1. Lire « N frame(s) run » dans la sortie serait redondant.
  Pas de hash de capture : comparer l'image de la partie pleine n'est pas
  demandé (le jeu est verrouillé par les e2e), et le hash bougerait avec
  chaque écart de rendu, ce qui ferait de la cible un second `make e2e`.
- **Sorties de la cible sous un répertoire à elle dans `tmp/`** : le dump
  texte final (troisième argument du rejeu, pour ne pas toucher
  `tmp/replay_dump.txt`) et le journal du rejeu (stdout et stderr). Pas de
  capture : le deuxième argument du rejeu est `/dev/null`, comme dans
  `sanitize-net`.
- **Forme de la recette : celle de `sanitize-run-*`** — rejeu redirigé vers
  le journal ; en cas d'échec, `cat` du journal, ligne
  `full-game: <fichier> stopped`, sortie 1 ; en cas de succès, ligne
  `full-game: <fichier> clean`. Ni `STATE` ni `REPLAY_FILL` ne sont
  positionnés : la mesure est l'étape 3.
- **Hors de la suite** : `full-game` n'est prérequis ni de `test`, ni de
  `e2e`, ni de `test-all`, ni de `sanitize-net`. Elle est ajoutée à
  `.PHONY`.
- **Légende du `Makefile`** : une entrée à la suite de `sanitize-net`, dans
  le même anglais que les autres : ce qu'elle rejoue, avec quel binaire,
  qu'elle passe quand le rejeu sort proprement, « outside the suite ». C'est
  la seule aide à la lecture admise dans un `Makefile` par la règle des
  commentaires.
- **CI** : une étape `make full-game` dans le job `test`, après
  `make sanitize-net`. C'est dans le périmètre : l'invariant du plan place la
  cible verrou au même rang que les deux commandes que la CI lance déjà, et
  l'objet de l'étape est qu'une régression « se voie ». Pas de nouveau job :
  le binaire de rejeu y est déjà construit par `make test-all`.
- **`README`** : la phrase qui présente `worst-case/full_game.json` gagne la
  mention de `make full-game` (le verrou), à côté de `make state` (la
  mesure).
- **Aucun fichier de `src/` ni de `tests/` n'est modifié** ; aucune taille de
  zone ne bouge.

## Testing Decisions

- **Un bon test** observe la cible de l'extérieur : son code de sortie et la
  ligne qu'elle affiche, jamais la recette elle-même.
- **Pas de test unitaire de la recette.** Sa logique est un code de sortie
  relu, comme celle de `sanitize-run-*`, qui n'a pas de test non plus ; un
  script extrait seulement pour être testé dans `make test` ajouterait une
  abstraction à usage unique.
- **Preuve du vert** : au commit qui introduit la cible, `make full-game`
  sort en 0 et affiche `full-game: worst-case/full_game.json clean`.
- **Preuve du rouge** (manuelle, consignée dans le message de commit) :
  dans un worktree jetable au commit parent de l'étape 1, la même recette
  échoue, affiche `p8_fatal: zone frame full, limit 262144 bytes` et
  `full-game: worst-case/full_game.json stopped`. Aucune taille n'étant
  surchargeable à la compilation, c'est le moyen le plus simple de montrer la
  cible rouge sans toucher au code.
- **Preuve que la boucle ne ralentit pas** : `make test-all` ne lance pas la
  partie pleine (sa sortie ne contient aucune ligne `full-game:`), et sa
  durée reste celle d'avant le commit.
- **Invariants du commit** : `make test-all`, `make sanitize-net` et
  `make full-game` passent ; tous les `tests/e2e/*.sha256` sont identiques
  (aucun n'est touché).
- **Antériorité** : `sanitize-net` (rejeu par fichier, journal recraché en cas
  d'échec, ligne de bilan), `replay` et `state` (prérequis sur le binaire de
  rejeu, `EVENTS` hors de la suite).

## Hypothèses sur l'étape 1

L'étape 1 n'est pas encore posée ; son PRD est
`docs/prd/triage/phase1-etape1-saturation-frame.md`. Ce PRD suppose :

- **H-1-1** — Avec l'étape 1, `worst-case/full_game.json` est joué jusqu'à sa
  dernière frame (7685 entrées) **avec les tailles actuelles**, sans autre
  saturation plus loin : le binaire de rejeu sort en 0. (Le PRD de l'étape 1
  envisage lui-même qu'un autre brouillon sature plus loin.)
- **H-1-2** — L'étape 1 ne change pas le contrat de sortie du pilote de
  rejeu : `p8_fatal` écrit `p8_fatal: <message>` sur stderr et sort en 3 ;
  un rejeu complet sort en 0 ; les arguments (événements, capture, dump)
  gardent leur sens.
- **H-1-3** — La partie pleine rejouée par le binaire `-O0` dure de l'ordre
  de quelques secondes à quelques dizaines de secondes (estimation non
  mesurée, sous 30 s visés) : assez lente pour rester hors de
  `make test-all`, assez rapide pour une étape CI et un lancement après
  chaque commit.
- **H-1-4** — L'étape 1 ne touche que le moteur (zone frame, bump zone),
  `planet_display` et leurs tests : ni le `Makefile` (cibles `replay`,
  `state`, `sanitize-net`, liste des sources du rejeu), ni
  `.github/workflows/ci.yml`, ni le `README`, ni
  `worst-case/full_game.json`. Les modifications de ce PRD ne recouvrent donc
  pas les siennes.
- **H-1-5** — L'étape 1 n'ajoute elle-même aucune cible qui rejoue la partie
  pleine (son PRD la déclare hors périmètre).
- **H-1-6** — Le commit parent de l'étape 1 sature toujours à la frame 4337
  sur `zone frame full, limit 262144 bytes` : c'est sur lui que se fait la
  preuve du rouge. Suppose que l'étape 1 arrive en un commit (ou une suite de
  commits) au-dessus de `8a9e0ce`, sans réécrire celui-ci.

## À revérifier quand le commit de l'étape 1 arrive

- **Toute la Solution et la preuve du vert** (H-1-1) : lancer
  `make state EVENTS=worst-case/full_game.json` et lire la tête de
  `tmp/state.txt` (pas de ligne `stopped p8_fatal`). Si la partie s'arrête
  encore, l'étape 2 attend : la cible verrou ne peut pas passer, et la règle
  du plan renvoie à l'étape 1 (nouveau relevé, pas de taille relevée).
- **Le critère de passage par code de sortie** (H-1-2) : relire
  `fatal_pc.c` et la fin du pilote de rejeu dans le commit de l'étape 1.
- **La place de la cible hors de la suite et en CI** (H-1-3) : chronométrer
  le rejeu. S'il dépasse la minute, garder la cible hors de la suite mais
  revoir son coût en CI ; s'il tient dans la seconde, la règle reste la même
  (`make e2e` ne s'allonge en aucun cas).
- **Les fichiers touchés** (H-1-4, H-1-5) : `git show --stat` du commit de
  l'étape 1 ; s'il modifie le `Makefile`, la CI ou le `README`, repartir de
  sa version.
- **La preuve du rouge** (H-1-6) : identifier le parent réel du commit de
  l'étape 1 et vérifier qu'il sature à la frame 4337.

## Out of Scope

- Mesurer les pics des zones et des caps, `tmp/state.txt` sur la partie
  pleine et sur chaque e2e (étape 3).
- Redimensionner les zones, passer `ZONE_BUDGET_BYTES` à 768 Ko (étape 4).
- Un rejeu de la partie pleine sous ASan/UBSan, ou son entrée dans
  `make sanitize-net`.
- Un hash de capture de la partie pleine, ou toute comparaison d'image.
- Rejouer d'autres fichiers de `worst-case/`, ou une boucle générique sur ce
  répertoire.
- Ajouter la cible verrou à `make test`, `make e2e` ou `make test-all`.
- Modifier le `CLAUDE.md` (section Build) ou les skills de pilotage
  (`prd-autopilot`, `plan-pilot`) pour qu'ils lancent la cible : c'est la
  décision d'un humain.
- Toute modification du jeu ou du moteur.

## Further Notes

- Aucun humain n'était disponible pour les points « à vérifier avec
  l'utilisateur » du skill `to-prd` (choix des modules, des tests). Ils ont
  été tranchés en appliquant le skill `guideline` (simplicité d'abord,
  changements chirurgicaux, critère de succès vérifiable) : une seule cible,
  pas de script extrait, pas de test unitaire de recette, binaire existant,
  preuve du rouge manuelle.
- L'invariant « à chaque commit » du plan n'est vérifié automatiquement que
  par la CI ; en local, c'est à qui commite de lancer `make full-game` à côté
  de `make test-all` et `make sanitize-net`. Le `CLAUDE.md` pourrait le dire
  dans sa section Build : à proposer à un humain.
- Une seule tranche d'implémentation suffit : la cible, sa légende, la ligne
  du `README` et l'étape CI forment un seul commit.
