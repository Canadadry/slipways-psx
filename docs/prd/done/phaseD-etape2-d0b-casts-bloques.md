---
title: "Plan 2, Phase D0b : les 17 casts bloqués — atteints, classés, revus comme clés, et convertis quand ils sont (b)"
description: "17 casts (int)pico8_fixed_to_double portent encore « à auditer » : ils n'appartiennent à aucune étape, si bien que le périmètre de D1 n'est pas fixé et que la Phase E ne peut pas fermer. Chacun doit être atteint (fixture du filet, test au niveau valeur ou entrée « non protégé par le filet »), puis classé selon les règles de C0, et ses 11 clés de table revues."
status: needs-triage
---

## Problem Statement

La Phase C est close et l'outillage de D (D0a) est en place. D'après `tools/float_inventory.txt`, il reste **17 casts `(int)pico8_fixed_to_double(…)` en « à auditer »** (`compte etape a auditer 17`). Aucune étape ne les possède (table de propriété, ligne « casts « à auditer » — bloqués »). Tant qu'ils ne sont pas classés :

- **le périmètre de D1 n'est pas fixé.** D1 doit savoir, avant de commencer, combien de sites (c) il fera diverger. Le plan exige que ce périmètre soit fixé *avant* D1 ;
- **la Phase E ne peut pas fermer.** D1 est la dernière étape qui convertit des casts, et un cast non classé n'a pas d'étape pour le convertir ;
- **la revue des clés fractionnaires (§3.2) reste ouverte pour 11 sites.** En Lua, `t[1.5]` est une clé distincte de `t[1]`. Le C convertit la clé en `int` *avant* la recherche. Le plan exige que « toujours entier par construction » soit établi site par site, pas supposé (§10, décision 14).

Il y a deux raisons au blocage. La première touche 16 des 17 : ils sont **NON COUVERT** par le filet (`tools/float_coverage.txt`, `tools/float_observe.txt` : 0 passage). La règle de B2 interdit alors tout classement tant qu'ils ne sont pas atteints par une fixture, par un test au niveau valeur ou par une entrée explicite « non protégé par le filet ». La seconde touche `pico8_pj_hk` `yr` : il est couvert (8 passages), mais sa preuve de signe est tombée avec la prémisse du non-débordement de `qc.t`. `yr` part de 3401 (lua.lua:1202), prend +1 par année (lua.lua:1213) et n'a pas de borne établie.

Plusieurs justifications portent un candidat. Aucun candidat ne classe.

## Solution

Les 17 sont traités par groupes de sites qui partagent un moyen d'atteinte. Chaque groupe suit le même ordre, pour qu'une porte ne soit jamais mesurée contre une référence posée après la modification qu'elle doit juger :

1. **Atteindre.** Une fixture d'événements scriptée (JSON écrit à la main ou par script, rejouée sans fenêtre) mène le jeu jusqu'au site. Elle **entre dans `FRAMES_EVENTS`**, et sa référence `frames.hash` est capturée **sur le code actuel, avant toute conversion**, dans un commit qui ne touche aucun site. Un test de scénario accompagne la fixture : il asserte l'état de jeu que la fixture doit atteindre (écran tech ouvert, `hl == "final score"`…), pour qu'une fixture qui cesse d'atteindre son site échoue au lieu de rester verte en silence.
2. **Si aucune fixture ne peut l'atteindre**, on établit pourquoi, en citant des lignes de `lua.lua` (par exemple une clé toujours chaîne dans les données du cart). Le site reçoit alors une entrée explicite **« non protégé par le filet »** dans `docs/plan-2.md`, avec cette source.
3. **Classer** avec les règles de C0, sans en ajouter : (a) cite une ligne de `lua.lua` qui contient `flr(` ; (b) exige une preuve de signe ≥ 0 ; sinon (c). L'observation reste un indice. L'annotation change dans le commit qui classe.
4. **Revoir la clé** des 11 sites clé de table : entière par construction (source citée), ou fractionnaire possible (écart consigné). Pour un site converti ici, la revue s'appuie sur le compte d'arguments fractionnaires de l'audit de D0a.
5. **Convertir** ce qui est (b), ou (a) à signe prouvé, **et atteint par le filet**. La méthode est celle de C1 : `pico8_fixed_flr_int`, ligne d'annotation retirée, entrée ajoutée à `tools/float_audit_c1.txt`, dans le même commit. Portes : `frames-check` identique sans `frames-baseline`, `float-audit` vert. Un cast (c), ou (a) à signe non prouvé, reçoit l'étape D1 dans l'annotation et **n'est pas converti ici**.

