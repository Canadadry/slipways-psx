---
title: "Reprendre une partie depuis un enregistrement dans le backend raylib"
description: "Le pilote interactif accepte un events.json en argument, le rejoue à l'écran, puis rend la main au joueur à la fin du fichier."
status: needs-triage
---

## Problem Statement

Le backend raylib enregistre systématiquement chaque session jouée. Ces enregistrements sont aujourd'hui consommables **uniquement** par le pilote headless : un agent peut les rejouer et inspecter le tampon d'écran, mais le joueur, lui, n'a aucun moyen de revoir sa propre partie, ni de repartir de l'état où elle s'est arrêtée.

Cela pose trois problèmes distincts.

D'abord, la partie est perdue. Une session de trente minutes interrompue par un crash — ou simplement par la fin de la journée — ne peut pas être reprise : il n'existe aucune sauvegarde, et le seul artefact qui décrit intégralement la partie est un enregistrement que rien d'interactif ne sait relire. Le déterminisme est pourtant acquis : le générateur pseudo-aléatoire de ce port est à graine fixe, et rejouer les mêmes entrées reproduit exactement le même état.

Ensuite, le joueur ne peut pas voir ce que l'agent voit. Quand un bug est diagnostiqué à partir d'un enregistrement, la conversation porte sur des numéros de frame et des vidages de tampon en texte. Le joueur n'a aucun moyen de regarder la séquence se dérouler et de reconnaître le moment où les choses tournent mal.

Enfin, atteindre un état de jeu profond est coûteux. Le segfault instruit récemment exigeait environ 9589 frames de jeu réel avant que l'état fautif n'existe : la contrainte est l'état accumulé, pas la complexité des entrées. Reprendre depuis un enregistrement est le seul moyen praticable pour un humain de revenir à un état tardif sans rejouer une demi-heure.

## Solution

Le pilote interactif accepte un chemin d'enregistrement en argument de ligne de commande. Au démarrage, il rejoue ce fichier à l'écran — le joueur voit sa partie se dérouler — puis, à la dernière frame, **rend la main** : la fenêtre reste ouverte, l'entrée redevient celle de la souris et du clavier, et la partie continue exactement là où l'enregistrement s'arrête.

Sans argument, le comportement actuel est inchangé : nouvelle partie à l'écran-titre.

Deux propriétés font la valeur de la fonctionnalité, et aucune n'est optionnelle.

La première est la **fidélité du démarrage**. Un enregistrement produit par le backend raylib commence à l'écran-titre et contient son propre clic de difficulté. Le pilote headless, lui, simule ce clic avant de rejouer, ce qui désynchronise immédiatement tout enregistrement raylib. C'est précisément le piège qui a rendu le dernier segfault irreproductible au premier essai. Le rejeu interactif doit partir du même état de départ que celui qui a produit l'enregistrement, sinon il diverge et n'a aucune valeur.

La seconde est la **continuité de l'enregistrement**. Une fois la main rendue, la session reste enregistrée, et l'enregistrement résultant doit être rejouable à son tour — donc contenir les frames rejouées **et** les nouvelles. Sans cela, reprendre une partie casse la chaîne : la session issue d'une reprise ne serait plus reproductible, et le mécanisme qui vient d'être corrigé pour que tout crash soit rejouable perdrait sa garantie dès la première reprise.

## User Stories

