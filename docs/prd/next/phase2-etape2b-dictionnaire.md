---
title: "Plan 5, étape 2b — dictionnaire complet (docs/renommage.tsv)"
description: "Avant de renommer quoi que ce soit, il faut la table exhaustive et sans collision de chaque ancien nom et de son nom anglais, relue par un humain, et le découpage des étapes 3 et 4 qui l'appliqueront."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 2b. **Dictionnaire complet — point d'arrêt humain.** `docs/renommage.tsv` : fichiers, types, fonctions, champs, constantes, statiques, globales, noms de tests et de fixtures, dérivés du vocabulaire et des règles du §4. Extraction exhaustive par l'AST clang (`-Xclang -ast-dump=json`), pas à la main : chaque déclaration de 1–2 caractères ou issue d'un digramme y figure. Les noms portés par une **chaîne** (clés d'étiquette comme `pico8_cg("jo")`, noms enregistrés par la couverture de classes, fichiers attendus de `tools/`) forment une catégorie à part du TSV : le compilateur ne les voit pas. Avant la relecture, le TSV satisfait un critère mécanique : **exhaustif** (chaque déclaration extraite a une entrée ou le statut explicite « conservé », ce qui y fait entrer les noms français, que ni l'une ni l'autre garde ne repère) et **sans collision** (aucun nom neuf en double, aucun masquage dans une même portée). La garde anti-cart est branchée en compteur. L'étape inscrit dans ce plan, comme entrées numérotées à part entière (3a, 3b…, 4a, 4b…, chacune avec son texte et ses modules), les sous-étapes des étapes 3 et 4 : `game/`, environ 70 % du code, y est découpé en familles de taille comparable. Sa note « Découvert » n'en garde que la justification par les volumes. L'humain relit un échantillon ; un nom corrigé dans l'échantillon fait revoir ceux qui en dérivent.

Phase 2 du pilotage (tronçon 2 : étapes 1, 2a, 2b), dernière étape du tronçon : elle se termine sur un **point d'arrêt humain**. Entrées : `docs/plan-5.md` (intro, §2, §4, §5, §6 entrées 0, 1, 2a, 3, 4), `docs/vocabulaire.md` (« Statut : validé » — on n'en contredit aucun nom), la note « Découvert » de l'étape 2a et l'outil qu'elle livre. Cette PRD est écrite **avant** que l'étape 2a soit faite : tout ce qu'elle suppose de l'outil est une hypothèse H-2a-k (section dédiée en fin de document), à revérifier dès que la note « Découvert » de 2a existe.

**Condition d'entrée** : les commits de l'étape 2a sont poussés sur la branche du chantier ; sa note « Découvert » existe sous l'entrée 2a du plan ; `make test-all` est vert à HEAD.

## Problem Statement

Le plan 5 va renommer en anglais tout le code d'un portage dont les noms viennent d'un cart minifié : des milliers de types, fonctions, champs, constantes, macros, statiques et globales, plus des noms de fichiers de tests et de fixtures. Le vocabulaire validé fixe 83 concepts et la vtable ; il ne dit rien des milliers de symboles qui en dérivent (`pico8_jo_kj_dispatch`, `pico8_qe_map_t`, un champ `nz` lu en passant), ni des noms déjà anglais ou encore français qui restent hors de son radar.

Sans une table complète, chaque sous-étape de l'étape 3 inventerait ses noms localement : collisions entre sous-étapes, deux sens d'un même digramme (`hp`, `kf`, `nk`, `cr`) mal départagés, noms français oubliés parce qu'aucune garde ne les voit, et plus aucun pont pour retourner fouiller le Lua une fois `docs/glossaire-noms.md` supprimé. Et les noms portés par une chaîne (`pico8_cg("jo")`, clés de la couverture de classes, fichiers attendus de `tools/`) échappent au compilateur : s'ils ne sont pas listés à part, ils seront renommés d'un seul côté.

Il faut aussi, avant de lancer le tronçon 3, savoir comment couper l'application : `game/` pèse environ deux tiers du code, une PRD unique serait illisible et un échec indébogable.

Enfin, l'humain ne peut pas relire des milliers de lignes : il lui faut une table mécaniquement saine (exhaustive, sans collision) et un échantillon ciblé où son jugement compte.

## Solution

L'étape produit et commite, sur la branche du chantier :

