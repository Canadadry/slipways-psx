---
title: "Plan 3, étape 2.9 : étude A en lecture seule, règle 1 et archétypes. Preuve compilateur `const` champ par champ (282 champs de `--champs` + 20 des composants émetteur et sonde), classement archétype / état avec ses écritures, forme, place et chemin de lecture des archétypes, test ponctuel de la règle 3 classe par classe, liste ordonnée de 2.12 et 2.13 réécrite dans le plan. Trois sous-étapes, aucun code changé"
description: "L'étude qui fixe, avant 2.12, quels champs quittent l'instance pour un archétype `static const` et lesquels restent de l'état. La preuve compilateur se fait sur une copie jetable : chaque champ est rendu `const`, `src/` et `tests/` sont compilés, et les erreurs donnent la liste de ses écritures. Mesuré sur `51bb145` : 302 champs en 4 min 15 s, 919 erreurs, pré-classement 63 archétypes probables, 58 constantes d'instance, 157 champs d'état. Produit `docs/etude-2-9-archetypes.md` et réécrit les lignes 2.12 et 2.13 du plan (annotations sur 2.14 à 2.18)."
status: needs-triage
---

## Problem Statement

**Écrit sur `51bb145`** (branche `claude/plan-pilot-3-7zdft0`, 2.8 en triage, **pas encore appliquée**). Pendant l'écriture, 2.8a a été commise (`30582c5`) et 2.8b était en cours dans l'arbre de travail. Ce PRD n'en a rien lu. Aucune ligne du dépôt n'a changé pour l'écrire. Les essais ont été faits sur une seule copie jetable de `51bb145` (`git clone` local dans le scratchpad), supprimée depuis. **Les numéros de ligne sont ceux de `51bb145`.** 2.8 réécrit des lignes dans `trade.h`, `planet.h`, `ui.h`, `wormhole.h` et leurs `.c`, donc l'agent retrouve chaque site par son texte.

Du point de vue du mainteneur du port : la phase 2 veut que chaque ancienne classe ait un **archétype** `static const`, qui porte ses défauts résolus le long de sa chaîne, et que l'instance ne garde que son **état**. La règle 1 tranche champ par champ : un champ va dans l'archétype si aucun code ne l'écrit sur une instance. L'humain a accepté la preuve par le compilateur (Q3). Les étapes 2.12 (archétypes et test ponctuel) et 2.13 (l'init lit l'archétype, les `pico8_X_start` disparaissent) sont **provisoires** dans le plan : leurs sous-étapes et leurs comptes attendent cette étude, comme D.10 à D.12 attendaient D.9.

Le relevé par nom de `--champs` ne suffit pas : il compte **par nom**, pas par type. Par exemple, `fa` compte 20 écritures pour `bg`, `fq`, `ie`… sans dire lesquelles touchent quel type (`composition_residue.sh:23-31`, « un majorant »). Seule la preuve `const`, faite sur un membre d'un type, donne les écritures de ce type.

### Texte du plan, recopié (`docs/plan-3.md` à `51bb145`, ligne 2.9)

