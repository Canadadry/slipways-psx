---
title: "Plan 4, étape 4c — Registres hv/cv/cg en zone partie, listes statiques dans la sous-zone chargement"
description: "Les registres hv, cv et cg et les deux listes statiques faites au chargement (g_shared_le, g_ga_by_shared) prennent encore leur capacité à la transitoire. 4c met les registres dans la génération de partie, ouvre la sous-zone chargement prévue par 4a pour les deux listes statiques, borne ce que laissent les agrandissements par copie, et refait la mesure et la projection du budget."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. L'étape 4 range les objets de partie dans la **zone partie**. Cette PRD traite la sous-étape **4c**. Rédigée pendant 4b, elle posait ce qu'elle supposait de 4b en hypothèses (H-4b-1 à H-4b-9). 4b est finie (f9e4f8b ; clôture 1538d87). Toutes les hypothèses ont été vérifiées à `HEAD` 1538d87 (section « Vérifié contre 4b »), et les sections qui en dépendaient sont réécrites sur les faits. Le reste a été lu à 9cf286e et relu à 1538d87 : aucun fichier de 4c n'a changé entre les deux.

**Le texte de l'étape**, recopié du plan à 1538d87 (4b finie) :

> 4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement, et si son remplissage ne monte pas pendant une phase sans action d'un e2e.
>    Elle se fait en sous-étapes, une PRD chacune :
>    - **4a. Socle.** La zone partie, vide, avec sa remise à zéro et son empoisonnement sur nouvelle partie, son accesseur de remplissage (octets pris, et maximum, §3) écrit par le rejeu, et l'inventaire du §4 des pointeurs qui survivent à une nouvelle partie. Aucun propriétaire converti.
>    - **4b et suivantes.** Une famille de propriétaires par PRD dans l'ordre fixé par la PRD 4a : 4b états de `pico8_gen.c` ; 4c tableaux croissants et registres (crée la sous-zone chargement) ; 4d planètes et relais ; 4e slipways et routes ; 4f trous de ver ; 4g recherche et économie ; 4h singletons de `jl`/`eu` et objets du joueur (famille ajoutée par 4a : les « Reçu » des étapes 1 à 3 leur reviennent). N'entre en zone partie qu'une allocation faite une fois par objet de partie ; un champ réaffecté par tick ou par action est réécrit en place, devient une valeur, va en zone frame ou prend une place. Chacune relève le maximum de remplissage sur tous les e2e et le note ; dès que le relevé cumulé, prolongé aux familles restantes, dépasserait ce que les 1,5 Mo laissent, l'étape s'arrête et le signale (§3) sans attendre la dernière famille.
>    - *Découvert (4a) :* la partie bascule en tête de `pico8_eu`, avant son `jl()` ; `_init` (titre, « restart », « again? ») ne bascule pas. L'ancienne génération est rendue au vidage suivant (en jeu, l'update qui suit le clic), après un point d'oubli de partie, vide à ce jour : rendre dans `jl()` laissait lire `ok:lj` sur l'ancien `ok`, la file, `fm`/`ju`. Zone à deux bouts, un seul stockage de 768 Ko ; une double bascule sans vidage est refusée ; `cart_load`/`shutdown` remettent tout à zéro. Un champ de plus dans `pico8_zone_choisie_t` grossissait chaque en-tête d'arène et faisait tomber `qy_fv` : la poignée de partie vit à part. Angle mort ASan : un pointeur gardé à travers deux bascules ne plante plus. Inventaire du §4 refait (éclats `ga` compris) : rien ne lit l'ancienne génération après son rendu. Les 81 relocations de l'étape 4 sont attribuées aux familles 4b–4h ; la seule marge est de 49 Ko sur la somme des zones, la zone particules gardant ses ~17 Ko libres pour les traînées `ts`. Le rejeu écrit `partie` et `max_octets partie`.
>    - *Reçu de l'étape 4a :* les traînées `ts` de `jc:ch` (par frame) vont à 4e, en zone particules, places de 24 o.
>    - *Découvert (4b) :* `pico8_jf`/`pico8_gn` prennent la zone de l'appelant ; `bw` fait ses états en zone partie après la bascule, `oh` de `bg:pq` va en zone frame. Le libérateur n'était pas `jo:pq` (comme le disait la table de 4a) mais `jo_set_oc`/`_ir`/`_shutdown`. Seuls `g_jo_oc`/`g_jo_ir` visent un état, repointés à chaque `eu` : point d'oubli toujours vide. Mesure : 608 o par partie, maximum 1216 au clic de `pause_restart` (deux générations) ; projection ≈ 262 Ko sous 768, on continue. Comptes : 24 / 84 → 23 / 80.
>    - *Reçu de l'étape 1 :* même chose pour les boîtes `vec` des objets de partie (`p`/`v`/`ra` de `qb`, `ra` de `nf`, `mo->v`, `re`/`p` de `jc:hq`, `p` de `pico8_bv`, `ie.gy`) ; `qb:ph` seul en fait trois par frame. L'état `oh` de `pico8_bg_pq` est un objet de frame fait par `pico8_jf`, qui ne prend pas de zone : l'étape 4, qui donne une zone aux états de `pico8_gen.c`, le range en zone frame.
>    - *Reçu de l'étape 2 :* restent à cette étape `ok.qa` (gardé par les hôtes de `bv` et le point souris de `jc:hq` : objet relu par des objets de partie), la structure `ok` et ses calques `ic`, la liste statique de départ, `ie.jb` (fait une fois par partie), `fv`, la structure `pj`, `hp.i`. Les structures de planète ne sont toujours pas libérées (seule leur rangée l'est).
>    - *Reçu de l'étape 3 :* la structure `g_` (et `fv`) et sa boîte `p` de `bv` restent à cette étape (`/* TROU-7 */`) ; `oe` d'une planète ordinaire passe par `pico8_jo_pq_dans`, qui prend déjà sa zone.

**La ligne « Partie » du §2**, recopiée :

> | Zone | Ce qu'elle contient | Durée de vie |
> |---|---|---|
> | **Partie** | planètes, relais, slipways, routes `trade`, recherche, trous de ver ; les tableaux croissants (`ents`, `ptrs`, registres) qui les contiennent ; les objets faits au chargement (`gb`, `hp.i`, listes statiques) dans une sous-zone chargement, prise dans la zone partie, remise à zéro par le seul chargement | une génération par partie, de `eu` à `eu` ; rendue d'un bloc au vidage de la file qui suit le `eu` suivant (zone à deux bouts) ; `_init` ne la rend pas |

**La ligne de 4c dans la table des familles de la PRD 4a**, retirée par b46601e. On la lit par `git show b46601e~1:docs/prd/triage/phase1-etape4a-socle-zone-partie.md`. Recopiée :

> | Ordre | Famille | Sites (fonction, ligne) | Relocations | Reçus traités |
> |---|---|---|---|---|
> | **4c** | **Tableaux croissants et registres** | `pico8_entity_init` (78, 82, 86 : `hv`, `cv`, `cg`) ; `pico8_ui_init` (145 : `g_shared_le`) ; `pico8_planet_family_init` (531 : `g_ga_by_shared`) ; crée la sous-zone chargement | `entity` 3 → sort ; `ui` 6 → 5 ; `planet_family` 6 → 5 | *Reçu 2* : la liste statique de départ |

La raison de l'ordre, recopiée de la même PRD :

> **4c** vient ensuite parce que toutes les autres familles inscrivent leurs objets dans `hv`/`cv`/`cg`. Elle fixe aussi la tenue des tableaux croissants dans une zone qui ne rend pas : un agrandissement laisse un bloc mort jusqu'à la génération suivante, ce qui coûte au plus le double de la taille finale. Enfin, elle crée la sous-zone chargement, dont 4g aura besoin.

La sous-zone chargement, telle que la PRD 4a l'a décidée (section « Module de zone partie ») :

> **Sous-zone chargement : décidée ici, créée par la famille 4c.** C'est un changement du plan : le §2 n'a pas cette durée de vie, et la clôture de 4a l'écrit dans la ligne « Partie ». 4c est la première famille qui en a besoin (`g_shared_le`, `g_ga_by_shared`). Ce sera une zone montante à une moitié, remise à zéro seulement par `pico8_cart_load()`/`pico8_cart_shutdown()`, prise **dans** `PICO8_ZONE_GAME_BYTES`, avec une vérification à la compilation que chargement + partie ≤ 768 Ko. Taille de départ au doigt mouillé : 64 Ko, prise sur les 768 (partie 704 Ko). Elle ne reçoit que ce que fait le chargement. Ce qui y grandirait ensuite (`g_shared_le`, où `ok.le` ajoute des boutons) doit y réserver sa capacité au chargement, comme la liste `qy` de l'étape 3, ou aller ailleurs. 4a ne la crée pas : aucune poignée sans utilisateur.

