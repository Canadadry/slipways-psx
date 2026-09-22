---
title: "Screencast : capture headless de l'écran frame par frame, empreinte versionnée, visualiseur raylib"
description: "Le filet de non-régression ne sait dire que « quelque chose a changé » — il ne montre jamais quoi, et personne ne peut regarder ce que le port dessine réellement sans jouer au jeu."
status: needs-triage
---

## Problem Statement

Le port dispose aujourd'hui de deux moyens d'observer ce qu'il dessine, et aucun des deux ne sert à instruire un refactor.

Le premier est `pico8_dump_screen` : il écrit **une seule** image, celle de la dernière frame du rejeu headless, en texte hexadécimal. Il répond à « à quoi ressemble la fin de ce scénario », jamais à « qu'est-ce qui s'est passé entre-temps ».

Le second est le pilote interactif raylib : il montre tout, mais il faut jouer, en direct, et rien n'en reste.

Le chantier qui vient — la suppression des dépendances flottantes et `malloc` décrite dans `docs/plan-2.md` — consiste à transformer 135 sites de code en affirmant qu'on ne change pas le comportement. C'est exactement la situation où un filet de non-régression est indispensable, et `plan-2.md` en a déjà spécifié un : un fichier d'empreintes, une par frame, versionné, dont la divergence signale une régression.

Ce filet a un défaut que son propre auteur avait accepté sans le mesurer : **une empreinte dit qu'il y a divergence, jamais laquelle**. Sur un refactor qui touche 61 constantes et 76 casts, apprendre que « le hash a changé » ne désigne aucun site. Il faut ensuite régénérer la frame des deux côtés, à la main, et comparer 128 lignes de texte — ce que `plan-2.md` prévoit explicitement, et qui revient à instruire chaque divergence par une manipulation séparée.

Il manque aussi, et c'est indépendant du refactor, un moyen pour un humain de **regarder** ce que le port dessine sur toute une session, sans lancer le jeu : valider à l'œil que le rendu est conforme à la cartouche d'origine est une question à laquelle aucun hash ne répondra jamais, et qu'aucune suite de tests ne pose.

## Solution

Trois pièces, qui tiennent ensemble par une seule idée : **le rejeu headless est déterministe, donc les pixels de toute une session sont une fonction du code et de l'`events.json`, régénérable à volonté et jamais stockée.**

1. **Une capture.** Une règle `make` fabrique un fichier de données de pixels — l'écran 128×128 de **chaque** frame du rejeu headless, en brut, sans compression. Ce fichier est local, temporaire, jetable, et il ne va ni dans git ni nulle part ailleurs.

2. **Une empreinte versionnée.** Le sha256 de ce fichier est écrit dans un fichier d'une ligne, versionné à côté de l'`events.json` dont il est issu. C'est le seul artefact durable de tout ce PRD. `make test` en dépend : si le port se met à dessiner autre chose, la suite rougit. Et `git blame` sur cette ligne répond à la question qui a motivé ce PRD : **à quel commit un humain a-t-il validé pour la dernière fois ce que le jeu affiche.**

3. **Un visualiseur.** Une fenêtre raylib qui ouvre le fichier de pixels et le joue **comme une vidéo**, avec les contrôles de vitesse déjà écrits pour le rejeu. C'est ce qui rend l'empreinte signable : personne ne peut valider un fichier binaire de plusieurs centaines de mégaoctets, mais tout le monde peut regarder une session défiler.

Le cycle d'usage est alors : `make test` rougit → on régénère les pixels au commit que `git blame` désigne, on les regarde défiler, on compare avec ceux d'aujourd'hui → soit c'est une régression et on corrige, soit c'est un changement voulu et on met à jour l'empreinte, ce qui **est** l'acte de signature.

## User Stories

