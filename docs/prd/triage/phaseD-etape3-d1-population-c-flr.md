---
title: "Plan 2, Phase D1 : les 29 casts de population (c) passent à flr, avec attribution par l'audit et re-baseline revu"
description: "Les 29 derniers casts (int)pico8_fixed_to_double(x) du cœur tronquent vers zéro alors que PICO-8 applique flr (bloc B du relevé) ; D1 les convertit en pico8_fixed_flr_int, attribue chaque première frame divergente à un site par l'audit avant/après, vérifie que cette correction d'affichage ne cascade pas dans l'état du jeu, liste les corrections que le filet ne voit pas, puis re-baseline volontairement."
status: needs-triage
---

## Problem Statement

Le relevé PICO-8 (`tools/probe_out.txt`, bloc B) a tranché : `spr()` et les primitives appliquent **`flr`** à leurs coordonnées, et `flr` arrondit vers −∞ (`flr(-1.5) → -2`, `flr(-0.5) → -1`). Le portage C a hissé la conversion au site d'appel sous la forme `(int)pico8_fixed_to_double(x)`, qui **tronque vers zéro**. Pour tout argument négatif non entier, le port est faux d'une unité (§3.2, décision 13). C'est le deuxième bug confirmé du chantier.

Après C1 et D0b, il reste **29 casts** de ce type dans le cœur, tous affectés à D1 par `tools/float_annotation.txt` (`compte etape D1 29`) :

- **27 de population (c)** : le Lua est muet et la valeur peut être négative (coordonnées écran, décalage de caméra, rayons après le repli de `qc.t`, argument de `music`/`sfx`, et les 5 branches « clé nombre » de G5) ;
- **2 de population (a) à signe non prouvé** : `pico8_mz_kj` `pico8_fixed_flr(ratio)` (`kk[flr(sr/self.oi*8)]`, lua.lua:790) et `pico8_pj_ph_map` `s6_val` (`s[6]=min(flr(total/2000),5)`, lua.lua:1361).

La phrase d'introduction de la Phase D parle encore de « 22 casts D1 (21 (c) + 1 (a)) ». C'est la photographie de C0. D0b y a ajouté 7 casts (G2 `s6`, G3 `ha2` `s`, les 5 de G5).

Tant qu'ils existent :

1. **Le bug reste.** Dès qu'une coordonnée fractionnaire passe sous zéro, l'objet est dessiné un pixel trop à droite ou trop bas. `tools/float_observe.txt` montre que le filet le produit déjà : `pico8_bl` (`camera(nc,0)`, secousse de caméra) reçoit un négatif fractionnaire, avec un minimum de `0xFF7BAA4D`.
2. **Le cœur garde 29 jetons flottants.** L'inventaire compte 29 casts, et plusieurs fichiers du cœur restent dans `tools/float_guard.txt` à cause d'eux seuls (`pico8_menu.c`, `pico8_score_panel_display.c`…).
3. **La correction va faire diverger le filet.** La Phase D exige que la **première** frame divergente de chaque fichier d'événements soit attribuée à un site par la règle d'attribution. Il faut aussi établir que ce changement d'affichage **ne cascade pas** dans l'état du jeu, versionner le rapport de divergence avant le re-baseline, et revoir les frames concernées. On ne subit pas le re-baseline comme une régression.
4. **Beaucoup de corrections seront invisibles au filet.** Les 5 sites de G5 ne sont jamais atteints. `oi`, `sr` et `flr(ratio)` ne deviennent négatifs qu'après le repli de `qc.t` à 32 768 frames, alors que le plus long fichier du filet en fait 9 729. Les sites vus négatifs ne reçoivent peut-être que des entiers (`pico8_rn` −1, `pico8_bg_to_int` −12, `pico8_hud_to_int` −3), et sur un entier, troncature et `flr` coïncident. La règle « Corrections invisibles au filet » exige qu'on dise, pour chacun, ce qui le protège.

## Solution

Les 29 casts deviennent `pico8_fixed_flr_int(<argument>)`, la fonction déjà utilisée par les 47 sites C1. Elle est testée au niveau valeur contre le bloc B. Chaque ligne d'annotation est retirée dans le même commit. Les sites convertis ne rejoignent **pas** `tools/float_audit_c1.txt` : l'audit les classe donc **D**, et vérifie pour eux « nouveau == `flr` ». Il donne aussi, par fichier d'événements, la frame du premier ancien ≠ nouveau de chaque site. C'est l'entrée de la règle d'attribution.

Le travail se fait en trois commits :

