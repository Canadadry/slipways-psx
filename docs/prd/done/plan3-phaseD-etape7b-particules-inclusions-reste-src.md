---
title: "Plan 3, étape D.7b : les enregistrements de particules de `qy` passent d'une table ob à une struct typée, lue et mutée en place par `ei` et `ga:kj` ; les inclusions mortes de `pico8_ob.h` quittent les entités et les en-têtes ; le test de `nf.l` laissé par D.7a lit le bouton typé — trois commits, aucun pixel changé, reste `ob` de `src/` hors des deux modules = R, liste close de 22 lignes"
description: "Après D.7a, il ne reste que 44 lignes `ob` dans les fichiers de `src/` de la ligne D.7b (16 fichiers) : les particules de `qy` (produites par `g_:mm` et `ga:mm`, lues par `ei` et `ga:kj`, qui réalloue un vecteur par particule et par frame), des inclusions de `pico8_ob.h` devenues mortes, un `assert` d'ordre d'init qui interroge le registre de `ob()`, et neuf lignes de commentaire qui décrivent la couche ob encore vivante jusqu'à D.8. D.7b type la particule, retire les inclusions et l'`assert`, admet les neuf commentaires dans le reste R, et répare un TU qui écrit par une table ob dans le bouton `nf.l` que D.7a a typé — comptes en lignes ET occurrences, classification (a)/(b)/(c), fuite par frame en baisse."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données ») :

> - D.7b — `ob`, reste de `src/` hors `pico8_ob.*` et `pico8_stash.*` : entités et interface (`planet*`, `probe`, `particle`, `slipway`, `wormhole`, `msgbox`, `mouse_reader` avec `g_nf_l`/`g_nf_r`, `ui*`, `help`, `scroll`, `trade`, `bg`, `ke`, `entity`, `bootstrap`, `hud`, `research_icon`) et en-têtes (`pico8_struct.h`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_value.h`, `pico8_list.h`), même règle. Empreinte 42 (42, 0) aujourd'hui. Compte : `ob` ≤ 827, et 0 dans `src/` hors ces deux modules. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur.

Lignes voisines qui partagent le compte `ob`, recopiées :

> - D.7c — `ob`, TU restants : fixtures construites en `pico8_ob_table_*` → structs, état construit directement. Empreinte 41 (0, 41) aujourd'hui. Compte : `ob` = 187 (`pico8_ob.c` 82, `pico8_ob.h` 32, `test_pico8_ob.c` 55, `pico8_stash.c` 8, `pico8_stash.h` 3, `test_pico8_stash.c` 7). §6 : aucun. `sanitize-net` et `class-coverage-net` : non, aucun fichier de `src/` ne change.
>
> - D.8 — suppression : `pico8_ob.c/.h` (avec `pico8_fi`), `pico8_stash.c/.h`, `test_pico8_ob.c` et `test_pico8_stash.c` (§6, tests de la couche), le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, leurs entrées du `Makefile`. Compte : `ob` = 0, `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : non.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » versionnées à `be2786c`, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b, D.5c, D.6a, D.6b, D.6c, D.7a ; D.1 n'en a pas) :

> Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.

> Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.

> Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).

> Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).

> Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp` : `lq_own`/`lq_inner`) ; `hp.mr`/`gp` en `pico8_str_seq_t`, `ho_apply` les prend typés ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages `po_own`/`fu_own` dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie ; une clé `qe` chaîne est absente) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus, `pico8_jc_hg_add`/`_has`) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`, qui partage son calcul avec `pico8_gi`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : 1882 (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de −100 (1882 − 1982).

> Découvert à D.5a : deux chunks (interfaces, puis `ke` ; ≈ 40 chemins, `pico8_game_data.*`, `pico8_bootstrap.c` et les TU qui plient l'identité de `me` hors du majorant). Défauts générés `pico8_X_start` (type `pico8_X_start_t`, `pico8_game_data.h`), un par classe à clés (`ke`, `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`), clés propres de la spécification, lus par les constructeurs ; les tables gardent l'objet de D.4a/D.4b ; `mz.jj` s'initialise de `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` : liste statique de `ui.c` (`pico8_ui_le_shared`) ; `qb.v`/`p`/`ra`, `iv.v`, `ui.hw` pointent la donnée générée (membres `const`) ; `nf.ra` pointe le statique non `const` `g_nf_ra`, affecté depuis `pico8_nf_start.ra` à `pico8_nf_init` (repli : `test_pico8_entity_kind` et `test_pico8_ie` écrivent `je.ra` en place) ; `nf.l`/`nf.r` restent `g_nf_l`/`g_nf_r` (D.7b). Le cycle `_init`/`_shutdown`/`_ready` des treize modules reste, sur un drapeau. `pico8_ke_start_field` (compté par `par-nom`) remplace `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_g__default`, `pico8_ie_default` (retirés à D.5b) et `pico8_jo_default` (D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`) : D.5c le supprime. `pico8_rk_default` (variante de dessin, comptée par `spec`) est à D.5b. `spec` après D.5a : 634 (au lieu de ≤ 779) ; les majorants `spec` suivants se décalent de −145 (634 − 779). `par-nom` : 1090.

> Découvert à D.5b : deux chunks (exclusion de `rk`, puis les sept classes ; 29 chemins, `pico8_game_data.*` et trois TU hors du majorant). `pico8_rk_default` (variante de dessin à argument omis) entre dans `SPEC_EXCLUSION`, preuve : ses quatre lignes ne portent aucune autre catégorie. Défauts générés `pico8_X_start` de `g_`, `fv`, `hp`, `ie`, `pj`, lus par les constructeurs ; `hy`, `trade` sans objet. `ie` : un seul défaut pour la chaîne et les objets prêtés, `kq`/`ra`/`eh` pointent la donnée générée, plus libérés à l'arrêt. `fv.hi`/`gq`/`kq`, `g_.ra`, `ie.kq`/`ra`/`eh`, `pj.cr.hi`/`gq` : membres `const` vers la donnée générée, aucun repli. `pj.cr` : `g_pj_cr` reste (son `ep` est muté), valeurs de `pico8_pj_cr_start`. `pj.ou`/`pj.ea` : rangées générées `pico8_pj_ou_row`/`pico8_pj_ea_row`, d'où `pico8_score_panel_init` construit une fois par init les tables d'état `g_pj_ou`/`g_pj_ea` (`pico8_jb_row_view`), jamais libérées comme celles de la spécification ; leurs lecteurs (`hb`, `hk`, `kj`, `pico8_mh_new`) restent des tables ob (D.6b, D.7a). `test_pico8_spec_defaults` n'a plus de registre ni de constante. `pico8_ke_start_field` n'a plus que `pico8_jo_default` et le cas `M_KE`. `spec` après D.5b : 430 (au lieu de ≤ 402 après le décalage de D.5a : 28 lignes des sites de D.5b, retirées par D.4a/D.4b, étaient comptées à la fois dans le majorant 547 et dans ce décalage ; majorant corrigé 430) ; le « = 0 » de D.5c est inchangé. `par-nom` : 1085.

> Découvert à D.5c : deux chunks (données, puis conversion et garde ; 27 chemins, `pico8_game_data.*`, `ke.c/.h`, le test ponctuel et le `Makefile` hors du majorant ; le lien du test ponctuel prend `pico8_planet_family_trade.c`, pour interroger `nr`/`fr`/`mx`). Défauts générés `pico8_X_start` des dix classes de l'arbre, clés propres, lus par les constructeurs, l'héritage par le `struct_init` de la parente ; chaînes dans des tableaux distincts, un par jeton (des littéraux se fusionneraient, TROU-3). `jo.kq`/`gq`, `pm.ng` : membres `const` vers la donnée générée, aucun repli. Le `field` des sortes est `pico8_X_kind_field` : `rr`, `s`, `me`, `od` seulement, lus dans `pico8_X_start`, puis la parente jusqu'à `pico8_jo_kind_field` (seul déclaré, `planet.h`) ; 0 si le module n'est pas prêt ; compté par `par-nom` (1106 après D.5c), à typer par D.9. `pico8_ke_start_field` supprimé. `_ready` sur `g_jo_up`, `g_family_up`, `g_family_trade_up`. `ga.by` : `g_ga_by_shared`, `pico8_list_t` statique non `const` vidée à chaque init (précédent `ui.le`), contenu à typer par D.7a. `spec` après D.5c : 0, dans `DATA_RESIDUE_GUARD`. `ob` : 1721 hors test ponctuel.

> Découvert à D.6a : un commit (30 chemins : `research_icon.c` pour `ie:ldown`, `pico8_game_data.*` et quatorze TU qui construisent dans une variable `b`, hors du majorant). Clés typées : `pico8_jo_keys_t` (`id`, `p`, `ra`, NULL = absent, `pico8_struct.h`) pour neuf classes et `new_entity` ; `pico8_ga_keys_t` (`jo` et `pq`, NULL = défauts de `ga` ; `k_` NULL sous `pq` = nil) ; la chaîne que `pm:pq` parsait devient `pico8_pm_ga_pq_keys` (`pico8_ga_pq_keys_t`, générée) ; la sorte `ga` passe par une adaptatrice (`pico8_ga_kind_new`, `pq` NULL). `pico8_X_new(NULL)` inchangé ; les classes « seulement `id` » (`qh`, `nl`, `os`, `hj`, `fr`, `mx`) l'assertent sur `p`/`ra`. Tests de mort d'une clé positionnelle, sans membre ou hors rangée à la construction retirés (le type les interdit). `pico8_ob.h` quitte `planet.h`, `planet_family.h`, `planet_family_trade.h` ; `pico8_struct.h` le garde (`slipway.h`, `wormhole.h` en dépendent par lui). Le commentaire l. 2 de `gh_row_lq` parle des clés de `js` : il passe à D.6b (le changer ici serait un renommage à type constant). `overrides` après D.6a : 329 lignes / 348 occurrences / 55 fichiers (au lieu de 316 : le « = 315 » de D.1 était le total des fichiers de l'empreinte, dont 30 lignes de D.6b/D.6c — `iv`/`jc` de `planet.c` et `planet_family_trade.c`, `jc` de `test_pico8_mx`, `gh_row_lq`, `ie` de `test_pico8_entity_kind` — et sans 16 lignes de D.6a — `ie:ldown`, `nr` de `test_pico8_mz`/`test_pico8_bg` ; la ligne de `k_` ajoutée par D.4a est dans le périmètre) ; le compte de D.6b est à reprendre : sur les 329 lignes, 232 sont à D.6b (lignes classées par le constructeur appelé ou déclaré, commentaires de `lq`/`jb`/`gh_row_lq` compris, `iv` d'`economy.c` et `jc` des TU `hp`/`hy`/`trade`/`ui`/`mx`/`mz` compris) et 97 à D.6c : « `overrides` = 97 » après D.6b au lieu de 117 ; le « = 0 » de D.6c est inchangé. `par-nom` : 1075.

> Découvert à D.6b : trois chunks (classes sans clé ; `iv`/`jc`/`fq` ; `lq`/`jb`/`js`/`mh` avec les ponts, `dr` et le §6 — 58 chemins : appelants à NULL, `economy.c`, `score_panel_display`, `research_display`, `data_init` et les TU qui construisent dans `b` hors du majorant). `bg`, `nf`, `qb`, `ui`, `mz` sans paramètre ; clés typées `pico8_iv_keys_t`, `pico8_jc_keys_t`, `pico8_fq_keys_t`, `pico8_lq_keys_t` (items `pico8_value_t`), `pico8_js_keys_t` (`lq`/`h_`/`jb` restent des sources de rangée `pico8_value_t`, présence TROU-23), `pico8_mh_keys_t` ; `jb` prend `const pico8_jb_row_t *`. `js:pq` copie une rangée par `pico8_lq_keys_of` et une liste par ses éléments (plus de vue ni de conversion) ; `jb:pq` n'emballe plus ses lignes chaîne ; `fq(de)` lit `pico8_de_my()`. Pont `pico8_dr()` fermé (`pico8_jb_new(&pico8_dr_table[i])`). Ponts nouveaux, paramètre `overrides` gardé (type inchangé, comptés, 21 lignes) : `pico8_lq_new_table`, `pico8_jb_new_table` (`layout.c`), `pico8_js_new_table` (`ui_button.c`), `pico8_mh_new_table` (`msgbox.c`) — fermés quand les rangées de `pz` et les sources `lq`/`h_`/`jb`/`mh` sont typées (`planet_display`, `menu`, `score_panel_display`, `research_display` : D.7a ; `help`, `msgbox` : D.7b ; le « 0 dans `layout` » de D.7a exige les deux premiers). Les TU d'un pont gardent leur table : `test_pico8_mh` (clé `lh` nil, clés refusées : 7 lignes `overrides`, table de même type et même nom) et `test_pico8_lq` (deux commentaires qui nomment la table passée au pont) tombent avec lui. Reportés, avec eux : `pico8_ix()`/`pico8_ha()`, `g_bo`, `pico8_bz()` (vues par init ; `ix` porte un item imbriqué que seule une vue sait donner à `lq:mn`), les vues à l'appel de `hp.lq` et de `kw`. `pico8_lq_table_view` sans appelant de `src/` ; une table qui porte `fs` ou `w` est refusée par le pont `lq` (aucun producteur n'en pose). `overrides` après D.6b : 127 lignes / 138 occurrences / 27 fichiers (97 lignes de D.6c + 21 des ponts + 9 des TU des ponts ; le « = 117 » de D.1 était un total de fichiers, qui comptait dans ceux de D.6c 23 lignes de D.6b — `iv` de `economy.c`, `jc` de `test_pico8_hp`/`_hy`/`_trade`) ; le « = 0 » de D.6c devient « = 30 » (les ponts et leurs TU), et `overrides` entre dans `DATA_RESIDUE_GUARD` à l'étape qui ferme le dernier pont. D.7a hérite d'un chantier plus lourd : fermer les ponts `lq`/`jb` demande de typer les producteurs de rangées de `planet_display`, `menu`, `score_panel_display`, `research_display` et les rangées `bs` de `pz` (fichier de D.7b) ; son PRD tranche l'ordre ou la fusion avec D.7b. `par-nom` : 1062.

> Découvert à D.6c : deux chunks (`hy`/`fv`/`pj`/`g_`/`ie`/`trade`, puis `hp` et le pont `fd` — 37 chemins : appelants à NULL, TU qui construisent dans `b`/`o`, `data_init`, `collect` et le test ponctuel hors du majorant). `hy`, `fv`, `pj` sans paramètre ; clés typées `pico8_g__keys_t`, `pico8_ie_keys_t`, `pico8_trade_keys_t` (triple `qe` du membre ; TROU-24 au site de `trade:kg`), `pico8_hp_keys_t` (`ri` : enregistrement `pico8_fd_entry_t`, `rr`) ; `hp` applique `ho` aux champs typés (`pico8_ho_gp_has`, extraite de `collect.c`). Pont `pico8_fd()` fermé (`pico8_research_build_gb(void)` parcourt `pico8_fd_table`) ; `pico8_ho_apply` sans appelant de `src/` (D.7a `collect`, ou D.8 avec le test ponctuel). `overrides` après D.6c : 30 lignes / 34 occurrences / 8 fichiers (les 21 lignes des quatre ponts de D.6b et les 9 de leurs TU, le « = 30 » de la note de D.6b ; le « = 0 » de D.1 comptait sans eux) ; `overrides` n'entre pas dans `DATA_RESIDUE_GUARD` : il y entre à l'étape qui ferme le dernier pont — `js`, dont les rangées `bs` de `pz` sont à D.7b ; D.7a ferme `lq`, `jb`, `mh` et les autres sources de `js` si son PRD ne réordonne pas. La table `{qe=r.e}` de `jo:cn` (`planet_trade.c`, `jy`) reste une `pico8_ob_table_new` : ce n'est pas une clé de construction (D.7a). `par-nom` : 1038.

> Découvert à D.7a : six chunks (familles isolées ; `gb` ; enregistrements `{qe,gc}` ; articles de `bl` et panneau de score ; pipeline d'affichage et chaînon `pz` ; `nb` — 74 chemins : suiveurs forcés, TU qui lisent `gb` ou les ponts, test ponctuel hors du majorant), **commis dans l'ordre 1, 2, 4, 3, 5, 6** : les enregistrements `{qe,gc}` de `jo.jy` (chunk 3) et les sous-rangées de `bz` (chunk 4) sont des tuples de `lq:mn`, que `layout.c` lisait comme tables ob ; typer l'une ou l'autre famille avant ce lecteur demandait une conversion typée → ob ou un `lq:mn` à double lecture, tous deux interdits. Le chunk 4 a donc pris tous les tuples de `lq:mn` (et les articles de `bl`, qui partageaient la même vue de `data_init`), le 3 a suivi, le 5 est resté le chaînon `pz` sans les tuples ; décision du pilote. **Ordre D.7a/D.7b : D.7a a pris à D.7b le chaînon `pz`** (`pico8_pz`, `pico8_mh_new_table`, `g_bo` et `fv.rb` de `help.c`, `pico8_js_new_table` et les branches `OB_TABLE` de `js:pq`), **`g_nf_l`/`g_nf_r`** (avec `bu`) et les lignes qui lisaient ses types convertis dans `planet.c`, `planet_family_trade.c`, `ui.c` ; D.7b ne les a plus. Formes : rangées, tuples de `lq:mn` et articles de `bl` en `pico8_jb_row_t` (sorte REF opaque pour `ld` ; `{qe=x}` gardé comme `has_qe`/`qe` de la rangée et lu `{x,5}`) ; `pz(p, bs, n)` sur `pico8_js_keys_t` ; sources `lq`/`h_`/`jb` typées (`lq_row`/`h_row` : `pico8_lq_keys_t`, `pico8_lq_keys_of_row` ; `jb_row`) ; `{qe,gc}` en `pico8_jy_t` ; `gb` tableau fixe (`pico8_gb_at`, `pico8_gb_find`) ; `nb.e_` (`pico8_nb_e_`, remis par `pico8_data_init`), `pj.ou`, `pj.ea`, `bz` (`pico8_pj_bz`), `nf.l`/`nf.r` statiques non `const` ; `pico8_de()` supprimé (`pico8_de_my`). Vues fermées : `ix`, `ha`, `bo`, `bz`, `nb`, `pico8_jb_row_view`, `jo_kw_view`, `hp:jb`, `pico8_lq_table_view` ; `pico8_data_init(void)`. `overrides` = 0, dans `DATA_RESIDUE_GUARD` depuis le chunk 5. `ob` dans les fichiers de la ligne : 13 lignes (enregistrements de constructeurs de `ob()` `'v'`/`'s'` dans `pi.c` et leur commentaire de `pi.h`, `'b'` et `pico8_ho_apply` dans `collect.c`/`.h`), servant le test ponctuel, supprimées par D.8 ; le « = 187 » de D.7c devient 187 + 13. `ob` après D.7a : 552 lignes / 613 occurrences, dont 199 dans le test ponctuel (hors test ponctuel : 353) ; majorant de D.7b : ≤ 309 hors test ponctuel (353 moins les 44 lignes `ob` de ses fichiers de `src/`). Les greps de sortie `pico8_fi\(` et `pico8_ob_table_t \*g_` ne rendent plus que `pico8_ob.c/.h` et `pico8_stash.c`, laissés à D.8. Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : 142406 / 110943 → 19213 / 59310 octets (3480 / 2423 → 1099 / 1481 allocations). `par-nom` : 1009. Correctifs aux messages de commit de D.7a (comptes remesurés sur chaque arbre) : (1) classification des lignes retirées, (a) construction supprimée, (b) signature, contrat ou variable dont le type passe d'une table ob à un type C, (c) même type, autre nom : 5e58f7c, `ob` 340 lignes = (a) 317 + (b) 23 (les contrats de `pico8_pz` dans `msgbox.h` et de `pico8_hp_jb` dans `research_display.c/.h` ; les variables retypées `b` de `pz_step`, `row` de `jo:fe`, `l` de `jo.ex`, `row` de `cr_step` ; dans les TU, `build`, `fold_built_from`, `positionals`, `call_row_my` et leurs 12 variables `t`/`spec`/`lq_t`/`h_t`/`b`/`jb`), `overrides` 18 = (a) 18 (les trois ponts et leurs deux commentaires de TU) ; (c) = 1, hors de toute catégorie comptée : `pico8_bz()` → `pico8_pj_bz()`, renommage à type égal fait après que fa997e7 avait déjà typé son retour, parce que la grep de sortie `pico8_(ix|ha|nb|bz|de)\(\)` contredisait la l. 326 du PRD (qui fait rendre à `pico8_bz` les rangées typées) ; aucun compte n'en bouge. 1a1687a, `ob` 58 lignes = (a) 54 + (b) 4 (`t` de `cr_step` et du prédicat de `cb`, `tech` de `test_pico8_data_init`, `record` du test ponctuel), `overrides` 0, aucune (c). (2) lignes qui gagnent une catégorie : 5e58f7c, `valeur` `help.c` +1, `layout.h` +1, `menu.h` +1, `ui_button.h` +1 ; `type-tag` `test_pico8_js` +1 ; `dispatch` `menu.h` +1, `test_pico8_fq` +1 ; 1a1687a, `ob` +7 et `valeur` +6 dans le test ponctuel (lecture de la référence décodée) ; fa997e7 omettait `valeur` `help.c` +1 et `layout.c` +2 ; c5bee4b : `valeur` `research.h` +1, `test_pico8_hp` +2, `test_pico8_lq` +4, `test_pico8_trade` +2. (3) fa997e7 a aussi ajouté une lecture ob → typée temporaire dans `jo:gh` (`jo_gh_record_tuple`, `planet_display.c` l. 85 : `qe` lu dans l'enregistrement ob pour poser le tuple typé), retirée par c5bee4b : l'ordre 4 avant 3 demandait lui aussi une conversion, le temps d'un commit. (4) c5bee4b : `sizeof(pico8_jy_t)` vaut 48, pas 40 (40 est `pico8_jb_row_t`). (5) La hausse de fuite de c5bee4b à 5e58f7c (+32 o / +4 allocations sur `message_box_hold`, +8 o / +1 sur `game_end_final_score`) vient d'allocations faites une fois par `pico8_fv_pq` à la construction de l'écran d'aide (le `ra` partagé et les `qf` de ses boutons, joignables par `g_bo` avant), pas d'une fuite par frame.

Puces de la phase D, recopiées :

> - Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
> - Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
> - Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

§6, recopié :

> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
> - Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés** par ce plan. Depuis la fusion de master (e2e rapide, #32), seuls `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` restent ; les autres ont été remplacés par les e2e de `tests/e2e/`, qui en portent la garde. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.

Règles du §7, recopiées :

> - **Filets depuis la fusion de master (e2e rapide)** : `FRAMES_EVENTS` n'existe plus ; `make e2e` rejoue les 26 e2e de `tests/e2e/` depuis l'écran titre, `make sanitize-net` et `make class-coverage-net` rejouent ce même jeu, `tools/class_coverage_net_expected.txt` y a été reposé à la fusion ; aucun TU ne rejoue de fichier d'événements : un TU construit son état directement, un rejeu est un e2e de `tests/e2e/` (moins de 180 frames, et il doit ajouter de la couverture de lignes). Les mentions de `FRAMES_EVENTS`, `frames-check` et `float-audit` dans les étapes livrées sont historiques.
>
> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.


### Lecture de ces textes pour D.7b

- **Ce que D.7a a pris à D.7b** (sa note, « Ordre D.7a/D.7b ») : le chaînon `pz` (`pico8_pz`, `pico8_mh_new_table`, `g_bo` et `fv.rb` de `help.c`, `pico8_js_new_table` et les branches `OB_TABLE` de `js:pq`), `g_nf_l`/`g_nf_r` (avec `bu`), et les lignes qui lisaient ses types convertis dans `planet.c`, `planet_family_trade.c`, `ui.c`. **D.7b ne les a plus** : aucun de ces sites n'est rouvert, sauf le TU de la section suivante, qu'un type de D.7a force et que D.7a a manqué.
- **Le majorant** : « majorant de D.7b : ≤ 309 hors test ponctuel (353 moins les 44 lignes `ob` de ses fichiers de `src/`) ». Il suppose les 44 lignes retirées et aucun TU touché ; ce PRD en garde 9 (commentaires, **R_b**, plus bas) et retire ≈ 26 lignes de TU forcées : le compte attendu reste sous 309 (≈ 292, section « Effet »).
- **R** : la note de D.7a le fixe à 13 lignes (`pi.c` 2, `pi.h` 1, `collect.c` 7, `collect.h` 3), qui servent le test ponctuel et meurent à D.8, et fait du « = 187 » de D.7c « 187 + 13 ». D.7b ne touche pas ces 13 lignes.
- **« Correctifs aux messages de commit de D.7a »** : quatre fautes de comptabilité à ne pas refaire — classification (a)/(b)/(c) absente ou fausse, dont une (c) non déclarée (`pico8_bz()` → `pico8_pj_bz()`) ; lignes qui gagnent une catégorie omises ; conversion ob → typée temporaire posée le temps d'un commit (`jo_gh_record_tuple`) ; chiffre faux dans un message (`sizeof`). D.7b les exige **dans le message de chaque commit**, pas dans un commit de rattrapage.
- **Leçon de l'ordre 1, 2, 4, 3, 5, 6 de D.7a** : un type n'est changé qu'avec **tous** ses producteurs et **tous** ses lecteurs dans le même commit ; un lecteur unique partagé par plusieurs familles (là-bas `lq:mn`) fixe le découpage. Ici, relevé plus bas (« Lecteurs uniques »).
- **§6 : aucun** (la ligne le dit). `test_pico8_probe_mm_rnd_order` n'est pas dans la liste du §6 ; il garde pourtant un ordre de tirage (`rnd(pn)` avant `rnd(0.5)`) : sa lecture est réécrite, son intention non, et une mutation en donne la preuve rouge (G).
- **`class-coverage-net` : seulement si le diff touche un constructeur.** Aucun n'est touché (`mm` n'est pas un constructeur, `f_`/`ez` ne changent pas) : non exigé ; si le diff en touche un, il devient exigé pour ce commit.
- **`sanitize-net` : à chaque commit** (règle commune de la liste de D, et consigne de pilotage), y compris le commit qui ne touche que `tests/`.

### État de départ

**Mesuré sur `be2786c`** (branche `claude/plan-pilot-3-7zdft0`, D.7a gradée ; `src/` et `tests/` identiques à `a1302b8`). **Rien n'a été compilé ni lancé** : seuls `make -s data-residue`, `sh tools/data_residue.sh --detail`, la recette d'occurrences de D.7a et des `grep` ont tourné ; le reste vient de la lecture des sources et de `pico8/lua.lua`.

Décisions de l'utilisateur qui engagent D.7b : **un TU ne rejoue jamais un `events.json`** (il construit son état) ; **rien de lent** ; **aucun `*.sha256` ni fichier de `tests/e2e/` touché** ; **un e2e nouveau doit ajouter de la couverture de lignes en moins de 180 frames**. Consigne liante : une étape précédente a été notée FAILED pour avoir fait baisser un compte par tassement ; toute mise en forme, tout renommage à type constant, alias, registre ou déplacement qui baisse un compte sans retirer la construction est interdit.

### Inventaire (mesuré sur `be2786c`)

**`make data-residue`** (lignes / fichiers) et **occurrences** (recette de « Anti-tassement ») :

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 5202 | 131 | 6009 |
| `ob` | 552 | 60 | 613 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 380 | 67 | 606 |
| `ref-audit` | 65 | 16 | 66 |
| `par-nom` | 1009 | 81 | 1064 |
| `dispatch` | 191 | 61 | 193 |
| `overrides` | 0 | 0 | 0 |
| `spec` | 0 | 0 | 0 |
| **résidu D** | **6608 lignes** | **135 fichiers** | — |

**`ob` par groupe** : `src/` 182 lignes (couche `pico8_ob.*` 114, `pico8_stash.*` 11, R de D.7a 13, **fichiers de D.7b 44**) ; test ponctuel 199 ; `test_pico8_ob` 55, `test_pico8_stash` 7 ; autres TU 115. Hors test ponctuel : 353.

**Les 44 lignes `ob` des fichiers de `src/` de la ligne** (16 fichiers ; la ligne annonçait « Empreinte 42 (42, 0) » avant D.7a) — lignes / occurrences, et toutes catégories du fichier :

| Fichier | `ob` | Nature des lignes `ob` | Autres catégories du fichier (l/o) | Sort |
|---|---|---|---|---|
| `planet_family.c` | 12 / 12 | inclusion l. 14 ; `ga:mm` l. 789, 812, 819-822 ; `ga:kj` l. 829, 844, 849, 862, 866 | `valeur` 116/126, `type-tag` 4/7, `ref-audit` 1/1, `par-nom` 59/59 | chunk 2 |
| `particle.c` | 9 / 9 | `ei` l. 132, 151, 153, 159, 168, 178, 182, 186, 190 | `valeur` 28/31, `type-tag` 5/9, `ref-audit` 2/2, `par-nom` 5/5, `dispatch` 1/1 | chunk 2 |
| `particle.h` | 2 / 2 | inclusion l. 8 ; contrat de l'enregistrement l. 17 | `valeur` 2/4 | chunk 2 |
| `probe.c` | 5 / 5 | `g_:mm` l. 407, 439, 444-446 | `valeur` 96/107, `type-tag` 12/22, `par-nom` 42/42 | chunk 2 |
| `probe.h` | 1 / 1 | inclusion l. 10 | `valeur` 8/17, `par-nom` 1/1 | chunk 2 |
| `help.c` | 1 / 1 | `assert` l. 242 sur `pico8_ob_lookup_constructor('v')` | `valeur` 28/34, `type-tag` 1/1, `par-nom` 8/8, `dispatch` 3/3 | chunk 3 |
| `help.h` | 1 / 1 | inclusion l. 8 | `valeur` 5/7, `dispatch` 1/1 | chunk 3 |
| `struct.h` | 1 / 1 | inclusion l. 7 | `valeur` 5/5, `type-tag` 5/5, `par-nom` 4/4, `dispatch` 1/1 | chunk 3 |
| `entity.h` | 1 / 1 | inclusion l. 5 | `valeur` 8/9, `par-nom` 5/6, `dispatch` 1/1 | chunk 3 |
| `slipway.c` | 1 / 1 | inclusion l. 16 | `valeur` 93/110, `type-tag` 8/14, `ref-audit` 2/2, `par-nom` 24/24, `dispatch` 3/3 | chunk 3 |
| `bootstrap.c` | 1 / 1 | inclusion l. 10 | `valeur` 13/13, `type-tag` 2/3, `par-nom` 4/4, `dispatch` 1/1 | chunk 3 |
| `ref_audit.h` | 4 / 4 | commentaire l. 12, 14, 18, 22 (la lecture inverse table ob ↔ liste) | `valeur` 2/2, `type-tag` 14/16, `ref-audit` 21/22 | **R_b** |
| `type_tag.h` | 2 / 3 | commentaire l. 8, 15 (ce que le tag discrimine) | `valeur` 1/2, `type-tag` 14/17 | **R_b** |
| `value.h` | 1 / 1 | commentaire l. 7 (« an ob() table entry ») | `valeur` 29/43, `dispatch` 5/6 | **R_b** |
| `list.h` | 1 / 1 | commentaire l. 13 (« the ob() table … is the other shape ») | `valeur` 8/10, `type-tag` 5/5 | **R_b** |
| `hud.c` | 1 / 1 | commentaire l. 8 (« rather than a pico8_ob() spec ») | — | **R_b** |

Les autres modules de la ligne (`planet.c`, `planet_display`, `planet_trade`, `planet_family_trade`, `wormhole`, `msgbox`, `mouse_reader`, `ui*`, `scroll`, `trade`, `bg`, `ke`, `research_icon`) portent **0** ligne `ob` : rien à y convertir.

**TU forcés** (lignes `ob` que le type changé force ; leurs autres lignes `ob` restent à D.7c) :

| TU | Lignes forcées | Pourquoi | Autres lignes `ob` (D.7c) |
|---|---|---|---|
| `test_pico8_spec_defaults` | 43, 53, 60, 67, inclusion 26 (5) | écrit et relit par `pico8_ob_table_set_name`/`get_name` dans `je.l`, qui est un `pico8_nf_button_t` depuis D.7a (section suivante) | — |
| `test_pico8_g_` | 411, 415 (2) | `fold_particle` lit l'enregistrement de `mm` | 8 |
| `test_pico8_ga` | 421, 425 (2) | idem (`p`, `v`, `a`, `l`) | 5 |
| `test_pico8_hj` | 231, 235, inclusion 31 (3) | idem | — |
| `test_pico8_mx` | 210, 214, inclusion 32 (3) | idem | — |
| `test_pico8_pm` | 209, 213, inclusion 32 (3) | idem | — |
| `test_pico8_ui` | 3046, 3049 (2) | `fold_hj_particle` (plie aussi des `qy` entiers, l. 3059) | 13 |
| `test_pico8_fv` | 516, 536-538 (4) | `test_particle_takes_the_help_palette` construit une particule à la main | 8 |
| `test_pico8_probe_mm_rnd_order` | 52, inclusion 22 (2) | lit `p` de la particule émise | — |
| **Total** | **≈ 26** | | |

Les `assert(… pico8_type_tag_of(pt.as.ref) == PICO8_TYPE_TAG_OB_TABLE)` des six `fold_*particle` (lignes `type-tag`, pas `ob`) tombent avec eux : la particule typée n'a pas d'en-tête (voir « Faits »).

### Découvert en écrivant ce PRD : un TU écrit par une table ob dans le bouton typé de D.7a

`test_second_jl_keeps_specs` (`test_pico8_spec_defaults.c`, l. 43-68) lit `je.l` par nom, le prend pour un `pico8_ob_table_t *` et y pose une clé `mark` par `pico8_ob_table_set_name`. Depuis `e252fde` (D.7a, chunk 1), `je.l` rend un REF vers `g_nf_l`, un `pico8_nf_button_t` statique de 16 octets (`has_on`, `on`, `jn`, `hm`). Dans un build ordinaire, `PICO8_REF_AUDIT_EXPECT_OB_TABLE` ne vérifie rien : l'écriture prend `jn`/`hm` pour le pointeur `entries`, puis écrit `count`/`capacity` **au-delà des 16 octets** de `g_nf_l`, dans `g_nf_r` voisin (ou une zone rouge d'ASan, si les TU en étaient compilés). Le test passe parce que la mémoire corrompue relit sa propre écriture ; il ne vérifie plus rien de `nf`. D.7a ne l'a pas listé parmi ses suiveurs forcés. **D.7b le répare** (chunk 1) : la ligne met `mouse_reader` avec `g_nf_l`/`g_nf_r` dans D.7b, et c'est un TU forcé par un type de ce groupe. Hypothèse H7.

### Ce que dit le cart (`pico8/lua.lua`)

- `f_(self)` (l. 685) : `self.qy={}`. `ez(self,n)` (l. 687) : `add(self.qy,self:mm())` tant que `rnd()<n`.
- `ei(self)` (l. 692-706) : pour chaque `p` de `self.qy` (`pairs`) : `local pv=p.v` ; `p.p+=pv` (nouveau vecteur) ; `p.v.x=…` puis `p.v.y=qg*pv.x+…` (en place : `pv` **est** `p.v`, la seconde ligne lit le `x` qu'on vient d'écrire) ; `if (p.a) p.v+=p.a` (nouveau vecteur) ; `p.l-=0.14` ; `del` si `p.l<=1`, sinon `pset`. Puis `ez(self,self.fc)`.
- `g_:mm()` (l. 727) : `{p=p, v=self.pk and v:pa() or v, l=self.ki}` — pas de `a`.
- `ga:mm()` (l. 1081) : `{p=p, v=p*self.ms, a=p*self.iu, l=rnd(self.py)+self.py}` — trois vecteurs neufs.
- `ga:kj(p)` (l. 1087) : lit `pt.p:qx()` et `pt.l` des trois premiers `qy[i]`, puis `ei(self)`, `self.p_=not pt`.

Aucun vecteur d'une particule n'est partagé : ni entre particules, ni avec un autre porteur. Muter `p` et `v` en place dans une struct rend donc les mêmes valeurs que les vecteurs neufs du cart (H2).

### Faits qui engagent la forme

- **Producteurs** : `pico8_g__mm` (`probe.c`, partagé par `g_`, `fv`, `hj`, `pm`, `mx`, `planet_family_trade`) et `pico8_ga_mm` (`planet_family.c`). Chacun alloue aujourd'hui une table ob, ses entrées, les chaînes de ses clés et deux ou trois vecteurs.
- **Lecteurs** : `pico8_ei` (`particle.c`), qui lit et **écrit** `p`, `v`, `l`, lit `a`, et **réalloue `p` à chaque frame** (et `v` quand `a` est présent) — une fuite par particule et par frame (TROU-7) ; `pico8_ga_kj` (`planet_family.c`), qui lit `p` et `l`. `ez` ne fait que stocker le REF rendu par `pico8_mv(self, "mm", …)`.
- **Conteneur** : `qy` reste une `pico8_list_t` d'éléments `pico8_value_t` REF (point 3 de l'arrêt, D.9 ; l'étude D.9 le note : « D.7b [change] celui de `qy` », son pointé). `pico8_list_del(qy, p_value)` compare des REF : inchangé.
- **Pas d'en-tête** : l'enregistrement typé n'a pas de `pico8_type_tag_t` en tête ; aucun lecteur ne doit plus l'interroger par `pico8_type_tag_of` (l'`assert` de `ei` l. 149-150 et ceux des six `fold_*particle` des TU tombent, rien ne les remplace par un tag).
- **Dispatch inchangé** : `mm` garde sa signature `pico8_value_t (void *, int, pico8_value_t, pico8_value_t)` (`dispatch`, D.9) et rend un REF vers l'enregistrement typé.
- **Lecture par nom inchangée** : `ei`, `ez`, `f_` lisent `fa`, `ra`, `qo`… et `qy` par nom (classe M de l'étude D.9) : D.7b n'y touche pas, `par-nom` ne bouge pas.

### Lecteurs uniques (la leçon de `lq:mn`)

1. **`ei` est le lecteur unique de tous les enregistrements de `mm`**, des deux producteurs ; `ga:kj` est le second lecteur de ceux de `ga`. Typer un producteur sans `ei` demanderait un `ei` à double lecture (tag ob ou struct), typer `ei` sans les deux producteurs une conversion : les deux sont interdits. **Les deux producteurs, les deux lecteurs et les huit TU qui lisent un enregistrement forment un seul commit** (chunk 2). Il n'y a aucun autre lecteur (H1 : `grep` de `"mm"`, `_mm`, `qy`, `pico8_ei` sur `src/` et `tests/`).
2. **L'`assert` de `pico8_help_init` est le seul lecteur du registre de `ob()` hors de la couche, de `pi.c` et du test ponctuel.** L'inclusion de `pico8_ob.h` par `help.h` ne peut partir qu'avec lui (même commit, chunk 3).
3. **`pico8_struct.h` est le seul chemin par lequel `slipway.h`, `ui.h` et `wormhole.h` reçoivent `pico8_list.h`** (par `pico8_ob.h`). Mesuré par un parcours du graphe d'inclusion de `src/` et `tests/` : retirer les huit inclusions de `pico8_ob.h` de la ligne (et celles des cinq TU qui n'en ont plus l'usage) ne fait perdre `pico8_value.h` ni `pico8_type_tag.h` à aucun fichier, et ne fait perdre `pico8_list.h` qu'à ces trois en-têtes, qui l'incluent donc eux-mêmes dans le commit qui retire celle de `struct.h` (chunk 3). `pico8_list.h` ne correspond à aucun motif : aucune ligne ne gagne de catégorie.
4. **`je.l` n'a qu'un lecteur par table ob restant** : le TU de la section précédente (chunk 1). `test_pico8_je` l. 85 ne teste que la présence.

## Solution

**Trois commits, sans aucun pixel changé**, chacun vert seul :

1. **`nf.l` lu typé dans `test_pico8_spec_defaults`** — le test pose une marque dans un champ du `pico8_nf_button_t` de `je.l` et vérifie qu'elle survit au second `jl()`, même intention ; son inclusion de `pico8_ob.h` part. Aucun fichier de `src/`.
2. **Particules de `qy` typées** — un enregistrement `{p, v, a présent ou non, l}` alloué une fois par `mm` (là où la table ob et ses vecteurs l'étaient), muté en place par `ei` (plus d'allocation par frame), lu par `ga:kj` ; contrats de `mm` et du module réécrits ; inclusions mortes de `particle.h`, `probe.h`, `planet_family.c` et de trois TU retirées ; huit TU suivent.
3. **Inclusions mortes et `assert` d'init** — `struct.h`, `entity.h`, `help.h`, `slipway.c`, `bootstrap.c` n'incluent plus `pico8_ob.h` ; `slipway.h`, `ui.h`, `wormhole.h` incluent `pico8_list.h` ; l'`assert` de `pico8_help_init` interroge le registre que `fv:kj` lit vraiment (`bl` → `pico8_pi_dispatch_lookup`) ; note « Découvert à D.7b ».

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert ; `make e2e` : **26 conformes** ; `make sanitize-net` : 26 propres ; `make data-guard` : code 0, `DATA_RESIDUE_GUARD` = `spec,overrides` inchangé ;
- `make data-residue` : `ob` **hors `pico8_ob.*` et `pico8_stash.*`, dans `src/`, = R, 22 lignes** (liste close) ; `ob` total ≈ 491 lignes (hors test ponctuel ≈ 292, ≤ 309) ; `stash` 28 / 4, `spec` 0, `overrides` 0 ; `ob`, `valeur`, `type-tag` strictement plus bas ; `par-nom` 1009, `dispatch` 191, `ref-audit` 65 inchangés (tout écart expliqué) ;
- **fuite par frame** (LeakSanitizer) : jamais plus haute qu'au départ, à aucun commit ; **strictement plus basse** après le chunk 2 ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  # 1. ob dans src/ hors des deux modules : les 22 lignes de R, rien d'autre
  grep -rnE '\bpico8_ob(_[A-Za-z0-9_]+)?\b|\bPICO8_OB_[A-Z0-9_]+|\bpico8_fi(_fn)?\b|pico8_ob\.h' src --include='*.[ch]' \
    | grep -vE '^src/pico8_(ob|stash)\.[ch]:' | cut -d: -f1 | sort | uniq -c
  #    attendu : 7 collect.c, 3 collect.h, 1 hud.c, 1 list.h, 2 pi.c, 1 pi.h, 4 ref_audit.h, 2 type_tag.h, 1 value.h
  # 2. inclusions de pico8_ob.h dans src/ : pico8_ob.c, pico8_collect.h, pico8_stash.h seulement
  grep -rln 'include "pico8_ob.h"' src
  # 3. le tag OB_TABLE dans src/ : pico8_ob.c, pico8_ob.h, pico8_ref_audit.h, pico8_type_tag.h seulement
  grep -rln 'PICO8_TYPE_TAG_OB_TABLE' src
  # 4. vide
  grep -rnE 'pico8_ob_lookup_constructor' src --include='*.[ch]' | grep -vE '^src/pico8_ob\.[ch]:'
  # 5. vide : plus aucun TU ne lit une particule ou nf.l comme table ob
  grep -rnE 'pico8_ob_table_(get|set)_name\([^,]*(pt|particle|nf_l)' tests
  ```

## User Stories

1. En tant que mainteneur du port, je veux qu'une particule de `qy` soit une struct typée, afin que `ei` et `ga:kj` lisent `p`, `v`, `a`, `l` par des champs et non par nom dans une table ob.
2. En tant que gardien de la mémoire, je veux que `ei` mette à jour `p` et `v` en place, afin qu'aucun vecteur ne soit plus alloué par particule et par frame.
3. En tant que gardien de la mémoire, je veux qu'un `mm` fasse **une** allocation typée là où il en faisait cinq ou plus (table, entrées, clés, vecteurs), et jamais une allocation là où il n'en faisait pas.
4. En tant que gardien de la mémoire, je veux une mesure LeakSanitizer avant et après chaque commit sur `message_box_hold` et `game_end_final_score`, afin de prouver que la fuite ne croît jamais et baisse au chunk 2.
5. En tant que gardien du comportement, je veux que la seconde ligne de rotation de `ei` lise le `x` qu'elle vient d'écrire (`pv` aliasait `p.v` dans le cart), afin que la trajectoire des particules reste identique.
6. En tant que gardien du comportement, je veux que `a` absent (particules de `g_`) et `a` présent (particules de `ga`) prennent les mêmes branches qu'avant, afin que `v` ne soit accéléré que pour `ga`.
7. En tant que gardien du comportement, je veux que `del` retire la même particule au même moment et que l'élément qui glisse dans la place libérée soit toujours sauté cette frame (`pairs`), afin que les frames soient identiques.
8. En tant que gardien du comportement, je veux que `ga:kj` lise `p` et `l` des trois premiers emplacements et pose `p_` d'après le troisième seul, afin que l'éclat meure au même moment.
9. En tant que gardien de l'ordre de tirage, je veux que `g_:mm` tire toujours `rnd(pn)` avant `rnd(0.5)`, et `ga:mm` ses tirages dans le même ordre, afin que le RNG reste aligné sur le cart.
10. En tant que gardien des TU, je veux que chaque `fold_*particle` plie les mêmes valeurs dans le même ordre, afin qu'aucune constante `*_HASH` ne change.
11. En tant que gardien des TU, je veux que la particule construite à la main par `test_pico8_fv` soit une struct du TU, un champ par ligne, afin que le test dise ce qu'il construit.
12. En tant que gardien des TU, je veux que `test_second_jl_keeps_specs` pose sa marque dans un champ du bouton typé `je.l`, afin qu'il cesse d'écrire hors de `g_nf_l` et vérifie de nouveau que le second `jl()` ne remet pas `nf` à zéro.
13. En tant que gardien de `test_pico8_probe_mm_rnd_order`, je veux une preuve rouge (tirages permutés → rouge), afin que sa réécriture garde sa garde.
14. En tant que gardien du compteur, je veux que les inclusions de `pico8_ob.h` devenues inutiles partent, et que les en-têtes qui recevaient `pico8_list.h` par elles l'incluent eux-mêmes, afin que la baisse soit une dépendance retirée et non un déplacement.
15. En tant que gardien du compteur, je veux que l'`assert` d'ordre d'init de `pico8_help_init` interroge le registre que `fv:kj` lit (`bl`), afin que la précondition dise ce dont l'aide dépend, sans passer par `ob()`.
16. En tant que gardien du compteur, je veux que les neuf lignes de commentaire qui décrivent la couche ob encore vivante restent, admises dans R et nommées, afin qu'aucune baisse ne vienne d'un commentaire réécrit pour échapper au motif.
17. En tant que gardien du compteur, je veux, dans le message de chaque commit, les comptes avant/après **en lignes et en occurrences**, par catégorie et par fichier, afin qu'une baisse prouve une construction retirée.
18. En tant que gardien du compteur, je veux dans chaque message la classification (a)/(b)/(c) de chaque ligne `ob` retirée, avec (c) = 0 ou déclarée et justifiée, afin que le correctif de D.7a ne soit plus nécessaire.
19. En tant que gardien du compteur, je veux dans chaque message la liste, par fichier, des lignes qui gagnent une catégorie, afin qu'aucun déplacement ne passe inaperçu.
20. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, aucun identifiant compté renommé à type constant, aucun alias ni registre ajouté, afin qu'aucune baisse ne soit un tassement.
21. En tant que gardien des filets, je veux `make e2e` à 26 et `make sanitize-net` à 26 propres à chaque commit, afin de prouver qu'aucun pixel ni accès mémoire n'a changé.
22. En tant que `plan-pilot`, je veux la décision sur R (22 lignes, dont 9 nouvelles) écrite avec les options rejetées et le texte de note qui modifie D.7c et D.8, afin de l'enregistrer.
23. En tant que `plan-pilot`, je veux le TU corrompu de D.7a nommé comme une surprise, afin de le relayer à la revue de phase.
24. En tant que lecteur de `docs/glossaire-noms.md`, je veux une entrée pour l'enregistrement de `qy`, afin de retrouver `{p,v,a,l}` du cart depuis le nom C.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Cible et reste R (G)

La ligne dit « 0 dans `src/` hors ces deux modules ». **Retenu : R, liste close de 22 lignes**, dont 13 de D.7a (inchangées) et **9 nouvelles, R_b**, toutes des commentaires :

| Fichier | Lignes | Ce qu'elles disent | Pourquoi elles restent | Meurent à |
|---|---|---|---|---|
| `pi.c` | 2 | registres `'v'`, `'s'` de `ob()` | test ponctuel (D.7a) | D.8 |
| `pi.h` | 1 | commentaire des deux registres | idem | D.8 |
| `collect.c` | 7 | `pico8_ho_apply` (6), registre `'b'` (1) | idem | D.8 |
| `collect.h` | 3 | inclusion, déclaration | idem | D.8 |
| `type_tag.h` | 2 (l. 8, 15) | ce que le tag discrimine, dont `pico8_ob_table_t` ; le risque de lire une liste comme table ob | `PICO8_TYPE_TAG_OB_TABLE` et `pico8_ob.h` existent jusqu'à D.8 : le commentaire est vrai | D.8 (qui retire `PICO8_TYPE_TAG_OB_TABLE`) |
| `ref_audit.h` | 4 (l. 12, 14, 18, 22) | la lecture inverse, table ob lue comme liste | idem ; les TU de D.7c construisent encore des tables ob | D.8 |
| `value.h` | 1 (l. 7) | « an ob() table entry (pico8_ob.h) » | idem | D.8 |
| `list.h` | 1 (l. 13) | « the ob() table … is the other shape » | idem | D.8 |
| `hud.c` | 1 (l. 8) | « rather than a pico8_ob() spec » | idem : `ob()` existe | D.8 |

- **Rejeté : réécrire ces neuf commentaires** pour qu'ils ne nomment plus `pico8_ob_table_t`/`ob()`. Leur sujet est toujours là ; les reformuler ne retire aucune construction : c'est la ligne (c) « même type, autre nom » qu'interdit la consigne, faite pour échapper au motif.
- **Rejeté : les supprimer.** L'anti-tassement interdit de supprimer un commentaire qui ne porte pas sur une table, une vue ou un pont retirés par l'étape.
- **À D.8** (pas ici) : quand `pico8_ob.h` et `PICO8_TYPE_TAG_OB_TABLE` disparaissent, ces lignes deviennent fausses ; leur retrait ou leur réécriture y est alors (a) « commentaire d'une table qui n'existe plus ».
- **Conséquence pour le plan** (note « Découvert à D.7b ») : R = 22 ; le « 187 + 13 » de D.7c devient « 187 + 22 ».

### Forme de l'enregistrement (G)

Forme de décision (le nom est indicatif ; l'implémenteur garde la convention du module) :

```c
/* un element de qy : ce que rend mm, {p,v,a,l} du cart ; a absent pour g_ */
typedef struct {
    pico8_vec_t p;
    pico8_vec_t v;
    int has_a;
    pico8_vec_t a;
    pico8_fixed_t l;
} pico8_qy_t;
```

- **Où** : `pico8_particle.h`, qui porte déjà le contrat « ce que tout `mm` doit rendre » ; `probe.c` et `planet_family.c` l'incluent déjà. Le contrat en commentaire (l. 13-19) est réécrit pour ce type : (b), contrat dont le type passe d'une table ob à une struct. Idem pour les commentaires de `pico8_g__mm` (`probe.h`) et `pico8_ga_mm` (`planet_family.h`) : « une allocation, jamais libérée » (TROU-7 gardé à ces deux sites).
- **Producteurs** : `mm` alloue **un** `pico8_qy_t` (un seul `malloc`), le remplit champ par champ **après** ses tirages (ordre des `rnd` inchangé), rend `pico8_value_ref(record)`. `g_:mm` : `has_a = 0`.
- **`ei`** : lit l'élément comme `pico8_qy_t *` ; `pv = rec->v` (instantané) ; `rec->p = p + pv` en place ; `rec->v.x = qo*pv.x + ro*pv.y` puis `rec->v.y = qg*rec->v.x + q_*pv.y` (le commentaire « In Lua `pv` aliases `p.v` » est gardé) ; si `has_a`, `rec->v += rec->a` en place ; `rec->l -= 0.14` ; `del` ou `pset` comme avant, `pset` à `cx + rec->p.x`. Les branches « `v` absent : `continue` » et « `p` absent : 0 » disparaissent (le type rend l'absence impossible : aucun producteur n'omet `p`, `v` ni `l`) ; `pico8_particle_truthy` (seul appelant : `a`) disparaît ; le `/* TROU-7 */` de l'allocation de `new_pos` disparaît avec l'allocation (précédent : `jc.ts` à D.7a).
- **`ga:kj`** : `pt` devient `const pico8_qy_t *` ; `pt->p`, `pt->l` ; `last_pt` idem ; le commentaire TROU-21 et celui du troisième emplacement sont gardés.
- **Rejetés** : garder des vecteurs alloués dans la struct (`pico8_vec_t *p`) — l'allocation par frame resterait, sans objet puisqu'aucun vecteur n'est partagé (H2) ; un tag en tête de la struct pour garder les `assert` — une forme nouvelle sans lecteur ; un tableau fixe de particules par entité — `qy` est une liste (D.9), et changer son conteneur sort de la ligne ; **libérer** l'enregistrement retiré par `del` — changement de TROU-7 hors périmètre (la fuite baisse sans lui).

### Chunk 1 — `nf.l` dans `test_pico8_spec_defaults` (G)

- `nf_l_of` rend `pico8_nf_button_t *` (le REF de `je.l`, qui est `&g_nf_l`) : (b). La marque devient une écriture d'un champ du bouton (par exemple `hm`, qu'aucun `bu` ne réécrit sans entrée pendant `jl()` : H7), relue après `jl()` avec l'identité du bouton ; les deux lignes `pico8_ob_table_set_name`/`get_name` : (a). L'inclusion de `pico8_ob.h` : (a), si plus rien ne l'emploie dans le TU.
- `pico8_mouse.h` fournit le type (le TU l'inclut si besoin : aucun motif).
- **Preuve rouge** : `pico8_nf_init` remet `g_nf_l` à zéro même quand `nf` est prêt (mutation M1) → le test rougit ; restauré. Consignée au commit.
- Aucun fichier de `src/` ; fuite et `sanitize-net` inchangés par construction (mesurés quand même).

### Chunk 2 — particules de `qy` (G)

- `src/` : `particle.h` (type, contrat, inclusion (a)), `particle.c` (`ei`), `probe.c` (`g_:mm`), `probe.h` (contrat ; inclusion (a) : `probe.h` n'emploie rien de `pico8_ob.h`), `planet_family.c` (`ga:mm`, `ga:kj`, inclusion (a)), `planet_family.h` (contrat).
- TU : `fold_particle` de `g_`, `ga`, `hj`, `mx`, `pm`, `fold_hj_particle` de `ui` lisent `pico8_qy_t` : **mêmes valeurs, même ordre** (`p.x`, `p.y`, `v.x`, `v.y`, puis `a` pour `ga`, puis `l`), mêmes aides `fnv`/`fold_vec` ; `assert` de tag remplacé par `pt.kind == PICO8_VALUE_REF && pt.as.ref != NULL` (qui y est déjà) ; `test_pico8_fv` construit la particule en `static pico8_qy_t` du TU (elle reste dans `qy` de l'aide après le test) ; `test_pico8_probe_mm_rnd_order` lit `rec->p`. Les inclusions de `pico8_ob.h` de `hj`, `mx`, `pm`, `probe_mm_rnd_order` partent si le TU n'en emploie plus rien (le graphe d'inclusion dit qu'ils ne perdent rien d'autre).
- `docs/glossaire-noms.md` : entrée « élément de `qy` » → `pico8_qy_t`.

### Chunk 3 — inclusions et `assert` d'init (G)

- `help.c` : `assert(pico8_ob_lookup_constructor('v') != NULL && "pico8_help_init() runs after pico8_pi_init()")` → même message, prédicat `pico8_pi_dispatch_lookup("s") != NULL` (le registre par chaîne que `bl` lit pour `fv.rb`, dont deux articles ont `fn = "s"` ; rempli par `pico8_pi_init`). Classée (b) : la précondition garde son objet (« `pi_init` a tourné »), sa lecture passe du registre de `ob()` au registre typé dont l'aide dépend réellement. **Rejetés** : supprimer l'`assert` (précondition vraie et utile, `help` n'appelle pas `pi_init` lui-même) ; ajouter un `pico8_pi_ready()` (API nouvelle alors qu'une lecture existe).
- `help.h`, `struct.h`, `entity.h`, `slipway.c`, `bootstrap.c` : inclusion de `pico8_ob.h` retirée, (a). `slipway.h`, `ui.h`, `wormhole.h` : `#include "pico8_list.h"` ajouté (ce qu'ils recevaient par `struct.h` → `pico8_ob.h`) ; aucun autre ajout attendu (le graphe d'inclusion le dit) ; un ajout imposé par la compilation est listé au commit, et s'il nomme `pico8_value.h` il gagne une ligne `valeur`, listée.
- `docs/plan-3.md` : note « Découvert à D.7b » en fin de ligne D.7b (texte dans Further Notes). Rien d'autre dans le plan.

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucun ordre de construction, aucun tirage du RNG, aucune valeur ; aucun constructeur, aucun appel `pico8_class_coverage_record`, `KNOWN[]`.
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements ; aucun e2e ajouté.
- Les 13 lignes de R de D.7a ; les 9 lignes de R_b ; `pico8_ob.*`, `pico8_stash.*`, le test ponctuel ; `tools/` ; le `Makefile` (`DATA_RESIDUE_GUARD` = `spec,overrides`).
- Les lectures par nom (`par-nom`), `pico8_mv`, `methods[]`, la représentation de `pico8_list_t` (D.9) ; les sites que D.7a a pris.
- Aucun défaut muté nouveau : la ligne n'en porte plus (`nf.l`/`nf.r` sont statiques non `const` remis à `pico8_nf_init` depuis D.7a).

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec.

- **Interdit** : joindre sur une ligne des déclarations, instructions, arguments, conditions ou champs qui étaient sur des lignes distinctes ; retirer des retours à la ligne dans une ligne comptée ; renommer un identifiant, un paramètre ou un commentaire compté **sans que le type de la donnée change** ; réécrire un commentaire compté dont le sujet existe toujours (R_b) ; macro, `typedef`, pointeur ou fonction enveloppe dont le seul rôle est de renommer un identifiant compté ; **un alias** ; **un registre** (tableau, table, `switch` ou boucle nouveaux qui nomment ensemble classes, clés ou valeurs attendues) dans un TU ; une aide de TU qui reconstruit une table par nom ; déplacer du code compté hors de `src`/`tests`, hors `.c`/`.h`, ou dans le test ponctuel ; supprimer un commentaire qui ne porte pas sur une donnée ou une inclusion retirées ; **toute vue ob nouvelle, toute `pico8_ob_table_new`, par appel, par frame ou par init** ; toute conversion typée ↔ ob, même le temps d'un commit (leçon de `jo_gh_record_tuple`) ; une allocation typée à un site qui n'allouait rien ; une donnée à deux formes à la fin d'un commit.
- **Classification exigée, au message de chaque commit** : chaque ligne `ob` retirée est (a) construction supprimée (allocation, pose ou lecture par nom, `assert` de tag, inclusion devenue inutile, commentaire d'une donnée qui n'existe plus), (b) signature, contrat, déclaration ou variable dont le type passe d'une table ob à un type C, ou (c) même type, autre nom. **(c) = 0** attendu ; une (c) inévitable est déclarée et justifiée ligne à ligne, jamais découverte après coup. Recette : `git diff -U0` par fichier, lignes `-` qui portent le motif `ob` en regard des lignes `+`. Attendu : chunk 1, (a) 3-4 + (b) 1-2 ; chunk 2, (a) majoritaire, (b) le contrat de `particle.h` et les variables `p`/`pt`/`last_pt`/`particle` retypées ; chunk 3, (a) 5 inclusions + (b) 1 (`assert`).
- **Mesure exigée, avant et après chaque commit** : par catégorie, **lignes** (`make data-residue`, et `--detail` par fichier) **ET occurrences**, le test ponctuel donné à part. Recette figée (`$S` : dossier du scratchpad) :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  X=$(sed -n "s/^SPEC_EXCLUSION='\(.*\)'\$/\1/p" tools/data_residue.sh)
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E "$X" "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  # par fichier : for f in <fichiers du diff>; do for c in …; do grep -cE / grep -oE … | wc -l; done; done
  ```

  Mesuré sur `be2786c` : `valeur` 6009, `ob` 613, `stash` 28, `type-tag` 606, `ref-audit` 66, `par-nom` 1064, `dispatch` 193, `overrides` 0, `spec` 0.
- **Contrôle** : pour chaque (catégorie, fichier) du diff, Δlignes et Δoccurrences ; **si Δlignes > Δoccurrences**, le message justifie ce couple (une construction retirée par ligne, pas une construction qui réapparaît ailleurs).
- **Lignes qui gagnent une catégorie**, listées au message, **par fichier et par catégorie**, y compris zéro (« aucune ») : attendu aucune ; une ligne `+` qui porte un motif qu'aucune ligne `-` en regard ne portait est une ligne gagnée.
- **Fichiers hors de l'empreinte** : aucun changement ; tout écart est dit et justifié au commit (≤ 3 chemins).

### Empreinte remesurée (G)

| Chunk | `src/` | `tests/` | docs | Total |
|---|---|---|---|---|
| 1 | — | `spec_defaults` (1) | — | 1 |
| 2 | `particle.c/.h`, `probe.c/.h`, `planet_family.c/.h` (6) | `g_`, `ga`, `hj`, `mx`, `pm`, `ui`, `fv`, `probe_mm_rnd_order` (8) | glossaire | 15 |
| 3 | `help.c/.h`, `struct.h`, `entity.h`, `slipway.c`, `bootstrap.c`, `slipway.h`, `ui.h`, `wormhole.h` (9) | — | `plan-3.md` | 10 |
| **Distincts** | **15** | **9** | **2** | **26** |

- **Écart avec la ligne (« Empreinte 42 (42, 0) »)** : D.7a a pris le chaînon `pz`, `nf` et leurs suiveurs ; restent 16 fichiers de `src/` à ligne `ob`, dont 5 gardés en R_b ; s'ajoutent les trois en-têtes qui reçoivent `pico8_list.h`, `planet_family.h` (contrat, sans ligne comptée) et les TU forcés.
- **Découpage (G) : trois commits**, ordre 1 (indépendant), 2, 3 (3 après 2 : `particle.h`/`probe.h` ne perdent leur usage d'`ob` qu'au chunk 2, et `help.c` reçoit `pico8_ob.h` par `help.h`, `probe.h` et `particle.h` jusque-là). **Rejetés** : **un commit** (un rouge sur la particule bloquerait le correctif de TU) ; **chunk 2 scindé producteurs/lecteurs** (double forme dans `ei`, voir « Lecteurs uniques ») ; **chunk 1 fondu dans 3** (c'est un correctif d'un défaut de D.7a : son commit doit se lire seul, comme `63670b3`) ; **chunk 3 avant 2** (les inclusions de `particle.h`/`probe.h` y sont encore employées).

### Effet sur `make data-residue`

| Catégorie | Départ (`be2786c`) | Après D.7b | Pourquoi |
|---|---|---|---|
| `ob` | 552 / 613 | **≈ 491** (−35 `src/`, ≈ −26 TU) ; hors test ponctuel ≈ 292 (≤ 309) | `src/` : 182 → **147** exact (couche 114 + stash 11 + R 22) |
| `ob` dans `src/` hors `pico8_ob.*`/`pico8_stash.*` | 57 (13 R + 44) | **22 = R** exact | liste close |
| `valeur` | 5202 / 6009 | strictement plus bas | poses `pico8_value_ref`/`fixed` des tables de `mm`, tests de sorte `PICO8_VALUE_REF`/`FIXED` de `ei`/`ga:kj`, `pico8_particle_truthy`, lignes des TU |
| `type-tag` | 380 / 606 | strictement plus bas (≈ −7) | `assert` de `ei`, six `fold_*particle` |
| `par-nom` | 1009 / 1064 | **1009** | aucun site par nom touché (étude D.9 : « D.7b n'enlève aucun site par nom ») |
| `dispatch` | 191 / 193 | **191** | `mm` garde sa signature |
| `ref-audit` | 65 / 66 | **65** | `particle.c` garde `PICO8_REF_AS_LIST` |
| `stash`, `spec`, `overrides` | 28, 0, 0 | 28, 0, 0 | — |

Le compte « ≈ » est remesuré au commit ; « = » et « exact » sont des égalités.

### Hypothèses (rien n'a été lancé)

- **H1** : aucun producteur ni lecteur d'un enregistrement de `qy` hors de `g__mm`, `ga_mm`, `ei`, `ga:kj` et des huit TU nommés (grep de `"mm"`, `_mm`, `qy`, `pico8_ei`, `pico8_ob_table_get_name(pt` sur `be2786c`).
- **H2** : aucun vecteur d'une particule n'a d'autre porteur (cart l. 727, 1081 ; C : `p_heap`/`v_heap`/`a_heap` locaux à `mm`) : la mutation en place rend les mêmes valeurs.
- **H3** : aucune constante `*_HASH` ne bouge (mêmes valeurs pliées dans le même ordre ; `fold_value` générique de `hp`/`hy` ne plie aucune particule). Démentie : arrêt sur la constante, jamais de mise à jour.
- **H4** : les deux e2e liants font tourner `ei` sur un `qy` non vide — `game_end_final_score` revient à l'écran titre, où `fv:kj` (`help.c` l. 78) appelle `ei` ; `message_box_hold` est en partie, avec des émetteurs `hj`/sondes : la fuite y baisse strictement au chunk 2. Démentie pour un e2e : il doit rester **égal** (jamais plus haut), la preuve qu'il n'atteint pas `ei` avec un `qy` non vide (gcov ou compteur de scratchpad) est au commit, et la baisse stricte est exigée sur l'autre et sur `tech_geoharvesting_burst` (éclat `ga`, mesure complémentaire).
- **H5** : la fuite de départ sur `be2786c` est celle de la fin de D.7a : `message_box_hold` 19213 o / 1099 allocations, `game_end_final_score` 59310 o / 1481. Démentie : le départ mesuré fait foi, l'écart est dit.
- **H6** : `pico8_pi_dispatch_lookup("s")` est non NULL exactement après `pico8_pi_init` ; `test_pico8_fv` (l. 643) et `cart.c` (l. 60, 86) appellent `pico8_help_init` après `pico8_pi_init` ; aucun TU n'appelle `pico8_pi_dispatch_clear` avant.
- **H7** : `test_second_jl_keeps_specs` écrit aujourd'hui hors de `g_nf_l` (section « Découvert ») ; le second `jl()` ne remet pas `g_nf_l` à zéro (`pico8_nf_init` ne tourne pas sur un `nf` prêt) et aucun `bu` ne réécrit le champ marqué pendant `jl()`.
- **H8** : les retraits d'inclusion compilent avec les trois seuls ajouts de `pico8_list.h` ; `-Werror` reste muet.
- **H9** : aucun constructeur touché : `class-coverage-net` non exigé.
- **H10** : `make sanitize-net` reste propre ; LeakSanitizer (`detect_leaks=1`) tourne dans le conteneur (il a tourné à D.7a).
- **H11** : les e2e atteignent `g__mm`, `ga_mm` (`tech_geoharvesting_burst`), `ei`, `ga:kj` ; un site non atteint est prouvé par un TU.
- **H12** : `constants-check`, `float-grep`, `float-guard` restent verts.
- **H13** : durées courtes : `make test` quelques secondes plus une recompilation ; `make e2e` ~1 s ; `sanitize-net` < 2 min ; mesure de fuite < 1 min par e2e.

## Implementation Chunks

Trois commits, dans cet ordre, chacun vert seul (`make test`, `make e2e`, `make sanitize-net`, `make data-guard`, mesure de fuite), chacun avec ses comptes, sa classification, ses lignes gagnées, et (chunk 2) son rapport gcov/mutations :

1. **`nf.l` lu typé** — `test_pico8_spec_defaults` seul ; preuve rouge M1.
2. **Particules de `qy`** — `pico8_qy_t`, `g_:mm`, `ga:mm`, `ei` en place, `ga:kj`, contrats ; huit TU ; inclusions mortes de `particle.h`, `probe.h`, `planet_family.c` et des TU ; glossaire.
3. **Inclusions et `assert` d'init** — cinq inclusions retirées, trois `pico8_list.h` ajoutés, `assert` de `pico8_help_init` sur le registre de `bl` ; note du plan.

Ordre conseillé dans chaque commit : mesure de départ (lignes et occurrences par fichier ; fuite) ; `src/` ; `make test` ; TU ; `make test` ; `make e2e`, `make sanitize-net` ; gcov, mutations ; mesure d'arrivée, classification, lignes gagnées, greps ; docs ; message.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, identités, pixels), pas la présence d'une ligne. D.7b change la forme d'une donnée, pas ce qu'elle vaut : **la preuve est que chaque producteur rend, et chaque lecteur lit et écrit, les mêmes valeurs qu'avant, et que tout ce qui mesure le comportement rend la même chose** — plus une fuite qui ne croît jamais.

### TU touchés (G)

- **Règle** : un TU n'est touché que pour les lignes qu'un type changé force ; ses autres fixtures ob restent à D.7c. Chaque état construit l'est directement (struct du TU, un champ par ligne, ou appel du producteur réel) ; aucune aide nouvelle qui reconstruise une table par nom ; aucun registre ; aucun rejeu d'événements ; les aides existantes (`fold_particle`, `fold_hj_particle`, `emitted_particle_p`, `nf_l_of`) sont retypées, sans cas ajouté.
- **Constantes** : aucune constante `*_HASH` ne change, ni de valeur ni de nom.
- **Test ajouté (G)** : un seul, dans `test_pico8_g_` ou `test_pico8_ga` (le TU où il s'écrit sans aide nouvelle) : `ei` sur une particule construite (`p`, `v`, `a` connus, `l` > 1) rend, après une frame, `p = p0 + v0`, `v` tourné avec la seconde ligne sur le `x` neuf, `v += a` si `has_a`, `l − 0.14`, et **la même adresse d'enregistrement** dans `qy` (mise à jour en place). Il couvre ce que les hachages de `mm` ne voient pas (`ei` lui-même).

### Preuve des tests (§7, gcov + mutations)

- **gcov** (copie jetable du scratchpad, `--coverage`), chunk 2 : chaque ligne ajoutée ou modifiée de `mm`, `ei`, `ga:kj` est exécutée par un TU ; une ligne atteinte seulement par les e2e est justifiée au commit.
- **Mutations**, chacune doit rougir un test, puis être annulée :
  - chunk 1 — **M1** : `pico8_nf_init` remet `g_nf_l` même sur un `nf` prêt → `test_pico8_spec_defaults` rouge (preuve rouge du correctif).
  - chunk 2 — **M2** : la seconde ligne de rotation lit `pv.x` (l'instantané) au lieu du `x` neuf → test ajouté ou `make e2e` rouge ; **M3** : `has_a` ignoré (jamais d'accélération) → TU `ga` (hachage de `kj`) ou test ajouté rouge ; **M4** : `g_:mm` tire `rnd(0.5)` avant `rnd(pn)` → `test_pico8_probe_mm_rnd_order` rouge (sa preuve rouge) ; **M5** : `ga:kj` pose `p_` d'après le premier emplacement → TU `ga` rouge ; **M6** : `ei` décrémente `l` de 0.15 → TU ou `make e2e` rouge.
  - chunk 3 — **M7** : l'`assert` de `pico8_help_init` interroge un nom jamais enregistré → `make test` rouge (le prédicat est bien évalué).
- Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée.

### Fuite par frame (liant)

`make sanitize-net` tourne avec `detect_leaks=0` (TROU-7) : il ne voit pas une fuite qui croît. D.7b retire une allocation par particule et par frame (`p`, et `v` quand `a` est présent) et remplace cinq allocations ou plus par une à chaque `mm` : la fuite doit **baisser**.

- **Binaire** : celui que construit `make sanitize-net` (`tmp/pico8_replay_sanitize`), lancé comme la cible le lance, sur `tests/e2e/message_box_hold.json` et `tests/e2e/game_end_final_score.json` (liants), plus `tests/e2e/tech_geoharvesting_burst.json` (complémentaire, éclat `ga`) :

  ```sh
  ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=exitcode=0:max_leaks=1 \
    ./tmp/pico8_replay_sanitize tests/e2e/<e2e>.json /dev/null "$S/<e2e>.dump.txt" > "$S/<e2e>.leak.log" 2>&1
  grep 'SUMMARY: AddressSanitizer' "$S/<e2e>.leak.log"
  ```
- **Avant** : sur `be2786c` (construit une fois dans une copie du scratchpad) ; **après** : sur l'arbre de chaque commit. Rapporté par e2e : octets et allocations perdus, et octets / frame.
- **Critère** : à chaque commit, octets et allocations perdus **≤ commit précédent** sur les deux e2e liants ; **strictement plus bas** après le chunk 2 (H4 pour le repli) ; chunks 1 et 3 : égaux (aucune allocation touchée), tout écart expliqué ligne à ligne comme une joignabilité d'init changée une seule fois (précédent : `pico8_fv_pq`, correctif (5) de D.7a), jamais comme une fuite par frame. `tech_geoharvesting_burst` : strictement plus bas après le chunk 2.
- **Repli** (LeakSanitizer indisponible) : `ASAN_OPTIONS=detect_leaks=0:print_stats=1`, octets alloués et nombre d'appels, même critère ; dit au commit.

### Pas de nouvel e2e (G)

Les lignes changées sont atteintes par les 26 e2e et par les TU (H11) ; un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 35 minutes pour l'étape** (chunk 1 ≈ 6, chunk 2 ≈ 18, chunk 3 ≈ 8, plus la mesure de départ).

| Cible (par commit) | Durée attendue |
|---|---|
| `make test` (×2) | quelques secondes chacun, plus une recompilation |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min |
| mesure de fuite (2 e2e, 3 au chunk 2) | < 1 min |
| gcov + mutations (chunk 2 ; M1, M7 aux autres) | < 4 min |
| `make data-guard`, `make data-residue`, recette, classification, greps | < 1 min |

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement : `#`, commande exacte, état de l'arbre (HEAD + chunk + étape), code de sortie, ligne clé, durée. Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre — **sauf la mesure de fuite du dernier commit, qu'il relance lui-même**.

**Cibles finales**, vertes sur l'arbre de chaque commit : `timeout 5m make test` ; `timeout 5m make e2e` : **26 « conforme »** ; `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ; `make data-guard` : code 0 ; `make data-residue` + recette : jalons du tableau « Effet » ; greps de la « Sortie » ; fuite ≤ commit précédent, < départ après le chunk 2 ; classification et anti-tassement passés.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit liste les chemins de son chunk (« Empreinte remesurée », écart ≤ 3 expliqué), rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements ; `tools/` et `Makefile` inchangés.
2. **R** : la grep 1 de la « Sortie » rend exactement les 22 lignes de la liste close ; aucune ligne de R n'est modifiée par le diff.
3. **Greps** 2 à 5 de la « Sortie » : exactement ce qui y est écrit.
4. **Allocation** : chaque `mm` fait un seul `malloc` là où il en faisait plusieurs ; `ei` n'alloue plus ; aucune `pico8_ob_table_new` ni vue ajoutée ; aucune conversion typée ↔ ob, à aucun commit.
5. **Comportement** : ordre des tirages de `mm` inchangé (diff) ; commentaire de l'alias `pv`/`p.v` gardé et respecté ; `del` inchangé.
6. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier, recalculés avec la recette ; couples Δlignes > Δoccurrences justifiés ; aucune jonction, alias, registre, déplacement, commentaire réécrit ou supprimé hors de son sujet ; classification (a)/(b)/(c) présente et juste, (c) = 0 ou déclarée ; lignes gagnées listées par fichier.
7. **Fuite** : il relance la mesure du dernier commit et la compare au départ ; lit les valeurs intermédiaires dans les messages.
8. **TU** : touchés aux seules lignes forcées ; constantes `*_HASH` inchangées ; preuves rouges M1 et M4 au commit ; test ajouté du chunk 2 présent.
9. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun récit de chantier ; aucun renvoi à D.7b ni au plan dans `src/` ou `tests/`.
10. **Docs** : glossaire à jour ; note « Découvert à D.7b » en fin de ligne D.7b, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `be2786c` (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas `ob` 552, `overrides` 0, `spec` 0, sans explication.
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »** ou `make sanitize-net` 26 propres, à un des trois commits ; si le diff touche un constructeur et que `make class-coverage-net` n'a pas tourné ou diffère de sa référence.
- `FAILED:` si un champ tombe sous aucune règle (§5), si un test de régression devrait changer d'intention, ou si un écart port/cart est corrigé au lieu d'être préservé (TROU).

**Périmètre**

- `FAILED:` si la grep 1 de la « Sortie » rend autre chose que les 22 lignes de R, ou si une des greps 2 à 5 ne rend pas ce qui y est écrit.
- `FAILED:` si une ligne de R (13 de D.7a, 9 de R_b) est modifiée ou supprimée.
- `FAILED:` si le diff ajoute une `pico8_ob_table_new`, une vue, une conversion typée ↔ ob (même temporaire), une allocation dans `ei`, ou une allocation typée à un site qui n'en faisait pas.
- `FAILED:` si une donnée a deux formes à la fin d'un commit (un producteur ou un lecteur d'enregistrement de `qy` resté ob).
- `FAILED:` si un site pris par D.7a est rouvert hors du TU `test_pico8_spec_defaults`.
- `FAILED:` si le diff touche un TROU autrement que par la disparition du `/* TROU-7 */` de l'allocation retirée de `ei` et le texte de contrat qui entoure ceux de `mm`.

**Gardes, comptes, fuite et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, si `DATA_RESIDUE_GUARD` ou `tools/data_residue.sh` change.
- `FAILED:` si `ob` de `src/` ne vaut pas 147 à la fin, si `ob`, `valeur` ou `type-tag` ne baisse pas strictement sur l'étape, si `stash` ≠ 28, `spec` ≠ 0, `overrides` ≠ 0, ou si `par-nom`, `dispatch`, `ref-audit` bougent sans explication au commit.
- `FAILED:` si la fuite (octets ou allocations perdus) monte d'un commit au suivant sur un e2e liant sans l'explication exigée, ou ne baisse pas strictement après le chunk 2 (hors repli H4 prouvé) ; si la mesure manque à un commit.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés, construction déplacée sur une autre ligne, renommage à type constant, commentaire compté réécrit ou supprimé hors de son sujet, alias, registre de TU, code déplacé hors du compteur ou dans le test ponctuel.
- `FAILED:` si le **message d'un commit** ne donne pas, par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences**, la classification (a)/(b)/(c) des lignes `ob` retirées (avec (c) = 0 ou chaque (c) déclarée et justifiée), la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris), la justification de chaque couple Δlignes > Δoccurrences ; un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` change.
- `FAILED:` si un test est retiré sans être listé au commit avec sa raison, ou si le test ajouté du chunk 2 manque.
- `FAILED:` si le rapport gcov ou une mutation manque, ou si une mutation ne rougit pas sans justification ; si la preuve rouge M1 ou M4 manque.
- `FAILED:` si le budget est dépassé sans note, s'il manque le tableau « Runs », ou si un message ne porte pas sa mesure de fuite.
- `FAILED:` s'il manque la note « Découvert à D.7b », ou si elle omet R = 22 et R_b, le compte `ob` réel, l'effet sur D.7c/D.8, le TU corrompu réparé, ou la fuite avant/après.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Ce que D.7a a pris** : le chaînon `pz`, les ponts, `nf.l`/`nf.r` et `bu`, les lignes suiveuses de `planet.c`, `planet_family_trade.c`, `ui.c` — sauf le TU forcé que D.7a a manqué.
- **R** : les 13 lignes de D.7a et les 9 de R_b (D.8).
- **D.7c** : les fixtures ob des TU que D.7b ne force pas (dont `by` de `test_pico8_ga` l. 578-579, les `fold_table` de `g_`, `fv`, `pj`, `fold_value` de `hp`/`hy`).
- **D.8** : `pico8_ob.c/.h`, `pico8_fi`, `pico8_stash.*`, le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, R.
- **D.9 et suites** : `par-nom` (lectures par nom de `ei`/`ez`/`f_`/`mm`, classe M), `dispatch` (`mm` à arguments `pico8_value_t`, `pico8_mv`), la représentation de `pico8_list_t` (`qy` reste une liste de REF).
- Libérer les particules retirées (TROU-7) ; la phase 2 ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **cible** : `ob` de `src/` hors des deux modules = **R, 22 lignes** (13 de D.7a + 9 commentaires R_b sur la couche encore vivante), pas 0 ; rejetés : réécrire ou supprimer ces commentaires (renommage pour échapper au motif, ou suppression hors sujet).
  - **forme** : un enregistrement typé par particule, vecteurs par valeur, `has_a`, alloué une fois par `mm`, muté en place par `ei` ; rejetés : vecteurs toujours alloués, tag d'en-tête, tableau fixe, libération.
  - **`assert` d'init** : même précondition, lue sur le registre de `bl` ; rejetés : le supprimer, ajouter `pico8_pi_ready()`.
  - **TU de D.7a** : `test_second_jl_keeps_specs` réparé ici, dans son propre commit, avec preuve rouge.
  - **tests** : huit TU aux seules lignes forcées, un test d'`ei` ajouté, sept mutations, deux preuves rouges, mesure LeakSanitizer ; pas d'e2e ; `class-coverage-net` non exigé (aucun constructeur).
  - **découpage** : trois commits (1, 15, 10 chemins).
- **Note du plan (G)**, à ajouter en fin de ligne D.7b (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.7b : trois chunks (`nf.l` dans `test_pico8_spec_defaults`, qu'une table ob écrivait hors du `pico8_nf_button_t` de D.7a ; particules de `qy` ; inclusions et `assert` d'init — N chemins). Formes : `pico8_qy_t` (`p`, `v`, `has_a`/`a`, `l` par valeur ; une allocation par `mm`, mise à jour en place par `ei`, plus d'allocation par frame) ; `pico8_help_init` lit `pico8_pi_dispatch_lookup` ; `slipway.h`, `ui.h`, `wormhole.h` incluent `pico8_list.h`. `pico8_ob.h` n'est plus inclus dans `src/` que par `pico8_ob.c`, `pico8_collect.h`, `pico8_stash.h`. R = 22 : les 13 de D.7a et 9 lignes de commentaire qui décrivent la couche encore vivante (`pico8_type_tag.h` 2, `pico8_ref_audit.h` 4, `pico8_value.h` 1, `pico8_list.h` 1, `pico8_hud.c` 1), que D.8 retire ou réécrit avec `pico8_ob.h` et `PICO8_TYPE_TAG_OB_TABLE` ; le « 187 + 13 » de D.7c devient 187 + 22. `ob` après D.7b : O lignes / O' occurrences (hors test ponctuel : H ; `src/` : 147). Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : 19213 / 59310 → A / B octets (1099 / 1481 → C / D allocations).

  N, O, O', H, A, B, C, D sont les valeurs réelles.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque commit contre ce PRD avant le commit (hors budget de tests) : anti-tassement, classification, lignes gagnées, R, greps.
- **Le message de chaque commit** porte : le périmètre ; la forme et ses rejets ; la classification (a)/(b)/(c) des lignes `ob` retirées ; les comptes avant et après **en lignes et en occurrences** par catégorie et par fichier, avec les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie, par fichier (« aucune » compris) ; les allocations retirées et ajoutées ; la mesure de fuite avant/après ; les tests retirés ou ajoutés ; gcov et mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses H qui le concernent.
- **Surprises pour le pilote** :
  - **D.7b est petite** : 44 lignes `ob` dans 16 fichiers de `src/` (la ligne en annonçait 42 fichiers) ; une seule donnée à typer, la particule.
  - **Un TU de D.7a écrit hors de ses bornes** : `test_second_jl_keeps_specs` pose une clé de table ob dans `g_nf_l` (16 octets) et déborde sur `g_nf_r` ; il passe parce qu'il relit sa propre corruption. Les TU ne sont pas compilés sous ASan : rien ne l'a vu. Une revue de phase pourrait chercher d'autres lectures « comme table ob » d'une donnée typée par D.4a-D.7a (motif : `pico8_ob_table_*` sur un REF lu par nom).
  - **La ligne dit « 0 », le texte des en-têtes ne peut pas tenir 0 avant D.8** : neuf lignes de commentaire décrivent une couche qui vit jusqu'à D.8 ; R passe de 13 à 22.
  - **`ei` fuit une allocation par particule et par frame** (`p`, et `v` sous `a`) : ~140 particules par éclat `ga` ; D.7b la supprime, comme D.7a l'a fait pour `jc.ts`.