Sortie : `compte etape a auditer 0` et `compte population a auditer 0` dans l'inventaire. `compte etape D1` vaut alors 22 plus les reclassés en D1, et `compte etape C1` revient à 0 après les conversions.

## User Stories

1. En tant qu'agent qui mènera D1, je veux qu'aucun cast ne reste « à auditer », pour que mon périmètre soit fixé avant que je commence.
2. En tant qu'agent qui mènera D1, je veux que chaque cast reclassé (c) porte l'étape D1 et une justification qui dit pourquoi, pour convertir une liste finie et justifiée.
3. En tant qu'agent, je veux que chaque site NON COUVERT soit atteint par une fixture du filet avant d'être classé quand c'est possible, pour que la règle de B2 soit respectée et que la porte de conversion ait quelque chose à mesurer.
4. En tant qu'agent, je veux que chaque nouvelle fixture entre dans `FRAMES_EVENTS`, pour que `frames-check`, `float-audit`, `float-coverage` et `float-observe` la rejouent sans configuration de plus.
5. En tant qu'agent, je veux que la référence `frames.hash` d'une nouvelle fixture soit capturée sur le code d'avant toute conversion, dans un commit qui ne touche aucun site, pour qu'une conversion fautive se voie comme une divergence et ne soit pas gelée dans la référence.
6. En tant qu'agent, je veux qu'un test de scénario asserte l'état atteint par chaque nouvelle fixture, pour qu'une fixture qui dérive cesse de passer pour une couverture.
7. En tant qu'agent, je veux que les fixtures soient des JSON d'événements scriptés, rejoués sans fenêtre, pour ne jamais lancer l'application raylib (CLAUDE.md).
8. En tant qu'agent, je veux que `float-coverage` et `float-observe` soient régénérés après l'ajout des fixtures, pour lire dans les rapports versionnés que le site est désormais atteint, et son minimum brut.
9. En tant qu'agent, je veux qu'un site qu'aucune fixture ne peut atteindre reçoive une entrée « non protégé par le filet » dans le plan, avec la ligne de `lua.lua` qui l'établit, pour qu'« inatteignable » soit sourcé et non constaté faute d'avoir cherché.
10. En tant qu'agent, je veux qu'un cast classé (a) cite une ligne de `lua.lua` qui contient `flr(`, vérifiée par l'inventaire, pour que la population (a) reste sourcée par construction.
11. En tant qu'agent, je veux qu'un cast classé (b) porte une preuve de signe en une ligne et soit atteint par le filet, pour que la preuve devienne une vérification exécutée par `float-audit`, comme en C1.
12. En tant qu'agent, je veux qu'un cast sans preuve de signe soit classé (c), même s'il est entier par construction, pour ne pas inventer une quatrième population (§11.5, règle « valeur entière par construction » de C0).
13. En tant qu'agent, je veux que `pico8_pj_hk` `yr` reçoive une décision sourcée : (b) seulement si une ligne de `lua.lua` borne `yr` en fin de partie, sinon (c) avec la raison, pour que la prémisse tombée ne soit pas remplacée par une supposition.
14. En tant qu'agent, je veux que les 11 sites clé de table portent chacun un verdict de revue de clé (« entière par construction : <source> » ou « fractionnaire possible : <source> »), pour que la décision 14 du §11 soit établie site par site.
15. En tant qu'agent, je veux que la revue d'une clé convertie ici cite le compte d'arguments fractionnaires de `float-audit`, pour appuyer la revue sur une mesure en plus de la source.
16. En tant qu'agent, je veux qu'une clé qui peut être fractionnaire soit consignée comme écart (trou de portage), et ne soit pas corrigée ici, pour garder D0b dans le volet flottant.
17. En tant qu'agent, je veux que chaque cast (b), ou (a) à signe prouvé, et atteint par le filet soit converti en `pico8_fixed_flr_int`, pour sortir son jeton flottant de l'inventaire.
18. En tant que mainteneur, je veux que chaque cast converti rejoigne `tools/float_audit_c1.txt` dans le commit qui le convertit, pour qu'il reçoive la garde de C1 (aucun négatif, ancien == nouveau, atteint) à chaque `make test`.
19. En tant que mainteneur, je veux que `frames-check` soit identique sur tout `FRAMES_EVENTS`, nouvelles fixtures comprises, et sans `frames-baseline`, à chaque commit de conversion, pour que D0b reste strictement préservateur.
20. En tant que mainteneur, je veux que `float-audit` soit vert après chaque conversion, avec le nouveau nombre de sites C1 tous atteints, pour que la porte de C1 couvre les nouveaux venus.
21. En tant que mainteneur, je veux qu'un cast dont la porte échoue repasse par l'annotation, en (c)/D1, au lieu d'être converti, pour qu'une preuve réfutée par la mesure ne soit jamais forcée.
22. En tant qu'agent, je veux que l'inventaire affiche `compte etape a auditer 0` à la sortie, pour énoncer la sortie de D0b en « compte = 0 » (règle de B2).
23. En tant que mainteneur, je veux que la table de propriété et un « État de D0b » dans `docs/plan-2.md` consignent le sort de chacun des 17, les nouvelles fixtures, les entrées « non protégé » et les comptes mesurés, pour que D1 parte d'un état écrit.
24. En tant que mainteneur, je veux que le coût ajouté à `make test` par les nouvelles fixtures soit mesuré et consigné, pour que la suite ne s'alourdisse pas sans que personne le voie.
25. En tant qu'agent, je veux qu'aucune constante, ni `tostr`, ni `pow` ne change, pour ne pas mêler D0b à C2b, D2 ou D3.
26. En tant qu'agent, je veux que les branches de dispatch par type (clé nombre / clé chaîne) restent en place même quand la branche nombre est inatteignable, pour ne changer que ce que l'étape exige.
27. En tant qu'humain, je veux que D0b ne lance jamais l'application raylib, pour que tout se vérifie sans fenêtre.

