---
title: "Plan 2, Phase B : capture du filet de non-régression par empreintes de frame (frames.hash)"
description: "Avant de toucher au premier site flottant, il faut une référence versionnée de ce que le port dessine frame par frame, qui désigne la frame divergente et dont on a vérifié qu'elle est reproductible."
status: needs-triage
---

## Problem Statement

`docs/plan-2.md` va transformer 135 sites flottants du cœur porté. La Phase C1 affirme « ces transformations ne changent rien à l'écran », et C2 puis D affirment « seules ces frames-là changent, et chacune pour une raison sourcée ». Aucune de ces affirmations n'est aujourd'hui falsifiable.

Le dépôt dispose déjà d'un filet partiel : `make screencast` refabrique les pixels de chaque frame du rejeu headless de l'`events.json` racine, et `make screencast-check` compare un sha256 **unique** de toute la capture à `events.json.sha256`. Ce filet a deux limites face au besoin de `plan-2.md` :

1. **Il n'est pas posé.** `events.json.sha256` n'existe pas : `screencast-check` ne compare rien et se contente de dire comment signer. Et le signer est, par contrat du dépôt, un acte humain (« j'ai regardé cette session et elle est juste »), qu'aucun humain n'est là pour faire, et qu'un agent n'a pas le droit de simuler.
2. **Il dit « quelque chose a changé », jamais quoi.** Un sha256 global ne désigne aucune frame. Or le critère §6.4 de `plan-2.md` demande de rattacher **chaque frame divergente** à une ligne du relevé de sonde. Sans index par frame, cette justification demande de refabriquer deux captures de 80 Mo et de les comparer à la main, pour chaque divergence.

Enfin, le déterminisme du rejeu a été établi en cadrage par lecture du code (LCG à graine fixe, pas de `time()`), pas par l'expérience. La sortie de phase exige qu'il soit **vérifié, pas supposé**.

Le filet doit être capturé **sur le code actuel, avant toute modification** : c'est la condition sans laquelle la Phase C n'est qu'une promesse (§9).

## Solution

Un fichier `frames.hash` versionné, **une empreinte de 8 octets par ligne, une ligne par frame**, dans l'ordre du rejeu, calculé à partir de la capture d'écran que le dépôt sait déjà produire. Il répond à « quelles frames ont changé ? » par un simple `diff` de deux fichiers texte dont les numéros de ligne sont les numéros de frame.

À côté, la capacité d'**émettre la frame N en entier**, dans le format texte exact de `pico8_dump_screen` (128 lignes de 128 chiffres hexa), pour régénérer la frame suspecte des deux côtés et différer 128 lignes : volume maîtrisé, diagnostic intact.

Deux cibles `make` : l'une compare les empreintes refabriquées à celles versionnées et, en cas d'écart, nomme les frames divergentes ; l'autre (re)pose la référence. La comparaison entre dans `make test`, de sorte que toute la suite devient le critère de C1.

La référence est posée **une fois**, par cette étape, sur la session humaine déjà présente à la racine (`events.json`, 9 729 frames, enregistrée par le mode enregistrement raylib — commit 741d633). Avant d'être versionnée, elle est produite **deux fois** indépendamment et les deux fichiers sont comparés : c'est la preuve de déterminisme demandée.

### Constat sur la session disponible, et ce qu'on en fait

Aucun humain n'est disponible pendant cette exécution pour enregistrer une nouvelle session. La partie « enregistrer la session via le mode enregistrement raylib » est donc satisfaite par l'enregistrement humain existant, fait avec ce mode — **pas** par une session nouvelle ni par un scénario fabriqué par l'agent.

Sa couverture de la chaîne sonde → wormhole → colonisation → slipway → commerce a été mesurée en headless, en rejouant `events.json` et en relevant à chaque frame les effectifs de `cg("g_")`, `cg("jc")`, `cg("np")`, les états `me` des planètes de `cg("jo")` et les crédits `fm.nz` (même méthode d'observation que `tests/test_pico8_playability.c`, validée sur `phase4_playthrough_events.json`, où elle voit bien sonde, `known`, deux `ly` puis `jc=1`) :

| Maillon | Couvert par `events.json` racine | Observation |
|---|---|---|
| Sonde | **oui**, abondamment | `g_` passe à 1 plus d'une vingtaine de fois |
| Wormhole / révélation | **non observé** | aucune planète ne passe jamais à l'état `known` |
| Colonisation | **partiel** | une seule planète passe à `ly` (frame 948), plusieurs passent par `ll` (planètes vides détruites), crédits de 100 à 68 |
| Slipway | **non** | `cg("jc")` reste à 0 sur les 9 729 frames |
| Commerce | **non** | conséquence du précédent |

La session contient aussi **quatre redémarrages de partie** (`np` retombe à 0 aux frames 2219, 3739, 4571, puis 90 planètes régénérées) : c'est en réalité cinq débuts de partie courts, pas une partie longue.

**Conséquence, consignée comme limitation connue et non comblée :** le filet posé par cette étape ne garde ni les slipways, ni le commerce, ni la révélation par wormhole. Les sites flottants qui ne s'exercent que sur ces chemins (formules d'économie, tracé des slipways, dessin des routes) ne sont **pas** protégés par `frames.hash`. Cette étape ne fabrique pas de session de substitution pour masquer le trou : une session couvrant toute la chaîne reste à enregistrer par un humain, et l'étendre est une re-baseline délibérée future, hors de cette étape. En l'état, la couverture de ces chemins reste celle, grossière, de `tests/test_pico8_playability.c`.

## User Stories

1. En tant que développeur de la Phase C1, je veux que `make test` échoue dès qu'une seule frame du rejeu de référence change, afin que « transformation neutre » soit vérifié par la suite et non affirmé.
2. En tant que développeur de la Phase C1, je veux que l'échec nomme les numéros des frames divergentes, afin de savoir où regarder sans refabriquer deux captures complètes.
3. En tant que développeur de la Phase C2, je veux la liste exacte des frames qui divergent après la correction des 16 constantes, afin de rattacher chacune à une constante précise du relevé.
4. En tant que développeur de la Phase D, je veux pouvoir re-poser la référence par une commande explicite et distincte de la vérification, afin qu'une re-baseline soit un acte volontaire et jamais un effet de bord de `make test`.
5. En tant que relecteur, je veux qu'une re-baseline apparaisse dans le diff comme des lignes modifiées de `frames.hash` aux numéros des frames touchées, afin de voir en revue combien de frames ont changé et lesquelles.
6. En tant que développeur, je veux que le numéro de ligne de `frames.hash` soit le numéro de frame, afin qu'un `diff` ordinaire suffise à localiser une divergence.
7. En tant que développeur, je veux émettre une frame N quelconque en entier dans le format du dump texte existant, afin de la comparer ligne à ligne entre deux versions du code.
8. En tant que développeur, je veux que la frame N émise soit identique, pixel pour pixel, à ce que `pico8_dump_screen` produirait pour cette frame, afin de n'avoir qu'un seul format de lecture d'écran dans le dépôt.
9. En tant qu'agent sans fenêtre, je veux que tout cet instrument fonctionne en headless, afin de ne jamais lancer l'appli raylib.
10. En tant que développeur, je veux que `frames.hash` soit dérivé de la capture que `make screencast` produit déjà, afin qu'il n'existe qu'un seul rejeu headless et qu'une seule capture, et que les deux instruments ne puissent pas diverger sur ce qu'ils observent.
11. En tant que développeur, je veux que la vérification ne relance pas le rejeu si ni le code ni l'`events.json` n'ont changé, afin que `make test` ne paie pas deux fois les ~11 s du rejeu.
12. En tant que responsable du chantier, je veux que la référence soit posée sur le code actuel avant toute modification des sites flottants, afin que la Phase C ait un point de comparaison qui ne contient aucune de ses propres transformations.
13. En tant que responsable du chantier, je veux que le déterminisme du rejeu soit démontré en produisant la référence deux fois et en comparant les deux fichiers, afin qu'une divergence future ne puisse pas être imputée au hasard.
14. En tant que responsable du chantier, je veux que la vérification de déterminisme compare aussi le nombre de lignes, afin qu'une capture tronquée ne passe pas pour stable.
15. En tant que lecteur futur, je veux savoir quelle session sert de référence et ce qu'elle ne couvre pas, afin de ne pas croire protégés des chemins de jeu que le filet ne traverse jamais.
16. En tant que développeur, je veux que l'empreinte ne dépende que des pixels, afin qu'elle soit identique sur Linux et macOS et insensible à la machine, à l'horodatage ou à un outil externe.
17. En tant que développeur, je veux qu'une capture illisible, vide ou tronquée fasse échouer la fabrication de `frames.hash` avec un message, afin de ne jamais versionner des empreintes calculées sur un fichier partiel.
18. En tant que développeur, je veux qu'une demande de frame hors bornes échoue clairement, afin de ne pas confondre « frame absente » et « frame noire ».
19. En tant que développeur, je veux qu'un `frames.hash` absent fasse dire à la vérification comment en poser un, sans échouer ni fabriquer quoi que ce soit, afin de garder le comportement déjà adopté par `screencast-check`.
20. En tant que développeur, je veux que le filet de `plan-2.md` soit distinct de la signature humaine `events.json.sha256`, afin que poser une base de non-régression ne se fasse jamais passer pour un « j'ai regardé et c'est juste ».
21. En tant que développeur, je veux que l'outil qui produit les empreintes et dumpe une frame soit un outil d'hôte ayant droit à la libc, afin de ne pas entrer dans le périmètre de suppression du flottant et de `malloc`.
22. En tant que développeur, je veux que la fonction d'empreinte soit testable seule par `make unit`, afin que sa sensibilité à un seul pixel soit prouvée et non supposée.

## Implementation Decisions

### Relation avec le mécanisme screencast existant : réutiliser, étendre, ne pas dupliquer

- **Une seule capture, une seule source de pixels.** `frames.hash` est calculé à partir du fichier de capture que `make screencast` fabrique déjà (et que le pilote de rejeu headless écrit après chaque `draw()`). Le pilote de rejeu n'est pas modifié, et aucun second pilote n'est écrit : la séquence d'amorçage existe déjà en trop d'exemplaires.
- **La lecture de la capture passe par le module screencast existant** (ouverture, validation de taille, nombre de frames, lecture de la frame N). Il reste le seul endroit qui connaît le format ; l'outil nouveau ne calcule aucun offset.
- **`events.json.sha256` et `screencast-sign` ne sont pas touchés.** Ils portent un sens différent — une signature humaine après visionnage — que cette étape ne peut pas fournir. Le sha256 global reste absent ; `frames.hash` est une base de non-régression posée par l'agent, conforme à l'avertissement de `plan-2.md` §5 (« base de non-régression, pas un oracle de fidélité »). Les deux coexistent sans se remplacer ; les fusionner n'est pas l'objet de cette étape.
- L'écart avec le PRD screencast (qui avait écarté « toute empreinte par frame ») est assumé : ce PRD avait renvoyé à `plan-2.md` le choix de la référence, et `plan-2.md` demande explicitement l'empreinte par frame. Le fichier de pixels reste jetable et hors dépôt ; seul `frames.hash` est versionné.

### La fonction d'empreinte

- **64 bits, FNV-1a**, sur les 8 192 octets empaquetés de la frame tels que la capture les stocke. 8 octets par frame comme le chiffre `plan-2.md` (§5), aucune dépendance, déterministe par construction et indépendant de la plateforme. Il ne s'agit pas de résister à un adversaire, seulement de changer quand un pixel change.
- Écrite en **16 chiffres hexadécimaux minuscules**, une ligne par frame, sans en-tête, sans numéro, ligne 1 = frame 0. Le nombre de lignes est le nombre de frames. Pas d'autre contenu dans le fichier : un `diff` ou un `cmp` suffit.
- Module pur, sans E/S : un tampon de frame en entrée, un entier 64 bits en sortie. Il ne connaît ni fichier ni format de capture.

### L'outil d'empreintes

- Un binaire d'hôte (hors cœur, garde la libc — `plan-2.md` §4), avec deux usages :
  - capture → `frames.hash` : lit toute la capture et écrit une empreinte par frame sur la sortie désignée ;
  - capture + N → frame N en entier, **dans le format texte exact de `pico8_dump_screen`** (ligne `128 128`, puis 128 lignes de 128 chiffres hexa minuscules).
- Pour ce second usage, le dump texte gagne la capacité d'écrire depuis un tampon de pixels (un par octet, 0–15, ce que rend la lecture de frame du module screencast) au lieu de la seule mémoire de jeu. `pico8_dump_screen` garde sa signature et son comportement ; s'il délègue à la nouvelle fonction, c'est la preuve qu'il n'existe qu'un format.
- Échecs (capture illisible, vide, tronquée ; index hors bornes ; écriture impossible) : code de sortie non nul et message sur la sortie d'erreur nommant le chemin ou l'index, jamais un fichier partiel présenté comme complet.

### Les cibles `make`

- Une cible de fichier fabrique les empreintes courantes dans `tmp/` à partir de la capture existante ; elle ne se refait que si la capture a changé.
- **`frames-check`** : compare les empreintes courantes au `frames.hash` versionné. Identiques → message de conformité. Différentes → échec, avec le nombre de frames divergentes, les premiers numéros de frame divergents, la différence de nombre de lignes le cas échéant, et la commande pour émettre une frame N. `frames.hash` absent → message expliquant comment le poser, sans échec, comme `screencast-check`.
- **`frames-baseline`** : recopie les empreintes courantes dans le `frames.hash` versionné. Jamais appelée par `make test` : une re-baseline est un acte volontaire (Phase D la prévoit explicitement).
- **`make test` inclut `frames-check`.** C'est ce qui fait de la suite le critère de C1. Le coût additionnel est nul quand la capture est à jour, puisque `screencast` et `frames-check` partagent le même fichier de pixels.
- La légende en tête du `Makefile` et le `.PHONY` sont étendus aux deux cibles, dans le style existant.

### La référence posée par cette étape

- Scénario de référence : **l'`events.json` racine**, tel que versionné au commit 741d633, déjà désigné par la variable de capture du `Makefile`. Il n'est pas copié ni déplacé. Le commentaire de `src/main.c` qui le décrit comme « un enregistrement de travail que la session du joueur écrase » est périmé (le backend raylib écrit désormais sous `tmp/events_recorded_<horodatage>.json`) ; il n'est pas modifié par cette étape, seulement signalé.
- **Code actuel, avant toute modification** : `frames.hash` est posé sur l'arbre tel qu'il est au début de cette étape, sans qu'aucun fichier du cœur porté ne soit touché. Les seules modifications de `src/` autorisées sont l'ajout de la fonction d'empreinte et de l'écriture de dump depuis un tampon, qui sont hors du chemin exécuté par le rejeu et ne peuvent donc pas changer les pixels. La preuve en est que la capture est identique octet pour octet avant et après ces ajouts.
- **Déterminisme vérifié** : deux rejeux complets et indépendants de l'`events.json` racine, depuis un `tmp/` nettoyé entre les deux, produisent deux `frames.hash` comparés par `cmp` ; ils doivent être identiques et compter 9 729 lignes. Un contrôle préalable en cadrage a déjà donné le même sha256 pour deux captures successives (80 Mo, ~11 s chacune) ; ce contrôle ne dispense pas de refaire la vérification sur `frames.hash` lui-même.
- **Limitation de couverture consignée** (voir Solution) : sonde oui, colonisation partielle, wormhole/révélation non observé, slipway et commerce absents.

## Implementation Chunks

1. **L'instrument, prouvé sur une fixture courte.** Fonction d'empreinte, écriture de dump depuis un tampon, outil d'hôte (empreintes et frame N), cibles `frames-check`/`frames-baseline`, entrée dans `make test` et dans `make unit` pour les tests. À la fin de ce chunk, l'instrument fonctionne mais aucune référence n'est posée : `frames-check` dit comment en poser une.
2. **La référence.** Double rejeu de l'`events.json` racine, comparaison des deux `frames.hash`, pose du fichier versionné, `make test` vert avec `frames-check` réellement comparant. Aucune ligne de `src/` n'est modifiée dans ce chunk.

L'ordre est contraint : le chunk 2 a besoin de l'outil du chunk 1, et le séparer garantit que la référence est posée par un outil déjà testé, jamais par un outil mis au point en même temps que la donnée qu'il produit.

## Testing Decisions

- **Un bon test porte sur ce qui sort** — des empreintes, des lignes, des pixels, des codes de sortie — jamais sur la structure interne de l'outil.
- **Fonction d'empreinte** (`make unit`) : valeur connue sur une frame entièrement à 0 et sur une frame de motif fixe (vecteurs calculés hors du code testé) ; changer **un seul pixel**, n'importe où — premier, dernier, quartet de poids fort ou faible — change l'empreinte ; deux frames identiques donnent la même empreinte.
- **Outil d'hôte** (`make unit`, sur une capture fabriquée par le vrai pilote de rejeu à partir de la fixture courte déjà utilisée par les tests de capture) : le nombre de lignes est le nombre de frames ; la ligne k est l'empreinte de la frame k lue par le module screencast (contrôle d'ordre : deux frames distinctes échangées doivent faire échouer le test) ; la frame N émise pour la dernière frame est **identique octet pour octet** au dump texte que le pilote écrit ; un index hors bornes et une capture tronquée échouent avec un code non nul.
- **Écriture de dump depuis un tampon** : même sortie que `pico8_dump_screen` sur une mémoire de jeu contenant les mêmes pixels. Art antérieur : les tests du dump texte existants.
- **Contrôle de mutation exigé**, comme pour les tests de capture : décaler l'index de frame d'une unité, ignorer un quartet dans l'empreinte, ou inverser deux lignes doit faire échouer un test.
- **Le mécanisme de vérification se teste par la situation qu'il doit empêcher** : une ligne altérée de `frames.hash` fait échouer `frames-check` en nommant cette frame ; un `frames.hash` absent ne fait pas échouer ; une référence à jour passe. Art antérieur : la manière dont `screencast-check` et le PRD des prérequis d'en-têtes ont vérifié leur propre mécanisme.
- **Déterminisme** : la double production décrite plus haut est la vérification de sortie de phase ; elle est exécutée et son résultat (identique, 9 729 lignes) consigné dans le message de commit qui pose `frames.hash`.
- Les nouveaux binaires de test suivent les instrumentations mémoire déjà utilisées par la suite. Aucun test ne lance l'appli raylib.

## Out of Scope

- **Toute modification d'un site flottant, d'une constante ou d'un cast** (Phases C et D). Cette étape ne touche pas au cœur porté.
- **Enregistrer une nouvelle session**, ou fabriquer un `events.json` pour couvrir slipway, commerce ou wormhole. La couverture partielle est consignée, pas comblée ; l'étendre demande un humain et sera une re-baseline volontaire.
- **Signer `events.json.sha256`** ou modifier `screencast-sign`/`screencast-check`.
- **Un comparateur visuel** de deux captures, ou toute évolution du visualiseur raylib.
- **Corriger le commentaire périmé de `src/main.c`** sur l'`events.json` racine (signalé seulement).
- **Le volet `malloc`** (`plan-2.md` §8), y compris la question de savoir si `frames.hash` suffit à le protéger.
- **La reprise de la sonde pour le cas `tostr` < 0.0001** (reliquat de Phase A).
- **Le son**, hors périmètre du portage.

## Further Notes

- `frames.hash` est une **base de non-régression, pas un oracle de fidélité** (`plan-2.md` §5). Il fige ce que le port dessine aujourd'hui, y compris les 16 constantes décalées d'un LSB et les casts faux sur négatifs déjà identifiés : c'est voulu. L'avertissement d'ordonnancement de §5 visait une capture faite avant de *savoir* que la population (c) relève de `flr` ; ce point est tranché par le bloc B, donc les divergences de C2 et D seront attendues et justifiées, pas prises pour des régressions.
- Les « checks with the user » prévus par `to-prd` (modules attendus, modules à tester, découpe) n'ont pas pu être posés à un humain. Faute d'outil pour lancer un agent séparé, ils ont été tranchés en appliquant la skill `guideline` : pas de second pilote, pas de format nouveau, pas de fusion avec la signature humaine, tests sur l'empreinte, l'outil et le mécanisme de vérification, découpe en deux chunks.
- Méthode de mesure de couverture (reproductible, headless) : rejouer l'`events.json` racine avec les points d'entrée de `src/main.c`, et après chaque `draw()` relever les effectifs de `cg("g_")`, `cg("jc")`, `cg("np")`, l'histogramme des `me` de `cg("jo")` et `fm.nz`. La même instrumentation sur `phase4_playthrough_events.json` voit bien les six étapes, ce qui valide la méthode.