La méthode de suivi du budget, recopiée de la PRD 4a :

> **Méthode, appliquée par chaque famille k (4b à 4h) :**
>
> 1. Après sa conversion, rejouer les 26 e2e avec `PICO8_REMPLISSAGE`. Relever le maximum de la zone partie sur l'ensemble (`M_k`), et celui de la sous-zone chargement à partir de 4c. Le noter dans le message du commit et dans « Découvert ». La forme est laissée à l'implémenteur : boucle de shell dans le commit, ou cible `make` hors suite comme `class-coverage`.
> 2. **Projection** : `P_k = M_k + Σ` (estimations des familles restantes) `× r`. `r` vaut le rapport « mesuré / estimé » des familles déjà faites, s'il dépasse 1, et 1 sinon. Une famille dont la mesure dépasse son estimation fait monter la projection des suivantes.
> 3. **Décision** :
>    - `P_k ≤ PICO8_ZONE_GAME_BYTES` (768 Ko, sous-zone chargement comprise) : on continue ;
>    - `768 Ko < P_k ≤ 768 Ko + marge`, avec marge = 1536 − (frame + interface + particules) = **49 Ko** à e88df7a : la famille relève `PICO8_ZONE_GAME_BYTES` d'un chiffre, dans son commit, et le dit. Les ~17 Ko libres dans les 128 Ko de la zone particules n'entrent pas dans cette marge : ils restent aux particules (traînées `ts` de 4e), et les reprendre serait rogner une autre zone (§3). Si 4e a besoin de relever la zone particules pour `ts`, ce relèvement est pris sur les 49 Ko, et la projection des familles suivantes le compte ;
>    - `P_k` au-delà : **arrêt** de l'étape 4 à la famille k, et signalement à l'humain (zone partie, limite voulue = `P_k` arrondi, somme actuelle des zones). On ne rogne ni la frame, ni l'interface, ni les particules (§3), et on n'attend pas la dernière famille.
> 4. **Croissance** : à chaque famille, le TU des places vivantes vérifie que les octets de la zone partie sont égaux aux frames 45 et 90 de `message_box_hold`. Une hausse est une allocation par tick de la famille. Elle se corrige dans la famille, pas en relevant la limite (§3).
> 5. Un `pico8_fatal` « zone partie pleine » en cours de famille se lit d'abord comme un site par tick ou par action entré tel quel, puis comme une limite trop basse. La note « Découvert » dit lequel.
>
> La sous-zone chargement suit la même méthode à part. Son maximum ne dépend pas de la bascule.

Les règles du plan qui cadrent 4c :

- §2 : la zone partie contient « les tableaux croissants (`ents`, `ptrs`, registres) qui les contiennent » et les objets faits au chargement, « dans une sous-zone chargement, prise dans la zone partie, remise à zéro par le seul chargement ». « Les tableaux croissants ne sont pas réécrits : ils gardent leur forme et prennent leur capacité dans leur zone au lieu de `realloc`. »
- §3 :
  - limites au doigt mouillé ;
  - somme des zones ≤ 1,5 Mo ; un relèvement qui la dépasserait arrête l'étape, qui le signale, sans rogner une autre zone ;
  - invariant de chaque commit : `make test-all`, `make sanitize-net`, `*.sha256` identiques ;
  - « Croissance par frame » : le remplissage d'une zone qui n'est pas remise à chaque frame est égal à deux frames séparées par une phase sans action.
- §4 :
  - un site douteux va dans la zone la plus longue *qui le couvre* ; seule la lecture le fait passer dans une zone plus courte ; ASan ne fait que rétrograder ;
  - avant de faire finir une durée de vie, la PRD inventorie mécaniquement les pointeurs du cœur qui survivent.
- §5 : fin d'étape = chaque site de l'inventaire converti, `alloc-guard` à égalité stricte, aucun compte n'a monté.

**Ce qui se passe aujourd'hui** (lu à 9cf286e, relu à 1538d87) :

- **Les registres.** `pico8_entity.c` tient `g_hv`, `g_cv[5].list` (`kj`, `ev`, `hd`, `hk`, `hb`) et `g_cg[10].list` (une par étiquette : `qh`, `hj`, `nr`, `g_`, `mh`, `jc`, `mz`, `jo`, `np`, `prober`). Ce sont 16 `pico8_ents_t`. `pico8_entity_init()` les remet à vide : d'abord `pico8_entity_shutdown()`, qui fait `pico8_ents_free` sur chacune, puis `pico8_ents_init(…, &pico8_zone_transitoire)` aux lignes 78, 82 et 86. Ce sont les 3 relocations de `entity`.
- **Qui les refait.** `pico8_entity_init()` n'a qu'un appelant dans le cœur : `pico8_jl()` (`pico8_bootstrap.c`). `jl()` met d'abord en file de sortie chaque entité de `hv` qui a `rendre`, puis appelle `pico8_entity_init()`. `jl()` est appelé par `_init` (démarrage, « restart », « again? ») et par `eu`, juste après la bascule de la zone partie. Les TU appellent aussi `pico8_entity_init()` directement.
- **Comment ils grandissent.** `pico8_ents_init` n'alloue rien (`items` NULL, capacité 0). `pico8_ents_add` double la capacité par `pico8_zone_grow` (4, 8, 16…). `pico8_ents_del` tasse sans rendre. Une liste ne rétrécit jamais : sa capacité est le plus grand nombre d'éléments qu'elle a tenus depuis son init.
- **Qui les lit.** Toujours à travers la structure statique : `pico8_hv()`, `pico8_cv(nom)` et `pico8_cg(étiquette)` rendent l'adresse de la `pico8_ents_t` statique, jamais `items`. La boucle de `dg` (TROU-5) relit `hv->items[i]` à chaque tour, et les itérateurs `pico8_ents_iter_t` sont des locaux qui gardent l'adresse de la structure. Aucun champ d'objet ne garde une liste de registre : le `grep` des affectations `= pico8_cg(`/`pico8_cv(`/`pico8_hv(` dans un champ ne trouve rien.
- **Les deux listes statiques.**
  - `g_shared_le` (`pico8_ui.c`), initialisée par `pico8_ui_init` (ligne 145 : `pico8_ents_init(&g_shared_le, &pico8_zone_transitoire)`). C'est la liste de départ de `ok.le` : un `ui` neuf a `le = &g_shared_le`. **Rien ne l'agrandit en jeu.** `ui:cj` et `pz` (`pico8_msgbox.c`) remplacent `ok.le` par une liste neuve, prise dans une place de la zone interface (`pico8_ui_le_new`), *avant* d'y ajouter des boutons. Seul un TU (`test_pico8_ui.c`) y ajoute un élément. La crainte de 4a (« où `ok.le` ajoute des boutons ») ne se vérifie pas à la lecture.
  - `g_ga_by_shared` (`pico8_planet_family.c`), initialisée par `pico8_planet_family_init` (ligne 531). C'est le `by` de chaque éclat `ga`. Les `by` des planètes sont réécrits par `bd_route_visit` (`jo->by = pico8_jo_ib(jo)`), qui ne parcourt que `cg.jo` ; un `ga` n'a aucune étiquette et n'y est pas. **Rien ne l'agrandit.**
  - Les deux sont faites par `pico8_cart_load()` (`pico8_ui_init`, `pico8_planet_family_init`) et vivent d'une partie à l'autre. `pico8_ui_init` est aussi rappelé par `jl()` quand `ui` n'est pas prêt (chemin sans `cart_load`, TU).
  - Comme `pico8_ents_init` n'alloue rien, leur seule référence à la libc est la **poignée** passée : 1 relocation chacune (`ui` 145, `planet_family` 531).
