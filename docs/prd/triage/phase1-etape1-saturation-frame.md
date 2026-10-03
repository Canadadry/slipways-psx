---
title: "Saturation de la zone frame à la frame 4337 de full_game.json"
description: "Un seul tick qui rejoue plusieurs fois economy_recompute garde dans la zone frame le brouillon mort de chaque planet_refresh, jusqu'à dépasser 256 Ko."
status: needs-triage
---

## Problem Statement

`make state EVENTS=worst-case/full_game.json` s'arrête à la frame 4337 sur
`p8_fatal: zone frame full, limit 262144 bytes`. Tant que la partie pleine ne va
pas au bout, aucun pic de zone ne peut être mesuré, et le plan 7 (une partie
pleine dans 768 Ko) est bloqué dès sa première étape. La règle du plan interdit
de relever `FRAME_ZONE_BYTES` avant d'avoir écarté une fuite.

## Diagnostic (prouvé, par trace gdb sur le binaire `replay`)

- **Ce n'est pas une accumulation d'une frame à l'autre.** La zone frame est
  bien remise à zéro en tête de chaque `cart_update60()` : les dix frames
  précédant le fatal consomment chacune environ 3,6 Ko. Le fatal arrive en une
  seule frame.
- **La frame fatale** est un dépôt de slipway (`ui_handle_button` →
  `slipway_drop`) qui fait avancer l'horloge (`targeting_confirm` et
  `slipway_drop` appellent tous deux `economy_advance_clock`) et franchit une
  année (`economy_new_year`). `economy_recompute` tourne donc **plusieurs fois
  dans le même tick**. Chaque passage reconstruit les routes commerciales et
  rafraîchit **chaque planète** (`planet_refresh`, via le message refresh).
- **Ce qui s'accumule : le brouillon de `planet_refresh`.** Pour construire la
  ligne de ressources d'une planète, `research_display_planet_resource_row`
  prend dans la zone frame un tableau de cellules et un
  `planet_resource_row_tuple` (232 octets) par cellule imbriquée, puis
  `layout_row_new` produit la ligne définitive dans le slot `SLOT_KIND_ROW` de
  la planète (zone ui) ; la mise en page des tuples imbriqués alloue encore
  dans la zone frame. Une fois la ligne posée dans son slot, ce brouillon est
  mort (le code de `layout_row_construct` le dit : un tuple n'est emprunté que
  le temps de la mise en page, son texte est copié), mais il n'est rendu qu'à
  la frame suivante.
- **Chiffres de la frame fatale** (environ 335 Ko demandés pour 256 Ko) :
  - tuples de `planet_refresh` : environ 173 Ko (745 tuples) ;
  - lignes et mises en page imbriquées sous `planet_refresh` : environ 72 Ko ;
  - cellules de `research_display_planet_resource_row` : environ 14 Ko ;
  - reconstruction des routes (`planet_rebuild_trades`, `trade_new`,
    `trade_extend`) : environ 70 Ko ;
  - le reste, moins de 3 Ko.
- **Pics antérieurs** : 229 Ko à la frame 3016, puis une montée de 188 à
  213 Ko entre les frames 4200 et 4325, au rythme du nombre de planètes
  colonisées (environ 48 au fatal). Le coût d'un recalcul croît avec la
  galaxie, et son brouillon est multiplié par le nombre de recalculs du tick.

**Lecture au sens du plan :** c'est bien une fuite, et non un besoin : de la
place jamais rendue dans la frame, une remise à zéro qui manque à l'échelle de
`planet_refresh`. Au pic, la mémoire vivante est faible (lignes dans leurs
slots, routes du tick) ; c'est le brouillon mort des recalculs successifs qui
remplit la zone.

## Solution

Rendre à la zone frame, dès la fin de chaque `planet_refresh`, tout ce que ce
rafraîchissement y a pris. Le moteur gagne une **remise en arrière bornée de la
zone frame** : on note une marque, on travaille, on revient à la marque, et la
place prise depuis la marque redevient libre (empoisonnée sous ASan, pour que
`make sanitize-net` attrape tout pointeur gardé dans ce brouillon).
`planet_refresh` l'encadre. Avec les tailles actuelles, la frame 4337 retombe
vers 80 Ko et `full_game.json` va jusqu'à sa dernière frame. Le jeu ne change
pas : tous les `tests/e2e/*.sha256` restent identiques.

## User Stories

