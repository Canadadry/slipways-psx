---
title: "Bilan des pics contre 768 Ko : excès, origine de chaque gros pic, candidats à réduire"
description: "Le tableau de la campagne donne le pic de chaque zone et de chaque cap, mais ni l'excès à résorber contre 768 Ko une fois la marge comptée, ni la frame et la cause de chaque gros pic, ni la liste chiffrée des réductions que 4b doit faire, ni la valeur de la marge fixe."
status: needs-triage
---

## Problem Statement

L'étape 3 du plan 7 est close : `make state` sans `EVENTS` rejoue la
partie pleine et les 26 e2e et écrit dans `tmp/state.txt` le plus haut pic
de chaque zone et de chaque cap, sa limite et le rejeu qui le tient ; la
section 2 du plan en reprend le tableau. Ce tableau ne suffit pas à lancer
les réductions de 4b :

- **Pas d'excès chiffré.** Le plan dit que la somme des zones doit passer
  de 1,5 Mo à 768 Ko (786 432 o) et que chaque zone est dimensionnée « sur
  son pic mesuré, plus une marge fixe, la même pour toutes ». Personne n'a
  encore fait la somme pic plus marge, zone par zone, contre 768 Ko : on ne
  sait pas combien d'octets il faut retirer.
- **La marge n'a pas de valeur.** La règle 3 du plan la veut fixe et
  commune ; ni son unité (octets ? proportion ?) ni sa valeur ne sont
  écrites. Or les caps se comptent en places, les zones en octets.
- **Les gros pics n'ont pas d'origine.** Pour `game`, `frame` et la file de
  sortie, on connaît le rejeu qui tient le pic, pas la frame où il tombe ni
  ce qui le fait. La règle 2 du plan veut qu'une saturation soit d'abord lue
  comme une fuite ; sans origine, on ne peut ni l'écarter ni la confirmer.
- **Le pic de `game` est mal compris.** La section 2 l'attribue au
  redémarrage (frame 7643), « où l'ancienne et la nouvelle génération du
  monde coexistent », une génération seule tenant « vers 153 Ko dans les
  e2e ». Un relevé fait pendant la rédaction de ce PRD (voir *Further
  Notes*) dit autre chose pour la partie pleine : la génération unique monte
  déjà à 581 640 o avant le clic de redémarrage, et le redémarrage n'ajoute
  que 1 608 o. Si c'est confirmé, réduire la coexistence ne rapporte
  presque rien et la bonne cible est ailleurs.
- **Aucune liste de candidats.** 4b fait « une réduction par commit, le
  plus gros candidat d'abord », avec « gain réel noté à côté du gain
  attendu ». Il lui faut une liste ordonnée, chaque entrée avec son gain
  attendu, et la preuve que la liste couvre l'excès — sinon le plan veut
  qu'on s'arrête et qu'on l'écrive.

## Solution

Une étape **sans code** : on mesure avec les outils qui existent déjà, on
calcule, on écrit le résultat dans le plan, et on ne touche ni `src/`, ni
`tests/`, ni `tools/`, ni le `Makefile`.

1. **La marge.** Une proportion du pic, la même pour chaque zone et chaque
   cap : **1/8**, arrondie au-dessus (à la cellule de zone pour des octets,
   à la place entière pour un compte). Elle s'écrit dans la règle 3 du plan.
2. **Le bilan**, ajouté sous le tableau de la section 2 : pour chaque zone,
   la taille cible (pic plus marge), la taille réservée aujourd'hui, et
   l'excès total contre 786 432 o — aussi l'excès sans marge, pour qu'on
   voie ce que pèse la marge.
3. **L'origine de chaque gros pic** (`game`, `frame`, file de sortie, et
   `ui` et `particles` qui culminent au même redémarrage) : la frame où il
   est atteint, ce qui l'occupe à cette frame, et un verdict — besoin
   permanent, transitoire de séquence (redémarrage, vidage, reconstruction)
   ou fuite. Pour `game`, le pic d'une génération seule séparé du pic des
   deux qui coexistent.
4. **Les candidats**, du plus gros au plus petit, chacun avec son gain
   attendu en octets de budget (marge comprise), écrits comme liste de
   travail de l'étape 4b. Si leur somme ne couvre pas l'excès, l'étape le
   dit dans le plan et s'arrête là, comme le veut sa dernière puce.

