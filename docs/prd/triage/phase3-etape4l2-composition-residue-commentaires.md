---
title: "Plan 5, morceau 4l2 — garde de composition en anglais : catégories, messages, données, noms du script et commentaires"
description: "Mettre tools/composition_residue.sh, ses données et son test en anglais — quinze noms de catégorie lus par le Makefile et le test traduits d'un coup avec tous leurs lecteurs, messages, pieds, option --champs → --fields, variable d'environnement, notes de composition_residue_sites.txt, noms internes du script, puis commentaires — sans qu'une mesure change, prouvé par les sorties de tous les modes égales par une table de traduction, la base de make égale par la même table, puis des sorties identiques à l'octet."
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
>    - 4l2 (971) : `tools/composition_residue`, `tools/composition_residue_sites`, `tests/test_composition_residue`.
>
>    Un morceau (4c1…) suit la PRD de sa sous-étape, avec ses commits et `make test-all` vert ; ses compteurs se lisent par `MODULES="<sa liste>"`, et `SUBSTEP=3x` à zéro clôt la sous-étape après son dernier morceau.

4l2 est le deuxième morceau de 4l : la garde de composition (`tools/composition_residue.sh`), sa liste de sites (`tools/composition_residue_sites.txt`) et son test (`tests/test_composition_residue.c`). Ses compteurs se lisent par `MODULES="tools/composition_residue tools/composition_residue_sites tests/test_composition_residue"` (noté `MODULES=<4l2>` ci-dessous). 4l2 ne clôt pas 4l : `SUBSTEP=3l` ne tombe à zéro qu'après 4l4. Il tient aussi la ligne `COMPOSITION_RESIDUE_GUARD` du `Makefile`, que 4l1 lui a laissée (note de 4l1, « Laissés ») : c'est un lecteur des catégories, dont l'écrivain est le script.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4). Étape 4l2 = l'entrée 4 appliquée au morceau 4l2, tel que la note de 4a l'a découpé (entrée 4l de `docs/plan-5.md`, liste ci-dessus).

Entrées lues : `docs/plan-5.md` §1, §2, §4, §5, l'entrée 4 et sa méthode, l'entrée 3l et sa note « Découvert », les notes de 4a à 4l1 ; `docs/consigne-etape-4.md` avec tous ses amendements (dont les points 6 et 8 amendés par 4i, 4k1, 4l1 : message sans lecteur, preuve d'un script, base de `make`) ; `docs/decisions-plan-5.md` (D-5 à D-12 portent sur les motifs, exemptions et catégories de ce script : aucune n'est appliquée) ; `CLAUDE.md` ; `.claude/skills/reprendre-les-commentaires/` (SKILL, `consigne-agent.md`, `modele-rapport.md`) ; `git log -5 --stat` (forme des commits de 4l1 : `refactor:` des messages, puis `docs:` des commentaires avec note, consigne et registre, puis la gradation de la PRD).

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même le skill `guideline` aux points « vérifier avec l'utilisateur » de `to-prd` (modules, tests, découpage) et l'a appliqué (changements chirurgicaux, critères vérifiables, hypothèses dites, aucune décision du registre préjugée). Sa règle 5 (« un commentaire est faux, une odeur ou un aveu ») cède devant la règle du `CLAUDE.md` et de la consigne de l'étape 4 (trois destinations), comme à 4a–4l1.

Tout ce qui suit a été **rejoué** dans un clone jetable de `efd9945` (supprimé ensuite) ; les artefacts sont sous `tmp/4l2_baseline/replay/` (patchs `s1.patch`, `s2.patch`, scripts `stage1.py`, `stage2.sed`, `capture.sh`, tables `categories.tsv`, `output.sed`, `sites_col3_table.tsv`, prototype `sites.txt`, sorties `before/`, `after/`, `after_s2/`, bases `db_before.txt`, `db_after.txt`, journal rouge `red_script_only.log`, piège `traps.txt`, compteurs `after_s1_counters/`). Le dépôt réel n'a reçu que des écritures sous `tmp/` (extraction fraîche, relevés).

**Condition d'entrée** : `origin/plan-5-noms-anglais` à `efd9945` ou un descendant (la PRD seule au-dessus), arbre propre, `make test-all` vert (26 e2e) ; `make rename-extract` frais sans `warning:` (4 lignes `partial` de `class_coverage` attendues) ; `rename-rekey-check SUBSTEP=3l` vert (0 introuvable) ; `rename-strings-verify SUBSTEP=3l` : 1 site en défaut sur 44 (le repli de `tools/frames_ref.sh`, D-1), 9 `unseen`. Tous retrouvés à `efd9945` (`tmp/4l2_baseline/extract0.log`, `rekey.txt`, `rsv.txt`).

## Problem Statement

La garde de composition est une des gardes de `make test` (`composition-guard`) et le compteur `make composition-residue`. C'est aussi le plus gros bloc de français restant hors des scripts de 4l4 :