## Implementation Decisions

- **Les 17 sites et leur moyen d'atteinte probable.** Ce sont des pistes à vérifier, pas des classements. La colonne « candidat » reprend l'annotation. Aucun candidat ne classe.

  | Groupe | Site(s) | Lua | Atteinte probable | Candidat |
  |---|---|---|---|---|
  | G1 — écran tech | `pico8_cr` `tl` ×2, `pico8_hy_kt` `ew` | lua.lua:955, 964, 1450 ; `ew=1` lua.lua:1200 | bouton `ix[2]` (`ra=v(0,0)`) ouvre `cr()` ; un clic sur une tech abordable du menu appelle `fm:kt(t)` (lua.lua:957) | (b) : `ew` vaut 1, puis +1 tant que < 4. Clés de table `ek[…]` |
  | G2 — panneau de score et fin de partie | `pico8_pj_ph_map` `score_val`, `s6_val` ; `pico8_pj_hk` `yr` | lua.lua:884, 888, 859 ; `s[6]=min(flr(total/2000),5)` lua.lua:1361 ; fin à `yr>=3426` lua.lua:1235 | bouton `ix[3]` (`ra=v(47,0)`) bascule `ju.c_` ; bouton `ix[1]` (`ra=v(118,0)`) fait `fm:hk()`, une année par clic, donc la fin de partie en environ 25 clics | `score` : littéral 0..6 → (b). `s6` : (a) via lua.lua:1361, signe à prouver (`total` peut-il être négatif ?). `yr` : voir plus bas |
  | G3 — menu pause | `pico8_ha2_my_dispatch` `s` | lua.lua:993 | bouton `ix[4]` (`ra=v(0,117)`), puis l'entrée « toggle music » | `-sgn(…)` vaut -1, 0 ou 1 : entier, mais négatif → (c) par la règle de C0 |
  | G4 — éclat de particules | `pico8_ga_kj` ×5 | lua.lua:1091 | un `ga` naît de `pm:pq()` (protostar, lua.lua:1030), c'est-à-dire d'une planète résolue en `pm` par `jo:hu` via `hp.cw` | `p.x`, `p.y` : coordonnées écran → (c). `flr(pt.l/2)` : (a) lua.lua:1091. Couleur `k_[…]` : littéraux de palette. Rayon `pt.p:qx()` : `sqrt` |
  | G5 — dispatch par type de clé | `pico8_ho_apply`, `set_by_value` (`economy_tiers`), `pico8_jc_display_get_by_value`, `pico8_research_lookup`, `pico8_jo_hu` | lua.lua:17-19, 1271, 657, 1422-1423, 456 | branche « clé nombre » d'une fonction C qui choisit selon le type de la clé. Plusieurs sites voisins sont pourtant exécutés par le filet (slipways de `slipway_drag`, `ho` à l'init de chaque rejeu) : la branche nombre n'y est probablement jamais prise, car les ids de planète sont des chaînes `"p"..id` (lua.lua:749) | à établir : branche atteignable par une fixture, ou inatteignable avec les données du cart |