Les commandes qui ont produit chaque chiffre (rejeux tronqués, scripts gdb)
vont dans le message de commit, pas dans `docs/`.

## User Stories

1. En tant que pilote de 4b, je veux une liste de candidats ordonnée du plus gros au plus petit, pour attaquer le plus gros d'abord comme le plan l'exige.
2. En tant que pilote de 4b, je veux le gain attendu de chaque candidat en octets de budget, pour noter le gain réel à côté et voir si l'estimation tient.
3. En tant que pilote de 4b, je veux savoir, pour chaque candidat, quelle zone il réduit et par quel mécanisme mesuré (quel allocateur, quelle séquence), pour ne pas redécouvrir la cause avant de coder.
4. En tant que pilote de 4b, je veux savoir si un candidat est une fuite, un transitoire ou un besoin, pour choisir entre corriger, étaler ou réduire.
5. En tant que pilote de 4c, je veux la taille cible de chaque zone (pic plus marge) calculée avec une règle écrite, pour redimensionner sans refaire le calcul.
6. En tant que pilote de 4c, je veux la taille cible des zones `ui` et `particles` dérivée de leurs caps (compte et taille de place de chaque sorte), parce que ces zones sont faites de places et qu'on les redimensionne par leurs caps.
7. En tant que pilote de 4c, je veux la cible de chaque cap (file de sortie comprise), pic plus marge, pour revoir chaque cap « de même contre son pic ».
8. En tant que pilote de 4d, je veux l'excès en octets contre 786 432 o, pour savoir quand la somme tient.
9. En tant que pilote du plan, je veux l'excès sans marge à côté de l'excès avec marge, pour voir ce que la marge coûte et décider en connaissance de cause si elle est en jeu.
10. En tant que pilote du plan, je veux que la valeur de la marge soit écrite dans les règles du plan, pour que 4b, 4c et 4d appliquent la même.
11. En tant que pilote du plan, je veux que la marge soit une proportion et pas un nombre d'octets, pour qu'elle s'applique aussi bien à un cap en places qu'à une zone en octets, ce que demande « la même pour toutes ».
12. En tant que pilote du plan, je veux savoir si les candidats couvrent l'excès, pour m'arrêter et l'écrire s'ils ne le couvrent pas, sans chaîne de réductions sans fin ni plafond relevé sans décision humaine.
13. En tant que pilote du plan, je veux le pic d'une génération de `game` séparé du pic des deux générations qui coexistent, pour savoir si c'est la coexistence ou la croissance d'une génération qui coûte.
14. En tant que pilote du plan, je veux la frame où chaque gros pic est atteint, pour reproduire le pire cas d'une zone en rejouant la partie pleine jusqu'à cette frame.
15. En tant que pilote du plan, je veux que la section 2 ne garde aucune phrase que le bilan contredit (la coexistence des deux générations comme cause du pic de `game`), pour que `docs/` ne décrive que ce qui est vrai.
16. En tant que relecteur, je veux que chaque chiffre du bilan se retrouve dans `tmp/state.txt` ou soit reproductible par une commande donnée dans le message de commit, pour vérifier sans croire sur parole.
17. En tant que relecteur, je veux que la somme écrite (cibles des zones moins 786 432 o) se recalcule à la main depuis le tableau, pour vérifier l'excès sans outil.
18. En tant que relecteur, je veux que le gain attendu d'un candidat se déduise d'une mesure (octets occupés par la cause à la frame du pic), jamais d'une estimation à l'œil, pour que l'écart gain réel / gain attendu de 4b ait un sens.
19. En tant que développeur, je veux que l'étape ne touche à aucun fichier de code, de test, d'outil ni au `Makefile`, pour qu'elle reste « sans code » et ne change ni le jeu ni la durée de la boucle.
20. En tant que développeur, je veux que les rejeux tronqués et les sessions gdb de l'enquête restent hors du dépôt (scratchpad ou `tmp/`), pour ne pas laisser d'instrumentation morte.
21. En tant que développeur, je veux que tous les `tests/e2e/*.sha256` restent identiques, pour respecter la règle 1 du plan sans y penser.
22. En tant qu'agent sans fenêtre, je veux une méthode d'attribution entièrement non interactive (rejeu sans fenêtre, gdb en mode batch), pour respecter la règle de ne jamais lancer l'appli raylib.
23. En tant que mainteneur futur, je veux trouver la méthode d'attribution (troncature, scripts gdb) dans le message de commit, pour la refaire après une réduction de 4b si un pic bouge de façon inattendue.
24. En tant que pilote de 4b, je veux que la marge, la cible et l'excès soient recalculés par la campagne après chaque réduction avec la même règle, pour suivre la marge restante pas à pas.
25. En tant que pilote du plan, je veux que les caps à leur limite (octets des places `particles`, `particles_help_screen`, `ephemerals`) apparaissent dans le bilan avec leur cible, parce qu'une cible au-dessus de la limite actuelle est un relèvement qui demande d'écarter la fuite d'abord.

