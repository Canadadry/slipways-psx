---
title: "Mesurer le pic de chaque zone et de chaque cap, sur la partie pleine et sur chaque e2e"
description: "L'étape 4 doit ramener chaque zone à son pic plus une marge fixe, mais tmp/state.txt ne donne ni le pic de la zone frame, ni celui des places de l'interface, ni celui de la file de sortie, ni les limites, et ne le fait que pour un rejeu à la fois."
status: needs-triage
---

## Problem Statement

L'étape 4 du plan 7 ramène chaque zone à « son pic mesuré, plus une marge
fixe, la même pour toutes », puis passe `ZONE_BUDGET_BYTES` de 1,5 Mo à
768 Ko. Pour le faire sans deviner, il faut, pour chaque zone et chaque cap,
un pic mesuré sur le pire cas connu : la partie pleine
(`worst-case/full_game.json`, désormais jouée jusqu'à sa frame 7685) **et**
chaque e2e de `tests/e2e/`, parce que la partie pleine n'ouvre ni boîte de
message ni fond modal — leurs places ne sont exercées que par les e2e.

Aujourd'hui, `make state EVENTS=<fichier>` écrit dans `tmp/state.txt` l'état
de la **dernière frame** d'**un** rejeu, et la section mémoire y est
incomplète :

- **zone `frame`** : absente. Le moteur ne garde aucun maximum de la zone
  frame ; depuis l'étape 1, `frame_zone_rewind` rend en cours de frame la
  place d'un brouillon, si bien que lire `used` en fin de frame sous-estime
  le pic.
- **zone `ui`** et ses six sortes de places (`button`, `modal_backdrop`,
  `message_box`, `toast`, `row`, `ephemerals`) : seulement les places
  vivantes à la dernière frame. Ni pic de places vivantes, ni plus grande
  occupation d'une place (les tailles de `button` et `message_box` sont
  « sans marge » selon le `README`).
- **zone `particles`** : les pics par sorte existent déjà (places vivantes,
  octets d'une arène, éléments d'une liste), mais pas de ligne pour la zone.
- **zone `game`** et sa sous-zone `load` : courant et pic, sans la limite.
- **file de sortie** (`EXIT_QUEUE_CAP`, relevé à 512 à l'étape 1 parce que
  le redémarrage de la partie pleine y met 286 entrées) : seulement le
  compte de la dernière frame, presque toujours 0 puisque la file est vidée
  à chaque frame.
- **aucune limite** n'est écrite : pour lire « pic / réservé », il faut
  ouvrir les sources.

Enfin, comparer 27 rejeux à la main (26 e2e et la partie pleine), chacun
écrasant `tmp/state.txt`, n'est pas une mesure : c'est une source d'oubli.

## Solution

Deux changements, aucun dans le jeu :

1. **Chaque rejeu mesure ses pics.** Le moteur garde, depuis le début du
   processus, le pic de chaque zone et de chaque cap ; la section mémoire de
   l'état (`tmp/state.txt` pour `make state EVENTS=…`) écrit, pour chacun,
   une ligne `<zone|cap> <nom> <courant> <pic> <limite>`. Les lignes
   existantes gardent leur préfixe et leur ordre de colonnes : elles gagnent
   des colonnes à droite, et de nouvelles lignes s'ajoutent (zone `frame`,
   zone `ui`, zone `particles`, octets max de chaque sorte de l'interface).
   Un `p8_fatal` écrit toujours l'état, pics compris, après la ligne
   `stopped p8_fatal frame <n>` : on lit ce qui a saturé et à combien.

2. **Une campagne de mesure.** `make state` **sans** `EVENTS` (qui sort
   aujourd'hui sur l'usage du rejeu, code 2) rejoue `full_game.json` et
   chaque e2e de `tests/e2e/`, en parallèle, chacun avec son état complet
   dans `tmp/state/<nom>.txt`, puis écrit dans **`tmp/state.txt`** le
   tableau des pics : pour chaque zone et chaque cap, le plus haut pic sur
   tous les rejeux, sa limite et le rejeu qui le tient. La campagne dure ce
   que dure la partie pleine (~4 s) ; elle est hors de `make test`, de
   `make e2e` et de `make test-all`.

`make state EVENTS=<fichier>` garde son usage : un rejeu, son état dans
`tmp/state.txt`.

## User Stories

