---
title: "Plan 2, Phase C1 : conversion strictement préservatrice des 37 casts (a)/(b) à signe prouvé"
description: "Les 37 casts que C0 a affectés à C1 passent de la troncature flottante `(int)pico8_fixed_to_double(x)` à un `flr` entier. Deux portes le vérifient : un `frames-check` identique sans re-baseline, et un build d'audit qui asserte, à chaque site converti et sur tout le filet, que l'ancienne et la nouvelle expression sont égales."
status: needs-triage
---

## Problem Statement

C0 a livré un classement sourcé des 76 casts `(int)pico8_fixed_to_double(…)` du cœur, dans `tools/float_annotation.txt`. 37 d'entre eux ont l'étape **C1** : 36 de population (b), dont le signe ≥ 0 est prouvé par une ligne de justification, et 1 de population (a) dont le signe est prouvé aussi (`pico8_gn_next`, `idx_fixed`, `lua.lua:739`). L'inventaire le confirme (`compte etape C1 37`), tout comme l'observation (les 37 sont couverts et « jamais negatif », 0 contradiction).

Pourtant, ces 37 sites passent encore par la virgule flottante. Chacun appelle `pico8_fixed_to_double`, qui rend un `double`, puis le tronque vers zéro. Ce seul appel suffit à faire échouer le fichier sous la garde `-mgeneral-regs-only` (« SSE register return with SSE disabled »). La garde compte aujourd'hui 32 fichiers en échec sur 58.