## Implementation Decisions

Les choix ci-dessous ont été tranchés sans humain, en appliquant le skill
`guideline` (simplicité d'abord, changements chirurgicaux, hypothèses
explicites, critère de succès vérifiable) ; aucun outil ne permettait de
lancer un agent neuf pour le faire.

- **Où vit le bilan : dans le plan, nulle part ailleurs.** Le `CLAUDE.md`
  veut que `docs/` ne décrive que ce qui est vrai ou à faire ; le bilan est
  les deux — des mesures vraies au commit de 4a et la liste de travail de
  4b à 4d. Il va donc dans le plan 7 :
  - section 2 : sous le tableau existant, la colonne ou les lignes « cible
    (pic + marge) » et l'excès avec et sans marge ; les puces corrigées
    (origine du pic de `game`, frame et verdict de chaque gros pic) ;
  - règle 3 : la valeur de la marge ;
  - étape 4b : la liste des candidats ordonnée, gain attendu de chacun, et
    la phrase « couvre / ne couvre pas l'excès ».
  Ni nouveau fichier dans `docs/`, ni annexe : quand le plan 7 sera fini,
  il sera supprimé avec son bilan, et git les garde. La méthode et les
  sorties brutes de l'enquête vont dans le message de commit. Alternative
  écartée : un document de bilan à part, qui serait « fini » dès 4b et
  devrait alors être supprimé, et qui doublerait le tableau de la section 2.

- **La marge : une proportion du pic, 1/8, arrondie au-dessus.** Le plan la
  veut « fixe, la même pour toutes » sans en donner l'unité ni la valeur.
  Une marge en octets ne s'applique pas à un cap compté en places ; une
  proportion s'applique aux deux, c'est la seule lecture où « la même pour
  toutes » tient pour les zones et pour les caps de 4c. 1/8 : une puissance
  de deux, assez pour qu'une partie un peu plus longue que `full_game.json`
  ne sature pas tout de suite, assez petite pour ne pas manger ce qu'on
  gagne. Arrondi : au multiple de cellule de zone (`zone_cell_t`) pour des
  octets, à la place entière pour un compte. La valeur s'écrit dans la
  règle 3 en une phrase. 4a ne choisit pas la marge selon ce qui
  arrangerait l'excès : la valeur est fixée avant le calcul, et le bilan
  montre l'excès sans marge à côté. Si l'excès n'est couvert qu'avec une
  marge plus petite, c'est un cas « ne couvre pas » à écrire, pas une
  marge à changer.

- **Taille cible d'une zone.**
  - `game` : pic de la ligne `zone game` (les deux générations, ce que teste
    `p8_fatal`) plus marge, plus la cible de `load` (pic de `zone load`
    plus marge) ; c'est la cible de `GAME_ZONE_BYTES`, qui contient `load`.
  - `frame` : pic plus marge.
  - `ui` et `particles` : **pas** leur ligne `zone`, mais la somme, sur
    leurs sortes, de (pic de places plus marge) × (taille de place cible) —
    la taille de place cible étant le pic `bytes_<sorte>` plus marge pour
    une sorte à arène, la taille du struct inchangée pour une sorte de
    taille fixe. Raison : ces zones sont des tableaux de places
    dimensionnés par leurs caps, et le tableau de campagne prend chaque pic
    sur un rejeu qui peut différer d'une ligne à l'autre. Les capacités
    d'éléments (`elements_*`) ont aussi leur cible.
  - Somme des cibles des quatre zones comptées par `ZONE_BUDGET_BYTES`
    (`frame`, `ui`, `particles`, `game`) moins 786 432 o : l'**excès**. La
    même somme avec une marge nulle : l'excès sans marge.

