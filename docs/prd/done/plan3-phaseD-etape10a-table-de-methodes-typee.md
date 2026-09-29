---
title: "Plan 3, étape D.10a : `methods[]` devient une table de classe typée (14 emplacements nommés, table d'états de `dg`, 4 méthodes appelées directement) ; `pico8_mv` supprimé, les 26 appels de `src/` et les 98 lignes de TU remplacés dans le même commit, sans pont ; deux chunks a1/a2"
description: "Le dispatch passe encore par `pico8_value_function_t methods[23]`, 63 adaptateurs `(void *self, int argc, pico8_value_t, pico8_value_t)` et `pico8_mv`, qui résout une méthode par son nom et la rend aussi comme valeur de champ. D.10a donne à chaque méthode sa signature C dans `pico8_struct_class_t`, met les cinq états lancés par `dg` dans une table typée indexée par un enum, appelle directement les méthodes à implémentation unique, retire `pico8_mv` et la méthode-valeur de `pico8_entity_get_field`, sans changer un pixel ni une constante de hachage ; les rappels (`my`, `hb`, `lh`, `eu`, `init`…) restent des `pico8_value_function_t` jusqu'à D.10b."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données ») :

> - D.10a — `dispatch`, table de méthodes : `methods[]` → struct de pointeurs typés, 23 emplacements nommés (Q2), et table d'états typée de `dg` (`ph`, `ly`, `ll`, `kz`, `pr` ; Q3) ; les 82 adaptateurs `(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1)` retirés ; les 26 appels `pico8_mv` de `src/` remplacés (appel direct de la fonction typée pour `eb`, `ib`, `gg`, `md`, `lq`, `lh` ; virtuel pour le reste) ; `dg`, `dc`, `ui:jz`, `lt`, `he` et les prédicats `*_is_*_class` sur les emplacements ; `pico8_entity_get_field` ne rend plus de méthode ; `hd=1` en fonction constante vraie ; `pico8_dispatch_context` gardé (Q4) ; sites `par-nom` G (9) et E de `mv` (1). Deux chunks : a1 (méthodes lancées par `dc`, `lt`, `he`, `ui:jz`, `dg`), a2 (le reste). Empreinte 37 (37, 0) aujourd'hui. Compte : `dispatch` `src/` ≤ 35 (67 moins les 29 lignes `pico8_mv`, `entity.c:177`, `methods[]` de `pico8_struct.h` et le commentaire de `pico8_dispatch_context.h`), ≤ 37 si `pico8_mv` reste en pont pour les TU ; `par-nom` ≈ 999 ; `valeur` rapporté. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non.

Lignes suivantes du même groupe, recopiées (D.10a en change le contenu, voir « Lecture ») :