- **Une fixture ajoutée entre dans `FRAMES_EVENTS`.** C'est le seul moyen pour que `float-audit` (et la porte C1 d'un site converti), `frames-check` et la couverture la voient. Un site atteint seulement par un test unitaire ne peut pas rejoindre la liste C1 : la classe C1 échoue sur NON ATTEINT. La liste de `FRAMES_EVENTS` reste explicite dans le `Makefile`. On y ajoute chaque fixture, avec une ligne de lecture si le nom ne dit pas ce qu'elle atteint. Nombre de fixtures : le plus petit qui atteint les groupes, une par groupe au plus (G2 peut en valoir une seule). On les garde courtes, parce que chacune alourdit `frames-check` et `float-audit` dans `make test`.

- **Ordre dans chaque groupe, imposé par le corollaire du §5 :** (1) un commit qui ajoute fixture, test de scénario, entrée `FRAMES_EVENTS` et référence `frames.hash` produite par `make frames-baseline`. Aucune référence existante ne diverge alors, donc la cible copie sans demander `ETAPE=`. Si une référence existante divergeait, c'est un bug et on s'arrête. (2) Rapports `float-coverage` / `float-observe` régénérés. (3) Commit de classement (annotation). (4) Commit de conversion des (b), ou (a) à signe prouvé. Jamais de fixture et de conversion dans le même commit.

