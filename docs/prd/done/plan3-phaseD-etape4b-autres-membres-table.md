---
title: "Plan 3, étape D.4b : les autres membres table (`bg.pl`, `hy.po`/`fu`, `fv.rb`, `mh.nu`, `g_.nu`/`k_`, `hp.mr`/`gp`/`fk`/`lq`, `pj.gs`/`i_`, `qb.pc`, `jc.nu`/`k_`/`hg`, `ui.ic`, `js._cr_t`, `mz.nu`/`kk`) deviennent des pointeurs vers des données typées — défauts générés une fois en `static const`, tables de partie en stockage typé dans la struct — ; `lt`/`he`/`ei` perdent leur branche ob ; `js._cr_t` ne porte plus d'enregistrement de la vue `pico8_nb()` — trois commits (chunks), aucun pixel changé"
description: "Après D.4a, vingt-trois membres table hors de l'arbre `jo` pointent encore des tables ob (défauts parsés par `ob()` ou tables construites en cours de partie), et trois lecteurs partagés (`lt`, `he`, `ei`) lisent à double forme à cause d'eux. D.4b donne à chacun une forme C tranchée, génère les défauts en `static const`, remplace les tables de partie par un stockage typé sans allocation, ferme les doubles formes, et mesure la baisse du résidu en lignes ET en occurrences, sans aucun tassement."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.4b — `ob`, autres membres table : `bg.pl` (et `g_bg_pl`), `hy.po`/`fu`, `fv.rb`, `mh.nu`, `g_.nu`/`k_`, `hp.mr`/`gp`/`fk`/`lq`, `pj.gs`/`i_`, `qb.pc` (et `g_qb_pc`), `jc.nu`/`k_`/`hg`, `ui.ic`, `js._cr_t`, `mz.nu`/`kk`, même règle. Empreinte 39 (27, 12). Compte : `ob` ≤ 1982. §6 : aucun. `class-coverage-net` : oui.

« Même règle » renvoie à la ligne D.4a, recopiée :

> - D.4a — `ob`, membres table de l'arbre `jo` : `jo.nu`/`ka`/`dz`/`cq`/`kw`/`em`/`nk`, `qh.px`/`ba`, `hj.k_`, `ga.k_`, `nr.hz`, `mx.ht` → struct ou tableau fixe par membre (troisième puce), forme tranchée par le PRD (le masque de bits de `nu` est la phase 2) ; leurs défauts table (`jo_default_table`, `family_default_table`, `family_trade_default_table`) sont générés `static const` dans la même étape (première puce). Empreinte 34 (22, 12). Compte : `ob` ≤ 2033. §6 : `gh_row_lq` (sa table `kw`, l. 102-120) et `ga_export_panel_by` (sa table `kw`, l. 98-117) construisent le nouveau `kw`, preuve rouge dans ce commit. `class-coverage-net` : oui (init des constructeurs). […]

En-tête de la liste de D et règles communes, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

Notes des étapes précédentes, recopiées (fins des lignes D.2, D.3 et D.4a) :

> Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.
>
> Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).
>
> Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).

Lecture des notes pour D.4b :

- **`ob` aujourd'hui, hors test ponctuel : 2037.** Les majorants suivants se décalent de 4 : **le majorant de D.4b est `ob` ≤ 1986** (1982 + 4), hors test ponctuel. D.4b doit donc retirer au moins 51 lignes `ob` hors test ponctuel, **par la conversion, jamais par la mise en forme** (section « Anti-tassement »).
- **D.4a a laissé `lt`/`he` (`nu`) et `ei` (`k_`, par `pico8_particle_field_k_`) à double forme jusqu'à D.4b** : D.4b retire la branche ob, puisque `mh.nu`, `g_.nu`/`k_` (et le `k_` de `fv`, porté par sa partie `g_`), `jc.nu`, `mz.nu` sont typés ici.
- **La vue `pico8_nb()` de D.2 est fermée à D.4b pour `js._cr_t`** : après D.4b, `js._cr_t` ne pointe plus un enregistrement de la vue ; `pico8_nb()` reste pour `menu`/`tech_gate` jusqu'à D.7a.

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

Lecture de ces règles pour D.4b :

- **`make sanitize-net` est exigé à chaque chunk** : chaque chunk change la durée de vie ou le propriétaire de tables (tables ob du tas → `static const` ; tables allouées en cours de partie → stockage dans la struct).
- **`make class-coverage-net` est exigé à chaque chunk** : chaque chunk touche des constructeurs (`*_struct_new`/`*_struct_init` qui posent les défauts ; `jc:pq`, appelé par la construction ; la clé de construction `_cr_t` de `js`). Sortie identique à `tools/class_coverage_net_expected.txt`.
- **§6 : aucun.** `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` ne changent pas.

### État de départ

HEAD `ffb16a5` (branche `claude/plan-pilot-3-7zdft0`), arbre propre. D.4a est livrée (`376a205`) et gradée (`ffb16a5`). Décisions de l'utilisateur qui engagent D.4b : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**. Consigne de pilotage, liante : **une étape précédente a été notée FAILED pour avoir fait baisser un compte par tassement de lignes** ; le compteur compte des lignes, et toute mise en forme qui baisse un compte sans retirer la construction est interdite (section « Anti-tassement »).

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `make -s data-residue`, `sh tools/data_residue.sh --detail` et un comptage d'occurrences par `grep -o` sur les motifs figés ont tourné. Le reste vient de `grep` et de la lecture des sources de `ffb16a5`. Tout fait d'exécution est une hypothèse (H1 à H22).

### Inventaire (mesuré sur `ffb16a5`)

**`make data-residue`** (lignes / fichiers) et **occurrences** des mêmes motifs (`grep -oE`, mêmes fichiers, même exclusion `SPEC_EXCLUSION` pour `spec`) :

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 6001 | 133 | 6927 |
| `ob` | 2134 | 125 | 2376 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 448 | 71 | 734 |
| `ref-audit` | 70 | 17 | 71 |
| `par-nom` | 1080 | 80 | 1135 |
| `dispatch` | 197 | 59 | 199 |
| `overrides` | 450 | 62 | 479 |
| `spec` | 822 | 68 | 890 |
| **résidu D** | **9445 lignes** | **142 fichiers** | — |

`ob` 2134 = **2037 hors test ponctuel** + 97 lignes (105 occurrences) de `tests/test_pico8_map_tables.c`.

**`ob` par fichier touché** (`--detail`) : `bg.c` 11, `bg.h` 3, `economy.c` 22, `economy.h` 5, `economy_formulas.c` 17, `economy_formulas.h` 5, `economy_score.c` 16, `economy_tiers.c` 15, `score_panel.c` 26, `score_panel.h` 9, `score_panel_display.c` 51, `tech_gate.c` 13, `tech_gate.h` 2, `help.c` 18, `help.h` 3, `msgbox.c` 34, `msgbox.h` 5, `probe.c` 19, `probe.h` 4, `research.c` 48, `research.h` 8, `research_display.c` 46, `planet_display.c` 32, `collect.c` 27, `collect.h` 8, `scroll.c` 12, `scroll.h` 3, `slipway.c` 21, `slipway.h` 4, `slipway_display.c` 17, `ui.c` 19, `ui.h` 3, `ui_button.c` 24, `ui_button.h` 4, `menu.c` 66, `wormhole.c` 16, `wormhole.h` 3, `wormhole_display.c` 2, `entity.c` 2, `particle.c` 13, `data_init.c` 78 ; `game_data.c/.h` et `map_tables.c/.h` 0 (purs).

**Les vingt-trois membres** (les `pico8_ob_table_t *` qui restent dans les structs de classe) :