1. **Un commit neutre de mise en ligne.** Aujourd'hui, `src/pico8_layout.c` a sur la même ligne un site C1 (`item->sprite_id`) et un futur site D (`pos.x`). L'audit compte par (fichier, ligne) : une fois `pos.x` converti, la garde C1 de `sprite_id` hériterait des négatifs et des écarts de `pos.x`. L'appel est donc remis en forme pour que `sprite_id`, `pos.x` et `pos.y` soient chacun sur sa ligne. Aucun caractère significatif ne change. `make test` reste vert, et `frames-check` identique. Les enregistrements d'audit de cet arbre (`tmp/audit/*.aud`) sont mis de côté : ils servent de référence « avant » pour vérifier l'absence de cascade.
2. **La conversion, l'attribution, la revue et le re-baseline**, dans un seul commit, comme le commit de valeurs de C2b :
   - conversion des 29 casts, suppression des 29 lignes d'annotation, inventaire régénéré ;
   - `make float-audit` : 0 échec, et on lit le premier ancien ≠ nouveau par fichier ;
   - `make frames-report ETAPE=D1` ;
   - on applique la règle d'attribution à chaque fichier divergent ;
   - on vérifie l'absence de cascade ;
   - revue sans fenêtre de chaque frame F, avant et après ;
   - `make frames-baseline ETAPE=D1`.

   Le commit porte `tools/frames_reports/D1.txt`, `tools/frames_reports/D1-attribution.txt` et les références réécrites.
3. **La consignation.** On régénère couverture et garde et on constate l'état de l'observation. Dans `docs/plan-2.md` : un « État de D1 » avec la liste des corrections non vues par le filet, chacune avec ce qui la protège ; la puce D1 marquée faite ; la ligne D1 de la table de propriété mise à jour.

Si l'une des vérifications du commit 2 échoue, on n'applique **pas** de re-baseline. Les cas d'échec sont :

- la règle d'attribution échoue pour un fichier ;
- un fichier diverge sans qu'aucun site D1 ait d'écart ;
- un enregistrement C1 change entre avant et après, c'est-à-dire une cascade d'état ;
- un « nouveau != flr ».

L'étape s'arrête. La conversion n'est pas commitée, et le constat est consigné dans le plan et rapporté.

## User Stories