La conversion doit rester **strictement préservatrice** (§7 Phase C, C1). Le plan en fait la condition de falsifiabilité du critère §6.4 : si une régression de C1 se mêlait aux divergences légitimes de C2 (16 constantes décalées d'un LSB), personne ne pourrait plus l'isoler. Deux risques précis existent.

1. **Le hash ne voit pas tout.** Un site atteint mais sans effet sur un pixel (une clé de table qui rend la même entrée, une borne de boucle sur un corps invisible) échappe au `frames.hash`. Un `frames-check` vert ne prouve donc pas qu'aucun site n'a changé de valeur.
2. **La preuve de signe n'est que consignée.** Une justification (b) est un raisonnement relu, pas une vérification exécutée. L'observation de C0 est un indice : elle a lu le minimum reçu à chaque ligne sur le filet, mais elle ne portait pas sur l'expression convertie et n'échouait pas.

Il n'existe par ailleurs aucune fonction pour convertir un 16.16 en entier. `pico8_fixed_flr` rend un `pico8_fixed_t` masqué, pas un `int`. Les aides locales `pico8_*_to_int` font toutes la troncature flottante. Le dépôt n'a pas de convention écrite pour le décalage à droite d'un négatif : `pico8_fixed_div` rappelle en commentaire qu'il est défini par l'implémentation, et `pico8_fixed_mul` s'appuie dessus sans le dire.

## Solution

C1 remplace, aux 37 sites C1 et nulle part ailleurs, l'expression `(int)pico8_fixed_to_double(x)` par un appel à une nouvelle fonction du module fixe, **`pico8_fixed_flr_int(x)`**. Elle rend la partie entière de `x`, arrondie vers −∞, dans un `int32_t`. Son nom dit `flr` parce que c'est sa sémantique : elle n'est pas une troncature, même si les deux coïncident sur les 37 sites C1, tous ≥ 0.

La conversion est gardée par **deux portes**, et les deux doivent passer à chaque commit de conversion :

- **Porte 1, le filet.** `make frames-check` est identique sur les 8 fichiers de `FRAMES_EVENTS`. **Aucun `frames-baseline`** : toute divergence est un bug introduit par C1.
- **Porte 2, l'audit.** Une nouvelle cible hors suite, `make float-audit`, compile le rejeu avec un drapeau dédié. Sous ce drapeau, chaque appel de `pico8_fixed_flr_int` calcule aussi l'ancienne expression `(int)pico8_fixed_to_double(x)` et enregistre, par site, les passages, les arguments négatifs et les écarts entre l'ancienne et la nouvelle valeur. La cible rejoue tout le filet puis joint ces enregistrements à la liste statique des appels de `pico8_fixed_flr_int` dans le cœur. Elle échoue s'il y a un seul écart, un seul argument négatif, ou un site jamais atteint.

Dans le même commit que sa conversion, chaque cast converti perd sa ligne d'annotation. L'inventaire le vérifie déjà : une annotation orpheline est une erreur, `float-inventory-check` casserait `make test`. L'inventaire versionné est régénéré, et ses comptes descendent de 37 casts : 76 → 39 casts et 165 → 128 jetons. En fin d'étape, la couverture, l'observation et la garde sont régénérées. La garde doit perdre les fichiers dont les seuls jetons exécutés étaient des casts C1, ce qui se mesure et ne se suppose pas. `docs/plan-2.md` reçoit un « État de C1 ».

## User Stories

1. En tant qu'agent qui mène C1, je veux convertir exactement les 37 casts dont l'annotation porte l'étape C1, pour ne toucher à aucun cast D1 ni « à auditer ».
2. En tant qu'agent, je veux une fonction unique du module fixe qui rende `flr(x)` comme entier, pour que les 37 sites s'écrivent de la même façon et que D1 réutilise la même fonction plus tard.
3. En tant qu'agent, je veux que cette fonction soit définie pour toute valeur 16.16, négatifs et extrêmes compris, sans comportement défini par l'implémentation, pour qu'elle reste juste sur la cible `mipsel` et sous UBSan.
4. En tant qu'agent, je veux que la fonction soit testée sur les valeurs `flr` relevées sur PICO-8 (`tools/probe_out.txt`, bloc B), pour que sa sémantique soit sourcée, conformément à §9 (« aucune valeur numérique non sourcée »).
5. En tant qu'agent, je veux que son nom dise `flr` et non `to_int`, pour qu'on ne la confonde pas avec les aides locales `pico8_*_to_int`, qui tronquent.
6. En tant qu'agent, je veux que `frames-check` reste identique sur les 8 fichiers d'événements après chaque commit de conversion, sans re-baseline, pour que toute divergence désigne une régression de C1.
7. En tant qu'agent, je veux un build d'audit qui asserte, à chaque appel de la nouvelle fonction, que l'ancienne troncature flottante et le nouveau `flr` entier rendent la même valeur, pour attraper un site dont le changement ne toucherait aucun pixel.
8. En tant qu'agent, je veux que l'audit compte aussi, par site, les arguments négatifs reçus, pour que la preuve de signe (b) devienne une vérification exécutée sur tout le filet et plus seulement consignée.
9. En tant qu'agent, je veux que l'audit échoue quand un site converti n'est atteint par aucun rejeu, pour qu'un site non audité ne passe jamais pour un site sain (NON ATTEINT n'est pas « sans écart », comme le NOT COVERED de `ref-audit`).
10. En tant qu'agent, je veux que l'audit trouve les sites par une lecture statique des sources du cœur, et non par la seule liste des sites atteints, pour qu'un site jamais exécuté apparaisse dans le rapport au lieu d'y manquer.
11. En tant qu'agent, je veux que l'audit rejoue exactement `FRAMES_EVENTS`, la même liste que `frames-check`, `float-coverage` et `float-observe`, pour que « tout le filet » ait un seul sens.
12. En tant qu'agent, je veux que l'audit compile dans son propre arbre d'objets et laisse le build par défaut textuellement inchangé, pour que `frames-check` et l'inventaire ne dépendent pas de l'instrumentation.
13. En tant qu'agent, je veux que l'audit reste hors de `make test`, comme `ref-audit`, `float-coverage` et `float-observe`, pour que la suite reste rapide, mais qu'il soit exigé par chaque commit de conversion de C1.
14. En tant qu'agent, je veux que le rapport d'audit s'imprime sur la sortie avec des comptes (sites, atteints, non atteints, écarts, négatifs) et un code de sortie non nul en cas d'échec, pour que la porte se lise sans interprétation.
15. En tant qu'agent, je veux que l'outil de jointure de l'audit soit testé sur des enregistrements fabriqués qui exercent chaque échec (écart, négatif, site non atteint, aucun site), pour que la porte ne puisse pas être verte par construction.
16. En tant qu'agent, je veux retirer la ligne d'annotation de chaque cast converti dans le même commit que sa conversion, pour que `float-inventory-check` reste vert sans laisser d'orpheline.
17. En tant qu'agent, je veux régénérer l'inventaire versionné dans le même commit, pour que `make test` reste vert et que les comptes suivent le code.
18. En tant qu'agent, je veux qu'en fin d'étape l'inventaire affiche `compte etape C1 0`, pour énoncer la sortie de C1 en « compte = 0 » d'après l'inventaire, comme la règle de B2 le prescrit.
19. En tant qu'agent, je veux convertir par groupes de fichiers, chaque groupe passant les deux portes avant le suivant, pour localiser une divergence dans un petit ensemble de sites.
20. En tant qu'agent, je veux qu'un cast placé dans une aide `pico8_*_to_int` (ici `ie_to_int`) soit converti dans le corps de l'aide, sans inliner ni scinder ses appelants, pour ne changer que ce que l'annotation couvre.
21. En tant qu'agent, je veux qu'un cast C1 qui partage sa ligne avec un cast D1 (`pico8_lq_ol_item_draw`) soit converti seul, et que son voisin reste intact, pour ne pas faire entrer une correction D1 dans une étape préservatrice.
22. En tant qu'agent, je veux qu'aucune constante `pico8_fixed_from_double(<littéral>)` ne change en C1, pour garder la frontière C1/C2 que le plan exige.
23. En tant que mainteneur, je veux que `float-coverage`, `float-observe` et `float-guard` soient régénérés en fin d'étape et leurs rapports versionnés, pour que la phase suivante parte de rapports à jour (« relancés en début de chaque phase »).
24. En tant que mainteneur, je veux que la liste des fichiers qui sortent de la garde `-mgeneral-regs-only` soit mesurée et consignée, pour que la décroissance vers la Phase E se lise dans l'historique.
25. En tant que mainteneur, je veux que `docs/plan-2.md` reçoive un « État de C1 » avec les nouveaux comptes (inventaire, observation, couverture, garde) et le résultat des deux portes, pour que C2a parte d'un état consigné.
26. En tant que mainteneur, je veux que la légende en tête du `Makefile` mentionne `make float-audit` (« hors suite »), pour que la cible soit trouvable.
27. En tant qu'agent qui mènera D1, je veux que l'en-tête ou la cible d'audit dise que l'assertion « argument ≥ 0 » est propre aux sites C1, pour que D1 sache qu'il devra la revoir quand des sites (c) appelleront la même fonction.
28. En tant qu'humain, je veux que C1 ne lance jamais l'application raylib, pour que la vérification reste entièrement headless (CLAUDE.md).

## Implementation Decisions

- **La fonction entière.** Le module fixe gagne `pico8_fixed_flr_int(pico8_fixed_t x)`, qui rend un `int32_t` : `flr(x)` en entier, arrondi vers −∞. Il n'existe aujourd'hui aucune aide de ce type : `pico8_fixed_flr` rend un 16.16 masqué.
  - *Définie partout, sans décalage de négatif* : elle se calcule comme `pico8_fixed_flr(x)` divisé par 2^16. Le masque de `pico8_fixed_flr` laisse un multiple exact de 65536, donc la division est exacte et le sens de troncature de `/` n'entre pas en jeu. Le résultat est défini pour tout `int32_t`, de `INT32_MIN` (→ −32768) à `INT32_MAX` (→ 32767). On écarte `x >> 16` : le décalage à droite d'un négatif est défini par l'implémentation (C99 6.5.7p5), `pico8_fixed_div` le rappelle en commentaire, et rien dans le dépôt n'en fait une règle documentée. Le compilateur réduit de toute façon la division en décalage.
  - *Nom* : `flr_int`, pas `to_int`. Les aides locales `pico8_*_to_int` tronquent via le flottant, et le nom doit dire la sémantique (§9 : « une sémantique d'arrondi vient du relevé ou d'un `flr` explicite du Lua, jamais d'un réflexe C »). Sur les 37 sites, troncature et `flr` coïncident, puisque le signe y est ≥ 0. La fonction reste pourtant un `flr` : D1 la réutilisera pour les sites (c), où c'est la correction voulue.
  - Le contrat va dans l'en-tête en une ligne, sur le modèle de celui de `pico8_fixed_flr` : « arrondi vers −∞, pas vers zéro ».

- **La forme convertie.** À chacun des 37 sites, `(int)pico8_fixed_to_double(<arg>)` devient `pico8_fixed_flr_int(<arg>)`. Rien d'autre ne change : ni l'argument, ni le code autour, ni les aides locales. `ie_to_int` garde son nom et ses appelants, et seul son corps change, puisque c'est son cast qui est annoté (C0 : le cast d'une aide est un seul site). Sur la ligne partagée de `pico8_lq_ol_item_draw`, seul `item->sprite_id` est converti ; `pos.x` et `pos.y` (D1) restent tels quels. Le type rendu (`int32_t`) remplace un `int` : c'est le même type sur l'hôte et sur `mipsel`, et le compilateur (`-Wall -Wextra -Werror`) signale toute conversion suspecte.

- **Les 37 sites**, tels que l'annotation les nomme (fichier : nombre) : `pico8_economy.c` 1, `pico8_economy_formulas.c` 2, `pico8_economy_score.c` 5, `pico8_economy_tiers.c` 1, `pico8_entity.c` 1, `pico8_gen.c` 2 (dont le (a) `pico8_gn_next`), `pico8_layout.c` 2, `pico8_msgbox.c` 1, `pico8_pi.c` 7, `pico8_planet.c` 1, `pico8_planet_display.c` 7, `pico8_research_display.c` 1, `pico8_research_icon.c` 1 (`ie_to_int`), `pico8_score_panel.c` 3, `pico8_tech_gate.c` 1, `pico8_wormhole_display.c` 1. La source de vérité est la liste des lignes `| C1 |` de l'annotation, pas ce décompte recopié.

- **Le build d'audit : un drapeau frère, pas une extension de l'observation.**
  - *Pourquoi un frère.* L'observation (`-DPICO8_FLOAT_OBSERVE`) est un relevé. Elle enveloppe `pico8_fixed_to_double`, n'échoue jamais sur une valeur et produit un rapport versionné joint à l'inventaire. L'audit est une porte. Il enveloppe `pico8_fixed_flr_int`, échoue au premier écart et ne produit pas de rapport versionné. Les fusionner changerait l'observation, qui doit rester telle que C0 l'a posée. Deux petits builds sur le même patron sont plus simples qu'un seul build à deux modes.
  - *Drapeau et arbre.* `-DPICO8_FLOAT_AUDIT`, avec son propre arbre d'objets sous `tmp/` (règle « un arbre d'objets par jeu de flags » du `Makefile`) et son propre binaire de rejeu, lié aux mêmes sources que `pico8_replay`, plus l'enregistreur. Comme pour l'observation, `pico8_fixed.c` est pris dans l'arbre par défaut, pour que la macro ne réécrive pas la définition de la fonction.
  - *Instrumentation sans toucher aux sites.* Sous le drapeau seulement, la queue de `pico8_fixed.h` inclut un en-tête d'audit. Cet en-tête remplace `pico8_fixed_flr_int(x)` par un appel d'enregistrement qui reçoit fichier, fonction et ligne. L'appel calcule la nouvelle valeur (la vraie fonction, nom entre parenthèses) et l'ancienne (`(int)` de `pico8_fixed_to_double`). Il enregistre, par (fichier, ligne) : les passages, les arguments négatifs et les écarts entre ancienne et nouvelle valeur, avec le premier argument fautif en hexa brut pour le diagnostic. Puis il rend la nouvelle valeur. Le rejeu audité produit donc les mêmes frames que le build converti. L'enregistreur vit hors du cœur, dans un fichier source à part comme celui de l'observation, et il a le droit au flottant et à la libc (§4 : les outils gardent la libc).
  - *Déversement.* À la sortie, vers un fichier nommé par une variable d'environnement, en mode ajout, sur le modèle de `PICO8_FLOAT_OBSERVE_OUT`. Les rejeux s'additionnent. Un rejeu qui meurt sur un `abort` ne déverse rien : ses sites sortent NON ATTEINT, et la cible échoue déjà sur le code de sortie du rejeu.
  - *Les deux assertions.* (i) Ancienne = nouvelle : c'est l'égalité que le plan demande pour tout site (a)/(b). (ii) Argument ≥ 0 : c'est la forme que le plan donne pour (b). On l'applique aussi au seul (a) de C1, dont l'étape C1 repose précisément sur la preuve de signe (règle de l'annotation : « (a) → C1 si le signe ≥ 0 est prouvé »). Les deux sont comptées séparément. Sur un argument ≥ 0, (i) découle de (ii), mais les compter toutes les deux laisse la porte lisible.
  - *Portée de l'assertion de signe.* Pendant C1, tous les appels de `pico8_fixed_flr_int` sont des sites C1, et l'assertion de signe vaut donc pour tous. Quand D1 fera appeler la même fonction à des sites (c), l'assertion de signe ne vaudra plus partout. L'en-tête d'audit le dit en une ligne, sous forme de piège à ne pas réintroduire (CLAUDE.md). C1 ne construit rien pour D1.
  - *Jointure.* Un script POSIX `sh`/`awk`, sur le modèle de `tools/float_observe.sh` et de `tools/ref_audit_report.sh`, lit la liste statique des appels `pico8_fixed_flr_int(` dans `CORE_SRCS`, hors de la définition dans `pico8_fixed.c`, par (fichier, ligne). Il la joint aux enregistrements et imprime une ligne par site : fonction, passages, négatifs, écarts, et `ok` / `ECART` / `NEGATIF` / `NON ATTEINT`. Viennent ensuite des comptes. Il rend un code non nul si un écart ou un négatif existe, si un site n'est pas atteint, ou si la lecture statique ne trouve aucun site (une porte qui ne voit rien ne passe pas). La granularité est la ligne. Deux appels d'une même ligne partagent leurs compteurs, et le rapport les marque ; un échec sur la ligne fait échouer la porte quel que soit l'appel fautif.
  - *Pas de rapport versionné.* La porte se lit sur la sortie et le code de retour. Le résultat (sites, passages totaux, 0 écart, 0 négatif, 0 non atteint) est consigné dans l'« État de C1 » et dans le message de chaque commit de conversion. Un fichier de plus sous `tools/` ne protégerait rien : la porte n'a de sens qu'au moment du commit.
  - *Cible.* `make float-audit`, hors de `make test`, déclarée `.PHONY`, avec une ligne dans la légende du `Makefile`. Elle rejoue chaque entrée de `FRAMES_EVENTS` depuis la variable elle-même, comme `float-observe`, puis lance la jointure.

