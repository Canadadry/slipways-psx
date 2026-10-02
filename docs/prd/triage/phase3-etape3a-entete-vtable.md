---
title: "Plan 5, sous-étape 3a — en-tête commun et vtable"
description: "Première application du renommage : l'en-tête commun des entités, la vtable, les états, les étiquettes et toutes les clés chaîne qui les nomment passent en anglais, avec l'outillage de méthode des sous-étapes posé avant et sans laisser une garde passer à vide."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3a. **En-tête commun et vtable.** `engine/entity_class` : `entity_t`, `entity_class_t`, membres de la vtable, états, étiquettes, `entity_kind_t`. Toutes les clés chaîne d'étiquette, de message (`entities_by_message`) et d'état, et les noms de classe, à tous leurs sites, `tools/class_coverage_net_expected.txt` et `tools/class_coverage_exempt.txt` compris. Branche la garde de portée en compteur. Tests : `test_entity_header`, `test_entity_kind`. Résidu ≤ 434.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), première étape du tronçon. Entrées : `docs/plan-5.md` (intro, §4, §5, entrée 3 avec ses paragraphes « Sous-étapes » et « Méthode des sous-étapes », notes « Découvert » de 0, 1, 2a et 2b), `docs/renommage.tsv` validé en `7bf3d4f` (« Statut : validé ») — on y renvoie, on n'en recopie rien —, l'outillage de 2a et 2b (`tools/ast_rename`, `tools/rename_{derive,strings,check,slice,count}.py`, leurs tests). Consigne `guideline` appliquée par l'agent rédacteur (pas d'agent frais disponible) aux points « vérifier avec l'utilisateur » de `to-prd` : modules et tests ci-dessous tranchés sans interview, sur le texte du plan.