1. En tant que développeur du portage, je veux capturer l'écran de chaque frame d'un rejeu headless, afin de disposer de la trace complète de ce que le port dessine sur un scénario.
2. En tant que développeur du portage, je veux que cette capture soit produite par le rejeu headless déjà existant et non par un second pilote, afin que les pixels capturés soient exactement ceux du rejeu que tout le monde exécute.
3. En tant que développeur du portage, je veux que la capture n'exige aucune fenêtre ni serveur graphique, afin qu'elle fonctionne dans les environnements d'agent, en intégration continue et sur une machine sans écran.
4. En tant que développeur du portage, je veux que le fichier de pixels reste hors du dépôt, afin que l'historique git ne grossisse pas d'un artefact régénérable.
5. En tant que développeur du portage, je veux que l'empreinte de ce fichier soit versionnée, afin de disposer d'un point de comparaison stable sans stocker les pixels eux-mêmes.
6. En tant que développeur du portage, je veux que `git blame` sur l'empreinte me dise à quel commit elle a été posée, afin de savoir où revenir pour régénérer les pixels de référence.
7. En tant que développeur du portage, je veux que `make test` échoue si les pixels produits ne correspondent plus à l'empreinte versionnée, afin qu'une régression de rendu soit détectée sans que j'aie à y penser.
8. En tant que développeur du portage, je veux que la capture soit une règle de fichier dont `make test` dépend, afin qu'elle ne soit refaite que lorsque le code ou le scénario ont changé, et non à chaque exécution de la suite.
9. En tant que développeur du portage entreprenant la suppression du flottant, je veux distinguer « j'ai transformé » de « j'ai changé », afin qu'une transformation censée être neutre qui ne l'est pas soit signalée immédiatement.
10. En tant que développeur du portage, je veux pouvoir mettre à jour délibérément l'empreinte quand une divergence est voulue, afin qu'une correction légitime ne reste pas indéfiniment rouge.
11. En tant que développeur du portage, je veux qu'une mise à jour d'empreinte apparaisse comme une ligne modifiée dans un diff, afin qu'elle soit visible en revue et ne puisse pas passer inaperçue.
12. En tant qu'humain, je veux ouvrir une fenêtre qui joue la session capturée comme une vidéo, afin de voir ce que le port dessine sans jouer au jeu et sans lire un fichier binaire.
13. En tant qu'humain, je veux pouvoir accélérer et ralentir la lecture, afin de traverser rapidement les passages sans intérêt et d'examiner les moments qui comptent.
14. En tant qu'humain, je veux savoir où j'en suis dans la lecture, afin de ne pas croire que le programme est figé et de pouvoir citer un numéro de frame.
15. En tant qu'humain, je veux que l'image affichée soit agrandie à une taille confortable, afin de distinguer les pixels individuels d'un écran de 128×128.
16. En tant qu'humain, je veux que la fenêtre se ferme normalement, afin de ne pas avoir à tuer le processus.
17. En tant qu'humain validant un rendu, je veux pouvoir regarder la session d'un bout à l'autre puis mettre à jour l'empreinte, afin que le commit correspondant signifie « j'ai regardé et j'accepte ».
18. En tant que développeur du portage, je veux que la capture puisse être régénérée à n'importe quel commit passé, afin de reconstituer les pixels de référence d'une empreinte ancienne.
19. En tant que développeur du portage, je veux que la comparaison entre deux captures se fasse sur des fichiers bruts, afin de pouvoir localiser la première frame divergente avec un outil ordinaire et sans écrire de code.
20. En tant qu'agent de développement, je veux que tout ce qui est décidable sans fenêtre soit testable par `make unit`, afin de pouvoir vérifier mon travail dans un environnement sans serveur graphique.
21. En tant que développeur du portage, je veux que le format du fichier de pixels soit trivial et documenté, afin qu'un script jetable puisse le lire sans dépendre du code du projet.
22. En tant que développeur du portage, je veux que la capture n'introduise aucune dépendance externe de compilation, afin que la construction reste autonome.
23. En tant que développeur du portage sur macOS, je veux que la règle de calcul d'empreinte fonctionne sur ma machine comme sur Linux, afin que l'empreinte versionnée soit la même des deux côtés.
24. En tant que développeur du portage, je veux savoir combien de temps coûte la capture, afin de décider en connaissance de cause de la durée du scénario de référence.
25. En tant que développeur du portage, je veux que l'ajout de la capture ne change pas le comportement du rejeu headless existant quand on ne la demande pas, afin que les cibles actuelles restent identiques.
26. En tant que développeur du portage, je veux que l'écriture des pixels n'altère en rien l'état du jeu, afin que la capture ne puisse pas modifier ce qu'elle observe.
27. En tant que développeur du portage, je veux que le visualiseur refuse clairement un fichier tronqué ou absent, afin de ne pas confondre un fichier incomplet avec une session courte.
28. En tant que développeur du portage, je veux que le visualiseur n'écrive jamais dans le fichier qu'il lit, afin qu'un aller-retour par le visualiseur ne puisse pas invalider une empreinte.
29. En tant que développeur du portage, je veux que la capture échoue bruyamment si elle ne peut pas écrire, afin de ne pas calculer une empreinte sur un fichier partiel.
30. En tant que développeur du portage, je veux que l'empreinte porte sur les pixels et rien d'autre, afin qu'elle ne dépende ni de la machine, ni de l'horodatage, ni d'un outil de compression.

