---
title: "Plan 3, étape D.12a : catégorie `liste` au compteur (premier commit seul) ; conteneur de pointeurs `pico8_ptrs_t` (Q9) ; registres L1 (`hv`, `cv[5]`, `cg[*]`, calques de `dc`, `ui.ic`, `ui.le`) et leurs parcours sur ce conteneur, `mz.ox` et `pico8_ij` avec eux ; `g_ga_by_shared` reporté à D.12b avec `jo.by` ; les TU suivent dans le même commit ; deux sous-étapes"
description: "Aucune catégorie du compteur ne voit `pico8_list_t`, et les registres d'entités (`hv`, `cv`, `cg`, calques de `dc`, `ui.ic`, `ui.le`) tiennent des `pico8_value_t` REF qu'on déballe à chaque lecture. D.12a ajoute la catégorie `liste` (Q10) dans un commit seul, crée le conteneur de pointeurs `pico8_ptrs_t` (Q9 : `void *`, `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé) et y fait passer les registres L1 et leurs parcours de `src/`, avec les 34 TU qui les lisent : `liste` passe d'environ 826 (après D.11e, hypothèse) à environ 600, mesuré et rapporté."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.11e est implémentée** (`plan-pilot`, étape 2b), sur l'arbre **commis** `de6351e` (« docs: PRD de D.11e revérifié après D.11d et mis en file, trois sous-étapes »), extrait par `git archive de6351e` dans le scratchpad. D.11e n'est pas gradée : tout ce que ce PRD suppose de son issue est une hypothèse `H-D11e-n` (table plus bas), à confronter à sa note « Découvert à D.11e » avant de passer ce PRD dans `docs/prd/triage/`. Seuls `grep`, `sed`, `awk`, `sh tools/data_residue.sh` (et une copie de ce script, dans le scratchpad, augmentée du motif de Q10 pour mesurer `liste`) ont tourné. **Rien n'a été compilé ni lancé.** Les numéros de ligne sont ceux de `de6351e` ; ceux que D.11e déplace sont marqués ⟲.

Le problème, vu du mainteneur du port : la phase D ne peut sortir que si `pico8_value.c` disparaît, et `pico8_list_t` (un tableau de `pico8_value_t`) le tient en vie. Les registres d'entités sont le cœur de cette dépendance : chaque entité inscrite par `lt` entre dans `hv`, dans les seaux de `cv` et dans ceux de `cg` sous la forme d'une `pico8_value_t` REF, et chaque parcours (`dg`, `dc`, `hy:hk`, `hy:lj`, `mz:pq`, `jc:ce`, `bv`, `ui:gw`…) la déballe par `.as.ref` en testant un genre qui ne peut valoir que REF. De plus, aucune catégorie du compteur ne compte `pico8_list_t` : les étapes D.12 n'ont pas de « Compte » tant que Q10 n'est pas appliquée.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `de6351e` (§5, « Phase D — Données », l. 277) :

