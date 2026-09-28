---
title: "Plan 3, étape D.5b : les spécifications de `g_`, `fv` (aide), `hp`, `ie` (et `ie_chain`), `pj`, `hy` et `trade` deviennent des défauts `static const` générés une fois (`pico8_X_start`) que les constructeurs lisent ; les lecteurs par nom `pico8_X_default`, les `*_defaults_reset`, les aides `*_default_<genre>` et les statiques recopiés disparaissent ; `pico8_rk_default`, variante de dessin, sort du motif `spec` par l'exclusion prouvée de D.0 — deux commits (chunks), aucun pixel changé"
description: "Après D.5a, sept spécifications de classe hors de l'arbre `jo` sont encore parsées par `ob()` à l'init de leur module : `g_` et `fv` les relisent par nom pour remplir des statiques recopiés, `ie` en parse deux (la chaîne de classe et les objets qu'il prête à chaque icône), `pj` y prend ses tables d'état `ou`/`ea` et garde à côté un `cr` recopié à la main, `hp` n'y a plus que des tables déjà générées, `hy` et `trade` sont vides. D.5b génère ces défauts une fois en `static const`, fait lire ces données par les constructeurs (une seule source), supprime les tables, leurs lecteurs et le registre de `test_pico8_spec_defaults`, règle le cas `pico8_rk_default` et mesure la baisse de `spec` en lignes ET en occurrences, sans tassement."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.5b — `spec`, `g_`/`fv` (aide), `hp`, `ie` (et `ie_chain`), `pj`/`rk`, `hy`, `trade`, même règle. Empreinte 23 (16, 7). Compte : `spec` ≤ 547. §6 : aucun. `class-coverage-net` : oui.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » des étapes livrées, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a) :

> Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.

> Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.

> Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).

> Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).

> Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp` : `lq_own`/`lq_inner`) ; `hp.mr`/`gp` en `pico8_str_seq_t`, `ho_apply` les prend typés ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages `po_own`/`fu_own` dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie ; une clé `qe` chaîne est absente) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus, `pico8_jc_hg_add`/`_has`) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`, qui partage son calcul avec `pico8_gi`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : 1882 (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de −100 (1882 − 1982).

> Découvert à D.5a : deux chunks (interfaces, puis `ke` ; ≈ 40 chemins, `pico8_game_data.*`, `pico8_bootstrap.c` et les TU qui plient l'identité de `me` hors du majorant). Défauts générés `pico8_X_start` (type `pico8_X_start_t`, `pico8_game_data.h`), un par classe à clés (`ke`, `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`), clés propres de la spécification, lus par les constructeurs ; les tables gardent l'objet de D.4a/D.4b ; `mz.jj` s'initialise de `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` : liste statique de `ui.c` (`pico8_ui_le_shared`) ; `qb.v`/`p`/`ra`, `iv.v`, `ui.hw` pointent la donnée générée (membres `const`) ; `nf.ra` pointe le statique non `const` `g_nf_ra`, affecté depuis `pico8_nf_start.ra` à `pico8_nf_init` (repli : `test_pico8_entity_kind` et `test_pico8_ie` écrivent `je.ra` en place) ; `nf.l`/`nf.r` restent `g_nf_l`/`g_nf_r` (D.7b). Le cycle `_init`/`_shutdown`/`_ready` des treize modules reste, sur un drapeau. `pico8_ke_start_field` (compté par `par-nom`) remplace `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_g__default`, `pico8_ie_default` (retirés à D.5b) et `pico8_jo_default` (D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`) : D.5c le supprime. `pico8_rk_default` (variante de dessin, comptée par `spec`) est à D.5b. `spec` après D.5a : 634 (au lieu de ≤ 779) ; les majorants `spec` suivants se décalent de −145 (634 − 779). `par-nom` : 1090.

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

Lecture de ces textes pour D.5b :

- **« Même règle »** renvoie à D.5a : une spécification de classe est une table constante parsée au démarrage (première puce) ; elle devient un défaut `static const` généré une fois depuis `pico8_ob(spec)` (`pico8_X_start`, type `pico8_X_start_t`, dans `pico8_game_data.h`), que le constructeur lit — une seule source — ; `g_X_spec`, `pico8_X_default`, `*_defaults_reset` et les aides `X_default_<genre>` disparaissent ; le test ponctuel est étendu dans les deux sens ; les TU comparent directement aux données générées, sans registre.
- **Le majorant de la ligne (`spec` ≤ 547), décalé par la note de D.5a (−145), donne ≤ 402. Il n'est pas tenable, et l'écart est mesuré** : le majorant 547 a été pris sur la base 939 de la liste en retirant 232 lignes aux sites de D.5b ; D.4a/D.4b en avaient déjà retiré 28 (`probe.c` 5, `help.c` 5, `research.c` 1, `score_panel.c` 4, `test_pico8_fv` 4, `test_pico8_g_` 9 : 243 → 215 lignes dans les fichiers de D.5b entre `3816427` et `64bb06e`), et ces 28 lignes sont aussi dans le décalage −145 de D.5a, qui les compte donc deux fois. Majorant corrigé : 402 + 28 = **430**, exactement le compte attendu (section « Effet sur `make data-residue` »). La note « Découvert à D.5b » le dit ; le « = 0 » de D.5c n'en dépend pas.
- **`pico8_rk_default`** : la note de D.5a le met à D.5b (« variante de dessin, comptée par `spec` »). Ce n'est pas un défaut de classe : c'est l'appel du cart `rk(127-h,127)`, sans couleur, exactement la sorte des six `_default` à argument omis que D.0 a exclus (`SPEC_EXCLUSION`). Tant qu'il est compté, `spec` ne peut pas tomber à 0 à D.5c. D.5b l'ajoute à l'exclusion, avec la preuve que la règle « Motifs figés » de D.0 exige (décision plus bas).
- **Fuite connue** : les vues construites à chaque frame par D.4b dans deux fichiers du périmètre (`fv:kj`, `help.c:90` ; `pj:kj`, `score_panel.c:317`) libèrent la table de tête mais pas les rangées imbriquées : elles fuient à chaque frame d'aide ou de panneau de score (TROU-7) jusqu'à D.7a. **D.5b n'ajoute aucune vue construite à l'appel ou à la frame**, et ne ferme pas celles-ci (décision plus bas).
- **`make sanitize-net` et `make class-coverage-net` au chunk 2** : il change la durée de vie de données (tables ob du tas parsées à l'init → `static const` ; objets `kq`/`ra`/`eh` de `ie`, libérés à l'arrêt → statiques) et touche sept constructeurs. Le chunk 1 ne touche pas `src/` : ni l'un ni l'autre.
- **§6 : aucun.** `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` ne lisent aucun défaut de ces classes ; ils ne changent pas.

### État de départ

HEAD `f42691f` (branche `claude/plan-pilot-3-7zdft0`), arbre propre. D.5a est livrée (`8cec2b9`, `050cf22`) et gradée (`f42691f`). Décisions de l'utilisateur qui engagent D.5b : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**. Consigne de pilotage, liante : **une étape précédente a été notée FAILED pour avoir fait baisser un compte par tassement de lignes** ; le compteur compte des lignes, et toute mise en forme, tout renommage, tout alias ou tout déplacement qui baisse un compte sans retirer la construction est interdit (section « Anti-tassement »).

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `make -s data-residue`, `sh tools/data_residue.sh --detail`, un comptage d'occurrences par `grep -o` sur les motifs figés et des `git show <rev>:<fichier> | grep -c` ont tourné. Le reste vient de `grep` et de la lecture des sources de `f42691f`. Tout fait d'exécution est une hypothèse (H1 à H19).

### Inventaire (mesuré sur `f42691f`)

**`make data-residue`** (lignes / fichiers) et **occurrences** des mêmes motifs (`grep -oE`, mêmes fichiers, même exclusion `SPEC_EXCLUSION` pour `spec`) — identiques aux comptes d'arrivée de D.5a :

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 5892 | 133 | 6809 |
| `ob` | 1954 | 123 | 2185 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 431 | 71 | 700 |
| `ref-audit` | 69 | 17 | 70 |
| `par-nom` | 1090 | 80 | 1145 |
| `dispatch` | 197 | 59 | 199 |
| `overrides` | 450 | 62 | 479 |
| `spec` | 634 | 41 | 684 |
| **résidu D** | **9063 lignes** | **142 fichiers** | — |

`ob` 1954 = **1820 hors test ponctuel** + 134 lignes (141 occurrences) de `tests/test_pico8_map_tables.c`.

**`spec` dans le périmètre** (lignes / occurrences ; `ob`, `par-nom` en lignes pour le contrôle croisé) :

| Fichier | `spec` | `ob` | `par-nom` | Ce qui est compté |
|---|---|---|---|---|
| `pico8_probe.c` / `.h` (`g_`) | 46/46, 2/2 | 16, 2 | 48, 1 | `g_g__spec`, `pico8_g__defaults_reset`, `g__default_string`/`_bool`/`_fixed`, seize paires `found = pico8_g__default(…)` / `g_g__X = g__default_…`, `pico8_g__default` (fin de chaîne `pico8_ke_start_field`, l. 857) |
| `pico8_help.c` / `.h` (`fv`) | 54/55, 1/1 | 16, 2 | 8, 0 | `g_fv_spec`, `pico8_fv_defaults_reset`, `help_default_fixed`/`_vec`/`_box`, vingt paires `pico8_fv_default` / `help_default_…`, `pico8_fv_default` (chaîne vers `pico8_g__default`) |
| `pico8_research.c` / `.h` (`hp`) | 7/8, 1/1 | 39, 4 | 17, 0 | `g_hp_spec`, parse, libération, `pico8_hp_default` |
| `pico8_research_icon.c` / `.h` (`ie`) | 19/20, 1/1 | 17, 2 | 24, 1 | `g_ie_chain_spec`, `g_ie_spec` (deux parses), `assert` d'ordre d'init, `pico8_ie_spec_ref`, libération objet par objet, `pico8_ie_default` (fin de chaîne, l. 497), `pico8_ie_eh` |
| `pico8_score_panel.c` / `.h` (`pj`) | 27/30, 1/1 | 20, 5 | 9, 0 | `g_pj_spec`, `g_pj_defaults[]` et ses lecteurs, `pico8_pj_defaults_reset`, `pj_default_table`, boucle d'init par nom, `pico8_pj_default` (fin de chaîne, l. 520) ; **l. 339 : l'appel `pico8_rk_default`** |
| `pico8_hud.c` / `.h` (`rk`) | 1/1, 2/2 | 1, 0 | 0, 0 | définition, déclaration et commentaire de `pico8_rk_default` |
| `pico8_economy.c` / `.h` (`hy`) | 7/8, 1/1 | 15, 2 | 3, 0 | `g_hy_spec` (chaîne vide), `pico8_hy_default` |
| `pico8_trade.c` / `.h` (`trade`) | 7/8, 1/1 | 16, 2 | 2, 0 | `g_trade_spec` (chaîne vide), `pico8_trade_default` |
| **modules** | **178/186** (dont `rk` 4/4) | | | |
| TU : `test_pico8_fv` 6/7 (l. 3, 354, 627, 628, 714, 895), `test_pico8_g_` 6/7 (l. 2, 402, 792, 1248, 1274, 1277), `test_pico8_spec_defaults` 7/7 (l. 67-79, le registre), `test_pico8_pj` 4/4 (l. 3, 126, 631, 634), `test_pico8_hp` 1/1 (l. 727), `test_pico8_js` 1/1 (l. 1396), `test_pico8_jo_spec_defaults` 1/1 (l. 461) | **26/28** | | | |

Hors périmètre, `spec` garde 430 lignes / 470 occurrences : l'arbre `jo` (`planet.c` 30+1, `planet_family.c/.h` 208+6, `planet_family_trade.c/.h` 88+3) et ses TU (94 lignes : `entity_kind`, `fr`, `ga`, `hj`, `jo`, `jo_spec_defaults` 10, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `ui` 18). C'est D.5c.

**Les sept spécifications** (chaîne exacte parsée par `ob()` à l'init du module) et ce que le constructeur pose aujourd'hui :

| Classe | Clés | Ce que pose le constructeur aujourd'hui | Lecteurs par nom |
|---|---|---|---|
| `g_` (`probe`) | `nu=o("g_","prober")`, `fx=1`, `dq="ly"`, `od=0`, `fp=f`, `fa=2`, `dw=0.025`, `py=0.06`, `ki=9`, `pn=3`, `pk=1`, `fc=1`, `k_=o(1,1,5,13,6)`, `qo=1.02`, `ro=-0.0525`, `qg=0.0525`, `q_=1.02`, `du=2` (18) | `pico8_g__struct_init` copie seize statiques `g_g__fx` … `g_g__du`, **remplis à l'init par lecture par nom** de la spécification (`pico8_g__default` → `g__default_*`), remis à zéro par `pico8_g__defaults_reset` ; `nu`, `k_` → objets de D.4b | `pico8_g__default` : chaîne de `fv`, TU `fv`, `g_`, `jo_spec_defaults` (l. 461), `spec_defaults` |
| `fv` (`help`, étend `g_`) | `fa=10`, `ra=v(64,28)`, `hi=v(34,56)`, `ca=11`, `gq=v(0,11)`, `eo=-1`, `ff=1`, `fc=5`, `k_=o(…)`, `qo=1.02`, `ro=-0.007`, `qg=0.007`, `q_=1.02`, `pn=5`, `du=15`, `py=0.0067`, `pk=1`, `ki=17`, `kq=b(-40,78,40,98)`, `ql=11`, `fg=5`, `rb=o(…)` (22) | `pico8_g__struct_init`, puis vingt statiques `g_fv_*` **remplis à l'init par lecture par nom** (`pico8_fv_default` → `g_` → `ke`) ; `ra`, `hi`, `gq`, `kq` sont **les vecteurs et la boîte du tas de la spécification**, partagés ; `k_`, `rb` → objets de D.4b | `pico8_fv_default` : TU `fv`, `g_`, `js` (l. 1396), `spec_defaults` |
| `hp` (`research`) | `mr=o(…)`, `gp=o("o","dm")`, `fk=o(f,2)` (3) | objets de D.4b (`pico8_hp_mr_names`, `pico8_hp_gp_names`, `pico8_hp_fk_row`) : la table parsée ne sert plus qu'au drapeau « prêt » et aux TU | TU `hp` (l. 727), `spec_defaults` |
| `ie` (`research_icon`), deux parses | chaîne de classe (`g_ie_chain_spec`) : `kq=b(0,0,9,8)`, `fa=12`, `id=0`, `ra=v(999,999)`, `eh=v(118,120)` (5) ; objets du module (`g_ie_spec`) : `kq`, `ra`, `eh`, reparsés | littéral `fa` 12 ; `kq`, `ra`, `eh` → les objets de `g_ie_spec` (`pico8_ie_spec_ref`), **libérés un par un à l'arrêt** ; `id` : le compteur de classe `g_ie_id`, remis au fixe 0 à l'init ; `pico8_ie_eh()` rend l'objet `eh` | `pico8_ie_default` (sur la chaîne) : TU `spec_defaults` ; `pico8_ie_spec_ref` : constructeur, `pico8_ie_eh` |
| `pj` (`score_panel`) | `fa=10`, `ea=o(o(42,43,1,fn="br"),o("",64,40,6,0.5,fn="qr"))`, `gs=o(…)`, `cr=o(hi=v(0,11),gq=v(0,11),ca=10)`, `i_=o(…)`, `ou=o(sept rangées o(f,x,y,c,fn="qr"))`, `ed=1` (7) | littéral `fa` 10 ; `gs`, `i_` → objets de D.4b ; `ed` absent = 1 (littéral de `pico8_pj_ed`) ; **`ou`, `ea` : tables d'état de la classe, prises dans la spécification** à l'init (`g_pj_defaults[]`), réécrites en place par `hb`/`hk`, lues par `kj`, passées à `pico8_mh_new` par `hk` ; **`cr` : le statique `g_pj_cr`, recopié à la main** (`g_pj_cr_hi`/`gq` = `{0, 11}`, `ca` 10), le `cr` parsé ne sert qu'à la présence de la clé | `pico8_pj_default` : boucle d'init, TU `pj`, `spec_defaults` |
| `hy` (`economy`) | *(chaîne vide)* | — | TU `spec_defaults` |
| `trade` | *(chaîne vide)* | — | TU `spec_defaults` |

**Faits qui engagent la forme :**

- **`g_` et `fv` lisent encore leur spécification par nom à l'init** pour remplir des statiques recopiés (36 statiques, deux fonctions de remise, six aides typées) : c'est le gros du compte (100 lignes `spec` de module).
- **`pj.ou` et `pj.ea` sont des défauts mutés** (règle commune : « un défaut muté […] `pj.ou`, `pj.ea`, `pj.cr` […] devient un tableau fixe statique non `const` ») : `hb` écrit l'index 1 de chaque rangée de `ou`, `hk` écrit `ea[2][1]`, `ea[3]` et `ea.lh`, puis passe `ea` comme clés de construction à `pico8_mh_new` (`score_panel_display.c`) ; `kj` en fait une liste pour `pico8_bl_default`. Tous ces consommateurs lisent une **table ob** : les typer, c'est D.6b (clés de `mh`) et D.7a (`score_panel`, `score_panel_display`, `pi`).
- **`pico8_ob_table_free` est superficielle** (`pico8_ob.c:52`) : aujourd'hui, les tables `ou`/`ea` (et leurs rangées) ne sont jamais libérées ; la libération de la spécification ne libère que ses entrées. De même pour les vecteurs et la boîte de `fv`, que les `fv` gardent après un arrêt.
- **`ie` libère ses objets `kq`/`ra`/`eh` à l'arrêt** alors que les icônes construites avant gardent leurs pointeurs (entités jamais libérées, TROU-7) : un défaut statique supprime ce pointeur pendant.
- **Aucune écriture en place** trouvée par `grep` dans `src/` sur `fv.hi`/`gq`/`kq`, `g_.ra`, `ie.kq`/`ra`/`eh`, `pj.cr.hi`/`gq` : ces membres sont rebranchés (`g__take_vec`, `ie_take_vec`, `set_field "ra"`), jamais écrits à travers ; `ie.gy` peut tenir le REF de `eh` (`pico8_cart.c:142`) et n'est jamais écrit à travers. Les deux écritures en place des TU (`test_pico8_entity_kind:343`, `test_pico8_ie:574`) visent `je.ra` (`nf`, déjà replié par D.5a).
- **`pico8_help_init` n'appelle pas `pico8_pi_init()`** et asserte qu'il a été appelé avant (`pico8_ob_lookup_constructor('v')`) ; `fv:kj` et `pj:kj` dispatchent par `pico8_bl_default` vers `br`/`qr`/`rf`/`s`, que `pico8_pi_init()` enregistre : ce contrat reste vrai sans la spécification.
- **`pico8_ke_start_field`** a quatre appelants de `src/` : `probe.c:857`, `score_panel.c:520`, `research_icon.c:497` (D.5b) et `planet.c:756` (`jo`, D.5c), plus le cas `M_KE` de `test_pico8_jo_spec_defaults`.
- **`test_pico8_spec_defaults` n'a plus que les sept modules de D.5b dans son registre** (enum, quatre `switch`, `NAMES`, `same_default`, trois constantes) : D.5b le vide entièrement.
- **Les quatre lignes de `pico8_rk_default`** (`hud.c:63`, `hud.h:29`, `hud.h:34`, `score_panel.c:339`) ne portent aucune autre catégorie (`grep` des huit autres motifs : 0 ligne) ; `tests/test_data_residue.c` teste l'exclusion (`test_omitted_argument_defaults_are_not_class_specs`).
- **Sites `pico8_jb_row_view` de `src/`** : 16 (`data_init.c` 7, `data_init.h` 1, `help.c` 1, `planet_display.c` 3, `research_display.c` 2, `score_panel.c` 1, `score_panel_display.c` 1).

## Solution

**Deux commits (chunks, section « Implementation Chunks »), sans aucun pixel changé** :

1. **`pico8_rk_default` sort du motif `spec` par l'exclusion de D.0**, étendue d'un septième nom, avec sa preuve et son test rouge d'abord. Aucun fichier de `src/` ne change.
2. **Les sept classes reçoivent leur défaut généré** et le lisent :
   - **`g_`, `fv`, `hp`, `ie`, `pj`** : un `pico8_X_start` `static const` de type `pico8_X_start_t` (`pico8_game_data.h`, module pur), exactement les clés de **sa** spécification, sous les types que la struct porte ; les tables pointent les objets de D.4b ou un objet nouveau par table (`pico8_pj_cr_start`, `pico8_pj_ea_row`, `pico8_pj_ou_row`). `hy`, `trade` : spécification vide, **aucun objet** (précédent `lq`/`jb`).
   - **Les constructeurs lisent ce défaut** (une seule source) ; les seize statiques de `g_`, les vingt de `fv`, `g_pj_defaults[]`, `g_pj_cr_hi`/`gq`, les deux parses de `ie`, leurs remises et aides disparaissent. Les vecteurs et boîtes partagés jamais écrits en place pointent la donnée générée (membres `const`), avec le repli de D.5a.
   - **`pj.ou`/`pj.ea`**, défauts mutés dont tous les lecteurs veulent encore une table ob : le module les construit **une fois par init** depuis leurs rangées générées (`pico8_jb_row_view`, le pont de D.4a), sous les noms `g_pj_ou`/`g_pj_ea`, avec la durée de vie qu'ils ont aujourd'hui (jamais libérés). `pj.cr` : le statique `g_pj_cr` reste (son `ep` est muté), ses valeurs viennent de `pico8_pj_cr_start`.
   - **Les sept `pico8_X_default`, `pico8_g__defaults_reset`, `pico8_fv_defaults_reset`, `pico8_pj_defaults_reset` et les aides `g__default_*`, `help_default_*`, `pj_default_table` disparaissent**, sans remplaçant par nom. Le cycle `_init`/`_shutdown`/`_ready` reste, sur un drapeau.
   - **Le test ponctuel est étendu** aux sept chaînes (et à la seconde parse de `ie`), dans les deux sens ; les TU qui lisaient ces défauts par nom comparent directement aux données générées, sans registre ; le registre de `test_pico8_spec_defaults` et ses trois constantes disparaissent, leur vérité étant reprise par des fonctions à leur nom.

**Sortie de l'étape** (après le chunk 2), constatée par commande :

- `make test` vert ; `make e2e` : 26 conformes ; `make sanitize-net` : 26 propres ; `make class-coverage-net` : identique ; `make data-guard` : code 0 ;
- `make data-residue` : `spec` = 430 (écart ≤ 6 expliqué), `par-nom` = 1085 (écart ≤ 3 expliqué), `stash` 28/4, `overrides` 450, `dispatch` 197, `ob` hors test ponctuel < 1820 ; comptes en lignes ET en occurrences, avant et après, par catégorie, par fichier et par chunk, conformes à « Anti-tassement » ;
- ce `grep` ne rend rien (hors un statique de repli de H3, nommé au commit) :

  ```sh
  grep -rnE '\bg_(g_|fv|hp|ie|ie_chain|pj|hy|trade)_spec\b|\bpico8_(g_|fv|hp|ie|pj|hy|trade)_default\b|\bpico8_(g_|fv|pj)_defaults_reset\b|\bg_pj_defaults\b|\b(g_|help)_default_(string|bool|fixed|vec|box)\b|\bpj_default_table\b|\bpico8_ie_spec_ref\b|\bg_g__(fx|dq|od|fp|fa|dw|py|ki|pn|pk|fc|qo|ro|qg|q_|du)\b|\bg_fv_(fa|ra|fc|qo|ro|qg|q_|pn|du|py|pk|ki|hi|ca|gq|eo|ff|kq|ql|fg)\b|\bg_pj_cr_(hi|gq)\b' src tests --include='*.[ch]'
  ```
- `grep -rn 'pico8_ob(' src/pico8_{probe,help,research,research_icon,score_panel,economy,trade}.c` ne rend aucune spécification (tout `pico8_ob(` restant est nommé au commit) ;
- `grep -rn 'pico8_ke_start_field' src tests` ne rend que `pico8_ke.c/.h`, `pico8_planet.c` et le cas `M_KE` de `test_pico8_jo_spec_defaults` ;
- `grep -rn 'pico8_jb_row_view' src` rend les 16 sites de `f42691f` **plus exactement deux**, dans `pico8_score_panel_init` (`ou`, `ea`).

## User Stories

1. En tant que mainteneur du port, je veux que les défauts de `g_`, `fv`, `hp`, `ie` et `pj` soient des données `static const` générées une fois, afin qu'aucune spécification hors de l'arbre `jo` ne soit plus parsée au démarrage.
2. En tant que mainteneur, je veux que `pico8_g__struct_init` et le constructeur de `fv` lisent leur défaut généré au lieu de 36 statiques remplis par lecture par nom à l'init, afin qu'une valeur de défaut n'existe qu'à un seul endroit.
3. En tant que mainteneur, je veux que `fv` ne porte que ses propres clés dans `pico8_fv_start` et hérite des autres par `pico8_g__struct_init`, afin que la chaîne `fv` → `g_` → `ke` reste celle du cart sans être recopiée.
4. En tant que mainteneur, je veux un type de défaut par classe, aux champs typés comme la struct, afin de lire `pico8_g__start.dw` sans passer par un nom.
5. En tant que gardien du comportement, je veux que les non-entiers (`g_.dw` 0.025, `py` 0.06, `qo`/`q_` 1.02, `ro`/`qg` ±0.0525 ; `fv.ro`/`qg` ±0.007, `py` 0.0067 ; les 0.5 de `pj.ea`) gardent les bits exacts qu'`ob()` rendait, afin qu'aucune trajectoire ni aucun texte ne bouge d'un pixel.
6. En tant que gardien du comportement, je veux que `g_.fp` reste le booléen faux, afin que `d_` transmette toujours 0 à `fm:nx`.
7. En tant que mainteneur, je veux que `fv.hi`/`gq`/`kq`, le `ra` de `fv`, `ie.kq`/`ra`/`eh` et `pj.cr.hi`/`gq` pointent la donnée générée, afin de supprimer les vecteurs recopiés ou reparsés.
8. En tant que gardien du comportement, je veux que toutes les icônes `ie` partagent toujours la même boîte `kq` et les mêmes vecteurs de départ `ra`/`eh`, afin que `_init` (`ie.gy = ie.eh`) et `ie:hb` se comportent pareil.
9. En tant que gardien de la mémoire, je veux que les objets `kq`/`ra`/`eh` de `ie` ne soient plus libérés à l'arrêt, afin qu'aucune icône construite avant un arrêt ne garde un pointeur pendant.
10. En tant que mainteneur, je veux que `ie` n'ait plus qu'une source pour sa chaîne de classe et pour les objets qu'il prête (aujourd'hui deux parses de `kq`, `ra`, `eh`), afin de supprimer `pico8_ie_spec_ref`.
11. En tant que gardien du comportement, je veux que le compteur de classe `ie.id` parte de `pico8_ie_start.id` à chaque init et que `fa` vienne de `pico8_ie_start.fa`, afin que les numéros de sprite ne changent pas.
12. En tant que mainteneur, je veux que `hp` lise `mr`, `gp`, `fk` par son `pico8_hp_start`, qui pointe les objets de D.4b, afin de ne pas doubler une donnée.
13. En tant que mainteneur, je veux que `hy` et `trade`, dont la spécification est vide, n'aient aucun défaut généré, afin de ne pas créer d'objet sans contenu.
14. En tant que mainteneur, je veux que `pj.fa` et le `1` de `pj.ed` absent viennent de `pico8_pj_start`, afin de supprimer deux littéraux recopiés.
15. En tant que mainteneur, je veux que `g_pj_cr` prenne `hi`, `gq`, `ca` de `pico8_pj_cr_start`, afin que le `cr` recopié à la main disparaisse et que le `cr` du cart ait une seule source.
16. En tant que gardien du comportement, je veux que `pj.ou` et `pj.ea` restent des tables d'état partagées par tous les panneaux, réécrites en place par `hb`/`hk`, égales à l'init à ce qu'`ob()` produisait, afin que le panneau de score, le bandeau d'année et la boîte de `hk` ne changent pas.
17. En tant que gardien de la mémoire, je veux que `pj.ou`/`pj.ea` gardent la durée de vie d'aujourd'hui (construits à l'init, jamais libérés), afin que `sanitize-net` reste propre sans étude de qui les retient.
18. En tant qu'appelant de `pico8_X_ready()`, `pico8_pj_ou()`, `pico8_pj_ea()`, `pico8_pj_cr()` et `pico8_ie_eh()`, je veux que leurs contrats (« 1 de l'init à l'arrêt », « NULL avant l'init et après l'arrêt », « absent avant l'init ») restent vrais, afin que `pico8_cart.c`, `menu.c`, `score_panel_display.c` et les `assert` d'ordre d'init ne changent pas.
19. En tant que gardien du compteur, je veux que les lecteurs par nom supprimés n'aient aucun remplaçant par nom, ni renommé hors motif, ni `strcmp` non compté, afin que la baisse de `spec` ne cache pas un lecteur.
20. En tant que gardien du compteur, je veux que `pico8_rk_default` sorte du compte `spec` par l'exclusion que D.0 a déjà prévue pour ses six semblables, avec la preuve que ses lignes ne portent aucune autre catégorie, afin que `spec` puisse tomber à 0 à D.5c sans renommer une fonction de dessin.
21. En tant que gardien du compteur, je veux que ce changement de motif soit un commit à part, sans `src/`, au compte exact (−4 lignes, −4 occurrences), afin qu'on ne puisse pas le confondre avec la conversion.
22. En tant que vérificateur, je veux un test ponctuel qui compare chaque défaut généré à `pico8_ob()` sur la chaîne exacte de départ, clé par clé et dans les deux sens, afin de prouver qu'aucune clé, aucun genre, aucun bit n'a été perdu.
23. En tant que vérificateur, je veux que le test ponctuel prouve que la seconde parse de `ie` égale les trois clés de sa chaîne de classe, et que `hy`/`trade` parsent une table vide, afin que l'unification et l'absence d'objet soient justifiées.
24. En tant que gardien des TU, je veux que chaque lecture par nom d'un défaut du périmètre devienne une égalité directe avec la donnée générée, écrite à son site, afin que le test dise ce qu'il vérifie.
25. En tant que gardien des TU, je veux qu'aucun registre (switch, tableau, table ou boucle nommant ensemble les modules, leurs lecteurs ou les données attendues) ne remplace les lecteurs supprimés, afin que les tests ne recréent pas la couche qu'on retire.
26. En tant que gardien des TU, je veux que les vérités des trois constantes de `test_pico8_spec_defaults` (défauts servis, struct neuve qui les reprend, cycle prêt/arrêt) soient reprises, module par module, par des fonctions à leur nom qui passent aussi sur l'arbre de départ, afin que supprimer les constantes ne perde rien.
27. En tant que gardien des filets, je veux `make e2e` à 26 conformes, `make sanitize-net` à 26 propres et `make class-coverage-net` identique après le chunk 2, afin de prouver qu'aucun pixel, accès mémoire ni instanciation n'a changé.
28. En tant que gardien de la mémoire, je veux que D.5b n'ajoute aucune vue construite à l'appel ou à la frame, afin que la fuite par frame laissée par D.4b ne grandisse pas.
29. En tant que gardien du compteur, je veux les comptes avant/après en lignes ET en occurrences par catégorie, par fichier et par chunk, afin qu'une baisse prouve une construction retirée.
30. En tant que lecteur de `docs/glossaire-noms.md`, je veux les noms `pico8_X_start` des cinq classes, `pico8_pj_cr_start`, `pico8_pj_ou_row`, `pico8_pj_ea_row`, `g_pj_ou`/`g_pj_ea` et la mise à jour des lignes `jr`, `ou`, `ea`, `cr`, `gy`/`eh`, `id`, afin de retrouver un défaut depuis son nom de cart.
31. En tant que `plan-pilot`, je veux une note « Découvert à D.5b » qui donne le compte `spec` réel, l'explication du majorant, l'exclusion de `rk`, la forme de `pj.ou`/`ea` et les appelants restants de `pico8_ke_start_field`, afin que D.5c, D.6c et D.7a partent de faits.
32. En tant que `plan-pilot`, je veux que les spécifications de D.5c, les clés de construction (`overrides`, D.6c), les ponts et vues de D.2 à D.4b et les appels `pico8_pi_init()` (D.7a) restent hors de ces commits, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### `pico8_rk_default` : exclusion prouvée, commit à part (G)

- **Ce n'est pas un défaut de classe.** `pico8_rk_default` est l'appel `rk(127-h,127)` du cart, sans couleur (`pj:kj`), qui dessine avec la couleur de plume courante : la même sorte que `pico8_spr_default`, `pico8_dv_default`, `pico8_io_default`, `pico8_bl_default`… que D.0 a exclus parce que « les compter rendrait `spec` impossible à éteindre ». D.0 l'a manqué ; D.5a l'a relevé et renvoyé ici.
- **Décision : `rk` entre dans `SPEC_EXCLUSION`** (`s/pico8_(spr|fixed_rnd|dv|io|lq_ol|bl|rk)_default//g`), et le commentaire « Ces six _default » devient « Ces sept ». C'est un changement de motif : la règle « Motifs figés » de D.0 l'admet **avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale**. Preuve, à recopier au commit : `grep -rnE "$(pattern <cat>)" src tests --include='*.[ch]' | grep -c rk_default` vaut 0 pour chacune des huit autres catégories, et les lignes retirées du compte sont exactement `hud.c:63`, `hud.h:29`, `hud.h:34`, `score_panel.c:339` (−4 lignes, −4 occurrences, résidu D −4).
- **TDD** : `test_omitted_argument_defaults_are_not_class_specs` (`tests/test_data_residue.c`) gagne un appel `rk_default` coupé en deux littéraux comme les autres jetons de fixture, dans le cas « tout exclu » ; rouge avant le changement du script, vert après.
- **Rejetés** : **renommer `pico8_rk_default` hors motif** (`pico8_rk_pen`…) — seule des sept variantes à changer de nom, contre la convention du glossaire, et un renommage qui baisse un compte est précisément ce qu'« Anti-tassement » interdit ; **le laisser compté** — `spec` ne pourrait pas tomber à 0 à D.5c ; **élargir l'exclusion par un motif générique** (`_default\(gfx`) — une exclusion ne s'étend que nom par nom, avec preuve.
- **Seul changement de motif admis par ce PRD.** Toute autre modification de `tools/data_residue.sh` (motifs, catégories, modes) ou de `DATA_RESIDUE_GUARD` est un échec.

### Forme des défauts générés (G)

- **Un type et un objet par classe qui a des clés**, dans `pico8_game_data.h`/`.c` (module pur : ni `pico8_ob.h`, ni `pico8_value.h`) : `pico8_g__start_t`/`pico8_g__start`, `pico8_fv_start_t`/`pico8_fv_start`, `pico8_hp_start_t`/`pico8_hp_start`, `pico8_ie_start_t`/`pico8_ie_start`, `pico8_pj_start_t`/`pico8_pj_start`. Nom en `_start` (précédent D.5a) ; aucun ne correspond à un motif figé.
- **Champs** : exactement les clés de la spécification de la classe, **sans celles de la parente** (`fv` ne répète pas `nu`, `fx`, `dq`, `od`, `fp`, `dw` de `g_` ; aucune classe ne répète `me`/`t` de `ke`), sous le type que porte le membre : `pico8_fixed_t` (non-entier : l'hexadécimal signé des bits qu'`ob()` rendait, règle de D.3, jamais `PICO8_FIXED_LIT`) ; `const char *` ; `int` pour `false` (`g_.fp`, seule clé booléenne, H2) ; `pico8_vec_t`, `pico8_qz_box_t` **par valeur** ; pointeur `const` vers l'objet de D.4b (`nu`, `k_`, `rb`, `mr`, `gp`, `fk`, `gs`, `i_`) ou vers l'objet nouveau d'une table.
- **Objets nouveaux, un par table** (règle de D.4a) : `pj.cr` → `const pico8_pj_cr_start_t pico8_pj_cr_start` (`hi`, `gq` par valeur, `ca`) ; `pj.ea`, `pj.ou` → `const pico8_jb_row_t pico8_pj_ea_row`, `pico8_pj_ou_row` (rangées de rangées, la forme de `pico8_pj_gs_row` : items chaîne, entier, `is_fixed` pour 0.5, `is_false` pour `f`, `fn`), avec leurs tableaux de rangées internes.
- **`ie`** : **un seul `pico8_ie_start`** (`kq`, `fa`, `id`, `ra`, `eh`) pour la chaîne de classe **et** les objets que le module prête ; la seconde parse (`"\nkq=b(0,0,9,8),\nra=v(999,999),\neh=v(118,120),\n"`) disparaît, le test ponctuel prouve que ses trois clés égalent celles de la chaîne. Rejeté : deux objets (deux sources pour trois valeurs).
- **`hy`, `trade`** : spécification vide, **aucun objet** ; leur `init` ne parse plus rien.
- Rejetés : **une instance `static const` de la struct comme défaut** (archétype de la phase 2) ; **un tableau de paires nom/valeur** (la table ob sous un autre nom) ; **garder les statiques ou littéraux et générer à côté** (deux sources).

### Constructeurs et membres (G)

- **`g_`** : `pico8_g__struct_init` pose chaque défaut depuis `pico8_g__start` (`nu`, `k_` compris) ; les seize statiques `g_g__*`, `pico8_g__defaults_reset` et `g__default_string`/`_bool`/`_fixed` disparaissent ; `pico8_probe_init` ne garde que la remise du `fm` provisoire.
- **`fv`** : après `pico8_g__struct_init`, le constructeur pose les treize clés de `fv` qui surchargent `g_` et ses neuf propres depuis `pico8_fv_start` ; les vingt statiques `g_fv_*`, `pico8_fv_defaults_reset` et `help_default_fixed`/`_vec`/`_box` disparaissent. `pico8_help_init` garde ses deux `assert` d'ordre (`pico8_pi_init()` avant — `fv:kj` dispatche par la table de `pi` —, `pico8_g__ready()`), `g_bo` et la remise de `eu`.
- **`hp`** : le constructeur lit `pico8_hp_start.mr`/`gp`/`fk`.
- **`ie`** : `fa` depuis `pico8_ie_start.fa` ; `kq`, `ra`, `eh` pointent `&pico8_ie_start.kq`/`ra`/`eh` ; `g_ie_id` repart de `pico8_ie_start.id` à chaque init ; `pico8_ie_eh()` rend `&pico8_ie_start.eh` quand le module est prêt, REF nil sinon (contrat gardé) ; la boucle qui libère les objets à l'arrêt et `pico8_ie_spec_ref` disparaissent ; les `assert` de `ie:hb`/`ie:ldown` lisent le drapeau.
- **`pj`** :
  - `fa` depuis `pico8_pj_start.fa` ; `pico8_pj_ed` rend `pico8_pj_start.ed` quand `ed` est absent ; `gs`, `i_` depuis `pico8_pj_start`.
  - **`ou`, `ea` (défauts mutés)** : `pico8_score_panel_init` construit **une fois** `g_pj_ou = pico8_jb_row_view(pico8_pj_start.ou)` et `g_pj_ea = pico8_jb_row_view(pico8_pj_start.ea)` ; `pico8_pj_ou()`, `pico8_pj_ea()` et `pico8_pj_field` les rendent (NULL avant l'init et après l'arrêt, contrat gardé). **À l'arrêt, les pointeurs sont remis à NULL sans libération** : c'est la durée de vie d'aujourd'hui (la spécification était libérée superficiellement, ses tables `ou`/`ea` jamais), marquée `/* TROU-7 */` au site. Rejetés : **libérer la tête à l'arrêt** — plus court de vie qu'aujourd'hui, sans preuve que `mh` (construit sur `ea` par `hk`) ou un panneau ne la retient pas ; **un tableau fixe statique typé** (lecture littérale de la règle commune) — `hb`, `hk`, `kj`, `pico8_mh_new` et `score_panel_display` lisent une table ob ; les typer, c'est D.6b/D.7a, et l'intermédiaire exigerait une vue à chaque frame dans `kj` (interdit) ; **garder une parse `pico8_ob("o(…)")` de `ou`/`ea`** — une spécification partielle encore parsée.
  - **`cr`** : `g_pj_cr` reste le statique non `const` (défaut muté : `menu.c` écrit `ep`) ; `hi`/`gq` pointent `&pico8_pj_cr_start.hi`/`gq` (membres de `pico8_pj_cr_t` `const`), `ca` est posé depuis `pico8_pj_cr_start.ca` par `pico8_score_panel_init` (avec la remise de `ep`, qui y est déjà) ; `g_pj_cr_hi`/`g_pj_cr_gq` disparaissent. `pico8_pj_cr()` rend `&g_pj_cr` quand le module est prêt, NULL sinon.
  - `g_pj_defaults[]`, `pj_default_table`, `pico8_pj_defaults_reset` et la boucle d'init par nom disparaissent.
- **`hy`, `trade`** : `init`/`shutdown`/`ready` sur un drapeau, plus rien d'autre.
- **Vecteurs et boîtes partagés jamais écrits en place** (H3) : `fv.hi`, `fv.gq`, `fv.kq`, `g_.ra` (qui reçoit `&pico8_fv_start.ra` pour un `fv`, et des vecteurs du tas par `set_field "ra"`), `ie.kq`, `ie.ra`, `ie.eh`, `pico8_pj_cr_t.hi`/`gq` deviennent des pointeurs `const` ; les lectures par nom rendent `pico8_value_ref((void *)membre)` (cast de D.3). **Repli**, par défaut, si H3 tombe (une écriture en place l'atteint, ou `const` sort des fichiers du module, de `menu.c` et de leurs TU) : ce défaut garde un statique non `const` sans initialiseur, affecté depuis `pico8_X_start` à l'init du module (précédent `g_nf_ra`), nommé au commit.
- **Cycle de vie** (G) : `_init`/`_shutdown`/`_ready` restent pour les sept modules ; le pointeur de spécification devient un drapeau `static int` au nom hors motif (`g_X_up`, précédent D.5a). Rejeté : les supprimer (`pico8_cart.c`, `pico8_bootstrap.c`, les `assert` d'ordre et des TU les appellent).
- **`pico8_pi_init()`** : les appels de `pico8_score_panel_init` et `pico8_research_icon_init` **restent** (H9 ; `pi` est à D.7a) ; les commentaires d'en-tête qui les justifiaient par la spécification disent seulement le contrat.
- **Commentaires** : les contrats d'en-tête qui nomment la spécification ou un lecteur supprimé (« Parses `g_`'s spec… any REF previously handed out by pico8_g__default() », « Frees the `ie` spec and the module's `kq` box… », « A REF is borrowed from the spec… », « pj's class state, read once at init from its spec ») sont réécrits pour dire ce qui reste vrai ou supprimés avec leur code. Quatre sortes de `CLAUDE.md` seulement ; aucun renvoi à D.5b ni au plan dans `src/` ou `tests/`.

### Ce qui remplace les lecteurs par nom (G)

| Lecteur aujourd'hui | Après D.5b | Pourquoi |
|---|---|---|
| `pico8_g__default`, `pico8_fv_default`, `pico8_hp_default`, `pico8_ie_default`, `pico8_pj_default`, `pico8_hy_default`, `pico8_trade_default` | **supprimés** ; aucun remplaçant par nom | leurs lecteurs de `src/` (inits de `g_`, `fv`, `pj` ; chaîne de `fv`) lisent désormais `pico8_X_start` ; le reste était des TU |
| `pico8_ie_spec_ref` (constructeur, `pico8_ie_eh`) | adresse du champ de `pico8_ie_start` | recherche par nom dans une table interne, sans lecteur externe |
| lectures de ces défauts dans les TU | égalité directe avec `pico8_X_start.<clé>` (ou l'objet de D.4b, `pico8_pj_ou()`, `pico8_pj_ea()`), écrite à son site | règle des TU plus bas |
| `pico8_ke_start_field` au bout de `pico8_g__default`, `pico8_pj_default`, `pico8_ie_default` | ces trois fins de chaîne disparaissent avec leur lecteur ; **`pico8_ke_start_field` reste** pour `pico8_jo_default` (`planet.c`) et le cas `M_KE` | D.5c le supprime (note de D.5a) |

- **Rejeté, critère d'échec** : renommer un `pico8_X_default` hors motif en gardant la recherche par nom, ou le remplacer par une suite de `strcmp` non comptée, dans `src/` ou dans un TU.

### Fuite par frame de D.4b (`help.c:90`, `score_panel.c:317`) : non fermée ici (G)

- **Décision** : D.5b ne la ferme pas et ne l'aggrave pas. Les deux sites restent tels quels ; les deux vues nouvelles de `pj.ou`/`ea` sont construites **une fois par init**, jamais à l'appel ni à la frame.
- **Pourquoi pas « à bon compte »** : la fermer demande une libération profonde des vues (une fonction nouvelle de la couche ob, lignes `ob` en plus, dans une étape dont la mesure est `spec`), la preuve que `pico8_bl_default` ne retient aucune rangée, et ne fermerait que deux des six vues par appel laissées par D.4b (`jo:gh`, `jo.ex`, `hk` resteraient) ; D.7a supprime ces vues elles-mêmes (`bl` sur des rangées typées), ce qui rendrait la libération profonde jetable dans quelques étapes. Rejeté : **fermer les deux sites du périmètre** (gain partiel, code jeté par D.7a).

### Le générateur (G) : jetable, en C, sur l'arbre de départ

- **Programme C de l'implémenteur, dans son scratchpad, jamais versionné** (précédent D.5a), compilé contre `$(DATA_ENGINE_SRCS)` de `f42691f`, **avant** toute modification : `pico8_pi_init()`, puis `pico8_ob(s, NULL)` sur les huit chaînes **copiées octet pour octet** des `init` (`g_`, `fv`, `hp`, la chaîne de classe de `ie`, la seconde parse de `ie`, `pj`, `hy`, `trade`). Il émet l'initialiseur désigné de chaque `pico8_X_start`, de `pico8_pj_cr_start`, `pico8_pj_ea_row`, `pico8_pj_ou_row` et de leurs rangées internes, un champ par ligne.
- **Il échoue fort** si une hypothèse de forme tombe (H2) : clés en nombre ou en ordre autres que ceux de l'inventaire ; genre hors {nombre, chaîne, `false`, vecteur, boîte, table déjà générée par D.4b, rangée de rangées, enregistrement `cr` à deux vecteurs et un nombre} ; table déjà générée dont le contenu diffère de l'objet existant ; `false` ailleurs que `g_.fp` ou qu'aux items de `ou` ; seconde parse de `ie` différente de la chaîne ; `hy`/`trade` non vides.
- Sortie collée dans `pico8_game_data.c`, mise en forme à la main (≤ 120 colonnes, **une déclaration par ligne, un champ par ligne**, comme les objets de D.4a à D.5a), sans valeur éditée. Le message de commit décrit le générateur en ≤ 15 lignes.

### `Makefile`, docs

- **Le `Makefile` ne change pas** : aucun fichier nouveau. `DATA_RESIDUE_GUARD` reste vide (aucune catégorie ne tombe à 0).
- `docs/glossaire-noms.md` (chunk 2) : lignes `pico8_X_start` (étendue à `g_`, `fv`, `hp`, `ie`, `pj` ; `hy`, `trade` à spécification vide), `pico8_pj_cr_start`, `pico8_pj_ou_row`/`pico8_pj_ea_row` et `g_pj_ou`/`g_pj_ea` ; lignes `jr` (seul l'arbre `jo` garde une spécification parsée), `ea`, `i_`/`ou`, `cr`, `gy`/`eh`, `id` (sur `ie`) mises à jour.
- `docs/plan-3.md` : note « Découvert à D.5b » en fin de ligne D.5b, **au chunk 2** (texte dans Further Notes). Rien d'autre dans le plan.

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucune condition de jeu, aucun tirage du RNG ; aucune valeur de défaut (le test ponctuel le prouve au bit près).
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements ; aucun e2e ajouté.
- Les spécifications de D.5c (arbre `jo`), `pico8_ke_start_field` et le cas `M_KE`.
- Les clés de construction (`overrides`) de ces classes (D.6c) ; le paramètre `overrides` n'est renommé nulle part.
- Les appels `pico8_pi_init()` (D.7a), `g_bo`/`g_bz` et les ponts et vues de D.2 à D.4b, dont les deux vues par frame (`help.c:90`, `score_panel.c:317`).
- `pico8_ob.c/.h`, `pico8_stash.*`, les motifs de `tools/data_residue.sh` hors l'ajout de `rk` à `SPEC_EXCLUSION`, `DATA_RESIDUE_GUARD`, `tools/class_coverage_*`, `KNOWN[]`.
- `gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`.
- **TROU** : aucun écart port/cart touché ni ajouté ; TROU-7 préservé (fuites par frame de D.4b, tables `ou`/`ea` jamais libérées, chaînes que `hb`/`hk` abandonnent).

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec, quelle que soit la catégorie.

- **Interdit** : joindre sur une ligne des déclarations, instructions, arguments, conditions, entrées d'initialiseur ou `case` qui étaient sur des lignes distinctes ; retirer des retours à la ligne dans une ligne comptée ; déplacer une construction comptée sur une ligne comptée existante ; macro, `typedef` ou fonction enveloppe dont le seul rôle est de renommer un identifiant compté ; **renommer un lecteur par nom hors motif** ou le remplacer par une recherche `strcmp` non comptée ; **un registre dans un TU** (tableau, table, `switch` ou boucle qui nomme ensemble modules, lecteurs ou données attendues) ; déplacer du code compté vers un fichier que le compteur ne lit pas ; supprimer un commentaire qui ne porte pas sur une spécification, un lecteur, une aide ou un contrat retiré par D.5b ; **changer un motif** au-delà de l'ajout de `rk` à `SPEC_EXCLUSION` (chunk 1).
- **Mesure exigée, avant et après chaque chunk** : par catégorie, **lignes** (`make data-residue`, et `--detail` par fichier) **ET occurrences** (recette ci-dessous, qui lit l'exclusion dans le script pour rester juste après le chunk 1), le test ponctuel donné à part. Recette figée pour l'implémenteur et le vérificateur :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  X=$(sed -n "s/^SPEC_EXCLUSION='\(.*\)'\$/\1/p" tools/data_residue.sh)
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E "$X" "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  ```

  Départ (`f42691f`) : `valeur` 6809, `ob` 2185 (dont 141 dans le test ponctuel), `stash` 28, `type-tag` 700, `ref-audit` 70, `par-nom` 1145, `dispatch` 199, `overrides` 479, `spec` 684.
- **Contrôle** : pour chaque catégorie et chaque fichier du diff, Δlignes = avant − après et Δoccurrences = avant − après. **Si Δlignes > Δoccurrences** dans un couple (catégorie, fichier), le vérificateur lit le diff de ce fichier : chaque ligne comptée retirée doit correspondre à une construction retirée ou convertie, pas à une construction qui réapparaît sur une autre ligne. Le message de commit liste ces couples et leur justification.
- **Les lignes qui gagnent une catégorie** sont listées au commit : attendues, `ob` +2 (`g_pj_ou`/`g_pj_ea`, déclarés `pico8_ob_table_t *`) ; toute autre ligne nouvelle comptée est justifiée.

### Empreinte remesurée

| Chunk | `src/` | `tests/` | Autres | Total |
|---|---|---|---|---|
| 1 — exclusion de `rk` | 0 | 1 : `test_data_residue` | `tools/data_residue.sh` | **2** |
| 2 — sept classes | 16 : `game_data.c/.h`, `probe.c/.h`, `help.c/.h`, `research.c/.h`, `research_icon.c/.h`, `score_panel.c/.h`, `economy.c/.h`, `trade.c/.h` (+ `menu.c` si `const` y descend, H3) | ≈ 9 : `test_pico8_map_tables`, `spec_defaults`, `jo_spec_defaults`, `js`, `fv`, `g_`, `hp`, `pj`, et `ie` pour l'assertion directe de `pico8_ie_start` | glossaire, plan | **≈ 27** |
| **Union** | **16** (+1) | **≈ 10** | 3 | **≈ 29** |

- **Écart avec la ligne du plan (23 : 16, 7)** : `src/` est exact ; le plan ne voyait ni `pico8_game_data.c/.h` (où naissent les données : ils remplacent, dans le compte, `hud.c/.h`, qui ne changent pas), ni `test_pico8_map_tables` et `test_pico8_js`/`test_pico8_jo_spec_defaults` (une ligne chacun), ni l'outil et son test (chunk 1).
- **Découpage** (G) : l'union est sous le seuil de ~40 ; la conversion reste **un seul commit**, parce que le registre de `test_pico8_spec_defaults` ne tient plus que ces sept modules — le couper en deux obligerait à reprendre ses trois constantes (sonde) pour un état intermédiaire que le commit suivant supprime. Le changement de motif est **un commit à part**, parce qu'il est d'une autre nature (outil, pas conversion) et que son compte doit être lisible seul. Rejetés : **tout en un** (le −4 de l'exclusion se noierait dans −200) ; **`g_`/`fv` puis le reste** (reprise des constantes pour rien).

### Effet sur `make data-residue`

Attendu (H17), lignes / occurrences :

| Catégorie | Départ | Après chunk 1 | Après chunk 2 | Pourquoi |
|---|---|---|---|---|
| `spec` | 634 ; 684 | **630 ; 680** | **430 ; 470** | chunk 1 : les quatre lignes de `pico8_rk_default` ; chunk 2 : −174/−182 dans les modules, −26/−28 dans les TU (inventaire) |
| `par-nom` | 1090 ; 1145 | **1090** (exact) | **1085 ; 1140** (écart ≤ 3 expliqué) | les trois fins de chaîne `pico8_ke_start_field` (`probe.c:857`, `score_panel.c:520`, `research_icon.c:497`) et deux lignes du registre (`test_pico8_spec_defaults` l. 375, 383) ; les assertions directes lisent des membres |
| `ob` | 1820 hors test (1954) ; 2044 hors test (2185) | **inchangé** | **< 1820 hors test**, attendu ≈ −45, rapporté | pointeurs et parses de spécification, libérations, `get_name`, `pj_default_table`, objets de `ie`, registre et `tables_same`/`row_equal` de `test_pico8_spec_defaults` ; +2 déclarations `g_pj_ou`/`g_pj_ea` ; le test ponctuel grandit (parses de référence) |
| `stash` | 28/4 ; 28 | **28/4** | **28/4** | — |
| `overrides` | 450 ; 479 | **450** | **450** (exact) | aucune ligne `overrides` touchée |
| `dispatch` | 197 ; 199 | **197** | **197** (exact) | — |
| `type-tag` | 431 ; 700 | inchangé | rapporté, attendu en baisse | `assert` de `help_default_vec`/`_box` et de la boucle d'init de `pj`, `fold_value` du registre |
| `ref-audit` | 69 ; 70 | inchangé | **≤ 69** | — |
| `valeur` | 5892 ; 6809 | inchangé | rapporté, attendu en baisse | signatures `pico8_value_t *out_value` des lecteurs supprimés, `g_pj_defaults[]` |
| **résidu D** | 9063 | **9059** (exact) | rapporté | — |

- **`spec` = 430 est attendu exact** : écart ≤ 6 **expliqué** au commit ; au-dessus de 436, ou toute baisse obtenue par tassement : échec. Le majorant brut de la ligne (≤ 547) est tenu ; le majorant décalé (≤ 402) ne l'est pas, pour la raison mesurée plus haut (28 lignes comptées deux fois), que la note corrige.
- Aucune catégorie ne tombe à 0 : **`DATA_RESIDUE_GUARD` reste vide.**

### Hypothèses (rien n'a été lancé)

- **H1** : `f42691f` est vert (`make test`, `make e2e` 26 conformes, `make sanitize-net` 26 propres, `make class-coverage-net` identique, `make data-guard` 0) et `make data-residue` égale l'inventaire. Démenti au départ : arrêt, D.5b ne répare rien.
- **H2** : ce qu'`ob()` produit a la forme de l'inventaire (genres et ordre des clés ; `g_.fp` et les `f` de `ou` seuls `false` ; `nu`, `k_`, `rb`, `mr`, `gp`, `fk`, `gs`, `i_` de même contenu que les objets de D.4b ; `ou`/`ea` rangées de rangées que `pico8_jb_row_t` tient, `fn` compris ; `cr` à deux vecteurs et un nombre ; seconde parse de `ie` égale à la chaîne sur `kq`/`ra`/`eh` ; `hy`/`trade` vides). Le générateur le vérifie. Démentie : le type suit ce qu'`ob()` produit, le test ponctuel juge ; arrêt seulement si une clé ne tombe sous aucune forme.
- **H3** : aucune écriture en place (dans `src/` ni dans un TU) n'atteint `fv.hi`/`gq`/`kq`, le `ra` d'un `fv`, `ie.kq`/`ra`/`eh`, `pj.cr.hi`/`gq`, ni ce que leurs lecteurs par nom rendent (`pz` de l'aide et de `cr`, copies de `msgbox`, `ie.gy` qui tient `eh`) ; le passage à `const` ne sort pas de `probe`, `help`, `research_icon`, `score_panel`, `menu` et de leurs TU. Démentie : repli « statique non `const` affecté à l'init » pour ce défaut.
- **H4** : aucun lecteur n'atteint un défaut du périmètre avant l'init de son module ; le drapeau « prêt » reproduit le contrat du pointeur de spécification ; `pico8_pj_ou()`/`ea()`/`cr()` et `pico8_ie_eh()` gardent leurs réponses avant l'init et après l'arrêt (`test_pico8_pj:682` le vérifie pour `cr`).
- **H5** : les seuls lecteurs par nom des défauts du périmètre sont ceux de l'inventaire ; aucune aide à nom non littéral (`field_fixed(inst, name)`…) ne lit une spécification.
- **H6** : les tables que `pico8_jb_row_view` construit depuis `pico8_pj_ou_row`/`pico8_pj_ea_row` sont, entrée par entrée, celles qu'`ob()` produisait (indices, `false`, bits de 0.5, `fn` après les items, chaîne vide) ; les chaînes de la vue sont des données générées que `hb`/`hk` remplacent sans jamais écrire à travers ni libérer.
- **H7** : durée de vie : aujourd'hui les tables `ou`/`ea` et les vecteurs de `fv` ne sont jamais libérés (libération superficielle), les objets de `ie` le sont à l'arrêt ; après D.5b, rien de tout cela n'est libéré ; `make sanitize-net` rend 26 propres (`detect_leaks=0`).
- **H8** : constantes de hachage : seules `READY_HASH`, `SPEC_HASH`, `STRUCT_HASH` de `test_pico8_spec_defaults` disparaissent (plus aucun module dans le registre) ; aucune constante des TU `fv`, `g_`, `pj`, `ie`, `hp`, `hy`, `trade`, `ui`, `entity_kind`, `jo_spec_defaults` ne bouge (ils plient des valeurs, pas l'identité avec la spécification). Toute autre constante qui bouge est un arrêt.
- **H9** : les appels `pico8_pi_init()` de `score_panel` et `research_icon` servent encore (`bl` de `kj` dispatche vers `br`/`qr`/`rf`/`s` que `pi` enregistre) ; ils restent, comme l'`assert` de `pico8_help_init`.
- **H10** : les bits émis pour les non-entiers de `g_`, `fv` et `pj.ea` égalent ceux des statiques d'aujourd'hui, remplis par la même parse : aucun changement possible, le pli des TU `g_`/`fv` le confirme.
- **H11** : `hy` et `trade` n'ont aucun lecteur de leur spécification hors du registre.
- **H12** : `g_ie_id` part aujourd'hui du fixe 0, la valeur de `id` dans la chaîne ; `ie.fa` = 12 est la valeur de la chaîne.
- **H13** : `make class-coverage-net` est identique : aucune instanciation ajoutée ni retirée (les fonctions nouvelles de `test_pico8_spec_defaults` construisent les mêmes structs que `build()` hier, et les TU ne comptent pas).
- **H14** : `-Werror` reste muet : aides, statiques et inclusions devenus sans usage supprimés ; casts `(void *)` sur données `const` selon D.3.
- **H15** : `test_data_residue` rougit avec la fixture `rk_default` avant le changement du script, et le script modifié garde tous ses autres cas verts.
- **H16** : `constants-check`, `float-grep`, `float-guard` restent verts : aucun `PICO8_FIXED_LIT` ni flottant ajouté.
- **H17** : `make data-residue` rend les comptes du tableau ; `ob` exact dépend de l'écriture.
- **H18** : durées courtes : `make test` en quelques secondes plus une recompilation large (`game_data.h`), `make e2e` ~1 s, `sanitize-net` et `class-coverage-net` < 2 min chacun, compilation comprise.
- **H19** : `make raylib` (non exigé, jamais lancé) lierait : aucun fichier nouveau.

## Implementation Chunks

Deux commits, dans cet ordre ; chacun est vert seul, porte ses comptes avant/après en lignes ET en occurrences, et ses preuves. Les cibles finales (« Sortie de l'étape ») ne s'appliquent qu'au dernier.

1. **D.5b-1 — `pico8_rk_default` hors du motif `spec`.** Fixture `rk_default` dans `test_omitted_argument_defaults_are_not_class_specs` (rouge), puis `rk` dans `SPEC_EXCLUSION` et « six » → « sept » dans son commentaire (vert). Aucun fichier de `src/`. Preuve au commit : le `grep` des huit autres catégories sur `rk_default` (0 chacune), les quatre lignes retirées du compte. Cibles : `spec` 630/680, résidu D 9059, toutes les autres catégories inchangées au chiffre près. Filet : `make test` seulement (ni `e2e`, ni `sanitize-net`, ni `class-coverage-net` : aucun binaire ne change).
2. **D.5b-2 — `g_`, `fv`, `hp`, `ie`, `pj`, `hy`, `trade`.** Générateur ; types et objets `pico8_X_start`, `pico8_pj_cr_start`, `pico8_pj_ea_row`, `pico8_pj_ou_row` ; test ponctuel ; constructeurs ; `g_pj_ou`/`g_pj_ea` construits à l'init ; membres `const` (ou repli) ; suppression des spécifications, des sept `pico8_X_default`, des remises, aides et statiques recopiés, de `pico8_ie_spec_ref` et de la libération des objets de `ie` ; drapeaux « prêt » ; TU convertis ; `test_pico8_spec_defaults` sans registre ; glossaire ; note « Découvert à D.5b ». Dépend du chunk 1 seulement pour le compte `spec` de départ (630). Cibles : `spec` 430, `par-nom` 1085, `ob` hors test < 1820.

**Repli pour le pilote** : si le chunk 2 dépasse 40 chemins à l'implémentation, `plan-pilot` le recoupe en 2a (`g_`, `fv`, `hp`, `hy`, `trade`, `ie`) et 2b (`pj`) ; 2a reprend alors les trois constantes de `test_pico8_spec_defaults` sur le seul module restant, avec une sonde (l'ancien pli sur les sept modules, où les six retirés lisent une reconstitution de l'ancien lecteur depuis les données générées, redonne exactement `0x05313C64`, `0x067B24A6`, `0x1A95E7B2`), et 2b les supprime.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, identités, pixels), pas la présence d'une ligne. D.5b change la source des défauts de sept classes : **la preuve est que les données générées égalent ce qu'`ob()` produisait des chaînes de départ, que chaque struct neuve et chaque table d'état reprennent exactement ces données, que chaque TU qui lisait un défaut par nom fige la même vérité sur la donnée générée, et que tout ce qui mesure le comportement rend la même chose.**

### Chunk 1 : `test_data_residue`

- Rouge d'abord : la fixture `"pico8_" "rk_default(gfx, mem, y1, y2);\n"` ajoutée au cas « tout exclu » fait sortir `spec\t1\t1` au lieu du pied vide ; vert après l'ajout de `rk`. Le cas mixte (`jo_default` et `default_fixed` comptés) reste tel quel.

### Le test ponctuel `tests/test_pico8_map_tables.c`, étendu (G)

- **Il construit son état, il ne rejoue rien.** Ses 2 lignes `stash` ne changent pas ; il ne nomme aucun motif de `spec`, d'`overrides` ni de `type-tag`.
- **Chaînes de référence** : `G__SRC`, `FV_SRC`, `HP_SRC`, `PJ_SRC` de D.4b sont vérifiées octet pour octet contre les `init` de `f42691f` (corrigées si elles divergent, au commit) ; s'ajoutent la chaîne de classe de `ie`, sa seconde parse, `HY_SRC` et `TRADE_SRC` (vides).
- **Pour chaque classe, dans les deux sens** : chaque clé de la table parsée a son champ dans `pico8_X_start` (ou l'objet qu'il pointe) et lui est égale — nombre au bit près, chaîne par `strcmp`, `false` par genre et valeur, vecteur et boîte par composante, table déjà générée par identité de pointeur plus contenu, `cr` champ par champ, `ou`/`ea` par `check_row` (déjà présent) ; et le nombre de clés parsées est le nombre de champs vérifiés. `ie` : la seconde parse a exactement trois clés, égales à `pico8_ie_start.kq`/`ra`/`eh`. `hy`, `trade` : table vide. Les vérifications de D.4b sur `nu`, `k_`, `rb`, `mr`, `gp`, `fk`, `gs`, `i_` continuent.

### TU touchés : lire la donnée générée, comparer directement (G)

- **Règle** : chaque lecture par nom d'un défaut du périmètre devient une égalité directe avec la donnée générée, écrite à son site, sans registre ; une boucle sur des noms qui appelait un lecteur par nom (`fold_fresh_probe`, `fold_fresh_fv` de `test_pico8_g_`, les boucles `tables[]` de `test_pico8_fv`, `spec[]` de `test_pico8_pj`) devient une suite d'assertions sur les membres, une par ligne.
- **`test_pico8_g_`** : `class_value` supprimé ; l. 402 compare `dw` à `pico8_g__start.dw` ; `fold_fresh_probe` asserte les seize scalaires sur `pico8_g__start` (hors `pn`/`du`/`py` quand la sonde est construite avec des clés) ; `fold_fresh_fv` les treize surcharges sur `pico8_fv_start`, les cinq autres sur `pico8_g__start`, `ra` **est** `&pico8_fv_start.ra` (hier : le vecteur de la spécification) ; l. 792 (« la spécification n'a pas de méthode ») disparaît : le test ponctuel porte le jeu de clés exact.
- **`test_pico8_fv`** : `class_ref` supprimé ; l. 354 et 714 : `ra`, `hi`, `gq`, `kq` **sont** les champs de `pico8_fv_start` (identité inversée et dite, hier « les pointeurs de la spécification ») ; l. 627-628 et 895 (méthodes absentes de la spécification) disparaissent, portées par le test ponctuel ; le cycle prêt (`pico8_fv_ready()`, arrêt, `!ready`, init) y est asserté.
- **`test_pico8_pj`** : `class_ref` supprimé ; l. 360, 368, 636 : `ea`/`ou` lus sur le panneau **sont** `pico8_pj_ea()`/`pico8_pj_ou()`, et une fonction nouvelle vérifie, avant tout `hb`/`hk`, que ces tables égalent `pico8_pj_ea_row`/`pico8_pj_ou_row` (`pico8_jb_row_view` comparée entrée par entrée, vue libérée dans le test) ; `cr->hi`/`gq` **sont** `&pico8_pj_cr_start.hi`/`gq`, `cr->ca` égale `pico8_pj_cr_start.ca` ; `fa` et `ed` absent égalent `pico8_pj_start.fa`/`ed` ; l. 631-634 (boucles sur les méthodes et les clés de la spécification) disparaissent, portées par le test ponctuel.
- **`test_pico8_hp`** : l. 727 disparaît (le jeu de clés est au test ponctuel) ; `mr`/`gp`/`fk` d'un `hp` neuf **sont** `pico8_hp_start.mr`/`gp`/`fk`.
- **`test_pico8_ie`** : `check_fresh_icon` asserte `kq`/`ra`/`eh` **sont** les champs de `pico8_ie_start` et `fa` égale `pico8_ie_start.fa` (les valeurs figées 12, 999, 118/120 restent) ; son commentaire « off the spec » est réécrit.
- **`test_pico8_js`** (l. 1396) : compare à `pico8_fv_start.ca`. **`test_pico8_jo_spec_defaults`** (l. 461) : la ligne disparaît ; ce qu'elle disait (`g_` hérite du `me` de `ke`) est porté par la fonction `g_` de `test_pico8_spec_defaults` ; ses constantes ne changent pas (le cas `g_` n'était pas plié).
- **`test_pico8_spec_defaults`** : l'enum, `spec_field`, `is_ready`, `build`, `shutdown_of`, `NAMES`, `fold_value`, `same_value`, `same_default`, `tags_equal`, `ints_equal`, `tables_same`, `row_equal`, `fold_ready`, `test_spec_defaults`, `test_struct_defaults` et les trois constantes disparaissent. **Une fonction à son nom par module** (`g_`, `fv`, `hp`, `ie`, `pj`, `hy`, `trade`), sans boucle ni table de modules, asserte : la struct neuve (`pico8_X_new(NULL)`, comme `build()` hier) pose chaque défaut égal au champ de `pico8_X_start` ou pointe la donnée, `me`/`t` égaux à `pico8_ke_start` ; son cycle prêt (1 après `pico8_cart_load()`/`pico8_jl()`, 0 après son arrêt). `main` asserte, avant `pico8_cart_load()`, que chacun des sept est non prêt (une assertion par module, écrite). `test_second_jl_keeps_specs` et les fonctions de D.5a restent. En-tête réécrit.
- **Sonde de la suppression des trois constantes** (au lieu d'une reprise) : les sept fonctions nouvelles sont écrites **après le générateur et les données, avant le passage des constructeurs et la suppression des lecteurs**, et passent sur cet arbre intermédiaire **en même temps que l'ancien registre** (trois constantes inchangées) ; le rapport de `make test` sur cet état figure au commit. Ce qu'elles ne portent pas (valeurs par nom des clés absentes) est porté par le jeu de clés exact du test ponctuel.

### Preuve des tests (§7, gcov + mutations)

- **gcov** (copie jetable du scratchpad, `--coverage`) au chunk 2 : chaque ligne ajoutée ou modifiée de `game_data.c`, des constructeurs et des `init`/`shutdown` du périmètre est exécutée par un TU ; les lignes atteintes seulement par les e2e sont justifiées au commit.
- **Mutations**, chacune doit rougir un test, puis être annulée :
  - chunk 1 — **M0** : `rk` retiré de `SPEC_EXCLUSION` → `test_data_residue` rouge (c'est son rouge TDD) ;
  - chunk 2 — **M1** : un bit de `pico8_g__start.dw` changé → test ponctuel rouge ; **M2** : le constructeur de `fv` prend `eo` dans `pico8_fv_start.ff` (−1 → 1) → `test_pico8_fv` ou la fonction `fv` de `test_pico8_spec_defaults` rouge ; **M3** : `ie.ra` pointe `&pico8_ie_start.eh` → `test_pico8_ie` / `test_pico8_entity_kind` rouge ; **M4** : `g_pj_ea` construit depuis `pico8_pj_ou_row` → `test_pico8_pj` rouge ; **M5** : `pico8_pj_ed` rend 0 quand `ed` est absent → `test_pico8_pj` rouge ; **M6** : `g_ie_id` repart de 1 → `test_pico8_ie` ou `test_pico8_entity_kind` rouge ; **M7** : `pico8_hp_start.gp` pointe `pico8_hp_mr_names` → test ponctuel ou `test_pico8_hp` rouge.
- Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée.

### Pas de nouvel e2e (G)

Les lignes changées sont atteintes par les 26 e2e et par les TU ; un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 2 minutes pour le chunk 1, 12 minutes pour le chunk 2, 15 minutes pour l'étape.**

| Cible | Durée attendue |
|---|---|
| chunk 1 : `make test`, `make data-residue`, recette | < 1 min |
| générateur (scratchpad, arbre de départ) | < 1 min, compilation comprise |
| `make test` (×3 au chunk 2 : données + TU nouveaux ; constructeurs ; TU convertis) | quelques secondes chacun, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H18) |
| `make class-coverage-net` | < 2 min (H18) |
| gcov + mutations (scratchpad) | < 4 min |
| `make data-guard`, `make data-residue`, recette d'occurrences | < 1 s |

Ordre conseillé au chunk 2 : mesure de départ (lignes et occurrences) ; générateur **avant toute modification** ; types, données, test ponctuel, fonctions nouvelles de `test_pico8_spec_defaults`, puis `make test` (ancien registre vert à côté) ; constructeurs, suppressions, puis `make test` ; TU convertis, registre retiré, puis `make test` ; `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois ; gcov, mutations ; mesure d'arrivée, contrôle anti-tassement, gardes, glossaire, note du plan.

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + chunk + « données » / « constructeurs » / « TU »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales de chaque chunk**, vertes sur l'arbre du chunk :

- chunk 1 : `timeout 5m make test` ; `make data-guard` : code 0 ; `make data-residue` + recette : `spec` 630/680, résidu D 9059, toute autre catégorie identique au départ en lignes et en occurrences.
- chunk 2 : `timeout 5m make test` ; `timeout 5m make e2e` : **26 « conforme »** ; `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ; `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ; `make data-guard` : code 0 ; `make data-residue` + recette : `spec` 430, `par-nom` 1085 (écarts bornés et expliqués), `stash` 28/4, `overrides` 450, `dispatch` 197, `ob` hors test ponctuel < 1820 ; contrôle anti-tassement passé.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque chunk liste les chemins de sa ligne « Empreinte remesurée » (écart ≤ 3 expliqué), rien d'autre. Chunk 1 : aucun fichier de `src/`. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements, `Makefile` inchangé, `gh_row_lq`/`ga_export_panel_by`/`layout_nil_item_draws_nothing` intacts.
2. **Motif** : le diff de `tools/data_residue.sh` est l'ajout de `|rk` dans `SPEC_EXCLUSION` et « six » → « sept » dans son commentaire, rien d'autre ; la preuve de grep est au commit.
3. **Données** : un `pico8_X_start` par classe à clés, `pico8_pj_cr_start`, `pico8_pj_ea_row`, `pico8_pj_ou_row`, dans `pico8_game_data.c`, initialiseur désigné, un champ par ligne, non-entiers en hexadécimal signé ; `pico8_game_data.*` n'inclut ni `pico8_ob.h`, ni `pico8_value.h`.
4. **Une seule source** : aucun constructeur ni `init` du périmètre ne pose plus un littéral ou un statique qui duplique une clé de sa classe (`g_pj_cr` compris) ; `grep` de la « Sortie de l'étape » vide.
5. **Lecteurs par nom** : aucun lecteur par nom des défauts du périmètre ne subsiste sous un autre nom ; `pico8_ke_start_field` n'a plus que `planet.c` et `M_KE`.
6. **Mémoire** : aucune donnée générée libérée ; `pico8_jb_row_view` : sites de `f42691f` plus les deux de `pico8_score_panel_init`, aucun à l'appel ni à la frame ; `g_pj_ou`/`g_pj_ea` jamais libérés, comme les tables de la spécification ; aucune `pico8_ob_table_new` ajoutée.
7. **Anti-tassement** : les comptes avant/après, en lignes ET en occurrences, par catégorie (et par fichier pour `--detail`), figurent à chaque commit et sont recalculés par le vérificateur avec la recette ; chaque couple où Δlignes > Δoccurrences est justifié et relu ; aucune jonction de lignes, aucun alias d'identifiant compté, aucun lecteur renommé, aucun déplacement hors du compteur, aucun commentaire sans rapport supprimé.
8. **TU** : plus aucune lecture par nom d'un défaut du périmètre ; les comparaisons avec les données générées sont écrites à leur site, sans registre ; les trois constantes supprimées le sont avec la sonde « ancien registre vert à côté des fonctions nouvelles » ; aucune autre constante ne bouge.
9. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun récit de chantier.
10. **Docs** : glossaire à jour au chunk 2 ; note « Découvert à D.5b » en fin de ligne D.5b au chunk 2, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `f42691f` (ou un commit de docs au-dessus, ou le chunk précédent) propre, ou si `make data-residue` n'y rend pas l'inventaire.
- `FAILED:` si le filet (`make test`, et au chunk 2 `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence.

**Périmètre**

- `FAILED:` si une spécification du périmètre est encore parsée, si un `pico8_X_default`, `*_defaults_reset`, aide `*_default_<genre>`, `pico8_ie_spec_ref` ou statique recopié du périmètre subsiste au chunk 2 (hors repli de H3 nommé au commit), ou si une ligne des `grep` de la « Sortie de l'étape » subsiste.
- `FAILED:` si une valeur générée a été écrite ou corrigée à la main, si un générateur est versionné, ou si `pico8_game_data.*` inclut `pico8_ob.h` ou `pico8_value.h`.
- `FAILED:` si un constructeur ou un `init` du périmètre garde un littéral ou un statique qui duplique une clé de sa classe.
- `FAILED:` si le diff convertit une spécification de D.5c, supprime `pico8_ke_start_field`, convertit une clé de construction, un appel `pico8_pi_init()`, `g_bo`/`g_bz`, un pont ou une vue de D.2 à D.4b, ferme ou modifie les vues de `help.c:90`/`score_panel.c:317`, ou touche un TROU.
- `FAILED:` si le diff ajoute un appel à `pico8_jb_row_view` ailleurs que les deux de `pico8_score_panel_init`, ou une table ob construite à chaque appel ou à chaque frame.
- `FAILED:` si le chunk 1 touche un fichier de `src/`, ou si `tools/data_residue.sh` change au-delà de `rk` dans `SPEC_EXCLUSION` et de son commentaire, ou si la preuve de grep manque au commit.
- `FAILED:` si le test ponctuel nomme un motif de `spec`, d'`overrides` ou de `type-tag`, ou porte plus de 2 lignes `stash`.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si le chunk 1 ne rend pas exactement `spec` 630 lignes / 680 occurrences et résidu D 9059, toute autre catégorie inchangée.
- `FAILED:` si le chunk 2 ne rend pas `stash` 28/4, `overrides` 450, `dispatch` 197 ; si `spec` dépasse 436 ou s'écarte de 430 sans explication ; si `par-nom` s'écarte de 1085 de plus de 3 ou sans explication ; si `ob` hors test ponctuel ne baisse pas strictement.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés dans une ligne comptée, construction comptée déplacée sur une autre ligne comptée, alias (macro, `typedef`, enveloppe) d'un identifiant compté, code déplacé hors de `src`/`tests` ou hors `.c`/`.h`, commentaire sans rapport supprimé.
- `FAILED:` si un lecteur par nom d'un défaut du périmètre survit sous un nom qu'aucun motif ne compte (renommage, recherche `strcmp` non comptée), dans `src/` ou dans un TU.
- `FAILED:` si un commit ne donne pas, par catégorie, les comptes avant/après **en lignes et en occurrences**, ou si un couple (catégorie, fichier) où Δlignes > Δoccurrences n'est pas justifié.
- `FAILED:` si un TU garde une lecture par nom d'un défaut du périmètre, ou si ses comparaisons passent par un registre ou par un lecteur par nom local au test.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` (ou `identity[]`) change, ou si les trois constantes de `test_pico8_spec_defaults` sont supprimées sans la sonde « ancien registre vert à côté des fonctions nouvelles » au commit.
- `FAILED:` si le rapport gcov ou une mutation manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si un chunk n'est pas un commit vert seul, ou si les chunks ne suivent pas l'ordre 1, 2.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si le message de commit ne porte pas les comptes avant et après et (chunk 2) la description du générateur.
- `FAILED:` s'il manque au chunk 2 la note « Découvert à D.5b », ou si elle omet le compte `spec` réel et l'explication du majorant, l'exclusion de `rk`, la forme de `pj.ou`/`ea` ou les appelants restants de `pico8_ke_start_field`.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les spécifications de D.5c** (arbre `jo`, aides `family_default_*` et `family_trade_default_*`, `field` des sortes) et **`pico8_ke_start_field`** avec le cas `M_KE` (D.5c).
- **Les clés de construction** (`overrides`) de `g_`, `fv`, `ie`, `hp`, `hy`, `trade`, `pj` : D.6c.
- **Le typage de `pj.ou`/`pj.ea`** et de leurs lecteurs (`hb`, `hk`, `kj`, `pico8_mh_new`, `score_panel_display`) : D.6b/D.7a ; ils restent des tables ob d'état, construites depuis la donnée générée.
- **La fuite par frame des vues de D.4b** (`fv:kj`, `pj:kj`) et les autres vues à l'appel (`jo:gh`, `jo.ex`, `hk`) : D.7a (TROU-7).
- **Les appels `pico8_pi_init()`**, `g_bo`, `g_bz` : D.7a / D.6b.
- **Le cycle `_init`/`_shutdown`/`_ready`** de ces modules : gardé.
- `par-nom`, `dispatch`, `pico8_list_t` : D.9.
- Les archétypes résolus le long de la chaîne `fv` → `g_` → `ke` : phase 2.
- `make raylib`, l'appli raylib, l'audio, le rendu, `make e2e-gap` et `tools/line_coverage_reference.txt`.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **`rk`** : exclusion prouvée (`SPEC_EXCLUSION` + `rk`), commit à part, test rouge d'abord ; renommage rejeté.
  - **forme** : un `pico8_X_start` `static const` par classe à clés (`g_`, `fv`, `hp`, `ie`, `pj`), clés propres, types de la struct, tables par l'objet de D.4b ou un objet nouveau (`pico8_pj_cr_start`, `pico8_pj_ea_row`, `pico8_pj_ou_row`) ; `ie` unifié (chaîne et objets prêtés) ; `hy`/`trade` sans objet.
  - **une seule source** : constructeurs et `init` lisent la donnée générée ; 36 statiques de `g_`/`fv` et le `cr` recopié supprimés ; vecteurs et boîtes partagés en `const` (repli : statique non `const` affecté à l'init).
  - **`pj.ou`/`ea`** : tables d'état construites une fois par init depuis leurs rangées générées, jamais libérées (durée de vie d'aujourd'hui) ; typage reporté à D.6b/D.7a.
  - **lecteurs par nom** : les sept `pico8_X_default` supprimés sans remplaçant ; `pico8_ke_start_field` gardé pour D.5c.
  - **fuite par frame** : non fermée, non aggravée (D.7a).
  - **tests** : test ponctuel étendu aux huit chaînes, dans les deux sens ; TU en comparaisons directes ; registre de `test_pico8_spec_defaults` et ses trois constantes supprimés, vérité reprise par sept fonctions à leur nom, sonde par coexistence ; huit mutations ; pas d'e2e ; `class-coverage-net` et `sanitize-net` au chunk 2.
  - **découpage** : deux chunks (exclusion, puis conversion) ; repli 2a/2b.
  - **anti-tassement** : comptes en lignes ET en occurrences, par catégorie, fichier et chunk ; recette qui lit l'exclusion dans le script ; interdits nommés, dont le registre de TU et le lecteur renommé hors motif.
- **Note du plan (G)**, à ajouter au chunk 2 en fin de ligne D.5b (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.5b : deux chunks (exclusion de `rk`, puis les sept classes ; ≈ 29 chemins, `pico8_game_data.*` et trois TU hors du majorant). `pico8_rk_default` (variante de dessin à argument omis) entre dans `SPEC_EXCLUSION`, preuve : ses quatre lignes ne portent aucune autre catégorie. Défauts générés `pico8_X_start` de `g_`, `fv`, `hp`, `ie`, `pj`, lus par les constructeurs ; `hy`, `trade` sans objet. `ie` : un seul défaut pour la chaîne et les objets prêtés, `kq`/`ra`/`eh` pointent la donnée générée, plus libérés à l'arrêt. `pj.cr` : `g_pj_cr` reste (son `ep` est muté), valeurs de `pico8_pj_cr_start`. `pj.ou`/`pj.ea` : rangées générées `pico8_pj_ou_row`/`pico8_pj_ea_row`, d'où `pico8_score_panel_init` construit une fois par init les tables d'état `g_pj_ou`/`g_pj_ea` (`pico8_jb_row_view`), jamais libérées comme celles de la spécification ; leurs lecteurs (`hb`, `hk`, `kj`, `pico8_mh_new`) restent des tables ob (D.6b, D.7a). `test_pico8_spec_defaults` n'a plus de registre ni de constante. `pico8_ke_start_field` n'a plus que `pico8_jo_default` et le cas `M_KE`. `spec` après D.5b : N (au lieu de ≤ 402 après le décalage de D.5a : 28 lignes des sites de D.5b, retirées par D.4a/D.4b, étaient comptées à la fois dans le majorant 547 et dans ce décalage ; majorant corrigé 430) ; le « = 0 » de D.5c est inchangé. `par-nom` : P.

  N est le compte `spec` réel, P le compte `par-nom` réel. Les noms sont ceux de l'implémentation (et un repli de H3 y est nommé s'il a servi).
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget de tests), anti-tassement compris.
- **Le message de commit** de chaque chunk porte : le périmètre ; (chunk 1) la preuve de grep et le compte exact ; (chunk 2) la forme par classe et ses rejets, le générateur en ≤ 15 lignes, la forme de `pj.ou`/`ea` et sa durée de vie ; les comptes avant et après **en lignes et en occurrences** par catégorie, le compte `ob` hors test, et les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie ; la sonde des constantes supprimées ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses qui le concernent.
- **Surprises pour le pilote** :
  - **Le majorant décalé de la ligne est faux de 28** : le décalage −145 de D.5a mélange ses propres retraits et ceux de D.4a/D.4b aux sites de D.5b, déjà comptés par le majorant 547. Mesuré ici par `git show` (243 → 215 lignes dans les fichiers de D.5b entre `3816427` et `64bb06e`). Le compte attendu, 430, est exactement le majorant corrigé ; la note le rattrape. D.5c (« = 0 ») n'en dépend pas.
  - **`pico8_rk_default` bloquait D.5c** : sans l'exclusion, `spec` ne pouvait pas tomber à 0. C'est le seul changement de motif de l'étape, dans son propre commit.
  - **`g_` et `fv` étaient les dernières classes à lire leur spécification par nom à l'init** (36 statiques remplis par nom) : c'est le gros de la baisse, et c'est une vraie construction retirée, pas une suppression de code mort.
  - **`pj` avait deux sources pour `cr`** (la spécification, jamais lue pour ses valeurs, et `g_pj_cr` recopié à la main) : D.5b en garde une.
  - **`ie` parsait deux fois les mêmes trois objets** et libérait ceux qu'il prêtait à des icônes qui survivent à l'arrêt : l'unification supprime ce pointeur pendant.
  - **`pj.ou`/`pj.ea` sont le seul cas où la règle « défaut muté → tableau fixe statique non `const` » ne s'applique pas encore à la lettre** : tous leurs lecteurs veulent une table ob, et les typer relève de D.6b/D.7a. La donnée générée est la source, la table ob n'est que l'état mutable construit depuis elle à l'init — même logique que le repli `g_nf_ra` de D.5a.
  - **La fuite par frame de D.4b est dans deux fichiers du périmètre** et reste ouverte : la fermer ici ne serait qu'à moitié et jeté par D.7a.