- **Ce qu'est un « gros pic ».** Au moins `game`, `frame` et la file de
  sortie (le plan les nomme) ; plus toute zone dont la cible dépasse 10 %
  de 786 432 o — `ui` et, s'il le faut, `particles`. Les autres caps ont
  leur cible dans le bilan, sans enquête d'origine.

- **Attribution d'un pic à une frame : rejeux tronqués.** Un fichier
  d'événements privé de ses dernières frames reste un rejeu valide
  (`frames` est une liste, une entrée par frame). Le pic d'une ligne ne
  fait que monter avec le nombre de frames rejouées : une bissection sur le
  nombre de frames, avec `make state EVENTS=<fichier tronqué>` (ou le
  binaire de rejeu avec `STATE`), donne la première frame où il est
  atteint en une douzaine de rejeux. Les fichiers tronqués vivent dans le
  scratchpad ou `tmp/`, jamais dans `worst-case/` ni `tests/e2e/`. On
  numérote les frames comme `stopped p8_fatal frame <n>` (indice de la
  boucle de rejeu, depuis 0).

- **Attribution d'un pic à une cause : gdb en mode batch sur le binaire de
  rejeu** (`-O0 -g`, celui de `make test`). Un point d'arrêt sur
  l'allocation de la zone visée (allocation et agrandissement de la zone à
  deux bouts pour `game`, de la zone bump `frame` pour `frame`, prise de
  place pour `ui` et `particles`, ajout à la file de sortie), avec la pile
  imprimée, sommée par premier appelant hors du socle d'allocation. Pour
  `frame`, on restreint aux allocations de la frame du pic (un point
  d'arrêt sur la boucle de frame avec un compteur d'ignorés), et on lit le
  `used` au reset qui suit ; les rewinds rendent de la place en cours de
  frame, donc la somme des allocations surestime le pic : le verdict se
  fait sur ce qui est tenu au reset, pas sur ce qui a été demandé. Les
  scripts gdb restent hors du dépôt ; leur texte va dans le message de
  commit. Aucune instrumentation ajoutée au code. Discipline : celle de
  `help-me-debug` — chaque verdict (fuite, transitoire, besoin) appuyé sur
  une mesure, chaque hypothèse écrite avant d'être testée.

- **Génération seule contre deux générations (`game`).** Le pic d'une
  génération seule de la partie pleine est le pic de `zone game` sur le
  rejeu tronqué juste avant le clic de redémarrage ; le coût de la
  coexistence est le pic complet moins celui-là. Le pic d'une génération
  dans les e2e vient de la campagne (aucun e2e ne redémarre la partie :
  à vérifier, hypothèse H-3-6). Le bilan dit aussi si, à la dernière frame
  de la partie pleine, l'ancienne génération a été rendue.

- **Verdict par gros pic.** *Besoin permanent* : la place tenue au pic sert
  encore aux frames suivantes (objets vivants du monde). *Transitoire de
  séquence* : la place n'est tenue que pendant une séquence identifiable
  (redémarrage, vidage de la file, reconstruction des échanges) et rendue
  après. *Fuite* : place tenue par des objets morts ou jamais rendue (un
  bloc d'une zone à génération qui n'est plus atteignable mais reste
  compté jusqu'au changement de génération entre dans ce cas, comme la
  règle 2 le demande). Le verdict nomme la séquence ou l'objet.

