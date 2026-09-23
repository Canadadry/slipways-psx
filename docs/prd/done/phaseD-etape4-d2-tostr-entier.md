---
title: "Plan 2, Phase D2 : pico8_value_tostr passe en arithmétique entière, prouvé identique à l'ancien %.4f par un test exhaustif"
description: "pico8_value_tostr passe un 16.16 par un double et snprintf(\"%.4f\"), ce qui laisse 3 jetons flottants dans le cœur et garde pico8_value.c dans la garde ; D2 le réécrit en arithmétique entière (fraction × 10 000, arrondi au pair comme printf), prouve par un test d'hôte exhaustif que la sortie est identique octet pour octet à l'ancienne, puis constate frames-check identique, sans re-baseline."
status: needs-triage
---

## Problem Statement

Texte de l'étape, recopié de `docs/plan-2.md` §7 Phase D :

> **D2 — `tostr`.** **Préservatrice, pas divergente** : le `tostr` actuel reproduit déjà 13 témoins sur 14 et le 14ᵉ est conservé tel quel. Porte : (i) un test d'hôte qui compare ancien et nouveau sur les 65 536 parties fractionnaires, des deux signes, et quelques parties entières dont ±32767/−32768 — seule différence admise, le cas `< 0.0001` ; attention aux 16 égalités exactes où `%.4f` arrondit au pair ; (ii) `frames-check` identique, **sans** `frames-baseline`.

La phrase source de l'introduction de la Phase D : « `pico8_value_tostr` réécrit en entier d'après le bloc C, le cas `< 0.0001` gardé sur le comportement actuel **et documenté comme non sourcé** ».

Et l'état du bloc C (§5) : `pico8_value_tostr` (`%.4f`, trim des zéros, trim du point) reproduit exactement PICO-8 sur 13 des 14 témoins, `2/3 → "0.6667"` et `1/3 → "0.3333"` compris. Le seul écart est `-0.0001` (brut −6) : PICO-8 affiche `"-0"`, le C affiche `"-0.0001"`. Aucune règle unique ne réconcilie ce cas avec `2/3 → "0.6667"`, donc il n'est pas deviné. Il ne bloque pas : aucune valeur affichée par le jeu n'a une magnitude inférieure à 0.0001.

Aujourd'hui, la branche `FIXED` de `pico8_value_tostr` convertit le 16.16 en `double` par `pico8_fixed_to_double`, l'imprime avec `snprintf("%.4f")`, puis élague les zéros de fin et le point nu. Conséquences :

1. **Trois jetons flottants restent dans le cœur.** L'inventaire (`tools/float_inventory.txt`) les attribue à D2 : `pico8_value_tostr` `autre` 1 (`double`), `autre` 2 (`pico8_fixed_to_double`) et `reecrit` 1 (`%.4f`). La table de propriété de la Phase C les affecte à D2.
2. **`pico8_value.c` est l'un des 3 fichiers encore en échec dans la garde** `-mgeneral-regs-only` (`tools/float_guard.txt`), avec `pico8_economy_formulas.c` (D3) et `pico8_fixed.c` (D5, E). Il n'y est que pour cette raison.
3. **La correction doit être invisible.** Le comportement actuel est déjà celui qu'on veut garder : 13 témoins sur 14, et le 14ᵉ est conservé. Toute différence de sortie, même sur une valeur qu'aucun fichier du filet n'affiche, serait une régression. Le filet (`frames-check`) n'en attrape qu'une petite partie : le jeu n'affiche que quelques nombres, presque tous entiers. Il faut donc prouver l'égalité au niveau valeur, sur tout le domaine, et non par argument.
4. **Le piège est l'arrondi de `printf`.** Une partie fractionnaire 16.16 vaut exactement `f / 65536`, et `f · 10 000 / 65 536 = f · 625 / 4 096`. Quand le reste de cette division vaut exactement 2 048, la valeur tombe pile au milieu entre deux décimales à 4 chiffres, et la glibc arrondit **au pair** (`0x0000.0800` → `"0.0312"`, pas `"0.0313"`). Comme 625 est impair, cela arrive pour `f ≡ 2 048 (mod 4 096)`, donc 16 valeurs de `f` par partie entière. Un arrondi « demi vers le haut » naïf se tromperait sur 8 d'entre elles. Mesuré en cadrage de cette PRD : sur 18 parties entières, cet arrondi fait 152 erreurs, alors que l'arrondi au pair n'en fait aucune.

