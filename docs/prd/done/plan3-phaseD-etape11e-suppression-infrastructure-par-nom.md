---
title: "Plan 3, étape D.11e : suppression de l'infrastructure `par-nom` — classe C par accesseurs typés de la table de classe (Q6, NULL = absent) et `jc.mt` par `get_mt`, `dw` par le crochet `probe_of`, `fo` par l'écrivain typé de `bv`, `ma`/`na` par cast vérifié ; oracle accesseurs = lecteurs né en e1, mort en e2 ; puis lecteurs `field`, écrivains `set_field`, `kind_field`, aides `*_take_*`, `pico8_entity_get_field`/`set_field`, `pico8_ke_field`, typedefs et emplacements supprimés ; structs-vues typées ; `par-nom` = 0 dans `DATA_RESIDUE_GUARD` ; trois sous-étapes d'environ 50 min"
description: "Après D.11d, `par-nom` ne compte plus que l'infrastructure de `src/` (292 lignes, 352 occurrences, 31 fichiers, mesuré à `fdb294b`) et 11 lignes de TU (mesuré à `579a748`, D.11d gradée) : 5 lignes T3 de trois faux descripteurs, 6 lectures par nom qu'aucun lecteur typé de `src/` ne permet encore (état partagé `gy`/`id` de `ie`, `fo` de `bv_et`, `ra`/`p` du point souris de `jc`). D.11e convertit les 14 sites de classe C et les 3 lignes de `jc.mt` sur des accesseurs typés de la table de classe (11 membres, 136 lignes d'initialiseurs), prouvés par un oracle accesseurs = lecteurs qui naît et meurt dans l'étape, puis supprime les 38 lecteurs `field` (39 descripteurs), 18 écrivains `set_field` (21 descripteurs), 10 `kind_field`, 35 aides `*_take_*`, `pico8_entity_get_field`/`set_field`, `pico8_ke_field` et les typedefs, après avoir exposé les trois lecteurs typés qui manquaient aux TU (`pico8_ie_gy()`/`pico8_ie_id()`, `pico8_bv_et_t` et `pico8_jc_mouse_point_t` publics) : `par-nom` = 0, gardé par `make data-guard`."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.11d était implémentée** (`plan-pilot`, étape 2b), puis **révisé après sa graduation** : D.11d est gradée (quatre sous-étapes VERIFIED, `6e2bba3` d1, `d2f00fb` d2, `26eff6a` + `7a1e852` d3, `87c1c71` d4, « docs: D.11d gradée » `579a748`). Les hypothèses `H-D11d-n` sont confrontées plus bas à `579a748` et aux notes « Découvert à D.11d1 » à « D.11d4 » ; les sections qui en dépendaient ont été refaites. La première version avait été écrite sur l'arbre **commis** `fdb294b` (« docs: D.11d en quatre sous-étapes, Q6 reconfirmée ; PRD de D.11d en file »), extrait par `git archive fdb294b` dans le scratchpad. Seuls `grep`, `sh tools/data_residue.sh` (avec `--detail`), la recette d'occurrences de D.8 et des scripts de lecture seule (classement des lignes comptées, graphe « qui répond à quel nom » des lecteurs et écrivains) ont tourné. **Rien n'a été compilé ni lancé.** La révision a relu et remesuré l'arbre commis `579a748` (`git archive`, même méthode, rien compilé ni lancé). `src/` n'a pas changé entre `fdb294b` et `579a748` (`git diff --stat` vide sur `src`, `tools`, `Makefile`) : les numéros de `src/` de ce PRD valent aux deux ; les numéros de `tests/` sont ceux de `579a748`.

Le problème, vu du mainteneur du port : après D.11a–D.11d, plus aucun site de `src/` ne lit ni n'écrit par nom, sauf **quatorze sites de classe C** (champs transversaux lus ou écrits sur une entité quelconque : `p_`, `nu`, `fa`, `ra`, `kq`, `ma`, `p`, `dw`, `fo`, `m_`, `ql`, `fg`, `fx`) et **`jc.mt`** (laissé par D.11a à cause d'une contrainte de lien). Tant que ces sites lisent par nom, toute l'infrastructure qui les sert reste vivante : 38 lecteurs `field` (39 descripteurs) et 18 écrivains `set_field` (21 descripteurs), 10 lecteurs de sortes `kind_field`, 35 aides `*_take_*`, `pico8_entity_get_field`/`set_field`, `pico8_ke_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` et leurs emplacements dans `pico8_struct_class_t` et `pico8_entity_kind_t` — environ 2 400 lignes de `src/` dont 271 sont comptées par `par-nom`. Et l'étape suivante de la liste (D.12, les listes) ne peut pas s'ouvrir tant que `par-nom` n'est pas à 0 et gardé (ordre Q1).

Ligne de l'étape, recopiée de `docs/plan-3.md` à `fdb294b` (§5, « Phase D — Données », l. 276), avec la note de D.11b qu'elle porte et la reconfirmation de Q6 par l'humain :

> - D.11e — `par-nom`, suppression de l'infrastructure. Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`, laissé à D.11a ; accesseur typé par champ dans la table de classe, NULL = absent, Q6 ; l'oracle accesseurs = lecteurs naît et meurt dans l'étape). Découvert à D.11b (pour D.11d/D.11e) : les littéraux `s`/`od` des `pico8_X_kind_rec` doublent ceux de `pico8_X_start` ; seul `test_kind_rec_matches_the_reader` en garde l'égalité — à sa suppression, garder une comparaison rec ↔ `pico8_X_start` ou des constantes nommées partagées. Puis : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks. Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non. Q6 reconfirmée (humain, 2026-09-29) après la revue du plan : accesseurs typés par champ, pas de remontée dans `ke`, même si la phase 2 en remplace une partie.

Décisions de D.9, recopiées (l. 265) :

> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Paragraphe « Liste de D », empreinte et règles communes, ordre, recopiés (l. 237, 239, 241) :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.

> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Toutes les autres lignes de la phase D (D.0 à D.12) **avec toutes leurs notes « Découvert à D.x »** (D.0, D.1, D.7c, D.8, D.10a, D.10b, D.10c, D.11a, D.11b, D.11c, et la décision humaine qui découpe D.11d), l'arrêt « champ sans règle », les comptes au départ de D.10, les parties du §5 qui fixent les règles (en-tête et phase 1a, phase 1b, phase D en entier, phases 2 et 3), le §6 et le §7 entiers sont recopiés tels quels, par extraction de lignes, en **Annexe A**. Les passages de l'étude D.9 sur l'infrastructure, la classe C, Q6, Q11 et la ligne D.11e de sa table d'étapes sont en **Annexe B**. L'**Annexe C** recopie l'annexe « Reporté à D.11e » du PRD de D.11b (inventaire C, table « qui répond », accesseurs, oracle, classes de TU, exception `pico8_na`, contrainte de lien de `jc.mt`), réutilisée et remesurée ici. L'**Annexe D** est l'inventaire ligne à ligne des 292 lignes `par-nom` de `src/`.

### Hypothèses `H-D11d-n`, confrontées à `579a748`

Source : le PRD de D.11d (`docs/prd/triage/plan3-phaseD-etape11d-tu-par-nom-membres-types.md` à `fdb294b`) et la ligne D.11d du plan (quatre sous-étapes, décision humaine du 2026-09-29). Chaque hypothèse dit ce qui en dépend ici ; la vérification se fait sur la note « Découvert à D.11d » et sur l'arbre gradé.

| Hypothèse | Énoncé | Ce qui en dépend dans ce PRD | Vérification à la graduation |
|---|---|---|---|
| H-D11d-1 | Après D.11d4, `par-nom` `tests/` = **5 lignes / 5 occurrences** dans 3 fichiers (T3) ; `src/` = **292 / 352** dans 31 fichiers ; total **297**, 34 fichiers ; `DATA_RESIDUE_GUARD` inchangé (`spec,overrides,stash,ob,dispatch`) | « État de départ », comptes de chaque sous-étape, critères `FAILED:` de départ | `make -s data-residue`, `--detail`, recette d'occurrences |
| H-D11d-2 | Les 5 lignes T3 sont les faux descripteurs de `test_pico8_iv` (`fake_toast_field` : `return pico8_ke_field(...)` ; `.field = fake_toast_field,`), `test_pico8_mo` (`g_ra_only_class = {.field = ra_only_field}`) et `test_pico8_ui` (`g_giver_class` : `.field = giver_field,` ; `.set_field = giver_set_field,`), **mêmes contenus** qu'à `fdb294b` (`iv.c:224`, `:229` ; `mo.c:107` ; `ui.c:843`, `:844`), **numéros changés** | e1 : conversion des trois faux (accesseurs `get_p_`, `get_ra`, `get_fa`, `set_m_`) | `grep -nE "$(pattern par-nom)" tests/*.c` rend exactement ces 5 lignes |
| H-D11d-3 | Plus aucun TU ne nomme `pico8_entity_get_field`/`set_field`, `cls->field`/`->set_field`, `pico8_X_struct_class()->field`, `kind->field`, `pico8_X_kind_field`, `pico8_ke_field` (hors T3), une aide `*_take_*`, ni une aide locale à paramètre de nom de membre | e2/e3 : aucun TU ne suit la suppression (hors oracle de e1 et T3) ; empreinte `tests/` de e2 = 1, de e3 = 0 | greps 2, 3 et 4 de la « Sortie » de D.11d |
| H-D11d-4 | `test_kind_rec_matches_the_reader` est parti ; `test_kind_reader_serves_the_generated_defaults` compare `kind_of(i)->rec` à `pico8_X_start` champ par champ (≥ 25 assertions, grep 5 de D.11d) ; `kind_field`/`kind_string`/`kind_fixed`/`kind_has` de `test_pico8_entity_kind` et `kind_answer` de `test_pico8_map_tables` sont partis | e2 supprime le `field` des sortes sans toucher un TU ; la note de D.11b reste honorée | grep 5 de D.11d ≥ 25 |
| H-D11d-5 | Les 47 tests T2 sont partis : aucun lecteur ni écrivain par nom n'a plus de test direct (table « D.11e : lecteurs dont les seuls tests partent » du PRD de D.11d) ; la valeur des champs C reste couverte par des T1 qui exercent les chemins (`dg`, `lt`/`he`, `dc`, `gk`, `gd`, `ij`, `ma`, `na`, survol de `ui:gw`, `ui:ev`, `mz:pq`, `mz:kj`, `jo:kj`, `jc:hq`) et par les 26 e2e | Mutations de e1 (chaque mutation nomme un TU T1 attendu rouge) ; classement `gcov` (ii)/(iii) | la note (« couverture perdue ») et `git show --stat` des quatre commits |
| H-D11d-6 | D.11d ne touche ni `src/`, ni `tools/`, ni le `Makefile` | numéros `src/` de ce PRD valables au départ | `git diff fdb294b <gradé> -- src tools Makefile` vide |
| H-D11d-7 | Les tests « `*_is_a_struct` » des vues et remplaçants (`test_ha_pz_is_a_struct`, `test_ke_view_is_a_struct`, `test_cr_is_a_struct`, `test_fm_is_a_struct`, `test_placeholder_is_a_struct` de `je` et de `mo`, `test_hy_is_a_struct`…) sont gardés, amputés de leurs lignes T2 : ils nomment encore `pico8_X_struct_class()` des vues | Décision « vues typées, en-tête et descripteur gardés » (aucun TU ne suit) | `grep -n 'struct_class()' tests/*.c` sur les vues |
| H-D11d-8 | `test_pico8_fr` (jc tenu depuis un `fr` : `mt` puis `fa`, vers l. 388-398 à `fdb294b`) et `test_pico8_jc` (voie longue, `far`, vers l. 1479) restent T1 et lisent `jc->mt`/`has_mt` typés | Mutation M9 (`get_mt` de `fr`) | la note ; lecture des deux tests |
| H-D11d-9 | `test_pico8_ui` garde un moyen de démarrer une partie avec galaxie dans un état frais (fils ou `start_game`) et ses constantes `KE_ME_HASH`/`KE_T_HASH` inchangées | Oracle de e1, écrit dans `test_pico8_ui` dans son propre état | lecture du TU gradé |
| H-D11d-10 | Les fonctions publiques à valeur que seuls les lecteurs appelaient dans `src/` (`pico8_lq_bg`, `pico8_pj_ed`) ont encore un appelant de TU après D.11d, ou n'en ont plus | Règle « orphelins » (e2/e3) : gardées si un TU les appelle, supprimées sinon | `grep -rnw` sur `tests/` |

**Verdicts à `579a748`** (`git archive`, `data_residue.sh`, greps, notes « Découvert à D.11d1..d4 » recopiées en Annexe A) :

