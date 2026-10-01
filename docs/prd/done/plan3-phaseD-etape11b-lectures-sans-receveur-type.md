---
title: "Plan 3, étape D.11b : les lectures par nom sans receveur typé deviennent typées — membres d'une fille lus sur la famille par le lien parent (`pico8_qh_of`, `pico8_nl_of`, B, Q5), sortes lues sur un enregistrement typé pointé par la sorte (K), cadre de `pz` en struct locale remplie par ses cinq appelants (F) ; deux chunks b1/b2 ; la classe C est à D.11e"
description: "Après D.11a, `par-nom` compte encore des lectures par nom dont le receveur n'a pas de type fixé par le cart : membres de `qh`/`nl` lus sur une planète quelconque, champs d'une sorte lus par une icône, champs du `p` de `pz`. D.11b les remplace par les formes que l'humain a tranchées à D.9, sans changer un pixel ; `par-nom` `src/` passe de 345 à 317."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.11a était implémentée** (`plan-pilot`, étape 2b), **puis révisé après sa graduation** : D.11a est gradée à `d4c4f4c` (VERIFIED), et la revue du plan qui a suivi a **déplacé la classe C de D.11b en tête de D.11e**. Tout ce qui suit a été lu et mesuré sur l'arbre **commis** `d4c4f4c`, extrait par `git archive d4c4f4c` dans le scratchpad : `grep`, `sh tools/data_residue.sh` (avec `--detail`) et la recette d'occurrences de D.8 seulement. **Rien n'a été compilé ni lancé.** Les numéros de ligne sont ceux de `d4c4f4c`. Les hypothèses `H-D11a-n` de la première version sont confrontées à `d4c4f4c` plus bas ; le contenu de C qu'avait préparé la première version est gardé en **Annexe C, « Reporté à D.11e »**, pour que le PRD de D.11e le reprenne.

Le problème, vu du mainteneur du port : après D.11a, parmi les lectures par nom qui restent dans `src/` hors de l'infrastructure (lecteurs `field`, écrivains `set_field`, `kind_field`, `*_take_*` des écrivains), trois familles ont un receveur dont le type n'est pas fixé au site, et une forme tranchée à D.9 :

- un membre d'une fille (`qh`, `nl`) lu sur une planète quelconque de la famille `jo` (classe B, Q5) ;
- les quatre champs qu'une icône de recherche lit sur la sorte qu'elle tient (`ie.ke`), par le `field` de la sorte (classe K) ;
- les sept champs que `pz` lit sur son `p`, qui est tour à tour un `fv`, la vue `pj_cr`, la vue `ha_pz` ou une planète (classe F).

Tant qu'elles passent par un nom, le compilateur ne vérifie ni le nom, ni le type, ni la présence, et l'infrastructure par nom (D.11e) ne peut pas tomber.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `d4c4f4c` (§5, « Phase D — Données », l. 273) :

