---
title: "Plan 2, Phase B1 : empreintes par frame de chaque fichier d'événements de jeu"
description: "Avant de toucher au premier site flottant, il faut une référence versionnée de ce que le port dessine, frame par frame, sur chaque session de jeu disponible, qui désigne la frame divergente et dont on a vérifié qu'elle est reproductible."
status: needs-triage
---

## Problem Statement

`docs/plan-2.md` va transformer 135 sites flottants du cœur porté. La Phase C1 affirme « ces transformations ne changent rien à l'écran », et C2 puis D affirment « seules ces frames-là changent, et chacune pour une raison sourcée ». Aucune de ces affirmations n'est aujourd'hui falsifiable.

Le dépôt dispose déjà d'un filet partiel : `make screencast` refabrique les pixels de chaque frame du rejeu headless de l'`events.json` racine, et `make screencast-check` compare un sha256 **unique** de toute la capture à `events.json.sha256`. Ce filet a trois limites face au besoin de `plan-2.md` :

1. **Il n'est pas posé.** `events.json.sha256` n'existe pas : `screencast-check` ne compare rien et se contente de dire comment signer. Et le signer est, par contrat du dépôt, un acte humain (« j'ai regardé cette session et elle est juste »), qu'aucun humain n'est là pour faire, et qu'un agent n'a pas le droit de simuler.
2. **Il dit « quelque chose a changé », jamais quoi.** Un sha256 global ne désigne aucune frame. Or le critère §6.4 de `plan-2.md` demande de rattacher **chaque frame divergente** à une ligne du relevé de sonde.
3. **Il ne porte que sur une session, et c'est la moins couvrante.** La session humaine de la racine n'atteint ni wormhole, ni slipway, ni commerce (voir le constat ci-dessous). Un filet limité à elle laisserait sans protection les chemins où vivent justement une bonne part des sites flottants, dont le `^0.75`.

Enfin, le déterminisme du rejeu a été établi en cadrage par lecture du code (LCG à graine fixe, pas de `time()`), pas par l'expérience. La sortie de phase exige qu'il soit **vérifié, pas supposé**, et donc pour chaque fichier d'événements du filet.

Le filet doit être capturé **sur le code actuel, avant toute modification** : c'est la condition sans laquelle la Phase C n'est qu'une promesse (§9).

## Solution

Pour **chaque fichier d'événements de jeu existant** — l'`events.json` racine et les fixtures de jeu non malformées de `tests/fixtures/` — un **fichier d'empreintes** versionné : une empreinte de 8 octets par ligne, une ligne par frame, dans l'ordre du rejeu. Un fichier d'empreintes par fichier d'événements. Chacun répond à « quelles frames ont changé ? » par un simple `diff` dont les numéros de ligne sont les numéros de frame.

À côté, la capacité d'**émettre la frame N en entier** d'un rejeu quelconque, dans le format texte exact de `pico8_dump_screen` (128 lignes de 128 chiffres hexa), pour régénérer la frame suspecte des deux côtés et différer 128 lignes : volume maîtrisé, diagnostic intact.

Deux cibles `make` : l'une compare, pour tous les fichiers d'événements du filet, les empreintes refabriquées à celles versionnées et, en cas d'écart, nomme le fichier d'événements et les frames divergentes ; l'autre (re)pose les références. La comparaison entre dans `make test`, de sorte que toute la suite devient le critère de C1.

Les références sont posées **une fois**, par cette étape, sur le code actuel. Avant d'être versionné, chaque fichier d'empreintes est produit **deux fois** indépendamment et les deux sorties sont comparées : c'est la preuve de déterminisme, faite fichier par fichier.

### Le filet : quels fichiers d'événements

Huit fichiers d'événements, rejoués par le pilote headless existant, avec sa séquence d'amorçage (difficulté n° 2 choisie par le pilote) :

| Fichier d'événements | Frames | Ce que le rejeu headless atteint (mesuré) |
|---|---|---|
| `events.json` (racine, session humaine, commit 741d633) | 9 729 | sondes nombreuses ; une colonisation ; planètes vidées (`ll`) ; **aucune** planète `known`, **aucun** slipway ; quatre redémarrages de partie |
| `phase4_playthrough_events.json` | 224 | sonde, planètes `known`, deux colonisations, un slipway |
| `built_planet_panel_events.json` | 278 | planètes `known`, deux colonisations, jusqu'à deux slipways |
| `empty_planet_cleared_events.json` | 229 | planètes `known`, état `kz`, `qh` non vide, une colonisation |
| `slipway_drag_events.json` | 226 | sonde, planètes `known`, `ll` ; **pas** de slipway sous ce pilote (voir ci-dessous) |
| `shop_button_hover_events.json` | 25 | interface seule, aucune entité de jeu créée |
| `screencast_capture_events.json` | 8 | interface seule |
| `top_menubar_events.json` | 4 | interface seule |

**Exclus, et pourquoi :** les trois fixtures `events_malformed_*` (refusées au chargement, c'est leur objet) ; `events_valid.json` et `demo_events.json`, qui sont des fixtures du **lecteur d'entrée** et du programme de démonstration de la Phase 1, pas des sessions de jeu (3 et 4 frames, jamais rejouées contre la cartouche par leurs tests).

Mesure faite en headless : rejeu de chaque fichier avec les points d'entrée du pilote, et relevé après chaque `draw()` des effectifs de `cg("g_")`, `cg("jc")`, `cg("qh")`, `cg("np")`, de l'histogramme des états `me` de `cg("jo")` et des crédits `fm.nz`. Chacun des sept rejeux de fixtures a été lancé deux fois avec capture : captures identiques octet pour octet, et même résultat sur la racine (80 Mo, ~11 s). Ce contrôle de cadrage ne dispense pas de la vérification de sortie sur les fichiers d'empreintes eux-mêmes.

**Sur la racine :** aucun humain n'est disponible pour enregistrer une nouvelle session. Le « enregistrer la session via le mode enregistrement raylib » de l'étape est satisfait par la session humaine existante, faite avec ce mode ; le trou de couverture qu'elle laisse est précisément ce que les fixtures comblent en partie. Aucune session n'est fabriquée par l'agent pour ce filet.

**Sur `slipway_drag_events.json` :** son test l'amorce sans passer par le choix de difficulté, alors que le pilote headless le choisit d'office ; sous le pilote, le rejeu ne suit donc pas le chemin prévu par sa fixture et n'atteint pas le slipway. Le filet l'empreinte tel quel : il est déterministe et exerce d'autres chemins. Ni le pilote ni la fixture ne sont modifiés pour l'y faire arriver.

**Ce que cette étape n'affirme pas :** que ces huit fichiers couvrent le commerce, ou un site flottant donné. Savoir quels sites sont atteints, et combien de fois, est l'objet de la mesure de couverture de l'étape suivante (B2), pas de celle-ci.

## User Stories

1. En tant que développeur de la Phase C1, je veux que `make test` échoue dès qu'une seule frame d'un seul rejeu du filet change, afin que « transformation neutre » soit vérifié par la suite et non affirmé.
2. En tant que développeur de la Phase C1, je veux que l'échec nomme le fichier d'événements et les numéros des frames divergentes, afin de savoir où regarder sans refabriquer de captures complètes.
3. En tant que développeur de la Phase C2, je veux la liste exacte, par fichier d'événements, des frames qui divergent après une constante corrigée, afin de rattacher chacune à une constante précise du relevé.
4. En tant que développeur de la Phase D, je veux pouvoir re-poser les références par une commande explicite et distincte de la vérification, afin qu'une re-baseline soit un acte volontaire et jamais un effet de bord de `make test`.
5. En tant que relecteur, je veux qu'une re-baseline apparaisse dans le diff comme des lignes modifiées des fichiers d'empreintes concernés, afin de voir en revue quels rejeux et quelles frames ont changé.
6. En tant que développeur, je veux que le numéro de ligne d'un fichier d'empreintes soit le numéro de frame, afin qu'un `diff` ordinaire suffise à localiser une divergence.
7. En tant que développeur, je veux que le fichier d'empreintes d'un rejeu se trouve à côté de son fichier d'événements et porte son nom, afin qu'aucune table de correspondance ne soit à maintenir.
8. En tant que développeur, je veux émettre une frame N quelconque d'un rejeu quelconque du filet, dans le format du dump texte existant, afin de la comparer ligne à ligne entre deux versions du code.
9. En tant que développeur, je veux que la frame N émise soit identique, pixel pour pixel, à ce que `pico8_dump_screen` produirait pour cette frame, afin de n'avoir qu'un seul format de lecture d'écran dans le dépôt.
10. En tant que responsable du chantier, je veux que le filet porte sur toutes les sessions de jeu existantes et pas seulement sur la session humaine, afin que wormhole et slipway, que la session humaine n'atteint jamais, soient sous le filet.
11. En tant que responsable du chantier, je veux que la liste des fichiers d'événements du filet soit explicite, afin qu'ajouter une fixture de jeu au filet soit une décision visible et qu'une fixture de lecteur d'entrée ou malformée n'y entre pas par accident.
12. En tant qu'agent sans fenêtre, je veux que tout cet instrument fonctionne en headless, afin de ne jamais lancer l'appli raylib.
13. En tant que développeur, je veux que les empreintes soient dérivées de captures faites par le pilote de rejeu headless existant, afin qu'il n'existe qu'un seul rejeu et que le filet observe exactement ce que `make replay` exécute.
14. En tant que développeur, je veux que la capture de l'`events.json` racine soit celle que `make screencast` produit déjà, afin que `make test` ne paie pas deux fois les ~11 s de ce rejeu.
15. En tant que développeur, je veux qu'une capture ne soit refaite que si le code ou son fichier d'événements a changé, afin que `make test` reste rapide quand rien n'a bougé.
16. En tant que responsable du chantier, je veux que les références soient posées sur le code actuel avant toute modification des sites flottants, afin que la Phase C ait un point de comparaison qui ne contient aucune de ses propres transformations.
17. En tant que responsable du chantier, je veux que le déterminisme soit démontré pour chaque fichier d'événements en produisant son fichier d'empreintes deux fois et en comparant, afin qu'une divergence future ne puisse être imputée au hasard sur aucun rejeu.
18. En tant que responsable du chantier, je veux que la vérification de déterminisme porte aussi sur le nombre de lignes, afin qu'une capture tronquée ne passe pas pour stable.
19. En tant que lecteur futur, je veux savoir quels fichiers sont dans le filet, ce que chacun atteint et pourquoi certaines fixtures en sont exclues, afin de ne pas croire protégé un chemin que le filet ne traverse pas.
20. En tant que développeur, je veux que l'empreinte ne dépende que des pixels, afin qu'elle soit identique sur Linux et macOS et insensible à la machine, à l'horodatage ou à un outil externe.
21. En tant que développeur, je veux qu'une capture illisible, vide ou tronquée fasse échouer la fabrication des empreintes avec un message nommant le fichier, afin de ne jamais versionner des empreintes calculées sur un fichier partiel.
22. En tant que développeur, je veux qu'une demande de frame hors bornes échoue clairement, afin de ne pas confondre « frame absente » et « frame noire ».
23. En tant que développeur, je veux que l'absence d'un fichier d'empreintes fasse dire à la vérification comment le poser, sans fabriquer ni comparer pour ce rejeu, afin de garder le comportement déjà adopté par `screencast-check` — tout en comparant les rejeux dont la référence existe.
24. En tant que développeur, je veux que le filet de `plan-2.md` reste distinct de la signature humaine `events.json.sha256`, afin que poser une base de non-régression ne se fasse jamais passer pour un « j'ai regardé et c'est juste ».
25. En tant que développeur, je veux que l'outil qui produit les empreintes et dumpe une frame soit un outil d'hôte ayant droit à la libc, afin de ne pas entrer dans le périmètre de suppression du flottant et de `malloc`.
26. En tant que développeur, je veux que la fonction d'empreinte soit testable seule par `make unit`, afin que sa sensibilité à un seul pixel soit prouvée et non supposée.

## Implementation Decisions

### Relation avec le mécanisme screencast existant : réutiliser, étendre, ne pas dupliquer

- **Un seul rejeu, un seul format de capture.** Chaque fichier d'empreintes est calculé à partir d'une capture faite par le pilote de rejeu headless existant, qui sait déjà écrire les pixels de chaque frame dans un fichier donné en argument. Le pilote n'est pas modifié, et aucun second pilote n'est écrit : la séquence d'amorçage existe déjà en trop d'exemplaires.
- La capture de la racine **est** la capture de `make screencast` ; les captures des fixtures sont produites de la même manière, par une règle de fichier générique, dans le répertoire temporaire ignoré par git. Elles restent jetables ; seuls les fichiers d'empreintes sont versionnés.
- **La lecture des captures passe par le module screencast existant** (ouverture, validation de taille, nombre de frames, lecture de la frame N). Il reste le seul endroit qui connaît le format ; l'outil nouveau ne calcule aucun offset.
- **`events.json.sha256` et `screencast-sign` ne sont pas touchés.** Ils portent un sens différent — une signature humaine après visionnage — que cette étape ne peut pas fournir. Les fichiers d'empreintes sont une base de non-régression posée par l'agent, conforme à `plan-2.md` §5 (« base de non-régression, pas un oracle de fidélité »). Les deux coexistent sans se remplacer.
- L'écart avec le PRD screencast, qui avait écarté « toute empreinte par frame » et renvoyé à `plan-2.md` le choix des sessions de référence, est assumé : `plan-2.md` demande explicitement l'empreinte par frame et un filet multi-sessions.

### Le filet et le nommage

- La liste des fichiers d'événements du filet est **une liste explicite** dans le `Makefile`, pas un motif sur `tests/fixtures/` : un motif ramasserait les fixtures malformées et celles du lecteur d'entrée, et ferait entrer silencieusement toute fixture future. La liste initiale est celle du tableau de la Solution (la racine et sept fixtures).
- Le fichier d'empreintes d'un fichier d'événements est **à côté de lui, sous son nom suivi d'un suffixe fixe** (`<fichier d'événements>.frames.hash`), sur le modèle de `events.json.sha256`. Pour la racine, c'est le `frames.hash` de `plan-2.md`.

### La fonction d'empreinte

- **64 bits, FNV-1a**, sur les 8 192 octets empaquetés de la frame tels que la capture les stocke. 8 octets par frame comme le chiffre `plan-2.md` (§5), aucune dépendance, déterministe par construction et indépendant de la plateforme. Il ne s'agit pas de résister à un adversaire, seulement de changer quand un pixel change.
- Écrite en **16 chiffres hexadécimaux minuscules**, une ligne par frame, sans en-tête, sans numéro, ligne 1 = frame 0. Le nombre de lignes est le nombre de frames. Rien d'autre dans le fichier : un `diff` ou un `cmp` suffit.
- Module pur, sans E/S : un tampon de frame en entrée, un entier 64 bits en sortie. Il ne connaît ni fichier ni format de capture.

### L'outil d'empreintes

- Un binaire d'hôte (hors cœur, garde la libc — `plan-2.md` §4), avec deux usages :
  - capture → fichier d'empreintes : lit toute la capture et écrit une empreinte par frame sur la sortie désignée ;
  - capture + N → frame N en entier, **dans le format texte exact de `pico8_dump_screen`** (ligne `128 128`, puis 128 lignes de 128 chiffres hexa minuscules).
- Pour ce second usage, le dump texte gagne la capacité d'écrire depuis un tampon de pixels (un par octet, 0–15, ce que rend la lecture de frame du module screencast) au lieu de la seule mémoire de jeu. `pico8_dump_screen` garde sa signature et son comportement ; s'il délègue à la nouvelle fonction, c'est la preuve qu'il n'existe qu'un format.
- Échecs (capture illisible, vide, tronquée ; index hors bornes ; écriture impossible) : code de sortie non nul et message sur la sortie d'erreur nommant le chemin ou l'index, jamais un fichier partiel présenté comme complet.

### Les cibles `make`

- Une règle de fichier fabrique, pour chaque fichier d'événements du filet, ses empreintes courantes dans le répertoire temporaire à partir de sa capture ; elle ne se refait que si la capture a changé, laquelle ne se refait que si le pilote ou le fichier d'événements a changé.
- **`frames-check`** : pour **chaque** fichier d'événements du filet, compare les empreintes courantes au fichier versionné. Toutes identiques → un message de conformité par rejeu. Au moins une différence → échec après avoir examiné tous les rejeux, avec pour chacun le nom du fichier d'événements, le nombre de frames divergentes, les premiers numéros divergents, la différence de nombre de lignes le cas échéant, et la commande pour émettre une frame N. Référence absente pour un rejeu → message expliquant comment la poser, sans échec pour ce rejeu, comme `screencast-check`.
- **`frames-baseline`** : recopie les empreintes courantes de tous les rejeux du filet dans leurs fichiers versionnés. Jamais appelée par `make test` : une re-baseline est un acte volontaire (Phase D la prévoit explicitement).
- **`make test` inclut `frames-check`.** C'est ce qui fait de la suite le critère de C1. Le rejeu de la racine est partagé avec `screencast-check`.
- Le pilote de rejeu écrit toujours son dump texte final à un chemin fixe du répertoire temporaire ; plusieurs rejeux lancés en parallèle se disputeraient ce fichier, et le test de capture headless le relit. Les règles de capture du filet ne dépendent pas de ce fichier, et ne doivent pas s'exécuter en même temps que ce test.
- La légende en tête du `Makefile` et le `.PHONY` sont étendus aux deux cibles, dans le style existant.

### Les références posées par cette étape

- **Code actuel, avant toute modification** : les fichiers d'empreintes sont posés sur l'arbre tel qu'il est au début de cette étape, sans qu'aucun fichier du cœur porté ne soit touché. Les seules modifications de `src/` autorisées sont l'ajout de la fonction d'empreinte et de l'écriture de dump depuis un tampon, qui sont hors du chemin exécuté par le rejeu et ne peuvent donc pas changer les pixels. La preuve en est que la capture de chaque rejeu est identique octet pour octet avant et après ces ajouts.
- **Déterminisme vérifié, par fichier** : pour chacun des huit fichiers d'événements, deux rejeux complets et indépendants, depuis un répertoire temporaire nettoyé entre les deux, produisent deux fichiers d'empreintes comparés par `cmp` ; ils doivent être identiques et compter autant de lignes que le fichier d'événements a de frames (9 729, 224, 278, 229, 226, 25, 8, 4).
- Le commentaire de `src/main.c` qui décrit l'`events.json` racine comme « un enregistrement de travail que la session du joueur écrase » est périmé (le backend raylib écrit désormais sous `tmp/events_recorded_<horodatage>.json`) ; il n'est pas modifié par cette étape, seulement signalé.

## Implementation Chunks

1. **L'instrument, prouvé sur une fixture courte.** Fonction d'empreinte, écriture de dump depuis un tampon, outil d'hôte (empreintes et frame N), règles de capture et d'empreintes par fichier d'événements, cibles `frames-check`/`frames-baseline`, entrée dans `make test` et tests dans `make unit`. À la fin de ce chunk, l'instrument fonctionne sur toute la liste mais aucune référence n'est posée : `frames-check` dit comment en poser.
2. **Les références.** Double production et comparaison pour chacun des huit fichiers d'événements, pose des fichiers versionnés, `make test` vert avec `frames-check` comparant réellement les huit. Aucune ligne de `src/` n'est modifiée dans ce chunk.

L'ordre est contraint : le chunk 2 a besoin de l'outil du chunk 1, et les séparer garantit que les références sont posées par un outil déjà testé, jamais par un outil mis au point en même temps que la donnée qu'il produit.

## Testing Decisions

- **Un bon test porte sur ce qui sort** — des empreintes, des lignes, des pixels, des codes de sortie — jamais sur la structure interne de l'outil.
- **Fonction d'empreinte** (`make unit`) : valeur connue sur une frame entièrement à 0 et sur une frame de motif fixe (vecteurs calculés hors du code testé) ; changer **un seul pixel**, n'importe où — premier, dernier, quartet de poids fort ou faible — change l'empreinte ; deux frames identiques donnent la même empreinte.
- **Outil d'hôte** (`make unit`, sur une capture fabriquée par le vrai pilote de rejeu à partir de la fixture courte déjà utilisée par les tests de capture) : le nombre de lignes est le nombre de frames ; la ligne k est l'empreinte de la frame k lue par le module screencast (contrôle d'ordre : deux frames distinctes échangées doivent faire échouer le test) ; la frame N émise pour la dernière frame est **identique octet pour octet** au dump texte que le pilote écrit ; un index hors bornes et une capture tronquée échouent avec un code non nul.
- **Écriture de dump depuis un tampon** : même sortie que `pico8_dump_screen` sur une mémoire de jeu contenant les mêmes pixels. Art antérieur : les tests du dump texte existants.
- **Contrôle de mutation exigé**, comme pour les tests de capture : décaler l'index de frame d'une unité, ignorer un quartet dans l'empreinte, ou inverser deux lignes doit faire échouer un test.
- **Le mécanisme de vérification se teste par la situation qu'il doit empêcher** : une ligne altérée dans le fichier d'empreintes d'**une** fixture fait échouer `frames-check` en nommant ce fichier d'événements et cette frame, et seulement lui ; une référence absente ne fait pas échouer ; des références à jour passent. Art antérieur : la manière dont `screencast-check` et le PRD des prérequis d'en-têtes ont vérifié leur propre mécanisme.
- **Déterminisme** : la double production décrite plus haut, pour les huit fichiers, est la vérification de sortie de phase ; elle est exécutée et son résultat (identique, nombre de lignes par fichier) consigné dans le message de commit qui pose les références.
- Les nouveaux binaires de test suivent les instrumentations mémoire déjà utilisées par la suite. Aucun test ne lance l'appli raylib.

## Out of Scope

- **Tout ce qui relève de B2** : l'inventaire scripté des sites flottants, la mesure de couverture du filet site par site, la garde de compilation du cœur. C'est l'étape suivante.
- **Toute modification d'un site flottant, d'une constante ou d'un cast** (Phases C et D). Cette étape ne touche pas au cœur porté.
- **Enregistrer une nouvelle session**, écrire une nouvelle fixture, ou modifier une fixture ou le pilote pour qu'un rejeu atteigne un chemin (y compris faire aboutir `slipway_drag_events.json` sous le pilote).
- **Signer `events.json.sha256`** ou modifier `screencast-sign`/`screencast-check`.
- **Un comparateur visuel** de deux captures, ou toute évolution du visualiseur raylib.
- **Corriger le commentaire périmé de `src/main.c`** sur l'`events.json` racine (signalé seulement).
- **Le volet `malloc`** (`plan-2.md` §8).
- **La reprise de la sonde pour le cas `tostr` < 0.0001** (reliquat de Phase A).
- **Le son**, hors périmètre du portage.

## Further Notes

- Les fichiers d'empreintes sont une **base de non-régression, pas un oracle de fidélité** (`plan-2.md` §5). Ils figent ce que le port dessine aujourd'hui, y compris les 16 constantes décalées d'un LSB et les casts faux sur négatifs déjà identifiés : c'est voulu. L'avertissement d'ordonnancement de §5 visait une capture faite avant de *savoir* que la population (c) relève de `flr` ; ce point est tranché par le bloc B, donc les divergences de C2 et D seront attendues et justifiées, pas prises pour des régressions.
- Les « checks with the user » prévus par `to-prd` (modules attendus, modules à tester, découpe) n'ont pas pu être posés à un humain. Faute d'outil pour lancer un agent séparé, ils ont été tranchés en appliquant la skill `guideline` : pas de second pilote, pas de format nouveau, pas de fusion avec la signature humaine, tests sur l'empreinte, l'outil et le mécanisme de vérification, découpe en deux chunks.
- Méthode de mesure de ce que chaque rejeu atteint (reproductible, headless) : rejouer le fichier d'événements avec les points d'entrée de `src/main.c` et, après chaque `draw()`, relever les effectifs de `cg("g_")`, `cg("jc")`, `cg("qh")`, `cg("np")`, l'histogramme des `me` de `cg("jo")` et `fm.nz`. La même instrumentation sur `phase4_playthrough_events.json` retrouve les étapes que `tests/test_pico8_playability.c` vérifie, ce qui valide la méthode.
