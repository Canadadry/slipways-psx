---
title: "Plan 3, étape D.12c : `lq.items` en union étiquetée locale à `layout`, `lq.fs`, `jb.ls`, `mh.items` et les listes locales d'affichage en tableaux typés ; `pico8_jp` et `pico8_nv` à valeur retirés ; les TU suivent dans le même commit ; deux sous-étapes"
description: "Après D.12b, plus aucune liste membre d'entité n'est une `pico8_list_t`, mais la mise en page (`lq.items`, `lq.fs`, `jb.ls`), la boîte de message (`mh.items`), `bl` et une vingtaine de listes locales d'affichage (`jo:gh`, `jo.ex`, `hp:lq`, `cb`, `bj`, `pj:ph`, `pj:kj`, `fv:kj`) en tiennent encore, ainsi que les deux dernières aides à valeur de `collect`. D.12c les fait passer sur une union étiquetée à cinq cas pour `lq.items` (Q10), sur des tableaux typés pour le reste (bornés par le cart quand il les borne), retire `pico8_jp`/`pico8_nv` et fait suivre 10 TU (dont les trois §6) : `liste` de `src/` tombe à 50 lignes, exact."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.12b était implémentée** (`plan-pilot`, étape 2b), sur l'arbre **commis** `cd9247d` (« docs: PRD de D.12b revérifié après D.12a et mis en file, deux sous-étapes ») extrait par `git archive`. D.12b n'est pas gradée : tout ce que ce PRD suppose de son issue est une hypothèse `H-D12b-n` (table plus bas), à confronter à sa note « Découvert à D.12b » avant de mettre ce PRD en file. Seuls `grep`, `sed`, `awk`, `sh tools/data_residue.sh` et la recette de mesure (motif de Q10) ont tourné. **Rien n'a été compilé ni lancé.** Les numéros de ligne sont ceux de `cd9247d` ; ceux que D.12b déplacera ou réécrira portent la marque **⟲** (Inventaire, Annexes C et D) et sont à remesurer **par contenu** à la graduation de D.12b.

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_list.c` disparaisse (D.12d). Après D.12b, les listes membres des entités sont des `pico8_ptrs_t`, mais la couche d'affichage garde des `pico8_list_t` de `pico8_value_t` partout où le cart construit une table qu'il passe à `lq()`, à `jb()`, à `mh()` ou à `bl()` :

- la rangée `lq` range ses positionnels (`lq.items`) comme des valeurs à genre libre, que `lq:mn` classe à chaque dessin ; ses entrées mises en page (`lq.fs`) sont une liste de REF vers des enregistrements alloués un par un ; la bulle `jb` range ses lignes (`jb.ls`) comme une liste de REF ;
- la boîte de message (`mh.items`) et `bl` passent leurs articles comme des REF dans une liste ;
- `jo:gh` construit trois ou quatre listes d'affichage hétérogènes et les passe à `js:pq` par une REF de liste (`PICO8_REF_AS_LIST`), que `pico8_js_lq` reconnaît à son étiquette ;
- `jo.ex` construit sa rangée dans une liste locale, `hp:lq` ajoute deux éléments à la liste de l'`lq` rendu ; `cb` et `bj` filtrent une table statique dans une liste neuve par `pico8_jp` ; `lq:pq` fait sa mise en page par `pico8_nv`.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `cd9247d` (§5, « Phase D — Données », l. 279) :

> - D.12c — `liste`, listes hétérogènes L3 et locales L4 : `lq.items`, `mh.items`, `jb.ls` → union étiquetée locale à `layout` {rien, fixe, chaîne, rangée, autre-vrai} (Q10), `lq.fs` en tableau typé de fixes ; listes locales d'affichage (`planet_display`, `menu`, `msgbox:pz`, `trade:kg`, `research*`, `score_panel*`, `tech_gate`, `slipway`, `probe`, `galaxy`) en tableaux fixes là où la borne est dans le cart. Empreinte ≈ 16 (15, 1). Compte : `liste` = 0 dans `src/` hors `pico8_list.c/.h`, `pico8_ref_audit.h` et `pico8_type_tag.h` (49 lignes aujourd'hui, à D.12d). §6 : `layout_nil_item_draws_nothing` passe son item par l'union, même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.

Décisions de D.9, recopiées (l. 265) :

> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Paragraphe « Liste de D », empreinte et règles communes, ordre, recopiés (l. 237, 239, 241) :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.

> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Comptes au départ de D.10 et règle des ponts de TU, recopiés (l. 267) :

> Comptes au départ de D.10 (HEAD 5a13008, `make -s data-residue` et `--detail` ; l'étude mesurait `2dec71f`, avant D.7a–D.8) : `valeur` 4961/126, `type-tag` 352/65, `ref-audit` 56/15, `par-nom` 1009/81 (`src/` 433/43, `tests/` 576/38), `dispatch` 191/61 (`src/` 67/30, `tests/` 124/31), résidu D 5868 lignes / 128 fichiers ; `liste`, pas encore au compteur, 927/99 avec le motif de Q10 (`src/` 496/61, `tests/` 431/38). Les classes A–M, T1/T2 et L1–L4 sont celles de l'étude (annexe A) ; ses « ≈ » viennent de son classement, chaque PRD remesure. Un TU qui nomme un symbole retiré par une étape de `src/` la suit dans le même commit, ou le symbole reste comme pont compté, fermé par l'étape TU de son groupe (D.10c, D.11d, D.12d) ; le PRD tranche. `sanitize-net` : aux seules étapes qui changent une durée de vie ou un propriétaire (§7).

Toutes les autres lignes de la phase D (D.0 à D.12d) **avec toutes leurs notes « Découvert à D.x »** (D.0, D.1, D.7c, D.8, D.10a, D.10b, D.10c, D.11a, D.11b, D.11c, D.11d1 à D.11d4 et la décision humaine qui découpe D.11d, D.11e1 à D.11e3, D.12a1, D.12a2), l'arrêt « champ sans règle », les parties du §5 qui fixent les règles (en-tête et phase 1a, phase 1b, phase D en entier, phases 2 et 3), le §6 et le §7 entiers sont recopiés tels quels, par extraction de lignes de `cd9247d`, en **Annexe A**. Les passages de l'étude D.9 sur `pico8_list_t` (§3.1, §3.2, point 2 du §4, table des étapes, Q9, Q10) sont en **Annexe B**. L'**Annexe C** est l'inventaire ligne à ligne des lignes `liste` de `src/` que l'étape retire, classées par sous-étape ; l'**Annexe D**, celui des TU.

Notes « Découvert à » et consignes antérieures qui visent D.12c, extraites de ces textes et des PRD de D.12a/D.12b :

- D.11e3, remarque de clôture : « `pico8_lq_bg` et `pico8_pj_ed` ne servent plus qu'aux TU et rendent un `pico8_value_t` ».
- PRD de D.12b, « Pour D.12c » : « `ji` et `merged` de `jo:gh` reçoivent des `pico8_value_ref(e)` de besoins ; `pico8_jp` (`cb`, `bj`) et `pico8_nv` (`layout`) à valeur ; nom des aides `_ptrs` ; les listes locales d'affichage restantes ». Et ses « Déplacements depuis D.12c, déclarés » : `hr_list` de `galaxy`, `{np}`/`{jc}` de `trade:kg` et les trois concaténations de `jo:gh` qui prennent `jy` (listes locales `prefix`/`last_only`) passent à D.12b.
- PRD de D.12b, « Pour D.12d » : « includes gardés (`particle.h`, `slipway.h`, `gen.h`, `planet_trade.h`, `research.h`, `trade.h`, s'ils le sont) ; les `FROZEN_LIST_TAG` des TU restent ». Ce PRD les prend en D.12c pour `src/` (voir « Includes »), puisque la ligne D.12c fixe `liste` = 0 dans `src/` hors quatre fichiers.

### Hypothèses `H-D12b-n` (D.12b en cours d'implémentation)

Source : le PRD de D.12b (`docs/prd/triage/plan3-phaseD-etape12b-listes-membres-aides-collect.md` à `cd9247d`, deux sous-étapes b1 : `qy`, `ts`, `invented`, `gen.oc` ; b2 : `oe`, `jy`/`hp.i`, `cc`, `by` avec `g_ga_by_shared`, `lp`/`rl`, `ib`) et sa ligne du plan. Chaque hypothèse dit ce qui en dépend ici et comment la vérifier à la graduation.

| Hypothèse | Énoncé | Ce qui en dépend dans ce PRD | Vérification à la graduation |
|---|---|---|---|
| H-D12b-1 | Après b2, `liste` ≈ 355 lignes (`src/` ≈ 200, `tests/` ≈ 155), rapporté dans la note ; `DATA_RESIDUE_GUARD` inchangé ; les 49 lignes de `pico8_list.c/.h`, `pico8_ref_audit.h`, `pico8_type_tag.h` et `pico8_value.h:7` inchangées | « État de départ », « Effet sur `make data-residue` », critère de départ | `make -s data-residue` ; recette ; note « Découvert à D.12b » |
| H-D12b-2 | Aides à valeur **retirées** par D.12b : `pico8_gu`, `pico8_gu_content`, `pico8_la`, `pico8_la_append` (et leurs déclarations). **Survivent** avec exactement ces appelants : `pico8_jp` (+ `pico8_jp_predicate_fn`) : `research_display.c` (`bj`), `tech_gate.c` (`cb`) ; `pico8_nv` (+ `pico8_nv_fn`) : `layout.c` (`lq:pq`) ; `collect.h` inclut encore `pico8_list.h` pour elles | « Aides de `collect` », grep 2 de la Sortie, c1 | `grep -nE '\bpico8_(jp|nv)\(' src/*.c` : 2 + 1 appelants et 2 définitions ; `grep -n pico8_list src/pico8_collect.h` |
| H-D12b-3 | Aides `_ptrs` gardées sous ces noms : `pico8_jp_ptrs`, `pico8_nv_ptrs`, `pico8_gu_ptrs`, `pico8_la_ptrs` (D.12a), `pico8_gu_content_ptrs` (nouvelle, b1) ; `pico8_nv_ptrs` sert `jo:lz`, `pico8_jp_ptrs` sert `jo:gh` | « Noms des aides » | lecture de `pico8_collect.h` |
| H-D12b-4 | `jo:gh` après b2 : `jx = pico8_jp_ptrs(jy, pico8_jo_gh_jp_step, &ctx)`, libéré par `pico8_ptrs_free` ; le prédicat prend `void *e` et fait `pico8_list_add(ctx->ji, pico8_value_ref(e))` ; `ji` reste une `pico8_list_t *` de tas ; les deux `merged` restent des `pico8_list_t *` de tas qui reçoivent l'en-tête puis chaque besoin enveloppé `pico8_value_ref(e)` écrits au site (≈ 3 à 5 lignes `liste` ajoutées) ; `prefix` et `last_only` ont disparu ; `lb`, la boucle `PICO8_REF_AS_LIST` des écarts et `bj_list` inchangés | « `jo:gh` », Annexe C (lignes ⟲ de `planet_display.c`), M17–M19 | lecture de `pico8_jo_gh` au HEAD gradé |
| H-D12b-5 | D.12b retire toutes les lignes `liste` de `galaxy.c`, `gen.c`, `particle.c`, `slipway.c`, `slipway_display.c`, `economy*.c`, `planet.c/.h`, `planet_family*.c`, `planet_trade.c`, `trade.c` hors include, et les lignes b1/b2 de son Annexe C ; aucune ligne `liste` nouvelle hors `jo:gh` | « Inventaire » (seuls restent les sites de l'Annexe C ci-dessous) | `sh tools/data_residue.sh --detail \| grep ^liste` ; recette par fichier |
| H-D12b-6 | Après b2 : `jo.jy`, `hp.i`, `jo.by` sont des `pico8_ptrs_t *` ; `pico8_jo_ex(const pico8_ptrs_t *is, …)` ; la liste locale `l` de `jo.ex`, les ajouts de `hp:lq` à `l->items`, `pico8_bj`, `pico8_cb`, `cb_list` de `menu.c` sont inchangés ; `pico8_menu_invented_lines` prend un `const pico8_ptrs_t *` | « `jo.ex` et `hp:lq` », « `cb`, `bj` » | lecture de `research_display.c`, `menu.c`, `tech_gate.c` |
| H-D12b-7 | Includes : D.12b applique la règle de D.12a ; parmi `collect.h`, `economy_formulas.h`, `gen.h`, `particle.h`, `planet_trade.h`, `research.h`, `slipway.h`, `trade.h`, `planet_family_trade.c`, `trade.c`, il garde l'`#include "pico8_list.h"` de ceux dont un dépendant s'en sert encore, et le dit à sa note ; `menu.c:17` et `research_display.h:7` gardés (listes de D.12c) | « Includes », Annexe C (lignes `inc` ⟲) | note de D.12b ; `grep -n 'pico8_list.h' src/*.[ch]` |
| H-D12b-8 | `FROZEN_LIST_TAG 0x9E5C1157u` existe dans les TU `fr`, `hj`, `mx`, `nl`, `nr`, `os`, `pm`, `hy` ; **pas** dans `ga`, `hp`, `lq`, `jb_ol` ; `test_pico8_hp` a déjà `VIEW_LIST_TAG = 0x9E5C1157u` | « Piège des étiquettes », c1 | `grep -n 'FROZEN_LIST_TAG\|VIEW_LIST_TAG' tests/*.c` |
| H-D12b-9 | D.12b modifie aussi les TU `ga`, `ga_export_panel_by` (§6, `by`, preuve rouge M22), `hp`, `js`, `pj`, `lq`, sur des lignes disjointes de celles de D.12c (numéros déplacés : ⟲ de l'Annexe D) ; ne touche ni `fm`, ni `jb_ol`, ni `layout_nil_item_draws_nothing`, ni `planet_gh_row_lq` (texte inchangé), ni `mh` | Annexe D | `git diff --stat cd9247d <HEAD gradé> -- tests` |
| H-D12b-10 | `Makefile`, `tools/` inchangés ; aucun fichier ni TU ajouté ; règle de lien tenue ; `make e2e` : 26 conformes ; `tools/class_coverage_net_expected.txt` inchangé | « `Makefile` et liens », filets | note de D.12b |
| H-D12b-11 | D.12b ne touche ni `layout.c/.h`, ni `pi.c/.h`, ni `msgbox.c/.h`, ni `help.c`, ni `score_panel*.c`, ni `ui_button.c`, ni `cart.c`, ni `probe.h`, ni `tech_gate.h`, ni `pico8_value.h` | numéros non marqués de l'Annexe C | `git diff --stat cd9247d <HEAD gradé> -- <ces chemins>` vide |
| H-D12b-12 | La ligne D.12c du plan n'est pas modifiée par la note de D.12b ; les API de Q12 (`pico8_lq_bg`, `pico8_pj_ed`, `pico8_jc_ce`, `pico8_jy_qe`…) inchangées | « Lecture de ces textes », Out of Scope | lecture du plan gradé |
| H-D12b-13 | Le repli b2o/b2t de D.12b n'a pas été pris, ou, s'il l'a été, D.12b est entière (b2o + b2t) à la graduation | tout | note de D.12b |

### Sections à revérifier une fois D.12b gradée

- **Toujours** : « État de départ » (comptes réels, H-D12b-1) ; critères `FAILED:` de départ (HEAD, `liste` de départ, `src/` de départ) ; « Effet sur `make data-residue` » ; tous les numéros marqués ⟲ de l'Inventaire, des mutations et des Annexes C et D (remesure **par contenu**, jamais par recopie) ; Annexe A (ligne D.12b et sa note « Découvert à D.12b », à recopier ; ligne D.12c si la note l'a modifiée, H-D12b-12).
- **Si H-D12b-2 ou -3 tombe** (une aide à valeur survit avec d'autres appelants, ou meurt déjà ; noms `_ptrs` changés) : « Aides de `collect` », grep 2, M1–M3, glossaire.
- **Si H-D12b-4 tombe** (forme de `jo:gh` après b2) : « `jo:gh` », Annexe C (`planet_display.c`), M17–M19, empreinte de c2.
- **Si H-D12b-5 tombe** (une ligne `liste` de D.12b laissée, ou ajoutée ailleurs) : Annexe C (reclasser en c1/c2), « Effet sur `make data-residue` » ; une ligne L2 laissée n'entre **pas** dans D.12c sans être déclarée.
- **Si H-D12b-6 tombe** : « `jo.ex` et `hp:lq` », « `cb`, `bj` ».
- **Si H-D12b-7 tombe** : « Includes », liste des lignes `inc`, empreinte de c2.
- **Si H-D12b-8 tombe** : « Piège des étiquettes » (nom de la constante figée par TU).
- **Si H-D12b-9 ou -11 tombe** : Annexes C et D, « Empreinte et découpage ».
- **Si H-D12b-10 tombe** : filets, « `Makefile` et liens ».
- **Si H-D12b-13 tombe** (D.12b incomplète) : ne pas mettre ce PRD en file ; le reprendre.

### Lecture de ces textes pour D.12c

- **« `lq.items`, `mh.items`, `jb.ls` → union étiquetée locale à `layout` {rien, fixe, chaîne, rangée, autre-vrai} (Q10) »** : les cinq cas sont ceux que `lq:mn` distingue (`pico8/lua.lua:300-312` ; étude D.9, §3.1 : « L'union locale ne porte que les cinq cas que `lq:mn` distingue »). **Seul `lq.items` passe par `lq:mn`.** `mh.items` ne tient que des articles de `bl` (des `pico8_jb_row_t` avec `fn` : `score_panel_display.c:81-88`, `:325-336`, `pico8/lua.lua:597-600`, `:869`, `:885-899`) et `jb.ls` ne tient que des rangées `lq` (`jb:pq` : `lua.lua:314-315` ; `layout.c:259-269`) : une `lq` n'est aucun des cinq cas. Les ranger dans l'union ajouterait des étiquettes mortes, ou un sixième cas. **Tranché (G)** : `lq.items` prend l'union ; `mh.items` et `jb.ls` prennent la forme Q9 de leur élément unique, un tableau typé de pointeurs (`const pico8_jb_row_t *`, `pico8_lq_t *`). Déclaré à la note comme lecture de Q10 ; rejeté : une union à un seul cas vivant par membre.
- **« `lq.fs` en tableau typé de fixes »** : dans le cart, `self.fs` est la liste des `{fn=…, d=v(x,0)}` que `nv` rend (`lua.lua:289-293`), une entrée par élément mis en page, pas des largeurs (le glossaire le dit : « `fs` … sur une instance `lq`, une entrée par élément mis en page » ; l'étude disait « largeurs », à tort). En C, ce sont déjà des `pico8_lq_item_t` (genre `NONE`/`TEXT`/`SPRITE`, décalage `d`, `layout.c:15-29`). **Tranché (G)** : `lq.fs` devient un tableau typé de `pico8_lq_item_t` avec son compte ; « de fixes » est lu comme une imprécision de l'étude, déclarée.
- **« listes locales d'affichage (`planet_display`, `menu`, `msgbox:pz`, `trade:kg`, `research*`, `score_panel*`, `tech_gate`, `slipway`, `probe`, `galaxy`) »**, ajustée par les déplacements que D.12b déclare : **`trade:kg` et `galaxy` sortent** (pris par D.12b, H-D12b-5) ; **`slipway` et `probe` n'ont plus de liste locale** à `cd9247d` (seul `probe.h:8` porte un include) ; **`msgbox:pz` n'a plus de liste** (`msgbox.c:333-360` construit `ep_lq` en clés, tableau d'une valeur ; `ui.le` est passé à `pico8_ptrs_t` par D.12a) : il reste `mh.items` (`msgbox.c:121-124`), que la ligne range en L3 ; **entrent**, non nommés par la ligne mais du même genre : `help.c:85-93` (`fv:kj`), `layout.c` (`jb_line`, `lq:pq`), `pi.c` (`bl`), `ui_button.c:232-239` (branche liste de `js:pq`), `cart.c:25` (include).
- **« en tableaux fixes là où la borne est dans le cart »** : appliqué à chaque liste dont le cart borne la taille (table ci-dessous, borne citée) ; **ailleurs**, la ligne ne dit rien. **Tranché (G)** : une liste locale dont la taille est **connue au site avant le remplissage** mais que le cart ne borne pas (`jo.ex`, les rangées de `jo:gh` qui prennent `jy`, `jb.ls`, `lq.items`, `lq.fs`) devient un tableau typé alloué à cette taille ; aucune ne reste liste, aucune ne passe par `pico8_ptrs_t` quand ses éléments ne sont pas des pointeurs. Ce n'est pas l'arrêt « champ sans règle » : Q9 fixe la forme (tableau typé), la ligne ne fait que dire quand il est fixe.
- **« Empreinte ≈ 16 (15, 1) »** : estimation de l'étude (`2dec71f`). Remesurée à `cd9247d` : **20 fichiers de `src/`** (plus 0 à 9 lignes d'include que D.12b laissera, H-D12b-7) et **10 TU** (11 si `mh` doit suivre), plus `docs/plan-3.md`, `docs/glossaire-noms.md`. Le « 1 » de TU tombe pour la même raison qu'à D.12a/D.12b : dix TU lisent `lq.items`, `lq.fs`, `jb.ls`, `mh.items` ou les résultats de `cb`, et un pont serait une cale.
- **« Compte : `liste` = 0 dans `src/` hors `pico8_list.c/.h`, `pico8_ref_audit.h` et `pico8_type_tag.h` (49 lignes aujourd'hui, à D.12d) »** : les 49 lignes sont exactes à `cd9247d` (16 + 14 + 16 + 3). Une **cinquantième** ligne existe, que la ligne omet : `pico8_value.h:7`, le commentaire de tête de `pico8_value_t` (« a list element (pico8_list.h), an entity field (pico8_struct.h) »). Il reste **vrai** tant que `pico8_list.c` range des `pico8_value_t`, c'est-à-dire jusqu'à D.12d ; le réécrire en D.12c pour sortir du motif serait « commentaire compté réécrit hors de son sujet », interdit. **Tranché (G)** : il reste, laissé à D.12d, déclaré. Compte de sortie : **`liste` de `src/` = 50 lignes, exact** (les 49 + `pico8_value.h:7`).
- **« §6 : `layout_nil_item_draws_nothing` passe son item par l'union, même intention (nil et fixe 0), preuve rouge »** : appliqué en c2 (voir « §6 »). **Incomplet, déclaré** : les deux autres §6 lisent aussi l'état que D.12c change : `test_pico8_planet_gh_row_lq` lit `lq->items` (`:64`, `:67`) et `lq->fs->count` (`:164-165`), son commentaire de tête nomme « `pico8_list_t*` » (`:1`) ; `test_pico8_ga_export_panel_by` lit `lq->items` (`:53`, `:56`). Tous deux suivent **dans leur façon de lire**, preuve rouge refaite à chaque sous-étape qui les réécrit.
- **« `class-coverage-net` : seulement si le diff touche un constructeur »** : **oui aux deux sous-étapes** : c1 touche `pico8_mh_struct_new` (`mh.items`), `pico8_lq_struct_new` par `lq:pq` (`fs`) et `pico8_jb_struct_new` par `jb:pq` (`ls`) ; c2 touche `pico8_lq_struct_new` (`items`) et ajoute la copie `pico8_lq_copy`, qui passe par le même constructeur (un enregistrement « lq » par construction, comme aujourd'hui `keys_of` + `lq_new`).
- **« `sanitize-net` : oui »** : à chaque sous-étape (tas de `fs`, `ls`, `items` ; tableaux de `jo:gh`, `jo.ex` ; `mh.items` quitte le tas).
- **Règle des ponts de TU (l. 267)** : tranchée comme à D.12a/D.12b, **premier terme** : les TU suivent dans le commit qui change l'état qu'ils lisent ; aucun pont.
- **Remarque de clôture de D.11e** (`pico8_lq_bg`, `pico8_pj_ed` rendent un `pico8_value_t` pour les seuls TU) : `pico8_lq_bg` est dans `layout`, que D.12c réécrit ; **tranché (G) : hors D.12c**. Elle ne nomme aucune liste (`layout.c:315-320`), son retour à genres mêlés (nil, booléen, fixe) est exactement l'objet de Q12, ouverte ; la typer ici trancherait Q12 pour un cas. Les TU `ga`, `hp` continuent de l'appeler. `pico8_pj_ed` : idem.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json`, rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes, alias, registre (tableau, `switch` ou boucle qui passe d'un nom à un membre), renommage pour échapper aux greps, cast non vérifié, cale permanente ; mutations restaurées par `cp` simple ; binaires lancés exactement comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan : `test_pico8_planet_gh_row_lq`, `Makefile:563`) ; sous-étapes d'environ 50 min ; jamais l'appli raylib.

### État de départ

**Mesuré à `cd9247d`** (D.12b non commencée), sans build, par `sh tools/data_residue.sh` et la recette de D.12a/D.12b :

| Mesure | Valeur à `cd9247d` | Départ réel de D.12c (HEAD gradé de D.12b) |
|---|---|---|
| `liste` (motif de Q10) | **581 lignes / 86 fichiers / 625 occ.** (`src/` 371 / 49, `tests/` 210 / 37) | ≈ 355 (`src/` ≈ 200, `tests/` ≈ 155), **à mesurer** (H-D12b-1) |
| lignes `liste` de `src/` hors les 49 et `value.h:7` | 321, dont 183 classées b1/b2 par D.12b et 10 `inc` | ≈ 135–145 : les lignes c1/c2 de l'Annexe C (80 + 49 à `cd9247d`), plus les lignes ⟲ que D.12b ajoute dans `jo:gh` et les `inc` qu'il garde |
| `valeur`, `type-tag`, `ref-audit` | 1245 / 115, 271 / 62, 42 / 14 ; résidu D 2005 / 121 | rapportés par D.12b |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom` | inchangé |

Recette (inchangée depuis D.12a ; `sh tools/data_residue.sh --detail | grep -E '^liste'` donne les lignes, la recette reste pour les occurrences) :

```sh
P='\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'liste\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire (mesuré à `cd9247d` ; ⟲ = ligne que D.12b déplace ou réécrit, à remesurer par contenu)

#### Listes de l'étape et leur forme cible

| Liste | Site (fichier:ligne à `cd9247d`) | Éléments | Borne | Forme cible | Sous-étape |
|---|---|---|---|---|---|
| `pico8_cb` (résultat) | `tech_gate.c:115-128` ⟲, `.h:22-26` ; appelants `menu.c:184` ⟲, `:293` ⟲ | entrées de `nb` | **cart** : `cb` = `jp(nb, …)` (`lua.lua:1455-1462`) : ≤ `#nb` = `PICO8_NB_COUNT` (19, `map_tables.h:12`) | tableau fixe `const pico8_nb_entry_t *[PICO8_NB_COUNT]` + compte, rendu par valeur | c1 |
| `pico8_bj` (résultat) | `research_display.c:350-364` ⟲, `.h:30-34` ⟲ ; appelant `planet_display.c:393` ⟲ (`jo:fe`) | projets de `gb` | **cart** : `bj` = `jp(gb, …)` (`lua.lua:1391-1392`), `gb` rempli d'un `hp` par entrée de `fd` (`lua.lua:1434-1440`) : ≤ `PICO8_FD_COUNT` (42, `map_tables.h:10` ; `g_gb[PICO8_FD_COUNT]`, `research.c:17`) | tableau fixe `pico8_hp_t *[PICO8_FD_COUNT]` + compte, rendu par valeur | c1 |
| `mh.items` | `msgbox.h:34`, `msgbox.c:121-124` ; clés `msgbox.h:55-61` ; producteurs `score_panel_display.c:77-88`, `:319-336` ; lecteur `msgbox.c:76` | articles de `bl` | **cart** : deux `mh(…)` : `pj:hk` (`lua.lua:858-869`, `pj.ea` + `pp` : ≤ 3) et `pj:ph` (`nv(bz, …)`, `lua.lua:885-899` : `#bz` = `PICO8_BZ_COUNT` = 12, `game_data.h:22`) | tableau fixe `const pico8_jb_row_t *items[12]` (cap nommée) + compte, `assert` porteur de message ; clés `const pico8_jb_row_t *const *items` | c1 |
| `rb` de `bl` | `pi.c:177-195`, `pi.h:26-35` | articles | selon l'appelant | paramètres `(const pico8_jb_row_t *const *rb, int n)` | c1 |
| `items` de `fv:kj` | `help.c:85-93` | articles de `fv.rb` | **cart** : `fv.rb` (`lua.lua:1159-1165`) : 4 articles (`pico8_fv_rb_row`, `game_data.c:268`) | tableau fixe local de 4 (cap nommée, `assert`) | c1 |
| `combined` de `pj:kj` | `score_panel.c:250-265` | articles de `i_` puis de `ou` | **cart** : `bl(la(pj.i_, pj.ou))` (`lua.lua:853`) ; `i_` 5 articles (`lua.lua:830-836`), `ou` 7 (`lua.lua:837-845`) : 12 | tableau fixe local de 12 (cap nommée, `assert`) | c1 |
| `lq.fs` | `layout.h:35`, `layout.c:160-174` (`lq:pq`), `:199-222` (`lq:ol`) | enregistrements `{fn, d}` (`pico8_lq_item_t`) | = nombre de positionnels à la construction (`nv(self, …)`, `lua.lua:289-293`), non borné par le cart | tableau typé `pico8_lq_item_t *fs` + `int` compte, alloué une fois à la construction (TROU-7) ; `pico8_lq_item_t` passe dans `layout.h` | c1 |
| `jb.ls` | `layout.h:16`, `layout.c:259-269` (`jb:pq`), `:273-303` (`jb:ol`) | rangées `lq` | = nombre d'items de la rangée (`jb:pq`, `lua.lua:314-315`) | tableau typé `pico8_lq_t **ls` + compte, alloué une fois (TROU-7) ; NULL = nil | c1 |
| `lq.items` | `layout.h:26`, `layout.c:332-373` ; lecteurs `lq:mn` (`:69-131`), `lq:pq`, copie `keys_of` (`:392-406`) ; `hp:lq` ajoute (`research_display.c:295-298` ⟲) | chaîne, fixe, nil/faux, tuple (rangée), vrai | non borné (`jo.ex` : `#is` + jusqu'à 4 + Σ(`jq`+1)) | **union étiquetée** à cinq cas, tableau typé + compte, alloué à la construction, agrandi par `pico8_lq_add` (l'`add(l, …)` de `hp:lq`, `lua.lua:1409-1413`) | c2 |
| `items` de `jb_line` | `layout.c:229-257` | valeurs scalaires d'une ligne | **cart** (données de bulles `dr`, `oy` de `nb`, `hp:jb`) : la ligne la plus longue a 7 items (`nb_9_oy_1`, `map_tables.c:186`) | tableau fixe local (cap nommée, `assert`), passé en clés | c2 |
| rangées 2..n de `jo:gh` : `ji`, `merged` ×2, `lb` | `planet_display.c:80`, `:146`, `:190-191`, `:195-240` ⟲, `:246-277`, `:280-287` | chaînes d'en-tête, besoins (`jy`), tuples, écart `{false,1}` | `lb` : **cart** 3 + 1 (`lua.lua:442-448`) ; `merged` : 1 + `#jx` (ou 1, ou `#jy`) + 1 ; `ji` : 1 + Σ`gc` + 1 (non bornés par le cart, connus au site) | clés `pico8_lq_keys_t` locales (`lq_row`), tableaux de valeurs : `lb` fixe de 4 ; les autres alloués à leur taille ; `ctx->ji` collecte les besoins dans un `pico8_ptrs_t` | c2 |
| `l` de `jo.ex` | `research_display.c:142-266` ⟲ | tuples, chaînes, `jo.em` | connu à l'entrée : `#is` + 2 + 2 + `#ml`·(⌊max(`jq`,1)⌋+1) (`lua.lua:518-538`) | tableau de valeurs alloué à cette taille, `assert` de remplissage, libéré après `pico8_lq_new` (comme la liste aujourd'hui) | c2 |
| `lq1`, `lq2` de `pj:ph` | `score_panel_display.c:262-295` | une chaîne ; un tuple | **cart** : `lq=o(" again? ")`, `lq=o(o(10,7))` (`lua.lua:874`, `:878`) : 1 | clés `lq_row` + tableau fixe d'une valeur | c2 |
| branche liste de `js:pq` | `ui_button.c:216-241` | — | — | supprimée (plus aucun producteur de REF de liste) ; `lq` recopié par `pico8_lq_copy` | c2 |

#### API dont le type change (classe (b))

| API | Aujourd'hui | Après D.12c | Appelants (`src/`) | Sous-étape |
|---|---|---|---|---|
| `pico8_cb` | rend `pico8_list_t` (à libérer) | rend une struct par valeur `{const pico8_nb_entry_t *items[PICO8_NB_COUNT]; int count;}` (nom hors motif) | `menu.c:184`, `:293` ⟲ | c1 |
| `pico8_bj` | rend `pico8_list_t` | rend une struct par valeur `{pico8_hp_t *items[PICO8_FD_COUNT]; int count;}` | `planet_display.c:393` ⟲ | c1 |
| `pico8_cb_predicate`, `pico8_bj_predicate` (statiques) | `(pico8_value_t e, void *ctx)` | `(const pico8_nb_entry_t *t, …)`, `(pico8_hp_t *b, …)` | `tech_gate.c`, `research_display.c` | c1 |
| `pico8_cr_step` (statique, `menu.c:95`) | `(pico8_value_t e, …)` | `(const pico8_nb_entry_t *t, …)` | `menu.c:190` ⟲ | c1 |
| `pico8_bl`, `pico8_bl_default` ; `pico8_bl_dispatch_one` (statique) | `const pico8_list_t *rb` ; `pico8_value_t e_val` | `const pico8_jb_row_t *const *rb, int n` ; `const pico8_jb_row_t *e` | `msgbox.c:76`, `help.c:92`, `score_panel.c:263` | c1 |
| `pico8_mh_keys_t.items` | `const pico8_value_t *` | `const pico8_jb_row_t *const *` | `score_panel_display.c:85`, `:331` | c1 |
| `pico8_pj_ph_map` (statique, `score_panel_display.c:177`) | rend `pico8_value_t` | rend `const pico8_jb_row_t *` | `:328` | c1 |
| `pico8_lq_pq_step` (statique), `pico8_lq_mn` (statique) | mappeur de `nv` ; `pico8_value_t f` | boucle au site (plus de mappeur) ; `pico8_lq_mn` prend une case de l'union (c2) | `layout.c` | c1, c2 |
| `pico8_lq_keys_of` | rend des clés qui empruntent `row->items.items` | **retirée** ; remplacée par `pico8_lq_t *pico8_lq_copy(const pico8_lq_t *row)` (même contenu, même mise en page, une construction « lq ») | `ui_button.c:229` | c2 |
| `pico8_lq_add` (nouvelle) | — | `void pico8_lq_add(pico8_lq_t *self, pico8_value_t v)` : l'`add(l, v)` du cart sur une `lq` déjà mise en page (sans refaire `fs`), TROU-7 | `research_display.c:295`, `:297` ⟲ | c2 |
| `pico8_jo_gh_jp_step` (statique) | ajoute `pico8_value_ref(e)` à une `pico8_list_t` (H-D12b-4) | ajoute `e` à un `pico8_ptrs_t` | `planet_display.c` ⟲ | c2 |

`pico8_lq_keys_t.items` **garde** `const pico8_value_t *` (voir « L'union, locale à `layout` »).

#### Aides de `collect` (après D.12b, H-D12b-2)

| Aide à valeur | Appelants restants | En D.12c | Sort |
|---|---|---|---|
| `pico8_jp` (+ `pico8_jp_predicate_fn`) | `research_display.c:361` ⟲ (`bj`), `tech_gate.c:126` ⟲ (`cb`) | filtres écrits au site sur la table statique, dans l'ordre de `jp` (`lua.lua:31-36`) | **retirée en c1** |
| `pico8_nv` (+ `pico8_nv_fn`) | `layout.c:170` (`lq:pq`) | boucle au site sur les positionnels, dans l'ordre de `nv` (`lua.lua:37-41` ; le mappeur rend toujours un élément) | **retirée en c1** |

`collect.h` perd alors `#include "pico8_list.h"` (ligne ⟲, H-D12b-2) ; il garde `pico8_ptrs.h` et les aides `_ptrs`.

#### Sites de `src/` par sous-étape (détail ligne à ligne : Annexe C)

**c1** (≈ 80 lignes `liste`, 17 fichiers) — `collect.c/.h` (`jp`, `nv`, include) ; `tech_gate.c/.h` (`cb`) ; `research_display.c/.h` (`bj`, include de `.h`) ; `menu.c` (`cb_list` ×2, `pico8_cr_step`, include) ; `planet_display.c` (`jo:fe` : `bj_list`) ; `pi.c/.h` (`bl`) ; `msgbox.c/.h` (`mh.items`, clés) ; `help.c` (`fv:kj`) ; `score_panel.c` (`pj:kj`, include) ; `score_panel_display.c` (clés de `mh`, `pico8_pj_ph_map` ; aucune ligne `liste`) ; `layout.c/.h` (`fs`, `ls`, `lq:pq` sans `nv`, `lq:ol`, `jb:pq`, `jb:ol`, `pico8_lq_item_t` publié).

**c2** (49 lignes `liste` à `cd9247d`, plus ≈ 5 lignes ⟲ que D.12b écrit dans `jo:gh` et les 0 à 9 `inc` qu'il laisse ; 8 à 17 fichiers) — `layout.c/.h` (union, `items`, `jb_line`, `pico8_lq_copy`, `pico8_lq_add`, include) ; `ui_button.c` (branche liste) ; `planet_display.c` (`jo:gh`) ; `research_display.c` (`jo.ex`, `hp:lq`) ; `score_panel_display.c` (`lq1`, `lq2`, include) ; `cart.c:25`, `probe.h:8` (includes) ; les includes que D.12b laisse (H-D12b-7).

**Laissée à D.12d, déclarée** : `pico8_value.h:7`.

Hors de l'étape, vérifiés : `menu.c:139` (`pico8_menu_invented_lines`, b1) ; `msgbox.c:333-360` (`ep_lq`, clés sans liste) ; `help.c:40-62`, `menu.c:104-126` (le corps de `pico8_cr_step`, hors sa signature), `:196-210`, `:361-399`, `planet_display.c:139-185`, `:335-372`, `score_panel_display.c:222-242` (clés `pico8_lq_keys_t` à tableaux de valeurs, sans ligne `liste`, **inchangées** : voir « L'union, locale à `layout` ») ; `slipway.c`, `probe.c`, `galaxy.c`, `trade.c` (plus aucune liste locale après D.12b).

#### TU (détail : Annexe D)

| TU | Ce qu'il lit | c1 | c2 |
|---|---|---|---|
| `fm` | `pico8_cb` (`:88-90`) | oui | — |
| `js` | `pico8_cb` (`:875-889` ⟲, `:1704-1725` ⟲) ; `lq->items` (`:198`, `:203`, `:1490-1491`) ; `js:pq` par une liste (`:1302`, `:1370-1374`) | oui | oui |
| `pj` | `mh->items` (`:721-722`, `:747`) ; `lq->items` (`:111`, `:116`, `:725-726`) | oui | oui |
| `lq` | `fs` (`:180-181`) ; `bl` (`:530-555`) ; `lq->items` (`:78`, `:83`, `:698-699`, `:743-746`) ; clés à `nil`/`false` (`:373-389`, `:525`) | oui | oui |
| `ga` | `fs` (`:491`, `:511`) ; `lq->items` (`:473`, `:513`, `:516`) | oui | oui |
| `hp` | `fs` (`:282`, `:300`) ; `lq->items` (`:283-303`) | oui | oui |
| `planet_gh_row_lq` (§6) | `fs` (`:164-165`) ; `lq->items` (`:64`, `:67`) ; commentaire de tête (`:1`) | oui (preuve rouge) | oui (preuve rouge) |
| `jb_ol` | `ls` (`:91`) | oui | — |
| `ga_export_panel_by` (§6) | `lq->items` (`:53`, `:56`) | — | oui (preuve rouge) |
| `layout_nil_item_draws_nothing` (§6) | `line->items.count` (`:112`) ; item passé par les clés | — | oui (preuve rouge, ligne du plan) |
| `mh` | clés de `mh` (`:327-359`, sans `items`) | seulement si la compilation l'exige | — |

**Piège des étiquettes (vérifié à `cd9247d`)** : trois TU plient l'étiquette de liste de `lq.fs`, que `pico8_type_tag_of` lit (`PICO8_TYPE_TAG_LIST`, `0x9E5C1157u`) : `ga:511` par `fold_ref` (`0xEEEE`, étiquette) ; `hp:300` par `fold_value` à profondeur 3 (genre REF, étiquette, compte, puis chaque `pico8_lq_item_t` comme REF : étiquette `UNKNOWN`, rang dans `hv`, rang dans `gb`) ; `lq:181` et `jb_ol:91` l'assertent. Un tableau typé n'a pas d'étiquette : `pico8_type_tag_of` y lirait le premier mot d'un `pico8_lq_item_t` (son genre) et rendrait `UNKNOWN`. Voir « Les TU suivent ».

**Piège « rien » (vérifié à `cd9247d`)** : l'union range nil **et** faux dans le même cas « rien » (`lq:mn` ne les distingue pas : `lua.lua:307`, `elseif f`). Or `CONSTRUCTIONS_HASH` de `test_pico8_lq` plie le **genre** de chaque positionnel stocké (`fold_value`, `:88-120`), et ses rangées « false » (`:373`), « nil » (`:378`) et « mixed c » (`:384-389`, nil **et** faux) ne diffèrent que par là. Une TU qui plierait une case « rien » sans savoir d'où elle vient ferait rougir la constante sans bug. Voir « Les TU suivent ».

#### `Makefile` et liens

- Aucun fichier nouveau. **Aucune ligne du `Makefile` ne change en D.12c** (attendu ; toute modification est un écart à justifier).
- Règle de lien : aucun fichier de `DATA_ENGINE_SRCS` (`Makefile:239-290`) ne nomme un symbole défini hors de cette liste. Symboles nouveaux ou retypés : `pico8_lq_copy`, `pico8_lq_add` (`layout.c`, dans la liste ; appelés par `ui_button.c`, `research_display.c`, dans la liste) ; `pico8_cb`, `pico8_bj`, `pico8_bl` (`tech_gate.c`, `research_display.c`, `pi.c`, dans la liste ; appelés aussi par `menu.c`, `help.c`, `score_panel.c`, hors liste, ce qui est permis). `test_pico8_layout_nil_item_draws_nothing` se lie sur une liste explicite (`Makefile:574` : `collect.c`, `list.c`, `ptrs.c` compris) : `layout.c` n'y appelle plus `collect.c` après c1, rien à changer ; `test_pico8_planet_gh_row_lq` est lié sous ASan (`asan_objs`, `Makefile:563`).

## Solution

**Deux sous-étapes** (D.12c1, D.12c2), même PRD, chacune verte seule, vérifiée et poussée (modèle de D.11d, D.11e, D.12a, D.12b) :

1. **D.12c1 — tableaux typés, aides à valeur retirées** (≈ 27 chemins, ≈ 45 min), un commit : `cb`, `bj` en tableaux fixes rendus par valeur, `pico8_jp` retirée ; `bl`, `mh.items`, `fv:kj`, `pj:kj` en tableaux d'articles (fixes, bornés par le cart) ; `lq.fs` et `jb.ls` en tableaux typés, `lq:pq` sans `nv`, `pico8_nv` retirée ; les 9 TU qui les lisent (dont `planet_gh_row_lq`, §6, preuve rouge) ; `docs/glossaire-noms.md`.
2. **D.12c2 — union de `lq.items`** (≈ 28 chemins selon H-D12b-7, ≈ 55 min), un commit : union étiquetée à cinq cas pour `lq.items`, `pico8_lq_copy`, `pico8_lq_add` ; `jo:gh`, `jo.ex`, `pj:ph`, `jb_line` en tableaux et clés ; branche liste de `js:pq` retirée ; derniers includes de `src/` ; les 8 TU qui lisent `lq.items` (dont les trois §6, preuves rouges) ; glossaire ; note « Découvert à D.12c ». Repli écrit si le plafond est atteint (« Empreinte et découpage »).

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (dont `make data-guard`, inchangé) ; `make e2e` : **26 conformes** ; `make sanitize-net` propre ; balayage ASan+UBSan des TU propre ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (c1 et c2) ;
- `make data-residue` : `liste` de `src/` = **50 lignes exactement** ; `liste` total, `valeur`, `type-tag`, `ref-audit` rapportés ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/`, rien dans `tools/`, rien dans le `Makefile` ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  P='\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h'
  # 1. liste de src/ hors les quatre fichiers de D.12d : une seule ligne, src/pico8_value.h:7
  grep -nE "$P" src/*.[ch] | grep -vE '^src/pico8_(list\.[ch]|ref_audit\.h|type_tag\.h):'
  # 2. plus aucune aide à valeur dans collect, ni ses typedefs, nulle part : vide
  grep -nE '\bpico8_(jp|nv)\b|\bpico8_(jp_predicate|nv)_fn\b' src/*.[ch] tests/*.[ch]
  # 3. l'union : exactement cinq étiquettes déclarées, dans layout.h (nom choisi par l'implémenteur, donné au message)
  grep -cE '^\s*PICO8_LQ_CELL_[A-Z_]+,?\s*(/\*.*)?$' src/pico8_layout.h     # 5 (si le préfixe diffère : même contrôle sur le préfixe choisi)
  # 4. la copie et l'ajout existent, keys_of n'existe plus : 2 lignes, puis vide
  grep -nE '\bpico8_lq_(copy|add)\(' src/pico8_layout.h
  grep -rn 'pico8_lq_keys_of\b' src tests
  # 5. règle de lien : les TU liés sur DATA_ENGINE_SRCS seule, ou sur une liste explicite, se lient
  make tmp/test_pico8_jb_ol tmp/test_pico8_planet_gh_row_lq tmp/test_pico8_ga_export_panel_by \
       tmp/test_pico8_layout_nil_item_draws_nothing tmp/test_pico8_data_init tmp/test_pico8_map_tables \
       tmp/test_pico8_bg_kj tmp/test_pico8_planet_kj_dispatch
  # 6. aucune constante de TU changée : vide
  git diff <HEAD de départ> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 7. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^liste'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que les positionnels d'une rangée `lq` soient rangés dans une union étiquetée à cinq cas (rien, fixe, chaîne, rangée, autre-vrai), afin que `lq:mn` lise l'étiquette que la construction a posée au lieu de reclasser une valeur à genre libre.
2. En tant que mainteneur du port, je veux que l'union reste locale à `layout` et que les clés de construction gardent leurs valeurs, afin que les dix producteurs de clés ne changent pas et que Q12 reste ouverte.
3. En tant que mainteneur du port, je veux que `lq.fs` soit un tableau de `pico8_lq_item_t` et `jb.ls` un tableau de `pico8_lq_t *`, afin qu'aucune REF de liste ne soit plus déballée pour dessiner une rangée ou une bulle.
4. En tant que mainteneur du port, je veux que `mh.items` et `bl` prennent des tableaux d'articles typés, bornés par les deux constructions du cart, afin que la boîte de message ne tienne plus de valeurs.
5. En tant que mainteneur du port, je veux que `cb` et `bj` rendent un tableau fixe borné par `nb` et par `fd`, afin que leurs appelants n'aient plus de liste à libérer.
6. En tant que mainteneur du port, je veux que `pico8_jp` et `pico8_nv` à valeur disparaissent avec leur dernier appelant, afin que `collect` n'expose plus que des aides sur `pico8_ptrs_t`.
7. En tant que mainteneur du port, je veux que `jo:gh`, `jo.ex` et `pj:ph` passent leurs rangées en clés (`lq_row`) et que `js:pq` ne reconnaisse plus de liste, afin que `PICO8_REF_AS_LIST` n'ait plus d'appelant dans `src/`.
8. En tant que mainteneur du port, je veux que l'`add(l, …)` de `hp:lq` sur une `lq` déjà mise en page reste un ajout qui ne refait pas `fs`, afin que la rangée garde ses deux éléments non dessinés, comme dans le cart.
9. En tant que mainteneur du port, je veux que chaque tableau borné par le cart porte sa borne en constante nommée et un `assert` porteur de message, et que chaque tableau non borné soit alloué à une taille connue au site, afin qu'aucun débordement silencieux ne remplace une liste qui grandissait.
10. En tant que mainteneur du port, je veux que les durées de vie restent celles d'aujourd'hui (ce qui fuit fuit, TROU-7 ; ce qui est libéré l'est au même point), afin que `sanitize-net` et le comportement ne changent pas.
11. En tant que gardien des tests, je veux que les dix TU qui lisent cet état suivent dans le commit qui le change, constantes inchangées, afin qu'ils testent le vrai état.
12. En tant que gardien des tests, je veux que les constantes qui plient l'étiquette de `lq.fs` la reçoivent d'une constante figée du TU, et que les cases « rien » soient pliées du genre de la valeur d'où elles viennent, afin qu'aucune constante ne rougisse sans bug.
13. En tant que gardien des tests, je veux que `layout_nil_item_draws_nothing` fasse passer son item par l'union, nil et fixe 0 comme aujourd'hui, avec sa preuve rouge, afin que la garde du bug d'origine reste la même.
14. En tant que gardien des tests, je veux que `planet_gh_row_lq` et `ga_export_panel_by` ne changent que leur façon de lire la rangée, preuve rouge refaite, afin que leurs gardes restent les mêmes.
15. En tant que gardien des filets, je veux `make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net` et le balayage ASan+UBSan verts à chaque sous-étape, afin de prouver qu'aucune durée de vie, aucun ordre, aucun pixel et aucune instanciation ne change.
16. En tant que gardien des tests, je veux `gcov` avant/après et des mutations ciblées restaurées par `cp`, afin que chaque ligne ajoutée soit exécutée et qu'aucune ligne survivante ne cesse de l'être.
17. En tant que gardien du compteur, je veux au message de chaque commit les comptes par catégorie et par fichier en lignes et en occurrences, la classification, les lignes gagnées (« aucune » compris) et chaque Δlignes > Δocc justifié, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
18. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, qu'aucun alias, registre, renommage pour les greps, cast non vérifié ni cale permanente ne soit ajouté, et que le commentaire vrai de `pico8_value.h:7` reste, afin que « `liste` de `src/` = 50 » veuille dire « retiré ».
19. En tant que `plan-pilot`, je veux deux sous-étapes d'environ 45 et 55 minutes, un repli écrit pour la seconde, et une note « Découvert à D.12c » qui dise les comptes réels, les lectures de la ligne et ce que D.12d trouve, afin que la suite parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q9 et Q10 sont des décisions humaines que ce PRD applique ; Q12 reste ouverte et n'est pas tranchée ici.

### Modules

- **Mise en page** : `pico8_layout.c/.h` (union, `items`, `fs`, `ls`, `lq:pq`, `lq:mn`, `lq:ol`, `jb:pq`, `jb:ol`, `jb_line`, `pico8_lq_copy`, `pico8_lq_add`).
- **Articles et boîte de message** : `pico8_pi.c/.h` (`bl`), `pico8_msgbox.c/.h` (`mh.items`, clés), `pico8_help.c` (`fv:kj`), `pico8_score_panel.c` (`pj:kj`), `pico8_score_panel_display.c` (`pj:hk`, `pj:ph`).
- **Filtres** : `pico8_tech_gate.c/.h` (`cb`), `pico8_research_display.c/.h` (`bj`, `jo.ex`, `hp:lq`), `pico8_menu.c` (`cr`, `ix[2].hb`), `pico8_planet_display.c` (`jo:fe`, `jo:gh`).
- **Boutons** : `pico8_ui_button.c` (`js:pq`).
- **Aides** : `pico8_collect.c/.h` (`jp`, `nv` retirées).
- **Includes** : `pico8_cart.c`, `pico8_probe.h`, et ceux que D.12b laisse.
- **TU** : 10 (11 si `mh` doit suivre). **Docs** : `plan-3.md` (note), `glossaire-noms.md`.
- Inchangés : `Makefile`, `tools/`, `pico8_list.c/.h`, `pico8_ptrs.c/.h`, `pico8_ref_audit.h`, `pico8_type_tag.h`, `pico8_value.h`, `test_pico8_list`, `test_pico8_ptrs`, les producteurs de clés `pico8_lq_keys_t` (`help`, `menu`, `msgbox:pz`, `planet_display` hors `jo:gh`, `score_panel_display` hors `lq1`/`lq2`).

### L'union, locale à `layout` (G)

```c
/* Les cinq cas que `lq:mn` distingue ; nil et false sont « rien ». */
typedef enum { PICO8_LQ_CELL_NOTHING, PICO8_LQ_CELL_FIXED, PICO8_LQ_CELL_STRING,
               PICO8_LQ_CELL_ROW, PICO8_LQ_CELL_TRUE } pico8_lq_cell_kind_t;
