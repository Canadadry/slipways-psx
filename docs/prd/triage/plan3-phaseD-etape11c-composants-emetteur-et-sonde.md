---
title: "Plan 3, étape D.11c : émetteur de particules et sonde deviennent des composants portés par pointeur (règle 2 de la phase 2, Q7) — `pico8_emitter_t` sur `g_`/`fv`, `hj`/`pm`/`mx`, `ga` ; `pico8_probe_t` sur `g_`/`fv`, la lignée `qh`, `mz`, `jc` ; crochets `emitter_of`/`probe_of` dans la table de classe ; `ke:ks` typé (G `ke.c:43`) ; deux chunks c1/c2"
description: "Après D.11b, `par-nom` compte encore les lectures et écritures par nom de l'émetteur de particules (`ei`, `ez`, `f_`, `g_:mm`) et de la sonde (`bv`, `dl`, `d_`), code partagé par des branches distinctes de l'arbre qui lit sur chaque hôte des champs rangés à des places différentes. D.11c range ces deux groupes de champs dans deux composants portés par pointeur, que le code partagé atteint par un crochet typé de la table de classe, sans changer un pixel ; `par-nom` `src/` passe de 317 à 292 (mesuré à `2abe98e`, D.11b gradée)."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.11b était implémentée** (`plan-pilot`, étape 2b), **puis révisé après sa graduation** : D.11b est gradée à `2abe98e` (VERIFIED). Tout ce qui suit a été lu et mesuré sur l'arbre **commis** `2abe98e`, extrait par `git archive 2abe98e` dans le scratchpad (la première version lisait `e2e5da0`) : `grep`, `sh tools/data_residue.sh` (avec `--detail`) et la recette d'occurrences de D.8 seulement. **Rien n'a été compilé ni lancé.** Les numéros de ligne sont ceux de `2abe98e`. Les hypothèses `H-D11b-n` de la première version sont confrontées à `2abe98e` plus bas ; toutes les sections qui en dépendaient ont été refaites.

Le problème, vu du mainteneur du port : le cart fait porter la même mécanique à des branches distinctes de l'arbre des classes.