1. En tant que mainteneur de l'étape 4, je veux une seule commande qui me donne le pic de chaque zone sur tous les rejeux connus, pour dimensionner chaque zone sans relire 27 fichiers.
2. En tant que mainteneur de l'étape 4, je veux, à côté de chaque pic, la limite actuelle, pour lire d'un coup d'œil la marge réelle et ce qu'on peut rendre.
3. En tant que mainteneur de l'étape 4, je veux savoir quel rejeu tient chaque pic, pour reproduire le pire cas d'une zone avec `make state EVENTS=<ce rejeu>`.
4. En tant que mainteneur de l'étape 4, je veux le pic de la zone `frame` sur toute la partie pleine, rewinds compris, pour ne pas la ramener sous le brouillon le plus lourd d'une frame.
5. En tant que mainteneur de l'étape 4, je veux le pic de places vivantes de chaque sorte de l'interface, pour ramener chaque compte (`BUTTON_COUNT`, `TOAST_COUNT`…) à son besoin.
6. En tant que mainteneur de l'étape 4, je veux la plus grande occupation d'une place de chaque sorte de l'interface, en-tête d'arène compris, pour ramener chaque taille de place (`BUTTON_SIZE`…) à son besoin.
7. En tant que mainteneur de l'étape 4, je veux que les pics des boîtes de message et des fonds modaux viennent des e2e, puisque la partie pleine n'en ouvre pas, pour ne pas les ramener à zéro.
8. En tant que mainteneur de l'étape 4, je veux le pic de la zone `particles` et celui de chacune de ses sortes (places, octets, éléments), pour la dimensionner comme les autres.
9. En tant que mainteneur de l'étape 4, je veux le pic de la zone `game` (les deux générations comptées ensemble, comme le teste `p8_fatal`) et celui de la sous-zone `load`, chacun avec sa limite, pour dimensionner `GAME_ZONE_BYTES` et `LOAD_ZONE_BYTES`.
10. En tant que mainteneur de l'étape 4, je veux le pic de la file de sortie sur la partie pleine, redémarrage compris, pour savoir si 512 places sont un besoin ou un excès.
11. En tant que mainteneur, je veux que le pic d'un rejeu porte sur tout le processus (chargement, écran titre, partie, redémarrage), pas sur la dernière frame, pour qu'aucun pic transitoire n'échappe.
12. En tant que mainteneur, je veux que la campagne échoue (code non nul) et nomme le rejeu quand l'un d'eux s'arrête sur un `p8_fatal`, après avoir écrit `tmp/state.txt`, pour ne jamais prendre un pic tronqué pour un pic complet.
13. En tant que mainteneur, je veux que la campagne rejoue les fichiers en parallèle, pour qu'elle dure ce que dure la partie pleine.
14. En tant que mainteneur, je veux garder l'état complet de chaque rejeu dans `tmp/state/<nom>.txt`, pour descendre dans le détail d'un rejeu sans le relancer.
15. En tant que mainteneur, je veux que `make state EVENTS=<fichier>` garde son usage et sa sortie dans `tmp/state.txt`, pour ne rien réapprendre.
16. En tant que mainteneur, je veux que la ligne de chaque zone et de chaque cap ait la même forme (`<zone|cap> <nom> <courant> <pic> <limite>`, `-` quand une valeur n'existe pas), pour la lire avec un seul `awk`.
17. En tant que mainteneur, je veux que les lignes existantes de la section mémoire gardent leur préfixe et leurs premières colonnes, pour qu'une lecture existante (`grep '^zone game'`) ne casse pas.
18. En tant que mainteneur, je veux qu'un `p8_fatal` écrive les pics dans l'état, pour lire à combien la zone saturée était montée et ce que faisaient les autres.
19. En tant que développeur du jeu, je veux que la mesure ne change rien au jeu, pour que tous les `tests/e2e/*.sha256` restent identiques.
20. En tant que développeur du jeu, je veux que la mesure ne change la taille d'aucune en-tête d'arène, pour qu'aucune place ajustée « sans marge » ne déborde à cause d'un compteur.
21. En tant que développeur, je veux que `make test` reste à environ 5 s et `make e2e` à environ 1 s, pour garder la boucle rapide imposée par le `CLAUDE.md`.
22. En tant que développeur, je veux que `make full-game` garde sa durée (~4 s), pour que le verrou de l'étape 2 reste lancé à chaque commit.
23. En tant que développeur, je veux que la mesure de la zone frame ne coûte rien sur le chemin d'allocation, pour ne pas ralentir la partie pleine ni, plus tard, la PSX.
24. En tant que développeur, je veux que `REPLAY_FILL` garde exactement son format, pour que `test_live_slots` ne bouge pas.
25. En tant que développeur, je veux que l'agrégateur de la campagne soit testé dans `make test` sur des états écrits à la main, sans rejeu, pour qu'il soit vérifié sans allonger la boucle.
26. En tant qu'agent sans fenêtre, je veux une mesure entièrement non interactive, pour respecter la règle « ne jamais lancer l'appli raylib ».
27. En tant que lecteur du `Makefile`, je veux trouver les deux usages de `make state` dans la légende, pour ne pas avoir à lire la recette.
28. En tant que lecteur du `README`, je veux que la phrase sur `full_game.json` dise où trouver les pics de tous les rejeux, pour savoir que la mesure couvre aussi les e2e.
29. En tant que pilote du plan, je veux que la mesure tourne aussi après chaque réduction de l'étape 4, pour voir la marge restante à chaque pas.

## Implementation Decisions

- **Définitions.** Une *zone* est l'une des réserves dont la somme est
  bornée par `ZONE_BUDGET_BYTES` : `frame`, `ui`, `particles`, `game` (les
  générations) et sa sous-zone `load`. Un *cap* est une limite fixe dont le
  dépassement est un `p8_fatal` : le compte et la taille de place de chaque
  sorte de `ui` et de `particles`, les éléments réservés des listes de
  particules et des points de transit, la file de sortie. Le *pic* est le
  maximum depuis le début du processus ; le *courant*, la valeur à la
  dernière frame, comme aujourd'hui.

- **Forme des lignes.** `zone <nom> <courant> <pic> <limite>` en octets ;
  `cap <nom> <courant> <pic> <limite>` en places, en éléments
  (`elements_*`) ou en octets (`bytes_*`), `-` pour une valeur inconnue (le
  courant d'un `elements_*` ou d'un `bytes_*`, comme aujourd'hui). Les
  colonnes existantes restent à leur place ; la limite s'ajoute à droite,
  le pic s'insère seulement là où il manquait (lignes `cap` de l'interface
  et de la file de sortie, qui n'avaient que le courant). Lignes nouvelles :
  `zone frame`, `zone ui`, `zone particles`, `cap bytes_<sorte>` pour
  chaque sorte de l'interface.

- **Limites écrites.** `frame` : `FRAME_ZONE_BYTES` ; `game` : les octets
  de générations (`GENERATION_ZONE_BYTES`) ; `load` : `LOAD_ZONE_BYTES` ;
  `ui` et `particles` : leurs octets réservés (somme des `SLOT_ZONE_BYTES`
  de leurs sortes, celle que vérifient déjà leurs gardes de compilation) ;
  un cap de places : le compte de la sorte ; un `bytes_*` : la taille de
  place ; un `elements_*` : la capacité réservée ; la file : `EXIT_QUEUE_CAP`.
  Les limites sont lues dans les constantes, jamais recopiées.

- **Pic de la zone `frame`.** Le module de la zone frame garde son pic ;
  il l'échantillonne quand `used` va baisser — à `frame_zone_rewind` et à
  `frame_zone_reset` — et à la lecture. Entre deux de ces instants, une
  zone bump ne fait que monter : l'échantillon est exact, sans coût sur le
  chemin d'allocation. Il expose son courant et son pic.

- **Piège : aucun champ dans `bump_zone_t`.** Une arène porte une
  `bump_zone_t` dans son en-tête ; un champ de plus grossirait chaque
  en-tête, donc l'occupation de chaque place, et ferait déborder les places
  ajustées « sans marge » (`button`, `message_box`) — un changement de
  comportement qu'une mesure ne doit pas causer. Le pic de la zone frame
  vit dans le module de la zone frame. `slot_zone_t` et `flip_zone_t`
  ne sont dans aucun en-tête d'arène.

- **Comptage par sorte, commun à `ui` et `particles`.** La zone
  `particles` compte déjà, par sorte, places vivantes, pic de places et
  plus grande occupation d'une arène (lue à sa libération, et sur les
  arènes encore vivantes à la lecture). La zone `ui` a besoin exactement du
  même comptage. Il est extrait dans le socle (`engine/`, à côté des sortes
  de places et des arènes), sous une interface étroite : prendre une place,
  la rendre, lire le vivant, le pic de places, l'occupation maximale, et
  l'occupation de la zone entière (somme des places vivantes multipliées par
  leur taille réservée) avec son pic. `particle_zone` s'y branche sans
  changer ce qu'elle rend ; `ui_zone` s'y branche à son tour. Le comptage
  des éléments (`particle_zone_record_elements`) reste propre aux
  particules. Alternative écartée : recopier dans `ui_zone` la quarantaine
  de lignes de `particle_zone`, deux copies d'un même compte à maintenir.

