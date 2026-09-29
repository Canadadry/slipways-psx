---
title: "Plan 3, étape D.8 : suppression de `pico8_ob.c/.h` et `pico8_stash.c/.h`, de leurs tests de la couche et du test ponctuel ; les 22 lignes R, la l. 2 de `gh_row_lq` et les trois plis hérités de D.7c ; `ob` = 0 et `stash` = 0 dans `DATA_RESIDUE_GUARD` ; deux commits"
description: "Après D.7c (hypothèses H-D7c), il reste 415 lignes `ob` (470 occurrences) et 28 lignes `stash` : la couche et ses deux tests, le test ponctuel (199), 22 lignes R de `src/`, la l. 2 d'un test du §6 et trois plis de hachage qui plient la valeur d'une énumération de `pico8_ob.h`. D.8 les supprime, garde les octets figés de `hp`/`hy` par une constante locale nommée (précédent `VIEW_INDEX_KEY` de D.7a), retire l'octet jamais atteint de `jc`, garde les tests du fichier du test ponctuel qui ne lisent pas la référence, sans changer une constante `*_HASH` ; la sortie écrite de la phase D n'est pas atteinte : elle attend les décisions humaines de D.9."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données ») :

> - D.8 — suppression : `pico8_ob.c/.h` (avec `pico8_fi`), `pico8_stash.c/.h`, `test_pico8_ob.c` et `test_pico8_stash.c` (§6, tests de la couche), le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, leurs entrées du `Makefile`. Compte : `ob` = 0, `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : non.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » versionnées à `57e2e73`, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b, D.5c, D.6a, D.6b, D.6c, D.7a, D.7b, D.7c ; D.1 n'en a pas). **La note de D.7c est celle du commit `57e2e73`, pas encore gradé** : tout ce que ce PRD en tire est une hypothèse `H-D7c-n` (section « Hypothèses »). La note de D.7b porte la liste R = 22 et les commentaires R_b que D.8 retire ou réécrit.

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

> Découvert à D.7b : trois chunks (`nf.l` dans `test_pico8_spec_defaults`, qu'une table ob écrivait hors du `pico8_nf_button_t` de D.7a ; particules de `qy` ; inclusions et `assert` d'init — 26 chemins). Le TU corrompu : `test_second_jl_keeps_specs` posait une clé de table ob dans `g_nf_l` (16 octets) et débordait sur `g_nf_r` depuis `e252fde` ; il passait en relisant sa propre corruption. Réparé (marque dans `hm`), preuve rouge sous ASan. Un balayage des 81 TU sous `-fsanitize=address,undefined` (copie jetable, `Makefile` inchangé) n'a rien trouvé d'autre ; les TU ne sont compilés sous ASan dans aucune cible. Formes : `pico8_qy_t` (`p`, `v`, `has_a`/`a`, `l` par valeur ; une allocation par `mm`, mise à jour en place par `ei`, plus d'allocation par frame) ; `pico8_help_init` lit `pico8_pi_dispatch_lookup` ; `slipway.h`, `ui.h`, `wormhole.h` incluent `pico8_list.h`. `pico8_ob.h` n'est plus inclus dans `src/` que par `pico8_ob.c`, `pico8_collect.h`, `pico8_stash.h`. R = 22 : les 13 de D.7a et 9 lignes de commentaire qui décrivent la couche encore vivante (`pico8_type_tag.h` 2, `pico8_ref_audit.h` 4, `pico8_value.h` 1, `pico8_list.h` 1, `pico8_hud.c` 1), que D.8 retire ou réécrit avec `pico8_ob.h` et `PICO8_TYPE_TAG_OB_TABLE` ; le « 187 + 13 » de D.7c devient 187 + 22. `ob` après D.7b : 491 lignes / 550 occurrences (hors test ponctuel : 292 ; `src/` : 147 ; TU hors test ponctuel, `test_pico8_ob` et `test_pico8_stash` : 83 lignes dans 28 fichiers, le travail de D.7c). Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : 19213 / 59310 → 19213 / 58910 octets (1099 / 1481 → 1099 / 1431 allocations) ; `message_box_hold` n'exécute ni `ei` ni un `mm` (son `qy` reste vide) ; `tech_geoharvesting_burst` : 240568 → 47328 octets (23752 → 1682 allocations).

> Découvert à D.7c : trois commits (inclusions seules ; valeurs hors rangée, tests fermés et `by` ; plis de hachage — 29 chemins, 28 TU et ce plan, aucun de `src/`). Les cas hors rangée qui offraient une table ob à un membre typé sont retirés quand un frère garde la clause, remplacés par une liste sinon (`ma`/`m_` de `test_jo_writers_stop_off_row`) ; les tests fermés passent une liste locale ; `test_pico8_ga` poussait des tables ob dans `by`, qui tient des routes depuis D.7a : il pousse des routes. Aucune constante `*_HASH` ne change. Arbitrage du pilote : les trois octets de sorte de clé (`fold_names` de `test_pico8_hp`, `fold_qe_map` de `test_pico8_hy`, `fold_hg` de `test_pico8_jc`) ne deviennent pas des littéraux `0u`/`1u` (un identifiant compté remplacé par sa valeur est un alias) : ces trois lignes et les trois inclusions de `pico8_ob.h` qu'elles demandent restent, et D.8 les hérite avec la l. 2 de `test_pico8_planet_gh_row_lq` (R_t : l'énoncé du bug d'un test du §6 nomme `pico8_ob_table_t`), d'où « §6 : `gh_row_lq`, commentaire de la l. 2 seulement, pas de preuve rouge » à D.8. Les octets de `hp` et `hy` sont atteints (les retirer rougit `DEFAULTS_HASH` et `CONSTRUCTION_HASH`) : D.8 doit plier la même valeur sans la couche, arbitrage à reprendre, jamais une constante mise à jour ; celui de `jc` ne l'est pas (aucune constante ne plie un `hg` non vide : sauter toute la boucle de `fold_hg` laisse chaque constante verte). `ob` après D.7c : 415 lignes / 470 occurrences dans 20 fichiers (hors test ponctuel : 216 = 187 + 22 + 1 + 6, les 6 étant ces trois plis et leurs trois inclusions) ; `tests/` ne nomme plus `PICO8_TYPE_TAG_OB_TABLE` ; `type-tag` 373 → 362 lignes (592 → 572 occurrences). Balayage ASan+UBSan jetable des 28 TU touchés : propres.

Puces de la phase D, recopiées (la deuxième fixe le commit où meurt le test ponctuel ; la quatrième est la sortie écrite de la phase) :

> - Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
> - Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
> - Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

Arrêt de la liste et ligne D.9, recopiés (ils disent pourquoi D.8 n'atteint pas la sortie de la phase) :

> **Arrêt « champ sans règle » (§7), constaté à l'écriture de la liste : elle s'arrête à D.8.** Aucune puce de D ne dit ce que deviennent `par-nom` (1081/80) et `dispatch` (197/59). La première et la troisième puce ne couvrent que des tables : `ob` et `stash`, plus, par lecture, `spec` et `overrides`. La **Sortie** nomme des fichiers à supprimer, pas la forme de leurs usagers, et D.0 ne fait que compter. `valeur` (5989/132), `type-tag` (445/71) et `ref-audit` (70/17) ne peuvent tomber qu'avec eux.
> 1. `par-nom` : `pico8_entity_get_field`/`set_field` (132 lignes de `src/`), `*field_truthy(`, les `field`/`set_field` des descripteurs (`pico8_struct_field_fn`, `pico8_struct_set_field_fn`), le `field` des sortes et 191 lignes d'aides `*_take_*`. La puce **Coexistence** de 1b les fait survivre à 1b, et aucune puce ne dit en quoi ils se changent. Les aides à nom non littéral (`field_fixed(inst, name)` et semblables, dans `economy_score.c`, `particle.c`, `probe.c`, `score_panel.c` et `entity_geom.c`) relèvent en plus de la clé dynamique du §5.
> 2. `dispatch` : `pico8_value_function_t methods[PICO8_METHOD_COUNT]` (`pico8_struct.h`), `pico8_mv()`, et les membres fonction (`js.my`/`hb`, `mh.lh`…). Typer les arguments demande une signature par méthode, qu'un tableau indexé par `pico8_method_t` ne tient pas. Or la note de D.0 dit que la table survit à D.
> 3. `pico8_list_t` : la note de D.0 exige d'en changer la représentation sans en fixer la forme. `lq.items` tient des chaînes, des fonctions, nil et le fixe 0 (`layout_nil_item_draws_nothing`), et aucune puce ne type ces éléments hétérogènes.
>
> - D.9 — **étude, sans conversion** (demandée par l'humain à l'arrêt) : pour chacun des trois points ci-dessus, recenser les sites par grep (fichier, fonction, noms lus ou écrits, genres des valeurs), les classer par forme de remplacement possible — accès au membre typé (`p->hp`) à la place de la lecture par nom ; fonction C par méthode, à arguments typés, à la place de `pico8_mv` et de la table `methods[]` ; tableau typé, union étiquetée locale ou struct à la place d'une `pico8_list_t` d'éléments `pico8_value_t` —, et pour chaque classe de sites : la forme retenue, le nombre de sites, ce qui n'entre dans aucune forme (clé dynamique du §5 incluse). Sortie : une section de ce plan qui propose les puces à ajouter à D (ou à reporter en phase 2), avec leur liste d'étapes et leurs comptes ; l'humain tranche avant la première étape qui les applique. D.2 à D.8 ne dépendent pas de cette étude.

§6, recopié :

> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
> - Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés** par ce plan. Depuis la fusion de master (e2e rapide, #32), seuls `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` restent ; les autres ont été remplacés par les e2e de `tests/e2e/`, qui en portent la garde. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.

Règles du §7, recopiées :

> - **Filets depuis la fusion de master (e2e rapide)** : `FRAMES_EVENTS` n'existe plus ; `make e2e` rejoue les 26 e2e de `tests/e2e/` depuis l'écran titre, `make sanitize-net` et `make class-coverage-net` rejouent ce même jeu, `tools/class_coverage_net_expected.txt` y a été reposé à la fusion ; aucun TU ne rejoue de fichier d'événements : un TU construit son état directement, un rejeu est un e2e de `tests/e2e/` (moins de 180 frames, et il doit ajouter de la couverture de lignes). Les mentions de `FRAMES_EVENTS`, `frames-check` et `float-audit` dans les étapes livrées sont historiques.
>
> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.
>
> L'autopilote **s'arrête et demande à l'humain** quand :
>
> - le filet reste rouge après **3 tentatives** de correction sur le même commit ;
> - un champ ne tombe sous aucune règle (§5 : clé dynamique à ensemble non fini ; en phase 2, champ ni archétype ni état, ou groupe au classement ambigu) ;
> - un test de régression de bug devrait être modifié **dans son intention** (pas seulement dans sa façon de lire l'état) pour passer ;
> - la porte de l'étape 0 échoue ;
> - le diff modifie ou supprime un `.sha256` du filet, ou retire un e2e de `tests/e2e/` — `make test` resterait vert.

### Lecture de ces textes pour D.8

- **Périmètre** : la ligne nomme `pico8_ob.c/.h` (avec `pico8_fi`), `pico8_stash.c/.h`, `test_pico8_ob.c`, `test_pico8_stash.c`, le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE` et leurs entrées du `Makefile`. Les notes y ajoutent ce qui meurt avec eux : les 22 lignes R (D.7a : 13 lignes de `pi.c`/`pi.h`/`collect.c`/`collect.h` « servant le test ponctuel, supprimées par D.8 » ; D.7b : 9 commentaires R_b « que D.8 retire ou réécrit avec `pico8_ob.h` et `PICO8_TYPE_TAG_OB_TABLE` »), la l. 2 de `test_pico8_planet_gh_row_lq` (R_t, D.7c) et les trois plis de `hp`/`hy`/`jc` avec leurs trois inclusions de `pico8_ob.h` (D.7c).
- **Compte** : « `ob` = 0, `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD` ». Tout ce qui porte le motif `ob` ou `stash` dans `src/` et `tests/` part ou est réécrit. Aucun reste n'est admis : pas de R, pas de R_t.
- **« §6 : aucun »** : amendé par la note de D.7c en « §6 : `gh_row_lq`, commentaire de la l. 2 seulement, pas de preuve rouge ». Aucune lecture du test ne change ; son intention (le bug qu'il garde) reste énoncée. Précédent : D.6a, même ligne.
- **« `class-coverage-net` : non »** : aucun constructeur d'entité, aucun `pico8_class_coverage_record`, aucun `KNOWN[]` ne change. Mais `src/` change, un `malloc` part (`pico8_pi_vec_v_constructor`) et les allocations de `pico8_ob.c`/`pico8_stash.c` disparaissent : `make sanitize-net` est exigé (§7, « En D et en 2 », « `malloc`/`free` retiré »), et `make class-coverage-net` passe en confirmation, une fois, au dernier commit (consigne du pilote : `src/` change ; `pico8_pi_init`, qu'appellent les `*_init` des classes, change).
- **Le test ponctuel** : la deuxième puce le définit par ce qu'il compare (« chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit ») et le fait supprimer « dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` ». Son fichier, `tests/test_pico8_map_tables.c`, porte aussi, depuis D.3-D.5c, des tests des modules typés qui ne lisent pas la référence (décision plus bas).
- **Sortie de la phase** : la quatrième puce veut `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés. D.8 en supprime deux sur cinq. Les trois autres tombent avec `valeur`, `type-tag`, `ref-audit`, que la liste de D ne couvre pas (« arrêt “champ sans règle” ») ; l'étude D.9 (`docs/prd/next/plan3-phaseD-etape9-etude.md`, **non tranchée par l'humain**) propose D.10-D.12 pour `dispatch`, `par-nom` et `pico8_list_t`, et annonce un quatrième point (`valeur`, `type-tag`, `ref-audit`, sa Q12). **D.8 est la dernière étape écrite de la liste : après sa gradation, `plan-pilot` s'arrête et attend l'humain** ; la revue de fin de phase (`warn-review`, filets de sortie de phase) n'est pas déclenchée par D.8.
- **Leçons des notes à ne pas refaire** : classification (a)/(b)/(c) absente ou fausse, (c) non déclarée, lignes qui gagnent une catégorie omises (correctifs de D.7a) ; une constante changée pour faire passer un pli (interdit par toute la phase).
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; aucune jonction de lignes, aucun alias, aucun registre, aucun renommage pour échapper aux greps ; jamais l'appli raylib.

### État de départ

**Mesuré sur `57e2e73`** (branche `claude/plan-pilot-3-7zdft0`, D.7c commitée en `24f697c`, `542bf1d`, `57e2e73`, **en cours de vérification, non gradée**), dans une copie `git archive 57e2e73` du scratchpad. **Rien n'a été compilé ni lancé** : seuls `sh tools/data_residue.sh` (avec `--detail`), la recette d'occurrences de D.7c, des `grep` et un parcours du graphe d'inclusion (`#include "…"`, récursif) ont tourné.

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 5125 | 131 | 5931 |
| `ob` | 415 | 20 | 470 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 362 | 67 | 572 |
| `ref-audit` | 65 | 16 | 66 |
| `par-nom` | 1009 | 81 | 1064 |
| `dispatch` | 191 | 61 | 193 |
| `overrides` | 0 | 0 | 0 |
| `spec` | 0 | 0 | 0 |
| **résidu D** | **6440 lignes** | **135 fichiers** | — |

`DATA_RESIDUE_GUARD` = `spec,overrides`.

### Inventaire de `ob` et `stash` (mesuré sur `57e2e73`, lignes / occurrences)

| Groupe | Fichier | `ob` | `stash` | Autres catégories sur ses lignes (fichier entier pour un fichier supprimé) |
|---|---|---|---|---|
| couche | `src/pico8_ob.c` | 82 / 104 | — | `valeur` 17/18, `type-tag` 1/1, `ref-audit` 7/7 |
| couche | `src/pico8_ob.h` | 32 / 46 | — | `valeur` 11/14, `type-tag` 3/4 |
| couche | `src/pico8_stash.c` | 8 / 8 | 7 / 7 | — |
| couche | `src/pico8_stash.h` | 3 / 3 | 7 / 7 | — |
| test de la couche | `tests/test_pico8_ob.c` | 55 / 65 | — | `valeur` 23/27 |
| test de la couche | `tests/test_pico8_stash.c` | 7 / 7 | 12 / 12 | `valeur` 3/3 |
| test ponctuel | `tests/test_pico8_map_tables.c` | 199 / 206 | 2 / 2 | `valeur` 97/103, `par-nom` 1/1 (`kind->field`, gardé) |
| R (D.7a) | `src/pico8_collect.c` l. 127-128, 137, 153, 156-157, 168 | 7 / 7 | — | `pico8_ho_apply` et l'enregistrement `'b'` : `valeur` 10/14 sur l. 127-169 |
| R (D.7a) | `src/pico8_collect.h` l. 7, 51-52 | 3 / 3 | — | — |
| R (D.7a) | `src/pico8_pi.c` l. 220, 222 | 2 / 2 | — | constructeurs `'v'`/`'s'` morts avec l'enregistrement : `valeur` 6/14 |
| R (D.7a) | `src/pico8_pi.h` l. 11 | 1 / 2 | — | — |
| R_b (D.7b) | `src/pico8_type_tag.h` l. 8, 15 | 2 / 3 | — | commentaire ; plus l. 39 (`#define … OB_TABLE`) et l. 59 (`pico8_type_tag_of`), `type-tag` |
| R_b (D.7b) | `src/pico8_ref_audit.h` l. 12, 14, 18, 22 | 4 / 4 | — | paragraphe l. 8-26 : `valeur` 2/2, `type-tag` 1/1 ; plus l. 163 (`case`), 226 et 249 (`PICO8_REF_AUDIT_EXPECT_OB_TABLE`) |
| R_b (D.7b) | `src/pico8_value.h` l. 7 | 1 / 1 | — | — |
| R_b (D.7b) | `src/pico8_list.h` l. 13 | 1 / 1 | — | — |
| R_b (D.7b) | `src/pico8_hud.c` l. 8 | 1 / 1 | — | — |
| R_t (D.7c) | `tests/test_pico8_planet_gh_row_lq.c` l. 2 | 1 / 1 | — | test du §6 |
| plis (D.7c) | `tests/test_pico8_hp.c` l. 32 (inclusion), 180 (`fold_names`) | 2 / 2 | — | atteint : `DEFAULTS_HASH` |
| plis (D.7c) | `tests/test_pico8_hy.c` l. 33, 173 (`fold_qe_map`) | 2 / 2 | — | atteint : constantes qui plient `po`/`fu`, dont `CONSTRUCTION_HASH` |
| plis (D.7c) | `tests/test_pico8_jc.c` l. 29, 266 (`fold_hg`) | 2 / 2 | — | **non atteint** (note de D.7c) |
| **Total** | 20 fichiers | **415 / 470** | **28 / 28** | hors test ponctuel : 216 = 187 + 22 + 1 + 6 |

**Graphe d'inclusion.** `pico8_ob.h` n'est inclus que par `pico8_ob.c`, `pico8_collect.h`, `pico8_stash.h`, `test_pico8_ob.c`, `test_pico8_hp.c`, `test_pico8_hy.c`, `test_pico8_jc.c` ; `pico8_stash.h` par `pico8_stash.c`, `test_pico8_stash.c` et le test ponctuel. `pico8_collect.h` est inclus par 24 fichiers. Parcours récursif, `pico8_ob.h` et `pico8_stash.h` retirés : **aucun fichier de `src/` ou `tests/` ne perd `pico8_type_tag.h`, `pico8_value.h` ni `pico8_list.h` alors qu'il en nomme un symbole** (H1) ; `pico8_ob.h` n'apporte rien d'autre. Aucune inclusion à ajouter.

**Appelants.** Hors de la couche, de ses tests et du test ponctuel, aucun appel de `pico8_ob*()`, `pico8_fi`, `pico8_unstash`, `pico8_stash_*`, `pico8_r()`/`pico8_iz()` ; `pico8_ho_apply` n'a que les deux appels du test ponctuel (l. 330, 597) ; `pico8_ob_register_constructor` n'a que les trois enregistrements de `pi_init` (`'b'` par `pico8_qz_register_constructor`, `'v'`, `'s'`), jamais lus hors de `pico8_ob.c`. `pico8_stash_init` n'est appelé par aucun module du jeu : le binaire de rejeu ne décode plus rien depuis D.2. **Le jeu n'exécute donc aucune ligne de ce qui part**, hors les trois enregistrements (écriture d'un tableau statique jamais relu).

**`PICO8_TYPE_TAG_OB_TABLE`** : `pico8_type_tag.h` l. 39 (définition) et l. 59 (`pico8_type_tag_of` le reconnaît), `pico8_ref_audit.h` l. 163 (`case` de l'audit par site, avec le compteur `n_ob_table` l. 66, 150, 164 et la colonne `ob_table=` l. 116-117), l. 226 et 249 (`PICO8_REF_AUDIT_EXPECT_OB_TABLE`, appelée seulement par `pico8_ob.c`), `pico8_ob.c`, `pico8_ob.h`, `test_pico8_ob.c`, le test ponctuel. `tests/` hors de ces deux-là ne le nomme plus (D.7c). Deux TU gardent sa **valeur** comme octet figé de hachage, nommée localement depuis D.7a : `VIEW_TAG = 0x9E5C0B7Au` dans `test_pico8_hp.c` l. 197 et `test_pico8_lq.c` l. 138.

**Les trois plis hérités** (texte à `57e2e73`) :

- `test_pico8_hp.c` l. 172-183, `fold_names` : « `REF`, the count, then per name the index-key kind, its position and the string: the format DEFAULTS_HASH froze. » ; l. 180 `h = fnv(h, (uint32_t)PICO8_OB_KEY_INDEX);`. Le même TU déclare déjà, l. 195-197, **`enum { VIEW_TAG = 0x9E5C0B7Au, VIEW_INDEX_KEY = 0, VIEW_NAME_KEY = 1, VIEW_LIST_TAG = 0x9E5C1157u };`** sous le commentaire « The frozen hashes were taken when rows were ob views: these are the bytes a view folded for its tag and its two kinds of key. » (fa997e7, D.7a).
- `test_pico8_hy.c` l. 164-176, `fold_qe_map` : « `REF`, the count, then per entry the index-key kind, `qe` and the value: the format the constants froze. » ; l. 173 `h = fnv(h, (uint32_t)PICO8_OB_KEY_INDEX);`.
- `test_pico8_jc.c` l. 256-270, `fold_hg` : « Each `id` as the table `hg[id]=true` held it: a named key, true. » ; l. 266 `h = fnv(h, (uint32_t)PICO8_OB_KEY_NAME);`.
- `pico8_ob.h` l. 11-14 : `typedef enum { PICO8_OB_KEY_INDEX, PICO8_OB_KEY_NAME } pico8_ob_key_kind_t;` — valeurs 0 et 1.

**Mentions non comptées d'un type qui disparaît** (hors motif, relevées pour décider) : `src/pico8_entity.h` l. 44 et `src/pico8_entity.c` l. 173, 189 (message d'`assert` : « an ob table or list REF ») ; `src/pico8_pi.c` l. 23 (« ob()'s five-value constructor ») ; `src/pico8_pi.h` l. 41-45 (`pico8_pi_init` « Must run before ob() parses a spec ») ; `tests/test_pico8_hp.c` l. 130-131 et `tests/test_pico8_hy.c` l. 131-132 (`fold_value` : « a list or an ob_table folded through `depth` levels », branche retirée par D.7c) ; `tests/test_pico8_planet_gh_row_lq.c` l. 3 (« this port has two ») et l. 136 ; `tests/test_pico8_ga_export_panel_by.c` l. 2-3 (énoncé du bug). `docs/trous-de-portage.md` : TROU-4 (fichiers `pico8_collect.c` `pico8_oo`, supprimé à D.7a, et `pico8_ob.c`) et TROU-9 (`pico8_ob.c`). `docs/glossaire-noms.md` l. 56-57, 261, 263, 653.

## Solution

**Deux commits**, chacun vert seul :

1. **Octet non atteint de `jc`** — `fold_hg` cesse de plier l'octet de sorte de clé ; `test_pico8_jc.c` perd son inclusion de `pico8_ob.h`. Aucune constante ne bouge (preuve que l'octet n'était pas atteint, isolée des autres).
2. **Suppression** — `pico8_ob.c/.h`, `pico8_stash.c/.h`, `test_pico8_ob.c`, `test_pico8_stash.c` supprimés ; le test ponctuel retiré de `test_pico8_map_tables.c` (le fichier garde les tests des modules typés qui ne lisent pas la référence) ; R : `pico8_ho_apply`, les enregistrements et constructeurs `'b'`/`'v'`/`'s'` et leurs contrats, `PICO8_TYPE_TAG_OB_TABLE` et ses usages, les commentaires R_b ; les octets de `hp`/`hy` pliés par une constante locale nommée, inclusions retirées ; l. 2-3 de `gh_row_lq` ; `Makefile` ; `DATA_RESIDUE_GUARD` = `spec,overrides,stash,ob` ; `docs/trous-de-portage.md`, `docs/glossaire-noms.md` ; note « Découvert à D.8 ».

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (dont `make data-guard` : code 0, `résidu gardé (spec,overrides,stash,ob) : 0 ligne(s)`) ; `make e2e` : **26 conformes** ; `make sanitize-net` propre ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ;
- `make data-residue` : **plus de ligne `ob` ni `stash`** ; `type-tag` ≈ 354 lignes / 562 occurrences, `ref-audit` = 56 / 57, `valeur` ≈ 4961 / 5742 (tableau « Effet ») ; `par-nom` 1009, `dispatch` 191, `overrides` 0, `spec` 0 inchangés ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/` ni de `tools/` touché ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  # 1. ob et stash dans src/ et tests/ : vide (sortie 1 de grep)
  . "$S/pat.sh"; grep -rnE "$(pattern ob)|$(pattern stash)" src tests --include='*.[ch]'
  # 2. fichiers supprimés : aucun ne subsiste
  ls src/pico8_ob.* src/pico8_stash.* tests/test_pico8_ob.c tests/test_pico8_stash.c 2>/dev/null
  # 3. le tag supprimé, et toute mention qui s'en sert : vide
  grep -rnE 'OB_TABLE|n_ob_table|ob_table=' src tests --include='*.[ch]'
  # 4. le Makefile ne nomme plus la couche : vide
  grep -nE 'pico8_(ob|stash)\.c|test_pico8_(ob|stash)\b' Makefile
  # 5. la garde
  grep -n '^DATA_RESIDUE_GUARD' Makefile     # DATA_RESIDUE_GUARD := spec,overrides,stash,ob
  # 6. aucune constante de hachage ne change
  git diff 57e2e73 HEAD -- tests | grep -E '^[-+].*(_HASH|identity\[)' # vide
  ```

## User Stories

1. En tant que mainteneur du port, je veux que `pico8_ob.c/.h` et `pico8_stash.c/.h` n'existent plus, afin que le port n'ait plus d'analyseur de spec ni de décodeur de chaîne en mémoire, que seules les données générées de D.2-D.7a servent.
2. En tant que mainteneur du port, je veux que `test_pico8_ob.c` et `test_pico8_stash.c` partent dans le commit qui supprime le code qu'ils testent, afin de suivre le §6.
3. En tant que mainteneur du port, je veux que la comparaison à la référence décodée ou parsée (le test ponctuel) parte dans ce même commit, afin de suivre la deuxième puce de la phase D.
4. En tant que gardien des TU, je veux que les tests de `test_pico8_map_tables.c` qui ne lisent pas la référence restent, afin que `pico8_gn`, `pico8_qe_map_set`/`_get`, `pico8_int_seq_has_fixed`, les bornes de `pico8_int_seq_at`/`pico8_fixed_seq_at`, les échecs des recherches `pico8_r_lookup`/`pico8_iz_lookup`/`pico8_hc_lookup`/`pico8_df_lookup`, les sortes éteintes et les chaînes distinctes de l'arbre `jo` (TROU-3) gardent leur seul TU.
5. En tant que gardien des TU, je veux qu'aucune assertion nouvelle ne soit écrite dans ce fichier, seulement les assertions existantes qui ne lisent pas la référence, reformulées sur la table typée, afin que le fichier ne devienne pas un nouveau chantier.
6. En tant que gardien du compteur, je veux `ob` = 0 et `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD` dans le commit qui les éteint, afin qu'aucune ligne de ces catégories ne revienne.
7. En tant que gardien du compteur, je veux que les 22 lignes R partent ou soient réécrites avec la couche, sans reste, afin que la ligne dise vrai (« `ob` = 0 »).
8. En tant que gardien des constantes, je veux que `DEFAULTS_HASH` de `hp` et les constantes de `hy` qui plient `po`/`fu` passent inchangées sans `pico8_ob.h`, afin que la suppression ne soit pas payée par une constante mise à jour.
9. En tant que gardien des constantes, je veux que l'octet figé de `hp` et de `hy` soit plié par une constante locale nommée, avec un commentaire qui dit que c'est l'octet gelé d'une énumération supprimée, afin qu'on ne le change pas en croyant qu'il est libre.
10. En tant que gardien du compteur, je veux que ces deux lignes soient déclarées (c) au message avec la preuve que l'énumération qu'elles nommaient est supprimée dans le même commit, afin qu'elles ne passent pas pour un renommage qui échappe au motif.
11. En tant que gardien des constantes, je veux que l'octet non atteint de `jc` parte dans un commit à lui, constantes inchangées, afin que « non atteint » soit prouvé par la suite et non affirmé.
12. En tant que gardien du §6, je veux que `gh_row_lq` ne change qu'aux l. 2-3, pour ne plus présenter comme vivant un type supprimé, corps et lectures inchangés, afin que son intention (le bug qu'il garde) reste énoncée.
13. En tant que gardien du §6, je veux qu'aucune preuve rouge ne soit exigée pour ce commentaire, et que le PRD le dise, afin que le vérificateur ne cherche pas une réécriture de lecture qui n'existe pas.
14. En tant que mainteneur du port, je veux que `PICO8_TYPE_TAG_OB_TABLE` disparaisse de `pico8_type_tag.h` et de `pico8_ref_audit.h`, avec le compteur `n_ob_table` et la colonne `ob_table=` de l'audit, afin que le tag ne décrive plus que des formes qui existent.
15. En tant que lecteur des en-têtes, je veux que les commentaires qui présentent la table ob comme une forme vivante (`pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_value.h`, `pico8_list.h`, `pico8_hud.c`, `pico8_pi.h`, `pico8_pi.c`) et les messages d'`assert` d'`entity.c`/`.h` ne la nomment plus, afin qu'ils disent le contrat actuel.
16. En tant que mainteneur du `Makefile`, je veux que `pico8_ob.c` et `pico8_stash.c` quittent `ENGINE_SRCS`, `DATA_ENGINE_SRCS`, la ligne de lien de `test_pico8_layout_nil_item_draws_nothing` et celle de `main_raylib`, et que `test_pico8_ob`/`test_pico8_stash` quittent `UNIT_TESTS` avec leurs règles, afin que rien ne compile plus la couche.
17. En tant que gardien des filets, je veux `make test` et `make e2e` verts à chaque commit, `make sanitize-net` propre et `make class-coverage-net` identique au dernier, afin de prouver que le jeu ne dépendait plus de la couche.
18. En tant que gardien du compteur, je veux, au message de chaque commit, les comptes avant/après en lignes ET en occurrences, par catégorie et par fichier, la classification (a)/(b)/(c) de chaque ligne `ob`/`stash` retirée, la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris), et la justification de chaque couple Δlignes > Δoccurrences, afin qu'aucune baisse ne soit un tassement.
19. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, aucun alias, registre, macro ni renommage ajouté pour échapper aux greps, hors les deux (c) déclarées, afin que « 0 » veuille dire « supprimé ».
20. En tant que gardien de `docs/trous-de-portage.md`, je veux que TROU-4 et TROU-9, dont tous les sites disparaissent, gardent leur numéro et disent qu'ils n'ont plus de site, afin que la numérotation reste stable et que l'écart du cart reste documenté.
21. En tant que lecteur du glossaire, je veux que les lignes `r`, `iz`, `ob(qs,mr)`, `pi` et « Emprunt de pointeur » ne renvoient plus à des identifiants supprimés, afin que le glossaire dise où vit chaque nom.
22. En tant que `plan-pilot`, je veux que le PRD dise ce que D.8 atteint et ce qu'il n'atteint pas de la sortie de la phase, afin de s'arrêter après la gradation et d'attendre les décisions humaines de D.9.
23. En tant que `plan-pilot`, je veux les hypothèses sur l'issue de D.7c étiquetées `H-D7c-n` et la liste des sections à revérifier, afin de comparer ce PRD à la note gradée de D.7c avant de le passer en triage.
24. En tant que lecteur du plan, je veux une note « Découvert à D.8 » qui donne les comptes réels, le choix sur les octets figés, le sort du fichier du test ponctuel et ce qui reste de la sortie de la phase, afin que la suite parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Modules

- **Supprimés** : la couche ob (analyseur de spec, table ordonnée, registre de constructeurs, `pico8_fi`), le module stash (décodage `unstash`, tables `r`/`iz` en ob), leurs deux tests de la couche.
- **`collect`** : perd `pico8_ho_apply` (la passe de renommage du cart sur une table ob, qui ne sert plus que le test ponctuel) et l'enregistrement du constructeur `'b'` (`pico8_qz_constructor`, `pico8_qz_register_constructor` et son contrat). `pico8_ho_gp_has` reste (lu par `hp`). Les inclusions que seules ces fonctions demandaient (`pico8_ob.h` dans l'en-tête ; `pico8_map_tables.h` dans le `.c` si `grep` montre que `pico8_r_lookup` n'y a plus d'appelant) partent.
- **`pi`** : `pico8_pi_init` n'enregistre plus que les fonctions de dessin de `bl()` ; les constructeurs `'v'` et `'s'` (`pico8_pi_vec_v_constructor`, `pico8_pi_spr_constructor`) partent avec leur enregistrement ; `pico8_pi_spr_call` reste (lu par `pico8_pi_spr_draw`). Le contrat de `pico8_pi_init` et le commentaire d'en-tête (« two consumers needing two lookups ») sont ramenés à un seul registre. L'inclusion de `pico8_collect.h` dans `pi.c` part si `grep` montre qu'aucun autre symbole de `collect` n'y est lu.
- **`type_tag`** : le tag ne distingue plus que liste, struct d'entité et inconnu ; `pico8_type_tag_of` ne reconnaît plus la valeur supprimée (une donnée qui la porterait lit « inconnu », et plus aucun objet ne la porte).
- **`ref_audit`** : l'audit par site ne compte plus les tables ob (`case`, compteur, colonne du rapport) ; `PICO8_REF_AUDIT_EXPECT_OB_TABLE` part dans ses deux formes ; le paragraphe d'ouverture, qui justifiait l'audit par la lecture d'une table ob comme liste, est ramené à ce que l'en-tête fait encore (conversion contrôlée d'un REF en liste, et sa mesure).
- **Aucun module profond à extraire** : l'étape retire du code, elle n'en crée pas.

