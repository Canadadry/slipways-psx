---
title: "Plan 3, étape D.7a : les tables ad hoc de l'affichage, de l'économie, de la recherche et des données passent de tables ob à des structs locales, des tableaux fixes ou des rangées typées ; les vues ob par appel, par frame et par init sont supprimées, les quatre ponts `overrides` de D.6b ferment avec le chaînon `pz` pris à D.7b, `overrides` = 0 entre dans `DATA_RESIDUE_GUARD` — six commits, aucun pixel changé"
description: "Une vingtaine de modules de `src/` construisent encore, à chaque appel, chaque frame ou chaque init, des tables ob pour des données dont la forme est connue (rangées d'affichage, enregistrements de souhait `{qe,gc}`, `gb`, `nb` et son drapeau `e_`, état du panneau de score, particules, boutons de souris) ; des vues ob de rangées générées fuient à chaque frame (`pj:kj`, `fv:kj`). D.7a les type famille par famille, ferme les vues et les quatre ponts de D.6b — ce qui exige de prendre à D.7b le chaînon `pz` —, et mesure la baisse en lignes ET en occurrences, plus la fuite par frame sous LeakSanitizer, sans tassement, alias, registre ni vue nouvelle."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.7a — `ob`, tables ad hoc de l'affichage, de l'économie, de la recherche et des données (troisième puce) : tout `ob` restant de `planet_display`, `research_display`, `score_panel_display` (`g_de`, `spec1`/`spec2`), `slipway_display`, `wormhole_display`, `layout`, `menu`, `collect`, `economy*`, `research` (`g_gb`, `pico8_fi`), `planet_trade` (enregistrements de `jo:ib`), `tech_gate`, `score_panel`, `pi`, `gen`, `galaxy`, `cart`, `data_init`, `game_data`, `mouse` → structs locales ou tableaux fixes, les TU touchés suivant dans le même commit. Empreinte 39 (39, 0) aujourd'hui. Compte : `ob` ≤ 1057, et 0 dans ces fichiers. §6 : `ga_export_panel_by` (`route_a`/`route_b`, l. 102-103 et 147-150, enregistrements de `jo:ib` poussés dans `ga.by`), preuve rouge. `class-coverage-net` : seulement si le diff touche un constructeur.