- **Candidat et gain attendu.** Un candidat est une cause mesurée (un
  appelant, une sorte d'objet, une séquence), pas une zone. Son gain
  attendu est la baisse du pic de la zone qu'il occupe si la cause
  disparaît ou se réduit à ce qu'elle tient vraiment, multipliée par
  1 + marge — un octet de pic en moins rend 1 + 1/8 octet de budget. Une
  cause qui ne tient pas la place au pic (allouée et rendue avant) ne
  rapporte rien, même si elle alloue beaucoup. La liste est ordonnée par
  gain attendu décroissant ; chaque entrée nomme la zone, la cause, le
  verdict, le gain, et ce que 4b devrait mesurer pour noter le gain réel.

- **Couverture.** Somme des gains attendus contre l'excès avec marge. Si
  elle le couvre, 4b démarre sur la liste. Sinon l'étape écrit dans le plan
  l'écart en octets, les candidats écartés et pourquoi, et s'arrête : pas
  de marge rabotée, pas de `ZONE_BUDGET_BYTES` relevé, pas de candidat
  inventé sans mesure.

- **Ce qu'on ne touche pas.** Aucune taille, aucune constante, aucune
  ligne de `src/`, `tests/`, `tools/`, du `Makefile` ni de la CI. Les
  fichiers d'événements versionnés restent intacts.

## Implementation Chunks

1. **Marge et cibles.** Lancer la campagne au commit courant, poser la
   règle de marge, calculer la cible de chaque zone et de chaque cap,
   l'excès avec et sans marge.
2. **Origine des gros pics.** Frame de chaque gros pic par rejeux
   tronqués ; génération seule contre coexistence pour `game` ; attribution
   par gdb de ce qui tient chaque pic ; verdict.
3. **Candidats et écriture.** Liste ordonnée avec gains attendus,
   couverture de l'excès, puis un seul commit du plan (section 2, règle 3,
   étape 4b) avec la méthode dans le message.

Les trois lots aboutissent à **un** commit : rien n'est écrit dans le plan
avant que la couverture soit connue, pour ne pas laisser un bilan à moitié
vrai.

## Testing Decisions

Pas de cycle TDD : aucun code n'est écrit. « Fini » se vérifie ainsi :

- **Forme du commit.** `git show --stat` ne montre que le plan 7 (et, le
  cas échéant, la sortie de ce PRD de `docs/prd/`). Aucun fichier de
  `src/`, `tests/`, `tools/`, du `Makefile`, de `worst-case/` ni de
  `tests/e2e/`.
- **Invariants du plan, inchangés par construction** mais lancés quand
  même, comme à chaque commit : `make test-all`, `make sanitize-net`,
  `make full-game` passent.
- **Traçabilité.** Chaque pic et chaque limite du bilan égale une ligne de
  `tmp/state.txt` produit par `make state` au même commit (la campagne est
  déterministe) ; chaque frame de pic et chaque attribution se refont avec
  une commande du message de commit et donnent le même nombre.
- **Arithmétique.** La somme des cibles moins 786 432 o égale l'excès
  écrit ; chaque cible égale pic × 9/8 arrondi au-dessus ; chaque gain
  attendu égale la part de pic de sa cause × 9/8. Un relecteur le refait à
  la main.
- **Couverture.** Le plan dit en une phrase si la somme des gains attendus
  couvre l'excès ; s'il dit « non », il n'y a pas d'étape 4b démarrable et
  il dit pourquoi.
- **Cohérence de la section 2.** Aucune phrase de la section 2 ne
  contredit le bilan (en particulier la cause du pic de `game`).
- **Revue indépendante.** Un agent neuf, sans le contexte de l'enquête,
  refait au moins une frame de pic et une attribution depuis le message de
  commit ; si son chiffre diffère, le bilan n'est pas fini.
- **Antériorité** : le relevé de départ du PRD de l'étape 3 (mesuré dans un
  worktree jetable, consigné dans le message de commit) ; l'enquête de
  l'étape 1 sur la saturation de `frame` (`help-me-debug`, cause prouvée
  avant correction).

## Hypothèses sur l'étape 3

Écrites pour une étape 3 en cours ; elle a été posée pendant la rédaction
(`107e87b` campagne, `c8f3d80` sortie du PRD, `33b6d6b` section 2 du plan
remplacée), et chacune a été relue contre ces commits.