| Membre | Nature | Valeur | Posé par | Lu par (`src/`) | Lu **par nom** dans `src/` |
|---|---|---|---|---|---|
| `bg.pl` | défaut de module | `o(0,0,0,72,73,74,75,88,89,90,91,88,91,89,91)`, parse à part `g_bg_pl` | `bg` struct_new | `bg:pq` (lit `g_bg_pl`, pas `self.pl`, par index) | aucun |
| `hy.po`, `hy.fu` | **table de partie**, clés = `qe` de `r` (entiers), valeurs fixes | `cm(0)`/`cm(1)` puis `bx`/`ck` ; nil avant le premier `lj` | `hy:lj` (libère l'ancienne, pose `pico8_bx()`/`pico8_ck()`) | `hy:nx` (`po[r.k]`) | `economy_score.c` `ky`/`bf` (`po`), `score_panel.c` `pj:hb` (`po`), `tech_gate.c` `gi` (`fu`, clé `t.nk`) |
| `fv.rb` | défaut de classe | 4 rangées de dessin `o(…,fn="s")`/`o("…",…,fn="qr")` | `fv` struct_new | `fv:kj` → `bl` (`pi.c`) | aucun |
| `mh.nu` | défaut de module | `o("mh")`, parse à part | struct_new | — | `lt`/`he` |
| `g_.nu`, `g_.k_` | défauts de classe (`g_`, et `fv` qui garde le `nu` de `g_` et a son propre `k_`) | `o("g_","prober")`, `o(1,1,5,13,6)` ; `fv.k_` = `o(0,1,1,2,4,9,10,9,4,2,1,1,1)` | struct_init de `g_`, struct_new de `fv` | — | `lt`/`he` (`nu`), `ei` (`k_`) |
| `hp.mr`, `hp.gp` | défauts de classe | `o("px","lr","fj","ja","i","o","il","dm")`, `o("o","dm")` | struct_new | construction (`ho_apply(rc, mr, gp, ri)`, `collect.c`) ; `research.c` garde en plus une copie `g_hp_mr_names` | aucun |
| `hp.fk` | défaut de classe | `o(f,2)` | struct_new, accesseur `pico8_hp_fk()` | `hp:lq` (`research_display.c`) le pousse **comme item de rangée** | aucun |
| `hp.lq` | **table par instance** | `{px, 32, {ii, 8}}` si `ii`, sinon nil | struct_new | `mv(b,"lq")` (`planet_display.c` `jo_fe_step`) → clé `lq` d'une rangée de `js` | par `mv` (le membre masque la méthode) |
| `pj.gs`, `pj.i_` | défauts de classe | `gs` = 5 entrées dont **deux `f`** (positions 3, 4) et des rangées avec **`0.5`** (non entier) ; `i_` = 5 rangées `fn="br"`/`"rf"` | struct_new ; accesseurs `pico8_pj_gs()`, `pico8_pj_i_()` | `pj:hk` (`gs[idx]` → item 3 de `ea`, puis `mh`), `pj:kj` (`i_` → `bl`) | aucun |
| `qb.pc` | défaut de module | `o(v(-1,0),v(1,0),v(0,-1),v(0,1))`, parse à part `g_qb_pc` | struct_new | `qb:kj`-mouvement (`pc[b+1]`) | aucun |
| `jc.nu`, `jc.k_` | défauts de module | `o("jc")`, `o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1)`, parse à part | struct_new | `slipway_display.c` (`k_[nh_i]`) | `lt`/`he` (`nu`) |
| `jc.hg` | **table de partie** : ensemble d'`id` de planètes (`hg[id]=true`) | `{}` | `jc:pq` | `slipway_display.c` `jc:ch` (`hg[t.id]`) | `economy_tiers.c` (écrit `hg[planet.id]=true` à travers la lecture par nom) |
| `ui.ic` | **table de partie** : 16 couches (`fa`) de listes d'entités | `{}` neuf **à chaque `ui:gw`** (TROU-7) | `ui:gw` (`pico8_ft_index`, `collect.c`) | `ui:jz` (couches 15 → 0) | aucun |
| `js._cr_t` | clé de construction : un enregistrement de la vue `pico8_nb()` | — | `js` struct_new (`button_take_table`) | — | `menu.c` `cr_row_my_dispatch` → `pico8_hy_kt(fm, t)` |
| `mz.nu`, `mz.kk` | défauts de module | `o("mz")`, `o(7,7,6,13,5,1,1,1,0)`, parse à part | struct_new | `wormhole_display.c` (`kk[idx]`) | `lt`/`he` (`nu`) |

**Autres faits qui engagent la forme :**

- `lt`/`he` (`entity.c:216-233`, `:256-274`) et `pico8_particle_field_k_` (`particle.c:44-55`) testent `PICO8_TYPE_TAG_OB_TABLE` pour choisir la branche : après la conversion de `mh`, `g_`/`fv`, `jc`, `mz`, **plus aucune entité n'a de `nu` ou de `k_` ob** (`grep 'nu=o(\|k_=o('` : ces quatre modules et l'arbre `jo`).
- `bl`/`bl_default` (`pi.c`, D.7a) lisent chaque rangée de dessin comme une table ob (`fn` par nom). `layout` (`lq:mn`, D.6b) lit un item REF comme une table ob. `pico8_js_lq_arg_table` (`ui_button.c:28-46`) **avorte** sur un REF non tagué.
- `pico8_jb_item_t` (D.2) ne porte qu'un entier `n`, une chaîne, une rangée ou `false` : les `0.5` de `pj.gs` n'y entrent pas. `pico8_jb_row_t` ne porte qu'une nommée, `c` : le `fn` des rangées de dessin n'y entre pas.
- `hy:kt` (`economy.c:368`) ne lit de `t` que `px` et, par `pico8_gi(t)`, `od` et `nk` ; `tech_gate` écrit `e_` sur les enregistrements de la vue, jamais `px`/`od`/`nk`. `pico8_gi` a trois appelants : `hy:kt`, `menu.c` `cr_step` et `tech_gate.c` `cb` (ces deux derniers sur les enregistrements de la vue, D.7a).
- `pico8_bx`/`pico8_ck`/`pico8_cm` (`economy_formulas.c`) construisent `po`/`fu` par `set_by_value` (clé fixe → index, chaîne → nom) : les clés viennent de `pico8_r_table[i].qe` (13 entiers), puis de `kw.qe` (fixe depuis D.4a) et du `qe` de la dernière entrée `jy` d'un `qh`.
- `cart.c:187` : `pico8_bw(90)`. Les `id` de planète sont les chaînes `"p1"` … `"p90"` (`galaxy.c:80-87`).
- `pico8_ho_apply` (`collect.c:178`) a deux clients : `hp` et le test ponctuel (`test_pico8_map_tables.c:228`, `:451`).
- Les TU comparent aujourd'hui les membres à la table de la spécification (`class_ref(pico8_g__default, "nu")` dans `test_pico8_g_`/`test_pico8_fv`, `class_ref("gs")` enveloppant `pico8_pj_default` dans `test_pico8_pj`, `pico8_qb_default("pc", …)` dans `test_pico8_qb`) et plient tags et contenus dans des constantes `*_HASH` (`test_pico8_hy` `fold_fm` plie `po`/`fu` en profondeur, `test_pico8_bg` `fold_member(…"pl")`, `test_pico8_pj` `pj_ref_is(…PICO8_TYPE_TAG_OB_TABLE)`, …). `test_pico8_spec_defaults` plie `same_value(spec, struct)` (identité des REF) dans `STRUCT_HASH`.

## Solution

**Trois commits (chunks, section « Implementation Chunks »), sans aucun pixel changé** :

1. **Chaque membre devient un pointeur vers une donnée typée** (forme tranchée membre par membre plus bas) : séquences et étiquettes de D.4a, rangées `pico8_jb_row_t` (étendues d'un item fixe non entier et d'une nommée `fn`), une séquence de vecteurs, et trois stockages de partie tenus **dans la struct**, sans allocation : la table par ressource de `hy` (`po`/`fu`), l'ensemble d'`id` de `jc` (`hg`), les couches de `ui` (`ic`).
2. **Les défauts sont générés une fois en `static const`** dans `pico8_game_data.c/.h`, un objet par table et par classe, par un générateur jetable qui lit ce qu'`ob()` produit des chaînes de départ. Les parses à part (`g_bg_pl`, `g_qb_pc`, le `spec` local de `mh`, `jc`, `mz`), les aides `help_default_table`, `g__default_table`, `pico8_jc_spec_table`, `pico8_mz_spec_table`, les défauts `gs`/`i_` de `g_pj_defaults` et les statiques `g_X_<membre>` disparaissent. Les spécifications restent parsées pour leurs autres clés (D.5a/D.5b).
3. **`lt`, `he` et `ei` n'ont plus qu'une forme**, typée.
4. **`js._cr_t` pointe une entrée de `pico8_nb_table`** (trouvée par `px`), plus un enregistrement de la vue `pico8_nb()` ; `hy:kt` prend cette entrée typée.
5. **Pendant la coexistence**, la lecture par nom d'un membre converti rend un REF non tagué vers sa donnée (règle de D.4a) ; les consommateurs qui exigent une table ob (`bl`, `layout`, `js`, `mh`) reçoivent une vue construite à l'appel par `pico8_jb_row_view`.
6. **Le test ponctuel est étendu** ; les TU touchés lisent les structs et comparent **directement** aux données générées ; le glossaire et une note « Découvert à D.4b » du plan suivent.

**Sortie de l'étape** (après le dernier chunk), constatée par commande :

- `make test` vert ; `make e2e` : 26 conformes ; `make sanitize-net` : 26 propres ; `make class-coverage-net` : identique ; `make data-guard` : code 0 ;
- `make data-residue` : `stash` 28/4, `overrides` 450, `dispatch` 197, `ob` hors test ponctuel **≤ 1986** ; comptes en lignes ET en occurrences, avant et après, par catégorie, conformes à « Anti-tassement » ;
- `grep -nE 'pico8_ob_table_t \*(pl|po|fu|rb|nu|k_|mr|gp|fk|lq|gs|i_|pc|hg|ic|_cr_t|kk);' src/pico8_{bg,economy,help,msgbox,probe,research,score_panel,scroll,slipway,ui,ui_button,wormhole}.h` ne rend rien ;
- `grep -rnE '\bg_(bg_pl|qb_pc|mh_nu|g__nu|g__k_|fv_k_|fv_rb|jc_nu|jc_k_|mz_nu|mz_kk|mr|gp|fk)\b|\b(help|g_)_default_table\b|\bpico8_(jc|mz)_spec_table\b|PJ_GS|PJ_I_' src tests --include='*.[ch]'` ne rend rien ;
- `grep -nE 'PICO8_TYPE_TAG_OB_TABLE' src/pico8_entity.c src/pico8_particle.c` ne rend plus les deux branches de `lt`/`he` ni celle de `pico8_particle_field_k_`.

## User Stories

1. En tant que mainteneur du port, je veux que `bg.pl`, `jc.k_`, `mz.kk`, `g_.k_` et le `k_` de `fv` soient des séquences d'entiers typées, afin de lire une couleur de palette par un index borné sans table ob.
2. En tant que mainteneur, je veux que les étiquettes `nu` de `mh`, `g_`, `jc` et `mz` soient des `pico8_str_seq_t` générés, afin que toutes les entités enregistrent leurs étiquettes par la même forme.
3. En tant que lecteur de `lt` et `he`, je veux une seule branche, typée, afin que l'enregistrement dans `cg` ne dépende plus d'un test de tag.
4. En tant que lecteur de `ei`, je veux lire `k_` comme une séquence typée pour toutes les entités, afin que la double forme laissée par D.4a disparaisse.
5. En tant que gardien du comportement, je veux que `cg` reçoive les mêmes étiquettes dans le même ordre, afin que `cg("mh")`, `cg("g_")`, `cg("prober")`, `cg("jc")`, `cg("mz")` soient identiques.
6. En tant que mainteneur, je veux que `qb.pc` soit une séquence de quatre vecteurs typée, afin que le défilement lise sa direction sans vecteur du tas.
7. En tant que mainteneur, je veux que `hp.mr` et `hp.gp` soient des listes de noms typées et que `ho_apply` les lise sous cette forme, afin que la construction d'un projet ne traverse plus de table ob pour ses clés.
8. En tant que mainteneur, je veux que la liste des clés acceptées par `hp` soit la donnée générée `mr`, afin de ne plus tenir deux copies des huit noms.
9. En tant que lecteur de `hp:lq`, je veux que `fk` soit une rangée typée poussée en vue, afin que la rangée affichée garde `false` puis `2`.
10. En tant que mainteneur, je veux que `hp.lq` soit une rangée tenue dans la struct du projet, afin qu'un projet à `ii` ne dépende plus d'une table ob allouée à sa construction.
11. En tant que gardien du comportement, je veux que `mv(b,"lq")` rende toujours le membre quand il existe et la méthode sinon, afin que le panneau de recherche garde ses rangées.
12. En tant que mainteneur, je veux que `fv.rb`, `pj.gs` et `pj.i_` soient des rangées de dessin typées (positionnelles puis `fn`), afin que les listes de dessin de l'aide et du score soient relisibles dans un diff.
13. En tant que gardien du comportement, je veux que les `0.5` de `pj.gs` et ses deux `false` survivent exactement, afin que « last year! » et « 5 years remain » s'affichent au même endroit et que les années sans message restent muettes.
14. En tant que mainteneur, je veux que `hy.po` et `hy.fu` soient une table par ressource tenue dans `hy`, afin que `lj` ne libère ni n'alloue plus de table à chaque recalcul.
15. En tant que gardien du comportement, je veux que `po[r.k]`, `po[r.p]`, `po[r.h]` et `fu[t.nk]` rendent les mêmes valeurs, afin que la trésorerie, le score et le coût des techs soient identiques.
16. En tant que mainteneur, je veux que `pico8_cm`, `pico8_bx` et `pico8_ck` remplissent une table typée fournie par l'appelant, afin que l'économie ne rende plus de table ob à libérer.
17. En tant que mainteneur, je veux que `jc.hg` soit un ensemble borné d'`id` tenu dans la struct, afin que la route de chaque voie de glisse ne soit plus une table ob.
18. En tant que gardien du comportement, je veux que `jc:ch` tire toujours son aléa avant de tester `hg`, afin que le RNG ne se désynchronise pas.
19. En tant que mainteneur, je veux que `ui.ic` soit seize couches de listes tenues dans `ui`, vidées à chaque `ui:gw`, afin que la souris ne fuie plus une table par frame.
20. En tant que gardien du comportement, je veux que `ui:jz` parcoure les couches de 15 à 0 dans le même ordre d'insertion, afin que la priorité entre entités superposées ne change pas.
21. En tant que mainteneur, je veux que `js._cr_t` pointe une entrée de `pico8_nb_table`, afin que le bouton d'une tech ne garde plus un enregistrement de la vue `pico8_nb()`.
22. En tant que lecteur de `hy:kt`, je veux recevoir cette entrée typée, afin de lire `px`, `od` et `nk` sans nom ob.
23. En tant qu'auteur de D.7a, je veux que `pico8_gi` garde son entrée ob pour `menu` et `tech_gate` en partageant son calcul avec l'entrée typée, afin de n'avoir qu'une formule à fermer.
24. En tant que gardien du comportement, je veux que `layout`, `js`, `bl` et `mh` reçoivent des tables ob de même contenu pour `fk`, `hp.lq`, `rb`, `gs[idx]` et `i_`, afin qu'aucun pixel ne change.
25. En tant qu'auteur de D.6b et D.7a, je veux que ces vues passent toutes par `pico8_jb_row_view`, afin de savoir quoi retirer.
26. En tant que vérificateur, je veux un test ponctuel qui compare chaque défaut généré à `pico8_ob()` sur les chaînes exactes de départ, afin de prouver que la génération n'a perdu ni une entrée, ni un ordre, ni un genre.
27. En tant que vérificateur, je veux que les aides nouvelles (table par ressource, ensemble d'`id`, recherche de `nb` par `px`) soient testées sur leurs bornes, afin de savoir qu'elles gardent le sens Lua.
28. En tant que gardien des TU, je veux que chaque identité « membre = défaut de la classe X » devienne une comparaison directe avec la donnée générée de X, écrite à son site, afin que le test dise ce qu'il vérifie sans registre.
29. En tant que gardien du compteur, je veux que les comptes avant/après soient donnés en lignes ET en occurrences par catégorie, afin qu'une baisse de compte prouve une construction retirée, pas une ligne tassée.
30. En tant que gardien du compteur, je veux qu'aucun nom nouveau ne corresponde à un motif figé, et qu'aucun alias (macro, typedef, enveloppe) ne cache un identifiant compté, afin que les comptes mesurent la conversion.
31. En tant que gardien des filets, je veux `make e2e` à 26 conformes, `make sanitize-net` à 26 propres et `make class-coverage-net` identique après chaque chunk, afin de prouver qu'aucun pixel, accès mémoire ni instanciation n'a changé.
32. En tant que lecteur de `docs/glossaire-noms.md`, je veux que les lignes des vingt-trois membres et des types nouveaux nomment les noms C d'après D.4b, afin de retrouver un membre depuis son nom de cart.
33. En tant que `plan-pilot`, je veux une note « Découvert à D.4b » qui donne les ponts restants et l'étape qui ferme chacun, et le compte `ob` réel, afin que D.5a et les suivantes partent de faits.
34. En tant que `plan-pilot`, je veux que les spécifications (D.5a/D.5b), les clés de construction autres que `_cr_t` (D.6b), les consommateurs ob (`pi`, `layout`, `js`, `menu`, `tech_gate` : D.6b/D.7a) restent hors de ces commits, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Les types (G)

Dans `pico8_game_data.h` (pur : ni `pico8_ob.h`, ni `pico8_value.h`) :

- `pico8_int_seq_t`, `pico8_str_seq_t` : ceux de D.4a, inchangés.
- **`pico8_vec_seq_t`** `{ int count; const pico8_vec_t *items; }` : `pico8_vec.h` n'inclut que `pico8_fixed.h`, le module reste pur.

Dans `pico8_map_tables.h` (les types de rangée de D.2) :

- **`pico8_jb_item_t` gagne un nombre non entier** : `int is_fixed; pico8_fixed_t fixed;` (« le nombre `fixed` quand `is_fixed` ; ses bits sont ceux qu'`ob()` rendait », règle de D.3). Les initialiseurs désignés existants restent valides (champs à zéro).
- **`pico8_jb_row_t` gagne `const char *fn`** (NULL quand absent), la nommée des rangées de dessin. Ordre des nommées dans une vue : `c`, puis `fn` (celui des chaînes du cart : `fn=` vient toujours après les positionnelles, jamais avec `c`, H2).
- `pico8_jb_row_view` (`data_init.c`) écrit ces deux champs : `pico8_value_fixed(fixed)` pour un item `is_fixed`, puis la nommée `fn` après `c`.
- **`const pico8_nb_entry_t *pico8_nb_lookup(const char *px)`** : recherche linéaire dans `pico8_nb_table`, NULL si absent ou `px` NULL (même contrat que `pico8_df_lookup`).

Dans `pico8_economy_formulas.h` :

- **`pico8_qe_map_t`** : `{ int count; struct { int qe; pico8_fixed_t v; } entries[PICO8_QE_MAP_CAP]; }`, entrées dans l'ordre d'insertion, `PICO8_QE_MAP_CAP` = `PICO8_R_COUNT` + 3 = 16. Aides `int pico8_qe_map_get(const pico8_qe_map_t *, pico8_fixed_t qe, pico8_fixed_t *out)` (0 si absent) et `void pico8_qe_map_set(pico8_qe_map_t *, pico8_fixed_t qe, pico8_fixed_t v)` (clé = `pico8_fixed_flr_int(qe)`, comme `set_by_value` ; assert au-delà de la capacité, message qui nomme le contrat : « a resource key outside r's 13 and the 3 spares — stop, do not widen »).
  - Ordre d'insertion gardé : un pli d'avant reconstitué (tag de table, compte, clé index, valeur) redonne l'ancienne constante (sonde, plus bas).
  - Rejeté : **un tableau indexé par la position dans `r`** (13 fixes, sans clé). Un `qe` hors de `r` (les TU du §6 posent `kw.qe` = 99) serait perdu ; l'ordre d'insertion d'une clé en plus aussi.
  - Rejeté : **un tableau indexé par la valeur de `qe`** (0..127). Borne arbitraire, et l'ordre d'insertion est perdu.
  - Rejeté : **garder la clé chaîne** de `set_by_value`. `qe` est un fixe depuis D.4a (`kw`) ; le `qe` d'une entrée `jy` est un fixe en jeu (H4). Une clé chaîne rend « absent » à la lecture, comme la table du cart où `cm` n'en pose jamais.

Dans `pico8_slipway.h` :

- **`pico8_jc_hg_t`** : `{ int count; const char *ids[PICO8_JC_HG_CAP]; }`, `PICO8_JC_HG_CAP` = 90 (les planètes de `bw(90)`), `id` empruntés aux planètes (jamais libérés, TROU-7 des chaînes `id`). Insertion sans doublon (`strcmp`), assert au-delà de la capacité. Un `id` non chaîne (un TU qui passerait un fixe) avorte, message au format des écrivains (« … does not fit the member's type — stop, do not widen the member ») (H5).
  - Rejeté : **clé = pointeur de planète**. Le cart indexe par `id` ; `jc:ch` lit `t.id` d'une entité qui n'est pas forcément la planète de `economy_tiers`.

Dans `pico8_ui.h` :

- **`pico8_ui_ic_t`** : `{ int layer_count; pico8_list_t layers[16]; }`. **Premier membre un `int`** (16), jamais un `pico8_list_t` : un REF vers la struct lit `UNKNOWN`, pas `LIST` (H8). Une entité dont `fa` sort de 0..15 avorte (message de contrat) : `jz` ne lit que 15..0 (H6).
  - Rejeté : **`pico8_list_t (*ic)[16]`**. Un REF vers le tableau lirait le tag `LIST` de la couche 0.

Règle commune (G) : **tout type nouveau a un premier mot qui n'est aucun tag** (un compte, un pointeur de chaîne) : `pico8_type_tag_of` lit `UNKNOWN` sur ses REF, et la lecture par nom d'un membre converti reste « REF non tagué » (règle de D.4a).

### Forme par membre (G)

| Membre | Forme C du membre | Donnée (générée, `pico8_game_data.c`) ou stockage | Écritures après D.4b |
|---|---|---|---|
| `bg.pl` | `const pico8_int_seq_t *pl` | `pico8_bg_pl_seq` ; `bg:pq` lit `self->pl` s'il n'est pas NULL, sinon `&pico8_bg_pl_seq` (le cart lit `bg.pl`, la classe) | struct_new |
| `hy.po`, `hy.fu` | `pico8_qe_map_t *po`, `*fu`, plus stockages `po_own`, `fu_own` | nil (NULL) avant le premier `lj` ; `lj` remplit `po_own`/`fu_own` par `pico8_bx(&self->po_own)`/`pico8_ck(&self->fu_own)` puis pointe `po`/`fu` dessus | `lj` |
| `fv.rb` | `const pico8_jb_row_t *rb` | `pico8_fv_rb_row` (4 items rangée, chacune positionnelles puis `fn`) | struct_new |
| `mh.nu` | `const pico8_str_seq_t *nu` | `pico8_mh_nu_tags` | struct_new |
| `g_.nu` | `const pico8_str_seq_t *nu` | `pico8_g__nu_tags` (aussi le `nu` de `fv`) | struct_init |
| `g_.k_` | `const pico8_int_seq_t *k_` | `pico8_g__k__seq` ; `fv` : `pico8_fv_k__seq` | struct_init, struct_new de `fv` |
| `hp.mr`, `hp.gp` | `const pico8_str_seq_t *` | `pico8_hp_mr_names`, `pico8_hp_gp_names` ; `g_hp_mr_names` (`research.c`) disparaît, `hp_accepts_key` lit `pico8_hp_mr_names` | struct_new |
| `hp.fk` | `const pico8_jb_row_t *fk` | `pico8_hp_fk_row` (`false`, 2) ; `pico8_hp_fk()` rend `const pico8_jb_row_t *`, jamais NULL (H9) | struct_new |
| `hp.lq` | `const pico8_jb_row_t *lq`, plus stockage `lq_own` (rangée externe, 3 items) et `lq_inner` (rangée, 2 items) dans la struct | `{px, 32, {ii, 8}}` : `ii` en item `is_fixed` (exact), `px` chaîne ; NULL sans `ii` | struct_new |
| `pj.gs` | `const pico8_jb_row_t *gs` | `pico8_pj_gs_row` : items rangée, `false`, `false`, rangée ; les `0.5` en items `is_fixed` ; `pico8_pj_gs()` rend `const pico8_jb_row_t *` | struct_new |
| `pj.i_` | `const pico8_jb_row_t *i_` | `pico8_pj_i__row` ; `pico8_pj_i_()` rend `const pico8_jb_row_t *` | struct_new |
| `qb.pc` | `const pico8_vec_seq_t *pc` | `pico8_qb_pc_vecs` (4 vecteurs) | struct_new |
| `jc.nu`, `jc.k_` | `const pico8_str_seq_t *`, `const pico8_int_seq_t *` | `pico8_jc_nu_tags`, `pico8_jc_k__seq` | struct_new |
| `jc.hg` | `pico8_jc_hg_t *hg`, plus stockage `hg_own` | vide ; NULL avant `jc:pq` | `jc:pq` (`hg_own.count = 0`, `hg = &hg_own`), `economy_tiers` (insertion à travers la lecture par nom) |
| `ui.ic` | `pico8_ui_ic_t *ic`, plus stockage `ic_own` | NULL avant le premier `gw` | `ui:gw` (vide les 16 couches — `pico8_list_free` puis `pico8_list_init` — puis `ic = &ic_own`) |
| `js._cr_t` | `const pico8_nb_entry_t *_cr_t` | une entrée de `pico8_nb_table` | clé de construction `_cr_t` (nil ou REF non tagué, règle de D.4a) |
| `mz.nu`, `mz.kk` | `const pico8_str_seq_t *`, `const pico8_int_seq_t *` | `pico8_mz_nu_tags`, `pico8_mz_kk_seq` | struct_new |

- **Défauts de module et défauts de classe, une seule forme** : les parses à part (`bg`, `qb`, `mh`, `jc`, `mz`) et les défauts lus dans la spécification (`g_`, `fv`, `hp`, `pj`) deviennent tous un objet généré par classe ; chaque classe garde **sa** donnée même quand deux contenus sont égaux (règle de D.4a, « le membre est le défaut de telle classe » reste une égalité de pointeurs).
- **`po`/`fu`, `hg`, `ic`, `hp.lq` : pointeur + stockage dans la struct** (G, précédent `kw`/`kw_own` de D.4a) : la présence par nom ne change pas (NULL → absent), aucune allocation de données n'est ajoutée, et trois allocations par cycle disparaissent (`po`/`fu` à chaque `lj`, `ic` à chaque frame, `hg` à chaque `jc:pq`). Les champs de stockage (`*_own`, `lq_inner`) ne sont jamais des champs par nom.
  - Rejeté : **`malloc` du stockage**, lecture littérale de `{}`. « Plus aucun `malloc` pour des données. »
  - Rejeté : **deux tampons alternés** pour garder « un `po` neuf à chaque `lj` » par identité de pointeur. Aucun lecteur de `src/` ne garde `po`, `fu` ou `ic` au-delà de l'appel (H4, H6) ; seule l'identité asserte dans deux TU change, et elle devient une assertion de contenu (plus bas). Garder l'identité par alternance serait écrire du code pour un test.
- **`fv.rb`, `pj.gs`, `pj.i_` : rangées de dessin** (G). Positionnelles, puis `fn` ; `gs` garde ses deux `false` (items `is_false`).
  - Rejeté : **`pico8_bz_entry_t` réutilisé** (`row` + `fn` + `score` + `ra`). Nom et champs propres à `bz` ; une nommée `fn` dans la rangée est plus simple et suffit aux trois.
  - Rejeté : **une vue ob persistante par module** (`g_fv_rb_view` construit par `pico8_help_init`). Le membre porte la donnée ; précédent de D.4a (vues à l'appel).
- **`js._cr_t`** (G) : `cr_step` (`menu.c`) pose la clé `_cr_t` de la rangée à `pico8_value_ref((void *)pico8_nb_lookup(px))`, `px` lu sur l'enregistrement de la vue (qui reste l'entrée de `cb()` jusqu'à D.7a) ; `cr_row_my_dispatch` lit `_cr_t` par nom (lecteur par nom qui survit, compté par `par-nom`) et appelle `pico8_hy_kt(fm, (const pico8_nb_entry_t *)ref)`. `button_take_table` devient une aide « nil ou REF non tagué » au même préfixe (`button_take_nb`), une table ob avorte.
  - `pico8_hy_kt(pico8_hy_t *, const pico8_nb_entry_t *t)` : `px` → `invented` si non NULL ; coût par **`pico8_gi_nb(t)`** (nouveau, `tech_gate.c`).
  - `pico8_gi(const pico8_ob_table_t *)` garde sa signature pour `menu`/`tech_gate` (D.7a) ; `pico8_gi` et `pico8_gi_nb` lisent `od` et `nk` puis appellent **une seule** fonction statique de calcul (`od · max(fu[nk], 0.55)`, arrondi bas) : pas deux formules.
  - Rejeté : **une vue construite à l'appel pour `hy:kt`**. `kt` ne lit que trois champs, tous dans l'entrée typée.
- **`ho_apply`** (G) prend `const pico8_str_seq_t *mr` et `const pico8_str_seq_t *gp` : `gu(gp, p)` devient une comparaison de chaînes. Le test ponctuel passe ses `mr`/`gp` de référence sous cette forme (séquences locales du test).
- **Noms** : suffixes de forme (`_tags`, `_seq`, `_names`, `_row`, `_vecs`) ; aucun ne correspond à un motif figé (`_default`, `_defaults`, `_spec`, `_field`, `_take_` hors écrivains, `pico8_ob`, `overrides`, `pico8_value_`).

### Le générateur (G) : jetable, en C, sur l'arbre de départ

- **Programme C de l'implémenteur, dans son scratchpad, jamais versionné**, compilé contre `$(DATA_ENGINE_SRCS)` de `ffb16a5`, **avant** toute modification : `pico8_pi_init()` (les chaînes contiennent `v(...)` et `b(...)`), puis `pico8_ob(s, NULL)` sur les chaînes **copiées octet pour octet** : spécifications de `g_` (`probe.c`), `fv` (`help.c`), `hp` (`research.c`), `pj` (`score_panel.c`) ; parses à part de `bg` (`g_bg_pl`), `qb` (`g_qb_pc`), `mh`, `jc`, `mz` (le `spec` local de leur `init`). Il émet l'objet `static const` de chaque clé de l'inventaire.
- **Il échoue fort** si une hypothèse de forme tombe (H2) : positionnelles non contiguës depuis 1 (hors les `false` de `gs`, qui sont des items) ; nommée dans une séquence d'entiers, de chaînes ou de vecteurs ; non-entier ailleurs que dans une rangée de `gs` ; rangée de dessin qui n'est pas « positionnelles (chaîne, nombre), puis `fn` seul » ; `fk` autre que `(false, 2)` ; `pc` autre que quatre vecteurs à composantes entières.
- Sortie collée dans `pico8_game_data.c`, mise en forme à la main (≤ 120 colonnes, **une déclaration par ligne comme les objets de D.4a**), sans valeur éditée ; `?` échappés (`\?`).
- Le message de commit du chunk décrit le générateur en ≤ 15 lignes.

### Coexistence : ce que rendent `field`/`set_field` (G)

Règle de D.4a, inchangée : **la lecture par nom d'un membre converti rend `pico8_value_ref((void *)membre)`** — un REF non tagué — ou 0 (absent) si le membre est NULL. Les présences ne changent pour aucun nom.

| Lecteur par nom (`src/`) | Après D.4b |
|---|---|
| `entity.c` `lt`/`he` (`nu`) | **une seule forme** : `const pico8_str_seq_t *` ; la branche ob et son test de tag disparaissent |
| `particle.c` `pico8_particle_field_k_` (`k_`, lu par `ei`) | **une seule forme** : rend `const pico8_int_seq_t *` ; la branche ob et `cs` (table) disparaissent |
| `economy_score.c` `ky`, `bf` (`po`) | lecture par nom puis `const pico8_qe_map_t *` ; `pico8_qe_map_get` |
| `score_panel.c` `pj:hb` (`po`) | idem |
| `tech_gate.c` `gi` (`fu`) | idem ; `nk` fixe → `pico8_qe_map_get`, `nk` chaîne → absent (H4) |
| `economy_tiers.c` (`hg`) | lecture par nom puis `pico8_jc_hg_t *` ; insertion de `planet.id` (chaîne) |
| `menu.c` `cr_row_my_dispatch` (`_cr_t`) | lecture par nom puis `const pico8_nb_entry_t *` |
| `planet_display.c` `jo_fe_step` (`mv(b,"lq")`) | `pico8_mv` **gardé** (règle « le membre masque la méthode ») ; un REF non tagué (le membre) passe à la clé `lq` de la rangée comme `pico8_jb_row_view(...)`, un `pico8_lq_t` (la méthode) tel quel |

- **Écrivains par nom** : aucun `set_field` de `src/` n'écrit ces noms (vérifié : `bg`, `fv`, `g_`, `qb`, `jc`, `js`, `mz` n'écrivent que des scalaires). La clé de construction `_cr_t` de `js` suit la règle « nil ou REF non tagué ».
- Les aides qui n'ont plus d'appelant sont supprimées (`-Werror`) : `help_default_table`, `g__default_table`, `pico8_jc_spec_table`, `pico8_mz_spec_table`, `pj_default_table` si `ea`/`ou` ne l'utilisent plus (sinon elle reste pour eux), `set_by_value`/`get_by_value` d'`economy_formulas.c` et `economy_score.c`/`score_panel.c` s'ils n'ont plus d'appelant, `pico8_help_positional_values`/`pico8_score_panel_positional_values` s'ils n'ont plus d'appelant. `pico8_ft_index`/`pico8_ft_name` (`collect.c`, publiques) perdent leur dernier appelant : **gardées**, notées au commit pour D.7a (`collect`), pour ne pas élargir le chunk 3.
- Rejeté : **garder `lt`/`he`/`ei` à double forme « au cas où »**. Aucune entité n'a plus de `nu` ou de `k_` ob (inventaire) ; une branche morte serait du code sans lecteur.

### Vues ob pour les consommateurs (G)

Toutes par `pico8_jb_row_view`, construites à l'appel, jamais libérées (TROU-7, comme les rangées que `jo:gh`, `jo.ex`, `hp:lq`, `pz` construisent déjà à chaque appel) ; `sanitize-net` tourne avec `detect_leaks=0` :

- `hp:lq` (`research_display.c`) : l'item `fk` est `pico8_jb_row_view(pico8_hp_fk())` (D.7a) ;
- `jo_fe_step` (`planet_display.c`) : la clé `lq` d'un projet à `ii` (D.7a, puis D.6b pour `js`) ;
- `fv:kj` : `rb` ; `pj:kj` : `i_` ; puis leurs positionnelles vers `bl_default` (D.7a, `pi`) ;
- `pj:hk` : `gs[idx]` (une rangée → sa vue ; `false` → `pico8_value_bool(0)`, comme la table le rendait ; hors bornes → nil) posé en item 3 de `ea` (D.7a).
- Rejeté : **vues persistantes** (voir plus haut) ; **libérer en profondeur** : aucune aide de libération profonde n'existe, et ajouter une durée de vie aux vues est hors de l'étape.

### `Makefile`, docs

- **Le `Makefile` ne change pas** : aucun fichier nouveau ; `game_data.c`, `map_tables.c`, `data_init.c`, `collect.c` sont déjà dans `DATA_ENGINE_SRCS` et les listes de `main_raylib`. `DATA_RESIDUE_GUARD` reste vide.
- `docs/glossaire-noms.md` (chaque chunk, ses membres) : lignes `pl`, `po`, `fu`, `rb`, `nu`, `k_`, `mr`, `gp`, `fk`, `lq` (de `hp`), `gs`, `i_`, `pc`, `hg`, `ic`, `_cr_t`, `kk` → noms C d'après D.4b ; lignes nouvelles `pico8_vec_seq_t`, `pico8_qe_map_t`, `pico8_jc_hg_t`, `pico8_ui_ic_t`, `pico8_nb_lookup`, `pico8_gi_nb`, champs `is_fixed`/`fn`.
- `docs/plan-3.md` : note « Découvert à D.4b » en fin de ligne D.4b, **au dernier chunk** (texte dans Further Notes).
- **Commentaires** : les contrats des en-têtes (« `nu` and `k_` point at the tables the module owns », « `gs`, `i_` are the tables of `pj`'s class, borrowed », « Frees the `po`/`fu` tables its own previous call installed ») sont réécrits pour dire ce que pointe chaque membre et qui possède le stockage ; les commentaires de double forme de `entity.c`/`particle.c` (ajoutés par D.4a) disparaissent avec elle. Quatre sortes de `CLAUDE.md` seulement ; aucun renvoi à D.4b ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucune condition de jeu, aucun tirage du RNG.
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements.
- Les spécifications (`g_X_spec`, `pico8_X_default`, défauts non table) : D.5a/D.5b. `pico8_X_default("nu")` rend toujours la table de la spécification ; plus aucun membre ne la pointe.
- Les défauts mutés de `pj` (`ea`, `ou`, `cr`) et `g_pj_defaults` pour eux : D.5b.
- Les clés de construction autres que `_cr_t` : D.6a/D.6b/D.6c. Le paramètre `overrides` n'est renommé nulle part.
- La vue `pico8_nb()` et ses lecteurs `menu`/`tech_gate`, `pico8_gi(ob)` : D.7a. `pi.c` (`bl`), `layout`, `js` : D.6b/D.7a.
- `pico8_ob.c/.h`, `pico8_stash.*`, `tools/data_residue.sh`, ses motifs, `DATA_RESIDUE_GUARD`, `tools/class_coverage_*`, `KNOWN[]`.
- `gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`.
- **TROU** : aucun écart port/cart n'est touché ni ajouté ; TROU-2, TROU-5, TROU-7, TROU-11 préservés (les `/* TROU-7 */` des sites dont l'allocation disparaît — `ui:gw` pour `ic` — tombent avec l'allocation qu'ils annotaient ; ceux qui restent vrais restent).

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec, quelle que soit la catégorie.

- **Interdit** : joindre sur une ligne des déclarations, instructions, arguments ou conditions qui étaient sur des lignes distinctes ; retirer des retours à la ligne dans une ligne comptée ; déplacer une construction comptée sur une ligne comptée existante ; macro, `typedef` ou fonction enveloppe dont le seul rôle est de renommer un identifiant compté (`#define OBT pico8_ob_table_t`, `typedef pico8_ob_table_t tbl_t`, `static … obn(void) { return pico8_ob_table_new(); }`) ; déplacer du code compté vers un fichier que le compteur ne lit pas (extension autre que `.c`/`.h`, dossier hors `src`/`tests`) ; supprimer un commentaire qui ne porte pas sur un membre, une aide ou un contrat retiré par D.4b.
- **Mesure exigée, avant et après chaque chunk** : par catégorie, **lignes** (`make data-residue`, et `--detail` par fichier) **ET occurrences** (`grep -ohE` avec la fonction `pattern` de `tools/data_residue.sh` sur `find src tests -name '*.[ch]'`, et pour `spec` après le `sed -E "$SPEC_EXCLUSION"`), le test ponctuel donné à part. Recette figée pour l'implémenteur et le vérificateur :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E 's/pico8_(spr|fixed_rnd|dv|io|lq_ol|bl)_default//g' "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  ```

  Départ (`ffb16a5`) : `valeur` 6927, `ob` 2376 (dont 105 dans le test ponctuel), `stash` 28, `type-tag` 734, `ref-audit` 71, `par-nom` 1135, `dispatch` 199, `overrides` 479, `spec` 890.
- **Contrôle** : pour chaque catégorie et chaque fichier du diff, Δlignes = avant − après et Δoccurrences = avant − après. **Si Δlignes > Δoccurrences** dans un couple (catégorie, fichier), le vérificateur lit le diff de ce fichier : chaque ligne comptée retirée doit correspondre à une construction retirée ou convertie, pas à une construction qui réapparaît sur une autre ligne. Le message de commit liste ces couples et leur justification (ligne ajoutée à deux occurrences, par exemple `pico8_ob_table_t *t = pico8_ob_table_new();` dans une vue).
- **TU** : chaque `class_ref(pico8_X_default, "<membre>")` d'un membre converti (et ses enveloppes locales : `class_ref("gs")` de `test_pico8_pj`, `pico8_qb_default("pc", …)` comme identité de `test_pico8_qb`) est remplacé par une **comparaison directe** avec la donnée générée, écrite à son site (`get_ref(e, "nu") == &pico8_g__nu_tags`). **Interdit** : un tableau, une table ou une boucle qui nomme ensemble les lecteurs, les membres ou les données attendues pour factoriser ces assertions (« registre ») ; une aide de test nouvelle qui masque un motif compté.

### Empreinte remesurée

| Chunk | `src/` | `tests/` | Docs | Total |
|---|---|---|---|---|
| 1 — étiquettes, palettes, vecteurs ; `lt`/`he`/`ei` fermés | 20 : `game_data.c/.h`, `entity.c`, `particle.c`, `msgbox.c/.h`, `probe.c/.h`, `help.c/.h`, `slipway.c/.h`, `slipway_display.c`, `wormhole.c/.h`, `wormhole_display.c`, `bg.c/.h`, `scroll.c/.h` | ≈ 9 : `test_pico8_map_tables`, `spec_defaults`, `mh`, `g_`, `fv`, `jc`, `mz`, `bg`, `qb` | glossaire | ≈ 30 |
| 2 — rangées : `fv.rb`, `pj.gs`/`i_`, `hp.mr`/`gp`/`fk`/`lq` | 16 : `map_tables.h`, `data_init.c`, `game_data.c/.h`, `help.c/.h`, `score_panel.c/.h`, `score_panel_display.c`, `research.c/.h`, `research_display.c`, `planet_display.c`, `collect.c/.h` | ≈ 7 : `map_tables`, `spec_defaults`, `fv`, `pj`, `mh`, `hp`, `data_init` | glossaire | ≈ 24 |
| 3 — tables de partie : `hy.po`/`fu`, `jc.hg`, `ui.ic`, `js._cr_t` | 19 : `economy.c/.h`, `economy_formulas.c/.h`, `economy_score.c`, `economy_tiers.c`, `score_panel.c`, `tech_gate.c/.h`, `slipway.c/.h`, `slipway_display.c`, `ui.c/.h`, `ui_button.c/.h`, `menu.c`, `map_tables.c/.h` | ≈ 10 : `hy`, `fm`, `hp`, `jc`, `ui`, `fq`, `bg`, `fv`, `js`, `map_tables` | glossaire, plan | ≈ 31 |
| **Union** | **≈ 46** | **≈ 22** | 2 | **≈ 70** |

- **Écart avec la ligne du plan (39 : 27, 12)** : le majorant du plan compte les fichiers qui portent une ligne `ob` ou nomment les membres ; il ne voit pas les fichiers forcés par un changement de signature (`collect.c/.h` par `ho_apply`, `tech_gate.c/.h` par `gi`, `map_tables.c/.h` par la rangée étendue et `pico8_nb_lookup`, `data_init.c` par la vue étendue), ni les TU qui lisent `ui.ic` d'une autre classe (`fq`, `bg`, `fv`). **Au-delà de ~40 : découpe en trois chunks** (règle de la liste de D : « découpée au-delà de ~40 fichiers »), chacun sous 40.
- Rejeté : **deux chunks** (statiques / de partie). Le premier réunirait 33 fichiers de `src/` et ≈ 14 TU, au-delà du seuil.

### Effet sur `make data-residue`

Attendu après le dernier chunk (H17) :

| Catégorie | Avant (lignes / occurrences) | Après | Pourquoi |
|---|---|---|---|
| `stash` | 28/4 ; 28 | **28/4** (exact) | — |
| `ob` | 2037 hors test (2134) ; 2271 hors test (2376) | **≤ 1986 hors test ponctuel**, et < 2037 après chaque chunk ; le test ponctuel varie | membres et statiques `pico8_ob_table_t *`, parses à part, aides de défaut, `set_by_value`/`get_by_value`, lectures `get_index`/`get_name` des membres, branches ob de `lt`/`he`/`ei`, `ft_index` de `ui:gw` ; + les vues à l'appel |
| `overrides` | 450 ; 479 | **450** (exact) | aucune ligne `overrides` touchée |
| `dispatch` | 197 ; 199 | **197** (exact) | `pico8_mv` de `jo_fe_step` gardé |
| `spec` | 822 ; 890 | **< 822** | aides `*_default_table`, lectures `pico8_X_default("<membre>")`, `class_ref(pico8_X_default, …)` des TU |
| `par-nom` | 1080 ; 1135 | **1080, écart ≤ 3 expliqué** | remplacements un pour un (`button_take_table` → `button_take_nb`) ; les lecteurs par nom survivent |
| `type-tag` | 448 ; 734 | rapporté, attendu en baisse | − tests de tag de `lt`/`he`/`ei`, − asserts `OB_TABLE` des aides de défaut ; + tag de `jo_fe_step` |
| `ref-audit` | 70 ; 71 | **≤ 70**, rapporté | `PICO8_REF_AS_LIST_CONST` de `ui:jz` tombe avec la table `ic` |
| `valeur` | 6001/133 ; 6927 | rapporté | — |

- **Le majorant `ob` ≤ 1986 est attendu mais pas garanti (G)**, comme à D.4a : les vues en ajoutent. L'implémenteur écrit le compte exact N (hors test ponctuel) dans chaque commit et dans la note ; les majorants `ob` suivants se décalent de N − 1982. Au-dessus de 1986 : expliqué au commit, pas un arrêt ; **au-dessus de 2036, ou toute baisse obtenue par tassement : échec.**
- Aucune catégorie ne tombe à 0 : **`DATA_RESIDUE_GUARD` reste vide.** Aucun motif ne change.

### Hypothèses (rien n'a été lancé)

- **H1** : `ffb16a5` est vert (`make test`, `make e2e` 26 conformes, `make sanitize-net` 26 propres, `make class-coverage-net` identique, `make data-guard` 0) et `make data-residue` égale l'inventaire. Démenti au départ : arrêt, D.4b ne répare rien.
- **H2** : ce qu'`ob()` produit a la forme de l'inventaire (entiers seuls dans `pl`, `k_`, `kk` ; chaînes seules dans `nu`, `mr`, `gp` ; `fk` = `false, 2` ; rangées de dessin « positionnelles puis `fn` » ; non-entiers seulement dans `gs` ; `pc` = quatre vecteurs entiers). Le générateur le vérifie. Démentie : le type suit ce qu'`ob()` produit, le test ponctuel juge ; arrêt seulement si un membre ne tombe sous aucune forme.
- **H3** : aucun lecteur par nom de `src/` autre que ceux de l'inventaire n'atteint ces noms ; les aides à nom non littéral (`field_fixed(inst, name)` et semblables) ne reçoivent que des noms scalaires.
- **H4** : en jeu, les clés de `po`/`fu` sont des fixes (les 13 `qe` de `r`, plus au plus trois autres) ; aucun lecteur de `src/` ne garde `po`/`fu` après `lj`. Les TU du §6 (`kw.qe` = 99) restent sous la capacité.
- **H5** : les `id` écrits dans `hg` sont des chaînes `"p<n>"`, au plus 90 par voie.
- **H6** : les `fa` des entités de `cv("hd")` sont dans 0..15 ; aucun `ui:gw` ne s'exécute pendant le parcours de `ui:jz`, donc vider les couches sur place équivaut à une table neuve.
- **H7** : `hy:kt` ne lit de `t` que `px`, `od`, `nk` ; pour chaque enregistrement de la vue, `pico8_nb_lookup(px)` rend l'entrée de même `px`, `od`, `nk`.
- **H8** : un REF vers chaque type nouveau lit `PICO8_TYPE_TAG_UNKNOWN` (premier mot = compte ≤ 90, ou pointeur de chaîne).
- **H9** : aucun TU ni e2e ne dépend de `pico8_hp_fk()`, `pico8_pj_gs()`, `pico8_pj_i_()` NULL avant `init` ou après `shutdown` (précédent H8 de D.4a).
- **H10** : les `px` des projets à `ii` sont toujours présents (la rangée `hp.lq` n'a pas de trou).
- **H11** : les TU réécrits gardent leurs constantes `*_HASH`, **sauf** là où l'ancien pli portait le tag de type (ou le contenu table ob) d'un membre converti : repérés à l'écriture, candidats `test_pico8_hy` (`fold_fm` plie `po`/`fu`), `test_pico8_bg` (`pl`), `test_pico8_g_` (`fold_table` de `nu`/`k_`), `test_pico8_pj` (`pj_ref_is` … `OB_TABLE`), `test_pico8_ui`, `test_pico8_jc`, `test_pico8_mz`, `test_pico8_qb`, `test_pico8_mh`, `test_pico8_hp`. Chaque constante reprise l'est avec sa **sonde** : l'ancien pli reconstitué sur la donnée typée redonne exactement l'ancienne constante.
- **H12** : `make sanitize-net` rend 26 propres : aucun `free` d'une donnée statique, aucune lecture hors bornes (aides bornées par `count`), vider les listes de `ic` ne laisse aucun pointeur pendant.
- **H13** : `make class-coverage-net` est identique : aucune instanciation ajoutée ni retirée.
- **H14** : `-Werror` reste muet : les aides devenues sans appelant sont supprimées ; les casts `(void *)` sur des données `const` suivent la règle de D.3.
- **H15** : durées courtes : `make test` en quelques secondes plus une recompilation large (`map_tables.h`, `game_data.h`), `make e2e` ~1 s, `sanitize-net` et `class-coverage-net` < 2 min chacun, compilation comprise.
- **H16** : les chaînes générées (`nu`, `mr`, `gp`, rangées) ne sont comparées que par contenu (`cg` et `ri` en gardent des copies ou les comparent par `strcmp`).
- **H17** : `make data-residue` rend les comptes du tableau ; `ob` exact dépend de l'écriture des vues.
- **H18** : `test_pico8_spec_defaults` garde `STRUCT_HASH` si `same_value(spec, struct)` gagne la règle de `test_pico8_jo_spec_defaults` (D.4a) : quand la spécification rend une table ob et la struct un REF non tagué, vrai si et seulement si ce REF est la donnée générée depuis la spécification du module qui a répondu (`g_`, `fv`, `hp`, `pj`) ; pour les parses à part (`bg`, `qb`, `mh`, `jc`, `mz`), la relation d'avant était déjà « faux » (tables distinctes) et le reste ; le contenu est asserté à côté. `READY_HASH`, `SPEC_HASH` ne lisent que les spécifications ; `test_second_jl_keeps_specs` lit `pc` de la spécification de `qb`, inchangée.
- **H19** : l'ordre des couches et des entités dans `ic` est l'ordre d'insertion de `pico8_ft_index` (liste par couche, ajout en fin).
- **H20** : les vues de rangée de dessin rendent les mêmes tables que les défauts parsés : positionnelles, `fn` ; `bl` n'y lit que des positionnelles et `fn`.
- **H21** : la fuite des vues à l'appel (quelques petites tables par frame d'aide ou de panneau de score ouvert) reste invisible sur un e2e (< 180 frames) et relève de TROU-7.
- **H22** : `make raylib` (non exigé, jamais lancé) lierait : aucun fichier nouveau.

## Implementation Chunks

Trois commits, dans cet ordre ; chacun est vert seul (filet complet), porte ses comptes avant/après en lignes ET en occurrences, et ses preuves. Les cibles finales (« Sortie de l'étape ») ne s'appliquent qu'au dernier ; chaque chunk exige `ob` hors test ponctuel **strictement inférieur** à celui du chunk précédent, `stash`/`overrides`/`dispatch` exacts.

1. **D.4b-1 — étiquettes, palettes, vecteurs ; `lt`/`he`/`ei` à une forme.** `mh.nu`, `g_.nu`/`k_` (et `fv.k_`), `jc.nu`/`k_`, `mz.nu`/`kk`, `bg.pl` (`g_bg_pl`), `qb.pc` (`g_qb_pc`) ; type `pico8_vec_seq_t` ; générateur et test ponctuel pour ces objets ; branches ob de `lt`, `he`, `pico8_particle_field_k_` retirées. Aucune dépendance aux deux autres.
2. **D.4b-2 — rangées.** Extension `is_fixed`/`fn` de la rangée et de `pico8_jb_row_view` ; `fv.rb`, `pj.gs`/`i_`, `hp.mr`/`gp`/`fk`/`lq` ; `ho_apply` typé ; vues à l'appel de `fk`, `hp.lq`, `rb`, `i_`, `gs[idx]`. Dépend du générateur de 1 (même programme, étendu).
3. **D.4b-3 — tables de partie.** `hy.po`/`fu` (`pico8_qe_map_t`, `cm`/`bx`/`ck` remplissent la table fournie), `jc.hg` (`pico8_jc_hg_t`), `ui.ic` (`pico8_ui_ic_t`), `js._cr_t` (`pico8_nb_lookup`, `hy:kt` typé, `pico8_gi_nb` et calcul partagé avec `pico8_gi`). Note « Découvert à D.4b » du plan.

**Repli pour le pilote** : si un chunk dépasse 40 chemins de code à l'implémentation, `plan-pilot` le recoupe selon ses membres (chaque membre d'un chunk est indépendant des autres du même chunk, sauf `lt`/`he`/`ei`, qui ferment avec le dernier `nu`/`k_` du chunk 1).

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, ordres, pixels, pointeurs), pas la présence d'une ligne. D.4b change la représentation de vingt-trois membres : **la preuve est que les données générées égalent ce qu'`ob()` produisait, que les stockages de partie rendent les mêmes lectures que les tables, que chaque TU qui figeait ces membres fige les mêmes vérités sur la nouvelle forme, et que tout ce qui mesure le comportement rend la même chose.**

### Le test ponctuel `tests/test_pico8_map_tables.c`, étendu (G)

- **Il construit son état, il ne rejoue rien.** Ses 2 lignes `stash` ne changent pas ; il ne nomme ni `spec` (`g_X_spec`, `pico8_X_default`), ni `overrides`, ni `type-tag`.
- **Chaînes de référence** : celles du générateur, copiées octet pour octet de `ffb16a5`, parsées après `pico8_pi_init()`.
- **Pour chaque défaut généré** : la clé de même nom de la table parsée existe ; séquence d'entiers → mêmes positionnelles, même compte, chaque valeur `pico8_fixed_from_int(items[i])`, aucune nommée ; étiquettes/noms → mêmes chaînes (`strcmp`) dans l'ordre ; séquence de vecteurs → mêmes composantes ; rangée → `pico8_jb_row_view(&row)` égale à la table parsée par l'aide d'égalité profonde existante (positionnelles, `false`, non-entiers au bit près, `c`, `fn`).
- **`pico8_jb_row_view`** sur une rangée à item `is_fixed` et à `fn` : positionnelle fixe exacte, nommée `fn` après `c`.
- **`ho_apply` typé** : les deux passes de référence (`nb`, `df`) donnent les mêmes enregistrements qu'avant, `mr`/`gp` passés en séquences locales du test.
- **`pico8_nb_lookup`** : chacun des 19 `px` rend l'entrée de même index ; `"zz"` et NULL rendent NULL.
- **`pico8_qe_map_t`** (lié par `DATA_ENGINE_SRCS` si `economy_formulas.c` y est ; sinon dans `test_pico8_hy`) : `set` puis `get` sur une clé, clé absente, deux `set` sur la même clé (une entrée), ordre d'insertion, clé non entière (`flr`).

### TU touchés : lire la struct, comparer directement (G)

- **Règle** (celle de D.4a, plus « Anti-tassement ») : chaque lecture par nom d'un membre converti comparée à la spécification (`class_ref(pico8_X_default, …)`, `class_ref("gs")`, `pico8_qb_default("pc", …)` comme identité) devient une égalité directe avec la donnée générée de la classe, écrite à son site ; `po`/`fu` se lisent par `pico8_qe_map_get`, `hg` par l'ensemble, `ic` par ses couches ; les accesseurs (`pico8_pj_gs()`, `pico8_pj_i_()`, `pico8_hp_fk()`) comparés aux données générées.
- **Constantes `*_HASH`** : inchangées, sauf l'exception de D.4a (pli du tag de type, ou du contenu table ob, d'un membre converti), candidats en H11. Pour chaque constante reprise : l'ancienne et la nouvelle valeur, la cause, et la **sonde** (l'ancien pli reconstitué sur la donnée typée redonne l'ancienne constante) au commit. Toute autre constante qui bouge est un arrêt : l'implémenteur cherche la vérité perdue.
- **Identités de fraîcheur** : `test_pico8_hy` (`po`/`fu` « neufs à chaque `lj` », l. 493, 766-768) et `test_pico8_ui` (`ic` neuf à chaque `gw`, l. 944) deviennent des assertions de **contenu recalculé** : après un changement d'état (un `mu` de planète, une entité survolée qui change de couche), la lecture suivante rend la nouvelle valeur, et une entrée qui n'est plus vraie a disparu. L'identité de pointeur n'est plus assertée : elle ne décrit plus rien d'observable par le jeu (H4, H6).
- **`test_pico8_jc`** : `hg` distinct par voie (l. 534, 1354) reste vrai (stockage par struct) ; les lectures `hg[id]` passent par l'ensemble.
- **`test_pico8_hy`, `test_pico8_hp`** : `tech(px, od)` construit un `pico8_nb_entry_t` local (px NULL compris) au lieu d'une table ob.
- **`test_pico8_js`** : `_cr_t` lu comme `const pico8_nb_entry_t *`, égal à `pico8_nb_lookup(px)` de la rangée ; la liste hors rangée gagne la table ob (qui avorte désormais).
- **`test_pico8_spec_defaults`** : règle de `same_value` en H18 ; `STRUCT_HASH` inchangé.
- **`test_pico8_fq`, `test_pico8_bg`, `test_pico8_fv`** (`in_layer`, lecture de `ok.ic`) : lisent les couches de `pico8_ui_ic_t`.

### Preuve des tests (§7, gcov + mutations), par chunk

- **gcov** (copie jetable du scratchpad, `--coverage`) : chaque ligne ajoutée ou modifiée de `game_data.c`, `map_tables.c`, `data_init.c`, `entity.c`, `particle.c`, `economy_formulas.c`, `slipway.c`, `ui.c`, `research.c` est exécutée par un TU ; les lignes des lecteurs d'affichage (`planet_display.c`, `research_display.c`, `score_panel_display.c`, `slipway_display.c`, `wormhole_display.c`, `help.c` `fv:kj`) sont justifiées au commit par le TU ou l'e2e qui les atteint.
- **Mutations**, chacune doit rougir un test, puis être annulée :
  - chunk 1 — **M1** : `lt` saute la première étiquette → `cg("g_")` ou `cg("mh")` vide dans leur TU ; **M2** : `pico8_particle_field_k_` rend `pico8_g__k__seq` pour `fv` → `test_pico8_fv`/`test_pico8_g_` rouge ; **M3** : `qb` lit `pc` dans l'ordre inverse → `test_pico8_qb` rouge ;
  - chunk 2 — **M4** : `pico8_jb_row_view` omet `fn` → test ponctuel rouge ; **M5** : `pj.gs` perd un `false` (item 3 → rangée 5) → `test_pico8_pj` ou `test_pico8_mh` (« 2 years remain » / année muette) rouge ; **M6** : `ho_apply` ignore `gp` → `test_pico8_hp` rouge ;
  - chunk 3 — **M7** : `pico8_qe_map_set` ajoute une entrée au lieu de mettre à jour → `test_pico8_hy` rouge ; **M8** : `ui:gw` ne vide pas les couches → `test_pico8_ui`/`test_pico8_fq` rouge ; **M9** : `pico8_gi_nb` lit `lv` au lieu de `od` → `test_pico8_hy` ou `test_pico8_hp` rouge.

  Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée.

### Pas de nouvel e2e (G)

Les lignes nouvelles sont atteintes par les 26 e2e et par les TU ; un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 10 minutes de temps total de tests par chunk, 30 minutes pour l'étape.**

| Cible (par chunk) | Durée attendue |
|---|---|
| générateur (scratchpad, arbre de départ) | < 1 min, compilation comprise |
| `make test` | quelques secondes, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H15) |
| `make class-coverage-net` | < 2 min (H15) |
| gcov + 3 mutations (scratchpad) | < 4 min |
| `make data-guard`, `make data-residue`, recette d'occurrences | < 1 s |

Ordre conseillé dans chaque chunk : mesure de départ (lignes et occurrences) ; générateur **avant toute modification** ; types, données, test ponctuel, puis `make test` ; membres, lecteurs, vues, puis `make test` ; TU, puis `make test` ; `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois ; gcov, mutations ; mesure d'arrivée, contrôle anti-tassement, gardes, glossaire (et note du plan au chunk 3).

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + chunk + « données » / « membres » / « TU »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales de chaque chunk**, vertes sur l'arbre du chunk :

1. `timeout 5m make test` ;
2. `timeout 5m make e2e` : **26 « conforme »** ;
3. `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ;
4. `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ;
5. `make data-guard` : code 0 ;
6. `make data-residue` + recette d'occurrences : `stash` 28/4, `overrides` 450, `dispatch` 197, `ob` hors test ponctuel strictement en baisse (≤ 1986 au chunk 3), contrôle anti-tassement passé.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` du chunk liste les chemins de sa ligne « Empreinte remesurée » (écart ≤ 3 expliqué), rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements, `Makefile` inchangé, `gh_row_lq`/`ga_export_panel_by`/`layout_nil_item_draws_nothing` intacts.
2. **Membres** : forme du tableau « Forme par membre » ; plus aucun `pico8_ob_table_t *` parmi eux ; les stockages (`*_own`, `lq_inner`) ne sont ni lus ni écrits par nom.
3. **Données** : défauts dans `pico8_game_data.c`, un objet par table et par classe ; `pico8_game_data.c/.h` et `pico8_map_tables.c/.h` n'incluent ni `pico8_ob.h`, ni `pico8_value.h`.
4. **Formes uniques** : `lt`, `he`, `pico8_particle_field_k_` n'ont plus de branche ob ni de test de tag.
5. **`js._cr_t`** : pointe une entrée de `pico8_nb_table` ; `hy:kt` prend `const pico8_nb_entry_t *` ; `pico8_gi` et `pico8_gi_nb` partagent un seul calcul ; `pico8_nb()` n'est plus atteint par `js`.
6. **Vues** : toutes par `pico8_jb_row_view`, aucune libérée par erreur (`free` d'une donnée statique).
7. **Anti-tassement** : les comptes avant/après, en lignes ET en occurrences, par catégorie (et par fichier pour `--detail`), figurent au commit et sont recalculés par le vérificateur avec la recette ; chaque couple où Δlignes > Δoccurrences est justifié et relu ; aucune jonction de lignes, aucun alias d'identifiant compté, aucun déplacement hors du compteur, aucun commentaire sans rapport supprimé.
8. **TU** : plus aucun `class_ref(pico8_X_default, "<membre converti>")` ni enveloppe équivalente ; les comparaisons avec les données générées sont écrites à leur site, sans registre ; les constantes reprises le sont avec leur sonde.
9. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun récit de chantier.
10. **Docs** : glossaire à jour à chaque chunk ; note « Découvert à D.4b » en fin de ligne D.4b au chunk 3, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `ffb16a5` (ou un commit de docs au-dessus, ou le chunk précédent) propre, ou si `make data-residue` n'y rend pas l'inventaire.
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence.

**Périmètre**

- `FAILED:` si un membre du chunk reste un `pico8_ob_table_t *`, ou si une ligne des `grep` de la « Sortie de l'étape » subsiste au dernier chunk.
- `FAILED:` si une valeur générée a été écrite ou corrigée à la main, si un générateur est versionné, ou si `pico8_game_data.*`/`pico8_map_tables.*` incluent `pico8_ob.h` ou `pico8_value.h`.
- `FAILED:` si un stockage de partie est alloué par `malloc`, ou si une table ob est encore allouée pour `po`/`fu`, `hg`, `ic` ou `hp.lq`.
- `FAILED:` si le diff convertit une spécification, un défaut muté (`ea`, `ou`, `cr`), une clé de construction autre que `_cr_t`, `pi`/`bl`, `layout`, la vue `pico8_nb()` ou `pico8_gi(ob)`, ou touche un TROU autrement que par la disparition d'une allocation qu'il annotait.
- `FAILED:` si le test ponctuel nomme un motif de `spec`, d'`overrides` ou de `type-tag`, ou porte plus de 2 lignes `stash`.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD`, `tools/data_residue.sh` ou un motif change.
- `FAILED:` si `make data-residue` ne rend pas `stash` 28/4, `overrides` 450, `dispatch` 197 ; si `ob` hors test ponctuel ne baisse pas strictement à chaque chunk, ou dépasse 2036 au dernier ; si `spec` n'est pas < 822 au dernier ; si `par-nom` s'écarte de 1080 de plus de 3 ou sans explication.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés dans une ligne comptée, construction comptée déplacée sur une autre ligne comptée, alias (macro, `typedef`, enveloppe) d'un identifiant compté, code déplacé hors de `src`/`tests` ou hors `.c`/`.h`, commentaire sans rapport supprimé.
- `FAILED:` si le commit ne donne pas, par catégorie, les comptes avant/après **en lignes et en occurrences**, ou si un couple (catégorie, fichier) où Δlignes > Δoccurrences n'est pas justifié.
- `FAILED:` si un TU garde `class_ref(pico8_X_default, "<membre converti>")` (ou une enveloppe équivalente) pour un membre converti, ou si ses comparaisons passent par un registre (tableau, table ou boucle nommant ensemble lecteurs, membres ou données attendues).
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` change hors l'exception « pli du tag ou du contenu table ob d'un membre converti », ou sans sa sonde au commit.
- `FAILED:` si le rapport gcov ou une mutation du chunk manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si un chunk n'est pas un commit vert seul, ou si les chunks ne suivent pas l'ordre 1, 2, 3.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si le message de commit ne porte pas les comptes avant et après et la description du générateur.
- `FAILED:` s'il manque au chunk 3 la note « Découvert à D.4b », ou si elle omet un pont et son étape, ou le compte `ob` réel.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les spécifications de classe** et leurs défauts non table (D.5a, D.5b), dont `pico8_X_default("<membre>")`, qui rend toujours la table de la spécification.
- **Les défauts mutés de `pj`** (`ea`, `ou`, `cr`) et `nf.l`/`nf.r` (D.5a/D.5b, tableau fixe statique non `const`).
- **Les clés de construction** autres que `_cr_t` (D.6a à D.6c).
- **Les consommateurs ob** : `pi`/`bl` (D.7a), `layout`/`js` (D.6b), `menu`, `tech_gate`, la vue `pico8_nb()` et `pico8_gi(ob)` (D.7a), `collect` (`pico8_ft_index`/`ft_name`, sans appelant après le chunk 3 : D.7a), les tables ad hoc de `planet_display`, `research_display`, `score_panel_display` (D.7a), et avec eux les vues construites à l'appel.
- **Libérer les vues** construites à l'appel : TROU-7.
- **Le masque de bits de `nu`** : phase 2.
- `par-nom`, `dispatch`, `pico8_list_t` (les couches de `ic` restent des `pico8_list_t`) : D.9.
- `make raylib`, l'appli raylib, l'audio, le rendu, `make e2e-gap` et `tools/line_coverage_reference.txt`.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **forme par membre** : `nu` (`mh`, `g_`, `jc`, `mz`) → `const pico8_str_seq_t *` ; `k_` (`g_`, `fv`, `jc`), `kk`, `pl` → `const pico8_int_seq_t *` ; `pc` → `const pico8_vec_seq_t *` ; `mr`, `gp` → `const pico8_str_seq_t *` (et `ho_apply` typé) ; `fk`, `rb`, `gs`, `i_` → `const pico8_jb_row_t *` (rangée étendue de `is_fixed`/`fixed` et `fn`) ; `hp.lq` → `const pico8_jb_row_t *` + stockage dans `hp` ; `po`/`fu` → `pico8_qe_map_t *` + stockages dans `hy` ; `hg` → `pico8_jc_hg_t *` + stockage dans `jc` ; `ic` → `pico8_ui_ic_t *` + stockage dans `ui` ; `_cr_t` → `const pico8_nb_entry_t *`.
  - **défauts** : générés une fois dans `pico8_game_data.c`, un objet par table et par classe ; parses à part et aides `*_default_table`/`*_spec_table` supprimés.
  - **coexistence** : règle de D.4a ; `lt`/`he`/`ei` à une forme ; `mv(b,"lq")` gardé ; `hy:kt` typé, `pico8_gi` gardé pour D.7a avec un calcul partagé.
  - **ponts** : `pico8_nb()` n'atteint plus `js` ; vues à l'appel par `pico8_jb_row_view` pour `fk`, `hp.lq`, `rb`, `i_`, `gs[idx]`.
  - **tests** : test ponctuel étendu ; TU en comparaisons directes, constantes gardées sauf pli de tag (avec sonde) ; identités de fraîcheur → assertions de contenu ; pas d'e2e ; gcov et neuf mutations ; `class-coverage-net` et `sanitize-net` à chaque chunk.
  - **découpage** : trois chunks (≈ 70 chemins en tout), chacun sous 40.
  - **anti-tassement** : comptes en lignes ET en occurrences, contrôle par couple (catégorie, fichier), interdits nommés.
- **Note du plan (G)**, à ajouter au chunk 3 en fin de ligne D.4b (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp`) ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : N (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de N − 1982.

  N est le compte réel, hors test ponctuel. Les noms sont ceux de l'implémentation.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget de tests), anti-tassement compris.
- **Le message de commit** de chaque chunk porte : le périmètre ; la forme par membre et ses rejets ; le générateur en ≤ 15 lignes ; les comptes avant et après **en lignes et en occurrences** par catégorie, le compte `ob` hors test, et les couples Δlignes > Δoccurrences justifiés ; les constantes reprises avec leur sonde ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses qui le concernent.
- **Surprises pour le pilote** :
  - **L'empreinte réelle (≈ 70 chemins) dépasse de loin celle de la ligne (39)** : `ho_apply` (`collect`), `hy:kt`/`gi` (`tech_gate`), la rangée étendue (`map_tables`, `data_init`) et la lecture de `ui.ic` par les TU d'autres classes (`fq`, `bg`, `fv`) ne portent pas de ligne des membres. D'où trois chunks.
  - **Trois membres ne sont pas des défauts mais des tables de partie** (`hy.po`/`fu` recalculées à chaque `lj`, `jc.hg` remplie par l'économie, `ui.ic` refaite à chaque frame) et **un quatrième est par instance** (`hp.lq`) : la « première puce » (générer) ne s'y applique pas, la troisième (struct locale ou tableau fixe) oui ; le stockage dans la struct supprime au passage trois allocations par cycle, dont une par frame.
  - **`pj.gs` porte des `0.5` et deux `false`** : la rangée de D.2 ne tenait ni un non-entier ni un `fn` ; elle est étendue plutôt que doublée d'un type de plus.
  - **`js._cr_t` entraîne `hy:kt` et `gi`** : `kt` prend l'entrée typée, `gi` garde son entrée ob pour `menu`/`tech_gate` jusqu'à D.7a, un seul calcul pour les deux.
  - **Deux TU assertent l'identité d'une table « neuve à chaque appel »** (`po`/`fu` de `hy`, `ic` de `ui`) : elle devient une assertion de contenu recalculé ; garder l'identité par double tampon a été rejeté.
  - **Anti-tassement** : l'objectif `ob` ≤ 1986 demande −51 lignes hors test ponctuel ; il doit venir de constructions retirées, et la preuve en est donnée en occurrences à côté des lignes.
