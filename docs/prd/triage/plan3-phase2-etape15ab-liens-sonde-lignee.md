---
title: "Plan 3, étapes 2.15a et 2.15b : liens `emitter`/`probe` au noyau, sonde aplatie, masque `lineage` ; `lien-parent` 109 → 51 → 42 ; `void-ptr` 7 → 3 ; `parente` 10 → 9 — deux commits, jeu identique, aucune empreinte changée"
description: "2.15a : deux liens mutables du noyau (`emitter`, `probe`), posés à `&self->X_own` par l'init de chaque porteur ; `emitter_of`, `probe_of`, leurs sept implémentations et quatre prototypes retirés ; lecteurs `particle.c`, `probe.c`, `planet_display.c`, TU `ui` sur les liens ; TU `struct_header` : deux liens de plus, les dix sortes, `g_`, `mz`, `mh`, `iv` construits (réserve du vérificateur de 2.14d), vérité de `*fx` de la sonde épinglée. 2.15b : `pico8_fv_t` disparaît (`fv` est une table et un archétype sur `pico8_g__t`, qui reçoit `m_`) ; membre `lineage` dans la table de classe, enum de quatre bits, douze masques ; `pico8_g__of` remplacé par le bit `G_`, `pico8_jo_is_family` par le bit `JO` ; `.parent` de `fv` retiré. Équivalence masque/descentes prouvée sur les 40 tables de `src/` par un programme lié aux objets. Essayé sur copie : vert, 26 e2e (aussi sous perturbation), 24 jumelles et 5 lignes K sans ÉCHEC. Aucune question à l'humain."
status: needs-triage
---

## Problem Statement

**Écrit sur `cecbcdb`** (branche `claude/plan-pilot-3-7zdft0`), qui porte 2.14d1 (`1f29e78`), 2.14d2 (`86c94b8`) et leurs notes ; `cecbcdb` ne diffère de `86c94b8` que dans `docs/`. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Essais sur une seule copie jetable (scratchpad `prd215ab/copie` et ses arbres de travail, supprimés depuis). Notation : `p` = `cecbcdb`, l'arbre d'après 2.14d2 ; `a` = `d650513`, `b` = `644e08d`, les commits de la copie qui portent les deux diffs de « Ce qui change ». Numéros de ligne : ceux de `p`, sauf mention. Historique : l'essai est parti de `1f29e78` + le diff 2.14d2 du PRD 2.14c–d (appliqué sans reste, arbre `src/ tests/ Makefile tools/` identique à `86c94b8` : `git diff --stat` vide), puis a été repris par `cherry-pick` sur `cecbcdb` ; toutes les mesures finales sont sur `cecbcdb` (Runs).

Vu du mainteneur du port : après 2.14, le code générique lit encore l'émetteur de particules et la sonde d'une entité par deux méthodes de table (`e->cls->emitter_of(e)`, `e->cls->probe_of(e)`, sept implémentations qui rendent un pointeur constant), demande « est-ce une sonde ? » en remontant la chaîne `parent` (`pico8_g__of`), « est-ce une planète ? » en comparant un pointeur de fonction d'état (`pico8_jo_is_family`), et l'aide `fv` enchâsse la sonde (`pico8_fv_t {pico8_g__t g_; int m_;}`), une des dix parentés qui restent. 2.15a et 2.15b posent les deux derniers liens de composant du noyau, aplatissent la sonde et remplacent les descentes `G_` et `JO` par un masque de lignée lu dans la table de classe ; 2.15c et 2.15d feront de même pour la planète.

### Texte du plan, recopié (`docs/plan-3.md` à `cecbcdb`)