- **Construction des fixtures.** Même format que les fixtures de jeu existantes (`frames` : `mouse_x`, `mouse_y`, `mouse_buttons`, `dpad`). On les écrit à la main ou avec un petit script (non versionné, ou versionné sous `tools/` s'il sert à régénérer), à partir des positions de boutons de `ix`/`ha` dans `lua.lua` et de la géométrie lue dans le code. On les vérifie seulement par rejeu sans fenêtre et par le test de scénario. Pour trouver un clic, on peut lire le dump texte d'une frame (`pico8_frames frame`). Aucune capture humaine, aucun enregistrement raylib.

- **Test de scénario par fixture.** Sur le modèle de `tests/test_pico8_top_menubar.c` et `tests/test_pico8_slipway_drag.c`, il rejoue la fixture sans fenêtre et asserte un fait d'état de jeu, pas un pixel : champ `ew` augmenté, `fm.hl == "final score"`, `ju.c_` vrai, un `ga` présent dans `cg`… On n'ajoute pas de compteur dans le cœur pour savoir si le site est atteint : la couverture régénérée l'établit.

- **Inatteignable : « non protégé par le filet », puis (c).** Pour un site de G5 dont la branche n'est jamais prise avec les données du cart, la justification cite les lignes qui l'établissent (origine des clés : ids en chaîne, contenu des tables décodées…). Le plan reçoit l'entrée « non protégé par le filet ». Le cast est classé **(c)/D1**, parce qu'une preuve de signe sur une valeur qui n'arrive jamais n'est pas une preuve de signe exécutable, et que la règle de C0 dit « sinon (c) ». En D1, `flr` y laisse toute clé entière inchangée. Le site tombera sous « correction non vue par le filet » de D1, qui décidera entre test au niveau valeur et entrée du plan. Écartés :
  - un (b) « vide » ajouté à la liste C1 : la porte C1 échouerait sur NON ATTEINT ;
  - un marqueur « inatteignable » dans l'audit : c'est de l'outillage que l'étape n'exige pas ;
  - la suppression de la branche morte : changement non demandé, et la branche est un contrat de fonction.

- **Classement.** Il suit mot pour mot les règles de C0 et le format de `tools/float_annotation.txt` : clé (fichier, fonction, extrait, occurrence), population, étape, justification d'une ligne. (a) cite `lua.lua:<ligne>` avec `flr(`, et l'inventaire le vérifie. (a) → C1 seulement si la justification prouve aussi le signe ≥ 0, sinon D1. (b) → C1 et exige d'être atteint par le filet. (c) → D1. L'observation est citée en complément, jamais à la place d'une preuve. Une valeur entière par construction mais sans preuve de signe reste (c) : c'est le cas de `ha2` s.

- **Décision `yr`, sourcée.** `yr` est entier par construction (littéral 3401, lua.lua:1202 ; `+=1`, lua.lua:1213), donc la question des clés fractionnaires ne se pose pas pour `gs[3426-yr]`. Le signe se décide en cherchant dans `lua.lua` une ligne qui empêche `hy:hk()` après `hl` (fin de partie, lua.lua:1235-1236) : un bouton `ix[1]` retiré par le `fq()` de `pj:ph` (lua.lua:870-871), des actions de planète désactivées, etc. Si une telle ligne borne `yr` (par exemple ≤ 3426 + k), la justification la cite : (b), converti et ajouté à la liste C1. Sinon, (c)/D1, avec la raison : « `+=1` sans borne, repli à 32767 non exclu ». La fixture G2 peut montrer si `yr` continue après la fin, mais c'est un indice, pas une preuve.

- **Revue des 11 clés de table** (`ho_apply`, `hy_kt`, `set_by_value`, `cr` ×2, `jo_hu`, `ga_kj` `flr(pt.l/2)`, `research_lookup`, `pj_hk`, `pj_ph_map` `score_val`, `jc_display_get_by_value`).
  - **Verdict par site**, dans la justification de l'annotation (la ligne reste d'une ligne) et dans l'« État de D0b » : « clé entière par construction : <source> » ou « clé fractionnaire possible : <source> ».
  - **Mesure** : pour un site converti ici, le compte « fractionnaires » de `make float-audit` sur tout le filet, cité à côté de la source. Pour un site qui part en D1, l'audit ne le voit pas encore (il n'appelle pas `pico8_fixed_flr_int`). Sa revue s'appuie sur la source et sur le minimum de `float-observe` (partie basse non nulle), et D1 la confirmera avec le compte de l'audit.
  - **Clé fractionnaire possible** : l'écart (Lua renvoie `nil`, le C trouve une entrée) est consigné dans `docs/trous-de-portage.md`, sous un nouveau TROU référencé par un `/* TROU-n */` nu au site. Il n'est **pas** corrigé ici.
  - La ligne D4 de la table de propriété n'est pas touchée. D4 reste réduite aux clés déjà converties avant D0b (16 C1, 1 D1). Les clés converties par D0b ont leur revue faite ici.

