# Étude 2.9 — règle 1 et archétypes

Étude A de la phase 2 (`docs/plan-3.md`, ligne 2.9), en lecture seule, faite selon le PRD `plan3-phase2-etape9-etude-archetypes.md`. Elle tranche, champ par champ, ce qui quitte l'instance pour un archétype `static const` et ce qui reste de l'état. Elle fixe aussi la forme, la place et le chemin de lecture des archétypes, le test ponctuel de la règle 3 et la liste ordonnée de 2.12 et 2.13.

Départ : `522824b` (branche `claude/plan-pilot-3-7zdft0`, 2.8c vérifiée). Les essais sont faits sur une seule copie jetable (`git clone` local dans le scratchpad), jamais dans l'arbre de travail. Les numéros de ligne sont ceux de `522824b`.

Classes du classement (PRD, G2) : **A** archétype ; **I** état initialisé par l'archétype ; **K** constante d'instance ; **E** état ; **S** statique de classe ; **H** à l'humain.

## 0. État de départ

| Hypothèse | Mesure sur `522824b` | Verdict |
|---|---|---|
| H-2.8-1 | `parente` 10/3, `lien-parent` 111/27, `accesseurs-q6` 237/33, `défauts` 669/52, `void-ptr` 8/4, `inclusion-mutuelle` 17/16 (43 lignes au `--check`), pied `résidu 2 : 1073 ligne(s), 85 fichier(s)` ; `COMPOSITION_RESIDUE_GUARD` = `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité` | vraie, mais **six** catégories nulles et gardées, pas sept (douze catégories, six non nulles) |
| H-2.8-2 | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27`, 36 types | vraie |
| H-2.8-3 | `trade.lp`/`rl`, `jo.oe`/`cc`/`by`, `ui.le`, `mz.ox` gardent leur nom (`pico8_ents_t *`) | vraie |
| H-2.8-4 | `pico8_ents.c:9` et `:27` écrivent `ents->items`, membre de `pico8_ents_t` | à moitié fausse : `--champs`, qui cherche par nom, les compte pour `lq.items` et `mh.items`. Le banc, qui prouve par type, ne les voit pas : `lq.items` n'a d'erreurs que dans `layout.c:317` et `:396`, `mh.items` que dans `msgbox.c:121`. Faux positifs écartés |
| H-2.8-5 | `pico8_game_data.h` : 540 lignes | vraie |

## 1. Classement (2.9a)

### Lecture de « construction »

Une écriture est **de construction** si elle est dans une fonction `*_new`/`*_init`, ou dans une aide `static` dont tous les appelants sont des `*_new`/`*_init`. Appelants relus :

- `pico8_lq_struct_alloc` (`layout.c:302`) : `pico8_lq_struct_new` et `pico8_lq_copy` ;
- `pico8_jb_struct_alloc` (`layout.c:406`) : `pico8_jb_struct_new` ;
- `hp_positions` (`research.c:78`) : `pico8_hp_struct_new` ;
- `pico8_lq_copy` (`layout.c:373`) n'est pas `static` : c'est un constructeur par copie (alloue puis `pico8_lq_struct_finish`). Il est appelé par `pico8_js_lq` et deux TU ;
- `pico8_ie_ke_view` (`research_icon.c:51`) construit une vue par valeur. L'affectation `view = pico8_ie_ke_view(self)` (`research_icon.c:171`, `test_pico8_ie.c:672`, `:690`) copie cette vue dans une locale. Ce n'est pas une écriture sur une instance.

Deux inits de module entrent aussi dans cette lecture : `pico8_probe_init` (remet à 0 le remplaçant statique `g_fm_placeholder`) et `pico8_score_panel_init` (reconstruit l'instance statique unique `g_pj_cr` depuis `pico8_pj_start.cr`). Ce sont les constructeurs de ces objets statiques.

Les `pq` (`pico8_jo_pq`, `pico8_js_pq`, `pico8_jc_pq`, `pico8_lq_pq`, `pico8_jb_pq`, `pico8_mh_pq`, `pico8_iv_pq`, `pico8_mz_pq`) et `pico8_f_` suivent la construction comme les méthodes du cart. Une écriture qui y figure classe le champ I ou E.

**Clé de construction** : une valeur lue dans `keys` et écrite par le `_new`. Si le champ a d'abord reçu un défaut de classe, il est **I**. Sinon il est **K**.

### Le banc

Le texte est celui de l'annexe A du PRD (`constproof.py`), sans changement. Le script insère `const` juste avant le nom du membre, puis compile avec `cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only` les 150 `.c` de `src/` et `tests/`. Les deux fichiers `class_coverage` sont compilés sous `-DPICO8_CLASS_COVERAGE`. Il rend ensuite l'en-tête d'origine et passe au champ suivant. La liste contient les 282 lignes de `--champs` et les 20 membres des composants : émetteur `pn`, `du`, `py`, `ki`, `pk`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`, `qy` ; sonde `et`, `dw`, `od`, `has_od`, `dq`, `fp`, `ik`, `has_ik`.

Le tri (`classify.py`, dans le scratchpad) rattache chaque ligne `error:` à son site, en trois sortes :

- écriture : `assignment of read-only member|location`, `… in read-only object`, `increment of read-only member` ;
- affectation de struct entière : `read-only variable 'view'` ;
- fuite d'adresse : `discards 'const' qualifier`.

Le classement final vient de la lecture de chaque champ non mécanique.

**Résultat : 302 champs, 915 lignes `error:`** (758 dans `src/`, 157 dans `tests/`). Il y a 867 écritures, 27 affectations de struct entière et 21 fuites d'adresse, plus 4 champs à 0 erreur. Pour les 282 champs de `--champs`, on compte 632 lignes dans `src/` et 156 dans `tests/`. Le multiensemble (type, champ, message) est **identique** au relevé du PRD sur `51bb145` : 2.8 n'a déplacé que des lignes. Le « 919 lignes d'erreur » du PRD comptait les 4 lignes `0 erreur` : il y a 915 erreurs.

| Classe | Champs | Devient |
|---|---|---|
| **A** | **69** | champ de l'archétype |
| **I** | **41** | reste dans l'instance, défaut dans l'archétype, copié par l'init |
| **K** | **87** | reste dans l'instance, sans défaut |
| **E** | **104** | reste dans l'instance |
| **S** | **1** | reste dans l'init, hors archétype |
| **H** | **0** | — |

Un seul champ est **S** : `mz.jj`. Il vaut `&g_mz_jj_wrapper` pour toute instance et n'est jamais réécrit. Mais sa valeur est l'adresse d'un `static` de `wormhole.c`, de type `pico8_mz_jj_t`, qui contient un en-tête du noyau. `game_data.c` ne peut pas le nommer (essai au §2). Ce lien vers un objet statique de classe reste posé par l'init, comme les défauts des champs I qui pointent un objet statique. Les autres états de classe partagés sont des **objets** statiques (`g_jo_kw_shared`, `g_ga_by_shared`, `g_shared_le`, `g_nf_ra`, `g_mx_ht_spent`, `g_ie_gy`), pas des membres. Le membre qui les pointe est I, puisque son défaut est l'adresse de l'objet et qu'il est réécrit ensuite. Ces objets restent statiques (règle 1, dernière phrase).

Aucun champ n'est **H**. Le cas que le PRD signalait, `jo.gq`, est tranché plus bas.

Par type (comptes égaux à ceux de `--champs`, 36 types, plus les deux composants) :

| Type | Champs | A | I | K | E | S |
|---|---|---|---|---|---|---|
| `bg` | 6 | 3 | 0 | 0 | 3 | 0 |
| `bv_et` | 1 | 0 | 0 | 0 | 1 | 0 |
| `fm_placeholder` | 4 | 0 | 0 | 0 | 4 | 0 |
| `fq` | 7 | 2 | 0 | 1 | 4 | 0 |
| `fr` | 1 | 1 | 0 | 0 | 0 | 0 |
| `fv` | 11 | 9 | 0 | 0 | 2 | 0 |
| `g_` | 10 | 2 | 1 | 4 | 3 | 0 |
| `ga` | 5 | 1 | 2 | 2 | 0 | 0 |
| `hj` | 3 | 1 | 0 | 2 | 0 | 0 |
| `hp` | 25 | 1 | 0 | 24 | 0 | 0 |
| `hy` | 15 | 0 | 4 | 1 | 10 | 0 |
| `ie_ke` | 6 | 0 | 0 | 6 | 0 | 0 |
| `ie` | 10 | 3 | 1 | 2 | 4 | 0 |
| `iv` | 8 | 2 | 1 | 2 | 3 | 0 |
| `jb` | 2 | 0 | 0 | 0 | 2 | 0 |
| `jc_mouse_point` | 2 | 0 | 0 | 2 | 0 | 0 |
| `jc` | 15 | 1 | 2 | 3 | 9 | 0 |
| `jc_too_far` | 2 | 1 | 0 | 0 | 1 | 0 |
| `jo` | 37 | 10 | 5 | 1 | 21 | 0 |
| `js` | 26 | 0 | 1 | 16 | 9 | 0 |
| `lq` | 13 | 0 | 0 | 8 | 5 | 0 |
| `mh` | 7 | 1 | 1 | 3 | 2 | 0 |
| `mx` | 1 | 0 | 1 | 0 | 0 | 0 |
| `mz_jj` | 1 | 1 | 0 | 0 | 0 | 0 |
| `mz` | 13 | 3 | 1 | 2 | 6 | 1 |
| `nf` | 1 | 0 | 1 | 0 | 0 | 0 |
| `nl` | 3 | 2 | 1 | 0 | 0 | 0 |
| `nr` | 2 | 2 | 0 | 0 | 0 | 0 |
| `pj_cr` | 4 | 3 | 0 | 0 | 1 | 0 |
| `pj` | 8 | 2 | 1 | 0 | 5 | 0 |
| `pm` | 2 | 2 | 0 | 0 | 0 | 0 |
| `qb` | 5 | 1 | 4 | 0 | 0 | 0 |
| `qh` | 7 | 4 | 1 | 2 | 0 | 0 |
| `struct_header` | 3 | 0 | 2 | 1 | 0 | 0 |
| `trade` | 5 | 0 | 0 | 5 | 0 | 0 |
| `ui` | 11 | 3 | 1 | 0 | 7 | 0 |
| `emitter` (composant) | 12 | 3 | 8 | 0 | 1 | 0 |
| `probe` (composant) | 8 | 5 | 2 | 0 | 1 | 0 |

Les 69 champs **A** : `bg.fa`/`hd`/`pl` ; `fq.fa`/`hd` ; `fr.mt` ; `fv.ca`/`eo`/`ff`/`fg`/`gq`/`hi`/`kq`/`ql`/`rb` ; `g_.fa`/`fx` ; `ga.fz` ; `hj.rr` ; `hp.fk` ; `ie.eh`/`fa`/`kq` ; `iv.fa`/`v` ; `jc.k_` ; `jc_too_far.qs` ; `jo.ca`/`cq`/`dz`/`em`/`fa`/`gq`/`hf`/`hs`/`ka`/`kq` ; `mh.fa` ; `mz_jj.qs` ; `mz.fa`/`kk`/`spawn_snd` ; `nl.nl`/`rr` ; `nr.hz`/`rr` ; `pj_cr.ca`/`gq`/`hi` ; `pj.gs`/`i_` ; `pm.fz`/`ng` ; `qb.pc` ; `qh.ba`/`hp`/`px`/`qh` ; `ui.fa`/`hw`/`ql` ; émetteur `ki`/`pk`/`fc` ; sonde `dw`/`dq`/`fp`/`ik`/`has_ik`.

Les 41 champs **I** : `g_.ra` ; `ga.iu`/`ms` ; `hy.nz`/`ew`/`lo`/`yr` ; `ie.ra` ; `iv.l` ; `jc.fa`/`nh` ; `jo.by`/`has_mi`/`kw`/`ma`/`mi` ; `js.fa` ; `mh.rh` ; `mx.ht` ; `mz.oi` ; `nf.ra` ; `nl.qw` ; `pj.fa` ; `qb.ma`/`p`/`ra`/`v` ; `qh.s` ; en-tête `me`/`t` ; `ui.le` ; émetteur `pn`/`du`/`py`/`k_`/`qo`/`ro`/`qg`/`q_` ; sonde `od`/`has_od`. Dix d'entre eux ne sont réécrits que par une clé de construction, jamais après : `ga.iu`/`ms`, `js.fa`, `mh.rh`, et six membres de l'émetteur (`pn`/`du`/`py` dans `pico8_g__struct_new` ; `py`/`k_`/`qo`/`ro`/`qg`/`q_` par `keys->pq` dans `pico8_ga_struct_new`). La règle 1 les garde dans l'instance : ils sont écrits sur l'instance, hors de la valeur de classe.

### Verdicts non mécaniques

Chaque verdict est en une phrase dans la dernière colonne de l'annexe A. Les points qui fondent une règle :

- **`jo.gq` (A), écrit par cinq TU.** `test_pico8_planet_gh_row_lq.c:121` et `test_pico8_ga_export_panel_by.c:112` sont deux tests de régression (§6 du plan). Les trois autres sont dans `test_pico8_lq.c` (`:782`, `:824`, `:855`). Dans ces tests, `gq` ne sert qu'au placement de la ligne : `pz` l'ajoute à `bp` (`msgbox.c:322-323`). Aucune assertion ne lit une position, seulement les éléments de la ligne (`"exports:"`, `"0/1"`, `"2/2"`, `fs_count == 4`). La valeur de l'archétype, `{0, 11}` au lieu de `{0, 9}`, ne change donc pas ce que ces tests prouvent. 2.13 retire l'écriture, puis rejoue la preuve du §6 : bug remis, test rouge. Le champ n'est pas H.
- **`g_.fa` (A)** : `test_pico8_g_.c:751` le met à 0 pour vérifier que `is_probe` tombe. **Sonde `dw` (A)** : `test_pico8_jc.c:853` (`test_bv`) le met à 0, comme celui de `mz`. Ce sont des TU unitaires, pas des tests de régression. 2.13 les réécrit sur un archétype de test à `fa`/`dw` nul, que le chemin retenu au §2 permet de poser (G4).
- **`hp.has_dm`/`has_fj`/`has_ii`/`has_il`/`has_o`/`has_rn` (K)** : le tri automatique les rangeait en A (0 puis 1, deux littéraux). Le 1 est posé selon l'enregistrement `ri` : c'est une valeur de l'instance.
- **Stockages propres** (`g_`/`ga`/`hj.emitter_own`, `g_`/`jc`/`mz`/`qh.probe_own`) : leur seule erreur est la fuite `&self->X_own` dans l'init, qui a été suivie. Les écritures faites par ce pointeur sont les lignes des composants `pico8_emitter`/`pico8_probe`. Ces champs sont **K** : le stockage appartient à l'instance.
- **Autres fuites suivies** : `pico8_ma` (`entity_geom.c:79`) écrit `*ma = clamped` dans `jo.ma` et `qb.ma` (I). `jc_mt_of(…, &self->mt)` écrit `jc.mt` (E). `pico8_ptrs_init(&ui->ic_own…)` remplit `ui.ic_own` (E). Les fuites de `pico8_hy_lj` portent sur `hy.fu_own`/`po_own` (E). Aucune fuite ne touche un champ A.
- **En-tête** : `cls` est **K**. C'est la classe de l'instance, argument de `pico8_ke_struct_init`, et le chemin vers l'archétype ; les TU `planet_kj_dispatch` l'échangent pour espionner. `me` et `t` sont **I** : défaut de `pico8_ke_start` ou de la classe, réécrits par `pico8_ke_ks`, `pico8_dg`, `pico8_mh_ev`.
- **`hy.nz`/`ew`/`lo`/`yr` (I)** : le constructeur leur pose 100, 1, 1 et 3401, des littéraux non nuls identiques pour toute instance, puis ils sont réécrits. Selon la définition du PRD, E est réservé aux défauts nil/0/NULL. `hy` n'a pas de `_start` : ces quatre défauts forment son archétype (§2).
- **Sonde locale sans classe** : `pico8_probe_t pb = {0}` (`research_icon.c:180`) est une valeur du composant sonde, construite dans `pico8_ie_kj` pour `pico8_dl_show`. Elle n'appartient à aucune classe. Ses deux écritures, `pb.has_od` (`:182`) et `pb.od` (`:183`), sont dans l'annexe A sous la sonde `has_od` et `od`. Elles ne classent rien : `od`/`has_od` sont I par `pico8_jc_hq`.
- **Instances statiques** (`jc_too_far.qs`, `mz_jj.qs`, `pj_cr.gq`/`hi`) : 0 erreur, parce que leur seule valeur vient de l'initialiseur (piège 4). Elles sont A. `pj_cr.ca` est A : sa seule écriture est l'init de module, depuis `pico8_pj_start.cr`.

### Contrôles de complétude

1. **Champs** : 302 lignes dans l'annexe A. `comm -3` entre la liste (type, champ) de `--champs` augmentée des 20 membres des composants et celle de l'annexe A → **vide**. Les comptes par type sont égaux à ceux de `--champs`.
2. **Preuve** : chaque champ A, I ou S a sa preuve `const`. On a, soit ses sites dans l'annexe A, soit « 0 erreur » avec la raison (initialiseur statique : les 4 champs A plus haut). Aucun champ n'est classé sur un grep seul.
3. **Écritures** : les 915 lignes `error:` du banc sont toutes dans l'annexe A, rattachées à un champ, fuites et structs entières comprises : 915 sites listés. `grep -nE 'memset|memcpy|calloc' src/*.c tests/*.c` rend 95 lignes, dont aucune ne vise un type d'entité. On y trouve des clés `pico8_js_keys_t`/`pico8_lq_keys_t` (`help.c:55`, `menu.c:175`, `:356`, `planet_display.c:152-153`, `:173`, `:341`, `:353`, `:384`), des boutons `pico8_nf_button_t` (`mouse_reader.c:62-63`) et des rangées de données (`pico8_jb_row_t`, `pico8_cr_row_t`, `pico8_hp_jb_t`, `pico8_jy_t`, `pico8_jo_ex_tuple_t`). S'y ajoutent des chaînes, la mémoire PICO-8, les trames d'entrée des TU et un `pico8_kw_t` (`test_pico8_lq.c:802`).
4. **H** : aucun champ H ; `jo.gq` est tranché plus haut.