> - 2.15 — `parente` et `lien-parent`, selon l'étude 2.10 (§2). Sonde et émetteur par deux liens du noyau ; porteur de sorte : un masque `lineage` dans la table de classe (bits `JO`, `QH`, `NL`, `G_`), équivalent aux descentes sur les douze classes (étude, contrôle 4) ; `pico8_jo_is_family` garde son nom et lit le bit `JO`. Forme sous A : un type planète de 33 membres (432 octets), un type sonde de 8 (264 octets), structs complètes dans l'étude. (2.9 : les archétypes ont déjà la forme aplatie. L'aplatissement ne les réécrit pas.) Preuves : chemin touché (mutations : lien `probe` NULL sur une sorte de la lignée `qh`, bit `QH` ôté de `nr`, bit `JO` ôté de `ga` → rouge) ; TU d'équivalence réécrit sur le masque ; 2.15d, objets sans `-g` identiques pour les modules dont seul un chemin change, sinon chemin touché. `class-coverage-net` : oui (comptes égaux à `tools/class_coverage_net_expected.txt`). `sanitize-net` : oui. Budget ≈ 4,5 × 50 min.
>   - 2.15a — liens `emitter`/`probe` posés par l'init de chaque porteur ; `emitter_of`, `probe_of` et leurs sept implémentations retirés (lecteurs `particle.c`, `probe.c`, `planet_display.c:468-469`, `test_pico8_ui.c:855-909`). Les membres `emitter`/`probe` des types restent (même init). Compte : `lien-parent` 109 → **51** ; `void-ptr` 7 → **3**.
>   - 2.15b — sonde aplatie (`help.h:21`) : `pico8_fv_t` disparaît, `pico8_g__t` reçoit `m_` ; membre `lineage` et ses douze masques ; `pico8_g__of` remplacé par le bit `G_` ; `.parent` de `fv` retiré. Compte : `parente` 10 → **9** ; `lien-parent` → **42**.

Décisions de l'humain (même fichier) : **planètes, A** (2026-09-30) — « un seul type "planète" pour les dix sortes, et de même un seul type pour la sonde et sa variante d'aide » : 2.15b fait la moitié « sonde » (`fv` n'a plus de type à lui). **Q-2.10, oui** — « chaque objet reçoit les liens de composant du noyau (environ 100 octets de plus par objet sur PC, 50 sur la PSX) » : 2.15a en ajoute deux, 16 octets. Notes « Découvert à 2.10 » : forme (liens `&def->X`/`&self->X`/NULL, posés une fois par l'init) ; « porteur de sorte : un masque `lineage` dans la table de classe […], équivalent aux quatre descentes sur les douze classes ; les étiquettes ne le sont pas (sept sortes différeraient) » ; « planète aplatie : 56 noms → 20 à l'archétype, 3 retirés par 2.14, 33 membres (432 octets) ; sonde : 21 → 11, 2, 8 (264 octets) ». Notes « Découvert à 2.11 » : `particle` ↔ `probe` est déjà tombée à 2.14b ; `planet_display` ↔ `planet_family` tombe à 2.15d, pas ici ; « le masque de lignée de 2.15b (bit `G_`) ne […] distingue pas » `g_` de `fv` (fiche 12, pour 2.17).

Étude 2.10, §2 (ce que ce PRD suit) : table « Découpage des 111 lignes » (2.15a − 58, 2.15b − 9) ; contrôle 4 (le masque d'une classe est sa chaîne `parent` restreinte à `jo`, `qh`, `nl`, `g_`) ; « deux liens du noyau, `struct pico8_emitter *emitter` et `struct pico8_probe *probe`, NULL pour une classe qui n'en porte pas, posés par l'init à `&self->emitter_own`/`&self->probe_own` […]. L'invariant "lien = membre" est posé par la même init et vérifié par un TU par classe porteuse » ; « la sonde d'abord : une `parente` (`help.h:21`), 15 lignes de `help.c`, deux TU ».

Règles communes du plan (`docs/plan-3.md:369`) : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée ; `class-coverage-net` et `sanitize-net` selon la ligne (ici : oui, oui). Ce PRD les tient toutes, aux deux commits.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que font 2.15a/2.15b |
|---|---|
| Noyau à quatorze membres, 112 octets ; « `emitter`/`probe` se déclarent à la suite, l'init les met à NULL sans changement » (PRD 2.14c–d, contrat pour 2.15a) | tenu : deux membres déclarés après `m_`, `pico8_struct_header_init` inchangée ; noyau **128** octets |
| `assert_links` : un lien de plus = une ligne ; l'attendu des porteurs d'émetteur et de sonde (idem) | **a** : deux lignes dans `assert_links` ; attendus de `fv`, `fr` (sonde), `jc` et le `jc` tiré d'un `fr` (sonde) ; test neuf `test_component_carriers_link_their_own` : `g_`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `ga`, `mz`, chacun comparé sur ses seize liens, plus « lien = membre » (`X->probe == X->header.probe`) |
| **Réserve du vérificateur de 2.14d** (coordinateur) : le TU ne construit ni `mz`, ni `g_`, ni `iv`, ni `mh` ; un lien parasite posé sur l'un d'eux ne serait pas vu | **a** : les quatre sont construits et comparés lien par lien (`mh` {`fa`, `p_`}, `iv` {`fa`, `ra`, `p_`}, `g_`, `mz`) ; jumelles **N-mz-emitter** et **N-iv-probe** : `struct_header` rouge à l'enfant (preuve 3) |
| **Détail du même vérificateur** : la vérité de `*fx` lue sur le `def` de `g_`/`fv` repose sur un archétype à 1.0 (en Lua, 0 est vrai) ; si 2.15b touche `fx` de la sonde, garder ou épingler ce contrat | 2.15b ne touche pas `fx` (le lien de `fv` reste `&def->fx`, `pico8_fv_def`) ; **a** l'épingle quand même : `assert(*g_->header.fx == pico8_fixed_from_int(1) && "…")` sur un `g_` et sur `fv`, avec message (piège qu'on réintroduirait) |
| « `pico8_d_` lit encore `self->cls->probe_of` » (idem) | **a** : `d_` lit `self->probe` |
| « `pico8_g__of` : `probe.c` ne l'emploie plus que dans `probe_host_set_ra_p` (`probe.c:82`) et sa définition » (idem) | **b** : `probe_host_set_ra_p` teste le bit `G_` puis prend la sonde ; `pico8_g__of` et son prototype partent, avec ses quatre appels de TU (`test_pico8_fv.c:251`, `test_pico8_g_.c:306`, `:664`, `:689`) |
| liens mutables : `ma`, `p_`, `m_` ; « tout autre lien est `const` » (idem) | `emitter` et `probe` sont **mutables** : `f_` écrit `emitter->qy`, `ez` étend la liste `qy`, `bv` écrit `probe->et` (preuve 2 : les rendre `const` donne exactement ces trois erreurs) ; le commentaire du noyau le dit |
| TU `struct_header` relancé sous `MALLOC_PERTURB_=90` et tcache coupé (idem) | inchangé ; chaque porteur neuf s'y construit donc sur un bloc rempli de `0xa5` |
| `particle.c:12` (`pico8_planet.h`) sans usage : pour 2.17 (idem) | non touché |
| `test_pico8_qh.c:705-724` fait le contrôle 4 pour `qh_of`/`nl_of` ; « 2.15c le réécrit sur le masque » (étude 2.10, §2) | **b** : le même test compare aussi le masque aux deux descentes, et `lineage` vaut `G_` pour `g_`, `fv`, 0 pour `qb`, `bg`, `mz`, `jc` (G3) ; 2.15c n'aura qu'à retirer le côté descente |

### État de départ attendu (`cecbcdb`, ou un descendant qui ne touche que `docs/`)

Mesuré sur `p` :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 109 27`, `void-ptr 7 3`, `inclusion-mutuelle 16 15` ; **`résidu 2 : 162 ligne(s), 45 fichier(s)`** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 238 ; 0 écriture : 15 ; 0 hors constructeur : 20` |
| `--check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 40 ligne(s)` |
| `COMPOSITION_RESIDUE_GUARD` (`Makefile:874`) | `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6` |
| casts / tests de classe : `grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests \| wc -l` ; `grep -rEo '(->cls\|header\.cls\|_struct_class\(\))' src tests \| wc -l` | 487 / 656 |
| `sizeof` (x86-64) : noyau, `g_`, `fv`, `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `ga`, `nr`, `fr` | 112, 240, 248, 288, 328, 336, 328, 392, 392, 400, 360, 328, 328 |
| graphe d'usage (`usegraph.py`, annexe A du PRD 2.11) | `paires 19` |
| `make test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | vert ; 26 conformes ; 26 conformes |

Un départ autre : arrêt, rapporté. Contrôle rapide : `git diff cecbcdb <départ> -- src tests Makefile tools` doit être vide ; sinon les diffs ci-dessous peuvent ne plus s'appliquer (`git apply --check`), et le PRD est à rejouer.

## Solution

Deux commits, chacun vert, jeu identique (26 e2e conformes, aussi sous `MALLOC_PERTURB_=165`), aucune constante d'empreinte changée.

**2.15a — liens `emitter`, `probe`** (`lien-parent` 109 → **51**, `void-ptr` 7 → **3**) :

- `pico8_struct_header_t` gagne `struct pico8_emitter *emitter` et `struct pico8_probe *probe` (déclarations anticipées remontées avant le noyau) ; `emitter_of`, `probe_of` quittent `pico8_struct_class_t` avec leur commentaire ; les sept implémentations (`pico8_g__emitter_of`, `pico8_g__probe_of`, `pico8_qh_probe_of`, `pico8_hj_emitter_of`, `ga_emitter_of`, `mz_probe_of`, `jc_probe_of`), leurs quatre prototypes publics (`probe.h:52-54`, `planet_family.h:36-37`, `:62-63`) et leurs dix-huit entrées de table partent.
- Poses, une par init de porteur, à `&self->X_own`, à côté de la pose du membre : `pico8_g__struct_init` (émetteur et sonde de `g_` et de `fv`), `pico8_qh_struct_init` (sonde des huit sortes de la lignée `qh`), `pico8_hj_struct_init` (émetteur de `hj`, `pm`, `mx`), `_new` de `ga` (émetteur), de `mz` et de `jc` (sonde). Les membres `emitter`/`probe` des types restent.
- Lecteurs : `f_`, `ez`, `ei` (`particle.c`), `bv`, `dl`, `d_`, `g_:mm` (`probe.c`), le rayon de sonde de `jo:kj` (`planet_display.c:468-469`) lisent `self->emitter`/`self->probe` ; les `assert` gardent leur message, sur la présence du lien. `ez` garde son sens (lien NULL : aucun ajout).
- TU : `test_pico8_ui.c` (`assert_own_emitter`/`_probe`, `test_components_carried`) lit les liens ; `test_pico8_struct_header.c` compare seize liens, construit `mh`, `iv`, `g_`, les huit sortes de la lignée `qh` (dont `jc` et `fr` déjà là), `ga`, `mz`, vérifie « lien = membre » et épingle `*fx` = 1.0 sur `g_` et `fv`.

**2.15b — sonde aplatie, masque de lignée** (`parente` 10 → **9**, `lien-parent` 51 → **42**) :

- `struct.h` : enum `pico8_lineage_t` (`PICO8_LINEAGE_JO`, `_QH`, `_NL`, `_G_`), macro `PICO8_LINEAGE(l)`, membre `unsigned lineage` de `pico8_struct_class_t` (après `tags`, comme lui) ; douze masques posés dans les tables : `jo`, `ga` → `JO` ; `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `fr` → `JO|QH` ; `nl` → `JO|QH|NL` ; `g_`, `fv` → `G_`. Les 28 autres tables de `src/` et les trois de `tests/` laissent 0.
- `pico8_jo_is_family` lit le bit `JO` (au lieu de l'identité `states[PICO8_STATE_LL] == pico8_jo_ll_dispatch`) ; `probe_host_set_ra_p` lit le bit `G_` puis prend la sonde (cast) ; `pico8_g__of` part (définition, prototype, marche `probe.c:478`).
- `pico8_fv_t` disparaît (`help.h:16-21`) : `pico8_g__t` reçoit `int m_` (dans le trou de bourrage après `p_` : `sizeof` inchangé, 256), `pico8_fv_new`, `pico8_fv_pq`, `pico8_fv_kj` prennent et rendent `pico8_g__t *` ; `self->g_.X` → `self->X` ; `.parent = pico8_g__struct_class` de `fv` retiré. `g_` ne pose pas de lien `m_` et laisse son `m_` indéterminé, comme avant (il n'en avait pas).
- TU : `test_pico8_fv.c`, `test_pico8_g_.c`, `test_pico8_spec_defaults.c`, `test_pico8_ui.c`, `test_pico8_struct_header.c` suivent le type ; les trois `is_help`/`is_probe` testent le bit `G_` ; `test_pico8_qh.c` (`test_qh_of_follows_parent`) compare le masque aux descentes sur les dix sortes et les quatre non-planètes.

Rien d'autre : ni `glossaire-noms.md` (`lineage` n'est pas un nom minifié), ni `trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni le `Makefile` (aucune catégorie n'atteint 0 : pas de garde neuve), ni `particle.c:12`.

## User Stories

1. En tant que mainteneur, je veux que le code générique lise l'émetteur et la sonde d'une entité par un lien du noyau, afin que la table de classe ne serve plus à rendre un pointeur constant par un appel indirect.
2. En tant que mainteneur, je veux savoir si une classe descend de `jo`, `qh`, `nl` ou `g_` par un bit de sa table, afin qu'aucune lecture ne remonte une chaîne `parent` et que 2.15c puisse retirer cette chaîne.
3. En tant qu'auteur de 2.15c, je veux que les bits `QH`/`NL` soient déjà posés et épinglés contre `pico8_qh_of`/`pico8_nl_of`, afin de n'avoir qu'à remplacer les lecteurs.
4. En tant que gardien de la mémoire, je veux que chaque porteur neuf soit construit dans le TU `struct_header` sous perturbation, afin qu'un lien oublié ou parasite rougisse.
5. En tant que gardien de la règle du parent, je veux qu'aucune mutation rouge au parent ne devienne verte à l'enfant.
6. En tant qu'humain qui a tranché A, je veux que l'aide `fv` n'ait plus de type à elle : une table et un archétype sur le type sonde.

## Implementation Decisions

### Modules

- **a** : `struct.h` ; lecteurs `particle.c`/`.h`, `probe.c`/`.h`, `planet_display.c` ; porteurs `probe.c` (`g_`, `fv`), `planet_family.c`/`.h` (`qh`, `hj`, `ga` et leurs tables), `planet_family_trade.c` (tables `nr`, `fr`, `mx`), `help.c` (table `fv`), `slipway.c` (`jc`), `wormhole.c` (`mz`) ; TU `ui`, `struct_header`.
- **b** : `struct.h` ; `help.c`/`.h`, `probe.c`/`.h`, `planet.c`, `planet_family.c`, `planet_family_trade.c` ; TU `fv`, `g_`, `qh`, `spec_defaults`, `struct_header`, `ui`.

### Inclusions

Aucune ligne `#include` ajoutée ni retirée dans `src/`, aux deux commits : `inclusion-mutuelle` **16 / 15, 40 lignes** partout ; graphe d'usage **19** partout (mêmes paires, mêmes symboles). Les TU incluent davantage : `struct_header` en **a** (`msgbox`, `planet_family`, `probe`, `wormhole`), `qh` en **b** (`bg`, `slipway`, `wormhole`), pour les classes qu'ils construisent. `planet_display.c` n'appelle plus `probe_of` mais n'employait pour cela aucun nom de `probe.h` : l'arête `planet_display` → `probe` ne bouge pas.

### Ce qui change

Les deux diffs ci-dessous sont le texte exact (`git diff p a`, `git diff a b`) ; `git apply --check` puis `git apply` des deux sur `cecbcdb` rendent `b` à l'identique (Runs 20). Lecture, en plus de la Solution :

- **Un lien, pas une méthode** : le lien vaut ce que la méthode rendait (le membre, lui-même `&self->X_own`), pour chaque classe ; les deux sont posés côte à côte dans la même init. Le programme de la preuve 5 lit, sur 29 classes, la présence de chaque composant et les valeurs que les lecteurs génériques en tirent : identique à `p`, `a`, `b`.
- **Mutables** : le noyau n'avait que des liens `const` hors `ma`, `p_`, `m_` ; `emitter` et `probe` visent l'instance (jamais `def` : un `pico8_emitter_def_t` n'est pas un `pico8_emitter_t`, preuve 2) et sont écrits à travers par `f_`, `ez`, `bv`.
- **`ez`** : l'ancien `emitter_of != NULL ? emitter_of(self) : NULL` devient `self->emitter` : NULL pour un non-porteur, par l'init du noyau.
- **Masque, pas étiquettes** : les étiquettes ne disent pas la lignée (étude 2.10, §2 : `ga` n'en a aucune, `nr`/`fr` n'ont pas `jo`) ; le masque d'une classe est sa chaîne `parent` restreinte aux quatre classes (preuve 4, sur les 40 tables).
- **`pico8_jo_is_family`** : même réponse sur les 40 tables (preuve 4) et sur la copie de table de `test_pico8_planet_kj_dispatch.c:89` (`g_spied_jo = *pico8_jo_struct_class()` copie aussi `lineage`, comme elle copiait `states`).
- **Aplatissement de la sonde** : `fv` n'a plus de struct ; ce qui la distingue de `g_` est sa table (`g_fv_struct_class`), son archétype (`pico8_fv_def`) et un lien `m_` que seul son `_new` pose. `m_` loge dans le bourrage de `pico8_g__t` : une sonde reste à 256 octets et une aide passe de 264 à 256.
- **Commentaires** (règle de `CLAUDE.md`) : celui du noyau dit qui écrit à travers les deux liens ; celui de `particle.h` nomme le lien ; celui de la table perd le paragraphe `emitter_of`/`probe_of` ; l'enum et le membre `lineage` portent leur contrat (une ligne chacun) ; celui de `pico8_g__t` dit que `m_` est à `fv` et que `g_` n'a pas de lien `m_` ; la phrase « `g_.ra` points at `pico8_fv_def`'s » passe sur `pico8_fv_new` (sous le seul `TROU-8` : `pico8_g__t` porte déjà `TROU-7`) ; l'`assert` de `*fx` porte son message.

#### Diff de 2.15a (`git diff p a`)

```diff
diff --git a/src/pico8_help.c b/src/pico8_help.c
index 896277c..f4d0ad2 100644
--- a/src/pico8_help.c
+++ b/src/pico8_help.c
@@ -108,8 +108,6 @@ static const pico8_struct_class_t g_fv_struct_class = {
     .ey = pico8_d__dispatch,
     .mm = pico8_g__mm,
     .parent = pico8_g__struct_class,
-    .emitter_of = pico8_g__emitter_of,
-    .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
diff --git a/src/pico8_particle.c b/src/pico8_particle.c
index 0c82e0c..2f9c995 100644
--- a/src/pico8_particle.c
+++ b/src/pico8_particle.c
@@ -32,9 +32,9 @@ void pico8_f_(pico8_struct_header_t *self) {
     if (self == NULL) {
         return;
     }
-    assert(self->cls->emitter_of != NULL && "f_: a class that carries no particle emitter has no `qy` -- stop");
+    assert(self->emitter != NULL && "f_: a class that carries no particle emitter has no `qy` -- stop");
 
-    emitter = self->cls->emitter_of(self);
+    emitter = self->emitter;
     qy = malloc(sizeof *qy);
     pico8_ptrs_init(qy);
     emitter->qy = qy;
@@ -47,7 +47,7 @@ void pico8_ez(pico8_struct_header_t *self, pico8_fixed_t n) {
     if (self == NULL) {
         return;
     }
-    emitter = self->cls->emitter_of != NULL ? self->cls->emitter_of(self) : NULL;
+    emitter = self->emitter;
     qy = emitter != NULL ? emitter->qy : NULL;
 
     while (pico8_fixed_rnd_default() < n) {
@@ -75,9 +75,9 @@ void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *s
     if (self == NULL) {
         return;
     }
-    assert(self->cls->emitter_of != NULL && "ei: a class that carries no particle emitter is no receiver -- stop");
+    assert(self->emitter != NULL && "ei: a class that carries no particle emitter is no receiver -- stop");
 
-    emitter = self->cls->emitter_of(self);
+    emitter = self->emitter;
     fa = emitter_host_fa_ra(self, &ra);
     if (fa < pico8_fixed_from_int(10) && !pico8_gd(self)) {
         return;
diff --git a/src/pico8_particle.h b/src/pico8_particle.h
index 11c0c01..c2aa581 100644
--- a/src/pico8_particle.h
+++ b/src/pico8_particle.h
@@ -10,7 +10,7 @@
 
 /* Throughout this module a NULL instance, gfx or mem argument is a safe
  * no-op and a non-NULL `self` must be a struct (asserted). `f_` and `ei`
- * stop on a class whose `emitter_of` is NULL; `ez` adds nothing there. */
+ * stop on an entity whose `emitter` link is NULL; `ez` adds nothing there. */
 
 /* An element of `qy`, `{p,v,a,l}` in the cart: every `mm` returns one on
  * the heap, `ei` updates it in place, nothing frees it. `has_a` is 0 for
diff --git a/src/pico8_planet_display.c b/src/pico8_planet_display.c
index ed34f76..96f1851 100644
--- a/src/pico8_planet_display.c
+++ b/src/pico8_planet_display.c
@@ -465,8 +465,8 @@ void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec
             pico8_fixed_t radius_f;
             int radius;
 
-            if (self->fo->cls->probe_of != NULL) {
-                dw = self->fo->cls->probe_of(self->fo)->def->dw;
+            if (self->fo->probe != NULL) {
+                dw = self->fo->probe->def->dw;
             }
             radius_f = pico8_fixed_mul(pico8_fixed_sqrt(dw), pico8_fixed_from_int(128));
             radius = pico8_fixed_flr_int(radius_f);
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index 3c37c2e..a0c6362 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -26,10 +26,6 @@ void pico8_qh_ly_dispatch(void *self, pico8_fixed_t t) {
     pico8_na((pico8_struct_header_t *)self, t);
 }
 
-pico8_probe_t *pico8_qh_probe_of(void *self) {
-    return ((pico8_qh_t *)self)->probe;
-}
-
 static const pico8_struct_class_t g_qh_struct_class = {
     .kj = pico8_jo_kj_dispatch,
     .ev = pico8_dl_dispatch,
@@ -48,12 +44,12 @@ static const pico8_struct_class_t g_qh_struct_class = {
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
     .parent = pico8_jo_struct_class,
-    .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
 void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     pico8_jo_struct_init(&self->jo, cls, def);
+    self->jo.header.probe = &self->probe_own;
     self->probe = &self->probe_own;
     self->probe->def = &self->jo.def->probe;
     self->probe->od = self->jo.def->probe.od;
@@ -118,7 +114,6 @@ static const pico8_struct_class_t g_nl_struct_class = {
             [PICO8_STATE_LY] = pico8_nl_ly_dispatch,
         },
     .parent = pico8_qh_struct_class,
-    .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
@@ -162,7 +157,6 @@ static const pico8_struct_class_t g_os_struct_class = {
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
     .parent = pico8_qh_struct_class,
-    .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
@@ -202,10 +196,6 @@ void pico8_hj_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_hj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_hj_t *)self, p);
 }
 
-pico8_emitter_t *pico8_hj_emitter_of(void *self) {
-    return ((pico8_hj_t *)self)->emitter;
-}
-
 static const pico8_struct_class_t g_hj_struct_class = {
     .kj = pico8_hj_kj_dispatch,
     .ev = pico8_dl_dispatch,
@@ -225,13 +215,12 @@ static const pico8_struct_class_t g_hj_struct_class = {
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
     .parent = pico8_qh_struct_class,
-    .emitter_of = pico8_hj_emitter_of,
-    .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
 void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     pico8_qh_struct_init(&self->qh, cls, def);
+    self->qh.jo.header.emitter = &self->emitter_own;
     self->emitter = &self->emitter_own;
     self->emitter->def = &self->qh.jo.def->emitter;
     self->emitter->py = self->qh.jo.def->emitter.py;
@@ -312,8 +301,6 @@ static const pico8_struct_class_t g_pm_struct_class = {
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
     .parent = pico8_hj_struct_class,
-    .emitter_of = pico8_hj_emitter_of,
-    .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
@@ -445,10 +432,6 @@ static void pico8_ga_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_ga_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_ga_t *)self, p);
 }
 
-static pico8_emitter_t *ga_emitter_of(void *self) {
-    return ((pico8_ga_t *)self)->emitter;
-}
-
 static const pico8_struct_class_t g_ga_struct_class = {
     .kj = pico8_ga_kj_dispatch,
     .hd = pico8_jo_hd_dispatch,
@@ -461,7 +444,6 @@ static const pico8_struct_class_t g_ga_struct_class = {
     .mm = pico8_ga_mm,
     .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
     .parent = pico8_jo_struct_class,
-    .emitter_of = ga_emitter_of,
     /* ga's cart list of tags is empty and replaces jo's: never jo's. */
     .tags = 0,
 };
@@ -471,6 +453,7 @@ static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
     pico8_jo_struct_init(&self->jo, &g_ga_struct_class, &pico8_ga_def);
+    self->jo.header.emitter = &self->emitter_own;
     self->emitter = &self->emitter_own;
     self->emitter->def = &self->jo.def->emitter;
     self->emitter->qo = self->jo.def->emitter.qo;
diff --git a/src/pico8_planet_family.h b/src/pico8_planet_family.h
index ced61f5..a943ee6 100644
--- a/src/pico8_planet_family.h
+++ b/src/pico8_planet_family.h
@@ -33,9 +33,6 @@ typedef struct {
     pico8_probe_t probe_own;
 } pico8_qh_t;
 
-/* The `probe_of` of qh's descriptor, for its daughters' to name. */
-pico8_probe_t *pico8_qh_probe_of(void *self);
-
 /* A trading hub: a laboratory, then `qw`, copied from its archetype and
  * written by its own `ly`. Nothing frees the struct: TROU-7. */
 typedef struct {
@@ -59,9 +56,6 @@ typedef struct {
     pico8_emitter_t emitter_own;
 } pico8_hj_t;
 
-/* The `emitter_of` of hj's descriptor, for `mx`'s to name (`pm`'s too). */
-pico8_emitter_t *pico8_hj_emitter_of(void *self);
-
 /* A protostar: an ascension and nothing of its own; its defaults are its
  * archetype's. Nothing frees the struct: TROU-7. */
 typedef struct {
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index fe8bca6..348c2db 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -176,7 +176,6 @@ static const pico8_struct_class_t g_nr_struct_class = {
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
     .parent = pico8_qh_struct_class,
-    .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
@@ -230,7 +229,6 @@ static const pico8_struct_class_t g_fr_struct_class = {
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
     .parent = pico8_nr_struct_class,
-    .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
@@ -324,8 +322,6 @@ static const pico8_struct_class_t g_mx_struct_class = {
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
     .parent = pico8_hj_struct_class,
-    .emitter_of = pico8_hj_emitter_of,
-    .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index 26235bf..3ee79c7 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -150,9 +150,9 @@ void pico8_bv(pico8_struct_header_t *self, pico8_vec_t *mp, pico8_vec_t *rp) {
     if (self == NULL) {
         return;
     }
-    assert(self->cls->probe_of != NULL && "bv: a class that carries no probe is no receiver -- stop");
+    assert(self->probe != NULL && "bv: a class that carries no probe is no receiver -- stop");
 
-    probe = self->cls->probe_of(self);
+    probe = self->probe;
     mp_vec = mp != NULL ? *mp : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
     p_vec = pico8_vec_div(mp_vec, pico8_fixed_from_int(128));
     /* TROU-7 */
@@ -224,9 +224,9 @@ void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *s
     if (self == NULL) {
         return;
     }
-    assert(self->cls->probe_of != NULL && "dl: a class that carries no probe is no receiver -- stop");
+    assert(self->probe != NULL && "dl: a class that carries no probe is no receiver -- stop");
 
-    pico8_dl_show(gfx, mem, self->me, self->cls->probe_of(self));
+    pico8_dl_show(gfx, mem, self->me, self->probe);
 }
 
 void pico8_dl_show(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *me, const pico8_probe_t *pb) {
@@ -292,9 +292,9 @@ int pico8_d_(pico8_struct_header_t *self) {
     if (self == NULL) {
         return 0;
     }
-    assert(self->cls->probe_of != NULL && "d_: a class that carries no probe is no receiver -- stop");
+    assert(self->probe != NULL && "d_: a class that carries no probe is no receiver -- stop");
 
-    probe = self->cls->probe_of(self);
+    probe = self->probe;
     has_et = probe->et != NULL;
 
     fm = pico8_fm_hy();
@@ -334,8 +334,8 @@ pico8_qy_t *pico8_g__mm(void *self_ptr) {
     pico8_qy_t *particle;
 
     if (self != NULL) {
-        assert(self->cls->emitter_of != NULL && "g_.mm: a class that carries no particle emitter has no mm -- stop");
-        emitter = self->cls->emitter_of(self);
+        assert(self->emitter != NULL && "g_.mm: a class that carries no particle emitter has no mm -- stop");
+        emitter = self->emitter;
         pn = emitter->pn;
         t = self->t;
         du = emitter->du;
@@ -376,8 +376,6 @@ static const pico8_struct_class_t g_g__struct_class = {
     .hq = pico8_bv_dispatch,
     .ey = pico8_d__dispatch,
     .mm = pico8_g__mm,
-    .emitter_of = pico8_g__emitter_of,
-    .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
 };
 
@@ -389,6 +387,8 @@ void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, con
     self->header.p = &self->p;
     self->header.fx = &self->def->fx;
     self->header.p_ = &self->p_;
+    self->header.emitter = &self->emitter_own;
+    self->header.probe = &self->probe_own;
     self->emitter = &self->emitter_own;
     self->emitter->def = &self->def->emitter;
     self->emitter->qy = NULL;
@@ -480,14 +480,6 @@ pico8_g__t *pico8_g__of(pico8_struct_header_t *e) {
     return cls != NULL ? (pico8_g__t *)e : NULL;
 }
 
-pico8_emitter_t *pico8_g__emitter_of(void *self) {
-    return ((pico8_g__t *)self)->emitter;
-}
-
-pico8_probe_t *pico8_g__probe_of(void *self) {
-    return ((pico8_g__t *)self)->probe;
-}
-
 pico8_g__t *pico8_g__new(const pico8_g__keys_t *keys) {
     return pico8_g__struct_new(keys);
 }
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index 90c6526..c7eb1be 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -49,10 +49,6 @@ typedef struct {
  * otherwise, and for a NULL `e`. */
 pico8_g__t *pico8_g__of(pico8_struct_header_t *e);
 
-/* The `emitter_of` and `probe_of` of g_'s descriptor, for `fv`'s to name. */
-pico8_emitter_t *pico8_g__emitter_of(void *self);
-pico8_probe_t *pico8_g__probe_of(void *self);
-
 /* TROU-8 */
 /* Puts the provisional `fm` back in the slot, its `jk`/`nz` nil. Shuts the
  * module down first, so it is idempotent. */
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index 83b2578..94adb7b 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -338,16 +338,11 @@ static int jc_is_endpoint_class(const pico8_struct_class_t *cls) {
     return cls->states[PICO8_STATE_LY] != NULL || cls == pico8_jo_struct_class();
 }
 
-static pico8_probe_t *jc_probe_of(void *self) {
-    return ((pico8_jc_t *)self)->probe;
-}
-
 static const pico8_struct_class_t g_jc_struct_class = {
     .kj = pico8_jc_kj_dispatch,
     .ev = pico8_jc_ev_dispatch,
     .hq = pico8_jc_hq_dispatch,
     .ey = pico8_jc_ey_dispatch,
-    .probe_of = jc_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JC),
 };
 
@@ -358,6 +353,7 @@ static pico8_jc_t *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
     self->def = &pico8_jc_def;
     self->fa = self->def->fa;
     self->header.fa = &self->fa;
+    self->header.probe = &self->probe_own;
     self->probe = &self->probe_own;
     self->probe->def = &self->def->probe;
     self->nh = self->def->nh;
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 955039b..03af969 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -74,13 +74,16 @@ static inline int pico8_tag_named(const char *name) {
 }
 
 typedef struct pico8_struct_class pico8_struct_class_t;
+struct pico8_emitter;
+struct pico8_probe;
 
 /* First member of every entity struct, and of the parent struct a child
  * embeds first. `me` and `t` are ke's (`me` NULL is nil). A component link
  * points at the field's storage, the archetype's for a field the class
  * never writes; NULL: the class has no such field, or it is nil. `ma` is
- * written through, by `pico8_ma`, `p_`, by `d_`, and `m_`, by `ui`'s
- * hover. `*fx` 0 is nil. */
+ * written through, by `pico8_ma`, `p_`, by `d_`, `m_`, by `ui`'s hover,
+ * and the particle emitter and the probe, which the instance stores in
+ * itself, by their modules. `*fx` 0 is nil. */
 typedef struct {
     const pico8_struct_class_t *cls;
     const char *me;
@@ -96,6 +99,8 @@ typedef struct {
     const pico8_fixed_t *fx;
     int *p_;
     int *m_;
+    struct pico8_emitter *emitter;
+    struct pico8_probe *probe;
 } pico8_struct_header_t;
 
 /* Every constructor calls it first: its `malloc` leaves the header
@@ -118,8 +123,6 @@ typedef void (*pico8_state_fn)(void *self, pico8_fixed_t t);
 
 struct pico8_qy;
 struct pico8_jo_trade_proposal;
-struct pico8_emitter;
-struct pico8_probe;
 struct pico8_str_seq;
 struct pico8_kind_rec;
 
@@ -128,11 +131,7 @@ struct pico8_kind_rec;
  * them; `ldown`, `rdown` and `hq` may keep theirs. NULL `ra`: nil. `has_hb`
  * is NULL when a non-NULL `hb` is enough for `e.hb` to exist; otherwise the
  * instance decides. `mm` returns a fresh particle the caller owns. `parent` returns the class
- * whose struct this one embeds first; the member is NULL for a root.
- * `emitter_of` and `probe_of` return the particle emitter and the probe the
- * instance stores in itself; the member is NULL for a class that carries
- * none. A class that sets one descends from ke; each is only ever
- * called as `e->cls->emitter_of(e)`, `e->cls->probe_of(e)`. */
+ * whose struct this one embeds first; the member is NULL for a root. */
 struct pico8_struct_class {
     void (*kj)(void *self, const pico8_vec_t *ra);
     void (*ev)(void *self, const pico8_vec_t *ra);
@@ -153,8 +152,6 @@ struct pico8_struct_class {
     /* `PICO8_TAG(t)` set: lt() registers every instance under tag t. */
     unsigned tags;
     const pico8_struct_class_t *(*parent)(void);
-    struct pico8_emitter *(*emitter_of)(void *self);
-    struct pico8_probe *(*probe_of)(void *self);
 };
 
 /* The construction keys of the jo tree (the cart's `mw(ob)` in `kv:jr`).
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index d283586..8e1ed89 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -193,10 +193,6 @@ static void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra) {
  * so only that second definition is ever reachable. Putting
  * pico8_bv_dispatch in the HQ entry instead would look faithful and would
  * drop the nearest-anchor search. */
-static pico8_probe_t *mz_probe_of(void *self) {
-    return ((pico8_mz_t *)self)->probe;
-}
-
 static const pico8_struct_class_t g_mz_struct_class = {
     .kj = pico8_mz_kj_dispatch,
     .ev = pico8_dl_dispatch,
@@ -207,7 +203,6 @@ static const pico8_struct_class_t g_mz_struct_class = {
             [PICO8_STATE_KZ] = pico8_mz_kz_dispatch,
             [PICO8_STATE_PR] = pico8_mz_pr_dispatch,
         },
-    .probe_of = mz_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_MZ),
 };
 
@@ -219,6 +214,7 @@ static pico8_mz_t *pico8_mz_struct_new(void) {
     self->header.fa = &self->def->fa;
     self->header.ra = &self->ra;
     self->header.p = &self->p;
+    self->header.probe = &self->probe_own;
     self->probe = &self->probe_own;
     self->probe->def = &self->def->probe;
     self->oi = self->def->oi;
diff --git a/tests/test_pico8_struct_header.c b/tests/test_pico8_struct_header.c
index 48f372f..77881a2 100644
--- a/tests/test_pico8_struct_header.c
+++ b/tests/test_pico8_struct_header.c
@@ -23,8 +23,11 @@
 #include "../src/pico8_layout.h"
 #include "../src/pico8_mem.h"
 #include "../src/pico8_mouse_reader.h"
+#include "../src/pico8_msgbox.h"
 #include "../src/pico8_planet.h"
+#include "../src/pico8_planet_family.h"
 #include "../src/pico8_planet_family_trade.h"
+#include "../src/pico8_probe.h"
 #include "../src/pico8_research.h"
 #include "../src/pico8_research_icon.h"
 #include "../src/pico8_scroll.h"
@@ -32,6 +35,7 @@
 #include "../src/pico8_trade.h"
 #include "../src/pico8_ui.h"
 #include "../src/pico8_ui_button.h"
+#include "../src/pico8_wormhole.h"
 
 static pico8_mem_t mem;
 static pico8_gfx_t gfx;
@@ -82,6 +86,8 @@ static void assert_links(const pico8_struct_header_t *h, pico8_struct_header_t w
     assert(h->fx == want.fx);
     assert(h->p_ == want.p_);
     assert(h->m_ == want.m_);
+    assert(h->emitter == want.emitter);
+    assert(h->probe == want.probe);
 }
 
 /* Non-carriers, one per constructor that does not go through ke; the mouse
@@ -112,6 +118,8 @@ static void test_carriers_link_their_storage(void) {
     pico8_fq_t *fq = pico8_fq_new(NULL);
     pico8_ui_t *ui = pico8_ui_new();
     pico8_qb_t *qb = pico8_qb_new();
+    pico8_mh_t *mh = pico8_mh_new(NULL);
+    pico8_iv_t *iv = pico8_iv_new(NULL);
     pico8_fv_t *fv = pico8_fv_new();
     pico8_jo_t *jo = pico8_jo_new(NULL);
     pico8_fr_t *fr = pico8_fr_new(NULL);
@@ -129,6 +137,8 @@ static void test_carriers_link_their_storage(void) {
     assert_links(&fq->header, (pico8_struct_header_t){.fa = &fq->def->fa, .p_ = &fq->p_, .m_ = &fq->m_});
     assert_links(&ui->header, (pico8_struct_header_t){.fa = &ui->def->fa, .ql = &ui->def->ql});
     assert_links(&qb->header, (pico8_struct_header_t){.ra = &qb->ra, .p = &qb->p, .ma = &qb->ma});
+    assert_links(&mh->header, (pico8_struct_header_t){.fa = &mh->def->fa, .p_ = &mh->p_});
+    assert_links(&iv->header, (pico8_struct_header_t){.fa = &iv->def->fa, .ra = &iv->ra, .p_ = &iv->p_});
     assert_links(&fv->g_.header, (pico8_struct_header_t){.fa = &fv->g_.def->fa,
                                                          .ra = &fv->g_.ra,
                                                          .p = &fv->g_.p,
@@ -137,7 +147,11 @@ static void test_carriers_link_their_storage(void) {
                                                          .fg = &fv->g_.def->fg,
                                                          .fx = &fv->g_.def->fx,
                                                          .p_ = &fv->g_.p_,
-                                                         .m_ = &fv->m_});
+                                                         .m_ = &fv->m_,
+                                                         .emitter = &fv->g_.emitter_own,
+                                                         .probe = &fv->g_.probe_own});
+    assert(*fv->g_.header.fx == pico8_fixed_from_int(1) &&
+           "a probe's fx is its archetype's 1.0, Lua's true: 0 would read as nil");
     assert_links(&jo->header, (pico8_struct_header_t){.fa = &jo->def->fa,
                                                       .ra = (const pico8_vec_t *const *)&jo->ra,
                                                       .p = (const pico8_vec_t *const *)&jo->p,
@@ -154,7 +168,72 @@ static void test_carriers_link_their_storage(void) {
                                                          .ma = &fr_jo->ma,
                                                          .fx = &fr_jo->fx,
                                                          .p_ = &fr_jo->p_,
-                                                         .m_ = &fr_jo->m_});
+                                                         .m_ = &fr_jo->m_,
+                                                         .probe = &fr->nr.qh.probe_own});
+}
+
+/* A planet's links, but for its emitter and probe. */
+static pico8_struct_header_t planet_links(pico8_jo_t *jo) {
+    return (pico8_struct_header_t){.fa = &jo->def->fa,
+                                   .ra = (const pico8_vec_t *const *)&jo->ra,
+                                   .p = (const pico8_vec_t *const *)&jo->p,
+                                   .kq = &jo->def->kq,
+                                   .ma = &jo->ma,
+                                   .fx = &jo->fx,
+                                   .p_ = &jo->p_,
+                                   .m_ = &jo->m_};
+}
+
+static void assert_qh_links(pico8_qh_t *qh, pico8_emitter_t *emitter) {
+    pico8_struct_header_t want = planet_links(&qh->jo);
+
+    want.emitter = emitter;
+    want.probe = &qh->probe_own;
+    assert_links(&qh->jo.header, want);
+    assert(qh->probe == qh->jo.header.probe);
+}
+
+/* Emitter and probe carriers: each link is the component the instance
+ * stores in itself, the one its own member points at. */
+static void test_component_carriers_link_their_own(void) {
+    pico8_g__t *g_ = pico8_g__new(NULL);
+    pico8_nl_t *nl = pico8_nl_new(NULL);
+    pico8_os_t *os = pico8_os_new(NULL);
+    pico8_hj_t *hj = pico8_hj_new(NULL);
+    pico8_pm_t *pm = pico8_pm_new(NULL);
+    pico8_mx_t *mx = pico8_mx_new(NULL);
+    pico8_nr_t *nr = pico8_nr_new(NULL);
+    pico8_ga_t *ga = pico8_ga_new(NULL);
+    pico8_mz_t *mz = pico8_mz_new();
+    pico8_struct_header_t want = planet_links(&ga->jo);
+
+    assert_links(&g_->header, (pico8_struct_header_t){.fa = &g_->def->fa,
+                                                      .ra = &g_->ra,
+                                                      .p = &g_->p,
+                                                      .fx = &g_->def->fx,
+                                                      .p_ = &g_->p_,
+                                                      .emitter = &g_->emitter_own,
+                                                      .probe = &g_->probe_own});
+    assert(g_->emitter == g_->header.emitter && g_->probe == g_->header.probe);
+    assert(*g_->header.fx == pico8_fixed_from_int(1) &&
+           "a probe's fx is its archetype's 1.0, Lua's true: 0 would read as nil");
+    assert_qh_links(pico8_qh_new(NULL), NULL);
+    assert_qh_links(&nl->qh, NULL);
+    assert_qh_links(&os->qh, NULL);
+    assert_qh_links(&hj->qh, &hj->emitter_own);
+    assert(hj->emitter == hj->qh.jo.header.emitter);
+    assert_qh_links(&pm->hj.qh, &pm->hj.emitter_own);
+    assert_qh_links(&mx->hj.qh, &mx->hj.emitter_own);
+    assert_qh_links(&nr->qh, NULL);
+    want.emitter = &ga->emitter_own;
+    assert_links(&ga->jo.header, want);
+    assert(ga->emitter == ga->jo.header.emitter);
+    assert_links(&mz->header, (pico8_struct_header_t){.fa = &mz->def->fa,
+                                                      .ra = &mz->ra,
+                                                      .p = &mz->p,
+                                                      .p_ = &mz->p_,
+                                                      .probe = &mz->probe_own});
+    assert(mz->probe == mz->header.probe);
 }
 
 static pico8_jc_t *jc_dragged_over_nothing(pico8_struct_header_t *qu) {
@@ -180,9 +259,12 @@ static void test_mouse_point_has_null_links(void) {
     assert(point != NULL && point->header.cls == pico8_jc_mouse_point_struct_class());
     assert_links(&point->header, (pico8_struct_header_t){.ra = &point->ra, .p = &point->p});
     assert(!jc->has_mt && jc->fa == pico8_fixed_from_int(4));
-    assert_links(&jc->header, (pico8_struct_header_t){.fa = &jc->fa, .p_ = &jc->p_});
+    assert_links(&jc->header, (pico8_struct_header_t){.fa = &jc->fa, .p_ = &jc->p_, .probe = &jc->probe_own});
+    assert(jc->probe == jc->header.probe);
     assert(from_fr->has_mt && from_fr->mt == pico8_fr_def.mt && from_fr->fa == pico8_fixed_from_int(2));
-    assert_links(&from_fr->header, (pico8_struct_header_t){.fa = &from_fr->fa, .mt = &from_fr->mt, .p_ = &from_fr->p_});
+    assert_links(&from_fr->header,
+                 (pico8_struct_header_t){
+                     .fa = &from_fr->fa, .mt = &from_fr->mt, .p_ = &from_fr->p_, .probe = &from_fr->probe_own});
 }
 
 int main(int argc, char **argv) {
@@ -202,6 +284,7 @@ int main(int argc, char **argv) {
     assert_malloc_fills();
     test_non_carriers_have_null_links();
     test_carriers_link_their_storage();
+    test_component_carriers_link_their_own();
     test_mouse_point_has_null_links();
 
     printf("test_pico8_struct_header: all tests passed\n");
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index 663ebb2..7aa6663 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -839,20 +839,20 @@ static uint32_t fold_ks_back(uint32_t h, void *e) {
 static void assert_own_emitter(void *self, void *other, size_t size) {
     pico8_struct_header_t *e = self;
     pico8_struct_header_t *o = other;
-    const char *at = (const char *)e->cls->emitter_of(e);
+    const char *at = (const char *)e->emitter;
 
     assert(e->cls == o->cls);
-    assert((const void *)at != (const void *)o->cls->emitter_of(o));
+    assert((const void *)at != (const void *)o->emitter);
     assert(at >= (const char *)self && at < (const char *)self + size);
 }
 
 static void assert_own_probe(void *self, void *other, size_t size) {
     pico8_struct_header_t *e = self;
     pico8_struct_header_t *o = other;
-    const char *at = (const char *)e->cls->probe_of(e);
+    const char *at = (const char *)e->probe;
 
     assert(e->cls == o->cls);
-    assert((const void *)at != (const void *)o->cls->probe_of(o));
+    assert((const void *)at != (const void *)o->probe);
     assert(at >= (const char *)self && at < (const char *)self + size);
 }
 
@@ -892,8 +892,8 @@ static void test_components_carried(void) {
     for (i = 0; i < 24; i++) {
         const pico8_struct_header_t *header = e[i];
 
-        assert((header->cls->emitter_of != NULL) == emitter[i]);
-        assert((header->cls->probe_of != NULL) == probe[i]);
+        assert((header->emitter != NULL) == emitter[i]);
+        assert((header->probe != NULL) == probe[i]);
     }
 
     assert_own_emitter(e[8], pico8_hj_new(NULL), sizeof(pico8_hj_t));
```

#### Diff de 2.15b (`git diff a b`)

```diff
diff --git a/src/pico8_help.c b/src/pico8_help.c
index f4d0ad2..5746283 100644
--- a/src/pico8_help.c
+++ b/src/pico8_help.c
@@ -39,7 +39,7 @@ static int pico8_fv_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_v
 #define PICO8_BO_ROW_CAP 4
 
 /* `pz(self, bo)`, each row's `my` the `eu` of this game. */
-void pico8_fv_pq(pico8_fv_t *self) {
+void pico8_fv_pq(pico8_g__t *self) {
     pico8_js_keys_t bs[PICO8_BO_COUNT];
     pico8_lq_keys_t lq[PICO8_BO_COUNT];
     pico8_lq_cell_t items[PICO8_BO_COUNT][PICO8_BO_ROW_CAP];
@@ -64,20 +64,20 @@ void pico8_fv_pq(pico8_fv_t *self) {
         bs[i].my = pico8_eu_field_value();
     }
     frame.has_eo = 1;
-    frame.eo = self->g_.def->eo;
-    frame.hi = &self->g_.def->hi;
+    frame.eo = self->def->eo;
+    frame.hi = &self->def->hi;
     frame.ff = 1;
     frame.has_ca = 1;
-    frame.ca = self->g_.def->ca;
-    frame.gq = &self->g_.def->gq;
-    frame.ra = self->g_.ra;
+    frame.ca = self->def->ca;
+    frame.gq = &self->def->gq;
+    frame.ra = self->ra;
     pico8_pz(&frame, bs, PICO8_BO_COUNT);
 }
 
 /* `fv.rb` holds four articles in the cart. */
 enum { PICO8_FV_RB_CAP = 4 };
 
-void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec_t p) {
+void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_g__t *self, pico8_vec_t p) {
     (void)p;
 
     pico8_ei(gfx, mem, (pico8_struct_header_t *)self);
@@ -87,9 +87,9 @@ void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec
         int n = 0;
         int i;
 
-        for (i = 0; self != NULL && self->g_.def->rb != NULL && i < self->g_.def->rb->count; i++) {
+        for (i = 0; self != NULL && self->def->rb != NULL && i < self->def->rb->count; i++) {
             assert(n < PICO8_FV_RB_CAP && "fv.rb has four articles in the cart");
-            items[n++] = self->g_.def->rb->items[i].row;
+            items[n++] = self->def->rb->items[i].row;
         }
         pico8_bl_default(gfx, mem, items, n);
     }
@@ -97,7 +97,7 @@ void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec
 
 void pico8_fv_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-    pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_fv_t *)self, p);
+    pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_g__t *)self, p);
 }
 
 static const pico8_struct_class_t g_fv_struct_class = {
@@ -107,32 +107,32 @@ static const pico8_struct_class_t g_fv_struct_class = {
     .hq = pico8_bv_dispatch,
     .ey = pico8_d__dispatch,
     .mm = pico8_g__mm,
-    .parent = pico8_g__struct_class,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_G_),
 };
 
-static pico8_fv_t *pico8_fv_struct_new(void) {
-    pico8_fv_t *self;
+static pico8_g__t *pico8_fv_struct_new(void) {
+    pico8_g__t *self;
 
     assert(g_fv_up && "pico8_help_init() must run first");
     self = malloc(sizeof *self);
-    pico8_g__struct_init(&self->g_, &g_fv_struct_class, &pico8_fv_def);
-    self->g_.header.kq = &self->g_.def->kq;
-    self->g_.header.ql = &self->g_.def->ql;
-    self->g_.header.fg = &self->g_.def->fg;
-    self->g_.header.m_ = &self->m_;
+    pico8_g__struct_init(self, &g_fv_struct_class, &pico8_fv_def);
+    self->header.kq = &self->def->kq;
+    self->header.ql = &self->def->ql;
+    self->header.fg = &self->def->fg;
+    self->header.m_ = &self->m_;
     self->m_ = 0;
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("fv");
 #endif
     pico8_fv_pq(self);
     pico8_f_((pico8_struct_header_t *)self);
-    pico8_ke_struct_pq(&self->g_.header);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
 /* This module, unlike most, does not call pico8_pi_init() itself, and `fv`
- * extends `g_`, not `ke`: both preconditions are asserted, not repaired. */
+ * is built by `g_`'s init: both preconditions are asserted, not repaired. */
 void pico8_help_init(void) {
     assert(pico8_pi_dispatch_lookup("s") != NULL && "pico8_help_init() runs after pico8_pi_init()");
     assert(pico8_g__ready() && "pico8_help_init() runs after pico8_probe_init()");
@@ -157,6 +157,6 @@ const pico8_struct_class_t *pico8_fv_struct_class(void) {
     return &g_fv_struct_class;
 }
 
-pico8_fv_t *pico8_fv_new(void) {
+pico8_g__t *pico8_fv_new(void) {
     return pico8_fv_struct_new();
 }
diff --git a/src/pico8_help.h b/src/pico8_help.h
index a948e8f..305905e 100644
--- a/src/pico8_help.h
+++ b/src/pico8_help.h
@@ -13,13 +13,6 @@
 /* Throughout this module a NULL instance, gfx or mem argument is a safe
  * no-op, and an absent or wrong-kind field reads as absent. */
 
-/* TROU-7 */
-/* `g_.ra` points at `pico8_fv_def`'s, shared by every help. */
-typedef struct {
-    pico8_g__t g_;
-    int m_;
-} pico8_fv_t;
-
 /* Builds `bo`; `pico8_help_shutdown()` frees it and is called first by an
  * init, so both are idempotent. Both also reset `eu` to the placeholder, so
  * the real one must be installed after every init. */
@@ -32,9 +25,9 @@ const pico8_struct_class_t *pico8_fv_struct_class(void);
 
 /* TROU-8 */
 /* The help's class carries `g_`'s tags, so `lt` registers it into `cg.g_` and
- * `cg.prober` too; nothing ever frees it. Asserts when called before
- * `pico8_help_init()`. */
-pico8_fv_t *pico8_fv_new(void);
+ * `cg.prober` too; nothing ever frees it. Its `ra` points at `pico8_fv_def`'s,
+ * shared by every help. Asserts when called before `pico8_help_init()`. */
+pico8_g__t *pico8_fv_new(void);
 
 /* `pico8_eu_field_value()` never returns NULL: an inert placeholder until
  * `pico8_help_set_eu()` installs the real `eu`, which
@@ -45,9 +38,9 @@ void pico8_help_set_eu(pico8_js_fn fn);
 /* Stamps the `eu` value current at call time onto every row of `bo`: rows
  * stamped before a `pico8_help_set_eu()` keep what they were given. A NULL
  * `self` still stamps them and only skips the `pz()` call. */
-void pico8_fv_pq(pico8_fv_t *self);
+void pico8_fv_pq(pico8_g__t *self);
 
-void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec_t p);
+void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_g__t *self, pico8_vec_t p);
 
 void pico8_fv_kj_dispatch(void *self, const pico8_vec_t *ra);
 
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 2038423..f3fbb9a 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -61,7 +61,7 @@ pico8_down_t pico8_jo_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *r
 }
 
 int pico8_jo_is_family(const pico8_struct_header_t *e) {
-    return e->cls->states[PICO8_STATE_LL] == pico8_jo_ll_dispatch;
+    return (e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_JO)) != 0;
 }
 
 void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw) {
@@ -87,6 +87,7 @@ static const pico8_struct_class_t g_jo_struct_class = {
     .cn = pico8_jo_cn_dispatch,
     .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),
 };
 
 void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index a0c6362..cfe7f24 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -45,6 +45,7 @@ static const pico8_struct_class_t g_qh_struct_class = {
         },
     .parent = pico8_jo_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
 void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
@@ -115,6 +116,7 @@ static const pico8_struct_class_t g_nl_struct_class = {
         },
     .parent = pico8_qh_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH) | PICO8_LINEAGE(PICO8_LINEAGE_NL),
 };
 
 static pico8_nl_t *pico8_nl_struct_new(const pico8_jo_keys_t *keys) {
@@ -158,6 +160,7 @@ static const pico8_struct_class_t g_os_struct_class = {
         },
     .parent = pico8_qh_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
 static pico8_os_t *pico8_os_struct_new(const pico8_jo_keys_t *keys) {
@@ -216,6 +219,7 @@ static const pico8_struct_class_t g_hj_struct_class = {
         },
     .parent = pico8_qh_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
 void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
@@ -302,6 +306,7 @@ static const pico8_struct_class_t g_pm_struct_class = {
         },
     .parent = pico8_hj_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
 static pico8_pm_t *pico8_pm_struct_new(const pico8_jo_keys_t *keys) {
@@ -446,6 +451,7 @@ static const pico8_struct_class_t g_ga_struct_class = {
     .parent = pico8_jo_struct_class,
     /* ga's cart list of tags is empty and replaces jo's: never jo's. */
     .tags = 0,
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),
 };
 
 static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index 348c2db..d422b63 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -177,6 +177,7 @@ static const pico8_struct_class_t g_nr_struct_class = {
         },
     .parent = pico8_qh_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
 /* Initializes an allocated pico8_nr_t under `cls` and `def`, its kind's
@@ -230,6 +231,7 @@ static const pico8_struct_class_t g_fr_struct_class = {
         },
     .parent = pico8_nr_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
 static pico8_fr_t *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
@@ -323,6 +325,7 @@ static const pico8_struct_class_t g_mx_struct_class = {
         },
     .parent = pico8_hj_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
 static pico8_mx_t *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index 3ee79c7..1634dc9 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -79,11 +79,12 @@ static int pico8_probe_ui_hw_call(pico8_vec_t rp) {
 /* `ra` and `p`, which a host of `bv` holds for itself: `g_`'s line, jo's
  * family (qh's line) or an `mz`. */
 static void probe_host_set_ra_p(pico8_struct_header_t *self, pico8_vec_t *ra, pico8_vec_t *p) {
-    pico8_g__t *g_ = pico8_g__of(self);
+    pico8_g__t *g_;
     pico8_jo_t *jo;
     pico8_mz_t *mz;
 
-    if (g_ != NULL) {
+    if (self->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_G_)) {
+        g_ = (pico8_g__t *)self;
         g_->ra = ra;
         g_->p = p;
         return;
@@ -377,6 +378,7 @@ static const pico8_struct_class_t g_g__struct_class = {
     .ey = pico8_d__dispatch,
     .mm = pico8_g__mm,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
+    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_G_),
 };
 
 void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, const pico8_g__def_t *def) {
@@ -471,15 +473,6 @@ const pico8_struct_class_t *pico8_g__struct_class(void) {
     return &g_g__struct_class;
 }
 
-pico8_g__t *pico8_g__of(pico8_struct_header_t *e) {
-    const pico8_struct_class_t *cls = e != NULL ? e->cls : NULL;
-
-    while (cls != NULL && cls != &g_g__struct_class) {
-        cls = cls->parent != NULL ? cls->parent() : NULL;
-    }
-    return cls != NULL ? (pico8_g__t *)e : NULL;
-}
-
 pico8_g__t *pico8_g__new(const pico8_g__keys_t *keys) {
     return pico8_g__struct_new(keys);
 }
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index c7eb1be..304fcd7 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -32,7 +32,8 @@ typedef struct pico8_probe {
 /* The emitter's `k_` points at
  * `pico8_g__k__seq` (`pico8_fv_k__seq` for `fv`); `ra` is the very REF `bv`
  * was passed, aliased. `emitter` and `probe` point at `emitter_own` and
- * `probe_own`: never copy the struct. */
+ * `probe_own`: never copy the struct. `m_` is `fv`'s: a `g_` has no `m_`
+ * link and leaves it indeterminate. */
 typedef struct {
     pico8_struct_header_t header;
     const pico8_g__def_t *def;
@@ -41,14 +42,11 @@ typedef struct {
     const pico8_vec_t *ra;
     const pico8_vec_t *p;
     int p_;
+    int m_;
     pico8_emitter_t emitter_own;
     pico8_probe_t probe_own;
 } pico8_g__t;
 
-/* `e` itself when its class is g_'s or has g_'s up its `parent` chain; NULL
- * otherwise, and for a NULL `e`. */
-pico8_g__t *pico8_g__of(pico8_struct_header_t *e);
-
 /* TROU-8 */
 /* Puts the provisional `fm` back in the slot, its `jk`/`nz` nil. Shuts the
  * module down first, so it is idempotent. */
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 03af969..162fa76 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -55,6 +55,16 @@ typedef enum {
 
 #define PICO8_TAG(t) (1u << (t))
 
+/* The four classes whose descendants the code asks after. */
+typedef enum {
+    PICO8_LINEAGE_JO,
+    PICO8_LINEAGE_QH,
+    PICO8_LINEAGE_NL,
+    PICO8_LINEAGE_G_,
+} pico8_lineage_t;
+
+#define PICO8_LINEAGE(l) (1u << (l))
+
 /* The tag named `name`, or -1 for any other name. */
 static inline int pico8_tag_named(const char *name) {
     static const char *const names[PICO8_TAG_COUNT] = {
@@ -151,6 +161,8 @@ struct pico8_struct_class {
     pico8_state_fn states[PICO8_STATE_COUNT];
     /* `PICO8_TAG(t)` set: lt() registers every instance under tag t. */
     unsigned tags;
+    /* `PICO8_LINEAGE(l)` set: the class is l's or descends from it. */
+    unsigned lineage;
     const pico8_struct_class_t *(*parent)(void);
 };
 
diff --git a/tests/test_pico8_fv.c b/tests/test_pico8_fv.c
index 0cb4ff8..c457b55 100644
--- a/tests/test_pico8_fv.c
+++ b/tests/test_pico8_fv.c
@@ -75,7 +75,7 @@ static int ents_listed(const pico8_ents_t *list, const void *e) {
 }
 
 /* 0 absent or false, 2 true. */
-static uint32_t m__state(const pico8_fv_t *fv) {
+static uint32_t m__state(const pico8_g__t *fv) {
     return fv->m_ ? 2 : 0;
 }
 
@@ -248,11 +248,12 @@ static void start_title(void) {
 
 /* The help, whatever its form: `g_`'s tags and `fa`=10 (a probe has 2). */
 static int is_help(pico8_struct_header_t *e) {
-    const pico8_g__t *g_ = e != NULL ? pico8_g__of(e) : NULL;
+    const pico8_g__t *g_ =
+        e != NULL && (e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_G_)) ? (const pico8_g__t *)e : NULL;
     return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->def->fa == pico8_fixed_from_int(10);
 }
 
-static pico8_fv_t *the_help(void) {
+static pico8_g__t *the_help(void) {
     const pico8_ents_t *hv = pico8_hv();
     pico8_struct_header_t *found = NULL;
     int i;
@@ -264,7 +265,7 @@ static pico8_fv_t *the_help(void) {
         }
     }
     assert(found != NULL && found->cls == pico8_fv_struct_class());
-    return (pico8_fv_t *)found;
+    return (pico8_g__t *)found;
 }
 
 static int in_layer(int layer, const void *e) {
@@ -290,7 +291,7 @@ static const pico8_js_t *as_js(const void *e) {
 static void test_built(void) {
     uint32_t h = 2166136261u;
     const pico8_ents_t *hv;
-    pico8_fv_t *fv;
+    pico8_g__t *fv;
     const pico8_g__t *g_;
     const pico8_emitter_t *em;
     const pico8_probe_t *pr;
@@ -300,7 +301,7 @@ static void test_built(void) {
 
     start_title();
     fv = the_help();
-    g_ = &fv->g_;
+    g_ = fv;
     em = g_->emitter;
     pr = g_->probe;
     hv = pico8_hv();
@@ -400,7 +401,7 @@ static void test_built(void) {
  * out of `kq`, `rp` retained since `fa` >= 10. */
 static void test_life(void) {
     uint32_t h = 2166136261u;
-    pico8_fv_t *fv;
+    pico8_g__t *fv;
     int i;
 
     start_title();
@@ -408,21 +409,21 @@ static void test_life(void) {
     for (i = 0; i < 3; i++) {
         pico8_dc("kj");
         h = fnv(h, screen_hash());
-        h = fnv(h, (uint32_t)size_of(fv->g_.emitter->qy));
+        h = fnv(h, (uint32_t)size_of(fv->emitter->qy));
     }
     pico8_dc("ev");
     h = fnv(h, screen_hash());
     for (i = 0; i < 3; i++) {
         pico8_dg();
-        h = fold_fixed_member(h, fv->g_.header.t);
-        h = fold_string_member(h, fv->g_.header.me);
+        h = fold_fixed_member(h, fv->header.t);
+        h = fold_string_member(h, fv->header.me);
         assert(ents_listed(pico8_hv(), fv));
     }
     assert(cls_of(fv)->hd(fv, vec_ref(64, 10), vec_ref(64, 112)) == 1);
     assert(cls_of(fv)->hd(fv, vec_ref(64, 112), vec_ref(64, 10)) == 0);
     assert(cls_of(fv)->hd(fv, vec_ref(24, 106), vec_ref(24, 106)) == 1);
     assert(cls_of(fv)->hd(fv, vec_ref(23, 106), vec_ref(23, 106)) == 0);
-    h = fnv(h, (uint32_t)size_of(fv->g_.emitter->qy));
+    h = fnv(h, (uint32_t)size_of(fv->emitter->qy));
     check("life", h, LIFE_HASH);
 }
 
@@ -443,7 +444,7 @@ static void mouse_at(int x, int y) {
  * help in layer `fa` of `ic`, the cursor sprite `ql`; then out, `m_` false. */
 static void test_hover(void) {
     uint32_t h = 2166136261u;
-    pico8_fv_t *fv;
+    pico8_g__t *fv;
     uint32_t drawn;
 
     start_title();
@@ -483,24 +484,24 @@ static void test_particle_takes_the_help_palette(void) {
     static pico8_qy_t particle;
     pico8_ptrs_t *qy;
     pico8_vec_t ra;
-    pico8_fv_t *fv;
+    pico8_g__t *fv;
     int at;
 
     start_title();
     fv = the_help();
-    assert(fv->g_.ra != NULL);
-    ra = *fv->g_.ra;
+    assert(fv->ra != NULL);
+    ra = *fv->ra;
     particle.p = pico8_vec_v(0, 0);
     particle.v = pico8_vec_v(0, 0);
     particle.has_a = 0;
     particle.l = PICO8_FIXED_LIT(0x00068000, 6.5);
-    qy = fv->g_.emitter->qy;
+    qy = fv->emitter->qy;
     qy->count = 0;
     pico8_ptrs_add(qy, &particle);
     pico8_camera_reset(&gfx);
     at = PICO8_ADDR_SCREEN + pico8_fixed_flr_int(ra.y) * PICO8_SCREEN_WIDTH + pico8_fixed_flr_int(ra.x);
     pico8_poke(&mem, at, 3);
-    pico8_ei(&gfx, &mem, &fv->g_.header);
+    pico8_ei(&gfx, &mem, &fv->header);
     assert(pico8_peek(&mem, at) == pico8_fv_k__seq.items[5] && pico8_fv_k__seq.items[5] == 9);
 }
 
diff --git a/tests/test_pico8_g_.c b/tests/test_pico8_g_.c
index 09211b4..10c1436 100644
--- a/tests/test_pico8_g_.c
+++ b/tests/test_pico8_g_.c
@@ -303,7 +303,8 @@ static pico8_bg_t *the_bg(void) {
 
 /* A probe, whatever its form: `g_`'s tags and `fa`=2 (an `fv` has 10). */
 static int is_probe(pico8_struct_header_t *e) {
-    const pico8_g__t *g_ = e != NULL ? pico8_g__of(e) : NULL;
+    const pico8_g__t *g_ =
+        e != NULL && (e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_G_)) ? (const pico8_g__t *)e : NULL;
     return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->def->fa == pico8_fixed_from_int(2);
 }
 
@@ -661,11 +662,12 @@ static void test_ey_void(void) {
 
 /* The help: `g_`'s tags and `fa`=10. */
 static int is_help(pico8_struct_header_t *e) {
-    const pico8_g__t *g_ = e != NULL ? pico8_g__of(e) : NULL;
+    const pico8_g__t *g_ =
+        e != NULL && (e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_G_)) ? (const pico8_g__t *)e : NULL;
     return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->def->fa == pico8_fixed_from_int(10);
 }
 
-/* The title screen's `fv`, whose class's parent is `g_`'s class: its
+/* The title screen's `fv`, of `g_`'s lineage: its
  * tags, `fx`, `dq` inherited, `ev`, `mm` the functions of `g_`. */
 static void test_fv(void) {
     uint32_t h = 2166136261u;
@@ -686,7 +688,7 @@ static void test_fv(void) {
         }
     }
     assert(found != NULL && !is_probe(found));
-    fv = pico8_g__of(found);
+    fv = (const pico8_g__t *)found;
     assert(fv->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(fv->def->fx == pico8_fixed_from_int(1));
     assert(fv->probe->def->dq != NULL && strcmp(fv->probe->def->dq, "ly") == 0);
diff --git a/tests/test_pico8_qh.c b/tests/test_pico8_qh.c
index da53fe1..4e8915a 100644
--- a/tests/test_pico8_qh.c
+++ b/tests/test_pico8_qh.c
@@ -16,6 +16,7 @@
 #include <unistd.h>
 
 #include "../src/pico8_assets.h"
+#include "../src/pico8_bg.h"
 #include "../src/pico8_bootstrap.h"
 #include "../src/pico8_cart.h"
 #include "../src/pico8_dispatch_context.h"
@@ -33,9 +34,11 @@
 #include "../src/pico8_research.h"
 #include "../src/pico8_research_icon.h"
 #include "../src/pico8_scroll.h"
+#include "../src/pico8_slipway.h"
 #include "../src/pico8_struct.h"
 #include "../src/pico8_ui_button.h"
 #include "../src/pico8_vec.h"
+#include "../src/pico8_wormhole.h"
 
 
 /* The tag class instances carried, which the frozen hashes fold: any
@@ -702,9 +705,17 @@ static void test_qh_is_a_struct(void) {
     }
 }
 
+static unsigned lineage_of(const pico8_struct_header_t *e) {
+    return e->cls->lineage;
+}
+
 /* qh's lineage answers pico8_qh_of() with itself, through one to three
- * `parent` links; jo, ga and NULL answer NULL. Only nl answers pico8_nl_of(). */
+ * `parent` links; jo, ga and NULL answer NULL. Only nl answers pico8_nl_of().
+ * Each class's `lineage` says the same; g_ and fv are of g_'s alone, the
+ * other classes of none. */
 static void test_qh_of_follows_parent(void) {
+    const unsigned jo = PICO8_LINEAGE(PICO8_LINEAGE_JO);
+    const unsigned qh = jo | PICO8_LINEAGE(PICO8_LINEAGE_QH);
     const pico8_entity_kind_t *const lineage[] = {pico8_qh_kind(), pico8_nl_kind(), pico8_os_kind(), pico8_hj_kind(),
                                                   pico8_pm_kind(), pico8_mx_kind(), pico8_nr_kind(), pico8_fr_kind()};
     pico8_jo_keys_t keys = {"s99", NULL, NULL};
@@ -716,12 +727,17 @@ static void test_qh_of_follows_parent(void) {
         e = lineage[i]->new_entity(&keys);
         assert(pico8_qh_of(e) == (const void *)e);
         assert(pico8_nl_of(e) == (lineage[i] == pico8_nl_kind() ? (const void *)e : NULL));
+        assert(lineage_of(e) == (lineage[i] == pico8_nl_kind() ? (qh | PICO8_LINEAGE(PICO8_LINEAGE_NL)) : qh));
     }
     e = pico8_ga_kind()->new_entity(&keys);
-    assert(pico8_qh_of(e) == NULL && pico8_nl_of(e) == NULL);
+    assert(pico8_qh_of(e) == NULL && pico8_nl_of(e) == NULL && lineage_of(e) == jo);
     e = (const pico8_struct_header_t *)pico8_jo_new(&keys);
-    assert(pico8_qh_of(e) == NULL && pico8_nl_of(e) == NULL);
+    assert(pico8_qh_of(e) == NULL && pico8_nl_of(e) == NULL && lineage_of(e) == jo);
     assert(pico8_qh_of(NULL) == NULL && pico8_nl_of(NULL) == NULL);
+    assert(lineage_of(&pico8_g__new(NULL)->header) == PICO8_LINEAGE(PICO8_LINEAGE_G_));
+    assert(lineage_of(&pico8_fv_new()->header) == PICO8_LINEAGE(PICO8_LINEAGE_G_));
+    assert(lineage_of(&pico8_qb_new()->header) == 0 && lineage_of(&pico8_bg_new()->header) == 0);
+    assert(lineage_of(&pico8_mz_new()->header) == 0 && lineage_of(&pico8_jc_new(NULL)->header) == 0);
 }
 
 /* The galaxy depends on the rnd state: run each scenario in a child forked
diff --git a/tests/test_pico8_spec_defaults.c b/tests/test_pico8_spec_defaults.c
index 26c76be..03d91a5 100644
--- a/tests/test_pico8_spec_defaults.c
+++ b/tests/test_pico8_spec_defaults.c
@@ -136,8 +136,7 @@ static void test_layout_ready(void) {
 
 /* A fresh fv: its archetype's defaults copied, ke's under them. */
 static void test_fv_defaults_and_ready(void) {
-    pico8_fv_t *fv = pico8_fv_new();
-    const pico8_g__t *g_ = &fv->g_;
+    const pico8_g__t *g_ = pico8_fv_new();
 
     assert(g_->def == &pico8_fv_def && g_->emitter->def == &pico8_fv_def.emitter &&
            g_->probe->def == &pico8_fv_def.probe);
diff --git a/tests/test_pico8_struct_header.c b/tests/test_pico8_struct_header.c
index 77881a2..674a348 100644
--- a/tests/test_pico8_struct_header.c
+++ b/tests/test_pico8_struct_header.c
@@ -120,7 +120,7 @@ static void test_carriers_link_their_storage(void) {
     pico8_qb_t *qb = pico8_qb_new();
     pico8_mh_t *mh = pico8_mh_new(NULL);
     pico8_iv_t *iv = pico8_iv_new(NULL);
-    pico8_fv_t *fv = pico8_fv_new();
+    pico8_g__t *fv = pico8_fv_new();
     pico8_jo_t *jo = pico8_jo_new(NULL);
     pico8_fr_t *fr = pico8_fr_new(NULL);
     pico8_jo_t *fr_jo = &fr->nr.qh.jo;
@@ -139,18 +139,18 @@ static void test_carriers_link_their_storage(void) {
     assert_links(&qb->header, (pico8_struct_header_t){.ra = &qb->ra, .p = &qb->p, .ma = &qb->ma});
     assert_links(&mh->header, (pico8_struct_header_t){.fa = &mh->def->fa, .p_ = &mh->p_});
     assert_links(&iv->header, (pico8_struct_header_t){.fa = &iv->def->fa, .ra = &iv->ra, .p_ = &iv->p_});
-    assert_links(&fv->g_.header, (pico8_struct_header_t){.fa = &fv->g_.def->fa,
-                                                         .ra = &fv->g_.ra,
-                                                         .p = &fv->g_.p,
-                                                         .kq = &fv->g_.def->kq,
-                                                         .ql = &fv->g_.def->ql,
-                                                         .fg = &fv->g_.def->fg,
-                                                         .fx = &fv->g_.def->fx,
-                                                         .p_ = &fv->g_.p_,
+    assert_links(&fv->header, (pico8_struct_header_t){.fa = &fv->def->fa,
+                                                         .ra = &fv->ra,
+                                                         .p = &fv->p,
+                                                         .kq = &fv->def->kq,
+                                                         .ql = &fv->def->ql,
+                                                         .fg = &fv->def->fg,
+                                                         .fx = &fv->def->fx,
+                                                         .p_ = &fv->p_,
                                                          .m_ = &fv->m_,
-                                                         .emitter = &fv->g_.emitter_own,
-                                                         .probe = &fv->g_.probe_own});
-    assert(*fv->g_.header.fx == pico8_fixed_from_int(1) &&
+                                                         .emitter = &fv->emitter_own,
+                                                         .probe = &fv->probe_own});
+    assert(*fv->header.fx == pico8_fixed_from_int(1) &&
            "a probe's fx is its archetype's 1.0, Lua's true: 0 would read as nil");
     assert_links(&jo->header, (pico8_struct_header_t){.fa = &jo->def->fa,
                                                       .ra = (const pico8_vec_t *const *)&jo->ra,
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index 7aa6663..f27fc4a 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -901,7 +901,7 @@ static void test_components_carried(void) {
     assert_own_emitter(e[10], pico8_ga_new(NULL), sizeof(pico8_ga_t));
     assert_own_emitter(e[13], pico8_mx_new(NULL), sizeof(pico8_mx_t));
     assert_own_emitter(e[14], pico8_g__new(NULL), sizeof(pico8_g__t));
-    assert_own_emitter(e[15], pico8_fv_new(), sizeof(pico8_fv_t));
+    assert_own_emitter(e[15], pico8_fv_new(), sizeof(pico8_g__t));
     assert_own_probe(e[5], pico8_qh_new(NULL), sizeof(pico8_qh_t));
     assert_own_probe(e[6], pico8_nl_new(NULL), sizeof(pico8_nl_t));
     assert_own_probe(e[7], pico8_os_new(NULL), sizeof(pico8_os_t));
@@ -911,7 +911,7 @@ static void test_components_carried(void) {
     assert_own_probe(e[12], pico8_fr_new(NULL), sizeof(pico8_fr_t));
     assert_own_probe(e[13], pico8_mx_new(NULL), sizeof(pico8_mx_t));
     assert_own_probe(e[14], pico8_g__new(NULL), sizeof(pico8_g__t));
-    assert_own_probe(e[15], pico8_fv_new(), sizeof(pico8_fv_t));
+    assert_own_probe(e[15], pico8_fv_new(), sizeof(pico8_g__t));
     assert_own_probe(e[19], pico8_jc_new(NULL), sizeof(pico8_jc_t));
     assert_own_probe(e[23], pico8_mz_new(), sizeof(pico8_mz_t));
 }
```

### Comptes, mesurés sur la copie

| Mesure | `p` | `a` | `b` |
|---|---|---|---|
| `lien-parent` (lignes / fichiers) | 109 / 27 | **51 / 22** | **42 / 17** |
| `void-ptr` | 7 / 3 | **3 / 3** | 3 / 3 |
| `parente` | 10 / 3 | 10 / 3 | **9 / 2** |
| `accesseurs-q6`, `défauts` et les six autres gardées | 0, gardées | 0, gardées | 0, gardées |
| `inclusion-mutuelle` (paires / modules, lignes du `--check`) | 16 / 15, 40 | 16 / 15, 40 | 16 / 15, 40 |
| graphe d'usage (`usegraph.py`) | 19 | 19 | 19 |
| pied `résidu 2` | 162 / 45 | **104 / 42** | **94 / 38** |
| `--champs` | 238 ; 15 ; 20 | **240 ; 15 ; 20** | 240 ; 15 ; 20 |
| casts (grep de la réserve 4 de 2.13a) | 487 | **480** | **484** |
| tests de classe (`->cls`, `header.cls`, `_struct_class()`) | 656 | **632** | **636** |
| `sizeof(pico8_struct_header_t)` (x86-64) | 112 | **128** | 128 |
| `sizeof` `g_` / `fv` | 240 / 248 | 256 / 264 | **256 / 256** (un seul type) |
| `sizeof` `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `ga`, `nr`, `fr` | 288, 328, 336, 328, 392, 392, 400, 360, 328, 328 | + 16 chacun | idem `a` |
| `COMPOSITION_RESIDUE_GUARD` | 9 catégories | 9 | 9 |
| constantes `*_HASH`/`FROZEN_*` changées | — | 0 | 0 |

**Attribution de `lien-parent`** (étude 2.10, annexe D ; une ligne sort avec le dernier symbole qu'elle nomme). `a`, **58** : les 52 lignes de `src/` qui nomment `emitter_of`/`probe_of` (`probe.c` 12, `planet_family.c` 11, `particle.c` 5, `struct.h` 4 — deux lignes du commentaire de table, les deux membres —, `planet_family.h` 4, `planet_family_trade.c` 4, `probe.h` 3, 2 dans `help.c`, `planet_display.c`, `slipway.c`, `wormhole.c`, 1 dans `particle.h`) et les 6 de `test_pico8_ui.c` ; sortent `particle.c`, `particle.h`, `slipway.c`, `wormhole.c`, `test_pico8_ui.c` du `--detail` (le pied perd 3 fichiers : `slipway.c` et `wormhole.c` y restent par leurs lignes `#include` de paires). `b`, **9** (numéros de `a`) : `pico8_g__of` (`probe.c:82`, `:474`, `probe.h:50`, `test_pico8_fv.c:251`, `test_pico8_g_.c:306`, `:664`, `:689`), la marche `probe.c:478`, `.parent` de `fv` (`help.c:110`) ; sortent `help.c`, `help.h` (par `parente`), `probe.c`, `probe.h`, `test_pico8_fv.c`, `test_pico8_g_.c`. 58 + 9 = 67 = 109 − 42 : la table de l'étude (2.15a − 58, 2.15b − 9) à la ligne près.

**`void-ptr`** (`a`) : sortent les quatre prototypes `pico8_qh_probe_of`, `pico8_hj_emitter_of`, `pico8_g__emitter_of`, `pico8_g__probe_of` ; restent `pico8_hd_true`, `pico8_ga_mm`, `pico8_g__mm` (2.16). Chacune de ces quatre lignes était aussi dans `lien-parent` : le pied ne perd que 58 lignes en `a`.

**`parente`** (`b`) : sort `help.h:19` (`pico8_g__t g_;`) ; restent les neuf structs de sorte (2.15d).

**`--champs`** : `a` + 2 (les deux liens du noyau) ; `b` − `fv.m_` + `g_.m_` = 0.

**Casts et tests de classe** (motif appliqué aux lignes `+`/`-` de chaque diff) : `a`, casts − 7 dans `src/` (les sept implémentations `return ((pico8_X_t *)self)->…`), 0 net dans les TU ; tests de classe − 18 dans `src/` (les `->cls->emitter_of`/`probe_of`), − 6 dans `test_pico8_ui.c`. `b`, casts 0 net dans `src/` (`probe.c` perd celui de `pico8_g__of`, gagne celui de `probe_host_set_ra_p` ; `help.c` change `(pico8_fv_t *)` en `(pico8_g__t *)`), + 4 dans les TU (les trois `is_help`/`is_probe` qui testent le bit puis prennent la sonde, `test_pico8_g_.c:689`) ; tests de classe 0 net dans `src/` (`pico8_jo_is_family`, `probe_host_set_ra_p`), + 4 dans les TU (trois tests de bit, `lineage_of`). **Aucun cast neuf dans `src/`**, net, aux deux commits.

### Contrat pour 2.15c et la suite (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| `lien-parent` 42 / 17 à `b` | 2.15c | 30 lignes `pico8_qh_of`/`pico8_nl_of` (`src/`, 11 : définitions `planet_family.c:561`, `:570`, prototypes `planet_family.h:109`, `:113`, lecteurs `planet.c:234`, `planet_display.c:154`, `:429`, `:436`, `planet_trade.c:151`, `economy_score.c:47`, `economy_tiers.c:261` ; TU, 19 : `fr`, `hj`, `mx`, `nl` ×4, `nr`, `os` ×2, `pm`, `qh` ×8), les neuf `.parent` des sortes, les deux marches (`planet_family.c:565`, `:574` à `b`), le membre `parent` (`struct.h:166` à `b`) |
| bits `QH`, `NL` posés dans les dix tables de planète et épinglés par `test_qh_of_follows_parent` (`test_pico8_qh.c`) contre les descentes | 2.15c | retirer le côté descente du test (le masque seul, dix sortes, `g_`, `fv`, `qb`, `bg`, `mz`, `jc`) ; mutations « bit `QH` ôté de `nr` », « bit `NL` ôté de `nl` » déjà rouges (`qh`) à `b` : lignes K |
| `pico8_jo_is_family` lit le bit `JO` ; `pico8_g__of` n'existe plus | 2.15c, 2.17 | le test de sonde est `e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_G_)` suivi du cast ; il ne distingue pas `g_` de `fv` (fiche 12 de l'étude 2.11) |
| liens mutables : `ma`, `p_`, `m_`, `emitter`, `probe` | toutes | `emitter`, `probe` visent toujours `&self->X_own`, posés à côté du membre ; 2.15c les pose dans `pico8_jo_struct_init` si les membres montent dans `pico8_jo_t` (ils restent à poser pour les seules sortes porteuses) |
| TU `struct_header` : `planet_links(jo)` (les liens d'une planète hors émetteur et sonde), `assert_qh_links(qh, emitter)` ; les dix sortes, `g_`, `fv`, `mz`, `jc`, `mh`, `iv` construits | 2.15c, 2.15d | les chemins `&fr->nr.qh.probe_own`, `&pm->hj.qh`… deviennent des chemins aplatis à 2.15d |
| `pico8_g__t.m_` : `fv` seul pose le lien et l'initialise ; un `g_` le laisse indéterminé | toutes | un lecteur de `m_` passe par le lien (`ui:gw`), jamais par le membre sur une entité inconnue |
| tailles à `b` : noyau 128, sonde 256, planètes 304 à 416 | 2.15c/d | la forme A de l'étude (planète 432) se mesure à 2.15d ; l'étude comptait la sonde à 264 : `m_` loge dans le bourrage après `p_` (256) |
| `particle.c:12` (`pico8_planet.h`) | 2.17 | inclusion sans usage depuis 2.14b |

### Anti-tassement

**Interdit** :
- poser `emitter`/`probe` ailleurs que dans l'init du porteur, à côté du membre (`pico8_g__struct_init`, `pico8_qh_struct_init`, `pico8_hj_struct_init`, `_new` de `ga`, `mz`, `jc`) ; les poser vers `def` ; les rendre `const` avec un cast pour écrire à travers ; écrire un de ces liens dans un TU ;
- garder `emitter_of`/`probe_of` dans `pico8_struct_class_t` « pour les TU », ou une méthode qui rend le composant sous un autre nom (étude 2.10, §2) ;
- retirer les membres `emitter`/`probe`/`X_own` des types (2.15c/d décident de leur place) ;
- un masque autre que la chaîne `parent` restreinte (preuve 4) ; déduire la lignée des étiquettes ; poser `lineage` sur une des 28 autres tables ;
- toucher `pico8_qh_of`, `pico8_nl_of`, le membre `parent` ou les neuf `.parent` des sortes (2.15c) ; toucher les structs de sorte (2.15d) ;
- garder `pico8_fv_t` sous un alias, ou `pico8_g__of` ;
- initialiser `m_` dans `pico8_g__struct_init` ou y poser le lien `m_` (un `g_` n'est jamais survolé : ce serait un lien sur un non-porteur, jumelle N-mz-m_ de 2.14d2) ;
- retirer ou ajouter une ligne `#include` dans `src/` (2.17) ; toucher `particle.c:12` ;
- changer un `*.sha256`, `tests/e2e/`, `tools/`, le `Makefile`, une constante `*_HASH`/`FROZEN_*`.

## Implementation Chunks

Deux commits, dans l'ordre, chacun vert. **≈ 50 min chacun**, preuves comprises (mesuré sur la copie : l'essai et ses preuves ont pris ≈ 25 min par sous-étape) : ≈ 10 min pour appliquer le diff, `make test`, `make e2e` (aussi sous `MALLOC_PERTURB_=165`), compteur, `--champs`, tailles ; jumelles et lignes K en arrière-plan (a : 17 jumelles et 1 ligne K, ≈ 5 min ; b : 7 et 4, ≈ 4 min) ; ≈ 10 min pour les preuves 2, 4 à 9.

1. **2.15a** : appliquer le diff de `a` ; `make test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e`, compteur, `--champs`, tailles ; preuves 1 à 3, 5, 6 (L), 7, 8 (jumelles PR, EM, N ; K-init-a ; réserve du vérificateur de 2.14d) ; note « Découvert à 2.15a » ; commit.
2. **2.15b** : idem avec le diff de `b` (jumelles JO, G_, N ; K-QH-nr, K-NL-nl, K-QH-jo, K-init-b) et la preuve 4 (programme de lignée) ; note ; commit.

**Repli écrit** (si un commit déborde de 1,5 fois son budget avant d'être fait), chacun un commit vert :
- a en **a1** (liens posés, lecteurs génériques et TU `ui` sur les liens, méthodes encore dans les tables mais plus lues : `lien-parent` → 87) puis **a2** (implémentations, entrées, prototypes, membres de table retirés ; TU `struct_header` : → 51, `void-ptr` → 3) ;
- b en **b1** (`lineage`, enum, douze masques, `pico8_jo_is_family` et `probe_host_set_ra_p` sur le masque, `pico8_g__of` et `.parent` de `fv` retirés, TU `qh`, `fv`, `g_` : `lien-parent` → 42, `parente` 10) puis **b2** (`pico8_fv_t` aplati : `parente` → 9).
Comptes du repli calculés sur l'attribution ci-dessus (a1 : les 22 lignes lectrices sortent, `particle.c` 5, `particle.h` 1, `planet_display.c` 2, `probe.c` 8, `test_pico8_ui.c` 6), non mesurés ; le repli, s'il sert, est écrit dans la note « Découvert à ».

## Testing Decisions

- **Pas de TU neuf** : `tests/test_pico8_struct_header.c` s'étend (deux liens dans `assert_links` ; `mh`, `iv` dans `test_carriers_link_their_storage` ; test neuf `test_component_carriers_link_their_own` dans le même TU ; épingles de `*fx`) ; `tests/test_pico8_qh.c` étend `test_qh_of_follows_parent` au masque. Le reste est réécriture : méthodes appelées par les TU en lectures de lien, `pico8_fv_t` en `pico8_g__t`, `pico8_g__of` en test de bit. Aucun rejeu d'`events.json` dans un TU.
- **Bon test ici** : une épingle compare le lien à l'adresse du stockage (`h->probe == &qh->probe_own`) et au membre (`qh->probe == qh->jo.header.probe`), ou à NULL pour un non-porteur ; un masque se compare à la descente qu'il remplace sur la même instance, et à 0 pour une non-planète.
- **Prior art** : PRD 2.14c–d (jumelles, lignes K, TU sous perturbation, programme `obs.c`, preuve `const`) ; étude 2.10, §2 et annexe D ; règle du parent (D.21) ; ensemble `L` (2.4 à 2.8).

### Preuves (parent de `a` = `cecbcdb` ; de `b` = le commit 2.15a)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `MALLOC_PERTURB_=165 make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e tools Makefile '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide, aux deux commits (Runs 5, 18).

2. **Preuve `const`** (`a`) : dans une copie de `a`, les deux liens déclarés `const struct pico8_emitter *`/`const struct pico8_probe *`, tout l'arbre compilé en `cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only` (`src/*.c`, `tests/*.c`, hors fichiers raylib ; l'erreur de base `raylib.h` de `main_screencast_viewer.c` mise à part) → **3** erreurs `assignment discards 'const' qualifier` : `particle.c:37` (`f_`, qui écrit `emitter->qy`), `:50` (`ez`, qui étend `qy`), `probe.c:155` (`bv`, qui écrit `probe->et`). Ce sont les trois écrivains à travers ; aucun autre lecteur n'écrit. Preuve négative (`neg.c`, à `b`) : `e->cls->lineage = 0` → `assignment of member 'lineage' in read-only object` ; `g->header.probe = &g->def->probe` et `g->header.emitter = &g->def->emitter` → `incompatible pointer type` (un lien ne peut pas viser l'archétype) ; `e->probe->et = NULL`, `e->emitter->qy = NULL`, `g->header.probe = &g->probe_own` compilent (Runs 12).

3. **Mutations, règle du parent.** Une jumelle par porteur et par composant (« lien laissé NULL ») et par non-porteur essayé (« lien posé sur un non-porteur ») ; en `b`, par bit et par classe (« bit ôté », « bit posé sur une non-planète »). Au parent, la mutation équivalente : pour 2.15a, l'entrée `.probe_of`/`.emitter_of` retirée de la table (une fonction `static` y est gardée nommée par `0 ? f : NULL`), ou une entrée neuve qui rend un composant statique ; pour 2.15b, le prédicat au parent rendu faux pour la classe (`pico8_jo_is_family` avec `&& e->cls != pico8_X_struct_class()`, `pico8_g__of` qui rend NULL pour `g_`, `.parent` de `fv` retiré) ou vrai pour une non-planète. À l'enfant : la pose du lien retirée ou mise à NULL après l'init de la sorte, ou le bit ôté/posé dans la table. `make -k -j4 unit` puis `make -k e2e` ; restaurer par `git checkout -f`. **ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant.** Lignes K : enfant seul, `make e2e` normal et sous `MALLOC_PERTURB_=165`. Script : `mut.py` + `muts.json` (hors dépôt).

**Jumelles de 2.15a** (parent `p`, enfant `a`) :

| # | Cible | Parent : mutation | Rouges au parent | Enfant : mutation | Rouges à l'enfant | e2e p / e | Verdict |
|---|---|---|---|---|---|---|---|
| PR-qh | sonde, lignée `qh` (8 sortes) | `.probe_of` retiré des huit tables | `fr`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `qh`, `ui` | pose retirée de `pico8_qh_struct_init` | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| PR-nl | sonde, `nl` | `.probe_of` retiré de la table `nl` | `jo`, `nl`, `qh`, `ui` | lien mis à NULL après l'init, dans le `_new` de `nl` | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| PR-fr | sonde, `fr` | idem, table `fr` | `fr`, `jo`, `qh`, `ui` | idem, `_new` de `fr` | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| PR-mx | sonde, `mx` | idem, table `mx` | `jo`, `mx`, `qh`, `ui` | idem, `_new` de `mx` | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| PR-g_ | sonde, `g_` et `fv` | `.probe_of` retiré des deux tables | `class_coverage`, `line_coverage`, `bg`, `fv`, `g_`, `headless_capture`, `jc`, `replay_from_title`, `ui` | pose retirée de `pico8_g__struct_init` | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| PR-fv | sonde, `fv` | `.probe_of` retiré de la table `fv` | `class_coverage`, `line_coverage`, `fv`, `headless_capture`, `replay_from_title`, `ui` | lien mis à NULL après l'init, dans le `_new` de `fv` | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| PR-jc | sonde, `jc` | `.probe_of = 0 ? jc_probe_of : NULL` | `hp`, `hy`, `jc`, `js`, `lq`, `mx`, `mz`, `trade`, `ui` | pose retirée | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| PR-mz | sonde, `mz` | `.probe_of = 0 ? mz_probe_of : NULL` | `mz`, `ui` | pose retirée | `mz`, `struct_header`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| EM-hj | émetteur, `hj`, `pm`, `mx` | `.emitter_of` retiré des trois tables | `entity_kind`, `ga`, `hj`, `hp`, `ie`, `jo`, `jo_spec_defaults`, `mx`, `pm`, `qh`, `ui` | pose retirée de `pico8_hj_struct_init` | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| EM-pm | émetteur, `pm` | idem, table `pm` | `entity_kind`, `ga`, `hj`, `hp`, `jo`, `jo_spec_defaults`, `pm`, `qh`, `ui` | lien mis à NULL après l'init, dans le `_new` de `pm` | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| EM-ga | émetteur, `ga` | `.emitter_of = 0 ? ga_emitter_of : NULL` | `entity_kind`, `ga`, `ga_export_panel_by`, `hj`, `hp`, `jc`, `jo`, `jo_spec_defaults`, `pm`, `qb`, `qh`, `ui` | pose retirée | les mêmes + `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| EM-g_ | émetteur, `g_` et `fv` | `.emitter_of` retiré des deux tables | 37 TU (tout ce qui boote le cart : `fv` est construit au titre), dont `struct_header` | pose retirée de `pico8_g__struct_init` | les mêmes 37 | 0 / 0 | conforme |
| EM-fv | émetteur, `fv` | `.emitter_of` retiré de la table `fv` | 36 TU, dont `struct_header` | lien mis à NULL après l'init, dans le `_new` de `fv` | les mêmes 36 | 0 / 0 | conforme |
| N-jo-probe | `jo` reçoit une sonde (statique, `def` = `pico8_probe_def_none`) | entrée `.probe_of` neuve | `ui` | lien posé dans `pico8_jo_struct_init` pour la seule table `jo` | `struct_header`, `ui` | 26 / 26 | conforme (+`struct_header`) |
| N-bg-emitter | `bg` reçoit un émetteur statique | entrée `.emitter_of` neuve | `ui` | lien posé dans le `_new` de `bg` | `struct_header`, `ui` | 26 / 26 | conforme (+`struct_header`) |
| N-mz-emitter | `mz` reçoit un émetteur statique (réserve) | idem, table `mz` | `ui` | idem, `_new` de `mz` | `struct_header`, `ui` | 26 / 26 | conforme (+`struct_header`) |
| N-iv-probe | `iv` reçoit une sonde statique (réserve) | idem, table `iv` | `ui` | idem, `_new` de `iv` | `struct_header`, `ui` | 26 / 26 | conforme (+`struct_header`) |

**Jumelles de 2.15b** (parent `a`, enfant `b`) :

| # | Cible | Parent : mutation | Rouges au parent | Enfant : mutation | Rouges à l'enfant | e2e p / e | Verdict |
|---|---|---|---|---|---|---|---|
| JO-ga | bit `JO`, `ga` | `pico8_jo_is_family` faux pour `pico8_ga_struct_class()` | `ga`, `hp`, `jo` | `JO` ôté du masque de `ga` | `ga`, `hp`, `jo`, `qh` | 26 / 26 | conforme (+`qh`) |
| JO-jo | bit `JO`, `jo` | idem pour `pico8_jo_struct_class()` | 31 TU | `JO` ôté du masque de `jo` | les mêmes 31 | 0 / 0 | conforme |
| JO-nl | bit `JO`, `nl` | idem pour `pico8_nl_struct_class()` | `jo`, `nl`, `qh` | `JO` ôté du masque de `nl` | `jo`, `nl`, `qh` | 0 / 0 | conforme |
| G_-g_ | bit `G_`, `g_` | `pico8_g__of` rend NULL pour la table `g_` | `bg`, `g_`, `jc` | `G_` ôté du masque de `g_` | `bg`, `g_`, `jc`, `qh` | 0 / 0 | conforme (+`qh`) |
| G_-fv | bit `G_`, `fv` | `.parent` de `fv` retiré | `fv`, `g_` | `G_` ôté du masque de `fv` | `fv`, `g_`, `qh` | 26 / 26 | conforme (+`qh`) |
| N-mz-G_ | `mz` reçoit `G_` | `pico8_g__of` rend `mz` | `mz` | `.lineage = G_` dans la table `mz` | `mz`, `qh` | 0 / 0 | conforme (+`qh`) |
| N-bg-JO | `bg` reçoit `JO` | `pico8_jo_is_family` vrai pour `bg` | aucun | `.lineage = JO` dans la table `bg` | `qh` | 26 / 26 | conforme (+`qh`) |

**Lignes K** (enfant seul ; e2e normal, puis sous `MALLOC_PERTURB_=165`) :

| # | Mutation | Commit | Rouges | e2e | e2e perturbé |
|---|---|---|---|---|---|
| K-init-a | l'init du noyau copie tout sauf `emitter` et `probe` (`memcpy` de `sizeof z - 2 * sizeof(void *)`) | `a` | `struct_header`, `ui` | 26/26 | 26/26 |
| K-QH-nr | `QH` ôté du masque de `nr` | `b` | `qh` | 26/26 | — |
| K-NL-nl | `NL` ôté du masque de `nl` | `b` | `qh` | 26/26 | — |
| K-QH-jo | `QH` posé sur `jo` | `b` | `qh` | 26/26 | — |
| K-init-b | comme K-init-a | `b` | `struct_header`, `ui` | 26/26 | 26/26 |

   Lecture : **aucun ÉCHEC** (24 jumelles, 48 lancements ; 5 lignes K, toutes rouges). `struct_header` s'ajoute à l'enfant de 2.15a pour chaque porteur et chaque non-porteur qu'il construit ; `qh` à l'enfant de 2.15b pour chaque bit qu'il épingle. **N-bg-JO** est vert au parent (aucun TU ne demande si `bg` est une planète) et rouge à l'enfant par le test du masque (G3). **K-init** : seuls les TU voient un lien `emitter`/`probe` indéterminé ; le jeu ne lit ces liens que sur des porteurs (e2e 26, aussi perturbé). Les bits `QH`/`NL` n'ont pas encore de lecteur dans `src/` (2.15c) : leurs mutations ne peuvent pas être des jumelles à `b` (au parent, `qh_of` lit la chaîne `parent`, que l'enfant garde) ; ce sont des lignes K, rouges. Les deux TU nommés `class_coverage`, `line_coverage` sont `test_class_coverage`, `test_line_coverage`.

4. **Masque et descentes, table par table** (`lin.c`, hors dépôt, texte en annexe) : lié aux objets de `src/` (sans `main`, `-no-pie`), il lit l'adresse des 40 tables `g_*_struct_class` dans `nm` de son propre binaire et imprime pour chacune `QH`, `NL`, `G_`, `JO` : à `p` et `a` (`-DV=0`) les quatre descentes (`pico8_qh_of`, `pico8_nl_of`, `pico8_g__of`, `pico8_jo_is_family` sur un en-tête `{.cls = table}`) ; à `b` (`-DV=1`) les quatre bits de `lineage`, et sur sa sortie d'erreur `pico8_qh_of`, `pico8_nl_of` (encore sur la chaîne), `pico8_jo_is_family` (déjà sur le bit) et la chaîne `parent`. Résultat : **40 lignes, sorties identiques aux trois arbres** (même `md5`, `7e4ab61e…`) ; à `b`, les 40 × 3 colonnes de contrôle égales aux bits. Table (les 28 autres tables : 0 partout — `bg`, `bv_et`, `fm_placeholder`, `fq`, `ha_pz`, `hp`, `hy`, `ie`, `ie_ke_view`, `iv`, `jb`, `jc`, `jc_mouse_point`, `jc_too_far`, `je_placeholder`, `js`, `lq`, `mh`, `mo_placeholder`, `mz`, `mz_jj`, `nf`, `nr_ldown_empty`, `pj`, `pj_cr`, `qb`, `trade`, `ui`) :

| Classe | Chaîne `parent` (`a`) | `qh_of` / `QH` | `nl_of` / `NL` | `g__of` / `G_` | `is_family` / `JO` |
|---|---|---|---|---|---|
| `jo` | `jo` | 0 | 0 | 0 | 1 |
| `qh` | `qh` → `jo` | 1 | 0 | 0 | 1 |
| `nl` | `nl` → `qh` → `jo` | 1 | 1 | 0 | 1 |
| `os` | `os` → `qh` → `jo` | 1 | 0 | 0 | 1 |
| `hj` | `hj` → `qh` → `jo` | 1 | 0 | 0 | 1 |
| `pm` | `pm` → `hj` → `qh` → `jo` | 1 | 0 | 0 | 1 |
| `mx` | `mx` → `hj` → `qh` → `jo` | 1 | 0 | 0 | 1 |
| `nr` | `nr` → `qh` → `jo` | 1 | 0 | 0 | 1 |
| `fr` | `fr` → `nr` → `qh` → `jo` | 1 | 0 | 0 | 1 |
| `ga` | `ga` → `jo` | 0 | 0 | 0 | 1 |
| `g_` | `g_` | 0 | 0 | 1 | 0 |
| `fv` | `fv` → `g_` (à `b` : `fv`) | 0 | 0 | 1 | 0 |

   Les trois tables de `tests/` (`g_ra_only_class`, `g_fake_toast_class`, `g_giver_class`) n'ont ni `.parent` ni `[PICO8_STATE_LL]` (grep) : 0 aux deux formes. La table du contrôle 4 de l'étude, à l'identique (Runs 9, 19).

5. **Valeurs observables, parent contre enfant** (`obs.c`, hors dépôt) : lié aux objets de `src/`, il boote le cart, construit une instance fraîche de 29 classes (les 24 de `test_components_carried`, `lq`, `jb`, `hp`, `hy`, `trade`) et imprime, pour chacune, la présence d'un émetteur et d'une sonde (par la méthode à `p`, par le lien à `a`/`b`), les réponses de lignée (`pico8_g__of` ou bit `G_`, `pico8_jo_is_family`, `pico8_qh_of`, `pico8_nl_of`), `*m_` par le lien ; pour une sonde, `def->dw`, `dq`, `fp`, `od`, `has_od`, `et`, puis `pico8_dl` et l'empreinte de la mémoire ; pour un émetteur, `def->ki`, `py`, `qo`, `ro`, `qg`, `q_`, `k_`, `pn`/`du` (hors `ga`, qui les laisse indéterminés), puis `pico8_ez(e, 3)` et la taille de `qy` avant et après. Compilé `-DV=0` sur `p`, `1` sur `a`, `2` sur `b` : **29 lignes, sorties identiques** (`6fedbb30…`), et identiques sous `MALLOC_PERTURB_=165` aux trois arbres (Runs 10, 19).

6. **Chemin touché, ensemble `L`** : `make line-coverage EVENTS="<les 26 e2e>"` à `p`, `a`, `b` (6064, 6057, 6052 lignes), appariement `difflib` des sources (`lset.py`, hors dépôt). **Aucune ligne inchangée perdue ni gagnée**, aux deux pas. `L` (couvertes au parent, retirées ou réécrites) : `a` **29** — `particle.c:35`, `:37`, `:50`, `:78`, `:80`, `planet_display.c:468-469`, `planet_family.c:29-30`, `:205-206`, `:448-449`, `probe.c:153`, `:155`, `:227`, `:229`, `:295`, `:297`, `:337-338`, `:483-484`, `:487-488`, `slipway.c:341-342`, `wormhole.c:196-197` (lecteurs et implémentations) ; `b` **26** — `help.c` (18 lignes réécrites de `fv`), `planet.c:64`, `probe.c:82`, `:86`, `:474-480` (`pico8_g__of` et sa marche). Lignes exécutables neuves : `a`, **22 sur 22** couvertes (les lecteurs réécrits et les sept poses : `planet_family.c:52`, `:223`, `:456`, `probe.c:390-391`, `slipway.c:356`, `wormhole.c:217`, numéros de `a`) ; `b`, **21 sur 21** (`help.c` réécrit, `planet.c:64`, `probe.c:86-87`) ; les lignes neuves non couvertes sont des déclarations, des initialiseurs de table (`.lineage = …`) et des commentaires (Runs 11, 19).

7. `class-coverage-net` : **oui**, `couverture: OK` à `a` et `b` (comptes égaux à `tools/class_coverage_net_expected.txt`). `sanitize-net` : **oui** (le noyau grandit de 16 octets, la sonde change de forme), `sanitize: 26 propre(s)` à `a` et `b` (Runs 13, 19).

8. **Réserve du vérificateur** : le TU `struct_header` construit `mh`, `iv`, `g_`, `mz` et les compare lien par lien ; N-mz-emitter et N-iv-probe rougissent `struct_header` à l'enfant (preuve 3) ; les épingles de `*fx` (`g_`, `fv`) sont dans le même TU.

9. **Graphe d'usage** : 19 aux trois arbres, mêmes paires et mêmes symboles (`diff` des sorties vide) ; `inclusion-mutuelle` 16 / 15 (40 lignes) aux trois.

### Greps de Sortie

À `b` (numéros de `b`) :
1. `grep -rnE 'header\.(emitter|probe) = ' src` → **7** lignes, toutes `&self->X_own` : `planet_family.c:53` (sonde, `qh`), `:227` (émetteur, `hj`), `:462` (émetteur, `ga`), `probe.c:392-393` (`g_`, deux), `slipway.c:356`, `wormhole.c:217` (sonde). Chacune est suivie de la pose du membre au même stockage.
2. `grep -rnE '\b(emitter_of|probe_of)\b' src tests` → vide (dès `a`).
3. `grep -rn '\.lineage = ' src | wc -l` → **12** ; `grep -rn 'lineage' src | grep -v '\.lineage = '` → `planet.c:64` (`pico8_jo_is_family`), `probe.c:86` (`probe_host_set_ra_p`), `struct.h` (enum, membre).
4. `grep -rnE 'pico8_fv_t|pico8_g__of' src tests` → vide ; `grep -n 'parent' src/pico8_help.c` → vide.
5. `grep -rn '\.parent = ' src | wc -l` → **9** (les neuf sortes, `planet_family.c` ×6, `planet_family_trade.c` ×3).
6. `git diff <parent> -- src | grep -E '^[-+]#include'` → vide aux deux commits ; `grep -rn 'pico8_struct_header_init(' src | wc -l` → 9 ; `grep -rnE '(\.|->)cls *= [^=]' src tests | grep -v '{'` → exactement `tests/test_pico8_planet_kj_dispatch.c:91` et `:128`.
7. `make composition-residue`, `--champs`, les greps de casts et de classe → la table « Comptes ».
8. `git diff --stat <parent>` : les fichiers de la section « Modules » et `docs/plan-3.md`, rien d'autre.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd215ab/copie`, arbres de travail `wt_*`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 1f29e78` ; diff 2.14d2 extrait du PRD 2.14c–d, `git apply --check`/`git apply`, commit | `1f29e78` + d2 | 0 | s'applique sans reste | 2 s |
| 2 | `make -j4 test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` ; compteur | d2 | 0 | vert ; 26 ; 26 ; pied 162 / 45 | 45 s |
| 3 | 2.15a écrit (`struct.h`, sept porteurs, lecteurs, TU `ui`, `struct_header`) ; `make test`, e2e ×2, compteur | `a` | 0 | vert ; 26 ; 26 ; `lien-parent 51 22`, `void-ptr 3 3` | 1 min |
| 4 | 2.15b écrit (enum, douze masques, `pico8_fv_t` aplati, `pico8_g__of` retiré, TU) ; idem | `b` | 0 | vert ; 26 ; 26 ; `parente 9 2`, `lien-parent 42 17` | 1 min |
| 5 | `base.sh` (test, e2e, e2e perturbé, compteur, `--champs`, `--check inclusion-mutuelle`, casts, tests de classe, `usegraph.py`, `sizeof`, garde) | `p`, `a`, `b` | 0 | table « Comptes » | 1 min |
| 6 | `usegraph.py` (premier jet tronqué à l'extraction, `IndentationError`) ré-extrait de l'annexe A du PRD 2.11 | les trois | 1 puis 0 | `paires 19` ×3, sorties identiques | < 2 s |
| 7 | message du coordinateur : partir de `1f29e78`/`86c94b8` ; `git fetch`, `git diff --stat <d2 de la copie> cecbcdb -- src tests Makefile tools` | — | 0 | vide (seuls `docs/` diffèrent) | 1 s |
| 8 | `cherry-pick` de `a`, `b` sur `cecbcdb` ; réserve du vérificateur (TU `struct_header` : `mh`, `iv`, épingles `*fx`), `commit --amend`, `b` repris (conflit sur les lignes `fv` du TU, résolu par la réécriture de `b`) | `cecbcdb` + a, b | 0 | TU `struct_header` vert | 1 min |
| 9 | `lin.c` sur `p`, `a` (`-DV=0`), `b` (`-DV=1`), symboles par `nm` | les trois | 0 | 40 lignes, trois sorties identiques ; contrôles de `b` égaux | < 5 s |
| 10 | `obs.c` sur `p` (0), `a` (1), `b` (2), normal et perturbé ; premier jet avec les décalages des composants (changent avec le noyau), remplacés par leur présence | les trois | 0 | 29 lignes, six sorties identiques | < 5 s |
| 11 | `make line-coverage` des 26 e2e ; `lset.py` | `p`, `a`, `b` | 0 | 6064, 6057, 6052 ; 0 perdue ; `L` 29 et 26 ; neuves 22/22, 21/21 | 20 s |
| 12 | preuve `const` (`syn.sh`, liens `const`) ; `neg.c` | `a`, `b` | 0 / 1 | 3 erreurs `discarded-qualifiers` ; 3 erreurs attendues | 1 min |
| 13 | `make class-coverage-net` ; `make -j2 sanitize-net` | `a`, `b` | 0 | `couverture: OK` ×2 ; `26 propre(s)` ×2 | 20 s |
| 14 | `mut.py`, 2.15a (17 jumelles, K-init-a) ; premier jet de PR-jc : `COMPILE` au parent (fonction `static` inutilisée), corrigé en `0 ? f : NULL` (PR-jc, PR-mz, EM-ga) | `p`, `a` | 0 | aucun ÉCHEC | ≈ 5 min |
| 15 | `mut.py`, 2.15b, premier jet : arrêt sur JO-ga (la tranche de table du script excluait la dernière ligne, où est `.lineage`), corrigé | `a`, `b` | 1 | — | < 1 min |
| 16 | `mut.py`, 2.15b (7 jumelles, 4 lignes K) | `a`, `b` | 0 | aucun ÉCHEC ; N-bg-JO vert aux deux bouts | ≈ 4 min |
| 17 | N-bg-JO : `test_qh_of_follows_parent` épingle aussi `bg`, `mz`, `jc` à 0 (G3), `commit --amend` de `b` ; `make test` ; les 7 jumelles et 4 lignes K de 2.15b relancées | `b` | 0 | N-bg-JO conforme (+`qh`) ; aucun ÉCHEC | ≈ 4 min |
| 18 | mise en forme : quatre lignes de TU > 120 colonnes coupées (`struct_header` en `a`, `fv`, `g_` en `b`) ; `base.sh` sur les commits finaux | `a`, `b` | 0 | table « Comptes » inchangée ; vert, 26, 26 | 1 min |
| 19 | `lin.c`, `obs.c`, `line-coverage` + `lset.py`, `class-coverage-net`, `sanitize-net` relancés sur `b` après 17 | `p`, `a`, `b` | 0 | inchangés | 1 min |
| 20 | les deux diffs extraits de ce PRD, `git apply --check`/`git apply` sur `cecbcdb`, `git diff` contre `b` | — | 0 | deux `git apply` sans reste ; diff vide | 2 s |
| 21 | `rm -rf` de la copie, des arbres de travail et des fichiers d'essai | — | 0 | supprimés | < 1 s |

Les jumelles ont tourné sur les arbres d'avant la ligne 18 (`a` et `b` qui n'en diffèrent que par la coupure de quatre lignes de TU) : objets de `src/` identiques, textes d'`assert` identiques.

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `cecbcdb` (ou un descendant qui ne touche que `docs/`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si 2.15a et 2.15b ne sont pas deux commits (hors repli écrit), dans cet ordre, chacun vert (`make test`, `make e2e` 26 conformes, et 26 sous `MALLOC_PERTURB_=165`), ou si un `*.sha256`, `tests/e2e/`, `tools/`, le `Makefile` ou une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant ; si une ligne K est verte.
- `FAILED:` si un lien `emitter`/`probe` est posé ailleurs que dans l'init de son porteur, vers autre chose que `&self->X_own`, ou écrit dans un TU ; s'il reste `emitter_of`/`probe_of` dans `src/` ou `tests/` à `a` ; si un membre `emitter`/`probe`/`X_own` d'un type est retiré.
- `FAILED:` si un masque diffère de la table de la preuve 4, si une des 28 autres tables reçoit `.lineage`, si la sortie de `lin.c` diffère entre `p`, `a` et `b`, ou si celle d'`obs.c` diffère (aussi sous perturbation).
- `FAILED:` s'il reste `pico8_fv_t`, `pico8_g__of` ou `.parent` dans la table de `fv` à `b` ; si `pico8_qh_of`, `pico8_nl_of`, le membre `parent` ou un `.parent` de sorte est touché.
- `FAILED:` si le TU `struct_header` ne construit pas `g_`, `mz`, `mh`, `iv`, les dix sortes de planète, ou ne compare pas `emitter`/`probe` pour chacun ; si l'épingle de `*fx` sur `g_`/`fv` manque ; si `test_qh_of_follows_parent` ne compare pas le masque.
- `FAILED:` si les comptes ne sont pas ceux de la table « Comptes » (`lien-parent` 51 / 42 ; `void-ptr` 3 / 3 ; `parente` 10 / 9 ; pied 104 / 94 ; `--champs` 240 / 240 ; noyau 128), si une catégorie gardée n'est plus à 0, si `inclusion-mutuelle` ou le graphe d'usage bougent, si une ligne `#include` de `src/` est ajoutée ou retirée, ou si un cast neuf net apparaît dans `src/`.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge ; si une ligne inchangée entre dans `L` ou qu'une ligne exécutable neuve n'est pas couverte.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un message de commit omet : les liens posés et leur porteur, les méthodes retirées, les masques, la table des jumelles avec leur résultat au parent, les lignes K, la preuve `const`, la table de la preuve 4, les comptes, `L`, le tableau « Runs ».
- `FAILED:` s'il manque une note « Découvert à 2.15a » ou « Découvert à 2.15b » au bout de sa ligne du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les champs d'état des sortes, `pico8_qh_of`/`pico8_nl_of`, le membre `parent` (2.15c) ; les neuf structs de sorte (2.15d) ; les membres `hd`/`mm` typés (2.16).
- Distinguer `g_` de `fv` sans étiquette ni masque (fiche 12 de l'étude 2.11, 2.17) ; `particle.c:12` et les paires d'inclusion (2.17).
- La taille du code C (Q-2.10 : hors phase 2) ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. `pico8_jo_is_family` passe au bit `JO` dès 2.15b**, comme la table de l'étude 2.10 (§2, ligne 2.15b) ; la ligne 2.15b du plan ne le nomme pas, la ligne 2.15 si. Même réponse sur les 40 tables (preuve 4) : le changement ne déplace rien.
- **G2. Les liens sont posés à `&self->X_own`, sur la ligne qui précède la pose du membre**, pas en recopiant le membre après coup : la forme de l'étude (« posés par l'init à `&self->emitter_own`/`&self->probe_own` ») et un seul stockage nommé deux fois ; « lien = membre » est vérifié par le TU.
- **G3. `test_qh_of_follows_parent` épingle le masque dès 2.15b** (dix sortes contre `qh_of`/`nl_of`, `g_`/`fv` à `G_`, `qb`, `bg`, `mz`, `jc` à 0) : sans lui, les bits `QH`/`NL`, qui n'ont pas de lecteur avant 2.15c, et un bit posé sur une non-planète (N-bg-JO, vert au premier essai) ne seraient vus par aucun TU. 2.15c n'aura qu'à retirer le côté descente.
- **G4. Les porteurs neufs du TU `struct_header` sont comparés lien par lien** (seize liens), pas seulement sur `emitter`/`probe` : c'est ce qui voit un lien parasite (réserve du vérificateur) ; deux aides (`planet_links`, `assert_qh_links`) évitent de répéter les huit liens d'une planète.
- **G5. L'épingle de `*fx` porte un message** (« a probe's fx is its archetype's 1.0, Lua's true: 0 would read as nil ») : c'est le piège que nomme le vérificateur ; 2.15b ne touche pas `fx`.
- **G6. `g_` laisse `m_` indéterminé** et le commentaire de `pico8_g__t` le dit : l'initialiser dans `pico8_g__struct_init` écrirait un champ sans lecteur pour `g_`, et poser son lien ferait de `g_` un porteur de `m_`, ce qu'il n'est pas (le cart n'écrit `m_` que sur ce que `ui:gw` survole).
- **G7. Le repli de 2.15a garde un état où les méthodes sont encore dans les tables mais plus lues** (a1) : c'est la seule coupe qui laisse chaque commit vert sans forme transitoire des lecteurs.

