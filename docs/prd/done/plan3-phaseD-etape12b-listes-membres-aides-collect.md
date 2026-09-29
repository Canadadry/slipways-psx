---
title: "Plan 3, étape D.12b : listes membres L2 (`jo.oe`, `jo.jy`/`hp.i`, `jo.cc`/`by` avec `g_ga_by_shared`, `trade.lp`/`rl`, proposition `ib`, `emitter->qy`, `jc.ts`, `hy.invented`) sur `pico8_ptrs_t`, `gen.oc` en tableau fixe ; `pico8_gu`, `pico8_gu_content`, `pico8_la`, `pico8_la_append` à valeur retirés ; les TU suivent dans le même commit ; deux sous-étapes"
description: "Après D.12a, les registres sont des conteneurs de pointeurs mais les listes membres des entités (routes, voies, besoins, particules, techs inventées, tickets de `gn`) restent des `pico8_list_t` de `pico8_value_t` REF qu'on déballe à chaque lecture. D.12b les fait passer sur `pico8_ptrs_t` (Q9), `gen.oc` en tableau fixe typé, type les API qui ne portaient une valeur que pour la liste (`pico8_trade_kg`/`_to`, `kf` de la proposition, `pico8_gn_next`, `pico8_jo_ib`, `pico8_jo_ex`), retire les aides de `collect` à valeur devenues sans appelant, et fait suivre les 25 TU qui lisent ces listes : `liste` baisse d'environ 230 lignes, rapporté."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.12a était implémentée** (`plan-pilot`, étape 2b), sur l'arbre **commis** `f2d5dad`, puis **révisé après sa graduation** : D.12a gradée (a1 `6ba781e` + `83e0ec0`, a2 `546ffe3`, VERIFIED ; « docs: D.12a gradée (registres sur pico8_ptrs_t) » `6cf4df4`). Les hypothèses `H-D12a-n` sont confrontées plus bas à `6cf4df4` et aux notes « Découvert à D.12a1/a2 » ; tous les numéros de ligne de ce PRD (Inventaire, décisions, mutations, Annexes C et D) ont été **remesurés par contenu** sur `git archive 6cf4df4` ; les marques de la première version sont retirées. Seuls `grep`, `sed`, `awk` et une recette de mesure (motif de Q10, recopiée du PRD de D.12a) ont tourné. **Rien n'a été compilé ni lancé.** Les numéros de ligne sont ceux de `6cf4df4` ; depuis `f2d5dad`, seuls ceux de `collect.c/.h` et de deux TU (`mx`, `ga_export_panel_by`) ont bougé.

Le problème, vu du mainteneur du port : `pico8_value.c` ne peut disparaître tant qu'une `pico8_list_t` tient des `pico8_value_t`. Après D.12a, les registres ne sont plus concernés, mais chaque entité porte encore ses listes membres sous cette forme : les voies d'une planète (`jo.oe`), ses besoins (`jo.jy`, copiés de `hp.i`), ses routes reçues et émises (`jo.cc`, `jo.by`, et le défaut partagé des `ga`, `g_ga_by_shared`), les planètes et voies d'une route (`trade.lp`/`rl`), la liste que partagent les propositions d'un même `jo:ib` (`ib`), les particules d'un émetteur (`emitter->qy`) et d'une voie (`jc.ts`), les techs inventées (`hy.invented`), les tickets de `gn` (`gen.oc`). Chaque lecture teste un genre qui ne peut valoir que REF (ou chaîne pour `invented`) puis déballe `.as.ref`. Et quatre aides de `collect` (`pico8_gu`, `pico8_gu_content`, `pico8_la`, `pico8_la_append`) n'existent plus que pour ces listes.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `6cf4df4`, inchangée depuis `f2d5dad` (§5, « Phase D — Données », l. 278) :

> - D.12b — `liste`, listes membres L2 (`jo.oe`, `jo.cc`/`by`, `trade.lp`/`rl`, `mz.ox`, proposition `ib`, `qy`, `jc.ts`, `hy.invented`, `jo.jy`/`hp.i` ; `gen.oc` en tableau fixe) et aides L4 de `collect` (`jp`, `nv`, `gu`/`gu_content`, `la_append` ; prédicat `int (*)(void *, void *ctx)`). Empreinte ≈ 25 (25, 0). Compte : `liste` rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.

Décisions de D.9, recopiées (l. 265) :

> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Paragraphe « Liste de D », empreinte et règles communes, ordre, recopiés (l. 237, 239, 241) :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.

> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Comptes au départ de D.10 et règle des ponts de TU, recopiés (l. 267) :

> Comptes au départ de D.10 (HEAD 5a13008, `make -s data-residue` et `--detail` ; l'étude mesurait `2dec71f`, avant D.7a–D.8) : `valeur` 4961/126, `type-tag` 352/65, `ref-audit` 56/15, `par-nom` 1009/81 (`src/` 433/43, `tests/` 576/38), `dispatch` 191/61 (`src/` 67/30, `tests/` 124/31), résidu D 5868 lignes / 128 fichiers ; `liste`, pas encore au compteur, 927/99 avec le motif de Q10 (`src/` 496/61, `tests/` 431/38). Les classes A–M, T1/T2 et L1–L4 sont celles de l'étude (annexe A) ; ses « ≈ » viennent de son classement, chaque PRD remesure. Un TU qui nomme un symbole retiré par une étape de `src/` la suit dans le même commit, ou le symbole reste comme pont compté, fermé par l'étape TU de son groupe (D.10c, D.11d, D.12d) ; le PRD tranche. `sanitize-net` : aux seules étapes qui changent une durée de vie ou un propriétaire (§7).

Toutes les autres lignes de la phase D (D.0 à D.12d) **avec toutes leurs notes « Découvert à D.x »** (D.0, D.1, D.7c, D.8, D.10a, D.10b, D.10c, D.11a, D.11b, D.11c, D.11d1 à D.11d4, D.11e1 à D.11e3, et la décision humaine qui découpe D.11d), l'arrêt « champ sans règle », les parties du §5 qui fixent les règles (en-tête et phase 1a, phase 1b, phase D en entier, phases 2 et 3), le §6 et le §7 entiers sont recopiés tels quels, par extraction de lignes, en **Annexe A**. Les passages de l'étude D.9 sur `pico8_list_t` (§3.1, §3.2, point 2 du §4, table des étapes, Q9, Q10) sont en **Annexe B**. L'**Annexe C** est l'inventaire ligne à ligne des lignes `liste` des fichiers de `src/` que l'étape touche, classées ; l'**Annexe D**, celui des TU.

Notes « Découvert à » qui visent D.12b, extraites de ces textes :

- D.11c : « Pour D.12b : `qy` est `emitter->qy`. »
- D.11e3, « Pour D.12 » : « `pico8_ij` lit `p` par `get_p` et garde `pico8_value_t *out_nearest` ; le prédicat de `mz.ox` garde `(pico8_value_t e, void *ctx)` » (tous deux pris par D.12a) ; remarque de clôture : « `pico8_lq_bg` et `pico8_pj_ed` ne servent plus qu'aux TU et rendent un `pico8_value_t` ».
- PRD de D.12a, « Pour D.12b » : « les aides à valeur `pico8_jp` (3 appelants), `pico8_la` (6), `pico8_gu` (1), `pico8_nv` (2), `pico8_gu_content` (1) ; le nom définitif des aides `_ptrs` ; `g_ga_by_shared` avec `jo.by` ; les aides locales de TU scindées sur `jo.oe`, `jc.ts`, `qy` ».

### Hypothèses `H-D12a-n` (D.12a en cours d'implémentation)

Source : le PRD de D.12a (`docs/prd/triage/plan3-phaseD-etape12a-liste-registres-conteneur-de-pointeurs.md` à `f2d5dad`) et sa ligne du plan (deux sous-étapes a1/a2). Chaque hypothèse dit ce qui en dépend ici et comment la vérifier à la graduation.

| Hypothèse | Énoncé | Ce qui en dépend dans ce PRD | Vérification à la graduation |
|---|---|---|---|
| H-D12a-1 | La catégorie `liste` est au compteur (motif de Q10, dernière de `CATEGORIES`), hors `DATA_RESIDUE_GUARD` ; après a1 commit 1, `liste` = 826 / 99 (exact) ; après a2, `liste` ≈ 600 (`src/` ≈ 376, `tests/` ≈ 225), rapporté dans la note | « État de départ », « Effet sur `make data-residue` », critère de départ | `make -s data-residue` ; `sh tools/data_residue.sh --detail \| grep ^liste` ; note « Découvert à D.12a » |
| H-D12a-2 | `pico8_ptrs_t` (`src/pico8_ptrs.c/.h`) : `{void **items; int count; int capacity}`, `pico8_ptrs_init`/`_free`/`_add(ptrs, void *e)`/`_del(ptrs, const void *e)` (premier identique, rend 1 si retiré)/`_all`/`_iter_next(iter, void **out)`/`_foreach(ptrs, void (*)(void *e, void *ctx), ctx)` ; NULL stockable ; pas d'étiquette de type ; dans `ENGINE_SRCS` et `DATA_ENGINE_SRCS` ; `test_pico8_ptrs` existe | toutes les décisions de forme ; « `pico8_ptrs_del` » dans `ei`, `jc:kj`, `jo:cn` ; piège du hachage (pas d'étiquette) | lecture de `pico8_ptrs.h` ; `grep -n pico8_ptrs Makefile` |
| H-D12a-3 | `pico8_collect.c/.h` gagnent `pico8_jp_ptrs` (+ `pico8_jp_ptrs_predicate_fn` = `int (*)(void *e, void *ctx)`, `qk` NULL toléré), `pico8_la_ptrs` (NULL asserté), `pico8_gu_ptrs(const pico8_ptrs_t *, const void *)` (NULL toléré, identité), `pico8_nv_ptrs` (+ `pico8_nv_ptrs_fn` = `int (*)(void *e, void *ctx, void **out)`, NULL asserté) ; les aides à valeur gardent **exactement** ces appelants : `pico8_jp` : `planet_display.c:195` (`jy`), `research_display.c:361` (`bj`), `tech_gate.c:126` (`cb`) ; `pico8_la` : `planet_display.c:202`, `:223`, `:226`, `planet_trade.c:130`, `trade.c:138`, `:140` ; `pico8_gu` : `planet_family_trade.c:141` (`t_rl`) ; `pico8_nv` : `layout.c:170`, `planet.c:335` (`hp.i`) ; `pico8_gu_content` : `tech_gate.c:39` (`invented`) ; `pico8_la_append` interne à `pico8_la` | « Aides de `collect` » (lesquelles meurent ici), greps de sortie 2 et 3 | `grep -nE '\bpico8_(jp|la|gu|gu_content|nv)\(' src/*.c` ; lecture de `pico8_collect.h` |
| H-D12a-4 | Registres, calques de `dc`, `ui.ic`, `ui.le`, `mz.ox` sur `pico8_ptrs_t` ; `pico8_hv()`/`pico8_cv()`/`pico8_cg()` rendent `const pico8_ptrs_t *` ; `pico8_ij` converti ; D.12b n'y touche pas | les lectures de `cg("jo")` dans les parcours de D.12b (`bd`, `bb`, `dh` via `for_each_cg`) sont déjà converties | lecture de `pico8_entity.h`, `pico8_wormhole.h` |
| H-D12a-5 | `g_ga_by_shared` inchangé : `static pico8_list_t g_ga_by_shared;` (`planet_family.c:25`), `self->jo.by = &g_ga_by_shared;` (`:618`), `pico8_list_init(&g_ga_by_shared);` (`:661`) — le grep 5 de la Sortie de D.12a le garde | décision « `g_ga_by_shared` avec `jo.by` » (b2) | `grep -n g_ga_by_shared src/pico8_planet_family.c` |
| H-D12a-6 | D.12a modifie, dans `src/` : `entity.c/.h`, `bootstrap.c`, `bg.c`, `economy.c`, `economy_formulas.c`, `economy_score.c`, `economy_tiers.c`, `probe.c`, `score_panel.c`, `slipway.c`, `wormhole.c/.h`, `ui.c/.h`, `msgbox.c`, `collect.c/.h`, `entity_geom.c/.h`, `ptrs.c/.h` (nouveaux), `Makefile` ; dans `tests/` : `test_data_residue`, `test_pico8_ptrs` (nouveau) et les 34 TU lecteurs de registres (Annexe D de son PRD), dont 22 que D.12b touche aussi (`fr`, `fv`, `g_`, `ga`, `ga_export_panel_by`, `hj`, `hp`, `hy`, `ie`, `jc`, `jo`, `js`, `lq`, `mx`, `nl`, `nr`, `os`, `pj`, `pm`, `qh`, `trade`, `ui`) | marques de l'Inventaire, des mutations et des Annexes C et D | `git diff --stat f2d5dad <HEAD gradé>` |
| H-D12a-7 | (Constaté au vérificateur de D.12a1 : `test_pico8_js` porte une aide transitoire `rank_in_le`, que D.12a2 doit fondre dans `rank_in` converti ; si elle survit, D.12b ne la touche pas.) D.12a scinde, dans les TU, les aides locales partagées entre L1 et L2 : la partie L2 garde la forme valeur sous un nom choisi par l'implémenteur (`test_pico8_mx.c` `listed` sur `jo.oe` ; `test_pico8_jc.c` `list_count` sur `jc->ts` et `jo.oe` ; `test_pico8_fv.c` et `test_pico8_g_.c` `size_of` sur `emitter->qy`), listée à son message ; les aides `list_size` des TU de l'arbre `jo` (L2 seulement) et `list_of`/`list_is`/`fold_ranks` de `test_pico8_trade` ne sont pas touchées | « Les TU suivent », aides de TU (Annexe D) | message des commits de D.12a2 ; `grep -n 'pico8_list_t' tests/test_pico8_{mx,jc,fv,g_}.c` |
| H-D12a-8 | Les en-têtes qui ne nomment plus la liste gardent `#include "pico8_list.h"` quand d'autres fichiers en dépendent par transitivité (`entity.h`, `ui.h`, `wormhole.h`, `entity_geom.h`) ; aucun include direct ajouté hors empreinte | règle des includes de D.12b (même règle) | message de D.12a ; `grep -n 'pico8_list.h' src/*.h` |
| H-D12a-9 | Toute ligne de lien explicite du `Makefile` qui nomme `collect.c` ou `entity.c` nomme aussi `pico8_ptrs.c` (grep 7 de D.12a) ; les 7 TU liés sur `DATA_ENGINE_SRCS` seule se lient | « `Makefile` et liens » : **aucune** modification du `Makefile` attendue en D.12b | grep 7 de D.12a sur le HEAD gradé |
| H-D12a-10 | `make e2e` : 26 conformes ; `tools/class_coverage_net_expected.txt` inchangé ; seul TU ajouté : `test_pico8_ptrs` | filets | note de D.12a |
| H-D12a-11 | D.12a ne convertit aucune liste L2 autre que `mz.ox`, aucune liste L3/L4, ne touche ni `pico8_list.c/.h`, ni `particle`, `gen`, `galaxy`, `trade`, `planet*`, `research*`, `menu`, `tech_gate`, `slipway_display` (hors `probe.c`, `slipway.c` pour `jc:ce`) | tout l'inventaire de D.12b non marqué | `git diff --stat f2d5dad <HEAD gradé> -- <ces chemins>` vide |
| H-D12a-12 | La ligne D.12b du plan n'est pas modifiée par la note de D.12a (qui dit ce que D.12b trouve) ; le repli a2p/a2s de D.12a n'a pas été pris, ou, s'il l'a été, ne change rien aux aides à valeur ni aux TU scindés | « Lecture de ces textes » | lecture du plan gradé |

**Verdicts à `6cf4df4`** (`git archive`, recette `liste`, greps, notes « Découvert à D.12a1/a2 » recopiées en Annexe A) :