1. En tant que mainteneur du portage, je veux que `make state EVENTS=worst-case/full_game.json` aille jusqu'à la dernière frame sans `p8_fatal`, pour pouvoir mesurer les pics de toutes les zones (étape 3).
2. En tant que mainteneur, je veux que la correction rende la place inutilement gardée au lieu de relever `FRAME_ZONE_BYTES`, pour respecter la règle « fuite d'abord » du plan.
3. En tant que mainteneur, je veux que `FRAME_ZONE_BYTES` et toutes les autres tailles de zone restent celles d'aujourd'hui, pour que le critère « va au bout avec les tailles actuelles » soit vérifiable.
4. En tant que mainteneur, je veux que tous les `tests/e2e/*.sha256` restent identiques, pour être sûr que le rendu et la logique du jeu n'ont pas bougé.
5. En tant que mainteneur, je veux que `make test-all` passe sans s'allonger, pour garder la boucle rapide.
6. En tant que mainteneur, je veux que `make sanitize-net` passe, pour prouver qu'aucun pointeur n'est gardé dans le brouillon rendu.
7. En tant que développeur du moteur, je veux pouvoir noter une marque sur la zone frame et y revenir, pour rendre un brouillon dont je connais la fin de vie sans attendre la frame suivante.
8. En tant que développeur du moteur, je veux qu'un bloc pris avant la marque reste intact après le retour à la marque, pour pouvoir encadrer un traitement au milieu d'une frame sans casser ce que la frame a déjà produit.
9. En tant que développeur du moteur, je veux que la place reprise depuis la marque soit réutilisable par les allocations suivantes de la même frame, pour qu'un tick qui rejoue plusieurs recalculs ne paie qu'un brouillon à la fois.
10. En tant que développeur du moteur, je veux que, sous ASan, un accès à un bloc pris après la marque échoue après le retour, pour qu'un pointeur oublié dans le brouillon soit attrapé par la suite sanitizer et non par un hash e2e changé au hasard.
11. En tant que développeur du moteur, je veux que le retour à une marque prise dans une frame précédente soit refusé par un `assert`, pour qu'on ne rende pas par erreur de la place qui ne nous appartient plus.
12. En tant que développeur du jeu, je veux qu'un `planet_refresh` laisse la zone frame dans l'état où il l'a trouvée, pour que le nombre de recalculs par tick et le nombre de planètes ne fassent plus grossir la zone.
13. En tant que développeur du jeu, je veux que la ligne de ressources d'une planète, posée dans son slot de la zone ui, s'affiche à l'identique après le retour à la marque, pour que l'inspection et la carte ne changent pas.
14. En tant que développeur du jeu, je veux que `research_project_layout_row`, qui construit aussi une ligne de ressources mais dans la zone frame et la complète ensuite par `layout_row_add`, ne soit pas encadré, pour ne pas rendre une ligne encore vivante.
15. En tant que développeur du jeu, je veux que la reconstruction des routes commerciales reste hors de l'encadrement, parce que des listes de routes prises dans la zone frame sont relues plus tard dans le tick (le dessin relit ce que la mise à jour a écrit).
16. En tant que relecteur, je veux que le diagnostic chiffré figure dans le message de commit, pour que l'historique explique pourquoi l'encadrement est là.
17. En tant que mainteneur de l'étape 2 (verrou), je veux que `full_game.json` rejoué jusqu'au bout soit l'état de départ, pour pouvoir l'enfermer dans une cible dédiée sans retoucher au code de jeu.

## Implementation Decisions

- **Module moteur `frame_zone` (approfondi).** Deux opérations nouvelles :
  noter une marque (la place prise à cet instant, dans la frame courante) et
  revenir à une marque (la place prise depuis redevient libre). Contrat C à
  écrire dans l'en-tête : un bloc pris avant la marque reste valide ; un bloc
  pris après est mort au retour ; une marque n'est valable que dans la frame où
  elle a été prise. La marque est une valeur opaque petite (pas d'allocation).
- **Module `zone` (bump zone).** La remise en arrière d'une bump zone vers une
  position donnée est l'opération de base ; sous ASan elle empoisonne
  l'intervalle rendu dans la moitié courante, comme `bump_zone_reset`
  empoisonne la moitié qu'il quitte. La marque doit permettre de détecter
  qu'un `frame_zone_reset` est passé entre-temps (par exemple en portant la
  moitié courante et un compteur de génération), et le retour l'affirme par un
  `assert` porteur de message.
- **Angle mort ASan à noter dans le contrat :** après le retour, les
  allocations suivantes reprennent les mêmes adresses ; un pointeur gardé lira
  le nouveau bloc sans planter. C'est le même angle mort que `arena_flush`,
  à documenter de la même façon. La protection reste : l'empoisonnement entre
  le retour et la réutilisation, et les hashes e2e.
