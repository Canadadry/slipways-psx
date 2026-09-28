---
title: "Plan 3, étape D.6c : les clés de construction du reste (`g_`, `fv`, `ie`, `hp`, `hy`, `trade`, `pj`) passent d'une table ob à une struct typée locale, ou à aucun paramètre ; `hp` prend l'enregistrement généré de `fd` et le pont `pico8_fd()` est fermé ; `overrides` tombe aux 30 lignes des quatre ponts de D.6b et de leurs TU, et n'entre pas encore dans `DATA_RESIDUE_GUARD` — deux commits, aucun pixel changé"
description: "Sept constructeurs reçoivent encore leurs clés dans une `pico8_ob_table_t` que l'appelant alloue, remplit par nom, passe puis libère (icônes de recherche de `eu`, routes de `jo:ib` et `trade:kg`, sonde à clés des TU), ou que `build_gb` tire de la vue `pico8_fd()` construite à chaque init ; trois autres n'acceptent qu'une table vide. D.6c type ces dix paramètres, fait lire à `hp` l'enregistrement généré `pico8_fd_entry_t`, ferme le pont `pico8_fd()`, et laisse `overrides` aux seuls ponts de D.6b et à leurs TU (30 lignes) — en mesurant la baisse en lignes ET en occurrences, sans tassement ni renommage à type constant."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.6c — `overrides`, reste : `g_` (`probe`), `fv` (`help`), `ie` (`research_icon`, `cart.c`), `hp` (`research`), `hy` (`economy`), `trade` (et `planet_trade.c`), `pj` (`score_panel`), même règle. Empreinte 21 (16, 5). Compte : `overrides` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui.

Lignes voisines, recopiées (la « même règle » est celle de D.6a, reprise par D.6b ; la note de D.6a est recopiée plus bas avec les autres) :

> - D.6a — `overrides`, constructeurs de l'arbre `jo` (`pico8_jo_new` … `pico8_mx_new`), `new_entity` de `pico8_entity_kind_t` (`pico8_struct.h`) et ses appelants (`galaxy.c`, `jo_kv`…) : la table de clés de construction devient une struct locale (troisième puce), paramètre renommé. Empreinte 11 (8, 3). Compte : `overrides` = 315. §6 : `gh_row_lq`, commentaire de la l. 2 seulement ; aucune lecture ne change, pas de preuve rouge. `class-coverage-net` : oui.
>
> - D.6b — `overrides`, mise en page et entités d'écran : `lq`/`jb` (`layout`), `js`/`fq` (`ui_button`), `mh`/`iv` (`msgbox`), `ui`, `qb` (`scroll`), `nf` (`mouse_reader`), `bg`, `mz` (`wormhole`), `jc` (`slipway`), appelants de `menu.c`, même règle. Empreinte 30 (19, 11). Compte : `overrides` = 117. §6 : `layout_nil_item_draws_nothing` (l. 106-112) passe son item par la struct de clés de `pico8_lq_new` ; l'item reste un `pico8_value_t`, élément de `lq.items` ; même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : oui.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » des étapes livrées, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b, D.5c, D.6a, D.6b) :

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
> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.

### Lecture de ces textes pour D.6c

- **« Même règle »** (celle de D.6a, reprise par D.6b) : la table de clés de construction devient une **struct locale** à l'appelant, passée par pointeur `const`, lue par le constructeur et jamais gardée ; une classe qui n'accepte aucune clé perd son paramètre ; le paramètre est renommé **seulement parce que son type change**. Un renommage à type constant ferait tomber `\boverrides\b` sans rien convertir : c'est le **tassement par renommage**, interdit et traduit en `FAILED:` (section « Anti-tassement »).
- **Le « = 0 » de la ligne ne peut pas tenir, et D.6c ne le vise pas.** D.6b a gardé, comptés sous le nom `overrides`, quatre ponts où une rangée ob **reçue d'un producteur non typé** est lue par nom (`pico8_lq_new_table`, `pico8_jb_new_table`, `pico8_js_new_table`, `pico8_mh_new_table` : 21 lignes / 25 occurrences, 6 fichiers), et les TU de ces ponts qui leur passent une table de même type et même nom (`test_pico8_mh` : `test_mh_nil_lh_key_reads_absent` 4 lignes, `build_with_key` 3 lignes ; `test_pico8_lq` : commentaires l. 258 et 281 — 9 lignes / 9 occurrences, 2 fichiers). Leurs producteurs (`planet_display`, `menu`, `score_panel_display`, `research_display`, `cart` : D.7a ; `help`, `msgbox`/`pz` : D.7b) sont hors de D.6c ; les fermer ici, c'est faire D.7a/D.7b dans D.6c, et les TU des ponts tombent avec eux. **Cible honnête de D.6c : `overrides` = 30 lignes / 34 occurrences / 8 fichiers, exactement les lignes des ponts et de leurs TU**, celle que fixe la note de D.6b (« le « = 0 » de D.6c devient « = 30 » (les ponts et leurs TU) »).
- **`DATA_RESIDUE_GUARD` ne change pas à D.6c.** La règle commune dit « une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit » ; `overrides` ne tombe pas à 0. Elle y entre **à l'étape qui ferme le dernier des quatre ponts** : dans l'ordre actuel de la liste, **D.7b** (le pont `js` reçoit les rangées `bs` de `pz`, `msgbox.c`, fichier de D.7b ; `lq`, `jb`, `mh` et les autres sources de `js` ferment avec D.7a). Si le PRD de D.7a fusionne ou réordonne D.7a/D.7b, c'est l'étape qui ferme le dernier pont. **Rejetés** : un garde partiel (compte plafonné, liste blanche des ponts dans `tools/data_residue.sh`) — les motifs et l'outil sont figés (note de D.0), et un plafond ajouterait un mécanisme que la liste ne prévoit pas ; D.6c le remplace par un contrôle du vérificateur (chaque ligne `overrides` restante est dans un des quatre ponts ou dans les deux TU qui les testent).
- **Le « 97 » de la note de D.6a, recompté sur `58e39e3`** : les dix-neuf fichiers de D.6c portent **exactement 97 lignes / 104 occurrences** `overrides` (inventaire plus bas), au texte de `09f0f56` (seuls les numéros de ligne de `cart.c` et `test_pico8_ie` ont bougé) ; les trois TU `test_pico8_hp`, `_hy`, `_trade` ne portent plus aucune ligne. D.6c part de **127 / 138 / 27** et retire **97 / 104 / 19**.
- **Pont de données nommé pour D.6c** : la note de D.2 dit « `pico8_fd()` (fermé par D.6c, entrée de `hp`) ». `hp` est le seul lecteur de `src/` de cette vue (`pico8_research_build_gb`, appelé par `pico8_data_init`) ; ses enregistrements n'ont que des items scalaires (`pico8_fd_entry_t` : chaînes, entiers, `il` faux). Il **ferme** ici, sur le modèle de `pico8_dr()` à D.6b : `hp` prend l'enregistrement généré, `build_gb` parcourt `pico8_fd_table`.
- **§6 : aucun.** `ga_export_panel_by` construit ses routes `route_a`/`route_b` comme des tables ob d'enregistrements de `jo:ib` (D.7a), sans appeler `pico8_trade_new` ; `gh_row_lq` et `layout_nil_item_draws_nothing` ne touchent aucune classe de D.6c. Aucun test du §6 n'est modifié.
- **`make class-coverage-net` et `make sanitize-net`** : oui aux deux, à chaque chunk : chacun touche des constructeurs, et chacun change la durée de vie d'une table (tables de clés du tas à la pile ; au chunk 2, la vue `fd` construite à chaque init, la copie de travail `ri` et le `rc` jetable de chaque `hp` disparaissent).

### D.6b livrée : hypothèses vérifiées sur `58e39e3`