typedef struct {
    pico8_lq_cell_kind_t kind;
    union { pico8_fixed_t fixed; const char *s; const pico8_jb_row_t *row; } as;
} pico8_lq_cell_t;
```

(Forme de décision, pas code à recopier ; noms proposés, hors de tout motif du compteur ; l'implémenteur peut en choisir d'autres, donnés au message.)

- **Où** : déclarée dans `layout.h` (le membre `lq.items` est public et lu par les TU) ; **seul `layout.c` en construit**. Les clés `pico8_lq_keys_t` **gardent** `const pico8_value_t *items` : ce sont les tables `{…}` du cart, que Q12 typera ou non ; le classement a lieu **une fois, à la construction**, dans une fonction statique de `layout.c` (nom proposé `pico8_lq_cell_of`), comme `lq:pq` fait déjà cuire `fs` à la construction. Correspondance, **exactement** celle que `lq:mn` applique aujourd'hui (`layout.c:69-131`) : `FIXED` → fixe ; `STRING` non NULL → chaîne ; `REF` non NULL → rangée (lue comme `pico8_jb_row_t` sans interroger son étiquette, comme aujourd'hui, `:76-82`) ; `REF` NULL et `BOOL` faux → rien ; `BOOL` vrai et `STRING` NULL → autre-vrai.
- **Pourquoi pas des clés en union** (rejeté) : dix producteurs de clés et quatre TU (`lq`, `js`, `layout_nil`, `pj`) changeraient, sans une ligne `liste` de plus retirée ; ce serait typer une part de `valeur` (Q12). Rejeté aussi : garder `lq.items` en valeurs dans un tableau typé (la ligne et Q10 demandent l'union).
- **`lq:mn`** prend une case ; pour une rangée, ses `f[1..4]` (items de `pico8_jb_row_t`) passent par `pico8_jb_item_value` puis par le même classement (ou une variante sur `pico8_jb_item_t`, sans logique dupliquée) ; le reste de la fonction est inchangé (largeur, `qw`, `qv`, genre dessiné).
- **Stockage** : `pico8_lq_cell_t *items` + `int` compte (nom au choix, hors motif ; `items_count` proposé), alloué à la construction à `max(count, 1)` cases (précédent : `calloc(bj_list.count > 0 ? … : 1, …)`, `planet_display.c:394`), jamais libéré (TROU-7, comme aujourd'hui). `pico8_lq_add` agrandit par `realloc` et range la case classée ; il ne refait pas `fs` (le cart n'appelle pas `pq` après `add`).
- **`pico8_lq_copy`** (remplace `pico8_lq_keys_of` + `pico8_lq_new` dans `js:pq`) : nouvelle `lq` qui copie les cases, `c`, `bg`, `score`, `ra` de la source, puis `lq:pq` ; **même** instanciation « lq » que l'appel d'aujourd'hui (une par copie) : elle passe par `pico8_lq_struct_new` ou par sa partie commune, jamais par un second enregistrement de couverture.
- **Contrat écrit** (commentaire de contrat de `layout.h`) : l'union ne distingue pas nil et false ; `pico8_lq_add` ne remet pas `fs` à jour ; `items` et `fs` ne sont jamais libérés (TROU-7).

### `lq.fs`, `jb.ls` (G, c1)

- `pico8_lq_item_t` (et son enum) passe de `layout.c` à `layout.h`, commentaire « Private to this file » réécrit ; `lq.fs` devient `pico8_lq_item_t *fs` + compte (`fs_count` proposé), alloué une fois à `max(n, 1)` éléments dans `lq:pq` (TROU-7 ; aujourd'hui un `malloc` par élément et une liste de tas, jamais libérés) ; `fs` NULL garde son sens « pas encore mis en page » (`lq_struct_new` le met à NULL avant `pq`).
- `lq:pq` : la boucle de `nv` est écrite au site, sur les positionnels dans l'ordre, un élément par positionnel (le mappeur rend toujours 1 : `layout.c:141-156`) ; le contexte `pico8_lq_pq_ctx_t` peut rester ou devenir des locales. En c1 elle lit encore `self->items.items[i]` (valeur) ; en c2, la case.
- `lq:ol` parcourt `fs[0..fs_count)` ; plus de test de genre (`entry.kind == REF`) : chaque élément est un enregistrement.
- `jb.ls` devient `pico8_lq_t **ls` + compte (`ls_count` proposé), alloué une fois à `max(n, 1)` dans `jb:pq` (TROU-7). **Piège (H6)** : `jb_new(NULL)` doit garder un `ls` **non NULL** de compte 0, car `jb:ol` dessine alors le cadre (h = 2) et `test_pico8_jb_ol` le fige (`NO_OVERRIDES_HASH`, `:78`) ; un `malloc(0)` qui rendrait NULL ferait sauter le cadre. D'où `max(n, 1)`. `jb:ol` lit `#self` comme le compte (commentaire `layout.c:271-272` gardé vrai).