- **Module `game/planet_display` : un seul site d'appel.** `planet_refresh`
  prend une marque en entrée et y revient avant de rendre la main, une fois la
  ligne posée (ou libérée) dans son slot `SLOT_KIND_ROW`. Rien d'autre n'est
  encadré : ni `economy_recompute` entier (les routes du tick sont relues par
  le dessin), ni `research_project_layout_row`.
- **Piège à vérifier avant d'encadrer :** la ligne définitive garde dans
  `items[]` des cellules de type ligne qui pointent vers les tuples du
  brouillon (`row_own_cell` les garde telles quelles) ; seul `placed_items[]`
  est censé être relu. Si quoi que ce soit relit `items[]` d'une ligne de
  planète après `planet_refresh` (un `layout_row_add`, une nouvelle mise en
  page), l'encadrement est faux. La preuve attendue : `make sanitize-net` vert
  et e2e identiques.
- **Tailles inchangées.** `zone_sizes.h` n'est pas touché ; ni `FRAME_ZONE_BYTES`
  ni `ZONE_BUDGET_BYTES` ne bougent dans cette étape.
- **Pas de fonction exposée pour les seuls tests** (pas d'accesseur « octets
  utilisés » ajouté pour l'occasion) : les tests observent le comportement
  (une zone qui ne se remplit plus, un bloc qui survit, un accès empoisonné).

## Testing Decisions

- **Un bon test** n'observe que le comportement externe : une allocation qui
  réussit ou un `p8_fatal`, le contenu d'un bloc qui survit, un accès qu'ASan
  refuse. Il ne lit pas les champs internes de la zone.
- **`frame_zone`, dans le test existant de la zone frame** (cas rejoués en
  processus fils sous ASan, l'enfant relu par son code de sortie et son
  stderr ; les cas « vivant dans la frame », « mort à la frame suivante » et
  « zone pleine » sont l'antériorité directe) :
  - un bloc pris avant la marque garde son contenu après le retour et de
    nouvelles allocations ;
  - un bloc pris après la marque est empoisonné après le retour (le fils meurt
    sous ASan) ;
  - prendre puis rendre, en boucle, bien plus que `FRAME_ZONE_BYTES` dans une
    même frame ne déclenche pas `zone frame full` ;
  - revenir à une marque prise avant un `frame_zone_reset` échoue sur
    l'`assert`.
- **`planet_display`, test de régression du bug, dans le test de la planète**
  (antériorité : les cas qui appellent déjà `planet_refresh` et vérifient la
  ligne posée dans son slot) : dans une seule frame, rafraîchir une planète à
  ligne pleine (la plus grande que `planet_refresh` sait construire, déjà
  utilisée par le test des lignes de mise en page) assez de fois pour dépasser
  256 Ko de brouillon cumulé ; le test est rouge aujourd'hui (`p8_fatal`), vert
  après. La ligne obtenue est la même qu'après un seul rafraîchissement.
- **Acceptation, hors `make test`** : `make state EVENTS=worst-case/full_game.json`
  va jusqu'à la dernière frame, sans `p8_fatal`. Ce rejeu n'entre ni dans
  `make test` ni dans `make e2e` (la cible verrou est l'étape 2).
- **Invariants** : `make test-all` et `make sanitize-net` passent ; tous les
  `tests/e2e/*.sha256` sont identiques.

## Out of Scope

- La cible verrou qui rejoue `full_game.json` (étape 2).
- La mesure des pics de zones et de caps, `tmp/state.txt` (étape 3).
- Tout redimensionnement de zone et le passage de `ZONE_BUDGET_BYTES` à 768 Ko
  (étape 4).
- Réduire le nombre de `economy_recompute` par tick, ou le brouillon de la
  reconstruction des routes : ce serait toucher à l'ordre des effets du cart,
  et la frame tient sans cela.
- Le budget de slipways et de relais, toute évolution du jeu.
- L'audio, les remesures sur MIPS.

## Further Notes

- Le diagnostic a été établi sans modifier le code : un point d'arrêt sur
  `p8_fatal`, puis un script gdb qui compte, par pile d'appel, les octets pris
  dans la zone frame et relève `used` à chaque `bump_zone_reset`.
- Si, une fois l'encadrement posé, `full_game.json` sature encore plus loin,
  c'est un autre brouillon : on refait le même relevé avant de toucher aux
  tailles, comme le veut le plan.
- Le message de commit porte les chiffres du diagnostic ; aucun commentaire
  de récit n'est ajouté au code (règle des commentaires du `CLAUDE.md`).
