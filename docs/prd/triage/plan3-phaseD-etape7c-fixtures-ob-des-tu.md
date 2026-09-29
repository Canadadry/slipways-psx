---
title: "Plan 3, étape D.7c : les TU cessent de construire et de lire des tables ob — inclusions mortes retirées, valeurs « hors rangée » et tests « fermés » sans table ob, fixture `by` de `test_pico8_ga` en routes, plis de hachage sans branche ob ; `src/` intact ; trois commits ; `ob` final = 187 + 22 + 1, plus le test ponctuel"
description: "Après D.7b (hypothèses H-D7b), il reste 83 lignes `ob` (87 occurrences) dans 28 TU hors du test ponctuel et des tests de la couche : 27 inclusions de `pico8_ob.h`, des tables ob offertes à des écrivains typés comme valeurs « hors rangée », trois tests « fermés » qui passent une table ob comme `self`, une fixture qui pousse des tables ob dans `ga.by` (qui tient des routes), et des plis de hachage qui gardent une branche ob jamais atteinte ou l'octet de sorte de clé d'une table ob. D.7c les retire sans toucher `src/`, sans changer une constante `*_HASH`, sans qu'un test perde son intention ; un commentaire de test du §6 reste (R_t = 1) ; comptes en lignes ET occurrences, classification (a)/(b)/(c) dans chaque message."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données ») :

> - D.7c — `ob`, TU restants : fixtures construites en `pico8_ob_table_*` → structs, état construit directement. Empreinte 41 (0, 41) aujourd'hui. Compte : `ob` = 187 (`pico8_ob.c` 82, `pico8_ob.h` 32, `test_pico8_ob.c` 55, `pico8_stash.c` 8, `pico8_stash.h` 3, `test_pico8_stash.c` 7). §6 : aucun. `sanitize-net` et `class-coverage-net` : non, aucun fichier de `src/` ne change.

Ligne suivante, qui hérite du reste de D.7c, recopiée :

> - D.8 — suppression : `pico8_ob.c/.h` (avec `pico8_fi`), `pico8_stash.c/.h`, `test_pico8_ob.c` et `test_pico8_stash.c` (§6, tests de la couche), le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, leurs entrées du `Makefile`. Compte : `ob` = 0, `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : non.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » versionnées à `7d05979`, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b, D.5c, D.6a, D.6b, D.6c, D.7a, D.7b ; D.1 n'en a pas). **La note de D.7b est celle du commit `7d05979`, pas encore gradé** : tout ce que ce PRD en tire est une hypothèse `H-D7b-n` (section « Hypothèses »).

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

### Lecture de ces textes pour D.7c

- **Périmètre** : « `ob`, TU restants : fixtures construites en `pico8_ob_table_*` → structs, état construit directement ». La ligne ne vise que `tests/`. Le test ponctuel (`test_pico8_map_tables.c`) est « hors compte jusqu'à D.8, où il est supprimé » (en-tête de la liste) ; `test_pico8_ob.c` et `test_pico8_stash.c` sont des tests de la couche (§6), supprimés à D.8 ; aucun des trois n'est touché.
- **Compte** : la ligne dit « = 187 » ; la note de D.7a l'a fait « 187 + 13 », celle de D.7b « 187 + 22 » (H-D7b-3). Ce PRD le recalcule ligne par ligne (« Cible », plus bas) : **187 + 22 + 1**, le 1 étant la l. 2 de `test_pico8_planet_gh_row_lq.c` (R_t), plus les 199 lignes du test ponctuel, hors compte.
- **« `sanitize-net` et `class-coverage-net` : non, aucun fichier de `src/` ne change »** : confirmé par l'inventaire — aucun des 83 sites ne demande de changer `src/` (graphe d'inclusion, écrivains et gardes lus plus bas). C'est un **critère** : un commit qui toucherait `src/` sort de la ligne ; l'implémenteur **s'arrête et le signale** au lieu de le faire (FAILED).
- **§6 : aucun** : aucun test du §6 n'est réécrit dans sa lecture. Deux d'entre eux perdent une inclusion morte (`gh_row_lq`, `layout_nil_item_draws_nothing`), rien d'autre ; la l. 2 de `gh_row_lq` reste (R_t). Précédent : D.6a a touché la seule l. 2 de `gh_row_lq`, « commentaire seulement ; aucune lecture ne change, pas de preuve rouge ».
- **Leçons des notes à ne pas refaire** : classification (a)/(b)/(c) absente, (c) non déclarée, lignes qui gagnent une catégorie omises, chiffre faux dans un message (correctifs de D.7a) ; un TU qui écrit par une table ob dans une donnée typée (D.7b, `test_second_jl_keeps_specs`) — D.7c en retire les derniers moyens.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; aucune jonction de lignes, aucun alias, aucun registre, aucun renommage pour échapper aux greps ; aucun test ne perd son intention — un test qui vérifiait un comportement de table ob d'un membre désormais typé est **retiré et listé avec sa raison**, jamais réécrit en silence pour vérifier moins.

### État de départ

**Mesuré sur `7d05979`** (branche `claude/plan-pilot-3-7zdft0`, D.7b commitée en `079c3fb`, `d56fff5`, `7d05979`, **en cours de vérification, non gradée**), dans une copie `git archive 7d05979` du scratchpad. **Rien n'a été compilé ni lancé** : seuls `sh tools/data_residue.sh` (avec `--detail`), la recette d'occurrences de D.7b et des `grep`/un parcours du graphe d'inclusion ont tourné.

**`make data-residue`** (lignes / fichiers) et **occurrences** (recette de « Anti-tassement ») :

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 5155 | 131 | 5962 |
| `ob` | 491 | 44 | 550 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 373 | 67 | 592 |
| `ref-audit` | 65 | 16 | 66 |
| `par-nom` | 1009 | 81 | 1064 |
| `dispatch` | 191 | 61 | 193 |
| `overrides` | 0 | 0 | 0 |
| `spec` | 0 | 0 | 0 |
| **résidu D** | **6529 lignes** | **135 fichiers** | — |

**`ob` par groupe** (lignes / occurrences) : `src/` 147 / 185 (couche `pico8_ob.c` 82/104, `pico8_ob.h` 32/46 ; `pico8_stash.c` 8/8, `pico8_stash.h` 3/3 ; R 22/24) ; test ponctuel 199 / 206 ; `test_pico8_ob.c` 55 / 65 ; `test_pico8_stash.c` 7 / 7 ; **autres TU 83 / 87 dans 28 fichiers** — le travail de D.7c. Hors test ponctuel : 292. Ces chiffres sont ceux de la note de D.7b (H-D7b-4).

### Inventaire des 83 lignes (mesuré sur `7d05979`)

Quatre familles, et un reste.

**F1 — inclusion morte de `pico8_ob.h` (27 lignes, 27 occurrences, une par TU sauf `test_pico8_map_tables`).** Parcours du graphe d'inclusion (`#include "…"`, récursif, `pico8_ob.h` retiré) : chacun des 28 TU atteint encore `pico8_list.h`, `pico8_type_tag.h` et `pico8_value.h` par un autre chemin (`pico8_struct.h`, `pico8_list.h`…). **Aucune inclusion à ajouter** (H1). Quatorze TU n'ont que cette ligne-là : `bg` (29), `fm` (30), `fq` (28), `fr` (32), `iv` (31), `je` (22), `jo` (30), `layout_nil_item_draws_nothing` (34, §6), `planet_gh_row_lq` (34, §6), `planet_kj_dispatch` (13), `nr` (31), `os` (30), `qh` (29), `trade` (34). Les treize autres perdent la leur avec le dernier usage du fichier.

