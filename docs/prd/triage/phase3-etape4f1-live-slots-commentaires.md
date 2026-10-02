---
title: "Plan 5, morceau 4f1 — commentaires, chaînes et locales de test_live_slots (places vivantes, zone de jeu, prises, slipways, trous de ver et sondes refusés, remplissage du rejeu)"
description: "Ramener à zéro, sur tests/test_live_slots seul, le français résiduel, les noms du cart, les citations et la locale cryptique, en laissant à 4l3 les seules chaînes lues de la sortie REPLAY_FILL, sans changer un pixel."
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
> 3f. **Sonde, trou de ver, particules.** `game/` : `probe`, `targeting`, `wormhole`, `wormhole_display`, `particle`, `particle_zone`. Tests : `test_probe`, `test_probe_spawn_particle_order`, `test_wormhole`, `test_particle_zone`, `test_live_slots`. Résidu ≤ 211.
>
> 4f. **Commentaires — sonde, trou de ver, particules.** Modules et tests de 3f. Coupée par 4a (charge 1 927) :
>    - 4f1 (1 242) : `tests/test_live_slots`, seul (un fichier ne se coupe pas).
>
>    (…, après la définition des morceaux de 4k et 4l :)
>
>    Un morceau (4c1…) suit la PRD de sa sous-étape, avec ses commits et `make test-all` vert ; ses compteurs se lisent par `MODULES="<sa liste>"`, et `SUBSTEP=3x` à zéro clôt la sous-étape après son dernier morceau.

4f1 est le premier morceau de 4f (charge 1 242 ≤ 1 382, remesurée ci-dessous à l'identique) : un seul fichier, `tests/test_live_slots.c` (1 488 lignes), qui ne se coupe pas. Ses compteurs se lisent par `MODULES=tests/test_live_slots` ; `SUBSTEP=3f` ne revient à zéro qu'après 4f2. Aucune donnée de `tools/` ni fixture n'est rangée dans le morceau (les cinq fixtures de 3f sont à 4f2).

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4). Morceau 4f1 = l'entrée 4 appliquée au morceau 4f1 défini par la note de 4a.