### Le fichier du test ponctuel (G)

La deuxième puce de la phase D définit le test ponctuel par sa comparaison à la référence ; D.3-D.5c ont logé dans le même fichier des tests de modules typés qui ne lisent aucune référence.

- **Retenu** : `tests/test_pico8_map_tables.c` **reste**, réduit aux tests qui ne lisent pas la référence. Partent : `decode`, `parse`, toutes les chaînes `*_SRC`, les comparateurs (`tables_equal`, `values_equal`, `row_equal`, `named_*`, `check_seq`, `check_names`, `check_row`, `member_of`, `chain_lookup`, `check_kind`…), les vues de comparaison (`df_entry_view`, `nb_reference`, `df_reference`), toute comparaison entrée par entrée, `pico8_load_map_json` et `pico8_pi_init` de `main` (ils ne servaient que la référence). Restent **tels quels** : `test_gn_pools_df_entries_by_their_r` (avec `never_drawn`), `test_int_seq_has_fixed_keeps_gu_on_integers`, `test_jo_tree_strings_are_distinct_objects`, `test_kinds_answer_nothing_once_down` (avec `kind_answer` et les trois `*_init` qui le précèdent), `test_nb_lookup_finds_each_px`, `test_qe_map_keeps_one_entry_per_key_in_order`. Restent **reformulées sur la table typée**, sans assertion nouvelle : les assertions de bornes (`pico8_int_seq_at`/`pico8_fixed_seq_at` rendent chaque position `1..count` et refusent `0` et `count + 1` sans toucher la sortie ; mêmes suites qu'à `57e2e73` : `es`, `b_`, `co`, `fb`, `ek`, `ct`, `ci`, et les suites que `check_seq` passait à `check_int_seq_at_bounds`) et les recherches (`pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_df_lookup` trouvent chaque entrée de leur table typée ; « zz » et, là où l'assertion existait, `NULL` ne trouvent rien), sur le modèle de `test_nb_lookup_finds_each_px`. Le nom du fichier et les noms des tests gardés ne changent pas ; les tests reformulés prennent un nom qui dit ce qu'ils vérifient (leur nom de `57e2e73` dit « matches the decoded table », qui n'est plus vrai). La règle de lien du `Makefile` reste (`DATA_ENGINE_SRCS`, `pico8_planet_family_trade.c`) et le fichier reste dans `UNIT_TESTS`.
- **Rejeté : supprimer le fichier entier** — lecture littérale de la ligne ; mais `pico8_gn`, `pico8_qe_map_set`, `pico8_int_seq_has_fixed`, `pico8_fixed_seq_at`, les échecs de `pico8_iz_lookup`/`pico8_r_lookup`, les sortes éteintes et les chaînes distinctes de TROU-3 n'ont aucun autre TU (grep à `57e2e73`) : ce serait retirer des gardes que la puce ne vise pas.
- **Rejeté : déplacer ces tests dans d'autres TU** (`test_pico8_hy`, `test_pico8_js`, `test_pico8_entity_kind`…) — six à huit fichiers de plus dans le commit, et le même code.
- **Rejeté : écrire les tests de recherche et de bornes qui manqueraient** — hors ligne ; seules les assertions existantes survivent.