1. En tant que joueur, je veux relancer le jeu sur un enregistrement, afin de reprendre ma partie là où elle s'était arrêtée plutôt que de la perdre.
2. En tant que joueur, je veux voir le rejeu se dérouler à l'écran, afin de reconnaître le moment où quelque chose a mal tourné.
3. En tant que joueur, je veux récupérer le contrôle à la fin du fichier sans que la fenêtre se ferme, afin d'enchaîner directement sur du jeu réel.
4. En tant que joueur, je veux que la partie reprise soit exactement celle que j'avais, afin que mes crédits, mes planètes et mes technologies soient ceux que j'avais laissés.
5. En tant que joueur, je veux lancer le jeu sans argument et obtenir une partie neuve, afin que la fonctionnalité n'impose rien à qui n'en veut pas.
6. En tant que joueur, je veux savoir où en est le rejeu pendant qu'il se déroule, afin de ne pas croire que le jeu est figé.
7. En tant que joueur, je veux pouvoir abréger un rejeu long, afin de ne pas attendre une demi-heure pour revenir à mon état de jeu.
8. En tant que joueur, je veux que la session reprise soit elle-même enregistrée, afin qu'un crash survenant après la reprise reste rapportable.
9. En tant que joueur, je veux que l'enregistrement issu d'une reprise soit rejouable à son tour, afin de pouvoir reprendre une partie déjà reprise.
10. En tant que développeur du portage, je veux atteindre un état de jeu profond sans rejouer la partie à la main, afin de vérifier un correctif sur l'état réel qui a produit le bug.
11. En tant que développeur du portage, je veux que le rejeu interactif parte du même état initial que le backend qui a produit l'enregistrement, afin qu'il ne diverge pas comme le pilote headless le fait.
12. En tant que développeur du portage, je veux qu'un fichier illisible ou malformé soit signalé clairement au lancement, afin de ne pas confondre une erreur de chemin avec une partie neuve.
13. En tant que développeur du portage, je veux que le pilote headless reste inchangé, afin que la chaîne de vérification automatisée ne dépende pas de cette fonctionnalité.
14. En tant que joueur, je veux que le marqueur de frame posé pendant le jeu reste cohérent après une reprise, afin de pouvoir continuer à désigner un instant précis.
15. En tant que développeur du portage, je veux que le rejeu affiche les frames plutôt que de sauter le dessin, afin que ce qui est rejoué soit exactement ce qui serait joué — le dessin ayant des effets de bord sur l'état.

## Implementation Decisions

- L'argument est optionnel et positionnel ; son absence conserve strictement le comportement actuel.
- Le rejeu et le jeu partagent la même boucle : à chaque frame, l'entrée provient soit du fichier, soit du matériel. La bascule est un changement de source d'entrée, pas un second mode avec sa propre boucle — un pilote qui dupliquerait la boucle divergerait de celle réellement jouée, ce qui est la faute que ce PRD existe pour éviter.
- Le pilote interactif ne simule aucun clic préalable : il démarre à l'écran-titre comme aujourd'hui, ce qui est l'état de départ des enregistrements qu'il consomme. Cette décision est le cœur de la fonctionnalité, pas un détail.
- Le dessin s'exécute à chaque frame rejouée. Sauter le dessin fait diverger l'état : c'est une propriété mesurée du moteur, vérifiée pendant l'instruction du dernier segfault, où le chemin fautif disparaissait sans les effets de bord du dessin.
- La vitesse de rejeu est réglable et le rejeu est interruptible. Un enregistrement de trente minutes rejoué à la vitesse du jeu prend trente minutes, ce qui annule le bénéfice principal. Le comportement par défaut et le mécanisme d'abrègement sont à arbitrer avec l'utilisateur ; la contrainte est qu'abréger ne doit jamais sauter de frame, seulement réduire l'attente entre frames, sinon l'état obtenu n'est plus celui de l'enregistrement.
- L'enregistrement de la session reprise contient les frames rejouées puis les nouvelles, de sorte qu'il reste un enregistrement complet et autonome, rejouable par les mêmes moyens. Reprendre une partie ne doit pas produire un fichier qui ne décrit qu'une moitié de l'histoire.
- L'enregistrement de chaque frame continue de précéder l'exécution du jeu, conformément au correctif récent : un crash survenant après la reprise doit laisser un fichier contenant la frame fautive.
- Un fichier absent ou malformé est une erreur de lancement explicite, jamais un repli silencieux sur une partie neuve : ce repli ferait croire à une perte de sauvegarde.
- L'état de jeu n'est jamais sérialisé. La reprise passe par le rejeu des entrées, ce que le déterminisme du générateur à graine fixe rend exact. Une sauvegarde d'état serait une fonctionnalité différente, plus rapide mais exigeant de sérialiser tout le graphe d'entités.