Lignes voisines, recopiées (D.6c, en cours d'implémentation ; D.7b, D.7c et D.8, qui partagent le compte `ob`) :

> - D.6c — `overrides`, reste : `g_` (`probe`), `fv` (`help`), `ie` (`research_icon`, `cart.c`), `hp` (`research`), `hy` (`economy`), `trade` (et `planet_trade.c`), `pj` (`score_panel`), même règle. Empreinte 21 (16, 5). Compte : `overrides` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui.
>
> - D.7b — `ob`, reste de `src/` hors `pico8_ob.*` et `pico8_stash.*` : entités et interface (`planet*`, `probe`, `particle`, `slipway`, `wormhole`, `msgbox`, `mouse_reader` avec `g_nf_l`/`g_nf_r`, `ui*`, `help`, `scroll`, `trade`, `bg`, `ke`, `entity`, `bootstrap`, `hud`, `research_icon`) et en-têtes (`pico8_struct.h`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_value.h`, `pico8_list.h`), même règle. Empreinte 42 (42, 0) aujourd'hui. Compte : `ob` ≤ 827, et 0 dans `src/` hors ces deux modules. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur.
>
> - D.7c — `ob`, TU restants : fixtures construites en `pico8_ob_table_*` → structs, état construit directement. Empreinte 41 (0, 41) aujourd'hui. Compte : `ob` = 187 (`pico8_ob.c` 82, `pico8_ob.h` 32, `test_pico8_ob.c` 55, `pico8_stash.c` 8, `pico8_stash.h` 3, `test_pico8_stash.c` 7). §6 : aucun. `sanitize-net` et `class-coverage-net` : non, aucun fichier de `src/` ne change.
>
> - D.8 — suppression : `pico8_ob.c/.h` (avec `pico8_fi`), `pico8_stash.c/.h`, `test_pico8_ob.c` et `test_pico8_stash.c` (§6, tests de la couche), le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, leurs entrées du `Makefile`. Compte : `ob` = 0, `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : non.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]`, union de deux greps sur les membres `m` de la struct (colonne `membre`) — accès direct : le fichier nomme `pico8_T_t` pour T la struct ou une fille qui l'enchâsse, et contient `->m`/`.m` non suivi de `(`, ou `offsetof(pico8_T_t, …m)` ; par nom : une ligne qui contient `_field` ou `_truthy` porte `"m"`. Majorant : un nom court partagé (`ra`, `fa`, `p`) compte partout où il est lu par nom.
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » versionnées à `f5d920c`, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b, D.5c, D.6a, D.6b ; celle de D.6c n'existe pas encore) :

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

Puces de la phase D, recopiées (la troisième est celle de D.7a) :

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


### Lecture de ces textes pour D.7a

- **Ce que D.7a doit fermer, d'après les notes** (chaque renvoi « D.7a » des notes, relevé un par un) :
  - **ponts de données** : `pico8_nb()` et `pico8_gi(ob)` (D.2, D.4b : `menu`/`tech_gate`) ; `pico8_data_init(mem)` qui ne lit plus `mem` (D.2 : « retrait à D.7a ») ; `pico8_bz()` (D.3 : `score_panel_display`) ; `pico8_ix()`/`pico8_ha()`, `g_bo`, `pico8_bz()` « reportés avec » les ponts (D.6b) ;
  - **vues à l'appel** par `pico8_jb_row_view` : `jo:gh` (`px`, `dz[mi]`), `jo.ex` (`em`), l'item `kw` de `jo:gh` (`jo_kw_view`) (D.4a : `planet_display`/`research_display`) ; `hp.fk`, `hp.lq` (D.4b : `research_display`/`planet_display`) ; `fv.rb`, `pj.i_` (D.4b : `pi`) ; `pj.gs[idx]` (D.4b : `score_panel`) ;
  - **tables d'état par init** `g_pj_ou`/`g_pj_ea` et leurs lecteurs `hb`, `hk`, `kj`, `pico8_mh_new` (D.5b : « D.6b, D.7a ») ;
  - **`pico8_ft_index`/`ft_name`** sans appelant (D.4b : `collect`) ; **`ga.by`** « contenu à typer par D.7a » (D.5c) ;
  - **les quatre ponts `overrides`** `pico8_lq_new_table`, `pico8_jb_new_table`, `pico8_js_new_table`, `pico8_mh_new_table` (D.6b : « `planet_display`, `menu`, `score_panel_display`, `research_display` : D.7a ; `help`, `msgbox` : D.7b ; le « 0 dans `layout` » de D.7a exige les deux premiers ») ;
  - **`pico8_ho_apply`** sans appelant de `src/` (D.6c, hypothèse H-D6c-3 : « D.7a `collect`, ou D.8 avec le test ponctuel ») et **`g_gb`** resté table ob (D.6c, H-D6c-2).
- **« 0 dans ces fichiers » ne tient pas à la lettre, pour une raison nommée** : le test ponctuel (hors compte jusqu'à D.8) parse encore des spécifications par `pico8_ob()` avec `v(…)`, `s(…)`, `b(…)`, et appelle `pico8_ho_apply` pour `nb` et `df`. Les enregistrements de constructeurs de `ob()` (`pi.c` : `'v'`, `'s'` ; `collect.c` : `'b'`) et `pico8_ho_apply` servent donc le test ponctuel jusqu'à sa suppression. Ils restent, **liste close** (section « Reste `ob` admis »), et meurent à D.8 avec `pico8_ob.c` et le test ponctuel. `gen`, `galaxy`, `game_data` sont déjà à 0 ligne `ob` : rien à y faire.
- **« Empreinte 39 (39, 0) » sous-estime d'un ordre de grandeur** : la ligne ne comptait que les fichiers de `src/` qui portent une ligne `ob`. Les types qui changent (rangées, `gb`, enregistrements `{qe,gc}`, articles de `bl`) forcent leurs lecteurs hors de la liste (`planet.c`, `planet_family_trade.c`, `ui.c`, `ui_button.c`, `msgbox.c`, `help.c`, `mouse_reader.c`) et une trentaine de TU. Mesure : ≈ 95 chemins distincts, découpés en six commits de 16 à 34 chemins (section « Empreinte remesurée »).
- **La troisième puce** (« structs locales ou tableaux fixes ») se lit, pour une table ad hoc : **constante** → donnée `static const` (rangée générée ou littérale locale) ; **mutée en cours de partie** → tableau fixe statique non `const`, remis à chaque init comme l'était la vue (règle commune du « défaut muté ») ; **construite à l'appel avec un contenu dynamique** (chaîne formatée, valeur calculée) → struct locale sur la pile si sa durée de vie finit avec l'appel, sinon **une** allocation typée là où l'appel allouait déjà une table ob, jamais là où il n'allouait rien (TROU-7 inchangé : ce que l'instance emprunte n'est pas libéré).
- **§6** : `ga_export_panel_by` (la ligne le nomme) et, **découvert ici**, `gh_row_lq` : ses lignes 158-160 lisent l'item 3 de la rangée « exports: » comme la table ob que `jo_kw_view` construit. Fermer cette vue (D.4a l'assigne à D.7a) force sa réécriture. `layout_nil_item_draws_nothing` n'est touché que si `pico8_lq_keys_t` change (il ne change pas : D.7a garde ses items `pico8_value_t`).

### Décision de plan : l'ordre D.7a / D.7b (G, à enregistrer par le pilote)

La note de D.6b l'annonce : « D.7a hérite d'un chantier plus lourd : fermer les ponts `lq`/`jb` demande de typer les producteurs de rangées de `planet_display`, `menu`, `score_panel_display`, `research_display` et les rangées `bs` de `pz` (fichier de D.7b) ; son PRD tranche l'ordre ou la fusion avec D.7b. » Le relevé sur `f5d920c` confirme le verrou et l'élargit.

- **Le verrou** : `pico8_pz(p, bs)` (`msgbox.c`) reçoit une liste de **tables ob** (`bs`), y pose `ra`, `qf`, `fa`, puis appelle `pico8_js_new_table(b)`. Ses producteurs sont `menu.c` (`cr` et `pz(ha,ha)`), `planet_display.c` (`jo:gh`, `jo:fe`) — fichiers de D.7a — et `help.c` (`fv:pq`, rangées de `bo`) — fichier de D.7b. Les valeurs que ces rangées transportent (`lq`, `h_`, `jb`, l'`oy` de `nb`, les tuples lus par `lq:mn`) arrivent à `js:pq` par ce pont, qui lit une `lq`/`h_`/`jb` table par `pico8_lq_new_table`/`pico8_jb_new_table`. **Tant que `pz` prend des tables ob, aucun producteur de D.7a ne peut typer sa rangée, et `nb` ne peut pas être typé** (sa valeur `oy`, typée, arriverait à `js:pq` qui attend une table taguée `OB_TABLE`), sauf à construire une vue par appel — interdit.
- **Options pesées** :
  - **(a) laisser les ponts à D.7b** — `menu`, `planet_display`, `layout`, `score_panel_display`, `research_display`, `cart` et `ui_button` garderaient leurs tables de rangée et leurs sources ob ; « 0 dans ces fichiers » et « 0 dans `layout` » tomberaient, `nb` resterait une vue, et D.7b devrait rouvrir tous les fichiers de D.7a. **Rejetée** : c'est livrer D.7a sans sa ligne ;
  - **(b) intervertir D.7a et D.7b** — D.7b ne peut pas typer `pz` sans que ses producteurs de D.7a le soient : verrou circulaire. **Rejetée** ;
  - **(c) fusionner D.7a et D.7b** — ≈ 130 chemins, les entités (`particle`, `probe`, `wormhole`, `bg`, `ke`, `entity`, `hud`…) n'ont aucun lien avec le verrou. **Rejetée** ;
  - **(d) retenue : fusion partielle nommée**. D.7a prend à D.7b **le chaînon `pz`** et lui seul : `pico8_pz`/`pico8_pz_step` et `pico8_mh_new_table` (`msgbox.c/.h`), `g_bo` et les rangées de `fv:pq` plus la vue par frame de `fv:kj` (`help.c`), `pico8_js_new_table` et les branches `OB_TABLE` de `js:pq` (`ui_button.c/.h`). S'y ajoutent les **suiveurs forcés** d'un type que D.7a change (seules les lignes qui lisent ou écrivent la donnée convertie) : `planet.c` (`jo:pq` lit `gb`, `jo:lz` copie les enregistrements `{qe,gc}` par `pico8_mw`), `planet_family_trade.c` (lit `gc`/`qe`), et **`g_nf_l`/`g_nf_r`** (`mouse_reader.c`, lecteur `ui.c`) avec `pico8_bu` (`mouse.c`, que la ligne met à D.7a : la fonction et la donnée qu'on lui passe sont une seule décision de type).
- **Conséquences à écrire dans le plan** (texte de la note plus bas ; le pilote les enregistre par la note « Découvert à D.7a », seul canal que le §7 prévoit, et peut consulter `plan-review` s'il le juge utile) :
  - `overrides` tombe à **0** à D.7a et entre dans `DATA_RESIDUE_GUARD` dans le commit qui ferme le dernier pont (chunk 5) ;
  - la ligne D.7b perd `pz`, `pico8_mh_new_table`, `g_bo`/`fv.rb`, `pico8_js_new_table`, `g_nf_l`/`g_nf_r` et les lignes suiveuses de `planet.c`, `planet_family_trade.c`, `ui.c` ; son majorant `ob` baisse d'autant (compte réel donné par la note) ;
  - la ligne D.7c et D.8 : le « = 187 » de D.7c devient « 187 + R », R étant le reste admis de D.7a (enregistrements de constructeurs de `ob()` et `pico8_ho_apply`, ≤ 15 lignes), que D.8 supprime avec `pico8_ob.c` et le test ponctuel.
- **Repli** : le chaînon `pz` est le chunk 5, précédé de quatre commits verts qui ne le supposent pas. S'il reste rouge après 3 tentatives, c'est l'arrêt du §7 (`FAILED:`), jamais un report silencieux des ponts : l'arbre des chunks 1-4 est cohérent, et le pilote tranche.

### D.6c n'est pas livrée à l'écriture de ce PRD

D.6c (clés de `g_`, `fv`, `ie`, `hp`, `hy`, `trade`, `pj` typées ; pont `pico8_fd()` fermé ; `overrides` = 30) est en cours d'implémentation et touche des fichiers de D.7a (`cart.c`, `research.c/.h`, `planet_trade.c`, `score_panel.c/.h`, `economy.c/.h`, `data_init.c/.h`, `collect.c/.h`, `help.c/.h`, `test_pico8_hp`, `test_pico8_data_init`, le test ponctuel). **Ce PRD suppose que D.6c atterrit comme son PRD le dit.** Chaque supposition est une hypothèse **H-D6c-n** (section « Hypothèses sur D.6c ») ; la section « À revérifier après D.6c » liste ce que `plan-pilot` relit contre la note « Découvert à D.6c » avant de passer ce PRD en `triage/`. **Aucun fichier non versionné n'a été lu** : tout vient de `f5d920c` (`git archive` dans le scratchpad) ; les numéros de ligne cités sont ceux de `f5d920c` (D.6c en déplace dans les fichiers nommés ci-dessus : retrouver chaque site par son texte).

### État de départ

**Mesuré sur `f5d920c`** (branche `claude/plan-pilot-3-7zdft0` ; `src/` et `tests/` identiques à `58e39e3`, D.6b gradée) ; **l'arbre de départ de D.7a sera le commit gradé de D.6c** (ou un commit de docs au-dessus). Décisions de l'utilisateur qui engagent D.7a : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**. Consigne de pilotage, liante : **une étape précédente a été notée FAILED pour avoir fait baisser un compte par tassement de lignes** ; le compteur compte des lignes, et toute mise en forme, tout renommage à type constant, tout alias, registre ou déplacement qui baisse un compte sans retirer la construction est interdit.

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `sh tools/data_residue.sh` (avec `--detail`), la recette d'occurrences de D.6c et des `grep` ont tourné, sur la copie `git archive f5d920c` du scratchpad ; le reste vient de la lecture des sources et de `pico8/lua.lua`.

### Inventaire (mesuré sur `f5d920c`)

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

**`ob` par groupe de la liste** (lignes / fichiers) : fichiers de `src/` de la ligne D.7a **515 / 34** (564 occurrences) ; autres fichiers de `src/` (D.7b) 128 / 28 ; TU hors test ponctuel et hors couche 450 / 40 ; test ponctuel 182 / 1 ; couche (`pico8_ob.*`, `pico8_stash.*`, `test_pico8_ob`, `test_pico8_stash`) 187 / 6. **Hors test ponctuel : 1280.** Le majorant de la ligne (« ≤ 1057 ») décalé par les notes (+32 D.2, +103 D.3, +4 D.4a, −100 D.4b) vaut **≤ 1096** hors test ponctuel.

**Toutes catégories, sur les 46 fichiers de `src/` de la ligne D.7a** (lignes / occurrences / fichiers) : `valeur` 895 / 1049 / 36, `ob` 515 / 564 / 34, `type-tag` 24 / 36 / 8, `ref-audit` 21 / 21 / 8, `par-nom` 105 / 105 / 16, `dispatch` 30 / 30 / 12, `overrides` 44 / 49 / 10, `stash` 0.

**`ob` par fichier de la ligne** (lignes / occurrences à `f5d920c` ; lignes que D.6c doit retirer selon son inventaire, H-D6c-6 ; départ estimé de D.7a ; reste attendu ; chunk qui le convertit) :

| Fichier | `f5d920c` | D.6c | Départ D.7a | Après D.7a | Chunk(s) |
|---|---|---|---|---|---|
| `data_init.c` | 72 / 84 | 15 (`fd`) | ≈ 57 | 0 | 4 (`bz`), 5 (`ix`, `ha`, `bo`, `pico8_jb_row_view`), 6 (`nb`) |
| `data_init.h` | 9 / 9 | 1 | ≈ 8 | 0 | 4, 5, 6 |
| `menu.c` / `.h` | 61 / 64 ; 3 / 3 | — | 64 | 0 | 5, 6 |
| `score_panel_display.c` / `.h` | 49 / 51 ; 1 / 1 | — | 50 | 0 | 1 (`ky`), 4, 5 |
| `research_display.c` / `.h` | 46 / 52 ; 2 / 2 | — | 48 | 0 | 2, 3, 5 |
| `research.c` / `.h` | 35 / 38 ; 4 / 4 | 24 ; 2 | ≈ 11 ; 2 | 0 | 2 (`g_gb`), 3 (`pico8_fi`) |
| `layout.c` / `.h` | 35 / 36 ; 5 / 5 | — | 40 | 0 | 5 |
| `planet_display.c` | 32 / 39 | — | 32 | 0 | 3 (`gc`), 5 |
| `collect.c` / `.h` | 26 / 31 ; 9 / 13 | — | 35 | **R** : ≤ 10 | 1 (`ft_*`), 3 (`qj`, `mw`), 4 (`oo`) |
| `planet_trade.c` | 17 / 18 | 6 | ≈ 11 | 0 | 3 |
| `score_panel.c` / `.h` | 16 / 17 ; 5 / 5 | 2 ; 1 | ≈ 14 ; 4 | 0 | 4 |
| `slipway_display.c` | 13 / 14 | — | 13 | 0 | 1 |
| `economy_tiers.c` | 11 / 13 | — | 11 | 0 | 3 |
| `pi.c` / `.h` | 10 / 10 ; 5 / 6 | — | 15 | **R** : ≤ 4 | 4 |
| `tech_gate.c` / `.h` | 10 / 10 ; 2 / 2 | — | 12 | 0 | 6 |
| `cart.c` | 9 / 9 | 5 (`ie`) | ≈ 4 | 0 | 5 |
| `economy_score.c` / `.h` | 9 / 9 ; 3 / 3 | — | 12 | 0 | 1 |
| `mouse.c` / `.h` | 5 / 5 ; 2 / 2 | — | 7 | 0 | 1 |
| `economy_formulas.c` / `.h` | 3 / 3 ; 1 / 1 | — | 4 | 0 | 3 |
| `economy.c` / `.h` | 2 / 2 ; 2 / 2 | 2 ; 2 | 0 | 0 | — |
| `wormhole_display.c` | 1 / 1 | — | 1 | 0 | 1 (inclusion morte) |
| `gen`, `galaxy`, `game_data` (`.c`/`.h`), `cart.h`, `planet_display.h`, `planet_trade.h`, `slipway_display.h`, `economy_tiers.h`, `wormhole_display.h` | 0 | — | 0 | 0 | — |
| **Total (46 fichiers)** | **515 / 564** | **≈ 60** | **≈ 455** | **R ≤ 15** | |

**Suiveurs forcés, fichiers de D.7b** (lignes `ob` / occurrences à `f5d920c` ; celles que D.7a retire) : `msgbox.c` 13 / 15 (toutes : pont `mh` l. 199-210, `pz_step` l. 413-435, préfixe `ep` de `pz` l. 467-473) ; `msgbox.h` 3 / 3 (inclusion l. 10 si plus rien ne l'emploie, déclaration l. 65, contrat l. 118) ; `help.c` 12 / 13 (D.7a : l. 20, 24, 33, 68, 70, 72, 90, 95, 266 ; D.6c : 206, 280 ; **reste à D.7b** : l. 251, l'`assert` sur `pico8_ob_lookup_constructor('v')`) ; `ui_button.c` 5 / 5 et `ui_button.h` 2 / 2 (pont `js`, branches `OB_TABLE`) ; `planet.c` 5 / 5 (l. 680, 694, 700, 702, 714) ; `planet_family_trade.c` 5 / 5 (l. 11, 383, 391, 393, 400) ; `mouse_reader.c` 6 / 6 ; `ui.c` 4 / 4 (l. 91, 98, 104, 106). **≈ 53 lignes.**

**TU** (lignes / occurrences `ob` à `f5d920c`, et ce qui les force) : `test_pico8_lq` 94 / 101 (ponts `lq`/`js`, `pico8_lq_table_view`, `gb`) ; `test_pico8_js` 48 / 58 (pont `js`, `pico8_ob(spec, extra)` l. 730, vue `dr` l. 906, `nb`, `gb`) ; `test_pico8_ui` 38 / 41 (`gb`, `g_`, boutons de `je`) ; `test_pico8_g_` 30 / 34 (dont `gb` 1 ; le reste est D.6c/D.7c) ; `test_pico8_pj` 28 / 32 (`ou`, `ea`, `de`, vues) ; `test_pico8_hp` 26 / 29 (`gb`, `hp:jb`, vues, `pico8_lq_table_view`) ; `test_pico8_fv` 16 / 20 (`rb`, `bo`) ; `test_pico8_trade` 16 / 20 (`gb`, `{qe,gc}`) ; `test_pico8_ie` 15 / 19 (D.6c) ; `test_pico8_ga` 14 / 14 (`gb`, `pico8_lq_table_view`) ; `test_pico8_jb_ol` 12 / 15 (pont `jb`) ; `test_pico8_entity_kind` 10 / 12 ; `test_pico8_hy` 9 / 11 ; `test_pico8_probe_mm_rnd_order` 8 / 10 (D.6c) ; `test_pico8_mx` 7 / 8 ; `test_pico8_data_init` 7 / 7 (`nb`, `fd`) ; `test_pico8_mh` 6 / 8 (pont `mh`) ; `test_pico8_ga_export_panel_by` 5 / 5 (§6) ; `test_pico8_nf` 5 / 5 ; `test_pico8_planet_gh_row_lq` 4 / 4 (§6) ; `test_pico8_os`, `_pm`, `_hj` 4 chacun ; `test_pico8_fq`, `_qh`, `_nl` 3 chacun ; `test_pico8_fr`, `_nr`, `_jo` 2 chacun. Les autres TU (`bg`, `fm`, `iv`, `jc`, `je`, `mo`, `mz`, `qb`, `spec_defaults`, `kj_dispatch`, `layout_nil_item_draws_nothing`) ne portent que des fixtures ob pour des API que D.7a ne change pas : D.7c. **Un TU n'est touché que pour les lignes qu'un type changé force** ; ses autres lignes `ob` restent à D.7c.

**Ce que dit le cart** (`pico8/lua.lua`) : `cb(bi)` (l. 1455) pose `t.e_=gi(t)<=fm.jk` sur **chaque** entrée de `nb` avant le filtre ; `gi(t)` (l. 1471) lit `t.od`, `t.nk` ; `hp:pq` range `self` dans `gb[px]` et `build_gb` dans `gb[i]` ; `jo:cn` (l. 479-496) lit et écrit `i.qe`, lit `i.gc`, préfixe `{qe=r.e}` ; `jo:ib` (l. 500) pousse dans `self.by` les `trade` rendus par `t:kg(self)` ; `pz(p,bs)` (l. 280) pose `b.ra`, `b.qf`, `b.fa` puis `js(b)` ; `bl(rb,nc)` (l. 597) appelle `oo(pi[e.fn],e)` sur chaque article ; `lq:mn(f)` (l. 300) lit un tuple `f` (`f.qe and {f.qe,5} or f`, puis `f[1..4]`).

**Faits qui engagent la forme** :

- **Les rangées générées de D.4a/D.4b sont déjà la forme typée** : `pico8_jb_row_t` (`items`, `c`, `fn`) porte `px`, `dz`, `em` de `jo`, `hp.fk`, `hp.lq`, `fv.rb`, `pj.i_`, `pj.gs`, `pj.ou`, `pj.ea`, les `lq` de `ix`/`ha`/`bo`/`bz`, l'`oy` de `nb`. Chaque vue que D.7a ferme n'existait que pour redonner à un lecteur ob ce que la rangée typée contient déjà.
- **Mutations persistantes** (« défaut muté ») : `cb` écrit `e_` sur les entrées de `nb` ; `pj:ph` écrit les items 1, 3 et 1..5 des rangées de `bz` (`st`) ; `pj:hb` écrit l'item 1 des sous-rangées de `ou` ; `pj:hk` écrit l'item 1 de `ea[2]`, pose ou retire `ea[3]`, pose `ea.lh` ; `menu` pose `my`/`hb` sur les rangées de `ix` et `ha`, `fv:pq` pose `my` sur celles de `bo` ; `pz` pose `ra`/`qf`/`fa` sur chaque rangée de `bs` ; `jo:cn` réécrit `i.qe` ; `economy_tiers` écrit `i.gc` ; `slipway_display` réécrit `ra`/`l` des particules de `jc.ts`. Chacune garde son observable exact.
- **`gb` est indexé deux fois** : par rang (1..34, ordre de `pico8_fd_table`) et par `px` (écrit par `hp:pq`, avant l'indice) ; `jo:pq` lit `gb[self.hp]` où `hp` est un nombre ou une chaîne.
- **`lq.items` reste une `pico8_list_t` de `pico8_value_t`** (point 3 de l'arrêt : D.9). Ce qui change, c'est le pointé de ses REF : aujourd'hui une table ob lue par `lq:mn` sans interroger son tag ; après D.7a une `pico8_jb_row_t`. Même règle pour `mh.items` et les listes de `bl`, `jo.jy`, `hp.i`, `ga.by` (pointé : un `pico8_trade_t`, déjà typé).
- **`class-coverage-net` compte les constructions** : un `lq` ou un `jb` construit en plus (par exemple un `lq` intermédiaire passé à `js:pq`, qui le recopie) changerait la sortie. Les sources de rangée restent donc des clés, jamais des instances ajoutées.
- **Les pointeurs non tagués ne doivent pas atteindre `pico8_type_tag_of`** : `pico8_js_lq` et `js:pq` dispatchent aujourd'hui par tag ; une `pico8_jb_row_t` n'a pas d'en-tête. La source typée passe par un champ typé, pas par un `pico8_value_t` REF.

## Solution

**Six commits (chunks 1 à 6), sans aucun pixel changé**, un par famille de données, chacun vert seul ; chaque famille est convertie avec **tous** ses producteurs et **tous** ses lecteurs dans le même commit (une donnée n'a jamais deux formes d'un commit à l'autre) :

1. **Familles isolées** : particules de `jc.ts` (`slipway_display`), résultat de `ky()` (`economy_score` → `score_panel_display`), états de bouton `nf.l`/`nf.r` avec `bu` (`mouse`, `mouse_reader`, `ui`), `pico8_ft_index`/`ft_name` supprimés, inclusion morte de `wormhole_display`, `pico8_data_init(void)`.
2. **`gb`** : tableau fixe de `pico8_hp_t *`, lu par rang et par `px` ; `jo:pq` (`planet.c`) et les ≈ 18 TU qui lisent `pico8_gb()` suivent.
3. **Enregistrements de souhait `{qe,gc}`** (`hp.i`, `jo.jy`) : struct typée ; `pico8_fi`, `pico8_mw`, `pico8_qj` quittent leurs appelants ; `jo:cn`, `economy_tiers`, `economy_formulas`, `planet_family_trade`, les lecteurs d'affichage suivent ; §6 `ga_export_panel_by`.
4. **Articles de `bl` et état du panneau de score** : `bl` lit des `pico8_jb_row_t` ; `pj.ou`, `pj.ea`, les rangées de `bz` deviennent des tableaux fixes non `const` ; vues par frame de `pj:kj` et `fv:kj` et vue par appel de `pj.gs[idx]` supprimées ; `mh` construit par clés typées ; pont `pico8_mh_new_table` fermé ; `pico8_oo` supprimé.
5. **Pipeline d'affichage et chaînon `pz`** : `pz` prend des clés de bouton typées ; rangées de `menu`, `planet_display`, `help`, `score_panel_display`, `cart` typées ; sources `lq`/`h_`/`jb` typées ; tuples de `lq:mn` en `pico8_jb_row_t` ; vues `ix`/`ha`/`bo`, `pico8_jb_row_view`, `jo_kw_view`, `hp:jb`, `pico8_lq_table_view` supprimées ; **ponts `lq`, `jb`, `js` fermés, `overrides` = 0, ajouté à `DATA_RESIDUE_GUARD`** ; §6 `gh_row_lq`.
6. **`nb`** : `cb` rend des entrées de `pico8_nb_table`, `e_` dans un tableau fixe non `const` ; `pico8_gi(ob)` supprimé ; vue `nb` et `pico8_nb()` fermées ; `data_init` n'a plus aucune table ob.

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert ; `make e2e` : **26 conformes** ; `make sanitize-net` : 26 propres ; `make class-coverage-net` : identique ; `make data-guard` : code 0 avec `DATA_RESIDUE_GUARD` = `spec,overrides` ;
- `make data-residue` : `overrides` 0 ; `spec` 0 ; `stash` 28 / 4 ; `ob` des 46 fichiers de la ligne = **R**, la liste close (≤ 15 lignes) ; `ob`, `valeur`, `type-tag` strictement plus bas ; comptes en lignes ET en occurrences, avant et après chaque chunk, par catégorie et par fichier ;
- **fuite par frame** (LeakSanitizer, section « Fuite par frame ») : sur `message_box_hold` et `game_end_final_score`, octets et allocations perdus jamais plus hauts qu'au départ après aucun chunk, **strictement plus bas** après le chunk 4 et au dernier ;
- ces `grep` ne rendent rien (après le dernier chunk) :

  ```sh
  grep -rnE 'pico8_(lq|jb|js|mh)_new_table|pico8_lq_table_view|pico8_jb_row_view|jo_kw_view' src tests --include='*.[ch]'
  grep -rnE 'pico8_(ix|ha|bo|bz|nb|gb|de)_view|pico8_(ix|ha|nb|bz|de)\(\)' src --include='*.[ch]'
  grep -rnE 'pico8_(ft_index|ft_name|qj|mw|oo|fi)\(|pico8_gi\(const pico8_ob' src --include='*.[ch]'
  grep -rnE 'g_(gb|bz|pj_ou|pj_ea|de|ix|ha|bo|nb|nf_l|nf_r)\b.*pico8_ob_table_t|pico8_ob_table_t \*g_' src --include='*.[ch]'
  grep -nE 'pico8_data_init\(pico8_mem_t' src/pico8_data_init.h
  sh tools/data_residue.sh --check overrides
  ```
- `sh tools/data_residue.sh --detail | awk -F'\t' '$1=="ob"'` ne rend, pour les 46 fichiers de la ligne, que les lignes de **R**.

## User Stories

1. En tant que mainteneur du port, je veux que les tables ad hoc de l'affichage, de l'économie, de la recherche et des données soient des structs locales, des tableaux fixes ou des rangées typées, afin qu'aucune de ces données ne soit plus lue ni écrite par nom dans une table ob.
2. En tant que mainteneur, je veux que chaque vue ob d'une rangée générée disparaisse (par appel, par frame, par init), afin que chaque lecteur lise la rangée typée qui existe déjà depuis D.4a/D.4b.
3. En tant que gardien de la mémoire, je veux que `pj:kj` et `fv:kj` ne construisent plus de vue à chaque frame, afin que la fuite par frame de leurs rangées imbriquées disparaisse.
4. En tant que gardien de la mémoire, je veux une mesure LeakSanitizer avant/après sur deux e2e, afin de prouver que la fuite par frame ne croît à aucun chunk et baisse à la fin.
5. En tant que gardien du comportement, je veux que chaque donnée mutée en cours de partie (`nb.e_`, `bz`, `pj.ou`, `pj.ea`, `jy`, particules, `nf.l`/`nf.r`) garde exactement ses écritures et ses relectures, afin que l'état affiché soit identique frame à frame.
6. En tant que gardien du comportement, je veux que `gb` rende le même `hp` par rang et par `px` qu'aujourd'hui (le dernier enregistré pour un `px` donné), afin que `jo:pq` et les écrans de recherche lisent les mêmes projets.
7. En tant que gardien du comportement, je veux que les enregistrements `{qe,gc}` distinguent `qe` absent et `gc` absent d'une valeur 0, afin que `jo:cn`, `economy_tiers` et l'affichage prennent les mêmes branches.
8. En tant que gardien de `lq:mn`, je veux qu'un tuple typé rende les mêmes `f`, `w`, `qw`, `qv` que la table qu'il remplace (dont `{qe=…}` → `{qe,5}`), afin que chaque rangée garde sa largeur et son dessin.
9. En tant que gardien de `pz`, je veux que `ra`, `qf` et `fa` soient posés sur les clés de chaque bouton dans le même ordre, avec le même vecteur `ra` partagé, afin que les boutons des panneaux soient au même endroit.
10. En tant que gardien de `class-coverage-net`, je veux qu'aucune instance `lq`, `jb`, `js`, `mh` ou `hp` ne soit construite en plus ou en moins, afin que la sortie reste identique.
11. En tant que gardien du compteur, je veux qu'`overrides` tombe à 0 avec la fermeture du dernier pont et entre dans `DATA_RESIDUE_GUARD` dans ce commit, afin que la catégorie soit gardée dès qu'elle est vide.
12. En tant que gardien du compteur, je veux que le reste `ob` des 46 fichiers de la ligne soit une liste close, nommée et justifiée par le test ponctuel, afin qu'un « 0 » manqué ne passe pas pour un oubli.
13. En tant que gardien du compteur, je veux les comptes avant/après chaque chunk en lignes ET en occurrences, par catégorie et par fichier, afin qu'une baisse prouve une construction retirée.
14. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, aucun identifiant compté renommé à type constant, aucun alias ni registre ajouté, afin qu'aucune baisse ne soit un tassement.
15. En tant que gardien du compteur, je veux qu'aucune vue ob, aucune `pico8_ob_table_new` et aucun appel à `pico8_jb_row_view` ne soient ajoutés, par appel, par frame ou par init, afin que la conversion ne soit pas une vue de plus.
16. En tant que gardien de la mémoire, je veux qu'une allocation typée par appel n'existe qu'à un site qui allouait déjà une table ob à cet appel, et qu'un contenu constant soit `static const`, afin que le tas porte moins qu'avant, jamais plus.
17. En tant que gardien des TU, je veux qu'un TU ne soit touché que pour les lignes qu'un type changé force, ses autres fixtures ob restant à D.7c, afin que D.7a reste une étape.
18. En tant que gardien des TU, je veux que chaque TU réécrit construise son état directement, sans registre, sans aide qui reconstruise une table par nom, sans rejouer d'événements, afin que le test dise ce qu'il construit.
19. En tant que gardien des TU, je veux qu'aucune constante `*_HASH` ne change, ni de valeur ni de nom, afin de prouver que les états comparés sont les mêmes.
20. En tant que gardien du §6, je veux que `ga_export_panel_by` pousse dans `ga.by` ce que `jo:ib` y pousse (des `trade`), et rougisse si `by` redevient une table ob, afin que sa garde reste celle du bug d'origine.
21. En tant que gardien du §6, je veux que `gh_row_lq` lise l'item `kw` typé de la rangée « exports: » et rougisse si la copie de `js:pq` lit la rangée au mauvais pas, afin que son intention (quatre items, dans l'ordre) reste intacte.
22. En tant que gardien du test ponctuel, je veux qu'il compare les tables générées à `pico8_ob()` sans passer par une vue supprimée, afin qu'il prouve toujours l'égalité entrée par entrée jusqu'à D.8.
23. En tant que gardien des filets, je veux `make e2e` à 26, `make sanitize-net` à 26 propres et `make class-coverage-net` identique à chaque chunk, afin de prouver qu'aucun pixel, accès mémoire ni instanciation n'a changé.
24. En tant que `plan-pilot`, je veux la décision d'ordre D.7a/D.7b écrite avec ses options rejetées et le texte de note qui modifie D.7b, D.7c et D.8, afin de l'enregistrer au lieu de la découvrir.
25. En tant que `plan-pilot`, je veux la liste des hypothèses H-D6c et des sections à relire contre la note de D.6c, afin de ne lancer D.7a sur aucune hypothèse démentie.
26. En tant que `plan-pilot`, je veux que D.7a ne touche dans les fichiers de D.7b que le chaînon `pz`, `nf.l`/`nf.r` et les lignes suiveuses nommées, afin que D.7b garde ses entités et son interface.
27. En tant que lecteur de `docs/glossaire-noms.md`, je veux une entrée par type nouveau (`{qe,gc}`, `gb`, article de `bl`, source de rangée, état des boutons), afin de retrouver le nom du cart depuis le nom C.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Convention (G)

- **Aucune forme nouvelle quand une forme existe** : une rangée est une `pico8_jb_row_t` (D.4a/D.4b), une clé de construction la struct de D.6a-D.6c (`pico8_lq_keys_t`, `pico8_js_keys_t`, `pico8_mh_keys_t`…), une donnée générée l'entrée de `pico8_game_data.h`/`pico8_map_tables.h` (`pico8_nb_entry_t`, `pico8_ix_entry_t`, `pico8_ha_entry_t`, `pico8_bo_entry_t`, `pico8_bz_entry_t`, `pico8_hp_start`…). Un type nouveau n'existe que pour une donnée qu'aucun type ne porte (enregistrement `{qe,gc}`, particule, état de bouton, résultat de `ky`, index `gb`).
- **Noms** : hors de tout motif figé (`_default`, `_spec`, `_defaults`, `overrides`, `_take_…(`, `…field…`, `pico8_ob…`, `pico8_value_…`) ; un type nouveau suit `pico8_<nom du cart>_t` (`pico8_jy_t`, `pico8_ts_t`, `pico8_ky_t`, `pico8_nf_button_t` : noms indicatifs, l'implémenteur garde la convention du module).
- **Durée de vie** : pile quand la donnée meurt avec l'appel (tuples passés à `lq:mn` par une rangée que `lq` **emprunte** ne meurent pas avec l'appel : voir « Allocation ») ; `static const` quand le contenu est constant ; tableau fixe statique non `const`, remis à l'init, quand il est muté (règle commune) ; une allocation typée par appel seulement là où une table ob était déjà allouée à cet appel.
- **Listes** : `pico8_list_t` reste (point 3 de l'arrêt, D.9) ; seul le type pointé par ses REF change, et chaque lecteur d'une liste dont le pointé change est dans le même commit.

### Allocation et fuite (G)

- **Contenu constant → aucune allocation** : les tuples littéraux (`{24,5}` de `cr`, `{false,1}` de `jo:gh`, `{false,-4}`, `{32,5}`/`{35,…}` de `research_display`), les rangées `{nk, px, c=6}` quand `nk`/`px` sont des chaînes de la donnée générée, les clés de `spec1`/`spec2` (`pj:ph`), `g_de` (une fonction `my`, déjà lue par `pico8_de_my()` depuis D.6b) deviennent `static const` ou des structs de pile recopiées par le constructeur qui les lit.
- **Contenu dynamique emprunté par une instance** (`lq.items` emprunte ses valeurs ; `mh.items` aussi) : **une** allocation typée (`pico8_jb_row_t` et son tableau d'items dans un même bloc) remplace la table ob et ses tableaux d'entrées ; jamais libérée comme elle (TROU-7), jamais à un site qui n'allouait rien. Chaque site est listé au commit (fichier, fonction, « avant : N `pico8_ob_table_new` ; après : 1 bloc typé »).
- **Vues supprimées, pas remplacées** : un lecteur d'une vue lit la rangée typée ; aucune conversion typée → ob ni ob → typée ne survit (une conversion est une vue).
- **`pz`** : le vecteur `ra` partagé par tous les boutons d'un même `pz` (« the same vector object, as in Lua ») reste **un** bloc par appel de `pz` ; `qf` un bloc par bouton (comme aujourd'hui) ; les clés de bouton que `pz` complète sont des copies de pile des rangées reçues (les rangées `ha`/`bo` ne portent plus `ra`/`qf`/`fa` entre deux appels : H-7).

### Forme par famille (G)

Forme de décision (types seulement pour la précision ; l'implémenteur reprend les types exacts des membres) :

```c
/* un enregistrement de hp.i / jo.jy : {qe=r[c]} a sa naissance, gc pose par economy_tiers */
typedef struct { int has_qe; pico8_fixed_t qe; int has_gc; pico8_fixed_t gc; } pico8_jy_t;

/* gb : les projets par rang ; gb[px] est le dernier hp enregistre sous ce px */
const pico8_hp_t *pico8_gb_at(int k);            /* 1..pico8_gb_count(), NULL hors bornes */
const pico8_hp_t *pico8_gb_find(const char *px); /* NULL si absent */

/* une particule de jc.ts */
typedef struct { pico8_vec_t ra; pico8_vec_t v; pico8_fixed_t l; } pico8_ts_t;

/* l'etat d'un bouton de nf (nf.l, nf.r), mute par bu */
typedef struct { int has_on; int on; int jn; int hm; } pico8_nf_button_t;

/* le resultat de ky() : s[1..6] */
typedef struct { pico8_fixed_t di, gl, el; const char *happiness; pico8_fixed_t total, tier; } pico8_ky_t;
```

| Famille | Aujourd'hui | Après D.7a | Producteurs | Lecteurs forcés |
|---|---|---|---|---|
| `jc.ts` | table `{ra,v,l}` par particule, `ra` réalloué à chaque frame | `pico8_ts_t` par particule, `ra` en place (plus d'allocation par frame) | `jc` (`slipway_display`) | `jc:lg` (`slipway_display`), TU `jc` |
| `ky()` | table rendue, libérée par l'appelant | `pico8_ky_t` par valeur ; la chaîne `happiness` reste allouée comme aujourd'hui (TROU-7) | `economy_score` | `pj:ph` (`score_panel_display`) |
| `nf.l`/`nf.r` | tables ob `g_nf_l`/`g_nf_r` | deux `pico8_nf_button_t` statiques non `const`, remis à `pico8_nf_init` ; `bu` les prend typés | `bu` (`mouse`) | `ui.c` (`jn`, `hm`), lecture par nom de `je.l`/`je.r` (rend un REF vers le bouton typé, compté `par-nom`) |
| `gb` | table ob (rang + `px`) | tableau fixe de `pico8_hp_t *` (capacité : les 34 projets de `pico8_fd_table`) et index `px` → rang, remis à `build_gb` | `hp:pq`, `build_gb` | `jo:pq` (`planet.c`), `research_display`, ≈ 18 TU |
| `hp.i`, `jo.jy` | liste de tables `{qe}`/`{qe,gc}` | liste de REF vers `pico8_jy_t` | `hp:pq` (boucle sur les caractères au lieu de `pico8_fi`), `jo:lz` (copie typée au lieu de `pico8_mw`), `jo:cn`, `economy_tiers` | `jo:cn`, `economy_tiers`, `economy_formulas` (`ck`), `planet_family_trade`, `research_display`, `planet_display`, TU |
| article de `bl` | table ob avec `fn` et `[1..5]`, ou vue | `const pico8_jb_row_t *` (`fn` + items) ; `pico8_jb_item_t` gagne **une** sorte REF opaque, pour le seul article `ld` (`lq` et vecteur `ra`) | `fv.rb`, `pj.i_`, `pj.ou`, `pj.ea`, `pj.gs[idx]`, rangées de `bz` (`pj:ph`) | `bl`/`pi` (chaque constructeur de `pi` lit l'item typé), `mh:ev` (liste inchangée) |
| état du panneau | vues `g_pj_ou`/`g_pj_ea`/`g_bz` par init, mutées | copies statiques non `const` des rangées générées (items et sous-rangées), remises à `pico8_score_panel_init` | `pj:hb`, `pj:hk`, `pj:ph` | `pj:kj`, `mh` |
| clés de `mh` | table ob `iw` → pont | `pico8_mh_keys_t` (items : REF vers les articles typés) | `pj:hk`, `pj:ph` | `mh` |
| rangée de `pz` | table ob par rangée | `pico8_js_keys_t` par rangée ; `pz` prend un tableau de clés (ou une liste de REF vers des clés), copie chacune sur la pile, pose `ra`/`qf`/`fa`, appelle `pico8_js_new` | `cr`, `pz(ha,ha)`, `jo:gh`, `jo:fe`, `fv:pq` | `pz` |
| sources `lq`/`h_` de `js` | `pico8_value_t` : table, liste, `lq` | champ typé : `const pico8_lq_keys_t *` (rangée à copier), liste, `lq`, nil/faux (présence TROU-23 inchangée) ; plus de branche `OB_TABLE` | les mêmes, `eu` (`ix`), `pj:ph` | `js:pq` |
| source `jb` de `js` | table ob de lignes → pont | `const pico8_jb_row_t *` (une ligne par item : chaîne ou rangée), lu par `pico8_jb_new` | `cr` (`oy` de `nb` ; « n invented » : les chaînes de `fm.invented`), `jo:fe` (`hp:jb`), `bo` | `js:pq`, `pico8_jb_new` |
| tuple de `lq:mn` | REF vers une table ob (`[1..4]`, `qe`) | REF vers `const pico8_jb_row_t *` ; `{qe=x}` produit directement `{x,5}` si `x` est vrai, sinon un tuple vide (mêmes `f`, `w`, `qv`) | `jo:gh` (`kw`, icônes, `h_`), `jo:fe`, `research_display`, `cr` | `lq:mn` |
| `ix`, `ha`, `bo` | vues par init, `my`/`hb` posés dessus | entrées générées + table `static const` des fonctions par rang (`my`, `hb`) ; `ha` : `pico8_ha_tail` pour `hi`/`gq`/`ca` | `menu_init`, `help_init` | `eu`, `ix4:my`, `fv:pq` |
| `nb` | vue par init, `e_` posé par `cb` | entrées de `pico8_nb_table` ; `e_` : tableau statique non `const` indexé par entrée, remis à l'init ; `cb` rend une liste de REF vers les entrées | `cb` | `cr`, `ix[2].hb`, `hy:kt` (déjà typé) |

- **`gb[px]`** : si deux projets partagent un `px`, la table ob rendait le **dernier** enregistré ; `pico8_gb_find` aussi (H-6 : les 34 `px` de `pico8_fd_table` sont distincts ; un `assert` à message le dit à `build_gb`).
- **`{qe,gc}`** : `qe` n'est jamais un booléen dans un enregistrement de souhait (`r[c]`, `r.e`, `hn.dm`, `t.qe` d'une route de `jo:ib`, fixe ou nil) : `assert` à message à l'écriture de `jo:cn`. L'égalité `pico8_value_equal(t_qe, qe_val)` devient une comparaison de présence et de fixe, même table de vérité.
- **Article `ld`** : **rejetés** — un `pico8_lq_t *` dans `pico8_jb_item_t` (ferait dépendre le module généré de `layout.h`) ; un type d'article distinct de la rangée (il faudrait convertir les rangées générées en articles : une vue) ; construire le `lq` au dessin (une instance par frame : `class-coverage-net` changerait).
- **Registre de `pi`** : son type de fonction prend l'article typé (`const pico8_jb_row_t *`), plus cinq `pico8_value_t` ; c'est un changement de signature, pas un renommage. Les enregistrements `'v'`, `'s'` (et `'b'` de `collect`) pour `pico8_ob()` gardent leurs constructeurs à valeurs : ils ne servent plus que le test ponctuel et `test_pico8_js`/`test_pico8_ob` (**R**). La fonction `spr` partagée par `'s'` et `"s"` est scindée en deux lecteurs d'un même appel `pico8_spr`, sans logique dupliquée au-delà de la lecture des arguments.
- **`pico8_ho_apply`** reste dans `collect.c` (**R**), appelée par le seul test ponctuel ; D.8 la supprime avec lui. **Rejeté** : la déplacer dans le test ponctuel (du code compté qui passe dans un fichier hors compte : tassement).
- **`pico8_data_init(void)`** : `mem` n'est plus lu depuis D.2 ; l'appel de `cart.c` et ceux de cinq TU (dont les deux §6, où seul l'argument change) suivent.

### Reste `ob` admis dans les 46 fichiers de la ligne (**R**, liste close, G)

| Fichier | Lignes admises | Pourquoi | Meurt à |
|---|---|---|---|
| `pi.c` | les deux `pico8_ob_register_constructor('v'…)`, `('s'…)` | `pico8_ob()` du test ponctuel, de `test_pico8_js`, de `test_pico8_ob` | D.8 |
| `pi.h` | le commentaire qui nomme les deux registres (≤ 2 lignes) | contrat encore vrai | D.8 |
| `collect.c` | `pico8_ho_apply` (≤ 6 lignes), `pico8_ob_register_constructor('b'…)` (1) | test ponctuel | D.8 |
| `collect.h` | inclusion de `pico8_ob.h`, déclaration de `pico8_ho_apply` (≤ 3 lignes) | idem | D.8 |

Toute autre ligne `ob` restante dans ces fichiers est un échec. `help.c` l. 251 (`pico8_ob_lookup_constructor('v')`) est dans un fichier de D.7b et y reste.

### Chunk 1 — familles isolées et `pico8_data_init(void)` (G)

- `jc.ts` (`slipway_display.c`, et `slipway.c` si l'inclusion de `pico8_ob.h` n'y sert plus) ; `ky` (`economy_score.c/.h`, et ses lectures `s[i]` dans `pj:ph`) ; `nf.l`/`nf.r` et `bu` (`mouse.c/.h`, `mouse_reader.c`, `ui.c`) ; `pico8_ft_index`/`ft_name` supprimés (`collect.c/.h`, aucun appelant) ; inclusion morte de `wormhole_display.c` ; `pico8_data_init(void)` (`data_init.c/.h`, `cart.c`, cinq TU).

### Chunk 2 — `gb` (G)

- `research.c/.h` : `g_gb` tableau fixe et index `px` ; `pico8_gb_at`, `pico8_gb_find`, `pico8_gb_count` (noms indicatifs) remplacent `pico8_gb()` ; `pico8_research_free_gb_instances` parcourt le tableau. `planet.c` (`jo:pq`) : `pico8_gb_at`/`pico8_gb_find` selon le genre de `hp` ; le commentaire l. 679-680 ne parle plus des primitives ob. `research_display` : le parcours de `gb` (l. 335) lit le tableau. TU : chaque `pico8_ob_table_get_index(pico8_gb(), k, &v)` devient `pico8_gb_at(k)`.

### Chunk 3 — enregistrements `{qe,gc}` et §6 `ga_export_panel_by` (G)

- `research.c` : `hp:pq` construit `hp.i` par une boucle sur les caractères de `i` (`pico8_r_lookup` par caractère, un `pico8_jy_t` par caractère, présent même si `r[c]` manque) ; `pico8_hp_pq_i_step` et l'appel de `pico8_fi` disparaissent. `planet.c` (`jo:lz`) : copie typée de chaque enregistrement (le cart copie par `mw` : une copie par enregistrement, jamais un partage) ; `pico8_mw`, `pico8_qj` supprimés (`collect.c/.h`, plus d'appelant). `planet_trade.c` (`jo:cn`), `planet_family_trade.c`, `economy_tiers.c`, `economy_formulas.c/.h`, `research_display.c` (l. 72, 177-180), `planet_display.c` (l. 110) lisent et écrivent les champs.
- **§6 `ga_export_panel_by`** : `route_a`/`route_b` deviennent deux `pico8_trade_t` construits comme `jo:ib` construit les siens (`pico8_trade_new(NULL)` ou clés typées de D.6c), poussés dans `by` ; l'`assert` de tag LIST sur `by` et les deux lectures « 0/2 », « 2/2 » restent. Preuve rouge : `ga` reçoit localement un `by` qui est une table ob vide castée (le bug d'origine, `by=o()`), le test rougit, correctif restauré ; consignée au commit.

### Chunk 4 — articles de `bl`, panneau de score, pont `mh` (G)

- `pi.c/.h` : `bl`/`bl_default` lisent des `const pico8_jb_row_t *` ; le registre par chaîne prend l'article typé ; `pico8_oo` supprimé (`collect.c/.h`). `pico8_jb_item_t` gagne la sorte REF opaque (`pico8_map_tables.h`) ; le test ponctuel ignore ce champ (aucune donnée générée ne le pose).
- `score_panel.c/.h` : `g_pj_ou`, `g_pj_ea`, `g_bz` deviennent des copies statiques non `const` (items et sous-rangées) des rangées générées, remises à `pico8_score_panel_init` ; `pico8_pj_ou()`/`pico8_pj_ea()`/`pico8_bz()` rendent ces rangées typées ; `pj:kj` passe `pj.i_` à `bl` **sans vue** (plus d'allocation par frame) ; `pj:hb` écrit l'item 1 de chaque sous-rangée de `ou`.
- `score_panel_display.c` : `pj:hk` écrit `ea` typé et construit `mh` par `pico8_mh_keys_t` ; `pp` pointe la rangée de `gs` sans vue ; `pj:ph` écrit les rangées de `bz` typées, construit chaque `lq` par `pico8_lq_keys_t` (plus de `pico8_lq_new_table(st)`) et l'article `ld` typé, puis `mh` par clés.
- `help.c` : `fv:kj` passe `fv.rb` à `bl` sans vue (plus d'allocation par frame).
- `msgbox.c/.h` : `pico8_mh_new_table` supprimé (plus d'appelant de `src/`), avec ses lignes de `test_pico8_mh` (7 lignes `overrides`, « tombent avec lui », note de D.6b).
- `data_init.c/.h` : `pico8_bz_view` supprimé ; le test ponctuel compare `pico8_bz_table` à `ob()` par une aide locale (plus bas).

### Chunk 5 — pipeline d'affichage, chaînon `pz`, ponts `lq`/`jb`/`js` (G)

- `msgbox.c/.h` : `pz(p, keys, n)` (forme indicative) ; `pz_step` copie la clé, pose `ra` (partagé), `qf` (un bloc), `fa` (TROU-9, sous la même condition), appelle `pico8_js_new` ; le préfixe `ep` (`{lq={ep,c=13}}`) est une clé de pile dont la source `lq` est une `pico8_lq_keys_t` de pile.
- `menu.c/.h` : `cr_step` construit une `pico8_js_keys_t` (`lq` : clés `{nk, px, c=6}` ; `h_` : `{{24,5}, gi}` avec tuple `static const` ; `jb` : l'`oy` typé de l'entrée — `pico8_nb_lookup(px)` comme pour `_cr_t` ; `my`/`_cr_t` comme aujourd'hui) ; la rangée « n invented » (`jb` : une rangée d'items chaîne de `fm.invented`) ; `ix`/`ha` lus depuis `pico8_ix_table`/`pico8_ha_table` avec leurs fonctions par rang ; `g_ha_pz` lit `pico8_ha_tail` ; `pico8_ix()`/`pico8_ha()` rendent les entrées typées ou disparaissent. `cart.c` (`eu`) construit ses boutons depuis `pico8_ix_table`.
- `planet_display.c` : `jo:gh` et `jo:fe` construisent leurs clés ; icônes, `em`, `hp.lq`, `hp.fk` lus comme rangées typées ; `jo_kw_view` supprimé (tuple `{qe,5}` ou vide) ; `hp:jb` (`research_display.c/.h`) rend une `pico8_jb_row_t` (une allocation, là où deux tables ob l'étaient).
- `research_display.c` : la rangée `l` et ses tuples typés ; vues de `em` et `fk` supprimées.
- `score_panel_display.c/.h` : `spec1`/`spec2` deviennent des clés `static const` ou de pile (`fa=15`, `ra`, `lq`) ; `g_de` et `pico8_de()` disparaissent (`pico8_de_my()` existe, D.6b) ; `test_pico8_fq`/`test_pico8_pj` suivent.
- `help.c` : `fv:pq` construit les clés de `bo` depuis `pico8_bo_table` (fonction `my` = `eu`) ; `g_bo` et `pico8_bo_view` disparaissent.
- `layout.c/.h` : `lq:mn` lit le tuple typé ; `pico8_lq_new_table`, `pico8_jb_new_table`, `pico8_layout_positional_values`, `pico8_lq_table_view` supprimés ; `jb:pq` ne lit plus de table. `ui_button.c/.h` : `pico8_js_new_table` supprimé, sources typées, plus de branche `OB_TABLE`.
- `data_init.c/.h` : `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_jb_row_view` supprimés.
- `DATA_RESIDUE_GUARD` = `spec,overrides` (`Makefile`), dans ce commit.
- **§6 `gh_row_lq`** : l'item 3 est lu comme le tuple typé que `jo:gh` pose pour `kw` (`{99,5}`) ; les assertions « quatre items, dans l'ordre, pas de cinquième, `fs` à 4 » restent. Preuve rouge : la copie de `js:pq` (`pico8_lq_keys_of` → items) lit un item de trop (le compte de la source plus un) ou au mauvais pas, le test rougit, correctif restauré.

### Chunk 6 — `nb`, fin de `data_init` (G)

- `tech_gate.c/.h` : `cb` parcourt `pico8_nb_table`, écrit `e_` dans le tableau statique pour **chaque** entrée avant le filtre (commentaire existant l. 109-111 gardé), rend une liste de REF vers les entrées ; `pico8_gi(ob)` supprimé, `pico8_gi_nb` est l'unique `gi` (renommé `pico8_gi` seulement si son type change ; sinon il garde son nom). `menu.c` : `cr_step` lit l'entrée typée et son `e_`. `data_init.c/.h` : `g_nb`, `data_init_nb_view`, `pico8_nb()` supprimés ; plus aucune inclusion de `pico8_ob.h`. TU : `test_pico8_data_init` (`PICO8_NB_COUNT == 38` et la première entrée), `test_pico8_js` (l. 1459), test ponctuel (`nb` comparé à `ho(nb,…)` d'`ob()` par l'aide locale).

### `Makefile`, docs

- `Makefile` : `DATA_RESIDUE_GUARD` gagne `overrides` au chunk 5 ; aucun fichier ajouté ni retiré des listes de sources (sauf si un module se vide entièrement : dit au commit). `tools/` ne change pas.
- `docs/glossaire-noms.md` : entrées des types nouveaux (`jy`, `gb`, `ts`, `nf.l`/`nf.r`, `ky`, article de `bl`) ; lignes `ix`, `ha`, `bo`, `bz`, `nb` sans vue.
- `docs/trous-de-portage.md` : aucune entrée ne change ; TROU-3, TROU-4, TROU-7, TROU-9, TROU-23 gardés à leurs sites.
- `docs/plan-3.md` : note « Découvert à D.7a » en fin de ligne D.7a (texte dans Further Notes). Rien d'autre dans le plan.

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucun ordre de construction, aucun tirage du RNG (`jc` : le `rnd` précède la particule, gardé), aucune valeur.
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements ; aucun e2e ajouté.
- Les entités et l'interface de D.7b hors du chaînon `pz` et des lignes suiveuses nommées ; `pico8_ob.c/.h`, `pico8_stash.*` ; `KNOWN[]` ; les écrivains et lecteurs par nom (`par-nom`, D.9) sauf ceux dont le seul objet était une table convertie ; `layout_nil_item_draws_nothing`.

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec.

- **Interdit** : joindre sur une ligne des déclarations, instructions, arguments, conditions, champs ou entrées d'initialiseur qui étaient sur des lignes distinctes ; retirer des retours à la ligne dans une ligne comptée ; renommer un identifiant, un paramètre ou un commentaire compté **sans que le type de la donnée change** ; macro, `typedef`, pointeur ou fonction enveloppe dont le seul rôle est de renommer un identifiant compté ; **un alias** qui ferait passer une table ob à une API typée ; **un registre** (tableau, table, `switch` ou boucle nouveaux qui nomment ensemble classes, clés ou valeurs attendues) dans un TU ; une aide de TU qui reconstruit une table ou une rangée par nom ; déplacer du code compté hors de `src`/`tests`, hors `.c`/`.h`, ou dans le test ponctuel ; supprimer un commentaire qui ne porte pas sur une table, une vue ou un pont retirés ; **toute vue ob nouvelle, toute `pico8_ob_table_new` et tout appel à `pico8_jb_row_view` ajoutés, par appel, par frame ou par init** ; toute conversion typée → ob ou ob → typée ; une allocation typée à un site qui n'allouait rien ; un double type durable (une donnée qui a deux formes à la fin d'un chunk).
- **Légitimité, contrôle exigé** : chaque ligne `ob` retirée est, au commit de son chunk, classée (a) construction supprimée (allocation, pose, lecture par nom, libération, boucle sur les entrées, vue, pont, aide morte, `assert` de tag `OB_TABLE`, inclusion devenue inutile, commentaire d'une table qui n'existe plus) ou (b) signature, déclaration ou contrat dont le type passe d'une table ob à un type C ; aucune ligne (c) « même type, autre nom ». Recette : `git diff -U0` de chaque fichier, lignes `-` qui portent le motif `ob` en regard des lignes `+`.
- **Mesure exigée, avant et après chaque chunk** : par catégorie, **lignes** (`make data-residue`, et `--detail` par fichier) **ET occurrences**, le test ponctuel donné à part. Recette figée (`$S` : dossier du scratchpad) :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  X=$(sed -n "s/^SPEC_EXCLUSION='\(.*\)'\$/\1/p" tools/data_residue.sh)
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E "$X" "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  ```

  Mesuré sur `f5d920c` : `valeur` 6438, `ob` 1625, `stash` 28, `type-tag` 654, `ref-audit` 69, `par-nom` 1117, `dispatch` 201, `overrides` 138, `spec` 0 (le départ réel est celui du commit gradé de D.6c).
- **Contrôle** : pour chaque catégorie et chaque fichier du diff, Δlignes et Δoccurrences. **Si Δlignes > Δoccurrences** dans un couple (catégorie, fichier), le vérificateur lit le diff de ce fichier : chaque ligne retirée correspond à une construction retirée ou convertie, pas à une construction qui réapparaît sur une autre ligne. Le message de commit liste ces couples et leur justification.
- **Lignes qui gagnent une catégorie**, listées au commit ; attendues : quelques-unes en `valeur` (conversion d'un item typé en argument `pico8_value_t` d'un constructeur de `pi`, lecture par nom d'un membre converti), chacune justifiée ; aucune en `ob`, `overrides`, `spec`.
- **Fichiers de D.7b** : seules les lignes nommées dans « Suiveurs forcés » changent ; tout autre changement dans un fichier de D.7b est un échec.

### Empreinte remesurée (G)

| Chunk | `src/` | `tests/` | docs / `Makefile` | Total |
|---|---|---|---|---|
| 1 | `slipway_display.c`, `slipway.c`?, `economy_score.c/.h`, `score_panel_display.c`, `mouse.c/.h`, `mouse_reader.c`, `ui.c`, `collect.c/.h`, `wormhole_display.c`, `data_init.c/.h`, `cart.c` (≈ 15) | `jc`, `nf`, `ui`, `pj`?, `data_init`, `map_tables`, `jb_ol`, `ga_export_panel_by`, `planet_gh_row_lq` (≈ 9) | glossaire | ≈ 25 |
| 2 | `research.c/.h`, `research_display.c`, `planet.c` (4) | `entity_kind`, `fr`, `g_`, `ga`, `hj`, `hp`, `hy`, `jo`, `js`, `lq`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `trade`, `ui` (18) | glossaire | ≈ 23 |
| 3 | `research.c`, `planet.c`, `planet_trade.c`, `planet_family_trade.c`, `economy_tiers.c`, `economy_formulas.c/.h`, `research_display.c`, `planet_display.c`, `collect.c/.h` (11) | ceux qui construisent ou lisent `jy`/`hp.i` (≈ 12 : `trade`, `mx`, `hy`, `ui`, `hp`, `ga`, `os`, `qh`, `jo`, `nl`, `pm`, `hj`…), `ga_export_panel_by` | glossaire | ≈ 25 |
| 4 | `pi.c/.h`, `collect.c/.h`, `map_tables.h`, `score_panel.c/.h`, `score_panel_display.c`, `help.c`, `msgbox.c/.h`, `data_init.c/.h` (13) | `pj`, `fv`, `mh`, `mo`?, `map_tables` (≈ 5) | glossaire | ≈ 19 |
| 5 | `msgbox.c/.h`, `menu.c/.h`, `planet_display.c`, `research_display.c/.h`, `score_panel_display.c/.h`, `help.c`, `cart.c`, `layout.c/.h`, `ui_button.c/.h`, `data_init.c/.h` (17) | `lq`, `js`, `jb_ol`, `ui`, `hp`, `ga`, `fv`, `pj`, `fq`, `mh`, `planet_gh_row_lq`, `map_tables` (12) | glossaire, `Makefile` | ≈ 31 |
| 6 | `tech_gate.c/.h`, `menu.c`, `data_init.c/.h` (5) | `data_init`, `js`, `map_tables`, `hy`? (≈ 4) | glossaire, `plan-3.md` | ≈ 11 |
| **Distincts** | **≈ 40** | **≈ 35** | **3** | **≈ 78-95** |

- **Écart avec la ligne (39 : 39, 0)** : la ligne comptait les fichiers de `src/` qui portaient une ligne `ob` ; elle ne voyait ni les lecteurs forcés (`planet.c`, `planet_family_trade.c`, `ui.c`, `mouse_reader.c`), ni le chaînon `pz` pris à D.7b (`msgbox`, `help`, `ui_button`), ni les TU qui lisent `gb` (≈ 18) ou les ponts, ni le test ponctuel. Chaque chunk reste sous ~40 chemins ; l'implémenteur remesure au départ réel (après D.6c) et donne l'écart au commit (≤ 5 par chunk, expliqué).
- **Découpage (G) : six commits**, dans cet ordre de dépendance : 1 (indépendant) ; 2 avant 3 (`hp:pq` écrit `gb` puis `hp.i` : deux conversions distinctes du même constructeur, un rouge à la fois) ; 4 avant 5 (les articles de `bl` et l'état du panneau sont des rangées typées que 5 suppose pour ses sources `lq` de `pj:ph`) ; 5 avant 6 (`cr` lit l'`oy` typé dès 5 ; `cb` ne peut rendre des entrées typées qu'une fois `js:pq` capable de les lire). **Rejetés** : **un commit** (≈ 90 chemins, un rouge bloquerait tout) ; **un commit par fichier** (un type changé toucherait deux commits : double forme) ; **`nb` d'abord** (son `oy` typé n'a aucun lecteur avant 5) ; **5 scindé en « rangées de `pz` » puis « sources »** (entre les deux, les sources resteraient des tables ob portées par des clés typées : il faudrait une vue ou une double forme dans `js:pq`) ; **transitions à double forme** (D.4a en a eu ; ici chaque famille tient dans un commit).

### Effet sur `make data-residue`

Attendu, lignes (départ : commit gradé de D.6c, noté D ; les valeurs de `f5d920c` entre parenthèses) :

| Catégorie | Départ | Après D.7a | Pourquoi |
|---|---|---|---|
| `overrides` | 30 ; 34 ; 8 (H-D6c-1) | **0**, dans `DATA_RESIDUE_GUARD` au chunk 5 | les quatre ponts et leurs 9 lignes de TU tombent : `mh` au chunk 4 (30 → 18 : `msgbox.c` 4, `msgbox.h` 1, `test_pico8_mh` 7), `lq`/`jb`/`js` au chunk 5 (18 → 0) |
| `spec` | 0 | **0** | garde |
| `stash` | 28 / 4 | **28 / 4** | le test ponctuel garde ses 2 lignes |
| `ob` (46 fichiers de la ligne) | ≈ 455 (515 à `f5d920c`, − ≈ 60 par D.6c) | **R ≤ 15** | liste close |
| `ob` (fichiers de D.7b) | 128 − D.6c | − ≈ 53 | chaînon `pz`, `nf`, suiveurs |
| `ob` (TU hors test ponctuel) | 450 − D.6c | − 150 à 250 (estimé) | seulement les lignes forcées |
| `ob` hors test ponctuel | D (1280 à `f5d920c` ; majorant décalé ≤ 1096) | **≈ D − 650 à 750**, très sous le majorant | somme des trois lignes précédentes |
| `valeur` | rapporté | **strictement plus bas** | poses `pico8_value_*` des tables retirées ; quelques lignes gagnées aux constructeurs de `pi`, listées |
| `type-tag` | rapporté | **strictement plus bas** | `assert` de tag `OB_TABLE` retirés (`menu`, `pz`, `fv:pq`, `pj:hk`, `pj:ph`, `jo:cn`…) |
| `par-nom` | ≈ 1038 (H-D6c-5) | ≤ départ, écart expliqué | `ui_je_button_field_bool`, `button_take_*` morts avec le pont `js` ; lecture par nom de `je.l`/`je.r` gardée |
| `dispatch` | rapporté | ≤ départ | les `pico8_value_function` des rangées de `pz` deviennent des champs de clé (même type : compté pareil) ; aucun ajouté |
| `ref-audit` | rapporté | ≤ départ | |

- **`overrides` = 0 est attendu exact** ; il entre dans `DATA_RESIDUE_GUARD` dans le commit du chunk 5. Le compte intermédiaire après le chunk 4 est remesuré et donné au commit (les 4 lignes du pont `mh` dans `msgbox.c`, sa déclaration, les 7 lignes de `test_pico8_mh`).
- **`ob` des 46 fichiers = R** est attendu exact au sens de la liste close ; tout écart est un échec.
- Les comptes de la ligne (« ≤ 1057 ») sont tenus largement parce que D.7a absorbe le chaînon `pz` et des suiveurs ; la note donne le compte réel et le nouveau majorant de D.7b.

### Hypothèses sur D.6c

- **H-D6c-1** : D.6c est livrée et gradée comme son PRD le dit : `overrides` = **30 lignes / 34 occurrences / 8 fichiers** (21 / 25 / 6 des quatre ponts, 9 / 9 / 2 de `test_pico8_mh` et `test_pico8_lq`, texte inchangé), hors de `DATA_RESIDUE_GUARD` (= `spec`), et la note dit que `overrides` y entre « à l'étape qui ferme le dernier pont ».
- **H-D6c-2** : pont `pico8_fd()` fermé (`g_fd`, `data_init_fd_view`, `pico8_fd()` supprimés) ; `pico8_research_build_gb(void)` parcourt `pico8_fd_table` ; **`g_gb` reste une table ob** et `pico8_gb()` garde sa signature.
- **H-D6c-3** : `pico8_ho_apply` reste dans `collect.c`, sans appelant de `src/`, appelée par le test ponctuel (`nb`, `df`) ; `pico8_ho_gp_has` est publique dans `collect.h`.
- **H-D6c-4** : clés typées `pico8_g__keys_t`, `pico8_ie_keys_t`, `pico8_trade_keys_t` (triple `qe`), `pico8_hp_keys_t` (`ri`, `rr`) ; `pico8_hy_new(void)`, `pico8_fv_new(void)`, `pico8_pj_new(void)` ; `eu` (`cart.c`) et `jo:ib` (`planet_trade.c`) n'allouent plus de table de clés.
- **H-D6c-5** : `par-nom` ≈ **1038** (départ − 24, ± 3).
- **H-D6c-6** : D.6c retire ≈ **60** lignes `ob` des 46 fichiers de la ligne D.7a (`cart.c` 5, `data_init.c` 15, `data_init.h` 1, `economy.c` 2, `economy.h` 2, `planet_trade.c` 6, `research.c` 24, `research.h` 2, `score_panel.c` 2, `score_panel.h` 1), d'où un départ ≈ 455 ; et 2 dans `help.c` (l. 206, 280), 1 dans `help.h`.
- **H-D6c-7** : D.6c n'ajoute ni vue, ni `pico8_ob_table_new`, ni appel à `pico8_jb_row_view` ; les vues et producteurs de rangées ob de `f5d920c` sont intacts (`jo:gh`, `jo:fe`, `cr`, `pz`, `hp:jb`, `ix`/`ha`/`bo`/`bz`/`nb`, `g_pj_ou`/`g_pj_ea`, `pj:kj`, `fv:kj`).
- **H-D6c-8** : D.6c ne touche pas `hp.i` : `hp:pq` le construit toujours par `pico8_fi` et des enveloppes `{qe=r[c]}` ob.
- **H-D6c-9** : les numéros de ligne de ce PRD sont ceux de `f5d920c` ; D.6c les déplace dans `cart.c`, `research.c`, `planet_trade.c`, `score_panel.c`, `economy.c`, `help.c`, `data_init.c`, `collect.c`, `test_pico8_hp`, `test_pico8_data_init`, le test ponctuel : chaque site est retrouvé par son texte.
- **H-D6c-10** : `test_pico8_hp`, `test_pico8_data_init` et le test ponctuel ne nomment plus `pico8_fd()` ; `test_pico8_data_init` garde `pico8_nb()->count == 38`.
- **H-D6c-11** : l'arbre de D.6c est vert (`make test`, `make e2e` 26, `make sanitize-net` 26, `make class-coverage-net` identique, `make data-guard` 0).
- **H-D6c-12** : D.6c ne touche pas `planet.c`, `planet_family_trade.c`, `ui.c`, `ui_button.c`, `msgbox.c`, `layout.c`, `menu.c`, `planet_display.c`, `research_display.c`, `score_panel_display.c`, `tech_gate.c`, `pi.c`, `mouse*.c`, `slipway_display.c`, `economy_*.c`.

**Démentie** : l'hypothèse fausse est nommée au commit, la section concernée de ce PRD est relue ; seule une ligne `ob`/`overrides` des fichiers de D.7a touchée par D.6c oblige à recalculer un compte attendu, jamais à l'abaisser par renommage.

### À revérifier après D.6c (contre sa note « Découvert à D.6c »)

1. **Inventaire / tableau par fichier** : départ réel de `ob` dans les 46 fichiers (≈ 455, H-D6c-6), comptes globaux (lignes et occurrences), numéros de ligne (H-D6c-9).
2. **Décision d'ordre** : `overrides` de départ (30 / 34 / 8) et phrase de la note sur l'étape qui posera le garde (H-D6c-1) — si D.6c a fermé un pont, le chunk qui pose le garde change.
3. **Chunks 2 et 3** : forme de `g_gb` et de `build_gb` après D.6c (H-D6c-2), construction de `hp.i` (H-D6c-8).
4. **Reste R** : `pico8_ho_apply` et `pico8_ho_gp_has` (H-D6c-3).
5. **Chunk 3 / §6 `ga_export_panel_by`** : forme de `pico8_trade_new` (H-D6c-4) pour construire `route_a`/`route_b`.
6. **Effet sur `make data-residue`** : départs de `par-nom` (H-D6c-5), `ob`, `valeur`, `type-tag`, `dispatch`.
7. **Empreinte remesurée** : fichiers que D.6c aurait touchés hors de son PRD (H-D6c-12) ; TU réécrits par D.6c (H-D6c-10).
8. **Critères d'échec** : l'arbre de départ (commit gradé de D.6c) et ses comptes.

### Hypothèses sur D.7a (rien n'a été lancé)

- **H1** : aucun lecteur d'une table convertie hors de l'inventaire (grep sur `f5d920c` : `src/` et les TU listés ; aucun pointeur de fonction ni lecture par nom d'un champ de ces tables hors des sites nommés).
- **H2** : les tuples typés rendent à `lq:mn` les mêmes `f`, `w`, `qw`, `qv` : un `nil` en position 1 et un `false` prennent la même branche (`f` faux), et aucun producteur ne pose un `nil` ailleurs qu'en position 1 (sinon une sorte « nil » est ajoutée à `pico8_jb_item_t`, justifiée au commit).
- **H3** : `lq_keys_of`/`js:pq` recopient les items d'une source typée dans le même ordre et au même nombre que la table ob ; aucune instance `lq`/`jb`/`js`/`mh` n'est ajoutée ni retirée (`class-coverage-net` identique).
- **H4** : les 34 `px` de `pico8_fd_table` sont distincts : `gb[px]` et `gb[i]` désignent le même `hp` ; l'ordre de parcours de `gb` par rang est celui de `build_gb` (les TU qui plient `gb` en hachage lisent le même ordre).
- **H5** : un enregistrement de souhait ne porte jamais un `qe` booléen.
- **H6** : les rangées `ha`/`bo` ne sont relues nulle part après `pz` (le `ra`/`qf`/`fa` que `pz` y posait n'a aucun autre lecteur) ; `my` posé par `fv:pq`/`menu_init` n'a d'autre lecteur que les boutons construits.
- **H7** : `pj:hk`, `pj:hb`, `pj:ph` ne lisent de `ea`, `ou`, `bz` que les items qu'ils écrivent et que `bl`/`mh`/`lq` dessinent ; une copie statique remise à l'init rend les mêmes valeurs que la vue reconstruite à l'init.
- **H8** : aucune constante `*_HASH` ne bouge. Démentie : arrêt sur la constante, jamais de mise à jour.
- **H9** : `make sanitize-net` reste propre : les données passent du tas au statique ou à la pile sans pointeur pendant (les tuples empruntés par un `lq` restent vivants : statiques ou alloués).
- **H10** : LeakSanitizer tourne dans le conteneur (`detect_leaks=1`) ; sinon repli prévu (section « Fuite par frame »).
- **H11** : `-Werror` reste muet ; aides mortes supprimées, inclusions devenues inutiles retirées.
- **H12** : `constants-check`, `float-grep`, `float-guard` restent verts.
- **H13** : durées courtes : `make test` quelques secondes plus une recompilation large ; `make e2e` ~1 s ; `sanitize-net` et `class-coverage-net` < 2 min chacun ; mesure de fuite < 1 min par e2e.
- **H14** : les 26 e2e atteignent `cr`, `pz(ha,ha)`, `jo:gh`, `jo:fe`, `fv:pq`, `fv:kj`, `pj:kj`, `pj:hk`, `pj:ph`, `jc:lg`, `bu`, `jo:cn`, `economy_tiers` ; un site non atteint est prouvé par un TU.
- **H15** : `make raylib` (non exigé, jamais lancé) lierait.

## Implementation Chunks

Six commits, dans cet ordre, chacun vert seul (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`, `make data-guard`, mesure de fuite), chacun avec ses comptes avant/après, sa classification et son rapport gcov/mutations :

1. **Familles isolées** — `jc.ts`, `ky`, `nf.l`/`nf.r` + `bu`, `ft_index`/`ft_name`, inclusion de `wormhole_display`, `pico8_data_init(void)`.
2. **`gb`** — tableau fixe et index `px` ; `jo:pq` ; ≈ 18 TU.
3. **`{qe,gc}`** — `hp.i` sans `pico8_fi`, `jo:lz` sans `pico8_mw`, `jo:cn`, `economy_tiers`, `economy_formulas`, `planet_family_trade`, lecteurs d'affichage ; `pico8_qj`/`pico8_mw` supprimés ; §6 `ga_export_panel_by`.
4. **Articles de `bl`, panneau de score, pont `mh`** — `bl` sur `pico8_jb_row_t`, `pico8_oo` supprimé, `ou`/`ea`/`bz` statiques, vues par frame de `pj:kj` et `fv:kj` supprimées, `mh` par clés, `pico8_mh_new_table` et `pico8_bz_view` supprimés.
5. **Pipeline d'affichage et chaînon `pz`** — rangées et sources typées, tuples de `lq:mn`, `ix`/`ha`/`bo`, vues à l'appel, ponts `lq`/`jb`/`js` fermés ; `overrides` = 0 dans `DATA_RESIDUE_GUARD` ; §6 `gh_row_lq`.
6. **`nb`** — `cb` typé, `e_` statique, `pico8_gi(ob)` et `pico8_nb()` supprimés ; note du plan.

Ordre conseillé dans chaque commit : mesure de départ (lignes et occurrences par fichier ; fuite) ; types et producteurs ; lecteurs de `src/` ; `make test` ; TU ; `make test` ; `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois ; gcov, mutations ; mesure d'arrivée (comptes, fuite), classification, anti-tassement, gardes ; docs.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, identités, pixels, instances), pas la présence d'une ligne. D.7a change la forme des données, pas ce qu'elles valent : **la preuve est que chaque producteur rend, et chaque lecteur lit, les mêmes valeurs qu'avant, et que tout ce qui mesure le comportement rend la même chose** — plus une mesure de fuite qui ne croît jamais.

### TU touchés (G)

- **Règle** : un TU n'est touché que pour les lignes qu'un type changé force (appel d'une API supprimée ou retypée, lecture d'une donnée dont le type change) ; ses autres fixtures ob restent à D.7c. Chaque état construit l'est directement (struct locale, rangée `static const` du TU, appel du producteur réel), un champ par ligne ; aucune aide nouvelle qui reconstruise une table ou une rangée par nom ; aucun registre ; aucun rejeu d'événements. Les aides existantes sont retypées, sans cas ajouté.
- **`gb`** : `pico8_ob_table_get_index(pico8_gb(), k, &v)` → `pico8_gb_at(k)` ; `pico8_gb()->entries[0]` (`test_pico8_jo` l. 489, qui lit aujourd'hui l'entrée **nom** du premier projet) → `pico8_gb_find(px du premier)` ou `pico8_gb_at(1)`, même `hp` (dit au commit).
- **Ponts** : `test_pico8_mh` (7 lignes `overrides`, `test_mh_nil_lh_key_reads_absent`, `build_with_key`) et `test_pico8_lq` (commentaires l. 258, 281) « tombent avec » leur pont (note de D.6b) : leurs vérités (clé `lh` nil ≡ absente, clés refusées) sont portées par le type de `pico8_mh_keys_t` ou retirées si le type les rend impossibles à écrire, listées au commit avec leur raison. Les tests de `pico8_lq_new_table`/`pico8_jb_new_table`/`pico8_js_new_table` (`test_pico8_lq`, `test_pico8_jb_ol`, `test_pico8_js`) passent par les clés typées quand l'intention existe encore (même rangée dessinée, même copie), sont retirés et listés sinon.
- **`pico8_lq_table_view`** (`test_pico8_lq` l. 718, 736 ; `test_pico8_ga` l. 452 ; `test_pico8_hp` l. 219) : les TU lisent les membres de `pico8_lq_t` (`items`, `c`, `bg`, `score`, `ra`, `fs`, `w`) ; même comparaison, même constante.
- **Constantes** : aucune constante `*_HASH` ne change, ni de valeur ni de nom. Un hachage qui pliait une vue (`fold_table` de `test_pico8_pj`/`test_pico8_fv`) plie la rangée typée **dans le même ordre et avec les mêmes octets par item** ; si c'est impossible sans changer la constante, arrêt (§7).

### Le test ponctuel `tests/test_pico8_map_tables.c` (G)

- Ses comparaisons par vue (`check_view(pico8_ix_view(), …)`, `bo`, `bz`, `ha`, `pico8_jb_row_view(&pico8_dr_table[i])`, l. 213, 556-568, 832, 1539 à `f5d920c`) deviennent une comparaison **récursive** d'une rangée ou d'une entrée typée à la table que `pico8_ob()` parse, par une aide locale au test (rang par rang, sous-rangée par sous-rangée, `c`/`fn`/clés nommées comprises) — un outil de comparaison, **aucune table ob construite depuis la donnée typée** (précédent : `fd` à D.6c).
- `nb` : comparaison de `pico8_nb_table` à `ho(nb, …)` d'`ob()` par la même aide (chunk 6). Il construit son état, ne rejoue rien, ne nomme aucun motif de `spec`, d'`overrides` ni de `type-tag` ; ses 2 lignes `stash` ne changent pas ; ses appels de `pico8_ho_apply` restent.

### Les tests du §6 (G)

- **`ga_export_panel_by`** (chunk 3) : `route_a`/`route_b` → deux `trade`, le reste identique ; preuve rouge au commit (bug d'origine remis : `by` = table ob vide, test rouge, correctif restauré). Son appel `pico8_data_init(&mem)` devient `pico8_data_init()` au chunk 1 (suiveur de signature, aucune assertion touchée).
- **`gh_row_lq`** (chunk 5) : l'item 3 lu comme tuple typé ; preuve rouge au commit (copie de `js:pq` d'un item de trop ou au mauvais pas, test rouge, correctif restauré). Même suiveur de signature au chunk 1.
- **`layout_nil_item_draws_nothing`** : non touché.
- Si l'une de ces réécritures exige de changer l'**intention** du test, c'est l'arrêt du §7.

### Preuve des tests (§7, gcov + mutations)

- **gcov** (copie jetable du scratchpad, `--coverage`), par chunk : chaque ligne ajoutée ou modifiée des producteurs et des lecteurs est exécutée par un TU ; celles atteintes seulement par les e2e sont justifiées au commit.
- **Mutations**, chacune doit rougir un test, puis être annulée :
  - 1 — **M1** : `jc:lg` ne décrémente plus `l` → TU `jc` ou `make e2e` rouge ; **M2** : `bu` ne pose plus `jn` → `test_pico8_nf` ou `test_pico8_ui` rouge ; **M3** : `ky` échange `s[5]` et `s[6]` → `test_pico8_pj` ou `make e2e` (`game_end_final_score`) rouge.
  - 2 — **M4** : `pico8_gb_find` rend le premier et non le dernier projet d'un `px` (injecter un doublon dans un TU) → TU `hp` rouge ; **M5** : `pico8_gb_at` décalé d'un rang → TU `jo`/`qh` rouge.
  - 3 — **M6** : `hp:pq` saute un caractère sans `r[c]` au lieu de poser un enregistrement sans `qe` → TU `hp` rouge ; **M7** : `jo:lz` partage l'enregistrement au lieu de le copier → TU `jo`/`trade` rouge ; **M8** : `jo:cn` ignore `has_gc` → TU `trade` ou `make e2e` rouge.
  - 4 — **M9** : `pj:hb` n'écrit plus l'item 1 des sous-rangées de `ou` → `test_pico8_pj` ou `make e2e` rouge ; **M10** : `pj:hk` ne retire plus `ea[3]` → `test_pico8_pj` rouge ; **M11** : l'article `ld` perd son `ra` → `make e2e` (`message_box_hold`) rouge.
  - 5 — **M12** : `pz` alloue un `ra` par bouton au lieu d'un partagé → TU `js`/`ui` rouge (identité du vecteur) ; **M13** : un tuple `{qe=x}` produit `{x,6}` → `test_pico8_planet_gh_row_lq` ou `make e2e` rouge ; **M14** : `fa` posé même sans `ca` (TROU-9 défait) → TU `js` rouge.
  - 6 — **M15** : `cb` n'écrit `e_` que pour les entrées gardées → TU `js`/`hy` rouge ; **M16** : `e_` pas remis à l'init → TU `data_init`/`js` rouge.
- Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée.

### Fuite par frame (liant)

`make sanitize-net` tourne avec `detect_leaks=0` (les entités ne sont jamais libérées, TROU-7) : il ne voit pas une fuite qui croît. D.7a supprime des vues par frame (`pj:kj` : 5 sous-rangées de `pj.i_` par frame, `fv:kj` : 4 de `fv.rb`) et des allocations par frame (`jc:lg` : un `ra` par particule) ; il doit **baisser** la fuite, jamais l'augmenter.

- **Binaire** : celui que construit `make sanitize-net` (`tmp/pico8_replay_sanitize`, ASan+UBSan), lancé comme la cible le lance, sur **`tests/e2e/message_box_hold.json`** (159 frames : barre de `pj:kj`, `mh`, panneau de score) et **`tests/e2e/game_end_final_score.json`** (63 frames : `pj:ph`, `pj:hk`, retour à l'écran titre et `fv:kj`) :

  ```sh
  ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=exitcode=0:max_leaks=1 \
    ./tmp/pico8_replay_sanitize tests/e2e/<e2e>.json /dev/null "$S/<e2e>.dump.txt" > "$S/<e2e>.leak.log" 2>&1
  grep 'SUMMARY: AddressSanitizer' "$S/<e2e>.leak.log"
  ```
- **Avant** : sur l'arbre de départ (commit gradé de D.6c, construit une fois dans une copie du scratchpad) ; **après** : sur l'arbre de chaque chunk. Rapporté par e2e : octets perdus, allocations perdues, et octets / frame (octets ÷ frames de l'e2e).
- **Critère** : après chaque chunk, octets et allocations perdus ≤ départ sur les deux e2e ; **strictement plus bas** après le chunk 4 et au dernier chunk. Toute hausse est un échec, sauf si elle est expliquée ligne à ligne par une allocation typée qui remplace une table ob (listée) et compensée dans le même chunk.
- **Repli (H10)** : si LeakSanitizer ne peut pas tourner (message « LeakSanitizer has encountered a fatal error » ou équivalent), la mesure est `ASAN_OPTIONS=detect_leaks=0:print_stats=1` (ligne « … malloced … by N calls ») sur les mêmes e2e, même critère sur les octets alloués et le nombre d'appels ; le repli est dit au commit.

### Pas de nouvel e2e (G)

Les lignes changées sont atteintes par les 26 e2e et par les TU (H14) ; un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 90 minutes pour l'étape (≈ 12 par chunk, plus les mesures de départ et de fuite).**

| Cible (par chunk) | Durée attendue |
|---|---|
| `make test` (×2 : producteurs et lecteurs ; TU) | quelques secondes chacun, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H13) |
| `make class-coverage-net` | < 2 min (H13) |
| mesure de fuite (2 e2e) | < 1 min |
| gcov + mutations (scratchpad) | < 4 min |
| `make data-guard`, `make data-residue`, recette d'occurrences, classification | < 1 min |

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + chunk + « producteurs » / « lecteurs » / « TU »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre — **sauf la mesure de fuite du dernier chunk, qu'il relance lui-même** sur les deux e2e.

**Cibles finales**, vertes sur l'arbre de chaque commit : `timeout 5m make test` ; `timeout 5m make e2e` : **26 « conforme »** ; `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ; `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ; `make data-guard` : code 0 ; `make data-residue` + recette : jalons du tableau « Effet », `overrides` 0 au chunk 5, `ob` des 46 fichiers = R au dernier ; fuite ≤ départ à chaque chunk, < départ aux chunks 4 et 6 ; classification et anti-tassement passés.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit liste les chemins de son chunk (« Empreinte remesurée », écart ≤ 5 expliqué), rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements ; `tools/` inchangé ; `Makefile` : seulement `DATA_RESIDUE_GUARD` au chunk 5 (et une liste de sources si un module se vide, dit au commit).
2. **Fichiers de D.7b** : seules les lignes des « Suiveurs forcés » changent (`msgbox`, `help` hors l. 251, `ui_button`, `planet.c`, `planet_family_trade.c`, `mouse_reader.c`, `ui.c`).
3. **Reste R** : `sh tools/data_residue.sh --detail` et `grep` du motif `ob` sur les 46 fichiers ne rendent que les lignes de la liste close.
4. **Vues et ponts** : `grep` de la « Sortie de l'étape » vides ; aucune `pico8_ob_table_new`, aucun `pico8_jb_row_view` ajoutés dans `src/` à aucun chunk (diff de chaque commit).
5. **Allocation** : chaque allocation typée par appel est à un site qui allouait une table ob à cet appel (liste du commit) ; les contenus constants sont `static const`.
6. **Défauts mutés** : `nb.e_`, `bz`, `pj.ou`, `pj.ea`, `nf.l`/`nf.r` sont des tableaux ou structs statiques non `const`, remis à l'init.
7. **Anti-tassement** : comptes avant/après chaque chunk, en lignes ET en occurrences, par catégorie et par fichier, recalculés avec la recette ; chaque couple Δlignes > Δoccurrences justifié ; aucune jonction de lignes, aucun alias, aucun registre de TU, aucun déplacement dans le test ponctuel ni hors du compteur, aucun commentaire sans rapport supprimé ; classification (a)/(b), aucune (c).
8. **Fuite** : il relance la mesure LeakSanitizer (ou le repli) du dernier chunk sur les deux e2e et la compare au départ du commit de D.6c ; lit les valeurs des chunks intermédiaires dans les messages de commit.
9. **TU et §6** : touchés seulement aux lignes forcées ; tests retirés listés avec leur raison ; constantes `*_HASH` inchangées ; preuves rouges des deux §6 au commit.
10. **Garde** : `DATA_RESIDUE_GUARD` = `spec,overrides` à partir du chunk 5, `make data-guard` vert.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun récit de chantier ; aucun renvoi à D.7a ni au plan dans `src/` ou `tests/`.
12. **Docs** : glossaire à jour ; note « Découvert à D.7a » en fin de ligne D.7a, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le commit gradé de D.6c (ou un commit de docs au-dessus) propre, si ce PRD n'a pas été relu contre la note de D.6c (section « À revérifier »), ou si `make data-residue` n'y rend pas `overrides` 30 et `spec` 0 sans explication de la note.
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence, à un des six commits.

**Périmètre**

- `FAILED:` si une ligne `ob` hors de R reste dans les 46 fichiers de la ligne après le dernier chunk, ou si une ligne de `grep` de la « Sortie de l'étape » subsiste.
- `FAILED:` si un fichier de D.7b change hors des lignes des « Suiveurs forcés », ou si une ligne de D.7b est convertie sans être forcée par un type de D.7a.
- `FAILED:` si le diff ajoute une `pico8_ob_table_new`, un appel à `pico8_jb_row_view`, une vue ou une conversion typée ↔ ob, par appel, par frame ou par init ; ou une allocation typée à un site qui n'en faisait pas.
- `FAILED:` si une donnée a deux formes (ob et typée) à la fin d'un chunk.
- `FAILED:` si un test du §6 change d'intention, ou si la preuve rouge de `ga_export_panel_by` ou de `gh_row_lq` manque au commit.
- `FAILED:` si le diff touche un TROU autrement que par le texte de contrat qui l'entoure (TROU-3, 4, 7, 9, 23 gardés à leurs sites).
- `FAILED:` si le test ponctuel construit une table ob depuis une donnée typée, nomme un motif de `spec`, d'`overrides` ou de `type-tag`, porte plus de 2 lignes `stash`, ou reçoit du code déplacé de `src/`.

**Gardes, comptes, fuite et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, si `overrides` n'est pas 0 et dans `DATA_RESIDUE_GUARD` au chunk 5, ou si `tools/data_residue.sh` change.
- `FAILED:` si `stash` n'est pas 28 / 4, `spec` 0, ou si `ob`, `valeur` ou `type-tag` ne baisse pas strictement sur l'étape.
- `FAILED:` si la fuite mesurée (octets ou allocations perdus, LeakSanitizer ou repli) monte sur un des deux e2e à un chunk sans l'explication exigée, ou ne baisse pas strictement après le chunk 4 et au dernier ; si la mesure manque à un commit.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés dans une ligne comptée, construction déplacée sur une autre ligne comptée, renommage à type constant, alias, registre de TU, code déplacé hors du compteur ou dans le test ponctuel, commentaire sans rapport supprimé.
- `FAILED:` si un commit ne donne pas, par catégorie, les comptes avant/après **en lignes et en occurrences**, si un couple Δlignes > Δoccurrences n'est pas justifié, si la classification (a)/(b) manque, ou si une ligne gagne une catégorie sans être listée.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` (ou `identity[]`) change.
- `FAILED:` si un test est retiré sans être listé au commit avec sa raison.
- `FAILED:` si le rapport gcov ou une mutation manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si un message de commit ne porte pas ses comptes avant et après et sa mesure de fuite.
- `FAILED:` s'il manque la note « Découvert à D.7a », ou si elle omet la décision d'ordre (ce que D.7a a pris à D.7b), le compte `ob` réel, R, `overrides` = 0 gardé, le nouveau majorant de D.7b ou l'effet sur D.7c/D.8.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **D.7b hors du chaînon `pz`** : entités et interface (`planet*` hors des lignes suiveuses, `probe`, `particle` et ses particules ob, `slipway`, `wormhole`, `msgbox` hors `pz`/pont `mh`, `ui*` hors `nf` et pont `js`, `help` l. 251, `scroll`, `trade`, `bg`, `ke`, `entity`, `bootstrap`, `hud`, `research_icon`) et en-têtes (`pico8_struct.h`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_value.h`, `pico8_list.h`).
- **D.7c** : les fixtures ob des TU que D.7a ne force pas.
- **D.8** : `pico8_ob.c/.h`, `pico8_fi`, `pico8_stash.*`, le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, et le reste R (enregistrements de `ob()`, `pico8_ho_apply`).
- **D.9** : `par-nom`, `dispatch` (`methods[]`, `pico8_mv`, membres fonction), la représentation de `pico8_list_t` (éléments `pico8_value_t` de `lq.items`, `mh.items`, `hp.i`, `jo.jy`, `ga.by`, `trade.lp`/`rl`), les constructeurs de `pi` à arguments `pico8_value_t`.
- La fuite des entités (TROU-7), les archétypes (phase 2) ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **ordre D.7a/D.7b** : fusion partielle nommée — D.7a prend le chaînon `pz` (`pz`, pont `mh`, `bo`/`fv.rb` de `help`, pont `js` de `ui_button`), `g_nf_l`/`g_nf_r` avec `bu`, et les lignes suiveuses de `planet.c`, `planet_family_trade.c`, `ui.c` ; rejetés : laisser les ponts à D.7b, intervertir, fusionner. `overrides` = 0 et gardé à D.7a.
  - **cible** : `ob` des 46 fichiers = R (liste close ≤ 15 lignes, test ponctuel), pas 0 ; aucune vue ni pont restant ; fuite par frame en baisse.
  - **formes** : rangées et tuples en `pico8_jb_row_t` (une sorte REF opaque ajoutée pour l'article `ld`), clés de D.6a-D.6c pour `pz`/`js`/`mh`/`lq`, entrées générées pour `ix`/`ha`/`bo`/`bz`/`nb`, types nouveaux seulement pour `{qe,gc}`, `gb`, particules, boutons de `nf`, `ky` ; défauts mutés en statiques non `const`.
  - **tests** : TU touchés aux seules lignes forcées ; test ponctuel par comparaison récursive sans vue ; deux §6 (dont `gh_row_lq`, découvert) avec preuve rouge ; seize mutations ; mesure LeakSanitizer ; pas d'e2e.
  - **découpage** : six commits par famille de données, ≈ 11 à 31 chemins chacun.
  - **D.6c** : douze hypothèses H-D6c écrites sur `f5d920c`, à vérifier contre sa note.
- **Note du plan (G)**, à ajouter en fin de ligne D.7a (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.7a : six chunks (familles isolées ; `gb` ; enregistrements `{qe,gc}` ; articles de `bl` et panneau de score ; pipeline d'affichage et chaînon `pz` ; `nb` — N chemins : suiveurs forcés, TU qui lisent `gb` ou les ponts, test ponctuel hors du majorant). **Ordre D.7a/D.7b : D.7a a pris à D.7b le chaînon `pz`** (`pico8_pz`, `pico8_mh_new_table`, `g_bo` et `fv.rb` de `help.c`, `pico8_js_new_table` et les branches `OB_TABLE` de `js:pq`), **`g_nf_l`/`g_nf_r`** (avec `bu`) et les lignes qui lisaient ses types convertis dans `planet.c`, `planet_family_trade.c`, `ui.c` ; D.7b ne les a plus. Formes : rangées, tuples de `lq:mn` et articles de `bl` en `pico8_jb_row_t` (sorte REF opaque pour `ld`) ; `pz` sur `pico8_js_keys_t` ; sources `lq`/`h_`/`jb` typées ; `{qe,gc}` en `pico8_jy_t` ; `gb` tableau fixe (`pico8_gb_at`, `pico8_gb_find`) ; `nb.e_`, `pj.ou`, `pj.ea`, `bz`, `nf.l`/`nf.r` statiques non `const`. Vues fermées : `ix`, `ha`, `bo`, `bz`, `nb`, `pico8_jb_row_view`, `jo_kw_view`, `hp:jb`, `pico8_lq_table_view` ; `pico8_data_init(void)`. `overrides` = 0, dans `DATA_RESIDUE_GUARD`. `ob` dans les fichiers de la ligne : R lignes (enregistrements de constructeurs de `ob()` dans `pi.c`/`collect.c`, `pico8_ho_apply`), servant le test ponctuel, supprimées par D.8 ; le « = 187 » de D.7c devient 187 + R. `ob` après D.7a : O (majorant de D.7b : M). Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : A → B octets. `par-nom` : P.

  N, R, O, M, A, B, P sont les valeurs réelles.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget de tests), anti-tassement, classification, périmètre D.7b et reste R compris.
- **Le message de chaque commit** porte : le périmètre du chunk ; la forme par famille et ses rejets ; la classification (a)/(b) des lignes `ob`/`overrides` retirées ; les comptes avant et après **en lignes et en occurrences** par catégorie et par fichier, et les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie ; la liste des allocations typées par appel et des sites devenus `static const` ; la mesure de fuite avant/après ; les tests retirés avec leur raison ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses H et H-D6c qui le concernent.
- **Surprises pour le pilote** :
  - **La ligne D.7a ne tient pas sans une part de D.7b** : `pz` est le goulot par lequel passent toutes les rangées d'affichage, et il est dans `msgbox.c`. La note de D.6b l'avait vu pour `lq`/`jb` ; le relevé montre qu'il bloque aussi `nb` (son `oy` part en `jb` par `pz`).
  - **`mouse` et `nf.l`/`nf.r` sont une seule donnée** que la ligne D.7a et la note de D.5a mettaient dans deux étapes.
  - **`gh_row_lq` est un §6 de D.7a** : la ligne ne nomme que `ga_export_panel_by`, mais l'item `kw` qu'il lit est la vue `jo_kw_view` que D.4a assigne à D.7a.
  - **« 0 dans ces fichiers » bute sur le test ponctuel** : tant qu'il parse par `pico8_ob()`, ses constructeurs `v`/`s`/`b` et `pico8_ho_apply` restent en `src/` ; ils meurent à D.8, que sa ligne ne dit pas encore.
  - **Des vues fuient à chaque frame** : `pj:kj` (5 sous-rangées de `pj.i_`) et `fv:kj` (4 de `fv.rb`) libèrent la table du haut, jamais les imbriquées ; `jc:lg` réalloue un `ra` par particule et par frame. `sanitize-net` (`detect_leaks=0`) ne le voit pas : D.7a ajoute une mesure LeakSanitizer.
  - **L'empreinte de la ligne (39) sous-estime** : ≈ 80 à 95 chemins, surtout ≈ 18 TU qui lisent `gb` et les TU des ponts.