Fichiers non compilés : le filtre `raylib_backend` du banc en exclut **cinq**, pas trois. Ce sont `main_raylib.c`, `main_screencast_viewer.c`, `pico8_raylib_backend.c`, plus `pico8_raylib_backend_coords.c` et `tests/test_pico8_raylib_backend_coords.c`. Ces deux derniers compilent sans `raylib.h`. Le grep des 38 noms de types d'entité et de composant (et de `struct_header`) ne trouve rien dans les cinq : aucune écriture n'échappe au banc.

## 2. Archétypes (2.9b)

### Inventaire

**26 classes reçoivent un archétype, 14 tables de classe n'en reçoivent pas.** Cela couvre les 40 tables `static const pico8_struct_class_t g_X_struct_class` (41 lignes au grep ; `layout.c:12` est une déclaration anticipée) et les 27 `pico8_X_start_t` (`game_data.h:117-423`).

| Classe (table) | `pico8_X_start_t` | Archétype | Raison |
|---|---|---|---|
| `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx` (10) | les 10 | oui, type **planète** `pico8_jo_def_t` | défauts de spécification ; un seul type pour les dix sortes (décision A) |
| `g_`, `fv` (2) | `g__`, `fv` | oui, type **sonde** `pico8_g__def_t` | idem, un seul type pour la sonde et sa variante d'aide (décision A) |
| `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`, `hp`, `ie`, `pj` (13) | les 13 | oui, un type par classe (`pico8_bg_def_t`…) | racines simples. `js` n'a que `fa` (I) et `nf` que `me`/`t`, mais la Sortie veut un archétype par ancienne classe |
| — | `ke` | pas d'archétype propre | `ke` est le noyau, sans table ni instance. Ses deux défauts (`me` = `"ph"`, `t` = 0) deviennent les deux premiers membres de chaque archétype d'une classe qui descend de `ke`. `pico8_ke_start` disparaît quand la dernière famille ne l'appelle plus |
| `pj_cr` | `pj_cr` | non | instance statique unique `g_pj_cr` : ses trois A (`ca`, `gq`, `hi`) passent dans son initialiseur statique, et `pico8_pj_cr_start` disparaît avec `pj` (2.13, famille a) |
| `hy` | — | oui, `pico8_hy_def_t`, **dans son module** | ses quatre I (`nz` 100, `ew` 1, `lo` 1, `yr` 3401). Pas de `me`/`t` : `hy` ne descend pas de `ke`. C'est la seule exception de place (voir plus bas) |
| `trade`, `lq`, `jb` | — | non | aucun défaut de classe : tous les champs sont K ou E |
| `mo`, `je`, `fm` (remplaçants) | — | non | remplaçants statiques ; seul `fm` a des champs, tous E |
| `bv_et`, `jc_too_far`, `mz_jj` | — | non | instances statiques uniques ; leurs A (`qs`) sont déjà dans l'initialiseur |
| `ha_pz` | — | non | objet jamais alloué, que `pico8_menu_init` fait pointer sur les valeurs de `ha` (`menu.c:47-51`). Ce n'est pas un type de `--champs` |
| `ie_ke_view` | — | non | vue construite à la demande (K) |
| `jc_mouse_point`, `nr_ldown_empty` | — | non | objets transitoires, sans défaut |

Contrôle 5 : les 27 `_start_t` et les 40 tables sont dans ce tableau. Les 36 types de `--champs` s'y retrouvent aussi : les neuf types de l'arbre `jo` qui ont des champs propres, `g_`/`fv`, les 13 racines, `pj_cr`, `hy`, `trade`, `lq`, `jb`, `fm_placeholder`, `bv_et`, `jc_too_far`, `mz_jj`, `ie_ke`, `jc_mouse_point`, et l'en-tête (noyau). `os` n'a aucun champ propre dans `--champs`, `ha_pz` n'y est pas.

### Forme

Les objets constants sont **résolus** le long de la chaîne : l'init parente, puis les défauts propres. Il y a **un type d'archétype par futur type aplati**. Le type planète réunit les champs A et I des dix types de l'arbre `jo`, un seul membre par nom : `rr` de `nl`/`hj`/`nr`, `fz` de `pm`/`ga`. Le type sonde réunit ceux de `g_` et `fv`. Les champs des composants forment deux sous-archétypes, `pico8_emitter_def_t` et `pico8_probe_def_t`, enchâssés dans les archétypes des classes qui portent le composant : planète, sonde, `mz`, `jc`. Une classe sans le composant y laisse des zéros, que rien ne lit, puisque sa table n'a ni `emitter_of` ni `probe_of`.

Les vecteurs et boîtes que les `_start` portent en ligne (`kq`, `gq`, `hi`, `ng`, `ra`, `p`, `v`, `hw`, `eh`) sont des membres **par valeur** de l'archétype. Le membre de l'instance, quand il reste (I), pointe dans l'archétype : `self->ra = &def->ra`. Un champ dont la valeur de classe est l'adresse d'un objet statique du module n'entre pas dans l'archétype. `game_data.c` ne peut pas nommer cet objet : c'est un `static` du module, et `mz.jj` contient en plus un en-tête du noyau. Ce champ reste écrit par l'init du module propriétaire. Sont concernés les défauts I `jo.kw` → `g_jo_kw_shared`, `jo.by` de `ga` → `g_ga_by_shared`, `ui.le` → `g_shared_le`, `nf.ra` → `g_nf_ra`, et le champ S `mz.jj` → `g_mz_jj_wrapper`. `jo.by`, NULL pour les neuf autres sortes, n'est pas non plus membre de `pico8_jo_def_t`. `kw_own` n'en est pas membre non plus : c'est un champ E, le stockage que `pico8_jo_lz` remplit, sans défaut de classe. `pico8_jo_def_t` a 32 membres.

Texte C de l'archétype de `nl` (trois niveaux de chaîne, groupe sonde), essayé sur la copie :

```c
typedef struct {
    pico8_fixed_t pn, du, py, ki, pk, fc;
    const pico8_int_seq_t *k_;
    pico8_fixed_t qo, ro, qg, q_;
} pico8_emitter_def_t;

typedef struct {
    pico8_fixed_t dw;
    pico8_fixed_t od;
    int has_od;
    const char *dq;
    pico8_fixed_t fp;
    pico8_fixed_t ik;
    int has_ik;
} pico8_probe_def_t;

typedef struct {
    const char *me;               /* noyau */
    pico8_fixed_t t;
    int hf;                       /* jo */
    pico8_fixed_t hs, fa;
    pico8_qz_box_t kq;
    pico8_fixed_t ma;
    const pico8_int_seq_t *ka;
    const pico8_jb_row_t *dz;
    const pico8_int_seq_t *cq;
    pico8_vec_t gq;
    pico8_fixed_t ca;
    const pico8_jb_row_t *em;
    pico8_fixed_t mi;
    int has_mi;
    pico8_fixed_t hp;             /* qh */
    const pico8_jb_row_t *px;
    int qh;
    pico8_fixed_t s;
    const pico8_int_seq_t *ba;
    const char *rr;               /* nl, hj, nr */
    pico8_fixed_t nl, qw;         /* nl */
    const char *fz;               /* pm, ga */
    pico8_vec_t ng;               /* pm */
    pico8_fixed_t iu, ms;         /* ga */
    const pico8_int_seq_t *hz;    /* nr */
    pico8_fixed_t mt;             /* fr */
    const pico8_int_seq_t *ht;    /* mx */
    pico8_probe_def_t probe;
    pico8_emitter_def_t emitter;
} pico8_jo_def_t;

const pico8_jo_def_t pico8_nl_def = {
    .me = qh_me_text, .t = 0x0,
    .hf = 0, .hs = 0x10000, .fa = 0x60000,
    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
    .ma = 0x0, .ka = &pico8_jo_ka_seq, .dz = pico8_jo_dz_rows, .cq = &pico8_jo_cq_seq,
    .gq = {0x0, 0xb0000}, .ca = 0x90000, .em = &pico8_jo_em_row,
    .mi = 0x10000, .has_mi = 1,
    .hp = 0x60000, .px = &pico8_nl_px_row, .qh = 0, .s = 0x440000, .ba = &pico8_nl_ba_seq,
    .rr = nl_rr_text, .nl = 0x10000, .qw = 0xf0000,
    .probe = {.dw = 0x666, .od = 0x140000, .has_od = 1, .dq = qh_dq_text, .fp = 0x20000, .ik = 0, .has_ik = 0},
};
```

Chaque valeur vient de la chaîne `pico8_ke_struct_init` → `pico8_jo_struct_init` → `pico8_qh_struct_init` → `pico8_nl_struct_new`, la dernière écriture l'emportant. Trois défauts de `qh` sont masqués par ceux de `nl` : `hp` 1 → 6, `s` 0xae → 0x44, `od` 15 → 20. `qh` masque lui-même `jo.hf` (1 → 0). La table champ → valeur → ligne source des 26 classes est l'annexe B : 471 lignes (258 A, 212 I, 1 S), produites par `chains.py` à partir des sites de construction du banc. `hy` y a été ajouté à la main, et `mz.jj` est noté S.

Le type sonde `pico8_g__def_t` porte `me`, `t`, `fa`, `fx`, `ra` (vecteur, avec un drapeau : NULL pour `g_`), les membres propres de `fv` (`hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb`), `probe` et `emitter`. Chaque racine simple a son type, avec `me`/`t` en tête s'il descend de `ke` (toutes sauf `hp`), puis ses A et ses défauts I. Exemple, `pico8_bg_def_t` = `{me, t, fa, hd, pl}`.

### Place : `pico8_game_data.h` (types) et `pico8_game_data.c` (valeurs)

Les archétypes prennent la place des `_start` d'aujourd'hui. Les types sont dans `game_data.h`, et les valeurs sont des objets `const` de portée fichier dans `game_data.c`, déclarés `extern` dans l'en-tête. Le « `static const` » du plan se lit « objet constant à durée de stockage statique ». Une liaison interne (`static`) le rendrait illisible pour le module de la classe et pour le test ponctuel.

Pourquoi pas le module de la classe : les valeurs nomment des données que `game_data.c` garde `static` (`jo_me_text`, `qh_me_text`, `qh_dq_text`, `nl_rr_text`… `game_data.c:435-438`). Les ranger dans `planet_family.c` exigerait d'exporter ces données, soit une dizaine de nouveaux `extern`. Et les valeurs de l'arbre `jo` seraient réparties sur trois modules (`planet.c`, `planet_family.c`, `planet_family_trade.c`) pour un seul type. Une fois `mz.jj` et les quatre défauts d'objets statiques sortis, les types n'emploient que `pico8_fixed_t`, `pico8_qz_box_t`, `pico8_vec_t`, `pico8_int_seq_t`, `pico8_jb_row_t`, `pico8_kw_t` et `const char *`. `game_data.h` les a déjà et n'a besoin d'aucun type du noyau. La paire `game_data` ↔ `struct` que 2.2 a coupée ne revient pas. Avec `mz.jj` dans `pico8_mz_def_t`, elle reviendrait : l'essai ne compile pas (`'g_mz_jj_wrapper' undeclared`), et nommer son type depuis `game_data.h` ajoute une paire (18, mesure du vérificateur).

**Exception : `hy`.** Son archétype ne nomme aucune donnée de `game_data.c`, et `economy.h` n'inclut pas `game_data.h`. Le type `pico8_hy_def_t` va donc dans `economy.h` et la valeur `static const` dans `economy.c`. C'est la lecture littérale du plan, et aucune inclusion n'est ajoutée. Dans `game_data.h`, il faudrait que `economy.h` inclue `game_data.h` (1re variante de la 3e passe : `unknown type name 'pico8_hy_def_t'`).

**Essai d'inclusion** (contrôle 7), sur la copie :

- les types `pico8_emitter_def_t`, `pico8_probe_def_t`, `pico8_jo_def_t` et `pico8_bg_def_t` ajoutés en fin de `game_data.h` ;
- `pico8_nl_def` et `pico8_bg_def` ajoutés en fin de `game_data.c` ;
- un membre `const pico8_jo_def_t *def` dans `pico8_jo_t` et un membre `const pico8_bg_def_t *def` dans `pico8_bg_t` ;
- l'affectation `def = &pico8_X_def` dans `pico8_nl_struct_new` et `pico8_bg_struct_new`.

Deux passes. La première avait `kw_own` en pointeur vers `pico8_jo_kw_start`. Tout compile (`-fsyntax-only`, 150 + 2 fichiers, 0 erreur). `inclusion-mutuelle` rend **17 / 16** (43 lignes au `--check`), et `parente`, `lien-parent`, `accesseurs-q6`, `void-ptr` sont inchangés. Mais `défauts` passe à 670 (+1) : la ligne `.kw_own = &pico8_jo_kw_start` nomme un objet au suffixe `_start`. D'où `kw_own` par valeur. La seconde passe porte le texte exact ci-dessus : 0 erreur, `inclusion-mutuelle` **17 / 16**, **43 lignes**, et toutes les catégories identiques au départ. Le pied est **1073 / 85**, `défauts` **669**. `--champs` passerait à 284 (les deux `def`). La copie est rendue à `522824b` après chaque passe (`git checkout -- .`, `status` vide).

Troisième passe, après la revue du vérificateur, sur une copie de `d9a2b2d`. Elle ajoute `mz` et `hy` à `nl` et `bg`. `pico8_mz_def_t` (`me`, `t`, `fa`, `kk`, `spawn_snd`, `oi`, `probe`) et `pico8_mz_def` vont dans `game_data.h/.c`, avec un `def` dans `pico8_mz_t` posé par `pico8_mz_struct_new`. `pico8_hy_def_t` va dans `economy.h`, `g_hy_def` dans `economy.c`, avec un `def` dans `pico8_hy_t`. Résultats : 0 erreur ; `inclusion-mutuelle` **17 / 16**, **43 lignes** ; pied **1073 / 85**, `défauts` 669, toutes catégories identiques. Variante témoin avec `.jj = &g_mz_jj_wrapper` dans `pico8_mz_def` : `game_data.c:684: 'g_mz_jj_wrapper' undeclared`. Copie supprimée ensuite.

### Chemin de lecture : (ii), un pointeur d'archétype typé dans l'instance

| Critère | (i) struct de classe par famille, descente par cast de `header.cls` | (ii) `const pico8_X_def_t *def` posé par l'init | (iii) accesseur `static` par module sur `header.cls` |
|---|---|---|---|
| `void *` nouveau | non | non | non |
| paire d'inclusion neuve | non | non (essai ci-dessus) | non |
| `parente` / `lien-parent` | à vérifier : une struct qui enchâsse `pico8_struct_class_t` en premier membre | inchangés (essai) | inchangés |
| casts | un cast descendant non vérifié par lecture, ce que la phase 2 retire | aucun | aucun, mais un aiguillage par comparaison de classe (10 branches pour la planète) |
| changement des 40 tables | 25 tables à remodeler (`.base = {…}`) | aucune | aucune |
| coût pour 2.14 | `get_kq` lit par le cast | `get_kq` rend `&self->def->kq`, sans table | un appel d'aiguillage par lecture |
| coût pour 2.15 | la famille garde sa struct de classe | le type aplati garde un seul `def` | l'aiguillage survit à l'aplatissement |
| TU qui veut un défaut autre (`g_.fa`, `dw`, §1) | table de test à construire | `inst->def = &archétype_de_test` | impossible sans une classe de test |

**Retenu : (ii).** C'est la seule option sans cast ni aiguillage, et elle ne touche aucune table. Elle permet aussi aux deux TU du §1 de poser un archétype de test. Le coût est un pointeur par instance. Le risque est que `def` et `cls` ne concordent pas. Le même `_new` pose les deux, et le test ponctuel (§3) compare la chaîne à `def` : ce risque est couvert. Pas de question à l'humain : aucune égalité sur les critères.

### Part copiée à l'init

Pour chaque classe, ce sont les champs I, dont l'init copie le défaut depuis `def`. S'y ajoutent `me`/`t` de l'en-tête pour toutes les classes qui descendent de `ke`. `kw_own` n'en est pas : c'est un E (corrigé à 2.12e1, annexe A). Les écritures qui rendent chacun de ces champs mutables sont dans la colonne « hors construction » de l'annexe A. Un champ noté « 0 ou NULL » y garde un défaut nul pour cette classe.