## Implementation Chunks

1. Lecture de l'argument et chargement de l'enregistrement dans le pilote interactif, avec l'erreur de lancement explicite. Aucun changement de comportement sans argument.
2. Source d'entrée commutable dans la boucle existante : les frames viennent du fichier tant qu'il en reste, du matériel ensuite. C'est le cœur fonctionnel — à ce stade la reprise marche, à vitesse de jeu.
3. Continuité de l'enregistrement : la session reprise écrit un enregistrement complet, contenant les frames rejouées puis les nouvelles, vérifié rejouable.
4. Confort de rejeu : réglage de vitesse, abrègement, indication de progression, cohérence du marqueur de frame après reprise.

## Testing Decisions

- Un bon test ici observe l'**état de jeu atteint**, pas la mécanique de lecture du fichier. Vérifier que le fichier a été lu ne prouve rien ; vérifier qu'après rejeu d'un enregistrement connu l'état correspond à celui attendu prouve la fonctionnalité.
- Contrainte majeure d'environnement : le pilote interactif ne peut pas être lancé, et raylib n'est pas vendorisé partout. La logique testable doit donc être placée hors du module raylib, comme l'a déjà été la conversion de coordonnées — un module sans dépendance raylib, testé directement, est le précédent à suivre.
- La logique réellement testable est la **source d'entrée commutable** : étant donné un enregistrement de N frames, elle fournit les frames du fichier pour les N premières puis signale l'épuisement. Elle se teste sans fenêtre et sans matériel, et c'est elle qui porte le comportement décrit par les user stories 1, 3 et 9.
- La continuité de l'enregistrement se teste de bout en bout en headless : produire un enregistrement de reprise, le recharger, et vérifier qu'il contient les frames rejouées suivies des nouvelles, et qu'il se recharge sans erreur. Prior art : les tests existants de l'écriture incrémentale d'enregistrement vérifient déjà qu'un fichier écrit frame par frame est identique à un fichier écrit en une fois.
- L'équivalence entre rejeu et jeu réel se vérifie en headless en comparant l'état atteint après rejeu d'un enregistrement à celui atteint en rejouant le même enregistrement par le pilote headless démarré sans clic simulé.
- Ce qui ne peut pas être vérifié ici doit être énoncé comme tel plutôt que contourné : la bascule effective du rejeu vers l'entrée matérielle dans une vraie fenêtre demande une session humaine. Le PRD ne doit pas prétendre la couvrir.

## Out of Scope

- La sauvegarde et le rechargement d'état de jeu sérialisé. Fonctionnalité distincte, décidée contre le rejeu d'entrées ci-dessus.
- Toute modification du pilote headless, qui reste la chaîne de vérification automatisée.
- Le rejeu en arrière, le pas à pas ou le déplacement libre dans la timeline.
- L'édition d'un enregistrement depuis le jeu.
- Le son, hors périmètre du portage.
- Une interface graphique de sélection d'enregistrement : l'argument de ligne de commande suffit.

## Further Notes

Cette fonctionnalité avait été demandée pendant l'instruction du segfault de relâchement de souris, sous condition — « si tu n'arrives pas à reproduire ». La reproduction ayant finalement abouti, elle n'a pas été construite. La condition a masqué le fait que son intérêt principal est indépendant du diagnostic : reprendre une partie perdue et revoir sa propre session valent par eux-mêmes.

Le déterminisme dont dépend toute la fonctionnalité n'est pas une hypothèse : il est déjà exploité par la suite headless, dont plusieurs tests rejouent des enregistrements et affirment des valeurs exactes de crédits et de positions à des frames données.
