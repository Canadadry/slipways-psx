---
title: "Plan 3, étape D.10b : les rappels (`js.my`/`hb`, `fq.my`, `mh.lh`, rangées de `menu`, `jo:fe`, `de.my`, `eu`, `init`) deviennent des pointeurs de fonction à la signature de leur unique appelant ; `hp.lq` perd son adaptateur ; `pico8_value_function_t`, `pico8_value_function()` et `PICO8_VALUE_FUNCTION` supprimés ; les 9 lignes de rappel des TU suivent sans pont, `dispatch` = 0 dans `DATA_RESIDUE_GUARD` ; deux chunks b1/b2"
description: "Après D.10a, les rappels passent encore par `pico8_value_function_t` (`self`, `argc`, deux `pico8_value_t`, retour `pico8_value_t`) et trois lecteurs par nom les rendent comme valeur : D.10b donne à chaque rappel le type exact que lit son unique appelant (`void (*)(pico8_js_t *)`, `int (*)(pico8_fq_t *)`, `int (*)(pico8_mh_t *)`), retire l'adaptateur de `hp.lq`, supprime le genre fonction de `pico8_value_t` et fait tomber `dispatch` à 0 (`src/` et TU) sans changer un pixel ; D.10c n'a plus d'objet."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.10a est vérifiée** (`plan-pilot`, étape 2b). D.10a est implémentée en deux commits locaux, `b7c719a` (a1) et `d05e35e` (a2, HEAD), non gradés. Tout ce qui suit a été lu et mesuré sur `git archive d05e35e` (arbre extrait dans le scratchpad) : `grep`, `sh tools/data_residue.sh`, `--detail` et la recette d'occurrences de D.8 seulement. **Rien n'a été compilé ni lancé.** Chaque résultat de D.10a dont ce PRD dépend est une hypothèse `H-D10a-n` (plus bas), et la section « Sections à revérifier » dit quoi relire quand D.10a sera gradée.

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données ») :