## Implementation Decisions

### Ce qui est capturé, et sous quelle forme

- **Pixels bruts, aucune compression.** Chaque frame occupe 8 192 octets : les 16 384 pixels de l'écran 128×128, deux pixels par octet (un quartet chacun, exactement la représentation de la mémoire écran de PICO-8). Le fichier est la concaténation des frames dans l'ordre, sans en-tête, sans index, sans séparateur. `frame N` commence à l'offset `N × 8192`, et le nombre de frames est la taille du fichier divisée par 8 192.
- **Cette décision a été prise après mesure, et elle annule une exigence initiale.** Un algorithme de compression spécifique avait été demandé. Les mesures sur les 9 729 frames réelles de l'`events.json` du dépôt donnent : 0,80 % des pixels changent d'une frame à l'autre, 11 % des frames sont identiques à la précédente, un XOR-delta + RLE maison atteint 41× (200 o/frame) pour moins de 10 µs par frame, `xz` sur le brut atteint 142× pour ~1,9 ms par frame, et un découpage en tuiles 8×8 suivi de `xz` atteint 192× (43 o/frame). **Aucun de ces gains n'a de destinataire** : le fichier n'est ni versionné, ni transmis, ni conservé. Compresser un fichier temporaire local que l'on efface après usage coûte du code, un format, un décodeur, et n'achète rien. Le brut est retenu parce qu'il est le seul format qui ne demande aucun code du tout.
- **Conséquence directe et souhaitable : l'empreinte devient insensible à l'environnement.** Un sha256 calculé sur une sortie compressée serait l'empreinte de *(pixels × codec × version de la bibliothèque × options × nombre de cœurs)* — vérifié sur cette machine : `gzip` grave le mtime dans son en-tête et produit des octets différents pour un contenu identique, et `xz -T0` produit une sortie différente de `xz -T1`. Une empreinte fausse pour une raison qui n'a rien à voir avec le jeu détruit la seule chose que l'instrument doit garantir : la capacité de distinguer « le code a changé » de « ma machine a changé ». Le brut supprime ce risque au lieu de le gérer.
- **Taille assumée.** 8 192 octets par frame, soit ~80 Mo pour les 9 729 frames de l'`events.json` actuel, ~850 Mo pour une session de 30 minutes. C'est un fichier temporaire dans le répertoire déjà ignoré par git ; la place disque est le prix explicitement accepté pour n'écrire aucun codec.

### Où les pixels sont produits

- La capture est faite par **le pilote de rejeu headless existant**, pas par un nouveau pilote. Le dépôt a déjà dupliqué une séquence de démarrage pour le backend raylib, et la raison y était énoncée : le pilote raylib a une boucle et une source d'entrée différentes. Ici ce n'est pas le cas — c'est exactement le même rejeu, avec une écriture en plus. Une troisième copie de la séquence d'amorçage créerait un second rejeu subtilement différent, et la baseline décrirait alors un déroulement que personne d'autre n'exécute.
- La capture est **optionnelle et désactivée par défaut** : le pilote headless invoqué comme aujourd'hui se comporte comme aujourd'hui, y compris son dump texte de la dernière frame, qui n'est ni remplacé ni modifié. La demande de capture se fait par un argument supplémentaire donnant le chemin de sortie.
- L'écriture a lieu **après `draw()` et avant la frame suivante**, en lecture seule sur la mémoire du jeu via le même accès mémoire que le dump texte existant. Rien dans la capture ne doit écrire dans l'état du jeu : une capture qui modifierait ce qu'elle observe invaliderait tout l'instrument.
- Un échec d'écriture (disque plein, chemin invalide) **interrompt le processus avec un code d'erreur**, contrairement à la politique de l'enregistreur de session interactif qui préfère perdre une frame plutôt que tuer la partie d'un humain. Ici le contexte est inverse : personne ne joue, et un fichier tronqué produirait silencieusement une empreinte fausse, c'est-à-dire un faux rouge indiscernable d'une régression.

### Le module de screencast