- **Pic de la file de sortie.** `exit_queue` garde le plus haut nombre
  d'entrées atteint, que la vidange ne remet pas à zéro, et l'expose.

- **Zones `game` et `load`.** Les pics existants (`game_zone_max`,
  `game_zone_load_zone_max`) sont repris tels quels ; seule la limite
  s'ajoute.

- **L'état reste en lecture seule.** L'écriture de l'état n'alloue dans
  aucune zone du jeu et n'appelle rien qui écrive (contrat actuel du module
  d'état). Lire un pic ne modifie que le compteur de pic lui-même
  (échantillon de la zone frame, arènes vivantes des sortes), jamais un
  état du jeu.

- **`REPLAY_FILL` inchangé**, format et contenu : c'est l'entrée de
  `test_live_slots`.

- **Campagne `make state` sans `EVENTS`.** Une règle par fichier rejoué
  (le modèle de `sanitize-run-*`), lancées en parallèle par le `-j` du
  `Makefile`, chacune
  `STATE=tmp/state/<nom>.txt` avec capture `/dev/null` et un dump à elle
  sous `tmp/state/` (jamais `tmp/replay_dump.txt`, que relisent deux tests
  de `make test`). Un rejeu qui s'arrête ne bloque pas les autres. Puis un
  script de `tools/` agrège les lignes `zone` et `cap` de tous les états en
  `tmp/state.txt` : en tête, la liste des rejeux avec `clean` ou
  `stopped p8_fatal frame <n>` ; puis une ligne par zone et par cap,
  `<zone|cap> <nom> <pic> <limite> <rejeu>`, le pic étant le plus haut sur
  tous les rejeux (en cas d'égalité, le premier dans l'ordre des fichiers,
  pour une sortie stable). Code de sortie non nul, et ligne
  `state: <fichier> stopped`, si un rejeu s'est arrêté ; `tmp/state.txt`
  est écrit dans tous les cas.