| Hypothèse | Verdict |
|---|---|
| H-D12a-1 | **tenue** (mesurée) : après a1 commit 1, `liste` 826 / 99 / 870 occ., exact ; après a2, **581 lignes / 86 fichiers / 625 occ.** (`src/` 371 / 49 / 411, `tests/` 210 / 37 / 214) au lieu de l'estimation ≈ 600 ; `valeur` 1245 / 115, `type-tag` 271 / 62, `ref-audit` 42 / 14, résidu D 2005 / 121 ; `DATA_RESIDUE_GUARD` inchangé |
| H-D12a-2 | **tenue** : `pico8_ptrs.h` exactement comme supposé (`del(const void *)`, `iter_next(void **)`, `foreach(void (*)(void *, void *))`, sans tag, NULL stockable) ; `pico8_ptrs.c` dans `ENGINE_SRCS` et `DATA_ENGINE_SRCS` ; `test_pico8_ptrs` existe |
| H-D12a-3 | **tenue** : `pico8_jp_ptrs`, `pico8_la_ptrs`, `pico8_gu_ptrs`, `pico8_nv_ptrs` aux signatures prévues (`collect.h:34-48`) ; appelants des aides à valeur exactement ceux prévus (`nv` : `layout.c:170`, `planet.c:335` ; `la` : `planet_display.c:202`, `:223`, `:226`, `planet_trade.c:130`, `trade.c:138`, `:140` ; `jp` : `planet_display.c:195`, `research_display.c:361`, `tech_gate.c:126` ; `gu` : `planet_family_trade.c:141` ; `gu_content` : `tech_gate.c:39`). `pico8_la_ptrs` a sa propre aide `pico8_la_ptrs_append` (`collect.c:117`) ; `pico8_la_append` à valeur reste interne à `pico8_la`. Le vérificateur de a1 note que `pico8_gu` à valeur n'est plus exécuté que par les e2e, et qu'une mutation de `pico8_gu` ne fait rougir aucun e2e |
| H-D12a-4 | **tenue** : registres, calques, `ui.ic`, `ui.le`, `mz.ox`, `pico8_ij` sur `pico8_ptrs_t` |
| H-D12a-5 | **tenue** : `static pico8_list_t g_ga_by_shared;` (`planet_family.c:25`), `:618`, `:661` inchangés |
| H-D12a-6 | **tenue en substance** : les fichiers de `src/` touchés sont conformes, `bg.c` compris (1 ligne). Touchés en plus : `Makefile` (a1 : listes, cible `test_pico8_ptrs`, deux lignes de lien), `tools/float_guard.txt` (réécrit par `make test`), `docs/glossaire-noms.md` (une ligne `_ptrs`). Numéros de ligne remesurés : **inchangés** dans tous les fichiers de `src/` de l'étape, **sauf `collect.c/.h`** (aides `_ptrs` insérées : `pico8_la_append` 56 → 104, `pico8_la` 60 → 108, `pico8_gu_content` 70 → 131 ; `.h` : `gu` 12 → 13, `la` 28-29 → 29-30, `gu_content` 34 → 53). Dans les TU, inchangés sauf `test_pico8_mx.c` (+11 : `list_size` 154, `fold_value(h, ib)` 479-481, `value_listed` 556, `jy` 565-571) et `test_pico8_ga_export_panel_by.c` (−2 : `by` 95, assertion d'étiquette 140, ajouts 143-144) |
| H-D12a-7 | **tenue**, noms constatés : `list_size` des dix TU de l'arbre `jo` inchangée (à valeur) ; `size_of` de `fv:208`, `g_:227` (à valeur, `qy`) ; `list_count` de `jc:248` (à valeur, `ts` et `oe` seulement, la part registre étant passée aux aides `_ptrs`) ; **`value_listed`** de `mx:89` (scindée de `listed` pour `jo.oe`, un appel en `:556`) ; `rank_in_le` de `test_pico8_js` fondue en a2 |
| H-D12a-8 | **fausse, dans le bon sens** : `entity.h`, `entity_geom.h`, `ui.h`, `wormhole.h` ont **perdu** `#include "pico8_list.h"` (aucun fichier ne s'en servait par eux). La règle des includes de D.12b reste la même ; elle vise les fichiers de l'étape qui l'incluent encore : `collect.h`, `economy_formulas.h`, `gen.h`, `particle.h`, `planet_trade.h`, `research.h`, `research_display.h`, `slipway.h`, `trade.h`, `tech_gate.h`, ainsi que `planet_family_trade.c`, `trade.c` et `menu.c` côté `.c` |
| H-D12a-9 | **tenue** : a2 ne touche pas le `Makefile` ; les lignes de lien qui nomment `collect.c`/`entity.c` nomment aussi `pico8_ptrs.c` |
| H-D12a-10 | **tenue** (notes et vérificateurs) : aucune constante changée, `class-coverage-net` identique ; aucun TU ajouté hors `test_pico8_ptrs` ; cas ajoutés : deux dans `test_pico8_ui` (a1), `test_ce_crosses_a_slipway` dans `test_pico8_jc` (a2) |
| H-D12a-11 | **tenue** : `particle`, `gen`, `galaxy`, `trade`, `planet*`, `research*`, `menu`, `tech_gate`, `slipway_display`, `pico8_list.*`, `test_pico8_map_tables`, `test_pico8_probe_mm_rnd_order`, `test_pico8_jo_spec_defaults` sont absents de `git diff --stat f2d5dad 6cf4df4` |
| H-D12a-12 | **tenue** : seule la ligne D.12a du plan change (sa note) ; la ligne D.12b est identique ; le repli a2p/a2s n'a pas servi |

Conséquences : aucune décision ni aucun découpage ne change. Départ réel : **`liste` 581 / 86 / 625 occ.** ; les 183 lignes classées b1/b2 (41 + 142) et les 10 `inc` sont toutes retrouvées par contenu, sans aucune ligne `liste` nouvelle dans les fichiers de l'étape. Apport du vérificateur de a1 (l'exclusion `t_rl` de `nr:cn` n'est gardée par rien) : un cas unitaire est exigé en b2 (voir « Cas nouveau exigé »).

### Sections revérifiées sur `6cf4df4`

Refaites : description et en-tête, verdicts, « État de départ », « Effet sur `make data-residue` », numéros de l'Inventaire, des mutations et des Annexes C et D (régénérées), Annexe A (ligne D.12a et ses notes, recopiée de `6cf4df4`). Inchangées et vérifiées : décisions, découpage, empreinte, critères. Si le HEAD de départ de D.12b n'est pas `6cf4df4` (ou un descendant qui ne touche que `docs/`), l'implémenteur remesure et explique l'écart.

(Liste d'origine, gardée pour trace :)

- **Toujours** : « État de départ » (comptes réels, H-D12a-1) ; critères `FAILED:` de départ (HEAD de départ, `liste` de départ) ; Annexe A (ligne D.12a et sa note « Découvert à D.12a », à recopier ; ligne D.12b si la note l'a modifiée, H-D12a-12) ; les numéros de l'Inventaire, des Mutations et des Annexes C et D (remesure **par contenu**, jamais par recopie).
- **Si H-D12a-2 tombe** (forme du conteneur, nom, présence d'une étiquette) : « Forme par liste », « Piège des hachages de TU », mutations M1–M3, M11, M13.
- **Si H-D12a-3 tombe** (noms, signatures ou appelants des aides `_ptrs` et à valeur) : « Aides de `collect` », greps 2 et 3 de la Sortie, « Effet sur `make data-residue` ».
- **Si H-D12a-5 tombe** (`g_ga_by_shared` converti ou déplacé par D.12a) : décision `jo.by`, grep 6, M22.
- **Si H-D12a-6 ou -11 tombe** (fichiers touchés en plus ou en moins) : Annexe C (reclasser), « Empreinte et découpage ».
- **Si H-D12a-7 tombe** (noms ou périmètre des aides scindées) : « Les TU suivent », Annexe D.
- **Si H-D12a-8 ou -9 tombe** : règle des includes ; « `Makefile` et liens ».
- **Si H-D12a-10 tombe** : filets, liste des TU.

### Lecture de ces textes pour D.12b

- **« listes membres L2 (`jo.oe`, `jo.cc`/`by`, `trade.lp`/`rl`, `mz.ox`, proposition `ib`, `qy`, `jc.ts`, `hy.invented`, `jo.jy`/`hp.i` ; `gen.oc` en tableau fixe) »**, ajustée par les déplacements que le PRD de D.12a déclare : **`mz.ox` sort** (pris par D.12a2 avec `hv`, `cg` et `pico8_ij`) ; **`g_ga_by_shared` entre** (défaut partagé de `jo.by` des `ga`, reporté par D.12a). `qy` est `emitter->qy` depuis D.11c (composant `pico8_emitter_t` porté par `g_`, `hj`, `ga` et leurs descendants). La « proposition `ib` » est le membre `ib` de `pico8_jo_trade_proposal_t` (`planet_trade.h:28`), pointeur vers la liste que rend `pico8_jo_ib` (c'est `self->by` quand il existe).
- **« `gen.oc` en tableau fixe »** : le cart n'appelle `gn` qu'une fois, `gn(df,2,…)` (`pico8/lua.lua:751`) ; la borne du pool est donc dans le cart : Σ flr(`r`·2) sur `df` = **66** (`test_pico8_map_tables.c:142`). Voir « `gen.oc` ».
- **« aides L4 de `collect` (`jp`, `nv`, `gu`/`gu_content`, `la_append` ; prédicat `int (*)(void *, void *ctx)`) »** : D.12a crée les versions `_ptrs` de `jp`/`la`/`gu`/`nv` à ce prédicat (H-D12a-3) ; D.12b y fait passer tous les appelants **L2** des aides à valeur, crée la seule aide qui manque (`pico8_gu_content_ptrs`, pour `invented`) et **retire les aides à valeur devenues sans appelant** : `pico8_gu`, `pico8_gu_content`, `pico8_la`, `pico8_la_append`. **Survivent jusqu'à D.12c** : `pico8_jp` (appelants `cb` de `tech_gate` et `bj` de `research_display`, listes locales L4 que la ligne D.12c nomme) et `pico8_nv` (appelant `layout`, largeurs fixes, L3). Voir « Aides de `collect` ».
- **« Empreinte ≈ 25 (25, 0) »** : estimation de l'étude (`2dec71f`). Remesurée à `f2d5dad`, confirmée à `6cf4df4` : **30 fichiers de `src/`** et **25 TU** (plus `docs/plan-3.md`, `docs/glossaire-noms.md`, `docs/trous-de-portage.md`). Le « 0 » de TU tombe pour la même raison qu'à D.12a : 25 TU lisent ces listes (Annexe D), et un pont serait une cale.
- **« Compte : `liste` rapporté »** : départ = `liste` après D.12a, **581** (`6cf4df4`) ; D.12b retire ≈ 170 lignes `liste` nettes de `src/` (183 classées b1/b2, Annexe C, dont une dizaine réécrites mais gardées et ≈ 4 ajoutées dans `jo:gh`) et ≈ 55 lignes de TU. Estimé ≈ 355 après D.12b, rapporté (« Effet sur `make data-residue` »).
- **« §6 : aucun »** : faux par lecture, déclaré. `test_pico8_ga_export_panel_by` (§6) lit `jo.by` (L2) : `pico8_list_t *by` (`:95`), `pico8_type_tag_of(by) == PICO8_TYPE_TAG_LIST` (`:140`), deux `pico8_list_add` (`:143-144`), et son commentaire de tête nomme « `pico8_list_t*` » (`:1`). Il suit en b2 **dans sa façon de lire l'état** ; comme il est réécrit, la preuve rouge du §6 est refaite (M22). `test_pico8_planet_gh_row_lq` (§6) ne lit aucune liste L2 (grep) ; il exerce `jo:gh`, dont D.12b réécrit les concaténations de `jy` : il doit rester vert, texte inchangé. `layout_nil_item_draws_nothing` : non touché.
- **« `class-coverage-net` : seulement si le diff touche un constructeur »** : **oui aux deux sous-étapes** : b1 touche `pico8_hy_struct_new` (`invented`) et `pico8_jc_pq` (appelé par le constructeur de `jc`) ; b2 touche `pico8_jo_pq` (`oe`), `pico8_hp_struct_new` (`hp.i`), `pico8_trade_struct_new` (`lp`/`rl`) et l'installation de `jo.by` dans le constructeur de `ga`.
- **« `sanitize-net` : oui »** : à chaque sous-étape (tas de `oe`, `jy`, `cc`, `by`, `lp`, `rl`, `ts`, `qy`, `invented` ; libération de `cc` par `bd` ; `gen.oc` quitte le tas).
- **Règle des ponts de TU (l. 267)** : tranchée comme à D.12a, **premier terme** : les TU suivent dans le commit qui change la liste ; aucun pont (un double tenu en parallèle ou un instantané seraient une cale, interdite).
- **Déplacements depuis D.12c, déclarés** : la ligne D.12c nomme `trade:kg` et `galaxy` parmi les listes locales ; or `{np}`/`{jc}` de `trade:kg` (`trade.c:108-143`) n'existent que pour être concaténées à `lp`/`rl`, et `hr_list` de `galaxy.c:46-108` n'existe que pour être passée à `gn` : elles changent avec `trade.lp`/`rl` et `gen.oc`. De même, les trois concaténations de `jo:gh` qui prennent `jy` (`planet_display.c:198-228`) sont réécrites ici (leurs listes locales `prefix`/`last_only` disparaissent) ; les autres listes d'affichage de `planet_display` (`ji`, `merged`, `lb`, `bj_list`, `PICO8_REF_AS_LIST`) restent à D.12c.
- **Remarque de clôture de D.11e** (`pico8_lq_bg`, `pico8_pj_ed` rendent un `pico8_value_t` pour les seuls TU) : aucune des deux ne nomme une liste (`layout.c:315`, `score_panel.c:296`) : **hors D.12b**, Q12. `hp_pose` : idem.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json`, rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes, alias, registre (tableau, `switch` ou boucle qui passe d'un nom à un membre), renommage pour échapper aux greps, cast non vérifié, cale permanente ; mutations restaurées par `cp` simple ; binaires lancés exactement comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan) ; sous-étapes d'environ 50 min ; jamais l'appli raylib.

### État de départ

**Mesuré à `f2d5dad`** (D.12a non commencée), puis **à `6cf4df4`** (D.12a gradée), sans build, recette du PRD de D.12a :

| Mesure | Valeur à `f2d5dad` | Départ réel de D.12b (`6cf4df4`) |
|---|---|---|
| `liste` (motif de Q10) | **826 lignes / 99 fichiers / 870 occ.** (`src/` 477 / 60, `tests/` 349 / 39) | **581 / 86 / 625 occ.** (`src/` 371 / 49 / 411, `tests/` 210 / 37 / 214), mesuré à `6cf4df4` (note de D.12a2) |
| lignes `liste` des fichiers de l'étape classées b1 / b2 (Annexe C) | 41 / 142 (plus 10 `inc`) | **mêmes 41 / 142 / 10**, retrouvées par contenu à `6cf4df4` |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom` | inchangé |
| `valeur`, `type-tag`, `ref-audit` | 1321 / 118, 272 / 62, 42 / 14 | **1245 / 115, 271 / 62, 42 / 14** ; résidu D **2005 / 121** (`6cf4df4`) |

Recette (réutilisée à chaque mesure ; `sh tools/data_residue.sh --detail | grep -E '^liste'` donne les lignes, la recette reste pour les occurrences) :

```sh
P='\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'liste\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire (remesuré par contenu à `6cf4df4`, D.12a gradée ; mesure d'origine à `f2d5dad`)

#### Listes membres et leur stockage

| Liste | Déclaration (fichier:ligne) | Éléments | Écrite par | Lue par (`src/`) | Sous-étape |
|---|---|---|---|---|---|
| `emitter->qy` | `particle.h:44` (`pico8_emitter_t`) ; mis à NULL `planet_family.c:331`, `:617`, `probe.c:470` | `pico8_qy_t *` (rendus par `cls->mm`) | `pico8_f_` (`particle.c:39-54`, tas, TROU-7), `pico8_ez` (`:56-75`, ajout) | `pico8_ei` (`particle.c:77-150`, boucle **à index** avec `del`, TROU-14), `pico8_ga_kj` (`planet_family.c:514-560`, index 1..3) | b1 |
| `jc.ts` | `slipway.h:45` ; NULL `slipway.c:450` | `pico8_ts_t *` | `pico8_jc_pq` (`slipway.c:119-133`, tas), `pico8_jc_ch` (`slipway_display.c:44`, ajout) | `pico8_jc_kj` (`slipway_display.c:134-165`, boucle **à index** avec `del`, TROU-14) | b1 |
| `hy.invented` | `economy.h:22` ; `economy.c:25`, `:33`, `:50-52` (tas, TROU-7) | chaînes `px` (`const char *`) | `pico8_hy_kt` (`economy.c:277-300`) | `pico8_ky` (`economy_score.c:199-200`, compte), `pico8_cr` (`menu.c:173-174`, compte ; `:210` `pico8_menu_invented_lines`, `menu.c:139-154`), `pico8_oj` (`tech_gate.c:28-40`, `pico8_gu_content`, TROU-3) | b1 |
| `gen.oc` | `gen.h:32` (`pico8_gn_state_t`) | entrées de `df` (`const pico8_df_entry_t *`), un ticket par copie | `pico8_gn` (`gen.c:47-75`) depuis `hr` (liste locale de `galaxy.c:46-108`) | `pico8_gn_next` (`gen.c:80-103`, tirage, `del`, fenêtre `nm[3]`) ; `pico8_gn_free` (`:105-111`) ; consommé par `pico8_jo_hh` (`planet.c:478-480`) | b1 |
| `jo.oe` | `planet.h:44` ; NULL `planet.c:170` | voies (`jc`) | `pico8_jo_pq` (`planet.c:285-300`, tas), `pico8_jo_kf` (`planet_trade.c:54-57`, ajout) | `pico8_jo_ib` (`planet_trade.c:206-249`, `all()`), `pico8_nr_gv`/`_hb` (`planet_family_trade.c:40`, `:95`, compte : texte inchangé), `pico8_nr_cn` (`:122-160`, `all()`), `pico8_jc_ce` (`slipway.c:291-313`, `all()`, rend l'élément) | b2 |
| `jo.jy` / `hp.i` | `planet.h:47` ; `research.h:43` | `pico8_jy_t *` | `hp.i` : `pico8_hp_struct_new` (`research.c:155-245`, tas) ; `jy` : `pico8_jo_lz` (`planet.c:321-341`, `nv` de `hp.i` avec copie `mw`), `pico8_jo_cn` (`planet_trade.c:103-137`, `del` puis `la({fresh}, jy)`), `bb_visit` (`economy_tiers.c:176-181`, ajout) | `pico8_jo_cn` (`all()` ×2), `pico8_mx_kf` (`planet_family_trade.c:296-340`, `all()`), `pico8_dh` (`economy_tiers.c:220-300`, index), `bx_visit`/`ck_visit` (`economy_formulas.c:74-125`), `pico8_jo_hb`/`pico8_jo_gh` (`planet_display.c:56-77`, `:137-232`), `pico8_hp_jb_join`/`pico8_jo_ex`/`pico8_hp_lq` (`research_display.c:31-60`, `:142-200`, `:268-290`) | b2 |
| `jo.cc` | `planet.h:59` ; NULL `planet.c:185` | routes (`pico8_trade_t *`) | `bd_reset_visit` (`economy_tiers.c:49-66`, libère et réalloue), `bd_route_visit` (`:138-150`, ajout) | `pico8_dh` (`:236-262`, index ; `:311`, compte) | b2 |
| `jo.by`, `g_ga_by_shared` | `planet.h:60` ; NULL `planet.c:186` ; `planet_family.c:25`, `:618`, `:661` | routes | `bd_route_visit` (`jo->by = pico8_jo_ib(jo)`, `economy_tiers.c:80-81`), `pico8_jo_ib` (rend `self->by` ou un tas neuf), fermeture `kf` de la proposition (ajout) | `bd_route_visit` (`all()`), `pico8_db` (`economy_score.c:87-140`, index), `pico8_dh` (`:309-311`, compte), `pico8_jo_hb` (`planet_display.c:72-75`, compte), `pico8_jo_gh` (`:260-261`, compte) | b2 |
| `trade.lp` / `trade.rl` | `trade.h:13-14`, clés `:38-39` | planètes / voies | `pico8_trade_struct_new` (`trade.c:23-48`, emprunt des clés), `pico8_jo_ib` (`planet_trade.c:221-229`, tas `{self}`/`{w}`), `pico8_trade_kg` (`trade.c:102-148`, `la(lp,{np})`, `la(rl,{jc})`) | `pico8_trade_to` (`trade.c:78-91`, dernier de `lp`), `pico8_trade_eq` (`:150-178`), `bd_route_visit` (`economy_tiers.c:101-133`, index), `pico8_nr_cn` (`planet_family_trade.c:138-143`, `gu(t_rl, w)`) | b2 |
| proposition `ib` | `planet_trade.h:28` | routes | `pico8_jo_ib_kf_closure` (`planet_trade.c:36-44`, ajout si absent par `gu_trade`) | `gu_trade` (`:19-34`), `lw.mu` (`:233`, compte) | b2 |

#### API dont la valeur ne portait qu'un élément de ces listes (type changé en D.12b, classe (b))

| API | Aujourd'hui | Après D.12b | Appelants (`src/`) |
|---|---|---|---|
| `pico8_gn` | `(const pico8_list_t *hr, og, r, r_ctx)` | `(const pico8_df_entry_t *hr, int hr_count, og, r, r_ctx)` | `galaxy.c:107` (b1) |
| `pico8_gn_next` | rend `pico8_value_t` | rend `const pico8_df_entry_t *` | `planet.c:478` (b1) |
| `pico8_gn_state_t` | `pico8_list_t oc` ; `pico8_value_t nm[3]` | tableau fixe `oc[…]` + compte ; `const pico8_df_entry_t *nm[3]` | — |
| `pico8_menu_invented_lines` (statique) | `const pico8_list_t *` | `const pico8_ptrs_t *` | `menu.c:210` (b1) |
| `pico8_jo_ib` | rend `pico8_list_t *` | rend `pico8_ptrs_t *` | `economy_tiers.c:80` (b2) |
| `pico8_jo_trade_proposal_kf_fn` | `(proposal, pico8_value_t a)` | `(proposal, pico8_trade_t *a)` | `planet_trade.c:172` (b2) |
| `pico8_trade_keys_t.lp`/`.rl`, `pico8_trade_t.lp`/`.rl` | `pico8_list_t *` | `pico8_ptrs_t *` | `planet_trade.c:228-229` (b2) |
| `pico8_trade_kg` | `(self, pico8_value_t np, pico8_value_t jc)` | `(self, pico8_struct_header_t *np, pico8_struct_header_t *jc)`, NULL = nil | `planet_trade.c:170`, `planet_family_trade.c:147` (b2) |
| `pico8_trade_to` | `(self, pico8_value_t *out)` | `(self, void **out)`, même contrat (1 et `*out` écrit ; 0 et `*out` intact) | `economy_score.c:110`, `economy_tiers.c:138`, `trade.c:174` (b2) |
| `pico8_jo_ex` | `(const pico8_list_t *is, …)` | `(const pico8_ptrs_t *is, …)` | `planet_display.c:74`, `research_display.c:285` (b2) |

#### Aides de `collect` (après D.12a, H-D12a-3)

| Aide à valeur | Appelants restants après D.12a | En D.12b | Sort |
|---|---|---|---|
| `pico8_gu_content` (`collect.c:131-148`) | `tech_gate.c:39` (`invented`) | → `pico8_gu_content_ptrs` (nouvelle, b1) | **retirée en b1** |
| `pico8_gu` (`collect.c:8-21`) | `planet_family_trade.c:141` (`t_rl`) | → `pico8_gu_ptrs` (b2) | **retirée en b2** |
| `pico8_la` (`:108-116`), `pico8_la_append` (`:104-106`) | `planet_trade.c:130` (`jy`), `trade.c:138`, `:140` (`lp`, `rl`), `planet_display.c:202`, `:223`, `:226` (en-têtes d'affichage + `jy`) | `planet_trade`, `trade` → `pico8_la_ptrs` ; `planet_display` : concaténation écrite au site (« Frontière L3 ») | **retirées en b2** |
| `pico8_jp` (`:23-38`) | `planet_display.c:195` (`jy`), `research_display.c:361` (`bj`), `tech_gate.c:126` (`cb`) | `planet_display` → `pico8_jp_ptrs` (b2) | **survit** (2 appelants L4, D.12c) |
| `pico8_nv` (`:40-54`) | `planet.c:335` (`hp.i`), `layout.c:170` (L3) | `planet.c` → `pico8_nv_ptrs` (b2) | **survit** (1 appelant L3, D.12c) |

#### Parcours et sites de `src/` par sous-étape (détail ligne à ligne : Annexe C)

**b1** — `particle.c` (`f_`, `ez`, `ei`), `particle.h` (`qy`), `planet_family.c` (`ga_kj` seul), `slipway.c` (`jc_pq` seul), `slipway.h` (`ts`), `slipway_display.c` (`jc_ch`, `jc_kj`), `economy.c` (`hy_struct_new`, `hy_kt`), `economy.h`, `economy_score.c` (`ky` seul), `menu.c` (`invented_lines` et le compte de `cr` ; `cb_list` reste à D.12c), `tech_gate.c` (`oj` seul ; `cb` reste à D.12c), `collect.c/.h` (`pico8_gu_content` → `pico8_gu_content_ptrs`), `gen.c/.h`, `galaxy.c` (`hr_list`), `planet.c` (`jo_hh`, `:478-480`, lecture typée de `gn_next` : ligne `valeur` retirée, aucune ligne `liste`).

**b2** — `planet.c` (`jo_pq`, `jo_lz` et son mappeur `pico8_jo_lz_mw_step`), `planet.h` (`oe`, `jy`, `cc`, `by`), `planet_trade.c/.h` (`gu_trade`, fermeture `kf`, `jo_kf`, `jo_cn`, `jo_ib`), `planet_family.c` (`g_ga_by_shared`), `planet_family_trade.c` (`nr_cn`, `mx_kf`, include), `trade.c/.h` (tout ce qui nomme `lp`/`rl`, `value_is_nil` retirée), `economy_tiers.c` (`bd_reset_visit`, `bd_route_visit`, `bb_visit`, `dh`), `economy_score.c` (`db`), `economy_formulas.c` (`bx_visit`, `ck_visit`), `planet_display.c` (`jo_hb`, `jo_gh_jp_step`, `jo_gh` : les sites de `jy` et `by` seulement), `research.c/.h` (`hp.i`), `research_display.c/.h` (`hp_jb_join`, `jo_ex` et son paramètre `is`, `hp_lq` ; la liste `l` et `bj` restent à D.12c), `slipway.c` (`jc_ce`, boucle sur `qu.oe`), `collect.c/.h` (`pico8_gu`, `pico8_la`, `pico8_la_append` retirées).

Hors de l'étape, vérifiés : `probe.c:470` (`qy = NULL`, texte inchangé) ; `planet_family_trade.c:40`, `:95` (`->oe->count` : texte inchangé, type seul) ; `pi.c` (`bl` parcourt `mh.items`, L3), `ui_button.c:232-235` (L3), `help.c` (L4) : D.12c.

#### TU (détail : Annexe D)

25 fichiers. Lignes à réécrire (Annexe D, colonne `L` : ligne `liste`) : ≈ 46 lignes `liste` propres aux listes L2 (b1 ≈ 16, b2 ≈ 30) plus les définitions d'aides locales et les lectures `->items[i].as.ref` / `.kind` des mêmes listes ; les lignes `(jo->oe != NULL)`, `pico8_value_ref(pico8_jo_ib(e))`, `->count` gardent leur texte (type seul).

| Groupe | TU | Sous-étape |
|---|---|---|
| `qy` | `fv`, `g_`, `ga`, `hj`, `mx`, `pm`, `probe_mm_rnd_order` | b1 |
| `ts` | `jc` | b1 |
| `invented` | `hy`, `ie`, `js`, `pj` | b1 |
| `gen.oc` | `map_tables` | b1 |
| `oe`, `jy`/`hp.i`, `cc`/`by`, `lp`/`rl`, `ib`, API de `trade` | `fr`, `ga`, `ga_export_panel_by` (§6), `hj`, `hp`, `hy`, `jc`, `jo`, `lq`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `trade`, `ui` (et `jo_spec_defaults`, texte inchangé attendu : `:220` ne compare que des pointeurs) | b2 |

**Aides locales de TU** (à `6cf4df4`, toutes encore à valeur, H-D12a-7) : `list_size` (`fr:142`, `ga:134`, `hj:154`, `jo:146`, `mx:154`, `nl:138`, `nr:139`, `os:152`, `pm:139`, `qh:184` ; sert `oe`, `jy`, `by`, `cc` **et** `qy` dans `ga`, `hj`, `mx`, `pm`) ; `list_of` (`fr:410`, `nr:418`, `oe` ; `trade:208`, `lp`/`rl`) ; `list_is`, `fold_ranks`, `route`, `count_kf` (`trade`) ; `size_of` (`fv:208`, `g_:227`, `qy`) ; `list_count` (`jc:248`, `ts` et `oe`) ; `value_listed` (`mx:89`, `jo.oe`, un appel `:556`) ; `invented_of`, `fold_list_member` (`hy:82`, `:161`).

**Piège des hachages (vérifié à `f2d5dad`, toujours vrai à `6cf4df4`)** : plusieurs constantes de TU plient **l'étiquette de type** de la liste, `PICO8_TYPE_TAG_LIST` (`0x9E5C1157u`), lue par `pico8_type_tag_of` : `fold_value(h, pico8_value_ref(pico8_jo_ib(e)))` passe par `fold_ref`, qui plie `pico8_type_tag_of(ref)` (`fr:400-402`, `hj:478-480`, `mx:479-481`, `nl:416-418`, `nr:408-410`, `os:439-441`, `pm:483-485`) ; `fold_list_member(h, hy->invented)` plie `pico8_type_tag_of(l)` puis chaque élément comme valeur chaîne (`hy:161-172`, `:197`). Un `pico8_ptrs_t` n'a pas d'étiquette (H-D12a-2) : `pico8_type_tag_of` y lirait les premiers octets de `items` et rendrait `UNKNOWN`, **la constante rougirait**. Voir « Les TU suivent ».

#### `Makefile` et liens

- Aucun fichier nouveau ; `pico8_ptrs.c` est déjà dans `ENGINE_SRCS` et `DATA_ENGINE_SRCS` (H-D12a-2, -9). **Aucune ligne du `Makefile` ne change en D.12b** (attendu ; toute modification est un écart à justifier).
- Règle de lien : aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste. `planet_family_trade.c`, `galaxy.c`, `menu.c` sont **hors** liste ; D.12b n'ajoute aucun appel d'un fichier de la liste vers eux (les appels ajoutés vont vers `pico8_ptrs.c`, `pico8_collect.c`, `pico8_trade.c`, `pico8_gen.c`, tous dans la liste). `test_pico8_ga_export_panel_by` se lie sur `DATA_ENGINE_SRCS` + `planet_family.c` (`Makefile:561`) : inchangé.

## Solution

**Deux sous-étapes** (D.12b1, D.12b2), même PRD, chacune verte seule, vérifiée et poussée (modèle de D.11d, D.11e, D.12a) :

1. **D.12b1 — listes hors du réseau des routes** (≈ 33 chemins, ≈ 45 min), un commit : `emitter->qy`, `jc.ts`, `hy.invented` (et `pico8_gu_content` → `pico8_gu_content_ptrs`), `gen.oc` en tableau fixe (avec `hr` de `galaxy`, `pico8_gn`/`_next` typés) ; les 13 TU qui les lisent ; `docs/trous-de-portage.md` (TROU-3 nomme l'aide), `docs/glossaire-noms.md`.
2. **D.12b2 — réseau des routes** (≈ 41 chemins, ≈ 55 min), un commit : `jo.oe`, `jo.jy`/`hp.i`, `jo.cc`, `jo.by` et `g_ga_by_shared`, `trade.lp`/`rl`, proposition `ib` ; API de `trade` et `kf` typées ; `pico8_gu`, `pico8_la`, `pico8_la_append` retirées ; les 18 TU qui les lisent (dont le §6 `ga_export_panel_by`) ; `docs/glossaire-noms.md` ; note « Découvert à D.12b ». Repli écrit si le plafond est atteint : b2o (`oe`, `jy`/`hp.i`) puis b2t (routes).

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (dont `make data-guard`, inchangé) ; `make e2e` : **26 conformes** ; `make sanitize-net` propre ; balayage ASan+UBSan des TU propre ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (b1 et b2) ;
- `make data-residue` : `liste` ≈ départ − 230, rapporté ; `valeur`, `type-tag`, `ref-audit` rapportés ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/`, rien dans `tools/`, rien dans le `Makefile` ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  # 1. plus aucune liste membre L2 en pico8_list_t dans src/ : vide
  grep -nE 'pico8_list_t \*(oe|jy|cc|by|lp|rl|ib|qy|ts|invented|i)\b|pico8_list_t (oc|g_ga_by_shared)\b' src/*.[ch]
  # 2. les aides à valeur retirées n'existent plus nulle part : vide
  grep -nE '\bpico8_(gu|gu_content|la|la_append)\b' src/*.[ch] tests/*.[ch]
  # 3. les aides à valeur qui survivent gardent exactement leurs appelants L3/L4 (D.12c)
  grep -nE '\bpico8_jp\(' src/*.c        # collect.c (définition), research_display.c (bj), tech_gate.c (cb) : 3 lignes
  grep -nE '\bpico8_nv\(' src/*.c        # collect.c (définition), layout.c : 2 lignes
  # 4. plus aucun déballage d'un élément de ces listes : vide
  grep -nE '\b(oe|jy|cc|by|lp|rl|ib|qy|ts|invented|i_list|is|t_rl|oc)(->|\.)items\[[^]]*\]\.(as|kind)' src/*.c
  # 5. gen et galaxy sans liste (l'include de gen.h relève de la règle des includes) : vide
  grep -n 'pico8_list' src/pico8_gen.c src/pico8_galaxy.c
  # 6. le défaut partagé de by suit jo.by : une ligne
  grep -c 'static pico8_ptrs_t g_ga_by_shared;' src/pico8_planet_family.c   # 1
  # 7. règle de lien : les TU liés sur la seule liste DATA_ENGINE_SRCS se lient
  make tmp/test_pico8_bg_kj tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_planet_gh_row_lq tmp/test_pico8_ga_export_panel_by
  # 8. aucune constante de TU changée : vide
  git diff <HEAD de départ> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 9. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^liste'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que les listes membres des entités (`oe`, `jy`, `hp.i`, `cc`, `by`, `lp`, `rl`, `ib`, `qy`, `ts`, `invented`) soient des `pico8_ptrs_t`, afin qu'aucune lecture ne teste plus un genre qui ne peut valoir que REF avant de déballer `.as.ref`.
2. En tant que mainteneur du port, je veux que chaque élément lu soit converti vers sa struct précise sous le même test de classe ou de famille qu'aujourd'hui, afin qu'un `void *` ne soit jamais lu à travers la mauvaise mise en page.
3. En tant que mainteneur du port, je veux `gen.oc` en tableau fixe typé borné par le seul appel du cart (`gn(df,2)`, 66 tickets), afin que Q9 (« tableaux fixes là où la borne est dans le cart ») s'applique là où la ligne le demande.
4. En tant que mainteneur du port, je veux que `pico8_gn_next` rende l'entrée de `df` tirée et que `nm[3]` tienne des pointeurs d'entrée, afin que `jo:hh` ne déballe plus une valeur.
5. En tant que mainteneur du port, je veux que les API de `trade` (`kg`, `to`) et le `kf` de la proposition prennent et rendent des pointeurs, afin qu'aucune enveloppe valeur ↔ pointeur ne subsiste autour d'une liste de pointeurs.
6. En tant que mainteneur du port, je veux que `hy.invented` tienne des chaînes et que `oj` la cherche par contenu (TROU-3), afin que la comparaison de chaînes reste explicite.
7. En tant que mainteneur du port, je veux que `g_ga_by_shared` change avec `jo.by`, afin que le défaut partagé des `ga` garde le type du membre qu'il remplit.
8. En tant que mainteneur du port, je veux que les boucles à index qui suppriment en cours de route (`ei` sur `qy`, `jc:kj` sur `ts`) restent à index sans compensation, afin que TROU-14 reste identique.
9. En tant que mainteneur du port, je veux que chaque `all()` reste un itérateur à tolérance et chaque boucle à index une boucle à index, afin que l'ordre de visite et les suppressions restent ceux d'aujourd'hui.
10. En tant que mainteneur du port, je veux que les aides à valeur de `collect` sans appelant après l'étape (`gu`, `gu_content`, `la`, `la_append`) soient retirées, et que celles qui survivent gardent exactement leurs appelants L3/L4, afin que D.12c trouve un état exact.
11. En tant que mainteneur du port, je veux qu'une liste d'affichage hétérogène (L3) qui reçoit des besoins de `jy` les reçoive enveloppés au site, sans aide de conversion, afin que D.12c n'ait qu'une forme à remplacer par son union.
12. En tant que gardien des tests, je veux que les 25 TU qui lisent ces listes suivent dans le commit qui change la liste, constantes inchangées, afin qu'ils testent la vraie liste.
13. En tant que gardien des tests, je veux que les constantes qui plient l'étiquette de type d'une liste la reçoivent d'une constante figée du TU, afin qu'un conteneur sans étiquette ne les fasse pas rougir à tort.
14. En tant que gardien des tests, je veux que le §6 `ga_export_panel_by` ne change que sa façon de lire `by`, avec sa preuve rouge refaite, afin que sa garde reste la même.
15. En tant que gardien des filets, je veux `make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net` et le balayage ASan+UBSan verts à chaque sous-étape, afin de prouver qu'aucune durée de vie, aucun ordre et aucune instanciation ne change.
16. En tant que gardien des tests, je veux `gcov` avant/après et des mutations ciblées restaurées par `cp`, afin que chaque ligne ajoutée soit exécutée et qu'aucune ligne survivante ne cesse de l'être.
17. En tant que gardien du compteur, je veux au message de chaque commit les comptes par catégorie et par fichier en lignes et en occurrences, la classification, les lignes gagnées (« aucune » compris) et chaque Δlignes > Δocc justifié, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
18. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, qu'aucun alias, registre, renommage pour les greps, cast non vérifié ni cale permanente ne soit ajouté, afin que la baisse veuille dire « retiré ».
19. En tant que `plan-pilot`, je veux deux sous-étapes d'environ 45 et 55 minutes, un repli écrit pour la seconde, et une note « Découvert à D.12b » qui dise les comptes réels, les écarts à la ligne et ce que D.12c/D.12d trouvent, afin que la suite parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q9 et Q10 sont des décisions humaines que ce PRD applique ; Q12 reste ouverte et n'est pas tranchée ici.

### Modules

- **Particules** : `pico8_particle.c/.h`, `pico8_planet_family.c` (`ga:kj`), `pico8_slipway.c/.h`, `pico8_slipway_display.c`.
- **Économie** : `pico8_economy.c/.h`, `pico8_economy_score.c`, `pico8_economy_tiers.c`, `pico8_economy_formulas.c`, `pico8_menu.c`, `pico8_tech_gate.c`.
- **Générateur** : `pico8_gen.c/.h`, `pico8_galaxy.c`, `pico8_planet.c` (`jo:hh`).
- **Réseau des routes** : `pico8_planet.c/.h`, `pico8_planet_trade.c/.h`, `pico8_planet_family.c`, `pico8_planet_family_trade.c`, `pico8_trade.c/.h`, `pico8_slipway.c` (`jc:ce`).
- **Besoins et projets** : `pico8_research.c/.h`, `pico8_research_display.c/.h`, `pico8_planet_display.c`.
- **Aides** : `pico8_collect.c/.h`.
- **TU** : 25 (Annexe D). **Docs** : `plan-3.md` (note), `glossaire-noms.md`, `trous-de-portage.md` (TROU-3).
- Inchangés : `Makefile`, `tools/`, `pico8_list.c/.h`, `pico8_ptrs.c/.h` (API de D.12a, H-D12a-2), `test_pico8_list`, `test_pico8_ptrs`, les registres.

### Forme par liste (G)

| Liste | Forme | Élément à la lecture | Test qui précède toute conversion vers une struct précise |
|---|---|---|---|
| `emitter->qy` | `pico8_ptrs_t *` (tas, TROU-7) | `pico8_qy_t *` (ce que `mm` rend) | aucun nécessaire : `mm` a pour type `struct pico8_qy *(*)(void *)` (`struct.h:91`), seul écrivain ; affectation implicite depuis `void *` |
| `jc.ts` | `pico8_ptrs_t *` | `pico8_ts_t *` | idem : seul écrivain `jc:ch`, qui y range un `pico8_ts_t *` |
| `hy.invented` | `pico8_ptrs_t *` | `const char *` | — (chaînes) |
| `gen.oc` | tableau fixe `const pico8_df_entry_t *oc[CAP]` + `int oc_count` | `const pico8_df_entry_t *` | — |
| `jo.oe` | `pico8_ptrs_t *` | voie | `assert(w->cls == pico8_jc_struct_class() …)` gardés mot pour mot (`planet_trade.c:244`, `planet_family_trade.c:151`, `slipway.c:306-308`) |
| `jo.jy`, `hp.i` | `pico8_ptrs_t *` | `pico8_jy_t *` | seuls écrivains : `pico8_jy_new` (`research.c`, `economy_tiers.c:179`, `planet_trade.c:123`) et la copie `mw` ; affectation implicite, pas de cast |
| `jo.cc`, `jo.by`, `ib`, `g_ga_by_shared` | `pico8_ptrs_t *` / `pico8_ptrs_t` | `pico8_trade_t *` | `assert(… == pico8_trade_struct_class() && "jo.by holds trades only")` et « jo.cc holds trades only » gardés ; `db` passe par `pico8_trade_to`, qui asserte (`assert_trade`) |
| `trade.lp`, `trade.rl` | `pico8_ptrs_t *` | planète / voie | `pico8_jo_is_family` et « trade.rl holds slipways only » gardés (`economy_tiers.c:121`, `:126-127`) |

- **Tests de genre** : `e.kind != PICO8_VALUE_REF || e.as.ref == NULL` devient `e == NULL` ; les `assert` de classe et de famille restent mot pour mot. Rejeté : retirer le test `NULL` (le conteneur accepte NULL ; le comportement sur un élément NULL reste « sauté »).
- **Forme des parcours gardée** : `all()` → `pico8_ptrs_all`/`_iter_next` ; boucle à index → boucle à index ; `del` → `pico8_ptrs_del` (premier identique : même résultat que `pico8_list_del` pour des REF, `pico8_value_equal` comparant les pointeurs, `value.c:46-47`). **TROU-14** : `ei` et `jc:kj` gardent leur indice non compensé. Rejeté : « simplifier » une boucle.
- **Propriété** : inchangée. `bd_reset_visit` libère `jo->cc` par `pico8_ptrs_free` + `free` comme aujourd'hui ; les tas jamais libérés restent jamais libérés (TROU-7) ; `g_ga_by_shared` est réinitialisé par `pico8_planet_family_init` comme aujourd'hui.
- **Includes** (règle de D.12a, H-D12a-8) : un en-tête qui nomme `pico8_ptrs_t` inclut `pico8_ptrs.h` ; un `.c` dont plus rien ne nomme `pico8_list_*` perd `#include "pico8_list.h"` ; un en-tête qui ne nomme plus la liste (`particle.h:7`, `slipway.h:7`, `gen.h:6`, `planet_trade.h:6`, `research.h:6`, `trade.h:5`) **garde** l'include si l'enlever casse un fichier qui s'en sert par transitivité, sinon l'enlève. Chaque include gardé ou retiré est listé au message (lignes `inc` de l'Annexe C).

### `gen.oc` (G)

- `pico8_gn_state_t` : `const pico8_df_entry_t *oc[PICO8_GN_OC_CAP]; int oc_count; const pico8_df_entry_t *nm[3]; int nm_set[3];` (le reste inchangé). La capacité est une constante nommée de `gen.h` (nom hors de tout motif du compteur ; `PICO8_GN_OC_CAP` proposé), valeur **66**, avec un `assert` porteur de message dans `pico8_gn` avant chaque ajout (« le pool est borné par l'unique `gn(df,2)` du cart : un `df` ou un `og` plus grands demandent un tableau plus grand »). Rejetés : `pico8_ptrs_t` (la ligne dit « tableau fixe ») ; une capacité calculée à l'exécution (ce serait un tas, pas un tableau fixe).
- `pico8_gn(const pico8_df_entry_t *hr, int hr_count, pico8_fixed_t og, …)` : `galaxy.c` passe `pico8_df_table, PICO8_DF_COUNT` et perd `hr_list` (4 lignes `liste`, déplacées de D.12c, déclaré) ; `test_pico8_map_tables` de même.
- `pico8_gn_next` rend `const pico8_df_entry_t *` ; le `del(oc,e)` du cart est transcrit sur le tableau : retrait du **premier** élément identique à `e`, décalage des suivants. Équivalence déclarée : les copies d'une entrée sont contiguës et identiques, donc retirer l'indice tiré donnerait le même tableau ; on garde la transcription littérale. `pico8_gn_nm_contains` compare des pointeurs (ce que faisait `pico8_value_equal` sur deux REF). `jo:hh` (`planet.c:478-480`) lit l'entrée directement.
- **Q12** : `gen.nm[3]` et `pico8_gn_next` quittent l'inventaire des valeurs à genres mêlés de l'étude (§4, point 2 : « `gen.nm[3]` ») : leur seul écrivain y range un élément de `oc`, qui ne tient que des entrées de `df`. Déclaré à la note, sur le modèle de D.10b/D.11a (« quittent la liste »).

### `hy.invented` (G)

- `pico8_ptrs_t *invented` (tas, TROU-7) ; `hy:kt` ajoute `t->px`. Le conteneur prend `void *` : l'ajout d'une chaîne constante porte un `(void *)` de **qualificatif seul** sur la chaîne, aucun lecteur n'écrit à travers (précédent : `pico8_value_ref((void *)&pico8_df_table[i])`, `galaxy.c:104`, `tech_gate.c:123`). C'est la seule forme de cast admise par ce PRD hors des conversions vérifiées ; elle est listée au message avec sa ligne. Rejetés : un tableau fixe `[PICO8_NB_COUNT]` (le cart ne borne pas `kt` : `hy:kt` ajoute sans tester, la borne ne tient que par le filtre de `cb`, et des TU appellent `kt` hors de ce filtre) ; ranger l'entrée `nb` au lieu de sa chaîne (les TU rangent des chaînes libres).
- `oj` : `pico8_gu_content` → **`pico8_gu_content_ptrs(const pico8_ptrs_t *qk, const char *kb)`** (nouvelle, `collect`, TROU-3 : égalité par contenu ; `qk` NULL → 0 ; contrat : chaque élément est une chaîne), appelée par `oj` seulement si `px` est une chaîne. Équivalence : l'ancienne comparait un élément chaîne à un `px` non chaîne par `pico8_value_equal`, toujours faux. `pico8_oj` garde sa signature (`pico8_value_t px`, nil/faux possibles : Q12).
- `menu.c` : `pico8_menu_invented_lines(const pico8_ptrs_t *)` lit des chaînes (la branche « toute autre valeur : ligne vide » devient « élément NULL : ligne vide ») ; le compte de `cr` et de `ky` : texte `->count` inchangé.

### Réseau des routes (G)

- **`pico8_trade_kg(self, pico8_struct_header_t *np, pico8_struct_header_t *jc)`**, NULL = nil ; `{np}`/`{jc}` deviennent des `pico8_ptrs_t` locaux (ou rien quand NULL), concaténés par `pico8_la_ptrs` ; `value_is_nil` retirée. Appelants : `&self->ke.header` (`jo:cn`), `&self->qh.jo.ke.header` et `w` (`nr:cn`), sans cast.
- **`pico8_trade_to(self, void **out)`** : même contrat (1 et `*out` = dernier de `lp` ; 0 et `*out` intact). Appelants : `economy_score.c:110`, `economy_tiers.c:138` gardent leur `assert(pico8_jo_is_family(…))` avant la conversion ; `pico8_trade_eq` compare des pointeurs.
- **`kf` de la proposition** : `(pico8_jo_trade_proposal_t *, pico8_trade_t *a)` ; `gu_trade(const pico8_ptrs_t *, const pico8_trade_t *)` (égalité de routes, pas d'identité : inchangé).
- **Pourquoi typer ces API** (G) : leur `pico8_value_t` ne portait qu'un élément d'une liste désormais de pointeurs (appelants : toujours `pico8_value_ref(ptr)` ou nil) ; les garder à valeur obligerait à déballer au bord de la liste, cale valeur ↔ conteneur interdite. Aucune n'est dans l'inventaire Q12 de l'étude sauf `gen.nm[3]` (ci-dessus) ; `pico8_trade_qe` (Q12) n'est pas touchée. Rejeté : garder les valeurs et asserter le genre au bord.
- **`g_ga_by_shared`** : `static pico8_ptrs_t g_ga_by_shared;`, `pico8_ptrs_init` dans `pico8_planet_family_init` ; `self->jo.by = &g_ga_by_shared;` texte inchangé.
- **`jo:cn`** : `pico8_list_del(jy, e)` → `pico8_ptrs_del` ; `la({fresh}, jy)` → `pico8_la_ptrs` avec un `{fresh}` local `pico8_ptrs_t`.
- **`jo:lz`** : `pico8_nv_ptrs(hp_instance->i, pico8_jo_lz_mw_step, NULL)`, mappeur `int (void *e, void *ctx, void **out)`, copie `mw` inchangée.
- **`jc:ce`** : la boucle sur `qu.oe` rend `pico8_value_ref(w)` (retour `pico8_value_t` de `pico8_jc_ce` gardé, Q12 ; ligne `valeur` gagnée ou modifiée, déclarée, comme `result` à D.12a).
- **`nr:cn`** : `pico8_gu_ptrs(t_rl, w)`.

### Frontière L3 dans `jo:gh` (G)

`jo:gh` construit des listes d'affichage **hétérogènes** (chaînes d'en-tête, besoins, tuples), `pico8_list_t` de valeurs jusqu'à l'union de D.12c. Quand `jy` devient un `pico8_ptrs_t` :

- le prédicat de `jp` (`pico8_jo_gh_jp_step`) prend `void *e` et ajoute `pico8_value_ref(e)` à `ctx->ji` (ligne `liste` modifiée, gagne `valeur`) ; `jx = pico8_jp_ptrs(jy, …)`, libéré par `pico8_ptrs_free` ;
- les trois concaténations `la({"needs:"}, jx)`, `la({"wants more:"}, {jy[#jy]})`, `la({"wants more:"}, jy)` (`planet_display.c:198-228`) sont **écrites au site** : `merged` reçoit l'en-tête puis chaque élément enveloppé `pico8_value_ref(e)`, dans l'ordre de `la` ; `prefix` et `last_only` disparaissent. Pas de fonction de conversion (ce serait une cale) ; D.12c remplacera ces ajouts par des items de son union.
- Rejetés : garder `pico8_la` à valeur pour ces sites (il faudrait une copie à valeur de `jx`/`jy`, un instantané) ; tirer toutes les listes d'affichage de `planet_display` dans D.12b (elles attendent l'union de D.12c).

### Aides de `collect` (G)

- Retirées : `pico8_gu_content` (b1), `pico8_gu`, `pico8_la`, `pico8_la_append` (b2), avec leurs déclarations et commentaires de contrat. Nouvelle : `pico8_gu_content_ptrs` (b1), marquée `/* TROU-3 */`.
- Gardées jusqu'à D.12c : `pico8_jp` (+ `pico8_jp_predicate_fn`) et `pico8_nv` (+ `pico8_nv_fn`), `pico8_list.h` inclus par `collect.h` pour elles.
- **Noms `_ptrs`** (laissés par D.12a à D.12b) : **gardés** tant qu'une aide à valeur du même nom court existe (`jp`, `nv` jusqu'à D.12c) ; D.12c ou D.12d tranche. Renommer `pico8_gu_ptrs` en `pico8_gu` ici n'apporte rien et ferait lire `gu` à valeur dans un diff ; rejeté.

### Les TU suivent (G, règle des ponts, l. 267)

- **Tranché** : chaque TU qui lit une liste L2 suit dans le commit qui la change ; aucun pont.
- **Aides locales** : une aide à paramètre `const pico8_list_t *` dont **tous** les appels portent sur des listes converties dans la sous-étape est retypée en place (`const pico8_ptrs_t *`). Une aide qui sert aussi une liste encore à valeur (`list_size` dans `ga`, `hj`, `mx`, `pm` en b1 : `qy` converti, `oe`/`jy`/`by` pas encore) : les appels convertis passent à une aide sur `pico8_ptrs_t` (existante depuis D.12a, ou créée, nommée par l'implémenteur, listée) ; l'aide à valeur est **retirée dans la sous-étape où part son dernier appel**, jamais renommée ni rendue à nouveau. Chaque ligne d'appel est donc éditée une fois. Les aides scindées par D.12a pour `oe`, `ts`, `qy` (H-D12a-7) sont retirées quand leurs appels passent à l'aide sur `pico8_ptrs_t`.
- **Lectures** : `->items[i].as.ref` → `->items[i]` ; tests de genre `items[i].kind == PICO8_VALUE_REF` retirés (le test `!= NULL` reste s'il existe) ; `pico8_list_add(l, pico8_value_ref(x))` → `pico8_ptrs_add(l, x)`.
- **Piège des hachages** (G) : là où une constante plie l'étiquette `PICO8_TYPE_TAG_LIST` d'une liste L2 (sept `fold_value(h, ib)`, `fold_list_member(invented)`), le TU plie une constante figée **`FROZEN_LIST_TAG 0x9E5C1157u`** (précédent exact : `FROZEN_INSTANCE_TAG` de `test_pico8_g_.c:50`, `hj:47`, `js:58`), dans la branche qui reconnaît la liste (par pointeur connu : `ib` est le retour de `pico8_jo_ib`, `invented` le membre), puis ses éléments comme aujourd'hui (les chaînes d'`invented` pliées comme une valeur chaîne : même mot de genre, mêmes octets). Constantes **inchangées**. Rejetés : réécrire la constante (interdit) ; nommer `PICO8_TYPE_TAG_LIST` dans le TU (ligne `liste` ajoutée pour être retirée à D.12d, et symbole que D.12d retire).
- **Assertions d'étiquette** : `ga:384`, `ga_export_panel_by:140`, `hy:84`, `hp:448` perdent leur moitié `PICO8_TYPE_TAG_LIST` (le type est vérifié à la compilation) et gardent le reste ; (a2) au message.
- **Constantes** : aucune `*_HASH`/`identity[]` ne change (mêmes éléments, même ordre, mêmes comptes). Une constante qui rougit est un bug de conversion (ou le piège ci-dessus oublié).
- **TU de `trade`** : `list_of`, `list_is`, `route`, `fold_ranks` passent à `pico8_ptrs_t` ; `ref()`/`nil()` des appels de `kg` deviennent le pointeur ou NULL ; les cas `same_value(out, sentinel)` de `trade_to` deviennent un pointeur sentinelle intact ; `count_kf` prend `pico8_trade_t *`.

### §6 : `test_pico8_ga_export_panel_by` (G)

Suit en b2, **façon de lire seulement** : `pico8_ptrs_t *by`, deux `pico8_ptrs_add(by, route_x)`, l'`assert` d'étiquette réduit à `by != NULL`, le commentaire de tête réécrit pour rester vrai (le contrat « `by` des `ga` est une vraie liste » est désormais tenu par le type du membre ; ce que le test garde : la ligne « exports: » compte les routes ajoutées dans le `by` que le `ga` a reçu). Intention, cas et chaînes attendues (« 0/2 », « 2/2 ») inchangés. **Preuve rouge refaite** (le §6 l'exige pour un test réécrit) : M22.

### Règle de lien (liant)

Aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste. Symboles nouveaux ou retypés : `pico8_gu_content_ptrs` (`collect.c`), `pico8_gn`/`_next`, `pico8_trade_kg`/`_to`, `pico8_jo_ib`, `pico8_jo_ex` (tous dans la liste). Preuve : grep 7 de la Sortie.

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la liste à valeur est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours pour échapper au motif ; alias (macro, `typedef` de `pico8_list_t`, enveloppe qui renomme une aide) ; registre (tableau, `switch` ou boucle qui passe d'un nom à un membre) ; cast d'un élément vers une struct précise sans son test ; cast de pointeur de fonction ; cast autre que le `(void *)` de qualificatif d'`invented` ; cale valeur ↔ conteneur (fonction de conversion, double tenu en parallèle, instantané) ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; tout changement d'une constante de hachage ; toute modification de `tests/e2e/`, d'un `*.sha256`, de `tools/`, du `Makefile`.
- **Classification exigée**, au message de chaque commit, pour chaque ligne `liste` retirée ou modifiée (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) :
  - **(a1) conteneur converti** : la même opération (déclaration, init, libération, ajout, retrait, parcours, type de membre) sur `pico8_ptrs_t` ou sur le tableau fixe de `gen` ;
  - **(a2) supprimée sans remplaçant** : include devenu inutile, assertion d'étiquette sur un conteneur typé, liste locale devenue inutile (`hr_list`, `prefix`, `last_only`, `{np}`/`{jc}` quand nil), aide à valeur retirée ;
  - **(b) signature dont le type de liste change** : attendues et listées : les API de la table « API … (classe (b)) », `pico8_menu_invented_lines`, `gu_trade`, les prédicats et mappeurs (`pico8_jo_gh_jp_step`, `pico8_jo_lz_mw_step`), les aides locales de TU ; toute autre est une erreur ;
  - **(c) même donnée, autre nom** : **attendu 0**. `pico8_gu_content_ptrs` est un type nouveau (élément `const char *`), pas (c).
- **Mesure exigée**, avant et après chaque commit, par catégorie (`valeur`, `type-tag`, `ref-audit`, `liste` ; `par-nom` = 0 constaté) et **par fichier** du diff : lignes (`--detail`) **et** occurrences (recette de « État de départ »).
- **Lignes qui gagnent une catégorie**, par fichier et par catégorie, « aucune » compris. Attendues : `valeur` : `pico8_value_ref(e)` posé sur un élément là où la suite lit une valeur (`jc:ce`, `jo:gh` : `ji` et les concaténations écrites au site) ; `liste` : les ajouts écrits au site dans `jo:gh` (≈ 3 à 5 lignes) ; `type-tag` : aucune. Toute autre est à justifier.
- **Lignes comptées modifiées, non retirées** (fichier:ligne) : listées (ex. : `planet_display.c:99`, `:200-201`, `:217-218`).
- **Δlignes > Δoccurrences** : chaque couple justifié (attendu : Δocc ≥ Δlignes).
- **Numéros de ligne** : ceux du **parent du commit**, jamais ceux de ce PRD sans remesure (remesurés à `6cf4df4`).

### Empreinte et découpage (G)

| Sous-étape | `src/` | `tests/` | autres | Chemins | Coût estimé |
|---|---|---|---|---|---|
| **b1** | `particle.c`, `particle.h`, `planet_family.c`, `slipway.c`, `slipway.h`, `slipway_display.c`, `economy.c`, `economy.h`, `economy_score.c`, `menu.c`, `tech_gate.c`, `collect.c`, `collect.h`, `gen.c`, `gen.h`, `galaxy.c`, `planet.c` (17) | `fv`, `g_`, `ga`, `hj`, `mx`, `pm`, `probe_mm_rnd_order`, `jc`, `hy`, `ie`, `js`, `pj`, `map_tables` (13) | `trous-de-portage.md`, `glossaire-noms.md` | 32 | ≈ 45 min |
| **b2** | `planet.c`, `planet.h`, `planet_trade.c`, `planet_trade.h`, `planet_family.c`, `planet_family_trade.c`, `trade.c`, `trade.h`, `economy_tiers.c`, `economy_score.c`, `economy_formulas.c`, `planet_display.c`, `research.c`, `research.h`, `research_display.c`, `research_display.h`, `slipway.c`, `collect.c`, `collect.h` (19) | `fr`, `ga`, `ga_export_panel_by`, `hj`, `hp`, `hy`, `jc`, `jo`, `lq`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `trade`, `ui` (17 ; `jo_spec_defaults` seulement si son texte doit changer) | `glossaire-noms.md`, `plan-3.md` | 38–39 | ≈ 55 min |

- **Pourquoi ce découpage** : l'étape entière fait ≈ 70 chemins (> ~40) : deux sous-étapes d'environ 50 min. La coupure suit le graphe des dépendances de `src/` : `qy`, `ts`, `invented`, `gen.oc` n'ont aucun lecteur commun avec le réseau des routes ; `oe`, `jy`, `cc`, `by`, `lp`, `rl`, `ib` sont liés par `jo:ib` (qui parcourt `oe` et fabrique des routes), `bd`/`dh` (qui lisent `by`, `cc`, `jy`, `lp`, `rl` ensemble) et les aides `list_size` de dix TU (qui plient `oe` et `jy` côte à côte). Coût du recouvrement : `ga`, `hj`, `mx`, `pm`, `jc`, `hy` sont édités dans les deux sous-étapes, sur des lignes disjointes (règle des aides : chaque appel une fois).
- Rythme mesuré : D.11c ≈ 23 min pour 37 chemins ; D.11d1..d4 ≈ 21 à 27 min par sous-étape ; D.12a (a1 et a2 vérifiées sans repli). Les éditions de TU sont mécaniques (aide, lecture `->items[i]`), sauf `test_pico8_trade` (API typées, ≈ 40 lignes) et le piège des hachages (8 sites).
- **Repli écrit** (G) : si b2 atteint son plafond (85 min) sans être finie, rien n'est commis ; `plan-pilot` relance b2 en deux : **b2o** — `jo.oe`, `jo.jy`/`hp.i` (`planet.c/.h`, `planet_trade.c` pour `kf`/`cn`/la boucle de `jo_ib`, `planet_family_trade.c` pour `nr:cn`/`mx:kf`, `slipway.c`, `economy_tiers.c` pour `bb`/`dh`, `economy_formulas.c`, `planet_display.c`, `research*`, et les TU `fr`, `hj`, `hp`, `hy`, `jc`, `jo`, `lq`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `trade` (`:507-520`), `ui` ; `pico8_la` garde alors ses appelants de `trade.c`) — puis **b2t** — routes (`cc`, `by`, `g_ga_by_shared`, `lp`/`rl`, `ib`, API de `trade`, `pico8_gu`/`pico8_la` retirées ; TU `trade`, `os`, `ga`, `ga_export_panel_by`, `jo`, et les sept `fold_value(h, ib)`). Rejeté comme plan principal : trois passes sur `jo`, `os`, `trade`.
- Rejetés : une seule sous-étape (≈ 1 h 40) ; une sous-étape par liste (≈ 9, dix TU édités cinq fois).

### Effet sur `make data-residue`

| Catégorie | Départ (après D.12a, H-D12a-1) | Après b1 | Après b2 | Pourquoi |
|---|---|---|---|---|
| `liste` total | **581** (`6cf4df4`) | ≈ 520 | **≈ 355**, rapporté | Annexe C : b1 41 lignes, b2 142 (dont ≈ 10 modifiées gardées, ≈ 4 ajoutées dans `jo:gh`) ; TU ≈ 16 (b1) et ≈ 30 à 35 (b2) |
| `liste` `src/` | **371** | ≈ 332 | ≈ 200 | reste : `pico8_list.c/.h`, `ref_audit.h`, `type_tag.h`, `value.h`, L3/L4 de D.12c, `jp`/`nv` à valeur |
| `liste` `tests/` | **210** | ≈ 190 | ≈ 155 | reste : D.12d |
| `valeur` | **1245** | baisse (`gn_next`, `nm`, tests de genre, `.as.ref`), rapportée | baisse, rapportée | quelques lignes gagnées déclarées (`jc:ce`, `jo:gh`) |
| `type-tag` | **271** | baisse (assertions d'étiquette de TU) | baisse | — |
| `DATA_RESIDUE_GUARD` | `…,dispatch,par-nom` | inchangé | inchangé | `liste` ≠ 0 |

« ≈ » est une estimation de ce PRD, jamais un critère ; « rapporté » est mesuré au commit, écart expliqué.

### Documentation (G)

- `docs/plan-3.md` (b2) : note « Découvert à D.12b » en fin de ligne D.12b, rien d'autre. Elle dit : les deux sous-étapes, leurs chemins et commits ; les comptes réels ; les formes (conteneur, tableau fixe de `gen`) ; `mz.ox` sorti (D.12a) et `g_ga_by_shared` entré ; les API typées et ce qui quitte l'inventaire Q12 (`gen.nm[3]`, `pico8_gn_next`, `trade_kg`/`_to`, `kf`) ; les aides à valeur retirées et celles qui restent avec leurs appelants ; les déplacements depuis D.12c (`trade:kg`, `galaxy`, trois concaténations de `jo:gh`) ; le piège des étiquettes pliées et les `FROZEN_LIST_TAG` ; les TU suiveurs (§6 compris) ; pour D.12c : `ji`/`merged` de `jo:gh` reçoivent des `pico8_value_ref(e)` à remplacer par l'union, `pico8_jp`/`pico8_nv` à valeur, le nom des aides `_ptrs` ; pour D.12d : les includes gardés.
- `docs/trous-de-portage.md` (b1) : TROU-3 nomme `pico8_gu_content_ptrs` au lieu de `pico8_gu_content` (le trou est inchangé, son contournement change de nom) ; TROU-7, TROU-14 inchangés et vrais.
- `docs/glossaire-noms.md` : lignes `gu`, `gu_content` (b1, b2), `la` (b2) : symbole C mis à jour (`pico8_gu_ptrs`, `pico8_gu_content_ptrs`, `pico8_la_ptrs`) ; la ligne « `pico8_gu_ptrs`, `pico8_nv_ptrs`, `pico8_jp_ptrs`, `pico8_la_ptrs` » ajoutée par D.12a (« les versions à valeur gardant leurs autres appelants ») est réécrite pour rester vraie (seules `jp` et `nv` gardent une version à valeur) ; `jp`, `nv` : les deux symboles tant que les deux existent.
- `docs/audit-ref-cast-reverse.md` : document d'audit historique, non modifié (il cite `pico8_la_append`).
- Commentaires : quatre sortes de `CLAUDE.md` ; les contrats devenus faux sont réécrits (`trade.h:9-10` « the REFs the keys held », `trade.h:52-53`, `planet_trade.h:37-38`, `gen.h:26`, `:39-44`, `collect.h`, commentaires de `pico8_emitter_t`) ; aucun renvoi à D.12b ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile` ; aucun e2e ni TU ajouté ou supprimé.
- Toute constante `*_HASH`/`identity[]` de TU.
- Le comportement : mêmes éléments, même ordre, mêmes suppressions, mêmes sauts (TROU-5, TROU-14), mêmes fuites (TROU-7), même comparaison par contenu (TROU-3) ; `make e2e` conforme au bit près.
- `pico8_list.c/.h`, `pico8_ptrs.c/.h`, `test_pico8_list`, `test_pico8_ptrs` ; les registres ; `pico8_jp`/`pico8_nv` à valeur et leurs appelants L3/L4 ; les listes L3 (`lq.items`, `mh.items`, `jb.ls`, `lq.fs`) et les listes locales d'affichage hors celles déplacées ici.
- Les API à valeur de Q12 : `pico8_jc_ce`, `pico8_trade_qe`, `pico8_oj`, `pico8_jy_qe`, `pico8_jc_hg_add`/`_has`, `pico8_lq_bg`, `pico8_pj_ed`.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `-Wall -Wextra -Werror` rend fatale toute fonction `static` inutilisée : une aide locale de TU ou un prédicat perd son dernier appel dans la sous-étape qui la retire ; `value_is_nil` de `trade.c` est retirée avec son dernier appel.
- **H2** : aucune constante de TU ne change, piège des étiquettes traité ; une constante qui rougit est un bug de conversion.
- **H3** : aucune liste de l'étape ne tient un élément d'un autre genre que celui de la table « Forme par liste » : les branches « pas un REF » sont mortes aujourd'hui (le `gcov` de départ le montre) ; exception connue : les TU `pj` rangent des **fixes** dans `invented` (`test_pico8_pj.c:705`, `:739`) pour n'en compter que le nombre : ils rangeront des chaînes (constante inchangée : seul le compte est plié) ; `menu_invented_lines` n'est pas atteint par ces cas (à constater).
- **H4** : 66 est le compte du pool à `og = 2` (`test_pico8_map_tables.c:142`) ; aucun TU n'appelle `pico8_gn` avec un autre `og`.
- **H5** : les 25 TU de l'Annexe D sont tous ceux qui lisent ces listes ou ces API après D.12a (H-D12a-6, -7) ; un TU oublié se révèle à la compilation, il suit (listé au message).

## Implementation Chunks

Deux sous-étapes, dans cet ordre, chacune verte seule (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`, balayage ASan+UBSan des TU), vérifiée et poussée avant la suivante, avec ses comptes, sa classification, ses lignes gagnées, son `gcov`, ses mutations et son tableau « Runs » :

1. **D.12b1 — `qy`, `ts`, `invented`, `gen.oc`** (32 chemins). Ordre :
   1. mesure de départ (H-D12a-1..-11 constatées, sinon arrêt et reprise du PRD) ; `gcov` de départ ;
   2. `particle.c/.h`, `planet_family.c` (`ga:kj`), TU `fv`, `g_`, `ga`, `hj`, `mx`, `pm`, `probe_mm_rnd_order` ;
   3. `slipway.c/.h` (`jc:pq`), `slipway_display.c`, TU `jc` ;
   4. `pico8_gu_content_ptrs` ; `economy.c/.h`, `economy_score.c`, `menu.c`, `tech_gate.c` ; `pico8_gu_content` retirée ; TU `hy`, `ie`, `js`, `pj` (piège : `fold_list_member`) ;
   5. `gen.c/.h`, `galaxy.c`, `planet.c` (`jo:hh`), TU `map_tables` ;
   6. docs (TROU-3, glossaire) ; filets ; `gcov` d'arrivée ; mutations M1–M10 ; mesure, classification, message ; **commit**.
2. **D.12b2 — réseau des routes** (≈ 39 chemins). Ordre :
   1. mesure de départ ; `gcov` de départ ;
   2. `trade.c/.h` (membres, clés, `kg`, `to`, `eq`) ; `planet_trade.c/.h` ; `planet.c/.h` ; `planet_family.c` (`g_ga_by_shared`) ; `planet_family_trade.c` ; `economy_tiers.c`, `economy_score.c`, `economy_formulas.c` ; `slipway.c` (`jc:ce`) ;
   3. `research.c/.h`, `research_display.c/.h`, `planet_display.c` (frontière L3) ;
   4. `collect.c/.h` : `pico8_gu`, `pico8_la`, `pico8_la_append` retirées ;
   5. les 17 TU (piège : sept `fold_value(h, ib)`) ; cas unitaire `pico8_nr_cn` / `t_rl` dans `test_pico8_nr` ; §6 `ga_export_panel_by` et sa preuve rouge ;
   6. filets ; `gcov` ; mutations M11–M25 ; mesure, classification ; glossaire ; note « Découvert à D.12b » ; **commit**.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (éléments, ordre, présence, effets d'un appel), pas la présence d'une ligne. D.12b change la représentation de onze listes et le type de cinq API sans changer un élément, un ordre ni un pixel. La preuve : les TU existants, qui lisent ces listes et plient leurs contenus dans des constantes inchangées ; les 26 e2e au bit près ; `gcov` et mutations ; `sanitize-net` et le balayage ASan+UBSan pour les durées de vie.

### TU touchés (G)

1. **b1** : `fv`, `g_`, `ga`, `hj`, `mx`, `pm`, `probe_mm_rnd_order` (`qy`) ; `jc` (`ts`) ; `hy`, `ie`, `js`, `pj` (`invented`) ; `map_tables` (`gn` : `state->oc_count == 66`, comptes par entrée sur `state->oc[j]`).
2. **b2** : `fr`, `ga`, `ga_export_panel_by` (§6), `hj`, `hp`, `hy`, `jc`, `jo`, `lq`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `trade`, `ui`.
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change, à aucun commit.
4. **Assertion nouvelle** (hors le cas exigé au point 5) : admise seulement pour couvrir une ligne ajoutée non exécutée ou pour faire rougir une mutation qui ne rougit rien (trou), dans un test gardé du module, listée au message avec la ligne qu'elle couvre ; rien de lent, aucun fichier d'événements.
5. **Cas nouveau exigé (b2), `test_pico8_nr`** : un `nr` dont `jo.oe` tient une voie `w` vers une planète acceptante, et une proposition `lw` dont `lw->t->rl` contient déjà `w` (`mu` > 0) : `pico8_nr_cn(self, lw)` ne propose **rien** pour `w` (`pico8_trade_kg` n'est pas atteint pour `w` : le `cn` de l'autre extrémité n'est pas appelé, `lw->mu` inchangé, rien ajouté à `lw->ib`) ; cas témoin : même montage avec un `rl` sans `w`, la proposition part (`mu` ou `ib` changent). État construit directement, aucun fichier d'événements, rien de lent. Couvre l'exclusion `t_rl` (`planet_family_trade.c:138-143`) ; M20 doit le faire rougir. Assertion admise au titre de la règle 4 (trou constaté), listée au message avec la ligne qu'elle couvre.
6. **Prior art** : TU suiveurs dans le même commit (D.10b, D.11c, D.12a) ; constante figée d'étiquette (`FROZEN_INSTANCE_TAG`, D.11d3) ; plis de forme déclarés (D.11d1..d4) ; §6 réécrit avec preuve rouge (D.11d).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`), lancés comme le `Makefile` ; union des lignes de `src/` exécutées.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée (règle 4), sinon justifiée au message (branche `NULL` d'un élément, attendue morte : H3). Attendu : `jc:ch`/`jc:kj` (`ts`) ne sont exercés que par les e2e (aucun TU ne les appelle à `f2d5dad`) ; `gn_next` : par les TU de partie (`jo:hh`) et les e2e.
- `L` = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide**, hors les tests de genre et les aides retirés. Une ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail, fichier sauvegardé par `cp` avant et restauré par `cp` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan) ; chacune doit rougir au moins le TU nommé, sinon c'est un trou, couvert par une assertion (règle 4) ou justifié (mutation équivalente, démontrée). Astuce : supprimer un appel ou une entrée peut échouer sous `-Werror` (variable ou fonction `static` inutilisée) ; neutraliser par `0 ? f(…) : …` ou `if (0)` plutôt que supprimer.

- b1 :
  - **M1** `pico8_ez` n'ajoute plus à `qy` → `test_pico8_hj` (`list_size(…qy) >= 2`, `:533`) / `test_pico8_g_` ;
  - **M2** `pico8_ei` ne retire plus une particule expirée → `test_pico8_g_` / `test_pico8_fv` (tailles pliées après des images), sinon `make e2e` ;
  - **M3** `pico8_ga_kj` lit `qy->items[i]` au lieu de `[i - 1]` → `test_pico8_ga`, sinon `make e2e` ;
  - **M4** `pico8_jc_ch` n'ajoute plus à `ts` → `make e2e` (aucun TU, attendu), sinon assertion ;
  - **M5** `pico8_hy_kt` n'ajoute plus à `invented` → `test_pico8_hy` (`KT_HASH`, compte) ;
  - **M6** `pico8_gu_content_ptrs` compare par pointeur au lieu de `strcmp` → `test_pico8_hp` (`pico8_oj`, `:525-539`) ; l'éditeur de liens peut fusionner deux littéraux identiques de fichiers différents, la mutation peut donc survivre : alors, assertion ajoutée dans `test_pico8_hp` avec une copie (tableau `char` local) du nom inventé, `pico8_oj` doit rendre 1 (TROU-3) ;
  - **M7** `pico8_menu_invented_lines` saute le premier élément → `test_pico8_js` (`cr my`, `:930-949`), sinon assertion ;
  - **M8** `pico8_gn` range un ticket de moins par entrée → `test_pico8_map_tables` (66) ;
  - **M9** `pico8_gn_next` ne retire plus le ticket tiré (`0 ?` sur le décalage) → `make e2e` (galaxie tirée autrement), ou un TU de partie ;
  - **M10** `pico8_gn_nm_contains` rend toujours 0 → `make e2e`, sinon justifiée.
- b2 :
  - **M11** `pico8_jo_kf` n'ajoute plus à `oe` → `test_pico8_jc` (`list_count(…oe) == 1`, `:548`) ;
  - **M12** `pico8_jo_ib` ignore `self->by` (tas neuf toujours) → `test_pico8_ga` (`list_size(…by) == 2`, `:599`) / `test_pico8_ga_export_panel_by` ;
  - **M13** la fermeture `kf` ajoute sans `gu_trade` → `test_pico8_trade` (`count_kf`) / `test_pico8_os` ;
  - **M14** `pico8_trade_kg` n'ajoute plus `np` → `test_pico8_trade` (`list_is(k->lp, lp_n + 1, …)`, `:356`) ;
  - **M15** `pico8_trade_to` rend le premier de `lp` → `test_pico8_trade` (`:309`) ;
  - **M16** `pico8_trade_eq` compare les derniers au lieu des premiers → `test_pico8_trade` (`:408-412`) ;
  - **M17** `bd_route_visit` n'ajoute plus à `dest->cc` → `test_pico8_jo` / `test_pico8_hy` (`dh`, `gc`), sinon assertion ;
  - **M18** `pico8_jo_cn` concatène `jy` puis `{fresh}` → `test_pico8_trade` (`:507-520`) / `test_pico8_hy`, sinon assertion ;
  - **M19** `pico8_jo_lz_mw_step` range l'élément de `hp.i` sans copie → `test_pico8_hp` (`:679`) ;
  - **M20** `pico8_nr_cn` : `pico8_gu_ptrs(t_rl, w)` toujours 0 → le **cas unitaire nouveau** de `test_pico8_nr` (« Cas nouveau exigé ») rougit ; restauré, vert. Constaté par le vérificateur de D.12a1 : aucun TU n'exécute `pico8_nr_cn`, et `pico8_gu` à valeur muté pour rendre 0 sur une correspondance laisse les 26 e2e conformes : l'exclusion `t_rl` n'est gardée par rien aujourd'hui ;
  - **M21** `jo:gh` : la ligne « needs: » ne reçoit plus les éléments de `jx` → `test_pico8_planet_gh_row_lq` (§6) / `test_pico8_lq`, sinon `make e2e` ;
  - **M22** (§6, preuve rouge) le constructeur de `ga` n'installe plus `g_ga_by_shared` (`self->jo.by` laissé NULL) → `test_pico8_ga_export_panel_by` rouge ; restauré, vert ;
  - **M23** `pico8_hp_jb_join` saute les éléments de `hp.i` → `test_pico8_hp`, sinon `make e2e` ;
  - **M24** `pico8_jo_ex` ignore `is` → `test_pico8_lq` (`:431-446`) ;
  - **M25** `pico8_mx_kf` ne parcourt plus `jy` → `test_pico8_mx`.
- Cibles remesurées par contenu à `6cf4df4` : `particle.c:39-150`, `planet_family.c:514-560`, `:596-660`, `slipway.c:119-133`, `:266-313`, `slipway_display.c:18-165`, `economy.c:23-56`, `:277-300`, `menu.c:139-215`, `tech_gate.c:28-40`, `collect.c:8-148`, `gen.c:32-111`, `galaxy.c:37-121`, `planet.c:285-341`, `:457-480`, `planet_trade.c:19-252`, `planet_family_trade.c:111-160`, `:296-340`, `trade.c:23-178`, `economy_tiers.c:49-312`, `economy_score.c:87-140`, `economy_formulas.c:74-125`, `planet_display.c:56-232`, `research.c:155-245`, `research_display.c:31-290`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit. Aucun e2e ajouté.
- `make sanitize-net` à chaque sous-étape (ligne du plan : « oui »).
- `make class-coverage-net` à chaque sous-étape (constructeurs touchés, « Lecture »).
- **Balayage ASan+UBSan des TU** à chaque sous-étape : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.7b, D.10a, D.11a, D.11c, D.11e, D.12a.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 1 h 40 de temps réel pour l'étape** (b1 ≈ 45 min, b2 ≈ 55 min), mesures, `gcov`, mutations et filets compris ; **plafonds** b1 70 min, b2 85 min (au-delà : repli b2o/b2t). Base : D.11c ≈ 23 min pour 37 chemins, D.11d1..d4 ≈ 21 à 27 min, D.12a (a1 et a2 vérifiées sans repli). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute (`planet.h`, `trade.h`, `particle.h` changent : recompilation large) |
| `make e2e` | ~1 s |
| `make sanitize-net` | de l'ordre de la minute |
| `make class-coverage-net` | quelques dizaines de secondes |
| `make line-coverage EVENTS="tests/e2e/*.json"` | moins d'une minute (lignes de `ts`, de `gn_next`) |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes par sous-étape |
| mutations (b1 : 10, b2 : 15 ; un `make` ciblé chacune) | < 10 min par sous-étape |
| balayage ASan+UBSan des TU | quelques minutes par sous-étape |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà d'un plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; une sous-étape inachevée n'est pas commise. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + sous-étape + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | `6cf4df4` (D.12a gradée), avant b1 | 0 | ligne `liste` : 581 lignes, 86 fichiers | … |
| 2 | recette `liste` | idem | 0 | `liste 581 lignes 86 fichiers 625 occ.` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que les chemins de sa ligne de la table « Empreinte » (plus un TU suiveur listé au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`.
2. **Greps** de la « Sortie », exactement, après b2 ; après b1 : grep 1 restreint à `qy|ts|invented|oc`, `pico8_gu_content\b` absent partout, grep 5 vide.
3. **Formes** : chaque `all()` resté itérateur, chaque boucle à index restée à index (`ei`, `jc:kj` : TROU-14 ; `dh`, `db`, `bd_route_visit` sur `rl`/`lp`) ; `del` du premier identique ; `gen.oc` tableau fixe borné par un `assert` porteur de message.
4. **Casts** : chaque conversion d'un élément vers une struct précise, ajoutée ou gardée dans une fonction touchée, est précédée dans la même fonction du test qui la précédait ; seul cast nouveau admis : le `(void *)` de qualificatif d'`invented` ; aucun cast de pointeur de fonction ; aucun `typedef` ni macro qui renomme `pico8_list_t` ou `pico8_ptrs_t`.
5. **API** : les signatures de la table « classe (b) », et elles seules, ont changé de type.
6. **Aides** : `pico8_gu`, `pico8_gu_content`, `pico8_la`, `pico8_la_append` absentes ; `pico8_jp`/`pico8_nv` avec exactement leurs appelants L3/L4 (grep 3) ; `pico8_gu_content_ptrs` marquée TROU-3.
7. **Liens** : grep 7.
8. **Constantes** : grep 8 vide ; chaque `FROZEN_LIST_TAG` vaut `0x9E5C1157u` et remplace un `pico8_type_tag_of` sur une liste L2, listé au message.
9. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (c) = 0, (b) limitée à la liste ; lignes gagnées et lignes comptées modifiées listées ; aucun alias, registre, cale.
10. **Écarts déclarés** : `mz.ox` absent (D.12a), `g_ga_by_shared` converti avec `jo.by` ; déplacements depuis D.12c (`trade:kg`, `galaxy`, `jo:gh`) ; §6 : façon de lire seulement, preuve rouge M22 rapportée.
11. **Preuve** : `gcov` (lignes ajoutées exécutées ou justifiées ; `L` vide), mutations restaurées par `cp`, `sanitize-net`, `class-coverage-net`, balayage ASan+UBSan rapportés.
12. **Commentaires** : quatre sortes de `CLAUDE.md` ; les contrats devenus faux réécrits ; aucun renvoi à D.12b ni au plan dans `src/` ou `tests/`.
13. **Docs** : note « Découvert à D.12b » en fin de ligne D.12b (b2) ; TROU-3 et glossaire à jour ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `6cf4df4`, le commit qui gradue D.12a (ou un descendant qui ne touche que `docs/`), propre ; ou si `liste` (recette) n'y vaut pas 581 lignes / 86 fichiers / 625 occ. sans explication.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change **dans son intention** (constantes, cas, chaînes attendues, garde), ou si la preuve rouge de `ga_export_panel_by` manque ; changer sa façon de lire `by` est attendu et déclaré.
- `FAILED:` si une liste ou un parcours ne tombe sous aucune forme de ce PRD ni du §5 (arrêt « champ sans règle »), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si `make sanitize-net`, le balayage ASan+UBSan des TU ou `make class-coverage-net` n'est pas propre ou identique, à l'une des sous-étapes.

**Périmètre et forme**

- `FAILED:` si `tools/` ou le `Makefile` change ; si un commit touche un chemin hors de sa ligne d'« Empreinte » sans le lister comme TU suiveur.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit (après b2), ou si les greps partiels du contrôle 2 échouent après b1.
- `FAILED:` si une liste L3 ou une liste locale d'affichage autre que `hr_list`, `{np}`/`{jc}` de `trade:kg` et `prefix`/`last_only` de `jo:gh` est convertie ; si `pico8_jp` ou `pico8_nv` à valeur perd ou gagne un appelant hors de ce PRD.
- `FAILED:` si un `all()` devient une boucle à index, ou l'inverse ; si une boucle de TROU-14 compense son indice ; si `gen.oc` n'est pas un tableau fixe.
- `FAILED:` si une conversion d'élément vers une struct précise n'a pas son test dans la même fonction, si un pointeur de fonction est casté, ou si un cast autre que le `(void *)` de qualificatif d'`invented` est ajouté.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `liste` après b2 n'est pas inférieur au départ d'au moins les lignes b1+b2 retirées (recomptées au message), ou si l'écart à l'estimation n'est pas expliqué.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, registre, renommage d'un identifiant vivant pour échapper aux greps, commentaire compté réécrit hors de son sujet, cale valeur ↔ conteneur, littéral remplaçant un identifiant compté, (c) non nulle, (b) hors de la liste.
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées ; la justification de chaque Δlignes > Δoccurrences ; les includes gardés ou retirés ; les aides de TU retypées, créées ou retirées ; les `FROZEN_LIST_TAG` ; le cast de qualificatif ; le rapport `gcov` ; les mutations. Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne ajoutée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` n'est pas vide.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification ; si le cas unitaire `pico8_nr_cn` / `t_rl` de `test_pico8_nr` manque, ou si M20 ne le fait pas rougir.
- `FAILED:` si un plafond est dépassé sans note (et sans repli b2o/b2t pour b2), ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.12b », ou si elle omet l'un de ces éléments : les comptes réels, les formes, `mz.ox`/`g_ga_by_shared`, les API typées et ce qui quitte Q12, les aides retirées et survivantes, les déplacements depuis D.12c, le piège des étiquettes, les TU suiveurs (§6 compris), ce que D.12c/D.12d trouvent.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- Les registres, `mz.ox`, `pico8_ij`, le conteneur lui-même : D.12a.
- Les listes hétérogènes L3 (`lq.items`, `mh.items`, `jb.ls`, `lq.fs`) et les listes locales d'affichage (`ji`, `merged`, `lb`, `bj_list`, `cb_list`, `l` de `jo_ex`, `menu`, `msgbox:pz`, `help`, `score_panel*`, `pi`), `pico8_jp`/`pico8_nv` à valeur, `cb`, `bj`, le nom définitif des aides `_ptrs` : D.12c.
- `pico8_list.c/.h`, `test_pico8_list`, `PICO8_REF_AS_LIST*`, `PICO8_TYPE_TAG_LIST`, les includes gardés, les TU restants de `liste` : D.12d.
- Les API à valeur de Q12 (`pico8_jc_ce`, `pico8_trade_qe`, `pico8_oj`, `pico8_jy_qe`, `pico8_lq_bg`, `pico8_pj_ed`…) et les catégories `valeur`, `type-tag`, `ref-audit` : Q12 (étude D.9b), ouverte, posée à l'humain à la fin de D.12.
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **Périmètre** : `mz.ox` sorti (D.12a), `g_ga_by_shared` entré ; `hr` de `galaxy`, `{np}`/`{jc}` de `trade:kg` et trois concaténations de `jo:gh` déplacés depuis D.12c, parce qu'ils n'existent que pour une liste de l'étape.
  - **Formes** : conteneur de D.12a pour dix listes ; `gen.oc` tableau fixe de 66 ; `invented` de chaînes, avec un cast de qualificatif seul, déclaré.
  - **API** : `trade_kg`/`_to`, `kf`, `gn`/`gn_next`, `jo_ib`, `jo_ex` typées (leur valeur ne portait qu'un élément) ; `gen.nm[3]` quitte Q12.
  - **Aides** : `gu`, `gu_content`, `la`, `la_append` à valeur meurent ici ; `jp`, `nv` à valeur survivent (appelants L3/L4 de D.12c) ; noms `_ptrs` gardés ; `pico8_gu_content_ptrs` nouvelle.
  - **TU** : suivent dans le même commit ; aides retypées ou relayées sans va-et-vient ; constante figée `FROZEN_LIST_TAG` là où l'étiquette était pliée.
  - **§6** : `ga_export_panel_by` suit, preuve rouge refaite (M22).
  - **Découpage** : deux sous-étapes (≈ 45 et ≈ 55 min), repli b2o/b2t écrit.
- **Surprises de mesure, pour `plan-pilot`** : (1) l'empreinte de la ligne (« ≈ 25 (25, 0) ») est de 30 fichiers de `src/` et 25 TU ; (2) **le piège des étiquettes** : huit constantes de TU plient `PICO8_TYPE_TAG_LIST` à travers `fold_ref`/`fold_list_member` ; un conteneur sans étiquette les ferait rougir sans bug ; (3) les listes de `jo:gh` sont hétérogènes : `jy` y entre par trois concaténations que ni `pico8_la` ni `pico8_la_ptrs` ne peuvent faire ; (4) `hy.invented` tient des chaînes constantes et le conteneur prend `void *` : un cast de qualificatif est inévitable (ou un tableau fixe que le cart ne borne pas) ; (5) la borne de `gen.oc` (66) n'est écrite nulle part dans `src/` : elle vient de `df` et de l'unique `gn(df,2)` ; (6) `pico8_gu_content` ne sert qu'à `invented`, `pico8_gu` qu'à `t_rl` : les deux meurent ; `pico8_la` meurt aussi, au prix d'écrire au site les concaténations d'affichage ; (7) `jc.ts` n'est exercé par aucun TU (seulement les e2e) ; (7 bis) `pico8_nr_cn` n'est exécuté par aucun TU et son exclusion `t_rl` n'est gardée par rien (vérificateur de D.12a1 : `pico8_gu` muté, 26 e2e conformes) : cas unitaire exigé en b2 ; (8) `§6 : aucun` est faux pour `ga_export_panel_by`, qui lit `jo.by`.
- **Pour D.12c** : `ji` et `merged` de `jo:gh` reçoivent des `pico8_value_ref(e)` de besoins ; `pico8_jp` (`cb`, `bj`) et `pico8_nv` (`layout`) à valeur ; nom des aides `_ptrs` ; les listes locales d'affichage restantes.
- **Pour D.12d** : includes gardés (`particle.h`, `slipway.h`, `gen.h`, `planet_trade.h`, `research.h`, `trade.h`, s'ils le sont) ; les `FROZEN_LIST_TAG` des TU restent (ils plient une constante, pas `PICO8_TYPE_TAG_LIST`).
- **Inventaire exhaustif** : Annexe C (`src/`), Annexe D (`tests/`), numéros remesurés à `6cf4df4`.
- **Révision après D.12a** : toutes les hypothèses `H-D12a-n` tiennent, sauf H-D12a-8 (D.12a a **retiré** l'include de `pico8_list.h` de ses quatre en-têtes au lieu de le garder : sans effet ici) et H-D12a-6 à la lettre (`Makefile`, `float_guard.txt`, glossaire touchés en plus : sans effet ici) ; départ réel `liste` 581 / 86 / 625 occ. ; numéros changés seulement dans `collect.c/.h`, `test_pico8_mx`, `test_pico8_ga_export_panel_by` ; découpage et décisions inchangés.

</content>
</invoke>

## Annexe A — textes du plan recopiés (`docs/plan-3.md` à `6cf4df4`, par extraction de lignes ; depuis `f2d5dad`, seule la ligne D.12a a changé, par ses notes « Découvert à D.12a1/a2 »)

### §5, en-tête et règles de la phase 1a (l. 74-92)

## 5. Phases

### Phase 1a — Entités en structs, champs `pico8_value_t`

Transformation mécanique. Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`, comme aujourd'hui.

- **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
- **Clés construites à l'exécution** : `self[bt.."held"]` (`pico8_ui.c:328`) → deux champs `lheld`/`rheld` ; `ldown`/`rdown` côté méthodes. Tout autre site de clé dynamique trouvé est énuméré de même, ou déclenche un arrêt (§7) si l'ensemble des clés n'est pas fini.
- **Défauts mutés sur la classe** : `ie.gy`, `ie.id` (`pico8_research_icon.c:158,182`, `pico8_cart.c:158`) deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** (connus : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`) ; la liste des étapes les nomme.
- **Instances sans classe** : converties comme des classes à part entière, sans parent.
- **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
- **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag (`jo` pour `cg.jo`) ; seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille. En 1a, un membre nil de trop est sans effet (absent à la lecture) ; la phase 2 reclasse.
- **Ordre** : une classe par commit, la parente avant ses filles (`ke` → `jo` → `qh` → …). Les racines sans parent dans n'importe quel ordre.
- **Premier commit** : une vraie racine sans parent ni fille (`jb`), qui prouve la coexistence. **Deuxième** : `ke` **dans le même commit** que sa fille directe sans descendance de plus petite emprise (mesurée par le PRD de l'étape), pour que la struct de `ke` soit construite et vue par le filet dès son introduction. Ensuite les autres filles directes sans descendance de `ke`, par emprise croissante ; l'arbre `jo` et `g_`→`fv` après elles ; `ui` et `pj` (clés dynamiques, défauts mutés) en fin de liste.
- **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a, qui les supprime toutes avec `pico8_class.c`. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe. Les descendants non convertis trouvent leurs champs hérités en remontant `parent`, qui reste valide.
- **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre (dispatcher hérité dans `defaults`, ou appel direct `jo.kj(self,…)`, `jo.kf(self,…)`) reste à double forme : lecture/écriture par `pico8_entity_get_field`/`set_field`, ou aiguillage sur le tag comme `pico8_ke_ks`. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante (pour `jo` : 1a.26). La caractérisation de 1a.17 lit au moins une instance de chaque fille non convertie construite par le filet (`qh`, et `pm`/`ga` via `kv[hp.cw]`) : champ hérité de `jo` et résultat d'une méthode de `jo` appelée sur elle.
- **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` (grep `->defaults`, ~100 sites dans 22 fichiers) migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct : une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec. Le PRD de `jo` tranche `kw` (écrit en place par `mx:kf`, `planet_family_trade.c:280`, si l'instance n'a pas couru `lz`) : défaut modifié en place → état statique partagé, pointé aussi par `defaults`.
- **Sortie** : plus aucune `pico8_class_t`/`pico8_instance_t` à l'exécution ; `pico8_class.c` supprimé ; `make class-coverage-net` toujours sans zéro — vérifié par `grep -rn 'pico8_class_t\|pico8_instance_t\|->defaults\|lookup_named' src/` vide.


### §5, règles de la phase 1b (l. 151-168)

### Phase 1b — Types C réels

Struct par struct, chaque membre `pico8_value_t` prend son type C (`pico8_fixed_t`, `pico8_vec_t`, pointeur de struct typé, `const char *`, booléen…).

- **Nil** : un indicateur de présence **uniquement** là où le code observe nil (test de vérité, `mv` qui rend 0 sur champ absent, affectation `= nil`). Rappel : en Lua `0` est vrai, donc un fixe à 0 n'est pas nil. Pas de valeur sentinelle.
- Les REF deviennent des pointeurs typés : les sites de `pico8_ref_audit.h` disparaissent au fil de l'eau.
- **Recensement (1b.0)** : outil seul, aucune struct typée, sur le modèle de l'étape 0.2 et du build `-DPICO8_CLASS_COVERAGE`. Un build à drapeau recense, par (struct, membre), l'ensemble des genres de `pico8_value_t` écrits et si nil est observé par un lecteur (test de vérité, `mv` qui rend 0, `= nil`). Crochets : init des structs, écrivains `set_field`, lecteurs `*_truthy` et par nom, les ~24 sites à table d'offsets `*(pico8_value_t *)((char *)self + offset)`. Passé sur les rejeux de `FRAMES_EVENTS` et `make test`. Sortie : une table versionnée, et la durée du rejeu ASan+UBSan (§7). Un membre sans genre observé est signalé : son PRD ajoute une caractérisation dirigée avant de le typer.
- **Coexistence** (fixée à l'écriture de la liste, agent `guideline`) : le `field`/`set_field` du descripteur est le seul point de conversion ; les sites par nom (`pico8_entity_get_field`/`set_field`, `*_truthy`, `src/` et `tests/`) ne changent pas.
  - Lecture : le membre typé est rendu emballé, du genre qu'il aurait eu en 1a, un pointeur (REF, chaîne, fonction) repassé tel quel ; absent exactement là où 1a rendait 0 — pointeur NULL, ou indicateur de présence faux.
  - Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` (arrêt « champ sans règle », §7).
  - Tables d'offsets (`members[]`, `defaults[]`, `pico8_X_member`) : elles ne listent que des `pico8_value_t`. Le commit qui type un membre l'en retire partout, tables des filles comprises (`offsetof(fille, parent.m)`), et l'initialise par affectation typée ; un défaut REF garde la même REF.
  - Présence : un membre non pointeur lisible par nom (`l`) qui peut valoir nil (nil observé, ou tenu) garde un indicateur, sauf preuve du PRD qu'aucune lecture par nom ne le voit nil.
  - Garantit : présent/absent, valeur et identité de REF lus par nom identiques à 1a ; aucune écriture `*(pico8_value_t *)` sur un membre typé. Vérifié à chaque étape par `grep -n 'offsetof(pico8_T_t, \([a-z0-9_]*\.\)*m)' src/` vide (m typé, T la struct et ses filles) et la mutation (2) du §7.
  - Un site par nom remplacé par un accès direct est noté dans « Découvert à 1b.N » ; les autres survivent à 1b.
- **Pilote (1b.1)** : la plus petite struct dont la ligne de recensement mêle au moins un fixe simple, un membre à nil observé et un membre REF. Son « Découvert à 1b.1 » fixe le patron (disposition du membre typé, indicateur de présence, écriture à l'init, emballage du lecteur) ; les PRD suivants le citent, et s'en écarter est une « lecture nouvelle d'une règle » (§7).
- **Liste** : chaque ligne porte son empreinte mesurée (fichiers de `src/` et `tests/` qui lisent ou écrivent ses membres) ; une ligne de plus de ~40 fichiers est découpée dans la liste elle-même, par groupe de membres (1b.Na, 1b.Nb…).
- **Sortie** : aucune struct d'entité ne contient de `pico8_value_t` — constaté par une commande nommée : un script qui parcourt les typedefs de struct d'entité (premier membre `pico8_struct_header_t` ou struct d'entité enchâssée) et liste tout membre `pico8_value_t` ; sortie vide. Le même script sert de compteur de progrès à chaque étape.


### §5, phase D, en entier (l. 228-280) : puces, note de D.0, « Liste de D » avec empreinte, règles communes et ordre, arrêt « champ sans règle », décisions de D.9, comptes au départ de D.10, lignes D.2 à D.12d avec toutes leurs notes « Découvert à D.x » (D.12a1 et D.12a2 comprises)

### Phase D — Données

- Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
- Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
- Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
- **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.
- **D.0 — mesure seule, sans conversion, sur le modèle de 1b.0** (ajouté à la revue de sortie de 1b) : un compteur nommé (`make data-residue`, `tools/data_residue.sh`) compte, par catégorie, dans `src/` et `tests/`, les usages de `pico8_value_t`/`pico8_value_*()`, `pico8_ob_table_t`/`pico8_ob*()`, `pico8_list_t` si le §6 le retire, `pico8_type_tag_of` et `pico8_ref_audit.h` — dont les lecteurs/écrivains par nom (`pico8_entity_get_field`/`set_field`, `*_truthy`, `field`/`set_field` des descripteurs et leurs aides `*_take_*`), le dispatch à arguments `pico8_value_t`, les `overrides` des constructeurs, les spécifications de classe lues par `pico8_ob()` et leurs `pico8_X_default`. Sa sortie vide **est** la sortie de la phase D, et chaque ligne de la liste porte son « Compte : N ». D.0 met aussi `make value-members` et `grep -rn offsetof src/` dans `make test`, en mode vérification (code non nul si non vide), à côté de `float-guard` ; une catégorie du compteur tombée à 0 y entre de même. La liste ordonnée de D est écrite après D.0, à partir de sa sortie : une catégorie par groupe nommé, découpée au-delà de ~40 fichiers ; chaque test de régression du §6 qui lit par nom est rattaché au groupe qui force sa réécriture, avec sa preuve rouge. Une catégorie qu'aucune puce de D ne couvre est l'arrêt « champ sans règle » du §7. Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.
- **D.1 — suppression du recensement**, premier groupe après D.0 : `pico8_member_census.c/.h`, les crochets `SEE`/`READ`/`WRITE`/`CHILD_EXIT`/`STOP` de `src/` et `tests/`, `tools/member_census.*`, la cible `member-census` et ses entrées de listes de sources, le mode `--c-table` de `tools/entity_value_members.sh`. Les messages d'`assert` qui renvoient à « census row » / « plan-3 section 5 » sont réécrits pour nommer le contrat violé. Aucun `.sha256` ne change ; `make class-coverage-net` identique.

Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.

Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

- D.2 — `stash` : tables décodées par `unstash` — `g_hc`, `g_fd`, `g_dr`, `g_nb` (`data_init.c`, avec la passe `nb_mr` et son routage `gp={"nk"}`), `g_r`, `g_iz` (`pico8_stash.c`, lus par `pico8_r()`/`pico8_iz()`) → `static const` générés (première puce) ; le test ponctuel naît ici : chaque table générée est égale, entrée par entrée, à `pico8_ob(pico8_unstash(mem, pd))`, la mémoire étant chargée par `pico8_load_map_json` (pas de rejeu). `pico8_stash.c/.h` et `test_pico8_stash.c` restent jusqu'à D.8. Empreinte 22 (15, 7). Compte : `stash` = 26 (`pico8_stash.c` 7, `pico8_stash.h` 7, `test_pico8_stash.c` 12) ; `ob` ≤ 2195. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.
- D.3 — `ob`, tables constantes parsées au démarrage (liste de la première puce) : `g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb` (`game_data.c`), `g_ek`, `g_df` avec sa passe `df_mr` (`data_init.c`), `g_ci` (`entity.c`), `g_ix`/`g_ha` (`menu.c`), `g_bo` (`help.c`), `g_ef`/`g_bz` (`score_panel.c`) → `static const` générés, test ponctuel étendu. Empreinte 21 (17, 4). Compte : `ob` ≤ 2105. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).
- D.4a — `ob`, membres table de l'arbre `jo` : `jo.nu`/`ka`/`dz`/`cq`/`kw`/`em`/`nk`, `qh.px`/`ba`, `hj.k_`, `ga.k_`, `nr.hz`, `mx.ht` → struct ou tableau fixe par membre (troisième puce), forme tranchée par le PRD (le masque de bits de `nu` est la phase 2) ; leurs défauts table (`jo_default_table`, `family_default_table`, `family_trade_default_table`) sont générés `static const` dans la même étape (première puce). Empreinte 34 (22, 12). Compte : `ob` ≤ 2033. §6 : `gh_row_lq` (sa table `kw`, l. 102-120) et `ga_export_panel_by` (sa table `kw`, l. 98-117) construisent le nouveau `kw`, preuve rouge dans ce commit. `class-coverage-net` : oui (init des constructeurs). Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).
- D.4b — `ob`, autres membres table : `bg.pl` (et `g_bg_pl`), `hy.po`/`fu`, `fv.rb`, `mh.nu`, `g_.nu`/`k_`, `hp.mr`/`gp`/`fk`/`lq`, `pj.gs`/`i_`, `qb.pc` (et `g_qb_pc`), `jc.nu`/`k_`/`hg`, `ui.ic`, `js._cr_t`, `mz.nu`/`kk`, même règle. Empreinte 39 (27, 12). Compte : `ob` ≤ 1982. §6 : aucun. `class-coverage-net` : oui. Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp` : `lq_own`/`lq_inner`) ; `hp.mr`/`gp` en `pico8_str_seq_t`, `ho_apply` les prend typés ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages `po_own`/`fu_own` dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie ; une clé `qe` chaîne est absente) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus, `pico8_jc_hg_add`/`_has`) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`, qui partage son calcul avec `pico8_gi`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : 1882 (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de −100 (1882 − 1982).
- D.5a — `spec`, `ke` et les classes d'interface : `g_X_spec`, `pico8_X_default`, `pico8_X_defaults_reset` et aides `X_default_<genre>` de `ke`, `bg`, `ui`, `js`, `fq`, `jb`, `lq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`, avec les appels `pico8_ke_default` des autres fichiers → défauts `static const` générés depuis `pico8_ob(spec)`, test ponctuel étendu. Lecture de la première puce : une spécification de classe est une table constante parsée au démarrage. `test_pico8_spec_defaults.c` et `test_pico8_jo_spec_defaults.c` lisent les défauts statiques, même intention. Empreinte 30 (21, 9). Compte : `spec` ≤ 779. §6 : aucun. `class-coverage-net` : oui. Découvert à D.5a : deux chunks (interfaces, puis `ke` ; ≈ 40 chemins, `pico8_game_data.*`, `pico8_bootstrap.c` et les TU qui plient l'identité de `me` hors du majorant). Défauts générés `pico8_X_start` (type `pico8_X_start_t`, `pico8_game_data.h`), un par classe à clés (`ke`, `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`), clés propres de la spécification, lus par les constructeurs ; les tables gardent l'objet de D.4a/D.4b ; `mz.jj` s'initialise de `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` : liste statique de `ui.c` (`pico8_ui_le_shared`) ; `qb.v`/`p`/`ra`, `iv.v`, `ui.hw` pointent la donnée générée (membres `const`) ; `nf.ra` pointe le statique non `const` `g_nf_ra`, affecté depuis `pico8_nf_start.ra` à `pico8_nf_init` (repli : `test_pico8_entity_kind` et `test_pico8_ie` écrivent `je.ra` en place) ; `nf.l`/`nf.r` restent `g_nf_l`/`g_nf_r` (D.7b). Le cycle `_init`/`_shutdown`/`_ready` des treize modules reste, sur un drapeau. `pico8_ke_start_field` (compté par `par-nom`) remplace `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_g__default`, `pico8_ie_default` (retirés à D.5b) et `pico8_jo_default` (D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`) : D.5c le supprime. `pico8_rk_default` (variante de dessin, comptée par `spec`) est à D.5b. `spec` après D.5a : 634 (au lieu de ≤ 779) ; les majorants `spec` suivants se décalent de −145 (634 − 779). `par-nom` : 1090.
- D.5b — `spec`, `g_`/`fv` (aide), `hp`, `ie` (et `ie_chain`), `pj`/`rk`, `hy`, `trade`, même règle. Empreinte 23 (16, 7). Compte : `spec` ≤ 547. §6 : aucun. `class-coverage-net` : oui. Découvert à D.5b : deux chunks (exclusion de `rk`, puis les sept classes ; 29 chemins, `pico8_game_data.*` et trois TU hors du majorant). `pico8_rk_default` (variante de dessin à argument omis) entre dans `SPEC_EXCLUSION`, preuve : ses quatre lignes ne portent aucune autre catégorie. Défauts générés `pico8_X_start` de `g_`, `fv`, `hp`, `ie`, `pj`, lus par les constructeurs ; `hy`, `trade` sans objet. `ie` : un seul défaut pour la chaîne et les objets prêtés, `kq`/`ra`/`eh` pointent la donnée générée, plus libérés à l'arrêt. `fv.hi`/`gq`/`kq`, `g_.ra`, `ie.kq`/`ra`/`eh`, `pj.cr.hi`/`gq` : membres `const` vers la donnée générée, aucun repli. `pj.cr` : `g_pj_cr` reste (son `ep` est muté), valeurs de `pico8_pj_cr_start`. `pj.ou`/`pj.ea` : rangées générées `pico8_pj_ou_row`/`pico8_pj_ea_row`, d'où `pico8_score_panel_init` construit une fois par init les tables d'état `g_pj_ou`/`g_pj_ea` (`pico8_jb_row_view`), jamais libérées comme celles de la spécification ; leurs lecteurs (`hb`, `hk`, `kj`, `pico8_mh_new`) restent des tables ob (D.6b, D.7a). `test_pico8_spec_defaults` n'a plus de registre ni de constante. `pico8_ke_start_field` n'a plus que `pico8_jo_default` et le cas `M_KE`. `spec` après D.5b : 430 (au lieu de ≤ 402 après le décalage de D.5a : 28 lignes des sites de D.5b, retirées par D.4a/D.4b, étaient comptées à la fois dans le majorant 547 et dans ce décalage ; majorant corrigé 430) ; le « = 0 » de D.5c est inchangé. `par-nom` : 1085.
- D.5c — `spec`, arbre `jo` : `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, aides `family_default_*` et `family_trade_default_*`, même règle ; le `field` des sortes (`pico8_entity_kind_t`, `g_qh_kind` … `g_mx_kind`) lit les défauts statiques sous un nom hors motif, compté par `par-nom`. Empreinte 19 (6, 13). Compte : `spec` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. Découvert à D.5c : deux chunks (données, puis conversion et garde ; 27 chemins, `pico8_game_data.*`, `ke.c/.h`, le test ponctuel et le `Makefile` hors du majorant ; le lien du test ponctuel prend `pico8_planet_family_trade.c`, pour interroger `nr`/`fr`/`mx`). Défauts générés `pico8_X_start` des dix classes de l'arbre, clés propres, lus par les constructeurs, l'héritage par le `struct_init` de la parente ; chaînes dans des tableaux distincts, un par jeton (des littéraux se fusionneraient, TROU-3). `jo.kq`/`gq`, `pm.ng` : membres `const` vers la donnée générée, aucun repli. Le `field` des sortes est `pico8_X_kind_field` : `rr`, `s`, `me`, `od` seulement, lus dans `pico8_X_start`, puis la parente jusqu'à `pico8_jo_kind_field` (seul déclaré, `planet.h`) ; 0 si le module n'est pas prêt ; compté par `par-nom` (1106 après D.5c), à typer par D.9. `pico8_ke_start_field` supprimé. `_ready` sur `g_jo_up`, `g_family_up`, `g_family_trade_up`. `ga.by` : `g_ga_by_shared`, `pico8_list_t` statique non `const` vidée à chaque init (précédent `ui.le`), contenu à typer par D.7a. `spec` après D.5c : 0, dans `DATA_RESIDUE_GUARD`. `ob` : 1721 hors test ponctuel.
- D.6a — `overrides`, constructeurs de l'arbre `jo` (`pico8_jo_new` … `pico8_mx_new`), `new_entity` de `pico8_entity_kind_t` (`pico8_struct.h`) et ses appelants (`galaxy.c`, `jo_kv`…) : la table de clés de construction devient une struct locale (troisième puce), paramètre renommé. Empreinte 11 (8, 3). Compte : `overrides` = 315. §6 : `gh_row_lq`, commentaire de la l. 2 seulement ; aucune lecture ne change, pas de preuve rouge. `class-coverage-net` : oui. Découvert à D.6a : un commit (30 chemins : `research_icon.c` pour `ie:ldown`, `pico8_game_data.*` et quatorze TU qui construisent dans une variable `b`, hors du majorant). Clés typées : `pico8_jo_keys_t` (`id`, `p`, `ra`, NULL = absent, `pico8_struct.h`) pour neuf classes et `new_entity` ; `pico8_ga_keys_t` (`jo` et `pq`, NULL = défauts de `ga` ; `k_` NULL sous `pq` = nil) ; la chaîne que `pm:pq` parsait devient `pico8_pm_ga_pq_keys` (`pico8_ga_pq_keys_t`, générée) ; la sorte `ga` passe par une adaptatrice (`pico8_ga_kind_new`, `pq` NULL). `pico8_X_new(NULL)` inchangé ; les classes « seulement `id` » (`qh`, `nl`, `os`, `hj`, `fr`, `mx`) l'assertent sur `p`/`ra`. Tests de mort d'une clé positionnelle, sans membre ou hors rangée à la construction retirés (le type les interdit). `pico8_ob.h` quitte `planet.h`, `planet_family.h`, `planet_family_trade.h` ; `pico8_struct.h` le garde (`slipway.h`, `wormhole.h` en dépendent par lui). Le commentaire l. 2 de `gh_row_lq` parle des clés de `js` : il passe à D.6b (le changer ici serait un renommage à type constant). `overrides` après D.6a : 329 lignes / 348 occurrences / 55 fichiers (au lieu de 316 : le « = 315 » de D.1 était le total des fichiers de l'empreinte, dont 30 lignes de D.6b/D.6c — `iv`/`jc` de `planet.c` et `planet_family_trade.c`, `jc` de `test_pico8_mx`, `gh_row_lq`, `ie` de `test_pico8_entity_kind` — et sans 16 lignes de D.6a — `ie:ldown`, `nr` de `test_pico8_mz`/`test_pico8_bg` ; la ligne de `k_` ajoutée par D.4a est dans le périmètre) ; le compte de D.6b est à reprendre : sur les 329 lignes, 232 sont à D.6b (lignes classées par le constructeur appelé ou déclaré, commentaires de `lq`/`jb`/`gh_row_lq` compris, `iv` d'`economy.c` et `jc` des TU `hp`/`hy`/`trade`/`ui`/`mx`/`mz` compris) et 97 à D.6c : « `overrides` = 97 » après D.6b au lieu de 117 ; le « = 0 » de D.6c est inchangé. `par-nom` : 1075.
- D.6b — `overrides`, mise en page et entités d'écran : `lq`/`jb` (`layout`), `js`/`fq` (`ui_button`), `mh`/`iv` (`msgbox`), `ui`, `qb` (`scroll`), `nf` (`mouse_reader`), `bg`, `mz` (`wormhole`), `jc` (`slipway`), appelants de `menu.c`, même règle. Empreinte 30 (19, 11). Compte : `overrides` = 117. §6 : `layout_nil_item_draws_nothing` (l. 106-112) passe son item par la struct de clés de `pico8_lq_new` ; l'item reste un `pico8_value_t`, élément de `lq.items` ; même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : oui. Découvert à D.6b : trois chunks (classes sans clé ; `iv`/`jc`/`fq` ; `lq`/`jb`/`js`/`mh` avec les ponts, `dr` et le §6 — 58 chemins : appelants à NULL, `economy.c`, `score_panel_display`, `research_display`, `data_init` et les TU qui construisent dans `b` hors du majorant). `bg`, `nf`, `qb`, `ui`, `mz` sans paramètre ; clés typées `pico8_iv_keys_t`, `pico8_jc_keys_t`, `pico8_fq_keys_t`, `pico8_lq_keys_t` (items `pico8_value_t`), `pico8_js_keys_t` (`lq`/`h_`/`jb` restent des sources de rangée `pico8_value_t`, présence TROU-23), `pico8_mh_keys_t` ; `jb` prend `const pico8_jb_row_t *`. `js:pq` copie une rangée par `pico8_lq_keys_of` et une liste par ses éléments (plus de vue ni de conversion) ; `jb:pq` n'emballe plus ses lignes chaîne ; `fq(de)` lit `pico8_de_my()`. Pont `pico8_dr()` fermé (`pico8_jb_new(&pico8_dr_table[i])`). Ponts nouveaux, paramètre `overrides` gardé (type inchangé, comptés, 21 lignes) : `pico8_lq_new_table`, `pico8_jb_new_table` (`layout.c`), `pico8_js_new_table` (`ui_button.c`), `pico8_mh_new_table` (`msgbox.c`) — fermés quand les rangées de `pz` et les sources `lq`/`h_`/`jb`/`mh` sont typées (`planet_display`, `menu`, `score_panel_display`, `research_display` : D.7a ; `help`, `msgbox` : D.7b ; le « 0 dans `layout` » de D.7a exige les deux premiers). Les TU d'un pont gardent leur table : `test_pico8_mh` (clé `lh` nil, clés refusées : 7 lignes `overrides`, table de même type et même nom) et `test_pico8_lq` (deux commentaires qui nomment la table passée au pont) tombent avec lui. Reportés, avec eux : `pico8_ix()`/`pico8_ha()`, `g_bo`, `pico8_bz()` (vues par init ; `ix` porte un item imbriqué que seule une vue sait donner à `lq:mn`), les vues à l'appel de `hp.lq` et de `kw`. `pico8_lq_table_view` sans appelant de `src/` ; une table qui porte `fs` ou `w` est refusée par le pont `lq` (aucun producteur n'en pose). `overrides` après D.6b : 127 lignes / 138 occurrences / 27 fichiers (97 lignes de D.6c + 21 des ponts + 9 des TU des ponts ; le « = 117 » de D.1 était un total de fichiers, qui comptait dans ceux de D.6c 23 lignes de D.6b — `iv` de `economy.c`, `jc` de `test_pico8_hp`/`_hy`/`_trade`) ; le « = 0 » de D.6c devient « = 30 » (les ponts et leurs TU), et `overrides` entre dans `DATA_RESIDUE_GUARD` à l'étape qui ferme le dernier pont. D.7a hérite d'un chantier plus lourd : fermer les ponts `lq`/`jb` demande de typer les producteurs de rangées de `planet_display`, `menu`, `score_panel_display`, `research_display` et les rangées `bs` de `pz` (fichier de D.7b) ; son PRD tranche l'ordre ou la fusion avec D.7b. `par-nom` : 1062.
- D.6c — `overrides`, reste : `g_` (`probe`), `fv` (`help`), `ie` (`research_icon`, `cart.c`), `hp` (`research`), `hy` (`economy`), `trade` (et `planet_trade.c`), `pj` (`score_panel`), même règle. Empreinte 21 (16, 5). Compte : `overrides` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. Découvert à D.6c : deux chunks (`hy`/`fv`/`pj`/`g_`/`ie`/`trade`, puis `hp` et le pont `fd` — 37 chemins : appelants à NULL, TU qui construisent dans `b`/`o`, `data_init`, `collect` et le test ponctuel hors du majorant). `hy`, `fv`, `pj` sans paramètre ; clés typées `pico8_g__keys_t`, `pico8_ie_keys_t`, `pico8_trade_keys_t` (triple `qe` du membre ; TROU-24 au site de `trade:kg`), `pico8_hp_keys_t` (`ri` : enregistrement `pico8_fd_entry_t`, `rr`) ; `hp` applique `ho` aux champs typés (`pico8_ho_gp_has`, extraite de `collect.c`). Pont `pico8_fd()` fermé (`pico8_research_build_gb(void)` parcourt `pico8_fd_table`) ; `pico8_ho_apply` sans appelant de `src/` (D.7a `collect`, ou D.8 avec le test ponctuel). `overrides` après D.6c : 30 lignes / 34 occurrences / 8 fichiers (les 21 lignes des quatre ponts de D.6b et les 9 de leurs TU, le « = 30 » de la note de D.6b ; le « = 0 » de D.1 comptait sans eux) ; `overrides` n'entre pas dans `DATA_RESIDUE_GUARD` : il y entre à l'étape qui ferme le dernier pont — `js`, dont les rangées `bs` de `pz` sont à D.7b ; D.7a ferme `lq`, `jb`, `mh` et les autres sources de `js` si son PRD ne réordonne pas. La table `{qe=r.e}` de `jo:cn` (`planet_trade.c`, `jy`) reste une `pico8_ob_table_new` : ce n'est pas une clé de construction (D.7a). `par-nom` : 1038.
- D.7a — `ob`, tables ad hoc de l'affichage, de l'économie, de la recherche et des données (troisième puce) : tout `ob` restant de `planet_display`, `research_display`, `score_panel_display` (`g_de`, `spec1`/`spec2`), `slipway_display`, `wormhole_display`, `layout`, `menu`, `collect`, `economy*`, `research` (`g_gb`, `pico8_fi`), `planet_trade` (enregistrements de `jo:ib`), `tech_gate`, `score_panel`, `pi`, `gen`, `galaxy`, `cart`, `data_init`, `game_data`, `mouse` → structs locales ou tableaux fixes, les TU touchés suivant dans le même commit. Empreinte 39 (39, 0) aujourd'hui. Compte : `ob` ≤ 1057, et 0 dans ces fichiers. §6 : `ga_export_panel_by` (`route_a`/`route_b`, l. 102-103 et 147-150, enregistrements de `jo:ib` poussés dans `ga.by`), preuve rouge. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.7a : six chunks (familles isolées ; `gb` ; enregistrements `{qe,gc}` ; articles de `bl` et panneau de score ; pipeline d'affichage et chaînon `pz` ; `nb` — 74 chemins : suiveurs forcés, TU qui lisent `gb` ou les ponts, test ponctuel hors du majorant), **commis dans l'ordre 1, 2, 4, 3, 5, 6** : les enregistrements `{qe,gc}` de `jo.jy` (chunk 3) et les sous-rangées de `bz` (chunk 4) sont des tuples de `lq:mn`, que `layout.c` lisait comme tables ob ; typer l'une ou l'autre famille avant ce lecteur demandait une conversion typée → ob ou un `lq:mn` à double lecture, tous deux interdits. Le chunk 4 a donc pris tous les tuples de `lq:mn` (et les articles de `bl`, qui partageaient la même vue de `data_init`), le 3 a suivi, le 5 est resté le chaînon `pz` sans les tuples ; décision du pilote. **Ordre D.7a/D.7b : D.7a a pris à D.7b le chaînon `pz`** (`pico8_pz`, `pico8_mh_new_table`, `g_bo` et `fv.rb` de `help.c`, `pico8_js_new_table` et les branches `OB_TABLE` de `js:pq`), **`g_nf_l`/`g_nf_r`** (avec `bu`) et les lignes qui lisaient ses types convertis dans `planet.c`, `planet_family_trade.c`, `ui.c` ; D.7b ne les a plus. Formes : rangées, tuples de `lq:mn` et articles de `bl` en `pico8_jb_row_t` (sorte REF opaque pour `ld` ; `{qe=x}` gardé comme `has_qe`/`qe` de la rangée et lu `{x,5}`) ; `pz(p, bs, n)` sur `pico8_js_keys_t` ; sources `lq`/`h_`/`jb` typées (`lq_row`/`h_row` : `pico8_lq_keys_t`, `pico8_lq_keys_of_row` ; `jb_row`) ; `{qe,gc}` en `pico8_jy_t` ; `gb` tableau fixe (`pico8_gb_at`, `pico8_gb_find`) ; `nb.e_` (`pico8_nb_e_`, remis par `pico8_data_init`), `pj.ou`, `pj.ea`, `bz` (`pico8_pj_bz`), `nf.l`/`nf.r` statiques non `const` ; `pico8_de()` supprimé (`pico8_de_my`). Vues fermées : `ix`, `ha`, `bo`, `bz`, `nb`, `pico8_jb_row_view`, `jo_kw_view`, `hp:jb`, `pico8_lq_table_view` ; `pico8_data_init(void)`. `overrides` = 0, dans `DATA_RESIDUE_GUARD` depuis le chunk 5. `ob` dans les fichiers de la ligne : 13 lignes (enregistrements de constructeurs de `ob()` `'v'`/`'s'` dans `pi.c` et leur commentaire de `pi.h`, `'b'` et `pico8_ho_apply` dans `collect.c`/`.h`), servant le test ponctuel, supprimées par D.8 ; le « = 187 » de D.7c devient 187 + 13. `ob` après D.7a : 552 lignes / 613 occurrences, dont 199 dans le test ponctuel (hors test ponctuel : 353) ; majorant de D.7b : ≤ 309 hors test ponctuel (353 moins les 44 lignes `ob` de ses fichiers de `src/`). Les greps de sortie `pico8_fi\(` et `pico8_ob_table_t \*g_` ne rendent plus que `pico8_ob.c/.h` et `pico8_stash.c`, laissés à D.8. Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : 142406 / 110943 → 19213 / 59310 octets (3480 / 2423 → 1099 / 1481 allocations). `par-nom` : 1009. Correctifs aux messages de commit de D.7a (comptes remesurés sur chaque arbre) : (1) classification des lignes retirées, (a) construction supprimée, (b) signature, contrat ou variable dont le type passe d'une table ob à un type C, (c) même type, autre nom : 5e58f7c, `ob` 340 lignes = (a) 317 + (b) 23 (les contrats de `pico8_pz` dans `msgbox.h` et de `pico8_hp_jb` dans `research_display.c/.h` ; les variables retypées `b` de `pz_step`, `row` de `jo:fe`, `l` de `jo.ex`, `row` de `cr_step` ; dans les TU, `build`, `fold_built_from`, `positionals`, `call_row_my` et leurs 12 variables `t`/`spec`/`lq_t`/`h_t`/`b`/`jb`), `overrides` 18 = (a) 18 (les trois ponts et leurs deux commentaires de TU) ; (c) = 1, hors de toute catégorie comptée : `pico8_bz()` → `pico8_pj_bz()`, renommage à type égal fait après que fa997e7 avait déjà typé son retour, parce que la grep de sortie `pico8_(ix|ha|nb|bz|de)\(\)` contredisait la l. 326 du PRD (qui fait rendre à `pico8_bz` les rangées typées) ; aucun compte n'en bouge. 1a1687a, `ob` 58 lignes = (a) 54 + (b) 4 (`t` de `cr_step` et du prédicat de `cb`, `tech` de `test_pico8_data_init`, `record` du test ponctuel), `overrides` 0, aucune (c). (2) lignes qui gagnent une catégorie : 5e58f7c, `valeur` `help.c` +1, `layout.h` +1, `menu.h` +1, `ui_button.h` +1 ; `type-tag` `test_pico8_js` +1 ; `dispatch` `menu.h` +1, `test_pico8_fq` +1 ; 1a1687a, `ob` +7 et `valeur` +6 dans le test ponctuel (lecture de la référence décodée) ; fa997e7 omettait `valeur` `help.c` +1 et `layout.c` +2 ; c5bee4b : `valeur` `research.h` +1, `test_pico8_hp` +2, `test_pico8_lq` +4, `test_pico8_trade` +2. (3) fa997e7 a aussi ajouté une lecture ob → typée temporaire dans `jo:gh` (`jo_gh_record_tuple`, `planet_display.c` l. 85 : `qe` lu dans l'enregistrement ob pour poser le tuple typé), retirée par c5bee4b : l'ordre 4 avant 3 demandait lui aussi une conversion, le temps d'un commit. (4) c5bee4b : `sizeof(pico8_jy_t)` vaut 48, pas 40 (40 est `pico8_jb_row_t`). (5) La hausse de fuite de c5bee4b à 5e58f7c (+32 o / +4 allocations sur `message_box_hold`, +8 o / +1 sur `game_end_final_score`) vient d'allocations faites une fois par `pico8_fv_pq` à la construction de l'écran d'aide (le `ra` partagé et les `qf` de ses boutons, joignables par `g_bo` avant), pas d'une fuite par frame.
- D.7b — `ob`, reste de `src/` hors `pico8_ob.*` et `pico8_stash.*` : entités et interface (`planet*`, `probe`, `particle`, `slipway`, `wormhole`, `msgbox`, `mouse_reader` avec `g_nf_l`/`g_nf_r`, `ui*`, `help`, `scroll`, `trade`, `bg`, `ke`, `entity`, `bootstrap`, `hud`, `research_icon`) et en-têtes (`pico8_struct.h`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_value.h`, `pico8_list.h`), même règle. Empreinte 42 (42, 0) aujourd'hui. Compte : `ob` ≤ 827, et 0 dans `src/` hors ces deux modules. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.7b : trois chunks (`nf.l` dans `test_pico8_spec_defaults`, qu'une table ob écrivait hors du `pico8_nf_button_t` de D.7a ; particules de `qy` ; inclusions et `assert` d'init — 26 chemins). Le TU corrompu : `test_second_jl_keeps_specs` posait une clé de table ob dans `g_nf_l` (16 octets) et débordait sur `g_nf_r` depuis `e252fde` ; il passait en relisant sa propre corruption. Réparé (marque dans `hm`), preuve rouge sous ASan. Un balayage des 81 TU sous `-fsanitize=address,undefined` (copie jetable, `Makefile` inchangé) n'a rien trouvé d'autre ; les TU ne sont compilés sous ASan dans aucune cible. Formes : `pico8_qy_t` (`p`, `v`, `has_a`/`a`, `l` par valeur ; une allocation par `mm`, mise à jour en place par `ei`, plus d'allocation par frame) ; `pico8_help_init` lit `pico8_pi_dispatch_lookup` ; `slipway.h`, `ui.h`, `wormhole.h` incluent `pico8_list.h`. `pico8_ob.h` n'est plus inclus dans `src/` que par `pico8_ob.c`, `pico8_collect.h`, `pico8_stash.h`. R = 22 : les 13 de D.7a et 9 lignes de commentaire qui décrivent la couche encore vivante (`pico8_type_tag.h` 2, `pico8_ref_audit.h` 4, `pico8_value.h` 1, `pico8_list.h` 1, `pico8_hud.c` 1), que D.8 retire ou réécrit avec `pico8_ob.h` et `PICO8_TYPE_TAG_OB_TABLE` ; le « 187 + 13 » de D.7c devient 187 + 22. `ob` après D.7b : 491 lignes / 550 occurrences (hors test ponctuel : 292 ; `src/` : 147 ; TU hors test ponctuel, `test_pico8_ob` et `test_pico8_stash` : 83 lignes dans 28 fichiers, le travail de D.7c). Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : 19213 / 59310 → 19213 / 58910 octets (1099 / 1481 → 1099 / 1431 allocations) ; `message_box_hold` n'exécute ni `ei` ni un `mm` (son `qy` reste vide) ; `tech_geoharvesting_burst` : 240568 → 47328 octets (23752 → 1682 allocations).
- D.7c — `ob`, TU restants : fixtures construites en `pico8_ob_table_*` → structs, état construit directement. Empreinte 41 (0, 41) aujourd'hui. Compte : `ob` = 187 (`pico8_ob.c` 82, `pico8_ob.h` 32, `test_pico8_ob.c` 55, `pico8_stash.c` 8, `pico8_stash.h` 3, `test_pico8_stash.c` 7). §6 : aucun. `sanitize-net` et `class-coverage-net` : non, aucun fichier de `src/` ne change. Découvert à D.7c : trois commits (inclusions seules ; valeurs hors rangée, tests fermés et `by` ; plis de hachage — 29 chemins, 28 TU et ce plan, aucun de `src/`). Les cas hors rangée qui offraient une table ob à un membre typé sont retirés quand un frère garde la clause, remplacés par une liste sinon (`ma`/`m_` de `test_jo_writers_stop_off_row`) ; les tests fermés passent une liste locale ; `test_pico8_ga` poussait des tables ob dans `by`, qui tient des routes depuis D.7a : il pousse des routes. Aucune constante `*_HASH` ne change. Arbitrage du pilote : les trois octets de sorte de clé (`fold_names` de `test_pico8_hp`, `fold_qe_map` de `test_pico8_hy`, `fold_hg` de `test_pico8_jc`) ne deviennent pas des littéraux `0u`/`1u` (un identifiant compté remplacé par sa valeur est un alias) : ces trois lignes et les trois inclusions de `pico8_ob.h` qu'elles demandent restent, et D.8 les hérite avec la l. 2 de `test_pico8_planet_gh_row_lq` (R_t : l'énoncé du bug d'un test du §6 nomme `pico8_ob_table_t`), d'où « §6 : `gh_row_lq`, commentaire de la l. 2 seulement, pas de preuve rouge » à D.8. Les octets de `hp` et `hy` sont atteints (les retirer rougit `DEFAULTS_HASH` et `CONSTRUCTION_HASH`) : D.8 doit plier la même valeur sans la couche, arbitrage à reprendre, jamais une constante mise à jour ; celui de `jc` ne l'est pas (aucune constante ne plie un `hg` non vide : sauter toute la boucle de `fold_hg` laisse chaque constante verte). `ob` après D.7c : 415 lignes / 470 occurrences dans 20 fichiers (hors test ponctuel : 216 = 187 + 22 + 1 + 6, les 6 étant ces trois plis et leurs trois inclusions) ; `tests/` ne nomme plus `PICO8_TYPE_TAG_OB_TABLE` ; `type-tag` 373 → 362 lignes (592 → 572 occurrences). Balayage ASan+UBSan jetable des 28 TU touchés : propres.
- D.8 — suppression : `pico8_ob.c/.h` (avec `pico8_fi`), `pico8_stash.c/.h`, `test_pico8_ob.c` et `test_pico8_stash.c` (§6, tests de la couche), le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, leurs entrées du `Makefile`. Compte : `ob` = 0, `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : non. Découvert à D.8 : deux commits (octet de `jc`, non atteint : `test_pico8_jc.c` seul, chaque constante inchangée, un octet remis reste vert ; suppression — 26 chemins, dont `tools/float_guard.txt`, que `make float-guard` réécrit dans `make test` : `garde fichiers` 60 → 58, les deux fichiers du coeur supprimés, précédent D.2 ; 27 en tout). `test_pico8_map_tables.c` perd la comparaison à la référence (`decode`, `parse`, les `*_SRC`, les comparateurs, `pico8_load_map_json`, `pico8_pi_init` et `pico8_data_init` de `main`) et garde les tests des modules typés qui ne la lisent pas : six tels quels, et les bornes de `pico8_int_seq_at`/`pico8_fixed_seq_at` et les recherches `r`/`iz`/`hc`/`df` reformulées sur la table typée, sans assertion nouvelle. Les octets de sorte de clé de `hp` et `hy` sont pliés par une constante locale nommée (`VIEW_INDEX_KEY`, existante ; `TABLE_INDEX_KEY`, nouvelle), octet gelé d'une énumération supprimée, (c) déclarées ; aucune constante `*_HASH` ne change. `PICO8_TYPE_TAG_OB_TABLE`, `PICO8_REF_AUDIT_EXPECT_OB_TABLE` et le compteur `ob_table` de l'audit supprimés (`tools/ref_audit_report.sh` inchangé, son mot `ob_table` est mort) ; `pico8_ho_apply`, les constructeurs `'b'`/`'v'`/`'s'` et leur enregistrement supprimés ; TROU-4 et TROU-9 sans site. `ob` = 0, `stash` = 0, dans `DATA_RESIDUE_GUARD` ; `type-tag` 352 lignes / 560 occurrences (au lieu de ≈ 354 : les renvois de `pico8_list.h` et de `pico8_ref_audit.h` à l'explication du pas de lecture de `pico8_type_tag.h`, retirée avec la table ob, partent aussi), `ref-audit` 56 / 57, `valeur` 4961 / 5742 ; `par-nom` 1009, `dispatch` 191 inchangés ; résidu D 5868 lignes dans 128 fichiers. `sanitize-net` propre, `class-coverage-net` identique. **Sortie de la phase non atteinte** : `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h` restent, avec `valeur`, `type-tag`, `ref-audit`, `par-nom`, `dispatch` ; la suite attend les décisions humaines de D.9.

**Arrêt « champ sans règle » (§7), constaté à l'écriture de la liste : elle s'arrête à D.8.** Aucune puce de D ne dit ce que deviennent `par-nom` (1081/80) et `dispatch` (197/59). La première et la troisième puce ne couvrent que des tables : `ob` et `stash`, plus, par lecture, `spec` et `overrides`. La **Sortie** nomme des fichiers à supprimer, pas la forme de leurs usagers, et D.0 ne fait que compter. `valeur` (5989/132), `type-tag` (445/71) et `ref-audit` (70/17) ne peuvent tomber qu'avec eux. Les décisions de D.9 (ci-dessous) lèvent cet arrêt pour `par-nom`, `dispatch` et `pico8_list_t` (D.10 à D.12) ; la Sortie de D exige encore le travail sur `valeur`, `type-tag` et `ref-audit`, suspendu à Q12.
1. `par-nom` : `pico8_entity_get_field`/`set_field` (132 lignes de `src/`), `*field_truthy(`, les `field`/`set_field` des descripteurs (`pico8_struct_field_fn`, `pico8_struct_set_field_fn`), le `field` des sortes et 191 lignes d'aides `*_take_*`. La puce **Coexistence** de 1b les fait survivre à 1b, et aucune puce ne dit en quoi ils se changent. Les aides à nom non littéral (`field_fixed(inst, name)` et semblables, dans `economy_score.c`, `particle.c`, `probe.c`, `score_panel.c` et `entity_geom.c`) relèvent en plus de la clé dynamique du §5.
2. `dispatch` : `pico8_value_function_t methods[PICO8_METHOD_COUNT]` (`pico8_struct.h`), `pico8_mv()`, et les membres fonction (`js.my`/`hb`, `mh.lh`…). Typer les arguments demande une signature par méthode, qu'un tableau indexé par `pico8_method_t` ne tient pas. Or la note de D.0 dit que la table survit à D.
3. `pico8_list_t` : la note de D.0 exige d'en changer la représentation sans en fixer la forme. `lq.items` tient des chaînes, des fonctions, nil et le fixe 0 (`layout_nil_item_draws_nothing`), et aucune puce ne type ces éléments hétérogènes.

- D.9 — **étude, sans conversion** (demandée par l'humain à l'arrêt) : pour chacun des trois points ci-dessus, recenser les sites par grep (fichier, fonction, noms lus ou écrits, genres des valeurs), les classer par forme de remplacement possible — accès au membre typé (`p->hp`) à la place de la lecture par nom ; fonction C par méthode, à arguments typés, à la place de `pico8_mv` et de la table `methods[]` ; tableau typé, union étiquetée locale ou struct à la place d'une `pico8_list_t` d'éléments `pico8_value_t` —, et pour chaque classe de sites : la forme retenue, le nombre de sites, ce qui n'entre dans aucune forme (clé dynamique du §5 incluse). Sortie : une section de ce plan qui propose les puces à ajouter à D (ou à reporter en phase 2), avec leur liste d'étapes et leurs comptes ; l'humain tranche avant la première étape qui les applique. D.2 à D.8 ne dépendent pas de cette étude.

**Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Comptes au départ de D.10 (HEAD 5a13008, `make -s data-residue` et `--detail` ; l'étude mesurait `2dec71f`, avant D.7a–D.8) : `valeur` 4961/126, `type-tag` 352/65, `ref-audit` 56/15, `par-nom` 1009/81 (`src/` 433/43, `tests/` 576/38), `dispatch` 191/61 (`src/` 67/30, `tests/` 124/31), résidu D 5868 lignes / 128 fichiers ; `liste`, pas encore au compteur, 927/99 avec le motif de Q10 (`src/` 496/61, `tests/` 431/38). Les classes A–M, T1/T2 et L1–L4 sont celles de l'étude (annexe A) ; ses « ≈ » viennent de son classement, chaque PRD remesure. Un TU qui nomme un symbole retiré par une étape de `src/` la suit dans le même commit, ou le symbole reste comme pont compté, fermé par l'étape TU de son groupe (D.10c, D.11d, D.12d) ; le PRD tranche. `sanitize-net` : aux seules étapes qui changent une durée de vie ou un propriétaire (§7).

- D.10a — `dispatch`, table de méthodes : `methods[]` → struct de pointeurs typés, 23 emplacements nommés (Q2), et table d'états typée de `dg` (`ph`, `ly`, `ll`, `kz`, `pr` ; Q3) ; les 82 adaptateurs `(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1)` retirés ; les 26 appels `pico8_mv` de `src/` remplacés (appel direct de la fonction typée pour `eb`, `ib`, `gg`, `md`, `lq`, `lh` ; virtuel pour le reste) ; `dg`, `dc`, `ui:jz`, `lt`, `he` et les prédicats `*_is_*_class` sur les emplacements ; `pico8_entity_get_field` ne rend plus de méthode ; `hd=1` en fonction constante vraie ; `pico8_dispatch_context` gardé (Q4) ; sites `par-nom` G (9) et E de `mv` (1). Deux chunks : a1 (méthodes lancées par `dc`, `lt`, `he`, `ui:jz`, `dg`), a2 (le reste). Empreinte 37 (37, 0) aujourd'hui. Compte : `dispatch` `src/` ≤ 35 (67 moins les 29 lignes `pico8_mv`, `entity.c:177`, `methods[]` de `pico8_struct.h` et le commentaire de `pico8_dispatch_context.h`), ≤ 37 si `pico8_mv` reste en pont pour les TU ; `par-nom` ≈ 999 ; `valeur` rapporté. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non. Découvert à D.10a : deux chunks (a1 : 73 chemins ; a2 : 62). `pico8_struct_class_t` porte 14 membres typés (`kj`, `ev`, `hd`, `hk`, `hb`, `ldown`, `rdown`, `gv`, `hq`, `ey`, `kf`, `cn`, `ol`, `mm`), la table `states[]` (`pico8_state_t`, `pico8_state_of`) et le crochet `has_hb` (présence de `hb` par instance, pour `js`) ; `ldown`/`rdown`/`gv` rendent `pico8_down_t {stop, held}` ; `eb`, `ib`, `gg`, `md` sont appelés directement (`pico8_jo_is_family`) ; `hd=1` est `pico8_hd_true` ; `pico8_ui_jz` prend un sélecteur. Pas de pont : les TU ont suivi (les lignes `pico8_mv` et les lignes `pico8_value_function` de méthode, `fns[]` et aides `method()` compris) ; D.10c ne garde que les rappels des TU (9 lignes : `fq` 3, `js` 4, `mh` 1, `pj` 1, plus ce que D.10b y laisse) et perd ses lignes `pico8_mv`. D.10b hérite des 19 lignes d'adaptateurs de rappel (dont `pico8_hp_lq_dispatch`, `pico8_jo_fe_my_dispatch`) et des membres `my`/`hb`/`lh`/`lq`, appelés directement depuis D.10a ; le crochet `has_hb` survit au typage de `js.hb` ; le commentaire de `pico8_value_function_t` (`pico8_value.h`, « appelé par dc()/mv() ») est à réécrire avec lui. Sites G laissés : `ui_button.c:599` (D.10b), `slipway.c:261` et `research_icon.c:182` (D.11a), `ke.c:43` (D.11c) ; pris à D.11b (classe C) : la présence des noms `bm` dans `lt`/`he` et de `gv` dans `jc:hq`. `lt`/`he` assertent eux-mêmes le tag de struct (l'assert passait par `pico8_entity_get_field`). TROU-11 sans site. `dispatch` : `src/` 35 lignes / 37 occurrences / 15 fichiers, `tests/` 9 ; `par-nom` 986 (`src/` 419 au lieu de 420 : le commentaire de la déclaration de `pico8_mv` nommait aussi `pico8_entity_get_field`) ; `valeur` 4272 / 4639, `type-tag` 347 / 551, `ref-audit` 55 / 56 ; résidu D 5137 lignes dans 126 fichiers. Balayage ASan+UBSan des TU propre, `class-coverage-net` identique aux deux commits.
- D.10b — `dispatch`, rappels : `js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, rappels des rangées de `menu` (`g_ix_my`, `g_ix_hb`, `ha`), `jo:fe`, `de.my` (`pico8_de_my`), `eu` de `help`, `init` de `score_panel` → pointeurs de fonction à la signature de leur unique appelant ; leurs lecteurs (`msgbox.c`, `research.c`, `ui_button.c`) ne les rendent plus comme valeur ; `PICO8_VALUE_FUNCTION`, `pico8_value_function()` et `pico8_value_function_t` retirés de `pico8_value.h/.c`. Empreinte 18 (18, 0) aujourd'hui. Compte : `dispatch` `src/` = 0, pont pour les TU excepté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non. Découvert à D.10b : deux chunks (b1 : 25 chemins ; b2 : 21). Rappels typés à la signature de leur appelant : `pico8_js_fn` (`void (*)(pico8_js_t *)` : `js.my`, `js.hb`, rangées de `menu`, `jo:fe`, `eu` — installé par `pico8_eu` lui-même —, `init`), `pico8_fq_my_fn` et `pico8_mh_lh_fn` (`int`, vérité du retour) ; `de.my` en deux variantes (`pico8_de_my()` rend celle de `fq(de)`) ; `hp.lq` sans adaptateur ; les lecteurs par nom ne rendent plus de rappel (`pico8_js_my` supprimé) ; `PICO8_VALUE_FUNCTION`, `pico8_value_function()`, `pico8_value_function_t` supprimés. Pas de pont : les 9 lignes de rappel des TU ont suivi, **D.10c n'a plus d'objet** (ses lignes `pico8_mv` sont parties à D.10a, ses lignes `pico8_value_function` à D.10a et D.10b) ; `dispatch` = 0, dans `DATA_RESIDUE_GUARD`. D.11a : les lectures par nom de `_cr_t`, de `lq` (`ix[2].hb`) et de `_jo_fe_*` se font sur un `self` déjà typé `pico8_js_t *`. Q12 : `pico8_js_my()` (nil, `false` ou fonction) n'existe plus ; le genre mêlé de `js.my` ne vit que dans le membre (`has_my` + NULL). `FQ_MEMBERS_HASH` reprise par sonde (pas « `my` rend nil » retiré). `par-nom` 986 (`src/` 418) ; `valeur` 4067 / 4364 ; résidu D 4933 lignes dans 124 fichiers.
- D.10c — `dispatch`, TU : 98 lignes `pico8_mv` → appels typés, 26 lignes `pico8_value_function` → rappels typés ; le pont éventuel de D.10a/D.10b supprimé. Empreinte 31 (0, 31). Compte : `dispatch` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change hors pont. Découvert à D.10c : vide, constatée sans commit de code — tout est parti à D.10a et D.10b, sans pont.
- D.11a — `par-nom`, receveurs de type connu et singletons : classe A (41 lignes, 36 appels d'aides ; cast vérifié vers la famille que le cart fixe), classe D (8, 21 ; `pico8_fm_hy()`, `pico8_je_nf()`, `pico8_mo_qb()`, NULL sur le remplaçant, Q8), E `je.l`/`je.r` (`pico8_ui_je_button_table`), et les aides H (39) devenues sans appelant. Empreinte ≈ 22 (22, 0). Compte : `par-nom` ≈ 910. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non. Découvert à D.11a : deux chunks (a1 : 19 chemins ; a2 : 29). Singletons (Q8) : `pico8_fm_hy()` rend NULL sur le `fm` provisoire (son assert part, les méthodes de `hy` prennent NULL ; le cas de mort `fm_hy_on_provisional` de `test_pico8_hy`, que le PRD n'avait pas vu, devient `pico8_fm_hy() == NULL`), `pico8_fm_placeholder()` le rend typé pour `jk`/`nz`, `pico8_je_nf()`, `pico8_mo_qb()` (une écriture sur le `mo` provisoire arrête toujours), `pico8_nf_l()`/`pico8_nf_r()` pour `je[bt]`. Receveurs connus : cast vérifié par `pico8_jo_is_family` ou une classe exacte (`jc`, `trade`, `qb`) ; `pico8_lt` prend `pico8_ke_t *` (porte de `hv`, `me`/`t` de `dg` lus typés ; `giver_t` de `test_pico8_ui` porte un `ke`) ; extrémités de voie par `pico8_jc_end_p`/`pico8_jc_end_ra` (planète ou point de souris ; `jc:ch` lit aussi `id` sur le point de souris : absent). G : `ie.gy` en `const pico8_vec_t *` (`pico8_ie_eh`/`pico8_ie_set_gy` typés, `ie_take_vec` supprimée, quatre cas `hb_from_gy` retirés). **`jc.mt` reste par nom** : H2 est fausse pour lui (`slipway.c` est dans `DATA_ENGINE_SRCS`, `planet_family_trade.c` non, et `pico8_fr_struct_class` casse le lien de `test_pico8_bg_kj`) ; `jc:hq` garde `pico8_jc_entity_value` et `jc_take_fixed` (3 lignes `par-nom`), que D.11b prend comme champ transversal (Q6, accesseur de la table de classe) ; la comparaison des `mt` de `jc:ce` est typée. **Reclassés en C (D.11b)** : `mz_pq_ox_predicate` (`fx` sur `hv`), `pico8_ij` (`p`) et `pico8_mzd_ra_of_ij` (`ra`), parce que `mz.ox` tient des planètes et des `g_`/`fv` (`fx=1`). Aides H restantes (appelant B/C/F/M, ou `mt`) : `field_truthy` d'`economy_score`/`economy_tiers`/`planet_display`, `field_fixed` de `planet_display`, `pico8_entity_geom_field_*`, `pico8_msgbox_field_vec`, `pico8_probe_field_fixed`/`_truthy`, `pico8_ui_field_raw`, `pico8_mzd_field_vec`, `pico8_jc_entity_value`/`jc_take_fixed`. Q12 : `pico8_ie_eh`/`pico8_ie_set_gy` quittent la liste ; `pico8_jc_hg_add`/`_has`, `pico8_trade_qe`, `pico8_jo_hf` reçoivent une valeur reconstruite du membre à leur appel. `par-nom` 913 (`src/` 345 : départ − 73, pas − 76, les 3 lignes de `mt`) ; `valeur` 3906 / 4202 ; résidu D 4736 lignes dans 124 fichiers.
- D.11b — `par-nom`, lectures sans receveur typé : B (7 + 1 ; test d'appartenance par le lien parent, `pico8_qh_of(jo)`, Q5), (C passe en tête de D.11e : ses descripteurs sont ceux que D.11e rouvre, revue du plan après D.11a), F (5 + 2 ; struct locale de cadre `{ep, eo, hi, ff, ca, gq, ra}` remplie par les cinq appelants de `pz`), K (4 ; enregistrement typé par sorte `{rr, has_s, s, me, has_od, od}`, résolu le long de la chaîne, pointé par `pico8_entity_kind_t`). Empreinte ≈ 15 (15, 0). Compte : `par-nom` ≈ 882. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non. Découvert à D.11b : deux chunks (b1 : 15 chemins ; b2 : 7). B (Q5) : `pico8_struct_class_t.parent` (9 filles de `jo`, `fv`) est un pointeur vers l'accesseur de la classe parente (`const pico8_struct_class_t *(*parent)(void)`), pas vers ses données : les descripteurs parents sont `static` dans d'autres unités et un initialiseur statique veut une constante d'adresse ; `pico8_qh_of()`/`pico8_nl_of()` remontent ce lien (NULL hors lignée) ; `pico8_jo_is_family` (test par `states[LL]`) pourrait s'écrire sur `parent`, non fait. La branche « `hp` chaîne » de `jo:pq` était inexprimable, retirée (`pico8_gb_find` sans appelant de `src/`, gardée pour ses TU). K : `pico8_entity_kind_t.rec` → `pico8_kind_rec_t` `{rr, has_s, s, me, has_od, od}`, généré dans `pico8_game_data.c` pour les 9 sortes (pas 11), mêmes chaînes que `pico8_X_start` (s/od en mêmes littéraux) ; la garde « prêt » du `field` des sortes n'est pas dans l'enregistrement ; l'oracle de transition `test_kind_rec_matches_the_reader` (`test_pico8_entity_kind`, par l'aide `kind_field` existante, aucune ligne `par-nom` ajoutée) est à retirer par D.11d/D.11e, après quoi D.11e peut supprimer le `field` des sortes. F : `pico8_pz(const pico8_pz_frame_t *, …)`, cadre rempli par `fv:pq`, `cr`, `ix[4].my`, `jo:gh`, `jo:fe` (aide `jo_pz_frame`) ; vues `pj_cr`/`ha_pz` laissées à D.11e ; le `ra` du cadre de `fv` n'est jamais lu (`fv.hi` toujours posé) ; le cas « une liste arrête `pz` » de `test_pz_closed` est retiré (inexprimable). `parent` est disponible pour C (D.11e). `par-nom` 884 (`src/` 317, `tests/` 567 : l'oracle n'ajoute pas la ligne prévue, d'où −1 sur les 885 du PRD) ; `valeur` 3876 / 4170 ; `type-tag` 343 / 544 ; résidu D 4681 lignes dans 124 fichiers.
- D.11c — `par-nom`, classe M (Q7) : émetteur de particules (`ei`, `ez`, `f_`, `g__mm`) et sonde (`bv`, `dl`, `d_`) → composants portés par pointeur, règle 2 de la phase 2 appliquée à ces deux groupes seulement. Empreinte ≈ 15 (15, 0) : `particle`, `probe` et les 13 fichiers des classes qui les portent. Compte : `par-nom` ≈ 862. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : oui. Découvert à D.11c : deux chunks (c1 : 14 chemins ; c2 : 23). Composants (règle 2, Q7) : `pico8_emitter_t` `{pn, du, py, ki, pk, pk_is_bool, fc, k_, qo, ro, qg, q_, qy}` porté par `g_`, `hj`, `ga` ; `pico8_probe_t` `{et, has_et, dw, od, has_od, dq, fp, fp_is_bool, ik, has_ik}` porté par `g_`, `qh`, `mz`, `jc` ; pointeur vers un stockage dans l'hôte (`emitter_own`/`probe_own`), aucune allocation ; crochets `emitter_of`/`probe_of` de `pico8_struct_class_t` (6 et 12 descripteurs), l'enchâssement inchangé. Champs d'hôte (`ra`, `p`, `p_`, `fa`) et de cible (`fo`, `qs`) par cast vérifié sur l'ensemble fermé (`pico8_g__of`, `pico8_jo_is_family`, classes exactes), `me`/`t` par `ke`. `pico8_dl_show` pour la vue de l'icône ; `pico8_ke_ks(self, const char *)` (G `ke.c:43`) ; `ei`/`f_` s'arrêtent sur un non-émetteur. `pico8_jc_too_far_t` dans `slipway.h`. Trois TU lisaient les défauts déplacés en membres et suivent (`test_pico8_spec_defaults`, `test_pico8_jo_spec_defaults`, `test_pico8_g_`), hors de l'empreinte du PRD ; l'écriture de `p_` par `d_` sur un `mz` est aussitôt écrasée par `mz:ey` (mutation équivalente), et le numéro de sfx de `ik` n'est observé par aucun TU (audio hors périmètre). Pour D.11e : `get_dw` n'a plus d'objet (ses 12 descripteurs sont les porteurs de `probe_of`), `set_fo` peut réutiliser l'écriture typée de `bv` (planète, `jc`, `bv_et`) en y ajoutant `jc_too_far`. Pour D.12b : `qy` est `emitter->qy`. `par-nom` 859 (`src/` 292) ; `valeur` 3815 ; résidu D 4605.
- D.11d — `par-nom`, TU : T1 (mise en état et lecture d'état) en accès aux membres typés ; T2 (tests des descripteurs, tests de mort des `assert` d'écrivain) supprimés comme tests de la couche (Q11, §6). Deux chunks d'environ 19 fichiers. Empreinte 38 (0, 38). Compte : `par-nom` ≈ 286 (l'infrastructure de `src/`). §6 : `test_pico8_planet_gh_row_lq` et `test_pico8_ga_export_panel_by` (3 lignes chacun) lisent les membres, preuve rouge. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change. Décidé après la PRD (humain, 2026-09-29) : ≈ 1 555 appels d'aides par nom à retyper, l'étape passe en quatre sous-étapes d'environ 50 min, même PRD (section « Découpage alternatif »), chacune vérifiée et poussée : D.11d1 (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr` ; `tests/` → 473), D.11d2 (`ga`, `hp`, `hy`, `trade`, `lq`, `entity_kind`, `map_tables`, `ie`, `ga_export_panel_by` ; → 369), D.11d3 (`ui`, `g_`, `fv`, `mz`, `bg`, `fq`, `mh`, `iv`, `planet_gh_row_lq` ; → 162), D.11d4 (`js`, `jc`, `pj`, `qb`, `nf`, `fm`, `je`, `mo`, `probe_mm_rnd_order`, `jb_ol`, `spec_defaults`, note du plan ; → 5). Découvert à D.11d1 : `par-nom` `tests/` 567 → 473 lignes / 622 → 499 occ. (29 fichiers, `src/` 292 inchangé, total 765), `valeur` 3815 → 3523 ; 94 lignes : (a1) 18, (a2) 76, (a3) 0, (b) = (c) = 0 ; constantes gardées toutes égales, plis de forme déclarés (5 à 21 bits par pli, 9 TU) ; 4 tests supprimés (`nl` : `test_nl_members_by_name` et `test_nl_writer_stops_off_row`, dont la seule cible était `write_qw_value`, T2-M, absent de la liste des 47 ; `fr` : `test_nr_members_by_name`, `test_fr_mt_by_name`), donc 48 pour l'étape ; `gcov` : 233 lignes de `src/` perdues, 196 d'infrastructure par nom, 4 couvertes par `test_pico8_value`, 33 par les e2e, aucune assertion ajoutée ; M2 (`hj` sans `k_`) ne rougit que `hj` : `pm`/`mx` reposent leur `k_` (`planet_family.c:673`, `planet_family_trade.c:450`), mordus par M2b/M2c ; hérité par D.11d2 : `bought`/`icon_of` lisent `ie->ke` après un test de classe, `credits()` lit `pico8_fm_hy()->nz`, `pico8_mo_qb()->ra` remplace la lecture de `mo`, même recette pour les TU suivants. Découvert à D.11d2 : `par-nom` `tests/` 473 → **371** lignes / 499 → 389 occ. (21 fichiers ; `src/` 292 inchangé ; total 663), pas 369 : `ie.gy` et `ie.id` sont l'état partagé de la classe (règle « Archétype ou état »), sans membre et sans lecteur typé dans `src/` ; `test_pico8_ie` les lit encore par nom dans deux aides sans paramètre de nom (`shared_gy`, `shared_id`, 2 lignes `pico8_entity_get_field`), leurs constantes pliant `gy`/`id` restent égales ; D.11e doit leur donner un lecteur typé (`pico8_ie_gy()`/`pico8_ie_id()`, absent de son PRD) avant de retirer `pico8_ie_field`, et le compte cible des pas suivants se décale de +2 (D.11d3 → 164, D.11d4 → 7) sauf rattrapage. 104 lignes retirées et 2 ajoutées (`ie`) : (a1) 15, (a2) 84, (a3) 5, (b) = (c) = 0 ; `valeur` 3523 → 3182, `type-tag` 343 → 332. 11 tests supprimés, comme prévu. `test_kind_reader_serves_the_generated_defaults` compare `rec` à `pico8_X_start` (31 lignes, M4b le prouve : `pico8_nr_start.s` changé seul, rouge à cette comparaison, `KIND_HASH` restant égal). Plis de forme : mots de genre (`PICO8_VALUE_*` pliés comme énumérateurs, lignes gagnant `valeur`, déclarées) dans `entity_kind`, `hp`, `hy`, `lq` ; 13 bits de présence par `ga` (BUILT_HASH) ; octet de genre de `t`/`me`/`fa` dans `ie`. `gcov` : 621 lignes de `src/` perdues : 487 d'infrastructure par nom (lecteurs `field` et `kind_field`, écrivains, `*_take_*`, `pico8_hy_hl`, `pico8_js_h_`, `pico8_lq_bg`, `value_is_nil`), 2 par un autre TU (`pico8_je`, `ui.c:212-213`, par `test_pico8_je`/`fm`/`js`), 131 par les e2e ; `collect.c:74` (`pico8_oj` sur le `fm` provisoire, que seul un test supprimé atteignait) reçoit une assertion dans `test_functions` de `test_pico8_hp`. M3 rougit `test_pico8_trade` par la partie (aucune route ne se forme), M4 par `KIND_HASH` avant la comparaison, d'où M4b ; R2, R3, R4, M5 comme prévu. Découvert à D.11d3 : `par-nom` `tests/` 371 → **166** lignes / 389 → 176 occ. (16 fichiers ; `src/` 292 inchangé ; total 458), pas 164 : le `fo` de la cible jetable `bv_et` (`g_bv_et`, struct privée de `pico8_probe.c`, sans lecteur typé) n'est lisible que par nom ; `test_pico8_g_` et `test_pico8_mz` le lisent dans une aide sans paramètre de nom (`bv_et_fo`, 1 ligne `pico8_entity_get_field` chacun), D.11e doit rendre la struct publique ou lui donner un accesseur avant de retirer `pico8_bv_et_field` (absent de son PRD), et la cible de D.11d4 passe à 9 sauf rattrapage. 207 lignes retirées (le PRD : 207 / 215), 2 ajoutées : (a1) 46, (a2) 161, (a3) 0, (b) = (c) = 0 ; `valeur` 3182 → 2295, `type-tag` 332 → 308. Tests supprimés : les 20 du PRD pour ces TU, plus 12 cas de mort d'écrivain par nom que l'inventaire n'avait pas listés (`test_fq_writer_stops_off_row`, `test_bg_writer_stops_off_row`, `test_fv_writer_stops_off_row`, `test_g__writer_stops_off_row`, `test_mz_writer_stops_off_row`, et dans `test_pico8_ui` `test_jo_writer_stops_off_row`, `test_jo_position_stops_off_row`, `test_jo_state_stops_off_row`, `test_jo_writers_stop_off_row`, `test_qh_writer_stops_off_row`, `test_hj_writer_stops_off_row`, `test_mx_writer_stops_off_row` : seule cible `write_off_row`/`write_m_by_name`, T2-M), donc 32 pour le pas ; `mz:812` (classé T1-W au PRD) est un cas de mort de l'écrivain générique sur `pico8_mz_jj_t`, qui n'a pas de membre `fo` : supprimé, (a2). Constantes gardées toutes égales ; plis de forme (mot de genre, bit de présence) dans `mh`, `fq`, `fv`, `g_`, `ui` (`PICO8_VALUE_STRING`/`FIXED` pliés par leur nom dans `me_of`/`fold_t`, six mots d'intrus par hachage `KE_ME`/`KE_T`) ; aucun dans `mz`, `bg`, `iv`. `gcov` : 992 lignes de `src/` perdues, 573 d'infrastructure par nom (lecteurs, écrivains, `*_take_*`, et `family_is_target_class`, `pico8_hj_pk`, `pico8_qh_et`, `pico8_g__et`, `mz_is_et_class`, dont le seul appelant est un lecteur ou un `*_take_*`), 406 par un autre TU, 8 par les e2e, 5 lignes sans instruction (accolade, blanc) ; `wormhole.c:243` (`pico8_mz_et` sans `et`) et `wormhole.c:47` (`fx` faux lu par le site C du prédicat de `ox`) reçoivent une assertion dans `check_fresh_mz` et `test_ox` de `test_pico8_mz` (constantes inchangées). R1 (ASan), R3, R4, R5, M6, M7 (`ui`, `bg`, `fq`, `fv`), M9 rougissent comme prévu. Découvert à D.11d4 (clôt D.11d) : `par-nom` `tests/` 166 → **11** lignes / 176 → 11 occ. (16 → 7 fichiers ; `src/` 292 / 352 inchangé ; total 303, 38 fichiers), pas 9 : le point souris que `jc:hq` pend sur `re` (`pico8_jc_mouse_point_t`, privé à `pico8_slipway.c:82`) n'a aucun lecteur typé de ses pointeurs `ra`/`p` (`pico8_jc_end_p`/`_ra` rendent des valeurs) ; `test_pico8_jc` les lit dans deux aides sans paramètre de nom (`mouse_point_ra`, `mouse_point_p`, classe assertée), dont dépendent `MOUSE_POINT_HASH`, `TOO_FAR_HASH`, `HQ_HASH` et les identités de `test_hq_mouse_point`. 157 lignes retirées / 167 occ. (le PRD : 157 / 167), 2 ajoutées : (a1) 31, (a2) 126, (a3) 0, (b) = (c) = 0 ; `je:72`, `mo:67` (T2-N à l'annexe C) sont typés comme `qb:323-325` (`pico8_je_nf() == NULL`, `pico8_mo_qb() == NULL`). `valeur` 2295 → 1833, `type-tag` 308 → 293. Tests supprimés : 12 (`js` 1, `jc` 4, `pj` 2, `qb` 2, `nf` 1, `fm` 1, `mo` 1) ; `test_js_writer_stops_off_row` et `test_jc_writer_stops_off_row` perdent leurs cas d'écrivain par nom et gardent leurs cas de construction (asserts de `pico8_js_new`/`pico8_jc_new`, clés typées), renommés `test_js_new_stops_off_row`/`test_jc_new_stops_off_qu`. Sur l'étape : 59 tests supprimés (d1 4, d2 11, d3 32, d4 12). Constantes gardées toutes égales (dont `JS_MEMBERS_HASH`, plié membre par membre) ; parties : `MEMBERS_HASH`, `FAR_MEMBERS_HASH`, `JC_MEMBERS_HASH`, `PJ_MEMBERS_HASH`, `QB_MEMBERS_HASH`, `NF_MEMBERS_HASH`. Plis de forme dans `js` (bit de présence et mot de genre des membres, 23 réponses 0 de l'écrivain brut, 10 intrus), `pj`, `jc` ; aucun ailleurs. `gcov` : 675 lignes de `src/` perdues, 322 d'infrastructure par nom (lecteurs, écrivains, `*_take_*`, et `pico8_g__et`, `pico8_pj_ed`, `pico8_js_h_`, `jc_is_fo_holder_class`, dont le seul appelant est un lecteur ou un `*_take_*`), 353 par un autre TU, 0 par les e2e ; `scroll.c:21` (`ph` sur un `v` nil) reçoit une assertion dans `test_qb_is_a_struct`. Couverture perdue sans reprise : les valeurs `hi`/`gq`/`ca` de la vue `ha_pz` (lecteur de struct-vue, T2 au PRD). M8, M10, M11 (et M11b sur `bu`) rougissent. **Pour D.11e** — lignes `par-nom` restantes dans `tests/` : T3 `test_pico8_iv.c:200`, `:205` (`fake_toast_field`, `bootstrap.c:87`), `test_pico8_mo.c:95` (`g_ra_only_class`, `entity_geom.c:49`), `test_pico8_ui.c:717`, `:718` (`giver_field`/`giver_set_field`, `ui.c:346`/`:359`/`:363`) ; exceptions `test_pico8_ie.c:95`, `:103` (`shared_gy`/`shared_id`), `test_pico8_g_.c:120` et `test_pico8_mz.c:148` (`bv_et_fo`), `test_pico8_jc.c:141`, `:151` (`mouse_point_ra`/`_p`). À exposer dans `src/` avant de retirer les lecteurs : `pico8_ie_gy()`/`pico8_ie_id()` (état partagé de `ie`), la struct `pico8_bv_et_t` ou un accesseur de son `fo`, la struct `pico8_jc_mouse_point_t` ou des accesseurs de ses pointeurs `ra`/`p` ; aucun n'est dans le PRD de D.11e. Tension §6 / ligne D.11d (tests de la couche supprimés ici, avant le code qu'ils testent) : tranchée pour la ligne, comme au PRD. La comparaison rec ↔ `pico8_X_start` reste portée par `test_kind_reader_serves_the_generated_defaults` (D.11d2).
- D.11e — `par-nom`, suppression de l'infrastructure. Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`, laissé à D.11a ; accesseur typé par champ dans la table de classe, NULL = absent, Q6 ; l'oracle accesseurs = lecteurs naît et meurt dans l'étape). Découvert à D.11b (pour D.11d/D.11e) : les littéraux `s`/`od` des `pico8_X_kind_rec` doublent ceux de `pico8_X_start` ; seul `test_kind_rec_matches_the_reader` en garde l'égalité — à sa suppression, garder une comparaison rec ↔ `pico8_X_start` ou des constantes nommées partagées. Puis : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks. Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non. Q6 reconfirmée (humain, 2026-09-29) après la revue du plan : accesseurs typés par champ, pas de remontée dans `ke`, même si la phase 2 en remplace une partie. Découpé à la PRD en trois sous-étapes d'environ 50 min, même PRD, chacune vérifiée et poussée : D.11e1 (classe C, trois faux descripteurs de TU, oracle ; `src/` 292 → 271), D.11e2 (arbre jo, sonde, voies, emplacement de sorte ; oracle retiré ; `pico8_bv_et_t` et `pico8_jc_mouse_point_t` publics ; → 87), D.11e3 (interface, `pico8_ie_gy()`/`pico8_ie_id()`, la couche elle-même, garde += `par-nom` ; → 0). Découvert à D.11e1 : `par-nom` `src/` 292/352 → 271/331 (27 fichiers), `tests/` 11 → 19 (6 exceptions + 13 lignes de l'oracle, hausse déclarée qui part en e2) ; onze accesseurs typés dans `pico8_struct_class_t` (`get_nu`, `get_fa`, `get_ra`, `get_p`, `get_kq`, `get_ql`, `get_fg`, `get_mt`, `get_fx`, `get_p_`, `set_m_`), **135** lignes d'initialiseurs dans 16 fichiers, pas 136 : la table « Qui répond » du PRD compte 21 descripteurs pour `get_ra` mais n'en énumère que 20, ensemble confirmé par relecture des lecteurs et par l'oracle ; descendants de `jo` sur `pico8_jo_get_X` (`planet.h`), `fv` sur `pico8_g__get_X` (`probe.h`). Écarts à la lettre de Q6, déclarés : `dw` par `probe_of`, `fo` par `pico8_probe_et_hang_fo` (public, branche `jc_too_far` ajoutée), `ma`/`na` par cast vérifié (dans `pico8_ma`, l'`assert` de famille passe avant la lecture ; `pico8_na` gagne `assert(self == NULL || pico8_jo_is_family(self))`) ; `get_mt` (`fr`, `jc`) lève la contrainte de lien de `jc.mt` sans toucher au `Makefile`. Oracle `test_accessors_agree_with_readers` (`test_pico8_ui`) : tout `hv` d'une partie plus une instance neuve des 24 classes construites, le point souris et `jc_too_far` de `jc:hq` sur le vide, un `jc` tenu depuis un `fr` (seul chemin qui exécute `get_mt` de `jc` présent), `lq`, `hy`, `hp`, `trade`, `ha_pz`, `pj_cr`, `mz_jj` et les trois remplaçants (après `pico8_entity_init`/`pico8_ui_init`/`pico8_probe_init`, ils valent NULL avant) ; aucun désaccord. Mutations : M11 (`get_p_` de `mz` sans `has_p_`) est équivalente (`p_` vaut 0 quand `has_p_` vaut 0 : init, écriture nil) et a été remplacée par M11b/M11c ; M6 et M8 ne rougissaient que l'oracle, qui meurt en e2 : deux assertions ajoutées (`test_pico8_mz` : une sonde entre dans `ox` ; `test_pico8_jc` : `gd` du point souris hors caméra puis dans la caméra). `get_fg` de `js` présent n'est exécuté que par les e2e. Découvert à D.11e2 : `par-nom` `src/` 271/331 → 87/111 (19 fichiers), `tests/` 19/20 → 2/2 (`test_pico8_ie`), total 290 → 89 (20 fichiers) ; 184 lignes retirées dans les 8 fichiers de e2 : 19 définitions et 13 délégations de lecteurs, 13 écrivains et 7 délégations, 28 aides `*_take_*` et 39 appels, 36 emplacements, 10 `kind_field` et leur déclaration, 9 délégations de sortes, et l'entrée `kind_field` des 9 initialiseurs de sortes (ligne gardée) ; l'emplacement `field` de `pico8_entity_kind_t` part (non compté). Oracle `test_accessors_agree_with_readers` retiré. `pico8_bv_et_t` (`probe.h`) et `pico8_jc_mouse_point_t` (`slipway.h`) publics ; `bv_et_fo` (`g_`, `mz`) et `mouse_point_ra`/`_p` (`jc`) les lisent après leur test de classe, constantes inchangées. Vues `fm`, `bv_et`, `jc_too_far`, `mz_jj`, `nr_ldown_empty` : descripteur vide `= {0}` ; `jc_mouse_point` réduit à `{get_ra, get_p}`. Orphelins supprimés : `pico8_g__et`, `pico8_qh_et`, `pico8_hj_pk` ; prédicats statiques `jo_is_fo_holder_class`, `family_is_target_class`, `jc_is_fo_holder_class`, `mz_is_et_class`. `gcov` : sans l'oracle au départ, `L` = 3 lignes de déclaration statique (`probe.c`, initialiseur de `g_fm_placeholder`), aucune ligne exécutable ; avec l'oracle, `L` ajoute 241 lignes des lecteurs par nom de e3 (que seul l'oracle appelait) et 33 lignes de 12 accesseurs que seul l'oracle exécutait (`get_mt` de `jc`, `get_p` du point souris, `get_ra`/`get_p`/`get_fa` de `mz`, `get_p` de `g_`, `get_ql` de `ui`, `get_ra` de `lq`, `nf`, `qb`, `get_p` de `qb`, `set_m_` de `ie`), recouvertes par des assertions typées ajoutées dans 8 TU (`jc`, `mz`, `g_`, `ui`, `lq`, `nf`, `qb`, `ie`). M15 (`.get_fx` de `qh` retiré) ne rougissait rien : aucun TU ne met une planète de classe `qh` à `fx` vrai devant un `mz` ; assertion ajoutée à `test_ox` (`test_pico8_mz`), M15 rougit `mz` ; M15b (`.get_ra` de `qh`) rougit `jo`. Découvert à D.11e3 (clôt D.11e) : `par-nom` `src/` 87/111 → **0** (19 fichiers → 0), `tests/` 2/2 → **0**, total 89/113 → 0 ; `DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom`, `make data-guard` vert et mordant (une ligne `par-nom` plantée dans une copie jetable rougit `make test` à `data-guard`). 89 lignes retirées : (a1) 2 (`test_pico8_ie.c:95`, `:103`), (a2) 86, (a3) 1 (`research_icon.h:19`), (b) = (c) = 0 ; 26 chemins (23 de `src/`, `test_pico8_ie`, `Makefile`, ce plan) plus `test_pico8_js` (commentaire nommant `button_take_bool`), `planet.h:22` et `planet_family_trade.c:57` (commentaires de contrat devenus faux), `glossaire-noms.md`. Couche retirée : `pico8_entity_get_field`/`set_field`, `pico8_ke_field`, typedefs `pico8_struct_field_fn`/`_set_field_fn`, emplacements `field`/`set_field` de `pico8_struct_class_t` et leur phrase de contrat, `#include "pico8_value.h"` de `pico8_struct.h` (plus rien n'y nomme `pico8_value_t`) ; lecteurs, écrivains, aides `*_take_*` et aides statiques de lecture (`value_is_nil`, `ui_ref_field`, `js_ref_field`/`_fixed_field`/`_value_field`) de `bg`, `hy`, `mo`, `lq`, `ha_pz`, `nf`, `mh`, `iv`, `hp`, `ie`, `ie_ke_view`, `pj`, `pj_cr`, `qb`, `trade`, `je`, `ui`, `js`, `fq`. `pico8_ie_gy()` (NULL = nil) et `pico8_ie_id(&out)` (0 = nil ; l'incrément de `id` est dans `ie:ldown`, pas `ie:hb`) ; `shared_gy`/`shared_id` de `test_pico8_ie` les lisent, `shared_id` rend désormais sa présence et le fixe par pointeur (`fold_shared` plie `0xF1`+fixe ou `0xDEAD` comme `fold_value`, retiré faute d'appelant), constantes inchangées. Sur l'étape : onze accesseurs (`get_nu`, `get_fa`, `get_ra`, `get_p`, `get_kq`, `get_ql`, `get_fg`, `get_mt`, `get_fx`, `get_p_`, `set_m_`), 135 lignes d'initialiseurs ; écarts à Q6 : `dw` par `probe_of`, `fo` par `pico8_probe_et_hang_fo`, `ma`/`na` par cast vérifié ; `get_mt` (`fr`, `jc`) tient la règle de lien (grep 6 : les 7 TU liés sur `DATA_ENGINE_SRCS` seule se lient), `Makefile` changé sur la seule ligne de garde ; oracle né en e1, mort en e2, aucun désaccord ; lecteurs typés pour les TU : `pico8_bv_et_t`, `pico8_jc_mouse_point_t`, `pico8_ie_gy`/`pico8_ie_id`, les six lignes d'exception retypées (`g_:120`, `mz:148`, `jc:141`, `:151`, `ie:95`, `:103`). Vues : les onze gardées, typées, sans lecteur ni écrivain ; descripteur vide `= {0}` pour `fm`, `bv_et`, `jc_too_far`, `mz_jj`, `nr_ldown_empty` (e2), `mo`, `je`, `ha_pz`, `pj_cr`, `ie_ke_view` (e3), et aussi les classes `hy`, `hp`, `trade` ; `jc_mouse_point` = `{get_ra, get_p}`. Orphelins supprimés : `pico8_g__et`, `pico8_qh_et`, `pico8_hj_pk` (e2), `pico8_hy_hl`, `pico8_js_h_` (e3) ; gardés : `pico8_lq_bg` (`test_pico8_ga`, `test_pico8_hp`), `pico8_pj_ed` (`test_pico8_spec_defaults`). `gcov` (section propre de chaque `.c`, sans les lignes d'en-tête que `gcov -t` y mêle) : `L` vide ; 14 lignes exécutées au départ supprimées (`pico8_entity_get_field`, branches `gy`/`id` de `pico8_ie_field`), remplacées par `pico8_ie_gy`/`pico8_ie_id`, exécutés sauf le `return 0` de `id` nil (jamais atteint avant comme après : `id` est un fixe dès l'init). Mutations : M16 (`.set_m_` de `js`) rougit `js`, `ui`, `fq`, `ga`, `hp`, `lq`… ; M17 (`.get_fa` de `pj`) rougit 9 TU de partie mais pas `test_pico8_pj`, qui ne lit pas `fa` par la table ; M20 (`pico8_ie_gy` NULL) rougit `ie` ; M21 (`ie:ldown` n'écrit plus `g_ie_id`) rougit `ie` et 7 TU de planète. Balayage ASan+UBSan (44 TU) propre, `class-coverage-net` identique. Greps de la « Sortie » : 1, 4 vides, 5 = 31 ; écarts : grep 2 rend `test_kind_fields_and_build` (`test_pico8_entity_kind.c:2`, `:134`, `:254`, faux positif de `kind_field`, nom gardé : un renommage pour la grep est interdit) ; grep 3 rend les 8 lignes `strcmp(name, "` de `hp_pose` (`research.c:75-101`), qui n'est pas un lecteur mais le port de `ho(gb, mr, gp)` (D.6c) : il pose sur un `hp` neuf les membres que les noms de `mr` désignent, à la construction depuis `fd` ; hors `par-nom`, laissé. Autres catégories après D.11e : `valeur` 1833 → 1321 (118 fichiers), `type-tag` 293 → 272 (62), `ref-audit` 42 (14), résidu D 2336 → 1631 (121 fichiers) ; **`liste`** (motif de Q10) 826 lignes / 870 occ. / 99 fichiers (`src/` 477/517, 60 ; `tests/` 349/353, 39), exactement H-D11e-3 (les 7 lignes de l'infrastructure parties en e2, aucune en e3). **Pour D.12** : `pico8_ij` lit `p` par `get_p` et garde `pico8_value_t *out_nearest` ; le prédicat de `mz.ox` garde `(pico8_value_t e, void *ctx)` ; `bv_et` et `jc_too_far` restent des cibles `et` ; plus aucun lecteur ne rend `ui.le`, `ui.ic`, `mz.ox` en REF ; H-D11e-6 ne tient pas à la lettre : D.11e a aussi touché `help.c`, `ke.c`/`.h`, `layout.c`, `menu.c`, `mouse_reader.c`, `planet.c`/`.h`, `planet_display.c`, `planet_family.c`/`.h`, `planet_family_trade.c`/`.h`, `research.c`, `research_icon.c`/`.h`, `scroll.c`, `struct.h`, `trade.c`, `ui_button.c`/`.h`, `wormhole_display.c` et les TU `js`, `lq`, `nf`, `qb` : ses numéros ⟲ se remesurent par contenu sur ces fichiers aussi. **Pour la phase 2** : les onze accesseurs sont les premiers que le noyau et les composants remplaceront (Q6 reconfirmée « même si la phase 2 en remplace une partie »). Relevé au vérificateur de D.11e3, pour D.12 ou la phase 2 : `hp_pose` (`research.c`, port de `ho(rc,mr,gp)` du cart, D.6c) reste un aiguillage nom → membre à la construction, hors `par-nom` ; `pico8_lq_bg` et `pico8_pj_ed` ne servent plus qu'aux TU et rendent un `pico8_value_t`.
- D.12a — `liste`, registres : catégorie `liste` ajoutée à `tools/data_residue.sh` dans un premier commit seul, motif `\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h` (Q10 ; la règle de D.0 fige les motifs existants, mais une catégorie nouvelle s'ajoute dans son propre commit avec son motif énoncé) ; conteneur de pointeurs (Q9) sous un nom hors de ce motif ; registres L1 (`hv`, `cv[5]`, `cg[*]`, calques de `dc`, `ui.ic`, `ui.le`, `g_ga_by_shared`) et leurs parcours. Empreinte ≈ 15 (15, 0) : `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud`. Compte : `liste` ≤ 927 moins L1, rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui. Découpé à la PRD en deux sous-étapes, même PRD, chacune vérifiée et poussée : D.12a1 (catégorie `liste` seule, puis conteneur `pico8_ptrs_t`, calques de `dc`, `ui.le`) et D.12a2 (`hv`, `cv`, `cg`, `ui.ic`, `mz.ox`, `ij`, et les TU qui les lisent ; repli a2p/a2s si le plafond est atteint). Découvert à D.12a1 : deux commits. (1) `6ba781e`, catégorie `liste` seule (motif de Q10, rendu à l'identique par `pattern liste`, le littéral coupé en deux chaînes shell pour tenir en 120 colonnes comme `par-nom`/`spec`) : révèle 826 lignes / 99 fichiers / 870 occ. (`src/` 477/60/517, `tests/` 349/39/353), résidu D 1631/121 → 2314/123, autres catégories inchangées. (2) conteneur `pico8_ptrs_t` (`src/pico8_ptrs.c/.h`, `void *`, `del` par identité, NULL stockable, sans tag, `test_pico8_ptrs`), calques de `dc`, `ui.le` (`g_shared_le`, `lj` par `pico8_gu_ptrs`, `cj` par `pico8_nv_ptrs`, `pz`) et les 8 TU qui lisent `le` (deux §6, façon de lire seulement) : `liste` 777/98/821 occ. (`src/` 455/60/495, `tests/` 322/38/326), `valeur` 1309/117, `type-tag` 271/62, `ref-audit` 42/14, résidu D 2256/122. Pour a2 : `test_pico8_js` lit `le` par une aide transitoire `rank_in_le` (2 appels), à fondre dans `rank_in`/`listed` convertis ; `pico8_gu` (valeur) n'a plus que `planet_family_trade.c` (`t_rl`) et n'est plus exécuté que par les e2e ; `pico8_nv` garde `layout` et `planet` ; `ui.h` garde `#include "pico8_list.h"` (`ic`). `tools/float_guard.txt` (réécrit par `make test`) passe à 59 fichiers, comme à D.2/D.8. Deux cas ajoutés à `test_pico8_ui` (clic sur une rangée de `le`, `pico8_ui_init` vide `le`) : M6 et M8 ne rougissaient aucun TU. Découvert à D.12a2 : un commit. `hv`, `cv`, `cg` (stockage, `lt`, `he`, accesseurs `const pico8_ptrs_t *`), `ui.ic`, `mz.ox`, source `cv` de `dc`, parcours de `dg` (index, TROU-5), `hy:hk`, `hy:lj`, `for_each_cg` ×3, `cz`, `bv`, `eg`, `jc:ce`, `mz:pq`, `mz:pr`, `gw`, `jz` (15 → 0) ; `pico8_jp_ptrs`, `pico8_la_ptrs` ; `pico8_ij` converti en place (`out_nearest` reste `pico8_value_t *`, Q12) ; les 31 TU lecteurs suivent (aucune constante ne change), `rank_in_le` de `test_pico8_js` fondue dans `rank_in`/`listed`. `liste` 581/86/625 occ. (`src/` 371/49/411, `tests/` 210/37/214), `valeur` 1245/115, `type-tag` 271/62, `ref-audit` 42/14, résidu D 2005/121. `entity.h`, `entity_geom.h`, `ui.h`, `wormhole.h` perdent `#include "pico8_list.h"` (aucun fichier ne s'en servait par eux). Clôture de D.12a, pour D.12b–D.12d : `g_ga_by_shared` reste une `pico8_list_t` (défaut de `jo.by`, D.12b) ; les aides à valeur gardent `layout`, `planet` (`nv`), `planet_display`, `planet_trade`, `trade` (`la`), `planet_display`, `research_display`, `tech_gate` (`jp`), `planet_family_trade` (`gu`) ; aides de TU restées à valeur (L2) : `list_size` de dix TU, `size_of` de `fv`/`g_`, `list_count` de `jc`, `value_listed` de `mx` (scindée de `listed` pour `jo.oe`) ; `cart.c` garde un include de `pico8_list.h` sans usage ; `test_pico8_ptrs` à côté de `test_pico8_list` pour D.12d. M11 (`dg` depuis l'index 1) ne rougit ni `iv` ni `mz` mais `qb`, `pm`, `ui` et 4 e2e ; M13 (`la` sans `b`) ne rougissait rien, e2e compris : cas `test_ce_crosses_a_slipway` ajouté à `test_pico8_jc`.
- D.12b — `liste`, listes membres L2 (`jo.oe`, `jo.cc`/`by`, `trade.lp`/`rl`, `mz.ox`, proposition `ib`, `qy`, `jc.ts`, `hy.invented`, `jo.jy`/`hp.i` ; `gen.oc` en tableau fixe) et aides L4 de `collect` (`jp`, `nv`, `gu`/`gu_content`, `la_append` ; prédicat `int (*)(void *, void *ctx)`). Empreinte ≈ 25 (25, 0). Compte : `liste` rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
- D.12c — `liste`, listes hétérogènes L3 et locales L4 : `lq.items`, `mh.items`, `jb.ls` → union étiquetée locale à `layout` {rien, fixe, chaîne, rangée, autre-vrai} (Q10), `lq.fs` en tableau typé de fixes ; listes locales d'affichage (`planet_display`, `menu`, `msgbox:pz`, `trade:kg`, `research*`, `score_panel*`, `tech_gate`, `slipway`, `probe`, `galaxy`) en tableaux fixes là où la borne est dans le cart. Empreinte ≈ 16 (15, 1). Compte : `liste` = 0 dans `src/` hors `pico8_list.c/.h`, `pico8_ref_audit.h` et `pico8_type_tag.h` (49 lignes aujourd'hui, à D.12d). §6 : `layout_nil_item_draws_nothing` passe son item par l'union, même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
- D.12d — `liste`, TU (431 lignes, 38 fichiers ; `test_pico8_list` réécrit sur le nouveau conteneur, il ne perd pas son objet, §6), en deux chunks ; `pico8_list.c/.h` supprimés, `PICO8_REF_AS_LIST*` et `PICO8_TYPE_TAG_LIST` retirés. Empreinte 42 (4, 38). Compte : `liste` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non.

### §5, phases 2 et 3 (l. 282-300)

### Phase 2 — Composition

La hiérarchie par enchâssement de 1a devient :

- un **noyau** `entity` commun (ce que `ke` apporte : table de méthodes, `me`, `t`, tags `nu` en masque de bits) ;
- des **archétypes** `static const` : les défauts de chaque classe, résolus une fois le long de sa chaîne (`NL_DEF = { .hp=6, .s=68, … }`) ;
- des **composants** optionnels portés par pointeur.

Règles, appliquées sans supervision :

1. **Archétype ou état** : un champ va dans l'archétype si aucun code ne l'écrit sur une instance (prouvé par grep) ; sinon dans l'état mutable de la struct. L'état partagé de classe (`ie.gy`, `ie.id`) reste statique.
2. **Composant** : un groupe de champs devient un composant s'il apparaît dans **deux branches distinctes** de l'arbre (exemple connu : `py`/`fc`/`k_`/`qo`/`ro`/`qg`/`q_`/`du` dans `hj`, `pm`, `mx` et `ga`, alors que `ga` descend de `jo`). Sinon il reste dans la struct du type.
3. Test ponctuel, supprimé en fin de phase : pour chaque ancienne classe, chaque champ résolu par sa chaîne est égal à la valeur de l'archétype correspondant (piège connu : le `nu=o()` vide de `ga` masque celui de `jo`).

- **Sortie** : aucune struct d'entité n'enchâsse plus la struct de sa parente (tout type = noyau + archétype + composants) ; chaque ancienne classe a son archétype `static const` ; le test ponctuel est passé pour chacune avant d'être supprimé.

### Phase 3 — Renommage

Un champ est renommé **seulement** si `docs/glossaire-noms.md` en donne le sens. Sinon, il garde son nom minifié. Aucun sens n'est inventé. Un commit par groupe de champs renommés ; le glossaire est mis à jour en conséquence (l'entrée pointe du nom minifié vers le nouveau nom).

### §6 (l. 302-305)

## 6. Tests existants

- Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
- Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés** par ce plan. Depuis la fusion de master (e2e rapide, #32), seuls `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` restent ; les autres ont été remplacés par les e2e de `tests/e2e/`, qui en portent la garde. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.

### §7 (l. 307-332)

## 7. Exécution

`plan-pilot`, phase après phase, un commit par classe (1a, 1b) ou par groupe (D, 2, 3).

- **Une étape `plan-pilot`** = une classe en 1a/1b, ou une préparation sans conversion nommée dans la liste (1a.16b), un groupe nommé en D, 2 et 3. La liste ordonnée des étapes d'une phase (classes et instances sans classe en 1a/1b, dans l'ordre du §5 ; groupes en D, 2 et 3) est écrite dans ce plan à l'entrée de la phase, avant son premier commit — pour 1b, après 1b.0 et avant 1b.1, à partir de la table de 1b.0 ; elle fait foi sur le §2 quand ils divergent, et chaque PRD d'étape reprend sa ligne telle quelle.
- Chaque commit de 1a passe `make class-coverage-net` (critère du PRD, hors `make test`), avec, pour chaque classe, un compte d'instanciations **égal** à celui de la porte 0.4 (valeurs versionnées à côté des exemptions à l'étape 1a.0). En 1b, seulement les étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]` ; toujours à la sortie de phase (note de sortie : une passe identique à `tools/class_coverage_net_expected.txt`).
- **ASan+UBSan, en 1b** : chaque étape ajoute un build ASan+UBSan du binaire de rejeu, passé sur `FRAMES_EVENTS` ; critère : sortie propre, sans comparaison d'empreinte. Si la durée mesurée à 1b.0 est trop longue, seulement aux étapes qui touchent une table de membres ou une boucle d'init, et à la sortie de phase.
- **Filets depuis la fusion de master (e2e rapide)** : `FRAMES_EVENTS` n'existe plus ; `make e2e` rejoue les 26 e2e de `tests/e2e/` depuis l'écran titre, `make sanitize-net` et `make class-coverage-net` rejouent ce même jeu, `tools/class_coverage_net_expected.txt` y a été reposé à la fusion ; aucun TU ne rejoue de fichier d'événements : un TU construit son état directement, un rejeu est un e2e de `tests/e2e/` (moins de 180 frames, et il doit ajouter de la couverture de lignes). Les mentions de `FRAMES_EVENTS`, `frames-check` et `float-audit` dans les étapes livrées sont historiques.
- **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.
- **Preuve des tests, à partir de 1a.23** : `gcov` (test de l'étape compilé en `--coverage` dans une copie jetable du scratchpad) montre que chaque ligne ajoutée ou modifiée par la conversion est exécutée par la caractérisation ; s'y ajoutent **2 à 3 mutations** ciblées sur ce que `gcov` ne prouve pas (valeur d'un défaut propre, entrée du descripteur remplacée, ordre du constructeur). Le vérificateur relit le diff et le rapport `gcov`, sans banc de mutations propre. Une ligne non exécutée est soit couverte par un test ajouté, soit justifiée dans le message de commit. En 1b, les mutations comprennent au moins : (1) indicateur de présence forcé vrai sur un membre à nil observé ; (2) lecture emballée d'un fixe typé qui rend « présent 0 » là où il était absent ; (3) quand ça s'applique, boucle d'init qui saute un membre typé. Chacune rougit un test, sinon le trou est justifié dans le message de commit.
- **Note « Découvert à 1a.N », à partir de 1a.27** : la ligne du plan ne garde que ce qui change une étape suivante — entrée d'enum ajoutée, fonction laissée à double forme **avec l'étape qui la fermera**, lecture nouvelle d'une règle du §5 ; le reste (sites, comptes, mesures) va dans le message de commit.
- **Nommage des PRD** : `plan3-phase<X>-etape<M>-*.md` (X ∈ `0`, `1a`, `1b`, `D`, `2`, `3`). « La plus haute » s'entend dans l'ordre du plan — `0` < `1a` < `1b` < `D` < `2` < `3`, puis `M` numérique, puis la lettre (`16` < `16b`, `42a` < `42b`) —, jamais dans l'ordre texte des noms. Seuls ces fichiers de `docs/prd/done/` comptent pour trouver l'étape suivante — les `phaseD-*`/`phaseE-*` de Plan 2 n'en sont pas. Les étapes 0.1 et 0.2 sont livrées, 0.3 est humaine : l'autopilote démarre à la porte 0.4, puis au premier commit de 1a.

L'autopilote **s'arrête et demande à l'humain** quand :

- le filet reste rouge après **3 tentatives** de correction sur le même commit ;
- un champ ne tombe sous aucune règle (§5 : clé dynamique à ensemble non fini ; en phase 2, champ ni archétype ni état, ou groupe au classement ambigu) ;
- un test de régression de bug devrait être modifié **dans son intention** (pas seulement dans sa façon de lire l'état) pour passer ;
- la porte de l'étape 0 échoue ;
- le diff modifie ou supprime un `.sha256` du filet, ou retire un e2e de `tests/e2e/` — `make test` resterait vert.

Un site ou une instance sans classe découvert en cours d'étape et **couvert par une règle existante du §5** n'est pas un arrêt : l'autopilote l'ajoute à la liste des étapes dans le commit qui le traite, avec une ligne « découvert à 1a.M » dans le message de commit. Seul un site qu'aucune règle ne couvre déclenche l'arrêt.

Un écart port/cart découvert en cours d'étape n'est pas un arrêt : il est **préservé à l'identique**, décrit par une nouvelle entrée `TROU-N` de `docs/trous-de-portage.md` (source, atteinte par le filet) et référencé par `/* TROU-N */` au site, dans le commit de l'étape. Le corriger est interdit dans ce plan ; si le préserver exige un champ qu'aucune règle du §5 ne couvre, c'est l'arrêt « champ sans règle ».

Chacune de ces conditions est écrite dans le PRD de l'étape concernée comme critère que le vérificateur de `prd-autopilot` traduit en `FAILED:` — c'est le seul arrêt que `plan-pilot` honore.

## Annexe B — étude D.9, `pico8_list_t` (`docs/prd/done/plan3-phaseD-etape9-etude.md`, inchangée à `6cf4df4`)

### §3.1 et §3.2 (l. 120-137)

## 3. `pico8_list_t`

### 3.1 Inventaire (motif proposé en Q10 : 574 lignes / 63 fichiers dans `src/`, 413 / 38 dans `tests/`)

| Classe | Listes | Éléments | Forme |
|---|---|---|---|
| **L1** — registres d'entités | `hv`, `cv[5]`, `cg[*]`, calques de `dc` (15), `ui.ic` (16), `ui.le` (partagée, puis tas), `g_ga_by_shared` | REF vers un en-tête d'entité | liste de pointeurs (Q9) ; `del` par identité, tolérance de `all()` (TROU-5) inchangées |
| **L2** — listes membres des entités | `jo.oe` (`jc`), `jo.cc`/`by` (`trade`), `trade.lp` (`jo`)/`rl` (`jc`), `mz.ox`, proposition `ib`, `qy` (particules de `g_`/`hj`/`pm`), `jc.ts` (`pico8_ts_t`, D.7a), `hy.invented` (chaînes `px`), `jo.jy`/`hp.i` (`pico8_jy_t`, D.7a), `gen.oc` (entrées de `df`) | REF homogène, ou chaîne pour `invented` | liste de pointeurs, élément typé à la lecture (le cast `.as.ref` d'aujourd'hui devient un cast de `void *`) ; tableau fixe quand la borne est connue (`gen.oc`) |
| **L3** — listes hétérogènes | `lq.items`, `mh.items`, `jb.ls` ; `lq.fs` (largeurs) | `lq.items` : chaîne, fixe, nil/faux, tuple REF (après D.7a une `pico8_jb_row_t`) ; une fonction ou un autre REF vrai compte comme le sprite 0 (`lq:mn`) | **union étiquetée locale** à `layout` : {rien, fixe, chaîne, rangée, autre-vrai} (Q10) ; `lq.fs` : tableau typé de fixes |
| **L4** — temporaires et aides génériques | `jp` (filtre), `nv` (map), `gu`/`gu_content`, `la_append` (`collect.c`) ; une quarantaine de listes locales (`planet_display` `bs`/`ji`/`jx`/`prefix`…, `menu`, `msgbox:pz`, `trade:kg`, `research`, `research_display`, `score_panel*`, `tech_gate`, `slipway`, `probe`, `galaxy`) | REF, chaînes | aides typées sur la liste de pointeurs (prédicat `int (*)(void *, void *ctx)`) ; locales bornées en tableaux fixes quand la borne est dans le cart |

`PICO8_REF_AS_LIST*` (`ref-audit`) et `PICO8_TYPE_TAG_LIST` (`type-tag`) tombent avec L1-L3. `test_pico8_list` n'est pas un test qui perd son objet : les sémantiques `del`/`all` de PICO-8 restent. Il est réécrit sur le nouveau conteneur (§6, premier point : il ne perd pas son objet).

### 3.2 Ce qui tombe déjà avec D.7a–D.8

- Les aides `*_positional_values` (`help`, `layout`, `menu`, `research_display`, `score_panel`, `score_panel_display`, `tech_gate`) et `ix_items` de `cart:eu` lisent des tables ob : 46 lignes du motif, en comptant `pico8_fi`, `ft_index`/`ft_name`.
- Le reste de `pico8_ob.c/.h` (D.8), et 5 lignes du test ponctuel (D.8).
- **Au total ≈ 55 lignes.** D.7a ne change que le pointé de `lq.items`, `mh.items`, `jy`, `hp.i` et `ts`, et D.7b celui de `qy`. Ces listes restent L1-L3.

### §4, points 1 et 2 (l. 141-147 : `gen.nm[3]` dans l'inventaire de Q12)

## 4. Ce qui n'entre dans aucune forme

1. **Classe M, 32 lignes** (16 comptées et 16 appels d'aides, `particle.c`, `probe.c`) : l'émetteur de particules et la sonde sont du code partagé par des branches distinctes (`g_`/`fv` et l'arbre `qh`, plus `mz` pour `dl`), qui lit par nom des champs que chaque branche porte à sa place. C'est la règle 2 de la phase 2. Le §5 cite d'ailleurs ce groupe `py`/`fc`/`k_`/`qo`/`ro`/`qg`/`q_`/`du` comme « exemple connu ». Aucune forme de D ne le couvre sans anticiper cette règle, ou sans ajouter environ 25 accesseurs jetables (Q7).
2. **Les valeurs à genres mêlés, hors des trois points** : `hf` et `gg` (booléen ou fixe, TROU-3), `qh_qh`, `et` (vraie cible, struct sans champ, ou `false`), les retours de `ldown`/`gv`, `js.my` (nil, `false` ou fonction), `js.h_` (TROU-23), `pj_ed`, `trade_qe`, l'`id` de `jc_hg`, `gen.nm[3]`, `pico8_jo_hf`… On compte **62 prototypes publics à `pico8_value_t`** ; environ 20 sont des méthodes que le point 2 type.
   - Une union étiquetée locale par cas est la forme probable, mais aucune puce ne la fixe.
   - `valeur`, `type-tag` et `ref-audit` ne tombent donc pas avec D.10-D.12. C'est un quatrième point, et le prochain arrêt probable (Q12).
3. **Aucune clé dynamique à ensemble non fini.** Toutes les clés à nom variable ont un ensemble fini et s'énumèrent :

### Étapes proposées : en-tête, lignes D.10a à D.12d, règles (l. 179-204)

### Étapes proposées, avec leurs comptes

Départ supposé : après D.8, `par-nom` ≈ 1008 et `dispatch` ≤ 199. Chaque PRD remesure. « ≈ » veut dire estimé par le classement de D.9, « ≤ » un majorant, « = » exact.

| Étape | Contenu | Empreinte estimée | Compte après |
|---|---|---|---|
| **D.10a** | table de méthodes typée (23 emplacements nommés et la table d'états), 81 adaptateurs retirés, 26 appels `pico8_mv` remplacés (appel direct : `eb`, `ib`, `gg`, `md`, `lq`, `lh` ; virtuel pour le reste), `dg`/`dc`/`jz`/`lt`/`he` sur les emplacements, prédicats `*_is_*_class` sur les emplacements, `pico8_entity_get_field` ne rend plus de méthode, `hd=1` en fonction constante, sites G (9) et E `mv` (1) | ≈ 40 fichiers de `src/` (29 `dispatch`, plus les fichiers d'adaptateurs seuls) : **découper** a1 (méthodes de `dc`, `lt`, `he`, `jz`, `dg`) / a2 (le reste) | `dispatch` src ≤ 46 (76 − 30) ; `par-nom` ≈ 998 ; `valeur` en forte baisse, rapportée |
| **D.10b** | rappels typés (`js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, rappels de `menu`/`jo:fe`/`de`, `eu`, `init`) ; `PICO8_VALUE_FUNCTION` et `pico8_value_function_t` retirés de `pico8_value.h` | ≈ 15 fichiers | `dispatch` src = 0 |
| **D.10c** | TU du dispatch : 98 `pico8_mv` deviennent des appels typés, 25 rappels | 31 fichiers | `dispatch` = 0, dans `DATA_RESIDUE_GUARD` |
| **D.11a** | `par-nom` A et D (41 + 8 lignes, 57 appels d'aides), E `je.l`/`je.r`, aides H mortes (39) | ≈ 22 fichiers | `par-nom` ≈ 909 |
| **D.11b** | B (test d'appartenance, 7 + 1), C (accesseurs transversaux, 16 + 6), F (cadre de `pz`, 5 + 2), K (sortes, 4) | ≈ 14 fichiers | `par-nom` ≈ 877 |
| **D.11c** | M selon Q7 (composants émetteur et sonde) | `particle`, `probe`, plus les 13 fichiers de classes qui les portent | `par-nom` ≈ 861 |
| **D.11d** | TU de `par-nom` : T1 en accès aux membres, T2 supprimés (Q11), deux §6 réécrits avec leur preuve rouge | 38 fichiers : **découper** en deux (≈ 19 chacun) | `par-nom` ≈ 286 |
| **D.11e** | suppression de l'infrastructure : 40 lecteurs, 18 écrivains, 10 `kind_field` et les emplacements des 11 sortes, 55 aides `*_take_*` (43 après D.7a), `pico8_entity_get_field`/`set_field`, typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les 11 structs-vues deviennent typées ou disparaissent : remplaçants `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view` | ≈ 35 fichiers de `src/` | `par-nom` = 0, dans `DATA_RESIDUE_GUARD` |
| **D.12a** | catégorie `liste` ajoutée (Q10) ; conteneur de pointeurs (Q9) ; registres L1 et leurs parcours | `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud` (≈ 15) | `liste` ≤ 987 − 55 (D.7a-D.8) − L1, rapporté |
| **D.12b** | listes membres L2 et aides L4 de `collect` | ≈ 25 fichiers | rapporté |
| **D.12c** | listes hétérogènes L3 (union locale) et locales d'affichage L4 | ≈ 15 fichiers | `liste` src = 0 |
| **D.12d** | TU (413 lignes, 38 fichiers ; `test_pico8_list` réécrit) : **découper** en deux ; `PICO8_REF_AS_LIST*`, `PICO8_TYPE_TAG_LIST` retirés | 38 | `liste` = 0, dans `DATA_RESIDUE_GUARD` |

Pour chaque étape :

- `class-coverage-net` passe si l'étape touche un constructeur : D.10a, D.11c, D.11e.
- `sanitize-net` passe à chaque étape qui change la durée de vie ou le propriétaire d'une donnée : D.11c, D.12a-c.
- §6 : les deux tests restants de la liste sont réécrits dans D.11d, et `layout_nil_item_draws_nothing` dans D.12c, qui lui fait passer son item par l'union ; chacun avec sa preuve rouge.
- Aucun `*.sha256` ni e2e ne change.


### Q9 et Q10 (l. 235-239)

**Q9. Conteneur des listes : (A) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité), plus des tableaux fixes typés là où la borne est dans le cart ; (B) un type de liste par type d'élément (macro) ; (C) garder `pico8_list_t` de `pico8_value_t` ?**
A. La tolérance de `all()` à la suppression (TROU-5) et la sémantique de `del` restent écrites une seule fois, et le cast `void *` au site remplace le cast `.as.ref` d'aujourd'hui. B multiplie ces sémantiques. C laisse survivre `pico8_value.c`, donc la Sortie de D devient impossible.

**Q10. Ajouter une catégorie `liste` au compteur (motif `\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h`, 987/101 à `2dec71f`), dans le premier commit de D.12a, et une union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ?**
Oui pour les deux. Sans catégorie, les étapes D.12 n'ont pas de « Compte », contrairement à la règle de la liste de D. L'union locale ne porte que les cinq cas que `lq:mn` distingue. Le nouveau conteneur doit prendre un nom hors du motif (règle commune : un nom nouveau ne correspond à aucun motif de la catégorie qu'il remplace).

## Annexe C — lignes `liste` des fichiers de `src/` de l'étape (remesurées par contenu à `6cf4df4`, motif de Q10)

Colonne 1 : `b1` / `b2` = retirée ou réécrite par cette sous-étape ; `inc` = `#include "pico8_list.h"` (règle des includes) ; `D12c` = liste L3/L4 ou aide à valeur qui survit, laissée à D.12c. Les lignes de registres (L1) sont parties avec D.12a. Lignes non comptées mais touchées (lectures `->items[i].as.ref`, tests de genre, `->count`) : table « Listes membres et leur stockage ».

#### `src/pico8_collect.c`

```
b2    8:int pico8_gu(const pico8_list_t *qk, pico8_value_t kb) {
b2    9:    pico8_list_iter_t it;
b2    14:    it = pico8_list_all(qk);
b2    15:    while (pico8_list_iter_next(&it, &e)) {
D12c  23:pico8_list_t pico8_jp(const pico8_list_t *qk, pico8_jp_predicate_fn ow, void *ctx) {
D12c  24:    pico8_list_t f;
D12c  25:    pico8_list_iter_t it;
D12c  27:    pico8_list_init(&f);
D12c  31:    it = pico8_list_all(qk);
D12c  32:    while (pico8_list_iter_next(&it, &e)) {
D12c  34:            pico8_list_add(&f, e);
D12c  40:pico8_list_t pico8_nv(const pico8_list_t *qk, pico8_nv_fn kp, void *ctx) {
D12c  41:    pico8_list_t m;
D12c  42:    pico8_list_iter_t it;
D12c  45:    pico8_list_init(&m);
D12c  46:    it = pico8_list_all(qk);
D12c  47:    while (pico8_list_iter_next(&it, &e)) {
D12c  50:            pico8_list_add(&m, v);
b2    105:    pico8_list_add((pico8_list_t *)ctx, value);
b2    108:pico8_list_t pico8_la(const pico8_list_t *a, const pico8_list_t *b) {
b2    109:    pico8_list_t t;
b2    111:    pico8_list_init(&t);
b2    112:    pico8_list_foreach(a, pico8_la_append, &t);
b2    113:    pico8_list_foreach(b, pico8_la_append, &t);
b1    131:int pico8_gu_content(const pico8_list_t *qk, pico8_value_t kb) {
b1    132:    pico8_list_iter_t it;
b1    137:    it = pico8_list_all(qk);
b1    138:    while (pico8_list_iter_next(&it, &e)) {
```

#### `src/pico8_collect.h`

```
inc   6:#include "pico8_list.h"
b2    13:int pico8_gu(const pico8_list_t *qk, pico8_value_t kb);
D12c  17: * caller releases with pico8_list_free(); a NULL `qk` gives an empty one. */
D12c  19:pico8_list_t pico8_jp(const pico8_list_t *qk, pico8_jp_predicate_fn ow, void *ctx);
D12c  24: * pico8_list_free(). `qk` must not be NULL. */
D12c  26:pico8_list_t pico8_nv(const pico8_list_t *qk, pico8_nv_fn kp, void *ctx);
b2    29: * caller releases with pico8_list_free(). Neither argument may be NULL. */
b2    30:pico8_list_t pico8_la(const pico8_list_t *a, const pico8_list_t *b);
b1    53:int pico8_gu_content(const pico8_list_t *qk, pico8_value_t kb);
```

#### `src/pico8_economy.c`

```
b1    25:    pico8_list_t *invented;
b1    51:    pico8_list_init(invented);
b1    292:            pico8_list_add(self->invented, pico8_value_string(t->px));
```

#### `src/pico8_economy.h`

```
b1    22:    pico8_list_t *invented;
```

#### `src/pico8_economy_score.c`

```
b2    90:    const pico8_list_t *by;
b1    199:    const pico8_list_t *invented = fm != NULL ? fm->invented : NULL;
```

#### `src/pico8_economy_formulas.c`

```
b2    102:    const pico8_list_t *jy;
```

#### `src/pico8_economy_formulas.h`

```
inc   6:#include "pico8_list.h"
```

#### `src/pico8_economy_tiers.c`

```
b2    51:    pico8_list_t *cc;
b2    59:        pico8_list_free(jo->cc);
b2    64:    pico8_list_init(cc);
b2    70:    pico8_list_t *by;
b2    71:    pico8_list_iter_t it;
b2    87:    it = pico8_list_all(by);
b2    88:    while (pico8_list_iter_next(&it, &e)) {
b2    90:        const pico8_list_t *rl;
b2    91:        const pico8_list_t *lp;
b2    145:                pico8_list_add(dest->cc, e);
b2    180:                    pico8_list_add(jo->jy, pico8_value_ref(entry));
b2    222:    const pico8_list_t *jy;
b2    223:    const pico8_list_t *cc;
b2    309:        const pico8_list_t *by = jo->by;
```

#### `src/pico8_gen.c`

```
b1    47:pico8_gn_state_t *pico8_gn(const pico8_list_t *hr, pico8_fixed_t og, pico8_gn_random_fn r, void *r_ctx) {
b1    49:    pico8_list_iter_t it;
b1    53:    pico8_list_init(&state->oc);
b1    61:    it = pico8_list_all(hr);
b1    62:    while (pico8_list_iter_next(&it, &e)) {
b1    70:            pico8_list_add(&state->oc, e);
b1    96:    pico8_list_del(&state->oc, e);
b1    109:    pico8_list_free(&state->oc);
```

#### `src/pico8_gen.h`

```
inc   6:#include "pico8_list.h"
b1    26: * native C equivalent (see pico8_jp_predicate_fn, pico8_list.h). */
b1    32:    pico8_list_t oc;
b1    45:pico8_gn_state_t *pico8_gn(const pico8_list_t *hr, pico8_fixed_t og, pico8_gn_random_fn r, void *r_ctx);
```

#### `src/pico8_galaxy.c`

```
b1    46:    pico8_list_t hr_list;
b1    102:    pico8_list_init(&hr_list);
b1    104:        pico8_list_add(&hr_list, pico8_value_ref((void *)&pico8_df_table[i]));
b1    108:    pico8_list_free(&hr_list);
```

#### `src/pico8_menu.c`

```
D12c  17:#include "pico8_list.h"
b1    139:static const pico8_jb_row_t *pico8_menu_invented_lines(const pico8_list_t *list) {
D12c  162:    pico8_list_t cb_list;
D12c  193:    pico8_list_free(&cb_list);
D12c  283:    pico8_list_t cb_list;
D12c  295:    pico8_list_free(&cb_list);
```

#### `src/pico8_particle.c`

```
b1    41:    pico8_list_t *qy;
b1    52:    pico8_list_init(qy);
b1    58:    pico8_list_t *qy;
b1    71:            pico8_list_add(qy, pico8_value_ref(self->cls->mm(self)));
b1    88:    pico8_list_t *qy;
b1    143:                pico8_list_del(qy, p_value);
```

#### `src/pico8_particle.h`

```
inc   7:#include "pico8_list.h"
b1    44:    pico8_list_t *qy;
```

#### `src/pico8_planet.c`

```
b2    287:    pico8_list_t *oe;
b2    294:    pico8_list_init(oe);
b2    322:    pico8_list_t jy;
b2    323:    pico8_list_t *jy_heap;
b2    333:    pico8_list_init(&jy);
```

#### `src/pico8_planet.h`

```
b2    44:    pico8_list_t *oe;
b2    47:    pico8_list_t *jy;
b2    59:    pico8_list_t *cc;
b2    60:    pico8_list_t *by;
```

#### `src/pico8_planet_display.c`

```
b2    57:    const pico8_list_t *jy_list;
b2    58:    const pico8_list_t *by_list;
D12c  80:    pico8_list_t *ji;
b2    99:        pico8_list_add(ctx->ji, e);
D12c  146:    pico8_list_t *ji;
b2    148:    const pico8_list_t *jy;
b2    149:    pico8_list_t jx;
D12c  190:    pico8_list_init(ji);
D12c  191:    pico8_list_add(ji, pico8_value_string("receives:"));
b2    198:        pico8_list_t prefix;
D12c  199:        pico8_list_t *merged = malloc(sizeof *merged);
b2    200:        pico8_list_init(&prefix);
b2    201:        pico8_list_add(&prefix, pico8_value_string("needs:"));
b2    203:        pico8_list_free(&prefix);
b2    215:            pico8_list_t prefix;
D12c  216:            pico8_list_t *merged = malloc(sizeof *merged);
b2    217:            pico8_list_init(&prefix);
b2    218:            pico8_list_add(&prefix, pico8_value_string("wants more:"));
b2    220:                pico8_list_t last_only;
b2    221:                pico8_list_init(&last_only);
b2    222:                pico8_list_add(&last_only, jy->items[jy->count - 1]);
b2    224:                pico8_list_free(&last_only);
b2    228:            pico8_list_free(&prefix);
b2    232:    pico8_list_free(&jx);
D12c  240:        pico8_list_free(ji);
D12c  250:            pico8_list_t *lb = malloc(sizeof *lb);
D12c  253:            pico8_list_init(lb);
D12c  257:                pico8_list_add(lb, pico8_value_string("makes:"));
D12c  258:                pico8_list_add(lb, pico8_value_string(mu_str));
b2    260:                const pico8_list_t *by = self->by;
D12c  272:                pico8_list_add(lb, pico8_value_string("exports:"));
D12c  273:                pico8_list_add(lb, pico8_value_string(combined));
D12c  275:            pico8_list_add(lb, pico8_value_ref((void *)jo_kw_tuple(kw)));
D12c  286:        pico8_list_add(PICO8_REF_AS_LIST(bs[i].lq.as.ref), pico8_value_ref((void *)&gap));
D12c  382:    pico8_list_t bj_list;
D12c  405:    pico8_list_free(&bj_list);
```

#### `src/pico8_planet_family.c`

```
b2    25:static pico8_list_t g_ga_by_shared;
b1    515:    const pico8_list_t *qy = NULL;
b2    661:    pico8_list_init(&g_ga_by_shared);
```

#### `src/pico8_planet_family_trade.c`

```
inc   10:#include "pico8_list.h"
b2    113:    const pico8_list_t *oe = NULL;
b2    114:    pico8_list_iter_t it;
b2    124:    it = pico8_list_all(oe);
b2    125:    while (pico8_list_iter_next(&it, &w_val)) {
b2    127:        const pico8_list_t *t_rl = NULL;
b2    305:        const pico8_list_t *jy;
b2    306:        pico8_list_iter_t it;
b2    312:        it = pico8_list_all(jy);
b2    313:        while (pico8_list_iter_next(&it, &i_val)) {
```

#### `src/pico8_planet_trade.c`

```
b2    19:static int gu_trade(const pico8_list_t *list, pico8_value_t v) {
b2    20:    pico8_list_iter_t it;
b2    26:    it = pico8_list_all(list);
b2    27:    while (pico8_list_iter_next(&it, &e)) {
b2    41:        pico8_list_add(self->ib, a);
b2    47:    pico8_list_t *oe = NULL;
b2    56:        pico8_list_add(oe, pico8_value_ref(sw));
b2    82:    pico8_list_t *jy = NULL;
b2    83:    pico8_list_iter_t it;
b2    106:        it = pico8_list_all(jy);
b2    107:        while (pico8_list_iter_next(&it, &e)) {
b2    115:                    pico8_list_del(jy, e);
b2    124:            pico8_list_t prefix;
b2    125:            pico8_list_t merged;
b2    126:            pico8_list_t *merged_heap;
b2    128:            pico8_list_init(&prefix);
b2    129:            pico8_list_add(&prefix, pico8_value_ref(fresh));
b2    131:            pico8_list_free(&prefix);
b2    142:    it = pico8_list_all(jy);
b2    143:    while (pico8_list_iter_next(&it, &e)) {
b2    179:pico8_list_t *pico8_jo_ib(pico8_jo_t *self) {
b2    180:    pico8_list_t *ib;
b2    183:    const pico8_list_t *oe = NULL;
b2    184:    pico8_list_iter_t it;
b2    195:        pico8_list_init(ib);
b2    208:    it = pico8_list_all(oe);
b2    209:    while (pico8_list_iter_next(&it, &w_val)) {
b2    212:        pico8_list_t *lp_list;
b2    213:        pico8_list_t *rl_list;
b2    222:        pico8_list_init(lp_list);
b2    223:        pico8_list_add(lp_list, pico8_value_ref(self));
b2    225:        pico8_list_init(rl_list);
b2    226:        pico8_list_add(rl_list, pico8_value_ref(w));
```

#### `src/pico8_planet_trade.h`

```
inc   6:#include "pico8_list.h"
b2    28:    pico8_list_t *ib;
b2    39:pico8_list_t *pico8_jo_ib(pico8_jo_t *self);
```

#### `src/pico8_research.c`

```
b2    157:    pico8_list_t *new_i_heap;
b2    230:    pico8_list_init(new_i_heap);
b2    238:            pico8_list_add(new_i_heap, pico8_value_ref(pico8_jy_new(&qe)));
b2    240:            pico8_list_add(new_i_heap, pico8_value_ref(pico8_jy_new(NULL)));
```

#### `src/pico8_research.h`

```
inc   6:#include "pico8_list.h"
b2    43:    pico8_list_t *i;
```

#### `src/pico8_research_display.c`

```
b2    32:    const pico8_list_t *i_list;
b2    37:    pico8_list_iter_t it;
b2    45:    it = pico8_list_all(i_list);
b2    46:    while (pico8_list_iter_next(&it, &entry)) {
b2    142:pico8_lq_t *pico8_jo_ex(const pico8_list_t *is, pico8_value_t o, pico8_fixed_t jq, pico8_fixed_t mj,
D12c  144:    pico8_list_t l;
D12c  157:    pico8_list_init(&l);
b2    178:        pico8_list_iter_t it = pico8_list_all(is);
b2    181:        while (pico8_list_iter_next(&it, &entry)) {
D12c  200:            pico8_list_add(&l, pico8_value_ref(&tuple->row));
D12c  211:        pico8_list_add(&l, pico8_value_ref(&t1->row));
D12c  220:        pico8_list_add(&l, pico8_value_ref(&t2->row));
D12c  228:        pico8_list_add(&l, pico8_value_string(jq_str));
D12c  229:        pico8_list_add(&l, pico8_value_ref((void *)&t3));
D12c  254:                pico8_list_add(&l, pico8_value_ref(&t4->row));
D12c  257:            pico8_list_add(&l, pico8_value_ref((void *)pico8_jo_em()));
D12c  264:    pico8_list_free(&l);
b2    270:    const pico8_list_t *is = NULL;
D12c  295:    pico8_list_add(&l->items, pico8_value_ref((void *)pico8_hp_fk()));
D12c  297:        pico8_list_add(&l->items, pico8_value_string(self->px));
D12c  350:pico8_list_t pico8_bj(const pico8_struct_header_t *jo_instance) {
D12c  351:    pico8_list_t items;
D12c  353:    pico8_list_t result;
D12c  356:    pico8_list_init(&items);
D12c  358:        pico8_list_add(&items, pico8_value_ref(pico8_gb_at(k)));
D12c  362:    pico8_list_free(&items);
```

#### `src/pico8_research_display.h`

```
D12c  7:#include "pico8_list.h"
b2    27:pico8_lq_t *pico8_jo_ex(const pico8_list_t *is, pico8_value_t o, pico8_fixed_t jq, pico8_fixed_t mj,
D12c  32: * caller owns and must release with pico8_list_free(). A `jo_instance`
D12c  34:pico8_list_t pico8_bj(const pico8_struct_header_t *jo_instance);
```

#### `src/pico8_slipway.c`

```
b1    120:    pico8_list_t *ts;
b1    129:    pico8_list_init(ts);
b2    292:        const pico8_list_t *oe = NULL;
b2    299:            pico8_list_iter_t it = pico8_list_all(oe);
b2    301:            while (pico8_list_iter_next(&it, &w_val)) {
```

#### `src/pico8_slipway.h`

```
inc   7:#include "pico8_list.h"
b1    45:    pico8_list_t *ts;
```

#### `src/pico8_slipway_display.c`

```
b1    44:                pico8_list_add(self->ts, pico8_value_ref(particle));
b1    134:            pico8_list_t *ts = self->ts;
b1    157:                        pico8_list_del(ts, t_val);
```

#### `src/pico8_tech_gate.c`

```
b1    30:    const pico8_list_t *invented_list = NULL;
D12c  115:pico8_list_t pico8_cb(pico8_value_t bi) {
D12c  116:    pico8_list_t items;
D12c  118:    pico8_list_t result;
D12c  121:    pico8_list_init(&items);
D12c  123:        pico8_list_add(&items, pico8_value_ref((void *)&pico8_nb_table[i]));
D12c  127:    pico8_list_free(&items);
```

#### `src/pico8_trade.c`

```
inc   9:#include "pico8_list.h"
b2    79:    const pico8_list_t *lp;
b2    104:    pico8_list_t empty_lp;
b2    105:    pico8_list_t empty_rl;
b2    106:    const pico8_list_t *lp;
b2    107:    const pico8_list_t *rl;
b2    108:    pico8_list_t np_list;
b2    109:    pico8_list_t jc_list;
b2    110:    pico8_list_t *new_lp;
b2    111:    pico8_list_t *new_rl;
b2    118:    pico8_list_init(&empty_lp);
b2    119:    pico8_list_init(&empty_rl);
b2    127:    pico8_list_init(&np_list);
b2    129:        pico8_list_add(&np_list, np);
b2    131:    pico8_list_init(&jc_list);
b2    133:        pico8_list_add(&jc_list, jc);
b2    142:    pico8_list_free(&np_list);
b2    143:    pico8_list_free(&jc_list);
b2    151:    const pico8_list_t *l_lp, *r_lp;
```

#### `src/pico8_trade.h`

```
inc   5:#include "pico8_list.h"
b2    13:    pico8_list_t *lp;
b2    14:    pico8_list_t *rl;
b2    38:    pico8_list_t *lp;
b2    39:    pico8_list_t *rl;
```

## Annexe D — TU (remesurés à `6cf4df4`)

Par fichier : d'abord les lignes qui nomment une liste L2, une API retypée ou une aide locale de liste (grep ; colonne 1 : sous-étape ; colonne 2 : `L` = ligne comptée `liste`, `-` = non comptée, souvent texte inchangé), puis toutes les lignes `liste` du fichier (les registres sont passés à `pico8_ptrs_t` avec D.12a). Classement automatique (motifs de noms), indicatif ; `list_size` sert `qy` (b1) et `oe`/`jy`/`by`/`cc` (b2) ; `list_count` de `jc` sert `ts` (b1) et `oe` (b2).

#### `tests/test_pico8_fr.c`
```
b2 L 142:static int list_size(const pico8_list_t *l) {
b2 - 307:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 310:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 318:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 319:    h = fnv(h, (uint32_t)(jo->by != NULL));
b2 - 348:    h = fnv(h, (uint32_t)list_size(jo->oe));
b2 - 349:    h = fnv(h, (uint32_t)list_size(jo->jy));
b2 - 373:    assert(a != b && as_qh(a)->jo.oe != as_qh(b)->jo.oe && as_qh(a)->jo.kw != as_qh(b)->jo.kw);
b2 - 400:    pico8_value_t ib = pico8_value_ref(pico8_jo_ib(e));
b2 L 410:static pico8_list_t *list_of(int n, void *item) {
b2 - 435:static uint32_t fold_route(uint32_t h, void *from) {
b2 - 496:    oe = as_qh(placed)->jo.oe;
b2 - 498:        as_qh(placed)->jo.oe = list_of(i, idle);
b2 - 507:    as_qh(placed)->jo.oe = oe;
b2 - 513:    h = fold_route(h, placed);
b2 - 517:    h = fold_route(h, slipgate);
b2 - 576:        assert(is_an_fr_struct(bare) && is_a_gate(bare) && bare->nr.qh.jo.id == NULL && bare->nr.qh.jo.oe != NULL &&
```

Toutes les lignes `liste` du fichier :

```
30:#include "../src/pico8_list.h"
142:static int list_size(const pico8_list_t *l) {
410:static pico8_list_t *list_of(int n, void *item) {
411:    pico8_list_t *l = malloc(sizeof *l);
413:    pico8_list_init(l);
415:        pico8_list_add(l, pico8_value_ref(item));
463:    pico8_list_t *oe;
```

#### `tests/test_pico8_fv.c`
```
b1 L 208:static int size_of(const pico8_list_t *list) {
b1 - 357:    h = fnv(h, (uint32_t)size_of(em->qy));
b1 - 358:    assert(size_of(em->qy) == 0);
b1 - 416:        h = fnv(h, (uint32_t)size_of(fv->g_.emitter->qy));
b1 - 430:    h = fnv(h, (uint32_t)size_of(fv->g_.emitter->qy));
b1 - 488:    static pico8_qy_t particle;
b1 - 502:    qy = fv->g_.emitter->qy;
```

Toutes les lignes `liste` du fichier :

```
31:#include "../src/pico8_list.h"
208:static int size_of(const pico8_list_t *list) {
489:    pico8_list_t *qy;
504:    pico8_list_add(qy, pico8_value_ref(&particle));
609:    pico8_list_t list;
612:    pico8_list_init(&list);
```

#### `tests/test_pico8_g_.c`
```
b2 - 94:static int list_count(const pico8_ptrs_t *list) {
b1 L 227:static int size_of(const pico8_list_t *list) {
b2 - 312:    assert(list_count(pico8_cg("g_")) == 0);
b1 - 365:    h = fnv(h, (uint32_t)size_of(em->qy));
b1 - 366:    assert(size_of(em->qy) == 0);
b1 - 402:    assert(a->emitter->qy != b->emitter->qy && b->emitter->qy != c->emitter->qy);
b1 - 412:    const pico8_qy_t *q;
b2 - 475:    assert(list_count(pico8_cg("np")) > 0);
b1 - 575:        h = fnv(h, (uint32_t)size_of(a->emitter->qy));
b1 - 576:        h = fnv(h, (uint32_t)size_of(b->emitter->qy));
b1 - 705:    assert(size_of(fv->emitter->qy) >= 0);
b1 - 706:    h = fnv(h, (uint32_t)size_of(fv->emitter->qy));
b1 - 715:        h = fnv(h, (uint32_t)size_of(fv->emitter->qy));
```

Toutes les lignes `liste` du fichier :

```
30:#include "../src/pico8_list.h"
227:static int size_of(const pico8_list_t *list) {
```

#### `tests/test_pico8_ga.c`
```
b2 L 134:static int list_size(const pico8_list_t *l) {
b2 - 317:    h = fnv(h, (uint32_t)list_size(jo->by));
b1 - 318:    h = fnv(h, (uint32_t)list_size(em->qy));
b2 - 319:    h = fnv(h, (uint32_t)list_size(jo->oe));
b1 - 335:    h = fnv(h, (uint32_t)(em->qy != NULL));
b2 - 352:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 355:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 363:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 364:    h = fnv(h, (uint32_t)(jo->by != NULL));
b2 - 367:    assert(jo->jy == NULL && !jo->has_kr);
b2 L 384:    assert(pico8_type_tag_of(jo->by) == PICO8_TYPE_TAG_LIST);
b1 - 423:    assert(as_ga(a)->emitter->qy != as_ga(b)->emitter->qy && as_ga(b)->emitter->qy != as_ga(c)->emitter->qy);
b2 - 424:    assert(as_ga(a)->jo.oe != as_ga(b)->jo.oe);
b2 - 425:    assert(as_ga(a)->jo.by == as_ga(b)->jo.by && as_ga(b)->jo.by == as_ga(c)->jo.by);
b1 - 434:    const pico8_qy_t *q;
b1 L 522:    pico8_list_t *qy = g->emitter->qy;
b1 - 561:        h = fnv(h, (uint32_t)list_size(as_ga(b)->emitter->qy));
b1 - 564:    as_ga(b)->emitter->qy->count = 3;
b2 - 592:    by = as_ga(c)->jo.by;
b2 - 593:    assert(by == as_ga(a)->jo.by && by->count == 0);
b2 - 599:    assert(list_size(as_ga(b)->jo.by) == 2);
b1 - 632:    static pico8_qy_t particle;
b1 - 651:    qy = em->qy;
```

Toutes les lignes `liste` du fichier :

```
29:#include "../src/pico8_list.h"
134:static int list_size(const pico8_list_t *l) {
384:    assert(pico8_type_tag_of(jo->by) == PICO8_TYPE_TAG_LIST);
522:    pico8_list_t *qy = g->emitter->qy;
542:    pico8_list_t *by;
594:    pico8_list_add(by, pico8_value_ref(pico8_trade_new(NULL)));
595:    pico8_list_add(by, pico8_value_ref(pico8_trade_new(NULL)));
636:    pico8_list_t *qy;
653:    pico8_list_add(qy, pico8_value_ref(&particle));
```

#### `tests/test_pico8_ga_export_panel_by.c`
```
b2 - 85:    return planet->jo.by;
b2 - 134:     * into whatever `self.by` already holds, here the value the `ga`
```

Toutes les lignes `liste` du fichier :

```
1:/* Six sites read the "by" field as a `pico8_list_t*`, so its producers
21:#include "../src/pico8_list.h"
95:    pico8_list_t *by;
140:    assert(pico8_type_tag_of(by) == PICO8_TYPE_TAG_LIST);
143:    pico8_list_add(by, pico8_value_ref(route_a));
144:    pico8_list_add(by, pico8_value_ref(route_b));
```

#### `tests/test_pico8_hj.c`
```
b2 L 154:static int list_size(const pico8_list_t *l) {
b1 - 204:    const pico8_qy_t *q;
b2 - 390:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 393:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 401:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 402:    h = fnv(h, (uint32_t)(jo->by != NULL));
b1 - 405:    assert(list_size(em->qy) == 0);
b2 - 421:    h = fnv(h, (uint32_t)list_size(jo->oe));
b2 - 422:    h = fnv(h, (uint32_t)list_size(jo->jy));
b1 - 445:    assert(as_hj(a)->emitter->qy != NULL && as_hj(a)->emitter->qy != as_hj(b)->emitter->qy);
b2 - 478:    pico8_value_t ib = pico8_value_ref(pico8_jo_ib(e));
b1 - 490:    return fnv(h, (uint32_t)list_size(as_hj(e)->emitter->qy));
b1 - 532:    h = fnv(h, (uint32_t)list_size(as_hj(placed)->emitter->qy));
b1 - 533:    assert(list_size(as_hj(placed)->emitter->qy) >= 2);
b1 - 562:    assert(list_size(hj->emitter->qy) == 0);
b2 - 655:        assert(is_an_hj_struct(bare) && bare->qh.jo.id == NULL && bare->qh.jo.oe != NULL &&
b1 - 656:               list_size(bare->emitter->qy) == 0 && listed(pico8_cg("hj"), bare));
```

Toutes les lignes `liste` du fichier :

```
28:#include "../src/pico8_list.h"
154:static int list_size(const pico8_list_t *l) {
```

#### `tests/test_pico8_hp.c`
```
b2 - 47:/* `gb[12]`, a colony that needs two resources and exports one; `gb[3]`, a
b2 - 447:        i_list = b->i;
b2 L 613:    const pico8_list_t *jy = p->jy;
b2 - 669:    assert(b->i->count == 2 && pico8_r_lookup("p", &p));
b2 - 670:    assert(((const pico8_jy_t *)b->i->items[0].as.ref)->row.has_qe);
b2 - 671:    assert(((const pico8_jy_t *)b->i->items[0].as.ref)->row.qe == pico8_fixed_from_int(p));
b2 - 672:    assert(!((const pico8_jy_t *)b->i->items[1].as.ref)->row.has_qe);
b2 - 676:    jy = as_jo(a)->jy;
b2 - 679:        assert(jy->items[k].as.ref != b->i->items[k].as.ref);
b2 - 680:        assert(memcmp(jy->items[k].as.ref, b->i->items[k].as.ref, sizeof(pico8_jy_t)) == 0);
b2 - 959:    assert(bare->i != NULL && bare->i->count == 0);
```

Toutes les lignes `liste` du fichier :

```
30:#include "../src/pico8_list.h"
137:        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST) {
138:            const pico8_list_t *l = v.as.ref;
430:        const pico8_list_t *i_list;
448:        assert(i_list != NULL && pico8_type_tag_of(i_list) == PICO8_TYPE_TAG_LIST);
613:    const pico8_list_t *jy = p->jy;
654:    const pico8_list_t *jy;
```

#### `tests/test_pico8_hy.c`
```
b2 - 45:/* `gb[12]`, a colony that needs two resources and exports one; `gb[3]`, a
b1 L 82:static const pico8_list_t *invented_of(const pico8_hy_t *hy) {
b1 L 83:    const pico8_list_t *l = hy->invented;
b1 L 161:static uint32_t fold_list_member(uint32_t h, const pico8_list_t *l) {
b1 - 197:    h = fold_list_member(h, hy->invented);
b1 - 331:    assert(bare->invented != NULL && bare->po == NULL && bare->fu == NULL && !bare->has_happiness);
b1 - 334:    assert(invented_of(bare)->count == 0);
b1 - 338:    assert(fm_hy()->invented != NULL && fm_hy()->po != NULL && fm_hy()->fu != NULL);
b1 - 341:    first = invented_of(fm_hy());
b1 - 342:    assert(first != invented_of(bare));
b1 - 345:    assert(invented_of(fm_hy()) != first && invented_of(fm_hy())->count == 0);
b1 L 376:    pico8_list_add(fm_hy()->invented, pico8_value_string("time compression"));
b2 - 489:        if (p->jy != NULL && p->kw != NULL && p->kw->has_qe) {
b2 L 508:    pico8_list_t needs;
b2 L 563:    pico8_list_init(&needs);
b2 L 564:    pico8_list_add(&needs, pico8_value_ref(need));
b2 - 565:    needer->jo.jy = &needs;
b2 - 568:    needer->jo.jy = NULL;
b1 - 594:    assert(invented_of(fm_hy())->count == 0 && fm_hy()->ew == pico8_fixed_from_int(1));
b1 - 599:        assert(invented_of(fm_hy())->count == (int)k + 1);
b1 - 608:    assert(invented_of(fm_hy())->count == (int)(sizeof pxs / sizeof pxs[0]));
```

Toutes les lignes `liste` du fichier :

```
30:#include "../src/pico8_list.h"
82:static const pico8_list_t *invented_of(const pico8_hy_t *hy) {
83:    const pico8_list_t *l = hy->invented;
84:    assert(l != NULL && pico8_type_tag_of(l) == PICO8_TYPE_TAG_LIST);
114:        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST) {
115:            const pico8_list_t *l = v.as.ref;
161:static uint32_t fold_list_member(uint32_t h, const pico8_list_t *l) {
327:    const pico8_list_t *first;
376:    pico8_list_add(fm_hy()->invented, pico8_value_string("time compression"));
508:    pico8_list_t needs;
563:    pico8_list_init(&needs);
564:    pico8_list_add(&needs, pico8_value_ref(need));
```

#### `tests/test_pico8_ie.c`
```
b1 - 518:    assert(pico8_fm_hy() != NULL && pico8_fm_hy()->invented != NULL);
b1 L 519:    pico8_list_add(pico8_fm_hy()->invented, pico8_value_string(rr));
```

Toutes les lignes `liste` du fichier :

```
30:#include "../src/pico8_list.h"
519:    pico8_list_add(pico8_fm_hy()->invented, pico8_value_string(rr));
```

#### `tests/test_pico8_jc.c`
```
b2 L 248:static int list_count(const pico8_list_t *l) {
b1 - 287:    h = fnv(h, (uint32_t)list_count(jc->ts));
b2 L 302:        const pico8_list_t *oe = as_jo(p)->oe;
b2 - 303:        h = fnv(h, (uint32_t)list_count(oe));
b1 - 474:    assert(list_count(jc->ts) == 0);
b1 - 512:    assert(as_jc(by_ldown)->ts != as_jc(by_lookup)->ts);
b2 - 532:    assert(list_count(as_jo(origin)->oe) == 0 && list_count(as_jo(target)->oe) == 0);
b2 - 541:    assert(list_count(as_jo(origin)->oe) == 0);
b2 - 548:    assert(list_count(as_jo(origin)->oe) == 1 && list_count(as_jo(target)->oe) == 1);
b2 - 736:    assert(list_count(as_jo(origin)->oe) == 1 && list_count(as_jo(target)->oe) == 1);
b2 - 740:    assert(list_count(as_jo(origin)->oe) == 1 && list_count(as_jo(target)->oe) == 1);
```

Toutes les lignes `liste` du fichier :

```
27:#include "../src/pico8_list.h"
248:static int list_count(const pico8_list_t *l) {
302:        const pico8_list_t *oe = as_jo(p)->oe;
```

#### `tests/test_pico8_jo.c`
```
b2 L 146:static int list_size(const pico8_list_t *l) {
b2 - 250:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 253:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 261:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 262:    h = fnv(h, (uint32_t)(jo->by != NULL));
b2 - 306:    h = fnv(h, (uint32_t)list_size(jo->oe));
b2 - 307:    h = fnv(h, (uint32_t)list_size(jo->jy));
b2 - 308:    h = fnv(h, (uint32_t)list_size(jo->by));
b2 - 309:    h = fnv(h, (uint32_t)list_size(jo->cc));
b2 - 519:    assert(a->hn == hp.as.ref && list_size(a->jy) >= 0);
b1 - 534: * `me` for the planet's state; any other draw is a `jo.oc()` entry, and
b2 - 582:        assert(is_a_jo_struct(bare) && bare->id == NULL && bare->ra == NULL && bare->oe != NULL);
```

Toutes les lignes `liste` du fichier :

```
28:#include "../src/pico8_list.h"
146:static int list_size(const pico8_list_t *l) {
```

#### `tests/test_pico8_jo_spec_defaults.c`
```
b2 - 220:    assert(ga->jo.by != NULL && ga->jo.by == other->jo.by);
```

Toutes les lignes `liste` du fichier :

```
```

#### `tests/test_pico8_js.c`
```
b2 - 50:/* `gb[12]`, a colony that needs two resources and exports one; `gb[3]`, a
b1 - 642:    assert(pico8_fm_hy() != NULL && pico8_fm_hy()->invented != NULL);
b1 L 643:    pico8_list_add(pico8_fm_hy()->invented, pico8_value_string(rr));
b1 - 942:    invented = pico8_fm_hy()->invented;
b1 - 945:    invented = pico8_fm_hy()->invented;
```

Toutes les lignes `liste` du fichier :

```
31:#include "../src/pico8_list.h"
643:    pico8_list_add(pico8_fm_hy()->invented, pico8_value_string(rr));
875:    pico8_list_t cb;
889:    pico8_list_free(&cb);
930:    const pico8_list_t *invented;
1302:    pico8_list_t *list = malloc(sizeof *list);
1370:    pico8_list_init(list);
1371:    pico8_list_add(list, pico8_value_string("from a list"));
1372:    pico8_list_add(list, pico8_value_fixed(pico8_fixed_from_int(3)));
1704:    pico8_list_t kept;
1725:    pico8_list_free(&kept);
```

#### `tests/test_pico8_lq.c`
```
b2 - 44:/* `gb[12]`, a colony that needs two resources and exports one; `gb[3]`, a
b2 L 431:        const pico8_list_t *is = p->i;
b2 - 435:            const void *lq = pico8_jo_ex(is, o, pico8_fixed_from_int(jqs[b]), pico8_fixed_from_int((int)b % 3),
b2 - 446:        const void *lq = pico8_jo_ex(NULL, o, pico8_fixed_from_int(2), 0, pico8_fixed_from_int(1));
b2 - 450:    assert(pico8_jo_ex(NULL, pico8_value_bool(0), pico8_fixed_from_int(1), 0, 0) == NULL);
b2 - 730:    planet->jy = jy;
b2 - 742:    assert(wants != NULL && receives != NULL && panel_row("needs:") == NULL);
```

Toutes les lignes `liste` du fichier :

```
29:#include "../src/pico8_list.h"
109:        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST) {
110:            const pico8_list_t *l = v.as.ref;
180:    const pico8_list_t *fs = as_lq(lq)->fs;
181:    assert(fs != NULL && pico8_type_tag_of(fs) == PICO8_TYPE_TAG_LIST);
431:        const pico8_list_t *is = p->i;
530:    pico8_list_t rb;
551:    pico8_list_init(&rb);
552:    pico8_list_add(&rb, pico8_value_ref(&entry));
715:    pico8_list_t *jy = malloc(sizeof *jy);
726:    pico8_list_init(jy);
727:    pico8_list_add(jy, pico8_value_ref(a));
728:    pico8_list_add(jy, pico8_value_ref(b));
```

#### `tests/test_pico8_map_tables.c`
```
b1 L 133:    pico8_list_t hr;
b1 L 137:    pico8_list_init(&hr);
b1 L 139:        pico8_list_add(&hr, pico8_value_ref((void *)&pico8_df_table[i]));
b1 - 141:    state = pico8_gn(&hr, pico8_fixed_from_int(2), never_drawn, NULL);
b1 - 142:    assert(state->oc.count == 66);
b1 - 148:        for (j = 0; j < state->oc.count; j++) {
b1 - 149:            got += state->oc.items[j].as.ref == &pico8_df_table[i];
b1 - 153:    pico8_gn_free(state);
b1 L 154:    pico8_list_free(&hr);
```

Toutes les lignes `liste` du fichier :

```
10:#include "../src/pico8_list.h"
133:    pico8_list_t hr;
137:    pico8_list_init(&hr);
139:        pico8_list_add(&hr, pico8_value_ref((void *)&pico8_df_table[i]));
154:    pico8_list_free(&hr);
```

#### `tests/test_pico8_mx.c`
```
b2 - 88:/* `listed` for a list member of an entity (`jo.oe`), not a registry. */
b2 L 89:static int value_listed(const pico8_list_t *list, const void *e) {
b2 L 154:static int list_size(const pico8_list_t *l) {
b1 - 187:    const pico8_qy_t *q;
b1 - 367:    h = fnv(h, (uint32_t)(em->qy != NULL));
b2 - 394:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 397:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 405:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 406:    h = fnv(h, (uint32_t)(jo->by != NULL));
b1 - 409:    assert(list_size(em->qy) == 0);
b2 - 428:    h = fnv(h, (uint32_t)list_size(jo->oe));
b2 - 429:    h = fnv(h, (uint32_t)list_size(jo->jy));
b1 - 452:    assert(as_hj(a)->emitter->qy != NULL && as_hj(a)->emitter->qy != as_hj(b)->emitter->qy);
b2 - 479:    pico8_value_t ib = pico8_value_ref(pico8_jo_ib(e));
b1 - 491:    return fnv(h, (uint32_t)list_size(as_hj(e)->emitter->qy));
b1 - 531:    h = fnv(h, (uint32_t)list_size(as_hj(placed)->emitter->qy));
b1 - 532:    assert(list_size(as_hj(placed)->emitter->qy) >= 2);
b2 - 556:    assert(value_listed(as_hj(synthesizer)->qh.jo.oe, jc));
b2 - 571:    as_hj(e)->qh.jo.jy = jy;
b2 - 582:    return fnv(h, (uint32_t)list_size(jo->oe));
b2 - 684:        assert(is_an_mx_struct(bare) && bare->hj.qh.jo.id == NULL && bare->hj.qh.jo.oe != NULL &&
b1 - 685:               bare->hj.qh.jo.kw != NULL && list_size(bare->hj.emitter->qy) == 0 && listed(pico8_cg("jo"), bare));
```

Toutes les lignes `liste` du fichier :

```
29:#include "../src/pico8_list.h"
89:static int value_listed(const pico8_list_t *list, const void *e) {
154:static int list_size(const pico8_list_t *l) {
565:    pico8_list_t *jy = malloc(sizeof *jy);
569:    pico8_list_init(jy);
570:    pico8_list_add(jy, pico8_value_ref(row));
```

#### `tests/test_pico8_nl.c`
```
b2 L 138:static int list_size(const pico8_list_t *l) {
b2 - 327:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 330:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 338:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 339:    h = fnv(h, (uint32_t)(jo->by != NULL));
b2 - 359:    h = fnv(h, (uint32_t)list_size(jo->oe));
b2 - 360:    h = fnv(h, (uint32_t)list_size(jo->jy));
b2 - 416:    pico8_value_t ib = pico8_value_ref(pico8_jo_ib(e));
b2 - 525:        assert(is_an_nl_struct(bare) && bare->qh.jo.id == NULL && bare->qh.jo.oe != NULL &&
```

Toutes les lignes `liste` du fichier :

```
28:#include "../src/pico8_list.h"
138:static int list_size(const pico8_list_t *l) {
```

#### `tests/test_pico8_nr.c`
```
b2 L 139:static int list_size(const pico8_list_t *l) {
b2 - 321:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 324:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 332:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 333:    h = fnv(h, (uint32_t)(jo->by != NULL));
b2 - 359:    h = fnv(h, (uint32_t)list_size(jo->oe));
b2 - 360:    h = fnv(h, (uint32_t)list_size(jo->jy));
b2 - 381:    assert(a != b && as_qh(a)->jo.oe != as_qh(b)->jo.oe && as_qh(a)->jo.kw != as_qh(b)->jo.kw);
b2 - 408:    pico8_value_t ib = pico8_value_ref(pico8_jo_ib(e));
b2 L 418:static pico8_list_t *list_of(int n, void *item) {
b2 - 481:    oe = as_qh(placed)->jo.oe;
b2 - 483:        as_qh(placed)->jo.oe = list_of(i, idle);
b2 - 492:    as_qh(placed)->jo.oe = oe;
b2 - 505:    assert(gate_kind(bare) == NR && as_qh(bare)->jo.id == NULL && as_qh(bare)->jo.oe != NULL &&
b2 - 551:        assert(is_an_nr_struct(bare) && bare->qh.jo.id == NULL && bare->qh.jo.oe != NULL && bare->qh.jo.kw != NULL);
```

Toutes les lignes `liste` du fichier :

```
27:#include "../src/pico8_list.h"
139:static int list_size(const pico8_list_t *l) {
418:static pico8_list_t *list_of(int n, void *item) {
419:    pico8_list_t *l = malloc(sizeof *l);
422:    pico8_list_init(l);
424:        pico8_list_add(l, pico8_value_ref(item));
442:    pico8_list_t *oe;
```

#### `tests/test_pico8_os.c`
```
b2 L 152:static int list_size(const pico8_list_t *l) {
b2 - 354:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 357:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 365:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 366:    h = fnv(h, (uint32_t)(jo->by != NULL));
b2 - 384:    h = fnv(h, (uint32_t)list_size(jo->oe));
b2 - 385:    h = fnv(h, (uint32_t)list_size(jo->jy));
b2 - 439:    pico8_value_t ib = pico8_value_ref(pico8_jo_ib(e));
b2 - 447:/* `pt.by` holding one export from `pt` to `oz`, as `jo:ib` leaves it: a
b2 - 450:    pico8_trade_keys_t keys = {0};
b2 - 458:    keys.lp = lp;
b2 - 463:    ((pico8_jo_t *)pt)->by = by;
b2 - 576:        assert(is_an_os_struct(bare) && bare->qh.jo.id == NULL && bare->qh.jo.oe != NULL &&
```

Toutes les lignes `liste` du fichier :

```
28:#include "../src/pico8_list.h"
152:static int list_size(const pico8_list_t *l) {
451:    pico8_list_t *lp = malloc(sizeof *lp);
452:    pico8_list_t *by = malloc(sizeof *by);
455:    pico8_list_init(lp);
456:    pico8_list_add(lp, pico8_value_ref(pt));
457:    pico8_list_add(lp, pico8_value_ref(oz));
460:    pico8_list_init(by);
461:    pico8_list_add(by, pico8_value_ref(t));
```

#### `tests/test_pico8_pj.c`
```
b1 L 705:        pico8_list_add(fm->invented, pico8_value_fixed(pico8_fixed_from_int(i)));
b1 L 739:        pico8_list_add(fm->invented, pico8_value_fixed(pico8_fixed_from_int(i)));
```

Toutes les lignes `liste` du fichier :

```
31:#include "../src/pico8_list.h"
705:        pico8_list_add(fm->invented, pico8_value_fixed(pico8_fixed_from_int(i)));
739:        pico8_list_add(fm->invented, pico8_value_fixed(pico8_fixed_from_int(i)));
```

#### `tests/test_pico8_pm.c`
```
b2 L 139:static int list_size(const pico8_list_t *l) {
b1 - 172:    const pico8_qy_t *q;
b1 - 301:    h = fnv(h, (uint32_t)list_size(g->emitter->qy));
b1 - 370:    h = fnv(h, (uint32_t)(em->qy != NULL));
b2 - 397:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 400:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 408:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 409:    h = fnv(h, (uint32_t)(jo->by != NULL));
b1 - 412:    assert(list_size(em->qy) == 0);
b2 - 432:    h = fnv(h, (uint32_t)list_size(jo->oe));
b2 - 433:    h = fnv(h, (uint32_t)list_size(jo->jy));
b1 - 460:    assert(as_hj(a)->emitter->qy != NULL && as_hj(a)->emitter->qy != as_hj(b)->emitter->qy);
b2 - 483:    pico8_value_t ib = pico8_value_ref(pico8_jo_ib(e));
b1 - 495:    return fnv(h, (uint32_t)list_size(as_hj(e)->emitter->qy));
b1 - 527:    h = fnv(h, (uint32_t)list_size(as_hj(placed)->emitter->qy));
b1 - 528:    assert(list_size(as_hj(placed)->emitter->qy) >= 2);
b1 - 571:               bare->hj.qh.jo.oe != NULL && list_size(bare->hj.emitter->qy) == 0 && listed(pico8_cg("jo"), bare));
```

Toutes les lignes `liste` du fichier :

```
28:#include "../src/pico8_list.h"
139:static int list_size(const pico8_list_t *l) {
```

#### `tests/test_pico8_probe_mm_rnd_order.c`
```
b1 - 46:    assert(probe->emitter != NULL && probe->emitter->qy != NULL);
b1 L 47:    it = pico8_list_all(probe->emitter->qy);
b1 - 49:    return ((const pico8_qy_t *)particle.as.ref)->p;
```

Toutes les lignes `liste` du fichier :

```
21:#include "../src/pico8_list.h"
42:    pico8_list_iter_t it;
47:    it = pico8_list_all(probe->emitter->qy);
48:    assert(pico8_list_iter_next(&it, &particle) && particle.kind == PICO8_VALUE_REF && particle.as.ref != NULL);
```

#### `tests/test_pico8_qh.c`
```
b2 L 184:static int list_size(const pico8_list_t *l) {
b2 - 330:    h = fnv(h, (uint32_t)(jo->oe != NULL));
b2 - 333:    h = fnv(h, (uint32_t)(jo->jy != NULL));
b2 - 341:    h = fnv(h, (uint32_t)(jo->cc != NULL));
b2 - 342:    h = fnv(h, (uint32_t)(jo->by != NULL));
b2 - 358:    h = fnv(h, (uint32_t)list_size(jo->oe));
b2 - 359:    h = fnv(h, (uint32_t)list_size(jo->jy));
b2 - 678:        assert(is_a_qh_struct(bare) && bare->jo.id == NULL && bare->jo.oe != NULL && listed(pico8_cg("qh"), bare));
```

Toutes les lignes `liste` du fichier :

```
27:#include "../src/pico8_list.h"
184:static int list_size(const pico8_list_t *l) {
```

#### `tests/test_pico8_trade.c`
```
b2 L 124:static uint32_t fold_ranks(uint32_t h, const pico8_list_t *l) {
b2 - 138:static uint32_t fold_route(uint32_t h, const pico8_trade_t *t) {
b2 - 140:    h = fold_ranks(h, t->lp);
b2 - 141:    h = fold_ranks(h, t->rl);
b2 - 143:    if (pico8_trade_to(t, &to)) {
b2 L 208:static pico8_list_t *list_of(int n, const pico8_value_t *items) {
b2 L 219:static pico8_trade_t *route(pico8_list_t *lp, pico8_list_t *rl, int has_qe, int qe_is_bool, pico8_fixed_t qe) {
b2 - 220:    pico8_trade_keys_t keys = {0};
b2 - 222:    keys.lp = lp;
b2 - 223:    keys.rl = rl;
b2 - 240:static pico8_trade_t *game_route(void) {
b2 L 263:        const pico8_list_t *by = as_jo(jo->items[i])->by;
b2 - 276:    pico8_trade_t *g = game_route();
b2 L 277:    pico8_list_t *lp = g->lp;
b2 L 278:    pico8_list_t *rl = g->rl;
b2 - 282:    h = fold_route(h, g);
b2 - 283:    t = route(lp, rl, g->has_qe, g->qe_is_bool, g->qe);
b2 - 285:    assert(t->lp == lp && t->rl == rl);
b2 - 287:    h = fold_route(h, t);
b2 - 290:    assert(t != NULL && t->lp == NULL && t->rl == NULL && !t->has_qe);
b2 - 291:    t = route(lp, NULL, 0, 0, 0);
b2 - 292:    assert(t->lp == lp && t->rl == NULL && !t->has_qe);
b2 - 304:    (void)game_route();
b2 - 308:    t = route(list_of(3, items), NULL, 0, 0, 0);
b2 - 309:    assert(pico8_trade_to(t, &out) && out.kind == PICO8_VALUE_REF && out.as.ref == planet(2));
b2 - 310:    t = route(list_of(1, items), NULL, 0, 0, 0);
b2 - 311:    assert(pico8_trade_to(t, &out) && out.as.ref == planet(0));
b2 - 314:    assert(!pico8_trade_to(route(list_of(0, items), NULL, 0, 0, 0), &out) && same_value(out, sentinel));
b2 - 315:    assert(!pico8_trade_to(pico8_trade_new(NULL), &out) && same_value(out, sentinel));
b2 - 316:    assert(!pico8_trade_to(NULL, &out) && same_value(out, sentinel));
b2 L 319:static int list_is(const pico8_list_t *l, int n, const pico8_value_t *items) {
b2 - 336:    pico8_trade_t *g = game_route();
b2 L 337:    const pico8_list_t *lp = g->lp;
b2 L 338:    const pico8_list_t *rl = g->rl;
b2 - 348:    k = pico8_trade_kg(g, ref(np), ref(jc));
b2 - 350:    assert(g->lp == lp && g->rl == rl && lp->count == lp_n && rl->count == rl_n);
b2 - 351:    assert(k->lp != lp && k->rl != rl);
b2 - 356:    assert(list_is(k->lp, lp_n + 1, want));
b2 - 361:    assert(list_is(k->rl, rl_n + 1, want));
b2 - 364:    k = pico8_trade_kg(g, ref(np), nil());
b2 - 365:    assert(k->lp->count == lp_n + 1 && list_is(k->rl, rl_n, rl->items));
b2 - 366:    assert(k->rl != rl);
b2 - 369:    k = pico8_trade_kg(g, nil(), nil());
b2 - 370:    assert(list_is(k->lp, lp_n, lp->items) && list_is(k->rl, rl_n, rl->items));
b2 L 372:    k = pico8_trade_kg(route((pico8_list_t *)lp, NULL, 0, 0, 0), ref(np), nil());
b2 - 373:    assert(k->lp->count == lp_n + 1 && k->rl->count == 0);
b2 - 376:    k = pico8_trade_kg(pico8_trade_new(NULL), ref(np), ref(jc));
b2 - 378:    assert(list_is(k->lp, 1, want));
b2 - 380:    assert(list_is(k->rl, 1, want));
b2 - 383:    assert(pico8_trade_kg(NULL, ref(np), ref(jc)) == NULL);
b2 - 395:    (void)game_route();
b2 - 405:    ta = route(list_of(2, a), NULL, 0, 0, 0);
b2 - 406:    tb = route(list_of(3, b), NULL, 0, 0, 0);
b2 - 408:    assert(pico8_trade_eq(ta, route(list_of(2, a), list_of(0, a), 1, 1, 1)));
b2 - 409:    assert(!pico8_trade_eq(ta, route(list_of(2, c), NULL, 0, 0, 0)));
b2 - 410:    assert(!pico8_trade_eq(ta, route(list_of(2, d), NULL, 0, 0, 0)));
b2 - 411:    assert(!pico8_trade_eq(ta, route(list_of(0, a), NULL, 0, 0, 0)));
b2 - 412:    assert(!pico8_trade_eq(route(list_of(0, a), NULL, 0, 0, 0), ta));
b2 - 435:    (void)route(NULL, NULL, 1, 1, 2);
b2 - 440:    (void)route(NULL, NULL, 0, 0, 1);
b2 - 445:    (void)route(NULL, NULL, 0, 1, 0);
b2 - 450:    (void)pico8_trade_to(e, &out);
b2 - 454:    (void)pico8_trade_kg(e, nil(), nil());
b2 - 470:    pico8_trade_t *g = game_route();
b2 - 496:static void count_kf(pico8_jo_trade_proposal_t *self, pico8_value_t a) {
b2 - 505:    pico8_trade_keys_t keys = {0};
b2 - 506:    pico8_jo_trade_proposal_t lw = {0};
b2 - 520:    qh->jo.jy = jy;
b2 - 524:    lw.kf = count_kf;
```

Toutes les lignes `liste` du fichier :

```
31:#include "../src/pico8_list.h"
124:static uint32_t fold_ranks(uint32_t h, const pico8_list_t *l) {
208:static pico8_list_t *list_of(int n, const pico8_value_t *items) {
209:    pico8_list_t *l = malloc(sizeof *l);
211:    pico8_list_init(l);
213:        pico8_list_add(l, items[i]);
219:static pico8_trade_t *route(pico8_list_t *lp, pico8_list_t *rl, int has_qe, int qe_is_bool, pico8_fixed_t qe) {
263:        const pico8_list_t *by = as_jo(jo->items[i])->by;
277:    pico8_list_t *lp = g->lp;
278:    pico8_list_t *rl = g->rl;
319:static int list_is(const pico8_list_t *l, int n, const pico8_value_t *items) {
337:    const pico8_list_t *lp = g->lp;
338:    const pico8_list_t *rl = g->rl;
372:    k = pico8_trade_kg(route((pico8_list_t *)lp, NULL, 0, 0, 0), ref(np), nil());
507:    pico8_list_t *jy = malloc(sizeof *jy);
517:    pico8_list_init(jy);
518:    pico8_list_add(jy, pico8_value_ref(want));
```

#### `tests/test_pico8_ui.c`
```
b2 L 491:    const pico8_list_t *oe = as_jo(planet)->oe;
```

Toutes les lignes `liste` du fichier :

```
36:#include "../src/pico8_list.h"
491:    const pico8_list_t *oe = as_jo(planet)->oe;
824:    pico8_list_t list;
826:    pico8_list_init(&list);
```
