---
title: "Plan 3, étape D.6a : les clés de construction de l'arbre `jo` (`pico8_jo_new` … `pico8_mx_new`, `new_entity` des sortes et leurs appelants) passent d'une table ob à une struct typée locale (`pico8_jo_keys_t`, `pico8_ga_keys_t`) ; le paramètre est renommé parce que son type change ; la table que `pm:pq` parse à chaque apparition devient une donnée générée — un commit, aucun pixel changé"
description: "Les dix constructeurs de l'arbre `jo` et le `new_entity` des sortes reçoivent encore leurs clés de construction dans une `pico8_ob_table_t` que chaque appelant alloue, remplit par nom, passe, puis libère (galaxie, `jo:hu`, `pm:pq`, `ie:ldown`, TU), et que chaque constructeur relit par `strcmp` sur ses entrées. D.6a remplace cette table par une struct locale typée, fait lire ses champs par les constructeurs, supprime les boucles, les aides d'écriture devenues mortes et la parse de `pm:pq`, et mesure la baisse d'`overrides` en lignes ET en occurrences, sans tassement ni renommage à type constant."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.6a — `overrides`, constructeurs de l'arbre `jo` (`pico8_jo_new` … `pico8_mx_new`), `new_entity` de `pico8_entity_kind_t` (`pico8_struct.h`) et ses appelants (`galaxy.c`, `jo_kv`…) : la table de clés de construction devient une struct locale (troisième puce), paramètre renommé. Empreinte 11 (8, 3). Compte : `overrides` = 315. §6 : `gh_row_lq`, commentaire de la l. 2 seulement ; aucune lecture ne change, pas de preuve rouge. `class-coverage-net` : oui.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » des étapes livrées, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b) :

> Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.

> Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.

> Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).

> Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).

> Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp` : `lq_own`/`lq_inner`) ; `hp.mr`/`gp` en `pico8_str_seq_t`, `ho_apply` les prend typés ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages `po_own`/`fu_own` dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie ; une clé `qe` chaîne est absente) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus, `pico8_jc_hg_add`/`_has`) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`, qui partage son calcul avec `pico8_gi`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : 1882 (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de −100 (1882 − 1982).

> Découvert à D.5a : deux chunks (interfaces, puis `ke` ; ≈ 40 chemins, `pico8_game_data.*`, `pico8_bootstrap.c` et les TU qui plient l'identité de `me` hors du majorant). Défauts générés `pico8_X_start` (type `pico8_X_start_t`, `pico8_game_data.h`), un par classe à clés (`ke`, `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`), clés propres de la spécification, lus par les constructeurs ; les tables gardent l'objet de D.4a/D.4b ; `mz.jj` s'initialise de `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` : liste statique de `ui.c` (`pico8_ui_le_shared`) ; `qb.v`/`p`/`ra`, `iv.v`, `ui.hw` pointent la donnée générée (membres `const`) ; `nf.ra` pointe le statique non `const` `g_nf_ra`, affecté depuis `pico8_nf_start.ra` à `pico8_nf_init` (repli : `test_pico8_entity_kind` et `test_pico8_ie` écrivent `je.ra` en place) ; `nf.l`/`nf.r` restent `g_nf_l`/`g_nf_r` (D.7b). Le cycle `_init`/`_shutdown`/`_ready` des treize modules reste, sur un drapeau. `pico8_ke_start_field` (compté par `par-nom`) remplace `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_g__default`, `pico8_ie_default` (retirés à D.5b) et `pico8_jo_default` (D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`) : D.5c le supprime. `pico8_rk_default` (variante de dessin, comptée par `spec`) est à D.5b. `spec` après D.5a : 634 (au lieu de ≤ 779) ; les majorants `spec` suivants se décalent de −145 (634 − 779). `par-nom` : 1090.

> Découvert à D.5b : deux chunks (exclusion de `rk`, puis les sept classes ; 29 chemins, `pico8_game_data.*` et trois TU hors du majorant). `pico8_rk_default` (variante de dessin à argument omis) entre dans `SPEC_EXCLUSION`, preuve : ses quatre lignes ne portent aucune autre catégorie. Défauts générés `pico8_X_start` de `g_`, `fv`, `hp`, `ie`, `pj`, lus par les constructeurs ; `hy`, `trade` sans objet. `ie` : un seul défaut pour la chaîne et les objets prêtés, `kq`/`ra`/`eh` pointent la donnée générée, plus libérés à l'arrêt. `fv.hi`/`gq`/`kq`, `g_.ra`, `ie.kq`/`ra`/`eh`, `pj.cr.hi`/`gq` : membres `const` vers la donnée générée, aucun repli. `pj.cr` : `g_pj_cr` reste (son `ep` est muté), valeurs de `pico8_pj_cr_start`. `pj.ou`/`pj.ea` : rangées générées `pico8_pj_ou_row`/`pico8_pj_ea_row`, d'où `pico8_score_panel_init` construit une fois par init les tables d'état `g_pj_ou`/`g_pj_ea` (`pico8_jb_row_view`), jamais libérées comme celles de la spécification ; leurs lecteurs (`hb`, `hk`, `kj`, `pico8_mh_new`) restent des tables ob (D.6b, D.7a). `test_pico8_spec_defaults` n'a plus de registre ni de constante. `pico8_ke_start_field` n'a plus que `pico8_jo_default` et le cas `M_KE`. `spec` après D.5b : 430 (au lieu de ≤ 402 après le décalage de D.5a : 28 lignes des sites de D.5b, retirées par D.4a/D.4b, étaient comptées à la fois dans le majorant 547 et dans ce décalage ; majorant corrigé 430) ; le « = 0 » de D.5c est inchangé. `par-nom` : 1085.

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

Lecture de ces textes pour D.6a :

- **« La table de clés de construction devient une struct locale (troisième puce) »** : les clés de construction sont une table ad hoc construite en cours de partie (`pico8_ob_table_new()` à chaque planète de la galaxie, à chaque `jo:hu`, à chaque `pm:pq`, à chaque `ie:ldown`). La troisième puce en fait une **struct locale** à l'appelant, passée par pointeur `const`, lue par le constructeur et jamais gardée.
- **« Paramètre renommé »** et la règle commune (« le paramètre `overrides` typé est renommé ») : le renommage n'est légitime **que parce que le type change** (`const pico8_ob_table_t *` → pointeur vers une struct typée). Le motif `overrides` est `\boverrides\b` : un renommage à type constant ferait tomber le compte sans rien convertir. C'est le **tassement par renommage**, interdit et traduit en `FAILED:` (section « Anti-tassement »).
- **Le « = 315 » de la ligne, corrigé en 316 par la note de D.4a, n'est pas le bon compte : il est 329** (lignes ; 348 occurrences), mesuré ici. Le « = 315 » de D.1 était le total des lignes `overrides` des onze fichiers de l'empreinte, pris comme exact (449 − 134). Or ces onze fichiers portent 30 lignes d'autres groupes (`pico8_planet.c` : `pico8_iv_new` 6 et `pico8_jc_new` 5 ; `pico8_planet_family_trade.c` : `pico8_jc_new` 5 ; `test_pico8_mx` : `pico8_jc_new` 4 ; `test_pico8_planet_gh_row_lq` l. 2, qui parle des clés de `js` : 1 — tout cela D.6b ; `test_pico8_entity_kind` : `pico8_ie_new` 9 — D.6c), et 16 lignes de D.6a sont hors de ces fichiers (`pico8_research_icon.c`, `ie:ldown`, appelant de `new_entity` : 6 ; `test_pico8_mz` et `test_pico8_bg`, qui construisent un `nr` : 5 + 5). La note de D.4a ajoute une ligne (la clé `k_` que `pm:pq` pose pour son `ga`) et en déduit 316, alors que cette ligne est dans le périmètre de D.6a (constructeur de `ga`) et tombe avec lui. Compte corrigé : 450 − 121 = **329** (section « Effet sur `make data-residue` »). La note « Découvert à D.6a » le dit ; les comptes de D.6b (« = 117 ») sont à reprendre par son PRD, le « = 0 » de D.6c n'en dépend pas.
- **§6, `gh_row_lq` l. 2** : la ligne dit que « `js:pq()` passes it on as class overrides, i.e. as a `pico8_ob_table_t*` ». Elle parle des clés de construction de `js`/`lq` (D.6b), qui restent une table ob après D.6a. Remplacer le mot ici ferait baisser `overrides` d'une ligne **sans aucun changement de type** : c'est exactement le renommage interdit. **Décision (G) : D.6a ne touche pas `test_pico8_planet_gh_row_lq.c`** ; la réécriture de ce commentaire passe à D.6b, qui change le type des clés de `js`/`lq` et rend la phrase historique. La note le dit. Aucune lecture ne change, pas de preuve rouge, comme la ligne le prévoit.
- **`make class-coverage-net` et `make sanitize-net`** : oui aux deux. Le diff touche dix constructeurs, et il supprime une allocation et une libération de table ob à chaque construction (galaxie, `jo:hu`, `pm:pq`, `ie:ldown`), plus la parse de `pm:pq` : la durée de vie des clés change (tas → pile).

### D.5c n'est pas livrée à l'écriture de ce PRD

D.5c (`spec` de l'arbre `jo` → défauts générés, `pico8_ke_start_field` supprimé, `spec` = 0 dans `DATA_RESIDUE_GUARD`) est écrite en parallèle et touche les mêmes fichiers (`pico8_planet*.c/.h`, constructeurs de l'arbre `jo`, TU de l'arbre). Ce PRD **suppose que D.5c atterrit comme sa ligne le dit**. Chaque supposition sur son résultat (forme des fichiers, corps des constructeurs, comptes) est une hypothèse **H-D5c-n** (section « Hypothèses sur D.5c »), et la section « À revérifier après D.5c » liste ce que `plan-pilot` relit contre la note « Découvert à D.5c » avant de lancer D.6a. Aucun fichier non versionné n'a été lu.

### État de départ

**Mesuré sur `e52b1c2`** (branche `claude/plan-pilot-3-7zdft0`, D.5b livrée et gradée) ; **l'arbre de départ de D.6a est le commit gradé de D.5c**, inconnu à l'écriture. Décisions de l'utilisateur qui engagent D.6a : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**. Consigne de pilotage, liante : **une étape précédente a été notée FAILED pour avoir fait baisser un compte par tassement de lignes** ; le compteur compte des lignes, et toute mise en forme, tout renommage à type constant, tout alias ou tout déplacement qui baisse un compte sans retirer la construction est interdit.

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `make -s data-residue`, `sh tools/data_residue.sh --detail`, un comptage d'occurrences par `grep -o` sur les motifs figés et des `git show 3816427:<fichier> | grep -c` ont tourné. Le reste vient de `grep` et de la lecture des sources de `e52b1c2` et de `pico8/lua.lua`. Tout fait d'exécution est une hypothèse (H1 à H14, H-D5c-1 à H-D5c-11).

### Inventaire (mesuré sur `e52b1c2`)

**`make data-residue`** (lignes / fichiers) et **occurrences** des mêmes motifs (recette de la section « Anti-tassement ») :

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 5831 | 133 | 6741 |
| `ob` | 1922 | 123 | 2152 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 423 | 70 | 686 |
| `ref-audit` | 69 | 17 | 70 |
| `par-nom` | 1085 | 80 | 1140 |
| `dispatch` | 197 | 59 | 199 |
| `overrides` | 450 | 62 | 479 |
| `spec` | 430 | 19 | 470 |
| **résidu D** | **8817 lignes** | **141 fichiers** | — |

Après D.5c (H-D5c-1, H-D5c-2) : `spec` 0, `overrides` inchangé (450/479, 62 fichiers), les autres catégories remesurées au départ.

**Ce que dit le cart** (`pico8/lua.lua`) : le constructeur de classe (`kv:jr`, l. 86-87) fait `ob=setmetatable(mw(ob),rj.ns)` : les clés passées sont **copiées** dans l'instance, une clé nil n'existe pas (table Lua), une clé absente lit la valeur de classe. Appelants : `bw` (l. 749) `jo({id="p"..id,p=p,ra=p*128})` ; `jo:hu` (l. 454) `kv[hp.cw]({p=self.p,ra=self.ra,id=self.id})` ; `pm:pq` (l. 1030-1034) `ga(ob([[qo=0.97,ro=-0.1255,qg=0.1255,q_=0.97,k_=o(…),iu=-0.009,py=9,ms=1.5]],{ra=self.ra+self.ng}))` ; `ie:ldown` (l. 934) `self.ke({id="s"..ie.id})`.

**Clés de construction par classe** (assertions « the only construction keys are … » des constructeurs, recoupées avec les appelants par `grep`) :

| Classe | Clés acceptées | Appelants de `src/` (clés passées) | Appelants TU non-NULL |
|---|---|---|---|
| `jo` | `id`, `p`, `ra` | `bw` (`galaxy.c` : `id`, `p`, `ra`) | `jo`, `ga`, `ui`, `g_`, `jc`, `hp` |
| `qh`, `nl`, `os`, `hj` | `id` | `ie:ldown` par `new_entity` (`id`) | `qh` (`new_entity`, `id`), mort : `qh`/`nl`/`os`/`hj` (`p` non-NULL, clé positionnelle) |
| `pm` | `p`, `ra`, `id` | `jo:hu` par `new_entity` (`kv["pm"]`) | `pm`, `hj`, `ga`, `ui` ; mort : `pm` (`hp`, positionnelle) |
| `ga` | `p`, `ra`, `id` (`jo:hu`) ; `ra`, `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms` (`pm:pq`) | `jo:hu` (`kv["ga"]`), `pm:pq` (`pico8_ga_new` direct) | `ga` (`keyed` : les sept fixes et `k_` ; `nil_k` : `k_` nil), `ui` ; mort : `ga` (`fa`, positionnelle, hors rangée) |
| `nr` | `id`, `p`, `ra` | `ie:ldown` (`id`) | `nr`, `mz`, `bg`, `ui` ; mort : `nr` (`hp`, positionnelle) |
| `fr`, `mx` | `id` | `ie:ldown` (`id`) | `ui` (`mx`, `id`) ; mort : `fr` (`p` nil, positionnelle), `mx` (`hp`, positionnelle) |

**Faits qui engagent la forme** :

- **`id`, `p`, `ra` : absent ≡ nil.** `pico8_jo_struct_init` est le seul à écrire `id`/`p`/`ra` (NULL) ; aucune des dix classes ne leur donne de défaut. Les aides d'écriture (`jo_take_vec`, `family_take_string`…) mettent NULL pour une valeur nil. Une clé présente à nil et une clé absente donnent donc la même instance, comme dans le cart.
- **Les sept fixes de `ga` ont un défaut de classe** (`qo`, `ro`, `qg`, `q_`, `iu`, `py`, `ms`, sans drapeau `has_`) : l'absence doit laisser le défaut. Tous les appelants réels les passent **ensemble, avec `k_`** (`pm:pq` ; le cas `keyed` du TU) ; aucun appelant n'en passe une partie.
- **`k_` de `ga` : nil explicite ≠ absent dans le port.** `family_take_seq` met NULL pour nil (le TU `nil_k` l'asserte : `k_` illisible), alors que l'absence laisse `&pico8_ga_k__seq`. Le cart ne peut pas passer `k_=nil` ; le chemin n'est atteint que par ce TU.
- **`new_entity` est générique sur les neuf sortes** (`g_qh_kind` … `g_mx_kind`, toutes de l'arbre `jo`) ; ses appelants : `jo:hu` (`p`, `ra`, `id` vers `pm`/`ga`), `ie:ldown` (`id` vers n'importe quelle sorte de `gx[]`), `test_pico8_qh` l. 686 (`id` vers les descendants), `test_pico8_entity_kind` l. 81 (NULL).
- **`pm:pq` parse à chaque apparition une chaîne constante** (`pico8_ob("qo=0.97,…,ms=1.5", defaults)`, `defaults` = `{ra=…}` alloué puis libéré) : une table constante parsée en cours de partie. `k_` y est posé à part (`pico8_pm_ga_k__seq`, D.4a). Le test ponctuel a déjà la chaîne du cart (`PM_PQ_KEYS_SRC`, avec `k_`).
- **Aides d'écriture qui meurent avec les boucles** (seuls usages dans les constructeurs) : `jo_take_vec` (`planet.c`), `family_take_string`, `family_take_seq` (`planet_family.c`), `family_trade_take_string`, `family_trade_take_vec` (`planet_family_trade.c`). Restent, pour les écrivains par nom : `jo_take_string` (`ep`), `family_take_vec` (`p`/`ra` des `set_field`), `family_take_fixed` (`nl.qw`).
- **Les TU construisent leurs tables dans une variable `b`**, que le motif ne voit pas : l'empreinte réelle (17 TU) dépasse de loin les 3 de la ligne.
- **Sites `overrides` qui restent après D.6a** (autres groupes, intouchés) : `pico8_planet.c` 11 (`pico8_iv_new` 6, `pico8_jc_new` 5 : D.6b), `pico8_planet_family_trade.c` 5 (`pico8_jc_new` : D.6b), `pico8_research_icon.c` 7 lignes / 8 occurrences (constructeur de `ie` : D.6c), `test_pico8_mz` 4 (`pico8_jc_new` : D.6b), `test_pico8_entity_kind` 9 (`pico8_ie_new` : D.6c), `test_pico8_mx` 4 (`pico8_jc_new` : D.6b), `test_pico8_planet_gh_row_lq` 1 (commentaire sur `js` : D.6b), et les 48 autres fichiers.

**`overrides` retiré par D.6a** (lignes / occurrences, `e52b1c2`) :

| Fichier | Retiré | Reste | Ce qui est compté |
|---|---|---|---|
| `pico8_planet.c` | 15/16 | 11/11 | `pico8_jo_struct_new` (signature, boucle, trois lectures), `pico8_jo_new`, `jo:hu` (déclaration, table, trois clés, appel `new_entity`, libération) |
| `pico8_planet.h` | 2/2 | 0 | contrat et signature de `pico8_jo_new` |
| `pico8_planet_family.c` | 43/49 | 0 | six constructeurs (`qh`, `nl`, `os`, `hj`, `pm`, `ga`), six `pico8_X_new`, `pm:pq` (parse, `k_`, appel, libération) |
| `pico8_planet_family.h` | 12/12 | 0 | six contrats et signatures |
| `pico8_planet_family_trade.c` | 18/21 | 5/5 | trois constructeurs (`nr`, `fr`, `mx`), trois `pico8_X_new` |
| `pico8_planet_family_trade.h` | 6/6 | 0 | trois contrats et signatures |
| `pico8_galaxy.c` | 8/8 | 0 | `bw` : déclaration, table, trois clés, commentaire, appel, libération |
| `pico8_struct.h` | 1/1 | 0 | `new_entity` |
| `pico8_research_icon.c` | 6/6 | 7/8 | `ie:ldown` : déclaration, commentaire, table, clé, appel `new_entity`, libération |
| `test_pico8_mz` | 5/5 | 4/4 | `nr` construit sur `p`/`ra` (l. 686-699) |
| `test_pico8_bg` | 5/5 | 0 | `nr` construit sur `p`/`ra` (l. 451-466) |
| **Total** | **121/131** | **329/348** | fichiers : 62 → 55 |

## Solution

**Un commit, sans aucun pixel changé** :

- **Trois types de clés, typés comme les membres qu'ils remplissent** :
  - `pico8_jo_keys_t` (`pico8_struct.h`, à côté de `pico8_entity_kind_t`, son seul consommateur générique) : `id`, `p`, `ra` — pointeurs, NULL = absent ;
  - `pico8_ga_pq_keys_t` (`pico8_game_data.h`, type d'une donnée générée) : les sept fixes et `k_` que `pm:pq` passe ;
  - `pico8_ga_keys_t` (`pico8_planet_family.h`) : un `pico8_jo_keys_t` et un pointeur `const pico8_ga_pq_keys_t *pq`, NULL = défauts de `ga`.
- **Les dix constructeurs prennent un pointeur `const` vers leur struct de clés** (`pico8_jo_keys_t` pour neuf, `pico8_ga_keys_t` pour `ga`), NULL = aucune clé, lu jamais gardé ; les champs non NULL sont posés directement dans les membres. **Les boucles sur les entrées, les `strcmp` et les cinq aides d'écriture devenues mortes disparaissent.** `pico8_X_new(NULL)` ne change pas.
- **`new_entity`** prend `const pico8_jo_keys_t *` ; l'entrée de la sorte `ga` est une adaptatrice qui enveloppe les clés de `jo` sans `pq`.
- **Appelants** : `bw`, `jo:hu`, `ie:ldown` construisent une struct locale ; `pm:pq` construit un `pico8_ga_keys_t` local dont `pq` pointe **la donnée générée `pico8_pm_ga_pq_keys`** (première puce : la chaîne constante parsée à chaque apparition devient `static const`, générée une fois) et dont `jo.ra` est le vecteur calculé. Plus aucune table ob n'est allouée pour construire une planète.
- **TU** : chaque table `b` de clés d'un constructeur de l'arbre devient une struct locale ; les tests de mort qu'un type rend impossibles à écrire (clé positionnelle, nom sans membre, valeur hors rangée) disparaissent ; ceux qui gardent un contrat vérifiable (« seulement `id` » pour `qh`, `nl`, `os`, `hj`, `fr`, `mx`) restent, sur `p` non NULL.
- **Test ponctuel étendu** : `pico8_pm_ga_pq_keys` égale la parse de `PM_PQ_KEYS_SRC`, clé par clé, dans les deux sens.

**Sortie de l'étape**, constatée par commande :

- `make test` vert ; `make e2e` : 26 conformes ; `make sanitize-net` : 26 propres ; `make class-coverage-net` : identique ; `make data-guard` : code 0 ;
- `make data-residue` : `overrides` = **329** lignes / **348** occurrences / 55 fichiers (exact ; au-delà : écart expliqué ou échec), `spec` 0, `stash` 28/4, `dispatch` 197, `par-nom` ≈ −31 (écart ≤ 3 expliqué), `ob` et `valeur` strictement plus bas ; comptes en lignes ET en occurrences, avant et après, par catégorie et par fichier ;
- ces `grep` ne rendent rien :

  ```sh
  grep -rnE 'pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_(struct_)?new\((const )?pico8_ob_table_t' src tests --include='*.[ch]'
  grep -rnE 'new_entity\)\((const )?pico8_ob_table_t|(new_entity|pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_new)\((b|overrides)\)' src tests --include='*.[ch]'
  grep -rnwE 'jo_take_vec|family_take_string|family_take_seq|family_trade_take_string|family_trade_take_vec' src
  grep -nE 'pico8_ob\(' src/pico8_planet_family.c src/pico8_galaxy.c
  ```
- `grep -rnw overrides src tests --include='*.[ch]'` ne rend que les sites de la dernière ligne de « Faits qui engagent la forme », aux mêmes comptes par fichier ;
- `grep -rn 'pico8_jb_row_view\|pico8_ob_table_new' src` : aucun site ajouté.

## User Stories

1. En tant que mainteneur du port, je veux que les clés de construction de l'arbre `jo` soient une struct typée locale à l'appelant, afin qu'aucune planète ne soit plus construite en allouant, remplissant par nom et libérant une table ob.
2. En tant que mainteneur, je veux que `id`, `p` et `ra` soient des pointeurs typés (`const char *`, `pico8_vec_t *`) dont NULL veut dire « absent », afin que le constructeur les pose sans `strcmp` ni aide d'écriture.
3. En tant que gardien du comportement, je veux qu'une clé `id`/`p`/`ra` nil et une clé absente donnent toujours la même instance, afin que la sémantique des tables Lua du cart (une clé nil n'existe pas) soit gardée.
4. En tant que gardien du comportement, je veux que les vecteurs et la chaîne passés soient adoptés par l'instance sans copie, comme aujourd'hui, afin que `bw`, `jo:hu` et `ie:ldown` gardent leurs aliasings (`p` partagé par `jo:hu` entre l'ancienne et la nouvelle planète, `ra` modifié en place par `na`).
5. En tant que mainteneur, je veux que `ga` reçoive les clés de `pm:pq` en un groupe typé (`pq`), NULL gardant les défauts de `ga`, afin de ne pas porter sept drapeaux de présence pour un groupe que tout appelant passe entier.
6. En tant que gardien du comportement, je veux qu'un `pq` dont `k_` est NULL donne toujours un `ga.k_` nil, afin que le cas `nil_k` de `test_pico8_ga` reste vrai.
7. En tant que mainteneur, je veux que la chaîne que `pm:pq` parse à chaque apparition devienne une donnée `static const` générée une fois (`pico8_pm_ga_pq_keys`), afin qu'aucune table constante ne soit plus parsée en cours de partie par ce site.
8. En tant que gardien du comportement, je veux que les bits de `qo` 0.97, `ro` −0.1255, `qg` 0.1255, `q_` 0.97, `iu` −0.009, `py` 9, `ms` 1.5 soient exactement ceux qu'`ob()` rendait, afin qu'aucune traînée de particules ne bouge d'un pixel.
9. En tant que mainteneur, je veux que `new_entity` prenne `const pico8_jo_keys_t *`, afin que `jo:hu` et `ie:ldown` construisent une sorte sans passer par une table.
10. En tant que gardien du comportement, je veux que la sorte `ga`, atteinte par `jo:hu` (`kv["ga"]`), construise un `ga` sans `pq`, afin que ses défauts de classe restent ceux d'aujourd'hui.
11. En tant qu'appelant de `pico8_X_new(NULL)` (TU, `pico8_cart.c`, `test_pico8_ga_export_panel_by`), je veux que NULL reste « aucune clé », afin que ces appels ne changent pas.
12. En tant que gardien du contrat, je veux que `qh`, `nl`, `os`, `hj`, `fr` et `mx` refusent toujours un `p` ou un `ra` non NULL par un `assert` à message, afin que le piège « une clé de plus n'a pas de membre » reste gardé là où le type commun ne l'interdit pas.
13. En tant que mainteneur, je veux que les boucles `for (k …) overrides->entries[k]`, leurs `strcmp` et les cinq aides d'écriture qui ne servaient qu'à elles disparaissent, afin que la baisse d'`overrides` et de `par-nom` corresponde à du code retiré.
14. En tant que gardien du compteur, je veux que le paramètre ne soit renommé que là où son type change, et que les sites `overrides` des autres groupes (`iv`, `jc`, `ie`, `js`) restent tels quels, afin que la baisse ne contienne aucun renommage à type constant.
15. En tant que gardien du compteur, je veux les comptes avant/après en lignes ET en occurrences, par catégorie et par fichier, afin qu'une baisse prouve une construction retirée.
16. En tant que gardien du compteur, je veux que le compte attendu (329) soit expliqué contre la ligne du plan (315, puis 316), afin que l'écart ne soit pris ni pour un tassement ni pour un oubli.
17. En tant que gardien des TU, je veux que chaque table `b` de clés d'un constructeur de l'arbre devienne une struct locale écrite à son site, sans registre ni aide qui reconstruise une table, afin que le test dise ce qu'il construit.
18. En tant que gardien des TU, je veux que les tests de mort devenus impossibles à écrire (clé positionnelle, nom sans membre, genre hors rangée) disparaissent avec la raison au commit, et que ceux qui gardent un contrat vérifiable restent, afin qu'aucune vérité ne se perde sans être dite.
19. En tant que gardien des TU, je veux qu'aucune constante `*_HASH` ne bouge, afin de prouver que les instances construites sont les mêmes.
20. En tant que vérificateur, je veux un test ponctuel qui compare `pico8_pm_ga_pq_keys` à la parse de la chaîne du cart, clé par clé, dans les deux sens, afin de prouver qu'aucune clé ni aucun bit n'a été perdu.
21. En tant que gardien des filets, je veux `make e2e` à 26 conformes, `make sanitize-net` à 26 propres et `make class-coverage-net` identique, afin de prouver qu'aucun pixel, accès mémoire ni instanciation n'a changé.
22. En tant que gardien de la mémoire, je veux que D.6a n'ajoute aucune table ob, aucune vue construite à l'appel ou à la frame, et retire les tables par construction, afin que le tas ne porte plus de clés.
23. En tant que gardien du §6, je veux que `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` ne changent pas, et que le commentaire de `gh_row_lq` passe à D.6b avec la raison, afin qu'aucun mot ne soit changé sans changement de type.
24. En tant que lecteur de `docs/glossaire-noms.md`, je veux les entrées `pico8_jo_keys_t`, `pico8_ga_keys_t`, `pico8_ga_pq_keys_t`/`pico8_pm_ga_pq_keys` et la mise à jour de `kv` (`new_entity`), afin de retrouver les clés du cart depuis leur nom.
25. En tant que `plan-pilot`, je veux une note « Découvert à D.6a » qui donne le compte réel, l'explication de l'écart, les types de clés et le report du commentaire de `gh_row_lq`, afin que D.6b et D.6c partent de faits.
26. En tant que `plan-pilot`, je veux la liste de ce qui dépend de D.5c et de ce qu'il faut relire contre sa note, afin de ne pas lancer D.6a sur une hypothèse démentie.
27. En tant que `plan-pilot`, je veux que les clés de `iv`, `jc`, `ie`, `trade`, `js`/`lq` (D.6b, D.6c), `g_kv`/`jo_kv`, le `field` des sortes (D.5c) et les écrivains par nom restent hors de ce commit, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Forme des clés, par classe (G)

Forme de décision (prose d'abord, types seulement pour la précision) :

```c
/* pico8_struct.h — NULL: the key is absent (a nil key does not exist). */
typedef struct {
    const char *id;
    pico8_vec_t *p;
    pico8_vec_t *ra;
} pico8_jo_keys_t;
/* new_entity: void *(*new_entity)(const pico8_jo_keys_t *keys); */

/* pico8_game_data.h — generated once from what ob() parses in pm:pq. */
typedef struct {
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
    pico8_fixed_t iu;
    pico8_fixed_t py;
    pico8_fixed_t ms;
    const pico8_int_seq_t *k_;
} pico8_ga_pq_keys_t;

/* pico8_planet_family.h — pq NULL: ga's own defaults stand. */
typedef struct {
    pico8_jo_keys_t jo;
    const pico8_ga_pq_keys_t *pq;
} pico8_ga_keys_t;
```

| Classe | Paramètre | Ce que le constructeur pose | Présence |
|---|---|---|---|
| `jo`, `pm`, `nr` | `const pico8_jo_keys_t *keys` | `id`, `p`, `ra` non NULL → membres | pointeur NULL = absent ≡ nil (défaut NULL) |
| `qh`, `nl`, `os`, `hj`, `fr`, `mx` | `const pico8_jo_keys_t *keys` | `id` ; `assert` porteur de message que `p` et `ra` sont NULL | idem |
| `ga` | `const pico8_ga_keys_t *keys` | `jo` comme ci-dessus ; si `pq` non NULL : les sept fixes et `k_` (NULL compris) → membres | `pq` NULL = défauts de `ga` ; `k_` NULL sous `pq` = nil |
| sortes (`new_entity`) | `const pico8_jo_keys_t *keys` | `pico8_X_struct_new` pour huit sortes ; pour `ga`, une adaptatrice statique `{*keys, NULL}` | NULL = aucune clé |

- **`pico8_X_new(NULL)` reste « aucune clé »** ; tous ses appelants (dont `test_pico8_ga_export_panel_by`, §6) ne changent pas. Contrat d'en-tête réécrit : la struct est lue, jamais gardée ; la chaîne et les vecteurs qu'elle pointe sont adoptés par l'instance, sans copie.
- **Pourquoi un type commun pour neuf classes** : `new_entity` est générique (neuf sortes, deux appelants qui ne connaissent pas la classe) ; un type par classe demanderait neuf adaptatrices. Les classes « seulement `id` » gardent leur contrat par un `assert` (piège : « une clé de plus n'a pas de membre — arrête, n'en ajoute pas »).
- **Pourquoi `pq` en groupe** : les sept fixes ont un défaut de classe, donc l'absence se distingue d'une valeur ; tous les appelants (`pm:pq`, le cas `keyed`) les passent ensemble avec `k_`. Un pointeur de groupe dit la présence une fois et porte, par `k_` NULL, le cas `nil_k` d'aujourd'hui.
- **Rejetés** :
  - **garder `pico8_ob_table_t` et renommer le paramètre** — renommage à type constant, c'est le tassement interdit ;
  - **des champs `pico8_value_t`** (ou un `pico8_value_t` par clé avec un drapeau) — la sémantique de la table sous un autre nom, `valeur` grandit ;
  - **un tableau de paires nom/valeur** ou toute recherche par nom (`strcmp`) — la table ob renommée ;
  - **un drapeau `has_` par fixe de `ga`** — sept drapeaux pour un groupe toujours entier ;
  - **un type par classe** — neuf adaptatrices pour `new_entity` sans gain ;
  - **deux paramètres pour `pico8_ga_new` (`keys`, `pq`)** — change les appels `pico8_ga_new(NULL)`, dont un test du §6 ;
  - **« NULL = défaut » pour `k_` sous `pq`** (fidèle au cart, où `k_=nil` n'existe pas) — change ce qu'asserte `nil_k` sans que le cart l'atteigne ; le chemin reste celui d'aujourd'hui.

### Constructeurs (G)

- Chaque `pico8_X_struct_new` garde son ordre : allocation, défauts (tels que D.5c les laisse, H-D5c-3), **clés**, enregistrement de couverture, chaîne `pq`/`f_`/`jo:pq`/`ke:pq`. Seul le bloc des clés change : `if (keys != NULL) { if (keys->id != NULL) … }` membre par membre, une instruction par ligne.
- **`ga`** : le bloc des clés pose `jo` puis, si `pq`, les sept fixes et `k_` ; la fonction publique `pico8_ga_new(const pico8_ga_keys_t *)` et l'adaptatrice de la sorte appellent le même corps (nom hors motif, choisi par l'implémenteur).
- **Aides mortes supprimées** : `jo_take_vec`, `family_take_string`, `family_take_seq`, `family_trade_take_string`, `family_trade_take_vec` ; `-Werror` l'exige. Les autres aides d'écriture restent (écrivains par nom, `par-nom`).
- **`assert` supprimés** : les « the only construction keys are … » des boucles disparaissent avec elles ; pour les six classes « seulement `id` », un `assert` à message sur `p`/`ra` NULL les remplace (un par constructeur).

### Appelants de `src/` (G)

- **`bw`** (`galaxy.c`) : `pico8_jo_keys_t keys` local, `.id`, `.p`, `.ra` posés ligne à ligne ; plus d'allocation ni de libération de table. Les commentaires qui disent le contrat (`p_heap`/`ra_heap`/`id_str` adoptés, jamais libérés ici) restent vrais et restent ; celui qui parle de la table libérée tombe avec elle. `/* TROU-7 */` reste au site.
- **`jo:hu`** (`planet.c`) : `pico8_jo_keys_t` local depuis `self->p`, `self->ra`, `self->id` (NULL restant NULL : la clé nil n'existe pas dans le cart), `target_kind->new_entity(&keys)`.
- **`pm:pq`** (`planet_family.c`) : `pico8_ga_keys_t` local, `.jo.ra` = le vecteur calculé (toujours alloué sur le tas, adopté par le `ga` — donnée d'entité, Plan 2 §8), `.pq = &pico8_pm_ga_pq_keys` ; la parse, la table `defaults`, la pose de `k_` et les deux libérations disparaissent.
- **`ie:ldown`** (`research_icon.c`) : `pico8_jo_keys_t` local avec `.id` ; le commentaire de contrat (« `id_str` n'est pas libéré : l'`id` de la planète garde ce pointeur ») reste, sans le mot `overrides`. Le constructeur de `ie` et ses clés (`ke`, `jb`) ne changent pas (D.6c).
- **Hors périmètre dans ces fichiers** : `pico8_iv_new`/`pico8_jc_new` de `planet.c` et `pico8_jc_new` de `planet_family_trade.c` (D.6b), `pico8_ie_new` et son constructeur (D.6c) : leurs lignes `overrides` ne changent pas d'un octet.

### Donnée générée de `pm:pq` et le générateur (G)

- **Objet** `const pico8_ga_pq_keys_t pico8_pm_ga_pq_keys` dans `pico8_game_data.c` (module pur : ni `pico8_ob.h`, ni `pico8_value.h`), `k_` = `&pico8_pm_ga_k__seq` (D.4a). Nom hors de tout motif figé (`_default`, `_spec`, `_defaults`, `overrides`).
- **Générateur jetable** (précédents D.4a à D.5b) : programme C de l'implémenteur, dans son scratchpad, jamais versionné, compilé contre `$(DATA_ENGINE_SRCS)` de l'arbre de départ **avant toute modification** ; `pico8_pi_init()`, puis `pico8_ob(s, NULL)` sur la chaîne **copiée octet pour octet** de `pm:pq` ; il émet l'initialiseur désigné, un champ par ligne, chaque fixe en hexadécimal signé des bits rendus (règle de D.3, jamais `PICO8_FIXED_LIT`). **Il échoue fort** si la parse n'a pas exactement les sept clés `qo`, `ro`, `qg`, `q_`, `iu`, `py`, `ms`, toutes fixes. Sortie collée à la main (≤ 120 colonnes), sans valeur éditée ; le message de commit décrit le générateur en ≤ 10 lignes.

### `Makefile`, docs

- **Le `Makefile` ne change pas** : aucun fichier nouveau. `DATA_RESIDUE_GUARD` reste celui que D.5c laisse (`spec`, H-D5c-1) : `overrides` ne tombe pas à 0.
- `docs/glossaire-noms.md` : entrées `pico8_jo_keys_t` (clés `id`/`p`/`ra` du constructeur de classe, `mw(ob)` du cart), `pico8_ga_keys_t`, `pico8_ga_pq_keys_t`/`pico8_pm_ga_pq_keys` ; ligne `kv` (le `new_entity` des sortes prend `pico8_jo_keys_t`) et ligne `k_` (`pico8_pm_ga_k__seq` est pointée par `pico8_pm_ga_pq_keys`) mises à jour.
- `docs/plan-3.md` : note « Découvert à D.6a » en fin de ligne D.6a (texte dans Further Notes). Rien d'autre dans le plan.

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucun ordre de construction, aucun tirage du RNG ; aucune valeur (le test ponctuel prouve les bits de `pm:pq`).
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements ; aucun e2e ajouté.
- Les lignes `overrides` des autres groupes (liste de l'inventaire), `g_kv`/`jo_kv` (table de sortes, pas de clés), le `field` des sortes et les défauts (D.5c), les écrivains par nom (`set_field`, aides `*_take_*` survivantes), `pico8_ob.c/.h`, `tools/data_residue.sh`, `tools/class_coverage_*`, `KNOWN[]`.
- `gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`.
- **TROU** : aucun écart port/cart touché ni ajouté ; TROU-7 préservé (planètes jamais libérées, `id_str`/vecteurs adoptés).

### Anti-tassement (liant)

Le compteur compte des **lignes**, et le motif `overrides` est un **nom de paramètre**. Une baisse obtenue sans retirer la construction comptée est un échec.

- **Interdit** : renommer un paramètre, une variable ou un commentaire `overrides` **sans que le type de la donnée change** (table ob → struct typée) — c'est le cas de tous les sites des autres groupes et de `gh_row_lq` ; joindre sur une ligne des déclarations, instructions, arguments, conditions, champs ou entrées d'initialiseur qui étaient sur des lignes distinctes ; retirer des retours à la ligne dans une ligne comptée ; macro, `typedef` ou fonction enveloppe dont le seul rôle est de renommer un identifiant compté (pas de `pico8_overrides_t`, pas de `#define`) ; **un registre dans un TU** (tableau, table, `switch` ou boucle nouveaux qui nomment ensemble classes, clés ou valeurs attendues ; les aides existantes `position_new`/`tree_new` de `test_pico8_ui` et `descendants[]` de `test_pico8_qh` sont retypées, pas étendues) ; **une aide de TU qui reconstruit une table de clés par nom** ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; supprimer un commentaire qui ne porte pas sur une clé, une boucle, une aide ou un contrat retiré par D.6a ; **toute vue ob nouvelle construite à l'appel ou à la frame**, toute `pico8_ob_table_new` ajoutée.
- **Légitimité du renommage, contrôle exigé** : chacune des 121 lignes `overrides` retirées est, au commit, classée (a) construction supprimée (allocation, pose, libération, boucle ou lecture d'entrée d'une table de clés), ou (b) signature, déclaration ou commentaire de contrat dont le paramètre passe de `pico8_ob_table_t` à `pico8_jo_keys_t`/`pico8_ga_keys_t` ; aucune ligne (c) « même type, autre nom ». Recette : `git diff -U0` de chaque fichier, lignes `-` qui portent `overrides` mises en regard des lignes `+` ; aucune ligne `+` ne porte `pico8_ob_table_t` dans une signature de l'arbre `jo` ou de `new_entity`.
- **Mesure exigée, avant et après** : par catégorie, **lignes** (`make data-residue`, et `--detail` par fichier) **ET occurrences** (recette ci-dessous), le test ponctuel donné à part. Recette figée pour l'implémenteur et le vérificateur :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  X=$(sed -n "s/^SPEC_EXCLUSION='\(.*\)'\$/\1/p" tools/data_residue.sh)
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E "$X" "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  ```

  Mesuré sur `e52b1c2` : `valeur` 6741, `ob` 2152, `stash` 28, `type-tag` 686, `ref-audit` 70, `par-nom` 1140, `dispatch` 199, `overrides` 479, `spec` 470. Départ réel : l'arbre de D.5c, remesuré.
- **Contrôle** : pour chaque catégorie et chaque fichier du diff, Δlignes et Δoccurrences. **Si Δlignes > Δoccurrences** dans un couple (catégorie, fichier), le vérificateur lit le diff de ce fichier : chaque ligne comptée retirée correspond à une construction retirée ou convertie, pas à une construction qui réapparaît sur une autre ligne. Le message de commit liste ces couples et leur justification.
- **Sites `overrides` hors périmètre** : leur compte par fichier (lignes et occurrences) est identique avant et après, et leur texte aussi (`git diff` vide sur ces lignes).
- **Les lignes qui gagnent une catégorie** sont listées au commit ; attendu : aucune (les types de clés ne portent ni `pico8_value`, ni `pico8_ob`, ni `pico8_type_tag`).

### Empreinte remesurée (G)

| | Chemins |
|---|---|
| `src/` (11) | `pico8_struct.h`, `pico8_planet.c/.h`, `pico8_planet_family.c/.h`, `pico8_planet_family_trade.c/.h`, `pico8_galaxy.c`, `pico8_research_icon.c`, `pico8_game_data.c/.h` |
| `tests/` (17) | `test_pico8_map_tables` (test ponctuel), `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, `ui`, `g_`, `jc`, `hp`, `mz`, `bg` |
| docs (2) | `docs/glossaire-noms.md`, `docs/plan-3.md` |
| **Total** | **30** |

- **Écart avec la ligne (11 : 8, 3)** : la ligne comptait les fichiers qui portent le mot `overrides`. Elle ne voyait pas `pico8_research_icon.c` (`ie:ldown`), `pico8_game_data.c/.h` (donnée de `pm:pq`), ni les TU qui construisent dans une variable `b` (quatorze) ; elle comptait `test_pico8_entity_kind`, `test_pico8_planet_gh_row_lq` et, par leurs lignes, des sites de D.6b/D.6c qui ne changent pas. `test_pico8_mx` change pour ses propres tests de mort de `mx`, pas pour sa ligne `jc`.
- **Découpage (G) : un seul commit.** 30 chemins, sous le seuil de ~40. Aucun découpage n'est vert seul : le type de `new_entity` lie à la fois les neuf sortes, `jo:hu` et `ie:ldown`, et les appelants de chaque constructeur changent avec sa signature. **Rejetés** : **par classe** (demanderait une double signature ou un alias, interdits) ; **donnée de `pm:pq` d'abord** (un objet sans lecteur, commit sans sens propre, et le test ponctuel se lit mieux avec son lecteur). Si l'implémentation dépasse 40 chemins, l'implémenteur le note au commit ; ce n'est pas un arrêt.

### Effet sur `make data-residue`

Attendu, lignes / occurrences (colonne « départ » : `e52b1c2` ; D.5c doit y laisser `overrides` identique, H-D5c-2) :

| Catégorie | Départ | Après D.6a | Pourquoi |
|---|---|---|---|
| `overrides` | 450 ; 479 (62 fichiers) | **329 ; 348 (55 fichiers)**, exact | les 121 lignes / 131 occurrences de l'inventaire ; tombent à 0 : `planet.h`, `planet_family.c/.h`, `planet_family_trade.h`, `galaxy.c`, `struct.h`, `test_pico8_bg` |
| `spec` | 0 (après D.5c) | **0** | garde |
| `par-nom` | P₀ (après D.5c) | **P₀ − 31** (écart ≤ 3 expliqué) | 26 lectures `*_take_*` des boucles et 5 définitions d'aides mortes (mesuré sur `e52b1c2`, H-D5c-6) |
| `ob` | rapporté | **strictement plus bas** ; `src/` ≈ −69, TU rapporté | `pico8_ob_table_t`/`pico8_ob_key_t`/`PICO8_OB_KEY_NAME` des boucles, tables des appelants, parse de `pm:pq`, signatures ; tables `b` des TU ; test ponctuel +0 à +2 |
| `valeur` | rapporté | **strictement plus bas** | `pico8_value_t value = …`, `pico8_value_ref`/`_string` des poses, aides mortes |
| `type-tag` | rapporté | ≤ départ, rapporté | `assert` des aides mortes (`jo_take_vec`, `family_take_seq`, `family_trade_take_vec`), tests de mort retirés |
| `stash` | 28/4 | **28/4** | — |
| `dispatch` | 197 ; 199 | **197 ; 199** | — |
| `ref-audit` | 69 ; 70 | **≤ 69** | — |

- **`overrides` = 329 est attendu exact** ; tout écart est expliqué ligne par ligne au commit, et **une baisse sous 329 est un échec** sauf à prouver qu'une ligne en plus a changé de type (jamais par renommage seul). Le « = 316 » de la note de D.4a ne l'est pas, pour la raison mesurée plus haut ; la note corrige.
- Aucune catégorie ne tombe à 0 : `DATA_RESIDUE_GUARD` ne change pas.

### Hypothèses sur D.5c

- **H-D5c-1** : D.5c est livrée et gradée comme sa ligne le dit : `spec` = 0 et dans `DATA_RESIDUE_GUARD` ; `pico8_ke_start_field`, les `pico8_X_default` des dix classes, les `g_X_spec` et les aides `family_default_*`/`family_trade_default_*` ont disparu ; l'arbre de départ de D.6a est son commit gradé (ou un commit de docs au-dessus).
- **H-D5c-2** : D.5c ne touche aucune ligne `overrides` : 450 lignes / 479 occurrences / 62 fichiers au départ de D.6a, même partage par fichier que l'inventaire (seuls les numéros de ligne bougent).
- **H-D5c-3** : chaque constructeur de l'arbre garde sa forme : `static void *pico8_X_struct_new(const pico8_ob_table_t *overrides)` (défauts, boucle de clés, couverture, chaîne `pq`), enveloppe publique `pico8_X_new`, sorte `{pico8_X_struct_new, <lecteur>}` ; D.5c ne change que le bloc des défauts, l'`assert` d'init (`g_X_spec` → drapeau) et le second membre des sortes.
- **H-D5c-4** : `pico8_entity_kind_t` garde `new_entity` tel quel (`pico8_struct.h`, ligne `overrides` unique) ; D.5c ne change que ce que pointe son `field` (nom hors motif, compté par `par-nom`), éventuellement son type.
- **H-D5c-5** : `pm:pq` (parse, table `defaults`, `k_`, appel, libérations) est intouché par D.5c ; `pico8_pm_ga_k__seq` et `PM_PQ_KEYS_SRC` existent toujours.
- **H-D5c-6** : les aides d'écriture (`jo_take_*`, `family_take_*`, `family_trade_take_*`) et leurs usages dans les boucles de clés sont intouchés par D.5c : les 31 lignes `par-nom` de D.6a existent au départ.
- **H-D5c-7** : D.5c ajoute dans `pico8_game_data.c/.h` des défauts `pico8_X_start` de l'arbre `jo` ; `pico8_ga_pq_keys_t` et `pico8_pm_ga_pq_keys` n'y entrent pas en collision (aucun nom en `_start` ni `_default`).
- **H-D5c-8** : les sites de construction des TU (tables `b`, tests de mort `build_with_unknown_key`/`_positional_key`, `build_off_row`/`build_id_off_row`, aides `position_new`/`tree_new`/`bare_jo`/`harvest`/`spawn`, `descendants[]`) existent après D.5c avec les mêmes clés ; D.5c n'y retire que des lectures de défauts (`pico8_jo_default` à `test_pico8_ui` l. 1876, `pico8_pm_default` l. 3235…).
- **H-D5c-9** : l'arbre de D.5c est vert (`make test`, `make e2e` 26, `make sanitize-net` 26, `make class-coverage-net` identique, `make data-guard` 0).
- **H-D5c-10** : D.5c ne touche ni `pico8_galaxy.c`, ni `ie:ldown`, ni les constructions de `nr` de `test_pico8_mz`/`test_pico8_bg`.
- **H-D5c-11** : `pico8_struct.h` garde ses inclusions ; `pico8_vec_t` y est atteignable (sinon une inclusion de `pico8_vec.h` s'ajoute, sans catégorie).

**Démentie** : l'hypothèse fausse est nommée au commit, la section concernée de ce PRD est relue ; seule une ligne `overrides` touchée par D.5c (H-D5c-2) ou une boucle de clés supprimée par D.5c (H-D5c-3) oblige à recalculer le compte attendu, sans jamais l'abaisser par renommage.

### À revérifier après D.5c (contre sa note « Découvert à D.5c »)

1. **Problem Statement / Inventaire** : tableau des catégories, partage `overrides` par fichier (H-D5c-2), numéros de ligne cités.
2. **Constructeurs** : forme de `pico8_X_struct_new`, nom du second membre des sortes, `assert` d'init (H-D5c-3, H-D5c-4).
3. **Donnée générée** : noms et emplacement des `pico8_X_start` de l'arbre dans `pico8_game_data.*` (H-D5c-7) ; `PM_PQ_KEYS_SRC` et `test_jo_tree_data_matches_the_parsed_class_literals` (H-D5c-5).
4. **Effet sur `make data-residue`** : départs de `par-nom`, `ob`, `valeur`, `type-tag` (H-D5c-6) ; `DATA_RESIDUE_GUARD` (H-D5c-1).
5. **Empreinte remesurée** : si D.5c a déplacé un constructeur ou ajouté un fichier à l'arbre.
6. **Testing Decisions** : noms des tests et aides de TU, constantes `*_HASH` que D.5c aurait changées (H-D5c-8).
7. **Critères d'échec** : l'arbre de départ (commit gradé de D.5c).

### Hypothèses sur D.6a (rien n'a été lancé)

- **H1** : l'arbre de départ est vert et `make data-residue` y rend l'inventaire décalé par D.5c. Démenti : arrêt, D.6a ne répare rien.
- **H2** : absent ≡ nil pour `id`, `p`, `ra` dans les dix classes (seul `pico8_jo_struct_init` les écrit, à NULL ; confirmé par `grep` sur `e52b1c2`).
- **H3** : aucun appelant de `src/` ne passe un `id` non chaîne ni un `p`/`ra` non vecteur ; seuls les tests de mort le font.
- **H4** : la chaîne de `pm:pq` a exactement sept clés fixes ; le générateur le vérifie.
- **H5** : aucune constante `*_HASH` ne bouge : les instances construites sont identiques ; les tests de mort retirés (`must_abort`) ne nourrissent aucun hachage. Démentie : arrêt sur la constante, jamais de mise à jour.
- **H6** : `pico8_ga_new` et la sorte `ga` construisent la même instance qu'aujourd'hui pour les mêmes clés (même ordre : clés `jo`, puis `pq`, puis `ga:pq`).
- **H7** : `make class-coverage-net` est identique : aucune instanciation ajoutée ni retirée.
- **H8** : `make sanitize-net` rend 26 propres : les clés passent du tas à la pile, rien d'autre ne change de propriétaire (`id_str`, `p_heap`, `ra_heap`, le vecteur de `pm:pq` restent adoptés).
- **H9** : `-Werror` reste muet : aides mortes supprimées, inclusions devenues inutiles retirées.
- **H10** : `constants-check`, `float-grep`, `float-guard` restent verts : aucun flottant ni `PICO8_FIXED_LIT` ajouté.
- **H11** : les tests de mort qui restent (`p` non NULL pour `qh`, `nl`, `os`, `hj`, `fr`, `mx`) rougissent si l'`assert` est retiré.
- **H12** : aucun appelant de `new_entity` ou des dix constructeurs hors de l'inventaire (`grep` sur `e52b1c2`).
- **H13** : durées courtes : `make test` en quelques secondes plus une recompilation large (`pico8_struct.h`), `make e2e` ~1 s, `sanitize-net` et `class-coverage-net` < 2 min chacun.
- **H14** : `make raylib` (non exigé, jamais lancé) lierait : aucun fichier nouveau.

## Implementation Chunks

Un seul commit (section « Découpage »). Ordre conseillé dans le commit : mesure de départ (lignes et occurrences, par fichier) ; générateur **avant toute modification** ; type et donnée `pico8_ga_pq_keys_t`/`pico8_pm_ga_pq_keys`, test ponctuel étendu, `make test` ; types de clés, `new_entity`, dix constructeurs, aides mortes, appelants de `src/` ; TU convertis ; `make test` ; `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois ; gcov, mutations ; mesure d'arrivée, contrôle de légitimité du renommage, anti-tassement, gardes ; glossaire, note du plan.

## Testing Decisions

Un bon test juge ce que le code rend (instances, valeurs, identités, pixels), pas la présence d'une ligne. D.6a change la forme des clés, pas ce qu'elles posent : **la preuve est que chaque constructeur pose, pour les mêmes clés, les mêmes membres qu'avant, que la donnée de `pm:pq` égale la parse du cart au bit près, et que tout ce qui mesure le comportement rend la même chose.**

### Le test ponctuel `tests/test_pico8_map_tables.c`, étendu (G)

- Il construit son état, il ne rejoue rien ; il ne nomme aucun motif de `spec`, d'`overrides` ni de `type-tag` ; ses 2 lignes `stash` ne changent pas.
- **`pico8_pm_ga_pq_keys` contre `parse(PM_PQ_KEYS_SRC)`, dans les deux sens** : la parse a exactement huit clés ; chacun des sept fixes égale son champ au bit près ; `k_` est `&pico8_pm_ga_k__seq` (dont le contenu est déjà vérifié par D.4a).

### TU touchés : clés en struct locale, mêmes constantes (G)

- **Règle** : chaque table `b` (ou `overrides`) passée à un constructeur de l'arbre ou à `new_entity` devient une struct locale initialisée à son site, un champ par ligne ; plus de `pico8_ob_table_new`/`set_name`/`free` pour ces constructions ; aucune aide nouvelle qui construise des clés par nom. Les aides existantes (`harvest`, `spawn`, `bare_jo`, `position_new`, `tree_new`, `descendants[]`) sont retypées, sans cas ajouté.
- **Tests de mort, un par un** :
  - **retirés, le type les rend impossibles à écrire** : clé positionnelle (`qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, `jo`) ; nom sans membre (`jo`, `pm`, `nr`, `mx` : `hp` ; `ga` : `fa`) ; valeur hors rangée à la construction (`ga` : `build_off_row` ; `ui` : `build_off_row` de `p`/`ra`, `build_id_off_row`). Seule la moitié « construction » part : les écritures hors rangée par `set_field` qui partagent ces boucles restent.
  - **gardés, sur `p` non NULL** : « seulement `id` » de `qh`, `nl`, `os`, `hj` (déjà sur `p` = vecteur) ; `fr` (aujourd'hui `p` nil, qui ne se distingue plus de l'absence : passe à un vecteur) ; `mx` (aujourd'hui `hp`, sans membre : passe à `p` vecteur). Même intention : la classe ne prend que `id`.
- **`test_pico8_ga`** : `keyed` passe un `pico8_ga_pq_keys_t` local (les sept fixes du test et `&k_`) ; `nil_k`, le même avec `k_` NULL, asserte toujours `k_` illisible ; `roads[1]` (par `pm:pq`) a toujours `k_` = `&pico8_pm_ga_k__seq`.
- **`test_pico8_qh`** : la boucle `descendants[i]->new_entity` passe `&keys` (`id` "s99").
- **`test_pico8_mz`, `test_pico8_bg`** : `nr` construit sur une struct locale ; `p`/`ra` lus par leur aide existante (`get(ly, …)`) puis convertis en pointeurs, sans nouvelle lecture par nom (le vecteur est déjà un REF).
- **Constantes** : aucune constante `*_HASH` ne change (H5). Toute constante qui bouge est un arrêt.

### Les trois tests du §6 (G)

- **Inchangés.** `gh_row_lq` et `ga_export_panel_by` n'appellent que `pico8_jo_new(NULL)`/`pico8_ga_new(NULL)` ; `layout_nil_item_draws_nothing` est à D.6b. Le commentaire l. 2 de `gh_row_lq` passe à D.6b (décision plus haut). Pas de preuve rouge.

### Preuve des tests (§7, gcov + mutations)

- **gcov** (copie jetable du scratchpad, `--coverage`) : chaque ligne ajoutée ou modifiée des dix constructeurs, de l'adaptatrice de `ga`, de `bw`, `jo:hu`, `pm:pq`, `ie:ldown` et de `pico8_game_data.c` est exécutée par un TU ; les lignes atteintes seulement par les e2e (`bw` par `pico8_eu`) sont justifiées au commit.
- **Mutations**, chacune doit rougir un test, puis être annulée :
  - **M1** : un bit de `pico8_pm_ga_pq_keys.ms` changé → test ponctuel rouge ;
  - **M2** : `ga` ignore `pq->k_` (garde son défaut) → `test_pico8_ga` rouge (`roads[1]`, `keyed`) ;
  - **M3** : sous `pq`, `k_` NULL laisse le défaut → `test_pico8_ga` rouge (`nil_k`) ;
  - **M4** : `jo:hu` ne passe pas `ra` → `test_pico8_ga` (`harvest`) ou `test_pico8_ui` rouge ;
  - **M5** : `ie:ldown` ne passe pas `id` → `test_pico8_entity_kind` rouge ;
  - **M6** : l'`assert` « seulement `id` » retiré de `fr` → `test_pico8_fr` rouge ;
  - **M7** : `nr` ignore `keys->p` → `test_pico8_nr`, `test_pico8_mz` ou `test_pico8_ui` rouge ;
  - **M8** : `bw` pose `.p = ra_heap` → un TU qui part de `start_game()` ou `make e2e` rouge.
- Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée.

### Pas de nouvel e2e (G)

Les lignes changées sont atteintes par les 26 e2e et par les TU ; un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 12 minutes pour l'étape.**

| Cible | Durée attendue |
|---|---|
| générateur (scratchpad, arbre de départ) | < 1 min, compilation comprise |
| `make test` (×2 : donnée + test ponctuel ; conversion complète) | quelques secondes chacun, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H13) |
| `make class-coverage-net` | < 2 min (H13) |
| gcov + mutations (scratchpad) | < 4 min |
| `make data-guard`, `make data-residue`, recette d'occurrences, contrôle du renommage | < 1 min |

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + « donnée » / « constructeurs » / « TU »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre du commit : `timeout 5m make test` ; `timeout 5m make e2e` : **26 « conforme »** ; `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ; `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ; `make data-guard` : code 0 ; `make data-residue` + recette : `overrides` 329/348/55 fichiers, `spec` 0, `stash` 28/4, `dispatch` 197, `par-nom` départ − 31 (± 3 expliqué), `ob` et `valeur` strictement plus bas ; contrôle de légitimité et anti-tassement passés.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` liste les 30 chemins de « Empreinte remesurée » (écart ≤ 3 expliqué), rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements, `Makefile` et `tools/` inchangés, `gh_row_lq`/`ga_export_panel_by`/`layout_nil_item_draws_nothing` intacts.
2. **Types** : `new_entity` et les dix constructeurs prennent `const pico8_jo_keys_t *` ou `const pico8_ga_keys_t *` ; aucun champ `pico8_value_t`, aucune recherche par nom dans les clés ; `grep` de la « Sortie de l'étape » vides.
3. **Donnée** : `pico8_pm_ga_pq_keys` dans `pico8_game_data.c`, initialiseur désigné, un champ par ligne, fixes en hexadécimal signé ; `pico8_game_data.*` n'inclut ni `pico8_ob.h`, ni `pico8_value.h` ; plus aucune parse dans `pm:pq`.
4. **Renommage** : les 121 lignes retirées sont classées (a)/(b), aucune (c) ; les sites des autres groupes sont identiques octet pour octet.
5. **Mémoire** : aucune `pico8_ob_table_new` ni `pico8_jb_row_view` ajoutée ; les tables de clés des quatre appelants de `src/` ont disparu.
6. **Anti-tassement** : comptes avant/après, en lignes ET en occurrences, par catégorie et par fichier, recalculés avec la recette ; chaque couple Δlignes > Δoccurrences justifié ; aucune jonction de lignes, aucun alias, aucun registre de TU, aucun déplacement hors du compteur, aucun commentaire sans rapport supprimé.
7. **TU** : plus aucune table de clés pour l'arbre `jo` ; tests de mort retirés listés au commit avec leur raison ; ceux gardés rougissent (M6) ; aucune constante `*_HASH` changée.
8. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun récit de chantier ; aucun renvoi à D.6a ni au plan dans `src/` ou `tests/`.
9. **Docs** : glossaire à jour ; note « Découvert à D.6a » en fin de ligne D.6a, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le commit gradé de D.5c (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas `overrides` 450/62 et `spec` 0 (H-D5c-1, H-D5c-2 démenties : relire le PRD avant tout code).
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence.

**Périmètre**

- `FAILED:` si un constructeur de l'arbre `jo` ou `new_entity` prend encore une `pico8_ob_table_t`, si une clé de construction de l'arbre est encore lue par nom (`strcmp`, table, paires), ou si une ligne des `grep` de la « Sortie de l'étape » subsiste.
- `FAILED:` si une ligne `overrides` d'un autre groupe (`iv`, `jc`, `ie`, `trade`, `js`/`lq`, `g_`, `fv`, `hp`, `hy`, `pj`, `ui`, `qb`, `nf`, `bg`, `mz`, `menu.c`) change, ou si `test_pico8_planet_gh_row_lq.c` est touché.
- `FAILED:` si le diff convertit le `field` des sortes, un défaut, `g_kv`/`jo_kv`, un écrivain par nom, ou touche un TROU.
- `FAILED:` si une valeur de `pico8_pm_ga_pq_keys` a été écrite ou corrigée à la main, si un générateur est versionné, ou si `pico8_game_data.*` inclut `pico8_ob.h` ou `pico8_value.h`.
- `FAILED:` si le diff ajoute une `pico8_ob_table_new`, un appel à `pico8_jb_row_view`, ou toute table ou vue construite à chaque appel ou à chaque frame.
- `FAILED:` si le test ponctuel nomme un motif de `spec`, d'`overrides` ou de `type-tag`, ou porte plus de 2 lignes `stash`.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` ou `tools/data_residue.sh` change.
- `FAILED:` si `overrides` ne rend pas 329 lignes / 348 occurrences sans explication ligne par ligne, ou descend sous 329 par un renommage ; si `stash` n'est pas 28/4, `dispatch` 197, `spec` 0 ; si `par-nom` s'écarte de départ − 31 de plus de 3 ou sans explication ; si `ob` ou `valeur` ne baisse pas strictement.
- `FAILED:` **si une ligne `overrides` disparaît par renommage sans changement de type** (classe (c)), ou si la classification (a)/(b) des 121 lignes manque au commit.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés dans une ligne comptée, construction comptée déplacée sur une autre ligne comptée, alias (macro, `typedef`, enveloppe) d'un identifiant compté, code déplacé hors de `src`/`tests` ou hors `.c`/`.h`, commentaire sans rapport supprimé.
- `FAILED:` si un TU construit ses clés par un registre, une table ou une aide par nom nouvelle.
- `FAILED:` si un commit ne donne pas, par catégorie, les comptes avant/après **en lignes et en occurrences**, ou si un couple (catégorie, fichier) où Δlignes > Δoccurrences n'est pas justifié.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` (ou `identity[]`) change.
- `FAILED:` si un test de mort est retiré sans être listé au commit avec sa raison, ou si un test de mort « seulement `id` » manque pour `qh`, `nl`, `os`, `hj`, `fr` ou `mx`.
- `FAILED:` si le rapport gcov ou une mutation manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si le message de commit ne porte pas les comptes avant et après et la description du générateur.
- `FAILED:` s'il manque la note « Découvert à D.6a », ou si elle omet le compte `overrides` réel et l'explication de l'écart avec 315/316, les types de clés, ou le report du commentaire de `gh_row_lq` à D.6b.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les clés de construction des autres classes** : `iv`, `jc` (y compris leurs appels depuis `planet.c` et `planet_family_trade.c`), `lq`/`jb`, `js`/`fq`, `mh`, `ui`, `qb`, `nf`, `bg`, `mz`, `menu.c` : D.6b ; `g_`, `fv`, `ie` (son constructeur, `pico8_cart.c`), `hp`, `hy`, `trade` (et `planet_trade.c`), `pj` : D.6c.
- **Le commentaire l. 2 de `test_pico8_planet_gh_row_lq`** : D.6b (il parle des clés de `js`).
- **Les défauts de l'arbre `jo`, le `field` des sortes, `pico8_ke_start_field`** : D.5c.
- **`g_kv`/`jo_kv`** (table `fz` → sorte) : ce n'est pas une table de clés ; elle reste.
- **Les écrivains et lecteurs par nom** (`set_field`, aides `*_take_*` survivantes, `pico8_entity_get_field`) : `par-nom`, D.9.
- **Le vecteur alloué par `pm:pq`**, les `p_heap`/`ra_heap`/`id_str` de `bw` : données d'entité adoptées (Plan 2 §8, TROU-7).
- Les archétypes, la fusion des clés dans la phase 2 ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **forme** : `pico8_jo_keys_t` (`id`, `p`, `ra`, NULL = absent ≡ nil) pour neuf classes et `new_entity` ; `pico8_ga_keys_t` = `jo` + `pq` (NULL = défauts ; `k_` NULL sous `pq` = nil, cas `nil_k` gardé) ; `pico8_ga_pq_keys_t`/`pico8_pm_ga_pq_keys` générés depuis la chaîne de `pm:pq` ; `pico8_X_new(NULL)` inchangé ; adaptatrice pour la sorte `ga`.
  - **renommage** : légitime seulement avec le changement de type, contrôlé ligne par ligne (classes (a)/(b), aucune (c)).
  - **compte** : 329, pas 315 ni 316 (majorant de fichiers pris pour exact ; ligne de D.4a dans le périmètre).
  - **§6** : `gh_row_lq` intouché, son commentaire passe à D.6b.
  - **tests** : clés en struct locale à leur site ; tests de mort impossibles retirés et listés, « seulement `id` » gardés sur `p` non NULL ; test ponctuel étendu à `pm:pq` ; huit mutations ; pas d'e2e ; `class-coverage-net` et `sanitize-net`.
  - **découpage** : un commit (le type de `new_entity` lie tout) ; 30 chemins.
  - **D.5c** : onze hypothèses H-D5c, sept sections à relire contre sa note.
- **Note du plan (G)**, à ajouter en fin de ligne D.6a (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.6a : un commit (≈ 30 chemins : `research_icon.c` pour `ie:ldown`, `pico8_game_data.*` et quatorze TU qui construisent dans une variable `b`, hors du majorant). Clés typées : `pico8_jo_keys_t` (`id`, `p`, `ra`, NULL = absent, `pico8_struct.h`) pour neuf classes et `new_entity` ; `pico8_ga_keys_t` (`jo` et `pq`, NULL = défauts de `ga`) ; la chaîne que `pm:pq` parsait devient `pico8_pm_ga_pq_keys` (`pico8_ga_pq_keys_t`, générée). `pico8_X_new(NULL)` inchangé ; les classes « seulement `id` » l'assertent sur `p`/`ra`. Tests de mort d'une clé positionnelle, sans membre ou hors rangée à la construction retirés (le type les interdit). Le commentaire l. 2 de `gh_row_lq` parle des clés de `js` : il passe à D.6b (le changer ici serait un renommage à type constant). `overrides` après D.6a : N (au lieu de 316 : le « = 315 » de D.1 était le total des fichiers de l'empreinte, dont 30 lignes de D.6b/D.6c — `iv`/`jc` de `planet.c` et `planet_family_trade.c`, `jc` de `test_pico8_mx`, `gh_row_lq`, `ie` de `test_pico8_entity_kind` — et sans 16 lignes de D.6a — `ie:ldown`, `nr` de `test_pico8_mz`/`test_pico8_bg` ; la ligne de `k_` ajoutée par D.4a est dans le périmètre ; attendu 329) ; le compte de D.6b est à reprendre, le « = 0 » de D.6c est inchangé. `par-nom` : P.

  N est le compte `overrides` réel, P le compte `par-nom` réel.
- **Contrôle d'étape** : un sous-agent frais relit le diff contre ce PRD avant le commit (hors budget de tests), anti-tassement et légitimité du renommage compris.
- **Le message de commit** porte : le périmètre ; la forme des clés par classe et ses rejets ; le générateur en ≤ 10 lignes ; la classification (a)/(b) des 121 lignes `overrides` retirées ; les comptes avant et après **en lignes et en occurrences** par catégorie et par fichier, et les couples Δlignes > Δoccurrences justifiés ; les tests de mort retirés, un par un, avec leur raison ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses H et H-D5c qui le concernent.
- **Surprises pour le pilote** :
  - **Le compte de la ligne est faux de 13** (329 et non 316) : le « = 315 » était un total par fichier pris pour exact ; il comptait 30 lignes d'autres groupes et en manquait 16 de D.6a ; la note de D.4a a ajouté une ligne qui est pourtant dans le périmètre. Les comptes de D.6b sont à reprendre avec la même méthode (lignes du groupe, pas fichiers du groupe).
  - **L'empreinte de la ligne (11) sous-estime d'un facteur trois** : les TU construisent leurs clés dans une variable `b`, invisible pour le motif `overrides` ; ils changent quand même, parce que la signature change.
  - **`new_entity` est générique** : il lie les neuf sortes, `jo:hu` et `ie:ldown` (fichier de D.6c) dans le même commit, d'où un type commun et un seul commit.
  - **La ligne du plan envoyait ici le commentaire de `gh_row_lq`**, qui parle des clés de `js` : le changer à D.6a serait le renommage à type constant que la règle interdit ; il passe à D.6b.
  - **`pm:pq` parsait une chaîne constante à chaque apparition** : c'est une table constante de la première puce, cachée dans les clés ; elle devient une donnée générée.
  - **Le type retire des tests de mort** (clé positionnelle, nom sans membre, genre hors rangée) : ce n'est pas une perte de garde, le compilateur la porte ; seul « seulement `id` » reste un contrat d'exécution.
