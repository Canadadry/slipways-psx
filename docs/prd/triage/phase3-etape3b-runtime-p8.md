---
title: "Plan 5, sous-étape 3b — runtime PICO-8"
description: "Les symboles du runtime PICO-8 émulé (p8/) passent en p8_, avec les deux outils qui écrivent ou lisent ces noms en texte (générateur de la table de sinus, comparateur de constantes), sans qu'une garde ni constants-check ne passe à vide."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3b. **Runtime PICO-8.** `p8/` : `fixed`, `quarter_wave`, `mem`, `gfx`, `palette`, `str`, `input`, `audio`, `assets`, `fatal` ; `tools/gen_quarter_wave.c` et `tools/constants_compare.sh` suivent. `pico8_input_load/save/append/free` et `pico8_fatal`, définis hors de `p8/`, passent en 3k et 3l. Tests : `test_fixed`, `test_mem`, `test_gfx`, `test_demo`, `test_str`, `test_input`, `test_asset_loading`. Résidu ≤ 100.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), deuxième étape. Entrées : `docs/plan-5.md` §4 (préfixe `p8_` pour les seuls symboles du runtime émulé), §5 (invariant, cibles de fin de PRD), entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et **toute la note « Découvert » de 3a**, qui fixe la méthode : on y renvoie sans la recopier (ordre des commandes, règle d'appartenance des chaînes, angle mort des littéraux composés, prévision des hachages, `scope-count`, porte `rename-rekey-check` et non `rename-table-check`). Prior art : commits `2a4bef8` (application de 3a) et `755609a` (chaînes de 3a). Consigne `guideline` chargée par l'agent rédacteur lui-même (aucun humain ni agent frais) aux points « vérifier avec l'utilisateur » de `to-prd` : modules, tests et découpage tranchés sur le texte du plan, sans interview.

**Condition d'entrée** : `origin/plan-5-noms-anglais` = HEAD de départ (`a52720d` ou descendant), arbre propre, `make test-all` vert ; `make rename-extract` frais sans ligne `warning:`, puis `make rename-rekey-check SUBSTEP=3b` vert.

## Problem Statement

Après 3a, le runtime PICO-8 émulé porte encore partout le préfixe du cart (`pico8_fixed_t`, `pico8_spr`, `PICO8_FIXED_LIT`, `pico8_gfx_t`…), en violation du §4 qui réserve `p8_` à ces seuls symboles. Ce sont les noms les plus lus du code : 179 identités renommées, **5 326 occurrences AST dans 201 fichiers**, de `src/` entier et des tests.

Ce qui rend 3b délicate n'est pas le volume (le compilateur arbitre) mais quatre endroits où ces noms vivent **en texte**, sans filet du compilateur :

- **`tools/constants_compare.sh`** (gardé par `make constants-check`, dans `make test`) repère les appels par la chaîne `PICO8_FIXED_LIT`, les découpe à une **position fixe** (`substr(call, 29, …)`, qui suppose la longueur du nom de la macro) et engendre un programme C qui déclare `pico8_fixed_t`. Ce fichier n'a aucune ligne dans le TSV : `rename-strings-verify` ne le voit pas, seul le compteur anti-cart y trouve 5 mentions en `string`.
- **`tools/gen_quarter_wave.c`** écrit par `printf` l'en-tête engendré `p8/quarter_wave.h` (2 056 lignes) : garde `PICO8_QUARTER_WAVE_H` et tableau `pico8_quarter_wave`. `rename-apply` renomme l'en-tête, pas les chaînes du générateur : sans elles, `make sin-table` (hors suite) régénérerait silencieusement les anciens noms.
- **`tests/test_constants_check.c`** plante 16 appels `PICO8_FIXED_LIT(…)` dans des littéraux composés, compilés contre le vrai `p8/fixed.h` ; **`tests/test_composition_residue.c`** plante un champ `pico8_fixed_t` dans une fixture de `parente`. Ces deux tests sont des modules de 3l, mais leurs lignes `chaîne` sont de 3b (écrivain `p8/fixed.h`, règle 5).
- **Gardes à motifs** : `SPEC_EXCLUSION` de `tools/data_residue.sh` exempte `pico8_spr_default` et `pico8_fixed_rnd_default` (inventaire de 3a : classe TSV, sous-étape 3b), écrits dans un groupe d'alternative que le compteur anti-cart ne voit pas ; et `usage-mutuel` de `composition_residue.sh` écarte les jetons `PICO8` / `PICO8_` (motif générique), alors que 3b introduit les premiers `P8_`.

## Solution

Une seule tranche, selon l'ordre de commandes de la note de 3a, sur `SUBSTEP=3b` : application des 179 identités, traductions de garde, recâblage, puis les 20 sites de chaînes et les mentions de `tools/constants_compare.sh`, recâblage à nouveau, relevés et note « Découvert ». À la fin : aucun ancien nom de 3b en `identifier` ni en `string` dans le dépôt ; les fichiers de 3b ne gardent que les 100 mentions de sous-étapes suivantes (`pico8_input_load/save/append/free` de 3k, `pico8_fatal` de 3l, `pico8_fixed_from_double`/`to_double` de `devtools/fixed_test_double`, `tmp/pico8_demo_dump.txt` ligne `fichier` de 3l…) ; `make constants-check` rend **la même table** qu'avant ; `make sin-table` reproduit l'en-tête renommé à l'octet ; pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3b, je veux partir d'un TSV dont chaque identité restante se résout (`rename-rekey-check SUBSTEP=3b` vert sur une extraction fraîche), afin que `rename-apply` ne refuse aucune entrée.
2. En tant qu'agent de 3b, je veux appliquer les 179 identités de `rename-slice SUBSTEP=3b` en une commande, afin que le compilateur arbitre tout oubli dans les 201 fichiers touchés.
3. En tant que lecteur du code, je veux que les symboles exportés du runtime portent `p8_` (`p8_fixed_t`, `p8_spr`, `p8_graphics_t`, `p8_memory_t`, `P8_FIXED_LIT`…) et que ses `static` perdent tout préfixe (`isqrt_u64`, `quarter_wave_at`, `FONT_ROWS_A`, `min`, `max`), afin que le §4 se lise dans le code.
4. En tant que lecteur du code, je veux que les champs de `p8_rgb_t` s'appellent `red`, `green`, `blue` et le paramètre de `p8_print` `character`, afin que `scope-count` ne trouve plus rien dans 3b.
5. En tant que mainteneur, je veux que `make constants-check` rende exactement la table d'avant (34 littéraux, 61 sites, mêmes lignes) une fois le comparateur traduit, afin de prouver que la traduction ne change pas ce qu'il compare.
6. En tant que mainteneur, je veux que la position du littéral dans un appel ne dépende plus d'une constante calée sur l'ancien nom de la macro, ou qu'elle soit recalée et prouvée par `test_constants_check`, afin qu'un renommage ne casse pas le découpage en silence.
7. En tant que mainteneur, je veux que `test_constants_check` reste le test de vivacité du comparateur, ses appels plantés écrits avec `P8_FIXED_LIT`, afin de savoir que le comparateur traduit mord encore (forme stricte, littéral absent du bloc A, régression).
8. En tant que mainteneur, je veux que `make sin-table` régénère `p8/quarter_wave.h` identique à l'en-tête renommé par l'AST, afin que la prochaine régénération ne réintroduise pas les anciens noms.
9. En tant que mainteneur, je veux que les deux exemptions de `SPEC_EXCLUSION` qui visent 3b soient traduites sans être retirées, et leur état (mortes tant que le motif générique `pico8_X_default` n'est pas tranché) noté, afin que l'humain décide au point d'arrêt d'avant l'étape 5.
10. En tant que mainteneur, je veux que `composition_residue.sh --detail` et `data_residue.sh --detail` soient identiques avant et après, et que `usage-mutuel` ne s'allume pas sur les nouveaux jetons `P8_`, afin qu'aucune garde ne change de verdict par un renommage.
11. En tant qu'agent de 3b, je veux renommer à tous leurs sites les 20 lignes `chaîne` de 3b (5 noms de test, 7 chemins `tmp/test_input_*`, 8 symboles cités dont ceux de `test_constants_check.c`, `test_composition_residue.c`, `gen_quarter_wave.c`), afin que `rename-strings-verify SUBSTEP=3b` soit vert.
12. En tant qu'agent de 3b, je veux chercher aussi mes noms à l'intérieur des littéraux composés, afin de ne pas reproduire l'angle mort constaté en 3a.
13. En tant qu'agent de 3b, je veux une liste prévue des tests à hachage figé touchés par mes chaînes (attendue vide), afin qu'un test rouge imprévu arrête la tranche au lieu d'être recopié.
14. En tant qu'agent de 3c, je veux trouver un TSV recâblé après l'application et après les chaînes, `rename-rekey-check` vert de 3b à 3l, afin de partir sur une table saine.
15. En tant qu'agent de 3k et 3l, je veux que `pico8_input_load/save/append/free` et `pico8_fatal` soient laissés, afin de les renommer avec leur module de définition.
16. En tant que relecteur humain, je veux une note « Découvert » sous 3b qui donne les relevés de fin et les écarts rencontrés, afin de trancher les exceptions et les gardes au point d'arrêt.
17. En tant que joueur, je ne veux voir aucun pixel changer, afin que le renommage ne soit qu'un renommage.

## Implementation Decisions

### Périmètre et appartenance

- Modules : ceux de la ligne 3b de `tools/rename_substeps.tsv` (dix modules de `p8/`, `tools/gen_quarter_wave`, `tools/constants_compare`, sept tests). `rename-slice SUBSTEP=3b` rend 179 identités (`p8/gfx` 95, `p8/fixed` 34, `p8/mem` 14, `p8/input` 8, `p8/audio` 7, `p8/palette` 7, `p8/assets` 5, `p8/str` 4, `p8/fatal` 2, `p8/quarter_wave` 2, `tests/test_gfx` 1). Le TSV dit les noms neufs : on y renvoie, on n'en recopie rien.
- Laissés à leur sous-étape (ils font le résidu) : `pico8_input_load/save/append/free` (3k), `pico8_fatal` (3l), `pico8_fixed_from_double`/`pico8_fixed_to_double` (`devtools/fixed_test_double`, 3k), la ligne `fichier` `tmp/pico8_demo_dump.txt` (3l), le nom de fichier `tools/pico8_reference_probe.p8` (3l).
- Sonde de référence PICO-8 : `tools/pico8_reference_probe_out.txt` ne porte aucun nom C et n'est **jamais** régénéré (il faut le vrai PICO-8) ; le `.p8` ne cite un nom C qu'en commentaire (étape 4l). Rien à y faire en 3b.
- Hors 3b, à l'étape 4 : les mentions en commentaire du `Makefile` (`PICO8_FIXED_LIT`, `pico8_fixed`), de `constants_compare.sh` (2 lignes d'en-tête), de `gen_quarter_wave.c`, et les messages d'`assert` de `game/hud.c` (`pico8_rectfill`) et `game/slipway_display.c` (`pico8_line`), que le compteur range en `comment`.

### Application et recâblage

- Ordre des commandes : celui de la note « Découvert » de 3a, avec `SUBSTEP=3b`. Prévision du recâblage après l'application, par la méthode de 3a : ≈ 851 identités à offset des sous-étapes suivantes situées après un token réécrit ; aucune identité restante ne contient dans son USR une fonction renommée par 3b. Un second recâblage suit les chaînes (noms de test et chemins `tmp/` raccourcis).
- Macros, déjà couvertes par 2a et à constater : `PICO8_FIXED_LIT` porte le seul `##` du dépôt (`raw##u`), dont aucun opérande n'est renommé — l'apply le renomme ; `PICO8_NORETURN` est `multidef` (`#ifdef`/`#else` de `p8/fatal.h`), une identité. Un refus de l'apply sur l'un d'eux est un défaut de l'outil, corrigé et noté dans le même commit.
- `p8/quarter_wave.h` est renommé par l'apply comme tout en-tête ; les chaînes `printf` de `tools/gen_quarter_wave.c` suivent avec les chaînes. Preuve : `make sin-table` puis `git diff --exit-code src/p8/quarter_wave.h` (constaté avant 3b : le générateur reproduit l'en-tête actuel à l'octet sur cette machine).

### `tools/constants_compare.sh`

- Traduit à la main (pas de ligne TSV) : nom de la macro dans la recherche, le motif strict et les messages d'erreur ; `p8_fixed_t` dans le programme engendré. Le découpage du littéral ne doit plus reposer sur une position calée sur l'ancien nom : soit il se déduit du nom, soit la constante est recalée — dans les deux cas `test_constants_check` le prouve.
- Preuve d'équivalence : la table complète de `constants_check` (fichier `table.txt` du répertoire de travail, pas seulement la ligne des comptes `litteraux 34 | sites 61 | ok 34 | ecart 0`) est identique avant et après. `tools/constants_check.sh` et `tools/constants_expected_gaps.txt` ne portent aucun nom de 3b et ne bougent pas.
- Le compteur anti-cart doit trouver `tools/constants_compare` à 0 en `string` ; ses 2 mentions en commentaire restent (étape 4).

### Chaînes

- Les 20 sites de `rename-strings-verify SUBSTEP=3b`, tous à renommer : noms de test `test_pico8_{fixed,gfx,mem,str,input}` ; 7 chemins `tmp/test_pico8_input_*.json` ; symboles cités `PICO8_FIXED_LIT` (`p8/fixed.h`, `tests/test_constants_check.c` — 16 littéraux composés), `pico8_fixed_t` (`p8/fixed.h`, `tests/test_composition_residue.c`), `PICO8_QUARTER_WAVE_H` et `pico8_quarter_wave` (`p8/quarter_wave.h`, `tools/gen_quarter_wave.c`). Les sites dans `p8/` sont déjà réécrits par l'apply.
- Littéraux composés : recherche de chaque ancien nom de 3b dans tout littéral C (mesurée : `test_constants_check.c`, `test_composition_residue.c`, `gen_quarter_wave.c`, plus les deux messages d'`assert` hors périmètre ci-dessus) ; à refaire au moment de l'édition.
- Hachages : liste prévue **vide** (aucun `fnv_str` ni pli ne porte un nom de test, un chemin `tmp/` ou un symbole de 3b ; à revérifier avant l'édition). Tout test rouge après les chaînes arrête la tranche (`help-me-debug`).

### Gardes à motifs

- `SPEC_EXCLUSION` : `pico8_spr_default` et `pico8_fixed_rnd_default` traduits en `p8_spr_default` et `p8_fixed_rnd_default`, les cinq noms de 3d inchangés, rien de retiré. Après 3b ces deux exemptions sont mortes (le motif `spec` ne voit que `pico8_X_default`), comme l'exclusion `lien-parent` en 3a : pas de test de vivacité possible, état noté pour la décision humaine déjà proposée par 3a (traduire le motif générique sans préfixe, ou retirer).
- `usage-mutuel` : mesurer `composition_residue.sh` (comptes et `--detail`) avant et après. S'il change à cause des jetons `P8` / `P8_`, 3b est la sous-étape du premier module concerné : l'exclusion générique est étendue à `P8`/`P8_` (sans retirer `PICO8`), avec un test de vivacité dans `tests/test_composition_residue.c`, et la note le dit ; sinon rien ne bouge et la note le constate.
- Aucun autre élément de l'inventaire de 3a ne vise un nom de 3b.

### Fin de PRD

Liste du §5 et de la note de 3a, sur `SUBSTEP=3b` : `make test-all` vert, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent (jamais lancés ; `platform/raylib_backend` lit `p8_rgb_t` et ses champs) ; `rename-tool-test` vert ; `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3a ; `data-residue` et `composition-residue` aux mêmes comptes ; `constants-check` même table ; `sin-table` sans diff ; après `rename-extract` frais, `cart-names-count SUBSTEP=3b` sans `warning:`, `done` 0 en `identifier` et `string`, `later identifier` ≤ 100 ; `rename-strings-verify SUBSTEP=3b` vert, 0 `unseen` ; `rename-rekey-check` vert de 3b à 3l ; `scope-count SUBSTEP=3b` à 0 (4 avant : `r`, `g`, `b`, `c`).

## Implementation Chunks

1. **Sous-étape 3b entière, note comprise.** Une seule tranche : le comparateur de constantes lie l'application (nom de la macro dans `p8/fixed.h`) aux chaînes (`test_constants_check.c`) et à `tools/constants_compare.sh` — séparés, `constants-check` et `test_constants_check` rougissent. Un commit de code (application, traductions de garde, chaînes, comparateur, générateur, TSV recâblé deux fois) qui laisse `make test-all` vert, puis la note « Découvert » sous l'entrée 3b dans le même commit ou un commit de docs qui suit : identités et occurrences appliquées, recâblages (prévu ≈ 851 / constaté), sort des macros `##` et `multidef`, table de `constants-check` identique, `sin-table`, exemptions de `spec` mortes, verdict sur `usage-mutuel`, littéraux composés trouvés, hachages (aucun), relevés de fin, exceptions de portée proposées s'il y en a. Plan corrigé dans le même commit si un constat le contredit.

## Testing Decisions

- Code du jeu et du runtime : aucun test nouveau. Filet = compilateur, `make test`, e2e à l'octet, `constants-check` à table égale, `sin-table` sans diff.
- `tests/test_constants_check.c` est le test de vivacité du comparateur traduit : ses cas existants (forme stricte, littéral absent du bloc A, aucune constante, régression, liste périmée) passent avec `P8_FIXED_LIT` ; aucun cas n'est retiré.
- Garde de composition : un test de vivacité dans `tests/test_composition_residue.c` seulement si l'exclusion de `usage-mutuel` doit être étendue (une paire plantée qui ne diffère que par un jeton `P8_` ne doit pas s'allumer ; une vraie paire s'allume encore).
- Outils de méthode (`rename-*`, `scope-count`) : inchangés ; si l'un a un défaut, il est corrigé avec son test dans le même commit, comme en 3a.

## Out of Scope

- Les noms de 3c–3l rencontrés dans les fichiers de 3b (résidu), dont `pico8_input_*` et `pico8_fatal`.
- Commentaires, messages d'`assert`, mentions entre backticks ou nues, `Makefile` compris : étape 4.
- Trancher les motifs génériques (`pico8_X_default`, `pico8_X(`…) et les exemptions mortes : décision humaine au point d'arrêt d'avant l'étape 5.
- Régénérer `tools/pico8_reference_probe_out.txt`, renommer `tools/pico8_reference_probe.p8` ou les binaires.
- Scinder `p8/input.h` (§7).
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD (arbre `a52720d`, extraction de fin de 3a) : `cart-names-count SUBSTEP=3b` `done` 1 795 `identifier`, 20 `string` ; `later` 100 `identifier` (borne 100), 2 `string` (`tmp/pico8_demo_dump.txt`, 3l) ; `rename-strings-verify SUBSTEP=3b` 20 sites, 0 `unseen` ; `scope-count SUBSTEP=3b` 4 ; `constants-check` `litteraux 34 | sites 61 | ok 34 | ecart 0` ; occurrences par origine : `ref` 2 189, `type_ref` 1 659, `macro_arg` 643, `macro_use` 576, `decl` 202, `macro_def` 39, `macro_cond` 10, `macro_body` 8.
- `rename-table-check` reste rouge par construction (note de 3a) : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` ≈ 1 min 30 ; la tranche en enchaîne quatre ou cinq.