> - D.10b — `dispatch`, rappels : `js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, rappels des rangées de `menu` (`g_ix_my`, `g_ix_hb`, `ha`), `jo:fe`, `de.my` (`pico8_de_my`), `eu` de `help`, `init` de `score_panel` → pointeurs de fonction à la signature de leur unique appelant ; leurs lecteurs (`msgbox.c`, `research.c`, `ui_button.c`) ne les rendent plus comme valeur ; `PICO8_VALUE_FUNCTION`, `pico8_value_function()` et `pico8_value_function_t` retirés de `pico8_value.h/.c`. Empreinte 18 (18, 0) aujourd'hui. Compte : `dispatch` `src/` = 0, pont pour les TU excepté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non.
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

Arrêt de la liste et ligne D.9, recopiés (ils disent d'où viennent D.10 à D.12) :

> **Arrêt « champ sans règle » (§7), constaté à l'écriture de la liste : elle s'arrête à D.8.** Aucune puce de D ne dit ce que deviennent `par-nom` (1081/80) et `dispatch` (197/59). La première et la troisième puce ne couvrent que des tables : `ob` et `stash`, plus, par lecture, `spec` et `overrides`. La **Sortie** nomme des fichiers à supprimer, pas la forme de leurs usagers, et D.0 ne fait que compter. `valeur` (5989/132), `type-tag` (445/71) et `ref-audit` (70/17) ne peuvent tomber qu'avec eux. Les décisions de D.9 (ci-dessous) lèvent cet arrêt pour `par-nom`, `dispatch` et `pico8_list_t` (D.10 à D.12) ; la Sortie de D exige encore le travail sur `valeur`, `type-tag` et `ref-audit`, suspendu à Q12.
> 1. `par-nom` : `pico8_entity_get_field`/`set_field` (132 lignes de `src/`), `*field_truthy(`, les `field`/`set_field` des descripteurs (`pico8_struct_field_fn`, `pico8_struct_set_field_fn`), le `field` des sortes et 191 lignes d'aides `*_take_*`. La puce **Coexistence** de 1b les fait survivre à 1b, et aucune puce ne dit en quoi ils se changent. Les aides à nom non littéral (`field_fixed(inst, name)` et semblables, dans `economy_score.c`, `particle.c`, `probe.c`, `score_panel.c` et `entity_geom.c`) relèvent en plus de la clé dynamique du §5.
> 2. `dispatch` : `pico8_value_function_t methods[PICO8_METHOD_COUNT]` (`pico8_struct.h`), `pico8_mv()`, et les membres fonction (`js.my`/`hb`, `mh.lh`…). Typer les arguments demande une signature par méthode, qu'un tableau indexé par `pico8_method_t` ne tient pas. Or la note de D.0 dit que la table survit à D.
> 3. `pico8_list_t` : la note de D.0 exige d'en changer la représentation sans en fixer la forme. `lq.items` tient des chaînes, des fonctions, nil et le fixe 0 (`layout_nil_item_draws_nothing`), et aucune puce ne type ces éléments hétérogènes.
>
> - D.9 — **étude, sans conversion** (demandée par l'humain à l'arrêt) : pour chacun des trois points ci-dessus, recenser les sites par grep (fichier, fonction, noms lus ou écrits, genres des valeurs), les classer par forme de remplacement possible — accès au membre typé (`p->hp`) à la place de la lecture par nom ; fonction C par méthode, à arguments typés, à la place de `pico8_mv` et de la table `methods[]` ; tableau typé, union étiquetée locale ou struct à la place d'une `pico8_list_t` d'éléments `pico8_value_t` —, et pour chaque classe de sites : la forme retenue, le nombre de sites, ce qui n'entre dans aucune forme (clé dynamique du §5 incluse). Sortie : une section de ce plan qui propose les puces à ajouter à D (ou à reporter en phase 2), avec leur liste d'étapes et leurs comptes ; l'humain tranche avant la première étape qui les applique. D.2 à D.8 ne dépendent pas de cette étude.

Notes « Découvert à D.x » versionnées à `1c9aae0`, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b, D.5c, D.6a, D.6b, D.6c, D.7a, D.7b, D.7c, D.8 ; D.1 et D.9 n'en ont pas) :

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

§5, règles des phases, recopiées (intro et puces de 1a, de 1b, phases 2 et 3 ; les listes d'étapes de 1a et 1b et la liste de D sont citées ci-dessus par leurs lignes utiles, pas recopiées) :

> ### Phase 1a — Entités en structs, champs `pico8_value_t`
>
> Transformation mécanique. Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`, comme aujourd'hui.
>
> - **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
> - **Clés construites à l'exécution** : `self[bt.."held"]` (`pico8_ui.c:328`) → deux champs `lheld`/`rheld` ; `ldown`/`rdown` côté méthodes. Tout autre site de clé dynamique trouvé est énuméré de même, ou déclenche un arrêt (§7) si l'ensemble des clés n'est pas fini.
> - **Défauts mutés sur la classe** : `ie.gy`, `ie.id` (`pico8_research_icon.c:158,182`, `pico8_cart.c:158`) deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** (connus : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`) ; la liste des étapes les nomme.
> - **Instances sans classe** : converties comme des classes à part entière, sans parent.
> - **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
> - **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag (`jo` pour `cg.jo`) ; seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille. En 1a, un membre nil de trop est sans effet (absent à la lecture) ; la phase 2 reclasse.
> - **Ordre** : une classe par commit, la parente avant ses filles (`ke` → `jo` → `qh` → …). Les racines sans parent dans n'importe quel ordre.
> - **Premier commit** : une vraie racine sans parent ni fille (`jb`), qui prouve la coexistence. **Deuxième** : `ke` **dans le même commit** que sa fille directe sans descendance de plus petite emprise (mesurée par le PRD de l'étape), pour que la struct de `ke` soit construite et vue par le filet dès son introduction. Ensuite les autres filles directes sans descendance de `ke`, par emprise croissante ; l'arbre `jo` et `g_`→`fv` après elles ; `ui` et `pj` (clés dynamiques, défauts mutés) en fin de liste.
> - **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a, qui les supprime toutes avec `pico8_class.c`. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe. Les descendants non convertis trouvent leurs champs hérités en remontant `parent`, qui reste valide.
> - **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre (dispatcher hérité dans `defaults`, ou appel direct `jo.kj(self,…)`, `jo.kf(self,…)`) reste à double forme : lecture/écriture par `pico8_entity_get_field`/`set_field`, ou aiguillage sur le tag comme `pico8_ke_ks`. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante (pour `jo` : 1a.26). La caractérisation de 1a.17 lit au moins une instance de chaque fille non convertie construite par le filet (`qh`, et `pm`/`ga` via `kv[hp.cw]`) : champ hérité de `jo` et résultat d'une méthode de `jo` appelée sur elle.
> - **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` (grep `->defaults`, ~100 sites dans 22 fichiers) migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct : une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec. Le PRD de `jo` tranche `kw` (écrit en place par `mx:kf`, `planet_family_trade.c:280`, si l'instance n'a pas couru `lz`) : défaut modifié en place → état statique partagé, pointé aussi par `defaults`.
> - **Sortie** : plus aucune `pico8_class_t`/`pico8_instance_t` à l'exécution ; `pico8_class.c` supprimé ; `make class-coverage-net` toujours sans zéro — vérifié par `grep -rn 'pico8_class_t\|pico8_instance_t\|->defaults\|lookup_named' src/` vide.

> ### Phase 1b — Types C réels
>
> Struct par struct, chaque membre `pico8_value_t` prend son type C (`pico8_fixed_t`, `pico8_vec_t`, pointeur de struct typé, `const char *`, booléen…).
>
> - **Nil** : un indicateur de présence **uniquement** là où le code observe nil (test de vérité, `mv` qui rend 0 sur champ absent, affectation `= nil`). Rappel : en Lua `0` est vrai, donc un fixe à 0 n'est pas nil. Pas de valeur sentinelle.
> - Les REF deviennent des pointeurs typés : les sites de `pico8_ref_audit.h` disparaissent au fil de l'eau.
> - **Recensement (1b.0)** : outil seul, aucune struct typée, sur le modèle de l'étape 0.2 et du build `-DPICO8_CLASS_COVERAGE`. Un build à drapeau recense, par (struct, membre), l'ensemble des genres de `pico8_value_t` écrits et si nil est observé par un lecteur (test de vérité, `mv` qui rend 0, `= nil`). Crochets : init des structs, écrivains `set_field`, lecteurs `*_truthy` et par nom, les ~24 sites à table d'offsets `*(pico8_value_t *)((char *)self + offset)`. Passé sur les rejeux de `FRAMES_EVENTS` et `make test`. Sortie : une table versionnée, et la durée du rejeu ASan+UBSan (§7). Un membre sans genre observé est signalé : son PRD ajoute une caractérisation dirigée avant de le typer.
> - **Coexistence** (fixée à l'écriture de la liste, agent `guideline`) : le `field`/`set_field` du descripteur est le seul point de conversion ; les sites par nom (`pico8_entity_get_field`/`set_field`, `*_truthy`, `src/` et `tests/`) ne changent pas.
>   - Lecture : le membre typé est rendu emballé, du genre qu'il aurait eu en 1a, un pointeur (REF, chaîne, fonction) repassé tel quel ; absent exactement là où 1a rendait 0 — pointeur NULL, ou indicateur de présence faux.
>   - Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` (arrêt « champ sans règle », §7).
>   - Tables d'offsets (`members[]`, `defaults[]`, `pico8_X_member`) : elles ne listent que des `pico8_value_t`. Le commit qui type un membre l'en retire partout, tables des filles comprises (`offsetof(fille, parent.m)`), et l'initialise par affectation typée ; un défaut REF garde la même REF.
>   - Présence : un membre non pointeur lisible par nom (`l`) qui peut valoir nil (nil observé, ou tenu) garde un indicateur, sauf preuve du PRD qu'aucune lecture par nom ne le voit nil.
>   - Garantit : présent/absent, valeur et identité de REF lus par nom identiques à 1a ; aucune écriture `*(pico8_value_t *)` sur un membre typé. Vérifié à chaque étape par `grep -n 'offsetof(pico8_T_t, \([a-z0-9_]*\.\)*m)' src/` vide (m typé, T la struct et ses filles) et la mutation (2) du §7.
>   - Un site par nom remplacé par un accès direct est noté dans « Découvert à 1b.N » ; les autres survivent à 1b.
> - **Pilote (1b.1)** : la plus petite struct dont la ligne de recensement mêle au moins un fixe simple, un membre à nil observé et un membre REF. Son « Découvert à 1b.1 » fixe le patron (disposition du membre typé, indicateur de présence, écriture à l'init, emballage du lecteur) ; les PRD suivants le citent, et s'en écarter est une « lecture nouvelle d'une règle » (§7).
> - **Liste** : chaque ligne porte son empreinte mesurée (fichiers de `src/` et `tests/` qui lisent ou écrivent ses membres) ; une ligne de plus de ~40 fichiers est découpée dans la liste elle-même, par groupe de membres (1b.Na, 1b.Nb…).
> - **Sortie** : aucune struct d'entité ne contient de `pico8_value_t` — constaté par une commande nommée : un script qui parcourt les typedefs de struct d'entité (premier membre `pico8_struct_header_t` ou struct d'entité enchâssée) et liste tout membre `pico8_value_t` ; sortie vide. Le même script sert de compteur de progrès à chaque étape.

> ### Phase 2 — Composition
>
> La hiérarchie par enchâssement de 1a devient :
>
> - un **noyau** `entity` commun (ce que `ke` apporte : table de méthodes, `me`, `t`, tags `nu` en masque de bits) ;
> - des **archétypes** `static const` : les défauts de chaque classe, résolus une fois le long de sa chaîne (`NL_DEF = { .hp=6, .s=68, … }`) ;
> - des **composants** optionnels portés par pointeur.
>
> Règles, appliquées sans supervision :
>
> 1. **Archétype ou état** : un champ va dans l'archétype si aucun code ne l'écrit sur une instance (prouvé par grep) ; sinon dans l'état mutable de la struct. L'état partagé de classe (`ie.gy`, `ie.id`) reste statique.
> 2. **Composant** : un groupe de champs devient un composant s'il apparaît dans **deux branches distinctes** de l'arbre (exemple connu : `py`/`fc`/`k_`/`qo`/`ro`/`qg`/`q_`/`du` dans `hj`, `pm`, `mx` et `ga`, alors que `ga` descend de `jo`). Sinon il reste dans la struct du type.
> 3. Test ponctuel, supprimé en fin de phase : pour chaque ancienne classe, chaque champ résolu par sa chaîne est égal à la valeur de l'archétype correspondant (piège connu : le `nu=o()` vide de `ga` masque celui de `jo`).
>
> - **Sortie** : aucune struct d'entité n'enchâsse plus la struct de sa parente (tout type = noyau + archétype + composants) ; chaque ancienne classe a son archétype `static const` ; le test ponctuel est passé pour chacune avant d'être supprimé.
>
> ### Phase 3 — Renommage
>
> Un champ est renommé **seulement** si `docs/glossaire-noms.md` en donne le sens. Sinon, il garde son nom minifié. Aucun sens n'est inventé. Un commit par groupe de champs renommés ; le glossaire est mis à jour en conséquence (l'entrée pointe du nom minifié vers le nouveau nom).

§6, recopié :

>
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

### Lecture de ces textes pour D.10a

- **Périmètre** : la ligne nomme la table de méthodes (Q2), la table d'états de `dg` (Q3), les adaptateurs, les 26 appels `pico8_mv` de `src/`, les lanceurs (`dg`, `dc`, `ui:jz`, `lt`, `he`), les prédicats `*_is_*_class`, la méthode rendue comme champ par `pico8_entity_get_field`, `hd=1`, `pico8_dispatch_context` (gardé, Q4), et les sites `par-nom` G (9) et E de `mv` (1). Elle ne nomme pas les rappels : `js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, `menu`, `jo:fe`, `de.my`, `eu`, `init` sont la ligne D.10b, qui retire `pico8_value_function_t`.
- **« 23 emplacements nommés »** : l'enum `pico8_method_t` a 23 entrées. L'étude (§2.2) en fait trois sorts : 14 méthodes virtuelles, 5 états lancés par `dg` (Q3 : « table d'états typée indexée par un enum »), 4 méthodes à implémentation unique appelées directement (« plus de table »). Un membre nommé par état **et** une table d'états doubleraient la donnée ; un membre pour `eb`/`ib`/`gg`/`md` ne serait lu par aucun code du jeu. Lecture retenue (G) : **les 23 méthodes ont chacune une place typée** — 14 membres nommés, 5 cases de la table d'états, 4 fonctions appelées directement —, aucune entrée sans lecteur. Rejeté : 23 membres nommés plus la table (données mortes et doublées).
- **« 82 adaptateurs »** : c'est le compte de la grep de l'étude (`^(static )?pico8_value_t X(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1)` dans `src/*.c`). À `1c9aae0` elle rend **63 adaptateurs de méthode** (pointés par `methods[]`) et **19 lignes de rappel** (`menu.c` 10 dont une déclaration avancée, `cart.c` 2, `help.c` 1, `score_panel.c` 1, `score_panel_display.c` 3, `research.c` 1 — `pico8_hp_lq_dispatch` —, `planet_display.c` 1 — `pico8_jo_fe_my_dispatch`). Les 19 rappels ont le type `pico8_value_function_t` que D.10b retire : ils sont à D.10b. Deux implémentations de `mm` (`pico8_g__mm`, `pico8_ga_mm`) ont la forme d'adaptateur sans que la grep les voie (paramètre `self_ptr`). D.10a traite donc **65 implémentations de méthode** : 61 deviennent des fonctions à la signature de leur emplacement, 4 disparaissent (appel direct de `pico8_jc_eb`, `pico8_jo_ib`, `pico8_jo_gg`, `pico8_qb_md`).
- **Direct / virtuel** : « appel direct de la fonction typée pour `eb`, `ib`, `gg`, `md`, `lq`, `lh` ; virtuel pour le reste ». `lq` (`hp`) et `lh` (`mh`) ne sont pas des méthodes de la table : ce sont des membres (`hp.lq` donnée ou repli sur `pico8_hp_lq`, `mh.lh` rappel). Deux appels `pico8_mv` de `src/` visent un troisième membre, `my` (`js:ldown`, `fq:ldown`), que la ligne ne cite pas : il suit la même règle que `lh` (appel direct du membre, dont le type reste `pico8_value_function_t` jusqu'à D.10b). « Virtuel » : `hd`, `kj`, `ev`, `hk`, `hb`, `ldown`, `rdown`, `gv`, `hq`, `ey`, `kf`, `cn`, `ol`, `mm`, et les états.
- **Pont des TU** : le paragraphe des comptes laisse le choix (« le symbole reste comme pont compté, fermé par l'étape TU de son groupe (D.10c) ; le PRD tranche »). Tranché plus bas : **aucun pont** ; les TU suivent dans le chunk qui retire ce qu'ils nomment. Conséquence écrite dans la note du plan : D.10c ne garde que les lignes de rappel des TU.
- **Compte** : « `dispatch` `src/` ≤ 35 […], ≤ 37 si `pico8_mv` reste en pont ». Sans pont : **= 35** lignes (37 occurrences), exact. `par-nom` « ≈ 999 » ; `valeur` « rapporté ».
- **« §6 : aucun »** : aucun des trois tests du §6 (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) ne nomme `pico8_mv`, `methods[]`, un adaptateur ni une méthode par son nom (grep à `1c9aae0`) : aucun n'est touché.
- **« `class-coverage-net` : oui. `sanitize-net` : non »** : chaque descripteur de classe change (constructeurs compris, qui posent `header.cls`) ; aucune durée de vie ni aucun propriétaire ne change. Le balayage ASan+UBSan des TU (consigne du pilote) est fait dans une copie jetable, à la fin.
- **Leçons des notes à ne pas refaire** : classification (a)/(b)/(c) absente ou fausse, (c) non déclarée, lignes qui gagnent une catégorie omises (correctifs de D.7a) ; conversion typée → valeur ajoutée le temps d'un commit (D.7a, point 3) ; constante de hachage « mise à jour » (interdit par toute la phase) ; TU corrompu qui passait en relisant sa propre corruption (D.7b : balayage ASan des TU).
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucune jonction de lignes, aucun alias, aucun registre, aucun renommage pour échapper aux greps, aucune conversion typée ↔ valeur gardée comme cale ; aucune conversion de pointeur de fonction (Q2 : un appel par un pointeur casté est un comportement indéfini) ; mutations restaurées par `cp` ; balayage ASan+UBSan des TU à la fin ; jamais l'appli raylib.

### État de départ

**Mesuré sur `1c9aae0`** (branche `claude/plan-pilot-3-7zdft0`, D.8 gradée, décisions de D.9 écrites dans le plan). Seuls `make -s data-residue`, `sh tools/data_residue.sh --detail`, la recette d'occurrences de D.8 et des `grep` ont tourné. **Rien n'a été compilé ni lancé.**

| Catégorie | Lignes | Fichiers | Occurrences | `src/` (lignes / fichiers) | `tests/` (lignes / fichiers) |
|---|---|---|---|---|---|
| `valeur` | 4961 | 126 | 5742 | — | — |
| `type-tag` | 352 | 65 | 560 | — | — |
| `ref-audit` | 56 | 15 | 57 | — | — |
| `par-nom` | 1009 | 81 | 1064 | 433 / 43 | 576 / 38 |
| `dispatch` | 191 | 61 | 193 | 67 / 30 (69 occ.) | 124 / 31 |
| `ob`, `stash`, `overrides`, `spec` | 0 | 0 | 0 | — | — |
| **résidu D** | **5868** | **128** | — | — | — |

`DATA_RESIDUE_GUARD := spec,overrides,stash,ob`. `make e2e` : 26 e2e dans `tests/e2e/`. 78 TU dans `tests/`.

### Inventaire (mesuré sur `1c9aae0`)

**Table et enum.** `pico8_struct.h` : `pico8_method_t` (23 entrées, `OL` … `MM`), `pico8_method_of(name)` (strcmp sur 23 noms), `pico8_struct_class_t { pico8_value_function_t methods[PICO8_METHOD_COUNT]; field; set_field; }` (l. 89 comptée `dispatch`). **40 descripteurs** dans 21 fichiers de `src/` (`bg` 1, `economy` 1, `entity` 1, `help` 1, `layout` 2, `menu` 1, `mouse_reader` 1, `msgbox` 2, `planet` 1, `planet_family` 6, `planet_family_trade` 4, `probe` 3, `research` 1, `research_icon` 2, `score_panel` 2, `scroll` 1, `slipway` 3, `trade` 1, `ui` 2, `ui_button` 2, `wormhole` 2), plus 4 descripteurs de TU (`test_pico8_iv` `g_fake_toast_class`, `test_pico8_js` l. 1828, `test_pico8_mo` `g_ra_only_class`, `test_pico8_ui` `g_giver_class`) et les copies de `test_pico8_planet_kj_dispatch` (`g_spied_qb`, `g_spied_jo`).

**Méthodes : implémentations distinctes, appelants, arguments et retours observés** (grep des initialiseurs `[PICO8_METHOD_X] = f`, corps des adaptateurs, sites `pico8_mv`) :

| Méthode | Implémentations (adaptateur → fonction typée) | Lanceur(s) dans `src/` | Arguments observés | Retour lu |
|---|---|---|---|---|
| `kj` | 12 : `bg` (ignore `ra`), `ei` (`particle`, sur `g_`, ignore), `fv`, `ga`, `hj` (`hj`/`pm`/`mx`), `ie`, `iv`, `jc` (ignore), `jo` (`jo`/`qh`/`nl`/`os`/`nr`/`fr`), `js`, `mz`, `pj` (ignore) | `dc("kj")` | `qc.ra` : REF de vecteur ou nil (→ `(0,0)`) | ignoré |
| `ev` | 5 : `dl` (`probe`, sur l'arbre `qh`, `g_`, `fv`, `mz`), `jc`, `js`, `mh`, `ui` — aucune ne lit `ra` | `dc("ev")` | `qc.ra` | ignoré |
| `hd` | 4 : `fv`, `ie`, `jo` (les 10 classes de l'arbre), `js` → toutes `pico8_gk` ; plus `hd=1` membre fixe de `bg` et `fq` (lu comme valeur par `mv`) | `ui:gw` | `qa` (`pico8_vec_t *`), `je.ra` (REF de vecteur ou nil) | vérité |
| `hk` | 2 : `jo` (ignore `yr`), `pj` (ignore `self`) | `hy:hk` (`cv.hk`) | `yr` fixe | ignoré |
| `hb` | 4 : `ie`, `jo`, `nr` (`nr`/`fr`), `pj` ; plus le **membre** `js.hb` (rappel de `ix[2]`) | `hy:lj` (`cv.hb`) | aucun | ignoré |
| `ldown` | 6 : `bg`, `fq`, `ie`, `jo`, `js`, `nr` | `ui:jz` | `qa`, `je.ra` (seul `bg:ldown`/`ie:ldown` les reçoivent, sans les lire) | nil, `false`, `true`, REF d'entité |
| `rdown` | 1 : `bg` (garde `rp` dans `mq`, rend `self`) | `ui:jz` | `qa`, `je.ra` | REF `bg` |
| `gv` | 2 : `jo` (nil ou `true`), `nr` (`true`/`false`) | `ui:jz("gv")` | l'entité tenue | vérité |
| `hq` | 4 : `bg` (lit `rp`, le garde dans `mq`), `bv` (`probe`, sur `g_`, `fv` et l'arbre `qh` hors `ga` ; écrit `ra` par nom), `jc` (garde `mp` dans le point souris), `mz` (passe à `bv`) | `ui:bh` | `qa`, `je.ra` | ignoré |
| `ey` | 3 : `d_` (`probe`, sur `g_`, `fv` et l'arbre `qh` ; ignore l'argument, rend une valeur ignorée), `jc`, `mz` (ignore) | `ui:bh` | l'entité trouvée par `jz("gv")` ou nil | ignoré |
| `kf` | 2 : `jo`, `mx` | `jc:ey` | le slipway | présence seulement (`assert`) |
| `cn` | 2 : `jo`, `nr` | `jo:ib`, `nr:cn` | `&proposition` (pile) | ignoré |
| `ol` | 2 : `lq` (vecteur, `a`), `jb` (`om` fixe, `a` ; reçoit 0 pour un vecteur) | `jo:kj` sur `self->lq` (`pico8_lq_t *`) | décalage vecteur, `a = 0.5` | ignoré |
| `mm` | 2 : `pico8_g__mm` (`g_`, `fv`, `hj`, `pm`), `pico8_ga_mm` | `ez` | aucun | la particule (`pico8_qy_t *` en REF) |
| `eb` | 1 : `jc` | `jo:ib`, `nr:cn`, `mx:kf`, `jo:kf`, `jc:ce` | `self` (le `jo`) | REF de l'autre bout ou nil |
| `ib` | 1 : `jo` (les 10) | `bd` (`cg.jo`) | aucun | `pico8_list_t *` |
| `gg` | 1 : `jo` (les 10) | `da`, `cz`, `sum_by_mi` (`cg.jo`) | aucun | vérité seule (`pico8_jo_gg` rend un `pico8_value_t`, TROU-3) |
| `md` | 1 : `qb` | `dc("kj")` sur `mo` (remplaçant provisoire sans `md`, ou le `qb`) | `ci[o]` fixe | ignoré |
| états `ph` | 5 : `iv`, `js`, `nf` (lit l'entrée), `pj`, `qb` (lit l'entrée) — aucun ne lit `t` | `dg` : `qc[qc.me](qc, qc.t)` | `t` | ignoré |
| états `ly` | 2 : `qh` (`pico8_na`, sur `qh`/`os`/`hj`/`pm`/`nr`/`fr`/`mx`), `nl` | `dg` | `t` fixe | ignoré |
| états `ll`, `kz`, `pr` | 1 chacun : `jo` (les 10), `mz`, `mz` | `dg` | `t` fixe | ignoré |

**`pico8_mv`** : 26 appels dans `src/` (7 lanceurs de a1, 19 de a2, tableau plus bas), sa définition (`entity.c:256`), sa déclaration (`entity.h:60`), un commentaire (`economy_score.h:13`). `pico8_entity_get_field` rend une méthode comme valeur (`entity.c:174-178`, ligne 177 comptée). Trois lecteurs de `src/` s'en servent : `lt` et `he` (présence des cinq noms `bm`, `entity.c:208, 238`) et `jc:hq` (présence de `gv` sur l'entité survolée, `slipway.c:243`).

**Membres lus par nom sous un nom de méthode** (grep des lecteurs `field`) : `bg.hd` et `fq.hd` (fixe 1, de `pico8_bg_start`/`pico8_fq_start`), `js.hb` (rappel), `js.my`, `fq.my`, `mh.lh` (rappels), `hp.lq` (donnée, ou `pico8_hp_lq_dispatch` quand NULL). Aucun autre lecteur ne répond à un nom de méthode.

**Prédicats par emplacement** (7) : `planet.c:247`, `probe.c:75`, `slipway.c:583` (`methods[HQ] == pico8_bv_dispatch`) ; `planet_family.c:139`, `probe.c:585`, `slipway.c:579`, `ui.c:391` (`methods[LY] != NULL`).

**Sites `par-nom` G et E** (annexe A de l'étude, relus à `1c9aae0`) : G = `bg.c:268` (`bg:rdown`, `rp`), `bg.c:296` (`bg:hq`, `rp`), `ke.c:43` (`ke:ks`, `me`), `research_icon.c:182` (`ie:hb`, `g_ie_gy`), `slipway.c:250` (`jc:hq`, `mp`), `slipway.c:261` (`jc:hq`, `mt`), `ui.c:444`, `ui.c:446` (`ui:bh`, `kn`), `ui_button.c:599` (`fq:ldown`, résultat de `my`) ; E = `entity.c:264` (`pico8_mv`, nom variable).

**TU.** `pico8_mv(` : 98 lignes dans 31 fichiers (noms littéraux : `hq` 13, `ph` 10, `ey` 10, `rdown` 8, `md` 6, `kj` 5, `lh` 4, `hb` 4, `ol` 3, `ldown` 3, `lq` 2, `my`, `mm`, `hd`, `ev`, `eb`, `p_` 1 ; le reste par des aides `call(e, name, …)` et des boucles sur des tableaux de noms). `pico8_value_function` : 26 lignes dans 15 fichiers, dont **17 servent des méthodes** (aides `method(e, name)` de `fr`, `fv`, `g_`, `ga`×2, `hj`, `mx`, `nr`, `pm` et leurs 4 lecteurs ; `want = …->methods[pico8_method_of(…)]` de `nl`, `os`, `qh` ; `fns[]` de `g_`) et **9 des rappels** (`fq` 3, `js` 4, `mh` 1, `pj` 1). 27 TU nomment `methods[`, `PICO8_METHOD_`, `pico8_method_of` ou un adaptateur. Des TU plient un retour de méthode dans une constante (`test_pico8_fr` l. 439-456, `test_pico8_nr` l. 444-451 : `gv`, `ldown` ; `test_pico8_fq` l. 445-457) ou la présence dans `cv` (boucles `bm` de `js`, `fr`, `hj`, `g_`, `ga`…).

## Solution

**Deux commits (chunks a1, a2), chacun vert seul**, sans pont compté :

1. **a1 — méthodes lancées par `dc`, `lt`, `he`, `ui:jz` et `dg`.** `pico8_struct_class_t` gagne les membres typés `kj`, `ev`, `hd`, `hk`, `hb`, `ldown`, `rdown`, `gv`, la table d'états `states[]` (`ph`, `ly`, `ll`, `kz`, `pr`) et le crochet de présence `has_hb` ; `methods[]` ne garde que les 9 méthodes de a2. Les 40 descripteurs passent aux initialiseurs désignés. `dc`, `lt`, `he`, `ui:gw`, `ui:bh`/`ui:jz`, `dg`, `hy:hk`/`hy:lj` lisent les membres ; `md` est appelé directement ; les prédicats sur `LY` lisent la table d'états ; `hd=1` de `bg`/`fq` devient la fonction constante vraie ; `ldown`/`rdown`/`gv` rendent `pico8_down_t {stop, held}` ; `jc:hq` lit la présence de `gv` sur le membre. 7 appels `pico8_mv` partent ; les TU qui nommaient ces méthodes suivent.
2. **a2 — le reste.** Membres typés `hq`, `ey`, `kf`, `cn`, `ol`, `mm` ; appels directs `pico8_jc_eb`, `pico8_jo_ib`, `pico8_jo_gg` ; appels directs des membres `hp.lq`, `mh.lh`, `js.my`, `fq.my` ; prédicats sur `HQ` ; les 19 derniers appels `pico8_mv` ; `pico8_mv`, `methods[]`, `pico8_method_t`, `pico8_method_of` et la branche méthode de `pico8_entity_get_field` supprimés ; commentaires de `pico8_dispatch_context.h` et `economy_score.h` réécrits ; les TU qui nommaient ces méthodes ou `pico8_mv` suivent ; glossaire, `trous-de-portage`, note « Découvert à D.10a ».

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (`make data-guard` : `résidu gardé (spec,overrides,stash,ob) : 0 ligne(s)`) ; `make e2e` : **26 conformes** ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (a1 et a2) ; balayage ASan+UBSan des TU propre ;
- `make data-residue` : `dispatch` **`src/` = 35 lignes / 37 occurrences / 15 fichiers** (les rappels de D.10b), `tests/` = 9 lignes (rappels) ; `par-nom` `src/` = 420 (433 − 13) ; `valeur`, `type-tag`, `ref-audit` en baisse, rapportés ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/` ni de `tools/` touché ; aucune constante `*_HASH`/`identity[]` changée (ou, pour une constante qui pliait la différence nil/`false` d'un retour de `ldown`/`gv`, reprise avec la sonde décrite plus bas, déclarée) ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  # 1. plus aucun pico8_mv, methods[], enum ou table de noms de méthode (tableaux de noms
  #    `methods[]` des TU compris) : vide
  grep -rnE '\bpico8_mv\b|methods\[|PICO8_METHOD_|pico8_method_(of|t)\b' src tests --include='*.[ch]'
  # 2. plus aucun adaptateur à arguments pico8_value_t pointé par une classe :
  #    ne rend que les 19 lignes de rappel (menu 10, cart 2, help 1, score_panel 1,
  #    score_panel_display 3, research 1, planet_display 1)
  grep -nE '^(static )?pico8_value_t [a-z0-9_]+\(void \*(self|self_ptr), int argc, pico8_value_t arg0, pico8_value_t arg1\)' src/*.c
  # 3. aucune conversion de pointeur de fonction (cast vers un type pointeur de fonction) : vide
  grep -rnE '\(\s*[a-z0-9_ ]+\s*\(\s*\*\s*\)\s*\(' src tests --include='*.[ch]'
  grep -rnE '\((const )?pico8_[a-z0-9_]+_fn(_t)?\)|\(pico8_value_function_t\)' src tests --include='*.[ch]'
  # 4. pico8_entity_get_field ne connaît plus les méthodes : vide
  grep -nE 'pico8_method|->methods|method_name' src/pico8_entity.c
  # 5. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^dispatch'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que chaque méthode ait sa signature C dans la table de sa classe, afin que le compilateur vérifie chaque appel et chaque surcharge (Q2).
2. En tant que mainteneur du port, je veux qu'aucun appel ne passe par un pointeur de fonction converti vers un autre type, afin qu'aucun appel ne soit un comportement indéfini.
3. En tant que mainteneur du port, je veux que les cinq états que lance `dg` soient une table typée indexée par un enum, `me` restant une chaîne convertie par `strcmp`, afin que `dg` garde la sémantique `qc[qc.me](qc, qc.t)` sans renommer `me` (Q3).
4. En tant que mainteneur du port, je veux que `eb`, `ib`, `gg`, `md` soient appelés directement, avec une vérification de la classe du receveur, afin qu'une méthode à implémentation unique ne coûte ni table ni aiguillage.
5. En tant que mainteneur du port, je veux que `hp.lq`, `mh.lh`, `js.my`, `fq.my` soient appelés directement par leur membre, afin que `pico8_mv` puisse disparaître avant que D.10b type ces rappels.
6. En tant que mainteneur du port, je veux que `pico8_mv` et `methods[]` disparaissent, afin que plus aucune méthode ne soit résolue par son nom.
7. En tant que mainteneur du port, je veux que `pico8_entity_get_field` ne rende plus de méthode comme valeur, afin qu'un lecteur par nom ne lise que des champs.
8. En tant que mainteneur du port, je veux que `lt` et `he` inscrivent une entité dans `cv` sur la présence d'un membre typé, avec exactement les mêmes entités qu'aujourd'hui (un `js` sans `hb` reste hors de `cv.hb`), afin que l'ordre et le contenu des registres ne bougent pas.
9. En tant que mainteneur du port, je veux que `hd=1` de `bg` et `fq` devienne une fonction constante vraie, afin que le survol et l'inscription dans `cv.hd` restent ce qu'ils sont.
10. En tant que mainteneur du port, je veux que `ldown`, `rdown` et `gv` rendent un petit struct `{stop, held}`, afin que `ui:jz` et `ui:bh` lisent la vérité et l'entité tenue sans valeur étiquetée.
11. En tant que mainteneur du port, je veux que les prédicats `*_is_*_class` testent les membres typés, afin de garder leur sens (`hq` partagé de la sonde, présence de l'état `ly`).
12. En tant que mainteneur du port, je veux que `pico8_dispatch_context` reste et que son commentaire dise ce qu'il sert encore, afin que `gfx`/`mem`/`input` n'élargissent pas les signatures (Q4).
13. En tant que gardien du compteur, je veux `dispatch` `src/` = 35 exactement, les 35 lignes étant les rappels de D.10b, afin que la ligne dise vrai sans pont.
14. En tant que gardien du compteur, je veux que chaque ligne `par-nom` retirée soit un site G, E ou une lecture de méthode par nom dont la cible disparaît, et que les sites G qui ne tombent pas avec un argument ou un retour typé par D.10a soient nommés avec leur étape, afin qu'aucun site ne soit « typé » en déplaçant le déballage.
15. En tant que gardien du compteur, je veux, au message de chaque commit, les comptes avant/après en lignes ET en occurrences par catégorie et par fichier, la classification (a)/(b)/(c) des lignes `dispatch` et `par-nom` retirées, les lignes qui gagnent une catégorie par fichier (« aucune » compris) et la justification de chaque Δlignes > Δoccurrences, afin qu'aucune baisse ne soit un tassement.
16. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, aucun alias, registre ni renommage ajouté pour échapper aux greps, et qu'aucune conversion typée ↔ valeur ne reste comme cale, afin que la baisse veuille dire « retiré ».
17. En tant que gardien des TU, je veux que chaque TU qui nommait `pico8_mv`, `methods[]`, `PICO8_METHOD_*`, un adaptateur ou une méthode lue par nom suive dans le chunk qui retire ce qu'il nomme, même intention, afin qu'aucun pont ne survive.
18. En tant que gardien des TU, je veux qu'un TU ne passe jamais d'un nom de méthode à un membre par une table ou un `switch` de noms, afin que les TU ne recréent pas le registre que D.10a retire.
19. En tant que gardien des constantes, je veux qu'aucune constante `*_HASH` ne change, chaque pli d'une méthode ou d'un retour de méthode reproduisant les mêmes octets depuis le membre typé, afin que l'identité de l'état reste prouvée.
20. En tant que gardien des constantes, je veux qu'une constante qui ne peut pas être reproduite (différence nil/`false` d'un retour typé en `{stop, held}`) ne soit reprise qu'avec une sonde qui prouve que le seul changement est le format du pli, afin qu'aucune mise à jour ne masque un changement d'état.
21. En tant que gardien des filets, je veux `make test` et `make e2e` verts à chaque commit, `class-coverage-net` identique à chaque commit et le balayage ASan+UBSan des TU propre à la fin, afin de prouver qu'aucun comportement n'a changé.
22. En tant que gardien de `docs/trous-de-portage.md`, je veux que TROU-11, dont le repli d'arité disparaît avec l'`argc` des adaptateurs, garde son numéro et dise qu'il n'a plus de site, afin que la numérotation reste stable.
23. En tant que lecteur du glossaire, je veux que la ligne `mv` dise ce que devient l'appel, afin que le glossaire ne renvoie pas à une fonction supprimée.
24. En tant que `plan-pilot`, je veux une note « Découvert à D.10a » qui dise ce qui change pour D.10b, D.10c, D.11a, D.11b et D.11c (rappels laissés, TU absorbés, sites G reportés, lectures de présence prises à D.11b), afin que les PRD suivants partent d'un état exact.
25. En tant que `plan-pilot`, je veux les inconnues d'exécution écrites en hypothèses H1…Hn, afin que le vérificateur sache quoi confirmer.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q1-Q11 sont des décisions humaines (plan, « Décisions de D.9 ») : ce PRD les applique, il ne les rouvre pas.

### Modules

- **`struct`** (l'en-tête des descripteurs) : la table de classe typée, le résultat `{stop, held}`, l'enum des états et sa conversion par `strcmp`. C'est le module profond de l'étape : une interface stable (un membre nommé par méthode virtuelle, une case par état) derrière laquelle chaque classe range ses implémentations ; elle se teste par les TU de classe (présence, identité, effet d'un appel) sans rien savoir des lanceurs.
- **`entity`** : `lt`, `he` et `dc` lisent les membres ; `pico8_hd_true` (la fonction constante vraie de `hd=1`) y est définie une fois ; `pico8_mv` et la branche méthode de `pico8_entity_get_field` disparaissent (a2).
- **`bootstrap`** : `dg` passe par la table d'états.
- **`ui`** : `gw` (survol), `bh`, `jz` (sélecteur d'emplacement typé au lieu du nom).
- **`economy`** : les boucles `cv.hk`/`cv.hb` de `hy:hk`/`hy:lj`.
- **`entity_geom`** : `pico8_gk` prend des vecteurs typés et rend un entier.
- **Classes** (21 fichiers de descripteurs) : chaque adaptateur de méthode devient une fonction à la signature exacte de son emplacement, sous **le même nom** (`pico8_X_Y_dispatch`, `pico8_g__mm`, `pico8_ga_mm`) : le nom ne correspond à aucun motif compté, les tables des filles et les comparaisons d'identité (`== pico8_bv_dispatch`) le gardent, et le changement est de type, pas de nom. Rejeté : renommer (`_slot`, `_m`) — churn dans 40 descripteurs et 20 TU sans rien retirer au compteur.
- **Aucun module nouveau.**

### La table de classe (G)

Forme retenue (fragment de décision, pas un code à recopier) :

```c
struct pico8_qy;                          /* particle.h : typedef struct pico8_qy {…} pico8_qy_t */
struct pico8_jo_trade_proposal;           /* déjà nommé dans planet_trade.h */
typedef struct pico8_struct_class pico8_struct_class_t;
typedef struct { pico8_type_tag_t tag; const pico8_struct_class_t *cls; } pico8_struct_header_t;

/* ldown, rdown, gv : `stop` = vérité Lua du retour ; `held` = l'entité rendue, NULL sinon.
   held != NULL implique stop. */
typedef struct { int stop; pico8_struct_header_t *held; } pico8_down_t;

typedef enum { PICO8_STATE_PH, PICO8_STATE_LY, PICO8_STATE_LL, PICO8_STATE_KZ, PICO8_STATE_PR,
               PICO8_STATE_COUNT } pico8_state_t;
typedef void (*pico8_state_fn)(void *self, pico8_fixed_t t);

struct pico8_struct_class {
    void (*kj)(void *self, const pico8_vec_t *ra);            /* NULL ra : nil */
    void (*ev)(void *self, const pico8_vec_t *ra);
    int (*hd)(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
    void (*hk)(void *self, pico8_fixed_t yr);
    void (*hb)(void *self);
    int (*has_hb)(const void *self);   /* NULL : `hb` non NULL suffit ; sinon l'instance décide */
    pico8_down_t (*ldown)(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
    pico8_down_t (*rdown)(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
    pico8_down_t (*gv)(void *self, const pico8_struct_header_t *s);
    void (*hq)(void *self, pico8_vec_t *qa, pico8_vec_t *ra);   /* peut garder qa/ra */
    void (*ey)(void *self, pico8_struct_header_t *kv);
    void (*kf)(void *self, pico8_struct_header_t *sw);
    void (*cn)(void *self, struct pico8_jo_trade_proposal *lw);
    void (*ol)(void *self, pico8_vec_t p, pico8_fixed_t a);
    struct pico8_qy *(*mm)(void *self);
    pico8_state_fn states[PICO8_STATE_COUNT];
    pico8_struct_field_fn field;
    pico8_struct_set_field_fn set_field;
};
```

- **Plat, pas imbriqué** : les membres sont ceux de `pico8_struct_class_t`, lus `cls->kj`. Pendant a1, `methods[]` (réduit aux 9 noms de a2) coexiste dans la même struct : un sous-struct nommé `methods` entrerait en collision avec lui. Rejeté : un sous-struct `m` (renommage sans objet à a2).
- **`self` en `void *`** (proposition de l'étude) : chaque implémentation a un `self` concret ; la fonction d'emplacement le convertit (conversion de pointeur d'objet, définie), jamais le pointeur de fonction. Rejeté : `pico8_struct_header_t *self` (même besoin d'une fonction d'emplacement, rien de gagné).
- **Pointeurs de vecteur `const` pour `kj`, `ev`, `hd`** (lus, jamais gardés) ; **non `const` pour `ldown`, `rdown`, `hq`** : `bg:rdown`/`bg:hq` gardent `je.ra` dans `bg.mq`, `jc:hq` garde `qa` dans son point souris, `bv` écrit `qa` dans `ra` (le membre garde le même pointeur qu'aujourd'hui le REF).
- **`pico8_qy_t` reçoit une étiquette de struct** (`typedef struct pico8_qy {…} pico8_qy_t;`) pour être nommé dans `pico8_struct.h` sans dépendance circulaire ; le type ne change pas.
- **`hd` rend `int`** (vérité), **`gv` rend `pico8_down_t`** (consigne du pilote, même type que `ldown`/`rdown` ; seul `stop` est lu, `held` est toujours NULL : aucun `gv` ne rend d'entité).
- **Un emplacement NULL veut dire « la classe n'a pas cette méthode »**, là où `mv` rendait 0.

### Signatures et implémentations, par emplacement (G)

| Emplacement | Implémentations (nom gardé) | Corps après D.10a | Chunk |
|---|---|---|---|
| `kj` | `bg`, `ei`, `fv`, `ga`, `hj`, `ie`, `iv`, `jc`, `jo`, `js`, `mz`, `pj` `_kj_dispatch` / `pico8_ei_dispatch` | `p = ra ? *ra : (0,0)` là où la fonction typée prend `p` ; `(void)ra` sinon ; `gfx`/`mem` par `pico8_dispatch_context` (Q4) | a1 |
| `ev` | `dl`, `jc`, `js`, `mh`, `ui` | `(void)ra` ; appel inchangé | a1 |
| `hd` | `fv`, `ie`, `jo`, `js` → `pico8_gk(self, qa, ra)` ; `bg`, `fq` → `pico8_hd_true` | le repli d'arité de `js` (`rp = argc >= 2 ? arg1 : arg0`) disparaît : l'emplacement a toujours deux arguments (TROU-11 sans site) | a1 |
| `hk` | `jo`, `pj` | `(void)yr` / `(void)self` comme aujourd'hui | a1 |
| `hb` | `ie`, `jo`, `nr`, `pj` ; `js` : **nouvelle** `pico8_js_hb_dispatch` (appelle `self->hb` s'il n'est pas NULL) + `has_hb` | voir « Présence par instance » | a1 |
| `ldown` | `bg`, `fq`, `ie`, `jo`, `js`, `nr` | rend le `pico8_down_t` de la fonction typée | a1 |
| `rdown` | `bg` | idem | a1 |
| `gv` | `jo`, `nr` | idem | a1 |
| `hq` | `bg`, `bv` (partagé : `g_`, `fv`, arbre `qh` hors `ga`), `jc`, `mz` | appel de la fonction typée retypée | a2 |
| `ey` | `d_` (partagé), `jc`, `mz` | `d_` : `(void)kv; (void)pico8_d_(self);` — le retour `pico8_value_t` de `pico8_d_` reste (Q12) ; `mz` : `(void)kv` | a2 |
| `kf` | `jo`, `mx` | appel direct de la fonction typée | a2 |
| `cn` | `jo`, `nr` | idem | a2 |
| `ol` | `lq`, `jb` | `lq` : `pico8_lq_ol(gfx, mem, self, p, a)` ; `jb` : `(void)p; pico8_jb_ol(gfx, mem, self, 0, a)` — exactement ce que l'adaptateur faisait d'un vecteur (`om` = 0), le seul lanceur passant un vecteur | a2 |
| `mm` | `pico8_g__mm`, `pico8_ga_mm` | signature `pico8_qy_t *(void *self)` : ce sont elles-mêmes les fonctions d'emplacement | a2 |
| états | `ph` : `iv`, `js`, `nf`, `pj`, `qb` ; `ly` : `qh`, `nl` ; `ll` : `jo` ; `kz`, `pr` : `mz` | signature `pico8_state_fn` ; `(void)t` pour `ph` | a1 |

**Fonctions typées dont la signature change** (appelées aujourd'hui seulement par leurs adaptateurs et par les TU, grep à `1c9aae0`, H3) :

- a1 : `pico8_gk(const pico8_struct_header_t *e, const pico8_vec_t *mp, const pico8_vec_t *rp)` → `int` ; `pico8_bg_ldown(pico8_bg_t *self)` (le `mp` jamais lu part) → `pico8_down_t` ; `pico8_bg_rdown(pico8_bg_t *self, pico8_vec_t *rp)` ; `pico8_ie_ldown(pico8_ie_t *self)` ; `pico8_jo_ldown`, `pico8_nr_ldown`, `pico8_js_ldown`, `pico8_fq_ldown` → `pico8_down_t` ; `pico8_jo_gv`, `pico8_nr_gv` → `pico8_down_t`.
- a2 : `pico8_bg_hq(pico8_bg_t *self, pico8_vec_t *rp)` (le `mp` jamais lu part) ; `pico8_bv(pico8_struct_header_t *self, pico8_vec_t *mp, pico8_vec_t *rp)` ; `pico8_mz_hq(pico8_mz_t *self, pico8_vec_t *mp, pico8_vec_t *rp)` ; `pico8_jc_hq(pico8_jc_t *self, pico8_vec_t *mp)` ; `pico8_jc_eb(const pico8_jc_t *self, const pico8_struct_header_t *jo)` → `pico8_struct_header_t *` (l'autre bout, NULL pour nil) ; `pico8_g__mm`, `pico8_ga_mm`.
- **Inchangées** : `pico8_jo_gg` (rend `pico8_value_t`, TROU-3 : `hf` booléen ou fixe, Q12), `pico8_d_`, `pico8_jo_ib` (`pico8_list_t *`), `pico8_qb_md`, `pico8_hp_lq`, `pico8_ke_ks`, les fonctions typées des autres emplacements.
- **Correspondance des retours** (liant, vérifiée par les TU) : `nil`/`false` → `{0, NULL}` ; `true` → `{1, NULL}` ; REF d'entité → `{1, entité}`. `nr:ldown` rend le `pico8_down_t` de `nr:gv` quand celui-ci refuse. La différence nil/`false` disparaît : son seul lecteur, `ui:jz`, ne lisait que la vérité (H4).

### Table d'états (Q3, G)

- `pico8_state_of(const char *me)` remplace `pico8_method_of` dans `pico8_struct.h` : `strcmp` sur les cinq noms `ph`, `ly`, `ll`, `kz`, `pr`, rend l'état ou -1 (contrat écrit : -1 pour `unknown`, `known` et tout autre nom). Le tableau de 23 noms devient un tableau de 5 : c'est la conversion que Q3 prescrit, pas un registre nouveau.
- `dg` : `me` lu comme aujourd'hui (lecture par nom, classe A, D.11a) ; état ≥ 0 et case non NULL → `qc->cls->states[état](qc, t)`. `t` : la lecture par nom existante de `t`, déballée comme chaque adaptateur d'état le faisait (fixe, sinon 0 ; `ke.t` est toujours un fixe, H5). Rejeté : lire `((pico8_ke_t *)qc)->t` ici — c'est le site A de D.11a.
- `me` qui nommerait une méthode non-état (`kj`…) : `mv` l'aurait appelée ; aucune valeur de `me` ne le fait (étude §2.3 : sept valeurs, cinq états, `unknown`, `known` ; H6). Pas d'`assert` ajouté : `dg` ignore, comme `mv` ignorait un nom sans méthode.

### Appels directs (G)

Le receveur est vérifié par un `assert` porteur de message, là où `mv` rendait 0 en silence ; le cart fixe la famille (H7).

- **`eb`** (5 sites : `pft.c:183, 365`, `planet_trade.c:67, 251`, `slipway.c:400`) : `pico8_jc_eb(…, self)`, après `assert(w->cls == pico8_jc_struct_class() && "…")` (idiome déjà présent à `slipway.c:396`). Receveurs : les éléments de `jo.oe` (seul `jo:kf` y ajoute, des slipways) et le `sw` que `jc:ey` passe à `kf`. Résultat comparé à `NULL` ou à `re` (`slipway.c:400` : deux NULL sont égaux, comme `pico8_value_equal(nil, ref(NULL))`).
- **`ib`** (`economy_tiers.c:95`) et **`gg`** (`economy_score.c:77`) : receveurs pris dans `cg.jo`. Vérification : `pico8_jo_is_family(pt)` (nouvelle, publique, `planet.c` : l'état `ll` de la classe est `pico8_jo_ll_dispatch`, que portent les dix classes de l'arbre et elles seules — même idiome que `== pico8_bv_dispatch`). `ib` : la liste rendue est écrite par nom dans `by` (site A, D.11a) — l'emballage en REF reste à ce seul site ; le parcours lit la liste typée (la ligne `PICO8_REF_AS_LIST_CONST(by_val.as.ref)` part, `ref-audit` −1, déclaré). `gg` : vérité du `pico8_value_t` rendu, par le prédicat existant. Rejeté : la présence d'un membre `gg`/`ib` (donnée sans lecteur) ; le lien parent de Q5 (D.11b).
- **`md`** (`entity.c:314`) : `if (g_mo != NULL && g_mo->cls == pico8_qb_struct_class()) pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)g_mo, ci);` — le remplaçant provisoire de `mo`, et toute classe de TU sans `md`, ne reçoivent rien, comme avec `mv`. Pas d'`assert` : le remplaçant est un état légitime avant `jl()`.
- **`lq`** (`planet_display.c:365`) : `b` est un `pico8_hp_t *` ; `b->lq != NULL` → la rangée (`pico8_lq_keys_of_row`, `keys->lq_row`) ; sinon `keys->lq = pico8_value_ref(pico8_hp_lq(b))` (le membre `keys->lq` reste `pico8_value_t`, clé de `js`, D.10b/Q12). Le test de tag qui distinguait une rangée d'un `lq` construit et l'`assert` « hp must expose lq » partent : le type les rend sans objet (`type-tag` −1, déclaré).
- **`lh`** (`msgbox.c:93`) : `lh_found = self->lh != NULL` ; appel `self->lh(self, 0, nil, nil)` ; le membre reste un `pico8_value_function_t` (D.10b).
- **`my`** (`ui_button.c:485`, `596`) : `js:ldown` appelle `self->my` si `self` et `my` ne sont pas NULL (résultat ignoré ; un `my` présent à `false` n'appelait rien) ; `fq:ldown` prend `self->my ? self->my(self, 0, nil, nil) : nil` et le passe au déballage existant de `p_` (site G `ui_button.c:599`, D.10b).

### Présence par instance : `js.hb` (G)

`lt`/`he` inscrivent une entité dans `cv.hb` quand `e.hb` existe. Pour toutes les classes, c'est une méthode ; pour `js`, c'est un membre posé par la clé de construction (seul `ix[2]` en porte un). Des TU plient l'appartenance à `cv` dans leurs constantes (boucles `bm` de `js`, `fr`, `hj`, `g_`, `ga`…), et `hy:lj` parcourt `cv.hb` : inscrire tous les `js` changerait les registres.

- **Retenu : le membre `has_hb` de la table de classe**, NULL pour toutes les classes sauf `js`, dont la fonction rend `self->hb != NULL`. Présence de `hb` = `cls->hb != NULL && (cls->has_hb == NULL || cls->has_hb(e))`. L'emplacement `hb` de `js` est `pico8_js_hb_dispatch`, qui appelle le rappel. D.10b, qui type `js.hb`, garde ce crochet.
- **Rejetés** : inscrire tous les `js` (registres changés) ; une seconde classe `js` pour les boutons qui portent `hb` (l'identité `cls == pico8_js_struct_class()` est lue par 6 TU) ; garder la présence par `pico8_entity_get_field` pour `hb` (lecture par nom d'un rappel, qui cesse de répondre à D.10b) ; une branche propre à `js` dans `entity.c`.

### `hd=1` (G)

`bg` et `fq` ont `hd` = `pico8_hd_true` (`entity.c`, déclarée dans `entity.h`, **une seule définition** : une fonction `static inline` d'en-tête aurait une adresse par unité de compilation). Elle rend 1 sans lire ses arguments ; `mv` rendait le fixe 1, vrai. Inscription dans `cv.hd` et survol inchangés. Les membres `bg.hd`/`fq.hd`, leur valeur de départ et la branche `"hd"` de leurs lecteurs par nom **restent** (lecture par nom, D.11 ; retirer une valeur générée touche `pico8_game_data.*` sans rien retirer au compteur) ; le jeu ne les lit plus, dit au message.

### Les 26 appels `pico8_mv` de `src/` (G)

| # | Site | Méthode | Remplacement | Chunk |
|---|---|---|---|---|
| 1 | `bootstrap.c:86` (`dg`) | état `me` | case de `states[]` | a1 |
| 2 | `economy.c:232` (`hy:hk`) | `hk` | `cls->hk(e, new_yr)` si non NULL | a1 |
| 3 | `economy.c:341` (`hy:lj`) | `hb` | `cls->hb(e)` si non NULL | a1 |
| 4 | `entity.c:314` (`dc`) | `md` | direct, garde de classe `qb` | a1 |
| 5 | `entity.c:329` (`dc`) | `kj`/`ev` | membre choisi par `nt` | a1 |
| 6 | `ui.c:357` (`ui:gw`) | `hd` | `cls->hd(e, qa, je_ra)` | a1 |
| 7 | `ui.c:499` (`ui:jz`) | `ldown`/`rdown`/`gv` | membre choisi par le sélecteur ; `stop` | a1 |
| 8 | `economy_score.c:77` | `gg` | direct | a2 |
| 9 | `economy_tiers.c:95` | `ib` | direct | a2 |
| 10 | `msgbox.c:93` | `lh` | membre | a2 |
| 11 | `particle.c:81` (`ez`) | `mm` | `cls->mm(self)` si non NULL | a2 |
| 12 | `planet_display.c:365` | `lq` | membre ou `pico8_hp_lq` | a2 |
| 13 | `planet_display.c:493` (`jo:kj`) | `ol` | `self->lq->header.cls->ol(…)`, `assert` de présence gardé | a2 |
| 14, 15 | `pft.c:183`, `189` (`nr:cn`) | `eb`, `cn` | direct, puis `cls->cn` si non NULL | a2 |
| 16 | `pft.c:365` (`mx:kf`) | `eb` | direct | a2 |
| 17 | `planet_trade.c:67` (`jo:kf`) | `eb` | direct | a2 |
| 18, 19 | `planet_trade.c:251`, `254` (`jo:ib`) | `eb`, `cn` | direct, puis `cls->cn` | a2 |
| 20, 21 | `slipway.c:304`, `305` (`jc:ey`) | `kf` | `cls->kf(qu, self)`, puis `re`, puis l'`assert` existant sur les deux présences (même ordre) | a2 |
| 22 | `slipway.c:400` (`jc:ce`) | `eb` | direct | a2 |
| 23 | `ui.c:453` (`ui:bh`) | `hq` | `cls->hq(held, qa, je_ra)` si `held` et le membre ne sont pas NULL | a2 |
| 24 | `ui.c:467` (`ui:bh`) | `ey` | `cls->ey(held, kv)` si non NULL | a2 |
| 25, 26 | `ui_button.c:485`, `596` | `my` | membre | a2 |

`e == NULL` : `mv` rendait 0 ; chaque remplacement garde ce test où le receveur peut être NULL (`held`, `sw`, `self`).

### Lanceurs (G)

- **`lt`/`he`** : les cinq noms `bm` restent l'ordre d'inscription (`kj`, `ev`, `hd`, `hk`, `hb`) ; la présence de chacun se lit sur son membre (`hb` avec `has_hb`), par une fonction locale à `entity.c` qui associe l'indice `bm` à son membre (le tableau `g_bm_names` existe déjà, il nomme les listes de `cv`). Les deux lectures `pico8_entity_get_field(e, g_cv[i].name, …)` partent (classe C, retirées à D.10a parce que leur cible disparaît, déclaré). La lecture de `nu` reste (C, D.11b).
- **`dc(nt)`** : garde son paramètre chaîne (`cv(nt)`, et `cart.c` l'appelle avec `"kj"`/`"ev"`, inchangé) ; choisit `kj` ou `ev` par `strcmp`, **`assert`** sur tout autre nom (un `dc("hd")` n'a plus de membre à lancer : c'est la fin de TROU-11) ; `ra` : la lecture par nom existante, un REF non NULL devient le pointeur, sinon NULL (ce que chaque adaptateur de `kj` faisait) ; membre NULL → rien (comme `mv`).
- **`ui:gw`** : `qa` est déjà `pico8_vec_t *` ; `je.ra` : la lecture par nom existante (`pico8_ui_field_raw`, classe D, D.11a), un REF non NULL devient le pointeur.
- **`ui:jz`** : `int pico8_ui_jz(pico8_ui_t *self, pico8_jz_slot_t rm, pico8_vec_t *qa, pico8_vec_t *ra, const pico8_struct_header_t *s, pico8_struct_header_t **out_kv, pico8_down_t *out_kn)`, `pico8_jz_slot_t` ∈ {`LDOWN`, `RDOWN`, `GV`} (enum de `ui.h`) ; un `switch` de trois cas choisit le membre (c'est le « paramètre qui choisit l'emplacement » de l'étude §2.3 ; il remplace la résolution par nom, déclaré). Parcours des couches inchangé (15 → 0, premier `stop` gagnant).
- **`ui:bh`** : `down` devient le sélecteur ; si `jz` gagne et `held != NULL` : `lheld = held` (gauche) ou `rheld = (pico8_bg_t *)held` (droite), chacun sous l'`assert` de classe que portaient `ui_take_held`/`ui_take_bg` (`ui_is_held_class`, `== pico8_bg_struct_class()`, messages ramenés au contrat qui reste) ; `/* TROU-6 */` reste au site. `ui_take_held`/`ui_take_bg` perdent leurs appelants et partent.

### Prédicats (G)

`methods[HQ] == pico8_bv_dispatch` → `cls->hq == pico8_bv_dispatch` (`planet.c:247`, `probe.c:75`, `slipway.c:583`, a2) ; `methods[LY] != NULL` → `cls->states[PICO8_STATE_LY] != NULL` (`planet_family.c:139`, `probe.c:585`, `slipway.c:579`, `ui.c:391`, a1). Comparaison de deux pointeurs de même type, aucune conversion.

### `pico8_entity_get_field` sans méthode (G)

a2 retire la recherche de méthode (`entity.c:170-178`) : le lecteur ne rend plus que les champs du descripteur. Ses lecteurs de `src/` qui visaient une méthode sont convertis dans le chunk de la méthode : `lt`/`he` (a1, plus haut) et **`jc:hq`** (`slipway.c:243`, présence de `gv` sur l'entité survolée, a1) → `hovered->cls->gv != NULL`. Ce dernier est un site C de D.11b, pris ici parce que sa cible disparaît (sinon `jc:hq` ne trouverait plus jamais `gv` : comportement changé) ; déclaré. Aucun autre lecteur de `src/` ne nomme une méthode (grep des 23 noms, `1c9aae0`).

### Sites `par-nom` G et E (G)

- **Retirés par D.10a, parce que l'argument ou le retour qu'ils déballaient arrive typé** : `bg.c:268` (`rdown`, a1), `ui.c:444`, `ui.c:446` (retour de `ldown`/`rdown`, a1), `bg.c:296` (`hq`, a2), `slipway.c:250` (`hq`, a2), et **E** `entity.c:264` (a2, avec `pico8_mv`). Les aides devenues sans appelant partent avec eux : `ui_take_held`, `ui_take_bg` (a1), `bg_take_vec`, `pico8_jc_mouse_point_take_ra` (a2) — lignes d'infrastructure `par-nom` (a).
- **Laissés, avec leur étape** : `ke.c:43` (`me` de `ke:ks` : `ks` n'est pas une méthode de la table ; son argument vient de `dq` lu par nom dans `d_`, classe M → D.11c) ; `slipway.c:261` (`mt` lu par nom sur `qu`/`re`, classe A → D.11a) ; `research_icon.c:182` (`g_ie_gy`, état de classe `ie.gy` en `pico8_value_t`, lu par nom et posé par `pico8_ie_set_gy` → D.11a) ; `ui_button.c:599` (retour du rappel `my` → D.10b). Les retirer ici demanderait de déballer la même valeur à l'appelant : un déplacement, interdit. Rejeté : les retirer quand même pour tenir « G (9) ».

### Frontières par nom qui restent (liant)

Aux sites où une lecture ou une écriture par nom qui survit (ou un conteneur de `pico8_value_t`) rencontre un emplacement typé, la conversion se fait **au site de cette lecture, une fois**, et part avec elle : `t` de `dg` (A, D.11a), `ra` de `dc` (C, D.11b), `je.ra` de `ui:gw`/`ui:bh` (D, D.11a), écriture de `ra` par `bv` (M, D.11c), écriture de `by` par `bd` (A, D.11a), ajout de la particule dans `qy` (liste L2, D.12b), `keys->lq` de `jo:fe` (clé `pico8_value_t` de `js`, D.10b/Q12). Ce ne sont pas des cales : chaque conversion existait dans un adaptateur ou dans `mv` et se trouve maintenant au seul site de la lecture qui l'impose. **Interdit** : une fonction ou macro qui convertit un emplacement typé en appel à valeurs (ou l'inverse) pour plusieurs sites ; une conversion ajoutée hors de cette liste. Chaque site est déclaré au message (lignes qui gardent ou gagnent `valeur`).

### Pont des TU : aucun (G)

- **Retenu : les TU suivent dans le chunk qui retire ce qu'ils nomment ; `pico8_mv` disparaît à a2 sans pont.** `dispatch` `src/` = 35 (pas 37).
- Raisons : (1) un pont ne dispense pas les TU de changer : 27 TU nomment `methods[`, `PICO8_METHOD_*`, `pico8_method_of`, un adaptateur à `pico8_value_t` ou une méthode lue par `pico8_entity_get_field`, que D.10a retire de toute façon ; 4 TU définissent des descripteurs positionnels ; 8 appellent une fonction retypée ; (2) un `pico8_mv` de pont devrait retraduire chaque nom vers son membre et chaque argument `pico8_value_t` vers son type — un `switch` de noms, c'est-à-dire le registre et la cale que la consigne interdit ; (3) il ne pourrait pas rendre à `test_pico8_jb_ol` le `om` fixe que l'emplacement `ol` ne porte plus.
- Conséquence (note du plan) : **D.10c perd les 98 lignes `pico8_mv` et les 17 lignes `pico8_value_function` de méthode** ; il lui reste les 9 lignes de rappel des TU (`fq`, `js`, `mh`, `pj`) et ce que D.10b y ajoute.

### Les rappels restent des `pico8_value_function_t` (G)

Les 19 lignes de rappel de la grep des « 82 » (`menu`, `cart`, `help`, `score_panel`, `score_panel_display`, `pico8_hp_lq_dispatch`, `pico8_jo_fe_my_dispatch`), les membres `js.my`/`hb`, `fq.my`, `mh.lh`, et les lecteurs qui les rendent comme valeur (`msgbox.c:134`, `research.c:202`, `ui_button.c:89, 159, 512`) ne changent pas : c'est D.10b. `pico8_hp_lq_dispatch` n'a plus d'appelant par `mv` après a2 ; il reste la valeur que le lecteur de `hp` rend pour `"lq"` (D.10b).

### `pico8_dispatch_context` (Q4)

Gardé. Son commentaire (l. 7-12) est ramené au contrat actuel : l'état de frame (`gfx`, `mem`, `input`) des méthodes atteintes par un membre de `pico8_struct_class_t`, dont la signature ne le porte pas ; mêmes règles d'emprunt. La l. 9 (`dispatch`) part (a2).

### Initialiseurs désignés (G)

a1 réécrit les 40 descripteurs de `src/` et les 4 de TU en initialiseurs désignés (`.kj = …, .field = …, .set_field = …`) : avec `-Wextra -Werror`, un initialiseur positionnel qui laisse des membres (`{{NULL}, f, NULL}`) déclenche `-Wmissing-field-initializers` ; un désigné non (H1). a2 retire les `.methods = {…}`. Aucune entrée ne change de valeur ; les tables des filles gardent les mêmes pointeurs que leurs parentes.

### TROU-11 et TROU-6 (G)

- **TROU-11** : le repli d'arité de `js:hd` et le `dc` qui pouvait lancer `hd` avec un argument disparaissent (a1). Numéro et texte gardés ; la ligne « Fichiers » devient « Sans site depuis D.10a : l'emplacement `hd` a toujours deux arguments et `dc` ne lance que `kj`/`ev` ». Les marques `/* TROU-11 */` (`entity.c:328`, `ui.c:356`) partent avec leurs lignes.
- **TROU-6** : `held` reste casté par `ui:bh` (vers `pico8_bg_t *` pour `rheld`), sous les mêmes `assert` de classe : inchangé, marque gardée.

### Documentation (G)

- `docs/glossaire-noms.md` : la ligne `mv(e,rm,...)` dit que l'appel passe par le membre typé de la classe du receveur, ou par la fonction directe pour une méthode à implémentation unique (`pico8_mv` supprimé) ; la ligne `dc(nt)` garde son sens.
- `docs/trous-de-portage.md` : TROU-11 (ci-dessus).
- `docs/plan-3.md` : note « Découvert à D.10a » en fin de ligne D.10a (texte en « Further Notes »), rien d'autre.
- Les PRD livrés, `docs/plan.md`, `docs/plan-2.md` : historiques, non touchés.

### Ce qui ne change pas

- `tools/` (motifs figés de `data_residue.sh` : `dispatch` reste `\bpico8_value_function(_t)?\b|\bpico8_mv\(`), le `Makefile` (H13), `tests/e2e/`, tout `*.sha256`, tout fichier d'événements ; aucun e2e ajouté.
- Les rappels et `pico8_value_function_t` (D.10b) ; `pico8_value.c/.h` ; les lecteurs et écrivains par nom hors des sites listés ; `me` reste une chaîne (Q3) ; `DATA_RESIDUE_GUARD` (`dispatch` ne tombe pas à 0).
- L'ordre des appels : chaque lanceur appelle les mêmes implémentations, dans le même ordre, avec les mêmes valeurs.

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours ; macro, `typedef`, constante, `enum`, pointeur ou fonction enveloppe qui renomme un identifiant vivant ; un alias ; un registre (tableau, table, `switch` ou boucle nouveaux qui nomment ensemble des valeurs, cas ou noms), **hors** les trois déclarés : les 5 noms de `pico8_state_of` (Q3, réduction du tableau de 23), le `switch` à 3 cas du sélecteur de `ui:jz`, la fonction de `entity.c` qui associe les 5 indices `bm` à leurs membres ; une conversion typée ↔ valeur hors de la liste « Frontières » ; une conversion de pointeur de fonction ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; supprimer un commentaire qui ne porte pas sur une construction retirée ; tout changement d'une constante de hachage hors de la sonde décrite.
- **Classification exigée**, au message de chaque commit, pour chaque ligne `dispatch` et `par-nom` retirée : **(a)** construction supprimée (appel `pico8_mv`, définition, déclaration, branche de lecture, aide sans appelant, commentaire d'une construction retirée), **(b)** déclaration, signature ou variable dont le type passe de `pico8_value_t`/`pico8_value_function_t` à un type C, **(c)** même donnée, autre nom. Attendu : **(c) = 0**, **(b) = 0** pour `dispatch` et `par-nom` en `src/` (les lignes retypées ne portaient pas ces motifs ; les (b) sont en `valeur`, rapportées). `dispatch` : a1 (a) 7 (les appels 1-7) ; a2 (a) 25 : les 19 appels, la définition et la déclaration de `pico8_mv`, la branche méthode de `pico8_entity_get_field` (`entity.c:177`), le membre `methods[]` (`pico8_struct.h:89`), et deux commentaires dont la phrase décrit un appel supprimé (`economy_score.h:13`, `pico8_dispatch_context.h:9`). `par-nom` `src/` : a1 (a) 8 (`entity.c:208`, `238`, `slipway.c:243`, `ui.c:444`, `446`, `bg.c:268`, définitions `ui.c:394`, `406`) ; a2 (a) 5 (`entity.c:264`, `bg.c:296`, `slipway.c:250`, définitions `bg.c:104`, `slipway.c:115`). Même classification pour les lignes des TU. Recette : `git diff -U0` par fichier, lignes `-` qui portent le motif en regard des lignes `+`.
- **Mesure exigée**, avant et après chaque commit, par catégorie, **lignes** (`make data-residue`, `--detail` par fichier) **ET occurrences** (recette figée de D.8, `$S` : dossier du scratchpad) :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur type-tag ref-audit par-nom dispatch; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  ```

  Mesuré sur `1c9aae0` : `valeur` 5742, `type-tag` 560, `ref-audit` 57, `par-nom` 1064, `dispatch` 193 (`src/` 69, `tests/` 124). Le détail par fichier (lignes et occurrences) de chaque fichier du diff est donné avant et après.
- **Contrôle** : pour chaque (catégorie, fichier) du diff, Δlignes et Δoccurrences ; si Δlignes > Δoccurrences en valeur absolue d'une baisse, le message justifie le couple (une ligne qui portait deux occurrences, ou une ligne gardée qui en perd une).
- **Lignes qui gagnent une catégorie**, listées par fichier et par catégorie, « aucune » compris. Attendues (à confirmer) : `valeur` aux sites de la liste « Frontières » quand la conversion arrive sur une ligne qui n'en portait pas ; `par-nom` : aucune ; `dispatch` : aucune ; `type-tag` : aucune.

### Empreinte remesurée et découpage (G)

| Chunk | `src/` | `tests/` | docs | `dispatch` retirées (`src/`) |
|---|---|---|---|---|
| a1 | ≈ 37 : `pico8_struct.h`, `entity.c/.h`, `bootstrap.c`, `ui.c/.h`, `economy.c`, `entity_geom.c/.h`, les 18 autres fichiers de descripteurs (`bg`, `help`, `layout`, `menu`, `mouse_reader`, `msgbox`, `planet`, `planet_family`, `planet_family_trade`, `probe`, `research`, `research_icon`, `score_panel`, `scroll`, `slipway`, `trade`, `ui_button`, `wormhole`), `particle.c/.h`, et les en-têtes des adaptateurs publics ou des fonctions retypées (`bg.h`, `help.h`, `planet.h`, `planet_family.h`, `planet_family_trade.h`, `probe.h`, `research_icon.h`, `ui_button.h`) | ≈ 30 (ceux qui lancent ou inspectent une méthode de a1, les 4 descripteurs de TU, `test_pico8_planet_kj_dispatch`, les appelants de `pico8_gk`/`ldown`/`gv`/`pico8_ui_jz`) | `trous-de-portage.md` | 7 |
| a2 | ≈ 20 : `pico8_struct.h`, `entity.c/.h`, `ui.c`, `economy_score.c/.h`, `economy_tiers.c`, `msgbox.c`, `particle.c`, `planet_display.c`, `planet_family_trade.c`, `planet_trade.c`, `slipway.c`, `ui_button.c`, `bg.c/.h`, `probe.c/.h`, `wormhole.c/.h`, `planet.c/.h`, `planet_family.c/.h`, `layout.c`, `scroll.c`, `dispatch_context.h`, `particle.h` | ≈ 30 (tous les appels `pico8_mv` restants, `methods[`, `PICO8_METHOD_*`) | `glossaire-noms.md`, `plan-3.md` | 25 |

- **Au-delà de ~40 chemins par chunk** : a1 ≈ 68, a2 ≈ 52 (l'implémenteur remesure). Le coupage plus fin ne réduit pas un commit : changer la forme de `pico8_struct_class_t` touche les 40 descripteurs (21 fichiers) et les 4 descripteurs de TU d'un coup, et chaque TU qui nomme une méthode de a1 ou le tableau `methods[]` doit suivre dans ce même commit (pas de pont). Rejetés : un commit préalable de format seul (initialiseurs désignés, 25 fichiers) — il ne retire rien et la ligne fixe deux chunks ; un chunk par méthode — chaque commit toucherait encore les 21 fichiers de descripteurs.
- **Pourquoi a1 avant a2** : les lanceurs de a1 (`dc`, `lt`/`he`, `jz`, `dg`) sont ceux qui dépendent de la présence et de la table d'états ; a2 n'a plus que des appelants directs et la suppression. La ligne fixe cet ordre.

### Effet sur `make data-residue`

| Catégorie (lignes / occ.) | Départ (`1c9aae0`) | Après a1 | Après a2 | Pourquoi |
|---|---|---|---|---|
| `dispatch` `src/` | 67 / 69 | 60 / 62 | **35 / 37** | exact : 7 puis 25 lignes (19 appels, définition et déclaration de `pico8_mv`, `entity.c:177`, `pico8_struct.h:89`, deux commentaires) |
| `dispatch` `tests/` | 124 / 124 | rapporté | **9 / 9** | 98 `pico8_mv` et 17 lignes de méthode ; restent les rappels `fq` 3, `js` 4, `mh` 1, `pj` 1 (H9) |
| `par-nom` `src/` | 433 | 425 | **420** | exact (sites de la section « Sites `par-nom` », lectures de présence, aides sans appelant) |
| `par-nom` `tests/` | 576 | rapporté | rapporté | lectures par nom de méthodes remplacées par des membres |
| `valeur` | 4961 / 5742 | baisse | baisse, rapportée | ≈ 143 lignes dans les corps d'adaptateurs, 20 prototypes publics, les arguments des 26 appels, `discard`, les TU ; hausses aux « Frontières » |
| `type-tag` | 352 / 560 | baisse | baisse | asserts de tag des aides `take` retirées, test de tag de `jo:fe` |
| `ref-audit` | 56 / 57 | = | 55 / 56 (H) | `PICO8_REF_AS_LIST_CONST` de `bd` |

« exact » est une égalité ; « rapporté » est mesuré au commit, écart expliqué.

### Hypothèses (rien n'a été compilé ni lancé)

- **H1** : GCC, avec `-std=c99 -Wall -Wextra -Werror`, n'émet pas `-Wmissing-field-initializers` pour un initialiseur désigné qui omet des membres, et l'émet pour les initialiseurs positionnels actuels une fois des membres ajoutés.
- **H2** : aucun code de `src/` hors des adaptateurs n'appelle `pico8_gk`, `pico8_bg_ldown/rdown/hq`, `pico8_ie_ldown`, `pico8_jo_ldown`, `pico8_js_ldown`, `pico8_fq_ldown`, `pico8_jo_gv`, `pico8_bv`, `pico8_mz_hq`, `pico8_jc_hq`, `pico8_jc_eb` ; seul `pico8_nr_ldown` appelle `pico8_nr_gv` (grep à `1c9aae0`).
- **H3** : les TU qui appellent ces fonctions sont `test_pico8_bg`, `test_pico8_entity_kind`, `test_pico8_fv`, `test_pico8_hp`, `test_pico8_js`, `test_pico8_mz`, `test_pico8_os`, `test_pico8_ui` (grep).
- **H4** : `ui:jz` est le seul lecteur des retours de `ldown`/`rdown`/`gv` dans le jeu ; aucune différence nil/`false` n'y est lue. Les e2e restent conformes.
- **H5** : `pico8_entity_get_field(qc, "t")` rend toujours le fixe `ke.t` pour une entité de `hv` ; la conversion de `dg` ne voit jamais un autre genre.
- **H6** : `me` ne prend que `ph`, `ly`, `ll`, `kz`, `pr`, `unknown`, `known` (étude §2.3).
- **H7** : `cg.jo` ne contient que des classes de l'arbre `jo` ; `jo.oe` et le `sw` de `kf` ne contiennent que des `jc` ; les `assert` de receveur ne se déclenchent ni dans les TU ni dans les e2e.
- **H8** : `js.hb` est posé avant que `lt` n'inscrive le bouton, et ne change plus ensuite.
- **H9** : les 9 lignes de rappel des TU ne nomment aucune méthode et restent telles quelles.
- **H10** : les constantes des TU qui plient un retour de `gv`/`ldown` (`test_pico8_fr` l. 439-456, `test_pico8_nr` l. 444-451) se reproduisent depuis `pico8_down_t` en pliant, à chaque site, le genre que rendait l'appelé (`nr:gv` : booléen ; `nr:ldown` refusé : `false` ; `jo:gv` refusé : nil) ; sinon, reprise avec sonde (« Constantes »).
- **H11** : `make test` recompile tout (en-tête commun `pico8_struct.h`) et reste de l'ordre de la minute ; `class-coverage-net` de l'ordre de quelques minutes.
- **H12** : aucun comportement du binaire de rejeu ne change : 26 e2e conformes, `class-coverage-net` identique.
- **H13** : les références nouvelles entre fichiers (`entity.c` → `pico8_qb_md`/`pico8_qb_struct_class` de `scroll.c` ; `economy_score.c`/`economy_tiers.c` → `pico8_jo_is_family`/`pico8_jo_gg`/`pico8_jo_ib` de `planet.c` ; `planet_trade.c`/`planet_family_trade.c` → `pico8_jc_eb` de `slipway.c` ; `bg.c`/`ui_button.c` → `pico8_hd_true` de `entity.c`) sont satisfaites par `ENGINE_SRCS` et `DATA_ENGINE_SRCS`, qui contiennent ces fichiers ; `test_pico8_layout_nil_item_draws_nothing` (liste courte) ne lie aucun fichier qui gagne une référence. Le `Makefile` ne change pas.

## Implementation Chunks

Deux commits, dans cet ordre, chacun vert seul (`make test`, `make e2e`, `make data-guard`, `make class-coverage-net`), chacun avec ses comptes, sa classification, ses lignes gagnées, ses frontières déclarées et ses mutations :

1. **a1 — méthodes lancées par `dc`, `lt`, `he`, `ui:jz`, `dg`.** Types (`pico8_down_t`, `pico8_state_t`, `pico8_state_of`, `pico8_state_fn`) et membres `kj`, `ev`, `hd`, `hk`, `hb`, `has_hb`, `ldown`, `rdown`, `gv`, `states[]` ; `methods[]` et `pico8_method_t` réduits aux 9 méthodes de a2 ; 40 + 4 descripteurs en initialiseurs désignés ; `lt`/`he` (présence par membre, `has_hb` de `js`), `dc` (`kj`/`ev`, `md` direct), `dg` (table d'états), `ui:gw`, `ui:bh`/`ui:jz` (sélecteur), `hy:hk`/`hy:lj` ; `pico8_hd_true` pour `bg`/`fq`, `pico8_js_hb_dispatch` ; fonctions retypées de a1 ; prédicats `LY` ; `jc:hq` lit `gv` sur le membre ; sites G `bg.c:268`, `ui.c:444`, `446` et aides `ui_take_held`/`ui_take_bg` ; TU qui nomment ces méthodes, `test_pico8_planet_kj_dispatch` ; TROU-11 sans site.
2. **a2 — le reste et la suppression.** Membres `hq`, `ey`, `kf`, `cn`, `ol`, `mm` ; `pico8_qy_t` étiqueté ; appels directs `eb`, `ib`, `gg` (`pico8_jo_is_family`), `lq`, `lh`, `my` ; fonctions retypées de a2 ; prédicats `HQ` ; sites G `bg.c:296`, `slipway.c:250`, E `entity.c:264`, aides `bg_take_vec`, `pico8_jc_mouse_point_take_ra` ; `pico8_mv`, `methods[]`, `pico8_method_t`, `pico8_method_of`, branche méthode de `pico8_entity_get_field` supprimés ; commentaires de `pico8_dispatch_context.h` et `economy_score.h` ; TU restants ; glossaire ; note « Découvert à D.10a » ; balayage ASan+UBSan des TU.

Ordre conseillé dans chaque commit : mesure de départ (lignes et occurrences par fichier) ; `pico8_struct.h` ; descripteurs ; lanceurs ; fonctions retypées ; TU ; `make test` ; `make e2e` ; `make class-coverage-net` ; `gcov` et mutations (copie jetable, restauration par `cp`) ; mesure d'arrivée, classification, lignes gagnées, greps de la « Sortie » ; docs ; message.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (valeurs, identités, registres, effets d'un appel), pas la présence d'une ligne. D.10a change le chemin d'appel, pas ce qui est appelé : **la preuve est que chaque TU garde son intention et sa constante, que les 26 e2e restent conformes et que `class-coverage-net` est identique**, et que chaque nouvelle ligne de `src/` est exécutée par un TU et mord sous mutation.

### TU touchés (G)

Règles, pour chaque TU qui nomme `pico8_mv`, `methods[`, `PICO8_METHOD_*`, `pico8_method_of`, un adaptateur par son ancienne signature, une méthode lue par `pico8_entity_get_field`, une fonction retypée, `pico8_ui_jz`, ou qui définit un descripteur :

1. **Appel d'une méthode** : par son membre (`e->cls->kj(e, &ra)`), la case d'état (`e->cls->states[PICO8_STATE_LY](e, t)`) ou la fonction directe (`pico8_jc_eb`, `pico8_jo_ib`, `pico8_jo_gg`, `pico8_qb_md`), avec des arguments typés ; les assertions sur le résultat lisent `{stop, held}` ou l'effet. Un appel qui passait un seul argument à `hd` (repli d'arité de TROU-11) passe le même pointeur deux fois.
2. **Présence, absence, identité** : `cls->X != NULL`, `== NULL`, `== pico8_hj_kj_dispatch` ; l'héritage (`nl`, `os`, `qh`…) compare membre par membre, explicitement. Pour `eb`, `ib`, `gg`, `md`, qui ne sont plus des données de classe : une assertion « la classe a `gg`/`ib` » devient `pico8_jo_is_family(e)` (vrai pour les dix classes de l'arbre, faux sinon) ; « `jc` a `eb` » et « `qb` a `md` » deviennent l'appel direct sur une instance, vérifié par son effet ; « la classe n'a pas `eb`/`md` » part, avec sa raison au message (la table ne porte plus ces noms).
3. **Pas de registre de TU** : aucun tableau, `switch` ni boucle qui passe d'un nom de méthode à un membre ; les aides `method(e, name)` et `call(e, name, …)` disparaissent, chaque site nomme son membre. Un tableau de noms qui mêlait champs et méthodes (`absent[]`, `unknown[]`, `names[]`) garde ses noms de champ (lecture par nom, D.11d) et perd ses noms de méthode, vérifiés sur les membres.
4. **Tests de `pico8_mv` lui-même** (rendre la valeur d'un champ non-fonction, rendre 0 sur un receveur NULL ou un nom absent, lire `p_` par `mv`) : tests de la couche, supprimés avec elle (§6, premier point), listés au message avec leur raison.
5. **Lecture d'une méthode comme champ** (`get(e, "kj").kind == PICO8_VALUE_FUNCTION`) : devient la présence du membre ; une assertion existante d'absence d'un champ reste. Au moins une assertion existante par classe touchée lit désormais « `pico8_entity_get_field(e, "<méthode>")` rend 0 » là où elle lisait une fonction — même objet (ce que le lecteur par nom rend pour ce nom), valeur attendue changée par la ligne ; listée au message.
6. **Descripteurs de TU** (`test_pico8_iv`, `test_pico8_js`, `test_pico8_mo`, `test_pico8_ui`) : initialiseurs désignés, mêmes membres. `test_pico8_planet_kj_dispatch` : `md` étant appelé directement sur un `mo` de classe `qb`, l'espion par copie de la table de `qb` ne peut plus s'interposer ; le test installe le vrai `qb` comme `mo` et observe l'effet de `md` (état de caméra/`gfx` qu'il pose) depuis l'espion de `kj`, posé sur une copie de la classe `jo` (même type, aucune conversion) : même intention (« `md` d'abord, puis le `kj` réel de la couche »).
7. **Constantes** : aucune `*_HASH` ni `identity[]` ne change. Un pli qui lisait une méthode par nom plie les mêmes octets depuis le membre (présence → `fnv(fnv(h, 1u), 0xFCu)`, absence → `fnv(h, 0u)`, selon le format de `fold_value` du TU). Un pli d'un retour de `gv`/`ldown` plie, à chaque site, le genre que l'appelé rendait (H10). **Seulement si** un octet ne peut pas être reproduit : la constante est reprise avec une **sonde**, déclarée au message : (i) dans une copie jetable de l'arbre parent du commit, remplacer le seul pli par sa forme nouvelle (même lecture, nouveau format) et noter la constante obtenue ; (ii) l'arbre du commit doit donner exactement cette constante ; toute autre différence est un échec.
8. **§6** : aucun des trois tests n'est touché.
9. **Test ajouté** : aucun fichier nouveau. Une assertion nouvelle n'est admise que pour couvrir une ligne nouvelle de `src/` que `gcov` montre non exécutée (par exemple `pico8_js_has_hb` sur un bouton sans `hb`, le cas `GV` du sélecteur, `pico8_jo_is_family` sur une classe hors de l'arbre), dans un TU existant de la classe, listée au message.
10. **Prior art** : les comparaisons d'identité `methods[HQ] == pico8_bv_dispatch` (prédicats, `test_pico8_g_` l. 727), `VIEW_*` de `test_pico8_hp` (octets gelés nommés), les boucles `bm` de `test_pico8_fr`/`test_pico8_hj` (appartenance à `cv`).

### Preuve des tests (§7 : `gcov` et mutations)

`gcov` (TU des classes touchées, compilés en `--coverage` dans une copie jetable du scratchpad) : chaque fonction d'emplacement, `pico8_hd_true`, `pico8_js_hb_dispatch`, le crochet `has_hb`, la fonction `bm` de `lt`/`he`, les trois cas du sélecteur de `jz`, le chemin d'état de `dg`, `pico8_state_of`, `pico8_jo_is_family`, chaque appel direct et sa garde. Une ligne non exécutée est couverte par une assertion (règle 9) ou justifiée au message.

Mutations, **une à la fois, dans l'arbre de travail, fichier sauvegardé par `cp` avant et restauré par `cp` après** (jamais `git checkout`/`stash`), chacune rougissant au moins un TU (nommé au message) :

- a1 — **M1** `has_hb` ignoré par `lt` (tous les `js` dans `cv.hb`) → un TU de `js` (pli `bm`/constante) rouge ; **M2** `pico8_state_of` échange `ly` et `ll` → TU `nl`/`qh`/`jo` rouge ; **M3** `pico8_hd_true` rend 0 → TU `bg`/`fq` (survol, `ui:gw`) rouge ; **M4** `jo:ldown` « known » rend `{0, NULL}` → TU `jo`/`ui` rouge ; **M5** garde de `md` inversée → `test_pico8_planet_kj_dispatch` rouge ; **M6** `dc` passe `NULL` pour `ra` à `kj` → un TU de dessin (`jo`, `mz`, `js`) rouge ; **M7** `jz` : `RDOWN` lance `ldown` → TU `bg`/`ui` rouge.
- a2 — **M8** `pico8_jc_eb` rend toujours `qu` → TU `jc`/`trade` rouge ; **M9** `ol` de `lq` passe `a = 0` → TU `lq`/`jo` rouge ; **M10** `ez` n'ajoute pas la particule → TU `g_`/`hj`/`ga` rouge ; **M11** membre `cn` de `nr` = `pico8_jo_cn_dispatch` → TU `nr`/`trade` rouge ; **M12** `jo:fe` appelle `pico8_hp_lq` même quand `b->lq` est posé → TU `hp`/`jo` rouge ; **M13** `bg:hq` ne garde plus `rp` dans `mq` → TU `bg` rouge.
- Une mutation qui ne rougit rien est un trou : couvert par une assertion (règle 9) ou justifié au message.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit ; aucun e2e ajouté (le chemin changé est exercé par les 26 e2e, qui passent tous par `dc`, `dg`, `ui:gw`/`ui:bh` ; un e2e nouveau n'ajouterait de couverture qu'à des lignes que les TU couvrent déjà).
- `make class-coverage-net` à chaque commit : identique à `tools/class_coverage_net_expected.txt` (la ligne dit « oui » ; chaque constructeur pose une classe dont la forme change).
- `make sanitize-net` : non (aucune durée de vie ni propriétaire ne change).
- **Balayage ASan+UBSan des TU** à la fin de a2 : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), comme à D.7b ; sortie propre. Il n'est pas refait à a1 (consigne : à la fin).

### Vérification, budget et tableau « Runs »

**Budget : environ 120 minutes pour l'étape** (a1 ≈ 70, a2 ≈ 50), mesures et filets compris ; la relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make test` | de l'ordre de la minute (recompilation complète : `pico8_struct.h` change) |
| `make e2e` | ~1 s |
| `make class-coverage-net` | quelques minutes, deux fois |
| `make data-residue`, recette, classification, greps | < 1 min |
| `gcov` des TU touchés | quelques minutes par chunk |
| mutations (13, chacune un `make` ciblé du TU) | < 10 min au total |
| balayage ASan+UBSan des TU | quelques minutes, une fois |

Au-delà du budget, l'implémenteur s'arrête et note ce qui n'a pas été lancé. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement : `#`, commande exacte, état de l'arbre (HEAD + chunk + étape), code de sortie, ligne clé, durée. Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que des chemins de `src/`, `tests/` et, à a1, `docs/trous-de-portage.md`, à a2, `docs/glossaire-noms.md` et `docs/plan-3.md` ; ni `tools/`, ni `tests/e2e/`, ni `*.sha256` ; le `Makefile` seulement si une liste de lien doit gagner un fichier (H13), entrée par entrée, déclarée.
2. **Greps** de la « Sortie » : exactement ce qui y est écrit (après a2) ; après a1, la grep 1 ne rend plus que les 9 noms de a2 et `pico8_mv`.
3. **Types** : aucune conversion de pointeur de fonction (greps 3) ; chaque fonction d'emplacement a exactement la signature de son membre ; aucun `pico8_value_t` dans une signature d'emplacement.
4. **Registres** : présence identique (les TU qui plient `cv`/`hv` gardent leurs constantes) ; `js` sans `hb` hors de `cv.hb`.
5. **Constantes** : aucune `*_HASH`/`identity[]` changée, ou changée avec sa sonde déclarée et reproductible.
6. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (c) = 0 ; lignes gagnées listées ; conversions limitées à la liste « Frontières » ; registres limités aux trois déclarés.
7. **Sites G** : les 5 retirés, les 4 laissés intacts et nommés avec leur étape.
8. **Mutations**, `gcov`, `class-coverage-net`, balayage ASan+UBSan rapportés.
9. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.10a ni au plan dans `src/` ou `tests/`.
10. **Docs** : note « Découvert à D.10a » en fin de ligne D.10a ; TROU-11 et la ligne `mv` du glossaire comme décrit ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `1c9aae0` (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas `dispatch` 191 (`src/` 67) et `par-nom` 1009 sans explication.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change, ou si un test devrait changer d'intention pour passer.
- `FAILED:` si `make class-coverage-net` diffère de l'attendu versionné à l'un des deux commits, ou si le balayage ASan+UBSan des TU n'est pas propre.
- `FAILED:` si un champ ou un site ne tombe sous aucune règle (arrêt « champ sans règle » du §7), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.

**Périmètre et types**

- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si un pointeur de fonction est converti vers un autre type, ou si une fonction d'emplacement n'a pas exactement la signature de son membre.
- `FAILED:` si `pico8_mv` (ou un équivalent qui résout une méthode par son nom) survit, en `src/` ou en TU, ou si un pont compté est ajouté.
- `FAILED:` si un js sans `hb` est inscrit dans `cv.hb`, ou si l'ordre ou le contenu de `hv`/`cv`/`cg` change.
- `FAILED:` si un rappel (`my`, `hb`, `lh`, `lq`, `eu`, `init`, rangées de `menu`, `jo:fe`, `de`) change de type : c'est D.10b.
- `FAILED:` si un fichier de `tools/` change, ou si le `Makefile` change ailleurs qu'à une liste de lien qui doit gagner un fichier (H13, déclarée).

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert.
- `FAILED:` si `dispatch` `src/` ne vaut pas 60 lignes après a1 et **35 lignes / 37 occurrences** après a2, si les 35 lignes ne sont pas les rappels listés, ou si `dispatch` `tests/` ne vaut pas 9 après a2 sans écart expliqué (H9).
- `FAILED:` si `par-nom` `src/` ne vaut pas 425 après a1 et 420 après a2 sans écart expliqué, ou si un des quatre sites G laissés disparaît par un déballage déplacé.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, construction déplacée, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, alias, registre hors des trois déclarés, conversion typée ↔ valeur hors de la liste « Frontières », code déplacé hors du compteur, ou une (c).
- `FAILED:` si le **message d'un commit** ne donne pas, par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences**, la classification (a)/(b)/(c) des lignes `dispatch` et `par-nom` retirées (chaque (c) déclarée et justifiée, attendu 0), la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris), la justification de chaque couple Δlignes > Δoccurrences, la liste des conversions de frontière ; un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change sans sa sonde déclarée et reproductible.
- `FAILED:` si un test supprimé n'est pas listé au commit avec sa raison (tests de `pico8_mv`, absences de `eb`/`md` sur une classe).
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si le budget est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.10a », ou si elle omet les comptes réels, l'absence de pont et ce qu'elle change pour D.10b, D.10c, D.11a, D.11b et D.11c.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les rappels** (`js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, `g_ix_my`/`g_ix_hb`/`ha`, `jo:fe`, `de.my`, `eu`, `init`), leurs 19 lignes d'adaptateur, `PICO8_VALUE_FUNCTION`, `pico8_value_function()` et `pico8_value_function_t` : D.10b. Les 9 lignes de rappel des TU : D.10c.
- **Les autres lectures par nom** : classes A, B, C (hors `lt`/`he` `bm` et `gv` de `jc:hq`), D, F, K, M, les quatre sites G laissés, T1/T2 : D.11a-D.11e. Les membres `bg.hd`/`fq.hd` et leurs valeurs de départ.
- **Les listes** (`cv`, `hv`, `cg`, `qy`, `by`…) : D.12.
- **`valeur`, `type-tag`, `ref-audit`** au-delà de ce que D.10a retire en passant ; les retours `pico8_value_t` de `pico8_jo_gg` (TROU-3) et `pico8_d_` : Q12, ouverte.
- **`me` en enum** (Q3, option B rejetée), `gfx`/`mem`/`input` en arguments (Q4, rejeté).
- `tools/`, le `Makefile` (hors H13), `tests/e2e/`, la phase 2, `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **« 23 emplacements »** : 14 membres nommés + 5 cases d'états + 4 appels directs ; rejeté : 23 membres et une table en plus.
  - **signatures** : table « Signatures et implémentations » ; `pico8_down_t {stop, held}` pour `ldown`/`rdown`/`gv` ; `self` en `void *` ; `const` pour les vecteurs lus seulement ; aucune conversion de pointeur de fonction.
  - **noms** : les fonctions d'emplacement gardent le nom de l'adaptateur qu'elles remplacent ; rejeté : renommer.
  - **« 82 adaptateurs »** : 63 de méthode (+ 2 `mm`) traités ; 19 de rappel laissés à D.10b.
  - **présence de `js.hb`** : crochet `has_hb` ; rejetés : tout inscrire, seconde classe, lecture par nom.
  - **appels directs** : garde de classe par `assert` (`jc`, famille `jo` par `pico8_jo_is_family`), `md` gardé par `if` ; `lq`/`lh`/`my` appelés par leur membre.
  - **pont des TU** : aucun ; D.10c réduit aux rappels.
  - **sites G** : 5 retirés, 4 laissés avec leur étape ; rejeté : tenir « 9 » en déplaçant le déballage.
  - **lectures prises à D.11b** : présence `bm` de `lt`/`he`, `gv` de `jc:hq`.
  - **frontières** : conversions au seul site de la lecture par nom qui survit, listées ; interdites ailleurs.
  - **`ui:jz`** : sélecteur `pico8_jz_slot_t` ; **`dc`** : paramètre chaîne gardé, `assert` hors `kj`/`ev` (fin de TROU-11).
  - **constantes** : mêmes octets ; sonde seulement si un octet ne se reproduit pas.
  - **filets** : `class-coverage-net` à chaque commit, pas de `sanitize-net`, balayage ASan+UBSan des TU à la fin.
  - **découpage** : deux chunks (a1 ≈ 68 chemins, a2 ≈ 52), au-delà de ~40 parce que la forme de la table de classe et les TU sans pont ne se coupent pas.
- **Note du plan (G)**, à ajouter en fin de ligne D.10a (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.10a : deux chunks (a1 : N1 chemins ; a2 : N2). `pico8_struct_class_t` porte 14 membres typés (`kj`, `ev`, `hd`, `hk`, `hb`, `ldown`, `rdown`, `gv`, `hq`, `ey`, `kf`, `cn`, `ol`, `mm`), la table `states[]` (`pico8_state_t`, `pico8_state_of`) et le crochet `has_hb` (présence de `hb` par instance, pour `js`) ; `ldown`/`rdown`/`gv` rendent `pico8_down_t {stop, held}` ; `eb`, `ib`, `gg`, `md` sont appelés directement (`pico8_jo_is_family`) ; `hd=1` est `pico8_hd_true` ; `pico8_ui_jz` prend un sélecteur. Pas de pont : les TU ont suivi (98 lignes `pico8_mv`, 17 lignes `pico8_value_function` de méthode) ; D.10c ne garde que les rappels des TU (9 lignes, plus ce que D.10b y laisse). D.10b hérite des 19 lignes d'adaptateurs de rappel (dont `pico8_hp_lq_dispatch`, `pico8_jo_fe_my_dispatch`) et des membres `my`/`hb`/`lh`/`lq`, appelés directement depuis D.10a ; le crochet `has_hb` survit au typage de `js.hb`. Sites G laissés : `ui_button.c:599` (D.10b), `slipway.c:261` et `research_icon.c:182` (D.11a), `ke.c:43` (D.11c) ; pris à D.11b (classe C) : la présence des noms `bm` dans `lt`/`he` et de `gv` dans `jc:hq`. TROU-11 sans site. `dispatch` : `src/` 35, `tests/` T ; `par-nom` P ; `valeur` V.

  N1, N2, T, P, V sont les valeurs réelles.
- **Le message de chaque commit** porte : le périmètre du chunk ; la classification (a)/(b)/(c) des lignes `dispatch` et `par-nom` retirées ; les comptes avant et après **en lignes et en occurrences** par catégorie et par fichier, avec les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie, par fichier (« aucune » compris) ; les conversions de frontière, site par site ; les registres déclarés ; les tests supprimés et leur raison, les assertions ajoutées et la ligne qu'elles couvrent ; les constantes (toutes inchangées, ou la sonde) ; les mutations ; `gcov` ; les filets ; le tableau « Runs » résumé ; les réponses aux hypothèses H qui le concernent ; ce que le jeu ne lit plus (`bg.hd`, `fq.hd`, `pico8_hp_lq_dispatch` par `mv`).
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget) : types, anti-tassement, classification, frontières, registres, constantes.
- **Surprises pour le pilote** :
  - **« 23 emplacements nommés » et « table d'états » se contredisent** si on les lit ensemble : cinq des 23 méthodes sont les états. Et « appel direct » pour quatre autres retire leur entrée (étude §2.2, « plus de table »).
  - **« 82 adaptateurs » compte 19 lignes de rappel**, qui sont à D.10b (ils ont le type que D.10b retire).
  - **Un pont `pico8_mv` est impossible sans registre** (traduire chaque nom en membre et chaque valeur en type), et ne dispenserait pas 27 TU de changer : D.10c est vidé de ses 98 lignes `pico8_mv`.
  - **`js.hb` est une présence par instance** : sans le crochet, `lt` inscrirait tous les boutons dans `cv.hb`.
  - **Quatre sites G sur neuf ne déballent pas un argument de méthode** : `mt` et `g_ie_gy` sont lus par nom ou posés comme valeur, `me` de `ks` vient d'une lecture de classe M, le résultat de `my` est un rappel.
  - **`jc:hq` lit la présence de `gv` par `pico8_entity_get_field`** : sans conversion à a1, le survol d'une planète pendant le tracé d'un slipway cesserait de la viser.
  - **L'emplacement `ol` de `jb` n'est atteint par aucun lanceur** (`jo.lq` est un `pico8_lq_t *`) ; il garde ce que l'adaptateur faisait d'un vecteur (`om` = 0), et `test_pico8_jb_ol` appelle `pico8_jb_ol` directement.
  - **Des constantes de TU plient la différence nil/`false` des retours de `gv`/`ldown`** (`fr`, `nr`), que `{stop, held}` ne porte plus : les mêmes octets se reproduisent par site (H10) ; sinon sonde.
  - **`test_pico8_planet_kj_dispatch` espionnait `md` par une copie de la table de `qb`** : l'appel direct la contourne ; le test change de moyen, pas d'intention.