- **Rejeux de la campagne** : `worst-case/full_game.json`, puis chaque
  `tests/e2e/*.json`, dans l'ordre trié déjà utilisé par `make e2e`. Pas
  de nouvelle liste à maintenir.

- **Hors de la suite** : la campagne n'est prérequis ni de `test`, ni de
  `e2e`, ni de `test-all`, ni de `full-game`, ni de la CI.

- **Légende du `Makefile`** : l'entrée de `make state` décrit ses deux
  usages (avec `EVENTS`, un rejeu ; sans, la campagne et le tableau des
  pics). **`README`** : la phrase sur `full_game.json` (réécrite par
  l'étape 2) nomme la campagne.

- **Aucune taille ne bouge**, `ZONE_BUDGET_BYTES` compris. Aucun fichier de
  `tests/e2e/` ni de `worst-case/` n'est touché.

## Implementation Chunks

1. **Pics dans le moteur et le jeu.** Pic de la zone frame ; comptage par
   sorte extrait et branché sur `particles` puis `ui` ; pic de la file de
   sortie. Chaque module avec ses tests unitaires. Aucune sortie ne change
   encore.
2. **Section mémoire de l'état.** Nouvelles lignes et colonnes de
   `zone`/`cap`, limites comprises ; `test_state` vérifie leur forme sur les
   rejeux qu'il fait déjà.
3. **Campagne.** Règles par fichier, script d'agrégation et son test sur
   des états écrits à la main, légende du `Makefile`, phrase du `README`.
   Relever le tableau obtenu et le consigner dans le message de commit :
   c'est l'entrée de l'étape 4.

## Testing Decisions

- **Un bon test** observe un module par son interface : ce qu'il rend après
  une suite d'allocations, de libérations, de rewinds ou de vidanges ; ce
  qu'une commande écrit et son code de sortie. Jamais l'emplacement d'un
  compteur.
- **Zone frame** (dans son test existant) : le pic retient le plus haut
  `used` atteint avant un rewind, puis avant un reset ; il ne baisse
  jamais ; le courant suit le rewind.
- **Comptage par sorte** (un test à son nom, nouveau module) : pic de
  places après prises et rendus ; occupation maximale lue à la libération
  et sur une arène encore vivante ; occupation de la zone et son pic.
- **`particle_zone`** : son test existant passe sans changement — preuve
  que l'extraction ne change rien.
- **`ui_zone`** (son test existant) : pic de places et occupation maximale
  d'une sorte, après prise et rendu d'arènes.
- **File de sortie** (son test existant) : le pic survit à la vidange.
- **État** (`test_state`, sans rejeu nouveau) : sur les rejeux qu'il fait
  déjà, la ligne `zone frame` existe avec un pic non nul et la limite
  `FRAME_ZONE_BYTES` ; chaque ligne `zone`/`cap` a cinq colonnes après le
  préfixe ; le pic n'est jamais sous le courant ; sur un rejeu qui ouvre
  une boîte de message, le pic de `message_box` est au moins 1. Le cas
  `p8_fatal` existant vérifie en plus que la section mémoire suit la ligne
  `stopped`.
- **Agrégateur** (un test C qui lance le script sur des états écrits à la
  main, comme `test_frames_check` et `test_e2e_limits`) : le pic retenu est
  le plus haut, avec son rejeu ; l'égalité garde le premier ; un état
  `stopped` donne un code non nul et la ligne qui le nomme, `tmp/state.txt`
  étant quand même écrit. Rapide : aucun rejeu.
- **Invariants du commit** : `make test-all`, `make sanitize-net` et
  `make full-game` passent ; tous les `tests/e2e/*.sha256` sont identiques ;
  les durées de `make test`, `make e2e` et `make full-game` restent celles
  d'avant (chronométrées avant et après, consignées dans le commit).
- **Preuve de la campagne** (manuelle, consignée dans le commit) :
  `make state` sans `EVENTS` sort en 0 et `tmp/state.txt` donne une ligne
  par zone et par cap ; le pic de `message_box` et de `modal_backdrop` y
  est tenu par un e2e, pas par `full_game.json`.
- **Antériorité** : `test_state` (rejeu avec `STATE`, relecture de lignes),
  `test_particle_zone` (pics par sorte), `test_frames_check` et
  `test_e2e_limits` (scripts de `tools/` testés sur entrées construites),
  `sanitize-run-*` (une règle par rejeu, en parallèle).

## Hypothèses sur l'étape 2

Écrites avant que l'étape 2 soit posée ; elle l'a été pendant la rédaction
(commit `0870794`, `build: make full-game, verrou mémoire sur la partie
pleine`), et chacune a été relue contre ce commit.

- **H-2-1** — La cible verrou s'appelle `full-game`, ne rejoue que
  `worst-case/full_game.json`, et n'entre ni dans `make test`, ni dans
  `make e2e`, ni dans `make test-all`. *Vérifiée au commit `0870794`.*
- **H-2-2** — La cible verrou n'écrit pas d'état : elle ne positionne ni
  `STATE` ni `REPLAY_FILL`, et n'écrit que sous son répertoire
  `tmp/full-game/` (dump et journal). Elle n'entre donc en conflit ni avec
  `tmp/state.txt` ni avec `tmp/state/`. *Vérifiée au commit `0870794`.*
- **H-2-3** — La cible verrou utilise le même binaire de rejeu `-O0` que
  `make state` : les compteurs de pic ajoutés ici y tournent aussi, et ne
  doivent pas allonger ses ~4,4 s. *Binaire vérifié au commit `0870794` ;
  la durée est à remesurer après cette étape.*
- **H-2-4** — La CI lance `make full-game` après `make sanitize-net`, et
  rien d'autre ne dépend de la mesure : cette étape n'ajoute rien à la CI.
  *Vérifiée au commit `0870794`.*
- **H-2-5** — L'étape 2 n'a touché ni le pilote de rejeu, ni le module
  d'état, ni les zones, ni leurs tests : seulement le `Makefile` (cible et
  légende), la CI et le `README`. Les modifications de ce PRD repartent de
  la version du `Makefile` et du `README` laissée par l'étape 2.
  *Vérifiée (`git show --stat 0870794`).*
