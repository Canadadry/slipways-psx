---
title: "Plan 5, morceau 4l1 — messages, légende et commentaires du Makefile (et tools/e2e_long_exceptions, vide)"
description: "Mettre le Makefile en anglais — dix-neuf messages de recette et d'$(error), toute la légende et les commentaires du corps, renvois périmés et affirmations fausses corrigés — sans changer une cible, une variable, un prérequis ni une recette, prouvé par la base de données de make (égale par la table de traduction pour les messages, identique à l'octet pour les commentaires), et ramener à zéro anti-cart et français en commentaire sur MODULES=<4l1>."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 4. **Commentaires, asserts, locales** — une PRD par sous-étape, sur la même liste qu'à l'étape 3. La note « Découvert » de la première sous-étape corrige la consigne des agents avant les suivantes. Un agent par module : commentaires et messages d'`assert` réécrits en anglais sans citer le cart (§4), mots français restants retirés, locales cryptiques à longue portée renommées. Le skill `reprendre-les-commentaires` cadre le travail. Un troisième compteur, de **français résiduel** (commentaires, chaînes, asserts), est branché au début de l'étape. Fin d'une sous-étape : les trois compteurs à zéro sur ses modules, toutes catégories, mentions entre backticks comprises.
>
>    **Sous-étapes** : les modules, tests et données de `tools/` de la sous-étape 3 de même lettre ; l'étape 3 finie, aucun résidu d'une sous-étape suivante ne reste.
>
>    **Méthode de l'étape 4** (consultation `plan-review` de fin d'étape 3) :
>    - *Trois sortes de commit, trois preuves.* Commentaires seuls : sortie de `gcc -fpreprocessed -dD -E -P` identique avant et après, nombre de lignes de code inchangé, ni essai à blanc ni TSV. Chaînes et messages d'`assert` : la rigueur de l'étape 3 (essai à blanc, test rouge avant la correction, tampons vérifiés). Locales renommées : par `rename-slice`/`rename-apply` seulement, ajoutées au TSV et aux surcharges.
>    - *Propriété des chaînes.* Une chaîne française ou qui cite le cart, lue par un test ou par la sortie d'un binaire (`expect_fatal`, `must_abort_saying`, noms de zone, sortie `REPLAY_FILL`), est traduite par la sous-étape qui tient son site écrivain, à tous ses sites d'un coup ; le commit de chaînes liste d'abord ses sites lecteurs hors de ses modules.
>    - *Extraction fraîche* (corrigée par 4a). Chaque 4x finit par `rename-extract` frais sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues), `rename-rekey-check SUBSTEP=3l` vert, puis les compteurs. `rename-rekey-check` sur les autres sous-étapes est rouge par construction depuis l'étape 3 (chaque ligne appliquée a perdu son ancien nom) : il n'est pas une porte. `rename-rekey` ne sert qu'à une 4x qui ajoute des lignes de locale, écrites depuis une extraction de l'arbre courant et appliquées dans le même commit, puisque les éditions de commentaires déplacent les lignes et que `rename-rekey` apparie par ligne. Les outils reçoivent la sous-étape 4x sous le nom `SUBSTEP=3x` ; un morceau (4c1…) sous `MODULES=` et sa liste.
>    - *Locales cryptiques.* 4a définit une liste candidate mesurable tirée de `declarations.tsv` (locale ou paramètre de portée > 40 lignes, nom de trois caractères ou moins ou non découpé en mots) ; chaque sous-étape la relève avant et après. Pour les 12 statiques locales proposées en exception, l'étape 4 propose aussi un nom dans « Découvert », pour que l'un ou l'autre verdict s'applique mécaniquement à l'étape 5.
>    - *Compteur de français.* Construit par 4a, d'abord, test d'abord : liste de mots-outils français choisis plus détection des accents (le code est en ASCII sans accents) ; fichier d'exceptions au format de `scope_exceptions.txt` ; mêmes colonnes et même scission fait / suivantes que `cart-names-count` ; jamais bloquant ; fixtures de détection, d'homographes (`place`, `file`, `limite`) et d'exception. Sa mesure de départ par sous-étape va dans la note de 4a ; une sous-étape hors norme est coupée, comme 3l.
>    - *Consigne des agents.* La PRD de 4a porte une version étape 4 de `consigne-agent.md` de `reprendre-les-commentaires`, qui lève point par point ses contradictions avec l'étape (code non modifié, assertions intouchables, chaîne humaine signalée sans correction, sortie du préprocesseur identique, destination glossaire) ; la note de 4a l'amende ensuite.
>    - *Citations du cart.* Relevé non bloquant par sous-étape, avant et après : commentaires qui citent `cart` (hors module `game/cart`), `Lua`, `lua.lua`, et noms du TSV d'une ou deux lettres employés nus.
>    - *Registre des décisions humaines.* `docs/decisions-plan-5.md` : une entrée numérotée par question (origine, question, options, nombre de sites, proposition prête à appliquer), ouverte par 4a à partir des notes de 3a–3l, complétée par chaque 4x, close par 4l. Une décision de nommage précise qu'elle s'applique aussi aux mentions entre backticks et en commentaire, avec un contrôle sur le nom remplacé.
>
> 3l. **Plateforme, binaires, outils.** `platform/` : `raylib_backend`, `raylib_backend_coords`, `fatal_pc` ; `cmd/` : `game`, `replay`, `frames`, `gif`, `minimize`, `screencast_viewer`, `class_coverage`. Binaires et sorties de `tmp/` (lignes `fichier`) dans le `Makefile`, les tests et `tools/` (`frames_ref.sh` par la voie (a), `float_grep_exclusions.txt`) ; `tools/composition_residue.sh` et `composition_residue_sites.txt`, `tools/minimize.sh`, `tools/pico8_reference_probe.p8` y ferment leurs dernières mentions. Tests : `test_raylib_backend_coords`, `test_frames_tool`, `test_replay_from_title`, `test_replay_usage`, `test_line_coverage`, et ceux des gardes de `tools/` : `test_alloc_guard`, `test_composition_residue`, `test_constants_check`, `test_data_residue`, `test_e2e_cleanup`, `test_e2e_limits`, `test_float_grep`, `test_float_guard`, `test_float_link`, `test_frames_check`. Fin : compteur à zéro partout, `identifier` et `string`.
>
> 4l. **Commentaires — plateforme, binaires, outils.** Modules, tests, scripts et données de `tools/` de 3l, et le `Makefile`. Coupée par 4a (charge 4 073, dont les fichiers rangés en 3l par 4a) :
>    - 4l1 (1 155) : `Makefile` (légende comprise), `tools/e2e_long_exceptions`.
>
>    Un morceau (4c1…) suit la PRD de sa sous-étape, avec ses commits et `make test-all` vert ; ses compteurs se lisent par `MODULES="<sa liste>"`, et `SUBSTEP=3x` à zéro clôt la sous-étape après son dernier morceau.

