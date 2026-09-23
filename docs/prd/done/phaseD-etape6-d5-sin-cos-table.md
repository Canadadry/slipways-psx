---
title: "Plan 2, Phase D5 : pico8_fixed_sin/cos lisent une table quart d'onde 16.16 pleine résolution, sans libc, exactes sur les 64 points du bloc E et identiques à l'ancien calcul sur les 65 536 fractions de tour"
description: "pico8_fixed_sin/cos passent par to_double, sin()/cos() de la libc et from_double, ce qui laisse 10 jetons flottants dans pico8_fixed.c et TROU-1 ouvert ; D5 les remplace par une table entière générée sur l'hôte (16 385 entrées, un quart de tour, indexée par les 16 bits bas de l'argument, cos = sin décalé d'un quart de tour, sans interpolation), assertée au bit près sur les 128 vecteurs sin/cos du bloc E et exhaustivement contre l'ancien calcul, mesurée par une paire d'audit libc/table, attribuée par la règle de la Phase D, re-baselinée seulement si le filet diverge."
status: needs-triage
---

## Problem Statement

Texte de l'étape, recopié de `docs/plan-2.md` §7 Phase D :

> **D5 — `sin`/`cos` sur table** (décision humaine). `pico8_fixed_sin`/`cos` lisent une table de valeurs 16.16 indexée par la fraction de tour, sans `sin()`/`cos()` libc. Modèle donné par l'humain : le portage de Celeste Classic sur GBA (JeffRuLz/Celeste-Classic-GBA, `src/sine.h` + `main.c:331-332` : 360 entrées `float`, index `(u16)(x*360) % 360`, sans interpolation) — le principe est repris, pas ses `float`. Porte : les 64 vecteurs `sin(i/64)`/`cos(i/64)` du bloc E reproduits **au bit près** ; hors de ces points, la table est une approximation **non sourcée**, documentée comme telle dans TROU-1 (taille, construction, interpolation éventuelle : à trancher par la PRD de l'étape). Divergente, avec re-baseline et attribution par l'audit avant/après (paire ancien libc / nouvelle table).

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

Contexte du plan, en résumé : le §4 et le §5 posaient une clôture (`rnd`/`sin`/`cos` non réécrits, blocs E et F « capturés, non exploités ») ; la note « Décision tranchée par l'humain » de la Phase E et la ligne D5 de la table de propriété la lèvent **pour `sin`/`cos` seulement** : les vecteurs `sin`/`cos` du bloc E deviennent les points de contrôle, `pico8_fixed_rnd` reste hors périmètre (TROU-17).

État d'aujourd'hui :

1. **`pico8_fixed_sin`/`_cos` calculent en `double`.** `turns = to_double(x)`, `radians = turns · 2π`, `sin()`/`cos()` de la libc, puis `from_double`, qui **arrondit au plus proche** (moitié loin de zéro). `sin` rend l'opposé : `sin(x) = −sin(2πx)`, l'axe y de l'écran PICO-8 pointant vers le bas.
2. **Dix jetons de l'inventaire** (`tools/float_inventory.txt`) : pour chacune des deux fonctions, `double` ×2, `pico8_fixed_to_double`, `pico8_fixed_from_double(…)` (autres) et `sin`/`cos` (réécrit). La table de propriété les affecte à D5. L'inventaire compte aujourd'hui **20** jetons (réécrits 2, autres 18).
3. **`pico8_fixed.c` est le dernier fichier en échec dans la garde** `-mgeneral-regs-only` (`tools/float_guard.txt`), pour D5 et pour les définitions `from_double`/`to_double` (Phase E).
4. **Constat de cadrage, mesuré (prototype hors dépôt, même formule et même arrondi que le C) : l'ancien calcul reproduit déjà les 128 vecteurs du bloc E au bit près** (64 `sin`, 64 `cos`). Le bloc E est cohérent avec « arrondi au plus proche de la valeur vraie » ; il est incohérent avec la troncature (62 écarts sur 128). La porte « 64 vecteurs au bit près » ne demande donc pas de changer une seule valeur aux points relevés.
5. **Les tests** (`tests/test_pico8_fixed.c`) n'assertent sur `sin`/`cos` que des contrats (0, quart de tour, demi-tour, période entière), sous un commentaire qui dit qu'aucune valeur bit-exacte n'est disponible. C'est faux depuis le relevé.
6. **Appelants dans le cœur** (et domaine des arguments, lu dans le code et `lua.lua`) :
   - `pico8_vec_qm` (`qm(fy,lx)`, `lua.lua:124-126`) appelle `cos(lx)` et `sin(lx)` **sur la même ligne**. Ses appelants : `pico8_ma` (`e.t*0.2`, grand, croît sans borne), `pico8_planet_family` (`rnd()`, dans [0,1), **à la génération de la galaxie**), `pico8_probe` `g_:mm` (`(flr(rnd(pn))+rnd(0.5))/pn + t*0.001`), `pico8_slipway_display` (`t/126`, `t/176`).
   - `pico8_na` (`sin(t*0.005)*0.06`, `lua.lua:578`) : **écrit `ra.y`, donc l'état**.
   - `pico8_wormhole` (`sin(t/40)*0.1`, `lua.lua:773`) : écrit le champ `oi`, donc l'état.
   - Arguments négatifs possibles : les compteurs `t` repassent à −32 768 après 32 768 frames (constat `qc.t` de C0) ; le plus long fichier du filet en fait 9 729. Les arguments dépassent largement 1 tour (`t*0.2`).
   - `sin`/`cos` nourrissent donc l'état (positions de planètes à l'initialisation, dérive d'entités, `oi`), pas seulement le dessin : toute valeur rendue différente se propagerait à tout le reste du rejeu.