- **Le chargement et l'arrêt** (`pico8_cart.c`) : `pico8_cart_load()` fait vidage, `pico8_zone_partie_reset()`, puis les inits de modules (dont `pico8_ui_init` et `pico8_planet_family_init`). `pico8_cart_shutdown()` fait vidage, reset, puis les shutdowns. **Ni l'un ni l'autre ne touche `hv`/`cv`/`cg`** : les registres de la session précédente restent pleins jusqu'au prochain `jl()`, qui commence par les **relire** (boucle d'abandon).
- **La zone partie** (`pico8_zone_partie.{h,c}`, `pico8_zone.{h,c}`) : une seule réserve de `PICO8_ZONE_GAME_BYTES` (768 Ko), zone à deux bouts. `grow` copie et laisse l'ancien bloc mort jusqu'au rendu de sa génération. `release` ne fait rien. `reset` empoisonne les deux bouts. Le rejeu écrit `partie <n>` et `max_octets partie <n>` en dernières lignes de `PICO8_REMPLISSAGE`.
- **Ce que 4b a mis dans la zone partie** (f9e4f8b) : `jo.oc`, sa source et `jo.ir`, faits par `bw` dans `eu` après la bascule, soit `G` = 608 o par génération sur x86-64. Hors du module, seul `pico8_galaxy.c` appelle `pico8_zone_partie()`. Le maximum relevé est 1216 o au clic de `pause_restart` (deux générations), et 608 o sur les 25 autres e2e.
- **Le verrou** à 1538d87 : 23 fichiers, 80 appels (`pico8_gen.c` sorti), dont `entity` 3, `ui` 6 et `planet_family` 6. Les lignes 78, 82, 86, 145 et 531 sont celles de la table de 4a : 4b n'a touché aucun de ces fichiers.

**Le problème.** Toutes les familles 4d à 4h inscrivent leurs objets dans `hv`/`cv`/`cg`. Tant que ces registres prennent la transitoire, aucune ne peut finir sans eux. Deux questions doivent être tranchées avant :

1. **Quelle durée de vie pour les registres ?** Ils ne vivent pas « d'une partie à l'autre » : chaque `jl()` les refait.
2. **Que coûte un tableau croissant dans une zone qui ne rend rien avant la génération suivante ?** Chaque agrandissement copie et laisse un bloc mort.

Une troisième question vient du reset, qui empoisonnera désormais des tableaux encore désignés par les registres :

3. **Que deviennent les registres quand `cart_load`/`cart_shutdown` remettent la zone à zéro ?** Aujourd'hui, `jl()` les relirait ensuite.

## Solution

- **Registres en génération de partie.** `pico8_entity_init()` passe la poignée `pico8_zone_partie()` aux 16 listes. Un `jl()` de `_init` les refait dans la génération courante, celle de la partie quittée. Le `jl()` d'`eu` les refait dans la nouvelle, puisqu'il suit la bascule. Les anciens tableaux restent lisibles jusqu'au rendu, puis sont empoisonnés.
- **Sous-zone chargement**, créée comme 4a l'a décidé : une zone montante de 64 Ko, prise sur les 768 Ko de la zone partie (la génération de partie passe à 704 Ko). Elle est remise à zéro par le seul `pico8_zone_partie_reset()`, donc par le chargement et l'arrêt. `g_shared_le` et `g_ga_by_shared` y prennent leur capacité. Comme rien ne les agrandit en jeu, la sous-zone reste vide sur les 26 e2e : elle est prête pour 4g (`gb`, `hp.i`).
- **Reset sans relecture.** Avant leur vidage, `pico8_cart_load()` et `pico8_cart_shutdown()` abandonnent les entités de `hv`, comme le fait `jl()`. Après le reset, ils vident les 16 registres sans les lire. Aucun `jl()` ne relit plus un tableau empoisonné, et les places des entités abandonnées reviennent à leurs zones comme aujourd'hui.
- **Borne des blocs morts.** Une liste de capacité finale `C` a pris au plus `2C − 4` pointeurs dans sa génération, morts compris. Par génération, au plus deux `jl()` (celui d'`eu` et un `_init`). La borne est vérifiée par un TU, à l'octet près, sur `pause_restart` après le rendu.
- **Remplissage** : le rejeu écrit aussi `chargement <n>` et `max_octets chargement <n>`.
- **Verrou** : `entity` sort, `ui` 6 → 5, `planet_family` 6 → 5. Prévision **23 / 80 → 22 / 75**.
- **Budget** : `M_4c` relevé sur les 26 e2e, projection refaite en séparant génération (704 Ko) et chargement (64 Ko), décision écrite.

Aucun pixel ne change, aucun tirage `rnd` n'est déplacé.

## User Stories

1. En tant que mainteneur du plan, je veux que `hv`, `cv` et `cg` prennent leur capacité dans la génération de partie, pour que les familles 4d à 4h inscrivent leurs objets dans des registres qui n'appellent plus la transitoire.
2. En tant que développeur, je veux que le `jl()` d'`eu` refasse les registres dans la nouvelle génération, après la bascule, pour que les registres d'une partie vivent exactement autant qu'elle.
3. En tant que développeur, je veux que le `jl()` d'`_init` refasse les registres dans la génération courante sans basculer, pour que l'écran titre vive dans la génération de la partie qu'il suit, comme `fm` et `ju` (4a).
4. En tant que développeur, je veux que les anciens tableaux d'un registre restent lisibles jusqu'au rendu de leur génération, pour qu'un parcours commencé avant le `jl()` d'`eu` (abandon dans `jl()`, fin de `ok:lj`) lise de la mémoire vivante.
5. En tant que développeur qui chasse un accès hors durée de vie, je veux que les anciens tableaux soient empoisonnés au rendu, pour qu'un pointeur oublié vers eux fasse planter `make sanitize-net` sur `pause_restart`.
6. En tant que développeur, je veux que `pico8_cart_load()` et `pico8_cart_shutdown()` abandonnent les entités de `hv` avant leur vidage, pour que leurs places reviennent aux zones interface et particules comme aujourd'hui, où le `jl()` suivant le faisait.
7. En tant que développeur, je veux qu'après le reset de la zone partie les 16 registres soient vidés sans être lus, pour qu'aucun `jl()` ultérieur ne relise un tableau empoisonné.
8. En tant que développeur, je veux que ce vidage des registres ne touche ni `mo` ni l'état « initialisé » du module, pour qu'un TU qui lit `mo` ou `cv` entre `cart_load` et `_init` garde le comportement d'aujourd'hui.
9. En tant que futur porteur PSX, je veux que la sous-zone chargement soit prise dans `PICO8_ZONE_GAME_BYTES`, avec un contrôle à la compilation, pour que la somme des zones lue dans `pico8_zone_sizes.h` reste ce que la PSX paiera.
10. En tant que développeur, je veux une poignée sur la sous-zone chargement, avec `alloc`, un `grow` qui copie et un `release` sans effet, pour que les listes faites au chargement y prennent leur capacité sans changer de forme.
11. En tant que développeur, je veux que la sous-zone chargement ne soit remise à zéro que par `pico8_zone_partie_reset()`, donc par le chargement et l'arrêt, et jamais par une bascule ni par un rendu, pour que `gb`, `hp.i` et les listes statiques vivent d'une partie à l'autre.
12. En tant que développeur qui chasse un accès hors durée de vie, je veux que la sous-zone chargement soit empoisonnée à son reset sous ASan, et qu'un bloc gardé à travers un `cart_load` fasse planter même après de nouvelles allocations, pour que le critère de 0a vaille aussi pour elle.
13. En tant que développeur, je veux qu'une sous-zone chargement pleine sorte par `pico8_fatal` « zone chargement pleine, limite 65536 octets », pour qu'une limite trop basse se lise et se relève en un chiffre.
14. En tant que développeur, je veux que `g_shared_le` et `g_ga_by_shared` prennent la poignée de la sous-zone chargement, pour qu'elles quittent la transitoire avec une durée de vie qui les couvre.
15. En tant que développeur, je veux que `pico8_ui_shutdown` et `pico8_planet_family_shutdown` oublient leur liste statique sans la lire, pour qu'après un `cart_shutdown` aucune liste ne désigne la sous-zone remise à zéro.
16. En tant que mainteneur du budget, je veux que le rejeu écrive `chargement <n>` et `max_octets chargement <n>` sous `PICO8_REMPLISSAGE`, dans la forme des lignes `partie`, pour suivre la sous-zone sans code nouveau dans les familles suivantes.
17. En tant que mainteneur, je veux un TU qui prouve, sur `pause_restart` après le rendu de f13, que les octets de la génération courante valent exactement les états de 4b plus `Σ (2C − 4)` pointeurs, calculés à partir des capacités publiques des 16 registres, pour que la borne des blocs morts soit tenue par un test et pas seulement par une lecture.
18. En tant que mainteneur, je veux que le relevé f45 = f90 de `message_box_hold` reste égal avec les registres dans la zone, pour prouver qu'aucun registre ne grandit par tick.
19. En tant que développeur, je veux un TU qui joue jusqu'au titre, fait `cart_shutdown` puis `cart_load` puis `_init`, sous ASan, sans crash, et qui retrouve les places d'interface vivantes de départ, pour prouver le reset sans relecture et l'abandon avant le vidage.
20. En tant que développeur, je veux un TU qui prenne un bloc dans la sous-zone chargement, le relise à travers `eu` et l'update qui rend l'ancienne génération, puis meure par ASan en le relisant après `cart_load`, pour prouver que la sous-zone ne suit pas les générations.
21. En tant que gardien du cliquet, je veux que `pico8_entity.c` sorte de la liste et que `ui` et `planet_family` baissent d'un appel chacun, sans aucune hausse, pour que le verrou reste à égalité stricte.
22. En tant que gardien de l'invariant, je veux les 26 `*.sha256` inchangés et `make sanitize-net` vert, pour prouver que changer la zone des registres ne déplace ni un pixel ni un tirage `rnd`.
23. En tant que mainteneur du budget, je veux `M_4c` (maximum de `max_octets partie`) et le maximum de `max_octets chargement` relevés sur les 26 e2e, notés dans le commit et dans « Découvert », pour remplacer l'estimation de 4c par une mesure.
24. En tant que mainteneur du budget, je veux la projection refaite en deux comptes, génération (≤ 704 Ko) et chargement (≤ 64 Ko), avec la décision écrite (continuer, déplacer la frontière entre les deux, relever, arrêter et signaler), pour que l'arrêt anticipé du §3 tienne compte de la sous-zone.
25. En tant que développeur des familles suivantes, je veux l'inventaire du §4 refait sur le code de 4c, pour les deux durées de vie qui finissent (génération et chargement), pour qu'aucun registre ni aucune liste statique ne laisse de référence oubliée.
26. En tant que développeur de 4g, je veux une ligne « *Reçu de l'étape 4c* » qui dise que la sous-zone existe, avec sa poignée, sa taille et son remplissage, pour que `gb` et `hp.i` y entrent sans la recréer.
27. En tant que lecteur du plan, je veux une note « Découvert » sous l'entrée 4, qui dise la durée de vie retenue pour les registres et pourquoi, la sous-zone créée, le reset sans relecture, la borne des blocs morts, `M_4c`, la projection et les comptes du verrou, pour que 4d parte d'un plan à jour.
28. En tant que mainteneur des commentaires, je veux que les seuls commentaires touchés soient les contrats C devenus faux (ce que libère `pico8_entity_shutdown`, ce que vit une liste de registre, ce que rend la poignée de chargement), pour respecter les quatre destinations du `CLAUDE.md`.

## Implementation Decisions

### Inventaire des sites de 4c

| Site | Fonction, ligne à 1538d87 (identique à 9cf286e) | Aujourd'hui | Durée de vie (lecture) | Zone retenue |
|---|---|---|---|---|
| `g_hv` | `pico8_entity_init` (`entity` 78) | poignée transitoire | d'un `jl()` au suivant : `_init` ou `eu` | **génération de partie** |
| `g_cv[i].list` (5) | `pico8_entity_init` (`entity` 82) | idem | idem | **génération de partie** |
| `g_cg[i].list` (10) | `pico8_entity_init` (`entity` 86) | idem | idem | **génération de partie** |
| `g_shared_le` | `pico8_ui_init` (`ui` 145) | poignée transitoire | du chargement au chargement suivant | **sous-zone chargement** |
| `g_ga_by_shared` | `pico8_planet_family_init` (`planet_family` 531) | poignée transitoire | du chargement au chargement suivant | **sous-zone chargement** |

### Durée de vie des registres : la génération, pas le chargement

- **Lecture.** Chaque `jl()` refait les 16 listes, donc aucune n'a la durée de vie du chargement. Les tableaux faits par le `jl()` d'un `_init` sont relus jusqu'au `jl()` d'`eu` suivant. Ceux faits par le `jl()` d'`eu` sont relus jusqu'au `jl()` suivant (`_init` de fin de partie, ou `eu`). Dans les deux cas, ce sont des tableaux de la génération courante au moment de leur `jl()`. Après le `jl()` d'`eu`, plus rien ne lit les anciens :
  - la boucle d'abandon de `jl()` est finie avant `pico8_entity_init()` ;
  - la fin de `ok:lj` et la boucle de `dg` passent par la structure statique, donc par les nouveaux tableaux.
- **Règle du §4.** La génération *couvre* les tableaux : un tableau fait au titre (génération G1, sans bascule) est relu au plus jusqu'au `jl()` d'`eu`, et G1 n'est rendue qu'au vidage d'après. C'est la zone la plus longue qui les couvre et qui reste bornée.
  - *Alternative écartée : la sous-zone chargement*, qui vit plus longtemps. Elle n'est remise à zéro qu'au chargement. Chaque `jl()` y laisserait ses tableaux, et le joueur qui enchaîne les parties la remplirait sans borne. Le §4 demande « la plus longue qui le couvre », pas une zone qui ne rend jamais.
- **Ce qu'ASan tranche.** Si un parcours gardait un ancien tableau après le rendu, `sanitize-net` planterait sur `pause_restart` (rendu de f13). Ce serait un trou de l'inventaire, à couper au point d'oubli de partie et à noter dans « Découvert ».

### Tableaux croissants dans une zone qui ne rend rien : borne des blocs morts

- **Forme inchangée** (§2) : `pico8_ents_grow` double toujours, par `pico8_zone_grow`, et la poignée de partie copie. 4c ne touche pas `pico8_ents.c`.
- **Coût d'une liste**, d'un `jl()` à la fin de sa génération : capacités 4, 8, …, `C`, soit `4 + 8 + … + C = 2C − 4` pointeurs pris, dont `C − 4` morts. Donc **au plus le double de la taille finale**, comme 4a l'avait annoncé. `C` est la puissance de deux qui suit le plus grand nombre d'éléments tenus. Ce n'est ni le nombre d'ajouts ni le nombre d'actions : `pico8_ents_del` tasse, une liste ne grandit que quand elle dépasse son pic.
- **Par génération** : au plus deux `jl()`, celui d'`eu` et au plus un `_init` (4a : un seul `_init` peut précéder un `eu`). Les listes du titre sont petites (quelques entités).
- **Règle de 4a sur les sites par tick ou par action.** Un agrandissement n'est pas une allocation par tick : il n'arrive qu'à l'inscription d'un objet qui fait dépasser le pic, au plus `log2(C/4) + 1` fois par liste et par `jl()`. La règle est tenue au sens de 4a (« une allocation faite une fois par objet de partie »), amortie. Le relevé f45 = f90 le contrôle.
- **Pic par lecture** (x86-64, pointeur de 8 o) : environ 90 planètes, les singletons, `bg`, sept `ie`, les boutons de menu et de panneau, les slipways. `hv` tient de l'ordre de 150 à 250 entités (`C` = 256, soit 4 Ko pris). Les cinq `cv` et dix `cg` sont des sous-ensembles. Cela donne **~10 à 15 Ko par `jl()` de partie**, et **~20 à 30 Ko au pic de la bascule** (partie quittée, titre, nouvelle partie). C'est l'estimation de 4a (~20 Ko), avec une borne haute. La mesure tranche.
- *Alternatives écartées* :
  - capacité réservée à l'init, comme `qy` : il faudrait une limite par registre, alors que le §2 garde la forme des tableaux croissants ;
  - agrandir en place quand le bloc est le dernier du bout : un gain d'au plus 2×, au prix d'une primitive de plus ;
  - libérer les anciens blocs : la zone à deux bouts ne rend que par génération.

### Sous-zone chargement

- **Stockage à part**, pas un découpage du stockage de la zone à deux bouts. `pico8_zone_sizes.h` gagne `PICO8_ZONE_CHARGEMENT_BYTES` (64 Ko). La zone à deux bouts prend `PICO8_ZONE_GAME_BYTES − PICO8_ZONE_CHARGEMENT_BYTES` (704 Ko). Un contrôle à la compilation exige que la sous-zone soit strictement plus petite que la zone partie. Le contrôle de somme ne change pas : `PICO8_ZONE_GAME_BYTES` reste à 768 Ko, et la somme à 1487 Ko sur 1536 (`pico8_zone_sizes.h` à 1538d87).
  - *Pourquoi à part* : la zone à deux bouts suppose que ses deux bouts appartiennent aux générations. Y réserver un préfixe fixe toucherait `bascule_fin`, le calcul du plein et l'empoisonnement du rendu. Deux stockages ne coûtent pas un octet de plus au budget : « prise dans la zone partie » est une affaire de budget, pas d'adresse.
  - **Pourquoi ce partage respecte le §3.** Le §3 interdit de « rogner une autre zone pour faire de la place ». Les zones du §3 sont les quatre lignes du §2 (frame, interface, particules, partie), dont les tailles entrent dans le contrôle de somme. La sous-zone chargement n'est pas une cinquième zone : le §2 la range dans la ligne « Partie » (« dans une sous-zone chargement, prise dans la zone partie »). Partager les 768 Ko entre génération (704) et chargement (64) est donc un découpage **interne** à la zone partie :
    - aucune autre zone ne perd un octet ;
    - le plafond ne bouge pas ;
    - la somme contrôlée reste `frame + interface + particules + PICO8_ZONE_GAME_BYTES` = 1487 Ko ;
    - la marge de 49 Ko reste entière.

    Le contrôle à la compilation tient ce découpage interne. Génération + chargement = `PICO8_ZONE_GAME_BYTES` par construction, puisque la génération est définie comme la différence. La sous-zone doit être strictement plus petite que la zone partie. La même lecture vaut pour la décision « déplacer la frontière » du budget : c'est un chiffre changé à l'intérieur d'une zone. En revanche, faire passer `PICO8_ZONE_GAME_BYTES` au-dessus de 768 Ko reste un relèvement soumis au §3 : d'abord la marge de 49 Ko, puis l'arrêt.
- **Primitive** : une zone montante existante (`pico8_bump_zone_t`), initialisée par `pico8_bump_zone_init` : une moitié en build normal, deux sous ASan, comme la zone frame. Pas `pico8_bump_zone_init_single`, dont le reset est interdit par `assert`. Sa poignée est `pico8_zone_choisie_bump` : `alloc` monte, `grow` copie, `release` ne fait rien. Aucune primitive nouvelle.
- **Module** : dans `pico8_zone_partie.{h,c}`, puisque c'est une sous-zone de la zone partie et qu'elle a le même reset. Interface ajoutée (noms laissés à l'implémenteur) :
  - la poignée de la sous-zone ;
  - ses octets pris et leur maximum sur la session.

  `pico8_zone_partie_reset()` remet à zéro **les deux** : la zone à deux bouts et la sous-zone. Aucun nouvel appel dans `pico8_cart.c`. La bascule et le rendu ne touchent pas la sous-zone. Le maximum de la sous-zone survit au reset, comme celui de la zone partie. Son nom de zone pour `pico8_fatal` est « chargement ».
- **Initialisation paresseuse**, comme la zone partie : `pico8_ui_init` et `pico8_planet_family_init` peuvent prendre la poignée avant tout reset (chemin `jl()` sans `cart_load`).
- **Ordre au chargement** : `pico8_cart_load()` fait déjà vidage, `pico8_zone_partie_reset()`, puis `pico8_ui_init` et `pico8_planet_family_init`. Les deux listes statiques sont donc refaites après le reset qui a rendu leurs tableaux, sans changement d'ordre.
- **À l'arrêt** : `pico8_cart_shutdown()` remet la sous-zone à zéro avant `pico8_planet_family_shutdown` et `pico8_ui_shutdown`. Ces deux shutdowns **oublient** leur liste statique (`items` NULL, compte et capacité 0, même poignée), sans lire `items`. C'est une remise à vide, pas un rendu. En jeu, `items` est toujours NULL, et l'oubli n'a d'effet que pour le TU qui ajoute à `g_shared_le`.
- **Ce qui y grandirait** : rien en jeu (voir « Ce qui se passe aujourd'hui »). Si une famille ultérieure y fait grandir une liste après le chargement, la règle de 4a s'applique : capacité réservée au chargement, ou autre zone. 4c ne réserve rien, puisque rien ne grandit.
- **Taille** : 64 Ko au doigt mouillé (4a), pour 4g (~32 Ko estimés pour `gb` et `hp.i`). Mesurée à 0 en 4c (prévision). Elle n'est pas réduite : une limite se change en un chiffre quand une mesure l'exige.

### Reset sans relecture des registres

- **Le piège.** Après 4c, `pico8_zone_partie_reset()` empoisonne les tableaux de `hv`/`cv`/`cg`, alors que les structures statiques les désignent encore, avec leur compte. Le `jl()` suivant (un `_init` après `cart_load`) commence par parcourir `hv->items` : lecture de mémoire empoisonnée sous ASan, mémoire réutilisée sur PSX. Aujourd'hui, les tableaux sont pris à la libc et survivent au reset, donc rien ne plante. Le jeu (`main.c`) ne charge qu'une fois, `hv` vide : seuls les TU qui rechargent après avoir joué sont touchés.
- **Décision**, dans `pico8_cart_load()` et `pico8_cart_shutdown()` :
  1. **avant le vidage**, abandonner les entités de `hv` qui ont `rendre`, exactement comme la boucle de tête de `jl()`. La boucle est mise en commun, pas recopiée : `jl()` l'appelle, et les deux fonctions du cart aussi ;
  2. vidage, puis reset (ordre actuel inchangé) ;
  3. **après le reset**, vider les 16 registres sans lire leurs tableaux : `items` NULL, compte et capacité 0, `seen` à 0. La poignée, `mo` et l'état « initialisé » du module ne changent pas. Une fonction de `pico8_entity` ; son nom est laissé à l'implémenteur.
- **Pourquoi abandonner avant**. Si on se contentait d'oublier, les entités de `hv` (boutons, panneaux, émetteurs) ne rendraient jamais leurs places d'interface et de particules dans un processus de TU qui recharge. Aujourd'hui, le `jl()` suivant les rend. En les abandonnant avant le vidage du chargement, elles sont rendues un peu plus tôt, mais dans le même ordre que si `jl()` l'avait fait. Leurs structures sont encore prises à la libc ou aux zones à places, pas à la zone partie, donc les `rendre` lisent de la mémoire vivante.
- *Alternatives écartées* :
  - appeler `pico8_entity_shutdown()` après le reset : il remet `g_initialized` à 0, et `pico8_cv` l'`assert` ; un TU qui lit `cv` entre `cart_load` et `_init` casserait ;
  - appeler `pico8_entity_init()` : il remet `mo` au tenant-lieu, ce qui change ce que lit un TU après `cart_load` ;
  - abandonner après le reset : les `rendre` liraient `hv->items`, empoisonné.
- `pico8_entity_shutdown` et `pico8_entity_init` gardent leur forme. `pico8_ents_free` sur une poignée de partie ne lit que la valeur de `items`, pas la mémoire. Le contrat de `pico8_entity.h` (« Releases this module's own bookkeeping (hv/cv/cg's backing arrays) ») devient : il oublie les tableaux, rendus avec leur génération.

### Inventaire du §4 pour 4c

Deux durées de vie finissent désormais sur des objets de 4c :

- **la génération** : rendue au vidage qui suit le `eu` suivant, ou remise à zéro au chargement et à l'arrêt ;
- **le chargement** : la sous-zone, remise à zéro au chargement et à l'arrêt.

**Méthode.** Refaire **mécaniquement**, sur le code du commit de 4c, les deux `grep` de 4a sur les fichiers de `CORE_SRCS` (65 à 1538d87) :

- `grep -nE '^\s*static\s[^(]*\*\s*[a-zA-Z_][a-zA-Z0-9_]*(\[[^]]*\])*\s*(;|=)'` (hors `const char *` et tables `* const`) ;
- `grep -nE '^static\s+(struct\s+)?[a-z0-9_]+(_t)?\s+[a-zA-Z_][a-zA-Z0-9_]*\s*(\[[^]]*\])*\s*(;|=)'` (hors `const` et scalaires), dont on lit les champs pointeurs.

Ajouter :

- un `grep` des `pico8_ents_t` et `pico8_ents_t *` de portée fichier et des champs `pico8_ents_t *` : il trouve toute liste qui peut désigner un registre ou une liste statique ;
- un `grep` des affectations `= pico8_hv(`, `= pico8_cv(`, `= pico8_cg(`, `= pico8_ui_le_shared(` dans un champ ou un statique.

Comparer avec la table de 4a, avec l'inventaire de 4b (seuls `g_jo_oc`/`g_jo_ir` visent un état, repointés à chaque `eu` ; `g_bw_oc_r_ctx` est supprimé ; le point d'oubli est vide), et avec les statiques que 4c ajoute (stockage, zone et poignée de la sous-zone). Tout pointeur nouveau est classé dans « Découvert ».

**Relevé attendu** (fait à 9cf286e ; les fichiers concernés n'ont pas changé à 1538d87) :

| Pointeur | Peut viser | Statut |
|---|---|---|
| `g_hv.items`, `g_cv[].list.items`, `g_cg[].list.items` (`entity`) | un tableau de la génération de leur `jl()` | **repointés** à NULL puis regrandis par le `jl()` d'`eu`, après la bascule et avant le rendu ; **vidés sans lecture** après le reset du chargement et de l'arrêt. Entre `_init` et `eu`, ils désignent la génération courante, vivante |
| `pico8_ents_iter_t` (`it.ents`), `hv` local de `dg`/`jl` | la structure statique, pas `items` | ne peuvent pas viser un tableau rendu ; ce sont des locaux qui relisent `items` à chaque pas |
| `g_shared_le.items` (`ui`) | la sous-zone chargement | NULL en jeu ; **refait** par `pico8_ui_init` après le reset de `cart_load` ; **oublié** par `pico8_ui_shutdown` dans `cart_shutdown` |
| `ok.le` quand il vaut `&g_shared_le` | la structure statique | ne peut pas viser un tableau (adresse d'un statique) |
| `g_ga_by_shared.items` (`planet_family`) | la sous-zone chargement | NULL en jeu ; **refait** par `pico8_planet_family_init` après le reset de `cart_load` ; **oublié** par `pico8_planet_family_shutdown` |
| `ga.by` (`&g_ga_by_shared`) | la structure statique | ne peut pas viser un tableau ; l'éclat est en zone particules (étape 3) |
| stockage, zone et poignée de la sous-zone (`zone_partie`, ajoutés par 4c) | eux-mêmes | ne peuvent pas viser un objet (type) |
| `jo.by`, `jo.cc` (listes de planète) | — | hors 4c : famille 4d (`bd_reset_visit`, `pico8_jo_ib`) |

- **Le point d'oubli de partie (`oublier_partie()`) reste vide.** `jl()` couvre la nouvelle partie, et le reset sans relecture couvre le chargement. Un pointeur trouvé ensuite par ASan est un trou de cet inventaire. Il est coupé au point d'oubli (génération) ou au reset (chargement), et noté dans « Découvert ».
- **Angle mort ASan** (4a) : un tableau gardé à travers deux bascules ne planterait pas. Il ne peut pas exister ici, puisque chaque `eu` refait les 16 listes. La sous-zone a deux moitiés sous ASan : un bloc gardé à travers **deux** chargements ne planterait pas. Seuls les TU chargent deux fois.

### Remplissage écrit par le rejeu

- `write_remplissage` (`main.c`) ajoute, après les deux lignes `partie`, `chargement <octets pris>` puis `max_octets chargement <n>`. La capture, le dump et le code de sortie ne changent pas.
- Le cas `le_rejeu_ecrit_le_remplissage_en_fin_de_session` de `tests/test_pico8_places_vivantes.c` compare le fichier, chaîne entière, à une attente construite par les accesseurs. Il gagne ces deux lignes.

### Verrou

- `tools/alloc_guard_exceptions.txt` : la ligne `src/pico8_entity.c 3` est retirée ; `src/pico8_ui.c 6 → 5` ; `src/pico8_planet_family.c 6 → 5`.
- `pico8_entity.c` n'inclut plus `pico8_zone_transitoire.h` (ni `<stdlib.h>` s'il ne sert plus). `pico8_ui.c` et `pico8_planet_family.c` gardent la transitoire pour leurs autres sites (4h, 4d).
- **Prévision : 23 / 80 → 22 / 75**, à partir de la liste à 1538d87. Elle part de la mesure faite au `HEAD` de 4c avant conversion. Un écart se note dans « Découvert ». Aucun compte ne monte : 4c n'ajoute aucune référence à la transitoire ni à la libc.

### Budget : mesure, projection, décision

- **Mesure.** Après conversion, rejouer chacun des 26 e2e avec `tmp/pico8_replay` et `PICO8_REMPLISSAGE`, comme 4b, par une boucle de shell citée dans le commit (il n'y a pas de cible `make`). Relever :
  - `M_4c` = maximum des `max_octets partie` ;
  - `K_4c` = maximum des `max_octets chargement`. Prévision : 0.

  Noter les deux dans le commit et dans « Découvert », par e2e pour les trois plus gros. Un `K_4c` non nul est expliqué avant la projection : une liste statique qui grandit contredit la lecture.
- **Les deux comptes.** Depuis 4c, la zone partie est faite de deux réserves : génération 704 Ko, chargement 64 Ko. Les estimations de 4a pour les familles restantes, réparties :
  - **génération** : 4d ~160, 4e ~30, 4f ~1, 4g ~6 (`hy`, sept `ie` et leurs `jb`, au pic de bascule), 4h ~12, soit **209 Ko** ;
  - **chargement** : 4g ~32 Ko (`gb`, `hp.i`).
- **Projection**, règle de 4a appliquée à chaque compte :
  - `P_4c = M_4c + 209 Ko × r` et `Q_4c = K_4c + 32 Ko × r` ;
  - `r = max(1, M_4c / (2 + 20 Ko))`, rapport « mesuré / estimé » des familles faites (4b ~2 Ko, 4c ~20 Ko, estimations de pic de 4a). `M_4c` est cumulé : il contient déjà les 1216 o de 4b, mesurés sous leur estimation. 4b avait trouvé `r` = 1 et `P_4b` ≈ 262 Ko (note « Découvert (4b) »). Une mesure de 4c au-dessus de 20 Ko fait monter la projection des suivantes.
  - Exemple : `M_4c` = 30 Ko donne `r` ≈ 1,4 et `P_4c` ≈ 30 + 290 ≈ 320 Ko.
- **Décision**, dans l'ordre :
  - `P_4c ≤ 704 Ko` et `Q_4c ≤ 64 Ko` : on continue. C'est la prévision ;
  - `P_4c + Q_4c ≤ 768 Ko`, mais l'un des deux déborde : 4c **déplace la frontière** (`PICO8_ZONE_CHARGEMENT_BYTES`) dans son commit, et le dit. Ce n'est pas rogner une autre zone : les deux réserves sont la même zone du §2 ;
  - `768 < P_4c + Q_4c ≤ 768 + marge` (marge 49 Ko à 1538d87) : 4c relève `PICO8_ZONE_GAME_BYTES` d'un chiffre ;
  - au-delà : **arrêt de l'étape 4 à 4c**, et signalement à l'humain : zone partie, limite voulue `P_4c + Q_4c` arrondie, somme actuelle des zones (1487 Ko sur 1536 à 1538d87). On ne rogne ni la frame, ni l'interface, ni les particules.
- **Un `pico8_fatal` « zone partie pleine »** pendant 4c se lit d'abord comme un registre qui grandit par tick, puis comme une limite trop basse. La note « Découvert » dit lequel des deux. Dans un TU qui enchaîne des `jl()` sans recharger, la correction est un `cart_load` dans son aide, pas un relèvement : c'est le précédent du double `eu` de 4a.

### Note « Découvert » et clôture

À la clôture, sous l'entrée 4 de `docs/plan-4.md`, une note « *Découvert (4c) :* » d'une poignée de phrases. Elle dit :

- les registres sont en génération, et pourquoi pas en chargement ;
- la sous-zone chargement est créée, avec un stockage à part, 64 Ko pris sur 768, un reset commun, ses lignes de remplissage, `g_shared_le` et `g_ga_by_shared` dedans, et le constat que rien ne les agrandit ;
- le reset sans relecture (abandon avant le vidage, registres vidés après) ;
- la borne `2C − 4` et le TU qui la tient ;
- l'inventaire, avec le point d'oubli resté vide ;
- `M_4c`, `K_4c`, `P_4c`, `Q_4c`, la décision ;
- les comptes du verrou.

Sous l'entrée 4, une ligne « *Reçu de l'étape 4c :* » : la sous-zone chargement existe, avec sa poignée, ses 64 Ko, son reset par `pico8_zone_partie_reset()` et ses lignes de remplissage ; 4g y met `gb`, `hp.i` et les `jy`. La ligne « Partie » du §2 dit déjà la sous-zone : rien n'est à corriger, sauf si la mesure contredit la lecture. La PRD est retirée de `docs/prd/` dans le commit de clôture.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 : `make test-all`, `make sanitize-net`, 26 `*.sha256` identiques, `alloc-guard` à égalité stricte. Aucun ne passe la transitoire à un nouvel appelant.

1. **Sous-zone chargement et listes statiques.**
   - Les tests d'abord : TU de la sous-zone dans `test_pico8_zone_partie.c`, attente de deux lignes de plus dans le cas du remplissage.
   - `PICO8_ZONE_CHARGEMENT_BYTES` et son contrôle. La sous-zone, sa poignée, ses accesseurs, et son reset dans `pico8_zone_partie_reset()`.
   - `g_shared_le` et `g_ga_by_shared` sur la poignée, et l'oubli dans les deux shutdowns.
   - Les lignes de `write_remplissage`.
   - Le verrou : `ui` 5, `planet_family` 5.
2. **Registres en génération et reset sans relecture.**
   - Les tests d'abord : TU du rechargement après le titre (rouge sous ASan sans la décision), TU de la borne `2C − 4` après f13 de `pause_restart`, attentes reprises de `prise_and_max` et de `pause_restart_compte_les_etats_de_bw`.
   - La poignée de partie dans `pico8_entity_init`. L'abandon mis en commun et appelé avant le vidage de `cart_load`/`cart_shutdown`. Le vidage des registres après le reset.
   - Le contrat de `pico8_entity.h`.
   - Le verrou sans `entity`.
3. **Mesure et clôture.**
   - Le relevé sur les 26 e2e, `M_4c`, `K_4c`, la projection et la décision, dans le message du commit.
   - L'inventaire refait par `grep`.
   - La note « Découvert », la ligne « *Reçu de l'étape 4c* », et le retrait de cette PRD.

Le morceau 1 peut passer seul : il ne touche pas aux registres. Le morceau 2 dépend du 1 seulement pour la forme du remplissage.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - les octets pris et le maximum, par les accesseurs ou par les lignes écrites sous `PICO8_REMPLISSAGE` ;
  - les capacités et les comptes publics des listes (`pico8_ents_t` est un type public, rendu par `pico8_hv()`, `pico8_cv()`, `pico8_cg()`) ;
  - un rapport ASan dans un processus enfant, un message de `pico8_fatal` ;
  - les places vivantes de la zone interface ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne lit ni les bouts de la zone ni `items`, sauf pour le relire après son rendu. Les tailles attendues se calculent dans le TU (`sizeof`, cellule de zone), jamais en dur.
- **Octets d'un registre**, aide de TU : pour une liste de capacité `C` > 0, la somme des blocs pris par `pico8_ents_grow` depuis 0, soit les capacités 4, 8, …, `C`, chacune de `PICO8_ZONE_CELLS(k × sizeof(pico8_struct_header_t *))` cellules. Pour `C` = 0, rien. `R` est la somme sur les 16 registres. Un `cg` jamais vu rend NULL et compte 0.
- **`tests/test_pico8_places_vivantes.c`** (ASan, un enfant par cas, aides `demarrer` et `jouer_jusqu_a` de 4a, sans `relever`, dont le vidage rendrait la génération lui-même) :
  - **borne des blocs morts, sur de vrais registres** (cas nouveau) : après f13 de `pause_restart`, la génération courante ne contient que ce qu'a fait le `eu` de f12 et ce qui a grandi depuis. Donc `pico8_zone_partie_prise()` vaut exactement `G + R`, avec `G` = `etats_de_bw()` de 4b (608 o), seul autre contenu de la génération et `R` calculé sur les capacités lues après f13. Cette égalité prouve à la fois que les registres sont dans la génération, qu'ils ne coûtent que `2C − 4` pointeurs, et qu'aucun autre objet n'y est entré par erreur ;
  - **cas de 4b `pause_restart_compte_les_etats_de_bw`** : il attend exactement `g` après f11, `2g` après f12, `g` après f13, et un maximum de `2g`, avec `g` = `etats_de_bw()` (608 o). Ces égalités ne tiennent plus : les registres y ajoutent leur part, et celle de f11 compte des blocs morts qu'aucune capacité publique ne montre. Le cas est réécrit, et l'aide `etats_de_bw()` est réutilisée comme `G` :
    - après f11 : prise ≥ `g` ;
    - après f12 : prise ≥ prise de f11 + `g` ;
    - après f13 : `G + R`, comme dans le cas précédent ;
    - maximum ≥ prise de f12.
  - **`pause_restart_devoile_sur_les_etats_de_la_nouvelle_partie`** (4b) : inchangé ;
  - **sentinelle de 4a** : les trois cas inchangés (inégalités seulement) ;
  - **croissance par frame** : f45 = f90 sur `message_box_hold`, inchangé, avec désormais des registres dans la zone. Une hausse est un registre qui grandit sans action : on la corrige dans 4c, pas en relevant la limite ;
  - **remplissage écrit par le rejeu** : l'attente gagne `chargement <n>` et `max_octets chargement <n>`, lus aux accesseurs dans le même processus ;
  - **rechargement après le titre** (cas nouveau, enfant ASan) : `demarrer("pause_restart")`, jouer jusqu'après f11 (titre, après « restart »), relever les places vivantes de la zone interface, puis `pico8_cart_shutdown()`, `pico8_cart_load()`, `pico8__init()` et une frame. L'enfant ne meurt pas. Les places vivantes valent celles d'un titre neuf (`pause_restart_revient_au_titre` donne la référence). Sans l'abandon, elles resteraient prises ; sans le vidage des registres, le `jl()` de `_init` planterait sur `hv->items`.
- **`tests/test_pico8_zone_partie.c`** :
  - **sous-zone chargement** (cas nouveaux, en enfant relancé par nom comme les cas existants) :
    - un bloc pris après `cart_load` se relit à travers `pico8_eu` puis `pico8__update60()` (bascule puis rendu de l'ancienne génération) ;
    - il meurt par `use-after-poison` après `pico8_cart_load()`, même après de nouvelles allocations dans la sous-zone (critère de 0a) ;
    - prise et maximum : 64 après un bloc, inchangés par `eu` et le rendu, 0 après `cart_shutdown`, le maximum restant ;
    - la sous-zone pleine sort en 3 avec « zone chargement pleine, limite 65536 octets » ;
  - **`prise_and_max`** : ses égalités exactes à 1538d87 (0 après `_init`, 64, `128 + g`, `64 + g`, maximum `128 + g`, avec sa propre aide `etats_de_bw()`) ne tiennent plus, puisque le `jl()` d'`_init` et celui d'`eu` prennent maintenant des registres. Elles sont réécrites en écarts : chaque bloc compté ajoute 64 ; après l'update qui rend, la prise vaut `G + R + 64`, avec `R` lu après l'update ; le maximum garde le pic d'avant le rendu ; après `cart_shutdown`, 0, maximum inchangé ;
  - **zone pleine** : si un cas existant attend le message de la zone partie avec sa limite, la limite devient `PICO8_ZONE_GAME_BYTES − PICO8_ZONE_CHARGEMENT_BYTES`, lue des macros ;
  - les autres cas restent inchangés.
- **`tests/test_pico8_ui.c`** (cas qui ajoute à `pico8_ui_le_shared()`) et **`tests/test_pico8_je.c`** (cycles `cart_shutdown`/`cart_load`/`jl()`) : verts sans changement d'attente. S'ils échouent, c'est une relecture après reset, à corriger dans 4c.
- **Tests existants** : verts, sans autre changement d'attente. Seule adaptation permise : une aide de TU qui enchaîne des `jl()` ou des `pico8_entity_init()` au point de remplir la zone partie gagne un `cart_load`.
- **Verrou** : `make alloc-guard` à égalité stricte, liste sans `entity`. Pas de nouveau cas dans `tests/test_alloc_guard.c`.
- **Invariant** : `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net`. Sur `pause_restart`, la génération rendue à f13 contient désormais les registres de la première partie et du titre.
- **Pas de TU propre à `pico8_ents`** : sa forme ne change pas, et ses TU existants restent verts.
- **Exemples dans le dépôt** :
  - `tests/test_pico8_places_vivantes.c` : rejeu par enfant, `demarrer`/`jouer_jusqu_a`, sentinelle de 4a, cas de 4b (`etats_de_bw()`), remplissage écrit ;
  - `tests/test_pico8_zone_partie.c` : cas en enfant relancé par nom, `use-after-poison` attendu ;
  - `tests/test_pico8_zone_frame.c` : zone montante à deux moitiés sous ASan ;
  - `tests/test_pico8_zone_particules.c` : zone pleine en 3.

## Out of Scope

- Les listes des planètes (`jo.by`, `jo.cc`, `ib`, `lp`, `rl`) et `bd_reset_visit` : famille 4d.
- Les calques `ic` de `ok` (`ui` 129, 295), la structure `ok`, `ok.qa` : famille 4h. `ui` reste dans la liste à 5.
- Les structures de planète (`planet_family` 62, 123, 167, 247, 316) : famille 4d. `planet_family` reste dans la liste à 5.
- `gb`, `hp.i`, `jy` : famille 4g, dans la sous-zone que 4c crée.
- Toute modification de la zone à deux bouts, de sa bascule, de son rendu ou de son point d'oubli : 4a.
- Réécrire la forme des tableaux croissants (`pico8_ents`, `pico8_ptrs`), ou agrandir en place.
- Relever `PICO8_ZONE_GAME_BYTES`, sauf si la projection l'exige.
- Retirer TROU-7 : étape 5.
- Lancer l'application raylib.

## Vérifié contre 4b

Relu à `HEAD` 1538d87, sans toucher git :

- la note « *Découvert (4b)* » sous l'entrée 4 de `docs/plan-4.md` ;
- `git show f9e4f8b --stat`, puis le code : `src/pico8_gen.{h,c}`, `src/pico8_galaxy.{h,c}`, `src/pico8_planet.{h,c}`, `src/pico8_bg.c`, `tests/test_pico8_places_vivantes.c`, `tests/test_pico8_zone_partie.c`, `tests/test_pico8_map_tables.c` ;
- `git diff 9cf286e 1538d87 --stat` : aucun fichier de 4c n'est touché ;
- `tools/alloc_guard_exceptions.txt` (23 fichiers / 80 appels) et `src/pico8_zone_sizes.h`.

Les hypothèses du brouillon, une par une :

- **H-4b-1 (verrou) — tenue.** `pico8_gen.c` est sorti de la liste : 23 fichiers, 80 appels. `entity` 3, `ui` 6 et `planet_family` 6 sont inchangés. La prévision de 4c part de là : 22 / 75.
- **H-4b-2 (budget) — tenue.**
  - `M_4b` = 1216 o au clic de `pause_restart` (deux générations), et 608 o sur les 25 autres e2e.
  - La projection donne ≈ 262 Ko sous 768 ; la décision est « on continue ».
  - `PICO8_ZONE_GAME_BYTES` vaut toujours 768 Ko, la somme 1487 Ko sur 1536, la marge 49 Ko. La règle de projection et les estimations de 4a n'ont pas changé.
- **H-4b-3 (fichiers de 4c intacts) — tenue.**
  - f9e4f8b touche `bg`, `galaxy`, `gen`, `planet` et trois TU.
  - Il ne touche ni `pico8_entity.c`, ni `pico8_bootstrap.c`, ni `pico8_ui.c`, ni `pico8_planet_family.c`, ni `pico8_ents.c`, ni le module de zone partie, ni `pico8_cart.c`, ni `main.c`.
  - Les lignes 78, 82, 86, 145 et 531 tiennent.
- **H-4b-4 (TU de 4b) — tenue, précisée.**
  - Le cas s'appelle `pause_restart_compte_les_etats_de_bw`. Il attend exactement `g` / `2g` / `g` après f11, f12 et f13, et un maximum de `2g`.
  - `prise_and_max` attend exactement 0, 64, `128 + g`, `64 + g` et un maximum de `128 + g`, puis 0 après `cart_shutdown`.
  - Chaque TU calcule `g` par une aide `etats_de_bw()`, que 4c réutilise.
  - Le cas `pause_restart_devoile_sur_les_etats_de_la_nouvelle_partie` (`jo:hh` après f13) lit `pico8_cg("jo")` par sa structure statique : il reste vert.
  - Testing Decisions nomme désormais ces cas.
- **H-4b-5 (inventaire) — tenue.**
  - Seuls `g_jo_oc`/`g_jo_ir` visent un état, et ils sont repointés à chaque `eu`.
  - `g_bw_oc_r_ctx` est supprimé et `oublier_partie()` reste vide.
  - La note de 4b relève un écart, sans effet sur 4c : le libérateur était `jo_set_oc`/`_ir`/`_shutdown`, et non `jo:pq` comme le disait la table de 4a.
- **H-4b-6 (contenu de la génération) — tenue.** Hors du module, seul `pico8_galaxy.c` appelle `pico8_zone_partie()`, pour les trois états de `bw`. L'égalité `G + R` après f13 est donc exacte.
- **H-4b-7 (pas de sous-zone) — tenue.** Il n'y a ni sous-zone ni constante nouvelle dans `pico8_zone_sizes.h`.
- **H-4b-8 (outil de mesure) — tenue sur le fond, tombée sur la forme.** Il n'y a pas de cible `make` : la mesure par e2e se fait en rejouant `tmp/pico8_replay` avec `PICO8_REMPLISSAGE`, par une boucle de shell citée dans le commit. 4c fait de même et lit aussi `max_octets chargement`.
- **H-4b-9 (note « Découvert (4b) ») — tenue.** Elle existe, et aucune ligne « *Reçu de l'étape 4b* » n'a été ajoutée.

**Ce que la vérification change dans cette PRD** :

- les renvois aux hypothèses sont remplacés par les faits ;
- Testing Decisions nomme les cas de 4b et réutilise `etats_de_bw()` ;
- la mesure passe par `tmp/pico8_replay` ;
- le budget cite `M_4b` et `P_4b` ;
- le texte de l'étape est recopié à 1538d87, avec la note de 4b.

Aucune décision de 4c ne change.

## Further Notes

- **Pas d'humain disponible, et pas d'outil `Agent` dans cette session pour lancer un agent `guideline` neuf.** J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage) :
  - **Hypothèses explicites** : ce qui dépendait de 4b était posé en H-4b-1 à H-4b-9. Toutes ont été vérifiées à 1538d87 (« Vérifié contre 4b ») : huit tiennent, et H-4b-8 tombe sur la forme (pas de cible `make`, mais une boucle sur `tmp/pico8_replay`). Le reste a été lu à 9cf286e, et ces fichiers n'ont pas changé.
    - La durée de vie des registres a été tranchée par lecture : `jl()` les refait, `pico8_entity_init` n'a pas d'autre appelant dans le cœur.
    - La crainte de 4a sur `g_shared_le` a été vérifiée : rien ne l'agrandit.
    - Le piège du reset est relevé, pas supposé : `jl()` relit `hv->items`.
  - **Simplicité** :
    - aucune primitive nouvelle (zone montante et poignée existantes) ;
    - un stockage à part plutôt qu'un découpage de la zone à deux bouts ;
    - un reset commun, donc aucun appel de plus dans `pico8_cart.c` pour la sous-zone ;
    - `pico8_ents` intact ;
    - pas de réserve de capacité pour des listes qui ne grandissent pas.
  - **Changements chirurgicaux** : 5 sites, plus le reset sans relecture, que la conversion rend nécessaire. Le reste de `ui` et de `planet_family` attend 4h et 4d.
  - **Critères vérifiables** :
    - 22 / 75 au verrou ;
    - `G + R` exact après f13 de `pause_restart` ;
    - f45 = f90 sur `message_box_hold` ;
    - `K_4c` = 0 ;
    - le TU de rechargement sans crash ;
    - 26 `*.sha256` inchangés et `sanitize-net` vert ;
    - `P_4c` ≤ 704 Ko et `Q_4c` ≤ 64 Ko, le découpage restant interne à la zone partie (§3).
- **Ce que 4c éprouve pour la suite.** C'est la première famille dont les objets grandissent dans une génération. Si la borne `2C − 4` tenait mal, par exemple parce qu'une liste est refaite plus souvent qu'à chaque `jl()`, l'égalité `G + R` le montrerait avant que 4d n'y mette 90 planètes.
- **Risque accepté.** Les registres grandissent avec le nombre d'entités vivantes, que le jeu ne borne pas pour les slipways (§7 : pas de budget de slipways). Une très longue partie peut donc approcher la limite. Le §6 accepte de le découvrir par un `pico8_fatal` qui nomme la zone.
- **Rendu un peu plus tôt dans les TU.** L'abandon avant le vidage de `cart_load`/`cart_shutdown` rend les places des entités au chargement, au lieu du `jl()` suivant. Seuls les TU qui rechargent après avoir joué le voient. Le jeu ne charge qu'une fois, `hv` vide.