**Condition d'entrée** (après le point d'arrêt humain du TSV) :
1. Les commits du tronçon 2 sont poussés : `origin/plan-5-noms-anglais` = HEAD de départ (`7bf3d4f` ou descendant), arbre propre, `make test-all` vert.
2. Corrections humaines à propager : **aucune** — `7bf3d4f` ne change que la ligne `Statut` du TSV ; le message de commit le dit (« sans correction », trois incohérences signalées et laissées : `typed_main_*`/`main_*_typed`, `oc`, champs de `ie`). Rien à repropager vers `docs/vocabulaire.md` ni `tools/rename_overrides.tsv`.
3. Contrôle mécanique rejoué sur la version validée : `make rename-extract rename-derive rename-strings rename-table-check`, puis `git diff --exit-code docs/renommage.tsv` (la dérivation garde l'en-tête `Statut : validé` et doit reproduire la table à l'octet ; tout écart arrête l'étape). `make rename-tool-test` vert. C'est la **dernière** fois que `rename-derive`, `rename-strings` et `rename-table-check` tournent avant la fin de l'étape 3 : une fois le code renommé, l'extraction ne contient plus les anciens noms et ces outils produiraient une autre table.

## Problem Statement

Le TSV validé dit comment s'appelle chaque symbole ; il reste à l'appliquer, en douze sous-étapes, sur un code dont les modules se citent en cycle. La première, 3a, est petite en lignes (`engine/entity_class.h`, 196 lignes, et deux tests) mais la plus large en portée : l'en-tête commun (`me`, `t`, `ra`, `p`, `fa`…) et la vtable (`kj`, `ev`, `hd`…) sont lus partout — 4 134 occurrences AST dans 120 fichiers — et les clés chaîne d'étiquette, de message et d'état et les noms de classe (`pico8_cg("jo")`, `"ph"`, `"kj"`) sont lus et écrits dans 72 fichiers, sans filet du compilateur.

Le plan exige, avant la première application, une méthode que l'outillage de 2b ne fournit pas encore : une table des sous-étapes lue par les outils (au lieu de motifs qui attrapent `planet_family` sous `game/planet*`), un compteur qui sépare ce qui doit être à zéro de ce qui reste légitimement aux sous-étapes suivantes, un contrôle mécanique des chaînes, l'inventaire des gardes de `make test` écrites en noms du cart, et la garde de portée en compteur.

En préparant cette PRD, trois pièges ont été mesurés que le plan ne nomme pas :

- **Identités périmées après l'application.** `rename-apply` reconnaît une entrée par égalité exacte de son USR. Or l'USR d'une locale, d'un paramètre ou d'une statique de fonction contient l'offset en octets de sa déclaration dans le fichier (`c:replay.c@6773@F@main@remplissage_path`). Renommer les tokens de 3a décale ces offsets : **834** des 1 006 identités à offset des sous-étapes suivantes deviendraient introuvables — `rename-apply` refuserait la tranche de 3b, et `cart-names-count` cesserait silencieusement de compter ces noms ambigus (résidu sous-estimé).
- **Garde de composition aveugle.** `tools/composition_residue.sh` part de `pico8_struct_header_t` pour trouver les types d'entité (catégories `parente`, `ptrs-entité`, `registres`, `--champs`) ; renommé en `entity_t`, ce point de départ ne trouve plus rien et ces catégories, gardées à zéro, passeraient à vide. Le TSV range pourtant les deux symboles cités par ce script (`pico8_struct_header_t`, `pico8_state_of`) en lecteur de `tools/composition_residue` (3l).
- **Hachages qui plient les états.** Au-delà des 14 sites `fnv_str(h, "lh")` notés en 2b (clés de libellé, d'autres sous-étapes), 18 fichiers de tests plient la chaîne d'état `header.me` dans un hachage figé (39 sites) : renommer `"ph"` → `"normal"`… change ces constantes à 3a.

## Solution

Quatre tranches, chacune un ou plusieurs commits qui laissent `make test-all` vert et `tests/e2e/*.sha256` intacts :

1. **Outillage de méthode** (aucun symbole touché) : table des sous-étapes et son contrôle d'appartenance dans `make test` ; `SUBSTEP=` pour `rename-slice` et `cart-names-count`, compteur scindé « faites » / « suivantes » ; contrôle des sites de chaînes avec test négatif ; outil de recâblage des identités ; résidus remesurés ; inventaire des gardes à motifs.
2. **Application des identifiants de 3a** : tranche `SUBSTEP=3a` appliquée, motifs de garde qui visent un nom de 3a traduits dans le même commit avec test de vivacité, identités des sous-étapes suivantes recâblées dans le TSV.
3. **Chaînes de 3a** : les 43 clés et noms de classe renommés à leurs 484 sites, les deux fichiers de couverture de classes compris, constantes de hachage recalculées après rougissement observé.
4. **Compteurs et note** : garde de portée branchée en compteur, relevés de fin, note « Découvert » sous 3a.

À la fin, `engine/entity_class.h` ne porte plus aucun nom du cart en code ni en chaîne ; aucun ancien nom de 3a ne reste comme identifiant ou chaîne dans le dépôt ; les fichiers de 3a ne gardent en `identifier` que des noms de sous-étapes suivantes, au plus la borne remesurée ; la sous-étape 3b peut partir sur un TSV dont chaque identité restante se résout.

## User Stories

1. En tant qu'agent de la condition d'entrée, je veux vérifier que les commits du tronçon 2 sont poussés et que la table validée se rederive à l'octet, afin de ne rien appliquer sur une table que l'outil ne reproduit plus.
2. En tant qu'agent de la condition d'entrée, je veux constater par le commit de validation qu'aucune correction humaine n'est à propager, afin de ne pas réécrire le vocabulaire ou les surcharges sans raison.
3. En tant qu'agent d'une sous-étape 3, je veux une table des sous-étapes qui liste explicitement modules et tests, afin que `game/planet*` n'attrape jamais `planet_family`.
4. En tant qu'agent d'une sous-étape 3, je veux `make rename-slice SUBSTEP=3x`, afin que ma tranche soit exactement mes modules sans retaper une liste.
5. En tant qu'agent d'une sous-étape 3, je veux `make cart-names-count SUBSTEP=3x` qui sépare les noms de ma sous-étape et des précédentes de ceux des suivantes, afin de lire mon critère de fin d'un coup d'œil.
6. En tant qu'agent d'une sous-étape 3, je veux que le compteur affiche la borne de résidu à côté du résidu mesuré, afin de juger « borne de bon sens » sans rouvrir le plan.
7. En tant que mainteneur, je veux qu'un test de `make test` échoue si un `module` du TSV n'est dans aucune sous-étape ou dans deux, afin qu'aucun nom ne tombe entre deux PRD.
8. En tant qu'agent d'une sous-étape 3, je veux savoir quelles lignes `chaîne` et `fichier` du TSV sont à moi (clés d'étiquette, de message, d'état et noms de classe à 3a ; site écrivain sinon ; nom cité pour un script de `tools/` ; `fichier` à 3l), afin de ne pas en oublier ni en voler une.
9. En tant qu'agent d'une sous-étape 3, je veux un contrôle mécanique qui, à chaque site listé de mes lignes chaîne, trouve le nouveau littéral et plus l'ancien, afin que le filet des chaînes ne repose pas sur ma relecture.
10. En tant que mainteneur, je veux que ce contrôle ait un test négatif (un site oublié, un ancien littéral resté), afin de savoir qu'il mord.
11. En tant qu'agent de 3b à 3l, je veux que les identités de ma tranche se résolvent encore après les sous-étapes précédentes, afin que `rename-apply` ne refuse pas des centaines d'entrées pour des offsets décalés.
12. En tant qu'agent d'une sous-étape 3, je veux un contrôle qui prouve, après mon application, que chaque identité renommée restante du TSV existe dans une extraction fraîche, afin de passer la main sur un TSV sain.
13. En tant que relecteur humain, je veux que le TSV reste ce qui a vraiment été appliqué, et que ses seules modifications pendant l'étape 3 soient des cellules `identité` recâblées, afin de garder le pont inverse lisible.
14. En tant que mainteneur, je veux l'inventaire de chaque motif de garde écrit en noms du cart (`composition_residue.sh`, `data_residue.sh`, leurs listes), avec la sous-étape qui le traduira, afin qu'aucune garde ne passe à vide en silence.
15. En tant qu'agent de 3a, je veux traduire dans le même commit que l'application les motifs qui visent un nom de 3a (point de départ `entity_t` de la fermeture des types d'entité, exclusion `entity_state_of`, noms de champs d'en-tête des accesseurs), afin que `composition-guard` voie encore ce qu'il gardait.
16. En tant que mainteneur, je veux un test de vivacité par motif traduit (une violation plantée écrite avec les nouveaux noms fait échouer `--check`), afin de prouver que la traduction mord.
17. En tant que mainteneur, je veux que la sortie `--detail` des deux scripts de résidu soit identique avant et après chaque traduction, afin de prouver qu'elle ne change pas ce que la garde voit.
18. En tant qu'agent de 3a, je veux appliquer les 97 identités de 3a (73 de l'en-tête, 17 de `test_entity_header`, 7 de `test_entity_kind`) en une commande, afin que le compilateur arbitre tout oubli.
19. En tant qu'agent de 3a, je veux renommer les 43 clés et noms de classe renommés du TSV à tous leurs sites dans un seul commit, écrivains et lecteurs ensemble, afin qu'aucune recherche par chaîne ne rate au milieu.
20. En tant qu'agent de 3a, je veux laisser `"trade"` et `"ui"` tels quels (statut `conservé`), afin de suivre la table validée.
21. En tant qu'agent de 3a, je veux connaître d'avance la liste des tests dont un hachage figé plie une chaîne de 3a, afin que le rougissement observé soit exactement celui prévu et rien d'autre.
22. En tant que mainteneur, je veux que chaque constante de hachage recalculée l'ait été après avoir vu le test échouer sur l'ancienne valeur, afin qu'on ne recopie pas une valeur sans preuve qu'elle bouge pour la seule raison attendue.
23. En tant que mainteneur, je veux que `tools/class_coverage_net_expected.txt` soit traduit (noms remplacés, lignes retriées par la règle du script) et non régénéré, afin que ses deux écarts périmés (`trade` 247/263, `lq` 2742/2763) restent visibles sous leurs nouveaux noms.
24. En tant que mainteneur, je veux que la sortie de `make class-coverage-net` après 3a soit exactement sa sortie d'avant, noms traduits et retriée, afin de prouver que la cible rouge l'est pour les mêmes raisons.
25. En tant que mainteneur, je veux que `e2e-gap` donne les mêmes 2 932 lignes avant et après, afin de prouver qu'un renommage de tokens ne déplace aucune ligne.
26. En tant qu'agent de 3a, je veux classer les 14 clés signalées (sans écrivain ou sans lecteur) dans la sous-étape qui les tient, afin que leur note sache qui les traite ; aucune n'est à 3a.
27. En tant qu'agent de l'étape 5, je veux une garde de portée déjà écrite et éprouvée en compteur par module, afin de n'avoir qu'à la rendre bloquante.
28. En tant que relecteur humain, je veux que la garde de portée ne connaisse que les exceptions `x` et `y` du vecteur, et que tout autre cas soit proposé dans « Découvert », afin que la liste d'exceptions ne s'allonge que par ma décision.
29. En tant qu'agent de 3b, je veux une note « Découvert » sous 3a qui donne les relevés de fin, les résidus remesurés, l'inventaire des gardes et les écarts rencontrés, afin de partir de faits.
30. En tant que relecteur humain au point d'arrêt d'avant l'étape 5, je veux trouver dans les notes des sous-étapes les exceptions proposées (portée, motifs intraduisibles), afin de les trancher toutes ensemble.
31. En tant que joueur, je ne veux voir aucun pixel changer, afin que le renommage ne soit qu'un renommage.

## Implementation Decisions

### Outillage figé : comment ce qui suit le respecte

La « Méthode des sous-étapes » exige des ajouts (table, `SUBSTEP=`, scission du compteur, contrôles) tout en figeant « le schéma du TSV et ses lecteurs ». Lecture retenue :
- **Schéma du TSV inchangé** : aucune colonne ajoutée, retirée, renommée ou réinterprétée. La table des sous-étapes est un **fichier de données nouveau**, à part.
- **Lecture du TSV inchangée** : aucun code d'analyse du format ne change. Tout nouveau code lit le TSV par le lecteur existant de `rename_count` (`read_table`), jamais par un sixième analyseur ; `rename_slice` garde sa boucle de lecture et n'ajoute qu'une autre source pour la liste de modules (sélection exacte, sans motif). La logique « quelle ligne appartient à quelle sous-étape » vit en aval de la lecture, dans un module neuf.
- **Le TSV comme donnée** : pendant l'étape 3, seule la cellule `identité` de lignes `identifiant` non encore appliquées peut changer (recâblage, ci-dessous), et toute entrée corrigée est reportée dans le même commit (§6, entrée 3). Le plan l'autorise ; le recâblage est un écrivain de cellules, pas un lecteur de format.
- **Garde-fou** : si une tranche découvre qu'il faut toucher la lecture du format, elle s'arrête, fusionne d'abord les lecteurs en un seul et rejoue `rename-derive-test`, `rename-strings-test`, `rename-check-test`, `rename-slice-test`, `rename-count-test` et `rename-tool-test`, comme le veut la règle.
- Les nouveaux outils de `tools/` sont en anglais (code, §2) ; ceux qui manipulent les anciens noms s'appellent `rename_*` et leurs tests `test_rename_*`, pour tomber sous l'exclusion existante des outils du dictionnaire (compteur et `rename-table-check`). Un fichier neuf qui n'est pas `rename_*` ne cite aucun nom distinctif du TSV.

### Table des sous-étapes (module neuf, profond)

- Un fichier de données de `tools/`, nommé `rename_*` et lisible par l'humain : pour chaque sous-étape (3a…3l, dans l'ordre du plan), sa borne de résidu et la liste **explicite** de ses modules au sens de la colonne `module` du TSV (`game/planet`, `tests/test_planet`, `tools/class_coverage_net_expected`, `Makefile`). Jamais de motif.
- Contenu repris du §6 : modules et tests de chaque entrée 3a–3l ; s'y ajoutent les modules du TSV que le §6 ne nomme qu'en prose, rangés selon ses règles : `tools/class_coverage_exempt` et `tools/class_coverage_net_expected` en 3a (texte de 3a), `tools/gen_quarter_wave` en 3b, `tests/heap_allocator` en 3c, `tools/composition_residue`, `tools/composition_residue_sites`, `tools/float_grep_exclusions`, `tools/frames_ref` et `Makefile` en 3l. Tout module que le test d'appartenance trouve sans place est rangé selon « un test suit le module qu'il teste », et noté dans « Découvert ».
- Un module Python neuf expose, sur les lignes lues par `read_table` et ce fichier : sous-étape d'un module ; sous-étape d'une ligne du TSV ; liste ordonnée des sous-étapes. **Sous-étape d'une ligne** :
  1. `identifiant` : celle de son `module`.
  2. `chaîne` de sorte clé d'étiquette, clé de message, nom d'état, nom de classe (et leurs combinaisons) : **3a**, quel que soit le site.
  3. `fichier` : **3l**.
  4. autre `chaîne` dont le site est un script ou une donnée de `tools/` : celle de l'identifiant qu'elle cite (ligne `identifiant` de même `ancien` ; pour la forme `type.champ`, la ligne du champ) — « une mention d'un script ou d'une donnée de `tools/` suit le nom qu'elle cite ». C'est ce qui range `pico8_struct_header_t` et `pico8_state_of`, cités par `composition_residue.sh`, en 3a. Sans identifiant cité unique : règle 5, et la ligne est listée.
  5. autre `chaîne` : la première sous-étape (dans l'ordre) parmi les modules de ses sites `écrivain` de même identité ; sans écrivain, parmi ceux de ses sites `lecteur`.
- **Contrôle d'appartenance**, cible de `make test` : chaque `module` du TSV est dans exactement une sous-étape ; aucun module n'est listé deux fois ; chaque sous-étape a une borne. Rapide, pur Python, lit le vrai `docs/renommage.tsv`.

### `SUBSTEP=` et compteur scindé

- `make rename-slice SUBSTEP=3a` : les lignes `identifiant` renommées des modules de la sous-étape, même format de sortie qu'aujourd'hui. `MODULES=` reste.
- `make cart-names-count SUBSTEP=3a` : mêmes fichiers que `MODULES=<modules de 3a>`, mêmes quatre colonnes, mais chaque mention est rangée sous « faites » (sous-étape du nom ≤ courante) ou « suivantes ». Sous-étape d'un nom : pour un identifiant ambigu, celle de son identité (exacte, par l'AST) ; pour un nom distinctif trouvé par texte, ou une chaîne, la **dernière** des sous-étapes de ses lignes renommées (l'AST garantit qu'il ne reste aucune occurrence d'une identité appliquée ; ce qui reste d'un nom partagé appartient à l'identité tardive). Pied de sortie : total « faites » par colonne, total « suivantes » par colonne, borne de résidu de la sous-étape. Le compteur n'échoue toujours jamais ; sans `SUBSTEP=` sa sortie est inchangée.
- Critère de fin (§6) lu sur cette sortie : « faites » à 0 en `identifier` et `string` ; « suivantes » en `identifier` ≤ borne ; `backtick` et `comment` ne comptent pas (étape 4).

### Résidus remesurés

En tranche 1, sur l'arbre pas encore renommé et après un `make rename-extract` frais, `cart-names-count SUBSTEP=` pour chacune de 3a–3k : la colonne « suivantes » `identifier` est le résidu de départ, écrit comme borne dans la table des sous-étapes. Si une valeur diffère de la colonne « Résidu » de la note de 2b ou du « Résidu ≤ N » d'une entrée 3x du §6, l'entrée du §6 est corrigée dans le même commit (la note de 2b reste un historique) et l'écart est expliqué dans « Découvert » de 3a (méthode de 2b différente, noms partagés).

### Contrôle des sites de chaînes (module neuf)

- `make rename-strings-verify SUBSTEP=3a` (nom libre, `rename_*`), hors suite : pour chaque ligne `chaîne` ou `fichier` renommée de la sous-étape (règle d'appartenance ci-dessus), à son site `fichier`, le nouveau nom est présent et l'ancien absent, avec la reconnaissance du compteur : dans un `.c`/`.h`, un littéral C exactement égal au nom (hors messages d'`assert`) ; dans un script, une donnée de `tools/` ou le `Makefile`, le nom en mot entier (chemins `tmp/` sous leurs deux écritures). Sortie : une ligne par site en défaut, puis un total ; code 1 s'il y en a un.
- Un même site peut porter l'ancien littéral pour une autre raison (même orthographe, autre sens) : le contrôle le signale, l'agent le tranche dans « Découvert », il ne l'exempte pas en silence.
- Le contrôle compare l'arbre à la table : il ne réécrit rien. Les chaînes sont éditées par l'agent, site par site (une substitution restreinte au littéral exact `"jo"` dans les seuls fichiers listés est acceptable ; jamais un remplacement de mot nu).

### Recâblage des identités (module neuf)

- `rename_*` neuf, hors suite, lancé après chaque `rename-apply` : à partir de l'extraction **d'avant** l'application (copie de `declarations.tsv`), de l'extraction **d'après** et de la tranche appliquée, il rend pour chaque ligne `identifiant` renommée non encore appliquée dont l'identité n'existe plus son identité nouvelle : la déclaration d'après de même sorte, même nom, même fichier et même ligne (renommer des tokens ne déplace aucune ligne), unique. Réécrit la seule cellule `identité` de ces lignes dans `docs/renommage.tsv`, le reste du fichier à l'octet.
- Mode contrôle : chaque identité renommée restante du TSV existe dans l'extraction fraîche, avec ce nom et ce fichier. Échoue sinon, en listant les lignes. C'est la porte de sortie de chaque sous-étape 3 ; à 3a, elle doit passer après recâblage des 834 identités à offset décalées.
- Une identité sans correspondant unique arrête la tranche : c'est un défaut de l'outil ou du TSV, corrigé dans le même commit et noté.

### Gardes à motifs : inventaire et traductions de 3a

- **Inventaire** (tranche 1, dans la note « Découvert » de 3a, une ligne par élément) : chaque motif, liste d'exemptions ou de sites de `tools/composition_residue.sh`, `tools/data_residue.sh`, `tools/composition_residue_sites.txt`, `tools/reachability_questions.txt` qui reconnaît du code par un nom du cart, avec la catégorie, le nom cité et sa sous-étape (table ci-dessus). Trois classes : nom du TSV (traduit par sa sous-étape, dans le même commit que l'application) ; motif générique en préfixe `pico8_` (`pico8_X_of`, `pico8_X_start`, `pico8_X_default`…), qu'aucune ligne du TSV ne porte ; nom historique absent du code et du TSV (code déjà retiré : `pico8_value_*`, `pico8_ob`…). Les deux dernières classes sont proposées pour décision humaine (traduction sans préfixe à la sous-étape où le premier module concerné passe, si `--detail` ne bouge pas ; sinon intraduisible) ; elles ne sont pas supprimées.
- **Traductions de 3a**, dans le commit de l'application (tranche 2) :
  - point de départ de la fermeture des types d'entité de `composition_residue.sh` (deux sites) : `pico8_struct_header_t` → `entity_t` ; la constante de test `KE_JO` de `tests/test_composition_residue.c` qui le plante suit ;
  - exclusion de `lien-parent` : `pico8_state_of` → `entity_state_of`, et son faux positif nommé dans le test ;
  - `accesseurs-q6` : les noms de champs d'en-tête traduits (`get_(nu|layer|pos|map_pos|hit_box|cursor_sprite|gpio_signal|infraspace|fold_anchor|is_dead)|set_is_hovered`) ; mesuré : aucun nom neuf du TSV ni aucune ligne actuelle n'y correspond, donc pas de faux positif introduit.
  - Chacune a son test de vivacité (violation plantée en nouveaux noms ⇒ `--check` échoue) et `composition_residue.sh --detail` / `data_residue.sh --detail` sont identiques avant et après la tranche.
- Les lignes `chaîne` correspondantes (`pico8_struct_header_t`, `pico8_state_of` sur `tools/composition_residue.sh`) sont donc de 3a par la règle 4, et passent au contrôle des sites.

### Application de 3a

- `make rename-extract` frais ; copie de l'extraction ; `make rename-slice SUBSTEP=3a` (97 identités) ; `make rename-apply TABLE=tmp/ast_rename/slice.tsv` ; traductions de garde ; `make rename-extract` ; recâblage puis son contrôle. Un seul commit avec le TSV recâblé.
- Les deux fonctions déclarées dans l'en-tête mais définies ailleurs (`pico8_jc_struct_class`, `pico8_g__struct_class`) et les `struct` avancés (`pico8_emitter`, `pico8_qy`, `pico8_kind_rec`, `pico8_probe`) appartiennent à leurs modules (3e, 3f, 3d) et ne bougent pas ici : ils font partie du résidu.

### Chaînes de 3a

- 43 identités renommées (2 conservées : `trade`, `ui`), 484 sites dans 72 fichiers : étiquettes, messages (`kj`, `ev`, `hd`, `hk`, `hb`, `ey`, `hq`, `ldown`), états (`ph`, `ly`, `ll`, `kz`, `pr`), noms de classe, et les deux symboles cités par `composition_residue.sh`. Un seul commit : un lookup par chaîne doit trouver le même nom des deux côtés.
- Recherche des registres par égalité (`strcmp`, ordre d'insertion) : aucun ordre ne dépend de l'orthographe d'une clé, d'où l'attente d'e2e identiques. Avant d'éditer, vérifier qu'aucun tampon de taille fixe ni aucune copie en zone ne dépend de la longueur d'une de ces clés (`"unfolding"` fait 9 caractères contre 2) ; un cas trouvé est noté et traité dans la tranche.
- `tools/class_coverage_exempt.txt` : nom traduit. `tools/class_coverage_net_expected.txt` : noms traduits, lignes retriées par la même commande `sort` que `tools/class_coverage.sh` dans le même environnement (les égalités de compte se départagent par nom : `fr`, `hj`, `nl`, `os`, `pm` à 1 changent d'ordre). Jamais régénéré depuis un rejeu.
- Hachages : avant l'édition, l'agent dresse la liste prévue des tests dont un hachage figé plie une chaîne de 3a (au moins les 18 fichiers qui plient `header.me` ; s'y ajoute tout pli de nom de classe, d'étiquette ou de message). Après l'édition, `make test` doit rougir **exactement** sur cette liste (sortie gardée pour le message de commit) ; chaque constante est alors remplacée par la valeur que le test affiche. Un test rouge hors liste arrête la tranche (enquête `help-me-debug`, pas de recopie).
- Les 14 clés signalées par `rename-strings` sont toutes de sorte `clé` (libellé ou dump), rangées hors de 3a par la règle 5 : `dm`, `fj`, `il`, `ja`, `lr`, `px`, `pz` (`tests/test_research_project`, 3g) ; `go`, `hm`, `jn` (3h) ; `je`, `ju`, `mo`, `ok` (`tests/test_live_slots`, 3f). La note de 3a le constate, avec la sous-étape que l'outil leur donne ; leur classement (écrivain trouvé, dynamique, morte) revient à ces sous-étapes.

### Garde de portée en compteur (§5, garde 2)

- `make scope-count [SUBSTEP=… | MODULES=…]`, hors suite, n'échoue jamais ; lit `declarations.tsv` d'une extraction fraîche (avertit comme le compteur anti-cart si elle est plus vieille que les sources). Pour chaque déclaration de nom de 1 ou 2 caractères : refusée si sa sorte est type, tag, fonction, champ, constante d'enum, macro, statique ou globale ; refusée si locale ou paramètre de portée > 40 lignes ; permise sinon. Par module du fichier de déclaration, une colonne par sorte refusée et une colonne `long-scope`.
- Exceptions : un fichier de `tools/` qui ne contient que `x` et `y` comme champs du type vecteur, écrits avec le nom actuel du type ; il cite donc `pico8_vec_t` et entre dans l'inventaire des gardes à motifs (traduit par 3c). Toute autre exception est **proposée** dans « Découvert », jamais ajoutée.
- État avant 3a, mesuré sur l'extraction de 2b : ≈ 940 refus (460 champs, 420 locales et paramètres à portée > 40, 31 statiques, 18 constantes, 7 fonctions, 1 macro). Le relevé de fin de 3a va dans la note.

### Fin de PRD

`make test-all` vert ; `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent (jamais lancés) ; `make rename-tool-test` vert ; `class-coverage-net` rouge avec exactement la sortie d'avant traduite ; `e2e-gap` rouge avec les mêmes 2 932 lignes ; après `make rename-extract` frais, `cart-names-count SUBSTEP=3a` sans ligne `warning:`, « faites » à 0 en `identifier` et `string`, « suivantes » `identifier` ≤ borne ; `rename-strings-verify SUBSTEP=3a` vert ; contrôle du recâblage vert ; `scope-count` relevé ; `data-residue` et `composition-residue` aux mêmes comptes.

## Implementation Chunks

1. **Outillage de méthode, entrée.** Condition d'entrée vérifiée. Table des sous-étapes, module d'appartenance et contrôle d'appartenance dans `make test` ; `SUBSTEP=` de `rename-slice` et `cart-names-count` avec scission ; contrôle des sites de chaînes ; outil de recâblage (testé sur mini-arbres) ; légende du `Makefile`. Résidus remesurés et écrits dans la table, §6 corrigé si besoin. Inventaire des gardes à motifs ouvert dans la note de 3a. Aucun symbole du code touché ; `make test-all` vert (les tests neufs y sont).
2. **Application des identifiants de 3a.** Tranche `SUBSTEP=3a` appliquée ; traductions de `composition_residue.sh` et tests de vivacité ; `--detail` identique ; extraction fraîche, recâblage du TSV, contrôle vert. `make test-all` vert, e2e intacts, `class-coverage-net` et `e2e-gap` à l'identique (aucune chaîne n'a bougé).
3. **Chaînes de 3a.** Liste des hachages prévue ; 484 sites et deux fichiers de couverture traduits ; rougissement observé égal à la liste, constantes recalculées ; `rename-strings-verify SUBSTEP=3a` vert ; `class-coverage-net` égal à la sortie d'avant traduite. `make test-all` vert, e2e intacts.
4. **Compteurs et note.** `scope-count` et son fichier d'exceptions, avec tests ; relevés de fin (cible par cible, liste « Fin de PRD ») ; note « Découvert » sous l'entrée 3a : résidus remesurés, recâblage (nombre de lignes, défaut de l'outil de 2a que cela corrige), inventaire des gardes avec classe et sous-étape de chaque élément, propositions d'exceptions et de traductions pour décision humaine, hachages recalculés, les 14 clés et leurs sous-étapes, modules rangés hors du §6, compteurs de fin. Plan corrigé dans le même commit si un constat le contredit.

## Testing Decisions

- Un bon test vérifie le comportement externe d'un outil sur un mini-arbre ou une mini-table (entrées → sortie, code de retour), pas sa structure interne. Prior art : `tests/test_rename_slice.py`, `tests/test_rename_count.py`, `tests/test_rename_strings.py` (tables et arbres temporaires, `subprocess`), `tests/test_composition_residue.c` (violations plantées, `--check`).
- **Table et appartenance** : un module absent, un module en double, une borne manquante font échouer le contrôle ; les cinq règles d'appartenance, une par cas (dont une chaîne de `tools/` qui suit l'identifiant cité, et une clé sans écrivain qui suit son lecteur).
- **`SUBSTEP=`** : la tranche d'une sous-étape ne contient que ses modules (`game/planet` sans `game/planet_family`) ; le compteur range une mention d'un nom fait sous « faites » et d'un nom tardif sous « suivantes », un nom partagé sous la dernière sous-étape ; sans `SUBSTEP=`, sortie identique à l'actuelle.
- **Contrôle des sites** : vert sur un arbre conforme ; rouge (test négatif) sur un site oublié, sur un ancien littéral resté à côté du nouveau, sur un chemin `tmp/` écrit `$(TMP_DIR)/…` ; un littéral dans un message d'`assert` n'est pas un site.
- **Recâblage** : une locale dont l'offset a glissé retrouve son identité ; une locale d'une fonction renommée aussi ; deux candidats ou aucun ⇒ échec sans écrire ; le TSV ne change qu'aux cellules `identité` (comparaison à l'octet du reste) ; le mode contrôle échoue sur une identité introuvable.
- **Gardes à motifs** : un test de vivacité par motif traduit, dans `tests/test_composition_residue.c` ; les tests existants restent verts.
- **Portée** : chaque sorte refusée, la limite à 40 lignes (40 permis, 41 refusé), `x`/`y` du vecteur permis, `x` champ d'un autre type refusé.
- **Code du jeu** : aucun test nouveau ; filet = compilateur, `make test`, e2e à l'octet, et rougissement prévu des hachages.
- Les tests neufs sont des cibles `*-test` dans `make test`, comme ceux de 2b ; les outils restent hors suite.

## Out of Scope

- Les noms des sous-étapes 3b–3l, même rencontrés dans les fichiers de 3a (résidu).
- Les commentaires, messages d'`assert`, mentions entre backticks et nues : étape 4 (4a pour les fichiers de 3a).
- Traduire les motifs génériques en préfixe `pico8_` ou les noms historiques des gardes : inventoriés et proposés, décidés par l'humain.
- Toute exception de garde nouvelle (portée, anti-cart) : proposée, pas ajoutée.
- Les alertes `warn-review` de fin de tronçon 2 (lecteurs en double, schéma en français, libclang fixé à deux endroits) : point d'arrêt d'avant l'étape 5.
- Réparer les références périmées de `class-coverage-net` et `e2e-gap` : tâche séparée, hors plan 5.
- Rendre une garde bloquante : étape 5.
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD (arbre `7bf3d4f`, extraction de 2b) : 97 identités de 3a, 4 134 occurrences AST dans 120 fichiers ; 43 + 2 identités chaîne de 3a, 484 sites renommés dans 72 fichiers ; 1 006 identités à offset dans les sous-étapes suivantes, dont 834 dans un fichier où 3a réécrit un token plus haut ; aucune fonction renommée par 3a n'apparaît dans l'USR d'une identité restante.
- Ordre de travail recommandé pour chaque sous-étape suivante, à confirmer par la note de 3a : `rename-extract` → copie → `rename-slice SUBSTEP=` → `rename-apply` → traductions de garde → `rename-extract` → recâblage et contrôle → chaînes et `rename-strings-verify` → `cart-names-count SUBSTEP=` et `scope-count`.
- Durées : chaque `rename-extract` ou `rename-apply` ≈ 1 min 30 ; la tranche 2 en enchaîne trois.
