---
title: "Plan 2, Phase D3 : `^0.75` de pico8_dj passe en fixe pur (sqrt · sqrt∘sqrt), prouvé sur les 16 vecteurs du bloc D, attribué par une paire d'audit ancien pow / nouveau"
description: "pico8_dj calcule (#(to.p-pb.p)/bq)^0.75 par une division double et pow() de la libc, ce qui laisse les 5 derniers jetons flottants hors pico8_fixed.c et garde pico8_economy_formulas.c dans la garde ; D3 le réécrit en pico8_fixed_div puis sqrt(x)·sqrt(sqrt(x)) en 16.16, prouvé au bit près sur les 16 vecteurs du bloc D, mesure ancien ≠ nouveau sur la valeur rendue par une paire d'audit, applique la règle d'attribution de la Phase D, et ne re-baseline que si le filet diverge."
status: needs-triage
---

## Problem Statement

Texte de l'étape, recopié de `docs/plan-2.md` §7 Phase D :

> **D3 — `^0.75`.** Divergente si le nouveau diffère de l'ancien. Constat fait en revue de fin de Phase C : `pico8_fixed_mul(s, pico8_fixed_sqrt(s))` avec `s = pico8_fixed_sqrt(x)` reproduit les 16 vecteurs du bloc D au bit près, et `pico8_fixed_sqrt` les 16 vecteurs `sqrt` du bloc E.