1. En tant qu'agent qui mène D1, je veux que chaque cast (c) applique `flr` et non la troncature, pour que le port dessine là où PICO-8 dessine pour toute coordonnée négative fractionnaire (bloc B, décision 13).
2. En tant qu'agent, je veux que les 2 casts (a) à signe non prouvé passent aussi à `flr`, pour que leur `flr(` explicite du Lua soit porté tel quel, sans preuve de signe à fournir.
3. En tant qu'agent, je veux convertir vers `pico8_fixed_flr_int`, la fonction déjà vérifiée contre le bloc B, pour qu'aucune nouvelle arithmétique ne soit introduite et que la valeur de chaque expression convertie ait déjà un test.
4. En tant qu'agent, je veux que la conversion soit mécanique (seul le cast change, ni l'argument ni le code autour), pour que le diff de chaque site se lise d'un coup d'œil et que le périmètre soit exactement les 29 lignes annotées D1.
5. En tant qu'agent, je veux que `pico8_music(-(int)pico8_fixed_to_double(s))` devienne `pico8_music(-pico8_fixed_flr_int(s))`, pour que la négation, qui est le `-sgn(...)` du Lua, reste hors de la conversion.
6. En tant qu'agent, je veux que `(int)pico8_fixed_to_double(pico8_fixed_flr(ratio))` devienne `pico8_fixed_flr_int(pico8_fixed_flr(ratio))`, sans supprimer le `flr` intérieur, pour que la conversion reste chirurgicale. La valeur est la même : `flr` d'un entier est cet entier.
7. En tant qu'agent, je veux retirer la ligne d'annotation de chaque cast converti dans le même commit, pour que `float-inventory-check` reste vert et que l'inventaire tombe à `compte cast 0`.
8. En tant qu'agent, je veux que les sites convertis n'entrent pas dans `tools/float_audit_c1.txt`, pour que l'audit les classe D. Pour eux, un négatif et un écart sont la correction voulue, pas un échec.
9. En tant qu'agent, je veux que le site C1 `sprite_id` de `pico8_lq_ol_item_draw` soit sur une autre ligne que `pos.x` avant la conversion, pour que la garde C1 ne partage pas ses compteurs avec un site D.
10. En tant qu'agent, je veux que cette remise en forme soit un commit à part, neutre, pour que les numéros de ligne des enregistrements d'audit soient les mêmes avant et après la conversion.
11. En tant qu'agent, je veux que `make float-audit` n'ait aucun échec après conversion (C1 inchangé, D sans `NOUVEAU != FLR`), pour que la porte de C1 et celle de D tiennent ensemble.
12. En tant qu'agent, je veux lire dans l'audit, pour chaque fichier d'événements, la plus petite frame de premier ancien ≠ nouveau et les sites qui l'atteignent, pour disposer de l'entrée de la règle d'attribution sans outil nouveau.
13. En tant qu'agent, je veux lancer `make frames-report ETAPE=D1` avant tout re-baseline, pour que le rapport de divergence complet soit versionné et relu avant décision.
14. En tant qu'agent, je veux vérifier, pour chaque fichier divergent, que sa première frame divergente F égale la plus petite frame de premier ancien ≠ nouveau parmi les sites D1 de ce fichier, pour que F soit attribuée à un site nommé par la règle du plan.
15. En tant qu'agent, je veux que, si plusieurs sites atteignent ensemble cette plus petite frame, tous soient nommés et que l'égalité soit consignée comme telle, pour qu'une attribution partagée soit un constat et non un choix arbitraire.
16. En tant qu'agent, je veux qu'une ligne partagée entre deux sites D (`cp.x`/`cp.y` de `pico8_qb_md`, `cp.x`/`cp.y` de `pico8_mz_kj`) désigne la ligne, comme D0a l'avait annoncé, pour ne pas réécrire du code dans le seul but de départager deux arguments d'un même appel.
17. En tant qu'agent, je veux qu'un fichier conforme au filet mais avec un écart à l'audit soit noté « correction sans effet sur l'empreinte », pour ne pas confondre « le filet ne l'a pas vue » avec « elle n'a pas eu lieu ».
18. En tant qu'agent, je veux que tout fichier divergent sans écart D1, ou dont F diffère de la frame désignée par l'audit, arrête l'étape, pour que ce qui n'est pas attribué ne soit jamais re-baseliné.
19. En tant qu'agent, je veux vérifier que D1 ne cascade pas : chaque ligne d'enregistrement d'audit de l'arbre d'avant (tous sites C1, par fichier d'événements) doit se retrouver à l'identique dans l'arbre d'après, pour qu'un changement d'état du jeu soit détecté au lieu d'être absorbé par le re-baseline.
20. En tant qu'agent, je veux établir, par lecture, que chaque site D1 ne nourrit qu'une primitive d'affichage (`camera`, `spr`, `circ`, `line`, `pset`, `rect`/`rectfill`), un appel audio sans effet (`music`, `sfx`), un choix d'icône ou de couleur, ou une branche morte, pour que « D1 ne doit pas cascader » repose aussi sur le code et pas seulement sur un échantillon.
21. En tant qu'agent, je veux que, si un site D1 nourrit l'état du jeu, l'étape s'arrête et le constat soit rapporté, pour que le plan, qui suppose un changement d'affichage seul, soit corrigé par une décision et pas par un re-baseline.
22. En tant que relecteur, je veux une revue sans fenêtre de chaque frame F (dump avant, dump après, lignes, pixels et boîte de l'écart), pour que chaque re-baseline ait été regardé, comme en C2b.
23. En tant que relecteur, je veux que la boîte de l'écart à F soit rapprochée de ce que dessine le site désigné (par exemple les entités décalées d'une colonne par la caméra de `pico8_bl`), pour que l'attribution soit confirmée par l'image et pas seulement par le compteur.
24. En tant qu'agent, je veux `tools/frames_reports/D1-attribution.txt`, sur le modèle de `C2b-attribution.txt`, pour chaque fichier. Il donne F, le ou les sites désignés avec la ligne de l'audit, la revue, le nombre de frames divergentes après F et le résultat de la vérification d'absence de cascade, et ce fichier doit être versionné pour que l'attribution se vérifie après coup sans `tmp/`.
25. En tant qu'agent, je veux `make frames-baseline ETAPE=D1` dans le commit de conversion, pour que `make test` soit vert à chaque commit et que `tools/frames_reports/D1.txt` soit versionné avec les références.
26. En tant qu'agent, je veux lister tous les sites D1 à zéro ancien ≠ nouveau sur le filet, non atteints compris, sous « correction non vue par le filet », pour que la règle de la Phase D soit appliquée à partir de la sortie de l'audit et pas d'une prévision.
27. En tant qu'agent, je veux que chaque correction non vue reçoive dans le plan une entrée explicite qui dit pourquoi le filet ne la voit pas (branche morte, repli de `qc.t`, argument déjà entier, jamais négatif fractionnaire) et ce qui la protège, pour que le critère de couverture de la Phase D soit tenu site par site.
28. En tant qu'agent, je veux que ce qui protège la valeur de chaque expression convertie soit le test existant de `pico8_fixed_flr_int` contre le bloc B, cité par son nom, pour ne pas écrire 29 tests qui retesteraient la même fonction à travers des chemins d'appel.
29. En tant qu'agent, je veux régénérer `tools/float_coverage.txt` et `tools/float_guard.txt` et lire leurs comptes, pour que l'« État de D1 » cite des chiffres mesurés.
30. En tant qu'agent, je veux constater que `make float-observe` s'arrête sur « aucun cast » (code 2) une fois l'inventaire à zéro cast. Je le consigne, et je garde `tools/float_observe.txt` comme la photographie de D0b, pour ne pas changer un outil dont l'objet a disparu.
31. En tant qu'agent, je veux un « État de D1 » dans `docs/plan-2.md`, pour que la phase D suivante reparte d'un état mesuré. Il contient :
    - la conversion ;
    - l'audit, avec les sites C1 et D, les passages, les négatifs et fractionnaires aux sites D, et le premier ancien ≠ nouveau par fichier ;
    - l'attribution ;
    - l'absence de cascade ;
    - la revue ;
    - les corrections non vues ;
    - l'inventaire, la couverture, la garde et l'observation ;
    - la durée de `make test`.
