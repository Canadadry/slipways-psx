---
title: "Plan 5, sous-étape 5f — docs et consignes : glossaire supprimé avec ses deux renvois, arborescence et noms neufs dans CLAUDE.md, README, raylib-vendoring et l'exemple travaillé de reprendre-les-commentaires, quatre pièges de D-65 dans sa consigne d'agent, puis relecture humaine des skills"
description: "Après 5e, le code est en noms anglais et gardé, mais les consignes et les docs hors plan ne le disent pas encore : CLAUDE.md cite pico8_struct_header_t et ne décrit pas l'arborescence src/p8, engine, game, platform, devtools, cmd ; docs/glossaire-noms.md, que le §2 du plan condamne, existe encore et deux fichiers y renvoient ; le README, docs/raylib-vendoring.md et l'exemple travaillé du skill reprendre-les-commentaires citent des noms et des chemins disparus ; D-65 attend ses quatre pièges dans la consigne d'agent. Les réécrire, mesure par mesure, en sept commits docs:, sans toucher au code ni à ce que lisent les gardes."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6, à `c97eb65`)

> 5. **Clôture.** Condition d'entrée : chaque entrée de `docs/decisions-plan-5.md` porte un verdict humain, pris dans l'ensemble fermé du registre (« Conventions » : `proposition`, `variante nommée`, `garder tel quel`, `reporter hors plan 5`), l'alerte 14 de D-68 comprise. Une PRD par sous-étape, dans l'ordre 5a–5f ; chacune applique les entrées de sa famille (colonne « Application » de l'index du registre) et finit par son contrôle de sortie, avec l'invariant du §5. Une entrée `garder tel quel` ou `reporter hors plan 5` n'édite rien, sauf la ligne d'exception d'un reste compté (5d).
>
> 5f. **Docs et consignes** (famille D : D-65). `CLAUDE.md` : nouvelle arborescence. `docs/glossaire-noms.md` supprimé ; `README`, les autres docs et les skills qui citent des symboles ou des chemins (dont `reprendre-les-commentaires`, avec D-65) mis à jour vers les nouveaux noms. Citent le glossaire, à `1b4b949` : `docs/vocabulaire.md:5` et `.claude/skills/reprendre-les-commentaires/exemple-travaille.md:25`.
>
>    **Sortie** : `git grep -n glossaire-noms` rend 0 hors du plan et du registre ; le `grep` de chaque ancien nom appliqué à l'étape 5 rend 0 sur `docs/`, `CLAUDE.md`, `README.md` et `.claude/`, hors `docs/renommage.tsv` (pont inverse) et `docs/vocabulaire.md` (dictionnaire), du plan et du registre ; relecture humaine des skills.

Phase 4 du pilotage (tronçon 4 : étape 5). 5f est la sixième et dernière sous-étape de l'entrée 5, après 5a (famille O), 5b (G), 5c (N), 5d (X et T) et 5e (gardes bloquantes). Elle finit le tronçon : sa sortie est le point d'arrêt humain de fin de chantier.

**Entrées lues**, toutes au commit `c97eb65` :
- `CLAUDE.md`, `README.md` ;
- `docs/plan-5.md` : §1 à §5 (dont le §3, arborescence, et le §4, règles de nommage), l'entrée 5, les entrées 5a à 5f et les notes « Découvert » de 0, de 5a (*Lecture de « ok »*, *Trois gardes*), de 5b (*La garde anti-cart ne voit pas les noms historiques*), de 5c (*Décisions prises sans humain*, *Reste à trancher*, *Pièges laissés*) et de 5d (*Décisions prises sans humain*, décisions 9, 10 et 13 ; *Pour 5e*) ;
- `docs/decisions-plan-5.md` : « Conventions » (famille D), l'index, D-65, D-66, D-67, la « Clôture (4l4) » ;
- tout `docs/` : `docs/glossaire-noms.md`, `docs/vocabulaire.md`, `docs/renommage.tsv` (en-tête et statuts), `docs/trous-de-portage.md`, `docs/raylib-vendoring.md`, `docs/psx-hello-world.md`, `docs/design/slipways-mechanics.md` ;
- tout `.claude/` (29 fichiers), dont les quatre fichiers du skill `reprendre-les-commentaires` ;
- le dernier état de la consigne de l'étape 4, source de D-65 : `git show dd1d875:docs/consigne-etape-4.md` ;
- la PRD de 5d (`git show 88efe31`, `git show c97eb65`) et celle de 5e (`docs/prd/next/phase4-etape5e-gardes-bloquantes.md`, lue sans la modifier), pour la forme et pour ce que 5e doit laisser ;
- dans le code, seulement ce qui décide de l'effet des gardes sur 5f : `source_files` de `tools/rename_count.py` (que `french_count.py` et `cart_citations.py` reprennent), `Names` et `is_ambiguous`, `tools/rename_tables.py`, les recettes `test`, `test-all`, `rename-derive` et `rename-strings` du `Makefile`, `read_vocabulary` de `tools/rename_derive.py`, `renamed_tests` de `tools/rename_strings.py`.

**Aucun humain n'était joignable** pendant la rédaction. `to-prd` demande de vérifier avec l'utilisateur les modules, les tests et le découpage : je l'ai fait seul. Chaque décision est écrite au § « Décisions prises sans humain », pour être relue au point d'arrêt de fin de 5f. La PRD est écrite dans `docs/prd/next/`, comme celle de 5e, parce que 5e n'est pas encore implémentée.

**5e n'est pas faite au moment de la rédaction.** Tout ce que cette PRD suppose de son issue est écrit en hypothèse `H-5e-k` (§ « Hypothèses sur 5e »), avec les sections à relire quand la note « Découvert » de 5e existera.

**Relu contre la note « Découvert » de 5e (`f26fb47`)**, et contre le code à `0d90194` sur un clone jetable (mesure de cette PRD rejouée, aucune cible `make` lancée). De `c97eb65` à `0d90194`, 5e a touché `CLAUDE.md` (la seule ligne « Build », ligne 15, hauteur inchangée), le `Makefile` (prérequis de `test` et `test-all`, deux cibles, légende), `docs/plan-5.md` (§5 et sa note, l'entrée 5f passe de la ligne 812 à 823), `tools/rename_substeps.tsv` (ligne 3a) et a ajouté `tests/test_text_guards.py` et `tests/test_ast_guard.py` ; puis elle a retiré sa PRD de `triage/` (`0d90194`). Ni `README.md`, ni `docs/` hors plan, ni `.claude/`, ni `docs/renommage.tsv`, ni aucun `tools/*.py` n'ont bougé.
- **H-5e-1** — tient. Cinq commits et la note ; `make test` contient `old-names-guard`, `french-guard`, `text-guards-test` ; `test-all: test e2e ast-guard ast-guard-test` ; sans libclang ou sans bindings, `make test-all` rend 2 en nommant le manque.
- **H-5e-2** — tient, précisé. La ligne « Build » nomme désormais les gardes (« gardes texte (anciens noms, français) », « garde AST (anciens noms courts, portée) ») et `make rename-tool-setup` ; `CLAUDE.md` garde ses 26 lignes, la ligne 5 (`pico8_struct_header_t`) est intacte : l'inventaire vaut tel quel. La section « Arborescence et noms » renvoie donc à la ligne « Build » plutôt qu'à la légende (corrigé au § « `CLAUDE.md`, section « Arborescence et noms » »).
- **H-5e-3** — tient. Recettes relues : `old-names-guard` et `french-guard` lisent `--root .` par `source_files` (`src/`, `tests/`, `tools/`, `Makefile`), la table par `RENAME_TABLE` et les `tools/*_exceptions.txt` ; `ast-guard` l'extraction et la même table. Les deux tests neufs copient sous `tmp/` le `Makefile`, la table (chemin relu dans le `Makefile`), `tools/*.py`, `tools/*_exceptions.txt`, `tools/rename_substeps.tsv` et `tools/ast_rename/` : aucun fichier de `docs/` hors table, ni `CLAUDE.md`, `README.md`, `.claude/`. Leurs étiquettes accentuées sont écrites `\u00e9` dans le code : rien de 5f n'y touche.
- **H-5e-4** — tient : `make test-all` 149 à 152 s par commit (note de 5e).
- **H-5e-5** — corrigé (un compte). Rien n'est renommé, la table est égale à l'octet, aucun doc ni skill n'est ajouté, les deux tests neufs sont hors du périmètre. `git grep -n glossaire-noms` rend toujours 7 lignes (plan : 16, 799, 808, 823, 825). Mesure rejouée sur les 34 fichiers comptés (les 35 du périmètre moins `docs/design/slipways-mechanics.md`, décision 6) : étape 5, toujours **1** (`exemple-travaille.md:63`) ; élargie, **41** au lieu de 40 : la ligne « Build » réécrite cite `make test e2e`, d'où un second `e2e` à `CLAUDE.md:15`. Les faux positifs passent de 27 à 28 (décision 4 corrigée) ; les 13 vraies occurrences sont les mêmes, aux mêmes lignes.
- **H-5e-6** — tient : aucun `tools/*.py` n'a changé ; `read_table`, `Names` et `is_ambiguous` sont ceux de `c97eb65`.
- **H-5e-7** — tient : seul le §5 du plan a changé ; §3, §4 et `src/` sont intacts.
- **H-5e-8** — tient : registre et vocabulaire non édités, aucune marque « appliqué » (décision 15 de 5e).
- **Fichiers neufs à citer dans les docs** : aucun. `tests/test_text_guards.py` et `tests/test_ast_guard.py` sont décrits par la légende du `Makefile`, et la ligne « Build » dit déjà ce que `make test` et `make test-all` gardent ; la section « Arborescence et noms » ne les nomme pas (décision 8 inchangée).

**Mesures faites pour cette PRD** sur un clone jetable au commit `c97eb65`, dans le scratchpad de l'agent, supprimé ensuite. Aucune commande git d'écriture n'a été lancée sur le dépôt ; aucune cible `make` n'a été lancée (rien de ce qui est mesuré ne demande de build).

- **Fichiers dans le périmètre de la sortie** : `git ls-files docs CLAUDE.md README.md .claude` rend 40 fichiers. Hors des exclusions de la sortie (`docs/renommage.tsv`, `docs/vocabulaire.md`, le plan, le registre) et de `docs/glossaire-noms.md` (supprimé), il en reste 35 : `CLAUDE.md`, `README.md`, quatre docs (`docs/trous-de-portage.md`, `docs/raylib-vendoring.md`, `docs/psx-hello-world.md`, `docs/design/slipways-mechanics.md`) et les 29 fichiers de `.claude/`.
- **`git grep -n glossaire-noms`** : 7 lignes. `docs/plan-5.md` 5 (dans la sortie, admises), `docs/vocabulaire.md:5`, `.claude/skills/reprendre-les-commentaires/exemple-travaille.md:25`. Le registre : 0. Aucun fichier de `src/`, `tests/`, `tools/` ni le `Makefile` ne nomme le glossaire : aucun outil ne le lit.
- **Anciens noms appliqués à l'étape 5** : 153 noms, relevés ainsi (script de mesure jetable, rejoué en 5f, décision 3) :
  - les 126 noms neufs de lignes de `docs/renommage.tsv` qui ont changé entre la fin de 5b (`f439a01`) et la fin de 5c (`4d6571c`), même identité ;
  - les `ancien` des lignes `renommé` dont l'identité n'existait pas à `f439a01` (les paramètres de D-29, D-31, D-32, D-50 réidentifiés par offset : `me`, `qa`, `ra`, `lw`, `sw`, `yr`, `kv`, `nj`, `oc`, `fy`, `gy`, `c_truthy`, `s_str`, et la fixture `grab_arenas_refused.json`) ;
  - les noms des quatre commits de 5c qui ne touchent pas le TSV (D-2, D-3, D-4, D-62 : `PICO8_CLASS_COVERAGE`, `PICO8_MALLOC_FILLS`, `PICO8_HOST_BUILD`, `_pico8_free`, `pico8_half`, `pico8_allocs`… 21 noms). D-18 et D-34/D-53 ne retirent aucun nom ; 5d ne renomme rien (`TROU` → `GAP` a déjà sa sortie, `git grep -n TROU`).
- **Leur grep sur les 34 fichiers comptés** (les 35 moins `docs/design/slipways-mechanics.md`, décision 6), avec la règle de la décision 3 (nom distinctif en mot entier ; nom ambigu comme identifiant dans un passage entre backticks ; lignes « Lua : » de `docs/trous-de-portage.md` et `docs/design/slipways-mechanics.md` hors comptage) : **1 occurrence**, `exemple-travaille.md:63` (`` `assert(oc.count > 0)` ``, `oc` de D-31). Sans la règle, une recherche en mot entier des noms d'une ou deux lettres rend des centaines de faux positifs (`me` de `help-me-debug`, `ra`, `oc` dans les lignes « Lua : »).
- **Élargi aux 3 852 anciens noms `renommé` du TSV** (et aux 153), même règle : 40 occurrences dans 14 fichiers (41 à `0d90194`, voir la relecture contre 5e). Les 13 vraies sont dans quatre fichiers :
  - `CLAUDE.md:5` : `pico8_struct_header_t` (→ `entity_t`) ;
  - `README.md:22` : `oublier`, `dk`, `jc->re`, `fo` ; et `README.md:21`, hors backticks, `js` et `mh` (vus par un second passage sur les digrammes nus) ;
  - `docs/raylib-vendoring.md:41` : `test_pico8_raylib_backend_coords` (avec les chemins des lignes 39 et 40) ;
  - `exemple-travaille.md` : `:26` `pico8_jo_kj`, `jo`, `bv` ; `:44` `pico8_fixed_cos` ; `:63` `oc` ; `:67` `pico8_sub` ; `:69` `pico8_input_save`.

  Les 27 autres (28 à `0d90194`) sont des faux positifs, listés à la décision 4 (`e2e`, `rm`, `origin`, `gh`, `i`, `a`, `id`, `sp`, `l`, `le`, `ligne`, `probe`).
- **Chemins périmés** (tout chemin `src/…`, `tests/…`, `tools/…`, `docs/…`, `.claude/…` et tout nom de fichier `*.c|h|py|sh|txt|json|tsv` des 35 fichiers, cherchés dans l'arbre) : seuls `docs/raylib-vendoring.md:39-41` (`src/main_raylib.c`, `src/pico8_raylib_backend.c`, `src/pico8_raylib_backend_coords.c`, `tests/test_pico8_raylib_backend_coords.c`) relèvent du plan 5. Le reste est généré ou extérieur (`psx-hello/build/…`, `third_party/raylib/…`, `docs/prd/…`, gabarits `<docs/glossaire.md>`), ou historique hors plan 5 (`docs/after-v1.md`, cité comme « historique git » ; `.claude/hooks/deny-absolute-paths.py` de `docs/psx-hello-world.md:43`, récit d'une session passée). Les cibles `make` citées existent toutes.
- **Symboles disparus sans ligne au TSV** (passages entre backticks dont un identifiant à `_` n'apparaît plus dans `src/`, `tests/`, `tools/`, `Makefile`) : dans le périmètre du plan, les mêmes que ci-dessus ; en plus, dans `exemple-travaille.md`, le module `` `ob` `` (`:56`, `:92`), retiré par `536ddd4` (« remove lua layer »), pas renommé.
- **`docs/vocabulaire.md`** : 42 mentions du glossaire, dont 38 renvois « gloss. § X » dans la colonne « source » de ses tableaux ; la phrase qui les explique est la ligne 5. `read_vocabulary` (`tools/rename_derive.py`) et `renamed_tests` (`tools/rename_strings.py`) ne lisent que les lignes de tableau : la ligne 5 n'en est pas une.
- **Ce que lisent les gardes** : `source_files` ne parcourt que `src/`, `tests/`, `tools/` et le `Makefile` ; la table est `docs/renommage.tsv` ; la partie AST lit les unités C. Aucun des fichiers que 5f modifie n'est lu par une garde ni par `make test` (voir § « Effet des gardes »).
- **Champs du jeu** (pour la tâche du `README.md:19`) : 655 lignes `champ` `renommé` et 278 `conservé` ; parmi les conservés d'au plus trois lettres, que des mots anglais (`ids`, `key`, `max`, `pos`, `row`, `run`, `seq`…) et `x`, `y`, l'exception du §4.

## Condition d'entrée

- Les commits de 5e sont poussés sur la branche du chantier et sa note « Découvert » est présente sous l'entrée 5e du plan (H-5e-1).
- **La sortie de 5e tient** : `make test` et `make test-all` verts sur l'arbre de fin de 5e, gardes comprises ; `tests/e2e/*.sha256` inchangés. Une garde rouge à l'entrée arrête 5f avant le premier commit : 5f ne touche aucun fichier qu'une garde lit, elle ne peut pas la réparer.
- **Lecture des verdicts.** Celle que 5a a consignée : « ok » vaut `proposition`. Pour D-65, la proposition est l'option (b) : quatre pièges reportés dans `consigne-agent.md` du skill, en français. Les lignes `Verdict` du registre ne sont pas éditées.
- **Mesure de départ égale à celle de cette PRD** (chunk 0), au détail des fichiers 5e (H-5e-2, H-5e-5).

## Problem Statement

Le code parle anglais depuis 5c, et les gardes branchées par 5e l'y tiennent. Les consignes et les docs, elles, décrivent encore l'arbre d'avant :

- **`CLAUDE.md`**, relu par chaque session, cite `pico8_struct_header_t`, qui n'existe plus, et ne dit rien de l'arborescence `src/p8`, `engine`, `game`, `platform`, `devtools`, `cmd` ni des règles de nommage. Un agent qui ajoute un fichier ou une fonction ne sait ni où la ranger ni comment la nommer ; il l'apprendra par une garde rouge. Et quand le chantier sera fini, le plan, qui porte seul ces règles, sera supprimé (règle « Docs » du `CLAUDE.md`).
- **`docs/glossaire-noms.md`** existe encore : le §2 du plan l'a condamné, `docs/renommage.tsv` est le seul pont inverse. Deux fichiers y renvoient.
- **Le `README`** dit « portage littéral, fonction par fonction », règle que le §2 a retirée ; une de ses tâches est faite (renommer les champs) ; deux autres nomment des objets par leurs digrammes du cart (`oublier`, `dk`, `jc->re`, `fo`, `js`, `mh`), illisibles sans le TSV.
- **`docs/raylib-vendoring.md`** liste quatre fichiers sous leurs chemins d'avant l'étape 1.
- **Le skill `reprendre-les-commentaires`**, que `CLAUDE.md` désigne pour toute reprise de commentaires, raconte son exemple travaillé avec des noms disparus (`pico8_jo_kj`, `pico8_sub`, `oc.count`, le module `ob`) et dit que `CLAUDE.md` porte « quatre destinations », quand il n'en porte plus que trois. Et D-65 attend les quatre pièges que la consigne de l'étape 4 a payés (une ligne de commentaire imprimée, un `#` d'ici-document, un commentaire dans un programme `awk`, un fichier dont un outil compare les numéros de ligne) : sans eux, le prochain rattrapage les repaiera.

Aucune garde ne voit ces fichiers : elles ne lisent que le code. Le travail est donc tenu par une mesure faite une fois, à la sortie, et par la relecture humaine que le plan exige pour les skills.

## Solution

**Une PRD, sept commits `docs:` ordonnés**, plus une mesure de départ sans commit, et un point d'arrêt humain à la fin. Aucun fichier de `src/`, `tests/`, `tools/`, ni le `Makefile`, ni `docs/renommage.tsv`, ni le plan hors de la note, ni le registre ne changent. Chaque commit garde `make test-all` vert et `tests/e2e/*.sha256` inchangés (invariant du §5).

1. **Le glossaire supprimé avec ses deux renvois** : `git rm docs/glossaire-noms.md` ; la ligne 5 de `docs/vocabulaire.md` et le paragraphe de l'exemple travaillé (lignes 23 à 28) réécrits dans le même commit, comme un écrivain avec ses lecteurs.
2. **`CLAUDE.md`** : `pico8_struct_header_t` → `entity_t`, et une section « Arborescence et noms » neuve.
3. **`README.md`** : « Comment c'est fait » dit que le C est la référence ; la tâche des champs, faite, part ; les deux tâches à digrammes nomment leurs objets en noms C.
4. **`docs/raylib-vendoring.md`** : les quatre chemins.
5. **L'exemple travaillé du skill** : les noms restants, en noms C ou par leur rôle.
6. **D-65** : les quatre pièges, dans une section neuve de `consigne-agent.md`.
7. **Note « Découvert » de 5f** sous l'entrée 5f du plan, avec les relevés de fin.

Puis **point d'arrêt humain** : relecture des skills (diff des deux fichiers du skill), et des points signalés au § « Further Notes ». La relecture n'est pas simulée : l'agent s'arrête et la demande.

## User Stories

1. En tant qu'agent qui ouvre une session, je veux que `CLAUDE.md` me dise dans quel répertoire de `src/` vit chaque sorte de code, pour ranger un fichier neuf sans le deviner.
2. En tant qu'agent, je veux que `CLAUDE.md` me donne les règles de nommage en trois lignes (anglais, préfixe, forme des fonctions, noms courts), pour écrire du code qu'aucune garde ne rougira.
3. En tant qu'agent, je veux que `CLAUDE.md` me dise comment écrire un `#include` (`"game/planet.h"`, `-Isrc`), pour ne pas chercher dans le `Makefile`.
4. En tant qu'agent, je veux que `CLAUDE.md` ne cite que des types qui existent (`entity_t`, pas `pico8_struct_header_t`), pour ne pas chercher un symbole disparu.
5. En tant qu'agent qui doit retrouver un nom du cart dans `pico8/lua.lua`, je veux que `CLAUDE.md` me désigne `docs/renommage.tsv` comme pont inverse, pour ne pas chercher un glossaire supprimé.
6. En tant que mainteneur, je veux que `docs/glossaire-noms.md` disparaisse, comme le §2 du plan l'a décidé, pour qu'il n'y ait qu'un pont inverse et qu'il ne diverge pas du code.
7. En tant que lecteur de `docs/vocabulaire.md`, je veux que sa phrase d'introduction dise que les renvois « gloss. § X » visent un glossaire supprimé, retrouvable dans git, pour ne pas chercher un fichier absent.
8. En tant que mainteneur des outils du dictionnaire, je veux que la retouche de `docs/vocabulaire.md` ne change rien de ce que `rename-derive` et `rename-strings` en lisent, pour ne pas changer le TSV qu'ils produiraient.
9. En tant que lecteur du `README`, je veux qu'il dise que le C est la référence et que `pico8/lua.lua` reste en archive, pour ne pas croire la règle du portage littéral encore en vigueur.
10. En tant que lecteur du `README`, je veux que la liste « À faire » ne garde que ce qui reste à faire, pour ne pas reprendre un renommage déjà fait.
11. En tant que lecteur du `README`, je veux que chaque tâche nomme ses objets par leurs noms C (`forget`, `slipway->end`, `targeted_by`, `button`, `message_box`), pour la comprendre sans ouvrir le TSV.
12. En tant que développeur qui pose raylib, je veux que `docs/raylib-vendoring.md` liste les fichiers sous leurs chemins actuels, pour les trouver.
13. En tant qu'orchestrateur d'une reprise de commentaires, je veux que l'exemple travaillé du skill cite des noms qui existent, ou nomme par leur rôle ceux qui ont disparu, pour vérifier l'exemple dans le code.
14. En tant qu'orchestrateur, je veux que l'exemple travaillé dise que ce dépôt est passé de quatre destinations à trois quand ses noms sont devenus parlants, pour comprendre pourquoi `CLAUDE.md` n'a plus de glossaire.
15. En tant qu'agent de reprise de commentaires, je veux savoir qu'une ligne de commentaire qu'un script imprime est une chaîne, pour ne pas changer la sortie du script.
16. En tant qu'agent de reprise, je veux savoir qu'un `#` dans un ici-document n'est pas un commentaire, pour ne pas changer le texte qu'écrit le script.
17. En tant qu'agent de reprise, je veux savoir qu'un commentaire glissé dans un programme `awk` entre apostrophes peut casser le script sans changer les lignes hors commentaire, et comment le voir (`sh -n`, sorties identiques), pour ne pas rendre un script mort qui passe le contrôle.
18. En tant qu'agent de reprise, je veux savoir qu'un fichier dont un outil compare les numéros de ligne garde la hauteur de chaque bloc, pour ne pas décaler une référence de couverture ou un message d'`assert`.
19. En tant que lecteur du skill hors de ce dépôt, je veux que ces quatre pièges soient écrits en termes généraux, avec ce dépôt pour seul exemple, pour qu'ils servent ailleurs (D-65 : « le skill sert aussi hors de ce chantier »).
20. En tant qu'humain au point d'arrêt, je veux relire le diff des skills, et lui seul, pour juger la consigne que les agents suivront après le chantier.
21. En tant qu'humain, je veux une mesure rejouable qui dise, fichier par fichier, ce qui reste d'anciens noms dans les docs et les skills, et pourquoi ce qui reste est admis, pour ne pas devoir relire 35 fichiers.
22. En tant que mainteneur, je veux savoir que les gardes ne voient pas les docs, pour ne pas croire qu'un ancien nom réintroduit dans `CLAUDE.md` serait rattrapé.
23. En tant que mainteneur, je veux qu'aucun commit de 5f ne touche un fichier qu'une garde lit, pour que 5f ne puisse ni rougir ni masquer une garde.
24. En tant qu'agent, je veux que l'appli raylib ne soit jamais lancée : `make raylib` et `make screencast-viewer` sont compilés et linkés, jamais exécutés.

## Implementation Decisions

### Inventaire exact des réécritures (mesuré à `c97eb65`)

| Fichier | Lignes | Ce qui change | Commit |
|---|---|---|---|
| `docs/glossaire-noms.md` | tout | supprimé | 1 |
| `docs/vocabulaire.md` | 5 | « « gloss. § X » renvoie à une section de `docs/glossaire-noms.md` » devient un renvoi à l'ancien glossaire des noms, supprimé par la sous-étape 5f du plan 5 et retrouvable dans git, sans le nom du fichier | 1 |
| `exemple-travaille.md` | 23–28 | les artefacts du chantier : la règle « des destinations, quatre à l'époque, trois depuis que les noms sont devenus parlants » ; le glossaire décrit au passé (191 symboles), remplacé par un plan de renommage et une table `ancien → nouveau` ; plus de digramme cité (`jo`, `bv`, `cg`, `pico8_jo_kj`) | 1 |
| `CLAUDE.md` | 5 | `pico8_struct_header_t` → `entity_t` | 2 |
| `CLAUDE.md` | neuve, après « Le portage » | section « Arborescence et noms » (ci-dessous) | 2 |
| `README.md` | 7 | « Portage littéral, fonction par fonction, de `pico8/lua.lua` » → « Porté depuis `pico8/lua.lua` (la source décompilée du cart), qui reste en archive : le C est désormais la référence » ; le renvoi à `docs/trous-de-portage.md` reste | 3 |
| `README.md` | 19 | tâche « Renommer les champs… hérité du cart » retirée (faite, décision 7) | 3 |
| `README.md` | 21 | « js et mh n'ont aucune marge » → les places des boutons (`button`) et des boîtes de message (`message_box`) | 3 |
| `README.md` | 22 | `oublier` → `forget` (de `src/game/cart.c`) ; `dk`, `jc->re`, `fo` → la difficulté (`economy_score_difficulty_quits`), `slipway->end`, `targeted_by` | 3 |
| `docs/raylib-vendoring.md` | 39–41 | `src/cmd/game.c`, `src/platform/raylib_backend.c`, `src/platform/raylib_backend_coords.c`, `tests/test_raylib_backend_coords.c` | 4 |
| `exemple-travaille.md` | 44 | `pico8_fixed_cos/_sin` → `p8_fixed_cos/_sin` | 5 |
| `exemple-travaille.md` | 56, 92 | le module `ob` → « le parseur de specs » (module retiré avec la couche Lua, `536ddd4`) | 5 |
| `exemple-travaille.md` | 63 | `` `assert(oc.count > 0)` `` → `` `assert(state->tickets_count > 0)` `` (D-31, `src/game/gen.h`) | 5 |
| `exemple-travaille.md` | 67, 69 | `pico8_sub` → `p8_sub`, `pico8_input_save` → `p8_input_save` ; `unstash()` reste : c'est la fonction Lua du cart, et le texte la cite comme telle | 5 |
| `consigne-agent.md` | neuve, avant « Contrôle avant de rendre la main » | D-65 (ci-dessous) | 6 |
| `docs/plan-5.md` | sous l'entrée 5f | note « Découvert » | 7 |

Les nombres racontés par l'exemple travaillé (lignes de commentaire, ratios, 191 symboles, 9 729 frames) ne changent pas : c'est l'histoire du chantier de référence. Les noms neufs choisis ne sont l'ancien nom d'aucune ligne `renommé` du TSV (vérifié : `forget`, `end`, `targeted_by`, `difficulty`, `button`, `message_box`, `entity_t`, `tickets_count`, `state`, `p8_sub`, `p8_input_save`, `p8_fixed_cos`, `p8_fixed_sin`, `economy_score_difficulty_quits`).

### `CLAUDE.md`, section « Arborescence et noms »

Courte : `CLAUDE.md` est relu à chaque session. Elle dit, en français :
- `src/` en six répertoires : `p8/` le runtime PICO-8 émulé (symboles en `p8_`, fichiers sans préfixe) ; `engine/` le socle objet et mémoire, sans planète ni slipway (`entity_t` et les classes dans `entity_class.h`, registres, listes, zones, file de sortie) ; `game/` le jeu ; `platform/` le backend raylib ; `devtools/` la vérification sans fenêtre (dump, rejeu, gif, screencast, couverture de classes) ; `cmd/` un fichier par binaire. Les includes s'écrivent depuis `src/` (`#include "game/planet.h"`, `-Isrc` seul).
- `tests/` à plat, `test_<module>.c`, les rejeux dans `tests/e2e/` ; `tools/` les scripts de contrôle et les gardes.
- Les noms : anglais et complets, aucun préfixe pour le jeu, `p8_` pour le seul runtime ; une fonction exportée s'écrit `module_verbe[_objet]` ; une ou deux lettres seulement pour une locale ou un paramètre dont la portée tient en 40 lignes. Les gardes de `make test` et de `make test-all` le vérifient (voir « Build »). `docs/renommage.tsv` mène d'un nom C au nom du cart.

Elle ne répète ni la liste des fichiers (le §3 du plan la dit « indicative ») ni la table du §4 ; elle ne nomme pas les cibles des gardes : la ligne « Build », réécrite par 5e, dit déjà où vit chacune (H-5e-2). Cette ligne ne change pas en 5f.

### D-65, la section neuve de `consigne-agent.md`

Titre : « Quand un commentaire n'en est pas tout à fait un ». Quatre puces, en français, à la deuxième personne comme le reste de la consigne, en termes généraux, chacune avec son contrôle :
1. une ligne de commentaire qu'un script imprime (son aide tirée de l'en-tête, `sed -n '2p' "$0"`) est une chaîne : elle se traite comme un message, se prouve par la sortie du script, et ne change pas de numéro de ligne ;
2. un `#` dans un ici-document n'est pas un commentaire, c'est du texte que le script écrit : on le laisse ou on le signale ;
3. un programme `awk` (ou tout programme) entre apostrophes dans un script shell : un commentaire glissé dedans peut le casser — une apostrophe dans le texte ferme la chaîne — sans changer une ligne hors commentaire ; aucun commentaire n'y entre, et le contrôle ajoute `sh -n` et des sorties identiques à l'octet ;
4. un fichier dont un outil compare les numéros de ligne (référence de couverture, message d'`assert` qui imprime sa ligne) garde la hauteur de chaque bloc de commentaire ; contrôle : les lignes qui ne sont pas des commentaires restent aux mêmes numéros.

La source est la consigne de l'étape 4 (`git show dd1d875:docs/consigne-etape-4.md`, amendements de 4l1, 4l2, 4l3, 4b et 4d) ; la section n'y renvoie pas : la consigne est morte avec l'étape. Les autres leçons de cette consigne (compteurs, `SUBSTEP`, TSV, `e2e-gap`) tiennent à ce chantier et ne sont pas reportées (D-65 : « les pièges qui ne tiennent pas à ce chantier »).

### Règle de mesure de la sortie

Le plan dit « le `grep` de chaque ancien nom appliqué à l'étape 5 ». Un `grep -w` nu des noms de deux lettres ne veut rien dire sur de la prose française (`me`, `ra`, `oc`, `gy`…). La mesure suit donc la frontière de la garde anti-cart (`tools/rename_count.py`, décision 3) :
- **nom distinctif** (`Names.distinctive_words` et les chemins de `Names.paths`) : en mot entier, n'importe où ;
- **nom ambigu** (`is_ambiguous`) : comme identifiant dans un passage entre backticks, ni collé à `-`, `/`, `.`, ni suivi d'une extension de fichier ;
- **fichiers** : `git ls-files docs CLAUDE.md README.md .claude`, moins `docs/renommage.tsv`, `docs/vocabulaire.md`, `docs/plan-5.md`, `docs/decisions-plan-5.md`, `docs/prd/` (décision 5) et `docs/design/slipways-mechanics.md` (décision 6) ; dans `docs/trous-de-portage.md`, les paragraphes qui commencent par « Lua : » (décision 6).

Deux ensembles de noms : les 153 de l'étape 5 (sortie du plan : **0**), et tous les `ancien` des lignes `renommé` (sortie élargie : **exactement la liste des faux positifs de la décision 4**). Plus un passage sur les digrammes nus (deux minuscules, ancien nom ambigu, hors mots courants français et anglais) qui a trouvé `js` et `mh` : relu à la main, sans valeur cible chiffrée.

Le script vit dans `tmp/5f/` et n'est pas commité, comme ceux de 5b et 5e : la mesure est faite une fois, ce n'est pas une garde (décision 9).

### Effet des gardes (5e) sur 5f

- **Ce qu'elles lisent** : `old-names-guard` et `french-guard` passent par `source_files` (`src/`, `tests/`, `tools/`, `Makefile`), plus la table `docs/renommage.tsv` et leurs fichiers d'exceptions de `tools/` ; `ast-guard` lit les unités C par libclang, et la même table. `text-guards-test` et `ast-guard-test` (5e) travaillent sur des copies sous `tmp/` faites de `Makefile`, `tools/` et la table.
- **Conséquence** : aucun fichier que 5f modifie ou supprime n'est lu par une garde. Un commit de 5f ne peut ni rougir ni verdir une garde ; `make test-all` y reste vert par construction, et l'invariant du §5 le constate (H-5e-3).
- **Le revers** : les gardes ne voient pas les docs ni les skills. Un `pico8_` réintroduit dans `CLAUDE.md` ou dans un skill ne rougit rien. 5f n'ajoute pas de garde sur les docs (décision 9) : le plan, le registre, `docs/vocabulaire.md` et la table elle-même sont pleins d'anciens noms, et une garde devrait les exclure.
- **`docs/vocabulaire.md`** est lu par `rename-derive` et `rename-strings`, hors suite. La retouche de sa ligne 5 doit laisser leur lecture égale : contrôle au commit 1 (décision 2).
- **Les fichiers d'exceptions et le `Makefile` ne bougent pas** : `cart-citations` et `french-count`, qui lisent la légende du `Makefile`, ne sont pas concernés.

### Décisions prises sans humain

1. **Le glossaire part dans le commit qui réécrit ses renvois** : supprimer d'abord laisserait deux renvois morts ; réécrire d'abord, deux renvois vers un fichier condamné. Comme 5d, écrivain et lecteurs ensemble.
2. **`docs/vocabulaire.md` : la ligne 5 seule.** Les 38 renvois « gloss. § X » de ses tableaux restent : ils sont dans la colonne « source », que les outils ne lisent pas, et ils disent d'où venait chaque sens à l'étape 0 ; la ligne 5 réécrite dit où les retrouver (git). Le vocabulaire est un dictionnaire, exclu de la sortie ; ses chemins d'avant l'étape 1 (`src/pico8_ke.h`…) restent pour la même raison. La phrase neuve ne contient pas `glossaire-noms` (sinon la sortie `git grep` tombe). Contrôle : sur l'avant et l'après, `read_vocabulary` (dans un `Lexicon` neuf) rend des attributs égaux, `renamed_tests` le même dictionnaire, et `rename-derive-test`, `rename-strings-test` restent verts. La décision 13 de 5d (« vocabulaire non édité ») visait ses lignes de concept ; la ligne 5 est l'unique exception, exigée par la sortie du plan.
3. **Règle de mesure** (§ « Règle de mesure de la sortie ») : la frontière distinctif/ambigu de la garde anti-cart, pas un `grep -w` nu. L'ensemble « appliqué à l'étape 5 » est relevé par différence du TSV entre `f439a01` et `4d6571c`, identité par identité, plus les quatre commits de 5c hors TSV ; 5d et 5e ne renomment rien.
4. **Faux positifs admis de la sortie élargie**, chacun pour sa raison, et eux seuls : `e2e` (nom de cible et de répertoire, `check-e2e`, `prd-autopilot`, `CLAUDE.md:15`) ; `rm`, `origin`, `gh` (commandes git et `gh`) ; `i` et `a` (exemples de code génériques de `tdd` et `i >= 1` de l'exemple travaillé, une locale d'aujourd'hui) ; `id` (exemple JavaScript de `tdd/mocking.md`) ; `sp` (registre MIPS de `docs/psx-hello-world.md`) ; `l`, `le` (options de link `-framework` de `docs/raylib-vendoring.md:34`) ; `ligne` (gabarit `<fichier:ligne>` de `modele-rapport.md`) ; `probe` (le nom C de la sonde, `docs/trous-de-portage.md:116`). À `0d90194`, ils font 28 occurrences (27 à `c97eb65`) : `e2e` 12 (`check-e2e` 8, `prd-autopilot` 2, `CLAUDE.md:15` 2, dont `make test e2e` ajouté par 5e), `rm` 2, `origin` 2, `gh` 1, `i` 3, `a` 1, `id` 1, `sp` 2, `l` 1, `le` 1, `ligne` 1, `probe` 1. La sortie élargie ne doit plus rendre que ces noms, dans ces fichiers ; leur compte est relevé au chunk 0 (égal à ce relevé si l'arbre de départ est `0d90194`) et ne bouge plus après, sauf un `e2e` neuf que la section « Arborescence et noms » écrirait, à dire dans la note.
5. **`docs/prd/` est hors des deux sorties** (`git grep` et anciens noms), comme pour `TROU` en 5d (décision 9) : cette PRD cite le glossaire et les anciens noms qu'elle fait disparaître, et elle est graduée après la note. La sortie du plan est écrite « hors du plan et du registre » ; la note le dit.
6. **`docs/design/slipways-mechanics.md` ne change pas.** Il décrit le cart (mécaniques et valeurs lues dans `lua.lua` et `map.json`, avec numéros de ligne) : ses 130 noms entre backticks sont des noms Lua de l'archive, au même titre que les lignes « Lua : » de `docs/trous-de-portage.md` que 5d a voulues. Le réécrire en noms C serait un autre document. Hors comptage, comme les lignes « Lua : ».
7. **La tâche « Renommer les champs… » du `README` est retirée**, pas réécrite : le plan 5 l'a faite (655 champs renommés ; aucun champ conservé n'est un digramme, hors `x` et `y`, exception du §4), et la garde de portée l'interdit désormais. `docs/` et le `README` ne disent que ce qui est vrai ou à faire.
8. **`CLAUDE.md` gagne les règles de nommage en trois lignes**, au-delà de la lettre (« nouvelle arborescence ») : le §4 du plan est la seule source de ces règles, et le plan sera supprimé une fois le chantier fini. Sans elles, un agent n'apprend la règle que par une garde rouge. La section ne recopie pas le tableau du §4.
9. **Pas de garde sur les docs.** La mesure de sortie est jetable ; une garde demanderait un outil neuf (famille O, close) et une liste d'exclusions qui contiendrait le plan, le registre, le vocabulaire et le TSV. Noté pour l'humain.
10. **L'exemple travaillé garde son histoire.** Les noms disparus y sont remplacés par leur nom C quand le TSV en donne un, sinon nommés par leur rôle (`ob`) ; les noms Lua du cart cités comme tels (`unstash()`) restent. L'affirmation sur `CLAUDE.md` (« quatre destinations ») est corrigée, parce qu'elle décrit ce dépôt au présent.
11. **`SKILL.md` du skill ne change pas.** Il est général : son glossaire est facultatif (« si les noms sont déjà parlants, ce fichier n'a pas lieu d'être »), et il traite le dépôt sans glossaire (« un glossaire mais pas de fichier d'écarts, ou l'inverse »). Sa ligne 23, « la règle des quatre destinations est déjà dans le fichier d'instructions du dépôt », est inexacte pour ce dépôt-ci (trois) : signalée à la relecture humaine, pas réécrite. `modele-rapport.md` (rubrique « glossaire » du rapport) non plus, pour la même raison.
12. **Les autres skills ne changent pas** : aucun ne cite un ancien nom ni un chemin périmé (mesuré). `fix-bug` (« a divergence from the cart… `/* GAP-n */` ») est juste.
13. **`docs/psx-hello-world.md` ne change pas** : il ne cite aucun nom du plan 5. Il est écrit en anglais, contre « les docs restent en français » (§2), et cite un hook disparu : hors famille D, noté pour l'humain.
14. **Un commit par fichier ou par motif**, sept en tout : chaque diff se relit seul, et le diff des skills (commits 1, 5, 6) se donne tel quel à l'humain.
15. **L'invariant du §5 est tenu à chaque commit**, malgré la règle du skill `git-commit` (« si seuls des `.md` changent, pas de test ») : le plan est plus précis. Sous 5e, `make test-all` dure environ 2 min 30 (H-5e-4).
16. **Le registre, `docs/renommage.tsv` et le plan hors de la note ne sont pas édités.** Pas de marque « appliqué » sur D-65.
17. **Le point d'arrêt n'est pas simulé.** La note dit « relecture humaine des skills : demandée » ; ni l'agent ni `plan-pilot` ne l'écrivent faite.

## Implementation Chunks

0. **Mesure de départ**, sans commit, dans `tmp/5f_baseline/`, sur l'arbre de fin de 5e :
   - `make test` et `make test-all` verts ;
   - `git grep -n glossaire-noms` : les 7 lignes de cette PRD, plus celles que 5e aurait ajoutées hors de `docs/prd/` (H-5e-5) ;
   - le script de mesure (`tmp/5f/measure.py`), sur les deux ensembles : 1 occurrence (étape 5), 41 (élargie, depuis 5e), aux mêmes lignes ;
   - le passage des chemins périmés : `docs/raylib-vendoring.md:39-41` et les chemins admis ;
   - `make raylib`, `make screencast-viewer` (compilés et linkés, jamais lancés), `class-coverage-net`, `e2e-gap` : sorties gardées.

   Un écart arrête 5f et se relit contre les hypothèses.
1. **`docs:`** — `docs/glossaire-noms.md` supprimé ; `docs/vocabulaire.md:5` et `exemple-travaille.md:23-28` réécrits. Contrôles : `git grep -n glossaire-noms` hors du plan et de `docs/prd/` rend 0 ; contrôle de lecture de la décision 2.
2. **`docs:`** — `CLAUDE.md` : `entity_t` et section « Arborescence et noms ». Contrôle : chaque répertoire et chaque chemin cité existe ; la ligne « Build » est égale à celle de fin de 5e.
3. **`docs:`** — `README.md` : lignes 7, 19, 21, 22.
4. **`docs:`** — `docs/raylib-vendoring.md:39-41`. Contrôle : les quatre chemins existent, et ce sont ceux de la recette `raylib` et de `test_raylib_backend_coords`.
5. **`docs:`** — `exemple-travaille.md` : lignes 44, 56, 63, 67, 69, 92. Contrôle : chaque nom C cité existe (`git grep -w`).
6. **`docs:`** — D-65, section neuve de `consigne-agent.md`.
7. **`docs:`** — note « Découvert » de 5f sous l'entrée 5f du plan. Elle contient :
   - les sept commits ;
   - les décisions 1 à 17 ;
   - les deux mesures de sortie, avant et après ;
   - ce qui reste pour l'humain (§ « Further Notes ») ;
   - tout écart à une hypothèse `H-5e-k` ;
   - les relevés de fin, et « relecture humaine des skills : demandée ».

**Total : 7 commits**, puis le point d'arrêt.

## Testing Decisions

- **Rien à tester par TDD** : 5f ne change aucun code. Les contrôles sont des mesures sur l'arbre, avant et après, et l'invariant du §5.
- **Une bonne mesure juge l'arbre, pas le diff** : elle relit les 34 fichiers comptés à chaque passage, pour qu'une mention oubliée hors des lignes inventoriées se voie.
- **Sorties chiffrées, pas « à peu près »** : après le commit 6, l'ensemble de l'étape 5 rend 0 ; l'ensemble élargi ne rend que les faux positifs de la décision 4, au compte du chunk 0. Une occurrence de plus ou de moins se lit.
- **Prior art** : la sortie `git grep -n TROU` de 5d (exclusions nommées, `docs/prd/` comprise) ; les scripts jetables de 5b (`tmp/5b_mutation/`) et de 5e (`tmp/5e_mutation/`), non commités.
- **Contrôle de non-régression des outils** : `rename-derive-test` et `rename-strings-test` verts, et la lecture du vocabulaire égale avant et après (décision 2). Ce sont les seuls outils qui lisent un fichier de `docs/` que 5f touche.

**Relevés de fin** (sortie de 5f et invariant du §5) :
- `make test-all` vert à chaque commit, `tests/e2e/*.sha256` inchangés ;
- `git grep -n glossaire-noms` : 0 hors de `docs/plan-5.md`, de `docs/decisions-plan-5.md` et de `docs/prd/` ;
- mesure de l'étape 5 : 0 ; mesure élargie : les seuls faux positifs de la décision 4, au compte du chunk 0, aux mêmes fichiers ;
- chemins périmés du plan 5 : 0 ;
- `make raylib` et `make screencast-viewer` compilés et linkés, jamais lancés ; `class-coverage-net` et `e2e-gap` : sorties égales au chunk 0 ;
- aucun fichier de `src/`, `tests/`, `tools/`, ni le `Makefile`, ni `docs/renommage.tsv` dans `git diff --stat` de la PRD.

## Out of Scope

- **Le code**, et tout ce qu'une garde lit : `src/`, `tests/`, `tools/`, le `Makefile`, `docs/renommage.tsv`, les fichiers d'exceptions. Les docstrings de `tools/` qui citent `docs/plan-5.md` (12 fichiers) restent jusqu'à la suppression du plan.
- **Supprimer le plan, le registre, `docs/vocabulaire.md` ou les outils `rename_*`** à la fin du chantier : décision humaine, après la relecture des notes « à relire au prochain point d'arrêt » qui vivent dans le plan.
- **Réécrire `docs/design/slipways-mechanics.md`** en noms C (décision 6), ou les renvois « gloss. § X » du vocabulaire (décision 2).
- **Traduire `docs/psx-hello-world.md`**, ou corriger son récit du hook disparu (décision 13).
- **Une garde sur les docs** (décision 9).
- **Les questions laissées par 5b à 5e** : noms historiques sans ligne au TSV, `static const g_…` hors classes, paramètres courts hors famille de D-29, entrées du registre des écarts signalées par 5d (GAP-3, 4, 9, 11, 23). Elles sont dans les notes et vont à l'humain.
- **La ligne « Build » du `CLAUDE.md`** : c'est 5e.
- Éditer le registre ou une ligne `Verdict`, lancer `rename-derive`, `rename-strings`, `rename-tool-setup`, `e2e-baseline` ou l'appli raylib.

## Hypothèses sur 5e

Je les ai écrites à `c97eb65`, 5e non faite, d'après sa PRD (`docs/prd/next/phase4-etape5e-gardes-bloquantes.md`). Chacune se vérifie sur la note « Découvert » de 5e et sur l'arbre de sa fin. Une hypothèse fausse fait relire les sections nommées avant d'ouvrir 5f.

- **H-5e-1** — 5e est poussée, sa note « Découvert » existe, et sa sortie tient : `make test` vert avec `old-names-guard`, `french-guard` et `text-guards-test` ; `make test-all` vert avec `ast-guard` et `ast-guard-test`, sur une machine qui a libclang et les bindings. Sans eux, `make test-all` échoue en nommant le manque ; 5f s'arrête alors et le dit, sans contourner l'invariant. *À relire* : Condition d'entrée ; Implementation Chunks, point 0 ; décision 15.
- **H-5e-2** — 5e ne touche, hors code, que la ligne « Build » du `CLAUDE.md` (décision 6 de 5e), la légende du `Makefile` et le §5 du plan dans sa note. Elle ne touche ni la ligne 5 du `CLAUDE.md`, ni le `README`, ni `docs/` hors plan, ni `.claude/` (« `gh-release` … je ne le touche pas »). Les numéros de ligne de l'inventaire valent donc encore, sauf dans `CLAUDE.md` si la ligne « Build » change de hauteur. *À relire* : Inventaire exact ; `CLAUDE.md`, section « Arborescence et noms » (placement, renvoi aux gardes) ; Implementation Chunks, point 2.
- **H-5e-3** — Les gardes lisent ce que dit 5e et rien d'autre : `source_files` (`src/`, `tests/`, `tools/`, `Makefile`), la table, les `tools/*_exceptions.txt`, les unités C ; ses deux tests de vivacité travaillent sous `tmp/`. Aucune ne lit `docs/` hors table, `CLAUDE.md`, `README.md` ni `.claude/`. *À relire* : Effet des gardes ; décisions 9 et 15.
- **H-5e-4** — `make test-all` dure environ 2 min 30 sur 4 cœurs (150 s mesurés par la PRD de 5e) : environ 18 min pour les sept commits. *À relire* : décision 15.
- **H-5e-5** — 5e ne renomme rien, ne change pas `docs/renommage.tsv`, et n'ajoute aucun fichier de `docs/` hors de `docs/prd/` ni de `.claude/`. Les 153 noms de l'étape 5, les 3 852 de la mesure élargie et les 35 fichiers du périmètre restent ceux de cette PRD ; ses deux tests neufs (`tests/test_text_guards.py`, `tests/test_ast_guard.py`) sont hors du périmètre de la sortie. Sa PRD est graduée avant que 5f ne commence. *À relire* : Mesures ; Règle de mesure ; décisions 3 à 5 ; Implementation Chunks, point 0.
- **H-5e-6** — `rename_tables.read_table`, `rename_count.Names` (`distinctive_words`, `paths`, `ambiguous`) et `rename_count.is_ambiguous` gardent leurs noms et leurs contrats : le script de mesure les importe. *À relire* : Règle de mesure ; décision 3.
- **H-5e-7** — 5e ne change ni le §3 ni le §4 du plan, ni l'arbre de `src/` : la section « Arborescence et noms » s'écrit d'après l'arbre et le §4 de fin de 5e. *À relire* : `CLAUDE.md`, section « Arborescence et noms » ; décision 8.
- **H-5e-8** — 5e ne fixe aucune convention de marquage du registre ; si elle en fixe une, 5f la suit pour D-65 dans son commit de note. *À relire* : décision 16 ; Implementation Chunks, point 7.

## Further Notes

- **Point d'arrêt humain, à la fin de 5f.** C'est la sortie du plan (« relecture humaine des skills ») et la fin du tronçon 4. L'agent s'arrête et demande :
  - la relecture du diff des skills, commits 1, 5 et 6 (`exemple-travaille.md`, `consigne-agent.md`) ;
  - un avis sur la ligne 23 de `SKILL.md` (« quatre destinations ») et la rubrique « glossaire » de `modele-rapport.md`, laissées telles quelles (décision 11) ;
  - la relecture des décisions prises sans humain de 5a à 5f, que chaque note renvoie « au prochain point d'arrêt ».
- **Ce que 5f laisse pour l'humain** :
  - **les docs ne sont gardées par rien** (décision 9) : la sortie de 5f est une photographie ;
  - **la fin du chantier** : le plan et le registre décrivent un travail fini ; la règle « Docs » du `CLAUDE.md` les fait supprimer, mais leurs notes portent les décisions à relire. Après la relecture, leur suppression touche aussi les 12 fichiers de `tools/` qui citent `docs/plan-5.md` dans une docstring ou un en-tête, et pose la question de `docs/vocabulaire.md` et des outils `rename_*`, qui n'ont plus d'usage hors chantier (`--substep` : alerte 11 de D-68). `docs/renommage.tsv` reste : c'est le pont inverse ;
  - **`docs/design/slipways-mechanics.md`** reste un document sur le cart, en noms Lua (décision 6) ;
  - **`docs/psx-hello-world.md`** est en anglais et cite un hook disparu (décision 13) ;
  - les questions ouvertes de 5b à 5e (§ « Out of Scope »).
- **Pourquoi les règles de nommage dans `CLAUDE.md`.** Le §2 du plan rend l'anglais permanent, et les gardes de 5e le tiennent ; mais une garde dit « non » sans dire comment. Trois lignes dans `CLAUDE.md` coûtent peu à chaque session et évitent un aller-retour par garde rouge (décision 8). Si l'humain préfère la lettre du plan, la section se réduit à l'arborescence.