### Les octets figés de `hp` et `hy` (G)

Depuis D.4b, `fold_names` (`hp`) et `fold_qe_map` (`hy`) plient une donnée typée « comme `fold_value` pliait la table » : l'octet de sorte de clé fait partie du format figé de `DEFAULTS_HASH` et des constantes de `hy` qui plient `po`/`fu` (note de D.7c : les retirer rougit `DEFAULTS_HASH` et `CONSTRUCTION_HASH`). Cet octet vient de la valeur (0) de `PICO8_OB_KEY_INDEX`, que D.8 supprime. L'arbitrage de D.7c (« un identifiant compté remplacé par sa valeur est un alias ») valait **tant que l'énumération vivait** : l'octet restait lié à sa source. À D.8 la source disparaît dans le même commit ; il n'y a plus d'identifiant à qui échapper, il reste un octet gelé à nommer.

- **Retenu : option 1, constante locale nommée, même valeur, commentaire de contrat.**
  - `hp` : **aucun identifiant nouveau**. Le TU a déjà, depuis D.7a, `VIEW_INDEX_KEY = 0`, l'octet qu'une vue ob pliait pour une clé positionnelle — le même octet, de la même énumération. La l. 180 plie `VIEW_INDEX_KEY` ; la déclaration `enum { VIEW_TAG, VIEW_INDEX_KEY, VIEW_NAME_KEY, VIEW_LIST_TAG }` et son commentaire remontent au-dessus de `fold_names` (déplacement d'une déclaration de TU, aucune catégorie), et le commentaire est élargi aux deux usages : ce sont les octets gelés que pliaient une vue ob et une table ob (étiquette, sortes de clé), d'une énumération et d'un tag supprimés ; en changer un rougit toute constante qui les plie.
  - `hy` : **une constante locale**, `enum { TABLE_INDEX_KEY = 0 };`, juste au-dessus de `fold_qe_map`, sous un commentaire de contrat de même sens (l'octet qu'une table ob pliait avant une clé positionnelle, gelé dans les constantes ; l'énumération qui le donnait est supprimée ; le changer rougit les constantes qui plient `po`/`fu`). La l. 173 plie `TABLE_INDEX_KEY`. Le nom ne correspond à aucun motif du compteur et ne reproduit pas le nom supprimé.
  - Pourquoi ce n'est pas l'alias interdit : (1) l'identifiant compté **n'existe plus** après le commit — ce n'est pas lui qu'on renomme, c'est un octet dont la source est supprimée qu'on nomme ; (2) le précédent est dans le dépôt, gradé : `VIEW_TAG = 0x9E5C0B7Au` (valeur de `PICO8_TYPE_TAG_OB_TABLE`) et `VIEW_INDEX_KEY`/`VIEW_NAME_KEY` dans `test_pico8_hp.c` et `test_pico8_lq.c` (fa997e7, D.7a) ; (3) les constantes inchangées prouvent que le même octet est plié ; (4) chaque ligne est **déclarée (c)** au message, avec ces trois raisons.
- **Rejeté : option 2, reprendre les constantes** (retirer l'octet, recalculer `DEFAULTS_HASH` et les constantes de `hy` qui plient `po`/`fu`, avec une sonde qui montre que seul cet octet a bougé). Toute la phase D fige les constantes (« arrêt sur la constante, jamais de mise à jour », D.7a-D.7c) ; une constante reprise ne prouve plus l'identité de l'état, et une sonde ne prouverait que ce qu'elle mesure ; enfin le précédent de l'option 1 existe.
- **Rejeté : le littéral `0u`** — même octet, mais sans nom ni contrat : l'arbitrage de D.7c l'a écarté, et un littéral nu dans un pli invite à le « corriger ».
- **Rejeté : `VIEW_INDEX_KEY` dans `hy`** — `po`/`fu` étaient des tables ob, pas des vues ; recopier le nom d'un autre TU n'apporte rien.

### L'octet non atteint de `jc` (G)

`fold_hg` plie `PICO8_OB_KEY_NAME` (1) avant chaque `id` ; la note de D.7c : « aucune constante ne plie un `hg` non vide : sauter toute la boucle de `fold_hg` laisse chaque constante verte ». La ligne part (a) « branche de pli non atteinte » ; le commentaire « as the table `hg[id]=true` held it: a named key, true » est ramené au format qui reste (le compte, puis chaque `id` et `true`) ; l'inclusion de `pico8_ob.h` part (a). **Preuve : chaque constante de `test_pico8_jc` passe inchangée** au commit 1. Rejeté : une constante locale comme pour `hp`/`hy` — nommer un octet qu'aucune constante ne gèle serait inventer un format à garder.

### `gh_row_lq`, l. 2-3 (G)

Texte à `57e2e73` : « A display row's "lq" holds a `pico8_list_t*`, whose elements `js:pq()` / takes as the row's positionals; the bug read it as a `pico8_ob_table_t*`. / Lua has one table type and this port has two: before the type tag the … ». Après D.8, le type nommé n'existe plus et « this port has two » est faux. Réécriture **limitée à ce que la suppression force** : la l. 2 dit que le bug lisait la liste comme la table d'`ob()` du port, type depuis supprimé ; la l. 3 passe « has two » au passé. Même intention (le bug gardé), aucune lecture ni assertion ne change : **pas de preuve rouge** (précédent D.6a sur la même ligne ; note de D.7c). (a) « commentaire d'une donnée qui n'existe plus ». La l. 136 (« whose "lq" really is an ob_table ») et l'en-tête de `ga_export_panel_by` (l. 2-3, énoncé du bug par `ob()`) ne sont pas comptés et ne nomment pas le type par son identifiant : **non touchés** (le §6 interdit d'affaiblir un énoncé de bug ; une passe de commentaires ultérieure les reprendra), listés au message.

### `PICO8_TYPE_TAG_OB_TABLE` et l'audit (G)

- Définition (l. 39) et reconnaissance dans `pico8_type_tag_of` (l. 59) retirées (a). Le commentaire d'ouverture de `pico8_type_tag.h` (l. 5-20 : formes étiquetées, pas de stride commun, « four separate heap-buffer-overflows ») est ramené aux formes qui restent (liste, struct d'entité, inconnu) ; l'historique des débordements va au message. Le paragraphe « CHOICE OF MAGIC VALUES » reste (il vaut pour `LIST` et `STRUCT`).
- `pico8_ref_audit.h` : `case`, compteur `n_ob_table` (champ, remise à zéro, colonne `ob_table=%lu` du `SITE`) et `PICO8_REF_AUDIT_EXPECT_OB_TABLE` (deux définitions) retirés (a). `tools/ref_audit_report.sh` lit les colonnes par nom et compte 0 une colonne absente : **inchangé** (hors suite, hors périmètre ; son mot `ob_table` devient mort, listé). Rejeté : garder la colonne à 0 (un compteur qu'aucun code n'incrémente) ; rejeté : réécrire l'outil (hors ligne, hors suite).

### Commentaires R_b et mentions (G)

Règle : un commentaire dont le sujet est supprimé par ce commit est retiré ou ramené à ce qui reste (a) ; aucun commentaire dont le sujet existe encore n'est touché. `pico8_value.h` l. 6-8 (la liste des emplacements de valeur perd « an ob() table entry ») ; `pico8_list.h` l. 12-14 (la phrase « the ob() table … is the other shape » part) ; `pico8_hud.c` l. 6-8 (la justification « rather than a pico8_ob() spec » part : toutes les données sont des tableaux C) ; `pico8_pi.h` l. 10-12 et 41-45, `pico8_pi.c` l. 21-23 ; messages d'`assert` de `pico8_entity.c` l. 173, 189 et commentaire de `pico8_entity.h` l. 44-45 (« an ob table or a list REF » → une liste). `test_pico8_hp.c` l. 130-131 et `test_pico8_hy.c` l. 131-132 (« a list or an ob_table » dans le contrat de `fold_value`, branche retirée par D.7c) : ramenés à « a list », fichiers déjà touchés. Les mentions d'`ob()` comme fonction du cart (provenance des données générées : `pico8_game_data.h` l. 49, 555, `pico8_map_tables.h` l. 35 ; TROU-3 du test des chaînes distinctes) **restent** : le cart garde son `ob()`.

### Documentation (G)

- `docs/trous-de-portage.md` : TROU-4 (`oo()` et le constructeur `s`) et TROU-9 (pas de suppression de champ nommé) n'ont plus de site : chacun garde son numéro et son texte, et gagne une ligne « Sans site depuis D.8 : … supprimé » à la place de sa ligne « Fichier(s) ». Les marques `/* TROU-4 */` (`pi.c` l. 221, `pico8_ob.c` l. 268) partent avec leurs lignes ; `/* TROU-7 */` de `pi.c` l. 154 part avec `pico8_pi_vec_v_constructor` (TROU-7 garde ses autres sites). Rejeté : supprimer les entrées (numérotation cassée, écart du cart oublié).
- `docs/glossaire-noms.md` : `r`, `iz` (sans `pico8_r`/`pico8_iz`/`pico8_stash_init`), `ob(qs,mr)` (plus d'équivalent : données générées), `pi` (sans `pico8_ob_register_constructor` ni la table `b`/`v`/`s`), « Emprunt de pointeur » (sans `pico8_ob_table_free`).
- `docs/plan.md`, `docs/plan-2.md`, `docs/audit-ref-cast-reverse.md`, les PRD : historiques, **non touchés**.

### `Makefile` (G)

Retirés : `src/pico8_ob.c` et `src/pico8_stash.c` de `ENGINE_SRCS` (l. 205, 222) et `DATA_ENGINE_SRCS` (l. 270, 283), de la ligne de lien de `test_pico8_layout_nil_item_draws_nothing` (l. 579, §6 : seul le `Makefile` change) et des deux lignes de `main_raylib` (l. 997-998) ; les règles de `test_pico8_ob` et `test_pico8_stash` (l. 429-433) et leurs entrées de `UNIT_TESTS` (l. 595-596). `DATA_RESIDUE_GUARD := spec,overrides,stash,ob`. Aucune autre ligne ; `make raylib` n'est pas exigé (vendoring), la grep 4 de la « Sortie » en tient lieu.

### Ce qui ne change pas

- `tools/` (dont `data_residue.sh` et ses motifs figés, `ref_audit_report.sh`, `line_coverage_reference.txt`), `tests/e2e/`, tout `*.sha256`, tout fichier d'événements ; aucun e2e ajouté.
- Toute constante `*_HASH` et tout tableau `identity[]`, de valeur et de nom.
- `tests/test_data_residue.c` : ses fixtures écrivent `"pico8_" "ob_table_t"` en deux morceaux pour tester le motif `ob` de l'outil ; la catégorie reste définie (motifs figés).
- `par-nom`, `dispatch`, `overrides`, `spec`.

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours ; macro, `typedef`, constante nommée, `enum`, pointeur ou fonction enveloppe dont le rôle est de renommer un identifiant **encore vivant** ; un alias ; un registre (tableau, table, `switch` ou boucle nouveaux qui nomment ensemble des valeurs, cas ou noms) ; une aide de TU nouvelle ; déplacer du code compté hors de `src`/`tests`, hors `.c`/`.h` ; supprimer un commentaire qui ne porte pas sur une donnée, un cas ou une inclusion retirés ; tout changement d'une constante de hachage. **Seule exception, déclarée** : les deux (c) des octets de `hp`/`hy`, dont l'énumération source est supprimée dans le même commit.
- **Classification exigée**, au message de chaque commit, pour chaque ligne `ob` ou `stash` retirée : (a) construction supprimée (fichier supprimé, allocation, pose ou lecture de table ob, enregistrement, branche de pli, inclusion devenue inutile, commentaire ou message d'une donnée qui n'existe plus), (b) déclaration ou variable dont le type passe d'une table ob à un type C, (c) même donnée, autre nom. Attendu : commit 1, `ob` (a) 2 ; commit 2, `ob` (a) 411 + **(c) 2** déclarées, (b) 0 ; `stash` (a) 28. Recette : `git diff -U0` par fichier, lignes `-` qui portent le motif en regard des lignes `+` ; un fichier supprimé compte en bloc (a).
- **Mesure exigée**, avant et après chaque commit, par catégorie, **lignes** (`make data-residue`, `--detail` par fichier) **ET occurrences**. Recette figée (`$S` : dossier du scratchpad) :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  X=$(sed -n "s/^SPEC_EXCLUSION='\(.*\)'\$/\1/p" tools/data_residue.sh)
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E "$X" "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  ```

  Mesuré sur `57e2e73` : `valeur` 5931, `ob` 470, `stash` 28, `type-tag` 572, `ref-audit` 66, `par-nom` 1064, `dispatch` 193, `overrides` 0, `spec` 0. Les fichiers supprimés sont mesurés sur l'arbre de départ du commit et rapportés en bloc.
- **Contrôle** : pour chaque (catégorie, fichier) du diff, Δlignes et Δoccurrences ; si Δlignes > Δoccurrences, le message justifie ce couple. Cas attendu inverse (Δoccurrences > Δlignes, ligne gardée) : `pico8_type_tag.h` l. 59 (`type-tag` −1 occurrence, 0 ligne) ; dit au message.
- **Lignes qui gagnent une catégorie**, listées par fichier et par catégorie, « aucune » compris. Attendu : **aucune** (`VIEW_INDEX_KEY`, `TABLE_INDEX_KEY`, les assertions reformulées du fichier gardé, les commentaires réécrits ne portent aucun motif qu'une ligne retirée en regard ne portait).

### Empreinte remesurée et découpage (G)

| Commit | Chemins | `ob` retirées (lignes / occ.) | `stash` retirées |
|---|---|---|---|
| 1 — octet de `jc` | `tests/test_pico8_jc.c` | 2 / 2 | 0 |
| 2 — suppression | supprimés (6) : `src/pico8_ob.c`, `src/pico8_ob.h`, `src/pico8_stash.c`, `src/pico8_stash.h`, `tests/test_pico8_ob.c`, `tests/test_pico8_stash.c` ; `src/` (11) : `pico8_collect.c`, `pico8_collect.h`, `pico8_pi.c`, `pico8_pi.h`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_value.h`, `pico8_list.h`, `pico8_hud.c`, `pico8_entity.h`, `pico8_entity.c` ; `tests/` (4) : `test_pico8_map_tables.c`, `test_pico8_hp.c`, `test_pico8_hy.c`, `test_pico8_planet_gh_row_lq.c` ; `Makefile` ; `docs/` (3) : `plan-3.md`, `trous-de-portage.md`, `glossaire-noms.md` | 413 / 468 | 28 / 28 |
| **Total** | **26 chemins** | **415 / 470** | **28 / 28** |

- **Pourquoi un seul gros commit 2** : le test ponctuel et les tests de la couche meurent « dans le commit qui supprime » la couche (puce 2, §6) ; les R servent le test ponctuel (D.7a) ; les R_b et la l. 2 de `gh_row_lq` sont vrais tant que la couche vit (les réécrire avant serait renommer un sujet vivant) ; les octets de `hp`/`hy` ne peuvent quitter l'énumération qu'à sa suppression (arbitrage de D.7c) ; la garde entre dans le commit qui éteint les catégories. Tout dépend de la suppression, rien ne la précède. 26 chemins, sous le plafond de ~40.
- **Pourquoi `jc` à part** : sa preuve est d'une autre nature (un octet retiré, format changé, constantes vertes parce que non atteint) que celle de `hp`/`hy` (octet gardé, format inchangé) ; isolée, elle ne peut être masquée par la suppression.
- **Rejetés** : un commit par groupe (couche, stash, R, tests) — chaque commit intermédiaire casserait le lien du test ponctuel ou laisserait une R vivante décrire un fichier supprimé ; `jc` fondu dans le commit 2 — la preuve « non atteint » se mêlerait à celle des octets gardés.

### Effet sur `make data-residue`

| Catégorie (lignes / occ.) | Départ (`57e2e73`) | Après 1 | Après 2 | Pourquoi |
|---|---|---|---|---|
| `ob` | 415 / 470 | 413 / 468 | **0 / 0** | exact ; dans `DATA_RESIDUE_GUARD` |
| `stash` | 28 / 28 | 28 / 28 | **0 / 0** | exact ; dans `DATA_RESIDUE_GUARD` |
| `type-tag` | 362 / 572 | = | **≈ 354 / 562** | `pico8_ob.c` 1/1, `pico8_ob.h` 3/4, `type_tag.h` l. 39 et une occurrence de l. 59, `ref_audit.h` l. 163, 226 et la l. 11 du paragraphe réécrit (à remesurer) |
| `ref-audit` | 65 / 66 | = | **56 / 57** | `pico8_ob.c` 7/7, `ref_audit.h` l. 226, 249 |
| `valeur` | 5125 / 5931 | = | **≈ 4961 / 5742** | fichiers supprimés 54/62, test ponctuel ≈ 92/97 (≈ 5/6 gardées), `collect.c` 10/14, `pi.c` 6/14, `ref_audit.h` 2/2 |
| `par-nom`, `dispatch`, `overrides`, `spec` | 1009, 191, 0, 0 | = | = | aucune ligne touchée (`kind->field` du fichier gardé reste) |

« Exact » et « = » sont des égalités ; « ≈ » est remesuré au commit et l'écart expliqué.

### Hypothèses (rien n'a été lancé)

**Sur l'issue de D.7c, non gradée** — à comparer à la note gradée avant de passer ce PRD en triage :

- **H-D7c-1** : D.7c est gradée sur `57e2e73` sans commit correctif qui touche `tests/test_pico8_hp.c`, `_hy.c`, `_jc.c`, `_planet_gh_row_lq.c`, `_map_tables.c` ni `src/` ; les numéros de ligne de l'inventaire sont ceux de `57e2e73` (hp 32, 180, 195-197 ; hy 33, 173 ; jc 29, 266 ; gh_row_lq 2-3).
- **H-D7c-2** : `ob` après D.7c = 415 lignes / 470 occurrences dans 20 fichiers ; hors test ponctuel 216 = 187 + 22 + 1 + 6 (6 : les trois plis et leurs trois inclusions).
- **H-D7c-3** : R = 22 et sa liste sont inchangés par D.7c (`pi.c` 2, `pi.h` 1, `collect.c` 7, `collect.h` 3 ; R_b `pico8_type_tag.h` 2, `pico8_ref_audit.h` 4, `pico8_value.h` 1, `pico8_list.h` 1, `pico8_hud.c` 1).
- **H-D7c-4** : l'arbitrage du pilote tient à la gradation : les trois plis et leurs inclusions restent jusqu'à D.8 ; les octets de `hp` et `hy` sont atteints (`DEFAULTS_HASH`, `CONSTRUCTION_HASH`), celui de `jc` ne l'est pas ; D.8 « doit plier la même valeur sans la couche, arbitrage à reprendre, jamais une constante mise à jour ».
- **H-D7c-5** : `tests/` ne nomme plus `PICO8_TYPE_TAG_OB_TABLE` hors `test_pico8_ob.c` et du test ponctuel ; `type-tag` = 362 lignes / 572 occurrences.
- **H-D7c-6** : comptes des autres catégories après D.7c : `valeur` 5125/5931, `stash` 28/28, `ref-audit` 65/66, `par-nom` 1009/1064, `dispatch` 191/193, `overrides` 0, `spec` 0 ; `DATA_RESIDUE_GUARD` = `spec,overrides`.
- **H-D7c-7** : la ligne D.8 et le reste du plan ne sont pas modifiés à la gradation (aucune proposition `plan-review` retenue sur eux) ; la note « Découvert à D.7c » gradée est celle de `57e2e73`, dont « D.8 les hérite avec la l. 2 de `test_pico8_planet_gh_row_lq` » et « §6 : `gh_row_lq`, commentaire de la l. 2 seulement, pas de preuve rouge ».
- **H-D7c-8** : l'`enum { VIEW_TAG, VIEW_INDEX_KEY, VIEW_NAME_KEY, VIEW_LIST_TAG }` de `test_pico8_hp.c` (l. 195-197) et celui de `test_pico8_lq.c` (l. 138) sont inchangés par D.7c (base de l'option 1).
- **H-D7c-9** : les commentaires « a list or an ob_table » de `fold_value` (`hp` l. 130-131, `hy` l. 131-132), restés après le retrait de la branche par D.7c, ne sont pas repris par la vérification de D.7c ; s'ils le sont, D.8 les retire de sa liste.
- **H-D7c-10** : le balayage ASan+UBSan jetable des 28 TU de D.7c est propre, et la vérification n'ajoute aucun reste dans `src/` ni dans `tests/`.

**Sur D.8** :

- **H1** : sans `pico8_ob.h` ni `pico8_stash.h`, chaque fichier restant atteint encore `pico8_type_tag.h`, `pico8_value.h`, `pico8_list.h` quand il en nomme un symbole (parcours du graphe à `57e2e73`) ; `-Werror` reste muet.
- **H2** : le binaire de rejeu n'exécute aucune ligne supprimée hors les trois enregistrements de `pico8_pi_init` (jamais relus) : `make e2e` rend 26 conformes, `class-coverage-net` est identique, `sanitize-net` propre, sans fuite nouvelle.
- **H3** : `PICO8_OB_KEY_INDEX` = 0 ; `VIEW_INDEX_KEY` = 0 ; `DEFAULTS_HASH` et les constantes de `hy` passent inchangées.
- **H4** : les tests gardés du fichier du test ponctuel ne dépendent ni de `pico8_load_map_json` ni de `pico8_pi_init` dans `main` (`pico8_jo_init` et les deux autres `*_init` appellent `pico8_pi_init` eux-mêmes).
- **H5** : aucun TU ne rejoue d'événements ; `make test` recompile tout (en-têtes communs touchés) et reste de l'ordre de la minute.
- **H6** : `constants-check`, `float-grep`, `float-guard`, `data-guard` restent verts.

## Implementation Chunks

Deux commits, dans cet ordre, chacun vert seul (`make test`, `make e2e`, `make data-guard`), chacun avec ses comptes, sa classification, ses lignes gagnées et ses mutations :

1. **Octet de `jc`** — `fold_hg` sans l'octet de sorte de clé, commentaire ramené au format, inclusion retirée ; constantes de `jc` inchangées.
2. **Suppression** — fichiers de la couche et leurs tests ; test ponctuel retiré du fichier, tests typés gardés ; R et R_b ; `PICO8_TYPE_TAG_OB_TABLE` et l'audit ; octets de `hp`/`hy` par constante locale nommée, (c) déclarées ; l. 2-3 de `gh_row_lq` ; `Makefile` et `DATA_RESIDUE_GUARD` ; `trous-de-portage`, `glossaire-noms` ; `make sanitize-net`, `make class-coverage-net` ; note « Découvert à D.8 ».

Ordre conseillé dans chaque commit : mesure de départ (lignes et occurrences par fichier, fichiers supprimés compris) ; édition ; `make test` ; `make e2e` ; mutations (copie jetable) ; filets du commit 2 ; mesure d'arrivée, classification, lignes gagnées, greps ; docs ; message.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, identités, arrêts), pas la présence d'une ligne. D.8 retire du code que le jeu n'exécute plus : **la preuve est que rien de ce que les TU et les e2e observent ne change** — constantes inchangées, 26 e2e conformes, `sanitize-net` propre, `class-coverage-net` identique — et que chaque garde gardée mord encore.

### TU touchés (G)

- **Supprimés** : `test_pico8_ob.c`, `test_pico8_stash.c` (tests de la couche, §6) ; la comparaison à la référence de `test_pico8_map_tables.c` (test ponctuel, puce 2). Listés au message, avec leurs tests.
- **Gardés** : les tests typés de `test_pico8_map_tables.c` (décision plus haut), listés un par un au message avec, pour les reformulés, les assertions d'origine qu'ils portent.
- **Plis** : `hp`, `hy`, `jc` ; aucune constante ne change, ni de valeur ni de nom.
- **§6** : `gh_row_lq`, l. 2-3 seulement. **Pas de preuve rouge** : aucune lecture, aucune assertion ne change ; le §6 n'en exige que pour une réécriture de lecture (précédent D.6a). Aucun autre test du §6 n'est touché (la ligne de lien de `layout_nil_item_draws_nothing` est dans le `Makefile`).
- **Test ajouté** : aucun (G) ; rien de nouveau n'est à couvrir, le code retiré n'est plus exécuté par le jeu.
- **Prior art** : `test_nb_lookup_finds_each_px` (forme des recherches reformulées), `check_int_seq_at_bounds` (forme des bornes), `VIEW_*` de `test_pico8_hp`/`test_pico8_lq` (octets gelés nommés).

### Preuve des tests (§7 : `gcov` et mutations)

`gcov` : la seule ligne exécutable modifiée hors suppression est la condition de `pico8_type_tag_of` (`pico8_type_tag.h` l. 59), exécutée par tout TU qui lit un tag ; aucune ligne ajoutée à `src/`. Les lignes reformulées du fichier gardé sont des tests. Rapport `gcov` non exigé, dit au message.

Mutations, **dans une copie jetable du scratchpad**, chacune abandonnée avec la copie :

- commit 1 — **M1** : remettre dans `fold_hg` un octet quelconque avant chaque `id` → toutes les constantes de `test_pico8_jc` **restent vertes** (confirme « non atteint » ; c'est l'inverse d'une mutation qui rougit, attendu et dit au message).
- commit 2 — **M2** : la l. 180 de `hp` plie `VIEW_INDEX_KEY + 1` → `DEFAULTS_HASH` rouge ; **M3** : `TABLE_INDEX_KEY = 1` dans `hy` → une constante qui plie `po`/`fu` rouge ; **M4** : `pico8_qe_map_set` n'arrondit plus la clé → `test_qe_map_keeps_one_entry_per_key_in_order` rouge ; **M5** : `pico8_int_seq_at` accepte l'indice 0 → le test de bornes reformulé rouge ; **M6** : `pico8_iz_lookup` rend la première entrée sur une clé absente → le test de recherche reformulé rouge.
- Une mutation qui ne se comporte pas comme écrit est un trou : justifié au commit ou couvert par une assertion existante réintroduite.

### Filets

- `make sanitize-net` (commit 2) : propre. Exigé par le §7 (« `malloc`/`free` retiré ») : `pico8_pi_vec_v_constructor` et les allocations de `pico8_ob.c`/`pico8_stash.c` partent.
- `make class-coverage-net` (commit 2) : identique à `tools/class_coverage_net_expected.txt` ; en confirmation (la ligne dit « non » : aucun constructeur d'entité ne change), passé parce que `src/` change et que `pico8_pi_init` change.
- `make e2e` à chaque commit : 26 conformes. Aucun e2e ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 25 minutes pour l'étape** (commit 1 ≈ 3 ; commit 2 ≈ 22, dont les filets).

| Cible | Durée attendue |
|---|---|
| `make test` | de l'ordre de la minute au commit 2 (recompilation complète : `pico8_type_tag.h`, `pico8_list.h`, `pico8_value.h` changent) ; quelques secondes au commit 1 |
| `make e2e` | ~1 s |
| `make data-guard`, `make data-residue`, recette, classification, greps | < 1 min |
| mutations | < 4 min |
| `make sanitize-net` | quelques minutes (build ASan+UBSan et 26 rejeux) |
| `make class-coverage-net` | quelques minutes |

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement : `#`, commande exacte, état de l'arbre (HEAD + commit + étape), code de sortie, ligne clé, durée. Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit liste les chemins de son chunk (tableau « Empreinte »), rien d'autre ; ni `tools/`, ni `tests/e2e/`, ni `*.sha256`.
2. **Greps** 1 à 6 de la « Sortie » : exactement ce qui y est écrit.
3. **Constantes** : aucune `*_HASH`/`identity[]` changée (diff) ; `hp` plie `VIEW_INDEX_KEY` (aucun identifiant nouveau), `hy` plie `TABLE_INDEX_KEY` (une constante, un commentaire de contrat), `jc` ne plie plus d'octet de sorte.
4. **§6** : `gh_row_lq` : diff limité aux l. 2-3 ; `ga_export_panel_by` et `layout_nil_item_draws_nothing` : sources inchangées.
5. **Fichier gardé** : chaque test gardé ou reformulé correspond à des assertions présentes à `57e2e73` ; aucune lecture de la référence, aucun `pico8_load_map_json`.
6. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; couples Δlignes > Δoccurrences justifiés ; classification présente et juste, (c) = 2 exactement, déclarées ; lignes gagnées listées par fichier.
7. **Mutations** M1-M6, `sanitize-net`, `class-coverage-net` rapportés.
8. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.8 ni au plan dans `src/` ou `tests/`.
9. **Docs** : note « Découvert à D.8 » en fin de ligne D.8 ; TROU-4/TROU-9 et le glossaire comme décrit ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le commit de gradation de D.7c (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas `ob` 415 et `stash` 28 (ou les valeurs de la note gradée, H-D7c-2), `overrides` 0, `spec` 0, sans explication.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change autrement qu'aux l. 2-3 de `gh_row_lq`, ou si un test devrait changer d'intention pour passer.
- `FAILED:` si `make sanitize-net` n'est pas propre ou si `make class-coverage-net` diffère de l'attendu versionné.

**Périmètre**

- `FAILED:` si une des greps 1 à 6 de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si un fichier de `tools/` change, ou si le `Makefile` change ailleurs qu'aux entrées nommées et à `DATA_RESIDUE_GUARD`.
- `FAILED:` si le fichier gardé du test ponctuel lit encore la référence, ou porte une assertion absente à `57e2e73`.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert ou si `DATA_RESIDUE_GUARD` ne vaut pas `spec,overrides,stash,ob` au commit 2.
- `FAILED:` si `ob` ou `stash` ne vaut pas 0, si `ref-audit` ne vaut pas 56 lignes, si `type-tag` ou `valeur` ne baisse pas, ou si `par-nom`, `dispatch`, `overrides`, `spec` bougent.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, construction déplacée, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, alias, registre, code déplacé hors du compteur ; ou si une (c) autre que les deux octets de `hp`/`hy` apparaît.
- `FAILED:` si le **message d'un commit** ne donne pas, par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences**, la classification (a)/(b)/(c) des lignes `ob`/`stash` retirées (chaque (c) déclarée et justifiée), la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris), la justification de chaque couple Δlignes > Δoccurrences ; un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si un test supprimé n'est pas listé au commit avec sa raison, ou si un test gardé du fichier du test ponctuel disparaît sans raison écrite.
- `FAILED:` si une mutation manque ou ne se comporte pas comme écrit sans justification.
- `FAILED:` si le budget est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.8 », ou si elle omet les comptes réels, le choix des octets figés, le sort du fichier du test ponctuel, ou ce qui reste de la sortie de la phase.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **`pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`** (le reste de la sortie de la phase), `valeur`, `type-tag`, `ref-audit` : aucune puce de D ne les couvre ; D.9 (non tranchée) et sa Q12.
- **`par-nom`, `dispatch`, `pico8_list_t`** : D.10-D.12 proposées par D.9, en attente de l'humain.
- **`tools/`** : `ref_audit_report.sh` (son mot `ob_table` devient mort), `line_coverage_reference.txt` (246 lignes de `pico8_ob.c`/`pico8_stash.c` ; référence « écrite une fois, on ne la recalcule pas », déjà décalée par les suppressions de 1a-D.7c, hors suite, pour `make e2e-gap`), `data_residue.sh` (motifs figés).
- **Les commentaires non comptés** qui ne sont pas dans un fichier touché ou qui énoncent un bug du §6 (`gh_row_lq` l. 136, `ga_export_panel_by` l. 2-3) : passe de commentaires ultérieure.
- **Les docs historiques** (`docs/plan.md`, `docs/plan-2.md`, `docs/audit-ref-cast-reverse.md`, PRD livrés).
- **La revue de fin de phase** (`warn-review`, `plan-review`) : la phase D ne sort pas à D.8.
- La phase 2 ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **modules** : couche et stash supprimés ; `collect`, `pi`, `type_tag`, `ref_audit` réduits ; aucun module nouveau.
  - **fichier du test ponctuel** : la comparaison part, les tests typés restent ; rejetés : tout supprimer, déplacer, compléter.
  - **octets de `hp`/`hy`** : option 1, constante locale nommée (`VIEW_INDEX_KEY` existant dans `hp`, `TABLE_INDEX_KEY` nouveau dans `hy`), (c) déclarées ; rejetés : reprendre les constantes (option 2), littéral, nom recopié.
  - **octet de `jc`** : retiré, commit à lui, constantes vertes.
  - **`gh_row_lq`** : l. 2-3, pas de preuve rouge ; l. 136 et `ga_export_panel_by` non touchés.
  - **audit** : compteur et colonne `ob_table` retirés, outil inchangé.
  - **docs** : TROU-4/TROU-9 « sans site », glossaire mis à jour.
  - **filets** : `sanitize-net` exigé, `class-coverage-net` en confirmation.
  - **découpage** : deux commits (1 et 25 chemins).
- **Note du plan (G)**, à ajouter en fin de ligne D.8 (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.8 : deux commits (octet de `jc`, non atteint ; suppression — N chemins). `test_pico8_map_tables.c` perd la comparaison à la référence et garde les tests des modules typés qui ne la lisent pas. Les octets de sorte de clé de `hp` et `hy` sont pliés par une constante locale nommée (`VIEW_INDEX_KEY`, `TABLE_INDEX_KEY`), octet gelé d'une énumération supprimée, (c) déclarées ; aucune constante `*_HASH` ne change. `PICO8_TYPE_TAG_OB_TABLE` et le compteur `ob_table` de l'audit supprimés. `ob` = 0, `stash` = 0, dans `DATA_RESIDUE_GUARD` ; `type-tag` T lignes, `ref-audit` A, `valeur` V. Sortie de la phase non atteinte : `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h` restent, avec `valeur`, `type-tag`, `ref-audit`, `par-nom`, `dispatch` ; la suite attend les décisions humaines de D.9.

  N, T, A, V sont les valeurs réelles.
- **Le message de chaque commit** porte : le périmètre ; la classification (a)/(b)/(c) des lignes `ob`/`stash` retirées ; les comptes avant et après **en lignes et en occurrences** par catégorie et par fichier (fichiers supprimés en bloc), avec les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie, par fichier (« aucune » compris) ; les tests supprimés, gardés et reformulés ; les mentions non comptées laissées ; l'historique retiré des commentaires (les quatre débordements de tas de `pico8_type_tag.h`, la lecture inverse de `pico8_ref_audit.h`) ; les mutations ; les filets ; le tableau « Runs » résumé ; les réponses aux hypothèses H qui le concernent.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque commit contre ce PRD avant le commit (hors budget de tests) : anti-tassement, classification, lignes gagnées, greps, fichier gardé.
- **Sections à revérifier quand D.7c sera gradée** (comparer à la note gradée et au commit de gradation, puis mettre à jour avant de passer en triage) :
  1. « Problem Statement » : la note « Découvert à D.7c » recopiée (mots et chiffres) et la ligne D.8 (H-D7c-7).
  2. « État de départ » et « Inventaire » : commit de départ, tableau `make data-residue`, occurrences, numéros de ligne (H-D7c-1, H-D7c-2, H-D7c-5, H-D7c-6).
  3. « Les octets figés de `hp` et `hy` » et « L'octet non atteint de `jc` » : l'arbitrage gradé du pilote, atteinte de chaque octet, l'`enum VIEW_*` (H-D7c-4, H-D7c-8).
  4. « Commentaires R_b et mentions » : R = 22 (H-D7c-3) ; les commentaires « or an ob_table » de `fold_value` (H-D7c-9).
  5. « Empreinte remesurée et découpage » et « Effet sur `make data-residue` » : tous les jalons (413/468, 0, ≈ 354/562, 56/57, ≈ 4961/5742).
  6. « Sortie » et « Critères d'échec », puce du départ.
  7. Toute reprise de `tests/` par la vérification de D.7c qui toucherait `hp`, `hy`, `jc` ou `gh_row_lq` (H-D7c-10).
- **Surprises pour le pilote** :
  - **La sortie écrite de la phase D n'est pas atteinte à D.8** : deux fichiers sur cinq ; `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h` attendent D.9 et au-delà. Après D.8, `plan-pilot` s'arrête.
  - **Le précédent de l'option 1 est déjà dans le dépôt** : `VIEW_TAG = 0x9E5C0B7Au` (la valeur de `PICO8_TYPE_TAG_OB_TABLE`) et `VIEW_INDEX_KEY`/`VIEW_NAME_KEY` gèlent depuis D.7a les octets de la couche dans `test_pico8_hp` et `test_pico8_lq` ; `hp` n'a besoin d'aucun nom nouveau.
  - **Le fichier du test ponctuel n'est pas que le test ponctuel** : `pico8_gn`, `pico8_qe_map_set`, `pico8_int_seq_has_fixed`, `pico8_fixed_seq_at`, `pico8_iz_lookup` n'ont pas d'autre TU (grep à `57e2e73`) ; le supprimer en entier retirerait leurs gardes.
  - **Le jeu n'exécute plus rien de la couche** hors trois écritures d'un registre jamais relu : la suppression ne doit rien changer aux e2e ni aux filets.
  - **D.7c a laissé « a list or an ob_table » dans le contrat de `fold_value` de `hp` et `hy`** après avoir retiré la branche (non compté) ; D.8 le reprend puisqu'il touche ces fichiers.
  - **`tools/line_coverage_reference.txt` nomme 246 lignes de la couche** ; hors suite et déjà périmé, laissé tel quel.