Entrées lues : `docs/plan-5.md` §1, §2, §4, §5, l'entrée 4 et sa méthode, l'entrée 3f et sa note « Découvert », les notes de 4a, 4b, 4c1, 4c2, 4d et 4e ; `docs/consigne-etape-4.md` (tous ses amendements, de 4a à 4e) ; `docs/decisions-plan-5.md` (D-1 à D-44 ; aucune entrée sans verdict n'est appliquée ici) ; le skill `reprendre-les-commentaires` (`SKILL.md`, `consigne-agent.md`, `modele-rapport.md`) ; la PRD de 4e (`c4f6777^`, modèle de forme) et `git log -5 --stat` : 4e s'est commitée en trois commits de code, chaînes (`refactor:`, `a74ae48`), locales avec TSV et surcharges (`refactor:`, `35e704a`), commentaires avec note, consigne et registre (`docs:`, `c4f6777`), puis la PRD graduée (`b6f4681`).

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions reposent sur le plan et sur des mesures à `b6f4681`, écritures dans `tmp/` seulement (`make rename-extract` frais, `rename-rekey-check SUBSTEP=3l`, compteurs ; sorties dans `tmp/4f1_baseline/`, gardes hors suite dans `tmp/4f1_base/`). Les états 1 et 2 et le rouge prévu ont été **joués** dans un clone local jetable hors du dépôt (répertoire de travail de l'agent, jamais poussé) : chaque chiffre ci-dessous est une observation. Le rejeu est gardé sous `tmp/4f1_baseline/replay/` (`stage1.py` : les traductions de l'état 1 ; `decision.tsv` et `tsv_edit.py` : l'état 2 ; `s1.patch`, `s2.patch` cumulatif ; sorties de `rename-table-check`, `rename_derive.py`, `cart-names-count` et `cart-citations` avant/après, journaux de `make test-all`) : un prototype à relire, pas à appliquer les yeux fermés.

**Condition d'entrée** : `origin/plan-5-noms-anglais` à `b6f4681` ou un descendant, arbre propre, `make test-all` vert ; `make rename-extract` frais sans `warning:` (4 lignes `partial` de `class_coverage` attendues) ; `rename-rekey-check SUBSTEP=3l` vert (0 introuvable) ; la mesure de départ ci-dessous retrouvée à l'identique. Sorties de `class-coverage-net`, `e2e-gap` et `tmp/line_coverage_gap.txt` relevées au départ (à `b6f4681` : `tmp/4f1_base/`, 41 et 2 934 lignes, fichier de gap 2 932 lignes, les deux cibles rouges).

## Problem Statement

`tests/test_live_slots.c` est le test le plus lourd de l'étape 4 : il rejoue les 26 e2e sous un invariant (les particules, listes, éclats et pointillés de transit comptés par `particle_zone` sont exactement ceux des émetteurs inscrits et des slipways), vérifie qu'une session revient à son niveau de départ, que la zone de jeu ne grandit pas pendant une fenêtre sans action, que les prises, slipways, trous de ver et sondes refusés recyclent leur arène, que la génération quittée meurt au flip (lecture d'une sentinelle sous ASan), et que le rejeu écrit son fichier de remplissage `REPLAY_FILL`. Ses identifiants ont été renommés à l'étape 3, mais **tout le reste est encore en français et parle le cart** : 159 lignes de commentaire en français (« Places vivantes des zones interface et particules… »), qui citent les digrammes entre backticks (`` `eu` ``, `` `mz` ``, `` `fo` ``, `` `hv` ``, `` `bd` ``, `` `kj` ``, `` `jo:ib` ``, `` `cg.jc` ``…), des fonctions et paramètres disparus (`` `relever` ``, `` `dessiner` ``, `` `rendre` ``, `AVANT_F0`, `PICO8_REMPLISSAGE`, « comme main.c ») et des fixtures sous leur ancien nom (« Fixture PRISES », « PLIAGES ») ; environ 110 messages d'`assert` et 40 libellés imprimés français, dont beaucoup citent le cart (`"jl() doit rendre…"`, `"mo.p, mo.v, mo.ra, je.ra et ok.qa doivent etre reecrits en place"`, `"planete introuvable dans cg.np"`) ; des chemins `tmp/test_places_vivantes_*` ; le nom d'arène `"mesure des bulles"`. Charge 1 242 : 942 mots français, 73 noms du cart, 226 citations, une locale cryptique.

Et une part de ses chaînes **n'est pas à lui** : les étiquettes du fichier de remplissage (`rangee`, `particule`, `max_vivantes`, `max_octets`, `partie`, `chargement`) sont écrites par `src/cmd/replay.c` (4l3) et seulement lues ici. Les traduire du seul côté du test casserait le test (rouge joué) ; la règle de propriété les laisse à 4l3, qui les traduira des deux côtés.

Enfin, des locales mentent ou parlent français : `grab_arena` tient trois fois un nombre d'octets (dérivé de `prise`), `mesure`, `neuf`, `motif`, `un` sont français, et `e2e` (le compte des e2e de `main`, portée 60) est la seule candidate de `cryptic-locals`.

## Solution

Appliquer à 4f1 la méthode prouvée de 4a à 4e : un seul morceau, trois états de code — chaînes (les seules écrites **et** lues dans le fichier, 142 lignes, aucun lecteur ailleurs, aucun rouge possible ; un rouge de contrôle joué montre pourquoi les étiquettes de `REPLAY_FILL` attendent 4l3), locales (9 identités par `rename-apply`, TSV et surcharges dans le même état, effets de bord des anciens noms mesurés), commentaires (préprocesseur identique) — puis la note « Découvert » de 4f1, les amendements de la consigne, une entrée neuve et un complément au registre. L'exécutant ne commite pas : il laisse deux patchs et trois messages dans `tmp/`, et l'arbre de travail à l'état final (§ « Remise »). À la fin, sur `MODULES=tests/test_live_slots` : français, anti-cart, citations et locales cryptiques à zéro, portée 2 (D-13), seuls restes français les 12 littéraux lecteurs de `REPLAY_FILL` (4l3), `make test-all` vert et `tests/e2e/*.sha256` inchangés.

## User Stories

1. En tant que lecteur de `test_live_slots.c`, je veux un en-tête et des commentaires en anglais qui disent ce que chaque cas vérifie et pourquoi ces frames-là, afin de comprendre le test sans le cart ni le français.
2. En tant que joueur, je veux que rien de ce que je vois ne change, afin que la traduction n'ait aucun effet sur le jeu (26 e2e conformes, `*.sha256` inchangés).
3. En tant que lecteur d'un échec, je veux des messages d'`assert` en anglais qui nomment les fonctions et les champs C (`reset_world`, `cart_start_game`, `targeted_by`, `main_ui`'s `mouse_world`) au lieu de `jl()`, `eu`, `fo`, `ok.qa`, afin de comprendre un arrêt sans glossaire.
4. En tant que lecteur de la sortie du test, je veux des libellés imprimés en anglais (`game=`, `particle=`, `refused probes 1/10`, `held slipway f36/f45`), afin de lire un relevé sans traduire.
5. En tant que mainteneur de `cmd/replay.c` (4l3), je veux que les étiquettes de `REPLAY_FILL` restent identiques des deux côtés jusqu'à ma sous-étape, et que ce PRD liste exactement leurs sites lecteurs dans le test, afin de les traduire d'un coup, écrivain et lecteurs, sans en oublier un.
6. En tant que relecteur, je veux que le commit de chaînes prouve par `grep` qu'aucune chaîne traduite n'a de lecteur hors du fichier (pas de constante de hachage, pas de `CASES[]`, aucune cible ni script qui lise la sortie du test), afin de savoir pourquoi aucun test ne rougit.
7. En tant que relecteur, je veux voir le rouge de contrôle (une étiquette de remplissage traduite du seul côté lecteur fait échouer `replay_writes_fill_at_session_end`), afin de croire la frontière tracée entre 4f1 et 4l3.
8. En tant que lecteur de `tmp/`, je veux des fichiers temporaires `tmp/test_live_slots_*` au lieu de `tmp/test_places_vivantes_*`, afin que le nom du fichier suive celui du test.
9. En tant que lecteur de `icons_bytes`, je veux `measure_arena` et `"tooltip-measure"`, comme dans `test_game_zone.c` (4c2), afin qu'un même geste porte un même nom.
10. En tant que lecteur des cas de zone, je veux `used_before`/`used` au lieu de `grab_arena` pour un nombre d'octets, afin qu'un nom ne mente plus.
11. En tant que lecteur, je veux `fresh`, `pattern`, `first_refusal` au lieu de `neuf`, `motif`, `un`, afin qu'aucun identifiant ne soit français.
12. En tant que lecteur de `main`, je veux `e2e_count` au lieu de `e2e` pour le nombre de rejeux, afin que `cryptic-locals` tombe à zéro et que le nom dise ce qu'il compte.
13. En tant que mainteneur du TSV, je veux que chaque locale renommée ait sa ligne `renommé` avec son identité courante et une ligne `identity` dans `tools/rename_overrides.tsv`, et que les trois `grab_arena` gardent leur ancien `prise` du cart (D-42), afin que le TSV reste le pont inverse.
14. En tant que mainteneur des sous-étapes suivantes, je veux que l'effet de bord du renommage de `un` (un mot français de deux lettres qui devient un ancien nom, compté `short` par `cart-citations` dans leurs commentaires français) soit mesuré et noté, et qu'aucune sous-étape close ne bouge, afin de ne pas découvrir ces sites en route.
15. En tant que relecteur, je veux un état de commentaires dont la sortie de `gcc -fpreprocessed -dD -E -P` est identique à l'octet, afin de le relire sans relire le code.
16. En tant que lecteur, je veux que chaque renvoi nomme un symbole qui existe (`take_reading`, `BEFORE_F0`, `REPLAY_FILL`, `src/cmd/game.c`, `GRAB_ARENAS`…), vérifié par `grep`, afin de le suivre.
17. En tant que lecteur, je veux que les affirmations des commentaires soient revérifiées en traduisant (2C − 4 pointeurs, sept icônes, 115 frames et 5 particules par draw, les frames des fixtures), afin qu'une traduction ne recopie pas une erreur.
18. En tant que lecteur, je veux que le commentaire du format de remplissage décrive les lignes par leur rôle sans citer les étiquettes françaises, afin qu'il reste vrai quand 4l3 les traduira.
19. En tant qu'humain du point d'arrêt, je veux une entrée neuve au registre sur les noms de test qui disent « grab arena » pour une prise, et un complément sur l'effet de `un`, afin de trancher ce que 4f1 ne peut pas trancher.
20. En tant qu'orchestrateur, je veux deux patchs cumulatifs et trois messages dans `tmp/`, afin de commiter les trois sortes de commit séparément avec `make test-all` vert à chacun.

## Implementation Decisions

### Mesure de départ (`b6f4681`, extraction fraîche, `MODULES=tests/test_live_slots`, écrits dans `tmp/4f1_baseline/` seulement)

`make rename-extract` : aucun `warning:`, 4 lignes `partial` (`cmd/class_coverage.c`, `tests/test_class_coverage.c`, × 2 configurations) ; `rename-rekey-check SUBSTEP=3l` : 0 identité introuvable.

| Compteur | id | backtick | string | comment | total |
|---|---|---|---|---|---|
| `french-count` | 0 | 0 | 30 | 912 | 942 |
| `cart-names-count` | 0 | 68 | 0 | 5 | 73 |
| `cart-citations` (cart/lua/short) | — | — | — | 2/0/224 | 226 |
| `cryptic-locals` | — | — | — | `e2e` de `main`, portée 60 | 1 |
| `scope-count` | — | — | — | `CV`, `CG` de `registries_bytes` (D-13) | 2 |

Charge 942 + 73 + 226 + 1 = 1 242, celle de 4a. Sous `SUBSTEP=3f`, le fichier pèse `done` 31 `backtick` + 5 `comment`, `later` 37 `backtick`. Hors morceau : `cryptic-locals` sans `SUBSTEP` 92 ; `cart-names-count` sans `SUBSTEP` `TOTAL` 2 550 (backtick 2 250) ; `cart-citations` sans `SUBSTEP` 82/41/1 552 ; `SUBSTEP=3a` à `3e` à 0 partout.

Détail :
- *Français* (`SITES=1`, `tmp/4f1_baseline/french_sites.txt`) : `string` 30 (libellés de `printf`/`fprintf`, lignes 257, 566, 620, 634, 721, 732, 916, 1021, 1137, 1165, 1292, 1386, 1418, 1430, 1478) ; `comment` 912, messages d'`assert` compris (le compteur les range en `comment`). Assert et libellés partent à l'état 1, commentaires à l'état 3. **Angle mort** (consigne, point 6) : les mots de contenu (`rangee`, `particule`, `partie`, `chargement`, `vivantes`, `octets`, `remplissage`, `releve`, `rejeu`, `eclat`, `prise`, `sonde`…) — relevé complémentaire par `grep -noE` des littéraux (`tmp/4f1_baseline/literals.txt`, 305 littéraux).
- *Anti-cart* : les 68 `backtick` sont les digrammes et anciens noms cités en commentaire (`` `eu` `` ×8, `` `mz` `` ×7, `` `fo` `` ×7, `` `hv` `` ×6, `` `bd` `` ×4, `` `ok` ``, `` `kj` ``, `` `cc` ``, `` `by` ``, `` `bg` `` ×3, `` `relever` ``, `` `oe` ``, `` `mo` ``, `` `ju` ``, `` `je` ``, `` `fv` ``, `` `fm` ``, `` `cg.jo` `` ×2, `` `rendre` ``, `` `rangee` ``, `` `particule` ``, `` `dessiner` ``, `` `jo:ib` ``, `` `jo:cn` ``, `` `nr:cn` ``, `` `jl()` ``, `` `ie.gy` ``, `` `cg.jc` ``, `` `bw` ``, `` `dr` ``, `` `hw` ``, `` `ox` ``, `` `ra` ``, `` `p` ``, `` `P` ``…) ; les 5 `comment`, des digrammes nus. Le critère est zéro.
- *Citations* : `cart` 2 (lignes 5 et 133, « le cart n'est chargé qu'une fois », « Charge le cart ») ; `short` 224, dont 94 dans des messages d'`assert` et libellés (observé : 224 → 130 à l'état 1), le reste des mots français de deux lettres qui sont d'anciens noms (`le`, `en`, `et`, `du`, `lu`…) et des digrammes nus.
- *Lignes de commentaire* (non vides moins préprocesseur) : 159 (1 363 non vides, 1 204 de code, 1 488 lignes).
- *Hauteur des blocs* : le fichier est un test, absent de `tools/line_coverage_reference.txt` ; ses blocs peuvent changer de hauteur (consigne, point 8). `e2e-gap` reste exigé égal (il ne lit que `src/`).
- *Hachages, `CASES[]`, `check()`* : aucun. Les cas tournent par pointeur de fonction (`in_child`, `in_child_dying_by_asan`, `count_in_child`) ; aucune constante de hachage, aucun nom de cas lu par `argv`. La règle des noms de cas et celle des libellés de `check()` ne s'appliquent pas.
- *`TROU-N`, `TODO`, `pico8_*`* : aucun dans le fichier ; `PICO8_REMPLISSAGE` (ligne 1331) est un renvoi périmé en commentaire (la variable est `REPLAY_FILL` depuis 3l).
- *Lecteurs de la sortie du test* : aucun. `unit-run-test_live_slots` (`Makefile:863`) lance le binaire sans lire sa sortie ; `git grep places_vivantes` hors du fichier ne rend que `docs/` (TSV, vocabulaire) ; aucun script de `tools/` ni cible ne lit `tmp/test_places_vivantes_*`.

### Propriété des chaînes : ce que 4f1 touche, ce qui attend 4l3

La sortie `REPLAY_FILL` est écrite par `src/cmd/replay.c:47-79` (`SLOT_KIND_NAMES`, `PARTICLE_NAMES`, les gabarits `max_vivantes`/`max_octets`/`max_elements`, `partie`, `max_octets partie`, `chargement`, `max_octets chargement`) ; `test_live_slots.c` en est le seul lecteur (`replay_writes_fill_at_session_end` compose la sortie attendue et la compare par `strcmp` ; `replay_writes_session_peak` y cherche des clés par `fill_value`). Règle (méthode de l'étape 4, propriété des chaînes ; consigne point 6, amendement de 4c2 : la propriété se décide par l'écrivain de la chaîne lue, pas par le mot) : ces littéraux sont **lecteurs**, à 4l3. **4f1 n'y touche pas**, exactement ces 12 littéraux sur 10 lignes :

| Ligne | Littéral | Rôle |
|---|---|---|
| 55 | `"rangee"` dans `NAMES[]` | lecteur de `SLOT_KIND_NAMES` ; le même tableau sert aussi au libellé de `print_reading`, qui reste donc hybride (`rangee=` dans une ligne anglaise) jusqu'à 4l3 |
| 1355 | `"particule"` dans `PARTICLE[]` | lecteur de `PARTICLE_NAMES` |
| 1362 | `"max_vivantes %s %d\n"` | lecteur |
| 1367 | `"max_octets %s %lu\n"` | lecteur |
| 1377 | `"partie %lu\nmax_octets partie %lu\n"` | lecteur |
| 1380 | `"chargement %lu\n"` | lecteur |
| 1382 | `"max_octets chargement %lu\n"` | lecteur |
| 1416, 1417, 1419 (×2), 1420 | `"particule"`, `"max_vivantes particule"`, `"max_vivantes burst"` ×2 (et `"burst"`, anglais) | clés de `fill_value` |

**Rouge de contrôle, joué** : la seule ligne 1377 traduite côté test (`"game %lu\nmax_bytes game %lu\n"`) fait échouer `test_live_slots` à la ligne 1387 (`strcmp(writes, expected) == 0`), puis `in_child` à la ligne 227 ; `unit-run-test_live_slots` sort 134. C'est la preuve que la frontière passe là ; l'exécutant le rejoue avant le commit de chaînes et le cite dans le message, puis restaure la ligne. Proposition pour 4l3, alignée sur les noms de zone de 4c2 et sur le vocabulaire : `rangee` → `row`, `particule` → `particle`, `max_vivantes` → `max_live`, `max_octets` → `max_bytes`, `partie` → `game`, `chargement` → `load` (tampon `line[48]` : `max_bytes particles_help_screen 6264` fait 36 octets) ; à écrire dans la note, rubrique « Laissés ».

Tout le reste est écrit **et** lu dans le fichier, ou seulement imprimé : 4f1 le traduit.

### État 1 — chaînes (`refactor:`)

**Aucun lecteur hors du fichier, donc aucun rouge possible** — en tête du message, avec la preuve : `git grep -nF` de chaque chemin `tmp/test_places_vivantes_*` et du nom d'arène sur `src tests tools Makefile .claude` ne rend que le fichier ; les libellés vont à `stdout`/`stderr`, qu'aucune cible ni script ne lit ; aucune constante de hachage ; le nom d'arène n'atteint que le gabarit fatal de `engine/zone.c`, qu'aucun test ne lit avec ce nom. **Joué dans le clone** (`tmp/4f1_baseline/replay/stage1.py`, 137 remplacements dont trois textes répétés : `"hors ecran, kj laisse fo"` ×3, `"la zone partie prend des octets par tick"` ×2, `"p34 est hors ecran"` ×2) : 142 lignes changées, autant de `+` que de `-`, hauteur du fichier gardée ; `make test-all` vert, 26 e2e conformes ; `french-count` 942 → 609 (`string` 30 → 0) ; `cart-citations` 226 → 132 (`short` 224 → 130) ; `cart-names-count` inchangé (73 : aucune chaîne n'y comptait) ; les seuls littéraux à mot de contenu français restants sont les 12 de 4l3.

Sites et propositions (le prototype les applique ; l'exécutant relit chaque nom C dans le code — `reset_world` est l'ancien `jl`, `cart_start_game` l'ancien `eu`, `economy_tiers_rebuild_incoming_routes` l'ancien `bd`, `galaxy_populate` l'ancien `bw`, `icon_tooltips_table` l'ancien `dr`, les projets de recherche l'ancien `gb`) :

- *Chemins* (`ASAN_LOG`, `FILL`, `CAPTURE_A/B`, `DUMP_A/B`) : `tmp/test_places_vivantes_{asan.txt,remplissage.txt,a.bin,b.bin,a.txt,b.txt}` → `tmp/test_live_slots_{asan.txt,fill.txt,a.bin,b.bin,a.txt,b.txt}`.
- *Nom d'arène* (`icons_bytes`, ligne 566) : `"mesure des bulles"` → `"tooltip-measure"`, comme `test_game_zone.c` (4c2).
- *Étiquettes passées comme nom d'e2e* : `check_invariant("titre", i)` → `"title"` ; `print_reading("pause_restart recharge", …)` → `"pause_restart reload"`.
- *Libellés imprimés* (`printf`/`fprintf`, sans lecteur) : `partie=`/`partie -` → `game=`/`game -`, `registres` → `registries`, `particule=` → `particle=`, `inscrites`/`listes`/`eclats`/`trainees` → `registered`/`lists`/`bursts`/`trails`, `etats de bw` → `galaxy_populate states`, `chargement=` → `load=`, `prises refusees` → `refused grabs`, `prise posee` → `placed grab`, `slipway tenu` → `held slipway`, `slipways refuses` → `refused slipways`, `wormhole tenu/refuses/ouvert` → `held/refused/open wormhole(s)`, `sondes refusees` → `refused probes`, `par jo.oc` → `through the type picker`, `titre sans entree` → `title without input`, `remplissage ecrit par le rejeu` → `fill file written by the replay`, la ligne d'invariant de `main` et celle des fenêtres sans action ; espace avant les deux-points retirée (`f%d:`).
- *Messages d'`assert`* (une centaine, sans lecteur) : en anglais, par le nom C ou le rôle, jamais un digramme ni un mot de deux lettres qui soit un ancien nom (consigne, point 6, amendement de 4e). Exemples : `"jl() doit rendre toutes les places qu'il abandonne"` → `"reset_world must give back every slot it drops"` ; `"entre _init et eu, fm est l'economie de la partie quittee"` → `"between cart_init and cart_start_game, the main economy is the quit game's"` ; `"hors ecran, kj laisse fo"` → `"off screen, the draw leaves targeted_by set"` ; `"planete introuvable dans cg.np"` → `"planet not found under the node tag"` ; `"la prise posee reprend l'arene libre du mz de f4 (rendu a f48), et ne prend que by et cc en plus"` → `"the placed grab reuses the free arena of the f4 wormhole (released at f48), and only adds its route lists"` ; `"une planete a des besoins, que P ne compte pas"` → `"a planet has needs, which planets_bytes does not count"` ; `"gb est dans la sous-zone chargement"` → `"the research projects live in the load zone"`. **Piège observé** : la première proposition pour `"mo.p, mo.v, mo.ra, je.ra et ok.qa doivent etre reecrits en place"` (« … mouse_world of the ui … ») gardait le mot nu `ui`, ancien nom que `cart-citations` compte en `short` ; elle devient `"the camera's pos_before_shake, velocity and pos, the mouse's pos and main_ui's mouse_world must be rewritten in place"`. Chaque message réécrit se relit par `cart-citations` avant le commit.
- *Guillemets* : `« restart »`, `« harvest »` des messages deviennent `'restart'`, `'harvest'` (ASCII ; le texte cité est dessiné par le jeu, il ne change pas).
- Inchangés : noms d'e2e et de fixtures (`"pause_restart"`, `tests/fixtures/*.json`), identifiants de planète (`"p27"`…), clés d'étiquette et de message (`"planet"`, `"node"`, `"slipway"`, `"wormhole"`, `CV`/`CG`, traduites en 3a), états (`"settled"`, `"unknown"`, `"unfolding"`, `"vanishing"`), noms de singleton (3f), `"use-after-poison"`, et les 12 littéraux de 4l3.

Preuves, dans le message : l'absence de lecteur (commandes ci-dessus) ; le rouge de contrôle et sa restauration ; une **table de traduction** (`sed`, dans le message) qui rend les lignes `+` depuis les lignes `-` ; `git diff --numstat` égal en `+` et `-` ; tampons : aucun littéral réécrit n'entre dans un tableau de taille fixe (`line[48]` ne reçoit que les lignes de remplissage, inchangées ; `pattern[64]`/`motif[64]` les clés, inchangées ; `path[256]` les noms d'e2e) ; `make test-all` vert, `tests/e2e/*.sha256` inchangés, `tmp/line_coverage_gap.txt` égal.

### État 2 — locales (`refactor:`), procédure de 4b, 4c2, 4d, 4e

**Extraction fraîche de l'état 1** d'abord (les offsets bougent avec les messages), copie sous `tmp/ast_rename/declarations_state1.tsv`. Puis 9 identités, par `rename-apply` seulement (`tmp/ast_rename/decision.tsv`, en-tête `ancien nouveau fichier identité`). Identités observées dans le rejeu (à relever de nouveau) :

| Fonction | Ancien | Nouveau | Pourquoi |
|---|---|---|---|
| `main` | `e2e` (`@59578`) | `e2e_count` | seule candidate de `cryptic-locals` (portée 60) ; le nombre d'e2e rejoués |
| `pause_restart_keeps_quit_game_during_click` | `grab_arena` (`@11775`) | `used_before` | **nom qui ment** (dérivé de `prise`) : les octets pris avant le clic ; comme `test_drag_mouse_point` (4e) |
| `message_box_hold_rewrites_boxes_in_place` | `grab_arena` (`@48373`) | `used_before` | idem, avant le tick |
| `idle_windows_of_current_e2e` | `grab_arena` (`@50114`) | `used` | les octets pris à la frame courante, comparés à `second` |
| `icons_bytes` | `mesure` (`@21199`) | `measure_arena` | français ; comme `test_game_zone.c` (4c2, proposé pour 4f1 par sa note) |
| `pause_restart_reloads_after_title` | `neuf` (`@45916`) | `fresh` | français ; le relevé d'avant f0 |
| `fill_value` | `motif` (`@58186`) | `pattern` | français ; le motif cherché `"\n<clé> "` |
| `refused_slipways_do_not_grow_game`, `refused_wormholes_do_not_grow_game` | `un` (`@35343`, `@39700`) | `first_refusal` | français ; la zone après le premier refus, comparée à `twenty` |

Choix des noms : chaque partie est un mot validé du TSV (`e2e`, `count`, `used`, `before`, `measure`, `arena`, `fresh`, `pattern`, `first`, `refusal` : vérifié par `validated_words` de `tools/scope_count.py`) ; aucun ne masque une fonction, globale, macro, type ni locale de la même fonction (`declarations.tsv` : `used` n'est ailleurs qu'un champ de `bump_zone_t` et un paramètre de `research_display.c` ; `fresh`, `pattern`, `measure_arena`, `used_before` sont des locales ou paramètres d'autres fonctions). Gardés : les locales courtes de portée ≤ 40 lignes nommées d'un digramme (`hv`, `jo`, `jc`, `np`, `fm`, `mz`, `ie`, `mo`, `je`, `ok`, `bg`, `ll`, `by`, `lp`, `rl`, `oe`), D-29, et `nlp`/`nrl` (trois lettres, portée 10 et 9, hors liste) ; les commentaires réécrits ne les citent plus. `CV`/`CG` restent (D-13 sans verdict).

Joué dans le clone : `rename-apply` rend **29 occurrences dans 1 fichier** (`decl`, `ref`, `macro_arg`) ; `make test-all` vert, 26 e2e conformes. Après une extraction fraîche de l'état 2 (sans `warning:`, 4 `partial`) : `cryptic-locals MODULES=tests/test_live_slots` 1 → 0, sans `SUBSTEP` 92 → 91 ; `scope-count` 2 (D-13) ; `rename-rekey-check SUBSTEP=3l` 0 introuvable ; `french-count` 609 inchangé.

**TSV et surcharges, dans le même état** (prototype `tsv_edit.py`, qui retrouve chaque ligne par fichier, nom courant et fonction traduite par les lignes `fonction` du TSV) :
- 6 lignes `conservé` (`e2e`, `mesure`, `neuf`, `motif`, `un` ×2) passent `renommé`, nom neuf, identité courante ;
- 3 lignes déjà `renommé` à l'étape 3 (`prise` → `grab_arena`) prennent le nom neuf et l'identité courante et **gardent leur `ancien` `prise` et leur `concept`** (règle de 4e, D-42).

9 lignes `identity <identité> <nouveau>` dans `tools/rename_overrides.tsv`, **insérées après la dernière ligne `identity` de 4e** (`test_drag_mouse_point@grab_arena`, ligne 1352), pas en fin de fichier. Preuves observées sur l'extraction de l'état 1 : `rename_derive.py` reproduit les 6 lignes `conservé` → `renommé` à l'octet et les 3 lignes `prise` à l'octet hors colonnes `ancien` et `concept` ; `rename-table-check` 27 708 → 27 693 : 18 violations tombent (9 « missing row », 9 « row without extracted identity »), 3 apparaissent, une par ligne `prise` renommée deux fois (« says prise …, the extraction grab_arena … », D-42). Pas de `rename-rekey`.

**Effets de bord, mesurés** (même arbre, TSV d'avant puis d'après) :
- `e2e` devient un ancien nom ambigu : les deux mentions `` `e2e` `` du fichier comptent en `backtick` (`cart-names-count` 68 → 70 ; `TOTAL` 2 250 → 2 252) ; `grep -rn '`e2e`' src tests tools` n'en trouve aucune ailleurs ; l'état 3 les réécrit.
- `mesure`, `neuf`, `motif` deviennent d'anciens noms : aucune mention entre backticks dans le dépôt. `grab_arena` ne devient pas un ancien nom (l'`ancien` reste `prise`).
- **`un` devient un ancien nom de deux lettres** : `cart-citations` compte en `short` chaque « un » nu d'un commentaire ou d'un message, donc chaque article français restant. Sans `SUBSTEP`, sur l'arbre de l'état 2 : `short` 1 458 → 1 693 (+235) — dans le fichier lui-même (+20, partis à l'état 3) et dans des modules de sous-étapes **suivantes** seulement (`tools/composition_residue.sh` 25, `Makefile` 20, `tests/test_screencast_player.c` 19, `src/devtools/screencast.h` 15, `tests/test_headless_capture.c` 13, `tests/test_screencast.c` 12…, et `src/game/particle_zone.[ch]`, `tests/test_particle_zone.c` de 4f2, `src/game/ui_zone.h`, `tests/test_ui_zone.c` de 4h, `src/game/cart.c` de 4j) ; `cart-citations SUBSTEP=3a` à `3e` restent 0/0/0. Ces « un » sont des mots français que `french-count` compte déjà et que leurs sous-étapes réécriront : la charge ne change pas, seule la mesure de départ de `cart-citations` de ces sous-étapes monte. Choix de ce PRD : renommer quand même (le §1 ne tolère aucun mot français dans le code) et le noter dans le complément de D-36 ; l'exécutant remesure ces chiffres et les écrit dans la note.

### État 3 — commentaires (`docs:`), consigne étape 4

« Fichier d'avant » : l'état 2, copié sous `tmp/4f1_before_comments/` (consigne, point 8). Un agent pour le fichier (un fichier ne se coupe pas), consigne `docs/consigne-etape-4.md`, rapport au format amendé ; sans agent, l'exécutant applique la consigne lui-même et remplit le rapport, comme de 4a à 4e.

Preuves : sortie de `gcc -fpreprocessed -dD -E -P` identique à l'octet ; lignes de code inchangées (1 204 non vides) ; lignes de commentaire avant/après dans le message (159 au départ ; la hauteur peut changer, le fichier est un test).

Points connus, sans préjuger du rapport :
- *En-tête* (lignes 1-5) : « pilote comme main.c » → le pilotage de `src/cmd/game.c` (`cheat_step`, `cart_update60`, `cart_draw` par frame : vérifié) ; « le cart n'est chargé qu'une fois par processus » → `cart_load()` une fois par processus enfant (relire si c'est un contrat de `cart_load` ou un choix du test) ; ni « cart » nu, ni français.
- *Renvois périmés*, chacun vérifié par `grep` : `AVANT_F0` → `BEFORE_F0` ; `` `relever` `` → `take_reading` (lignes 307 et 1189) ; `` `dessiner` `` → le paramètre `draw` ; `` `rendre` `` → la libération du slipway ; `PICO8_REMPLISSAGE` → `REPLAY_FILL` ; « Fixture PRISES / SLIPWAYS / PLIAGES / SONDES / PLIAGE_ABANDONNE » → `GRAB_ARENAS`, `SLIPWAYS`, `WORMHOLES`, `PROBES`, `WORMHOLE_ABANDONED` ; « Empreinte `P` » → `planets_bytes`.
- *Digrammes réécrits* par la fonction C ou le rôle : `eu` → `cart_start_game`, `jl()` → `reset_world`, `bd` → `economy_tiers_rebuild_incoming_routes`, `bw` → `galaxy_populate`, `jo:ib`/`jo:cn`/`nr:cn` → `planet_rebuild_trades`/`planet_offer_trade`/`slipgate_offer_trade`, `kf` → `planet_attach_slipway`, `hv` → les entités vivantes (`live_entities()`), `cg.jo`/`cg.jc`/`cg.np` → les listes des étiquettes `planet`/`slipway`/`node`, `fo` → `targeted_by`, `kj` → le draw, `mz` → le trou de ver, `ox` → `anchors`, `oe`/`by`/`cc`/`jy` → `slipways`/`outgoing_routes`/`incoming_routes`/`needs`, `mo`/`je`/`ok`/`ju`/`bg`/`fv` → caméra, souris, `main_ui`, panneau de score, fond, écran d'aide, `fm` → l'économie principale, `ie`/`jb`/`dr`/`ie.gy` → icône de structure, bulle, `icon_tooltips_table`, la position partagée, `hw` → `panel_box`, `jo.oc`/`jo.ir` → `g_type_picker`/`g_planet_vanish_roll` (relire `populate_galaxy_states` : deux `random_walk_t` et un `weighted_picker_t`), `_init` → `cart_init`, `gb` → les projets de recherche. Les locales courtes conservées (`hv`, `jo`, `mz`…) ne sont pas citées entre backticks : leur rôle.
- *Mentions à réécrire par le rôle* : `` `e2e` `` ×2 (effet de l'état 2) ; `` `frames` ``, `` `test` `` (paramètres) ; `` `particule` ``, `` `rangee` `` (étiquettes de 4l3 : par leur rôle, « the particle count », « the row slot kind »).
- *Format du remplissage* (lignes 1331-1336) : contrat d'un fichier qu'un autre programme écrit (destination 4, exception « format de fichier ») ; il décrit l'ordre des lignes **par leur rôle, sans citer les étiquettes** françaises, pour rester vrai après 4l3.
- *Affirmations à revérifier en traduisant* (consigne, rubrique 7) : « les registres prennent 2C − 4 pointeurs » (somme 4 + 8 + … + C de `registry_bytes` : vrai pour C puissance de deux ≥ 4) ; « sept icônes `ie` de `hv`, dans l'ordre de `dr` » (`ICON_TOOLTIPS_COUNT` vaut 7, `map_tables.h:11`) ; « aucune particule de l'aide ne meurt avant 115 frames : 5 par draw » (relire l'émetteur de l'écran d'aide et sa durée de vie dans les données) ; les frames décrites des cinq fixtures (relire les `.json` ou le cas qui les asserte) ; « Le `bd` de f139 garde la route du slipway posé à f136 » ; « f7 : restart » de `pause_restart` ; « Recharger rend les places des entités de hv » ; le contrat de `read_file` (tampon terminé par un NUL, libéré par l'appelant).
- *Pièges et contrats à garder* (destinations 3 et 4, en anglais, 40 mots au plus) : le `volatile` de `g_spend` ; « sans `take_reading` » de `play_until` et des fenêtres sans action (son vidage rendrait la génération en avance) ; « lu après un vidage » de `check_invariant` ; « un rejeu par processus enfant » ; le `static` de `far` ; `same_counts` hors `particle` ; le propriétaire du tampon de `read_file`.
- *Récit à sortir vers le message de commit* : ce qui raconte l'histoire du test plutôt que ce qu'il vérifie.

### Ce qui reste hors de 4f1

- Les 12 littéraux lecteurs de `REPLAY_FILL` (§ Propriété), à 4l3 avec leur écrivain `src/cmd/replay.c`, son commentaire d'en-tête (lignes 24-27) et ses messages `"replay: impossible d'ouvrir…"`.
- `CV`, `CG` (D-13), les paramètres et locales courts de digramme (D-29), `PICO8_CLASS_COVERAGE` (D-2) : non tranchés, non appliqués.
- Les noms de fonction et la macro qui disent « grab arena » pour une prise (D-45, neuve) ; la fixture `tests/fixtures/grab_arenas_refused.json` (4f2).
- La macro `MAX_READINGS` (ligne 53), définie et jamais employée : notée, pas supprimée (hors des trois sortes de commit).

### Registre (`docs/decisions-plan-5.md`)

Une entrée neuve, au format des conventions, ligne d'index comprise :
- **D-45** (4f1) — noms de test qui disent « grab arena » pour une prise. Dans `test_live_slots.c`, « prise » désigne le geste (une structure tirée de son icône et tenue), pas l'arène : `refused_grab_arenas_do_not_grow_game`, `placed_grab_arena` (rend la planète posée), `placed_grab_arena_reuses_dead_wormhole_arena`, `refused_grab_arena_crosses_new_game`, `offscreen_planet_forgets_refused_grab_arena`, la macro `GRAB_ARENAS` et la fixture `tests/fixtures/grab_arenas_refused.json` (4f2) suivent le concept `prise` → `grab arena` du vocabulaire. Options : (a) `grab_arena` → `grab` dans ces cinq fonctions et la macro (`refused_grabs_do_not_grow_game`, `placed_grab`, `placed_grab_reuses_dead_wormhole_arena`, `refused_grab_crosses_new_game`, `offscreen_planet_forgets_refused_grab`, `GRABS`), fixture `grabs_refused.json` (ligne `fichier`, procédure de 3l) ; (b) garder. Sites : à mesurer par le `grep` des conventions (de l'ordre de 3 par fonction, 5 pour la macro, 2 pour la fixture à `b6f4681`). Proposition (a), procédure N ; noms neufs à 0 site. Portée : mentions en commentaire comprises.

Complément d'une entrée existante, sans verdict : **D-36** — un renommage de locale de l'étape 4 rend aussi compté un mot français de deux lettres : `un` → `first_refusal` (4f1) fait compter à `cart-citations` chaque « un » nu des sous-étapes suivantes (+235 `short` sans `SUBSTEP`, chiffre de l'exécution) ; aucune sous-étape close n'est touchée. La règle (b) de D-36, étendue à `tools/cart_citations.py`, l'aurait évité.

Si le rapport d'agent soulève une autre question de nommage ou un faux positif qui résiste, une entrée s'ajoute de même ; la note le dit dans tous les cas.

### Fin de PRD

Ordre : `make rename-extract` frais sans `warning:` (4 `partial`) ; `rename-rekey-check SUBSTEP=3l` vert ; compteurs sur `MODULES=tests/test_live_slots`, puis `SUBSTEP=3a` à `3e`, puis sans `SUBSTEP`.

**Critère de fin** :
- `french-count MODULES=tests/test_live_slots` : 0, quatre colonnes ;
- `cart-names-count MODULES=tests/test_live_slots` : 0, quatre colonnes ;
- `cart-citations MODULES=tests/test_live_slots` : 0/0/0 ;
- `cryptic-locals MODULES=tests/test_live_slots` : 0 ;
- `scope-count MODULES=tests/test_live_slots` : 2 (`CV`, `CG`, D-13) ;
- `SUBSTEP=3a` à `3e` : `french-count`, `cart-names-count` et `cart-citations` toujours 0 ;
- relevé complémentaire des mots de contenu : exactement les 12 littéraux de 4l3 (et des homographes anglais, listés dans la note) ;
- `git grep -n 'places_vivantes\|mesure des bulles' -- src tests tools Makefile` : 0 ;

**Invariant (§5)** : `make test-all` vert à chacun des trois états, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ; `rename-tool-test` et les huit `rename-*-test` verts ; `class-coverage-net` et `e2e-gap` rouges, sorties égales à celles de `tmp/4f1_base/` (`e2e-gap` par `tmp/line_coverage_gap.txt`) ; `data-residue` et `composition-residue` à 0, `--detail` et `--champs` identiques ; `make sin-table` reproduit `quarter_wave.h` à l'octet.

### Note « Découvert » de 4f1 (sous l'entrée 4f de `docs/plan-5.md`, « morceau 4f1 », comme 4c1)

Elle porte : mesure de départ et de fin (relevé complémentaire compris) ; la frontière avec 4l3 (les 12 littéraux lecteurs, le rouge de contrôle, la proposition de noms pour 4l3, la ligne hybride de `print_reading`) ; l'absence de lecteur hors du fichier et pourquoi aucun rouge ; les 9 identités, les noms qui mentent, les chiffres de `rename-table-check` et de `rename_derive.py` ; les effets de bord de `e2e` et de `un` ; lignes de commentaire et affirmations corrigées de l'état 3 ; D-45 et le complément de D-36 ; `MAX_READINGS` ; et les amendements de `docs/consigne-etape-4.md` tirés de l'exécution — au moins ceux-ci, si elle les confirme :
- *Point 6* : un même littéral peut être à la fois libellé imprimé et lecteur de la sortie d'un binaire (`NAMES[]` de `test_live_slots.c`) : il reste lecteur, donc à la sous-étape de l'écrivain ; la ligne imprimée reste hybride jusque-là.
- *Point 6* : un commentaire qui décrit un format écrit par une autre sous-étape le décrit par le rôle des lignes, sans citer des étiquettes que cette sous-étape traduira.
- *Point 8* : avant de renommer une locale au nom français de deux lettres (`un`), mesurer `cart-citations` sans `SUBSTEP` avec le TSV d'avant et d'après : le mot devient un ancien nom et chaque occurrence nue des sous-étapes suivantes compte en `short` ; les sous-étapes closes doivent rester à 0.

## Implementation Chunks

Un seul morceau (charge 1 242 ≤ 1 382, un seul fichier). L'exécutant produit, dans cet ordre, `make test-all` vert à chaque état :
1. rouge de contrôle joué puis restauré ; état 1, chaînes ; `git diff HEAD > tmp/4f1_stage1.patch`, message `tmp/4f1_1_msg.txt` (`refactor: plan 5 morceau 4f1 — messages, libellés, chemins et nom d'arène de test_live_slots en anglais`, absence de lecteur, frontière avec 4l3 et table de traduction en tête) ;
2. extraction fraîche, état 2, locales, TSV et surcharges ; `git diff HEAD > tmp/4f1_stage2.patch` (cumulatif : états 1 et 2), message `tmp/4f1_2_msg.txt` (`refactor: plan 5 morceau 4f1 — locales de test_live_slots au nom de leur rôle`) ;
3. état 3, commentaires, note « Découvert », amendements de la consigne, D-45 et le complément de D-36, laissés dans l'arbre de travail ; message `tmp/4f1_3_msg.txt` (`docs: plan 5 morceau 4f1 — commentaires de test_live_slots en anglais, note Découvert`).

## Remise

L'exécutant ne commite pas. Il laisse :
- `tmp/4f1_stage1.patch` : diff complet de `HEAD` à l'état 1 (s'applique par `git apply` sur `HEAD` propre) ;
- `tmp/4f1_stage2.patch` : diff complet de `HEAD` à l'état 2, cumulatif ;
- `tmp/4f1_1_msg.txt`, `tmp/4f1_2_msg.txt`, `tmp/4f1_3_msg.txt` : messages de commit en français, récit de chantier et preuves compris, terminés par les lignes d'attribution de la session ;
- l'arbre de travail à l'état 3 : commentaires, note, consigne, registre.

## Testing Decisions

- Aucun test neuf, aucune assertion ajoutée ni modifiée dans sa condition (consigne, points 5 et 7) : seuls des messages et des littéraux sans lecteur changent. Le filet : le test lui-même (il compare la sortie du rejeu à l'octet : le rouge de contrôle prouve qu'il voit une étiquette déplacée), le compilateur et les 26 e2e pour l'état 2, la sortie du préprocesseur pour l'état 3.
- Un bon contrôle juge une sortie observable : statut de sortie, `*.sha256`, fichier de gap, sorties à l'octet des gardes hors suite, compteurs sur extraction fraîche.
- Prior art : 4e (`a74ae48` chaînes, `35e704a` locales avec TSV, surcharges et D-42, `c4f6777` commentaires) ; 4c2 (`mesure` → `measure_arena`, `"tooltip-measure"`, frontière avec `REPLAY_FILL`) ; 4c1 (messages hybrides voulus jusqu'à la sous-étape de l'écrivain).

## Out of Scope

- Les 12 littéraux lecteurs de `REPLAY_FILL` et tout `src/cmd/replay.c` (4l3).
- Toute ligne de code hors des chaînes de l'état 1 et des 9 identités de l'état 2 ; tout fichier hors de `tests/test_live_slots.c`, sauf le TSV, les surcharges, le plan, la consigne et le registre.
- Les noms de fonction, de macro et de fixture (D-45), `CV`/`CG` (D-13), toute entrée du registre sans verdict ; `TROU-N` → `GAP-N` (étape 5) ; `MAX_READINGS`.
- Le formatage du code, les outils de comptage et leurs règles (D-36 (b) reste une proposition).

## Further Notes

- Le fichier n'est cité par aucune donnée de `tools/` : la hauteur des blocs y est libre, et aucune garde à motifs (`composition_residue.sh`, `data_residue.sh`, hors `tests/`) ne le lit.
- Si l'humain préfère ne pas renommer `un` avant l'exécution (pour éviter les +235 `short` des sous-étapes suivantes), l'état 2 perd deux identités et deux lignes TSV ; le mot reste alors un identifiant français, à porter au registre.
- Le libellé `jo.oc` relevé par 3f part à l'état 1 (« through the type picker ») ; D-31 n'en compte plus ce site.