- **Annotation et inventaire.**
  - Chaque cast converti perd sa ligne dans `tools/float_annotation.txt` **dans le même commit**. Le mécanisme existant l'impose : une annotation sans cast est une erreur (code 4), et `float-inventory-check` fait échouer `make test`. Aucun changement du script d'inventaire n'est nécessaire. `pico8_fixed_flr_int` ne contient ni `double` ni `float`, et l'appel converti n'est plus un jeton.
  - L'en-tête de l'annotation garde ses règles : elles valent encore pour les casts restants et pour les reclassements futurs des 17 « à auditer » vers C1.
  - `tools/float_inventory.txt` est régénéré (`make float-inventory`) dans chaque commit de conversion. Comptes attendus en fin d'étape, que l'inventaire confirme et qui ne sont pas recopiés à l'avance : `compte cast 39`, `compte total 128`, `compte population (a) 1`, `(b) 0`, `(c) 21`, `a auditer 17`, `compte etape C1 0`, `D1 22`, `a auditer 17`. Constantes (61), réécrits (5) et autres (23) inchangés.

- **Rapports hors suite, en fin d'étape.**
  - `make float-coverage` et `make float-observe` sont régénérés. Leurs lignes suivent l'inventaire, et les 37 casts convertis en disparaissent. Pour l'observation, les comptes attendus sont 76 → 39, dont 56 − 37 = 19 « jamais negatif », sous réserve de ce que la mesure donne. `observation contradiction` doit rester à 0.
  - `make float-guard` est régénéré. Prévision à vérifier, pas à affirmer : sortent de la garde les fichiers dont tous les jetons exécutés étaient des casts C1, c'est-à-dire `pico8_economy_score.c`, `pico8_entity.c`, `pico8_gen.c`, `pico8_msgbox.c`, `pico8_research_display.c`, `pico8_research_icon.c`, `pico8_score_panel.c` et `pico8_tech_gate.c`, soit 32 → 24. Les constantes `from_double(<littéral>)` restantes compilent sous la garde (angle mort consigné en B2), donc elles n'empêchent pas la sortie. La liste mesurée fait foi et va dans l'« État de C1 ». Un fichier prévu qui ne sort pas est un constat à expliquer, pas une raison de modifier le code. Pas de cliquet de garde dans `make test` (décision de C0, inchangée).

