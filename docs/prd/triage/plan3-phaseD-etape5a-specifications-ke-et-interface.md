---
title: "Plan 3, étape D.5a : les spécifications de `ke` et des classes d'interface (`bg`, `ui`, `js`, `fq`, `jb`, `lq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`) deviennent des défauts `static const` générés une fois (`pico8_X_start`) que les constructeurs lisent ; les lecteurs par nom `pico8_X_default`, `pico8_ui_defaults_reset` et les aides `ui_default_*` disparaissent ; `pico8_ke_default` est remplacé, au bout des lecteurs de D.5b/D.5c, par un lecteur compté par `par-nom` — deux commits (chunks), aucun pixel changé"
description: "Après D.4b, treize spécifications de classe sont encore parsées par `ob()` à l'init de leur module alors que leurs constructeurs posent déjà les mêmes valeurs à la main : la table parsée ne sert plus qu'aux lecteurs par nom (`pico8_X_default`, surtout des TU), à l'init de `ui` et au drapeau « prêt ». D.5a génère ces défauts une fois en `static const`, fait lire ces données par les constructeurs (une seule source), supprime les tables et leurs lecteurs, et mesure la baisse de `spec` en lignes ET en occurrences, sans tassement."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.5a — `spec`, `ke` et les classes d'interface : `g_X_spec`, `pico8_X_default`, `pico8_X_defaults_reset` et aides `X_default_<genre>` de `ke`, `bg`, `ui`, `js`, `fq`, `jb`, `lq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`, avec les appels `pico8_ke_default` des autres fichiers → défauts `static const` générés depuis `pico8_ob(spec)`, test ponctuel étendu. Lecture de la première puce : une spécification de classe est une table constante parsée au démarrage. `test_pico8_spec_defaults.c` et `test_pico8_jo_spec_defaults.c` lisent les défauts statiques, même intention. Empreinte 30 (21, 9). Compte : `spec` ≤ 779. §6 : aucun. `class-coverage-net` : oui.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » des étapes livrées, recopiées (D.0, D.2, D.3, D.4a, D.4b) :

> Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.

> Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.

> Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).

> Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).

> Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp` : `lq_own`/`lq_inner`) ; `hp.mr`/`gp` en `pico8_str_seq_t`, `ho_apply` les prend typés ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages `po_own`/`fu_own` dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie ; une clé `qe` chaîne est absente) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus, `pico8_jc_hg_add`/`_has`) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`, qui partage son calcul avec `pico8_gi`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : 1882 (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de −100 (1882 − 1982).

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

Lecture de ces textes pour D.5a :

- **Le majorant de la ligne (`spec` ≤ 779) est pris sur la base de la liste (939)** ; D.4a et D.4b ont déjà retiré 146 lignes `spec` (939 → 793), surtout des `class_ref(pico8_X_default, …)` de membres table. Aucune note n'a encore décalé les majorants `spec`. Mesuré ici (inventaire), D.5a retire 159 lignes : **attendu `spec` = 634** (section « Effet sur `make data-residue` »), largement sous 779. La note « Découvert à D.5a » donne le compte réel N et décale les majorants `spec` suivants de N − 779 (règle des notes D.2 à D.4b).
- **Aucun des membres table de ces classes n'est encore une table ob** (D.4a/D.4b : `nu`, `k_`, `kk`, `pl`, `pc` pointent déjà des objets de `pico8_game_data.c`) : D.5a ne touche que les clés non table et les tables parsées que plus aucun membre ne pointe.
- **Ponts laissés par D.4b** (`pico8_nb()`, `pico8_gi(ob)`, vues `pico8_jb_row_view` à l'appel de `hp.fk`, `hp.lq`, `fv.rb`, `pj.i_`, `pj.gs[idx]`) : aucun n'est dans le périmètre de D.5a, aucun n'est touché.
- **Fuite connue qui grandit** : les vues à l'appel de D.4b (`fv:kj`, `help.c:90`, et `pj:kj`, `score_panel.c:317`) libèrent la table de tête mais pas les rangées imbriquées que `pico8_jb_row_view` alloue : **elles fuient à chaque frame d'aide ou de panneau de score ouvert** (TROU-7), jusqu'à D.7a qui les ferme. **D.5a n'ajoute aucune vue construite à l'appel ni aucune table allouée par frame** : aucune de ses données n'a besoin d'une table ob pour être lue (critère d'échec).
- **`make sanitize-net` à chaque chunk** : chaque chunk change la durée de vie de données (tables ob du tas, parsées à l'init → données `static const` ; la liste `le` de `ui`, allouée à l'init → liste statique du module).
- **`make class-coverage-net` à chaque chunk** : chaque chunk touche des constructeurs (`*_struct_new`, `pico8_ke_struct_init`, appelé par tous). Sortie identique à `tools/class_coverage_net_expected.txt`.
- **§6 : aucun.** `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` ne lisent aucun défaut de ces classes par nom ; ils ne changent pas.

### État de départ

HEAD `64bb06e` (branche `claude/plan-pilot-3-7zdft0`), arbre propre. D.4b est livrée (`32a3888`, `f54cef9`, `4bba78c`) et gradée (`64bb06e`). Décisions de l'utilisateur qui engagent D.5a : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**. Consigne de pilotage, liante : **une étape précédente a été notée FAILED pour avoir fait baisser un compte par tassement de lignes** ; le compteur compte des lignes, et toute mise en forme, tout renommage ou tout déplacement qui baisse un compte sans retirer la construction est interdit (section « Anti-tassement »).

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `make -s data-residue`, `sh tools/data_residue.sh --detail` et un comptage d'occurrences par `grep -o` sur les motifs figés ont tourné. Le reste vient de `grep` et de la lecture des sources de `64bb06e`. Tout fait d'exécution est une hypothèse (H1 à H20).

### Inventaire (mesuré sur `64bb06e`)

**`make data-residue`** (lignes / fichiers) et **occurrences** des mêmes motifs (`grep -oE`, mêmes fichiers, même exclusion `SPEC_EXCLUSION` pour `spec`) :

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 5927 | 133 | 6844 |
| `ob` | 1995 | 124 | 2226 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 433 | 71 | 704 |
| `ref-audit` | 69 | 17 | 70 |
| `par-nom` | 1080 | 80 | 1135 |
| `dispatch` | 197 | 59 | 199 |
| `overrides` | 450 | 62 | 479 |
| `spec` | 793 | 67 | 860 |
| **résidu D** | **9212 lignes** | **142 fichiers** | — |

`ob` 1995 = **1882 hors test ponctuel** (le compte de la note D.4b) + 113 lignes (120 occurrences) de `tests/test_pico8_map_tables.c`.

**`spec` dans les fichiers du périmètre** (lignes / occurrences, `ob` et `par-nom` en lignes pour le contrôle croisé) :

| Fichier | `spec` | `ob` | `par-nom` |
|---|---|---|---|
| `pico8_ke.c` / `.h` | 7/8, 2/2 | 5, 0 | 3, 1 |
| `pico8_bg.c` / `.h` | 8/9, 1/1 | 6, 2 | 11, 0 |
| `pico8_ui.c` / `.h` | 24/25, 1/1 | 15, 2 | 15, 1 |
| `pico8_ui_button.c` / `.h` (`js`, `fq`) | 16/18, 2/2 | 23, 3 | 39, 0 |
| `pico8_layout.c` / `.h` (`lq`, `jb`) | 14/16, 2/2 | 53, 5 | 13, 0 |
| `pico8_scroll.c` / `.h` (`qb`) | 8/9, 1/1 | 7, 2 | 9, 0 |
| `pico8_mouse_reader.c` / `.h` (`nf`) | 8/9, 1/1 | 13, 2 | 3, 0 |
| `pico8_msgbox.c` / `.h` (`mh`, `iv`) | 16/18, 2/2 | 28, 4 | 18, 0 |
| `pico8_wormhole.c` / `.h` (`mz`) | 8/9, 1/1 | 6, 1 | 17, 0 |
| `pico8_slipway.c` / `.h` (`jc`) | 8/9, 1/1 | 9, 1 | 26, 0 |
| **modules** | **131/144** | | |
| `pico8_score_panel.c:520`, `pico8_planet.c:756`, `pico8_probe.c:857`, `pico8_research_icon.c:497` (appel `pico8_ke_default` au bout de `pico8_pj/jo/g_/ie_default`) | 4 lignes qui **restent** `spec` (elles nomment aussi `g_X_spec`) ; 4 occurrences retirées | | |
| `pico8_bootstrap.c:33` (commentaire) | 1/1 | | |
| TU : `test_pico8_spec_defaults` 12, `test_pico8_ui` 5 (l. 4, 144, 773, 775, 1115), `test_pico8_iv` 3, `test_pico8_mh` 2, `test_pico8_fq` 2, `test_pico8_nf` 1, `test_pico8_lq` 1, `test_pico8_jo_spec_defaults` 1 (l. 50) | 27/27 | | |

**Les treize spécifications** (chaîne exacte parsée par `ob()` à l'init du module) et ce que le constructeur pose aujourd'hui :

| Classe | Clés de la spécification | Ce que pose le constructeur aujourd'hui | Lecteurs par nom de la spécification |
|---|---|---|---|
| `ke` | `me="ph"`, `t=0` | `pico8_ke_struct_init` : littéraux `"ph"`, `0` | `pico8_ke_default`, au bout des lecteurs des 11 autres classes et de `pico8_pj_default`, `pico8_jo_default`, `pico8_g__default`, `pico8_ie_default` ; TU `test_pico8_jo_spec_defaults` (`M_KE`), `test_pico8_ui` (`identity_of`) |
| `bg` | `fa=1`, `hd=1`, `pl=o(…)` | littéraux ; `pl` → `&pico8_bg_pl_seq` (D.4b) | TU `test_pico8_spec_defaults` |
| `ui` | `ql=58`, `fa=15`, `le=o()`, `hw=b(10,10,118,118)` | **lus par nom à l'init** (`pico8_ui_default` → `ui_default_fixed`/`_list`/`_box` → statiques `g_ui_ql`/`fa`/`le`/`hw`, remis par `pico8_ui_defaults_reset`) ; `le=o()` remplacé dans la table par une liste du tas | TU `test_pico8_spec_defaults`, `test_pico8_ui` (`class_ref`, 12 sites) |
| `js` | `fa=11` | littéral | TU `test_pico8_spec_defaults` |
| `fq` | `fa=14`, `hd=1` | littéraux | TU `test_pico8_fq`, `test_pico8_spec_defaults` |
| `lq`, `jb` | *(chaîne vide)* | — | TU `test_pico8_lq` (« `ol` absent »), `test_pico8_spec_defaults` |
| `qb` | `v=v(0,0)`, `ma=0`, `p=v(192,192)`, `ra=v(192,192)`, `pc=o(…)` | statiques non `const` `g_qb_v`/`g_qb_p`/`g_qb_ra` (`p`, `ra` remis à 192 par `pico8_qb_init`) ; `ma` littéral ; `pc` → `&pico8_qb_pc_vecs` | TU `test_pico8_spec_defaults` (dont la marque de `test_second_jl_keeps_specs` sur `pc`) |
| `nf` | `l=o()`, `r=o()`, `ra=v(0,0)` | statique `g_nf_ra` ; `l`/`r` lus par `field` dans les tables du module `g_nf_l`/`g_nf_r` (D.7b), jamais dans la spécification | TU `test_pico8_nf` (`ra` ≠ celui de la spécification), `test_pico8_spec_defaults` (marque sur `l`) |
| `mh` | `nu=o("mh")`, `fa=14`, `rh=5` | littéraux ; `nu` → `&pico8_mh_nu_tags` | TU `test_pico8_mh`, `test_pico8_spec_defaults` |
| `iv` | `fa=9`, `l=0`, `v=v(0,-0.34)` | littéraux ; statique `g_iv_v = {0, -PICO8_FIXED_LIT(0x0000570a, 0.34)}` | TU `test_pico8_iv`, `test_pico8_spec_defaults` |
| `mz` | `nu`, `me="kz"`, `dw=0`, `dq="pr"`, `spawn_snd=52`, `ik=49`, `oi=0`, `od=3`, `fp=1`, `fa=5`, `kk`, `jj=o(qs="-too far-")` | littéraux ; `nu`/`kk` → objets de D.4b ; `jj` → l'enveloppe statique `g_mz_jj_wrapper` (en-tête de struct, `qs` littéral) | TU `test_pico8_spec_defaults` |
| `jc` | `nu`, `me="kz"`, `fa=2`, `fp=f`, `ik=48`, `dw=0.004`, `k_`, `nh=20` | littéraux (`dw` = `PICO8_FIXED_LIT(0x00000106, 0.004)`, `fp` = 0) ; `nu`/`k_` → objets de D.4b | TU `test_pico8_spec_defaults` |

**Faits qui engagent la forme :**

- **Chaque constructeur pose déjà ses défauts à la main** ; aucun `field` de ces classes ne lit sa spécification. La table parsée ne sert plus qu'aux lecteurs `pico8_X_default` (hors `src/` : des TU ; dans `src/` : l'init de `ui` et les chaînes vers `ke`) et au drapeau « prêt » (`pico8_X_ready()` = spécification non NULL).
- **Aucune écriture en place** dans les vecteurs et la boîte partagés (`grep` : aucun `->v->x =`, `*self->p =`… dans `src/`) : `qb`, `iv`, `nf` remplacent le pointeur par un vecteur du tas (`scroll.c:220-225`, `msgbox.c:242,259`, `mouse_reader.c:126`), `ui.hw` n'est jamais écrit.
- **`ui.le`** (défaut) n'est jamais modifié en place par `src/` : `ui:cj` et `pz` (`msgbox.c`) posent une liste neuve. Seul `test_second_jl_keeps_specs` y ajoute une marque (puis la retire).
- **`jo` a son propre `me`** (`"unknown"`) ; `pj`, `g_`, `fv`, `ie`, `hy`, `hp`, `trade` héritent celui de `ke` par la chaîne. Aucun code de `src/` ne compare des pointeurs `me` (`pico8_dg` dispatche par `pico8_mv` sur la chaîne).
- Les chaînes rendues par `ob()` sont des copies du tas (`pico8_sub`) : **aucun pointeur de chaîne de la spécification n'est aujourd'hui partagé avec une struct** ; les TU qui plient l'identité d'une chaîne (`identity_of` de `test_pico8_ui`, `same_value` des deux tests de défauts) y lisent donc « distinct ».
- `pico8_qb_init`, `pico8_nf_init`, `pico8_ui_init`, `pico8_iv_init` appellent `pico8_pi_init()` pour parser `v(…)`/`b(…)` ; `pico8_help_init` compte, par un `assert`, sur un `pico8_pi_init()` appelé avant lui par d'autres.
- `pico8_ke_ready()` est lu par `pico8_jl()` (garde « construire une fois ») et par les `assert` de `pico8_qb_init`/`pico8_nf_init` ; `pico8_jc_ready()` et `pico8_mz_ready()` par les `assert` d'ordre d'init de `jo:ldown`/`bg:ldown`. `pico8_cart.c` (21 lignes), `pico8_bootstrap.c` (12) et des TU (`test_pico8_je` 8…) appellent ces `_init`/`_shutdown`/`_ready`.

## Solution

**Deux commits (chunks, section « Implementation Chunks »), sans aucun pixel changé** :

1. **Chaque classe du périmètre qui a des clés reçoit un défaut généré** `pico8_X_start`, objet `static const` d'un type `pico8_X_start_t` propre à la classe, déclaré dans `pico8_game_data.h` (pur), qui tient exactement les clés de **sa** spécification (pas celles de `ke`), sous les types que la struct porte déjà. Les tables gardent l'objet qu'elles ont depuis D.4a/D.4b, que le défaut pointe.
2. **Les constructeurs lisent ce défaut** au lieu de leurs littéraux : une seule source pour chaque valeur. Les vecteurs et la boîte jamais écrits en place (`qb.v`/`p`/`ra`, `iv.v`, `nf.ra`, `ui.hw`) pointent directement la donnée générée.
3. **Les tables parsées, `pico8_X_default`, `pico8_ui_defaults_reset` et `ui_default_fixed`/`_list`/`_box` disparaissent.** Le cycle `_init`/`_shutdown`/`_ready` de chaque module reste, sur un drapeau.
4. **`pico8_ke_default` est remplacé, au bout des lecteurs par nom de `pj`, `jo`, `g_` et `ie` (D.5b/D.5c), par `pico8_ke_start_field`**, lecteur par nom du défaut généré de `ke`, compté par `par-nom`.
5. **Le test ponctuel est étendu** aux treize spécifications (chaînes copiées octet pour octet) ; les TU qui lisaient ces défauts par nom comparent directement aux données générées, sans registre ; les quelques constantes de hachage qui bougent le font avec leur sonde.

**Sortie de l'étape** (après le dernier chunk), constatée par commande :

- `make test` vert ; `make e2e` : 26 conformes ; `make sanitize-net` : 26 propres ; `make class-coverage-net` : identique ; `make data-guard` : code 0 ;
- `make data-residue` : `spec` = 634 (écart ≤ 6 expliqué), `stash` 28/4, `overrides` 450, `dispatch` 197, `par-nom` 1088 (écart ≤ 3 expliqué), `ob` hors test ponctuel < 1882 ; comptes en lignes ET en occurrences, avant et après, par catégorie et par chunk, conformes à « Anti-tassement » ;
- `grep -rnE '\bg_(ke|bg|ui|js|fq|lq|jb|qb|nf|mh|iv|mz|jc)_spec\b|\bpico8_(ke|bg|ui|js|fq|lq|jb|qb|nf|mh|iv|mz|jc)_default\b|\bpico8_ui_defaults_reset\b|\bui_default_(fixed|list|box)\b|\bg_ui_(ql|fa|le|hw)\b|\bg_(qb_v|qb_p|qb_ra|iv_v|nf_ra)\b' src tests --include='*.[ch]'` ne rend rien (hors un statique gardé par le repli de H3, nommé au commit) ;
- `grep -rn 'pico8_ob(' src/pico8_{ke,bg,ui,ui_button,layout,scroll,mouse_reader,msgbox,wormhole,slipway}.c` ne rend aucune spécification (les `pico8_ob(` restants, s'il y en a, ne sont pas des spécifications de classe et sont nommés au commit) ;
- `grep -rn 'pico8_jb_row_view' src` rend exactement les sites de `64bb06e` (aucune vue ajoutée).

## User Stories

1. En tant que mainteneur du port, je veux que les défauts de `ke`, `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz` et `jc` soient des données `static const` générées une fois, afin qu'aucune spécification de classe d'interface ne soit plus parsée au démarrage.
2. En tant que mainteneur, je veux que chaque constructeur lise son défaut généré au lieu d'un littéral recopié, afin qu'une valeur de défaut n'existe qu'à un seul endroit.
3. En tant que mainteneur, je veux un type de défaut par classe, aux champs typés comme la struct, afin de lire `pico8_bg_start.fa` sans passer par un nom.
4. En tant que mainteneur, je veux que les défauts table déjà générés par D.4a/D.4b (`nu`, `k_`, `kk`, `pl`, `pc`) soient pointés par le défaut de leur classe, afin de ne pas doubler une donnée.
5. En tant que gardien du comportement, je veux que les non-entiers (`jc.dw` = 0.004, la composante −0.34 de `iv.v`) gardent les bits exacts qu'`ob()` rendait, afin qu'aucun mouvement ne change d'un pixel.
6. En tant que gardien du comportement, je veux que `jc.fp` reste le booléen faux, afin que la lecture par nom de `fp` rende toujours `false`.
7. En tant que mainteneur, je veux que `qb.v`/`p`/`ra`, `iv.v`, `nf.ra` et `ui.hw` pointent la donnée générée, afin de supprimer cinq vecteurs statiques recopiés et la remise à 192 de `pico8_qb_init`.
8. En tant que gardien du comportement, je veux que toutes les instances d'une classe partagent toujours le même vecteur ou la même boîte de défaut, afin que les identités observées (`ra_je == ra_a`) restent vraies.
9. En tant que mainteneur, je veux que `ui` ne lise plus rien par nom à son init, afin que `pico8_ui_defaults_reset` et les trois aides `ui_default_*` disparaissent.
10. En tant que mainteneur, je veux que la liste `le` partagée par les `ui` neufs soit une liste statique du module, rendue par un accesseur, afin qu'elle ne soit plus une liste du tas glissée dans une table ob.
11. En tant que gardien du comportement, je veux que cette liste soit vide et partagée par tous les `ui` neufs, afin que `ui:cj`, `pz` et le survol se comportent pareil.
12. En tant que mainteneur, je veux que l'enveloppe `mz.jj` tienne sa chaîne `qs` de la donnée générée, afin que « -too far- » n'existe qu'une fois.
13. En tant que mainteneur, je veux que `lq` et `jb`, dont la spécification est vide, n'aient aucun défaut généré, afin de ne pas créer d'objet sans contenu.
14. En tant qu'appelant de `pico8_X_ready()`, je veux que le contrat « 1 de l'init à l'arrêt, 0 sinon » reste vrai pour les treize modules, afin que `pico8_jl()`, `pico8_cart.c` et les `assert` d'ordre d'init ne changent pas.
15. En tant que lecteur de `pico8_jo_default`, `pico8_pj_default`, `pico8_g__default` et `pico8_ie_default`, je veux que `me` et `t` hérités de `ke` soient toujours servis, afin que ces lecteurs gardent leurs réponses jusqu'à D.5b/D.5c.
16. En tant que gardien du compteur, je veux que le lecteur par nom qui survit pour `ke` soit compté par `par-nom`, afin que la baisse de `spec` ne cache pas un lecteur par nom renommé.
17. En tant que vérificateur, je veux un test ponctuel qui compare chaque défaut généré à `pico8_ob()` sur la chaîne exacte de départ, clé par clé et dans les deux sens, afin de prouver qu'aucune clé, aucun genre, aucun bit n'a été perdu.
18. En tant que vérificateur, je veux que le test ponctuel prouve que `lq`/`jb` parsent une table vide et que `ui.le`, `nf.l`, `nf.r` parsent des tables vides, afin que l'absence de donnée générée soit justifiée.
19. En tant que gardien des TU, je veux que chaque lecture par nom d'un défaut du périmètre devienne une égalité directe avec la donnée générée, écrite à son site, afin que le test dise ce qu'il vérifie.
20. En tant que gardien des TU, je veux qu'aucun registre (switch, tableau, table ou boucle nommant ensemble les modules, leurs lecteurs ou leurs données) ne remplace les lecteurs supprimés, afin que les tests ne recréent pas la couche qu'on retire.
21. En tant que gardien des TU, je veux que `test_pico8_spec_defaults` et `test_pico8_jo_spec_defaults` gardent leur intention (défauts servis, struct neuve qui les reprend, cycle prêt/arrêt, second `jl()` qui ne refait rien), afin de ne perdre aucune vérité.
22. En tant que gardien des TU, je veux que toute constante de hachage reprise le soit avec sa sonde, afin de prouver que seule la cause nommée l'a fait bouger.
23. En tant que gardien du compteur, je veux les comptes avant/après en lignes ET en occurrences par catégorie et par chunk, afin qu'une baisse prouve une construction retirée.
24. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, aucun identifiant compté aliasé, aucun code déplacé hors de portée du compteur, afin que les comptes mesurent la conversion.
25. En tant que gardien des filets, je veux `make e2e` à 26 conformes, `make sanitize-net` à 26 propres et `make class-coverage-net` identique après chaque chunk, afin de prouver qu'aucun pixel, accès mémoire ni instanciation n'a changé.
26. En tant que gardien de la mémoire, je veux que D.5a n'ajoute aucune vue ob construite à l'appel, afin que la fuite par frame laissée par D.4b ne grandisse pas.
27. En tant que lecteur de `docs/glossaire-noms.md`, je veux les noms `pico8_X_start`, `pico8_X_start_t`, `pico8_ke_start_field`, `pico8_ui_le_shared` et `pico8_mz_jj_qs`, afin de retrouver un défaut depuis son nom de cart.
28. En tant que `plan-pilot`, je veux une note « Découvert à D.5a » qui donne le compte `spec` réel, le décalage des majorants suivants, le lecteur `pico8_ke_start_field` et l'étape qui le retire, afin que D.5b et D.5c partent de faits.
29. En tant que `plan-pilot`, je veux que les spécifications de D.5b/D.5c, les clés de construction (`overrides`, D.6a-c), `g_nf_l`/`g_nf_r` (D.7b) et les appels `pico8_pi_init()` (D.7a) restent hors de ces commits, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Forme des défauts générés (G)

- **Un type et un objet par classe qui a des clés**, dans `pico8_game_data.h`/`.c` (module pur : ni `pico8_ob.h`, ni `pico8_value.h` ; il peut inclure `pico8_geom.h`, qui n'inclut que `pico8_fixed.h` et `pico8_vec.h`) : `pico8_ke_start_t`/`pico8_ke_start`, et de même pour `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`. Nom en `_start` (précédent : `pico8_jo_kw_start`) : les valeurs avec lesquelles une instance neuve commence. Aucun ne correspond à un motif figé (`_default`, `_defaults`, `_spec`, `_defaults_reset`, `_default_<genre>`).
- **Champs** : exactement les clés de la spécification de la classe, **sans celles de `ke`** (la classe ne les répète pas ; `me` n'apparaît que chez `ke`, `mz` et `jc`, qui le surchargent), sous le type que porte le membre de la struct : `pico8_fixed_t` pour un nombre (un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, règle de D.3, jamais `PICO8_FIXED_LIT`) ; `const char *` pour une chaîne ; `int` pour `false` (`jc.fp` = 0, seule clé booléenne du périmètre, H2) ; `pico8_vec_t` et `pico8_qz_box_t` **par valeur** ; pointeur `const` vers l'objet de D.4a/D.4b pour une table déjà générée (`nu`, `k_`, `kk`, `pl`, `pc`).
- **Tables sans objet généré** (G) : `ui.le`, `nf.l`, `nf.r` sont des `o()` vides ; `le` devient une liste statique de `ui.c` (plus bas) ; `nf.l`/`nf.r` sont déjà servis par les tables du module `g_nf_l`/`g_nf_r`, jamais par la spécification — ils restent tels quels jusqu'à D.7b (« défaut muté », tableau fixe statique non `const`). Le test ponctuel prouve que ces trois clés parsent une table vide.
- **`mz.jj`** (G) : la table `o(qs="-too far-")` devient l'objet `const char pico8_mz_jj_qs[]` (un objet par table et par classe, règle de D.4a) ; l'enveloppe statique `g_mz_jj_wrapper` (qui garde son en-tête de struct, lu par nom) l'initialise par son adresse (constante d'adresse, admise dans un initialiseur statique). Rejeté : un champ `jj_qs` dans `pico8_mz_start` en plus de l'objet (deux sources).
- **`lq`, `jb`** : spécification vide, **aucun objet** ; `pico8_layout_init` ne parse plus rien.
- Rejetés : **une instance `static const` de la struct de la classe comme défaut** (archétype de la phase 2, avec en-tête, chaîne de `ke` et membres d'état) ; **un tableau de paires nom/valeur** (recrée la table ob sous un autre nom) ; **garder les littéraux des constructeurs et générer à côté** (deux sources, dont une morte hors des tests).

### Constructeurs et membres (G)

- Chaque `*_struct_new` du périmètre (et `pico8_ke_struct_init`) prend ses défauts du `pico8_X_start` de sa classe, membre par membre, à la place du littéral ; `me` de `mz`/`jc` vient de leur `start`, `me`/`t` de toute struct descendant de `ke` viennent de `pico8_ke_start` (chunk 2).
- **Vecteurs et boîte partagés jamais écrits en place** (fait de l'inventaire, H3) : `qb.v`, `qb.p`, `qb.ra`, `iv.v`, `nf.ra` deviennent `const pico8_vec_t *`, `ui.hw` `const pico8_qz_box_t *`, et pointent le champ de la donnée générée (`&pico8_qb_start.p`…). Les statiques `g_qb_v`/`g_qb_p`/`g_qb_ra`, `g_iv_v`, `g_nf_ra`, `g_ui_hw` et la remise à 192 de `pico8_qb_init` disparaissent. Les lectures par nom rendent `pico8_value_ref((void *)membre)` (cast de D.3). **Repli** si H3 tombe (une écriture en place atteint l'un d'eux, ou le passage à `const` sort des fichiers du module et de son TU) : ce défaut garde un statique non `const` (règle du défaut muté), affecté depuis `pico8_X_start` à l'init du module, noté au commit.
- **`ui`** : `pico8_ui_init` ne lit plus rien par nom ; `g_ui_ql`, `g_ui_fa`, `g_ui_le`, `g_ui_hw`, `pico8_ui_defaults_reset` et `ui_default_fixed`/`_list`/`_box` disparaissent ; `ql`/`fa` viennent de `pico8_ui_start`. **`le`** : une liste statique non `const` de `ui.c`, vidée par `pico8_list_init` à chaque `pico8_ui_init` (comme la liste du tas neuve d'aujourd'hui), jamais libérée (aucun élément tant que `src/` ne l'écrit pas, H15) ; accesseur **`pico8_list_t *pico8_ui_le_shared(void)`** (précédents `pico8_ui_hw()`, `pico8_mz_jj_wrapper()`), qui remplace `class_ref("le")` des TU. `pico8_ui_hw()` garde son contrat (NULL avant l'init et après l'arrêt, sinon `&pico8_ui_start.hw`).
- **Cycle de vie** (G) : `pico8_X_init`/`_shutdown`/`_ready` restent pour les treize modules ; le pointeur de spécification est remplacé par un drapeau `static int` au nom hors motif. `pico8_ui_init` garde la pose du marqueur `je`, `pico8_nf_init` ses tables `g_nf_l`/`g_nf_r`. Rejeté : **supprimer ces fonctions** — `pico8_cart.c`, `pico8_bootstrap.c`, `test_pico8_je` et une douzaine de TU les appellent, les `assert` d'ordre d'init (`jo:ldown` → `jc`, `bg:ldown` → `mz`, `qb`/`nf` → `ke`) les lisent ; ce serait doubler l'empreinte pour un gain qu'aucune catégorie ne compte.
- **`pico8_pi_init()`** (G) : les appels de `pico8_qb_init`, `pico8_nf_init`, `pico8_ui_init`, `pico8_iv_init` **restent** (d'autres inits, comme `pico8_help_init`, comptent sur un appel antérieur, H9 ; `pi` est à D.7a). Les `assert(pico8_ob_lookup_constructor('v') … "qb's spec parses …")` de `qb`/`nf`, dont le contrat nomme une spécification qui n'existe plus, disparaissent avec elle.
- **Commentaires** : les contrats des en-têtes (« REF is borrowed from the spec, valid until … », « Parses ke's spec », « Frees the spec, and with it every value pico8_ke_default() handed out », le commentaire de `pico8_jl()` « a second parse of ke's spec would free … ») sont réécrits pour dire ce qui reste vrai (« 1 de l'init à l'arrêt », « rend `&pico8_ui_start.hw` ») ou supprimés avec le code qu'ils décrivaient ; le commentaire de `bg.c:157` sur `pico8_pi_init()` part avec la spécification. Quatre sortes de `CLAUDE.md` seulement ; aucun renvoi à D.5a ni au plan dans `src/` ou `tests/`.

### Ce qui remplace les lecteurs par nom `pico8_X_default(name, out)` (G)

| Lecteur aujourd'hui | Après D.5a | Pourquoi |
|---|---|---|
| `pico8_bg/ui/js/fq/lq/jb/qb/nf/mh/iv/mz/jc_default` (`src/`) | **supprimés** ; aucun remplaçant par nom | aucun lecteur de `src/` hors l'init de `ui`, qui lit désormais `pico8_ui_start` |
| lectures de ces défauts dans les TU | égalité directe avec `pico8_X_start.<clé>` (ou l'objet de D.4a/D.4b, `pico8_ui_le_shared()`, `pico8_mz_jj_qs`), écrite à son site | règle des TU plus bas |
| `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_jo_default`, `pico8_g__default`, `pico8_ie_default` | **`pico8_ke_start_field(name, out)`** (`pico8_ke.c/.h`) : même contrat que `pico8_ke_field` sur une struct qui tiendrait `pico8_ke_start` (`me` absent si NULL, `t` toujours présent), sans condition de `pico8_ke_ready()` (chaque appelant est déjà gardé par sa propre spécification). Une seule formule : il appelle `pico8_ke_field` sur un `pico8_ke_t` local rempli depuis `pico8_ke_start` | ces quatre lecteurs **survivent** jusqu'à D.5b (`pj`, `g_`, `ie`) et D.5c (`jo`), dont ils sont le périmètre ; ils doivent toujours servir `me`/`t` hérités. Le nom correspond au motif `par-nom` (`pico8_…_field`) : **le lecteur par nom qui survit reste compté**, règle commune |
| `M_KE` de `test_pico8_jo_spec_defaults` (`spec_field`) | le cas lit `pico8_ke_start_field` ; **aucun cas ajouté** au switch | les dix autres cas sont les lecteurs de D.5c, qui résolvent encore par `ke` (`PARENT`) ; le cas `ke` suit le lecteur que `src/` garde pour eux et part avec eux à D.5c |
| les douze cas de `test_pico8_spec_defaults` (`lq`, `jb`, `js`, `fq`, `nf`, `qb`, `mh`, `iv`, `ui`, `bg`, `mz`, `jc`) | **retirés** du registre (`spec_field`, `is_ready`, `build`, `shutdown_of`, enum) ; restent les sept de D.5b | leur lecteur n'existe plus dans `src/` ; un lecteur local au test par module serait un registre (interdit) |

- **Rejeté : renommer `pico8_X_default` hors motif en gardant la recherche par nom** (par exemple `pico8_bg_lookup(name, out)`), **ou remplacer `pico8_ke_default` par une suite de `strcmp` non comptée** : un lecteur par nom qui survit doit être compté par `par-nom` ; c'est un critère d'échec.
- `pico8_ke_start_field` perd ses derniers appelants de `src/` à D.5b (`pj`, `g_`, `ie`) et D.5c (`jo`), avec le cas `M_KE` : **D.5c le supprime** (note du plan).

### Le générateur (G) : jetable, en C, sur l'arbre de départ

- **Programme C de l'implémenteur, dans son scratchpad, jamais versionné**, compilé contre `$(DATA_ENGINE_SRCS)` de `64bb06e`, **avant** toute modification : `pico8_pi_init()` (les chaînes contiennent `v(…)` et `b(…)`), puis `pico8_ob(s, NULL)` sur les treize chaînes **copiées octet pour octet** des `init` (`ke`, `bg`, `ui`, `js`, `fq`, `lq`, `jb`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`). Il émet l'initialiseur désigné de chaque `pico8_X_start` (un champ par ligne) et `pico8_mz_jj_qs`.
- **Il échoue fort** si une hypothèse de forme tombe (H2) : clé d'un genre hors {nombre, chaîne, `false`, vecteur, boîte, table déjà générée par D.4a/D.4b, table vide de `le`/`l`/`r`, `jj` à une seule nommée chaîne} ; table déjà générée dont le contenu diffère de l'objet existant ; `lq`/`jb` non vides ; `false` ailleurs que `jc.fp`.
- Sortie collée dans `pico8_game_data.c`, mise en forme à la main (≤ 120 colonnes, **une déclaration par ligne, un champ par ligne** comme les objets de D.4a/D.4b), sans valeur éditée ; `?` échappés (`\?`).
- Le message de commit du chunk décrit le générateur en ≤ 15 lignes. Le chunk 2 réutilise le même programme pour `ke`.

### `Makefile`, docs

- **Le `Makefile` ne change pas** : aucun fichier nouveau ; `pico8_game_data.c` est déjà dans `DATA_ENGINE_SRCS` et les listes de `main_raylib`. `DATA_RESIDUE_GUARD` reste vide (aucune catégorie ne tombe à 0).
- `docs/glossaire-noms.md` (chaque chunk, ses classes) : lignes nouvelles `pico8_X_start`/`pico8_X_start_t` (onze classes), `pico8_ui_le_shared`, `pico8_mz_jj_qs`, `pico8_ke_start_field` ; lignes `ql`, `fa`, `hw`, `le` (de `ui`), `jj`, `pl`, `pc`, `ra`, `v`, `p` qui nomment un statique supprimé → nom d'après D.5a ; la ligne `jr` (« `pico8_X_default` la lit, puis la parente ») dit que `ke` et les classes d'interface n'ont plus de spécification parsée.
- `docs/plan-3.md` : note « Découvert à D.5a » en fin de ligne D.5a, **au dernier chunk** (texte dans Further Notes).

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucune condition de jeu, aucun tirage du RNG ; aucune valeur de défaut (le test ponctuel le prouve au bit près).
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements.
- Les spécifications de D.5b (`g_`/`fv`, `hp`, `ie`/`ie_chain`, `pj`/`rk`, `hy`, `trade`) et de D.5c (arbre `jo`), leurs `pico8_X_default`, `g_X_spec`, `*_defaults_reset` : seule la fin de chaîne de quatre d'entre eux change (`pico8_ke_default` → `pico8_ke_start_field`).
- Les clés de construction (`overrides`) de ces classes : D.6b. Le paramètre `overrides` n'est renommé nulle part.
- `g_nf_l`/`g_nf_r` (D.7b), `pico8_pi_init()` (D.7a), les ponts et vues laissés par D.2 à D.4b.
- `pico8_ob.c/.h`, `pico8_stash.*`, `tools/data_residue.sh`, ses motifs, `DATA_RESIDUE_GUARD`, `tools/class_coverage_*`, `KNOWN[]`.
- `gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`.
- **TROU** : aucun écart port/cart n'est touché ni ajouté ; TROU-7 (dont les vues de D.4b qui fuient par frame) préservé à l'identique.

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec, quelle que soit la catégorie.

- **Interdit** : joindre sur une ligne des déclarations, instructions, arguments, conditions ou entrées d'initialiseur qui étaient sur des lignes distinctes ; retirer des retours à la ligne dans une ligne comptée ; déplacer une construction comptée sur une ligne comptée existante ; macro, `typedef` ou fonction enveloppe dont le seul rôle est de renommer un identifiant compté (`#define OBT pico8_ob_table_t`, `static int bgd(const char *n, pico8_value_t *o) { return pico8_bg_default(n, o); }`) ; **renommer un lecteur par nom hors motif** (`pico8_X_default` → `pico8_X_lookup`) ou le remplacer par une recherche `strcmp` non comptée ; déplacer du code compté vers un fichier que le compteur ne lit pas (extension autre que `.c`/`.h`, dossier hors `src`/`tests`, fichier généré à la construction) ; supprimer un commentaire qui ne porte pas sur une spécification, un lecteur, une aide ou un contrat retiré par D.5a.
- **Mesure exigée, avant et après chaque chunk** : par catégorie, **lignes** (`make data-residue`, et `--detail` par fichier) **ET occurrences** (recette ci-dessous), le test ponctuel donné à part. Recette figée pour l'implémenteur et le vérificateur :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E 's/pico8_(spr|fixed_rnd|dv|io|lq_ol|bl)_default//g' "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  ```

  Départ (`64bb06e`) : `valeur` 6844, `ob` 2226 (dont 120 dans le test ponctuel), `stash` 28, `type-tag` 704, `ref-audit` 70, `par-nom` 1135, `dispatch` 199, `overrides` 479, `spec` 860.
- **Contrôle** : pour chaque catégorie et chaque fichier du diff, Δlignes = avant − après et Δoccurrences = avant − après. **Si Δlignes > Δoccurrences** dans un couple (catégorie, fichier), le vérificateur lit le diff de ce fichier : chaque ligne comptée retirée doit correspondre à une construction retirée ou convertie, pas à une construction qui réapparaît sur une autre ligne. Le message de commit liste ces couples et leur justification. Cas attendu à l'inverse (Δoccurrences > Δlignes) : les quatre lignes de chaîne (`score_panel.c:520`…) perdent une occurrence `spec` et restent comptées (elles nomment `g_X_spec`, D.5b/D.5c).
- **Les lignes qui gagnent une catégorie** sont listées au commit : `par-nom` (+4 lignes de chaîne, +3 pour `pico8_ke_start_field` — déclaration, définition, appel de `pico8_ke_field` —, +1 pour le cas `M_KE`) ; toute autre ligne nouvelle comptée est justifiée.
- **TU** : chaque lecture par nom d'un défaut du périmètre (`pico8_X_default(…)`, `class_ref(…)` de `test_pico8_ui`, `spec_ref(M_QB/M_NF/M_UI, …)`, `spec_me(pico8_ke_default)`) est remplacée par une **comparaison directe** avec la donnée générée, écrite à son site (`assert(got.as.fixed == pico8_mh_start.fa)`, `assert(ok->hw == &pico8_ui_start.hw)`). **Interdit** : un tableau, une table, un `switch` ou une boucle qui nomme ensemble les modules, leurs lecteurs ou les données attendues pour factoriser ces assertions (« registre ») ; un lecteur par nom local au test sur les données générées ; une aide de test nouvelle qui masque un motif compté.

### Empreinte remesurée

| Chunk | `src/` | `tests/` | Docs | Total |
|---|---|---|---|---|
| 1 — classes d'interface (`bg`, `ui`, `js`, `fq`, `lq`, `jb`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`) | 20 : `game_data.c/.h`, `bg.c/.h`, `ui.c/.h`, `ui_button.c/.h`, `layout.c/.h`, `scroll.c/.h`, `mouse_reader.c/.h`, `msgbox.c/.h`, `wormhole.c/.h`, `slipway.c/.h` | ≈ 9 : `test_pico8_map_tables`, `spec_defaults`, `ui`, `mh`, `fq`, `iv`, `nf`, `lq`, et `mo` si un `const` y descend (H3) | glossaire | ≈ 30 |
| 2 — `ke` et les fins de chaîne | 9 : `game_data.c/.h`, `ke.c/.h`, `bootstrap.c`, `score_panel.c`, `planet.c`, `probe.c`, `research_icon.c` | ≈ 4 : `test_pico8_map_tables`, `jo_spec_defaults`, `ui`, `spec_defaults` (+ tout TU qui plie l'identité de `me`, H7) | glossaire, plan | ≈ 15 |
| **Union** | **27** | **≈ 11** | 2 | **≈ 40** |

- **Écart avec la ligne du plan (30 : 21, 9)** : le majorant du plan compte les fichiers qui portent une ligne `spec` des classes ; il ne voit pas `pico8_game_data.c/.h` (où naissent les données), `pico8_bootstrap.c` (commentaire et contrat de `pico8_jl()`), ni les TU qui plient l'identité de `me` sans nommer un lecteur. L'union touche le seuil de ~40 : **découpe en deux chunks** (règle de la liste de D), chacun sous 30, et découpe qui isole le risque — le chunk 2 est le seul qui change l'identité d'une donnée partagée par toutes les entités (`me`, `t` de `ke`).
- Rejetés : **un seul commit** (≈ 40 chemins, et une reprise de constantes pour deux causes à la fois) ; **`ke` d'abord** (le chunk 1 devrait alors réécrire les fins de chaîne des onze lecteurs d'interface qu'il supprime ensuite).

### Effet sur `make data-residue`

Attendu (H17), lignes / occurrences :

| Catégorie | Départ | Après chunk 1 | Après chunk 2 | Pourquoi |
|---|---|---|---|---|
| `spec` | 793 ; 860 | **646 ; 701** | **634 ; 684** | chunk 1 : −122 lignes/−134 occ. des dix-huit fichiers de module (hors `ke`), −25/−25 dans les TU ; chunk 2 : `ke.c/.h` −9/−10, `bootstrap.c` −1/−1, `test_pico8_jo_spec_defaults` −1/−1, `test_pico8_ui` l. 1115 −1/−1, et −4 occurrences (0 ligne) sur les quatre fins de chaîne |
| `par-nom` | 1080 ; 1135 | **1080** (écart ≤ 2 expliqué) | **1088** (écart ≤ 3 expliqué) | chunk 2 : +4 fins de chaîne, +3 pour `pico8_ke_start_field` (déclaration, définition, appel de `pico8_ke_field`), +1 cas `M_KE` ; les assertions directes des TU lisent les membres, pas par nom |
| `ob` | 1882 hors test (1995) ; 2106 hors test (2226) | **< 1882 hors test**, attendu ≈ −55 | **strictement < chunk 1**, attendu ≈ −5 | pointeurs `g_X_spec`, parses, libérations, `get_name`/`set_name` de la spécification (et de `le` dans `ui_init`), `assert` sur `pico8_ob_lookup_constructor`, lectures ob de `test_second_jl_keeps_specs` ; le test ponctuel grandit (parses de référence) |
| `stash` | 28/4 ; 28 | **28/4** (exact) | **28/4** (exact) | — |
| `overrides` | 450 ; 479 | **450** (exact) | **450** (exact) | aucune ligne `overrides` touchée |
| `dispatch` | 197 ; 199 | **197** (exact) | **197** (exact) | — |
| `type-tag` | 433 ; 704 | rapporté, attendu en baisse | rapporté | − `assert` de `ui_default_list`/`_box`, − `fold_value`/`same_default` des cas retirés |
| `ref-audit` | 69 ; 70 | **≤ 69**, rapporté | **≤ 69** | — |
| `valeur` | 5927/133 ; 6844 | rapporté, attendu en baisse | rapporté | signatures `pico8_value_t *out_value` des lecteurs supprimés |

- **`spec` = 634 est attendu exact** : écart ≤ 6 **expliqué** au commit (une ligne de commentaire réécrite qui garde un nom de lecteur de D.5b, par exemple) ; au-dessus de 640, ou toute baisse obtenue par tassement : échec. Le majorant de la ligne (≤ 779) est tenu de loin.
- Aucune catégorie ne tombe à 0 : **`DATA_RESIDUE_GUARD` reste vide.** Aucun motif ne change.

### Hypothèses (rien n'a été lancé)

- **H1** : `64bb06e` est vert (`make test`, `make e2e` 26 conformes, `make sanitize-net` 26 propres, `make class-coverage-net` identique, `make data-guard` 0) et `make data-residue` égale l'inventaire. Démenti au départ : arrêt, D.5a ne répare rien.
- **H2** : ce qu'`ob()` produit a la forme de l'inventaire (genres par clé ; `jc.fp` seul `false` ; `lq`/`jb` vides ; `le`, `l`, `r` tables vides ; `jj` une seule nommée `qs` chaîne ; `nu`, `k_`, `kk`, `pl`, `pc` de même contenu que les objets de D.4a/D.4b). Le générateur le vérifie. Démentie : le type suit ce qu'`ob()` produit, le test ponctuel juge ; arrêt seulement si une clé ne tombe sous aucune forme.
- **H3** : aucune écriture en place (dans `src/` ni dans un TU) n'atteint `qb.v`/`p`/`ra`, `iv.v`, `nf.ra`, `ui.hw` ni le vecteur ou la boîte rendus par leur lecture par nom ; le passage des membres à `const` ne sort pas de `scroll`, `msgbox`, `mouse_reader`, `ui` et de leurs TU. Démentie : repli « statique non `const` affecté à l'init » pour ce défaut.
- **H4** : aucun lecteur n'atteint un défaut du périmètre avant l'init de son module (les constructeurs l'assertent déjà pour `ui`, `jc`, `mz`) ; le drapeau « prêt » reproduit exactement le contrat du pointeur de spécification (la sonde de `READY_HASH` le prouve).
- **H5** : les seuls lecteurs par nom des défauts du périmètre sont ceux de l'inventaire ; aucune aide à nom non littéral (`field_fixed(inst, name)`…) ne lit une spécification.
- **H6** : aucun code de `src/` ne compare des pointeurs `me` ni ne garde un pointeur rendu par `pico8_ke_default` au-delà de l'arrêt de `ke`.
- **H7** : les constantes de hachage qui bougent sont exactement : chunk 1, `SPEC_HASH`, `STRUCT_HASH`, `READY_HASH` de `test_pico8_spec_defaults` (modules retirés du pli) ; chunk 2, `STRUCT_HASH` de `test_pico8_spec_defaults` (identité de `me` hérité par `pj`, `g_`, `fv`, `ie`, `hy`, `hp`, `trade`), `KE_ME_HASH` et le tableau `identity[]` de `test_pico8_ui` (les entités à `me` = `"ph"` passent de 0 à 1). Toute autre constante qui bouge est un arrêt.
- **H8** : `test_pico8_jo_spec_defaults` garde ses quatre constantes : `jo` a son propre `me` ; `t` est plié par valeur ; `build(M_KE)` est NULL.
- **H9** : les appels `pico8_pi_init()` de `qb`, `nf`, `ui`, `iv` servent encore d'autres inits (`pico8_help_init` l'asserte) ; ils restent.
- **H10** : les bits qu'`ob()` rend pour `0.004` et `-0.34` sont ceux des littéraux actuels (`0x106`, `-0x570a`) — `test_pico8_iv` et le pli `STRUCT_HASH` le vérifient aujourd'hui.
- **H11** : aucun TU ni e2e ne dépend de `pico8_ui_hw()` NULL ailleurs qu'avant l'init et après l'arrêt (contrat gardé), ni d'une liste `le` du tas distincte d'un `pico8_ui_init` à l'autre.
- **H12** : `make sanitize-net` rend 26 propres : aucun `free` d'une donnée statique (`pico8_list_free` n'est jamais appelé sur la liste `le` partagée), aucune lecture hors bornes.
- **H13** : `make class-coverage-net` est identique : aucune instanciation ajoutée ni retirée.
- **H14** : `-Werror` reste muet : aides, statiques et inclusions devenus sans usage supprimés ; casts `(void *)` sur données `const` selon D.3.
- **H15** : `src/` n'ajoute jamais d'élément à la liste `le` partagée (`ui:cj` et `pz` posent une liste neuve) ; seul `test_second_jl_keeps_specs` y écrit une marque, qu'il retire.
- **H16** : `pico8_ke_start_field` sans garde `pico8_ke_ready()` ne change aucune réponse observée : chacun de ses quatre appelants de `src/` rend 0 avant sa propre init.
- **H17** : `make data-residue` rend les comptes du tableau ; `ob` exact dépend de l'écriture.
- **H18** : durées courtes : `make test` en quelques secondes plus une recompilation large (`game_data.h`), `make e2e` ~1 s, `sanitize-net` et `class-coverage-net` < 2 min chacun, compilation comprise.
- **H19** : `constants-check`, `float-grep`, `float-guard` restent verts : les deux `PICO8_FIXED_LIT` retirés (`jc.dw`, `g_iv_v`) ne sont référencés par aucune liste d'outils, et aucun flottant n'est ajouté.
- **H20** : `make raylib` (non exigé, jamais lancé) lierait : aucun fichier nouveau.

## Implementation Chunks

Deux commits, dans cet ordre ; chacun est vert seul (filet complet), porte ses comptes avant/après en lignes ET en occurrences, et ses preuves. Les cibles finales (« Sortie de l'étape ») ne s'appliquent qu'au dernier.

1. **D.5a-1 — classes d'interface.** Types et objets `pico8_X_start` de `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`, et `pico8_mz_jj_qs` ; générateur et test ponctuel pour eux et pour `lq`/`jb` ; constructeurs lisant les défauts ; membres `const` vers les vecteurs et la boîte ; `pico8_ui_le_shared` ; suppression des spécifications, des douze `pico8_X_default`, de `pico8_ui_defaults_reset`, des aides `ui_default_*` et des statiques recopiés ; drapeaux « prêt » ; TU convertis ; `test_pico8_spec_defaults` perd ses douze cas (constantes reprises, sonde). Ne touche pas `ke`. Cibles : `spec` 646, `par-nom` 1080, `ob` hors test < 1882.
2. **D.5a-2 — `ke` et les fins de chaîne.** `pico8_ke_start`, `pico8_ke_struct_init` le lit, `pico8_ke_start_field` ; spécification de `ke` et `pico8_ke_default` supprimés ; quatre fins de chaîne ; commentaire de `pico8_jl()` ; `M_KE` de `test_pico8_jo_spec_defaults`, `identity_of` de `test_pico8_ui` et `STRUCT_HASH` de `test_pico8_spec_defaults` (identité de `me`, sonde) ; note « Découvert à D.5a » du plan. Dépend du chunk 1 (plus aucun lecteur d'interface n'appelle `pico8_ke_default`). Cibles : `spec` 634, `par-nom` 1088, `ob` hors test < chunk 1.

**Repli pour le pilote** : si le chunk 1 dépasse 40 chemins à l'implémentation, `plan-pilot` le recoupe par fichier de module (`ui_button`/`layout`/`msgbox` d'un côté, le reste de l'autre) ; chaque classe est indépendante des autres du chunk, sauf `test_pico8_spec_defaults`, dont les constantes sont reprises une fois par sous-chunk, chacune avec sa sonde.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, identités, pixels), pas la présence d'une ligne. D.5a change la source des défauts de treize classes : **la preuve est que les données générées égalent ce qu'`ob()` produisait des chaînes de départ, que chaque struct neuve reprend exactement ces données, que chaque TU qui lisait un défaut par nom fige la même vérité sur la donnée générée, et que tout ce qui mesure le comportement rend la même chose.**

### Le test ponctuel `tests/test_pico8_map_tables.c`, étendu (G)

- **Il construit son état, il ne rejoue rien.** Ses 2 lignes `stash` ne changent pas ; il ne nomme ni `spec` (`g_X_spec`, `pico8_X_default`, `_defaults_reset`, `_default_<genre>`), ni `overrides`, ni `type-tag`.
- **Chaînes de référence** : les treize chaînes des `init`, copiées octet pour octet de `64bb06e` (les constantes partielles de D.4b, `MH_SRC`, `JC_SRC`, `MZ_SRC`, `BG_PL_SRC`, `QB_PC_SRC`, sont remplacées par les chaînes complètes ; leurs vérifications de D.4b continuent sur les mêmes clés), parsées après `pico8_pi_init()`.
- **Pour chaque classe, dans les deux sens** : chaque clé de la table parsée a son champ dans `pico8_X_start` (ou l'objet qu'il pointe, ou l'une des tables vides nommées) et lui est égale — nombre au bit près, chaîne par `strcmp`, `false` par genre et valeur, vecteur et boîte par composante, table déjà générée par identité de pointeur plus contenu (vérifié par D.4a/D.4b) — ; et le nombre de clés de la table parsée est celui que la classe couvre (aucune clé oubliée, aucune en trop). `lq`, `jb` : table vide. `ui.le`, `nf.l`, `nf.r` : table vide. `mz.jj` : une nommée `qs`, égale à `pico8_mz_jj_qs`.
- Chunk 2 : `ke` (`me`, `t`) de même ; `pico8_ke_start_field` n'y est pas (le test ponctuel ne dépend que de `DATA_ENGINE_SRCS`).

### TU touchés : lire la donnée générée, comparer directement (G)

- **Règle** : chaque lecture par nom d'un défaut du périmètre devient une égalité directe avec la donnée générée, écrite à son site, sans registre (section « Anti-tassement »).
- **`test_pico8_mh`, `test_pico8_fq`, `test_pico8_iv`** (`test_defaults_match_the_parsed_spec`) : même fonction, même intention (« le défaut de la struct est ce que la classe déclare »), comparée à `pico8_mh_start`, `pico8_fq_start`, `pico8_iv_start` ; les valeurs figées (`14`, `5`, `1`…) restent assertées. `iv` : l'identité s'inverse et se dit : `v` de la struct **est** `&pico8_iv_start.v` (hier : distinct du vecteur parsé, même contenu) ; deux `iv` neufs le partagent toujours.
- **`test_pico8_nf`** : `ra` des trois `nf` **est** `&pico8_nf_start.ra` (hier : distinct du vecteur parsé) ; valeur (0, 0) inchangée.
- **`test_pico8_lq`** : la ligne « `lq` n'a pas de défaut `ol` » est supprimée : ce qu'elle disait (la spécification de `lq` n'a aucune clé) est porté par le test ponctuel (table parsée vide) ; les assertions voisines (`ol` lu comme méthode, `pico8_lq_ready()`) restent.
- **`test_pico8_ui`** : `class_ref` (l. 142-146) est supprimé ; ses douze sites comparent à `pico8_ui_le_shared()` et `&pico8_ui_start.hw` ; `test_ui_is_a_struct` (l. 773-777) : « `ev` absent, les quatre clés présentes » devient l'égalité directe des quatre défauts (`ql`, `fa` avec `pico8_ui_start`, `le` partagée et vide, `hw`) ; le « exactement ces quatre clés » est porté par le test ponctuel. Chunk 2 : `identity_of` compare à `pico8_ke_start.me` (l. 1115) ; `identity[]` et `KE_ME_HASH` repris (exception E2).
- **`test_pico8_spec_defaults`** (chunk 1) : les douze modules du périmètre quittent l'enum et les quatre `switch` ; `test_spec_defaults`, `test_struct_defaults`, le pli « prêt » ne couvrent plus que les sept de D.5b. Pour chaque module retiré qui a une struct construisible sans état (`bg`, `js`, `qb`, `mz`), **une fonction à son nom**, sans boucle ni table de modules, asserte : ses membres de défaut égaux aux champs de `pico8_X_start` (ou pointant la donnée), `me`/`t` hérités (`"ph"`/0 au chunk 1, `pico8_ke_start` au chunk 2), et son cycle prêt (1 après `pico8_jl()`/`pico8_cart_load()`, 0 après son arrêt). `lq`/`jb` : cycle prêt seulement. `mh`, `fq`, `iv`, `nf`, `ui` : portés par leur TU (plus haut). `jc` (non construit par ce test, hier comme aujourd'hui) : ses lignes de constructeur sont atteintes par `test_pico8_jc` ; si la mutation « `jc.fa` ← `pico8_jc_start.ik` » n'y rougit pas, une assertion directe y est ajoutée (empreinte +1, notée). `test_second_jl_keeps_specs` garde son intention (le second `jl()` ne refait ni `qb`, ni `nf`, ni `ui`) sur ce qui reste d'état de module : une marque écrite dans la table `l` de `nf` (lue sur `pico8_je()`) et dans la liste `pico8_ui_le_shared()` survit au second `jl()`, et la marque de `ui` est retirée ensuite ; `qb` n'a plus d'état remis par son init : son cas est remplacé par « `pico8_qb_ready()` vaut 1 avant et après ».
- **`test_pico8_jo_spec_defaults`** (chunk 2) : le cas `M_KE` lit `pico8_ke_start_field` ; constantes inchangées (H8) ; `test_second_jl_keeps_ke` garde sa forme et gagne l'assertion directe `strcmp(pico8_ke_start.me, "ph") == 0 && pico8_ke_start.t == 0`.

### Constantes de hachage : deux exceptions nommées, chacune avec sa sonde (G)

- **E1 (chunk 1) — modules retirés du pli** de `test_pico8_spec_defaults` : `SPEC_HASH`, `STRUCT_HASH`, `READY_HASH` sont repris sur les sept modules restants. **Sonde** : une copie jetable (scratchpad, jamais versionnée) de l'ancien test, sur les dix-neuf modules, où les douze cas retirés lisent une reconstitution de ce que l'ancien lecteur rendait, construite depuis les données générées (genre, tag qu'avait la table/liste/vecteur/boîte parsé, compte, valeur, contenu de chaîne ; bit d'identité selon l'ancienne relation : 1 seulement pour `ui.le` et `ui.hw`, que la struct empruntait à la spécification ; 0 pour les chaînes, copies du tas), redonne exactement `0x85C1D9B5`, `0x7BA6CB86`, `0x1DAB5885`.
- **E2 (chunk 2) — identité de `me` de `ke`** : la struct et le lecteur par nom rendent désormais le même pointeur `pico8_ke_start.me`. `STRUCT_HASH` de `test_pico8_spec_defaults` (bit `same` de `me` pour `pj`, `g_`, `fv`, `ie`, `hy`, `hp`, `trade`), `KE_ME_HASH` et `identity[]` de `test_pico8_ui` sont repris. **Sonde** : l'ancien pli où l'identité avec le `me` de `ke` est reconstituée comme « la copie parsée, distincte de toute struct » redonne exactement l'ancienne constante.
- Pour chaque constante reprise : ancienne et nouvelle valeur, cause, sonde au commit. **Toute autre constante qui bouge est un arrêt** : l'implémenteur cherche la vérité perdue.

### Preuve des tests (§7, gcov + mutations), par chunk

- **gcov** (copie jetable du scratchpad, `--coverage`) : chaque ligne ajoutée ou modifiée de `game_data.c`, des constructeurs et des `init` du périmètre, de `ke.c` (chunk 2) est exécutée par un TU ; les lignes d'affichage atteintes seulement par les e2e sont justifiées au commit.
- **Mutations**, chacune doit rougir un test, puis être annulée :
  - chunk 1 — **M1** : un bit de `pico8_jc_start.dw` changé (`0x106` → `0x105`) → test ponctuel rouge ; **M2** : `pico8_ui_new` pose une copie de la boîte au lieu de `&pico8_ui_start.hw` → `test_pico8_ui` rouge ; **M3** : `pico8_mz_new` prend `od` dans `pico8_mz_start.fp` (3 → 1) → la fonction `mz` de `test_pico8_spec_defaults` rouge ; si `jc` est muté de même (`fa` ← `ik`) sans rougir, l'assertion directe de `test_pico8_jc` est ajoutée ; **M4** : `pico8_ui_init` ne vide plus la liste `le` partagée → `test_pico8_spec_defaults` (`test_second_jl_keeps_specs`) ou `test_pico8_ui` rouge ;
  - chunk 2 — **M5** : `pico8_ke_struct_init` pose un littéral `"ph"` au lieu de `pico8_ke_start.me` → `test_pico8_ui` (`identity[]`) rouge ; **M6** : `pico8_ke_start_field` ne sert plus `t` → `test_pico8_jo_spec_defaults` et `test_pico8_spec_defaults` rouges ; **M7** : `pico8_ke_start.t` généré à 1 → test ponctuel rouge.
- Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée.

### Pas de nouvel e2e (G)

Les lignes changées sont atteintes par les 26 e2e et par les TU ; un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 10 minutes de temps total de tests par chunk, 20 minutes pour l'étape.**

| Cible (par chunk) | Durée attendue |
|---|---|
| générateur (scratchpad, arbre de départ) | < 1 min, compilation comprise |
| sonde des constantes (scratchpad) | < 1 min |
| `make test` | quelques secondes, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H18) |
| `make class-coverage-net` | < 2 min (H18) |
| gcov + mutations (scratchpad) | < 4 min |
| `make data-guard`, `make data-residue`, recette d'occurrences | < 1 s |

Ordre conseillé dans chaque chunk : mesure de départ (lignes et occurrences) ; générateur **avant toute modification** ; types, données, test ponctuel, puis `make test` ; constructeurs, suppressions, puis `make test` ; TU, sonde, puis `make test` ; `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois ; gcov, mutations ; mesure d'arrivée, contrôle anti-tassement, gardes, glossaire (et note du plan au chunk 2).

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + chunk + « données » / « constructeurs » / « TU »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales de chaque chunk**, vertes sur l'arbre du chunk :

1. `timeout 5m make test` ;
2. `timeout 5m make e2e` : **26 « conforme »** ;
3. `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ;
4. `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ;
5. `make data-guard` : code 0 ;
6. `make data-residue` + recette d'occurrences : cibles du chunk (`spec` 646 puis 634, `par-nom` 1080 puis 1088, écarts bornés et expliqués), `stash` 28/4, `overrides` 450, `dispatch` 197, `ob` hors test ponctuel strictement en baisse ; contrôle anti-tassement passé.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` du chunk liste les chemins de sa ligne « Empreinte remesurée » (écart ≤ 3 expliqué), rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements, `Makefile` inchangé, `gh_row_lq`/`ga_export_panel_by`/`layout_nil_item_draws_nothing` intacts.
2. **Données** : un `pico8_X_start` par classe à clés, dans `pico8_game_data.c`, initialiseur désigné, un champ par ligne, non-entiers en hexadécimal signé ; `pico8_game_data.*` n'inclut ni `pico8_ob.h`, ni `pico8_value.h`.
3. **Une seule source** : aucun constructeur du périmètre ne pose plus un littéral qui duplique une clé de sa classe ; `grep` de la « Sortie de l'étape » vide.
4. **Lecteurs par nom** : aucun lecteur par nom des défauts du périmètre ne subsiste sous un autre nom ; `pico8_ke_start_field` est le seul, compté par `par-nom`, appelé par les quatre fins de chaîne et le cas `M_KE`.
5. **Mémoire** : aucune donnée générée libérée ; aucune vue ni table ob ajoutée par appel ou par frame (`pico8_jb_row_view`, `pico8_ob_table_new` : sites de `64bb06e` seulement dans les fichiers du diff, moins ceux retirés).
6. **Anti-tassement** : les comptes avant/après, en lignes ET en occurrences, par catégorie (et par fichier pour `--detail`), figurent au commit et sont recalculés par le vérificateur avec la recette ; chaque couple où Δlignes > Δoccurrences est justifié et relu ; aucune jonction de lignes, aucun alias d'identifiant compté, aucun lecteur renommé, aucun déplacement hors du compteur, aucun commentaire sans rapport supprimé.
7. **TU** : plus aucune lecture par nom d'un défaut du périmètre ; les comparaisons avec les données générées sont écrites à leur site, sans registre ; les constantes reprises le sont sous E1/E2 avec leur sonde.
8. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun récit de chantier.
9. **Docs** : glossaire à jour à chaque chunk ; note « Découvert à D.5a » en fin de ligne D.5a au chunk 2, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `64bb06e` (ou un commit de docs au-dessus, ou le chunk précédent) propre, ou si `make data-residue` n'y rend pas l'inventaire.
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence.

**Périmètre**

- `FAILED:` si une spécification du périmètre est encore parsée, si un `pico8_X_default`, `pico8_ui_defaults_reset`, `ui_default_*` ou statique recopié du périmètre subsiste au dernier chunk (hors repli de H3 nommé au commit), ou si une ligne des `grep` de la « Sortie de l'étape » subsiste.
- `FAILED:` si une valeur générée a été écrite ou corrigée à la main, si un générateur est versionné, ou si `pico8_game_data.*` inclut `pico8_ob.h` ou `pico8_value.h`.
- `FAILED:` si un constructeur du périmètre garde un littéral qui duplique une clé de sa classe.
- `FAILED:` si le diff convertit une spécification de D.5b/D.5c (au-delà de la fin de chaîne `pico8_ke_default` → `pico8_ke_start_field`), une clé de construction, `g_nf_l`/`g_nf_r`, un appel `pico8_pi_init()`, un pont ou une vue de D.2 à D.4b, ou touche un TROU.
- `FAILED:` si le diff ajoute un appel à `pico8_jb_row_view`, ou une table ob construite à chaque appel ou à chaque frame.
- `FAILED:` si le test ponctuel nomme un motif de `spec`, d'`overrides` ou de `type-tag`, ou porte plus de 2 lignes `stash`.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD`, `tools/data_residue.sh` ou un motif change.
- `FAILED:` si `make data-residue` ne rend pas `stash` 28/4, `overrides` 450, `dispatch` 197 ; si `spec` dépasse 652 au chunk 1 ou 640 au dernier, ou s'écarte de 646/634 sans explication ; si `par-nom` s'écarte de 1080 (chunk 1) ou 1088 (dernier) de plus de 3 ou sans explication ; si `ob` hors test ponctuel ne baisse pas strictement à chaque chunk.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés dans une ligne comptée, construction comptée déplacée sur une autre ligne comptée, alias (macro, `typedef`, enveloppe) d'un identifiant compté, code déplacé hors de `src`/`tests` ou hors `.c`/`.h`, commentaire sans rapport supprimé.
- `FAILED:` si un lecteur par nom d'un défaut du périmètre survit sous un nom qu'aucun motif ne compte (renommage, recherche `strcmp` non comptée), dans `src/` ou dans un TU.
- `FAILED:` si le commit ne donne pas, par catégorie, les comptes avant/après **en lignes et en occurrences**, ou si un couple (catégorie, fichier) où Δlignes > Δoccurrences n'est pas justifié.
- `FAILED:` si un TU garde une lecture par nom d'un défaut du périmètre, ou si ses comparaisons passent par un registre (tableau, table, `switch` ou boucle nommant ensemble modules, lecteurs ou données attendues) ou par un lecteur par nom local au test ; le seul cas de registre admis est le cas `M_KE` **existant** de `test_pico8_jo_spec_defaults`, sans cas ajouté.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` (ou `identity[]`) change hors E1/E2, ou sans sa sonde au commit.
- `FAILED:` si le rapport gcov ou une mutation du chunk manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si un chunk n'est pas un commit vert seul, ou si les chunks ne suivent pas l'ordre 1, 2.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si le message de commit ne porte pas les comptes avant et après et la description du générateur.
- `FAILED:` s'il manque au chunk 2 la note « Découvert à D.5a », ou si elle omet le compte `spec` réel et son décalage, `pico8_ke_start_field` et l'étape qui le retire.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les spécifications de D.5b** (`g_`/`fv`, `hp`, `ie`/`ie_chain`, `pj`/`rk`, `hy`, `trade`) **et de D.5c** (arbre `jo`, aides `family_default_*`, sortes `pico8_entity_kind_t`), sauf la fin de chaîne de `pico8_pj_default`, `pico8_jo_default`, `pico8_g__default`, `pico8_ie_default`.
- **`pico8_rk_default`** (`pico8_hud.c/.h`, `score_panel.c:339`) : une variante de dessin à argument omis, pas un défaut de classe, que le motif `spec` compte (elle n'est pas dans `SPEC_EXCLUSION`) : à D.5b (`pj`/`rk`).
- **Les clés de construction** (`overrides`) de ces classes : D.6b.
- **`nf.l`/`nf.r`** (`g_nf_l`/`g_nf_r`, tables ob du module) : D.7b.
- **Les appels `pico8_pi_init()`** : D.7a (`pi`).
- **Le cycle `_init`/`_shutdown`/`_ready`** de ces modules : gardé ; sa suppression n'est dans aucune ligne de D.
- **Les ponts, vues et fuites laissés par D.2 à D.4b** (dont les rangées imbriquées des vues de `fv:kj`/`pj:kj`, fuite par frame fermée par D.7a) : TROU-7.
- `par-nom`, `dispatch`, `pico8_list_t` (la liste `le` reste une `pico8_list_t`) : D.9.
- Les archétypes résolus le long de la chaîne : phase 2.
- `make raylib`, l'appli raylib, l'audio, le rendu, `make e2e-gap` et `tools/line_coverage_reference.txt`.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **forme** : un `pico8_X_start` `static const` par classe à clés, type `pico8_X_start_t` dans `pico8_game_data.h`, clés propres de la spécification, types de la struct, tables par l'objet de D.4a/D.4b ; `mz.jj` par `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` liste statique du module (`pico8_ui_le_shared`) ; `nf.l`/`nf.r` inchangés (D.7b).
  - **une seule source** : les constructeurs lisent le défaut généré ; vecteurs et boîte partagés pointés en `const` (repli : statique non `const` affecté à l'init).
  - **lecteurs par nom** : les douze `pico8_X_default` d'interface supprimés sans remplaçant ; `pico8_ke_default` remplacé par `pico8_ke_start_field` (compté par `par-nom`) pour les quatre lecteurs de D.5b/D.5c qui survivent, et pour le cas `M_KE` existant de `test_pico8_jo_spec_defaults`.
  - **cycle de vie** : gardé, sur un drapeau ; `pico8_pi_init()` gardé.
  - **tests** : test ponctuel étendu aux treize chaînes, dans les deux sens ; TU en comparaisons directes ; `test_pico8_spec_defaults` perd ses douze cas (E1) ; identité de `me` (E2) ; sondes ; sept mutations ; pas d'e2e ; `class-coverage-net` et `sanitize-net` à chaque chunk.
  - **découpage** : deux chunks (interfaces, puis `ke`), ≈ 30 et ≈ 15 chemins.
  - **anti-tassement** : comptes en lignes ET en occurrences, contrôle par couple (catégorie, fichier), interdits nommés, dont le lecteur par nom renommé hors motif.
- **Note du plan (G)**, à ajouter au chunk 2 en fin de ligne D.5a (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.5a : deux chunks (interfaces, puis `ke` ; ≈ 40 chemins, `pico8_game_data.*`, `pico8_bootstrap.c` et les TU qui plient l'identité de `me` hors du majorant). Défauts générés `pico8_X_start` (type `pico8_X_start_t`, `pico8_game_data.h`), un par classe à clés, clés propres de la spécification, lus par les constructeurs ; les tables gardent l'objet de D.4a/D.4b ; `mz.jj` s'initialise de `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` : liste statique de `ui.c` (`pico8_ui_le_shared`) ; `qb.v`/`p`/`ra`, `iv.v`, `nf.ra`, `ui.hw` pointent la donnée générée (membres `const`) ; `nf.l`/`nf.r` restent `g_nf_l`/`g_nf_r` (D.7b). Le cycle `_init`/`_shutdown`/`_ready` des treize modules reste, sur un drapeau. `pico8_ke_start_field` (compté par `par-nom`) remplace `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_g__default`, `pico8_ie_default` (retirés à D.5b) et `pico8_jo_default` (D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`) : D.5c le supprime. `pico8_rk_default` (variante de dessin, comptée par `spec`) est à D.5b. `spec` après D.5a : N (au lieu de ≤ 779) ; les majorants `spec` suivants se décalent de N − 779. `par-nom` : P.

  N est le compte `spec` réel, P le compte `par-nom` réel. Les noms sont ceux de l'implémentation.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget de tests), anti-tassement compris.
- **Le message de commit** de chaque chunk porte : le périmètre ; la forme par classe et ses rejets ; le générateur en ≤ 15 lignes ; les comptes avant et après **en lignes et en occurrences** par catégorie, le compte `ob` hors test, et les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie ; les constantes reprises avec leur sonde (E1/E2) ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses qui le concernent.
- **Surprises pour le pilote** :
  - **Les spécifications d'interface sont presque mortes** : chaque constructeur pose déjà ses défauts à la main (littéraux et statiques recopiés), aucun `field` ne lit la spécification ; la table parsée ne sert plus qu'aux TU, à l'init de `ui` et au drapeau « prêt ». La conversion est donc surtout une suppression, plus le choix de la source unique (la donnée générée), que les constructeurs doivent lire pour que la génération ne soit pas une donnée morte.
  - **`spec` va bien plus bas que le majorant** (≈ 634 contre ≤ 779) : le majorant est pris sur la base 939 de la liste, et D.4a/D.4b ont déjà retiré 146 lignes `spec` sans décaler les majorants. La note le rattrape (N − 779).
  - **`par-nom` monte de ≈ 8** : le lecteur par nom de `ke` survit pour les quatre lecteurs de D.5b/D.5c, sous un nom compté ; c'est voulu (règle commune), pas une régression.
  - **Deux TU encodent l'identité de pointeur de `me`** (`identity_of` de `test_pico8_ui`, `same_value` de `test_pico8_spec_defaults`) : les chaînes parsées étaient des copies du tas, jamais partagées ; une source unique les rend identiques. D'où E2, isolée dans le chunk 2.
  - **Trois TU assertent qu'un vecteur de struct est distinct de celui de la spécification** (`iv.v`, `nf.ra`) ou identique à celui qu'elle prêtait (`ui.le`, `ui.hw`) : ces identités se réécrivent vers la donnée générée ou la liste du module, même intention (quelle donnée la struct emprunte).
  - **`pico8_rk_default`** est une variante de dessin que le motif `spec` compte (hors `SPEC_EXCLUSION`) : D.5b devra la traiter (renommage avec preuve, ou exclusion par un changement de motif, qui exige la preuve de la note D.0).
  - **La fuite par frame des vues de D.4b** (`fv:kj`, `pj:kj`) grandit avec la durée d'ouverture de l'aide ou du panneau de score ; D.5a n'y ajoute rien, D.7a la ferme.