### Surprises de mesure, pour `plan-pilot`

1. **La sonde reste à 256 octets**, pas 264 comme l'étude : `m_` loge dans le bourrage après `p_`. L'aide passe de 264 à 256.
2. **Un bit sans lecteur ne peut pas avoir de jumelle** : à `b`, `QH`/`NL` ne sont lus que par le TU ; la mutation « bit `QH` ôté de `nr` » que le plan nomme est une ligne K ici et devient une jumelle à 2.15c.
3. **N-bg-JO était vert aux deux bouts** au premier essai : aucun TU ne demandait si une non-planète est une planète. Corrigé par G3.
4. **K-init n'atteint que les TU** : le jeu ne lit `emitter`/`probe` que sur des porteurs, même sous perturbation (26 e2e) ; seul le TU `struct_header` (et `test_components_carried` de `ui`) voit l'oubli.
5. Les numéros du plan ont dérivé : `test_pico8_ui.c:855-909` est `:837-915` à `p` (les deux aides et `test_components_carried`), `help.h:21` est `:16-21`.

### Questions pour l'humain

Aucune.

### Annexe — `lin.c` (preuve 4, texte exact)

Construit par : `cc -std=c99 -DV=<0|1> -Isrc -c lin.c && cc -no-pie -o lin lin.o $(ls tmp/obj/src/pico8_*.o | grep -v fixed_test_double) -lm`, après `make test` dans l'arbre ; lancé par `nm lin | awk '$3 ~ /^g_[a-z0-9_]+_struct_class$/ {print $1, $3}' | sort -k2 | ./lin`.

