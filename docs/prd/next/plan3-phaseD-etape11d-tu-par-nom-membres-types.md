---
title: "Plan 3, étape D.11d : les TU cessent de lire et d'écrire par nom — T1 (mise en état, lecture d'état) en membres typés à plis de hachage identiques au bit près, T2 (tests des descripteurs, cas de mort des écrivains) supprimés comme tests de la couche (Q11, §6), trois faux descripteurs qui nourrissent un site C gardés pour D.11e (T3) ; `gh_row_lq` et `ga_export_panel_by` lisent les membres avec leur preuve rouge ; aucun fichier de `src/` ; deux chunks d1/d2"
description: "Après D.11c, `par-nom` compte encore 567 lignes dans 38 TU, plus environ 2 480 appels non comptés d'aides locales par nom (`lookup`, `get`, `has`, `fold_field`…) que ces lignes définissent. D.11d type la mise en état et la lecture d'état (T1) sans changer une constante de hachage, supprime les tests dont l'objet est le lecteur ou l'écrivain par nom (T2, Q11), garde les trois faux descripteurs qui nourrissent un site C de `src/` jusqu'à D.11e (T3) : `par-nom` `tests/` passe de 567 à 5, total ≈ 297 (mesuré à `a917e3a`, D.11c en cours : hypothèses H-D11c)."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.11c est implémentée** (`plan-pilot`, étape 2b). Tout ce qui suit a été lu et mesuré sur l'arbre **commis** `a917e3a` (« docs: PRD de l'étape D.11c … mis en file »), extrait par `git archive a917e3a` dans le scratchpad : `grep`, `sh tools/data_residue.sh` (avec `--detail`), la recette d'occurrences de D.8 et des scripts de lecture seule (graphe d'appel des fonctions `static` de chaque TU). **Rien n'a été compilé ni lancé.** Les numéros de ligne sont ceux de `a917e3a`. Ce que D.11c changera est écrit en hypothèses `H-D11c-n` (plus bas) ; les sections qui en dépendent sont marquées ⟲ et listées dans « Sections à revérifier ».

Le problème, vu du mainteneur du port : après D.11a–D.11c, `src/` ne lit plus par nom que ses sites de classe C (D.11e) et l'infrastructure elle-même ; mais les TU, eux, lisent et écrivent presque tout par nom. 38 fichiers de `tests/` portent 567 lignes `par-nom` (622 occurrences), et ces lignes sont surtout des **aides locales** (`lookup`, `get`, `has`, `get_ref`, `must_fixed`, `fold_field`, `fold_vec`, `bool_state`, `write_*`…) que les tests appellent avec un littéral de nom. Ces appels ne sont pas comptés (le motif ne voit que la définition de l'aide), mais ils sont **environ 2 480** : chaque aide ne meurt qu'avec son dernier appelant. Tant qu'un TU lit par nom, D.11e ne peut supprimer ni `pico8_entity_get_field`/`set_field`, ni les lecteurs `field`/écrivains `set_field` des descripteurs, ni les `kind_field` : le compilateur les verrait encore nommés.

L'étude D.9 (§1.2) a classé ces lignes en deux formes : **T1** (mise en état et lecture d'état : le TU connaît son type) et **T2** (tests des descripteurs eux-mêmes : leur objet disparaît avec eux). L'humain a tranché **Q11** : les T2 sont supprimés comme tests de la couche.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `a917e3a` (§5, « Phase D — Données », l. 275) :

> - D.11d — `par-nom`, TU : T1 (mise en état et lecture d'état) en accès aux membres typés ; T2 (tests des descripteurs, tests de mort des `assert` d'écrivain) supprimés comme tests de la couche (Q11, §6). Deux chunks d'environ 19 fichiers. Empreinte 38 (0, 38). Compte : `par-nom` ≈ 286 (l'infrastructure de `src/`). §6 : `test_pico8_planet_gh_row_lq` et `test_pico8_ga_export_panel_by` (3 lignes chacun) lisent les membres, preuve rouge. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change.

Décisions de D.9, recopiées (l. 265) :

> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Paragraphe « Liste de D », empreinte et règles communes, ordre, recopiés (l. 237, 239, 241) :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.

> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Comptes au départ de D.10, avec la règle des ponts de TU qui nomme D.11d (l. 267) :