- **Consignation dans `docs/plan-2.md`.** Un « État de C1 » s'ajoute après la puce C1 ou après l'« État de C0 », sur leur modèle. Il consigne :
  - les comptes de l'inventaire ;
  - le résultat des deux portes : `frames-check` identique sur 8, et l'audit avec son nombre de sites, 0 écart, 0 négatif, 0 non atteint ;
  - la garde (avant/après, avec la liste des fichiers sortis) ;
  - l'observation et la couverture régénérées.

  Dans la table de propriété, la ligne « casts, étape C1 » est marquée faite, sans modifier le total de C0, qui reste la photographie de 165 jetons. La somme restante (128) est donnée dans l'« État de C1 ».

- **Commentaires.** Les nouveaux fichiers et cibles suivent `CLAUDE.md` : le contrat de la fonction (arrondi vers −∞) ; le piège de l'en-tête d'audit (`pico8_fixed.c` hors du drapeau, assertion de signe propre aux sites C1) ; le contrat du script de jointure (ses codes de sortie) ; l'aide à la lecture des commandes du `Makefile`. Le récit de chantier va dans les messages de commit.

## Implementation Chunks

1. **La fonction `pico8_fixed_flr_int`.** Déclaration et contrat dans l'en-tête du module fixe, définition sans décalage de négatif, tests de valeur. Acceptation : `make test` vert, `frames-check` identique (aucun site n'appelle encore la fonction), inventaire inchangé.
2. **La porte d'audit.** Le drapeau, l'en-tête d'audit inclus en queue de l'en-tête fixe, l'enregistreur, l'arbre d'objets et le binaire, le script de jointure, la cible `float-audit`, la légende du `Makefile`, et le test de la jointure sur entrées fabriquées. Acceptation : le test de jointure est vert dans `make test`, et `make float-audit` sur l'arbre actuel **échoue** avec « aucun site », ce qui est attendu tant qu'aucun cast n'est converti. Ce chunk précède toute conversion : la porte existe et a été vue échouer avant de servir.
3. **Conversions, groupe 1 : économie et tableaux de score.** `pico8_economy.c`, `pico8_economy_formulas.c`, `pico8_economy_score.c`, `pico8_economy_tiers.c`, `pico8_score_panel.c`, `pico8_tech_gate.c`, soit 13 casts, en majorité des clés de table (la classe de sites qui échappe le plus au hash). Même commit : conversion, retrait des 13 annotations, inventaire régénéré. Acceptation : `make test` vert, dont `frames-check` identique sur 8 sans re-baseline, et `make float-audit` vert (13 sites, tous atteints, 0 écart, 0 négatif).
4. **Conversions, groupe 2 : affichage.** `pico8_planet.c`, `pico8_planet_display.c`, `pico8_research_display.c`, `pico8_research_icon.c` (`ie_to_int`), `pico8_msgbox.c`, `pico8_wormhole_display.c`, `pico8_layout.c` (le seul `item->sprite_id` de la ligne partagée, plus `item->color`), soit 14 casts. Même commit et mêmes portes (27 sites audités, cumulés).
5. **Conversions, groupe 3 : constructeurs et génération.** `pico8_pi.c`, `pico8_entity.c`, `pico8_gen.c` (dont le (a) `pico8_gn_next`), soit 10 casts. Même commit et mêmes portes (37 sites audités). Acceptation supplémentaire : l'inventaire affiche `compte etape C1 0` et l'annotation ne contient plus aucune ligne `| C1 |`.
6. **Rapports et consignation.** Régénérer `float-coverage`, `float-observe` et `float-guard`, et versionner leurs rapports. Écrire l'« État de C1 » dans `docs/plan-2.md` et marquer la ligne C1 de la table de propriété. Acceptation : `observation contradiction 0` ; la liste de garde mesurée est consignée avec les fichiers sortis ; les comptes de l'« État » sont ceux des rapports, pas recopiés à la main.