**F2 — une table ob offerte comme valeur « hors rangée » à un écrivain typé (tests `*_stops_off_row` et semblables).** Le membre est typé ; la table ob est un des cas de la rangée de valeurs refusées, à côté d'une liste (`pico8_hv()`), d'une struct, d'un vecteur, de valeurs à union nulle. Les écrivains de `src/` ne connaissent pas le tag `OB_TABLE` (`grep PICO8_TYPE_TAG_OB_TABLE src` ne rend que `pico8_ob.c/.h`, `pico8_ref_audit.h`, `pico8_type_tag.h`) : ils refusent sur la sorte (`kind != FIXED`…) ou sur `tag != UNKNOWN`/`!= STRUCT`/`!= LIST`, la clause que prend aussi une liste, une struct ou un nil REF.

| TU | Lignes | Écrivain(s) | Frères déjà dans la rangée (même clause, à confirmer au commit) |
|---|---|---|---|
| `entity_kind` | 725 (`table`, lu l. 749, 758) | `m_` de `ie`, `ie.gy` | `list`, `kind`, `other`, `vref` ; `other` pour `gy` |
| `fv` | 832 (`off[5]`) | `m_` de `fv` | `vec_ref`, `pico8_hv()`, `g_`, `fv` |
| `g_` | 1021 | `fo` (`bv`) | `hv`, `qu`, `jc`, `t`, `mz.jj` |
| `g_` | 1497, 1512, 1534 | `qy`/`ra`/`p`, `et`, `p_` | `jc`, `vec`, `hv`, huit structs ; `vec` pour `p_` |
| `ga` | 971 | `qy` | `jo`, `vec` |
| `jc` | 1650 (`table`, trois rangées) | `fo`, `p_`, `qu` (construction) | `list`, `vec`, `jo`, `ga`, `other`… |
| `js` | 1671 (`table`, trois rangées) | `m_`, `lq`, `lq`/`h_` de construction | `vec`, `list`, `js`, `jb`, `dr` |
| `mz` | 1209 (`table`, trois rangées) | membres de `mz` | `list`, `jo`, `other` |
| `nl` | 642 | `qw` (`family_take_fixed`, clause `kind == FIXED`) | nil REF (même clause) |
| `qb` | 741 | `v` | `hv`, `a` |
| `ui` | 1004 (`left[8]`, `right[6]`) | `lheld`/`rheld` | `hv`, `vec`, structs |
| `ui` | 1696, 1699 | `kw`, `hn`, `nk` de `jo`/`pm` | `hv`, `e[i]`, `gb_project(0)` |
| `ui` | 1919, 2268, 2937, 3280, 3431 | membres de `hj`, état de `hp`, `et`, `qy`, `ht` | `hv`/`list`, `e[i]`, `vec`, structs |
| `ui` | 2597 | `ma`, `kr`, `mi`, `m_`, `cc`, `by`, `fo` (écrivains génériques de `jo`) | `hv` pour `kr`/`mi`/`fo`, `vec`/`e[i]` pour `cc`/`by`/`fo` ; **aucun REF non nil pour `ma` et `m_`** |
| `ui` | 2965 | `s` de `qh`/`fr` | nil REF (clause de sorte, à confirmer) |

**F3 — une table ob passée comme `self` ou comme argument à une fonction qui n'accepte qu'une struct (tests « fermés »).**

| TU | Lignes | Test | Contrat testé | Ce qui reste sans la table |
|---|---|---|---|---|
| `fv` | 886, 892, 893 | `test_g__closed` | « `g_`'s functions take a struct or NULL as `self` » (neuf fonctions) | rien : la table est le seul cas non-struct |
| `js` | 1879, 1883 | `test_pz_closed` | « `pz` takes a struct or NULL » | rien |
| `ui` | 1040, 1045 | `test_ke_closed` | « `ks` takes a struct or NULL, and nothing but a struct enters `hv` » | rien |
| `mo` | 242, 247, 248, 253 | `test_accessors_refuse_a_table` | « the by-name accessors take entity structs only » | le cas `&list` (liste locale vide), déjà là |

**F4 — plis de hachage et aides de lecture.**

| TU | Lignes `ob` | Nature | Atteinte à `7d05979` |
|---|---|---|---|
| `fv` | 139, 144, 145 (+ l. 142, 218-219 `type-tag`) | `fold_table` et la branche `OB_TABLE` de `fold_value` | aucune valeur pliée n'est une table ob (H4) |
| `g_` | 191, 196, 197 (+ 194, 1166-1167) | `fold_table` et le `case OB_TABLE` de `fold_member` | idem |
| `pj` | 157, 162, 163 (+ 160, 235-236) | `fold_table` et sa branche | idem |
| `hp` | 166, 171 (+ 165) | branche `OB_TABLE` de `fold_value` | idem |
| `hy` | 160, 165 (+ 159) | idem | idem |
| `mz` | 941 (+ 940) | `case OB_TABLE` du pli de REF | idem |
| `qb` | 507 (+ 506) | branche `OB_TABLE` du pli | idem |
| `ga` | 193 (+ 192) | repli de `size_of` : `assert(tag == OB_TABLE)`, compte de la table | jamais pris : chaque nom demandé est `k_`, `nu` ou une liste (H4) |
| `hp` | 194 | `fold_names` : `fnv(h, PICO8_OB_KEY_INDEX)` avant chaque position | **atteinte** : l'octet fait partie du format figé de `DEFAULTS_HASH` |
| `hy` | 188 | `fold_qe_map` : `fnv(h, PICO8_OB_KEY_INDEX)` avant chaque `qe` | **atteinte** (constantes de `hy` qui plient `po`/`fu`) |
| `jc` | 266 | `fold_hg` : `fnv(h, PICO8_OB_KEY_NAME)` avant chaque `id` | **atteinte** dès que `hg` n'est pas vide (constantes de `jc`) |

Les trois dernières lignes ne lisent aucune table ob : depuis D.4b, elles plient une donnée typée (`pico8_str_seq_t`, `pico8_qe_map_t`, `pico8_jc_hg_t`) « comme `fold_value` pliait la table », pour que les constantes restent. Le seul lien avec la couche est la **valeur** de l'énumération `pico8_ob_key_kind_t` (`PICO8_OB_KEY_INDEX` = 0, `PICO8_OB_KEY_NAME` = 1, `pico8_ob.h` l. 12-13).

**F5 — la fixture `by` de `test_pico8_ga` (l. 574, 575).** `test_pico8_ga` (`LIFE_HASH`) pousse deux `pico8_ob_table_new()` dans `by`, puis vérifie que `ga:hb` change la rangée d'exports et que `size_of(b, "by") == 2`. Depuis D.7a, `by` (`g_ga_by_shared`, partagé par tous les `ga`) tient des **routes** (`pico8_trade_t`, poussées par `jo:ib`) ; `jo:hb` n'en lit que le compte, mais `pico8_db` (`economy_score.c` l. 119-140) les lit comme routes (`pico8_trade_to` → `assert_trade`). La fixture a donc la mauvaise forme ; elle ne corrompt rien aujourd'hui (le compte seul est lu dans ce test). Précédent pour la bonne forme : le test du §6 `ga_export_panel_by` pousse `pico8_trade_new(NULL)` deux fois (l. 147-150).

**Le reste, R_t = 1** : `test_pico8_planet_gh_row_lq.c` l. 2, commentaire d'en-tête d'un test du §6 : « the bug read it as a `pico8_ob_table_t*` ». Il décrit le bug que le test garde.

## Solution

**Trois commits, aucun fichier de `src/` touché, aucune constante `*_HASH` changée**, chacun vert seul :