32. En tant que mainteneur, je veux que la ligne D1 de la table de propriété soit marquée faite et dise que D1 a converti 29 casts (22 de C0 et 7 venus de D0b), pour que la table reste lisible sans être réécrite.
33. En tant que mainteneur, je veux qu'aucun commentaire ne soit ajouté aux sites convertis, pour respecter la règle des quatre destinations de `CLAUDE.md`. Le récit va dans les messages de commit.
34. En tant qu'agent sans yeux, je veux que toute vérification se fasse sans fenêtre, pour respecter `CLAUDE.md` : jamais de lancement de l'application raylib.

## Implementation Decisions

- **Les 29 sites**, lus dans `tools/float_annotation.txt`. On les reprend ici pour situer ce que le filet peut voir. L'observation est celle de `tools/float_observe.txt` (D0b), qui est un indice et non une prévision.

  | Groupe | Site(s) | Consommateur | Observation D0b |
  |---|---|---|---|
  | Caméra | `pico8_bl` `nc` ; `pico8_qb_md` `cp.x`, `cp.y` (ligne partagée) | `camera` | `nc` vu négatif **fractionnaire** (`0xFF7BAA4D`) ; `cp` min 0 |
  | Aides `*_to_int` | `pico8_bg_to_int`, `pico8_hud_to_int`, `pico8_particle_to_int`, `pico8_probe_to_int`, `pico8_jc_display_to_int`, `pico8_ui_to_int` (`ui.c`, `ui_button.c`) | primitives de dessin de leurs appelants | `bg` −12, `hud` −3 (minimums entiers) ; les autres jamais négatifs |
  | Coordonnées directes | `pico8_lq_ol_item_draw` `pos.x`, `pos.y` ; `pico8_jo_kj` `adjusted.x`, `.y` ; `pico8_mz_kj` `p.x`, `p.y`, `cp.x`, `cp.y` (ligne partagée) | `spr`, `circfill`, `circ`, `line` | jamais négatifs |
  | Rayons et indice (repli de `qc.t`) | `pico8_mz_kj` `oi`, `sr`, `pico8_fixed_flr(ratio)` | `circ`, couleur `kk[…]` | jamais négatifs ; (c) seulement après 32 768 frames |
  | Audio | `pico8_rn` `no` ; `pico8_ha2_my_dispatch` `s` | `sfx`, `music` : ne font rien ([TROU-10](../../trous-de-portage.md)) | `no` −1 (entier) ; `s` vu 1 fois, 1 |
  | Icône | `pico8_pj_ph_map` `s6_val` | comparaison `i <= s[6]`, choix de sprite | min 0 |
  | G5, branches mortes | `pico8_ho_apply`, `set_by_value`, `pico8_jc_display_get_by_value`, `pico8_research_lookup`, `pico8_jo_hu` | recherche de clé nombre | NON COUVERT, non protégés par le filet (D0b) |

- **Forme de la conversion** (méthode de C1). `(int)pico8_fixed_to_double(<arg>)` devient `pico8_fixed_flr_int(<arg>)`, et rien d'autre ne change. Les deux formes composées sont `-pico8_fixed_flr_int(s)` et `pico8_fixed_flr_int(pico8_fixed_flr(ratio))`. Les aides `*_to_int` gardent leur nom et leur signature : leur corps seul change.