## Solution

`pico8_value_tostr` calcule la représentation décimale de sa branche `FIXED` **en entiers**, sans `double` ni `%.4f`. La décision est d'obtenir la même sortie que l'ancien code sur **toute** entrée 16.16, y compris le cas `< 0.0001` : on garde la sortie actuelle (brut −6 → `"-0.0001"`), qui n'est pas sourcée. Cette identité est prouvée par un test d'hôte exhaustif qui garde l'ancienne implémentation **dans le test seulement**. `frames-check` constate ensuite l'identité sur le filet, sans `frames-baseline`.

Algorithme retenu (validé par un prototype de cadrage, voir « Further Notes ») :

- signe : négatif si le brut est < 0. Magnitude `m` = valeur absolue du brut, en non signé 32 bits (brut `0x80000000` → `m = 0x80000000`, sans débordement) ;
- partie entière `ip = m >> 16`, fraction `f = m & 0xFFFF` ;
- `p = f · 625`, `q = p >> 12`, `r = p & 4095` ; `q` augmente de 1 si `r > 2048`, ou si `r == 2048` et `q` est impair (au pair) ;
- si `q == 10 000`, alors `ip` augmente de 1 et `q = 0` (retenue : `0x7FFF.FFFF` → `"32768"`, comme aujourd'hui) ;
- le texte est `"-"` si négatif, puis `ip`, `"."`, `q` sur 4 chiffres avec zéros de tête, **puis l'élagage existant, inchangé** (zéros de fin, puis point nu).

Le `"-"` est émis pour tout brut négatif, même quand les 4 décimales sont nulles. C'est ce que fait `printf` (brut −1 à −3 → `"-0.0000"` → `"-0"` aujourd'hui), et c'est préservé.

Le travail se fait en trois commits :

1. **Le test d'abord, vert sur le code actuel.** On ajoute à la suite de `pico8_value` : (a) un test des 14 témoins du bloc C, recopiés du relevé ; (b) le test exhaustif ancien contre `pico8_value_tostr`, avec l'ancienne implémentation comme helper de test isolé. Sur le code actuel, ce second test compare l'ancien à lui-même : c'est normal, il sert de filet posé avant la réécriture. `make test` est vert.
2. **La réécriture.** La branche `FIXED` passe à l'algorithme entier. Le test exhaustif, les témoins, `make test` (dont `frames-check` identique sur les 12 fichiers de `FRAMES_EVENTS` et `float-inventory-check` avec l'inventaire régénéré) sont verts. **Aucun `frames-baseline`.** L'en-tête de `pico8_value_tostr` et le nouveau trou de portage sont mis à jour dans ce commit.
3. **La consignation.** Régénérer `make float-guard` et `make float-coverage`. Dans `docs/plan-2.md` : un « État de D2 », la puce D2 marquée faite, la ligne D2 de la table de propriété mise à jour, et la phrase « même motif pour D2 » de D0a annotée par la décision « pas de paire d'audit » (voir Implementation Decisions).

Si le test exhaustif trouve un écart que l'algorithme ne peut pas reproduire, ou si `frames-check` diverge sur un seul fichier, l'étape s'arrête. On ne re-baseline pas, on ne commite pas la réécriture, et on consigne le constat dans le plan. D2 est préservatrice par définition : une divergence n'est jamais attribuée, elle est un échec.

## User Stories