- **ses noms de catégorie sont français** (`parente`, `lien-parent`, `accesseurs-q6`, `défauts`, `aiguillage-nom`, `un-ou-faux`, `ptrs-entité`, `registres`, `noyau-inclut`, `atteinte-à-prouver`, `inclusion-mutuelle`, `usage-mutuel` ; seuls `nu`, `void-ptr` et `void-self` ne le sont pas) et trois portent un accent. Ce ne sont pas des libellés : c'est l'**interface en ligne de commande** du script. Le `Makefile` les lui passe (`--check $(COMPOSITION_RESIDUE_GUARD)`, seule ligne non ASCII du `Makefile` depuis 4l1), le script les imprime dans chaque mode, et `tests/test_composition_residue.c` lit ces sorties à l'octet et en passe à `--check`. Le compteur de français en voit les mots-outils (`un`, `ou`, `à`, `entité`, `défauts`) dans les trois fichiers ; il ne voit pas `parente`, `registres`, `noyau`, `atteinte`, `mutuelle`, mots de contenu ;
- ses messages sont français : les pieds `résidu 2 : N ligne(s), M fichier(s)` et `résidu gardé (<cats>) : N ligne(s)`, le pied de `--champs` (`champs : N ; 0 écriture : K ; 0 hors constructeur : M`), la ligne `info … N lignes` de `--detail`, les onze refus (`categorie inconnue : …`, `echec de grep`…) ; l'option `--champs` et la variable d'environnement `COMPOSITION_ATTEINTE` aussi ;
- `tools/composition_residue_sites.txt` (11 lignes de notes tabulées) écrit sa colonne 3 — imprimée par `--detail` sous `void-ptr-r2` — en français, avec des digrammes du cart (`hv`, `cv`, `cg`, `jo.oe`) et des renvois à une étude disparue (`(2.11r-b1)`, `(2.20c)`…) ; sa colonne 4 cite `bucket_src` (ancien nom compté depuis 4c2), des numéros de ligne périmés (`entity.c:217`, `cart.c:208-209`, `help.c:180`…) et des digrammes ;
- ses noms internes sont français (`SORTIE`, `NOYAU`, `ATTEINTE`, `LIEN_EXCLUSION`, `PTRS_NON_ENTITE`, `REGISTRES_NON_ENTITE`, `read_registres`, `champs`, `non_entite`, `out_parente`…) ;
- ses 129 lignes de commentaire et les 38 du test sont françaises, citent douze anciens noms (`pico8_ptrs_t` ×5, `pico8_nv_ptrs`, `pico8_jo_ex`…), des digrammes (`hy.invented`, `jo.jy`, `hp.i`, `jc.ts`, `ke`, `jo`), des sites `fichier:ligne` à revérifier, et du récit de chantier (« la phase 2 », « la règle 1 », « l'étude 2.11 (usegraph.py) », fichier absent du dépôt).

Traduire les catégories est risqué parce que c'est un contrat : le script traduit seul, `composition-guard` refuse la liste du `Makefile` (code 2) et le test rougit ; le `Makefile` traduit seul, même refus. Et une traduction qui toucherait l'ordre des catégories, les fichiers de travail qui portent leur nom, ou la sémantique de `--detail`/`--champs`, changerait ce que la garde mesure sans qu'aucun compteur le voie.

## Solution

Appliquer à 4l2 la méthode de 4a à 4l1, en morceau (`MODULES=<4l2>`), en **trois états de code** :

1. **Chaînes, catégories, données** (`refactor:`) : les quinze catégories passent en anglais (douze traduites, `nu`, `void-ptr`, `void-self` gardées) **à tous leurs sites d'un coup** — le script (écrivain), le `Makefile` et le test (lecteurs) — avec les messages, les pieds, l'option `--champs` → `--fields`, la variable `COMPOSITION_ATTEINTE` → `COMPOSITION_REACHABILITY`, les colonnes 3 et 4 de `composition_residue_sites.txt` et les attentes du test. Rouge d'abord (écrivain seul : `composition-guard` en code 2 et `test_composition_residue` à la ligne 77), puis vert avec les lecteurs. Preuve : sortie de chaque mode et de chaque refus, sur l'arbre réel, passée par une table de traduction, égale à l'octet à la sortie d'après, codes compris ; base de `make` égale par la même table.
2. **Noms internes du script** (`refactor:`) : variables, fonctions, variables `awk` et fichiers de travail français renommés à la main (le script n'est pas dans l'AST ni au TSV) ; preuve : toutes les sorties **identiques à l'octet**, `sh -n`, `make test` vert.
3. **Commentaires** (`docs:`), avec la note « Découvert », les amendements de la consigne et le registre : commentaires du script et du test en anglais, sans cart, sans récit, affirmations revérifiées ; preuve : `diff` des lignes hors commentaire vide et sorties identiques pour le script, `gcc -fpreprocessed` identique pour le test.

L'exécutant ne commite pas : il laisse deux patchs et trois messages dans `tmp/`, et l'arbre de travail à l'état final (§ « Remise »). À la fin, sur `MODULES=<4l2>` : les cinq compteurs à zéro, toutes colonnes.

## User Stories

1. En tant que mainteneur de la garde de composition, je veux des noms de catégorie anglais (`parent-link`, `entity-ptrs`, `mutual-usage`…), afin de lire `--check`, `--detail` et le pied sans traduire.
2. En tant qu'humain qui lance `make test`, je veux que `composition-guard` reste vert et garde exactement les mêmes quinze catégories, dans le même ordre, afin que la garde ne perde rien en changeant de langue.
3. En tant que lecteur du `Makefile`, je veux que `COMPOSITION_RESIDUE_GUARD` soit en ASCII et nomme les catégories que le script connaît, afin que le `Makefile` n'ait plus aucun caractère non ASCII.
4. En tant que relecteur du commit de chaînes, je veux un rouge joué par l'écrivain seul (le script traduit, ses lecteurs non), afin de savoir que le test et la garde lisent vraiment ces noms et ces messages.
5. En tant que relecteur, je veux que chaque sortie du script — sans option, `--detail`, `--fields`, `--check` de la liste du `Makefile` et d'une seule catégorie, et les huit refus en code 2 — soit égale à l'octet, passée par une table de traduction publiée, à la sortie d'avant, codes compris, afin d'être sûr que seule la langue a changé.
6. En tant qu'utilisateur de `--detail` et `--champs`, je veux que leur sémantique reste identique (mêmes lignes, même ordre, mêmes nombres : 260 champs, 15 sans écriture, 27 sans écriture hors constructeur), afin que les relevés de 4l3, 4l4 et de l'étape 5 se comparent à ceux d'avant par la table.
7. En tant que lecteur de `--fields` (ex-`--champs`), je veux une option et un pied anglais, afin que l'option dise ce qu'elle relève.
8. En tant que test, je veux que `COMPOSITION_REACHABILITY` remplace `COMPOSITION_ATTEINTE` des deux côtés dans le même commit, afin que ma liste d'atteinte de fixture soit toujours lue.
9. En tant que lecteur de `--detail`, je veux que les lignes `void-ptr-r2` disent en anglais, avec les noms C actuels, pourquoi chaque site n'a pas d'amont non typé, sans renvoi à une étude disparue, afin que la note reste vraie sans document à côté.
10. En tant que lecteur de `composition_residue_sites.txt`, je veux une colonne 4 qui nomme les appelants par fonction et par fichier, sans numéro de ligne périmé ni digramme du cart, afin de pouvoir la revérifier par `grep`.
11. En tant que mainteneur de `rename-strings-verify`, je veux que les noms que le TSV attend dans le script et dans la liste de sites (colonne 1, valeurs de `PTRS_NON_ENTITE`/`REGISTRES_NON_ENTITE`, `entity_state_of`, `entity_t`) restent intacts, afin que `SUBSTEP=3l` reste à 1 site en défaut sur 44.
12. En tant que lecteur du script, je veux des noms internes anglais (`FOOTER_CATEGORIES`, `CORE_HEADERS`, `REACHABILITY`, `read_registries`, `fields`…), afin que plus aucun mot français ne reste dans le code.
13. En tant que relecteur du commit des noms internes, je veux des sorties identiques à l'octet et `sh -n` vert, afin de savoir qu'aucun renommage n'a touché une valeur.
14. En tant que lecteur de l'en-tête du script, je veux un contrat d'usage en anglais qui décrit chaque mode, chaque format de sortie et chaque code de sortie tels qu'ils sont après l'état 1, afin de pouvoir écrire un lecteur sans lire le code.
15. En tant que lecteur des listes d'exemption, je veux que chaque exemption nomme le membre ou la fonction C et l'écrivain qui la prouve, revérifiés, afin de pouvoir rejouer la preuve.
16. En tant que lecteur du test, je veux des commentaires anglais qui disent pourquoi chaque fixture a la forme qu'elle a (jetons coupés en deux littéraux, formes vues et limites nommées), sans digrammes du cart, afin de ne pas retirer par erreur une coupure qui protège le test de sa propre mesure.
17. En tant que relecteur du commit de commentaires, je veux une preuve qui voie un commentaire glissé dans un programme `awk` entre apostrophes, afin qu'une apostrophe anglaise (« entity's ») ne casse pas le script sans que le `diff` des lignes hors commentaire le voie.
18. En tant que mainteneur du registre, je veux que D-5 à D-8, D-10, D-11 et D-12 citent les noms neufs (catégories, `PARENT_LINK_EXCLUSION`, `--fields`) et les numéros de ligne d'après l'état 3, et deux entrées neuves (fixtures `pico8_` du test, macro `ATT`), afin que l'humain tranche avant l'étape 5 sur des renvois justes.
19. En tant que pilote du plan, je veux une note « Découvert » qui donne les compteurs de fin de 4l2 sur `MODULES=<4l2>`, `SUBSTEP=3l` et sans `SUBSTEP`, et la forme anglaise des pieds pour que 4l4 l'aligne sur `tools/data_residue.sh`, afin que 4l3 et 4l4 partent d'une mesure connue.
20. En tant qu'orchestrateur, je veux deux patchs et trois messages dans `tmp/`, afin de commiter les trois sortes de commit séparément, `make test-all` vert à chacun.