### Articles : `bl`, `mh.items`, `fv:kj`, `pj:kj`, `pj:hk`, `pj:ph` (G, c1)

- `pico8_bl(gfx, mem, const pico8_jb_row_t *const *rb, int n, pico8_fixed_t nc)` et `pico8_bl_default(gfx, mem, rb, n)` ; `pico8_bl_dispatch_one` prend l'article ; la boucle reste une boucle sur les articles dans l'ordre, `camera`/`nc = -nc` avant chaque article (TROU inchangé). Le commentaire de contrat de `pi.h:26-33` (« Every element of `rb` must be a PICO8_VALUE_REF… ») est réécrit : le type le dit.
- `mh.items` : tableau fixe `const pico8_jb_row_t *items[PICO8_MH_ITEMS_CAP]` (nom proposé) de **12** (= `PICO8_BZ_COUNT`, `pj:ph` ; `pj:hk` en passe 2 ou 3) + compte, `assert(keys->count <= CAP && "…")` porteur de message dans le constructeur (« deux `mh()` dans le cart : `pj:hk` (≤ 3 articles) et `pj:ph` (un par rangée de `bz`) ; un troisième demande un tableau plus grand »). Le tableau quitte le tas (`sanitize-net`).
- `pico8_mh_keys_t.items` : `const pico8_jb_row_t *const *items` ; `score_panel_display.c` : `pj:hk` remplit un tableau de 3 articles, `pj:ph` un tableau `iw[PICO8_BZ_COUNT]` ; `pico8_pj_ph_map` rend l'article (`&st->row` ou `&wrapped->row`), sans `pico8_value_ref`.
- `fv:kj` (`help.c`) : tableau local de 4 articles (cap nommée, `assert` : « `fv.rb` a quatre articles dans le cart »), rempli de `self->rb->items[i].row`.
- `pj:kj` (`score_panel.c`) : tableau local de 12 (cap nommée, `assert` : « `la(pj.i_, pj.ou)` : 5 + 7 articles dans le cart »), `i_` puis `ou`, comme `la`.