Si une porte échoue sur un groupe, on ne passe pas au groupe suivant et on ne fait pas de re-baseline. Le site fautif se localise par l'audit (ligne en `ECART`/`NEGATIF`) ou par la première frame divergente, et sa preuve de signe est réfutée : le cast repasse par l'annotation (population et étape corrigées, probablement (c)/D1) au lieu d'être converti.

## Testing Decisions

- Un bon test porte sur ce que l'unité rend (une valeur, un rapport, un code de sortie) à partir d'entrées qu'il fabrique. Il exerce d'abord ce que l'unité doit empêcher, et n'inspecte ni les expressions `awk` ni les fichiers intermédiaires.
- **`pico8_fixed_flr_int`** (extension de `tests/test_pico8_fixed.c`) :
  - les dix couples `flr` du bloc B de `tools/probe_out.txt` (`0xfffe.8000 → -2`, `0xffff.8000 → -1`, `0xffff.199a → -1`, `0xffff.0000 → -1`, `0xfffd.8000 → -3`, `0x0001.8000 → 1`, `0x0000.8000 → 0`, `0x0000.e666 → 0`, `0x0001.0000 → 1`, `0x0000.0000 → 0`), recopiés du relevé et jamais dérivés ;
  - les extrêmes `0x7fffffff → 32767` et `0x80000000 → -32768` ;
  - le brut −1 → −1. C'est le cas qu'un `x >> 16` mal fait ou une troncature rendrait 0, comme le fait déjà le test de `pico8_fixed_flr`.

  C'est désormais le premier test du module qui asserte une valeur relevée (§5, « le relevé devient une fixture »). Le test tourne aussi sous l'arbre UBSan existant, s'il couvre `tests/test_pico8_fixed.c`.