## Implementation Decisions

### Mesure de départ (`efd9945`, extraction fraîche, écrits dans `tmp/4l2_baseline/` seulement)

`make rename-extract` : aucun `warning:`, 4 lignes `partial` (`extract0.log`) ; `rename-rekey-check SUBSTEP=3l` : 0 introuvable (`rekey.txt`) ; `rename-strings-verify SUBSTEP=3l` : 1 en défaut sur 44, 9 `unseen` (`rsv.txt`) ; `make test` et `make e2e` verts (rejoués dans le clone).

Sur `MODULES=<4l2>` (`sh tmp/4l2_baseline/counters.sh . <sortie>`) :

| Compteur | id | backtick | string | comment | total |
|---|---|---|---|---|---|
| `french-count` | 0 | 0 | 206 | 611 | 817 |
| `cart-names-count` | 0 | 0 | 1 | 12 | 13 |
| `cart-citations` (cart/lua/short) | — | — | — | 0/0/175 | 175 |
| `cryptic-locals` | — | — | — | — | 0 |
| `scope-count` | — | — | — | — | 0 |

Charge **1 005** (817 + 13 + 175), contre 971 dans la note de 4a : +33 `short` (les « un » rendus comptés par le renommage `un` → `first_refusal` de 4f1, D-36) et +1 `string` (`bucket_src`, ancien nom distinctif depuis le renommage `bucket_src` → `message_bucket` de 4c2, cité par la colonne 4 de la ligne `draw_layers` de la liste de sites). Sous le seuil de 1 382 : un seul morceau.

