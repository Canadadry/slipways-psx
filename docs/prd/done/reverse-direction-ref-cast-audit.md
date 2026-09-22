---
title: "Audit des casts REF en direction inverse (ob_table lu comme liste)"
description: "Les ~36 sites castant une valeur REF en pico8_list_t* ne peuvent pas déborder, mais peuvent lire silencieusement des valeurs fausses — et le tag de type les rend désormais vérifiables."
status: needs-triage
---

## Problem Statement

Ce port modélise l'unique type table de Lua par trois structs C distincts (liste, table ordonnée, instance de classe). Une valeur `REF` ne porte aucun type : c'est au code appelant de savoir ce qu'il tient. Quand il se trompe, le compilateur ne dit rien, parce que les trois structs ont la même forme.

Cette ambiguïté a déjà produit quatre bugs indépendants (`pico8_gn`, `mz:hq`, le champ `jb` de `js:pq`, puis son champ `lq`), tous dans la même direction : une **liste lue comme table ordonnée**. Les éléments de la table faisant le double de la taille de ceux d'une liste, la boucle de copie sort de l'allocation — un vrai heap-buffer-overflow, bruyant, détecté par ASan et parfois par un segfault.

La direction **inverse** — une table ordonnée lue comme liste — existe aussi dans le code, sur environ 36 sites recensés par l'audit de pré-Phase 5. Elle a été classée sans danger, et elle l'est au sens strict : lire des éléments de 32 octets comme des éléments de 16 sous-lit, donc ne déborde jamais. Mais elle ne produit pas pour autant des valeurs justes : elle réinterprète le contenu de la table à un pas de moitié, ce qui donne des `pico8_value_t` fabriqués à partir de morceaux de clés et de valeurs voisines. Le symptôme n'est pas un crash mais un affichage faux, une valeur absurde ou une ligne vide — et personne ne le remarque.

C'est exactement le profil de bug le plus coûteux de ce port : silencieux, non couvert par les fixtures scriptées, et découvert seulement quand un humain finit par regarder le bon pixel. Le même audit qui a classé ces 36 sites sans danger a par ailleurs produit un **faux négatif** sur le champ `lq`, ce qui invite à ne pas prendre ses conclusions pour acquises.

## Solution

Reprendre ces sites maintenant que l'outil manque n'existe plus. Le tag de type introduit par le correctif du segfault `lq` place un discriminant en premier membre des trois structs : n'importe quelle `REF` peut désormais être interrogée sur ce qu'elle est réellement, avec une réponse « je ne sais pas » explicite pour tout le reste.

Pour chaque site castant une `REF` en liste, l'une de trois conclusions, **documentée site par site** :

1. **Sûr par construction** — la `REF` ne peut venir que d'un producteur qui fabrique une liste, et ce producteur est nommé dans le commentaire. C'est la seule conclusion qui autorise à ne rien changer, et elle doit citer le producteur, pas simplement affirmer l'invariant.
2. **Ambigu** — plusieurs producteurs alimentent le champ, ou l'un d'eux fabrique une table. Le site passe par une désambiguïsation sur le tag, comme le fait désormais l'argument `lq`/`h_` de `js:pq`.
3. **Déjà faux** — le site lit effectivement une table comme une liste aujourd'hui. C'est un bug d'affichage à corriger, avec un test qui le démontre.

Le livrable n'est pas « 36 gardes ajoutés » : c'est un classement justifié où chaque « sûr » nomme sa raison, plus les correctifs des cas 2 et 3.

## User Stories

1. En tant que développeur du portage, je veux savoir lesquels des sites de cast inverse sont réellement sûrs, afin de ne pas devoir les réexaminer à chaque bug d'affichage inexpliqué.
2. En tant que développeur du portage, je veux que chaque site déclaré sûr nomme le producteur qui le garantit, afin qu'un lecteur ultérieur puisse vérifier la conclusion au lieu de la croire.
3. En tant que développeur du portage, je veux que les sites ambigus interrogent le tag de type plutôt que de supposer, afin qu'un nouveau producteur ne réintroduise pas la famille en silence.
4. En tant que développeur du portage, je veux qu'un site lisant réellement une table comme une liste soit traité comme un bug d'affichage et non comme une curiosité théorique, afin qu'il soit corrigé et couvert par un test.
5. En tant que joueur, je veux que les panneaux affichent les vraies valeurs de la partie, afin de ne pas prendre de décisions sur des chiffres fabriqués par une mauvaise lecture mémoire.
6. En tant que développeur du portage, je veux que l'audit énumère les formes que chaque garde ne traite pas, afin de ne pas répéter le faux négatif qui avait classé le champ `lq` sûr parce qu'un garde existait.
7. En tant que développeur du portage, je veux une méthode d'audit reproductible plutôt qu'une passe manuelle unique, afin que la prochaine phase puisse la rejouer à moindre coût.
8. En tant que développeur du portage, je veux distinguer les sites atteints par les fixtures existantes de ceux qu'aucun scénario n'exerce, afin de savoir lesquels ne sont couverts par rien.
9. En tant qu'agent instruisant un bug futur, je veux pouvoir demander à une `REF` ce qu'elle est, afin de diagnostiquer une confusion de type sans reconstruire un coredump.
10. En tant que développeur du portage, je veux que l'audit produise une trace écrite dans le journal de portage, afin que sa conclusion survive à la session qui l'a produite.
11. En tant que développeur du portage, je veux savoir si le tag permet de supprimer des désambiguïsations heuristiques déjà en place, afin de réduire le nombre de mécanismes concurrents.
12. En tant que développeur du portage, je veux que l'instrumentation dynamique utilisée pour valider le tag soit réutilisable, afin de mesurer les tags réellement observés sur un rejeu plutôt que de raisonner uniquement statiquement.