- **Remise en ligne de `pico8_lq_ol_item_draw`, avant la conversion.** L'appel `pico8_spr_default(…)` s'étale aujourd'hui sur deux lignes, avec `sprite_id` (C1) et `pos.x` (futur D) sur la même. Il est reformaté : un argument converti par ligne à partir de `sprite_id`, pour que `sprite_id`, `pos.x` et `pos.y` aient chacun leur ligne. C'est la seule ligne de tout le cœur où un site C1 et un futur site D se partagent une ligne (vérifié en préparant ce PRD). La clé de la liste C1 (fichier, fonction, extrait, occurrence) ne dépend pas du numéro de ligne : elle survit. Les deux lignes partagées D/D (`pico8_scroll.c` `cp.x`/`cp.y`, `pico8_wormhole_display.c` `cp.x`/`cp.y`) restent telles quelles : D0a avait accepté que la frame du premier écart y désigne la ligne.

- **Classe D, sans liste.** L'audit classe D tout appel de `pico8_fixed_flr_int` absent de la liste C1. Aucune liste D n'est créée. La porte D reste celle de D0a : échec sur `NOUVEAU != FLR` seulement. Négatifs, fractionnaires et ancien ≠ nouveau sont des mesures, et zéro ancien ≠ nouveau sort « correction non vue par le filet ». L'outil d'audit ne change pas.

- **Règle d'attribution appliquée** (plan, « Règle d'attribution d'une étape D »). Pour chaque fichier d'événements :
  - F est la première frame divergente dans `tools/frames_reports/D1.txt` ;
  - A est la valeur imprimée par l'audit, « premier ancien!=nouveau frame A | sites », pour ce fichier ;
  - les sites C1 n'ayant jamais d'écart, A est le minimum parmi les sites D1.

  Les cas possibles :

  | F | A | Verdict |
  |---|---|---|
  | conforme | aucun | conforme |
  | conforme | une valeur | « correction sans effet sur l'empreinte », consigné |
  | F | A = F | attribué au(x) site(s) nommé(s) |
  | F | A < F ou A > F | **échec de la règle** |
  | F | aucun | **échec de la règle** |

  Un échec arrête l'étape. On n'élargit pas la règle en cours de route pour « expliquer » un A < F, par exemple un écart sans effet sur un pixel aux frames précédentes. On le rapporte, et la décision revient au plan.

- **Égalité à A.** Si plusieurs sites (ou une ligne partagée) atteignent A, l'audit les nomme tous. L'attribution les nomme tous, et consigne « égalité ». La revue à F dit, quand elle le peut, lequel dessine dans la boîte de l'écart. On ne découpe pas la conversion en sous-étapes pour départager : aucun harnais de variantes par site n'existe (`constant-variants` ne voit plus de constante depuis C2b), et le seul négatif fractionnaire vu sur le filet est `pico8_bl`.