1. En tant qu'agent qui mène D2, je veux que `pico8_value_tostr` n'utilise plus ni `double`, ni `pico8_fixed_to_double`, ni `%.4f`, pour que les 3 jetons D2 disparaissent de l'inventaire.
2. En tant qu'agent, je veux que `pico8_value.c` compile sous `-mgeneral-regs-only`, pour qu'il sorte de `tools/float_guard.txt` (3 → 2 fichiers en échec).
3. En tant qu'agent, je veux que la sortie de la réécriture soit identique octet pour octet à celle de l'ancien code sur toute entrée testée, pour que D2 reste préservatrice comme le plan l'exige.
4. En tant qu'agent, je veux que l'égalité soit établie par un test qui énumère les 65 536 parties fractionnaires, pas par un raisonnement sur l'arrondi, pour que la preuve soit exécutable et rejouée à chaque `make test`.
5. En tant qu'agent, je veux que ce test couvre les deux signes, pour que le `"-"` de tête, et en particulier `"-0"` pour les bruts −1 à −3, soit vérifié.
6. En tant qu'agent, je veux que le test couvre les parties entières 0, ±1, 32767, −32767 et −32768, pour que les bornes du format 16.16 soient vérifiées, dont la magnitude 32768 qui ne tient pas dans un `int16`.
7. En tant qu'agent, je veux que le test couvre aussi des parties entières à chaque changement de nombre de chiffres (9/10, 99/100, 999/1000, 9999/10000), pour qu'une erreur d'écriture de la partie entière ne passe pas inaperçue.
8. En tant qu'agent, je veux que le test couvre les fractions négatives de magnitude inférieure à 1 (brut −1 à −65 535), pour que le cas « partie entière nulle et négatif » soit vérifié, puisque `-0` n'existe pas en entier.
9. En tant qu'agent, je veux que la retenue de l'arrondi vers la partie entière (fraction ≥ 0.99995) soit couverte, pour que `0x7FFF.FFFF` rende `"32768"` comme aujourd'hui.
10. En tant qu'agent, je veux que les 16 égalités exactes par partie entière soient comptées par le test, pour qu'un test qui les manquerait (domaine mal énuméré) échoue au lieu de passer à vide.
11. En tant qu'agent, je veux que l'arrondi du nouveau code soit au pair sur ces égalités, pour reproduire la glibc, et que le test montre qu'un arrondi demi vers le haut échouerait.
12. En tant qu'agent, je veux que l'ancienne implémentation vive seulement dans le fichier de test, comme helper nommé et isolé, pour que le cœur ne garde aucun flottant (§6.1 admet les helpers de test isolés).
13. En tant qu'agent, je veux que ce helper calcule son `double` directement depuis le brut (`brut / 65536.0`) plutôt que par `pico8_fixed_to_double`, pour qu'il survive à la Phase E, qui retirera cette fonction du cœur.
14. En tant qu'agent, je veux que ce helper reprenne l'élagage d'origine à l'identique, pour qu'il soit exactement l'ancien `tostr` et pas une réinterprétation.
15. En tant qu'agent, je veux un test des 14 témoins du bloc C, recopiés du relevé (brut → chaîne), pour que la fonction ait enfin une assertion sourcée au niveau valeur, au lieu des seuls contrats actuels (`3`, `0`, `-5`).
16. En tant qu'agent, je veux que le 14ᵉ témoin (brut −6) asserte la sortie actuelle `"-0.0001"` et renvoie au trou de portage, pour que l'écart connu avec PICO-8 (`"-0"`) soit écrit dans le test et pas caché.
17. En tant qu'agent, je veux que les tests soient posés et verts **avant** la réécriture, pour que la réécriture se fasse sous un filet déjà en place (TDD).
18. En tant qu'agent, je veux que la réécriture ne touche que la branche `FIXED`, pour que les branches `STRING`, `BOOL`, `REF`, `FUNCTION`, la propriété mémoire et TROU-13 restent inchangés.
19. En tant qu'agent, je veux garder l'élagage existant tel quel après l'écriture entière, pour que le diff soit minimal et que la seule nouveauté soit le calcul des chiffres.
20. En tant qu'agent, je veux calculer `f · 625` en non signé 32 bits (au plus 65 535 × 625 = 40 959 375), pour qu'aucun débordement ni comportement indéfini ne soit possible.
21. En tant qu'agent, je veux calculer la magnitude en non signé, pour que le brut `0x80000000` ne déclenche pas le comportement indéfini de la négation d'`INT32_MIN`.
22. En tant qu'agent, je veux que `frames-check` soit identique sur les 12 fichiers de `FRAMES_EVENTS`, sans `frames-baseline`, pour que la porte (ii) de D2 soit tenue.
23. En tant qu'agent, je veux que toute divergence de `frames-check` soit traitée comme un échec de l'étape, et non attribuée, pour que D2 ne puisse pas glisser vers une étape divergente.
24. En tant qu'agent, je veux régénérer l'inventaire dans le commit de réécriture, pour que `float-inventory-check` reste vert (réécrits 5 → 4, autres 23 → 21, total 28 → 25).
25. En tant qu'agent, je veux régénérer la garde et la couverture au commit de consignation, pour que les fichiers versionnés reflètent l'état après D2.
26. En tant qu'agent, je veux un nouveau trou de portage qui documente le cas `< 0.0001` comme non sourcé, pour que la décision du plan (« documenté comme non sourcé ») ait une adresse, et que `pico8_value_tostr` y renvoie par un `/* TROU-n */` nu.
27. En tant qu'agent, je veux que le commentaire d'en-tête de `pico8_value_tostr` cesse de dire « not PICO-8's exact stringify rule », pour qu'il dise le contrat vérifié : 13 témoins du bloc C reproduits, le reste renvoyé au trou.
28. En tant que relecteur humain, je veux un « État de D2 » dans le plan, avec les comptes de l'inventaire, de la garde et de la couverture et le temps du test exhaustif, pour lire la sortie de l'étape sans rien relancer.
29. En tant que relecteur, je veux que le plan note pourquoi D2 ne branche pas de paire sur l'audit, alors que D0a l'annonçait, pour que l'écart avec D0a soit une décision tracée et non un oubli.
30. En tant que mainteneur futur (volet `malloc`), je veux que la propriété de la chaîne rendue (possédée par l'appelant, à libérer) ne change pas, pour que les ~25 sites d'appel restent corrects sans modification.
31. En tant qu'agent, je veux que le test exhaustif reste assez rapide pour `make unit` (environ une seconde), pour qu'il tourne à chaque `make test` au lieu d'être une vérification ponctuelle.

## Implementation Decisions

- **Module modifié : `pico8_value`, branche `FIXED` de `pico8_value_tostr` seulement.** L'interface ne change pas : même signature, même chaîne possédée par l'appelant, même allocation par le `strdup` interne. Les autres branches ne bougent pas.
- **Algorithme : fraction × 10 000 en entier, arrondi au pair**, décrit dans « Solution ». On le choisit parce que la valeur 16.16 est exacte en décimal et que la glibc arrondit l'expansion exacte selon le mode courant (au plus proche, égalités au pair). Il n'y a donc qu'une seule règle à reproduire. Le prototype de cadrage l'a confirmé : 0 écart sur 1 245 183 entrées (18 parties entières × 65 536, plus les 65 535 fractions négatives pures), dont 288 égalités. La preuve qui compte reste le test livré, pas ce prototype.
- **Formatage des entiers : `snprintf` sur des entiers, puis l'élagage existant.** On ne retire pas `stdio` de `pico8_value.c`. Le volet de ce chantier est le flottant ; `stdio`/`malloc` relèvent du §8, dont le cadrage n'est pas fait. Écrire les chiffres à la main serait hors périmètre. Seule condition : aucun format flottant.
- **Le cas `< 0.0001` reste sur le comportement actuel, y compris sa sortie.** Brut −6 → `"-0.0001"`, bruts −1 à −3 → `"-0"`, bruts 1 à 3 → `"0"`. Le nouveau code **n'a donc aucune différence admise** avec l'ancien : la « seule différence admise » du plan n'est pas consommée. Pour le trou de portage, PICO-8 n'est connu que sur le brut −6 (`"-0"`). Sa règle pour les autres magnitudes inférieures à 0.0001 est inconnue, et ne se tranchera que par une relance d'une ligne de la sonde (reliquat de la Phase A).
- **Nouveau trou de portage** dans `docs/trous-de-portage.md`, au numéro libre suivant (TROU-22 à la date de cette PRD) : « `tostr` sous 0.0001 n'est pas sourcé ». Il donne le témoin relevé (brut −6 : PICO-8 `"-0"`, port `"-0.0001"`), dit qu'aucune règle unique ne concilie ce témoin avec `2/3 → "0.6667"`, et renvoie au reliquat de la Phase A. Fichier : `src/pico8_value.c`. La branche `FIXED` y renvoie par un `/* TROU-22 */` nu, et le commentaire d'en-tête de `pico8_value_tostr` est aligné sur le contrat vérifié. Règle des commentaires de `CLAUDE.md` : contrat C et renvoi au trou seulement.
- **Pas de paire d'audit pour D2 (décision prise par la guideline, à défaut d'humain).** La phrase de D0a « même motif pour D2 » prévoyait une paire ancien `%.4f` contre nouveau, branchée sur `pico8_float_audit_record`. Elle n'est pas branchée, pour trois raisons :
  - l'audit sert à **attribuer** des frames divergentes, et D2 n'en a aucune à attribuer : une divergence y est un échec, pas une attribution ;
  - le test d'hôte énumère tout le domaine fractionnaire, alors qu'une paire d'audit ne reverrait que le sous-ensemble de valeurs que le filet affiche, pour une preuve strictement plus faible ;
  - une paire obligerait à garder `%.4f` et `double` dans une unité du build d'audit, donc à maintenir un code flottant de plus sans gain.

  `pico8_float_audit.{c,h}` et `tools/float_audit.sh` ne changent pas. La décision est consignée dans l'« État de D2 » du plan.