| Hypothèse | Verdict |
|---|---|
| H-D11d-1 | **fausse** : `par-nom` `tests/` = **11 lignes / 11 occurrences dans 7 fichiers**, pas 5 dans 3 ; `src/` 292 / 352 dans 31 fichiers (tenu) ; total **303**, **38 fichiers** ; `DATA_RESIDUE_GUARD` inchangé (tenu). `valeur` 1833 / 123 fichiers, `type-tag` 293 / 63, `ref-audit` 42 / 14 ; résidu D 2336 lignes, 124 fichiers |
| H-D11d-2 | **tenue** pour les contenus ; numéros : `test_pico8_iv.c:200`, `:205` ; `test_pico8_mo.c:95` ; `test_pico8_ui.c:717`, `:718` |
| H-D11d-3 | **fausse en six lignes** : hors T3, `test_pico8_ie.c:95`, `:103` (`shared_gy`/`shared_id` : `gy`/`id` sont l'état partagé de la classe `ie`, sans membre ni lecteur typé), `test_pico8_g_.c:120` et `test_pico8_mz.c:148` (`bv_et_fo` : `pico8_bv_et_t` privé à `probe.c`), `test_pico8_jc.c:141`, `:151` (`mouse_point_ra`/`_p` : `pico8_jc_mouse_point_t` privé à `slipway.c`) lisent encore par `pico8_entity_get_field`, chacune dans une aide sans paramètre de nom, derrière un test de classe. Aucune autre ligne (`cls->field`, `kind_field`, `pico8_ke_field` hors T3, `*_take_*`) |
| H-D11d-4 | **tenue** : 31 lignes de comparaison `rec` ↔ `pico8_X_start` (grep 5), prouvées par M4b ; aides de sortes parties |
| H-D11d-5 | **tenue, avec un compte différent** : **59** tests supprimés (d1 4, d2 11, d3 32, d4 12), pas 47 ; `gcov` de D.11d : chaque ligne perdue d'infrastructure par nom, d'un autre TU ou des e2e (assertions ajoutées pour `collect.c:74`, `wormhole.c:243`, `wormhole.c:47`, `scroll.c:21`) ; les valeurs `hi`/`gq`/`ca` de la vue `ha_pz` ne sont plus lues par aucun TU (lecteur de vue, T2) |
| H-D11d-6 | **tenue** (`git diff --stat fdb294b 579a748 -- src tools Makefile` vide) |
| H-D11d-7 | **tenue** : `pico8_X_struct_class()` des onze vues encore nommés par des TU (`js`, `pj`, `ie`, `fm`, `hy`, `je`, `mo`, `jc`, `mz`, `g_`, `hj`, `os`, `qh`) |
| H-D11d-8 | **tenue** : `test_pico8_fr.c:438-448` (`made->has_mt`, `made->mt`, jc depuis un `fr`), `:514`, `test_pico8_jc.c:281`, `:486` lisent `mt` typé |
| H-D11d-9 | **tenue** : `start_game()` (`test_pico8_ui.c:205`) |
| H-D11d-10 | **tenue, branche « gardées »** : `pico8_lq_bg` a encore des appelants de TU (`test_pico8_ga`, `test_pico8_hp`), `pico8_pj_ed` aussi (`test_pico8_spec_defaults`) : **gardées** ; `pico8_g__et`, `pico8_qh_et`, `pico8_hj_pk`, `pico8_hy_hl`, `pico8_js_h_` n'ont aucun appelant de TU (la note de D.11d les classe « infrastructure par nom » au `gcov`) : supprimées |

Conséquences : départ **303** (`tests/` 11 / 11, 7 fichiers) ; D.11e doit **exposer trois lecteurs typés** dans `src/` et retyper les six lignes d'exception avant de retirer les lecteurs qu'elles appellent (nouvelle section « Lecteurs typés pour les TU ») ; `tests/` après e1 = 6 + O, après e2 = 2, après e3 = 0 ; empreintes, budget, mutations et critères refaits.

### Sections revérifiées sur `579a748`

Refaites : description, « État de départ », « Lecture », « Inventaire » (nouvelle sous-section « Exceptions de D.11d »), « Classes locales de TU » (numéros), « Solution », « Modules », « Lecteurs typés pour les TU » (nouvelle), « Orphelins », « Empreinte et découpage », « Effet sur `make data-residue` », « Implementation Chunks », « TU touchés », « Mutations » (M18-M21), budget, contrôles, critères `FAILED:`, « Further Notes », Annexe A (ligne D.11d et ses notes, recopiée de `579a748`). Inchangées et vérifiées : inventaire `src/` (Annexe D, `src/` identique), « Qui répond », « Accesseurs », « Oracle », « Vues ». Si le HEAD de départ de D.11e n'est pas `579a748` (ou un descendant qui ne touche que `docs/`), l'implémenteur remesure et explique l'écart.

(Liste d'origine des sections à revérifier, gardée pour trace :) « État de départ » (comptes, 34 fichiers) ; « Classes locales de TU » et les numéros de ligne T3 (H-D11d-2) ; « Oracle » (H-D11d-9) ; « Empreinte et découpage » (chemins `tests/` de chaque sous-étape, H-D11d-3) ; « Effet sur `make data-residue` » ; « Orphelins » (H-D11d-10) ; « Mutations » (TU nommés, H-D11d-5, H-D11d-8) ; critères `FAILED:` de départ et de comptes ; « Vues » (H-D11d-7) ; Annexe A (ligne D.11d et sa note « Découvert à D.11d », à recopier). Les numéros de `src/` ne sont à refaire que si H-D11d-6 tombe.

### Lecture de ces textes pour D.11e

- **« Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`…) »** : remesuré à `fdb294b`, **14 sites comptés** exactement, plus 4 définitions d'aides comptées (18 lignes), 8 appels d'aides non comptés, et 3 lignes comptées de `jc.mt` : **21 lignes / 21 occurrences, 8 fichiers**. Les numéros du plan viennent d'un arbre plus ancien : `wormhole.c:78` est aujourd'hui **`wormhole.c:70`** (`pico8_mz_pq_ox_predicate`, `fx`), `entity_geom.c:99` est **`entity_geom.c:104`** (`pico8_ij`, appel d'aide pour `p`), `wormhole_display.c:41` est inchangé (appel d'aide pour `ra` ; la ligne comptée est la définition l. 21). Rapporté, sans effet sur le contenu.
- **« accesseur typé par champ dans la table de classe, NULL = absent, Q6 »**, **reconfirmée par l'humain le 2026-09-29** (« accesseurs typés par champ, pas de remontée dans `ke`, même si la phase 2 en remplace une partie ») : appliquée à chaque champ transversal lu sur un receveur **ouvert** (registres `hv`, `cv`, calques de `dc`, `ui.o_`/`gj`, `mz.ox`, extrémités de voie, listes de `ij`). Trois champs de l'annexe de D.11b n'en reçoivent pas, parce qu'une forme déjà décidée par le plan les couvre exactement (voir « Accesseurs ») : `dw` (composant sonde, Q7 : le crochet `probe_of` **est** l'accès typé de ce groupe, recommandation de D.11c), `fo` (écrivain typé de `bv`, recommandation de D.11c), `ma` (receveur fermé et déjà asserté par D.11a : forme A, cast vérifié, comme l'exception `pico8_na` que l'annexe de D.11b retenait déjà). **Aucune de ces formes n'est une remontée dans `ke`** ; ce sont des écarts à Q6 déclarés, que la note rapporte. `get_mt` s'ajoute (`jc.mt`) : 11 accesseurs au lieu des 13 + 1 de l'annexe de D.11b.
- **« l'oracle accesseurs = lecteurs naît et meurt dans l'étape »** : né en **e1** (dans `test_pico8_ui`), mort au **commit de e2**, qui supprime les premiers lecteurs qu'il compare. Ses lignes `par-nom` sont des hausses déclarées de e1.
- **« Découvert à D.11b … garder une comparaison rec ↔ `pico8_X_start` »** : portée par D.11d (H-D11d-4) ; D.11e supprime le `field` des sortes sans toucher cette comparaison, et un critère `FAILED:` l'exige intacte à chaque sous-étape.
- **« lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t` »** : 38 lecteurs pour 39 descripteurs (`os` nomme celui de `qh` ; `jb`, 40ᵉ descripteur, n'en a aucun) et 18 écrivains pour 21 descripteurs (`os`, `pm`, `fr` nomment celui de leur parente), 60 lignes d'emplacements ; plus les aides statiques non comptées qu'ils sont seuls à appeler (`jo_field_*`, `g__fixed_field`, `jc_fixed_field`, `mz_ref_field`…, prédicats de porteurs `jo_is_fo_holder_class`, `jc_is_fo_holder_class`, `family_is_target_class`, `mz_is_et_class`).
- **« les 10 `kind_field` et les emplacements des 11 sortes »** : **10 fonctions** (`jo` + 9) et **9 emplacements** : il y a 9 sortes, pas 11 (déjà constaté par la note de D.11b). Rapporté.
- **« les aides `*_take_*` restantes »** : 35 définitions (34 d'écrivains, plus `jc_take_fixed` de `jc.mt`), 46 appels.
- **« `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` »** : plus `pico8_ke_field` (`ke.c:44`, `ke.h:36`), lecteur de repli de 14 lecteurs, non nommé par la ligne mais compté et sans objet après eux.
- **« les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`) »** : les onze deviennent **typées** (plus aucun lecteur ni écrivain par nom ; leur struct et, pour chacune, la donnée qu'un consommateur typé lit déjà) ; **aucune ne disparaît**, parce que chacune a encore un consommateur (table « Vues ») ; leur en-tête et leur descripteur (vide, ou avec `get_ra`/`get_p` pour `jc_mouse_point`) restent, parce que leur identité de classe est lue par `src/` ou par un test gardé (H-D11d-7).
- **« Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks »** : remesuré à `579a748`, **46 chemins distincts de `src/` et `tests/`** (39 de `src/`, 7 TU), plus le `Makefile` (ligne `DATA_RESIDUE_GUARD` seulement) et `docs/plan-3.md` ; trois sous-étapes, voir « Empreinte et découpage ».
- **« Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD` »** : exact, au dernier commit (e3) ; c'est **la seule ligne du `Makefile` qui change** (précédent : D.10b pour `dispatch`). Les listes de sources (`DATA_ENGINE_SRCS`, `ENGINE_SRCS`) et les règles de lien restent telles quelles : la règle de lien de ce PRD se prouve avec elles.
- **« §6 : aucun »**, **« `class-coverage-net` : oui »** (à chaque sous-étape : e1 change toutes les tables de classe des constructeurs de l'arbre `jo`), **« `sanitize-net` : non »** (aucune durée de vie ni propriétaire ne change) ; le balayage ASan+UBSan des TU reste exigé (un accesseur qui caste vers le mauvais type est un comportement indéfini que seul ce balayage voit).
- **Règle de lien** (contrainte de `jc.mt`, note de D.11a) : **aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste**. Hors liste : `cart`, `cheat`, `coverage`, `galaxy`, `help`, `menu`, `planet_family_trade`, `research_icon`, `score_panel`, `score_panel_display`. D'où `get_mt` dans la table (posé par le descripteur de `fr`, lu par `jc:hq` sans nommer `planet_family_trade.c`).
- **« Découvert à D.11d2 », « D.11d3 », « D.11d4 » (pour D.11e)** : `pico8_ie_gy()`/`pico8_ie_id()`, la struct `pico8_bv_et_t` (ou un accesseur de son `fo`) et la struct `pico8_jc_mouse_point_t` (ou des accesseurs de `ra`/`p`) sont à exposer dans `src/` avant de retirer les lecteurs `pico8_ie_field`, `pico8_bv_et_field` et `pico8_jc_mouse_point_field` ; les six lignes de TU qui les lisent par nom sont retypées dans le même commit, pour que `tests/` soit à 0 avant que `par-nom` entre dans `DATA_RESIDUE_GUARD`. Ce sont des lecteurs typés (esprit de Q6 : accès typé, NULL = absent), aucun chemin par nom nouveau. Voir « Lecteurs typés pour les TU ».
- **Règle des ponts de TU (l. 267)** : un TU qui nomme un symbole retiré par une sous-étape le suit dans le même commit ; D.11e n'en laisse aucun (c'est la dernière étape `par-nom`).
- **Leçons des notes à ne pas refaire** : classification (a)/(b)/(c) absente ou fausse ; lignes qui gagnent une catégorie omises ; lignes comptées modifiées non déclarées ; constante de hachage « mise à jour » ; identifiant compté remplacé par un alias ou sa valeur (D.7c) ; lecture par nom déplacée dans une aide non comptée ; cas de mort ou appelant de TU non vus (D.11a) ; mutation non restaurée par `cp` ; cast sans test ; oubli d'un descripteur qui répond à un nom (il lirait « absent » en silence).
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json`, rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes, alias, registre, renommage pour échapper aux greps, cast non vérifié, cast de pointeur de fonction, cale valeur ↔ typé permanente ; mutations restaurées par `cp` simple ; binaires lancés exactement comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan) ; sous-étapes d'au plus ~40 chemins et d'environ 50 minutes ; jamais l'appli raylib.

### État de départ

**Mesuré sur `fdb294b`** (arbre archivé, D.11d en cours ; première version), sans build :

| Catégorie | Lignes | Fichiers | Occurrences | `src/` (lignes / occ., fichiers) | `tests/` (lignes / occ., fichiers) |
|---|---|---|---|---|---|
| `valeur` | 3815 | 123 | rapporté | rapporté | rapporté |
| `type-tag` | 343 | 63 | rapporté | rapporté | rapporté |
| `ref-audit` | 42 | 14 | rapporté | 42 | 0 |
| `par-nom` | 859 | 69 | 974 | **292 / 352**, 31 | 567 / 622, 38 |
| **résidu D** | **4605** | **124** | | | |

**Départ de D.11e, mesuré à `579a748`** (D.11d gradée) : `par-nom` **303** lignes, 38 fichiers : `src/` 292 / 352 (31 fichiers, inchangé), `tests/` **11 / 11** (7 fichiers : 5 T3, 6 exceptions) ; `valeur` 1833 / 123 fichiers, `type-tag` 293 / 63, `ref-audit` 42 / 14 ; résidu D 2336 lignes, 124 fichiers. `DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch` (`Makefile` l. 889).

Recette (réutilisée à chaque mesure ; `$S` : dossier du scratchpad) :

```sh
sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
for c in valeur type-tag ref-audit par-nom; do
  printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
for f in $F; do n=$(grep -cE "$(pattern par-nom)" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$(pattern par-nom)" $f | wc -l)"; done
```

Les 60 lignes d'emplacements de descripteurs portent **deux** occurrences chacune (`.field = pico8_X_field,`) ; toutes les autres lignes comptées de `src/` en portent une (232 + 2 × 60 = 352).

### Inventaire (mesuré sur `fdb294b`)

Méthode : chaque ligne comptée de `src/` a été rattachée à sa fonction englobante et classée (Annexe D) ; pour chacun des 38 lecteurs et 18 écrivains, les noms comparés par `strcmp` et la délégation (`return pico8_<parente>_struct_class()->field(…)`, `return pico8_ke_field(…)`, `return pico8_hj_field(…)`) ont été extraits, d'où la table « Qui répond » ; les receveurs de chaque site ont été relus (appelants, registres).

**Répartition des 292 lignes (352 occurrences)** :

| Nature | Lignes | Sous-étape |
|---|---|---|
| sites C et `jc.mt` (14 sites, 4 définitions d'aides, 3 lignes de `mt`) | 21 | e1 |
| emplacements `.field`/`.set_field` des descripteurs (2 occurrences chacune) | 60 | e2 (36), e3 (24) |
| définitions de lecteurs `field` (38 de descripteurs, `pico8_entity_get_field`, `pico8_ke_field`) | 40 | e2 / e3 |
| délégations de lecteurs (`return …->field(…)`, `return pico8_ke_field(…)`) | 23 | e2 / e3 |
| définitions d'écrivains `set_field` | 18 | e2 / e3 |
| délégations d'écrivains | 7 | e2 / e3 |
| définitions d'aides `*_take_*` (hors `jc_take_fixed`) | 34 | e2 / e3 |
| appels d'aides `*_take_*` | 46 | e2 / e3 |
| `kind_field` : définitions (10) et déclaration (`planet.h:81`) | 11 | e2 |
| `kind_field` : délégations | 9 | e2 |
| entrées `kind_field` des 9 initialiseurs de sortes (ligne gardée, entrée retirée) | 9 | e2 |
| corps de `pico8_entity_get_field`/`set_field` (`entity.c:204`, `:205`, `:208`, `:215`, `:216`) | 5 | e3 |
| en-têtes : `entity.h:49`, `:52`, `:54` ; `ke.h:36` ; `research_icon.h:19` (commentaire) ; `struct.h:43`, `:47`, `:104`, `:105` | 9 | e3 |
| **Total** | **292** | e1 21, e2 184, e3 87 |

#### Classe C et `jc.mt` (e1)

14 sites comptés, 4 définitions d'aides comptées, 8 appels d'aides non comptés, 3 lignes comptées de `jc.mt` (et 2 appels non comptés) : 21 lignes, 21 occurrences, 8 fichiers.

| Fichier:ligne (`src/pico8_`) | Fonction | Membre | Receveur (d'où il vient) | Remplacement | Classe au commit |
|---|---|---|---|---|---|
| `bootstrap.c:87` | `pico8_dg` | R `p_` (vérité) | élément de `hv` : tout `ke` inscrit | `qc->cls->get_p_ != NULL && qc->cls->get_p_(qc)` ; `pico8_dg_truthy` (l. 19, statique, non comptée) perd son seul appelant : supprimée | (a1) |
| `entity.c:237` | `pico8_lt` | R `nu` | tout `ke` inscrit | `tags = e->cls->get_nu != NULL ? e->cls->get_nu(e) : NULL` ; NULL n'inscrit rien, comme `all(nil)` | (a1) |
| `entity.c:266` | `pico8_he` | R `nu` | idem | idem | (a1) |
| `entity.c:300` | `pico8_dc` | R `fa` | élément de `cv("kj")`/`cv("ev")` | `qc->cls->get_fa == NULL \|\| !qc->cls->get_fa(qc, &fa)` → `continue` | (a1) |
| `entity.c:329` | `pico8_dc` | R `ra` | idem | `ra = qc->cls->get_ra != NULL ? qc->cls->get_ra(qc) : NULL` | (a1) |
| `entity_geom.c:36`, `:37` (appels) | `pico8_gk` | R `fa`, `ra` | `e` : `fv` (`help.c`), `jo` (`planet.c`), `ie` (`research_icon.c`), `js` (`ui_button.c`), ou NULL | `get_fa` (absent → 0), `get_ra` (NULL → zéro) ; `e == NULL` garde les replis | appels non comptés |
| `entity_geom.c:42` | `pico8_gk` | R `kq` | idem | `e != NULL && e->cls->get_kq != NULL ? e->cls->get_kq(e) : NULL`, puis `pico8_qz_call` si non NULL | (a1) |
| `entity_geom.c:49` (appel) | `pico8_gd` | R `ra` | planète, hôte d'émetteur (`particle.c`), extrémité de voie (`slipway_display.c` : planète ou point de souris), classe locale de `test_pico8_mo` | `get_ra` (NULL → zéro) | appel non compté |
| `entity_geom.c:64` | `pico8_na` | R `ra` (écrit **au travers**) | `self` : `qh:ly`, `nl:ly` (famille `jo` seule) | **cast vérifié** (exception, voir « Accesseurs ») : `assert(self == NULL \|\| pico8_jo_is_family(self))`, puis `((pico8_jo_t *)self)->ra` | (a1) |
| `entity_geom.c:73` (appel), `:82` | `pico8_ma` | R/W `ma` | `e` : `jo` (`planet_display.c`) ou `qb` (`scroll.c`), **déjà asserté** par D.11a (`entity_geom.c:79-80`) | **cast vérifié** sur les deux branches de l'assert : `pico8_jo_is_family(e)` → `&((pico8_jo_t *)e)->ma`, sinon `e->cls == pico8_qb_struct_class()` → `&((pico8_qb_t *)e)->ma` ; lecture puis écriture de `*ma` ; `e == NULL` lit 0 et n'écrit rien, comme aujourd'hui | (a1) ; l'assert remonte avant la lecture (piège, voir « Accesseurs ») |
| `entity_geom.c:104` (appel) | `pico8_ij` | R `p` | élément de `cg("jo")`, `cg("np")`, `mz.ox` (planètes et `g_`/`fv`) | `get_p` (NULL → zéro ; élément NULL → zéro) | appel non compté |
| `entity_geom.c:19`, `:27` | `pico8_entity_geom_field_vec`/`_fixed` | définitions | — | supprimées | (a2), (a2) |
| `planet_display.c:484` | `pico8_jo_kj` | R `dw` | `self->fo` : un porteur de sonde (`g_`, `fv`, lignée `qh`, `jc` ; `mz` jamais sur une planète) | `self->fo->cls->probe_of != NULL ? self->fo->cls->probe_of(self->fo)->dw : 0` (crochet de D.11c) | (a1) |
| `slipway.c:258` | `pico8_jc_hq` | W `fo` | `self->probe->et` = `jc:ce(self)` : planète (famille `jo`), `jc`, ou la vue `jc_too_far` | `pico8_probe_et_hang_fo(self->probe->et, &self->header)` : l'aide typée de `bv` rendue publique, avec une branche `jc_too_far` ajoutée | (a1) |
| `slipway.c:249`, `:250` (appels), `:251` | `pico8_jc_hq` | R `mt` sur `qu`, puis `re` ; W `jc.mt` | `qu` : planète ; `re` : planète tenue (`gv` : arbre `jo`) ou point de souris | `get_mt` sur `qu`, sinon sur `re` : `if (qu != NULL && qu->cls->get_mt != NULL && qu->cls->get_mt(qu, &mt))` … ; `self->mt`/`self->has_mt` posés directement ; un fixe est toujours vrai en Lua, d'où « présent ⇒ pris » | (a1) (`:251`) |
| `slipway.c:127`, `:164` | `pico8_jc_entity_value`, `jc_take_fixed` | définitions | — | supprimées (`pico8_jc_truthy` garde un autre appelant, `:273`) | (a2), (a2) |
| `ui.c:304`, `:367` (appels) | `pico8_ui_ev`, `pico8_ui_gw` | R `ql`, `fg` | `self->o_` : entité survolée (`cv("hd")`) | `get_ql` (absent → `self->ql`), `get_fg` (absent → 0) | appels non comptés |
| `ui.c:346` | `pico8_ui_gw` | R `fa` | élément survolé de `cv("hd")` | `get_fa` (absent → pas de calque) | (a1) |
| `ui.c:359`, `:363` | `pico8_ui_gw` | W `m_` | `self->gj`, `self->o_` | `assert(e->cls->set_m_ != NULL && "…")` puis `e->cls->set_m_(e, 0)` / `(e, 1)` : l'`assert` d'écriture générique d'aujourd'hui (« a generic write names a field this struct has no member for -- stop, do not add a member ») est gardé mot pour mot à ces deux sites | (a1), (a1) |
| `ui.c:56` | `pico8_ui_field_raw` | définition | — | supprimée | (a2) |
| `wormhole.c:70` | `pico8_mz_pq_ox_predicate` | R `fx` (présence puis vérité) | élément de `hv` | `p->cls->get_fx != NULL && p->cls->get_fx(p)` (`pico8_mz_truthy` garde un autre appelant, `:167`) | (a1) |
| `wormhole_display.c:41` (appel) | `pico8_mzd_ra_of_ij` | R `ra` | `mz.ij` ∈ `mz.ox` | `get_ra` (NULL → zéro) ; `pico8_mzd_value_vec` perd son seul appelant : supprimée | appel non compté |
| `wormhole_display.c:21` | `pico8_mzd_field_vec` | définition | — | supprimée | (a2) |

Classement attendu au commit de e1 : **(a1) 15** (`bootstrap.c:87`, `entity.c:237`, `:266`, `:300`, `:329`, `entity_geom.c:42`, `:64`, `:82`, `planet_display.c:484`, `slipway.c:251`, `:258`, `ui.c:346`, `:359`, `:363`, `wormhole.c:70`), **(a2) 6** (`entity_geom.c:19`, `:27`, `slipway.c:127`, `:164`, `ui.c:56`, `wormhole_display.c:21`), (b) 0, (c) 0.

#### Qui répond à chaque nom (mesuré à `fdb294b`)

Lecteurs et écrivains relus, délégation à la parente comprise. C'est l'ensemble **exact** des descripteurs qui reçoivent chaque accesseur (règle « NULL = absent »). Les « 10 de `jo` » : `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`.

| Accesseur | Descripteurs (fichier) | Nombre |
|---|---|---|
| `get_nu` | 10 de `jo` ; `g_` (`probe.c`) ; `fv` (`help.c`, par délégation) ; `mh` (`msgbox.c`) ; `jc` (`slipway.c`) ; `mz` (`wormhole.c`) | 15 |
| `get_fa` | 10 de `jo` ; `g_`, `fv` ; `bg` ; `mh`, `iv` ; `ie` ; `pj` ; `jc` ; `ui` ; `js`, `fq` ; `mz` | 22 |
| `get_ra` | 10 de `jo` ; `g_`, `fv` ; `lq` ; `nf` ; `iv` ; `ie` ; `qb` ; `jc_mouse_point` ; `js` ; `mz` | 21 |
| `get_p` | 10 de `jo` ; `g_`, `fv` ; `qb` ; `jc_mouse_point` ; `mz` | 15 |
| `get_kq` | 10 de `jo` ; `fv` ; `ie` ; `js` | 13 |
| `get_ql` | `fv` ; `ui` | 2 |
| `get_fg` | `fv` ; `js` | 2 |
| `get_fx` | 10 de `jo` ; `g_`, `fv` | 12 |
| `get_p_` | 10 de `jo` ; `g_`, `fv` ; `mh`, `iv` ; `jc` ; `fq` ; `mz` | 17 |
| `set_m_` | 10 de `jo` ; `fv` ; `bg` ; `ie` ; `js`, `fq` | 15 |
| `get_mt` | `fr` (`planet_family_trade.c`) ; `jc` | 2 |
| **Total** | 136 lignes d'initialiseurs dans 16 fichiers | |

Écarts avec l'annexe de D.11b (Annexe C) : mêmes ensembles pour les dix noms communs ; `get_ma` (11), `get_dw` (12) et `set_fo` (13) ne sont pas créés (voir « Accesseurs ») ; `get_mt` (2) s'ajoute. Les autres descripteurs (`hy`, `hp`, `trade`, `jb`, `pj_cr`, `ha_pz`, `mz_jj`, `nr_ldown_empty`, `ie_ke_view`, `jc_too_far`, `bv_et` et les trois remplaçants de `mo`/`je`/`fm`) ne répondent à aucun de ces noms : membres NULL. `jc_too_far` et `bv_et` répondent à `fo`, qui passe par l'écrivain typé de `bv`.

**Pièges de genre relevés** (à reproduire, pas à « corriger ») : `g_` sert `fx` comme un **fixe** (`g__fixed_field(g_->fx)`) : sa vérité Lua est toujours vraie, même à 0 — `get_fx` de `g_` rend 1 sans lire la valeur ; `jo` sert `fx` comme un booléen (`has_fx` puis `fx`). `mz` sert `fp` comme un fixe (hors C). `jc` sert `fa` toujours présent, `mt` selon `has_mt`. `ui` sert `ql`, `fv` sert `ql` et `fg`, `js` sert `fg` : relire chaque branche pour sa condition de présence.

#### Infrastructure (e2, e3)

Par fichier, lignes comptées (occurrences) retirées par e2 et e3, hors e1 (détail ligne à ligne : Annexe D) :

| Sous-étape | Fichier (`src/pico8_`) | Lignes / occ. | Contenu |
|---|---|---|---|
| e2 | `planet.c` | 33 / 35 | lecteur et écrivain de `jo`, 10 aides `jo_take_*` et leurs 17 appels, 2 emplacements, `pico8_jo_kind_field` |
| e2 | `planet.h` | 1 / 1 | déclaration de `pico8_jo_kind_field` |
| e2 | `planet_family.c` | 58 / 70 | lecteurs et écrivains de `qh`, `nl`, `hj`, `pm`, `ga` (et ceux que `os` nomme), aides `family_take_*`, 12 emplacements, 6 `kind_field`, 6 délégations, 6 entrées de sortes |
| e2 | `planet_family_trade.c` | 31 / 38 | lecteurs et écrivains de `nr`, `fr`, `mx`, vue `nr_ldown_empty`, aides `family_trade_take_*`, 7 emplacements, 3 `kind_field` et leurs entrées |
| e2 | `probe.c` | 22 / 27 | lecteur et écrivain de `g_`, vues `fm` et `bv_et`, aides `g__take_*`, `bv_et_take_holder`, 5 emplacements |
| e2 | `help.c` | 8 / 10 | lecteur et écrivain de `fv`, `help_take_bool`, 2 emplacements |
| e2 | `slipway.c` | 17 / 22 | lecteur et écrivain de `jc`, vues `jc_too_far`, `jc_mouse_point`, aides `jc_*_take_*`, 5 emplacements |
| e2 | `wormhole.c` | 14 / 17 | lecteur et écrivain de `mz`, vue `mz_jj`, aides `mz_take_*`, 3 emplacements |
| **e2** | **8 fichiers** | **184 / 220** | |
| e3 | `bg.c` | 7 / 9 | lecteur, écrivain, `bg_take_bool`, 2 emplacements |
| e3 | `economy.c` | 2 / 3 | lecteur de `hy`, emplacement |
| e3 | `entity.c` | 8 / 9 | remplaçant de `mo`, `pico8_entity_get_field`/`set_field` |
| e3 | `entity.h` | 3 / 3 | leurs déclarations et commentaire |
| e3 | `ke.c`, `ke.h` | 2 / 2 | `pico8_ke_field` |
| e3 | `layout.c` | 2 / 3 | lecteur de `lq`, emplacement |
| e3 | `menu.c` | 2 / 3 | vue `ha_pz` |
| e3 | `mouse_reader.c` | 3 / 4 | lecteur de `nf` |
| e3 | `msgbox.c` | 6 / 8 | lecteurs de `mh`, `iv` |
| e3 | `research.c` | 2 / 3 | lecteur de `hp` |
| e3 | `research_icon.c`, `.h` | 10 / 13 | lecteur et écrivain de `ie`, vue `ie_ke_view`, commentaire `.h:19` |
| e3 | `score_panel.c` | 5 / 7 | lecteur de `pj`, vue `pj_cr` |
| e3 | `scroll.c` | 9 / 11 | lecteur et écrivain de `qb`, aides `qb_take_*` |
| e3 | `struct.h` | 4 / 4 | typedefs et emplacements |
| e3 | `trade.c` | 2 / 3 | lecteur de `trade` |
| e3 | `ui.c` | 5 / 7 | remplaçant de `je`, lecteur de `ui` |
| e3 | `ui_button.c` | 15 / 19 | lecteurs et écrivains de `js`, `fq`, aides `button_take_*` |
| **e3** | **19 fichiers** | **87 / 111** | |

**Non compté, à supprimer avec eux** (`-Werror` rend fatale toute fonction `static` inutilisée ; environ 2 400 lignes de corps de lecteurs, écrivains, aides et `kind_field` au total) : aides statiques de lecture (`jo_field_string`/`_list`/`_vec`/`_data`, `g__fixed_field`, `g__ref_field`, `jc_fixed_field`, `jc_ref_field`, `mz_ref_field`, `mz_fixed_field`…), prédicats de porteurs que seuls des écrivains appellent (`jo_is_fo_holder_class`, `jc_is_fo_holder_class`, `family_is_target_class`, `mz_is_et_class` ; `bv_et_is_holder_class` garde un appelant dans `probe_et_hang_fo`), gardes « prêt » de `kind_field` si elles perdent leur dernier usage.

**Orphelins publics** (fonctions déclarées dans un `.h` dont le seul appelant de `src/` est un lecteur) : `pico8_g__et` (`probe.c:577`), `pico8_qh_et` (`planet_family.c:111`), `pico8_hj_pk` (`planet_family.c:459`), `pico8_hy_hl` (`economy.c:77`), `pico8_js_h_` (`ui_button.c:124`) : aucun appelant de TU à `fdb294b` → **supprimés** avec leur déclaration (orphelins créés par l'étape). `pico8_lq_bg` (`layout.c:333` ; appelée par `test_pico8_ga` et `test_pico8_hp` à `579a748`) et `pico8_pj_ed` (`score_panel.c:359` ; appelée par `test_pico8_spec_defaults`) : **gardées** (H-D11d-10 tenue dans cette branche), listées au message. `pico8_jo_hf`, `pico8_jo_ready` gardent d'autres appelants. Liste revérifiée par `grep -rnw` au commit.

#### Vues (structs-vues)

| Vue (fichier) | Consommateur typé restant (preuve) | Identité de classe lue par | Devient |
|---|---|---|---|
| remplaçant de `mo` (`entity.c`) | `pico8_mo()`, `pico8_mo_qb()` (NULL dessus) | `test_pico8_mo` (`test_placeholder_is_a_struct`) | typée : descripteur vide (e3) |
| remplaçant de `je` (`ui.c`) | `pico8_je()`, `pico8_je_nf()` | `test_pico8_je` | typée : descripteur vide (e3) |
| remplaçant de `fm` (`probe.c`) | `pico8_fm_placeholder()` lu par `jk`/`nz` | `cheat.c:28` | typée : descripteur vide (e2) |
| `jc_too_far` (`slipway.c`) | `probe_et_qs` (`qs`), `pico8_probe_et_hang_fo` (`fo`, e1) | `probe.c:292`, `test_pico8_jc` | typée : descripteur vide (e2) |
| `jc_mouse_point` (`slipway.c`) | `pico8_jc_end_p`/`_ra`, `pico8_gd` (`get_ra`) | `slipway.c` (`jc_end_point`), `test_pico8_jc` | typée : descripteur `{get_ra, get_p}` (e1 pose, e2 retire `field`) |
| `bv_et` (`probe.c`) | `probe_et_hang_fo` (`fo`), cible `et` de `bv` | `probe.c:183`, `planet_family.c:137`, `wormhole.c:349`, TU | typée : descripteur vide (e2) |
| `nr_ldown_empty` (`planet_family_trade.c`) | argument `s` de `pico8_nr_gv` (lit `s->cls`) | `pico8_nr_gv` | typée : descripteur vide (e2) ; elle modélise le `{}` du cart : garder l'objet est le portage littéral (rejeté : passer NULL, équivalent aujourd'hui mais s'éloigne du cart) |
| `pj_cr` (`score_panel.c`) | `menu.c:245`, init de `pj` | `test_pico8_pj` (`test_cr_is_a_struct`) | typée : descripteur vide (e3) |
| `mz_jj` (`wormhole.c`) | `probe_et_qs`, cible `et` de `mz` | `probe.c:289`, `wormhole.c:349`, TU | typée : descripteur vide (e2) |
| `ha_pz` (`menu.c`) | cadre de `pz` rempli depuis `g_ha_pz` (`menu.c:411-414`), drapeau `pico8_ha_pz()` (`cart.c:172`) | `test_pico8_js` (`test_ha_pz_is_a_struct`) | typée : descripteur vide (e3) |
| `ie_ke_view` (`research_icon.c`) | `pico8_ie_ke_view()` lu par `ie:hb` et le dessin (`rr`, `s`, `me`, `od`) | `test_pico8_ie` (`test_ke_view_is_a_struct`) | typée : descripteur vide (e3) |

« Descripteur vide » : tous ses membres NULL, initialisé sans nom de membre fictif ; l'objet et sa classe gardent leur identité. **Aucune ne disparaît** : chacune a encore un consommateur (colonne 2). Rejeté : retirer l'en-tête et la classe des vues dont seule une TU lit l'identité (`pj_cr`, `ha_pz`, `ie_ke_view`) — changement hors du compteur, qui ferait suivre trois tests gardés sans rien retirer au résidu (règle « chirurgical »).

#### Exceptions de D.11d : six lectures par nom de TU sans lecteur typé dans `src/` (mesuré à `579a748`)

| Ligne de TU | Aide (sans paramètre de nom) | Ce qu'elle lit | Lecteur de `src/` appelé | Constantes qui en dépendent | Remplacement (sous-étape) |
|---|---|---|---|---|---|
| `test_pico8_g_.c:120` | `bv_et_fo` (classe assertée `pico8_bv_et_struct_class()`) | `fo` de la cible jetable `g_bv_et` | `pico8_bv_et_field` (`probe.c:59`) | celles du TU, inchangées | `((const pico8_bv_et_t *)t)->fo` après le même test de classe ; `pico8_bv_et_t` déplacé de `probe.c:54-57` dans `probe.h` (e2) |
| `test_pico8_mz.c:148` | `bv_et_fo` | idem | idem | idem | idem (e2) |
| `test_pico8_jc.c:141` | `mouse_point_ra` (classe assertée, `is_throwaway`) | pointeur `ra` du point souris (`NULL` si absent) | `pico8_jc_mouse_point_field` (`slipway.c:88`) | `MOUSE_POINT_HASH`, `TOO_FAR_HASH`, `HQ_HASH`, identités de `test_hq_mouse_point` | `((const pico8_jc_mouse_point_t *)re)->ra` ; `pico8_jc_mouse_point_t` déplacé de `slipway.c:82-86` dans `slipway.h`, à côté de `pico8_jc_too_far_t` (e2) |
| `test_pico8_jc.c:151` | `mouse_point_p` | pointeur `p` | idem | idem | `->p` (e2) |
| `test_pico8_ie.c:95` | `shared_gy` | `g_ie_gy` (`research_icon.c:23`), état partagé de la classe | `pico8_ie_field` (`research_icon.c:245-246`) | constantes du TU pliant `gy` | `pico8_ie_gy()` (e3) |
| `test_pico8_ie.c:103` | `shared_id` (et `shared_id_fixed`) | `g_ie_id` (`research_icon.c:24`, `pico8_value_t` : nil ou fixe) | `pico8_ie_field` (`:247-248`, nil → absent) | constantes du TU pliant `id` | `pico8_ie_id(&out)` (e3) |

#### Classes locales de TU (T3 de D.11d, H-D11d-2)

| TU | Classe locale | Site C qu'elle nourrit | e1 |
|---|---|---|---|
| `test_pico8_iv` (`:200`, `:205`) | `g_fake_toast_class` (`test_dg_drives_a_ke_struct`) | `bootstrap.c:87` (`p_` de `dg`) | `.field = fake_toast_field` et la fonction (dont la ligne `return pico8_ke_field(…)`) → `.get_p_ = fake_toast_get_p_`, qui rend la vérité Lua de son `p_` ; le test inchangé (le toast sort de `hv` au troisième `dg`) |
| `test_pico8_mo` (`:95`) | `g_ra_only_class` (`test_gd_on_placeholder`) | `entity_geom.c:49` (`ra` de `gd`) | `{.field = ra_only_field}` → `{.get_ra = ra_only_get_ra}` ; `GD_NEAR`/`GD_FAR` inchangées |
| `test_pico8_ui` (`:717`, `:718`) | `g_giver_class` (`giver_field`, `giver_set_field`) | `ui.c:346` (`fa`), `ui.c:359`/`:363` (`m_`) | `.get_fa` (15, toujours présent) et `.set_m_` (sans effet, comme son écrivain) ; constantes inchangées |

`test_pico8_planet_kj_dispatch` copie la table de `jo` (`g_spied_jo = *pico8_jo_struct_class()`) : la copie emporte les accesseurs ; rien à changer.

## Solution

**Trois sous-étapes** (D.11e1, D.11e2, D.11e3), un commit chacune, chacune vérifiée et poussée, même PRD (modèle de la décision humaine qui a découpé D.11d) :

1. **D.11e1 — classe C** (27 chemins) : onze accesseurs typés dans `pico8_struct_class_t` (Q6), posés sur les 136 lignes de la table « Qui répond » ; les 14 sites C et `jc.mt` les lisent ; `dw` passe par `probe_of`, `fo` par l'écrivain typé de `bv` rendu public (branche `jc_too_far` ajoutée), `ma` et `na` par cast vérifié ; les quatre aides par nom de C meurent ; les trois faux descripteurs de TU (T3) passent aux accesseurs ; l'**oracle** accesseurs = lecteurs naît dans `test_pico8_ui`. `par-nom` `src/` 292 → 271.
2. **D.11e2 — arbre `jo`, sonde, voies** (18 chemins) : l'oracle meurt ; `pico8_bv_et_t` et `pico8_jc_mouse_point_t` deviennent publics et les quatre lignes d'exception de `test_pico8_g_`, `test_pico8_mz`, `test_pico8_jc` les lisent typées ; lecteurs, écrivains, aides `*_take_*`, vues et emplacements de `planet`, `planet_family`, `planet_family_trade`, `probe`, `help`, `slipway`, `wormhole` ; les 10 `kind_field` et l'emplacement `field` de `pico8_entity_kind_t`. `par-nom` `src/` 271 → 87, `tests/` → **2** (`test_pico8_ie`).
3. **D.11e3 — interface et fin de la couche** (26 chemins) : `pico8_ie_gy()`/`pico8_ie_id()` et les deux lignes d'exception de `test_pico8_ie` ; lecteurs, écrivains et vues du reste ; `pico8_entity_get_field`/`set_field`, `pico8_ke_field`, typedefs et emplacements `field`/`set_field` de `pico8_struct_class_t` ; `par-nom` ajouté à `DATA_RESIDUE_GUARD` ; note « Découvert à D.11e ». `par-nom` = 0.

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert, dont `make data-guard` : `résidu gardé (spec,overrides,stash,ob,dispatch,par-nom) : 0 ligne(s)` ; `make e2e` : **26 conformes** ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ;
- balayage ASan+UBSan des TU propre ;
- `make data-residue` : `par-nom` absent de la sortie (0) ; `valeur`, `type-tag`, `ref-audit` rapportés (baisse : les `pico8_value_*` des lecteurs, écrivains et aides partent) ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/` ni de `tools/` ; du `Makefile`, seule la ligne `DATA_RESIDUE_GUARD` change ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  . "$S/pat.sh"
  # 1. par-nom : vide dans src/ et tests/
  grep -nE "$(pattern par-nom)" src/*.[ch] tests/*.[ch]
  # 2. plus aucun reste de la couche : vide
  grep -rnE '\bfield\)\(|\(\*set_field\)|struct_field_fn|->(set_)?field\b|\.(set_)?field =|kind_field|pico8_ke_field|_take_[a-z]' src tests
  # 3. plus aucun lecteur qui passe d'un nom à un membre dans src/ : vide
  grep -nE 'strcmp\(name, "' src/*.c
  # 4. aucune fonction-accesseur appelée directement : vide (hors définitions, chacune n'est
  #    atteinte que par `x->cls->get_y(x…)` / `x->cls->set_m_(x…)`, même `x`)
  grep -nE '[a-z0-9_]+_(get_(nu|fa|ra|p|kq|ql|fg|mt|fx|p_)|set_m_)\(' src/*.c tests/*.c \
    | grep -vE '^[^:]+:[0-9]+:(static )?(int|void|const|pico8_)'
  # 5. la comparaison rec <-> pico8_X_start survit (note de D.11b, H-D11d-4)
  grep -nE 'rec->(rr|s|me|od|has_s|has_od)\b.*pico8_[a-z]+_start\.' tests/test_pico8_entity_kind.c | wc -l   # >= 25
  # 6. règle de lien : aucun fichier de DATA_ENGINE_SRCS ne nomme un symbole défini hors de la liste
  make tmp/test_pico8_bg_kj tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_planet_gh_row_lq tmp/test_pico8_ga_export_panel_by
  # 7. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^par-nom'   # vide
  ```

## User Stories

1. En tant que mainteneur du port, je veux que chaque champ transversal lu sur une entité quelconque (`p_`, `nu`, `fa`, `ra`, `p`, `kq`, `ql`, `fg`, `fx`, `mt`) passe par un accesseur typé de la table de classe, afin que plus aucun site de `src/` ne lise par nom (Q6, reconfirmée).
2. En tant que mainteneur du port, je veux que l'absence d'un champ se dise par un membre NULL de la table, posé exactement sur les descripteurs dont le lecteur répond au nom aujourd'hui, afin qu'aucune classe ne lise « absent » par oubli.
3. En tant que mainteneur du port, je veux que `m_` soit écrit par `set_m_`, et que son absence arrête le run avec le même message qu'aujourd'hui, afin que la règle « une struct ne gagne jamais de champ » reste gardée.
4. En tant que mainteneur du port, je veux que `dw` sur `jo.fo` se lise par le crochet `probe_of` de D.11c, afin de ne pas créer un accesseur pour un champ que le composant sonde porte déjà.
5. En tant que mainteneur du port, je veux que `jc:hq` accroche `fo` par l'écrivain typé que `bv` utilise déjà, étendu à `jc_too_far`, afin qu'il n'existe qu'un écrivain de `fo` par famille de cibles, avec ses `assert` de porteur.
6. En tant que mainteneur du port, je veux que `jc.mt` se lise par `get_mt`, afin que `slipway.c` ne nomme aucun symbole de `planet_family_trade.c` (règle de lien de `DATA_ENGINE_SRCS`).
7. En tant que mainteneur du port, je veux que `pico8_na` et `pico8_ma`, dont les receveurs sont une famille fermée déjà assertée, lisent par cast vérifié, afin de ne pas ôter le `const` d'une donnée générée ni créer un accesseur pour un seul site.
8. En tant que gardien des tests, je veux un oracle qui compare, sur des instances réelles de chaque classe, chaque accesseur au lecteur par nom qu'il remplace (présence et valeur), afin que la conversion soit prouvée au moment où les deux coexistent.
9. En tant que gardien des tests, je veux que cet oracle disparaisse dès que les lecteurs disparaissent, afin qu'il ne devienne pas un test de la couche qui survit à la couche.
10. En tant que gardien des tests, je veux que les trois faux descripteurs de TU passent aux accesseurs sans changer leur test ni leurs constantes, afin que `dg`, `gd` et `ui:gw` restent testés sur une classe locale.
11. En tant que mainteneur du port, je veux que les 38 lecteurs, 18 écrivains, 10 `kind_field`, 35 aides `*_take_*`, `pico8_entity_get_field`/`set_field`, `pico8_ke_field`, les typedefs et les emplacements disparaissent, afin que la couche par nom n'existe plus.
12. En tant que mainteneur du port, je veux que les onze structs-vues n'aient plus de lecteur ni d'écrivain par nom et gardent leur identité de classe, afin que leurs consommateurs typés et leurs tests d'identité restent valides.
13. En tant que mainteneur du port, je veux que les fonctions statiques et publiques que seuls les lecteurs appelaient disparaissent avec eux (sauf si un TU les appelle), afin de ne laisser aucun orphelin créé par l'étape.
14. En tant que gardien du compteur, je veux `par-nom` = 0 et ajouté à `DATA_RESIDUE_GUARD`, afin que `make test` refuse toute lecture par nom réintroduite.
15. En tant que gardien du compteur, je veux au message de chaque commit, par catégorie et par fichier, les lignes et occurrences avant/après, la classification (a1)/(a2)/(a3)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris), les lignes comptées modifiées non retirées, et chaque Δlignes > Δoccurrences justifié, avec les numéros du parent du commit, afin qu'aucune baisse ne soit un tassement.
16. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, qu'aucun alias, registre, renommage pour les greps, cast non vérifié ou de pointeur de fonction, ni cale valeur ↔ typé ne soit ajouté, afin que la baisse veuille dire « retiré ».
17. En tant que gardien des filets, je veux `make test`, `make e2e`, `make class-coverage-net` et le balayage ASan+UBSan des TU verts à chaque sous-étape, afin de prouver qu'aucun accesseur ne lit hors de sa struct et qu'aucune classe ne change d'instanciations.
18. En tant que gardien des tests, je veux `gcov` avant/après chaque sous-étape et des mutations ciblées, restaurées par `cp`, afin que chaque ligne ajoutée soit exécutée et que chaque ligne perdue soit de l'infrastructure.
19. En tant que gardien de la note de D.11b, je veux que la comparaison `rec` ↔ `pico8_X_start` survive à la suppression du `field` des sortes, afin que la duplication des littéraux `s`/`od` reste gardée.
20. En tant que `plan-pilot`, je veux trois sous-étapes d'environ 50 minutes, chacune verte seule, et une note « Découvert à D.11e » qui dise les comptes réels, les écarts à Q6, les vues, les orphelins et ce que D.12 et la phase 2 trouvent, afin que la suite parte d'un état exact.
21. En tant que gardien des tests, je veux que les six lectures par nom que D.11d a dû laisser (état partagé `gy`/`id` de `ie`, `fo` de `bv_et`, `ra`/`p` du point souris) passent par des lecteurs typés exposés dans `src/`, constantes inchangées, afin que `tests/` soit à 0 avant que `par-nom` entre dans la garde.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q6 (reconfirmée le 2026-09-29), Q7 et Q11 sont des décisions humaines que ce PRD applique ; Q12 reste ouverte et n'est pas tranchée ici.

### Modules

- **Table de classe** (`pico8_struct.h`) : e1 ajoute onze membres accesseurs ; e2 retire l'emplacement `field` de `pico8_entity_kind_t` ; e3 retire `field`/`set_field` de `pico8_struct_class_t` et les deux typedefs, et réécrit le commentaire de contrat de la table.
- **Consommateurs C** : `bootstrap`, `entity`, `entity_geom`, `planet_display`, `slipway`, `ui`, `wormhole`, `wormhole_display`.
- **Descripteurs qui reçoivent des accesseurs** (e1) : `planet` (`jo`, accesseurs publics pour les descendants, déclarés dans `planet.h`), `planet_family` (`qh`, `nl`, `os`, `hj`, `pm`, `ga`), `planet_family_trade` (`nr`, `fr`, `mx`), `probe` (`g_`, publics pour `fv`, déclarés dans `probe.h`), `help` (`fv`), `bg`, `msgbox` (`mh`, `iv`), `research_icon` (`ie`), `score_panel` (`pj`), `scroll` (`qb`), `layout` (`lq`), `mouse_reader` (`nf`), `ui` (`ui`), `ui_button` (`js`, `fq`), `slipway` (`jc`, `jc_mouse_point`), `wormhole` (`mz`).
- **Écrivain typé de `fo`** : `probe_et_hang_fo` (`probe.c`, statique à `fdb294b`) devient public, `pico8_probe_et_hang_fo`, déclaré dans `probe.h`, avec une quatrième branche `jc_too_far`.
- **Lecteurs typés pour les TU** : `pico8_bv_et_t` (`probe.h`) et `pico8_jc_mouse_point_t` (`slipway.h`) publics (e2) ; `pico8_ie_gy()`, `pico8_ie_id()` (`research_icon.c/.h`, e3).
- **TU** : e1 `test_pico8_iv`, `test_pico8_mo`, `test_pico8_ui` (faux descripteurs, oracle) ; e2 `test_pico8_ui` (oracle retiré), `test_pico8_g_`, `test_pico8_mz`, `test_pico8_jc` (exceptions) ; e3 `test_pico8_ie` (exceptions).
- **Aucun fichier nouveau**, aucun TU ajouté ni supprimé.

### Accesseurs (G, e1)

```c
/* pico8_struct.h, dans struct pico8_struct_class, après `probe_of` */
    /* A NULL member: the class has no such field -- what its reader answered
     * for every instance. A reader returns NULL, or 0 leaving *out untouched,
     * when the field is nil; `get_fx` and `get_p_` return its Lua truth.
     * Each is only ever called as `e->cls->get_X(e, ...)`. */
    const pico8_str_seq_t *(*get_nu)(const void *self);
    int (*get_fa)(const void *self, pico8_fixed_t *out);
    const pico8_vec_t *(*get_ra)(const void *self);
    const pico8_vec_t *(*get_p)(const void *self);
    const pico8_qz_box_t *(*get_kq)(const void *self);
    int (*get_ql)(const void *self, pico8_fixed_t *out);
    int (*get_fg)(const void *self, pico8_fixed_t *out);
    int (*get_mt)(const void *self, pico8_fixed_t *out);
    int (*get_fx)(const void *self);
    int (*get_p_)(const void *self);
    void (*set_m_)(void *self, int m_);
```

(Forme reprise de l'annexe de D.11b, Annexe C, moins `get_ma`, `get_dw`, `set_fo`, plus `get_mt`.)

- **Un membre par champ, nommé** (Q6) ; aucun tableau, `switch` ni boucle qui passe d'un nom à un membre (registre). Signatures suivant l'usage des consommateurs : pointeur pour les pointeurs, `int` + sortie pour les fixes qui peuvent être nil, vérité pour les booléens lus comme vérité, `int` pour `set_m_` (les deux sites écrivent un booléen constant).
- **Règle « NULL = absent »** (G, reprise de D.11b) : un descripteur reçoit l'accesseur d'un nom **si et seulement si** son lecteur (ou son écrivain, pour `m_`) répond à ce nom aujourd'hui, directement ou par délégation : table « Qui répond », 136 lignes. Chaque accesseur rend exactement ce que rend la branche de son lecteur : même condition de présence (`has_X`, pointeur NULL), même valeur, même vérité Lua (piège `fx` de `g_`). `set_m_` fait ce que fait la branche `m_` de l'écrivain (membre **et** drapeau `has_m_`). Rejeté : ne poser l'accesseur que sur les classes « atteignables » par chaque consommateur (preuve d'atteignabilité par site ; `hv` et `cv` atteignent presque tout ; un oubli lirait « absent » en silence).
- **Héritage** : les descripteurs de l'arbre `jo` nomment les accesseurs de `jo` (publics, `pico8_jo_get_X`, déclarés dans `planet.h`) ; `fv` nomme ceux de `g_` pour les noms qu'il délègue (`pico8_g__get_X`, `probe.h`) et les siens pour `kq`, `ql`, `fg`, `m_` ; exactement comme ils nomment déjà les dispatchers hérités (D.10a) et les crochets (D.11c). Rejetés : une résolution qui remonte `parent` quand le membre est NULL (machinerie nouvelle ; « NULL = absent » deviendrait « NULL sur toute la chaîne ») ; monter ces champs dans `ke` (option B de Q6, refusée deux fois par l'humain).
- **Cast dans les fonctions-accesseurs** : `return ((const pico8_jo_t *)self)->ra;`. Une fonction-accesseur n'est atteinte que par `e->cls->get_X(e…)` : la table qui la tient est le test de classe, exactement comme les adaptateurs de D.10a et les six crochets de D.11c. **Exception déclarée** à la règle « cast précédé d'un test dans la même fonction », limitée aux fonctions installées dans un membre accesseur d'un descripteur, jamais appelées directement (grep 4 de la « Sortie »). Chaque fonction a **la signature exacte du membre** : aucun cast de pointeur de fonction.
- **Noms** : `get_X`/`set_m_` dans la table ; `pico8_jo_get_X`, `pico8_g__get_X` (publics), `X_get_Y` (statiques) pour les fonctions. Aucun ne correspond au motif `par-nom` (`field`, `_take_`, `field_truthy`) : ce ne sont pas des renommages, ce sont des accès typés nouveaux, et les lecteurs par nom restent jusqu'à e2/e3.
- **Consommateurs** : table « Classe C ». Chaque site teste le membre (`cls->get_X != NULL`) avant l'appel ; `set_m_` asserte sa présence avec le message de l'écrivain générique d'aujourd'hui.

#### Trois champs sans accesseur, deux casts vérifiés : écarts à Q6 déclarés (G)

| Champ, site | Forme retenue | Pourquoi pas un accesseur | Rejeté |
|---|---|---|---|
| `dw` sur `jo.fo` (`planet_display.c:484`) | `fo->cls->probe_of != NULL ? fo->cls->probe_of(fo)->dw : 0` | Les 12 descripteurs qui répondent à `dw` sont **exactement** les 12 porteurs de `probe_of` (D.11c) ; tous répondent « présent » (le fixe du composant) ; le crochet est l'accès typé du groupe (Q7). Recommandation de D.11c, retenue. | `get_dw` sur 12 descripteurs, doublon du crochet |
| `fo` écrit par `jc:hq` (`slipway.c:258`) | `pico8_probe_et_hang_fo(et, self)` : l'aide de `bv` (branches famille `jo`, `jc`, `bv_et`, chacune avec l'`assert` de porteur de l'écrivain d'aujourd'hui), plus une branche `et->cls == pico8_jc_too_far_struct_class()` qui asserte `self->cls == pico8_jc_struct_class()` (l'assert de `jc_too_far_take_jc`) puis écrit `((pico8_jc_too_far_t *)et)->fo = (pico8_jc_t *)self` | Les cibles de `jc:ce` sont un ensemble fermé (planète, `jc`, `jc_too_far`), toutes déjà servies par une aide typée sauf `jc_too_far`. Recommandation de D.11c, retenue. `probe.c` nomme `pico8_jc_too_far_struct_class()` (déjà à `probe.c:292`) : `slipway.c` est dans `DATA_ENGINE_SRCS`, la règle de lien tient. | `set_fo` sur 13 descripteurs ; une copie privée des branches dans `slipway.c` (deux écrivains de `fo` par famille) |
| `ma` (`entity_geom.c:73`, `:82`) | cast vérifié, deux branches (famille `jo`, `qb` exacte) | Receveur **fermé et déjà asserté** par D.11a ; forme A de D.11a, comme `pico8_na`. **Piège** : aujourd'hui la lecture de `ma` (l. 73) précède l'assert (l. 79) ; avec un cast, l'assert (ou le test de branche) doit venir **avant** la lecture, sinon c'est un cast non vérifié. `e == NULL` : `ma` lu 0, rien d'écrit, comme aujourd'hui. | `get_ma` sur 11 descripteurs pour un seul site (l'annexe de D.11b le prévoyait) |
| `ra` écrit au travers (`pico8_na`, `entity_geom.c:64`) | `assert(self == NULL \|\| pico8_jo_is_family(self))` puis `((pico8_jo_t *)self)->ra` | `get_ra` rend `const pico8_vec_t *` (données générées `const` chez `qb`, `iv`, `g_`, `ie`) ; les deux seuls appelants sont `qh:ly` et `nl:ly`. Exception déjà retenue par l'annexe de D.11b. | `get_ra` non `const` (ôterait le `const` par cast) ; un second accesseur mutable pour un site |
| `mt` (`slipway.c:249-251`) | **accesseur** `get_mt` (`fr`, `jc`) | Contrainte de lien : `slipway.c` (dans la liste) ne peut pas nommer `pico8_fr_struct_class()` (`planet_family_trade.c`, hors liste). Q6 la lève. | cast par classe exacte (casse `test_pico8_bg_kj`, note de D.11a) |

Aucune de ces formes ne monte un champ dans `ke`. La note « Découvert à D.11e » les liste comme écarts à la lettre de Q6.

### Oracle accesseurs = lecteurs (G, né en e1, mort en e2)

- **Où** : un test nouveau de `test_pico8_ui`, `test_accessors_agree_with_readers`, **dans son propre état** (fils ou partie fraîche, H-D11d-9), sans constante de hachage, pour ne rien plier dans les constantes existantes.
- **Sur quoi** : chaque entité de `hv` d'une partie démarrée avec galaxie ; une instance de chaque classe de la table « Qui répond » absente de `hv` que le TU sait construire (constructeurs typés de chaque classe) ; les vues atteignables (`jc_mouse_point` par `jc:hq` sur le vide, `jc_too_far` par `jc:ce` hors de portée) ; `bv_et` et les remplaçants sont couverts par leurs membres NULL (lecteurs qui ne répondent à aucun nom C), vérifiés sur l'objet que rendent `pico8_mo()`/`pico8_je()`/`pico8_fm()` avant `pico8_jl()`.
- **Comment** : pour chacun des 10 accesseurs `get_*`, **une comparaison écrite par nom** (pas de table de noms, pas de boucle sur des noms) : membre NULL ⇒ `pico8_entity_get_field(e, "X", &v)` rend 0 ; membre non NULL ⇒ même présence et même valeur (fixe égal, pointeur identique, vérité égale à la vérité Lua de `v`). Pour `set_m_` : sur une instance jetable de chaque classe, membre non NULL ⇒ après `set_m_(e, 1)`, le lecteur rend `m_` vrai, puis faux après `set_m_(e, 0)` ; membre NULL ⇒ `cls->set_field == NULL || !cls->set_field(e, "m_", pico8_value_bool(1))`.
- **Comptes** : ses lignes `pico8_entity_get_field`/`->set_field` sont des **hausses déclarées** de `par-nom` `tests/` (attendu : 11 à 13 lignes) et de `valeur` ; elles partent au commit de e2, avec le test entier. L'oracle n'est pas lent (quelques dizaines d'entités, des comparaisons).
- **Ce qu'il ne prouve pas** : l'ensemble « NULL = absent » sur les classes que le TU ne construit pas ; il est prouvé par la table « Qui répond » (lecture des lecteurs) et contrôlé par le vérificateur (contrôle 5).
- Rejetés : un oracle permanent (test de la couche, qui ne compile plus après e3) ; un oracle dans chaque TU de classe (37 fichiers de plus dans e1) ; comparer par une boucle sur un tableau de noms (registre).

### Lecteurs typés pour les TU (G, e2 et e3 ; notes de D.11d2 à D.11d4)

Six lignes de TU lisent encore par nom parce que `src/` ne donne aucun accès typé à la donnée. Chacune est dans une aide sans paramètre de nom, derrière un test de classe ; elle part dans le **commit qui supprime le lecteur qu'elle appelle**, remplacée par un accès typé.

| Donnée | Forme retenue | Sous-étape | Rejeté |
|---|---|---|---|
| `fo` de `bv_et` | la struct `pico8_bv_et_t` (`{header; fo}`) passe de `probe.c` dans `probe.h`, contrat : « la cible jetable de `bv`, statique, jamais libérée ; seul `fo` est écrit, par `pico8_probe_et_hang_fo` » ; le TU lit `((const pico8_bv_et_t *)t)->fo` après son test de classe existant | e2 (avec `pico8_bv_et_field`) | un accesseur `pico8_bv_et_fo()` (fonction pour une lecture de membre ; la struct publique est le précédent de `pico8_jc_too_far_t`, D.11c) |
| `ra`/`p` du point souris | la struct `pico8_jc_mouse_point_t` (`{header; ra; p}`) passe de `slipway.c` dans `slipway.h`, à côté de `pico8_jc_too_far_t`, avec son contrat (neuve à chaque `jc:hq` sur le vide, jamais libérée, `ra` est le `mp` de l'appelant) ; le TU lit `->ra`/`->p` après son test de classe | e2 (avec `pico8_jc_mouse_point_field`) | des accesseurs `pico8_jc_mouse_point_ra()`/`_p()` (même raison) ; faire rendre des pointeurs à `pico8_jc_end_p`/`_ra` (changerait deux API à valeur que `src/` appelle) |
| `gy` de `ie` (état partagé de la classe) | `const pico8_vec_t *pico8_ie_gy(void)` : `g_ie_gy`, NULL = nil | e3 (avec `pico8_ie_field`) | exposer la variable (état mutable global en écriture libre) ; un membre dans chaque `ie` (la donnée n'appartient à aucune icône, règle « Archétype ou état ») |
| `id` de `ie` (état partagé) | `int pico8_ie_id(pico8_fixed_t *out)` : 0 quand `id` est nil (`g_ie_id` REF NULL), sinon 1 et `*out` = le fixe ; même réponse que la branche `id` de `pico8_ie_field` | e3 | rendre `pico8_value_t` (nouvelle API à valeur, Q12) ; retyper `g_ie_id` lui-même (hors `par-nom`, changement de représentation non demandé) |

- Aucun chemin par nom nouveau, aucune table de noms ; `pico8_ie_gy`/`pico8_ie_id` ne correspondent à aucun motif `par-nom` ; ce sont des lecteurs d'état typés, dans l'esprit de Q6 (NULL/0 = absent).
- Les aides de TU (`bv_et_fo`, `mouse_point_ra`/`_p`, `shared_gy`/`shared_id`) gardent leur nom et leur contrat, seul leur corps change ; **aucune constante ne change** (`MOUSE_POINT_HASH`, `TOO_FAR_HASH`, `HQ_HASH`, constantes de `ie`).
- Chaque ligne retirée est (a1) au message ; les lignes nouvelles de `src/` gagnent au plus `valeur` dans `pico8_ie_id` si elle lit `g_ie_id.kind` (déclarée).

### Suppression (G, e2 et e3)

- **Lecteurs et écrivains** : chaque fonction `pico8_X_field`/`pico8_X_set_field`, ses délégations, ses aides `*_take_*` et leurs appels, ses aides statiques de lecture, et ses deux lignes d'emplacement. Une fonction statique ou un tableau statique que la suppression rend inutilisé part dans le même commit (H1).
- **Sortes** (e2) : les 10 `kind_field`, leurs délégations, la déclaration de `pico8_jo_kind_field` (`planet.h:81`) et son commentaire, l'emplacement `field` de `pico8_entity_kind_t` et son commentaire (`struct.h`), l'entrée `kind_field` des 9 initialiseurs de sortes (la ligne reste : `{pico8_X_struct_new, &pico8_X_kind_rec}`). La garde « prêt » (`g_jo_up`…) n'est pas dans `rec` (note de D.11b) : les gardes gardent leurs autres usages (`pico8_jo_ready` a d'autres appelants).
- **Couche** (e3) : `pico8_entity_get_field`/`set_field` (définitions, déclarations, commentaires), `pico8_ke_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn`, les emplacements `field`/`set_field` de `pico8_struct_class_t`, le commentaire de contrat de la table réécrit (plus de phrase sur `field`/`set_field`), `#include "pico8_value.h"` de `pico8_struct.h` retiré **si** plus rien n'y nomme `pico8_value_t` (baisse `valeur` rapportée).
- **Ordre à l'intérieur de e2 et e3** : descripteurs d'abord (initialiseurs, puis fonctions), pour que chaque état intermédiaire compile ; en e2, `pico8_entity_get_field` reste mais n'a plus d'appelant de `src/` ni de TU : son `assert(cls->field != NULL)` n'est plus atteint.
- **Orphelins** : voir « Inventaire ». Règle : une fonction publique dont le dernier appelant était un lecteur ou un écrivain supprimé part avec sa déclaration **si aucun TU ne l'appelle** ; sinon elle reste, listée. Rejeté : garder les orphelins « pour Q12 » (Q12 portera sur des consommateurs vivants ; un orphelin n'en est pas un).
- **Commentaires** : les contrats devenus faux (« never a field by name » des stockages de composants, `pico8_ke.h:26`, `pico8_research_icon.h:14-19`, `pico8_entity.h:62-63`, `pico8_ui.h:76`, `pico8_struct.h` « `field` is NULL only for… ») sont réécrits ou supprimés selon les quatre sortes de `CLAUDE.md` ; aucun renvoi à D.11e ni au plan.

### Règle de lien (liant)

Aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste. Les symboles nouveaux nommés par des fichiers de la liste : `pico8_probe_et_hang_fo` (`probe.c`, dans la liste) nommé par `slipway.c` ; `pico8_jo_get_X` (`planet.c`) nommés par `planet_family.c` ; les accesseurs de `g_` (`probe.c`) nommés par `help.c` (hors liste : sens autorisé). Aucun site C ne nomme de symbole : il passe par la table. Preuve : les TU liés sur la seule liste (`test_pico8_bg_kj`, `_data_init`, `_map_tables`, `_jb_ol`, `_planet_kj_dispatch`, `_planet_gh_row_lq`, `_ga_export_panel_by` avec `planet_family.c`) se lient (grep 6 de la « Sortie ») ; les listes du `Makefile` ne changent pas.

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la lecture par nom est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours (rendre `probe_et_hang_fo` public sous `pico8_probe_et_hang_fo` n'est pas compté et n'est pas un renommage pour les greps : déclaré) ; alias (macro, `typedef`, pointeur de fonction, enveloppe qui renomme un lecteur) ; registre (tableau, `switch` ou boucle qui passe d'un nom à un membre) ; cast d'en-tête sans test, hors fonctions-accesseurs et crochets ; cast de pointeur de fonction ; cale valeur ↔ typé (fonction de conversion, `pico8_value_t` reconstruit pour un accesseur, lecture par nom déplacée dans une aide non comptée) ; littéral remplaçant un identifiant compté ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; tout changement d'une constante de hachage ; toute modification de `tests/e2e/`, d'un `*.sha256`, de `tools/`, et du `Makefile` hors de la ligne `DATA_RESIDUE_GUARD` (e3).
- **Classification exigée**, au message de chaque commit, pour chaque ligne `par-nom` retirée ou modifiée (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) :
  - **(a1) site typé** : lecture ou écriture par nom remplacée par un accès typé (e1 : 15 lignes, listées dans « Classe C ») ;
  - **(a2) construction supprimée** : définition, délégation, appel d'aide, emplacement, déclaration (e1 : 6 ; e2 : 175 ; e3 : 86) ;
  - **(a2′) entrée retirée d'une ligne gardée** : les 9 initialiseurs de sortes (e2), chacun perd son occurrence et reste ;
  - **(a3) commentaire** dont le contrat devient faux, réécrit : `research_icon.h:19` (e3 ; `entity.h:52` part avec sa déclaration : (a2)) ;
  - **(b)** : signature dont le type change — attendu 0 ;
  - **(c)** : même donnée, autre nom — **attendu 0**.
- **Mesure exigée**, avant et après chaque commit, par catégorie (`valeur`, `type-tag`, `ref-audit`, `par-nom`) et **par fichier** du diff : lignes (`--detail`) **et** occurrences (recette de « État de départ »).
- **Lignes qui gagnent une catégorie**, par fichier et par catégorie, « aucune » compris. Attendues : e1, `par-nom` et `valeur` dans `test_pico8_ui` (oracle, déclarées) ; e1, `type-tag` si un accesseur ou une branche de `pico8_probe_et_hang_fo` asserte un tag (attendu : aucun) ; e2, e3 : **aucune**.
- **Lignes comptées modifiées, non retirées** (fichier:ligne) : e1 aucune hors (a1) ; e2 les 9 initialiseurs de sortes.
- **Δlignes > Δoccurrences** : chaque couple justifié. Attendu : aucun (les emplacements de descripteurs portent deux occurrences : Δoccurrences ≥ Δlignes ; en e1, 21 = 21).
- **Numéros de ligne** : ceux du **parent du commit** (HEAD avant le commit) ; pour e1, ceux de ce PRD si H-D11d-6 tient.

### Empreinte et découpage (G)

| Sous-étape | `src/` | `tests/` | autres | Chemins | `par-nom` `src/` retirées (l. / occ.) | `par-nom` `tests/` après | Coût estimé |
|---|---|---|---|---|---|---|---|
| **e1 — classe C** | `struct.h` ; sites : `bootstrap.c`, `entity.c`, `entity_geom.c`, `planet_display.c`, `slipway.c`, `ui.c`, `wormhole.c`, `wormhole_display.c` ; descripteurs : `planet.c`/`.h`, `planet_family.c`, `planet_family_trade.c`, `probe.c`/`.h`, `help.c`, `bg.c`, `msgbox.c`, `research_icon.c`, `score_panel.c`, `scroll.c`, `layout.c`, `mouse_reader.c`, `ui_button.c` (24) | `iv`, `mo`, `ui` (3) | — | **27** | 21 / 21 | 6 + O (exceptions + oracle, 11 à 13) | ≈ 55 min |
| **e2 — arbre `jo`, sonde, voies** | `planet.c`/`.h`, `planet_family.c`/`.h`, `planet_family_trade.c`/`.h`, `probe.c`/`.h`, `help.c`, `slipway.c`/`.h`, `wormhole.c`/`.h`, `struct.h` (14 ; les `.h` pour les orphelins, les structs publiées `pico8_bv_et_t`/`pico8_jc_mouse_point_t` et les commentaires de contrat devenus faux : « never a field by name », « with no field ») | `ui`, `g_`, `mz`, `jc` (4) | — | **18** | 184 / 220 | **2** (`ie`) | ≈ 50 min |
| **e3 — interface, fin de la couche** | `bg.c`, `economy.c`/`.h`, `entity.c`/`.h`, `ke.c`/`.h`, `layout.c`, `menu.c`, `mouse_reader.c`, `msgbox.c`, `research.c`, `research_icon.c`/`.h`, `score_panel.c`, `scroll.c`, `struct.h`, `trade.c`, `ui.c`/`.h`, `ui_button.c`/`.h` (23) | `ie` (1) | `Makefile` (guard), `docs/plan-3.md` (note) | **26** | 87 / 111 | **0** | ≈ 50 min |

- Chemins distincts de l'étape : 39 de `src/` et 7 TU (la ligne disait ≈ 45 (42, 3) ; les 7 TU sont ceux qui portent encore une ligne `par-nom` à `579a748`), plus le `Makefile` et le plan. Chaque sous-étape sous les ~40 chemins.
- **Pourquoi trois** : l'utilisateur préfère des pas d'environ 50 minutes, chacun vérifié et poussé (décision de D.11d). e1 est le « premier chunk » de la ligne (classe C entière, avec `jc.mt`, et l'oracle) ; e2 et e3 coupent la suppression par familles de fichiers qui se délèguent entre elles (arbre `jo` et sonde : `fv` délègue à `g_`, les filles de `jo` à `jo` ; interface : les lecteurs qui délèguent à `pico8_ke_field`, supprimé avec eux), de sorte que chaque état intermédiaire compile et que la table perde ses emplacements en dernier.
- **Pourquoi e1 n'est pas coupée** : l'oracle ne prouve la conversion que si tous les accesseurs existent quand il tourne ; couper C laisserait des sites C lus par nom après un commit qui prétend les convertir.
- Rejetés : deux sous-étapes (e2 + e3 ≈ 100 min, au-delà de la préférence) ; quatre (e2 coupée en arbre `jo` / sonde : deux pas de ≈ 25 min, coût fixe doublé). Placement des exceptions : chacune dans la sous-étape qui supprime le lecteur qu'elle appelle (e2 : `bv_et`, point souris ; e3 : `ie`) ; rejeté : toutes en e1 (e1 déjà la plus lourde, et l'exposition n'y est pas nécessaire).

### Effet sur `make data-residue`

| Catégorie | Départ (après D.11d, H-D11d-1) | Après e1 | Après e2 | Après e3 | Pourquoi |
|---|---|---|---|---|---|
| `par-nom` `src/` | 292 / 352 occ. (tenu à `579a748`) | **271** / 331 | **87** / 111 | **0** | exact |
| `par-nom` `tests/` | 11 / 11 (7 fichiers) | **6** + O (T3 −5 ; oracle, déclaré) | **2** (`ie`) | **0** | exact hors O |
| `par-nom` fichiers | 38 | 32 : 27 de `src/` (31 − `bootstrap`, `entity_geom`, `planet_display`, `wormhole_display`) + `ie`, `g_`, `mz`, `jc`, `ui` (oracle) | 20 (19 de `src/` + `ie`) | 0 | |
| `valeur`, `type-tag` | 1833, 293 (`579a748`) | rapportés (hausse `valeur` de l'oracle ; baisse des aides C) | forte baisse, rapportée | baisse, rapportée | les `pico8_value_*` des lecteurs, écrivains et aides partent |
| `ref-audit` | 42 | rapporté | rapporté | rapporté | |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch` | inchangé | inchangé | **`…,dispatch,par-nom`** | ligne du plan |

« exact » est une égalité ; « rapporté » est mesuré au commit, écart expliqué.

### Documentation (G)

- `docs/plan-3.md` (e3) : note « Découvert à D.11e » en fin de ligne D.11e, rien d'autre. Elle dit : les trois sous-étapes et leurs chemins ; les comptes réels ; les onze accesseurs et leurs 136 lignes ; les écarts à Q6 (`dw` par `probe_of`, `fo` par `pico8_probe_et_hang_fo`, `ma`/`na` par cast vérifié) ; `get_mt` et la règle de lien ; l'oracle (classes comparées, désaccords trouvés) ; les trois lecteurs typés exposés pour les TU et les six lignes retypées ; les vues (toutes typées, aucune disparue, descripteurs vides) ; les orphelins supprimés et gardés ; ce que D.12 trouve (`ij` et `mz.ox` sur listes ; `bv_et`, `jc_too_far` en cibles `et`) et ce que la phase 2 trouve (accesseurs remplaçables par le noyau et les composants, Q6 reconfirmée « même si la phase 2 en remplace une partie »).
- `docs/glossaire-noms.md`, `docs/trous-de-portage.md` : inchangés (aucun nom minifié ne change de sens, aucun écart port/cart créé ; un écart découvert serait préservé et décrit, §7).
- Commentaires : quatre sortes de `CLAUDE.md` ; le contrat nouveau de la table (NULL = absent, présence, vérité, appel par la table seulement) est un contrat C que le nom ne dit pas.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile` hors de la ligne `DATA_RESIDUE_GUARD` ; aucun e2e ni TU ajouté ou supprimé.
- Toute constante `*_HASH`/`identity[]` de TU.
- Le comportement : chaque accesseur rend ce que rend son lecteur (oracle), chaque site lit la même valeur ; `make e2e` conforme au bit près.
- Les listes (`hv`, `cv`, `cg`, `mz.ox`, `pico8_ij`) restent des `pico8_list_t` : D.12.
- Les API à valeur (Q12) non orphelines.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `-Wall -Wextra -Werror` rend fatale toute fonction `static` inutilisée : aides, prédicats, tableaux que seuls les lecteurs utilisaient partent dans le même commit (grep de chaque symbole après suppression).
- **H2** : un descripteur « vide » s'écrit sans avertissement sous `-Wextra` (aucun membre nommé fictif ; forme au choix de l'implémenteur, sans `-Wno-*`).
- **H3** : aucune constante d'un TU ne change (les accesseurs rendent ce que rendaient les lecteurs) ; une constante qui rougit est un bug de conversion, jamais une mise à jour.
- **H4** : l'oracle ne trouve aucun désaccord ; un désaccord est un bug de l'accesseur (corrigé dans e1), sauf s'il révèle un lecteur qui ment sur sa présence, auquel cas c'est un écart port/cart préservé (§7), rapporté.
- **H5** : la ligne D.11e du plan gradé et sa note D.11d ne changent rien à ce PRD (sections à revérifier ci-dessus).

## Implementation Chunks

Trois sous-étapes, dans cet ordre, un commit chacune, chacune verte seule (`make test`, `make e2e`, `make data-guard`, `make class-coverage-net`, balayage ASan+UBSan des TU), vérifiée et poussée avant la suivante, avec ses comptes, sa classification, ses lignes gagnées, son `gcov`, ses mutations et son tableau « Runs » :

1. **D.11e1 — classe C** (27 chemins). Ordre conseillé :
   1. mesure de départ (lignes et occurrences par fichier ; H-D11d-1 et H-D11d-2 constatées, sinon arrêt et reprise du PRD) ; `gcov` de départ ;
   2. `pico8_struct.h` : les onze membres et leur contrat ;
   3. fonctions-accesseurs, classe par classe, en relisant la branche du lecteur (ou de l'écrivain) de chaque nom ; initialiseurs des 136 lignes, descendants nommant ceux de leur parente ;
   4. `pico8_probe_et_hang_fo` public, branche `jc_too_far` ;
   5. les 14 sites, `jc.mt`, les 8 appels ; les quatre aides par nom et les orphelins statiques qu'elles laissent (`pico8_dg_truthy`, `pico8_mzd_value_vec`, `pico8_jc_entity_value`, `jc_take_fixed`, `pico8_entity_geom_field_*`, `pico8_ui_field_raw`, `pico8_mzd_field_vec`) ;
   6. TU : trois faux descripteurs, oracle ;
   7. `make test`, `make e2e`, `make class-coverage-net` ; `gcov` d'arrivée ; mutations M1-M13 ; balayage ASan+UBSan ; mesure d'arrivée, classification, message.
   `par-nom` : `src/` 292 → **271** ; `tests/` 11 → **6** + O (oracle, déclaré).
2. **D.11e2 — arbre `jo`, sonde, voies** (18 chemins) : oracle retiré de `test_pico8_ui` ; `pico8_bv_et_t` dans `probe.h`, `pico8_jc_mouse_point_t` dans `slipway.h`, et les aides `bv_et_fo` (`test_pico8_g_`, `test_pico8_mz`) et `mouse_point_ra`/`_p` (`test_pico8_jc`) les lisent typées, constantes égales ; puis `planet`, `planet_family`, `planet_family_trade`, `probe`, `help`, `slipway`, `wormhole` : emplacements, lecteurs, écrivains, aides, vues (`fm`, `bv_et`, `jc_too_far`, `jc_mouse_point` réduit à `{get_ra, get_p}`, `mz_jj`, `nr_ldown_empty`) ; les 10 `kind_field`, leurs entrées et l'emplacement de `pico8_entity_kind_t` ; orphelins (`pico8_g__et`, `pico8_qh_et`, `pico8_hj_pk`). `par-nom` : `src/` 271 → **87** ; `tests/` → **2** (`test_pico8_ie`). Mutations M14, M15, M18, M19.
3. **D.11e3 — interface et fin de la couche** (26 chemins) : `pico8_ie_gy()`/`pico8_ie_id()` et les aides `shared_gy`/`shared_id` de `test_pico8_ie` retypées ; puis `bg`, `economy`, `layout`, `menu`, `mouse_reader`, `msgbox`, `research`, `research_icon`, `score_panel`, `scroll`, `trade`, `ui`, `ui_button` (lecteurs, écrivains, aides, vues `mo`, `je`, `ha_pz`, `pj_cr`, `ie_ke_view`, orphelins `pico8_hy_hl`, `pico8_js_h_` ; `pico8_lq_bg`, `pico8_pj_ed` gardées) ; puis `pico8_ke_field`, `pico8_entity_get_field`/`set_field`, typedefs et emplacements de `pico8_struct_class_t` ; `DATA_RESIDUE_GUARD` += `par-nom` ; note « Découvert à D.11e ». `par-nom` = **0**. Mutations M16, M17, M20, M21.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (valeurs, présences, identités, effets d'un appel), pas la présence d'une ligne. D.11e change la façon dont `src/` lit quatorze champs, puis retire une couche que plus rien n'atteint. La preuve :

- e1 : l'**oracle** compare chaque accesseur à son lecteur sur des instances réelles ; les **mutations** montrent que les TU T1 (restés après D.11d) voient chaque accesseur ; `gcov` montre chaque ligne ajoutée exécutée ; les 26 e2e restent conformes au bit près ;
- e2, e3 : `gcov` montre qu'**aucune ligne survivante** de `src/` n'a cessé d'être exécutée par les TU ; les mutations montrent que les descripteurs édités gardent leurs accesseurs vivants ; e2e conformes.

### TU touchés (G)

1. **e1** : `test_pico8_iv`, `test_pico8_mo`, `test_pico8_ui` (faux descripteurs : table « Classes locales de TU » ; tests et constantes inchangés) ; `test_pico8_ui` reçoit l'oracle. Aucun autre TU ne change ; un TU qui casse sans nommer un symbole changé est un **bug de e1** : il part à `help-me-debug`.
2. **e2** : `test_pico8_ui` (oracle retiré, avec ses aides éventuelles) ; `test_pico8_g_`, `test_pico8_mz` (`bv_et_fo`), `test_pico8_jc` (`mouse_point_ra`/`_p`) : corps typé, contrat et constantes inchangés. Un autre TU qui nomme un symbole retiré le suit dans le même commit, listé.
3. **e3** : `test_pico8_ie` (`shared_gy`/`shared_id`) ; même règle.
4. **Constantes** : aucune `*_HASH`/`identity[]` ne change, à aucune sous-étape.
5. **Assertion nouvelle** : admise seulement pour couvrir une ligne ajoutée non exécutée (e1) ou pour rougir une mutation qui ne rougit rien (trou), dans un test gardé de la classe, en accès typé, listée au message avec la ligne qu'elle couvre ; rien de lent, aucun fichier d'événements.
6. **Prior art** : table de méthodes typée et descendants nommant les fonctions de leur parente (D.10a) ; crochets `emitter_of`/`probe_of` et aides d'hôte à ensemble fermé (D.11c) ; cast vérifié sur famille fermée (D.11a, `pico8_jc_end_p`) ; lien parent (D.11b) ; oracle de transition `test_kind_rec_matches_the_reader` (D.11b, retiré par D.11d).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`), lancés comme le `Makefile` ; union des lignes de `src/` exécutées.

- e1 : chaque ligne **ajoutée ou modifiée** de `src/` (fonctions-accesseurs, sites, branche `jc_too_far`, casts de `ma`/`na`) est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée (règle 5) ou justifiée au message (accesseur posé par la règle « NULL = absent » que rien n'atteint : attendu pour `get_p` de `jc_mouse_point`, `get_mt` de `jc`, et les accesseurs de classes jamais inscrites dans le registre que leur site parcourt ; chacun listé, l'oracle l'exécute).
- e2, e3 : `L` = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide**. Une ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail, fichier sauvegardé par `cp` avant et restauré par `cp` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan, `test_pico8_planet_gh_row_lq`) ; chacune doit rougir au moins le TU nommé, sinon c'est un trou, couvert par une assertion (règle 5) ou justifié (mutation équivalente, démontrée) :

- e1 :
  - **M1** `pico8_jo_get_nu` rend NULL → `test_pico8_jo` (ou tout TU de planète qui lit `cg("jo")`) ;
  - **M2** `.get_fa` retiré du descripteur de `js` → `test_pico8_ui`/`test_pico8_js` (calques de `ui.ic`, `dc`) ;
  - **M3** `get_p_` de `fq` rend 0 → `test_pico8_fq` (le bouton ne sort plus de `hv`) ;
  - **M4** `.set_m_` retiré du descripteur de `jo` → `test_pico8_ui` (survol d'une planète : l'`assert` de `ui:gw` arrête) ;
  - **M5** `pico8_ma` n'écrit plus `*ma` → `test_pico8_qb`/`test_pico8_jo` (décroissance de `ma`) ;
  - **M6** `get_fx` de `g_` rend 0 → `test_pico8_mz` (`mz.ox` sans sonde) ;
  - **M7** branche `jc_too_far` de `pico8_probe_et_hang_fo` retirée → `test_pico8_jc` (voie trop longue : l'`assert` final arrête) ;
  - **M8** `.get_ra` retiré du descripteur de `jc_mouse_point` → `test_pico8_jc` (`gd` du point de souris), sinon trou justifié ;
  - **M9** `.get_mt` retiré du descripteur de `fr` → `test_pico8_fr` (jc tenu depuis un `fr`, H-D11d-8) / `test_pico8_jc` ;
  - **M10** `planet_display.c` lit `dw` = 0 au lieu de `probe_of(fo)->dw` → un TU T1 de `jo` qui dessine une planète dont `fo` est posé, sinon assertion ajoutée (règle 5) ;
  - **M11** `get_p_` de `mz` ignore `has_p_` → l'oracle (et `test_pico8_mz`) ;
  - **M12** `get_fa` de `mh` rend `fa + 1` → l'oracle (et `test_pico8_mh`) ;
  - **M13** `fake_toast_get_p_` rend toujours 0 → `test_pico8_iv` (le toast ne sort plus de `hv`) : le faux est bien câblé.
- e2 : **M14**, **M15** : dans deux fichiers différents édités par e2, retirer un accesseur **vivant** d'un descripteur dont e2 retire les lignes `.field`/`.set_field` (exemples : `.get_ra` de `g_` → `test_pico8_g_` ; `.get_fx` de `qh` → `test_pico8_mz`) : chacun rougit un TU (les éditions de descripteurs n'ont rien emporté de vivant).
- e3 : **M16**, **M17**, même forme (exemples : `.set_m_` de `js` → `test_pico8_ui`/`test_pico8_js` ; `.get_fa` de `pj` → `test_pico8_pj`).
- e2, exceptions : **M18** la branche `bv_et` de `pico8_probe_et_hang_fo` n'écrit plus `g_bv_et.fo` → `test_pico8_g_`/`test_pico8_mz` (lecture typée de `bv_et_fo`) ; **M19** `jc:hq` ne pose plus `point->p` (NULL) → `test_pico8_jc` (`MOUSE_POINT_HASH`, `test_hq_mouse_point`).
- e3, exceptions : **M20** `pico8_ie_gy()` rend NULL → `test_pico8_ie` (constantes pliant `gy`) ; **M21** `ie:hb` n'incrémente plus `g_ie_id` (`research_icon.c:163-165`) → `test_pico8_ie` (`shared_id_fixed`).
- Cibles vérifiées à `fdb294b` (`src/` identique à `579a748`) : `entity_geom.c:72-86` (`pico8_ma`), `probe.c:188-209` (`probe_et_hang_fo`), `slipway.c:249-258`, `planet_display.c:478-488`, `wormhole.c:62-74`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même membre, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit. Aucun e2e ajouté.
- `make class-coverage-net` à chaque sous-étape (ligne du plan : « oui ») : sortie identique à `tools/class_coverage_net_expected.txt`.
- `make sanitize-net` : **non exigé** (ligne du plan ; aucune durée de vie ni propriétaire ne change).
- **Balayage ASan+UBSan des TU** à chaque sous-étape : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.7b, D.10a, D.10b, D.11a, D.11c. Sortie propre (un accesseur qui caste vers le mauvais type est ce que ce balayage voit).
- Aucun TU ne rejoue un fichier d'événements ; l'oracle n'est pas lent.

### Vérification, budget et tableau « Runs »

**Budget : environ 2 h 35 de temps réel pour l'étape** (e1 ≈ 55 min, e2 ≈ 50 min, e3 ≈ 50 min), mesures, `gcov`, mutations et filets compris ; **plafonds** e1 80 min, e2 75 min, e3 75 min. Base mesurée : **D.11c ≈ 23 min pour 37 chemins** (≈ 0,6 min par chemin : build, lancement, mesure), D.11b ≈ 15 min pour 22 chemins, **D.11d1..d4 ≈ 21 à 27 min chacune pour 9 à 12 TU** réécrits (≈ 2,3 min par fichier lourd). D.11e1 touche 27 chemins (≈ 17 min au même rythme) mais **écrit** environ 55 fonctions-accesseurs et 136 lignes d'initialiseurs en relisant chaque branche de lecteur (≈ 20 min), un oracle (≈ 8 min) et 13 mutations (≈ 10 min) ; e2 et e3 suppriment surtout (≈ 1 500 et ≈ 900 lignes non comptées), guidés par le compilateur, avec `gcov` complet et `class-coverage-net` (≈ 10 min chacun). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute (recompilation large : `pico8_struct.h` change à chaque sous-étape) |
| `make e2e` | ~1 s |
| `make class-coverage-net` | quelques dizaines de secondes |
| `make line-coverage EVENTS="tests/e2e/*.json"` | moins d'une minute, une fois en e1 |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes par sous-étape |
| mutations (e1 : 13 ; e2, e3 : 4 chacune ; un `make` ciblé chacune) | e1 < 12 min ; e2, e3 < 6 min |
| balayage ASan+UBSan des TU | quelques minutes par sous-étape |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà d'un plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; une sous-étape inachevée n'est pas commise. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + sous-étape + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | `579a748`, avant e1 | 0 | `par-nom 303 38` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que les chemins de sa ligne de la table « Empreinte » (plus un TU suiveur listé). Ni `tools/`, ni `tests/e2e/`, ni `*.sha256` ; le `Makefile` seulement en e3, et seulement la ligne `DATA_RESIDUE_GUARD`.
2. **Greps** de la « Sortie », exactement, après e3 ; après e1, aucun `pico8_entity_get_field`/`set_field` dans les 8 fichiers de sites et `strcmp(name` absent de `entity_geom.c`, `bootstrap.c`, `planet_display.c`, `wormhole_display.c` ; après e2, `grep -nE "$(pattern par-nom)"` vide dans les 8 fichiers `.c` de e2, et dans `tests/` ne rend que `test_pico8_ie` (2 lignes).
3. **Casts** : chaque cast d'un en-tête ou d'un `void *` vers une struct, ajouté ou gardé dans une fonction touchée, est précédé dans la même fonction d'un test de lignée, de famille ou de classe (`pico8_ma`, `pico8_na`, branche `jc_too_far`) — sauf dans les fonctions-accesseurs, dont aucun appel direct n'existe (grep 4). Aucun cast de pointeur de fonction.
4. **Liens** : grep 6 ; aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole nouveau défini hors de la liste.
5. **« NULL = absent »** : pour chaque accesseur, l'ensemble des descripteurs qui le posent est celui de la table « Qui répond », ni plus ni moins (grep des initialiseurs, comparé ligne à ligne) ; chaque fonction-accesseur relue contre la branche de son lecteur (présence, valeur, vérité Lua ; piège `fx` de `g_`).
6. **Sites** : les 14 sites, `jc.mt` et les 8 appels de l'inventaire convertis en e1 ; l'assert de `set_m_` garde le message de l'écrivain générique ; `pico8_ma` asserte avant de lire.
7. **Oracle** : présent et vert en e1, absent en e2 ; ses lignes gagnées déclarées.
8. **Constantes** : aucune `*_HASH`/`identity[]` changée (`git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)'` vide).
9. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (b) = 0, (c) = 0 ; lignes gagnées et lignes comptées modifiées listées ; aucun registre, alias, cale valeur ↔ typé.
10. **Vues** : onze descripteurs de vues sans `field`/`set_field`, présents ; aucune vue supprimée ; `jc_mouse_point` avec `get_ra`/`get_p`.
11. **Orphelins** : aucune fonction publique restée sans appelant dans `src/` ni `tests/` parmi celles que l'étape a privées de leur appelant (liste au message) ; aucune fonction retirée qui garde un appelant.
12. **Preuve** : `gcov` (e1 : lignes ajoutées exécutées ou justifiées ; e2/e3 : `L` vide), mutations restaurées par `cp`, `class-coverage-net`, balayage ASan+UBSan rapportés.
13. **Commentaires** : quatre sortes de `CLAUDE.md` ; les contrats devenus faux réécrits ; aucun renvoi à D.11e ni au plan dans `src/` ou `tests/`.
14. **Docs** : note « Découvert à D.11e » en fin de ligne D.11e (e3) ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `579a748` (ou un descendant qui ne touche que `docs/`), propre ; ou si `make data-residue` n'y rend pas `par-nom` 303 sur 38 fichiers (`src/` 292 / 352 occurrences, 31 fichiers ; `tests/` 11 / 11, 7 fichiers) sans explication ; ou si les 11 lignes de `tests/` n'y sont pas les 5 T3 et les 6 exceptions de l'« Inventaire ».
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) change, ou si un test devrait changer d'intention pour passer.
- `FAILED:` si un champ ou un site ne tombe sous aucune forme de ce PRD ni du §5 (arrêt « champ sans règle »), ou si un écart port/cart découvert (désaccord de l'oracle qui révèle un lecteur infidèle au cart) n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si le balayage ASan+UBSan des TU ou `make class-coverage-net` n'est pas propre, à l'une des trois sous-étapes.

**Périmètre et forme**

- `FAILED:` si un fichier de `tools/` change, ou si le `Makefile` change ailleurs que sur la ligne `DATA_RESIDUE_GUARD`, ou avant e3.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit (après e3), ou si les greps partiels du contrôle 2 échouent après e1 ou e2.
- `FAILED:` si un accesseur est posé sur un descripteur hors de la table « Qui répond », ou manque sur un descripteur de la table ; si un membre accesseur est ajouté qui n'est pas l'un des onze ; si `dw`, `fo` ou `ma` reçoivent un accesseur sans que le message dise pourquoi la forme retenue ici ne tient pas.
- `FAILED:` si une fonction-accesseur est appelée directement, si un cast vers une struct ajouté hors fonction-accesseur n'a pas son test de lignée, de famille ou de classe dans la même fonction, ou si un pointeur de fonction est casté.
- `FAILED:` si un champ est monté dans `pico8_ke_t` (Q6, refusée par l'humain).
- `FAILED:` si une vue est supprimée, ou si une vue garde un lecteur ou un écrivain par nom après e3.
- `FAILED:` si la comparaison rec ↔ `pico8_X_start` disparaît (grep 5 < 25) à l'une des sous-étapes.
- `FAILED:` si l'oracle manque en e1 ou survit à e2.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` ne contient pas `par-nom` après e3.
- `FAILED:` si `par-nom` `src/` ne vaut pas 271 après e1, 87 après e2, 0 après e3 ; ou si `par-nom` `tests/` ne vaut pas 6 + O après e1 (les 6 exceptions et les lignes déclarées de l'oracle), **2** après e2 (les deux lignes de `test_pico8_ie`), **0** après e3.
- `FAILED:` si `pico8_bv_et_t` ou `pico8_jc_mouse_point_t` ne sont pas publics après e2, ou `pico8_ie_gy()`/`pico8_ie_id()` absents après e3 ; si une ligne d'exception est retypée autrement que par ces accès (nouvelle aide par nom, registre, `pico8_value_t` reconstruit) ; si une constante d'un TU d'exception change.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, registre, renommage d'un identifiant vivant pour échapper aux greps, commentaire compté réécrit hors de son sujet, cale valeur ↔ typé, lecture par nom déplacée dans une aide non comptée, littéral remplaçant un identifiant compté, (b) ou (c) non déclarée.
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(a2′)/(a3)/(b)/(c) avec numéros du parent ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées ; la justification de chaque Δlignes > Δoccurrences ; les écarts à Q6 (e1) ; les orphelins supprimés et gardés (e2, e3) ; le rapport `gcov` ; les mutations. Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne ajoutée en e1 n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` n'est pas vide en e2 ou e3.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si un plafond est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.11e », ou si elle omet l'un de ces éléments : les comptes réels, les accesseurs, les écarts à Q6, `get_mt` et la règle de lien, l'oracle, les lecteurs typés exposés pour les TU (`pico8_bv_et_t`, `pico8_jc_mouse_point_t`, `pico8_ie_gy`/`pico8_ie_id`), les vues, les orphelins, ce que D.12 et la phase 2 trouvent.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- Les listes (`pico8_list_t` de `hv`, `cv`, `cg`, `mz.ox`, `pico8_ij`, `qy`…) : D.12. `pico8_ij` garde son `pico8_value_t *out_nearest`.
- Les API à valeur non orphelines (`pico8_jo_hf`, `pico8_rn`, `pico8_value_tostr`…) et les catégories `valeur`, `type-tag`, `ref-audit` : Q12 (étude D.9b), ouverte.
- La composition de la phase 2 (noyau, archétypes, composants hors émetteur et sonde) ; remplacer les accesseurs par le noyau : phase 2.
- Supprimer l'en-tête ou la classe d'une vue ; changer `nr_ldown_empty` en NULL.
- Réécrire les prédicats de porteurs (`cls->hq == pico8_bv_dispatch …`) sur `probe_of` (D.11c le permettait ; non requis).
- Tout nouveau test hors oracle et assertions de la règle 5 ; tout e2e ; `tools/` ; les listes de sources du `Makefile` ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **Accesseurs** : onze membres typés, un par champ (Q6, reconfirmée), sur les 136 descripteurs qui répondent aujourd'hui ; héritage par nommage explicite des fonctions de la parente. Rejetés : résolution par `parent`, remontée dans `ke`, table de noms.
  - **`dw`, `fo`** : formes de D.11c (crochet `probe_of`, écrivain typé de `bv` public avec une branche `jc_too_far`), recommandations de son PRD retenues ; **`ma`, `na`** : cast vérifié sur famille fermée déjà assertée. Écarts à la lettre de Q6, déclarés ; aucun champ dans `ke`.
  - **`jc.mt`** : `get_mt` sur `fr` et `jc` ; règle de lien tenue sans toucher au `Makefile`.
  - **Oracle** : un test de `test_pico8_ui`, né en e1, retiré par le commit de e2 ; comparaisons écrites par nom, sans table.
  - **Vues** : toutes typées, aucune supprimée, descripteurs vides gardés pour leur identité.
  - **Orphelins** : supprimés s'ils n'ont plus d'appelant ni dans `src/` ni dans `tests/`.
  - **Exceptions de D.11d** : `pico8_bv_et_t` et `pico8_jc_mouse_point_t` publics (e2), `pico8_ie_gy()`/`pico8_ie_id()` (e3), chacun dans la sous-étape qui retire le lecteur par nom correspondant.
  - **Découpage** : trois sous-étapes (≈ 55, 50, 50 min) ; e1 = classe C entière (« premier chunk » de la ligne).
  - **`Makefile`** : seule la ligne `DATA_RESIDUE_GUARD` change (ligne du plan) ; les listes de sources restent.
- **Révision après D.11d** : H-D11d-1 et H-D11d-3 fausses (11 lignes de TU au départ, dont 6 exceptions sans lecteur typé dans `src/`), H-D11d-5 tenue avec 59 tests supprimés au lieu de 47 ; les autres tenues.
- **Surprises de mesure, pour `plan-pilot`** : (1) les numéros `wormhole.c:78`/`entity_geom.c:99` de la ligne D.11e sont d'un arbre ancien (aujourd'hui `:70`/`:104`) ; (2) 9 sortes, pas 11 (déjà dit par la note de D.11b, la ligne ne l'a pas repris) ; (3) le volume réel de e2/e3 est dominé par ≈ 2 400 lignes non comptées (corps de lecteurs et d'aides) pour 271 lignes comptées ; (4) `g_` sert `fx` comme un fixe, toujours vrai en Lua : `get_fx` de `g_` rend 1 sans lire la valeur ; (5) dans `pico8_ma`, la lecture de `ma` précède l'`assert` de famille : le cast vérifié impose de les inverser ; (6) cinq fonctions publiques à valeur (Q12) n'ont plus d'appelant après les lecteurs (`pico8_g__et`, `pico8_qh_et`, `pico8_hj_pk`, `pico8_hy_hl`, `pico8_js_h_`) : elles partent, ce qui retire cinq prototypes de la liste de Q12.
- **Pour D.12** : `pico8_ij` lit `p` par `get_p` sur les éléments de `cg("jo")`, `cg("np")`, `mz.ox` ; quand ces listes deviennent des listes de pointeurs, le site garde l'accesseur.
- **Pour la phase 2** : `get_nu` (noyau : tags `nu`), `get_fa`/`get_ra`/`get_p` (position et calque, candidats de composant), `get_p_`/`get_fx` : l'humain a accepté que la phase 2 en remplace une partie.
- **Inventaire exhaustif** : Annexe D (une ligne par ligne comptée de `src/`, numéros à `fdb294b`).

## Annexe A — textes du plan recopiés (`docs/plan-3.md` à `579a748`, par extraction de lignes ; seule la ligne D.11d a changé depuis `fdb294b`, par ses notes « Découvert à D.11d1..d4 »)

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

### §5, phase D, en entier (l. 228-280) : puces, note de D.0, « Liste de D » avec empreinte, règles communes et ordre, arrêt « champ sans règle », décisions de D.9, comptes au départ de D.10, lignes D.2 à D.12 avec toutes leurs notes « Découvert à D.x » (D.11d1 à D.11d4 comprises)

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
> - D.11e — `par-nom`, suppression de l'infrastructure. Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`, laissé à D.11a ; accesseur typé par champ dans la table de classe, NULL = absent, Q6 ; l'oracle accesseurs = lecteurs naît et meurt dans l'étape). Découvert à D.11b (pour D.11d/D.11e) : les littéraux `s`/`od` des `pico8_X_kind_rec` doublent ceux de `pico8_X_start` ; seul `test_kind_rec_matches_the_reader` en garde l'égalité — à sa suppression, garder une comparaison rec ↔ `pico8_X_start` ou des constantes nommées partagées. Puis : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks. Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non. Q6 reconfirmée (humain, 2026-09-29) après la revue du plan : accesseurs typés par champ, pas de remontée dans `ke`, même si la phase 2 en remplace une partie.
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

## Annexe B — étude D.9 (`docs/prd/done/plan3-phaseD-etape9-etude.md`, inchangée à `fdb294b`) : infrastructure, classe C, table des étapes, Q6, Q11

### §1.1, infrastructure et classes de sites (l. 15-45)

> ### 1.1 Inventaire, `src/` (462 lignes)
>
> **Infrastructure : 298 lignes.** Elle meurt quand plus aucun consommateur par nom ne l'atteint :
>
> | Nature | Lignes | Objets |
> |---|---|---|
> | lecteurs `field` des descripteurs | 65 | 40 fonctions `pico8_X_field` (dont 3 remplaçants : `mo`, `je`, `fm`) |
> | écrivains `set_field` | 74 | 18 fonctions `pico8_X_set_field` |
> | aides `*_take_*` (définitions) | 57 | 55 aides `static` |
> | lecteurs de sorte (`kind_field`, D.5c) | 19 | 10 fonctions (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`) |
> | emplacements `field`/`set_field` des 40 descripteurs et des 11 sortes | 69 | `pico8_struct_class_t`, `pico8_entity_kind_t` |
> | en-têtes (typedefs `pico8_struct_field_fn`/`_set_field_fn`, prototypes, commentaires) | 14 | `pico8_struct.h`, `pico8_entity.h`, `pico8_ke.h`… |
>
> **Sites : 164 lignes comptées et 82 lignes d'appel d'aides**, classées par forme de remplacement :
>
> | Classe | Forme retenue | Lignes comptées | Appels d'aides | Fichiers |
> |---|---|---|---|---|
> | **A** — receveur de type connu : `self` typé, membre pointeur typé (`self->lq`, `self->qu`, `dk`), ou élément d'une famille fixée par le cart (`cg.jo`, `jo.by`/`cc` → `trade`, `trade.rl` → `jc`), `me`/`t` de `ke` sur `hv` | accès au membre typé (`pt->mi`, `pt->has_mi`), avec un cast vérifié de l'en-tête vers la famille | 41 | 36 | 18 |
> | **B** — membre d'une fille lu sur la famille `jo` (`qh`, `nl`, `hp`, `px`, `s`, `ba`, `qw`) | test d'appartenance puis accès typé : `pico8_qh_of(jo)` rend NULL hors de la lignée (Q5) | 7 | 1 | 5 |
> | **C** — champ transversal lu sur une entité quelconque (`hv`, `cv`, calques de `dc`, `ui.ic`, `ui.o_`/`gj`, `jc.et`, `jo.fo`) : `p_`, `fa`, `ra`, `nu`, `ma`, `kq`, `dw`, `m_`, `ql`, `fg`, présence de `gv` et des cinq noms `bm` | accesseur typé par champ dans la table de la classe, NULL = absent (Q6) | 16 | 6 | 6 |
> | **D** — singleton à remplaçant : `fm` (remplaçant `{jk,nz}` ou `hy`), `je` (remplaçant ou `nf`), `mo` (remplaçant ou `qb`) | accesseur de singleton typé (`pico8_fm_hy()`, `pico8_je_nf()`, `pico8_mo_qb()`, NULL sur le remplaçant) ; le remplaçant de `fm` reste une struct typée pour `jk`/`nz` | 8 | 21 | 11 |
> | **E** — nom variable | `pico8_mv(e, method_name)` : disparaît avec le point 2 ; `pico8_ui_je_button_table(g_je, bt)`, `bt` ∈ {`l`,`r`} : deux accès (`nf.l`, `nf.r`), typés par D.7a | 2 | — | 2 |
> | **F** — vue lue par `pz` : `p` peut être la vue `ha_pz`, `fv`, `jo`, `pj`… ; lit `ep`, `eo`, `hi`, `ff`, `ca`, `gq`, `ra` | struct locale de cadre (`{ep, eo, hi, ff, ca, gq, ra}`), remplie par chacun des cinq appelants depuis ses membres typés | 5 | 2 | 1 |
> | **G** — `*_take_*` hors descripteur, qui déballe un argument ou un retour de méthode (`rp`, `me` de `ks`, `kn`, `mt`, résultat de `my`, `g_ie_gy`) | disparaît avec le point 2 : l'argument arrive typé | 9 | — | 6 |
> | **H** — définitions des aides par nom (`field_truthy`, `field_fixed`, `pico8_probe_field_*`, `fm_field_*`, `pico8_ui_field_*`…, 27 aides dans 15 fichiers) | disparaissent avec leurs appelants (A, B, C, D, M) | 39 | — | 15 |
> | **K** — lecture des sortes : `pico8_ie_ke_view` lit `rr`, `s`, `me`, `od` par `kind->field` | enregistrement typé par sorte (`{rr, has_s, s, me, has_od, od}`), résolu le long de la chaîne à la génération, pointé par `pico8_entity_kind_t` | 4 | — | 1 |
> | **M** — groupe partagé entre branches : émetteur de particules (`ei`, `ez`, `f_`, `g__mm` : `pn`, `t`, `du`, `py`, `ki`, `pk`, `qy`, `fa`, `ra`, `qo`, `qg`, `ro`, `q_`, `k_`, `fc`, sur `g_`, `fv`, `hj`, `pm`, `mx`, `ga`) et sonde (`bv`, `dl`, `d_` : `ra`, `p`, `et`, `fo`, `me`, `od`, `dq`, `fp`, `ik`, `p_`, `dw`, `qs`, sur `g_`, `fv`, les huit classes de l'arbre `qh`, `mz` pour `dl`) | **aucune forme de D** : c'est la règle 2 de la phase 2 (composant). Voir Q7 | 16 | 16 | 2 |
> | **X** — pont `js`/`lq` (`pico8_js_new_table`, `pico8_lq_new_table`) | **meurt avec D.7a** (plus les 12 définitions `button_take_*`/`lq_take_*` de l'infrastructure) | 17 | — | 2 |
> | **Total** | | **164** | **82** | |
>
> Genres lus : fixes (`mi`, `jh`, `hs`, `t`, `fa`, `ma`, `dw`, `en`, `trade`, `od`, `s`, `qw`…), booléens (`p_`, `m_`, `fx`), `hf` (booléen ou fixe, TROU-3), chaînes (`me`, `px`, `hl`, `id`), REF (listes `jy`/`cc`/`by`/`lp`/`rl`/`oe`/`qy`/`invented`, `kw`, `hn`, `hg`, `nk`, `po`, vecteurs `ra`/`p`, entités `qu`/`re`/`et`/`fo`/`o_`). Écritures : `kr`, `mi`, `cc`, `by`, `lq`, `ma`, `v`, `m_`, `fo`, `qy`, `ra`, `p`, `et`, `p_`.

### Étapes proposées : en-tête et ligne D.11e (l. 183-184, 192)

> | Étape | Contenu | Empreinte estimée | Compte après |
> |---|---|---|---|
> | **D.11e** | suppression de l'infrastructure : 40 lecteurs, 18 écrivains, 10 `kind_field` et les emplacements des 11 sortes, 55 aides `*_take_*` (43 après D.7a), `pico8_entity_get_field`/`set_field`, typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les 11 structs-vues deviennent typées ou disparaissent : remplaçants `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view` | ≈ 35 fichiers de `src/` | `par-nom` = 0, dans `DATA_RESIDUE_GUARD` |

### Q6 et Q11 (l. 226-227, 241-242)

> **Q6. Champs transversaux (C : `p_`, `fa`, `ra`, `nu`, `ma`, `kq`, `dw`, `m_`, `ql`, `fg`, présence de `gv`) : (A) un accesseur typé par champ dans la table de classe, NULL = absent ; (B) monter ces champs dans `ke` ?**
> A. Le noyau de la phase 2 ne nomme que la table de méthodes, `me`, `t` et `nu`. Monter `p_`/`fa`/`ra` dans `ke` changerait la mise en page de 40 structs pour 22 lignes.
>
> **Q11. Tests des descripteurs (T2 : environ 145 lignes `cls->field`/`set_field` et les tests de mort des `assert` d'écrivain) : les supprimer avec les descripteurs, comme tests de la couche (§6, premier point) ?**
> Oui. Leur objet, la lecture et l'écriture par nom, disparaît. Les valeurs des membres restent couvertes par les TU en accès typé (T1) et par les e2e.

## Annexe C — « Reporté à D.11e » du PRD de D.11b (`docs/prd/done/plan3-phaseD-etape11b-lectures-sans-receveur-type.md`, l. 781-896, recopiée telle quelle ; ses numéros sont ceux de `d4c4f4c`, remesurés dans l'« Inventaire » de ce PRD)

> La revue du plan après D.11a a déplacé C en tête de D.11e (« ses descripteurs sont ceux que D.11e rouvre »). Ce qui suit est ce que la première version de ce PRD avait préparé pour C, **gardé pour le PRD de D.11e**, qui le remesure et le tranche à nouveau : rien de cette annexe ne lie D.11b, et aucun critère `FAILED:` de D.11b ne s'y rapporte. Les numéros de ligne des **sites** sont ceux de `d4c4f4c` ; ceux des **appelants** cités dans la colonne « Receveur » viennent de `5e75832` et sont à relire.
>
> ### C.1 Inventaire
>
> **Classe C — champs transversaux (D.11e, premier chunk ; numéros de `d4c4f4c`).** 14 sites comptés et 4 définitions d'aides comptées (18 lignes), 8 appels d'aides, 8 fichiers ; dont les trois reclassés par D.11a (✱).
>
> | Fichier:ligne | Fonction | Membre | Receveur (d'où il vient) | Remplacement | c/h/d |
> |---|---|---|---|---|---|
> | `bootstrap.c:87` | `pico8_dg` | R `p_` (vérité) | élément de `hv` (`pico8_ke_t *` après D.11a) : toute classe inscrite | `cls->get_p_ != NULL && cls->get_p_(qc)` ; `pico8_dg_truthy` (l. 19) perd son seul appelant : supprimée | c |
> | `entity.c:237` | `pico8_lt` | R `nu` | toute entité inscrite | `tags = e->cls->get_nu != NULL ? e->cls->get_nu(e) : NULL` ; NULL n'inscrit rien, comme `all(nil)` | c |
> | `entity.c:266` | `pico8_he` | R `nu` | idem | idem | c |
> | `entity.c:300` | `pico8_dc` | R `fa` | élément de `cv("kj")`/`cv("ev")` | `cls->get_fa == NULL \|\| !cls->get_fa(qc, &fa)` → `continue` | c |
> | `entity.c:329` | `pico8_dc` | R `ra` | idem | `ra = cls->get_ra != NULL ? cls->get_ra(qc) : NULL` | c |
> | `entity_geom.c:36`, `:37`, `:42` | `pico8_gk` | R `fa`, `ra`, `kq` | `e` : `fv` (`help.c:36`), `jo` (`planet.c:47`), `ie` (`research_icon.c:275`), `js` (`ui_button.c:49`), ou NULL | accesseurs `get_fa` (absent → 0), `get_ra` (NULL → zéro), `get_kq` (NULL → 0) ; `e == NULL` garde les replis | h, h, c |
> | `entity_geom.c:49` | `pico8_gd` | R `ra` | `e` : planète (`planet.c:844`, `planet_display.c:431`), émetteur (`particle.c:107`), extrémité de voie (`slipway_display.c:116` : planète ou point de souris) | `get_ra` (NULL → zéro) | h |
> | `entity_geom.c:64` | `pico8_na` | R `ra` (écrit au travers) | `self` : `qh:ly` (`pico8_qh_ly_dispatch`, lignée `qh`) et `nl:ly` : famille `jo` seule | **cast vérifié** (exception, voir « C ») : `assert(pico8_jo_is_family(self) && "…")`, `((pico8_jo_t *)self)->ra` | c |
> | `entity_geom.c:73`, `:82` | `pico8_ma` | R/W `ma` | `e` : `jo` (`planet_display.c:437`) ou `qb` (`scroll.c:195`), asserté par D.11a (H-D11a-7) | `pico8_fixed_t *ma = e->cls->get_ma(e)` (adresse du membre), lecture puis `*ma = clamped` | h, c |
> | `entity_geom.c:104` ✱ | `pico8_ij` | R `p` | élément de `cg("jo")` (`galaxy.c:70`), `cg("np")` (`probe.c:220`) ou `mz.ox` (`wormhole.c:112` : planètes et `g_`/`fv`) | `get_p` (NULL → zéro) | h |
> | `entity_geom.c:19`, `:27` | `pico8_entity_geom_field_vec`/`_fixed` | définitions | — | supprimées | d, d |
> | `planet_display.c:482` | `pico8_jo_kj` | R `dw` | `self->fo` : ce qui accroche `fo` (`jo_is_fo_holder_class` : classe dont `hq` est `bv` — `g_`, `fv`, lignée `qh` — ou `jc`) | `get_dw` (absent → 0) | c |
> | `slipway.c:263` | `pico8_jc_hq` | W `fo` | `self->et` = `jc:ce(self)` : planète, `jc`, ou la vue `jc_too_far` | `self->et->cls->set_fo(self->et, &self->header)` (membre NULL → assert gardé : « un écrivain générique nomme un champ sans membre ») | c |
> | `ui.c:304` | `pico8_ui_ev` | R `ql` | `self->o_` : entité survolée (`cv("hd")`) | `get_ql` (absent → `self->ql`) | h |
> | `ui.c:346` | `pico8_ui_gw` | R `fa` | élément de `cv("hd")` survolé | `get_fa` (absent → non posé en calque) | c |
> | `ui.c:359`, `:363` | `pico8_ui_gw` | W `m_` | `self->gj`, `self->o_` | `cls->set_m_(e, 0)`, `cls->set_m_(e, 1)` (membre NULL → même assert) | c, c |
> | `ui.c:367` | `pico8_ui_gw` | R `fg` | `self->o_` | `get_fg` (absent → 0) | h |
> | `ui.c:56` | `pico8_ui_field_raw` | définition | — | supprimée (ses appelants `je.ra` sont partis à D.11a, H-D11a-6) | d |
> | `wormhole.c:70` ✱ | `pico8_mz_pq_ox_predicate` | R `fx` (présence, puis vérité) | élément de `hv` | `cls->get_fx != NULL && cls->get_fx(p)` | c |
> | `wormhole_display.c:41` ✱ | `pico8_mzd_ra_of_ij` | R `ra` | `mz.ij` ∈ `mz.ox` | `get_ra` (NULL → zéro) | h |
> | `wormhole_display.c:21` | `pico8_mzd_field_vec` | définition | — | supprimée | d |
>
> Qui répond à chaque nom aujourd'hui (lecteurs et écrivains lus, délégation à la parente comprise) — c'est l'ensemble exact des descripteurs qui reçoivent l'accesseur (règle « NULL = absent », voir « C ») :
>
> | Accesseur | Descripteurs (fichier) | Nombre |
> |---|---|---|
> | `get_nu` | les 10 de l'arbre `jo` ; `g_`, `fv` (`probe.c`, `help.c`) ; `jc` ; `mz` ; `mh` | 15 |
> | `get_fa` | les 10 de `jo` ; `g_`, `fv` ; `bg` ; `mh`, `iv` ; `ie` ; `pj` ; `jc` ; `ui` ; `js`, `fq` ; `mz` | 22 |
> | `get_ra` | les 10 de `jo` ; `g_`, `fv` ; `lq` ; `nf` ; `iv` ; `ie` ; `qb` ; `jc_mouse_point` ; `js` ; `mz` | 21 |
> | `get_p` | les 10 de `jo` ; `g_`, `fv` ; `qb` ; `jc_mouse_point` ; `mz` | 15 |
> | `get_kq` | les 10 de `jo` ; `fv` ; `ie` ; `js` | 13 |
> | `get_ma` | les 10 de `jo` ; `qb` | 11 |
> | `get_dw` | les 8 de la lignée `qh` ; `g_`, `fv` ; `jc` ; `mz` | 12 |
> | `get_ql` | `fv` ; `ui` | 2 |
> | `get_fg` | `fv` ; `js` | 2 |
> | `get_fx` | les 10 de `jo` ; `g_`, `fv` | 12 |
> | `get_p_` | les 10 de `jo` ; `g_`, `fv` ; `mh`, `iv` ; `jc` ; `fq` ; `mz` | 17 |
> | `set_m_` | les 10 de `jo` ; `fv` ; `bg` ; `ie` ; `js`, `fq` | 15 |
> | `set_fo` | les 10 de `jo` ; `jc` ; `jc_too_far` ; `bv_et` | 13 |
>
> Les autres descripteurs (`hy`, `hp`, `trade`, `jb`, `pj_cr`, `ha_pz`, `mz_jj`, `nr_ldown_empty`, `ie_ke_view`, et les remplaçants `mo`/`je`/`fm`) ne répondent à aucun de ces noms : membres NULL. Les « 10 de `jo` » sont `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx` ; la « lignée `qh` » les 8 sans `jo` ni `ga`.
>
>
> **Ajouté par D.11a — `jc.mt` (3 lignes comptées, `slipway.c`)** : `pico8_jc_entity_value` (définition l. 131, ligne comptée l. 133), `jc_take_fixed` (définition l. 170, comptée), l'appel `jc_take_fixed(mt_val, &self->mt, &self->has_mt)` (l. 257), avec `qu_mt = pico8_jc_entity_value(self->qu, "mt")` et le repli sur `re` (l. 255-256). Qui répond à `mt` : `pico8_fr_field` (défaut propre de `fr`) seul, parmi les extrémités. **Contrainte de lien** (note de D.11a) : `slipway.c` est dans `DATA_ENGINE_SRCS`, `planet_family_trade.c` non ; nommer `pico8_fr_struct_class()` depuis `slipway.c` casse le lien de `test_pico8_bg_kj`. Un accesseur `get_mt` dans la table de classe (posé par le descripteur de `fr`, lu par `cls->get_mt` dans `jc:hq`) ne nomme aucun symbole de `planet_family_trade.c` depuis `slipway.c` : c'est la forme Q6, et elle lève la contrainte. À trancher par D.11e.
>
> ### C.2 Décisions préparées
>
> (Titre d'origine : « C — accesseurs transversaux (G, b3) ». Les renvois à `b3` valent « premier chunk de D.11e ».)
>
> ```c
> /* pico8_struct.h, dans struct pico8_struct_class, après `parent` */
>     /* A NULL member: the class has no such field, as its `field` reader
>      * would answer 0 (or its `set_field` refuse the name). A reader returns
>      * NULL, or 0 leaving *out untouched, when the field is nil; `get_fx`
>      * and `get_p_` return its Lua truth. `get_ma` returns the member's own
>      * address. `set_fo` asserts what may hang `fo` on the class. */
>     const pico8_str_seq_t *(*get_nu)(const void *self);
>     int (*get_fa)(const void *self, pico8_fixed_t *out);
>     const pico8_vec_t *(*get_ra)(const void *self);
>     const pico8_vec_t *(*get_p)(const void *self);
>     const pico8_qz_box_t *(*get_kq)(const void *self);
>     pico8_fixed_t *(*get_ma)(void *self);
>     int (*get_dw)(const void *self, pico8_fixed_t *out);
>     int (*get_ql)(const void *self, pico8_fixed_t *out);
>     int (*get_fg)(const void *self, pico8_fixed_t *out);
>     int (*get_fx)(const void *self);
>     int (*get_p_)(const void *self);
>     void (*set_m_)(void *self, int m_);
>     void (*set_fo)(void *self, pico8_struct_header_t *fo);
> ```
>
> - **Un membre par champ, nommé** (Q6) ; aucun tableau, `switch` ni boucle qui passe d'un nom à un membre (ce serait un registre). Les signatures suivent l'usage des consommateurs : pointeur pour les pointeurs, `int` + sortie pour les fixes qui peuvent être nil, vérité pour les booléens lus comme vérité, adresse pour `ma` (lu puis écrit par `pico8_ma`).
> - **Règle « NULL = absent »** (G) : un descripteur reçoit l'accesseur d'un nom **si et seulement si** son `field` (ou son `set_field`, pour `m_`/`fo`) répond à ce nom aujourd'hui, directement ou par délégation à sa parente. Table « Qui répond à chaque nom » : 13 noms, 170 lignes de descripteurs. Chaque accesseur rend exactement ce que rend la branche de son lecteur : même condition de présence (`has_X`, pointeur NULL), même valeur. Un écrivain fait ce que fait la branche de son `set_field` (présence posée, assert de genre gardé pour `fo`). Rejeté : ne poser l'accesseur que sur les classes « atteignables » par chaque consommateur (exige une preuve d'atteignabilité par consommateur ; `hv` et `cv` atteignent de toute façon presque toutes les classes ; un oubli lirait silencieusement « absent »).
> - **Héritage** : les descripteurs de l'arbre `jo` nomment les accesseurs de `jo` (et `pico8_qh_get_dw` pour la lignée `qh`), `fv` ceux de `g_` pour les noms qu'il délègue, exactement comme ils nomment déjà `pico8_jo_kj_dispatch` (précédent D.10a). Rejeté : une résolution qui remonte `parent` quand le membre est NULL (machinerie nouvelle, et « NULL = absent » deviendrait « NULL sur toute la chaîne ») ; monter ces champs dans `ke` (option B de Q6, refusée par l'humain).
> - **Noms** : `get_X`/`set_X` dans la table, `pico8_X_get_Y`/`pico8_X_set_Y` (ou statiques `X_get_Y`) pour les fonctions ; aucun ne correspond au motif `par-nom` (`field`, `_take_`, `field_truthy`). Ce ne sont pas des renommages : ce sont des accès typés nouveaux, et les lecteurs par nom restent (D.11e).
> - **Consommateurs** : table C. Chaque site teste le membre (`cls->get_X != NULL`) avant l'appel, sauf `set_m_`/`set_fo`, dont l'absence est un arrêt : `assert(cls->set_m_ != NULL && "…")`, message qui nomme le contrat (« un écrivain générique nomme un champ que cette struct n'a pas — stop, n'ajoute pas de membre »), comme `pico8_entity_set_field` aujourd'hui.
> - **Exception `pico8_na`** (G) : il écrit **au travers** de `ra` (`ra_vec->y += …`) ; `get_ra` rend `const pico8_vec_t *` parce que plusieurs classes pointent une donnée générée `const` (`qb`, `iv`, `g_`, `ie`). Ses deux seuls appelants sont `qh:ly` (`pico8_qh_ly_dispatch`) et `nl:ly` : famille `jo`. Retenu : `assert(self == NULL || pico8_jo_is_family(self))` puis `((pico8_jo_t *)self)->ra`, le cast vérifié de D.11a. Rejetés : un `get_ra` non `const` (retirerait le `const` d'une donnée générée par cast) ; un second accesseur mutable (un membre pour un seul site). Écart à Q6 déclaré au message et dans la note.
> - **`pico8_ma`** : `get_ma` rend l'adresse de `jo.ma` ou `qb.ma` ; D.11a y a déjà l'assert de famille (H-D11a-7), qui reste.
> - **Rejetés** : lire ces champs par un cast au site (receveurs ouverts : `hv`, `cv`, `ui.o_`, `mz.ox`, `jo.fo`) ; un seul accesseur générique à nom (c'est `field`).
>
> ### C.3 TU préparés
>
> Classes locales de TU qui répondent par nom à un nom de C, et doivent gagner l'accesseur dans le commit qui fait lire ce nom par accesseur (sans quoi le TU lirait « absent » par hasard) :
>
> - **`test_pico8_iv`** : `g_fake_toast_class` gagne `.get_p_`, qui rend la vérité de son `p_` ; `test_dg_drives_a_ke_struct` inchangé (le toast sort de `hv` au troisième `dg`). Son `.field` reste (D.11d).
> - **`test_pico8_mo`** : `g_ra_only_class` gagne `.get_ra` ; `GD_NEAR`/`GD_FAR` inchangées. Son `.field` reste.
> - **`test_pico8_ui`** : `g_giver_class` gagne `.get_fa` (15) et `.set_m_` (sans effet, comme son `set_field`) ; ses constantes inchangées. Plus un cas nouveau, **« accesseurs et lecteurs d'accord »**, sans constante, dans son propre état (un fils ou un `start_game` frais, pour ne rien plier dans les constantes existantes) : sur chaque entité de `hv` d'une partie démarrée avec galaxie, et sur une instance de chaque classe de la table « Qui répond » absente de `hv` quand le TU sait la construire, pour chacun des 11 lecteurs, **une comparaison écrite par nom** (pas de table de noms) : membre NULL ⇔ `pico8_entity_get_field` rend 0 ; sinon même présence et même valeur (pointeur identique pour les REF). Les classes que le TU ne peut pas construire (vues statiques de `slipway.c`, `bv_et`) sont couvertes par leurs TU existants (`test_pico8_jc`, `test_pico8_g_`) ou justifiées au message. Ses lignes `pico8_entity_get_field` gagnent `par-nom`, déclarées ; D.11d/D.11e le retirent avec les lecteurs.
>
> `test_pico8_planet_kj_dispatch` copie la table de `jo` (`g_spied_jo = *pico8_jo_struct_class()`) : la copie emporte les membres nouveaux. Classes locales à `d4c4f4c` : `test_pico8_iv.c:227`, `test_pico8_mo.c:107`, `test_pico8_ui.c:839` (`giver_t` porte désormais un `pico8_ke_t`, l. 800-801).
>
> ### C.4 Mutations préparées
>
>   - **M11** `get_nu` de `jo` rend NULL → presque tous les TU de planète (`cg("jo")` vide) ;
>   - **M12** `get_fa` de `js` absent → `test_pico8_ui`/`test_pico8_js` (calques de `ui.ic`, `dc`) ;
>   - **M13** `get_p_` de `fq` rend 0 → `test_pico8_fq` (le bouton ne sort plus de `hv`) ;
>   - **M14** `set_m_` de `jo` retiré de son descripteur → `test_pico8_ui` (survol d'une planète : l'assert s'arrête) ;
>   - **M15** `pico8_ma` n'écrit plus `*ma` → `test_pico8_qb`/`test_pico8_jo` (décroissance de `ma`) ;
>   - **M16** `get_fx` de `g_` retiré → `test_pico8_mz` (`mz.ox` sans sonde) ;
>   - **M17** `set_fo` de `jc_too_far` retiré → `test_pico8_jc` (voie trop longue : l'assert s'arrête) ;
>   - **M18** `get_ra` de `jc_mouse_point` retiré → `test_pico8_jc` (`gd` du point de souris) ;
>   - **M19** un accesseur rend « présent » là où son lecteur rend 0 (par exemple `get_dw` de `jc` sans condition) → l'oracle de `test_pico8_ui`.
>
> ### C.5 Coût mesuré
>
> 13 accesseurs (plus `get_mt` si D.11e le retient), 28 descripteurs dans 16 fichiers, ~170 lignes d'initialiseurs et ~60 petites fonctions, pour 18 lignes comptées (21 avec `mt`) ; 25 fichiers de `src/` et 3 TU à ce seul chunk, plus l'oracle. Précédent de forme : la table de méthodes typée de D.10a, dont chaque descripteur nomme explicitement les fonctions de sa parente.
>

## Annexe D — inventaire ligne à ligne (`src/` à `fdb294b`, identique à `579a748` ; 292 lignes `par-nom`, 352 occurrences)

Colonnes : numéro de ligne, occurrences du motif sur la ligne, nature (voir « Inventaire »), sous-étape qui la retire (ou, pour les 9 initialiseurs de sortes, qui en retire l'entrée), texte (tronqué à 110 caractères).

#### `src/pico8_bg.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 44 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_bg_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 76 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 79 | 1 | aide `*_take_*` (déf.) | e3 | `static void bg_take_bool(pico8_value_t value, int *out, int *has) {` |
| 93 | 1 | écrivain `set_field` (déf.) | e3 | `static int pico8_bg_set_field(void *self, const char *name, pico8_value_t value) {` |
| 97 | 1 | appel d'aide `*_take_*` | e3 | `bg_take_bool(value, &bg->m_, &bg->has_m_);` |
| 109 | 2 | emplacement de descripteur | e3 | `.field = pico8_bg_field,` |
| 110 | 2 | emplacement de descripteur | e3 | `.set_field = pico8_bg_set_field,` |

#### `src/pico8_bootstrap.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 87 | 1 | site C | e1 | `if (pico8_entity_get_field(qc, "p_", &p_val) && pico8_dg_truthy(p_val)) {` |

#### `src/pico8_economy.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 21 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_hy_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 88 | 2 | emplacement de descripteur | e3 | `.field = pico8_hy_field,` |

#### `src/pico8_entity.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 38 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_mo_placeholder_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 46 | 2 | emplacement de descripteur | e3 | `.field = pico8_mo_placeholder_field,` |
| 198 | 1 | lecteur `field` (déf.) | e3 | `int pico8_entity_get_field(const void *e, const char *name, pico8_value_t *out_value) {` |
| 204 | 1 | `pico8_entity_get/set_field` (corps) | e3 | `assert(header->cls->field != NULL && "entity: a struct a generic consumer reaches needs a field reader");` |
| 205 | 1 | `pico8_entity_get/set_field` (corps) | e3 | `return header->cls->field(e, name, out_value);` |
| 208 | 1 | `pico8_entity_get/set_field` (corps) | e3 | `void pico8_entity_set_field(void *e, const char *name, pico8_value_t value) {` |
| 215 | 1 | `pico8_entity_get/set_field` (corps) | e3 | `assert(header->cls->set_field != NULL && "entity: a struct a generic writer reaches needs a field writer");` |
| 216 | 1 | `pico8_entity_get/set_field` (corps) | e3 | `written = header->cls->set_field(e, name, value);` |
| 237 | 1 | site C | e1 | `if (pico8_entity_get_field(e, "nu", &nu_val) && nu_val.kind == PICO8_VALUE_REF && nu_val.as.ref != NULL) {` |
| 266 | 1 | site C | e1 | `if (pico8_entity_get_field(e, "nu", &nu_val) && nu_val.kind == PICO8_VALUE_REF && nu_val.as.ref != NULL) {` |
| 300 | 1 | site C | e1 | `if (!pico8_entity_get_field(qc, "fa", &fa_val) \|\| fa_val.kind != PICO8_VALUE_FIXED) {` |
| 329 | 1 | site C | e1 | `if (pico8_entity_get_field(qc, "ra", &ra_val) && ra_val.kind == PICO8_VALUE_REF) {` |

#### `src/pico8_entity.h`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 49 | 1 | en-tête | e3 | `int pico8_entity_get_field(const void *e, const char *name, pico8_value_t *out_value);` |
| 52 | 1 | en-tête | e3 | `* pico8_entity_get_field() does, that `e` is an entity struct, and that it` |
| 54 | 1 | en-tête | e3 | `void pico8_entity_set_field(void *e, const char *name, pico8_value_t value);` |

#### `src/pico8_entity_geom.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 19 | 1 | site C | e1 | `if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {` |
| 27 | 1 | site C | e1 | `if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {` |
| 42 | 1 | site C | e1 | `if (e != NULL && pico8_entity_get_field(e, "kq", &kq) && kq.kind == PICO8_VALUE_REF && kq.as.ref != NULL) {` |
| 64 | 1 | site C | e1 | `if (self != NULL && pico8_entity_get_field(self, "ra", &ra) && ra.kind == PICO8_VALUE_REF && ra.as.ref != N…` |
| 82 | 1 | site C | e1 | `pico8_entity_set_field(e, "ma", pico8_value_fixed(clamped));` |

#### `src/pico8_help.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 102 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_fv_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 160 | 1 | lecteur (délégation) | e2 | `return pico8_g__struct_class()->field(self, name, out_value);` |
| 163 | 1 | aide `*_take_*` (déf.) | e2 | `static void help_take_bool(pico8_value_t value, int *out, int *has) {` |
| 176 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_fv_set_field(void *self, const char *name, pico8_value_t value) {` |
| 179 | 1 | appel d'aide `*_take_*` | e2 | `help_take_bool(value, &fv->m_, &fv->has_m_);` |
| 182 | 1 | écrivain (délégation) | e2 | `return pico8_g__struct_class()->set_field(self, name, value);` |
| 192 | 2 | emplacement de descripteur | e2 | `.field = pico8_fv_field,` |
| 193 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_fv_set_field,` |

#### `src/pico8_ke.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 44 | 1 | lecteur `field` (déf.) | e3 | `int pico8_ke_field(const void *self, const char *name, pico8_value_t *out_value) {` |

#### `src/pico8_ke.h`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 36 | 1 | en-tête | e3 | `int pico8_ke_field(const void *self, const char *name, pico8_value_t *out_value);` |

#### `src/pico8_layout.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 322 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_lq_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 370 | 2 | emplacement de descripteur | e3 | `.field = pico8_lq_field,` |

#### `src/pico8_menu.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 49 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_ha_pz_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 77 | 2 | emplacement de descripteur | e3 | `.field = pico8_ha_pz_field,` |

#### `src/pico8_mouse_reader.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 27 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_nf_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 42 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 53 | 2 | emplacement de descripteur | e3 | `.field = pico8_nf_field,` |

#### `src/pico8_msgbox.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 87 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_mh_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 105 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 112 | 2 | emplacement de descripteur | e3 | `.field = pico8_mh_field,` |
| 239 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_iv_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 271 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 277 | 2 | emplacement de descripteur | e3 | `.field = pico8_iv_field,` |

#### `src/pico8_planet.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 90 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_kw(pico8_value_t value, pico8_kw_t **out) {` |
| 101 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_nk(pico8_value_t value, const pico8_df_entry_t **out) {` |
| 113 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_hp(pico8_value_t value, pico8_hp_t **out) {` |
| 126 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_string(pico8_value_t value, const char **out) {` |
| 137 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_list(pico8_value_t value, pico8_list_t **out) {` |
| 149 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_bool(pico8_value_t value, int *out, int *has) {` |
| 162 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_fixed(pico8_value_t value, pico8_fixed_t *out, int *has) {` |
| 175 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_lq(pico8_value_t value, pico8_lq_t **out) {` |
| 188 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_ma(pico8_value_t value, pico8_fixed_t *out) {` |
| 198 | 1 | aide `*_take_*` (déf.) | e2 | `static void jo_take_holder(pico8_value_t value, pico8_struct_header_t **out) {` |
| 242 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_jo_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 383 | 1 | lecteur (délégation) | e2 | `return pico8_ke_field(self, name, out_value);` |
| 389 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_jo_set_field(void *self, const char *name, pico8_value_t value) {` |
| 393 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_ma(value, &jo->ma);` |
| 397 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_kw(value, &jo->kw);` |
| 401 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_list(value, &jo->oe);` |
| 405 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_hp(value, &jo->hn);` |
| 409 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_string(value, &jo->ep);` |
| 413 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_list(value, &jo->jy);` |
| 417 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_nk(value, &jo->nk);` |
| 421 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_bool(value, &jo->fx, &jo->has_fx);` |
| 425 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_lq(value, &jo->lq);` |
| 429 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_holder(value, &jo->fo);` |
| 433 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_fixed(value, &jo->jh, &jo->has_jh);` |
| 437 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_bool(value, &jo->p_, &jo->has_p_);` |
| 441 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_fixed(value, &jo->kr, &jo->has_kr);` |
| 445 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_list(value, &jo->cc);` |
| 449 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_list(value, &jo->by);` |
| 453 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_fixed(value, &jo->mi, &jo->has_mi);` |
| 457 | 1 | appel d'aide `*_take_*` | e2 | `jo_take_bool(value, &jo->m_, &jo->has_m_);` |
| 473 | 2 | emplacement de descripteur | e2 | `.field = pico8_jo_field,` |
| 474 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_jo_set_field,` |
| 584 | 1 | `kind_field` (déf./décl.) | e2 | `int pico8_jo_kind_field(const char *name, pico8_value_t *out_value) {` |

#### `src/pico8_planet.h`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 81 | 1 | `kind_field` (déf./décl.) | e2 | `int pico8_jo_kind_field(const char *name, pico8_value_t *out_value);` |

#### `src/pico8_planet_display.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 484 | 1 | site C | e1 | `if (pico8_entity_get_field(self->fo, "dw", &dw_val) &&` |

#### `src/pico8_planet_family.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 70 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_qh_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 118 | 1 | lecteur (délégation) | e2 | `return pico8_jo_struct_class()->field(self, name, out_value);` |
| 121 | 1 | aide `*_take_*` (déf.) | e2 | `static void family_take_vec(pico8_value_t value, pico8_vec_t **out) {` |
| 140 | 1 | aide `*_take_*` (déf.) | e2 | `static void family_take_target(pico8_value_t value, pico8_struct_header_t **out, int *has) {` |
| 157 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_qh_set_field(void *self, const char *name, pico8_value_t value) {` |
| 161 | 1 | appel d'aide `*_take_*` | e2 | `family_take_target(value, &qh->probe->et, &qh->probe->has_et);` |
| 165 | 1 | appel d'aide `*_take_*` | e2 | `family_take_vec(value, &qh->jo.ra);` |
| 169 | 1 | appel d'aide `*_take_*` | e2 | `family_take_vec(value, &qh->jo.p);` |
| 172 | 1 | écrivain (délégation) | e2 | `return pico8_jo_struct_class()->set_field(self, name, value);` |
| 192 | 2 | emplacement de descripteur | e2 | `.field = pico8_qh_field,` |
| 193 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_qh_set_field,` |
| 246 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_nl_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 264 | 1 | lecteur (délégation) | e2 | `return pico8_qh_struct_class()->field(self, name, out_value);` |
| 267 | 1 | aide `*_take_*` (déf.) | e2 | `static void family_take_fixed(pico8_value_t value, pico8_fixed_t *out) {` |
| 275 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_nl_set_field(void *self, const char *name, pico8_value_t value) {` |
| 277 | 1 | appel d'aide `*_take_*` | e2 | `family_take_fixed(value, &((pico8_nl_t *)self)->qw);` |
| 280 | 1 | écrivain (délégation) | e2 | `return pico8_qh_struct_class()->set_field(self, name, value);` |
| 317 | 2 | emplacement de descripteur | e2 | `.field = pico8_nl_field,` |
| 318 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_nl_set_field,` |
| 372 | 2 | emplacement de descripteur | e2 | `.field = pico8_qh_field,` |
| 373 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_qh_set_field,` |
| 436 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_hj_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 492 | 1 | lecteur (délégation) | e2 | `return pico8_qh_struct_class()->field(self, name, out_value);` |
| 495 | 1 | aide `*_take_*` (déf.) | e2 | `static void family_take_list(pico8_value_t value, pico8_list_t **out) {` |
| 508 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_hj_set_field(void *self, const char *name, pico8_value_t value) {` |
| 510 | 1 | appel d'aide `*_take_*` | e2 | `family_take_list(value, &((pico8_hj_t *)self)->emitter->qy);` |
| 513 | 1 | écrivain (délégation) | e2 | `return pico8_qh_struct_class()->set_field(self, name, value);` |
| 534 | 2 | emplacement de descripteur | e2 | `.field = pico8_hj_field,` |
| 535 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_hj_set_field,` |
| 613 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_pm_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 630 | 1 | lecteur (délégation) | e2 | `return pico8_hj_field(self, name, out_value);` |
| 652 | 2 | emplacement de descripteur | e2 | `.field = pico8_pm_field,` |
| 653 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_hj_set_field,` |
| 704 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_ga_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 752 | 1 | lecteur (délégation) | e2 | `return pico8_jo_struct_class()->field(self, name, out_value);` |
| 757 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_ga_set_field(void *self, const char *name, pico8_value_t value) {` |
| 759 | 1 | appel d'aide `*_take_*` | e2 | `family_take_list(value, &((pico8_ga_t *)self)->emitter->qy);` |
| 762 | 1 | écrivain (délégation) | e2 | `return pico8_jo_struct_class()->set_field(self, name, value);` |
| 886 | 2 | emplacement de descripteur | e2 | `.field = pico8_ga_field,` |
| 887 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_ga_set_field,` |
| 968 | 1 | `kind_field` (déf./décl.) | e2 | `static int pico8_qh_kind_field(const char *name, pico8_value_t *out_value) {` |
| 984 | 1 | `kind_field` (délégation) | e2 | `return pico8_jo_kind_field(name, out_value);` |
| 987 | 1 | emplacement de sorte | e2 | `static const pico8_entity_kind_t g_qh_kind = {pico8_qh_struct_new, pico8_qh_kind_field, &pico8_qh_kind_rec};` |
| 1005 | 1 | `kind_field` (déf./décl.) | e2 | `static int pico8_nl_kind_field(const char *name, pico8_value_t *out_value) {` |
| 1021 | 1 | `kind_field` (délégation) | e2 | `return pico8_qh_kind_field(name, out_value);` |
| 1024 | 1 | emplacement de sorte | e2 | `static const pico8_entity_kind_t g_nl_kind = {pico8_nl_struct_new, pico8_nl_kind_field, &pico8_nl_kind_rec};` |
| 1060 | 1 | `kind_field` (déf./décl.) | e2 | `static int pico8_os_kind_field(const char *name, pico8_value_t *out_value) {` |
| 1072 | 1 | `kind_field` (délégation) | e2 | `return pico8_qh_kind_field(name, out_value);` |
| 1075 | 1 | emplacement de sorte | e2 | `static const pico8_entity_kind_t g_os_kind = {pico8_os_struct_new, pico8_os_kind_field, &pico8_os_kind_rec};` |
| 1093 | 1 | `kind_field` (déf./décl.) | e2 | `static int pico8_hj_kind_field(const char *name, pico8_value_t *out_value) {` |
| 1109 | 1 | `kind_field` (délégation) | e2 | `return pico8_qh_kind_field(name, out_value);` |
| 1112 | 1 | emplacement de sorte | e2 | `static const pico8_entity_kind_t g_hj_kind = {pico8_hj_struct_new, pico8_hj_kind_field, &pico8_hj_kind_rec};` |
| 1130 | 1 | `kind_field` (déf./décl.) | e2 | `static int pico8_pm_kind_field(const char *name, pico8_value_t *out_value) {` |
| 1142 | 1 | `kind_field` (délégation) | e2 | `return pico8_hj_kind_field(name, out_value);` |
| 1145 | 1 | emplacement de sorte | e2 | `static const pico8_entity_kind_t g_pm_kind = {pico8_pm_struct_new, pico8_pm_kind_field, &pico8_pm_kind_rec};` |
| 1163 | 1 | `kind_field` (déf./décl.) | e2 | `static int pico8_ga_kind_field(const char *name, pico8_value_t *out_value) {` |
| 1167 | 1 | `kind_field` (délégation) | e2 | `return pico8_jo_kind_field(name, out_value);` |
| 1182 | 1 | emplacement de sorte | e2 | `static const pico8_entity_kind_t g_ga_kind = {pico8_ga_kind_new, pico8_ga_kind_field, &pico8_ga_kind_rec};` |

#### `src/pico8_planet_family_trade.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 50 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_nr_ldown_empty_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 58 | 2 | emplacement de descripteur | e2 | `.field = pico8_nr_ldown_empty_field,` |
| 176 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_nr_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 193 | 1 | lecteur (délégation) | e2 | `return pico8_qh_struct_class()->field(self, name, out_value);` |
| 196 | 1 | aide `*_take_*` (déf.) | e2 | `static void family_trade_take_fixed(pico8_value_t value, pico8_fixed_t *out) {` |
| 203 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_nr_set_field(void *self, const char *name, pico8_value_t value) {` |
| 205 | 1 | appel d'aide `*_take_*` | e2 | `family_trade_take_fixed(value, &((pico8_nr_t *)self)->qh.s);` |
| 208 | 1 | écrivain (délégation) | e2 | `return pico8_qh_struct_class()->set_field(self, name, value);` |
| 228 | 2 | emplacement de descripteur | e2 | `.field = pico8_nr_field,` |
| 229 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_nr_set_field,` |
| 273 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_fr_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 278 | 1 | lecteur (délégation) | e2 | `return pico8_nr_field(self, name, out_value);` |
| 298 | 2 | emplacement de descripteur | e2 | `.field = pico8_fr_field,` |
| 299 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_nr_set_field,` |
| 379 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_mx_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 388 | 1 | lecteur (délégation) | e2 | `return pico8_hj_struct_class()->field(self, name, out_value);` |
| 391 | 1 | aide `*_take_*` (déf.) | e2 | `static void family_trade_take_seq(pico8_value_t value, const pico8_int_seq_t **out) {` |
| 404 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_mx_set_field(void *self, const char *name, pico8_value_t value) {` |
| 406 | 1 | appel d'aide `*_take_*` | e2 | `family_trade_take_seq(value, &((pico8_mx_t *)self)->ht);` |
| 409 | 1 | écrivain (délégation) | e2 | `return pico8_hj_struct_class()->set_field(self, name, value);` |
| 430 | 2 | emplacement de descripteur | e2 | `.field = pico8_mx_field,` |
| 431 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_mx_set_field,` |
| 494 | 1 | `kind_field` (déf./décl.) | e2 | `static int pico8_nr_kind_field(const char *name, pico8_value_t *out_value) {` |
| 510 | 1 | `kind_field` (délégation) | e2 | `return pico8_qh_kind()->field(name, out_value);` |
| 513 | 1 | emplacement de sorte | e2 | `static const pico8_entity_kind_t g_nr_kind = {pico8_nr_struct_new, pico8_nr_kind_field, &pico8_nr_kind_rec};` |
| 531 | 1 | `kind_field` (déf./décl.) | e2 | `static int pico8_fr_kind_field(const char *name, pico8_value_t *out_value) {` |
| 547 | 1 | `kind_field` (délégation) | e2 | `return pico8_nr_kind_field(name, out_value);` |
| 550 | 1 | emplacement de sorte | e2 | `static const pico8_entity_kind_t g_fr_kind = {pico8_fr_struct_new, pico8_fr_kind_field, &pico8_fr_kind_rec};` |
| 568 | 1 | `kind_field` (déf./décl.) | e2 | `static int pico8_mx_kind_field(const char *name, pico8_value_t *out_value) {` |
| 584 | 1 | `kind_field` (délégation) | e2 | `return pico8_hj_kind()->field(name, out_value);` |
| 587 | 1 | emplacement de sorte | e2 | `static const pico8_entity_kind_t g_mx_kind = {pico8_mx_struct_new, pico8_mx_kind_field, &pico8_mx_kind_rec};` |

#### `src/pico8_probe.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 27 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_fm_placeholder_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 48 | 2 | emplacement de descripteur | e2 | `.field = pico8_fm_placeholder_field,` |
| 59 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_bv_et_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 76 | 1 | aide `*_take_*` (déf.) | e2 | `static void bv_et_take_holder(pico8_value_t value, pico8_struct_header_t **out) {` |
| 88 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_bv_et_set_field(void *self, const char *name, pico8_value_t value) {` |
| 92 | 1 | appel d'aide `*_take_*` | e2 | `bv_et_take_holder(value, &et->fo);` |
| 99 | 2 | emplacement de descripteur | e2 | `.field = pico8_bv_et_field,` |
| 100 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_bv_et_set_field,` |
| 505 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_g__field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 591 | 1 | lecteur (délégation) | e2 | `return pico8_ke_field(self, name, out_value);` |
| 594 | 1 | aide `*_take_*` (déf.) | e2 | `static void g__take_list(pico8_value_t value, pico8_list_t **out) {` |
| 605 | 1 | aide `*_take_*` (déf.) | e2 | `static void g__take_vec(pico8_value_t value, pico8_vec_t **out) {` |
| 617 | 1 | aide `*_take_*` (déf.) | e2 | `static void g__take_target(pico8_value_t value, pico8_struct_header_t **out, int *has) {` |
| 632 | 1 | aide `*_take_*` (déf.) | e2 | `static void g__take_bool(pico8_value_t value, int *out, int *has) {` |
| 645 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_g__set_field(void *self, const char *name, pico8_value_t value) {` |
| 649 | 1 | appel d'aide `*_take_*` | e2 | `g__take_list(value, &g_->emitter->qy);` |
| 655 | 1 | appel d'aide `*_take_*` | e2 | `g__take_vec(value, &ra);` |
| 660 | 1 | appel d'aide `*_take_*` | e2 | `g__take_vec(value, &g_->p);` |
| 664 | 1 | appel d'aide `*_take_*` | e2 | `g__take_target(value, &g_->probe->et, &g_->probe->has_et);` |
| 668 | 1 | appel d'aide `*_take_*` | e2 | `g__take_bool(value, &g_->p_, &g_->has_p_);` |
| 680 | 2 | emplacement de descripteur | e2 | `.field = pico8_g__field,` |
| 681 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_g__set_field,` |

#### `src/pico8_research.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 76 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_hp_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 203 | 2 | emplacement de descripteur | e3 | `.field = pico8_hp_field,` |

#### `src/pico8_research_icon.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 47 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_ie_ke_view_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 82 | 2 | emplacement de descripteur | e3 | `.field = pico8_ie_ke_view_field,` |
| 241 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_ie_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 295 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 304 | 1 | aide `*_take_*` (déf.) | e3 | `static void ie_take_bool(pico8_value_t value, int *out, int *has) {` |
| 318 | 1 | écrivain `set_field` (déf.) | e3 | `static int pico8_ie_set_field(void *self, const char *name, pico8_value_t value) {` |
| 322 | 1 | appel d'aide `*_take_*` | e3 | `ie_take_bool(value, &ie->m_, &ie->has_m_);` |
| 333 | 2 | emplacement de descripteur | e3 | `.field = pico8_ie_field,` |
| 334 | 2 | emplacement de descripteur | e3 | `.set_field = pico8_ie_set_field,` |

#### `src/pico8_research_icon.h`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 19 | 1 | en-tête | e3 | `* read off a `pico8_ie_ke_t`, never pico8_entity_get_field() on the kind` |

#### `src/pico8_score_panel.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 64 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_pj_cr_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 91 | 2 | emplacement de descripteur | e3 | `.field = pico8_pj_cr_field,` |
| 336 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_pj_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 371 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 385 | 2 | emplacement de descripteur | e3 | `.field = pico8_pj_field,` |

#### `src/pico8_scroll.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 31 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_qb_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 66 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 69 | 1 | aide `*_take_*` (déf.) | e3 | `static void qb_take_vec(pico8_value_t value, const pico8_vec_t **out) {` |
| 80 | 1 | aide `*_take_*` (déf.) | e3 | `static void qb_take_fixed(pico8_value_t value, pico8_fixed_t *out) {` |
| 88 | 1 | écrivain `set_field` (déf.) | e3 | `static int pico8_qb_set_field(void *self, const char *name, pico8_value_t value) {` |
| 92 | 1 | appel d'aide `*_take_*` | e3 | `qb_take_vec(value, &qb->v);` |
| 96 | 1 | appel d'aide `*_take_*` | e3 | `qb_take_fixed(value, &qb->ma);` |
| 104 | 2 | emplacement de descripteur | e3 | `.field = pico8_qb_field,` |
| 105 | 2 | emplacement de descripteur | e3 | `.set_field = pico8_qb_set_field,` |

#### `src/pico8_slipway.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 28 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_jc_too_far_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 48 | 1 | aide `*_take_*` (déf.) | e2 | `static void jc_too_far_take_jc(pico8_value_t value, pico8_jc_t **out) {` |
| 60 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_jc_too_far_set_field(void *self, const char *name, pico8_value_t value) {` |
| 64 | 1 | appel d'aide `*_take_*` | e2 | `jc_too_far_take_jc(value, &too_far->fo);` |
| 71 | 2 | emplacement de descripteur | e2 | `.field = pico8_jc_too_far_field,` |
| 72 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_jc_too_far_set_field,` |
| 88 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_jc_mouse_point_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 109 | 2 | emplacement de descripteur | e2 | `.field = pico8_jc_mouse_point_field,` |
| 127 | 1 | site C | e1 | `if (e != NULL && pico8_entity_get_field(e, name, &v)) {` |
| 164 | 1 | site C | e1 | `static void jc_take_fixed(pico8_value_t value, pico8_fixed_t *out, int *has) {` |
| 251 | 1 | site C | e1 | `jc_take_fixed(mt_val, &self->mt, &self->has_mt);` |
| 258 | 1 | site C | e1 | `pico8_entity_set_field(self->probe->et, "fo", pico8_value_ref(self));` |
| 489 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_jc_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 545 | 1 | lecteur (délégation) | e2 | `return pico8_ke_field(self, name, out_value);` |
| 557 | 1 | aide `*_take_*` (déf.) | e2 | `static void jc_take_holder(pico8_value_t value, pico8_struct_header_t **out) {` |
| 569 | 1 | aide `*_take_*` (déf.) | e2 | `static void jc_take_bool(pico8_value_t value, int *out, int *has) {` |
| 583 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_jc_set_field(void *self, const char *name, pico8_value_t value) {` |
| 587 | 1 | appel d'aide `*_take_*` | e2 | `jc_take_holder(value, &jc->fo);` |
| 591 | 1 | appel d'aide `*_take_*` | e2 | `jc_take_bool(value, &jc->p_, &jc->has_p_);` |
| 606 | 2 | emplacement de descripteur | e2 | `.field = pico8_jc_field,` |
| 607 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_jc_set_field,` |

#### `src/pico8_struct.h`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 43 | 1 | en-tête | e3 | `typedef int (*pico8_struct_field_fn)(const void *self, const char *name, pico8_value_t *out_value);` |
| 47 | 1 | en-tête | e3 | `typedef int (*pico8_struct_set_field_fn)(void *self, const char *name, pico8_value_t value);` |
| 104 | 1 | en-tête | e3 | `pico8_struct_field_fn field;` |
| 105 | 1 | en-tête | e3 | `pico8_struct_set_field_fn set_field;` |

#### `src/pico8_trade.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 21 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_trade_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 49 | 2 | emplacement de descripteur | e3 | `.field = pico8_trade_field,` |

#### `src/pico8_ui.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 35 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_je_placeholder_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 43 | 2 | emplacement de descripteur | e3 | `.field = pico8_je_placeholder_field,` |
| 56 | 1 | site C | e1 | `if (inst != NULL && pico8_entity_get_field(inst, name, &v)) {` |
| 107 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_ui_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 142 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 147 | 2 | emplacement de descripteur | e3 | `.field = pico8_ui_field,` |
| 346 | 1 | site C | e1 | `if (pico8_entity_get_field(e, "fa", &fa_val) && fa_val.kind == PICO8_VALUE_FIXED) {` |
| 359 | 1 | site C | e1 | `pico8_entity_set_field(self->gj, "m_", pico8_value_bool(0));` |
| 363 | 1 | site C | e1 | `pico8_entity_set_field(self->o_, "m_", pico8_value_bool(1));` |

#### `src/pico8_ui_button.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 106 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_js_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 170 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 173 | 1 | aide `*_take_*` (déf.) | e3 | `static void button_take_bool(pico8_value_t value, int *out, int *has) {` |
| 186 | 1 | aide `*_take_*` (déf.) | e3 | `static void button_take_lq(pico8_value_t value, pico8_lq_t **out) {` |
| 198 | 1 | écrivain `set_field` (déf.) | e3 | `static int pico8_js_set_field(void *self, const char *name, pico8_value_t value) {` |
| 202 | 1 | appel d'aide `*_take_*` | e3 | `button_take_lq(value, &js->lq);` |
| 206 | 1 | appel d'aide `*_take_*` | e3 | `button_take_bool(value, &js->m_, &js->has_m_);` |
| 220 | 2 | emplacement de descripteur | e3 | `.field = pico8_js_field,` |
| 221 | 2 | emplacement de descripteur | e3 | `.set_field = pico8_js_set_field,` |
| 478 | 1 | lecteur `field` (déf.) | e3 | `static int pico8_fq_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 503 | 1 | lecteur (délégation) | e3 | `return pico8_ke_field(self, name, out_value);` |
| 506 | 1 | écrivain `set_field` (déf.) | e3 | `static int pico8_fq_set_field(void *self, const char *name, pico8_value_t value) {` |
| 510 | 1 | appel d'aide `*_take_*` | e3 | `button_take_bool(value, &fq->m_, &fq->has_m_);` |
| 519 | 2 | emplacement de descripteur | e3 | `.field = pico8_fq_field,` |
| 520 | 2 | emplacement de descripteur | e3 | `.set_field = pico8_fq_set_field,` |

#### `src/pico8_wormhole.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 21 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_mz_jj_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 35 | 2 | emplacement de descripteur | e2 | `.field = pico8_mz_jj_field,` |
| 70 | 1 | site C | e1 | `if (!pico8_entity_get_field(p, "fx", &fx_val)) {` |
| 264 | 1 | lecteur `field` (déf.) | e2 | `static int pico8_mz_field(const void *self, const char *name, pico8_value_t *out_value) {` |
| 331 | 1 | lecteur (délégation) | e2 | `return pico8_ke_field(self, name, out_value);` |
| 334 | 1 | aide `*_take_*` (déf.) | e2 | `static void mz_take_vec(pico8_value_t value, pico8_vec_t **out) {` |
| 352 | 1 | aide `*_take_*` (déf.) | e2 | `static void mz_take_et(pico8_value_t value, pico8_struct_header_t **out, int *has) {` |
| 367 | 1 | aide `*_take_*` (déf.) | e2 | `static void mz_take_bool(pico8_value_t value, int *out, int *has) {` |
| 381 | 1 | écrivain `set_field` (déf.) | e2 | `static int pico8_mz_set_field(void *self, const char *name, pico8_value_t value) {` |
| 385 | 1 | appel d'aide `*_take_*` | e2 | `mz_take_vec(value, &mz->ra);` |
| 389 | 1 | appel d'aide `*_take_*` | e2 | `mz_take_vec(value, &mz->p);` |
| 393 | 1 | appel d'aide `*_take_*` | e2 | `mz_take_et(value, &mz->probe->et, &mz->probe->has_et);` |
| 397 | 1 | appel d'aide `*_take_*` | e2 | `mz_take_bool(value, &mz->p_, &mz->has_p_);` |
| 421 | 2 | emplacement de descripteur | e2 | `.field = pico8_mz_field,` |
| 422 | 2 | emplacement de descripteur | e2 | `.set_field = pico8_mz_set_field,` |

#### `src/pico8_wormhole_display.c`

| l. | occ. | nature | sous-étape | ligne |
|---|---|---|---|---|
| 21 | 1 | site C | e1 | `if (inst != NULL && pico8_entity_get_field(inst, name, &v)) {` |