- **L'émetteur de particules** (`f_` crée la liste `qy`, `ez` y ajoute des particules par `self:mm()`, `ei` les avance et les dessine, `g_:mm` en façonne une) tourne sur la sonde `g_` et l'aide `fv`, sur l'ascension `hj` et ses filles `pm`, `mx`, et sur l'éclat `ga` — trois branches : `g_`, `hj` (sous `qh` sous `jo`), `ga` (sous `jo`).
- **La sonde** (`bv` suit la souris et choisit la cible `et`, `dl` dessine l'info-bulle, `d_` valide la cible) tourne sur `g_`/`fv`, sur les huit classes de la lignée `qh`, sur le trou de ver `mz` (`bv`, `dl`, `d_`) et sur la voie `jc` (`dl`, `d_`) — quatre branches.

Chaque branche range ces champs à sa place (`g_->dw`, `qh->dw`, `mz->dw`, `jc->dw` ; `g_->qo`, `hj->qo`, `ga->qo`), si bien que le code partagé (`particle.c`, `probe.c`) les lit et les écrit **par nom** : 23 lignes comptées par `par-nom` dans ces deux fichiers, plus l'aide `*_take_*` de `ke:ks` (G `ke.c:43`) qui déballe l'état que `d_` lui passe. Tant qu'elles passent par un nom, le compilateur ne vérifie ni le nom, ni le type, ni la présence, et l'infrastructure par nom (D.11e) ne peut pas tomber. L'étude D.9 (§4, point 1) a constaté qu'aucune forme de D ne couvrait ce cas ; l'humain a tranché Q7 : la règle 2 de la phase 2 (composant porté par pointeur), appliquée dès D **à ces deux groupes seulement**.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `2abe98e` (§5, « Phase D — Données », l. 274) :

> - D.11c — `par-nom`, classe M (Q7) : émetteur de particules (`ei`, `ez`, `f_`, `g__mm`) et sonde (`bv`, `dl`, `d_`) → composants portés par pointeur, règle 2 de la phase 2 appliquée à ces deux groupes seulement. Empreinte ≈ 15 (15, 0) : `particle`, `probe` et les 13 fichiers des classes qui les portent. Compte : `par-nom` ≈ 862. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : oui.

Décisions de D.9, recopiées (Q7 lie cette étape ; Q5 et Q6 la touchent par le lien `parent` et par D.11e) :

> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Paragraphe « Liste de D » et règles communes, recopiés (l. 237 et 239) :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.

> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

Ligne D.11b, recopiée à `2abe98e` (l. 273), avec sa note « Découvert à D.11b » :

> - D.11b — `par-nom`, lectures sans receveur typé : B (7 + 1 ; test d'appartenance par le lien parent, `pico8_qh_of(jo)`, Q5), (C passe en tête de D.11e : ses descripteurs sont ceux que D.11e rouvre, revue du plan après D.11a), F (5 + 2 ; struct locale de cadre `{ep, eo, hi, ff, ca, gq, ra}` remplie par les cinq appelants de `pz`), K (4 ; enregistrement typé par sorte `{rr, has_s, s, me, has_od, od}`, résolu le long de la chaîne, pointé par `pico8_entity_kind_t`). Empreinte ≈ 15 (15, 0). Compte : `par-nom` ≈ 882. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non. Découvert à D.11b : deux chunks (b1 : 15 chemins ; b2 : 7). B (Q5) : `pico8_struct_class_t.parent` (9 filles de `jo`, `fv`) est un pointeur vers l'accesseur de la classe parente (`const pico8_struct_class_t *(*parent)(void)`), pas vers ses données : les descripteurs parents sont `static` dans d'autres unités et un initialiseur statique veut une constante d'adresse ; `pico8_qh_of()`/`pico8_nl_of()` remontent ce lien (NULL hors lignée) ; `pico8_jo_is_family` (test par `states[LL]`) pourrait s'écrire sur `parent`, non fait. La branche « `hp` chaîne » de `jo:pq` était inexprimable, retirée (`pico8_gb_find` sans appelant de `src/`, gardée pour ses TU). K : `pico8_entity_kind_t.rec` → `pico8_kind_rec_t` `{rr, has_s, s, me, has_od, od}`, généré dans `pico8_game_data.c` pour les 9 sortes (pas 11), mêmes chaînes que `pico8_X_start` (s/od en mêmes littéraux) ; la garde « prêt » du `field` des sortes n'est pas dans l'enregistrement ; l'oracle de transition `test_kind_rec_matches_the_reader` (`test_pico8_entity_kind`, par l'aide `kind_field` existante, aucune ligne `par-nom` ajoutée) est à retirer par D.11d/D.11e, après quoi D.11e peut supprimer le `field` des sortes. F : `pico8_pz(const pico8_pz_frame_t *, …)`, cadre rempli par `fv:pq`, `cr`, `ix[4].my`, `jo:gh`, `jo:fe` (aide `jo_pz_frame`) ; vues `pj_cr`/`ha_pz` laissées à D.11e ; le `ra` du cadre de `fv` n'est jamais lu (`fv.hi` toujours posé) ; le cas « une liste arrête `pz` » de `test_pz_closed` est retiré (inexprimable). `parent` est disponible pour C (D.11e). `par-nom` 884 (`src/` 317, `tests/` 567 : l'oracle n'ajoute pas la ligne prévue, d'où −1 sur les 885 du PRD) ; `valeur` 3876 / 4170 ; `type-tag` 343 / 544 ; résidu D 4681 lignes dans 124 fichiers.

Ligne D.11e, recopiée à `2abe98e` (l. 276), avec la note de D.11b qui y a été ajoutée (duplication des littéraux de `pico8_kind_rec`), parce que son premier chunk (classe C) touche la sonde (`get_dw`, `set_fo`) et que D.11c doit lui éviter de refaire ce travail :

> - D.11e — `par-nom`, suppression de l'infrastructure. Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`, laissé à D.11a ; accesseur typé par champ dans la table de classe, NULL = absent, Q6 ; l'oracle accesseurs = lecteurs naît et meurt dans l'étape). Découvert à D.11b (pour D.11d/D.11e) : les littéraux `s`/`od` des `pico8_X_kind_rec` doublent ceux de `pico8_X_start` ; seul `test_kind_rec_matches_the_reader` en garde l'égalité — à sa suppression, garder une comparaison rec ↔ `pico8_X_start` ou des constantes nommées partagées. Puis : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks. Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non.

Note de D.10a qui confie à D.11c le site G `ke.c:43`, extrait recopié de la note « Découvert à D.10a » (l. 269) :

> Sites G laissés : `ui_button.c:599` (D.10b), `slipway.c:261` et `research_icon.c:182` (D.11a), `ke.c:43` (D.11c) ; pris à D.11b (classe C) : la présence des noms `bm` dans `lt`/`he` et de `gv` dans `jc:hq`.

Les autres textes du plan qui lient cette étape, à `2abe98e` — la section « Phase D » entière (puces dont la **Sortie**, note de D.0, « Liste de D » avec empreinte, règles communes et ordre, toutes les lignes D.2 à D.12 avec **toutes** leurs notes « Découvert à D.x », l'arrêt « champ sans règle », les décisions de D.9, les comptes au départ de D.10), les règles de phase du §5 (1a, 1b, **2**, 3), le §6 et le §7 — sont recopiés tels quels, par extraction de lignes, en **Annexe A**. Les lignes de l'étude D.9 qui décrivent M et Q7 sont en **Annexe B**.

### Hypothèses `H-D11b-n`, confrontées à `2abe98e`

La première version de ce PRD, écrite sur `e2e5da0` pendant l'implémentation de D.11b, supposait son issue. Confrontation avec l'arbre gradé et la note « Découvert à D.11b » :

| Hypothèse | Énoncé (résumé) | Verdict à `2abe98e` |
|---|---|---|
| H-D11b-1 | `par-nom` `src/` 317, `tests/` 568, total 885 | **fausse pour `tests/`** : `src/` **317** / 377 occ. (tenue), `tests/` **567** / 622 occ., total **884** / 999 occ. (l'oracle K n'a pas ajouté la ligne prévue, et b2 en a retiré une) ; `valeur` 3876 / 4170, `type-tag` 343 / 544, `ref-audit` 44 / 45 ; résidu D 4681 lignes, 124 fichiers ; `par-nom` sur 70 fichiers. Tous les comptes attendus de D.11c se décalent de −1 sur `tests/` et le total |
| H-D11b-2 | `pico8_struct_class_t` porte `const pico8_struct_class_t *parent` (données), posé sur 9 filles de `jo` et `fv` | **fausse dans sa forme** : `parent` est un **pointeur vers l'accesseur** de la parente, `const pico8_struct_class_t *(*parent)(void)`, dernier membre de `struct pico8_struct_class` (`struct.h:100`) ; posé sur les 10 descripteurs prévus (`help.c:194` : `.parent = pico8_g__struct_class`). `pico8_g__of` remonte donc `cls->parent()` comme `pico8_qh_of` (`planet_family.c:1006`), sans cast ; le repli par identité de crochet n'a plus d'objet |
| H-D11b-3 | `pico8_qh_of`/`pico8_nl_of` existent (`const`) | **tenue** (`planet_family.h:166`, `:170`) ; D.11c n'en a pas besoin |
| H-D11b-4 | D.11b ne touche ni `particle`, ni `probe`, ni `wormhole`, ni `slipway`, ni `slipway_display`, ni `ke` | **tenue** : ces onze fichiers (`.c` et `.h`) sont identiques à `e2e5da0` (`cmp`) ; leurs numéros de ligne restent |
| H-D11b-5 | vue `pico8_ie_ke_t` gardée (`me`, `has_od`, `od`), appel de `dl` décalé | **tenue** : vue dans `research_icon.h:41-49`, appel à `research_icon.c:214` |
| H-D11b-6 | `help.c` : `.parent` sur `fv`, appels de `ei`/`f_` gardés, décalés | **tenue** : `ei` `:82`, table `:187-194`, défauts d'émetteur `:207-217`, `f_` `:231` |
| H-D11b-7 | `planet_family*` : `.parent` ajoutés, sites de M inchangés, décalés | **tenue** (numéros refaits ci-dessous) |
| H-D11b-8 | `planet.c` : appels de `ke:ks` gardés, décalés | **tenue** : `:773`, `:829` |
| H-D11b-9 | `Makefile`, `DATA_ENGINE_SRCS`, `tools/` inchangés | **tenue** (`diff`) |
| H-D11b-10 | TU de D.11b : `qh`, `entity_kind`, `js` ; ceux de D.11c intacts | **tenue** : `test_pico8_fv`, `_jc`, `_ui`, `_g_`, `_jo`, `_ga`, `_hj`, `_mx`, `_pm`, `_probe_mm_rnd_order`, `_ga_export_panel_by`, `_planet_gh_row_lq` identiques à `e2e5da0` : leurs numéros de ligne restent |
| H-D11b-11 | la note de D.11b ne réaffecte ni M, ni `ke.c:43`, ni les prédicats de porteurs de `fo` | **tenue** (note l. 273 ; `planet.c:195`, `probe.c:73`, `slipway.c:558` inchangés). La note ajoute : `pico8_jo_is_family` pourrait s'écrire sur `parent` (non fait) ; une note pour D.11d/D.11e dans la ligne D.11e (littéraux `s`/`od` des `pico8_X_kind_rec`), sans effet sur D.11c |
| H-D11b-12 | aucune constante changée | **tenue** selon la graduation (VERIFIED), non recomptée ici |

Conséquences pour ce PRD : départ `par-nom` **884** (`src/` 317, `tests/` 567) ; arrivée **859** (la ligne dit ≈ 862 : −3, le site G `ke.c:43` et le −1 de D.11b) ; `pico8_g__of` écrit sur `cls->parent()` ; numéros refaits dans `help.c`, `planet_family.c/.h`, `planet_family_trade.c`, `planet.c`, `research_icon.c`, `pico8_struct.h`. Leçon de la note reprise : un initialiseur statique de descripteur veut une constante d'adresse — les crochets `emitter_of`/`probe_of` pointent des **fonctions** publiques, jamais les données `static` d'une autre unité.

### Sections revérifiées sur `2abe98e`

Refaites dans cette révision : État de départ, Inventaire (tous les numéros des fichiers touchés par D.11b), comptes attendus, classification, empreinte, greps de la « Sortie », critères `FAILED:` de départ et de comptes, budget. Plus rien n'est ⟲ : si le HEAD de départ de D.11c n'est pas `2abe98e` (ou un descendant qui ne touche que `docs/`), l'implémenteur remesure et explique l'écart.

### Lecture de ces textes pour D.11c

- **Périmètre** : la ligne nomme deux groupes, pris tels quels — l'**émetteur** (`ei`, `ez`, `f_`, `g__mm`) et la **sonde** (`bv`, `dl`, `d_`). S'y ajoutent : les **définitions d'aides par nom** de `particle.c` et `probe.c` qui perdent leur dernier appelant (classe H de l'étude : une aide meurt dans le chunk où part son dernier appelant) ; le **site G `ke.c:43`** (`ke_take_string`, déballage de l'état passé à `ke:ks`), que la note de D.10a confie à D.11c et dont `d_` est l'appelant qui produit une valeur. Hors périmètre : les sites C et `jc.mt` (D.11e, premier chunk), les lecteurs `field`/écrivains `set_field` des descripteurs et leurs aides `*_take_*` (D.11e), les T1/T2 (D.11d), les listes (`qy` est L2, D.12b).
- **« composants portés par pointeur, règle 2 de la phase 2 appliquée à ces deux groupes seulement »** : un groupe de champs qui apparaît dans deux branches distinctes devient un composant (§5, phase 2, règle 2). Le groupe est l'ensemble des champs que le code partagé lit ou écrit **et que l'hôte ne tient pas pour lui-même** : les champs propres de la sonde et de l'émetteur. Les champs de l'hôte que ce code touche aussi (`ra`, `p`, `p_`, `fa`, `me`, `t`, et `fo`/`qs` de la cible) restent chez l'hôte : ce sont des champs transversaux (classe C de l'étude, Q6) ou du noyau `ke` ; D.11c les atteint par un cast vérifié sur l'ensemble fermé d'hôtes que le cart fixe (forme A de D.11a), sans accesseur de table (voir « Champs d'hôte »). Les autres règles de la phase 2 (archétypes, noyau, test ponctuel, sortie « plus d'enchâssement ») ne s'appliquent pas : **l'enchâssement parent → fille reste**.
- **« Empreinte ≈ 15 (15, 0) : `particle`, `probe` et les 13 fichiers des classes qui les portent »** : remesurée à **18 fichiers de `src/` et 5 TU** (plus `docs/plan-3.md`) : `pico8_struct.h` (crochets), `slipway.h`/`wormhole.h`/`planet_family.h` (structs d'hôtes), `slipway_display.c` (lecture de `jc.et`), `research_icon.c` (`dl` sur la vue), `ke.c/.h` et `planet.c` (G). Les TU suivent un contrat changé (`ke:ks`, `ei` sur un non-émetteur, `g_.dw` écrit en membre) ; voir « Empreinte ».
- **« Compte : `par-nom` ≈ 862 »** : ce PRD attend `src/` **−25** (317 → 292 : 23 lignes M, 2 lignes G), `tests/` ± 0 (567), soit un total **859**. L'écart de −3 avec la ligne : le site G `ke.c:43` (deux lignes), que l'estimation de l'étude ne comptait pas dans M, et le −1 de `tests/` constaté à D.11b (884 au lieu de 885). Rapporté.
- **« §6 : aucun »** : `test_pico8_ga_export_panel_by` construit des `ga` (dont la struct change) et `test_pico8_planet_gh_row_lq` des planètes (dont `qh` change) : ni l'un ni l'autre ne nomme un membre déplacé ni une fonction dont la signature change (grep à `2abe98e`). Ils doivent rester verts, sans modification.
- **« `class-coverage-net` : oui »** : chaque constructeur d'hôte change (pose du pointeur de composant). Exigé à chaque commit, identique à `tools/class_coverage_net_expected.txt`.
- **« `sanitize-net` : oui »** : exigé à chaque commit. La forme retenue ne change **aucune** durée de vie ni aucun propriétaire (stockage du composant dans l'hôte, voir « Porté par pointeur »), mais la ligne l'exige et elle est bon marché ; sortie propre attendue.
- **Risque écrit (Q7)** : la règle 2 s'applique ici **sans** le test ponctuel d'archétype (règle 3 de la phase 2). Le risque : un défaut recopié dans le mauvais membre de composant, ou pris de la mauvaise classe, en déplaçant les `struct_init` (piège connu du même ordre : le `nu=o()` vide de `ga`, et TROU-21 pour le `k_` d'un `ga` né de `pm`). Parade retenue, sans test d'archétype : (1) chaque hôte a son TU qui lit par nom, sur une instance construite, chacun des noms déplacés (grep à `2abe98e` : `qo`, `fc`, `py`, `du`, `k_` dans `fv`, `g_`, `ga`, `hj`, `mx`, `pm`, `ui` ; `pn`, `ki`, `pk` dans `fv`, `g_`, `hj`, `mx`, `pm`, `ui` ; `dw`, `dq`, `fp`, `od` dans `fr`, `fv`, `g_`, `hj`, `jc`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh`, `ui` ; `ik` dans `g_`, `jc`, `mz` ; `qy` et `et` dans la plupart) et les plie dans ses constantes ou les compare à `pico8_X_start` : un défaut mal placé les rougit ; (2) des mutations ciblées sur les `struct_init` (M3, M9) ; (3) un cas structurel nouveau dans `test_pico8_ui` (quelles classes portent quel composant, composants distincts par instance). Le lecteur par nom de chaque hôte lisant désormais le composant, un « oracle lecteur = composant » serait tautologique : il n'est pas retenu.
- **Leçons des notes à ne pas refaire** (D.7a à D.11a, et D.11b) : classification (a)/(b)/(c) absente ou fausse ; (c) non déclarée ; lignes qui gagnent une catégorie omises ; conversion typé → valeur ajoutée le temps d'un commit (d'où le site G dans le même chunk que `d_`) ; constante de hachage « mise à jour » ; octet gelé remplacé par un littéral nu ; TU corrompu qui passait en relisant sa propre corruption (balayage ASan des TU) ; site « typé » en déplaçant son déballage dans une aide non comptée ; **symbole défini hors de `DATA_ENGINE_SRCS` nommé depuis un fichier de cette liste** (D.11a) ; **cas de mort d'un TU non vu par le PRD** (D.11a) : l'implémenteur cherche par grep, dans `tests/`, chaque symbole dont le contrat change.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json`, et rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes, alias, registre, renommage pour échapper aux greps, cast non vérifié, cale valeur ↔ typé permanente ; mutations restaurées par `cp` ; binaires lancés exactement comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan) ; balayage ASan+UBSan des TU ; chunks d'au plus ~40 chemins ; jamais l'appli raylib.

### État de départ

**Mesuré sur `2abe98e`** (arbre archivé), `sh tools/data_residue.sh`, `--detail` et la recette d'occurrences de D.8, sans build :

| Catégorie | Lignes | Fichiers | Occurrences | `src/` (lignes / occ.) | `tests/` (lignes / occ.) |
|---|---|---|---|---|---|
| `valeur` | 3876 | 123 | 4170 | rapporté | rapporté |
| `type-tag` | 343 | 63 | 544 | rapporté | rapporté |
| `ref-audit` | 44 | 15 | 45 | 44 / 45 | 0 |
| `par-nom` | 884 | 70 | 999 | **317 / 377** | **567 / 622** |
| **résidu D** | **4681** | **124** | — | | |

`DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch` (`Makefile`, inchangé).

Catégories par fichier de l'empreinte à `2abe98e` (lignes / occurrences) :

| Fichier | `valeur` | `type-tag` | `ref-audit` | `par-nom` |
|---|---|---|---|---|
| `pico8_struct.h` | 4/4 | 5/5 | 0/0 | 4/4 |
| `pico8_particle.c` | 12/12 | 4/7 | 2/2 | **5/5** |
| `pico8_particle.h` | 1/1 | 0/0 | 0/0 | 0/0 |
| `pico8_probe.c` | 81/83 | 11/20 | 0/0 | **40/45** |
| `pico8_probe.h` | 3/3 | 0/0 | 0/0 | 0/0 |
| `pico8_help.c` | 18/18 | 1/1 | 0/0 | 8/10 |
| `pico8_planet_family.c` | 93/93 | 4/7 | 1/1 | 58/70 |
| `pico8_planet_family.h` | 4/4 | 0/0 | 0/0 | 0/0 |
| `pico8_planet_family_trade.c` | 33/34 | 3/4 | 1/1 | 31/38 |
| `pico8_wormhole.c` | 49/52 | 3/5 | 0/0 | 15/18 |
| `pico8_wormhole.h` | 2/2 | 0/0 | 0/0 | 0/0 |
| `pico8_slipway.c` | 59/61 | 7/12 | 1/1 | 21/26 |
| `pico8_slipway.h` | 4/4 | 0/0 | 0/0 | 0/0 |
| `pico8_slipway_display.c` | 5/5 | 0/0 | 0/0 | 0/0 |
| `pico8_research_icon.c` | 27/30 | 1/1 | 0/0 | 9/12 |
| `pico8_ke.c` | 7/7 | 3/4 | 0/0 | **3/3** |
| `pico8_ke.h` | 3/3 | 0/0 | 0/0 | 1/1 |
| `pico8_planet.c` | 81/83 | 7/13 | 0/0 | 33/35 |
| `tests/test_pico8_fv.c` | 89/93 | 5/8 | 0/0 | 12/16 |
| `tests/test_pico8_jc.c` | 192/205 | 6/11 | 0/0 | 56/56 |
| `tests/test_pico8_ui.c` | 384/390 | 20/38 | 0/0 | 40/41 |
| `tests/test_pico8_g_.c` | 180/194 | 13/20 | 0/0 | 29/30 |
| `tests/test_pico8_jo.c` | 37/38 | 14/26 | 0/0 | 5/5 |

**Liens** (leçon de D.11a) : `DATA_ENGINE_SRCS` (`Makefile` l. 238-289) contient `ke.c`, `particle.c`, `planet.c`, `planet_family.c`, `probe.c`, `slipway.c`, `slipway_display.c`, `wormhole.c`, `entity.c`, `entity_geom.c` ; **pas** `help.c`, `planet_family_trade.c`, `research_icon.c`. Les symboles nouveaux et leurs lieux (voir « Décisions ») respectent la règle : ce qu'un fichier de la liste nomme est défini dans la liste ; `help.c`, `planet_family_trade.c`, `research_icon.c` nomment des symboles de la liste, jamais l'inverse (H2).

### Inventaire (mesuré sur `2abe98e`)

Recette : `sh tools/data_residue.sh --detail`, `grep -nE "$(pattern par-nom)" src/pico8_particle.c src/pico8_probe.c src/pico8_ke.c`, puis grep de chaque membre déplacé (`grep -nE '(->|\.)(qy|qo|ro|qg|q_|fc|k_|py|ki|pn|pk|pk_is_bool|du)\b'` et `'(->|\.)(et|has_et|dw|od|has_od|dq|fp|ik)\b'`) dans `src/` et `tests/`, et de chaque fonction dont le contrat change. Chaque site a été lu. **Chaque ligne `par-nom` retirée porte une seule occurrence** (vérifié ligne à ligne : les seules lignes à deux occurrences de `probe.c` sont les emplacements `.field`/`.set_field` des descripteurs, l. 48, 99, 100, 615, 616, infrastructure non touchée) : Δlignes = Δoccurrences attendu partout.

Notations : **R** lu, **W** écrit ; **c** ligne comptée ; **h** appel d'une aide par nom (non compté) ; **d** définition d'aide comptée. « Hôtes » : les classes dont une instance atteint le site (grep des tables de classe et des appels directs).

**Qui appelle quoi** (tables de classe et appels directs, `src/` à `2abe98e`) :

| Fonction | Par la table (`.X = …`) | Appels directs (type statique) |
|---|---|---|
| `pico8_f_` | — | constructeurs de `g_` (`probe.c:669`), `fv` (`help.c:231`), `hj` (`planet_family.c:564`), `pm` (`:677`), `ga` (`ga:pq`, `:750`), `mx` (`planet_family_trade.c:468`) |
| `pico8_ez` | — | `ga:pq` (`planet_family.c:751`, `pico8_ga_t *`), `ei` (`particle.c:167`) |
| `pico8_ei` | `.kj = pico8_ei_dispatch` : `g_` seul (`probe.c:610`) | `fv:kj` (`help.c:82`, `pico8_fv_t *`), `hj:kj` (`planet_family.c:400`, `pico8_hj_t *`, pour `hj`/`pm`/`mx`), `ga:kj` (`:840`, `pico8_ga_t *`) |
| `pico8_g__mm` | `.mm` : `g_` (`probe.c:614`), `fv` (`help.c:191`), `hj` (`planet_family.c:512`), `pm` (`:627`), `mx` (`planet_family_trade.c:422`) | — (appelé par `ez` via `self->cls->mm`) |
| `pico8_bv` | `.hq = pico8_bv_dispatch` : `g_`, `fv`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` (10 tables) | `mz:hq` (`wormhole.c:98`, `pico8_mz_t *`) |
| `pico8_dl` | `.ev = pico8_dl_dispatch` : `g_`, `fv`, les 8 de la lignée `qh`, `mz` (11 tables) | `jc:ev` (`slipway.c:455`, `pico8_jc_t *`), icône de recherche (`research_icon.c:214`, **vue** `pico8_ie_ke_t` sur la pile, ni `ke` ni porteur) |
| `pico8_d_` | `.ey = pico8_d__dispatch` : `g_`, `fv`, les 8 de la lignée `qh` (10 tables) | `jc:ey` (`slipway.c:278`), `mz:ey` (`wormhole.c:167`) |
| `pico8_ke_ks` | — | `d_` (`probe.c:340`, état `dq or "ph"`), `jo:lz`/`jo:ll` (`planet.c:773`, littéral `"ly"`), `jo:hh` (`planet.c:829`, `nk.me or "known"`) |

**Classe M — émetteur (`particle.c`, et `g_:mm` dans `probe.c`).** 8 lignes comptées (c1).

| Fichier:ligne | Fonction | Membre | Hôtes (type statique au site) | Remplacement | c/h/d |
|---|---|---|---|---|---|
| `particle.c:18` | `pico8_particle_field_fixed` | définition | — | supprimée (sans appelant) | d |
| `particle.c:26` | `pico8_particle_field_vec` | définition | — | supprimée | d |
| `particle.c:34` | `pico8_particle_field_list` | définition (et `PICO8_REF_AS_LIST`, `ref-audit`) | — | supprimée | d |
| `particle.c:43` | `pico8_particle_field_k_` | définition | — | supprimée | d |
| `particle.c:65` | `pico8_f_` | W `qy` | `self` : en-tête ; hôtes `g_`, `fv`, `hj`, `pm`, `mx`, `ga` | `em = self->cls->emitter_of(self)` (crochet non NULL asserté : même arrêt qu'aujourd'hui, où l'écrivain d'une classe sans `qy` s'arrête) ; `em->qy = qy` | c |
| `particle.c:77` | `pico8_ez` | R `qy` | idem, plus tout en-tête d'un TU | `em = cls->emitter_of != NULL ? cls->emitter_of(self) : NULL` ; `qy = em != NULL ? em->qy : NULL` ; **boucle inchangée** (elle tire `rnd()` même sans `qy`) | h |
| `particle.c:106` | `pico8_ei` | R `fa` (hôte) | `g_`/`fv`, `hj`/`pm`/`mx`, `ga` | aide d'hôte `emitter_host_fa_ra` (cast vérifié : lignée `g_` par `pico8_g__of`, famille `jo` par `pico8_jo_is_family`) | h |
| `particle.c:111` | `pico8_ei` | R `ra` (hôte) | idem | idem ; `ra` NULL → vecteur nul, comme aujourd'hui | h |
| `particle.c:114-117` | `pico8_ei` | R `qo`, `qg`, `ro`, `q_` | idem | `em->qo` … `em->q_` (crochet asserté non NULL, voir « `ei` sur un non-émetteur ») | h ×4 |
| `particle.c:118` | `pico8_ei` | R `k_` | idem | `cs = em->k_` (NULL = absent, comme aujourd'hui) | h |
| `particle.c:120` | `pico8_ei` | R `qy` | idem | `qy = em->qy` | h |
| `particle.c:167` | `pico8_ei` | R `fc` | idem | `pico8_ez(self, em->fc)` | h |
| `probe.c:129`, `:131` | `pico8_probe_field_truthy` | définition | — | supprimée (son seul appelant est `:395`) | d, d |
| `probe.c:378-382` | `pico8_g__mm` | R `pn`, `t`, `du`, `py`, `ki` | `.mm` de `g_`, `fv`, `hj`, `pm`, `mx` ; `NULL` (TU) | `em = self->cls->emitter_of(self)` ; `t = ((const pico8_ke_t *)self)->t` (le crochet non NULL est le test de classe, voir « `ke` des hôtes ») ; `self == NULL` : chaque champ lu 0, **comme aujourd'hui** | h ×5 |
| `probe.c:395` | `pico8_g__mm` | R `pk` (vérité) | idem | **piège** : un `pk` fixe est **toujours vrai** en Lua (0 compris) : `pk_truthy = em->pk_is_bool ? em->pk != 0 : 1` — `g_` et `fv` ont un `pk` fixe (vrai), `hj`/`pm`/`mx` suivent `pk_is_bool` | c |

`probe.c:123` (`pico8_probe_field_fixed`) garde un appelant après c1 (`bv`, `dw`, `:195`) : il meurt en c2.

Sites de l'émetteur hors `particle.c`/`probe.c` qui suivent le déplacement des membres (texte changé, aucune ligne comptée retirée) :
- `planet_family.c:780` (`ga:mm`, R `py` : `self->emitter->py`), `:806` (`ga:kj`, R `qy`), `:872-881` et `:901-907` (constructeur de `ga`, défauts et clés `pq`), `:410-418` (`pico8_hj_pk`), `:526-538` (`hj` `struct_init`), `:645-656` (`pm`), `planet_family_trade.c:440-451` (`mx`), `help.c:207-217` (`fv`), `probe.c:621`, `:635-645`, `:657-663` (`g_` `struct_init` et clés).
- Lecteurs et écrivains par nom (infrastructure, D.11e), qui lisent désormais le composant : `probe.c:463-496` (`g_`), `:584` (écrivain `qy`, ligne comptée **modifiée**), `planet_family.c:430-475` (`hj`), `:494` (écrivain `qy` de `hj`, comptée, modifiée), `:689-730` (`ga`), `:738` (écrivain `qy` de `ga`, comptée, modifiée).

**Classe M — sonde (`probe.c`).** 15 lignes comptées (c2).

| Fichier:ligne | Fonction | Membre | Hôtes | Remplacement | c/h/d |
|---|---|---|---|---|---|
| `probe.c:123` | `pico8_probe_field_fixed` | définition | — | supprimée (derniers appelants : `:195` en c2, `:378-382` partis en c1) | d |
| `probe.c:185` | `pico8_bv` | W `ra` (hôte) | `g_`/`fv`, lignée `qh`, `mz` | aide d'hôte `probe_host_set_ra_p` (cast vérifié : lignée `g_`, famille `jo`, `mz` exacte ; toute autre classe s'arrête) | c |
| `probe.c:191` | `pico8_bv` | W `p` (hôte) | idem | idem (même appel, même ordre : `ra`, puis `p` alloué, **avant** `ij` : une planète tenue est elle-même dans `cg.np`) | c |
| `probe.c:195` | `pico8_bv` | R `dw` | idem | `pb->dw` (`pb = self->cls->probe_of(self)`, asserté non NULL) | h |
| `probe.c:227` | `pico8_bv` | W `et` | idem | `pb->et = cible ou NULL (faux)` ; `pb->has_et = 1` ; l'assert de genre de l'écrivain (`g__take_target`/`family_take_target`/`mz_take_et`) devient un assert de classe de cible au site, même ensemble (lignée `LY`, `jo`, `jc`, cible jetable) | c |
| `probe.c:232` | `pico8_bv` | W `fo` sur la cible `et` | cible : famille `jo` (par `cg.np`), `jc` (par `cg.jc`), `g_bv_et` | aide `probe_et_hang_fo` : `pico8_jo_is_family(et)` → `((pico8_jo_t *)et)->fo` ; `et->cls == pico8_jc_struct_class()` → `((pico8_jc_t *)et)->fo` ; `et == &g_bv_et.header` → `g_bv_et.fo` ; chaque branche garde l'assert de porteur de son écrivain d'aujourd'hui (`jo_is_fo_holder_class`, `jc_is_fo_holder_class`, `bv_et_is_holder_class`), sans l'élargir | c |
| `probe.c:255` | `pico8_dl` | R `me` (hôte, `ke`) | `g_`/`fv`, lignée `qh`, `mz`, `jc` ; vue de l'icône | `pico8_dl(header)` : `((const pico8_ke_t *)self)->me` après le test du crochet ; la vue passe par `pico8_dl_show(gfx, mem, me, pb)` (voir « `dl` ») | c |
| `probe.c:263` | `pico8_dl` | R `et` (vérité) | idem | `pb->has_et && pb->et != NULL` | c |
| `probe.c:268` | `pico8_dl` | R `qs` sur la cible `et` | cible : `mz.jj` (`pico8_mz_jj_t`), vue `jc_too_far` (`g_jc_too_far`) répondent ; planète, `jc`, `g_bv_et` n'ont pas `qs` | aide `probe_et_qs` : `et->cls == pico8_mz_jj_struct_class()` → `->qs` ; `et->cls == pico8_jc_too_far_struct_class()` → `->qs` (typedef `pico8_jc_too_far_t` passé de `slipway.c` à `slipway.h`, précédent `pico8_mz_jj_t`) ; sinon NULL (absent) | c |
| `probe.c:279` | `pico8_dl` | R `od` (vérité) | idem | `pb->has_od` (un fixe est vrai) ; le texte `od.."$"` par `pico8_value_tostr(pico8_value_fixed(pb->od))` (API à valeur, Q12, déclarée) | c |
| `probe.c:317` | `pico8_d_` | R `et` (vérité) | `g_`/`fv`, lignée `qh`, `jc`, `mz` | `pb->has_et && pb->et != NULL` | c |
| `probe.c:322` | `pico8_d_` | R `od` | idem | `jt = pb->has_od ? pb->od : 0` (un `od` non fixe lisait 0 ; aucun hôte n'en a) | c |
| `probe.c:337` | `pico8_d_` | R `dq` (vérité) | idem | `pb->dq != NULL ? pb->dq : "ph"`, passé **typé** à `pico8_ke_ks` (G) | c |
| `probe.c:342` | `pico8_d_` | R `fp` (vérité, puis fixe) | idem | **piège** : un `fp` booléen donne `hx = 0` (vrai reste booléen, faux devient 0) ; un `fp` fixe est vrai et donne `hx = fp` : `hx = pb->fp_is_bool ? 0 : pb->fp` (`g_`, `fv`, `jc` : booléen ; lignée `qh`, `mz` : fixe) | c |
| `probe.c:350` | `pico8_d_` | R `ik` (vérité) | idem | `pb->has_ik ? pb->ik : 54` (`mz`, `jc` : présent ; `g_`, `qh` : absent) ; `pico8_rn(pico8_value_fixed(…))` (API à valeur, Q12) | c |
| `probe.c:359` | `pico8_d_` | W `p_` (hôte) | `g_`/`fv`, lignée `qh`, `jc`, `mz` | aide d'hôte `probe_host_raise_p_` (lignée `g_`, famille `jo`, `jc`, `mz` exacts ; chacun a `int p_; int has_p_;`) | c |

**G — `ke:ks` (`ke.c`).** 2 lignes comptées (c2).

| Fichier:ligne | Fonction | Remplacement | c/d |
|---|---|---|---|
| `ke.c:27` | `ke_take_string` | supprimée : `pico8_ke_ks(pico8_struct_header_t *self, const char *me)` prend l'état typé (NULL = nil, emprunté) | d |
| `ke.c:43` | `pico8_ke_ks` | `((pico8_ke_t *)self)->me = me` (cast existant, derrière l'assert de tag existant) | c |

Appelants de `ke:ks` qui suivent : `planet.c:773` (`"ly"`), `planet.c:829` (`ks_arg` passe de `pico8_value_t` à `const char *` : `nk->me` ou `"known"`), `probe.c:340` (`d_`). TU : `test_pico8_g_.c:495-496`, `test_pico8_jc.c:1441`, `test_pico8_jo.c:463`, `test_pico8_ui.c:518`, `:1024`, `:1040`, `:1158`, `:1161` (`pico8_value_ref(NULL)` → `NULL`), `:1164`, `:1168`, `:1327`, `:1332`, et les sept cas de mort de `test_ke_ks_stops_off_row` (`:1176-1224`, voir « TU »).

**Sites de la sonde hors `probe.c` qui suivent le déplacement des membres** (texte changé ; lignes comptées modifiées signalées) :
- `g_` : `probe.c:410-418` (`pico8_g__et`), lecteur `:443-460`, `:505` (`et`), écrivain `:599` (comptée, modifiée), `struct_init` `:624-634`.
- lignée `qh` : `planet_family.c:48-57` (`pico8_qh_et`), lecteur `:66-116`, écrivain `:157` (comptée, modifiée), `struct_init` `:193-206`, défauts `od` des filles `:325` (`nl`), `:376` (`os`), `:543` (`hj`) ; `planet_family_trade.c:240-243` (`nr` : `od`, `fp`, `dw`), `:310` (`fr`), `:455` (`mx`).
- `mz` : `wormhole.c:112-113` (`mz:hq` pose `jj` dans `et`), `:242-248` (`pico8_mz_et`), lecteur `:270-293`, `:313-319`, écrivain `:393` (comptée, modifiée), `struct_init` `:427-441`.
- `jc` : `slipway.c:252-253` (`jc:hq` pose `od`), `:259-261` (`jc:hq` pose `et`, **avec `has_et = et != NULL`** : pour `jc`, NULL est nil, jamais faux, comme le lit son lecteur `jc_ref_field`), `:263` (site C de D.11e : **seul son receveur** `self->et` devient `self->probe->et` ; ligne comptée, modifiée, **pas convertie**), lecteur `:503-511`, `:531-538`, `struct_init` `:618-631` ; `slipway_display.c:121` (`self->et`).
- TU : `test_pico8_jc.c:827` (`((pico8_g__t *)probe)->dw = 0` → `->probe->dw`).

**Pas touchés** (vérifié par grep) : `pico8_bv_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch`, `pico8_ei_dispatch`, `pico8_g__mm` restent les entrées des tables (10, 11, 10, 1, 5 tables) : les prédicats de porteurs qui comparent `cls->hq == pico8_bv_dispatch` (`probe.c:73`, `planet.c:195`, `slipway.c:558`) et les tests qui comparent ces pointeurs (`test_pico8_fv.c:365-368`, `test_pico8_g_.c:336-340`, `:630-634`, `:730-731`, `test_pico8_hj.c:349`, `:616`, `test_pico8_ga.c:712`) restent vrais. Les signatures publiques de `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_g__mm`, `pico8_bv`, `pico8_dl`, `pico8_d_` ne changent pas (les 13 TU qui les appellent ne suivent pas) ; seules changent `pico8_ke_ks` (G) et, par ajout, `pico8_dl_show`. Aucune instance d'hôte n'est construite par valeur ni copiée (grep : ni `pico8_X_t v;` ni affectation de struct d'hôte dans `src/`/`tests/`), ce qui garde sûr le pointeur vers le stockage propre.

## Solution

**Deux commits (chunks c1, c2), chacun vert seul**, sans pont :

1. **c1 — émetteur.**
   - `pico8_emitter_t` (`pico8_particle.h`) : `{pn, du, py, ki, pk, pk_is_bool, fc, k_, qo, ro, qg, q_, qy}`.
   - `pico8_struct_class_t` gagne `emitter_of` (NULL : la classe ne porte pas d'émetteur).
   - Hôtes : `g_` (et `fv` par enchâssement), `hj` (et `pm`, `mx`), `ga` portent `pico8_emitter_t *emitter` et son stockage `emitter_own` ; leurs membres propres du groupe disparaissent ; leurs `struct_init`, lecteurs et écrivains par nom passent par le composant.
   - `f_`, `ez`, `ei`, `g_:mm`, `ga:mm`, `ga:kj` lisent le composant ; les quatre aides par nom de `particle.c` et `pico8_probe_field_truthy` meurent.
   - `test_pico8_fv` perd un demi-cas devenu illégal (`ei` sur un bouton) ; `test_pico8_ui` gagne un cas structurel.
2. **c2 — sonde et `ke:ks`.**
   - `pico8_probe_t` (`pico8_probe.h`) : `{et, has_et, dw, od, has_od, dq, fp, fp_is_bool, ik, has_ik}`.
   - `pico8_struct_class_t` gagne `probe_of`.
   - Hôtes : `g_` (et `fv`), `qh` (et ses sept descendants), `mz`, `jc` portent `pico8_probe_t *probe` et `probe_own`.
   - `bv`, `dl`, `d_` lisent le composant ; champs d'hôte par aides à cast vérifié ; `pico8_dl_show` pour la vue de l'icône ; `pico8_ke_ks(self, const char *me)`.
   - `pico8_probe_field_fixed` et `ke_take_string` meurent ; `pico8_jc_too_far_t` passe dans `slipway.h`.
   - TU : `test_pico8_jc`, `_ui`, `_g_`, `_jo` suivent `ke:ks` et `g_.dw` ; le cas structurel de `test_pico8_ui` s'étend à la sonde.
   - Note « Découvert à D.11c » ; balayage ASan+UBSan des TU.

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (`make data-guard` inchangé : `résidu gardé (spec,overrides,stash,ob,dispatch) : 0 ligne(s)`) ;
- `make e2e` : **26 conformes** ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ; `make sanitize-net` propre ;
- balayage ASan+UBSan des TU propre ;
- `make data-residue` : `par-nom` `src/` = **292** (317 − 25), `tests/` = **567**, total **859** ; le détail par fichier égal à la table « Effet sur `make data-residue` » ; `valeur`, `type-tag`, `ref-audit` rapportés ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/` ni de `tools/` touché, le `Makefile` inchangé, aucune constante `*_HASH`/`identity[]` changée ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  # 1. plus aucune lecture ni écriture par nom dans le code partagé : vide
  grep -nE 'pico8_entity_(get|set)_field\(|_take_[a-z]+\(|field_truthy\(' src/pico8_particle.c src/pico8_ke.c
  grep -nE 'pico8_entity_(get|set)_field\(' src/pico8_probe.c
  #    attendu pour probe.c : vide ; ses lignes par-nom restantes sont les lecteurs,
  #    écrivains et emplacements des descripteurs (fm_placeholder, bv_et, g_), D.11e
  # 2. aides mortes : vide
  grep -nE '\b(pico8_particle_field_[a-z_]+|pico8_probe_field_(fixed|truthy)|ke_take_string)\b' src/*.[ch]
  # 3. membres du groupe hors composant (ni g_, ni hj, ni ga, ni qh, ni mz, ni jc ne les portent plus en propre)
  grep -nE '^\s+(pico8_fixed_t|const pico8_int_seq_t \*|pico8_list_t \*|int) (pn|du|py|ki|pk|pk_is_bool|fc|k_|qo|ro|qg|q_|qy);' \
      src/pico8_probe.h src/pico8_planet_family.h
  grep -nE '^\s+(pico8_fixed_t|const char \*|int|pico8_struct_header_t \*) (et|has_et|dw|od|has_od|dq|fp|ik);' \
      src/pico8_probe.h src/pico8_planet_family.h src/pico8_wormhole.h src/pico8_slipway.h
  #    attendu, premier grep : les trois lignes pn, du, py de pico8_g__keys_t (clés, pas l'état) ;
  #    second grep : les huit lignes et, has_et, dw, od, has_od, dq, fp, ik de pico8_probe_t ;
  #    le k_ de jc (palette de voie, pas de l'émetteur) reste dans pico8_jc_t
  # 4. crochets : 6 emitter_of, 12 probe_of dans les descripteurs
  grep -hE '^\s*\.emitter_of = ' src/*.c | wc -l   # 6
  grep -hE '^\s*\.probe_of = ' src/*.c | wc -l     # 12
  # 5. les entrées partagées des tables ne changent pas
  grep -hE '^\s*\.hq = pico8_bv_dispatch,' src/*.c | wc -l   # 10
  grep -hE '^\s*\.ev = pico8_dl_dispatch,' src/*.c | wc -l   # 11
  grep -hE '^\s*\.ey = pico8_d__dispatch,' src/*.c | wc -l   # 10
  grep -hE '^\s*\.mm = pico8_g__mm,' src/*.c | wc -l        # 5
  # 6. ke:ks typé
  grep -n 'void pico8_ke_ks' src/pico8_ke.h src/pico8_ke.c   # const char *me
  grep -n 'pico8_ke_ks(.*pico8_value_' src/*.c tests/*.c     # vide
  # 7. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^par-nom'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que les champs propres de l'émetteur de particules vivent dans un composant `pico8_emitter_t` porté par pointeur par `g_`, `hj` et `ga`, afin que `ei`, `ez`, `f_` et `g_:mm` les lisent typés au lieu de les chercher par nom sur trois branches de l'arbre (règle 2 de la phase 2, Q7).
2. En tant que mainteneur du port, je veux que les champs propres de la sonde vivent dans un composant `pico8_probe_t` porté par `g_`, la lignée `qh`, `mz` et `jc`, afin que `bv`, `dl` et `d_` les lisent et les écrivent typés.
3. En tant que mainteneur du port, je veux que la table de classe dise, par un crochet typé `emitter_of`/`probe_of`, si une classe porte un composant et où, afin que le code partagé atteint par la table (`bv`, `dl`, `d_`, `mm`) trouve son composant sans nom ni test de classe par site.
4. En tant que mainteneur du port, je veux que les champs de l'hôte que ce code touche (`ra`, `p`, `p_`, `fa`, `me`, `t`) et ceux de la cible (`fo`, `qs`) soient atteints par un cast vérifié sur l'ensemble fermé d'hôtes que le cart fixe, afin qu'aucun ne passe par un nom et qu'aucun accesseur partiel de D.11e ne soit anticipé.
5. En tant que mainteneur du port, je veux que les entrées `pico8_bv_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch`, `pico8_ei_dispatch` et `pico8_g__mm` des tables et les signatures publiques des sept fonctions restent, afin que les prédicats de porteurs de `fo` et les 13 TU qui les appellent ne changent pas.
6. En tant que mainteneur du port, je veux que la vérité de `pk` et le repli de `fp` soient reproduits exactement (un fixe est vrai, même 0 ; `fp` booléen donne 0), afin que les particules de la sonde et l'horloge avancée par `d_` restent identiques.
7. En tant que mainteneur du port, je veux qu'une `jc` garde « NULL = nil » pour son `et` dans le composant, afin que `d_` et `dl` lisent sur une voie ce qu'ils lisent aujourd'hui.
8. En tant que mainteneur du port, je veux que `ke:ks` reçoive l'état typé (`const char *`, NULL = nil), afin que le site G `ke.c:43` disparaisse avec l'aide qui déballait la valeur que `d_` lui passait.
9. En tant que mainteneur du port, je veux que l'icône de recherche dessine l'info-bulle de sa sorte par `pico8_dl_show`, à partir de sa vue et d'un composant local, afin qu'un objet qui n'est ni `ke` ni porteur ne passe plus par le lecteur par nom.
10. En tant que mainteneur du port, je veux que chaque aide par nom sans appelant soit supprimée dans le chunk où part son dernier appelant, afin qu'aucune aide morte ne reste pour D.11e.
11. En tant que mainteneur du port, je veux que le stockage de chaque composant soit dans son hôte (pointeur vers `X_own`, précédents `kw`/`kw_own` et `hg`/`hg_own`), afin qu'aucune allocation, aucune fuite (TROU-7) ni aucun propriétaire ne change.
12. En tant que mainteneur du port, je veux qu'aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste, afin que les TU liés sur elle se lient encore sans toucher au `Makefile` (leçon de D.11a).
13. En tant que gardien du compteur, je veux `par-nom` `src/` égal à 317 − 8 après c1 et 317 − 25 après c2, fichier par fichier comme la table de ce PRD, afin que chaque baisse soit un site retiré.
14. En tant que gardien du compteur, je veux, au message de chaque commit, pour chaque catégorie et chaque fichier : les comptes avant/après en lignes et en occurrences ; la classification (a1)/(a2)/(b)/(c) des lignes `par-nom` retirées ; les lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées sans être retirées ; la justification de chaque Δlignes > Δoccurrences. Afin qu'aucune baisse ne soit un tassement.
15. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, et qu'aucun alias, registre, renommage pour les greps, cast non vérifié ni cale valeur ↔ typé permanente ne soit ajouté, afin que la baisse veuille dire « retiré ».
16. En tant que gardien des TU, je veux que chaque cas retiré (demi-cas « `ei` sur un bouton », sept cas de mort de `ke:ks`) soit listé au message avec la raison qui le rend inexprimable, afin qu'aucune couverture ne disparaisse sans trace.
17. En tant que gardien des TU, je veux un cas structurel qui dise quelles classes portent quel composant et que deux instances n'en partagent pas, afin de prouver les crochets et le stockage propre sans oracle tautologique.
18. En tant que gardien des constantes, je veux qu'aucune constante `*_HASH`/`identity[]` ne change, afin que l'identité de l'état — dont les défauts déplacés dans les composants — reste prouvée par les TU de chaque hôte.
19. En tant que gardien des filets, je veux `make test`, `make e2e`, `make class-coverage-net` et `make sanitize-net` verts à chaque commit et le balayage ASan+UBSan des TU propre, afin de prouver qu'aucun comportement n'a changé.
20. En tant que `plan-pilot`, je veux une note « Découvert à D.11c » qui dise les deux composants, leurs hôtes, les crochets, les aides d'hôte, `pico8_dl_show`, `ke:ks` typé, ce que D.11e peut réutiliser (`get_dw`, `set_fo`) et ce que D.11d et D.12b trouvent déplacé, afin que la suite parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q7 est une décision humaine (plan, « Décisions de D.9 ») : ce PRD l'applique ; Q6 est à D.11e ; Q12 reste ouverte et n'est pas tranchée ici.

### Modules

- **`struct`** (`pico8_struct.h`) : deux crochets dans `struct pico8_struct_class`, `emitter_of` (c1) et `probe_of` (c2), déclarés par `struct pico8_emitter;`/`struct pico8_probe;` (précédent `struct pico8_qy;`).
- **`particle`** : `pico8_emitter_t` ; `f_`, `ez`, `ei` sur le composant ; aide d'hôte `emitter_host_fa_ra` (c1).
- **`probe`** : `pico8_probe_t`, `pico8_g__of()`, `pico8_dl_show()`, crochets `pico8_g__emitter_of`/`pico8_g__probe_of` (publics : `help.c` les nomme pour `fv`) ; `g_:mm` (c1) ; `bv`, `dl`, `d_` et les aides d'hôte et de cible (c2).
- **`planet_family`** : structs `hj`, `ga`, `qh` ; crochets `pico8_hj_emitter_of` (public : `mx`), `ga_emitter_of` (statique), `pico8_qh_probe_of` (public : `nr`, `fr`, `mx`) (c1, c2).
- **`planet_family_trade`**, **`help`** : `struct_init`/constructeurs et crochets de leurs descripteurs (c1, c2).
- **`wormhole`**, **`slipway`** (avec `slipway_display`) : structs `mz`, `jc`, crochets statiques, sites de `et`/`od` ; `pico8_jc_too_far_t` dans `slipway.h` (c2).
- **`research_icon`** : l'appel de `dl` sur la vue (c2). **`ke`**, **`planet`** : `ke:ks` typé (c2).
- **Aucun module nouveau**, aucun symbole nouveau hors de `DATA_ENGINE_SRCS` nommé depuis cette liste (H2) : le `Makefile` ne change pas.

### Composants (G)

Forme retenue (fragment de décision, pas un code à recopier) :

```c
/* pico8_particle.h */
/* What `ei`, `ez`, `f_` and `g_:mm` read of their host, the particle
 * emitter's own fields. `qy` is NULL until `f_`, then owned by the host and
 * never freed (TROU-7). `pn`, `du`, `ki`, `pk` are read by `g_:mm` only: a
 * `ga` has none, reads none, and holds 0 there. */
typedef struct pico8_emitter {
    pico8_fixed_t pn;
    pico8_fixed_t du;
    pico8_fixed_t py;
    pico8_fixed_t ki;
    pico8_fixed_t pk;
    int pk_is_bool; /* 1: `pk` holds a boolean (0/1), 0: a fixed */
    pico8_fixed_t fc;
    const pico8_int_seq_t *k_;
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
    pico8_list_t *qy;
} pico8_emitter_t;

/* pico8_probe.h */
/* What `bv`, `dl` and `d_` read and write of their host, the probe's own
 * fields. `et` is borrowed; with `has_et`, a NULL `et` is `false` (a `jc`
 * keeps `has_et == (et != NULL)`: its NULL is nil). `dq` NULL is nil. */
typedef struct pico8_probe {
    pico8_struct_header_t *et;
    int has_et;
    pico8_fixed_t dw;
    pico8_fixed_t od;
    int has_od;
    const char *dq;
    pico8_fixed_t fp;
    int fp_is_bool; /* 1: `fp` holds a boolean (0/1), 0: a fixed */
    pico8_fixed_t ik;
    int has_ik;
} pico8_probe_t;
```

- **Qui porte quoi** (règle 2 : le groupe apparaît dans deux branches distinctes ou plus) :

  | Composant | Hôtes (struct qui porte le pointeur) | Descripteurs qui posent le crochet | Branches |
  |---|---|---|---|
  | `pico8_emitter_t` | `pico8_g__t` (donc `fv`), `pico8_hj_t` (donc `pm`, `mx`), `pico8_ga_t` | `g_`, `fv`, `hj`, `pm`, `mx`, `ga` (6) | `g_` ; `jo`→`qh`→`hj` ; `jo`→`ga` |
  | `pico8_probe_t` | `pico8_g__t` (donc `fv`), `pico8_qh_t` (donc `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`), `pico8_mz_t`, `pico8_jc_t` | `g_`, `fv`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`, `mz`, `jc` (12) | `g_` ; `jo`→`qh` ; `mz` ; `jc` |

  La vue de l'icône (`pico8_ie_ke_t`) n'est pas un hôte : elle garde sa forme (D.11e) et passe un composant local à `pico8_dl_show`.
- **Contenu par hôte** (ce que chaque lecteur par nom répond aujourd'hui, reproduit par les `has_*`/`*_is_bool` posés au `struct_init`) : `g_` : `dq`, `od` (toujours), `fp` booléen, `dw`, `et`/`has_et`, pas de `ik` ; `pk` fixe ; les 12 champs d'émetteur. `qh` : `dq`, `od` (toujours), `fp` fixe, `dw`, `et`/`has_et`, pas de `ik`. `mz` : `dw`, `dq`, `ik` (présent), `od` (toujours), `fp` fixe, `et`/`has_et`. `jc` : `fp` booléen, `ik` (présent), `dw`, `od`/`has_od` (absent jusqu'au premier `hq`), `et` (NULL = nil), pas de `dq`. `hj` : 12 champs d'émetteur, `pk_is_bool` de `pico8_X_start`. `ga` : `fc`, `k_`, `qo`, `ro`, `qg`, `q_`, `py`, `qy` ; `pn`, `du`, `ki`, `pk` à 0, jamais lus (son `mm` est `ga:mm`).
- **Un seul émetteur, pas deux** (G) : la ligne et l'étude nomment un groupe (`ei`, `ez`, `f_`, `g__mm`) ; `ga` en porte la partie commune plus `py`, qu'il lit dans son propre `mm`. Rejeté : un second composant `{pn, du, ki, pk}` pour `g_:mm` seul (il apparaît aussi dans deux branches, `g_` et `hj`, mais scinder le groupe de l'étude en deux composants ajoute un crochet pour quatre champs, sans lecteur de plus) ; laisser `py` de `ga` dans `ga` (même nom, deux places pour un même champ du groupe).
- **Noms** : `pico8_emitter_t`/`pico8_probe_t`, membres `emitter`/`probe` et stockages `emitter_own`/`probe_own` (« never a field by name », précédent `kw_own`), crochets `emitter_of`/`probe_of`. Aucun ne correspond à un motif figé (`par-nom`, `spec`, `overrides`). Rejeté : `em` (nom minifié du cart : `jo.em`, glosé), `pb` (illisible).

### Porté par pointeur (G)

- Chaque hôte porte `pico8_X_t *X` (le pointeur de la règle 2) et son stockage `pico8_X_t X_own` ; le `struct_init` de l'hôte pose `X = &X_own` puis les défauts. Aucune instance d'hôte n'est copiée (vérifié), et un contrat de struct le dit.
- **Rejetés** : un composant alloué à part (`malloc`, jamais libéré puisque l'hôte ne l'est pas : une fuite TROU-7 de plus par entité, les comptes LeakSanitizer des notes de D.7a/D.7b bougent, et aucun lecteur ne partage de composant aujourd'hui) ; un composant enchâssé par valeur sans pointeur (ce n'est pas la règle 2, « porté par pointeur », et la phase 2 en aura besoin pour les pools de Plan 2 §8) ; les pointeurs de composant dans `pico8_ke_t` (change la disposition des ~30 structs d'entité et anticipe le noyau de la phase 2 hors des deux groupes).

### Crochets `emitter_of` / `probe_of` (G)

```c
/* pico8_struct.h, dans struct pico8_struct_class, après `parent` (dernier membre à 2abe98e) */
    /* The class's particle emitter / probe component, or NULL member: the
     * class carries none. A class that sets one embeds pico8_ke_t first.
     * Only ever called as `e->cls->probe_of(e)`. */
    struct pico8_emitter *(*emitter_of)(void *self);
    struct pico8_probe *(*probe_of)(void *self);
```

- **Un crochet par composant**, posé dans chaque descripteur d'hôte, le descendant nommant celui de sa parente (précédent : les dispatchers hérités de D.10a) : `fv` nomme `pico8_g__emitter_of`/`pico8_g__probe_of` ; `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` nomment `pico8_qh_probe_of` ; `pm`, `mx` nomment `pico8_hj_emitter_of`.
- **Cast dans le crochet** : `return ((pico8_hj_t *)self)->emitter;`. Le crochet n'est atteint que par `e->cls->X_of(e)` : la table qui le tient est le test de classe (règle des adaptateurs de D.10a, dont `pico8_hj_kj_dispatch`). Exception déclarée à la règle « cast précédé d'un test dans la même fonction », limitée aux six fonctions-crochets ; le vérificateur contrôle par grep que chaque appel d'un crochet a la forme `e->cls->X_of(e)` avec le même `e`.
- **Consommateurs** : `bv`, `dl`, `d_` (entrées partagées de dix ou onze tables), `g_:mm` (cinq tables), `f_`/`ez`/`ei` (appelés sur l'en-tête), et, plus tard, D.11e (voir « D.11e »).
- **Rejetés** : des adaptateurs par racine (`pico8_g__hq_dispatch`, `pico8_qh_hq_dispatch`…) avec des fonctions cœur à paramètres typés (changent 36 entrées de tables, les trois prédicats de porteurs de `fo` qui comparent `cls->hq == pico8_bv_dispatch`, et les signatures publiques qu'appellent 13 TU) ; une fonction `pico8_probe_of(e)` à tests de lignée (un `switch` de classes pour trouver le composant, là où la table peut le dire) ; des accesseurs par champ (`get_dw`…), forme de Q6 pour C, que Q7 a écartée pour M (« environ 25 accesseurs jetables »).

### Champs d'hôte et de cible (G)

Les fonctions partagées touchent aussi des champs que l'hôte tient pour lui-même, ou que la cible tient. Ils ne vont pas dans un composant (ce ne sont pas des champs du groupe : `jo.ra`/`jo.p` sont la position de toute planète). Ils sont atteints par **cast vérifié sur l'ensemble fermé que le cart fixe** (forme A de D.11a : `pico8_jo_is_family` ou une classe exacte ; ici aussi la lignée `g_`), dans des aides statiques de leur module, chacune finissant par un `assert` dont le message nomme le contrat (« une classe hors de celles dont le `hq`/`ey`/`kj` lance cette fonction ») :

| Aide (module) | Champ | Ensemble fermé | Branches |
|---|---|---|---|
| `emitter_host_fa_ra` (`particle.c`) | R `fa`, `ra` | hôtes de `ei` | lignée `g_` (`pico8_g__of`) → `g_->fa`, `g_->ra` ; famille `jo` (`hj`, `pm`, `mx`, `ga`) → `jo->fa`, `jo->ra` |
| `probe_host_set_ra_p` (`probe.c`) | W `ra`, `p` | hôtes de `bv` | lignée `g_` ; famille `jo` (lignée `qh`) ; `mz` exacte |
| `probe_host_raise_p_` (`probe.c`) | W `p_`, `has_p_` | hôtes de `d_` | lignée `g_` ; famille `jo` ; `jc` exacte ; `mz` exacte |
| `probe_et_hang_fo` (`probe.c`) | W `fo` de la cible | cibles de `bv` | famille `jo` ; `jc` exacte ; `&g_bv_et` ; chaque branche garde l'assert de porteur de l'écrivain d'aujourd'hui |
| `probe_et_qs` (`probe.c`) | R `qs` de la cible | cibles de `dl` qui répondent | `mz.jj` exacte ; `jc_too_far` exacte ; toute autre cible : absent (NULL), comme aujourd'hui |

- **`ke` des hôtes** : `me` (`dl`) et `t` (`g_:mm`) sont lus par `(const pico8_ke_t *)self` après le test du crochet non NULL : une classe qui pose un crochet enchâsse `pico8_ke_t` en premier (contrat du crochet ; tous les hôtes le font : `g_`, `jo`, `mz`, `jc`) ; c'est le cast que `pico8_ke_ks` fait déjà sur tout en-tête.
- **`pico8_g__of(e)`** (`probe.c/.h`, public) : `e` casté en `pico8_g__t *` si sa classe est `g_` ou descend de `g_`, NULL sinon (et pour NULL) ; la boucle remonte `cls = cls->parent != NULL ? cls->parent() : NULL` jusqu'à `&g_g__struct_class`, exactement comme `pico8_qh_of` (`planet_family.c:1006`) : aucun cast de classe, le cast final de l'en-tête suit le test dans la même fonction ; forme de Q5, non `const` (les aides écrivent). `fv` y répond par son `.parent = pico8_g__struct_class` (`help.c:194`), sans que `probe.c` nomme un symbole de `help.c`.
- **Pourquoi pas les accesseurs de D.11e** : ce sont des champs transversaux (classe C, Q6), que D.11e type pour **tous** les descripteurs qui y répondent ; en poser une partie ici donnerait des accesseurs à « NULL = absent » faux pour les classes pas encore servies. Les ensembles ci-dessus sont ceux des hôtes que le cart fait atteindre, prouvés par grep des tables.
- **Pas un registre** : aucune de ces aides ne passe d'un nom à un membre ; chacune est une suite de tests de famille ou de classe, chacun suivi du cast qu'il vérifie (précédent `pico8_jc_end_p`, D.11a).
- **Rejetés** : des pointeurs vers les membres de l'hôte dans le composant (alias internes, faux après toute copie) ; passer `fa`/`ra`/`me` en paramètres (change les signatures publiques et 13 TU) ; un `pico8_vec_t **` pour `ra` (impossible sans cast : `g_.ra` est `const pico8_vec_t *`, `jo.ra` ne l'est pas).

### `ei` sur un non-émetteur (G)

Aujourd'hui `ei` sur une classe sans `qy` lit ses champs « absents » (0), ne dessine rien et tire un `rnd()` dans la boucle de `ez`. Aucun chemin du jeu ne le fait (les quatre appelants et la table de `g_` sont des hôtes, grep). Retenu : `ei`, comme `f_`, **s'arrête** sur une classe sans émetteur (`assert` sur le crochet, message nommant le contrat). Le demi-cas de `test_particle_takes_the_help_palette` (`test_pico8_fv.c:501-505`, `ei` sur un bouton) devient illégal : **retiré, listé**. `ez` garde sa tolérance (`qy` NULL, boucle intacte) : son contrat « une classe sans `mm` n'ajoute rien » est celui du cart. `g_:mm` garde `self == NULL` → champs à 0 (`test_g__closed` l'appelle sur NULL). Rejeté : un `ei` sans effet sur un non-émetteur (changerait silencieusement le tirage `rnd()` au lieu de s'arrêter).

### `dl` sur la vue de l'icône (G)

```c
/* pico8_probe.h */
/* The tooltip `dl` draws for a probe host: `self` must carry a probe. */
void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self);
/* The same, for a caller that is not a host: `me` NULL is nil, `pb` is read
 * during the call only. */
void pico8_dl_show(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *me, const pico8_probe_t *pb);
```

`pico8_dl` lit `me` et le composant, puis appelle `pico8_dl_show` (une seule copie du corps). L'icône remplit un composant local `{0}` depuis sa vue (`has_od`, `od` ; `et` absent) et appelle `pico8_dl_show(gfx, mem, view.me, &pb)`. Rejeté : donner à la vue un crochet et un composant (la vue n'est pas `ke`, et D.11e la fait disparaître) ; garder la vue lue par nom (laisserait `:255`, `:279` comptées).

### `ke:ks` typé (G, site G `ke.c:43`)

`void pico8_ke_ks(pico8_struct_header_t *self, const char *me)` : `me` NULL est nil, emprunté ; `t` remis à 0. Dans c2, avec `d_`, pour ne jamais construire une valeur le temps d'un commit. Les cas de mort de `test_ke_ks_stops_off_row` (fixe, vrai, faux, faux à zéro, fixe à zéro, struct, chaîne NULL) sont inexprimables (le type interdit les six premiers ; « chaîne NULL » est désormais nil) : le test est **retiré, listé** ; `KE_ME_HASH` inchangée (le cas nil plie `fnv(h, 0u)` comme avant).

### Consommateurs à valeur (G)

Restent des API à valeur (Q12), non retypées : `pico8_rn` (`ik`, 57), `pico8_value_tostr` (`od..'$'`), le retour de `pico8_d_`, `pico8_g__et`/`pico8_qh_et`/`pico8_mz_et`/`pico8_hj_pk` (lisent désormais le composant), `pico8_ij` et les éléments de `cg` lus par `bv` (listes, D.12). Le site lit le membre typé et construit, à l'unique appel, la valeur que le lecteur par nom lui passait, par le constructeur existant. Chaque ligne qui gagne `valeur` pour cela est listée au message. Aucune fonction de conversion n'est ajoutée.

### Ce que D.11e, D.11d et D.12b trouvent (G)

- **D.11e, `get_dw`** : l'ensemble de ses descripteurs (Annexe C de D.11b : les 8 de la lignée `qh`, `g_`, `fv`, `jc`, `mz` — 12) est **exactement** celui des porteurs de `probe_of`. Le site `planet_display.c` (`dw` sur `jo.fo`) peut lire `fo->cls->probe_of(fo)->dw` derrière `probe_of != NULL` : l'accesseur `get_dw` n'a plus d'objet.
- **D.11e, `set_fo`** : `probe_et_hang_fo` écrit déjà `fo` typé sur trois des quatre familles de cibles (famille `jo`, `jc`, `bv_et`). Le site C `slipway.c:263` (`jc:hq`) vise planète, `jc` ou `jc_too_far` : D.11e peut rendre l'aide publique et lui ajouter la branche `jc_too_far` (struct exposée par D.11c) au lieu de poser un `set_fo` sur 13 descripteurs. Recommandation, pas décision : D.11e tranche.
- **D.11e, lecteurs et écrivains par nom** des hôtes : D.11c les fait lire et écrire le composant (lignes comptées modifiées, listées) ; D.11e les supprime. Les vues `bv_et` (son `fo` écrit typé par `bv`) et `jc_too_far` (typedef dans `slipway.h`) restent des structs-vues pour D.11e.
- **D.11e, `get_ra`/`get_p`/`get_fa`/`get_p_`** : non posés ici ; les aides d'hôte de D.11c restent privées à `probe.c`/`particle.c` et peuvent être remplacées par ces accesseurs sans rien changer d'autre.
- **Prédicats de porteurs** (`cls->hq == pico8_bv_dispatch …`) : inchangés ; D.11e peut les réécrire sur `probe_of` s'il le veut (ensembles à garder exacts : `mz` n'accroche jamais une planète).
- **D.11d** : les T1 qui liront un membre déplacé typé écriront `->probe->dw`, `->emitter->qy` ; `test_pico8_jc.c:827` le fait déjà.
- **D.12b** : `qy` (L2) vit dans `pico8_emitter_t` ; son type change là, pas sa place.

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la lecture par nom est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours ; macro, `typedef`, constante, `enum`, pointeur ou fonction enveloppe qui renomme un identifiant vivant, ou un alias ; un registre nouveau (tableau, `switch` ou boucle qui passe d'un nom à un membre) ; un cast d'en-tête sans test de lignée, de famille, de classe ou (pour les seuls six crochets) sans passer par la table ; une cale valeur ↔ typé (fonction de conversion, ou lecture par nom déplacée dans une aide non comptée — y compris vers `pico8_entity_geom_field_*`, dont les appels ne sont pas comptés) ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; supprimer un commentaire qui ne porte pas sur une construction retirée ou un contrat devenu faux ; tout changement d'une constante de hachage.
- **Classification exigée**, au message de chaque commit, pour chaque ligne `par-nom` retirée (recette : `git diff -U0` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) :
  - **(a1) site** : lecture ou écriture par nom remplacée par l'accès typé ;
  - **(a2) construction supprimée** : définition d'aide sans appelant ;
  - **(b)** : signature, déclaration ou variable dont le type change — attendu 1 : `ke.c:43`, où l'écriture typée remplace l'appel d'aide dans `pico8_ke_ks` dont le paramètre passe de `pico8_value_t` à `const char *` ; à classer (a1) ou (b) selon la lecture, déclaré ;
  - **(c)** : même donnée, autre nom, **attendu 0**.
- **Attendu, par chunk (`src/`, lignes, numéros de `2abe98e`, identiques à `e2e5da0` : `probe.c`, `particle.c`, `ke.c` inchangés par D.11b)** :
  - c1, 8 lignes : (a1) `particle.c:65`, `probe.c:395` (2) ; (a2) `particle.c:18`, `:26`, `:34`, `:43`, `probe.c:129`, `:131` (6).
  - c2, 17 lignes : (a1) `probe.c:185`, `:191`, `:227`, `:232`, `:255`, `:263`, `:268`, `:279`, `:317`, `:322`, `:337`, `:342`, `:350`, `:359` (14) ; (a2) `probe.c:123`, `ke.c:27` (2) ; (a1)/(b) `ke.c:43` (1).
  - `tests/` : aucune ligne comptée retirée ni ajoutée attendue (le cas structurel n'appelle aucun lecteur par nom ; les cas retirés n'en portent pas).
- **Lignes comptées modifiées, non retirées** (déclarées au message, fichier:ligne) : écrivains `probe.c:584` (`qy`), `:599` (`et`), `planet_family.c:157` (`et`), `:494` (`qy` de `hj`), `:738` (`qy` de `ga`), `wormhole.c:393` (`et`), et le site C `slipway.c:263` (receveur seulement). Chacune garde son unique occurrence.
- **Mesure exigée**, avant et après chaque commit, par catégorie : les **lignes** (`make data-residue`, `--detail` par fichier) **et les occurrences** (recette figée de D.8, `$S` : dossier du scratchpad) :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur type-tag ref-audit par-nom; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  ```

  Mesuré sur `2abe98e` : `valeur` 4170, `type-tag` 544, `ref-audit` 45, `par-nom` 999 (`src/` 377, `tests/` 622). Le détail par fichier (lignes et occurrences) de chaque fichier du diff est donné avant et après.
- **Contrôle Δlignes/Δoccurrences** : chacune des 25 lignes retirées porte **une** occurrence (vérifié à `e2e5da0`, lignes identiques à `2abe98e`) : attendu Δlignes = Δoccurrences partout ; tout écart est justifié au message.
- **Lignes qui gagnent une catégorie**, listées par fichier et par catégorie, « aucune » compris. Attendues : `valeur` dans `probe.c` (`pico8_value_fixed` pour `rn` de `ik` et pour `tostr` de `od`, si ces lignes sont nouvelles) ; `type-tag` dans les aides d'hôte si un assert de tag y est écrit ; aucune autre ; chacune déclarée. Baisses attendues, rapportées : `valeur` (`particle.c` : les quatre aides et `p_value` ; `probe.c` : `me_val`, `et_val`, `od_val`, `dq_val`, `fp_val`, `ik_val` ; `ke.c` : `ke_take_string` ; `planet.c` : `ks_arg`, `"ly"` ; TU de `ke:ks`), `ref-audit` (`particle.c:35`, et son inclusion si plus rien ne l'utilise).

### Empreinte remesurée et découpage (G)

| Chunk | `src/` | `tests/` | autres | `par-nom` `src/` retirées |
|---|---|---|---|---|
| c1 | 9 : `struct.h`, `particle.c/.h`, `probe.c/.h`, `help.c`, `planet_family.c/.h`, `planet_family_trade.c` | 2 : `test_pico8_fv`, `test_pico8_ui` | — | 8 |
| c2 | 16 : `struct.h`, `probe.c/.h`, `help.c`, `planet_family.c/.h`, `planet_family_trade.c`, `wormhole.c/.h`, `slipway.c/.h`, `slipway_display.c`, `research_icon.c`, `ke.c/.h`, `planet.c` | 4 : `test_pico8_jc`, `test_pico8_ui`, `test_pico8_g_`, `test_pico8_jo` | `docs/plan-3.md` (note) | 17 |

- c1 : 11 chemins, c2 : 21 : bien sous les ~40 de la règle. En tout **18 fichiers distincts de `src/`, 5 TU** et le plan (la ligne dit ≈ 15 : « particle, probe et les 13 fichiers des classes » ; s'ajoutent `struct.h`, les en-têtes d'hôtes, `slipway_display.c`, `research_icon.c` et le site G).
- **Pourquoi deux chunks** : chaque composant a sa preuve et ses mutations ; l'émetteur (c1) ne touche ni `mz`, ni `jc`, ni `ke`. Un seul commit (≈ 30 chemins) tiendrait sous la règle mais mêlerait deux changements de disposition de structs et un changement de signature publique (`ke:ks`) ; deux commits se prouvent séparément.
- **Pourquoi cet ordre** : c1 est le plus petit et ne change aucune signature ; c2 porte `ke:ks` et la note. L'ordre inverse marche aussi.
- Un fichier peut être touché par les deux chunks (`struct.h`, `probe.c/.h`, `help.c`, `planet_family*`, `test_pico8_ui`).

### Effet sur `make data-residue`

| Catégorie | Départ (`2abe98e`) | Après c1 | Après c2 | Pourquoi |
|---|---|---|---|---|
| `par-nom` `src/` | 317 / 377 occ. | **309** / 369 | **292** / 352 | exact : 8 puis 17 lignes, une occurrence chacune |
| `par-nom` `tests/` | 567 / 622 occ. | 567 | 567 | aucune ligne comptée touchée ; rapporté |
| `par-nom` total | 884 / 999 occ. | 876 | **859** | la ligne dit ≈ 862 : −3, le site G `ke.c:43` (−2) et le −1 de `tests/` de D.11b |
| `valeur` | 3876 / 4170 | baisse, rapportée | baisse, rapportée | variables `pico8_value_t` des sites, des aides et de `ke:ks` supprimées ; hausses déclarées |
| `type-tag` | 343 / 544 | rapporté | rapporté | les asserts de tag de `f_`/`ez`/`ei`/`bv`/`dl`/`d_`/`g_:mm` restent (`test_g__closed` passe une liste) |
| `ref-audit` | 44 / 45 | −1 ou −2 | = | `PICO8_REF_AS_LIST` de `pico8_particle_field_list`, et l'inclusion de `pico8_ref_audit.h` si plus rien ne l'utilise |

`par-nom` par fichier de `src/` (départ → c1 → c2) : `particle.c` 5 → 0 → 0, `probe.c` 40 → 37 → 22, `ke.c` 3 → 3 → 1. Tous les autres inchangés. « exact » est une égalité ; « rapporté » est mesuré au commit, écart expliqué.

### Documentation (G)

- `docs/plan-3.md` (c2) : note « Découvert à D.11c » en fin de ligne D.11c (texte en « Further Notes »), rien d'autre.
- `docs/glossaire-noms.md` : inchangé ; aucun nom minifié ne change de sens. Les noms nouveaux (`pico8_emitter_t`, `pico8_probe_t`, `emitter_of`, `probe_of`, `pico8_g__of`, `pico8_dl_show`) sont des mots anglais ou composés de noms déjà glosés.
- `docs/trous-de-portage.md` : inchangé. TROU-7 reste aux sites (`qy` de `f_`, `p` de `bv`, particules de `mm`), TROU-21 à `ga:kj`. Aucun écart port/cart n'est créé ; l'arrêt de `ei` sur un non-émetteur n'est atteignable par aucun chemin du cart.
- Les PRD livrés, `docs/plan.md` et `docs/plan-2.md` sont historiques et ne sont pas touchés.

### Ce qui ne change pas

- `tools/`, le `Makefile`, `tests/e2e/`, tout `*.sha256`, tout fichier d'événements ; aucun e2e ajouté.
- Les entrées des tables (`pico8_bv_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch`, `pico8_ei_dispatch`, `pico8_g__mm`, `pico8_ga_mm`) et les signatures publiques de `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_g__mm`, `pico8_bv`, `pico8_dl`, `pico8_d_` ; les prédicats de porteurs de `fo` ; l'enchâssement parent → fille de toutes les structs.
- Les lecteurs `field`, écrivains `set_field`, `kind_field`, aides `*_take_*` des écrivains et `pico8_entity_get_field`/`set_field` (D.11e) — seuls leurs corps suivent le déplacement des membres ; les sites C, dont `slipway.c:263` et `planet_display.c` (`dw`), et `jc.mt` (D.11e).
- L'ordre des appels et des tirages aléatoires (`bv` : `ra`, `p`, puis `ij` ; `g_:mm` : `rnd(pn)` puis `rnd(0.5)` ; `ez` tire même sans `qy`), les fuites (TROU-7), les arrêts d'aujourd'hui (sauf `ei` sur un non-émetteur, déclaré).

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `planet_family.h`, `wormhole.h` et `slipway.h` peuvent inclure `pico8_probe.h` sans cycle (constaté : la fermeture des inclusions de `pico8_probe.h` est `coverage`, `economy`, `economy_formulas`, `fixed`, `game_data`, `geom`, `gfx`, `ke`, `list`, `map_tables`, `mem`, `particle`, `struct`, `type_tag`, `value`, `vec`) ; `particle.c` inclut `pico8_probe.h` et `pico8_planet.h` (gardes d'inclusion). Sinon, déclaration anticipée `struct pico8_probe`, déclarée.
- **H2** (leçon de D.11a) : aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste. Constaté par grep à `2abe98e` (liste inchangée) : `particle.c` nommera `pico8_g__of` (`probe.c`) et `pico8_jo_is_family` (`planet.c`) ; `probe.c` nommera `pico8_jo_is_family`, `pico8_jc_struct_class`, `pico8_jc_too_far_struct_class` (`slipway.c`), `pico8_mz_struct_class`, `pico8_mz_jj_struct_class` (`wormhole.c`) — tous dans la liste ; `planet_family.c` ses propres crochets ; `help.c` (hors liste) nomme `pico8_g__emitter_of`/`pico8_g__probe_of` (`probe.c`) ; `planet_family_trade.c` (hors liste) `pico8_qh_probe_of`/`pico8_hj_emitter_of` (`planet_family.c`) ; `research_icon.c` (hors liste) `pico8_dl_show`. Si un lien casse malgré tout : arrêt, PRD à reprendre ; **le `Makefile` ne change pas**.
- **H3** : aucune instance d'hôte n'est construite par valeur, copiée ou `memcpy`-ée dans `src/` ni `tests/` (grep à `2abe98e`) : le pointeur vers `X_own` reste valide. `test_pico8_planet_kj_dispatch` copie une **table de classe**, pas une instance : la copie emporte `emitter_of`/`probe_of` (NULL pour `jo`).
- **H4** : toutes les tables de classe (`src/`, `tests/`) sont initialisées par membres désignés (c'est ce qui a permis à D.11b d'ajouter `parent` en dernier membre sans toucher les autres tables) : les deux crochets sont NULL partout où ils ne sont pas posés (grep à revérifier au commit : aucun initialiseur positionnel de `pico8_struct_class_t`).
- **H5** : aucun comportement du binaire de rejeu ne change : 26 e2e conformes ; `class-coverage-net` identique (les constructeurs gardent leur ordre d'appel et leur `pico8_class_coverage_record`).
- **H6** : aucune constante `*_HASH`/`identity[]` ne change ; le cas structurel ne plie rien ; les cas retirés s'exécutaient dans un fils (`must_abort`) ou ne pliaient rien de ce qu'ils vérifiaient (demi-cas du bouton : `screen_hash() == before`, non plié).
- **H7** : `pico8_sfx` (appelé par `pico8_rn`) n'est observé par aucun TU pour `ik` ; une mutation sur `has_ik` peut ne rien rougir : trou à justifier (audio hors périmètre), ou couvert par `test_pico8_mz`/`test_pico8_jc` s'ils observent le numéro de sfx.
- **H8** : `make value-members` (`entity_value_members.sh --check`) ne voit pas `pico8_emitter_t`/`pico8_probe_t` comme structs d'entité (pas d'en-tête `pico8_struct_header_t` en premier membre) et ne rougit pas ; `grep -rn offsetof src/` reste vide (aucun `offsetof` pour trouver un composant).

## Implementation Chunks

Deux commits, dans cet ordre. Chacun est vert seul (`make test`, `make e2e`, `make data-guard`, `make class-coverage-net`, `make sanitize-net`), avec ses comptes, sa classification, ses lignes gagnées et modifiées, son `gcov` et ses mutations :

1. **c1 — émetteur.**
   - `pico8_emitter_t` ; crochet `emitter_of` dans `pico8_struct_class_t`, posé dans 6 descripteurs.
   - `g_`, `hj`, `ga` portent `emitter`/`emitter_own` ; `struct_init` et constructeurs de `g_`, `fv`, `hj`, `pm`, `mx`, `ga` (défauts, clés `g_`, clés `pq` de `ga`) ; lecteurs, écrivains et `pico8_hj_pk` sur le composant.
   - `f_`, `ez`, `ei`, `g_:mm`, `ga:mm`, `ga:kj` ; aide `emitter_host_fa_ra` ; `pico8_g__of`.
   - Aides mortes : quatre de `particle.c`, `pico8_probe_field_truthy`.
   - TU : `test_pico8_fv` (demi-cas retiré), `test_pico8_ui` (cas structurel, partie émetteur).
   - 11 chemins, `par-nom` `src/` −8.
2. **c2 — sonde et `ke:ks`.**
   - `pico8_probe_t` ; crochet `probe_of`, posé dans 12 descripteurs.
   - `g_`, `qh`, `mz`, `jc` portent `probe`/`probe_own` ; `struct_init` de `g_`, `qh`, filles (`od`, `fp`, `dw` de `nl`, `os`, `hj`, `nr`, `fr`, `mx`), `mz`, `jc` ; lecteurs, écrivains et `pico8_g__et`/`pico8_qh_et`/`pico8_mz_et` sur le composant ; `mz:hq`, `jc:hq`, `slipway_display.c`.
   - `bv`, `dl`, `d_` ; aides `probe_host_set_ra_p`, `probe_host_raise_p_`, `probe_et_hang_fo`, `probe_et_qs` ; `pico8_dl_show` et l'appel de l'icône ; `pico8_jc_too_far_t` dans `slipway.h`.
   - `pico8_ke_ks(self, const char *)` et ses appelants.
   - Aides mortes : `pico8_probe_field_fixed`, `ke_take_string`.
   - TU : `test_pico8_jc` (`dw`, `ke:ks`), `test_pico8_ui` (`ke:ks`, cas de mort retirés, cas structurel étendu), `test_pico8_g_`, `test_pico8_jo` (`ke:ks`).
   - Note « Découvert à D.11c » ; balayage ASan+UBSan des TU.
   - 21 chemins, `par-nom` `src/` −17.

Ordre conseillé dans chaque commit :
1. mesure de départ (lignes et occurrences par fichier) ;
2. en-têtes, puis `.c` de `src/`, puis TU ;
3. `make test`, `make e2e`, `make class-coverage-net`, `make sanitize-net` ;
4. `gcov` et mutations (copie jetable, restauration par `cp`) ;
5. mesure d'arrivée, classification, lignes gagnées et modifiées, greps de la « Sortie » ;
6. docs, message.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (valeurs, présences, identités, effets d'un appel), pas la présence d'une ligne. D.11c change **où vivent** des champs et **comment** le code partagé les atteint, pas ce qu'il lit. La preuve :
- chaque TU garde son intention et sa constante (les TU de chaque hôte lisent par nom, sur des instances construites, chacun des champs déplacés : un défaut mal placé les rougit) ;
- les 26 e2e restent conformes, `class-coverage-net` identique, `sanitize-net` propre ;
- chaque ligne nouvelle de `src/` est exécutée par un TU (`gcov`) ou justifiée, et mord sous mutation ;
- un cas structurel prouve qui porte quel composant.

### TU touchés (G)

1. **`test_pico8_ui` (c1, étendu en c2)** : un cas nouveau, sans constante, dans son propre fils (`in_fresh_child`), sur les 24 entités que le TU construit déjà (`e[0..23]`, l. 1119-1142) : `emitter_of != NULL` exactement pour `hj`, `pm`, `ga`, `mx`, `g_`, `fv` ; `probe_of != NULL` exactement (c2) pour les 8 de la lignée `qh`, `g_`, `fv`, `jc`, `mz` ; pour deux instances fraîches d'une même classe porteuse, les deux composants sont distincts et chacun est dans son hôte (`(char *)comp >= (char *)e && (char *)comp < (char *)e + sizeof(struct de l'hôte)`, écrit par hôte, sans table de noms). Aucune lecture par nom : `par-nom` `tests/` inchangé.
2. **`test_pico8_fv` (c1)** : `test_particle_takes_the_help_palette` perd son demi-cas « `ei` sur un bouton » (déclarations `button` et `before`, l. 495-496 ; l. 501-505 : le choix du bouton, son `assert(!has(button, "k_") …)`, l'appel et l'`assert(screen_hash() == before)`), **retiré, listé** : un non-émetteur n'est plus un receveur légal de `ei`. La seconde moitié (la palette de l'aide) reste telle quelle. `test_g__closed` inchangé (liste → arrêt, NULL → rien, pour les neuf fonctions).
3. **`test_pico8_ui` (c2)** : appels de `ke:ks` typés (`pico8_value_string(x)` → `x`, `pico8_value_ref(NULL)` → `NULL`) ; `test_ke_ks_stops_off_row`, ses sept aides (`ks_fixed` … `ks_null_string`) et son `in_fresh_child` **retirés, listés** (inexprimables) ; `KE_ME_HASH` inchangée. `test_ke_closed` garde `ks_on_a_table` (une liste en `self`) et `pico8_ke_ks(NULL, "ph")`.
4. **`test_pico8_jc` (c2)** : `:827` écrit `->probe->dw` ; `:1441` `ke:ks` typé. **`test_pico8_g_`** (`:495-496`) et **`test_pico8_jo`** (`:463`) : `ke:ks` typé.
5. **Pas de pont**, aucun autre TU touché. Un TU qui casse sans nommer un symbole changé est un **bug de D.11c**, pas un TU à adapter : il part à `help-me-debug`. Avant chaque commit, l'implémenteur cherche dans `tests/` tout cas de mort ou toute constante qui dépend d'un contrat changé (`pico8_ei`, `pico8_f_`, `pico8_ke_ks`, `pico8_dl`, les `struct_init` d'hôtes) — leçon de `fm_hy_on_provisional` à D.11a.
6. **Constantes** : aucune `*_HASH` ni `identity[]` ne change. Une constante qui rougit est un échec, jamais une mise à jour.
7. **§6** : aucun des trois tests n'est touché ; `test_pico8_planet_gh_row_lq` (lié sur `DATA_ENGINE_SRCS`, sous ASan) et `test_pico8_ga_export_panel_by` (construit des `ga`) doivent rester verts.
8. **Test ajouté** : aucun fichier nouveau. Au-delà du cas structurel, une assertion nouvelle n'est admise que pour couvrir une ligne nouvelle de `src/` que `gcov` montre non exécutée, dans un TU existant de la classe, listée au message avec la ligne qu'elle couvre.
9. **Prior art** : stockage propre pointé : `jo.kw`/`kw_own`, `jc.hg`/`hg_own` ; crochet de table hérité par nom : dispatchers de D.10a, `has_hb` ; cast vérifié sur famille fixée par le cart : `pico8_jc_end_p` (D.11a), `pico8_qh_of` (D.11b) ; vérité d'un champ booléen ou fixe : `pico8_hj_pk`, `hf_is_bool` ; tests de mort dans un fils : `must_abort`.

### Preuve des tests (§7 : `gcov` et mutations)

`gcov` compile les TU des classes touchées en `--coverage`, dans une copie jetable du scratchpad. Il doit montrer exécutés :
- c1 : `f_`, `ez` (avec et sans `qy`), `ei` (chaque branche de `emitter_host_fa_ra` : lignée `g_`, famille `jo`), `g_:mm` (`pk` booléen et fixe, `self` NULL), chaque crochet d'émetteur, chaque `struct_init` modifié ;
- c2 : `bv` (chaque branche de `probe_host_set_ra_p` et de `probe_et_hang_fo`), `dl` (avec et sans `et`, `qs` de `mz.jj` et de `jc_too_far`, `od`), `pico8_dl_show` depuis l'icône, `d_` (chaque branche de `probe_host_raise_p_`, `fp` booléen et fixe, `ik` présent et absent, `dq` NULL et posé), chaque crochet de sonde, `ke:ks`.

Une ligne non exécutée est couverte par une assertion (règle 8) ou justifiée au message (en particulier les branches `assert` de fin d'aide, atteintes seulement par un appel illégal).

Mutations **une à la fois, dans l'arbre de travail**. Le fichier est sauvegardé par `cp` avant et restauré par `cp` après (jamais `git checkout`/`stash`). Chaque TU est lancé exactement comme le `Makefile` le lance ; un TU lié sous ASan l'est avec `ASAN_OPTIONS=detect_leaks=0`. Chaque mutation doit rougir au moins un TU, nommé au message :

- c1 :
  - **M1** `g_:mm` prend la vérité de `pk` comme `pk != 0` pour un `pk` fixe → `test_pico8_g_` (particules pliées), `test_pico8_probe_mm_rnd_order`, `test_pico8_fv` ;
  - **M2** le descripteur de `ga` ne pose pas `emitter_of` → `test_pico8_ga` (`ga:pq` s'arrête), le cas structurel ;
  - **M3** le `struct_init` de `hj` ne recopie pas `k_` (reste NULL) → `test_pico8_hj`/`test_pico8_pm`/`test_pico8_mx` (lecture de `k_` par nom), `test_pico8_ui` ;
  - **M4** `emitter_host_fa_ra` rend un `ra` nul pour la famille `jo` → `test_pico8_ga` (pixels de particules), `test_pico8_hj` ;
  - **M5** `f_` n'écrit pas `emitter->qy` → `test_pico8_fv`, `test_pico8_hj`, `test_pico8_probe_mm_rnd_order` ;
  - **M6** `emitter_own` partagé (pointeur vers un statique) → le cas structurel.
- c2 :
  - **M7** `d_` rend `hx = fp` pour un `fp` booléen → `test_pico8_g_`/`test_pico8_jc` (horloge avancée par `fm:nx`), sinon trou justifié ;
  - **M8** `probe_et_hang_fo` saute la branche `jc` → `test_pico8_g_` (`assert(get_ref(jc, "fo") == a)`, l. 491) ;
  - **M9** le `struct_init` de `nr` ne pose pas `dw` depuis `pico8_nr_start` → `test_pico8_nr` (lecture de `dw`), `test_pico8_ui` ;
  - **M10** `probe_et_qs` ne lit pas `mz.jj` → `test_pico8_mz` (texte « -too far- » à l'écran), sinon trou justifié ;
  - **M11** `jc:hq` ne pose pas `has_et` → `test_pico8_jc` (`d_`/`dl` sur une voie) ;
  - **M12** `probe_host_raise_p_` saute la branche `mz` → `test_pico8_mz` (`mz:ey`) ;
  - **M13** `pico8_ke_ks(self, NULL)` laisse `me` inchangé → `test_pico8_ui` (`KE_ME_HASH`) ;
  - **M14** `d_` ignore `has_ik` (toujours 54) → `test_pico8_mz`/`test_pico8_jc` si le numéro de sfx est observé, sinon trou justifié (H7).
- Une mutation qui ne rougit rien est un trou : couvert par une assertion (règle 8) ou justifié au message.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit. Aucun e2e ajouté : les 26 passent par la sonde (clic dans le vide, placement de planète tenue), `jc` (voies), `mz` (trou de ver), les particules de `g_`/`hj`/`ga` ; un e2e nouveau n'ajouterait de couverture qu'à des lignes que les TU couvrent déjà.
- `make class-coverage-net` : **à chaque commit** (ligne de l'étape), identique à `tools/class_coverage_net_expected.txt`.
- `make sanitize-net` : **à chaque commit** (ligne de l'étape), sortie propre.
- **Balayage ASan+UBSan des TU** à la fin de c2 : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.7b, D.10a, D.10b, D.11a. Sortie propre.

### Vérification, budget et tableau « Runs »

**Budget : environ 35 minutes de temps réel pour l'étape** (c1 ≈ 14, c2 ≈ 21), mesures et filets compris ; plafond 60 minutes. Base mesurée : D.11a ≈ 35 min pour ≈ 48 chemins, **D.11b ≈ 15 min pour 22 chemins** (≈ 0,7 min par chemin) ; D.11c touche 32 chemins (≈ 22 min au même rythme), mais déplace des membres dans 13 structs d'hôtes (tous les `struct_init`, lecteurs et écrivains qui les nomment), ajoute deux crochets et une signature publique (`ke:ks`), et exige `class-coverage-net` et `sanitize-net` à chaque commit, ce que D.11b n'exigeait pas : d'où ≈ +50 %. La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make test` | de l'ordre de la minute (recompilation large : `pico8_struct.h` change à chaque chunk) |
| `make e2e` | ~1 s |
| `make class-coverage-net`, `make sanitize-net` | quelques dizaines de secondes chacun |
| `make data-residue`, recette, classification, greps | < 1 min |
| `gcov` des TU touchés | quelques minutes par chunk |
| mutations (6 + 8, chacune un `make` ciblé du TU) | < 12 min |
| balayage ASan+UBSan des TU | quelques minutes, une fois |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été lancé. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements, et le cas nouveau n'est pas lent (24 entités construites, des comparaisons de pointeurs).

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + chunk + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | `2abe98e`, avant c1 | 0 | `par-nom 884 70` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que les chemins de la table « Empreinte ». Ni `tools/`, ni `Makefile`, ni `tests/e2e/`, ni `*.sha256`.
2. **Greps** de la « Sortie » : exactement ce qui y est écrit, après c2 ; après c1, les greps 1 (`particle.c`), 2 (`pico8_particle_field_*`, `pico8_probe_field_truthy`), 3 (émetteur) et 4 (`emitter_of`).
3. **Casts** : chaque cast d'un en-tête ou d'un `void *` vers une struct, ajouté ou gardé dans une fonction touchée, est précédé dans la même fonction d'un test de lignée, de famille ou de classe — sauf dans les six fonctions-crochets, dont chaque appel a la forme `e->cls->X_of(e)` (vérifié par grep). Les lectures `(const pico8_ke_t *)self` suivent un test de crochet non NULL.
4. **Liens** : aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole nouveau défini hors de la liste (H2) ; `make test` le prouve pour les TU de cette liste.
5. **Sites** : chaque site M et le site G de l'inventaire sont convertis ; aucun site C ni ligne de `mt` n'est converti (`slipway.c:263` ne change que de receveur ; `planet_display.c`, `dw` sur `jo.fo`, intact).
6. **Constantes** : aucune `*_HASH`/`identity[]` changée.
7. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (c) = 0 ; lignes gagnées et lignes comptées modifiées listées ; aucune cale valeur ↔ typé, aucun registre, aucun alias.
8. **Pièges** : vérité de `pk` (fixe toujours vrai), repli de `fp` (booléen → 0), `has_et` de `jc`, ordre `ra`/`p`/`ij` dans `bv`, boucle de `ez` intacte sans `qy`.
9. **Tests retirés** listés avec leur raison. Mutations, `gcov`, `class-coverage-net`, `sanitize-net` et balayage ASan+UBSan rapportés.
10. **Commentaires** : quatre sortes de `CLAUDE.md` (sens d'un nom minifié, écart connu, piège, contrat C). Les contrats nouveaux (composants, crochets, stockage propre, `pico8_g__of`, `pico8_dl_show`, `ke:ks`) disent ce que le nom ne dit pas (NULL, présence, qui possède quoi). Aucun renvoi à D.11c ni au plan dans `src/` ou `tests/`.
11. **Docs** : note « Découvert à D.11c » en fin de ligne D.11c ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `2abe98e` (ou un descendant qui ne touche que `docs/`), propre, ou si `make data-residue` n'y rend pas `par-nom` 884 (`src/` 317, `tests/` 567) et 70 fichiers sans explication, ou si une des aides que ce PRD tue (`pico8_particle_field_*`, `pico8_probe_field_fixed`/`_truthy`, `ke_take_string`) n'y existe plus, ou si `pico8_struct_class_t` n'y porte pas `parent` en pointeur d'accesseur.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change, ou si un test devrait changer d'intention pour passer.
- `FAILED:` si `make class-coverage-net` diffère de l'attendu versionné, si `make sanitize-net` n'est pas propre, ou si le balayage ASan+UBSan des TU n'est pas propre — à l'un des deux commits.
- `FAILED:` si un champ ou un site ne tombe sous aucune règle (arrêt « champ sans règle » du §7 ; en phase 2 : « groupe au classement ambigu »), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.

**Périmètre et types**

- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si un cast vers une struct n'a pas son test de lignée, de famille ou de classe dans la même fonction, hors des six crochets appelés par `e->cls->X_of(e)`.
- `FAILED:` si un fichier de `DATA_ENGINE_SRCS` nomme un symbole défini hors de cette liste, ou si un lien de TU casse et que le `Makefile` est changé pour le réparer.
- `FAILED:` si une lecture change de valeur : vérité de `pk` (un fixe est vrai), `hx` de `d_` (booléen → 0, fixe → `fp`), `ik` absent → 54, `dq` absent → `"ph"`, `od` absent → 0 (et pas de texte `$`), `et` nil ou faux, `qs` absent hors `mz.jj`/`jc_too_far`, `k_` NULL → couleur 0, `ra` NULL → vecteur nul ; ou si l'ordre des tirages `rnd()` change.
- `FAILED:` si un composant est alloué à part, partagé entre deux instances, ou si un membre du groupe reste en double dans un hôte ; si `offsetof` apparaît dans `src/`.
- `FAILED:` si une entrée de table (`pico8_bv_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch`, `pico8_ei_dispatch`, `pico8_g__mm`) ou un prédicat de porteur de `fo` change, ou si la signature publique de `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_g__mm`, `pico8_bv`, `pico8_dl` ou `pico8_d_` change.
- `FAILED:` si un site C, une ligne de `mt` ou un accesseur de Q6 (`get_*`/`set_*` de la table) est converti ou ajouté (D.11e), si un lecteur `field`, écrivain `set_field` ou `kind_field` est supprimé (D.11e), ou si une vue (`bv_et`, `jc_too_far`, `ie_ke_view`…) est supprimée.
- `FAILED:` si l'enchâssement parent → fille d'une struct change (la phase 2 n'est appliquée qu'à ces deux groupes).
- `FAILED:` si un fichier de `tools/` ou le `Makefile` change.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert.
- `FAILED:` si `par-nom` `src/` ne vaut pas 309 après c1 et 292 après c2 (`tests/` 567, total 876 puis 859), ou si le détail par fichier diffère de la table sans écart expliqué.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, construction déplacée, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, alias, registre nouveau, cale valeur ↔ typé, lecture par nom déplacée dans une aide non comptée, code déplacé hors du compteur, ou une (c).
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) des lignes `par-nom` retirées ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; la liste des lignes comptées modifiées sans être retirées ; la justification de chaque couple Δlignes > Δoccurrences. Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si un cas de test retiré n'est pas listé au commit avec la raison qui le rend inexprimable.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si le plafond de 60 minutes est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.11c », ou si elle omet l'un de ces éléments : les comptes réels, les deux composants et leurs hôtes, les crochets, le stockage propre, les aides d'hôte à ensemble fermé, `pico8_dl_show`, `ke:ks` typé, l'arrêt de `ei` sur un non-émetteur, ce que D.11e peut réutiliser (`get_dw`, `set_fo`), `qy` dans le composant pour D.12b.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Classe C et `jc.mt`** (D.11e, premier chunk), dont `slipway.c:263` (`fo` écrit par `jc:hq`, seul son receveur change ici) et `planet_display.c` (`dw` sur `jo.fo`) ; les accesseurs de Q6. **TU** au-delà des cinq qui suivent un contrat changé (D.11d : T1/T2, les deux tests §6). **Infrastructure** (D.11e : lecteurs `field`, écrivains `set_field`, `*_take_*` des écrivains, `kind_field`, `pico8_entity_get_field`/`set_field`, structs-vues).
- Les autres règles de la phase 2 : archétypes, noyau, test ponctuel d'archétype, fin de l'enchâssement ; tout autre groupe de champs.
- Retyper une API à valeur (`pico8_rn`, `pico8_value_tostr`, `pico8_d_`, `pico8_g__et`/`pico8_qh_et`/`pico8_mz_et`, `pico8_hj_pk`) : Q12, ouverte.
- Les listes (`qy`, `cg`, `hv`) : D.12. D.11c garde `qy` en `pico8_list_t *`.
- Réécrire les prédicats de porteurs de `fo` sur `probe_of` ; réécrire `pico8_jo_is_family` sur `parent`.
- `valeur`, `type-tag`, `ref-audit` au-delà de ce que D.11c retire en passant.
- `tools/`, le `Makefile`, `tests/e2e/`, `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **Composants** : `pico8_emitter_t` (un seul, `ga` y garde 0 pour les quatre champs de `g_:mm`) et `pico8_probe_t` ; hôtes `g_`/`hj`/`ga` et `g_`/`qh`/`mz`/`jc` ; `jc` compte comme quatrième branche de la sonde (`jc:ev` et `jc:ey` appellent `dl` et `d_` sur lui-même). Rejetés : deux composants d'émetteur, `py` de `ga` hors composant.
  - **Porté par pointeur** : pointeur vers un stockage dans l'hôte (`X_own`). Rejetés : `malloc` par composant, composant par valeur, pointeurs dans `ke`.
  - **Crochets** `emitter_of`/`probe_of` dans la table, hérités par nom ; entrées de tables et signatures publiques inchangées. Rejetés : adaptateurs par racine, `switch` de lignée pour le composant, accesseurs par champ.
  - **Champs d'hôte et de cible** : aides à cast vérifié sur l'ensemble fermé des hôtes et cibles que le cart fixe ; `me`/`t` par `ke` derrière le crochet. Rejetés : accesseurs partiels de Q6, paramètres, alias internes.
  - **Contrats** : `ei` et `f_` s'arrêtent sur un non-émetteur ; `ez` garde sa boucle ; `dl` a une forme typée pour la vue ; `ke:ks` prend `const char *`.
  - **Tests** : cas structurel dans `test_pico8_ui` ; demi-cas du bouton et sept cas de mort de `ke:ks` retirés ; aucun oracle tautologique.
  - **Découpage** : c1 (émetteur, 11 chemins), c2 (sonde et `ke:ks`, 21) ; la note à c2.
  - **Filets** : `class-coverage-net` et `sanitize-net` à chaque commit (ligne), balayage ASan+UBSan des TU à la fin.
- **Note du plan (G)**, à ajouter en fin de ligne D.11c (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.11c : deux chunks (c1 : N1 chemins ; c2 : N2). Composants (règle 2, Q7) : `pico8_emitter_t` `{pn, du, py, ki, pk, pk_is_bool, fc, k_, qo, ro, qg, q_, qy}` porté par `g_`, `hj`, `ga` ; `pico8_probe_t` `{et, has_et, dw, od, has_od, dq, fp, fp_is_bool, ik, has_ik}` porté par `g_`, `qh`, `mz`, `jc` ; pointeur vers un stockage dans l'hôte (`X_own`), aucune allocation ; crochets `emitter_of`/`probe_of` de `pico8_struct_class_t` (6 et 12 descripteurs), l'enchâssement inchangé. Champs d'hôte (`ra`, `p`, `p_`, `fa`) et de cible (`fo`, `qs`) par cast vérifié sur l'ensemble fermé (`pico8_g__of`, `pico8_jo_is_family`, classes exactes), `me`/`t` par `ke`. `pico8_dl_show` pour la vue de l'icône ; `pico8_ke_ks(self, const char *)` (G `ke.c:43`) ; `ei`/`f_` s'arrêtent sur un non-émetteur. `pico8_jc_too_far_t` dans `slipway.h`. Pour D.11e : `get_dw` n'a plus d'objet (ses 12 descripteurs sont les porteurs de `probe_of`), `set_fo` peut réutiliser l'écriture typée de `bv` (planète, `jc`, `bv_et`) en y ajoutant `jc_too_far`. Pour D.12b : `qy` est `emitter->qy`. `par-nom` P (`src/` S) ; `valeur` V ; résidu D R.

  N1, N2, P, S, V, R sont les valeurs réelles.
- **Le message de chaque commit** porte : le périmètre du chunk ; la classification (a1)/(a2)/(b)/(c) des lignes `par-nom` retirées ; les comptes avant et après **en lignes et en occurrences**, par catégorie et par fichier, avec les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie, par fichier (« aucune » compris) ; les lignes comptées modifiées sans être retirées ; les cas de test retirés et leur raison, les cas et assertions ajoutés et la ligne qu'ils couvrent ; les constantes (inchangées), les mutations, `gcov`, les filets ; le tableau « Runs » résumé ; les réponses aux hypothèses H et H-D11b qui le concernent.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget) : casts et crochets, liens, pièges (`pk`, `fp`, `has_et` de `jc`), anti-tassement, classification, sites laissés, constantes, cas retirés.
- **Surprises pour le pilote** :
  - **La sonde a quatre branches, pas trois** : `jc` appelle `dl` et `d_` sur lui-même (`slipway.c:455`, `:278`) et porte `dw`, `od`, `fp`, `ik`, `et` ; l'étude ne citait que `g_`, `fv`, l'arbre `qh` et `mz`.
  - **`dl` a un appelant qui n'est pas une entité** : l'icône de recherche passe sa vue de sorte (ni `ke`, ni porteur) ; d'où `pico8_dl_show`.
  - **Deux pièges de vérité Lua** dans le code partagé : un `pk` fixe est toujours vrai (les particules de `g_` tournent toujours d'un quart de tour), un `fp` booléen donne 0 à `fm:nx`.
  - **Le site G `ke.c:43`** n'est pas dans la ligne D.11c mais dans la note de D.10a : pris ici (avec `d_`, son producteur de valeur) ; il retire sept cas de mort de `test_pico8_ui` et fait −2 de plus sur `par-nom` (859 au lieu de ≈ 862, avec le −1 de `tests/` constaté à D.11b).
  - **L'empreinte passe de ≈ 15 à 18 fichiers de `src/` et 5 TU** (en-têtes d'hôtes, `struct.h`, `slipway_display.c`, `research_icon.c`, G).
  - **Q7 sans test d'archétype** : l'oracle « composant = lecteur » est tautologique une fois les lecteurs redirigés ; la garde des défauts déplacés repose sur les TU par hôte et les mutations de `struct_init`.
  - **`parent` est un pointeur d'accesseur** (note de D.11b), pas de données : `pico8_g__of` appelle `cls->parent()` ; les crochets de D.11c nomment des fonctions publiques pour la même raison (constante d'adresse dans un initialiseur statique).
  - **Ce que D.11e économise** : l'accesseur `get_dw` (12 descripteurs) et, s'il le veut, `set_fo` (13 descripteurs), soit une part notable des ~170 lignes d'initialiseurs que l'Annexe C de D.11b chiffrait.
## Annexe A — textes du plan recopiés (`docs/plan-3.md` à `2abe98e`)

Recopiés tels quels, sans retouche, par extraction de lignes (aucune saisie à la main). Les listes d'étapes livrées de 1a (l. 93-149) et de 1b (l. 169-226) ne sont pas recopiées : historique livré, aucune de leurs lignes ne lie D.11c ; les règles de ces phases le sont. La section « Phase D » est recopiée en entier : ses puces (dont la **Sortie**), la note de D.0, le paragraphe « Liste de D » avec l'empreinte, les règles communes et l'ordre, toutes les lignes D.2 à D.12 avec **toutes** leurs notes « Découvert à D.x » (D.0, D.2 à D.8, D.10a, D.10b, D.10c, D.11a, D.11b — dont sa note pour D.11d/D.11e insérée dans la ligne D.11e ; D.1, D.9, D.11c à D.12d n'en ont pas encore), l'arrêt « champ sans règle », les décisions de D.9 et les comptes au départ de D.10.

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


### §5, phase D, en entier (l. 228-280)

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
- D.11c — `par-nom`, classe M (Q7) : émetteur de particules (`ei`, `ez`, `f_`, `g__mm`) et sonde (`bv`, `dl`, `d_`) → composants portés par pointeur, règle 2 de la phase 2 appliquée à ces deux groupes seulement. Empreinte ≈ 15 (15, 0) : `particle`, `probe` et les 13 fichiers des classes qui les portent. Compte : `par-nom` ≈ 862. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : oui.
- D.11d — `par-nom`, TU : T1 (mise en état et lecture d'état) en accès aux membres typés ; T2 (tests des descripteurs, tests de mort des `assert` d'écrivain) supprimés comme tests de la couche (Q11, §6). Deux chunks d'environ 19 fichiers. Empreinte 38 (0, 38). Compte : `par-nom` ≈ 286 (l'infrastructure de `src/`). §6 : `test_pico8_planet_gh_row_lq` et `test_pico8_ga_export_panel_by` (3 lignes chacun) lisent les membres, preuve rouge. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change.
- D.11e — `par-nom`, suppression de l'infrastructure. Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`, laissé à D.11a ; accesseur typé par champ dans la table de classe, NULL = absent, Q6 ; l'oracle accesseurs = lecteurs naît et meurt dans l'étape). Découvert à D.11b (pour D.11d/D.11e) : les littéraux `s`/`od` des `pico8_X_kind_rec` doublent ceux de `pico8_X_start` ; seul `test_kind_rec_matches_the_reader` en garde l'égalité — à sa suppression, garder une comparaison rec ↔ `pico8_X_start` ou des constantes nommées partagées. Puis : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks. Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non.
- D.12a — `liste`, registres : catégorie `liste` ajoutée à `tools/data_residue.sh` dans un premier commit seul, motif `\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h` (Q10 ; la règle de D.0 fige les motifs existants, mais une catégorie nouvelle s'ajoute dans son propre commit avec son motif énoncé) ; conteneur de pointeurs (Q9) sous un nom hors de ce motif ; registres L1 (`hv`, `cv[5]`, `cg[*]`, calques de `dc`, `ui.ic`, `ui.le`, `g_ga_by_shared`) et leurs parcours. Empreinte ≈ 15 (15, 0) : `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud`. Compte : `liste` ≤ 927 moins L1, rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
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

## Annexe B — étude D.9, classe M et Q7 (`docs/prd/done/plan3-phaseD-etape9-etude.md`)

Ligne M du tableau de classement (§1.1), recopiée :

> | **M** — groupe partagé entre branches : émetteur de particules (`ei`, `ez`, `f_`, `g__mm` : `pn`, `t`, `du`, `py`, `ki`, `pk`, `qy`, `fa`, `ra`, `qo`, `qg`, `ro`, `q_`, `k_`, `fc`, sur `g_`, `fv`, `hj`, `pm`, `mx`, `ga`) et sonde (`bv`, `dl`, `d_` : `ra`, `p`, `et`, `fo`, `me`, `od`, `dq`, `fp`, `ik`, `p_`, `dw`, `qs`, sur `g_`, `fv`, les huit classes de l'arbre `qh`, `mz` pour `dl`) | **aucune forme de D** : c'est la règle 2 de la phase 2 (composant). Voir Q7 | 16 | 16 | 2 |

Ligne H (aides par nom, dont `pico8_probe_field_*`), recopiée :

> | **H** — définitions des aides par nom (`field_truthy`, `field_fixed`, `pico8_probe_field_*`, `fm_field_*`, `pico8_ui_field_*`…, 27 aides dans 15 fichiers) | disparaissent avec leurs appelants (A, B, C, D, M) | 39 | — | 15 |

§4, point 1, recopié :

> 1. **Classe M, 32 lignes** (16 comptées et 16 appels d'aides, `particle.c`, `probe.c`) : l'émetteur de particules et la sonde sont du code partagé par des branches distinctes (`g_`/`fv` et l'arbre `qh`, plus `mz` pour `dl`), qui lit par nom des champs que chaque branche porte à sa place. C'est la règle 2 de la phase 2. Le §5 cite d'ailleurs ce groupe `py`/`fc`/`k_`/`qo`/`ro`/`qg`/`q_`/`du` comme « exemple connu ». Aucune forme de D ne le couvre sans anticiper cette règle, ou sans ajouter environ 25 accesseurs jetables (Q7).

Ligne D.11c du tableau des étapes proposées (§5), recopiée :

> | **D.11c** | M selon Q7 (composants émetteur et sonde) | `particle`, `probe`, plus les 13 fichiers de classes qui les portent | `par-nom` ≈ 861 |

Q7, question et recommandation, recopiées :

> **Q7. Classe M (émetteur de particules, sonde ; 32 lignes) : (A) anticiper en D la règle 2 de la phase 2 pour ces deux groupes seulement (composant porté par pointeur) ; (B) environ 25 accesseurs typés, jetés en phase 2 ; (C) exclure ces sites du compte de D (`par-nom` ≠ 0 à la Sortie, reporté en phase 2) ?**
> A. Le plan cite déjà ce groupe comme l'exemple connu de composant, et la règle 2 est écrite, pas inventée. B produit du code jetable. C casse « sortie vide = sortie de D ». Risque à noter dans le PRD : la règle 2 s'applique sans le test ponctuel d'archétype, qui ne porte que sur les défauts.

Sites M de l'annexe A de l'étude (mesurés à `2dec71f`) :

```
M  particle.c         pico8_ei / pico8_ez / pico8_f_  R:fa R:fc R:q_ R:qg R:qo R:qy R:ra R:ro R:k_ W:qy
M  probe.c            pico8_bv / pico8_d_ / pico8_dl / pico8_g__mm  R:dw R:dq R:et R:fp R:ik R:od R:me R:qs R:du R:ki R:pk R:pn R:py R:t W:et W:fo W:p W:ra W:p_
```