### `cb`, `bj`, et la fin de `pico8_jp`/`pico8_nv` (G, c1)

- `pico8_cb(bi)` rend par valeur une struct `{const pico8_nb_entry_t *items[PICO8_NB_COUNT]; int count;}` (nom hors motif, `pico8_cb_kept_t` proposé) ; le filtre de `jp` est écrit au site : pour chaque entrée de `pico8_nb_table` **dans l'ordre**, le prédicat (qui écrit `e_` pour **chaque** entrée visitée, effet de bord réel, `tech_gate.h:22-25`), puis l'ajout si vrai. Aucune libération chez l'appelant. `menu.c` : `cr` et `ix[2].hb` lisent `.count`/`.items[i]` ; `pico8_cr_step` prend l'entrée typée ; le test `kind == REF && as.ref != NULL` (`menu.c:189`) disparaît (chaque élément est une entrée).
- `pico8_bj(jo)` : de même sur `pico8_gb_at(1..pico8_gb_count())`, struct `{pico8_hp_t *items[PICO8_FD_COUNT]; int count;}` (`pico8_bj_kept_t` proposé) ; `jo:fe` (`planet_display.c:392-402`) parcourt les projets gardés, `calloc` de `bs`/`rows` inchangé.
- **Équivalence déclarée** : `jp` parcourt par `all()` avec tolérance à la suppression (TROU-5) ; ici la source est une table statique que le prédicat ne modifie pas : un `for` donne le même ordre et les mêmes éléments. Rejeté : construire un `pico8_ptrs_t` de la table pour appeler `pico8_jp_ptrs` (tas pour rien, et le résultat reste une liste à libérer).
- **Retirées en c1** : `pico8_jp`, `pico8_jp_predicate_fn`, `pico8_nv`, `pico8_nv_fn`, leurs commentaires de contrat, et `#include "pico8_list.h"` de `collect.h` (s'il ne casse rien, règle des includes).
- **Noms `_ptrs`** (laissés à « D.12c ou D.12d » par D.12b) : **gardés** (G). Renommer `pico8_jp_ptrs` en `pico8_jp` une fois la version à valeur morte serait un renommage pur (classe (c)), sans donnée retirée ; il appartient à la phase 3 si le glossaire le justifie. La ligne du glossaire des aides `_ptrs` est réécrite pour rester vraie (plus aucune version à valeur).

### `jo:gh` (G, c2)

État supposé après D.12b : H-D12b-4.

- Chaque rangée 2..n reçoit des clés locales : un tableau `pico8_lq_keys_t row_keys[PICO8_JO_GH_ROWS]` dans `pico8_jo_gh` (le `bs` y pointe par `lq_row`, `lq` nil), et un tableau de valeurs par rangée. `bs[i].lq = pico8_value_ref(liste)` disparaît.
- `ctx->ji` devient un `pico8_ptrs_t` qui **collecte les besoins** (`pico8_jo_gh_jp_step` y ajoute `e` autant de fois que `gc`, comme aujourd'hui) ; la rangée « receives: » est alors écrite dans un tableau de `1 + ji.count + 1` valeurs (en-tête, `pico8_value_ref(e)` par besoin, écart) ; le collecteur est libéré dans tous les cas ; le tableau suit la durée de vie de la liste d'aujourd'hui (fuit si la rangée est gardée, TROU-7, marqué ; n'existe pas sinon).
- Les deux `merged` : tableau de `1 + n + 1` valeurs (`n` = `jx.count`, ou 1 pour `{jy[#jy]}`, ou `jy->count`), en-tête puis besoins dans l'ordre de `la`, puis l'écart ; fuit comme aujourd'hui (TROU-7, marqué).
- `lb` : tableau de **taille fixe 4** (borne du cart : trois éléments `lua.lua:442-445` + l'écart `:447-448`), alloué comme la liste d'aujourd'hui et, comme elle, jamais libéré (TROU-7, marqué) : même durée de vie.
- La boucle des écarts (`planet_display.c:280-287`) reste une boucle sur les rangées 2..n et ajoute l'écart en dernière case réservée de chaque tableau (plus de `PICO8_REF_AS_LIST`).
- Rejetés : des tableaux locaux sur la pile (changerait la durée de vie des rangées que `pz` garde — `pz` les recopie, mais TROU-7 dit que ces tables fuient, et le corriger est interdit, §7) ; une union dans les clés (voir plus haut).

### `jo.ex` et `hp:lq` (G, c2)

- `jo.ex` : taille connue à l'entrée (`#is`, `jq` après la branche `jq > 5`, `#ml`) ; tableau de valeurs alloué à `#is + 2 + 2 + #ml·(⌊max(jq',1)⌋ + 1)`, rempli dans l'ordre d'aujourd'hui, `assert` porteur de message sur le remplissage ; clés `items`/`count` ; libéré après `pico8_lq_new` comme la liste aujourd'hui (`research_display.c:264`). Les tuples gardent leur `calloc` (TROU-7).
- `hp:lq` : `pico8_lq_add(l, pico8_value_ref((void *)pico8_hp_fk()))` puis, si `px`, `pico8_lq_add(l, pico8_value_string(self->px))` (le `(void *)` de qualificatif existe déjà à cette ligne, `:295`).

### `js:pq` (G, c2)

`pico8_js_lq` : `row` → `pico8_lq_new(row)` ; nil ou non-REF → `pico8_lq_new(NULL)` ; `lq` → `pico8_lq_copy(src)` ; toute autre REF → `assert` porteur de message (« js:pq's lq()/h_() argument: a REF is an lq; a row comes as keys »). Plus aucun producteur de `src/` ne passe une liste après c2 (`jo:gh`, `pj:ph` passent des clés). Le commentaire de tête (`ui_button.c:216-218`) et celui de `pico8_js_keys_t` (`ui_button.h:63-67`, « else a list, an `lq` row… ») sont réécrits pour rester vrais.

### `jb_line`, `pj:ph` (G, c2)

- `jb_line` : tableau fixe local de valeurs, cap nommée = 7 (plus longue ligne de bulle des données du cart, `nb_9_oy_1`, `map_tables.c:186` ; `assert` porteur de message) ; le reste inchangé.
- `pj:ph` : `lq1`, `lq2` deviennent des clés `pico8_lq_keys_t` locales à tableau d'une valeur, passées par `lq_row` (`lq` nil) ; libérées au même point (fin de bloc).

### Includes (G)

Règle de D.12a/D.12b : un fichier dont plus rien ne nomme `pico8_list_*` perd `#include "pico8_list.h"` **dans la sous-étape où part sa dernière ligne**, sauf si un dépendant s'en sert par transitivité (alors gardé jusqu'à ce que ce dépendant perde la sienne, au plus tard en c2) ; chaque include retiré ou gardé est listé au message. **À la fin de c2, aucun fichier de `src/` hors les quatre de D.12d n'inclut `pico8_list.h`** (grep 1). Un **TU** qui s'en servait par transitivité gagne un `#include "../src/pico8_list.h"` direct (ligne `liste` ajoutée dans `tests/`, déclarée, retirée par D.12d) ; aucun TU ne perd son include direct en D.12c (D.12d).

### Les TU suivent (G, règle des ponts, l. 267)

