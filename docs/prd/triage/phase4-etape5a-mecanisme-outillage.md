---
title: "Plan 5, étape 5a — mécanisme des gardes (D-66 (c), D-67) construit sans bloquer, puis verdicts d'outillage de la famille O avant les renommages de 5c"
description: "Construire, test d'abord et sans les brancher, les trois cibles de garde que 5e rendra bloquantes — anti-cart texte sans libclang, garde AST (anti-cart des noms ambigus et portée) sur une extraction fraîche, français — puis appliquer les verdicts d'outillage de D-27, D-28, D-36, D-68 (un lecteur du TSV et des déclarations, des règles partagées, un lecteur de triplets, deux sources uniques dans le Makefile, la règle de D-36 (b)), de sorte que 5c renomme avec des outils sans doublon ni lecteur faux, chaque compteur sans SUBSTEP rendant exactement les restes de la « Clôture (4l4) »."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 5. **Clôture.** Condition d'entrée : chaque entrée de `docs/decisions-plan-5.md` porte un verdict humain, pris dans l'ensemble fermé du registre (« Conventions » : `proposition`, `variante nommée`, `garder tel quel`, `reporter hors plan 5`), l'alerte 14 de D-68 comprise. Une PRD par sous-étape, dans l'ordre 5a–5f ; chacune applique les entrées de sa famille (colonne « Application » de l'index du registre) et finit par son contrôle de sortie, avec l'invariant du §5. Une entrée `garder tel quel` ou `reporter hors plan 5` n'édite rien, sauf la ligne d'exception d'un reste compté (5d).
>
> 5a. **Mécanisme des gardes et outillage** (famille O : D-27, D-28, D-36, D-40, D-42, D-66, D-67, D-68 hors alerte 14). Le mécanisme retenu par D-66 est construit, test d'abord, sans encore bloquer ; puis les verdicts d'outillage, avant les renommages de 5c qui passent par ces outils (`rename-apply`, lecteurs du TSV).
>
>    **Sortie** : `rename-tool-test`, les `rename-*-test`, `scope-count-test`, `french-count-test` et `cart-citations-test` verts ; sur une extraction fraîche sans `warning:`, les compteurs sans `SUBSTEP` rendent les restes de la « Clôture (4l4) » du registre, pas un de plus.

Phase 4 du pilotage (tronçon 4 : étape 5). Étape 5a = première PRD du tronçon ; elle renvoie au registre validé sans en recopier les entrées (§ en tête du plan).

Entrées lues : `docs/plan-5.md` en entier (§1–§5, l'entrée 5 et 5a–5f, les notes de l'étape 4 dont « Fin de l'étape 4 » et ses trois puces de consultation) ; `docs/decisions-plan-5.md` (« Conventions », index, colonne « Application », entrées D-27, D-28, D-36, D-40, D-42, D-66, D-67, D-68, et les entrées X que 5a prépare sans les appliquer : D-1, D-13, D-33, D-37, D-60) ; `CLAUDE.md` ; `tools/rename_count.py`, `rename_check.py`, `rename_strings.py`, `rename_substeps.py`, `rename_slice.py`, `rename_rekey.py`, `rename_derive.py`, `rename_strings_verify.py`, `scope_count.py`, `french_count.py`, `cart_citations.py`, `install_ast_rename_bindings.sh`, `tools/ast_rename/` (`plan.py`, `bindings.py`, `__main__.py`) ; les cibles du `Makefile` (légende, `test`, `test-all`, lignes 1202–1335) ; `tests/test_rename_count.py`, `test_scope_count.py`, `test_french_count.py`, `test_rename_strings.py`, `test_rename_substeps.py`, `test_ast_rename.py`.

Aucun humain n'était disponible pour les points « vérifier avec l'utilisateur » de `to-prd` (modules, tests, découpage) : l'agent rédacteur a chargé lui-même le skill `guideline` et l'a appliqué (changements chirurgicaux, critères vérifiables, hypothèses dites plutôt que choisies en silence, rien au-delà des verdicts). Sa règle 5 sur les commentaires cède devant celle du `CLAUDE.md` (trois destinations), comme à l'étape 4.

Mesures faites pour cette PRD, sur l'arbre réel à `39bf814`, écrites sous `tmp/5a_baseline/` seulement (aucune commande git d'écriture ; `make test-all` a réécrit `tools/float_guard.txt` à l'identique, `git status` propre).

## Condition d'entrée

**Constat** : à `39bf814` (`origin/plan-5-noms-anglais`, arbre propre), le commit humain « ok » a rempli **66** lignes `- **Verdict** : ok` — une par entrée D-1 à D-68, sauf D-28 et D-68. Les deux lignes « Verdict (un par alerte) » de D-28 (ligne 460) et de D-68 (ligne 938, « l'alerte 14 d'abord ») sont restées telles que 4l4 et la consultation les avaient laissées, vides.

**Interprétation retenue** (donnée par l'orchestrateur, qui rapporte la session humaine ; la PRD l'adopte sans éditer le registre) :

- « ok » vaut le verdict `proposition` de la convention : la proposition prête de l'entrée, appliquée telle quelle.
- Pour D-28 et D-68, « ok » vaut la proposition de **chaque alerte**, l'alerte 14 comprise (elle relève de 5b, pas de 5a).
- D-66 n'a pas de proposition préférée. En séance, l'humain a accepté la recommandation de l'orchestrateur, **l'option (c)**, après qu'on lui a dit explicitement que « ok/go » valait (c) : la partie texte de la garde anti-cart dans `make test`, sans libclang ; la partie AST (noms d'une ou deux lettres et garde de portée) dans une cible bloquante séparée, chaînée dans `make test-all`. (c) rouvre la lettre du §5 (« dans `make test` ») ; c'est l'humain qui l'a prise.
- Les lignes `Verdict` du registre ne sont pas éditées par 5a. La note « Découvert » de 5a rappelle cette lecture, pour qu'elle soit relue au prochain point d'arrêt.

**Vérifié à `39bf814`** (`tmp/5a_baseline/`) :

- `make -j4 test-all` vert (33 s, `test_all.log`).
- `make rename-extract` frais (141 s sur cette machine ; 98 s dans D-66) : aucune ligne `warning:`, 4 lignes `partial` (`extract.log`).
- Les compteurs sans `SUBSTEP` rendent exactement le tableau « Restes comptés » de la « Clôture (4l4) » : `cart-names-count` `TOTAL` 0/0/1/0 (`tools/frames_ref`, D-1) ; `french-count` 0/0/36/0 ; `cart-citations` 6/2/7 ; `cryptic-locals` 12 ; `scope-count` 12 (`static`) (`cnc.txt`, `french.txt`, `cites.txt`, `cryptic.txt`, `scope.txt`).
- `rename-tool-test` vert (37 tests, 7,5 s) ; `rename-substeps-check` vert ; `rename-rekey-check SUBSTEP=3l` : 0 introuvable ; `rename-strings-verify SUBSTEP=3l` : 1 site en défaut sur 44, 9 `unseen` (code 1, comme à la fin de 4l4).

## Problem Statement

L'étape 5 renomme (5c), pose les lignes d'exception (5d), puis rend les gardes bloquantes (5e). Trois choses manquent pour que ce soit sûr.

**1. Les gardes ne savent pas échouer.** `tools/rename_count.py`, `tools/scope_count.py` et `tools/french_count.py` disent tous « never fails: exit 0 ». La garde anti-cart mélange deux sortes de recherche dans un même passage : par texte, sans libclang, et par l'AST, qui lit `tmp/ast_rename/occurrences.tsv`. Or D-66 (c) les sépare : le texte va dans `make test`, l'AST dans une cible à part. Sans fichier d'occurrences, l'outil compte aujourd'hui le texte seul et imprime un `warning:`, mais rend toujours 0. Il n'existe ni mode « échoue », ni mode « AST seule », ni cible.

**2. L'outillage que 5c va utiliser a des doublons et un lecteur faux.** D-28 et D-68 en dressent la liste, et chaque proposition a reçu « ok » :

- le TSV est lu par quatre fonctions `read_table` ;
- les déclarations sont lues par trois `read_declarations`, et l'en-tête du TSV est relu à la main deux fois ;
- `rename_count` et `rename_substeps` s'importent l'un l'autre ;
- `plan.py` recopie la règle de visibilité de `rename_check.py` et tient une liste de mots-clés plus courte de 15 mots (`bool`, `true`, `typeof`… y passent comme nom neuf) ;
- `scope_count.py` redéfinit `module_of`, importe dans un corps de fonction et exige `--exceptions` sous `--cryptic`, qui ne le lit pas ;
- deux lecteurs de triplets d'exception sont parallèles ;
- `french_count.py` redéclare `COLUMNS` ;
- le `Makefile` écrit le chemin du TSV dans 11 recettes, et la version de libclang est fixée à deux endroits.

S'y ajoutent deux lecteurs faux :

- `CLASS_RECORD` de `tools/rename_strings.py` (D-27) cherche `pico8_class_coverage_record(`, qui n'existe plus depuis 3k ;
- `rename_count.py` compte une mention entre backticks d'un nom ambigu partout, même quand ce nom n'a été renommé que comme locale d'un autre fichier. Les étapes 4b, 4e, 4g et 4h ont dû réécrire des commentaires à cause de ce faux positif (D-36). 5c en créerait d'autres : D-38 renomme 80 statiques `mem`/`gfx`, D-43 des paramètres, D-29 et D-50 des locales.

**3. L'ordre compte.** 5c passe par `rename-apply` (donc `plan.py`) et par les lecteurs du TSV. Son cliquet est le tableau des restes, relu après chaque commit. Corriger ces outils après 5c, ce serait renommer avec des outils faux, et mesurer le cliquet avec une règle qui changerait en cours de route.

## Solution

Neuf morceaux, chacun avec ses commits et `make test-all` vert. L'ordre suit le texte de 5a : d'abord le mécanisme, puis les verdicts d'outillage.

1. **Mécanisme de D-66 (c).** Deux options s'ajoutent à `rename_count.py`. `--part text|ast` choisit une moitié du comptage ; sans l'option, le comptage reste identique à l'octet. `--check` fait rendre 1 s'il reste quelque chose. `scope_count.py` reçoit aussi `--check`. Deux cibles sont créées sans être branchées : `old-names-guard` (texte, sans libclang) et `ast-guard` (extraction fraîche, puis anti-cart AST et portée).
2. **Base de D-67 (a).** `french_count.py --check` et une cible `french-guard`, elle aussi non branchée.
3. **Un module de lecture** (D-28 alerte 1, D-68 alertes 1, 4, 6, 10) : `tools/rename_tables.py`, qui n'importe aucun outil. Il lit le TSV et les déclarations, et trouve la ligne d'en-tête. Tous les autres outils l'importent, ce qui supprime l'import circulaire.
4. **Règles partagées** (D-28 alerte 2, D-68 alertes 3 et 5) : `plan.py` importe les mots-clés et la règle de visibilité de `rename_check.py`, et il ne reste qu'un `module_of`.
5. **Triplets et `--cryptic`** (D-68 alertes 9 et 7) : un seul lecteur de triplets d'exception ; `--exceptions` devient facultatif sous `--cryptic`.
6. **Deux sources uniques dans le `Makefile`** (D-68 alerte 12, D-28 alerte 4) : `RENAME_TABLE` et `CLANG_MAJOR`.
7. **D-27** : `CLASS_RECORD` cherche `class_coverage_record(`.
8. **D-36 (b)** : un nom ambigu dont toutes les lignes `renommé` sont des locales ou des paramètres ne compte en `backtick` que dans le fichier de ces lignes.
9. **Note « Découvert » de 5a** et relevés de fin.

À la fin : les compteurs sans `SUBSTEP` rendent les restes de la « Clôture (4l4) », pas un de plus. Les trois cibles de garde existent et sont rouges exactement de ces restes. Ni `make test` ni `make test-all` ne les appellent : c'est 5e qui les branche, après les exceptions de 5d.

## User Stories

1. En tant qu'exécutant de 5e, je veux une cible `old-names-guard` qui rende 1 tant qu'un ancien nom est trouvé par texte, afin de la mettre dans `make test` sans rien écrire de neuf.
2. En tant qu'humain qui a choisi D-66 (c), je veux que `old-names-guard` passe sans libclang ni bindings, afin que `make test` reste rapide et hors réseau.
3. En tant qu'exécutant de 5e, je veux une cible `ast-guard` qui lance une extraction fraîche, puis la partie AST de l'anti-cart et la garde de portée, et qui rende un code non nul si l'une ou l'autre trouve quelque chose. Je pourrai alors la chaîner dans `make test-all`.
4. En tant qu'humain sans libclang, je veux que `ast-guard` échoue avant tout comptage avec le message de l'outil (`libclang not found`, ou `clang Python bindings not found: run make rename-tool-setup`), pour ne pas prendre un échec d'outillage pour une garde verte.
5. En tant que lecteur d'une garde rouge, je veux le même tableau par module que le compteur, puis le code de sortie, afin de voir quel module est en faute sans relancer.
6. En tant que mainteneur du compteur anti-cart, je veux que les parties texte et AST, additionnées colonne par colonne, rendent exactement le comptage sans `--part`, afin qu'aucune mention ne tombe entre les deux gardes.
7. En tant que mainteneur, je veux que `cart-names-count` sans option rende la même sortie qu'avant, à l'octet, afin que les relevés des étapes 3 et 4 restent comparables.
8. En tant que pilote de 5e, je veux une cible `french-guard` (D-67 (a)) qui rende 1 tant qu'il reste un mot français après exceptions, afin de brancher la troisième garde sans écrire d'outil.
9. En tant que relecteur de 5a, je veux que les trois gardes, lancées sur l'arbre réel, soient rouges exactement des restes de la clôture (D-1 ; D-13 ; D-33, D-55, D-59, D-61), afin de savoir qu'elles verront le vert de 5d et rien d'autre.
10. En tant qu'exécutant de 5c, je veux un seul lecteur du TSV et un seul lecteur des déclarations, afin qu'une correction de format ne soit à faire qu'une fois.
11. En tant que mainteneur, je veux que `rename_substeps` s'importe sans charger `rename_count`, afin de supprimer l'import circulaire.
12. En tant qu'exécutant de 5c, je veux que `rename-apply` refuse un nom neuf `bool`, `true`, `typeof`… comme le fait déjà `rename-table-check`, afin qu'un renommage ne casse pas une compilation C23.
13. En tant que mainteneur de `rename-apply`, je veux que `plan.py` tire sa règle de portée et de visibilité de `rename_check.py`, afin que les deux outils ne divergent jamais sur ce qu'est une collision.
14. En tant que mainteneur des compteurs, je veux un seul `module_of` et un seul lecteur de triplets d'exception, afin que les lignes d'exception de 5d se rangent par module de la même façon dans les trois compteurs.
15. En tant qu'utilisateur de `make cryptic-locals`, je veux que `--cryptic` n'exige plus un `--exceptions` qu'il ne lit pas.
16. En tant que lecteur du `Makefile`, je veux le chemin du TSV dans une seule variable et la version de libclang en un seul endroit, afin qu'un changement ne touche qu'une ligne.
17. En tant que mainteneur de `rename-strings`, je veux que `CLASS_RECORD` apparie l'appel `class_coverage_record(` que le code écrit vraiment (D-27).
18. En tant qu'exécutant de 5c, je veux qu'une mention `` `mem` `` dans l'en-tête d'un autre module ne compte plus quand `mem` n'a été renommé que comme locale ou paramètre (D-36 (b)), afin que les renommages de D-38, D-43, D-29 et D-50 ne fassent pas monter le cliquet par des faux positifs.
19. En tant que pilote, je veux que chaque morceau garde `make test-all` vert et les sorties des outils identiques, sauf ce que son verdict change, afin de relire le tronçon commit par commit.
20. En tant qu'humain au prochain point d'arrêt, je veux que la note « Découvert » de 5a dise comment « ok » a été lu pour D-28, D-66 et D-68, afin de corriger tôt si la lecture est fausse.

## Implementation Decisions

### Ce que chaque entrée de la famille O demande à 5a

| Entrée | Verdict lu | Ce que 5a édite |
|---|---|---|
| D-27 | `proposition` (a) | `CLASS_RECORD` → `\bclass_coverage_record\(\s*$`, et ses fixtures de `tests/test_rename_strings.py` (morceau 7) |
| D-28, alerte 1 | `proposition` | un lecteur du TSV (morceau 3 ; avec D-68 alerte 1, ce lecteur vit dans le module de lecture plutôt que dans `rename_count`) |
| D-28, alerte 2 | `proposition` | `plan.py` importe `C_KEYWORDS` de `rename_check.py` (morceau 4) |
| D-28, alerte 3 | `proposition` | rien en 5a : le schéma du TSV est gardé ; les lignes d'exception sont celles de D-33 (5d) |
| D-28, alerte 4 | `proposition` | `CLANG_MAJOR := 18` dans le `Makefile`, passé au script d'installation (morceau 6) |
| D-36 | `proposition` (b) | règle de `rename_count.py` en `backtick`, test d'abord (morceau 8) |
| D-40 | `proposition` (a) | rien : le contrôle reste humain |
| D-42 | `proposition` (a) | rien : la ligne garde l'`ancien` du cart. Le volet (c) n'est dû que « si la garde de l'étape 5 lit `rename-table-check` » ; sous D-66 (c), aucune des gardes ne le lit |
| D-66 | `variante nommée` (c), lecture de séance | morceau 1 |
| D-67 | `proposition` (a) | morceau 2 (la base de la garde) ; le branchement est fait en 5e |
| D-68, alertes 1, 4, 6, 10 | appliquer en 5a | morceau 3 |
| D-68, alertes 3, 5 | appliquer en 5a | morceau 4 |
| D-68, alertes 7, 9 | appliquer en 5a | morceau 5 |
| D-68, alerte 12 | appliquer en 5a | morceau 6 |
| D-68, alertes 2, 8, 11, 16 | `reporter hors plan 5` | rien |
| D-68, alertes 13, 15 | abandonner (`garder tel quel`) | rien |
| D-68, alerte 14 | famille G | 5b, pas 5a |

### Mécanisme de D-66 (c) (morceau 1)

**Où passe la frontière texte/AST.** C'est celle qui existe déjà dans `rename_count.py`, pas une frontière nouvelle :

- la **partie texte** compte les noms distinctifs, les chemins `tmp/`, et les noms ambigus là où le texte suffit : backtick dont le contenu est exactement le nom, chaîne aux sites que le TSV liste ;
- la **partie AST** compte les identifiants des noms ambigus, c'est-à-dire les occurrences de leurs identités dans `occurrences.tsv`.

Un nom est ambigu s'il a moins de trois caractères, s'il est un mot en minuscules, s'il est gardé par une autre ligne, ou s'il est le nom neuf d'une ligne. À `39bf814` : 440 noms ambigus, dont 313 d'une ou deux lettres, et 1 384 identités. La formule de D-66 (c), « noms d'une ou deux lettres », désigne donc en fait l'ensemble des noms ambigus en identifiant : c'est la seule partie qui demande l'AST. On l'écrit pour que 5e ne cherche pas une frontière à deux lettres qui n'existe pas.

**`rename_count.py`** :

- `--part text` : la partie texte seule. `--occurrences` devient facultatif, n'est pas lu, et aucun `warning:` n'est imprimé.
- `--part ast` : la colonne `identifier` des noms ambigus seule. `--occurrences` est obligatoire.
- Sans `--part` : la sortie actuelle, identique à l'octet, avertissements compris.
- `--check` : même tableau, puis code 1 si `TOTAL` est > 0, sinon 0. Code 2 si `--part ast` (ou sans `--part`) n'a pas d'occurrences, ou des occurrences plus vieilles que les sources ; le message nomme `make rename-extract`.
- `--check` est refusé avec `--substep` (sortie scindée `done`/`later`, sans objet pour une garde).
- Sans `--check`, l'outil rend toujours 0, comme aujourd'hui.

**`scope_count.py`** : `--check` rend 1 si le `total` est > 0, et 2 si les déclarations manquent ou sont plus vieilles que les sources. Il est refusé avec `--cryptic` : la liste des locales cryptiques n'est pas une garde.

**Les docstrings**, qui sont l'aide `--help`, disent le contrat des codes de sortie. « Never fails: exit 0 » devient « exit 0 unless --check ».

**Cibles du `Makefile`**, chacune avec son entrée de légende (en anglais, sans le mot `cart` ni `Lua` : le `Makefile` est compté, D-56) :

- `old-names-guard` : `rename_count.py --part text --check`, sans `$(AST_RENAME_ENV)`.
- `ast-guard` : prérequis `rename-extract`, puis `rename_count.py --part ast --check` et `scope_count.py --check` sur `$(AST_RENAME_DIR)`. Les deux tableaux s'impriment même si le premier échoue (statut accumulé, aucun tube : un `| tee` perdrait le code).
- Ni l'une ni l'autre n'entre dans `test` ou `test-all` en 5a.

**Rouges attendus sur l'arbre réel**, qui sont les restes de la clôture :

- `old-names-guard` : `tools/frames_ref` `string` 1 (D-1), code 1 ;
- `ast-guard` : partie AST 0 ; portée `static` 12 (D-13) ; code 1.

**Contrôles de D-66 (c), relevés en 5a** :

- `make test LIBCLANG=/nonexistent AST_RENAME_BINDINGS=/nonexistent` est vert (aucune cible de `make test` n'utilise `$(AST_RENAME_ENV)`) ;
- `make old-names-guard` avec les mêmes variables rend le même tableau qu'avec libclang ;
- `make ast-guard LIBCLANG=/nonexistent` rend 2 sur `ast_rename: libclang not found: /nonexistent`, avant tout comptage. Ce comportement est relevé à `39bf814` sur `rename-extract` (`tmp/5a_baseline/noclang/`).

Le contrôle de 5e (une violation plantée rougit la vraie cible) reste à 5e.

**Coûts relevés** : la partie texte prend 2,4 s sur l'arbre réel sans bindings (`env -u PYTHONPATH LIBCLANG=/nonexistent`). La partie AST coûte l'extraction (98 à 141 s) plus 2,3 s de comptage. C'est pour 5e : chaîner `ast-guard` porte `make test-all` d'environ 33 s à environ 3 min sur cette machine.

### Base de D-67 (a) (morceau 2)

`french_count.py --check` rend 1 si le `TOTAL` après exceptions est > 0. Il est refusé avec `--substep`. `--sites` reste permis, pour qu'une garde rouge puisse lister ses mentions.

La cible `french-guard` lance `french_count.py --root . --exceptions tools/french_exceptions.txt --check`, sans `--substeps`. Elle n'est pas branchée. Sur l'arbre réel, elle est rouge de `string` 36 (D-33 26, D-55 1, D-59 8, D-61 1), code 1.

La légende du `Makefile` dit pour les trois gardes « fails on any residue; not yet in make test ».

### Module de lecture (morceau 3)

Il s'agit de `tools/rename_tables.py`. Le nom `rename_*` est **obligatoire** : `SKIPPED_FILES` exclut `tools/rename_[a-z_]+.py` et `tests/test_rename_[a-z_]+.py` des compteurs. Un module nommé autrement, qui contiendrait `renommé`, `catégorie` ou `identité`, ajouterait des restes au français et sortirait 5a de son critère.

Le module n'importe aucun outil. Il offre :

- **la lecture d'une table au schéma du TSV** (`docs/renommage.tsv`, `remainder.tsv`) : les lignes `#` de tête (que `rename_check` et `rename_strings` réécrivent), puis les lignes en dictionnaires selon la ligne d'en-tête du fichier (contrat actuel de `rename_count.read_table`), les cellules manquantes valant `""`.
  - `rename_strings` reconstruit ses listes dans l'ordre de `rename_derive.COLUMNS` à l'appel. C'est égal tant que l'en-tête est `COLUMNS`, ce qui est le cas des deux fichiers lus.
  - `rename_slice` passe par ce lecteur ;
- **la position et les colonnes de la ligne d'en-tête** d'une liste de lignes. `rewrite` de `rename_rekey.py` s'en sert et garde chaque autre octet ; il n'y a plus de second analyseur d'en-tête ;
- **la lecture d'un `declarations.tsv`** en dictionnaires par l'en-tête. `scope_count` les prend tels quels ; `rename_derive.Declaration` et `rename_check.Decl` se construisent depuis un dictionnaire plutôt que depuis une liste positionnelle ;
- plus tard (morceau 5), **la lecture des triplets d'exception**.

Les imports suivent :

- `rename_count`, `rename_check`, `rename_strings`, `rename_substeps`, `rename_rekey`, `rename_strings_verify`, `rename_slice`, `rename_derive`, `scope_count` et `cart_citations` importent leurs lecteurs de ce module ;
- `rename_count.read_table` est retiré, sans alias. `tests/test_rename_substeps.py`, qui l'appelait, passe au module ;
- `rename_substeps` n'importe plus `rename_count`. Contrôle : un test lance `python3 -c` dans `tools/`, importe `rename_substeps` et vérifie que `rename_count` n'est pas dans `sys.modules` ;
- dans `scope_count`, les imports passent en tête de fichier (alerte 6) ;
- `french_count.COLUMNS` est importé de `rename_count` (alerte 10).

**Hors du module : `read_table` de `tools/ast_rename/plan.py`.** D-28 le compte parmi les quatre lecteurs, mais il lit un autre format : la table de `rename-apply` (`ancien nouveau fichier [identité]`, trois ou quatre colonnes, en-tête anglais ou français), depuis un texte, avec des numéros de ligne pour ses refus. Il ne lit jamais `docs/renommage.tsv` (la procédure N lui donne `decision.tsv`, `make rename-slice` lui donne `slice.tsv`). Le forcer dans le lecteur du TSV changerait son contrat. Il reste donc tel quel, et la note « Découvert » le signale comme écart à la lettre de D-28, alerte 1.

### Règles partagées (morceau 4)

- **Mots-clés** : `plan.py` importe `C_KEYWORDS` de `rename_check.py` (59 mots au lieu de 44). C'est un changement de comportement voulu : `rename-apply` refuse désormais un nom neuf parmi les 15 mots en plus. Test d'abord dans `tests/test_ast_rename.py` (une entrée dont le nom neuf est `bool` est refusée avec « is not a C identifier »), rouge avant.
- **Visibilité et chevauchement** : `rename_check.py` expose sa règle sous forme de fonctions sur des valeurs simples : l'étendue `(ligne, portée)`, où une portée inconnue court jusqu'à la fin du fichier, et « partagé » = en-tête ou liaison `extern`. `overlap`, `Decl.visibility` et `Decl.lines` s'en servent ; `_scopes_overlap` et `_shared` de `plan.py` sont remplacés par ces fonctions. Le comportement est identique. Il est prouvé par `rename-tool-test` et `rename-check-test`, verts avant et après sans changer un test, et par la sortie de `rename_check.py` égale à l'octet (§ « Preuves »). Le chemin d'import vient de `$(AST_RENAME_ENV)`, qui met `tools/` dans `PYTHONPATH`, et de `tests/test_ast_rename.py`, lancé avec ce même environnement.
- **`module_of`** : `scope_count` utilise `rename_derive.module_of`, et sa copie est retirée. Le contrôle d'alerte 5 est `make scope-count` et `make cryptic-locals` identiques à l'octet avant et après.

### Triplets et `--cryptic` (morceau 5)

- Le module de lecture offre un lecteur de triplets : les lignes non vides hors `#`, coupées sur la tabulation, les trois premières cellules complétées par `""`. Chaque fichier garde son schéma (`colonne module mot`, `sorte parent nom`). `french_count` met le mot en minuscules à l'appel, comme aujourd'hui ; `scope_count` le prend tel quel.
- Ce même lecteur servira à `tools/cart_names_exceptions.txt`, que D-1 crée en 5d. 5a ne crée pas ce fichier et ne fait pas lire d'exceptions à `rename_count.py` : c'est l'application de D-1, famille X.
- `--exceptions` devient facultatif sous `--cryptic`, test d'abord dans `tests/test_scope_count.py` (sous `--cryptic`, sans `--exceptions`, l'outil rend 0 et la liste ; sans `--cryptic`, il rend toujours l'erreur d'argparse). La recette `cryptic-locals` ne le passe plus. Il reste obligatoire pour le compteur de portée.

### Deux sources uniques dans le `Makefile` (morceau 6)

**`RENAME_TABLE := docs/renommage.tsv`** est défini près de `RENAME_SUBSTEPS`. Toutes les recettes l'emploient : les 11 de `39bf814` (19 mentions en tout, dont 8 dans la légende) et celles des cibles de garde ajoutées aux morceaux 1 et 2. La légende garde le chemin écrit en clair. Contrôle : `grep -n 'docs/renommage.tsv' Makefile`, hors lignes `#`, rend la seule définition ; la base de `make` (`make -pn`, prior art de 4l1) est égale avant et après une fois passée par la table de la substitution.

**`CLANG_MAJOR := 18`** :

- `LIBCLANG ?= /usr/lib/llvm-$(CLANG_MAJOR)/lib/libclang-$(CLANG_MAJOR).so.1` ;
- `rename-tool-setup` lance `CLANG_MAJOR=$(CLANG_MAJOR) sh tools/install_ast_rename_bindings.sh` ;
- le script refuse de tourner sans `CLANG_MAJOR` (`${CLANG_MAJOR:?…}`) et installe `clang==$CLANG_MAJOR.*`. Le pin exact `18.1.8` disparaît : `bindings.py` ne vérifie que la majeure, comme le dit le commentaire du script.

**Ce qu'on ne lance jamais** : `rename-tool-setup` (réseau). La preuve passe par `make -n rename-tool-setup`, par `sh -n` du script, et par le refus du script lancé sans la variable, code non nul, avant `pip`.

### D-27 (morceau 7)

Test d'abord : les fixtures de `tests/test_rename_strings.py` qui appellent `pico8_class_coverage_record(` (lignes 172, 212, 262, 263) passent à `class_coverage_record(`. Avec l'ancienne regex, le test est rouge. Ensuite, la regex change.

**Effet mesuré hors du dépôt**, sur une copie du TSV dans `tmp/5a_baseline/d27/`, regex corrigée seule : `rename_strings.py` émet 239 lignes `nom de classe` de plus (une par site d'un nom de classe littéral de `src/`), au nom neuf vide, et 27 marques « nouveau nom indécis » de plus (4 → 31). La cause est que l'ancre `pico8_%s_struct_class` est du même âge que la regex : les tables de classe s'appellent aujourd'hui `g_<x>_class`, et D-21 les renomme en 5c.

La proposition ne touche que la regex. L'outil n'est relancé ni sur le vrai TSV ni par aucune cible de 5a ou de 5c. 5a ne corrige donc pas l'ancre et la signale dans « Découvert ». Rendre `rename-strings` exact sur l'arbre courant n'est demandé par aucune entrée.

### D-36 (b) (morceau 8)

**La règle**, dans `Names` de `rename_count.py` : un nom ambigu dont **toutes** les lignes `renommé`, `catégorie` `identifiant`, ont la `sorte` `locale` ou `paramètre`, ne compte en `backtick` que dans les `fichier` de ces lignes. Les colonnes `identifier` (par l'AST), `string` et `comment` ne changent pas.

**Test d'abord** dans `tests/test_rename_count.py` :

- un nom `mem` renommé seulement comme locale de `a.c` : `` `mem` `` compte dans `a.c`, pas dans `b.h` ;
- le même nom avec, en plus, une ligne `champ` : il compte partout ;
- un nom distinctif n'est pas touché.

**Contrôle.** Le contrôle écrit dans D-36 (« 26 `backtick` de moins ») est périmé, puisque l'étape 4 a réécrit ces mentions : `backtick` vaut 0 sans `SUBSTEP` à `39bf814`. On le remplace par deux relevés :

- sur l'arbre réel, `cart-names-count` est identique à l'octet avant et après (`TOTAL` 0/0/1/0) ;
- sur une copie de `src tests tools Makefile` sous `tmp/5a/d36/`, une mention `` `mem` `` plantée dans un en-tête qui ne déclare pas de locale `mem` compte `backtick` 1 avant la règle et 0 après.

**Ce que D-36 ne demande pas** : D-36 (complément 4f1) observe que la même règle étendue à `short` de `tools/cart_citations.py` aurait évité les « un » comptés, mais ne la propose pas. 5a ne l'étend pas. Risque pour 5c : un renommage de locale d'une ou deux lettres rend comptée chaque mention nue de l'ancien nom. Le cliquet de 5c le verra ; la note le dit.

### Pièges

- **Fixtures de fichiers comptés.** Les tests `tests/test_scope_count.py` et `tests/test_cart_citations.py` sont comptés par le français (D-33 en excepte des triplets, mais une exception retire toutes les mentions de son triplet, et le reste de la clôture se compte avant exceptions). Un fixture neuf n'y écrit aucun mot français littéral nouveau (`renommé`, `catégorie`…) : il réutilise les constructeurs de lignes existants. Contrôle : `french-count MODULES="tests/test_scope_count tests/test_cart_citations tools/scope_count tools/cart_citations tools/french_count Makefile"` égal à l'octet avant et après chaque morceau.
- **Commentaires et légende neufs.** Ils sont en anglais et suivent les trois destinations du `CLAUDE.md`. Ils ne contiennent ni `cart`, ni `Lua`, ni un mot nu d'une ou deux lettres qui soit un ancien nom du TSV (compté `short` par `cart_citations`). Les options et les jetons vont entre backticks, comme à l'étape 4. On dit « old-name guard », jamais « anti-cart ».
- **Code de sortie dans une recette.** Dans `ast-guard`, ni `| tee` ni `&&` ne doivent faire sauter le second compteur ou perdre le premier code.
- **Pas de `rename-derive` sur le vrai TSV** : sa cible écrit `docs/renommage.tsv`. Les preuves lancent `tools/rename_derive.py --output tmp/…`. Il en va de même pour `rename_strings.py`, qui réécrit la `--table` qu'on lui donne : une copie sous `tmp/`.
- **Extraction et mtime.** Une édition de `.c` ou `.h` rend l'extraction plus vieille que les sources, et `--check` rend 2. 5a n'édite aucun `.c` ni `.h` : une seule extraction fraîche suffit en fin de PRD, mais on la refait si un morceau touche du C.

## Implementation Chunks

Chaque morceau est test d'abord, quand il change un comportement. Il se termine par `make test-all` vert, `tests/e2e/*.sha256` inchangés, et la capture du § « Preuves » égale à celle du morceau précédent, sauf ce que son verdict change, différence nommée dans son message de commit.

1. **Mécanisme de D-66 (c)** — `feat:`. Tests rouges d'abord :
   - `tests/test_rename_count.py` : `--part text` sans occurrences rend 0 et pas de `warning:` ; `--part ast` ne compte que les identifiants ambigus ; texte + AST = défaut, colonne par colonne ; `--check` rend 1 sur une violation, 0 sur un arbre propre, 2 sans occurrences sous `--part ast` ; `--check --substep` est refusé ;
   - `tests/test_scope_count.py` : `--check` rend 1, 0, puis 2 sur des déclarations plus vieilles que les sources ; `--check --cryptic` est refusé.

   Ensuite le code, les cibles `old-names-guard` et `ast-guard`, et leur légende. Relevés : les rouges attendus sur l'arbre réel et les trois contrôles sans libclang.
2. **Base de D-67 (a)** — `feat:`. Tests rouges d'abord dans `tests/test_french_count.py` : `--check` rend 1, 0, puis 0 quand une exception retire la seule mention ; `--check --substep` est refusé. Ensuite la cible `french-guard` et sa légende. Relevé : rouge de 36 sur l'arbre réel.
3. **Module de lecture** — `refactor:`. Test neuf `tests/test_rename_tables.py` d'abord (lignes `#` de tête gardées, en-tête du fichier, cellules manquantes, en-tête trouvé pour `rewrite`, déclarations par en-tête, import de `rename_substeps` sans `rename_count`), avec la cible `rename-tables-test` ajoutée à `make test`. Puis les appelants. Capture égale à l'octet.
4. **Règles partagées** — test d'abord pour les mots-clés (`refactor:` pour la règle de visibilité et `module_of`, sorties égales ; `fix:` pour les mots-clés, rouge puis vert dans `rename-tool-test`).
5. **Triplets et `--cryptic`** — `refactor:` pour le lecteur (sorties égales), test d'abord pour `--exceptions` facultatif.
6. **`RENAME_TABLE` et `CLANG_MAJOR`** — `refactor:`. Base de `make` égale par la table de la substitution ; `make -n rename-tool-setup` ; `sh -n` ; refus du script sans la variable.
7. **D-27** — `fix:`. Fixtures rouges d'abord, puis la regex. L'effet sur une copie du TSV est relevé dans le message.
8. **D-36 (b)** — `fix:`. Test rouge d'abord, puis la règle. Arbre réel identique ; violation plantée sur une copie : 1 → 0.
9. **Note « Découvert » de 5a** — `docs:`, sous l'entrée 5a de `docs/plan-5.md`. Elle contient :
   - la lecture de « ok » (D-28, D-66, D-68) ;
   - la frontière texte/AST ;
   - les trois gardes et leurs rouges ;
   - les écarts à la lettre (le `read_table` de `plan.py`, le pin `18.1.8`, le contrôle périmé de D-36) ;
   - l'ancre de D-27 ;
   - la règle de D-36 non étendue à `short` ;
   - le coût de `ast-guard` pour 5e ;
   - les relevés de fin.

   Le registre n'est pas édité.

## Testing Decisions

- **Un bon test** lance l'outil en boîte noire, comme tous les `tests/test_*.py` de `tools/` : un mini-TSV, une mini-extraction, un mini-arbre sous `tmp/test_*_work`, puis la sortie et le code. Il n'inspecte pas l'intérieur des modules, sauf pour l'API de module déjà testée ainsi (`rename_substeps`, `rename_tables`) et pour le contrôle d'import (`sys.modules` dans un sous-processus).
- **Testés** :
  - `rename_count.py` (`--part`, `--check`, D-36) ;
  - `scope_count.py` (`--check`, `--exceptions` facultatif) ;
  - `french_count.py` (`--check`) ;
  - `rename_tables.py` (test neuf) ;
  - `plan.py` (mots-clés, dans `rename-tool-test`) ;
  - `rename_strings.py` (D-27).
- **Prouvés par sorties égales**, sans test neuf : les refactors des morceaux 3 à 6. Ce sont des changements de structure, que les tests existants couvrent déjà, et qui doivent être verts avant et après sans qu'on les retouche, sauf les imports de `rename_count.read_table`.
- **Les cibles du `Makefile`** ne sont pas lancées par un test : aucun test ne lance `make`, prior art de 4l1. On les relève dans la note par leurs sorties et leurs codes, et `make -n` montre leurs recettes.
- **Prior art** : `tests/test_rename_count.py` (`run()`, `Substep`), `tests/test_scope_count.py` (`decl()`, `run()`, test de péremption par mtime), `tests/test_french_count.py`, `tests/test_rename_substeps.py` (API de module), `tests/test_ast_rename.py` (refus de `rename-apply`).

### Preuves (capture avant/après, sous `tmp/5a/`)

La capture est un script qui écrit sortie, erreur et code de chaque commande, sur l'arbre réel, à partir de la même extraction fraîche :

- `make cart-names-count`, `scope-count`, `cryptic-locals`, `cart-citations`, `french-count`, `french-count SITES=1` ;
- les mêmes avec `SUBSTEP=3l` ;
- `rename-substeps-check`, `rename-rekey-check SUBSTEP=3l`, `rename-strings-verify SUBSTEP=3l` ;
- `rename-slice SUBSTEP=3l` (sortie `slice.tsv` comprise) ;
- `rename_derive.py --output tmp/…` et `--remainder tmp/…`, puis `rename_check.py` sur ce reste (code 1, 27 943 violations à `39bf814`) ;
- `rename_strings.py` sur une copie du TSV, table réécrite et marques comprises ;
- à partir du morceau 1, les trois gardes.

Chaque morceau est égal à l'octet au précédent, sauf :

- le morceau 4 : seul le refus de `bool` change, et aucune commande de la capture ne le voit ;
- le morceau 7 : la sortie de `rename_strings.py`, qui change comme mesuré ;
- le morceau 8 : rien ne change sur l'arbre réel.

### Relevés de fin (sortie de 5a et invariant du §5)

- `make test-all` vert ; `tests/e2e/*.sha256` inchangés ;
- `rename-tool-test`, les neuf `rename-*-test` (`rename-tables-test` compris), `scope-count-test`, `french-count-test` et `cart-citations-test` verts ;
- `make raylib` et `make screencast-viewer` compilent et linkent, sans jamais être lancés ;
- `class-coverage-net` et `e2e-gap` : sorties égales à celles de `39bf814` ;
- après `rename-extract` frais sans `warning:` (4 `partial`), les cinq compteurs sans `SUBSTEP` rendent le tableau « Restes comptés » de la clôture à l'identique ;
- `old-names-guard`, `ast-guard` et `french-guard` sont rouges de ces restes seulement, avec les codes attendus.

## Out of Scope

- Brancher une garde dans `test` ou `test-all`, et toucher la recette de `test` ou sa légende : c'est 5e.
- Les fichiers et lignes d'exception : `tools/cart_names_exceptions.txt` (D-1), les lignes de D-33, D-55, D-59, D-61 dans `tools/french_exceptions.txt`, et les règles de D-13, D-37, D-60 dans `scope_count.py`. Tout cela relève de 5d (famille X), même si le lecteur de triplets de 5a les servira.
- L'alerte 14 de D-68 et toute la famille G : 5b.
- Tout renommage du code ou du TSV : 5c. Les lignes `Verdict` du registre.
- D-28 alerte 3, D-40, D-42 : leur verdict n'édite rien.
- D-68 alertes 2, 8, 11, 16 (reportées hors plan 5) et 13, 15 (abandonnées).
- Corriger l'ancre `pico8_%s_struct_class` de `rename_strings.py`, et étendre la règle de D-36 à `cart_citations.py` : aucune proposition ne le demande, la note les signale.
- Lancer `rename-tool-setup` (réseau), `rename-derive` sur le vrai TSV, `e2e-baseline`, ou l'appli raylib.

## Further Notes

- **Ordre des morceaux.** Le texte de 5a met le mécanisme avant les verdicts d'outillage, et l'ordre suit ce texte. Les morceaux 3 à 6 ne dépendent pas des morceaux 1 et 2, mais les gardes, une fois construites, entrent dans la capture et protègent les refactors qui suivent.
- **Pour 5d.**
  - Le lecteur de triplets existe.
  - `rename_count.py` n'a pas encore d'option d'exceptions : D-1 l'ajoute, test d'abord, « une ligne retire exactement son triplet ».
  - Sous `--check`, les exceptions doivent s'appliquer avant le test de `TOTAL`, comme pour le français et la portée.
- **Pour 5e.**
  - `old-names-guard` et `french-guard` vont dans `test` ;
  - `ast-guard` va dans `test-all` ;
  - la légende de `test` et celle de `test-all` changent ;
  - le coût de `ast-guard` est surtout l'extraction (98 à 141 s).
- **Pour 5c.** `rename-apply` refuse désormais les 15 mots-clés de C23. La règle de D-36 (b) vaut pour chaque renommage de locale ou de paramètre de 5c (D-29, D-38, D-43, D-50). Elle ne vaut pas pour `short` de `cart_citations`.
- **Commits** : messages en français, préfixe `<type>: plan 5 étape 5a — …`, avec les lignes d'attribution de session.