## Solution

`pico8_fixed_sin`/`_cos` lisent une **table entière de quart d'onde, à pleine résolution** : 16 385 valeurs 16.16 de `sin` au sens mathématique pour les fractions de tour `r/65536`, `r = 0..16384`. L'argument est réduit à sa fraction de tour par un **masque des 16 bits bas** (période 1 exacte, négatifs compris, sans flottant), le quadrant (2 bits hauts de la fraction) replie l'index sur le quart d'onde et fixe le signe ; `cos(x)` est `sin` standard de `x + 1/4` (même table, décalage d'un quart de tour) ; `pico8_fixed_sin` rend l'opposé. Pas d'interpolation : la table a une entrée par valeur de fraction représentable.

Chaque entrée est `arrondi au plus proche de sin(2π·r/65536)·65536`, exactement ce que calcule l'ancien code. La table est **produite sur l'hôte** par un générateur versionné qui recopie l'ancienne expression, et versionnée comme source générée ; le cœur ne contient que des entiers.

Conséquences, qui sont la raison du choix :

- les 64 + 64 vecteurs du bloc E sont reproduits au bit près (ce sont des entrées de la table : `i/64` tour = `r = 1024·i`) ;
- sur les 65 536 fractions de tour, la nouvelle fonction rend **exactement** ce que l'ancienne rend sur la même fraction (mesuré en cadrage : 0 écart pour `sin`, 0 pour `cos`) ;
- les multiples de 1/4 sont exacts (0, ±1) ;
- D5 est donc **attendue préservatrice** : le filet ne doit pas bouger, sauf là où l'ancien calcul ne respectait pas lui-même la période 1 (un arrondi de la libc sur un argument de plusieurs milliers de tours) — cas que l'audit mesure. Le chemin « divergente » de la règle d'attribution reste écrit et prêt.

La preuve de valeur tient en trois tests : les 128 vecteurs du bloc E recopiés, un balayage exhaustif des 65 536 fractions contre l'ancienne expression (copie isolée dans le test), et les contrats de période, de signe et de symétrie. La mesure sur le filet est une paire d'audit ancien libc / nouvelle table, branchée sur `pico8_float_audit_record` à chaque site d'appel. Puis la règle d'attribution. Deux issues :

- **Le filet ne diverge pas** (attendu) : S vide, `D5.txt` (« conforme » partout) et `D5-attribution.txt` versionnés quand même, **aucun `frames-baseline`**. Les sites sont listés « transformation sans écart sur le filet », protégés par le balayage exhaustif.
- **Le filet diverge** : S lu dans l'audit, contrôles 1 à 3, départage `sin`/`cos` (point 5), cascade selon la règle de D3, revue sans fenêtre à F, `D5.txt` et `D5-attribution.txt` versionnés **avant** `make frames-baseline ETAPE=D5`.

Trois commits (voir Implementation Chunks) : tests du bloc E et contrats sur le code actuel (neutre) ; générateur, table, réécriture, paire d'audit, attribution et re-baseline éventuel ; consignation.

## User Stories

1. En tant qu'agent qui mène D5, je veux que `pico8_fixed_sin`/`_cos` n'utilisent plus ni `double`, ni `sin()`/`cos()`, ni `pico8_fixed_to_double`/`from_double`, pour que les 10 jetons D5 disparaissent de l'inventaire.
2. En tant qu'agent, je veux que les deux fonctions lisent une table de valeurs 16.16 indexée par la fraction de tour, pour suivre la décision humaine (principe du portage Celeste GBA, sans ses `float`).
3. En tant qu'agent, je veux que la fraction de tour soit obtenue en masquant les 16 bits bas de l'argument brut, pour que la réduction de période soit entière, exacte, et correcte sur les négatifs en complément à deux.
4. En tant qu'agent, je veux que la table ait une entrée par fraction représentable (pleine résolution), pour qu'aucune interpolation ni arrondi d'index ne soit à inventer et que la fonction ne dépende d'aucune règle non sourcée de plus.
5. En tant qu'agent, je veux ne stocker qu'un quart d'onde et replier par quadrant, pour diviser la table par quatre sans perdre un bit (écart mesuré : 0 sur 65 536 fractions).
6. En tant qu'agent, je veux que `cos` lise la table de `sin` décalée d'un quart de tour, pour n'avoir qu'une table et une seule logique de repli.
7. En tant qu'agent, je veux que `pico8_fixed_sin` garde l'inversion de signe de PICO-8, pour que `sin(0.25) = −1` reste vrai et que les appelants ne changent pas.
8. En tant qu'agent, je veux que les multiples de 1/4 de tour rendent exactement 0, 1 ou −1, pour tenir les contrats existants.
9. En tant qu'agent, je veux un test qui asserte les 64 vecteurs `## e sin` et les 64 vecteurs `## e cos` de `tools/probe_out.txt` au bit près, recopiés et jamais dérivés, pour que la porte de l'étape soit une assertion de `make test`.
10. En tant qu'agent, je veux que ce test passe **avant** la réécriture, sur l'ancien calcul, pour prouver par l'exécution que l'ancien code était déjà exact aux points relevés et que la table n'y change rien.
11. En tant qu'agent, je veux un balayage exhaustif des 65 536 fractions qui compare la nouvelle `sin` et la nouvelle `cos` à l'ancienne expression appliquée à la même fraction, pour que l'identité hors des points relevés soit prouvée et non échantillonnée.
12. En tant qu'agent, je veux un test de période sur des parties entières variées, négatives comprises (−32 768, −1, 1, 2, 100, 32 767), pour que `f(x) = f(fraction(x))` soit asserté.
13. En tant qu'agent, je veux un test de symétrie (`sin(−x) = −sin(x)`, `cos(−x) = cos(x)`) sur quelques fractions non triviales, pour qu'un repli de quadrant faux sur les négatifs soit attrapé même s'il était cohérent avec lui-même.
14. En tant qu'agent, je veux que la table soit produite par un générateur hôte versionné, qui reprend l'ancienne expression mot pour mot (même 2π, même arrondi que `from_double`), pour que la provenance de chaque entrée soit lisible et reproductible.
15. En tant qu'agent, je veux que le fichier généré soit versionné et que le cœur ne contienne que des entiers, pour que le générateur, outil d'hôte (§4), garde la libc sans que le cœur en dépende.
16. En tant qu'agent, je veux que le générateur ne soit pas relancé par `make test`, et que le balayage exhaustif soit la protection du fichier généré, pour qu'une édition à la main ou une régénération différente fasse échouer la suite sans contrôle de fraîcheur supplémentaire.
17. En tant qu'agent, je veux une paire d'audit ancien libc / nouvelle table à chaque site d'appel, branchée sur `pico8_float_audit_record`, pour mesurer S, A, les appels, les négatifs et les fractionnaires sur le filet au lieu de les supposer.
18. En tant qu'agent, je veux que l'ancienne expression n'existe que dans le build d'audit, dans `pico8_float_audit.c`, pour que le cœur ne voie aucun `double` même sous `-DPICO8_FLOAT_AUDIT`.
19. En tant qu'agent, je veux que la ligne partagée de `pico8_vec_qm` (`cos` et `sin` sur la même ligne) soit consignée comme telle, pour que son compte double se lise comme les lignes partagées de D1.
20. En tant qu'agent, je veux appliquer la règle d'attribution telle qu'elle est écrite, pour que D5 soit jugée comme D1 et D3.
21. En tant qu'agent, je veux que, si le filet diverge alors qu'aucun site n'a d'ancien ≠ nouveau, l'étape s'arrête (F sans A), pour qu'un pixel qui bouge sans valeur auditée ne soit jamais re-baseliné.
22. En tant qu'agent, je veux que, si S contient à la fois `sin` et `cos`, les contrôles 2 et 3 soient répétés fonction par fonction, pour que l'attribution nomme la fonction et non « les deux ».
23. En tant qu'agent, je veux que le contrôle de cascade suive la règle écrite pour D3 (identité C1 exigée sur les fichiers sans A, mesurée et consignée sur les fichiers avec A), puisque `sin`/`cos` écrivent dans l'état, pour que l'écart avec la lettre de la sortie de phase reste une décision tracée.
24. En tant qu'agent, je veux que `make frames-report ETAPE=D5` soit lancé dans tous les cas et `D5.txt` versionné, pour que la sortie de l'étape se lise sans rien relancer.
25. En tant qu'agent, je veux un `D5-attribution.txt` au format de `D3-attribution.txt`, pour lire S, A, F, A→F, les contrôles, la cascade et la revue au même endroit.
26. En tant qu'agent, je veux que `make frames-baseline ETAPE=D5` ne soit lancé que si au moins un fichier diverge et que les contrôles passent, pour ne jamais re-baseliner à vide ni subir une régression.
27. En tant qu'agent, je veux une revue sans fenêtre à F sur chaque fichier divergent, en confirmation, pour savoir quels pixels bougent.
28. En tant qu'agent, je ne veux jamais lancer le binaire raylib, pour respecter `CLAUDE.md`.
29. En tant qu'agent, je veux régénérer l'inventaire dans le commit de réécriture, pour que `float-inventory-check` reste vert (réécrits 2 → 0, autres 18 → 10, total 20 → 10).
30. En tant qu'agent, je veux que `#include <math.h>` et la constante 2π sortent de `pico8_fixed.c` s'ils deviennent orphelins, pour ne pas laisser de reste de la réécriture.
31. En tant qu'agent, je veux vérifier et consigner que `pico8_fixed.c` reste dans la garde à cause des définitions `from_double`/`to_double`, pour que la garde (1 → 1) ne soit pas lue comme un échec de D5.
32. En tant que relecteur humain, je veux un « État de D5 » dans le plan (comptes, garde, couverture, audit, attribution, taille de la table, coût de `make test`), pour lire la sortie de l'étape sans rien relancer.
33. En tant que relecteur, je veux que TROU-1 dise que `sin`/`cos` sont sur table, exactes aux 64 points du bloc E et non sourcées ailleurs, et que `rnd` relève de TROU-17, pour qu'aucun trou ne décrive un calcul qui n'existe plus.
34. En tant que relecteur, je veux que le commentaire de `tests/test_pico8_fixed.c` qui dit qu'aucune valeur bit-exacte n'existe pour `sin`/`cos` soit corrigé, pour qu'il ne mente plus (il reste vrai pour `rnd`).
35. En tant que relecteur, je veux que l'écart entre « Divergente, avec re-baseline » (texte du plan) et le choix d'une table préservatrice soit écrit dans l'« État de D5 », pour pouvoir le contester.
36. En tant que mainteneur futur (Phase E), je veux que plus rien dans le cœur n'appelle `pico8_fixed_to_double`/`from_double` ni la libm, pour que E n'ait plus qu'à retirer les définitions et `-lm`.
37. En tant que mainteneur futur (reprise PSX), je veux que la taille de la table (65 540 octets en lecture seule) soit consignée, pour que le coût mémoire soit une donnée connue au moment de choisir de la réduire.

## Implementation Decisions

- **Taille et indexation : quart d'onde, pleine résolution, sans interpolation.** (Décision prise par la guideline, à défaut d'humain.)
  - Table de 16 385 entiers 32 bits signés : `T[r] = arrondi_moitié_loin_de_zéro(sin(2π·r/65536)·65536)`, `r = 0..16384`. `T[0] = 0`, `T[16384] = 0x0001.0000`. Environ 40 entrées proches du quart valent `0x0001.0000` : un stockage 16 bits non signé ne suffit pas, d'où 32 bits (65 540 octets).
  - `f = x & 0xFFFF` (fraction de tour, complément à deux), `q = f >> 14` (quadrant), `r = f & 0x3FFF`. `sin` standard : q0 `T[r]`, q1 `T[16384 − r]`, q2 `−T[r]`, q3 `−T[16384 − r]`. `cos(x)` = `sin` standard de `(f + 0x4000) & 0xFFFF`. `pico8_fixed_sin(x)` = opposé du `sin` standard de `f`.
  - Les 64 points du bloc E sont les entrées `r = 1024·i` repliées : ce sont des valeurs de table, pas des points d'ancrage d'une interpolation.
  - **Options écartées, mesurées en cadrage** :
    - *64 entrées (les seuls points relevés), sans interpolation* : pas de 5,6° ; `qm(qd, rnd())` place les planètes à la génération, elles tomberaient sur 64 rayons. Rejeté.
    - *Table du modèle Celeste (≈ 360 pas) ou 1 024 pas, sans interpolation* : erreur jusqu'à ~400 LSB ; chaque valeur rendue ou presque change, sans source.
    - *257 entrées (1/1 024 de tour) et interpolation linéaire entière* : ~1 Ko, écart ≤ 1 LSB avec l'ancien, mais **14 416 fractions sur 65 536 différentes (22 %)** pour `sin`, autant pour `cos`. Ces divergences ne seraient justifiées par aucune ligne du relevé, ce que le §6.4 interdit (« chacune justifiée par une ligne du relevé de sonde »), et elles introduiraient des valeurs non sourcées nouvelles (§9).
  - Raison du choix : c'est la seule forme qui tient à la fois la porte (64 points au bit près), le §6.4 (aucune divergence non sourcée) et le §9 (aucune valeur nouvelle non sourcée : hors des points, la table rend ce que le port rendait déjà). Son coût est la mémoire (voir Further Notes).
