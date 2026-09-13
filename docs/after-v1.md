# Idées pour après la v1

Stock d'idées d'interaction manette, volontairement écartées de la v1 (voir `docs/plan.md`, Phase 2) pour garder celle-ci simple. Rien ici n'est planifié ni engagé — à réévaluer seulement une fois qu'une v1 propre et fonctionnelle existe, et seulement si elle s'avère trop limitée en usage réel.

## Mode saut (navigation par graphe de voisins, 8 directions)

Alternative ou complément au mode souris v1 : le D-pad saute de spot en spot via un graphe de voisins précalculé, au lieu de déplacer un curseur libre en pixels.

Invention propre au portage manette (absente du PICO-8, où le clic vise directement un point) : pour permettre au D-pad de sauter de planète en planète malgré une disposition procédurale (pas une grille), chaque spot précalcule son meilleur voisin dans chacune des 8 directions cardinales (E/SE/S/SW/W/NW/N/NE, angles 0/45/90/135/180/225/270/315).

Pour un spot donné et une direction donnée : parmi tous les autres spots dont l'écart angulaire à la direction est inférieur à 90°, on choisit celui qui minimise `distance × (1 + 2 × écart_angulaire / 180)` — une pénalité qui privilégie un spot bien aligné sur l'axe à un spot plus proche mais excentré, sans l'exclure totalement (facteur de pénalité `2`, cône de tolérance `90°`, ces deux constantes étant elles-mêmes une invention d'interaction, pas une valeur à chercher dans `pico8/lua.lua`). Ce calcul serait fait une fois à la génération (et recalculé pour les spots affectés lors d'une insertion dynamique — wormhole, structures spéciales), pas à chaque frame.

Un mode saut ne couvre pas tout seul les positions qui ne sont pas des spots existants (pose de sonde, de wormhole) : il faudrait le combiner avec le curseur libre du mode souris pour ces cas, pas le remplacer entièrement.

## Mapping dédié par panneau / action

Alternative au mode souris générique v1 (curseur + clic universel) : une state machine de modes avec des boutons dédiés par action et par panneau (tech, choix de colonisation, tracé de liaison, etc.), plutôt qu'un seul curseur générique qui émule une souris partout.

Piste notée : remplacer le drag-and-drop de tracé de slipway par un mode « sélection source → sélection destination » en 2 pressions (au lieu d'un maintien de bouton + déplacement du curseur). Les règles de blocage (portée, croisement, planète sur le trajet) restent identiques quel que soit le mapping — elles ne dépendent pas de la souris dans le code source.

Non retenu pour la v1 : ajoute de la complexité (une state machine par panneau) pour un gain qui reste à démontrer tant que le mode souris générique n'a pas été essayé en pratique.