- Un module dédié, **sans aucune dépendance à raylib**, porte tout ce qui touche au format : écrire une frame depuis la mémoire du jeu, ouvrir un fichier en lecture, en donner le nombre de frames, et lire la frame *N* dans un tampon de 128×128 quartets. C'est un module profond au sens où l'entend le dépôt : une interface étroite qui ne changera pas, devant un format qui pourrait changer.
- Il est le seul endroit où le format est connu. Ni le pilote headless, ni le visualiseur ne calculent d'offset eux-mêmes.
- Il est testable par `make unit` sans fenêtre : on écrit des frames connues, on les relit, on vérifie qu'on récupère les mêmes valeurs aux mêmes positions, dans l'ordre, y compris hors séquence.

### L'empreinte

- Un fichier d'une ligne, **versionné**, portant le sha256 du fichier de pixels, nommé d'après l'`events.json` dont il dérive.
- Le calcul se fait avec l'outil du système. **`sha256sum` n'existe pas sur macOS**, où l'équivalent est `shasum -a 256` ; le `Makefile` sait déjà tester le système d'exploitation pour d'autres raisons (paquets X11, options d'édition de liens), et doit le faire ici aussi. Une empreinte qui ne se calculerait que sur Linux rendrait la cible inutilisable sur la machine où le relevé PICO-8 de référence a été fait.
- Une divergence fait **échouer** la vérification avec un message qui nomme les deux empreintes et rappelle comment régénérer et visualiser. Le message est la seule interface humaine d'un échec, et un échec muet sur un hash ne dit rien à personne.
- **Mettre à jour l'empreinte est un acte délibéré**, fait par un humain qui a regardé, et matérialisé par une ligne modifiée dans un commit. Aucune cible ne doit mettre à jour l'empreinte automatiquement en cas de divergence : ce serait l'annulation silencieuse du filet.

### Intégration au système de construction

- Le fichier de pixels est une **règle de fichier** ordinaire, avec pour prérequis les objets du moteur et l'`events.json` du scénario. La suite de tests **dépend** de ce fichier sans le fabriquer elle-même : `make` le régénère uniquement quand un source ou le scénario ont changé, mécanisme déjà en place dans ce dépôt depuis le passage aux objets intermédiaires et aux dépendances générées par le compilateur.
- **Coût mesuré, à assumer explicitement.** Le rejeu des 9 729 frames de l'`events.json` actuel prend 9,8 s (construction non optimisée), plus ~1,5 s pour l'écriture des pixels. Pendant un chantier qui touche `src/` à chaque itération, le fichier est donc régénéré à chaque `make test`, ce qui ajoute ~11 s à la suite. Sur un scénario de 30 minutes, ce serait ~2 minutes. **La durée du scénario de référence est donc un arbitrage direct sur le temps de la boucle rouge-vert**, et mérite d'être tranchée avant de capturer une longue session.
- Le fichier de pixels va dans le répertoire temporaire déjà ignoré par git. Rien de ce PRD n'ajoute de fichier volumineux au dépôt.
- Aucune dépendance externe de compilation n'est introduite : pas de bibliothèque de compression, pas d'implémentation de sha256 dans le code du projet.

### Le visualiseur

- Un binaire raylib distinct, construit par sa propre cible, **jamais lancé par un agent** : il n'y a pas de serveur graphique dans les environnements d'agent, et un agent n'a de toute façon aucun moyen de regarder une fenêtre.
- Il ouvre le fichier de pixels passé en argument, le joue en continu, et affiche l'image agrandie à un facteur entier — le backend interactif utilise déjà un facteur 6 pour la même image de 128×128, et il n'y a aucune raison de diverger.
- **Les contrôles de vitesse sont ceux qui existent déjà** pour le rejeu du backend interactif : échelle de multiplicateurs, deux touches pour monter et descendre, une pour décapper, et une ligne d'état formatée « frame courante / total, vitesse ». Ce module est déjà testé unitairement ; le réutiliser plutôt que d'en écrire un second évite deux échelles de vitesse divergentes dans le même dépôt.
- Le visualiseur **n'ouvre jamais le fichier en écriture**. Un aller-retour par le visualiseur ne doit pas pouvoir changer un octet et donc invalider une empreinte.
- Un fichier absent, illisible, vide, ou dont la taille n'est pas un multiple de 8 192 octets est **refusé avant l'ouverture de la fenêtre**, avec un message explicite. Une fenêtre qui s'ouvre sur du noir est indiscernable d'une session qui commence par un écran noir.
- La conversion d'un index de palette PICO-8 vers une couleur est **déjà faite** par le backend interactif pour présenter la mémoire écran ; le visualiseur affiche exactement la même chose et ne doit pas redéfinir sa propre palette.