- **Conversion** (méthode de C1, inchangée). `(int)pico8_fixed_to_double(<arg>)` devient `pico8_fixed_flr_int(<arg>)` ; ni l'argument ni le code autour ne changent. Dans le même commit : la ligne d'annotation est retirée, une ligne est ajoutée à `tools/float_audit_c1.txt` (fichier | fonction | extrait | occurrence, extrait = l'argument de `pico8_fixed_flr_int(`), et `tools/float_inventory.txt` est régénéré. Portes : `make test` vert, dont `frames-check` identique sur tout `FRAMES_EVENTS` sans `frames-baseline` et `float-audit` vert (sites C1 = 37 + convertis, tous atteints, 0 écart, 0 négatif). Si une porte échoue, le cast repasse en (c)/D1 dans l'annotation, sans être converti ni re-baseliné.

- **Invariants.** Aucune constante, ni `pico8_value_tostr`, ni `pico8_dj` ne change. Les casts D1 existants ne sont pas convertis. `constants-check` et `float-inventory-check` restent verts. Les rapports hors suite (`float-coverage`, `float-observe`, `float-guard`) sont régénérés et versionnés en fin d'étape.

- **Consignation dans `docs/plan-2.md`.** On ajoute un « État de D0b » après l'« État de D0a », qui consigne :
  - le sort de chacun des 17 : population, étape, moyen d'atteinte, verdict de clé ;
  - les fixtures ajoutées et leur nombre de frames ;
  - les entrées « non protégé par le filet » ;
  - les comptes de l'inventaire (`a auditer 0`, C1, D1) ;
  - la sortie de `float-audit` (sites C1, passages, fractionnaires aux sites convertis) ;
  - la garde avant/après ;
  - l'observation (0 contradiction) ;
  - la durée de `make test` avant/après.

  La table de propriété reste la photographie de C0. L'« État » donne la nouvelle répartition, et la puce D0b est marquée faite.

- **Commentaires** (CLAUDE.md) : un `/* TROU-n */` nu pour toute clé fractionnaire consignée ; l'aide à la lecture d'une ligne de `FRAMES_EVENTS` au besoin ; dans les tests de scénario, ce qui fonde chaque assertion. Le récit va dans les messages de commit.

## Implementation Chunks

Les chunks 1 à 4 suivent un même motif : fixture et référence d'abord, puis classement, puis conversion. Les chunks 1 à 3 ne dépendent que de D0a et peuvent se faire dans n'importe quel ordre. 5 et 6 viennent après.

1. **G1 — écran tech** (`cr` ×2, `hy_kt`). Fixture qui ouvre `ix[2]` et invente une tech abordable, test de scénario (`ew` ou `#invented` augmente), entrée `FRAMES_EVENTS` et référence posée sur le code actuel. Couverture et observation régénérées, puis classement des 3, revue des 3 clés, et conversion de ce qui est (b) et atteint. Acceptation : les 3 ne sont plus NON COUVERT ; `make test` vert à chaque commit ; `float-audit` compte les convertis parmi les sites C1.
2. **G2 — panneau de score et fin de partie** (`pj_ph_map` ×2, `pj_hk` `yr`). Fixture qui ouvre `ix[3]` puis avance jusqu'à la fin de partie par `ix[1]`, et test de scénario (`c_` vrai, `hl == "final score"`). Même séquence qu'en 1, plus la décision `yr` sourcée. Acceptation : celle de 1, et la justification de `yr` cite ses lignes.
3. **G3 — menu pause** (`ha2_my_dispatch`). Fixture qui ouvre `ix[4]` et active « toggle music », puis test de scénario. Classement attendu (c)/D1, sans conversion. Acceptation : le site est couvert et son annotation porte (c)/D1 avec la valeur vue.
4. **G4 — éclat de particules** (`ga_kj` ×5). On établit d'abord dans `lua.lua` le chemin qui fait naître un `ga` (protostar via `jo:hu` `hp.cw`), puis fixture, test de scénario (un `ga` vivant), classement, revue de la clé `flr(pt.l/2)`, et conversion des (b) et (a) signés atteints. Si le chemin est hors de portée d'une fixture raisonnable, les sites reçoivent une entrée « non protégé par le filet » avec la raison, puis (c)/D1, comme en 5. Acceptation : celle de 1.
5. **G5 — dispatch par type de clé** (`ho_apply`, `set_by_value`, `jc_display_get_by_value`, `research_lookup`, `jo_hu`). Pour chacun, on établit dans `lua.lua` si la branche nombre peut être prise. Si oui, une fixture l'atteint, avec la même séquence qu'en 1. Sinon, l'entrée « non protégé par le filet » va au plan, avec la source, et le site passe en (c)/D1. La revue de clé est faite pour les 5. Acceptation : `compte etape a auditer 0` et `compte population a auditer 0` dans l'inventaire, `make test` vert.
6. **Rapports et consignation.** `float-coverage`, `float-observe` et `float-guard` sont régénérés et versionnés, « État de D0b » écrit, puce D0b marquée faite, trous de portage ajoutés au besoin. Acceptation : `observation contradiction 0` ; les comptes de l'« État » sont lus dans les rapports, pas recopiés à l'avance.

## Testing Decisions

- Un bon test porte sur un comportement externe : l'état de jeu atteint par un rejeu, le rapport d'un outil, le code de sortie. Il n'inspecte pas l'implémentation d'un site.
- **Tests de scénario**, un par nouvelle fixture, dans `make unit`. Ils rejouent sans fenêtre et assertent un fait d'état de jeu que seule l'atteinte du site rend vrai. Précédents : `tests/test_pico8_top_menubar.c`, `tests/test_pico8_slipway_drag.c`, `tests/test_pico8_playability.c`.
- **Non-régression** : `make frames-check` identique sur tout `FRAMES_EVENTS`, sans `frames-baseline`, à chaque commit de classement et de conversion. La référence d'une nouvelle fixture est posée dans son commit d'ajout, avant tout classement.
- **Porte de conversion** : `make float-audit` (dans `make test`), avec la classe C1 élargie aux convertis, tous atteints, 0 écart, 0 négatif. Un site converti devient donc une vérification exécutée de sa preuve de signe.
- **Classement** : les contrôles mécaniques de `float-inventory-check` (annotation orpheline ou manquante, (a) non sourcé contre `lua.lua`, étape incohérente). Le jugement de preuve se fait en revue du commit, comme en C0.
- **Pas de test unitaire au niveau valeur pour les sites (c)/D1 de D0b** : la règle « Corrections invisibles au filet » s'applique à l'étape qui convertit, D1. D0b ne livre des tests que s'il en faut un pour *atteindre* un site. Dans ce cas, le site n'entre pas dans la liste C1, faute d'atteinte par le filet.
- Les tests d'outils existants (`test_float_audit`, `test_float_inventory`, `test_float_observe`, `test_frames_check`) ne changent pas : D0b ne modifie aucun outil.

## Out of Scope

- **La conversion des casts (c), ou (a) à signe non prouvé** : ils partent en D1, avec les 22 existants.
- **La correction d'une clé fractionnaire** : elle est consignée en trou de portage, pas corrigée.
- **Toute constante**, `pico8_value_tostr` (D2), `^0.75` / `pow` (D3), `sin`/`cos` et les définitions de `from_double`/`to_double` (décision ouverte), D0c.
- **Toute modification d'outil** : ni marqueur « inatteignable » dans l'audit, ni changement de l'inventaire ou de l'annotation au-delà de leurs lignes.
- **La suppression de branches mortes** de dispatch par type.
- **Tout `frames-baseline` qui re-baseline une référence existante.** Seules les références des nouvelles fixtures sont posées.
- **L'audio** (hors périmètre du portage) : la fixture du menu pause active `music()` pour atteindre le site, sans rien vérifier du son.
- **Le volet `malloc`** (§8).
- **Lancer l'application raylib.**

## Further Notes

- **Texte de l'étape (docs/plan-2.md, §7 Phase D).** « **D0b — Les 17 casts bloqués.** Sortie : « à auditer » = 0. Chacun est atteint par une fixture scriptée ou un test au niveau valeur, ou reçoit une entrée explicite « non protégé par le filet », puis est classé ; la revue de clé de ses 11 sites clé de table se fait ici, appuyée par le compte d'arguments fractionnaires de D0a. Un cast reclassé (b) est converti comme en C1 (et rejoint la liste figée) ; un cast reclassé (c) rejoint D1, dont le périmètre est ainsi fixé **avant** D1. »
- **Corrections invisibles au filet** (même section). « pour chaque étape D, les sites à zéro appel ancien ≠ nouveau sur le filet sont listés « correction non vue par le filet » (comme les 13 constantes de C2b). Chacun reçoit un test au niveau valeur de l'expression convertie (par exemple une entrée négative fractionnaire qui doit donner `flr`) ou une entrée explicite dans ce plan. Critère de couverture, pas de ligne `gcov` : C a montré que NON COUVERT ne prouve rien dans un sens comme dans l'autre. Structurel pour D1 : `oi`, `sr` et `flr(ratio)` ne sont (c) que par le repli de `qc.t` à 32 768 frames, alors que le plus long fichier du filet en fait 9 729. » Pour D0b, les conversions sont (b) et préservatrices, donc ancien == nouveau par construction. Les sites que D0b envoie en D1 grossissent la liste de D1 à traiter sous cette règle.
- **NON COUVERT ne prouve rien.** C2a l'a montré (`pico8_g__mm`, « ligne sans code »). Un site G5 n'est pas déclaré inatteignable sur la foi de la couverture. Il l'est sur une source dans `lua.lua`.
- **Candidats relevés en préparant ce PRD** (à vérifier, non classants) : `s[6]` vient de `min(flr(total/2000),5)` (lua.lua:1361, contient `flr(`) ; `i<=s[6]` compare un entier `i` à `s[6]`, et pour un entier `i`, `i <= x` équivaut à `i <= flr(x)` quel que soit le signe. Le fait peut être noté, mais il ne remplace pas la preuve de signe exigée pour (a) → C1.
- **Coût.** Chaque fixture ajoutée est rejouée deux fois de plus par `make test` (`frames-check`, `float-audit`). La fixture de fin de partie compte au moins 25 clics. On consigne la durée avant/après dans l'« État de D0b ». Si le coût dépasse nettement celui de D0a (+13 s), on le consigne, sans sortir une cible de la suite sans décision.