- **Absence de cascade, deux preuves.**
  1. *Lecture.* Pour chaque site, et pour chaque appelant d'une aide `*_to_int`, on établit que la valeur ne va qu'à :
     - une primitive d'affichage ;
     - `pico8_camera`, qui n'écrit que `camera_x`/`camera_y`, et ceux-ci ne sont lus que par le dessin ;
     - `music`/`sfx`, qui ne font rien ;
     - un choix d'icône ou de couleur ;
     - une branche morte.

     C'est consigné dans l'attribution.
  2. *Mesure.* Les enregistrements `tmp/audit/<events>.aud` de l'arbre du commit 1 (avant) et de l'arbre converti (après) sont comparés. Chaque ligne `AUD` d'avant doit exister à l'identique dans l'après, pour chaque fichier d'événements. Ce sont tous des sites C1 : appels, négatifs, écarts, fractionnaires, frame. L'après n'a en plus que des lignes de sites D. Les 47 sites C1 échantillonnent la logique de jeu (`pico8_gn_next`, `pico8_pj_hk`, `pico8_hy_kt`…) : si l'état change, leurs comptes bougent.

  Le couple de preuves est consigné sans script versionné : la commande va dans le message de commit, le résultat dans l'attribution. Un écart de l'une ou l'autre arrête l'étape (« D1 nourrit l'état du jeu »). Les frames divergentes après F sont notées « suite, affichage seul » dans l'attribution, avec leur nombre, et non « cascade d'état ».

- **Revue sans fenêtre à F** (méthode de C2b). On capture avant et après avec `pico8_frames frame`. L'arbre d'avant est celui du commit 1. On compte les lignes et les pixels qui diffèrent sur 128, et on donne la boîte de l'écart en indices 0..127. On la rapproche de ce que dessine le site désigné : pour `pico8_bl`, un décalage d'une colonne des entités dessinées sous la secousse. Aucun jugement de justesse du pixel : la justesse vient du bloc B.

- **Si rien ne diverge** (le négatif fractionnaire de `pico8_bl` pourrait ne changer aucun pixel), D1 n'est pas divergente. `frames-check` reste identique, sans `frames-baseline`. On ne versionne ni `D1.txt` ni l'attribution, et l'« État de D1 » le consigne, avec les écarts d'audit sans effet sur l'empreinte.

- **Corrections non vues : ce qui compte comme « test au niveau valeur ».** L'expression convertie à chaque site est `pico8_fixed_flr_int(<arg>)`. Sa valeur sur des entrées négatives fractionnaires est assertée contre le relevé par `test_flr_int_matches_probe_block_b` (et par `test_flr_int_extremes` et `test_flr_int_raw_minus_one_is_minus_one_not_zero`) dans `tests/test_pico8_fixed.c`. Les deux formes composées se ramènent à ce test :
  - `-flr(s)` : la négation est hors de la conversion ;
  - `flr(flr(r)) = flr(r)`.

  **Pas de test par site.** Il faudrait atteindre chaque site par un chemin d'appel, ce qui est impossible pour G5 et le repli de `qc.t`. Un tel test retesterait la même fonction. Chaque site non vu reçoit une **entrée explicite dans le plan** : site, raison pour laquelle le filet ne le voit pas, protection (le test ci-dessus, par son nom). Raisons attendues, à confirmer par l'audit :
  - G5, non atteint ;
  - `oi`, `sr`, `flr(ratio)`, qui ne deviennent (c) qu'après le repli de `qc.t`, et `flr(ratio)` est entier par construction ;
  - « négatifs entiers seulement » (probablement `pico8_rn`, `bg`, `hud`) ;
  - « jamais négatif fractionnaire sur le filet » pour les autres.

- **Clés de table des 5 sites G5.** En Lua, une clé nombre fractionnaire est une clé distincte (§3.2). Sur ces branches, `flr` ne rend pas le port fidèle, pas plus que la troncature. D0b a établi que ces clés sont des chaînes par construction, donc qu'aucune clé nombre n'arrive jamais. On n'ouvre aucun trou de portage, et on ne supprime pas la branche, qui est le contrat de la fonction. D4 garde la revue de la seule clé D1 à valeur entière, `kk[flr(ratio)]`.

- **Observation sans objet.** Une fois l'inventaire à zéro cast, `make float-observe` s'arrête sur « aucun cast » (code 2, vérifié en préparant ce PRD sur une copie convertie). Le Makefile ne remplace alors pas la copie versionnée. On ne touche pas l'outil : sa garde « jointure vide » ne distingue pas un inventaire vide, et adapter ou retirer l'outil n'est pas l'objet de D1. On consigne le constat dans l'« État de D1 », et `tools/float_observe.txt` reste la photographie de D0b. `float_inventory.sh` accepte une annotation sans ligne de cast, ce qui a aussi été vérifié : `compte cast 0`, `compte total 28`.

- **Invariants.** Les éléments suivants ne changent pas :
  - la liste C1 et l'outil d'audit ;
  - `pico8_fixed_flr_int` ;
  - les constantes, `pico8_value_tostr` (D2), `pico8_dj` (D3), `pico8_fixed_sin`/`cos` (D5) ;
  - les définitions `from_double`/`to_double`.

  `constants-check`, `float-inventory-check` et `float-audit` restent verts dans `make test` à chaque commit.

- **Consignation dans `docs/plan-2.md`.** On ajoute un « État de D1 » après l'« État de D0b ». Il contient :
  - les chiffres lus dans les outils, jamais recopiés à l'avance ;
  - un bloc « Corrections non vues par le filet (D1) », une ligne par site ;
  - la puce D1 marquée « ✅ faite, voir État de D1 » ;
  - la ligne D1 de la table de propriété mise à jour, avec les jetons de C0 inchangés et la mention des 7 venus de D0b.

  L'introduction de la Phase D (« 22 casts D1 ») n'est pas réécrite : l'« État » dit 29.

## Implementation Chunks

1. **Remise en ligne neutre de `pico8_lq_ol_item_draw`.**
   - *Travail.* Reformatage de l'appel à `pico8_spr_default`, pour que `sprite_id` (C1) ne partage plus sa ligne avec `pos.x`. `make float-audit` est lancé et ses `tmp/audit/*.aud` sont copiés hors de `tmp/audit/` (le prochain passage les efface) : c'est la référence « avant ».
   - *Acceptation.*
     - `make test` vert ;
     - `frames-check` identique sur les 12 fichiers, sans `frames-baseline` ;
     - l'audit donne les mêmes comptes qu'avant (47 sites C1, 0 site D, 0 ligne partagée, mêmes passages) ;
     - les lignes `AUD` sont identiques à celles d'avant le reformatage, à la ligne de `pico8_layout.c` près.
2. **Conversion, attribution, revue, re-baseline.** Les étapes, dans l'ordre :
   1. conversion des 29 casts ;
   2. suppression de leurs 29 lignes d'annotation ;
   3. `make float-inventory` ;
   4. `make float-audit`, qui doit donner 0 échec, 47 C1 et les sites D ;
   5. `make frames-report ETAPE=D1` ;
   6. règle d'attribution par fichier ;
   7. les deux preuves d'absence de cascade ;
   8. revue à F ;
   9. `tools/frames_reports/D1-attribution.txt` écrit ;
   10. `make frames-baseline ETAPE=D1`.

   *Acceptation.*
   - `make test` vert ;
   - l'inventaire indique `compte cast 0` et `compte etape D1 0` ;
   - chaque fichier divergent a F = A et un ou des sites nommés ;
   - toutes les lignes `AUD` d'avant sont retrouvées à l'identique ;
   - `D1.txt` et `D1-attribution.txt` sont versionnés dans le commit.

   *Sinon :* arrêt sans commit de conversion ni re-baseline. Le constat est consigné dans le plan (sous « État de D1 », marqué bloqué) et rapporté.
3. **Rapports et consignation.**
   - *Travail.* `make float-coverage` et `make float-guard` sont régénérés et versionnés. On constate que `make float-observe` rend le code 2. On écrit l'« État de D1 », avec les corrections non vues, la puce et la ligne de la table de propriété, et on mesure la durée de `make test`.
   - *Acceptation.*
     - chaque site D1 à zéro ancien ≠ nouveau dans l'audit a son entrée ;
     - les comptes de l'« État » sont ceux des rapports ;
     - la liste des fichiers sortis de la garde est lue dans `tools/float_guard.txt`.

## Testing Decisions

- Un bon test porte sur un comportement externe : la valeur d'une fonction contre le relevé, le rapport et le code de sortie d'un outil, l'empreinte d'un rejeu. Il n'inspecte pas l'implémentation d'un site.
- **Niveau valeur** : les tests existants de `pico8_fixed_flr_int` dans `tests/test_pico8_fixed.c` (bloc B du relevé, extrêmes, −1 brut). D1 n'en ajoute pas : les 29 expressions converties sont cette fonction, et la décision est expliquée plus haut. Aucun test n'est retiré.
- **Porte d'audit** (`make float-audit`, dans `make test`) : C1 inchangé (atteint, 0 écart, 0 négatif), D sans `NOUVEAU != FLR`. C'est une vérification exécutée sur tout le filet que la nouvelle valeur est `flr` à chaque appel atteint.
- **Non-régression** :
  - `frames-check` identique au commit 1 ;
  - au commit 2, divergence attribuée, puis re-baseline versionné ;
  - au commit 3, `frames-check` identique aux nouvelles références.
- **Absence de cascade** : la comparaison des enregistrements `AUD` des sites C1 entre l'arbre d'avant et celui d'après. Elle est manuelle et consignée, et n'entre pas dans la suite : c'est une vérification d'étape, pas un invariant permanent.
- **Tests d'outils existants** (`test_float_audit`, `test_float_inventory`, `test_float_observe`, `test_frames_report`, `test_frames_check`) : ils ne changent pas, puisque D1 ne modifie aucun outil.
- Précédents : C1 pour la conversion et la porte d'audit (`docs/prd/done/phaseC-etape2-c1-conversion-preservatrice.md`) ; C2b pour le rapport, l'attribution, la revue sans fenêtre et le re-baseline (`docs/prd/done/phaseC-etape4-c2b-constantes-du-releve.md`, `tools/frames_reports/C2b-attribution.txt`).

## Out of Scope

- **D2** (`tostr`), **D3** (`^0.75`), **D4** (revue des clés, au-delà du constat sur G5), **D5** (`sin`/`cos` sur table), **E** (définitions `from_double`/`to_double`, `-lm`, garde vide).
- **Toute modification d'outil** : l'audit, l'observation, l'inventaire, les rapports de frames. Ni liste D, ni enregistrement de toutes les frames d'écart, ni harnais de variantes par site.
- **Le retrait ou l'adaptation de `float-observe`**, devenu sans objet. C'est un constat pour E ou pour l'humain.
- **La suppression des branches mortes de G5**, ou la simplification de `flr_int(flr(ratio))`.
- **TROU-12** (`kk[0]` noir) et **TROU-20/21** : D1 ne les corrige pas, même si `flr(ratio)` et les coordonnées de `pico8_mz_kj` les touchent.
- **L'audio** : `music`/`sfx` restent sans effet ; convertir leur argument ne vérifie rien du son.
- **Le volet `malloc`** (§8).
- **Lancer l'application raylib.**

## Further Notes

- **Texte de l'étape (docs/plan-2.md, §7 Phase D).**
  - *Introduction* : « Périmètre, d'après la table de propriété (Phase C) : **22 casts D1** (21 (c) + 1 (a) de signe non prouvé), **les 17 casts « à auditer »** (dont 11 clés de table), **3 jetons D2** (`tostr`), **5 jetons D3** (`^0.75`), et la revue des clés de table (D4). […] La sortie de chaque étape s'énonce en « compte d'étape = 0 » dans l'inventaire annoté. Sources, inchangées : population (c) → **`flr`** (bloc B, correction de bug sourcée) ; […] clés de table revues contre la sémantique des clés fractionnaires (§3.2). »
  - *Puce* : « **D1 — population (c).** Divergente, avec re-baseline. »
  - *Règle d'attribution d'une étape D* : « la première frame divergente F de `tools/frames_reports/<étape>.txt` doit égaler la plus petite « frame du premier ancien ≠ nouveau » parmi les sites de l'étape ; ce site est désigné. Même règle qu'en C2b, lue dans un seul rejeu. »
  - *Corrections invisibles au filet* : « pour chaque étape D, les sites à zéro appel ancien ≠ nouveau sur le filet sont listés « correction non vue par le filet » (comme les 13 constantes de C2b). Chacun reçoit un test au niveau valeur de l'expression convertie (par exemple une entrée négative fractionnaire qui doit donner `flr`) ou une entrée explicite dans ce plan. Critère de couverture, pas de ligne `gcov` : C a montré que NON COUVERT ne prouve rien dans un sens comme dans l'autre. Structurel pour D1 : `oi`, `sr` et `flr(ratio)` ne sont (c) que par le repli de `qc.t` à 32 768 frames, alors que le plus long fichier du filet en fait 9 729. »
  - *Sortie de phase* : « pour chaque étape D divergente et chaque fichier d'événements, la première frame divergente est attribuée à un site par la règle ci-dessus, la suite étiquetée cascade (un changement d'affichage seul, D1, ne doit pas cascader), et le rapport de divergence est versionné avant le re-baseline ; `frames.hash` re-baseliné **volontairement**, avec revue des frames concernées — jamais subi comme une régression. D2 ne re-baseline pas. »