> **Règle d'attribution d'une étape D.** Soit S l'ensemble des sites de l'étape ayant au moins un ancien ≠ nouveau sur le filet (audit), A la plus petite frame de premier ancien ≠ nouveau, F la première frame divergente de `tools/frames_reports/<étape>.txt`.
> 1. **A > F, ou F sans A : échec.** Un pixel a bougé avant toute valeur auditée.
> 2. **Complément neutre :** l'arbre de l'étape, les seuls sites de S ramenés à l'ancienne forme, donne `frames-check` identique à la référence d'avant l'étape sur tous les fichiers, sans `frames-baseline`. Cela prouve que les sites hors S (dont les corrections non vues) ne changent aucun pixel — y compris un glissement d'argument que l'audit ne peut pas voir.
> 3. **Variante suffisante :** l'arbre d'avant l'étape, les seuls sites de S convertis, donne des empreintes identiques à celles de l'arbre de l'étape, sur toutes les frames. Cela prouve l'absence d'interaction.
> 4. Si 2 et 3 passent, toutes les frames divergentes sont attribuées à S, nommé. A < F est permis et consigné (« effet différé de A à F ») ; la revue sans fenêtre à F est une confirmation, pas la porte.
> 5. Si S a plus d'un site et qu'on veut les départager, on répète 2 et 3 site par site ; sinon on consigne « attribution partagée : S ».
>
> Le contrôle d'absence de cascade sur l'état (enregistrements C1 de l'audit identiques avant/après, plus lecture du code) est maintenu : il répond à une autre question.

> **Corrections invisibles au filet :** pour chaque étape D, les sites à zéro appel ancien ≠ nouveau sur le filet sont listés « correction non vue par le filet » (comme les 13 constantes de C2b). Chacun reçoit un test au niveau valeur de l'expression convertie (par exemple une entrée négative fractionnaire qui doit donner `flr`) ou une entrée explicite dans ce plan. Critère de couverture, pas de ligne `gcov` : C a montré que NON COUVERT ne prouve rien dans un sens comme dans l'autre.

> **Sortie de phase :** pour chaque étape D divergente, toutes les frames divergentes sont attribuées à S par les contrôles complément/variante de la règle ci-dessus ; F, A et l'écart A→F consignés dans `<étape>-attribution.txt` ; aucune cascade sur l'état (enregistrements C1 de l'audit identiques avant/après), et le rapport de divergence est versionné avant le re-baseline ; `frames.hash` re-baseliné **volontairement**, avec revue des frames concernées — jamais subi comme une régression. D2 ne re-baseline pas.

Sources de la formule : §5 bloc D (`x^0.75 = sqrt(x) · sqrt(sqrt(x))`, « deux `sqrt` et une multiplication, tout en fixe. Les 16 vecteurs servent de test »), décision actée 16, risque §10 (« le `pow(^0.75)` en fixe n'est pas reproductible à partir de 16 points témoins » : accepté, statut actuel).

Aujourd'hui, `pico8_dj` (le coût d'un slipway, `dj` de `lua.lua:1259-1261` : `ec=(#(to.p-pb.p)/bq)^0.75`, `flr(bn*bk()*max(0.5,ec))`) calcule `ec` ainsi : `len_sq` et `bq` passent en `double`, sont divisés **en double**, élevés à `0.75` par `pow()` de la libc, et le résultat revient en 16.16 par `pico8_fixed_from_double`, qui **arrondit au plus proche**. Conséquences :

1. **Cinq jetons flottants restent dans `pico8_economy_formulas.c`.** L'inventaire (`tools/float_inventory.txt`) les attribue à D3 : `pico8_dj` `autre` 1 (`double`), `reecrit` 1 (`pow`), `autre` 2 et 3 (`pico8_fixed_to_double` ×2), `reecrit` 2 (`pico8_fixed_from_double(ec_d)`). La table de propriété de la Phase C les affecte à D3.
2. **`pico8_economy_formulas.c` est l'un des 2 fichiers encore en échec dans la garde** `-mgeneral-regs-only` (`tools/float_guard.txt`), avec `pico8_fixed.c` (D5, E). Il n'y est que pour `pico8_dj`.
3. **Le calcul actuel n'est pas celui de PICO-8, deux fois.** (a) En Lua, `/` est la division 16.16 : le quotient est tronqué **avant** la puissance, alors que le C divise en double exact. (b) PICO-8 calcule `^0.75` comme deux `sqrt` 16.16 tronquées et une multiplication 16.16 tronquée (bloc D), pas comme `pow` réel arrondi au plus proche. Mesuré en cadrage (prototype hors dépôt, `bq` = `0x0000.1902`) : sur les 524 289 valeurs brutes de `len_sq` de 0 à `0x0008.0000`, l'ancien `ec` diffère du nouveau sur **524 012**. L'écart est d'un ou quelques LSB : après `flr(bn*bk*max(0.5,ec))`, la valeur **rendue** diffère sur 261 entrées sur 524 289 à `bk` = 1 (première à `len_sq` = `0x0000.1eb7`), et sur 447 à 40 slipways.
4. **`pico8_dj` nourrit l'état du jeu**, pas seulement l'affichage : sa valeur est stockée dans le champ `od` du slipway en cours de tracé (`pico8_slipway.c`, `jc:hq`), qui est un coût. Si une valeur rendue change sur le filet, l'économie peut suivre un autre chemin, contrairement à D1 où chaque site ne nourrissait qu'une primitive de dessin.
5. **Couverture** : `pico8_dj` est atteint par le filet, 23 passages (`tools/float_coverage.txt`, lignes des 5 jetons). Vu la mesure du point 3, on s'attend à ce que ses 23 valeurs rendues soient identiques ancien et nouveau ; ce n'est pas supposé, c'est mesuré par l'audit.

## Solution

`pico8_dj` calcule `ec` en 16.16 pur, comme le cart : `ec = pow075(pico8_fixed_div(len_sq, bq))`, où `pow075(x) = pico8_fixed_mul(s, pico8_fixed_sqrt(s))` avec `s = pico8_fixed_sqrt(x)`. Le reste de la fonction (`max(0.5, ec)`, `bn*bk`, `flr`) ne change pas. Plus de `double`, de `pow`, ni de `from_double`/`to_double` dans `pico8_economy_formulas.c`.

La preuve de valeur est un test unitaire sur les 16 vecteurs du bloc D, plus les 16 vecteurs `sqrt` du bloc E. La mesure sur le filet est une paire d'audit ancien/nouveau branchée sur `pico8_float_audit_record`, **au niveau de la valeur rendue par `pico8_dj`**. Puis on applique la règle d'attribution de la Phase D. Deux issues :

- **Le filet ne diverge pas** (attendu, cf. point 3 et 5) : `frames-check` identique sur les 12 fichiers de `FRAMES_EVENTS`, S vide, `pico8_dj` listé « correction non vue par le filet », protégé par le test des 16 vecteurs. **Aucun `frames-baseline`.** `D3.txt` (rapport « conforme » partout) et `D3-attribution.txt` sont versionnés quand même, pour que la sortie de l'étape se lise sans rien relancer.
- **Le filet diverge** : S = {`pico8_dj`}, A et F lus par fichier, contrôles 1 à 3, cascade (voir la décision ci-dessous), revue sans fenêtre à F, `D3.txt` et `D3-attribution.txt` versionnés **avant** `make frames-baseline ETAPE=D3`.

Le travail se fait en trois commits (voir Implementation Chunks) : tests et aide `pow075` d'abord (neutre) ; paire d'audit, réécriture de `pico8_dj`, attribution et re-baseline éventuel ; consignation.

## User Stories

1. En tant qu'agent qui mène D3, je veux que `pico8_dj` n'utilise plus ni `double`, ni `pow`, ni `pico8_fixed_to_double`, ni `pico8_fixed_from_double`, pour que les 5 jetons D3 disparaissent de l'inventaire.
2. En tant qu'agent, je veux que `pico8_economy_formulas.c` compile sous `-mgeneral-regs-only`, pour qu'il sorte de `tools/float_guard.txt` (2 → 1 fichier en échec : `pico8_fixed.c`).
3. En tant qu'agent, je veux que `^0.75` soit calculé comme PICO-8 le fait d'après le bloc D, `sqrt(x)·sqrt(sqrt(x))` en 16.16, pour que la formule soit sourcée par le relevé et non par la libc.
4. En tant qu'agent, je veux un test qui asserte les 16 vecteurs `## d pow075` de `tools/probe_out.txt` au bit près, pour que la fidélité de `^0.75` soit prouvée au niveau valeur à chaque `make test`.
5. En tant qu'agent, je veux un test qui asserte les 16 vecteurs `## e sqrt` au bit près sur `pico8_fixed_sqrt`, pour que la brique dont dépend `^0.75` ait elle aussi une assertion sourcée (aujourd'hui elle n'a que des contrats : `4 → 2`, `0.25 → 0.5`, 0, négatif).
6. En tant qu'agent, je veux que les vecteurs soient recopiés du relevé tels quels (brut en hexa), jamais dérivés, pour suivre la règle de traçabilité du §9 et le motif de `test_flr_int_matches_probe_block_b`.
7. En tant qu'agent, je veux que la division `#d/bq` se fasse par `pico8_fixed_div`, comme le `/` 16.16 de Lua, pour supprimer la division double qui était un accident de portage au même titre que les casts `(int)`.
8. En tant qu'agent, je veux que `^0.75` vive dans une fonction pure de `pico8_fixed`, à côté de `pico8_fixed_sqrt`, pour qu'elle soit testable sans instance, sans `cg` ni `bk()`.
9. En tant qu'agent, je veux que cette fonction rende 0 pour 0 et un négatif, pour que le cas `pb.p == to.p` (quotient nul, `max(0.5, 0)` = 0.5) reste celui d'aujourd'hui, et que son contrat sur un négatif soit écrit.
10. En tant qu'agent, je veux que la réécriture ne touche que la ligne de `ec` dans `pico8_dj`, pour que `max(0.5, ec)`, `bn*bk` et `flr` restent tels quels et que le diff soit minimal.
11. En tant qu'agent, je veux une paire d'audit ancien/nouveau branchée sur `pico8_float_audit_record`, comme D0a l'annonçait pour D3, pour que S et A soient mesurés sur le filet et non supposés.
12. En tant qu'agent, je veux que la paire compare la **valeur rendue** par `pico8_dj` (ancienne expression complète : division double, `pow`, `from_double`, puis la même fin), pour que « ancien ≠ nouveau » signifie « l'état du jeu reçoit une autre valeur » et non « un LSB de `ec`, écrasé par `flr` ».
13. En tant qu'agent, je veux que l'argument enregistré soit le quotient 16.16 `len_sq/bq`, pour que « premier argument fautif » et « fractionnaires » décrivent l'entrée de `^0.75`.
14. En tant qu'agent, je veux que l'ancienne expression n'existe que dans le build d'audit, dans `pico8_float_audit.c`, pour que le cœur ne garde aucun `double` même sous `-DPICO8_FLOAT_AUDIT`.
15. En tant qu'agent, je veux que `tools/float_audit.sh` lise l'enregistrement D3 comme un site D sans lecture statique, ce qu'il fait déjà (« paires D2, D3 »), pour ne pas modifier l'outil de rapport.
16. En tant qu'agent, je veux appliquer la règle d'attribution telle qu'elle est écrite : A > F ou F sans A = échec, complément neutre, variante suffisante, pour que D3 soit jugée comme D1.
17. En tant qu'agent, je veux que, si S est vide et le filet identique, `pico8_dj` soit listé « correction non vue par le filet » avec son test de valeur nommé, pour tenir « Corrections invisibles au filet ».
18. En tant qu'agent, je veux que, si le filet diverge alors que S est vide, l'étape s'arrête (F sans A), pour qu'un pixel qui bouge sans valeur auditée ne soit jamais re-baseliné.
19. En tant qu'agent, je veux que `make frames-baseline ETAPE=D3` ne soit lancé que si au moins un fichier diverge et que les contrôles passent, pour ne jamais re-baseliner à vide ni subir une régression.
20. En tant qu'agent, je veux que `D3.txt` soit versionné avant tout re-baseline, pour tenir la sortie de phase.
21. En tant qu'agent, je veux un `D3-attribution.txt` au format de `D1-attribution.txt`, pour que S, A, F, A→F, les contrôles 2 et 3, la cascade et la revue se lisent au même endroit que pour D1.
22. En tant qu'agent, je veux une revue sans fenêtre à F sur chaque fichier divergent (dump de la frame F avant, après et variante), en confirmation, pour savoir quels pixels bougent (typiquement un coût affiché ou un chemin d'économie différent).
23. En tant qu'agent, je veux que le contrôle d'absence de cascade soit appliqué selon une règle écrite pour D3, puisque `pico8_dj` nourrit l'état, pour que l'écart avec la lettre de la sortie de phase soit une décision tracée et non un contournement.
24. En tant qu'agent, je ne veux jamais lancer le binaire raylib, pour respecter `CLAUDE.md` : toute vérification passe par le rejeu headless et le dump texte.
25. En tant qu'agent, je veux régénérer l'inventaire dans le commit de réécriture, pour que `float-inventory-check` reste vert (réécrits 4 → 2, autres 21 → 18, total 25 → 20 ; `pico8_economy_formulas.c` n'y figure plus).
26. En tant qu'agent, je veux régénérer garde et couverture au commit de consignation, pour que les fichiers versionnés reflètent l'état après D3.
27. En tant qu'agent, je veux que `#include <math.h>` sorte de `pico8_economy_formulas.c` s'il ne sert plus, pour ne pas laisser un orphelin créé par la réécriture.
28. En tant que relecteur humain, je veux un « État de D3 » dans le plan (comptes d'inventaire, garde, couverture, audit, attribution, coût de `make test`), pour lire la sortie de l'étape sans rien relancer.
29. En tant que relecteur, je veux que la ligne D3 de la table de propriété et la puce D3 soient marquées faites, pour que la table reste la source des comptes.
30. En tant que relecteur, je veux que le risque §10 « `pow(^0.75)` pas reproductible à partir de 16 points » soit mis à jour avec ce qui est désormais su (16/16 au bit près, hors de ces points la formule est celle du relevé, pas une approximation), pour que le risque ne reste pas écrit au passé.
31. En tant que relecteur, je veux que TROU-1 cesse de citer `^0.75` de `dj()` et `pico8_economy_formulas.c`, et que le `/* TROU-1 */` de `pico8_dj` parte, pour qu'aucun trou de portage ne décrive un écart refermé.
32. En tant que mainteneur futur (Phase E), je veux que plus rien dans `pico8_economy_formulas.c` n'appelle `pico8_fixed_to_double`/`from_double`, pour que leurs définitions puissent partir en E.

## Implementation Decisions

- **Formule : `pow075(x) = mul(s, sqrt(s))`, `s = sqrt(x)`, en 16.16 tronqué.** Source : bloc D (§5), décision 16, constat de revue de fin de Phase C. Revérifié en cadrage de cette PRD par un prototype hors dépôt : **16/16** vecteurs du bloc D au bit près avec `pico8_fixed_sqrt` et `pico8_fixed_mul` actuels. La preuve qui compte est le test livré.
- **Où vit `pow075` : `pico8_fixed`, nouvelle fonction publique** (par exemple `pico8_fixed_pow075`), à côté de `pico8_fixed_sqrt`. Raisons : elle n'est que de l'arithmétique 16.16 ; elle se teste sans instance ni état global ; et `pico8_fixed.c` est compilé **sans** `-DPICO8_FLOAT_AUDIT`, donc aucun conflit avec le build d'audit. Contrat écrit dans l'en-tête : rend `x^0.75` à la manière de PICO-8 (deux `sqrt` tronquées, un produit tronqué) ; 0 → 0 ; négatif → 0, par `pico8_fixed_sqrt` (réponse défensive du port, non sourcée, comme celle de `sqrt`). Pas de variante générique `pow(x, y)` : le cart n'a qu'une puissance non entière.
- **Division : `pico8_fixed_div(len_sq, bq)`.** C'est le `/` de Lua, déjà utilisé partout dans le port. Le passage en double était un accident de portage (division exacte là où le cart tronque). La règle d'arrondi de la division PICO-8 n'est pas dans le relevé ; elle n'est pas nouvelle pour D3 (le port entier en dépend), donc aucun trou de portage n'est ouvert pour elle ici.
- **Domaine de `x` vu par `pico8_dj`**, établi depuis `lua.lua` :
  - `len_sq = #(to.p-pb.p)` est `rg` du vecteur avec lui-même (`lua.lua:119-120`), une somme de carrés : ≥ 0 tant que le produit ne déborde pas. `p` vaut `ra/128` (`lua.lua:552`, et `{ra=mp,p=mp/128}` pour le point provisoire du tracé, `lua.lua:622`), donc une composante de `p` n'atteint 128 (débordement du carré) qu'à 16 384 pixels : hors de toute galaxie.
  - `x = len_sq/bq` avec `bq = 0.0977` (`0x0000.1902`, relevé) déborde au-delà de `len_sq` ≈ 3 201, soit une distance d'environ 56 unités (7 200 pixels) : hors d'atteinte aussi.
  - `x = 0` quand les deux extrémités coïncident : ancien et nouveau rendent `ec = 0`, puis `max(0.5, 0)` = 0.5. Inchangé.
  - Donc : `x ≥ 0` sur tout chemin du jeu. Pas d'assertion ajoutée dans `pico8_dj` (scénario impossible, pas de gestion d'erreur pour lui) ; le contrat « négatif → 0 » de `pow075` suffit.
- **Paire d'audit : une seule, au niveau de la valeur rendue par `pico8_dj`.** (Décision prise par la guideline, à défaut d'humain.)
  - Nouveau : la valeur rendue par `pico8_dj` réécrite. Ancien : la même fin (`max(0.5, ·)`, `bn*bk`, `flr`) appliquée à l'ancien `ec` = `from_double(pow(to_double(len_sq)/to_double(bq), 0.75))`. Argument : le quotient 16.16 `pico8_fixed_div(len_sq, bq)`.
  - Pourquoi pas au niveau de `ec` : le prototype montre que l'ancien `ec` diffère du nouveau sur 99,9 % des entrées, d'un ou quelques LSB, que `flr` efface presque toujours. Un audit sur `ec` marquerait A au premier appel de `pico8_dj` sur chaque fichier, sans rapport avec un pixel. Seule la valeur rendue entre dans l'état (`od`), donc c'est elle qui départage « correction vue » et « correction non vue ».
  - Implantation : l'ancienne expression vit dans `pico8_float_audit.c` (build d'audit seulement, qui garde la libc), exposée par une fonction qui prend `len_sq` et `bq` et rend l'ancien `ec` en 16.16. `pico8_dj` l'appelle dans un bloc `#ifdef PICO8_FLOAT_AUDIT` de quelques lignes, qui calcule l'ancienne valeur rendue et appelle `pico8_float_audit_record(__FILE__, __func__, __LINE__, quotient, ancien != nouveau, 0)`. Hors audit, `pico8_economy_formulas.c` ne voit ni `double` ni `math.h`. Le motif macro de `pico8_fixed_flr_int` ne convient pas ici : il ne verrait que le quotient, pas l'ancienne division double.
  - **Colonne « nouveau ≠ référence » : toujours 0 pour D3.** Il n'y a pas de référence indépendante sur le filet : la vérité de `^0.75` est le relevé, et elle est assertée par le test des 16 vecteurs, pas par l'audit. La porte D du rapport (`NOUVEAU != FLR`) ne peut donc pas échouer sur ce site. C'est écrit dans l'en-tête de `pico8_float_audit.h` (qui mentionne déjà D3) et dans l'« État de D3 ». `tools/float_audit.sh` ne change pas.
  - Clé de l'enregistrement : (fichier, ligne) de l'appel à `pico8_float_audit_record` dans `pico8_dj`, un site D. Si D1 et D3 étaient mesurés ensemble, les 29 sites D1 restent tels quels : le rapport compte 30 sites D.