Par fichier :
- `tests/test_composition_residue.c` : français 0/0/123/150 (`string` : `résidu`, `gardé`, `ligne`… des attentes, `défauts`, `un`, `ou`, `entité`, `à` des catégories, `écriture` du pied de `--champs`) ; citations `short` 40 (`ke`, `jo`, `un`, `et`, `le`, `du`, lettres `b`, `d`, `e`, `f`, `g`, `u` des commentaires de cas) ; anti-cart 0 (les fixtures `"pico8_" "jo_t"`, coupées en deux littéraux, ne sont vues par aucun compteur : D-57 ci-dessous) ; 38 lignes de commentaire (définition de la consigne), 733 lignes non vides après le préprocesseur.
- `tools/composition_residue.sh` : français 0/0/41/461 (`string` : catégories des lignes 47, 50, 61, 64, 246, 606, 614, refus, `de` de `echec de …`, pieds) ; anti-cart 0/0/0/12 en commentaire (`pico8_ptrs_t` ×5, `pico8_nv_ptrs`, `pico8_gu_content_ptrs`, `pico8_jo_ex`, `pico8_jp_ptrs`, `pico8_la_ptrs`, `pico8_jp_ents`, `pico8_ents_cat` ; relevé par `tmp/4l2_baseline/anticart_sites.py`) ; citations `short` 135 (articles et digrammes `hy`, `jo`, `jy`, `hp`, `i`, `jc`, `ts`, `qy`, `ji` des listes d'exemption, `.c`/`.h`, `a -> b`) ; 746 lignes, 129 lignes `#` (toutes en colonne 0), 617 hors commentaire ; aucun commentaire en fin de ligne de code ni dans un programme `awk`.
- `tools/composition_residue_sites.txt` : français 0/0/42/0 (`aucun`, `est`, `typé`, `prédicat`…) ; anti-cart 0/0/1/0 (`bucket_src`) ; aucune ligne de commentaire.

Sur `SUBSTEP=3l` (les chiffres de fin de 4l1) : français `done` 0/0/517/1 925 (2 442), `later` 0 ; anti-cart `done` 0/0/2/34 (36) ; citations 13/4/513 ; `cryptic-locals` 17 ; portée 0. Sans `SUBSTEP` : anti-cart `TOTAL` 36, citations 13/4/513, français 2 468 (543 `string`, 1 925 `comment`), `cryptic-locals` 19, `scope-count` 12 (D-13). `SUBSTEP=3a` à `3j` : `tmp/4l2_baseline/others.txt`, égal à l'octet à `tmp/4l1_baseline/others.txt`.

### Le contrat de la ligne de commande : qui lit quoi

Relevé par `git grep` de chaque nom et de chaque message sur tout le dépôt :
- **Noms de catégorie** : écrits par le script (`CATEGORIES`, liste du pied, cas de `pattern()`, `exclusion()`, `collect()`, fichiers de travail `"$work/<cat>"` et `"$work/<cat>.pairs"`), lus par la ligne `COMPOSITION_RESIDUE_GUARD` du `Makefile` (passée à `--check`) et par `tests/test_composition_residue.c` (attentes à l'octet, arguments de `--check`). Les docs (notes du plan, registre D-5 à D-11) les citent aussi : le plan et les notes sont de l'historique, intouchés ; le registre reçoit des compléments (ci-dessous). Aucun autre code, aucune skill, ni `README` ni `CLAUDE.md`.
- **Pieds et messages** : lus par le seul test (à l'octet ; `strstr` de `"résidu 2 : 0"` et de `nope` dans `ERR`). `tests/test_data_residue.c` lit `résidu D` et `résidu gardé` — c'est la sortie de `tools/data_residue.sh`, autre écrivain (4l4) : intouché.
- **`--champs`** : lu par le test (trois appels) ; aucune cible du `Makefile`. Les notes et les PRD citent `--champs` dans leurs relevés : après 4l2, les relevés se font par `--fields` (amendement de la consigne).
- **`COMPOSITION_ATTEINTE`, `COMPOSITION_R2`** : lues par le script, posées par le seul test (`run_residue`). `R2` n'est pas français : gardée.
- **Colonne 3 de la liste de sites** : imprimée par `--detail` (`void-ptr-r2\t<col. 1>\t<col. 3>`) ; colonnes 2 et 4 lues par personne (le script n'imprime que `$1` et `$3`) ; colonne 1 lue par `rename-strings-verify` (lignes `chaîne` du TSV) : intouchée.
- **Valeurs lues par `rename-strings-verify SUBSTEP=3l`** dans le script (`emitter_t.particles`, `collect_*_pointer_list`, `planet_t.needs`, `entity_state_of`, `entity_t`…) : intouchées, seuls les noms des variables qui les portent changent à l'état 2.
- `tests/test_rename_strings*.py` et `test_rename_substeps.py` citent le chemin du script dans leurs fixtures, pas son contenu : intouchés.

**Décision : traduire les catégories, à tous leurs sites dans un seul commit** (propriété des chaînes : l'écrivain est le script, 4l2 tient l'écrivain et ses deux lecteurs). 4l1 l'a prévu (sa user story 15 et sa note, « Laissés »). Ne pas les traduire laisserait cinq mots-outils comptés `string` dans trois fichiers (dont le `Makefile` de 4l1, clos à 7 en attendant 4l2) et des mots de contenu que le compteur ne voit pas (consigne, amendement de 4c1) : il faudrait les proposer en exception, pour un contrat qu'aucun lecteur extérieur ne fige. La traduction ne change ni l'ordre (le pied et `--detail` suivent l'ordre de `CATEGORIES`, `--check` celui de son argument, `--detail` trie les fichiers et les paires, jamais les catégories), ni les mesures.

Table des catégories (`tmp/4l2_baseline/replay/categories.tsv`) — douze traduites, trois gardées :

| Avant | Après | Pourquoi ce nom |
|---|---|---|
| `parente` | `parent` | struct parente enchâssée en premier membre |
| `lien-parent` | `parent-link` | accès à la parente (`_of`, `->parent`) |
| `accesseurs-q6` | `header-accessors` | accesseurs des champs d'en-tête (`get_pos`…, cf. le cas `test_accessors_bite_on_every_header_field_name`) ; `q6` renvoyait à une question d'étude disparue |
| `défauts` | `defaults` | |
| `nu` | `nu` (gardé) | nomme l'ancien champ qu'il chasse ; D-11 en propose le retrait |
| `aiguillage-nom` | `name-dispatch` | |
| `un-ou-faux` | `one-or-false` | |
| `void-ptr`, `void-self` | gardés | déjà anglais |
| `ptrs-entité` | `entity-ptrs` | |
| `registres` | `registries` | nom déjà pris par trois cas du test (`test_registries_*`) |
| `noyau-inclut` | `core-includes` | |
| `atteinte-à-prouver` | `reachability-to-prove` | la liste lue est `tools/reachability_questions.txt` |
| `inclusion-mutuelle` | `mutual-inclusion` | nom déjà pris par un cas du test |
| `usage-mutuel` | `mutual-usage` | |

Contrôles des noms : aucun ne contient d'espace, de virgule, de `/`, de `|` ni de `:` (le script les met dans `case " $CATEGORIES "`, `case ,$checked,`, `sed "s|^|$cat: |"`, `sed "s/^/$cat\t/"`) ; aucun n'est préfixe d'un autre au sens de ces motifs (délimités par espace ou virgule) ; aucun mot-outil français, aucun ancien nom du TSV d'une ou deux lettres.

### État 1 — chaînes, catégories, données (`refactor:`)

Rejoué par `python3 tmp/4l2_baseline/replay/stage1.py <racine> script|readers` (`s1.patch`, 256 lignes changées, autant de `+` que de `-`, aucune ligne ajoutée ni retirée ; `git apply --check` vert sur `efd9945`). Seules les lignes hors commentaire changent (les commentaires reviennent à l'état 3).

*Script* (63 lignes) — les catégories partout où elles sont du code (listes, cas, `[ "$1" = … ]`), les fichiers de travail qui portent un nom de catégorie (`$work/parent`, `$work/entity-ptrs`, `$work/registries` et ses `.structs`/`.decl`, `$work/mutual-inclusion[.pairs]`, `$work/mutual-usage[.pairs]` : `collect` lit `"$work/$1"`, ils doivent suivre), et :

| Avant | Après |
|---|---|
| `--champs) mode=champs` | `--fields) mode=champs` (la valeur interne change à l'état 2) |
| `${COMPOSITION_ATTEINTE:-…}` | `${COMPOSITION_REACHABILITY:-…}` |
| `--check sans categorie` (×2) | `--check without a category` |
| `categorie inconnue : $cat` | `unknown category: $cat` |
| `categorie vide dans : $checked` | `empty category in: $checked` |
| `option inconnue : $1` | `unknown option: $1` |
| `dossier absent : $dir` | `missing directory: $dir` |
| `aucun .c/.h dans : $*` | `no .c or .h file in: $*` |
| `liste d'atteinte absente : …` | `missing reachability list: …` |
| `liste R2 absente : $R2` | `missing R2 list: $R2` |
| `echec de grep` / `sed` / `awk` | `grep failed` / `sed failed` / `awk failed` |
| `champs : %d ; 0 écriture : %d ; 0 hors constructeur : %d` | `fields: %d; no write: %d; no write outside a constructor: %d` |
| `info\t%s\t%s lignes` | `info\t%s\t%s lines` |
| `résidu gardé ($checked) : $n ligne(s)` | `guarded residue ($checked): $n line(s)` |
| `résidu 2 : $lines ligne(s), $files fichier(s)` | `residue 2: $lines line(s), $files file(s)` |

Le préfixe `composition_residue:` des refus et les codes (0, 1, 2) ne changent pas. `residue 2` garde son `2` (le pied, `R2`, `void-ptr-r2`, `RESIDUE_2_SITES` vont ensemble ; `tools/data_residue.sh` imprime `résidu D` et le traduira à 4l4) : traduction littérale, pas de renommage.

*Liste de sites* (11 lignes) — colonne 1 intouchée (vérifié : `cut -f1` identique) ; colonne 3, imprimée, par la table `sites_col3_table.tsv` : `aucun` → `none`, `est typé`/`typee` → `is typed`, les digrammes par le nom C du TSV (`hv` → `g_live_entities`, `cv` → `g_entities_by_message`, `jo.oe` → `planet_t.slipways`, « prédicat typé de entity_list_filter sur cg » → « typed entity_list_filter predicate over entities_by_tag »), les renvois `(2.11r-b1)`, `(2.20a)`… retirés (aucun document ne les porte : `git grep` vide hors du fichier) ; colonne 2 relue (les trois `targeting_*` sont bien définis dans `src/game/probe.c`) ; colonne 4, que personne ne lit, réécrite en anglais avec les noms C et des appelants nommés par fonction et fichier, sans numéro de ligne (ceux d'avant étaient périmés : `entity.c:217` est `entity_unregister` dans `entity_registry.c`, `cart.c:208-209` est `cart_draw`, `help.c:180` appelle désormais `emitter_init_in`), chaque appelant revérifié par `grep` ; prototype rejoué : `tmp/4l2_baseline/replay/sites.txt` (l'exécutant le relit contre le code et le corrige au besoin ; seule la colonne 3 est contrainte par la table de sortie).

*Lecteurs* : la ligne `COMPOSITION_RESIDUE_GUARD` du `Makefile` (mêmes quinze catégories, même ordre) ; `tests/test_composition_residue.c` (181 lignes : attentes de pied, de `--check`, de `--fields`, de la ligne `info`, de `EMPTY_FOOTER`, `strstr` de `"residue 2: 0"`, arguments de `--check`, `COMPOSITION_REACHABILITY=`). Les coupures `"n" "u"` et les fixtures `"pico8_" "…"` ne bougent pas (D-57).

**Rouge d'abord** (rejoué, `red_script_only.log`) : script et liste de sites seuls, `make -k test` rouge en deux endroits — `composition_residue: unknown category: noyau-inclut` (`composition-guard`, code 2) et `tests/test_composition_residue.c:77` (`test_clean_tree_prints_only_the_empty_footer`, le pied vide). Puis les lecteurs : `make test` vert, `make e2e` vert (26).

**Preuve** (rejouée, écrite telle quelle dans le message) :
- `sh tmp/4l2_baseline/replay/capture.sh <racine> <sortie> [--fields]` relève quinze exécutions sur l'arbre réel : sans option, `--detail`, `--champs`/`--fields`, `--check` de toute la liste du `Makefile`, `--check` de sa première catégorie, `make composition-guard`, `make composition-residue`, et les huit refus (option inconnue, catégorie inconnue, catégorie vide, `--check` sans catégorie, dossier absent, aucun `.c`/`.h`, liste d'atteinte absente, liste R2 absente), chacune avec sa sortie, son erreur et son code. Avant (`before/`), passé par `sed -E -f output.sed`, égal à l'octet à après (`after/`), codes identiques ; 27 lignes différentes avant la table ; aucune ancienne catégorie ni aucun caractère non ASCII dans `after/`. `before/detail.out` et `before/fields.out` sont égaux à l'octet à `tmp/4l1_end/cr_detail.txt` et `cr_champs.txt`.
- Base de `make` (`sh tmp/4l1_baseline/replay/db_proof.sh`) de `HEAD`, passée par la table, égale à l'octet à celle de l'état 1 ; une seule ligne différente avant la table (la variable).
- Vivacité : sur l'arbre réel toutes les catégories sont à 0 ; chaque catégorie sous son nom neuf mord encore sur les fixtures du test (`test_one_occurrence_per_category_in_table_order` et les cas par catégorie, verts).
- `rename-strings-verify SUBSTEP=3l` : 1 en défaut sur 44, 9 `unseen`, comme au départ ; `rename-rekey-check SUBSTEP=3l` vert.
- Compteurs après l'état 1 (rejoués, extraction fraîche, `after_s1_counters/`) : `MODULES=<4l2>` français 0/0/0/611, anti-cart 0/0/0/12, citations 0/0/175 inchangées ; `Makefile` français `string` 7 → 2 (`du`, D-55 ; `par`, 4l4) et `grep -nP '[^\x00-\x7f]' Makefile` vide ; `SUBSTEP=3l` français `done` 2 442 → 2 231 (`string` 517 → 306), anti-cart 36 → 35 (`string` 2 → 1) ; sans `SUBSTEP` français 2 468 → 2 257, anti-cart 36 → 35 ; le reste égal à l'octet.

### État 2 — noms internes du script (`refactor:`)

Le script n'est ni dans l'AST ni au TSV : `rename-slice`/`rename-apply` ne s'y appliquent pas (précédent : `ECART` → `GAP` de `tools/constants_compare.sh` à 4b). Renommage à la main par `tmp/4l2_baseline/replay/stage2.sed`, lignes hors commentaire seulement (32 lignes, `s2.patch`) :

| Avant | Après |
|---|---|
| `SORTIE` | `FOOTER_CATEGORIES` |
| `LIEN_EXCLUSION` | `PARENT_LINK_EXCLUSION` |
| `PTRS_NON_ENTITE` | `ENTITY_PTRS_EXEMPT` |
| `REGISTRES_NON_ENTITE` | `REGISTRIES_EXEMPT` |
| `NOYAU` | `CORE_HEADERS` |
| `ATTEINTE` | `REACHABILITY` |
| `read_registres` | `read_registries` |
| `champs()`, `mode=champs` | `fields()`, `mode=fields` |
| `awk` : `non_entite`, `reg_non_entite`, `out_parente`, `non` | `exempt`, `reg_exempt`, `out_parent`, `exempt` |
| `$work/noyau`, `$work/champs` | `$work/core`, `$work/fields` |

`NU_EXCLUSION`, `VOID_EXCLUSION`, `REG_TYPES`, `R2`, `CLEAN_AWK` et les autres noms (anglais ou abréviations) restent. Les valeurs ne changent pas (`rename-strings-verify` les lit).

**Preuve** (rejouée) : `capture.sh … --fields` après l'état 2 (`after_s2/`) identique à l'octet à `after/`, codes compris ; `sh -n` vert ; `make test` et `make e2e` verts ; `grep -nwE 'SORTIE|LIEN_EXCLUSION|PTRS_NON_ENTITE|REGISTRES_NON_ENTITE|NOYAU|ATTEINTE|read_registres|non_entite|out_parente|non|champs|noyau'` des lignes hors commentaire : vide. Aucun rouge possible (aucun lecteur hors du fichier ; amendement de 4i) : la preuve est l'identité des sorties.

### État 3 — commentaires (`docs:`), note, consigne, registre

Un agent par fichier (le script, le test ; la liste de sites n'a pas de commentaire), ou l'exécutant lui-même comme à 4l1, sous la consigne de l'étape 4 et ces points propres au morceau :
- *Fichier d'avant* : l'état 2 (copies sous `tmp/4l2_before_comments/` avant de commencer).
- *En-tête du script* (destination 4, contrat d'un format lu hors du fichier : le `Makefile` et le test) : usage, modes, formats de chaque ligne et de chaque pied, codes de sortie, tels qu'ils sont **après l'état 1** (`residue 2: …`, `guarded residue (<cats>): …`, `fields: …; no write: …; no write outside a constructor: …`, `--fields`, `[dir...]`). Récit parti au message : « la phase 2 », « Vide (sortie de la phase 2) », « relevé de la règle 1 », « l'étude 2.11 (usegraph.py) » et « ce que usegraph.py ne fait pas » (`usegraph.py` n'est pas dans le dépôt).
- *Motifs qui chassent d'anciens noms* (`pico8_X_of`, `pico8_X_start`, `pico8_X(`, `hp_pose`… ; D-5, D-7, D-8, D-11 en attente) : le commentaire décrit ce que le motif reconnaît, le motif cité tel quel entre backticks s'il le faut ; aucun compteur ne voit `pico8_<nom>` générique. Les noms d'anciens symboles cités en prose (`pico8_ptrs_t` ×5 → `pointer_list_t`, `pico8_nv_ptrs` → `collect_map_pointer_list`, `pico8_gu_content_ptrs` → `collect_contains_string`, `pico8_jo_ex` → `research_display_planet_resource_row`, `pico8_jp_ptrs` → `collect_filter_pointer_list`, `pico8_la_ptrs` → `collect_concat_pointer_list`, `pico8_jp_ents` → `entity_list_filter`, `pico8_ents_cat` → `entity_list_concat`) prennent le nom C.
- *Listes d'exemption* (`ENTITY_PTRS_EXEMPT`, `REGISTRIES_EXEMPT`) : chaque exemption reste prouvée par son écrivain (piège : une exemption sans preuve masquerait un vrai registre), écrite avec le membre C (`economy_t.invented`, `planet_t.needs`, `research_project_t.inputs`, `slipway_t.transit_dots`, `emitter_t.particles`, `planet_inspection_panel_ctx_t.unmet_needs` — nom actuel, D-44 non appliquée) et l'écrivain retrouvé par `grep` (fonction et fichier ; les `fichier:ligne` d'avant — `economy.c:284`, `economy_tiers.c:167`, `research.c:233`, `slipway_display.c:43`, `particle.c:65`, `planet_display.c:54`/`:79`/`:186`, `planet.c:319`, `tech_gate.c:26`, `planet_trade.c:128` — se revérifient tous ; au moins `economy.c:284` ne désigne plus l'écrivain de `invented`).
- *Affirmations à revérifier en traduisant* (consigne, rubrique 7) : le « sed BSD ne connaît pas `\b` » de `NU_EXCLUSION` face au « ce sed-ci veut le sed GNU » de `VOID_EXCLUSION` ; la fixture `"nu"` échappée de `tests/test_data_residue.c` (toujours là, ligne 108) ; « registres lit hors `tests/` et `engine/pointer_list.[ch]` » ; « Non vus » et « limites nommées » de `registries` contre les cas du test qui les fixent ; dans le test, « il ne porte aucun jeton de `tools/data_residue.sh`, que `make test` garde à 0 dans `tests/` » ; les numéros de ligne cités par les commentaires de cas (`:15`, `:16`, `e.h:18`, `f.c:3`, `f.h:19`…) contre les fixtures.
- *Jetons comptés `short`* (consigne, amendements 4a, 4i, 4l1) : `.c`/`.h` entre backticks ou en mots ; « a -> b » des paires de modules → « module A uses module B » ou backticks ; lettres de fixture (`b`, `d`, `e`, `f`, `g`, `u`) entre backticks (`` `d.h` ``) ; digrammes du cart (`ke`, `jo`, `hy`, `jy`, `hp`, `jc`, `ts`, `qy`, `ji`) par le nom C (`entity_root_t`, le type planète de la fixture…).
- *Pièges gardés* (40 mots au plus chacun) : jetons de fixture coupés en deux littéraux (sinon le test entre dans le résidu qu'il mesure) ; une exclusion est un `sed` sur le texte, jamais un filtre de ligne ; mesure sur rien = code 2, jamais un vide ; `entity_state_of` n'est pas une lignée ; `LC_ALL=C` ; le statut brut de `system()` non décodable en C99.
- *Hauteur* : aucune contrainte (ni le script ni le test ne sont dans `tools/line_coverage_reference.txt` ; aucune portée du test n'est au bord : la plus longue fait 12 lignes). Les numéros de ligne cités par le registre se relèvent après l'état 3.
- *Contrôles* : pour le script, `diff <(grep -v '^[[:space:]]*#' AVANT) <(grep -v '^[[:space:]]*#' F)` vide **et** `sh -n F` **et** `capture.sh … --fields` identique à l'octet à `after_s2/` : le `diff` ne voit pas une ligne de commentaire glissée dans un programme `awk` entre apostrophes, où l'apostrophe d'un mot anglais (« entity's ») ferme la chaîne (rejoué, `traps.txt` : `diff` vide, `sh -n` en erreur ligne 250, script en code 2) ; aucune ligne de commentaire ne va dans un programme `awk` (toutes restent en colonne 0 hors des chaînes `'…'`). Pour le test, `gcc -fpreprocessed -dD -E -P` identique à l'octet. Lignes de commentaire relevées avant (129 ; 38) et après.

**Preuve de l'état 3** : les contrôles ci-dessus sur les fichiers finaux ; `make test-all` vert ; extraction fraîche puis compteurs.

### Registre (`docs/decisions-plan-5.md`)

Entrées neuves, au format des conventions, ligne d'index comprise :
- **D-57 — Noms du cart dans les fixtures de `tests/test_composition_residue.c`**. Origine : 4e (« Laissés »), 4l2. Question : les fixtures écrivent des noms de type et de fonction au préfixe et aux digrammes du cart (`pico8_" "jo_t` ×18, `qh_t` ×5, `qh_of` ×5, `nl_t` ×4, `mh_t`, `g__mm`, `reg_items`, `state_fn`…), coupés en deux littéraux : aucun compteur ne les voit. Une partie est exigée par un motif qui chasse ce préfixe (`parent-link`, `defaults`, `void-ptr`, `name-dispatch`, `nu` : D-5, D-7, D-8, D-11) ; le reste est arbitraire. Options : (a) garder tout tant que ces motifs vivent ; (b) renommer les noms arbitraires en noms C (`planet_t`…), garder ceux qu'un motif exige ; (c) suivre D-11 (les fixtures d'une catégorie retirée partent avec elle). Sites : 83 occurrences de `pico8_` dans le fichier. Proposition : (a), puis (c) au verdict de D-11 ; contrôle : `make test` vert. Portée : fixtures seulement.
- **D-58 — Macro `ATT` de `tests/test_composition_residue.c`**. Origine : 4l2. Question : `ATT`, abréviation de « atteinte » (statut `conservé` au TSV validé), nomme la liste d'atteinte de fixture ; après l'état 1, elle est passée à `COMPOSITION_REACHABILITY`. La renommer ? Options : (a) `REACHABILITY_LIST` ; (b) garder. Sites : 7 (définition et six emplois). Proposition : (a), procédure N (identité `macro:tests/test_composition_residue.c:ATT`, ligne `renommé` au TSV et `name` aux surcharges) ; contrôle : `make test` vert, `grep -cw ATT tests/test_composition_residue.c` = 0.

Compléments d'entrées existantes, sans verdict (noms neufs et numéros de ligne relevés après l'état 3) :
- **D-5** : la catégorie garde son nom `void-ptr` ; le motif passe à sa ligne d'après l'état 3 ; fixtures intouchées (D-57).
- **D-6** : `LIEN_EXCLUSION` s'appelle `PARENT_LINK_EXCLUSION` (état 2) ; lignes de définition et d'emploi relevées.
- **D-7** : `lien-parent` s'appelle `parent-link` ; ligne relevée.
- **D-8** : `défauts` → `defaults`, `--champs` → `--fields` ; lignes du motif et du `gsub` de `--fields` relevées.
- **D-10** : `usage-mutuel` → `mutual-usage` ; ligne de l'exclusion `PICO8`/`PICO8_` relevée.
- **D-11** : `aiguillage-nom` → `name-dispatch`, `un-ou-faux` → `one-or-false`, `nu` gardé ; lignes relevées ; les six catégories de `data_residue.sh` intouchées (4l4).
- **D-12** : `--fields` ; pied « no write » 15, « no write outside a constructor » 27 ; rien d'autre ne bouge.
- **D-55** : après 4l2, `french-count MODULES=Makefile` `string` 2 (`du`, `par`) ; 1 avec la ligne de D-55.

### Note « Découvert » de 4l2 (sous l'entrée 4l de `docs/plan-5.md`, après la note de 4l1, au format des notes de 4k1 à 4l1 : « **Découvert** (morceau 4l2, PRD `docs/prd/triage/phase3-etape4l2-composition-residue-commentaires.md`) »)

Elle porte : mesure de départ et de fin (charge 1 005, l'écart à la prévision de 4a) ; la décision sur les catégories et sa table ; le rouge de l'écrivain seul ; la preuve par la table de sortie et la base de `make` ; les noms internes et leur preuve ; les lignes de commentaire avant et après, le piège de l'apostrophe ; les affirmations corrigées ; D-57, D-58 et les compléments ; ce qui reste à 4l3 et 4l4 ; les compteurs, point de départ de 4l3. Et, pour 4l4 : la forme anglaise des pieds (`residue 2: …`, `guarded residue (<cats>): N line(s)`) pour que `tools/data_residue.sh` s'y aligne (`residue D: …`), sans l'imposer.

Amendements de la consigne (`docs/consigne-etape-4.md`), datés « *Amendé par 4l2* » :
- *Point 6* (complément de 4i et 4l1) : un nom de catégorie, d'option ou de variable d'environnement d'un script, lu par le `Makefile` ou un test, est un contrat de ligne de commande : traduit par la sous-étape de l'écrivain, avec tous ses lecteurs, dans un seul commit ; rouge joué par l'écrivain seul ; preuve par la sortie de chaque mode et de chaque refus, passée par une table, égale à l'octet, codes compris, et par la base de `make` égale par la même table.
- *Point 8* (complément de 4a et 4l1) : pour un script qui embarque des programmes `awk` entre apostrophes, la commande des scripts ne voit pas une ligne de commentaire glissée dans un programme (une apostrophe anglaise y ferme la chaîne) : s'y ajoutent `sh -n` et des sorties identiques à l'octet ; aucun commentaire n'entre dans un programme `awk`.
- *Relevés de fin* : depuis 4l2, `--champs` s'appelle `--fields` ; « `--detail` et `--fields` identiques » se compare aux relevés d'avant par `tmp/4l2_baseline/replay/output.sed`.

### Ce qui reste hors de 4l2

- `tools/data_residue.sh`, `DATA_RESIDUE_GUARD`, `tests/test_data_residue.c` (et leur `résidu D`, `résidu gardé`), `tools/reachability_questions.txt` (lu par `reachability-to-prove`, écrit en français) : 4l4.
- `src/cmd/`, `src/platform/`, `tools/frames_ref.sh` et le reste de 4l3.
- Les verdicts de D-5 à D-12, D-44, D-55, D-57, D-58 ; `TROU-N` → `GAP-N` (étape 5) ; la suppression de `docs/consigne-etape-4.md` et la clôture du registre (4l4).

### Fin de PRD

Ordre : `make rename-extract` frais sans `warning:` (4 `partial`) ; `rename-rekey-check SUBSTEP=3l` vert ; compteurs sur `MODULES=<4l2>`, puis `SUBSTEP=3l`, puis `SUBSTEP=3a` à `3j`, puis sans `SUBSTEP` (`sh tmp/4l2_baseline/counters.sh . <sortie>`, `sh tmp/4k3_baseline/others.sh .`).

**Critère de fin** :
- `MODULES=<4l2>` : `french-count`, `cart-names-count` 0, quatre colonnes ; `cart-citations` 0/0/0 ; `cryptic-locals` 0 ; `scope-count` 0 ;
- `SUBSTEP=3l`, prévision : français `done` 2 442 → 1 620 (`string` 517 → 306, `comment` 1 925 → 1 314), `later` 0 ; anti-cart `done` 36 → 23 (`string` 2 → 1, `comment` 34 → 22) ; citations 13/4/513 → 13/4/338 ; `cryptic-locals` 17 ; `scope-count` 0 ;
- `SUBSTEP=3a` à `3j` : égaux à l'octet à `tmp/4l2_baseline/others.txt` ;
- sans `SUBSTEP`, prévision : anti-cart `TOTAL` 36 → 23, citations 13/4/513 → 13/4/338, français 2 468 → 1 646 (`string` 543 → 332, `comment` 1 925 → 1 314), `cryptic-locals` 19, `scope-count` 12 (D-13) ;
- relecture : `grep -nP '[^\x00-\x7f]'` des trois fichiers et du `Makefile` : 0 ; `git grep -nwE 'parente|registres|noyau|atteinte|mutuelle|mutuel|aiguillage|accesseurs|défauts|résidu|gardé|champs|dossier|categorie|echec|aucun'` des trois fichiers et du `Makefile` : 0 ; `grep -n 'pico8\|PICO8'` du script : les seuls motifs et l'exclusion de D-5, D-7, D-8, D-10, D-11 (et leurs mentions entre backticks en commentaire, s'il y en a) ; `cut -f1` de la liste de sites identique à celui d'avant ;
- garde : `make composition-guard` vert, `make composition-residue` `residue 2: 0 line(s), 0 file(s)` ; `--detail` et `--fields` égaux par `output.sed` à `tmp/4l1_end/cr_detail.txt` et `cr_champs.txt`.

**Invariant (§5)** : `make test-all` vert à chacun des trois états, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ; `make gif` et `make float-link` verts ; `rename-tool-test` et les huit `rename-*-test` verts ; `class-coverage-net` et `e2e-gap` rouges, sorties égales à celles de `tmp/4l1_end/` (`class-coverage-net` hors lignes `cc` ; `tmp/line_coverage_gap.txt` égal à l'octet) ; `data-residue` à 0, `--detail` de `data_residue.sh` identique ; `make sin-table` reproduit `quarter_wave.h` à l'octet ; `rename-strings-verify SUBSTEP=3l` : 1 en défaut sur 44, 9 `unseen`.

## Implementation Chunks

Un seul morceau (charge 1 005 ≤ 1 382). L'exécutant produit, dans cet ordre, `make test-all` vert à chaque état :
1. `capture.sh . tmp/4l2_s1/before` sur `HEAD` ; l'état 1 (`stage1.py . script`, rouge relevé dans `tmp/4l2_s1/red.log`, puis `stage1.py . readers`, la colonne 4 de la liste de sites relue) ; `capture.sh . tmp/4l2_s1/after --fields`, égalité par `output.sed` ; base de `make` par la table ; `git diff HEAD > tmp/4l2_stage1.patch` (égal à `tmp/4l2_baseline/replay/s1.patch` si la colonne 4 n'a pas bougé) ; message `tmp/4l2_1_msg.txt` (`refactor: plan 5 morceau 4l2 — catégories, messages et données de la garde de composition en anglais`).
2. l'état 2 (`sed -i -f stage2.sed tools/composition_residue.sh`) ; sorties identiques à `tmp/4l2_s1/after`, `sh -n` ; `git diff <état 1> > tmp/4l2_stage2.patch` (égal à `s2.patch`) ; message `tmp/4l2_2_msg.txt` (`refactor: plan 5 morceau 4l2 — noms internes de la garde de composition en anglais`) ; copies de l'état 2 sous `tmp/4l2_before_comments/`.
3. l'état 3 (agents ou exécutant), ses contrôles, extraction fraîche, compteurs ; note « Découvert », amendements de la consigne, D-57, D-58 et compléments, laissés dans l'arbre de travail ; message `tmp/4l2_3_msg.txt` (`docs: plan 5 morceau 4l2 — commentaires de la garde de composition et de son test en anglais, note Découvert`).

## Remise

L'exécutant ne commite pas. Il laisse :
- `tmp/4l2_stage1.patch` : diff complet de `HEAD` à l'état 1 (s'applique par `git apply` sur `HEAD` propre) ;
- `tmp/4l2_stage2.patch` : diff de l'état 1 à l'état 2 ;
- `tmp/4l2_1_msg.txt`, `tmp/4l2_2_msg.txt`, `tmp/4l2_3_msg.txt` : messages de commit en français, dans l'ordre des commits, récit de chantier (« la phase 2 », « la règle 1 », « l'étude 2.11 », les renvois `(2.11r-b1)`…) et preuves compris, terminés par les lignes d'attribution de la session ; le troisième est celui de l'arbre final ;
- l'arbre de travail à l'état 3 : commentaires du script et du test, note, consigne, registre.

## Testing Decisions

- Aucun test neuf, aucune assertion ajoutée ni retirée, aucune condition changée : les attentes de `tests/test_composition_residue.c` changent avec les sorties qu'elles lisent (propriété des chaînes), dans le commit de l'état 1, après un rouge joué par l'écrivain seul. Le filet : `make test` (le test du script, `composition-guard`, `rename-strings-verify-test`…), `make e2e`, et les quinze exécutions de `capture.sh` sur l'arbre réel.
- Un bon contrôle juge une sortie observable : la sortie, l'erreur et le code de chaque mode et de chaque refus, la base de `make`, les compteurs sur extraction fraîche ; jamais le texte du script.
- La vivacité de chaque catégorie sous son nom neuf est prouvée par les cas existants du test, qui plantent une occurrence par catégorie et la lisent sous ce nom (procédure G, sans motif changé).
- Prior art : 4a (`tools/class_coverage.sh`, rouge d'abord avec lecteurs hors du module, sortie égale par la table), 4b (`tools/constants_compare.sh`, `ECART` → `GAP` et lecteurs dans `tests/test_constants_check.c` et `tools/constants_check.sh`), 4l1 (table de sortie, base de `make`, pièges rejoués), 4k3 (forme de la note).

## Out of Scope

- Toute ligne du script, du test, de la liste de sites ou du `Makefile` hors des littéraux et des noms des états 1 et 2 et des commentaires de l'état 3 : motifs, exclusions, exemptions (valeurs), lecteurs `awk`, ordre des catégories, sémantique de `--detail` et `--fields`.
- Traduire ou retirer un motif, une exemption ou une catégorie (D-5 à D-12) ; renommer `nu`, `void-ptr`, `void-self`, `R2`/`residue 2` ; renommer `ATT` (D-58) ou les fixtures `pico8_` (D-57).
- `tools/data_residue.sh` et tout le reste de 4l3, 4l4 ; les docs, `CLAUDE.md`, les skills (en français, §2) ; les notes « Découvert » déjà écrites, qui citent les anciens noms (historique).

## Further Notes

- La rigueur porte sur l'état 1 : c'est le seul qui touche un contrat. Les états 2 et 3 se prouvent par des sorties identiques à l'octet.
- `--fields` change le nom d'une option que les relevés de fin de toutes les PRD citaient (`--champs`) : la consigne le dit, et `output.sed` rend comparables les relevés d'avant et d'après.
- 4l2 ne clôt pas 4l : la note donne les totaux de départ de 4l3.