- **Décisions prises sans humain** (guideline : le plus simple qui se vérifie) :
  - pas de test par site : une entrée de plan qui cite le test de valeur existant ;
  - égalité à A acceptée et nommée, sans sous-étapes ;
  - une seule ligne remise en forme (la ligne mixte C1/D) ;
  - `float-observe` constaté, pas modifié ;
  - l'absence de cascade prouvée par lecture et par la comparaison des enregistrements C1, sans nouvel outil.

  Chacune se conteste au triage.
- **Prévision, pas critère.** D'après l'observation de D0b, le seul site susceptible de faire diverger le filet est `pico8_bl`. Les minimums négatifs de `pico8_rn`, `bg` et `hud` sont entiers, mais un minimum ne dit rien des autres valeurs reçues. C'est l'audit qui décide : les chiffres de l'« État » sont ceux qu'il imprime.
- **Numérotation des frames.** D0a a aligné la frame d'audit (`current_frame` du contexte de dispatch) sur celle de `frames_check.sh` et `frames_report.sh`. Un décalage systématique d'une frame entre A et F serait un défaut d'outil. On le rapporte comme un échec de la règle, sans le corriger en cours d'étape.
- **Coût.** L'audit enregistre en plus les sites D, dont `pico8_particle_to_int` et `pico8_hud_to_int`, qui ont chacun des centaines de milliers à des millions de passages. La durée de `make float-audit` et de `make test` est donc mesurée et consignée avant et après.