> - D.10b — `dispatch`, rappels : `js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, rappels des rangées de `menu` (`g_ix_my`, `g_ix_hb`, `ha`), `jo:fe`, `de.my` (`pico8_de_my`), `eu` de `help`, `init` de `score_panel` → pointeurs de fonction à la signature de leur unique appelant ; leurs lecteurs (`msgbox.c`, `research.c`, `ui_button.c`) ne les rendent plus comme valeur ; `PICO8_VALUE_FUNCTION`, `pico8_value_function()` et `pico8_value_function_t` retirés de `pico8_value.h/.c`. Empreinte 18 (18, 0) aujourd'hui. Compte : `dispatch` `src/` = 0, pont pour les TU excepté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non.

Lignes voisines du même groupe, recopiées (D.10a porte sa note « Découvert à D.10a » ; D.10b en vide la ligne D.10c, voir « Lecture ») :

> - D.10a — `dispatch`, table de méthodes : `methods[]` → struct de pointeurs typés, 23 emplacements nommés (Q2), et table d'états typée de `dg` (`ph`, `ly`, `ll`, `kz`, `pr` ; Q3) ; les 82 adaptateurs `(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1)` retirés ; les 26 appels `pico8_mv` de `src/` remplacés (appel direct de la fonction typée pour `eb`, `ib`, `gg`, `md`, `lq`, `lh` ; virtuel pour le reste) ; `dg`, `dc`, `ui:jz`, `lt`, `he` et les prédicats `*_is_*_class` sur les emplacements ; `pico8_entity_get_field` ne rend plus de méthode ; `hd=1` en fonction constante vraie ; `pico8_dispatch_context` gardé (Q4) ; sites `par-nom` G (9) et E de `mv` (1). Deux chunks : a1 (méthodes lancées par `dc`, `lt`, `he`, `ui:jz`, `dg`), a2 (le reste). Empreinte 37 (37, 0) aujourd'hui. Compte : `dispatch` `src/` ≤ 35 (67 moins les 29 lignes `pico8_mv`, `entity.c:177`, `methods[]` de `pico8_struct.h` et le commentaire de `pico8_dispatch_context.h`), ≤ 37 si `pico8_mv` reste en pont pour les TU ; `par-nom` ≈ 999 ; `valeur` rapporté. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non. Découvert à D.10a : deux chunks (a1 : 73 chemins ; a2 : 62). `pico8_struct_class_t` porte 14 membres typés (`kj`, `ev`, `hd`, `hk`, `hb`, `ldown`, `rdown`, `gv`, `hq`, `ey`, `kf`, `cn`, `ol`, `mm`), la table `states[]` (`pico8_state_t`, `pico8_state_of`) et le crochet `has_hb` (présence de `hb` par instance, pour `js`) ; `ldown`/`rdown`/`gv` rendent `pico8_down_t {stop, held}` ; `eb`, `ib`, `gg`, `md` sont appelés directement (`pico8_jo_is_family`) ; `hd=1` est `pico8_hd_true` ; `pico8_ui_jz` prend un sélecteur. Pas de pont : les TU ont suivi (les lignes `pico8_mv` et les lignes `pico8_value_function` de méthode, `fns[]` et aides `method()` compris) ; D.10c ne garde que les rappels des TU (9 lignes : `fq` 3, `js` 4, `mh` 1, `pj` 1, plus ce que D.10b y laisse) et perd ses lignes `pico8_mv`. D.10b hérite des 19 lignes d'adaptateurs de rappel (dont `pico8_hp_lq_dispatch`, `pico8_jo_fe_my_dispatch`) et des membres `my`/`hb`/`lh`/`lq`, appelés directement depuis D.10a ; le crochet `has_hb` survit au typage de `js.hb` ; le commentaire de `pico8_value_function_t` (`pico8_value.h`, « appelé par dc()/mv() ») est à réécrire avec lui. Sites G laissés : `ui_button.c:599` (D.10b), `slipway.c:261` et `research_icon.c:182` (D.11a), `ke.c:43` (D.11c) ; pris à D.11b (classe C) : la présence des noms `bm` dans `lt`/`he` et de `gv` dans `jc:hq`. `lt`/`he` assertent eux-mêmes le tag de struct (l'assert passait par `pico8_entity_get_field`). TROU-11 sans site. `dispatch` : `src/` 35 lignes / 37 occurrences / 15 fichiers, `tests/` 9 ; `par-nom` 986 (`src/` 419 au lieu de 420 : le commentaire de la déclaration de `pico8_mv` nommait aussi `pico8_entity_get_field`) ; `valeur` 4272 / 4639, `type-tag` 347 / 551, `ref-audit` 55 / 56 ; résidu D 5137 lignes dans 126 fichiers. Balayage ASan+UBSan des TU propre, `class-coverage-net` identique aux deux commits.
> - D.10c — `dispatch`, TU : 98 lignes `pico8_mv` → appels typés, 26 lignes `pico8_value_function` → rappels typés ; le pont éventuel de D.10a/D.10b supprimé. Empreinte 31 (0, 31). Compte : `dispatch` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change hors pont.

Décisions de D.9, recopiées :

> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Comptes au départ de D.10, avec la règle du pont des TU, recopiés :

> Comptes au départ de D.10 (HEAD 5a13008, `make -s data-residue` et `--detail` ; l'étude mesurait `2dec71f`, avant D.7a–D.8) : `valeur` 4961/126, `type-tag` 352/65, `ref-audit` 56/15, `par-nom` 1009/81 (`src/` 433/43, `tests/` 576/38), `dispatch` 191/61 (`src/` 67/30, `tests/` 124/31), résidu D 5868 lignes / 128 fichiers ; `liste`, pas encore au compteur, 927/99 avec le motif de Q10 (`src/` 496/61, `tests/` 431/38). Les classes A–M, T1/T2 et L1–L4 sont celles de l'étude (annexe A) ; ses « ≈ » viennent de son classement, chaque PRD remesure. Un TU qui nomme un symbole retiré par une étape de `src/` la suit dans le même commit, ou le symbole reste comme pont compté, fermé par l'étape TU de son groupe (D.10c, D.11d, D.12d) ; le PRD tranche. `sanitize-net` : aux seules étapes qui changent une durée de vie ou un propriétaire (§7).

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » versionnées à `d05e35e`, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b, D.5c, D.6a, D.6b, D.6c, D.7a, D.7b, D.7c, D.8 ; D.1 et D.9 n'en ont pas ; celle de D.10a est dans sa ligne, recopiée plus haut) :

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

> Découvert à D.8 : deux commits (octet de `jc`, non atteint : `test_pico8_jc.c` seul, chaque constante inchangée, un octet remis reste vert ; suppression — 26 chemins, dont `tools/float_guard.txt`, que `make float-guard` réécrit dans `make test` : `garde fichiers` 60 → 58, les deux fichiers du coeur supprimés, précédent D.2 ; 27 en tout). `test_pico8_map_tables.c` perd la comparaison à la référence (`decode`, `parse`, les `*_SRC`, les comparateurs, `pico8_load_map_json`, `pico8_pi_init` et `pico8_data_init` de `main`) et garde les tests des modules typés qui ne la lisent pas : six tels quels, et les bornes de `pico8_int_seq_at`/`pico8_fixed_seq_at` et les recherches `r`/`iz`/`hc`/`df` reformulées sur la table typée, sans assertion nouvelle. Les octets de sorte de clé de `hp` et `hy` sont pliés par une constante locale nommée (`VIEW_INDEX_KEY`, existante ; `TABLE_INDEX_KEY`, nouvelle), octet gelé d'une énumération supprimée, (c) déclarées ; aucune constante `*_HASH` ne change. `PICO8_TYPE_TAG_OB_TABLE`, `PICO8_REF_AUDIT_EXPECT_OB_TABLE` et le compteur `ob_table` de l'audit supprimés (`tools/ref_audit_report.sh` inchangé, son mot `ob_table` est mort) ; `pico8_ho_apply`, les constructeurs `'b'`/`'v'`/`'s'` et leur enregistrement supprimés ; TROU-4 et TROU-9 sans site. `ob` = 0, `stash` = 0, dans `DATA_RESIDUE_GUARD` ; `type-tag` 352 lignes / 560 occurrences (au lieu de ≈ 354 : les renvois de `pico8_list.h` et de `pico8_ref_audit.h` à l'explication du pas de lecture de `pico8_type_tag.h`, retirée avec la table ob, partent aussi), `ref-audit` 56 / 57, `valeur` 4961 / 5742 ; `par-nom` 1009, `dispatch` 191 inchangés ; résidu D 5868 lignes dans 128 fichiers. `sanitize-net` propre, `class-coverage-net` identique. **Sortie de la phase non atteinte** : `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h` restent, avec `valeur`, `type-tag`, `ref-audit`, `par-nom`, `dispatch` ; la suite attend les décisions humaines de D.9.

Puces de la phase D, recopiées (la quatrième est la sortie écrite de la phase) :

> - Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
> - Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
> - Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

§6, recopié :

> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
> - Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés** par ce plan. Depuis la fusion de master (e2e rapide, #32), seuls `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` restent ; les autres ont été remplacés par les e2e de `tests/e2e/`, qui en portent la garde. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.

§7, recopié :

> `plan-pilot`, phase après phase, un commit par classe (1a, 1b) ou par groupe (D, 2, 3).
>
> - **Une étape `plan-pilot`** = une classe en 1a/1b, ou une préparation sans conversion nommée dans la liste (1a.16b), un groupe nommé en D, 2 et 3. La liste ordonnée des étapes d'une phase (classes et instances sans classe en 1a/1b, dans l'ordre du §5 ; groupes en D, 2 et 3) est écrite dans ce plan à l'entrée de la phase, avant son premier commit — pour 1b, après 1b.0 et avant 1b.1, à partir de la table de 1b.0 ; elle fait foi sur le §2 quand ils divergent, et chaque PRD d'étape reprend sa ligne telle quelle.
> - Chaque commit de 1a passe `make class-coverage-net` (critère du PRD, hors `make test`), avec, pour chaque classe, un compte d'instanciations **égal** à celui de la porte 0.4 (valeurs versionnées à côté des exemptions à l'étape 1a.0). En 1b, seulement les étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]` ; toujours à la sortie de phase (note de sortie : une passe identique à `tools/class_coverage_net_expected.txt`).
> - **ASan+UBSan, en 1b** : chaque étape ajoute un build ASan+UBSan du binaire de rejeu, passé sur `FRAMES_EVENTS` ; critère : sortie propre, sans comparaison d'empreinte. Si la durée mesurée à 1b.0 est trop longue, seulement aux étapes qui touchent une table de membres ou une boucle d'init, et à la sortie de phase.
> - **Filets depuis la fusion de master (e2e rapide)** : `FRAMES_EVENTS` n'existe plus ; `make e2e` rejoue les 26 e2e de `tests/e2e/` depuis l'écran titre, `make sanitize-net` et `make class-coverage-net` rejouent ce même jeu, `tools/class_coverage_net_expected.txt` y a été reposé à la fusion ; aucun TU ne rejoue de fichier d'événements : un TU construit son état directement, un rejeu est un e2e de `tests/e2e/` (moins de 180 frames, et il doit ajouter de la couverture de lignes). Les mentions de `FRAMES_EVENTS`, `frames-check` et `float-audit` dans les étapes livrées sont historiques.
> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.
> - **Preuve des tests, à partir de 1a.23** : `gcov` (test de l'étape compilé en `--coverage` dans une copie jetable du scratchpad) montre que chaque ligne ajoutée ou modifiée par la conversion est exécutée par la caractérisation ; s'y ajoutent **2 à 3 mutations** ciblées sur ce que `gcov` ne prouve pas (valeur d'un défaut propre, entrée du descripteur remplacée, ordre du constructeur). Le vérificateur relit le diff et le rapport `gcov`, sans banc de mutations propre. Une ligne non exécutée est soit couverte par un test ajouté, soit justifiée dans le message de commit. En 1b, les mutations comprennent au moins : (1) indicateur de présence forcé vrai sur un membre à nil observé ; (2) lecture emballée d'un fixe typé qui rend « présent 0 » là où il était absent ; (3) quand ça s'applique, boucle d'init qui saute un membre typé. Chacune rougit un test, sinon le trou est justifié dans le message de commit.
> - **Note « Découvert à 1a.N », à partir de 1a.27** : la ligne du plan ne garde que ce qui change une étape suivante — entrée d'enum ajoutée, fonction laissée à double forme **avec l'étape qui la fermera**, lecture nouvelle d'une règle du §5 ; le reste (sites, comptes, mesures) va dans le message de commit.
> - **Nommage des PRD** : `plan3-phase<X>-etape<M>-*.md` (X ∈ `0`, `1a`, `1b`, `D`, `2`, `3`). « La plus haute » s'entend dans l'ordre du plan — `0` < `1a` < `1b` < `D` < `2` < `3`, puis `M` numérique, puis la lettre (`16` < `16b`, `42a` < `42b`) —, jamais dans l'ordre texte des noms. Seuls ces fichiers de `docs/prd/done/` comptent pour trouver l'étape suivante — les `phaseD-*`/`phaseE-*` de Plan 2 n'en sont pas. Les étapes 0.1 et 0.2 sont livrées, 0.3 est humaine : l'autopilote démarre à la porte 0.4, puis au premier commit de 1a.
>
> L'autopilote **s'arrête et demande à l'humain** quand :
>
> - le filet reste rouge après **3 tentatives** de correction sur le même commit ;
> - un champ ne tombe sous aucune règle (§5 : clé dynamique à ensemble non fini ; en phase 2, champ ni archétype ni état, ou groupe au classement ambigu) ;
> - un test de régression de bug devrait être modifié **dans son intention** (pas seulement dans sa façon de lire l'état) pour passer ;
> - la porte de l'étape 0 échoue ;
> - le diff modifie ou supprime un `.sha256` du filet, ou retire un e2e de `tests/e2e/` — `make test` resterait vert.
>
> Un site ou une instance sans classe découvert en cours d'étape et **couvert par une règle existante du §5** n'est pas un arrêt : l'autopilote l'ajoute à la liste des étapes dans le commit qui le traite, avec une ligne « découvert à 1a.M » dans le message de commit. Seul un site qu'aucune règle ne couvre déclenche l'arrêt.
>
> Un écart port/cart découvert en cours d'étape n'est pas un arrêt : il est **préservé à l'identique**, décrit par une nouvelle entrée `TROU-N` de `docs/trous-de-portage.md` (source, atteinte par le filet) et référencé par `/* TROU-N */` au site, dans le commit de l'étape. Le corriger est interdit dans ce plan ; si le préserver exige un champ qu'aucune règle du §5 ne couvre, c'est l'arrêt « champ sans règle ».
>
> Chacune de ces conditions est écrite dans le PRD de l'étape concernée comme critère que le vérificateur de `prd-autopilot` traduit en `FAILED:` — c'est le seul arrêt que `plan-pilot` honore.

### Hypothèses sur l'issue de D.10a (`H-D10a-n`)

Chacune est ce que ce PRD suppose de l'état laissé par D.10a ; les huit premières sont constatées sur `d05e35e`, aucune n'est encore confirmée par la vérification de D.10a.

- **H-D10a-1** : D.10a est gradée sur deux commits, `b7c719a` (a1) puis `d05e35e` (a2), ou sur des commits qui les amendent sans toucher aux rappels ; la note « Découvert à D.10a » est celle de la ligne D.10a recopiée plus haut.
- **H-D10a-2** : `dispatch` `src/` = **35 lignes / 37 occurrences / 15 fichiers**, toutes des rappels (par fichier : `help.c` 3, `help.h` 1, `menu.c` 4, `menu.h` 1, `msgbox.c` 1, `msgbox.h` 2, `research.c` 1, `score_panel.c` 3, `score_panel.h` 2, `score_panel_display.c` 1, `score_panel_display.h` 1, `ui_button.c` 3, `ui_button.h` 6, `value.c` 1 (2 occ.), `value.h` 5 (6 occ.)) ; `dispatch` `tests/` = **9 lignes / 9 occurrences** (`test_pico8_fq` 3 : l. 339, 430, 542 ; `test_pico8_js` 4 : l. 179, 182, 768, 1440 ; `test_pico8_mh` 1 : l. 372 ; `test_pico8_pj` 1 : l. 198) ; aucun `pico8_mv`, aucun pont.
- **H-D10a-3** : `par-nom` = 986 lignes / 1102 occurrences (`src/` 419, `tests/` 567) ; `valeur` 4272 / 4639 ; `type-tag` 347 / 551 ; `ref-audit` 55 / 56 ; résidu D 5137 lignes / 126 fichiers ; `DATA_RESIDUE_GUARD := spec,overrides,stash,ob`.
- **H-D10a-4** : le site G laissé à D.10b est le déballage du retour de `fq.my` par `button_take_bool(my_result, &self->p_, &self->has_p_)` dans `pico8_fq_ldown` (`ui_button.c:599` dans la note, **l. 603** sur `d05e35e`).
- **H-D10a-5** : le commentaire de `pico8_value_function_t` (`pico8_value.h` l. 27-38) nomme encore `dc()`/`mv()` ; D.10a l'a laissé à D.10b.
- **H-D10a-6** : `lh`, `my`, `hb` sont appelés directement par leur membre, encore typé `pico8_value_function_t` : `msgbox.c:91` (`mh:ev`), `ui_button.c:68` (`pico8_js_hb_dispatch`), `:486` (`js:ldown`), `:601` (`fq:ldown`) ; `hp.lq` NULL est remplacé par l'appel direct de `pico8_hp_lq` dans `jo:fe` (`planet_display.c`, `pico8_jo_fe_keys`).
- **H-D10a-7** : le crochet `has_hb` de `pico8_struct_class_t` existe (`pico8_js_has_hb`, `ui_button.c:60`) et sert à `lt`/`he` ; D.10b le garde.
- **H-D10a-8** : les 19 lignes d'adaptateur de rappel sont à leur place (`menu.c` 39 — déclaration avancée —, 103, 297, 310, 323, 354, 373, 389, 398, 416 ; `cart.c` 149, 204 et leurs déclarations `cart.h` 30, 39 ; `help.c` 21 ; `score_panel.c` 150 ; `score_panel_display.c` 55, 116, 131 ; `research.c` 73 ; `planet_display.c` 319), plus 5 rappels de TU (`test_pico8_js` `counting_my`, `test_pico8_fq` `counting_my`/`scripted_my`, `test_pico8_mh` `counting_lh`, `test_pico8_pj` `pico8_pj_ea_lh_witness`).
- **H-D10a-9** : la vérification de D.10a ne change aucun des fichiers de l'empreinte de D.10b au-delà de numéros de ligne ; si elle en change un (correctif de a2), les sections ci-dessous marquées ⟲ sont à remesurer.
- **H-D10a-10** : `test_pico8_bg`, `test_pico8_fv`, `test_pico8_hp`, `test_pico8_ui` n'ont pas été réécrits par D.10a dans les lignes que D.10b touche (appel `pico8_ix_my(i)(NULL, 0, nil(), nil())`, lectures de `"my"`/`"lq"`, `pico8_eu_field_value()`).

### Sections à revérifier quand D.10a sera gradée

Comparer à la note finale de D.10a et au HEAD gradé, puis corriger ce PRD avant de le passer en `docs/prd/triage/` :

1. ⟲ **État de départ** (comptes, occurrences, `par-nom` 419, résidu D) et le critère `FAILED:` de départ (HEAD attendu).
2. ⟲ **Inventaire** : numéros de ligne des rappels, des lecteurs (`ui_button.c:89, 159, 513`, `msgbox.c:133`, `research.c:202`), du site G (`ui_button.c:603`) et des 9 lignes de TU.
3. ⟲ **Classification attendue** (a)/(b)/(c) et la table « Effet sur `make data-residue` ».
4. ⟲ **Empreinte et découpage** (listes de fichiers de b1/b2).
5. **« D.10c devient vide »** : ne tient que si D.10a n'a laissé aucun `pico8_mv` ni pont dans les TU (H-D10a-2).
6. **Constantes** : la liste des constantes touchées (H3) suppose les TU de `d05e35e`.
7. La note « Découvert à D.10b » (gabarit en « Further Notes »), qui cite les comptes de départ.

### Lecture de ces textes pour D.10b

- **Périmètre** : la ligne nomme neuf rappels et trois lecteurs, et la suppression du genre fonction de `pico8_value.h/.c`. À `d05e35e` (H-D10a-2) les 35 lignes `dispatch` de `src/` sont exactement ces rappels, leurs lecteurs et la déclaration du type : **rien d'autre ne porte la catégorie**.
- **« à la signature de leur unique appelant »** : un rappel est un membre (ou un statique) qui n'a qu'un lanceur dans le jeu. Relevé à `d05e35e` : `js.my` → `js:ldown` ; `js.hb` → `pico8_js_hb_dispatch` (emplacement `hb` de la classe `js`, lancé par `hy:lj`) ; `fq.my` → `fq:ldown` ; `mh.lh` → `mh:ev`. Les rangées de `menu`, `jo:fe`, `eu` et `init` n'ont pas d'appelant propre : ce sont des **valeurs installées dans `js.my`** (ou `js.hb` pour `ix[2]`), donc de son type. **Exception : `de.my`** — la table `de` du cart est passée à la fois à `js(ob(…, de))` (`pj:ph`, deuxième bouton) et à `fq(de)` (`ix[3].my`) : sa fonction est stockée dans `js.my` **et** dans `fq.my`, deux appelants de signatures différentes (décision plus bas).
- **`hp.lq`** : le membre `hp.lq` est une **donnée** (la rangée `lq_own`), pas une fonction ; la méthode `hp:lq` qu'il masque est `pico8_hp_lq`, appelée directement par `jo:fe` depuis D.10a (H-D10a-6). Il ne reste qu'un adaptateur, `pico8_hp_lq_dispatch`, qui ne sert plus qu'au lecteur par nom (`research.c:202`) : il n'y a **aucun pointeur de fonction à créer** pour `hp.lq`, seulement l'adaptateur et la branche du lecteur à retirer.
- **« leurs lecteurs ne les rendent plus comme valeur »** : les lecteurs par nom `pico8_js_field` (`"my"`, `"hb"`), `pico8_fq_field` (`"my"`), `pico8_mh_field` (`"lh"`) et `pico8_hp_field` (`"lq"` quand le membre est NULL) cessent de répondre pour un rappel ; le nom lit alors « absent », comme un nom de méthode depuis D.10a (`pico8_entity_get_field` ne rend plus de méthode). Aucun code de `src/` ne lit ces noms par nom (grep à `d05e35e` : seuls les lecteurs eux-mêmes les nomment) ; seuls des TU les lisent.
- **Pont des TU** : le paragraphe des comptes laisse le choix. Tranché plus bas : **aucun pont** ; les 9 lignes de rappel des TU suivent dans le chunk qui change leur type. La ligne D.10c (« 98 lignes `pico8_mv` → appels typés, 26 lignes `pico8_value_function` → rappels typés ; le pont éventuel supprimé ; `dispatch` = 0 ajouté à `DATA_RESIDUE_GUARD` ») est alors **entièrement absorbée** : D.10a a pris les 98 lignes `pico8_mv` et 17 des 26, D.10b prend les 9 dernières et l'ajout à la garde. **D.10c devient vide** (voir « Further Notes » pour ce que le pilote en fait).
- **Compte** : « `dispatch` `src/` = 0, pont pour les TU excepté ». Sans pont : `dispatch` = **0 partout**, et la règle commune (« une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit ») s'applique à D.10b : **le `Makefile` change d'une ligne** (`DATA_RESIDUE_GUARD`).
- **« Empreinte 18 (18, 0) »** : 18 fichiers de `src/` (les 15 qui portent `dispatch` plus `cart.c/.h` et `planet_display.c`, qui portent les rappels sans le motif) — remesuré, exact. Le « 0 » de `tests/` ne compte que le motif `dispatch` : retirer `PICO8_VALUE_FUNCTION` de l'énumération touche en plus **25 TU** qui le nomment (bras `case PICO8_VALUE_FUNCTION:` des plis, cas hors rangée `zero_union(PICO8_VALUE_FUNCTION)`, lectures par nom d'un rappel), comptés par `valeur`, pas par `dispatch`.
- **« §6 : aucun »** : `test_pico8_planet_gh_row_lq`, `test_pico8_ga_export_panel_by` et `test_pico8_layout_nil_item_draws_nothing` ne nomment ni `PICO8_VALUE_FUNCTION`, ni `pico8_value_function`, ni un rappel, ni `"my"`/`"lh"`/`"hb"` (grep à `d05e35e`) : aucun n'est touché.
- **« `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non »** : les constructeurs de `js`, `fq`, `mh` copient les rappels (`self->my = keys->my`…) sans que leur texte ait à changer ; aucune durée de vie ni aucun propriétaire ne change. Balayage ASan+UBSan des TU à la fin (consigne du pilote).
- **Leçons des notes à ne pas refaire** : classification (a)/(b)/(c) absente ou fausse, (c) non déclarée, lignes qui gagnent une catégorie omises (correctifs de D.7a) ; conversion typée → valeur ajoutée le temps d'un commit (D.7a, point 3) ; constante de hachage « mise à jour » ; octet gelé remplacé par un littéral nu (arbitrage de D.7c, résolu à D.8 par une constante locale nommée, (c) déclarée) ; TU corrompu qui passait en relisant sa propre corruption (D.7b : balayage ASan des TU).
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; aucune jonction de lignes, aucun alias, aucun registre, aucun renommage pour échapper aux greps, aucune cale typée ↔ valeur ; **aucune conversion de pointeur de fonction** ; mutations restaurées par `cp` ; balayage ASan+UBSan des TU à la fin ; jamais l'appli raylib.

### État de départ ⟲

**Mesuré sur `d05e35e`** (arbre archivé, D.10a non gradée). `make -s data-residue` n'a pas été lancé (pas de build) : `sh tools/data_residue.sh` et `--detail` directement, et la recette d'occurrences de D.8.

| Catégorie | Lignes | Fichiers | Occurrences | `src/` (lignes) | `tests/` (lignes) |
|---|---|---|---|---|---|
| `valeur` | 4272 | 125 | 4639 | — | — |
| `type-tag` | 347 | 64 | 551 | — | — |
| `ref-audit` | 55 | 15 | 56 | — | — |
| `par-nom` | 986 | 81 | 1102 | 419 | 567 |
| `dispatch` | 44 | 19 | 46 | 35 (37 occ., 15 fichiers) | 9 (9 occ., 4 fichiers) |
| `ob`, `stash`, `overrides`, `spec` | 0 | 0 | 0 | — | — |
| **résidu D** | **5137** | **126** | — | — | — |

### Inventaire (mesuré sur `d05e35e`) ⟲

**Le type.** `pico8_value.h` : `PICO8_VALUE_FUNCTION` (dernier de `pico8_value_kind_t`, l. 19), commentaire de la déclaration avancée (l. 22), commentaire l. 27-38 et typedef `pico8_value_function_t` (l. 39), membre `function` de l'union (l. 53), `pico8_value_function()` (l. 61), deux mentions dans les commentaires de `pico8_value_equal` (l. 71) et `pico8_value_tostr`. `pico8_value.c` : définition (l. 36), bras `FUNCTION` de `pico8_value_equal` (l. 56-57) et de `pico8_value_tostr` (l. 112).

**Rappels : stockage, valeurs installées, appelant.**

| Rappel | Stockage | Valeurs installées (`src/`) | Valeurs de TU | Unique appelant | Retour lu |
|---|---|---|---|---|---|
| `js.my` | `pico8_js_t.my` + `has_my` (présent et NULL = `false`) ; `pico8_js_keys_t.my`/`has_my` | `ix[1..4]` (`g_ix_my[]`), rangées de `ha` (`init`, `ha2`, `ha3`, `ha4`, tableau local `my[]` de `pico8_ix4_my_dispatch`), rangées de `cr` (`pico8_cr_row_my_dispatch`), rangées de `bo` (`eu`), `pj:ph` (`init`, puis `de.my`), `jo:fe` (`pico8_jo_fe_my_dispatch`) | `counting_my` (`js`, rend le fixe 7) | `pico8_js_ldown` (`ui_button.c:486`) | ignoré |
| `js.hb` | `pico8_js_t.hb`, `pico8_js_keys_t.hb` | `ix[2]` (`g_ix_hb[1]` = `pico8_ix2_hb_dispatch`) | — | `pico8_js_hb_dispatch` (`ui_button.c:68`) | ignoré |
| `fq.my` | `pico8_fq_t.my`, `pico8_fq_keys_t.my` (NULL = nil) | `ix[3]` : `fq(de)` → `pico8_de_my()` | `counting_my`, `scripted_my` (`fq`) | `pico8_fq_ldown` (`ui_button.c:601`) → `p_` par `button_take_bool` (site G, l. 603) | nil ou booléen (`assert` sinon) ; `dg` n'en lit que la vérité (`if qc.p_`) |
| `mh.lh` | `pico8_mh_t.lh`, `pico8_mh_keys_t.lh`, `pico8_pj_ea_t.lh` (état `g_pj_ea`) | `pj:hk` (`pico8_pj_hk_lh_dispatch`, `t>0`), `pj:ph` (`pico8_pj_ph_lh_dispatch`, `ju.c_`) | `counting_lh` (`mh`), `pico8_pj_ea_lh_witness` (`pj`) | `pico8_mh_ev` (`msgbox.c:91`), vérité par `pico8_msgbox_truthy` | vérité |
| `hp.lq` | membre donnée `const pico8_jb_row_t *lq` | — (repli : méthode `pico8_hp_lq`, appelée directement par `jo:fe`) | — | aucun appel par valeur : `pico8_hp_lq_dispatch` (`research.c:73`) n'est plus que la valeur que rend le lecteur (`research.c:202`) | — |
| `eu` | `g_eu_dispatch` (`help.c:29`), `pico8_eu_field_value()`, `pico8_help_set_eu()` ; `cart.c:92` installe `pico8_eu_dispatch` | — | — | copié dans `js.my` des rangées de `bo` (`pico8_fv_pq`) | — |
| `init` | `g_init_dispatch` (`score_panel.c:158`), `pico8_init_field_value()`, `pico8_score_panel_set_init()` ; `cart.c:93` installe `pico8_init_dispatch` | — | — | copié dans `js.my` (`pj:ph`, `ha[1]`) | — |
| `de.my` | `pico8_de_my()` (`score_panel_display.c:373`) rend `pico8_pj_de_my_dispatch` (bascule `ju.c_`, rend `true`) | — | appelé nu par `test_pico8_pj` l. 343, installé par `test_pico8_fq` | **deux** : `js.my` (`pj:ph`, `spec2.my`) et `fq.my` (`ix[3]`) | ignoré (`js`) / vérité (`fq`) |

**Lecteurs qui rendent un rappel comme valeur** : `pico8_js_my()` (`ui_button.c:82-90`, public, seul appelant le lecteur `"my"` l. 146), la branche `"hb"` (l. 155-160), la branche `"my"` de `pico8_fq_field` (l. 509-514), la branche `"lh"` de `pico8_mh_field` (`msgbox.c:129-133`), le repli fonction de `"lq"` de `pico8_hp_field` (`research.c:200-203`). S'y ajoutent les deux accesseurs qui rendent `eu`/`init` emballés (`pico8_eu_field_value`, `pico8_init_field_value`), lus seulement par `.as.function` (`help.c:69`, `score_panel_display.c:298`, `menu.c:418`, `cart.c:131-132`).

**Sites `par-nom` touchés** : le seul site G de la ligne, `ui_button.c:603` (retour de `fq.my`). Les lectures par nom **dans** les rappels restent (classe A de l'étude, D.11a) : `_cr_t` de `pico8_cr_row_my_dispatch` (`menu.c`), l'écriture de `lq` par `pico8_ix2_hb_dispatch` (`menu.c`), `_jo_fe_pt`/`_jo_fe_b`/`_jo_fe_od` de `pico8_jo_fe_my_dispatch` (`planet_display.c`).

**TU** (grep à `d05e35e`) : les 9 lignes `dispatch` (H-D10a-2) ; **25 TU** nomment `PICO8_VALUE_FUNCTION`, `.as.function`, `pico8_value_function`, `pico8_eu_field_value`, `pico8_init_field_value`, `pico8_de_my` ou `pico8_ix_my` : `bg`, `entity_kind`, `fq`, `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `iv`, `jc`, `jo`, `js`, `lq`, `mh`, `mx`, `mz`, `nl`, `nr`, `os`, `pj`, `pm`, `qb`, `qh`, `ui`. Ils s'y rattachent par : (i) un bras `case PICO8_VALUE_FUNCTION:` de leur pli (`fold_value` et semblables, 18 TU) ; (ii) des cas hors rangée qui offrent un `zero_union(PICO8_VALUE_FUNCTION)` ou une valeur fonction à un écrivain par nom qui doit s'arrêter (`g_`, `ga`, `jc`, `mz`, `ui`, `js`, `fv`, `iv`, `fq`, `bg`, `entity_kind`, `nl`, `qb`) ; (iii) des lectures par nom d'un rappel (`js` `"my"`/`"hb"`, `fq` `"my"`, `mh` `"lh"`, `hp` `"lq"`, `fv` `"my"`) ; (iv) des appels de rappel à quatre arguments (`js` `call_my`, `mh`, `pj` `de_my`/`ix3_my`, `ui` `call_ix_my`). Des constantes plient le **genre** d'un rappel lu par nom (`(uint32_t)v.kind` = 4, valeur de `PICO8_VALUE_FUNCTION`) : `test_pico8_fq` (`fold_raw`, `FQ_MEMBERS_HASH`), `test_pico8_mh` (`fold_member`, `MEMBERS_BY_NAME_HASH`), `test_pico8_hp` (`GB_HASH`, `RAW_PRESENCE_HASH`, `FUNCTIONS_HASH`), `test_pico8_js` (`fold_js_members`) ; d'autres plient son octet de format `0xFC` (`test_pico8_js` avec `fn_index`, `test_pico8_pj` `fold_ea`, `test_pico8_hp` `fold_value`).

## Solution

**Deux commits (chunks b1, b2), chacun vert seul**, sans pont :

1. **b1 — rappels typés.** Trois types de rappel, chacun celui de son appelant : `js.my`/`js.hb` `void (*)(pico8_js_t *self)`, `fq.my` `int (*)(pico8_fq_t *self)`, `mh.lh` (et `pj.ea.lh`) `int (*)(pico8_mh_t *self)`. Toutes les valeurs installées prennent ce type (même nom de fonction) ; `eu` s'installe par `pico8_eu` lui-même (`pico8_eu_dispatch` part) ; `de.my` devient deux fonctions, une par type, sur un même corps ; `pico8_hp_lq_dispatch` part. Les lecteurs par nom ne répondent plus pour un rappel ; `pico8_js_my()` part ; `fq:ldown` pose `p_` depuis l'entier rendu (site G retiré). Les 9 lignes `dispatch` des TU et les TU qui appellent, installent ou lisent par nom un rappel suivent (8 TU). `dispatch` : `src/` 35 → 6 (le type seul, dans `pico8_value.h/.c`), `tests/` 9 → 0.
2. **b2 — suppression du genre fonction.** `PICO8_VALUE_FUNCTION`, le membre `function` de l'union, `pico8_value_function_t`, `pico8_value_function()`, leurs bras dans `pico8_value_equal`/`pico8_value_tostr` et les commentaires qui les nomment ; les 17 TU restants perdent leurs bras `case PICO8_VALUE_FUNCTION:` (morts depuis b1) et leurs cas hors rangée devenus inexprimables ; `dispatch` = 0, ajouté à `DATA_RESIDUE_GUARD` ; note « Découvert à D.10b » ; balayage ASan+UBSan des TU.

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (`make data-guard` : `résidu gardé (spec,overrides,stash,ob,dispatch) : 0 ligne(s)`) ; `make e2e` : **26 conformes** ; balayage ASan+UBSan des TU propre ; `make class-coverage-net` identique à l'attendu si et seulement si un chunk touche un constructeur (attendu : non) ;
- `make data-residue` : `dispatch` **0** (`src/` et `tests/`) ; `par-nom` `src/` = **418** (419 − le site G) ; `valeur`, `type-tag`, `ref-audit` rapportés ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/` ni de `tools/` touché ; aucune constante `*_HASH`/`identity[]` changée hors `FQ_MEMBERS_HASH`, reprise avec la sonde décrite plus bas (et toute autre seulement avec la même sonde, déclarée) ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  # 1. plus aucun type, constructeur ni genre « fonction » de pico8_value : vide
  grep -rnE '\bpico8_value_function(_t)?\b|\bPICO8_VALUE_FUNCTION\b|\.as\.function\b' src tests --include='*.[ch]'
  # 2. plus aucun rappel à arguments pico8_value_t (adaptateurs de src/ et de TU) : vide
  grep -rnE 'int argc, pico8_value_t arg0, pico8_value_t arg1' src tests --include='*.[ch]'
  # 3. aucune conversion de pointeur de fonction : vide (un prototype nomme ses paramètres,
  #    sinon `(pico8_js_fn)` y serait pris pour un cast)
  grep -rnE '\(\s*[a-z0-9_ ]+\s*\(\s*\*\s*\)\s*\(' src tests --include='*.[ch]'
  grep -rnE '\((const )?pico8_[a-z0-9_]+_fn(_t)?\)' src tests --include='*.[ch]'
  # 4. plus aucun accesseur de rappel emballé ni adaptateur de hp.lq : vide
  grep -rnE '\bpico8_js_my\b|\bpico8_hp_lq_dispatch\b|\bpico8_eu_dispatch\b' src tests --include='*.[ch]'
  # 5. comptes et garde
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^dispatch'; grep -n '^DATA_RESIDUE_GUARD' Makefile
  ```

## User Stories

1. En tant que mainteneur du port, je veux que chaque rappel ait le type exact que lit son unique appelant, afin que le compilateur vérifie chaque installation et chaque appel.
2. En tant que mainteneur du port, je veux qu'aucun appel ne passe par un pointeur de fonction converti vers un autre type, afin qu'aucun appel ne soit un comportement indéfini.
3. En tant que mainteneur du port, je veux que `js.my` et `js.hb` reçoivent le bouton typé (`pico8_js_t *`) et ne rendent rien, afin que `eu` s'installe tel quel et que les rappels de `menu` et de `jo:fe` n'aient plus d'arguments fictifs.
4. En tant que mainteneur du port, je veux que `fq.my` rende la vérité que `fq:ldown` range dans `p_`, afin que le déballage `button_take_bool` du retour (site G) disparaisse.
5. En tant que mainteneur du port, je veux que `mh.lh` et `pj.ea.lh` rendent la vérité que `mh:ev` teste, afin que la boîte de message ne passe plus par une valeur étiquetée.
6. En tant que mainteneur du port, je veux que `de.my`, stocké à la fois dans un bouton et dans un fond modal, existe une fois par type sur un même corps, afin de ne convertir aucun pointeur de fonction.
7. En tant que mainteneur du port, je veux que `hp.lq` ne garde que sa donnée et que la méthode `hp:lq` reste appelée directement, afin que `pico8_hp_lq_dispatch` disparaisse.
8. En tant que mainteneur du port, je veux que `eu` et `init`, installés par `pico8_cart_load` dans `help` et `score_panel`, soient des pointeurs typés, afin que le pont de dépendance (ces modules ne lient pas `cart.c`) reste sans valeur étiquetée.
9. En tant que mainteneur du port, je veux que les lecteurs par nom ne rendent plus de rappel, afin qu'un lecteur par nom ne lise que des données.
10. En tant que mainteneur du port, je veux que `PICO8_VALUE_FUNCTION`, `pico8_value_function()` et `pico8_value_function_t` disparaissent, afin que `pico8_value_t` ne porte plus de fonction.
11. En tant que mainteneur du port, je veux que le crochet `has_hb` survive, afin qu'un bouton sans `hb` reste hors de `cv.hb`.
12. En tant que gardien du compteur, je veux `dispatch` = 0 dans `src/` et `tests/`, et `dispatch` dans `DATA_RESIDUE_GUARD` dans le même commit, afin que la catégorie ne puisse pas revenir.
13. En tant que gardien du compteur, je veux `par-nom` `src/` = 418 exactement, la seule ligne retirée étant le site G, afin qu'aucun site de D.11 ne soit « typé » en déplaçant son déballage.
14. En tant que gardien du compteur, je veux, au message de chaque commit, les comptes avant/après en lignes ET en occurrences par catégorie et par fichier, la classification (a)/(b)/(c) des lignes `dispatch` et `par-nom` retirées, les lignes qui gagnent une catégorie par fichier (« aucune » compris) et la justification de chaque Δlignes > Δoccurrences, afin qu'aucune baisse ne soit un tassement.
15. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, aucun alias, registre ni renommage ajouté pour échapper aux greps, et qu'aucune conversion typée ↔ valeur ne reste comme cale, afin que la baisse veuille dire « retiré ».
16. En tant que gardien des TU, je veux que chaque TU qui installe, appelle ou lit par nom un rappel suive dans le chunk qui change son type, même intention, afin qu'aucun pont ne survive et que D.10c n'ait plus rien à faire.
17. En tant que gardien des TU, je veux que chaque cas de test rendu inexprimable (valeur fonction offerte à un écrivain, retour de `fq.my` nil ou d'un autre genre qu'un booléen) soit retiré et listé au message avec la raison, afin qu'aucune couverture ne disparaisse sans trace.
18. En tant que gardien des constantes, je veux qu'aucune constante `*_HASH` ne change, chaque pli d'un rappel lu par nom repliant les mêmes octets depuis le membre typé, afin que l'identité de l'état reste prouvée.
19. En tant que gardien des constantes, je veux qu'une constante dont le scénario perd un cas inexprimable (`FQ_MEMBERS_HASH`) ne soit reprise qu'avec une sonde qui prouve que la seule différence est ce cas, afin qu'aucune mise à jour ne masque un changement d'état.
20. En tant que gardien des filets, je veux `make test` et `make e2e` verts à chaque commit et le balayage ASan+UBSan des TU propre à la fin, afin de prouver qu'aucun comportement n'a changé.
21. En tant que lecteur du glossaire, je veux que les lignes `my`, `de` et `de.my` disent ce que deviennent ces rappels, afin que le glossaire ne renvoie pas à une valeur fonction.
22. En tant que `plan-pilot`, je veux une note « Découvert à D.10b » qui dise que D.10c n'a plus d'objet et ce que D.11a hérite (lectures par nom dans les rappels, sur un `self` désormais typé), afin que les PRD suivants partent d'un état exact.
23. En tant que `plan-pilot`, je veux les inconnues d'exécution écrites en hypothèses H1…Hn, et celles sur D.10a en `H-D10a-n`, afin que le vérificateur et le pilote sachent quoi confirmer.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q1-Q11 sont des décisions humaines (plan, « Décisions de D.9 ») : ce PRD les applique ; Q12 reste ouverte et n'est pas tranchée ici (voir « Q12 »).

### Modules

- **`value`** (`pico8_value.h/.c`) : perd le genre fonction (b2). Module profond de la couche : son interface (constructeurs, `pico8_value_equal`, `pico8_value_tostr`) ne change pas pour les quatre genres restants.
- **`ui_button`** : les types de rappel de `js` et `fq`, leurs appelants (`js:ldown`, `pico8_js_hb_dispatch`, `fq:ldown`), les lecteurs `"my"`/`"hb"`.
- **`msgbox`** : le type de `lh`, son appelant (`mh:ev`), le lecteur `"lh"`.
- **`research`** : l'adaptateur de `hp.lq` et le repli du lecteur.
- **`menu`**, **`help`**, **`score_panel`**, **`score_panel_display`**, **`cart`**, **`planet_display`** : les valeurs installées et les ponts `eu`/`init`.
- **Aucun module nouveau.** Les fonctions de rappel gardent **leur nom** (`pico8_ix1_my_dispatch`, `pico8_cr_row_my_dispatch`, `pico8_jo_fe_my_dispatch`, `pico8_pj_hk_lh_dispatch`…) : le changement est de type, pas de nom, comme à D.10a. Rejeté : renommer (churn sans rien retirer au compteur, et soupçon de renommage pour la grep).

### Les trois types de rappel (G)

Forme retenue (fragment de décision, pas un code à recopier) :

```c
/* pico8_ui_button.h */
typedef struct pico8_js pico8_js_t;
typedef void (*pico8_js_fn)(pico8_js_t *self);          /* js.my, js.hb */
struct pico8_js { pico8_ke_t ke; /* … */ pico8_js_fn my; int has_my; /* … */ pico8_js_fn hb; /* … */ };

typedef struct pico8_fq pico8_fq_t;
typedef int (*pico8_fq_my_fn)(pico8_fq_t *self);        /* vérité Lua du retour */
struct pico8_fq { pico8_ke_t ke; pico8_fixed_t fa; pico8_fixed_t hd; pico8_fq_my_fn my; int p_; int has_p_; int m_; int has_m_; };

/* pico8_msgbox.h */
typedef struct pico8_mh pico8_mh_t;
typedef int (*pico8_mh_lh_fn)(pico8_mh_t *self);        /* vérité Lua du retour */
```

- **Le receveur est typé** : chaque appelant passe `self` de son type (`js:ldown` le bouton, `fq:ldown` le fond, `mh:ev` la boîte), et le cart appelle chaque rappel comme `self:my()`/`self:lh()`/`self:hb()`. Rejeté : `void *self` (le type d'un rappel n'a qu'un receveur possible ; `void *` obligerait chaque rappel à reconvertir).
- **Étiquette de struct** (`struct pico8_js`, `struct pico8_fq`, `struct pico8_mh`) : le membre nomme le type de la struct dans sa propre définition ; le type ne change pas (précédent : `pico8_qy` à D.10a). Rejeté : un `typedef` de fonction à paramètre `void *` pour éviter l'étiquette.
- **Retours** : `js.my` et `js.hb` ne rendent rien (`js:ldown` et `hy:lj` jettent le retour ; les valeurs installées rendent toutes nil) ; `fq.my` et `mh.lh` rendent un `int`, la vérité Lua de ce que le cart rendait — c'est tout ce que leurs appelants lisent (`p_` n'est lu dans le jeu que par `dg`, en vérité ; `mh:ev` teste la vérité). Rejetés : `pico8_value_t` (garde `valeur` et le site G, et c'est la valeur étiquetée que la ligne retire) ; un tri-état `{has, value}` pour `fq.my` (un indicateur de présence seulement là où le code observe nil — §5, 1b — et aucun `fq.my` du jeu ne rend nil : `de.my` rend `true`).
- **`fq:ldown`** : `my` NULL → `p_` nil comme aujourd'hui (`p_ = 0`, `has_p_ = 0`) ; sinon `p_ = my(self)`, `has_p_ = 1`. Le `button_take_bool(my_result, …)` (site G, l. 603) part ; `button_take_bool` reste pour les écrivains de `m_`.
- **Noms de type** hors de tout motif compté (`pico8_js_fn`, `pico8_fq_my_fn`, `pico8_mh_lh_fn` ; précédents `pico8_state_fn`, `pico8_pi_fn`) : ce sont des types nouveaux, pas l'alias d'un identifiant vivant (`pico8_value_function_t` a une autre signature et disparaît).

### Rappel par rappel (G)

| Rappel | Signature | Valeurs installées après D.10b | Stockage | Chunk |
|---|---|---|---|---|
| `js.my` | `pico8_js_fn` | `pico8_ix1_my_dispatch`…`pico8_ix4_my_dispatch`, `pico8_ha2/3/4_my_dispatch`, `pico8_cr_row_my_dispatch`, `pico8_jo_fe_my_dispatch`, `pico8_init_dispatch`, `pico8_eu`, la variante `js` de `de.my` ; placeholders de `help`/`score_panel` | `pico8_js_t.my` + `has_my` (inchangé : présent et NULL = `false`), `pico8_js_keys_t.my`, `g_ix_my[]`, `my[]` local de `ix[4]`, `pico8_cr_row_t.row.my` | b1 |
| `js.hb` | `pico8_js_fn` | `pico8_ix2_hb_dispatch` | `pico8_js_t.hb`, `pico8_js_keys_t.hb`, `g_ix_hb[]` ; `pico8_js_hb_dispatch` appelle `js->hb(js)` ; `has_hb` gardé (H-D10a-7) | b1 |
| `fq.my` | `pico8_fq_my_fn` | `pico8_de_my()` (variante `fq` de `de.my`, rend 1) | `pico8_fq_t.my`, `pico8_fq_keys_t.my` | b1 |
| `mh.lh` | `pico8_mh_lh_fn` | `pico8_pj_hk_lh_dispatch`, `pico8_pj_ph_lh_dispatch` | `pico8_mh_t.lh`, `pico8_mh_keys_t.lh`, `pico8_pj_ea_t.lh` | b1 |
| `hp.lq` | — (donnée) | — | membre `lq` inchangé ; `pico8_hp_lq_dispatch` supprimé ; le lecteur `"lq"` ne répond que si le membre est posé | b1 |
| rangées de `menu` | `pico8_js_fn` | `ix[1..4].my`, `ix[2].hb`, `ha[1..4].my` | `g_ix_my[]`, `g_ix_hb[]` (`static const pico8_js_fn`), `my[]` local ; `pico8_ix_my(i)` rend `pico8_js_fn` | b1 |
| `jo:fe` | `pico8_js_fn` | `pico8_jo_fe_my_dispatch` | clé `my` des boutons de `jo:fe` | b1 |
| `de.my` | `pico8_fq_my_fn` **et** `pico8_js_fn` | voir « `de.my` » | `pico8_de_my()` rend `pico8_fq_my_fn` ; la variante `js` est statique à `score_panel_display.c` | b1 |
| `eu` (`help`) | `pico8_js_fn` | `pico8_eu` (installé par `pico8_cart_load`) ; placeholder `pico8_fv_eu_placeholder` retypé | `g_eu_dispatch` ; `pico8_eu_field_value()` rend `pico8_js_fn` ; `pico8_help_set_eu(pico8_js_fn)` | b1 |
| `init` (`score_panel`) | `pico8_js_fn` | `pico8_init_dispatch` retypé (`(void)self; pico8__init();`) ; placeholder `pico8_pj_init_placeholder` retypé | `g_init_dispatch` ; `pico8_init_field_value()` rend `pico8_js_fn` ; `pico8_score_panel_set_init(pico8_js_fn)` | b1 |

- **`self` NULL** : les rappels qui testaient `inst == NULL` (`cr`, `ix[2].hb`, `jo:fe`) gardent le test sur `self` ; les TU les appellent avec NULL (`test_pico8_ui` `call_ix_my`, `test_pico8_pj` `ix3_my`/`de_my`).
- **Lectures par nom dans les rappels** (`_cr_t`, `lq` écrit par `ix[2].hb`, `_jo_fe_*`) : **inchangées** (classe A, D.11a), même si `self` est désormais typé. Rejeté : les remplacer par `self->_cr_t` ici (site d'une autre étape ; D.10a a refusé de même le `t` de `dg`). La note le signale à D.11a : ces trois sites n'ont plus besoin de cast.
- **Accesseurs gardés sous leur nom** : `pico8_eu_field_value`, `pico8_init_field_value`, `pico8_ix_my`, `pico8_de_my` changent de type de retour, pas de nom (aucun ne porte de motif compté ; renommer serait du churn). Rejeté : `pico8_help_eu()`/`pico8_score_panel_init_fn()`.
- **`pico8__init` assert** (`cart.c:131-132`) : compare les pointeurs typés (`pico8_eu_field_value() == pico8_eu`, `pico8_init_field_value() == pico8_init_dispatch`), même message ramené au contrat.

### `de.my` (G)

La table `de` du cart sert deux fois : `js(ob(…, de))` dans `pj:ph` et `fq(de)` dans `ix[3].my`. Sa fonction est donc lue par deux appelants de types différents.

- **Retenu : deux fonctions, un corps.** Un corps statique de `score_panel_display.c` fait la bascule `ju.c_ = not ju.c_` ; la variante `fq` (`pico8_fq_my_fn`, que rend `pico8_de_my()`) l'appelle et rend 1 (`return true`) ; la variante `js` (`pico8_js_fn`, statique, installée par `pj:ph` dans `spec2.my`) l'appelle. `pico8_de_my()` garde son nom et son contrat (« la fonction que lit `fq(de)` de `ix[3]` »).
- **Rejetés** : un type commun à `js.my` et `fq.my` (`int (*)(void *)` : tous les rappels de bouton rendraient une vérité que personne ne lit, sur un receveur non typé) ; une seule fonction appelée par un pointeur converti (comportement indéfini, interdit).
- Conséquence : le bouton de `pj:ph` et le fond de `ix[3]` ne portent plus le même pointeur. Aucun code du jeu ne les compare ; aucun TU non plus (grep `pico8_de_my` à `d05e35e` : `test_pico8_fq`, `test_pico8_pj` seulement, sur le fond) — H5.

### `eu` et `init` (G)

- **`eu`** : `pico8_eu(pico8_js_t *nj)` a déjà la signature de `js.my` (le cart appelle `eu` comme `self:my()`, `self` étant le bouton cliqué, qui devient `dk`). `pico8_cart_load` installe `pico8_eu` ; `pico8_eu_dispatch` et sa déclaration partent (a). Rejeté : garder un adaptateur `pico8_js_fn` qui ne fait qu'appeler `pico8_eu` (fonction sans objet).
- **`init`** : `pico8__init(void)` ne prend pas le bouton ; `pico8_init_dispatch` reste l'adaptateur, retypé `void pico8_init_dispatch(pico8_js_t *self)` ; son commentaire de `cart.h` est ramené au contrat (le bouton est ignoré).
- Les ponts `pico8_help_set_eu`/`pico8_score_panel_set_init` et leurs placeholders restent : `help` et `score_panel` ne lient pas `cart.c` (listes courtes de TU). `pico8_help.h` inclut `pico8_ui_button.h` pour nommer `pico8_js_fn` (H1).

### Lecteurs par nom (G)

- `pico8_js_field` perd les branches `"my"` et `"hb"` ; `pico8_js_my()` (public, seul appelant cette branche) part, avec sa déclaration et son commentaire (a). Le nom `"my"` lit « absent » même pour un `my` présent à `false` : aucun lecteur de `src/` ne lit `"my"` par nom ; l'état `false` reste lisible sur le membre (`has_my && my == NULL`) et `js:ldown`/`js:kj` le lisent là.
- `pico8_fq_field` perd `"my"`, `pico8_mh_field` perd `"lh"`, `pico8_hp_field` rend « absent » pour `"lq"` quand le membre est NULL (la branche qui rend la rangée reste) ; le commentaire de `pico8_hp_field` (l. 80-83, « falls back to the class's `lq` method, as the cart's `mv(b,"lq")` ») est ramené au contrat qui reste (NULL : absent ; la méthode est `pico8_hp_lq`, appelée directement).
- Écart avec le cart : lu par nom, `b.lq` du cart rend la méthode ; le port rend « absent ». Même traitement que les méthodes à D.10a (`pico8_entity_get_field` ne rend plus de méthode) : ce lecteur est la couche que D.11e supprime, aucun code du jeu ne lit ces noms par nom — **pas de TROU**, déclaré au message.

### `pico8_value` sans genre fonction (G, b2)

- Retirés : `PICO8_VALUE_FUNCTION` (dernier de l'énumération : aucune autre valeur ne change, H9), le membre `function` de l'union, le typedef et son commentaire (l. 27-38, celui qui nomme encore `dc()`/`mv()`, H-D10a-5), le commentaire de la déclaration avancée (l. 22 ; la déclaration `typedef struct pico8_value pico8_value_t;` reste, elle ne sert plus qu'à l'ordre des définitions, ou se fond dans la définition — au choix le plus court, déclaré), `pico8_value_function()` (déclaration et définition), les bras `FUNCTION` de `pico8_value_equal` et `pico8_value_tostr`, et leurs mentions dans les deux commentaires (l. 71 ; « REF and FUNCTION render the empty string »).
- `sizeof(pico8_value_t)` : inchangé sur la cible (tous les membres de l'union font au plus 8 octets, H7) ; aucun TU ne plie `sizeof`.
- `docs/trous-de-portage.md` : TROU-13 **inchangé** — son corollaire rapporte ce qu'affirmait un ancien commentaire ; la branche `REF` qu'il concerne reste. Rejeté : réécrire l'historique d'un TROU.

### Pont des TU : aucun (G)

- **Retenu : les TU suivent dans le chunk qui change ce qu'ils nomment** ; aucun symbole ne reste en pont compté. `dispatch` = 0 à la fin de b2, `src/` et `tests/`.
- Raisons : (1) un pont `pico8_value_function_t` à côté des types de rappel demanderait une conversion entre les deux (une cale typée ↔ valeur, ou une conversion de pointeur de fonction), toutes deux interdites ; (2) les 9 lignes sont dans 4 TU que b1 doit toucher de toute façon (ils installent ou appellent des rappels retypés) ; (3) garder `PICO8_VALUE_FUNCTION` pour les TU seulement laisserait des bras de pli morts et des cas hors rangée sans producteur.
- **Conséquence : D.10c est vide.** Ses « 98 lignes `pico8_mv` » sont parties à D.10a, ses « 26 lignes `pico8_value_function` » à D.10a (17) et D.10b (9), il n'y a pas de pont, et `dispatch` entre dans `DATA_RESIDUE_GUARD` à b2. La note le dit (« Further Notes »).

### Un chunk ne touche un TU qu'une fois (G)

b1 touche les 8 TU qui installent, appellent ou lisent par nom un rappel (`js`, `fq`, `mh`, `pj`, `hp`, `ui`, `bg`, `fv`) et y retire **aussi** leurs mentions de `PICO8_VALUE_FUNCTION`/`.as.function` (bras de pli, cas hors rangée), mortes dès b1 ; b2 touche les 17 autres. Rejeté : repasser sur un TU de b1 en b2 (deux diffs pour une intention).

### Frontières et registres

- **Aucune conversion typée ↔ valeur ajoutée.** Les conversions qui restent préexistent et ne bougent pas : `keys->lq = pico8_value_ref(pico8_hp_lq(b))` de `jo:fe` (clé `pico8_value_t` de `js`, Q12), `pico8_rn(has_my ? pico8_value_fixed(…) : pico8_value_bool(0))` de `js:ldown`. Une conversion ajoutée est un échec.
- **Aucun registre nouveau.** Les trois tableaux de `menu.c` (`g_ix_my`, `g_ix_hb`, `my[]` de `ix[4]`) existent et changent de type d'élément seulement.

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours ; macro, `typedef`, constante, `enum`, pointeur ou fonction enveloppe qui renomme un identifiant vivant ; un alias ; un registre nouveau ; une conversion typée ↔ valeur ; une conversion de pointeur de fonction ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; supprimer un commentaire qui ne porte pas sur une construction retirée ; tout changement d'une constante de hachage hors de la sonde décrite ; un littéral nu à la place d'un identifiant compté dans un pli (arbitrage de D.7c) — l'octet gelé passe par une constante locale nommée (précédent D.8), (c) déclarée.
- **Classification exigée** ⟲, au message de chaque commit, pour chaque ligne `dispatch` et `par-nom` retirée : **(a)** construction supprimée (définition, déclaration, emballage `pico8_value_function(…)` d'un lecteur ou d'un accesseur, adaptateur, commentaire d'une construction retirée, cas de test retiré), **(b)** déclaration, signature ou variable dont le type passe de `pico8_value_function_t` à un type de rappel C, **(c)** même donnée, autre nom. Attendu, `dispatch` `src/` :
  - b1 (29 lignes) — **(a) 7** : `help.c:32`, `score_panel.c:161` (retour emballé de l'accesseur, réécrit sans emballage), `msgbox.c:133`, `research.c:202`, `ui_button.c:89` (`pico8_js_my`), `:159`, `:513` (branches de lecteur) ; **(b) 22** : `help.c:29`, `:35`, `help.h:55`, `menu.c:406`, `:412`, `:417`, `:470`, `menu.h:21`, `msgbox.h:26`, `:55`, `score_panel.c:158`, `:164`, `score_panel.h:59`, `:116`, `score_panel_display.c:373`, `score_panel_display.h:22`, `ui_button.h:33`, `:37`, `:78`, `:79`, `:128`, `:144` ;
  - b2 (6 lignes) — **(a) 6** : `value.h:19`, `:22`, `:39`, `:53`, `:61`, `value.c:36` ;
  - `dispatch` `tests/` (b1, 9 lignes) — **(a) 4** : `test_pico8_js:1440`, `test_pico8_fq:430`, `:542` (cas retiré), `test_pico8_pj:198` ; **(b) 5** : `test_pico8_js:179`, `:182`, `:768`, `test_pico8_fq:339`, `test_pico8_mh:372` ;
  - `par-nom` `src/` : **(a) 1** (`ui_button.c:603`, b1) ; `par-nom` `tests/` : lectures par nom d'un rappel remplacées par le membre, rapportées et classées de même ;
  - **(c) = 0** pour `dispatch` et `par-nom` ; les seules (c) admises sont, en `valeur`, les octets gelés nommés de la section « Constantes », listés.
  Recette : `git diff -U0` par fichier, lignes `-` qui portent le motif en regard des lignes `+`.
- **Mesure exigée**, avant et après chaque commit, par catégorie, **lignes** (`make data-residue`, `--detail` par fichier) **ET occurrences** (recette figée de D.8, `$S` : dossier du scratchpad) :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur type-tag ref-audit par-nom dispatch; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  ```

  Mesuré sur `d05e35e` : `valeur` 4639, `type-tag` 551, `ref-audit` 56, `par-nom` 1102, `dispatch` 46 (`src/` 37, `tests/` 9). Le détail par fichier (lignes et occurrences) de chaque fichier du diff est donné avant et après.
- **Contrôle** : pour chaque (catégorie, fichier) du diff, Δlignes et Δoccurrences ; si Δlignes > Δoccurrences en valeur absolue d'une baisse, le message justifie le couple. Attendus : `dispatch` `value.h` −5 lignes / −6 occ. et `value.c` −1 / −2 (b2 : deux lignes portaient deux occurrences, Δocc > Δlignes) ; ailleurs Δlignes = Δocc.
- **Lignes qui gagnent une catégorie**, listées par fichier et par catégorie, « aucune » compris. Attendues : **aucune** (`help.h` gagne une inclusion de `pico8_ui_button.h`, qui ne porte aucun motif ; un octet gelé nommé ne porte aucun motif).

### Empreinte remesurée et découpage (G) ⟲

| Chunk | `src/` | `tests/` | autres | `dispatch` retirées |
|---|---|---|---|---|
| b1 | 16 : `ui_button.c/.h`, `msgbox.c/.h`, `research.c`, `menu.c/.h`, `help.c/.h`, `score_panel.c/.h`, `score_panel_display.c/.h`, `cart.c/.h`, `planet_display.c` | 8 : `js`, `fq`, `mh`, `pj`, `hp`, `ui`, `bg`, `fv` | `docs/glossaire-noms.md` | `src/` 29, `tests/` 9 |
| b2 | 2 : `pico8_value.h/.c` | 17 : `entity_kind`, `fr`, `g_`, `ga`, `hj`, `iv`, `jc`, `jo`, `lq`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qb`, `qh` | `Makefile` (`DATA_RESIDUE_GUARD`), `docs/plan-3.md` (note) | `src/` 6 |

- b1 ≈ 25 chemins, b2 ≈ 21 : sous les ~40 de la règle ; **pourquoi deux chunks** : b1 change des types et un comportement observable par les TU (retour de `fq.my`, lecteurs par nom) ; b2 est une suppression mécanique, prouvée par la compilation (aucune valeur fonction n'existe plus après b1, H2). Un seul commit (≈ 45 chemins) mêlerait les deux preuves. Rejeté : un chunk par rappel (chaque commit toucherait `ui_button.h` et les mêmes TU, et un `pico8_value_function_t` vivant obligerait des conversions entre rappels déjà typés et rappels encore valeurs).
- **Pourquoi b1 avant b2** : b2 n'est possible que lorsque plus rien ne produit une valeur fonction.

### Effet sur `make data-residue` ⟲

| Catégorie (lignes / occ.) | Départ (`d05e35e`) | Après b1 | Après b2 | Pourquoi |
|---|---|---|---|---|
| `dispatch` `src/` | 35 / 37 | **6 / 8** | **0 / 0** | exact : 29 lignes des rappels, puis les 6 du type |
| `dispatch` `tests/` | 9 / 9 | **0 / 0** | 0 / 0 | exact : les 9 lignes de rappel (H-D10a-2) |
| `par-nom` `src/` | 419 | **418** | 418 | exact : le site G `ui_button.c:603` |
| `par-nom` `tests/` | 567 | baisse, rapportée | = | lectures par nom de `"my"`/`"hb"`/`"lh"`/`"lq"` remplacées par le membre |
| `valeur` | 4272 / 4639 | baisse, rapportée | baisse, rapportée | ≈ 22 signatures d'adaptateur (`src/` et `cart.h`) et 5 de TU, leurs `return pico8_value_ref(NULL)`, `pico8_js_my`, les accesseurs `eu`/`init`, `lh_result`/`my_result` ; en b2 le type ; en tout, les 66 lignes de TU qui nomment `PICO8_VALUE_FUNCTION` ou `.as.function` (b1 et b2) |
| `type-tag` | 347 / 551 | = (rapporté) | = | aucune ligne attendue |
| `ref-audit` | 55 / 56 | = | = | aucune ligne attendue |

« exact » est une égalité ; « rapporté » est mesuré au commit, écart expliqué.

### Documentation (G)

- `docs/glossaire-noms.md` (b1) : la ligne `my` (« callback de clic d'un bouton ») dit le type (`pico8_js_fn` sur `js`, `pico8_fq_my_fn` sur `fq`) ; les lignes `de` et `de.my` disent que la fonction de `de` existe en deux variantes typées, `pico8_de_my()` rendant celle de `fq(de)`.
- `docs/plan-3.md` (b2) : note « Découvert à D.10b » en fin de ligne D.10b (texte en « Further Notes »), rien d'autre.
- `docs/trous-de-portage.md` : inchangé (TROU-13, plus haut).
- Les PRD livrés, `docs/plan.md`, `docs/plan-2.md` : historiques, non touchés.

### Ce qui ne change pas

- `tools/` (motif figé `dispatch` : `\bpico8_value_function(_t)?\b|\bpico8_mv\(`), `tests/e2e/`, tout `*.sha256`, tout fichier d'événements ; aucun e2e ajouté ; le `Makefile` hors de la ligne `DATA_RESIDUE_GUARD`.
- Les clés `pico8_value_t` de `js` (`lq`, `h_`, `jb`) et les éléments de `lq.items` (Q12, D.12c) ; `has_my`/`has_hb` ; les lectures par nom dans les rappels (D.11a) ; `pico8_value_equal`/`pico8_value_tostr` pour les quatre genres restants.
- L'ordre des appels : chaque appelant lance le même rappel, au même moment, avec le même receveur.

### Q12

Q12 (valeurs à genres mêlés, dont « `js.my`/`h_` ») reste ouverte ; D.10b ne la tranche pas. Mais D.10b retire `pico8_js_my()`, un des 62 prototypes publics à `pico8_value_t` que Q12 recense (il rendait nil, `false` ou une fonction) : le genre mêlé de `js.my` ne vit plus que dans le membre (`has_my` + pointeur NULL), où il était déjà. La note le signale pour l'étude D.9b.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `pico8_help.h` peut inclure `pico8_ui_button.h` sans cycle (aucun en-tête n'inclut `pico8_help.h` à `d05e35e`) ; `pico8_score_panel.h` voit déjà `pico8_js_t` et `pico8_mh_t` par `pico8_msgbox.h`.
- **H2** : à la fin de b1, `pico8_value_function(` n'a plus d'appelant dans `src/` ni dans `tests/` (hors sa définition) : aucune valeur fonction n'existe à l'exécution, et les bras `FUNCTION` que b2 retire sont morts.
- **H3** : parmi les constantes, **seule `FQ_MEMBERS_HASH` bouge** (son scénario perd le pas « `my` rend nil », inexprimable) ; `MY_TRUE_SEQUENCE_HASH`, `MY_FALSE_SEQUENCE_HASH`, `BARE_SEQUENCE_HASH` (`fq`), `MEMBERS_BY_NAME_HASH` et les séquences de `mh`, `GB_HASH`, `RAW_PRESENCE_HASH`, `FUNCTIONS_HASH` (`hp`), `BUILT_HASH` (`pj`, `fv`), `HB_HASH` et les plis de `js` se reproduisent depuis les membres, mêmes octets.
- **H4** : les cas hors rangée retirés (valeur fonction offerte à un écrivain) s'exécutent dans un fils (`must_abort*`, `stops_on`) et ne sont pliés dans aucune constante.
- **H5** : `fn_index` de `test_pico8_js` ne voit pas à la fois la variante `js` et la variante `fq` de `de.my` ; les rangs de première vue ne changent pas.
- **H6** : la variante `js` de `de.my` est exécutée par un TU existant (clic sur le bouton de `pj:ph`) ; sinon une assertion est ajoutée (règle 9 des tests).
- **H7** : `sizeof(pico8_value_t)` ne change pas.
- **H8** : `pico8_msgbox_truthy` garde d'autres appelants (`msgbox.c:410, 420, 427, 461`) ; `lh_result` part, pas l'aide.
- **H9** : `PICO8_VALUE_FUNCTION` est le dernier genre : sa suppression ne renumérote rien, et les plis qui replient `(uint32_t)kind` d'autres genres ne changent pas.
- **H10** : aucun comportement du binaire de rejeu ne change : 26 e2e conformes.
- **H11** : `make data-guard` accepte `dispatch` dans `DATA_RESIDUE_GUARD` (`--check spec,overrides,stash,ob,dispatch`) et rend 0 ligne après b2.
- **H12** : aucun constructeur n'a à changer de texte (les affectations `self->my = keys->my`… gardent leur forme) ; si un chunk en touche un, `class-coverage-net` est lancé pour ce chunk.

## Implementation Chunks

Deux commits, dans cet ordre, chacun vert seul (`make test`, `make e2e`, `make data-guard`), chacun avec ses comptes, sa classification, ses lignes gagnées et, pour b1, ses mutations :

1. **b1 — rappels typés.** Types `pico8_js_fn`, `pico8_fq_my_fn`, `pico8_mh_lh_fn` et étiquettes de struct ; membres et clés retypés (`js.my`/`hb`, `fq.my`, `mh.lh`, `pj.ea.lh`) ; appelants (`js:ldown`, `pico8_js_hb_dispatch`, `fq:ldown` sans `button_take_bool`, `mh:ev`) ; valeurs installées retypées sous leur nom (`menu`, `jo:fe`, `pj`, placeholders) ; `de.my` en deux variantes ; `eu` installé par `pico8_eu`, `pico8_eu_dispatch` supprimé, `pico8_init_dispatch` retypé ; ponts `help`/`score_panel` retypés ; `pico8_hp_lq_dispatch` supprimé ; lecteurs `"my"`, `"hb"`, `"lh"`, `"lq"` (NULL) sans rappel, `pico8_js_my` supprimé ; les 8 TU (`js`, `fq`, `mh`, `pj`, `hp`, `ui`, `bg`, `fv`), y compris leurs mentions de `PICO8_VALUE_FUNCTION` ; glossaire ; sonde de `FQ_MEMBERS_HASH` ; `gcov` et mutations.
2. **b2 — suppression du genre fonction.** `pico8_value.h/.c` ; les 17 TU restants (bras de pli morts, cas hors rangée inexprimables) ; `DATA_RESIDUE_GUARD` gagne `dispatch` ; note « Découvert à D.10b » ; balayage ASan+UBSan de tous les TU.

Ordre conseillé dans chaque commit : mesure de départ (lignes et occurrences par fichier) ; en-têtes ; `.c` de `src/` ; TU ; `make test` ; `make e2e` ; (b1) sonde, `gcov` et mutations (copie jetable, restauration par `cp`) ; mesure d'arrivée, classification, lignes gagnées, greps de la « Sortie » ; docs ; message.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (valeurs, identités, registres, effets d'un appel), pas la présence d'une ligne. D.10b change le type des rappels, pas ce qu'ils font : **la preuve est que chaque TU garde son intention et sa constante (sauf la sonde déclarée), que les 26 e2e restent conformes**, et que chaque nouvelle ligne de `src/` est exécutée par un TU et mord sous mutation.

### TU touchés (G)

Règles, pour chaque TU qui installe, appelle ou lit par nom un rappel, ou qui nomme `PICO8_VALUE_FUNCTION`, `.as.function`, `pico8_value_function(_t)`, `pico8_eu_field_value`, `pico8_init_field_value`, `pico8_de_my`, `pico8_ix_my` :

1. **Rappels de TU** (`counting_my` de `js` et de `fq`, `scripted_my`, `counting_lh`, `pico8_pj_ea_lh_witness`) : au type du membre qu'ils remplissent, même effet (compteur, receveur vu, réponse). `counting_my` de `js` rendait le fixe 7, que `js:ldown` jetait : il ne rend plus rien. Les aides à quatre arguments (`call_my`, `call_ix_my`, `de_my`, `ix3_my`, appels directs `->lh(b, 0, nil, nil)`) appellent le rappel typé (`b->my(b)`, `pico8_ix_my(i)(NULL)`, `pico8_de_my()(NULL)`, `b->lh(b)`) ; une aide qui rendait le retour nil n'en rend plus.
2. **Lecture par nom d'un rappel** (`"my"`, `"hb"`, `"lh"`, `"lq"` NULL) : devient la lecture du membre (`js->my`, `js->has_my`, `js->hb`, `fq->my`, `mh->lh`, `hp->lq`) ; l'identité se compare sur le pointeur typé (`== pico8_eu_field_value()`, `== counting_lh`, `fn_index(js->my)`). **Au moins une assertion existante par classe touchée** (`js`, `fq`, `mh`, `hp`) lit désormais « `pico8_entity_get_field(e, "<rappel>")` rend 0 » là où elle lisait une fonction — même objet, valeur attendue changée par la ligne ; listée au message.
3. **Pas de registre de TU** : aucun tableau, `switch` ni boucle qui passe d'un nom de rappel à un membre ; un tableau de noms qui mêlait champs et rappels (`js_names`, `names[]` de `fq`, `MEMBER_NAMES` de `mh`, `hp_members`) garde ses noms de champ (lecture par nom, D.11d) et plie le rappel **à part**, depuis son membre, **à sa place dans l'ordre** pour garder les octets.
4. **Constantes** : aucune `*_HASH` ni `identity[]` ne change, sauf `FQ_MEMBERS_HASH` (règle 6). Un pli qui lisait un rappel par nom plie les mêmes octets depuis le membre : présent → ce que le pli écrivait pour une fonction (octet de format du TU, `0xFCu`, et `fn_index` ou l'identité comparée là où le pli les écrivait ; **genre** `4` là où le pli repliait `(uint32_t)v.kind`), absent → l'octet d'absence du TU (`0xDEADu`, `0u`). Le genre `4` est la valeur de `PICO8_VALUE_FUNCTION`, que b2 supprime : il passe par **une constante locale nommée par TU** (octet gelé d'une énumération supprimée, précédent D.8), (c) de `valeur` déclarée, jamais un littéral nu. Attendus : `test_pico8_fq`, `test_pico8_mh`, `test_pico8_hp`, `test_pico8_js`. Le `0xFCu` de format est un littéral du TU lui-même (pas un identifiant compté) : il reste littéral.
5. **Cas inexprimables, retirés et listés au message avec leur raison** :
   - valeur fonction offerte à un écrivain par nom qui doit s'arrêter : `test_pico8_fq` (`pico8_value_function(counting_my)`, `zero_of(PICO8_VALUE_FUNCTION)` sur `m_`), `test_pico8_bg` (`pico8_eu_field_value()` sur `m_`), `test_pico8_js` (`zero_union(PICO8_VALUE_FUNCTION)` sur `m_` et `lq`), et en b2 les `zero_union`/`zero_function`/`null_function` de genre fonction de `g_`, `ga`, `jc`, `mz`, `ui`, `fv`, `iv`, `entity_kind`, `nl`, `qb` — raison : le genre n'existe plus ; l'écrivain ne peut plus en recevoir ;
   - `fq.my` qui rend autre chose qu'un booléen (`ldown_returning` : fixe 1, fixe 0, chaîne vide, `"x"`, REF) — raison : `fq.my` rend un `int`, le type interdit ces retours ; l'`assert` de `button_take_bool` qu'ils prouvaient n'est plus sur ce chemin ;
   - le pas « `my` rend nil » du scénario de `test_fq_members_by_name` — raison : un `int` n'a pas de nil ; le pas « rend `zero_of(PICO8_VALUE_BOOL)` » devient « rend 0 » (même valeur, `false`).
   La lecture par nom `must_abort(bare, "my")` (écriture par nom refusée) reste : elle est exprimable.
6. **Sonde de `FQ_MEMBERS_HASH`** (la seule reprise attendue) : (i) dans une copie jetable de l'arbre parent de b1, retirer du scénario le seul pas « `my` rend nil » (même ordre pour le reste, `g_script_len` et l'`assert` du nombre d'appels ajustés), exprimé dans l'API de cet arbre, et noter la constante obtenue ; (ii) l'arbre de b1 doit donner exactement cette constante. Toute autre différence est un échec. Une autre constante qui ne se reproduit pas (H3 fausse) suit la même sonde, déclarée.
7. **§6** : aucun des trois tests n'est touché.
8. **Test ajouté** : aucun fichier nouveau. Une assertion nouvelle n'est admise que pour couvrir une ligne nouvelle de `src/` que `gcov` montre non exécutée (par exemple la variante `js` de `de.my`, H6), dans un TU existant de la classe, listée au message.
9. **Prior art** : les règles de TU de D.10a (plis « présence → octets de fonction » depuis le membre, octets gelés `VIEW_*` de `test_pico8_hp`, `TABLE_INDEX_KEY` de D.8), `fn_index` de `test_pico8_js` (identité par ordre de première vue), les cas hors rangée de D.7c.

### Preuve des tests (§7 : `gcov` et mutations)

`gcov` (TU des classes touchées, compilés en `--coverage` dans une copie jetable du scratchpad), b1 : `js:ldown`, `pico8_js_hb_dispatch`, `fq:ldown` (les deux branches de `my`), `mh:ev` (les deux vérités de `lh`), les deux variantes de `de.my` et leur corps, `pico8_init_dispatch`, les placeholders, les lecteurs `"my"`/`"hb"`/`"lh"`/`"lq"`. Une ligne non exécutée est couverte par une assertion (règle 8) ou justifiée au message.

Mutations de b1, **une à la fois, dans l'arbre de travail, fichier sauvegardé par `cp` avant et restauré par `cp` après** (jamais `git checkout`/`stash`), chacune rougissant au moins un TU (nommé au message) :

- **M1** `js:ldown` n'appelle plus `my` → `test_pico8_js` (`my_calls`) rouge ;
- **M2** `pico8_js_hb_dispatch` n'appelle plus `hb` → `test_pico8_js` (`HB_HASH`, `lq` de `ix[2]`) rouge ;
- **M3** `fq:ldown` laisse `has_p_ = 0` quand `my` est présent → `test_pico8_fq` (`MY_TRUE_SEQUENCE_HASH` : le fond n'est plus retiré par `dg`) rouge ;
- **M4** `fq:ldown` pose `p_ = 1` sans lire le retour → `test_pico8_fq` (`MY_FALSE_SEQUENCE_HASH`) rouge ;
- **M5** `mh:ev` tient `lh` pour toujours faux → `test_pico8_mh` (séquences de l'année) rouge ;
- **M6** la variante `fq` de `de.my` ne bascule plus `ju.c_` → `test_pico8_pj` rouge ;
- **M7** le lecteur de `hp` répond `"lq"` présent quand le membre est NULL → `test_pico8_hp` (assertion « rend 0 » de la règle 2) rouge ;
- **M8** `pico8_cart_load` installe `pico8_init_dispatch` comme `eu` → un TU qui passe par `pico8__init` (assert de `cart.c`) ou `test_pico8_fv` rouge ;
- **M9** `g_ix_hb[1]` = NULL → `test_pico8_js` (`has_hb`, `cv.hb`) rouge.
- Une mutation qui ne rougit rien est un trou : couvert par une assertion (règle 8) ou justifié au message.

b2 n'a pas de mutation : il ne retire que du code mort (H2) ; la preuve est la compilation, les constantes inchangées et le balayage. Déclaré au message.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit ; aucun e2e ajouté (les 26 passent par `js:ldown` des boutons de `ix`/`bo`, par `mh:ev` et par `fq(de)` ; un e2e nouveau n'ajouterait de couverture qu'à des lignes que les TU couvrent déjà).
- `make class-coverage-net` : seulement si un chunk touche un constructeur (H12) ; le vérificateur le constate sur le diff.
- `make sanitize-net` : non (aucune durée de vie ni propriétaire ne change).
- **Balayage ASan+UBSan des TU** à la fin de b2 : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), comme à D.7b et D.10a ; sortie propre.

### Vérification, budget et tableau « Runs »

**Budget : environ 90 minutes pour l'étape** (b1 ≈ 60, b2 ≈ 30), mesures et filets compris ; la relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make test` | de l'ordre de la minute (recompilation : `pico8_ui_button.h`, puis `pico8_value.h`, changent) |
| `make e2e` | ~1 s |
| `make data-residue`, recette, classification, greps | < 1 min |
| sonde de `FQ_MEMBERS_HASH` (un TU, deux arbres) | < 5 min |
| `gcov` des TU touchés (b1) | quelques minutes |
| mutations (9, chacune un `make` ciblé du TU) | < 10 min |
| balayage ASan+UBSan des TU | quelques minutes, une fois |

Au-delà du budget, l'implémenteur s'arrête et note ce qui n'a pas été lancé. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement : `#`, commande exacte, état de l'arbre (HEAD + chunk + étape), code de sortie, ligne clé, durée. Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que des chemins de `src/`, `tests/` et, à b1, `docs/glossaire-noms.md`, à b2, `docs/plan-3.md` et la seule ligne `DATA_RESIDUE_GUARD` du `Makefile` ; ni `tools/`, ni `tests/e2e/`, ni `*.sha256`.
2. **Greps** de la « Sortie » : exactement ce qui y est écrit (après b2) ; après b1, la grep 1 ne rend que `pico8_value.h/.c` et les TU de b2, la grep 2 est vide.
3. **Types** : aucune conversion de pointeur de fonction (greps 3) ; chaque valeur installée a exactement le type du membre qui la reçoit ; aucun `pico8_value_t` dans un type de rappel.
4. **Registres** : `js` sans `hb` hors de `cv.hb` ; ordre et contenu de `hv`/`cv`/`cg` inchangés (constantes des TU).
5. **Constantes** : aucune `*_HASH`/`identity[]` changée, hors `FQ_MEMBERS_HASH` avec sa sonde reproductible (et toute autre avec la même sonde, déclarée).
6. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (c) = 0 pour `dispatch`/`par-nom`, octets gelés nommés listés ; lignes gagnées listées ; aucune conversion typée ↔ valeur ajoutée ; aucun registre nouveau.
7. **Site G** : `ui_button.c:603` retiré ; les lectures par nom des rappels (`_cr_t`, `lq`, `_jo_fe_*`) intactes.
8. **Tests retirés** listés avec leur raison ; mutations, `gcov`, sonde, balayage ASan+UBSan rapportés.
9. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.10b ni au plan dans `src/` ou `tests/`.
10. **Docs** : note « Découvert à D.10b » en fin de ligne D.10b ; glossaire comme décrit ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le HEAD où D.10a est gradée (attendu `d05e35e`, ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas `dispatch` 44 (`src/` 35, `tests/` 9) et `par-nom` 986 sans explication (H-D10a-2, H-D10a-3).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change, ou si un test devrait changer d'intention pour passer.
- `FAILED:` si `make class-coverage-net` diffère de l'attendu versionné à un chunk qui touche un constructeur, ou si le balayage ASan+UBSan des TU n'est pas propre.
- `FAILED:` si un champ ou un site ne tombe sous aucune règle (arrêt « champ sans règle » du §7), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.

**Périmètre et types**

- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si un pointeur de fonction est converti vers un autre type, ou si une valeur installée n'a pas exactement le type de son membre.
- `FAILED:` si `pico8_value_function_t` (ou un type équivalent à arguments `pico8_value_t`) survit, en `src/` ou en TU, ou si un pont compté est ajouté.
- `FAILED:` si un `js` sans `hb` est inscrit dans `cv.hb`, ou si l'ordre ou le contenu de `hv`/`cv`/`cg` change.
- `FAILED:` si une lecture par nom d'un rappel (`_cr_t`, `lq` de `ix[2].hb`, `_jo_fe_*`) est convertie : c'est D.11a.
- `FAILED:` si un fichier de `tools/` change, ou si le `Makefile` change ailleurs qu'à la ligne `DATA_RESIDUE_GUARD`.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `dispatch` n'est pas dans `DATA_RESIDUE_GUARD` après b2.
- `FAILED:` si `dispatch` ne vaut pas `src/` 6 / `tests/` 0 après b1 et **0** après b2.
- `FAILED:` si `par-nom` `src/` ne vaut pas 418 après b1 et b2 sans écart expliqué.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, construction déplacée, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, alias, registre nouveau, conversion typée ↔ valeur ajoutée, littéral nu à la place d'un octet gelé, code déplacé hors du compteur, ou une (c) de `dispatch`/`par-nom`.
- `FAILED:` si le **message d'un commit** ne donne pas, par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences**, la classification (a)/(b)/(c) des lignes `dispatch` et `par-nom` retirées, les octets gelés nommés, la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris), la justification de chaque couple Δlignes > Δoccurrences ; un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change sans sa sonde déclarée et reproductible.
- `FAILED:` si un cas de test retiré n'est pas listé au commit avec la raison qui le rend inexprimable.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si le budget est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.10b », ou si elle omet les comptes réels, que D.10c est vide et ce qui change pour D.11a et Q12.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **D.10c** : plus rien à y faire (voir « Further Notes »).
- **Les lectures par nom** dans les rappels (`_cr_t`, `lq` de `ix[2].hb`, `_jo_fe_*`, classe A) et toutes les autres classes : D.11a-D.11e ; les lecteurs `field` eux-mêmes : D.11e.
- **Les clés `pico8_value_t` de `js`** (`lq`, `h_`, `jb`), `lq.items`, `mh.items` et les listes : D.12 ; les retours `pico8_value_t` de `pico8_jo_gg` (TROU-3) et `pico8_d_` : Q12, ouverte.
- **`valeur`, `type-tag`, `ref-audit`** au-delà de ce que D.10b retire en passant ; `pico8_value.c` lui-même reste (Sortie de D, Q12).
- `tools/`, le `Makefile` hors `DATA_RESIDUE_GUARD`, `tests/e2e/`, la phase 2, `make raylib`, l'appli raylib, l'audio (`rn`, `music` restent tels quels), le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **types** : `pico8_js_fn` (`void (*)(pico8_js_t *)`) pour `js.my`/`js.hb` et toutes les valeurs qu'on y installe ; `pico8_fq_my_fn` (`int (*)(pico8_fq_t *)`) ; `pico8_mh_lh_fn` (`int (*)(pico8_mh_t *)`) ; receveur typé, étiquettes de struct ; rejetés : `void *self`, un type commun, `pico8_value_t` en retour, un tri-état pour `fq.my`.
  - **`de.my`** : deux variantes typées sur un corps ; rejetés : type commun, conversion de pointeur.
  - **`hp.lq`** : rien à typer ; adaptateur et repli du lecteur retirés.
  - **`eu`** : `pico8_eu` installé tel quel, `pico8_eu_dispatch` supprimé ; **`init`** : adaptateur retypé.
  - **noms** : fonctions et accesseurs gardent leur nom ; rejeté : renommer.
  - **lecteurs** : ne répondent plus pour un rappel (absent), `pico8_js_my` supprimé ; pas de TROU (couche, aucun lecteur du jeu).
  - **pont des TU** : aucun ; D.10c vide ; `dispatch` dans `DATA_RESIDUE_GUARD` à b2.
  - **lectures par nom dans les rappels** : laissées à D.11a.
  - **constantes** : mêmes octets depuis le membre, genre gelé nommé ; sonde pour `FQ_MEMBERS_HASH` seulement.
  - **découpage** : b1 (types, ≈ 25 chemins), b2 (suppression, ≈ 21) ; un TU touché une fois.
  - **filets** : pas de `class-coverage-net` sauf constructeur touché, pas de `sanitize-net`, balayage ASan+UBSan des TU à la fin.
- **D.10c devient vide.** Recommandation au pilote (hors du pouvoir de ce PRD, qui ne change pas le plan) : l'étape suivante de `plan-pilot` après D.10b est D.10c ; son PRD n'a qu'à constater, par les greps 1 et 5 de la « Sortie » et `DATA_RESIDUE_GUARD`, que sa ligne est accomplie, sans commit de code, puis passer en `docs/prd/done/` pour que D.11a devienne l'étape suivante. La note « Découvert à D.10b » l'écrit dans le plan.
- **Note du plan (G)**, à ajouter en fin de ligne D.10b (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.10b : deux chunks (b1 : N1 chemins ; b2 : N2). Rappels typés à la signature de leur appelant : `pico8_js_fn` (`void (*)(pico8_js_t *)` : `js.my`, `js.hb`, rangées de `menu`, `jo:fe`, `eu` — installé par `pico8_eu` lui-même —, `init`), `pico8_fq_my_fn` et `pico8_mh_lh_fn` (`int`, vérité du retour) ; `de.my` en deux variantes (`pico8_de_my()` rend celle de `fq(de)`) ; `hp.lq` sans adaptateur ; les lecteurs par nom ne rendent plus de rappel (`pico8_js_my` supprimé) ; `PICO8_VALUE_FUNCTION`, `pico8_value_function()`, `pico8_value_function_t` supprimés. Pas de pont : les 9 lignes de rappel des TU ont suivi, **D.10c n'a plus d'objet** (ses lignes `pico8_mv` sont parties à D.10a, ses lignes `pico8_value_function` à D.10a et D.10b) ; `dispatch` = 0, dans `DATA_RESIDUE_GUARD`. D.11a : les lectures par nom de `_cr_t`, de `lq` (`ix[2].hb`) et de `_jo_fe_*` se font sur un `self` déjà typé `pico8_js_t *`. Q12 : `pico8_js_my()` (nil, `false` ou fonction) n'existe plus ; le genre mêlé de `js.my` ne vit que dans le membre (`has_my` + NULL). `FQ_MEMBERS_HASH` reprise par sonde (pas « `my` rend nil » retiré). `par-nom` P (`src/` 418) ; `valeur` V ; résidu D R.

  N1, N2, P, V, R sont les valeurs réelles.
- **Le message de chaque commit** porte : le périmètre du chunk ; la classification (a)/(b)/(c) des lignes `dispatch` et `par-nom` retirées ; les comptes avant et après **en lignes et en occurrences** par catégorie et par fichier, avec les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie, par fichier (« aucune » compris) ; les octets gelés nommés ; les cas de test retirés et leur raison, les assertions ajoutées et la ligne qu'elles couvrent ; les constantes (inchangées, ou la sonde) ; les mutations ; `gcov` ; les filets ; le tableau « Runs » résumé ; les réponses aux hypothèses H et `H-D10a` qui le concernent ; ce que le jeu ne lit plus (`b.lq`, `js.my`, `fq.my`, `mh.lh` par nom).
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget) : types, anti-tassement, classification, constantes, cas retirés.
- **Surprises pour le pilote** :
  - **`de.my` a deux appelants** (`js.my` de `pj:ph`, `fq.my` de `ix[3]`) : « l'unique appelant » ne tient pas pour lui ; deux variantes typées.
  - **`hp.lq` n'est pas un rappel** : c'est une donnée dont la méthode de repli est appelée directement depuis D.10a ; D.10b n'a qu'un adaptateur à retirer.
  - **`eu` n'a plus besoin d'adaptateur** : `pico8_eu(pico8_js_t *)` a déjà le type de `js.my`.
  - **Le « (18, 0) » de l'empreinte ne voit que `src/`** : retirer `PICO8_VALUE_FUNCTION` touche 25 TU (comptés par `valeur`), d'où les deux chunks.
  - **`fq.my` typé en `int` rend inexprimables** le retour nil et les retours d'un autre genre qu'un booléen que `test_pico8_fq` scénarisait : `FQ_MEMBERS_HASH` est reprise par sonde, les cas d'`assert` retirés.
  - **Des TU plient le genre `4` d'un rappel lu par nom** (`fq`, `mh`, `hp`, `js`) : octet gelé nommé, (c) de `valeur` déclarée (précédent D.8).
  - **D.10c est vide** à la fin de D.10b.
  - **Q12** perd un de ses 62 prototypes (`pico8_js_my`).