```c
/* Lignee de chaque table de classe : descentes (V=0) contre masque (V=1). */
#include <stdio.h>
#include <string.h>
#include "pico8_struct.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_probe.h"
static unsigned long addrs[64]; static char names[64][96]; static int n;
static const char *name_of(const void *c){ int i; for(i=0;i<n;i++) if((const void*)addrs[i]==c) return names[i]; return "?"; }
int main(void){
    unsigned long a; char nm[96]; int i;
    while (scanf("%lx %95s", &a, nm) == 2) { addrs[n] = a; strcpy(names[n], nm); n++; }
    for (i = 0; i < n; i++) {
        const pico8_struct_class_t *c = (const pico8_struct_class_t *)addrs[i];
        pico8_struct_header_t h = {.cls = c};
        const pico8_struct_class_t *p;
        char chain[256] = "";
        for (p = c; p != NULL; p = p->parent != NULL ? p->parent() : NULL) { strcat(chain, name_of(p) + 2); chain[strlen(chain) - 13] = 0; strcat(chain, ">"); }
#if V == 0
        printf("%-34s QH=%d NL=%d G_=%d JO=%d\n", names[i], pico8_qh_of(&h) != NULL, pico8_nl_of(&h) != NULL,
               pico8_g__of(&h) != NULL, pico8_jo_is_family(&h));
#else
        printf("%-34s QH=%d NL=%d G_=%d JO=%d\n", names[i], (c->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) != 0,
               (c->lineage & PICO8_LINEAGE(PICO8_LINEAGE_NL)) != 0, (c->lineage & PICO8_LINEAGE(PICO8_LINEAGE_G_)) != 0,
               (c->lineage & PICO8_LINEAGE(PICO8_LINEAGE_JO)) != 0);
        fprintf(stderr, "%-34s qh_of=%d nl_of=%d is_family=%d\n", names[i], pico8_qh_of(&h) != NULL, pico8_nl_of(&h) != NULL, pico8_jo_is_family(&h));
#endif
        fprintf(stderr, "chain %-34s %s\n", names[i], chain);
    }
    return 0;
}
```