- **H-2-6** — L'invariant du plan « dès l'étape 2, la cible verrou aussi »
  s'applique à chaque commit de cette étape : `make full-game` passe à
  chacun. *Conséquence du plan, à tenir.*
- **H-2-7** — La cible verrou juge sur le code de sortie du rejeu et ne lit
  pas `tmp/state.txt` : changer le format de l'état ne la casse pas.
  *Vérifiée au commit `0870794`.*

## À revérifier quand l'étape 2 est posée

L'étape 2 étant posée à `0870794`, ces points sont à relire au moment
d'implémenter, au cas où un commit ultérieur l'aurait modifiée :

- **H-2-1, H-2-2, H-2-7** : relire la recette de `full-game` dans le
  `Makefile` — son nom, ses sorties, l'absence de `STATE`. Si elle se met à
  écrire un état, la campagne ne doit pas partager son chemin.
- **H-2-3** : chronométrer `make full-game` avant et après le premier lot ;
  un écart visible veut dire qu'un compteur est sur le chemin chaud.
- **H-2-4** : relire `.github/workflows/ci.yml` ; la campagne n'y entre pas.
- **H-2-5** : `git log` sur le `Makefile`, le `README`, le pilote de rejeu,
  le module d'état et les zones depuis `0870794` ; repartir de leur version
  courante.