- **Inventaire après D2.** Les 3 jetons de `pico8_value_tostr` disparaissent : réécrits 5 → 4, autres 23 → 21, total 28 → 25. `pico8_value.c` n'y a plus aucune ligne. Le fichier d'annotation ne change pas, car D2 ne porte aucun cast.
- **Garde après D2.** On attend que `pico8_value.c` sorte de `tools/float_guard.txt` (3 → 2 : `pico8_economy_formulas.c`, `pico8_fixed.c`). On vérifie en compilant sous la garde, sans le supposer. Si `pico8_value.c` y restait (par exemple à cause d'un prototype `double` de `pico8_fixed.h` qu'il utiliserait encore), c'est un constat à consigner, pas une raison de modifier `pico8_fixed.h` dans D2.
- **Aucun site d'appel ne change.** Les ~25 appels de `pico8_value_tostr` (panneau de score, menu, sondes, recherche, planètes, collecte…) gardent leur code.

## Implementation Chunks

1. **Tests d'abord (commit neutre).** Helper de test isolé « ancien `tostr` », test exhaustif (qui compare aussi les égalités), test des 14 témoins du bloc C. Vert sur le code actuel ; aucune ligne de `src/` ne change.
2. **Réécriture entière.** Branche `FIXED` de `pico8_value_tostr` en entiers ; en-tête et TROU-22 ; inventaire régénéré. `make test` vert, dont `frames-check` identique sur les 12 fichiers ; aucun `frames-baseline`.
3. **Consignation.** `make float-guard` et `make float-coverage` régénérés ; dans `docs/plan-2.md` : « État de D2 », puce D2 ✅, ligne D2 de la table de propriété, note sous D0a sur la paire d'audit non branchée.