> Comptes au départ de D.10 (HEAD 5a13008, `make -s data-residue` et `--detail` ; l'étude mesurait `2dec71f`, avant D.7a–D.8) : `valeur` 4961/126, `type-tag` 352/65, `ref-audit` 56/15, `par-nom` 1009/81 (`src/` 433/43, `tests/` 576/38), `dispatch` 191/61 (`src/` 67/30, `tests/` 124/31), résidu D 5868 lignes / 128 fichiers ; `liste`, pas encore au compteur, 927/99 avec le motif de Q10 (`src/` 496/61, `tests/` 431/38). Les classes A–M, T1/T2 et L1–L4 sont celles de l'étude (annexe A) ; ses « ≈ » viennent de son classement, chaque PRD remesure. Un TU qui nomme un symbole retiré par une étape de `src/` la suit dans le même commit, ou le symbole reste comme pont compté, fermé par l'étape TU de son groupe (D.10c, D.11d, D.12d) ; le PRD tranche. `sanitize-net` : aux seules étapes qui changent une durée de vie ou un propriétaire (§7).

Ligne D.11e, recopiée (l. 276), avec la note de D.11b qui la complète et qui lie D.11d (« à sa suppression, garder une comparaison rec ↔ `pico8_X_start` ») :

> - D.11e — `par-nom`, suppression de l'infrastructure. Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`, laissé à D.11a ; accesseur typé par champ dans la table de classe, NULL = absent, Q6 ; l'oracle accesseurs = lecteurs naît et meurt dans l'étape). Découvert à D.11b (pour D.11d/D.11e) : les littéraux `s`/`od` des `pico8_X_kind_rec` doublent ceux de `pico8_X_start` ; seul `test_kind_rec_matches_the_reader` en garde l'égalité — à sa suppression, garder une comparaison rec ↔ `pico8_X_start` ou des constantes nommées partagées. Puis : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks. Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non.

§6, recopié (l. 302-305) :

> ## 6. Tests existants

> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.

> - Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés** par ce plan. Depuis la fusion de master (e2e rapide, #32), seuls `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` restent ; les autres ont été remplacés par les e2e de `tests/e2e/`, qui en portent la garde. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.

Toutes les autres lignes de la phase D (D.0 à D.12, **avec toutes leurs notes « Découvert à D.x »** : D.0, D.1, D.7c, D.8, D.10a, D.10b, D.10c, D.11a, D.11b), l'arrêt « champ sans règle », le §6 et le §7 entiers sont recopiés tels quels, par extraction de lignes, en **Annexe A**. Les passages de l'étude D.9 sur T1/T2, la table des étapes (ligne D.11d) et Q11 sont en **Annexe B**. L'inventaire ligne à ligne est en **Annexe C**.

### Hypothèses `H-D11c-n` (D.11c n'est pas gradée)

Source : le PRD de D.11c (`docs/prd/triage/plan3-phaseD-etape11c-composants-emetteur-et-sonde.md` à `a917e3a`), rien d'autre. Chacune est à confronter à la note « Découvert à D.11c » et au diff de D.11c avant que ce PRD passe en `triage/`.

| Hypothèse | Énoncé | Ce qui en dépend ici |
|---|---|---|
| H-D11c-1 | Après D.11c : `par-nom` `src/` **292** / 352 occ., `tests/` **567** / 622 occ., total **859** ; `particle.c` à 0, donc 69 fichiers ; `valeur`, `type-tag`, `ref-audit` en baisse, rapportés | départ, arrivée (≈ 297), critères `FAILED:` de comptes |
| H-D11c-2 | D.11c n'ajoute ni ne retire aucune ligne `par-nom` de `tests/` (son cas structurel de `test_pico8_ui` passe par `emitter_of`/`probe_of`, les cas qu'il retire n'en portent pas) | l'inventaire de l'Annexe C reste exact en contenu |
| H-D11c-3 | D.11c touche cinq TU : `test_pico8_fv` (l. 495-496 et 501-505 retirées : décalage de −7 des lignes comptées suivantes, 548 → 541 … 788 → 781, et 3 appels d'aides de moins), `test_pico8_ui` (`ke:ks` typé à 518, 1024, 1040, 1158, 1161, 1164, 1168, 1327, 1332 ; `test_ke_ks_stops_off_row` et ses sept aides `ks_*`, l. 1176-1224, retirés ; un cas structurel ajouté), `test_pico8_jc` (827, 1441 : même nombre de lignes), `test_pico8_g_` (495-496), `test_pico8_jo` (463). Une inclusion ajoutée (`pico8_probe.h`, `pico8_particle.h`) décalerait tout le fichier | numéros de l'Annexe C pour ces cinq fichiers (⟲) ; chunk d2 |
| H-D11c-4 | Membres déplacés dans des composants portés par pointeur : `pico8_emitter_t` `{pn, du, py, ki, pk, pk_is_bool, fc, k_, qo, ro, qg, q_, qy}` sur `g_` (donc `fv`), `hj` (donc `pm`, `mx`), `ga` ; `pico8_probe_t` `{et, has_et, dw, od, has_od, dq, fp, fp_is_bool, ik, has_ik}` sur `g_` (`fv`), `qh` et ses sept descendants, `mz`, `jc` ; `k_` de `jc` (palette de voie) reste dans `pico8_jc_t` | chaque T1 qui lit ou écrit un de ces noms écrit `->emitter->X` / `->probe->X` (la note « Ce que D.11d trouve » du PRD de D.11c le dit : « les T1 qui liront un membre déplacé typé écriront `->probe->dw`, `->emitter->qy` ; `test_pico8_jc.c:827` le fait déjà ») |
| H-D11c-5 | `pico8_ke_ks(pico8_struct_header_t *, const char *me)` : les appels de `test_ke_me_by_name`/`test_ke_t_by_name` sont déjà typés ; `me_of` y reste par nom jusqu'à D.11d | conversion de ces deux tests (T1, voir « Tests mixtes ») |
| H-D11c-6 | Aucune constante `*_HASH`/`identity[]` ne change à D.11c ; `KE_ME_HASH` inchangée | la règle « plis identiques au bit près » part des constantes de `a917e3a` |
| H-D11c-7 | D.11c ne touche ni les trois faux descripteurs T3 (`test_pico8_iv.c:218-230`, `test_pico8_mo.c:94-107`, `test_pico8_ui.c:799-847`), ni les sites C qu'ils nourrissent (`bootstrap.c:87` `p_`, `entity_geom.c:49` `ra`, `ui.c:346` `fa`, `ui.c:359`/`:363` `m_`) | T3 |
| H-D11c-8 | `pico8_jc_too_far_t` passe dans `slipway.h` ; `pico8_mz_jj_t` y est déjà (`wormhole.h`) | lectures T1 de `qs`/`fo` des cibles dans `test_pico8_mz` |
| H-D11c-9 | `Makefile`, `DATA_ENGINE_SRCS`, `tools/` inchangés ; `-Wall -Wextra -Werror` inchangé | aides mortes à supprimer ; liens des TU |
| H-D11c-10 | Aucun cas de mort de `test_pico8_ui`/`_fv` que D.11c retire n'est dans la liste T2 de ce PRD (les sept de `ke:ks` ne portent aucune ligne `par-nom`) | liste des tests T2 |

### Sections à revérifier quand D.11c sera gradée

« État de départ » (comptes, nombre de fichiers), « Hypothèses `H-D11c-n` » (verdict par ligne), Annexe C pour `test_pico8_fv`, `_ui`, `_jc`, `_g_`, `_jo` (numéros ⟲), « Empreinte et découpage » (contenu de d2, appels d'aides de `fv`/`ui`), « Effet sur `make data-residue` », les greps de la « Sortie », les critères `FAILED:` de départ et de comptes, le budget. Toute ligne T1 qui nomme un membre de H-D11c-4 est à relire contre la forme réelle des composants.

### Lecture de ces textes pour D.11d

- **Périmètre** : les 567 lignes `par-nom` de `tests/` à `a917e3a` (38 fichiers, H-D11c-2) **et** les appels non comptés des aides que ces lignes définissent : une aide ne meurt qu'avec son dernier appelant, et remplacer ses appels par une autre aide par nom serait un renommage pour échapper aux greps. Rien dans `src/`, `tools/`, le `Makefile`, `tests/e2e/`.
- **« T1 (mise en état et lecture d'état) en accès aux membres typés »** : un TU qui construit un état ou lit un état le fait sur la struct (`qh->jo.mi = …; qh->jo.has_mi = 1;`, étude §1.2), derrière un test de classe ou de lignée quand son pointeur est un en-tête.
- **« T2 (tests des descripteurs, tests de mort des `assert` d'écrivain) supprimés comme tests de la couche (Q11, §6) »** : un test dont l'objet est la réponse du lecteur ou de l'écrivain par nom est supprimé. Le §6 dit « supprimés dans le commit qui supprime le code testé » ; la ligne D.11d, ajoutée après D.9 avec les décisions de l'humain, place cette suppression **ici**, avant D.11e : la ligne, plus précise et postérieure, l'emporte (G), et la tension est rapportée dans la note (« Documentation »). Ce que D.11e perd à ne plus trouver ces tests est traité dans « D.11e ».
- **« Deux chunks d'environ 19 fichiers. Empreinte 38 (0, 38) »** : remesuré, 38 TU exactement ; d1 18 fichiers, d2 20 (plus le plan pour la note) ; voir « Empreinte et découpage ».
- **« Compte : `par-nom` ≈ 286 (l'infrastructure de `src/`) »** : ce PRD attend **≈ 297** : `src/` 292 (H-D11c-1 ; 286 était l'estimation de l'infrastructure par l'étude, D.11c mesure 292 avec ses sites C et `mt` restants), plus **5 lignes T3** dans trois TU, gardées pour D.11e — la ligne D.11e le prévoit déjà (« Empreinte ≈ 45 (42, **3**) »). Rapporté.
- **« §6 : `test_pico8_planet_gh_row_lq` et `test_pico8_ga_export_panel_by` (3 lignes chacun) lisent les membres, preuve rouge »** : confirmé, trois lignes chacun (`gh_row_lq` 47, 60, 167 ; `ga_export_panel_by` 38, 49, 89) ; seule leur façon de lire change ; preuve rouge dans le commit (§6 : bug remis localement, test rouge, correctif restauré).
- **« `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change »** : pris tel quel. Le balayage ASan+UBSan des TU touchés reste exigé (précédent D.7c, TU seuls) : un cast faux dans un TU est un comportement indéfini que seul ce balayage voit.
- **Règle « Un TU qui nomme un symbole retiré par une étape de `src/` la suit » (l. 267)** : sans objet ici (aucun symbole de `src/` ne change) ; D.11d est l'étape TU de `par-nom` qui ferme les ponts éventuels : il n'y en a aucun (D.11a–D.11c n'en ont posé aucun, notes).
- **Note de D.11b pour D.11d/D.11e** (ligne D.11e) : `test_kind_rec_matches_the_reader` est T2 et part ici ; la comparaison rec ↔ `pico8_X_start` survit, portée par `test_kind_reader_serves_the_generated_defaults` converti (voir « Sortes »).
- **Leçons des notes à ne pas refaire** : classification (a)/(b)/(c) absente ou fausse ; lignes qui gagnent une catégorie omises ; **constante de hachage « mise à jour »** ; **identifiant compté remplacé par sa valeur (alias, D.7c)** ; TU corrompu qui passait en relisant sa propre corruption (balayage ASan) ; lecture par nom déplacée dans une aide non comptée ; **cas de mort ou constante d'un TU non vus par le PRD** (D.11a) ; mutation non restaurée par `cp`.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json`, rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; les tests gardent une couverture réelle (toute couverture perdue est listée et justifiée) ; interdits : jonction de lignes, alias, registre, renommage pour échapper aux greps, cale valeur ↔ typé permanente ; mutations restaurées par `cp` ; binaires lancés exactement comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan, dont `test_pico8_planet_gh_row_lq`) ; chunks d'au plus ~40 chemins ; jamais l'appli raylib.

### État de départ

**Mesuré sur `a917e3a`** (arbre archivé), sans build :

| Catégorie | Lignes | Fichiers | Occurrences | `src/` (lignes / occ.) | `tests/` (lignes / occ.) |
|---|---|---|---|---|---|
| `valeur` | 3876 | 123 | 4170 | rapporté | rapporté |
| `type-tag` | 343 | 63 | 544 | rapporté | rapporté |
| `ref-audit` | 44 | 15 | 45 | 44 / 45 | 0 |
| `par-nom` | 884 | 70 | 999 | 317 / 377 | **567 / 622** (38 fichiers) |
| **résidu D** | **4681** | **124** | — | | |

Départ attendu de D.11d (H-D11c-1) : `par-nom` 859 (`src/` 292 / 352, `tests/` 567 / 622), 69 fichiers ; `DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch` inchangé.

Recette (réutilisée à chaque mesure ; `$S` : dossier du scratchpad) :

```sh
sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
for c in valeur type-tag ref-audit par-nom; do
  printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
for f in tests/*.c; do n=$(grep -cE "$(pattern par-nom)" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$(pattern par-nom)" $f | wc -l)"; done
```

### Inventaire (mesuré sur `a917e3a`)

Méthode : chaque ligne comptée a été rattachée à sa fonction englobante ; pour chaque fonction `static` d'un TU, l'ensemble des tests qui l'atteignent a été calculé (graphe d'appel, pointeurs de fonction compris) ; chaque test à ligne ou appel par nom a été classé sur son commentaire d'en-tête et son contenu ; les lignes des tests gardés dont l'objet est le descripteur ont été classées une à une. Les appels d'aides par nom (non comptés) ont été comptés de la même façon : ligne qui appelle une fonction du TU prenant un `const char *` et atteignant, directement ou non, une ligne comptée.

**Résultat** : 567 lignes = **T1 193**, **T2 369**, **T3 5**. Appels non comptés d'aides par nom : **1 556 à typer** (dans des tests ou aides T1), **925 supprimés** (dans des tests ou aides T2). **47 tests** supprimés en entier (14 en d1, 33 en d2) ; une trentaine de tests « `*_is_a_struct` » et assimilés **gardés**, amputés de leurs lignes T2.

Codes de l'Annexe C (la lettre après T1/T2/T3) :

| Code | Classe | Ce que devient la ligne | Classification au commit |
|---|---|---|---|
| A | T1 | aide par nom (définition) : meurt quand ses appelants T1 sont typés et ses appelants T2 supprimés | (a2) |
| R | T1 | lecture d'état → membre typé, cast derrière un test de classe ou de lignée | (a1) |
| W | T1 | mise en état → écriture typée du membre **et** de son drapeau (`has_*`, `*_is_bool`), exactement l'effet de l'écrivain lu dans `src/` | (a1) |
| S | T1 | singleton → accesseur typé (`pico8_ok()`, `pico8_je_nf()`, `pico8_fm_hy()`, `pico8_mo_qb()`) | (a1) |
| C | T1 | commentaire d'en-tête dont le contrat (« lu par `pico8_entity_get_field` ») devient faux : réécrit | (a3) |
| D | T2 | test du descripteur : lecteur brut comparé au générique, emplacement `field`/`set_field`, lecteur hérité, lecteur de sorte | (a2) |
| M | T2 | cible d'un cas de mort d'`assert` d'écrivain (`must_abort`, `write_off_row`, `write_*`) | (a2) |
| N | T2 | réponse du lecteur ou de l'écrivain par nom : nom étranger, méthode lue comme champ (`hd`, `hq`, `ly`, `my`), absent par forme, écriture acceptée | (a2) |
| X | T2 | ligne d'un test ou d'une aide T2 supprimés en entier | (a2) |
| F | T3 | faux descripteur d'un TU qui nourrit un site C de `src/` : gardé jusqu'à D.11e | gardée |

**T3 — les trois faux descripteurs gardés** (5 lignes, 5 occurrences) :

| Lignes | Faux | Site C de `src/` qu'il nourrit | Pourquoi il ne peut pas partir ici |
|---|---|---|---|
| `test_pico8_iv.c:224`, `:229` | `fake_toast_field` / `g_fake_toast_class` (`test_dg_drives_a_ke_struct`) | `bootstrap.c:87` : `dg` lit `p_` par `pico8_entity_get_field` | sans `.field`, `p_` serait absent : le toast ne sortirait plus de `hv` et le test changerait d'objet |
| `test_pico8_mo.c:107` | `g_ra_only_class` (`test_gd_on_placeholder`) | `entity_geom.c:49` : `gd` lit `ra` par nom | idem : `ra` absent, `gd` lirait (0,0) pour les deux entités |
| `test_pico8_ui.c:843`, `:844` | `g_giver_class` (`giver_field`, `giver_set_field`) | `ui.c:346` (`fa` du survolé), `ui.c:359`/`:363` (`m_` écrit sur `gj`/`o_`) | sans `.field`, `fa` absent ; sans `.set_field`, l'écriture de `m_` s'arrête |

Ces cinq lignes tombent avec le site C qu'elles servent, au premier chunk de D.11e (accesseur de Q6). Elles sont exactement les « 3 » fichiers de TU de l'empreinte de D.11e.

**Par chunk** (numéros de chaque ligne : Annexe C) :

**Chunk d1**

| Fichier | `par-nom` l. / occ. | T1 | T2 | T3 | appels à typer | appels supprimés | tests T2 entiers | constantes `*_HASH` du fichier | `valeur` l. | `type-tag` l. |
|---|---|---|---|---|---|---|---|---|---|---|
| `test_pico8_jo.c` | 5 / 5 | 2 | 3 | 0 | 38 | 0 | 0 | 3 | 37 | 14 |
| `test_pico8_qh.c` | 9 / 9 | 4 | 5 | 0 | 52 | 0 | 0 | 3 | 42 | 19 |
| `test_pico8_nl.c` | 21 / 22 | 5 | 16 | 0 | 40 | 9 | 1 | 3 | 70 | 4 |
| `test_pico8_os.c` | 12 / 15 | 6 | 6 | 0 | 38 | 0 | 0 | 2 | 44 | 5 |
| `test_pico8_hj.c` | 7 / 10 | 4 | 3 | 0 | 57 | 0 | 0 | 3 | 40 | 6 |
| `test_pico8_pm.c` | 7 / 11 | 3 | 4 | 0 | 58 | 0 | 0 | 2 | 37 | 4 |
| `test_pico8_mx.c` | 10 / 15 | 4 | 6 | 0 | 56 | 0 | 0 | 3 | 41 | 4 |
| `test_pico8_nr.c` | 9 / 17 | 4 | 5 | 0 | 52 | 0 | 0 | 2 | 50 | 5 |
| `test_pico8_fr.c` | 14 / 19 | 4 | 10 | 0 | 54 | 14 | 2 | 4 | 62 | 4 |
| `test_pico8_ga.c` | 15 / 19 | 3 | 12 | 0 | 71 | 32 | 2 | 3 | 102 | 7 |
| `test_pico8_hp.c` | 9 / 10 | 4 | 5 | 0 | 51 | 22 | 1 | 8 | 82 | 9 |
| `test_pico8_hy.c` | 9 / 10 | 5 | 4 | 0 | 56 | 29 | 1 | 6 | 57 | 10 |
| `test_pico8_trade.c` | 13 / 14 | 4 | 9 | 0 | 34 | 16 | 1 | 1 | 65 | 7 |
| `test_pico8_lq.c` | 7 / 8 | 4 | 3 | 0 | 28 | 18 | 1 | 7 | 97 | 10 |
| `test_pico8_entity_kind.c` | 25 / 25 | 1 | 24 | 0 | 40 | 44 | 4 | 4 | 81 | 4 |
| `test_pico8_map_tables.c` | 1 / 1 | 0 | 1 | 0 | 0 | 10 | 1 | 0 | 5 | 0 |
| `test_pico8_ie.c` | 22 / 22 | 12 | 10 | 0 | 81 | 0 | 0 | 6 | 52 | 5 |
| `test_pico8_ga_export_panel_by.c` | 3 / 3 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 13 | 2 |
| **18 fichiers** | **198 / 235** | **72** | **126** | **0** | **806** | **194** | **14** | **60** | | |

**Chunk d2**

| Fichier | `par-nom` l. / occ. | T1 | T2 | T3 | appels à typer | appels supprimés | tests T2 entiers | constantes `*_HASH` du fichier | `valeur` l. | `type-tag` l. |
|---|---|---|---|---|---|---|---|---|---|---|
| `test_pico8_ui.c` | 40 / 41 | 4 | 34 | 2 | 86 | 266 | 10 | 21 | 384 | 20 |
| `test_pico8_js.c` | 31 / 31 | 11 | 20 | 0 | 158 | 11 | 1 | 15 | 170 | 10 |
| `test_pico8_jc.c` | 56 / 56 | 10 | 46 | 0 | 132 | 110 | 5 | 10 | 192 | 6 |
| `test_pico8_pj.c` | 14 / 16 | 2 | 12 | 0 | 92 | 57 | 2 | 5 | 90 | 17 |
| `test_pico8_g_.c` | 29 / 30 | 5 | 24 | 0 | 79 | 87 | 3 | 7 | 180 | 13 |
| `test_pico8_fv.c` | 12 / 16 | 3 | 9 | 0 | 59 | 26 | 1 | 4 | 89 | 5 |
| `test_pico8_mz.c` | 64 / 66 | 27 | 37 | 0 | 45 | 58 | 2 | 7 | 140 | 6 |
| `test_pico8_bg.c` | 22 / 22 | 12 | 10 | 0 | 32 | 17 | 1 | 6 | 70 | 6 |
| `test_pico8_qb.c` | 22 / 23 | 8 | 14 | 0 | 25 | 29 | 2 | 3 | 67 | 4 |
| `test_pico8_nf.c` | 14 / 17 | 6 | 8 | 0 | 13 | 11 | 1 | 2 | 40 | 3 |
| `test_pico8_iv.c` | 8 / 8 | 2 | 4 | 2 | 13 | 23 | 1 | 5 | 43 | 4 |
| `test_pico8_fq.c` | 22 / 22 | 9 | 13 | 0 | 3 | 8 | 1 | 6 | 49 | 2 |
| `test_pico8_mh.c` | 11 / 11 | 10 | 1 | 0 | 3 | 15 | 1 | 5 | 32 | 4 |
| `test_pico8_fm.c` | 6 / 7 | 1 | 5 | 0 | 10 | 13 | 1 | 0 | 12 | 2 |
| `test_pico8_je.c` | 5 / 7 | 4 | 1 | 0 | 0 | 0 | 0 | 0 | 7 | 2 |
| `test_pico8_mo.c` | 7 / 7 | 2 | 4 | 1 | 0 | 0 | 1 | 0 | 16 | 4 |
| `test_pico8_probe_mm_rnd_order.c` | 1 / 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 5 | 0 |
| `test_pico8_jb_ol.c` | 1 / 2 | 0 | 1 | 0 | 0 | 0 | 0 | 5 | 1 | 4 |
| `test_pico8_planet_gh_row_lq.c` | 3 / 3 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 14 | 2 |
| `test_pico8_spec_defaults.c` | 1 / 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 |
| **20 fichiers** | **369 / 387** | **121** | **243** | **5** | **750** | **731** | **33** | **101** | | |

**Tests T2 supprimés en entier** (47) : `test_pico8_bg` : `test_bg_members_by_name` ; `test_pico8_entity_kind` : `test_kind_rec_matches_the_reader`, `test_ie_ke_view_by_name`, `test_ie_members_by_name`, `test_ie_writer_stops_off_row` ; `test_pico8_fm` : `test_fm_members_by_name` ; `test_pico8_fq` : `test_fq_members_by_name` ; `test_pico8_fr` : `test_nr_members_by_name`, `test_fr_mt_by_name` ; `test_pico8_fv` : `test_fv_members_by_name` ; `test_pico8_g_` : `test_bv_et_members_by_name`, `test_bv_et_writer_stops_off_row`, `test_g__members_by_name` ; `test_pico8_ga` : `test_ga_members_by_name`, `test_ga_writer_stops_off_row` ; `test_pico8_hp` : `test_hp_members_by_name` ; `test_pico8_hy` : `test_hy_members_by_name` ; `test_pico8_iv` : `test_iv_members_by_name` ; `test_pico8_jc` : `test_mouse_point_members_by_name`, `test_too_far_members_by_name`, `test_too_far_writer_stops_off_row`, `test_jc_members_by_name`, `test_jc_writer_stops_off_row` ; `test_pico8_js` : `test_ha_pz_members_by_name` ; `test_pico8_lq` : `test_lq_members_by_name` ; `test_pico8_map_tables` : `test_kinds_answer_nothing_once_down` ; `test_pico8_mh` : `test_mh_members_by_name` ; `test_pico8_mo` : `test_accessors_refuse_a_table` ; `test_pico8_mz` : `test_jj_qs_by_name`, `test_mz_members_by_name` ; `test_pico8_nf` : `test_nf_members_by_name` ; `test_pico8_nl` : `test_nl_members_by_name` ; `test_pico8_pj` : `test_cr_members_by_name`, `test_pj_members_by_name` ; `test_pico8_qb` : `test_qb_members_by_name`, `test_qb_writer_stops_off_row` ; `test_pico8_trade` : `test_trade_members_by_name` ; `test_pico8_ui` : `test_ui_members_by_name`, `test_jo_members_by_name`, `test_jo_written_by_name`, `test_jo_position_by_name`, `test_jo_state_by_name`, `test_jo_writers_by_name`, `test_qh_members_by_name`, `test_hj_members_by_name`, `test_pm_members_by_name`, `test_mx_members_by_name`.

**Tests au nom trompeur, classés T1** (leur objet est un comportement, pas le lecteur) : `test_ke_me_by_name` et `test_ke_t_by_name` (`test_pico8_ui` : le `me` et le `t` de chacune des 24 classes, et l'effet de `ke:ks`/`dg` ; ce sont les seuls tests directs de `ke:ks` sur `me`, et la mutation M13 de D.11c s'y appuie), `test_g__t_key_stops_off_row` (`test_pico8_ui` : la clé `t` d'un `g_`, lue une fois), `test_jb_state_by_name` (`test_pico8_jb_ol` : l'état dessiné ; seule sa ligne 140, qui compare les emplacements `field`/`set_field` à NULL, est T2), `test_mh_nil_lh_key_reads_absent` (constructeur de `mh` : clé `lh` nulle → pas de `lh`), les trois `test_defaults_match_the_parsed_spec` (`fq`, `iv`, `mh` : défauts générés, lus sur les membres ; leurs lectures de `hd`/`my` par nom sont T2-N).

## Solution

**Deux commits (chunks d1, d2), chacun vert seul**, sans toucher `src/` :

1. **d1 — arbre `jo`, recherche, économie, sortes** (18 TU, dont `ga_export_panel_by`, §6) : chaque lecture et écriture d'état passe par la struct ; les aides par nom meurent avec leurs appelants ; les tests des descripteurs et les cas de mort des écrivains partent ; `test_kind_reader_serves_the_generated_defaults` compare `rec` à `pico8_X_start` ; `ga_export_panel_by` lit `ok->le`, `js->lq`, `jo.by`, prouvé rouge.
2. **d2 — interface, sonde, voies, singletons** (20 TU, dont `planet_gh_row_lq`, §6) : même traitement ; `test_ke_me_by_name`/`test_ke_t_by_name` lisent `me`/`t` sur `pico8_ke_t` ; les trois faux descripteurs T3 restent ; `gh_row_lq` lit `ok->le`, `js->lq`, `lq->fs`, prouvé rouge ; note « Découvert à D.11d ».

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (`make data-guard` : `résidu gardé (spec,overrides,stash,ob,dispatch) : 0 ligne(s)`) ; `make e2e` : **26 conformes** ;
- balayage ASan+UBSan des 38 TU touchés propre ;
- **aucune constante `*_HASH`/`identity[]` d'un test gardé ne change** : chaque ligne `-`/`+` d'une constante dans `git diff` est la suppression d'une constante que seul un test T2 nommait, listée ;
- `make data-residue` : `par-nom` `tests/` = **5 lignes / 5 occurrences** (T3), `src/` = 292 (H-D11c-1, inchangé), total **≈ 297** ; `valeur`, `type-tag` rapportés (forte baisse attendue) ;
- aucun fichier de `src/`, `tools/`, `tests/e2e/`, aucun `*.sha256`, le `Makefile` inchangé ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  . "$S/pat.sh"
  # 1. par-nom dans tests/ : les trois faux T3 seulement
  grep -nE "$(pattern par-nom)" tests/*.c
  #    attendu : tests/test_pico8_iv.c (2 lignes : return pico8_ke_field(...) ; .field = fake_toast_field,)
  #              tests/test_pico8_mo.c (1 : g_ra_only_class = {.field = ra_only_field})
  #              tests/test_pico8_ui.c (2 : .field = giver_field, ; .set_field = giver_set_field,)
  # 2. plus aucun lecteur ni écrivain générique appelé par un TU : vide
  grep -nE 'pico8_entity_(get|set)_field|kind_field|kind->field|cls->(set_)?field|_struct_class\(\)->(set_)?field' tests/*.c
  # 3. plus aucune aide qui passe d'un nom à un membre : seuls les faux T3 comparent un nom
  grep -nE 'strcmp\(name' tests/*.c
  #    attendu : fake_toast_field (iv), ra_only_field (mo), giver_field et giver_set_field (ui)
  # 4. plus aucune aide de TU à paramètre de nom de membre : vide, ou chaque ligne rendue listée au
  #    message avec ce que le nom désigne (libellé de check, nom de fichier…)
  grep -nE '^static [^(]+\([^)]*const char \*(name|n|field|member)\b' tests/*.c
  # 5. la comparaison rec <-> pico8_X_start survit (note de D.11b)
  grep -nE 'rec->(rr|s|me|od|has_s|has_od)\b.*pico8_[a-z]+_start\.' tests/test_pico8_entity_kind.c | wc -l   # >= 25
  # 6. les deux tests du §6 : diff limité aux trois lectures, à leurs déclarations et au commentaire qui les décrit
  git diff <départ> -- tests/test_pico8_planet_gh_row_lq.c tests/test_pico8_ga_export_panel_by.c
  # 7. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^par-nom'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que chaque TU construise et lise l'état de ses entités sur leurs membres typés, afin que D.11e puisse supprimer les lecteurs et écrivains par nom sans qu'un TU ne les nomme encore.
2. En tant que mainteneur du port, je veux que chaque aide locale par nom (`lookup`, `get`, `has`, `get_ref`, `fold_field`, `must_fixed`…) disparaisse avec son dernier appelant, afin qu'aucune lecture par nom ne survive déplacée dans une aide non comptée.
3. En tant que mainteneur du port, je veux que les tests dont l'objet est le lecteur ou l'écrivain par nom (réponse par nom, lecteur brut contre générique, emplacements `field`/`set_field`, cas de mort des `assert` d'écrivain) soient supprimés, comme l'humain l'a décidé (Q11), afin de ne pas maintenir des tests d'une couche condamnée.
4. En tant que gardien des tests, je veux qu'**aucune constante de hachage d'un test gardé ne change**, afin que chaque réécriture typée prouve, par la constante elle-même, qu'elle lit exactement ce que la lecture par nom lisait.
5. En tant que gardien des tests, je veux que les bits qu'un pli tirait de la forme du lecteur (présence d'un membre que la struct a toujours, ou n'a jamais) soient pliés à l'identique et déclarés, afin de garder les constantes sans prétendre qu'ils portent encore une information.
6. En tant que gardien des tests, je veux que chaque test supprimé soit listé au commit avec la raison qui en fait un test de la couche, et chaque couverture perdue listée avec ce qui la reprend (autre TU, e2e, D.11e), afin qu'aucune couverture réelle ne disparaisse en silence.
7. En tant que gardien des tests, je veux une mesure `gcov` avant/après de chaque chunk sur les lignes de `src/` exécutées par ses TU, afin que chaque ligne perdue soit de l'infrastructure par nom (D.11e) ou couverte par un autre TU ou par les e2e.
8. En tant que gardien des tests de régression (§6), je veux que `test_pico8_planet_gh_row_lq` et `test_pico8_ga_export_panel_by` ne changent que leur façon de lire (`ok->le`, `js->lq`, `lq->fs`, `jo.by`), chacun prouvé rouge avec son bug remis, afin que leur objet reste intact.
9. En tant que mainteneur du port, je veux que `test_kind_rec_matches_the_reader` parte (l'oracle de transition de D.11b) mais que `rec` reste comparé à `pico8_X_start` champ par champ, afin que la duplication des littéraux `s`/`od` reste gardée (note de D.11b).
10. En tant que mainteneur du port, je veux que les trois faux descripteurs qui nourrissent un site C de `src/` restent tels quels jusqu'à D.11e, afin que les tests de `dg`, `gd` et `ui:lj` gardent leur objet.
11. En tant que mainteneur du port, je veux que `test_ke_me_by_name` et `test_ke_t_by_name` soient convertis et non supprimés, afin que le `me`/`t` de chaque classe et l'effet de `ke:ks` restent testés.
12. En tant que mainteneur du port, je veux que chaque cast d'un en-tête vers une struct dans un TU suive un test de classe ou de lignée (ou le retour typé d'un constructeur), afin qu'un TU ne lise pas une struct au mauvais type sans le dire.
13. En tant que gardien du compteur, je veux `par-nom` `tests/` à 5 (T3), et au message de chaque commit, par catégorie et par fichier, les lignes et occurrences avant/après, la classification (a1)/(a2)/(a3)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris), les plis de forme déclarés et chaque Δlignes > Δoccurrences justifié, afin qu'aucune baisse ne soit un tassement.
14. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, qu'aucun alias, registre (tableau, `switch` ou boucle qui passe d'un nom à un membre), renommage pour les greps, cale valeur ↔ typé ou littéral remplaçant un identifiant compté ne soit ajouté, afin que la baisse veuille dire « retiré ».
15. En tant que gardien des filets, je veux `make test` et `make e2e` verts à chaque commit et le balayage ASan+UBSan des TU touchés propre, afin de prouver qu'aucun TU ne lit hors de sa struct.
16. En tant que `plan-pilot`, je veux une note « Découvert à D.11d » qui dise les comptes réels, les 47 tests supprimés, les T3 laissés à D.11e, les plis de forme, la couverture perdue et la tension §6/ligne, afin que D.11e parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q11 est une décision humaine (plan, « Décisions de D.9 ») : ce PRD l'applique ; Q12 reste ouverte et n'est pas tranchée ici.

### Modules

- **Aucun module de `src/`**. 38 TU de `tests/` et, au dernier commit, `docs/plan-3.md` (note).
- **Aucun fichier nouveau**, aucun TU supprimé (chaque TU garde au moins un test) : le `Makefile` ne change pas.

### Critère T1 / T2 / T3 (G)

- **T2** : un test (ou la partie d'un test) dont l'**objet** est la réponse du lecteur ou de l'écrivain par nom : présence ou genre d'un nom, nom étranger refusé, méthode lue comme champ, lecteur brut égal au générique, lecteur d'une classe parente, emplacements `field`/`set_field` d'un descripteur, écriture par nom acceptée ou arrêtée (`must_abort`), lecteur de sorte (`kind->field`) et sa garde « prêt ». L'objet est lu dans le commentaire d'en-tête du test et dans ses assertions ; le nom du test ne suffit pas (voir « Tests au nom trompeur »).
- **T1** : tout le reste qui passe par un nom : un test qui construit un état, lance un comportement (constructeur, méthode, action de jeu) et lit l'état qui en résulte, ou le plie dans une constante.
- **T3** : une ligne qui ne lit ni n'écrit par nom elle-même, mais **fournit** un lecteur ou un écrivain par nom à un site C de `src/` que D.11e convertit (faux descripteurs de TU).
- **Tests mixtes** (les « `*_is_a_struct` », `check_fresh_*`, `test_ke_me_by_name`…) : gardés ; leurs lignes T2 partent une à une, leurs lignes T1 sont typées. Un pli de hachage d'un test gardé qui contenait une réponse de lecteur garde ses bits (voir « Plis »).
- Rejeté : supprimer tout test qui nomme un champ par nom (perd les tests de comportement, dont `ke:ks`) ; garder les T2 jusqu'à D.11e (contraire à la ligne ; voir « D.11e »).

### Réécriture typée (G)

- **Receveur** : le TU connaît son type. Un pointeur venu d'un constructeur typé (`pico8_nr_new` rend `pico8_nr_t *`) est utilisé tel quel ; un en-tête (élément de `hv`/`cg`, `void *` d'une aide) est casté **après** un test dans la même fonction : classe exacte (`h->cls == pico8_X_struct_class()`), `pico8_jo_is_family`, `pico8_qh_of`/`pico8_nl_of`, `pico8_g__of` (H-D11c). Une aide locale `as_X(e)` qui asserte la classe puis caste est admise (une par type et par TU ; elle prend un objet, jamais un nom).
- **Membres déplacés par D.11c** : `->emitter->X`, `->probe->X` (H-D11c-4) ; la présence et le genre se lisent sur `has_*`/`*_is_bool` du composant.
- **Écriture (W)** : reproduit exactement l'effet de l'écrivain de `src/` pour ce nom (membre, drapeau `has_*`, `*_is_bool`, conversion de faux/nil) ; l'implémenteur lit l'écrivain avant chaque W et cite la ligne au message si l'effet n'est pas une simple affectation.
- **Singletons** : `pico8_ok()` (typé `pico8_ui_t *`), `pico8_je_nf()` (NULL sur le remplaçant), `pico8_fm_hy()` (NULL sur le provisoire ; `pico8_fm_placeholder()` pour `jk`/`nz`), `pico8_mo_qb()`.
- **Interdits** (liant) : une aide qui passe d'un nom à un membre (`strcmp`, `switch`, table de noms associée à des pointeurs ou des décalages : registre) ; `offsetof` ; reconstruire un `pico8_value_t` à partir d'un membre typé **pour le seul pli** (cale valeur ↔ typé) ; un cast d'en-tête sans test.
- **Tables de noms** (`static const char *const x_names[]` parcourues par une aide par nom) : elles meurent avec leurs boucles ; un pli qui les parcourait devient une suite explicite de membres typés (ou un tableau de **valeurs** typées, sans nom), dans le même ordre.

### Plis de hachage : identiques au bit près (G)

- **Toute constante `*_HASH`/`identity[]` d'un test gardé reste la même**. C'est la preuve de la conversion : un pli typé qui ne rend pas la constante lit autre chose que ce que lisait le nom ; on corrige la réécriture, **jamais** la constante (leçon « constante mise à jour »). Une constante qui ne peut pas rester égale est un arrêt, rapporté (§7 : intention du test).
- Chaque TU garde son encodage (ses propres littéraux de genre : `0xF1u` fixe, `0xB0u` booléen, `0x57u` chaîne, `0xDEADu` absent, `0x4Eu` + rang dans `hv`, `0xEEEEu` + `pico8_type_tag_of`) ; `fold_field(h, e, "x")` devient une aide typée par genre (`fold_fixed_member(h, v)`, `fold_bool_member(h, b)`, `fold_absent(h)`…), qui prend une **valeur**, jamais un nom, et plie les mêmes octets dans le même ordre.
- **Plis de forme (p)** : un bit que le pli tirait d'une question au lecteur dont la réponse est fixée par la struct — `has(e, "x")` pour un membre toujours présent (1) ou qui n'existe pas dans la struct (0), `v.kind` d'un membre de genre unique — est plié à l'identique, depuis la donnée typée quand elle existe (`has_x`, `x != NULL`, `x_is_bool`), sinon par la constante que la forme garantit. **Ce n'est pas l'alias de D.7c** (un identifiant **compté** remplacé par sa valeur) : aucun identifiant compté n'est remplacé, la réponse était calculée à l'exécution et la struct la fixe désormais à la compilation. Chaque pli de forme est **déclaré** au message (fichier, fonction, nombre de bits de forme), et un commentaire au-dessus du bloc le dit (piège : « constant by the struct's shape, kept so that `X_HASH` still proves the typed reads »). Le genre plié comme énumérateur (`(uint32_t)v.kind` de `me_of` dans `test_pico8_ui`) reste `PICO8_VALUE_STRING` par son nom : la ligne gagne `valeur`, déclarée (un littéral serait l'alias de D.7c).
- Rejeté : recalculer les constantes après suppression des bits de forme (auto-référent : la nouvelle valeur ne prouve rien) ; garder `fold_value(h, pico8_value_fixed(m))` (cale valeur ↔ typé, nouvelles lignes `valeur` que Q12 devra retirer).

### Sortes : la comparaison rec ↔ `pico8_X_start` (G, note de D.11b)

- `test_kind_rec_matches_the_reader` (T2 : l'oracle de transition de D.11b, rec contre lecteur) **part**.
- `test_kind_reader_serves_the_generated_defaults` (T1) **devient la comparaison exigée** : ses 36 assertions `kind_string(i, "rr") == pico8_nr_start.rr`, `kind_fixed(i, "s") == pico8_nr_start.s`, `!kind_has(8, "od")`… lisent `kind_of(i)->rec` (`rec->rr == pico8_nr_start.rr`, `rec->has_s && rec->s == pico8_nr_start.s`, `!rec->has_od && rec->od == 0`…), même table de vérité, mêmes pointeurs de chaînes. C'est exactement « garder une comparaison rec ↔ `pico8_X_start` » ; les constantes nommées partagées (l'autre option de la note) ne sont pas retenues : elles changeraient `src/`.
- `test_kind_fields_and_build` (`KIND_HASH`) plie la présence et la valeur lues sur `rec` au lieu du lecteur, au bit près.
- `kind_field`, `kind_string`, `kind_fixed`, `kind_has` meurent ; `test_pico8_map_tables` perd `test_kinds_answer_nothing_once_down` et `kind_answer` (T2 : la garde « prêt » du lecteur de sorte n'est pas dans `rec`, note de D.11b).
- Après D.11d, plus aucun TU ne nomme le `field` des sortes : D.11e peut le supprimer (condition posée par la note de D.11b).

### D.11e : lecteurs dont les seuls tests partent (G)

La consigne demande de trancher chaque lecteur dont le seul test part ici. Réponse, par famille :

| Lecteur ou écrivain (supprimé par D.11e) | Tests directs à `a917e3a` | Encore atteint par `src/` après D.11c ? | Décision |
|---|---|---|---|
| lecteurs `field` des 40 descripteurs, pour les noms que lisent les sites C (`p_`, `fa`, `ra`, `kq`, `ma`, `p`, `dw`, `fo`, `ql`, `m_`, `fg`, `fx`, `mt`) | T2 de chaque classe | oui, par les sites C (`bootstrap.c:87`, `entity_geom.c`, `planet_display.c:484`, `slipway.c:133`/`:263`, `ui.c:304-367`, `wormhole.c:70`, `wormhole_display.c:41`) | T2 supprimés ; la valeur de ces champs reste couverte par les T1 qui exercent ces chemins (survol, `gd`, `dg`, `ui:lj`, `jc:hq`) et par les 26 e2e (mesure `gcov` exigée) ; D.11e écrit son oracle accesseurs = lecteurs sur des lecteurs encore vivants (ligne D.11e) |
| lecteurs `field` pour tous les autres noms, lecteurs des structs-vues (`ie_ke_view`, `ha_pz`, `mz_jj`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `pj_cr`) | T2 | non (consommateurs typés depuis D.11a–D.11c) | code mort jusqu'à D.11e : supprimés sans reprise |
| écrivains `set_field` et leurs `assert` de genre | cas de mort T2 | oui pour `m_` (`ui.c:359`/`:363`), `ma` (`entity_geom.c:82`), `fo` (`slipway.c:263`) | T2 supprimés ; chacun de ces appels de `src/` construit sa valeur par un constructeur au genre fixe (`pico8_value_bool`, `pico8_value_fixed`, `pico8_value_ref`) : l'`assert` ne peut pas y mordre ; D.11e les supprime |
| `kind_field` des 11 sortes | T2 (`map_tables`, `entity_kind`) | non (K à D.11b) | supprimés ; `rec` comparé à `pico8_X_start` (ci-dessus) |
| `pico8_entity_get_field`/`set_field` sur une liste (`test_accessors_refuse_a_table`) | T2 (`test_pico8_mo`) | oui (infrastructure) | supprimé : test de l'infrastructure que D.11e retire |

**Aucun test gardé jusqu'à D.11e** hors T3 (G). Rejeté : garder les T2 des noms C jusqu'à D.11e (la ligne D.11d les supprime ici, et l'oracle de D.11e, qui compare accesseurs et lecteurs sur des instances construites, donne au moment de la conversion la seule preuve qui compte) ; convertir un T2 en T1 (son objet, la réponse par nom, n'a pas de forme typée).

### §6 : `planet_gh_row_lq` et `ga_export_panel_by` (G)

- **Seule la façon de lire change**, aucune assertion retirée ni affaiblie, mêmes valeurs attendues :
  - `ok_le()` (`gh_row_lq:43-53`, `ga_export_panel_by:34-41`) : `pico8_ok()->le` (typé `pico8_ui_t *`), assert non NULL ;
  - `button_lq()` (`gh_row_lq:54-65`, `ga_export_panel_by:43-51`) : l'élément de `le` est asserté `js` (`cls == pico8_js_struct_class()`) puis `((const pico8_js_t *)button)->lq`, assert non NULL ;
  - `fs` (`gh_row_lq:167`) : `exports_lq->fs`, assert non NULL, `count == 4` ;
  - `planet_by_ref()` (`ga_export_panel_by:86-92`) : `planet->jo.by`. Son commentaire (« Read through the field interface, not the class defaults… ») est réécrit dans le même sens : le membre de l'instance, pas le défaut de classe — l'intention ne change pas, ce n'est pas un arrêt §7.
- **Preuve rouge** sur le code final, dans l'arbre de travail, une mutation à la fois, fichier sauvegardé et restauré par `cp`, binaire lancé comme le `Makefile` (`make tmp/test_pico8_planet_gh_row_lq && ASAN_OPTIONS=detect_leaks=0 ./tmp/test_pico8_planet_gh_row_lq` ; `make tmp/test_pico8_ga_export_panel_by && ./tmp/test_pico8_ga_export_panel_by`). Le bug d'origine (table ob lue comme liste) n'est plus exprimable depuis D.8 ; on remet son **symptôme** sur le chemin que la ligne réécrite lit :
  - **R1** (`gh_row_lq`, bug 8361468, « the entry count came out of the list's capacity slot ») : dans `pico8_js_lq` (`ui_button.c`, cas liste), `keys.count = list->count + 1` → rouge attendu : ASan (lecture hors allocation) ou `!lq_item(exports_lq, 5)`/`count == 4` ;
  - **R2** (`ga_export_panel_by`, bug 9183fee, « `by` n'est pas la liste que lisent ses lecteurs ») : dans l'init de `ga` (`planet_family.c`, `self->jo.by = &g_ga_by_shared;`), la ligne retirée → rouge attendu à `assert(by != NULL)` (lecture réécrite) ;
  - **R3** (les deux, `ok->le`) : dans `pz` (`msgbox.c`, `ok->le = le_heap;`), `ok->le` laissé à la liste vide → rouge attendu à `le->count >= 2` / `exports_lq != NULL` ;
  - **R4** (les deux, `js->lq`) : dans `pico8_js_pq`, `self->lq` construit depuis `h_`/`h_row` au lieu de `lq`/`lq_row` → rouge attendu : rangée « exports: » introuvable ;
  - **R5** (`gh_row_lq`, `lq->fs`) : dans `lq:pq` (`layout.c`), une entrée de moins dans `fs_heap` après `pico8_nv` → rouge attendu à `count == 4`.
  Le commit note, pour chacune : la mutation, l'échec observé (assertion ou rapport ASan), la restauration, le vert. **Si une mutation ne rougit pas son test, c'est un arrêt (§7), pas un test durci.**
- Les binaires `tmp/test_pico8_planet_gh_row_lq_refaudit` et `tmp/test_pico8_ga_export_panel_by_refaudit` compilent (cibles du `Makefile`, `make ref-audit` n'est pas exigé).

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la lecture par nom est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours ; alias (macro, `typedef`, pointeur de fonction, enveloppe qui renomme un lecteur) ; registre (voir « Réécriture typée ») ; cale valeur ↔ typé ; lecture par nom déplacée dans une aide non comptée ou hors de `tests/*.c` ; littéral remplaçant un identifiant compté ; tout changement d'une constante de hachage d'un test gardé ; toute écriture sous `src/`.
- **Classification exigée**, au message de chaque commit, pour chaque ligne `par-nom` retirée (recette : `git diff -U0` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) :
  - **(a1) site typé** : lecture ou écriture par nom remplacée par l'accès au membre (T1-R, W, S) ;
  - **(a2) construction supprimée** : test T2, aide morte, cible de cas de mort (T1-A, T2-*) ;
  - **(a3) commentaire** dont le contrat devient faux, réécrit (T1-C : `test_pico8_hp.c:2`, `test_pico8_hy.c:2`, `:66`, `test_pico8_lq.c:1`, `test_pico8_trade.c:3`) ;
  - **(b)** : signature dont le type change — attendu 0 (une aide qui perd son paramètre de nom et prend un objet ou une valeur est comptée (a2) + nouvelle aide, déclarée) ;
  - **(c)** : même donnée, autre nom — **attendu 0**.
- **Mesure exigée**, avant et après chaque commit, par catégorie (`valeur`, `type-tag`, `ref-audit`, `par-nom`) et **par fichier** du diff : lignes (`--detail`) **et** occurrences (recette de « État de départ »).
- **Lignes qui gagnent une catégorie**, par fichier et par catégorie, « aucune » compris. Attendues : `valeur` pour les genres pliés comme énumérateurs (`PICO8_VALUE_STRING` de `me_of`) et pour les membres qui sont eux-mêmes des `pico8_value_t` (éléments de `lq.items`, clés `pico8_js_keys_t.lq`/`h_`/`jb`) ; `type-tag` si un test de classe passe par `pico8_type_tag_of` ; aucune autre.
- **Plis de forme (p)** : par fichier et par fonction, le nombre de bits pliés depuis la forme.
- **Δlignes > Δoccurrences** : chaque couple justifié (attendu : aucun ; des lignes à deux occurrences partent, d'où Δoccurrences ≥ Δlignes).
- **Tests supprimés** : chacun nommé, avec sa raison (code D/M/N de l'Annexe C) et la couverture qu'il emporte (voir « Couverture perdue »). **Constantes supprimées** : chacune nommée, avec le test T2 qui était le seul à la nommer.
- **Numéros de ligne** : ceux du **départ du commit** (HEAD avant le commit), pas ceux de ce PRD si D.11c les a décalés (H-D11c-3).

### Empreinte et découpage (G)

| Chunk | TU (`tests/test_pico8_*.c`) | Chemins | `par-nom` `tests/` retirées (lignes / occ.) | À typer (comptées + appels) | Supprimées (comptées + appels) | Tests T2 entiers |
|---|---|---|---|---|---|---|
| d1 | `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`, `hp`, `hy`, `trade`, `lq`, `entity_kind`, `map_tables`, `ie`, `ga_export_panel_by` (§6) | 18 | 198 / 235 | 72 + 806 | 126 + 194 | 14 |
| d2 | `ui`, `js`, `jc`, `pj`, `g_`, `fv`, `mz`, `bg`, `qb`, `nf`, `iv`, `fq`, `mh`, `fm`, `je`, `mo`, `probe_mm_rnd_order`, `jb_ol`, `planet_gh_row_lq` (§6), `spec_defaults` ; `docs/plan-3.md` | 21 | 364 / 382 (5 T3 restent) | 121 + 750 | 243 + 731 | 33 |

- d1 : 18 chemins ; d2 : 20 TU + `docs/plan-3.md` = 21 chemins : sous les ~40 de la règle.
- **Pourquoi ce découpage** : d1 regroupe l'arbre `jo` et ce qui s'y branche (lignées `qh`/`hj`, `ga`, recherche `hp`, économie `hy`, routes, rangées `lq`, sortes et icônes `ie`) ; d2 l'interface (`ui`, `js`, `pj`, `fq`, `mh`, `iv`, `bg`), la sonde et ses cibles (`g_`, `fv`, `mz`, `jc`), les singletons (`fm`, `je`, `mo`, `qb`, `nf`). Les cinq TU que D.11c touche (`fv`, `ui`, `jc`, `g_`, `jo`) sont en d2 sauf `jo` : d1 dépend le moins des numéros ⟲. Charge équilibrée : d1 ≈ 880 lignes à typer (72 comptées + 806 appels) et ≈ 320 à supprimer ; d2 ≈ 870 à typer (121 + 750) et ≈ 970 à supprimer.
- **Pourquoi deux commits, pas plus** : la ligne en fixe deux ; chacun se prouve seul (TU verts, constantes égales, `gcov`, mutations). Un chunk qui déborde son budget n'est pas coupé en cours de route : l'implémenteur s'arrête au plafond et le note (voir « Budget »).

### Effet sur `make data-residue` ⟲

| Catégorie | Départ (H-D11c-1) | Après d1 | Après d2 | Pourquoi |
|---|---|---|---|---|
| `par-nom` `tests/` | 567 / 622 occ. | 369 / 387 | **5 / 5** | exact : d1 retire les 198 lignes de ses 18 TU, d2 les 364 non-T3 des siens |
| `par-nom` `src/` | 292 / 352 | 292 | 292 | aucun fichier de `src/` |
| `par-nom` total | 859 | 661 | **297** | la ligne dit ≈ 286 : +11, soit `src/` 292 au lieu des 286 de l'étude (+6) et les 5 T3 |
| `par-nom` fichiers | 69 | 51 | **34** | 31 de `src/` + 3 TU (T3) |
| `valeur` | rapporté | forte baisse, rapportée | forte baisse, rapportée | les `pico8_value_t` des aides et des tests T2 partent ; hausses déclarées |
| `type-tag` | rapporté | baisse, rapportée | baisse, rapportée | `pico8_type_tag_of` des tests T2 ; ceux des tests gardés restent |
| `ref-audit` | 0 dans `tests/` | 0 | 0 | — |

« exact » est une égalité ; « rapporté » est mesuré au commit, écart expliqué.

### Documentation (G)

- `docs/plan-3.md` (d2) : note « Découvert à D.11d » en fin de ligne D.11d, rien d'autre. Elle dit : les comptes réels ; les 47 tests supprimés (nombre, par TU) et les constantes parties ; les T3 laissés à D.11e (3 TU, 5 lignes, sites C servis) ; les plis de forme (nombre, TU) ; la comparaison rec ↔ `pico8_X_start` portée par `test_kind_reader_serves_the_generated_defaults` ; la couverture perdue (lignes `gcov`, toutes d'infrastructure ou couvertes par e2e) ; la tension « §6 : supprimés dans le commit qui supprime le code » contre la ligne D.11d, tranchée pour la ligne (G), à faire relire par `plan-review` si le pilote le juge utile ; le volume réel (≈ 2 500 appels d'aides par nom non comptés, que l'étude ne voyait pas), pour le budget de D.12d.
- `docs/glossaire-noms.md`, `docs/trous-de-portage.md` : inchangés (aucun nom minifié ne change de sens, aucun écart port/cart créé).
- Commentaires des TU : quatre sortes de `CLAUDE.md` ; un commentaire qui décrit une lecture par nom devenue typée est réécrit ou supprimé ; aucun renvoi à D.11d ni au plan.

### Ce qui ne change pas

- `src/`, `tools/`, le `Makefile`, `tests/e2e/`, tout `*.sha256`, tout fichier d'événements ; aucun e2e ajouté ; aucun TU ajouté ni supprimé.
- Les constantes `*_HASH`/`identity[]` des tests gardés ; les assertions des tests gardés hors de leurs lignes T2 ; les tests sans ligne ni appel par nom.
- Les trois faux descripteurs T3.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `-Wall -Wextra -Werror` (`Makefile` l. 99) rend fatale toute fonction `static` ou tout tableau `static const` devenu inutilisé : les aides, tables de noms, `must_abort*`, `zero_union`… que seuls des T2 utilisaient partent dans le même commit (grep de chaque fonction après suppression).
- **H2** : aucun TU ne devient vide ; aucun n'est retiré du `Makefile`.
- **H3** : chaque constante d'un test gardé est atteignable au bit près par des lectures typées (plis de forme compris). Une constante qui ne l'est pas : arrêt rapporté, jamais une mise à jour.
- **H4** : les tests T2 n'exécutent aucune ligne de `src/` que ni un test gardé ni les 26 e2e n'exécutent, hors infrastructure par nom (à mesurer : « Couverture perdue ») ; une ligne qui fait exception reçoit une assertion dans un test gardé de sa classe.
- **H5** : aucun test T1 n'écrit par nom un champ dont l'écrivain de `src/` fait plus qu'une affectation sans que ce PRD le voie ; l'implémenteur lit l'écrivain avant chaque W (leçon D.11a).
- **H6** : les 5 lignes T3 et leurs sites C sont intacts après D.11c (H-D11c-7).

## Implementation Chunks

Deux commits, dans cet ordre, chacun vert seul (`make test`, `make e2e`, `make data-guard`), avec ses comptes, sa classification, ses lignes gagnées, ses plis de forme, son `gcov`, ses mutations et, pour d2, la note :

1. **d1 — arbre `jo`, recherche, économie, sortes** (18 TU) : `test_pico8_jo`, `_qh`, `_nl`, `_os`, `_hj`, `_pm`, `_mx`, `_nr`, `_fr`, `_ga`, `_hp`, `_hy`, `_trade`, `_lq`, `_entity_kind`, `_map_tables`, `_ie`, `_ga_export_panel_by` (§6, R2-R4). `par-nom` `tests/` −198 lignes (−235 occurrences). 14 tests supprimés.
2. **d2 — interface, sonde, voies, singletons** (20 TU + le plan) : `test_pico8_ui`, `_js`, `_jc`, `_pj`, `_g_`, `_fv`, `_mz`, `_bg`, `_qb`, `_nf`, `_iv`, `_fq`, `_mh`, `_fm`, `_je`, `_mo`, `_probe_mm_rnd_order`, `_jb_ol`, `_planet_gh_row_lq` (§6, R1, R3-R5), `_spec_defaults` ; `docs/plan-3.md`. `par-nom` `tests/` −364 lignes (−382 occurrences), 5 T3 restent. 33 tests supprimés.

Ordre conseillé dans chaque commit :
1. mesure de départ (lignes et occurrences par fichier), `gcov` de départ des TU du chunk ;
2. par TU : supprimer les T2 (tests, appels dans `main`, aides mortes), puis typer les T1 en gardant chaque constante ; `make tmp/test_pico8_<nom>` et le lancer comme le `Makefile` après chaque TU ;
3. `make test`, `make e2e` ;
4. `gcov` d'arrivée, diff, classement des lignes perdues ; mutations (restaurées par `cp`) ; balayage ASan+UBSan des TU du chunk ;
5. mesure d'arrivée, classification, lignes gagnées, plis de forme, greps de la « Sortie » ;
6. (d2) note ; message.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (valeurs, présences, identités, effets d'un appel), pas la présence d'une ligne. D.11d ne change aucun code testé : il change la **façon dont les tests lisent**. La preuve :
- chaque test gardé garde ses assertions et **ses constantes** (le pli typé rend la même constante que le pli par nom : c'est la preuve que la lecture typée lit la même chose) ;
- chaque test supprimé est un test de la couche (Q11), listé, sa couverture perdue mesurée ;
- les tests du §6 sont prouvés rouges ;
- les 26 e2e restent conformes.

### Couverture perdue (G), et pourquoi elle est acceptable

| Ce que les T2 testaient | Ce qui le reprend | Pourquoi c'est acceptable |
|---|---|---|
| la réponse par nom de chaque lecteur (présence, genre, absent, étranger), lecteur brut = générique, lecteur hérité | rien : l'objet disparaît à D.11e | c'est la couche (Q11) ; les **valeurs** des membres restent pliées par les T1 typés (constantes inchangées) |
| les `assert` de genre des écrivains (cas de mort) | rien | les écrivains partent à D.11e ; leurs trois familles d'appelants de `src/` passent des valeurs de genre fixe (voir « D.11e ») |
| les emplacements `field`/`set_field` des descripteurs | rien | idem ; D.11e retire les emplacements |
| le lecteur de sorte et sa garde « prêt » | `rec` comparé à `pico8_X_start` | `rec` est statique, sans garde (note de D.11b) |
| les lectures par nom des structs-vues | les consommateurs typés (`pz` sur son cadre, `dl_show`, `probe_et_qs`, `jc_end_p`) et leurs TU | lecteurs morts depuis D.11a–D.11c |
| les **comportements** qu'un T2 lançait pour atteindre un état (constructeurs, `hq`, `bv`, `ey`, `dg`, écritures de `src/`) | les T1 des TU de chaque classe (`test_built`, `test_life`, `test_hq`, `test_bv`…) et les 26 e2e | **mesuré** : `gcov` avant/après (ci-dessous) ; toute ligne perdue hors infrastructure reçoit une assertion dans un test gardé, ou est couverte par un e2e |
| `pico8_entity_get_field`/`set_field` sur une liste (`test_accessors_refuse_a_table`) | rien | infrastructure retirée à D.11e |

**Mesure `gcov` exigée par chunk** : dans une copie jetable du scratchpad, les TU du chunk compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`), lancés comme le `Makefile`, au départ et à l'arrivée ; union des lignes de `src/` exécutées ; `L` = départ \ arrivée. Chaque ligne de `L` est classée : (i) infrastructure par nom (lecteur `field`, écrivain `set_field`, `kind_field`, aide `*_take_*`, `pico8_entity_get_field`/`set_field`, lecteur de struct-vue) ; (ii) exécutée par un autre TU (union de tous les TU gardés) ; (iii) exécutée par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`, un rejeu instrumenté des 26 e2e, pas un TU). Une ligne ni (i), ni (ii), ni (iii) : une assertion est ajoutée dans un test gardé de sa classe (règle ci-dessous), listée au message. Le décompte (i)/(ii)/(iii) va au message et à la note.

### TU touchés (G)

1. **Les 38 TU de l'inventaire**, ni plus ni moins. Un TU qui casse sans nommer de lecture par nom est un **bug de D.11d** : il part à `help-me-debug`.
2. **Tests supprimés** : les 47 de la liste, leurs appels dans `main`/`in_fresh_child`/`spawn`, leurs aides devenues mortes (H1), les constantes que seuls eux nommaient.
3. **Tests gardés amputés** : les lignes T2 des « `*_is_a_struct` » (emplacements `field`/`set_field`, lecteur brut, `must_abort` sur un écrivain, lecture d'une méthode comme champ) ; le reste du test (classe, enregistrement, entrées de méthodes, `hd`, tags) reste mot pour mot.
4. **`test_ke_me_by_name`, `test_ke_t_by_name`** (`test_pico8_ui`) : `me_of` lit `((const pico8_ke_t *)e)->me`/`->t` (tout hôte enchâsse `pico8_ke_t` en premier) ; les comparaisons au lecteur brut (`me_of`, l. 1047), à `pico8_hj_struct_class()->field` (l. 1170, 1334) et les intrus (l. 1153, 1300) partent ; les `fnv(h, 0u)` que la boucle d'intrus pliait sont des plis de forme, gardés et déclarés ; `KE_ME_HASH`, `KE_T_HASH` inchangées.
5. **Sortes** : voir « Sortes ».
6. **Constantes** : aucune d'un test gardé ne change ; une constante qui rougit est corrigée dans la réécriture, jamais dans la constante.
7. **Assertion nouvelle** : admise seulement pour couvrir une ligne de `src/` perdue par la suppression d'un T2 (classe (iii) absente), dans un test gardé de la classe, en accès typé, listée au message avec la ligne qu'elle couvre.
8. **Prior art** : réécritures de TU sans changer de constante (D.7c, trois commits, « aucune constante `*_HASH` ne change ») ; tests de régression réécrits dans leur seule façon de lire, preuve rouge par bug remis (1a.34) ; singletons typés (D.11a) ; `pico8_qh_of` (D.11b) ; test mixte « `*_is_a_struct` » (1a).

### Mutations

Une à la fois, dans l'arbre de travail, fichier sauvegardé par `cp` avant et restauré par `cp` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan). Chaque mutation doit rougir au moins le TU nommé ; sinon c'est un trou, couvert par une assertion (règle 7) ou justifié :

- d1 :
  - **R2, R3, R4** (§6, `ga_export_panel_by`) ;
  - **M1** : le `struct_init` de `nl` ne pose pas `qw` depuis `pico8_nl_start` → `test_pico8_nl` (pli typé de `qw`) ;
  - **M2** : `pico8_hj_struct_init` ne recopie pas `k_` dans son émetteur → `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx` (plis typés `->emitter->k_`) ;
  - **M3** : le constructeur de `trade` ne pose pas `qe` → `test_pico8_trade` ;
  - **M4** : dans `pico8_game_data.c`, le `s` de `pico8_nr_kind_rec` diffère de `pico8_nr_start.s` → `test_pico8_entity_kind` (la comparaison rec ↔ start mord) ;
  - **M5** : dans `ie:kj`, la condition sur `m_` inversée → `test_pico8_ie` (`test_draw`, écriture typée de `m_`), sinon trou justifié.
- d2 :
  - **R1, R3, R4, R5** (§6, `planet_gh_row_lq`) ;
  - **M6** : `pico8_ke_ks(self, NULL)` laisse `me` inchangé → `test_pico8_ui` (`KE_ME_HASH`, lecture typée) ;
  - **M7** : `ui:gw` ne remplit pas `ic` → `test_pico8_ui`/`test_pico8_bg`/`test_pico8_fq` (`pico8_ok()->ic`) ;
  - **M8** : `jc:hq` ne pose pas `probe->has_et` → `test_pico8_jc` (lecture typée) ;
  - **M9** : `mz:hq` ne pose pas `jj` dans `probe->et` → `test_pico8_mz` ;
  - **M10** : `qb:ph` ne décroît pas `ma` → `test_pico8_qb` (`pico8_mo_qb()->ma`) ;
  - **M11** : `pico8_nf_new` ne partage pas `l`/`r` → `test_pico8_nf`.
- Les lignes et fonctions exactes sont celles du HEAD de départ du commit (H-D11c) ; une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même membre, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit. Aucun e2e ajouté.
- `make class-coverage-net`, `make sanitize-net` : **non exigés** (ligne de l'étape : aucun fichier de `src/`).
- **Balayage ASan+UBSan des TU du chunk** : compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`. Sortie propre (un cast au mauvais type dans un TU est ce que ce balayage voit).
- Aucun TU ne rejoue un fichier d'événements ; aucune assertion ajoutée n'est lente.

### Vérification, budget et tableau « Runs »

**Budget : environ 3 heures de temps réel pour l'étape** (d1 ≈ 80 min, d2 ≈ 100 min), mesures, `gcov`, mutations et filets compris ; **plafond 5 heures**. Base : D.11b ≈ 15 min pour 22 chemins (≈ 0,7 min par chemin) couvre le coût fixe par fichier (38 chemins ≈ 27 min : build, lancement, mesure) ; mais D.11b changeait quelques dizaines de lignes, D.11d en réécrit ≈ 1 750 au bit près (193 comptées T1 + 1 556 appels) et en supprime ≈ 1 300 (369 + 925) ; à ≈ 12 lignes typées par minute (mécanique, chaque pli vérifié par sa constante) ≈ 145 min, plus ≈ 15 min de suppressions et d'aides mortes, plus ≈ 20 min de `gcov`, mutations et balayage. La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute |
| `make e2e` | ~1 s |
| `make line-coverage EVENTS="tests/e2e/*.json"` | moins d'une minute (26 rejeux instrumentés), une fois par chunk |
| `gcov` des TU du chunk (départ et arrivée) | quelques minutes par chunk |
| mutations (d1 : 8 ; d2 : 10, chacune un `make` ciblé) | < 15 min en tout |
| balayage ASan+UBSan des TU du chunk | quelques minutes par chunk |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; un chunk inachevé n'est pas commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + chunk + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | départ, avant d1 | 0 | `par-nom 859 69` (H-D11c-1) | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que les TU de son chunk (et `docs/plan-3.md` pour d2). Ni `src/`, ni `tools/`, ni `Makefile`, ni `tests/e2e/`, ni `*.sha256`.
2. **Greps** de la « Sortie », exactement.
3. **Constantes** : `git diff <départ> -- tests | grep -E '^[-+].*(_HASH|identity\[)'` : seules des suppressions de constantes que seul un test supprimé nommait, chacune listée.
4. **Tests supprimés** : exactement les 47 de la liste (plus ou moins un, justifié) ; aucun test dont l'objet est un comportement (critère « T1 / T2 / T3 »).
5. **Casts** : chaque cast d'un en-tête ou d'un `void *` vers une struct, ajouté dans un TU, suit un test de classe ou de lignée dans la même fonction, ou part du retour typé d'un constructeur.
6. **Registre et cales** : aucune aide de TU ne passe d'un nom à un membre (grep 3 et 4 de la « Sortie ») ; aucun `pico8_value_*()` construit pour le seul pli ; aucun `offsetof`.
7. **Plis de forme** : déclarés, commentés, comptés ; aucun ne remplace un identifiant compté.
8. **§6** : diff limité aux lectures ; preuve rouge R1-R5 au message ; commentaire de `planet_by_ref` réécrit dans le même sens.
9. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (b) = 0, (c) = 0 ; lignes gagnées listées.
10. **Couverture** : `gcov` avant/après et classement (i)/(ii)/(iii) au message ; assertions ajoutées listées avec leur ligne.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.11d ni au plan dans `tests/`.
12. **Docs** : note « Découvert à D.11d » en fin de ligne D.11d ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si D.11c n'est pas gradée, ou si le HEAD de départ n'est pas celui de sa graduation (ou un descendant qui ne touche que `docs/`), propre ; ou si `make data-residue` n'y rend pas `par-nom` `tests/` 567 lignes sur 38 fichiers sans explication (H-D11c-1/2) ; ou si les hypothèses H-D11c n'ont pas été confrontées à la note « Découvert à D.11c » (ce PRD doit alors être passé de `next/` à `triage/` par `plan-pilot`, revu).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change au-delà de sa façon de lire (assertion retirée ou affaiblie, valeur attendue changée), si sa preuve rouge (bug remis, échec observé, correctif restauré, vert) manque au message, ou si un bug remis ne le rend pas rouge ; ou si un test devrait changer d'intention pour passer.
- `FAILED:` si le balayage ASan+UBSan des TU touchés n'est pas propre, à l'un des deux commits.
- `FAILED:` si une ligne ne tombe sous aucune des classes T1/T2/T3 (arrêt « champ sans règle » du §7), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.

**Périmètre et forme**

- `FAILED:` si un fichier de `src/`, de `tools/` ou le `Makefile` change.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si une ligne T3 est retirée ou modifiée, ou si une autre ligne `par-nom` reste dans `tests/`.
- `FAILED:` si un cast vers une struct ajouté dans un TU n'a pas son test de classe ou de lignée dans la même fonction (hors retour typé d'un constructeur).
- `FAILED:` si une aide de TU passe d'un nom à un membre (registre), si un `pico8_value_t` est construit pour le seul pli, ou si `offsetof` apparaît.
- `FAILED:` si un test dont l'objet est un comportement est supprimé (en particulier `test_ke_me_by_name`, `test_ke_t_by_name`, `test_g__t_key_stops_off_row`, `test_jb_state_by_name`, `test_mh_nil_lh_key_reads_absent`, les trois `test_defaults_match_the_parsed_spec`, `test_kind_reader_serves_the_generated_defaults`, `test_kind_fields_and_build`, les T3).
- `FAILED:` si la comparaison rec ↔ `pico8_X_start` disparaît (grep 5).

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert.
- `FAILED:` si `par-nom` `tests/` ne vaut pas 369 après d1 et 5 après d2, ou si `par-nom` `src/` change.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, registre, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, cale valeur ↔ typé, lecture par nom déplacée dans une aide non comptée, littéral remplaçant un identifiant compté, (b) ou (c) non déclarée.
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(a3)/(b)/(c) ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; les plis de forme ; la justification de chaque Δlignes > Δoccurrences ; les tests et constantes supprimés avec leur raison ; le classement `gcov` (i)/(ii)/(iii). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` d'un test gardé change.
- `FAILED:` si une ligne de `src/` perdue par les TU (`gcov`) n'est ni de l'infrastructure par nom, ni couverte par un autre TU, ni par les e2e, sans assertion ajoutée ni justification.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si le plafond de 5 heures est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.11d », ou si elle omet l'un de ces éléments : les comptes réels, les tests supprimés, les T3 et leurs sites C, les plis de forme, la comparaison rec ↔ `pico8_X_start`, la couverture perdue, la tension §6/ligne.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Tout `src/`** : les sites C et `jc.mt`, les accesseurs de Q6, les lecteurs `field`, écrivains `set_field`, `kind_field`, aides `*_take_*`, `pico8_entity_get_field`/`set_field`, structs-vues (D.11e).
- **Les T3** (trois faux descripteurs) : D.11e, premier chunk, avec les sites C qu'ils servent.
- Les listes des TU (`pico8_list_t`, `PICO8_REF_AS_LIST*`) : D.12d. Les `pico8_value_t` des TU hors des aides par nom : `valeur`, après Q12.
- Tout nouveau test hors des assertions de la règle 7 ; tout e2e.
- `tools/`, le `Makefile`, `tests/e2e/`, `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **Périmètre** : les 567 lignes et les ≈ 2 480 appels non comptés des aides qu'elles définissent (une aide ne meurt qu'avec son dernier appelant). Rejeté : ne traiter que les lignes comptées (laisserait les aides, donc les lignes).
  - **Critère T2** : l'objet du test (commentaire d'en-tête, assertions), pas son nom ; `test_ke_me_by_name`/`test_ke_t_by_name` et quatre autres « `*_by_name` » sont T1.
  - **T3** : trois faux descripteurs gardés pour D.11e (5 lignes), ce qui porte le compte à ≈ 297 au lieu de ≈ 286.
  - **Plis** : constantes inchangées, plis typés au bit près, plis de forme déclarés. Rejetés : constantes recalculées, `pico8_value_t` reconstruits pour plier.
  - **Sortes** : la comparaison rec ↔ `pico8_X_start` portée par le test converti des défauts de sortes.
  - **D.11e** : aucun T2 gardé ; la valeur des champs C reste couverte par les T1 et les e2e, mesurée par `gcov` ; la tension avec le §6 est rapportée.
  - **§6** : cinq mutations R1-R5 qui remettent le symptôme de chaque bug sur le chemin réécrit.
  - **Découpage** : d1 (arbre `jo`, 18 TU), d2 (interface, sonde, singletons, 20 TU + le plan).
- **Surprise de mesure, pour `plan-pilot`** : l'étude D.9 et la ligne comptaient 576 puis 567 lignes ; le travail réel est dominé par les ≈ 2 480 appels non comptés des aides par nom (≈ 1 556 à typer). Le budget de ce PRD est calculé sur ce volume, pas sur le nombre de chemins. D.12d (`liste` des TU, 431 lignes, 38 fichiers) aura sans doute la même forme : ses aides locales (`list_size`, `rank_in`, `listed`) seront à compter de la même façon.
- **Tension §6 / ligne D.11d** : le §6 fait supprimer un test de la couche « dans le commit qui supprime le code testé » ; la ligne D.11d, ajoutée avec les décisions de D.9, les supprime ici. Ce PRD suit la ligne (plus précise, postérieure, approuvée) et le rapporte dans la note ; si `plan-pilot` préfère aligner le texte du §6, c'est un changement de méthode pour `plan-review`, pas pour l'implémenteur.
- **Inventaire exhaustif** : Annexe C (une ligne par fonction et par classe, numéros à `a917e3a`). Les numéros de `test_pico8_fv`, `_ui`, `_jc`, `_g_`, `_jo` sont à reprendre après D.11c (H-D11c-3) ; le contenu, non (H-D11c-2).

## Annexe A — textes du plan recopiés (`docs/plan-3.md` à `a917e3a`, par extraction de lignes)

### §5, phase D, en entier (l. 228-280) : puces, note de D.0, « Liste de D » avec empreinte, règles communes et ordre, arrêt « champ sans règle », décisions de D.9, comptes au départ de D.10, lignes D.2 à D.12 avec toutes leurs notes « Découvert à D.x »

> #### Phase D — Données
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
> - D.11c — `par-nom`, classe M (Q7) : émetteur de particules (`ei`, `ez`, `f_`, `g__mm`) et sonde (`bv`, `dl`, `d_`) → composants portés par pointeur, règle 2 de la phase 2 appliquée à ces deux groupes seulement. Empreinte ≈ 15 (15, 0) : `particle`, `probe` et les 13 fichiers des classes qui les portent. Compte : `par-nom` ≈ 862. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : oui.
> - D.11d — `par-nom`, TU : T1 (mise en état et lecture d'état) en accès aux membres typés ; T2 (tests des descripteurs, tests de mort des `assert` d'écrivain) supprimés comme tests de la couche (Q11, §6). Deux chunks d'environ 19 fichiers. Empreinte 38 (0, 38). Compte : `par-nom` ≈ 286 (l'infrastructure de `src/`). §6 : `test_pico8_planet_gh_row_lq` et `test_pico8_ga_export_panel_by` (3 lignes chacun) lisent les membres, preuve rouge. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change.
> - D.11e — `par-nom`, suppression de l'infrastructure. Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`, laissé à D.11a ; accesseur typé par champ dans la table de classe, NULL = absent, Q6 ; l'oracle accesseurs = lecteurs naît et meurt dans l'étape). Découvert à D.11b (pour D.11d/D.11e) : les littéraux `s`/`od` des `pico8_X_kind_rec` doublent ceux de `pico8_X_start` ; seul `test_kind_rec_matches_the_reader` en garde l'égalité — à sa suppression, garder une comparaison rec ↔ `pico8_X_start` ou des constantes nommées partagées. Puis : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks. Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non.
> - D.12a — `liste`, registres : catégorie `liste` ajoutée à `tools/data_residue.sh` dans un premier commit seul, motif `\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h` (Q10 ; la règle de D.0 fige les motifs existants, mais une catégorie nouvelle s'ajoute dans son propre commit avec son motif énoncé) ; conteneur de pointeurs (Q9) sous un nom hors de ce motif ; registres L1 (`hv`, `cv[5]`, `cg[*]`, calques de `dc`, `ui.ic`, `ui.le`, `g_ga_by_shared`) et leurs parcours. Empreinte ≈ 15 (15, 0) : `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud`. Compte : `liste` ≤ 927 moins L1, rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
> - D.12b — `liste`, listes membres L2 (`jo.oe`, `jo.cc`/`by`, `trade.lp`/`rl`, `mz.ox`, proposition `ib`, `qy`, `jc.ts`, `hy.invented`, `jo.jy`/`hp.i` ; `gen.oc` en tableau fixe) et aides L4 de `collect` (`jp`, `nv`, `gu`/`gu_content`, `la_append` ; prédicat `int (*)(void *, void *ctx)`). Empreinte ≈ 25 (25, 0). Compte : `liste` rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
> - D.12c — `liste`, listes hétérogènes L3 et locales L4 : `lq.items`, `mh.items`, `jb.ls` → union étiquetée locale à `layout` {rien, fixe, chaîne, rangée, autre-vrai} (Q10), `lq.fs` en tableau typé de fixes ; listes locales d'affichage (`planet_display`, `menu`, `msgbox:pz`, `trade:kg`, `research*`, `score_panel*`, `tech_gate`, `slipway`, `probe`, `galaxy`) en tableaux fixes là où la borne est dans le cart. Empreinte ≈ 16 (15, 1). Compte : `liste` = 0 dans `src/` hors `pico8_list.c/.h`, `pico8_ref_audit.h` et `pico8_type_tag.h` (49 lignes aujourd'hui, à D.12d). §6 : `layout_nil_item_draws_nothing` passe son item par l'union, même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
> - D.12d — `liste`, TU (431 lignes, 38 fichiers ; `test_pico8_list` réécrit sur le nouveau conteneur, il ne perd pas son objet, §6), en deux chunks ; `pico8_list.c/.h` supprimés, `PICO8_REF_AS_LIST*` et `PICO8_TYPE_TAG_LIST` retirés. Empreinte 42 (4, 38). Compte : `liste` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non.

### §6 et §7 (l. 302-332)

> ### 6. Tests existants
>
> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
> - Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés** par ce plan. Depuis la fusion de master (e2e rapide, #32), seuls `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` restent ; les autres ont été remplacés par les e2e de `tests/e2e/`, qui en portent la garde. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.
>
> ### 7. Exécution
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

## Annexe B — étude D.9 (`docs/prd/done/plan3-phaseD-etape9-etude.md` à `a917e3a`) : T1/T2, D.11d et Q11

### §1.2 (l. 49-55)

> ### 1.2 Inventaire, `tests/` (576 lignes, 38 fichiers)
>
> `pico8_entity_get_field` : 248 lignes. `pico8_entity_set_field` : 200. Appels directs des descripteurs `cls->field`/`->set_field` : 145. Lectures de sorte : 3. Les noms les plus lus sont `fo` 31, `m_` 26, `p_` 21, `p` 17, `ra` 15, `et` 14. Deux formes :
>
> - **T1** — mise en état et lecture d'état d'un TU : accès aux membres typés (`qh->jo.mi = …; qh->jo.has_mi = 1;`). Le TU connaît son type.
> - **T2** — tests des descripteurs eux-mêmes (lecture par nom, refus d'un genre hors rangée, `assert` « champ sans règle » en mort) : leur objet disparaît avec les descripteurs. Ils sont supprimés comme tests de la couche (§6, premier point ; Q11).
> - §6 : `test_pico8_planet_gh_row_lq` (3 lignes) et `test_pico8_ga_export_panel_by` (3 lignes) sont réécrits pour lire les membres, avec leur preuve rouge.

### Étapes proposées, avec leurs comptes (l. 181-205)

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
> ---

### Q11 (l. 241-242)

> **Q11. Tests des descripteurs (T2 : environ 145 lignes `cls->field`/`set_field` et les tests de mort des `assert` d'écrivain) : les supprimer avec les descripteurs, comme tests de la couche (§6, premier point) ?**
> Oui. Leur objet, la lecture et l'écriture par nom, disparaît. Les valeurs des membres restent couvertes par les TU en accès typé (T1) et par les e2e.

## Annexe C — inventaire ligne à ligne (`tests/` à `a917e3a`, 567 lignes `par-nom`)

Classe et code : voir « Inventaire » (A, R, W, S, C : T1 ; D, M, N, X : T2 ; F : T3). « — » : le code suffit. Les lignes d’une même fonction et d’une même classe sont regroupées. Numéros ⟲ pour `test_pico8_fv`, `_ui`, `_jc`, `_g_`, `_jo` (H-D11c-3). Les appels non comptés des aides par nom ne sont pas listés un à un : leur nombre est donné par fichier, et chaque aide qu’ils appellent est une ligne T1-A ou T2 ci-dessous.

#### `test_pico8_bg.c` — 22 lignes : T1 12, T2 10 ; appels d'aides par nom non comptés : 32 à typer, 17 supprimés

Tests T2 supprimés en entier : `test_bg_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `BG_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 99 | `has` | T1-A | — |
| 104 | `get` | T1-A | — |
| 110 | `must_fixed` | T1-A | — |
| 116 | `is_string` | T1-A | — |
| 122 | `fold_vec` | T1-A | — |
| 134 | `bool_state` | T1-A | — |
| 145 | `ok_ref` | T1-A | — |
| 184 | `find_bg` | T1-R | objet trouvé par sa classe (`cls == pico8_bg_struct_class()`) et `fa` typé ; même objet, asserté |
| 185 | `find_bg` | T1-R | idem |
| 208 | `vec_of` | T1-A | — |
| 218 | `pl_of` | T1-R | — |
| 381 | `in_layer` | T1-S | — |
| 565 | `must_abort` | T2-M | — |
| 590, 593 | `test_bg_is_a_struct` | T2-N | — |
| 604 | `test_bg_is_a_struct` | T2-D | — |
| 609 | `fold_member` | T2-D | — |
| 610 | `fold_member` | T2-X | — |
| 668, 670 | `write_m_` | T2-M | — |
| 741 | `test_bg_members_by_name` | T2-D | — |
| 750 | `write_m_by_name` | T2-M | — |

#### `test_pico8_entity_kind.c` — 25 lignes : T1 1, T2 24 ; appels d'aides par nom non comptés : 40 à typer, 44 supprimés

Tests T2 supprimés en entier : `test_ie_ke_view_by_name`, `test_ie_members_by_name`, `test_ie_writer_stops_off_row`, `test_kind_rec_matches_the_reader` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `BEHAVIOR_HASH`, `IE_MEMBERS_HASH`, `RAW_PRESENCE_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 76 | `kind_field` | T1-A | `kind_field` meurt ; ses appelants T1 lisent `kind_of(i)->rec` (voir « Sortes ») |
| 306, 317 | `ie_bool_state` | T2-X | — |
| 346, 383 | `test_ie_ke_view_by_name` | T2-D | — |
| 347, 384, 385, 395, 412, 416, 429 | `test_ie_ke_view_by_name` | T2-X | — |
| 434, 435 | `ie_read` | T2-D | — |
| 527 | `must_abort` | T2-M | — |
| 599, 691 | `test_ie_members_by_name` | T2-D | — |
| 601, 655, 658, 662, 665, 680 | `test_ie_members_by_name` | T2-X | — |
| 720 | `write_m_by_name` | T2-M | — |
| 741 | `test_ie_writer_stops_off_row` | T2-X | — |

#### `test_pico8_fm.c` — 6 lignes : T1 1, T2 5 ; appels d'aides par nom non comptés : 10 à typer, 13 supprimés

Tests T2 supprimés en entier : `test_fm_members_by_name`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 48 | `lookup` | T1-A | — |
| 178 | `write_jk` | T2-M | — |
| 215 | `test_fm_is_a_struct` | T2-D | — |
| 229, 231 | `assert_raw_absent` | T2-D | — |
| 237 | `assert_raw_fixed` | T2-D | — |

#### `test_pico8_fq.c` — 22 lignes : T1 9, T2 13 ; appels d'aides par nom non comptés : 3 à typer, 8 supprimés

Tests T2 supprimés en entier : `test_fq_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `FQ_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 82 | `fold_field` | T1-A | — |
| 106 | `ok_o_` | T1-S | — |
| 184 | `test_bare_veil_swallows_the_click_and_stays` | T1-R | — |
| 206 | `test_false_my_keeps_the_veil` | T1-R | — |
| 218 | `in_layer` | T1-S | — |
| 241, 261 | `test_hover_loop_on_the_veil` | T1-R | — |
| 274, 280 | `test_defaults_match_the_parsed_spec` | T1-R | — |
| 277, 281 | `test_defaults_match_the_parsed_spec` | T2-N | — |
| 317, 320 | `test_fq_is_a_struct_registered_under_hd` | T2-N | — |
| 344, 345 | `fold_raw` | T2-D | — |
| 408 | `must_abort` | T2-M | — |
| 440, 464 | `test_fq_members_by_name` | T2-X | — |
| 466, 478, 485 | `test_fq_members_by_name` | T2-D | — |
| 497 | `write_m_by_name` | T2-M | — |

#### `test_pico8_fr.c` — 14 lignes : T1 4, T2 10 ; appels d'aides par nom non comptés : 54 à typer, 14 supprimés

Tests T2 supprimés en entier : `test_fr_mt_by_name`, `test_nr_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `FR_MT_HASH`, `NR_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 106 | `lookup` | T1-A | — |
| 116 | `get` | T1-A | — |
| 448, 457 | `test_life` | T1-W | — |
| 489 | `write_fixed` | T2-M | — |
| 532, 535 | `read_both` | T2-D | — |
| 566, 574, 578 | `test_fr_is_a_struct` | T2-D | — |
| 620, 628 | `fold_hb` | T2-X | — |
| 661, 662 | `test_nr_members_by_name` | T2-D | — |

#### `test_pico8_fv.c` — 12 lignes : T1 3, T2 9 ; appels d'aides par nom non comptés : 59 à typer, 26 supprimés

Tests T2 supprimés en entier : `test_fv_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `FV_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 92 | `lookup` | T1-A | — |
| 102 | `get` | T1-A | — |
| 308 | `in_layer` | T1-S | — |
| 548 | `write_fixed` | T2-M | — |
| 601, 602 | `test_fv_is_a_struct` | T2-D | — |
| 633, 634 | `read_both` | T2-D | — |
| 701, 702 | `write_m_` | T2-M | — |
| 737 | `fold_written_by_name` | T2-D | — |
| 788 | `write_off_row` | T2-M | — |

#### `test_pico8_g_.c` — 29 lignes : T1 5, T2 24 ; appels d'aides par nom non comptés : 79 à typer, 87 supprimés

Tests T2 supprimés en entier : `test_bv_et_members_by_name`, `test_bv_et_writer_stops_off_row`, `test_g__members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `BV_ET_MEMBERS_HASH`, `G__MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 109 | `lookup` | T1-A | — |
| 119 | `get` | T1-A | — |
| 250 | `vec_of` | T1-A | — |
| 270 | `the_bg` | T1-R | idem `find_bg` de `test_pico8_bg` |
| 271 | `the_bg` | T1-R | idem |
| 672 | `write_fixed` | T2-M | — |
| 738, 749 | `test_g__is_a_struct` | T2-D | — |
| 784 | `test_bv_et_is_a_struct` | T2-N | — |
| 794, 795 | `fold_bv_et_fo` | T2-D | — |
| 870, 918, 933 | `test_bv_et_members_by_name` | T2-D | — |
| 871, 916, 921, 925, 927, 929 | `test_bv_et_members_by_name` | T2-X | — |
| 942 | `write_fo_by_name` | T2-M | — |
| 1014, 1015 | `read_both` | T2-D | — |
| 1031, 1032 | `write_by_name` | T2-M | — |
| 1072 | `np_target_at` | T2-X | — |
| 1368, 1371 | `test_g__members_by_name` | T2-D | — |
| 1418 | `write_off_row` | T2-M | — |

#### `test_pico8_ga.c` — 15 lignes : T1 3, T2 12 ; appels d'aides par nom non comptés : 71 à typer, 32 supprimés

Tests T2 supprimés en entier : `test_ga_members_by_name`, `test_ga_writer_stops_off_row` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `GA_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 103 | `lookup` | T1-A | — |
| 113 | `get` | T1-A | — |
| 551 | `test_life` | T1-W | — |
| 644 | `write_fixed` | T2-M | — |
| 714, 722, 725, 738 | `test_ga_is_a_struct` | T2-D | — |
| 764, 765 | `read_both` | T2-D | — |
| 885, 899 | `test_ga_members_by_name` | T2-D | — |
| 887, 890 | `test_ga_members_by_name` | T2-X | — |
| 919 | `write_off_row` | T2-M | — |

#### `test_pico8_ga_export_panel_by.c` — 3 lignes : T1 3, T2 0 ; appels d'aides par nom non comptés : 0 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 38 | `ok_le` | T1-R | §6 : `pico8_ok()->le` |
| 49 | `button_lq` | T1-R | §6 : `((const pico8_js_t *)button)->lq`, classe `js` assertée |
| 89 | `planet_by_ref` | T1-R | §6 : `planet->jo.by` (membre de l'instance, pas le défaut de classe : l'intention du commentaire tient) |

#### `test_pico8_hj.c` — 7 lignes : T1 4, T2 3 ; appels d'aides par nom non comptés : 57 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 109 | `lookup` | T1-A | — |
| 119 | `get` | T1-A | — |
| 397 | `credits` | T1-S | — |
| 479 | `test_life` | T1-W | — |
| 550 | `write_fixed` | T2-M | — |
| 570 | `write_id` | T2-M | — |
| 620 | `test_hj_is_a_struct` | T2-D | — |

#### `test_pico8_hp.c` — 9 lignes : T1 4, T2 5 ; appels d'aides par nom non comptés : 51 à typer, 22 supprimés

Tests T2 supprimés en entier : `test_hp_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `RAW_PRESENCE_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 2 | `(en-tête)` | T1-C | — |
| 96 | `lookup` | T1-A | — |
| 338 | `pos_of` | T1-R | — |
| 349 | `must_fixed` | T1-A | — |
| 822 | `write_px` | T2-M | — |
| 944 | `test_hp_is_a_struct` | T2-D | — |
| 1002 | `raw_read` | T2-D | — |
| 1159, 1197 | `test_hp_members_by_name` | T2-X | — |

#### `test_pico8_hy.c` — 9 lignes : T1 5, T2 4 ; appels d'aides par nom non comptés : 56 à typer, 29 supprimés

Tests T2 supprimés en entier : `test_hy_members_by_name`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 2, 66 | `(en-tête)` | T1-C | — |
| 98 | `lookup` | T1-A | — |
| 225 | `pos_of` | T1-R | — |
| 236 | `must_fixed` | T1-A | — |
| 667 | `write_nz` | T2-M | — |
| 697 | `test_hy_is_a_struct` | T2-D | — |
| 716 | `read_present` | T2-D | — |
| 724 | `read_absent` | T2-D | — |

#### `test_pico8_ie.c` — 22 lignes : T1 12, T2 10 ; appels d'aides par nom non comptés : 81 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 89 | `lookup` | T1-A | — |
| 99 | `get` | T1-A | — |
| 110 | `must_fixed` | T1-A | — |
| 116 | `is_string` | T1-A | — |
| 133 | `vec_of` | T1-A | — |
| 319 | `check_fresh_icon` | T2-N | — |
| 354 | `test_construction` | T2-N | — |
| 511, 516, 520 | `test_draw` | T1-W | — |
| 535 | `invent` | T1-S | — |
| 570 | `test_ke_view` | T1-S | — |
| 577, 579 | `test_ke_view` | T1-W | — |
| 648 | `write_kh` | T2-M | — |
| 652 | `write_ra` | T2-M | — |
| 656 | `write_gy` | T2-M | — |
| 660 | `write_id` | T2-M | — |
| 688, 713 | `test_ie_is_a_struct` | T2-N | — |
| 722 | `write_od` | T2-M | — |
| 758 | `test_ke_view_is_a_struct` | T2-N | — |

#### `test_pico8_iv.c` — 8 lignes : T1 2, T2 4, T3 2 ; appels d'aides par nom non comptés : 13 à typer, 23 supprimés

Tests T2 supprimés en entier : `test_iv_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `IV_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 90 | `field` | T1-A | — |
| 191 | `test_mz_scan_skips_a_live_toast` | T1-R | — |
| 224, 229 | `fake_toast_field` | T3-F | — |
| 323 | `same_value` | T2-D | — |
| 328, 329 | `read_member` | T2-D | — |
| 395 | `must_abort_write` | T2-M | — |

#### `test_pico8_jb_ol.c` — 1 lignes : T1 0, T2 1 ; appels d'aides par nom non comptés : 0 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 140 | `test_jb_state_by_name` | T2-D | — |

#### `test_pico8_jc.c` — 56 lignes : T1 10, T2 46 ; appels d'aides par nom non comptés : 132 à typer, 110 supprimés

Tests T2 supprimés en entier : `test_jc_members_by_name`, `test_jc_writer_stops_off_row`, `test_mouse_point_members_by_name`, `test_too_far_members_by_name`, `test_too_far_writer_stops_off_row` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `FAR_MEMBERS_HASH`, `JC_MEMBERS_HASH`, `MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 103 | `lookup` | T1-A | — |
| 113 | `get` | T1-A | — |
| 124 | `must_fixed` | T1-A | — |
| 130 | `is_string` | T1-A | — |
| 342 | `pos_of` | T1-R | — |
| 433 | `vec_of` | T1-A | — |
| 467 | `check_fresh_jc` | T1-R | `jc->probe->fp`, `fp_is_bool` (H-D11c-4) ; plié comme le booléen faux d'aujourd'hui |
| 478 | `check_fresh_jc` | T1-R | — |
| 649, 661 | `test_draw` | T1-W | — |
| 904 | `write_re` | T2-M | — |
| 908 | `write_nh` | T2-M | — |
| 912 | `write_fa` | T2-M | — |
| 936, 938 | `test_jc_is_a_struct` | T2-N | — |
| 946 | `write_qs` | T2-M | — |
| 950 | `write_ra` | T2-M | — |
| 980 | `test_too_far_is_a_struct` | T2-N | — |
| 986 | `write_p` | T2-M | — |
| 990 | `write_fo` | T2-M | — |
| 1031, 1032 | `read_member` | T2-D | — |
| 1127, 1128 | `read_far` | T2-D | — |
| 1179, 1185, 1189 | `test_too_far_members_by_name` | T2-X | — |
| 1182, 1194 | `test_too_far_members_by_name` | T2-D | — |
| 1212 | `write_fo_fixed` | T2-M | — |
| 1216 | `write_fo_false` | T2-M | — |
| 1226 | `write_fo_zero_false` | T2-M | — |
| 1230 | `write_fo_string` | T2-M | — |
| 1234 | `write_fo_planet` | T2-M | — |
| 1238 | `write_fo_vec` | T2-M | — |
| 1277, 1278 | `read_jc` | T2-D | — |
| 1517, 1523, 1525, 1526, 1528, 1534, 1537, 1539, 1542, 1544, 1547, 1548, 1552, 1553 | `test_jc_members_by_name` | T2-X | — |
| 1520, 1557, 1561, 1567 | `test_jc_members_by_name` | T2-D | — |
| 1586 | `write_off_row` | T2-M | — |

#### `test_pico8_je.c` — 5 lignes : T1 4, T2 1 ; appels d'aides par nom non comptés : 0 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 72 | `assert_placeholder` | T2-N | — |
| 82, 84 | `assert_an_nf` | T1-S | — |
| 103 | `test_lj_on_placeholder` | T1-R | `pico8_ok()->lheld == NULL && ->rheld == NULL` |
| 104 | `test_lj_on_placeholder` | T1-R | `pico8_ok()->qa` |

#### `test_pico8_jo.c` — 5 lignes : T1 2, T2 3 ; appels d'aides par nom non comptés : 38 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 91 | `lookup` | T1-A | — |
| 101 | `get` | T1-A | — |
| 555 | `write_ra` | T2-M | — |
| 559 | `write_id` | T2-M | — |
| 563 | `write_hp` | T2-M | — |

#### `test_pico8_js.c` — 31 lignes : T1 11, T2 20 ; appels d'aides par nom non comptés : 158 à typer, 11 supprimés

Tests T2 supprimés en entier : `test_ha_pz_members_by_name`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 113 | `lookup` | T1-A | — |
| 123 | `get` | T1-A | — |
| 138 | `must_fixed` | T1-A | — |
| 144 | `is_string` | T1-A | — |
| 161 | `vec_of` | T1-A | — |
| 435 | `title_button` | T1-R | — |
| 445 | `pos_of` | T1-R | — |
| 456 | `world_fixed` | T1-A | — |
| 636 | `invent` | T1-S | — |
| 982, 986 | `test_draw` | T1-W | — |
| 1056 | `write_ra` | T2-M | — |
| 1060 | `write_my` | T2-M | — |
| 1064 | `write_mb` | T2-M | — |
| 1093, 1095 | `test_js_is_a_struct` | T2-N | — |
| 1134, 1135 | `read_both` | T2-D | — |
| 1280, 1281 | `write_named` | T2-M | — |
| 1338, 1345 | `fold_written_by_name` | T2-D | — |
| 1685 | `write_off_row` | T2-M | — |
| 1744 | `write_hi` | T2-M | — |
| 1748 | `write_ca` | T2-M | — |
| 1752 | `write_ep` | T2-M | — |
| 1791, 1792 | `test_ha_pz_is_a_struct` | T2-N | — |
| 1817 | `ha_pz_vec_as_ha` | T2-X | — |
| 1825 | `ha_pz_ca_is_14` | T2-X | — |
| 1831 | `ha_pz_misses` | T2-D | — |

#### `test_pico8_lq.c` — 7 lignes : T1 4, T2 3 ; appels d'aides par nom non comptés : 28 à typer, 18 supprimés

Tests T2 supprimés en entier : `test_lq_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `PANEL_PRESENCE_HASH`, `RAW_PRESENCE_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 1 | `(en-tête)` | T1-C | — |
| 77 | `lookup` | T1-A | — |
| 587 | `pos_of` | T1-R | — |
| 598 | `must_fixed` | T1-A | — |
| 769 | `write_w` | T2-M | — |
| 819 | `test_lq_is_a_struct` | T2-D | — |
| 852 | `fold_raw` | T2-D | — |

#### `test_pico8_map_tables.c` — 1 lignes : T1 0, T2 1 ; appels d'aides par nom non comptés : 0 à typer, 10 supprimés

Tests T2 supprimés en entier : `test_kinds_answer_nothing_once_down`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 181 | `kind_answer` | T2-D | — |

#### `test_pico8_mh.c` — 11 lignes : T1 10, T2 1 ; appels d'aides par nom non comptés : 3 à typer, 15 supprimés

Tests T2 supprimés en entier : `test_mh_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `MEMBERS_BY_NAME_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 94 | `fold_field` | T1-A | — |
| 240, 243 | `test_two_years_in_a_row` | T1-R | — |
| 284 | `test_scores_box_follows_c_` | T1-R | — |
| 312, 315, 318, 322, 323 | `test_defaults_match_the_parsed_spec` | T1-R | — |
| 386 | `fold_member` | T2-X | — |
| 405 | `has_member` | T1-A | — |

#### `test_pico8_mo.c` — 7 lignes : T1 2, T2 4, T3 1 ; appels d'aides par nom non comptés : 0 à typer, 0 supprimés

Tests T2 supprimés en entier : `test_accessors_refuse_a_table`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 67 | `assert_placeholder` | T2-N | — |
| 83 | `assert_a_qb` | T1-S | — |
| 107 | `ra_only_field` | T3-F | — |
| 138 | `test_lj_on_placeholder` | T1-R | `pico8_ok()->qa` |
| 197 | `test_placeholder_is_a_struct` | T2-N | — |
| 223 | `get_ra` | T2-D | — |
| 227 | `set_ra` | T2-D | — |

#### `test_pico8_mx.c` — 10 lignes : T1 4, T2 6 ; appels d'aides par nom non comptés : 56 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 110 | `lookup` | T1-A | — |
| 120 | `get` | T1-A | — |
| 469 | `test_life` | T1-W | — |
| 503 | `give_row` | T1-W | — |
| 579 | `write_fixed` | T2-M | — |
| 633, 637, 640 | `test_mx_is_a_struct` | T2-D | — |
| 653, 654 | `test_mx_is_a_struct` | T2-N | — |

#### `test_pico8_mz.c` — 64 lignes : T1 27, T2 37 ; appels d'aides par nom non comptés : 45 à typer, 58 supprimés

Tests T2 supprimés en entier : `test_jj_qs_by_name`, `test_mz_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `MZ_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 95 | `has` | T1-A | — |
| 100 | `get` | T1-A | — |
| 106 | `must_fixed` | T1-A | — |
| 112 | `is_string` | T1-A | — |
| 117 | `fold_fixed` | T1-A | — |
| 127 | `fold_vec` | T1-A | — |
| 139 | `bool_state` | T1-A | — |
| 150 | `fold_string` | T1-A | — |
| 166 | `et_kind` | T1-R | `mz->probe->et`/`has_et` (H-D11c-4) |
| 217 | `pos_of` | T1-R | — |
| 302 | `vec_of` | T1-A | — |
| 338, 344, 347 | `check_fresh_mz` | T1-R | — |
| 349 | `check_fresh_mz` | T1-R | `((const pico8_mz_jj_t *)jj)->qs` |
| 389 | `fold_ox` | T1-R | — |
| 410 | `test_ox` | T1-R | — |
| 428 | `fold_hq` | T1-R | `mz->probe->et` (H-D11c-4) |
| 430 | `fold_hq` | T1-R | `qs` de la cible : `pico8_mz_jj_t` ou `pico8_jc_too_far_t`, classe testée |
| 435 | `fold_hq` | T1-R | `fo` de la cible typée |
| 438 | `fold_hq` | T1-R | — |
| 581 | `paid_mz_on_unknown` | T1-W | — |
| 718 | `check_fo_is` | T1-R | `mz->probe->et` (H-D11c-4) |
| 719 | `check_fo_is` | T1-R | `fo` de la cible typée |
| 767 | `test_jj_wrapper` | T1-R | `->probe->et` des deux `mz` (H-D11c-4) |
| 805 | `test_mz_is_a_struct` | T1-R | `((pico8_mz_t *)mz)->jj == pico8_mz_jj_wrapper()` |
| 812 | `test_mz_is_a_struct` | T1-W | écriture typée de `fo` du `pico8_mz_jj_t` |
| 823 | `test_mz_is_a_struct` | T2-N | — |
| 843, 862 | `test_jj_qs_by_name` | T2-D | — |
| 846, 848, 855, 856 | `test_jj_qs_by_name` | T2-X | — |
| 881 | `read_mz` | T2-D | — |
| 882 | `read_mz` | T2-X | — |
| 1063, 1068, 1070, 1071, 1075, 1077, 1079, 1086, 1091, 1094, 1095, 1098, 1101, 1103, 1106, 1108, 1111, 1112, 1116, 1117, 1118, 1119 | `test_mz_members_by_name` | T2-X | — |
| 1066, 1089, 1124, 1128, 1134 | `test_mz_members_by_name` | T2-D | — |
| 1176 | `write_off_row` | T2-M | — |

#### `test_pico8_nf.c` — 14 lignes : T1 6, T2 8 ; appels d'aides par nom non comptés : 13 à typer, 11 supprimés

Tests T2 supprimés en entier : `test_nf_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `NF_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 76 | `button` | T1-S | — |
| 98 | `ra_of_je` | T1-S | — |
| 167 | `test_jl_builds_a_mouse_reader` | T2-N | — |
| 246 | `test_placeholder_je` | T1-R | idem `je:103` |
| 247 | `test_placeholder_je` | T1-R | `pico8_ok()->qa` |
| 297 | `test_nf_is_a_struct_sharing_its_buttons` | T2-N | — |
| 298, 299 | `test_nf_is_a_struct_sharing_its_buttons` | T1-R | — |
| 306 | `test_nf_is_a_struct_sharing_its_buttons` | T2-D | — |
| 311 | `fold_member` | T2-D | — |
| 312 | `fold_member` | T2-X | — |
| 380 | `must_abort` | T2-M | — |
| 451, 453 | `test_nf_members_by_name` | T2-X | — |

#### `test_pico8_nl.c` — 21 lignes : T1 5, T2 16 ; appels d'aides par nom non comptés : 40 à typer, 9 supprimés

Tests T2 supprimés en entier : `test_nl_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `NL_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 105 | `lookup` | T1-A | — |
| 115 | `get` | T1-A | — |
| 303 | `fold_plate` | T2-N | — |
| 355 | `credits` | T1-S | — |
| 421, 424 | `test_life` | T1-W | — |
| 455 | `write_rr` | T2-M | — |
| 459 | `write_nl` | T2-M | — |
| 463 | `write_hp` | T2-M | — |
| 467 | `write_id` | T2-M | — |
| 471 | `write_qw` | T2-M | — |
| 517 | `test_nl_is_a_struct` | T2-D | — |
| 522 | `fold_raw` | T2-D | — |
| 569, 576, 581 | `test_nl_members_by_name` | T2-X | — |
| 584, 585, 586, 592 | `test_nl_members_by_name` | T2-D | — |
| 600 | `write_qw_value` | T2-M | — |

#### `test_pico8_nr.c` — 9 lignes : T1 4, T2 5 ; appels d'aides par nom non comptés : 52 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 104 | `lookup` | T1-A | — |
| 114 | `get` | T1-A | — |
| 450, 459 | `test_life` | T1-W | — |
| 506 | `write_fixed` | T2-M | — |
| 559, 569, 573, 586 | `test_nr_is_a_struct` | T2-D | — |

#### `test_pico8_os.c` — 12 lignes : T1 6, T2 6 ; appels d'aides par nom non comptés : 38 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 106 | `lookup` | T1-A | — |
| 116 | `get` | T1-A | — |
| 237 | `clicked_game` | T1-R | — |
| 387 | `credits` | T1-S | — |
| 434 | `export_to` | T1-W | — |
| 446 | `fold_db` | T1-W | copie typée de `mi` et `has_mi` |
| 514 | `write_qw` | T2-M | — |
| 518 | `write_rr` | T2-M | — |
| 522 | `write_hs` | T2-M | — |
| 526 | `write_hp` | T2-M | — |
| 530 | `write_id` | T2-M | — |
| 562 | `test_os_is_a_struct` | T2-D | — |

#### `test_pico8_pj.c` — 14 lignes : T1 2, T2 12 ; appels d'aides par nom non comptés : 92 à typer, 57 supprimés

Tests T2 supprimés en entier : `test_cr_members_by_name`, `test_pj_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `PJ_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 100 | `lookup` | T1-A | — |
| 110 | `get` | T1-A | — |
| 628 | `write_c_` | T2-M | — |
| 681 | `test_pj_is_a_struct` | T2-D | — |
| 693 | `write_ep` | T2-M | — |
| 716 | `test_cr_is_a_struct` | T2-D | — |
| 754 | `cr_misses` | T2-D | — |
| 802 | `pj_read` | T2-D | — |
| 804 | `pj_read` | T2-X | — |
| 878 | `write_c_nil` | T2-M | — |
| 885 | `write_ed_zero_false` | T2-M | — |
| 892 | `write_gz_zero_fixed` | T2-M | — |
| 896 | `write_fa` | T2-M | — |
| 954 | `test_pj_members_by_name` | T2-D | — |

#### `test_pico8_planet_gh_row_lq.c` — 3 lignes : T1 3, T2 0 ; appels d'aides par nom non comptés : 0 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 47 | `ok_le` | T1-R | §6 : `pico8_ok()->le` |
| 60 | `button_lq` | T1-R | §6 : `((const pico8_js_t *)button)->lq`, classe `js` assertée |
| 167 | `test_gh_exports_row_lq_receives_its_four_items` | T1-R | §6 : `exports_lq->fs` |

#### `test_pico8_pm.c` — 7 lignes : T1 3, T2 4 ; appels d'aides par nom non comptés : 58 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 109 | `lookup` | T1-A | — |
| 119 | `get` | T1-A | — |
| 476 | `test_life` | T1-W | — |
| 507 | `write_fixed` | T2-M | — |
| 531 | `write_id` | T2-M | — |
| 567, 571 | `test_pm_is_a_struct` | T2-D | — |

#### `test_pico8_probe_mm_rnd_order.c` — 1 lignes : T1 1, T2 0 ; appels d'aides par nom non comptés : 0 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 47 | `emitted_particle_p` | T1-R | `((pico8_g__t *)probe)->emitter->qy` (H-D11c-4) |

#### `test_pico8_qb.c` — 22 lignes : T1 8, T2 14 ; appels d'aides par nom non comptés : 25 à typer, 29 supprimés

Tests T2 supprimés en entier : `test_qb_members_by_name`, `test_qb_writer_stops_off_row` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `QB_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 82 | `vec_of` | T1-A | — |
| 93 | `ref_of` | T1-A | — |
| 101 | `fixed_of` | T1-A | — |
| 198 | `check_fresh_mo` | T1-R | — |
| 278 | `test_shake_decays` | T1-S | `pico8_mo_qb()->ma` |
| 323 | `test_placeholder_mo` | T1-S | `pico8_mo_qb() == NULL` (remplaçant) |
| 324, 325 | `test_placeholder_mo` | T1-S | idem |
| 432 | `test_qb_is_a_struct` | T2-N | — |
| 458 | `same_value` | T2-D | — |
| 463 | `get2` | T2-D | — |
| 464 | `get2` | T2-X | — |
| 561 | `must_abort` | T2-M | — |
| 575 | `test_qb_members_by_name` | T2-D | — |
| 606, 616, 623, 631, 633 | `test_qb_members_by_name` | T2-X | — |
| 673 | `write_v_by_name` | T2-M | — |
| 677 | `write_ma_by_name` | T2-M | — |
| 711 | `test_qb_writer_stops_off_row` | T2-X | — |

#### `test_pico8_qh.c` — 9 lignes : T1 4, T2 5 ; appels d'aides par nom non comptés : 52 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 105 | `lookup` | T1-A | — |
| 115 | `get` | T1-A | — |
| 349 | `credits` | T1-S | — |
| 511 | `fold_descendant` | T1-W | — |
| 597 | `write_hp` | T2-M | — |
| 601 | `write_id` | T2-M | — |
| 605 | `write_s` | T2-M | — |
| 609 | `write_od` | T2-M | — |
| 613 | `write_et` | T2-M | — |

#### `test_pico8_spec_defaults.c` — 1 lignes : T1 1, T2 0 ; appels d'aides par nom non comptés : 0 à typer, 0 supprimés

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 45 | `nf_l_of` | T1-R | — |

#### `test_pico8_trade.c` — 13 lignes : T1 4, T2 9 ; appels d'aides par nom non comptés : 34 à typer, 16 supprimés

Tests T2 supprimés en entier : `test_trade_members_by_name`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 3 | `(en-tête)` | T1-C | — |
| 76 | `lookup` | T1-A | — |
| 172 | `pos_of` | T1-R | — |
| 183 | `must_fixed` | T1-A | — |
| 455 | `write_qe` | T2-M | — |
| 507 | `test_trade_is_a_struct` | T2-D | — |
| 536 | `trade_misses` | T2-D | — |
| 537 | `trade_misses` | T2-X | — |
| 544 | `trade_qe_is` | T2-X | — |
| 556 | `trade_list_is` | T2-X | — |
| 568, 570, 572 | `test_trade_members_by_name` | T2-X | — |

#### `test_pico8_ui.c` — 40 lignes : T1 4, T2 34, T3 2 ; appels d'aides par nom non comptés : 86 à typer, 266 supprimés

Tests T2 supprimés en entier : `test_hj_members_by_name`, `test_jo_members_by_name`, `test_jo_position_by_name`, `test_jo_state_by_name`, `test_jo_writers_by_name`, `test_jo_written_by_name`, `test_mx_members_by_name`, `test_pm_members_by_name`, `test_qh_members_by_name`, `test_ui_members_by_name` ; constantes nommées dans ces tests (celles qu'aucun test gardé ne nomme partent avec eux) : `HJ_MEMBERS_HASH`, `JO_MEMBERS_HASH`, `JO_POSITION_HASH`, `JO_STATE_HASH`, `JO_WRITERS_HASH`, `JO_WRITTEN_HASH`, `MX_MEMBERS_HASH`, `PM_MEMBERS_HASH`, `QH_MEMBERS_HASH`, `UI_MEMBERS_HASH`.

| Lignes | Fonction | Classe | Précision |
|---|---|---|---|
| 121 | `lookup` | T1-A | — |
| 250 | `pos_of` | T1-R | — |
| 261 | `placed` | T1-R | — |
| 717 | `write_o_` | T2-M | — |
| 740 | `test_ui_is_a_struct` | T2-D | — |
| 757, 758 | `read_ui` | T2-D | — |
| 843, 844 | `giver_set_field` | T3-F | — |
| 874 | `write_named` | T2-M | — |
| 969 | `test_ui_members_by_name` | T2-D | — |
| 1047 | `me_of` | T2-D | — |
| 1048 | `me_of` | T1-A | `me_of` lit `((const pico8_ke_t *)e)->me`/`->t` ; sa comparaison au lecteur brut (l. 1047) part |
| 1153 | `test_ke_me_by_name` | T2-N | — |
| 1170 | `test_ke_me_by_name` | T2-D | — |
| 1300 | `test_ke_t_by_name` | T2-N | — |
| 1334 | `test_ke_t_by_name` | T2-D | — |
| 1546 | `test_jo_members_by_name` | T2-N | — |
| 1583, 1584 | `write_by_name` | T2-M | — |
| 1655 | `write_off_row` | T2-M | — |
| 1825, 1826, 1880 | `test_jo_position_by_name` | T2-D | — |
| 1834, 1835 | `test_jo_position_by_name` | T2-X | — |
| 2137 | `test_jo_state_by_name` | T2-D | — |
| 2207 | `test_jo_state_by_name` | T2-N | — |
| 2440 | `test_jo_writers_by_name` | T2-D | — |
| 2521 | `test_jo_writers_by_name` | T2-N | — |
| 2776 | `np_target_at` | T2-X | — |
| 2874, 2889 | `test_qh_members_by_name` | T2-D | — |
| 3103 | `test_hj_members_by_name` | T2-X | — |
| 3133 | `test_hj_members_by_name` | T2-D | — |
| 3221 | `test_pm_members_by_name` | T2-D | — |
| 3313 | `mx_join_row` | T2-X | — |
| 3365, 3367 | `test_mx_members_by_name` | T2-D | — |
| 3370 | `test_mx_members_by_name` | T2-X | — |