## Implementation Decisions

- Le discriminant de type existe déjà et couvre les trois formes ; ce PRD le **consomme**, il ne le redéfinit pas.
- L'audit combine deux méthodes, parce que l'échec précédent vient d'avoir utilisé une seule. La méthode statique énumère les sites et leurs producteurs. La méthode dynamique instrumente les accesseurs de liste et de table pour vérifier le tag à l'entrée, puis fait tourner le rejeu enregistré complet et la suite : un site jamais atteint par aucun des deux est signalé comme non couvert, pas comme sûr.
- Un site déclaré sûr par construction reçoit un commentaire nommant son producteur. Un invariant non écrit est ce qui a permis le faux négatif précédent.
- Les sites ambigus passent par la désambiguïsation sur tag, avec les quatre réponses traitées explicitement, y compris l'inconnue. Le cas inconnu est bruyant (assertion), pas dégradé silencieusement : une ligne vide est plus difficile à remarquer qu'un arrêt.
- Ce PRD ne modifie aucune fonction portée dans son comportement. Toute divergence trouvée entre le C et `pico8/lua.lua` pendant l'audit est signalée séparément, pas corrigée au passage.
- Si l'audit révèle qu'une catégorie entière de sites disparaîtrait en changeant la représentation chez le producteur (comme cela a été fait pour le champ `jb` des lignes de menu), cette option est chiffrée avant d'ajouter des gardes un par un.

## Testing Decisions

- Un bon test ici observe un **résultat visible** — une valeur affichée, une ligne de panneau, un compteur — pas la forme interne d'une structure. Un test qui affirme qu'un garde a été appelé teste l'implémentation ; un test qui affirme que le panneau affiche le bon nombre teste le comportement.
- Prior art direct : le test de régression du segfault `lq` atteint le bug par construction d'état plutôt que par rejeu, et vérifie à la fois l'absence d'erreur sanitizer et le contenu exact de la ligne, y compris l'absence d'un index fantôme. Le même patron s'applique à un site de lecture inverse : construire l'état, lire la valeur affichée, comparer.
- Prior art pour la vérification au pixel : les tests de barre de menu supérieure et d'infobulle de boutique comparent des glyphes réellement rendus par la police du port. Un cas de lecture inverse produisant un texte faux se démontre de cette façon.
- Tout cas reproduisant une corruption mémoire tourne sous sanitizer, avec la détection de fuites désactivée — ce port fuit par conception, faute de ramasse-miettes. Le code de sortie seul n'est pas un oracle fiable pour cette famille : le segfault précédent disparaissait à `-O1` et `-O2` alors que le débordement était toujours signalé.
- Les sites classés « sûr par construction » ne reçoivent pas de test dédié ; leur justification est le commentaire nommant le producteur.

## Out of Scope

- Toute refonte de la représentation visant à fusionner liste et table ordonnée en un seul type. C'est la réponse de fond, mais elle touche l'ensemble du port et ne se décide pas dans un PRD d'audit.
- La direction déjà traitée (liste lue comme table) : elle est couverte par le tag et son test de régression.
- Le son, hors périmètre du portage.
- Toute correction de fidélité vis-à-vis de `pico8/lua.lua` découverte pendant l'audit : signalée, pas traitée ici.

## Further Notes

La valeur de ce PRD tient moins aux corrections qu'il produira — il est possible que les 36 sites soient effectivement sûrs — qu'à la trace écrite qu'il laissera. L'audit précédent a conclu « sûr par construction » sur un site qui ne l'était pas, et cette conclusion a été crue pendant toute une phase. Un audit dont chaque ligne nomme sa raison est réfutable ; un audit qui compte des sites ne l'est pas.

Le coût est modéré : le tag rend chaque vérification locale et mécanique, là où l'audit précédent devait raisonner sur des invariants non écrits.