- **Règle d'attribution appliquée à D3.** S ⊆ {`pico8_dj`} : c'est le seul site de l'étape. Conséquence consignée dans `D3-attribution.txt` :
  - Contrôle 1 : lu tel quel, par fichier, sur l'audit et `D3.txt`.
  - Contrôle 2 (complément neutre) : « S ramené à l'ancienne forme » = l'arbre d'avant D3 (plus l'aide `pow075` et ses tests, qui ne changent aucun pixel). On le **lance quand même** : `frames-check` sur l'arbre du commit 1 (tests et aide seulement), contre les références d'avant D3. Il doit être conforme 12/12 ; c'est la preuve que le commit neutre l'est.
  - Contrôle 3 (variante suffisante) : « arbre d'avant, S converti » = l'arbre de D3, au bloc d'audit près, qui ne change aucune valeur rendue. Il est **satisfait par construction** et consigné comme tel ; on ne fabrique pas d'arbre de variante.
  - Point 5 sans objet (S a au plus un site).
- **Absence de cascade sur l'état, pour D3.** (Décision prise par la guideline, à défaut d'humain.) La lettre de la sortie de phase (« enregistrements C1 de l'audit identiques avant/après ») suppose que l'étape ne touche que l'affichage, ce qui valait pour D1. `pico8_dj` écrit un coût dans l'état. La règle retenue :
  - **Sur chaque fichier sans A** (aucune valeur rendue différente) : les enregistrements C1 d'avant D3 se retrouvent **à l'identique** dans l'arbre D3. Toute différence est un échec : l'état a bougé sans valeur auditée.
  - **Sur chaque fichier avec A** : les enregistrements C1 peuvent différer à partir de A, puisque l'économie peut suivre un autre chemin. On mesure et on consigne le nombre de sites C1 dont les compteurs changent. Ce n'est pas une cascade au sens du plan (un site hors S qui changerait de valeur), c'est l'effet attendu de S sur l'état. La porte C1 de `float-audit` (0 écart, 0 négatif, 0 non atteint) reste, elle, dure sur tous les fichiers.
  - L'écart avec la lettre de la sortie de phase est écrit dans l'« État de D3 » et dans la sortie de phase (une phrase : « pour une étape qui écrit dans l'état, l'identité C1 n'est exigée que sur les fichiers sans A »).
  - Si S est vide, le premier cas s'applique à tous les fichiers, et la vérification redevient celle de D1.
- **Revue sans fenêtre à F**, en confirmation, pour chaque fichier divergent : dump texte de la frame F sur l'arbre d'avant et sur l'arbre D3 (outil de dump de frame de `frames_check`/`frames_report`), lignes et boîte des pixels changés en indices 0..127, et lecture de ce qui est dessiné là (coût affiché du slipway, crédits, autre chemin). Format de `D1-attribution.txt`.
- **Re-baseline.** `make frames-report ETAPE=D3` est lancé dans tous les cas et `tools/frames_reports/D3.txt` est versionné (« conforme » partout si rien ne diverge). `make frames-baseline ETAPE=D3` seulement si au moins un fichier diverge et que les contrôles passent. Rien ne diverge : pas de baseline.
- **Inventaire après D3.** Les 5 jetons de `pico8_dj` disparaissent : réécrits 4 → 2 (les `sin`/`cos` de `pico8_fixed.c`, D5), autres 21 → 18, total 25 → 20. `pico8_economy_formulas.c` n'a plus aucune ligne. Le fichier d'annotation ne change pas (aucun cast). Les nouveaux jetons éventuels vus par l'inventaire dans `pico8_fixed.c` (aucun attendu : `pow075` n'utilise ni `double` ni `from_double`) seraient un constat à consigner.
- **Garde après D3.** On attend que `pico8_economy_formulas.c` sorte de `tools/float_guard.txt` (2 → 1 : `pico8_fixed.c`). On le vérifie en compilant sous la garde, on ne le suppose pas.
- **Commentaires et TROU-1** (règle de `CLAUDE.md`). TROU-1 (`docs/trous-de-portage.md`) cite aujourd'hui « l'exposant fractionnaire `^0.75` de `dj()` » et liste `src/pico8_economy_formulas.c` dans ses fichiers. Une fois `^0.75` sourcé par le bloc D, ce n'est plus un écart connu : la phrase et le fichier sortent de TROU-1 (qui garde `sin`/`cos`, D5), et le `/* TROU-1 */` au-dessus de `ec` dans `pico8_dj` part avec eux. Le contrat de `pow075` dans `pico8_fixed.h` tient en une ligne (formule du bloc D, 0 et négatif → 0). L'en-tête de `pico8_dj` (« `#(to.p-pb.p)` est la distance au carré ») reste. La note « fidélité numérique fractionnaire » que le risque §10 attribue à `pico8_economy_formulas.h` n'y existe plus (reprise des commentaires) : rien à aligner dans l'en-tête, seul le risque §10 est réécrit.
- **Aucun autre site ne change.** L'unique appelant (`pico8_slipway.c`, `jc:hq`) garde son code.

## Implementation Chunks

1. **Tests et aide `pow075` (commit neutre).** Test des 16 vecteurs `sqrt` du bloc E sur `pico8_fixed_sqrt` (vert tout de suite) ; test des 16 vecteurs du bloc D sur la nouvelle `pico8_fixed_pow075` (rouge, puis vert en l'écrivant) ; contrats 0 et négatif. `pico8_dj` n'est pas touché. `make test` vert, `frames-check` identique : c'est aussi le contrôle 2 (complément neutre) de la règle.
2. **Paire d'audit et réécriture de `pico8_dj`.** Ancienne expression exposée dans `pico8_float_audit.c`, bloc d'audit dans `pico8_dj`, ligne `ec` réécrite (`pico8_fixed_div` + `pow075`), `math.h` retiré s'il est orphelin, `/* TROU-1 */` retiré et TROU-1 mis à jour, inventaire régénéré. `make float-audit` : relever S, A, les appels et fractionnaires du site `pico8_dj`. `make frames-report ETAPE=D3`. Attribution (contrôles 1 à 3, cascade selon la règle D3, revue à F si divergence) écrite dans `tools/frames_reports/D3-attribution.txt`. Puis, seulement si divergence attribuée : `make frames-baseline ETAPE=D3`. `make test` vert. `D3.txt` et `D3-attribution.txt` dans ce commit.
3. **Consignation.** `make float-guard` et `make float-coverage` régénérés. Dans `docs/plan-2.md` : « État de D3 », puce D3 ✅, ligne D3 de la table de propriété, phrase de la sortie de phase sur la cascade pour une étape qui écrit dans l'état, risque §10 `^0.75` mis à jour. TROU-1 est modifié dans le commit 2, avec le retrait du `/* TROU-1 */` de `pico8_dj`.

Si le test des 16 vecteurs ne passe pas avec `pico8_fixed_sqrt`/`pico8_fixed_mul` actuels, ou si le contrôle 1 échoue (F sans A, A > F), ou si un fichier sans A voit ses enregistrements C1 changer, l'étape s'arrête : pas de re-baseline, pas de commit de la réécriture, constat consigné dans le plan.

## Testing Decisions

- **Un bon test ici regarde la valeur 16.16 rendue** pour une entrée brute donnée, et la compare au relevé. Il ne regarde pas les intermédiaires (`s`, `sqrt(s)`), ni la façon dont le produit est calculé.
- **Test des 16 vecteurs du bloc D**, dans la suite `pico8_fixed` (lancée par `make unit`, donc par `make test`) : 16 couples (brut d'entrée → brut attendu) **recopiés** de `tools/probe_out.txt` `## d pow075`, de `0x0000.4000 → 0x0000.5a82` à `0x0080.0000 → 0x0026.0df0`, avec le commentaire de provenance du test du bloc B (« copied from the probe, never derived »). Égalité exacte sur chaque couple. On recopie plutôt que d'analyser le fichier à l'exécution : c'est le motif existant (`test_flr_int_matches_probe_block_b`) et il évite une dépendance du test à un chemin de fichier.
- **Test des 16 vecteurs `sqrt` du bloc E** sur `pico8_fixed_sqrt`, même forme, `## e sqrt` (`0x0000.4000 → 0x0000.8000` … `0x0080.0000 → 0x000b.504f`). Il fait de la brique de `^0.75` une fonction sourcée, et non plus seulement contractuelle.
- **Contrats de `pow075`** : 0 → 0, un négatif → 0, 1 → 1 (déjà dans les vecteurs), 16 → 8 (idem).
- **Pas de test de `pico8_dj` au niveau valeur rendue.** Il faudrait monter deux instances, `cg.jc` et `bk()` ; la seule ligne changée est `ec`, protégée par les tests ci-dessus. Le filet (23 passages) et l'audit couvrent l'intégration. Si `pico8_dj` sort « correction non vue par le filet », sa protection nommée dans le plan est le test des 16 vecteurs.
- **Audit** : `make float-audit` vert (dans `make test`), avec un site D de plus. Le test d'outil `tests/test_float_audit.c` ne change pas, puisque `tools/float_audit.sh` ne change pas. On vérifie juste à la main, dans l'enregistrement, qu'une ligne `AUD` de `pico8_dj` apparaît sur les fichiers qui l'atteignent.
- **Filet** : `frames-check` sur les 12 fichiers, avant (commit 1) et après (commit 2). Si divergence, rapport, attribution et re-baseline comme décrit.
- **Prior art** : `tests/test_pico8_fixed.c` (`test_flr_int_matches_probe_block_b`, `test_lit_is_the_raw_probe_value`, les contrats `sqrt` existants) ; `tools/frames_reports/D1-attribution.txt` pour le format d'attribution ; la paire `pico8_fixed_flr_int` de `pico8_float_audit.c` pour l'appel de `pico8_float_audit_record`.
- **Jamais de fenêtre** : aucune vérification ne lance le binaire raylib ; la revue à F passe par le dump texte.

## Out of Scope

- Une puissance générique `pow(x, y)` en fixe : le cart n'a qu'une puissance non entière, et les quatre autres `^` sont déjà des multiplications (§3.3).
- La règle d'arrondi de la division PICO-8 (non relevée) et toute relance de la sonde.
- Changer `bn`, `bq`, `bk()`, `max(0.5, ·)` ou le `flr` final de `pico8_dj`.
- D4 (clés de table), D5 (`sin`/`cos` sur table), la Phase E (retrait des définitions `from_double`/`to_double`, `-lm`), les jetons de commentaires et les « lignes sans code » de l'inventaire.
- Modifier `tools/float_audit.sh` ou son test : l'outil lit déjà les paires D3.
- L'outil d'observation, arrêté depuis D1 (code 2) : constat laissé à E.
- Le volet `malloc` (§8), dont les `malloc` du point provisoire de `jc:hq`.
- L'audio, et tout lancement de l'application raylib.

## Further Notes

- **Prototype de cadrage** (bac à sable, non versionné), `pico8_fixed_sqrt`/`_mul`/`_div` du dépôt, `bq` = `0x1902`, `bn` = 6 :
  - bloc D : 16/16 au bit près ;
  - `ec` ancien ≠ nouveau : 524 012 sur 524 289 valeurs de `len_sq` de 0 à `0x0008.0000`. En gardant la division 16.16 mais `pow` de la libc, il en reste 523 321 : l'écart vient surtout de la formule, un peu de la division ;
  - valeur rendue ancien ≠ nouveau : `bk` = 1 (0 à 15 slipways) 261 sur 524 289, première à `len_sq` = `0x0000.1eb7` ; 20 slipways 283 ; 30 slipways 372 ; 40 slipways 447.
  Sur 23 appels du filet, une divergence est donc peu probable, mais possible. Ces chiffres guident la PRD ; ils ne remplacent pas l'audit.
- **Hors des 16 points**, la nouvelle `^0.75` n'est pas une approximation : c'est la formule que le relevé désigne, appliquée avec les primitives 16.16 du port. Sa fidélité hors des points dépend seulement de celle de `pico8_fixed_sqrt` (16/16 sur le bloc E, troncature d'une racine entière exacte) et de `pico8_fixed_mul` (troncature du produit). À écrire ainsi dans le risque §10.
- **Écart avec la consigne reçue** : la demande parlait de vecteurs « analysés depuis `probe_out.txt` ». La PRD les recopie, comme le fait le test du bloc B ; le choix est dans Testing Decisions.
- **Couverture** : les 5 jetons ont 23 passages chacun (`tools/float_coverage.txt`). Après D3, ils disparaissent de la couverture avec l'inventaire.