- **H-2-6** : lancer `make full-game` à chaque commit de l'étape.

## Out of Scope

- Redimensionner une zone, un compte ou une taille de place, ou toucher
  `ZONE_BUDGET_BYTES` (étape 4). La marge fixe, sa valeur et la somme des
  pics face au budget sont aussi l'affaire de l'étape 4.
- Les arènes de prise (`PLANET_GRAB_ARENA_BYTES`, `GRAB_ARENAS_FREE_MAX`) :
  prises dans la zone `game`, dont le pic les compte déjà ; l'étape 4 ne
  les redimensionne pas.
- Les capacités gardées par `assert` sur des tableaux statiques
  (`SLIPWAY_SERVED_PLANETS_CAP`, `MESSAGE_BOX_ITEMS_CAP`…) : ni zones, ni
  `p8_fatal`.
- La mesure sur MIPS (plan 11) : les tailles d'en-tête et de place sont
  celles de x86-64.
- Ajouter la campagne à `make test`, `make e2e`, `make test-all`,
  `make full-game` ou à la CI.
- Changer le format de `REPLAY_FILL`.
- Un pic mesuré sous ASan : la zone frame y a deux moitiés, mais le pic est
  compté en octets réels, comme le budget.
- Toute modification du jeu.

## Further Notes

- **Relevé de départ** (code de `9640846`, identique à `0870794` pour les
  zones ; mesuré avec la sortie actuelle, donc sans les pics manquants) :
  `full_game.json` joue ses 7685 frames en 3,9 s. Pic de la zone `game`
  583 248 octets pour 720 896 de générations — bien au-dessus des 220 Ko
  notés au point de départ du plan, parce que la partie va désormais
  jusqu'au redémarrage de la frame 7643, où les deux générations coexistent
  jusqu'à la vidange. `load` 21 936 pour 65 536. Particules : 788 places
  `particle` au pic, `transit_dots` 385 sur 680, listes `particles` 16 sur
  24, `burst` 3 sur 8. Sur les 26 e2e (1,1 s en série) : pic `game`
  153 416 ; boîtes de message et fonds modaux vivants à la dernière frame
  dans 9 e2e, jamais dans la partie pleine — d'où la campagne sur les e2e.
- Le pic de `game` compte les deux générations à la fois, ce que vérifie
  `p8_fatal` : c'est sur lui que l'étape 4 dimensionne, pas sur une
  génération seule. À dire dans le message de commit du lot 3, avec le
  tableau.
- Aucun humain n'était disponible pour les points « à vérifier avec
  l'utilisateur » du skill `to-prd` (choix des modules, des tests), et
  aucun outil ne permettait de lancer un agent neuf. Ils ont été tranchés
  en appliquant moi-même le skill `guideline` (simplicité d'abord,
  changements chirurgicaux, hypothèses explicites, critère de succès
  vérifiable) : `make state` étendu plutôt qu'une nouvelle cible, un seul
  module extrait (le comptage par sorte, qui a deux utilisateurs), pas de
  somme des pics face au budget, pas de mesure des arènes de prise.
- Les chiffres du relevé ont été mesurés dans un worktree jetable au commit
  `9640846`, retiré ensuite.