Ce PRD a été écrit sur `09f0f56` pendant l'implémentation de D.6b, sous quatorze hypothèses H-D6b, puis relu contre l'arbre gradé de D.6b (`58e39e3` ; commits `7f45ca4`, `11e1cf4`, `ca29aaa`) et sa note. **Tenues** : les 97 lignes de D.6c intactes ; la convention (`pico8_X_keys_t`, `const … *keys`, `void` pour `bg`, `nf`, `qb`, `ui`, `mz`) et aucun nom en collision ; pont `pico8_dr()` fermé, `eu` construit ses `jb` par `pico8_jb_new(&pico8_dr_table[i])` dans la boucle où restent les six lignes `ie` ; `test_pico8_ie` (`dr_row`) et `test_pico8_entity_kind` (`ie_jb`) lisent `pico8_dr_table` ; `DATA_RESIDUE_GUARD` = `spec`, `Makefile` et `tools/` inchangés ; arbre vert ; le commentaire de `pico8_data_init` dit « fd and nb are built » ; `g_fd`, `data_init_fd_view`, `pico8_fd()` et `test_fd_view_matches_the_decoded_table_plus_rr` intacts ; aucun constructeur ni appelant de D.6c touché ; le test ponctuel garde ses 2 lignes `stash`. **Démenties et corrigées ici** : le départ est **127 / 138 / 27**, pas 118 / 129 / 25, parce que D.6b a laissé, en plus des 21 lignes des ponts, **9 lignes des TU des ponts** (`test_pico8_mh` 7, `test_pico8_lq` 2) ; la note de D.6b fixe donc la cible de D.6c à **« = 30 »**, pas « = 21 ». Toutes les sections qui en dépendaient (Lecture, Inventaire, Solution, Sortie, Effet sur `make data-residue`, jalons, contrôles, critères d'échec) ont été relues et corrigées. Rien d'autre ne dépend encore de D.6b.

### État de départ

**Mesuré sur `58e39e3`** (branche `claude/plan-pilot-3-7zdft0`, D.6b gradée) ; **l'arbre de départ de D.6c est ce commit** (ou un commit de docs au-dessus). Décisions de l'utilisateur qui engagent D.6c : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**. Consigne de pilotage, liante : **une étape précédente a été notée FAILED pour avoir fait baisser un compte par tassement de lignes** ; le compteur compte des lignes, et toute mise en forme, tout renommage à type constant, tout alias, registre ou déplacement qui baisse un compte sans retirer la construction est interdit.

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `sh tools/data_residue.sh` (avec `--detail`) et la recette d'occurrences ont tourné, d'abord sur la copie `git archive 09f0f56` du scratchpad, puis sur l'arbre versionné de `58e39e3` ; le reste vient de `grep` et de la lecture des sources et de `pico8/lua.lua`. Tout fait d'exécution est une hypothèse (H1 à H15).

### Inventaire (mesuré sur `58e39e3`)

**`make data-residue`** (lignes / fichiers) et **occurrences** (recette de la section « Anti-tassement ») :

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 5550 | 133 | 6438 |
| `ob` | 1462 | 109 | 1625 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 406 | 69 | 654 |
| `ref-audit` | 68 | 17 | 69 |
| `par-nom` | 1062 | 81 | 1117 |
| `dispatch` | 199 | 60 | 201 |
| `overrides` | 127 | 27 | 138 |
| `spec` | 0 | 0 | 0 |
| **résidu D** | **7694 lignes** | **140 fichiers** | — |

`overrides` **127 / 138 / 27 fichiers** : 97 / 104 / 19 de D.6c, 21 / 25 / 6 des ponts, 9 / 9 / 2 des TU des ponts.

**Ce que dit le cart** (`pico8/lua.lua`) : le constructeur de classe (`kv:jr`) copie les clés passées dans l'instance (`mw(ob)`) ; une clé nil n'existe pas. `g_=ke:jr(…)` (l. 708) n'est construit par le cart qu'**sans clé** (`bg:ldown`) ; le chemin à clés `pn`/`t`/`du`/`py` n'existe que pour le TU `probe_mm_rnd_order` (« road (c) » de `test_pico8_g_`). `ie({ke=b,jb=jb(dr[i])})` (l. 1185, `eu`). `trade({lp={self},rl={w},qe=qe})` (l. 504, `jo:ib`) et `trade({lp=la(self.lp,{np}),rl=la(self.rl,{jc}),qe=self.qe})` (l. 607, `trade:kg`). `gb` (l. 1434-1441) : `for ri in all(fd) do if type(ri)=="string" then rr=ri else ri.rr=rr add(gb,hp(ri)) end end`, et `hp:pq` (l. 1403) fait `ho(gb,hp.mr,hp.gp)(self)` : les positionnels 1-8 prennent les noms de `mr`, ceux que `gp` nomme (`o`, `dm`) passent par `r`. `hy`, `fv`, `pj` ne sont construits que sans clé (`eu`, `_init`).

**Clés de construction par classe** (boucles et `assert` des constructeurs, recoupés avec les appelants par `grep`) :

| Classe | Clés acceptées | Appelants de `src/` (source des clés) | Forme décidée |
|---|---|---|---|
| `hy`, `fv`, `pj` | aucune (`assert` : NULL ou vide) | `cart.c` : tous NULL | pas de paramètre |
| `g_` | `t`, `pn`, `du`, `py` (fixes) | `bg.c` (`bg:ldown`) : NULL ; clés seulement dans les TU | `pico8_g__keys_t` |
| `ie` | `ke` (sorte), `jb` (infobulle), nil ≡ absent | `cart.c` (`eu`) — littérales | `pico8_ie_keys_t` |
| `trade` | `lp`, `rl` (listes), `qe` (nil, fixe ou booléen) | `planet_trade.c` (`jo:ib`), `trade.c` (`trade:kg`) — littérales | `pico8_trade_keys_t` |
| `hp` | positionnels 1-8, `rr`, `ii`, `cw`, `go`, `rn` | `research.c` (`build_gb`) : enregistrements de la vue `pico8_fd()` | `pico8_hp_keys_t` (enregistrement généré + `rr`) |

**Faits qui engagent la forme** :

- **Absent ≡ défaut du membre** pour `ie` (`ke`, `jb` NULL ; une clé présente à nil donne NULL, `ie_take_kind`/`ie_take_jb`), `trade` (`lp`, `rl` NULL), `g_.t` (`pico8_ke_start.t` = 0) : aucun drapeau.
- **`g_.pn`/`du`/`py` ont un défaut non nul** (`pico8_g__start` : `0x30000`, `0x20000`, `0xf5c`) et une clé à 0 le remplace (`test_pico8_g_` l. 1436-1444, `zero_union`) : présence explicite `has_pn`/`has_du`/`has_py`.
- **`trade.qe` est un triple** (`has_qe`, `qe_is_bool`, `qe`) dans le membre ; la clé a la même forme. `jo:ib` passe `kw.qe` s'il existe (fixe), sinon nil ; `trade:kg` passe `self.qe`, ou `false` quand il est nil (**TROU-24**, marqueur au site, gardé).
- **`hp` lit un enregistrement de `fd` augmenté de `rr`** et rien d'autre : `pico8_fd_entry_t` porte déjà les positionnels (`npos`, `px`, `lr`, `fj`, `ja`, `i`, `o`, `il`/`il_false`, `dm`) et `ii`/`go`/`cw`/`rn` ; `rr` est la dernière entrée-titre (`header`) vue par `build_gb`. `o` et `dm` sont des chaînes que `ho` traduit par `r` (`pico8_hp_start.gp` = {`o`, `dm`}) ; toutes les valeurs de `pico8_fd_table` y sont trouvées (sinon `hp_take_fixed` aborterait à chaque init : H10).
- **La copie de travail de `hp`** (`ri`, table ob) et le **`rc` jetable** (`scratch_rc`) n'existent que pour appeler `pico8_ho_apply` sur une table ob ; `gb[px]` est posé à part (`pico8_ob_table_set_name(g_gb, …)`). Les deux disparaissent avec la table de clés.
- **`pico8_ho_apply` n'a qu'un appelant de `src/`, `hp`** ; le test ponctuel l'appelle pour `nb` et `df`. Après D.6c, elle reste **sans appelant de `src/`** (comme `pico8_lq_table_view` après D.6b), fermée avec `collect` (D.7a) ou avec le test ponctuel (D.8).
- **`pico8_research_build_gb(fd)`** : seuls appelants `pico8_data_init` (`g_fd`) et `test_pico8_hp` (`pico8_fd()`, deux fois) ; la vue `fd` n'a pas d'autre lecteur de `src/`. `pico8_fd()` est lue par `test_pico8_hp` (l. 632, 642, 793), `test_pico8_data_init` (l. 27) et le test ponctuel (l. 268).
- **Les TU construisent souvent dans une variable `b` ou `o`**, invisible pour le motif (`test_pico8_g_`, `_ui`, `_fv`, `_pj`, `_hy`, `_hp`, `_trade`, `_os`) : l'empreinte réelle dépasse celle de la ligne.

**`overrides` de D.6c, par fichier** (lignes / occurrences ; numéros à `58e39e3`, texte inchangé depuis `09f0f56` ; chunks définis plus bas ; classe de la section « Anti-tassement ») :

| Fichier | Lignes / occ. | Chunk | Lignes (n° à `58e39e3`) et classe |
|---|---|---|---|
| `pico8_cart.c` | 6 / 6 | 1 | 218 déclaration, 223 allocation, 224-225 poses `ke`/`jb`, 227 libération : (a) ; 226 appel `pico8_ie_new(overrides)` : (a) (l'argument devient `&keys`) |
| `pico8_economy.c` | 4 / 5 | 1 | 92, 166, 167 signatures et relais : (b) ; 113 `assert` NULL ou vide : (a) |
| `pico8_economy.h` | 1 / 1 | 1 | 53 déclaration : (b) |
| `pico8_help.c` | 4 / 5 | 1 | 206, 280, 281 : (b) ; 236 `assert` : (a) |
| `pico8_help.h` | 2 / 2 | 1 | 47 contrat, 50 déclaration : (b) |
| `pico8_score_panel.c` | 4 / 5 | 1 | 416, 481, 482 : (b) ; 430 `assert` : (a) |
| `pico8_score_panel.h` | 2 / 2 | 1 | 69 contrat, 72 déclaration : (b) |
| `pico8_probe.c` | 6 / 7 | 1 | 698, 765, 766 : (b) ; 705 boucle, 706-707 lecture d'entrée : (a) |
| `pico8_probe.h` | 2 / 2 | 1 | 75 contrat, 79 déclaration : (b) |
| `pico8_research_icon.c` | 7 / 8 | 1 | 414, 474, 475 : (b) ; 430 boucle, 431 clé, 438, 440 lectures : (a) |
| `pico8_research_icon.h` | 2 / 2 | 1 | 63 contrat, 66 déclaration : (b) |
| `pico8_trade.c` | 13 / 14 | 1 | 63, 134, 135 : (b) ; 77-79 boucle et entrée, 172 déclaration, 205-208 allocation et poses, 210 appel, 211 libération : (a) |
| `pico8_trade.h` | 2 / 2 | 1 | 10 commentaire de la struct (« the REFs the overrides held ») : (b) ; 38 déclaration : (b) |
| `pico8_planet_trade.c` | 7 / 7 | 1 | 231 déclaration, 248-251 allocation et poses, 253 appel, 254 libération : (a) |
| `test_pico8_ie` | 10 / 10 | 1 | 276-282 (`build_icon`) : (a) ; 669-674 (`build_with_ra`, test de mort retiré) : (a) |
| `test_pico8_entity_kind` | 9 / 9 | 1 | 317-320 (icônes de `test_ie_ke_view_by_name`), 495-505 (`ie_with`) : (a) |
| `test_pico8_probe_mm_rnd_order` | 7 / 7 | 1 | 29-37 (`new_probe`) : (a) |
| `pico8_research.c` | 8 / 9 | 2 | 293 commentaire de la copie de travail, 334-341 boucle, clé, poses dans `ri` : (a) ; 295, 440, 441 : (b) |
| `pico8_research.h` | 1 / 1 | 2 | 77 déclaration : (b) |
| **Total D.6c** | **97 / 104** (19 fichiers) | 1 : 88 / 94 ; 2 : 9 / 10 | (a) 64 lignes, (b) 33 lignes, (c) 0 |

(Décompte (a)/(b) : (b) = 3 lignes par constructeur `.c` × 7 + les 12 lignes d'en-tête = 33 ; (a) = le reste, 64. L'implémenteur refait ce décompte sur le diff réel.)

**Lignes `overrides` après D.6c** : les 21 lignes / 25 occurrences des quatre ponts de D.6b (`pico8_layout.c` 9 / 11, `pico8_layout.h` 2 / 2, `pico8_ui_button.c` 4 / 5, `pico8_ui_button.h` 1 / 1, `pico8_msgbox.c` 4 / 5, `pico8_msgbox.h` 1 / 1) et les 9 lignes / 9 occurrences de leurs TU (`test_pico8_mh` 7 / 7 : l. 463, 469-471, 493, 495-496 ; `test_pico8_lq` 2 / 2 : commentaires l. 258, 281), **intouchées** : 30 / 34 / 8.

## Solution

**Deux commits (chunks 1 et 2), sans aucun pixel changé** :

- **Classes sans clé** (`hy`, `fv`, `pj`) : `pico8_X_new(void)`. La seule table admise était NULL ou vide ; plus de paramètre, plus d'`assert` ; les messages d'`assert` qui nomment `pico8_X_new(NULL)` nomment `pico8_X_new()`.
- **Clés typées comme les membres qu'elles remplissent**, NULL = aucune clé, lues jamais gardées : `pico8_g__keys_t`, `pico8_ie_keys_t`, `pico8_trade_keys_t`, `pico8_hp_keys_t`.
- **Appelants à clés littérales** (`eu` de `cart.c`, `jo:ib` de `planet_trade.c`, `trade:kg`) : struct sur la pile, plus aucune table allouée.
- **`hp`** prend l'enregistrement généré `const pico8_fd_entry_t *` et `rr` ; il applique `ho` aux champs typés (noms de `mr`, traduction par `r` des membres que `gp` nomme) ; la copie de travail `ri` et le `rc` jetable disparaissent. **`build_gb`** parcourt `pico8_fd_table`.
- **Pont `pico8_fd()` fermé** : `g_fd`, `data_init_fd_view`, `pico8_fd()` et leur contrat disparaissent de `data_init.c/.h`.
- **TU** : chaque table de clés d'un constructeur typé devient une struct locale écrite à son site ; les tests de mort qu'un type rend impossibles à écrire disparaissent, listés au commit avec leur raison.
- **`overrides` = 30**, les quatre ponts de D.6b et leurs TU, intouchés ; **`DATA_RESIDUE_GUARD` inchangé** (`spec`) ; `overrides` y entre à l'étape qui ferme le dernier pont (D.7b dans l'ordre actuel).

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert ; `make e2e` : 26 conformes ; `make sanitize-net` : 26 propres ; `make class-coverage-net` : identique ; `make data-guard` : code 0 ;
- `make data-residue` : `overrides` = **30** lignes / **34** occurrences / **8** fichiers, `spec` 0, `stash` 28/4, `ob`, `valeur` et `type-tag` strictement plus bas, `par-nom` = départ − 24 (écart ≤ 3 expliqué) ; comptes en lignes ET en occurrences, avant et après chaque chunk, par catégorie et par fichier ;
- ces `grep` ne rendent rien :

  ```sh
  grep -rnE 'pico8_(g_|fv|ie|hp|hy|trade|pj)_(struct_)?new\((const )?pico8_ob_table_t' src tests --include='*.[ch]'
  grep -rnE 'pico8_(hy|fv|pj)_new\([^)]' src tests --include='*.[ch]'
  grep -rnE 'pico8_fd\(\)|data_init_fd_view|\bg_fd\b' src tests --include='*.[ch]'
  grep -rnwE 'g__take_fixed|ie_take_(kind|jb)|hp_take_(string|fixed|il)|trade_list_key|hp_accepts_key' src
  grep -nE 'pico8_ob_table_new' src/pico8_cart.c src/pico8_planet_trade.c src/pico8_trade.c
  grep -nE 'scratch_rc|pico8_ho_apply' src/pico8_research.c
  ```
- `grep -rnw overrides src tests --include='*.[ch]'` ne rend que les 21 lignes des quatre ponts (`pico8_lq_new_table`, `pico8_jb_new_table`, `pico8_js_new_table`, `pico8_mh_new_table` et leurs quatre déclarations) et les 9 lignes de `test_pico8_mh`/`test_pico8_lq`, au texte du départ ;
- `grep -rn 'pico8_jb_row_view\|pico8_ob_table_new' src` : aucun site ajouté.

## User Stories

1. En tant que mainteneur du port, je veux que les sept derniers constructeurs à table de clés prennent une struct typée ou rien, afin qu'aucune icône de recherche, route, sonde ni projet ne soit plus construit en allouant, remplissant par nom et libérant une table ob.
2. En tant que mainteneur, je veux que `hy`, `fv` et `pj`, qui n'acceptaient qu'une table vide, n'aient plus de paramètre, afin que le compilateur porte le contrat « aucune clé ».
3. En tant que gardien du comportement, je veux que `ie` reçoive `ke` et `jb` typés, absents ≡ NULL, afin que les sept icônes de `eu` soient identiques.
4. En tant que gardien du comportement, je veux que `trade` reçoive `lp`, `rl` et le triple `qe` du membre, afin que les routes de `jo:ib` et de `trade:kg` aient le même `qe` (nil, fixe ou booléen) qu'avant.
5. En tant que gardien de TROU-24, je veux que `trade:kg` donne toujours `false` à la route neuve quand `self.qe` est nil, au même site marqué, afin que l'écart connu reste préservé à l'identique.
6. En tant que gardien du comportement, je veux que `g_` distingue une clé `pn`/`du`/`py` à 0 d'une clé absente, afin que la sonde à clés du TU `probe_mm_rnd_order` et celle à zéros de `test_pico8_g_` gardent leurs membres.
7. En tant que mainteneur, je veux que `hp` prenne l'enregistrement généré de `fd` et son `rr`, afin que `gb` soit construit sans vue ob, sans copie de travail ni `rc` jetable par projet.
8. En tant que gardien du comportement, je veux que `hp` nomme ses positionnels par `mr` et traduise par `r` ceux que `gp` nomme, comme `ho(gb,hp.mr,hp.gp)`, afin que `o` et `dm` gardent leurs valeurs.
9. En tant que gardien du comportement, je veux que `il` distingue absent, faux et chaîne, et que `ii`/`rn` gardent leur présence, afin que les lignes d'infobulle et la rangée `lq` des projets soient identiques.
10. En tant que mainteneur, je veux que `build_gb` parcoure `pico8_fd_table` et que le pont `pico8_fd()` disparaisse, afin que `data_init` ne construise plus cette vue à chaque init.
11. En tant que gardien du compteur, je veux qu'`overrides` tombe à 30 lignes, exactement les quatre ponts de D.6b et les TU qui leur passent une table, afin que la seule lecture par nom de clés qui reste soit celle des rangées reçues de producteurs non typés.
12. En tant que gardien du garde, je veux que `DATA_RESIDUE_GUARD` ne change pas et que l'étape qui y fera entrer `overrides` soit nommée, afin qu'aucun garde ne soit posé sur un compte non nul.
13. En tant que gardien du compteur, je veux qu'aucune des 97 lignes retirées ne soit un renommage à type constant, afin que la baisse ne contienne aucune ligne déplacée sous un autre nom.
14. En tant que gardien du compteur, je veux les comptes avant/après chaque chunk en lignes ET en occurrences, par catégorie et par fichier, afin qu'une baisse prouve une construction retirée.
15. En tant que gardien du compteur, je veux que l'écart entre la ligne (« = 0 ») et le compte réel (30) soit expliqué par la note, afin qu'il ne soit pris ni pour un échec ni pour un tassement.
16. En tant que gardien des TU, je veux que chaque table de clés d'un constructeur typé devienne une struct locale écrite à son site, sans registre ni aide qui reconstruise une table, afin que le test dise ce qu'il construit.
17. En tant que gardien des TU, je veux que les aides existantes (`build_icon`, `ie_with`, `new_probe`, `keyed`, `route`, `trade_of`, `export_to`) soient retypées sans cas ajouté, afin que les TU gardent leur forme.
18. En tant que gardien des TU, je veux que les tests de mort devenus impossibles à écrire disparaissent avec leur raison au commit, afin qu'aucune vérité ne se perde sans être dite.
19. En tant que gardien des TU, je veux qu'aucune constante `*_HASH` ne bouge, afin de prouver que les instances construites sont les mêmes.
20. En tant que gardien du test ponctuel, je veux qu'il compare `pico8_fd_table` entrée par entrée à `decode(mem, 8494)`, sans le pont, afin que la donnée générée reste prouvée égale à celle du cart.
21. En tant que gardien des filets, je veux `make e2e` à 26, `make sanitize-net` à 26 propres et `make class-coverage-net` identique à chaque chunk, afin de prouver qu'aucun pixel, accès mémoire ni instanciation n'a changé.
22. En tant que gardien de la mémoire, je veux que D.6c n'ajoute aucune table ob ni vue construite à l'appel, à la frame ou à l'init, afin que le tas ne porte pas plus de tables qu'avant.
23. En tant que lecteur de `docs/glossaire-noms.md`, je veux les entrées des quatre structs de clés et de `pico8_ho_gp_has`, et la ligne `fd` sans vue, afin de retrouver les clés du cart depuis leur nom.
24. En tant que `plan-pilot`, je veux une note « Découvert à D.6c » qui donne le compte réel, le report du garde et l'étape qui le posera, le pont `fd` fermé et `pico8_ho_apply` sans appelant de `src/`, afin que D.7a et D.7b partent de faits.
25. En tant que `plan-pilot`, je veux la liste de ce qui dépend de D.6b et de ce qu'il faut relire contre sa note, afin de ne pas lancer D.6c sur une hypothèse démentie.
26. En tant que `plan-pilot`, je veux que les ponts de D.6b, les producteurs de rangées ob et `g_gb` restent hors de ce commit, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Convention (G)

Celle de D.6a et D.6b : type `pico8_X_keys_t`, paramètre `const pico8_X_keys_t *keys`, NULL = aucune clé ; la struct est lue, jamais gardée ; chaînes, listes, vecteurs et structs qu'elle pointe sont empruntés par l'instance sans copie, comme aujourd'hui. Classe sans clé : `pico8_X_new(void)`. Chaque clé a le type du membre qu'elle remplit ; un drapeau de présence n'existe que là où le membre en a un ou a un défaut non nul. Aucun nom nouveau ne correspond à un motif figé (`_default`, `_spec`, `_defaults`, `overrides`, `_take_…(`, `…field…`).

### Forme des clés, par classe (G)

Forme de décision (types seulement pour la précision ; l'implémenteur reprend les types exacts des membres) :

```c
/* pico8_probe.h */
typedef struct {
    pico8_fixed_t t;       /* ke.t; its default is 0, so absent == 0 */
    int has_pn;
    pico8_fixed_t pn;
    int has_du;
    pico8_fixed_t du;
    int has_py;
    pico8_fixed_t py;
} pico8_g__keys_t;

/* pico8_research_icon.h */
typedef struct { const pico8_entity_kind_t *ke; pico8_jb_t *jb; } pico8_ie_keys_t;

/* pico8_trade.h */
typedef struct {
    pico8_list_t *lp;
    pico8_list_t *rl;
    int has_qe;            /* the member's triple */
    int qe_is_bool;
    pico8_fixed_t qe;
} pico8_trade_keys_t;

/* pico8_research.h */
typedef struct {
    const pico8_fd_entry_t *ri; /* a record of pico8_fd_table, never a header; NULL: no positional */
    const char *rr;             /* NULL: nil */
} pico8_hp_keys_t;
```

| Classe | Paramètre | Ce que le constructeur pose |
|---|---|---|
| `hy`, `fv`, `pj` | aucun | ses défauts seulement ; `assert` de table vide supprimé |
| `g_` | `const pico8_g__keys_t *keys` | `ke.t` ; `pn`/`du`/`py` sous leurs drapeaux ; puis `f_` et `pq` comme aujourd'hui |
| `ie` | `const pico8_ie_keys_t *keys` | `ke`, `jb` (NULL = nil) ; puis `pq` |
| `trade` | `const pico8_trade_keys_t *keys` | `lp`, `rl`, et le triple `qe` recopié ; `assert` à message : `qe_is_bool` ⇒ `qe` ∈ {0, 1}, `has_qe` nul ⇒ `qe_is_bool` et `qe` nuls |
| `hp` | `const pico8_hp_keys_t *keys` | `ho` appliqué à l'enregistrement (plus bas), `rr`, puis `gb[px]`, `lq` de `ii`, `i` par `fi` comme aujourd'hui |

- **`hp` et `ho`, sur l'enregistrement typé** : pour chaque position `p` < `ri->npos`, le membre nommé `self->mr->items[p]` prend le champ de rang `p` de l'enregistrement (`px`, `lr`, `fj` en fixe, `ja`, `i`, `o`, `il` — faux si `il_false` —, `dm`) ; un membre que `self->gp` nomme passe par `r` (`pico8_r_lookup`), comme `r[ri[i]]`. Le membre de chaque nom est posé directement (la table des noms `mr` reste celle de la classe, l'ordre des positions est celui de `mr`) ; `ii`/`rn` sous `has_ii`/`has_rn`, `cw`/`go` si non NULL, `rr` de la clé. Deux `assert` à message remplacent ceux des aides mortes : `ri->header` NULL (une entrée-titre n'est pas un projet) ; une valeur que `gp` traduit est trouvée dans `r` (sinon elle ne tient pas dans le membre fixe). L'`assert` « `ii` sans `px` » reste.
- **`pico8_ho_gp_has`** (`collect.c/.h`) : l'aide statique `ho_gp_has` devient publique sous ce nom, sans changer de corps ; `pico8_ho_apply` et `hp` l'appellent. **Rejetés** : dupliquer la boucle dans `research.c` (deux copies d'une règle du cart) ; écrire en dur « `o` et `dm` passent par `r` » (la classe porte `gp`, le port lit `gp`, une mutation de `gp` doit se voir).
- **`build_gb`** : `pico8_research_build_gb(void)` parcourt `pico8_fd_table` ; une entrée-titre pose le `rr` courant (`g_rr`), un enregistrement construit un `hp` sur `{ .ri = e, .rr = g_rr }` et l'ajoute à `gb` à l'indice suivant. `gb` reste la table ob `g_gb` (D.7a, `research`). `pico8_research_positional_values` meurt (seul appelant). Le contrat d'en-tête ne parle plus d'un `fd` reçu ni de l'ordre de libération de ses chaînes (elles sont statiques).
- **Pourquoi `hp` prend `pico8_fd_entry_t`** : les clés d'un `hp` sont exactement un enregistrement de `fd` plus `rr`, et ce type est déjà la forme générée de ces enregistrements (D.2). Une struct à treize champs recopierait l'enregistrement champ par champ, et chaque appelant (un seul, `build_gb`) ferait cette copie ; précédent : `jb` sur `pico8_jb_row_t` (D.6b).
- **Rejetés** :
  - **garder `pico8_ob_table_t` et renommer le paramètre** — renommage à type constant, le tassement interdit ;
  - **retirer les clés de `g_` et faire écrire les membres par le TU après construction** — `pq` et `f_` lisent l'instance à la construction : l'ordre changerait, et le chemin (c) de `test_pico8_g_` cesserait d'être testé ;
  - **un `pico8_value_t qe` dans `pico8_trade_keys_t`** — il faudrait le relire par genre dans le constructeur (lecture de valeur étiquetée que le membre a déjà typée) ; les deux appelants ont le triple sous la main ;
  - **une struct de clés à treize champs pour `hp`** — voir plus haut ;
  - **garder `pico8_ho_apply` dans `hp` sur une table construite depuis l'enregistrement** — c'est une vue par construction, interdite ;
  - **`build_gb` paramétré par un tableau et un compte** — un seul tableau existe ; le TU reconstruit `gb` par le même appel ;
  - **une struct pour les classes sans clé** — le type `void` dit « rien » ;
  - **poser `overrides` dans `DATA_RESIDUE_GUARD` avec les ponts en liste blanche** — outil et motifs figés.

### Chunk 1 — `hy`, `fv`, `pj`, `g_`, `ie`, `trade` (G)

- **`hy`, `fv`, `pj`** : `pico8_X_new(void)`, `static void *pico8_X_struct_new(void)` ; l'`assert` « NULL ou vide » et son message disparaissent ; contrats d'en-tête réécrits sans le paramètre (« `overrides` must be NULL or empty: a key asserts » tombe, le reste du contrat demeure). Appelants : `cart.c` (`fv`, `hy`, `pj`), TU.
- **`g_`** : `pico8_g__new(const pico8_g__keys_t *keys)` ; la boucle, `g__take_fixed` (morte, `-Werror`) et l'`assert` « the only construction keys are pn, t, du, py » disparaissent. `bg.c` (`pico8_g__new(NULL)`) ne change pas.
- **`ie`** : `pico8_ie_new(const pico8_ie_keys_t *keys)` ; la boucle, `ie_take_kind`, `ie_take_jb` (mortes) et l'`assert` « the only construction keys are ke and jb » disparaissent ; `ie_take_bool` reste (écrivain `m_`). `eu` (`cart.c`) : `pico8_ie_keys_t keys = { gx[i], jb }` sur la pile, l'`jb` étant celui que `eu` construit déjà sur `&pico8_dr_table[i]`.
- **`trade`** : `pico8_trade_new(const pico8_trade_keys_t *keys)` ; la boucle, `trade_list_key` (morte) et l'`assert` « every construction passes only lp, rl, qe » disparaissent. `jo:ib` : `keys.lp`, `keys.rl` (les listes allouées comme aujourd'hui), `keys.has_qe = kw != NULL && kw->has_qe`, `keys.qe = kw->qe` ; le `pico8_value_t qe` local meurt. `trade:kg` : `lp`, `rl` comme aujourd'hui ; sous `/* TROU-24 */`, `has_qe = 1`, `qe_is_bool = self->has_qe ? self->qe_is_bool : 1`, `qe = self->has_qe ? self->qe : 0` ; `qe_val` meurt. Le commentaire de `pico8_trade_t` (`trade.h` l. 10) dit « the REFs the keys held » ; `/* TROU-7 */` et « Never freed » restent.

### Chunk 2 — `hp`, `build_gb`, pont `fd` (G)

- **`hp`** : `pico8_hp_new(const pico8_hp_keys_t *keys)` ; la boucle, `hp_accepts_key`, `g_hp_named_keys`, `hp_take_string`/`_fixed`/`_il` (mortes), la copie de travail `ri`, `scratch_rc` et l'appel de `pico8_ho_apply` disparaissent ; le commentaire l. 292-294 perd la copie de travail et le `rc` (il garde ce que le code ne dit pas : `i` réécrit après `ho`, `gb[px]` tient la struct) ; `/* TROU-7 */` reste.
- **`pico8_ho_gp_has`** : promotion de l'aide statique de `collect.c`, déclarée dans `collect.h`.
- **`build_gb`** et **pont `pico8_fd()` fermé** : `pico8_research_build_gb(void)` ; `g_fd`, `data_init_fd_view`, `pico8_fd()` et leur contrat supprimés ; `pico8_data_init` appelle `pico8_research_init()` puis `pico8_research_build_gb()` ; le commentaire de `pico8_data_init` (« fd and nb are built ») ne parle plus de `fd`.

### `Makefile`, docs

- **Le `Makefile` ne change pas** : aucun fichier nouveau, `DATA_RESIDUE_GUARD` reste `spec`. `tools/` ne change pas.
- `docs/glossaire-noms.md` : entrées `pico8_g__keys_t`, `pico8_ie_keys_t`, `pico8_trade_keys_t` (chunk 1), `pico8_hp_keys_t` (`ri`, `rr`) et `pico8_ho_gp_has` (chunk 2) ; ligne `fd` : « `pico8_fd_table` » seul (la vue disparaît) ; ligne `gb` inchangée.
- `docs/trous-de-portage.md` : aucune entrée ne change (TROU-24 décrit le cart et la struct, pas la table de clés ; TROU-7 inchangé).
- `docs/plan-3.md` : note « Découvert à D.6c » en fin de ligne D.6c (texte dans Further Notes). Rien d'autre dans le plan.

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucun ordre de construction (`g_` : clés, puis `f_`, puis `pq` ; `hp` : membres, puis `gb[px]`, `lq`, `i`), aucun tirage du RNG, aucune valeur.
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements ; aucun e2e ajouté.
- Les 21 lignes des quatre ponts de D.6b et les 9 de leurs TU (même texte), leurs appelants, les producteurs de rangées ob, `g_gb` et `pico8_gb()`, `pico8_nb()`, `pico8_ho_apply` (corps inchangé, sauf l'appel de `pico8_ho_gp_has`), les écrivains par nom (`set_field`, `ie_take_bool`, `g__take_target`/`_bool`), `pico8_ob.c/.h`, `tools/`, `KNOWN[]`, les tests du §6.
- **TROU** : aucun écart port/cart corrigé ni ajouté ; TROU-7 et TROU-24 préservés à leurs sites.

### Anti-tassement (liant)

Le compteur compte des **lignes**, et le motif `overrides` est un **nom de paramètre**. Une baisse obtenue sans retirer la construction comptée est un échec.

- **Interdit** : renommer un paramètre, une variable ou un commentaire `overrides` **sans que le type de la donnée change** — c'est le cas des 30 lignes des ponts et de leurs TU ; joindre sur une ligne des déclarations, instructions, arguments, conditions, champs ou entrées d'initialiseur qui étaient sur des lignes distinctes ; retirer des retours à la ligne dans une ligne comptée ; macro, `typedef` ou fonction enveloppe dont le seul rôle est de renommer un identifiant compté ; **un registre dans un TU** (tableau, table, `switch` ou boucle nouveaux qui nomment ensemble classes, clés ou valeurs attendues ; les aides existantes sont retypées, pas étendues) ; **une aide de TU qui reconstruit une table de clés par nom** pour un constructeur typé ; **un alias** (`#define`, pointeur de fonction, variable) qui ferait passer une table ob à un constructeur typé ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; supprimer un commentaire qui ne porte pas sur une clé, une boucle, une aide ou un contrat retiré par D.6c ; **toute vue ob nouvelle construite à l'appel, à la frame ou à l'init**, toute `pico8_ob_table_new` ou tout appel à `pico8_jb_row_view` ajouté dans `src/`.
- **Légitimité, contrôle exigé** : chacune des **97 lignes / 104 occurrences** `overrides` retirées est, au commit de son chunk, classée (a) construction supprimée (allocation, pose, libération, boucle, lecture d'entrée, `assert` de table vide, variable ou commentaire d'une table de clés qui n'existe plus), ou (b) signature, déclaration ou commentaire de contrat dont le paramètre passe à une struct typée ou à rien ; aucune ligne (c) « même type, autre nom ». Recette : `git diff -U0` de chaque fichier, lignes `-` qui portent `overrides` en regard des lignes `+` ; aucune ligne `+` ne porte `pico8_ob_table_t` dans une signature d'un des sept constructeurs, ni `overrides`.
- **Mesure exigée, avant et après chaque chunk** : par catégorie, **lignes** (`make data-residue`, et `--detail` par fichier) **ET occurrences** (recette ci-dessous), le test ponctuel donné à part. Recette figée pour l'implémenteur et le vérificateur (`$S` : dossier du scratchpad) :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  X=$(sed -n "s/^SPEC_EXCLUSION='\(.*\)'\$/\1/p" tools/data_residue.sh)
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E "$X" "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  ```

  Mesuré sur `58e39e3` (départ) : `valeur` 6438, `ob` 1625, `stash` 28, `type-tag` 654, `ref-audit` 69, `par-nom` 1117, `dispatch` 201, `overrides` 138, `spec` 0.
- **Contrôle** : pour chaque catégorie et chaque fichier du diff, Δlignes et Δoccurrences. **Si Δlignes > Δoccurrences** dans un couple (catégorie, fichier), le vérificateur lit le diff de ce fichier : chaque ligne comptée retirée correspond à une construction retirée ou convertie, pas à une construction qui réapparaît sur une autre ligne. Le message de commit liste ces couples et leur justification.
- **Sites hors périmètre** : les 30 lignes des ponts et de leurs TU ont, par fichier, le même compte (lignes et occurrences) et le même texte avant et après (`git diff` vide sur ces lignes).
- **Lignes qui gagnent une catégorie**, listées au commit ; attendues : **aucune**. (Les structs de clés n'ont aucun champ `pico8_value_t` ni fonction ; `pico8_ho_gp_has` n'a aucun motif.) Une ligne qui en gagne une est expliquée au commit ou retirée.

### Empreinte remesurée (G)

| Chunk | `src/` | `tests/` | docs | Total |
|---|---|---|---|---|
| 1 | `economy.c/.h`, `help.c/.h`, `score_panel.c/.h`, `probe.c/.h`, `research_icon.c/.h`, `cart.c`, `trade.c/.h`, `planet_trade.c` (14) | `hy`, `fv`, `pj`, `g_`, `ui`, `jc`, `spec_defaults`, `probe_mm_rnd_order`, `ie`, `entity_kind`, `trade`, `os` (12) | glossaire (1) | 27 |
| 2 | `research.c/.h`, `collect.c/.h`, `data_init.c/.h` (6) | `hp`, `map_tables` (test ponctuel), `data_init` (3) | glossaire, `plan-3.md` (2) | 11 |
| **Distincts** | **20** | **15** | **2** | **37** |

- **Écart avec la ligne (21 : 16, 5)** : la ligne comptait les fichiers qui portent le mot, ceux de D.6b compris. Elle ne voyait pas les appelants à `NULL` des classes sans clé (`test_pico8_ui`, `_jc`, `_g_`, `_fv`, `_pj`, `_hy`, `_spec_defaults`), les TU qui construisent dans `b` ou `o` (`_g_`, `_ui`, `_hp`, `_trade`, `_os`), ni le pont `fd` (`data_init.c/.h`, `collect.c/.h`, `test_pico8_data_init`, le test ponctuel) ; elle comptait des fichiers de D.6b.
- **Découpage (G) : deux commits**, 37 chemins, sous le seuil de ~40 mais de risques distincts : le chunk 1 ne touche que des constructeurs et leurs appelants littéraux ; le chunk 2 change le chemin de construction de `gb` à l'init, ferme un pont de données et touche le test ponctuel. Chacun vert seul, dans cet ordre (2 ne dépend de 1 que par la convention). **Rejetés** : **un commit** (un rouge de `hp` bloquerait six classes triviales) ; **un commit par classe** (sept commits pour des diffs de quelques lignes, le glossaire et les TU partagés — `test_pico8_ui` construit `g_`, `fv`, `pj` — touchés plusieurs fois) ; **`hp` d'abord** (aucun gain : les deux chunks sont indépendants).

### Effet sur `make data-residue`

Attendu, lignes / occurrences / fichiers (départ : `58e39e3`) :

| Catégorie | Départ | Après 1 | Après 2 | Pourquoi |
|---|---|---|---|---|
| `overrides` | 127 ; 138 ; 27 | **39 ; 44 ; 10** | **30 ; 34 ; 8**, exact | inventaire par fichier ; restent les quatre ponts et leurs TU |
| `spec` | 0 | 0 | **0** | garde |
| `par-nom` | 1062 | **1053** | **1038** (écart ≤ 3 expliqué) | `g__take_fixed` (définition, 4 usages), `ie_take_kind`/`ie_take_jb` (2 définitions, 2 usages) ; `hp_take_string`/`_fixed`/`_il` (3 définitions, 12 usages) |
| `ob` | rapporté | **plus bas** | **strictement plus bas** | tables des appelants et des TU, boucles des constructeurs ; copie de travail, `rc`, vue `fd`, `pico8_fd()`, `build_gb(fd)`, `pico8_research_positional_values` |
| `valeur` | rapporté | plus bas | **strictement plus bas** | poses `pico8_value_*` des tables retirées, `qe`/`qe_val` locaux, lectures d'entrée ; vue `fd` ; aucun gain |
| `type-tag` | rapporté | plus bas | **≤** | `assert` de tag de `ie_take_kind`/`ie_take_jb`/`trade_list_key` |
| `dispatch` | 199 ; 201 | = | **=** | aucun champ fonction dans les clés |
| `ref-audit` | rapporté | = | **=** | |
| `stash` | 28/4 | 28/4 | **28/4** | test ponctuel : 2 lignes inchangées |

- **`overrides` = 30 / 34 / 8 est attendu exact** ; tout écart est expliqué ligne par ligne au commit, et **une baisse sous 30 est un échec** (seuls les ponts et leurs TU restent, et leur texte est gelé). Le « = 0 » de la ligne ne l'est pas, pour la raison dite plus haut ; la note corrige.
- Aucune catégorie ne tombe à 0 : `DATA_RESIDUE_GUARD` ne change pas.

### Hypothèses sur D.6c (rien n'a été lancé)

- **H1** : l'arbre de départ est vert et `make data-residue` y rend l'inventaire. Démenti : arrêt, D.6c ne répare rien.
- **H2** : aucun appelant des sept constructeurs hors de l'inventaire (grep sur `58e39e3` : `src/` et les TU listés ; aucun pointeur de fonction vers eux, aucune sorte `pico8_entity_kind_t` pour `g_`, `fv`, `ie`, `hp`, `hy`, `trade`, `pj`).
- **H3** : `pico8_fd_table` porte les mêmes valeurs que la vue `fd` (le test ponctuel le prouve à `58e39e3`) ; chaque `o`/`dm` d'un enregistrement est une clé de `r`.
- **H4** : `hp` construit sur `{ri, rr}` pose, pour chacun des 34 enregistrements, les mêmes membres qu'aujourd'hui (présence comprise : `il` absent / faux / chaîne, `dm` seulement si `npos` = 8, `ii`/`rn` sous leurs drapeaux) et les mêmes pointeurs de chaîne (ceux de `pico8_fd_table`, que la vue recopiait sans copie).
- **H5** : aucune constante `*_HASH` (ni son nom) ne bouge ; les tests de mort retirés ne nourrissent aucun hachage. Démentie : arrêt sur la constante, jamais de mise à jour.
- **H6** : `make class-coverage-net` est identique : aucune instanciation ajoutée ni retirée (une `hp` par enregistrement, une `ie` par sorte, une `trade` par proposition et par `kg`).
- **H7** : `make sanitize-net` rend 26 propres : les tables de clés passent du tas à la pile, la vue `fd`, la copie de travail et le `rc` disparaissent, rien d'autre ne change de propriétaire (les listes `lp`/`rl` restent allouées et jamais libérées, TROU-7).
- **H8** : `-Werror` reste muet : aides mortes supprimées (`g__take_fixed`, `ie_take_kind`, `ie_take_jb`, `trade_list_key`, `hp_accepts_key`, `hp_take_*`, `g_hp_named_keys`, `pico8_research_positional_values`, `value_is_nil` si elle n'a plus d'usage), inclusions devenues inutiles retirées (`pico8_ob.h` là où plus rien ne l'emploie).
- **H9** : `constants-check`, `float-grep`, `float-guard` restent verts : aucun flottant ni `PICO8_FIXED_LIT` ajouté.
- **H10** : aucune valeur `o`/`dm` de `pico8_fd_table` ne manque dans `r` (sinon l'init aborterait déjà à `58e39e3`).
- **H11** : durées courtes : `make test` en quelques secondes plus une recompilation large ; `make e2e` ~1 s ; `sanitize-net` et `class-coverage-net` < 2 min chacun, par chunk.
- **H12** : les 26 e2e atteignent `eu`, `data_init`/`build_gb` et `jo:ib` ; `trade:kg` est prouvé au moins par `test_pico8_trade` ; un site non atteint est prouvé par un TU.
- **H13** : `test_pico8_hp` et le test ponctuel peuvent comparer `gb` et `pico8_fd_table` sans vue : les pointeurs de chaîne qu'ils testent sont ceux de la table générée.
- **H14** : la promotion de `ho_gp_has` en `pico8_ho_gp_has` ne change aucun comportement de `pico8_ho_apply` (test ponctuel vert).
- **H15** : `make raylib` (non exigé, jamais lancé) lierait : aucun fichier nouveau.

## Implementation Chunks

Deux commits, dans cet ordre, chacun vert seul (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`, `make data-guard`), chacun avec ses comptes avant/après, sa classification et son rapport gcov/mutations :

1. **1 — `hy`, `fv`, `pj`, `g_`, `ie`, `trade`** : trois classes sans paramètre, trois structs de clés, appelants littéraux (`eu`, `jo:ib`, `trade:kg`), aides mortes, TU ; glossaire (trois entrées). `overrides` → 39 / 44 / 10.
2. **2 — `hp`, `build_gb`, pont `fd`** : `pico8_hp_keys_t`, `ho` sur l'enregistrement typé, `pico8_ho_gp_has`, `build_gb(void)`, fermeture de `pico8_fd()`, test ponctuel, `test_pico8_hp`, `test_pico8_data_init` ; glossaire, note du plan. `overrides` → 30 / 34 / 8.

Ordre conseillé dans chaque commit : mesure de départ (lignes et occurrences, par fichier) ; types et constructeurs ; appelants de `src/` ; `make test` ; TU ; `make test` ; `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois ; gcov, mutations ; mesure d'arrivée, classification, anti-tassement, gardes ; docs.

## Testing Decisions

Un bon test juge ce que le code rend (instances, valeurs, identités, pixels), pas la présence d'une ligne. D.6c change la forme des clés, pas ce qu'elles posent : **la preuve est que chaque constructeur pose, pour les mêmes clés, les mêmes membres qu'avant, que `gb` construit depuis `pico8_fd_table` est le même que depuis sa vue, et que tout ce qui mesure le comportement rend la même chose.**

### TU touchés : clés en struct locale, mêmes constantes (G)

- **Règle** : chaque table (`b`, `o`, `overrides`…) passée à un constructeur typé devient une struct locale initialisée à son site, un champ par ligne ; plus de `pico8_ob_table_new`/`set_*`/`free` pour ces constructions ; aucune aide nouvelle qui construise des clés par nom. Les aides existantes sont retypées, sans cas ajouté : `build_icon` (`test_pico8_ie`), `ie_with` (`test_pico8_entity_kind` : ses drapeaux `set_ke`/`set_jb` et ses `pico8_value_t` deviennent deux pointeurs ; l'appel « présent à nil » l. 548 devient identique à l'appel « absent » l. 547 — la vérité qu'il portait, nil ≡ absent, est portée par le type, dit au commit), `new_probe` (`probe_mm_rnd_order`), `keyed` (`test_pico8_g_`), `route`, `trade_of` (`test_pico8_trade` : le `qe` passé en valeur devient le triple), `export_to` (`test_pico8_os`).
- **Classes sans clé** : chaque `pico8_hy_new(NULL)`, `pico8_fv_new(NULL)`, `pico8_pj_new(NULL)` des TU devient `pico8_X_new()` ; les commentaires de TU qui nomment `pico8_fv_new(NULL)`/`pico8_pj_new(NULL)` (`test_pico8_fv` l. 2 et 764, `test_pico8_pj` l. 1) les suivent. Les `pico8_g__new(NULL)`, `pico8_ie_new(NULL)`, `pico8_hp_new(NULL)`, `pico8_trade_new(NULL)` ne changent pas.
- **Tests de mort, un par un** (liste au commit) :
  - **retirés, le type les rend impossibles à écrire** : `hy` `build_with_key` ; `fv` `build_with_named_key`, `build_with_positional_key` ; `pj` `build_with_named_key`, `build_with_positional_key` ; `g_` `build_with_unknown_key`, `build_with_positional_key`, la boucle de `must_abort_all(NULL, keys[i], build_with_off_row_key, …)` (clés hors type) et `build_with_off_row_key` ; `test_pico8_ui` `g__with_t_key` sur nil, faux, vrai, chaînes, REF (le cas « fixe 0 » de `test_g__t_key_stops_off_row` reste, en struct : `t` = 0) ; `ie` `build_with_ra` ; `test_pico8_entity_kind` l. 719 et 724 (`ke`/`jb` hors type) ; `hp` `build_with_named_key`, `build_with_positional_9` ; `trade` `build_with_named_key`, `build_with_positional_key` ;
  - **gardés** : les écritures hors rangée par `set_field` (`write_px`, `write_qe`, `must_abort_all` des écrivains de `g_`…) ne changent pas ; l'`assert` « `ii` sans `px` » de `hp` et les deux `assert` nouveaux de `hp` (entrée-titre, valeur absente de `r`) et de `trade` (triple incohérent) sont prouvés par un test de mort chacun, sur une clé locale (un `pico8_fd_entry_t` local pour `hp`) — ce n'est pas un registre : une clé par test.
- **Constantes** : aucune constante `*_HASH` ne change, ni de valeur ni de nom (`KE_T_HASH`, `G__MEMBERS_HASH`, `RAW_PRESENCE_HASH`, `HARVESTED_WORLD_HASH` compris). Toute constante qui bouge est un arrêt.

### `test_pico8_hp`, `test_pico8_data_init` et le test ponctuel (chunk 2, G)

- **`test_pico8_hp`** : `test_rebuild_and_shutdown` appelle `pico8_research_build_gb()` ; `test_hp_members_by_name` parcourt `pico8_fd_table` au lieu de la vue (même filtre : les entrées qui ne sont pas des titres, `rr` = le dernier titre vu) et compare les membres aux champs de l'enregistrement (`record_string` retypée) ; mêmes assertions, même `RAW_PRESENCE_HASH`, mêmes comptes `il`.
- **`test_pico8_data_init`** : `assert(pico8_fd()->count == 42)` devient `PICO8_FD_COUNT == 42` et une assertion sur `pico8_gb()` (ses 34 projets positionnels, `GB_COUNT` de `test_pico8_hp`) — même intention (`data_init` rend `fd` et `gb` prêts), sans le pont.
- **Le test ponctuel** : `test_fd_view_matches_the_decoded_table_plus_rr` devient une comparaison de `pico8_fd_table`, entrée par entrée et champ par champ, à `decode(mem, 8494)` (titre ↔ chaîne ; positionnels 1..`npos` ↔ `px`, `lr`, `fj` fixe, `ja`, `i`, `o`, `il` ou faux, `dm` ; `ii`, `go`, `cw`, `rn` ↔ clés nommées ; nombre de clés égal) ; une aide locale au test rend la valeur de rang `p` d'une entrée (outil de comparaison, pas de table ob construite depuis la table typée). La partie « + `rr` » (la vue ajoutait `rr`) passe à `test_pico8_hp`, qui vérifie déjà `rr` sur chaque projet. Il construit son état, ne rejoue rien, ne nomme aucun motif de `spec`, d'`overrides` ni de `type-tag` ; ses 2 lignes `stash` ne changent pas ; ses appels de `pico8_ho_apply` (`nb`, `df`) restent.

### Les tests du §6

Aucun n'est touché (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) ; aucune preuve rouge n'est due. `ga_export_panel_by` construit ses routes en tables ob (D.7a), sans `pico8_trade_new`.

### Preuve des tests (§7, gcov + mutations)

- **gcov** (copie jetable du scratchpad, `--coverage`), par chunk : chaque ligne ajoutée ou modifiée des constructeurs, de `eu`, `jo:ib`, `trade:kg`, `build_gb`, `pico8_ho_gp_has` et de `data_init` est exécutée par un TU ; celles atteintes seulement par les e2e sont justifiées au commit.
- **Mutations**, chacune doit rougir un test, puis être annulée :
  - 1 — **M1** : `_init` (`cart.c`) n'appelle plus `pico8_fv_new()` → `test_pico8_fv` ou `make e2e` rouge ; **M2** : `g_` ne pose pas `pn` sous `has_pn` → `test_pico8_probe_mm_rnd_order` ou `test_pico8_g_` rouge ; **M3** : `g_` ignore `keys->t` → `test_pico8_ui` (`KE_T_HASH`) rouge ; **M4** : `g_` pose `pn`/`du`/`py` même sans drapeau → `test_pico8_ui` (sonde `zero`, clé `t` seule : ses `pn`/`du`/`py` lus, assertion ajoutée si absente) rouge ; **M5** : `ie` ignore `keys->ke` → `test_pico8_ie` ou `test_pico8_entity_kind` rouge ; **M6** : `ie` ignore `keys->jb` → `test_pico8_ie` rouge ; **M7** : `trade` ignore `qe_is_bool` → `test_pico8_trade` rouge ; **M8** : `trade:kg` pose `has_qe = 0` quand `self.qe` est nil (TROU-24 défait) → `test_pico8_trade` rouge ; **M9** : `jo:ib` passe toujours `has_qe = 0` → `test_pico8_trade`, `test_pico8_os` ou `make e2e` rouge.
  - 2 — **M10** : `hp` ne traduit plus par `r` les membres de `gp` → `test_pico8_hp` rouge ; **M11** : `hp` ignore `il_false` → `test_pico8_hp` (comptes `il`) rouge ; **M12** : `hp` pose `dm` quand `npos` < 8 → `test_pico8_hp` (`RAW_PRESENCE_HASH`) rouge ; **M13** : `build_gb` ne passe pas `rr` → `test_pico8_hp` rouge ; **M14** : `pico8_ho_gp_has` rend toujours 0 → `test_pico8_hp` et le test ponctuel rouges.
- Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée.

### Pas de nouvel e2e (G)

Les lignes changées sont atteintes par les 26 e2e et par les TU (H12) ; un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 25 minutes pour l'étape (≈ 10 par chunk, plus les mesures).**

| Cible (par chunk) | Durée attendue |
|---|---|
| `make test` (×2 : constructeurs et appelants ; TU) | quelques secondes chacun, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H11) |
| `make class-coverage-net` | < 2 min (H11) |
| gcov + mutations (scratchpad) | < 4 min |
| `make data-guard`, `make data-residue`, recette d'occurrences, classification | < 1 min |

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + chunk + « constructeurs » / « appelants » / « TU »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre de chaque commit : `timeout 5m make test` ; `timeout 5m make e2e` : **26 « conforme »** ; `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ; `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ; `make data-guard` : code 0 ; `make data-residue` + recette : `overrides` aux jalons 39/44/10 puis 30/34/8, `spec` 0, `stash` 28/4, `ob` et `valeur` strictement plus bas au dernier chunk, `par-nom` départ − 24 (± 3 expliqué) ; classification et anti-tassement passés.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit liste les chemins de son chunk (« Empreinte remesurée », écart ≤ 3 expliqué), rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements, `Makefile` et `tools/` inchangés, tests du §6 intacts.
2. **Types** : les sept constructeurs prennent une struct de clés ou rien ; aucune recherche par nom d'une clé de construction hors des quatre ponts ; `grep` de la « Sortie de l'étape » vides.
3. **Reste `overrides`** : les 30 lignes restantes sont toutes dans `pico8_lq_new_table`, `pico8_jb_new_table`, `pico8_js_new_table`, `pico8_mh_new_table` ou leurs déclarations (21), ou dans `test_pico8_mh` (7) et `test_pico8_lq` (2), au texte du départ.
4. **Renommage** : les 97 lignes retirées classées (a)/(b), aucune (c).
5. **Mémoire** : aucune `pico8_ob_table_new` ni `pico8_jb_row_view` ajoutée dans `src/` ; les tables de clés de `eu`, `jo:ib`, `trade:kg`, la copie de travail et le `rc` de `hp`, la vue `fd` ont disparu.
6. **Anti-tassement** : comptes avant/après chaque chunk, en lignes ET en occurrences, par catégorie et par fichier, recalculés avec la recette ; chaque couple Δlignes > Δoccurrences justifié ; aucune jonction de lignes, aucun alias, aucun registre de TU, aucun déplacement hors du compteur, aucun commentaire sans rapport supprimé ; aucune ligne ne gagne de catégorie sans être listée.
7. **TU** : plus aucune table de clés pour un constructeur typé ; tests de mort retirés listés avec leur raison ; ceux ajoutés pour les `assert` nouveaux rougissent si l'`assert` est retiré ; aucune constante `*_HASH` changée.
8. **Garde** : `DATA_RESIDUE_GUARD` = `spec` ; la note nomme l'étape où `overrides` y entrera.
9. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun récit de chantier ; aucun renvoi à D.6c ni au plan dans `src/` ou `tests/`.
10. **Docs** : glossaire à jour ; note « Découvert à D.6c » en fin de ligne D.6c, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `58e39e3` (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas `overrides` 127/27 et `spec` 0.
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence, à un des deux commits.

**Périmètre**

- `FAILED:` si un des sept constructeurs prend encore une `pico8_ob_table_t`, si une clé de construction est lue par nom hors des quatre ponts de D.6b, ou si une ligne des `grep` de la « Sortie de l'étape » subsiste.
- `FAILED:` si une ligne d'un pont de D.6b ou de ses TU (`test_pico8_mh`, `test_pico8_lq`) change, si un pont est fermé, renommé ou ajouté, si un producteur de rangées ob est converti, si `g_gb`/`pico8_gb()`, `pico8_nb()`, `pico8_jb_row_view` ou le corps de `pico8_ho_apply` (hors l'appel de `pico8_ho_gp_has`) change, ou si un test du §6 est touché.
- `FAILED:` si le diff ajoute une `pico8_ob_table_new` ou un appel à `pico8_jb_row_view` dans `src/`, ou toute table ou vue construite à chaque appel, à chaque frame ou à l'init.
- `FAILED:` si le diff touche un TROU autrement que par le texte de contrat qui l'entoure (TROU-7, TROU-24 gardés à leurs sites).
- `FAILED:` si le test ponctuel nomme un motif de `spec`, d'`overrides` ou de `type-tag`, construit une table ob depuis `pico8_fd_table`, ou porte plus de 2 lignes `stash`.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` ou `tools/data_residue.sh` change.
- `FAILED:` si `overrides` ne rend pas 30 lignes / 34 occurrences / 8 fichiers (et 39/44/10 après le chunk 1) sans explication ligne par ligne, ou descend sous 30 ; si `stash` n'est pas 28/4, `spec` 0 ; si `par-nom` s'écarte de départ − 24 de plus de 3 ou sans explication ; si `ob` ou `valeur` ne baisse pas strictement.
- `FAILED:` **si une ligne `overrides` disparaît par renommage sans changement de type** (classe (c)), ou si la classification (a)/(b) manque au commit.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés dans une ligne comptée, construction comptée déplacée sur une autre ligne comptée, alias (macro, `typedef`, enveloppe, pointeur) d'un identifiant compté, code déplacé hors de `src`/`tests` ou hors `.c`/`.h`, commentaire sans rapport supprimé.
- `FAILED:` si un TU construit les clés d'un constructeur typé par un registre, une table ou une aide par nom nouvelle.
- `FAILED:` si un commit ne donne pas, par catégorie, les comptes avant/après **en lignes et en occurrences**, si un couple Δlignes > Δoccurrences n'est pas justifié, ou si une ligne gagne une catégorie sans être listée.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` (ou `identity[]`) change.
- `FAILED:` si un test de mort est retiré sans être listé au commit avec sa raison, ou si un `assert` nouveau (`hp`, `trade`) n'a pas son test de mort.
- `FAILED:` si le rapport gcov ou une mutation manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si un message de commit ne porte pas ses comptes avant et après.
- `FAILED:` s'il manque la note « Découvert à D.6c », ou si elle omet le compte `overrides` réel et l'écart avec le « = 0 », l'étape où `overrides` entrera dans `DATA_RESIDUE_GUARD`, le pont `fd` fermé ou `pico8_ho_apply` sans appelant de `src/`.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les quatre ponts de D.6b** (`pico8_lq_new_table`, `pico8_jb_new_table`, `pico8_js_new_table`, `pico8_mh_new_table`) et leurs 21 lignes, avec les 9 lignes de leurs TU (`test_pico8_mh`, `test_pico8_lq`) : ils ferment avec leurs producteurs (D.7a, D.7b) ; `overrides` entre alors dans `DATA_RESIDUE_GUARD`.
- **Les producteurs de rangées ob** (`planet_display`, `research_display`, `score_panel_display`, `menu`, `help`, `pz`) et les vues `ix`/`ha`/`bo`/`bz`, `pico8_nb()` : D.7a/D.7b.
- **`g_gb`, `pico8_gb()`** (table ob des projets), les enregistrements de `jo:ib` (`route_a`/`route_b` de `ga_export_panel_by`), `pico8_ho_apply` (ob, sans appelant de `src/` après D.6c) : D.7a (`research`, `planet_trade`, `collect`) ou D.8 (test ponctuel).
- **Les listes `lp`/`rl` de `trade`** (`pico8_list_t` de `pico8_value_t`) et leur fuite (TROU-7) : point 3 de l'arrêt, D.9.
- **Les écrivains et lecteurs par nom** (`set_field`, `ie_take_bool`, `g__take_target`/`_bool`, `pico8_entity_get_field`) : `par-nom`, D.9.
- Les archétypes, la phase 2 ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **cible** : `overrides` = 30 / 34 / 8 (les quatre ponts de D.6b et leurs TU, « = 30 » de la note de D.6b), pas 0 ; `DATA_RESIDUE_GUARD` inchangé ; `overrides` y entre à l'étape qui ferme le dernier pont (D.7b dans l'ordre actuel), pas de garde partiel.
  - **forme** : `(void)` pour `hy`, `fv`, `pj` ; `pico8_g__keys_t` (`t` sans drapeau, `pn`/`du`/`py` sous drapeau) ; `pico8_ie_keys_t` (deux pointeurs) ; `pico8_trade_keys_t` (listes et triple `qe` du membre) ; `pico8_hp_keys_t` (enregistrement généré `pico8_fd_entry_t` et `rr`), `ho` appliqué aux champs typés avec `pico8_ho_gp_has`.
  - **pont de données** : `pico8_fd()` fermé ; `build_gb(void)` sur `pico8_fd_table`.
  - **tests** : clés en struct locale à leur site, aides existantes retypées ; tests de mort impossibles retirés et listés ; un test de mort par `assert` nouveau ; test ponctuel sur `pico8_fd_table` sans vue ; quatorze mutations ; pas d'e2e ; `class-coverage-net` et `sanitize-net` à chaque chunk ; §6 non touché.
  - **découpage** : deux commits (37 chemins ; `hp` et le pont `fd` à part).
  - **D.6b** : quatorze hypothèses H-D6b écrites sur `09f0f56`, vérifiées sur `58e39e3` ; le départ (127) et la cible (30) corrigés d'après la note de D.6b.
- **Note du plan (G)**, à ajouter en fin de ligne D.6c (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.6c : deux chunks (`hy`/`fv`/`pj`/`g_`/`ie`/`trade`, puis `hp` et le pont `fd` — 37 chemins : appelants à NULL, TU qui construisent dans `b`/`o`, `data_init`, `collect` et le test ponctuel hors du majorant). `hy`, `fv`, `pj` sans paramètre ; clés typées `pico8_g__keys_t`, `pico8_ie_keys_t`, `pico8_trade_keys_t` (triple `qe` du membre ; TROU-24 au site de `trade:kg`), `pico8_hp_keys_t` (`ri` : enregistrement `pico8_fd_entry_t`, `rr`) ; `hp` applique `ho` aux champs typés (`pico8_ho_gp_has`, extraite de `collect.c`). Pont `pico8_fd()` fermé (`pico8_research_build_gb(void)` parcourt `pico8_fd_table`) ; `pico8_ho_apply` sans appelant de `src/` (D.7a `collect`, ou D.8 avec le test ponctuel). `overrides` après D.6c : N (les 21 lignes des quatre ponts de D.6b et les 9 de leurs TU, le « = 30 » de la note de D.6b ; le « = 0 » de D.1 comptait sans eux) ; `overrides` n'entre pas dans `DATA_RESIDUE_GUARD` : il y entre à l'étape qui ferme le dernier pont — `js`, dont les rangées `bs` de `pz` sont à D.7b ; D.7a ferme `lq`, `jb`, `mh` et les autres sources de `js` si son PRD ne réordonne pas. `par-nom` : P.

  N est le compte `overrides` réel, P le compte `par-nom` réel.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget de tests), anti-tassement, classification et reste `overrides` compris.
- **Le message de chaque commit** porte : le périmètre du chunk ; la forme des clés par classe et ses rejets ; la classification (a)/(b) des lignes `overrides` retirées ; les comptes avant et après **en lignes et en occurrences** par catégorie et par fichier, et les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie (aucune attendue) ; les tests de mort retirés, un par un, avec leur raison ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses H qui le concernent.
- **Surprises pour le pilote** :
  - **Le « = 0 » de la ligne est hors d'atteinte par construction** : D.6b a choisi (et justifié) de garder quatre ponts nommés `overrides` pour ne pas typer ses producteurs, et leurs TU passent une table de même nom ; D.6c ne peut les fermer sans faire D.7a/D.7b. Le vrai « = 0 » et l'entrée dans le garde glissent à D.7b.
  - **Le « 97 » de la note de D.6a tient exactement** sur `09f0f56` et sur `58e39e3` (97 lignes / 104 occurrences / 19 fichiers) ; les trois TU `test_pico8_hp`, `_hy`, `_trade`, que la ligne mettait dans l'empreinte de D.6c, ne portent que des lignes de D.6b (`jc`).
  - **`g_` n'a de clés que pour un TU** : le cart ne construit la sonde que sans clé ; `pico8_g__keys_t` sert le chemin (c) de `probe_mm_rnd_order`/`test_pico8_g_`. Le supprimer changerait l'ordre `pq`/clés : gardé.
  - **`hp` coûtait une vue par init et deux tables par projet** (vue `fd`, copie de travail `ri`, `rc` jetable) pour un `ho` dont le résultat était relu champ par champ ; le typage les retire toutes, et `pico8_ho_apply` perd son dernier appelant de `src/`.
  - **L'empreinte de la ligne (21) sous-estime** : 37 chemins, surtout des TU qui appellent `pico8_fv_new(NULL)`/`pico8_pj_new(NULL)` ou construisent dans `b`.