1. **`docs/renommage.tsv`**, le pont inverse unique (§2) : une ligne par déclaration extraite par l'outil de 2a, colonnes `ancien`, `nouveau`, `fichier`, plus les colonnes que la mécanique exige (cf. Implementation Decisions), avec un statut explicite (`renommé` ou `conservé`). Deux catégories : **identifiants** (vus par le compilateur, appliqués par l'outil) et **chaînes** (clés et noms que seul le texte porte, appliqués des deux côtés à la main par chaque sous-étape). Une en-tête de fichier porte le marqueur de validation `Statut : à valider`.
2. **Un contrôle mécanique** rejouable (cible `make` hors suite) qui prouve que le TSV est exhaustif par rapport à l'extraction et sans collision, avant toute relecture ; il sera rejoué par la condition d'entrée de 3a sur la version validée.
3. **La garde anti-cart en compteur** (§5, garde 1) : une cible `make` hors suite, non bloquante, qui compte par module les anciens noms du TSV restants, en trois catégories (identifiant, entre backticks, dans une chaîne), les noms courts via l'extraction AST.
4. **Les sous-étapes 3a, 3b…, 4a, 4b…** inscrites dans `docs/plan-5.md` comme entrées numérotées à part entière, chacune avec son texte et ses modules ; `game/` découpé en familles de taille comparable.
5. **La note « Découvert »** sous l'entrée 2b, qui ne garde du découpage que la justification par les volumes.
6. **Un guide d'échantillon** pour l'humain : quelles lignes relire en priorité, et la règle « un nom corrigé fait revoir ceux qui en dérivent ».

L'étape ne touche aucun symbole du code. Elle s'arrête : le tronçon 3 ne reprend qu'après que l'humain a passé le marqueur à `Statut : validé`. L'agent ne bascule jamais ce marqueur.

## User Stories

1. En tant qu'agent d'une sous-étape 3, je veux une table qui me donne le nouveau nom de chaque symbole de mes modules, afin de n'inventer aucun nom.
2. En tant qu'agent d'une sous-étape 3, je veux que la table soit directement consommable par l'outil `apply` de 2a, afin que l'application soit une commande.
3. En tant qu'agent d'une sous-étape 3, je veux filtrer la table par module, afin de n'appliquer que ma tranche.
4. En tant qu'agent d'une sous-étape 3, je veux qu'une entrée désigne une déclaration sans ambiguïté, même quand deux champs `p` existent, afin que l'outil ne refuse pas pour homonymie.
5. En tant qu'agent d'une sous-étape 3, je veux la liste à part des noms portés par une chaîne, avec chaque site qui les écrit et chaque site qui les lit, afin de les renommer des deux côtés dans le même commit.
6. En tant qu'agent d'une sous-étape 3, je veux savoir quels fichiers de tests et de fixtures changent de nom, et vers quoi, afin que les tests suivent dans la sous-étape de leur module.
7. En tant qu'agent d'une sous-étape 3, je veux un compteur anti-cart par module, afin de prouver ma fin de sous-étape (« compteur à zéro pour ses modules, catégories identifiants et chaînes »).
8. En tant qu'agent d'une sous-étape 4, je veux que le compteur distingue les mentions entre backticks, afin de savoir ce qui me revient et ce qui revenait à l'étape 3.
9. En tant qu'agent de l'étape 5, je veux une garde anti-cart déjà écrite et éprouvée en compteur, afin de n'avoir qu'à la rendre bloquante.
10. En tant que relecteur humain, je veux que la table soit déjà prouvée exhaustive et sans collision, afin de ne relire que le sens des noms.
11. En tant que relecteur humain, je veux un échantillon désigné (doutes du vocabulaire, doubles sens, noms courts, noms français, collisions évitées), afin que mon temps aille où il compte.
12. En tant que relecteur humain, je veux, pour chaque nom de l'échantillon, voir de quel concept du vocabulaire il dérive, afin de juger la dérivation et pas seulement le mot.
13. En tant que relecteur humain, je veux qu'une correction d'un nom de l'échantillon se propage aux noms qui en dérivent, afin de ne pas corriger 40 fois `jo` dans 40 noms.
14. En tant que relecteur humain, je veux un marqueur de validation que moi seul bascule, afin que le tronçon 3 ne parte pas sans mon accord.
15. En tant qu'agent de la condition d'entrée de 3a, je veux rejouer le contrôle mécanique sur la version validée, afin qu'une correction humaine n'ait pas introduit une collision.
16. En tant que mainteneur futur, je veux retrouver le nom Lua d'un symbole C depuis la table, afin de retourner fouiller `pico8/lua.lua` sans glossaire.
17. En tant que mainteneur, je veux que chaque déclaration extraite ait une ligne, conservée ou renommée, afin que les noms français et les noms déjà anglais soient décidés et non oubliés.
18. En tant que mainteneur, je veux que les noms courts de locales et de paramètres à portée ≤ 40 lignes soient marqués conservés plutôt qu'omis, afin que l'exhaustivité soit vérifiable par différence.
19. En tant que mainteneur, je veux que les noms neufs suivent le §4 (`module_verbe`, `snake_case_t`, `MODULE_UPPER_CASE`, `p8_` pour le runtime), afin que le résultat soit homogène.
20. En tant que mainteneur, je veux que les deux sens d'un même digramme (`hp`, `kf`, `nk`, `cr`, `p` sur la caméra) aient deux noms différents, comme le vocabulaire l'a fixé.
21. En tant que mainteneur, je veux que les noms de binaires d'outil (`tmp/pico8_replay`…) soient tranchés dans la table, afin de lever la question laissée par l'étape 1.
22. En tant que mainteneur, je veux que `tools/frames_ref.sh` continue de reconstruire les captures des commits anciens, afin que le renommage des binaires ne casse pas l'outil de comparaison de frames.
23. En tant que pilote (`plan-pilot`), je veux les sous-étapes 3 et 4 inscrites dans le plan comme entrées numérotées, afin d'en faire une étape de phase chacune.
24. En tant que pilote, je veux des sous-étapes de taille comparable, afin qu'aucune PRD ne soit hors de portée d'un agent.
25. En tant que pilote, je veux que l'ordre des sous-étapes respecte les dépendances (3a d'abord : en-tête commun et vtable), afin que chaque sous-étape compile sur la précédente.
26. En tant que relecteur des commits au point d'arrêt, je veux une note « Découvert » courte qui justifie le découpage par les volumes, afin de valider le découpage sans relire tout le calcul.
27. En tant qu'agent de 2b, je veux connaître ce que l'outil de 2a gère et ne gère pas (macros, `##`, configurations), afin de marquer les entrées que l'outil ne pourra pas appliquer.
28. En tant que mainteneur, je veux que le TSV soit trié de façon stable et diffable, afin que la correction humaine se lise dans un `git diff`.