- **Tranché** : chaque TU qui lit un état changé suit dans le commit qui le change ; aucun pont.
- **Lectures** : `lq->items.items[i]` → la case (`.kind`, `.as.*`) ; `lq->items.count` → le compte ; `lq->fs->count` → `fs_count` ; `jb->ls->count` → `ls_count` ; `mh->items.items[k].as.ref` → `mh->items[k]` ; résultats de `pico8_cb` : `.count`, `.items[k] == t`, plus de `pico8_list_free`.
- **Aides locales de lecture** (`row_count`/`row_item` de `js:198-203`, `pj:111-116`, `lq:78-83` ; `lq_item`/`lq_index_string` des deux §6 ; `fold_lq_positional` de `ga` ; `fold_lq_item` de `hp`) : retypées en place (une case au lieu d'une valeur), chaque appel édité une fois.
- **Piège des étiquettes (c1)** : là où une constante plie l'étiquette `PICO8_TYPE_TAG_LIST` de `lq.fs`, le TU plie une constante figée : `hp` réutilise son `VIEW_LIST_TAG` (`0x9E5C1157u`, déjà là, H-D12b-8), `ga` reçoit `FROZEN_LIST_TAG 0x9E5C1157u` (précédent exact : D.12b, `FROZEN_INSTANCE_TAG` de D.11d3). `hp` plie ensuite le compte puis chaque élément comme la REF qu'il était (`pico8_value_ref((void *)&lq->fs[i])` à profondeur 2 : genre REF, étiquette `UNKNOWN`, rangs « absent » — mêmes octets). Les assertions `lq:181`, `jb_ol:91` perdent leur moitié `PICO8_TYPE_TAG_LIST` (le type est vérifié à la compilation) et gardent le reste (`fs != NULL`, `ls != NULL && ls_count == 0`). Rejetés : réécrire une constante (interdit) ; nommer `PICO8_TYPE_TAG_LIST` dans un TU pour un tableau (faux).
- **Piège « rien » (c2)** : une case « rien » est pliée **du genre de la valeur d'où elle vient**. (1) Dans les rangées que le TU construit lui-même à partir de clés (`test_pico8_lq` : `fold_built_from`, `test_calls` ; `js` : `fold_keys`, les clés `:766-778`), le TU plie le genre de **sa clé** (`k->items[i]`) après avoir asserté que la case stockée a l'étiquette attendue (rien pour nil/faux, fixe, chaîne, rangée) et plie le contenu **lu dans la case** ; l'état est toujours lu, la clé ne fournit que le mot de genre que l'union a fondu. (2) Partout ailleurs (états construits par `src/` : `pj:ph`, `jo:gh`, `jo.ex`, `hp:lq`), une case « rien » est pliée comme `false` : à `cd9247d`, le seul producteur `src/` d'un positionnel nil au premier niveau est `pico8_jb_item_value` sur un `is_ref` NULL, qui n'apparaît que **dans** un tuple (`research_display.c:193-195`), jamais au premier niveau ; les faux de premier niveau viennent de `bz` (`game_data.c:112`) par `pico8_pj_ph_item_value`. **Mesure préalable M0 (obligatoire, avant toute conversion de c2)** : instrumenter temporairement les plis de positionnels des TU `lq`, `js`, `pj`, `hp`, `ga` (copie `cp`, restaurée par `cp`) pour compter, par constante, les positionnels nil et faux de premier niveau pliés ; consigner au tableau « Runs ». Si une constante de la famille (2) plie un nil, ou si une même rangée de la famille (2) mêle nil et faux, **arrêt** (conflit entre Q10 et une constante figée : « champ sans règle », §7), rapporté à `plan-pilot`.
- **Branche liste de `js:pq` (c2)** : le cas « a list as `lq(x)`'s argument » de `fold_keys` (`js:1302`, `:1370-1374`) devient le même contenu passé par des clés (`lq_row`, deux valeurs « from a list », 3) ; `fold_js_members` plie la `js` construite, identique : constante `JS_MEMBERS_HASH` inchangée ; le commentaire de tête de la fonction (`:1297-1298`, « a row and a list as `lq(x)`'s argument ») est réécrit. Cas `stops_on` (`:1641-1660`) : les valeurs hors rangée (vecteur, `jb`, `js`) s'arrêtent toujours sur l'`assert` réécrit ; inchangés.
- **`test_pico8_lq` (c1)** : `test_calls` passe son article à `pico8_bl_default` par un tableau d'un article (`:530-555`) ; `fs_count` (`:179-183`). **(c2)** : `positionals` garde ses tableaux de valeurs (les clés ne changent pas) ; `receives`/`wants` (`:743-746`) comparent `.as.row` à `&a->row`.
- **Constantes** : aucune `*_HASH`/`identity[]` ne change (mêmes éléments, même ordre, mêmes comptes, mêmes pixels). Une constante qui rougit est un bug de conversion (ou l'un des deux pièges oublié).
- **Assertion nouvelle** : admise seulement pour couvrir une ligne ajoutée non exécutée ou pour faire rougir une mutation qui ne rougit rien (trou), dans un test gardé du module, listée au message avec la ligne qu'elle couvre ; rien de lent, aucun fichier d'événements.

### §6 (G)

- **`layout_nil_item_draws_nothing` (c2, ligne du plan)** : `render_one_item` passe toujours son item par les clés (`keys.items = &item`) ; **il passe donc par l'union** : l'`assert` `line->items.count == 1` (`:112`) lit le compte et **asserte en plus l'étiquette stockée** (« rien » pour nil, « fixe » de valeur 0 pour le contrôle) : même intention (nil ne dessine rien ; fixe 0 dessine le sprite 0 là où la mise en page le place), mêmes assertions de pixels. Le commentaire de tête (`:1-21`) reste vrai ; sa phrase « the REF(NULL) clause » de `layout.c` est réécrite pour nommer le classement de l'union si elle devient fausse. **Preuve rouge** (M20) : le classement range `REF` NULL en « autre-vrai » (le bug d'origine : nil vrai, sprite 0) → rouge ; restauré, vert. Le piège décrit au commentaire de `layout.c:34-37` (« La clause REF-NULL n'est pas optionnelle ») est gardé, déplacé sur le classement.
- **`planet_gh_row_lq`** : **c1** : `fs->count == 4` → `fs_count == 4` (`:164-165`) ; preuve rouge (M9 bis) : `lq:pq` met en page un élément de trop (`fs_count` = n + 1) → rouge. **c2** : `lq_item` (`:62-69`) lit la case ; les assertions « exports: », « 0/1 », tuple `{qe=99}` (`:151-157`), écart, « pas de cinquième » deviennent des lectures d'étiquette et de contenu, **mêmes valeurs attendues** ; son commentaire de tête (`:1-16`, « holds a `pico8_list_t*` », « copy loop ») est réécrit pour dire ce qu'il garde encore : la rangée d'exports reçoit ses quatre items, dans l'ordre, sans fantôme, une entrée de `fs` par item. Preuve rouge (M17) : la copie des clés dans `lq.items` prend un élément de trop → « pas de cinquième » rouge.
- **`ga_export_panel_by` (c2)** : `lq_index_string` (`:50-58`) lit la case ; chaînes attendues (« 0/2 », « 2/2 ») inchangées. Preuve rouge : celle de D.12b (M22 de son PRD : le constructeur de `ga` n'installe plus `g_ga_by_shared`), refaite.

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la liste est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours pour échapper au motif (dont `pico8_value.h:7`) ; alias (macro, `typedef` de `pico8_list_t`, enveloppe qui renomme une aide) ; registre ; cast d'un élément vers une struct précise sans son test ; cast de pointeur de fonction ; cast autre que les `(void *)` de qualificatif **déjà présents** aux lignes touchées ; cale valeur ↔ conteneur (fonction de conversion d'une liste en tableau, double tenu en parallèle, instantané) ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; tout changement d'une constante de hachage ; toute modification de `tests/e2e/`, d'un `*.sha256`, de `tools/`, du `Makefile`. Le classement valeur → case de `layout.c` n'est **pas** une cale : il n'existe qu'à la construction d'une `lq`, remplace le classement que `lq:mn` refaisait à chaque dessin, et ne rend jamais de valeur.
- **Classification exigée**, au message de chaque commit, pour chaque ligne `liste` retirée ou modifiée (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) :
  - **(a1) conteneur converti** : la même opération (déclaration, init, ajout, parcours, type de membre ou de paramètre) sur un tableau typé, fixe ou alloué, ou sur l'union ;
  - **(a2) supprimée sans remplaçant** : include devenu inutile, assertion d'étiquette sur un tableau typé, liste locale devenue inutile, libération d'une liste devenue tableau par valeur, aide à valeur retirée, branche liste de `js:pq` ;
  - **(b) signature dont le type de liste change** : celles de la table « API … (classe (b)) », et elles seules ; plus les aides locales de TU ;
  - **(c) même donnée, autre nom** : **attendu 0**.
- **Mesure exigée**, avant et après chaque commit, par catégorie (`valeur`, `type-tag`, `ref-audit`, `liste` ; `par-nom` = 0 constaté) et **par fichier** du diff : lignes (`--detail`) **et** occurrences (recette de « État de départ »).
- **Lignes qui gagnent une catégorie**, par fichier et par catégorie, « aucune » compris. Attendues : `valeur` : `pico8_value_ref(e)` des besoins dans les tableaux de `jo:gh` (c2 ; ils y étaient déjà, dans des `pico8_list_add`) ; les tableaux de valeurs de `jb_line`, `jo.ex`, `pj:ph` ; le classement de `layout.c` (lit une valeur) ; `liste` : les `#include "../src/pico8_list.h"` ajoutés à un TU (règle des includes) ; `type-tag` : aucune. Toute autre est à justifier.
- **Lignes comptées modifiées, non retirées** (fichier:ligne) : listées (attendu : aucune dans `src/` à la fin de c2 hors les quatre fichiers et `value.h:7`).
- **Δlignes > Δoccurrences** : chaque couple justifié (attendu : Δocc ≥ Δlignes).
- **Numéros de ligne** : ceux du **parent du commit**, jamais ceux de ce PRD sans remesure.

### Empreinte et découpage (G)

| Sous-étape | `src/` | `tests/` | autres | Chemins | Coût estimé |
|---|---|---|---|---|---|
| **c1** | `collect.c`, `collect.h`, `tech_gate.c`, `tech_gate.h`, `research_display.c`, `research_display.h`, `menu.c`, `planet_display.c`, `pi.c`, `pi.h`, `msgbox.c`, `msgbox.h`, `help.c`, `score_panel.c`, `score_panel_display.c`, `layout.c`, `layout.h` (17) | `fm`, `js`, `pj`, `lq`, `ga`, `hp`, `planet_gh_row_lq` (§6), `jb_ol` (8 ; `mh` seulement si la compilation l'exige) | `glossaire-noms.md` | 26–27 | ≈ 45 min |
| **c2** | `layout.c`, `layout.h`, `ui_button.c`, `planet_display.c`, `research_display.c`, `score_panel_display.c`, `cart.c`, `probe.h` (8) + les fichiers dont D.12b garde l'include (0 à 9, H-D12b-7) | `lq`, `js`, `pj`, `hp`, `ga`, `ga_export_panel_by` (§6), `planet_gh_row_lq` (§6), `layout_nil_item_draws_nothing` (§6) (8) + les TU qui gagnent un include direct | `glossaire-noms.md`, `plan-3.md` | 18–30 | ≈ 55 min |

- **Pourquoi ce découpage** : l'étape entière fait ≈ 45–55 chemins (> ~40) et ≈ 1 h 40 : deux sous-étapes d'environ 50 min. La coupure suit le graphe : c1 ne touche aucune valeur de `lq.items` (tableaux d'éléments d'un seul type : entrées, projets, articles, enregistrements, rangées `lq`) ; c2 est l'union et tout ce qui fournit des positionnels à une `lq` par une liste. Les deux aides à valeur meurent en c1 (`nv` part avec `fs`). Recouvrement : `layout`, `planet_display`, `research_display`, `score_panel_display` et six TU sont édités dans les deux sous-étapes, sur des lignes disjointes (sauf la boucle de `lq:pq`, écrite en c1 sur la valeur et retouchée en c2 pour la case, déclaré).
- Rythme mesuré : D.11c ≈ 23 min pour 37 chemins ; D.11d1..d4 ≈ 21 à 27 min ; D.12a (a1, a2) et D.12b1 sans repli. c2 est plus conceptuelle (union, deux pièges de TU, trois §6) : estimation haute.
- **Repli écrit** (G) : si c2 atteint son plafond (85 min) sans être finie, rien n'est commis ; `plan-pilot` relance c2 en deux : **c2r** — rangées en clés : `jo:gh`, `jo.ex`, `pj:ph` passent leurs tableaux de **valeurs** par `lq_row`, branche liste de `js:pq` retirée, `pico8_lq_keys_of` gardé ; TU `js` (cas « from a list »), `lq` (`receives`/`wants` inchangés), `planet_gh_row_lq` (§6, preuve rouge) ; puis **c2u** — l'union : `lq.items`, `pico8_lq_copy`, `pico8_lq_add` (`hp:lq` y passe), `jb_line`, includes ; TU `lq`, `js`, `pj`, `hp`, `ga`, les trois §6. Coût du repli : `hp:lq` et les lectures de `planet_gh_row_lq` sont éditées deux fois, déclaré. Rejeté comme plan principal : trois passes sur `lq` et `js`.
- Rejetés : une seule sous-étape (≈ 1 h 40) ; une sous-étape par liste (≈ 12, `lq` et `js` édités cinq fois).

### Effet sur `make data-residue`

| Catégorie | Départ (après D.12b, H-D12b-1) | Après c1 | Après c2 | Pourquoi |
|---|---|---|---|---|
| `liste` `src/` | ≈ 200, à mesurer | ≈ départ − 80 | **50, exact** | Annexe C : c1 ≈ 80 lignes (collect 19, tech_gate 9, research_display 9, menu 5, planet_display 2, pi 9, msgbox 4, help 4, score_panel 6, layout 13) ; c2 : 49 + ≈ 5 lignes ⟲ de `jo:gh` + les `inc` laissés (0 à 9) ; reste : `pico8_list.c/.h`, `ref_audit.h`, `type_tag.h` (49) et `value.h:7` |
| `liste` `tests/` | ≈ 155, à mesurer | ≈ départ − 12 | ≈ départ − 17, plus les includes ajoutés, rapporté | c1 : `fm` 2, `js` 4, `lq` 5, `jb_ol` 1 ; c2 : `js` 4, `planet_gh_row_lq` 1 (Annexe D, colonne `L`) ; le reste est D.12d |
| `valeur` | rapportée par D.12b | baisse (tests de genre de `jo:fe`, `cr`, `bl`, `lq:ol`, `jb:ol` ; prédicats), rapportée | baisse (`lq:mn` sur des cases, `js:pq`), lignes gagnées déclarées | — |
| `type-tag` | rapportée | baisse (assertions de TU) | baisse (`ui_button.c:232`) | — |
| `ref-audit` | rapportée | inchangé | baisse (`planet_display.c:286`, `ui_button.c:235`) | `PICO8_REF_AS_LIST` n'a plus d'appelant dans `src/` |
| `DATA_RESIDUE_GUARD` | `…,dispatch,par-nom` | inchangé | inchangé | `liste` ≠ 0 (D.12d) |

« ≈ » est une estimation de ce PRD, jamais un critère ; « exact » est un critère ; « rapporté » est mesuré au commit, écart expliqué.

### Documentation (G)

- `docs/plan-3.md` (c2) : note « Découvert à D.12c » en fin de ligne D.12c, rien d'autre. Elle dit : les deux sous-étapes, leurs chemins et commits ; les comptes réels (`liste` `src/` = 50) ; les lectures de la ligne (union pour `lq.items` seul, `mh.items`/`jb.ls` en tableaux typés ; `fs` = enregistrements, pas des fixes ; clés gardées en valeurs ; tableaux alloués à la taille du site quand le cart ne borne pas) ; `value.h:7` laissé à D.12d ; les déplacements (`trade:kg`, `galaxy` pris par D.12b ; `slipway`, `probe`, `msgbox:pz` sans liste ; `help`, `pi`, `ui_button`, `cart` entrés) ; `pico8_jp`/`pico8_nv` retirées, noms `_ptrs` gardés ; `pico8_lq_keys_of` → `pico8_lq_copy`, `pico8_lq_add` ; les deux pièges de TU et M0 ; les §6 suiveurs et leurs preuves rouges ; pour D.12d : `PICO8_REF_AS_LIST*` sans appelant dans `src/`, `pico8_type_tag_of` des listes lu par les seuls TU, includes de TU ajoutés, `value.h:7`.
- `docs/glossaire-noms.md` : `jp`, `nv` (c1) : symbole C → « écrit au site » ou `pico8_jp_ptrs`/`pico8_nv_ptrs` ; ligne des aides `_ptrs` réécrite (plus de version à valeur) ; `cb` (struct rendue) ; `mh` (clés : articles typés) ; `fs` (tableau de `pico8_lq_item_t`), `ls` (tableau de `lq`) ; l'union (c2) ; `pico8_lq_copy`, `pico8_lq_add`.
- `docs/trous-de-portage.md` : TROU-7 inchangé et vrai (« les constructeurs de rangées (`jo.ex`, `hp:lq`, `pz`…) allouent une table que personne ne libère jamais ») ; TROU-5, TROU-23, TROU-25 inchangés. Aucune entrée nouvelle attendue.
- Commentaires : quatre sortes de `CLAUDE.md` ; les contrats devenus faux sont réécrits (`collect.h:14-26`, `tech_gate.h:22-26`, `research_display.h:30-34`, `pi.h:26-33`, `msgbox.h:21-24`, `:52-54`, `layout.h:13`, `:19-23`, `:53-55`, `:77-79`, `layout.c:13-14`, `:34-37`, `:133-134`, `ui_button.c:216-218`, `ui_button.h:63-67`) ; aucun renvoi à D.12c ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile` ; aucun e2e ni TU ajouté ou supprimé.
- Toute constante `*_HASH`/`identity[]` de TU.
- Le comportement : mêmes rangées, mêmes éléments, même ordre, mêmes pixels, mêmes `e_` écrits par `cb`, même tremblement de `bl`, mêmes fuites (TROU-7), `make e2e` conforme au bit près.
- Les clés `pico8_lq_keys_t` (valeurs) et leurs producteurs hors `jo:gh`/`jo.ex`/`pj:ph`/`jb_line` ; `pico8_lq_keys_of_row`.
- `pico8_list.c/.h`, `pico8_ptrs.c/.h`, `pico8_ref_audit.h`, `pico8_type_tag.h`, `pico8_value.h`, les aides `_ptrs` (noms et signatures), `test_pico8_list`, `test_pico8_ptrs`.
- Les API à valeur de Q12 : `pico8_lq_bg`, `pico8_pj_ed`, `pico8_js_keys_t.lq`/`h_`/`jb`, `pico8_jc_ce`, `pico8_jy_qe`, `pico8_oj`.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `-Wall -Wextra -Werror` rend fatale toute fonction `static` inutilisée : `pico8_lq_pq_step` (c1), `pico8_jb_item_value` si plus rien ne l'appelle (c2), les prédicats rendus inutiles, les aides locales de TU perdent leur dernier appel dans la sous-étape qui les retire.
- **H2** : aucune constante de TU ne change, les deux pièges traités ; M0 confirme le piège « rien ».
- **H3** : la case « autre-vrai » n'est produite par aucun état de `src/` ni de TU à `cd9247d` (aucun `pico8_value_bool(1)` en positionnel ; `STRING` NULL jamais produit) : sa ligne de classement est une branche morte, justifiée au rapport `gcov` (elle transcrit `elseif f then` du cart, `lua.lua:307-308`).
- **H4** : les bornes du cart citées tiennent pour les données du port (`PICO8_NB_COUNT` 19, `PICO8_FD_COUNT` 42, `PICO8_BZ_COUNT` 12, `fv.rb` 4, `i_` 5 + `ou` 7, ligne de bulle la plus longue 7) ; un TU qui passerait plus d'articles à `mh`/`bl` se révèle par l'`assert`, il suit (listé).
- **H5** : les 10 TU de l'Annexe D sont tous ceux qui lisent ces états ou ces API après D.12b ; un TU oublié se révèle à la compilation, il suit (listé au message).
- **H6** : `jb_new(NULL)` et une `lq` sans positionnel gardent un `ls`/`fs` non NULL de compte 0 (allocation à `max(n, 1)`), sinon `NO_OVERRIDES_HASH` (`jb_ol`) rougit.

## Implementation Chunks

Deux sous-étapes, dans cet ordre, chacune verte seule (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`, balayage ASan+UBSan des TU), vérifiée et poussée avant la suivante, avec ses comptes, sa classification, ses lignes gagnées, son `gcov`, ses mutations et son tableau « Runs » :

1. **D.12c1 — tableaux typés, aides à valeur retirées** (26–27 chemins). Ordre :
   1. mesure de départ (H-D12b-1..-13 constatées, sinon arrêt et reprise du PRD) ; `gcov` de départ ;
   2. `tech_gate.c/.h` (`cb`), `menu.c` (`cr`, `ix[2].hb`, `pico8_cr_step`), TU `fm`, `js` (`:875-889`, `:1704-1725`) ;
   3. `research_display.c/.h` (`bj`), `planet_display.c` (`jo:fe`) ;
   4. `collect.c/.h` : `pico8_jp` retirée ;
   5. `pi.c/.h` (`bl`), `msgbox.c/.h` (`mh.items`, clés), `score_panel_display.c` (`pj:hk`, `pj:ph`), `help.c`, `score_panel.c` ; TU `pj` (`mh->items`), `lq` (`test_calls`) ;
   6. `layout.c/.h` : `pico8_lq_item_t` publié, `fs`, `lq:pq` sans `nv`, `lq:ol`, `ls`, `jb:pq`, `jb:ol` ; `collect.c/.h` : `pico8_nv` retirée, include ; TU `lq` (`fs_count`), `ga`, `hp` (piège des étiquettes), `jb_ol`, `planet_gh_row_lq` (§6) et sa preuve rouge ;
   7. glossaire ; filets ; `gcov` d'arrivée ; mutations M1–M12 ; mesure, classification, message ; **commit**.
2. **D.12c2 — union de `lq.items`** (18–30 chemins). Ordre :
   1. mesure de départ ; `gcov` de départ ; **M0** (piège « rien ») ;
   2. `layout.c/.h` : union, classement, `items`, `lq:mn` et `lq:pq` sur les cases, `pico8_lq_copy` (et retrait de `pico8_lq_keys_of`), `pico8_lq_add`, `jb_line` ;
   3. `ui_button.c` (`js:pq`) ; `research_display.c` (`jo.ex`, `hp:lq`) ; `score_panel_display.c` (`lq1`, `lq2`) ; `planet_display.c` (`jo:gh`) ;
   4. includes (`cart.c`, `probe.h`, ceux que D.12b laisse, `layout.h`/`.c`, `score_panel_display.c`) ; TU qui gagnent un include direct ;
   5. TU `lq`, `js`, `pj`, `hp`, `ga` ; §6 `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by` et leurs preuves rouges ;
   6. filets ; `gcov` ; mutations M13–M24 ; mesure, classification ; glossaire ; note « Découvert à D.12c » ; **commit**.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (éléments, ordre, pixels, effets d'un appel), pas la présence d'une ligne. D.12c change la représentation de six listes membres ou paramètres et d'une douzaine de listes locales sans changer un élément, un ordre ni un pixel. La preuve : les TU existants, qui lisent ces états et plient leurs contenus dans des constantes inchangées ; les 26 e2e au bit près ; `gcov` et mutations ; `sanitize-net` et le balayage ASan+UBSan pour les durées de vie.

### TU touchés (G)

1. **c1** : `fm`, `js` (`cb`) ; `pj` (`mh.items`) ; `lq` (`bl`, `fs`) ; `ga`, `hp` (`fs`, piège des étiquettes) ; `jb_ol` (`ls`) ; `planet_gh_row_lq` (§6, `fs`) ; `mh` seulement si la compilation l'exige.
2. **c2** : `lq`, `js`, `pj`, `hp`, `ga` (cases, piège « rien ») ; `ga_export_panel_by`, `planet_gh_row_lq`, `layout_nil_item_draws_nothing` (§6).
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change, à aucun commit.
4. **Prior art** : TU suiveurs dans le même commit (D.10b, D.11c, D.12a, D.12b) ; constante figée d'étiquette (`FROZEN_INSTANCE_TAG`, D.11d3 ; `FROZEN_LIST_TAG`, D.12b ; `VIEW_LIST_TAG` de `hp`) ; plis de forme déclarés (D.11d1..d4) ; §6 réécrit avec preuve rouge (D.11d, D.12b).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`), lancés comme le `Makefile` ; union des lignes de `src/` exécutées.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée, sinon justifiée au message (branche « autre-vrai » : H3 ; `assert` de borne). Attendus : `fv:kj` (`help.c`) et `pj:kj` (`score_panel.c`) ne sont peut-être exercés que par les e2e (aide, fin de partie) ; `pj:ph` (`lq1`/`lq2`) par `test_pico8_js`/`pj` ou les e2e.
- `L` = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide**, hors les tests de genre et les aides retirés (`pico8_jp`, `pico8_nv`, `pico8_lq_keys_of`, branche liste de `js:pq`, `pico8_lq_pq_step`). Une ligne de `L` est un comportement perdu : arrêt, rapporté. **Si une aide à valeur perd sa route de TU** (plus aucun TU n'exécute un appelant survivant qui passait par elle), une assertion unitaire est exigée sur l'appelant qui reste (par exemple `pico8_cb` : l'ordre des entrées gardées et `e_` de chaque entrée, déjà dans `test_pico8_js`, `:1703-1727` ; `pico8_bj` : aucun TU ne l'appelle directement à `cd9247d`, voir M3).

**Mutations**, une à la fois, dans l'arbre de travail, fichier sauvegardé par `cp` avant et restauré par `cp` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan : `test_pico8_planet_gh_row_lq`) ; chacune doit rougir au moins le TU nommé, sinon c'est un trou, couvert par une assertion ou justifié (mutation équivalente, démontrée). **Astuce** : supprimer un appel ou une entrée peut échouer sous `-Werror` (variable ou fonction `static` inutilisée) ; neutraliser par `0 ? f(…) : …`, `if (0)` ou une condition toujours fausse plutôt que supprimer.

- c1 :
  - **M1** `pico8_cb` garde une entrée que le prédicat rejette (`0 ?` sur le test) → `test_pico8_js` (`test_cb_writes_every_e_`, `test_hb`) / `test_pico8_fm` ;
  - **M2** `pico8_cb` s'arrête à `PICO8_NB_COUNT - 1` → `test_pico8_js` (`e_` de chaque entrée, `:1719`) ;
  - **M3** `pico8_bj` saute le premier projet gardé → `test_pico8_js` (`test_fe`), sinon `make e2e`, sinon assertion ajoutée (`jo:fe` : rangées dans l'ordre de `gb`) ;
  - **M4** `pico8_bl` saute le dernier article → `test_pico8_lq` (`test_calls`) / `test_pico8_pj` ;
  - **M5** `pico8_bl` ne retourne plus `nc` (`nc = -nc` neutralisé) → `make e2e` (boîte de message qui tremble), sinon assertion ;
  - **M6** le constructeur de `mh` copie un article de moins → `test_pico8_pj` (`mh->items` compte `PICO8_BZ_COUNT`, `:747`) ;
  - **M7** `fv:kj` saute le premier article de `rb` → `make e2e` (écran d'aide), sinon assertion ;
  - **M8** `pj:kj` omet la part `ou` → `make e2e` (fin de partie), sinon assertion ;
  - **M9** `lq:pq` met en page un élément de moins (`fs_count` = n − 1) → `test_pico8_lq` (`fs_count`, `CONSTRUCTIONS_HASH`) ; **M9 bis** (§6, preuve rouge) un de plus → `test_pico8_planet_gh_row_lq` rouge ;
  - **M10** `lq:ol` commence à `fs[1]` → `test_pico8_lq` (`fold_drawn`) ;
  - **M11** `jb:pq` saute la première ligne → `test_pico8_jb_ol` ;
  - **M12** `jb:pq` alloue `n` (et non `max(n, 1)`) avec un `ls` NULL quand n = 0 (simulé : `ls = NULL` quand `n == 0`) → `test_pico8_jb_ol` (`:91`, `NO_OVERRIDES_HASH`).
- c2 :
  - **M13** le classement range `BOOL` faux en fixe 0 → `test_pico8_lq` (`CONSTRUCTIONS_HASH`, rangée « false », pixels) ;
  - **M14** le classement range une chaîne en « rien » → `test_pico8_lq` / `test_pico8_ga_export_panel_by` ;
  - **M15** `lq:mn` ignore le `qw` d'une rangée → `test_pico8_lq` (`JO_EX_HASH`) / `test_pico8_ga` ;
  - **M16** `pico8_lq_add` n'ajoute pas → `test_pico8_lq` (`HP_LQ_HASH`) / `test_pico8_hp` ;
  - **M17** (§6, preuve rouge) la copie des clés dans `lq.items` prend un élément de trop → `test_pico8_planet_gh_row_lq` (« pas de cinquième ») rouge ; restauré, vert ;
  - **M18** `pico8_lq_copy` perd `c` → `test_pico8_js` (`JS_MEMBERS_HASH`) / `test_pico8_lq` ;
  - **M19** `jo:gh` n'ajoute plus l'écart aux rangées 2..n → `test_pico8_planet_gh_row_lq` / `test_pico8_lq` (`:743`, comptes 4 et 5) ;
  - **M20** (§6, preuve rouge de la ligne du plan) le classement range `REF` NULL en « autre-vrai » → `test_pico8_layout_nil_item_draws_nothing` rouge (sprite 0 dessiné) ; restauré, vert ;
  - **M21** `jo:gh` : `ji` perd une répétition de `gc` → `test_pico8_lq` (`receives` compte 5, `:743-746`) ;
  - **M22** `jo.ex` intervertit `t1` et `t2` → `test_pico8_lq` (`JO_EX_HASH`) / `test_pico8_hp` / `test_pico8_ga` ;
  - **M23** `pj:ph` : `lq1` reçoit une chaîne vide → `test_pico8_js`/`test_pico8_pj` si l'un plie ces boutons, sinon `make e2e`, sinon assertion ;
  - **M24** `js:pq` rend la source au lieu d'une copie (`pico8_lq_copy` court-circuité) → `test_pico8_js` (`b->h_ != row`, `:1367`) ;
  - **§6 `ga_export_panel_by`** : preuve rouge de D.12b refaite (le constructeur de `ga` n'installe plus `g_ga_by_shared`) → rouge ; restauré, vert.
- Cibles (à `cd9247d`, ⟲ à remesurer) : `tech_gate.c:79-128` ⟲, `research_display.c:305-364` ⟲, `menu.c:92-215` ⟲, `:278-305` ⟲, `planet_display.c:376-410` ⟲, `pi.c:160-195`, `msgbox.c:60-80`, `:108-135`, `help.c:79-95`, `score_panel.c:248-266`, `score_panel_display.c:75-90`, `:175-340`, `layout.c:13-466`, `ui_button.c:216-241`, `planet_display.c:78-290` ⟲, `research_display.c:142-300` ⟲. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit. Aucun e2e ajouté.
- `make sanitize-net` à chaque sous-étape (ligne du plan : « oui »).
- `make class-coverage-net` à chaque sous-étape (constructeurs touchés, « Lecture »).
- **Balayage ASan+UBSan des TU** à chaque sous-étape : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.7b, D.10a, D.11a, D.11c, D.11e, D.12a, D.12b.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 1 h 40 de temps réel pour l'étape** (c1 ≈ 45 min, c2 ≈ 55 min), mesures, `gcov`, mutations et filets compris ; **plafonds** c1 70 min, c2 85 min (au-delà : repli c2r/c2u). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute (`layout.h`, `msgbox.h`, `pi.h` changent : recompilation large) |
| `make e2e` | ~1 s |
| `make sanitize-net` | de l'ordre de la minute |
| `make class-coverage-net` | quelques dizaines de secondes |
| `make line-coverage EVENTS="tests/e2e/*.json"` | moins d'une minute |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes par sous-étape |
| M0 (instrumentation temporaire de 5 TU, un lancement chacun) | < 3 min |
| mutations (c1 : 13, c2 : 13 ; un `make` ciblé chacune) | < 10 min par sous-étape |
| balayage ASan+UBSan des TU | quelques minutes par sous-étape |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà d'un plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; une sous-étape inachevée n'est pas commise. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + sous-étape + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | HEAD gradé de D.12b, avant c1 | 0 | ligne `liste` : N lignes, M fichiers | … |
| 2 | recette `liste` | idem | 0 | `liste N lignes M fichiers K occ.` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que les chemins de sa ligne de la table « Empreinte » (plus un TU suiveur listé au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`.
2. **Greps** de la « Sortie », exactement, après c2 ; après c1 : grep 2 vide ; aucune ligne `liste` restante dans `collect.*`, `tech_gate.*`, `pi.*`, `msgbox.*`, `help.c`, `score_panel.c`, ni sur `fs`/`ls`/`bj`/`cb_list`/`bj_list`.
3. **Formes** : `lq.items` en union à cinq cas, classée une fois à la construction ; `fs`, `ls` en tableaux typés alloués à `max(n, 1)` ; `mh.items`, `cb`, `bj`, `fv:kj`, `pj:kj`, `lb`, `lq1`/`lq2`, `jb_line` en tableaux fixes avec borne nommée et `assert` porteur de message ; `jo.ex`, `ji`, `merged` alloués à une taille calculée au site ; ordre de parcours inchangé partout.
4. **Casts** : aucun cast nouveau hors les `(void *)` de qualificatif déjà présents aux lignes touchées ; aucune conversion d'un `void *` vers une struct précise sans son test (les besoins de `ji` sont des `pico8_jy_t *` par affectation implicite ; `&e->row`) ; aucun cast de pointeur de fonction ; aucun `typedef` ni macro qui renomme `pico8_list_t`.
5. **API** : les signatures de la table « classe (b) », et elles seules, ont changé ; `pico8_lq_keys_t` inchangée.
6. **Aides** : `pico8_jp`, `pico8_nv`, leurs typedefs, `pico8_lq_keys_of` absents ; aides `_ptrs` inchangées.
7. **Liens** : grep 5.
8. **Constantes** : grep 6 vide ; chaque constante figée d'étiquette vaut `0x9E5C1157u` et remplace un `pico8_type_tag_of` sur `lq.fs`, listée au message ; M0 rapporté et conforme.
9. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (c) = 0, (b) limitée à la liste ; lignes gagnées et lignes comptées modifiées listées ; `pico8_value.h:7` intact ; aucun alias, registre, cale.
10. **Écarts déclarés** : lectures de la ligne (union pour `lq.items` seul ; `fs` ; clés en valeurs ; `value.h:7`) ; déplacements (`trade:kg`, `galaxy`, `slipway`, `probe`, `msgbox:pz` ; `help`, `pi`, `ui_button`, `cart` entrés) ; trois §6 suiveurs, preuves rouges rapportées ; `pico8_lq_bg` hors étape.
11. **Preuve** : `gcov` (lignes ajoutées exécutées ou justifiées ; `L` vide), mutations restaurées par `cp`, `sanitize-net`, `class-coverage-net`, balayage ASan+UBSan rapportés.
12. **Commentaires** : quatre sortes de `CLAUDE.md` ; les contrats devenus faux réécrits ; aucun renvoi à D.12c ni au plan dans `src/` ou `tests/`.
13. **Docs** : note « Découvert à D.12c » en fin de ligne D.12c (c2) ; glossaire à jour ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le commit qui gradue D.12b (ou un descendant qui ne touche que `docs/`), propre ; ou si une hypothèse `H-D12b-n` n'y tient pas sans que ce PRD ait été repris (« Sections à revérifier »).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 (`layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`) change **dans son intention** (valeurs attendues, cas, garde, assertion retirée ou affaiblie), ou si l'une de ses preuves rouges manque ; changer sa façon de lire la rangée est attendu et déclaré.
- `FAILED:` si une liste ou un parcours ne tombe sous aucune forme de ce PRD ni du §5 (arrêt « champ sans règle »), dont le cas M0 ; ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si `make sanitize-net`, le balayage ASan+UBSan des TU ou `make class-coverage-net` n'est pas propre ou identique, à l'une des sous-étapes.

**Périmètre et forme**

- `FAILED:` si `tools/` ou le `Makefile` change ; si un commit touche un chemin hors de sa ligne d'« Empreinte » sans le lister comme TU suiveur.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit (après c2), ou si les contrôles partiels du point 2 échouent après c1.
- `FAILED:` si `pico8_lq_keys_t.items` change de type, si l'union a plus ou moins de cinq cas, si `mh.items` ou `jb.ls` prend l'union, si un tableau borné par le cart n'a pas sa borne nommée et son `assert` porteur de message, ou si une liste de l'étape devient un `pico8_ptrs_t` alors que ses éléments ne sont pas des pointeurs.
- `FAILED:` si une durée de vie change : un tableau qui remplace une liste qui fuyait est libéré, ou l'inverse (TROU-7) ; si `jb_new(NULL)` perd son cadre (H6).
- `FAILED:` si une conversion d'élément vers une struct précise n'a pas son test dans la même fonction, si un pointeur de fonction est casté, ou si un cast nouveau est ajouté.
- `FAILED:` si `pico8_lq_bg`, `pico8_pj_ed` ou une autre API de Q12 change de type.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `liste` de `src/` après c2 ne vaut pas **exactement 50** (les 49 lignes de `pico8_list.c/.h`, `pico8_ref_audit.h`, `pico8_type_tag.h` et `pico8_value.h:7`), ou si `pico8_value.h:7` a été réécrit.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, registre, renommage d'un identifiant vivant pour échapper aux greps, commentaire compté réécrit hors de son sujet, cale valeur ↔ conteneur, littéral remplaçant un identifiant compté, (c) non nulle, (b) hors de la liste.
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées ; la justification de chaque Δlignes > Δoccurrences ; les includes gardés, retirés ou ajoutés ; les aides de TU retypées, créées ou retirées ; les constantes figées d'étiquette ; M0 (c2) ; le rapport `gcov` ; les mutations. Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne ajoutée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` n'est pas vide ; si une aide à valeur perd sa route de TU sans assertion unitaire sur l'appelant qui reste.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification ; si M20 ne fait pas rougir `layout_nil_item_draws_nothing`.
- `FAILED:` si un plafond est dépassé sans note (et sans repli c2r/c2u pour c2), ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.12c », ou si elle omet l'un de ces éléments : les comptes réels, les lectures de la ligne, les formes, les déplacements, les aides retirées et les noms `_ptrs` gardés, les API changées, les deux pièges de TU et M0, les §6 suiveurs, ce que D.12d trouve.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- Les listes membres L2, `trade:kg`, `galaxy` (`hr_list`), les concaténations de `jy` dans `jo:gh` : D.12b.
- `pico8_list.c/.h`, `test_pico8_list`, `PICO8_REF_AS_LIST*`, `PICO8_TYPE_TAG_LIST`, `pico8_value.h:7`, les includes de `pico8_list.h` dans les TU, les TU restants de `liste` (aides `list_size`, branches « liste » des `fold_value` de TU, `FROZEN_LIST_TAG`) : D.12d.
- Les clés `pico8_lq_keys_t` en union, `pico8_js_keys_t.lq`/`h_`/`jb`, `pico8_lq_bg`, `pico8_pj_ed`, et toute API à valeur de Q12 ; les catégories `valeur`, `type-tag`, `ref-audit` : Q12 (étude D.9b), ouverte, posée à l'humain à la fin de D.12.
- Le renommage des aides `_ptrs` : phase 3, si le glossaire le justifie.
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **Union** : pour `lq.items` seul, locale à `layout`, classée à la construction ; clés gardées en valeurs ; `mh.items`, `jb.ls` en tableaux typés de leur élément unique ; `fs` en tableau de `pico8_lq_item_t`.
  - **Bornes** : tableau fixe là où le cart borne (`cb` 19, `bj` 42, `mh` 12, `fv:kj` 4, `pj:kj` 12, `lb` 4, `lq1`/`lq2` 1, `jb_line` 7) ; tableau alloué à la taille du site ailleurs (`items`, `fs`, `ls`, `jo.ex`, `ji`, `merged`).
  - **API** : `cb`/`bj` rendent une struct par valeur ; `bl` prend un tableau d'articles ; `pico8_lq_keys_of` → `pico8_lq_copy` ; `pico8_lq_add` nouvelle.
  - **Aides** : `pico8_jp`, `pico8_nv` meurent en c1 ; filtres et mise en page écrits au site ; noms `_ptrs` gardés.
  - **Durées de vie** : inchangées (TROU-7).
  - **Compte** : 50 et non 49, `pico8_value.h:7` laissé vrai à D.12d.
  - **TU** : suivent dans le même commit ; constante figée pour l'étiquette de `fs` ; case « rien » pliée du genre de sa source, M0 avant c2.
  - **§6** : les trois suivent, preuves rouges.
  - **`pico8_lq_bg`** : hors D.12c (Q12).
  - **Découpage** : deux sous-étapes (≈ 45 et ≈ 55 min), repli c2r/c2u écrit.
- **Surprises de mesure, pour `plan-pilot`** : (1) Q10 range `mh.items` et `jb.ls` dans l'union, mais aucun des deux ne passe par `lq:mn` : ils sont homogènes (articles ; rangées `lq`, qui ne sont aucun des cinq cas) ; (2) « `lq.fs` en tableau typé de fixes » est une imprécision de l'étude : `fs` tient les enregistrements dessinés `{fn, d}`, pas des largeurs ; (3) **l'union fond nil et faux, que `CONSTRUCTIONS_HASH` de `test_pico8_lq` distingue** (rangées « false », « nil », « mixed c ») : sans le pli « du genre de la source », la constante rougirait sans bug ; M0 le vérifie avant c2, arrêt sinon ; (4) trois constantes de TU plient l'étiquette de liste de `lq.fs` (`ga`, `hp` à profondeur 3, et deux assertions) ; (5) la ligne compte 49 lignes restantes, il en reste 50 : le commentaire de `pico8_value.h:7` nomme `pico8_list.h` et reste vrai jusqu'à D.12d ; (6) `hp:lq` ajoute deux éléments à une `lq` **après** sa mise en page (le cart le fait : `lua.lua:1409-1413`) : `lq.items` doit pouvoir grandir, `fs` non ; (7) `jb_new(NULL)` dessine un cadre parce que `ls` est une liste vide non NULL : un `malloc(0)` le perdrait ; (8) la ligne nomme `slipway`, `probe`, `msgbox:pz`, `trade:kg`, `galaxy` : à `cd9247d`, les trois premiers n'ont plus de liste locale et les deux derniers passent à D.12b ; elle omet `help`, `pi` (`bl`), `ui_button` (branche liste de `js:pq`), `jb_line`, `cart.c` ; (9) « §6 : `layout_nil_item_draws_nothing` » est incomplet : `planet_gh_row_lq` et `ga_export_panel_by` lisent aussi `lq.items` ; (10) l'empreinte de la ligne (« ≈ 16 (15, 1) ») est de 20 fichiers de `src/` (plus les includes que D.12b laisse) et 10 TU ; (11) `pico8_lq_keys_t` peut rester à valeur : c'est ce qui rend l'union « locale à `layout` » et épargne dix producteurs.
- **Pour D.12d** : `PICO8_REF_AS_LIST*` n'a plus d'appelant dans `src/` (seul `pico8_ref_audit.h` le définit) ; `pico8_type_tag_of` ne rencontre plus de liste dans `src/` hors `pico8_list.c` ; `pico8_value.h:7` ; les includes de TU ajoutés par c2 ; les branches « liste » des `fold_value` de TU (`lq:109`, `hp:137`) et les constantes figées d'étiquette restent (elles plient une constante).
- **Inventaire exhaustif** : Annexe C (`src/`), Annexe D (`tests/`), numéros de `cd9247d`, ⟲ à remesurer.

## Annexe A — textes du plan recopiés (`docs/plan-3.md` à `cd9247d`, par extraction de lignes)

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


### §5, phase D, en entier (l. 228-280) : puces, note de D.0, « Liste de D » avec empreinte, règles communes et ordre, arrêt « champ sans règle », décisions de D.9, comptes au départ de D.10, lignes D.2 à D.12d avec toutes leurs notes « Découvert à D.x »

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
- D.12b — `liste`, listes membres L2 (`jo.oe`, `jo.cc`/`by`, `trade.lp`/`rl`, `mz.ox`, proposition `ib`, `qy`, `jc.ts`, `hy.invented`, `jo.jy`/`hp.i` ; `gen.oc` en tableau fixe) et aides L4 de `collect` (`jp`, `nv`, `gu`/`gu_content`, `la_append` ; prédicat `int (*)(void *, void *ctx)`). Empreinte ≈ 25 (25, 0). Compte : `liste` rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui. Découpé à la PRD en deux sous-étapes, même PRD, chacune vérifiée et poussée : D.12b1 (`qy`, `ts`, `invented`, `gen.oc`) et D.12b2 (réseau des routes : `oe`, `jy`, `cc`, `by` avec `g_ga_by_shared`, `lp`/`rl`, `ib` ; repli b2o/b2t).
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

## Annexe B — étude D.9, `pico8_list_t` (`docs/prd/done/plan3-phaseD-etape9-etude.md`, inchangée à `cd9247d`)

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

### §4, en entier (l. 141-152 : le point 2 nomme les valeurs à genres mêlés de Q12)

## 4. Ce qui n'entre dans aucune forme

1. **Classe M, 32 lignes** (16 comptées et 16 appels d'aides, `particle.c`, `probe.c`) : l'émetteur de particules et la sonde sont du code partagé par des branches distinctes (`g_`/`fv` et l'arbre `qh`, plus `mz` pour `dl`), qui lit par nom des champs que chaque branche porte à sa place. C'est la règle 2 de la phase 2. Le §5 cite d'ailleurs ce groupe `py`/`fc`/`k_`/`qo`/`ro`/`qg`/`q_`/`du` comme « exemple connu ». Aucune forme de D ne le couvre sans anticiper cette règle, ou sans ajouter environ 25 accesseurs jetables (Q7).
2. **Les valeurs à genres mêlés, hors des trois points** : `hf` et `gg` (booléen ou fixe, TROU-3), `qh_qh`, `et` (vraie cible, struct sans champ, ou `false`), les retours de `ldown`/`gv`, `js.my` (nil, `false` ou fonction), `js.h_` (TROU-23), `pj_ed`, `trade_qe`, l'`id` de `jc_hg`, `gen.nm[3]`, `pico8_jo_hf`… On compte **62 prototypes publics à `pico8_value_t`** ; environ 20 sont des méthodes que le point 2 type.
   - Une union étiquetée locale par cas est la forme probable, mais aucune puce ne la fixe.
   - `valeur`, `type-tag` et `ref-audit` ne tombent donc pas avec D.10-D.12. C'est un quatrième point, et le prochain arrêt probable (Q12).
3. **Aucune clé dynamique à ensemble non fini.** Toutes les clés à nom variable ont un ensemble fini et s'énumèrent :
   - `me` : 7 valeurs ;
   - `nt` : 2 valeurs ; les noms `bm` : 5 ;
   - `rm` : 3 valeurs ;
   - `bt` : 2 valeurs ;
   - les 27 aides à nom en paramètre n'ont que des appelants à littéral (82 lignes).

### Étapes proposées : en-tête, lignes D.10a à D.12d, règles (l. 179-203)

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

## Annexe C — lignes `liste` de `src/` que D.12c retire (mesurées à `cd9247d`, motif de Q10)

Colonne 1 : sous-étape (`c1`, `c2` ; `D12d` = laissée, déclarée). Colonne 2 : `⟲` = numéro (ou existence, pour `inc`) à remesurer par contenu après D.12b. Les lignes b1/b2 de l'Annexe C du PRD de D.12b ne sont pas reprises (H-D12b-5). Totaux à `cd9247d` : c1 **80**, c2 **49** + 9 `inc` que D.12b décide (H-D12b-7), D12d 1.

Lignes que D.12b réécrit en gardant la catégorie ou qu'il ajoute (H-D12b-4), **c2 ⟲**, absentes de `cd9247d` sous cette forme : dans `pico8_planet_display.c`, l'ajout à `ctx->ji` du prédicat (`:99` à `cd9247d`, `pico8_list_add(ctx->ji, e)`, classée b2 « modifiée gardée ») et les ajouts à `merged` écrits au site (≈ 3 à 4 lignes).

#### `src/pico8_cart.c`

```
c2  -  25:#include "pico8_list.h"
```

#### `src/pico8_collect.c`

```
c1  ⟲  23:pico8_list_t pico8_jp(const pico8_list_t *qk, pico8_jp_predicate_fn ow, void *ctx) {
c1  ⟲  24:    pico8_list_t f;
c1  ⟲  25:    pico8_list_iter_t it;
c1  ⟲  27:    pico8_list_init(&f);
c1  ⟲  31:    it = pico8_list_all(qk);
c1  ⟲  32:    while (pico8_list_iter_next(&it, &e)) {
c1  ⟲  34:            pico8_list_add(&f, e);
c1  ⟲  40:pico8_list_t pico8_nv(const pico8_list_t *qk, pico8_nv_fn kp, void *ctx) {
c1  ⟲  41:    pico8_list_t m;
c1  ⟲  42:    pico8_list_iter_t it;
c1  ⟲  45:    pico8_list_init(&m);
c1  ⟲  46:    it = pico8_list_all(qk);
c1  ⟲  47:    while (pico8_list_iter_next(&it, &e)) {
c1  ⟲  50:            pico8_list_add(&m, v);
```

#### `src/pico8_collect.h`

```
c1  ⟲ inc  6:#include "pico8_list.h"
c1  ⟲  17: * caller releases with pico8_list_free(); a NULL `qk` gives an empty one. */
c1  ⟲  19:pico8_list_t pico8_jp(const pico8_list_t *qk, pico8_jp_predicate_fn ow, void *ctx);
c1  ⟲  24: * pico8_list_free(). `qk` must not be NULL. */
c1  ⟲  26:pico8_list_t pico8_nv(const pico8_list_t *qk, pico8_nv_fn kp, void *ctx);
```

#### `src/pico8_economy_formulas.h`

```
c2  ⟲ inc (si D.12b le garde)  6:#include "pico8_list.h"
```

#### `src/pico8_gen.h`

```
c2  ⟲ inc (si D.12b le garde)  6:#include "pico8_list.h"
```

#### `src/pico8_help.c`

```
c1  -  85:        pico8_list_t items;
c1  -  88:        pico8_list_init(&items);
c1  -  90:            pico8_list_add(&items, pico8_value_ref((void *)self->rb->items[i].row));
c1  -  93:        pico8_list_free(&items);
```

#### `src/pico8_layout.c`

```
c2  -  11:#include "pico8_list.h"
c1  -  162:    pico8_list_t *fs_heap;
c1  -  201:    pico8_list_iter_t it;
c1  -  216:    it = pico8_list_all(self->fs);
c1  -  217:    while (pico8_list_iter_next(&it, &entry)) {
c2  -  232:    pico8_list_t items;
c2  -  246:    pico8_list_init(&items);
c2  -  248:        pico8_list_add(&items, pico8_jb_item_value(&item->row->items[i]));
c2  -  255:    pico8_list_free(&items);
c1  -  261:    pico8_list_t *ls_heap = malloc(sizeof *ls_heap);
c1  -  264:    pico8_list_init(ls_heap);
c1  -  266:        pico8_list_add(ls_heap, pico8_value_ref(pico8_jb_line(&row->items[i])));
c1  -  275:    const pico8_list_t *ls;
c1  -  278:    pico8_list_iter_t it;
c1  -  295:    it = pico8_list_all(ls);
c1  -  296:    while (pico8_list_iter_next(&it, &entry)) {
c2  -  348:    pico8_list_init(&self->items);
c2  -  351:            pico8_list_add(&self->items, keys->items[k]);
```

#### `src/pico8_layout.h`

```
c2  -  6:#include "pico8_list.h"
c1  -  16:    pico8_list_t *ls;
c2  -  26:    pico8_list_t items;
c1  -  35:    pico8_list_t *fs;
```

#### `src/pico8_menu.c`

```
c1  ⟲  17:#include "pico8_list.h"
c1  ⟲  162:    pico8_list_t cb_list;
c1  ⟲  193:    pico8_list_free(&cb_list);
c1  ⟲  283:    pico8_list_t cb_list;
c1  ⟲  295:    pico8_list_free(&cb_list);
```

#### `src/pico8_msgbox.c`

```
c1  -  121:    pico8_list_init(&self->items);
c1  -  124:            pico8_list_add(&self->items, keys->items[i]);
```

#### `src/pico8_msgbox.h`

```
c1  -  8:#include "pico8_list.h"
c1  -  34:    pico8_list_t items;
```

#### `src/pico8_particle.h`

```
c2  ⟲ inc (si D.12b le garde)  7:#include "pico8_list.h"
```

#### `src/pico8_pi.c`

```
c1  -  177:void pico8_bl(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb, pico8_fixed_t nc) {
c1  -  178:    pico8_list_iter_t it = pico8_list_all(rb);
c1  -  181:    while (pico8_list_iter_next(&it, &e_val)) {
c1  -  188:void pico8_bl_default(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb) {
c1  -  189:    pico8_list_iter_t it = pico8_list_all(rb);
c1  -  192:    while (pico8_list_iter_next(&it, &e_val)) {
```

#### `src/pico8_pi.h`

```
c1  -  6:#include "pico8_list.h"
c1  -  34:void pico8_bl(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb, pico8_fixed_t nc);
c1  -  35:void pico8_bl_default(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb);
```

#### `src/pico8_planet_display.c`

```
c2  ⟲  80:    pico8_list_t *ji;
c2  ⟲  146:    pico8_list_t *ji;
c2  ⟲  190:    pico8_list_init(ji);
c2  ⟲  191:    pico8_list_add(ji, pico8_value_string("receives:"));
c2  ⟲  199:        pico8_list_t *merged = malloc(sizeof *merged);
c2  ⟲  216:            pico8_list_t *merged = malloc(sizeof *merged);
c2  ⟲  240:        pico8_list_free(ji);
c2  ⟲  250:            pico8_list_t *lb = malloc(sizeof *lb);
c2  ⟲  253:            pico8_list_init(lb);
c2  ⟲  257:                pico8_list_add(lb, pico8_value_string("makes:"));
c2  ⟲  258:                pico8_list_add(lb, pico8_value_string(mu_str));
c2  ⟲  272:                pico8_list_add(lb, pico8_value_string("exports:"));
c2  ⟲  273:                pico8_list_add(lb, pico8_value_string(combined));
c2  ⟲  275:            pico8_list_add(lb, pico8_value_ref((void *)jo_kw_tuple(kw)));
c2  ⟲  286:        pico8_list_add(PICO8_REF_AS_LIST(bs[i].lq.as.ref), pico8_value_ref((void *)&gap));
c1  ⟲  382:    pico8_list_t bj_list;
c1  ⟲  405:    pico8_list_free(&bj_list);
```

#### `src/pico8_planet_family_trade.c`

```
c2  ⟲ inc (si D.12b le garde)  10:#include "pico8_list.h"
```

#### `src/pico8_planet_trade.h`

```
c2  ⟲ inc (si D.12b le garde)  6:#include "pico8_list.h"
```

#### `src/pico8_probe.h`

```
c2  -  8:#include "pico8_list.h"
```

#### `src/pico8_research.h`

```
c2  ⟲ inc (si D.12b le garde)  6:#include "pico8_list.h"
```

#### `src/pico8_research_display.c`

```
c2  ⟲  144:    pico8_list_t l;
c2  ⟲  157:    pico8_list_init(&l);
c2  ⟲  200:            pico8_list_add(&l, pico8_value_ref(&tuple->row));
c2  ⟲  211:        pico8_list_add(&l, pico8_value_ref(&t1->row));
c2  ⟲  220:        pico8_list_add(&l, pico8_value_ref(&t2->row));
c2  ⟲  228:        pico8_list_add(&l, pico8_value_string(jq_str));
c2  ⟲  229:        pico8_list_add(&l, pico8_value_ref((void *)&t3));
c2  ⟲  254:                pico8_list_add(&l, pico8_value_ref(&t4->row));
c2  ⟲  257:            pico8_list_add(&l, pico8_value_ref((void *)pico8_jo_em()));
c2  ⟲  264:    pico8_list_free(&l);
c2  ⟲  295:    pico8_list_add(&l->items, pico8_value_ref((void *)pico8_hp_fk()));
c2  ⟲  297:        pico8_list_add(&l->items, pico8_value_string(self->px));
c1  ⟲  350:pico8_list_t pico8_bj(const pico8_struct_header_t *jo_instance) {
c1  ⟲  351:    pico8_list_t items;
c1  ⟲  353:    pico8_list_t result;
c1  ⟲  356:    pico8_list_init(&items);
c1  ⟲  358:        pico8_list_add(&items, pico8_value_ref(pico8_gb_at(k)));
c1  ⟲  362:    pico8_list_free(&items);
```

#### `src/pico8_research_display.h`

```
c1  ⟲  7:#include "pico8_list.h"
c1  ⟲  32: * caller owns and must release with pico8_list_free(). A `jo_instance`
c1  ⟲  34:pico8_list_t pico8_bj(const pico8_struct_header_t *jo_instance);
```

#### `src/pico8_score_panel.c`

```
c1  -  16:#include "pico8_list.h"
c1  -  252:    pico8_list_t combined;
c1  -  255:    pico8_list_init(&combined);
c1  -  257:        pico8_list_add(&combined, pico8_value_ref((void *)i_->items[i].row));
c1  -  260:        pico8_list_add(&combined, pico8_value_ref((void *)ou_state->row.items[i].row));
c1  -  265:    pico8_list_free(&combined);
```

#### `src/pico8_score_panel_display.c`

```
c2  -  12:#include "pico8_list.h"
c2  -  262:        pico8_list_t lq1;
c2  -  263:        pico8_list_t lq2;
c2  -  269:        pico8_list_init(&lq1);
c2  -  270:        pico8_list_add(&lq1, pico8_value_string(" again? "));
c2  -  281:        pico8_list_free(&lq1);
c2  -  283:        pico8_list_init(&lq2);
c2  -  284:        pico8_list_add(&lq2, pico8_value_ref((void *)&back));
c2  -  295:        pico8_list_free(&lq2);
```

#### `src/pico8_slipway.h`

```
c2  ⟲ inc (si D.12b le garde)  7:#include "pico8_list.h"
```

#### `src/pico8_tech_gate.c`

```
c1  ⟲  115:pico8_list_t pico8_cb(pico8_value_t bi) {
c1  ⟲  116:    pico8_list_t items;
c1  ⟲  118:    pico8_list_t result;
c1  ⟲  121:    pico8_list_init(&items);
c1  ⟲  123:        pico8_list_add(&items, pico8_value_ref((void *)&pico8_nb_table[i]));
c1  ⟲  127:    pico8_list_free(&items);
```

#### `src/pico8_tech_gate.h`

```
c1  -  5:#include "pico8_list.h"
c1  -  24: * list the caller owns and must release with pico8_list_free(), of REFs to
c1  -  26:pico8_list_t pico8_cb(pico8_value_t bi);
```

#### `src/pico8_trade.c`

```
c2  ⟲ inc (si D.12b le garde)  9:#include "pico8_list.h"
```

#### `src/pico8_trade.h`

```
c2  ⟲ inc (si D.12b le garde)  5:#include "pico8_list.h"
```

#### `src/pico8_ui_button.c`

```
c2  -  232:    assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST &&
c2  -  235:        const pico8_list_t *list = PICO8_REF_AS_LIST_CONST(v.as.ref);
```

#### `src/pico8_value.h`

```
D12d  -  7: * list element (pico8_list.h), an entity field (pico8_struct.h). Lua has a single dynamically-typed value
```

## Annexe D — TU (mesurés à `cd9247d`)

Colonne 1 : sous-étape ; colonne 2 : `L` = ligne comptée `liste`, `-` = non comptée (texte ou type qui change, ou pli à réécrire). Un titre marqué ⟲ est un TU que D.12b modifie aussi (H-D12b-9) : numéros à remesurer par contenu. Les autres lignes `liste` de ces TU (includes, `list_size`, branches « liste » des `fold_value`) restent à D.12d.

#### `tests/test_pico8_fm.c`

```
c1 L 88:    pico8_list_t kept = pico8_cb(pico8_value_bool(1));
c1 - 89:
c1 L 90:    pico8_list_free(&kept);
```

#### `tests/test_pico8_js.c` ⟲

```
c2 - 196:
c2 - 197:static int row_count(const void *lq) {
c2 - 198:    return ((const pico8_lq_t *)lq)->items.count;
c2 - 199:}
c2 - 200:
c2 - 201:static pico8_value_t row_item(const void *lq, int i) {
c2 - 202:    assert(i >= 0 && i < row_count(lq));
c2 - 203:    return ((const pico8_lq_t *)lq)->items.items[i];
c2 - 204:}
c1 L 875:    pico8_list_t cb;
c1 - 887:    cb = pico8_cb(nil());
c1 - 888:    affordable = cb.count > 0;
c1 L 889:    pico8_list_free(&cb);
c2 - 1297:/* Each optional key unset, the fixed ones and `my` given at zero, `h_`
c2 - 1298: * false, TROU-23, a row and a list as `lq(x)`'s argument. */
c2 - 1299:static uint32_t fold_keys(uint32_t h, pico8_lq_t *row) {
c2 - 1300:    const pico8_js_keys_t none = {0};
c2 - 1301:    pico8_js_keys_t k;
c2 L 1302:    pico8_list_t *list = malloc(sizeof *list);
c2 - 1369:
c2 L 1370:    pico8_list_init(list);
c2 L 1371:    pico8_list_add(list, pico8_value_string("from a list"));
c2 L 1372:    pico8_list_add(list, pico8_value_fixed(pico8_fixed_from_int(3)));
c2 - 1373:    k = none;
c2 - 1374:    k.lq = pico8_value_ref(list);
c2 - 1375:    b = built(k);
c2 - 1376:    assert(b->lq != NULL && is_lq(b->lq) && row_count(b->lq) == 2);
c2 - 1490:            assert(lq->items.count >= 2 && lq->items.items[1].kind == PICO8_VALUE_STRING);
c2 - 1491:            assert(t != NULL && t == pico8_nb_lookup(lq->items.items[1].as.string));
c1 L 1704:    pico8_list_t kept;
c1 - 1712:    kept = pico8_cb(nil());
c1 - 1720:        for (k = 0; k < kept.count; k++) {
c1 - 1721:            is_kept |= kept.items[k].as.ref == t;
c1 L 1725:    pico8_list_free(&kept);
```

#### `tests/test_pico8_pj.c` ⟲

```
c2 - 109:
c2 - 110:static int row_count(const void *lq) {
c2 - 111:    return ((const pico8_lq_t *)lq)->items.count;
c2 - 112:}
c2 - 113:
c2 - 114:static pico8_value_t row_item(const void *lq, int i) {
c2 - 115:    assert(i >= 0 && i < row_count(lq));
c2 - 116:    return ((const pico8_lq_t *)lq)->items.items[i];
c2 - 117:}
c1 - 717:static pico8_value_t ph_row_item(const pico8_mh_t *mh, int k, int i) {
c1 - 718:    const pico8_jb_row_t *ld;
c1 - 719:    const pico8_lq_t *lq;
c1 - 720:
c1 - 721:    assert(k < mh->items.count && mh->items.items[k].kind == PICO8_VALUE_REF);
c1 - 722:    ld = mh->items.items[k].as.ref;
c1 - 723:    assert(ld->fn != NULL && strcmp(ld->fn, "ld") == 0 && ld->items[0].is_ref);
c1 - 724:    lq = ld->items[0].ref;
c1 - 725:    assert(i <= lq->items.count);
c1 - 726:    return lq->items.items[i - 1];
c1 - 727:}
c1 - 747:    assert(mh != NULL && mh->items.count == PICO8_BZ_COUNT);
```

#### `tests/test_pico8_lq.c` ⟲

```
c2 - 76: * in ascending index order. */
c2 - 77:static int row_count(const void *lq) {
c2 - 78:    return ((const pico8_lq_t *)lq)->items.count;
c2 - 79:}
c2 - 80:
c2 - 81:static pico8_value_t row_item(const void *lq, int i) {
c2 - 82:    assert(i >= 0 && i < row_count(lq));
c2 - 83:    return ((const pico8_lq_t *)lq)->items.items[i];
c2 - 84:}
c1 - 178:
c1 - 179:static int fs_count(const void *lq) {
c1 L 180:    const pico8_list_t *fs = as_lq(lq)->fs;
c1 L 181:    assert(fs != NULL && pico8_type_tag_of(fs) == PICO8_TYPE_TAG_LIST);
c1 - 182:    return fs->count;
c1 - 183:}
c2 - 371:    }
c2 - 372:    {
c2 - 373:        pico8_value_t a[] = {pico8_value_bool(0)};
c2 - 374:        k = positionals(1, a);
c2 - 375:        h = fold_built_from(h, "false", &k);
c2 - 376:    }
c2 - 377:    {
c2 - 378:        pico8_value_t a[] = {pico8_value_ref(NULL)};
c2 - 379:        k = positionals(1, a);
c2 - 380:        h = fold_built_from(h, "nil", &k);
c2 - 381:    }
c2 - 382:    {
c2 - 383:        static const int f[] = {13, 8};
c2 - 384:        pico8_value_t a[] = {str("a"), num(12), pico8_value_ref(NULL), pico8_value_bool(0), str("bc"),
c2 - 385:                             pico8_value_ref(NULL), num(40)};
c2 - 386:        a[2] = tuple_of(2, f);
c2 - 387:        k = with_c(positionals(7, a), 9);
c2 - 388:        h = fold_built_from(h, "mixed c", &k);
c2 - 389:    }
c2 - 390:    h = fold_built_from(h, "NULL", NULL);
c1 L 530:    pico8_list_t rb;
c1 - 549:    entry.items = entry_items;
c1 - 550:    entry.fn = "ld";
c1 L 551:    pico8_list_init(&rb);
c1 L 552:    pico8_list_add(&rb, pico8_value_ref(&entry));
c1 - 553:    pico8_gfx_init(&gfx);
c1 - 554:    pico8_cls(&mem);
c1 - 555:    pico8_bl_default(&gfx, &mem, &rb);
c2 - 696:        lq = ((const pico8_js_t *)button)->lq;
c2 - 697:
c2 - 698:        if (lq != NULL && lq->items.count > 0) {
c2 - 699:            v = lq->items.items[0];
c2 - 700:            if (v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, head) == 0) {
c2 - 741:    receives = panel_row("receives:");
c2 - 742:    assert(wants != NULL && receives != NULL && panel_row("needs:") == NULL);
c2 - 743:    assert(wants->items.count == 4 && receives->items.count == 5);
c2 - 744:    assert(wants->items.items[1].as.ref == &a->row && wants->items.items[2].as.ref == &b->row);
c2 - 745:    assert(receives->items.items[1].as.ref == &a->row && receives->items.items[2].as.ref == &b->row &&
c2 - 746:           receives->items.items[3].as.ref == &b->row);
```

#### `tests/test_pico8_ga.c` ⟲

```
c2 - 471:
c2 - 472:static uint32_t fold_lq_positional(uint32_t h, const pico8_lq_t *lq, int i) {
c2 - 473:    pico8_value_t v = lq->items.items[i];
c2 - 474:
c2 - 475:    h = fnv(h, (uint32_t)(i + 1));
c2 - 476:    return v.kind == PICO8_VALUE_REF && v.as.ref != NULL ? fold_tuple(h, v.as.ref) : fold_value(h, v);
c1 - 489:           lq->header.cls == pico8_lq_struct_class());
c1 - 490:    bg = pico8_lq_bg(lq);
c1 - 491:    k = lq->fs->count;
c1 - 511:    h = fold_value(fnv_str(h, "fs"), pico8_value_ref(lq->fs));
c2 - 513:    for (i = k; i < lq->items.count; i++) {
c2 - 514:        h = fold_lq_positional(h, lq, i);
c2 - 515:    }
c2 - 516:    return fnv(h, (uint32_t)(lq->items.count + n + 2));
```

#### `tests/test_pico8_hp.c` ⟲

```
c1 - 280:    assert(pico8_type_tag_of(lq) == PICO8_TYPE_TAG_STRUCT && lq->header.cls == pico8_lq_struct_class());
c1 - 281:    bg = pico8_lq_bg(lq);
c1 - 282:    k = lq->fs->count;
c1 - 283:    h = fold_view_head(h, lq->items.count + (lq->has_c != 0) + !(bg.kind == PICO8_VALUE_REF && bg.as.ref == NULL) +
c2 - 284:                              (lq->has_score != 0) + (lq->ra != NULL) + 2);
c2 - 285:    for (i = 0; i < k; i++) {
c2 - 286:        h = fold_lq_item(fold_view_index(h, i + 1), lq->items.items[i], 3);
c2 - 287:    }
c1 - 300:    h = fold_value(fold_view_name(h, "fs"), pico8_value_ref(lq->fs), 3);
c2 - 302:    for (i = k; i < lq->items.count; i++) {
c2 - 303:        h = fold_lq_item(fold_view_index(h, i + 1), lq->items.items[i], 3);
c2 - 304:    }
```

#### `tests/test_pico8_planet_gh_row_lq.c` (§6)

```
c2 L 1:/* A display row's "lq" holds a `pico8_list_t*`, whose elements `js:pq()`
c2 - 2: * takes as the row's positionals; the bug read it as the deleted ob() table.
c2 - 62:/* The row's item at 1-based `index`, as a table lookup would find it. */
c2 - 63:static int lq_item(const pico8_lq_t *lq, int index, pico8_value_t *out) {
c2 - 64:    if (index < 1 || index > lq->items.count) {
c2 - 65:        return 0;
c2 - 66:    }
c2 - 67:    *out = lq->items.items[index - 1];
c2 - 68:    return 1;
c2 - 69:}
c1 - 163:    /* `lq:pq()` ran over exactly those four: one `fs` entry per item. */
c1 - 164:    assert(exports_lq->fs != NULL);
c1 - 165:    assert(exports_lq->fs->count == 4);
```

#### `tests/test_pico8_jb_ol.c`

```
c1 L 91:    assert(direct->ls != NULL && pico8_type_tag_of(direct->ls) == PICO8_TYPE_TAG_LIST && direct->ls->count == 0);
```

#### `tests/test_pico8_ga_export_panel_by.c` (§6) ⟲

```
c2 - 50:
c2 - 51:static const char *lq_index_string(const pico8_lq_t *lq, int index) {
c2 - 52:    pico8_value_t v;
c2 - 53:    if (index < 1 || index > lq->items.count) {
c2 - 54:        return NULL;
c2 - 55:    }
c2 - 56:    v = lq->items.items[index - 1];
c2 - 57:    return (v.kind == PICO8_VALUE_STRING) ? v.as.string : NULL;
c2 - 58:}
```

#### `tests/test_pico8_layout_nil_item_draws_nothing.c` (§6)

```
c2 - 103:/* `lq({item}):ol(v(DRAW_X,DRAW_Y))` -- one real row holding one
c2 - 104: * positional item, rendered onto a cleared screen. */
c2 - 105:static void render_one_item(pico8_value_t item) {
c2 - 106:    pico8_lq_keys_t keys = {0};
c2 - 107:    pico8_lq_t *line;
c2 - 108:
c2 - 109:    keys.items = &item;
c2 - 110:    keys.count = 1;
c2 - 111:    line = pico8_lq_new(&keys);
c2 - 112:    assert(line != NULL && line->items.count == 1);
c2 - 113:
c2 - 114:    pico8_cls(&mem);
c2 - 115:    pico8_lq_ol_default(&gfx, &mem, line, pico8_vec_v(pico8_fixed_from_int(DRAW_X), pico8_fixed_from_int(DRAW_Y)));
c2 - 116:}
```

`test_pico8_mh` : ses clés (`:327-359`) ne posent pas `items` ; il ne suit que si la compilation l'exige (listé au message).