4l1 est le premier morceau de 4l : le `Makefile` (légende comprise) et `tools/e2e_long_exceptions.txt`. Ses compteurs se lisent par `MODULES="Makefile tools/e2e_long_exceptions"` (noté `MODULES=<4l1>` ci-dessous). `tools/e2e_long_exceptions.txt` est **vide** (0 octet, intouché depuis `3debeea`) : tout le travail est dans le `Makefile`. 4l1 ne clôt pas 4l : `SUBSTEP=3l` ne tombe à zéro qu'après 4l4.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4). Étape 4l1 = l'entrée 4 appliquée au morceau 4l1, tel que la note de 4a l'a découpé (entrée 4l de `docs/plan-5.md`, liste ci-dessus).

Entrées lues : `docs/plan-5.md` §1, §2, §4, §5, l'entrée 4 et sa méthode, l'entrée 3l et sa note « Découvert », les notes de 4a, 4b, 4c1, 4c2, 4d, 4e, 4f1, 4f2, 4g, 4h, 4i, 4j, 4k1, 4k2 et 4k3 ; la PRD de 4a (`91f99a6`), qui dit : « Les lignes neuves de la légende du `Makefile` s'écrivent en anglais. Le reste de la légende, en français, revient à 4l. » ; `docs/consigne-etape-4.md` (tous ses amendements, de 4a à 4k3) ; `docs/decisions-plan-5.md` (D-1 à D-54 ; aucune entrée sans verdict n'est appliquée ici) ; le skill `reprendre-les-commentaires` (`SKILL.md`, `consigne-agent.md`, `modele-rapport.md`) ; le `CLAUDE.md` (« Dans un `Makefile`, seule l'aide à la lecture d'une commande reste » ; « La légende des cibles est en tête du `Makefile` ») ; la PRD de 4k3 (`04a5385`, modèle de forme) et `git log -5 --stat` : 4k3 s'est commitée en trois commits de code — chaîne (`refactor:`, `9ae939e`), locales avec TSV et surcharges (`refactor:`, `8192613`), commentaires avec note, consigne et registre (`docs:`, `f5764e4`) — puis la PRD graduée (`ce65f6e`).

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même le skill `guideline` aux points « vérifier avec l'utilisateur » de `to-prd` (modules, tests, découpage) et l'a appliqué (changements chirurgicaux, critères vérifiables, hypothèses dites, aucune décision du registre préjugée). Sa règle 5 (« un commentaire est faux, une odeur ou un aveu ») cède devant la règle des commentaires du `CLAUDE.md` et la consigne de l'étape 4, qui priment dans ce dépôt. Les décisions reposent sur le plan et sur des mesures à `ce65f6e`, après une extraction fraîche ; écritures dans `tmp/` seulement (sorties dans `tmp/4l1_baseline/`). L'état 1 a été **joué** dans un clone local jetable hors du dépôt (répertoire de travail de l'agent, `third_party/` lié, jamais poussé, effacé ensuite) : chaque chiffre de l'état 1 ci-dessous est une observation. Le rejeu est gardé sous `tmp/4l1_baseline/replay/` : `stage1.sed` (la table de l'état 1, 19 expressions sur les lignes sources), `output.sed` (la même table sur les sorties imprimées), `s1.patch` (vérifié par `git apply --check` sur `HEAD`), `capture.sh` (base de données de `make` et `make -n` des cibles touchées, plus les quatre `$(error)`), `run_targets.sh` (sorties réelles des cibles dont un message change), `before/` et `after/` (leurs sorties, avant et après l'état 1, `testall.log` compris), `readers.txt` (le `git grep -nF` de chaque fragment), `db_proof.sh` (la preuve de l'état 2) et `traps.txt` (deux pièges de cette preuve, rejoués). Les scripts de relevé sont `tmp/4l1_baseline/counters.sh` (compteurs de `MODULES=<4l1>`, `SUBSTEP=3l` et sans `SUBSTEP`), `tmp/4k3_baseline/side.sh` et `others.sh`, repris tels quels. Un prototype à relire, pas à appliquer les yeux fermés. L'état 2 (les commentaires) n'a pas été rédigé : seule sa preuve a été jouée.

**Condition d'entrée** : `origin/plan-5-noms-anglais` à `ce65f6e` ou un descendant (la PRD seule au-dessus), arbre propre, `make test-all` vert (26 e2e) ; `make rename-extract` frais sans `warning:` (4 lignes `partial` de `class_coverage` attendues) ; `rename-rekey-check SUBSTEP=3l` vert (0 introuvable) ; `rename-strings-verify SUBSTEP=3l` : 1 site en défaut sur 44 (le repli de `tools/frames_ref.sh`, D-1), 9 `unseen` (`tmp/4l1_baseline/rsv.txt`) ; la mesure de départ ci-dessous retrouvée à l'identique. Sorties de `class-coverage-net`, `e2e-gap` et `tmp/line_coverage_gap.txt` relevées au départ (`tmp/4l1_baseline/ccnet.txt` (sans lignes `cc`), `e2egap_raw.txt`, `line_coverage_gap.txt` ; 2 932 lignes de gap ; les deux cibles rouges, code 2 ; égales à l'octet à celles de `tmp/4k3_baseline/`) ; `composition_residue.sh --detail`, `--champs` et `data_residue.sh --detail` relevés (`cr_detail.txt`, `cr_champs.txt`, `dr.txt`, égaux à l'octet à ceux de 4k3 ; les scripts se lancent par `bash`).

## Problem Statement

Le `Makefile` est la porte d'entrée de tout le dépôt : `CLAUDE.md` renvoie à sa légende, chaque skill lance `make test` ou `make e2e`, chaque PRD de l'étape 4 cite ses cibles. C'est aussi le dernier gros bloc de français du code hors de `tools/` et `src/cmd/` :

- sa légende (lignes 1–200) est en français sauf les cinq entrées que 4a et la suite ont écrites en anglais (`cryptic-locals`, `cart-citations-test`, `cart-citations`, `french-count-test`, `french-count`) ; ses 82 lignes de commentaire du corps aussi ; `french-count` y compte 963 mots en commentaire ;
- dix-neuf messages de recette sont en français (`e2e: 26 e2e dans tests/e2e`, `écart: … ni couvertes par make e2e …`, `sanitize: 26 propre(s)`, `EVENTS manque : …`…), dont huit que le compteur ne voit qu'à moitié ou pas du tout (`manque`, `sale`, `propre(s)`, `apport`, `écart` hors accent : mots de contenu) ;
- quatre commentaires nomment des symboles disparus (`pico8_cart_load()`, `pico8_data_init()`, `PICO8_FIXED_LIT` ×2, comptés par l'anti-cart ; `pico8_fixed`, non compté) ;
- plusieurs affirmations sont fausses ou renvoient à des documents disparus : `ENGINE_SRCS` dit exclure « les deux points d'entrée (cmd/replay.c, cmd/game.c) et les modules qui ne servent qu'au driver interactif », quand il exclut les sept `src/cmd/*.c` et onze autres fichiers (huit de `devtools/`, `p8/palette.c`, deux de `platform/`) ; la légende de `make test` ne liste que sept de ses dix-neuf prérequis ; celle de `class-coverage-net` commence par « idem », placée après `e2e-judge` dont elle n'est pas la variante ; `float-guard`, `float-grep`, `float-link` renvoient à des « §6.1 », « §6.2 », « §6.3 » qu'aucun document ne porte plus ; `data-residue` et `composition-residue` parlent de « la phase D » et de « la phase 2 » d'un chantier fini ;
- le relevé des citations y compte 168 mentions : 152 `short` (articles français, mais aussi des options et des motifs de fichiers : `-j`, `-p`, `.c`, `%.o`, `<n>`, `(re)`), 14 `cart` (dont « l'écart », que la regex lit `cart`, et les noms des cibles `cart-names-count` et `cart-citations`) et 2 `lua`.

Le `Makefile` n'est pas un fichier C : la preuve de l'étape 4 pour les commentaires (sortie de `gcc -fpreprocessed` identique) ne s'y applique pas, et celle des scripts (point 8 de la consigne, `diff` des lignes hors `#`) est aveugle à deux erreurs propres à `make` (rejouées, ci-dessous). Et les noms de ses cibles ne doivent pas bouger : `CLAUDE.md`, `README`, les skills, les docs et deux assertions de `tests/test_frames_check.c` (`"make e2e-ref"`, `"make e2e-baseline"`) les citent.

## Solution

Appliquer à 4l1 la méthode de 4a à 4k3, en morceau (`MODULES=<4l1>`), en deux états de code :

1. **Messages** (`refactor:`) : les dix-neuf messages de recette et d'`$(error)` traduits par une table (`stage1.sed`), aucune ligne ajoutée ni retirée ; aucun lecteur, donc aucun rouge possible (amendement de 4i) ; preuve : la base de données de `make` sans ses lignes `#` égale à l'octet par la table, et la sortie réelle de chaque cible touchée égale à l'octet par la table de sortie (`output.sed`), codes de sortie compris.
2. **Commentaires** (`docs:`), avec la note « Découvert », les amendements de la consigne et le registre : légende et corps en anglais, renvois corrigés, affirmations fausses corrigées ; preuve : la base de données de `make` sans ses lignes `#` **identique à l'octet**, plus le `diff` des lignes hors commentaire de la consigne (qui seul voit les `$(error)` des conditionnelles non prises).

Aucune cible, aucune variable, aucun prérequis, aucune recette ne change hors des littéraux de l'état 1. L'exécutant ne commite pas : il laisse un patch et deux messages dans `tmp/`, et l'arbre de travail à l'état final (§ « Remise »). À la fin, sur `MODULES=<4l1>` : anti-cart, locales cryptiques et portée à zéro ; français à zéro en commentaire, et en `string` hors de trois écrivains d'autres morceaux et d'un faux positif proposé en exception ; citations réduites aux noms des cibles du compteur et à sa propre description, proposées au registre.

## User Stories

1. En tant qu'humain qui ouvre le dépôt, je veux une légende du `Makefile` entièrement en anglais, afin de trouver la bonne cible sans traduire.
2. En tant qu'humain ou agent qui suit `CLAUDE.md`, `README`, une skill ou une PRD, je veux que chaque nom de cible reste exactement le même (`test`, `e2e`, `test-all`, `raylib`, `raylib-vendor-setup`, `e2e-ref`, `e2e-baseline`, `cart-names-count`, `cart-citations`…), afin que chaque commande citée marche encore.
3. En tant que lecteur de la légende de `make test`, je veux qu'elle dise tout ce que la cible lance (les gardes et les tests des outils marqués « in make test »), afin de ne pas croire la boucle rapide plus petite qu'elle n'est.
4. En tant que lecteur de la légende de `class-coverage-net`, je veux qu'elle dise de quoi elle est la variante (`class-coverage`), afin qu'un « idem » placé après `e2e-judge` ne m'induise pas en erreur.
5. En tant que lecteur de `float-guard`, `float-grep`, `float-link`, `data-residue`, `composition-residue`, je veux que leur légende dise ce que la cible vérifie, sans renvoi à un « §6.x » ni à une « phase » de chantier disparus, afin qu'elle reste vraie sans document à côté.
6. En tant que lecteur de `ENGINE_SRCS`, je veux que son commentaire dise ce qu'il exclut vraiment (tous les points d'entrée de `src/cmd/` et les modules des outils et du jeu interactif), afin de savoir où ajouter un module neuf.
7. En tant que lecteur de `constants-check`, je veux que la légende et le commentaire nomment `P8_FIXED_LIT`, la macro que lit `tools/constants_compare.sh` depuis 4b, afin qu'un `grep` mène au code.
8. En tant que lecteur des listes de sources, je veux `cart_load()`, `data_init()` et `p8/fixed.c` à la place de `pico8_cart_load()`, `pico8_data_init()` et `pico8_fixed`, afin que les renvois existent.
9. En tant qu'humain qui lance `make e2e`, `make e2e-gap`, `make gif`, `make sanitize-net`, `make e2e-cleanup`, `make e2e-judge`, `make line-coverage[-contrib]` ou `make data-guard`, je veux leurs lignes de bilan en anglais, afin de lire une sortie d'une seule langue — hors des lignes imprimées par les scripts de `tools/` (4l2, 4l4).
10. En tant qu'humain qui oublie `EVENTS` ou `OUT`, je veux le refus de `replay`, `screencast`, `minimize` et `e2e-judge` en anglais, avec le même code de sortie (2) et avant toute compilation, afin de comprendre ce qui manque.
11. En tant que skill `check-e2e`, qui recopie verbatim la sortie de `make e2e`, je veux que seul le texte de la ligne de bilan change, pas les lignes `frames:` de `tools/frames_check.sh` (4l4) ni le code de sortie, afin que mon diagnostic reste lisible.
12. En tant que mainteneur de `tools/frames_ref.sh`, je veux que la ligne de règle `$(TMP_DIR)/replay:` reste intacte au début d'une ligne, afin que la voie (a) de 3l trouve toujours le pilote d'un worktree.
13. En tant que relecteur, je veux que l'état 1 ne change que dix-neuf littéraux, prouvés par la base de données de `make` égale par la table et par les sorties réelles égales par la table, afin de relire les messages seuls.
14. En tant que relecteur de l'état 2, je veux une preuve que la base de données de `make` n'a pas bougé d'un octet, afin d'être sûr qu'aucun commentaire n'a avalé une règle (un `\` en fin de commentaire) ni n'est devenu une ligne de recette (un `#` après une tabulation).
15. En tant que mainteneur des gardes de 4l2 et 4l4, je veux que les noms de catégories de `COMPOSITION_RESIDUE_GUARD` et `DATA_RESIDUE_GUARD` et le motif `grep '^garde '` de `float-guard` restent intouchés, afin que les traduise la sous-étape qui tient leur écrivain, à tous leurs sites d'un coup.
16. En tant que mainteneur du registre, je veux une entrée pour le faux positif `du` (la commande `du -ch` de la recette de `gif`) et une pour les citations `cart`/`lua` que la légende ne peut pas perdre (noms des cibles du compteur, description du relevé), et les numéros de ligne du `Makefile` cités par D-2 et D-28 remis à jour, afin que l'humain tranche avant l'étape 5 sur des renvois justes.
17. En tant que pilote du plan, je veux une note « Découvert » qui donne les compteurs de fin de 4l1 sur `MODULES=<4l1>`, `SUBSTEP=3l` et sans `SUBSTEP`, afin que 4l2 parte d'une mesure connue.
18. En tant qu'orchestrateur, je veux un patch et deux messages dans `tmp/`, afin de commiter les deux sortes de commit séparément, `make test-all` vert à chacun.

## Implementation Decisions

### Mesure de départ (`ce65f6e`, extraction fraîche, écrits dans `tmp/4l1_baseline/` seulement)

`make rename-extract` : aucun `warning:`, 4 lignes `partial` (`extract0.log`) ; `rename-rekey-check SUBSTEP=3l` : 0 identité introuvable (`rekey.txt`) ; `rename-strings-verify SUBSTEP=3l` : 1 en défaut sur 44, 9 `unseen` (`rsv.txt`) ; `make test-all` vert (rejoué dans le clone, `replay/before/testall.log`).

Sur `MODULES=<4l1>` :

| Compteur | id | backtick | string | comment | total |
|---|---|---|---|---|---|
| `french-count` | 0 | 0 | 39 | 963 | 1 002 |
| `cart-names-count` | 0 | 0 | 0 | 4 | 4 |
| `cart-citations` (cart/lua/short) | — | — | — | 14/2/152 | 168 |
| `cryptic-locals` | — | — | — | 0 | 0 |
| `scope-count` | — | — | — | — | 0 |

Charge **1 174** (1 002 + 4 + 168), contre 1 155 dans la note de 4a (à `b2481ab`) : +19 `short`, les « un » rendus comptés par le renommage `un` → `first_refusal` de 4f1 (« `Makefile` 19 », note de 4f1 ; complément de D-36). Sous le seuil de 1 382 : un seul morceau. `tools/e2e_long_exceptions` : 0 partout (fichier vide).

Sur `SUBSTEP=3l` : français `done` 0/0/549/2 888 (3 437), `later` 0 ; anti-cart `done` 0/0/2/38 (40), `later` 0 ; citations 21/4/665 ; `cryptic-locals` 17 ; portée 0. Sans `SUBSTEP` : anti-cart `TOTAL` 40 (0/0/2/38), citations 21/4/665, français 3 463 (575 `string`, 2 888 `comment`), `cryptic-locals` 19, `scope-count` 12 (D-13) — les chiffres de fin de 4k3. `SUBSTEP=3a` à `3j` : `tmp/4l1_baseline/others.txt`, égal à l'octet à `tmp/4k3_baseline/others.txt`.

Détail :
- *Français*, `string` (39, `fr_sites.txt`) : les messages de l'état 1 (lignes 952, 958, 965, 997, 1003, 1012, 1015, 1021, 1041, 1048, 1062, 1064, 1068, 1095 ; 32 mots) ; `du` de la commande `du -ch` (`:958`) ; `par` de la catégorie `par-nom` (`DATA_RESIDUE_GUARD`, `:1054`) ; `un`, `ou`, `à`, `entité`, `défauts` des catégories `un-ou-faux`, `atteinte-à-prouver`, `ptrs-entité`, `défauts` (`COMPOSITION_RESIDUE_GUARD`, `:1077`). Mots de contenu que le compteur ne voit pas (consigne, point 6), relus à la main : `manque` (`:878`, `:1009`, `:1021`, `:1026`), `sale` (`:1092`), `propre(s)` (`:1095`), `reference posee` (`:965`), `écart: nul`, `<sortie.json>` (`:1021`).
- *Français*, `comment` (963) : la légende (200 lignes `#`, dont les 24 lignes anglaises de 4a à 4k3, 177–200) et le corps (82 lignes `#`).
- *Anti-cart*, `comment` (4) : `pico8_cart_load` (`:274`), `pico8_data_init` (`:351`), `PICO8_FIXED_LIT` (`:71`, `:1099`). Non comptés, réécrits quand même : `pico8_fixed` (`:446`, le fichier `src/p8/fixed.c`). **Intouchés** : `PICO8_CLASS_COVERAGE` (`:232` le `-D`, `:971` le commentaire ; D-2) et `tools/pico8_reference_probe_out.txt` (`:72`, `:1100`, `:1101` ; nom de fichier de 4l3, la sonde garde son nom).
- *Citations* : `cart` 14 — « l'écart » ×3 (`:42`, `:77`, `:1005` ; la regex ne voit pas `é` comme une lettre), « booter un cart » ×3 (`:274`, `:352`, `:727`), `anti-cart` (`:136`), les noms des cibles `cart-names-count` (`:136`, `:188`) et `cart-citations[-test]` (`:184`, `:186`), « of the cart » (`:187`), `` `cart` `` et `game/cart` (`:188`) ; `lua` 2 (`` `Lua` `` `:188`, `` `lua.lua` `` `:189`) ; `short` 152, articles français et ces jetons techniques, qui resteraient comptés en anglais : `-j` (`:1`, `:732`, `:826`, `:870`), `-p` (`:246`), `.c` et `.o` (`:236`), `%.o` (`:427`), `tests/*.c` (`:1182`, ligne déjà anglaise), `<n>` et « n frames » (`:48-49`), `(re)` (`:21`).
- *Locales cryptiques*, *portée* : 0 (aucune déclaration C).
- *Lignes de commentaire* : 282 (`grep -c '^[[:space:]]*#' Makefile` ; légende 200, corps 82), 1 036 lignes hors commentaire, 1 318 en tout. Toutes les lignes `#` sont en colonne 0 ; aucune ligne de recette ne commence par une tabulation suivie de `#` ; aucun commentaire en fin de ligne de code.

### Qui lit le `Makefile` : ce qui bouge, ce qui ne bouge pas

Relevé fait sur `src tests tools .claude README.md CLAUDE.md docs` (hors historique du plan et du registre ; `replay/readers.txt`) :
- *Noms de cibles* : cités par `CLAUDE.md`, `README`, les skills (`check-e2e`, `fix-bug`, `tdd`, `prd-autopilot`), les docs et `tests/test_frames_check.c` (`:82` `"make e2e-ref"`, `:105` `"make e2e-baseline"`, lus dans la sortie de `tools/frames_check.sh`). **Intouchés**, de même que `.PHONY`, les variables (`MODULES`, `SUBSTEP`, `EVENTS`, `OUT`, `OTHERS`, `DIR`, `MIN_CHUNK`, `MECHANICAL`, `TABLE`, `BEFORE`, `SITES`) et les chemins de `tmp/`.
- *Ligne de règle `$(TMP_DIR)/replay:`* : lue par `tools/frames_ref.sh:114` (`grep -q -e '^$(TMP_DIR)/replay:'` dans le `Makefile` d'un worktree, voie (a) de 3l). **Intouchée** ; contrôle `grep -c '^$(TMP_DIR)/replay:' Makefile` = 1 à chaque état.
- *Sortie de `make e2e`* : recopiée verbatim par `check-e2e` (« Symptom, verbatim »), lue par aucun code. Seule la ligne de bilan du `Makefile` change (`e2e: 26 e2e dans tests/e2e` → `e2e: 26 e2e in tests/e2e`) ; les lignes `frames:` et `e2e:` des scripts (`frames_check.sh`, `e2e_limits.sh`, 4l4) restent françaises jusque-là.
- *Les dix-neuf messages de l'état 1* : `git grep -nF` de chaque fragment (`e2e dans`, `GIF dans`, `reference posee`, `couverture de lignes`, `apport de `, `manque`, `écart:`, `ni couvertes`, `couvre toute la référence`, `à retirer`, `rien n'est supprimé`, `ajoute `, `offsetof dans`, `catégories gardées`, ` sale`, `propre(s)`, `sortie.json`) : aucun lecteur ; les seules autres occurrences sont des commentaires ou des messages sans rapport (`tools/frames_check.sh:3`, `tests/test_line_coverage.c:1`, `tools/e2e_limits.sh:26`, `tools/line_coverage.sh:9`, `:13`, `:22`, `src/cmd/class_coverage.c:5`…). Aucun test ne lance `make` et ne lit sa sortie.
- *Lecteurs d'un autre écrivain, intouchés (propriété des chaînes)* : `grep '^garde ' $(FLOAT_GUARD)` (`:1113`) lit la ligne `garde fichiers … en-echec …` de `tools/float_guard.sh` (lue aussi par `tests/test_float_guard.c:55`, `:63`, `:74`) → **4l4** ; `DATA_RESIDUE_GUARD` (`:1054`, `par-nom`, `liste`, `valeur`…) est la liste des catégories de `tools/data_residue.sh --check` → **4l4** ; `COMPOSITION_RESIDUE_GUARD` (`:1077`) celle de `tools/composition_residue.sh --check` → **4l2** (et D-10, D-11, sans verdict, en parlent). La sortie de `data-guard` devient hybride entre l'état 1 et 4l4 (`offsetof in src/: 0` puis `résidu gardé (…) : 0 ligne(s)`), comme `rangee=` dans une ligne anglaise à 4f1.
- *Numéros de ligne du `Makefile`* : cités par D-2 (`:232`, `:971`) et D-28 (`:1178`) ; ils bougent à l'état 2 (la légende change de hauteur) : mis à jour au registre dans le même commit (ci-dessous). Les notes « Découvert » qui citent une ligne (`Makefile:871` de 4k1, « ligne 71 » de 4b) sont de l'historique : intouchées. Aucun outil ne lit un numéro de ligne du `Makefile` ; les messages de `make` en portent (`Makefile:878: *** …`), aucun lecteur.
- *`tools/e2e_long_exceptions.txt`* : vide, lu par `tools/e2e_limits.sh` (format `<events> raison`, décrit aussi par le commentaire `:898` du `Makefile`) : **aucun diff** ; le commentaire `:898` dit `` `<events> reason` ``, le commentaire du script reste à 4l4.

### État 1 — messages (`refactor:`)

`tmp/4l1_baseline/replay/stage1.sed`, 19 expressions, 19 lignes changées, autant de `+` que de `-` (`s1.patch`) :

| Ligne | Avant | Après |
|---|---|---|
| 878 | `$(error EVENTS manque : make … EVENTS=<events.json>)` | `$(error EVENTS missing: make … EVENTS=<events.json>)` |
| 952 | `e2e: N e2e dans $(E2E_DIR)` | `e2e: N e2e in $(E2E_DIR)` |
| 958 | `gif: N GIF dans $(GIF_DIR)/, <taille>` | `gif: N GIF(s) in $(GIF_DIR)/, <taille>` |
| 965 | `e2e: reference posee dans <ref>` | `e2e: reference set in <ref>` |
| 997 | `couverture de lignes: N ligne(s) dans …` | `line coverage: N line(s) in …` |
| 1003 | `apport de $(EVENTS): N ligne(s) dans …` | `contribution of $(EVENTS): N line(s) in …` |
| 1009 | `sed 's/^/manque: /'` | `sed 's/^/missing: /'` |
| 1012 | `écart: N ligne(s) de <ref> ni couvertes par make e2e (N e2e) ni dans <unreachable>, dans <gap>` | `gap: N line(s) of <ref> neither covered by make e2e (N e2e) nor in <unreachable>, in <gap>` |
| 1015 | `écart: nul -- make e2e couvre toute la référence hors <unreachable>` | `gap: none -- make e2e covers the whole reference outside <unreachable>` |
| 1021 | `$(error EVENTS ou OUT manque : make minimize … OUT=<sortie.json>)` | `$(error EVENTS or OUT missing: make minimize … OUT=<output.json>)` |
| 1026 | `$(error EVENTS manque : make e2e-judge …)` | `$(error EVENTS missing: make e2e-judge …)` |
| 1040 | `sed 's/^/à retirer: /'` | `sed 's/^/to remove: /'` |
| 1041 | `e2e-cleanup: N fichier(s) à retirer sur M, rien n'est supprimé` | `e2e-cleanup: N file(s) to remove out of M, nothing is deleted` |
| 1048 | `e2e-judge: <events> ajoute N ligne(s) à make e2e, dans …` | `e2e-judge: <events> adds N line(s) to make e2e, in …` |
| 1062 | `data-guard : offsetof dans src/` | `data-guard: offsetof in src/` |
| 1064 | `offsetof dans src/ : 0` | `offsetof in src/: 0` |
| 1068 | `catégories gardées : aucune` | `guarded categories: none` |
| 1092 | `sanitize: <events> sale` | `sanitize: <events> dirty` |
| 1095 | `sanitize: N propre(s)` | `sanitize: N clean` |

`minimize: … frame(s)` (`:1032`) et les quatre `usage: make …` (`:1230`, `:1261`, `:1270`, `:1305`) sont déjà anglais : intouchés. **Aucun rouge n'est possible** (aucun lecteur ; amendement de 4i) ; la preuve, rejouée dans le clone et écrite telle quelle dans le message :
- base de données : `make -rpnq FORCE | grep -v '^#'` avant, passée par `stage1.sed`, égale à l'octet à celle d'après (`DB-EQUAL-BY-TABLE` ; 41 lignes différentes avant la table : les 15 lignes de recette nommées et les 26 instances de `sanitize-run-%`) ;
- `make -n` de `e2e`, `gif`, `e2e-baseline`, `line-coverage`, `line-coverage-contrib`, `e2e-gap`, `minimize`, `e2e-cleanup`, `e2e-judge`, `data-guard`, `sanitize-net`, `float-guard` : mêmes codes de sortie, seules changent les lignes des messages (`replay/before/n_*.txt`, `after/n_*.txt`) ;
- sorties réelles, passées par `output.sed`, égales à l'octet à celles d'après, codes de sortie compris (`replay/before/out_*.txt`, `after/out_*.txt`) : `e2e` (0, `e2e: 26 e2e in tests/e2e`), `e2e-gap` (2 ; 2 932 lignes `missing:` et le bilan `gap: 2932 line(s) …`), `data-guard` (0), `line-coverage EVENTS=<premier e2e>` (0, 3 466 lignes), `line-coverage-contrib` (0, 55 lignes), `e2e-judge` (0, 4 lignes), `e2e-cleanup` (0, `0 file(s) to remove out of 26`), `minimize … MECHANICAL=1` (0, inchangé), `gif` (0), `sanitize-net` (0, `sanitize: 26 clean`) ; les quatre refus (`make replay`, `screencast`, `minimize`, `e2e-judge` sans variable : code 2, message traduit, même ligne) ;
- non jouées, prouvées par la base seule : `e2e-baseline` (**jamais lancée** : elle réécrit les références), la branche `sanitize: <events> dirty`, la branche `data-guard: offsetof in src/`, la branche `guarded categories: none` (`DATA_RESIDUE_GUARD` n'est pas vide) ;
- `make test-all` vert dans le clone (26 e2e), `git apply --check` de `s1.patch` sur `HEAD` ;
- compteurs après l'état 1 (rejoués) : `MODULES=<4l1>` français `string` 39 → 7 (`du`, `par`, et les cinq de `COMPOSITION_RESIDUE_GUARD`), `comment` 963 ; sans `SUBSTEP` français 3 463 → 3 431 (`string` 543) ; citations inchangées (les recettes ne sont pas lues par le relevé) ; anti-cart inchangé.

### État 2 — commentaires (`docs:`), note, consigne, registre

Un agent, sur le `Makefile` seul, sous la consigne de l'étape 4 et ces points propres au morceau (`tools/e2e_long_exceptions.txt` n'a pas d'agent : il est vide) :
- *Règle du `CLAUDE.md` pour un `Makefile`* : la légende reste (une entrée par cible, en tête) ; dans le corps, seule l'aide à la lecture d'une commande reste (pourquoi cette forme de règle, ce piège). Le récit (« la phase D », « la phase 2 », « l'ancien filet », « §6.x ») part au message de commit ; un piège reste, dit en anglais (`Do not merge the six`, `.SECONDARY`, `.DELETE_ON_ERROR`, l'ordre des deux TU qui relisent `tmp/replay_dump.txt`, le code de sortie de `sha256` qu'un pipe perdrait, la référence de couverture comparée et jamais recalculée).
- *Langue* : anglais, les noms des cibles, variables, fichiers et commandes tels quels. Les lignes anglaises de la légende (177–200) restent, sauf la ligne 187 (ci-dessous).
- *Jetons comptés `short`*, même en anglais (consigne, amendement de 4a) : une option ou un motif nu d'une ou deux lettres qui est un ancien nom (`-j`, `-p`, `.c`, `.o`, `%.o`, `*.c`, `*.d`, `<n>`, `(re)`, `(s)`, et les mots `cp`, `rm`, `ls`, `id`, `vs`, `OS`, `os`, `ms`, `px`) compte. On l'écrit entre backticks (`` `-j` ``, `` `mkdir -p` ``, `` `$(call objs,<sources>)` ``, `` `tests/*.c` ``, `` `MIN_CHUNK=<n>` ``) ou en mots (« sets or resets », « the C sources », « object files », « OS-specific », jamais « (s) »). Jamais `` `e2e` `` seul entre backticks (ancien nom ambigu depuis 4f1).
- *Anti-cart* : `pico8_cart_load()` → `cart_load()`, `pico8_data_init()` → `data_init()`, `pico8_fixed` → `p8/fixed.c`, `PICO8_FIXED_LIT` → `P8_FIXED_LIT` (légende de `constants-check` et commentaire de sa règle). « Boots a cart » se dit « calls `cart_load()` » ou « boots the game ». `PICO8_CLASS_COVERAGE` (D-2) et `tools/pico8_reference_probe_out.txt` restent tels quels.
- *Citations* : « l'écart » disparaît avec le français ; `anti-cart` (`:136`) → « old-name counter » ; la ligne 187 (« what comments and assert messages cite of the cart beyond ») se réécrit sans « the cart » (« what comments and assert messages still cite beyond »). Restent, irréductibles tant que les cibles gardent leur nom : `cart-names-count` (entrée de la cible et renvoi de l'entrée `cart-citations`), `cart-citations-test`, `cart-citations`, et la description de ce que le relevé cherche (`` `cart` ``, `game/cart`, `` `Lua` ``, `` `lua.lua` ``) : **6/2/0** attendus, proposés au registre (D-56).
- *Affirmations à corriger en traduisant* (consigne, rubrique 7), relues sur le `Makefile` :
  - légende de `make test` : elle nomme sept prérequis sur dix-neuf ; elle dit les gardes et renvoie aux tests d'outils marqués « in make test » (les douze `*-test`/`*-check` qui le disent déjà) ;
  - légende de `class-coverage-net` : « idem » suit `e2e-judge` ; elle dit « like class-coverage, over the e2e of tests/e2e/ » ;
  - légendes de `float-guard` (« §6.3 »), `float-grep` (« grep du §6.1 »), `float-link` (« §6.2 ») : aucun document ne porte ces sections ; elles disent ce qui est vérifié (un fichier du cœur qui a besoin du flottant ; le `grep` des types et appels flottants sur le cœur ; le lien sans `-lm`) ;
  - légendes de `data-residue` (« la phase D sort sur une sortie vide ») et `composition-residue` (« la phase 2 ») : elles disent ce qu'elles comptent et que les catégories gardées sont dans `DATA_RESIDUE_GUARD` et `COMPOSITION_RESIDUE_GUARD` ;
  - légende de `minimize` : l'étage mécanique est la commande `mechanical` de `tmp/minimize`, que `tools/minimize.sh` lance avant la passe guidée (`tools/minimize.sh:36`, `:64`) ;
  - commentaire de `ENGINE_SRCS` (`:272-274`) : faux ; la liste exclut les sept points d'entrée de `src/cmd/`, `p8/palette.c`, `platform/raylib_backend*.c` et les modules de `devtools/` que seuls les outils et le jeu interactif lient (`fixed_test_double`, `gif`, `minimize`, `replay_control`, `replay_source`, `screencast`, `screencast_player`, `session_recorder`) — dix-huit fichiers, relevés par `comm` sur la base de `make` ;
  - commentaire de `CORE_SRCS` (`:346-348`) : « inventaire, couverture et garde flottantes » ; ses lecteurs sont `constants-check`, `float-guard`, `alloc-guard` et `float-grep` ;
  - commentaire de `DATA_ENGINE_SRCS` (`:351-352`) : vrai (`ENGINE_SRCS` moins `cart.c`, `cheat.c`, `galaxy.c`, `help.c`, `menu.c`, `score_panel*.c`, `structure_icon.c`, `planet_family_trade.c`, `devtools/class_coverage.c`) ; `pico8_data_init()` → `data_init()`.
- *Hauteur* : aucune contrainte (`e2e-gap` ne lit pas le `Makefile`) ; les numéros de ligne cités par D-2 et D-28 se relèvent après l'état 2 et se reportent au registre.
- *Contrôles de l'agent* : la commande de la consigne (`diff <(grep -v '^[[:space:]]*#' AVANT) <(grep -v '^[[:space:]]*#' Makefile)`, vide) **et** la base de `make` (`sh tmp/4l1_baseline/replay/db_proof.sh <sortie>`, avant et après, identiques à l'octet). `make -rpnq FORCE` ne compile ni n'écrit rien : l'agent peut la lancer malgré le point 8 (« sans `make` »), mais aucune autre cible.

**Preuve de l'état 2** (rejouée sur une édition d'essai dans le clone, `replay/traps.txt`) :
1. `sh tmp/4l1_baseline/replay/db_proof.sh` avant (sur l'arbre de l'état 1) et après, **sans aucun build entre les deux** (les fragments `.d` inclus par `-include` font partie de la base) : identiques à l'octet. Elle couvre chaque cible, variable, prérequis et recette. Rejoué : un `\` en fin de la ligne de commentaire qui précède `$(OBJ_DIR)/%.o: %.c` avale la règle — la base change, le `diff` de la consigne ne voit rien ; une ligne `<TAB># …` insérée entre deux règles devient une ligne de recette — même chose.
2. `diff <(grep -v '^[[:space:]]*#' AVANT) <(grep -v '^[[:space:]]*#' Makefile)` vide, lignes vides comprises : seul à voir les trois `$(error)` des conditionnelles non prises, que la base ne contient pas.
3. Nombre de lignes hors commentaire : 1 036 avant et après ; lignes de commentaire mesurées avant (282) et après.
4. `grep -c '^$(TMP_DIR)/replay:' Makefile` = 1.

### Ce qui reste hors de 4l1

- `grep '^garde '` de `float-guard` et `DATA_RESIDUE_GUARD` (4l4, écrivains `tools/float_guard.sh` et `tools/data_residue.sh`) ; `COMPOSITION_RESIDUE_GUARD` (4l2, `tools/composition_residue.sh`) : 6 mots `string` restent comptés sur `Makefile` jusque-là.
- Les lignes imprimées par les scripts de `tools/` (`frames:`, `e2e: … manque`, `résidu gardé`, `garde fichiers`, `couverture:`…) : 4l2, 4l4 ; `src/cmd/`, `src/platform/` (4l3).
- `PICO8_CLASS_COVERAGE` (D-2) ; la version de libclang (D-28) ; les catégories mortes (D-10, D-11) ; les entrées de légende absentes (`psx-hello-clean`, `psn00bsdk-toolchain-setup` n'ont pas la leur) : signalées dans la note, pas ajoutées (traduire n'ajoute pas d'entrée).

### Registre (`docs/decisions-plan-5.md`)

Entrées neuves, au format des conventions, ligne d'index comprise :
- **D-55 — Exception `du` du compteur de français dans le `Makefile`**. Origine : 4l1 (relevé de fin). Question : `du` (la commande `du -ch` de la recette de `gif`, qui imprime la taille de `gif/`) est compté `string` comme l'article français « du » ; c'est du code, intouchable. L'excepter ? Options : (a) excepter ; (b) garder compté. Sites : 1 (`Makefile`, recette de `gif`). Proposition prête à appliquer : (a), la ligne `string<TAB>Makefile<TAB>du` ajoutée à `tools/french_exceptions.txt` ; contrôle (fait par la PRD sur une copie du fichier) : `french-count MODULES=Makefile` `string` 7 → 6, et 0 après 4l2 et 4l4.
- **D-56 — Citations `cart`/`lua` irréductibles de la légende du `Makefile`**. Origine : 4l1. Question : la légende nomme les cibles `cart-names-count`, `cart-citations-test`, `cart-citations` et décrit ce que le relevé cherche (`` `cart` `` hors de `game/cart`, `` `Lua` ``, `` `lua.lua` ``) ; `tools/cart_citations.py` les compte (sa regex prend `-` pour une frontière ; il n'exclut que ses propres fichiers). Options : (a) accepter ce reste (le relevé n'est pas une garde du §5) ; (b) que `tools/cart_citations.py` ne compte ni `cart-names-count`/`cart-citations` (noms de cibles) ni la ligne de légende qui le décrit, test d'abord ; (c) renommer les deux cibles (`old-names-count`, …), avec `CLAUDE.md`, le plan, le registre, la consigne et les skills qui les citent. Sites : 6 `cart`, 2 `lua` (attendus après l'état 2). Proposition : (a), rien à appliquer ; contrôle : `cart-citations MODULES=Makefile` rend exactement ces 6/2/0. Portée : aucune mention ailleurs dans le code (les outils du dictionnaire et `tools/cart_citations.py` sont exclus du relevé).

Compléments d'entrées existantes, sans verdict :
- **D-2** : les deux sites du `Makefile` (le `-D` de `COVERAGE_CFLAGS` et le commentaire de la règle de `tmp/class_coverage`) passent aux numéros de ligne d'après l'état 2 ; le commentaire, traduit, garde le nom `PICO8_CLASS_COVERAGE` ; 35 sites inchangés.
- **D-28** : la ligne de `LIBCLANG ?=` passe à son numéro d'après l'état 2.
- **D-36** : les 19 « un » du `Makefile` rendus comptés par 4f1 partent avec le français à 4l1.

Si l'agent soulève une autre question (une cible au nom trompeur, une entrée de légende manquante qu'il faudrait ajouter), une entrée s'ajoute (D-57…) ; la note le dit dans tous les cas.

### Fin de PRD

Ordre : `make rename-extract` frais sans `warning:` (4 `partial`) ; `rename-rekey-check SUBSTEP=3l` vert ; compteurs sur `MODULES=<4l1>`, puis `SUBSTEP=3l`, puis `SUBSTEP=3a` à `3j`, puis sans `SUBSTEP` (`sh tmp/4l1_baseline/counters.sh . <sortie>`, `sh tmp/4k3_baseline/others.sh .`).

**Critère de fin** :
- `MODULES=<4l1>` : `cart-names-count` 0, quatre colonnes ; `french-count` 0 en `identifier`, `backtick` et `comment`, **7** en `string` (`du`, D-55 ; `par`, 4l4 ; `un`, `ou`, `à`, `entité`, `défauts`, 4l2), 6 avec la ligne de D-55 sur une copie de `tools/french_exceptions.txt` ; `cart-citations` **6/2/0** (D-56) ; `cryptic-locals` 0 ; `scope-count` 0 ;
- `SUBSTEP=3l`, prévision : français `done` 3 437 → 2 442 (`string` 549 → 517, `comment` 2 888 → 1 925), `later` 0 ; anti-cart `done` 40 → 36 (`comment` 38 → 34, `string` 2) ; citations 21/4/665 → 13/4/513 ; `cryptic-locals` 17 ; `scope-count` 0 ;
- `SUBSTEP=3a` à `3j` : égaux à l'octet à `tmp/4l1_baseline/others.txt` ;
- sans `SUBSTEP`, prévision : anti-cart `TOTAL` 40 → 36, citations 21/4/665 → 13/4/513, français 3 463 → 2 468 (`string` 543, `comment` 1 925), `cryptic-locals` 19, `scope-count` 12 (D-13) ;
- relecture : `grep -nP '[^\x00-\x7f]' Makefile` : 0 ; `grep -nwiE 'dans|manque|sale|propre|cible|binaire|lignes?|fichiers?|rejeu|cœur|écart|recette|sortie' Makefile` : 0 ; `grep -n 'pico8\|PICO8' Makefile` : `PICO8_CLASS_COVERAGE` ×2 (D-2) et `tools/pico8_reference_probe_out.txt` ×3 seulement ; `git diff --stat -- tools/e2e_long_exceptions.txt` vide ;
- noms : la base de `make` (`db_proof.sh`) de l'état final égale à celle de `HEAD` passée par `stage1.sed`.

**Invariant (§5)** : `make test-all` vert à chacun des deux états, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ; `make gif` et `make float-link` verts ; `rename-tool-test` et les huit `rename-*-test` verts ; `class-coverage-net` et `e2e-gap` rouges, sorties égales à celles de `tmp/4l1_baseline/` (`class-coverage-net` hors lignes `cc` ; `e2e-gap` par `tmp/line_coverage_gap.txt`, 2 932 lignes, et sa sortie brute égale par `replay/output.sed`) ; `data-residue` et `composition-residue` à 0, `--detail` et `--champs` identiques ; `make sin-table` reproduit `quarter_wave.h` à l'octet ; `rename-strings-verify SUBSTEP=3l` : 1 en défaut sur 44, 9 `unseen`, comme au départ.

### Note « Découvert » de 4l1 (sous l'entrée 4l de `docs/plan-5.md`, après la liste des morceaux, au format des notes de 4k1 à 4k3 : « **Découvert** (morceau 4l1, PRD `docs/prd/triage/phase3-etape4l1-makefile-commentaires.md`) »)

Elle porte : mesure de départ et de fin (charge 1 174, et l'écart à la prévision de 4a, +19 « un » de 4f1) ; les dix-neuf messages, l'absence de lecteur et la preuve par les deux tables ; les lecteurs laissés à 4l2 et 4l4 ; les lignes de commentaire avant et après, la preuve par la base de `make` et les deux pièges rejoués ; les affirmations corrigées ; D-55, D-56 et les compléments ; ce qui reste à 4l2, 4l3, 4l4 ; les compteurs `MODULES=<4l1>`, `SUBSTEP=3l` et sans `SUBSTEP`. Et les amendements de `docs/consigne-etape-4.md` tirés de l'exécution — au moins ceux-ci, si elle les confirme :
- *Point 8* (*Preuve*, complément de 4a) : pour le `Makefile`, la preuve d'un commit de commentaires est la base de données de `make` sans ses lignes `#` (`make -rpnq FORCE | grep -v '^#'`), identique à l'octet avant et après, sans build entre les deux ; la commande des scripts ne voit ni un `\` en fin de commentaire qui avale la ligne suivante, ni un `#` après une tabulation qui devient une ligne de recette, et la base ne voit pas les `$(error)` des conditionnelles non prises : les deux ensemble. `make -rpnq` ne compile rien : l'agent peut la lancer.
- *Point 6* (complément de 4i) : un message de recette du `Makefile` sans lecteur se prouve par la base de `make` égale par la table de traduction (appliquée aux lignes sources) et par la sortie réelle de chaque cible touchée égale par la même table appliquée aux sorties, codes de sortie compris ; une cible qui réécrit l'état du dépôt (`e2e-baseline`) n'est jamais lancée, la base suffit. Une recette qui lit la sortie d'un script (`grep '^garde '`) ou lui passe ses catégories (`*_RESIDUE_GUARD`) est un lecteur : laissée à la sous-étape de l'écrivain.
- *Point 2* (complément de 4a et 4i) : dans un commentaire de `Makefile` ou de script, une option ou un motif d'une ou deux lettres (`-j`, `-p`, `.c`, `%.o`, `*.d`, `<n>`, `(re)`, `(s)`) et les mots `cp`, `rm`, `OS`, `id`, `vs` comptent `short` : entre backticks ou en mots ; « l'écart » compte `cart` (la regex ne prend pas `é` pour une lettre).

## Implementation Chunks

Un seul morceau (charge 1 174 ≤ 1 382). L'exécutant produit, dans cet ordre, `make test-all` vert à chaque état :
1. les dix-neuf messages (`stage1.sed`, relu) ; preuve par la base et par les sorties (`capture.sh`, `run_targets.sh`, `output.sed`, avant et après, dans le dépôt : sorties sous `tmp/4l1_s1/`) ; `git diff HEAD > tmp/4l1_stage1.patch` (égal à `tmp/4l1_baseline/replay/s1.patch` si rien n'a bougé), message `tmp/4l1_1_msg.txt` (`refactor: plan 5 morceau 4l1 — messages des recettes du Makefile en anglais`, table de traduction, absence de lecteur et liste `git grep -nF`, preuves des deux tables, lecteurs laissés à 4l2 et 4l4) ;
2. copie de l'état 1 et de sa base (`db_proof.sh tmp/4l1_before_comments/db.txt`, `cp Makefile tmp/4l1_before_comments/`) ; état 2 (agent), preuve de la base, extraction fraîche, compteurs ; note « Découvert », amendements de la consigne, D-55, D-56 et les compléments, laissés dans l'arbre de travail ; message `tmp/4l1_2_msg.txt` (`docs: plan 5 morceau 4l1 — légende et commentaires du Makefile en anglais, note Découvert`, récit parti des commentaires : phases, « §6.x », « ancien filet »).

## Remise

L'exécutant ne commite pas. Il laisse :
- `tmp/4l1_stage1.patch` : diff complet de `HEAD` à l'état 1 (s'applique par `git apply` sur `HEAD` propre) ;
- `tmp/4l1_1_msg.txt`, `tmp/4l1_2_msg.txt` : messages de commit en français, dans l'ordre des commits, récit de chantier et preuves compris, terminés par les lignes d'attribution de la session ; le second est celui de l'arbre final ;
- l'arbre de travail à l'état 2 : commentaires du `Makefile`, note, consigne, registre.

## Testing Decisions

- Aucun test neuf, aucune assertion touchée : seuls dix-neuf littéraux de recette et des commentaires changent. Le filet : `make test-all` (qui passe par `unit`, les gardes et `e2e`, donc par chaque recette de la suite), `tests/test_frames_check.c` (les noms `make e2e-ref` et `make e2e-baseline` dans la sortie de `frames_check.sh`), les cibles hors suite rejouées par `run_targets.sh`, et la base de `make` pour tout le reste.
- Un bon contrôle juge une sortie observable : la base de données de `make`, la sortie réelle et le code de sortie de chaque cible touchée, `*.sha256`, le fichier de gap, les sorties à l'octet des gardes hors suite, les compteurs sur extraction fraîche.
- Prior art : 4a et 4b (tables de traduction qui rendent les lignes d'après depuis celles d'avant, `class-coverage-net` « égal par la table »), 4i et 4k3 (chaîne sans rouge possible, prouvée par `git grep -nF` et le vert), 4a (commande de preuve d'un script), 4k3 (forme de la PRD et de la note).

## Out of Scope

- Toute ligne du `Makefile` hors des dix-neuf littéraux de l'état 1 et des lignes `#` de l'état 2 : cibles, variables, prérequis, recettes, `.PHONY`, ordre des règles.
- Les écrivains des sorties lues par le `Makefile` et leurs catégories (`tools/float_guard.sh`, `tools/data_residue.sh`, 4l4 ; `tools/composition_residue.sh`, 4l2) ; tout le reste de `tools/`, `src/cmd/`, `src/platform/` (4l2–4l4) ; les docs, `CLAUDE.md`, les skills (en français, §2).
- Les entrées en attente de verdict (D-1, D-2, D-10, D-11, D-28, D-36, D-55, D-56) ; `TROU-N` → `GAP-N` (étape 5) ; la suppression de `docs/consigne-etape-4.md` et la clôture du registre (4l4).
- Ajouter, retirer ou renommer une entrée de la légende, une cible ou un message.

## Further Notes

- La charge est presque toute en commentaire (963 mots sur 1 174) ; la rigueur porte sur la preuve, puisque `gcc -fpreprocessed` ne s'applique pas : la base de données de `make` est la seule qui voie un commentaire qui avale une règle.
- `make e2e-baseline` n'est jamais lancée, même pour prouver son message : `check-e2e` l'interdit et elle réécrit les 26 références.
- 4l1 ne clôt pas 4l : la note donne les totaux de départ de 4l2.
