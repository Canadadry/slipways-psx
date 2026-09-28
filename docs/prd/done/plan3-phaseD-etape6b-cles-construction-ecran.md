---
title: "Plan 3, étape D.6b : les clés de construction de la mise en page et des entités d'écran (`lq`, `jb`, `js`, `fq`, `mh`, `iv`, `ui`, `qb`, `nf`, `bg`, `mz`, `jc`) passent d'une table ob à une struct typée locale, ou à aucun paramètre ; les appelants qui reçoivent une rangée ob d'ailleurs passent par quatre ponts qui gardent leur nom `overrides` ; le pont `pico8_dr()` est fermé — trois commits, aucun pixel changé"
description: "Douze constructeurs d'écran reçoivent encore leurs clés dans une `pico8_ob_table_t` que l'appelant alloue, remplit par nom, passe puis libère (toasts, slipways, menus, lignes d'infobulle), ou qu'il reçoit toute faite d'une vue ou d'une rangée `bs`. D.6b type ces douze paramètres, fait construire les clés sur la pile là où l'appelant les écrit lui-même, supprime les vues et conversions construites à chaque appel par `js:pq` et `jb:pq`, ferme le pont `pico8_dr()`, et laisse aux producteurs de rangées ob (D.7a, D.7b) quatre ponts comptés — en mesurant la baisse d'`overrides` en lignes ET en occurrences, sans tassement ni renommage à type constant."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.6b — `overrides`, mise en page et entités d'écran : `lq`/`jb` (`layout`), `js`/`fq` (`ui_button`), `mh`/`iv` (`msgbox`), `ui`, `qb` (`scroll`), `nf` (`mouse_reader`), `bg`, `mz` (`wormhole`), `jc` (`slipway`), appelants de `menu.c`, même règle. Empreinte 30 (19, 11). Compte : `overrides` = 117. §6 : `layout_nil_item_draws_nothing` (l. 106-112) passe son item par la struct de clés de `pico8_lq_new` ; l'item reste un `pico8_value_t`, élément de `lq.items` ; même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : oui.

Lignes voisines, recopiées (la « même règle » est celle de D.6a ; D.6c fixe ce qui reste après D.6b) :

> - D.6a — `overrides`, constructeurs de l'arbre `jo` (`pico8_jo_new` … `pico8_mx_new`), `new_entity` de `pico8_entity_kind_t` (`pico8_struct.h`) et ses appelants (`galaxy.c`, `jo_kv`…) : la table de clés de construction devient une struct locale (troisième puce), paramètre renommé. Empreinte 11 (8, 3). Compte : `overrides` = 315. §6 : `gh_row_lq`, commentaire de la l. 2 seulement ; aucune lecture ne change, pas de preuve rouge. `class-coverage-net` : oui.
>
> - D.6c — `overrides`, reste : `g_` (`probe`), `fv` (`help`), `ie` (`research_icon`, `cart.c`), `hp` (`research`), `hy` (`economy`), `trade` (et `planet_trade.c`), `pj` (`score_panel`), même règle. Empreinte 21 (16, 5). Compte : `overrides` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Point 3 de l'arrêt « champ sans règle », recopié (il fixe ce que D.6b ne type pas) :

> 3. `pico8_list_t` : la note de D.0 exige d'en changer la représentation sans en fixer la forme. `lq.items` tient des chaînes, des fonctions, nil et le fixe 0 (`layout_nil_item_draws_nothing`), et aucune puce ne type ces éléments hétérogènes.

Notes « Découvert à D.x » des étapes livrées, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b, D.5c ; D.6a n'a pas encore la sienne) :

> Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.

> Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.

> Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).

> Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).

> Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp` : `lq_own`/`lq_inner`) ; `hp.mr`/`gp` en `pico8_str_seq_t`, `ho_apply` les prend typés ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages `po_own`/`fu_own` dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie ; une clé `qe` chaîne est absente) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus, `pico8_jc_hg_add`/`_has`) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`, qui partage son calcul avec `pico8_gi`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : 1882 (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de −100 (1882 − 1982).

> Découvert à D.5a : deux chunks (interfaces, puis `ke` ; ≈ 40 chemins, `pico8_game_data.*`, `pico8_bootstrap.c` et les TU qui plient l'identité de `me` hors du majorant). Défauts générés `pico8_X_start` (type `pico8_X_start_t`, `pico8_game_data.h`), un par classe à clés (`ke`, `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`), clés propres de la spécification, lus par les constructeurs ; les tables gardent l'objet de D.4a/D.4b ; `mz.jj` s'initialise de `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` : liste statique de `ui.c` (`pico8_ui_le_shared`) ; `qb.v`/`p`/`ra`, `iv.v`, `ui.hw` pointent la donnée générée (membres `const`) ; `nf.ra` pointe le statique non `const` `g_nf_ra`, affecté depuis `pico8_nf_start.ra` à `pico8_nf_init` (repli : `test_pico8_entity_kind` et `test_pico8_ie` écrivent `je.ra` en place) ; `nf.l`/`nf.r` restent `g_nf_l`/`g_nf_r` (D.7b). Le cycle `_init`/`_shutdown`/`_ready` des treize modules reste, sur un drapeau. `pico8_ke_start_field` (compté par `par-nom`) remplace `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_g__default`, `pico8_ie_default` (retirés à D.5b) et `pico8_jo_default` (D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`) : D.5c le supprime. `pico8_rk_default` (variante de dessin, comptée par `spec`) est à D.5b. `spec` après D.5a : 634 (au lieu de ≤ 779) ; les majorants `spec` suivants se décalent de −145 (634 − 779). `par-nom` : 1090.

> Découvert à D.5b : deux chunks (exclusion de `rk`, puis les sept classes ; 29 chemins, `pico8_game_data.*` et trois TU hors du majorant). `pico8_rk_default` (variante de dessin à argument omis) entre dans `SPEC_EXCLUSION`, preuve : ses quatre lignes ne portent aucune autre catégorie. Défauts générés `pico8_X_start` de `g_`, `fv`, `hp`, `ie`, `pj`, lus par les constructeurs ; `hy`, `trade` sans objet. `ie` : un seul défaut pour la chaîne et les objets prêtés, `kq`/`ra`/`eh` pointent la donnée générée, plus libérés à l'arrêt. `fv.hi`/`gq`/`kq`, `g_.ra`, `ie.kq`/`ra`/`eh`, `pj.cr.hi`/`gq` : membres `const` vers la donnée générée, aucun repli. `pj.cr` : `g_pj_cr` reste (son `ep` est muté), valeurs de `pico8_pj_cr_start`. `pj.ou`/`pj.ea` : rangées générées `pico8_pj_ou_row`/`pico8_pj_ea_row`, d'où `pico8_score_panel_init` construit une fois par init les tables d'état `g_pj_ou`/`g_pj_ea` (`pico8_jb_row_view`), jamais libérées comme celles de la spécification ; leurs lecteurs (`hb`, `hk`, `kj`, `pico8_mh_new`) restent des tables ob (D.6b, D.7a). `test_pico8_spec_defaults` n'a plus de registre ni de constante. `pico8_ke_start_field` n'a plus que `pico8_jo_default` et le cas `M_KE`. `spec` après D.5b : 430 (au lieu de ≤ 402 après le décalage de D.5a : 28 lignes des sites de D.5b, retirées par D.4a/D.4b, étaient comptées à la fois dans le majorant 547 et dans ce décalage ; majorant corrigé 430) ; le « = 0 » de D.5c est inchangé. `par-nom` : 1085.

> Découvert à D.5c : deux chunks (données, puis conversion et garde ; 27 chemins, `pico8_game_data.*`, `ke.c/.h`, le test ponctuel et le `Makefile` hors du majorant ; le lien du test ponctuel prend `pico8_planet_family_trade.c`, pour interroger `nr`/`fr`/`mx`). Défauts générés `pico8_X_start` des dix classes de l'arbre, clés propres, lus par les constructeurs, l'héritage par le `struct_init` de la parente ; chaînes dans des tableaux distincts, un par jeton (des littéraux se fusionneraient, TROU-3). `jo.kq`/`gq`, `pm.ng` : membres `const` vers la donnée générée, aucun repli. Le `field` des sortes est `pico8_X_kind_field` : `rr`, `s`, `me`, `od` seulement, lus dans `pico8_X_start`, puis la parente jusqu'à `pico8_jo_kind_field` (seul déclaré, `planet.h`) ; 0 si le module n'est pas prêt ; compté par `par-nom` (1106 après D.5c), à typer par D.9. `pico8_ke_start_field` supprimé. `_ready` sur `g_jo_up`, `g_family_up`, `g_family_trade_up`. `ga.by` : `g_ga_by_shared`, `pico8_list_t` statique non `const` vidée à chaque init (précédent `ui.le`), contenu à typer par D.7a. `spec` après D.5c : 0, dans `DATA_RESIDUE_GUARD`. `ob` : 1721 hors test ponctuel.

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

### Lecture de ces textes pour D.6b

- **« Même règle »** (celle de D.6a) : la table de clés de construction devient une **struct locale** à l'appelant, passée par pointeur `const`, lue par le constructeur et jamais gardée ; le paramètre est renommé **seulement parce que son type change** (`const pico8_ob_table_t *` → struct typée). Un renommage à type constant ferait tomber `\boverrides\b` sans rien convertir : c'est le **tassement par renommage**, interdit et traduit en `FAILED:` (section « Anti-tassement »).
- **Le « = 117 » n'est pas le bon compte.** Le PRD de D.6a (en cours d'implémentation) a montré que le « = 315 » de D.6a était le total des lignes `overrides` des fichiers de son empreinte, pris pour exact (il est 329). Le « = 117 » est construit de la même façon et a le même défaut : les fichiers de D.6c portent 23 lignes de D.6b (`economy.c` : les deux `pico8_iv_new` de `hy:lj`, 11 lignes ; `test_pico8_hp`, `test_pico8_hy`, `test_pico8_trade` : un `pico8_jc_new` chacun, 12 lignes). Recompté **ligne par ligne** (inventaire plus bas) : sur les 329 lignes que D.6a doit laisser (H-D6a-1), **232 lignes / 244 occurrences** sont de D.6b et **97 / 104** de D.6c. Si D.6b ne laissait rien, le compte serait 97 ; il est **118 / 129** parce que D.6b garde, comptés, les quatre ponts décidés plus bas (21 lignes / 25 occurrences). Section « Effet sur `make data-residue` ».
- **Ce que D.6b ne peut pas typer.** Les éléments d'une rangée `lq` (`lq.items`) restent des `pico8_value_t` : la ligne le dit pour `layout_nil_item_draws_nothing`, et le point 3 de l'arrêt le laisse à D.9. Un item imbriqué (`o(o(15,7))` des menus, `{24,5}` de `cr`, la vue `kw` de `jo:gh`) est une table ob que `lq:mn` lit par indice et par `qe`. Or presque toutes les clés de `js`, `lq`, `jb` et `mh` arrivent **dans des rangées ob construites ailleurs** : les `bs` de `pz` (produits par `planet_display` et `menu` — D.7a —, par `help` et le préfixe `ep` de `pz` — D.7b), les lignes `ix` de `eu`, `spec1`/`spec2`, `st`, `l` et `iw` de `score_panel_display`/`research_display` (D.7a nomme `spec1`/`spec2`). Les typer ici, c'est faire D.7a dans D.6b, et une vue ob par appel serait encore nécessaire pour chaque item imbriqué.
- **D'où les ponts (G)** : là où l'appelant **écrit lui-même** sa table de clés (clés littérales, ou valeurs qu'il vient de calculer), il construit la struct sur la pile ; là où il **reçoit** une rangée ob d'ailleurs (élément de liste, vue, argument), il la passe à un **pont** par classe (`pico8_lq_new_table`, `pico8_jb_new_table`, `pico8_js_new_table`, `pico8_mh_new_table`) qui lit la rangée par nom, remplit la struct et appelle le constructeur typé. Le paramètre d'un pont **garde le nom `overrides`** : son type ne change pas, il reste compté, et la baisse d'`overrides` ne contient ainsi aucune ligne déplacée sous un autre nom. Les ponts tombent quand leurs producteurs sont typés (D.7a, D.7b).
- **Les ponts de données nommés pour D.6b par les notes** : `pico8_dr()` (entrée de `jb`) **est fermé** — `jb` prend la rangée générée `const pico8_jb_row_t *`, dont les lignes n'ont que des items scalaires. `pico8_ix()`/`pico8_ha()`, `g_bo`, `pico8_bz()` **ne le sont pas** : ce sont des vues construites une fois par init, leurs rangées passent par `pz` ou `eu` (ob jusqu'à D.7a/D.7b), et `ix` porte un item imbriqué (`o(o(15,7))`) que seule une vue sait donner à `lq:mn` ; les fermer ici demanderait une vue par appel, interdite. Même chose pour les vues à l'appel de `hp.lq` et de l'item `kw` de `jo:gh` (producteur `planet_display`, D.7a). La note « Découvert à D.6b » les reporte, avec la raison.
- **Ce que D.6b supprime quand même à chaque appel** : dans `js:pq`, la vue `pico8_lq_table_view` d'une rangée copiée (remplacée par `pico8_lq_keys_of`, sans table) et la conversion liste → table (`pico8_ob_table_from_list`, TROU-7) ; dans `jb:pq`, la table qui emballe chaque ligne chaîne ; chez les appelants à clés littérales (`planet.c`, `economy.c`, `planet_family_trade.c`, `menu.c`), la table allouée, remplie, passée et libérée.
- **§6** : `layout_nil_item_draws_nothing` passe son item par `pico8_lq_keys_t` (preuve rouge) ; le commentaire l. 1-2 de `gh_row_lq`, reporté ici par D.6a (H-D6a-4), devient faux au changement de type de `js:pq` et est réécrit, avec sa preuve rouge (le bug d'origine — une liste lue comme une table — est remis dans la branche liste de `js:pq`) ; `ga_export_panel_by` n'est pas touché.
- **`make class-coverage-net` et `make sanitize-net`** : oui aux deux, à chaque chunk : chacun touche des constructeurs, et chacun change la durée de vie d'une table de clés (tas → pile) ou retire une vue.

### D.6a n'est pas livrée à l'écriture de ce PRD

D.6a (clés de l'arbre `jo` → `pico8_jo_keys_t`/`pico8_ga_keys_t`, `new_entity` typé, `overrides` = 329) est implémentée en parallèle et touche des fichiers que D.6b touche aussi (`pico8_planet.c`, `pico8_planet_family_trade.c`, `test_pico8_ui`, `test_pico8_jc`, `test_pico8_hp`, `test_pico8_mz`, `test_pico8_mx`, le test ponctuel). Ce PRD **suppose que D.6a atterrit comme son PRD le dit**. Chaque supposition sur son résultat est une hypothèse **H-D6a-n** (section « Hypothèses sur D.6a »), et la section « À revérifier après D.6a » liste ce que `plan-pilot` relit contre la note « Découvert à D.6a » avant de lancer D.6b. **Aucun fichier non versionné n'a été lu** : tout vient de `a29154e` (extrait par `git archive` dans le scratchpad).

### État de départ

**Mesuré sur `a29154e`** (branche `claude/plan-pilot-3-7zdft0`, D.5c livrée et gradée, D.6a en file) ; **l'arbre de départ de D.6b est le commit gradé de D.6a**, inconnu à l'écriture. Décisions de l'utilisateur qui engagent D.6b : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**. Consigne de pilotage, liante : **une étape précédente a été notée FAILED pour avoir fait baisser un compte par tassement de lignes** ; le compteur compte des lignes, et toute mise en forme, tout renommage à type constant, tout alias, registre ou déplacement qui baisse un compte sans retirer la construction est interdit.

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `sh tools/data_residue.sh` et la recette d'occurrences ont tourné, sur une copie `git archive HEAD` du scratchpad ; le reste vient de `grep` et de la lecture des sources de `a29154e` et de `pico8/lua.lua`. Tout fait d'exécution est une hypothèse (H1 à H16, H-D6a-1 à H-D6a-12).

### Inventaire (mesuré sur `a29154e`)

**`make data-residue`** (lignes / fichiers) et **occurrences** (recette de la section « Anti-tassement ») :

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 5732 | 132 | 6625 |
| `ob` | 1898 | 122 | 2127 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 411 | 69 | 664 |
| `ref-audit` | 68 | 17 | 69 |
| `par-nom` | 1106 | 81 | 1161 |
| `dispatch` | 197 | 59 | 199 |
| `overrides` | 450 | 62 | 479 |
| `spec` | 0 | 0 | 0 |
| **résidu D** | **8349 lignes** | **140 fichiers** | — |

Après D.6a (H-D6a-1) : `overrides` **329 / 348 / 55 fichiers**, les autres catégories remesurées au départ.

**Ce que dit le cart** (`pico8/lua.lua`) : le constructeur de classe (`kv:jr`, l. 86-87) copie les clés passées dans l'instance (`mw(ob)`) ; une clé nil n'existe pas. `js:pq` (l. 261) fait `self.lq=lq(mv(self,"lq"))`, `self.h_=self.h_ and lq(self.h_)`, `self.jb=self.jb and jb(self.jb)` : les clés `lq`, `h_`, `jb` d'un bouton sont des **sources de rangée** que `lq()`/`jb()` copient à leur tour. `lq:pq` (l. 289) parcourt `all(self)` et recalcule `fs` et `w` ; `jb:pq` (l. 314) fait une `lq` par ligne (`nv(self, …)`). `pz` (msgbox) pose `ra`, `qf`, `fa` dans chaque rangée de `bs` puis fait `js(b)`. `ix`/`ha` (l. 965-990) : rangées `{ra, lq=o(o(15,7))}` et `{lq=o(31,"restart     ")}`.

**Clés de construction par classe** (boucles et `assert` des constructeurs, recoupés avec les appelants par `grep`) :

| Classe | Clés acceptées | Appelants de `src/` (source des clés) | Forme décidée |
|---|---|---|---|
| `bg`, `nf`, `qb`, `ui`, `mz` | aucune (`assert` : NULL ou vide) | `cart.c` (`bg`), `bootstrap.c` (`qb`, `nf`, `ui`), `bg.c` (`mz`) : tous NULL | pas de paramètre |
| `iv` | `ra`, `[1]` (`at1`), `[2]` (`at2`) | `planet.c` (`ra`, texte, couleur), `economy.c` ×2 (`hy:lj` : texte, couleur) — littérales | `pico8_iv_keys_t` |
| `jc` | `qu` | `planet.c` (`jo:ldown`), `planet_family_trade.c` (`nr:ldown`) — littérales | `pico8_jc_keys_t` |
| `fq` | `my` | `menu.c` (`fq(de)`), `score_panel_display.c` (NULL) | `pico8_fq_keys_t` |
| `lq` | positionnels, `c`, `bg`, `score`, `ra`, `fs`, `w` | `menu.c` (`ix[2].hb`, littérale) ; `layout.c` (`jb:pq`, ligne reçue) ; `ui_button.c` (`js:pq`, source reçue : table, liste ou rangée) ; `score_panel_display.c` (`st`), `research_display.c` (`l`) — tables ob des producteurs D.7a | `pico8_lq_keys_t` + pont |
| `jb` | positionnels seulement | `cart.c` (`eu` : entrée de `pico8_dr()`) ; `ui_button.c` (`js:pq`, source reçue) | `const pico8_jb_row_t *` + pont |
| `js` | `fa`, `lq`, `h_`, `jb`, `ra`, `qf`, `my`, `hb`, `fg`, `px`, `trade`, `en`, `_cr_t`, `_jo_fe_pt`/`_b`/`_od` | `msgbox.c` (`pz` : rangées `bs`), `cart.c` (`eu` : rangées `ix`), `score_panel_display.c` (`spec1`/`spec2`, parse par appel) — toutes ob reçues | `pico8_js_keys_t` + pont |
| `mh` | positionnels (ordre d'entrée), `rh`, `lh` | `score_panel_display.c` (`pj:hk` : état `g_pj_ea` ; `pj:ph` : `iw_table`) | `pico8_mh_keys_t` + pont |

**Faits qui engagent la forme** :

- **Absent ≡ défaut du membre** pour `iv` (`ra` NULL, `at1` NULL, `at2` 0), `jc` (`qu` NULL), `fq` (`my` NULL) : aucun drapeau de présence n'est nécessaire.
- **`jc.qu` est un `pico8_struct_header_t *`** (neuf classes de planète possibles) : le type n'interdit pas une classe fausse, l'`assert` de classe de `jc_take_endpoint` reste un contrat d'exécution.
- **`js.fa` a un défaut** (`pico8_js_start.fa`) et un `fa` nil est refusé : présence explicite. **`js.my`** distingue absent, `false` (drapeau posé, fonction NULL) et fonction. **TROU-23** : une clé `h_` ou `jb` présente à nil construit une `lq`/un `jb` vide ; `false` n'en construit pas (`h_`) ou est refusé (`jb`).
- **`lq.fs` et `lq.w` ne survivent pas à `lq:pq`**, qui les réécrit ; seule la vue `pico8_lq_table_view` les produisait (copie d'une rangée par `js:pq`), et D.6b la remplace par `pico8_lq_keys_of`.
- **`lq` range ses positionnels par indice croissant, trous resserrés** (`pico8_layout_positional_values`, TROU-25) ; **`mh` les prend dans l'ordre des entrées** de la table. Les deux lectures restent dans leurs ponts respectifs.
- **Une liste passée comme source de `lq`** (`jo:gh`) donnait, par `pico8_ob_table_from_list`, une table aux indices 1..n, nil compris (`pico8_ob_table_set` garde une valeur nil, `pico8_list_iter_next` rend chaque élément) : les éléments de la liste, dans l'ordre, sont exactement les positionnels que `lq` voyait.
- **Les lignes de `pico8_dr_table`** sont des rangées imbriquées d'items `.s`/`.n` seulement, sans `c` ni `fn` au premier niveau (lu dans `pico8_map_tables.c`) : `jb` peut les lire sans vue.
- **`pico8_de()`** (`g_de` = `{my=…}`, `score_panel_display.c`, D.7a) sert `fq(de)` (`menu.c`) et la parse de `spec2` ; `fq` n'en lit que `my`.
- **Les TU construisent souvent dans une variable `b`**, invisible pour le motif : l'empreinte réelle dépasse celle de la ligne.

**`overrides` à D.6b, par fichier** (lignes / occurrences ; « avant » = ce que D.6a laisse, H-D6a-1/2 ; chunks définis plus bas) :

| Fichier | Avant | Après | Chunk | Ce qui est compté |
|---|---|---|---|---|
| `pico8_bg.c` / `.h` | 4/5, 2/2 | 0 | 1a | signature, `assert` vide, `pico8_bg_new` ; contrat |
| `pico8_mouse_reader.c` / `.h` | 4/5, 2/2 | 0 | 1a | idem `nf` |
| `pico8_scroll.c` / `.h` | 4/5, 2/2 | 0 | 1a | idem `qb` |
| `pico8_ui.c` / `.h` | 4/5, 2/2 | 0 | 1a | idem `ui` |
| `pico8_wormhole.c` / `.h` | 4/5, 2/2 | 0 | 1a | idem `mz` |
| `pico8_slipway.c` / `.h` | 6/7, 2/2 | 0 | 1b | constructeur de `jc` (boucle, `qu`), `pico8_jc_new` ; contrat |
| `pico8_planet.c` | 11/11 | 0 | 1b | `jo` : toast `iv` (6) et `jc` de `jo:ldown` (5) |
| `pico8_planet_family_trade.c` | 5/5 | 0 | 1b | `jc` de `nr:ldown` |
| `pico8_economy.c` | 15/16 | 4/5 | 1b | −11 : les deux `iv` de `hy:lj` ; restent les 4 lignes du constructeur de `hy` (D.6c) |
| `pico8_msgbox.c` | 12/14 | 4/5 | 1b (−6/−7 `iv`), 2 (−2/−2 `mh`) | constructeurs `iv` et `mh` ; reste le pont `mh` |
| `pico8_msgbox.h` | 4/4 | 1/1 | 1b (−2), 2 (−1) | contrats et signatures ; reste la déclaration du pont |
| `pico8_ui_button.c` | 16/18 | 4/5 | 1b (−6/−7 `fq`), 2 (−6/−6 `js`) | constructeurs `fq` et `js`, `js:pq` (`lq(x)` : vue, conversion, libération, commentaire l. 24) ; reste le pont `js` |
| `pico8_ui_button.h` | 4/4 | 1/1 | 1b (−2), 2 (−1) | idem |
| `pico8_layout.c` | 20/22 | 9/11 | 2 | `jb:pq` et son pas (variable locale, emballage), constructeurs `lq`/`jb`, fonctions publiques ; restent les ponts `lq` (5/6) et `jb` (4/5) |
| `pico8_layout.h` | 4/4 | 2/2 | 2 | commentaire de `pico8_lq_t` (l. 21-22), deux signatures ; restent les deux déclarations de ponts |
| `pico8_menu.c` | 5/5 | 0 | 2 | `ix[2].hb` : table littérale de `lq` |
| `test_pico8_iv` | 21 | 0 | 1b | tables de clés de `iv` (dont `must_abort_key`) |
| `test_pico8_jc` | 13 | 0 | 1b | tables de clés de `jc` |
| `test_pico8_fq` | 8 | 0 | 1b | tables de clés de `fq` |
| `test_pico8_js`, `_mx`, `_mz`, `_ui`, `_hp`, `_hy`, `_trade` | 4 chacun | 0 | 1b | un `jc` construit sur `qu` |
| `test_pico8_lq` | 6 | 0 | 1b (−4 : `jc`), 2 (−2 : commentaires l. 258, 281) | |
| `test_pico8_mh` | 16 | 0 | 2 | tables de clés de `mh` |
| `test_pico8_jb_ol` | 5 | 0 | 2 | aides `draw_jb`/`draw_by_name`, commentaire l. 81 |
| `test_pico8_layout_nil_item_draws_nothing` | 4 | 0 | 2 | §6, l. 107-112 |
| `test_pico8_planet_gh_row_lq` | 1 | 0 | 2 | §6, commentaire l. 1-2 |
| **Total D.6b** | **232/244** | **21/25** (ponts) | | fichiers : 55 → 25 |

**Sites `overrides` de D.6c, intouchés** (97 lignes / 104 occurrences, 19 fichiers) : `pico8_cart.c` 6/6 (`ie` de `eu`), `pico8_economy.c` 4/5 et `.h` 1/1 (`hy`), `pico8_help.c` 4/5 et `.h` 2/2 (`fv`), `pico8_planet_trade.c` 7/7, `pico8_probe.c` 6/7 et `.h` 2/2 (`g_`), `pico8_research.c` 8/9 et `.h` 1/1 (`hp`), `pico8_research_icon.c` 7/8 et `.h` 2/2 (`ie`), `pico8_score_panel.c` 4/5 et `.h` 2/2 (`pj`), `pico8_trade.c` 13/14 et `.h` 2/2, `test_pico8_ie` 10, `test_pico8_entity_kind` 9, `test_pico8_probe_mm_rnd_order` 7.

## Solution

**Trois commits (chunks 1a, 1b, 2), sans aucun pixel changé** :

- **Classes sans clé** (`bg`, `nf`, `qb`, `ui`, `mz`) : `pico8_X_new(void)`. La seule table admise était NULL ou vide ; plus de paramètre, plus d'`assert`.
- **Clés typées comme les membres qu'elles remplissent**, NULL = aucune clé, lues jamais gardées : `pico8_iv_keys_t`, `pico8_jc_keys_t`, `pico8_fq_keys_t`, `pico8_lq_keys_t`, `pico8_js_keys_t`, `pico8_mh_keys_t` ; `jb` prend la rangée générée `const pico8_jb_row_t *`.
- **Appelants à clés littérales** (`planet.c`, `economy.c`, `planet_family_trade.c`, `menu.c`) : struct sur la pile, plus aucune table allouée. `fq(de)` lit `my` par `pico8_de_my()`.
- **`js:pq`** : une rangée copiée passe par `pico8_lq_keys_of` (plus de vue), une liste par ses éléments (plus de conversion), une table ob par le pont `lq`. **`jb:pq`** : une ligne chaîne devient une clé à un item (plus d'emballage), une ligne ob passe par le pont `lq`.
- **Pont `pico8_dr()` fermé** : `eu` construit ses `jb` sur `&pico8_dr_table[i]` ; `g_dr`, `data_init_dr_view` et `pico8_dr()` disparaissent.
- **Quatre ponts**, seuls lieux où une rangée ob reçue est lue par nom : `pico8_lq_new_table`, `pico8_jb_new_table`, `pico8_js_new_table`, `pico8_mh_new_table` ; paramètre `overrides` gardé (type inchangé, compté), fermés par D.7a/D.7b.
- **TU** : chaque table de clés d'un constructeur typé devient une struct locale écrite à son site ; les tests de mort qu'un type rend impossibles à écrire disparaissent ; ceux qui gardent un contrat vérifiable (classe de `jc.qu`, clés refusées par les ponts) restent.
- **§6** : `layout_nil_item_draws_nothing` et `gh_row_lq` réécrits, preuve rouge chacun.

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert ; `make e2e` : 26 conformes ; `make sanitize-net` : 26 propres ; `make class-coverage-net` : identique ; `make data-guard` : code 0 ;
- `make data-residue` : `overrides` = **118** lignes / **129** occurrences / **25** fichiers (au-delà : écart expliqué ligne par ligne, jamais par renommage), `spec` 0, `stash` 28/4, `ob` et `valeur` strictement plus bas, `par-nom` ≈ −13 (écart ≤ 3 expliqué) ; comptes en lignes ET en occurrences, avant et après chaque chunk, par catégorie et par fichier ;
- ces `grep` ne rendent rien :

  ```sh
  grep -rnE 'pico8_(lq|jb|js|fq|mh|iv|ui|qb|nf|bg|mz|jc)_(struct_)?new\((const )?pico8_ob_table_t' src tests --include='*.[ch]'
  grep -rnE 'pico8_(bg|nf|qb|ui|mz)_new\([^)]' src tests --include='*.[ch]'
  grep -rnE 'pico8_dr\(\)|data_init_dr_view|\bg_dr\b' src tests --include='*.[ch]'
  grep -nE 'pico8_lq_table_view|pico8_ob_table_from_list|pico8_js_lq_arg_table' src/pico8_ui_button.c
  grep -nE 'pico8_ob_table_new' src/pico8_planet.c src/pico8_planet_family_trade.c
  grep -rnwE 'iv_take_(vec|string|fixed)|lq_take_(list|w)' src
  ```
- `grep -rnw overrides src tests --include='*.[ch]'` ne rend que les 97 lignes de D.6c (mêmes comptes par fichier, même texte) et les 21 lignes des quatre ponts ;
- `grep -rn 'pico8_jb_row_view\|pico8_ob_table_new' src` : aucun site ajouté.

## User Stories

1. En tant que mainteneur du port, je veux que les douze constructeurs d'écran prennent une struct typée ou rien, afin qu'aucun toast, slipway, bouton, rangée ni infobulle ne soit plus construit en allouant, remplissant par nom et libérant une table ob là où l'appelant connaît ses clés.
2. En tant que mainteneur, je veux que `bg`, `nf`, `qb`, `ui` et `mz`, qui n'acceptaient qu'une table vide, n'aient plus de paramètre, afin que le compilateur porte le contrat « aucune clé ».
3. En tant que gardien du comportement, je veux que `iv`, `jc` et `fq` reçoivent `ra`/`at1`/`at2`, `qu` et `my` typés, absents ≡ défaut du membre, afin que les toasts de `jo`/`hy:lj`, les slipways de `jo:ldown`/`nr:ldown` et le voile de `fq(de)` soient identiques.
4. En tant que gardien du contrat, je veux que `jc` refuse toujours un `qu` qui n'est pas une planète de départ, par un `assert` à message, afin que le piège que le type `pico8_struct_header_t *` n'interdit pas reste gardé.
5. En tant que mainteneur, je veux que `lq` reçoive ses positionnels comme un tableau de `pico8_value_t` et `c`/`bg`/`score`/`ra` typés avec leur présence, afin que la rangée garde ses items hétérogènes (§6, D.9) sans table autour.
6. En tant que gardien du comportement, je veux qu'un item nil et un item fixe 0 d'une rangée ne dessinent rien, afin que `layout_nil_item_draws_nothing` garde son intention.
7. En tant que mainteneur, je veux que `js:pq` copie une rangée `lq` par `pico8_lq_keys_of`, sans vue ob, afin qu'aucune table ne soit plus allouée à chaque bouton qui recopie une rangée.
8. En tant que gardien du comportement, je veux qu'une liste passée comme `lq` d'un bouton donne ses éléments, dans l'ordre et nil compris, comme positionnels, afin que les rangées de `jo:gh` soient identiques sans conversion liste → table.
9. En tant que gardien du §6, je veux que `gh_row_lq` rougisse si la branche liste de `js:pq` relit la liste comme une table, afin que le bug d'origine reste gardé après le changement de type.
10. En tant que mainteneur, je veux que `jb` prenne la rangée générée `const pico8_jb_row_t *`, afin que `eu` construise ses infobulles de bâtiment sans la vue `pico8_dr()`.
11. En tant que gardien du comportement, je veux que chaque ligne chaîne d'un `jb` devienne une `lq` à un item sans table d'emballage, afin que les infobulles gardent leurs pixels.
12. En tant que mainteneur, je veux un seul pont par classe pour les rangées ob reçues, paramètre `overrides` gardé, afin que la lecture par nom qui reste soit localisée, comptée et fermable par l'étape qui type ses producteurs.
13. En tant que gardien du compteur, je veux qu'aucune ligne d'un pont ne soit un renommage de l'ancien paramètre, afin que la baisse d'`overrides` ne contienne aucune ligne déplacée sous un autre nom.
14. En tant que gardien du comportement, je veux que TROU-23 (une clé `h_` ou `jb` nil construit une rangée ou une infobulle vide) soit gardé, afin que le survol de `jo:fe` reste celui du filet.
15. En tant que gardien du comportement, je veux que TROU-25 (positionnels par indice croissant, trous resserrés) reste dans le pont `lq`, afin que la rangée « score 0 » du panneau de fin reste centrée comme aujourd'hui.
16. En tant que gardien du comportement, je veux que `mh` garde l'ordre des entrées de sa table pour ses items, afin que `pj:hk`/`pj:ph` affichent les mêmes boîtes.
17. En tant que mainteneur, je veux que `fq(de)` lise sa fonction par `pico8_de_my()`, afin que `menu.c` ne lise pas `g_de` par nom et que `g_de` reste à D.7a.
18. En tant qu'appelant de `pico8_X_new(NULL)` des classes à clés, je veux que NULL reste « aucune clé », afin que ces appels (TU, `score_panel_display`, `js:pq`) ne changent pas.
19. En tant que gardien du compteur, je veux les comptes avant/après chaque chunk en lignes ET en occurrences, par catégorie et par fichier, afin qu'une baisse prouve une construction retirée.
20. En tant que gardien du compteur, je veux que le compte attendu (118) soit expliqué contre la ligne (117), afin que la coïncidence de chiffre ne soit prise ni pour une confirmation ni pour un tassement.
21. En tant que gardien des TU, je veux que chaque table de clés d'un constructeur typé devienne une struct locale écrite à son site, sans registre ni aide qui reconstruise une table, afin que le test dise ce qu'il construit.
22. En tant que gardien des TU, je veux que les tests de mort devenus impossibles à écrire disparaissent avec leur raison au commit, et que ceux des ponts restent, afin qu'aucune vérité ne se perde sans être dite.
23. En tant que gardien des TU, je veux qu'aucune constante `*_HASH` ne bouge, afin de prouver que les instances construites sont les mêmes.
24. En tant que gardien des filets, je veux `make e2e` à 26, `make sanitize-net` à 26 propres et `make class-coverage-net` identique à chaque chunk, afin de prouver qu'aucun pixel, accès mémoire ni instanciation n'a changé.
25. En tant que gardien de la mémoire, je veux que D.6b n'ajoute aucune table ob ni vue construite à l'appel ou à la frame, afin que le tas ne porte pas plus de tables qu'avant.
26. En tant que lecteur de `docs/glossaire-noms.md`, je veux les entrées des structs de clés, des ponts, de `pico8_lq_keys_of` et de `pico8_de_my`, afin de retrouver les clés du cart depuis leur nom.
27. En tant que lecteur de `docs/trous-de-portage.md`, je veux que TROU-23 nomme le lieu où la présence nil est gardée, afin que l'entrée reste vraie.
28. En tant que `plan-pilot`, je veux une note « Découvert à D.6b » qui donne le compte réel, les ponts et leur étape de fermeture, les ponts de données reportés et la conséquence pour le « = 0 » de D.6c, afin que D.6c et D.7a partent de faits.
29. En tant que `plan-pilot`, je veux la liste de ce qui dépend de D.6a et de ce qu'il faut relire contre sa note, afin de ne pas lancer D.6b sur une hypothèse démentie.
30. En tant que `plan-pilot`, je veux que les clés de D.6c (`ie`, `hy`, `hp`, `g_`, `fv`, `trade`, `pj`) et les producteurs de rangées ob restent hors de ce commit, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Convention (G)

Celle de D.6a (H-D6a-3) : type `pico8_X_keys_t`, paramètre `const pico8_X_keys_t *keys`, NULL = aucune clé ; la struct est lue, jamais gardée ; chaînes, vecteurs et valeurs qu'elle pointe sont empruntés par l'instance sans copie, comme aujourd'hui. Chaque clé a le type du membre qu'elle remplit ; un drapeau de présence n'existe que là où le membre en a un ou a un défaut non nul. Aucun nom nouveau ne correspond à un motif figé (`_default`, `_spec`, `_defaults`, `overrides`), sauf le paramètre des ponts, voulu.

### Forme des clés, par classe (G)

Forme de décision (types seulement pour la précision ; l'implémenteur reprend les types exacts des membres) :

```c
/* pico8_msgbox.h */
typedef struct { pico8_vec_t *ra; const char *at1; pico8_fixed_t at2; } pico8_iv_keys_t;
typedef struct {
    const pico8_value_t *items; /* positionals in the table's entry order */
    int count;
    int has_rh;                 /* 0: rh keeps pico8_mh_start.rh */
    pico8_fixed_t rh;
    pico8_value_function_t lh;  /* NULL: nil */
} pico8_mh_keys_t;

/* pico8_slipway.h */
typedef struct { pico8_struct_header_t *qu; } pico8_jc_keys_t;

/* pico8_ui_button.h */
typedef struct { pico8_value_function_t my; } pico8_fq_keys_t;
typedef struct {
    int has_fa;                 /* 0: fa keeps pico8_js_start.fa */
    pico8_fixed_t fa;
    pico8_value_t lq;           /* row source lq(x) copies: table, list, lq row or nil */
    int has_h_;                 /* h_ present: nil builds an empty row (TROU-23), false none */
    pico8_value_t h_;
    int has_jb;                 /* jb present: nil builds an empty jb (TROU-23) */
    pico8_value_t jb;
    pico8_vec_t *ra;
    pico8_vec_t *qf;
    int has_my;                 /* has_my with my NULL: false */
    pico8_value_function_t my;
    /* hb, fg/has_fg, px, trade/has_trade, en/has_en, _cr_t, _jo_fe_pt, _jo_fe_b,
     * _jo_fe_od/has__jo_fe_od: the member types, one field per line */
} pico8_js_keys_t;

/* pico8_layout.h */
typedef struct {
    const pico8_value_t *items; /* positionals, in order; copied into lq.items, never kept */
    int count;
    int has_c;
    pico8_fixed_t c;
    int has_bg;
    int bg_is_bool;
    pico8_fixed_t bg;
    int has_score;
    pico8_fixed_t score;
    pico8_vec_t *ra;
} pico8_lq_keys_t;
/* pico8_jb_t *pico8_jb_new(const pico8_jb_row_t *row); NULL: no line */
```

| Classe | Paramètre | Ce que le constructeur pose |
|---|---|---|
| `bg`, `nf`, `qb`, `ui`, `mz` | aucun | ses défauts seulement ; `assert` de table vide supprimé |
| `iv` | `const pico8_iv_keys_t *keys` | `ra`, `at1`, `at2` (NULL / 0 = défaut) |
| `jc` | `const pico8_jc_keys_t *keys` | `qu` ; `assert` à message : NULL ou planète de départ (classe de `jc_take_endpoint`) |
| `fq` | `const pico8_fq_keys_t *keys` | `my` |
| `mh` | `const pico8_mh_keys_t *keys` | `items` ajoutés dans l'ordre, `rh` si présent, `lh` |
| `lq` | `const pico8_lq_keys_t *keys` | `items` copiés dans `lq.items`, `c`/`bg`/`score`/`ra` sous leurs drapeaux ; `lq:pq` recalcule `fs`/`w` |
| `jb` | `const pico8_jb_row_t *row` | une `lq` par item : `.s` → rangée à un item ; `.row` → rangée des items scalaires de la ligne (`.s`, `.n`, `.fixed`, `false`) et de son `c` ; autre item → rangée vide ; `assert` à message : pas de `c`/`fn` au premier niveau, pas de rangée ni de `fn` dans une ligne |
| `js` | `const pico8_js_keys_t *keys` | les membres typés ; puis `js:pq` avec `lq`, `h_` (présent à nil → `true`, TROU-23), `jb` (idem) comme aujourd'hui |

- **`js:pq` et ses sources de rangée** (`pico8_js_lq`) : table ob → `pico8_lq_new_table` ; liste → `pico8_lq_keys_t` dont `items` sont les éléments de la liste ; rangée `lq` → `pico8_lq_keys_of(row)` ; nil ou `true` → `pico8_lq_new(NULL)` ; tout autre REF → `assert` (message gardé). `jb` : table ob → `pico8_jb_new_table` ; `true` (TROU-23) → `pico8_jb_new(NULL)` ; `false` → `assert` (message gardé).
- **`pico8_lq_keys_of(const pico8_lq_t *row)`** rend les clés par valeur : `items` empruntés à `row->items` (tous, dans l'ordre), `c`/`bg`/`score`/`ra` de la rangée. Aucune allocation.
- **Pourquoi `lq`/`h_`/`jb` de `js` et les `items` de `lq`/`mh` restent des `pico8_value_t`** : ce sont des sources de rangée et des items hétérogènes (table, liste, rangée, nil, `true`/`false`, fonction, fixe) que leurs producteurs (D.7a, D.7b) construisent encore en ob ; le point 3 de l'arrêt les laisse à D.9. La **table de clés** disparaît ; les **valeurs** restent des valeurs.
- **Pourquoi `jb` prend `pico8_jb_row_t`** : les clés d'un `jb` sont exactement ses lignes positionnelles, et ce type est déjà la forme générée des rangées `dr`/`nb.oy`/`px`. Une struct de plus n'ajouterait qu'une enveloppe.
- **Rejetés** :
  - **garder `pico8_ob_table_t` et renommer le paramètre** — renommage à type constant, le tassement interdit ;
  - **des clés imbriquées typées pour `js.lq`/`h_`/`jb`** (`const pico8_lq_keys_t *`, lignes de `jb` en `pico8_lq_keys_t`) — tous les producteurs sont ob jusqu'à D.7a/D.7b : le pont devrait lire les rangées imbriquées et gérer leur stockage, pour un typage cosmétique ; les items imbriqués demanderaient encore une vue ;
  - **faire construire les `lq`/`jb` du bouton par l'appelant** — `js:pq` du cart les construit lui-même ; l'ordre des constructions changerait, le port cesserait d'être littéral ;
  - **typer tous les producteurs de rangées ici** (`planet_display`, `research_display`, `score_panel_display`, `menu`, `help`, `pz`) — c'est D.7a/D.7b dans D.6b (≈ 60 chemins de plus), et les items imbriqués resteraient ob ;
  - **un pont au paramètre renommé** (`row`, `src`) — les lignes de la boucle y seraient déplacées sous un autre nom au même type : c'est la classe (c) interdite ; elles restent donc `overrides` et comptées ;
  - **une struct pour les classes sans clé** — une struct vide pour dire « rien » ; le type `void` le dit ;
  - **« NULL = défaut » par drapeau pour `iv`/`jc`/`fq`** — les défauts sont NULL/0, un drapeau n'ajouterait rien.

### Les quatre ponts (G)

Seuls lieux où une rangée ob **reçue** est lue par nom. Chacun lit sa table comme le constructeur le fait aujourd'hui (mêmes `assert`, mêmes aides `*_take_*` qui écrivent maintenant dans la struct), remplit une struct locale, appelle le constructeur typé et libère ce qu'il a alloué pour les items.

| Pont | Fichier | Lit | Appelants |
|---|---|---|---|
| `pico8_lq_t *pico8_lq_new_table(const pico8_ob_table_t *overrides)` | `layout.c/.h` | positionnels par indice croissant (TROU-25, marqueur déplacé avec la lecture), `c`, `bg`, `score`, `ra` ; un autre nom `assert` (message gardé, `fs`/`w` compris) | `js:pq` (table), `jb` (pont, ligne table), `score_panel_display.c` (`st`), `research_display.c` (`l`) |
| `pico8_jb_t *pico8_jb_new_table(const pico8_ob_table_t *overrides)` | `layout.c/.h` | positionnels ; un nom `assert` (message gardé) ; ligne chaîne → rangée à un item, ligne table → `pico8_lq_new_table`, autre → rangée vide | `js:pq` (table) |
| `pico8_js_t *pico8_js_new_table(const pico8_ob_table_t *overrides)` | `ui_button.c/.h` | les seize clés, avec la présence de `h_`/`jb` (TROU-23 appliqué par le constructeur) ; clé inconnue ou positionnelle `assert` (message gardé) | `pz` (`msgbox.c`), `eu` (`cart.c`), `pj:ph` (`spec1`/`spec2`) |
| `pico8_mh_t *pico8_mh_new_table(const pico8_ob_table_t *overrides)` | `msgbox.c/.h` | positionnels dans l'ordre des entrées, `rh`, `lh` ; autre clé `assert` (message gardé) | `pj:hk` (`g_pj_ea`), `pj:ph` (`iw_table`) |

- **Lignes `overrides` des ponts, exactement** (squelette liant) : pour `lq`, la signature, l'en-tête de boucle (2 occurrences), les lectures `key`/`value` de l'entrée, la lecture des positionnels — 5 lignes / 6 occurrences ; pour `jb`, la signature, l'en-tête de boucle de l'`assert` (2), l'`assert`, la lecture des positionnels — 4 / 5 ; pour `js` et `mh`, la signature, l'en-tête de boucle (2), `key`, `value` — 4 / 5 chacun ; une déclaration par pont dans son en-tête — 4 / 4. **Total 21 lignes / 25 occurrences.** Le commentaire de contrat d'un pont ne répète pas le nom du paramètre (« table lue, jamais gardée ; ses valeurs sont empruntées »).
- **Pas de vue, pas de registre** : un pont ne construit aucune table ob ; il peut allouer une `pico8_list_t` temporaire pour les items (libérée après la construction, `sanitize-net` le vérifie).
- **Étape qui les fermera** : quand les rangées de `pz` et les sources `lq`/`h_`/`jb`/`mh` sont typées — producteurs `planet_display`, `menu`, `score_panel_display`, `research_display` (D.7a) et `help`, `msgbox`/`pz` (D.7b). La note le dit ; le PRD de D.7a tranche l'ordre (le « 0 dans `layout` » de D.7a exige les ponts `lq`/`jb` fermés).

### Chunk 1a — classes sans clé (G)

- `pico8_bg_new(void)`, `pico8_nf_new(void)`, `pico8_qb_new(void)`, `pico8_ui_new(void)`, `pico8_mz_new(void)` ; `static void *pico8_X_struct_new(void)` ; l'`assert` « NULL ou vide » et son message disparaissent (le type l'interdit) ; les `assert` de `ui` qui nomment `pico8_ui_new(NULL)` sont réécrits pour nommer `pico8_ui_new()`. Contrats d'en-tête réécrits sans le paramètre.
- Appelants : `cart.c` (`bg`), `bootstrap.c` (`qb`, `nf`, `ui`), `bg.c` (`mz`), TU.

### Chunk 1b — `iv`, `jc`, `fq` (G)

- **`iv`** : `pico8_iv_new(const pico8_iv_keys_t *keys)` ; la boucle, les trois aides `iv_take_vec`/`_string`/`_fixed` (mortes, `-Werror`) et l'`assert` « every key is ra, [1] or [2] » disparaissent. Appelants : `planet.c` (toast de `jo`, `ra` du toast = `self->ra`, texte, couleur), `economy.c` (`hy:lj`, deux toasts : « -no money- »/8 et texte/couleur) — **lignes de D.6c touchées : aucune** ; la déclaration commune `overrides` de `hy:lj` (l. 243) part avec ses deux tables.
- **`jc`** : `pico8_jc_new(const pico8_jc_keys_t *keys)` ; `jc_take_endpoint` devient l'`assert` de classe du constructeur (même message) ; la boucle et l'`assert` « the only construction key is qu » disparaissent. Appelants : `planet.c` (`jo:ldown`), `planet_family_trade.c` (`nr:ldown`).
- **`fq`** : `pico8_fq_new(const pico8_fq_keys_t *keys)` ; boucle et `assert` « the only key is my » disparaissent ; `button_take_function` reste (pont `js`). `menu.c` (`ix[3].my`) : `keys.my = pico8_de_my()`. **`pico8_de_my()`** (`score_panel_display.c/.h`) rend la fonction que `g_de` porte (`pico8_pj_de_my_dispatch`), sans lire `g_de` ; `pico8_de()` et `g_de` restent (parse de `spec2`, TU ; D.7a).

### Chunk 2 — `lq`, `jb`, `js`, `mh`, ponts, `dr`, §6 (G)

- **`lq`** : `pico8_lq_new(const pico8_lq_keys_t *keys)` ; le constructeur copie `items` dans `lq.items`, pose `c`/`bg`/`score`/`ra` ; `lq_take_list` et `lq_take_w` meurent (aucune source de `fs`/`w` ne survit) ; `pico8_lq_keys_of` ajouté ; le pont `lq` garde la boucle. `menu.c` (`ix[2].hb`) : un item fixe (24 ou 37) dans une struct locale. `pico8_lq_table_view` reste, **sans appelant de `src/`** (TU seulement ; fermée avec `layout` en D.7a).
- **`jb`** : `pico8_jb_new(const pico8_jb_row_t *row)` ; le pas de `jb:pq` perd sa variable locale et sa table d'emballage ; pont `jb`. `eu` (`cart.c`) : `pico8_jb_new(&pico8_dr_table[i])` — **les six lignes `overrides` de `ie` dans la même boucle ne changent pas d'un octet**.
- **Pont `pico8_dr()` fermé** : `g_dr`, `data_init_dr_view`, `pico8_dr()` et leur contrat supprimés de `data_init.c/.h` ; le commentaire de `pico8_data_init` qui dit « fd, dr and nb are built » est corrigé.
- **`js`** : `pico8_js_new(const pico8_js_keys_t *keys)` ; `pico8_js_lq_arg_table` et son commentaire (l. 24) disparaissent ; `pico8_js_lq` suit la décision plus haut ; le marqueur TROU-7 de la conversion liste → table part avec elle (TROU-7 reste, entrée générale) ; `/* TROU-23 */` reste au site de la substitution nil → `true`, dans le constructeur. `ui_button.c` n'inclut plus `pico8_ref_audit.h` si `PICO8_REF_AS_LIST_CONST` n'y a plus d'usage. Appelants : `pz` (`pico8_js_new_table(b)`), `eu` (`pico8_js_new_table(row)`), `pj:ph` (`spec1`/`spec2` par le pont ; leur parse par appel reste, D.7a).
- **`mh`** : `pico8_mh_new(const pico8_mh_keys_t *keys)` ; pont `mh` ; `pj:hk` et `pj:ph` passent par lui (leurs tables sont l'état `g_pj_ea` et une table d'ob construite d'une liste, D.7a).
- **§6** : voir « Testing Decisions ».

### `Makefile`, docs

- **Le `Makefile` ne change pas** : aucun fichier nouveau. `DATA_RESIDUE_GUARD` reste `spec` (H-D6a-6) : `overrides` ne tombe pas à 0.
- `docs/glossaire-noms.md` : entrées `pico8_iv_keys_t`, `pico8_jc_keys_t`, `pico8_fq_keys_t` (chunk 1b), `pico8_lq_keys_t`, `pico8_js_keys_t`, `pico8_mh_keys_t`, `pico8_lq_keys_of`, les quatre ponts `pico8_X_new_table`, `pico8_de_my` ; lignes `lq`, `jb`, `js` mises à jour (clés, `jb` sur `pico8_jb_row_t`).
- `docs/trous-de-portage.md` : TROU-23, dernière phrase — la présence nil est portée par `has_h_`/`has_jb` des clés de `js` (posées par le pont), et le constructeur de `js` la remplace par `true`. Aucune autre entrée.
- `docs/plan-3.md` : note « Découvert à D.6b » en fin de ligne D.6b (texte dans Further Notes). Rien d'autre dans le plan.

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucun ordre de construction (`js:pq` construit toujours ses `lq`/`jb` après les clés), aucun tirage du RNG, aucune valeur.
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements ; aucun e2e ajouté.
- Les 97 lignes `overrides` de D.6c (liste de l'inventaire), `pz` hormis son appel de `js`, les producteurs de rangées (`planet_display`, `research_display` hormis son appel de `lq`, `score_panel_display` hormis ses appels et `pico8_de_my`, `help`, `menu` hormis `ix[2].hb`/`ix[3].my`), les vues `ix`/`ha`/`bo`/`bz`, `pico8_jb_row_view`, `jo_kw_view`, `pico8_nb()`, `g_de`, les écrivains par nom (`set_field`, aides `*_take_*` des ponts), `pico8_ob.c/.h`, `tools/`, `KNOWN[]`.
- `ga_export_panel_by`.
- **TROU** : aucun écart port/cart corrigé ni ajouté ; TROU-7, TROU-23, TROU-25 préservés (le marqueur TROU-7 de la conversion liste → table part avec le code qu'il marquait ; aucune fuite n'est ajoutée).

### Anti-tassement (liant)

Le compteur compte des **lignes**, et le motif `overrides` est un **nom de paramètre**. Une baisse obtenue sans retirer la construction comptée est un échec.

- **Interdit** : renommer un paramètre, une variable ou un commentaire `overrides` **sans que le type de la donnée change** — c'est le cas des 97 sites de D.6c et des ponts ; joindre sur une ligne des déclarations, instructions, arguments, conditions, champs ou entrées d'initialiseur qui étaient sur des lignes distinctes ; retirer des retours à la ligne dans une ligne comptée ; macro, `typedef` ou fonction enveloppe dont le seul rôle est de renommer un identifiant compté ; **un registre dans un TU** (tableau, table, `switch` ou boucle nouveaux qui nomment ensemble classes, clés ou valeurs attendues ; les aides existantes sont retypées, pas étendues) ; **une aide de TU qui reconstruit une table de clés par nom** pour un constructeur typé ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; supprimer un commentaire qui ne porte pas sur une clé, une boucle, une aide ou un contrat retiré par D.6b ; **toute vue ob nouvelle construite à l'appel ou à la frame**, toute `pico8_ob_table_new` ajoutée dans `src/`.
- **Légitimité, contrôle exigé** : chacune des **211 lignes / 219 occurrences** `overrides` retirées est, au commit de son chunk, classée (a) construction supprimée (allocation, pose, libération, boucle, lecture d'entrée, variable ou commentaire d'une table de clés qui n'existe plus), ou (b) signature, déclaration ou commentaire de contrat dont le paramètre passe à une struct typée, à `pico8_jb_row_t` ou à rien ; aucune ligne (c) « même type, autre nom ». Chacune des **21 lignes** des ponts est classée (e) : ligne d'un pont, table ob de même type **et même nom** `overrides`, gardée et comptée ; aucune ligne d'un pont hors des quatre fonctions et de leurs déclarations. Recette : `git diff -U0` de chaque fichier, lignes `-` qui portent `overrides` en regard des lignes `+` ; aucune ligne `+` ne porte `pico8_ob_table_t` dans une signature d'un des douze constructeurs.
- **Mesure exigée, avant et après chaque chunk** : par catégorie, **lignes** (`make data-residue`, et `--detail` par fichier) **ET occurrences** (recette ci-dessous), le test ponctuel donné à part. Recette figée pour l'implémenteur et le vérificateur :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  X=$(sed -n "s/^SPEC_EXCLUSION='\(.*\)'\$/\1/p" tools/data_residue.sh)
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E "$X" "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  ```

  Mesuré sur `a29154e` : `valeur` 6625, `ob` 2127, `stash` 28, `type-tag` 664, `ref-audit` 69, `par-nom` 1161, `dispatch` 199, `overrides` 479, `spec` 0. Départ réel : l'arbre de D.6a, remesuré.
- **Contrôle** : pour chaque catégorie et chaque fichier du diff, Δlignes et Δoccurrences. **Si Δlignes > Δoccurrences** dans un couple (catégorie, fichier), le vérificateur lit le diff de ce fichier : chaque ligne comptée retirée correspond à une construction retirée ou convertie, pas à une construction qui réapparaît sur une autre ligne. Le message de commit liste ces couples et leur justification.
- **Sites hors périmètre** : les 97 lignes de D.6c ont, par fichier, le même compte (lignes et occurrences) et le même texte avant et après (`git diff` vide sur ces lignes).
- **Lignes qui gagnent une catégorie**, listées au commit ; attendues seulement : `valeur` (`items` de `pico8_lq_keys_t`/`pico8_mh_keys_t`, `lq`/`h_`/`jb` de `pico8_js_keys_t`, et les tableaux d'items locaux des appelants), `dispatch` (champs fonction `my`, `hb`, `lh` des structs et `pico8_de_my`). Aucune autre.

### Empreinte remesurée (G)

| Chunk | `src/` | `tests/` | docs | Total |
|---|---|---|---|---|
| 1a | `bg.c/.h`, `mouse_reader.c/.h`, `scroll.c/.h`, `ui.c/.h`, `wormhole.c/.h`, `cart.c`, `bootstrap.c` (12) | `bg`, `g_`, `ie`, `iv`, `jc`, `je`, `mo`, `mz`, `nf`, `qb`, `spec_defaults`, `ui` (12) | — | 24 |
| 1b | `msgbox.c/.h`, `slipway.c/.h`, `ui_button.c/.h`, `planet.c`, `planet_family_trade.c`, `economy.c`, `menu.c`, `score_panel_display.c/.h` (12) | `iv`, `jc`, `fq`, `js`, `lq`, `mx`, `mz`, `ui`, `hp`, `hy`, `trade`, `g_` (12) | glossaire (1) | 25 |
| 2 | `layout.c/.h`, `ui_button.c/.h`, `msgbox.c/.h`, `menu.c`, `cart.c`, `score_panel_display.c`, `research_display.c`, `data_init.c/.h` (12) | `js`, `lq`, `mh`, `jb_ol`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `data_init`, `map_tables` (test ponctuel), `entity_kind`, `ie` (10) | glossaire, `trous-de-portage.md`, `plan-3.md` (3) | 25 |
| **Distincts** | **29** | **26** | **3** | **58** |

- **Écart avec la ligne (30 : 19, 11)** : la ligne comptait les fichiers qui portent le mot. Elle ne voyait pas les appelants à `NULL` des classes sans clé (`bootstrap.c`, `cart.c`, douze TU), `economy.c` (toasts de `hy:lj`), `score_panel_display.c`, `research_display.c`, `data_init.c/.h` (pont `dr`), ni les TU qui construisent dans une variable `b` ; elle comptait des fichiers de D.6c (`test_pico8_hp`, `_hy`, `_trade` y sont pour leur `jc`, pas pour leur classe).
- **Découpage (G) : trois commits**, 58 chemins au-dessus du seuil de ~40, chacun vert seul et dans cet ordre : 1a ne dépend de rien ; 1b ne touche ni `lq` ni `js` (le constructeur de `fq` et celui de `js` partagent `ui_button.c`, pas de signature) ; 2 s'appuie sur 1b (`ui_button.c`, `msgbox.c`, `menu.c`). **Rejetés** : **un commit** (58 chemins) ; **un commit par classe** (`lq`, `jb`, `js` sont liés par `js:pq` et `jb:pq` : une signature à la fois demanderait une double forme ou un alias, interdits) ; **les ponts d'abord** (ponts sans constructeur typé : un commit sans objet).

### Effet sur `make data-residue`

Attendu, lignes / occurrences / fichiers (départ : ce que D.6a laisse, H-D6a-1) :

| Catégorie | Départ | Après 1a | Après 1b | Après 2 | Pourquoi |
|---|---|---|---|---|---|
| `overrides` | 329 ; 348 ; 55 | **299 ; 313 ; 45** | **174 ; 185 ; 31** | **118 ; 129 ; 25**, exact | inventaire par fichier ; restent D.6c (97 ; 104 ; 19) et les ponts (21 ; 25 ; 6) |
| `spec` | 0 | 0 | 0 | **0** | garde |
| `par-nom` | P₀ | P₀ | ≈ P₀ − 9 | **≈ P₀ − 13** (écart ≤ 3 expliqué) | `iv_take_*` (3 définitions, 3 usages), `jc_take_endpoint` (définition, usage), usage de `button_take_function` par `fq` ; `lq_take_list`/`lq_take_w` (2 définitions, 2 usages) |
| `ob` | rapporté | plus bas | plus bas | **strictement plus bas** à chaque chunk | tables des appelants et des TU, emballage de `jb:pq`, vue et conversion de `js:pq`, `pico8_js_lq_arg_table`, pont `dr` ; ponts : aucune ligne ajoutée hors déplacement |
| `valeur` | rapporté | ≤ | plus bas | **strictement plus bas** | poses `pico8_value_*` des tables retirées ; gains listés (champs `pico8_value_t` des clés) |
| `type-tag` | rapporté | ≤ | ≤ | **≤ départ** | `assert` de tag des aides mortes, `switch` de `pico8_js_lq_arg_table` |
| `dispatch` | 197 ; 199 | = | rapporté | **rapporté, chaque ligne gagnée listée** | champs fonction des clés, `pico8_de_my` ; `pico8_value_function(...)` des tables de TU retirées |
| `ref-audit` | rapporté | = | = | **≤** | inclusion de `ui_button.c` si devenue inutile |
| `stash` | 28/4 | 28/4 | 28/4 | **28/4** | test ponctuel : 2 lignes inchangées |

- **`overrides` = 118 / 129 est attendu exact** ; tout écart est expliqué ligne par ligne au commit, et **une baisse sous 118 est un échec** sauf à prouver qu'une ligne de plus a changé de type (jamais par renommage). Le « = 117 » de la ligne ne l'est pas, pour la raison mesurée plus haut ; la note corrige.
- Aucune catégorie ne tombe à 0 : `DATA_RESIDUE_GUARD` ne change pas.
- **Conséquence pour D.6c** : son « = 0 » ne peut pas tenir tant que les ponts vivent ; il devient **« = 21 » (les ponts)**, et `overrides` entre dans `DATA_RESIDUE_GUARD` à l'étape qui ferme le dernier pont. La note le dit.

### Hypothèses sur D.6a

- **H-D6a-1** : D.6a est livrée et gradée comme son PRD le dit : `overrides` = **329 lignes / 348 occurrences / 55 fichiers** au départ de D.6b, dont les 232 / 244 de D.6b et les 97 / 104 de D.6c aux comptes par fichier de l'inventaire.
- **H-D6a-2** : D.6a ne touche le texte d'aucune ligne `overrides` de D.6b ni de D.6c ; seuls les numéros de ligne bougent (`pico8_planet.c`, `pico8_planet_family_trade.c`, `pico8_research_icon.c`, `test_pico8_mz`, `test_pico8_mx`, `test_pico8_ui`, `test_pico8_jc`, `test_pico8_hp`).
- **H-D6a-3** : la convention de D.6a est `pico8_jo_keys_t` (`id`, `p`, `ra`, NULL = absent, `pico8_struct.h`), `pico8_ga_keys_t`, `pico8_ga_pq_keys_t`, paramètre `const … *keys`, NULL = aucune clé, `pico8_X_new(NULL)` inchangé ; `new_entity` prend `const pico8_jo_keys_t *`. D.6b la reprend ; ses TU construisent les planètes de `jc.qu` avec ces clés.
- **H-D6a-4** : `test_pico8_planet_gh_row_lq.c` est intouché par D.6a ; son commentaire l. 1-2 est reporté à D.6b (la note de D.6a le dit).
- **H-D6a-5** : aucun nom de D.6a n'entre en collision avec ceux de D.6b (`pico8_{iv,jc,fq,lq,js,mh}_keys_t`, `pico8_X_new_table`, `pico8_lq_keys_of`, `pico8_de_my`).
- **H-D6a-6** : `DATA_RESIDUE_GUARD` = `spec` ; `make data-guard` vert.
- **H-D6a-7** : la note de D.6a laisse le « = 0 » de D.6c inchangé et dit « le compte de D.6b est à reprendre » ; D.6b le reprend et propose de changer celui de D.6c.
- **H-D6a-8** : D.6a retire `jo_take_vec`, `family_take_string`, `family_take_seq`, `family_trade_take_string`, `family_trade_take_vec`, et aucune aide de `layout`, `ui_button`, `msgbox`, `slipway` ; les ≈ 13 lignes `par-nom` de D.6b existent au départ.
- **H-D6a-9** : l'arbre de D.6a est vert (`make test`, `make e2e` 26, `make sanitize-net` 26, `make class-coverage-net` identique, `make data-guard` 0).
- **H-D6a-10** : D.6a ne touche ni `layout`, `ui_button`, `msgbox`, `slipway`, `menu`, `cart.c`, `bootstrap.c`, `economy.c`, `data_init`, `score_panel_display`, `research_display`, ni les classes sans clé.
- **H-D6a-11** : D.6a ajoute `pico8_pm_ga_pq_keys` (`pico8_game_data.c/.h`) ; D.6b n'ajoute rien à `pico8_game_data.*`.
- **H-D6a-12** : le test ponctuel, étendu par D.6a (`pm:pq`), garde `test_dr_view_matches_the_decoded_table` et ses 2 lignes `stash`.

**Démentie** : l'hypothèse fausse est nommée au commit, la section concernée de ce PRD est relue ; seule une ligne `overrides` de D.6b ou D.6c touchée par D.6a (H-D6a-1/2) oblige à recalculer le compte attendu, sans jamais l'abaisser par renommage.

### À revérifier après D.6a (contre sa note « Découvert à D.6a »)

1. **Problem Statement / Inventaire** : compte réel `overrides` de D.6a, partage 232 / 97 par fichier (H-D6a-1/2), numéros de ligne cités.
2. **Convention et forme des clés** : noms des types et du paramètre de D.6a (H-D6a-3/5).
3. **Chunk 1b / TU `jc`** : forme des clés de planète dans `test_pico8_{jc,js,lq,mx,mz,ui,hp,hy,trade,g_}` (H-D6a-3).
4. **§6 `gh_row_lq`** : report effectif du commentaire (H-D6a-4).
5. **Effet sur `make data-residue`** : départs de `par-nom`, `ob`, `valeur`, `dispatch`, `type-tag` (H-D6a-8), `DATA_RESIDUE_GUARD` (H-D6a-6), phrase sur D.6c (H-D6a-7).
6. **Empreinte remesurée** : fichiers que D.6a aurait touchés hors de son PRD (H-D6a-10) ; test ponctuel (H-D6a-12).
7. **Critères d'échec** : l'arbre de départ (commit gradé de D.6a).

### Hypothèses sur D.6b (rien n'a été lancé)

- **H1** : l'arbre de départ est vert et `make data-residue` y rend l'inventaire. Démenti : arrêt, D.6b ne répare rien.
- **H2** : aucun appelant d'un des douze constructeurs hors de l'inventaire (grep sur `a29154e` : `src/` et 23 TU ; aucun pointeur de fonction vers eux).
- **H3** : `lq:pq` réécrit toujours `fs` et `w` ; aucune rangée ob produite hors de `pico8_lq_table_view` ne porte `fs` ni `w` (grep : seules `bg` et `score` sont posées par nom hors de `layout.c`).
- **H4** : les lignes de `pico8_dr_table` n'ont que des items `.s`/`.n` et ni `c` ni `fn` au premier niveau ; `pico8_jb_new(&pico8_dr_table[i])` construit les mêmes lignes que la vue.
- **H5** : aucune constante `*_HASH` (ni son nom) ne bouge ; les tests de mort retirés ne nourrissent aucun hachage. Démentie : arrêt sur la constante, jamais de mise à jour.
- **H6** : `pico8_lq_keys_of(row)` et la vue d'hier donnent la même rangée : items dans l'ordre (positionnels ajoutés après `pq` compris), `c`/`bg`/`score`/`ra` identiques.
- **H7** : la liste passée à `js:pq` et la table que `pico8_ob_table_from_list` en tirait ont les mêmes positionnels (vérifié par lecture de `pico8_ob.c`/`pico8_list.c`).
- **H8** : `make class-coverage-net` est identique : aucune instanciation ajoutée ni retirée (une `lq` par `lq(x)`, une par ligne de `jb`).
- **H9** : `make sanitize-net` rend 26 propres : les tables de clés passent du tas à la pile, les listes temporaires des ponts sont libérées, rien d'autre ne change de propriétaire.
- **H10** : `-Werror` reste muet : aides mortes supprimées, inclusions devenues inutiles retirées.
- **H11** : `constants-check`, `float-grep`, `float-guard` restent verts : aucun flottant ni `PICO8_FIXED_LIT` ajouté.
- **H12** : les tests de mort gardés (classe de `jc.qu`, clés refusées par les ponts) rougissent si leur `assert` est retiré.
- **H13** : `pico8_de_my()` rend la fonction que `g_de` porte (`test_pico8_pj` l. 319 lit toujours `g_de`).
- **H14** : durées courtes : `make test` en quelques secondes plus une recompilation large ; `make e2e` ~1 s ; `sanitize-net` et `class-coverage-net` < 2 min chacun, par chunk.
- **H15** : les 26 e2e atteignent `eu`, `pz`, `jo:gh`, `pj:ph`, `hy:lj` et un toast de `jo` ; un site non atteint est prouvé par un TU.
- **H16** : `make raylib` (non exigé, jamais lancé) lierait : aucun fichier nouveau.

## Implementation Chunks

Trois commits, dans cet ordre, chacun vert seul (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`, `make data-guard`), chacun avec ses comptes avant/après, sa classification et son rapport gcov/mutations :

1. **1a — classes sans clé** : `bg`, `nf`, `qb`, `ui`, `mz` sans paramètre ; appelants et TU ; tests de mort « une clé `assert` » retirés. `overrides` → 299 / 313 / 45.
2. **1b — `iv`, `jc`, `fq`** : structs de clés, appelants à clés littérales (`planet.c`, `economy.c`, `planet_family_trade.c`, `menu.c` `ix[3]`), `pico8_de_my`, aides mortes, TU ; glossaire (trois entrées). `overrides` → 174 / 185 / 31.
3. **2 — `lq`, `jb`, `js`, `mh`, ponts, `dr`, §6** : structs, `pico8_lq_keys_of`, ponts, `js:pq`/`jb:pq`, `eu`, `pz`, `pj:hk`/`pj:ph`, `research_display`, `ix[2].hb`, fermeture de `pico8_dr()`, test ponctuel, les deux tests du §6 avec preuve rouge, TU ; glossaire, TROU-23, note du plan. `overrides` → 118 / 129 / 25.

Ordre conseillé dans chaque commit : mesure de départ (lignes et occurrences, par fichier) ; types et constructeurs ; appelants de `src/` ; `make test` ; TU ; `make test` ; `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois ; gcov, mutations ; mesure d'arrivée, classification, anti-tassement, gardes ; docs.

## Testing Decisions

Un bon test juge ce que le code rend (instances, valeurs, identités, pixels), pas la présence d'une ligne. D.6b change la forme des clés, pas ce qu'elles posent : **la preuve est que chaque constructeur pose, pour les mêmes clés, les mêmes membres qu'avant, que chaque pont lit une rangée ob comme le constructeur la lisait, que `jb` sur une rangée générée dessine comme sur sa vue, et que tout ce qui mesure le comportement rend la même chose.**

### TU touchés : clés en struct locale, mêmes constantes (G)

- **Règle** : chaque table (`b`, `overrides`, `t`…) passée à un constructeur typé devient une struct locale initialisée à son site, un champ par ligne ; plus de `pico8_ob_table_new`/`set_*`/`free` pour ces constructions ; aucune aide nouvelle qui construise des clés par nom. Les aides existantes (`draw_jb`, `draw_by_name`, `render_one_item`, `dr_row`, les constructeurs de planète de `jc`) sont retypées, sans cas ajouté.
- **Les tests des ponts gardent leurs tables** : ce qu'un pont seul fait (clé inconnue ou positionnelle refusée, valeur hors type refusée, TROU-23, TROU-25, ordre des entrées de `mh`, source liste/rangée/table de `js:pq`) reste testé sur une table ob passée au pont — ce sont ses tests, ils tombent avec lui.
- **Tests de mort, un par un** (liste au commit) :
  - **retirés, le type les rend impossibles à écrire** : `ui` (`build_with_named_key`, `build_with_positional_key`), `mz` (table non vide), `iv` (`must_abort_key` : `ra`/`[1]`/`[2]` hors type ou clé inconnue), `fq` (`build_with_key`), `jc` (`build_with_ra`), et tout test « table non vide » de `bg`/`nf`/`qb` ;
  - **gardés** : `jc` `build_off_row` (un `qu` d'une classe qui n'est pas une planète de départ : le type `pico8_struct_header_t *` le permet) ; `js` (`build_with_unknown_key`, `_positional_key`, `build_with_mb`, `build_off_row`), `lq` (`build_with_named_key`), `mh` (`build_with_key`, `build_rh_string`, `build_rh_nil`, `build_lh_fixed`) **contre leur pont** ; les écritures hors rangée par `set_field` (`must_abort_write` de `iv`…) ne changent pas.
- **`pico8_dr()` dans les TU** : `test_pico8_jb_ol`, `test_pico8_ie`, `test_pico8_entity_kind` passent `&pico8_dr_table[i]` à `pico8_jb_new` ; `test_pico8_js` (l. 907), qui met une rangée `dr` dans la clé `jb` d'un bouton, la prend par `pico8_jb_row_view(&pico8_dr_table[i])` (vue de TU, précédents `test_pico8_fv`/`_pj`/`_hp`) et passe par le pont ; `test_pico8_data_init` (l. 29-38) perd sa lecture de `pico8_dr()` — `data_init` ne construit plus `dr` — et vérifie à la place `PICO8_DR_COUNT` et le titre de la première entrée (« -lab- ») sur `pico8_dr_table`, même intention.
- **`test_pico8_lq`** : la copie `rows[5]` (l. 927-928, `RAW_PRESENCE_HASH`) passe par `pico8_lq_keys_of(rows[0])` — le chemin de `src/` — au lieu d'une vue ; les tests de `pico8_lq_table_view` (l. 719, 737) restent.
- **Constantes** : aucune constante `*_HASH` ne change, ni de valeur ni de nom (`NO_OVERRIDES_HASH` de `test_pico8_jb_ol` compris, hors motif : casse). Toute constante qui bouge est un arrêt.

### Le test ponctuel `tests/test_pico8_map_tables.c` (G)

- `test_dr_view_matches_the_decoded_table` compare désormais `pico8_dr_table`, entrée par entrée (par `pico8_jb_row_view`, outil de comparaison du test), à `decode(mem, 9807)` : même intention, sans le pont. Il construit son état, ne rejoue rien, ne nomme aucun motif de `spec`, d'`overrides` ni de `type-tag` ; ses 2 lignes `stash` ne changent pas.

### Les tests du §6 (G)

- **`layout_nil_item_draws_nothing`** : `render_one_item` pose l'item dans un `pico8_lq_keys_t` local (`items` = l'item, `count` = 1) et appelle `pico8_lq_new` ; l'item reste un `pico8_value_t`, élément de `lq.items`. Assertions inchangées (nil : rien dessiné ; fixe 0 : rien ; contrôle positif : le sprite). **Preuve rouge** : clause REF NULL retirée de `pico8_layout_truthy` → test rouge ; restaurée → vert. Consignée au commit du chunk 2.
- **`gh_row_lq`** : commentaire l. 1-2 réécrit (la liste de `lq` d'une rangée de `jo:gh` est lue par `js:pq` comme ses positionnels ; le reste du commentaire, historique du bug, reste vrai) ; assertions inchangées. **Preuve rouge** : la branche liste de `pico8_js_lq` passe la liste au pont `lq` par une conversion de pointeur (le bug d'origine) → test rouge (ASan ou assertions de champs) ; restaurée → vert.
- **`ga_export_panel_by`** : intouché.

### Preuve des tests (§7, gcov + mutations)

- **gcov** (copie jetable du scratchpad, `--coverage`), par chunk : chaque ligne ajoutée ou modifiée des constructeurs, des ponts, de `pico8_lq_keys_of`, de `pico8_js_lq`, du pas de `jb:pq`, de `pico8_de_my` et des appelants de `src/` est exécutée par un TU ; celles atteintes seulement par les e2e (`eu`, `hy:lj`) sont justifiées au commit.
- **Mutations**, chacune doit rougir un test, puis être annulée :
  - 1a — **M1** : `eu` n'appelle plus `pico8_bg_new()` → `test_pico8_bg` ou `make e2e` rouge.
  - 1b — **M2** : `iv` ignore `keys->ra` → `test_pico8_iv` rouge ; **M3** : `iv` ignore `keys->at2` → `test_pico8_iv` ou `test_pico8_hy` rouge ; **M4** : `jc` ignore `keys->qu` → `test_pico8_jc` rouge ; **M5** : l'`assert` de classe de `jc.qu` retiré → `build_off_row` rouge ; **M6** : `pico8_de_my()` rend NULL → `test_pico8_fq` rouge.
  - 2 — **M7** : `lq` ignore `has_c` → `test_pico8_lq` rouge ; **M8** : `pico8_lq_keys_of` s'arrête au nombre de `fs` → `test_pico8_lq` rouge ; **M9** : le constructeur de `js` ne remplace plus un `h_` présent à nil par `true` (TROU-23) → `test_pico8_js` rouge ; **M10** : `jb` saute le `c` d'une ligne ou inverse deux lignes → `test_pico8_jb_ol` rouge ; **M11** : le pont `mh` ignore `rh` → `test_pico8_mh` rouge ; **M12** : `eu` passe `&pico8_dr_table[i + 1]` → `test_pico8_ie`/`test_pico8_entity_kind` ou `make e2e` rouge ; **M13/M14** : les deux preuves rouges du §6.
- Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée.

### Pas de nouvel e2e (G)

Les lignes changées sont atteintes par les 26 e2e et par les TU (H15) ; un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 35 minutes pour l'étape (≈ 10 par chunk, plus les preuves du §6).**

| Cible (par chunk) | Durée attendue |
|---|---|
| `make test` (×2 : constructeurs et appelants ; TU) | quelques secondes chacun, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H14) |
| `make class-coverage-net` | < 2 min (H14) |
| gcov + mutations (scratchpad) | < 4 min |
| `make data-guard`, `make data-residue`, recette d'occurrences, classification | < 1 min |
| preuves rouges du §6 (chunk 2) | < 2 min |

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + chunk + « constructeurs » / « appelants » / « TU »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre de chaque commit : `timeout 5m make test` ; `timeout 5m make e2e` : **26 « conforme »** ; `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ; `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ; `make data-guard` : code 0 ; `make data-residue` + recette : `overrides` aux jalons 299/313/45, 174/185/31, 118/129/25, `spec` 0, `stash` 28/4, `ob` et `valeur` strictement plus bas au dernier chunk, `par-nom` départ − 13 (± 3 expliqué) ; classification et anti-tassement passés.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit liste les chemins de son chunk (« Empreinte remesurée », écart ≤ 3 expliqué), rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements, `Makefile` et `tools/` inchangés, `ga_export_panel_by` intact.
2. **Types** : les douze constructeurs prennent une struct de clés, `const pico8_jb_row_t *` ou rien ; aucune recherche par nom hors des quatre ponts ; `grep` de la « Sortie de l'étape » vides.
3. **Ponts** : exactement quatre, paramètre `overrides`, 21 lignes / 25 occurrences en tout, aucune vue ob construite, listes temporaires libérées ; chaque appelant d'un pont reçoit sa table d'ailleurs (aucun appelant qui l'écrit lui-même).
4. **Renommage** : les 211 lignes retirées classées (a)/(b), aucune (c) ; les 21 lignes des ponts classées (e) ; les 97 sites de D.6c identiques octet pour octet.
5. **Mémoire** : aucune `pico8_ob_table_new` ni `pico8_jb_row_view` ajoutée dans `src/` ; les tables de clés des appelants littéraux, l'emballage de `jb:pq`, la vue et la conversion de `js:pq` et le pont `dr` ont disparu.
6. **Anti-tassement** : comptes avant/après chaque chunk, en lignes ET en occurrences, par catégorie et par fichier, recalculés avec la recette ; chaque couple Δlignes > Δoccurrences justifié ; aucune jonction de lignes, aucun alias, aucun registre de TU, aucun déplacement hors du compteur, aucun commentaire sans rapport supprimé ; lignes gagnant une catégorie limitées à la liste attendue.
7. **TU** : plus aucune table de clés pour un constructeur typé ; tests de mort retirés listés avec leur raison ; ceux gardés rougissent (M5, tests des ponts) ; aucune constante `*_HASH` changée.
8. **§6** : les deux preuves rouges consignées ; `layout_nil_item_draws_nothing` passe par `pico8_lq_keys_t`, son item reste un `pico8_value_t`.
9. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun récit de chantier ; aucun renvoi à D.6b ni au plan dans `src/` ou `tests/`.
10. **Docs** : glossaire à jour ; TROU-23 à jour ; note « Découvert à D.6b » en fin de ligne D.6b, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le commit gradé de D.6a (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas `overrides` 329/55 et `spec` 0 (H-D6a-1 démentie : relire le PRD avant tout code).
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence, à un des trois commits.

**Périmètre**

- `FAILED:` si un des douze constructeurs prend encore une `pico8_ob_table_t`, si une clé de construction est lue par nom hors des quatre ponts, ou si une ligne des `grep` de la « Sortie de l'étape » subsiste.
- `FAILED:` si un pont n'a pas le paramètre `overrides`, si un cinquième pont apparaît, ou si un appelant qui écrit lui-même ses clés passe par un pont.
- `FAILED:` si une des 97 lignes `overrides` de D.6c change, si un producteur de rangées est converti au-delà de son appel de constructeur, si `g_de`, `pico8_de()`, une vue `ix`/`ha`/`bo`/`bz`, `pico8_nb()`, `pico8_jb_row_view` ou `jo_kw_view` change, ou si le diff touche un TROU autrement que par le marqueur qui part avec la conversion liste → table.
- `FAILED:` si le diff ajoute une `pico8_ob_table_new` ou un appel à `pico8_jb_row_view` dans `src/`, ou toute table ou vue construite à chaque appel ou à chaque frame.
- `FAILED:` si le test ponctuel nomme un motif de `spec`, d'`overrides` ou de `type-tag`, ou porte plus de 2 lignes `stash`.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` ou `tools/data_residue.sh` change.
- `FAILED:` si `overrides` ne rend pas 118 lignes / 129 occurrences / 25 fichiers (et les jalons de 1a/1b) sans explication ligne par ligne, ou descend sous 118 par un renommage ; si `stash` n'est pas 28/4, `spec` 0 ; si `par-nom` s'écarte de départ − 13 de plus de 3 ou sans explication ; si `ob` ou `valeur` ne baisse pas strictement.
- `FAILED:` **si une ligne `overrides` disparaît par renommage sans changement de type** (classe (c)), si une ligne d'un pont porte un autre nom que `overrides`, ou si la classification (a)/(b)/(e) manque au commit.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés dans une ligne comptée, construction comptée déplacée sur une autre ligne comptée, alias (macro, `typedef`, enveloppe) d'un identifiant compté, code déplacé hors de `src`/`tests` ou hors `.c`/`.h`, commentaire sans rapport supprimé.
- `FAILED:` si un TU construit les clés d'un constructeur typé par un registre, une table ou une aide par nom nouvelle.
- `FAILED:` si un commit ne donne pas, par catégorie, les comptes avant/après **en lignes et en occurrences**, si un couple Δlignes > Δoccurrences n'est pas justifié, ou si une ligne gagne une catégorie hors de la liste attendue sans être listée.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` (ou `identity[]`) change.
- `FAILED:` si une preuve rouge du §6 manque (`layout_nil_item_draws_nothing`, `gh_row_lq`), ou si l'item de `layout_nil_item_draws_nothing` cesse d'être un `pico8_value_t` de `lq.items`.
- `FAILED:` si un test de mort est retiré sans être listé au commit avec sa raison, ou si un test de mort d'un pont ou de la classe de `jc.qu` manque.
- `FAILED:` si le rapport gcov ou une mutation manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si un message de commit ne porte pas ses comptes avant et après.
- `FAILED:` s'il manque la note « Découvert à D.6b », ou si elle omet le compte `overrides` réel et l'explication de l'écart avec 117, les ponts et leur étape de fermeture, les ponts de données reportés, ou la conséquence pour le « = 0 » de D.6c.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les clés de D.6c** : `ie` (son constructeur et `eu`), `hy` (son constructeur), `hp`, `g_`, `fv`, `trade` (et `planet_trade.c`), `pj`.
- **Les producteurs de rangées ob** et ce qui ferme les ponts : `bs` de `pz` (`planet_display`, `menu` `cr`/`ha`, `help`, préfixe `ep`), rangées `ix` de `eu`, `spec1`/`spec2` (et leur parse par appel), `st`, `l`, `iw`/`g_pj_ea`, `g_de` : D.7a/D.7b.
- **Les vues de données** `pico8_ix()`/`pico8_ha()`, `g_bo`, `pico8_bz()`, `pico8_nb()`, les vues à l'appel de `hp.lq`, `px`/`dz[mi]`, `em` et `jo_kw_view` : D.7a/D.7b.
- **Les items de `lq`, les sources `lq`/`h_`/`jb` de `js`, les items de `mh`** (éléments `pico8_value_t` hétérogènes) : D.9, point 3.
- **`pico8_lq_table_view`** (sans appelant de `src/` après D.6b) : retirée avec `layout` (D.7a) ou les TU (D.7c).
- **Les écrivains et lecteurs par nom** (`set_field`, aides `*_take_*` des ponts, `pico8_entity_get_field`) : `par-nom`, D.9.
- Les archétypes, la phase 2 ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **forme** : `(void)` pour `bg`, `nf`, `qb`, `ui`, `mz` ; `pico8_iv_keys_t`, `pico8_jc_keys_t`, `pico8_fq_keys_t` sans drapeau (défauts NULL/0) ; `pico8_lq_keys_t` (items `pico8_value_t`, `c`/`bg`/`score`/`ra` sous drapeaux) ; `pico8_js_keys_t` (membres typés, `lq`/`h_`/`jb` sources de rangée `pico8_value_t` avec présence TROU-23) ; `pico8_mh_keys_t` ; `jb` sur `const pico8_jb_row_t *` ; `pico8_lq_keys_of` ; `pico8_de_my`.
  - **ponts** : quatre, paramètre `overrides` gardé parce que son type ne change pas, 21 lignes comptées, fermés par D.7a/D.7b ; aucun appelant qui écrit ses clés n'y passe.
  - **ponts de données** : `pico8_dr()` fermé ; `ix`/`ha`/`bo`/`bz` et les vues à l'appel de `hp.lq`/`kw` reportés (producteurs ob, item imbriqué que seule une vue sait donner à `lq:mn`).
  - **compte** : 118 / 129 (97 de D.6c + 21 des ponts), pas 117 (total de fichiers pris pour exact) ; le « = 0 » de D.6c devient « = 21 ».
  - **§6** : `layout_nil_item_draws_nothing` par `pico8_lq_keys_t`, `gh_row_lq` (commentaire reporté par D.6a), deux preuves rouges ; `ga_export_panel_by` intouché.
  - **tests** : clés en struct locale à leur site ; tests de mort impossibles retirés et listés, ceux des ponts et de `jc.qu` gardés ; test ponctuel sur `pico8_dr_table` ; quatorze mutations ; pas d'e2e ; `class-coverage-net` et `sanitize-net` à chaque chunk.
  - **découpage** : trois commits (58 chemins).
  - **D.6a** : douze hypothèses H-D6a, sept sections à relire contre sa note.
- **Note du plan (G)**, à ajouter en fin de ligne D.6b (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.6b : trois chunks (classes sans clé ; `iv`/`jc`/`fq` ; `lq`/`jb`/`js`/`mh` avec les ponts, `dr` et le §6 — 58 chemins : appelants à NULL, `economy.c`, `score_panel_display`, `research_display`, `data_init` et les TU qui construisent dans `b` hors du majorant). `bg`, `nf`, `qb`, `ui`, `mz` sans paramètre ; clés typées `pico8_iv_keys_t`, `pico8_jc_keys_t`, `pico8_fq_keys_t`, `pico8_lq_keys_t` (items `pico8_value_t`), `pico8_js_keys_t` (`lq`/`h_`/`jb` restent des sources de rangée `pico8_value_t`, présence TROU-23), `pico8_mh_keys_t` ; `jb` prend `const pico8_jb_row_t *`. `js:pq` copie une rangée par `pico8_lq_keys_of` et une liste par ses éléments (plus de vue ni de conversion) ; `jb:pq` n'emballe plus ses lignes chaîne ; `fq(de)` lit `pico8_de_my()`. Pont `pico8_dr()` fermé (`pico8_jb_new(&pico8_dr_table[i])`). Ponts nouveaux, paramètre `overrides` gardé (type inchangé, comptés, 21 lignes) : `pico8_lq_new_table`, `pico8_jb_new_table` (`layout.c`), `pico8_js_new_table` (`ui_button.c`), `pico8_mh_new_table` (`msgbox.c`) — fermés quand les rangées de `pz` et les sources `lq`/`h_`/`jb`/`mh` sont typées (`planet_display`, `menu`, `score_panel_display`, `research_display` : D.7a ; `help`, `msgbox` : D.7b ; le « 0 dans `layout` » de D.7a exige les deux premiers). Reportés, avec eux : `pico8_ix()`/`pico8_ha()`, `g_bo`, `pico8_bz()` (vues par init ; `ix` porte un item imbriqué que seule une vue sait donner à `lq:mn`), les vues à l'appel de `hp.lq` et de `kw`. `pico8_lq_table_view` sans appelant de `src/`. `overrides` après D.6b : N (97 lignes de D.6c + 21 des ponts ; le « = 117 » de D.1 était un total de fichiers, qui comptait dans ceux de D.6c 23 lignes de D.6b — `iv` de `economy.c`, `jc` de `test_pico8_hp`/`_hy`/`_trade`) ; le « = 0 » de D.6c devient « = 21 », et `overrides` entre dans `DATA_RESIDUE_GUARD` à l'étape qui ferme le dernier pont. `par-nom` : P.

  N est le compte `overrides` réel, P le compte `par-nom` réel.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget de tests), anti-tassement, classification et ponts compris.
- **Le message de chaque commit** porte : le périmètre du chunk ; la forme des clés par classe et ses rejets ; la classification (a)/(b)/(e) des lignes `overrides` touchées ; les comptes avant et après **en lignes et en occurrences** par catégorie et par fichier, et les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie ; les tests de mort retirés, un par un, avec leur raison ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses H et H-D6a qui le concernent ; pour le chunk 2, les deux preuves rouges du §6.
- **Surprises pour le pilote** :
  - **La coïncidence 117 / 118 est trompeuse** : le « = 117 » était un total de fichiers (comme le « = 315 » de D.6a) ; le compte réel des sites de D.6c est 97, et les 21 lignes restantes sont les ponts, qu'aucune ligne du plan ne prévoyait.
  - **Le « = 0 » de D.6c ne peut pas tenir** : les ponts vivent jusqu'à ce que les producteurs de rangées soient typés (D.7a/D.7b) ; `overrides` n'entrera dans `DATA_RESIDUE_GUARD` qu'à ce moment-là.
  - **Les notes de D.3/D.4a/D.4b envoyaient ici la fermeture de `ix`/`ha`/`bo`/`bz` et des vues de `hp.lq`/`kw`** : elles n'ont pas vu que les rangées passent par `pz` (ob jusqu'à D.7b) et que les items imbriqués des rangées (`o(o(15,7))`) sont des tables que `lq:mn` lit par indice (D.9). Seul `pico8_dr()` ferme sans vue.
  - **L'empreinte de la ligne (30) sous-estime du double** : les classes sans clé ont des appelants à NULL partout (`bootstrap.c`, douze TU), et `economy.c`/`score_panel_display.c`/`research_display.c` appellent des constructeurs de D.6b.
  - **D.7a hérite d'un chantier plus lourd** : fermer les ponts `lq`/`jb` (« 0 dans `layout` ») demande de typer les producteurs de rangées de `planet_display`, `menu`, `score_panel_display` et `research_display`, et les rangées `bs` de `pz` (fichier de D.7b) ; son PRD devra trancher l'ordre ou la fusion avec D.7b.
  - **`lq.fs`/`lq.w` n'étaient des clés que par la vue de copie** : `lq:pq` les recalcule ; le type les retire sans perte.