> - D.11b — `par-nom`, lectures sans receveur typé : B (7 + 1 ; test d'appartenance par le lien parent, `pico8_qh_of(jo)`, Q5), (C passe en tête de D.11e : ses descripteurs sont ceux que D.11e rouvre, revue du plan après D.11a), F (5 + 2 ; struct locale de cadre `{ep, eo, hi, ff, ca, gq, ra}` remplie par les cinq appelants de `pz`), K (4 ; enregistrement typé par sorte `{rr, has_s, s, me, has_od, od}`, résolu le long de la chaîne, pointé par `pico8_entity_kind_t`). Empreinte ≈ 15 (15, 0). Compte : `par-nom` ≈ 882. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non.

Décisions de D.9, recopiées (Q5 lie cette étape ; Q6 lie désormais D.11e) :

> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Les autres textes du plan qui lient cette étape, à `d4c4f4c` — la section « Phase D » entière (ses puces, dont la **Sortie** ; la note de D.0 ; le paragraphe « Liste de D » avec l'empreinte, les règles communes et l'ordre ; toutes les lignes D.2 à D.12 avec leurs notes « Découvert à D.x », **dont celle de D.11a** ; l'arrêt « champ sans règle » ; les décisions de D.9 ; les comptes au départ de D.10), les règles de phase du §5 (1a, 1b, 2, 3), le §6 et le §7 — sont recopiés tels quels en **Annexe A**. La classe de chaque site est celle de l'annexe A de l'étude D.9, dont les lignes B, F, K (et C, pour l'Annexe C) sont recopiées en **Annexe B**.

### Hypothèses `H-D11a-n`, confrontées à `d4c4f4c`

La première version de ce PRD, écrite sur `5e75832`, supposait l'issue de D.11a. Confrontation avec l'arbre gradé et la note « Découvert à D.11a » :

| Hypothèse | Énoncé (résumé) | Verdict à `d4c4f4c` |
|---|---|---|
| H-D11a-1 | D.11a gradée en deux commits a1/a2, avec sa note | **tenue** (a1 `a1b2082`, 19 chemins ; a2 `181fecc`, 29 ; note dans `d4c4f4c`) |
| H-D11a-2 | `par-nom` `src/` 418 → **342**, `tests/` 568 | **fausse pour `src/`** : **345** (départ − 73) ; `jc.mt` reste par nom dans `jc:hq` (3 lignes : `pico8_jc_entity_value`, `jc_take_fixed`, l'appel), parti à D.11e avec C. `tests/` 568 tenue ; total 913 |
| H-D11a-3 | accesseurs de singletons (`pico8_fm_hy`, `pico8_fm_placeholder`, `pico8_je_nf`, `pico8_mo_qb`, `pico8_nf_l`/`_r`) | **tenue** (`probe.h:116,119`, `ui.h:82`, `entity.h:69`, `mouse_reader.h:32`) |
| H-D11a-4 | `pico8_lt(pico8_ke_t *)` ; `nu`/`fa`/`ra`/`p_` de `lt`/`he`/`dc`/`dg` intacts | **tenue** (`entity.h:42` ; `entity.c:237`, `:266`, `:300`, `:329` ; `bootstrap.c:87`) — sans objet pour D.11b (C) |
| H-D11a-5 | les trois reclassés intacts, avec leurs aides | **tenue** (`wormhole.c:70`, `entity_geom.c:104`, `wormhole_display.c:41`) — sans objet pour D.11b (C, D.11e) |
| H-D11a-6 | aides supprimées / restantes | **tenue pour B et F** (`field_truthy` d'`economy_score`/`economy_tiers`/`planet_display`, `field_fixed` de `planet_display`, `pico8_msgbox_field_vec` restent ; les autres aides listées sont parties). **Fausse pour `slipway.c`** : `pico8_jc_entity_value` (l. 131) et `jc_take_fixed` (l. 170) existent encore (appelant `jc:hq`, `mt`) ; elles vont à D.11e |
| H-D11a-7 | `pico8_ma` asserte `jo`/`qb` | **tenue** (`entity_geom.c:79`) — sans objet (C) |
| H-D11a-8 | `pico8_jc_end_p`/`_ra` existent ; `jc:hq` lit `o_` typé **et `mt` sur `fr`** ; écriture de `fo` intacte | **en partie fausse** : `jc_end_p`/`_ra` existent (`slipway.h:72-73`), `o_` typé, mais `mt` reste par nom (note de D.11a : lier `pico8_fr_struct_class` casse `test_pico8_bg_kj`, dont le lien `DATA_ENGINE_SRCS` ne contient pas `planet_family_trade.c`). Sans objet pour D.11b ; leçon de lien reprise en H2 |
| H-D11a-9 | `giver_t` porte un `pico8_ke_t` | **tenue** (`test_pico8_ui.c:800-801`, `:847`) — sans objet (C) |
| H-D11a-10 | D.11a ne touche pas les lignes B, F, K | **tenue** : seuls les numéros bougent (`economy_score.c`, `economy_tiers.c`, `planet_display.c`, `planet_trade.c`, `msgbox.c`, `menu.c`) ; `planet.c:635`, `research_icon.c:85-129`, `help.c:67` inchangés |
| H-D11a-11 | aucun membre ajouté à `pico8_struct_class_t` ni à `pico8_entity_kind_t` | **tenue** (`pico8_struct.h` identique à `5e75832`) |
| H-D11a-12 | `economy_truthy` d'`economy_tiers.c` garde un appelant hors `field_truthy` | **tenue** (`economy_tiers.c:302`, vérité de `pico8_jo_hf(jo)`) |
| H-D11a-13 | constantes, `class_coverage_net_expected.txt`, `Makefile`, `tools/` inchangés | **tenue** pour `Makefile` et `tools/` (identiques à `5e75832`) ; constantes : tenue selon la graduation (VERIFIED), non recomptée ici |
| H-D11a-14 | `valeur`/`type-tag`/`ref-audit` rapportés par D.11a | **tenue** : 3906/4202, 345/547, 44/45 (lignes/occurrences), remesurés ci-dessous |

Conséquences pour ce PRD : le départ est `src/` **345** (pas 342) ; les lignes de `mt` ne sont pas à D.11b ; toutes les lignes ⟲ ont été refaites ; la leçon de lien de D.11a (un fichier de `DATA_ENGINE_SRCS` ne doit nommer aucun symbole défini hors de cette liste) devient H2 et un critère `FAILED:`.

### Sections revérifiées sur `d4c4f4c`

Refaites dans cette révision : État de départ, Inventaire (tous les numéros), comptes attendus par fichier et classification, empreinte et chemins des chunks, TU, greps de la « Sortie », critères `FAILED:`, gabarit de la note, budget. Plus rien n'est ⟲ : si le HEAD de départ de D.11b n'est pas `d4c4f4c` (ou un descendant qui ne touche que `docs/`), l'implémenteur remesure et explique l'écart.

### Lecture de ces textes pour D.11b

- **Périmètre** : la ligne nomme trois classes, prises telles quelles : **B, F, K**. C n'en fait plus partie (« C passe en tête de D.11e »), ni les lignes de `jc.mt` que D.11a a laissées (la ligne D.11e les nomme). S'y ajoutent, comme à D.11a, les **définitions d'aides par nom devenues sans appelant** (classe H de l'étude) et les **aides `*_take_*` de la sorte** (`ie_ke_take_string`/`_fixed`, qui n'ont que les quatre appelants K) : une aide meurt dans l'étape où part son dernier appelant. Hors périmètre : C et `mt` (D.11e, premier chunk), M et le G `ke.c:43` (D.11c), T1/T2 (D.11d), l'infrastructure (D.11e).
- **« B (7 + 1) »** : 7 lignes comptées et 1 appel d'aide — mesuré identique à `d4c4f4c` (table B). Q5 : un lien parent **ajouté à `pico8_struct_class_t`**, et `pico8_qh_of(jo)` qui rend NULL hors de la lignée. Les membres lus sont ceux de `qh` (`qh`, `hp`, `px`, `s`, `ba`) et de `nl` (`nl`, `qw`) : il faut donc `pico8_qh_of` **et** `pico8_nl_of`, sur le même modèle.
- **« F (5 + 2) »** : mesuré identique (5 lignes comptées, 2 appels de `pico8_msgbox_field_vec`), plus la définition de l'aide (1 ligne comptée). « Remplie par les cinq appelants » : `fv:pq` (`help.c:67`), `cr` (`menu.c:280`, la vue `pj_cr`), `ix[4].my` (`menu.c:404`, la vue `ha_pz`), `jo:gh` (`planet_display.c:293`) et `jo:fe` (`:409`).
- **« K (4) »** : mesuré identique (les 4 appels `ie_ke_take_*` de `pico8_ie_ke_view`), plus les deux aides `ie_ke_take_string`/`_fixed` et leurs deux lectures `kind->field` (4 lignes comptées). « Résolu le long de la chaîne » : chaque enregistrement porte ce que le `field` de la sorte rend aujourd'hui en remontant sa chaîne. « Pointé par `pico8_entity_kind_t` » : un membre pointeur ajouté à la struct de sorte. **9 sortes** mesurées (`qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`), pas 11 (la ligne D.11e dit encore « 11 sortes »).
- **« Empreinte ≈ 15 (15, 0) »** : remesurée à **16 fichiers de `src/` et 3 TU** (plus `docs/plan-3.md`) : voir « Empreinte ».
- **« Compte : `par-nom` ≈ 882 »** : l'estimation de ce PRD donne `src/` **−28** (345 → 317), `tests/` ± 0 (+1 à b1, −1 à b2), soit un total **885**. L'écart de 3 avec la ligne sont les trois lignes de `mt`, que la ligne ne compte plus (≈ 882 = 910 − 28) mais que D.11a a laissées dans le départ (913). Le compte est rapporté, écart expliqué.
- **« §6 : aucun »** : `test_pico8_planet_gh_row_lq` exerce `jo:gh` (sites B `:163`, `:223` et F `:293`) mais lit par nom depuis le TU (D.11d) ; `test_pico8_ga_export_panel_by` et `test_pico8_layout_nil_item_draws_nothing` non plus ne sont touchés. Ils doivent rester verts.
- **« `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non »** : aucun constructeur ne change de texte ; les descripteurs de classe et les sortes gagnent un membre initialisé statiquement (`parent`, `rec`), ce qui n'est pas un constructeur. Pas de `class-coverage-net` exigé ; l'implémenteur peut le lancer (quelques secondes) et le rapporter. Aucune durée de vie ni aucun propriétaire ne change (le `ra` partagé et les `qf` de `pz` restent fuités comme aujourd'hui, TROU-7) : pas de `sanitize-net`. Balayage ASan+UBSan des TU à la fin (consigne du pilote).
- **Leçons des notes à ne pas refaire** (D.7a à D.11a) : classification (a)/(b)/(c) absente ou fausse ; (c) non déclarée ; lignes qui gagnent une catégorie omises ; conversion typé → valeur ajoutée le temps d'un commit ; constante de hachage « mise à jour » ; octet gelé remplacé par un littéral nu ; TU corrompu qui passait en relisant sa propre corruption (d'où le balayage ASan des TU) ; site « typé » en déplaçant son déballage dans une aide non comptée ; **symbole défini hors de `DATA_ENGINE_SRCS` nommé depuis un fichier de cette liste** (D.11a, `jc.mt`) ; **cas de mort d'un TU non vu par le PRD** (D.11a, `fm_hy_on_provisional`) : l'implémenteur cherche par grep, dans `tests/`, chaque symbole dont le contrat change (`pico8_pz`, `pico8_ie_ke_view`, `pico8_jo_pq`).
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json`, et rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes, alias, registre, renommage pour échapper aux greps, cast non vérifié, cale valeur ↔ typé permanente ; mutations restaurées par `cp` ; binaires lancés exactement comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour un TU lié sous ASan) ; balayage ASan+UBSan des TU ; chunks d'au plus ~40 chemins ; jamais l'appli raylib.

### État de départ

**Mesuré sur `d4c4f4c`** (arbre archivé), `sh tools/data_residue.sh`, `--detail` et la recette d'occurrences de D.8, sans build :

| Catégorie | Lignes | Fichiers | Occurrences | `src/` (lignes / occ.) | `tests/` (lignes) |
|---|---|---|---|---|---|
| `valeur` | 3906 | 123 | 4202 | rapporté | rapporté |
| `type-tag` | 345 | 64 | 547 | rapporté | rapporté |
| `ref-audit` | 44 | 15 | 45 | 44 | 0 |
| `par-nom` | 913 | 73 | 1028 | 345 / 405 | 568 |
| **résidu D** | **4736** | **124** | — | | |

`DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch` (`Makefile` l. 889).

`par-nom` `src/` des fichiers de l'empreinte à `d4c4f4c` : `economy_score.c` 3, `economy_tiers.c` 3, `planet.c` 34, `planet_display.c` 7, `planet_trade.c` 1, `research_icon.c` 17, `msgbox.c` 12, `help.c` 8, `menu.c` 2, `planet_family.c` 58, `planet_family_trade.c` 31, `struct.h` 4, `planet.h` 1 ; `game_data.c/.h`, `msgbox.h`, `planet_family.h` 0. Les fichiers de descripteurs et de données ne perdent aucune ligne : ils gagnent `parent`/`rec`, dont aucun nom ne correspond au motif `par-nom`.

**Liens** (leçon de D.11a) : `DATA_ENGINE_SRCS` (`Makefile` l. 238-289) contient `economy_score.c`, `economy_tiers.c`, `game_data.c`, `msgbox.c`, `planet.c`, `planet_display.c`, `planet_family.c`, `planet_trade.c`, `probe.c` ; **pas** `planet_family_trade.c`, `help.c`, `menu.c`, `research_icon.c`. Tous les symboles nouveaux que nomment les fichiers de cette liste (`pico8_qh_of`, `pico8_nl_of` dans `planet_family.c` ; `pico8_X_kind_rec` dans `game_data.c` ; `pico8_pz` dans `msgbox.c`) y sont définis ; les fichiers hors liste (`planet_family_trade.c`, `help.c`, `menu.c`, `research_icon.c`) nomment des symboles de la liste, jamais l'inverse (H2).

### Inventaire (mesuré sur `d4c4f4c`)

Recette : `sh tools/data_residue.sh --detail`, puis `grep -nE "$(pattern par-nom)" src/*.[ch]`, puis `grep` de chaque aide à nom en paramètre et de chaque nom lu dans les lecteurs `field`/`set_field` (`grep -n 'strcmp(name, "X")' src/*.c`). Chaque site a été lu, et chaque ligne retirée ne porte qu'une occurrence du motif (vérifié : Δlignes = Δoccurrences partout). Notations : **R** lu, **W** écrit ; **c** ligne comptée par `par-nom`, **h** ligne d'appel d'une aide (non comptée) ; **d** définition d'aide comptée.

**Classe B — membre d'une fille lu sur la famille `jo` (b1).** 7 lignes comptées, 1 appel d'aide, 5 fichiers. Qui répond au nom : `hp`, `px`, `qh`, `s`, `ba` ne sont répondus que par `pico8_qh_field` (`planet_family.c:66-115`), donc par la lignée `qh` (`qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`) ; `nl` et `qw` que par `pico8_nl_field` (`:235-254`), et `nl` n'a pas de fille. `jo` et `ga` ne répondent à aucun.

| Fichier:ligne | Fonction | Membre | Receveur (type statique → famille) | Remplacement | c/h |
|---|---|---|---|---|---|
| `economy_score.c:59` | `pico8_da` | R `qh` (vérité) | `pt` : `pico8_struct_header_t *` ∈ `cg("jo")` → famille `jo` | `const pico8_qh_t *qh = pico8_qh_of(pt)` ; `qh != NULL && economy_truthy(pico8_qh_qh(qh))` (genre mêlé, Q12 : l'API à valeur existante est gardée, précédent `pico8_jo_hf` de D.11a) | c |
| `economy_tiers.c:284` | `pico8_dh` | R `nl` (vérité) | `pt` → famille `jo` (asserté par D.11a dans `pico8_dh`) | `pico8_nl_of(pt) != NULL` : `nl.nl` est un fixe toujours présent, donc toujours vrai | c |
| `planet.c:635` | `pico8_jo_pq` | R `hp` | `self` : `pico8_jo_t *` (toute la famille) | `qh = pico8_qh_of(&self->ke.header)` ; `qh != NULL` → `pico8_gb_at(pico8_fixed_flr_int(qh->hp))`. La branche `PICO8_VALUE_STRING` (`pico8_gb_find`) est **inexprimable** : `qh.hp` est un `pico8_fixed_t` depuis 1b.24, aucun lecteur ne rend `hp` chaîne ; retirée, déclarée (a) | c |
| `planet_display.c:163` | `pico8_jo_gh` | R `px` | `self` : `pico8_jo_t *` | `qh != NULL && qh->px != NULL` → `icon_row = qh->px` | c |
| `planet_display.c:223` | `pico8_jo_gh` | R `qh` (vérité) | idem | `qh != NULL && pico8_jo_truthy(pico8_qh_qh(qh))` | c |
| `planet_display.c:444` | `pico8_jo_kj` | R `qw` | `self` : `pico8_jo_t *` | `nl = pico8_nl_of(...)` ; `pico8_dv(mem, nl != NULL ? nl->qw : 0)` (absent → 0, comme `field_fixed`) | h |
| `planet_display.c:452` | `pico8_jo_kj` | R `s` | idem | `else if (qh != NULL) spr_index = pico8_fixed_flr_int(qh->s)` (`qh.s` toujours présent) | c |
| `planet_trade.c:156` | `pico8_jo_cn` | R `ba` | `self` : `pico8_jo_t *` | `qh != NULL && qh->ba != NULL` → `pico8_int_seq_has_fixed(qh->ba, …)` | c |

Aides B qui meurent (b1, définitions, 7 lignes comptées) : `field_truthy` d'`economy_score.c` (l. 25, 27), d'`economy_tiers.c` (l. 31, 33), de `planet_display.c` (l. 39, 41) ; `field_fixed` de `planet_display.c` (l. 49 ; sa définition l. 47 n'est pas comptée). Leur commentaire (`planet_display.c:36-38`, « Readers of a name no `jo` has a member for (`qh`, `qw`)… ») part avec elles.

**Classe K — sortes (b1).** 4 lignes comptées de site et 4 lignes comptées d'aides, 1 fichier.

| Fichier:ligne | Fonction | Lecture | Remplacement | c/d |
|---|---|---|---|---|
| `research_icon.c:126-129` | `pico8_ie_ke_view` | `rr`, `s`, `me`, `od` par `ie_ke_take_*(kind, "…")` → `kind->field` | `const pico8_kind_rec_t *rec = kind->rec` ; `view.rr = rec->rr`, `view.has_s = rec->has_s`, `view.s = rec->s`, `view.me = rec->me`, `view.has_od = rec->has_od`, `view.od = rec->od` | c ×4 |
| `research_icon.c:85`, `:89`, `:98`, `:103` | `ie_ke_take_string`, `ie_ke_take_fixed` | définitions et leurs deux `kind->field(` | supprimées : sans appelant ; leurs asserts de genre deviennent le type | d ×4 |

Contenu attendu des 9 enregistrements, lu le long des chaînes `pico8_X_kind_field` (`planet_family.c:929-1125`, `planet_family_trade.c:491-584`, `planet.c:584`) :

| Sorte | `rr` | `s` | `me` | `od` |
|---|---|---|---|---|
| `qh` | absent | `qh` | `qh` (`"kz"`) | `qh` |
| `nl` | `nl` | `nl` | `qh` | `nl` |
| `os` | absent | `os` | `qh` | `os` |
| `hj` | `hj` | `hj` | `qh` | `hj` |
| `pm` | `hj` | `pm` | `pm` (`"ly"`) | `hj` |
| `ga` | absent | absent | `jo` (`"unknown"`) | absent |
| `nr` | `nr` | `nr` | `qh` | `nr` |
| `fr` | `fr` | `fr` | `qh` | `fr` |
| `mx` | `mx` | `mx` | `qh` (par `hj`) | `mx` |

(« `qh` » : la valeur de `pico8_qh_start` ; les chaînes sont **les mêmes tableaux** que ceux de `pico8_X_start` — `qh_me_text`, `nl_rr_text`… de `pico8_game_data.c` —, parce que `test_pico8_entity_kind` compare l'identité des pointeurs `rr`/`me` de la vue à ceux du `field` de la sorte.)

**Classe F — cadre de `pz` (b2).** 5 lignes comptées, 2 appels d'aide, 1 définition d'aide comptée, 1 fichier ; 5 appelants dans 3 fichiers, et 3 appels de TU.

| Fichier:ligne | Fonction | Lecture sur `p` | Remplacement | c/h/d |
|---|---|---|---|---|
| `msgbox.c:368` | `pico8_pz_step` | R `ca` (TROU-9) | `ctx->frame->has_ca` → `b.fa = ctx->frame->ca; b.has_fa = 1` ; l'assert de genre devient le type | c |
| `msgbox.c:374` | `pico8_pz_step` | R `gq` | `gq = frame->gq != NULL ? *frame->gq : zéro` | h |
| `msgbox.c:401` | `pico8_pz` | R `ep` (vérité) | `has_ep = frame->ep != NULL` ; l'item de la rangée reste un `pico8_value_t` (élément de `lq.items`, L3, D.12c) : `ep_item = pico8_value_string(frame->ep)`, déclaré | c |
| `msgbox.c:411` | `pico8_pz` | R `eo` (vérité) | `rn_arg = frame->has_eo ? pico8_value_fixed(frame->eo) : pico8_value_fixed(62)` (`pico8_rn` prend une valeur, Q12, inchangé) | c |
| `msgbox.c:418` | `pico8_pz` | R `hi` | `frame->hi != NULL ? *frame->hi : …` | c |
| `msgbox.c:422` | `pico8_pz` | R `ra` | `… : (frame->ra != NULL ? *frame->ra : zéro)` | h |
| `msgbox.c:452` | `pico8_pz` | R `ff` (vérité) | `frame->ff && ok != NULL` | c |
| `msgbox.c:49` | `pico8_msgbox_field_vec` | définition | supprimée (sans appelant ; D.11a a converti ceux de `iv:pq`, H-D11a-10) | d |

Meurt aussi `pico8_msgbox_truthy` (`msgbox.c:35`, non comptée) : ses quatre appelants sont les lignes F (`-Werror`). L'assert d'en-tête de `pz` (`pico8_type_tag_of(p) == PICO8_TYPE_TAG_STRUCT`, `msgbox.c:398-399`) devient le type (`type-tag` −1 ligne, déclaré).

Ce que chaque appelant répond aujourd'hui (lecteurs lus, `strcmp(name, "X")`) et donc ce qu'il met dans le cadre ; tout le reste est absent :

| Appelant | `p` | `ep` | `eo` | `hi` | `ff` | `ca` | `gq` | `ra` |
|---|---|---|---|---|---|---|---|---|
| `help.c:67` `fv:pq` | `pico8_fv_t *` | — | `fv->eo` (fixe, toujours) | `fv->hi` | vrai (`fv->ff` fixe, toujours) | `fv->ca` (toujours) | `fv->gq` | `ra` de `g_` (règle du lecteur `"ra"` de `pico8_g__field`) |
| `menu.c:280` `cr` | vue `pico8_pj_cr_t *` | `cr->ep` | — | `cr->hi` | — | `cr->ca` (toujours) | `cr->gq` | — |
| `menu.c:404` `ix[4].my` | vue `g_ha_pz` | — | — | `hi` | — | `has_ca`/`ca` | `gq` | — |
| `planet_display.c:293` `jo:gh` | `pico8_jo_t *` | `self->ep` | — | — | — | `self->ca` (règle du lecteur) | `self->gq` | `self->ra` |
| `planet_display.c:409` `jo:fe` | `pico8_jo_t *` | idem | — | — | — | idem | idem | idem |

Aucune fille de `jo` ne répond à `ep`, `eo`, `hi`, `ff`, `ca`, `gq` ni `ra` en propre (grep des lecteurs) : le cadre rempli depuis `jo` est exact pour toute planète.


**Sites par nom des fichiers de l'empreinte, laissés** (classe, étape) : `planet_display.c:482` (`dw` sur `jo.fo`, C, D.11e) ; les lecteurs `field`, écrivains `set_field`, `kind_field`, `*_take_*` des écrivains (D.11e) ; les sites C et `mt` des autres fichiers (D.11e, Annexe C) ; M et `ke.c:43` (D.11c). Aucun site A, D, E ou G de D.11a n'est rouvert.

**TU** : D.11b ne change la forme d'aucune lecture par nom de TU (D.11d). Trois TU suivent, dans le chunk qui change ce qu'ils nomment ou ce dont ils dépendent (règle du pont, tranchée : **aucun pont**) :
- `test_pico8_qh` (b1) : un cas ajouté, sans constante, sur `pico8_qh_of`/`pico8_nl_of` (voir « TU touchés ») ;
- `test_pico8_entity_kind` (b1) : un cas ajouté, sans constante, qui compare l'enregistrement de chacune des 9 sortes au `field` de la sorte ;
- `test_pico8_js` (b2) : `test_pz_fa_only_from_ca` (l. 1864-1887) passe un cadre ; `test_pz_closed` (l. 1923-1930) perd son cas « une liste arrête `pz` » (`must_abort(pz_on_a_table, &list)` et `pz_on_a_table`, l. 1858-1860), **inexprimable** avec un cadre typé, retiré et listé ; `pico8_pz(NULL, NULL, 0)` reste.
`test_pico8_planet_kj_dispatch` copie la table de `jo` (`g_spied_jo = *pico8_jo_struct_class()`) : la copie emporte `parent` (NULL pour `jo`), rien à changer. Aucun autre TU n'appelle `pico8_pz` ni ne déclare de `pico8_entity_kind_t` (grep à `d4c4f4c`).

## Solution

**Deux commits (chunks b1, b2), chacun vert seul**, sans pont :

1. **b1 — lien parent (B) et enregistrements de sorte (K).**
   - `pico8_struct_class_t` gagne `parent` (la classe dont la struct est enchâssée en premier ; NULL pour une racine), posé dans les 9 descripteurs de filles de l'arbre `jo` et dans celui de `fv` (parente `g_`).
   - `pico8_qh_of()` et `pico8_nl_of()` (`planet_family.c/.h`) rendent la struct typée, ou NULL hors de la lignée, en remontant `parent`.
   - Les 7 lignes B et l'appel `qw` lisent les membres typés ; `field_truthy`/`field_fixed` de trois modules meurent.
   - `pico8_entity_kind_t` gagne `rec`, un pointeur vers un enregistrement typé `pico8_kind_rec_t` `{rr, has_s, s, me, has_od, od}` généré une fois dans `pico8_game_data.c` pour chacune des 9 sortes, résolu le long de sa chaîne ; `pico8_ie_ke_view` le lit ; `ie_ke_take_*` meurent.
   - `test_pico8_qh` et `test_pico8_entity_kind` gagnent chacun un cas.
2. **b2 — cadre de `pz` (F).**
   - `pico8_pz(const pico8_pz_frame_t *frame, const pico8_js_keys_t *bs, int n)` ; `pico8_pz_frame_t` `{ep, has_eo/eo, hi, ff, has_ca/ca, gq, ra}` (`msgbox.h`), rempli par chacun des cinq appelants depuis ses membres typés, sur sa pile.
   - `pico8_msgbox_field_vec` et `pico8_msgbox_truthy` meurent.
   - `test_pico8_js` suit (un cas retiré, inexprimable).
   - Enfin : note « Découvert à D.11b », balayage ASan+UBSan des TU.

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (`make data-guard` inchangé : `résidu gardé (spec,overrides,stash,ob,dispatch) : 0 ligne(s)`) ;
- `make e2e` : **26 conformes** ;
- balayage ASan+UBSan des TU propre ;
- `make data-residue` : `par-nom` `src/` = **317** (345 − 28), `tests/` = **568** (+1 à b1, −1 à b2 ; tout autre écart expliqué), total **885** ; le détail par fichier égal à la table « Effet sur `make data-residue` » ; `valeur`, `type-tag`, `ref-audit` rapportés ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/` ni de `tools/` touché, le `Makefile` inchangé, aucune constante `*_HASH`/`identity[]` changée ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  # 1. aides par nom, de sorte et de pz mortes : vide
  grep -nE '\b(field_truthy|field_fixed)\(' src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_planet_display.c
  grep -nE '\b(pico8_msgbox_field_vec|pico8_msgbox_truthy|ie_ke_take_(string|fixed))\b' src/*.[ch]
  # 2. lecteur par nom dans les fichiers de sites de D.11b : seule la ligne C de jo:kj (dw, D.11e)
  grep -nE 'pico8_entity_(get|set)_field\(' src/pico8_economy_score.c src/pico8_economy_tiers.c \
      src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_trade.c src/pico8_msgbox.c
  #    attendu : une ligne, src/pico8_planet_display.c (self->fo, "dw")
  # 3. plus aucune lecture de sorte par nom hors des kind_field (D.11e) : vide
  grep -nE 'kind->field\(|_kind\(\)->field\(' src/pico8_research_icon.c
  # 4. lien parent : 10 lignes (9 filles de jo, fv)
  grep -nE '^\s*\.parent = ' src/*.c | wc -l
  # 5. enregistrements de sorte : 9 définitions, 9 sortes qui les pointent
  grep -cE '^const pico8_kind_rec_t pico8_[a-z_]+_kind_rec\b' src/pico8_game_data.c
  grep -nE 'pico8_entity_kind_t g_[a-z_]+_kind = ' src/*.c
  # 6. pz ne prend plus d'en-tête : chaque appel passe un cadre, aucun cast d'en-tête
  grep -n 'pico8_pz(' src/*.c src/*.h tests/*.c
  # 7. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^par-nom'
  ```

## User Stories

1. En tant que mainteneur du port, je veux qu'un membre de `qh` ou de `nl` lu sur une planète quelconque passe par `pico8_qh_of()`/`pico8_nl_of()`, afin que la lecture soit un accès typé derrière un test d'appartenance à la lignée (Q5), et non un nom.
2. En tant que mainteneur du port, je veux que `pico8_struct_class_t` porte le lien vers la classe parente, afin que l'appartenance à une lignée se teste sur la table de classe elle-même, sans prédicat ad hoc.
3. En tant que mainteneur du port, je veux que `pico8_qh_of()` rende NULL pour `jo`, `ga` et toute classe hors de l'arbre, et la struct pour `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, afin que la présence lue soit celle du lecteur par nom d'aujourd'hui.
4. En tant que mainteneur du port, je veux que la branche « `hp` chaîne » de `jo:pq`, inexprimable depuis que `qh.hp` est un fixe, soit retirée et déclarée, afin qu'aucun code mort ne survive au typage.
5. En tant que mainteneur du port, je veux qu'une icône de recherche lise `rr`, `s`, `me`, `od` de sa sorte sur un enregistrement typé pointé par la sorte, afin que K n'ait plus de lecture par nom et que les aides `ie_ke_take_*` disparaissent.
6. En tant que mainteneur du port, je veux que chaque enregistrement de sorte soit généré une fois, résolu le long de la chaîne de la sorte, et pointe les mêmes chaînes que `pico8_X_start`, afin que la valeur et l'identité lues soient celles d'aujourd'hui.
7. En tant que mainteneur du port, je veux que `pz` reçoive un cadre typé que chacun de ses cinq appelants remplit depuis ses membres, afin que `pz` ne lise plus rien par nom sur un `p` hétérogène.
8. En tant que mainteneur du port, je veux que le cadre dise la présence de `eo` et `ca` et la nullité de `ep`, `hi`, `gq`, `ra`, afin que les replis de `pz` (62, `ra`, vecteur nul, `fa` de `js`) restent ceux d'aujourd'hui (TROU-9 compris).
9. En tant que mainteneur du port, je veux que chaque aide par nom sans appelant soit supprimée dans le chunk où part son dernier appelant, afin qu'aucune aide morte ne reste pour D.11e.
10. En tant que mainteneur du port, je veux qu'aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste, afin que les TU liés sur elle (`test_pico8_bg_kj`, `test_pico8_planet_gh_row_lq`…) se lient encore sans toucher au `Makefile` (leçon de D.11a).
11. En tant que gardien du compteur, je veux `par-nom` `src/` égal à 345 − 22 après b1 et 345 − 28 après b2, fichier par fichier comme la table de ce PRD, afin que chaque baisse soit un site retiré.
12. En tant que gardien du compteur, je veux, au message de chaque commit, pour chaque catégorie et chaque fichier : les comptes avant/après en lignes et en occurrences ; la classification (a1)/(a2)/(b)/(c) des lignes `par-nom` retirées ; les lignes qui gagnent une catégorie (« aucune » compris) ; la justification de chaque Δlignes > Δoccurrences. Afin qu'aucune baisse ne soit un tassement.
13. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, et qu'aucun alias, registre, renommage pour les greps, cast non vérifié ni cale valeur ↔ typé permanente ne soit ajouté, afin que la baisse veuille dire « retiré ».
14. En tant que gardien des TU, je veux que le cas « une liste arrête `pz` », rendu inexprimable par le cadre typé, soit retiré et listé au message avec sa raison, afin qu'aucune couverture ne disparaisse sans trace.
15. En tant que gardien des TU, je veux un cas qui compare l'enregistrement de chacune des 9 sortes au `field` de la sorte, présence, valeur et identité des chaînes, afin de prouver « résolu le long de la chaîne », y compris pour `pm` et `ga` qu'aucune icône ne tient.
16. En tant que gardien des constantes, je veux qu'aucune constante `*_HASH`/`identity[]` ne change, afin que l'identité de l'état reste prouvée.
17. En tant que gardien des filets, je veux `make test` et `make e2e` verts à chaque commit et le balayage ASan+UBSan des TU propre, afin de prouver qu'aucun comportement n'a changé.
18. En tant que `plan-pilot`, je veux une note « Découvert à D.11b » qui dise le lien parent, `pico8_qh_of`/`pico8_nl_of`, l'enregistrement de sorte, le cadre de `pz`, l'oracle de transition à retirer par D.11d/D.11e et les écarts de compte, afin que D.11c-D.11e partent d'un état exact.
19. En tant que `plan-pilot`, je veux le contenu de C préparé par la première version gardé en annexe « Reporté à D.11e », afin que le PRD de D.11e le reprenne sans le refaire.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q5 est une décision humaine (plan, « Décisions de D.9 ») : ce PRD l'applique ; Q6 est à D.11e ; Q12 reste ouverte et n'est pas tranchée ici.

### Modules

- **`struct`** (`pico8_struct.h`) : `pico8_struct_class_t` gagne `parent`, `pico8_entity_kind_t` gagne `rec` (b1). L'en-tête inclut `pico8_game_data.h` pour `pico8_kind_rec_t` (H1).
- **`game_data`** : `pico8_kind_rec_t` et les 9 enregistrements générés (b1).
- **`planet_family`** : `pico8_qh_of()`, `pico8_nl_of()`, les liens `parent` et `rec` de ses six filles (b1).
- **`planet_family_trade`** : liens `parent` et `rec` de `nr`, `fr`, `mx` (b1).
- **`help`** : `parent` de `fv` (b1) ; cadre de `fv:pq` (b2).
- **`msgbox`** : `pico8_pz_frame_t` et la signature de `pico8_pz` (b2).
- **Consommateurs, sites seulement** : `economy_score`, `economy_tiers`, `planet`, `planet_display`, `planet_trade` (B) ; `research_icon` (K) ; `help`, `menu`, `planet_display` (F).
- **Aucun module nouveau**, aucun symbole nouveau hors de `DATA_ENGINE_SRCS` nommé depuis cette liste (H2) : le `Makefile` ne change pas.

### B — lien parent et appartenance (G, b1)

Forme retenue (fragment de décision, pas un code à recopier) :

```c
/* pico8_struct.h, dans struct pico8_struct_class */
    /* The class whose struct this one embeds first; NULL for a root. */
    const pico8_struct_class_t *parent;

/* pico8_planet_family.h */
/* `e` itself when its class is qh or descends from it, NULL otherwise (and for NULL). */
const pico8_qh_t *pico8_qh_of(const pico8_struct_header_t *e);
/* `e` itself when its class is nl, NULL otherwise. */
const pico8_nl_t *pico8_nl_of(const pico8_struct_header_t *e);
```

- **`parent` posé** dans chaque descripteur dont la struct enchâsse en premier la struct d'une autre classe à descripteur : `qh`, `ga` → `jo` ; `nl`, `os`, `hj`, `nr` → `qh` ; `pm`, `mx` → `hj` ; `fr` → `nr` ; `fv` → `g_` (10 lignes). NULL partout ailleurs (structs qui enchâssent `pico8_ke_t`, sans descripteur, ou racines). `fv` n'a pas de consommateur à D.11b ; il est posé parce qu'un `parent` NULL y serait un contrat faux.
- **`pico8_qh_of`** remonte `e->cls`, puis `->parent`, jusqu'à `&g_qh_struct_class` (rend `e` casté) ou NULL (rend NULL). Le cast est vérifié par ce test, dans la même fonction. `pico8_nl_of` fait de même vers `&g_nl_struct_class` (même boucle ; `nl` n'a pas de fille aujourd'hui, mais la règle est celle de Q5, pas un test de classe exact). Les deux boucles sont écrites dans `planet_family.c`, sans aide générique (deux usages).
- **Sites** : voir la table B. `pico8_qh_qh()` (valeur à genre mêlé, Q12) garde son rôle pour la vérité de `qh.qh` (précédent D.11a : `pico8_jo_hf`) ; aucune fonction de conversion n'est ajoutée.
- **`pico8_jo_is_family`** (test par `states[LL]`) n'est pas réécrit sur `parent` : hors périmètre, aucune ligne comptée n'en dépend. Mentionné dans la note comme simplification possible.
- **Rejetés** : remonter `qh`, `hp`, `px`, `s`, `ba`, `nl`, `qw` dans `jo` (option B de Q5, refusée par l'humain) ; un test de classe exact par site (8 classes pour `qh`) ; un prédicat `*_is_qh_class` sur `states[]` (fragile, et Q5 nomme le lien parent) ; `pico8_qh_of` non `const` (tous les sites B lisent).

### K — enregistrement de sorte (G, b1)

```c
/* pico8_game_data.h */
/* A kind's `rr`, `s`, `me`, `od` as its chain resolves them: NULL string, or
 * has_* 0, when absent. */
typedef struct {
    const char *rr;
    int has_s;
    pico8_fixed_t s;
    const char *me;
    int has_od;
    pico8_fixed_t od;
} pico8_kind_rec_t;
extern const pico8_kind_rec_t pico8_qh_kind_rec; /* … nl, os, hj, pm, ga, nr, fr, mx */

/* pico8_struct.h, dans pico8_entity_kind_t */
    const pico8_kind_rec_t *rec; /* never NULL */
```

- **Générés une fois** dans `pico8_game_data.c`, à côté des `pico8_X_start`, avec le contenu de la table K de l'inventaire ; chaque chaîne est le tableau nommé que `pico8_X_start` pointe déjà (`qh_me_text`, `nl_rr_text`, `hj_rr_text`, `pm_me_text`, `jo_me_text`, `nr_rr_text`, `fr_rr_text`, `mx_rr_text`), jamais un littéral nouveau (identité lue par `test_pico8_entity_kind` ; TROU-3 pour les littéraux fusionnés).
- **Nom** : `pico8_X_kind_rec`, hors des motifs figés de `spec` (`_default`, `_defaults_reset`, `g_X_spec`/`_defaults`) et de `par-nom`.
- **Garde « module prêt »** : le `field` d'une sorte rend 0 quand son module (ou celui d'un ancêtre) est arrêté ; l'enregistrement, `static const`, n'a pas cette garde. `pico8_ie_ke_view` ne lit une sorte que depuis une icône vivante, donc modules prêts (H3). Le `field` des sortes et sa garde restent (D.11e) ; `test_kinds_answer_nothing_once_down` (`test_pico8_map_tables`) les teste et n'est pas touché.
- **Rejetés** : un enregistrement construit à l'init par une fonction (état mutable pour une donnée constante ; la ligne dit « résolu le long de la chaîne » à la génération, comme D.5c) ; un accesseur `rec()` qui rend NULL hors init (reproduit la garde, mais pas ses cas partiels — `ga` : `me` de `jo` seulement si `jo` est prêt — et n'est pas « pointé ») ; garder `ie_ke_take_*` en lisant l'enregistrement (aide sans objet).

### F — cadre de `pz` (G, b2)

```c
/* pico8_msgbox.h */
/* What `pz` reads of its `p`, filled by the caller from its own members:
 * NULL pointer or has_* 0 = nil; `ff` is the Lua truth of `p.ff`. `pz`
 * reads it during the call and keeps none of it. */
typedef struct {
    const char *ep;
    int has_eo;
    pico8_fixed_t eo;
    const pico8_vec_t *hi;
    int ff;
    int has_ca;
    pico8_fixed_t ca;
    const pico8_vec_t *gq;
    const pico8_vec_t *ra;
} pico8_pz_frame_t;

/* A NULL `frame` is a no-op. */
void pico8_pz(const pico8_pz_frame_t *frame, const pico8_js_keys_t *bs, int n);
```

- **Cinq appelants**, chacun remplit un cadre local (`= {0}`) depuis ses membres, sur la table F de l'inventaire. Les deux appelants de `planet_display.c` partagent une aide statique qui remplit le cadre d'une planète (même contenu, un seul endroit). Chaque membre posé reproduit la condition de présence du lecteur par nom de la classe (`fv.eo`, `fv.ca`, `jo.ca`, `pj_cr.ca` toujours présents ; `ha_pz.ca` selon `has_ca` ; pointeurs NULL = nil).
- **Replis inchangés** : `ra` de `pz` = `hi` si non NULL, sinon `ra` du cadre, sinon vecteur nul ; `rn(eo or 62)` ; `ep` en tête de rangée seulement s'il est non NULL ; `fa` de chaque bouton = `ca` si présent (TROU-9) ; le `ra` partagé et les `qf` alloués comme aujourd'hui (TROU-7).
- **Consommateurs à valeur** : l'item `ep` de la rangée (`lq.items`, L3) et l'argument de `pico8_rn` restent des `pico8_value_t` (D.12c, Q12) : construits au site par `pico8_value_string`/`pico8_value_fixed`, déclarés en lignes qui gagnent `valeur`. Pas une cale : aucune fonction de conversion, une API à valeur qui préexiste.
- **Vues `pj_cr` et `ha_pz`** : gardées telles quelles, avec leurs lecteurs (D.11e) ; `menu.c` lit leurs membres typés pour remplir le cadre.
- **Rejetés** : passer les sept valeurs en arguments de `pz` (neuf paramètres, et le cadre est la forme que la ligne nomme) ; un accesseur de table de classe pour ces sept noms (les vues `pj_cr`/`ha_pz` n'ont pas de méthode, et F a sa forme propre) ; garder `const pico8_struct_header_t *p` à côté du cadre (cale).

### Consommateurs à valeur (G)

Restent des API à valeur (Q12), non retypées : `pico8_qh_qh`, `pico8_rn`, les items de rangée (`lq.items`). Le site lit le membre typé et construit, à l'unique appel, la valeur que le lecteur par nom lui passait, par le constructeur existant. Chaque ligne qui gagne `valeur` pour cela est listée au message. Aucune fonction de conversion n'est ajoutée.


### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la lecture par nom est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours ; macro, `typedef`, constante, `enum`, pointeur ou fonction enveloppe qui renomme un identifiant vivant, ou un alias ; un registre nouveau (tableau, `switch` ou boucle qui passe d'un nom à un membre) ; un cast d'en-tête sans test de lignée, de famille ou de classe dans la même fonction ; une cale valeur ↔ typé (fonction de conversion, ou lecture par nom déplacée dans une aide non comptée) ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; supprimer un commentaire qui ne porte pas sur une construction retirée ou un contrat devenu faux ; tout changement d'une constante de hachage.
- **Classification exigée**, au message de chaque commit, pour chaque ligne `par-nom` retirée (recette : `git diff -U0` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) :
  - **(a1) site** : lecture par nom remplacée par l'accès typé ;
  - **(a2) construction supprimée** : définition d'aide sans appelant, aide `*_take_*` sans appelant ;
  - **(b)** : signature, déclaration ou variable dont le type change, attendu 0 pour `par-nom` ;
  - **(c)** : même donnée, autre nom, **attendu 0**.
- **Attendu, par chunk (`src/`, lignes, numéros de `d4c4f4c`)** :
  - b1, 22 lignes : (a1) `economy_score.c:59`, `economy_tiers.c:284`, `planet.c:635`, `planet_display.c:163`, `:223`, `:452`, `planet_trade.c:156`, `research_icon.c:126`, `:127`, `:128`, `:129` (11) ; (a2) `economy_score.c:25`, `:27`, `economy_tiers.c:31`, `:33`, `planet_display.c:39`, `:41`, `:49`, `research_icon.c:85`, `:89`, `:98`, `:103` (11).
  - b2, 6 lignes : (a1) `msgbox.c:368`, `:401`, `:411`, `:418`, `:452` ; (a2) `msgbox.c:49`.
  - `tests/` : b1 ajoute 1 ligne comptée (l'aide locale de l'oracle K appelle `kind->field`), déclarée comme ligne qui gagne `par-nom` (oracle de transition, retiré par D.11d/D.11e avec les `kind_field`) ; b2 retire 1 ligne comptée (`test_pico8_js.c:1876`, `{.field = p_field}` de la classe locale de `pz`, (a2)).
- **Mesure exigée**, avant et après chaque commit, par catégorie : les **lignes** (`make data-residue`, `--detail` par fichier) **et les occurrences** (recette figée de D.8, `$S` : dossier du scratchpad) :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur type-tag ref-audit par-nom; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  ```

  Mesuré sur `d4c4f4c` : `valeur` 4202, `type-tag` 547, `ref-audit` 45, `par-nom` 1028 (`src/` 405). Le détail par fichier (lignes et occurrences) de chaque fichier du diff est donné avant et après.
- **Contrôle Δlignes/Δoccurrences** : chacune des 28 lignes `par-nom` retirées de `src/` porte **une** occurrence (vérifié à `5e75832`, lignes inchangées depuis) : attendu Δlignes = Δoccurrences partout ; tout écart est justifié au message.
- **Lignes qui gagnent une catégorie**, listées par fichier et par catégorie, « aucune » compris. Attendues :
  - `valeur` : `msgbox.c` (item `ep`, argument de `rn`), `economy_score.c`/`planet_display.c` si une ligne nouvelle porte `pico8_value_*` pour la vérité de `qh.qh`, `test_pico8_js.c` si le cadre y construit une valeur, `test_pico8_entity_kind.c` (l'oracle compare des `pico8_value_t`) ;
  - `par-nom` : `test_pico8_entity_kind.c` (b1, oracle) ;
  - aucune autre ; chacune déclarée.

### Empreinte remesurée et découpage (G)

| Chunk | `src/` | `tests/` | autres | `par-nom` `src/` retirées |
|---|---|---|---|---|
| b1 | 13 : `struct.h`, `game_data.c/.h`, `planet_family.c/.h`, `planet_family_trade.c`, `help.c`, `research_icon.c`, `economy_score.c`, `economy_tiers.c`, `planet.c`, `planet_display.c`, `planet_trade.c` | 2 : `test_pico8_qh`, `test_pico8_entity_kind` | — | 22 |
| b2 | 5 : `msgbox.c/.h`, `help.c`, `menu.c`, `planet_display.c` | 1 : `test_pico8_js` | `docs/plan-3.md` (note) | 6 |

- b1 : 15 chemins, b2 : 7 : bien sous les ~40 de la règle. En tout, **16 fichiers distincts de `src/`, 3 TU** et le plan (la ligne dit ≈ 15 : `game_data.c/.h` et `planet_family.h` s'ajoutent aux fichiers de sites).
- **Pourquoi deux chunks** : chaque forme a sa preuve et ses mutations ; b1 réunit B et K parce qu'ils touchent la même paire `struct.h`/`planet_family*` et restent petits ; b2 change une signature publique (`pico8_pz`) et ses cinq appelants. Un seul commit (≈ 21 chemins) tiendrait sous la règle, mais mêlerait deux changements de contrat publics sans lien ; deux commits de taille D.11a/2 se prouvent séparément.
- **Pourquoi cet ordre** : b1 ajoute des membres sans rien retirer à personne ; b2, qui change une signature publique, vient ensuite et porte la note. L'ordre inverse marche aussi.
- Un fichier de `src/` peut être touché par les deux chunks (`help.c`, `planet_display.c`), un TU par un seul.

### Effet sur `make data-residue`

| Catégorie | Départ (`d4c4f4c`) | Après b1 | Après b2 | Pourquoi |
|---|---|---|---|---|
| `par-nom` `src/` | 345 / 405 occ. | **323** / 383 | **317** / 377 | exact : 22 puis 6 lignes, une occurrence chacune |
| `par-nom` `tests/` | 568 | 569 | 568 | oracle K (+1), classe locale de `pz` (−1) ; rapporté |
| `par-nom` total | 913 | 892 | **885** | la ligne dit ≈ 882 : +3, les lignes de `mt` (D.11e) |
| `valeur` | 3906 / 4202 | baisse, rapportée | baisse, rapportée | variables `pico8_value_t` des sites et des aides supprimées ; hausses déclarées plus haut |
| `type-tag` | 345 / 547 | = | − 1 (assert d'en-tête de `pz`) | rapporté |
| `ref-audit` | 44 / 45 | = | = | aucun `PICO8_REF_*` sur ces sites (à vérifier au diff) |

`par-nom` par fichier de `src/` (départ → arrivée) : `economy_score.c` 3→0, `economy_tiers.c` 3→0, `planet.c` 34→33, `planet_display.c` 7→1 (reste `:482`, `dw`, C), `planet_trade.c` 1→0, `research_icon.c` 17→9, `msgbox.c` 12→6. Tous les autres sont inchangés. « exact » est une égalité ; « rapporté » est mesuré au commit, écart expliqué.

### Documentation (G)

- `docs/plan-3.md` (b2) : note « Découvert à D.11b » en fin de ligne D.11b (texte en « Further Notes »), rien d'autre.
- `docs/glossaire-noms.md` : inchangé ; aucun nom minifié ne change de sens. Les noms nouveaux (`parent`, `pico8_qh_of`, `pico8_nl_of`, `pico8_kind_rec_t`, `rec`, `pico8_pz_frame_t`) sont des mots anglais ou composés de noms déjà glosés.
- `docs/trous-de-portage.md` : inchangé. TROU-3, TROU-7, TROU-9 restent à leurs sites. Aucun écart port/cart n'est créé. La branche « `hp` chaîne » retirée n'était atteignable par aucun lecteur : ce n'est pas un écart observable.
- Les PRD livrés, `docs/plan.md` et `docs/plan-2.md` sont historiques et ne sont pas touchés.

### Ce qui ne change pas

- `tools/`, le `Makefile`, `tests/e2e/`, tout `*.sha256`, tout fichier d'événements ; aucun e2e ajouté.
- Les lecteurs `field`, les écrivains `set_field`, les `kind_field` et le `field` des sortes, les aides `*_take_*` des écrivains et `pico8_entity_get_field`/`set_field` (D.11e) ; les vues `pj_cr`, `ha_pz`, `ie_ke_view` et leurs lecteurs.
- Les sites C et `mt` (D.11e), M et `ke.c:43` (D.11c) ; `pico8_jo_is_family` ; les accesseurs de D.11a.
- Les signatures de `pico8_ie_ke_view`, `pico8_jo_pq`, `pico8_da`, `pico8_dh`, `pico8_jo_cn` ; seule celle de `pico8_pz` change.
- L'ordre des appels et des tirages aléatoires (`rn` de `pz` avant les boutons), les fuites (TROU-7), les arrêts d'aujourd'hui.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `pico8_struct.h` peut inclure `pico8_game_data.h` sans cycle (constaté par grep : `game_data.h` inclut `fixed`, `geom`, `map_tables`, `vec`, aucun n'inclut `struct.h`). Sinon, déclaration anticipée par `struct pico8_kind_rec` étiquetée (précédent `struct pico8_qy`), déclarée.
- **H2** (leçon de D.11a) : aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste. Constaté par grep à `d4c4f4c` : `pico8_qh_of`/`pico8_nl_of` (définis dans `planet_family.c`, dans la liste) sont appelés depuis `economy_score.c`, `economy_tiers.c`, `planet.c`, `planet_display.c`, `planet_trade.c` (dans la liste) ; les enregistrements (`game_data.c`, dans la liste) sont nommés par `planet_family.c` et `planet_family_trade.c` ; `pico8_pz` (`msgbox.c`) par `planet_display.c`, `help.c`, `menu.c`. Les liens `parent` de `planet_family_trade.c` (hors liste) nomment `pico8_qh_struct_class`/`pico8_hj_struct_class` (dans la liste), celui de `help.c` (hors liste) `pico8_g__struct_class` (`probe.c`, dans la liste). Si un lien casse malgré tout, arrêt et PRD à reprendre : **le `Makefile` ne change pas**.
- **H3** : aucun appel de `pico8_ie_ke_view` n'a lieu pendant que le module d'une sorte, ou de `jo`, est arrêté (la garde « prêt » du `field` des sortes n'est pas reproduite par l'enregistrement). `test_pico8_ie` et `test_pico8_entity_kind` appellent la vue après `start_game`/init ; les e2e ne coupent aucun module en jeu.
- **H4** : aucune des filles de `jo` ne répond en propre à `ep`, `eo`, `hi`, `ff`, `ca`, `gq`, `ra` (grep des lecteurs) : le cadre d'une planète est exact.
- **H5** : aucun comportement du binaire de rejeu ne change : 26 e2e conformes.
- **H6** : aucune constante `*_HASH`/`identity[]` ne change ; les cas nouveaux ne plient rien dans une constante existante, et le cas retiré de `test_pz_closed` s'exécutait dans un fils (`must_abort`) sans constante.
- **H7** : `pico8_gb_find` perd son seul appelant de `src/` (la branche chaîne de `jo:pq`, `planet.c:641`) mais garde ceux de `test_pico8_hp`/`test_pico8_jo` : elle reste (hors `par-nom`, API testée), signalée dans la note.

## Implementation Chunks

Deux commits, dans cet ordre. Chacun est vert seul (`make test`, `make e2e`, `make data-guard`), avec ses comptes, sa classification, ses lignes gagnées, son `gcov` et ses mutations :

1. **b1 — lien parent (B) et enregistrements de sorte (K).**
   - `parent` dans `pico8_struct_class_t`, posé dans 10 descripteurs ; `pico8_qh_of()`, `pico8_nl_of()`.
   - Sites B ; aides `field_truthy` ×3, `field_fixed` de `planet_display` supprimées.
   - `pico8_kind_rec_t`, 9 enregistrements générés, `rec` dans `pico8_entity_kind_t` et dans les 9 sortes ; `pico8_ie_ke_view` sur `rec` ; `ie_ke_take_*` supprimées.
   - Cas ajoutés : `test_pico8_qh`, `test_pico8_entity_kind`.
   - 15 chemins, `par-nom` `src/` −22.
2. **b2 — cadre de `pz` (F).**
   - `pico8_pz_frame_t`, `pico8_pz` retypé ; cinq appelants.
   - `pico8_msgbox_field_vec`, `pico8_msgbox_truthy` supprimées.
   - `test_pico8_js` : `test_pz_fa_only_from_ca` sur un cadre, cas « liste » de `test_pz_closed` retiré.
   - Note « Découvert à D.11b » ; balayage ASan+UBSan des TU.
   - 7 chemins, `par-nom` `src/` −6, `tests/` −1.

Ordre conseillé dans chaque commit :
1. mesure de départ (lignes et occurrences par fichier) ;
2. en-têtes, puis `.c` de `src/`, puis TU ;
3. `make test`, `make e2e` ;
4. `gcov` et mutations (copie jetable, restauration par `cp`) ;
5. mesure d'arrivée, classification, lignes gagnées, greps de la « Sortie » ;
6. docs, message.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (valeurs, présences, identités, effets d'un appel), pas la présence d'une ligne. D.11b change la **forme** des lectures, pas ce qu'elles lisent. La preuve :
- chaque TU garde son intention et sa constante ;
- les 26 e2e restent conformes ;
- chaque ligne nouvelle de `src/` est exécutée par un TU (`gcov`) ou justifiée, et mord sous mutation ;
- un oracle de transition (K) compare l'enregistrement typé au `field` de la sorte, tant que ce `field` existe.

### TU touchés (G)

1. **`test_pico8_qh` (b1)** : un cas nouveau, sans constante. Pour chacune des 9 sortes de l'arbre et pour `jo` : `pico8_qh_of(e) == e` pour `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, NULL pour `jo` et `ga` ; `pico8_nl_of(e) == e` pour `nl` seul ; `pico8_qh_of(NULL) == NULL`. Il couvre la remontée de `parent` sur deux et trois niveaux (`pm`, `fr`) et prouve chaque lien posé. `test_pico8_qh` lie `$(ENGINE_SRCS)` (il construit `nr`/`fr`/`mx`, déjà aujourd'hui, l. 670).
2. **`test_pico8_entity_kind` (b1)** : un cas nouveau, sans constante, séparé de `test_ie_ke_view_by_name` (dont `RAW_PRESENCE_HASH`/`ABSENT_COUNT` ne changent pas) : pour chacune des 9 sortes (les 7 de `gx`, plus `pm` et `ga`, qu'aucune icône ne tient), chaque champ de `kind->rec` est égal à ce que rend `kind->field` pour `rr`, `s`, `me`, `od` — présence, valeur, et **identité** du pointeur pour les chaînes. Une seule ligne d'appel à `kind->field` (aide locale), déclarée.
3. **`test_pico8_js` (b2)** : `test_pz_fa_only_from_ca` passe un cadre sans `ca`, puis avec `has_ca = 1, ca = 14` : même intention (TROU-9), mêmes assertions (`fa` 11 puis 14) ; la classe locale `p_field` et `g_p_with_ca` partent. `test_pz_closed` garde `pico8_pz(NULL, NULL, 0)` ; son `must_abort(pz_on_a_table, &list)` et `pz_on_a_table` sont **retirés, listés au message** : un cadre typé ne peut pas être une liste, l'assert d'en-tête qu'ils prouvaient n'existe plus.
4. **Pas de pont**, aucun autre TU touché. Un TU qui casse sans nommer un symbole changé est un **bug de D.11b**, pas un TU à adapter : il part à `help-me-debug`. Avant chaque commit, l'implémenteur cherche dans `tests/` tout cas de mort ou toute constante qui dépend d'un contrat changé (`pico8_pz`, `pico8_ie_ke_view`, `pico8_jo_pq`, `pico8_da`, `pico8_dh`) — leçon de `fm_hy_on_provisional` à D.11a.
5. **Constantes** : aucune `*_HASH` ni `identity[]` ne change. Une constante qui rougit est un échec, jamais une mise à jour.
6. **§6** : aucun des trois tests n'est touché ; `test_pico8_planet_gh_row_lq` (lié sur `DATA_ENGINE_SRCS`, sous ASan) exerce les sites B et F de `jo:gh` et doit rester vert.
7. **Test ajouté** : aucun fichier nouveau. Au-delà des deux cas ci-dessus, une assertion nouvelle n'est admise que pour couvrir une ligne nouvelle de `src/` que `gcov` montre non exécutée, dans un TU existant de la classe, listée au message avec la ligne qu'elle couvre.
8. **Prior art** : lien de classe et appartenance : `pico8_jo_is_family`, `jc_is_endpoint_class` ; données générées et identité des chaînes : D.5c, `test_pico8_entity_kind` ; cadre local rempli par l'appelant : `pico8_js_keys_t`, `pico8_lq_keys_t` (D.6b, D.7a) ; tests de mort dans un fils : `must_abort`.

### Preuve des tests (§7 : `gcov` et mutations)

`gcov` compile les TU des classes touchées en `--coverage`, dans une copie jetable du scratchpad. Il doit montrer exécutés :
- b1 : `pico8_qh_of`/`pico8_nl_of` (branches trouvé, remonté, NULL), chaque site B (branches lignée et hors lignée), `pico8_ie_ke_view` sur `rec` ;
- b2 : `pico8_pz` avec et sans chaque membre du cadre, chacun des cinq remplissages.

Une ligne non exécutée est couverte par une assertion (règle 7) ou justifiée au message.

Mutations **une à la fois, dans l'arbre de travail**. Le fichier est sauvegardé par `cp` avant et restauré par `cp` après (jamais `git checkout`/`stash`). Chaque TU est lancé exactement comme le `Makefile` le lance ; un TU lié sous ASan l'est avec `ASAN_OPTIONS=detect_leaks=0`. Chaque mutation doit rougir au moins un TU, nommé au message :

- b1 :
  - **M1** `pico8_qh_of` ne remonte pas `parent` (test de classe exact) → `test_pico8_qh` (cas nouveau), `test_pico8_nl`/`_os`/`_hj` (`hp`, `px`) ;
  - **M2** `.parent` de `nr` mis à NULL → `test_pico8_qh` (cas nouveau : `fr`, `nr`) ;
  - **M3** `pico8_nl_of` rend toujours NULL → `test_pico8_nl` (`dh`, palette `qw`) ou le cas nouveau ;
  - **M4** l'enregistrement de `pm` prend `me` de `qh` au lieu de `pm` → `test_pico8_entity_kind` (cas nouveau) ;
  - **M5** l'enregistrement de `nl` pointe une copie de `"trade league"` au lieu de `nl_rr_text` → `test_pico8_entity_kind` (identité) ;
  - **M6** `jo:cn` lit `ba` NULL → `test_pico8_trade`/`test_pico8_nr` (acceptation d'un échange), sinon trou justifié.
- b2 :
  - **M7** `pz_step` ignore `has_ca` → `test_pico8_js` (`test_pz_fa_only_from_ca`) ;
  - **M8** le cadre de `fv:pq` met `ra` NULL → `test_pico8_fv` (position des boutons d'aide), sinon trou justifié ;
  - **M9** le cadre d'une planète omet `ep` → `test_pico8_jo`/`test_pico8_hp`/`test_pico8_planet_gh_row_lq` (ligne d'en-tête de `jo:gh`/`jo:fe`) ;
  - **M10** `pz` tient `ff` pour toujours faux → `test_pico8_fv` (`le` vidé), sinon trou justifié.
- Une mutation qui ne rougit rien est un trou : couvert par une assertion (règle 7) ou justifié au message.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit. Aucun e2e ajouté : les 26 passent par `pz` (menus, aide, panneaux de planète), `jo:gh`, `da`/`dh` et les sortes des icônes ; un e2e nouveau n'ajouterait de couverture qu'à des lignes que les TU couvrent déjà.
- `make class-coverage-net` : non exigé (aucun constructeur ne change) ; s'il est lancé, identique à `tools/class_coverage_net_expected.txt`.
- `make sanitize-net` : non (aucune durée de vie ni aucun propriétaire ne change).
- **Balayage ASan+UBSan des TU** à la fin de b2 : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.7b, D.10a, D.10b, D.11a. Sortie propre.

### Vérification, budget et tableau « Runs »

**Budget : environ 30 minutes de temps réel pour l'étape** (b1 ≈ 18, b2 ≈ 12), mesures et filets compris ; plafond 45 minutes. Base : D.11a a pris ≈ 35 minutes de temps réel pour ≈ 48 chemins ; D.11b en touche 22, mais ajoute deux oracles/cas et une signature publique. La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make test` | de l'ordre de la minute (recompilation large : `pico8_struct.h` change à b1) |
| `make e2e` | ~1 s |
| `make data-residue`, recette, classification, greps | < 1 min |
| `gcov` des TU touchés | quelques minutes par chunk |
| mutations (6 + 4, chacune un `make` ciblé du TU) | < 10 min |
| balayage ASan+UBSan des TU | quelques minutes, une fois |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été lancé. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements, et aucun cas nouveau n'est lent (un état construit, des lectures).

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement : `#`, commande exacte, état de l'arbre (HEAD + chunk + étape), code de sortie, ligne clé, durée. Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que les chemins de la table « Empreinte ». Ni `tools/`, ni `Makefile`, ni `tests/e2e/`, ni `*.sha256`.
2. **Greps** de la « Sortie » : exactement ce qui y est écrit, après b2 ; après b1, les greps 1 (première ligne et `ie_ke_take_*`), 2 (hors `msgbox.c`), 3, 4 et 5.
3. **Casts** : chaque cast d'un `pico8_struct_header_t *` vers une struct, ajouté ou gardé dans une fonction touchée, est précédé dans la même fonction d'un test de lignée (`pico8_qh_of`/`pico8_nl_of` le portent), de famille ou de classe. Relu dans le diff, cast par cast. Les appels de `pico8_pz` ne castent plus d'en-tête.
4. **Liens** : aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole nouveau défini hors de la liste (H2) ; `make test` le prouve pour les TU de cette liste.
5. **Sites** : chaque site B, F et K de l'inventaire est converti ; aucun site C, `mt`, M ni G (`ke.c:43`) n'est touché, en particulier `planet_display.c:482` (`dw`) et `slipway.c:131-257` (`mt`).
6. **Constantes** : aucune `*_HASH`/`identity[]` changée.
7. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (b) = (c) = 0 pour `par-nom` ; lignes gagnées listées ; aucune cale valeur ↔ typé, aucun registre, aucun alias.
8. **Tests retirés** listés avec leur raison. Mutations, `gcov` et balayage ASan+UBSan rapportés.
9. **Commentaires** : quatre sortes de `CLAUDE.md` (sens d'un nom minifié, écart connu, piège, contrat C). Les contrats nouveaux (`parent`, `rec`, `pico8_qh_of`/`pico8_nl_of`, cadre) disent ce que le nom ne dit pas (NULL, présence, qui garde quoi). Aucun renvoi à D.11b ni au plan dans `src/` ou `tests/`.
10. **Docs** : note « Découvert à D.11b » en fin de ligne D.11b ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `d4c4f4c` (ou un descendant qui ne touche que `docs/`), propre, ou si `make data-residue` n'y rend pas `par-nom` 913 (`src/` 345, `tests/` 568) sans explication, ou si une des aides que ce PRD tue n'y existe plus.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change, ou si un test devrait changer d'intention pour passer.
- `FAILED:` si le balayage ASan+UBSan des TU n'est pas propre, ou si `make class-coverage-net`, lancé, diffère de l'attendu versionné.
- `FAILED:` si un champ ou un site ne tombe sous aucune règle (arrêt « champ sans règle » du §7), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.

**Périmètre et types**

- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si un cast d'en-tête vers une struct n'a pas son test de lignée, de famille ou de classe dans la même fonction.
- `FAILED:` si un fichier de `DATA_ENGINE_SRCS` nomme un symbole défini hors de cette liste, ou si un lien de TU casse et que le `Makefile` est changé pour le réparer.
- `FAILED:` si une lecture par valeur par défaut change : 0 pour `qw`, `self.s` puis 43, pas d'icône sans `px`, rien de `ba`, 62 pour `rn`, vecteur nul pour `ra`/`gq`, `fa` de `js` sans `ca` — sur un receveur hors lignée ou sans le champ.
- `FAILED:` si un enregistrement de sorte diffère de ce que rend le `field` de la sorte (présence, valeur, identité des chaînes).
- `FAILED:` si un site C, une ligne de `mt`, un site M ou G (`ke.c:43`) est converti (D.11c, D.11e), si un lecteur `field`, écrivain `set_field` ou `kind_field` est supprimé (D.11e), ou si une vue (`pj_cr`, `ha_pz`, `ie_ke_view`…) est supprimée.
- `FAILED:` si un fichier de `tools/` ou le `Makefile` change.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert.
- `FAILED:` si `par-nom` `src/` ne vaut pas 323 après b1 et 317 après b2, ou si le détail par fichier diffère de la table sans écart expliqué.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, construction déplacée, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, alias, registre nouveau, cale valeur ↔ typé, lecture par nom déplacée dans une aide non comptée, code déplacé hors du compteur, ou une (c).
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) des lignes `par-nom` retirées ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; la justification de chaque couple Δlignes > Δoccurrences. Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si un cas de test retiré n'est pas listé au commit avec la raison qui le rend inexprimable.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si le plafond de 45 minutes est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.11b », ou si elle omet l'un de ces éléments : les comptes réels, le lien `parent` et `pico8_qh_of`/`pico8_nl_of`, `pico8_kind_rec_t`/`rec` (9 sortes, garde « prêt » non reproduite), `pico8_pz_frame_t`, l'oracle de transition laissé à D.11d/D.11e, `pico8_gb_find` sans appelant de `src/`.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Classe C et `jc.mt`** (D.11e, premier chunk ; ce que la première version de ce PRD en avait préparé est en Annexe C). **Classe M et G `ke.c:43`** (D.11c). **TU** (D.11d : T1/T2, les deux tests §6, l'oracle K de b1). **Infrastructure** (D.11e : lecteurs `field`, écrivains `set_field`, `*_take_*` des écrivains, `kind_field` et le `field` des sortes, `pico8_entity_get_field`/`set_field`, structs-vues).
- Retyper une API à valeur (`pico8_qh_qh`, `pico8_rn`, items de rangée, `pico8_jo_hf`) : Q12, ouverte.
- Les listes (`cg`, `lq.items`) : D.12. D.11b lit leurs éléments comme aujourd'hui.
- Réécrire `pico8_jo_is_family` sur `parent` ; supprimer `pico8_gb_find`.
- `valeur`, `type-tag`, `ref-audit` au-delà de ce que D.11b retire en passant.
- `tools/`, le `Makefile`, `tests/e2e/`, la phase 2, `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **B** : `parent` dans la table de classe, posé sur les 9 filles de `jo` et `fv` ; `pico8_qh_of`/`pico8_nl_of` remontent la chaîne, `const`. Rejetés : membres montés dans `jo`, test de classe exact par site, prédicat sur `states[]`.
  - **K** : `pico8_kind_rec_t` généré dans `pico8_game_data.c`, mêmes tableaux de chaînes, `rec` jamais NULL ; garde « prêt » non reproduite (H3). Rejetés : construction à l'init, accesseur `rec()`.
  - **F** : `pico8_pz_frame_t` `{ep, has_eo/eo, hi, ff, has_ca/ca, gq, ra}`, rempli sur la pile par les cinq appelants ; vues `pj_cr`/`ha_pz` gardées. Rejetés : neuf paramètres, accesseurs de classe, en-tête gardé à côté.
  - **Liens** : aucune dépendance nouvelle d'un fichier de `DATA_ENGINE_SRCS` vers un fichier hors liste (leçon de D.11a) ; le `Makefile` ne change pas.
  - **Tests** : cas `pico8_qh_of` dans `test_pico8_qh`, oracle K dans `test_pico8_entity_kind` ; un cas retiré dans `test_pico8_js`.
  - **Découpage** : b1 (B+K, 15 chemins), b2 (F, 7) ; un TU touché une fois ; la note à b2.
  - **Filets** : ni `class-coverage-net` ni `sanitize-net` exigés, balayage ASan+UBSan des TU à la fin.
- **Note du plan (G)**, à ajouter en fin de ligne D.11b (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.11b : deux chunks (b1 : N1 chemins ; b2 : N2). B (Q5) : `pico8_struct_class_t.parent` (9 filles de `jo`, `fv`), `pico8_qh_of()`/`pico8_nl_of()` (NULL hors lignée) ; la branche « `hp` chaîne » de `jo:pq` était inexprimable, retirée (`pico8_gb_find` sans appelant de `src/`, gardée pour ses TU). K : `pico8_entity_kind_t.rec` → `pico8_kind_rec_t` `{rr, has_s, s, me, has_od, od}`, généré dans `pico8_game_data.c` pour les 9 sortes (pas 11), mêmes chaînes que `pico8_X_start` ; la garde « prêt » du `field` des sortes n'est pas dans l'enregistrement ; D.11e peut supprimer le `field` des sortes une fois l'oracle de `test_pico8_entity_kind` retiré. F : `pico8_pz(const pico8_pz_frame_t *, …)`, cadre rempli par `fv:pq`, `cr`, `ix[4].my`, `jo:gh`, `jo:fe` ; vues `pj_cr`/`ha_pz` laissées à D.11e. `parent` est disponible pour C (D.11e). `par-nom` P (`src/` S) ; `valeur` V ; résidu D R.

  N1, N2, P, S, V, R sont les valeurs réelles.
- **Le message de chaque commit** porte : le périmètre du chunk ; la classification (a1)/(a2)/(b)/(c) des lignes `par-nom` retirées ; les comptes avant et après **en lignes et en occurrences**, par catégorie et par fichier, avec les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie, par fichier (« aucune » compris) ; les cas de test retirés et leur raison, les cas et assertions ajoutés et la ligne qu'ils couvrent ; les constantes (inchangées), les mutations, `gcov`, les filets ; le tableau « Runs » résumé ; les réponses aux hypothèses H qui le concernent.
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget) : casts, liens, anti-tassement, classification, sites laissés, constantes, cas retirés.
- **Surprises pour le pilote** :
  - **H-D11a-2 était fausse** : départ `src/` 345, pas 342 ; les trois lignes de `mt` sont à D.11e.
  - **K compte 9 sortes, pas 11** (la ligne D.11e dit encore « 11 sortes »), et la garde « module prêt » du `field` des sortes ne passe pas dans un enregistrement `static const` (H3).
  - **La branche « `hp` chaîne » de `jo:pq` est morte** depuis 1b.24 ; son retrait laisse `pico8_gb_find` sans appelant de `src/`.
  - **Un cas de `test_pico8_js` devient inexprimable** (une liste passée à `pz`).
  - **`par-nom` 885** après D.11b (la ligne dit ≈ 882 : les 3 lignes de `mt`).
  - **La forme C sera lourde à D.11e** : 13 accesseurs, 28 descripteurs dans 16 fichiers, ~170 lignes d'initialiseurs, plus les trois lignes de `mt` et leurs contraintes de lien (Annexe C).

## Annexe A — textes du plan recopiés (`docs/plan-3.md` à `d4c4f4c`)

Recopiés tels quels, sans retouche. Les listes d'étapes livrées de 1a (l. 93-149) et de 1b (l. 169-226) ne sont pas recopiées : historique livré, aucune de leurs lignes ne lie D.11b ; les règles de ces phases le sont. La section « Phase D » est recopiée en entier : ses puces (dont la **Sortie**), la note de D.0, le paragraphe « Liste de D » avec l'empreinte, les règles communes et l'ordre, toutes les lignes D.2 à D.12 avec leurs notes « Découvert à D.x » (D.0, D.2 à D.8, D.10a, D.10b, D.10c, **D.11a** ; D.1 et D.9 n'en ont pas), l'arrêt « champ sans règle », les décisions de D.9, les comptes au départ de D.10, et les lignes D.11b et D.11e révisées par la revue du plan après D.11a.

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

### §5, phase D, en entier (l. 228-280)

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
> - D.11b — `par-nom`, lectures sans receveur typé : B (7 + 1 ; test d'appartenance par le lien parent, `pico8_qh_of(jo)`, Q5), (C passe en tête de D.11e : ses descripteurs sont ceux que D.11e rouvre, revue du plan après D.11a), F (5 + 2 ; struct locale de cadre `{ep, eo, hi, ff, ca, gq, ra}` remplie par les cinq appelants de `pz`), K (4 ; enregistrement typé par sorte `{rr, has_s, s, me, has_od, od}`, résolu le long de la chaîne, pointé par `pico8_entity_kind_t`). Empreinte ≈ 15 (15, 0). Compte : `par-nom` ≈ 882. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non.
> - D.11c — `par-nom`, classe M (Q7) : émetteur de particules (`ei`, `ez`, `f_`, `g__mm`) et sonde (`bv`, `dl`, `d_`) → composants portés par pointeur, règle 2 de la phase 2 appliquée à ces deux groupes seulement. Empreinte ≈ 15 (15, 0) : `particle`, `probe` et les 13 fichiers des classes qui les portent. Compte : `par-nom` ≈ 862. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : oui.
> - D.11d — `par-nom`, TU : T1 (mise en état et lecture d'état) en accès aux membres typés ; T2 (tests des descripteurs, tests de mort des `assert` d'écrivain) supprimés comme tests de la couche (Q11, §6). Deux chunks d'environ 19 fichiers. Empreinte 38 (0, 38). Compte : `par-nom` ≈ 286 (l'infrastructure de `src/`). §6 : `test_pico8_planet_gh_row_lq` et `test_pico8_ga_export_panel_by` (3 lignes chacun) lisent les membres, preuve rouge. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change.
> - D.11e — `par-nom`, suppression de l'infrastructure. Premier chunk : classe C (14 sites dont `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` et `jc.mt` de `slipway.c`, laissé à D.11a ; accesseur typé par champ dans la table de classe, NULL = absent, Q6 ; l'oracle accesseurs = lecteurs naît et meurt dans l'étape). Puis : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 45 (42, 3), en 2 ou 3 chunks. Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non.
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

## Annexe B — étude D.9, classes B, C, F, K (`docs/prd/done/plan3-phaseD-etape9-etude.md`)

Lignes du tableau de classement (§1.1) ; C est gardée pour l'Annexe C :

> | Classe | Forme retenue | Lignes comptées | Appels d'aides | Fichiers |
> |---|---|---|---|---|
> | **B** — membre d'une fille lu sur la famille `jo` (`qh`, `nl`, `hp`, `px`, `s`, `ba`, `qw`) | test d'appartenance puis accès typé : `pico8_qh_of(jo)` rend NULL hors de la lignée (Q5) | 7 | 1 | 5 |
> | **C** — champ transversal lu sur une entité quelconque (`hv`, `cv`, calques de `dc`, `ui.ic`, `ui.o_`/`gj`, `jc.et`, `jo.fo`) : `p_`, `fa`, `ra`, `nu`, `ma`, `kq`, `dw`, `m_`, `ql`, `fg`, présence de `gv` et des cinq noms `bm` | accesseur typé par champ dans la table de la classe, NULL = absent (Q6) | 16 | 6 | 6 |
> | **F** — vue lue par `pz` : `p` peut être la vue `ha_pz`, `fv`, `jo`, `pj`… ; lit `ep`, `eo`, `hi`, `ff`, `ca`, `gq`, `ra` | struct locale de cadre (`{ep, eo, hi, ff, ca, gq, ra}`), remplie par chacun des cinq appelants depuis ses membres typés | 5 | 2 | 1 |
> | **K** — lecture des sortes : `pico8_ie_ke_view` lit `rr`, `s`, `me`, `od` par `kind->field` | enregistrement typé par sorte (`{rr, has_s, s, me, has_od, od}`), résolu le long de la chaîne à la génération, pointé par `pico8_entity_kind_t` | 4 | — | 1 |

Sites de l'annexe A de l'étude (mesurés à `2dec71f`), classes B, C, F, K, et les trois sites A que D.11a a reclassés en C :

```
B  economy_score.c    pico8_da                  R:qh
B  economy_tiers.c    pico8_dh                  R:nl
B  planet.c           pico8_jo_pq               R:hp
B  planet_display.c   pico8_jo_gh               R:px R:qh
B  planet_display.c   pico8_jo_kj               R:qw R:s
B  planet_trade.c     pico8_jo_cn               R:ba
C  bootstrap.c        pico8_dg                  R:p_
C  entity.c           pico8_dc                  R:fa R:ra
C  entity.c           pico8_lt / pico8_he       R:(noms bm) R:nu
C  entity_geom.c      pico8_gd / gk / ma / na   R:ra R:fa R:kq R:ma W:ma
C  planet_display.c   pico8_jo_kj               R:dw (sur jo.fo)
C  slipway.c          pico8_jc_hq               R:gv W:fo (sur jc.et)
C  ui.c               pico8_ui_ev / pico8_ui_gw R:ql R:fa R:fg W:m_ W:m_
F  msgbox.c           pico8_pz / pico8_pz_step  R:eo R:ep R:ff R:hi R:ra R:ca R:gq
K  research_icon.c    pico8_ie_ke_view          rr s me od (kind->field)
A  entity_geom.c      pico8_ij                  R:p
A  wormhole.c         pico8_mz_pq_ox_predicate  R:fx
A  wormhole_display.c pico8_mzd_ra_of_ij        R:ra
```

## Annexe C — Reporté à D.11e (classe C et `jc.mt`)

La revue du plan après D.11a a déplacé C en tête de D.11e (« ses descripteurs sont ceux que D.11e rouvre »). Ce qui suit est ce que la première version de ce PRD avait préparé pour C, **gardé pour le PRD de D.11e**, qui le remesure et le tranche à nouveau : rien de cette annexe ne lie D.11b, et aucun critère `FAILED:` de D.11b ne s'y rapporte. Les numéros de ligne des **sites** sont ceux de `d4c4f4c` ; ceux des **appelants** cités dans la colonne « Receveur » viennent de `5e75832` et sont à relire.

### C.1 Inventaire

**Classe C — champs transversaux (D.11e, premier chunk ; numéros de `d4c4f4c`).** 14 sites comptés et 4 définitions d'aides comptées (18 lignes), 8 appels d'aides, 8 fichiers ; dont les trois reclassés par D.11a (✱).

| Fichier:ligne | Fonction | Membre | Receveur (d'où il vient) | Remplacement | c/h/d |
|---|---|---|---|---|---|
| `bootstrap.c:87` | `pico8_dg` | R `p_` (vérité) | élément de `hv` (`pico8_ke_t *` après D.11a) : toute classe inscrite | `cls->get_p_ != NULL && cls->get_p_(qc)` ; `pico8_dg_truthy` (l. 19) perd son seul appelant : supprimée | c |
| `entity.c:237` | `pico8_lt` | R `nu` | toute entité inscrite | `tags = e->cls->get_nu != NULL ? e->cls->get_nu(e) : NULL` ; NULL n'inscrit rien, comme `all(nil)` | c |
| `entity.c:266` | `pico8_he` | R `nu` | idem | idem | c |
| `entity.c:300` | `pico8_dc` | R `fa` | élément de `cv("kj")`/`cv("ev")` | `cls->get_fa == NULL \|\| !cls->get_fa(qc, &fa)` → `continue` | c |
| `entity.c:329` | `pico8_dc` | R `ra` | idem | `ra = cls->get_ra != NULL ? cls->get_ra(qc) : NULL` | c |
| `entity_geom.c:36`, `:37`, `:42` | `pico8_gk` | R `fa`, `ra`, `kq` | `e` : `fv` (`help.c:36`), `jo` (`planet.c:47`), `ie` (`research_icon.c:275`), `js` (`ui_button.c:49`), ou NULL | accesseurs `get_fa` (absent → 0), `get_ra` (NULL → zéro), `get_kq` (NULL → 0) ; `e == NULL` garde les replis | h, h, c |
| `entity_geom.c:49` | `pico8_gd` | R `ra` | `e` : planète (`planet.c:844`, `planet_display.c:431`), émetteur (`particle.c:107`), extrémité de voie (`slipway_display.c:116` : planète ou point de souris) | `get_ra` (NULL → zéro) | h |
| `entity_geom.c:64` | `pico8_na` | R `ra` (écrit au travers) | `self` : `qh:ly` (`pico8_qh_ly_dispatch`, lignée `qh`) et `nl:ly` : famille `jo` seule | **cast vérifié** (exception, voir « C ») : `assert(pico8_jo_is_family(self) && "…")`, `((pico8_jo_t *)self)->ra` | c |
| `entity_geom.c:73`, `:82` | `pico8_ma` | R/W `ma` | `e` : `jo` (`planet_display.c:437`) ou `qb` (`scroll.c:195`), asserté par D.11a (H-D11a-7) | `pico8_fixed_t *ma = e->cls->get_ma(e)` (adresse du membre), lecture puis `*ma = clamped` | h, c |
| `entity_geom.c:104` ✱ | `pico8_ij` | R `p` | élément de `cg("jo")` (`galaxy.c:70`), `cg("np")` (`probe.c:220`) ou `mz.ox` (`wormhole.c:112` : planètes et `g_`/`fv`) | `get_p` (NULL → zéro) | h |
| `entity_geom.c:19`, `:27` | `pico8_entity_geom_field_vec`/`_fixed` | définitions | — | supprimées | d, d |
| `planet_display.c:482` | `pico8_jo_kj` | R `dw` | `self->fo` : ce qui accroche `fo` (`jo_is_fo_holder_class` : classe dont `hq` est `bv` — `g_`, `fv`, lignée `qh` — ou `jc`) | `get_dw` (absent → 0) | c |
| `slipway.c:263` | `pico8_jc_hq` | W `fo` | `self->et` = `jc:ce(self)` : planète, `jc`, ou la vue `jc_too_far` | `self->et->cls->set_fo(self->et, &self->header)` (membre NULL → assert gardé : « un écrivain générique nomme un champ sans membre ») | c |
| `ui.c:304` | `pico8_ui_ev` | R `ql` | `self->o_` : entité survolée (`cv("hd")`) | `get_ql` (absent → `self->ql`) | h |
| `ui.c:346` | `pico8_ui_gw` | R `fa` | élément de `cv("hd")` survolé | `get_fa` (absent → non posé en calque) | c |
| `ui.c:359`, `:363` | `pico8_ui_gw` | W `m_` | `self->gj`, `self->o_` | `cls->set_m_(e, 0)`, `cls->set_m_(e, 1)` (membre NULL → même assert) | c, c |
| `ui.c:367` | `pico8_ui_gw` | R `fg` | `self->o_` | `get_fg` (absent → 0) | h |
| `ui.c:56` | `pico8_ui_field_raw` | définition | — | supprimée (ses appelants `je.ra` sont partis à D.11a, H-D11a-6) | d |
| `wormhole.c:70` ✱ | `pico8_mz_pq_ox_predicate` | R `fx` (présence, puis vérité) | élément de `hv` | `cls->get_fx != NULL && cls->get_fx(p)` | c |
| `wormhole_display.c:41` ✱ | `pico8_mzd_ra_of_ij` | R `ra` | `mz.ij` ∈ `mz.ox` | `get_ra` (NULL → zéro) | h |
| `wormhole_display.c:21` | `pico8_mzd_field_vec` | définition | — | supprimée | d |

Qui répond à chaque nom aujourd'hui (lecteurs et écrivains lus, délégation à la parente comprise) — c'est l'ensemble exact des descripteurs qui reçoivent l'accesseur (règle « NULL = absent », voir « C ») :

| Accesseur | Descripteurs (fichier) | Nombre |
|---|---|---|
| `get_nu` | les 10 de l'arbre `jo` ; `g_`, `fv` (`probe.c`, `help.c`) ; `jc` ; `mz` ; `mh` | 15 |
| `get_fa` | les 10 de `jo` ; `g_`, `fv` ; `bg` ; `mh`, `iv` ; `ie` ; `pj` ; `jc` ; `ui` ; `js`, `fq` ; `mz` | 22 |
| `get_ra` | les 10 de `jo` ; `g_`, `fv` ; `lq` ; `nf` ; `iv` ; `ie` ; `qb` ; `jc_mouse_point` ; `js` ; `mz` | 21 |
| `get_p` | les 10 de `jo` ; `g_`, `fv` ; `qb` ; `jc_mouse_point` ; `mz` | 15 |
| `get_kq` | les 10 de `jo` ; `fv` ; `ie` ; `js` | 13 |
| `get_ma` | les 10 de `jo` ; `qb` | 11 |
| `get_dw` | les 8 de la lignée `qh` ; `g_`, `fv` ; `jc` ; `mz` | 12 |
| `get_ql` | `fv` ; `ui` | 2 |
| `get_fg` | `fv` ; `js` | 2 |
| `get_fx` | les 10 de `jo` ; `g_`, `fv` | 12 |
| `get_p_` | les 10 de `jo` ; `g_`, `fv` ; `mh`, `iv` ; `jc` ; `fq` ; `mz` | 17 |
| `set_m_` | les 10 de `jo` ; `fv` ; `bg` ; `ie` ; `js`, `fq` | 15 |
| `set_fo` | les 10 de `jo` ; `jc` ; `jc_too_far` ; `bv_et` | 13 |

Les autres descripteurs (`hy`, `hp`, `trade`, `jb`, `pj_cr`, `ha_pz`, `mz_jj`, `nr_ldown_empty`, `ie_ke_view`, et les remplaçants `mo`/`je`/`fm`) ne répondent à aucun de ces noms : membres NULL. Les « 10 de `jo` » sont `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx` ; la « lignée `qh` » les 8 sans `jo` ni `ga`.


**Ajouté par D.11a — `jc.mt` (3 lignes comptées, `slipway.c`)** : `pico8_jc_entity_value` (définition l. 131, ligne comptée l. 133), `jc_take_fixed` (définition l. 170, comptée), l'appel `jc_take_fixed(mt_val, &self->mt, &self->has_mt)` (l. 257), avec `qu_mt = pico8_jc_entity_value(self->qu, "mt")` et le repli sur `re` (l. 255-256). Qui répond à `mt` : `pico8_fr_field` (défaut propre de `fr`) seul, parmi les extrémités. **Contrainte de lien** (note de D.11a) : `slipway.c` est dans `DATA_ENGINE_SRCS`, `planet_family_trade.c` non ; nommer `pico8_fr_struct_class()` depuis `slipway.c` casse le lien de `test_pico8_bg_kj`. Un accesseur `get_mt` dans la table de classe (posé par le descripteur de `fr`, lu par `cls->get_mt` dans `jc:hq`) ne nomme aucun symbole de `planet_family_trade.c` depuis `slipway.c` : c'est la forme Q6, et elle lève la contrainte. À trancher par D.11e.

### C.2 Décisions préparées

(Titre d'origine : « C — accesseurs transversaux (G, b3) ». Les renvois à `b3` valent « premier chunk de D.11e ».)

```c
/* pico8_struct.h, dans struct pico8_struct_class, après `parent` */
    /* A NULL member: the class has no such field, as its `field` reader
     * would answer 0 (or its `set_field` refuse the name). A reader returns
     * NULL, or 0 leaving *out untouched, when the field is nil; `get_fx`
     * and `get_p_` return its Lua truth. `get_ma` returns the member's own
     * address. `set_fo` asserts what may hang `fo` on the class. */
    const pico8_str_seq_t *(*get_nu)(const void *self);
    int (*get_fa)(const void *self, pico8_fixed_t *out);
    const pico8_vec_t *(*get_ra)(const void *self);
    const pico8_vec_t *(*get_p)(const void *self);
    const pico8_qz_box_t *(*get_kq)(const void *self);
    pico8_fixed_t *(*get_ma)(void *self);
    int (*get_dw)(const void *self, pico8_fixed_t *out);
    int (*get_ql)(const void *self, pico8_fixed_t *out);
    int (*get_fg)(const void *self, pico8_fixed_t *out);
    int (*get_fx)(const void *self);
    int (*get_p_)(const void *self);
    void (*set_m_)(void *self, int m_);
    void (*set_fo)(void *self, pico8_struct_header_t *fo);
```

- **Un membre par champ, nommé** (Q6) ; aucun tableau, `switch` ni boucle qui passe d'un nom à un membre (ce serait un registre). Les signatures suivent l'usage des consommateurs : pointeur pour les pointeurs, `int` + sortie pour les fixes qui peuvent être nil, vérité pour les booléens lus comme vérité, adresse pour `ma` (lu puis écrit par `pico8_ma`).
- **Règle « NULL = absent »** (G) : un descripteur reçoit l'accesseur d'un nom **si et seulement si** son `field` (ou son `set_field`, pour `m_`/`fo`) répond à ce nom aujourd'hui, directement ou par délégation à sa parente. Table « Qui répond à chaque nom » : 13 noms, 170 lignes de descripteurs. Chaque accesseur rend exactement ce que rend la branche de son lecteur : même condition de présence (`has_X`, pointeur NULL), même valeur. Un écrivain fait ce que fait la branche de son `set_field` (présence posée, assert de genre gardé pour `fo`). Rejeté : ne poser l'accesseur que sur les classes « atteignables » par chaque consommateur (exige une preuve d'atteignabilité par consommateur ; `hv` et `cv` atteignent de toute façon presque toutes les classes ; un oubli lirait silencieusement « absent »).
- **Héritage** : les descripteurs de l'arbre `jo` nomment les accesseurs de `jo` (et `pico8_qh_get_dw` pour la lignée `qh`), `fv` ceux de `g_` pour les noms qu'il délègue, exactement comme ils nomment déjà `pico8_jo_kj_dispatch` (précédent D.10a). Rejeté : une résolution qui remonte `parent` quand le membre est NULL (machinerie nouvelle, et « NULL = absent » deviendrait « NULL sur toute la chaîne ») ; monter ces champs dans `ke` (option B de Q6, refusée par l'humain).
- **Noms** : `get_X`/`set_X` dans la table, `pico8_X_get_Y`/`pico8_X_set_Y` (ou statiques `X_get_Y`) pour les fonctions ; aucun ne correspond au motif `par-nom` (`field`, `_take_`, `field_truthy`). Ce ne sont pas des renommages : ce sont des accès typés nouveaux, et les lecteurs par nom restent (D.11e).
- **Consommateurs** : table C. Chaque site teste le membre (`cls->get_X != NULL`) avant l'appel, sauf `set_m_`/`set_fo`, dont l'absence est un arrêt : `assert(cls->set_m_ != NULL && "…")`, message qui nomme le contrat (« un écrivain générique nomme un champ que cette struct n'a pas — stop, n'ajoute pas de membre »), comme `pico8_entity_set_field` aujourd'hui.
- **Exception `pico8_na`** (G) : il écrit **au travers** de `ra` (`ra_vec->y += …`) ; `get_ra` rend `const pico8_vec_t *` parce que plusieurs classes pointent une donnée générée `const` (`qb`, `iv`, `g_`, `ie`). Ses deux seuls appelants sont `qh:ly` (`pico8_qh_ly_dispatch`) et `nl:ly` : famille `jo`. Retenu : `assert(self == NULL || pico8_jo_is_family(self))` puis `((pico8_jo_t *)self)->ra`, le cast vérifié de D.11a. Rejetés : un `get_ra` non `const` (retirerait le `const` d'une donnée générée par cast) ; un second accesseur mutable (un membre pour un seul site). Écart à Q6 déclaré au message et dans la note.
- **`pico8_ma`** : `get_ma` rend l'adresse de `jo.ma` ou `qb.ma` ; D.11a y a déjà l'assert de famille (H-D11a-7), qui reste.
- **Rejetés** : lire ces champs par un cast au site (receveurs ouverts : `hv`, `cv`, `ui.o_`, `mz.ox`, `jo.fo`) ; un seul accesseur générique à nom (c'est `field`).

### C.3 TU préparés

Classes locales de TU qui répondent par nom à un nom de C, et doivent gagner l'accesseur dans le commit qui fait lire ce nom par accesseur (sans quoi le TU lirait « absent » par hasard) :

- **`test_pico8_iv`** : `g_fake_toast_class` gagne `.get_p_`, qui rend la vérité de son `p_` ; `test_dg_drives_a_ke_struct` inchangé (le toast sort de `hv` au troisième `dg`). Son `.field` reste (D.11d).
- **`test_pico8_mo`** : `g_ra_only_class` gagne `.get_ra` ; `GD_NEAR`/`GD_FAR` inchangées. Son `.field` reste.
- **`test_pico8_ui`** : `g_giver_class` gagne `.get_fa` (15) et `.set_m_` (sans effet, comme son `set_field`) ; ses constantes inchangées. Plus un cas nouveau, **« accesseurs et lecteurs d'accord »**, sans constante, dans son propre état (un fils ou un `start_game` frais, pour ne rien plier dans les constantes existantes) : sur chaque entité de `hv` d'une partie démarrée avec galaxie, et sur une instance de chaque classe de la table « Qui répond » absente de `hv` quand le TU sait la construire, pour chacun des 11 lecteurs, **une comparaison écrite par nom** (pas de table de noms) : membre NULL ⇔ `pico8_entity_get_field` rend 0 ; sinon même présence et même valeur (pointeur identique pour les REF). Les classes que le TU ne peut pas construire (vues statiques de `slipway.c`, `bv_et`) sont couvertes par leurs TU existants (`test_pico8_jc`, `test_pico8_g_`) ou justifiées au message. Ses lignes `pico8_entity_get_field` gagnent `par-nom`, déclarées ; D.11d/D.11e le retirent avec les lecteurs.

`test_pico8_planet_kj_dispatch` copie la table de `jo` (`g_spied_jo = *pico8_jo_struct_class()`) : la copie emporte les membres nouveaux. Classes locales à `d4c4f4c` : `test_pico8_iv.c:227`, `test_pico8_mo.c:107`, `test_pico8_ui.c:839` (`giver_t` porte désormais un `pico8_ke_t`, l. 800-801).

### C.4 Mutations préparées

  - **M11** `get_nu` de `jo` rend NULL → presque tous les TU de planète (`cg("jo")` vide) ;
  - **M12** `get_fa` de `js` absent → `test_pico8_ui`/`test_pico8_js` (calques de `ui.ic`, `dc`) ;
  - **M13** `get_p_` de `fq` rend 0 → `test_pico8_fq` (le bouton ne sort plus de `hv`) ;
  - **M14** `set_m_` de `jo` retiré de son descripteur → `test_pico8_ui` (survol d'une planète : l'assert s'arrête) ;
  - **M15** `pico8_ma` n'écrit plus `*ma` → `test_pico8_qb`/`test_pico8_jo` (décroissance de `ma`) ;
  - **M16** `get_fx` de `g_` retiré → `test_pico8_mz` (`mz.ox` sans sonde) ;
  - **M17** `set_fo` de `jc_too_far` retiré → `test_pico8_jc` (voie trop longue : l'assert s'arrête) ;
  - **M18** `get_ra` de `jc_mouse_point` retiré → `test_pico8_jc` (`gd` du point de souris) ;
  - **M19** un accesseur rend « présent » là où son lecteur rend 0 (par exemple `get_dw` de `jc` sans condition) → l'oracle de `test_pico8_ui`.

### C.5 Coût mesuré

13 accesseurs (plus `get_mt` si D.11e le retient), 28 descripteurs dans 16 fichiers, ~170 lignes d'initialiseurs et ~60 petites fonctions, pour 18 lignes comptées (21 avec `mt`) ; 25 fichiers de `src/` et 3 TU à ce seul chunk, plus l'oracle. Précédent de forme : la table de méthodes typée de D.10a, dont chaque descripteur nomme explicitement les fonctions de sa parente.