## Testing Decisions

- **Un bon test ici regarde la chaîne rendue** par `pico8_value_tostr` pour un brut 16.16 donné, et rien d'autre : pas la valeur intermédiaire `q`, pas la façon dont l'arrondi est codé. Le test exhaustif compare l'API publique à un oracle (l'ancien code). Le test de témoins la compare au relevé PICO-8.
- **Test exhaustif**, dans la suite unitaire de `pico8_value` (lancée par `make unit`, donc par `make test`) :
  - domaine : pour chaque partie entière de la liste {0, 1, 2, 9, 10, 99, 100, 999, 1000, 9999, 10000, 32766, 32767, −1, −2, −10, −32767, −32768}, les 65 536 fractions, avec brut = (partie entière << 16) | fraction ; en plus, les bruts −1 à −65 535 (fractions négatives pures, partie entière « −0 ») ;
  - assertion : `strcmp` égal entre le helper « ancien » et `pico8_value_tostr`, sur **chaque** entrée. Le premier écart est imprimé (brut en hexa, deux chaînes) avant l'échec, pour qu'un diagnostic ne demande pas de débogueur ;
  - garde contre un test vide : le test compte les entrées où `f · 625 ≡ 2048 (mod 4096)` et asserte que ce compte vaut 16 × le nombre de parties entières. Il asserte aussi le nombre total d'entrées ;
  - le helper « ancien » est nommé comme tel (par exemple `old_tostr_percent_4f_for_test`), `static` dans le fichier de test. Il calcule `brut / 65536.0` lui-même et reprend l'élagage d'origine octet pour octet ;
  - coût : le prototype mesure 0,37 s à `-O2` pour 1,25 M comparaisons. À `-O0` on attend environ 1 s. Le temps réel est consigné dans « État de D2 ».