## Implementation Decisions

### Le TSV : forme

- Fichier texte tabulé, UTF-8, une ligne d'en-tête de colonnes, précédé de lignes de commentaire (`#`) qui portent le marqueur `Statut : à valider` et un renvoi à ce plan. Le marqueur suit la convention de `docs/vocabulaire.md` (une ligne `Statut : …` lisible par un `grep`). L'agent l'écrit à `à valider` et **ne le change jamais** ; la condition d'entrée de 3a vérifie `validé`.
- Colonnes : les trois du §2 en tête (`ancien`, `nouveau`, `fichier` = fichier de la déclaration, chemin d'après l'étape 1), puis `catégorie` (`identifiant` / `chaîne` / `fichier`), `sorte` (type, fonction, champ, constante, macro, statique, globale, paramètre, locale, fichier de test, fixture, binaire, clé…), `statut` (`renommé` / `conservé`), `module` (module cible du §3, qui sert au filtrage par sous-étape), `concept` (le concept de `docs/vocabulaire.md` dont le nom dérive, vide sinon), `note` (vide, ou `manuel` quand l'outil ne sait pas réécrire), `identité` (l'identité de l'extraction, H-2a-3). `nouveau` vaut `ancien` quand le statut est `conservé`.
- Une ligne par **déclaration** (identité), pas par occurrence. Les occurrences restent dans la sortie de l'extraction, régénérable.
- Tri stable : par module, puis catégorie, sorte, ancien nom, identité.
- `apply` de 2a reçoit le TSV filtré (par module, catégorie `identifiant`, statut `renommé`) ; si son format d'entrée diffère (H-2a-4), 2b fait suivre l'outil — c'est ce que la PRD de 2a a prévu (« l'étape 2b peut faire évoluer ce format : l'outil suit ») — plutôt que de tordre le TSV.

### Production : extraction, pas la main

- L'ensemble des lignes `identifiant` est **l'image exacte** de la sortie `extract` de 2a sur l'arbre courant, toutes configurations (H-2a-5). Aucune ligne `identifiant` n'est ajoutée à la main ; aucune identité extraite n'est omise.
- Les **noms neufs** sont dérivés par un script de dérivation (commité dans `tools/`, en anglais) qui applique, dans l'ordre : (1) les noms explicites de `docs/vocabulaire.md` (concepts, en-tête commun, vtable, arborescence) ; (2) les règles de composition du §4 sur les préfixes et suffixes (`pico8_jo_X` → `planet_X`, `pico8_X_dispatch` → `X_<méthode>_method`, `pico8_X_def` → `X_archetype`, `PICO8_X` → `MODULE_X`, `pico8_` du runtime → `p8_`, fonction `static` sans préfixe de module) ; (3) une table de surcharges à la main, petite, commitée avec le script, pour ce que les règles ne savent pas dire. Le script est rejouable : la condition d'entrée de 3a le relance après correction humaine pour propager (« une correction fait revoir ceux qui en dérivent »).
- Un nom que la dérivation ne sait pas produire n'est pas deviné : la ligne est écrite avec `nouveau` vide et listée dans un fichier de reste ; l'agent les tranche à la main (surcharge) avant le contrôle mécanique. Un reste non vide fait échouer le contrôle.
- **Jamais contre le vocabulaire validé** : un nom du vocabulaire est appliqué tel quel, doutes compris (la colonne « doute » a été tranchée par la validation). Si la dérivation ou une collision force à s'en écarter, l'agent ne change pas le vocabulaire : il écrit le nom forcé dans le TSV, le signale dans la note « Découvert » et le place en tête de l'échantillon.
- **Conservé** : statut explicite pour (a) les noms déjà conformes au §4 ; (b) les locales et paramètres de 1–2 caractères à portée ≤ 40 lignes (tolérance du §1, portée mesurée par l'extraction, H-2a-6) ; (c) `x`/`y` champs de vecteur ; (d) les membres conservés du vocabulaire (`states`, `tags`, `lineage`, `emitter`, `ui`). Un **nom français** n'est jamais conservé dans la catégorie identifiant : il est renommé (périmètre §2). Un nom court à portée > 40 lignes, ou qui nomme autre chose qu'une locale ou un paramètre, n'est jamais conservé.
- Les macros sont des déclarations comme les autres si l'outil les extrait (H-2a-7) ; une macro ou une occurrence que l'outil déclare ne pas savoir réécrire (`##`, corps de `#define` ambigu) garde sa ligne au statut `renommé`, avec une marque `manuel` dans une colonne `note` : la sous-étape 3 concernée la traitera à la main, le compilateur arbitrant. Ces lignes entrent d'office dans l'échantillon.

### Catégorie « chaîne »

- Recensement par recherche textuelle outillée (pas par l'AST, qui ne voit pas le contenu des chaînes), sur trois familles nommées par le plan, plus ce que la recherche découvre :
  1. **Clés d'étiquette et de message** : `pico8_cg("…")`, `pico8_cv("…")` et équivalents (aujourd'hui une vingtaine de sites dans `game/`), ainsi que les noms d'état chaîne (`"ph"`, `"ly"`…). Les noms neufs sont ceux du vocabulaire : `"draw"`, `"draw_overlay"`, `"hit_test"`, `"new_year"`, `"refresh"` pour la vtable, `"normal"`, `"settled"`, `"placing"`, `"unfolding"`, `"vanishing"` pour les états ; pour les étiquettes, le nom anglais du concept (`"jo"` → `"planet"`).
  2. **Noms enregistrés par la couverture de classes** (devtools `class_coverage`) et leurs références attendues (`tools/class_coverage_*.txt`).
  3. **Fichiers et noms attendus de `tools/`** : listes d'exceptions et références qui citent des symboles ou des chemins (`*_exceptions.txt`, `*_exempt.txt`, `line_coverage_reference.txt`, `float_guard.txt`…), chaînes `"test_X: all tests passed"`, chemins `tmp/test_X` (relevés par la note « Découvert » de l'étape 1).
- Une ligne de chaîne donne l'ancien texte, le nouveau, et le fichier de chaque site **écrivain** et **lecteur** (plusieurs lignes, même `ancien`, ou une colonne de sites : au choix de l'implémentation, tant que le compteur anti-cart et l'agent de 3 le lisent). Une clé dont un seul côté est trouvé est signalée dans « Découvert ».
- Les chaînes affichées au joueur (texte du jeu) sont hors catégorie : elles font partie du rendu, que l'invariant fige.

### Catégorie « fichier » : tests, fixtures, binaires

- Noms de fichiers de `tests/` et `tests/fixtures/` encore marqués par le cart après l'étape 1 (ex. `test_pico8_*`), dérivés du nom du module testé ; noms des `*.sha256`/`.json` d'e2e seulement s'ils portent un ancien nom.
- **Binaires d'outil** (`tmp/pico8_replay`, `pico8_frames`, `pico8_class_coverage`, `pico8_minimize`, `main_raylib`, `pico8_replay_gcov`…) : question ouverte laissée par l'étape 1, tranchée ici. Décision : ils prennent le nom de leur fichier `cmd/` (`tmp/replay`, `tmp/frames`, `tmp/game`…) **sauf** si le renommage casse `tools/frames_ref.sh`, qui lance `make tmp/pico8_replay` puis `./tmp/pico8_replay` dans un worktree d'un commit **ancien**. Le TSV le règle par l'une de deux voies, à choisir par l'agent sur pièces et à justifier dans « Découvert » : (a) `frames_ref.sh` garde l'ancien nom pour les commits anciens et le nouveau pour les récents (même schéma que son test de `src/main.c` gardé exprès), ou (b) le binaire de rejeu garde son nom, marqué `conservé` avec cette raison. L'application est faite en 3, pas ici.

### Contrôle mécanique (avant relecture)

Une cible `make` hors suite (nom indicatif `rename-table-check`), qui lit le TSV et la sortie d'extraction et échoue sur :

- **Exhaustivité** : toute identité extraite a une ligne ; toute ligne `identifiant` correspond à une identité extraite ; aucun `nouveau` vide ; fichier de reste vide.
- **Collision** : aucun nom neuf porté par deux identités dans un même espace de noms C où ils se rencontreraient — même portée de fichier pour les globales, fonctions, types et macros (et portée globale du programme pour tout ce qui a une liaison externe ou est déclaré dans un en-tête), même struct pour les champs, même fonction pour les locales et paramètres — ; aucun **masquage** (une locale ou un paramètre dont le nom neuf est celui d'une globale, d'un type ou d'une macro visible dans sa portée) ; aucun nom neuf égal à un mot réservé C ou C++ (`class`, cf. vocabulaire `cls`) ; aucune collision entre un nom neuf et un nom conservé.
- **Fichiers** : noms cibles de fichiers uniques (exigé par l'intro du plan).
- **Règles du §4** (contrôle de forme, avertissement seulement) : `snake_case_t` pour les types, `UPPER_CASE` pour les macros et constantes, préfixe `p8_` réservé aux symboles de `src/p8/`.
- Sortie : le contrôle imprime les violations, sort en échec s'il y en a. Le TSV n'est livré à la relecture que contrôle vert.

### Garde anti-cart en compteur

- Un script `tools/` (anglais) et une cible `make` hors suite (nom indicatif `cart-names-count`), **jamais en échec** (code de sortie 0) à cette étape ; l'étape 5 la rendra bloquante et l'intégrera à `make test`.
- Entrée : les anciens noms des lignes `renommé` du TSV. Sortie : un tableau par module (§3) × catégorie (`identifiant`, `backtick`, `chaîne`), et un total.
- Noms longs (≥ 3 caractères et non homonymes d'un mot ou d'un nom neuf) : recherche texte en mot entier, classée par contexte (commentaire entre backticks, chaîne, code). Noms courts ou homonymes (`p`, `t`, `jo`…) : catégorie identifiant comptée par l'extraction AST (une occurrence d'une identité renommée = une), H-2a-8 ; en backtick et en chaîne, recherche texte délimitée par les backticks ou les guillemets (`` `p` ``, `"jo"`).
- À HEAD de cette étape, le compteur est massivement non nul : c'est son état de départ, imprimé dans « Découvert » (total par module), qui sert de référence aux sous-étapes 3.
- La dépendance aux bindings libclang (si l'extraction en a besoin, H-2a-1) est assumée hors suite ; la PRD de 2a renvoie déjà le choix pour `make test` à l'étape 5.

### Découpage des étapes 3 et 4

- **3a** est fixée par le plan : en-tête commun et vtable (`entity_t`, `entity_class_t`, membres de la vtable, clés chaîne de `entities_by_message`, états). Les sous-étapes suivantes sont inscrites par 2b.
- Unité de mesure : lignes de `src/` du module + lignes des tests et fixtures qui le suivent (§6 entrée 3 : « les tests, fixtures et données de `tools/` suivent dans la sous-étape de leur module ») + nombre de lignes `renommé` du TSV du module. Mesure faite par script sur l'arbre de HEAD, chiffres dans « Découvert ».
- Ordre : du bas vers le haut des dépendances (runtime `p8/`, puis `engine/`, puis familles de `game/`, puis `platform/`, `devtools/`, `cmd/`), pour qu'un module renommé ne soit utilisé que par des modules qui le suivent ou qui sont déjà passés ; une famille peut regrouper petits répertoires (`platform/` + `cmd/`).
- `game/` (≈ 15 400 lignes à HEAD, plus de la moitié de `src/`) : coupé en **familles de modules apparentés** de taille comparable (cible : écart max/min ≤ ×2 en lignes pondérées), par concept du vocabulaire plutôt qu'alphabétiquement. Point de départ indicatif, à confirmer par la mesure : planète (`planet*`, `structure_icon`, `galaxy`, `gen`) ; slipway et commerce (`slipway*`, `trade`, `planet_trade`, `planet_family_trade`) ; économie et recherche (`economy*`, `research*`, `main_economy`, `tech_gate`) ; sonde, ciblage, trou de ver, particules (`probe`, `targeting`, `wormhole*`, `particle*`) ; interface (`ui*`, `layout`, `message_box`, `menu`, `hud`, `help`, `score_panel*`, `draw_functions`, `mouse*`, `background`, `scroll`) ; données (`game_data`, `map_tables`, `data_init`, `cart`, `cheat`). Une famille trop lourde (interface) se coupe en deux.
- Chaque entrée inscrite au §6 suit la forme des entrées existantes : numéro (`3b.`…), titre gras, texte (ce qu'elle renomme, ses critères de fin repris du §6 entrée 3 ou 4), et la liste explicite de ses modules (répertoire + noms). Les sous-étapes 4 reprennent **la même liste** (§6 entrée 4) : 4a, 4b… pointent vers les mêmes modules que 3a, 3b…. L'entrée 3 et l'entrée 4 existantes restent comme chapeaux.
- La note « Découvert » de 2b ne garde du découpage que la justification par les volumes (tableau famille → lignes pondérées), pas le raisonnement.

### Point d'arrêt et relecture

- Livrable à l'humain : le TSV contrôle vert, le compteur à son état de départ, le plan à jour, et une section « Échantillon à relire » dans la note « Découvert » (ou en tête du TSV en commentaires, au choix ; la note est préférée car le TSV reste une table).
- Échantillon désigné, environ 150 lignes : (1) toute ligne dont le concept porte une colonne « doute » non vide dans le vocabulaire ; (2) les doubles sens (`hp`, `kf`, `nk`, `cr`, `p` de la caméra, `px`) ; (3) les noms tranchés par surcharge à la main ; (4) toutes les lignes `chaîne` et `fichier` (binaires compris) ; (5) un tirage reproductible (graine fixée) de lignes par module ; (6) les noms français renommés ; (7) tout nom où une collision a forcé un écart.
- Consigne écrite à l'humain : corriger soit le vocabulaire, soit la table de surcharges (pas le TSV directement), puis passer le marqueur à `validé` ; la condition d'entrée de 3a relance la dérivation et le contrôle mécanique sur la version validée (intro du plan).

## Implementation Chunks

1. **Lecture de 2a et condition d'entrée** : note « Découvert » de 2a, revue des hypothèses H-2a-k ci-dessous, mise à jour des sections concernées de cette PRD si l'une tombe. → vérif : chaque H-2a-k marquée confirmée ou corrigée dans la note de travail.
2. **Extraction de référence** sur HEAD, toutes configurations ; liste des unités non analysées vide (sinon la traiter avant de continuer). → vérif : deux extractions successives identiques.
3. **Dérivation des noms** (script + surcharges) et reste à vide. → vérif : tests du script sur un mini-vocabulaire et une mini-extraction ; reste vide sur le dépôt.
4. **Catégories chaîne et fichier**, binaires compris (décision `frames_ref.sh`). → vérif : chaque clé chaîne a au moins un écrivain et un lecteur, ou est signalée.
5. **Contrôle mécanique** et TSV vert. → vérif : tests du contrôle (collision, masquage, réservé, exhaustivité) ; cible verte sur le dépôt.
6. **Compteur anti-cart**. → vérif : tests sur un mini-arbre ; état de départ imprimé ; code de sortie 0.
7. **Découpage 3/4** inscrit au plan, avec la mesure. → vérif : chaque module du §3 apparaît dans exactement une sous-étape ; écart des familles de `game/` ≤ ×2.
8. **Note « Découvert »**, échantillon, marqueur `à valider`, correction de la mention `-Xclang -ast-dump=json` de l'entrée 2b si 2a ne l'a pas déjà faite (H-2a-2). Invariant du §5 en fin de PRD. Arrêt.

## Testing Decisions

- **Un bon test** donne au script une entrée fabriquée (mini-extraction, mini-vocabulaire, mini-TSV, mini-arbre de sources) et vérifie sa sortie et son code de retour, jamais ses structures internes.
- **Dérivation** : un concept du vocabulaire appliqué à un préfixe (`pico8_jo_kj` → `planet_draw`), une méthode `_dispatch`, un archétype `_def`, une fonction `static` qui perd son préfixe, un nom sans règle qui tombe dans le reste, une surcharge qui l'emporte sur la règle, la propagation d'un concept corrigé à ses dérivés.
- **Contrôle mécanique** : deux identités vers le même nom global (échec), deux champs de structs différentes vers le même nom (succès), deux champs d'une même struct (échec), locale masquant une globale renommée (échec), `class` (échec), identité extraite sans ligne (échec), ligne sans identité (échec), deux fichiers vers le même nom cible (échec).
- **Compteur anti-cart** : mini-arbre avec un ancien nom long en code, en backtick, en chaîne, un nom court en champ et en locale homonyme non renommée ; comptes attendus par catégorie et module ; code de sortie 0 même non nul.
- **Où** : les tests de ces scripts suivent le schéma des tests d'outils de `tools/` (prior art : `tests/test_composition_residue.c`, `test_data_residue`, `test_alloc_guard` — mini-arbre dans `tmp/`, sortie et code de retour). Ceux qui n'ont pas besoin des bindings libclang entrent dans `make test` ; ceux qui en ont besoin vont avec la cible de tests de l'outil de 2a, hors suite.
- **Invariant du §5** en fin de PRD : `make test-all` vert, `*.sha256` de `tests/e2e/` inchangés, `make raylib`, `screencast-viewer`, `class-coverage-net`, `e2e-gap` dans leur état d'avant l'étape (rouges connus de l'étape 1 inchangés). Aucun code de `src/` ne change, donc aucune différence attendue.

## Out of Scope

- Appliquer un seul renommage au code, aux tests, aux fixtures ou aux binaires : étape 3.
- Commentaires, messages d'`assert`, mentions entre backticks, locales cryptiques à longue portée au-delà de leur ligne de TSV : étape 4.
- Le compteur de français résiduel (étape 4) et la garde de portée (étape 3).
- Rendre la garde anti-cart bloquante ou l'intégrer à `make test` : étape 5.
- Modifier `docs/vocabulaire.md` : il est validé ; un désaccord va dans « Découvert ».
- Basculer le marqueur `Statut : à valider` : l'humain seul.
- Corriger l'outil de 2a au-delà de ce que le format du TSV exige.
- Supprimer `docs/glossaire-noms.md` : étape 5.
- Lancer l'appli raylib : jamais.

## Hypothèses sur l'étape 2a (pas encore faite)

Chacune est à confronter à la note « Découvert » de 2a, à l'outil livré et au `Makefile`, avant de lancer cette PRD.

- **H-2a-1** — L'outil est en Python 3 sur **libclang** (bindings installés par une cible `make` hors suite, nom indicatif `rename-tool-setup`), pas sur `-Xclang -ast-dump=json` ; il vit dans `tools/` et se lance par une cible `make` qui lui passe les flags du `Makefile`.
- **H-2a-2** — La note de 2a a corrigé la mention `-Xclang -ast-dump=json` de l'entrée 2b du plan si le moteur diffère. Si non, 2b la corrige (chunk 8) ; le texte copié en tête de cette PRD garde la formulation d'origine.
- **H-2a-3** — Chaque déclaration a une identité stable inter-unités (USR clang ; pour une macro, nom + fichier + offset de définition), lisible, utilisable comme colonne du TSV et comme clé d'`apply`.
- **H-2a-4** — `extract` produit un fichier tabulé trié de façon stable, une ligne par identité, colonnes : identité, sorte, nom, parent, fichier et ligne de déclaration, liaison, portée en lignes, nombre d'occurrences, marqueurs `short`/`homonym` ; `apply` lit une table `ancien`, `nouveau`, `fichier` (+ identité optionnelle) et refuse en bloc tout cas non sûr.
- **H-2a-5** — `extract` couvre toutes les unités `.c` suivies sous `src/`, `tests/`, `tools/`, dans les configurations défaut, `PICO8_CLASS_COVERAGE` et ASan, unités raylib comprises après `raylib-vendor-setup` ; la liste des unités non analysées est vide sur le dépôt.
- **H-2a-6** — La portée d'une locale ou d'un paramètre est mesurée en lignes par l'extraction (ce qui permet le statut `conservé` des noms courts).
- **H-2a-7** — Les macros sont des déclarations extraites avec leurs utilisations ; arguments de macro et corps de `#define` à candidat unique sont gérés ; `##` et `#` sur un identifiant renommé sont détectés et refusés, pas gérés (un seul `##`, sur un littéral, dans `PICO8_FIXED_LIT`).
- **H-2a-8** — Une sortie optionnelle d'occurrences (identité, fichier, offset, longueur, origine) existe et suffit au compteur anti-cart pour les noms courts et homonymes.
- **H-2a-9** — L'outil ne touche ni commentaires ni chaînes, et ne renomme pas de fichiers : les catégories `chaîne` et `fichier` du TSV restent hors de lui.
- **H-2a-10** — 2a a rendu le code à son état d'origine : aucun symbole n'a changé depuis l'étape 1, les noms cités ici (`pico8_cg`, `pico8_cv`, `pico8_struct_header_t`, `tmp/pico8_replay`…) existent tels quels.

**Sections à revérifier quand la note « Découvert » de 2a existe** : « Le TSV : forme » (colonnes `identité` et format d'`apply`, H-2a-3, H-2a-4) ; « Production : extraction, pas la main » (couverture, portées, macros, H-2a-5 à H-2a-7) ; « Catégorie chaîne » et « Catégorie fichier » (frontière avec l'outil, H-2a-9, H-2a-10) ; « Contrôle mécanique » (portées et parents nécessaires au masquage, H-2a-4, H-2a-6) ; « Garde anti-cart en compteur » (H-2a-1, H-2a-8) ; « Implementation Chunks » 1, 2 et 8 (H-2a-2) ; « Testing Decisions », répartition `make test` / hors suite (H-2a-1) ; et cette section elle-même.

## Further Notes

- **Mesures de rédaction** (HEAD `4ddba32`, après l'étape 1) : lignes par répertoire de `src/` : `p8` 3 474, `engine` 2 406, `game` 15 385, `platform` 288, `devtools` 1 571, `cmd` 781 (≈ 64 % pour `game/`, le plan dit « environ 70 % » en comptant probablement les tests) ; `tests/` 36 861 lignes `.c`/`.h`. Plus gros modules de `game/` : `game_data` 1 186, `planet_family` 848, `layout` 637, `planet_display` 610, `ui` 594, `planet` 594, `probe` 578, `slipway` 562. Une vingtaine de sites `pico8_cg("…")` dans `game/`. `tools/frames_ref.sh` lance `make tmp/pico8_replay` et `./tmp/pico8_replay` dans un worktree ancien (lignes 113 et 132).
- **Pas d'humain pour les points « vérifier avec l'utilisateur » de `to-prd`** : je n'avais pas d'outil pour lancer un agent `guideline` neuf ; j'ai chargé le skill `guideline` moi-même et tranché selon lui. Choix tranchés : les noms neufs dérivés par script + surcharges plutôt qu'à la main (rejouable pour propager les corrections humaines) ; l'humain corrige le vocabulaire ou les surcharges, pas le TSV ; compteur anti-cart hors suite (dépendance aux bindings) ; tests des scripts sans bindings dans `make test` ; binaires d'outil tranchés entre deux voies explicites pour `frames_ref.sh` ; échantillon dirigé par les doutes du vocabulaire plutôt que tiré au hasard seul.
- Le point d'arrêt ferme le tronçon 2 : la PRD de 3a n'est écrite qu'après la validation du TSV, et renvoie au TSV sans en recopier le contenu (intro du plan).