| Classe | Champs I copiés (hors `me`/`t`) |
|---|---|
| `jo` | `jo.by` (0 ou NULL), `jo.has_mi` (nil), `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi` (nil) |
| `qh` | `jo.by` (0 ou NULL), `jo.has_mi`, `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi`, `qh.s`, `probe.od`, `probe.has_od` |
| `nl` | `jo.by` (0 ou NULL), `jo.has_mi`, `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi`, `nl.qw`, `qh.s`, `probe.od`, `probe.has_od` |
| `os` | `jo.by` (0 ou NULL), `jo.has_mi`, `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi`, `qh.s`, `probe.od`, `probe.has_od` |
| `hj` | `jo.by` (0 ou NULL), `jo.has_mi`, `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi`, `qh.s`, `emitter.pn`, `emitter.du`, `emitter.py`, `emitter.k_`, `emitter.qo`, `emitter.ro`, `emitter.qg`, `emitter.q_`, `probe.od`, `probe.has_od` |
| `pm` | `jo.by` (0 ou NULL), `jo.has_mi`, `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi`, `qh.s`, `emitter.pn`, `emitter.du`, `emitter.py`, `emitter.k_`, `emitter.qo`, `emitter.ro`, `emitter.qg`, `emitter.q_`, `probe.od`, `probe.has_od` |
| `mx` | `jo.by` (0 ou NULL), `jo.has_mi`, `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi`, `mx.ht`, `qh.s`, `emitter.pn`, `emitter.du`, `emitter.py`, `emitter.k_`, `emitter.qo`, `emitter.ro`, `emitter.qg`, `emitter.q_`, `probe.od`, `probe.has_od` |
| `ga` | `ga.iu`, `ga.ms`, `jo.by` (défaut `&g_ga_by_shared`, objet statique du module : reste écrit par l'init), `jo.has_mi` (0 ou NULL), `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi` (nil), `emitter.pn` (nil), `emitter.du` (nil), `emitter.py`, `emitter.k_`, `emitter.qo`, `emitter.ro`, `emitter.qg`, `emitter.q_` |
| `nr` | `jo.by` (0 ou NULL), `jo.has_mi`, `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi`, `qh.s`, `probe.od`, `probe.has_od` |
| `fr` | `jo.by` (0 ou NULL), `jo.has_mi`, `jo.kw` (défaut `&g_jo_kw_shared`, objet statique du module : reste écrit par l'init), `jo.ma`, `jo.mi`, `qh.s`, `probe.od`, `probe.has_od` |
| `g_` | `g_.ra` (0 ou NULL), `emitter.pn`, `emitter.du`, `emitter.py`, `emitter.k_`, `emitter.qo`, `emitter.ro`, `emitter.qg`, `emitter.q_`, `probe.od`, `probe.has_od` |
| `fv` | `g_.ra`, `emitter.pn`, `emitter.du`, `emitter.py`, `emitter.k_`, `emitter.qo`, `emitter.ro`, `emitter.qg`, `emitter.q_`, `probe.od`, `probe.has_od` |
| `bg` | — |
| `ui` | `ui.le` (défaut `&g_shared_le`, objet statique du module : reste écrit par l'init) |
| `js` | `js.fa` |
| `fq` | — |
| `qb` | `qb.ma`, `qb.p`, `qb.ra`, `qb.v` |
| `nf` | `nf.ra` (défaut `&g_nf_ra`, objet statique du module : reste écrit par l'init) |
| `mh` | `mh.rh` |
| `iv` | `iv.l` |
| `mz` | `mz.oi`, `probe.od`, `probe.has_od` ; `mz.jj` (S, `&g_mz_jj_wrapper`) reste écrit par l'init |
| `jc` | `jc.fa`, `jc.nh`, `probe.od` (0 ou NULL), `probe.has_od` (nil) |
| `hp` | — |
| `ie` | `ie.ra` |
| `pj` | `pj.fa` |
| `hy` | `hy.nz`, `hy.ew`, `hy.lo`, `hy.yr` (pas de `me`/`t`) |

## 3. Test ponctuel de la règle 3 (2.9c)

**Point d'accroche : un TU neuf, `tests/test_pico8_archetype_chain.c`.** Il naît à 2.12a et gagne une fonction par classe à chaque sous-étape de 2.12. À 2.13, il perd chaque fonction dans le commit qui retire la chaîne de sa famille, et le fichier disparaît au dernier. Les TU `jo_spec_defaults`, `spec_defaults` et `entity_kind` restent : 2.13 les réécrit pour comparer l'instance à son `def`. Mettre le test ponctuel dans ces TU mêlerait une garde durable et un test voué à disparaître. Ce serait le travail deux fois.

**Ce que compare chaque fonction.** Elle prend la valeur par la chaîne, c'est-à-dire un `pico8_X_new(NULL)` frais après les inits de module, comme les TU de défauts d'aujourd'hui. Seuls les `_new` portent les défauts des classes feuilles. Elle la compare à la valeur de `pico8_X_def`, champ par champ, pour tous les champs A et I de la classe (annexe B). Les chaînes et les boîtes se comparent par contenu, puisque l'archétype porte ses propres vecteurs par valeur.

Trois exceptions, écrites dans le TU :

- `jo.kw`, `jo.by` de `ga`, `ui.le`, `nf.ra`, `mz.jj` : leur valeur de classe est un objet statique du module, hors archétype (§2). Ils se comparent à cet objet, et `jo.by` des neuf autres sortes à NULL. Ils ne comptent pas dans les comparaisons à l'archétype.
- `jo.kw_own` n'est pas un champ d'archétype (E, corrigé à 2.12e1) : c'est le stockage de la table que `pico8_jo_lz` crée, lu seulement par `kw` après que `lz` a écrit ses trois membres. Le test ne le compare sur aucune sorte ; `assert_lz_ran` vérifie que `kw` pointe dessus sur les huit sortes où `lz` tourne (toutes sauf `jo` et `ga`).
- La sonde `et` et l'émetteur `qy` (E, groupes Q7) ne sont pas dans l'archétype. Le test vérifie la valeur que le `_new` leur laisse : `et` NULL, et `qy` tel que `pico8_f_` le laisse.

**Groupes de Q7.** Chaque membre nommé par le plan, pour chaque classe que le plan lui associe : classe du champ, puis valeur résolue.

| Classe | `pn` | `du` | `py` | `ki` | `pk` | `fc` | `k_` | `qo` | `ro` | `qg` | `q_` | `qy` |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `g_` | I `0x30000` | I `0x20000` | I `0xf5c` | A `0x90000` | A `0x10000` | A `0x10000` | I `&g__k__seq` | I `0x1051e` | I `-0xd70` | I `0xd70` | I `0x1051e` | E `NULL` |
| `fv` | I `0x50000` | I `0xf0000` | I `0x1b6` | A `0x110000` | A `0x10000` | A `0x50000` | I `&fv_k__seq` | I `0x1051e` | I `-0x1ca` | I `0x1ca` | I `0x1051e` | E `NULL` |
| `hj` | I `0x50000` | I `0x78000` | I `0x1eb8` | A `0x90000` | A `0x10000` | A `0x8000` | I `&hj_k__seq` | I `0xfae0` | I `-0x11ec` | I `0x11ea` | I `0xfae0` | E `NULL` |
| `pm` | I `0x80000` | I `0x20000` | I `0xf5c` | A `0x90000` | A `0` | A `0x10000` | I `&pm_k__seq` | I `0x1051e` | I `-0xd70` | I `0xd70` | I `0x1051e` | E `NULL` |
| `mx` | I `0x50000` | I `0x50000` | I `0x1eb8` | A `0x90000` | A `0x10000` | A `0x4ccc` | I `&mx_k__seq` | I `0xfae0` | I `-0x11ec` | I `0x11ea` | I `0xfae0` | E `NULL` |
| `ga` | I `0` | I `0` | I `0x60000` | A `0` | A `0` | A `0x0` | I `&ga_k__seq` | I `0xe666` | I `0x0` | I `0x0` | I `0xe666` | E `NULL` |

| Classe | `et` | `dw` | `od` | `dq` | `fp` | `ik` |
|---|---|---|---|---|---|---|
| `g_` | E `NULL` | A `0x666` | I `0x0` | A `"ly"` | A `0` | A `0` |
| `fv` | E `NULL` | A `0x666` | I `0x0` | A `"ly"` | A `0` | A `0` |
| `qh` | E `NULL` | A `0x666` | I `0xf0000` | A `qh_dq_text` | A `0x20000` | A `0` |
| `nl` | E `NULL` | A `0x666` | I `0x140000` | A `qh_dq_text` | A `0x20000` | A `0` |
| `os` | E `NULL` | A `0x666` | I `0xf0000` | A `qh_dq_text` | A `0x20000` | A `0` |
| `hj` | E `NULL` | A `0x666` | I `0x3c0000` | A `qh_dq_text` | A `0x20000` | A `0` |
| `pm` | E `NULL` | A `0x666` | I `0x3c0000` | A `qh_dq_text` | A `0x20000` | A `0` |
| `nr` | E `NULL` | A `0x28e` | I `0xa0000` | A `qh_dq_text` | A `0x10000` | A `0` |
| `fr` | E `NULL` | A `0x28e` | I `0x190000` | A `qh_dq_text` | A `0x10000` | A `0` |
| `mx` | E `NULL` | A `0x666` | I `0x280000` | A `qh_dq_text` | A `0x20000` | A `0` |
| `mz` | E `NULL` | A `0x0` | I `0x30000` | A `"pr"` | A `0x10000` | A `0x310000` |
| `jc` | E `NULL` | A `0x106` | I `0` | A `NULL` | A `0` | A `0x300000` |

Tout le groupe émetteur est A ou I pour les six classes, sauf `qy` (E). `pn`, `du`, `py`, `k_`, `qo`, `ro`, `qg`, `q_` sont I, parce que des clés de construction les réécrivent (`g_`, `ga`). Le groupe sonde est A pour `dw`, `dq`, `fp` et `ik`. `od` est I : `pico8_jc_hq` le réécrit. `et` est E.

**Masquages.** Ce sont les champs qu'une classe pose à une autre valeur que son ascendante. Il y en a 102 sur 24 classes : les cibles de la mutation « défaut hérité masqué ». Le dernier défaut de chaque chaîne est l'annexe B, et la chaîne complète est `masks.tsv` dans le scratchpad.

| Classe | Famille | Comparés (A + I) | Champs masqués |
|---|---|---|---|
| `bg` | a | 3 + 2 | — |
| `ui` | a | 3 + 2 | — |
| `js` | a | 0 + 3 | — |
| `fq` | a | 2 + 2 | — |
| `qb` | a | 1 + 6 | — |
| `nf` | a | 0 + 2 | — |
| `mh` | a | 1 + 3 | — |
| `iv` | a | 2 + 3 | — |
| `hp` | a | 1 + 0 | — |
| `ie` | a | 3 + 3 | — |
| `pj` | a | 2 + 3 | — |
| `hy` | a | 0 + 4 | — |
| `mz` | b | 8 + 5 | `en-tête.me` |
| `jc` | b | 6 + 6 | `en-tête.me` |
| `g_` | c | 10 + 13 | — |
| `fv` | c | 19 + 13 | `g_.fa`, `g_.ra`, `emitter.pn`, `emitter.du`, `emitter.py`, `emitter.ki`, `emitter.fc`, `emitter.k_`, `emitter.ro`, `emitter.qg` |
| `jo` | d | 10 + 5 | `en-tête.me` |
| `qh` | d | 19 + 8 | `jo.has_mi`, `jo.hf`, `jo.mi`, `en-tête.me` |
| `nl` | d | 21 + 9 | `jo.has_mi`, `jo.hf`, `jo.mi`, `qh.ba`, `qh.hp`, `qh.px`, `qh.qh`, `qh.s`, `en-tête.me`, `probe.od` |
| `os` | d | 19 + 8 | `jo.has_mi`, `jo.hf`, `jo.hs`, `jo.mi`, `qh.hp`, `qh.px`, `qh.qh`, `qh.s`, `en-tête.me` |
| `hj` | e1 | 23 + 16 | `jo.has_mi`, `jo.hf`, `jo.mi`, `qh.hp`, `qh.px`, `qh.qh`, `qh.s`, `en-tête.me`, `probe.od` |
| `pm` | e1 | 25 + 16 | `jo.has_mi`, `jo.hf`, `jo.mi`, `qh.hp`, `qh.px`, `qh.qh`, `qh.s`, `en-tête.me`, `emitter.pn`, `emitter.du`, `emitter.py`, `emitter.pk`, `emitter.fc`, `emitter.k_`, `emitter.qo`, `emitter.ro`, `emitter.qg`, `emitter.q_`, `probe.od` |
| `mx` | e1 | 23 + 17 | `hj.rr`, `jo.has_mi`, `jo.hf`, `jo.mi`, `qh.hp`, `qh.px`, `qh.qh`, `qh.s`, `en-tête.me`, `emitter.du`, `emitter.fc`, `emitter.k_`, `probe.od` |
| `ga` | e2 | 14 + 15 | `jo.by`, `jo.fa`, `jo.hf`, `jo.kq`, `en-tête.me` |
| `nr` | e2 | 21 + 8 | `jo.has_mi`, `jo.hf`, `jo.kq`, `jo.mi`, `qh.s`, `en-tête.me`, `probe.dw`, `probe.od`, `probe.fp` |
| `fr` | e2 | 22 + 8 | `jo.has_mi`, `jo.hf`, `jo.kq`, `jo.mi`, `nr.hz`, `nr.rr`, `qh.s`, `en-tête.me`, `probe.dw`, `probe.od`, `probe.fp` |

**Preuve qu'il mord : deux mutations par famille**, une à la fois, sur l'archétype, jamais sur la chaîne :

| Famille | Mutation 1 (défaut propre changé) | Mutation 2 (défaut hérité masqué, rendu à la valeur de l'ascendante) |
|---|---|---|
| a | `pico8_bg_def.hd` | La famille n'a **aucun défaut hérité masqué**, donc pas de cible au sens strict. On change `pico8_ui_def.me`, défaut hérité de `ke` sans masquage, pour vérifier que la valeur héritée est comparée |
| b | `pico8_mz_def.probe.ik` | `pico8_jc_def.me` rendu à `"ph"` (`ke`), masqué par `"kz"` |
| c | `pico8_fv_def.ql` | `pico8_fv_def.fa` rendu à `0x20000` (`g_`), masqué par `0xa0000` |
| d | `pico8_nl_def.qw` | `pico8_nl_def.hf` rendu à 1 (`jo`), masqué par 0 (`qh`) |
| e1 | `pico8_pm_def.ng` | `pico8_mx_def.rr` rendu au texte de `hj` |
| e2 | `pico8_fr_def.mt` | `pico8_ga_def.kq` rendu à la boîte de `jo` |

Chaque mutation doit rendre le TU rouge. Une mutation verte est un `FAILED:` de la sous-étape.

## 4. Liste ordonnée de 2.12 et 2.13

Familles du plan provisoire, avec deux corrections. `pj_cr` va avec `pj` (famille a), sans archétype propre. `ke` n'a pas d'archétype : ses `me`/`t` entrent dans chaque archétype dès 2.12, et `pico8_ke_start` sort au dernier commit de 2.13. Les familles d et e forment un seul type d'archétype : d crée `pico8_jo_def_t` complet, avec les champs des dix sortes, et e n'ajoute que des valeurs. e est coupée en deux pour tenir ≈ 50 min (208 comparaisons). `hy` rejoint la famille a. Les comparaisons excluent les cinq champs qui pointent un objet statique (§3).

**Comptes `défauts`** : `composition_residue.sh --check défauts` rend 669 lignes. Chacune est attribuée par les noms `pico8_X_start` qu'elle porte, et elle sort du compte quand le dernier de ces noms disparaît. Une ligne qui nomme plusieurs familles va donc à la dernière : `ke` se range en e2, avec `test_pico8_map_tables.c:156` et `test_pico8_spec_defaults.c:111`. Par famille : a1 66, a2 67 (dont `pj_cr` 8), b 29, c 104, d 179 (dont `pico8_jo_kw_start` 3), e1 127, e2 97 (dont `ke` 25).

2.12 — archétypes et test ponctuel, six sous-étapes :

| Sous-étape | Classes | Comparaisons ajoutées (A + I) | `défauts` |
|---|---|---|---|
| 2.12a | `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `hp`, `ie`, `pj`, `hy` | 51 | 669 |
| 2.12b | `mz`, `jc`, avec `pico8_probe_def_t` | 25 | 669 |
| 2.12c | `g_`, `fv` (type sonde), avec `pico8_emitter_def_t` | 55 | 669 |
| 2.12d | `jo`, `qh`, `nl`, `os`, avec le type planète complet | 100 (99 après 2.12e1) | 669 |
| 2.12e1 | `hj`, `pm`, `mx` | 120 | 669 |
| 2.12e2 | `ga`, `nr`, `fr` | 88 | 669 |

2.13 — l'init lit l'archétype, sept sous-étapes :

| Sous-étape | Classes | `défauts` d'arrivée |
|---|---|---|
| 2.13a1 | `bg`, `ui`, `js`, `fq`, `nf`, `mh`, `iv`, `hp`, `hy` (aucune ligne `_start`) | **603** |
| 2.13a2 | `qb`, `ie`, `pj`, `pj_cr` | **536** |
| 2.13b | `mz`, `jc` | **507** |
| 2.13c | `g_`, `fv` | **403** |
| 2.13d | `jo`, `qh`, `nl`, `os` | **224** |
| 2.13e1 | `hj`, `pm`, `mx` | **97** |
| 2.13e2 | `ga`, `nr`, `fr`, puis `ke` | **0**, gardé |

L'ordre d puis e tient à 2.13. Dès 2.13d, `pico8_jo_struct_init` copie tout le `def` de la classe concrète, que chaque `_new` de l'arbre lui passe : les dix archétypes existent depuis 2.12e2. Jusqu'à 2.13e, les `_new` de la famille e réécrivent encore leurs propres `_start` par-dessus, aux mêmes valeurs.

## 5. Questions à l'humain

Aucune. Aucun champ n'est H, et le chemin de lecture ne laisse pas deux options à égalité.

## Annexe A — les 302 champs

Colonnes : type (sans `pico8_`/`_t`), champ, classe, écritures `src/` de construction, écritures `src/` hors construction, écritures `tests/` (`T:` = `tests/test_pico8_`), verdict quand la classe n'est pas mécanique. Chaque site se lit `fichier:ligne fonction` (sans `src/pico8_` ni `pico8_`).

| Type | Champ | Cl. | Construction (`src/`) | Hors construction (`src/`) | `tests/` | Verdict |
|---|---|---|---|---|---|---|
| `bg` | `fa` | A | bg.c:70 bg_struct_new | — | — |  |
| `bg` | `has_m_` | E | bg.c:75 bg_struct_new | bg.c:53 bg_set_m_ | — |  |
| `bg` | `hd` | A | bg.c:71 bg_struct_new | — | — |  |
| `bg` | `m_` | E | bg.c:74 bg_struct_new | bg.c:52 bg_set_m_ | — |  |
| `bg` | `mq` | E | bg.c:73 bg_struct_new | bg.c:199 bg_rdown; bg.c:229 bg_hq | — |  |
| `bg` | `pl` | A | bg.c:72 bg_struct_new | — | — |  |
| `bv_et` | `fo` | E | — | probe.c:139 probe_et_hang_fo | — | champ d'une instance statique, écrit par `pico8_probe_et_hang_fo` |
| `fm_placeholder` | `has_jk` | E | probe.c:504 probe_init | cheat.c:31 apply | — | idem `jk` |
| `fm_placeholder` | `has_nz` | E | probe.c:506 probe_init | cheat.c:33 apply | — | idem `jk` |
| `fm_placeholder` | `jk` | E | probe.c:503 probe_init | cheat.c:30 apply | — | champ du remplaçant statique `g_fm_placeholder`, remis à 0 par `pico8_probe_init`, écrit par `apply` |
| `fm_placeholder` | `nz` | E | probe.c:505 probe_init | cheat.c:32 apply | — | idem `jk` |
| `fq` | `fa` | A | ui_button.c:356 fq_struct_new | — | — |  |
| `fq` | `has_m_` | E | ui_button.c:362 fq_struct_new | ui_button.c:341 fq_set_m_ | — |  |
| `fq` | `has_p_` | E | ui_button.c:360 fq_struct_new | ui_button.c:401 fq_ldown; ui_button.c:404 fq_ldown | — |  |
| `fq` | `hd` | A | ui_button.c:357 fq_struct_new | — | — |  |
| `fq` | `m_` | E | ui_button.c:361 fq_struct_new | ui_button.c:340 fq_set_m_ | — |  |
| `fq` | `my` | K | ui_button.c:358 fq_struct_new; ui_button.c:364 fq_struct_new | — | — |  |
| `fq` | `p_` | E | ui_button.c:359 fq_struct_new | ui_button.c:400 fq_ldown; ui_button.c:403 fq_ldown | — |  |
| `fr` | `mt` | A | planet_family_trade.c:268 fr_struct_new | — | — |  |
| `fv` | `ca` | A | help.c:168 fv_struct_new | — | — |  |
| `fv` | `eo` | A | help.c:170 fv_struct_new | — | — |  |
| `fv` | `ff` | A | help.c:171 fv_struct_new | — | — |  |
| `fv` | `fg` | A | help.c:174 fv_struct_new | — | — |  |
| `fv` | `gq` | A | help.c:169 fv_struct_new | — | — |  |
| `fv` | `has_m_` | E | help.c:153 fv_struct_new | help.c:121 fv_set_m_ | — |  |
| `fv` | `hi` | A | help.c:167 fv_struct_new | — | — |  |
| `fv` | `kq` | A | help.c:172 fv_struct_new | — | — |  |
| `fv` | `m_` | E | help.c:152 fv_struct_new | help.c:120 fv_set_m_ | — |  |
| `fv` | `ql` | A | help.c:173 fv_struct_new | — | — |  |
| `fv` | `rb` | A | help.c:175 fv_struct_new | — | — |  |
| `g_` | `emitter` | K | probe.c:442 g__struct_init | — | — |  |
| `g_` | `emitter_own` | K | probe.c:442 g__struct_init (fuite) | — | — | stockage propre du composant ; fuite `&self->emitter_own` suivie : ses écritures sont celles de `pico8_emitter` |
| `g_` | `fa` | A | help.c:154 fv_struct_new; probe.c:457 g__struct_init | — | T:g_.c:751 test_g__is_a_struct | écrit seulement par les inits ; `test_pico8_g_.c:751` le met à 0 pour voir `is_probe` tomber : 2.13 le réécrit sur une classe d'archétype `fa` nul |
| `g_` | `fx` | A | probe.c:450 g__struct_init | — | — |  |
| `g_` | `has_p_` | E | probe.c:449 g__struct_init | probe.c:290 probe_host_raise_p_ | — |  |
| `g_` | `p` | E | probe.c:445 g__struct_init | probe.c:89 probe_host_set_ra_p | — |  |
| `g_` | `p_` | E | probe.c:448 g__struct_init | probe.c:289 probe_host_raise_p_ | — |  |
| `g_` | `probe` | K | probe.c:446 g__struct_init | — | — |  |
| `g_` | `probe_own` | K | probe.c:446 g__struct_init (fuite) | — | — | stockage propre du composant ; fuite `&self->probe_own` suivie : ses écritures sont celles de `pico8_probe` |
| `g_` | `ra` | I | help.c:155 fv_struct_new; probe.c:444 g__struct_init | probe.c:88 probe_host_set_ra_p | — |  |
| `ga` | `emitter` | K | planet_family.c:571 ga_struct_new | — | — |  |
| `ga` | `emitter_own` | K | planet_family.c:571 ga_struct_new (fuite) | — | — | stockage propre du composant ; fuite `&self->emitter_own` suivie : ses écritures sont celles de `pico8_emitter` |
| `ga` | `fz` | A | planet_family.c:570 ga_struct_new | — | — |  |
| `ga` | `iu` | I | planet_family.c:582 ga_struct_new; planet_family.c:606 ga_struct_new | — | — | défaut `_start`, réécrit par clé de construction dans le `_new`, jamais après |
| `ga` | `ms` | I | planet_family.c:584 ga_struct_new; planet_family.c:608 ga_struct_new | — | — | défaut `_start`, réécrit par clé de construction dans le `_new`, jamais après |
| `hj` | `emitter` | K | planet_family.c:290 hj_struct_init | — | — |  |
| `hj` | `emitter_own` | K | planet_family.c:290 hj_struct_init (fuite) | — | — | stockage propre du composant ; fuite `&self->emitter_own` suivie : ses écritures sont celles de `pico8_emitter` |
| `hj` | `rr` | A | planet_family.c:289 hj_struct_init; planet_family_trade.c:375 mx_struct_new | — | — |  |
| `hp` | `cw` | K | research.c:154 hp_struct_new; research.c:169 hp_struct_new | — | — |  |
| `hp` | `dm` | K | research.c:108 hp_positions; research.c:149 hp_struct_new | — | — |  |
| `hp` | `fj` | K | research.c:90 hp_positions; research.c:141 hp_struct_new | — | T:js.c:1604 test_fe_free_row |  |
| `hp` | `fk` | A | research.c:138 hp_struct_new; research.c:159 hp_struct_new | — | — |  |
| `hp` | `go` | K | research.c:155 hp_struct_new; research.c:170 hp_struct_new | — | — |  |
| `hp` | `has_dm` | K | research.c:109 hp_positions; research.c:150 hp_struct_new | — | — | posé à 1 par `hp_positions` ou le `_new` selon l'enregistrement `ri` : valeur de l'instance |
| `hp` | `has_fj` | K | research.c:91 hp_positions; research.c:142 hp_struct_new | — | T:js.c:1603 test_fe_free_row | posé à 1 par `hp_positions` ou le `_new` selon l'enregistrement `ri` : valeur de l'instance |
| `hp` | `has_ii` | K | research.c:153 hp_struct_new; research.c:167 hp_struct_new | — | — | posé à 1 par `hp_positions` ou le `_new` selon l'enregistrement `ri` : valeur de l'instance |
| `hp` | `has_il` | K | research.c:105 hp_positions; research.c:148 hp_struct_new | — | — | posé à 1 par `hp_positions` ou le `_new` selon l'enregistrement `ri` : valeur de l'instance |
| `hp` | `has_o` | K | research.c:101 hp_positions; research.c:146 hp_struct_new | — | — | posé à 1 par `hp_positions` ou le `_new` selon l'enregistrement `ri` : valeur de l'instance |
| `hp` | `has_rn` | K | research.c:157 hp_struct_new; research.c:173 hp_struct_new | — | — | posé à 1 par `hp_positions` ou le `_new` selon l'enregistrement `ri` : valeur de l'instance |
| `hp` | `i` | K | research.c:144 hp_struct_new; research.c:214 hp_struct_new | — | — |  |
| `hp` | `ii` | K | research.c:152 hp_struct_new; research.c:166 hp_struct_new | — | — |  |
| `hp` | `il` | K | research.c:104 hp_positions; research.c:147 hp_struct_new | — | — |  |
| `hp` | `ja` | K | research.c:94 hp_positions; research.c:143 hp_struct_new | — | — |  |
| `hp` | `lq` | K | research.c:158 hp_struct_new; research.c:195 hp_struct_new | — | — | NULL puis `&self->lq_own` dans le `_new` |
| `hp` | `lq_inner` | K | research.c:190 hp_struct_new | — | — |  |
| `hp` | `lq_inner_items` | K | research.c:188 hp_struct_new; research.c:189 hp_struct_new | — | — |  |
| `hp` | `lq_own` | K | research.c:194 hp_struct_new | — | — |  |
| `hp` | `lq_own_items` | K | research.c:191 hp_struct_new; research.c:192 hp_struct_new; research.c:193 hp_struct_new | — | — |  |
| `hp` | `lr` | K | research.c:87 hp_positions; research.c:140 hp_struct_new | — | — |  |
| `hp` | `o` | K | research.c:100 hp_positions; research.c:145 hp_struct_new | — | — |  |
| `hp` | `px` | K | research.c:84 hp_positions; research.c:139 hp_struct_new | — | — |  |
| `hp` | `rn` | K | research.c:156 hp_struct_new; research.c:172 hp_struct_new | — | — |  |
| `hp` | `rr` | K | research.c:151 hp_struct_new; research.c:177 hp_struct_new | — | — |  |
| `hy` | `ew` | I | economy.c:30 hy_struct_new; economy.c:46 hy_struct_new | economy.c:294 hy_kt | — | défaut littéral non nul du constructeur, identique pour toute instance, puis réécrit : I (la classe E est réservée à nil/0/NULL) ; archétype `pico8_hy_def_t` |
| `hy` | `fu` | E | economy.c:35 hy_struct_new | economy.c:212 hy_lj | — |  |
| `hy` | `fu_own` | E | economy.c:37 hy_struct_new | economy.c:210 hy_lj (fuite); economy.c:212 hy_lj (fuite) | — | stockage de liste ; fuite dans `pico8_hy_lj` suivie : la liste y est remplie |
| `hy` | `ge` | E | economy.c:28 hy_struct_new; economy.c:44 hy_struct_new | economy.c:234 hy_lj | — |  |
| `hy` | `happiness` | E | economy.c:38 hy_struct_new | economy.c:237 hy_lj | T:pj.c:716 test_ky_reads_invented_and_happiness; T:pj.c:751 test_ph_rows_show_ky |  |
| `hy` | `has_happiness` | E | economy.c:39 hy_struct_new | economy.c:238 hy_lj | T:pj.c:717 test_ky_reads_invented_and_happiness; T:pj.c:752 test_ph_rows_show_ky |  |
| `hy` | `has_hl` | E | economy.c:41 hy_struct_new | economy.c:249 hy_lj | T:js.c:564 game_end; T:pj.c:405 test_shared; T:pj.c:458 test_life; T:pj.c:754 test_ph_rows_show_ky |  |
| `hy` | `hl` | E | economy.c:40 hy_struct_new | economy.c:243 hy_lj; economy.c:245 hy_lj; economy.c:247 hy_lj | T:js.c:563 game_end; T:pj.c:404 test_shared; T:pj.c:457 test_life; T:pj.c:753 test_ph_rows_show_ky |  |
| `hy` | `invented` | K | economy.c:31 hy_struct_new; economy.c:50 hy_struct_new | — | — |  |
| `hy` | `jk` | E | economy.c:29 hy_struct_new; economy.c:45 hy_struct_new | cheat.c:37 apply; economy.c:136 hy_hk; economy.c:280 hy_kt | T:hy.c:569 test_kt; T:js.c:553 tech_screen; T:js.c:1664 test_cb_writes_every_e_; T:js.c:1680 test_cb_writes_every_e_ |  |
| `hy` | `lo` | I | economy.c:32 hy_struct_new; economy.c:51 hy_struct_new | economy.c:99 hy_nx; economy.c:104 hy_nx; menu.c:252 ix1_my_dispatch | — | défaut littéral non nul du constructeur, identique pour toute instance, puis réécrit : I (la classe E est réservée à nil/0/NULL) ; archétype `pico8_hy_def_t` |
| `hy` | `nz` | I | economy.c:27 hy_struct_new; economy.c:43 hy_struct_new | cheat.c:38 apply; economy.c:135 hy_hk; economy.c:171 hy_lu | T:fr.c:380 set_credits; T:hj.c:446 set_credits; T:hy.c:520 test_lj; T:hy.c:526 test_lj; T:hy.c:531 test_lj; T:mx.c:452 set_credits; T:mz.c:566 test_ey; T:nl.c:384 set_credits; T:nr.c:389 set_credits; T:os.c:409 set_credits; T:qh.c:373 set_credits | défaut littéral non nul du constructeur, identique pour toute instance, puis réécrit : I (la classe E est réservée à nil/0/NULL) ; archétype `pico8_hy_def_t` |
| `hy` | `po` | E | economy.c:34 hy_struct_new | economy.c:211 hy_lj | — |  |
| `hy` | `po_own` | E | economy.c:36 hy_struct_new | economy.c:209 hy_lj (fuite); economy.c:211 hy_lj (fuite) | — | idem `fu_own` |
| `hy` | `yr` | I | economy.c:33 hy_struct_new; economy.c:52 hy_struct_new | economy.c:138 hy_hk | T:hy.c:514 test_lj; T:hy.c:519 test_lj | défaut littéral non nul du constructeur, identique pour toute instance, puis réécrit : I (la classe E est réservée à nil/0/NULL) ; archétype `pico8_hy_def_t` |
| `ie_ke` | `has_od` | K | research_icon.c:61 ie_ke_view; research_icon.c:68 ie_ke_view | research_icon.c:171 ie_kj (struct entière) | T:ie.c:672 test_ke_view_is_a_struct (struct entière); T:ie.c:690 test_ke_view_is_a_struct (struct entière) | vue construite par `pico8_ie_ke_view` depuis `kind->rec` ; l'« écriture » de `pico8_ie_kj` est l'affectation de la vue entière à une locale |
| `ie_ke` | `has_s` | K | research_icon.c:58 ie_ke_view; research_icon.c:65 ie_ke_view | research_icon.c:171 ie_kj (struct entière) | T:ie.c:672 test_ke_view_is_a_struct (struct entière); T:ie.c:690 test_ke_view_is_a_struct (struct entière) | vue construite par `pico8_ie_ke_view` depuis `kind->rec` ; l'« écriture » de `pico8_ie_kj` est l'affectation de la vue entière à une locale |
| `ie_ke` | `me` | K | research_icon.c:59 ie_ke_view; research_icon.c:66 ie_ke_view | research_icon.c:171 ie_kj (struct entière) | T:ie.c:672 test_ke_view_is_a_struct (struct entière); T:ie.c:690 test_ke_view_is_a_struct (struct entière) | vue construite par `pico8_ie_ke_view` depuis `kind->rec` ; l'« écriture » de `pico8_ie_kj` est l'affectation de la vue entière à une locale |
| `ie_ke` | `od` | K | research_icon.c:60 ie_ke_view; research_icon.c:67 ie_ke_view | research_icon.c:171 ie_kj (struct entière) | T:ie.c:672 test_ke_view_is_a_struct (struct entière); T:ie.c:690 test_ke_view_is_a_struct (struct entière) | vue construite par `pico8_ie_ke_view` depuis `kind->rec` ; l'« écriture » de `pico8_ie_kj` est l'affectation de la vue entière à une locale |
| `ie_ke` | `rr` | K | research_icon.c:56 ie_ke_view; research_icon.c:63 ie_ke_view | research_icon.c:171 ie_kj (struct entière) | T:ie.c:672 test_ke_view_is_a_struct (struct entière); T:ie.c:690 test_ke_view_is_a_struct (struct entière) | vue construite par `pico8_ie_ke_view` depuis `kind->rec` ; l'« écriture » de `pico8_ie_kj` est l'affectation de la vue entière à une locale |
| `ie_ke` | `s` | K | research_icon.c:57 ie_ke_view; research_icon.c:64 ie_ke_view | research_icon.c:171 ie_kj (struct entière) | T:ie.c:672 test_ke_view_is_a_struct (struct entière); T:ie.c:690 test_ke_view_is_a_struct (struct entière) | vue construite par `pico8_ie_ke_view` depuis `kind->rec` ; l'« écriture » de `pico8_ie_kj` est l'affectation de la vue entière à une locale |
| `ie` | `eh` | A | research_icon.c:244 ie_struct_new | — | — |  |
| `ie` | `fa` | A | research_icon.c:242 ie_struct_new | — | — |  |
| `ie` | `has_kh` | E | research_icon.c:248 ie_struct_new | research_icon.c:103 ie_hb | — |  |
| `ie` | `has_m_` | E | research_icon.c:250 ie_struct_new | research_icon.c:222 ie_set_m_ | T:ie.c:486 test_draw; T:ie.c:492 test_draw; T:ie.c:497 test_draw; T:ie.c:555 test_ke_view; T:ie.c:558 test_ke_view |  |
| `ie` | `jb` | K | research_icon.c:246 ie_struct_new; research_icon.c:253 ie_struct_new | — | — |  |
| `ie` | `ke` | K | research_icon.c:245 ie_struct_new; research_icon.c:252 ie_struct_new | — | — |  |
| `ie` | `kh` | E | research_icon.c:247 ie_struct_new | research_icon.c:102 ie_hb | — |  |
| `ie` | `kq` | A | research_icon.c:241 ie_struct_new | — | — |  |
| `ie` | `m_` | E | research_icon.c:249 ie_struct_new | research_icon.c:221 ie_set_m_ | T:ie.c:485 test_draw; T:ie.c:491 test_draw; T:ie.c:496 test_draw; T:ie.c:554 test_ke_view; T:ie.c:557 test_ke_view |  |
| `ie` | `ra` | I | research_icon.c:243 ie_struct_new | research_icon.c:101 ie_hb | — | défaut `&pico8_ie_start.ra`, réécrit par `pico8_ie_hb` vers `g_ie_gy` (état de classe statique, règle 1) |
| `iv` | `at1` | K | msgbox.c:264 iv_struct_new; msgbox.c:268 iv_struct_new | — | — |  |
| `iv` | `at2` | K | msgbox.c:265 iv_struct_new; msgbox.c:269 iv_struct_new | — | — |  |
| `iv` | `fa` | A | msgbox.c:258 iv_struct_new | — | — |  |
| `iv` | `has_p_` | E | msgbox.c:263 iv_struct_new | msgbox.c:199 iv_ph | — |  |
| `iv` | `l` | I | msgbox.c:259 iv_struct_new | msgbox.c:196 iv_ph | — |  |
| `iv` | `p_` | E | msgbox.c:262 iv_struct_new | msgbox.c:198 iv_ph | — |  |
| `iv` | `ra` | E | msgbox.c:261 iv_struct_new; msgbox.c:267 iv_struct_new | msgbox.c:176 iv_pq; msgbox.c:193 iv_ph | — |  |
| `iv` | `v` | A | msgbox.c:260 iv_struct_new | — | — |  |
| `jb` | `ls` | E | layout.c:413 jb_struct_alloc | layout.c:245 jb_pq | — |  |
| `jb` | `ls_count` | E | layout.c:414 jb_struct_alloc | layout.c:250 jb_pq | — |  |
| `jc_mouse_point` | `p` | K | — | slipway.c:135 jc_hq | — | objet transitoire construit en ligne par `pico8_jc_hq`, jamais réécrit |
| `jc_mouse_point` | `ra` | K | — | slipway.c:132 jc_hq | — | objet transitoire construit en ligne par `pico8_jc_hq`, jamais réécrit |
| `jc` | `fa` | I | slipway.c:403 jc_struct_new | slipway.c:151 jc_hq | — |  |
| `jc` | `fo` | E | slipway.c:421 jc_struct_new | probe.c:125 probe_et_hang_fo; slipway_display.c:171 jc_kj | T:jc.c:632 test_draw |  |
| `jc` | `has_mt` | E | slipway.c:419 jc_struct_new | slipway.c:144 jc_hq | T:jc.c:468 check_fresh_jc |  |
| `jc` | `has_p_` | E | slipway.c:423 jc_struct_new | probe.c:302 probe_host_raise_p_; slipway.c:173 jc_ey | — |  |
| `jc` | `hg` | E | slipway.c:414 jc_struct_new | slipway.c:112 jc_pq | — |  |
| `jc` | `hg_own` | E | — | slipway.c:111 jc_pq; slipway.c:112 jc_pq (fuite) | — | stockage de liste, rempli par `pico8_jc_pq` (écriture et fuite) |
| `jc` | `k_` | A | slipway.c:410 jc_struct_new | — | — |  |
| `jc` | `mt` | E | slipway.c:418 jc_struct_new | slipway.c:143 jc_hq; slipway.c:144 jc_hq (fuite); slipway.c:144 jc_hq (fuite) | T:jc.c:467 check_fresh_jc | fuites `jc_mt_of(…, &self->mt)` suivies : `jc_mt_of` écrit par ce pointeur |
| `jc` | `nh` | I | slipway.c:411 jc_struct_new | slipway.c:177 jc_ey; slipway_display.c:117 jc_kj | — |  |
| `jc` | `p_` | E | slipway.c:422 jc_struct_new | probe.c:301 probe_host_raise_p_; slipway.c:172 jc_ey | — |  |
| `jc` | `probe` | K | slipway.c:404 jc_struct_new | — | — |  |
| `jc` | `probe_own` | K | slipway.c:404 jc_struct_new (fuite) | — | — | stockage propre du composant ; fuite `&self->probe_own` suivie : ses écritures sont celles de `pico8_probe` |
| `jc` | `qu` | K | slipway.c:412 jc_struct_new; slipway.c:428 jc_struct_new | — | — |  |
| `jc` | `re` | E | slipway.c:415 jc_struct_new | slipway.c:127 jc_hq; slipway.c:136 jc_hq; slipway.c:178 jc_ey | T:g_.c:507 a_slipway; T:nr.c:607 test_cn_skips_a_slipway_already_in_rl |  |
| `jc` | `ts` | E | slipway.c:413 jc_struct_new | slipway.c:109 jc_pq | — |  |
| `jc_too_far` | `fo` | E | — | probe.c:132 probe_et_hang_fo | — | idem `bv_et.fo` |
| `jc_too_far` | `qs` | A | 0 erreur | — | — | 0 erreur : initialiseur de l'instance statique `g_jc_too_far` (piège 4), jamais réécrit |
| `jo` | `by` | I | planet.c:169 jo_struct_init; planet_family.c:586 ga_struct_new | economy_tiers.c:69 bd_route_visit | T:os.c:463 export_to | NULL, sauf `ga` : `&g_ga_by_shared` (liste statique partagée, S) ; réécrit par `bd_route_visit` |
| `jo` | `ca` | A | planet.c:148 jo_struct_init | — | — |  |
| `jo` | `cc` | E | planet.c:168 jo_struct_init | economy_tiers.c:53 bd_reset_visit | T:trade.c:560 test_dh_counts_numbered_routes_only |  |
| `jo` | `cq` | A | planet.c:145 jo_struct_init | — | — |  |
| `jo` | `dz` | A | planet.c:144 jo_struct_init | — | — |  |
| `jo` | `em` | A | planet.c:149 jo_struct_init | — | — |  |
| `jo` | `ep` | E | planet.c:155 jo_struct_init | planet.c:315 jo_lz; planet.c:448 jo_hh | — |  |
| `jo` | `fa` | A | planet.c:140 jo_struct_init; planet_family.c:589 ga_struct_new | — | — |  |
| `jo` | `fo` | E | planet.c:161 jo_struct_init | planet_display.c:480 jo_kj; probe.c:117 probe_et_hang_fo | T:jc.c:644 test_draw |  |
| `jo` | `fx` | E | planet.c:158 jo_struct_init | planet.c:407 jo_hu; planet_trade.c:64 jo_kf | T:mz.c:406 test_ox; T:mz.c:423 test_ox |  |
| `jo` | `gq` | A | planet.c:147 jo_struct_init | — | T:ga_export_panel_by.c:112 test_ga_export_line_counts_the_routes_in_by; T:lq.c:782 test_gh_wants_more; T:lq.c:824 test_gh_w_maker_wants_no_more; T:lq.c:855 test_gh_lab_wants_its_last; T:planet_gh_row_lq.c:121 test_gh_exports_row_lq_receives_its_four_items | écrit seulement par l'init ; les 5 écritures sont des TU (dont 2 de régression) : `gq` n'y sert qu'au placement de la ligne, jamais asserté ; 2.13 retire l'écriture |
| `jo` | `has_fx` | E | planet.c:159 jo_struct_init | planet.c:408 jo_hu; planet_trade.c:65 jo_kf | T:mz.c:407 test_ox; T:mz.c:424 test_ox |  |
| `jo` | `has_jh` | E | planet.c:163 jo_struct_init | planet_trade.c:137 jo_cn | — |  |
| `jo` | `has_kr` | E | planet.c:167 jo_struct_init | economy.c:226 hy_lj | — |  |
| `jo` | `has_m_` | E | planet.c:173 jo_struct_init | planet.c:112 jo_set_m_ | — |  |
| `jo` | `has_mi` | I | planet.c:171 jo_struct_init; planet_family.c:79 qh_struct_init | economy_tiers.c:170 bb_visit | T:hj.c:536 test_life; T:lq.c:776 test_gh_wants_more; T:lq.c:818 test_gh_w_maker_wants_no_more; T:lq.c:849 test_gh_lab_wants_its_last; T:mx.c:539 test_life; T:nl.c:458 test_life; T:nl.c:462 test_life; T:os.c:477 fold_db; T:pm.c:518 test_life | drapeau de `mi` : 1 posé avec `pico8_qh_start.mi`, réécrit par `bb_visit` |
| `jo` | `has_p_` | E | planet.c:165 jo_struct_init | planet.c:353 jo_ll; planet.c:393 jo_hu; planet_family.c:529 ga_kj; probe.c:296 probe_host_raise_p_ | — |  |
| `jo` | `hf` | A | planet.c:138 jo_struct_init; planet_family.c:80 qh_struct_init; planet_family.c:587 ga_struct_new | — | — |  |
| `jo` | `hn` | E | planet.c:154 jo_struct_init | planet.c:314 jo_lz | — |  |
| `jo` | `hs` | A | planet.c:139 jo_struct_init; planet_family.c:220 os_struct_new | — | — |  |
| `jo` | `id` | K | planet.c:150 jo_struct_init; planet.c:190 jo_struct_new; planet_family.c:94 qh_struct_new; planet_family.c:170 nl_struct_new; planet_family.c:226 os_struct_new; planet_family.c:320 hj_struct_new; planet_family.c:421 pm_struct_new; planet_family.c:598 ga_struct_new; planet_family_trade.c:211 nr_struct_new; planet_family_trade.c:278 fr_struct_new; planet_family_trade.c:396 mx_struct_new | — | — |  |
| `jo` | `jh` | E | planet.c:162 jo_struct_init | planet_trade.c:136 jo_cn | — |  |
| `jo` | `jy` | E | planet.c:156 jo_struct_init | planet.c:323 jo_lz; planet_trade.c:133 jo_cn | T:hy.c:544 test_lj; T:hy.c:547 test_lj; T:lq.c:775 test_gh_wants_more; T:lq.c:816 test_gh_w_maker_wants_no_more; T:lq.c:848 test_gh_lab_wants_its_last; T:mx.c:573 give_row; T:nr.c:604 test_cn_skips_a_slipway_already_in_rl; T:trade.c:502 test_cn_wildcard_want; T:trade.c:523 test_cn_nil_want_after_kg; T:trade.c:559 test_dh_counts_numbered_routes_only |  |
| `jo` | `ka` | A | planet.c:143 jo_struct_init | — | — |  |
| `jo` | `kq` | A | planet.c:141 jo_struct_init; planet_family.c:588 ga_struct_new; planet_family_trade.c:201 nr_struct_init | — | — |  |
| `jo` | `kr` | E | planet.c:166 jo_struct_init | economy.c:225 hy_lj | — |  |
| `jo` | `kw` | I | planet.c:146 jo_struct_init | planet.c:328 jo_lz | T:ga.c:592 test_life; T:ga_export_panel_by.c:105 test_ga_export_line_counts_the_routes_in_by; T:lq.c:817 test_gh_w_maker_wants_no_more; T:planet_gh_row_lq.c:113 test_gh_exports_row_lq_receives_its_four_items | défaut `&g_jo_kw_shared`, objet statique partagé réécrit en place (S, hors de la liste), repointé sur `kw_own` par `pico8_jo_lz` |
| `jo` | `kw_own` | E | planet.c:174 jo_struct_init | planet.c:325 jo_lz; planet.c:326 jo_lz; planet.c:327 jo_lz; planet.c:328 jo_lz (fuite) | — | stockage de la table que `jo_lz` crée ; lu seulement par `kw`, après `lz` ; la valeur initiale n'est jamais lue (corrigé à 2.12e1) |
| `jo` | `lq` | E | planet.c:160 jo_struct_init | planet_display.c:56 jo_hb | — |  |
| `jo` | `m_` | E | planet.c:172 jo_struct_init | planet.c:111 jo_set_m_ | — |  |
| `jo` | `ma` | I | planet.c:142 jo_struct_init | entity_geom.c:79 ma (fuite); planet.c:345 jo_kd | — | fuite `&jo->ma` de `pico8_ma` (`entity_geom.c:79`) suivie : `*ma = clamped` y écrit |
| `jo` | `mi` | I | planet.c:170 jo_struct_init; planet_family.c:78 qh_struct_init | economy_tiers.c:169 bb_visit | T:hj.c:535 test_life; T:lq.c:777 test_gh_wants_more; T:lq.c:819 test_gh_w_maker_wants_no_more; T:lq.c:850 test_gh_lab_wants_its_last; T:mx.c:538 test_life; T:nl.c:457 test_life; T:nl.c:461 test_life; T:os.c:476 fold_db; T:pm.c:517 test_life |  |
| `jo` | `nk` | E | planet.c:157 jo_struct_init | planet.c:403 jo_hu; planet.c:447 jo_hh | — |  |
| `jo` | `oe` | E | planet.c:153 jo_struct_init | planet.c:278 jo_pq | T:fr.c:507 test_life; T:fr.c:516 test_life; T:nr.c:493 test_life; T:nr.c:502 test_life |  |
| `jo` | `p` | E | planet.c:151 jo_struct_init; planet.c:184 jo_struct_new; planet_family.c:415 pm_struct_new; planet_family.c:592 ga_struct_new; planet_family_trade.c:214 nr_struct_new | probe.c:95 probe_host_set_ra_p | — |  |
| `jo` | `p_` | E | planet.c:164 jo_struct_init | planet.c:352 jo_ll; planet.c:392 jo_hu; planet_family.c:528 ga_kj; probe.c:295 probe_host_raise_p_ | — |  |
| `jo` | `ra` | E | planet.c:152 jo_struct_init; planet.c:187 jo_struct_new; planet_family.c:418 pm_struct_new; planet_family.c:595 ga_struct_new; planet_family_trade.c:217 nr_struct_new | probe.c:94 probe_host_set_ra_p | T:ga_export_panel_by.c:109 test_ga_export_line_counts_the_routes_in_by; T:lq.c:778 test_gh_wants_more; T:lq.c:820 test_gh_w_maker_wants_no_more; T:lq.c:851 test_gh_lab_wants_its_last; T:planet_gh_row_lq.c:118 test_gh_exports_row_lq_receives_its_four_items; T:planet_kj_dispatch.c:86 test_dc_kj_dispatches_mo_md_then_real_jo_kj |  |
| `js` | `_cr_t` | K | ui_button.c:130 js_struct_new; ui_button.c:152 js_struct_new | — | — |  |
| `js` | `_jo_fe_b` | K | ui_button.c:132 js_struct_new; ui_button.c:154 js_struct_new | — | — |  |
| `js` | `_jo_fe_od` | K | ui_button.c:133 js_struct_new; ui_button.c:155 js_struct_new | — | — |  |
| `js` | `_jo_fe_pt` | K | ui_button.c:131 js_struct_new; ui_button.c:153 js_struct_new | — | — |  |
| `js` | `en` | K | ui_button.c:128 js_struct_new; ui_button.c:150 js_struct_new | — | — |  |
| `js` | `fa` | I | ui_button.c:135 js_struct_new; ui_button.c:138 js_struct_new | — | — | défaut `_start`, réécrit par clé de construction dans le `_new`, jamais après |
| `js` | `fg` | K | ui_button.c:123 js_struct_new; ui_button.c:145 js_struct_new | — | — |  |
| `js` | `h_` | E | ui_button.c:113 js_struct_new | ui_button.c:215 js_pq; ui_button.c:218 js_pq | T:js.c:1347 fold_keys |  |
| `js` | `has__jo_fe_od` | K | ui_button.c:134 js_struct_new; ui_button.c:156 js_struct_new | — | — |  |
| `js` | `has_en` | K | ui_button.c:129 js_struct_new; ui_button.c:151 js_struct_new | — | — |  |
| `js` | `has_fg` | K | ui_button.c:124 js_struct_new; ui_button.c:146 js_struct_new | — | — |  |
| `js` | `has_h_` | E | ui_button.c:114 js_struct_new | ui_button.c:216 js_pq; ui_button.c:219 js_pq | — |  |
| `js` | `has_m_` | E | ui_button.c:121 js_struct_new | ui_button.c:88 js_set_m_ | T:js.c:1004 test_draw; T:js.c:1009 test_draw; T:js.c:1219 write_m_ |  |
| `js` | `has_my` | K | ui_button.c:119 js_struct_new; ui_button.c:143 js_struct_new | — | — |  |
| `js` | `has_trade` | K | ui_button.c:127 js_struct_new; ui_button.c:149 js_struct_new | — | — |  |
| `js` | `hb` | K | ui_button.c:122 js_struct_new; ui_button.c:144 js_struct_new | — | — |  |
| `js` | `jb` | E | ui_button.c:115 js_struct_new | ui_button.c:222 js_pq | T:js.c:999 test_draw |  |
| `js` | `kq` | E | ui_button.c:112 js_struct_new | ui_button.c:211 js_pq | — |  |
| `js` | `lq` | E | ui_button.c:110 js_struct_new | menu.c:285 ix2_hb_dispatch; ui_button.c:209 js_pq | T:js.c:1256 fold_written; T:js.c:1259 fold_written; T:js.c:1261 fold_written; T:js.c:1265 fold_written |  |
| `js` | `m_` | E | ui_button.c:120 js_struct_new | ui_button.c:87 js_set_m_ | T:js.c:1003 test_draw; T:js.c:1008 test_draw; T:js.c:1218 write_m_ |  |
| `js` | `mb` | E | ui_button.c:111 js_struct_new | ui_button.c:210 js_pq | — |  |
| `js` | `my` | K | ui_button.c:118 js_struct_new; ui_button.c:142 js_struct_new | — | — |  |
| `js` | `px` | K | ui_button.c:125 js_struct_new; ui_button.c:147 js_struct_new | — | — |  |
| `js` | `qf` | K | ui_button.c:117 js_struct_new; ui_button.c:141 js_struct_new | — | — |  |
| `js` | `ra` | E | ui_button.c:116 js_struct_new; ui_button.c:140 js_struct_new | ui_button.c:240 js_ph | — |  |
| `js` | `trade` | K | ui_button.c:126 js_struct_new; ui_button.c:148 js_struct_new | — | — |  |
| `lq` | `bg` | K | layout.c:308 lq_struct_alloc; layout.c:344 lq_struct_new; layout.c:382 lq_copy | — | — |  |
| `lq` | `bg_is_bool` | K | layout.c:310 lq_struct_alloc; layout.c:346 lq_struct_new; layout.c:384 lq_copy | — | — |  |
| `lq` | `c` | K | layout.c:306 lq_struct_alloc; layout.c:340 lq_struct_new; layout.c:380 lq_copy | — | — |  |
| `lq` | `fs` | E | layout.c:314 lq_struct_alloc | layout.c:142 lq_pq | — |  |
| `lq` | `fs_count` | E | layout.c:315 lq_struct_alloc | layout.c:154 lq_pq | — |  |
| `lq` | `has_bg` | K | layout.c:309 lq_struct_alloc; layout.c:345 lq_struct_new; layout.c:383 lq_copy | — | — |  |
| `lq` | `has_c` | K | layout.c:307 lq_struct_alloc; layout.c:341 lq_struct_new; layout.c:381 lq_copy | — | — |  |
| `lq` | `has_score` | K | layout.c:312 lq_struct_alloc; layout.c:350 lq_struct_new; layout.c:386 lq_copy | — | — |  |
| `lq` | `items` | E | layout.c:317 lq_struct_alloc | layout.c:396 lq_add | — |  |
| `lq` | `items_count` | E | layout.c:319 lq_struct_alloc | layout.c:398 lq_add | — |  |
| `lq` | `ra` | K | layout.c:313 lq_struct_alloc; layout.c:352 lq_struct_new; layout.c:387 lq_copy | — | — |  |
| `lq` | `score` | K | layout.c:311 lq_struct_alloc; layout.c:349 lq_struct_new; layout.c:385 lq_copy | — | — |  |
| `lq` | `w` | E | layout.c:316 lq_struct_alloc | layout.c:155 lq_pq | — |  |
| `mh` | `fa` | A | msgbox.c:111 mh_struct_new | — | — |  |
| `mh` | `has_p_` | E | msgbox.c:115 mh_struct_new | msgbox.c:42 mh_pq; msgbox.c:82 mh_ev | — |  |
| `mh` | `items` | K | msgbox.c:121 mh_struct_new | — | — |  |
| `mh` | `items_count` | K | msgbox.c:116 mh_struct_new; msgbox.c:121 mh_struct_new | — | — |  |
| `mh` | `lh` | K | msgbox.c:113 mh_struct_new; msgbox.c:126 mh_struct_new | — | — |  |
| `mh` | `p_` | E | msgbox.c:114 mh_struct_new | msgbox.c:41 mh_pq; msgbox.c:81 mh_ev | — |  |
| `mh` | `rh` | I | msgbox.c:112 mh_struct_new; msgbox.c:124 mh_struct_new | — | — | défaut `_start`, réécrit par clé de construction dans le `_new`, jamais après |
| `mx` | `ht` | I | planet_family_trade.c:374 mx_struct_new | planet_family_trade.c:326 mx_kf | — | défaut `pico8_mx_start.ht`, réécrit par `pico8_mx_kf` vers `&g_mx_ht_spent` |
| `mz_jj` | `qs` | A | 0 erreur | — | — | 0 erreur : initialiseur de l'instance statique (piège 4), jamais réécrit |
| `mz` | `fa` | A | wormhole.c:255 mz_struct_new | — | — |  |
| `mz` | `has_p_` | E | wormhole.c:264 mz_struct_new | probe.c:309 probe_host_raise_p_; wormhole.c:125 mz_ey; wormhole.c:171 mz_pr | — |  |
| `mz` | `ij` | E | wormhole.c:262 mz_struct_new | wormhole.c:65 mz_hq | — |  |
| `mz` | `jj` | S | wormhole.c:257 mz_struct_new | — | — | constant pour la classe, mais la valeur est `&g_mz_jj_wrapper`, `static` de `wormhole.c` dont le type contient un en-tête du noyau : `game_data.c` ne peut pas le nommer ; reste posé par l'init, hors archétype |
| `mz` | `kk` | A | wormhole.c:256 mz_struct_new | — | — |  |
| `mz` | `oi` | I | wormhole.c:251 mz_struct_new | wormhole.c:99 mz_kz | — |  |
| `mz` | `ox` | E | wormhole.c:258 mz_struct_new | wormhole.c:49 mz_pq | — |  |
| `mz` | `p` | E | wormhole.c:260 mz_struct_new | probe.c:102 probe_host_set_ra_p | T:mz.c:601 paid_mz_on_unknown |  |
| `mz` | `p_` | E | wormhole.c:263 mz_struct_new | probe.c:308 probe_host_raise_p_; wormhole.c:124 mz_ey; wormhole.c:170 mz_pr | — |  |
| `mz` | `probe` | K | wormhole.c:245 mz_struct_new | — | — |  |
| `mz` | `probe_own` | K | wormhole.c:245 mz_struct_new (fuite) | — | — | stockage propre du composant ; fuite `&self->probe_own` suivie : ses écritures sont celles de `pico8_probe` |
| `mz` | `ra` | E | wormhole.c:259 mz_struct_new | probe.c:101 probe_host_set_ra_p | — |  |
| `mz` | `spawn_snd` | A | wormhole.c:248 mz_struct_new | — | — |  |
| `nf` | `ra` | I | mouse_reader.c:47 nf_struct_new | mouse_reader.c:100 nf_ph | T:iv.c:97 fresh_world | défaut `&g_nf_ra` (vecteur statique, S), réécrit par `pico8_nf_ph` |
| `nl` | `nl` | A | planet_family.c:157 nl_struct_new | — | — |  |
| `nl` | `qw` | I | planet_family.c:158 nl_struct_new | planet_family.c:113 nl_ly | T:qh.c:533 fold_descendant |  |
| `nl` | `rr` | A | planet_family.c:156 nl_struct_new | — | — |  |
| `nr` | `hz` | A | planet_family_trade.c:195 nr_struct_init; planet_family_trade.c:269 fr_struct_new | — | — |  |
| `nr` | `rr` | A | planet_family_trade.c:196 nr_struct_init; planet_family_trade.c:270 fr_struct_new | — | — |  |
| `pj_cr` | `ca` | A | score_panel.c:340 score_panel_init | — | — | seule écriture : `pico8_score_panel_init` reconstruit l'instance statique unique depuis `pico8_pj_start.cr` |
| `pj_cr` | `ep` | E | score_panel.c:341 score_panel_init | menu.c:235 cr | — | champ de l'instance statique `g_pj_cr`, NULL à l'init du module, écrit par `pico8_cr` |
| `pj_cr` | `gq` | A | 0 erreur | — | — | 0 erreur : membre `const` de l'instance statique `g_pj_cr`, initialiseur (piège 4) |
| `pj_cr` | `hi` | A | 0 erreur | — | — | 0 erreur : membre `const` de l'instance statique `g_pj_cr`, initialiseur (piège 4) |
| `pj` | `c_` | E | score_panel.c:298 pj_struct_new | menu.c:296 ix3_my_dispatch; score_panel_display.c:98 pj_de_toggle; score_panel_display.c:279 pj_ph | T:mh.c:241 set_c_ |  |
| `pj` | `ed` | E | score_panel.c:297 pj_struct_new | score_panel_display.c:320 pj_ph | — |  |
| `pj` | `fa` | I | score_panel.c:294 pj_struct_new | score_panel_display.c:283 pj_ph | — |  |
| `pj` | `gs` | A | score_panel.c:295 pj_struct_new | — | — |  |
| `pj` | `gz` | E | score_panel.c:300 pj_struct_new | score_panel.c:251 pj_kj; score_panel_display.c:281 pj_ph | T:mh.c:264 test_scores_box_follows_c_ |  |
| `pj` | `has_c_` | E | score_panel.c:299 pj_struct_new | menu.c:297 ix3_my_dispatch; score_panel_display.c:99 pj_de_toggle; score_panel_display.c:280 pj_ph | T:mh.c:242 set_c_ |  |
| `pj` | `has_gz` | E | score_panel.c:301 pj_struct_new | score_panel_display.c:282 pj_ph | T:mh.c:265 test_scores_box_follows_c_ |  |
| `pj` | `i_` | A | score_panel.c:296 pj_struct_new | — | — |  |
| `pm` | `fz` | A | planet_family.c:397 pm_struct_new | — | — |  |
| `pm` | `ng` | A | planet_family.c:396 pm_struct_new | — | — |  |
| `qb` | `ma` | I | scroll.c:49 qb_struct_new | entity_geom.c:79 ma (fuite); planet_family.c:446 ga_pq | T:qb.c:229 test_shake_decays | même fuite `pico8_ma`, suivie jusqu'à `*ma = clamped` ; `pico8_ga_pq` l'écrit aussi |
| `qb` | `p` | I | scroll.c:50 qb_struct_new | scroll.c:127 qb_ph | — |  |
| `qb` | `pc` | A | scroll.c:52 qb_struct_new | — | — |  |
| `qb` | `ra` | I | scroll.c:51 qb_struct_new | scroll.c:132 qb_ph | — |  |
| `qb` | `v` | I | scroll.c:48 qb_struct_new | bg.c:226 bg_hq; scroll.c:128 qb_ph | T:qb.c:375 test_qb_is_a_struct |  |
| `qh` | `ba` | A | planet_family.c:76 qh_struct_init; planet_family.c:164 nl_struct_new | — | — |  |
| `qh` | `hp` | A | planet_family.c:64 qh_struct_init; planet_family.c:160 nl_struct_new; planet_family.c:215 os_struct_new; planet_family.c:303 hj_struct_init; planet_family.c:409 pm_struct_new; planet_family_trade.c:387 mx_struct_new | — | — |  |
| `qh` | `probe` | K | planet_family.c:66 qh_struct_init | — | — |  |
| `qh` | `probe_own` | K | planet_family.c:66 qh_struct_init (fuite) | — | — | stockage propre du composant ; fuite `&self->probe_own` suivie : ses écritures sont celles de `pico8_probe` |
| `qh` | `px` | A | planet_family.c:65 qh_struct_init; planet_family.c:162 nl_struct_new; planet_family.c:216 os_struct_new; planet_family.c:305 hj_struct_init; planet_family.c:410 pm_struct_new; planet_family_trade.c:389 mx_struct_new | — | — |  |
| `qh` | `qh` | A | planet_family.c:68 qh_struct_init; planet_family.c:159 nl_struct_new; planet_family.c:217 os_struct_new; planet_family.c:304 hj_struct_init | — | — |  |
| `qh` | `s` | I | planet_family.c:75 qh_struct_init; planet_family.c:161 nl_struct_new; planet_family.c:218 os_struct_new; planet_family.c:307 hj_struct_init; planet_family.c:411 pm_struct_new; planet_family_trade.c:199 nr_struct_init; planet_family_trade.c:272 fr_struct_new; planet_family_trade.c:388 mx_struct_new | planet_family_trade.c:100 nr_hb | — |  |
| `struct_header` | `cls` | K | economy.c:26 hy_struct_new; ke.c:35 ke_struct_init; layout.c:305 lq_struct_alloc; layout.c:412 jb_struct_alloc; research.c:137 hp_struct_new; research_icon.c:55 ie_ke_view; trade.c:32 trade_struct_new | research_icon.c:171 ie_kj (struct entière); slipway.c:131 jc_hq | T:ie.c:672 test_ke_view_is_a_struct (struct entière); T:ie.c:690 test_ke_view_is_a_struct (struct entière); T:planet_kj_dispatch.c:91 test_dc_kj_dispatches_mo_md_then_real_jo_kj; T:planet_kj_dispatch.c:128 test_dc_kj_dispatches_mo_md_then_real_jo_kj | la classe de l'instance, argument de `pico8_ke_struct_init` : c'est le chemin vers l'archétype, il reste dans le noyau ; les TU `planet_kj_dispatch` l'échangent pour espionner ; les 3 sites « struct entière » (`research_icon.c:171`, `T:ie.c:672`, `:690`) sont la vue `ie_ke` copiée dans une locale |
| `struct_header` | `me` | I | ke.c:36 ke_struct_init; planet.c:175 jo_struct_init; planet_family.c:81 qh_struct_init; planet_family.c:412 pm_struct_new; slipway.c:402 jc_struct_new; wormhole.c:244 mz_struct_new | ke.c:30 ke_ks; research_icon.c:171 ie_kj (struct entière) | T:ie.c:672 test_ke_view_is_a_struct (struct entière); T:ie.c:690 test_ke_view_is_a_struct (struct entière) | défaut `_start.me` de 7 classes, réécrit par `pico8_ke_ks` ; les 3 sites « struct entière » (`research_icon.c:171`, `T:ie.c:672`, `:690`) sont la vue `ie_ke` copiée dans une locale |
| `struct_header` | `t` | I | ke.c:37 ke_struct_init; probe.c:479 g__struct_new | bootstrap.c:74 dg; ke.c:31 ke_ks; msgbox.c:65 mh_ev; research_icon.c:171 ie_kj (struct entière) | T:ie.c:672 test_ke_view_is_a_struct (struct entière); T:ie.c:690 test_ke_view_is_a_struct (struct entière) | défaut `pico8_ke_start.t`, réécrit par `pico8_dg`, `pico8_ke_ks`, `pico8_mh_ev`, et par clé dans `pico8_g__struct_new` ; les 3 sites « struct entière » (`research_icon.c:171`, `T:ie.c:672`, `:690`) sont la vue `ie_ke` copiée dans une locale |
| `trade` | `has_qe` | K | trade.c:36 trade_struct_new; trade.c:46 trade_struct_new | — | — |  |
| `trade` | `lp` | K | trade.c:33 trade_struct_new; trade.c:43 trade_struct_new | — | — |  |
| `trade` | `qe` | K | trade.c:35 trade_struct_new; trade.c:45 trade_struct_new | — | — |  |
| `trade` | `qe_is_bool` | K | trade.c:37 trade_struct_new; trade.c:47 trade_struct_new | — | — |  |
| `trade` | `rl` | K | trade.c:34 trade_struct_new; trade.c:44 trade_struct_new | — | — |  |
| `ui` | `fa` | A | ui.c:101 ui_struct_new | — | — |  |
| `ui` | `gj` | E | ui.c:111 ui_struct_new | ui.c:230 ui_lj | — |  |
| `ui` | `hw` | A | ui.c:103 ui_struct_new | — | — |  |
| `ui` | `ic` | E | ui.c:105 ui_struct_new | ui.c:280 ui_gw | — |  |
| `ui` | `ic_own` | E | ui.c:106 ui_struct_new; ui.c:108 ui_struct_new (fuite) | ui.c:277 ui_gw (fuite); ui.c:278 ui_gw (fuite); ui.c:280 ui_gw (fuite) | — | stockage du registre ; fuites `pico8_ptrs_init(&ui->ic_own…)` suivies : `pico8_ui_gw` le vide et le remplit |
| `ui` | `le` | I | ui.c:102 ui_struct_new | msgbox.c:387 pz; msgbox.c:399 pz; ui.c:466 ui_cj | — | défaut `&g_shared_le` (liste statique, S), réécrit par `pico8_ui_cj` et `pico8_pz` |
| `ui` | `lheld` | E | ui.c:112 ui_struct_new | ui.c:362 ui_bh; ui.c:388 ui_bh | — |  |
| `ui` | `o_` | E | ui.c:110 ui_struct_new | ui.c:281 ui_gw; ui.c:300 ui_gw | T:hp.c:397 built_world; T:hp.c:399 built_world; T:hy.c:257 built_world; T:hy.c:259 built_world; T:jc.c:363 dragged; T:jc.c:412 hq_over; T:js.c:514 built_world; T:js.c:516 built_world; T:lq.c:673 built_planet_panel; T:lq.c:675 built_planet_panel; T:mz.c:248 world; T:mz.c:250 world; T:trade.c:174 join; T:trade.c:176 join |  |
| `ui` | `qa` | E | ui.c:104 ui_struct_new | ui.c:215 ui_lj | — |  |
| `ui` | `ql` | A | ui.c:100 ui_struct_new | — | — |  |
| `ui` | `rheld` | E | ui.c:113 ui_struct_new | ui.c:365 ui_bh; ui.c:390 ui_bh | — |  |
| `emitter` | `pn` | I | help.c:162 fv_struct_new; planet_family.c:293 hj_struct_init; planet_family.c:400 pm_struct_new; planet_family.c:572 ga_struct_new; planet_family_trade.c:378 mx_struct_new; probe.c:461 g__struct_init; probe.c:481 g__struct_new | — | — | défaut de classe dans 6 inits, réécrit par clé de construction (`pn`/`du`/`py` dans `pico8_g__struct_new`, `py`/`k_`/`qo`/`ro`/`qg`/`q_` par `keys->pq` de `pico8_ga_struct_new`), jamais après |
| `emitter` | `du` | I | help.c:163 fv_struct_new; planet_family.c:297 hj_struct_init; planet_family.c:402 pm_struct_new; planet_family.c:573 ga_struct_new; planet_family_trade.c:382 mx_struct_new; probe.c:469 g__struct_init; probe.c:484 g__struct_new | — | — | défaut de classe dans 6 inits, réécrit par clé de construction (`pn`/`du`/`py` dans `pico8_g__struct_new`, `py`/`k_`/`qo`/`ro`/`qg`/`q_` par `keys->pq` de `pico8_ga_struct_new`), jamais après |
| `emitter` | `py` | I | help.c:164 fv_struct_new; planet_family.c:291 hj_struct_init; planet_family.c:398 pm_struct_new; planet_family.c:583 ga_struct_new; planet_family.c:607 ga_struct_new; planet_family_trade.c:376 mx_struct_new; probe.c:459 g__struct_init; probe.c:487 g__struct_new | — | — | défaut de classe dans 6 inits, réécrit par clé de construction (`pn`/`du`/`py` dans `pico8_g__struct_new`, `py`/`k_`/`qo`/`ro`/`qg`/`q_` par `keys->pq` de `pico8_ga_struct_new`), jamais après |
| `emitter` | `ki` | A | help.c:166 fv_struct_new; planet_family.c:292 hj_struct_init; planet_family.c:399 pm_struct_new; planet_family.c:574 ga_struct_new; planet_family_trade.c:377 mx_struct_new; probe.c:460 g__struct_init | — | — |  |
| `emitter` | `pk` | A | help.c:165 fv_struct_new; planet_family.c:294 hj_struct_init; planet_family.c:401 pm_struct_new; planet_family.c:575 ga_struct_new; planet_family_trade.c:379 mx_struct_new; probe.c:462 g__struct_init | — | — |  |
| `emitter` | `fc` | A | help.c:156 fv_struct_new; planet_family.c:295 hj_struct_init; planet_family.c:403 pm_struct_new; planet_family.c:576 ga_struct_new; planet_family_trade.c:380 mx_struct_new; probe.c:463 g__struct_init | — | — |  |
| `emitter` | `k_` | I | help.c:157 fv_struct_new; planet_family.c:296 hj_struct_init; planet_family.c:404 pm_struct_new; planet_family.c:581 ga_struct_new; planet_family.c:605 ga_struct_new; planet_family_trade.c:381 mx_struct_new; probe.c:464 g__struct_init | — | — | défaut de classe dans 6 inits, réécrit par clé de construction (`pn`/`du`/`py` dans `pico8_g__struct_new`, `py`/`k_`/`qo`/`ro`/`qg`/`q_` par `keys->pq` de `pico8_ga_struct_new`), jamais après |
| `emitter` | `qo` | I | help.c:158 fv_struct_new; planet_family.c:298 hj_struct_init; planet_family.c:405 pm_struct_new; planet_family.c:577 ga_struct_new; planet_family.c:601 ga_struct_new; planet_family_trade.c:383 mx_struct_new; probe.c:465 g__struct_init | — | — | défaut de classe dans 6 inits, réécrit par clé de construction (`pn`/`du`/`py` dans `pico8_g__struct_new`, `py`/`k_`/`qo`/`ro`/`qg`/`q_` par `keys->pq` de `pico8_ga_struct_new`), jamais après |
| `emitter` | `ro` | I | help.c:159 fv_struct_new; planet_family.c:299 hj_struct_init; planet_family.c:406 pm_struct_new; planet_family.c:578 ga_struct_new; planet_family.c:602 ga_struct_new; planet_family_trade.c:384 mx_struct_new; probe.c:466 g__struct_init | — | — | défaut de classe dans 6 inits, réécrit par clé de construction (`pn`/`du`/`py` dans `pico8_g__struct_new`, `py`/`k_`/`qo`/`ro`/`qg`/`q_` par `keys->pq` de `pico8_ga_struct_new`), jamais après |
| `emitter` | `qg` | I | help.c:160 fv_struct_new; planet_family.c:300 hj_struct_init; planet_family.c:407 pm_struct_new; planet_family.c:579 ga_struct_new; planet_family.c:603 ga_struct_new; planet_family_trade.c:385 mx_struct_new; probe.c:467 g__struct_init | — | — | défaut de classe dans 6 inits, réécrit par clé de construction (`pn`/`du`/`py` dans `pico8_g__struct_new`, `py`/`k_`/`qo`/`ro`/`qg`/`q_` par `keys->pq` de `pico8_ga_struct_new`), jamais après |
| `emitter` | `q_` | I | help.c:161 fv_struct_new; planet_family.c:301 hj_struct_init; planet_family.c:408 pm_struct_new; planet_family.c:580 ga_struct_new; planet_family.c:604 ga_struct_new; planet_family_trade.c:386 mx_struct_new; probe.c:468 g__struct_init | — | — | défaut de classe dans 6 inits, réécrit par clé de construction (`pn`/`du`/`py` dans `pico8_g__struct_new`, `py`/`k_`/`qo`/`ro`/`qg`/`q_` par `keys->pq` de `pico8_ga_struct_new`), jamais après |
| `emitter` | `qy` | E | planet_family.c:302 hj_struct_init; planet_family.c:585 ga_struct_new; probe.c:443 g__struct_init | particle.c:50 f_ | — | NULL dans les 3 inits porteuses, alloué par `pico8_f_` après la construction |
| `probe` | `et` | E | planet_family.c:77 qh_struct_init; probe.c:447 g__struct_init; slipway.c:420 jc_struct_new; wormhole.c:261 mz_struct_new | probe.c:201 bv; slipway.c:146 jc_hq; wormhole.c:70 mz_hq | — | NULL à l'init, cible posée par `pico8_bv`, `pico8_jc_hq`, `pico8_mz_hq` |
| `probe` | `dw` | A | planet_family.c:74 qh_struct_init; planet_family_trade.c:200 nr_struct_init; probe.c:458 g__struct_init; slipway.c:409 jc_struct_new; wormhole.c:246 mz_struct_new | — | T:jc.c:853 test_bv | écrit seulement par les inits ; `test_pico8_jc.c:853` (`test_bv`) le met à 0 : 2.13 le réécrit sur un archétype de test à `dw` nul |
| `probe` | `od` | I | planet_family.c:69 qh_struct_init; planet_family.c:163 nl_struct_new; planet_family.c:219 os_struct_new; planet_family.c:306 hj_struct_init; planet_family_trade.c:197 nr_struct_init; planet_family_trade.c:271 fr_struct_new; planet_family_trade.c:390 mx_struct_new; probe.c:452 g__struct_init; slipway.c:416 jc_struct_new; wormhole.c:252 mz_struct_new | research_icon.c:183 ie_kj; slipway.c:139 jc_hq | — | défaut dans 8 inits, réécrit par `pico8_jc_hq` ; `pico8_ie_kj` n'écrit qu'une sonde locale `pb` |
| `probe` | `has_od` | I | planet_family.c:70 qh_struct_init; probe.c:453 g__struct_init; slipway.c:417 jc_struct_new; wormhole.c:253 mz_struct_new | research_icon.c:182 ie_kj; slipway.c:140 jc_hq | — | drapeau de `od` : 1 posé avec le défaut de `g_`, `qh`, `mz`, réécrit par `pico8_jc_hq` |
| `probe` | `dq` | A | planet_family.c:67 qh_struct_init; probe.c:451 g__struct_init; slipway.c:405 jc_struct_new; wormhole.c:247 mz_struct_new | — | — |  |
| `probe` | `fp` | A | planet_family.c:71 qh_struct_init; planet_family_trade.c:198 nr_struct_init; probe.c:454 g__struct_init; slipway.c:406 jc_struct_new; wormhole.c:254 mz_struct_new | — | — |  |
| `probe` | `ik` | A | planet_family.c:72 qh_struct_init; probe.c:455 g__struct_init; slipway.c:407 jc_struct_new; wormhole.c:249 mz_struct_new | — | — |  |
| `probe` | `has_ik` | A | planet_family.c:73 qh_struct_init; probe.c:456 g__struct_init; slipway.c:408 jc_struct_new; wormhole.c:250 mz_struct_new | — | — |  |

## Annexe B — valeurs résolues par classe

Correction 2.12e1 : les entrées `jo.kw_own` ci-dessous sont un E, hors archétype.

Pour chaque classe qui reçoit un archétype : champ (type déclarant.membre) = valeur résolue (`[I]` : défaut d'un champ I), et la ligne qui la pose aujourd'hui, dernière écriture de la chaîne. Les valeurs `pico8_X_start.m` sont lues dans `game_data.c`, et les clés de construction sont exclues.

- **`jo`** (18) : `jo.by` = `NULL` [I] (planet.c:169) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x60000` (planet.c:140) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `0` [I] (planet.c:171) ; `jo.hf` = `1` (planet.c:138) ; `jo.hs` = `0x10000` (planet.c:139) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{-0x70000, -0x70000, 0x70000, 0x70000}` (planet.c:141) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0` [I] (planet.c:170) ; `en-tête.me` = `jo_me_text` [I] (planet.c:175) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`qh`** (30) : `jo.by` = `NULL` [I] (planet.c:169) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x60000` (planet.c:140) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `1` [I] (planet_family.c:79) ; `jo.hf` = `0` (planet_family.c:80) ; `jo.hs` = `0x10000` (planet.c:139) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{-0x70000, -0x70000, 0x70000, 0x70000}` (planet.c:141) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0x10000` [I] (planet_family.c:78) ; `qh.ba` = `&qh_ba_seq` (planet_family.c:76) ; `qh.hp` = `0x10000` (planet_family.c:64) ; `qh.px` = `&qh_px_row` (planet_family.c:65) ; `qh.qh` = `1` (planet_family.c:68) ; `qh.s` = `0xae0000` [I] (planet_family.c:75) ; `en-tête.me` = `qh_me_text` [I] (planet_family.c:81) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `probe.dw` = `0x666` (planet_family.c:74) ; `probe.od` = `0xf0000` [I] (planet_family.c:69) ; `probe.has_od` = `1` [I] (planet_family.c:70) ; `probe.dq` = `qh_dq_text` (planet_family.c:67) ; `probe.fp` = `0x20000` (planet_family.c:71) ; `probe.ik` = `0` (planet_family.c:72) ; `probe.has_ik` = `0` (planet_family.c:73).
- **`nl`** (33) : `jo.by` = `NULL` [I] (planet.c:169) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x60000` (planet.c:140) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `1` [I] (planet_family.c:79) ; `jo.hf` = `0` (planet_family.c:80) ; `jo.hs` = `0x10000` (planet.c:139) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{-0x70000, -0x70000, 0x70000, 0x70000}` (planet.c:141) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0x10000` [I] (planet_family.c:78) ; `nl.nl` = `0x10000` (planet_family.c:157) ; `nl.qw` = `0xf0000` [I] (planet_family.c:158) ; `nl.rr` = `nl_rr_text` (planet_family.c:156) ; `qh.ba` = `&nl_ba_seq` (planet_family.c:164) ; `qh.hp` = `0x60000` (planet_family.c:160) ; `qh.px` = `&nl_px_row` (planet_family.c:162) ; `qh.qh` = `0` (planet_family.c:159) ; `qh.s` = `0x440000` [I] (planet_family.c:161) ; `en-tête.me` = `qh_me_text` [I] (planet_family.c:81) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `probe.dw` = `0x666` (planet_family.c:74) ; `probe.od` = `0x140000` [I] (planet_family.c:163) ; `probe.has_od` = `1` [I] (planet_family.c:70) ; `probe.dq` = `qh_dq_text` (planet_family.c:67) ; `probe.fp` = `0x20000` (planet_family.c:71) ; `probe.ik` = `0` (planet_family.c:72) ; `probe.has_ik` = `0` (planet_family.c:73).
- **`os`** (30) : `jo.by` = `NULL` [I] (planet.c:169) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x60000` (planet.c:140) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `1` [I] (planet_family.c:79) ; `jo.hf` = `0` (planet_family.c:80) ; `jo.hs` = `0x8000` (planet_family.c:220) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{-0x70000, -0x70000, 0x70000, 0x70000}` (planet.c:141) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0x10000` [I] (planet_family.c:78) ; `qh.ba` = `&qh_ba_seq` (planet_family.c:76) ; `qh.hp` = `0x40000` (planet_family.c:215) ; `qh.px` = `&os_px_row` (planet_family.c:216) ; `qh.qh` = `0` (planet_family.c:217) ; `qh.s` = `0x640000` [I] (planet_family.c:218) ; `en-tête.me` = `qh_me_text` [I] (planet_family.c:81) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `probe.dw` = `0x666` (planet_family.c:74) ; `probe.od` = `0xf0000` [I] (planet_family.c:219) ; `probe.has_od` = `1` [I] (planet_family.c:70) ; `probe.dq` = `qh_dq_text` (planet_family.c:67) ; `probe.fp` = `0x20000` (planet_family.c:71) ; `probe.ik` = `0` (planet_family.c:72) ; `probe.has_ik` = `0` (planet_family.c:73).
- **`hj`** (42) : `hj.rr` = `hj_rr_text` (planet_family.c:289) ; `jo.by` = `NULL` [I] (planet.c:169) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x60000` (planet.c:140) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `1` [I] (planet_family.c:79) ; `jo.hf` = `0` (planet_family.c:80) ; `jo.hs` = `0x10000` (planet.c:139) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{-0x70000, -0x70000, 0x70000, 0x70000}` (planet.c:141) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0x10000` [I] (planet_family.c:78) ; `qh.ba` = `&qh_ba_seq` (planet_family.c:76) ; `qh.hp` = `0x20000` (planet_family.c:303) ; `qh.px` = `&hj_px_row` (planet_family.c:305) ; `qh.qh` = `0` (planet_family.c:304) ; `qh.s` = `0xa60000` [I] (planet_family.c:307) ; `en-tête.me` = `qh_me_text` [I] (planet_family.c:81) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `emitter.pn` = `0x50000` [I] (planet_family.c:293) ; `emitter.du` = `0x78000` [I] (planet_family.c:297) ; `emitter.py` = `0x1eb8` [I] (planet_family.c:291) ; `emitter.ki` = `0x90000` (planet_family.c:292) ; `emitter.pk` = `0x10000` (planet_family.c:294) ; `emitter.fc` = `0x8000` (planet_family.c:295) ; `emitter.k_` = `&hj_k__seq` [I] (planet_family.c:296) ; `emitter.qo` = `0xfae0` [I] (planet_family.c:298) ; `emitter.ro` = `-0x11ec` [I] (planet_family.c:299) ; `emitter.qg` = `0x11ea` [I] (planet_family.c:300) ; `emitter.q_` = `0xfae0` [I] (planet_family.c:301) ; `probe.dw` = `0x666` (planet_family.c:74) ; `probe.od` = `0x3c0000` [I] (planet_family.c:306) ; `probe.has_od` = `1` [I] (planet_family.c:70) ; `probe.dq` = `qh_dq_text` (planet_family.c:67) ; `probe.fp` = `0x20000` (planet_family.c:71) ; `probe.ik` = `0` (planet_family.c:72) ; `probe.has_ik` = `0` (planet_family.c:73).
- **`pm`** (44) : `hj.rr` = `hj_rr_text` (planet_family.c:289) ; `jo.by` = `NULL` [I] (planet.c:169) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x60000` (planet.c:140) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `1` [I] (planet_family.c:79) ; `jo.hf` = `0` (planet_family.c:80) ; `jo.hs` = `0x10000` (planet.c:139) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{-0x70000, -0x70000, 0x70000, 0x70000}` (planet.c:141) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0x10000` [I] (planet_family.c:78) ; `pm.fz` = `pm_fz_text` (planet_family.c:397) ; `pm.ng` = `&{0x10000, 0x0}` (planet_family.c:396) ; `qh.ba` = `&qh_ba_seq` (planet_family.c:76) ; `qh.hp` = `0x30000` (planet_family.c:409) ; `qh.px` = `&pm_px_row` (planet_family.c:410) ; `qh.qh` = `0` (planet_family.c:304) ; `qh.s` = `0x620000` [I] (planet_family.c:411) ; `en-tête.me` = `pm_me_text` [I] (planet_family.c:412) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `emitter.pn` = `0x80000` [I] (planet_family.c:400) ; `emitter.du` = `0x20000` [I] (planet_family.c:402) ; `emitter.py` = `0xf5c` [I] (planet_family.c:398) ; `emitter.ki` = `0x90000` (planet_family.c:399) ; `emitter.pk` = `0` (planet_family.c:401) ; `emitter.fc` = `0x10000` (planet_family.c:403) ; `emitter.k_` = `&pm_k__seq` [I] (planet_family.c:404) ; `emitter.qo` = `0x1051e` [I] (planet_family.c:405) ; `emitter.ro` = `-0xd70` [I] (planet_family.c:406) ; `emitter.qg` = `0xd70` [I] (planet_family.c:407) ; `emitter.q_` = `0x1051e` [I] (planet_family.c:408) ; `probe.dw` = `0x666` (planet_family.c:74) ; `probe.od` = `0x3c0000` [I] (planet_family.c:306) ; `probe.has_od` = `1` [I] (planet_family.c:70) ; `probe.dq` = `qh_dq_text` (planet_family.c:67) ; `probe.fp` = `0x20000` (planet_family.c:71) ; `probe.ik` = `0` (planet_family.c:72) ; `probe.has_ik` = `0` (planet_family.c:73).
- **`mx`** (43) : `hj.rr` = `mx_rr_text` (planet_family_trade.c:375) ; `jo.by` = `NULL` [I] (planet.c:169) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x60000` (planet.c:140) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `1` [I] (planet_family.c:79) ; `jo.hf` = `0` (planet_family.c:80) ; `jo.hs` = `0x10000` (planet.c:139) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{-0x70000, -0x70000, 0x70000, 0x70000}` (planet.c:141) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0x10000` [I] (planet_family.c:78) ; `mx.ht` = `&mx_ht_seq` [I] (planet_family_trade.c:374) ; `qh.ba` = `&qh_ba_seq` (planet_family.c:76) ; `qh.hp` = `0x50000` (planet_family_trade.c:387) ; `qh.px` = `&mx_px_row` (planet_family_trade.c:389) ; `qh.qh` = `0` (planet_family.c:304) ; `qh.s` = `0x420000` [I] (planet_family_trade.c:388) ; `en-tête.me` = `qh_me_text` [I] (planet_family.c:81) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `emitter.pn` = `0x50000` [I] (planet_family_trade.c:378) ; `emitter.du` = `0x50000` [I] (planet_family_trade.c:382) ; `emitter.py` = `0x1eb8` [I] (planet_family_trade.c:376) ; `emitter.ki` = `0x90000` (planet_family_trade.c:377) ; `emitter.pk` = `0x10000` (planet_family_trade.c:379) ; `emitter.fc` = `0x4ccc` (planet_family_trade.c:380) ; `emitter.k_` = `&mx_k__seq` [I] (planet_family_trade.c:381) ; `emitter.qo` = `0xfae0` [I] (planet_family_trade.c:383) ; `emitter.ro` = `-0x11ec` [I] (planet_family_trade.c:384) ; `emitter.qg` = `0x11ea` [I] (planet_family_trade.c:385) ; `emitter.q_` = `0xfae0` [I] (planet_family_trade.c:386) ; `probe.dw` = `0x666` (planet_family.c:74) ; `probe.od` = `0x280000` [I] (planet_family_trade.c:390) ; `probe.has_od` = `1` [I] (planet_family.c:70) ; `probe.dq` = `qh_dq_text` (planet_family.c:67) ; `probe.fp` = `0x20000` (planet_family.c:71) ; `probe.ik` = `0` (planet_family.c:72) ; `probe.has_ik` = `0` (planet_family.c:73).
- **`ga`** (32) : `ga.fz` = `ga_fz_text` (planet_family.c:570) ; `ga.iu` = `-0xf5c` [I] (planet_family.c:582) ; `ga.ms` = `0x30000` [I] (planet_family.c:584) ; `jo.by` = `&g_ga_by_shared` [I] (planet_family.c:586) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x30000` (planet_family.c:589) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `0` [I] (planet.c:171) ; `jo.hf` = `0` (planet_family.c:587) ; `jo.hs` = `0x10000` (planet.c:139) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{0x0, 0x0, -0x10000, -0x10000}` (planet_family.c:588) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0` [I] (planet.c:170) ; `en-tête.me` = `jo_me_text` [I] (planet.c:175) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `emitter.pn` = `0` [I] (planet_family.c:572) ; `emitter.du` = `0` [I] (planet_family.c:573) ; `emitter.py` = `0x60000` [I] (planet_family.c:583) ; `emitter.ki` = `0` (planet_family.c:574) ; `emitter.pk` = `0` (planet_family.c:575) ; `emitter.fc` = `0x0` (planet_family.c:576) ; `emitter.k_` = `&ga_k__seq` [I] (planet_family.c:581) ; `emitter.qo` = `0xe666` [I] (planet_family.c:577) ; `emitter.ro` = `0x0` [I] (planet_family.c:578) ; `emitter.qg` = `0x0` [I] (planet_family.c:579) ; `emitter.q_` = `0xe666` [I] (planet_family.c:580).
- **`nr`** (32) : `jo.by` = `NULL` [I] (planet.c:169) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x60000` (planet.c:140) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `1` [I] (planet_family.c:79) ; `jo.hf` = `0` (planet_family.c:80) ; `jo.hs` = `0x10000` (planet.c:139) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{-0x40000, -0x40000, 0x50000, 0x50000}` (planet_family_trade.c:201) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0x10000` [I] (planet_family.c:78) ; `nr.hz` = `&nr_hz_seq` (planet_family_trade.c:195) ; `nr.rr` = `nr_rr_text` (planet_family_trade.c:196) ; `qh.ba` = `&qh_ba_seq` (planet_family.c:76) ; `qh.hp` = `0x10000` (planet_family.c:64) ; `qh.px` = `&qh_px_row` (planet_family.c:65) ; `qh.qh` = `1` (planet_family.c:68) ; `qh.s` = `0xaa0000` [I] (planet_family_trade.c:199) ; `en-tête.me` = `qh_me_text` [I] (planet_family.c:81) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `probe.dw` = `0x28e` (planet_family_trade.c:200) ; `probe.od` = `0xa0000` [I] (planet_family_trade.c:197) ; `probe.has_od` = `1` [I] (planet_family.c:70) ; `probe.dq` = `qh_dq_text` (planet_family.c:67) ; `probe.fp` = `0x10000` (planet_family_trade.c:198) ; `probe.ik` = `0` (planet_family.c:72) ; `probe.has_ik` = `0` (planet_family.c:73).
- **`fr`** (33) : `fr.mt` = `0x10000` (planet_family_trade.c:268) ; `jo.by` = `NULL` [I] (planet.c:169) ; `jo.ca` = `0x90000` (planet.c:148) ; `jo.cq` = `&jo_cq_seq` (planet.c:145) ; `jo.dz` = `jo_dz_rows` (planet.c:144) ; `jo.em` = `&jo_em_row` (planet.c:149) ; `jo.fa` = `0x60000` (planet.c:140) ; `jo.gq` = `&{0x0, 0xb0000}` (planet.c:147) ; `jo.has_mi` = `1` [I] (planet_family.c:79) ; `jo.hf` = `0` (planet_family.c:80) ; `jo.hs` = `0x10000` (planet.c:139) ; `jo.ka` = `&jo_ka_seq` (planet.c:143) ; `jo.kq` = `&{-0x40000, -0x40000, 0x50000, 0x50000}` (planet_family_trade.c:201) ; `jo.kw` = `&g_jo_kw_shared` [I] (planet.c:146) ; `jo.kw_own` = `&jo_kw_start` [I] (planet.c:174) ; `jo.ma` = `0x0` [I] (planet.c:142) ; `jo.mi` = `0x10000` [I] (planet_family.c:78) ; `nr.hz` = `&fr_hz_seq` (planet_family_trade.c:269) ; `nr.rr` = `fr_rr_text` (planet_family_trade.c:270) ; `qh.ba` = `&qh_ba_seq` (planet_family.c:76) ; `qh.hp` = `0x10000` (planet_family.c:64) ; `qh.px` = `&qh_px_row` (planet_family.c:65) ; `qh.qh` = `1` (planet_family.c:68) ; `qh.s` = `0x4e0000` [I] (planet_family_trade.c:272) ; `en-tête.me` = `qh_me_text` [I] (planet_family.c:81) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `probe.dw` = `0x28e` (planet_family_trade.c:200) ; `probe.od` = `0x190000` [I] (planet_family_trade.c:271) ; `probe.has_od` = `1` [I] (planet_family.c:70) ; `probe.dq` = `qh_dq_text` (planet_family.c:67) ; `probe.fp` = `0x10000` (planet_family_trade.c:198) ; `probe.ik` = `0` (planet_family.c:72) ; `probe.has_ik` = `0` (planet_family.c:73).
- **`g_`** (23) : `g_.fa` = `0x20000` (probe.c:457) ; `g_.fx` = `0x10000` (probe.c:450) ; `g_.ra` = `NULL` [I] (probe.c:444) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `emitter.pn` = `0x30000` [I] (probe.c:461) ; `emitter.du` = `0x20000` [I] (probe.c:469) ; `emitter.py` = `0xf5c` [I] (probe.c:459) ; `emitter.ki` = `0x90000` (probe.c:460) ; `emitter.pk` = `0x10000` (probe.c:462) ; `emitter.fc` = `0x10000` (probe.c:463) ; `emitter.k_` = `&g__k__seq` [I] (probe.c:464) ; `emitter.qo` = `0x1051e` [I] (probe.c:465) ; `emitter.ro` = `-0xd70` [I] (probe.c:466) ; `emitter.qg` = `0xd70` [I] (probe.c:467) ; `emitter.q_` = `0x1051e` [I] (probe.c:468) ; `probe.dw` = `0x666` (probe.c:458) ; `probe.od` = `0x0` [I] (probe.c:452) ; `probe.has_od` = `1` [I] (probe.c:453) ; `probe.dq` = `"ly"` (probe.c:451) ; `probe.fp` = `0` (probe.c:454) ; `probe.ik` = `0` (probe.c:455) ; `probe.has_ik` = `0` (probe.c:456).
- **`fv`** (32) : `fv.ca` = `0xb0000` (help.c:168) ; `fv.eo` = `-0x10000` (help.c:170) ; `fv.ff` = `0x10000` (help.c:171) ; `fv.fg` = `0x50000` (help.c:174) ; `fv.gq` = `&{0x0, 0xb0000}` (help.c:169) ; `fv.hi` = `&{0x220000, 0x380000}` (help.c:167) ; `fv.kq` = `&{-0x280000, 0x4e0000, 0x280000, 0x620000}` (help.c:172) ; `fv.ql` = `0xb0000` (help.c:173) ; `fv.rb` = `&fv_rb_row` (help.c:175) ; `g_.fa` = `0xa0000` (help.c:154) ; `g_.fx` = `0x10000` (probe.c:450) ; `g_.ra` = `&{0x400000, 0x1c0000}` [I] (help.c:155) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `emitter.pn` = `0x50000` [I] (help.c:162) ; `emitter.du` = `0xf0000` [I] (help.c:163) ; `emitter.py` = `0x1b6` [I] (help.c:164) ; `emitter.ki` = `0x110000` (help.c:166) ; `emitter.pk` = `0x10000` (help.c:165) ; `emitter.fc` = `0x50000` (help.c:156) ; `emitter.k_` = `&fv_k__seq` [I] (help.c:157) ; `emitter.qo` = `0x1051e` [I] (help.c:158) ; `emitter.ro` = `-0x1ca` [I] (help.c:159) ; `emitter.qg` = `0x1ca` [I] (help.c:160) ; `emitter.q_` = `0x1051e` [I] (help.c:161) ; `probe.dw` = `0x666` (probe.c:458) ; `probe.od` = `0x0` [I] (probe.c:452) ; `probe.has_od` = `1` [I] (probe.c:453) ; `probe.dq` = `"ly"` (probe.c:451) ; `probe.fp` = `0` (probe.c:454) ; `probe.ik` = `0` (probe.c:455) ; `probe.has_ik` = `0` (probe.c:456).
- **`bg`** (5) : `bg.fa` = `0x10000` (bg.c:70) ; `bg.hd` = `0x10000` (bg.c:71) ; `bg.pl` = `&bg_pl_seq` (bg.c:72) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`ui`** (6) : `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `ui.fa` = `0xf0000` (ui.c:101) ; `ui.hw` = `&{0xa0000, 0xa0000, 0x760000, 0x760000}` (ui.c:103) ; `ui.le` = `&g_shared_le` [I] (ui.c:102) ; `ui.ql` = `0x3a0000` (ui.c:100).
- **`js`** (3) : `js.fa` = `0xb0000` [I] (ui_button.c:135) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`fq`** (4) : `fq.fa` = `0xe0000` (ui_button.c:356) ; `fq.hd` = `0x10000` (ui_button.c:357) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`qb`** (7) : `qb.ma` = `0x0` [I] (scroll.c:49) ; `qb.p` = `&{0xc00000, 0xc00000}` [I] (scroll.c:50) ; `qb.pc` = `&qb_pc_vecs` (scroll.c:52) ; `qb.ra` = `&{0xc00000, 0xc00000}` [I] (scroll.c:51) ; `qb.v` = `&{0x0, 0x0}` [I] (scroll.c:48) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`nf`** (3) : `nf.ra` = `&g_nf_ra` [I] (mouse_reader.c:47) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`mh`** (4) : `mh.fa` = `0xe0000` (msgbox.c:111) ; `mh.rh` = `0x50000` [I] (msgbox.c:112) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`iv`** (5) : `iv.fa` = `0x90000` (msgbox.c:258) ; `iv.l` = `0x0` [I] (msgbox.c:259) ; `iv.v` = `&{0x0, -0x570a}` (msgbox.c:260) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`mz`** (14, dont 1 S) : `mz.fa` = `0x50000` (wormhole.c:255) ; `mz.jj` = `&g_mz_jj_wrapper` [S, hors archétype] (wormhole.c:257) ; `mz.kk` = `&mz_kk_seq` (wormhole.c:256) ; `mz.oi` = `0x0` [I] (wormhole.c:251) ; `mz.spawn_snd` = `0x340000` (wormhole.c:248) ; `en-tête.me` = `"kz"` [I] (wormhole.c:244) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `probe.dw` = `0x0` (wormhole.c:246) ; `probe.od` = `0x30000` [I] (wormhole.c:252) ; `probe.has_od` = `1` [I] (wormhole.c:253) ; `probe.dq` = `"pr"` (wormhole.c:247) ; `probe.fp` = `0x10000` (wormhole.c:254) ; `probe.ik` = `0x310000` (wormhole.c:249) ; `probe.has_ik` = `1` (wormhole.c:250).
- **`jc`** (12) : `jc.fa` = `0x20000` [I] (slipway.c:403) ; `jc.k_` = `&jc_k__seq` (slipway.c:410) ; `jc.nh` = `0x140000` [I] (slipway.c:411) ; `en-tête.me` = `"kz"` [I] (slipway.c:402) ; `en-tête.t` = `0x0` [I] (ke.c:37) ; `probe.dw` = `0x106` (slipway.c:409) ; `probe.od` = `0` [I] (slipway.c:416) ; `probe.has_od` = `0` [I] (slipway.c:417) ; `probe.dq` = `NULL` (slipway.c:405) ; `probe.fp` = `0` (slipway.c:406) ; `probe.ik` = `0x300000` (slipway.c:407) ; `probe.has_ik` = `1` (slipway.c:408).
- **`hp`** (1) : `hp.fk` = `&hp_fk_row` (research.c:159).
- **`ie`** (6) : `ie.eh` = `&{0x760000, 0x780000}` (research_icon.c:244) ; `ie.fa` = `0xc0000` (research_icon.c:242) ; `ie.kq` = `&{0x0, 0x0, 0x90000, 0x80000}` (research_icon.c:241) ; `ie.ra` = `&{0x3e70000, 0x3e70000}` [I] (research_icon.c:243) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`pj`** (5) : `pj.fa` = `0xa0000` [I] (score_panel.c:294) ; `pj.gs` = `&pj_gs_row` (score_panel.c:295) ; `pj.i_` = `&pj_i__row` (score_panel.c:296) ; `en-tête.me` = `"ph"` [I] (ke.c:36) ; `en-tête.t` = `0x0` [I] (ke.c:37).
- **`hy`** (4) : `hy.nz` = `0x640000` [I] (economy.c:43) ; `hy.ew` = `0x10000` [I] (economy.c:46) ; `hy.lo` = `0x10000` [I] (economy.c:51) ; `hy.yr` = `0xd490000` [I] (economy.c:52).

## Annexe C — tableau « Runs »

Sur la copie jetable de `522824b`, sauf les lignes 1 et 2 (arbre de travail, lecture seule).

| # | Commande | Code | Ligne clé | Durée |
|---|---|---|---|---|
| 1 | `sh tools/composition_residue.sh` | 0 | `résidu 2 : 1073 ligne(s), 85 fichier(s)` ; six catégories non nulles | 0,7 s |
| 2 | `composition_residue.sh --champs` | 0 | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27` | 0,3 s |
| 3 | `git clone --no-hardlinks` local vers `etude29/tree` | 0 | HEAD `522824b` | < 2 s |
| 4 | `constproof.py` du brouillon du PRD (`prd29/`) | 1 | `FileNotFoundError: 'list.tsv'` : chemin relatif après `chdir`, le piège que l'annexe A du PRD corrige ; relancé avec le texte de l'annexe | < 1 s |
| 5 | `timeout 900 python3 constproof.py list.tsv` (302 champs) | 0 | `secs 221.8 fields 302` ; `missing.tsv` vide ; 919 lignes dont 4 `0 erreur` | 222 s |
| 6 | comparaison (type, champ, message) avec `prd29/writes.*.tsv` (`51bb145`) | 0 | aucune différence | < 1 s |
| 7 | `classify.py` (tri automatique) | 0 | `A? 71, E? 124, K? 65, I? 38, Z 4` ; 915 sites | < 1 s |
| 8 | lecture et reclassement, `annex.py` | 0 | A 70, I 38, K 87, E 107 ; 302 lignes, 915 sites | < 1 s |
| 9 | `comm -3` des listes (type, champ) | 0 | vide | < 1 s |
| 10 | `grep memset\|memcpy\|calloc` ; grep des types dans les cinq fichiers exclus | 0 / 1 | aucune entité touchée ; aucun type nommé | < 1 s |
| 11 | `git -C tree status --short` après le banc | 0 | vide | < 1 s |
| 12 | `chains.py` (valeurs résolues par classe, masquages) | 0 | 25 classes, 467 lignes (259 A, 208 I) ; 103 champs à écritures successives, dont 102 masquages entre niveaux | < 1 s |
| 13 | essai d'inclusion sur la copie (types et `pico8_nl_def`/`pico8_bg_def` dans `game_data.h/.c`, `def` dans `jo`/`bg`) puis `-fsyntax-only` | 0 | 0 erreur | 2 s |
| 14 | `composition_residue.sh` et `--check inclusion-mutuelle` sur la copie modifiée | 0 | `inclusion-mutuelle 17 16` ; 43 lignes ; `défauts 670` (+1 : `pico8_jo_kw_start`) ; pied 1074 / 85 ; `--champs` 284 | 1 s |
| 15 | `git -C tree checkout -- .` puis `status --short` | 0 | vide | < 1 s |
| 16 | seconde passe, texte exact du §2 (`kw_own` par valeur) : `-fsyntax-only` puis compteur | 0 | 0 erreur ; `inclusion-mutuelle 17 16`, 43 lignes ; `défauts 669` ; pied `1073 / 85` | 3 s |
| 17 | `git -C tree checkout -- .` puis `status --short` | 0 | vide | < 1 s |
| 18 | `fam.py` : attribution des 669 lignes de `--check défauts` par nom `pico8_X_start` | 0 | a1 66, a2 67, b 29, c 104, d 179, e1 127, e2 97 | < 1 s |
| 19 | `chains.py` relancé (`hp` sans `ke`), tables Q7 et masquages | 0 | 467 lignes ; 102 masquages entre niveaux | < 1 s |
| 20 | `timeout 600 make test` (arbre de travail, aucun code changé) | 0 | `garde fichiers 58 en-echec 0` | 5 s |
| 21 | `make composition-residue` (arbre de travail, fin de 2.9c) | 0 | `résidu 2 : 1073 ligne(s), 85 fichier(s)` | 0,7 s |
| 22 | `rm -rf etude29/tree` | 0 | copie supprimée ; journaux et scripts gardés dans `etude29/` | < 1 s |
| 23 | copie jetable de `d9a2b2d` ; 3e passe d'inclusion avec `.jj = &g_mz_jj_wrapper` dans `pico8_mz_def` | 1 | `game_data.c:684: 'g_mz_jj_wrapper' undeclared` (confirme le vérificateur) | 3 s |
| 24 | 3e passe, `pico8_hy_def_t` dans `game_data.h` | 1 | `economy.h:34: unknown type name 'pico8_hy_def_t'` (`economy.h` n'inclut pas `game_data.h`) | 3 s |
| 25 | 3e passe retenue : `nl`, `bg`, `mz` (sans `jj`) dans `game_data.h/.c`, `hy` dans `economy.h/.c` | 0 | 0 erreur ; `inclusion-mutuelle 17 16`, 43 lignes ; `défauts 669` ; pied `1073 / 85` | 3 s |
| 26 | grep des types d'entité dans `pico8_raylib_backend_coords.c` et son TU | 1 | aucun | < 1 s |
| 27 | recomptage : A 69, I 42, K 87, E 103, S 1 ; comparaisons 2.12 = 51 / 25 / 55 / 103 / 123 / 91 ; `rm -rf etude29/tree` | 0 | copie supprimée | < 1 s |