- **Test des témoins du bloc C** : 14 couples (brut → chaîne attendue) recopiés de `tools/probe_out.txt` `## c tostr`, jamais dérivés : `0xfffe.8000 → "-1.5"`, `0x0001.0000 → "1"`, `0 → "0"`, `0x0000.2666 → "0.15"`, `0x0000.451e → "0.27"`, `0x0000.1999 → "0.1"`, `0x0002.8000 → "2.5"`, `0x0064.0000 → "100"`, `0x03e8.0000 → "1000"`, `0x8000.0000 → "-32768"`, `0x7fff.0000 → "32767"`, `0x0000.5555 → "0.3333"`, `0x0000.aaaa → "0.6667"`. Le 14ᵉ, `0xffff.fffa`, asserte `"-0.0001"` (sortie conservée) avec un message qui nomme TROU-22 et la valeur PICO-8 `"-0"`.
- **Test d'égalités ciblé**, court et lisible, en plus de l'exhaustif : `0x0000.0800 → "0.0312"` et `0x0000.1800 → "0.0938"`, pour que le piège soit visible sans lire la boucle.
- **Porte (ii)** : `make frames-check` identique sur les 12 fichiers de `FRAMES_EVENTS`, **sans** `frames-baseline`, lancé dans `make test`.
- **Tests existants** : les 5 tests `tostr` actuels (`3`, `0`, `-5`, chaîne, booléen) restent tels quels.
- **Prior art** : `tests/test_pico8_value.c`, qui a le style des tests `tostr` actuels ; `tests/test_pico8_fixed.c`, où `test_flr_int_matches_probe_block_b` assertait déjà des valeurs recopiées du relevé ; les tests de valeur de C1/D1 pour le motif « expression convertie contre la référence ».
- **Jamais de fenêtre** : aucune vérification ne lance le binaire raylib.

## Out of Scope

- Changer la sortie pour une valeur quelconque, y compris aligner le brut −6 sur le `"-0"` de PICO-8. Il faudrait d'abord relancer la sonde, et ce serait une étape divergente.
- Brancher une paire D2 sur l'audit (`pico8_float_audit_record`) : décidé non, voir Implementation Decisions.
- Retirer `snprintf`, `stdio.h`, `malloc` ou le `strdup` interne de `pico8_value.c` : c'est le volet `malloc`/libc (§8), dont le cadrage n'est pas fait.
- TROU-13 (`tostr` d'un champ absent rend `""`) et le `tostr` hexadécimal (`tostr(x, true)`), qui n'est pas porté.
- D3 (`^0.75`), D4 (clés de table), D5 (`sin`/`cos`), la Phase E (retrait de `pico8_fixed_from_double`/`_to_double`) et les jetons de commentaires.
- Toute modification de `pico8_fixed.h`/`pico8_fixed.c`.
- L'outil d'observation, qui s'arrête depuis D1 (code 2). Ce constat est laissé à E.
- L'audio, et tout lancement de l'application raylib.

## Further Notes

- **Prototype de cadrage** (bac à sable, non versionné). L'algorithme de « Solution » comparé à `snprintf("%.4f")` + élagage : 0 écart sur 1 245 183 entrées, 288 égalités (16 × 18). La variante « demi vers le haut » fait 152 écarts, par exemple `0x0000.4800` : ancien `"0.2812"`, demi-haut `"0.2813"`. Sorties de l'ancien code relevées au passage : brut −6 → `"-0.0001"`, brut −3 → `"-0"`, `0x0000.0800` → `"0.0312"`, `0x0000.1800` → `"0.0938"`, `0x8000.0000` → `"-32768"`, `0x7FFF.FFFF` → `"32768"`. Ces chiffres guident l'écriture du test ; ils ne le remplacent pas.
- **Pourquoi pas les 2³² bruts.** Le prototype fait environ 3,4 M comparaisons par seconde à `-O2`, donc les 2³² bruts prendraient une vingtaine de minutes, hors budget de `make test`. L'arrondi de la fraction ne dépend pas de la partie entière, sauf par la retenue et le signe, que le domaine retenu couvre pour chaque fraction. Les parties entières retenues couvrent toutes les longueurs de 1 à 5 chiffres, dans les deux signes.
- **Hôte de référence.** L'oracle est la glibc de l'hôte (GCC 13.3, x86_64). Le test tourne sur cet hôte ; une libc qui n'arrondirait pas au pair ferait échouer le test sur l'oracle, pas sur le nouveau code. On consigne cette dépendance dans l'« État de D2 », sans rien construire pour l'éviter.
- **Couverture** : la branche `FIXED` de `pico8_value_tostr` est atteinte par le filet (panneau de score, menu, planètes). Le test exhaustif la protège de toute façon au niveau valeur. Il n'y a pas de « correction non vue par le filet », puisque D2 ne corrige rien.