- **Comportement PICO-8 conservé.** Inversion de signe de `sin`, période 1 exacte (par le masque, y compris sur les négatifs et au-delà de ±1 tour), multiples de 1/4 exacts. Pas de cas particulier par valeur.
- **Production de la table : générateur hôte, sortie versionnée.** (Décision prise par la guideline.)
  - Un petit programme C d'hôte dans `tools/`, construit et lancé par une cible `make` hors suite (par exemple `make sin-table`). Il recopie l'ancienne expression : même constante 2π que `pico8_fixed.c`, même arrondi que `pico8_fixed_from_double`. C plutôt que Python ou awk : la provenance est l'ancien code, lu tel quel.
  - Sortie : un en-tête généré sous `src/`, inclus par `pico8_fixed.c` seul, qui déclare la table `static const int32_t`, une valeur par entrée en hexadécimal brut. Première ligne : un commentaire « généré par … ; ne pas éditer à la main » (piège qu'on réintroduirait, admis par la règle des commentaires). Aucun des mots `double`/`float` ni de motif `sin(`/`cos(` dans le fichier généré : l'inventaire ne lit que les `.c` du cœur, mais le nom de la table et le commentaire l'évitent quand même.
  - Pas de contrôle de fraîcheur dans `make test` : le balayage exhaustif compare chaque entrée utilisée à l'ancienne expression ; une table éditée ou régénérée autrement fait échouer la suite. L'hôte de référence est celui du plan (GCC 13.3, glibc, x86_64) ; une autre libm qui arrondirait différemment une entrée ferait échouer le test, ce qui est le signal voulu.
- **Garantie des 64 points de contrôle : test recopié du bloc E.** 64 couples `sin` et 64 couples `cos` (argument `i/64` en brut = `i·0x400`, attendu en hexa brut) recopiés de `## e sin` / `## e cos`, commentaire de provenance « copied from the probe, never derived », motif de `test_sqrt_matches_probe_block_e`. Pas d'analyse du fichier à l'exécution.
- **Paire d'audit : une par appel, au niveau de la valeur rendue par `sin`/`cos`.** (Décision prise par la guideline.)
  - Motif de `pico8_fixed_flr_int` : sous `-DPICO8_FLOAT_AUDIT`, `pico8_float_audit.h` redéfinit `pico8_fixed_sin(x)` et `pico8_fixed_cos(x)` en appels à des fonctions d'audit qui reçoivent `__FILE__`, `__func__`, `__LINE__` ; celles-ci appellent la vraie fonction (entre parenthèses, pas la macro), calculent l'ancienne expression libc (copie privée dans `pico8_float_audit.c`, qui garde la libc), appellent `pico8_float_audit_record(fichier, fonction, ligne, x, ancien ≠ nouveau, 0)` et rendent la nouvelle valeur. `pico8_fixed.c` reste compilé sans le drapeau (le `Makefile` le fait déjà).
  - Argument enregistré : `x` brut. Les colonnes négatifs et fractionnaires répondent à la question « quels arguments ces sites voient-ils ? ».
  - **Colonne « nouveau ≠ référence » : toujours 0**, comme D3. La vérité aux points relevés est assertée par le test du bloc E, pas par l'audit. Écrit dans l'en-tête de l'audit et dans l'« État de D5 ». `tools/float_audit.sh` ne change pas (il lit déjà des paires D sans lecture statique).
  - Sites attendus : 3 lignes, 4 appels — `pico8_vec_qm` (`cos` et `sin`, **ligne partagée**), `pico8_na`, le site de `oi` de `pico8_wormhole.c`. Tous classés D (absents de `tools/float_audit_c1.txt`).
  - Pour départager `sin` et `cos` sur la ligne partagée, on ne touche pas au code de `pico8_vec_qm` : le départage passe par les variantes par fonction (point 5 de la règle).
- **Règle d'attribution appliquée à D5.** L'unité de conversion est la **fonction** (`sin`, `cos`), pas le site d'appel : S = les fonctions dont au moins un site a un ancien ≠ nouveau.
  - Contrôle 1 : lu par fichier sur l'audit et `D5.txt`.
  - Si S est vide : complément neutre = arbre du commit 1 (tests seulement) contre les références d'avant D5, **lancé quand même** ; variante suffisante satisfaite par construction, consignée.
  - Si S n'est pas vide : complément neutre = arbre D5 avec les fonctions de S ramenées à l'ancienne expression ; variante suffisante = arbre d'avant avec les seules fonctions de S sur table ; si S = {`sin`, `cos`}, les deux contrôles sont répétés fonction par fonction.
- **Cascade : règle de D3.** (Décision prise par la guideline.) `sin`/`cos` écrivent dans l'état (`pico8_na` → `ra.y`, `oi`, positions générées par `qm`). Sur chaque fichier **sans A**, les enregistrements C1 d'avant D5 se retrouvent à l'identique dans l'arbre D5 (différence = échec). Sur chaque fichier **avec A**, ils peuvent différer à partir de A : nombre de sites C1 changés mesuré et consigné, porte C1 de `float-audit` dure partout. Si S est vide, le premier cas vaut pour les 12 fichiers.
- **Re-baseline.** `make frames-report ETAPE=D5` dans tous les cas, `tools/frames_reports/D5.txt` et `D5-attribution.txt` versionnés dans le commit de réécriture ; `make frames-baseline ETAPE=D5` seulement si un fichier diverge et que les contrôles passent.
- **Corrections invisibles au filet.** Si S est vide, les sites sont listés « transformation sans écart sur le filet » (ce n'est pas une correction : la valeur est la même par construction), avec leur protection nommée : balayage exhaustif et test du bloc E.
- **Inventaire après D5.** Les 10 jetons de `pico8_fixed_cos`/`_sin` disparaissent : réécrits 2 → 0, autres 18 → 10, total 20 → 10. Restent : les 6 jetons des définitions `from_double`/`to_double` (E), le mot `float` hors fonction de `pico8_fixed.c`, et les lignes de `pico8_galaxy.c` et `pico8_ob.c` (E). Tout jeton nouveau serait un constat à consigner.
- **Garde après D5.** `pico8_fixed.c` reste en échec (les définitions rendent ou prennent un `double`) : 1 → 1, vérifié en relançant la garde, et consigné comme attendu jusqu'à E.
- **Orphelins.** `#include <math.h>` et la constante 2π sortent de `pico8_fixed.c` si plus rien ne s'en sert ; la constante vit alors dans le générateur et dans la copie d'audit. `-lm` n'est pas retiré (Phase E) ; l'« État de D5 » consigne si le cœur appelle encore une fonction de la libm.
- **Commentaires et TROU-1** (règle de `CLAUDE.md`).
  - TROU-1 réécrit : `sin`/`cos` lisent une table générée ; exactes au bit près sur les 64 points `i/64` du bloc E ; ailleurs, elles rendent l'arrondi au plus proche de la valeur vraie, ce que rendait l'ancien calcul, **sans source PICO-8** ; `pico8_fixed_from_double` reste défini (E) ; `rnd` : voir TROU-17.
  - Les marqueurs `/* TROU-1 */` restent sur `sin`/`cos` (`.c` et `.h`), puisque l'écart hors des points reste connu. Celui du test est déplacé ou ajusté avec le commentaire corrigé.
  - Le contrat en tête des fonctions dans `pico8_fixed.h` (tours, inversion de signe) ne change pas.
- **Aucun appelant ne change.**

## Implementation Chunks

1. **Tests sur le code actuel (commit neutre).** Test des 128 vecteurs du bloc E, test de période sur parties entières négatives et grandes, test de symétrie. Tous passent sur l'ancien calcul (vérifié en cadrage pour le bloc E). Le commentaire « aucune valeur bit-exacte » est corrigé pour `sin`/`cos`. `make test` vert, `frames-check` identique : c'est aussi le complément neutre si S est vide.
2. **Table, réécriture, audit, attribution.** Générateur hôte et cible `make` ; en-tête généré versionné ; `pico8_fixed_sin`/`_cos` réécrites sur la table ; balayage exhaustif ajouté aux tests (copie isolée de l'ancienne expression dans le test) ; `math.h` et 2π retirés s'ils sont orphelins ; paire d'audit ; TROU-1 et commentaires mis à jour ; inventaire régénéré. `make float-audit` : S, A, appels, négatifs, fractionnaires par site. `make frames-report ETAPE=D5`. Attribution et cascade dans `D5-attribution.txt`. Puis, seulement si divergence attribuée, `make frames-baseline ETAPE=D5`. `make test` vert. `D5.txt` et `D5-attribution.txt` dans ce commit.
3. **Consignation.** `make float-guard` et `make float-coverage` régénérés. Dans `docs/plan-2.md` : « État de D5 » (dont la taille de la table et l'écart avec « Divergente »), puce D5 ✅, ligne D5 de la table de propriété, risque §10 « Dérive vers la réécriture de `sin`/`cos`/`rnd` » mis à jour (`sin`/`cos` faits sur décision humaine, `rnd` toujours clos).

L'étape s'arrête, sans commit de la réécriture et avec un constat consigné dans le plan, si : le test du bloc E échoue sur l'ancien code au chunk 1, le balayage exhaustif trouve un écart, le contrôle 1 échoue (F sans A, A > F), ou un fichier sans A voit ses enregistrements C1 changer.

## Testing Decisions

- **Un bon test ici compare la valeur 16.16 rendue** pour un argument brut à une valeur attendue (relevé, ou ancienne expression sur la même fraction). Il ne regarde ni l'index, ni le quadrant, ni la table elle-même.
- **Bloc E**, suite `pico8_fixed` (`make unit`, donc `make test`) : 128 couples recopiés, égalité exacte. Passe avant et après la réécriture.
- **Balayage exhaustif** : pour chaque fraction `f` de 0 à 65 535, nouvelle `sin(f)` = `−arrondi(sin(2π·f/65536)·65536)` et nouvelle `cos(f)` = `arrondi(cos(…)·65536)`, par une copie isolée de l'ancienne expression dans le test (motif de D2 pour `tostr`). Les tests sont un outil d'hôte (§4), ils gardent la libc.
- **Période** : pour des parties entières `n` dans {−32 768, −1, 1, 2, 100, 32 767} et quelques fractions, `f(n + frac) = f(frac)`.
- **Symétrie** : `sin(−x) = −sin(x)`, `cos(−x) = cos(x)` sur quelques fractions non multiples de 1/4.
- **Contrats existants** (0, quart de tour, demi-tour, période entière) conservés tels quels.
- **Mesure non assertée** : combien de couples (partie entière, fraction) donnent un ancien calcul différent de l'ancien calcul sur la fraction seule (le seul endroit où D5 peut changer une valeur). En cadrage : 0 sur 200 000 arguments bruts tirés au hasard. Consigné dans l'« État de D5 », pas asserté : c'est une propriété de l'ancien code, pas du nouveau.
- **Audit** : `make float-audit` vert (dans `make test`), avec les nouveaux sites D ; on vérifie à la main les lignes `AUD` des trois lignes d'appel. `tests/test_float_audit.c` ne change pas.
- **Filet** : `frames-check` sur les 12 fichiers après le commit 1 et après le commit 2.
- **Prior art** : `test_sqrt_matches_probe_block_e`, `test_pow075_matches_probe_block_d`, `test_flr_int_matches_probe_block_b` (`tests/test_pico8_fixed.c`) ; le balayage de D2 dans `tests/test_pico8_value.c` ; la paire `pico8_fixed_flr_int` de `pico8_float_audit.c` ; `tools/frames_reports/D3-attribution.txt` pour le format.
- **Jamais de fenêtre** : toute vérification passe par le rejeu headless et le dump texte.

## Out of Scope

- `pico8_fixed_rnd` et le bloc F : clôture maintenue (TROU-17).
- Une table plus petite, une interpolation, ou un stockage 16 bits : décision possible à la reprise PSX, pas ici.
- La vérification `mipsel` soft-float (hors périmètre par décision humaine).
- La Phase E : définitions `from_double`/`to_double`, `-lm`, mots de commentaire, lignes sans code de l'inventaire.
- D4 (clés de table).
- Toute relance de la sonde, par exemple pour relever `sin`/`cos` hors des 64 points.
- Modifier `tools/float_audit.sh` ou son test ; réparer l'outil d'observation (arrêté depuis D1).
- Le volet `malloc` (§8), l'audio, et tout lancement de l'application raylib.

## Further Notes

- **Prototype de cadrage** (bac à sable, non versionné ; même formule et même arrondi que le C) :
  - ancien calcul contre bloc E : 128/128 au bit près avec l'arrondi au plus proche ; 62/128 faux avec la troncature ;
  - table quart d'onde pleine résolution repliée : 0 écart sur 65 536 fractions pour `sin`, 0 pour `cos`, contre l'ancien calcul sur la même fraction ;
  - ancien calcul sur l'argument complet contre ancien calcul sur sa fraction : 0 écart sur 200 000 bruts aléatoires ;
  - 257 entrées + interpolation linéaire entière : écart max 1 LSB, 14 416 fractions différentes sur 65 536.
  Ces chiffres guident la PRD ; ils ne remplacent ni les tests ni l'audit.
- **Écart avec le texte du plan.** La puce D5 dit « Divergente, avec re-baseline » et appelle la table « approximation non sourcée » hors des points. Avec la table choisie, D5 est attendue préservatrice, et, hors des points, la table n'est pas une nouvelle approximation : elle garde l'arrondi que le port produisait déjà, toujours sans source PICO-8. Le chemin divergent reste entièrement spécifié. L'« État de D5 » l'écrit, pour que l'humain puisse le contester.
- **Écart avec le modèle Celeste.** Même principe (table indexée par la fraction de tour, lecture directe, pas d'interpolation) ; résolution différente (65 536 pas par tour au lieu de 360), parce que l'argument 16.16 n'a que 65 536 fractions et que chaque pas de moins crée des valeurs sans source.
- **Coût mémoire, à connaître pour PSX** : 16 385 × 4 = 65 540 octets en lecture seule, environ 3 % des 2 Mo de la PSX. Réductions possibles plus tard (8e d'onde avec `cos`, encodage 16 bits avec un cas particulier pour les ~40 entrées qui valent 1, ou table plus petite avec divergence assumée), toutes hors de cette étape.
- **Écart avec la consigne reçue** : la consigne évoquait un test qui « analyse ou recopie » le bloc E ; la PRD recopie, comme les tests des blocs B, D et E `sqrt`.