### Partage du travail entre ce qui est testable et ce qui ne l'est pas

- Tout ce qui est décidable sans fenêtre vit hors du module raylib : le format, la lecture, le nombre de frames, la validation de taille, la vitesse, la ligne d'état. Le fichier raylib se limite à ouvrir une fenêtre, téléverser une image et la dessiner.
- Ce n'est pas une préférence de style : c'est la seule façon pour l'agent qui écrit ce visualiseur de vérifier son travail avant de le livrer, puisqu'il ne pourra jamais l'exécuter. Le dépôt a déjà appliqué cette séparation trois fois — conversion de coordonnées, source de rejeu, enregistreur de session — et l'en-tête de l'une d'elles dit explicitement que l'extraction était motivée par le test et non par le design.

## Implementation Chunks

1. **Le module de screencast et son format.** Écriture d'une frame depuis la mémoire du jeu, lecture d'un fichier, nombre de frames, lecture de la frame *N*, validation de taille. Aucune dépendance raylib, testé par `make unit`. Rien d'autre ne bouge ; à la fin de ce chunk aucune cible n'a changé de comportement.
2. **La capture dans le pilote headless.** Argument optionnel de chemin de sortie, écriture après le dessin de chaque frame, erreur fatale en cas d'échec d'écriture. Le comportement sans cet argument est inchangé, ce qui est vérifiable directement.
3. **La règle de fabrication, l'empreinte et son contrôle.** Cible de fichier pour les pixels, calcul de l'empreinte compatible Linux et macOS, fichier d'empreinte versionné, dépendance depuis la suite de tests, message d'échec utile. C'est le chunk qui rend l'instrument opérant : à la fin, une modification du rendu fait rougir la suite.
4. **Le visualiseur.** Cible raylib distincte, réutilisation du contrôle de vitesse existant et de la présentation de l'image existante, refus explicite d'un fichier invalide. Jamais lancé par un agent ; vérifié par les tests unitaires de ses modules et par une relecture, la validation finale étant faite par un humain qui l'ouvre.

L'ordre est contraint : 2 a besoin du format de 1, 3 a besoin de la capture de 2, 4 a besoin d'un fichier à lire. Les chunks 1 à 3 forment un instrument complet et utile même si le chunk 4 n'est jamais fait — c'est la découpe qui permet d'arrêter après 3 sans rien perdre.

## Testing Decisions

- **Un bon test ici porte sur ce qui sort, pas sur comment c'est écrit.** Les tests assertent des valeurs de pixels à des positions données, des nombres de frames, des refus sur des fichiers invalides — jamais la présence d'un champ, d'une fonction ou d'une structure interne. Le précédent du dépôt est net : un test qui se contenterait de compter les frames passerait sur une source qui les sert à l'envers, remarque déjà consignée à propos du module de rejeu et qui s'applique mot pour mot ici.
- **Module de screencast, testé par `make unit`** : écriture puis relecture d'une suite de frames distinctes, lecture hors séquence, cohérence du nombre de frames avec la taille du fichier, refus d'une taille non multiple de la taille d'une frame, refus d'un fichier absent. Art antérieur direct : les tests du module de rejeu et ceux du dump texte, qui écrivent et relisent un fichier temporaire.
- **Fidélité de la capture** : une frame capturée doit être identique, pixel pour pixel, à ce que le dump texte existant produit pour la même frame. C'est le test qui empêche la capture de devenir une seconde vérité divergente ; il se construit en rejouant un scénario court et en comparant les deux sorties pour la dernière frame, seul point où le dump texte existant est comparable.
- **Neutralité de la capture sur le jeu** : un rejeu avec capture et un rejeu sans capture doivent produire le même dump texte final. C'est la garantie que l'instrument n'altère pas ce qu'il mesure, et elle est vérifiable sans écrire un seul test unitaire nouveau.
- **Contrôle de mutation exigé** sur les tests de lecture : supprimer l'avance du curseur, inverser deux frames ou décaler un offset d'une frame doit faire échouer un test. Le dépôt a déjà pratiqué cette vérification explicitement sur le module de rejeu.
- **Le mécanisme de construction se teste par la situation qu'il doit empêcher** : modifier un source du moteur doit provoquer la régénération du fichier de pixels ; ne rien modifier ne doit rien régénérer ; une empreinte fausse doit faire échouer la suite avec un message nommant les deux valeurs. Art antérieur : la façon dont le PRD des prérequis d'en-têtes a vérifié son propre mécanisme.
- **Le visualiseur n'est pas testé par un agent qui le regarde**, parce qu'aucun agent ne le peut. Ses modules le sont ; la fenêtre elle-même est validée par un humain. La conséquence est une exigence sur le découpage, pas sur les tests : si une partie du visualiseur n'est pas testable sans fenêtre, c'est qu'elle est au mauvais endroit.
- Les binaires de test tournent sous les instrumentations mémoire et comportement indéfini déjà en place, comme le reste de la suite.