- **H-3-1** — `make state` sans `EVENTS` rejoue `full_game.json` puis les
  e2e, écrit un état par rejeu sous `tmp/state/` et, dans `tmp/state.txt`,
  une ligne `replay <fichier> clean|stopped…` par rejeu puis une ligne
  `<zone|cap> <nom> <pic> <limite> <rejeu>` par zone et par cap. *Vérifiée
  à `107e87b` (outil d'agrégation et recette du `Makefile`).*
- **H-3-2** — L'état d'un rejeu garde la forme
  `<zone|cap> <nom> <courant> <pic> <limite>` de `f742d84`, et
  `make state EVENTS=…` garde son usage : la bissection sur rejeux tronqués
  lit la colonne du pic. *Vérifiée à `107e87b` (seuls `Makefile`, `README`,
  l'outil et son test changent).*
- **H-3-3** — Les 27 rejeux sortent `clean` au commit de clôture. *Dit par
  le message de `107e87b` ; à relancer au commit de 4a.*
- **H-3-4** — Les pics de la partie pleine sont ceux de `f742d84` (lot 3
  sans code de jeu ni de moteur) : `game` 583 248 / 720 896, `frame`
  247 640 / 262 144, `ui` 142 343 / 342 422, `particles` 61 855 / 130 822,
  `load` 21 936 / 65 536, file de sortie 278 / 512. *Vérifiée : identiques
  dans le message de `107e87b` et la section 2 à `33b6d6b`.*
- **H-3-5** — `game`, `frame`, `ui`, `particles`, `load`, la file de
  sortie, `row` et `particle` sont tenus par `full_game.json` ; `button`
  (12, `pause_restart`), `message_box` (3) et `toast` (42,
  `game_end_bankrupt`) par des e2e. L'attribution par frame des gros pics
  se fait donc sur la partie pleine. *Vérifiée (message de `107e87b`).*
- **H-3-6** — Aucun e2e ne redémarre une partie après en avoir joué une
  longue : le pic de `game` d'un e2e est celui d'une génération (vers
  153 Ko). *Non vérifiée : à confirmer par l'état de chaque e2e sous
  `tmp/state/` (`pause_restart` redémarre, sur une partie courte).*
- **H-3-7** — La section 2 du plan a été remplacée par le tableau de
  campagne à la clôture de l'étape 3 ; 4a ajoute son bilan dessous et
  corrige ses puces, sans refaire le tableau. *Vérifiée à `33b6d6b` ; la
  puce « l'ancienne et la nouvelle génération coexistent » y est restée,
  4a la corrige si le relevé se confirme.*
- **H-3-8** — Le tableau de campagne prend chaque ligne sur son propre
  rejeu (agrégation ligne par ligne) : le pic de `zone ui` et les pics de
  ses caps peuvent venir de rejeux différents, d'où les cibles `ui` et
  `particles` calculées depuis les caps. *Vérifiée (outil d'agrégation à
  `107e87b`).*
- **H-3-9** — La ligne `zone game` compte les seules générations (limite
  `GENERATION_ZONE_BYTES`), `load` à part ; `ZONE_BUDGET_BYTES` compte
  `GAME_ZONE_BYTES`, qui les contient toutes deux. *Vérifiée à `f742d84`
  (limite 720 896 = 768 Ko − 64 Ko).*
- **H-3-10** — Les limites `ui` et `particles` du tableau (342 422 et
  130 822) sont les octets réservés par les places, pas les constantes
  `UI_ZONE_BYTES` et `PARTICLES_ZONE_BYTES` (343 040 et 131 072) que compte
  le budget ; la somme réservée aujourd'hui se calcule sur les constantes
  (1 487 Ko). *Vérifiée à `f742d84`.*
- **H-3-11** — Le binaire de rejeu reste construit en `-O0 -g` avec ses
  symboles, et les variables de zone restent des statiques nommées, ce qui
  rend l'attribution par gdb possible sans code. *Vérifiée à `f742d84` par
  le relevé de rédaction ; à revérifier si le `Makefile` change.*
- **H-3-12** — L'étape 3 n'a ajouté à la campagne ni troncature ni
  attribution par frame : 4a les fait avec les outils existants. *Vérifiée
  (`107e87b`).*

## À revérifier quand l'étape 3 est posée

L'étape 3 est posée à `107e87b` / `33b6d6b`. Au moment d'implémenter, au
cas où un commit ultérieur l'aurait modifiée :

- **H-3-1, H-3-2, H-3-8** : relire l'outil d'agrégation et la recette de
  `make state` ; si la forme des lignes change, adapter la lecture de la
  bissection.
- **H-3-3** : lancer `make state` au commit de 4a ; un rejeu `stopped`
  arrête 4a (on ne fait pas de bilan sur un pic tronqué).
- **H-3-4, H-3-5** : comparer `tmp/state.txt` au tableau de la section 2 ;
  un écart veut dire qu'un commit a bougé un pic, et le bilan part des
  chiffres du jour.
- **H-3-6** : lire la ligne `zone game` de `tmp/state/pause_restart.txt`
  et des autres e2e.
- **H-3-7** : relire la section 2 ; si le tableau a changé de forme,
  ajouter le bilan sans le refaire.
- **H-3-9, H-3-10** : relire les constantes de tailles des zones.
- **H-3-11** : `gdb -batch` sur le binaire de rejeu, un point d'arrêt sur
  l'allocation de la zone `game`, avant de lancer l'attribution.

## Out of Scope

- Toute réduction (4b), tout redimensionnement de zone ou de cap (4c),
  `ZONE_BUDGET_BYTES` (4d).
- Toute instrumentation ajoutée au code, toute cible du `Makefile`, tout
  outil versionné : l'enquête se fait avec le rejeu, `make state`, `jq` et
  gdb.
- Le choix de la manière de réduire un candidat (recycler, rétrécir,
  libérer plus tôt) : 4a nomme la cause et le gain, 4b choisit le moyen.
- Changer la valeur de la marge pour faire tenir l'excès.
- Les remesures sur MIPS (plan 11) : tailles x86-64.
- Le budget de slipways et de relais (hors périmètre du plan).

## Further Notes

- **Relevé de rédaction** (partie pleine seule, worktree jetable à
  `f742d84`, retiré ensuite ; les zones y sont identiques à `107e87b`). Il
  ne remplace pas le travail de 4a, il dit où chercher et pourquoi la
  section 2 est à corriger :
  - Bissection sur rejeux tronqués : `game`, `ui`, `particles` et la file
    de sortie atteignent leur pic à la frame 7643 (le redémarrage) ;
    `frame` à la frame 6505.
  - `game` : 78 088 o à la frame 99, 192 896 à la frame 3999, 419 496 à la
    frame 5999, **581 640 o à la frame 7642**, juste avant le
    redémarrage ; 583 248 o au pic. La coexistence coûte 1 608 o ; à la
    dernière frame la zone tient toujours 583 248 o (rien n'est rendu). Le
    pic de `game` est donc la croissance d'une seule génération sur une
    longue partie, pas la coexistence — à confirmer par 4a.
  - gdb sur l'allocation de la zone `game` sur toute la partie : 1 966
    allocations, 584 361 o demandés, dont **442 368 o par 216 arènes de
    prise** de 2 048 o (`planet_grab_arena_open`) ; ensuite
    `planet_create` 39 600 o, `entity_register` 37 824 o, `trade_create`
    11 808 o. Une structure posée garde son arène de prise jusqu'au
    changement de génération : premier candidat probable (gain à mesurer
    sur l'occupation réelle d'une arène posée). Le PRD de l'étape 3
    laissait ces arènes hors de la mesure et disait que l'étape 4 ne les
    redimensionne pas : les réduire est une réduction (4b), pas un
    redimensionnement de zone, et 4a doit le trancher explicitement.
  - gdb sur la zone `frame` pendant la frame 6505 : `used` 247 640 o au
    reset qui la termine ; allocations dominées par
    `planet_resource_row_tuple`, `trade_create` et
    `planet_rebuild_trades` (une reconstruction des échanges après un
    `slipway_drop`). Les rewinds rendent une partie de ces allocations ;
    la part tenue reste à mesurer. Pic croissant avec la taille du monde
    (5 672 o à la frame 99, 221 832 o à la frame 5999) : à lire d'abord
    comme une fuite dans la frame (règle 2), comme à l'étape 1.
  - Ordre de grandeur, sans les cibles par caps : somme des pics des
    zones 1 057 022 o, soit environ 270 Ko d'excès sans marge et près de
    380 Ko avec 1/8. Les arènes de prise et le pic de `frame` sont du même
    ordre que cet excès.
- Aucun humain ni agent neuf n'était disponible pour les points « à
  vérifier avec l'utilisateur » du skill `to-prd` ; ils ont été tranchés en
  appliquant le skill `guideline` (voir *Implementation Decisions*).
- Le PRD ne propose aucun module ni test : l'étape est sans code par
  définition du plan.