- **Jointure d'audit** (nouveau test dans `unit`, sur le patron de `tests/test_float_observe.c`), sur des sources et des enregistrements fabriqués :
  - un site sans écart ni négatif donne `ok` et un code 0 ;
  - un écart donne `ECART` et un code non nul ;
  - un argument négatif donne `NEGATIF` et un code non nul ;
  - un site présent dans les sources mais absent des enregistrements donne `NON ATTEINT` et un code non nul ;
  - des sources sans aucun appel donnent une erreur ;
  - la définition dans `pico8_fixed.c` n'est pas comptée comme un site ;
  - deux appels sur une même ligne sont marqués comme ligne partagée ;
  - les enregistrements de deux rejeux s'additionnent ;
  - deux exécutions donnent la même sortie.
- **L'enregistreur** n'a pas de test unitaire dédié. On l'accepte ainsi : le build d'audit compile ; `make float-audit` échoue sur l'arbre sans site (chunk 2), puis passe avec le nombre de sites attendu après chaque groupe. Précédents : `-DPICO8_FLOAT_OBSERVE` et `-DPICO8_REF_AUDIT`.
- **La conversion** n'a pas de test unitaire par site. Ses deux portes sont `frames-check` (dans `make test`) et `float-audit` (exigé à chaque commit de conversion, résultat cité dans le message de commit). `float-inventory-check` (dans `make test`) vérifie de son côté qu'aucune annotation n'est orpheline et qu'aucun cast restant n'a perdu la sienne.
- Les tests d'outils entrent dans `unit`, donc dans `make test`. Le build d'audit réel en reste dehors, comme `float-observe`, `float-coverage` et `ref-audit`.