> - 2.9 — **étude A, lecture seule : règle 1 et archétypes** (sur le modèle de D.14). Pour chaque champ des types d'entité (`--champs`, ≈ 280 après 2.8), la preuve compilateur `const` (Q3) : archétype ou état, écritures listées ; un champ qu'aucune règle ne classe est posé à l'humain. Puis : la forme des archétypes (`static const`, résolus le long de la chaîne, `NL_DEF = { .hp=6, .s=68, … }`), leur place (`game_data.h`, 574 lignes, ou le module de la classe) sans nouvelle paire d'inclusion, le chemin par lequel une instance lit un champ constant (table de classe), la part copiée dans l'état à l'init ; le test ponctuel de la règle 3, classe par classe, où les **deux groupes de Q7** sont nommés comme devant le passer : émetteur (`pn`, `du`, `py`, `ki`, `pk`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`, `qy`) pour `g_`, `fv`, `hj`, `pm`, `mx`, `ga` ; sonde (`et`, `dw`, `od`, `dq`, `fp`, `ik`) pour `g_`, `fv`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`, `mz`, `jc` ; enfin la liste ordonnée de 2.12 et 2.13 avec leurs comptes `défauts`. Sortie : `docs/prd/done/plan3-phase2-etape9-etude-archetypes.md`. Questions à l'humain en français simple, sans noms minifiés ni numéros de TROU, 100 mots au plus chacune. Compte : aucun. Budget ≈ 50 min (relevé scripté sur copie jetable).

Règles de la phase qui s'appliquent (recopiées) :

> 1. **Archétype ou état** : un champ va dans l'archétype si aucun code ne l'écrit sur une instance (prouvé par grep, ou par le compilateur : le champ rendu `const` dans une copie jetable, les erreurs de compilation listent ses écritures — Q3) ; sinon dans l'état mutable de la struct. L'état partagé de classe (`ie.gy`, `ie.id`) reste statique.
> 2. **Composant** : un groupe de champs devient un composant s'il apparaît dans **deux branches distinctes** de l'arbre […].
> 3. Test ponctuel, supprimé en fin de phase : pour chaque ancienne classe, chaque champ résolu par sa chaîne est égal à la valeur de l'archétype correspondant (piège connu : le `nu=o()` vide de `ga` masque celui de `jo`).
>
> **règle 1 (Q3)** [preuves communes] : tout champ qui quitte l'instance pour une table de classe ou un archétype a sa preuve compilateur — le champ rendu `const` dans une copie jetable, `src/` et `tests/` compilés, chaque erreur listée ; seules les écritures des `*_new`/`*_init` sont admises, toute autre est un arrêt « champ sans règle ».

Décisions de l'humain qui cadrent l'étude : **Q3** (preuve compilateur acceptée) ; **question A du 2026-09-30** (un seul type « planète » pour les dix sortes, la sorte n'étant plus que sa table de méthodes et son archétype ; de même un seul type pour la sonde et sa variante d'aide). 2.9 ne fait pas l'aplatissement, c'est le travail de 2.15. Mais la forme des archétypes qu'elle choisit doit déjà être celle que A impose : **un type d'archétype par futur type aplati, une valeur `static const` par sorte**.

### État de départ attendu (après 2.8c) — hypothèses `H-2.8-k`

L'étude part de l'arbre d'après 2.8. Chaque hypothèse est à revérifier dès que la note « Découvert à 2.8c » existe :

- **H-2.8-1** : `make composition-residue` rend `parente` 10, `lien-parent` 111, `accesseurs-q6` 237, `défauts` 669, `void-ptr` 8, `inclusion-mutuelle` 17 / 16 (43 lignes), les sept autres à 0 et gardées, et le pied **`1073 / 85`**. La garde finit par `,ptrs-entité`.
- **H-2.8-2** : `--champs` rend **`champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27`**, inchangé par 2.8 (PRD 2.8, preuve 7). Mesuré à `51bb145` : 282 / 15 / 27, sur 36 types.
- **H-2.8-3** : les sept membres de liste (`trade.lp`/`rl`, `jo.oe`/`cc`/`by`, `ui.le`, `mz.ox`) gardent leur nom. Seul leur type change (`pico8_ents_t *`). Le banc ne dépend pas du type.
- **H-2.8-4** : le module neuf `pico8_ents` n'a aucun type d'entité et ne touche aucun champ. Le banc le compile avec le reste de `src/`.
- **H-2.8-5** : `pico8_game_data.h` compte **540 lignes** (mesuré à `51bb145` ; le plan dit 574, chiffre d'avant 2.4–2.7), et 2.8 ne le touche pas.

À revoir si une hypothèse tombe : « État de départ », le tableau des chiffres de pré-classement (§ Mesures), le tableau « Runs ».

## Solution

Une étude en trois sous-étapes, un commit de documentation chacune. Elle produit :

1. **`docs/etude-2-9-archetypes.md`**, le livrable (voir G1 pour le chemin), qui contient :
   - §1, le **classement** : une ligne par champ (302), avec sa classe, ses écritures, sa preuve et son verdict ;
   - §2, les **archétypes** : quelles classes en reçoivent un, leur forme, leur place, le chemin de lecture et la part copiée à l'init ;
   - §3, le **test ponctuel** de la règle 3, classe par classe ;
   - §4, la **liste ordonnée** de 2.12 et 2.13 avec leurs comptes `défauts` ;
   - §5, les questions à l'humain (s'il y en a) ;
   - en annexe, le banc et le tableau « Runs ».
2. **`docs/plan-3.md`** mis à jour : lignes 2.12 et 2.13 réécrites (sous-étapes, familles, comptes, preuves, budgets) ; 2.14 à 2.18 annotées là où 2.9 change quelque chose (voir « Plan ») ; note « Découvert à 2.9 » sur la ligne 2.9 ; ligne d'estimation mise à jour.

Aucun fichier de `src/`, `tests/`, `tools/` ou du `Makefile` ne change. `make composition-residue` rend le même pied avant et après.

## User Stories

1. En tant que pilote de 2.12, je veux la liste des champs de chaque archétype, avec pour chacun la valeur par classe et la ligne qui la pose aujourd'hui, pour écrire l'archétype sans relire la chaîne.
2. En tant que pilote de 2.12, je veux, pour chaque classe, les champs que le test ponctuel compare, groupes émetteur et sonde compris, et les masquages (un défaut propre qui recouvre celui d'une ascendante), pour choisir les deux mutations par famille.
3. En tant que pilote de 2.13, je veux la part de chaque archétype que l'init copie dans l'état, et les sites d'écriture qui rendent ces champs mutables, pour réécrire l'init sans perdre une écriture.
4. En tant que vérificateur, je veux que chaque champ de `--champs` et des deux composants soit classé avec la preuve `const` qui le fonde, ou posé à l'humain, pour qu'aucun champ ne passe à l'archétype sur la foi d'un grep.
5. En tant que pilote de 2.10 et 2.15, je veux savoir quels champs lus par les accesseurs Q6 sont constants par classe, pour qu'un accesseur lu sur l'archétype ne soit pas refait à l'aplatissement.
6. En tant qu'humain, je veux au plus quelques questions, en français simple, seulement là où aucune règle ne tranche.

## Implementation Decisions

### Les classes du classement (§1 de l'étude)

Chaque couple (type, champ) reçoit une classe et une seule :

| Classe | Définition | Devient |
|---|---|---|
| **A** — archétype | Aucune écriture dans `src/` hors construction. Chaque écriture de construction pose une **constante de classe** : un défaut `pico8_X_start.m`, un littéral, l'adresse d'un objet statique. La valeur est la même pour toutes les instances de la classe. | champ de l'archétype, lu par la table de classe (§2) |
| **I** — état initialisé par l'archétype | La construction pose un défaut de classe, puis le champ est réécrit : hors construction (`pq`, méthode, `set_m_`, pointeur qui s'échappe), ou dans le `_new` par une clé de construction. | reste dans l'instance ; son défaut est dans l'archétype et l'init l'y copie (« part copiée ») |
| **K** — constante d'instance | Écrit seulement pendant la construction, mais avec une valeur propre à l'instance : clé (`keys->id`), argument, allocation, adresse d'un membre de l'instance (`&self->emitter_own`). | reste dans l'instance, sans défaut dans l'archétype |
| **E** — état | Pas de défaut de classe (nil, 0, `NULL` posés à l'init) et réécrit ensuite. | reste dans l'instance |
| **S** — statique de classe | État partagé par toutes les instances (règle 1, dernière phrase : `ie.gy`, `ie.id`) : membre qui pointe un objet statique réécrit en place (`jo.kw` → `g_jo_kw_shared`), ou variable de module. | reste statique |
| **H** — à l'humain | Aucune des classes ci-dessus ne tranche, ou le classement exigerait de changer l'intention d'un test de régression (§6 du plan). | question du §5 |

Une ligne du tableau de l'étude contient : type, champ, classe, écritures `src/` de construction (fichier:ligne, fonction, texte), écritures `src/` hors construction, écritures `tests/`, et le verdict en une phrase quand la classe n'est pas mécanique.

**Ce qui compte comme construction** (lecture de la règle 1, à écrire dans la note « Découvert à 2.9 ») : une fonction `*_new`/`*_init`, ou une aide `static` dont **tous** les appelants sont des `*_new`/`*_init`. Aides de ce genre relevées à `51bb145` : `pico8_lq_struct_alloc`, `pico8_jb_struct_alloc`, `hp_positions`, `pico8_lq_copy`, `pico8_ie_ke_view` (l'étude vérifie leurs appelants). Les `pq` (`pico8_jo_pq`, `pico8_js_pq`, `pico8_jc_pq`, `pico8_lq_pq`) sont appelés par les `_new`, mais ce sont les méthodes du cart qui suivent la construction. Ils écrivent de l'état. **Une écriture dans un `pq` classe le champ I ou E, jamais A.**

**Écriture par un TU sur un champ A** : le verdict se fonde sur `src/`. Les écritures de TU sont listées, avec la façon dont 2.13 réécrira chaque TU (construire une autre classe dont l'archétype porte la valeur voulue, ou lire l'archétype). Si c'est un **test de régression de bug** (§6 du plan : `gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) et que la réécriture change ce que le test prouve, le champ est classé **H**. Cas mesuré : `jo.gq`, pré-classé A, est écrit par `test_pico8_planet_gh_row_lq.c:121` et `test_pico8_ga_export_panel_by.c:112`, plus trois tests de `test_pico8_lq.c`. L'étude doit le trancher par écrit.

### Le banc de preuve (§1, exécuté à 2.9a)

- **Copie** : une seule copie jetable de l'arbre d'après 2.8c dans le scratchpad (`git clone` local ou `git archive`), jamais l'arbre de travail : un implémenteur peut y travailler en parallèle. La copie est supprimée à la fin de 2.9c. Les journaux restent dans le scratchpad.
- **Liste des champs** : `sh tools/composition_residue.sh --champs`, puis les lignes à quatre colonnes (282 attendues, H-2.8-2), **plus les 20 champs des deux composants**, que `--champs` ne lit pas parce que `pico8_emitter_t`/`pico8_probe_t` ne sont pas des types d'entité : émetteur `pn`, `du`, `py`, `ki`, `pk`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`, `qy` (`particle.h:31-44`), sonde `et`, `dw`, `od`, `has_od`, `dq`, `fp`, `ik`, `has_ik` (`probe.h:24-33`). Soit **302 champs**.
- **Preuve, un champ à la fois** : dans l'en-tête qui déclare le type, insérer `const` **juste avant le nom du membre** (`int const hf;`, `const pico8_qz_box_t *const kq;`, `char const x[4];`). Puis compiler **tous** les `.c` de `src/` et `tests/` en `cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only`. Les deux fichiers `class_coverage` se compilent avec `-DPICO8_CLASS_COVERAGE`. Remettre l'en-tête d'origine, puis passer au champ suivant. Chaque ligne `error:` est une écriture, ou une fuite d'adresse, de ce type-là. Le texte exact essayé est en annexe A.
- **Pourquoi un champ à la fois** : rendre `const` tous les champs d'un type en une passe a été mesuré sur `jo`. On obtient 159 erreurs en 0,6 s, mais deux champs diffèrent de la passe champ par champ : `kw_own` donne 1 site au lieu de 5, `ma` 2 au lieu de 3. Des erreurs sans nom de membre (sous-objet, adresse) ne se rattachent plus à leur champ. La passe groupée ne sert que de contrôle croisé.
- **Fichiers non compilés** : `main_raylib.c`, `main_screencast_viewer.c`, `pico8_raylib_backend.c` (sans `raylib.h` dans la copie). Ils ne nomment aucun type d'entité (grep à `51bb145` : `replay_source`, `mem`, `gfx`, `cheat`, `backend`…). L'étude refait ce grep et le cite.

**Pièges mesurés** (chacun a produit une erreur ou un faux résultat pendant les essais) :

1. **`const T *m` ne rend pas le membre constant**, seulement ce qu'il pointe. Il faut `T *const m`. Une première passe qui préfixait la ligne ne voyait aucune écriture des pointeurs.
2. **Éditer la ligne par motif, pas par numéro.** Un `sed '28s/…/'` a visé `hs` au lieu de `hf` : 13 erreurs sans rapport, toutes de fichiers déjà en échec sans leurs options.
3. **Messages à rattacher au champ.** `assignment of read-only member` (725 sur les 792 des 282 champs) ; `assignment of member 'x' in read-only object` (écriture dans un sous-objet : `kw_own.qe`, `.mu`, `.has_qe`) ; `assignment of read-only location` (tableaux : `hp.lq_inner_items[0]`, `mh.items[…]`) ; `increment of read-only member` (`lq.items_count++`, `mh.items_count++`) ; `assignment of read-only variable 'view'` (**affectation de struct entière**, `research_icon.c:171` et `test_pico8_ie.c:672`, `:690` : elle tombe sur chaque champ de `pico8_ie_ke_t` et de l'en-tête) ; `discards 'const' qualifier` (**fuite d'adresse**, pas une écriture : `&self->emitter_own`, `&self->probe_own`, `pico8_ma` à `entity_geom.c:79` qui rend `&jo->ma`/`&qb->ma`, `pico8_ptrs_init(&ui->ic_own)`, `jc_mt_of(…, &self->mt)`). Une fuite d'adresse se suit jusqu'aux écritures faites par ce pointeur. Sinon le champ est **H**.
4. **Ce que `const` ne voit pas** : les initialiseurs (`static T x = { .m = … }`, `(T){…}` en déclaration), qui ne sont pas des affectations. Quatre champs n'ont aucune erreur pour cette raison : `jc_too_far.qs`, `mz_jj.qs`, `pj_cr.gq`, `pj_cr.hi`, tous des instances statiques. Ne le voient pas non plus : `memset`/`memcpy`/`calloc` sur une entité et les écritures par un `void *`. L'étude fait `grep -nE 'memset|memcpy|calloc' src/*.c tests/*.c` et classe chaque site qui touche un type d'entité. À `51bb145`, les trois `memset` proches (`help.c:55`, `menu.c:356`, `mouse_reader.c:62-63`) visent des clés `pico8_js_keys_t` et les boutons statiques de `nf`, pas des champs d'instance de la liste. L'étude le revérifie.
5. **Un membre enchâssé propage le `const`** : `jo.hf` constant rend fautives les écritures `self->jo.hf` des inits de `qh`, `ga`… C'est voulu, c'est ainsi que la chaîne apparaît. La classe (A/I…) se donne **par couple (type déclarant, champ)**, et la valeur **par classe concrète** (§2).

### Archétypes (§2, rédigé à 2.9b)

L'étude écrit, avec mesures à l'appui :

1. **Inventaire des classes** : les **40 tables de classe** `static const pico8_struct_class_t g_X_struct_class` (dont 13 `= {0}` : `hy`, `mo`/`je`/`fm` remplaçants, `bv_et`, `ha_pz`, `hp`, `ie_ke_view`, `pj_cr`, `nr_ldown_empty`, `jc_too_far`, `trade`, `mz_jj`), les **27 `pico8_X_start_t`** (`game_data.h:117-423`, dont `ke` et `pj_cr` que la ligne 2.12 provisoire ne nomme pas) et les **36 types** de `--champs`. Pour chacun : archétype oui ou non, avec la raison (pas de défaut, instance statique unique, remplaçant, vue…). **Critère** : chaque `pico8_X_start_t` a un archétype ou une raison écrite de disparaître autrement.
2. **Forme** : `static const`, valeurs **résolues** le long de la chaîne (l'init parente puis les défauts propres : `NL_DEF` porte les défauts de `jo`, puis `qh`, puis `nl`). Sous la décision A : **un type d'archétype par futur type aplati** (planète : `jo` et ses neuf sortes ; sonde : `g_` et `fv`), une valeur par classe concrète. Pour les autres racines : un type par classe. L'étude donne le texte C d'**un** archétype complet (`nl` recommandé : trois niveaux de chaîne, groupe sonde) et la table champ → valeur → ligne source pour toutes les classes.
3. **Place** : dans le module de la classe (`planet_family.c`…) ou dans `game_data.h/.c`. Critère mesuré : **aucune nouvelle paire d'inclusion**. Preuve : sur la copie, écrire l'archétype de `nl` (et celui de `bg`, racine simple) à la place retenue, compiler, relancer `composition_residue.sh`. `inclusion-mutuelle` doit rester **17 paires / 43 lignes** (H-2.8-1). Rappel de la dépendance du plan : un archétype qui nommerait un type du noyau depuis `game_data.h` recrée la paire `game_data` ↔ `struct`, que 2.2 a coupée.
4. **Chemin de lecture** : comment une instance lit un champ A « par la table de classe ». Aujourd'hui, `pico8_struct_class_t` (`struct.h:115-155`) n'a pas de membre d'archétype, et ses types diffèrent par famille. L'étude compare au moins :
   - (i) une struct de classe par famille qui enchâsse `pico8_struct_class_t` en premier membre et porte l'archétype typé. On descend depuis `header.cls` par un cast. Vérifier que le lecteur de structs du compteur ne la prend pas pour une entité : elle ne descend pas de l'en-tête, donc `parente` inchangé ;
   - (ii) un pointeur d'archétype typé, posé dans l'instance à l'init ;
   - (iii) un accesseur `static` par module qui rend l'archétype de `self->header.cls`.

   Critères : aucun `void *` nouveau dans un prototype public (`void-ptr`), aucune paire neuve, aucun `parente`/`lien-parent` neuf, coût pour 2.14 (accesseurs Q6 qui liraient l'archétype) et pour 2.15 (aplatissement). L'étude retient une option et la justifie. Elle ne pose la question à l'humain que si deux options restent à égalité sur ces critères.
5. **Part copiée à l'init** : pour chaque classe, les champs I, avec leur défaut dans l'archétype et l'écriture qui les rend mutables.

### Test ponctuel de la règle 3 (§3, rédigé à 2.9c)

Pour chaque classe qui reçoit un archétype, l'étude donne :

- la **liste des champs comparés** (A et I) : valeur par la chaîne (`pico8_X_struct_init` sur une struct allouée, sans `pq` ni `lt`), contre la valeur de l'archétype ;
- les **groupes Q7**, nommés comme le plan l'exige : émetteur (`pn`, `du`, `py`, `ki`, `pk`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`, `qy`) pour `g_`, `fv`, `hj`, `pm`, `mx`, `ga` ; sonde (`et`, `dw`, `od`, `dq`, `fp`, `ik`) pour `g_`, `fv`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`, `mz`, `jc`. Pour chaque membre de ces groupes, sa classe (mesuré sur `51bb145` : tout le groupe émetteur n'est écrit qu'à la construction, sauf `qy`, écrit hors construction ; `pn`/`du`/`py` de `g_` sont réécrits par clé dans `pico8_g__struct_new`, donc I pour `g_` ; sonde : `et`, `od`, `has_od` écrits hors construction, `dw` écrit par un TU) ;
- les **masquages** : chaque champ dont une classe fille pose une valeur autre que son ascendante (exemple de même nature que le piège `nu` de `ga`, qui n'existe plus depuis 2.7 : `hf` vaut 1 pour `jo` et faux pour `qh` et `ga`). Ce sont les cibles de la mutation « défaut hérité masqué » de 2.12 ;
- le **point d'accroche** : TU existant (`jo_spec_defaults`, `spec_defaults`, `entity_kind`, ou TU de la classe) ou TU neuf, et la preuve qu'il mord (deux mutations par famille, comme 2.12 le demande).

### Plan (§4 de l'étude et `docs/plan-3.md`, 2.9c)

- **Liste ordonnée de 2.12 et 2.13** : familles (celles du plan provisoire, corrigées par l'inventaire : `ke`, `pj_cr`), sous-étapes à ≈ 50 min, et compte `défauts` d'arrivée de chaque sous-étape, calculé sur `composition_residue.sh --check défauts` groupé par famille (669 lignes à `51bb145` ; les plus gros fichiers sont `test_pico8_jo_spec_defaults.c` 178, `test_pico8_spec_defaults.c` 76, `planet_family.c` 74, `game_data.h` 56, `test_pico8_entity_kind.c` 31, `game_data.c` 30, `planet_family_trade.c` 29, `help.c` 22). Pour chaque sous-étape : modules, preuves (preuve `const` de chaque champ d'archétype, test ponctuel rouge sous deux mutations), `class-coverage-net`/`sanitize-net` et budget, sur le modèle des lignes 2.4 à 2.8.
- **Réécriture dans `docs/plan-3.md`** : lignes 2.12 et 2.13 remplacées ; 2.14 à 2.18 gardées, avec une incise « (2.9 : …) » là où l'étude change un fait. Exemples : les accesseurs Q6 dont le champ est A (`get_kq`, `get_ql`, `get_fg`, `get_mt`…) liront l'archétype ; un champ K/I qui empêche une lecture d'archétype. Note **« Découvert à 2.9 »** sur la ligne 2.9 : ce qui change une étape suivante (lecture « construction » de la règle 1, champs H tranchés, forme et chemin retenus, chemin du livrable). Ligne « Estimation » mise à jour. **Rien d'autre du plan ne change.**

### Mesures faites pour ce PRD (sur `51bb145`)

Pré-classement **automatique** par le banc (annexe A) et un script de tri. Il ne remplace pas la lecture : il chiffre le travail. Règles du tri : « construction » = nom en `_new`/`_init` ; « constante » = partie droite `_start.`, `NULL`, un entier, `pico8_fixed_from_int(n)`, `&g_…`, `PICO8_…`.

| Pré-classe | Champs | Exemples |
|---|---|---|
| A probable | **61** | `jo.hf`, `hs`, `fa`, `kq`, `ka`, `dz`, `cq`, `ca`, `em` ; `qh.ba`, `hp`, `px`, `qh` ; tout `fv` sauf `m_`/`has_m_` ; `bg.fa`/`hd`/`pl` ; `ui.fa`/`hw`/`ql` ; `jc.k_` ; `mz.fa`/`kk`/`jj`/`spawn_snd` |
| A probable, écrit par un TU | **2** | `g_.fa` (`test_pico8_g_.c:751`) ; `jo.gq` (cinq TU, dont deux de régression, voir plus haut) |
| construction seule, valeur d'instance (K probable) | **58** | clés de `js` (17), `trade` (7), `hp` (`cw`, `go`, `ii`, `rn`, `rr`…), `emitter`/`probe` = `&self->…_own`, `jo.id` |
| écrit hors construction (I ou E) | **157** | dont **19** posés depuis un défaut `_start` (I probable) : `jo.ma`, `mi`, `kw_own`, `qh.s`, `nl.qw`, `qb.p`/`ra`/`v`/`ma`, `jc.fa`/`nh`, `g_.ra`, `ie.ra`, `iv.l`, `mx.ht`, `mz.oi`, `pj.fa`, en-tête `me`/`t` |
| aucune erreur (initialiseur statique) | **4** | `jc_too_far.qs`, `mz_jj.qs`, `pj_cr.gq`, `pj_cr.hi` |
| composants (20) | 20 | émetteur : 11 construction seule, `qy` hors construction ; sonde : `et`, `od`, `has_od` hors construction, `dw` écrit par un TU |

Le pré-classement fait **deux erreurs connues**, que l'étude corrige à la lecture. (a) Les aides de construction non nommées `_new`/`_init` (`pico8_lq_struct_alloc` 14 champs, `hp_positions` 11, `pico8_jb_struct_alloc` 3, `pico8_lq_copy` 8, `pico8_ie_ke_view` 7) poussent des champs K vers « hors construction ». (b) Un `= 0`/`NULL` constant dans l'init range en « A » des champs que personne ne réécrit (par exemple `hp.has_ii`, `has_rn`) : ce sont des constantes, ou des champs morts, à trancher à la lecture.

Écritures relevées : **919 lignes d'erreur** (792 pour les 282 champs, dont 632 dans `src/` et 156 dans `tests/` ; 127 pour les composants). Temps : **240 s** pour 282 champs et **14,5 s** pour 20, soit ≈ 0,85 s par champ sur 4 cœurs.

## Implementation Chunks

Trois sous-étapes, un commit chacune, ≈ 50 min. Le plan donnait ≈ 50 min pour toute l'étude : **écart déclaré**, voir « Surprises ».

1. **2.9a — relevé et classement (§1)**. Revérifier H-2.8-1 à 5. Monter la copie et le banc (annexe A), passer les 302 champs (≈ 5 min), rattacher les erreurs sans nom (pièges 3 et 4), faire le grep `memset`/`memcpy`/`calloc`, lire les appelants des aides de construction, classer chaque champ A/I/K/E/S/H. Tableau complet dans l'étude (annexe si plus de 100 lignes, §1 en résumé par type), contrôles de complétude 1 à 4. Commit : `docs(etude): 2.9a …`.
2. **2.9b — archétypes (§2)**. Inventaire des 40 tables, 27 `_start_t` et 36 types ; forme selon la décision A ; valeurs résolues par classe (table champ → valeur → ligne source) ; place et chemin de lecture, avec l'essai d'inclusion sur copie (`nl`, `bg`) et le compteur relancé ; part copiée à l'init. Commit : `docs(etude): 2.9b …`.
3. **2.9c — règle 3, liste ordonnée, plan (§3, §4, §5)**. Test ponctuel par classe, groupes Q7 et masquages ; familles et sous-étapes de 2.12 et 2.13, comptes `défauts` ; réécriture de 2.12 et 2.13, incises sur 2.14 à 2.18, note « Découvert à 2.9 », estimation ; questions à l'humain ; tableau « Runs » ; copie supprimée. Commit : `docs(plan): 2.9c …`.

**Repli** (si une sous-étape dépasse 1,5 fois son budget avant son commit) :
- 2.9a en deux : **2.9a1**, banc et tableau brut des 302 champs (écritures seulement, sans classe) ; **2.9a2**, classement et verdicts ;
- 2.9c en deux : **2.9c1**, §3 et §4 dans l'étude ; **2.9c2**, réécriture du plan et note.

Le repli est écrit dans la note « Découvert à 2.9 ».

## Testing Decisions

Aucun code ne change, donc pas de `gcov` ni de mutation. Les preuves portent sur la complétude et sur la reproductibilité du relevé.

### Contrôles de complétude (écrits dans l'étude, avec leur sortie)

1. **Champs** : le nombre de lignes du tableau §1 = lignes de `--champs` (282, H-2.8-2) + 20 des composants = **302**. `comm -3` entre la liste (type, champ) de `--champs` et celle du tableau → **vide**. Les comptes par type sont égaux à ceux de `--champs` (36 types ; `jo` 37, `js` 26, `hp` 25, `jc` 15, `hy` 15…).
2. **Preuve** : chaque champ classé A, I ou S a sa preuve `const` : sites listés, ou « 0 erreur » avec la raison (piège 4). Aucun champ n'est classé sur un grep seul.
3. **Aucune écriture sans classe** : chaque ligne d'erreur du banc est rattachée à un champ et à un site du tableau. Le total des sites du tableau est égal au nombre de lignes `error:` du banc (919 à `51bb145`, à remesurer), fuites d'adresse et affectations de struct entière comprises.
4. **H** : chaque champ H est une question du §5, ou il est tranché dans l'étude avec sa raison. Ce contrôle vaut aussi pour le cas `jo.gq` des tests de régression.
5. **Classes** : chaque `pico8_X_start_t` (27) et chaque table de classe (40) apparaît dans l'inventaire du §2 avec « archétype » ou sa raison.
6. **Q7** : les 12 membres émetteur et les 6 membres sonde nommés par le plan apparaissent dans le §3 pour **chacune** des classes que le plan leur associe (6 et 12 classes).
7. **Inclusions** : sur la copie, avec l'archétype de `nl` et celui de `bg` écrits à la place retenue, `composition_residue.sh` rend `inclusion-mutuelle` **17 / 16** (43 lignes) et aucune catégorie en hausse, sauf `défauts` si l'archétype nomme un `pico8_X_start` (à chiffrer).
8. **Arbre réel intact** : `git diff --stat <parent>` ne montre que `docs/etude-2-9-archetypes.md` et, à 2.9c, `docs/plan-3.md`. `make composition-residue` rend le même pied qu'au départ (1073 / 85). `make test` n'est pas exigé (aucun code), mais il est recommandé une fois à 2.9c.

### Budget et tableau « Runs »

Budget : **3 × ≈ 50 min**. Au-delà de 1,5 fois, repli. Jamais l'appli raylib ; `make raylib` n'est pas exigé. `timeout` sur toute commande longue (le banc : `timeout 900`). Pour attendre un processus de fond, attendre son PID, jamais `pgrep -f`.

L'agent de l'étude tient un tableau « Runs » en annexe de l'étude. Voici celui des essais de ce PRD, sur une copie de `51bb145` :

| # | Commande | Code | Ligne clé | Durée |
|---|---|---|---|---|
| 1 | `sh tools/composition_residue.sh` | 0 | `résidu 2 : 1083 ligne(s), 88 fichier(s)` ; `ptrs-entité 7 4`, `void-ptr 11 7`, `inclusion-mutuelle 17 16` | 0,8 s |
| 2 | `composition_residue.sh --champs` | 0 | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27` ; 36 types | 0,3 s |
| 3 | `-fsyntax-only` de 150 `.c` + 2 sous `-DPICO8_CLASS_COVERAGE` (arbre intact) | 0 | aucune erreur ; 3 fichiers raylib exclus | 0,7 s |
| 4 | `jo.hf` rendu `const` (premier essai par numéro de ligne : `hs` visé, 13 erreurs hors sujet) | 1 | 3 erreurs : `planet.c:138` (`pico8_jo_struct_init`), `planet_family.c:80` (`pico8_qh_struct_init`), `:587` (`pico8_ga_struct_new`) | 0,7 s |
| 5 | tous les membres de `jo` `const` d'un coup (premier essai `const T *` : pointeurs non protégés, refait en `T *const`) | 1 | 159 erreurs ; écarts avec la passe champ par champ sur `kw_own` (1 contre 5) et `ma` (2 contre 3) | 0,6 s |
| 6 | les 37 membres de `jo`, un par un (boucle shell) | 1 | 35 champs sur 37 identiques à la passe groupée | 28 s |
| 7 | banc Python (annexe A), 282 champs (premier lancement : chemin relatif après `chdir`, refait) | 0 | 792 lignes d'erreur, 0 champ introuvable, 4 champs à 0 erreur | 240 s |
| 8 | banc, 20 champs des composants | 0 | 127 lignes d'erreur | 14,5 s |
| 9 | tri automatique (pré-classes) | 0 | 61 / 2 / 58 / 157 (19 depuis un défaut) / 4 | < 1 s |
| 10 | `grep memset\|memcpy\|calloc` ; grep des types dans les trois fichiers raylib | 0 | aucune entité touchée | < 1 s |
| 11 | `git status --short` de la copie après chaque passe | 0 | vide | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas 2.8c vérifiée (pied 1073 / 85, garde finie par `,ptrs-entité`, `--champs` 282 / 15 / 27) et que l'étude ne l'explique pas.
- `FAILED:` si un fichier hors de `docs/etude-2-9-archetypes.md` et `docs/plan-3.md` change (le déplacement de ce PRD par le pilote excepté), ou si un essai a été fait dans l'arbre de travail plutôt que dans la copie.
- `FAILED:` si le contrôle 1 n'est pas vide (un champ de `--champs` ou des deux composants manque au tableau, ou le compte par type diffère).
- `FAILED:` si un champ est classé A, I ou S sans sa preuve `const` (contrôle 2), ou si le contrôle 3 ne boucle pas.
- `FAILED:` si un champ classé A a, dans `src/`, une écriture hors construction (au sens défini ici), ou une fuite d'adresse non suivie.
- `FAILED:` si un `pico8_X_start_t` ou une table de classe manque à l'inventaire (contrôle 5), ou si un membre Q7 manque pour une classe nommée (contrôle 6).
- `FAILED:` si la place retenue n'a pas son essai d'inclusion sur copie, ou si cet essai montre une paire neuve (contrôle 7).
- `FAILED:` si la forme des archétypes contredit la décision A (un type par sorte de planète, ou deux types pour la sonde et sa variante), sans question à l'humain.
- `FAILED:` si 2.12 ou 2.13 n'a pas, dans le plan, ses sous-étapes, ses comptes `défauts` d'arrivée, ses preuves et ses budgets, ou si le plan change ailleurs que sur 2.9 (note), 2.12 à 2.18 et la ligne d'estimation.
- `FAILED:` si une question à l'humain dépasse 100 mots, emploie un nom minifié ou un numéro de TROU, ou porte sur un champ qu'une règle tranche.
- `FAILED:` si le tableau « Runs » manque, ou si la copie n'est pas supprimée à la fin de 2.9c.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Toute modification de code, de TU, d'outil ou du `Makefile`. Aucun archétype n'est commis : c'est 2.12.
- **Règle 2** (composants), accesseurs Q6, lien parent et forme de l'aplatissement : étude B (2.10). 2.9 ne fait qu'annoter 2.14 et 2.15.
- Les 17 paires d'inclusion : étude C (2.11). 2.9 vérifie seulement qu'elle n'en crée pas.
- Les structs de TU (`test_pico8_iv.c`, `test_pico8_ui.c`) et les types hors `--champs` autres que les deux composants : pas d'archétype.
- Renommer un champ (phase 3). L'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Le livrable est `docs/etude-2-9-archetypes.md`, pas `docs/prd/done/plan3-phase2-etape9-etude-archetypes.md`.** Le plan donne comme sortie le chemin de ce PRD une fois gradué. D.9 et D.14, écrites sans PRD, étaient elles-mêmes le fichier de `done/`. Ici, le pilote déplace ce PRD vers `done/` sous ce nom, et `plan-pilot` lit ce nom pour trouver l'étape suivante : l'étude ne peut pas l'occuper. La note « Découvert à 2.9 » corrige la sortie de la ligne 2.9. 2.10 et 2.11 ont le même conflit (voir « Surprises »).
- **G2. Six classes au lieu de deux.** La règle 1 dit « archétype ou état ». K (constante d'instance), I (état initialisé par l'archétype) et S (statique) sont des sous-cas de l'état que 2.12 et 2.13 traitent différemment : K n'a pas de défaut, I en a un que l'init copie, S n'est pas dans l'instance. H est l'arrêt « champ sans règle ».
- **G3. « Construction » englobe les aides `static` appelées seulement par des `_new`/`_init`, pas les `pq`.** C'est une lecture de la règle 1, écrite dans la note. Sans elle, les champs posés par `pico8_lq_struct_alloc` ou `hp_positions` passeraient pour de l'état alors qu'ils ne bougent plus après la construction.
- **G4. Le verdict se fonde sur `src/`, les écritures de TU sont du travail pour 2.13.** Un TU qui fabrique un état impossible en jeu ne fait pas d'un défaut un état. Exception : un test de régression dont l'intention changerait (§6 du plan), classé H.
- **G5. Un champ à la fois, pas un type à la fois.** Mesuré : la passe groupée perd le rattachement de deux champs sur 37. Avec ≈ 0,85 s par champ, le coût est de 5 min.
- **G6. Forme des archétypes alignée dès 2.9 sur la décision A.** Sinon, 2.15 récrirait les archétypes nés à 2.12.

### Surprises de mesure, pour `plan-pilot`

1. **Budget** : le plan donne ≈ 50 min pour 2.9 ; ce PRD en compte **3 × 50 min**. Le relevé scripté prend 5 min, mais le rattachement des erreurs sans nom, la lecture de ≈ 80 champs non mécaniques (58 K probables, 19 I probables, aides de construction), l'inventaire des classes, l'essai d'inclusion et la réécriture de 2.12–2.13 ne tiennent pas dans une seule sous-étape. La ligne « Estimation » gagne 2 sous-étapes.
2. **Conflit de chemin de sortie** : les lignes 2.9, 2.10 et 2.11 nomment comme sortie le chemin que leur propre PRD prendra dans `done/`. G1 le règle pour 2.9. Il faut appliquer la même règle aux PRD de 2.10 et 2.11 (`docs/etude-2-10-composants.md`, `docs/etude-2-11-inclusions.md`).
3. **`--champs` ne voit pas les composants** : les 20 champs de l'émetteur et de la sonde, que la règle 3 nomme, n'y sont pas. L'étude les ajoute.
4. **La ligne 2.12 provisoire oublie deux `pico8_X_start_t`** : `ke` (`me`, `t` de l'en-tête, I probables) et `pj_cr` (instance statique, 0 écriture).
5. **`game_data.h` fait 540 lignes, pas 574** (chiffre de 2.0, d'avant 2.4 à 2.7).
6. **Deux tests de régression écrivent `jo.gq`**, pré-classé A (voir G4). C'est le seul cas où l'étude peut devoir poser une question.

### Questions pour l'humain

Aucune. Celles que l'étude trouvera (champs H) sont posées par elle, dans son §5.

## Annexe A — le banc essayé (texte exact, à poser dans la copie)

`constproof.py`, lancé depuis le dossier de la copie (`tree/` à côté du script) : `timeout 900 python3 constproof.py <liste.tsv>`, où `liste.tsv` a quatre colonnes (type, champ, deux comptes ignorés). C'est la sortie de `--champs`, plus 20 lignes `pico8_emitter\t<m>\t0\t0` / `pico8_probe\t<m>\t0\t0`. Sortie : `writes.tsv` (type, champ, `fichier:ligne`, fonction englobante, message), `missing.tsv` (champs non trouvés : doit être vide). Tous les chemins sont absolus : un premier essai a échoué sur un chemin relatif après le `chdir`.

```python
#!/usr/bin/env python3
import re, subprocess, sys, os, glob, time
S = os.path.dirname(os.path.abspath(__file__)); T = os.path.join(S, 'tree')
LIST = os.path.abspath(sys.argv[1]); os.chdir(T)
def strip(l): return re.sub(r'/\*.*?\*/', '', re.sub(r'//.*', '', l))
bodies, aliases = [], {}
for h in sorted(glob.glob('src/*.h')):
    L = open(h).read().split('\n'); depth = 0; cur = None
    for i, raw in enumerate(L):
        c = strip(raw)
        m = re.match(r'^typedef\s+struct\s+(\w+)\s+(\w+)\s*;', c)
        if m and depth == 0: aliases[m.group(2)] = m.group(1); aliases[m.group(1)] = m.group(2)
        if depth == 0 and cur is None:
            m = re.match(r'^(typedef\s+)?struct(\s+(\w+))?\s*\{', c)
            if m: cur = [h, set([m.group(3)] if m.group(3) else []), i, None]
        depth += c.count('{') - c.count('}')
        if cur is not None and depth == 0 and '}' in c:
            m = re.search(r'\}\s*(\w+)', c)
            if m: cur[1].add(m.group(1))
            cur[3] = i; bodies.append(cur); cur = None
def find(typ, mem):
    names = {typ, aliases.get(typ, '')}
    for f, ns, a, b in bodies:
        if ns & names:
            L = open(f).read().split('\n')
            for i in range(a + 1, b):
                c = strip(L[i])
                if '(' not in c and re.search(r'(?<![\w.>])' + re.escape(mem) + r'\s*(\[[^\]]*\])?\s*[;,]', c):
                    return f, i
    return None, None
FN = re.compile(r'^[a-z][^;]*?(\w+)\s*\(')
def func_of(path, line, cache={}):
    if path not in cache: cache[path] = open(path).read().split('\n')
    fn = '(fichier)'
    for l in cache[path][:line]:
        if l.startswith('}'): fn = '(fichier)'
        m = FN.match(l)
        if m and not l.rstrip().endswith(';'): fn = m.group(1)
    return fn
files = [f for f in sorted(glob.glob('src/*.c') + glob.glob('tests/*.c'))
         if not re.search(r'main_raylib|main_screencast_viewer|raylib_backend|class_coverage', f)]
cov = ['src/main_class_coverage.c', 'tests/test_class_coverage.c']
def compile_all():
    base = ['cc', '-std=c99', '-Wall', '-Wextra', '-Werror', '-Isrc', '-fsyntax-only']
    jobs = [(base, f) for f in files] + [(base + ['-DPICO8_CLASS_COVERAGE'], f) for f in cov]
    out = []
    for i in range(0, len(jobs), 8):
        ps = [subprocess.Popen(a + [f], stderr=subprocess.PIPE, stdout=subprocess.DEVNULL, text=True) for a, f in jobs[i:i+8]]
        out += [p.communicate()[1] for p in ps]
    return '\n'.join(out)
fields = [l.rstrip('\n').split('\t')[:2] for l in open(LIST) if l.count('\t') == 3]
out = open(os.path.join(S, 'writes.tsv'), 'w'); miss = open(os.path.join(S, 'missing.tsv'), 'w')
t0 = time.time()
for typ, mem in fields:
    f, i = find(typ, mem)
    if f is None: miss.write(f'{typ}\t{mem}\n'); continue
    orig = open(f).read(); L = orig.split('\n')
    # const juste avant le nom : 'T *const m', 'int const m' (piège 1)
    L[i] = re.sub(r'(?<![\w.>])(' + re.escape(mem) + r')(\s*(\[[^\]]*\])?\s*[;,])', r'const \1\2', L[i], count=1)
    open(f, 'w').write('\n'.join(L))
    try: err = compile_all()
    finally: open(f, 'w').write(orig)
    n = 0
    for e in err.split('\n'):
        m = re.match(r'^([\w./]+):(\d+):(\d+): error: (.*)$', e)
        if not m: continue
        n += 1; p, ln = m.group(1), int(m.group(2))
        out.write(f'{typ}\t{mem}\t{p}:{ln}\t{func_of(p, ln)}\t{m.group(4)}\n')
    if n == 0: out.write(f'{typ}\t{mem}\t-\t-\t0 erreur\n')
    out.flush()
print('secs', round(time.time() - t0, 1), 'fields', len(fields))
```

Limites connues du banc, à traiter à la lecture : `func_of` repère la fonction englobante par la dernière ligne de colonne 0 de la forme `nom(` (suffisant sur `src/` à `51bb145`, à vérifier sur les sites H) ; une déclaration de plusieurs membres sur une ligne (`int a, b;`) reçoit `const` sur le bon déclarateur seulement si le nom est unique sur la ligne ; le banc ne voit ni les initialiseurs ni `memset`/`memcpy` (piège 4).