> - D.12a — `liste`, registres : catégorie `liste` ajoutée à `tools/data_residue.sh` dans un premier commit seul, motif `\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h` (Q10 ; la règle de D.0 fige les motifs existants, mais une catégorie nouvelle s'ajoute dans son propre commit avec son motif énoncé) ; conteneur de pointeurs (Q9) sous un nom hors de ce motif ; registres L1 (`hv`, `cv[5]`, `cg[*]`, calques de `dc`, `ui.ic`, `ui.le`, `g_ga_by_shared`) et leurs parcours. Empreinte ≈ 15 (15, 0) : `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud`. Compte : `liste` ≤ 927 moins L1, rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.

Décisions de D.9, recopiées (l. 265) :

> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Paragraphe « Liste de D », empreinte et règles communes, ordre, recopiés (l. 237, 239, 241) :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.

> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Comptes au départ de D.10 et règle des ponts de TU, recopiés (l. 267) :

> Comptes au départ de D.10 (HEAD 5a13008, `make -s data-residue` et `--detail` ; l'étude mesurait `2dec71f`, avant D.7a–D.8) : `valeur` 4961/126, `type-tag` 352/65, `ref-audit` 56/15, `par-nom` 1009/81 (`src/` 433/43, `tests/` 576/38), `dispatch` 191/61 (`src/` 67/30, `tests/` 124/31), résidu D 5868 lignes / 128 fichiers ; `liste`, pas encore au compteur, 927/99 avec le motif de Q10 (`src/` 496/61, `tests/` 431/38). Les classes A–M, T1/T2 et L1–L4 sont celles de l'étude (annexe A) ; ses « ≈ » viennent de son classement, chaque PRD remesure. Un TU qui nomme un symbole retiré par une étape de `src/` la suit dans le même commit, ou le symbole reste comme pont compté, fermé par l'étape TU de son groupe (D.10c, D.11d, D.12d) ; le PRD tranche. `sanitize-net` : aux seules étapes qui changent une durée de vie ou un propriétaire (§7).

Toutes les autres lignes de la phase D (D.0 à D.12d) **avec toutes leurs notes « Découvert à D.x »** (D.0, D.1, D.7c, D.8, D.10a, D.10b, D.10c, D.11a, D.11b, D.11c, D.11d1 à D.11d4, et la décision humaine qui découpe D.11d), l'arrêt « champ sans règle », les parties du §5 qui fixent les règles (en-tête et phase 1a, phase 1b, phase D en entier, phases 2 et 3), le §6 et le §7 entiers sont recopiés tels quels, par extraction de lignes, en **Annexe A**. Les passages de l'étude D.9 sur `pico8_list_t` (§3.1, §3.2, lignes D.12a–D.12d de sa table d'étapes, Q9, Q10) sont en **Annexe B**. L'**Annexe C** est l'inventaire ligne à ligne des lignes `liste` des fichiers de `src/` que l'étape touche, classées L1 ou non ; l'**Annexe D**, celui des lectures de registres dans les TU.

### Hypothèses `H-D11e-n` (D.11e en cours, à confronter à sa note)

Source : le PRD de D.11e (`docs/prd/triage/plan3-phaseD-etape11e-suppression-infrastructure-par-nom.md` à `de6351e`), sa ligne du plan (trois sous-étapes e1..e3). Chaque hypothèse dit ce qui en dépend ici.

| Hypothèse | Énoncé | Ce qui en dépend dans ce PRD | Vérification à la graduation |
|---|---|---|---|
| H-D11e-1 | `par-nom` = 0 dans `src/` et `tests/`, gardé : `DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom` ; `make data-guard` vert | « État de départ » ; critère « aucune lecture par nom réintroduite » (gardé par `make test`, rien à ajouter ici) | `make -s data-residue` ; ligne `DATA_RESIDUE_GUARD` du `Makefile` |
| H-D11e-2 | `pico8_struct_class_t` a perdu `field`/`set_field` et gagné les onze accesseurs de Q6 (`get_nu`, `get_fa`, `get_ra`, `get_p`, `get_kq`, `get_ql`, `get_fg`, `get_mt`, `get_fx`, `get_p_`, `set_m_`) ; les parcours que D.12a réécrit les lisent déjà : `lt`/`he` (`get_nu`), `dc` (`get_fa`, `get_ra`), `dg` (`get_p_`), `ui:gw` (`get_fa`, `set_m_`), prédicat de `mz.ox` (`get_fx`), `pico8_ij` (`get_p`) | Corps des boucles de l'« Inventaire » : D.12a ne change que la forme du parcours (élément `void *` au lieu de `pico8_value_t`), jamais la lecture de champ qui est dedans | lecture de `pico8_struct.h` et des sept fonctions |
| H-D11e-3 | `liste` après D.11e = **826 lignes / 99 fichiers** (`src/` 477 / 60, `tests/` 349 / 39), **870 occurrences** : D.11e ne retire que les 7 lignes `liste` de l'infrastructure par nom (`planet.c:137`, `:143`, `:218` ; `planet_family.c:495`, `:501` ; `probe.c:594`, `:600` : `jo_take_list`, `jo_field_list`, `family_take_list`, `g__take_list`) et n'en ajoute aucune | « État de départ », « Effet sur `make data-residue` », critères `FAILED:` de départ | copie de `data_residue.sh` avec le motif de Q10 (recette de « État de départ ») |
| H-D11e-4 | Plus aucun lecteur ni écrivain ne rend `ui.le`, `ui.ic`, `mz.ox` ni un registre comme `pico8_value_t` REF (`ui_ref_field(ui->le)`, `ui_ref_field(ui->ic)`, `ui.c:119`, `:128` ; `mz_ref_field(mz->ox)`, `wormhole.c:305` partis) ; rien n'applique donc `pico8_type_tag_of` à ces conteneurs | Décision « conteneur sans étiquette de type » | `grep -nE '(->|\.)(le|ic|ox)\b' src/*.c` ; `grep -n 'type_tag_of' src/*.c` sur ces conteneurs |
| H-D11e-5 | D.11e ne change ni le type des registres, ni `pico8_list.*`, ni `pico8_collect.*`, ni `galaxy.c`, `economy_formulas.c`, `economy_score.c`, `economy_tiers.c`, `entity_geom.h`, ni les aides de registre des TU (`listed`, `rank_in`, `count_of`, `list_count`, `list_size`, `size_of`), ni les listes de sources du `Makefile`, ni `tools/` | Numéros non marqués ⟲ de ce PRD ; décision « les TU suivent » ; édition du `Makefile` et de `tools/data_residue.sh` telle qu'écrite | `git diff --stat de6351e <HEAD gradé> -- <ces chemins>` vide |
| H-D11e-6 | D.11e déplace des lignes dans : `entity.c` (`:38-47`, `:198-220` supprimées ; `:237-238`, `:266-267`, `:300`, `:329` réécrites), `entity.h` (`:45-54`), `bootstrap.c` (`:19`, `:87`), `bg.c`, `economy.c`/`.h`, `probe.c`/`.h`, `score_panel.c`, `slipway.c`/`.h`, `wormhole.c`/`.h`, `ui.c`/`.h`, `msgbox.c`, `entity_geom.c` (`:19-27`, `:64`, `:73-82`, `:104`), et dans les TU `iv`, `mo`, `ui`, `g_`, `mz`, `jc`, `ie` | Numéros marqués ⟲ (remesurés par contenu, jamais recopiés) | `git diff de6351e <HEAD gradé> --stat` |
| H-D11e-7 | Les structs-vues sont typées à descripteur vide ; `pico8_bv_et_t` (`probe.h`), `pico8_jc_mouse_point_t` (`slipway.h`) sont publics, `pico8_ie_gy()`/`pico8_ie_id()` existent ; aucune vue n'est inscrite par `lt` (seul un `ke` entre dans `hv`) | Aucun effet attendu : les registres ne tiennent que des `ke` | lecture de la note |
| H-D11e-8 | `pico8_ij` lit `p` par `get_p` (NULL → zéro), garde `pico8_value_t *out_nearest` et son `assert` « ij walks a list of entities » ; le prédicat de `mz.ox` garde sa signature `(pico8_value_t e, void *ctx)` | Décisions « `pico8_ij` converti en place », « `pico8_jp_ptrs` » | lecture de `entity_geom.c`, `wormhole.c` |
| H-D11e-9 | `make e2e` : 26 conformes ; `tools/class_coverage_net_expected.txt` inchangé ; aucun TU ajouté ni supprimé par D.11e | Filets ; liste des 34 TU lecteurs de registres | `make e2e`, `git diff --stat` |
| H-D11e-10 | `test_pico8_ui` n'a plus l'oracle accesseurs = lecteurs (né en e1, mort en e2) ; ses lectures de `hv`/`cv`/`cg`/`ic`/`le` sont celles de `de6351e`, numéros changés | Inventaire des TU (Annexe D), « TU touchés » | `grep` de l'Annexe D sur `test_pico8_ui.c` |
| H-D11e-11 | La ligne D.12a du plan n'est pas modifiée par la note de D.11e (qui dit ce que D.12 trouve : `ij` et `mz.ox` sur listes, `bv_et`, `jc_too_far` en cibles `et`) | « Lecture de ces textes », décision `mz.ox` | lecture du plan gradé |

### Sections à revérifier quand D.11e est gradée

- **Toujours** : « État de départ » (comptes réels, H-D11e-1, -3) ; critères `FAILED:` de départ ; Annexe A (ligne D.11e et sa note « Découvert à D.11e », à recopier ; ligne D.12a si la note l'a modifiée, H-D11e-11).
- **Si H-D11e-6 tient** (seuls les fichiers listés bougent) : remesurer par contenu les numéros ⟲ de l'« Inventaire », des « Mutations » et des Annexes C et D ; rien d'autre.
- **Si H-D11e-2 ou H-D11e-8 tombe** (forme des accesseurs, `pico8_ij`, prédicat de `mz.ox`) : « Aides communes », « Parcours », mutations M10, M12, M14.
- **Si H-D11e-4 tombe** (un lecteur rend encore un registre comme REF) : décision « conteneur sans étiquette » : rouvrir, voir son « Rejeté ».
- **Si H-D11e-5 tombe** : « Makefile et liens », « TU touchés », Annexe D.
- **Si H-D11e-9 ou -10 tombe** : « Empreinte et découpage », « TU touchés ».

### Lecture de ces textes pour D.12a

- **« catégorie `liste` ajoutée à `tools/data_residue.sh` dans un premier commit seul, motif … »** : le premier commit de l'étape ne contient que la catégorie : `CATEGORIES` gagne `liste` en dernier, `pattern()` gagne un bras `liste` au motif **recopié tel quel**, et `tests/test_data_residue.c` gagne les cas qui jugent cette catégorie (littéraux coupés comme les cas existants, pour qu'il n'ajoute aucune ligne à aucune catégorie). Aucun motif existant ne change (règle de D.0 : « Motifs figés ») ; le message énonce le motif et les comptes que la catégorie révèle.
- **« conteneur de pointeurs (Q9) sous un nom hors de ce motif »** : `pico8_ptrs_t`, `src/pico8_ptrs.c/.h`, opérations `pico8_ptrs_*`. Ce n'est pas un renommage de `pico8_list_t` : l'élément change de type (`void *` au lieu de `pico8_value_t`), `del` compare des pointeurs, il n'y a pas d'étiquette de type ; `pico8_list_t` reste pour ses autres usagers jusqu'à D.12d. Voir « Conteneur ».
- **« registres L1 (`hv`, `cv[5]`, `cg[*]`, calques de `dc`, `ui.ic`, `ui.le`, `g_ga_by_shared`) et leurs parcours »** : remesuré à `de6351e`, six des sept passent. **`g_ga_by_shared` ne peut pas passer seul** : c'est le défaut partagé vers lequel pointe `jo.by` de chaque `ga` (`planet_family.c:25`, `:914`, `:957` ⟲) ; son type est celui du membre `jo.by` (liste des routes, L2, `trade`), que D.12b convertit. Il est **reporté à D.12b**, déclaré (écart à la ligne). Inversement, **`mz.ox` (L2 dans l'étude) entre dans D.12a** : son seul producteur est le filtre de `hv` (`wormhole.c:87`, `jp(hv, …)`), et son seul lecteur de `src/`, `pico8_ij` (`wormhole.c:104`), est aussi appelé sur `cg("jo")` et `cg("np")` : `hv`, `cg`, `mz.ox` et `pico8_ij` ne peuvent changer que dans le même commit sans cale. Déclaré.
- **« leurs parcours »** : chaque fonction de `src/` qui reçoit un registre et en lit les éléments (table « Parcours »), plus les aides de `collect` et `entity_geom` qu'elle appelle **sur ce registre** : `pico8_jp` (filtre, 5 sites sur des registres), `pico8_la` (concaténation, 1 site), `pico8_gu` (appartenance, 1 site sur `ui.le`), `pico8_nv` (application, 1 site sur `ui.le`), `pico8_ij` (plus proche, 3 sites). Les appels de ces aides sur des listes L2/L3/L4 (`jy`, `items` locales, `t_rl`, `invented`, `lq.items`, `hp.i`) restent à D.12b/D.12c. D'où quatre aides nouvelles sur le conteneur (`pico8_jp_ptrs`, `pico8_la_ptrs`, `pico8_gu_ptrs`, `pico8_nv_ptrs`) et `pico8_ij` converti en place. Voir « Aides communes ».
- **« Empreinte ≈ 15 (15, 0) : `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud` »** : estimation de l'étude à `2dec71f`. Remesurée à `de6351e` : **22 chemins de `src/`** (dont 2 nouveaux) et **35 TU** (34 lecteurs de registres + `test_pico8_ptrs` nouveau), plus `tests/test_data_residue.c`, `tools/data_residue.sh`, le `Makefile`, le plan. `cart.c` ne lit aucun registre (seulement un `#include "pico8_list.h"` sans usage : hors L1, laissé à D.12c/D.12d) ; `galaxy.c` n'appelle que `pico8_ij(p_vec, pico8_cg("jo"), &nearest)`, dont le texte ne change pas ; `hud.c` ne nomme aucune liste. S'ajoutent : `probe.c`, `score_panel.c`, `slipway.c`, `wormhole.c`/`.h` (parcours de `cg`/`hv`), `msgbox.c` (construction de `ui.le`), `collect.c`/`.h`, `entity_geom.c`/`.h` (aides). Le « 0 » de TU tombe : **34 TU lisent les registres** (Annexe D), voir « Les TU suivent ».
- **« Compte : `liste` ≤ 927 moins L1, rapporté »** : 927 était le compte au départ de D.10 ; D.11d a retiré des lignes de TU (431 → 349) : **833 à `de6351e`**, 826 attendu après D.11e (H-D11e-3). Compte de l'étape : **`liste` ≤ 826 − (lignes L1 retirées), rapporté**, estimé ≈ 600 (« Effet sur `make data-residue` »).
- **« §6 : aucun »** : faux par lecture, déclaré. Deux tests de régression du §6 lisent `ok.le` (`test_pico8_ga_export_panel_by.c:34-38`, `:100`, `:125-127`, `:150-152` ; `test_pico8_planet_gh_row_lq.c:43-47`, `:105`, `:137-140`) et suivent `ui.le` **dans leur façon de lire l'état seulement** (type du pointeur, élément `void *`), intention et constantes inchangées ; ce n'est pas l'arrêt du §7 (« dans son intention »). Le troisième, `layout_nil_item_draws_nothing`, ne change pas : seule sa ligne de lien du `Makefile` gagne `src/pico8_ptrs.c` (`pico8_collect.c` en dépendra).
- **« `class-coverage-net` : seulement si le diff touche un constructeur »** : D.12a1 ne touche aucun constructeur (à constater sur le diff) ; D.12a2 touche `pico8_ui_struct_new` (initialisation de `ic_own.layers`, `ui.c:163-166` ⟲) : **oui** en a2.
- **« `sanitize-net` : oui »** : à chaque sous-étape (stockage des registres, tas de `ui.le`, `mz.ox`, listes filtrées temporaires : autant de durées de vie qui changent de représentation).
- **Règle des ponts de TU (l. 267)** : tranchée ici pour **le premier terme** : les TU suivent dans le même commit. Le « pont compté » supposerait que `pico8_hv()` et consorts rendent encore une `pico8_list_t` aux TU : soit un double tenu à jour en parallèle du vrai registre (les TU testeraient la copie, pas le registre que `src/` parcourt), soit un instantané reconstruit à chaque appel (cale valeur ↔ typé ; un pointeur gardé par un TU deviendrait périmé en silence). Les deux sont interdits (consignes). Voir « Les TU suivent ».
- **Leçon de D.11d** (des TU lisent ce que `src/` n'expose pas typé) : ici l'inverse, **des TU lisent directement les registres** (`pico8_hv()->count`, `->items[i].as.ref`, `ok->le`, `ok->ic->layers[fa]`) par des aides locales (`listed`, `rank_in`, `count_of`, `list_count`, `size_of`, `ok_le`, `le_of`). Recensé fichier par fichier (Annexe D) ; les constantes de hachage qui plient `rank_in(pico8_hv(), …)` restent égales parce que l'ordre d'inscription ne change pas.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json`, rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes, alias, registre (tableau, `switch` ou boucle qui passe d'un nom à un membre), renommage pour échapper aux greps, cast non vérifié, cale permanente ; mutations restaurées par `cp` simple ; binaires lancés exactement comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan) ; sous-étapes d'environ 50 min ; jamais l'appli raylib.

### État de départ

**Mesuré sur `de6351e`** (arbre archivé, D.11e en cours), sans build :

| Catégorie | Lignes | Fichiers | `src/` (lignes / fichiers) | `tests/` (lignes / fichiers) |
|---|---|---|---|---|
| `valeur` | 1833 | 123 | rapporté | rapporté |
| `type-tag` | 293 | 63 | rapporté | rapporté |
| `ref-audit` | 42 | 14 | 42 / 14 | 0 |
| `par-nom` | 303 | 38 | 292 / 31 | 11 / 7 |
| **résidu D** (9 catégories) | **2336** | **124** | | |
| `liste` (motif de Q10, pas encore au compteur) | **833** (877 occ.) | **99** | **484** (524 occ.) / 60 | **349** (353 occ.) / 39 |
| résidu D avec `liste` | **3019** | **124** | 1896 / 82 | 1123 / 42 |

**Départ attendu de D.12a** (après D.11e, hypothèses) : `par-nom` 0 (H-D11e-1) ; `liste` **826 / 99** (`src/` 477 / 60, `tests/` 349 / 39 ; 870 occurrences, H-D11e-3) ; `valeur`, `type-tag` en baisse, rapportés ; `DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom`.

Répartition des jetons `liste` à `de6351e` (occurrences, `src/` / `tests/`) : `pico8_list_t` 210 / 210, `pico8_list_add` 59 / 47, `pico8_list_init` 51 / 21, `pico8_list_free` 43 / 11, `pico8_list.h` 33 / 37, `pico8_list_iter_t` 32 / 4, `pico8_list_all` 29 / 4, `pico8_list_iter_next` 28 / 4, `PICO8_TYPE_TAG_LIST` 12 / 10, `pico8_list_del` 9 / 4, `PICO8_REF_AS_LIST` 5 / 0, `pico8_list_foreach` 4 / 1, `PICO8_REF_AS_LIST_CONST` 4 / 0, `pico8_list_foreach_fn` 3 / 0, `pico8_list_grow` 2 / 0.

Recette (réutilisée à chaque mesure ; `$S` : dossier du scratchpad ; avant le premier commit, `pattern liste` n'existe pas encore dans le script : on l'écrit à la main) :

```sh
P='\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'liste\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
# après le premier commit : sh tools/data_residue.sh --detail | grep -E '^liste'
```

### Inventaire (mesuré sur `de6351e` ; ⟲ = fichier que D.11e modifie, numéros à remesurer par contenu)

#### Registres et leur stockage (`src/pico8_entity.c` ⟲, `pico8_entity.h` ⟲, `pico8_ui.c` ⟲, `pico8_ui.h` ⟲)

| Registre | Stockage (fichier:ligne) | Écrit par | Accesseur public | Sous-étape |
|---|---|---|---|---|
| `hv` | `static pico8_list_t g_hv;` (`entity.c:34`) ; init `:136`, libération `:109` | `pico8_lt` `:227` (ajout), `pico8_he` `:256` (`del`, TROU-5) | `const pico8_list_t *pico8_hv(void)` (`entity.c:148`, `entity.h:27`) | a2 |
| `cv[5]` | `pico8_cv_entry_t { name; pico8_list_t list; }` (`entity.c:15-20`) ; init `:140`, libération `:112` | `pico8_lt` `:231`, `pico8_he` `:260`, par `pico8_bm_has` | `const pico8_list_t *pico8_cv(const char *)` (`entity.c:152`, `entity.h:33`) | a2 |
| `cg[*]` | `pico8_cg_entry_t { tag; pico8_list_t list; }`, tableau `realloc` (`entity.c:25-32`) ; création `:95`, libération `:117` | `pico8_lt` `:241` (tags `nu`, lus par `get_nu` après D.11e ⟲ `:237`), `pico8_he` `:271` (⟲ `:266`) | `const pico8_list_t *pico8_cg(const char *)` (`entity.c:193`, `entity.h:39`) | a2 |
| calques de `dc` | `pico8_list_t layers[15];` local (`entity.c:279`) ; init `:286`, ajout `:307`, lecture `:312`, libération `:339` | `pico8_dc` seul | aucun | **a1** |
| `ui.ic` | `pico8_ui_ic_t { layer_count; pico8_list_t layers[16]; }` (`ui.h:22-25`) ; `ic_own` initialisé dans `pico8_ui_struct_new` (`ui.c:163-166`), vidé par `gw` (`ui.c:329-333`) | `pico8_ui_gw` `:351` (⟲ `:346` `get_fa`) | membre `ic` de `pico8_ui_t` (`ui.h:39-40`) | a2 |
| `ui.le` | membre `pico8_list_t *le` (`ui.h:36`) ; partagée `static pico8_list_t g_shared_le;` (`ui.c:25`), vidée par `pico8_ui_init` (`ui.c:181`) ; tas de `pz` (`msgbox.c:405-426`) et de `cj` (`ui.c:517-519`), TROU-7 | `pico8_pz` (`msgbox.c:408`, `:411`), `pico8_ui_cj` | `pico8_list_t *pico8_ui_le_shared(void)` (`ui.c:31`, `ui.h:68`) ; membre `le` | **a1** |
| `mz.ox` (L2 → ici) | membre `pico8_list_t *ox` (`wormhole.h:37` ⟲) ; tas (`wormhole.c:86-88` ⟲, TROU-7) | `pico8_mz_pq` (filtre de `hv`) | membre `ox` | a2 |
| `g_ga_by_shared` | `static pico8_list_t g_ga_by_shared;` (`planet_family.c:25` ⟲), `jo.by = &g_ga_by_shared` (`:914`) | routes de `trade` (`jo.by`, L2) | membre `jo.by` | **reporté à D.12b** |

#### Parcours dans `src/`

| Fichier:ligne (`src/pico8_`) | Fonction | Registre | Forme du parcours (gardée) | Ce qui change | Sous-étape |
|---|---|---|---|---|---|
| `entity.c:222-246` ⟲ | `pico8_lt` | `hv`, `cv[i]`, `cg[tag]` | ajouts | `pico8_list_add(…, pico8_value_ref(e))` → `pico8_ptrs_add(…, e)` (3 lignes) | a2 |
| `entity.c:251-275` ⟲ | `pico8_he` | idem | `del` par identité | `pico8_list_del(…, pico8_value_ref(e))` → `pico8_ptrs_del(…, e)` (3) | a2 |
| `entity.c:102-146`, `:148-161`, `:193-196` ⟲ | `shutdown`, `init`, `pico8_hv`/`cv`/`cg`, `cg_find_or_create` | tous | — | stockage, init/free, types de retour | a2 |
| `entity.c:277-341` ⟲ | `pico8_dc` | `cv(nt)` (source), calques | index | calques : `pico8_ptrs_t layers[15]` (a1) ; source `cv` : `bucket_src->items[i]` sans `.as.ref` (a2) | a1, a2 |
| `bootstrap.c:63-91` ⟲ | `pico8_dg` | `hv` | **index** (TROU-5) | `const pico8_ptrs_t *hv` ; `hv->items[i]` ; les deux déclarations `qc`/`ke` restent (en-tête et `ke` du même élément) | a2 |
| `bg.c:221-240` ⟲ | `pico8_bg_ldown` | `cg("g_")` | compte | type du pointeur | a2 |
| `economy.c:223-236` ⟲ | `pico8_hy_hk` | `cv("hk")` | `all()` | itérateur `pico8_ptrs_iter_t` ; le test de genre REF disparaît, le test `!= NULL` reste | a2 |
| `economy.c:298-314` ⟲ | `pico8_hy_lj` | `cg("jo")` | `all()` | idem | a2 |
| `economy.c:337-350` ⟲ | `pico8_hy_lj` | `cv("hb")` | `all()` | idem | a2 |
| `economy_formulas.c:50-62` | `for_each_cg` (statique) | `cg(tag)` | `all()` | idem | a2 |
| `economy_formulas.c:145-146` | `pico8_bk` | `cg("jc")` | compte | type du pointeur | a2 |
| `economy_score.c:28-40` | `for_each_cg` (statique) | `cg(tag)` | `all()` | idem | a2 |
| `economy_score.c:55-56` | `pico8_da` | `cg("qh")` | compte | type | a2 |
| `economy_score.c:69-83` | `cz_predicate`, `pico8_cz` | `cg("jo")` | `jp` | `pico8_jp_ptrs` ; prédicat `int (void *e, void *ctx)` | a2 |
| `economy_tiers.c:34-46` | `for_each_cg` (statique) | `cg(tag)` | `all()` | idem | a2 |
| `probe.c:128-136`, `:236`, `:244-254` ⟲ | `pico8_bv_jc_predicate`, `pico8_bv` | `cg("np")` (`ij`), `cg("jc")` (`jp`) | `ij`, `jp` | `ij` : texte inchangé ; `jp` → `pico8_jp_ptrs`, prédicat en `void *`, `first = pico8_value_ref(filtered.items[0])` (la suite lit une `pico8_value_t`, Q12) | a2 |
| `score_panel.c:141-165` ⟲ | `pico8_eg_gg_pred`, `pico8_eg` | `cg("jo")` | `jp` | `pico8_jp_ptrs`, prédicat en `void *` | a2 |
| `slipway.c:318-345`, `:401-428` ⟲ | prédicats `np`/`jc` de `jc:ce`, `pico8_jc_ce` | `cg("np")`, `cg("jc")` | `jp`, `la` | `pico8_jp_ptrs` ×2, `pico8_la_ptrs` ; `result = pico8_value_ref(ds.items[0])` (retour `pico8_value_t` gardé, Q12) | a2 |
| `wormhole.c:62-89` ⟲ | `pico8_mz_pq_ox_predicate`, `pico8_mz_pq` | `hv` → `mz.ox` | `jp` | `pico8_jp_ptrs(pico8_hv(), …)` ; `ox` : `pico8_ptrs_t *` | a2 |
| `wormhole.c:104` ⟲ | `pico8_mz_hq` | `mz.ox` | `ij` | texte inchangé | a2 |
| `wormhole.c:182-213` ⟲ | `pico8_mz_pr` | `cg("jo")` | **index** | `jo_list->items[i]` (`pt_val` devient un pointeur ; le test de genre disparaît, `NULL` reste, l'`assert` de famille reste) | a2 |
| `ui.c:247-290` ⟲ | `pico8_ui_lj` | `ui.le` | `gu` | `pico8_ptrs_t *le_ptr_before` ; `pico8_gu_ptrs(le_ptr_before, self->o_)` | a1 |
| `ui.c:318-355` ⟲ | `pico8_ui_gw` | `cv("hd")`, `ui.ic` | index | `hd_bucket` : `const pico8_ptrs_t *`, `->items[i]` ; calques : `pico8_ptrs_free`/`_init`/`_add` | a2 |
| `ui.c:443-480` ⟲ | `pico8_ui_jz` | `ui.ic` | index (15 → 0) | `const pico8_ptrs_t *bucket` ; `bucket->items[i]` | a2 |
| `ui.c:497-520` ⟲ | `pico8_ui_cj_he_mapper`, `pico8_ui_cj` | `ui.le` | `nv` | `pico8_nv_ptrs` ; mappeur `int (void *e, void *ctx, void **out)` ; tas `pico8_ptrs_t` | a1 |
| `ui.c:25`, `:31-33`, `:159`, `:181` ⟲ | `g_shared_le`, `pico8_ui_le_shared`, `struct_new`, `pico8_ui_init` | `ui.le` | — | type ; `pico8_ptrs_init` | a1 |
| `ui.c:163-166`, `:329-333` ⟲ | `pico8_ui_struct_new`, `pico8_ui_gw` | `ui.ic` | — | `pico8_ptrs_init`/`_free` | a2 |
| `msgbox.c:352-428` ⟲ | `pico8_pz` | `ui.le` | ajouts | `pico8_ptrs_t *le_heap`, `pico8_ptrs_add(le_heap, pico8_pz_step(…))`, tas vide `empty_heap` | a1 |

« Forme du parcours gardée » : un `all()` reste un itérateur à tolérance de suppression, une boucle à index reste une boucle à index (TROU-5 dans `dg`), un `foreach` reste un `foreach`. Rejeté : « simplifier » un `all()` en boucle à index (un rappel `hk`/`hb` qui ferait `he` pendant le parcours changerait de comportement).

#### Aides communes (`src/pico8_collect.c/.h`, `src/pico8_entity_geom.c/.h` ⟲ pour `.c`)

| Aide | Appels sur un registre L1 (convertis ici) | Appels sur une liste L2/L3/L4 (restent) | Décision |
|---|---|---|---|
| `pico8_jp` (filtre, `collect.c:68`) | `economy_score.c:79`, `probe.c:249`, `score_panel.c:163`, `slipway.c:417`, `:418`, `wormhole.c:87` | `planet_display.c:195` (`jy`), `research_display.c:361`, `tech_gate.c:126` (listes locales) | `pico8_jp_ptrs` nouveau (a2) ; `pico8_jp` gardé pour les trois autres |
| `pico8_la` (concaténation, `collect.c:105`) | `slipway.c:419` | `planet_display.c:202`, `:223`, `:226`, `planet_trade.c:130`, `trade.c:167`, `:169` | `pico8_la_ptrs` nouveau (a2) |
| `pico8_gu` (appartenance, `collect.c:53`) | `ui.c:276` (`ui.le`) | `planet_family_trade.c:150` (`t_rl`) | `pico8_gu_ptrs` nouveau (a1) |
| `pico8_nv` (application, `collect.c:85`) | `ui.c:512` (`ui.le`) | `layout.c:170` (`lq.items` → largeurs fixes : ne peut pas devenir une liste de pointeurs), `planet.c:673` (`hp.i`) | `pico8_nv_ptrs` nouveau (a1) |
| `pico8_ij` (plus proche, `entity_geom.c:87` ⟲) | `galaxy.c:69` (`cg("jo")`), `probe.c:236` (`cg("np")`), `wormhole.c:104` (`mz.ox`) | aucun | **converti en place** (a2) : `const pico8_ptrs_t *j_` ; `out_nearest` reste `pico8_value_t *` (Q12, comme D.11e) |
| `pico8_gu_content`, `pico8_la_append` | aucun | `tech_gate.c:39` ; interne à `pico8_la` | inchangés (D.12b) |

Aucun TU n'appelle `pico8_jp`, `pico8_ij`, `pico8_gu`, `pico8_nv` ni `pico8_la` (grep) : les aides nouvelles ne sont exercées que par `src/` ; `gcov` et les mutations le vérifient.

#### Lectures de registres dans les TU (détail ligne à ligne : Annexe D)

34 fichiers ; lignes qui appellent `pico8_hv()`/`pico8_cv()`/`pico8_cg()` : 166 / 82 / 132 (29 / 27 / 24 fichiers) ; lignes qui lisent `ok->le`, `ok->ic`, `pico8_ui_le_shared()` ou leurs aides : dans 10 fichiers ; 65 aides locales à paramètre `const pico8_list_t *` ; 37 variables locales `const pico8_list_t *x = pico8_hv()/cv()/cg()` ; environ 127 lectures directes d'éléments (`->items[i].as.ref`) hors aides. ⟲ : `iv`, `mo`, `ui`, `g_`, `mz`, `jc`, `ie`.

| Groupe | TU | Sous-étape |
|---|---|---|
| `ui.le` seul (lectures directes, `ok_le()`, `le_of()`, `pico8_ui_le_shared()`) | `test_pico8_ga_export_panel_by` (§6), `test_pico8_planet_gh_row_lq` (§6, lié sous ASan), `test_pico8_spec_defaults`, `test_pico8_lq`, `test_pico8_pj`, `test_pico8_fv`, `test_pico8_js`, `test_pico8_ui` ⟲ | a1 (8) |
| `hv`/`cv`/`cg`/`ui.ic`/`mz.ox` | `bg`, `fq`, `fr`, `fv`, `g_` ⟲, `ga`, `hj`, `hp`, `hy`, `ie` ⟲, `iv` ⟲, `jc` ⟲, `je`, `jo`, `js`, `lq`, `mh`, `mo` ⟲, `mx`, `mz` ⟲, `nf`, `nl`, `nr`, `os`, `pj`, `planet_kj_dispatch`, `pm`, `qb`, `qh`, `trade`, `ui` ⟲ | a2 (31) |

**Aides locales partagées entre L1 et L2** (repérées, à scinder dans le TU, la partie L2 garde la forme valeur jusqu'à D.12b/D.12d) : `test_pico8_mx.c:545` (`listed(…->qh.jo.oe, jc)`), `test_pico8_jc.c:258`, `:297`, `:313`, `:482`, `:536`, `:545`, `:552`, `:740`, `:744` (`list_count` sur `jc->ts` et `jo.oe`), `test_pico8_fv.c:357-358`, `:416`, `:430` et `test_pico8_g_.c:231`, `:368-369`, `:578-579`, `:708-718` (`size_of` sur `emitter->qy`), `test_pico8_fv.c:400` (`size_of(pico8_ok()->le)`, a1). `test_pico8_js.c:386` (`rank_in(ok_le(), b)`) et `:622` (`listed(ok_le(), …)`) : `ui.le` passe en a1 alors que `rank_in`/`listed` servent aussi `hv`/`cv`/`cg` jusqu'à a2 : en a1, ces deux appels lisent le conteneur par une aide locale du TU sur `pico8_ptrs_t` (nommée par l'implémenteur), fondue en a2 dans `rank_in`/`listed` convertis (le seul va-et-vient de l'étape, 2 appels, déclaré).

#### `Makefile` et liens

- Listes : `ENGINE_SRCS` (`Makefile:169-229`, `src/pico8_list.c` à `:198`) et `DATA_ENGINE_SRCS` (`:238-288`, `:262`) gagnent `src/pico8_ptrs.c`.
- Lignes de lien explicites qui nomment `src/pico8_collect.c` ou `src/pico8_entity.c` hors listes : `$(TMP_DIR)/test_pico8_layout_nil_item_draws_nothing` (`:569`, §6, test inchangé) et `$(TMP_DIR)/main_raylib` (`:985`, prérequis, et `:986`, commande) : gagnent `src/pico8_ptrs.c`. `make raylib` n'est pas lancé (il faut le vendoring) ; la ligne est contrôlée par grep.
- Nouveau : cible `$(TMP_DIR)/test_pico8_ptrs` (sur le modèle de `:419`, `test_pico8_list` : le TU et `src/pico8_ptrs.c` seuls) et entrée `test_pico8_ptrs` dans `UNIT_TESTS` (`:572-`).
- `DATA_RESIDUE_GUARD` (`:889`) : **inchangé** (`liste` ne tombe pas à 0).
- Règle de lien : `pico8_ptrs.c` n'appelle que la libc ; il entre dans `DATA_ENGINE_SRCS` ; `collect.c`, `entity.c`, `entity_geom.c`, `ui.c`, `msgbox.c`, `wormhole.c` (tous dans la liste) le nomment.

## Solution

**Deux sous-étapes** (D.12a1, D.12a2), même PRD, chacune verte seule, vérifiée et poussée (modèle des découpages de D.11d et D.11e) :

1. **D.12a1 — compteur, conteneur, calques de `dc`, `ui.le`** (20 chemins, ≈ 45 min), **deux commits** :
   - **commit 1, seul** : `tools/data_residue.sh` gagne la catégorie `liste` (motif de Q10 recopié), `tests/test_data_residue.c` la juge. Aucun autre fichier.
   - **commit 2** : `pico8_ptrs.c/.h` et `test_pico8_ptrs.c` (sémantiques de Q9) ; `Makefile` (listes, liens, cible) ; calques de `dc` ; `ui.le` (`pico8_ui_le_shared`, `lj`, `cj`, `pz`), avec `pico8_gu_ptrs` et `pico8_nv_ptrs` ; les 8 TU qui lisent `ui.le`.
2. **D.12a2 — `hv`, `cv`, `cg`, `ui.ic`, `mz.ox`** (≈ 51 chemins, ≈ 60 min) : stockage et accesseurs des trois registres ; parcours de la table « Parcours » ; `pico8_jp_ptrs`, `pico8_la_ptrs` ; `pico8_ij` converti ; `ui.ic` ; `mz.ox` ; les 31 TU qui les lisent ; note « Découvert à D.12a ».

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (dont `make data-guard`, inchangé, et `test_data_residue`, `test_pico8_ptrs`) ; `make e2e` : **26 conformes** ; `make sanitize-net` propre ; balayage ASan+UBSan des TU propre ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (a2) ;
- `make data-residue` : une ligne `liste` ; `liste` ≤ 826 − L1, rapporté (≈ 600) ; `valeur`, `type-tag`, `ref-audit` rapportés ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/` ; `tools/` : seul `data_residue.sh`, au commit 1 ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  # 1. plus aucun registre L1 en pico8_list_t dans src/ : vide
  grep -nE 'pico8_list_t (g_hv|layers)|pico8_list_t list;|pico8_list_t \*(le|ox)\b|g_shared_le|pico8_list_t \*pico8_(hv|cv|cg|ui_le_shared)\b' src/*.[ch]
  # 2. plus aucun parcours de registre qui déballe une valeur : vide
  grep -nE 'pico8_(hv|cv|cg)\([^)]*\)|hd_bucket|hk_list|hb_list|jo_list|qh_list|g_tag|->ox\b|ic->layers|->le\b' src/*.c | grep -E 'as\.ref|pico8_list_|pico8_value_ref\(e\)'
  # 3. le conteneur hors de tous les motifs : aucune ligne comptée dans ses fichiers
  sh tools/data_residue.sh --detail | grep -E 'pico8_ptrs\.[ch]'            # vide
  # 4. l'ancien jp/la/gu/nv n'est plus appelé sur un registre : vide
  grep -nE 'pico8_(jp|la|gu|nv)\((pico8_(hv|cv|cg)|jo_list|le_ptr_before|self->le|&part1)' src/*.c
  # 5. g_ga_by_shared inchangé (reporté à D.12b) : une ligne
  grep -c 'static pico8_list_t g_ga_by_shared;' src/pico8_planet_family.c   # 1
  # 6. règle de lien : les TU liés sur la seule liste DATA_ENGINE_SRCS se lient
  make tmp/test_pico8_bg_kj tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_planet_gh_row_lq tmp/test_pico8_ga_export_panel_by
  # 7. toute ligne de lien qui nomme collect.c ou entity.c nomme aussi ptrs.c
  grep -nE 'src/pico8_(collect|entity)\.c' Makefile | grep -v 'pico8_ptrs\.c' | grep -vE '^[0-9]+: +src/'   # vide
  # 8. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^liste'
  ```

## User Stories

1. En tant que gardien du compteur, je veux une catégorie `liste` au motif de Q10, ajoutée dans un commit qui ne contient qu'elle et son test, afin que les étapes D.12 aient un « Compte » et que l'ajout ne se mêle à aucune conversion.
2. En tant que gardien du compteur, je veux que ce commit ne change aucun motif existant ni aucune ligne de `src/`/`tests/` hors `test_data_residue.c`, et que son message énonce le motif et les comptes révélés, afin que la règle « motifs figés » de D.0 reste vraie.
3. En tant que gardien du compteur, je veux que les cas nouveaux de `test_data_residue` coupent leurs littéraux, afin que le test du compteur n'ajoute aucune ligne à aucune catégorie.
4. En tant que mainteneur du port, je veux un seul conteneur de pointeurs (`void *`), avec `add`, `del` par identité, `all` à tolérance de suppression et `foreach`, afin que la sémantique PICO-8 de ces primitives reste écrite une fois (Q9).
5. En tant que mainteneur du port, je veux que ce conteneur porte un nom hors du motif `liste` et qu'il ne soit pas un renommage de `pico8_list_t` (élément `void *`, pas d'étiquette), afin que la baisse de `liste` veuille dire « converti ».
6. En tant que gardien des tests, je veux un TU du conteneur qui reprend les cas de `test_pico8_list` sur des pointeurs (ordre, `del` du premier identique, `del` absent sans effet, `all` qui tolère la suppression de l'élément courant, `foreach`), afin que ses sémantiques soient jugées sans passer par le jeu.
7. En tant que mainteneur du port, je veux que `hv`, `cv`, `cg` stockent des pointeurs d'en-tête et que leurs accesseurs rendent `const pico8_ptrs_t *`, afin qu'aucun parcours ne déballe plus une `pico8_value_t` qui ne peut être qu'un REF.
8. En tant que mainteneur du port, je veux que `dg` parcoure `hv` par index comme aujourd'hui, afin que TROU-5 reste identique.
9. En tant que mainteneur du port, je veux que chaque `all()` reste un itérateur à tolérance et chaque boucle à index reste à index, afin que l'ordre de visite et les sauts restent ceux d'aujourd'hui.
10. En tant que mainteneur du port, je veux que les calques de `dc`, `ui.ic` et `ui.le` soient des conteneurs de pointeurs, afin que le dessin, l'ordre de priorité des clics et les boutons de panneau lisent l'entité directement.
11. En tant que mainteneur du port, je veux que `mz.ox`, fabriqué par un filtre de `hv`, et `pico8_ij`, qui parcourt `cg` et `mz.ox`, changent avec `hv` et `cg`, afin qu'aucune cale ne convertisse entre les deux formes.
12. En tant que mainteneur du port, je veux des aides `jp`/`la`/`gu`/`nv` sur le conteneur pour les seuls parcours de registres, les aides à valeur restant pour leurs autres appelants, afin de ne pas tirer dans D.12a les listes L2/L3/L4 de D.12b et D.12c.
13. En tant que mainteneur du port, je veux que `g_ga_by_shared` reste une `pico8_list_t` jusqu'à D.12b, afin que son type suive `jo.by`, dont il est le défaut partagé.
14. En tant que gardien des tests, je veux que les 34 TU qui lisent les registres suivent dans le commit qui change le registre, constantes inchangées, afin qu'ils testent le vrai registre et pas une copie.
15. En tant que gardien des tests, je veux que les deux tests §6 qui lisent `ok.le` ne changent que leur façon de lire l'état, afin que leur garde reste la même.
16. En tant que gardien des filets, je veux `make test`, `make e2e`, `make sanitize-net` et le balayage ASan+UBSan des TU verts à chaque sous-étape, et `make class-coverage-net` en a2, afin de prouver qu'aucune durée de vie, aucun ordre et aucune instanciation ne change.
17. En tant que gardien des tests, je veux `gcov` avant/après chaque sous-étape et des mutations ciblées restaurées par `cp`, afin que chaque ligne ajoutée soit exécutée et qu'aucune ligne survivante ne cesse de l'être.
18. En tant que gardien du compteur, je veux au message de chaque commit, par catégorie et par fichier, les lignes et occurrences avant/après, la classification, les lignes qui gagnent une catégorie (« aucune » compris), et chaque Δlignes > Δoccurrences justifié, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
19. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, qu'aucun alias, registre, renommage pour les greps, cast non vérifié ni cale permanente ne soit ajouté, afin que la baisse veuille dire « retiré ».
20. En tant que `plan-pilot`, je veux deux sous-étapes d'environ 45 et 60 minutes, un repli écrit si la seconde dépasse son plafond, et une note « Découvert à D.12a » qui dise les comptes réels, les écarts à la ligne (`g_ga_by_shared`, `mz.ox`, TU, §6) et ce que D.12b–D.12d trouvent, afin que la suite parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q9 et Q10 sont des décisions humaines que ce PRD applique ; Q12 reste ouverte et n'est pas tranchée ici.

### Modules

- **Compteur** (`tools/data_residue.sh`, commit 1) : catégorie `liste`.
- **Conteneur** (nouveau, `src/pico8_ptrs.c/.h`) : `pico8_ptrs_t` et ses opérations.
- **Registres** : `pico8_entity.c/.h` (`hv`, `cv`, `cg`, calques de `dc`), `pico8_ui.c/.h` (`ui.ic`, `ui.le`), `pico8_msgbox.c` (construction de `ui.le` par `pz`), `pico8_wormhole.c/.h` (`mz.ox`).
- **Parcours** : `pico8_bootstrap.c`, `pico8_bg.c`, `pico8_economy.c`, `pico8_economy_formulas.c`, `pico8_economy_score.c`, `pico8_economy_tiers.c`, `pico8_probe.c`, `pico8_score_panel.c`, `pico8_slipway.c`, `pico8_wormhole.c`, `pico8_ui.c`.
- **Aides** : `pico8_collect.c/.h` (quatre aides `_ptrs`), `pico8_entity_geom.c/.h` (`pico8_ij`).
- **Build** : `Makefile` (listes, deux lignes de lien, cible et entrée du TU nouveau).
- **TU** : `tests/test_data_residue.c` (commit 1), `tests/test_pico8_ptrs.c` (nouveau), 34 TU lecteurs de registres.
- Inchangés : `galaxy.c`, `cart.c`, `hud.c`, `pico8_list.c/.h`, `test_pico8_list.c`, `planet_family.c` (`g_ga_by_shared`).

### Catégorie `liste` (G, commit 1)

- `CATEGORIES="valeur ob stash type-tag ref-audit par-nom dispatch overrides spec liste"` (dernière : les sorties existantes gardent leur ordre) ; bras `liste) printf '%s\n' '\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h' ;;`. Rien d'autre dans le script (ni commentaire d'en-tête, qui ne liste pas les catégories).
- `tests/test_data_residue.c` : `test_one_occurrence_per_category_in_table_order` gagne un dixième fichier (`"pico8_" "list_t l;"`) et sa ligne `liste\t1\t1` avant le pied (`10 ligne(s), 10 fichier(s)`) ; un cas de bornes : `pico8_list.h`, `PICO8_REF_AS_LIST_CONST`, `PICO8_TYPE_TAG_LIST` comptent, `mypico8_list_t`, `pico8_listing`, `pico8_ptrs_t` ne comptent pas. Littéraux coupés : `sh tools/data_residue.sh --detail tests | grep test_data_residue` vide avant et après.
- Message : le motif, `liste` = N lignes / M fichiers / K occurrences (`src/` et `tests/`), résidu D avant/après (à `de6351e` : 2336 → 3019 lignes, 124 fichiers inchangés ; remesuré au départ réel), « aucune autre catégorie ne change » (comptes identiques), « aucune ligne de `src/` ni de `tests/` hors `test_data_residue.c` ».
- Rejeté : ajouter la catégorie dans le commit du conteneur (la ligne l'interdit) ; la mettre dans `DATA_RESIDUE_GUARD` (elle n'est pas à 0).

### Conteneur (G, Q9)

Forme retenue (`src/pico8_ptrs.h`) :

```c
/* PICO-8's add()/del()/all()/foreach() over plain pointers. The container
 * owns only `items`: what an element points at is the caller's. NULL is a
 * storable element. */
typedef struct {
    void **items;
    int count;
    int capacity;
} pico8_ptrs_t;

void pico8_ptrs_init(pico8_ptrs_t *ptrs);
void pico8_ptrs_free(pico8_ptrs_t *ptrs);
void pico8_ptrs_add(pico8_ptrs_t *ptrs, void *e);
/* Removes the FIRST element identical to `e` (pointer equality); 1 if one was removed. */
int pico8_ptrs_del(pico8_ptrs_t *ptrs, const void *e);

typedef struct {
    const pico8_ptrs_t *ptrs;
    int index;
    int last_count;
} pico8_ptrs_iter_t;
pico8_ptrs_iter_t pico8_ptrs_all(const pico8_ptrs_t *ptrs);
int pico8_ptrs_iter_next(pico8_ptrs_iter_t *iter, void **out);

typedef void (*pico8_ptrs_foreach_fn)(void *e, void *ctx);
void pico8_ptrs_foreach(const pico8_ptrs_t *ptrs, pico8_ptrs_foreach_fn fn, void *ctx);
```

- **Sémantiques** reprises de `pico8_list.c` à l'identique, élément près : croissance par doublement depuis 4 (`abort()` sur échec) ; `del` décale les suivants ; tolérance de `all()` (« récupère après la suppression de l'élément courant, pas d'un précédent ») ; `foreach` sans tolérance. Les commentaires de contrat de `pico8_list.h` (sources PICO-8 wiki Del/All) sont réécrits pour le conteneur selon les quatre sortes de `CLAUDE.md` (contrat C que le nom ne dit pas), pas recopiés tels quels.
- **`del` par identité** : `pico8_list_del` compare par `pico8_value_equal`, qui pour deux REF compare les pointeurs (`value.c:47-48`) : même résultat pour tous les éléments des registres (tous REF). Aucun changement de comportement.
- **Pas d'étiquette de type** (G) : `PICO8_TYPE_TAG_LIST` existe pour qu'un REF nu soit reconnu par `pico8_type_tag_of` ; un registre n'est jamais une valeur REF après D.11e (H-D11e-4). Rejeté : un `PICO8_TYPE_TAG_PTRS` (catégorie `type-tag` en hausse pour un lecteur qui n'existe pas). Si H-D11e-4 tombe, rouvrir ici.
- **`assert` de contrat** : les primitives assertent `ptrs != NULL` (message au modèle de `"list primitive called on a NULL list"`), comme aujourd'hui ; pas de `PICO8_REF_AUDIT_EXPECT_*` (audit des REF, sans objet sur un pointeur typé).
- **Nom** : `pico8_ptrs` ne correspond à aucun motif du compteur (vérifié : `valeur`, `ob`, `stash`, `type-tag`, `ref-audit`, `par-nom`, `dispatch`, `overrides`, `spec`, `liste`). Déclaré comme **nom nouveau d'un type nouveau**, pas comme renommage (règle commune : « un nom nouveau ne correspond à aucun motif figé »). Rejetés : `pico8_list2_t`, `pico8_ref_list_t` (lus comme des variantes ou des alias de la liste ; `ref` évoque les valeurs REF qu'on quitte) ; un type par élément (Q9, option B, refusée) ; macro générique.
- **Coexistence** : `pico8_list.c/.h` et `test_pico8_list` restent intacts (usagers L2/L3/L4) jusqu'à D.12d, qui « réécrit `test_pico8_list` sur le nouveau conteneur ». D.12a crée `test_pico8_ptrs` ; D.12d tranchera s'il absorbe `test_pico8_list` (noté pour D.12d).

### Aides communes (G)

- **`pico8_jp_ptrs`**, **`pico8_la_ptrs`**, **`pico8_gu_ptrs`**, **`pico8_nv_ptrs`** dans `pico8_collect.c/.h`, chacune la transcription de l'aide à valeur sur le conteneur (même parcours : `all()` pour `jp`/`gu`/`nv`, `foreach` pour `la` ; même tolérance de NULL : `jp`/`gu` acceptent un `qk` NULL, `nv`/`la` l'assertent) :

  ```c
  typedef int (*pico8_jp_ptrs_predicate_fn)(void *e, void *ctx);
  pico8_ptrs_t pico8_jp_ptrs(const pico8_ptrs_t *qk, pico8_jp_ptrs_predicate_fn ow, void *ctx);
  pico8_ptrs_t pico8_la_ptrs(const pico8_ptrs_t *a, const pico8_ptrs_t *b);
  int pico8_gu_ptrs(const pico8_ptrs_t *qk, const void *kb);
  typedef int (*pico8_nv_ptrs_fn)(void *e, void *ctx, void **out);
  pico8_ptrs_t pico8_nv_ptrs(const pico8_ptrs_t *qk, pico8_nv_ptrs_fn kp, void *ctx);
  ```

  Seuls les appels de la colonne « registre L1 » de la table « Aides communes » les utilisent. `pico8_gu_ptrs` n'a pas le commentaire TROU-3 de `pico8_gu` (qui porte sur l'égalité de valeurs à genres mêlés) : l'appartenance est l'identité.
- **Pourquoi pas une conversion en place** : `pico8_nv` sert aussi `layout` (des largeurs fixes, jamais des pointeurs) ; `pico8_jp`, `pico8_la`, `pico8_gu` servent des listes L2/L4 de D.12b ; les convertir en place tirerait `jy`, `hp.i`, `t_rl`, les listes locales de `research_display`, `tech_gate`, `planet_display`, `trade` dans D.12a. Les aides à valeur gardent **exactement** leurs appelants restants ; D.12b convertit ceux-ci, supprime les aides à valeur devenues sans appelant et tranche le nom définitif (garder `_ptrs` ou reprendre `pico8_jp`… une fois l'ancien parti). Noté pour D.12b.
- **Pas un renommage pour les greps** : les aides à valeur restent, comptées, avec leurs appelants ; les nouvelles ont un autre type de paramètre et d'élément. Déclaré au message.
- **`pico8_ij` converti en place** (a2) : ses trois appelants passent tous au conteneur dans la même sous-étape. Paramètre `const pico8_ptrs_t *j_` ; élément `void *` ; l'`assert` « ij walks a list of entities » garde son test d'étiquette (`e == NULL || pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT`, ligne `type-tag` modifiée, déclarée) ; `*out_nearest = pico8_value_ref(e)` (sortie à valeur gardée, Q12 ; ligne `valeur` modifiée ou gagnée, déclarée). Les textes des appels (`galaxy.c:69`, `probe.c:236`, `wormhole.c:104`) ne changent pas.
- **Prédicats et mappeurs** (`cz_predicate`, `pico8_bv_jc_predicate`, `pico8_eg_gg_pred`, `pico8_jc_ce_np_predicate`, `pico8_jc_ce_jc_predicate`, `pico8_mz_pq_ox_predicate`, `pico8_ui_cj_he_mapper`) : paramètre `void *e` ; le test `e.kind != PICO8_VALUE_REF || e.as.ref == NULL` devient `e == NULL` ; les `assert` de classe ou de famille qu'ils portent restent, mot pour mot.

### Registres (G)

- **Stockage** : `g_hv` → `pico8_ptrs_t` ; `pico8_cv_entry_t.list`, `pico8_cg_entry_t.list` → `pico8_ptrs_t` (noms de membres gardés : chirurgical) ; `pico8_ui_ic_t.layers[16]` → `pico8_ptrs_t` ; `pico8_ui_t.le` → `pico8_ptrs_t *` ; `g_shared_le` → `pico8_ptrs_t` ; `pico8_mz_t.ox` → `pico8_ptrs_t *` ; calques de `dc` → `pico8_ptrs_t layers[15]`.
- **Accesseurs** : `const pico8_ptrs_t *pico8_hv(void)`, `pico8_cv(const char *)`, `pico8_cg(const char *)` ; `pico8_ptrs_t *pico8_ui_le_shared(void)`. Même contrat (NULL pour un nom de `cv` inconnu ou un tag jamais vu ; pointeur de `cg` invalidé par un `lt` qui crée un tag) : commentaires gardés, « PICO8_VALUE_REF values whose .as.ref is » réécrit (« pointers to »).
- **Éléments** : `lt` ajoute `e` (l'en-tête du `ke`), `he` retire `e` ; aucun registre ne reçoit autre chose (seul `lt` écrit `hv`/`cv`/`cg` ; `gw` n'ajoute que des éléments de `cv("hd")` à `ic` ; `pz` n'ajoute que des retours de `pico8_pz_step`).
- **Lecture des éléments** : un élément est un `void *` ; il est affecté à `pico8_struct_header_t *` (conversion implicite C, pas de cast : ce que `lt` y a mis) ; toute conversion vers une struct **précise** (`pico8_jo_t *`, `pico8_jc_t *`, `pico8_ke_t *`…) garde le test de famille ou de classe qui la précède aujourd'hui (`pico8_jo_is_family`, `cls == pico8_jc_struct_class()`, `assert` « cv holds entities only », « cg.jo holds planets only », « hv holds structs only ») ; aucune n'est ajoutée sans test. La lecture `ke` de `dg` (`pico8_ke_t *ke = hv->items[i]`) est celle d'aujourd'hui (`.as.ref`), sous l'`assert` existant : inchangée en substance, déclarée.
- **Includes** : un `.c` dont plus rien ne nomme `pico8_list_*` perd son `#include "pico8_list.h"` (`bootstrap.c:8`, `ui.c:11` si `ui.c` n'en a plus besoin) ; un en-tête qui ne nomme plus la liste (`entity.h:5`, `ui.h:8`, `wormhole.h:8`, `entity_geom.h:5`) **garde** l'include si l'enlever casse un fichier qui s'en servait par transitivité (13 fichiers de `src/` et 2 TU s'en servent sans l'inclure : relevé) ; sinon il l'enlève. Rejeté : ajouter des includes directs dans une quinzaine de fichiers hors empreinte (lignes gagnant `liste`, pour les retirer à D.12d). Chaque include gardé ou retiré est listé au message ; ils tombent avec `pico8_list.h` (D.12d).
- **`g_ga_by_shared`** : inchangé (reporté à D.12b avec `jo.by`, dont il est le défaut partagé). Rejeté : le convertir seul (types incompatibles avec `jo.by`) ; tirer `jo.by` dans D.12a (L2 : `trade`, `economy_tiers`, `economy_score`, `planet_trade`, `ga`…).
- **`mz.ox`** : `pico8_mz_pq` alloue un `pico8_ptrs_t` (TROU-7 inchangé : jamais libéré), `*heap_ox = pico8_jp_ptrs(pico8_hv(), pico8_mz_pq_ox_predicate, NULL)`. Déclaré : `mz.ox` sort de la liste de D.12b.

### Les TU suivent (G, règle des ponts, l. 267)

- **Tranché** : chaque TU qui lit un registre suit dans le commit qui change ce registre ; aucun pont. Rejetés : un double `pico8_list_t` tenu en parallèle pour les TU (les TU testeraient la copie ; interdit : cale et doublon d'état) ; un instantané reconstruit à chaque appel de l'ancien accesseur (cale valeur ↔ typé, pointeurs gardés par un TU périmés en silence).
- **Forme dans un TU** : les aides locales (`listed`, `rank_in`, `count_of`, `list_count`, `list_size`, `size_of`, `ok_le`, `le_of`, `button_lq`…) prennent `const pico8_ptrs_t *` ou un élément `void *` ; les lectures directes `->items[i].as.ref` deviennent `->items[i]` ; les tests de genre `items[i].kind == PICO8_VALUE_REF` disparaissent (le test `!= NULL` reste s'il existe). Une aide qui sert aussi une liste L2 (table « Aides locales partagées ») est scindée : la partie L2 garde sa forme valeur (comptée, jusqu'à D.12b/D.12d), nommée par l'implémenteur, listée.
- **Constantes** : aucune `*_HASH`/`identity[]` ne change (l'ordre d'inscription, les rangs et les comptes sont les mêmes). Une constante qui rougit est un bug de conversion.
- **Assertion d'étiquette** : `test_pico8_ui.c:305` ⟲ (`pico8_type_tag_of(pico8_ui_le_shared()) == PICO8_TYPE_TAG_LIST`) perd sa moitié d'étiquette (le type est vérifié à la compilation) et garde `le_of(ok)->count == 0` ; (a2) au message. Les autres `PICO8_TYPE_TAG_LIST` de TU portent sur des listes L2/L3 (`jo->by`, `by`, `hp.i`, `jb.ls`, `lq.fs`…) : inchangés.
- **§6** : `test_pico8_ga_export_panel_by` (`ok_le()`, boucles `:125-127`, `:150-152` ; `button_lq` prend l'élément) et `test_pico8_planet_gh_row_lq` (`:43-47`, `:137-140`) suivent `ui.le` en a1, façon de lire seulement ; leur `pico8_list_t *by`, `lq->items` (L2/L3) ne changent pas.

### Règle de lien (liant)

Aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste. Symboles nouveaux : `pico8_ptrs_*` (`pico8_ptrs.c`, ajouté à la liste) ; `pico8_*_ptrs` de `collect.c` (dans la liste). Preuve : grep 6 de la « Sortie » (TU liés sur la seule liste) et grep 7 (lignes de lien explicites).

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la liste à valeur est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours pour échapper au motif ; alias (macro, `typedef` de `pico8_list_t`, enveloppe qui renomme une aide) ; registre (tableau, `switch` ou boucle qui passe d'un nom à un membre) ; cast d'un élément vers une struct précise sans test ; cast de pointeur de fonction ; cale valeur ↔ conteneur (fonction de conversion, double tenu en parallèle, instantané) ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; tout changement d'une constante de hachage ; toute modification de `tests/e2e/`, d'un `*.sha256`, de `tools/` hors du commit 1 ; du `Makefile` hors des lignes de « `Makefile` et liens ».
- **Classification exigée**, au message de chaque commit, pour chaque ligne `liste` retirée ou modifiée (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) :
  - **(a1) conteneur converti** : la même opération (déclaration, init, libération, ajout, retrait, parcours, type d'accesseur) sur `pico8_ptrs_t` ;
  - **(a2) supprimée sans remplaçant** : include devenu inutile, assertion d'étiquette `PICO8_TYPE_TAG_LIST` sur un conteneur désormais typé, variable devenue inutile ;
  - **(b) signature dont le type de liste change** : attendues et listées : `pico8_hv`, `pico8_cv`, `pico8_cg`, `pico8_ui_le_shared`, `pico8_ij`, les prédicats et mappeurs de « Aides communes », les aides locales de TU ; toute autre est une erreur ;
  - **(c) même donnée, autre nom** : **attendu 0**. `pico8_ptrs_*` et `pico8_*_ptrs` sont déclarés « type nouveau », pas (c).
- **Mesure exigée**, avant et après chaque commit, par catégorie (`valeur`, `type-tag`, `ref-audit`, `liste` ; `par-nom` = 0 constaté) et **par fichier** du diff : lignes (`--detail`) **et** occurrences (recette de « État de départ »).
- **Lignes qui gagnent une catégorie**, par fichier et par catégorie, « aucune » compris. Attendues : commit 1, toutes les lignes `liste` révélées (création de la catégorie, pas une hausse) ; `valeur` : `pico8_value_ref(…)` posé sur un élément là où la suite lit une valeur (`probe.c` `first`, `slipway.c` `result`, `entity_geom.c` `out_nearest`), déclarées ; `type-tag` : aucune (l'`assert` de `ij` est une ligne modifiée) ; `liste` : les parties L2 scindées d'aides de TU et les includes gardés éventuellement déplacés, déclarées.
- **Lignes comptées modifiées, non retirées** (fichier:ligne) : listées (ex. : `assert` de `ij`, `type-tag`).
- **Δlignes > Δoccurrences** : chaque couple justifié (une ligne à deux jetons `liste` compte deux occurrences ; attendu : Δocc ≥ Δlignes).
- **Numéros de ligne** : ceux du **parent du commit** (HEAD avant le commit), jamais ceux de ce PRD sans remesure (⟲).

### Empreinte et découpage (G)

| Sous-étape | `src/` | `tests/` | autres | Chemins | Coût estimé |
|---|---|---|---|---|---|
| **a1, commit 1** | — | `test_data_residue.c` | `tools/data_residue.sh` | 2 | ≈ 8 min |
| **a1, commit 2** | `ptrs.c`, `ptrs.h` (nouveaux), `entity.c` (calques), `ui.c`, `ui.h`, `msgbox.c`, `collect.c`, `collect.h` (8) | `ptrs` (nouveau) ; `ga_export_panel_by`, `planet_gh_row_lq`, `spec_defaults`, `lq`, `pj`, `fv`, `js`, `ui` (9) | `Makefile` | 18 | ≈ 37 min |
| **a2** | `entity.c`, `entity.h`, `bootstrap.c`, `bg.c`, `economy.c`, `economy_formulas.c`, `economy_score.c`, `economy_tiers.c`, `probe.c`, `score_panel.c`, `slipway.c`, `wormhole.c`, `wormhole.h`, `ui.c`, `ui.h`, `collect.c`, `collect.h`, `entity_geom.c`, `entity_geom.h` (19) | les 31 du groupe a2 | `docs/plan-3.md` (note) | 51 | ≈ 60 min |

- **Pourquoi a2 dépasse ~40 chemins, et pourquoi elle n'est pas coupée** : l'unité indivisible sans cale est « `hv` + `cg` + `mz.ox` + `pico8_ij` » (le filtre de `hv` fabrique `mz.ox`, que `pico8_ij` parcourt avec `cg`) plus **tous** leurs TU lecteurs (30) ; `cv` et `ui.ic` partagent avec eux les aides `listed`/`rank_in` de ces mêmes TU. Couper `cv`/`ui.ic` à part coûterait une seconde passe sur 27 TU avec une aide dédoublée temporaire dans chacun ; couper `hv` de `cg` coûterait un `pico8_ij` dédoublé temporaire. Les 31 éditions de TU sont mécaniques (signature et corps des aides, lectures `->items[i]`), environ 6 à 12 lignes par fichier. Rythme mesuré : D.11c ≈ 23 min pour 37 chemins (≈ 0,6 min par chemin) ; D.11d1..d4 ≈ 21 à 27 min pour 9 à 12 TU **lourdement** réécrits (≈ 2,3 min par fichier lourd). a2 : ≈ 19 fichiers de `src/` (≈ 150 lignes) ≈ 20 min, 31 TU légers ≈ 20 min, preuves ≈ 20 min.
- **Repli écrit** (G) : si a2 atteint son plafond (85 min) sans être finie, rien n'est commis ; `plan-pilot` relance a2 en deux : **a2p — préparation des TU** (sans `src/` : chaque lecture directe d'un élément de registre `->items[i].as.ref` passe par une aide locale existante ou une aide locale d'accès, **aucune ligne comptée touchée**, `liste` = exact, constantes égales ; modèle : 1a.16b, préparation sans conversion) puis **a2s — bascule** (`src/` et les seules aides des TU). Rejeté comme plan principal : il ajoute une passe sur 31 TU et des aides créées pour être réécrites.
- **Pourquoi `ui.le` en a1** : ses lecteurs de TU lisent `le` directement (boucles, `->count`) ; seuls deux appels (`test_pico8_js.c:386`, `:622`) passent par `rank_in`/`listed`, d'où une aide locale transitoire dans `test_pico8_js` (déclarée). `ui.le` et les calques de `dc` donnent au conteneur deux usagers réels dès sa naissance, avec des filets (`pz`, `cj`, `lj`, dessin par calques) sans toucher aux trois registres.
- **Pourquoi `ui.ic` en a2** : ses lecteurs de TU (`bg`, `fq`, `fv`, `ui`) passent par `listed`, aide qu'ils partagent avec `hv`/`cv`/`cg`.
- Rejetés : trois sous-étapes par registre (≈ 85 éditions de TU au lieu de 39, doublons transitoires) ; une seule sous-étape (≈ 1 h 45) ; mettre le conteneur seul en a1 sans usager (code mort une sous-étape).

### Effet sur `make data-residue`

| Catégorie | Départ (H-D11e-3) | Après a1 commit 1 | Après a1 | Après a2 | Pourquoi |
|---|---|---|---|---|---|
| `liste` total | 826 (pas au compteur) | **826 / 99 fichiers** (exact : révélée) | ≈ 784 (≈ −42 : calques 5, `ui.le` `src/` 17, TU ≈ 20) | **≤ 826 − L1**, ≈ 600, rapporté | L1 `src/` ≈ 101 lignes (Annexe C), TU ≈ 110 à 130 |
| `liste` `src/` | 477 | 477 | ≈ 455 | ≈ 376 | Annexe C |
| `liste` `tests/` | 349 | 349 (+0 : littéraux coupés) | ≈ 329 | ≈ 225 | Annexe D |
| résidu D | ≈ 2250 (après D.11e) | + les lignes `liste` seules (≈ +680 à `de6351e`) | rapporté | rapporté | |
| `valeur`, `type-tag`, `ref-audit` | rapportés | inchangés (exact) | baisse `valeur` (déballages `pico8_value_ref(e)`), rapportée | baisse, rapportée | |
| `DATA_RESIDUE_GUARD` | `…,dispatch,par-nom` | inchangé | inchangé | inchangé | `liste` ≠ 0 |

« exact » est une égalité ; « rapporté » est mesuré au commit, écart expliqué ; « ≈ » est une estimation de ce PRD, jamais un critère.

### Documentation (G)

- `docs/plan-3.md` (a2) : note « Découvert à D.12a » en fin de ligne D.12a, rien d'autre. Elle dit : les deux sous-étapes, leurs chemins et commits ; le motif et les comptes révélés de `liste` ; le conteneur (`pico8_ptrs_t`, pas d'étiquette) ; les comptes réels après chaque commit ; `g_ga_by_shared` reporté à D.12b avec `jo.by` ; `mz.ox` et `pico8_ij` passés en D.12a ; les aides `_ptrs` et les appelants que D.12b trouve sur les aides à valeur ; les TU qui ont suivi (34, dont deux §6, façon de lire seulement) et les aides L2 scindées que D.12b/D.12d trouvent ; les includes gardés ; `cart.c` (include sans usage) pour D.12c/D.12d ; pour D.12d, `test_pico8_ptrs` à côté de `test_pico8_list`.
- `docs/glossaire-noms.md`, `docs/trous-de-portage.md` : inchangés (TROU-5 et TROU-7 restent vrais et à leur place ; aucun nom minifié ne change de sens).
- Commentaires : quatre sortes de `CLAUDE.md` ; le contrat du conteneur (propriété de `items` seulement, NULL stockable, `del` du premier identique, tolérance de `all`) est un contrat C que le nom ne dit pas.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256` ; `tools/` hors du commit 1 ; aucun e2e ajouté ni supprimé ; aucun TU supprimé (un ajouté : `test_pico8_ptrs`).
- Toute constante `*_HASH`/`identity[]` de TU.
- Le comportement : mêmes éléments, même ordre, mêmes suppressions, mêmes sauts (TROU-5), mêmes fuites (TROU-7) ; `make e2e` conforme au bit près.
- `pico8_list.c/.h`, `test_pico8_list`, les aides à valeur de `collect` et leurs appelants L2/L3/L4, `g_ga_by_shared`, les listes membres L2 (hors `mz.ox`), les listes hétérogènes L3.
- Les API à valeur (Q12) : `pico8_ij` garde `pico8_value_t *out_nearest`, `pico8_jc_ce` rend une `pico8_value_t`.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `-Wall -Wextra -Werror` rend fatale toute fonction `static` inutilisée : aucun prédicat ni mappeur ne perd son dernier appelant (chacun passe à l'aide `_ptrs`).
- **H2** : aucune constante d'un TU ne change (ordre d'inscription et contenus identiques) ; une constante qui rougit est un bug de conversion.
- **H3** : aucun parcours de registre ne dépend du genre de ses éléments (tous REF) : les tests de genre retirés sont morts aujourd'hui (le `gcov` de départ le montre : branche « pas un REF » jamais prise).
- **H4** : `pico8_pz_step` peut rendre NULL, et `ui.le` peut alors tenir NULL comme il tient aujourd'hui `REF NULL` : le conteneur accepte NULL, `pico8_gu_ptrs(le, NULL)` le trouve comme `pico8_gu(le, REF NULL)` aujourd'hui.
- **H5** : les 34 TU lecteurs de registres sont ceux de l'Annexe D après D.11e (H-D11e-5, -10).

## Implementation Chunks

Deux sous-étapes, dans cet ordre, chacune verte seule (`make test`, `make e2e`, `make sanitize-net`, balayage ASan+UBSan des TU ; `make class-coverage-net` en a2 et en a1 si un constructeur est touché), vérifiée et poussée avant la suivante, avec ses comptes, sa classification, ses lignes gagnées, son `gcov`, ses mutations et son tableau « Runs » :

1. **D.12a1 — compteur, conteneur, calques de `dc`, `ui.le`** (20 chemins). Ordre :
   1. mesure de départ (H-D11e-1, -3, -5 constatées, sinon arrêt et reprise du PRD) ; `gcov` de départ ;
   2. **commit 1** : `tools/data_residue.sh`, `tests/test_data_residue.c` ; `make test` ; mesure ; message ;
   3. `pico8_ptrs.c/.h`, `tests/test_pico8_ptrs.c`, `Makefile` ;
   4. calques de `dc` (`entity.c`) ;
   5. `pico8_gu_ptrs`, `pico8_nv_ptrs` ; `ui.le` (`ui.c/.h`, `msgbox.c`) ; les 8 TU ;
   6. `make test`, `make e2e`, `make sanitize-net`, balayage ASan+UBSan ; `gcov` d'arrivée ; mutations M1–M8 ; mesure, classification, message ; **commit 2**.
2. **D.12a2 — `hv`, `cv`, `cg`, `ui.ic`, `mz.ox`** (51 chemins). Ordre :
   1. mesure de départ ; `gcov` de départ ;
   2. `entity.c/.h` (stockage, `lt`, `he`, accesseurs, source de `dc`) ;
   3. `pico8_jp_ptrs`, `pico8_la_ptrs`, `pico8_ij` ;
   4. parcours de la table « Parcours » (`bootstrap`, `bg`, `economy*`, `probe`, `score_panel`, `slipway`, `wormhole.c/.h`, `ui.c/.h` pour `hd` et `ic`) ;
   5. les 31 TU ;
   6. `make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`, balayage ASan+UBSan ; `gcov` ; mutations M9–M17 ; mesure, classification ; note « Découvert à D.12a » ; **commit**.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (éléments, ordre, présence, effets d'un appel), pas la présence d'une ligne. D.12a change la représentation de six registres sans changer un élément, un ordre ni un pixel. La preuve : un TU du conteneur qui juge ses sémantiques ; les TU existants, qui lisent les registres et plient leurs rangs dans des constantes inchangées ; les 26 e2e au bit près ; `gcov` et mutations ; `sanitize-net` et le balayage ASan+UBSan pour les durées de vie.

### TU touchés (G)

1. **a1, commit 1** : `test_data_residue` (cas `liste`, littéraux coupés).
2. **a1, commit 2** : `test_pico8_ptrs` (nouveau) ; les 8 TU de `ui.le` (table « Lectures de registres dans les TU ») : type du pointeur, élément `void *`, constantes inchangées.
3. **a2** : les 31 TU du groupe a2 ; aides scindées listées.
4. **`test_pico8_ptrs`** (prior art : `test_pico8_list`, 7 cas) : `add` en ordre et croissance au-delà de 4 ; `del` du premier identique (deux pointeurs égaux : le premier part) ; `del` absent sans effet (rend 0) ; `del(NULL)` retire le premier NULL ; `all` dans l'ordre ; `all` qui tolère la suppression de l'élément courant (et pas d'un précédent : le cas documenté) ; `all` sur vide ; `foreach` une fois chacun, dans l'ordre, `ctx` passé. Rien de lent.
5. **Constantes** : aucune `*_HASH`/`identity[]` ne change, à aucun commit.
6. **Assertion nouvelle** : admise seulement pour couvrir une ligne ajoutée non exécutée ou pour faire rougir une mutation qui ne rougit rien (trou), dans un test gardé du module, listée au message avec la ligne qu'elle couvre ; rien de lent, aucun fichier d'événements.
7. **Prior art** : registres et ordre plié en constantes (`rank_in(pico8_hv(), …)` dans 20 TU) ; tolérance de `all` (`test_pico8_list`) ; TU suiveurs dans le même commit (D.10b, D.11c) ; lignes de lien du `Makefile` (D.11e, règle de lien).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`), lancés comme le `Makefile` ; union des lignes de `src/` exécutées.

- Chaque ligne **ajoutée ou modifiée** de `src/` (conteneur, aides `_ptrs`, `ij`, parcours) est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée (règle 6), sinon justifiée au message (branche NULL d'un prédicat, attendue morte : H3).
- `L` = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide**, hors les tests de genre retirés (qui ne survivent pas). Une ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail, fichier sauvegardé par `cp` avant et restauré par `cp` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan, `test_pico8_planet_gh_row_lq`) ; chacune doit rougir au moins le TU nommé, sinon c'est un trou, couvert par une assertion (règle 6) ou justifié (mutation équivalente, démontrée) :

- a1 :
  - **M1** `pico8_ptrs_del` retire le **dernier** identique → `test_pico8_ptrs` ;
  - **M2** `pico8_ptrs_iter_next` sans tolérance (toujours `index + 1`) → `test_pico8_ptrs` ;
  - **M3** `pico8_ptrs_foreach` saute le dernier → `test_pico8_ptrs` ;
  - **M4** `pico8_dc` range dans `layers[15 - layer]` au lieu de `layers[layer - 1]` → un TU qui dessine par `dc` (`test_pico8_planet_kj_dispatch`, `test_pico8_bg`…), sinon `make e2e` rouge, sinon assertion ;
  - **M5** `pico8_pz` n'ajoute plus la rangée de tête `ep` à `le` → `test_pico8_pj` (« l'en-tête `ep` puis une rangée par… ») / `test_pico8_js` ;
  - **M6** `pico8_gu_ptrs` rend toujours 0 → `test_pico8_ui` ou `test_pico8_js` (clic sur un bouton du panneau), sinon assertion ;
  - **M7** `pico8_nv_ptrs` n'appelle plus `kp` → `test_pico8_js`/`test_pico8_fv` (les boutons de `le` restent dans `hv` après `cj`) ;
  - **M8** `pico8_ui_init` ne vide plus `g_shared_le` → `test_pico8_spec_defaults`/`test_pico8_ui`.
- a2 :
  - **M9** `pico8_lt` n'inscrit plus dans le seau `hd` (`i == 2`) → `test_pico8_bg` (`listed(pico8_cv("hd"), bg)`) / `test_pico8_ui` ;
  - **M10** `pico8_he` ne retire plus de `hv` → `test_pico8_iv` (le toast sort de `hv`) / `test_pico8_mz` ;
  - **M11** `pico8_dg` commence à l'index 1 → `test_pico8_iv`, `test_pico8_mz` (TROU-5) ;
  - **M12** `pico8_jp_ptrs` garde les éléments que le prédicat refuse → `test_pico8_mz` (`test_ox` : `ox` exclut `first` et `unlit`) / `test_pico8_hy` (`cz`) ;
  - **M13** `pico8_la_ptrs` oublie `b` → `test_pico8_jc` (`jc:ce` vers un `jc` quand aucun `np` ne convient) ;
  - **M14** `pico8_ij` rend le premier élément au lieu du plus proche → `test_pico8_mz` / `test_pico8_g_` (cible de la sonde) ;
  - **M15** `pico8_ui_gw` range tout dans `layers[0]` → `test_pico8_ui` (priorité de `jz`, calques 15 → 0) ;
  - **M16** `pico8_lt` n'inscrit plus dans `cg` → `test_pico8_bg` (`LDOWN_G_AFTER`) / `test_pico8_jo` ;
  - **M17** `pico8_hy_hk` n'appelle plus les `hk` de `cv("hk")` → `test_pico8_hy` (ou le TU d'une classe à `hk`), sinon assertion.
- Cibles vérifiées à `de6351e` : `entity.c:222-341`, `bootstrap.c:63-91`, `collect.c:53-132`, `entity_geom.c:87-114`, `ui.c:247-520`, `msgbox.c:352-428`, `wormhole.c:62-104` (⟲ pour toutes sauf `collect.c`). Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit, commit 1 compris. Aucun e2e ajouté.
- `make sanitize-net` à chaque sous-étape (ligne du plan : « oui »).
- `make class-coverage-net` : a2 (constructeur `pico8_ui_struct_new` touché) ; a1 seulement si son diff touche un constructeur (attendu : non ; constaté sur `git diff`).
- **Balayage ASan+UBSan des TU** à chaque sous-étape : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.7b, D.10a, D.11a, D.11c, D.11e.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 1 h 45 de temps réel pour l'étape** (a1 ≈ 45 min dont commit 1 ≈ 8 min, a2 ≈ 60 min), mesures, `gcov`, mutations et filets compris ; **plafonds** a1 70 min, a2 85 min (au-delà : repli a2p/a2s). Base mesurée : D.11c ≈ 23 min pour 37 chemins, D.11d1..d4 ≈ 21 à 27 min pour 9 à 12 TU lourds, D.11e (à relever dans sa note). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute (`pico8_entity.h` et `pico8_ui.h` changent : recompilation large) |
| `make e2e` | ~1 s |
| `make sanitize-net` | de l'ordre de la minute |
| `make class-coverage-net` | quelques dizaines de secondes (a2) |
| `make line-coverage EVENTS="tests/e2e/*.json"` | moins d'une minute, si une ligne ajoutée n'est pas couverte par les TU |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes par sous-étape |
| mutations (a1 : 8, a2 : 9 ; un `make` ciblé chacune) | < 8 min par sous-étape |
| balayage ASan+UBSan des TU | quelques minutes par sous-étape |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà d'un plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; une sous-étape inachevée n'est pas commise. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + sous-étape + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | `<HEAD gradé D.11e>`, avant a1 | 0 | `par-nom` absent | … |
| 2 | recette `liste` | idem | 0 | `liste 826 lignes 99 fichiers 870 occ.` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que les chemins de sa ligne de la table « Empreinte » (plus un TU suiveur listé). Commit 1 : exactement `tools/data_residue.sh` et `tests/test_data_residue.c`. Ni `tests/e2e/`, ni `*.sha256` ; `tools/` seulement au commit 1 ; le `Makefile` seulement au commit 2 de a1, sur les lignes de « `Makefile` et liens ».
2. **Commit 1** : `git diff <parent> -- tools/data_residue.sh` ne montre que la ligne `CATEGORIES` et le bras `liste` ; le motif est **identique** caractère pour caractère à celui de la ligne D.12a ; les comptes des neuf catégories existantes sont égaux avant/après ; `test_data_residue.c` n'a aucune ligne comptée.
3. **Greps** de la « Sortie », exactement, après a2 ; après a1 : aucune `pico8_list_t` pour `le`, `g_shared_le`, `layers[15]` (`dc`) dans `src/` ; `pico8_gu(`/`pico8_nv(` absents de `ui.c`.
4. **Conteneur** : relu contre `pico8_list.c` (croissance, décalage de `del`, tolérance de `all`, `foreach`) ; aucune étiquette ; nom hors de tous les motifs (grep 3).
5. **Casts** : chaque conversion d'un élément vers une struct précise, ajoutée ou gardée dans une fonction touchée, est précédée dans la même fonction du test qui la précédait (famille, classe, `assert`) ; aucun cast de pointeur de fonction ; aucun `typedef` ni macro qui renomme `pico8_list_t`.
6. **Liens** : greps 6 et 7 ; `pico8_ptrs.c` dans `ENGINE_SRCS` et `DATA_ENGINE_SRCS`.
7. **Forme des parcours** : chaque `all()` est resté un itérateur, chaque boucle à index une boucle à index (`dg` : TROU-5) ; ordre 15 → 0 de `jz` gardé.
8. **Constantes** : aucune `*_HASH`/`identity[]` changée (`git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)'` vide).
9. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (c) = 0, (b) limitée à la liste ; lignes gagnées et lignes comptées modifiées listées ; aucun alias, registre, cale.
10. **Écarts déclarés** : `g_ga_by_shared` intact (grep 5) ; `mz.ox` et `pico8_ij` convertis ; aides à valeur gardées avec exactement leurs appelants non L1 (table « Aides communes ») ; §6 : façon de lire seulement.
11. **Preuve** : `gcov` (lignes ajoutées exécutées ou justifiées ; `L` vide), mutations restaurées par `cp`, `sanitize-net`, `class-coverage-net` (a2), balayage ASan+UBSan rapportés.
12. **Commentaires** : quatre sortes de `CLAUDE.md` ; les contrats devenus faux (« PICO8_VALUE_REF values whose .as.ref… », `entity.h:24-26`) réécrits ; aucun renvoi à D.12a ni au plan dans `src/` ou `tests/`.
13. **Docs** : note « Découvert à D.12a » en fin de ligne D.12a (a2) ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le commit qui gradue D.11e (ou un descendant qui ne touche que `docs/`), propre ; ou si `par-nom` n'y est pas à 0 et gardé ; ou si `liste` (recette) n'y vaut pas 826 lignes / 99 fichiers sans explication (H-D11e-3).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change **dans son intention** (constantes, cas, garde) ; changer sa façon de lire `ok.le` est attendu et déclaré.
- `FAILED:` si un registre ou un parcours ne tombe sous aucune forme de ce PRD ni du §5 (arrêt « champ sans règle »), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si `make sanitize-net` ou le balayage ASan+UBSan des TU n'est pas propre, à l'une des sous-étapes ; ou si `make class-coverage-net` n'est pas identique en a2 (ou en a1 si un constructeur est touché).

**Périmètre et forme**

- `FAILED:` si le commit 1 contient autre chose que `tools/data_residue.sh` et `tests/test_data_residue.c`, change un motif existant, ou énonce un motif différent de celui de la ligne D.12a ; si `tools/` change après le commit 1 ; si le `Makefile` change ailleurs que sur les lignes de « `Makefile` et liens ».
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit (après a2), ou si les greps partiels du contrôle 3 échouent après a1.
- `FAILED:` si le conteneur porte un nom qui correspond à un motif du compteur, porte une étiquette de type sans que H-D11e-4 soit tombée, ou s'il est un alias (`typedef`, macro) de `pico8_list_t`.
- `FAILED:` si `g_ga_by_shared` ou `jo.by` change ; si une aide à valeur de `collect` perd un appelant non L1 ou garde un appelant L1 ; si une liste L2 autre que `mz.ox`, une liste L3 ou une liste L4 est convertie.
- `FAILED:` si un `all()` devient une boucle à index, ou l'inverse ; si l'ordre de `dg` ou de `jz` change.
- `FAILED:` si une conversion d'élément vers une struct précise n'a pas son test dans la même fonction, ou si un pointeur de fonction est casté.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `liste` après a2 n'est pas ≤ 826 − (lignes L1 retirées, recomptées au message) ou si l'écart à l'estimation n'est pas expliqué ; si `liste` `tests/` change au commit 1.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, registre, renommage d'un identifiant vivant pour échapper aux greps, commentaire compté réécrit hors de son sujet, cale valeur ↔ conteneur, littéral remplaçant un identifiant compté, (c) non nulle, (b) hors de la liste.
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées ; la justification de chaque Δlignes > Δoccurrences ; les includes gardés ou retirés ; les aides de TU scindées ; le rapport `gcov` ; les mutations. Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne ajoutée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` n'est pas vide.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si un plafond est dépassé sans note (et sans repli a2p/a2s pour a2), ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.12a », ou si elle omet l'un de ces éléments : les comptes réels (motif et comptes révélés compris), le conteneur, `g_ga_by_shared` reporté, `mz.ox`/`pico8_ij` avancés, les aides `_ptrs` et ce que D.12b trouve, les TU suiveurs (§6 compris), ce que D.12c/D.12d trouvent.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- `g_ga_by_shared` et `jo.by`, et toutes les listes membres L2 hors `mz.ox` (`jo.oe`, `jo.cc`, `trade.lp`/`rl`, `ib`, `qy`, `jc.ts`, `hy.invented`, `jo.jy`/`hp.i`, `gen.oc`) : D.12b.
- Les aides à valeur de `collect` (`pico8_jp`, `pico8_la`, `pico8_gu`, `pico8_gu_content`, `pico8_nv`, `pico8_la_append`) et leurs appelants non L1 ; le nom définitif des aides `_ptrs` : D.12b.
- Les listes hétérogènes L3 (`lq.items`, `mh.items`, `jb.ls`, `lq.fs`) et les listes locales L4 d'affichage ; l'include orphelin de `cart.c` : D.12c.
- `pico8_list.c/.h`, `test_pico8_list`, `PICO8_REF_AS_LIST*`, `PICO8_TYPE_TAG_LIST`, les TU restants de `liste` : D.12d.
- Les API à valeur (`pico8_ij` `out_nearest`, `pico8_jc_ce`, `pico8_probe_truthy`…) et les catégories `valeur`, `type-tag`, `ref-audit` : Q12 (étude D.9b), ouverte.
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **Catégorie** : commit 1 = script + son TU, motif recopié ; `liste` dernière de `CATEGORIES`.
  - **Conteneur** : `pico8_ptrs_t` (`void *`, sans étiquette, NULL stockable), sémantiques de `pico8_list` à l'identique ; son propre TU.
  - **Aides** : quatre aides `_ptrs` pour les seuls parcours de registres ; `pico8_ij` converti en place ; aides à valeur gardées pour leurs autres appelants.
  - **Périmètre** : `g_ga_by_shared` reporté à D.12b (type de `jo.by`) ; `mz.ox` avancé (fabriqué par le filtre de `hv`, parcouru par `ij` avec `cg`).
  - **TU** : suivent dans le même commit (premier terme de la règle l. 267) ; pont rejeté ; deux §6 changent leur façon de lire `ok.le`.
  - **Includes** : retirés des `.c` qui n'en ont plus besoin ; gardés dans les en-têtes dont d'autres fichiers dépendent par transitivité.
  - **Découpage** : deux sous-étapes (≈ 45 et ≈ 60 min) ; a2 au-dessus de ~40 chemins parce qu'indivisible sans cale ; repli a2p/a2s écrit.
- **Surprises de mesure, pour `plan-pilot`** : (1) l'empreinte de la ligne (« ≈ 15 (15, 0) ») vient de `2dec71f` : remesurée, 22 chemins de `src/` et 35 TU ; `cart`, `galaxy`, `hud` n'ont rien à changer, et `probe`, `score_panel`, `slipway`, `wormhole`, `msgbox`, `collect`, `entity_geom` s'ajoutent ; (2) **34 TU lisent directement les registres** (`pico8_hv()` dans 29, `pico8_cv()` dans 27, `pico8_cg()` dans 24 ; 65 aides locales à `const pico8_list_t *`) : le « 0 » de TU de la ligne est intenable sans pont, et un pont serait une cale ; (3) `g_ga_by_shared` n'est pas un registre : c'est le défaut partagé de `jo.by` (L2) ; (4) `mz.ox` (L2) est le produit direct d'un filtre de `hv`, et `pico8_ij` lie `cg` et `mz.ox` ; (5) deux tests §6 lisent `ok.le` : « §6 : aucun » est faux par lecture (façon de lire seulement) ; (6) `liste` = 833 à `de6351e` (pas 927 : D.11d a retiré 82 lignes de TU), 826 attendu après D.11e ; (7) `test_data_residue.c` doit juger la catégorie : le « commit seul » contient deux fichiers ; (8) `pico8_collect.c` gagne une dépendance au conteneur : la ligne de lien du §6 `layout_nil_item_draws_nothing` et celle de `main_raylib` changent (le test, non).
- **Pour D.12b** : les aides à valeur `pico8_jp` (3 appelants), `pico8_la` (6), `pico8_gu` (1), `pico8_nv` (2), `pico8_gu_content` (1) ; le nom définitif des aides `_ptrs` ; `g_ga_by_shared` avec `jo.by` ; les aides locales de TU scindées sur `jo.oe`, `jc.ts`, `qy`.
- **Pour D.12c** : l'include sans usage de `cart.c:25` ; `pico8_nv` de `layout` (largeurs fixes : jamais un conteneur de pointeurs).
- **Pour D.12d** : `test_pico8_ptrs` existe ; « `test_pico8_list` réécrit sur le nouveau conteneur » peut devenir « ses cas restants absorbés par `test_pico8_ptrs` » ; les includes gardés dans `entity.h`, `ui.h`, `wormhole.h`, `entity_geom.h` tombent avec `pico8_list.h`.
- **Inventaire exhaustif** : Annexe C (`src/`), Annexe D (`tests/`), numéros à `de6351e`.

## Annexe A — textes du plan recopiés (`docs/plan-3.md` à `de6351e`, par extraction de lignes)

### §5, en-tête et règles de la phase 1a (l. 74-92)

> ## 5. Phases
>
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
>

### §5, règles de la phase 1b (l. 151-168)

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
>

### §5, phase D, en entier (l. 228-280) : puces, note de D.0, « Liste de D » avec empreinte, règles communes et ordre, arrêt « champ sans règle », décisions de D.9, comptes au départ de D.10, lignes D.2 à D.12d avec toutes leurs notes « Découvert à D.x » (D.11d1 à D.11d4 comprises ; la note « Découvert à D.11e » n'existe pas encore à `de6351e` : à recopier à la graduation)

> ### Phase D — Données
>
> - Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
> - Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
> - Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.
> - **D.0 — mesure seule, sans conversion, sur le modèle de 1b.0** (ajouté à la revue de sortie de 1b) : un compteur nommé (`make data-residue`, `tools/data_residue.sh`) compte, par catégorie, dans `src/` et `tests/`, les usages de `pico8_value_t`/`pico8_value_*()`, `pico8_ob_table_t`/`pico8_ob*()`, `pico8_list_t` si le §6 le retire, `pico8_type_tag_of` et `pico8_ref_audit.h` — dont les lecteurs/écrivains par nom (`pico8_entity_get_field`/`set_field`, `*_truthy`, `field`/`set_field` des descripteurs et leurs aides `*_take_*`), le dispatch à arguments `pico8_value_t`, les `overrides` des constructeurs, les spécifications de classe lues par `pico8_ob()` et leurs `pico8_X_default`. Sa sortie vide **est** la sortie de la phase D, et chaque ligne de la liste porte son « Compte : N ». D.0 met aussi `make value-members` et `grep -rn offsetof src/` dans `make test`, en mode vérification (code non nul si non vide), à côté de `float-guard` ; une catégorie du compteur tombée à 0 y entre de même. La liste ordonnée de D est écrite après D.0, à partir de sa sortie : une catégorie par groupe nommé, découpée au-delà de ~40 fichiers ; chaque test de régression du §6 qui lit par nom est rattaché au groupe qui force sa réécriture, avec sa preuve rouge. Une catégorie qu'aucune puce de D ne couvre est l'arrêt « champ sans règle » du §7. Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.
> - **D.1 — suppression du recensement**, premier groupe après D.0 : `pico8_member_census.c/.h`, les crochets `SEE`/`READ`/`WRITE`/`CHILD_EXIT`/`STOP` de `src/` et `tests/`, `tools/member_census.*`, la cible `member-census` et ses entrées de listes de sources, le mode `--c-table` de `tools/entity_value_members.sh`. Les messages d'`assert` qui renvoient à « census row » / « plan-3 section 5 » sont réécrits pour nommer le contrat violé. Aucun `.sha256` ne change ; `make class-coverage-net` identique.
>
> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.
>
> - D.2 — `stash` : tables décodées par `unstash` — `g_hc`, `g_fd`, `g_dr`, `g_nb` (`data_init.c`, avec la passe `nb_mr` et son routage `gp={"nk"}`), `g_r`, `g_iz` (`pico8_stash.c`, lus par `pico8_r()`/`pico8_iz()`) → `static const` générés (première puce) ; le test ponctuel naît ici : chaque table générée est égale, entrée par entrée, à `pico8_ob(pico8_unstash(mem, pd))`, la mémoire étant chargée par `pico8_load_map_json` (pas de rejeu). `pico8_stash.c/.h` et `test_pico8_stash.c` restent jusqu'à D.8. Empreinte 22 (15, 7). Compte : `stash` = 26 (`pico8_stash.c` 7, `pico8_stash.h` 7, `test_pico8_stash.c` 12) ; `ob` ≤ 2195. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.
> - D.3 — `ob`, tables constantes parsées au démarrage (liste de la première puce) : `g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb` (`game_data.c`), `g_ek`, `g_df` avec sa passe `df_mr` (`data_init.c`), `g_ci` (`entity.c`), `g_ix`/`g_ha` (`menu.c`), `g_bo` (`help.c`), `g_ef`/`g_bz` (`score_panel.c`) → `static const` générés, test ponctuel étendu. Empreinte 21 (17, 4). Compte : `ob` ≤ 2105. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).
> - D.4a — `ob`, membres table de l'arbre `jo` : `jo.nu`/`ka`/`dz`/`cq`/`kw`/`em`/`nk`, `qh.px`/`ba`, `hj.k_`, `ga.k_`, `nr.hz`, `mx.ht` → struct ou tableau fixe par membre (troisième puce), forme tranchée par le PRD (le masque de bits de `nu` est la phase 2) ; leurs défauts table (`jo_default_table`, `family_default_table`, `family_trade_default_table`) sont générés `static const` dans la même étape (première puce). Empreinte 34 (22, 12). Compte : `ob` ≤ 2033. §6 : `gh_row_lq` (sa table `kw`, l. 102-120) et `ga_export_panel_by` (sa table `kw`, l. 98-117) construisent le nouveau `kw`, preuve rouge dans ce commit. `class-coverage-net` : oui (init des constructeurs). Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).
> - D.4b — `ob`, autres membres table : `bg.pl` (et `g_bg_pl`), `hy.po`/`fu`, `fv.rb`, `mh.nu`, `g_.nu`/`k_`, `hp.mr`/`gp`/`fk`/`lq`, `pj.gs`/`i_`, `qb.pc` (et `g_qb_pc`), `jc.nu`/`k_`/`hg`, `ui.ic`, `js._cr_t`, `mz.nu`/`kk`, même règle. Empreinte 39 (27, 12). Compte : `ob` ≤ 1982. §6 : aucun. `class-coverage-net` : oui. Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp` : `lq_own`/`lq_inner`) ; `hp.mr`/`gp` en `pico8_str_seq_t`, `ho_apply` les prend typés ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages `po_own`/`fu_own` dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie ; une clé `qe` chaîne est absente) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus, `pico8_jc_hg_add`/`_has`) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`, qui partage son calcul avec `pico8_gi`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : 1882 (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de −100 (1882 − 1982).
> - D.5a — `spec`, `ke` et les classes d'interface : `g_X_spec`, `pico8_X_default`, `pico8_X_defaults_reset` et aides `X_default_<genre>` de `ke`, `bg`, `ui`, `js`, `fq`, `jb`, `lq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`, avec les appels `pico8_ke_default` des autres fichiers → défauts `static const` générés depuis `pico8_ob(spec)`, test ponctuel étendu. Lecture de la première puce : une spécification de classe est une table constante parsée au démarrage. `test_pico8_spec_defaults.c` et `test_pico8_jo_spec_defaults.c` lisent les défauts statiques, même intention. Empreinte 30 (21, 9). Compte : `spec` ≤ 779. §6 : aucun. `class-coverage-net` : oui. Découvert à D.5a : deux chunks (interfaces, puis `ke` ; ≈ 40 chemins, `pico8_game_data.*`, `pico8_bootstrap.c` et les TU qui plient l'identité de `me` hors du majorant). Défauts générés `pico8_X_start` (type `pico8_X_start_t`, `pico8_game_data.h`), un par classe à clés (`ke`, `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`), clés propres de la spécification, lus par les constructeurs ; les tables gardent l'objet de D.4a/D.4b ; `mz.jj` s'initialise de `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` : liste statique de `ui.c` (`pico8_ui_le_shared`) ; `qb.v`/`p`/`ra`, `iv.v`, `ui.hw` pointent la donnée générée (membres `const`) ; `nf.ra` pointe le statique non `const` `g_nf_ra`, affecté depuis `pico8_nf_start.ra` à `pico8_nf_init` (repli : `test_pico8_entity_kind` et `test_pico8_ie` écrivent `je.ra` en place) ; `nf.l`/`nf.r` restent `g_nf_l`/`g_nf_r` (D.7b). Le cycle `_init`/`_shutdown`/`_ready` des treize modules reste, sur un drapeau. `pico8_ke_start_field` (compté par `par-nom`) remplace `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_g__default`, `pico8_ie_default` (retirés à D.5b) et `pico8_jo_default` (D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`) : D.5c le supprime. `pico8_rk_default` (variante de dessin, comptée par `spec`) est à D.5b. `spec` après D.5a : 634 (au lieu de ≤ 779) ; les majorants `spec` suivants se décalent de −145 (634 − 779). `par-nom` : 1090.
> - D.5b — `spec`, `g_`/`fv` (aide), `hp`, `ie` (et `ie_chain`), `pj`/`rk`, `hy`, `trade`, même règle. Empreinte 23 (16, 7). Compte : `spec` ≤ 547. §6 : aucun. `class-coverage-net` : oui. Découvert à D.5b : deux chunks (exclusion de `rk`, puis les sept classes ; 29 chemins, `pico8_game_data.*` et trois TU hors du majorant). `pico8_rk_default` (variante de dessin à argument omis) entre dans `SPEC_EXCLUSION`, preuve : ses quatre lignes ne portent aucune autre catégorie. Défauts générés `pico8_X_start` de `g_`, `fv`, `hp`, `ie`, `pj`, lus par les constructeurs ; `hy`, `trade` sans objet. `ie` : un seul défaut pour la chaîne et les objets prêtés, `kq`/`ra`/`eh` pointent la donnée générée, plus libérés à l'arrêt. `fv.hi`/`gq`/`kq`, `g_.ra`, `ie.kq`/`ra`/`eh`, `pj.cr.hi`/`gq` : membres `const` vers la donnée générée, aucun repli. `pj.cr` : `g_pj_cr` reste (son `ep` est muté), valeurs de `pico8_pj_cr_start`. `pj.ou`/`pj.ea` : rangées générées `pico8_pj_ou_row`/`pico8_pj_ea_row`, d'où `pico8_score_panel_init` construit une fois par init les tables d'état `g_pj_ou`/`g_pj_ea` (`pico8_jb_row_view`), jamais libérées comme celles de la spécification ; leurs lecteurs (`hb`, `hk`, `kj`, `pico8_mh_new`) restent des tables ob (D.6b, D.7a). `test_pico8_spec_defaults` n'a plus de registre ni de constante. `pico8_ke_start_field` n'a plus que `pico8_jo_default` et le cas `M_KE`. `spec` après D.5b : 430 (au lieu de ≤ 402 après le décalage de D.5a : 28 lignes des sites de D.5b, retirées par D.4a/D.4b, étaient comptées à la fois dans le majorant 547 et dans ce décalage ; majorant corrigé 430) ; le « = 0 » de D.5c est inchangé. `par-nom` : 1085.
> - D.5c — `spec`, arbre `jo` : `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, aides `family_default_*` et `family_trade_default_*`, même règle ; le `field` des sortes (`pico8_entity_kind_t`, `g_qh_kind` … `g_mx_kind`) lit les défauts statiques sous un nom hors motif, compté par `par-nom`. Empreinte 19 (6, 13). Compte : `spec` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. Découvert à D.5c : deux chunks (données, puis conversion et garde ; 27 chemins, `pico8_game_data.*`, `ke.c/.h`, le test ponctuel et le `Makefile` hors du majorant ; le lien du test ponctuel prend `pico8_planet_family_trade.c`, pour interroger `nr`/`fr`/`mx`). Défauts générés `pico8_X_start` des dix classes de l'arbre, clés propres, lus par les constructeurs, l'héritage par le `struct_init` de la parente ; chaînes dans des tableaux distincts, un par jeton (des littéraux se fusionneraient, TROU-3). `jo.kq`/`gq`, `pm.ng` : membres `const` vers la donnée générée, aucun repli. Le `field` des sortes est `pico8_X_kind_field` : `rr`, `s`, `me`, `od` seulement, lus dans `pico8_X_start`, puis la parente jusqu'à `pico8_jo_kind_field` (seul déclaré, `planet.h`) ; 0 si le module n'est pas prêt ; compté par `par-nom` (1106 après D.5c), à typer par D.9. `pico8_ke_start_field` supprimé. `_ready` sur `g_jo_up`, `g_family_up`, `g_family_trade_up`. `ga.by` : `g_ga_by_shared`, `pico8_list_t` statique non `const` vidée à chaque init (précédent `ui.le`), contenu à typer par D.7a. `spec` après D.5c : 0, dans `DATA_RESIDUE_GUARD`. `ob` : 1721 hors test ponctuel.
> - D.6a — `overrides`, constructeurs de l'arbre `jo` (`pico8_jo_new` … `pico8_mx_new`), `new_entity` de `pico8_entity_kind_t` (`pico8_struct.h`) et ses appelants (`galaxy.c`, `jo_kv`…) : la table de clés de construction devient une struct locale (troisième puce), paramètre renommé. Empreinte 11 (8, 3). Compte : `overrides` = 315. §6 : `gh_row_lq`, commentaire de la l. 2 seulement ; aucune lecture ne change, pas de preuve rouge. `class-coverage-net` : oui. Découvert à D.6a : un commit (30 chemins : `research_icon.c` pour `ie:ldown`, `pico8_game_data.*` et quatorze TU qui construisent dans une variable `b`, hors du majorant). Clés typées : `pico8_jo_keys_t` (`id`, `p`, `ra`, NULL = absent, `pico8_struct.h`) pour neuf classes et `new_entity` ; `pico8_ga_keys_t` (`jo` et `pq`, NULL = défauts de `ga` ; `k_` NULL sous `pq` = nil) ; la chaîne que `pm:pq` parsait devient `pico8_pm_ga_pq_keys` (`pico8_ga_pq_keys_t`, générée) ; la sorte `ga` passe par une adaptatrice (`pico8_ga_kind_new`, `pq` NULL). `pico8_X_new(NULL)` inchangé ; les classes « seulement `id` » (`qh`, `nl`, `os`, `hj`, `fr`, `mx`) l'assertent sur `p`/`ra`. Tests de mort d'une clé positionnelle, sans membre ou hors rangée à la construction retirés (le type les interdit). `pico8_ob.h` quitte `planet.h`, `planet_family.h`, `planet_family_trade.h` ; `pico8_struct.h` le garde (`slipway.h`, `wormhole.h` en dépendent par lui). Le commentaire l. 2 de `gh_row_lq` parle des clés de `js` : il passe à D.6b (le changer ici serait un renommage à type constant). `overrides` après D.6a : 329 lignes / 348 occurrences / 55 fichiers (au lieu de 316 : le « = 315 » de D.1 était le total des fichiers de l'empreinte, dont 30 lignes de D.6b/D.6c — `iv`/`jc` de `planet.c` et `planet_family_trade.c`, `jc` de `test_pico8_mx`, `gh_row_lq`, `ie` de `test_pico8_entity_kind` — et sans 16 lignes de D.6a — `ie:ldown`, `nr` de `test_pico8_mz`/`test_pico8_bg` ; la ligne de `k_` ajoutée par D.4a est dans le périmètre) ; le compte de D.6b est à reprendre : sur les 329 lignes, 232 sont à D.6b (lignes classées par le constructeur appelé ou déclaré, commentaires de `lq`/`jb`/`gh_row_lq` compris, `iv` d'`economy.c` et `jc` des TU `hp`/`hy`/`trade`/`ui`/`mx`/`mz` compris) et 97 à D.6c : « `overrides` = 97 » après D.6b au lieu de 117 ; le « = 0 » de D.6c est inchangé. `par-nom` : 1075.
> - D.6b — `overrides`, mise en page et entités d'écran : `lq`/`jb` (`layout`), `js`/`fq` (`ui_button`), `mh`/`iv` (`msgbox`), `ui`, `qb` (`scroll`), `nf` (`mouse_reader`), `bg`, `mz` (`wormhole`), `jc` (`slipway`), appelants de `menu.c`, même règle. Empreinte 30 (19, 11). Compte : `overrides` = 117. §6 : `layout_nil_item_draws_nothing` (l. 106-112) passe son item par la struct de clés de `pico8_lq_new` ; l'item reste un `pico8_value_t`, élément de `lq.items` ; même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : oui. Découvert à D.6b : trois chunks (classes sans clé ; `iv`/`jc`/`fq` ; `lq`/`jb`/`js`/`mh` avec les ponts, `dr` et le §6 — 58 chemins : appelants à NULL, `economy.c`, `score_panel_display`, `research_display`, `data_init` et les TU qui construisent dans `b` hors du majorant). `bg`, `nf`, `qb`, `ui`, `mz` sans paramètre ; clés typées `pico8_iv_keys_t`, `pico8_jc_keys_t`, `pico8_fq_keys_t`, `pico8_lq_keys_t` (items `pico8_value_t`), `pico8_js_keys_t` (`lq`/`h_`/`jb` restent des sources de rangée `pico8_value_t`, présence TROU-23), `pico8_mh_keys_t` ; `jb` prend `const pico8_jb_row_t *`. `js:pq` copie une rangée par `pico8_lq_keys_of` et une liste par ses éléments (plus de vue ni de conversion) ; `jb:pq` n'emballe plus ses lignes chaîne ; `fq(de)` lit `pico8_de_my()`. Pont `pico8_dr()` fermé (`pico8_jb_new(&pico8_dr_table[i])`). Ponts nouveaux, paramètre `overrides` gardé (type inchangé, comptés, 21 lignes) : `pico8_lq_new_table`, `pico8_jb_new_table` (`layout.c`), `pico8_js_new_table` (`ui_button.c`), `pico8_mh_new_table` (`msgbox.c`) — fermés quand les rangées de `pz` et les sources `lq`/`h_`/`jb`/`mh` sont typées (`planet_display`, `menu`, `score_panel_display`, `research_display` : D.7a ; `help`, `msgbox` : D.7b ; le « 0 dans `layout` » de D.7a exige les deux premiers). Les TU d'un pont gardent leur table : `test_pico8_mh` (clé `lh` nil, clés refusées : 7 lignes `overrides`, table de même type et même nom) et `test_pico8_lq` (deux commentaires qui nomment la table passée au pont) tombent avec lui. Reportés, avec eux : `pico8_ix()`/`pico8_ha()`, `g_bo`, `pico8_bz()` (vues par init ; `ix` porte un item imbriqué que seule une vue sait donner à `lq:mn`), les vues à l'appel de `hp.lq` et de `kw`. `pico8_lq_table_view` sans appelant de `src/` ; une table qui porte `fs` ou `w` est refusée par le pont `lq` (aucun producteur n'en pose). `overrides` après D.6b : 127 lignes / 138 occurrences / 27 fichiers (97 lignes de D.6c + 21 des ponts + 9 des TU des ponts ; le « = 117 » de D.1 était un total de fichiers, qui comptait dans ceux de D.6c 23 lignes de D.6b — `iv` de `economy.c`, `jc` de `test_pico8_hp`/`_hy`/`_trade`) ; le « = 0 » de D.6c devient « = 30 » (les ponts et leurs TU), et `overrides` entre dans `DATA_RESIDUE_GUARD` à l'étape qui ferme le dernier pont. D.7a hérite d'un chantier plus lourd : fermer les ponts `lq`/`jb` demande de typer les producteurs de rangées de `planet_display`, `menu`, `score_panel_display`, `research_display` et les rangées `bs` de `pz` (fichier de D.7b) ; son PRD tranche l'ordre ou la fusion avec D.7b. `par-nom` : 1062.
> - D.6c — `overrides`, reste : `g_` (`probe`), `fv` (`help`), `ie` (`research_icon`, `cart.c`), `hp` (`research`), `hy` (`economy`), `trade` (et `planet_trade.c`), `pj` (`score_panel`), même règle. Empreinte 21 (16, 5). Compte : `overrides` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. Découvert à D.6c : deux chunks (`hy`/`fv`/`pj`/`g_`/`ie`/`trade`, puis `hp` et le pont `fd` — 37 chemins : appelants à NULL, TU qui construisent dans `b`/`o`, `data_init`, `collect` et le test ponctuel hors du majorant). `hy`, `fv`, `pj` sans paramètre ; clés typées `pico8_g__keys_t`, `pico8_ie_keys_t`, `pico8_trade_keys_t` (triple `qe` du membre ; TROU-24 au site de `trade:kg`), `pico8_hp_keys_t` (`ri` : enregistrement `pico8_fd_entry_t`, `rr`) ; `hp` applique `ho` aux champs typés (`pico8_ho_gp_has`, extraite de `collect.c`). Pont `pico8_fd()` fermé (`pico8_research_build_gb(void)` parcourt `pico8_fd_table`) ; `pico8_ho_apply` sans appelant de `src/` (D.7a `collect`, ou D.8 avec le test ponctuel). `overrides` après D.6c : 30 lignes / 34 occurrences / 8 fichiers (les 21 lignes des quatre ponts de D.6b et les 9 de leurs TU, le « = 30 » de la note de D.6b ; le « = 0 » de D.1 comptait sans eux) ; `overrides` n'entre pas dans `DATA_RESIDUE_GUARD` : il y entre à l'étape qui ferme le dernier pont — `js`, dont les rangées `bs` de `pz` sont à D.7b ; D.7a ferme `lq`, `jb`, `mh` et les autres sources de `js` si son PRD ne réordonne pas. La table `{qe=r.e}` de `jo:cn` (`planet_trade.c`, `jy`) reste une `pico8_ob_table_new` : ce n'est pas une clé de construction (D.7a). `par-nom` : 1038.
> - D.7a — `ob`, tables ad hoc de l'affichage, de l'économie, de la recherche et des données (troisième puce) : tout `ob` restant de `planet_display`, `research_display`, `score_panel_display` (`g_de`, `spec1`/`spec2`), `slipway_display`, `wormhole_display`, `layout`, `menu`, `collect`, `economy*`, `research` (`g_gb`, `pico8_fi`), `planet_trade` (enregistrements de `jo:ib`), `tech_gate`, `score_panel`, `pi`, `gen`, `galaxy`, `cart`, `data_init`, `game_data`, `mouse` → structs locales ou tableaux fixes, les TU touchés suivant dans le même commit. Empreinte 39 (39, 0) aujourd'hui. Compte : `ob` ≤ 1057, et 0 dans ces fichiers. §6 : `ga_export_panel_by` (`route_a`/`route_b`, l. 102-103 et 147-150, enregistrements de `jo:ib` poussés dans `ga.by`), preuve rouge. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.7a : six chunks (familles isolées ; `gb` ; enregistrements `{qe,gc}` ; articles de `bl` et panneau de score ; pipeline d'affichage et chaînon `pz` ; `nb` — 74 chemins : suiveurs forcés, TU qui lisent `gb` ou les ponts, test ponctuel hors du majorant), **commis dans l'ordre 1, 2, 4, 3, 5, 6** : les enregistrements `{qe,gc}` de `jo.jy` (chunk 3) et les sous-rangées de `bz` (chunk 4) sont des tuples de `lq:mn`, que `layout.c` lisait comme tables ob ; typer l'une ou l'autre famille avant ce lecteur demandait une conversion typée → ob ou un `lq:mn` à double lecture, tous deux interdits. Le chunk 4 a donc pris tous les tuples de `lq:mn` (et les articles de `bl`, qui partageaient la même vue de `data_init`), le 3 a suivi, le 5 est resté le chaînon `pz` sans les tuples ; décision du pilote. **Ordre D.7a/D.7b : D.7a a pris à D.7b le chaînon `pz`** (`pico8_pz`, `pico8_mh_new_table`, `g_bo` et `fv.rb` de `help.c`, `pico8_js_new_table` et les branches `OB_TABLE` de `js:pq`), **`g_nf_l`/`g_nf_r`** (avec `bu`) et les lignes qui lisaient ses types convertis dans `planet.c`, `planet_family_trade.c`, `ui.c` ; D.7b ne les a plus. Formes : rangées, tuples de `lq:mn` et articles de `bl` en `pico8_jb_row_t` (sorte REF opaque pour `ld` ; `{qe=x}` gardé comme `has_qe`/`qe` de la rangée et lu `{x,5}`) ; `pz(p, bs, n)` sur `pico8_js_keys_t` ; sources `lq`/`h_`/`jb` typées (`lq_row`/`h_row` : `pico8_lq_keys_t`, `pico8_lq_keys_of_row` ; `jb_row`) ; `{qe,gc}` en `pico8_jy_t` ; `gb` tableau fixe (`pico8_gb_at`, `pico8_gb_find`) ; `nb.e_` (`pico8_nb_e_`, remis par `pico8_data_init`), `pj.ou`, `pj.ea`, `bz` (`pico8_pj_bz`), `nf.l`/`nf.r` statiques non `const` ; `pico8_de()` supprimé (`pico8_de_my`). Vues fermées : `ix`, `ha`, `bo`, `bz`, `nb`, `pico8_jb_row_view`, `jo_kw_view`, `hp:jb`, `pico8_lq_table_view` ; `pico8_data_init(void)`. `overrides` = 0, dans `DATA_RESIDUE_GUARD` depuis le chunk 5. `ob` dans les fichiers de la ligne : 13 lignes (enregistrements de constructeurs de `ob()` `'v'`/`'s'` dans `pi.c` et leur commentaire de `pi.h`, `'b'` et `pico8_ho_apply` dans `collect.c`/`.h`), servant le test ponctuel, supprimées par D.8 ; le « = 187 » de D.7c devient 187 + 13. `ob` après D.7a : 552 lignes / 613 occurrences, dont 199 dans le test ponctuel (hors test ponctuel : 353) ; majorant de D.7b : ≤ 309 hors test ponctuel (353 moins les 44 lignes `ob` de ses fichiers de `src/`). Les greps de sortie `pico8_fi\(` et `pico8_ob_table_t \*g_` ne rendent plus que `pico8_ob.c/.h` et `pico8_stash.c`, laissés à D.8. Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : 142406 / 110943 → 19213 / 59310 octets (3480 / 2423 → 1099 / 1481 allocations). `par-nom` : 1009. Correctifs aux messages de commit de D.7a (comptes remesurés sur chaque arbre) : (1) classification des lignes retirées, (a) construction supprimée, (b) signature, contrat ou variable dont le type passe d'une table ob à un type C, (c) même type, autre nom : 5e58f7c, `ob` 340 lignes = (a) 317 + (b) 23 (les contrats de `pico8_pz` dans `msgbox.h` et de `pico8_hp_jb` dans `research_display.c/.h` ; les variables retypées `b` de `pz_step`, `row` de `jo:fe`, `l` de `jo.ex`, `row` de `cr_step` ; dans les TU, `build`, `fold_built_from`, `positionals`, `call_row_my` et leurs 12 variables `t`/`spec`/`lq_t`/`h_t`/`b`/`jb`), `overrides` 18 = (a) 18 (les trois ponts et leurs deux commentaires de TU) ; (c) = 1, hors de toute catégorie comptée : `pico8_bz()` → `pico8_pj_bz()`, renommage à type égal fait après que fa997e7 avait déjà typé son retour, parce que la grep de sortie `pico8_(ix|ha|nb|bz|de)\(\)` contredisait la l. 326 du PRD (qui fait rendre à `pico8_bz` les rangées typées) ; aucun compte n'en bouge. 1a1687a, `ob` 58 lignes = (a) 54 + (b) 4 (`t` de `cr_step` et du prédicat de `cb`, `tech` de `test_pico8_data_init`, `record` du test ponctuel), `overrides` 0, aucune (c). (2) lignes qui gagnent une catégorie : 5e58f7c, `valeur` `help.c` +1, `layout.h` +1, `menu.h` +1, `ui_button.h` +1 ; `type-tag` `test_pico8_js` +1 ; `dispatch` `menu.h` +1, `test_pico8_fq` +1 ; 1a1687a, `ob` +7 et `valeur` +6 dans le test ponctuel (lecture de la référence décodée) ; fa997e7 omettait `valeur` `help.c` +1 et `layout.c` +2 ; c5bee4b : `valeur` `research.h` +1, `test_pico8_hp` +2, `test_pico8_lq` +4, `test_pico8_trade` +2. (3) fa997e7 a aussi ajouté une lecture ob → typée temporaire dans `jo:gh` (`jo_gh_record_tuple`, `planet_display.c` l. 85 : `qe` lu dans l'enregistrement ob pour poser le tuple typé), retirée par c5bee4b : l'ordre 4 avant 3 demandait lui aussi une conversion, le temps d'un commit. (4) c5bee4b : `sizeof(pico8_jy_t)` vaut 48, pas 40 (40 est `pico8_jb_row_t`). (5) La hausse de fuite de c5bee4b à 5e58f7c (+32 o / +4 allocations sur `message_box_hold`, +8 o / +1 sur `game_end_final_score`) vient d'allocations faites une fois par `pico8_fv_pq` à la construction de l'écran d'aide (le `ra` partagé et les `qf` de ses boutons, joignables par `g_bo` avant), pas d'une fuite par frame.
> - D.7b — `ob`, reste de `src/` hors `pico8_ob.*` et `pico8_stash.*` : entités et interface (`planet*`, `probe`, `particle`, `slipway`, `wormhole`, `msgbox`, `mouse_reader` avec `g_nf_l`/`g_nf_r`, `ui*`, `help`, `scroll`, `trade`, `bg`, `ke`, `entity`, `bootstrap`, `hud`, `research_icon`) et en-têtes (`pico8_struct.h`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_value.h`, `pico8_list.h`), même règle. Empreinte 42 (42, 0) aujourd'hui. Compte : `ob` ≤ 827, et 0 dans `src/` hors ces deux modules. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.7b : trois chunks (`nf.l` dans `test_pico8_spec_defaults`, qu'une table ob écrivait hors du `pico8_nf_button_t` de D.7a ; particules de `qy` ; inclusions et `assert` d'init — 26 chemins). Le TU corrompu : `test_second_jl_keeps_specs` posait une clé de table ob dans `g_nf_l` (16 octets) et débordait sur `g_nf_r` depuis `e252fde` ; il passait en relisant sa propre corruption. Réparé (marque dans `hm`), preuve rouge sous ASan. Un balayage des 81 TU sous `-fsanitize=address,undefined` (copie jetable, `Makefile` inchangé) n'a rien trouvé d'autre ; les TU ne sont compilés sous ASan dans aucune cible. Formes : `pico8_qy_t` (`p`, `v`, `has_a`/`a`, `l` par valeur ; une allocation par `mm`, mise à jour en place par `ei`, plus d'allocation par frame) ; `pico8_help_init` lit `pico8_pi_dispatch_lookup` ; `slipway.h`, `ui.h`, `wormhole.h` incluent `pico8_list.h`. `pico8_ob.h` n'est plus inclus dans `src/` que par `pico8_ob.c`, `pico8_collect.h`, `pico8_stash.h`. R = 22 : les 13 de D.7a et 9 lignes de commentaire qui décrivent la couche encore vivante (`pico8_type_tag.h` 2, `pico8_ref_audit.h` 4, `pico8_value.h` 1, `pico8_list.h` 1, `pico8_hud.c` 1), que D.8 retire ou réécrit avec `pico8_ob.h` et `PICO8_TYPE_TAG_OB_TABLE` ; le « 187 + 13 » de D.7c devient 187 + 22. `ob` après D.7b : 491 lignes / 550 occurrences (hors test ponctuel : 292 ; `src/` : 147 ; TU hors test ponctuel, `test_pico8_ob` et `test_pico8_stash` : 83 lignes dans 28 fichiers, le travail de D.7c). Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : 19213 / 59310 → 19213 / 58910 octets (1099 / 1481 → 1099 / 1431 allocations) ; `message_box_hold` n'exécute ni `ei` ni un `mm` (son `qy` reste vide) ; `tech_geoharvesting_burst` : 240568 → 47328 octets (23752 → 1682 allocations).
> - D.7c — `ob`, TU restants : fixtures construites en `pico8_ob_table_*` → structs, état construit directement. Empreinte 41 (0, 41) aujourd'hui. Compte : `ob` = 187 (`pico8_ob.c` 82, `pico8_ob.h` 32, `test_pico8_ob.c` 55, `pico8_stash.c` 8, `pico8_stash.h` 3, `test_pico8_stash.c` 7). §6 : aucun. `sanitize-net` et `class-coverage-net` : non, aucun fichier de `src/` ne change. Découvert à D.7c : trois commits (inclusions seules ; valeurs hors rangée, tests fermés et `by` ; plis de hachage — 29 chemins, 28 TU et ce plan, aucun de `src/`). Les cas hors rangée qui offraient une table ob à un membre typé sont retirés quand un frère garde la clause, remplacés par une liste sinon (`ma`/`m_` de `test_jo_writers_stop_off_row`) ; les tests fermés passent une liste locale ; `test_pico8_ga` poussait des tables ob dans `by`, qui tient des routes depuis D.7a : il pousse des routes. Aucune constante `*_HASH` ne change. Arbitrage du pilote : les trois octets de sorte de clé (`fold_names` de `test_pico8_hp`, `fold_qe_map` de `test_pico8_hy`, `fold_hg` de `test_pico8_jc`) ne deviennent pas des littéraux `0u`/`1u` (un identifiant compté remplacé par sa valeur est un alias) : ces trois lignes et les trois inclusions de `pico8_ob.h` qu'elles demandent restent, et D.8 les hérite avec la l. 2 de `test_pico8_planet_gh_row_lq` (R_t : l'énoncé du bug d'un test du §6 nomme `pico8_ob_table_t`), d'où « §6 : `gh_row_lq`, commentaire de la l. 2 seulement, pas de preuve rouge » à D.8. Les octets de `hp` et `hy` sont atteints (les retirer rougit `DEFAULTS_HASH` et `CONSTRUCTION_HASH`) : D.8 doit plier la même valeur sans la couche, arbitrage à reprendre, jamais une constante mise à jour ; celui de `jc` ne l'est pas (aucune constante ne plie un `hg` non vide : sauter toute la boucle de `fold_hg` laisse chaque constante verte). `ob` après D.7c : 415 lignes / 470 occurrences dans 20 fichiers (hors test ponctuel : 216 = 187 + 22 + 1 + 6, les 6 étant ces trois plis et leurs trois inclusions) ; `tests/` ne nomme plus `PICO8_TYPE_TAG_OB_TABLE` ; `type-tag` 373 → 362 lignes (592 → 572 occurrences). Balayage ASan+UBSan jetable des 28 TU touchés : propres.
> - D.8 — suppression : `pico8_ob.c/.h` (avec `pico8_fi`), `pico8_stash.c/.h`, `test_pico8_ob.c` et `test_pico8_stash.c` (§6, tests de la couche), le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, leurs entrées du `Makefile`. Compte : `ob` = 0, `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : non. Découvert à D.8 : deux commits (octet de `jc`, non atteint : `test_pico8_jc.c` seul, chaque constante inchangée, un octet remis reste vert ; suppression — 26 chemins, dont `tools/float_guard.txt`, que `make float-guard` réécrit dans `make test` : `garde fichiers` 60 → 58, les deux fichiers du coeur supprimés, précédent D.2 ; 27 en tout). `test_pico8_map_tables.c` perd la comparaison à la référence (`decode`, `parse`, les `*_SRC`, les comparateurs, `pico8_load_map_json`, `pico8_pi_init` et `pico8_data_init` de `main`) et garde les tests des modules typés qui ne la lisent pas : six tels quels, et les bornes de `pico8_int_seq_at`/`pico8_fixed_seq_at` et les recherches `r`/`iz`/`hc`/`df` reformulées sur la table typée, sans assertion nouvelle. Les octets de sorte de clé de `hp` et `hy` sont pliés par une constante locale nommée (`VIEW_INDEX_KEY`, existante ; `TABLE_INDEX_KEY`, nouvelle), octet gelé d'une énumération supprimée, (c) déclarées ; aucune constante `*_HASH` ne change. `PICO8_TYPE_TAG_OB_TABLE`, `PICO8_REF_AUDIT_EXPECT_OB_TABLE` et le compteur `ob_table` de l'audit supprimés (`tools/ref_audit_report.sh` inchangé, son mot `ob_table` est mort) ; `pico8_ho_apply`, les constructeurs `'b'`/`'v'`/`'s'` et leur enregistrement supprimés ; TROU-4 et TROU-9 sans site. `ob` = 0, `stash` = 0, dans `DATA_RESIDUE_GUARD` ; `type-tag` 352 lignes / 560 occurrences (au lieu de ≈ 354 : les renvois de `pico8_list.h` et de `pico8_ref_audit.h` à l'explication du pas de lecture de `pico8_type_tag.h`, retirée avec la table ob, partent aussi), `ref-audit` 56 / 57, `valeur` 4961 / 5742 ; `par-nom` 1009, `dispatch` 191 inchangés ; résidu D 5868 lignes dans 128 fichiers. `sanitize-net` propre, `class-coverage-net` identique. **Sortie de la phase non atteinte** : `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h` restent, avec `valeur`, `type-tag`, `ref-audit`, `par-nom`, `dispatch` ; la suite attend les décisions humaines de D.9.
>
> **Arrêt « champ sans règle » (§7), constaté à l'écriture de la liste : elle s'arrête à D.8.** Aucune puce de D ne dit ce que deviennent `par-nom` (1081/80) et `dispatch` (197/59). La première et la troisième puce ne couvrent que des tables : `ob` et `stash`, plus, par lecture, `spec` et `overrides`. La **Sortie** nomme des fichiers à supprimer, pas la forme de leurs usagers, et D.0 ne fait que compter. `valeur` (5989/132), `type-tag` (445/71) et `ref-audit` (70/17) ne peuvent tomber qu'avec eux. Les décisions de D.9 (ci-dessous) lèvent cet arrêt pour `par-nom`, `dispatch` et `pico8_list_t` (D.10 à D.12) ; la Sortie de D exige encore le travail sur `valeur`, `type-tag` et `ref-audit`, suspendu à Q12.
> 1. `par-nom` : `pico8_entity_get_field`/`set_field` (132 lignes de `src/`), `*field_truthy(`, les `field`/`set_field` des descripteurs (`pico8_struct_field_fn`, `pico8_struct_set_field_fn`), le `field` des sortes et 191 lignes d'aides `*_take_*`. La puce **Coexistence** de 1b les fait survivre à 1b, et aucune puce ne dit en quoi ils se changent. Les aides à nom non littéral (`field_fixed(inst, name)` et semblables, dans `economy_score.c`, `particle.c`, `probe.c`, `score_panel.c` et `entity_geom.c`) relèvent en plus de la clé dynamique du §5.
> 2. `dispatch` : `pico8_value_function_t methods[PICO8_METHOD_COUNT]` (`pico8_struct.h`), `pico8_mv()`, et les membres fonction (`js.my`/`hb`, `mh.lh`…). Typer les arguments demande une signature par méthode, qu'un tableau indexé par `pico8_method_t` ne tient pas. Or la note de D.0 dit que la table survit à D.
> 3. `pico8_list_t` : la note de D.0 exige d'en changer la représentation sans en fixer la forme. `lq.items` tient des chaînes, des fonctions, nil et le fixe 0 (`layout_nil_item_draws_nothing`), et aucune puce ne type ces éléments hétérogènes.
>
> - D.9 — **étude, sans conversion** (demandée par l'humain à l'arrêt) : pour chacun des trois points ci-dessus, recenser les sites par grep (fichier, fonction, noms lus ou écrits, genres des valeurs), les classer par forme de remplacement possible — accès au membre typé (`p->hp`) à la place de la lecture par nom ; fonction C par méthode, à arguments typés, à la place de `pico8_mv` et de la table `methods[]` ; tableau typé, union étiquetée locale ou struct à la place d'une `pico8_list_t` d'éléments `pico8_value_t` —, et pour chaque classe de sites : la forme retenue, le nombre de sites, ce qui n'entre dans aucune forme (clé dynamique du §5 incluse). Sortie : une section de ce plan qui propose les puces à ajouter à D (ou à reporter en phase 2), avec leur liste d'étapes et leurs comptes ; l'humain tranche avant la première étape qui les applique. D.2 à D.8 ne dépendent pas de cette étude.
>
> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.
>
> Comptes au départ de D.10 (HEAD 5a13008, `make -s data-residue` et `--detail` ; l'étude mesurait `2dec71f`, avant D.7a–D.8) : `valeur` 4961/126, `type-tag` 352/65, `ref-audit` 56/15, `par-nom` 1009/81 (`src/` 433/43, `tests/` 576/38), `dispatch` 191/61 (`src/` 67/30, `tests/` 124/31), résidu D 5868 lignes / 128 fichiers ; `liste`, pas encore au compteur, 927/99 avec le motif de Q10 (`src/` 496/61, `tests/` 431/38). Les classes A–M, T1/T2 et L1–L4 sont celles de l'étude (annexe A) ; ses « ≈ » viennent de son classement, chaque PRD remesure. Un TU qui nomme un symbole retiré par une étape de `src/` la suit dans le même commit, ou le symbole reste comme pont compté, fermé par l'étape TU de son groupe (D.10c, D.11d, D.12d) ; le PRD tranche. `sanitize-net` : aux seules étapes qui changent une durée de vie ou un propriétaire (§7).
>
> - D.10a — `dispatch`, table de méthodes : `methods[]` → struct de pointeurs typés, 23 emplacements nommés (Q2), et table d'états typée de `dg` (`ph`, `ly`, `ll`, `kz`, `pr` ; Q3) ; les 82 adaptateurs `(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1)` retirés ; les 26 appels `pico8_mv` de `src/` remplacés (appel direct de la fonction typée pour `eb`, `ib`, `gg`, `md`, `lq`, `lh` ; virtuel pour le reste) ; `dg`, `dc`, `ui:jz`, `lt`, `he` et les prédicats `*_is_*_class` sur les emplacements ; `pico8_entity_get_field` ne rend plus de méthode ; `hd=1` en fonction constante vraie ; `pico8_dispatch_context` gardé (Q4) ; sites `par-nom` G (9) et E de `mv` (1). Deux chunks : a1 (méthodes lancées par `dc`, `lt`, `he`, `ui:jz`, `dg`), a2 (le reste). Empreinte 37 (37, 0) aujourd'hui. Compte : `dispatch` `src/` ≤ 35 (67 moins les 29 lignes `pico8_mv`, `entity.c:177`, `methods[]` de `pico8_struct.h` et le commentaire de `pico8_dispatch_context.h`), ≤ 37 si `pico8_mv` reste en pont pour les TU ; `par-nom` ≈ 999 ; `valeur` rapporté. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non. Découvert à D.10a : deux chunks (a1 : 73 chemins ; a2 : 62). `pico8_struct_class_t` porte 14 membres typés (`kj`, `ev`, `hd`, `hk`, `hb`, `ldown`, `rdown`, `gv`, `hq`, `ey`, `kf`, `cn`, `ol`, `mm`), la table `states[]` (`pico8_state_t`, `pico8_state_of`) et le crochet `has_hb` (présence de `hb` par instance, pour `js`) ; `ldown`/`rdown`/`gv` rendent `pico8_down_t {stop, held}` ; `eb`, `ib`, `gg`, `md` sont appelés directement (`pico8_jo_is_family`) ; `hd=1` est `pico8_hd_true` ; `pico8_ui_jz` prend un sélecteur. Pas de pont : les TU ont suivi (les lignes `pico8_mv` et les lignes `pico8_value_function` de méthode, `fns[]` et aides `method()` compris) ; D.10c ne garde que les rappels des TU (9 lignes : `fq` 3, `js` 4, `mh` 1, `pj` 1, plus ce que D.10b y laisse) et perd ses lignes `pico8_mv`. D.10b hérite des 19 lignes d'adaptateurs de rappel (dont `pico8_hp_lq_dispatch`, `pico8_jo_fe_my_dispatch`) et des membres `my`/`hb`/`lh`/`lq`, appelés directement depuis D.10a ; le crochet `has_hb` survit au typage de `js.hb` ; le commentaire de `pico8_value_function_t` (`pico8_value.h`, « appelé par dc()/mv() ») est à réécrire avec lui. Sites G laissés : `ui_button.c:599` (D.10b), `slipway.c:261` et `research_icon.c:182` (D.11a), `ke.c:43` (D.11c) ; pris à D.11b (classe C) : la présence des noms `bm` dans `lt`/`he` et de `gv` dans `jc:hq`. `lt`/`he` assertent eux-mêmes le tag de struct (l'assert passait par `pico8_entity_get_field`). TROU-11 sans site. `dispatch` : `src/` 35 lignes / 37 occurrences / 15 fichiers, `tests/` 9 ; `par-nom` 986 (`src/` 419 au lieu de 420 : le commentaire de la déclaration de `pico8_mv` nommait aussi `pico8_entity_get_field`) ; `valeur` 4272 / 4639, `type-tag` 347 / 551, `ref-audit` 55 / 56 ; résidu D 5137 lignes dans 126 fichiers. Balayage ASan+UBSan des TU propre, `class-coverage-net` identique aux deux commits.
> - D.10b — `dispatch`, rappels : `js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, rappels des rangées de `menu` (`g_ix_my`, `g_ix_hb`, `ha`), `jo:fe`, `de.my` (`pico8_de_my`), `eu` de `help`, `init` de `score_panel` → pointeurs de fonction à la signature de leur unique appelant ; leurs lecteurs (`msgbox.c`, `research.c`, `ui_button.c`) ne les rendent plus comme valeur ; `PICO8_VALUE_FUNCTION`, `pico8_value_function()` et `pico8_value_function_t` retirés de `pico8_value.h/.c`. Empreinte 18 (18, 0) aujourd'hui. Compte : `dispatch` `src/` = 0, pont pour les TU excepté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non. Découvert à D.10b : deux chunks (b1 : 25 chemins ; b2 : 21). Rappels typés à la signature de leur appelant : `pico8_js_fn` (`void (*)(pico8_js_t *)` : `js.my`, `js.hb`, rangées de `menu`, `jo:fe`, `eu` — installé par `pico8_eu` lui-même —, `init`), `pico8_fq_my_fn` et `pico8_mh_lh_fn` (`int`, vérité du retour) ; `de.my` en deux variantes (`pico8_de_my()` rend celle de `fq(de)`) ; `hp.lq` sans adaptateur ; les lecteurs par nom ne rendent plus de rappel (`pico8_js_my` supprimé) ; `PICO8_VALUE_FUNCTION`, `pico8_value_function()`, `pico8_value_function_t` supprimés. Pas de pont : les 9 lignes de rappel des TU ont suivi, **D.10c n'a plus d'objet** (ses lignes `pico8_mv` sont parties à D.10a, ses lignes `pico8_value_function` à D.10a et D.10b) ; `dispatch` = 0, dans `DATA_RESIDUE_GUARD`. D.11a : les lectures par nom de `_cr_t`, de `lq` (`ix[2].hb`) et de `_jo_fe_*` se font sur un `self` déjà typé `pico8_js_t *`. Q12 : `pico8_js_my()` (nil, `false` ou fonction) n'existe plus ; le genre mêlé de `js.my` ne vit que dans le membre (`has_my` + NULL). `FQ_MEMBERS_HASH` reprise par sonde (pas « `my` rend nil » retiré). `par-nom` 986 (`src/` 418) ; `valeur` 4067 / 4364 ; résidu D 4933 lignes dans 124 fichiers.
> - D.10c — `dispatch`, TU : 98 lignes `pico8_mv` → appels typés, 26 lignes `pico8_value_function` → rappels typés ; le pont éventuel de D.10a/D.10b supprimé. Empreinte 31 (0, 31). Compte : `dispatch` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change hors pont. Découvert à D.10c : vide, constatée sans commit de code — tout est parti à D.10a et D.10b, sans pont.
> - D.11a — `par-nom`, receveurs de type connu et singletons : classe A (41 lignes, 36 appels d'aides ; cast vérifié vers la famille que le cart fixe), classe D (8, 21 ; `pico8_fm_hy()`, `pico8_je_nf()`, `pico8_mo_qb()`, NULL sur le remplaçant, Q8), E `je.l`/`je.r` (`pico8_ui_je_button_table`), et les aides H (39) devenues sans appelant. Empreinte ≈ 22 (22, 0). Compte : `par-nom` ≈ 910. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non. Découvert à D.11a : deux chunks (a1 : 19 chemins ; a2 : 29). Singletons (Q8) : `pico8_fm_hy()` rend NULL sur le `fm` provisoire (son assert part, les méthodes de `hy` prennent NULL ; le cas de mort `fm_hy_on_provisional` de `test_pico8_hy`, que le PRD n'avait pas vu, devient `pico8_fm_hy() == NULL`), `pico8_fm_placeholder()` le rend typé pour `jk`/`nz`, `pico8_je_nf()`, `pico8_mo_qb()` (une écriture sur le `mo` provisoire arrête toujours), `pico8_nf_l()`/`pico8_nf_r()` pour `je[bt]`. Receveurs connus : cast vérifié par `pico8_jo_is_family` ou une classe exacte (`jc`, `trade`, `qb`) ; `pico8_lt` prend `pico8_ke_t *` (porte de `hv`, `me`/`t` de `dg` lus typés ; `giver_t` de `test_pico8_ui` porte un `ke`) ; extrémités de voie par `pico8_jc_end_p`/`pico8_jc_end_ra` (planète ou point de souris ; `jc:ch` lit aussi `id` sur le point de souris : absent). G : `ie.gy` en `const pico8_vec_t *` (`pico8_ie_eh`/`pico8_ie_set_gy` typés, `ie_take_vec` supprimée, quatre cas `hb_from_gy` retirés). **`jc.mt` reste par nom** : H2 est fausse pour lui (`slipway.c` est dans `DATA_ENGINE_SRCS`, `planet_family_trade.c` non, et `pico8_fr_struct_class` casse le lien de `test_pico8_bg_kj`) ; `jc:hq` garde `pico8_jc_entity_value` et `jc_take_fixed` (3 lignes `par-nom`), que D.11b prend comme champ transversal (Q6, accesseur de la table de classe) ; la comparaison des `mt` de `jc:ce` est typée. **Reclassés en C (D.11b)** : `mz_pq_ox_predicate` (`fx` sur `hv`), `pico8_ij` (`p`) et `pico8_mzd_ra_of_ij` (`ra`), parce que `mz.ox` tient des planètes et des `g_`/`fv` (`fx=1`). Aides H restantes (appelant B/C/F/M, ou `mt`) : `field_truthy` d'`economy_score`/`economy_tiers`/`planet_display`, `field_fixed` de `planet_display`, `pico8_entity_geom_field_*`, `pico8_msgbox_field_vec`, `pico8_probe_field_fixed`/`_truthy`, `pico8_ui_field_raw`, `pico8_mzd_field_vec`, `pico8_jc_entity_value`/`jc_take_fixed`. Q12 : `pico8_ie_eh`/`pico8_ie_set_gy` quittent la liste ; `pico8_jc_hg_add`/`_has`, `pico8_trade_qe`, `pico8_jo_hf` reçoivent une valeur reconstruite du membre à leur appel. `par-nom` 913 (`src/` 345 : départ − 73, pas − 76, les 3 lignes de `mt`) ; `valeur` 3906 / 4202 ; résidu D 4736 lignes dans 124 fichiers.
> - D.11b — `par-nom`, lectures sans receveur typé : B (7 + 1 ; test d'appartenance par le lien parent, `pico8_qh_of(jo)`, Q5), (C passe en tête de D.11e : ses descripteurs sont ceux que D.11e rouvre, revue du plan après D.11a), F (5 + 2 ; struct locale de cadre `{ep, eo, hi, ff, ca, gq, ra}` remplie par les cinq appelants de `pz`), K (4 ; enregistrement typé par sorte `{rr, has_s, s, me, has_od, od}`, résolu le long de la chaîne, pointé par `pico8_entity_kind_t`). Empreinte ≈ 15 (15, 0). Compte : `par-nom` ≈ 882. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non. Découvert à D.11b : deux chunks (b1 : 15 chemins ; b2 : 7). B (Q5) : `pico8_struct_class_t.parent` (9 filles de `jo`, `fv`) est un pointeur vers l'accesseur de la classe parente (`const pico8_struct_class_t *(*parent)(void)`), pas vers ses données : les descripteurs parents sont `static` dans d'autres unités et un initialiseur statique veut une constante d'adresse ; `pico8_qh_of()`/`pico8_nl_of()` remontent ce lien (NULL hors lignée) ; `pico8_jo_is_family` (test par `states[LL]`) pourrait s'écrire sur `parent`, non fait. La branche « `hp` chaîne » de `jo:pq` était inexprimable, retirée (`pico8_gb_find` sans appelant de `src/`, gardée pour ses TU). K : `pico8_entity_kind_t.rec` → `pico8_kind_rec_t` `{rr, has_s, s, me, has_od, od}`, généré dans `pico8_game_data.c` pour les 9 sortes (pas 11), mêmes chaînes que `pico8_X_start` (s/od en mêmes littéraux) ; la garde « prêt » du `field` des sortes n'est pas dans l'enregistrement ; l'oracle de transition `test_kind_rec_matches_the_reader` (`test_pico8_entity_kind`, par l'aide `kind_field` existante, aucune ligne `par-nom` ajoutée) est à retirer par D.11d/D.11e, après quoi D.11e peut supprimer le `field` des sortes. F : `pico8_pz(const pico8_pz_frame_t *, …)`, cadre rempli par `fv:pq`, `cr`, `ix[4].my`, `jo:gh`, `jo:fe` (aide `jo_pz_frame`) ; vues `pj_cr`/`ha_pz` laissées à D.11e ; le `ra` du cadre de `fv` n'est jamais lu (`fv.hi` toujours posé) ; le cas « une liste arrête `pz` » de `test_pz_closed` est retiré (inexprimable). `parent` est disponible pour C (D.11e). `par-nom` 884 (`src/` 317, `tests/` 567 : l'oracle n'ajoute pas la ligne prévue, d'où −1 sur les 885 du PRD) ; `valeur` 3876 / 4170 ; `type-tag` 343 / 544 ; résidu D 4681 lignes dans 124 fichiers.
> - D.11c — `par-nom`, classe M (Q7) : émetteur de particules (`ei`, `ez`, `f_`, `g__mm`) et sonde (`bv`, `dl`, `d_`) → composants portés par pointeur, règle 2 de la phase 2 appliquée à ces deux groupes seulement. Empreinte ≈ 15 (15, 0) : `particle`, `probe` et les 13 fichiers des classes qui les portent. Compte : `par-nom` ≈ 862. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : oui. Découvert à D.11c : deux chunks (c1 : 14 chemins ; c2 : 23). Composants (règle 2, Q7) : `pico8_emitter_t` `{pn, du, py, ki, pk, pk_is_bool, fc, k_, qo, ro, qg, q_, qy}` porté par `g_`, `hj`, `ga` ; `pico8_probe_t` `{et, has_et, dw, od, has_od, dq, fp, fp_is_bool, ik, has_ik}` porté par `g_`, `qh`, `mz`, `jc` ; pointeur vers un stockage dans l'hôte (`emitter_own`/`probe_own`), aucune allocation ; crochets `emitter_of`/`probe_of` de `pico8_struct_class_t` (6 et 12 descripteurs), l'enchâssement inchangé. Champs d'hôte (`ra`, `p`, `p_`, `fa`) et de cible (`fo`, `qs`) par cast vérifié sur l'ensemble fermé (`pico8_g__of`, `pico8_jo_is_family`, classes exactes), `me`/`t` par `ke`. `pico8_dl_show` pour la vue de l'icône ; `pico8_ke_ks(self, const char *)` (G `ke.c:43`) ; `ei`/`f_` s'arrêtent sur un non-émetteur. `pico8_jc_too_far_t` dans `slipway.h`. Trois TU lisaient les défauts déplacés en membres et suivent (`test_pico8_spec_defaults`, `test_pico8_jo_spec_defaults`, `test_pico8_g_`), hors de l'empreinte du PRD ; l'écriture de `p_` par `d_` sur un `mz` est aussitôt écrasée par `mz:ey` (mutation équivalente), et le numéro de sfx de `ik` n'est observé par aucun TU (audio hors périmètre). Pour D.11e : `get_dw` n'a plus d'objet (ses 12 descripteurs sont les porteurs de `probe_of`), `set_fo` peut réutiliser l'écriture typée de `bv` (planète, `jc`, `bv_et`) en y ajoutant `jc_too_far`. Pour D.12b : `qy` est `emitter->qy`. `par-nom` 859 (`src/` 292) ; `valeur` 3815 ; résidu D 4605.
> - D.11d — `par-nom`, TU : T1 (mise en état et lecture d'état) en accès aux membres typés ; T2 (tests des descripteurs, tests de mort des `assert` d'écrivain) supprimés comme tests de la couche (Q11, §6). Deux chunks d'environ 19 fichiers. Empreinte 38 (0, 38). Compte : `par-nom` ≈ 286 (l'infrastructure de `src/`). §6 : `test_pico8_planet_gh_row_lq` et `test_pico8_ga_export_panel_by` (3 lignes chacun) lisent les membres, preuve rouge. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change. Décidé après la PRD (humain, 2026-09-29) : ≈ 1 555 appels d'aides par nom à retyper, l'étape passe en quatre sous-étapes d'environ 50 min, même PRD (section « Découpage alternatif »), chacune vérifiée et poussée : D.11d1 (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr` ; `tests/` → 473), D.11d2 (`ga`, `hp`, `hy`, `trade`, `lq`, `entity_kind`, `map_tables`, `ie`, `ga_export_panel_by` ; → 369), D.11d3 (`ui`, `g_`, `fv`, `mz`, `bg`, `fq`, `mh`, `iv`, `planet_gh_row_lq` ; → 162), D.11d4 (`js`, `jc`, `pj`, `qb`, `nf`, `fm`, `je`, `mo`, `probe_mm_rnd_order`, `jb_ol`, `spec_defaults`, note du plan ; → 5). Découvert à D.11d1 : `par-nom` `tests/` 567 → 473 lignes / 622 → 499 occ. (29 fichiers, `src/` 292 inchangé, total 765), `valeur` 3815 → 3523 ; 94 lignes : (a1) 18, (a2) 76, (a3) 0, (b) = (c) = 0 ; constantes gardées toutes égales, plis de forme déclarés (5 à 21 bits par pli, 9 TU) ; 4 tests supprimés (`nl` : `test_nl_members_by_name` et `test_nl_writer_stops_off_row`, dont la seule cible était `write_qw_value`, T2-M, absent de la liste des 47 ; `fr` : `test_nr_members_by_name`, `test_fr_mt_by_name`), donc 48 pour l'étape ; `gcov` : 233 lignes de `src/` perdues, 196 d'infrastructure par nom, 4 couvertes par `test_pico8_value`, 33 par les e2e, aucune assertion ajoutée ; M2 (`hj` sans `k_`) ne rougit que `hj` : `pm`/`mx` reposent leur `k_` (`planet_family.c:673`, `planet_family_trade.c:450`), mordus par M2b/M2c ; hérité par D.11d2 : `bought`/`icon_of` lisent `ie->ke` après un test de classe, `credits()` lit `pico8_fm_hy()->nz`, `pico8_mo_qb()->ra` remplace la lecture de `mo`, même recette pour les TU suivants. Découvert à D.11d2 : `par-nom` `tests/` 473 → **371** lignes / 499 → 389 occ. (21 fichiers ; `src/` 292 inchangé ; total 663), pas 369 : `ie.gy` et `ie.id` sont l'état partagé de la classe (règle « Archétype ou état »), sans membre et sans lecteur typé dans `src/` ; `test_pico8_ie` les lit encore par nom dans deux aides sans paramètre de nom (`shared_gy`, `shared_id`, 2 lignes `pico8_entity_get_field`), leurs constantes pliant `gy`/`id` restent égales ; D.11e doit leur donner un lecteur typé (`pico8_ie_gy()`/`pico8_ie_id()`, absent de son PRD) avant de retirer `pico8_ie_field`, et le compte cible des pas suivants se décale de +2 (D.11d3 → 164, D.11d4 → 7) sauf rattrapage. 104 lignes retirées et 2 ajoutées (`ie`) : (a1) 15, (a2) 84, (a3) 5, (b) = (c) = 0 ; `valeur` 3523 → 3182, `type-tag` 343 → 332. 11 tests supprimés, comme prévu. `test_kind_reader_serves_the_generated_defaults` compare `rec` à `pico8_X_start` (31 lignes, M4b le prouve : `pico8_nr_start.s` changé seul, rouge à cette comparaison, `KIND_HASH` restant égal). Plis de forme : mots de genre (`PICO8_VALUE_*` pliés comme énumérateurs, lignes gagnant `valeur`, déclarées) dans `entity_kind`, `hp`, `hy`, `lq` ; 13 bits de présence par `ga` (BUILT_HASH) ; octet de genre de `t`/`me`/`fa` dans `ie`. `gcov` : 621 lignes de `src/` perdues : 487 d'infrastructure par nom (lecteurs `field` et `kind_field`, écrivains, `*_take_*`, `pico8_hy_hl`, `pico8_js_h_`, `pico8_lq_bg`, `value_is_nil`), 2 par un autre TU (`pico8_je`, `ui.c:212-213`, par `test_pico8_je`/`fm`/`js`), 131 par les e2e ; `collect.c:74` (`pico8_oj` sur le `fm` provisoire, que seul un test supprimé atteignait) reçoit une assertion dans `test_functions` de `test_pico8_hp`. M3 rougit `test_pico8_trade` par la partie (aucune route ne se forme), M4 par `KIND_HASH` avant la comparaison, d'où M4b ; R2, R3, R4, M5 comme prévu. Découvert à D.11d3 : `par-nom` `tests/` 371 → **166** lignes / 389 → 176 occ. (16 fichiers ; `src/` 292 inchangé ; total 458), pas 164 : le `fo` de la cible jetable `bv_et` (`g_bv_et`, struct privée de `pico8_probe.c`, sans lecteur typé) n'est lisible que par nom ; `test_pico8_g_` et `test_pico8_mz` le lisent dans une aide sans paramètre de nom (`bv_et_fo`, 1 ligne `pico8_entity_get_field` chacun), D.11e doit rendre la struct publique ou lui donner un accesseur avant de retirer `pico8_bv_et_field` (absent de son PRD), et la cible de D.11d4 passe à 9 sauf rattrapage. 207 lignes retirées (le PRD : 207 / 215), 2 ajoutées : (a1) 46, (a2) 161, (a3) 0, (b) = (c) = 0 ; `valeur` 3182 → 2295, `type-tag` 332 → 308. Tests supprimés : les 20 du PRD pour ces TU, plus 12 cas de mort d'écrivain par nom que l'inventaire n'avait pas listés (`test_fq_writer_stops_off_row`, `test_bg_writer_stops_off_row`, `test_fv_writer_stops_off_row`, `test_g__writer_stops_off_row`, `test_mz_writer_stops_off_row`, et dans `test_pico8_ui` `test_jo_writer_stops_off_row`, `test_jo_position_stops_off_row`, `test_jo_state_stops_off_row`, `test_jo_writers_stop_off_row`, `test_qh_writer_stops_off_row`, `test_hj_writer_stops_off_row`, `test_mx_writer_stops_off_row` : seule cible `write_off_row`/`write_m_by_name`, T2-M), donc 32 pour le pas ; `mz:812` (classé T1-W au PRD) est un cas de mort de l'écrivain générique sur `pico8_mz_jj_t`, qui n'a pas de membre `fo` : supprimé, (a2). Constantes gardées toutes égales ; plis de forme (mot de genre, bit de présence) dans `mh`, `fq`, `fv`, `g_`, `ui` (`PICO8_VALUE_STRING`/`FIXED` pliés par leur nom dans `me_of`/`fold_t`, six mots d'intrus par hachage `KE_ME`/`KE_T`) ; aucun dans `mz`, `bg`, `iv`. `gcov` : 992 lignes de `src/` perdues, 573 d'infrastructure par nom (lecteurs, écrivains, `*_take_*`, et `family_is_target_class`, `pico8_hj_pk`, `pico8_qh_et`, `pico8_g__et`, `mz_is_et_class`, dont le seul appelant est un lecteur ou un `*_take_*`), 406 par un autre TU, 8 par les e2e, 5 lignes sans instruction (accolade, blanc) ; `wormhole.c:243` (`pico8_mz_et` sans `et`) et `wormhole.c:47` (`fx` faux lu par le site C du prédicat de `ox`) reçoivent une assertion dans `check_fresh_mz` et `test_ox` de `test_pico8_mz` (constantes inchangées). R1 (ASan), R3, R4, R5, M6, M7 (`ui`, `bg`, `fq`, `fv`), M9 rougissent comme prévu. Découvert à D.11d4 (clôt D.11d) : `par-nom` `tests/` 166 → **11** lignes / 176 → 11 occ. (16 → 7 fichiers ; `src/` 292 / 352 inchangé ; total 303, 38 fichiers), pas 9 : le point souris que `jc:hq` pend sur `re` (`pico8_jc_mouse_point_t`, privé à `pico8_slipway.c:82`) n'a aucun lecteur typé de ses pointeurs `ra`/`p` (`pico8_jc_end_p`/`_ra` rendent des valeurs) ; `test_pico8_jc` les lit dans deux aides sans paramètre de nom (`mouse_point_ra`, `mouse_point_p`, classe assertée), dont dépendent `MOUSE_POINT_HASH`, `TOO_FAR_HASH`, `HQ_HASH` et les identités de `test_hq_mouse_point`. 157 lignes retirées / 167 occ. (le PRD : 157 / 167), 2 ajoutées : (a1) 31, (a2) 126, (a3) 0, (b) = (c) = 0 ; `je:72`, `mo:67` (T2-N à l'annexe C) sont typés comme `qb:323-325` (`pico8_je_nf() == NULL`, `pico8_mo_qb() == NULL`). `valeur` 2295 → 1833, `type-tag` 308 → 293. Tests supprimés : 12 (`js` 1, `jc` 4, `pj` 2, `qb` 2, `nf` 1, `fm` 1, `mo` 1) ; `test_js_writer_stops_off_row` et `test_jc_writer_stops_off_row` perdent leurs cas d'écrivain par nom et gardent leurs cas de construction (asserts de `pico8_js_new`/`pico8_jc_new`, clés typées), renommés `test_js_new_stops_off_row`/`test_jc_new_stops_off_qu`. Sur l'étape : 59 tests supprimés (d1 4, d2 11, d3 32, d4 12). Constantes gardées toutes égales (dont `JS_MEMBERS_HASH`, plié membre par membre) ; parties : `MEMBERS_HASH`, `FAR_MEMBERS_HASH`, `JC_MEMBERS_HASH`, `PJ_MEMBERS_HASH`, `QB_MEMBERS_HASH`, `NF_MEMBERS_HASH`. Plis de forme dans `js` (bit de présence et mot de genre des membres, 23 réponses 0 de l'écrivain brut, 10 intrus), `pj`, `jc` ; aucun ailleurs. `gcov` : 675 lignes de `src/` perdues, 322 d'infrastructure par nom (lecteurs, écrivains, `*_take_*`, et `pico8_g__et`, `pico8_pj_ed`, `pico8_js_h_`, `jc_is_fo_holder_class`, dont le seul appelant est un lecteur ou un `*_take_*`), 353 par un autre TU, 0 par les e2e ; `scroll.c:21` (`ph` sur un `v` nil) reçoit une assertion dans `test_qb_is_a_struct`. Couverture perdue sans reprise : les valeurs `hi`/`gq`/`ca` de la vue `ha_pz` (lecteur de struct-vue, T2 au PRD). M8, M10, M11 (et M11b sur `bu`) rougissent. **Pour D.11e** — lignes `par-nom` restantes dans `tests/` : T3 `test_pico8_iv.c:200`, `:205` (`fake_toast_field`, `bootstrap.c:87`), `test_pico8_mo.c:95` (`g_ra_only_class`, `entity_geom.c:49`), `test_pico8_ui.c:717`, `:718` (`giver_field`/`giver_set_field`, `ui.c:346`/`:359`/`:363`) ; exceptions `test_pico8_ie.c:95`, `:103` (`shared_gy`/`shared_id`), `test_pico8_g_.c:120` et `test_pico8_mz.c:148` (`bv_et_fo`), `test_pico8_jc.c:141`, `:151` (`mouse_point_ra`/`_p`). À exposer dans `src/` avant de retirer les lecteurs : `pico8_ie_gy()`/`pico8_ie_id()` (état partagé de `ie`), la struct `pico8_bv_et_t` ou un accesseur de son `fo`, la struct `pico8_jc_mouse_point_t` ou des accesseurs de ses pointeurs `ra`/`p` ; aucun n'est dans le PRD de D.11e. Tension §6 / ligne D.11d (tests de la couche supprimés ici, avant le code qu'ils testent) : tranchée pour la ligne, comme au PRD. La comparaison rec ↔ `pico8_X_start` reste portée par `test_kind_reader_serves_the_generated_defaults` (D.11d2).
> - D.11e — `par-nom`, suppression de l'infrastructure. Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`, laissé à D.11a ; accesseur typé par champ dans la table de classe, NULL = absent, Q6 ; l'oracle accesseurs = lecteurs naît et meurt dans l'étape). Découvert à D.11b (pour D.11d/D.11e) : les littéraux `s`/`od` des `pico8_X_kind_rec` doublent ceux de `pico8_X_start` ; seul `test_kind_rec_matches_the_reader` en garde l'égalité — à sa suppression, garder une comparaison rec ↔ `pico8_X_start` ou des constantes nommées partagées. Puis : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks. Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non. Q6 reconfirmée (humain, 2026-09-29) après la revue du plan : accesseurs typés par champ, pas de remontée dans `ke`, même si la phase 2 en remplace une partie. Découpé à la PRD en trois sous-étapes d'environ 50 min, même PRD, chacune vérifiée et poussée : D.11e1 (classe C, trois faux descripteurs de TU, oracle ; `src/` 292 → 271), D.11e2 (arbre jo, sonde, voies, emplacement de sorte ; oracle retiré ; `pico8_bv_et_t` et `pico8_jc_mouse_point_t` publics ; → 87), D.11e3 (interface, `pico8_ie_gy()`/`pico8_ie_id()`, la couche elle-même, garde += `par-nom` ; → 0).
> - D.12a — `liste`, registres : catégorie `liste` ajoutée à `tools/data_residue.sh` dans un premier commit seul, motif `\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h` (Q10 ; la règle de D.0 fige les motifs existants, mais une catégorie nouvelle s'ajoute dans son propre commit avec son motif énoncé) ; conteneur de pointeurs (Q9) sous un nom hors de ce motif ; registres L1 (`hv`, `cv[5]`, `cg[*]`, calques de `dc`, `ui.ic`, `ui.le`, `g_ga_by_shared`) et leurs parcours. Empreinte ≈ 15 (15, 0) : `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud`. Compte : `liste` ≤ 927 moins L1, rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
> - D.12b — `liste`, listes membres L2 (`jo.oe`, `jo.cc`/`by`, `trade.lp`/`rl`, `mz.ox`, proposition `ib`, `qy`, `jc.ts`, `hy.invented`, `jo.jy`/`hp.i` ; `gen.oc` en tableau fixe) et aides L4 de `collect` (`jp`, `nv`, `gu`/`gu_content`, `la_append` ; prédicat `int (*)(void *, void *ctx)`). Empreinte ≈ 25 (25, 0). Compte : `liste` rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
> - D.12c — `liste`, listes hétérogènes L3 et locales L4 : `lq.items`, `mh.items`, `jb.ls` → union étiquetée locale à `layout` {rien, fixe, chaîne, rangée, autre-vrai} (Q10), `lq.fs` en tableau typé de fixes ; listes locales d'affichage (`planet_display`, `menu`, `msgbox:pz`, `trade:kg`, `research*`, `score_panel*`, `tech_gate`, `slipway`, `probe`, `galaxy`) en tableaux fixes là où la borne est dans le cart. Empreinte ≈ 16 (15, 1). Compte : `liste` = 0 dans `src/` hors `pico8_list.c/.h`, `pico8_ref_audit.h` et `pico8_type_tag.h` (49 lignes aujourd'hui, à D.12d). §6 : `layout_nil_item_draws_nothing` passe son item par l'union, même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
> - D.12d — `liste`, TU (431 lignes, 38 fichiers ; `test_pico8_list` réécrit sur le nouveau conteneur, il ne perd pas son objet, §6), en deux chunks ; `pico8_list.c/.h` supprimés, `PICO8_REF_AS_LIST*` et `PICO8_TYPE_TAG_LIST` retirés. Empreinte 42 (4, 38). Compte : `liste` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non.

### §5, phases 2 et 3 (l. 282-300)

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

### §6 (l. 302-305)

> ## 6. Tests existants
>
> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
> - Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés** par ce plan. Depuis la fusion de master (e2e rapide, #32), seuls `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` restent ; les autres ont été remplacés par les e2e de `tests/e2e/`, qui en portent la garde. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.

### §7 (l. 307-332)

> ## 7. Exécution
>
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

## Annexe B — étude D.9, `pico8_list_t` (`docs/prd/done/plan3-phaseD-etape9-etude.md`, inchangée à `de6351e`)

### §3.1 et §3.2 (l. 120-137)

> ## 3. `pico8_list_t`
>
> ### 3.1 Inventaire (motif proposé en Q10 : 574 lignes / 63 fichiers dans `src/`, 413 / 38 dans `tests/`)
>
> | Classe | Listes | Éléments | Forme |
> |---|---|---|---|
> | **L1** — registres d'entités | `hv`, `cv[5]`, `cg[*]`, calques de `dc` (15), `ui.ic` (16), `ui.le` (partagée, puis tas), `g_ga_by_shared` | REF vers un en-tête d'entité | liste de pointeurs (Q9) ; `del` par identité, tolérance de `all()` (TROU-5) inchangées |
> | **L2** — listes membres des entités | `jo.oe` (`jc`), `jo.cc`/`by` (`trade`), `trade.lp` (`jo`)/`rl` (`jc`), `mz.ox`, proposition `ib`, `qy` (particules de `g_`/`hj`/`pm`), `jc.ts` (`pico8_ts_t`, D.7a), `hy.invented` (chaînes `px`), `jo.jy`/`hp.i` (`pico8_jy_t`, D.7a), `gen.oc` (entrées de `df`) | REF homogène, ou chaîne pour `invented` | liste de pointeurs, élément typé à la lecture (le cast `.as.ref` d'aujourd'hui devient un cast de `void *`) ; tableau fixe quand la borne est connue (`gen.oc`) |
> | **L3** — listes hétérogènes | `lq.items`, `mh.items`, `jb.ls` ; `lq.fs` (largeurs) | `lq.items` : chaîne, fixe, nil/faux, tuple REF (après D.7a une `pico8_jb_row_t`) ; une fonction ou un autre REF vrai compte comme le sprite 0 (`lq:mn`) | **union étiquetée locale** à `layout` : {rien, fixe, chaîne, rangée, autre-vrai} (Q10) ; `lq.fs` : tableau typé de fixes |
> | **L4** — temporaires et aides génériques | `jp` (filtre), `nv` (map), `gu`/`gu_content`, `la_append` (`collect.c`) ; une quarantaine de listes locales (`planet_display` `bs`/`ji`/`jx`/`prefix`…, `menu`, `msgbox:pz`, `trade:kg`, `research`, `research_display`, `score_panel*`, `tech_gate`, `slipway`, `probe`, `galaxy`) | REF, chaînes | aides typées sur la liste de pointeurs (prédicat `int (*)(void *, void *ctx)`) ; locales bornées en tableaux fixes quand la borne est dans le cart |
>
> `PICO8_REF_AS_LIST*` (`ref-audit`) et `PICO8_TYPE_TAG_LIST` (`type-tag`) tombent avec L1-L3. `test_pico8_list` n'est pas un test qui perd son objet : les sémantiques `del`/`all` de PICO-8 restent. Il est réécrit sur le nouveau conteneur (§6, premier point : il ne perd pas son objet).
>
> ### 3.2 Ce qui tombe déjà avec D.7a–D.8
>
> - Les aides `*_positional_values` (`help`, `layout`, `menu`, `research_display`, `score_panel`, `score_panel_display`, `tech_gate`) et `ix_items` de `cart:eu` lisent des tables ob : 46 lignes du motif, en comptant `pico8_fi`, `ft_index`/`ft_name`.
> - Le reste de `pico8_ob.c/.h` (D.8), et 5 lignes du test ponctuel (D.8).
> - **Au total ≈ 55 lignes.** D.7a ne change que le pointé de `lq.items`, `mh.items`, `jy`, `hp.i` et `ts`, et D.7b celui de `qy`. Ces listes restent L1-L3.

### Étapes proposées : en-tête, lignes D.12a à D.12d, règles (l. 179-204)

> ### Étapes proposées, avec leurs comptes
>
> Départ supposé : après D.8, `par-nom` ≈ 1008 et `dispatch` ≤ 199. Chaque PRD remesure. « ≈ » veut dire estimé par le classement de D.9, « ≤ » un majorant, « = » exact.
>
> | Étape | Contenu | Empreinte estimée | Compte après |
> |---|---|---|---|
> | **D.10a** | table de méthodes typée (23 emplacements nommés et la table d'états), 81 adaptateurs retirés, 26 appels `pico8_mv` remplacés (appel direct : `eb`, `ib`, `gg`, `md`, `lq`, `lh` ; virtuel pour le reste), `dg`/`dc`/`jz`/`lt`/`he` sur les emplacements, prédicats `*_is_*_class` sur les emplacements, `pico8_entity_get_field` ne rend plus de méthode, `hd=1` en fonction constante, sites G (9) et E `mv` (1) | ≈ 40 fichiers de `src/` (29 `dispatch`, plus les fichiers d'adaptateurs seuls) : **découper** a1 (méthodes de `dc`, `lt`, `he`, `jz`, `dg`) / a2 (le reste) | `dispatch` src ≤ 46 (76 − 30) ; `par-nom` ≈ 998 ; `valeur` en forte baisse, rapportée |
> | **D.10b** | rappels typés (`js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, rappels de `menu`/`jo:fe`/`de`, `eu`, `init`) ; `PICO8_VALUE_FUNCTION` et `pico8_value_function_t` retirés de `pico8_value.h` | ≈ 15 fichiers | `dispatch` src = 0 |
> | **D.10c** | TU du dispatch : 98 `pico8_mv` deviennent des appels typés, 25 rappels | 31 fichiers | `dispatch` = 0, dans `DATA_RESIDUE_GUARD` |
> | **D.11a** | `par-nom` A et D (41 + 8 lignes, 57 appels d'aides), E `je.l`/`je.r`, aides H mortes (39) | ≈ 22 fichiers | `par-nom` ≈ 909 |
> | **D.11b** | B (test d'appartenance, 7 + 1), C (accesseurs transversaux, 16 + 6), F (cadre de `pz`, 5 + 2), K (sortes, 4) | ≈ 14 fichiers | `par-nom` ≈ 877 |
> | **D.11c** | M selon Q7 (composants émetteur et sonde) | `particle`, `probe`, plus les 13 fichiers de classes qui les portent | `par-nom` ≈ 861 |
> | **D.11d** | TU de `par-nom` : T1 en accès aux membres, T2 supprimés (Q11), deux §6 réécrits avec leur preuve rouge | 38 fichiers : **découper** en deux (≈ 19 chacun) | `par-nom` ≈ 286 |
> | **D.11e** | suppression de l'infrastructure : 40 lecteurs, 18 écrivains, 10 `kind_field` et les emplacements des 11 sortes, 55 aides `*_take_*` (43 après D.7a), `pico8_entity_get_field`/`set_field`, typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les 11 structs-vues deviennent typées ou disparaissent : remplaçants `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view` | ≈ 35 fichiers de `src/` | `par-nom` = 0, dans `DATA_RESIDUE_GUARD` |
> | **D.12a** | catégorie `liste` ajoutée (Q10) ; conteneur de pointeurs (Q9) ; registres L1 et leurs parcours | `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud` (≈ 15) | `liste` ≤ 987 − 55 (D.7a-D.8) − L1, rapporté |
> | **D.12b** | listes membres L2 et aides L4 de `collect` | ≈ 25 fichiers | rapporté |
> | **D.12c** | listes hétérogènes L3 (union locale) et locales d'affichage L4 | ≈ 15 fichiers | `liste` src = 0 |
> | **D.12d** | TU (413 lignes, 38 fichiers ; `test_pico8_list` réécrit) : **découper** en deux ; `PICO8_REF_AS_LIST*`, `PICO8_TYPE_TAG_LIST` retirés | 38 | `liste` = 0, dans `DATA_RESIDUE_GUARD` |
>
> Pour chaque étape :
>
> - `class-coverage-net` passe si l'étape touche un constructeur : D.10a, D.11c, D.11e.
> - `sanitize-net` passe à chaque étape qui change la durée de vie ou le propriétaire d'une donnée : D.11c, D.12a-c.
> - §6 : les deux tests restants de la liste sont réécrits dans D.11d, et `layout_nil_item_draws_nothing` dans D.12c, qui lui fait passer son item par l'union ; chacun avec sa preuve rouge.
> - Aucun `*.sha256` ni e2e ne change.
>

### Q9 et Q10 (l. 235-239)

> **Q9. Conteneur des listes : (A) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité), plus des tableaux fixes typés là où la borne est dans le cart ; (B) un type de liste par type d'élément (macro) ; (C) garder `pico8_list_t` de `pico8_value_t` ?**
> A. La tolérance de `all()` à la suppression (TROU-5) et la sémantique de `del` restent écrites une seule fois, et le cast `void *` au site remplace le cast `.as.ref` d'aujourd'hui. B multiplie ces sémantiques. C laisse survivre `pico8_value.c`, donc la Sortie de D devient impossible.
>
> **Q10. Ajouter une catégorie `liste` au compteur (motif `\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h`, 987/101 à `2dec71f`), dans le premier commit de D.12a, et une union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ?**
> Oui pour les deux. Sans catégorie, les étapes D.12 n'ont pas de « Compte », contrairement à la règle de la liste de D. L'union locale ne porte que les cinq cas que `lq:mn` distingue. Le nouveau conteneur doit prendre un nom hors du motif (règle commune : un nom nouveau ne correspond à aucun motif de la catégorie qu'il remplace).

## Annexe C — lignes `liste` des fichiers de `src/` de l'étape (`de6351e`, motif de Q10 ; ⟲ = fichier que D.11e modifie, numéros à remesurer par contenu)

Classe : **a1** / **a2** = ligne L1 que la sous-étape convertit ou retire ; **inc** = `#include "pico8_list.h"` (retiré d'un `.c` devenu sans usage, gardé dans un en-tête dont d'autres dépendent : « Registres », Includes) ; **reste** = liste L2/L3/L4, hors D.12a. `collect.c`/`.h` : leurs lignes `liste` sont les aides à valeur, gardées (les aides `_ptrs` sont ajoutées sans ligne `liste`). Hors de ces fichiers, D.11e retire 7 lignes (`planet.c:137`, `:143`, `:218` ; `planet_family.c:495`, `:501` ; `probe.c:594`, `:600`), marquées « (D.11e) » pour `probe.c` (H-D11e-3).

#### `src/pico8_entity.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 17 | a2 | `pico8_list_t list;` |
| 27 | a2 | `pico8_list_t list;` |
| 34 | a2 | `static pico8_list_t g_hv;` |
| 95 | a2 | `pico8_list_init(&g_cg[g_cg_count].list);` |
| 109 | a2 | `pico8_list_free(&g_hv);` |
| 112 | a2 | `pico8_list_free(&g_cv[i].list);` |
| 117 | a2 | `pico8_list_free(&g_cg[i].list);` |
| 136 | a2 | `pico8_list_init(&g_hv);` |
| 140 | a2 | `pico8_list_init(&g_cv[i].list);` |
| 148 | a2 | `const pico8_list_t *pico8_hv(void) {` |
| 152 | a2 | `const pico8_list_t *pico8_cv(const char *name) {` |
| 193 | a2 | `const pico8_list_t *pico8_cg(const char *tag) {` |
| 227 | a2 | `pico8_list_add(&g_hv, pico8_value_ref(e));` |
| 231 | a2 | `pico8_list_add(&g_cv[i].list, pico8_value_ref(e));` |
| 241 | a2 | `pico8_list_add(&bucket->list, pico8_value_ref(e));` |
| 256 | a2 | `pico8_list_del(&g_hv, pico8_value_ref(e));` |
| 260 | a2 | `pico8_list_del(&g_cv[i].list, pico8_value_ref(e));` |
| 271 | a2 | `pico8_list_del(&bucket->list, pico8_value_ref(e));` |
| 278 | a2 | `const pico8_list_t *bucket_src;` |
| 279 | a1 | `pico8_list_t layers[15];` |
| 286 | a1 | `pico8_list_init(&layers[i]);` |
| 307 | a1 | `pico8_list_add(&layers[layer - 1], pico8_value_ref(qc));` |
| 312 | a1 | `pico8_list_t *bucket = &layers[o - 1];` |
| 339 | a1 | `pico8_list_free(&layers[i]);` |

#### `src/pico8_entity.h` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 5 | inc | `#include "pico8_list.h"` |
| 27 | a2 | `const pico8_list_t *pico8_hv(void);` |
| 33 | a2 | `const pico8_list_t *pico8_cv(const char *name);` |
| 39 | a2 | `const pico8_list_t *pico8_cg(const char *tag);` |

#### `src/pico8_bootstrap.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 8 | inc | `#include "pico8_list.h"` |
| 64 | a2 | `const pico8_list_t *hv = pico8_hv();` |

#### `src/pico8_bg.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 222 | a2 | `const pico8_list_t *g_tag;` |

#### `src/pico8_economy.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 93 | reste | `pico8_list_t *invented;` |
| 119 | reste | `pico8_list_init(invented);` |
| 198 | a2 | `const pico8_list_t *hk_list;` |
| 199 | a2 | `pico8_list_iter_t it;` |
| 227 | a2 | `it = pico8_list_all(hk_list);` |
| 228 | a2 | `while (pico8_list_iter_next(&it, &e)) {` |
| 276 | a2 | `const pico8_list_t *jo_list;` |
| 277 | a2 | `pico8_list_iter_t it;` |
| 300 | a2 | `it = pico8_list_all(jo_list);` |
| 301 | a2 | `while (pico8_list_iter_next(&it, &e)) {` |
| 338 | a2 | `const pico8_list_t *hb_list = pico8_cv("hb");` |
| 340 | a2 | `pico8_list_iter_t it2;` |
| 342 | a2 | `it2 = pico8_list_all(hb_list);` |
| 343 | a2 | `while (pico8_list_iter_next(&it2, &e2)) {` |
| 370 | reste | `pico8_list_add(self->invented, pico8_value_string(t->px));` |

#### `src/pico8_economy_formulas.c`

| Ligne | Classe | Texte |
|---|---|---|
| 51 | a2 | `const pico8_list_t *list = pico8_cg(tag);` |
| 52 | a2 | `pico8_list_iter_t it;` |
| 57 | a2 | `it = pico8_list_all(list);` |
| 58 | a2 | `while (pico8_list_iter_next(&it, &e)) {` |
| 102 | reste | `const pico8_list_t *jy;` |
| 145 | a2 | `const pico8_list_t *jc = pico8_cg("jc");` |

#### `src/pico8_economy_score.c`

| Ligne | Classe | Texte |
|---|---|---|
| 29 | a2 | `const pico8_list_t *list = pico8_cg(tag);` |
| 30 | a2 | `pico8_list_iter_t it;` |
| 35 | a2 | `it = pico8_list_all(list);` |
| 36 | a2 | `while (pico8_list_iter_next(&it, &e)) {` |
| 55 | a2 | `const pico8_list_t *qh_list = pico8_cg("qh");` |
| 78 | a2 | `const pico8_list_t *jo_list = pico8_cg("jo");` |
| 79 | a2 | `pico8_list_t filtered = pico8_jp(jo_list, cz_predicate, NULL);` |
| 82 | a2 | `pico8_list_free(&filtered);` |
| 90 | reste | `const pico8_list_t *by;` |
| 199 | reste | `const pico8_list_t *invented = fm != NULL ? fm->invented : NULL;` |

#### `src/pico8_economy_tiers.c`

| Ligne | Classe | Texte |
|---|---|---|
| 35 | a2 | `const pico8_list_t *list = pico8_cg(tag);` |
| 36 | a2 | `pico8_list_iter_t it;` |
| 41 | a2 | `it = pico8_list_all(list);` |
| 42 | a2 | `while (pico8_list_iter_next(&it, &e)) {` |
| 51 | reste | `pico8_list_t *cc;` |
| 59 | reste | `pico8_list_free(jo->cc);` |
| 64 | reste | `pico8_list_init(cc);` |
| 70 | reste | `pico8_list_t *by;` |
| 71 | reste | `pico8_list_iter_t it;` |
| 87 | reste | `it = pico8_list_all(by);` |
| 88 | reste | `while (pico8_list_iter_next(&it, &e)) {` |
| 90 | reste | `const pico8_list_t *rl;` |
| 91 | reste | `const pico8_list_t *lp;` |
| 145 | reste | `pico8_list_add(dest->cc, e);` |
| 180 | reste | `pico8_list_add(jo->jy, pico8_value_ref(entry));` |
| 222 | reste | `const pico8_list_t *jy;` |
| 223 | reste | `const pico8_list_t *cc;` |
| 309 | reste | `const pico8_list_t *by = jo->by;` |

#### `src/pico8_probe.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 244 | a2 | `pico8_list_t filtered;` |
| 254 | a2 | `pico8_list_free(&filtered);` |
| 594 | (D.11e) | `static void g__take_list(pico8_value_t value, pico8_list_t **out) {` |
| 600 | (D.11e) | `pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_LIST &&` |

#### `src/pico8_score_panel.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 16 | reste | `#include "pico8_list.h"` |
| 159 | a2 | `pico8_list_t filtered;` |
| 165 | a2 | `pico8_list_free(&filtered);` |
| 280 | reste | `pico8_list_t combined;` |
| 283 | reste | `pico8_list_init(&combined);` |
| 285 | reste | `pico8_list_add(&combined, pico8_value_ref((void *)i_->items[i].row));` |
| 288 | reste | `pico8_list_add(&combined, pico8_value_ref((void *)ou_state->row.items[i].row));` |
| 293 | reste | `pico8_list_free(&combined);` |

#### `src/pico8_slipway.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 203 | reste | `pico8_list_t *ts;` |
| 212 | reste | `pico8_list_init(ts);` |
| 377 | reste | `const pico8_list_t *oe = NULL;` |
| 384 | reste | `pico8_list_iter_t it = pico8_list_all(oe);` |
| 386 | reste | `while (pico8_list_iter_next(&it, &w_val)) {` |
| 405 | a2 | `pico8_list_t part1;` |
| 406 | a2 | `pico8_list_t part2;` |
| 407 | a2 | `pico8_list_t ds;` |
| 420 | a2 | `pico8_list_free(&part1);` |
| 421 | a2 | `pico8_list_free(&part2);` |
| 426 | a2 | `pico8_list_free(&ds);` |

#### `src/pico8_wormhole.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 77 | a2 | `pico8_list_t *heap_ox;` |
| 182 | a2 | `const pico8_list_t *jo_list;` |

#### `src/pico8_wormhole.h` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 8 | inc | `#include "pico8_list.h"` |
| 37 | a2 | `pico8_list_t *ox;` |

#### `src/pico8_ui.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 11 | inc | `#include "pico8_list.h"` |
| 25 | a1 | `static pico8_list_t g_shared_le;` |
| 31 | a1 | `pico8_list_t *pico8_ui_le_shared(void) {` |
| 165 | a2 | `pico8_list_init(&self->ic_own.layers[i]);` |
| 181 | a1 | `pico8_list_init(&g_shared_le);` |
| 250 | a1 | `pico8_list_t *le_ptr_before;` |
| 321 | a2 | `const pico8_list_t *hd_bucket;` |
| 330 | a2 | `pico8_list_free(&self->ic_own.layers[i]);` |
| 331 | a2 | `pico8_list_init(&self->ic_own.layers[i]);` |
| 351 | a2 | `pico8_list_add(&self->ic->layers[fa], pico8_value_ref(e));` |
| 457 | a2 | `const pico8_list_t *bucket = &self->ic->layers[mf];` |
| 505 | a1 | `pico8_list_t *heap_le;` |
| 512 | a1 | `pico8_list_t discarded = pico8_nv(self->le, pico8_ui_cj_he_mapper, NULL);` |
| 513 | a1 | `pico8_list_free(&discarded);` |
| 518 | a1 | `pico8_list_init(heap_le);` |

#### `src/pico8_ui.h` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 8 | inc | `#include "pico8_list.h"` |
| 24 | a2 | `pico8_list_t layers[PICO8_UI_IC_LAYERS];` |
| 36 | a1 | `pico8_list_t *le;` |
| 68 | a1 | `pico8_list_t *pico8_ui_le_shared(void);` |

#### `src/pico8_msgbox.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 126 | reste | `pico8_list_init(&self->items);` |
| 129 | reste | `pico8_list_add(&self->items, keys->items[i]);` |
| 358 | a1 | `pico8_list_t *le_heap;` |
| 406 | a1 | `pico8_list_init(le_heap);` |
| 408 | a1 | `pico8_list_add(le_heap, pico8_value_ref(pico8_pz_step(&ctx, &ep_row)));` |
| 411 | a1 | `pico8_list_add(le_heap, pico8_value_ref(pico8_pz_step(&ctx, &bs[i])));` |
| 416 | a1 | `pico8_list_free(le_heap);` |
| 424 | a1 | `pico8_list_t *empty_heap = malloc(sizeof *empty_heap);` |
| 425 | a1 | `pico8_list_init(empty_heap);` |

#### `src/pico8_entity_geom.c` ⟲

| Ligne | Classe | Texte |
|---|---|---|
| 87 | a2 | `pico8_fixed_t pico8_ij(pico8_vec_t p, const pico8_list_t *j_, pico8_value_t *out_nearest) {` |
| 89 | a2 | `pico8_list_iter_t it;` |
| 96 | a2 | `it = pico8_list_all(j_);` |
| 97 | a2 | `while (pico8_list_iter_next(&it, &e)) {` |

#### `src/pico8_entity_geom.h`

| Ligne | Classe | Texte |
|---|---|---|
| 5 | inc | `#include "pico8_list.h"` |
| 31 | a2 | `pico8_fixed_t pico8_ij(pico8_vec_t p, const pico8_list_t *j_, pico8_value_t *out_nearest);` |

#### `src/pico8_collect.c`

| Ligne | Classe | Texte |
|---|---|---|
| 8 | reste | `int pico8_gu(const pico8_list_t *qk, pico8_value_t kb) {` |
| 9 | reste | `pico8_list_iter_t it;` |
| 14 | reste | `it = pico8_list_all(qk);` |
| 15 | reste | `while (pico8_list_iter_next(&it, &e)) {` |
| 23 | reste | `pico8_list_t pico8_jp(const pico8_list_t *qk, pico8_jp_predicate_fn ow, void *ctx) {` |
| 24 | reste | `pico8_list_t f;` |
| 25 | reste | `pico8_list_iter_t it;` |
| 27 | reste | `pico8_list_init(&f);` |
| 31 | reste | `it = pico8_list_all(qk);` |
| 32 | reste | `while (pico8_list_iter_next(&it, &e)) {` |
| 34 | reste | `pico8_list_add(&f, e);` |
| 40 | reste | `pico8_list_t pico8_nv(const pico8_list_t *qk, pico8_nv_fn kp, void *ctx) {` |
| 41 | reste | `pico8_list_t m;` |
| 42 | reste | `pico8_list_iter_t it;` |
| 45 | reste | `pico8_list_init(&m);` |
| 46 | reste | `it = pico8_list_all(qk);` |
| 47 | reste | `while (pico8_list_iter_next(&it, &e)) {` |
| 50 | reste | `pico8_list_add(&m, v);` |
| 57 | reste | `pico8_list_add((pico8_list_t *)ctx, value);` |
| 60 | reste | `pico8_list_t pico8_la(const pico8_list_t *a, const pico8_list_t *b) {` |
| 61 | reste | `pico8_list_t t;` |
| 63 | reste | `pico8_list_init(&t);` |
| 64 | reste | `pico8_list_foreach(a, pico8_la_append, &t);` |
| 65 | reste | `pico8_list_foreach(b, pico8_la_append, &t);` |
| 70 | reste | `int pico8_gu_content(const pico8_list_t *qk, pico8_value_t kb) {` |
| 71 | reste | `pico8_list_iter_t it;` |
| 76 | reste | `it = pico8_list_all(qk);` |
| 77 | reste | `while (pico8_list_iter_next(&it, &e)) {` |

#### `src/pico8_collect.h`

| Ligne | Classe | Texte |
|---|---|---|
| 6 | reste | `#include "pico8_list.h"` |
| 12 | reste | `int pico8_gu(const pico8_list_t *qk, pico8_value_t kb);` |
| 16 | reste | `* caller releases with pico8_list_free(); a NULL `qk` gives an empty one. */` |
| 18 | reste | `pico8_list_t pico8_jp(const pico8_list_t *qk, pico8_jp_predicate_fn ow, void *ctx);` |
| 23 | reste | `* pico8_list_free(). `qk` must not be NULL. */` |
| 25 | reste | `pico8_list_t pico8_nv(const pico8_list_t *qk, pico8_nv_fn kp, void *ctx);` |
| 28 | reste | `* caller releases with pico8_list_free(). Neither argument may be NULL. */` |
| 29 | reste | `pico8_list_t pico8_la(const pico8_list_t *a, const pico8_list_t *b);` |
| 34 | reste | `int pico8_gu_content(const pico8_list_t *qk, pico8_value_t kb);` |

#### `src/pico8_galaxy.c`

| Ligne | Classe | Texte |
|---|---|---|
| 46 | reste | `pico8_list_t hr_list;` |
| 102 | reste | `pico8_list_init(&hr_list);` |
| 104 | reste | `pico8_list_add(&hr_list, pico8_value_ref((void *)&pico8_df_table[i]));` |
| 108 | reste | `pico8_list_free(&hr_list);` |

#### `src/pico8_cart.c`

| Ligne | Classe | Texte |
|---|---|---|
| 25 | reste | `#include "pico8_list.h"` |


## Annexe D — lectures de registres dans les TU (`de6351e` ; ⟲ = TU que D.11e modifie, numéros à remesurer par contenu)

Colonnes : lignes qui appellent `pico8_hv()`/`pico8_cv()`/`pico8_cg()` ; lignes qui lisent `ok->le`, `ok->ic`, `pico8_ui_le_shared()`, `ok_le()` ou `le_of()` ; aides locales à paramètre `const pico8_list_t *` (ligne de leur définition). Les lectures d'éléments qui suivent ces lignes (`->items[i].as.ref`, `->count`) sont dans les mêmes fonctions.

| TU | Sous-étape | `hv`/`cv`/`cg` (n : lignes) | `le`/`ic` (n : lignes) | Aides `const pico8_list_t *` |
|---|---|---|---|---|
| `test_pico8_bg.c` | a2 (`ic`) | 13 : 145, 203, 204, 219, 315, 316, 324, 330, 333, 334, 393, 416, 439 | 1 : 338 | 82, 92 |
| `test_pico8_fq.c` | a2 (`ic`) | 9 : 87, 88, 136, 148, 171, 192, 286, 287, 289 | 1 : 197 | 64 |
| `test_pico8_fr.c` | a2 | 9 : 120, 121, 196, 350, 352, 355, 357, 563, 578 | 0 : — | 73, 83, 142 |
| `test_pico8_fv.c` | a1 (`le`) + a2 (`ic`, registres) | 13 : 145, 146, 258, 308, 387, 389, 390, 391, 392, 393, 424, 518, 519 | 3 : 273, 291, 400 | 67, 77, 208 |
| `test_pico8_g_.c` ⟲ | a2 | 23 : 129, 130, 277, 316, 378, 380, 382, 383, 384, 385, 386, 437, 478, 479, 586, 592, 593, 594, 684, 711, 712, 713, 759 | 0 : — | 80, 90, 94, 231 |
| `test_pico8_ga.c` | a2 | 15 : 122, 123, 221, 253, 259, 262, 278, 279, 283, 397, 399, 402, 601, 603, 619 | 0 : — | 71, 81, 134 |
| `test_pico8_ga_export_panel_by.c` | a1 | 0 : — | 4 : 37, 38, 125, 150 | — |
| `test_pico8_hj.c` | a2 | 10 : 128, 129, 276, 423, 425, 428, 430, 575, 645, 656 | 0 : — | 76, 86, 154 |
| `test_pico8_hp.c` | a2 | 11 : 146, 369, 646, 714, 717, 719, 738, 757, 760, 924, 928 | 0 : — | 77 |
| `test_pico8_hy.c` | a2 | 12 : 263, 305, 410, 417, 425, 444, 446, 447, 449, 451, 467, 484 | 0 : — | — |
| `test_pico8_ie.c` ⟲ | a2 | 8 : 160, 161, 177, 316, 317, 318, 350, 351 | 0 : — | 72, 82 |
| `test_pico8_iv.c` ⟲ | a2 | 9 : 118, 127, 132, 216, 222, 226, 266, 267, 269 | 0 : — | 71 |
| `test_pico8_jc.c` ⟲ | a2 | 18 : 179, 180, 212, 228, 231, 246, 249, 252, 307, 358, 405, 409, 444, 493, 494, 495, 533, 785 | 0 : — | 82, 92, 258 |
| `test_pico8_je.c` | a2 | 2 : 51, 55 | 0 : — | 35 |
| `test_pico8_jo.c` | a2 | 15 : 136, 137, 269, 271, 274, 286, 379, 416, 420, 443, 464, 481, 486, 568, 586 | 0 : — | 71, 81, 146 |
| `test_pico8_js.c` | a1 (`le`) + a2 | 18 : 227, 228, 382, 384, 442, 487, 526, 575, 598, 704, 733, 914, 917, 919, 1028, 1395, 1521, 1551 | 18 : 332, 386, 390, 394, 407, 588, 622, 656, 678, 905, 934, 1198, 1223, 1440, 1476, 1510, 1694, 1698 | 96, 106 |
| `test_pico8_lq.c` | a1 (`le`) + a2 | 1 : 610 | 9 : 254, 357, 384, 532, 658, 667, 683, 685, 693 | — |
| `test_pico8_mh.c` | a2 | 16 : 100, 101, 102, 128, 163, 164, 177, 182, 183, 234, 272, 278, 280, 285, 341, 343 | 0 : — | 76, 86 |
| `test_pico8_mo.c` ⟲ | a2 | 3 : 62, 64, 79 | 0 : — | 40 |
| `test_pico8_mx.c` | a2 | 9 : 121, 122, 250, 419, 421, 424, 426, 662, 674 | 0 : — | 74, 84, 143 |
| `test_pico8_mz.c` ⟲ | a2 | 23 : 165, 168, 228, 290, 343, 344, 345, 354, 355, 359, 360, 378, 393, 447, 453, 474, 502, 560, 598, 606, 607, 611, 737 | 0 : — | 78, 88 |
| `test_pico8_nf.c` | a2 | 4 : 60, 159, 279, 280 | 0 : — | 45 |
| `test_pico8_nl.c` | a2 | 9 : 116, 117, 234, 361, 363, 366, 368, 516, 526 | 0 : — | 69, 79, 138 |
| `test_pico8_nr.c` | a2 | 10 : 117, 118, 206, 361, 363, 366, 368, 507, 518, 539 | 0 : — | 70, 80, 139 |
| `test_pico8_os.c` | a2 | 10 : 126, 127, 222, 270, 386, 388, 391, 393, 568, 577 | 0 : — | 74, 84, 152 |
| `test_pico8_pj.c` | a1 (`le`) + a2 | 20 : 191, 192, 311, 321, 337, 357, 358, 359, 360, 361, 362, 363, 365, 398, 444, 445, 470, 471, 472, 475 | 3 : 526, 534, 535 | 73, 83, 87 |
| `test_pico8_planet_gh_row_lq.c` | a1 | 0 : — | 3 : 46, 47, 134 | — |
| `test_pico8_planet_kj_dispatch.c` | a2 | 1 : 71 | 0 : — | — |
| `test_pico8_pm.c` | a2 | 9 : 117, 118, 278, 434, 436, 439, 441, 563, 571 | 0 : — | 70, 80, 139 |
| `test_pico8_qb.c` | a2 | 5 : 69, 145, 260, 306, 368 | 0 : — | 54 |
| `test_pico8_qh.c` | a2 | 15 : 146, 147, 253, 360, 362, 365, 433, 435, 482, 559, 611, 614, 616, 665, 678 | 0 : — | 74, 84, 184 |
| `test_pico8_spec_defaults.c` | a1 | 0 : — | 2 : 53, 67 | — |
| `test_pico8_trade.c` | a2 | 3 : 107, 231, 247 | 0 : — | 61, 219, 319 |
| `test_pico8_ui.c` ⟲ | a1 (`le`) + a2 (`ic`, registres) | 34 : 149, 229, 259, 280, 281, 282, 283, 285, 288, 323, 324, 333, 338, 367, 404, 448, 452, 524, 547, 548, 557, 563, 580, 581, 583, 587, 589, 611, 617, 622, 623, 745, 919, 999 | 29 : 4, 107, 108, 109, 153, 157, 297, 303, 305, 314, 319, 322, 327, 328, 331, 332, 333, 334, 337, 353, 361, 576, 585, 609, 618, 620, 621, 666, 668 | 93, 103 |