## Out of Scope

- **Toute compression du fichier de pixels.** Décision prise sur mesures ; y revenir demanderait un besoin nouveau — transmettre le fichier, le conserver, ou le rendre navigable sans le lire séquentiellement — dont aucun n'existe aujourd'hui.
- **Toute empreinte par frame.** Localiser la première frame divergente se fait en comparant deux fichiers bruts locaux avec un outil ordinaire ; un index par frame coûterait plus cher que la donnée qu'il indexe.
- **Un comparateur visuel de deux captures** — deux sessions côte à côte, canal de différence, saut à la première divergence. Explicitement écarté : il n'a pas été demandé, et il ne sert à rien tant qu'il n'existe pas de baseline signée. Le format brut ne lui ferme aucune porte si le besoin apparaît.
- **L'export en vidéo, en GIF ou en images** depuis le visualiseur.
- **La capture pendant une session raylib interactive.** Les pixels d'une session sont reproductibles à partir de son enregistrement d'entrées ; les capturer en direct coûterait du temps de frame à un humain qui joue, pour une donnée régénérable.
- **La navigation aléatoire dans le visualiseur** (barre de défilement, saut à une frame arbitraire, chapitres à partir du fichier de marqueurs). Le format brut la rend triviale à ajouter plus tard ; elle n'est pas dans la demande.
- **Le choix du scénario de référence** — quelle session enregistrée sert de baseline, et sa durée. Ce PRD fournit l'instrument ; `docs/plan-2.md` §5 porte ce choix, y compris son avertissement d'ordonnancement.
- **Le son**, hors périmètre du portage.

## Further Notes

Ce PRD est l'outillage d'un autre chantier. `docs/plan-2.md` §5 décrit son « Instrument 2 — filet de non-régression par empreintes » et §6 en fait son critère de fin n° 4. Ce qui est écrit ici en est la réalisation, avec deux écarts par rapport au texte de `plan-2.md`, assumés :

- **Une seule empreinte pour toute la session, au lieu d'une par frame.** `plan-2.md` prévoyait un fichier d'empreintes ligne par ligne. Une empreinte unique suffit à détecter, et la localisation se fait sur les fichiers de pixels eux-mêmes plutôt que sur un index — ce qui est moins de code et moins de choses à maintenir cohérentes.
- **Les pixels de chaque frame existent réellement, au lieu d'être régénérables « à la demande ».** `plan-2.md` comptait sur la capacité du dumper à réémettre la frame *N* ; ce PRD la produit systématiquement, ce qui est ce qui rend le visualiseur possible.

L'avertissement d'ordonnancement de `plan-2.md` §5 reste entier et s'applique tel quel : **capturer la baseline avant d'avoir tranché la population (c) figerait la troncature actuelle**, et toute correction ultérieure vers `flr` apparaîtrait comme une régression. Poser une empreinte, c'est affirmer que ce qui est dessiné est ce qu'on veut ; le faire trop tôt transforme un doute en fausse certitude.

Enfin, une note sur ce qui a été mesuré pendant le cadrage, pour ne pas refaire le travail : sur les 9 729 frames de l'`events.json` du dépôt, 0,80 % des pixels changent d'une frame à l'autre et 11 % des frames sont identiques à la précédente. Ces chiffres sont ce qui rendait la compression attrayante, et ils restent vrais ; ils n'ont simplement pas de destinataire tant que le fichier n'est ni conservé ni transmis. Si un jour l'un de ces deux besoins apparaît, un XOR-delta sur la frame précédente avec des images-clés périodiques est le bon point de départ — mesuré à 41× sans entropie et 113× avec, pour moins de 20 µs par frame — et le module de screencast est l'unique endroit à modifier.