## Out of Scope

- **Les casts D1** (21 (c) et le (a) `pico8_mz_kj` `pico8_fixed_flr(ratio)`) et les **17 « à auditer »**. Aucun n'est converti, reclassé ou atteint par une nouvelle fixture. Si C1 réfute une preuve (b), le cast est reclassé et non converti : c'est le seul changement d'annotation permis hors des retraits.
- **Les constantes** `pico8_fixed_from_double(<littéral>)`, les 16 décalées d'un LSB comprises (C2a/C2b).
- **Les réécrits** (`%.4f`, `pow`, `from_double(ec_d)`) et les `pico8_fixed_to_double` hors cast (D2, D3).
- **`pico8_fixed_sin`/`cos`/`rnd`** et les définitions `from_double`/`to_double` : décision ouverte, non tranchée ici.
- **La revue des clés de table fractionnaires (D4).** Les 16 clés C1 sont converties parce que leur population est (b), et D4 garde sa liste.
- **Scinder, renommer ou inliner les aides `pico8_*_to_int`**, y compris `ie_to_int`.
- **Tout `frames-baseline`.**
- **Un cliquet de garde dans `make test`**, la garde `mipsel` soft-float.
- **Le volet `malloc`** (§8).
- **Lancer l'application raylib.**

## Further Notes

- Tous les sites C1 sont couverts par le filet, d'après `tools/float_observe.txt` (passages de 1 à 1 155 213, minimums entre `0x00000000` et `0x00AE0000`). L'exigence « 0 non atteint » de la porte d'audit est donc tenable sans nouvelle fixture. Certains sites ne sont atteints que 1 à 4 fois (`pico8_jo_gh` 1, `pico8_jo_pq` 2, `pico8_jo_gh_jp_step` 2, `pico8_db` 4) : l'audit y vérifie peu de valeurs. C'est une limite du filet, à rappeler dans l'« État de C1 », pas une raison de bloquer.
- `radius_f` (`pico8_jo_kj`) est le seul site C1 dont l'argument observé est fractionnaire (`0x00143C00`). C'est là que l'égalité ancienne/nouvelle porte vraiment sur l'arrondi, et non sur un entier déjà exact.
- `pico8_fixed_mul` décale à droite un `int64_t` possiblement négatif sans le documenter. C'est un constat, hors du périmètre de C1 : on n'y touche pas.
- La garde prévue (32 → 24) est une prévision tirée de l'inventaire. La mesure la remplace dans l'« État de C1 ».