1. **Inclusions seules** — les quatorze TU dont l'inclusion de `pico8_ob.h` est la seule ligne `ob` la perdent. Deux sont du §6 : diff d'une ligne chacun, corps inchangé.
2. **Valeurs hors rangée, tests fermés, fixture `by`** — les cas F2 retirés (ou remplacés quand la table était le seul REF non nil que refuse la clause) ; les tests F3 passent une liste locale vide au lieu d'une table ob ; F5 pousse deux routes ; les inclusions de `entity_kind`, `js`, `mo`, `nl`, `ui` partent.
3. **Plis de hachage** — `fold_table` et les branches `OB_TABLE` de F4 retirés ; `size_of` de `ga` n'accepte plus qu'une liste ; les trois octets de sorte de clé deviennent la valeur de l'octet (c déclarée, trois lignes) ; les inclusions de `fv`, `g_`, `ga`, `hp`, `hy`, `jc`, `mz`, `pj`, `qb` partent ; note « Découvert à D.7c ».

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert ; `make e2e` : **26 conformes** (cible rapide, `src/` inchangé) ; `make data-guard` : code 0, `DATA_RESIDUE_GUARD` = `spec,overrides` inchangé ;
- `make data-residue` : `ob` **409 lignes / 17 fichiers** (occurrences **464**) = 187 (couche et tests de la couche) + 22 (R) + 1 (R_t) + 199 (test ponctuel, hors compte) ; hors test ponctuel **210** ; `type-tag` 373 → **362** lignes (592 → 572 occurrences, ± ce que la reprise de `size_of` de `ga` laisse, dit au commit) ; `valeur` strictement plus bas ; `stash` 28, `par-nom` 1009, `dispatch` 191, `ref-audit` 65, `overrides` 0, `spec` 0 inchangés ;
- `git diff --stat 7d05979..HEAD -- src Makefile tools tests/e2e` vide ; aucun `*.sha256` touché ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  # 1. ob dans tests/ hors du test ponctuel et des tests de la couche : la seule l. 2 de gh_row_lq
  grep -rnE '\bpico8_ob(_[A-Za-z0-9_]+)?\b|\bPICO8_OB_[A-Z0-9_]+|\bpico8_fi(_fn)?\b|pico8_ob\.h' tests --include='*.[ch]' \
    | grep -vE '^tests/test_pico8_(map_tables|ob|stash)\.c:'
  #    attendu : tests/test_pico8_planet_gh_row_lq.c:2: * takes as the row's positionals; the bug read it as a `pico8_ob_table_t*`.
  # 2. inclusions de pico8_ob.h dans tests/ : test_pico8_ob.c seulement (le test ponctuel l'atteint par pico8_collect.h/pico8_stash.h)
  grep -rln 'include "../src/pico8_ob.h"' tests
  # 3. le tag OB_TABLE dans tests/ : vide
  grep -rn 'PICO8_TYPE_TAG_OB_TABLE' tests
  # 4. src/ : les 147 lignes de 7d05979, inchangées
  git diff --quiet 7d05979 HEAD -- src && echo src-inchangé
  ```

## User Stories

1. En tant que mainteneur du port, je veux qu'aucun TU hors du test ponctuel et des tests de la couche ne construise, ne lise ni n'inclue plus la couche ob, afin que D.8 supprime `pico8_ob.c/.h` sans rouvrir un seul TU.
2. En tant que `plan-pilot`, je veux que D.7c ne touche aucun fichier de `src/`, afin que la ligne dise vrai (« `sanitize-net` et `class-coverage-net` : non ») et que la vérification reste légère.
3. En tant que `plan-pilot`, je veux qu'un commit qui aurait besoin de changer `src/` s'arrête et le signale, afin qu'un changement de comportement ne se glisse pas dans une étape de tests.
4. En tant que gardien des TU, je veux qu'aucune constante `*_HASH` ne change, ni de valeur ni de nom, afin de prouver que chaque état comparé est le même et que chaque pli retiré n'était pas atteint.
5. En tant que gardien des TU, je veux qu'un cas de rangée « hors rangée » qui offrait une table ob à un membre typé soit retiré et listé avec sa raison quand un frère de la même rangée prend la même clause de refus, afin que la rangée garde toute sa garde sans nommer une forme qui disparaît.
6. En tant que gardien des TU, je veux que, là où la table ob était le seul REF non nil que la clause refusait (`ma`, `m_` de `ui` l. 2597, et tout site que la lecture de l'écrivain range là), elle soit remplacée par une liste déjà employée comme valeur hors rangée dans le TU, afin que la clause reste exercée.
7. En tant que gardien des TU, je veux que les tests « fermés » (`test_g__closed`, `test_pz_closed`, `test_ke_closed`) passent une liste locale vide au lieu d'une table ob, afin de garder leur contrat « une struct ou NULL, rien d'autre » avec une forme non-struct qui survit à D.8.
8. En tant que gardien des TU, je veux que `test_accessors_refuse_a_table` garde son cas liste et perde son cas table ob, retiré et listé, afin que son contrat reste gardé sans la forme qui disparaît.
9. En tant que gardien des TU, je veux que la fixture `by` de `test_pico8_ga` pousse deux routes, la forme que `by` tient depuis D.7a, afin que la rangée d'exports soit calculée sur des données du bon type et que `pico8_db` ne puisse plus y lire une table ob comme route.
10. En tant que gardien des TU, je veux que `fold_table` et les branches `OB_TABLE` des plis disparaissent, afin que les hachages ne parlent plus que des formes typées qu'ils voient réellement, la constante inchangée en étant la preuve.
11. En tant que gardien des TU, je veux que `size_of` de `test_pico8_ga` asserte une liste là où il retombait sur une table ob, afin qu'une forme inattendue s'arrête au lieu d'être comptée.
12. En tant que gardien des constantes, je veux que les trois octets de sorte de clé de `hp`, `hy`, `jc` restent pliés avec la même valeur, sans nom de la couche, afin que `DEFAULTS_HASH` et les constantes de `hy`/`jc` ne bougent pas et que ces TU n'incluent plus `pico8_ob.h`.
13. En tant que gardien du compteur, je veux que ces trois lignes soient **déclarées (c)** dans le message, ligne par ligne, avec la preuve qu'aucun identifiant n'est introduit et que l'inclusion part, afin qu'elles ne passent pas pour un renommage caché.
14. En tant que gardien du compteur, je veux, dans le message de chaque commit, les comptes avant/après en lignes et en occurrences, par catégorie et par fichier, afin qu'une baisse prouve une construction retirée.
15. En tant que gardien du compteur, je veux la classification (a)/(b)/(c) de chaque ligne `ob` retirée, avec (c) = 0 aux commits 1 et 2 et (c) = 3 déclarées au commit 3, afin que le correctif de D.7a ne soit plus nécessaire.
16. En tant que gardien du compteur, je veux la liste, par fichier et par catégorie, des lignes qui gagnent une catégorie (« aucune » compris), afin qu'aucun déplacement ne passe inaperçu.
17. En tant que gardien du compteur, je veux la justification de chaque couple (catégorie, fichier) où Δlignes > Δoccurrences, afin qu'aucune construction ne réapparaisse ailleurs.
18. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, aucun alias, registre, macro ou constante nommée ajoutés, aucun identifiant compté renommé, afin qu'aucune baisse ne soit un tassement.
19. En tant que gardien du §6, je veux que `gh_row_lq` et `layout_nil_item_draws_nothing` ne perdent qu'une inclusion morte, corps inchangé, et que la l. 2 de `gh_row_lq` reste, afin qu'aucun test de régression ne soit réécrit dans cette étape.
20. En tant que gardien du §6, je veux que la l. 2 de `gh_row_lq` soit nommée comme reste (R_t) avec l'étape qui la reprendra, afin que D.8 sache qu'elle doit la réécrire (commentaire seul, précédent D.6a).
21. En tant que gardien des filets, je veux `make test` et `make e2e` verts à chaque commit, et un balayage ASan+UBSan jetable des TU touchés au dernier commit, afin de prouver qu'aucune fixture reprise ne lit ni n'écrit hors de ses bornes.
22. En tant que gardien des tests, je veux des mutations ciblées (dans une copie jetable) qui prouvent que les frères gardés, la liste des tests fermés, la fixture `by` et les octets de hachage rougissent toujours un test, afin qu'aucun retrait n'ait affaibli une garde.
23. En tant que `plan-pilot`, je veux les hypothèses sur l'issue de D.7b étiquetées `H-D7b-n` et la liste des sections à revérifier, afin de comparer ce PRD à la note gradée de D.7b avant de le passer en triage.
24. En tant que `plan-pilot`, je veux une proposition chiffrée (non implémentée) d'une cible ASan+UBSan des TU, afin de la soumettre à `plan-review`.
25. En tant que lecteur du plan, je veux une note « Découvert à D.7c » qui donne le compte réel, R_t et son effet sur D.8, afin que D.8 parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Modules

Seuls des TU changent : 28 fichiers de `tests/`, plus `docs/plan-3.md` (note). Aucun module de `src/`, aucun outil, pas le `Makefile` (les TU gardent `pico8_ob.c` dans leurs sources de lien jusqu'à D.8). Aucun module profond à extraire : l'étape retire des fixtures, elle n'en crée pas (**aucune aide de TU nouvelle**).

### Cible et reste (G)

Après D.7c, `ob` = **409 lignes** :

| Groupe | Lignes | Pourquoi elles restent | Meurent à |
|---|---|---|---|
| `pico8_ob.c` / `pico8_ob.h` | 82 / 32 | la couche | D.8 |
| `test_pico8_ob.c` | 55 | test de la couche (§6) | D.8 |
| `pico8_stash.c` / `.h` / `test_pico8_stash.c` | 8 / 3 / 7 | module et test de la couche | D.8 |
| R (H-D7b-2) | 22 | `pi.c` 2, `pi.h` 1, `collect.c` 7, `collect.h` 3 (servent le test ponctuel) ; commentaires R_b : `type_tag.h` 2, `ref_audit.h` 4, `value.h` 1, `list.h` 1, `hud.c` 1 | D.8 |
| **R_t** | **1** | `test_pico8_planet_gh_row_lq.c` l. 2 (ci-dessous) | D.8 |
| test ponctuel | 199 | lecture de la référence que parse `ob()`/`unstash` et vues de comparaison (`df_entry_view`), hors compte (en-tête de la liste) | D.8 |

Soit **187 + 22 + 1 = 210 hors test ponctuel**.

**R_t, la l. 2 de `gh_row_lq` (G).** Elle énonce le bug que garde un test du §6 (« the bug read it as a `pico8_ob_table_t*` ») ; le type existe jusqu'à D.8, le commentaire est vrai.
- **Rejeté : la réécrire** pour ne plus nommer le type — même sujet, autre nom : le renommage pour échapper au motif que l'utilisateur interdit, et le rejet déjà prononcé pour R_b à D.7b.
- **Rejeté : la supprimer** — c'est l'énoncé du bug d'un test que le §6 interdit d'affaiblir.
- **À D.8** : le type disparaît, la ligne devient l'historique d'un type supprimé ; sa réécriture y est (a) « commentaire d'une donnée qui n'existe plus ». Le « §6 : aucun » de D.8 devient « §6 : `gh_row_lq`, commentaire de la l. 2 seulement, pas de preuve rouge » (précédent D.6a). Écrit dans la note.

### F1 — inclusions (G)

Chaque inclusion de `pico8_ob.h` part **dans le commit qui retire le dernier usage du TU** : (a) « inclusion devenue inutile ». Aucune inclusion n'est ajoutée (H1) ; si la compilation en impose une, elle est listée au commit, et si elle nomme `pico8_value.h` elle gagne une ligne `valeur`, listée.

### F2 — valeurs hors rangée (G)

Règle, site par site, **après lecture de l'`assert` de l'écrivain** (citée au commit) :

- **Retrait** : un autre cas de la même rangée prend la même clause de refus (sorte non admise, ou tag autre que celui qu'accepte l'écrivain) → la table ob est retirée de la rangée. (a). Le test garde son intention (« ce membre refuse tout ce qui n'est pas X ») ; ce qu'il perd, « refuse aussi une table ob », est un comportement de table ob d'un membre typé : **listé** au message (fichier, test, cas, clause, frère qui la garde).
- **Remplacement** : la table était le seul REF non nil qui prenait sa clause (lu à `7d05979` : `ma` et `m_` de `test_gen_writer…` dans `ui` l. 2597 ; `nl` l. 642 et `ui` l. 2965 si leur écrivain ne refuse pas un nil REF par la même clause de sorte) → **une liste**, `pico8_value_ref((void *)pico8_hv())`, la valeur hors rangée que ces mêmes TU emploient déjà. À l. 2597, forme conseillée : la ligne ob part et la condition de la ligne `pico8_hv()` existante s'étend à `ma` et `m_` (aucune ligne nouvelle portant un motif). (a) pour la ligne ob retirée ; la ligne de liste ne gagne aucune catégorie qu'une ligne retirée en regard ne portait.
- Les variables `table` devenues inutiles (`entity_kind`, `jc`, `js`, `mz`) partent avec leurs usages ; les tableaux à indices fixes (`ui` l. 993-1024, `left[11]`/`right[9]`) sont réindexés, bornes de boucle comprises.
- **Rejetés** : garder la table ob jusqu'à D.8 (D.8 rouvrirait onze TU que sa ligne ne nomme pas) ; la remplacer partout par une liste (doublons de cas là où une liste est déjà présente : un cas de plus sans clause de plus) ; un tableau « rangée commune » partagé entre tests (registre).

### F3 — tests fermés (G)

- `test_g__closed` (`fv`), `test_pz_closed` (`js`), `test_ke_closed` (`ui`) : la table ob devient **une liste locale vide** (`pico8_list_t list; pico8_list_init(&list);`, précédent : `test_accessors_refuse_a_table` de `mo`), passée par adresse. Chaque fonction garde son `assert(tag == STRUCT…)` : la liste prend la même clause. Les poses `ra`/`fa` (`fv` l. 892-893) partent : elles montraient qu'une table **portant les champs lus** était refusée — une lecture par nom de table ob, qui n'existe plus dans `src/` (les lecteurs par nom assertent une struct, `entity.c` l. 172, 188). Listé au message comme perte assumée, avec cette raison. Le commentaire de chaque test (« an ob table stops it ») change de sujet : réécrit pour la liste (son sujet a disparu).
- `test_accessors_refuse_a_table` (`mo`) : le cas table ob et ses deux poses (`ra`, `md`) sont retirés, le cas liste reste ; commentaire ramené au cas qui reste. Listé.
- Noms des tests et des aides (`pz_on_a_table`, `ks_on_a_table`, `lt_a_table`) **inchangés** : en Lua une liste est une table, le nom reste vrai, et un renommage serait du bruit.
- **Rejetés** : supprimer ces tests (les gardes `tag == STRUCT` de `pz`, `ks`, `lt` et des neuf fonctions de `g_` n'auraient plus de TU) ; passer une struct d'une autre classe (les gardes ne vérifient pas toutes la classe : le test pourrait ne plus avorter) ; passer `pico8_hv()` à `lt` (liste globale ; une liste locale n'a aucun effet de bord).

### F4 — plis de hachage (G)

- **`fold_table`** (`fv`, `g_`, `pj`) et **chaque branche ou `case` `OB_TABLE`** (`fv`, `g_`, `pj`, `hp`, `hy`, `mz`, `qb`) : retirés. (a) « lecture d'une table ob ». Preuve de non-atteinte : **les constantes `*_HASH` passent inchangées** (une table ob atteinte tomberait dans la branche par défaut et changerait le hachage). Les commentaires voisins qui citent `fold_table` (« A tag sequence as fold_table folds a table of the same positionals »…) décrivent désormais un format dont la référence a disparu : réécrits pour dire le format (position, valeur, compte), sans renvoi à une table ob.
- **`size_of` de `ga`** : le repli `assert(tag == OB_TABLE)` + compte de table part (a) ; le test de liste devient l'`assert` final (même ligne `type-tag`, reformulée). Une forme inattendue s'arrête au lieu d'être lue.
- **Les trois octets de sorte de clé** (`hp` l. 194, `hy` l. 188, `jc` l. 266) : `fnv(h, (uint32_t)PICO8_OB_KEY_INDEX)` → `fnv(h, 0u)`, `fnv(h, (uint32_t)PICO8_OB_KEY_NAME)` → `fnv(h, 1u)`, chacun avec un commentaire de contrat du format (« l'octet que plient les constantes figées avant une position / un nom »). **Déclarés (c)**, trois lignes, au message du commit 3. Pourquoi c'est admis, et pourquoi ce n'est pas un renommage pour échapper au motif :
  1. la dépendance disparaît réellement : le TU compile sans `pico8_ob.h`, que D.8 peut supprimer sans le rouvrir (un renommage garde la dépendance, comme `pico8_bz` → `pico8_pj_bz` gardait la fonction) ;
  2. aucun identifiant n'est introduit (ni macro, ni constante nommée, ni `enum` recopié : ce serait l'alias interdit) ;
  3. les mêmes octets sont pliés : les constantes inchangées le prouvent.
  - **Rejeté : retirer l'octet et mettre à jour les constantes touchées** (`DEFAULTS_HASH` de `hp`, les constantes de `hy` qui plient `po`/`fu`, jusqu'à sept de `jc` quand `hg` n'est pas vide) : toute la phase D fige les constantes (« arrêt sur la constante, jamais de mise à jour », D.7a, D.7b), et une constante changée ne prouverait plus rien.
  - **Rejeté : garder ces trois lignes et leurs inclusions jusqu'à D.8** (R_t de 6 lignes) : D.8 devrait rouvrir trois TU que sa ligne ne nomme pas, pour y prendre la même décision.
  - Si le pilote ou le vérificateur juge que cette (c) tombe sous l'interdit de l'utilisateur, le repli est le rejet n° 2 (garder jusqu'à D.8, R_t = 4 lignes + 3 inclusions), **jamais** la mise à jour des constantes.

### F5 — `by` de `test_pico8_ga` (G)

Les deux `pico8_list_add(by, pico8_value_ref(pico8_ob_table_new()))` deviennent `pico8_list_add(by, pico8_value_ref(pico8_trade_new(NULL)))` : (b) « variable dont le type passe d'une table ob à un type C » (l'élément poussé). `pico8_trade_new` n'enregistre rien dans `hv`, ne tire pas le RNG (lu : `trade.c` l. 54-79) ; `jo:hb` ne lit que le compte : `LIFE_HASH` inchangé (H5). `pico8_trade.h` est déjà atteint par le TU (H1). Listé au message comme **fixture de mauvaise forme corrigée** (surprise, voir Further Notes).

### Ce qui ne change pas

- `src/`, `Makefile`, `tools/`, `tests/e2e/`, tout `*.sha256`, tout fichier d'événements ; aucun e2e ajouté.
- `test_pico8_map_tables.c`, `test_pico8_ob.c`, `test_pico8_stash.c`.
- R (22 lignes) ; R_t.
- Toute constante `*_HASH` et tout tableau `identity[]`, de valeur et de nom ; tout nom de test et d'aide.
- `par-nom`, `dispatch`, `ref-audit`, `stash`, `spec`, `overrides` (aucune ligne de ces catégories n'est touchée ; tout écart est expliqué).
- `DATA_RESIDUE_GUARD` (`ob` ne tombe pas à 0).

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; retirer des retours à la ligne dans une ligne comptée ; renommer un identifiant, un paramètre ou un commentaire compté sans que la donnée change de type ; réécrire un commentaire compté dont le sujet existe toujours (R, R_t) ; macro, `typedef`, constante nommée, `enum` recopié, pointeur ou fonction enveloppe dont le rôle est de renommer un identifiant compté ; **un alias** ; **un registre** (tableau, table, `switch` ou boucle nouveaux qui nomment ensemble des valeurs, cas ou noms) ; une aide de TU nouvelle ; déplacer du code compté hors de `src`/`tests`, hors `.c`/`.h`, ou dans le test ponctuel ; supprimer un commentaire qui ne porte pas sur une donnée, un cas ou une inclusion retirés ; toute `pico8_ob_table_new`, toute vue ; tout changement de `src/`.
- **Classification exigée, au message de chaque commit** : chaque ligne `ob` retirée est (a) construction supprimée (allocation, pose ou lecture de table ob, branche de pli, cas de rangée, inclusion devenue inutile, commentaire d'une donnée qui n'existe plus), (b) déclaration ou variable dont le type passe d'une table ob à un type C, ou (c) même donnée, autre nom. Attendu : commit 1, (a) 14 ; commit 2, (a) 38 + (b) 2 (F5) ; commit 3, (a) 25 + **(c) 3 déclarées** (F4, octets). Recette : `git diff -U0` par fichier, lignes `-` qui portent le motif `ob` en regard des lignes `+`.
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

  Mesuré sur `7d05979` : `valeur` 5962, `ob` 550, `stash` 28, `type-tag` 592, `ref-audit` 66, `par-nom` 1064, `dispatch` 193, `overrides` 0, `spec` 0.
- **Contrôle** : pour chaque (catégorie, fichier) du diff, Δlignes et Δoccurrences ; **si Δlignes > Δoccurrences**, le message justifie ce couple.
- **Lignes qui gagnent une catégorie**, listées au message, **par fichier et par catégorie**, « aucune » compris. Attendu : aucune (les lignes de liste de F2, de routes de F5, d'octets de F4 portent au plus `valeur`, déjà portée par la ligne retirée en regard ; les listes locales de F3 ne portent aucun motif).
- **Fichiers hors de l'empreinte** : aucun changement ; tout écart est dit et justifié (≤ 3 chemins), jamais un fichier de `src/`.

### Empreinte remesurée et découpage (G)

| Commit | Fichiers | Lignes `ob` retirées / occurrences | Chemins |
|---|---|---|---|
| 1 — inclusions seules | `bg`, `fm`, `fq`, `fr`, `iv`, `je`, `jo`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `planet_kj_dispatch`, `nr`, `os`, `qh`, `trade` | 14 / 14 | 14 |
| 2 — hors rangée, fermés, `by` | `entity_kind` (2), `fv` (4), `g_` (4), `ga` (3), `jc` (1), `js` (4), `mo` (5), `mz` (1), `nl` (2), `qb` (1), `ui` (13) | 40 / 44 | 11 |
| 3 — plis de hachage, note | `fv` (4), `g_` (4), `ga` (2), `hp` (4), `hy` (4), `jc` (2), `mz` (2), `pj` (4), `qb` (2) ; `docs/plan-3.md` | 28 / 28 | 10 |
| **Distincts** | **28 TU + 1 doc** | **82 / 86** | **29** |

- **Écart avec la ligne (« Empreinte 41 (0, 41) »)** : 28 TU portent une ligne `ob` hors des trois tests exclus (la mesure de D.1 comptait aussi ceux que D.2-D.7b ont vidés).
- Ordre 1, 2, 3 : 1 est indépendant et sans risque ; 2 avant 3 parce que `fv`, `g_`, `ga`, `jc`, `mz`, `qb` gardent un usage d'`ob` (leurs plis) jusqu'au 3 et ne perdent leur inclusion qu'à ce moment ; 3 porte seul la décision (c) et la preuve « constantes inchangées », isolée pour qu'un rejet ne bloque pas le reste.
- **Rejetés** : un commit (28 fichiers, trois natures de preuve mêlées) ; un commit par TU (28 messages de comptabilité pour des retraits d'une ligne) ; F4 avant F2 (même raison que l'ordre choisi, inversée : les inclusions partiraient deux fois).

### Effet sur `make data-residue`

| Catégorie | Départ (`7d05979`) | Après 1 | Après 2 | Après 3 | Pourquoi |
|---|---|---|---|---|---|
| `ob` lignes / occ. | 491 / 550 | 477 / 536 | 437 / 492 | **409 / 464** | exact ; 17 fichiers à la fin |
| `ob` hors test ponctuel | 292 | 278 | 238 | **210** | 187 + 22 + 1 |
| `type-tag` | 373 / 592 | 373 / 592 | 373 / 592 | **362 / 572** (≈, `size_of` de `ga` dit au commit) | 11 lignes `OB_TABLE` des plis |
| `valeur` | 5155 / 5962 | = | strictement plus bas | strictement plus bas | `pico8_value_ref(pico8_ob_table_new())` retirés, déclarations `pico8_value_t table` |
| `stash`, `ref-audit`, `par-nom`, `dispatch`, `overrides`, `spec` | 28, 65, 1009, 191, 0, 0 | = | = | = | aucune ligne de ces catégories touchée |

« Exact » et « = » sont des égalités ; « ≈ » est remesuré au commit.

### Hypothèses (rien n'a été lancé)

**Sur l'issue de D.7b, non gradée** — à comparer à la note gradée avant de passer ce PRD en triage :

- **H-D7b-1** : D.7b est gradée sur `7d05979` sans commit correctif qui touche `tests/` ; les numéros de ligne de l'inventaire sont ceux de `7d05979`.
- **H-D7b-2** : R = 22 et la liste R_b de la note (`pico8_type_tag.h` 2, `pico8_ref_audit.h` 4, `pico8_value.h` 1, `pico8_list.h` 1, `pico8_hud.c` 1) plus les 13 de D.7a (`pi.c` 2, `pi.h` 1, `collect.c` 7, `collect.h` 3).
- **H-D7b-3** : la cible de D.7c est « 187 + 22 » (le « 187 + 13 » de D.7a devenu « 187 + 22 »).
- **H-D7b-4** : `ob` après D.7b = 491 lignes / 550 occurrences ; hors test ponctuel 292 ; `src/` 147 ; TU hors test ponctuel et tests de la couche : 83 lignes dans 28 fichiers.
- **H-D7b-5** : `pico8_ob.h` n'est inclus dans `src/` que par `pico8_ob.c`, `pico8_collect.h`, `pico8_stash.h` ; `slipway.h`, `ui.h`, `wormhole.h` incluent `pico8_list.h` (base du parcours d'inclusion H1).
- **H-D7b-6** : `test_pico8_spec_defaults`, `hj`, `mx`, `pm`, `probe_mm_rnd_order` n'ont plus de ligne `ob` (chunks 1-2 de D.7b), et les fixtures de particules des TU sont typées (`pico8_qy_t`).
- **H-D7b-7** : le balayage des 81 TU sous ASan+UBSan de D.7b n'a trouvé que `test_second_jl_keeps_specs` (base de la proposition de cible, et de l'idée qu'aucun des 83 sites ne corrompt déjà la mémoire).
- **H-D7b-8** : comptes des autres catégories après D.7b : `valeur` 5155/5962, `type-tag` 373/592, `ref-audit` 65/66, `par-nom` 1009/1064, `dispatch` 191/193, `stash` 28/28, `overrides` 0, `spec` 0 ; `DATA_RESIDUE_GUARD` = `spec,overrides`.
- **H-D7b-9** : la ligne D.7c et la ligne D.8 du plan ne sont pas modifiées à la gradation (aucune proposition `plan-review` retenue sur elles) ; la note « Découvert à D.7b » gradée est celle de `7d05979`, mots et chiffres.
- **H-D7b-10** : aucun nouveau reste R n'est ajouté par la vérification de D.7b (ni commentaire gardé, ni site laissé) dans `src/`.

**Sur D.7c** :

- **H1** : sans `pico8_ob.h`, chacun des 28 TU atteint encore `pico8_list.h`, `pico8_type_tag.h`, `pico8_value.h` et ce dont il se sert (parcours du graphe d'inclusion à `7d05979` ; `pico8_trade.h` atteint par `test_pico8_ga`) ; `-Werror` reste muet.
- **H2** : aucun écrivain ni garde de `src/` ne traite le tag `OB_TABLE` à part ; chaque cas F2 retiré a un frère qui prend la même clause, sauf les sites rangés en « remplacement ».
- **H3** : les gardes des tests fermés (`pz`, `ks`, `lt`, les neuf fonctions de `g_`, les accesseurs par nom) avortent sur une liste locale vide dans le même enfant forké.
- **H4** : aucune valeur pliée par un hachage des TU n'est une table ob à `7d05979` (seul `src/` pourrait en produire, et il n'en produit plus pour les entités) ; `size_of` de `ga` ne prend jamais son repli. Démentie : une constante rougit → arrêt, jamais de mise à jour.
- **H5** : `LIFE_HASH` ne dépend que du compte de `by` ; deux routes au lieu de deux tables ob le laissent inchangé.
- **H6** : `PICO8_OB_KEY_INDEX` = 0 et `PICO8_OB_KEY_NAME` = 1 (`pico8_ob.h` l. 12-13, premier et second énumérateurs) ; les constantes de `hp`, `hy`, `jc` passent inchangées avec `0u`/`1u`.
- **H7** : aucun des 28 TU ne rejoue d'événements ; `make test` dure quelques secondes plus la recompilation des 28 TU.
- **H8** : `constants-check`, `float-grep`, `float-guard`, `data-guard` restent verts.

## Implementation Chunks

Trois commits, dans cet ordre, chacun vert seul (`make test`, `make e2e`, `make data-guard`), chacun avec ses comptes, sa classification, ses lignes gagnées, sa liste de tests ou cas retirés/remplacés et ses mutations :

1. **Inclusions seules** — quatorze TU, une ligne chacun ; deux du §6 (corps inchangé).
2. **Valeurs hors rangée, tests fermés, `by`** — onze TU ; F2 (retrait ou remplacement, clause citée), F3 (liste locale), F5 (routes) ; inclusions de `entity_kind`, `js`, `mo`, `nl`, `ui`.
3. **Plis de hachage** — neuf TU ; `fold_table` et branches `OB_TABLE`, `size_of` de `ga`, trois octets déclarés (c), inclusions ; balayage ASan+UBSan jetable ; note « Découvert à D.7c ».

Ordre conseillé dans chaque commit : mesure de départ (lignes et occurrences par fichier) ; édition ; `make test` ; `make e2e` ; mutations (copie jetable) ; mesure d'arrivée, classification, lignes gagnées, greps ; docs ; message.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, identités, arrêts), pas la présence d'une ligne. D.7c ne change aucun code de `src/` : **la preuve est que chaque test touché garde sa garde** — les constantes inchangées pour les plis, un arrêt toujours obtenu pour chaque clause de refus, une mutation qui rougit toujours le test repris.

### TU touchés (G)

- **Règle** : un TU n'est touché qu'aux lignes de l'inventaire et à celles qu'elles entraînent (variable devenue inutile, indices, borne de boucle, commentaire dont le sujet disparaît). Aucune aide nouvelle, aucun registre, aucun rejeu d'événements, aucun nom de test ou d'aide changé.
- **Constantes** : aucune constante `*_HASH` ni `identity[]` ne change, ni de valeur ni de nom.
- **Tests et cas retirés** : aucun test entier n'est retiré. Chaque cas retiré (F2 retrait, F3 `ra`/`fa`/`md` et cas table de `mo`) est listé au message : fichier, test, cas, raison (« comportement de table ob d'un membre typé ; la clause reste gardée par <frère> »).
- **Test ajouté** : aucun (G). Rien de nouveau n'est à couvrir : `src/` ne change pas.
- **Prior art** : `test_accessors_refuse_a_table` (liste locale), `ga_export_panel_by` (routes dans `by`), les rangées hors rangée des 1b (valeurs à union nulle, `pico8_hv()`), les plis « comme `fold_table` pliait la table » de D.4b/D.7a.

### Preuve des tests (§7 : mutations)

`gcov` ne s'applique pas (aucune ligne de `src/` ajoutée ou modifiée, et les enfants qui avortent n'écrivent pas leurs compteurs). Les mutations se font **dans une copie jetable du scratchpad**, jamais dans l'arbre du dépôt ; chacune doit rougir le test nommé, puis être abandonnée avec la copie :

- commit 2 — **M1** : un écrivain à clause de tag (par exemple celui de `qy` de `ga`, ou `ma` des écrivains génériques de `jo`) accepte tout REF tagué → la rangée reprise rougit (preuve que le frère ou la liste de remplacement garde la clause) ; **M2** : la garde `tag == STRUCT` de `pico8_pz` retirée → `test_pz_closed` rouge avec la liste ; **M3** : idem pour `pico8_ke_ks` → `test_ke_closed` rouge ; **M4** : `jo:hb` ignore le compte de `by` → `test_pico8_ga` (`LIFE_HASH` ou l'`assert` de rangée) rouge avec les routes.
- commit 3 — **M5** : `fold_hg` plie `0u` au lieu de `1u` → une constante de `jc` rouge (l'octet est bien plié et lié) ; **M6** : `fold_names` omet l'octet → `DEFAULTS_HASH` rouge ; **M7** : un membre plié par `test_pico8_g_` rendu comme liste vide au lieu de sa donnée typée → constante rouge (le pli sans branche ob voit toujours la forme).
- Une mutation qui ne rougit pas est un trou : justifié au commit ou couvert par une assertion ajoutée au test repris.

### Balayage mémoire des TU (G)

Au commit 3, dans une copie jetable (`Makefile` inchangé, précédent : le balayage de D.7b) : les 28 TU touchés compilés et lancés sous `-fsanitize=address,undefined -fno-sanitize-recover=undefined`, `ASAN_OPTIONS=detect_leaks=0` (TROU-7). Critère : 28 propres. ~20 s avec `-j4`. Rapporté au message.

### Pas de nouvel e2e, pas de `sanitize-net`, pas de `class-coverage-net`

`src/` ne change pas : le binaire de rejeu est identique, `sanitize-net` et `class-coverage-net` n'ont rien à voir (la ligne le dit). `make e2e` (~1 s) tourne quand même à chaque commit, comme confirmation. Aucun e2e ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 minutes pour l'étape** (commit 1 ≈ 3, commit 2 ≈ 9, commit 3 ≈ 8).

| Cible (par commit) | Durée attendue |
|---|---|
| `make test` | quelques secondes, plus la recompilation des TU touchés |
| `make e2e` | ~1 s |
| `make data-guard`, `make data-residue`, recette, classification, greps | < 1 min |
| mutations (commits 2 et 3) | < 4 min |
| balayage ASan+UBSan des 28 TU (commit 3) | ~20 s avec `-j4` |

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement : `#`, commande exacte, état de l'arbre (HEAD + commit + étape), code de sortie, ligne clé, durée. Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre de chaque commit : `timeout 5m make test` ; `timeout 5m make e2e` : **26 « conforme »** ; `make data-guard` : code 0 ; `make data-residue` + recette : jalons du tableau « Effet » ; greps de la « Sortie » ; classification et anti-tassement passés.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit liste les chemins de son chunk (tableau « Empreinte »), rien d'autre ; **aucun chemin de `src/`**, ni `Makefile`, `tools/`, `tests/e2e/`, `*.sha256`.
2. **Reste** : la grep 1 de la « Sortie » rend la seule l. 2 de `gh_row_lq` ; R et R_t non modifiés.
3. **Greps** 2 à 4 : exactement ce qui y est écrit.
4. **§6** : `gh_row_lq` et `layout_nil_item_draws_nothing` : diff = une ligne d'inclusion chacun.
5. **Intention** : pour chaque cas retiré ou remplacé, la clause de l'écrivain citée et le frère qui la garde ; aucun test entier retiré ; aucun nom de test ou d'aide changé.
6. **Constantes** : aucune `*_HASH`/`identity[]` changée (diff) ; les trois octets de F4 sont `0u`/`1u` aux lignes déclarées, sans identifiant nouveau.
7. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier, recalculés avec la recette ; couples Δlignes > Δoccurrences justifiés ; aucune jonction, alias, registre, déplacement ; classification présente et juste, (c) = 3 exactement, déclarées ; lignes gagnées listées par fichier.
8. **Mutations** M1-M7 et balayage ASan+UBSan rapportés.
9. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.7c ni au plan dans `tests/`.
10. **Docs** : note « Découvert à D.7c » en fin de ligne D.7c, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le commit de gradation de D.7b (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas `ob` 491 (ou la valeur de la note gradée, H-D7b-4), `overrides` 0, `spec` 0, sans explication.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un commit touche un fichier de `src/` — **l'implémenteur s'arrête et le signale au pilote au lieu de le faire** (la ligne dit « aucun fichier de `src/` ne change ») ; idem pour le `Makefile` et `tools/`.
- `FAILED:` si un test de régression du §6 change autrement que par une inclusion retirée, ou si un test devrait changer d'intention pour passer.

**Périmètre**

- `FAILED:` si la grep 1 de la « Sortie » rend autre chose que la l. 2 de `gh_row_lq`, ou si une des greps 2 à 4 ne rend pas ce qui y est écrit.
- `FAILED:` si une ligne de R ou R_t est modifiée ou supprimée.
- `FAILED:` si `test_pico8_map_tables.c`, `test_pico8_ob.c` ou `test_pico8_stash.c` est touché.
- `FAILED:` si le diff ajoute une `pico8_ob_table_new`, une vue, une aide de TU, un registre, une macro, une constante nommée ou un `enum` pour remplacer un identifiant compté.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` ou `tools/data_residue.sh` change.
- `FAILED:` si `ob` ne vaut pas 409 lignes / 464 occurrences à la fin (ou 491 − 82 / 550 − 86 rapportés à un départ gradé différent, écart expliqué), si `type-tag` ou `valeur` ne baisse pas, si `stash` ≠ 28, `spec` ≠ 0, `overrides` ≠ 0, ou si `par-nom`, `dispatch`, `ref-audit` bougent.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés, construction déplacée, renommage à type constant (hors les trois (c) déclarées de F4), commentaire compté réécrit ou supprimé hors de son sujet, alias, registre, code déplacé hors du compteur.
- `FAILED:` si le **message d'un commit** ne donne pas, par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences**, la classification (a)/(b)/(c) des lignes `ob` retirées (avec chaque (c) déclarée et justifiée), la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris), la justification de chaque couple Δlignes > Δoccurrences ; un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si un cas ou un test est retiré sans être listé au commit avec sa raison et la clause qui reste gardée.
- `FAILED:` si une mutation manque ou ne rougit pas sans justification ; si le balayage ASan+UBSan du commit 3 manque ou n'est pas propre.
- `FAILED:` si le budget est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.7c », ou si elle omet le compte `ob` réel, R_t et son effet sur D.8, les trois (c), la fixture `by` corrigée.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **`src/`** entier, dont R (22 lignes) et `pico8_ob.*`/`pico8_stash.*` : D.8.
- **Le test ponctuel** (199 lignes, lecture et vues de comparaison de la référence) et **les tests de la couche** `test_pico8_ob.c`, `test_pico8_stash.c` : D.8.
- **R_t** (l. 2 de `gh_row_lq`) : D.8, commentaire seul.
- **Le `Makefile`** : les TU gardent `pico8_ob.c` dans leurs sources de lien ; D.8 retire les entrées.
- **La cible ASan+UBSan des TU** : proposée (Further Notes), pas implémentée.
- **`PICO8_TYPE_TAG_OB_TABLE`** dans `src/` : D.8 (D.7c vide seulement `tests/` de ce tag).
- **D.9 et suites** : `par-nom`, `dispatch`, `pico8_list_t` (les rangées hors rangée restent des `pico8_value_t`).
- La phase 2 ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **modules** : 28 TU et la note du plan ; aucun module de `src/`, aucune aide nouvelle ; `sanitize-net`/`class-coverage-net` non exigés, confirmés par l'inventaire.
  - **cible** : `ob` = 187 + 22 + 1 (R_t, commentaire du §6) + 199 (test ponctuel) = 409 ; rejetés : réécrire ou supprimer R_t.
  - **F2** : retrait quand un frère prend la même clause, liste `pico8_hv()` sinon ; rejetés : garder jusqu'à D.8, remplacer partout, rangée commune.
  - **F3** : liste locale vide ; rejetés : supprimer les tests, struct d'une autre classe, `pico8_hv()` global.
  - **F4** : branches retirées sous la preuve des constantes ; trois octets en valeur littérale, (c) déclarées ; rejetés : mettre à jour les constantes, garder jusqu'à D.8 (repli si la (c) est refusée).
  - **F5** : deux routes ; précédent `ga_export_panel_by`.
  - **tests** : aucun test ajouté ni retiré en entier ; cas retirés listés ; sept mutations en copie jetable ; balayage ASan+UBSan jetable au commit 3.
  - **découpage** : trois commits (14, 11, 10 chemins).
- **Note du plan (G)**, à ajouter en fin de ligne D.7c (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.7c : trois commits (inclusions seules ; valeurs hors rangée, tests fermés et `by` ; plis de hachage — N chemins, aucun de `src/`). Les cas hors rangée qui offraient une table ob à un membre typé sont retirés quand un frère garde la clause, remplacés par une liste sinon ; les tests fermés passent une liste locale ; `test_pico8_ga` poussait des tables ob dans `by`, qui tient des routes depuis D.7a : il pousse des routes. Trois octets de sorte de clé (`hp`, `hy`, `jc`) sont pliés par leur valeur, déclarés (c) ; aucune constante `*_HASH` ne change. R_t = 1 : la l. 2 de `test_pico8_planet_gh_row_lq` (énoncé du bug, nomme `pico8_ob_table_t`) reste ; D.8 la réécrit : « §6 : `gh_row_lq`, commentaire de la l. 2 seulement, pas de preuve rouge ». `ob` après D.7c : O lignes / O' occurrences (hors test ponctuel : H = 187 + 22 + 1) ; `tests/` ne nomme plus `PICO8_TYPE_TAG_OB_TABLE`.

  N, O, O', H sont les valeurs réelles.
- **Le message de chaque commit** porte : le périmètre ; la classification (a)/(b)/(c) des lignes `ob` retirées ; les comptes avant et après **en lignes et en occurrences** par catégorie et par fichier, avec les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie, par fichier (« aucune » compris) ; les cas et tests retirés ou remplacés, avec la clause citée ; les mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses H qui le concernent.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque commit contre ce PRD avant le commit (hors budget de tests) : anti-tassement, classification, lignes gagnées, intention des cas retirés, greps.
- **Sections à revérifier quand D.7b sera gradée** (comparer à la note gradée et au commit de gradation, puis mettre à jour avant de passer en triage) :
  1. « Problem Statement » : la note « Découvert à D.7b » recopiée (mots et chiffres), et les lignes D.7c/D.8 recopiées (H-D7b-9).
  2. « État de départ » : commit de départ, tableau `make data-residue`, occurrences, `ob` par groupe (H-D7b-4, H-D7b-8).
  3. « Inventaire » : numéros de ligne et fichiers de F1-F5 et de R_t (H-D7b-1, H-D7b-6) ; parcours d'inclusion (H-D7b-5).
  4. « Cible et reste » : R = 22 et sa liste (H-D7b-2, H-D7b-10), le « 187 + 22 » (H-D7b-3).
  5. « Empreinte remesurée et découpage » et « Effet sur `make data-residue` » : tous les jalons (477/536, 437/492, 409/464, 210, `type-tag` 362/572).
  6. « Sortie » : greps et comptes attendus ; « Critères d'échec », puce du départ.
  7. « Balayage mémoire des TU » et la proposition ci-dessous : si la vérification de D.7b a trouvé un autre débordement de TU (H-D7b-7), il est à rattacher ici avant tout.
- **Proposition, non implémentée — cible ASan+UBSan des TU** (à soumettre à `plan-review`, méthode seulement) : le balayage de D.7b a trouvé qu'un TU (`test_second_jl_keeps_specs`) écrivait hors de `g_nf_l` depuis `e252fde` et passait en relisant sa propre corruption ; `make test` ne compile aucun TU sous sanitizer (seul `gh_row_lq` l'est, sous ASan). Proposition : une cible `make unit-sanitize` qui construit chaque binaire de `UNIT_TESTS` dans l'arbre d'objets `obj-sanitize` que `sanitize-net` construit déjà (ASan+UBSan, `-fno-sanitize-recover`), lancé avec `ASAN_OPTIONS=detect_leaks=0` (TROU-7) ; **coût mesuré par D.7b : ~20 s avec `-j4`** ; hors de `make test` (qui reste la boucle de quelques secondes), exigée aux étapes qui touchent `tests/` et à la sortie de phase, comme `sanitize-net` l'est pour `src/` (§7, « En D et en 2 »). D.7c en fait l'essai à la main, jetable, au commit 3.
- **Surprises pour le pilote** :
  - **D.7c ne touche pas `src/`, et le peut** : aucun des 83 sites n'exige un changement de `src/` ; aucune inclusion à ajouter.
  - **`test_pico8_ga` poussait des tables ob dans `by`**, qui tient des routes depuis D.7a (`pico8_db` les lit comme `pico8_trade_t`) : même famille que le TU de D.7b, sans corruption ici (seul le compte est lu), à corriger quand même.
  - **Trois plis gardent la valeur d'une énumération de la couche** pour que leurs constantes survivent (`hp`, `hy`, `jc`) : c'est la seule (c) de l'étape, déclarée, et le seul point où l'interdit « pas de renommage pour échapper aux greps » demande un arbitrage ; le repli est écrit.
  - **Le compte final n'est pas « 187 + 22 »** mais 187 + 22 + 1 : la l. 2 de `gh_row_lq` énonce le bug d'un test du §6 et ne peut ni partir ni changer avant D.8, dont le « §6 : aucun » devient « commentaire de la l. 2 ».
  - **L'empreinte de la ligne (41) surestime** : 28 TU portent encore une ligne `ob`, dont 14 par leur seule inclusion.
