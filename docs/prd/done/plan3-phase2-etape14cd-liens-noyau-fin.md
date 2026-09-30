---
title: "Plan 3, étapes 2.14c, 2.14d1 et 2.14d2 : les liens de composant `kq`, `ql`, `fg`, `mt`, `ma`, `fx`, `p_`, `m_` au noyau ; `accesseurs-q6` 123 → 80 → 32 → 0, gardé ; `lien-parent` 110 → 109 ; `void-ptr` 8 → 7 — trois commits, jeu identique, huit empreintes de TU à trancher"
description: "2.14c : cinq liens (`kq` de `js` monté dans le noyau, `ql`, `fg`, `mt` reposé aux deux écritures de `has_mt`, `ma` mutable) ; `pico8_ma` n'emploie plus aucun nom de `planet.h` ni de `scroll.h` ; TU `struct_header` étendu, dont un vrai test du point souris de `jc:hq` sous perturbation. 2.14d1 : `fx` (`jo.fx` en `pico8_fixed_t`), `p_` mutable, `probe_host_raise_p_` réduit à deux lignes, `has_fx`/`has_p_` retirés. 2.14d2 : `m_` mutable, `has_m_` retiré, table de classe sans accesseur, `accesseurs-q6` gardé. Essayé sur copie : vert, 26 e2e (aussi sous perturbation) à chaque commit ; jumelles et lignes K sans ÉCHEC. Retirer `has_p_`/`has_m_` change huit constantes d'empreinte de TU (nil et faux confondus) : une question à l'humain avant 2.14d1."
status: needs-triage
---

## Problem Statement

**Écrit sur `2a91975`** (branche `claude/plan-pilot-3-7zdft0`), qui porte 2.14a (`56ce4f6`), 2.14b (`8fee30f`), les commentaires refermés de `6e1fe46` et les notes de 2.14a/b ; un implémenteur travaillait dans l'arbre de travail (fichiers non commités non lus). Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Essais sur une seule copie jetable (scratchpad `prd214cd/copie`, supprimée depuis). Notation : `p` = `2a91975`, l'arbre d'après 2.14b ; `c` = `684dca8`, `d1` = `e7d4ce9`, `d2` = `3a56600`, les commits de la copie qui portent les trois diffs de « Ce qui change ». Les numéros de ligne cités sont ceux de `p`, sauf mention. Historique : un premier essai, interrompu par un redémarrage, avait été fait sur `2859307` + les diffs de 2.13e2b/e2c et de 2.14a/b ; ses diffs ont été réappliqués sur `2ba060c` + les diffs du PRD 2.14ab (identique en `src/ tests/ Makefile tools/`), puis repris par `cherry-pick` sur `2a91975` (qui n'en diffère que par trois commentaires d'en-tête et une ligne de `test_pico8_pj.c`, sans conflit) ; toutes les mesures, mutations et preuves ont été relancées sur `2a91975` (Runs). Deux changements du diff en sont sortis : une jumelle (MT-jc, G2) et les réserves du vérificateur de 2.14a/b (table suivante).

Vu du mainteneur du port : après 2.14b, le code générique lit encore sept groupes de champs par la table de classe (`get_kq`, `get_ql`, `get_fg`, `get_mt`, `get_fx`, `get_p_`, `set_m_` : 123 lignes du compteur), et `pico8_ma` lit `ma` par un test de famille suivi d'un cast. 2.14c, 2.14d1 et 2.14d2 posent les huit derniers liens de composant du noyau, vident `accesseurs-q6` et le mettent sous garde ; `pico8_struct_class_t` n'a plus aucun accesseur de champ.

### Texte du plan, recopié (`docs/plan-3.md` à `2a91975`)

> - 2.14 — `accesseurs-q6`, selon l'étude 2.10 (§1) : chaque emplacement Q6 remplacé par un lien typé du noyau vers le stockage que 2.9 fixe (`&def->X` pour un champ A sur ce type, `&self->X` sinon, NULL pour un non-porteur), posé une fois par l'init. `ql` est A sur ses deux porteurs (lecture d'archétype par le lien), les neuf autres groupes sont des composants. Les lecteurs génériques lisent `e->X` ; les accès des modules porteurs ne changent pas. Comptes calculés sur `--check accesseurs-q6`, une ligne sortant avec le dernier emplacement qu'elle nomme (étude, annexe A). Preuves : chemin touché (`gcov` des lecteurs du groupe ; mutations : lien laissé NULL sur un porteur, lien posé sur un non-porteur là où un lecteur observe l'absence, `dc`, `pico8_gk`, `jc_mt_of`) ; un lien de champ A vise `def` (grep de l'init) ; retrait d'un `has_X` prouvé par le grep de ses lectures et par une mutation. `class-coverage-net` : oui. `sanitize-net` : oui (le noyau grandit). Budget ≈ 5 × 50 min.
>   - 2.14c — `kq` (le membre de `js` monte dans le noyau), `ql`, `fg`, `mt` (lien de `jc` reposé à ses deux écritures de `has_mt`), et le lien `ma` (`entity_geom.c:79`). Compte : → **80**. (2.11 : retire les derniers usages de `planet` et de `scroll` dans `pico8_ma` ; `entity_geom` ↔ `planet` et `entity_geom` ↔ `scroll` gardent `pico8_na` et `pico8_gd`, coupés à 2.17e.)
>   - 2.14d1 — `fx` (`jo.fx` en `pico8_fixed_t`), `p_` (lien mutable, `probe_host_raise_p_` réduit) ; `has_fx` et `has_p_` retirés. Compte : → **32** ; `lien-parent` → **109** (`probe.c:283`).
>   - 2.14d2 — `m_` (lien mutable) ; `has_m_` retiré ; commentaire `struct.h:139-141`. Compte : → **0**, gardé ; `void-ptr` 8 → **7** (`pico8_jo_set_m_`, `planet.h:122`).

Décision de l'humain (même fichier) : **Q-2.10** — « oui, chaque objet reçoit les liens de composant du noyau (environ 100 octets de plus par objet sur PC, 50 sur la PSX). La taille du code C est reprise plus tard, hors phase 2. » Notes « Découvert à 2.10 » : forme (R2) ; `ql` lu sur l'archétype, les autres groupes composants, de même `ma` (`jo`, `qb`) hors Q6 ; noyau 24 → 112 → 128 octets ; « `has_fx`, `has_p_`, `has_m_` ne sont lus que dans `has_X && X` : retirés avec leur groupe ». Notes « Découvert à 2.11 » : trois paires dépendent de Q-2.10, dont `entity_geom` ↔ `planet` et `entity_geom` ↔ `scroll` (lien `ma`) ; elles ne tombent pas ici, 2.17e les coupe.

Règles communes du plan (`docs/plan-3.md:368`) : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; **aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée (§3 : aucun re-baseline)** ; `défauts` reste à 0 et gardé. Ce PRD tient toutes ces règles en 2.14c ; en 2.14d1 et 2.14d2, le retrait de `has_p_` et `has_m_` que le plan demande ne peut pas les tenir toutes (voir « Question pour l'humain »).

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que font 2.14c/2.14d1/2.14d2 |
|---|---|
| `pico8_struct_header_init(self, cls)` met **tout** le noyau à zéro ; un lien neuf n'a qu'à être déclaré, puis posé par l'init du porteur après elle ; ne jamais réécrire `cls` à la main (PRD 2.14ab, « Contrat pour 2.14c ») | tenu : huit membres déclarés, aucune init touchée ; grep de Sortie 6 (deux réaffectations de TU seulement, neuf appels de l'init) |
| `test_pico8_struct_header.c` : chaque lien neuf entre dans `assert_links` ; le TU se relance sous `MALLOC_PERTURB_=90` (idem) | `assert_links(h, want)` prend désormais l'attendu comme un `pico8_struct_header_t` (littéral composé, liens non nommés à NULL) : un lien de plus = une ligne de comparaison (G1) ; attendus de `fq`, `ui`, `qb`, `fv`, `jo`, `fr`, `js` avec `fg`, `jc` ajoutés |
| liens `ra`, `p` : lecture `e->ra != NULL ? *e->ra : NULL` (idem) | `ma` suit la même forme mais **mutable** (`pico8_fixed_t *`), comme `p_` et `m_` (`int *`) ; les cinq autres sont `const` (preuve 2) |
| bouts d'un `jc` : `jc_mt_of` lit encore `get_mt` sur les mêmes bouts (idem) | **c** : `jc_mt_of` lit `e->mt` ; le lien `mt` de `jc` est reposé à ses deux écritures de `has_mt` (`_new`, `jc:hq`) |
| `particle.c:12` (`pico8_planet.h`) sans usage depuis 2.14b : pour 2.17 (idem) | non touché ; reporté au contrat pour 2.17 |
| point souris : un chemin qui oublierait l'init du noyau n'était vu que par le grep (K-mp vert à 2.14a) ; **réserve** : si 2.14c touche `jc`, un vrai test (demande de `plan-pilot`) | **c** : `jc_mt_of` lit désormais `re->mt` sur le point souris ; `test_mouse_point_has_null_links` (TU `struct_header`, sous perturbation) construit le point par `jc:hq` et compare ses dix liens ; K-mp devient **rouge** (TU et e2e, preuve 3) |
| `test_pico8_pj.c`, `ed_fixed(ju) == 1` : la première sous-étape qui touche ce fichier (idem) | levée par `6e1fe46` (l'assertion est partie) ; un correctif de ce TU est annoncé : fichier non touché par 2.14c/d1/d2 |
| **Réserve 1 du vérificateur de 2.14a/b** (`2a91975`) : le point souris de `jc` est le seul constructeur hors `ke` que le TU `struct_header` ne couvre pas (mutation « init retirée » toute verte) ; 2.14c touche `jc` : l'ajouter | **c** : `test_mouse_point_has_null_links` construit le point par `jc:hq` et épingle ses liens ; K-mp **rouge** à `c` (preuve 3) |
| **Réserve 2** : le commentaire du TU dit que tout `malloc` remplit son bloc, faux sous glibc pour un bloc recyclé par le tcache ; corriger, et rendre le TU robuste si simple | **c** : la relance pose aussi `GLIBC_TUNABLES=glibc.malloc.tcache_count=0` ; `assert_malloc_fills` lit désormais un bloc **recyclé** (`calloc`, `free`, `malloc`) ; commentaire corrigé. Mesuré : sous `MALLOC_PERTURB_=90` seul, un bloc recyclé sort du tcache non rempli (0/64 octets à `0xa5`) et `assert_malloc_fills` arrête le TU ; tcache coupé, 64/64 (preuve 9, G8) |
| **Réserve 3** : le lien `fa` de `jc` (`&self->fa`, champ I) n'est gardé par aucun TU (mutation vers `&def->fa` verte partout) | **c** : `test_mouse_point_has_null_links` épingle `assert_links(&jc->header, {.fa = &jc->fa})` sur deux `jc` tirés par `jc:hq` ; jumelle I-jc : verte à `p`, **rouge** (`struct_header`) à `c` |
| **Réserve 4** : « Non-carriers, one per constructor that does not go through ke » inexact tant que la réserve 1 n'est pas faite | **c** : le commentaire renvoie au test du point souris |
| `emitter_host_fa_ra` ne lit que `self->fa`, `self->ra` (idem) | sans objet (2.15) |
| « `has_fx`, `has_p_`, `has_m_` ne sont lus que dans `has_X && X` » (étude 2.10, §1) | vrai pour `src/` (`git grep` à `p`) ; **faux pour les TU** : 21 TU lisent `has_p_`/`has_m_` (`git grep` à `2ba060c`), et six d'entre eux (`fq`, `ga`, `mz`, `bg`, `fv`, `js`) plient dans une empreinte l'état « faux présent » que le retrait confond avec nil — d'où la question |

### Question Q-2.10 : tranchée « oui » (`259fabe`)

2.14c à 2.14d2 en dépendent entièrement. Mesuré : `sizeof(pico8_struct_header_t)` passe de 48 à **88** (c), **104** (d1), **112** octets (d2) sur x86-64 — le 112 de l'étude 2.10.

### État de départ attendu (`2a91975`, ou un descendant qui ne touche que `docs/` ou `tests/test_pico8_pj.c`)

Mesuré sur `p` :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 110 27`, `accesseurs-q6 123 21`, `void-ptr 8 4`, `inclusion-mutuelle 16 15` ; **`résidu 2 : 286 ligne(s), 54 fichier(s)`** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 245 ; 0 écriture : 15 ; 0 hors constructeur : 20` |
| `--check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 40 ligne(s)` |
| `COMPOSITION_RESIDUE_GUARD` (`Makefile:874`) | `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts` |
| casts / tests de classe (greps de la réserve 4 de 2.13a) | 500 / 677 |
| `sizeof(pico8_struct_header_t)` | 48 |
| graphe d'usage (`usegraph.py`, annexe A du PRD 2.11) | `paires 19` |
| `make test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | vert ; 26 conformes ; 26 conformes |

Un départ autre : arrêt, rapporté. Contrôle rapide : `git diff 2a91975 <départ> -- src tests Makefile tools ':!tests/test_pico8_pj.c'` doit être vide ; sinon les diffs ci-dessous peuvent ne plus s'appliquer (`git apply --check`), et le PRD est à rejouer.

## Solution

Trois commits, chacun vert, jeu identique (26 e2e conformes, aussi sous `MALLOC_PERTURB_=165`).

**2.14c — `kq`, `ql`, `fg`, `mt`, `ma`** (`accesseurs-q6` 123 → **80**) :

- `pico8_struct_header_t` gagne `const pico8_qz_box_t *kq`, `const pico8_fixed_t *ql`, `*fg`, `*mt` et `pico8_fixed_t *ma` (mutable : `pico8_ma` écrit à travers). Les membres `get_kq`, `get_ql`, `get_fg`, `get_mt` de la table partent, avec sept implémentations (`fv_get_kq`/`_ql`/`_fg`, `ie_get_kq`, `pico8_jo_get_kq`, `js_get_kq`/`_fg`, `ui_get_ql`, `pico8_fr_get_mt`, `jc_get_mt`), leurs entrées et le prototype `planet.h`.
- Poses : `kq` = `&def->kq` pour `fv` (après `pico8_g__struct_init`), `ie`, l'arbre `jo` (`pico8_jo_struct_init`) ; **`js.kq` monte dans le noyau** : le membre part, `js:pq` écrit `self->header.kq = pico8_qz(…)` (un seul stockage, jamais libéré, comme avant) ; `ql` = `&def->ql` pour `fv` et `ui` ; `fg` = `&def->fg` pour `fv`, `self->has_fg ? &self->fg : NULL` pour `js` après ses clés ; `mt` = `&def->mt` pour `fr` (après `pico8_nr_struct_init`), et pour `jc` `NULL` au `_new` puis `self->has_mt ? &self->mt : NULL` dans `jc:hq`, juste après l'écriture de `has_mt` ; `ma` = `&self->ma` pour l'arbre `jo` et `qb`.
- Lecteurs : `pico8_gk` lit `e->kq` ; `ui:ev` lit `o_->ql` (sinon `self.ql`) ; `ui:gw` lit `o_->fg` (sinon 0) ; `jc_mt_of` lit `e->mt` ; `pico8_ma` lit et écrit `e->ma`, sous un `assert` de présence du lien. `entity_geom.c` n'emploie plus `pico8_qb_struct_class` ; il garde `pico8_jo_is_family`, `pico8_jo_t` (`pico8_na`) et `pico8_qb_t` (`pico8_gd`).
- TU : `test_pico8_jc.c` gagne `test_hq_takes_mt_from_its_start` (un `jc` tiré d'un `fr` porte `mt` et son lien, tiré d'un `jo` ni l'un ni l'autre ; G2) ; `test_pico8_js.c` lit `b->header.kq` ; `test_pico8_ui.c:324` et `test_pico8_jc.c:467-470` deviennent des épingles (`other->header.ql == &other->def->ql`, `jc->header.mt == NULL`) ; `test_pico8_struct_header.c` compare dix liens, construit huit porteurs de plus, le point souris et deux `jc` par `jc:hq` (réserves 1, 3, 4) et coupe le tcache de glibc dans sa relance (réserve 2).

**2.14d1 — `fx`, `p_`** (`accesseurs-q6` 80 → **32**, `lien-parent` 110 → **109**) :

- `pico8_struct_header_t` gagne `const pico8_fixed_t *fx` et `int *p_` (mutable : `d_` écrit à travers) ; `get_fx`, `get_p_` partent avec neuf implémentations, leurs entrées et quatre prototypes (`planet.h`, `probe.h`).
- `jo.fx` passe de `int` à `pico8_fixed_t` (0 ou 1.0 ; `*fx` 0 est nil, contrat du noyau) ; `has_fx` part (deux écritures à 1 deviennent `pico8_fixed_from_int(1)`, `planet.c:385`, `planet_trade.c:64`). Liens `fx` : `&def->fx` pour `g_`/`fv` (`pico8_g__struct_init` ; l'archétype vaut 1.0, la vérité de Lua), `&self->fx` pour l'arbre `jo`. Lecteur `mz:pq` (`wormhole.c:44`) : `p->fx != NULL && *p->fx != 0`.
- `has_p_` part des sept types (`jo`, `g_`, `fq`, `iv`, `mh`, `jc`, `mz`) avec ses 22 écritures ; lien `p_` = `&self->p_` posé par chaque constructeur. `probe_host_raise_p_` (`probe.c:280-309`) devient `assert(self->p_ != NULL …); *self->p_ = 1;` : plus de `pico8_g__of` (`probe.c:282` sort de `lien-parent`), de test de famille ni de cast. Lecteur `dg` (`bootstrap.c:70`) : `qc->p_ != NULL && *qc->p_`.
- TU : les plis `has_fx`/`has_p_` deviennent `(X->fx != 0)`/`(X->p_ != 0)`, les `assert(has_p_ && p_)` deviennent `assert(p_)` ; le faux `toast` de `test_pico8_iv.c` pose son lien après son init ; `test_pico8_mz.c:386-409` perd l'état « `fx` faux présent » ; **quatre constantes d'empreinte changent** (`test_pico8_fq.c` `MY_FALSE_SEQUENCE_HASH` retirée au profit de `BARE_SEQUENCE_HASH`, `test_pico8_mz.c` `EY_HASH` et `PR_HASH`, `test_pico8_ga.c` `LIFE_HASH`) ; `struct_header` compare `fx` et `p_`.

**2.14d2 — `m_`** (`accesseurs-q6` 32 → **0**, gardé ; `void-ptr` 8 → **7**) :

- `pico8_struct_header_t` gagne `int *m_` ; `set_m_` part de la table avec son commentaire (`struct.h:148-151` à `p`), ses six implémentations, leurs entrées et `pico8_jo_set_m_` (`planet.h`, la ligne `void-ptr`, avec le commentaire devenu orphelin au-dessus).
- `has_m_` part des six types (`bg`, `fq`, `fv`, `ie`, `jo`, `js`) ; lien `m_` = `&self->m_` posé par chaque constructeur (`fv` : `self->g_.header.m_`). `ui:gw` écrit `*gj->m_ = 0` et `*o_->m_ = 1` sous les deux `assert` gardés (le lien au lieu de `set_m_`) ; les trois lecteurs `has_m_ && m_` (`research_icon.c:178`, `ui_button.c:241`, `:265`) lisent `m_`.
- `accesseurs-q6` entre dans `COMPOSITION_RESIDUE_GUARD` (`Makefile`).
- TU : plis et `assert` comme en d1 ; les `m__state` à trois états (`bg`, `fv`, `js`, `ui`) rendent 0 ou 2 ; `giver` de `test_pico8_ui.c` reçoit `.m_ = &g_giver_m_` dans son initialiseur ; `test_pico8_ie.c:496-498` écrit par le lien ; **quatre constantes d'empreinte changent** (`test_pico8_fq.c` `HOVER_SEQUENCE_HASH`, `test_pico8_bg.c` `HOVER_HASH`, `test_pico8_fv.c` `HOVER_HASH`, `test_pico8_js.c` `JS_MEMBERS_HASH`) ; `struct_header` compare `m_`.

Rien d'autre : ni `glossaire-noms.md` (aucun nom minifié neuf), ni `trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `test_pico8_pj.c`, ni `particle.c`. Le `Makefile` en d2 seulement (la garde).

## User Stories

1. En tant que mainteneur, je veux que le code générique lise et écrive les huit derniers groupes de champs par un lien du noyau, afin que la table de classe ne porte plus aucun accesseur de champ et qu'`accesseurs-q6` soit gardé à 0.
2. En tant que gardien de la règle 1, je veux qu'un lien de champ A vise l'archétype, qu'un champ écrit par la classe vise l'instance, et que seuls `ma`, `p_`, `m_` — écrits à travers par `pico8_ma`, `d_`, `ui` — soient mutables.
3. En tant que gardien de la mémoire, je veux que le point souris de `jc:hq` ait enfin un test qui rougit s'il oublie l'init du noyau.
4. En tant que gardien de la règle du parent, je veux qu'aucune mutation rouge au parent ne devienne verte à l'enfant.
5. En tant que gardien des empreintes, je veux que toute constante de TU qui change soit nommée, justifiée et reproduite au parent par le seul changement du pli, avant qu'on la change.
6. En tant qu'auteur de 2.15, je veux un noyau à quatorze membres (onze liens), une init qui met déjà à NULL les liens `emitter`/`probe` que j'ajouterai, et un `assert_links` où un lien de plus est une ligne.

## Implementation Decisions

### Modules

- **c** : `struct.h` ; `entity_geom.c`/`.h`, `geom.h` (commentaire de `pico8_qz`) ; porteurs `help` (`fv`), `research_icon` (`ie`), `planet` (arbre `jo`), `planet_family`, `planet_family_trade` (`fr`), `scroll` (`qb`), `ui` (`ui`), `ui_button` (`js`), `slipway` (`jc`) ; en-têtes `planet.h`, `ui_button.h` ; TU `jc`, `js`, `ui`, `struct_header`.
- **d1** : `struct.h` ; `bootstrap.c`, `wormhole.c`/`.h`, `probe.c`/`.h`, `planet.c`/`.h`, `planet_trade.c`, `planet_family.c`, `planet_family_trade.c`, `help.c`, `msgbox.c`/`.h`, `slipway.c`/`.h`, `ui_button.c`/`.h` ; TU `fq`, `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `iv`, `jc`, `jo`, `mh`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh`, `struct_header`.
- **d2** : `Makefile` ; `struct.h` ; `bg.c`/`.h`, `help.c`/`.h`, `planet.c`/`.h`, `planet_family.c`, `planet_family_trade.c`, `research_icon.c`/`.h`, `ui.c`, `ui_button.c`/`.h` ; TU `bg`, `fq`, `fr`, `fv`, `ga`, `hj`, `ie`, `jo`, `js`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `struct_header`, `ui`.

### Inclusions

Aucune ligne `#include` ajoutée ni retirée dans `src/`, aux trois commits (seul `test_pico8_struct_header.c` inclut, en c, six en-têtes de plus : `help`, `planet`, `planet_family_trade`, `scroll`, `slipway`, `ui`, pour les porteurs qu'il construit) : `inclusion-mutuelle` **16 / 15, 40 lignes** partout ; graphe d'usage **19** partout. En c, l'arête `entity_geom` → `scroll` perd `pico8_qb_struct_class` et ne garde que `pico8_qb_t` (`pico8_gd`) ; `entity_geom` → `planet` garde `pico8_jo_is_family`, `pico8_jo_t` (`pico8_na`) : exactement ce que l'étude 2.11 (fiches 3 et c3) laisse à 2.17e. En d1, `probe.c` n'emploie plus `pico8_jc_struct_class`/`pico8_mz_struct_class` dans `probe_host_raise_p_`, mais les emploie ailleurs : aucune arête ne sort (usage 19). `particle.c:12` reste (G6 du PRD 2.14ab, contrat pour 2.17).


### Ce qui change

Les trois diffs ci-dessous sont le texte exact (`git diff p c`, `git diff c d1`, `git diff d1 d2`), `git apply --check` puis `git apply` sur `p` rendant `d2` à l'identique (Runs 17). Lecture, en plus de la Solution :

- **Un lien, pas une copie** : aucun lien ne copie une valeur dans le noyau. `js.kq` n'est pas copié mais **déplacé** : son seul écrivain (`js:pq`) écrit le noyau, et ses lecteurs (`pico8_gk`, quatre sites de `test_pico8_js.c`) le lisent là.
- **`mt` de `jc` suit `has_mt`** : `has_mt` n'est écrit qu'au `_new` (0) et dans `jc:hq` ; le lien est reposé à ces deux endroits, jamais ailleurs. Sur l'arbre `jo`, seul `fr` porte `mt` (les neuf autres sortes ont un `def->mt` que le cart ne lit pas : leur lien reste NULL).
- **`ma`** : `pico8_ma(p, e)` garde son `assert`, sur la présence du lien (l'ancien portait sur la famille ou la classe `qb`) ; c'est ce que la fonction suit. Un lien `&self->def->ma` ne compile pas (`-Werror=discarded-qualifiers`, preuve 2).
- **`fx`** : l'archétype de `g_` et de `fv` vaut 1.0 ; `*fx != 0` rend donc la vérité que `pico8_g__get_fx` rendait en constante. Pour `jo`, 0 est nil (le `_new` pose 0, chaque écriture pose 1.0) : c'est le contrat « `*fx` 0 is nil » du noyau.
- **`p_`** : `probe_host_raise_p_` n'aiguille plus ; son `assert` porte sur la présence du lien. Il admet désormais tout porteur de `p_` (`fq`, `iv`, `mh` en plus des quatre hôtes de `d_`), ce qui est plus large que l'ancien : aucun `ey` de ces trois-là n'appelle `d_` (G4).
- **`m_`** : `ui:gw` garde ses deux `assert` (le cart écrit `m_` sur n'importe quelle table, le port refuse un non-porteur) ; un non-porteur survolé casse toujours le jeu, comme avant.
- **Nil et faux** : sans `has_p_`/`has_m_`, un `p_` ou `m_` faux et un nil sont le même état (0). Le cart ne les lit qu'en vérité (`qc.p_ then`, `self.m_ then`, `self.m_ and 12 or 13` : grep de `pico8/lua.lua`, preuve 7) : aucune valeur du jeu ne change (e2e 26, programme de la preuve 5). Les TU qui pliaient `has_X` à part voient l'état « faux présent » devenir nil : c'est la seule cause des huit constantes changées (preuve 6).
- **Commentaires** : celui du noyau dit quels liens sont écrits à travers et que `*fx` 0 est nil ; celui de `pico8_ma` (`entity_geom.h`) dit son contrat ; celui de `pico8_qz` (`geom.h`) nomme le lien au lieu du membre ; le commentaire de `set_m_` part avec lui (plan : « commentaire `struct.h:139-141` », `:148-151` à `p`), comme celui de `planet.h` qui annonçait les accesseurs de `jo`.

#### Diff de 2.14c (`git diff p c`)

```diff
diff --git a/src/pico8_entity_geom.c b/src/pico8_entity_geom.c
index 2f63197..d0d58c0 100644
--- a/src/pico8_entity_geom.c
+++ b/src/pico8_entity_geom.c
@@ -34,7 +34,7 @@ int pico8_gk(const pico8_struct_header_t *e, const pico8_vec_t *mp, const pico8_
 
     chosen = (fa >= pico8_fixed_from_int(10)) ? rp : mp;
     point = pico8_vec_sub(chosen != NULL ? *chosen : pico8_entity_geom_zero(), ra);
-    kq = e != NULL && e->cls->get_kq != NULL ? e->cls->get_kq(e) : NULL;
+    kq = e != NULL ? e->kq : NULL;
     if (kq != NULL) {
         return pico8_qz_call(kq, point);
     }
@@ -71,9 +71,8 @@ pico8_vec_t pico8_ma(pico8_vec_t p, pico8_struct_header_t *e) {
     pico8_fixed_t t = pico8_fixed_from_int(0);
 
     if (e != NULL) {
-        assert((pico8_jo_is_family(e) || e->cls == pico8_qb_struct_class()) &&
-               "ma: e is a planet (jo:kj) or the camera (qb:ph)");
-        ma = pico8_jo_is_family(e) ? &((pico8_jo_t *)e)->ma : &((pico8_qb_t *)e)->ma;
+        assert(e->ma != NULL && "ma: e is a planet (jo:kj) or the camera (qb:ph)");
+        ma = e->ma;
     }
     decremented = pico8_fixed_sub(ma != NULL ? *ma : pico8_fixed_from_int(0), PICO8_FIXED_LIT(0x00001999, 0.1));
     clamped = (decremented > 0) ? decremented : 0;
diff --git a/src/pico8_entity_geom.h b/src/pico8_entity_geom.h
index 76feb92..b758301 100644
--- a/src/pico8_entity_geom.h
+++ b/src/pico8_entity_geom.h
@@ -22,6 +22,8 @@ int pico8_gd(const pico8_struct_header_t *e);
  * bob. Rebinding the field to a fresh vector, as `qb:kj` does, would not. */
 void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t);
 
+/* `e` is NULL or carries `ma` (asserted): a planet or the camera. Decays
+ * `e.ma` in place. */
 pico8_vec_t pico8_ma(pico8_vec_t p, pico8_struct_header_t *e);
 
 /* Reads every element of `j_` as an entity instance. `*out_nearest` is
diff --git a/src/pico8_geom.h b/src/pico8_geom.h
index 69e88e7..6fb614d 100644
--- a/src/pico8_geom.h
+++ b/src/pico8_geom.h
@@ -34,7 +34,7 @@ typedef struct {
 
 /* C has no closures, so the predicate the cart returns becomes this box
  * plus pico8_qz_call. The box is heap-allocated and belongs to the caller,
- * which parks it in a member (js.kq) and never frees it. */
+ * which parks it in a link (`js`'s `header.kq`) and never frees it. */
 pico8_qz_box_t *pico8_qz(pico8_fixed_t xl, pico8_fixed_t yt, pico8_fixed_t xr, pico8_fixed_t yb);
 
 /* True iff p is inside [xl,xr]x[yt,yb], by mid()'s clamp-then-compare
diff --git a/src/pico8_help.c b/src/pico8_help.c
index ddbe946..d46fd0a 100644
--- a/src/pico8_help.c
+++ b/src/pico8_help.c
@@ -100,20 +100,6 @@ void pico8_fv_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_fv_t *)self, p);
 }
 
-static const pico8_qz_box_t *fv_get_kq(const void *self) {
-    return &((const pico8_fv_t *)self)->g_.def->kq;
-}
-
-static int fv_get_ql(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_fv_t *)self)->g_.def->ql;
-    return 1;
-}
-
-static int fv_get_fg(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_fv_t *)self)->g_.def->fg;
-    return 1;
-}
-
 static void fv_set_m_(void *self, int m_) {
     pico8_fv_t *fv = self;
 
@@ -132,9 +118,6 @@ static const pico8_struct_class_t g_fv_struct_class = {
     .emitter_of = pico8_g__emitter_of,
     .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = fv_get_kq,
-    .get_ql = fv_get_ql,
-    .get_fg = fv_get_fg,
     .get_fx = pico8_g__get_fx,
     .get_p_ = pico8_g__get_p_,
     .set_m_ = fv_set_m_,
@@ -146,6 +129,9 @@ static pico8_fv_t *pico8_fv_struct_new(void) {
     assert(g_fv_up && "pico8_help_init() must run first");
     self = malloc(sizeof *self);
     pico8_g__struct_init(&self->g_, &g_fv_struct_class, &pico8_fv_def);
+    self->g_.header.kq = &self->g_.def->kq;
+    self->g_.header.ql = &self->g_.def->ql;
+    self->g_.header.fg = &self->g_.def->fg;
     self->m_ = 0;
     self->has_m_ = 0;
 #ifdef PICO8_CLASS_COVERAGE
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 5f1730c..b51d2bf 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -76,10 +76,6 @@ void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
     pico8_jo_ll((pico8_jo_t *)self, t);
 }
 
-const pico8_qz_box_t *pico8_jo_get_kq(const void *self) {
-    return &((const pico8_jo_t *)self)->def->kq;
-}
-
 int pico8_jo_get_fx(const void *self) {
     const pico8_jo_t *jo = self;
 
@@ -110,7 +106,6 @@ static const pico8_struct_class_t g_jo_struct_class = {
     .cn = pico8_jo_cn_dispatch,
     .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
@@ -124,6 +119,8 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     /* Read-only: the planet modules write through their members. */
     self->header.ra = (const pico8_vec_t *const *)&self->ra;
     self->header.p = (const pico8_vec_t *const *)&self->p;
+    self->header.kq = &self->def->kq;
+    self->header.ma = &self->ma;
     self->ma = self->def->ma;
     self->kw = &g_jo_kw_shared;
     self->id = NULL;
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index 53799e2..c4b893f 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -102,7 +102,6 @@ void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw);
 
 /* The field accessors jo's descriptor holds, for a daughter's `static const`
  * descriptor to name. */
-const pico8_qz_box_t *pico8_jo_get_kq(const void *self);
 int pico8_jo_get_fx(const void *self);
 int pico8_jo_get_p_(const void *self);
 void pico8_jo_set_m_(void *self, int m_);
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index 581d882..9f2cbf2 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -50,7 +50,6 @@ static const pico8_struct_class_t g_qh_struct_class = {
     .parent = pico8_jo_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
@@ -124,7 +123,6 @@ static const pico8_struct_class_t g_nl_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
@@ -172,7 +170,6 @@ static const pico8_struct_class_t g_os_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
@@ -240,7 +237,6 @@ static const pico8_struct_class_t g_hj_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
@@ -331,7 +327,6 @@ static const pico8_struct_class_t g_pm_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
@@ -485,7 +480,6 @@ static const pico8_struct_class_t g_ga_struct_class = {
     .emitter_of = ga_emitter_of,
     /* ga's cart list of tags is empty and replaces jo's: never jo's. */
     .tags = 0,
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index 6695c36..8a45543 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -178,7 +178,6 @@ static const pico8_struct_class_t g_nr_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
@@ -216,11 +215,6 @@ static pico8_nr_t *pico8_nr_struct_new(const pico8_jo_keys_t *keys) {
     return self;
 }
 
-static int pico8_fr_get_mt(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_fr_t *)self)->nr.qh.jo.def->mt;
-    return 1;
-}
-
 static const pico8_struct_class_t g_fr_struct_class = {
     .kj = pico8_jo_kj_dispatch,
     .ev = pico8_dl_dispatch,
@@ -241,10 +235,8 @@ static const pico8_struct_class_t g_fr_struct_class = {
     .parent = pico8_nr_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
-    .get_mt = pico8_fr_get_mt,
     .set_m_ = pico8_jo_set_m_,
 };
 
@@ -253,6 +245,7 @@ static pico8_fr_t *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
     pico8_nr_struct_init(&self->nr, &g_fr_struct_class, &pico8_fr_def);
+    self->nr.qh.jo.header.mt = &self->nr.qh.jo.def->mt;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "fr: the only construction key is id (ie:ldown); another key has no member "
@@ -340,7 +333,6 @@ static const pico8_struct_class_t g_mx_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
diff --git a/src/pico8_research_icon.c b/src/pico8_research_icon.c
index 503ad8b..8d365a7 100644
--- a/src/pico8_research_icon.c
+++ b/src/pico8_research_icon.c
@@ -202,10 +202,6 @@ static int pico8_ie_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_v
     return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
 }
 
-static const pico8_qz_box_t *ie_get_kq(const void *self) {
-    return &((const pico8_ie_t *)self)->def->kq;
-}
-
 static void ie_set_m_(void *self, int m_) {
     pico8_ie_t *ie = self;
 
@@ -218,7 +214,6 @@ static const pico8_struct_class_t g_ie_struct_class = {
     .hd = pico8_ie_hd_dispatch,
     .hb = pico8_ie_hb_dispatch,
     .ldown = pico8_ie_ldown_dispatch,
-    .get_kq = ie_get_kq,
     .set_m_ = ie_set_m_,
 };
 
@@ -230,6 +225,7 @@ static pico8_ie_t *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
     self->def = &pico8_ie_def;
     self->header.fa = &self->def->fa;
     self->header.ra = &self->ra;
+    self->header.kq = &self->def->kq;
     self->ra = &self->def->ra;
     self->ke = NULL;
     self->jb = NULL;
diff --git a/src/pico8_scroll.c b/src/pico8_scroll.c
index 96f90dc..6a7f9e9 100644
--- a/src/pico8_scroll.c
+++ b/src/pico8_scroll.c
@@ -38,6 +38,7 @@ static pico8_qb_t *pico8_qb_struct_new(void) {
     self->def = &pico8_qb_def;
     self->header.ra = &self->ra;
     self->header.p = &self->p;
+    self->header.ma = &self->ma;
     self->v = &self->def->v;
     self->ma = self->def->ma;
     self->p = &self->def->p;
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index e3bb528..9815a4b 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -43,7 +43,11 @@ pico8_vec_t pico8_jc_end_ra(const pico8_struct_header_t *end) {
 }
 
 static int jc_mt_of(const pico8_struct_header_t *e, pico8_fixed_t *out) {
-    return e != NULL && e->cls->get_mt != NULL && e->cls->get_mt(e, out);
+    if (e == NULL || e->mt == NULL) {
+        return 0;
+    }
+    *out = *e->mt;
+    return 1;
 }
 
 void pico8_jc_hg_add(pico8_jc_hg_t *hg, const char *id) {
@@ -120,6 +124,7 @@ void pico8_jc_hq(pico8_jc_t *self, pico8_vec_t *mp) {
     /* A fixed is always true in Lua: `qu.mt or re.mt` takes the first present. */
     self->mt = pico8_fixed_from_int(0);
     self->has_mt = jc_mt_of(self->qu, &self->mt) || jc_mt_of(self->re, &self->mt);
+    self->header.mt = self->has_mt ? &self->mt : NULL;
 
     self->probe->et = pico8_jc_ce(self);
     if (self->probe->et != NULL) {
@@ -338,16 +343,6 @@ static pico8_probe_t *jc_probe_of(void *self) {
     return ((pico8_jc_t *)self)->probe;
 }
 
-static int jc_get_mt(const void *self, pico8_fixed_t *out) {
-    const pico8_jc_t *jc = self;
-
-    if (!jc->has_mt) {
-        return 0;
-    }
-    *out = jc->mt;
-    return 1;
-}
-
 static int jc_get_p_(const void *self) {
     const pico8_jc_t *jc = self;
 
@@ -361,7 +356,6 @@ static const pico8_struct_class_t g_jc_struct_class = {
     .ey = pico8_jc_ey_dispatch,
     .probe_of = jc_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JC),
-    .get_mt = jc_get_mt,
     .get_p_ = jc_get_p_,
 };
 
@@ -383,6 +377,7 @@ static pico8_jc_t *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
     self->probe->has_od = self->def->probe.has_od;
     self->mt = 0;
     self->has_mt = 0;
+    self->header.mt = NULL;
     self->probe->et = NULL;
     self->fo = NULL;
     self->p_ = 0;
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 5332c38..d31fd00 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -78,7 +78,8 @@ typedef struct pico8_struct_class pico8_struct_class_t;
 /* First member of every entity struct, and of the parent struct a child
  * embeds first. `me` and `t` are ke's (`me` NULL is nil). A component link
  * points at the field's storage, the archetype's for a field the class
- * never writes; NULL: the class has no such field. */
+ * never writes; NULL: the class has no such field, or it is nil. `ma` is
+ * written through, by `pico8_ma`. */
 typedef struct {
     const pico8_struct_class_t *cls;
     const char *me;
@@ -86,6 +87,11 @@ typedef struct {
     const pico8_fixed_t *fa;
     const pico8_vec_t *const *ra;
     const pico8_vec_t *const *p;
+    const pico8_qz_box_t *kq;
+    const pico8_fixed_t *ql;
+    const pico8_fixed_t *fg;
+    const pico8_fixed_t *mt;
+    pico8_fixed_t *ma;
 } pico8_struct_header_t;
 
 /* Every constructor calls it first: its `malloc` leaves the header
@@ -145,14 +151,9 @@ struct pico8_struct_class {
     const pico8_struct_class_t *(*parent)(void);
     struct pico8_emitter *(*emitter_of)(void *self);
     struct pico8_probe *(*probe_of)(void *self);
-    /* A NULL member: the class has no such field, for every instance. A
-     * reader returns NULL, or 0 leaving *out untouched, when the field is
-     * nil; `get_fx` and `get_p_` return its Lua truth. `set_m_` also marks
-     * `m_` present. Each is only ever called as `e->cls->get_X(e, ...)`. */
-    const pico8_qz_box_t *(*get_kq)(const void *self);
-    int (*get_ql)(const void *self, pico8_fixed_t *out);
-    int (*get_fg)(const void *self, pico8_fixed_t *out);
-    int (*get_mt)(const void *self, pico8_fixed_t *out);
+    /* A NULL member: the class has no such field, for every instance.
+     * `get_fx` and `get_p_` return its Lua truth. `set_m_` also marks `m_`
+     * present. Each is only ever called as `e->cls->get_X(e, ...)`. */
     int (*get_fx)(const void *self);
     int (*get_p_)(const void *self);
     void (*set_m_)(void *self, int m_);
diff --git a/src/pico8_ui.c b/src/pico8_ui.c
index c81694d..539be6a 100644
--- a/src/pico8_ui.c
+++ b/src/pico8_ui.c
@@ -74,14 +74,8 @@ static void pico8_ui_ev_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_ui_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_ui_t *)self);
 }
 
-static int ui_get_ql(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_ui_t *)self)->def->ql;
-    return 1;
-}
-
 static const pico8_struct_class_t g_ui_struct_class = {
     .ev = pico8_ui_ev_dispatch,
-    .get_ql = ui_get_ql,
 };
 
 static pico8_ui_t *pico8_ui_struct_new(void) {
@@ -93,6 +87,7 @@ static pico8_ui_t *pico8_ui_struct_new(void) {
     pico8_ke_struct_init(&self->header, &g_ui_struct_class, pico8_ui_def.me, pico8_ui_def.t);
     self->def = &pico8_ui_def;
     self->header.fa = &self->def->fa;
+    self->header.ql = &self->def->ql;
     self->le = &g_shared_le;
     self->qa = NULL;
     self->ic = NULL;
@@ -237,12 +232,7 @@ void pico8_ui_ev(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_ui_t *sel
     if (self->rheld != NULL) {
         spr_id = 9;
     } else if (self->o_ != NULL) {
-        pico8_fixed_t ql = self->def->ql;
-
-        if (self->o_->cls->get_ql != NULL) {
-            self->o_->cls->get_ql(self->o_, &ql);
-        }
-        spr_id = pico8_ui_to_int(ql);
+        spr_id = pico8_ui_to_int(self->o_->ql != NULL ? *self->o_->ql : self->def->ql);
     } else {
         spr_id = pico8_ui_to_int(self->def->ql);
     }
@@ -307,12 +297,7 @@ void pico8_ui_gw(pico8_mem_t *mem, pico8_ui_t *self) {
     }
 
     if (self->o_ != NULL) {
-        pico8_fixed_t fg = pico8_fixed_from_int(0);
-
-        if (self->o_->cls->get_fg != NULL) {
-            self->o_->cls->get_fg(self->o_, &fg);
-        }
-        poke_val = pico8_ui_to_int(fg);
+        poke_val = pico8_ui_to_int(self->o_->fg != NULL ? *self->o_->fg : pico8_fixed_from_int(0));
     } else {
         int prev = pico8_peek(mem, 0x5f80);
         poke_val = (prev - 1 > 0) ? prev - 1 : 0;
diff --git a/src/pico8_ui_button.c b/src/pico8_ui_button.c
index f009977..9dbbef3 100644
--- a/src/pico8_ui_button.c
+++ b/src/pico8_ui_button.c
@@ -58,20 +58,6 @@ static void pico8_js_hb_dispatch(void *self) {
     }
 }
 
-static const pico8_qz_box_t *js_get_kq(const void *self) {
-    return ((const pico8_js_t *)self)->kq;
-}
-
-static int js_get_fg(const void *self, pico8_fixed_t *out) {
-    const pico8_js_t *js = self;
-
-    if (!js->has_fg) {
-        return 0;
-    }
-    *out = js->fg;
-    return 1;
-}
-
 static void js_set_m_(void *self, int m_) {
     pico8_js_t *js = self;
 
@@ -87,8 +73,6 @@ static const pico8_struct_class_t g_js_struct_class = {
     .has_hb = pico8_js_has_hb,
     .ldown = pico8_js_ldown_dispatch,
     .states = {[PICO8_STATE_PH] = pico8_js_ph_dispatch},
-    .get_kq = js_get_kq,
-    .get_fg = js_get_fg,
     .set_m_ = js_set_m_,
 };
 
@@ -98,7 +82,6 @@ static pico8_js_t *pico8_js_struct_new(const pico8_js_keys_t *keys) {
     pico8_ke_struct_init(&self->header, &g_js_struct_class, pico8_js_def.me, pico8_js_def.t);
     self->lq = NULL;
     self->mb = 0;
-    self->kq = NULL;
     self->h_ = NULL;
     self->has_h_ = 0;
     self->jb = NULL;
@@ -147,6 +130,7 @@ static pico8_js_t *pico8_js_struct_new(const pico8_js_keys_t *keys) {
         self->_jo_fe_od = keys->_jo_fe_od;
         self->has__jo_fe_od = keys->has__jo_fe_od;
     }
+    self->header.fg = self->has_fg ? &self->fg : NULL;
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("js");
 #endif
@@ -200,7 +184,7 @@ void pico8_js_pq(pico8_js_t *self, const pico8_js_keys_t *keys) {
 
     self->lq = pico8_js_lq(keys->lq_copy, keys->lq_row);
     self->mb = pico8_fixed_add(self->lq->w, pico8_fixed_from_int(2));
-    self->kq = pico8_qz(pico8_fixed_from_int(0), pico8_fixed_from_int(-1), self->mb, pico8_fixed_from_int(11));
+    self->header.kq = pico8_qz(pico8_fixed_from_int(0), pico8_fixed_from_int(-1), self->mb, pico8_fixed_from_int(11));
 
     /* TROU-23 */
     if (keys->h_row != NULL || (keys->has_h_ && !keys->h_is_false)) {
diff --git a/src/pico8_ui_button.h b/src/pico8_ui_button.h
index df549bf..057aa6a 100644
--- a/src/pico8_ui_button.h
+++ b/src/pico8_ui_button.h
@@ -31,7 +31,6 @@ struct pico8_js {
     pico8_fixed_t fa;
     pico8_lq_t *lq;
     pico8_fixed_t mb;
-    pico8_qz_box_t *kq;
     pico8_lq_t *h_;
     int has_h_; /* 0: nil; 1: present, and a NULL `h_` is `false` */
     pico8_jb_t *jb;
diff --git a/tests/test_pico8_jc.c b/tests/test_pico8_jc.c
index 74875b8..f99de76 100644
--- a/tests/test_pico8_jc.c
+++ b/tests/test_pico8_jc.c
@@ -435,8 +435,6 @@ static void *first_with_me(const char *tag, const char *me) {
  * `pq` made; registered like any `ke` entity with `kj` and `ev`. */
 static void check_fresh_jc(const void *e, const void *qu) {
     const pico8_jc_t *jc = as_jc(e);
-    pico8_jc_t with_mt = *jc;
-    pico8_fixed_t mt = 0;
     const pico8_int_seq_t *k_;
     const pico8_jc_hg_t *hg;
     size_t i;
@@ -464,10 +462,7 @@ static void check_fresh_jc(const void *e, const void *qu) {
     assert(hg == &jc->hg_own && hg->count == 0);
 
     assert(jc->re == NULL && !jc->probe->has_od && jc->probe->od == 0 && !jc->has_mt && jc->probe->et == NULL);
-    assert(!cls_of(jc)->get_mt(jc, &mt) && mt == 0);
-    with_mt.mt = pico8_fixed_from_int(3);
-    with_mt.has_mt = 1;
-    assert(cls_of(&with_mt)->get_mt(&with_mt, &mt) && mt == with_mt.mt);
+    assert(jc->header.mt == NULL);
     assert(jc->fo == NULL && !jc->has_p_);
     assert(cls_of(jc)->hq != NULL && cls_of(jc)->ey != NULL);
     assert(cls_of(jc)->ev != NULL && cls_of(jc)->kj != NULL);
@@ -572,6 +567,20 @@ static void test_hq(void) {
     check("hq", h, HQ_HASH);
 }
 
+static void test_hq_takes_mt_from_its_start(void) {
+    void *from_jo;
+    void *from_fr;
+
+    world();
+    from_jo = build_jc(pico8_jo_new(NULL));
+    hq_over(from_jo, NULL, vec_ref(-640, -640));
+    assert(!as_jc(from_jo)->has_mt && as_jc(from_jo)->header.mt == NULL);
+    from_fr = build_jc(&pico8_fr_new(NULL)->nr.qh.jo);
+    hq_over(from_fr, NULL, vec_ref(-640, -640));
+    assert(as_jc(from_fr)->has_mt && as_jc(from_fr)->mt == pico8_fr_def.mt);
+    assert(as_jc(from_fr)->header.mt == &as_jc(from_fr)->mt);
+}
+
 /* `A and B or C`: a falsy `re` falls back to `qu`. */
 static void test_eb(void) {
     uint32_t h = 2166136261u;
@@ -1098,6 +1107,7 @@ int main(void) {
     in_fresh_child(test_construction);
     in_fresh_child(test_ey);
     in_fresh_child(test_hq);
+    in_fresh_child(test_hq_takes_mt_from_its_start);
     in_fresh_child(test_eb);
     in_fresh_child(test_draw);
     in_fresh_child(test_bv);
diff --git a/tests/test_pico8_js.c b/tests/test_pico8_js.c
index efae9e1..075aba4 100644
--- a/tests/test_pico8_js.c
+++ b/tests/test_pico8_js.c
@@ -303,7 +303,7 @@ static uint32_t fold_built_h_(uint32_t h, const pico8_js_t *b) {
 
 static uint32_t fold_kq(uint32_t h, const pico8_js_t *b) {
     static const int dy[] = {-2, -1, 0, 5, 9, 10, 11};
-    const pico8_qz_box_t *kq = b->kq;
+    const pico8_qz_box_t *kq = b->header.kq;
     int mb = pico8_fixed_flr_int(b->mb);
     int dx[6];
     size_t i;
@@ -1134,7 +1134,7 @@ static uint32_t fold_js_members(uint32_t h, const void *e) {
     h = fold_member_fixed(h, 1, b->fa);
     h = fold_member_opt_ref(h, b->lq);
     h = fold_member_fixed(h, 1, b->mb);
-    h = b->kq != NULL ? fold_kq(fold_kind(h, REF_KIND), b) : fnv(h, 0u);
+    h = b->header.kq != NULL ? fold_kq(fold_kind(h, REF_KIND), b) : fnv(h, 0u);
     if (b->has_h_ && b->h_ == NULL) {
         h = fold_bool(fold_kind(h, BOOL_KIND), 0);
     } else {
@@ -1211,7 +1211,7 @@ static void write_m_(pico8_js_t *b, int present, int value) {
  * the typed reads. */
 static uint32_t fold_written(uint32_t h, void *e, void *other) {
     pico8_js_t *b = js_of(e);
-    const pico8_qz_box_t *kq_before = b->kq;
+    const pico8_qz_box_t *kq_before = b->header.kq;
     pico8_fixed_t mb = b->mb;
     pico8_lq_t *row;
     int i;
@@ -1245,7 +1245,7 @@ static uint32_t fold_written(uint32_t h, void *e, void *other) {
     b->lq = row;
     h = fold_js_members(h, e);
     b->lq = NULL;
-    assert(b->kq == kq_before);
+    assert(b->header.kq == kq_before);
     assert(b->mb == mb);
     h = fold_js_members(h, e);
     b->lq = row;
@@ -1395,7 +1395,7 @@ static uint32_t members_title_and_game(void) {
         assert(lq != NULL && is_lq(lq) && row_count(lq) == 0);
         assert(b->fa == pico8_fixed_from_int(11));
         assert(b->mb == pico8_fixed_add(lq->w, pico8_fixed_from_int(2)));
-        assert(b->kq != NULL);
+        assert(b->header.kq != NULL);
         assert(!b->has_h_ && b->jb == NULL && b->ra == NULL && b->qf == NULL && !b->has_m_);
         assert(!b->has_fg && b->px == NULL && !b->has_trade && !b->has_en && b->_cr_t == NULL);
         assert(b->_jo_fe_pt == NULL && b->_jo_fe_b == NULL && !b->has__jo_fe_od);
diff --git a/tests/test_pico8_struct_header.c b/tests/test_pico8_struct_header.c
index a64ada1..8f9ece5 100644
--- a/tests/test_pico8_struct_header.c
+++ b/tests/test_pico8_struct_header.c
@@ -1,7 +1,8 @@
 /* The kernel's component links on entities whose constructor `malloc`s
- * without zeroing. The binary runs itself again with every `malloc`
- * filling its block (MALLOC_PERTURB_ on glibc, MallocScribble on macOS),
- * so a link no init set is not NULL. */
+ * without zeroing. The binary runs itself again with `malloc` filling every
+ * block it returns (MallocScribble on macOS; MALLOC_PERTURB_ on glibc, whose
+ * tcache hands a freed block back unfilled, hence the tcache turned off), so
+ * a link no init set is not NULL. */
 
 #define _POSIX_C_SOURCE 200809L
 
@@ -17,13 +18,19 @@
 #include "../src/pico8_dispatch_context.h"
 #include "../src/pico8_economy.h"
 #include "../src/pico8_gfx.h"
+#include "../src/pico8_help.h"
 #include "../src/pico8_input.h"
 #include "../src/pico8_layout.h"
 #include "../src/pico8_mem.h"
 #include "../src/pico8_mouse_reader.h"
+#include "../src/pico8_planet.h"
+#include "../src/pico8_planet_family_trade.h"
 #include "../src/pico8_research.h"
 #include "../src/pico8_research_icon.h"
+#include "../src/pico8_scroll.h"
+#include "../src/pico8_slipway.h"
 #include "../src/pico8_trade.h"
+#include "../src/pico8_ui.h"
 #include "../src/pico8_ui_button.h"
 
 static pico8_mem_t mem;
@@ -35,6 +42,7 @@ static void rerun_with_malloc_filling(char **argv) {
         return;
     }
     setenv("MALLOC_PERTURB_", "90", 1);
+    setenv("GLIBC_TUNABLES", "glibc.malloc.tcache_count=0", 1);
     setenv("MallocScribble", "1", 1);
     setenv("PICO8_MALLOC_FILLS", "1", 1);
     execv(argv[0], argv);
@@ -42,13 +50,17 @@ static void rerun_with_malloc_filling(char **argv) {
     exit(1);
 }
 
-/* Reads a fresh block on purpose: through `volatile`, which the compiler
+/* Reads a recycled block on purpose: through `volatile`, which the compiler
  * does not flag as uninitialized. */
 static void assert_malloc_fills(void) {
-    void *block = malloc(64);
-    const volatile unsigned char *bytes = block;
+    void *block = calloc(1, 64);
+    const volatile unsigned char *bytes;
     int i;
 
+    assert(block != NULL);
+    free(block);
+    block = malloc(64);
+    bytes = block;
     assert(block != NULL);
     for (i = 0; i < 64; i++) {
         assert(bytes[i] != 0 && bytes[i] == bytes[0] && "malloc must fill its block, or this test proves nothing");
@@ -56,28 +68,34 @@ static void assert_malloc_fills(void) {
     free(block);
 }
 
-/* Each link: the one a class carries, or NULL. */
-static void assert_links(const pico8_struct_header_t *h, const pico8_fixed_t *fa, const pico8_vec_t *const *ra,
-                         const pico8_vec_t *const *p) {
-    assert(h->fa == fa);
-    assert(h->ra == ra);
-    assert(h->p == p);
+/* Each link of `want`: the one a class carries, or NULL; its `cls`, `me`
+ * and `t` are not compared. */
+static void assert_links(const pico8_struct_header_t *h, pico8_struct_header_t want) {
+    assert(h->fa == want.fa);
+    assert(h->ra == want.ra);
+    assert(h->p == want.p);
+    assert(h->kq == want.kq);
+    assert(h->ql == want.ql);
+    assert(h->fg == want.fg);
+    assert(h->mt == want.mt);
+    assert(h->ma == want.ma);
 }
 
-/* Non-carriers, one per constructor that does not go through ke. */
+/* Non-carriers, one per constructor that does not go through ke; the mouse
+ * point, which `jc:hq` builds, has its own test below. */
 static void test_non_carriers_have_null_links(void) {
     pico8_ie_t *ie = pico8_ie_new(NULL);
     pico8_ie_ke_t view = pico8_ie_ke_view(ie);
     pico8_nf_t *nf = pico8_nf_new();
     pico8_lq_t *lq = pico8_lq_new(NULL);
 
-    assert_links(&nf->header, NULL, (const pico8_vec_t *const *)&nf->ra, NULL);
-    assert_links(&lq->header, NULL, &lq->ra, NULL);
-    assert_links(&pico8_jb_new(NULL)->header, NULL, NULL, NULL);
-    assert_links(&pico8_hp_new(NULL)->header, NULL, NULL, NULL);
-    assert_links(&pico8_hy_new()->header, NULL, NULL, NULL);
-    assert_links(&pico8_trade_new(NULL)->header, NULL, NULL, NULL);
-    assert_links(&view.header, NULL, NULL, NULL);
+    assert_links(&nf->header, (pico8_struct_header_t){.ra = (const pico8_vec_t *const *)&nf->ra});
+    assert_links(&lq->header, (pico8_struct_header_t){.ra = &lq->ra});
+    assert_links(&pico8_jb_new(NULL)->header, (pico8_struct_header_t){0});
+    assert_links(&pico8_hp_new(NULL)->header, (pico8_struct_header_t){0});
+    assert_links(&pico8_hy_new()->header, (pico8_struct_header_t){0});
+    assert_links(&pico8_trade_new(NULL)->header, (pico8_struct_header_t){0});
+    assert_links(&view.header, (pico8_struct_header_t){0});
 }
 
 /* Carriers: the archetype's field for a class that never writes it, the
@@ -86,10 +104,68 @@ static void test_carriers_link_their_storage(void) {
     pico8_bg_t *bg = pico8_bg_new();
     pico8_js_t *js = pico8_js_new(NULL);
     pico8_ie_t *ie = pico8_ie_new(NULL);
+    pico8_js_keys_t with_fg = {.has_fg = 1};
+    pico8_js_t *js_fg = pico8_js_new(&with_fg);
+    pico8_ui_t *ui = pico8_ui_new();
+    pico8_qb_t *qb = pico8_qb_new();
+    pico8_fv_t *fv = pico8_fv_new();
+    pico8_jo_t *jo = pico8_jo_new(NULL);
+    pico8_fr_t *fr = pico8_fr_new(NULL);
+    pico8_jo_t *fr_jo = &fr->nr.qh.jo;
+
+    assert_links(&bg->header, (pico8_struct_header_t){.fa = &bg->def->fa});
+    assert_links(&js->header, (pico8_struct_header_t){.fa = &js->fa, .ra = &js->ra, .kq = js->header.kq});
+    assert(js->header.kq != NULL);
+    assert_links(&js_fg->header,
+                 (pico8_struct_header_t){.fa = &js_fg->fa, .ra = &js_fg->ra, .kq = js_fg->header.kq, .fg = &js_fg->fg});
+    assert_links(&ie->header, (pico8_struct_header_t){.fa = &ie->def->fa, .ra = &ie->ra, .kq = &ie->def->kq});
+    assert_links(&ui->header, (pico8_struct_header_t){.fa = &ui->def->fa, .ql = &ui->def->ql});
+    assert_links(&qb->header, (pico8_struct_header_t){.ra = &qb->ra, .p = &qb->p, .ma = &qb->ma});
+    assert_links(&fv->g_.header, (pico8_struct_header_t){.fa = &fv->g_.def->fa,
+                                                         .ra = &fv->g_.ra,
+                                                         .p = &fv->g_.p,
+                                                         .kq = &fv->g_.def->kq,
+                                                         .ql = &fv->g_.def->ql,
+                                                         .fg = &fv->g_.def->fg});
+    assert_links(&jo->header, (pico8_struct_header_t){.fa = &jo->def->fa,
+                                                      .ra = (const pico8_vec_t *const *)&jo->ra,
+                                                      .p = (const pico8_vec_t *const *)&jo->p,
+                                                      .kq = &jo->def->kq,
+                                                      .ma = &jo->ma});
+    assert_links(&fr_jo->header, (pico8_struct_header_t){.fa = &fr_jo->def->fa,
+                                                         .ra = (const pico8_vec_t *const *)&fr_jo->ra,
+                                                         .p = (const pico8_vec_t *const *)&fr_jo->p,
+                                                         .kq = &fr_jo->def->kq,
+                                                         .mt = &fr_jo->def->mt,
+                                                         .ma = &fr_jo->ma});
+}
+
+static pico8_jc_t *jc_dragged_over_nothing(pico8_struct_header_t *qu) {
+    pico8_jc_keys_t keys = {0};
+    pico8_vec_t *mp = malloc(sizeof *mp);
+    pico8_jc_t *jc;
+
+    keys.qu = qu;
+    jc = pico8_jc_new(&keys);
+    *mp = pico8_vec_v(pico8_fixed_from_int(-640), pico8_fixed_from_int(-640));
+    pico8_ok()->o_ = NULL;
+    pico8_jc_struct_class()->hq(jc, mp, NULL);
+    return jc;
+}
+
+/* The point `jc:hq` builds over nothing carries `ra` and `p` alone; the
+ * slipway's `mt` is its start's, else nil. */
+static void test_mouse_point_has_null_links(void) {
+    pico8_jc_t *jc = jc_dragged_over_nothing(&pico8_jo_new(NULL)->header);
+    pico8_jc_mouse_point_t *point = (pico8_jc_mouse_point_t *)jc->re;
+    pico8_jc_t *from_fr = jc_dragged_over_nothing(&pico8_fr_new(NULL)->nr.qh.jo.header);
 
-    assert_links(&bg->header, &bg->def->fa, NULL, NULL);
-    assert_links(&js->header, &js->fa, &js->ra, NULL);
-    assert_links(&ie->header, &ie->def->fa, &ie->ra, NULL);
+    assert(point != NULL && point->header.cls == pico8_jc_mouse_point_struct_class());
+    assert_links(&point->header, (pico8_struct_header_t){.ra = &point->ra, .p = &point->p});
+    assert(!jc->has_mt && jc->fa == pico8_fixed_from_int(4));
+    assert_links(&jc->header, (pico8_struct_header_t){.fa = &jc->fa});
+    assert(from_fr->has_mt && from_fr->mt == pico8_fr_def.mt && from_fr->fa == pico8_fixed_from_int(2));
+    assert_links(&from_fr->header, (pico8_struct_header_t){.fa = &from_fr->fa, .mt = &from_fr->mt});
 }
 
 int main(int argc, char **argv) {
@@ -109,6 +185,7 @@ int main(int argc, char **argv) {
     assert_malloc_fills();
     test_non_carriers_have_null_links();
     test_carriers_link_their_storage();
+    test_mouse_point_has_null_links();
 
     printf("test_pico8_struct_header: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index 6150e66..ce5ff54 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -318,10 +318,9 @@ static void test_shared(void) {
     pico8_ui_t *ok = start_game();
     pico8_ui_t *other = pico8_ui_new();
     const pico8_ents_t *le = ok->le;
-    pico8_fixed_t ql;
 
     assert(other != NULL && other != ok);
-    assert(other->header.cls->get_ql(other, &ql) && ql == other->def->ql);
+    assert(other->header.ql == &other->def->ql);
     assert(other->le == le && le == pico8_ui_le_shared());
     h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), other));
     h = fnv(h, (uint32_t)ents_rank_in(pico8_cv("ev"), other));
```

#### Diff de 2.14d1 (`git diff c d1`)

```diff
diff --git a/src/pico8_bootstrap.c b/src/pico8_bootstrap.c
index 3b26663..4941569 100644
--- a/src/pico8_bootstrap.c
+++ b/src/pico8_bootstrap.c
@@ -67,7 +67,7 @@ void pico8_dg(void) {
             }
         }
 
-        if (qc->cls->get_p_ != NULL && qc->cls->get_p_(qc)) {
+        if (qc->p_ != NULL && *qc->p_) {
             pico8_he(qc);
         }
 
diff --git a/src/pico8_help.c b/src/pico8_help.c
index d46fd0a..94842c8 100644
--- a/src/pico8_help.c
+++ b/src/pico8_help.c
@@ -118,8 +118,6 @@ static const pico8_struct_class_t g_fv_struct_class = {
     .emitter_of = pico8_g__emitter_of,
     .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_g__get_fx,
-    .get_p_ = pico8_g__get_p_,
     .set_m_ = fv_set_m_,
 };
 
diff --git a/src/pico8_msgbox.c b/src/pico8_msgbox.c
index 482f88f..e8ff988 100644
--- a/src/pico8_msgbox.c
+++ b/src/pico8_msgbox.c
@@ -39,7 +39,6 @@ void pico8_mh_pq(pico8_mh_t *self) {
     previous = pico8_kc();
     if (previous != NULL) {
         previous->p_ = 1;
-        previous->has_p_ = 1;
     }
     pico8_msgbox_set_kc(self);
 }
@@ -79,22 +78,14 @@ void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mh_t *self) {
 
     if (t > PICO8_FIXED_LIT(0x000a8000, 10.5)) {
         self->p_ = 1;
-        self->has_p_ = 1;
         pico8_msgbox_set_kc(NULL);
         assert(pico8_kc() == NULL && "`self.p_,kc=true` is one value for two targets: kc gets nil, not true");
     }
 }
 
-static int mh_get_p_(const void *self) {
-    const pico8_mh_t *mh = self;
-
-    return mh->has_p_ && mh->p_;
-}
-
 static const pico8_struct_class_t g_mh_struct_class = {
     .ev = pico8_mh_ev_dispatch,
     .tags = PICO8_TAG(PICO8_TAG_MH),
-    .get_p_ = mh_get_p_,
 };
 
 static pico8_mh_t *pico8_mh_struct_new(const pico8_mh_keys_t *keys) {
@@ -106,8 +97,8 @@ static pico8_mh_t *pico8_mh_struct_new(const pico8_mh_keys_t *keys) {
     self->header.fa = &self->def->fa;
     self->rh = self->def->rh;
     self->lh = NULL;
+    self->header.p_ = &self->p_;
     self->p_ = 0;
-    self->has_p_ = 0;
     self->items_count = 0;
     if (keys != NULL) {
         assert(keys->count <= PICO8_MH_ITEMS_CAP &&
@@ -191,7 +182,6 @@ void pico8_iv_ph(pico8_iv_t *self) {
     self->l = new_l;
 
     self->p_ = new_l >= pico8_fixed_from_int(5);
-    self->has_p_ = 1;
 }
 
 static void pico8_iv_ph_dispatch(void *self, pico8_fixed_t t) {
@@ -223,16 +213,9 @@ static void pico8_iv_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_iv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_iv_t *)self, p);
 }
 
-static int iv_get_p_(const void *self) {
-    const pico8_iv_t *iv = self;
-
-    return iv->has_p_ && iv->p_;
-}
-
 static const pico8_struct_class_t g_iv_struct_class = {
     .kj = pico8_iv_kj_dispatch,
     .states = {[PICO8_STATE_PH] = pico8_iv_ph_dispatch},
-    .get_p_ = iv_get_p_,
 };
 
 static pico8_iv_t *pico8_iv_struct_new(const pico8_iv_keys_t *keys) {
@@ -244,8 +227,8 @@ static pico8_iv_t *pico8_iv_struct_new(const pico8_iv_keys_t *keys) {
     self->header.ra = &self->ra;
     self->l = self->def->l;
     self->ra = NULL;
+    self->header.p_ = &self->p_;
     self->p_ = 0;
-    self->has_p_ = 0;
     self->at1 = NULL;
     self->at2 = pico8_fixed_from_int(0);
     if (keys != NULL) {
diff --git a/src/pico8_msgbox.h b/src/pico8_msgbox.h
index 38e7e9e..990db53 100644
--- a/src/pico8_msgbox.h
+++ b/src/pico8_msgbox.h
@@ -30,7 +30,6 @@ struct pico8_mh {
     pico8_fixed_t rh;
     pico8_mh_lh_fn lh; /* NULL: `lh` is nil */
     int p_;
-    int has_p_; /* 0: `p_` is nil, and `p_` is then never read */
     const pico8_jb_row_t *items[PICO8_MH_ITEMS_CAP];
     int items_count;
 };
@@ -84,7 +83,6 @@ typedef struct {
     pico8_fixed_t l;
     const pico8_vec_t *ra;
     int p_;
-    int has_p_; /* 0: `p_` is nil, and `p_` is then never read */
     const char *at1;
     pico8_fixed_t at2;
 } pico8_iv_t;
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index b51d2bf..4b15a54 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -76,18 +76,6 @@ void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
     pico8_jo_ll((pico8_jo_t *)self, t);
 }
 
-int pico8_jo_get_fx(const void *self) {
-    const pico8_jo_t *jo = self;
-
-    return jo->has_fx && jo->fx;
-}
-
-int pico8_jo_get_p_(const void *self) {
-    const pico8_jo_t *jo = self;
-
-    return jo->has_p_ && jo->p_;
-}
-
 void pico8_jo_set_m_(void *self, int m_) {
     pico8_jo_t *jo = self;
 
@@ -106,8 +94,6 @@ static const pico8_struct_class_t g_jo_struct_class = {
     .cn = pico8_jo_cn_dispatch,
     .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
@@ -121,6 +107,8 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     self->header.p = (const pico8_vec_t *const *)&self->p;
     self->header.kq = &self->def->kq;
     self->header.ma = &self->ma;
+    self->header.fx = &self->fx;
+    self->header.p_ = &self->p_;
     self->ma = self->def->ma;
     self->kw = &g_jo_kw_shared;
     self->id = NULL;
@@ -132,13 +120,11 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     self->jy = NULL;
     self->nk = NULL;
     self->fx = 0;
-    self->has_fx = 0;
     self->lq = NULL;
     self->fo = NULL;
     self->jh = 0;
     self->has_jh = 0;
     self->p_ = 0;
-    self->has_p_ = 0;
     self->kr = 0;
     self->has_kr = 0;
     self->cc = NULL;
@@ -325,7 +311,6 @@ void pico8_jo_ll(pico8_jo_t *self, pico8_fixed_t t) {
         return;
     }
     self->p_ = t > pico8_fixed_from_int(60);
-    self->has_p_ = 1;
 }
 
 /* `kv`, indexed by `fz`: `pm` and `ga` are the only kinds whose spec sets
@@ -365,7 +350,6 @@ void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance) {
         pico8_jo_keys_t keys;
 
         self->p_ = 1;
-        self->has_p_ = 1;
 
         target_kind = jo_kv(hp_instance->cw);
 
@@ -379,8 +363,7 @@ void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance) {
     } else {
         pico8_ke_ks((pico8_struct_header_t *)self, "ly");
         pico8_jo_lz(self, hp_instance);
-        self->fx = 1;
-        self->has_fx = 1;
+        self->fx = pico8_fixed_from_int(1);
         fp = pico8_ov("autoassemblers", pico8_fixed_from_int(3), pico8_fixed_from_int(2));
     }
 
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index c4b893f..51b7d17 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -34,14 +34,12 @@ typedef struct {
     const char *ep;
     pico8_ptrs_t *jy;
     const pico8_df_entry_t *nk;
-    int fx;
-    int has_fx;
+    pico8_fixed_t fx;
     pico8_lq_t *lq;
     pico8_struct_header_t *fo;
     pico8_fixed_t jh;
     int has_jh;
     int p_;
-    int has_p_;
     pico8_fixed_t kr;
     int has_kr;
     pico8_ents_t *cc;
@@ -102,8 +100,6 @@ void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw);
 
 /* The field accessors jo's descriptor holds, for a daughter's `static const`
  * descriptor to name. */
-int pico8_jo_get_fx(const void *self);
-int pico8_jo_get_p_(const void *self);
 void pico8_jo_set_m_(void *self, int m_);
 
 /* `e` is one of the ten classes of jo's tree: the only ones whose state
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index 9f2cbf2..df56191 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -50,8 +50,6 @@ static const pico8_struct_class_t g_qh_struct_class = {
     .parent = pico8_jo_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
@@ -123,8 +121,6 @@ static const pico8_struct_class_t g_nl_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
@@ -170,8 +166,6 @@ static const pico8_struct_class_t g_os_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
@@ -237,8 +231,6 @@ static const pico8_struct_class_t g_hj_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
@@ -327,8 +319,6 @@ static const pico8_struct_class_t g_pm_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
@@ -453,7 +443,6 @@ void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec
 
     pico8_ei(gfx, mem, (pico8_struct_header_t *)self);
     self->jo.p_ = last_pt == NULL;
-    self->jo.has_p_ = 1;
 }
 
 static void pico8_ga_kj_dispatch(void *self, const pico8_vec_t *ra) {
@@ -480,8 +469,6 @@ static const pico8_struct_class_t g_ga_struct_class = {
     .emitter_of = ga_emitter_of,
     /* ga's cart list of tags is empty and replaces jo's: never jo's. */
     .tags = 0,
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index 8a45543..c181b32 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -178,8 +178,6 @@ static const pico8_struct_class_t g_nr_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
@@ -235,8 +233,6 @@ static const pico8_struct_class_t g_fr_struct_class = {
     .parent = pico8_nr_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
@@ -333,8 +329,6 @@ static const pico8_struct_class_t g_mx_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_jo_get_fx,
-    .get_p_ = pico8_jo_get_p_,
     .set_m_ = pico8_jo_set_m_,
 };
 
diff --git a/src/pico8_planet_trade.c b/src/pico8_planet_trade.c
index 54d3f7b..3333b1c 100644
--- a/src/pico8_planet_trade.c
+++ b/src/pico8_planet_trade.c
@@ -60,9 +60,8 @@ void pico8_jo_kf(pico8_jo_t *self, pico8_struct_header_t *sw) {
     }
     if (other != NULL) {
         assert(pico8_jo_is_family(other) && "jo:kf: the other end of a registered slipway is a planet");
-        if (((const pico8_jo_t *)other)->has_fx && ((const pico8_jo_t *)other)->fx) {
-            self->fx = 1;
-            self->has_fx = 1;
+        if (((const pico8_jo_t *)other)->fx != 0) {
+            self->fx = pico8_fixed_from_int(1);
         }
     }
 
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index ebc08a9..26235bf 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -279,33 +279,8 @@ void pico8_dl_dispatch(void *self, const pico8_vec_t *ra) {
 /* Raises `p_`, which a host of `d_` holds for itself: `g_`'s line, jo's
  * family (qh's line), a `jc` or an `mz`. */
 static void probe_host_raise_p_(pico8_struct_header_t *self) {
-    pico8_g__t *g_ = pico8_g__of(self);
-    pico8_jo_t *jo;
-    pico8_jc_t *jc;
-    pico8_mz_t *mz;
-
-    if (g_ != NULL) {
-        g_->p_ = 1;
-        g_->has_p_ = 1;
-        return;
-    }
-    if (pico8_jo_is_family(self)) {
-        jo = (pico8_jo_t *)self;
-        jo->p_ = 1;
-        jo->has_p_ = 1;
-        return;
-    }
-    if (self->cls == pico8_jc_struct_class()) {
-        jc = (pico8_jc_t *)self;
-        jc->p_ = 1;
-        jc->has_p_ = 1;
-        return;
-    }
-    assert(self->cls == pico8_mz_struct_class() &&
-           "d_: a host neither of g_'s line, of jo's family, a jc nor an mz, none of whose ey runs d_ -- stop");
-    mz = (pico8_mz_t *)self;
-    mz->p_ = 1;
-    mz->has_p_ = 1;
+    assert(self->p_ != NULL && "d_: a host that carries no p_, none of whose ey runs d_ -- stop");
+    *self->p_ = 1;
 }
 
 int pico8_d_(pico8_struct_header_t *self) {
@@ -395,18 +370,6 @@ pico8_qy_t *pico8_g__mm(void *self_ptr) {
     return particle;
 }
 
-/* `fx` is a fixed here, and a fixed is always true in Lua, even 0. */
-int pico8_g__get_fx(const void *self) {
-    (void)self;
-    return 1;
-}
-
-int pico8_g__get_p_(const void *self) {
-    const pico8_g__t *g_ = self;
-
-    return g_->has_p_ && g_->p_;
-}
-
 static const pico8_struct_class_t g_g__struct_class = {
     .kj = pico8_ei_dispatch,
     .ev = pico8_dl_dispatch,
@@ -416,8 +379,6 @@ static const pico8_struct_class_t g_g__struct_class = {
     .emitter_of = pico8_g__emitter_of,
     .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fx = pico8_g__get_fx,
-    .get_p_ = pico8_g__get_p_,
 };
 
 void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, const pico8_g__def_t *def) {
@@ -426,6 +387,8 @@ void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, con
     self->header.fa = &self->def->fa;
     self->header.ra = &self->ra;
     self->header.p = &self->p;
+    self->header.fx = &self->def->fx;
+    self->header.p_ = &self->p_;
     self->emitter = &self->emitter_own;
     self->emitter->def = &self->def->emitter;
     self->emitter->qy = NULL;
@@ -435,7 +398,6 @@ void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, con
     self->probe->def = &self->def->probe;
     self->probe->et = NULL;
     self->p_ = 0;
-    self->has_p_ = 0;
     self->probe->od = self->def->probe.od;
     self->probe->has_od = self->def->probe.has_od;
     self->emitter->py = self->def->emitter.py;
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index 41aea93..90c6526 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -41,7 +41,6 @@ typedef struct {
     const pico8_vec_t *ra;
     const pico8_vec_t *p;
     int p_;
-    int has_p_; /* 0: `p_` is nil, and is then never read */
     pico8_emitter_t emitter_own;
     pico8_probe_t probe_own;
 } pico8_g__t;
@@ -50,12 +49,9 @@ typedef struct {
  * otherwise, and for a NULL `e`. */
 pico8_g__t *pico8_g__of(pico8_struct_header_t *e);
 
-/* The `emitter_of`, `probe_of` and field accessors of g_'s descriptor, for
- * `fv`'s to name. */
+/* The `emitter_of` and `probe_of` of g_'s descriptor, for `fv`'s to name. */
 pico8_emitter_t *pico8_g__emitter_of(void *self);
 pico8_probe_t *pico8_g__probe_of(void *self);
-int pico8_g__get_fx(const void *self);
-int pico8_g__get_p_(const void *self);
 
 /* TROU-8 */
 /* Puts the provisional `fm` back in the slot, its `jk`/`nz` nil. Shuts the
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index 9815a4b..83b2578 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -153,7 +153,6 @@ void pico8_jc_ey(pico8_jc_t *self, pico8_struct_header_t *re) {
             pico8_rn(pico8_fixed_from_int(-1));
         }
         self->p_ = 1;
-        self->has_p_ = 1;
         return;
     }
 
@@ -343,12 +342,6 @@ static pico8_probe_t *jc_probe_of(void *self) {
     return ((pico8_jc_t *)self)->probe;
 }
 
-static int jc_get_p_(const void *self) {
-    const pico8_jc_t *jc = self;
-
-    return jc->has_p_ && jc->p_;
-}
-
 static const pico8_struct_class_t g_jc_struct_class = {
     .kj = pico8_jc_kj_dispatch,
     .ev = pico8_jc_ev_dispatch,
@@ -356,7 +349,6 @@ static const pico8_struct_class_t g_jc_struct_class = {
     .ey = pico8_jc_ey_dispatch,
     .probe_of = jc_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JC),
-    .get_p_ = jc_get_p_,
 };
 
 static pico8_jc_t *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
@@ -380,8 +372,8 @@ static pico8_jc_t *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
     self->header.mt = NULL;
     self->probe->et = NULL;
     self->fo = NULL;
+    self->header.p_ = &self->p_;
     self->p_ = 0;
-    self->has_p_ = 0;
     if (keys != NULL && keys->qu != NULL) {
         assert(jc_is_endpoint_class(keys->qu->cls) &&
                "jc.qu: a key neither nil nor a planet a slipway starts from (jo, qh, nl, os, hj, pm, nr, fr, mx) "
diff --git a/src/pico8_slipway.h b/src/pico8_slipway.h
index c25bf5e..ac4cac3 100644
--- a/src/pico8_slipway.h
+++ b/src/pico8_slipway.h
@@ -47,7 +47,6 @@ typedef struct {
     int has_mt;                /* 0: `mt` is nil, and is then never read */
     pico8_struct_header_t *fo; /* borrowed, never freed */
     int p_;
-    int has_p_; /* 0: `p_` is nil, and is then never read */
     pico8_probe_t *probe;
     pico8_probe_t probe_own;
 } pico8_jc_t;
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index d31fd00..89374a6 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -79,7 +79,7 @@ typedef struct pico8_struct_class pico8_struct_class_t;
  * embeds first. `me` and `t` are ke's (`me` NULL is nil). A component link
  * points at the field's storage, the archetype's for a field the class
  * never writes; NULL: the class has no such field, or it is nil. `ma` is
- * written through, by `pico8_ma`. */
+ * written through, by `pico8_ma`, and `p_`, by `d_`. `*fx` 0 is nil. */
 typedef struct {
     const pico8_struct_class_t *cls;
     const char *me;
@@ -92,6 +92,8 @@ typedef struct {
     const pico8_fixed_t *fg;
     const pico8_fixed_t *mt;
     pico8_fixed_t *ma;
+    const pico8_fixed_t *fx;
+    int *p_;
 } pico8_struct_header_t;
 
 /* Every constructor calls it first: its `malloc` leaves the header
@@ -151,11 +153,8 @@ struct pico8_struct_class {
     const pico8_struct_class_t *(*parent)(void);
     struct pico8_emitter *(*emitter_of)(void *self);
     struct pico8_probe *(*probe_of)(void *self);
-    /* A NULL member: the class has no such field, for every instance.
-     * `get_fx` and `get_p_` return its Lua truth. `set_m_` also marks `m_`
-     * present. Each is only ever called as `e->cls->get_X(e, ...)`. */
-    int (*get_fx)(const void *self);
-    int (*get_p_)(const void *self);
+    /* NULL: the class has no `m_`, for every instance. Also marks `m_`
+     * present; only ever called as `e->cls->set_m_(e, m_)`. */
     void (*set_m_)(void *self, int m_);
 };
 
diff --git a/src/pico8_ui_button.c b/src/pico8_ui_button.c
index 9dbbef3..aa401d5 100644
--- a/src/pico8_ui_button.c
+++ b/src/pico8_ui_button.c
@@ -299,12 +299,6 @@ static pico8_down_t pico8_fq_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_v
     return pico8_fq_ldown((pico8_fq_t *)self);
 }
 
-static int fq_get_p_(const void *self) {
-    const pico8_fq_t *fq = self;
-
-    return fq->has_p_ && fq->p_;
-}
-
 static void fq_set_m_(void *self, int m_) {
     pico8_fq_t *fq = self;
 
@@ -315,7 +309,6 @@ static void fq_set_m_(void *self, int m_) {
 static const pico8_struct_class_t g_fq_struct_class = {
     .hd = pico8_hd_true,
     .ldown = pico8_fq_ldown_dispatch,
-    .get_p_ = fq_get_p_,
     .set_m_ = fq_set_m_,
 };
 
@@ -326,8 +319,8 @@ static pico8_fq_t *pico8_fq_struct_new(const pico8_fq_keys_t *keys) {
     self->def = &pico8_fq_def;
     self->header.fa = &self->def->fa;
     self->my = NULL;
+    self->header.p_ = &self->p_;
     self->p_ = 0;
-    self->has_p_ = 0;
     self->m_ = 0;
     self->has_m_ = 0;
     if (keys != NULL) {
@@ -368,10 +361,8 @@ pico8_down_t pico8_fq_ldown(pico8_fq_t *self) {
         return out;
     }
     self->p_ = 0;
-    self->has_p_ = 0;
     if (self->my != NULL) {
         self->p_ = self->my(self) != 0;
-        self->has_p_ = 1;
     }
     return out;
 }
diff --git a/src/pico8_ui_button.h b/src/pico8_ui_button.h
index 057aa6a..f1fd347 100644
--- a/src/pico8_ui_button.h
+++ b/src/pico8_ui_button.h
@@ -124,7 +124,6 @@ struct pico8_fq {
     const pico8_fq_def_t *def;
     pico8_fq_my_fn my;
     int p_;
-    int has_p_;
     int m_;
     int has_m_;
 };
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index 95a10e9..d283586 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -41,7 +41,7 @@ void pico8_mz_pq(pico8_mz_t *self) {
     pico8_ents_init(heap_ox);
     it = pico8_ents_all(pico8_hv());
     while (pico8_ents_iter_next(&it, &p)) {
-        if (p != NULL && p->cls->get_fx != NULL && p->cls->get_fx(p)) {
+        if (p != NULL && p->fx != NULL && *p->fx != 0) {
             pico8_ents_add(heap_ox, p);
         }
     }
@@ -121,7 +121,6 @@ void pico8_mz_ey(pico8_mz_t *self) {
     }
 
     self->p_ = result;
-    self->has_p_ = 1;
 }
 
 static void pico8_mz_ey_dispatch(void *self, pico8_struct_header_t *kv) {
@@ -167,7 +166,6 @@ void pico8_mz_pr(pico8_mz_t *self, pico8_fixed_t t) {
     }
 
     self->p_ = t > pico8_fixed_from_int(25);
-    self->has_p_ = 1;
 }
 
 static void pico8_mz_pr_dispatch(void *self, pico8_fixed_t t) {
@@ -199,12 +197,6 @@ static pico8_probe_t *mz_probe_of(void *self) {
     return ((pico8_mz_t *)self)->probe;
 }
 
-static int mz_get_p_(const void *self) {
-    const pico8_mz_t *mz = self;
-
-    return mz->has_p_ && mz->p_;
-}
-
 static const pico8_struct_class_t g_mz_struct_class = {
     .kj = pico8_mz_kj_dispatch,
     .ev = pico8_dl_dispatch,
@@ -217,7 +209,6 @@ static const pico8_struct_class_t g_mz_struct_class = {
         },
     .probe_of = mz_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_MZ),
-    .get_p_ = mz_get_p_,
 };
 
 static pico8_mz_t *pico8_mz_struct_new(void) {
@@ -239,8 +230,8 @@ static pico8_mz_t *pico8_mz_struct_new(void) {
     self->p = NULL;
     self->probe->et = NULL;
     self->ij = NULL;
+    self->header.p_ = &self->p_;
     self->p_ = 0;
-    self->has_p_ = 0;
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("mz");
 #endif
diff --git a/src/pico8_wormhole.h b/src/pico8_wormhole.h
index 174e545..409fdd4 100644
--- a/src/pico8_wormhole.h
+++ b/src/pico8_wormhole.h
@@ -35,7 +35,6 @@ typedef struct {
     const pico8_vec_t *p;
     pico8_struct_header_t *ij; /* borrowed, never freed */
     int p_;
-    int has_p_; /* 0: `p_` is nil, and is then never read */
     pico8_probe_t *probe;
     pico8_probe_t probe_own;
 } pico8_mz_t;
diff --git a/tests/test_pico8_fq.c b/tests/test_pico8_fq.c
index dc56564..ea7aa18 100644
--- a/tests/test_pico8_fq.c
+++ b/tests/test_pico8_fq.c
@@ -27,7 +27,6 @@
 #define MY_TRUE_TICKS 6
 #define MY_TRUE_SEQUENCE_HASH 0xA6A43C27u
 #define BARE_SEQUENCE_HASH 0x52BE382Au
-#define MY_FALSE_SEQUENCE_HASH 0xE43E9427u
 #define HOVER_SEQUENCE_HASH 0x27C4BCB8u
 #define HOVER_CURSOR_HASH 0x04C40AB9u
 
@@ -83,7 +82,7 @@ static uint32_t fold_bool_member(uint32_t h, int has, int b) {
  * sequence hashes still prove the typed reads. */
 static uint32_t fold_veil(uint32_t h, const pico8_fq_t *e) {
     h = fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)e->header.t);
-    h = fold_bool_member(h, e->has_p_, e->p_);
+    h = fold_bool_member(h, (e->p_ != 0), e->p_);
     h = fold_bool_member(h, e->has_m_, e->m_);
     h = fnv(h, (uint32_t)ents_listed(pico8_hv(), e));
     return fnv(h, (uint32_t)ents_listed(pico8_cv("hd"), e));
@@ -164,7 +163,7 @@ static void test_bare_veil_swallows_the_click_and_stays(void) {
     for (i = 0; i < IDLE_TICKS; i++) {
         if (i == LDOWN_TICK) {
             assert(ldown(veil));
-            assert(!veil->has_p_);
+            assert(!veil->p_);
         }
         pico8_dg();
         h = fold_veil(h, veil);
@@ -173,7 +172,8 @@ static void test_bare_veil_swallows_the_click_and_stays(void) {
     check("bare", h, BARE_SEQUENCE_HASH);
 }
 
-/* A `my` answering false leaves `p_` false: the veil stays. */
+/* A `my` answering false leaves `p_` false, which reads as nil: the veil
+ * stays, as a bare one. */
 static void test_false_my_keeps_the_veil(void) {
     uint32_t h = 2166136261u;
     pico8_fq_t *veil;
@@ -185,13 +185,13 @@ static void test_false_my_keeps_the_veil(void) {
         if (i == LDOWN_TICK) {
             assert(ldown(veil));
             assert(g_my_calls == 1);
-            assert(veil->has_p_ && !veil->p_);
+            assert(!veil->p_);
         }
         pico8_dg();
         h = fold_veil(h, veil);
     }
     assert(ents_listed(pico8_hv(), veil) && ents_listed(pico8_cv("hd"), veil));
-    check("my false", h, MY_FALSE_SEQUENCE_HASH);
+    check("my false", h, BARE_SEQUENCE_HASH);
 }
 
 static int in_layer(int layer, const void *e) {
diff --git a/tests/test_pico8_fr.c b/tests/test_pico8_fr.c
index b99fbdb..135ea09 100644
--- a/tests/test_pico8_fr.c
+++ b/tests/test_pico8_fr.c
@@ -309,11 +309,11 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)jo->has_fx);
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)jo->has_jh);
-    h = fnv(h, (uint32_t)jo->has_p_);
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)jo->has_kr);
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
diff --git a/tests/test_pico8_fv.c b/tests/test_pico8_fv.c
index f95c8fc..d2438a9 100644
--- a/tests/test_pico8_fv.c
+++ b/tests/test_pico8_fv.c
@@ -354,7 +354,7 @@ static void test_built(void) {
     h = fold_fixed_member(h, g_->header.t);
     h = fnv(h, (uint32_t)size_of(em->qy));
     assert(size_of(em->qy) == 0);
-    assert(!fv->has_m_ && g_->p == NULL && pr->et == NULL && !g_->has_p_);
+    assert(!fv->has_m_ && g_->p == NULL && pr->et == NULL && !g_->p_);
     assert(cls_of(fv)->hk == NULL && cls_of(fv)->hb == NULL && cls_of(fv)->states[PICO8_STATE_PH] == NULL &&
            cls_of(fv)->ldown == NULL);
 
diff --git a/tests/test_pico8_g_.c b/tests/test_pico8_g_.c
index 8947703..09211b4 100644
--- a/tests/test_pico8_g_.c
+++ b/tests/test_pico8_g_.c
@@ -165,7 +165,7 @@ static uint32_t fold_ref_member(uint32_t h, const void *ref) {
 }
 
 static uint32_t fold_p_(uint32_t h, const pico8_g__t *g_) {
-    return g_->has_p_ ? fold_bool_member(h, g_->p_ != 0) : fnv(h, 0u);
+    return (g_->p_ != 0) ? fold_bool_member(h, g_->p_ != 0) : fnv(h, 0u);
 }
 
 /* `g_`'s 18 spec defaults, in spec order. */
@@ -373,7 +373,7 @@ static uint32_t fold_probe(uint32_t h, pico8_g__t *e) {
     assert(em->k_ == &pico8_g__k__seq);
     h = fnv(h, (uint32_t)size_of(em->qy));
     assert(size_of(em->qy) == 0);
-    assert(e->ra == NULL && e->p == NULL && pr->et == NULL && !e->has_p_);
+    assert(e->ra == NULL && e->p == NULL && pr->et == NULL && !e->p_);
     assert(cls_of(e)->hd == NULL && cls_of(e)->hk == NULL && cls_of(e)->hb == NULL &&
            cls_of(e)->states[PICO8_STATE_LY] == NULL && cls_of(e)->states[PICO8_STATE_PH] == NULL);
     assert(cls_of(e)->hq == pico8_bv_dispatch);
@@ -583,7 +583,7 @@ static void test_life(void) {
     }
     assert(cls_of(a)->ey == pico8_d__dispatch);
     assert(pico8_d_(&a->header) == 0);
-    assert(a->has_p_ && a->p_);
+    assert(a->p_);
     pico8_dg();
     assert(!ents_listed(pico8_hv(), a) && !ents_listed(pico8_cg("g_"), a) && !ents_listed(pico8_cg("prober"), a));
     assert(ents_listed(pico8_hv(), b) && ents_listed(pico8_cg("g_"), b));
@@ -603,7 +603,7 @@ static void test_ey_on(void) {
     assert(cls_of(a)->ey == pico8_d__dispatch);
     out = pico8_d_(&a->header);
     assert(out == 0);
-    assert(a->has_p_ && a->p_);
+    assert(a->p_);
     h = fold_string_member(h, a->header.me);
     h = fold_fixed_member(h, a->header.t);
     h = fold_p_(h, a);
@@ -622,7 +622,7 @@ static void test_ey_off(void) {
     assert(cls_of(a)->ey == pico8_d__dispatch);
     out = pico8_d_(&a->header);
     assert(out == 1);
-    assert(!a->has_p_);
+    assert(!a->p_);
     assert(is_me(&a->header, "ly"));
     assert(a->header.t == pico8_fixed_from_int(0));
     h = fold_string_member(h, a->header.me);
@@ -646,13 +646,13 @@ static void test_ey_void(void) {
     cls_of(a)->hq(a, mouse_at(pico8_vec_add(middle, vec_xy(0, 3))), vec_ref(vec_xy(2, 2)));
     assert(et_kind(a) == 4);
     assert(et_fo(a) == a);
-    assert(!a->has_p_);
+    assert(!a->p_);
     nz = fm_hy()->nz;
     me = a->header.me;
     assert(cls_of(a)->ey == pico8_d__dispatch);
     out = pico8_d_(&a->header);
     assert(out == 0);
-    assert(a->has_p_ && a->p_);
+    assert(a->p_);
     assert(fm_hy()->nz == nz);
     assert(a->header.me == me);
     assert(et_kind(a) == 4);
diff --git a/tests/test_pico8_ga.c b/tests/test_pico8_ga.c
index 7aafbb8..a06ea5d 100644
--- a/tests/test_pico8_ga.c
+++ b/tests/test_pico8_ga.c
@@ -39,7 +39,7 @@
 
 
 #define BUILT_HASH 0xEA365129u
-#define LIFE_HASH 0x85D7319Eu
+#define LIFE_HASH 0x196B693Au
 #define BURST_FRAMES 86
 
 /* The tag `lq.fs` carried as a tagged list, which the frozen hashes fold:
@@ -114,7 +114,7 @@ static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
 }
 
 static uint32_t fold_p_(uint32_t h, const pico8_jo_t *jo) {
-    return jo->has_p_ ? fold_bool_member(h, jo->p_) : fold_absent(h);
+    return (jo->p_ != 0) ? fold_bool_member(h, jo->p_) : fold_absent(h);
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -357,11 +357,11 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)(jo->has_fx != 0));
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)(jo->has_jh != 0));
-    h = fnv(h, (uint32_t)(jo->has_p_ != 0));
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)(jo->has_kr != 0));
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
@@ -572,11 +572,11 @@ static void test_life(void) {
     }
 
     h = fold_cut(h, a, 3);
-    assert(!(as_ga(a)->jo.has_p_ && as_ga(a)->jo.p_));
+    assert(!as_ga(a)->jo.p_);
     h = fold_cut(h, a, 2);
-    assert(as_ga(a)->jo.has_p_ && as_ga(a)->jo.p_);
+    assert(as_ga(a)->jo.p_);
     h = fold_cut(h, a, 0);
-    assert(as_ga(a)->jo.has_p_ && as_ga(a)->jo.p_);
+    assert(as_ga(a)->jo.p_);
 
     h = fold_bool(h, cls_of(b)->hd(b, as_ga(b)->jo.ra, as_ga(b)->jo.ra));
     h = fold_bool(h, cls_of(b)->hd(b, vec_ref(near_mo(0, 0)), vec_ref(near_mo(0, 0))));
diff --git a/tests/test_pico8_hj.c b/tests/test_pico8_hj.c
index 67ba7bd..b570021 100644
--- a/tests/test_pico8_hj.c
+++ b/tests/test_pico8_hj.c
@@ -381,11 +381,11 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)jo->has_fx);
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)jo->has_jh);
-    h = fnv(h, (uint32_t)jo->has_p_);
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)jo->has_kr);
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
diff --git a/tests/test_pico8_hp.c b/tests/test_pico8_hp.c
index 8b663ae..1b254ae 100644
--- a/tests/test_pico8_hp.c
+++ b/tests/test_pico8_hp.c
@@ -646,8 +646,8 @@ static uint32_t fold_planet(uint32_t h, void *e) {
     h = fold_jy(h, p);
     h = fold_kw(h, p);
     h = fold_string_member(h, p->header.me);
-    h = fold_bool_member(h, p->has_fx, p->fx);
-    h = fold_bool_member(h, p->has_p_, p->p_);
+    h = fold_bool_member(h, (p->fx != 0), p->fx);
+    h = fold_bool_member(h, (p->p_ != 0), p->p_);
     h = fold_fixed_member(h, 1, p->ma);
     h = p->nk != NULL ? fnv(h, (uint32_t)(p->nk - pico8_df_table)) : fnv(h, 0xFFFFFFFFu);
     return fnv(h, (uint32_t)ents_rank_in(pico8_hv(), p));
diff --git a/tests/test_pico8_iv.c b/tests/test_pico8_iv.c
index 5db0f3c..7de526e 100644
--- a/tests/test_pico8_iv.c
+++ b/tests/test_pico8_iv.c
@@ -185,15 +185,8 @@ static void fake_toast_ph(void *self, pico8_fixed_t t) {
     }
 }
 
-static int fake_toast_get_p_(const void *self) {
-    const fake_toast_t *f = self;
-
-    return f->p_ != 0;
-}
-
 static const pico8_struct_class_t g_fake_toast_class = {
     .states = {[PICO8_STATE_PH] = fake_toast_ph},
-    .get_p_ = fake_toast_get_p_,
 };
 
 static void test_dg_drives_a_ke_struct(void) {
@@ -202,6 +195,7 @@ static void test_dg_drives_a_ke_struct(void) {
 
     fresh_world();
     pico8_ke_struct_init(&f->header, &g_fake_toast_class, "ph", 0);
+    f->header.p_ = &f->p_;
     f->p_ = 0;
     pico8_ke_struct_pq(&f->header);
     assert(ents_listed(pico8_hv(), f));
diff --git a/tests/test_pico8_jc.c b/tests/test_pico8_jc.c
index f99de76..4ce6592 100644
--- a/tests/test_pico8_jc.c
+++ b/tests/test_pico8_jc.c
@@ -108,7 +108,7 @@ static pico8_vec_t vec_of(const pico8_vec_t *v) {
 
 /* 0 absent, 1 false, 2 true. */
 static uint32_t p__state(const pico8_jc_t *jc) {
-    if (!jc->has_p_) {
+    if (!jc->p_) {
         return 0;
     }
     return jc->p_ ? 2u : 1u;
@@ -463,7 +463,7 @@ static void check_fresh_jc(const void *e, const void *qu) {
 
     assert(jc->re == NULL && !jc->probe->has_od && jc->probe->od == 0 && !jc->has_mt && jc->probe->et == NULL);
     assert(jc->header.mt == NULL);
-    assert(jc->fo == NULL && !jc->has_p_);
+    assert(jc->fo == NULL && !jc->p_);
     assert(cls_of(jc)->hq != NULL && cls_of(jc)->ey != NULL);
     assert(cls_of(jc)->ev != NULL && cls_of(jc)->kj != NULL);
     assert(cls_of(jc)->hd == NULL && cls_of(jc)->states[PICO8_STATE_KZ] == NULL &&
diff --git a/tests/test_pico8_jo.c b/tests/test_pico8_jo.c
index 9fa1cba..5ae88cb 100644
--- a/tests/test_pico8_jo.c
+++ b/tests/test_pico8_jo.c
@@ -107,7 +107,7 @@ static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
 
 /* 0 absent, 1 false, 2 true. */
 static uint32_t p__state(const pico8_jo_t *jo) {
-    return !jo->has_p_ ? 0u : jo->p_ ? 2u : 1u;
+    return jo->p_ ? 2u : 0u;
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -237,11 +237,11 @@ static uint32_t fold_planet(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)jo->has_fx);
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)jo->has_jh);
-    h = fnv(h, (uint32_t)jo->has_p_);
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)jo->has_kr);
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
@@ -282,7 +282,7 @@ static uint32_t fold_live(uint32_t h, const void *e) {
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
     h = fold_fixed_member(h, jo->ma);
-    h = jo->has_fx ? fold_bool_member(h, jo->fx) : fold_absent(h);
+    h = (jo->fx != 0) ? fold_bool_member(h, jo->fx) : fold_absent(h);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = jo->has_kr ? fold_fixed_member(h, jo->kr) : fold_absent(h);
     h = fold_string_member(h, jo->ep);
diff --git a/tests/test_pico8_mh.c b/tests/test_pico8_mh.c
index 64303b2..7a44a85 100644
--- a/tests/test_pico8_mh.c
+++ b/tests/test_pico8_mh.c
@@ -92,7 +92,7 @@ enum { FIXED_KIND = 0, BOOL_KIND = 2 };
  * sequence hashes still prove the typed reads. */
 static uint32_t fold_box(uint32_t h, const pico8_mh_t *e) {
     h = fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)e->header.t);
-    if (e->has_p_) {
+    if (e->p_) {
         h = fnv(fnv(h, (uint32_t)BOOL_KIND), (uint32_t)e->p_);
     } else {
         h = fnv(h, 0xDEADu);
@@ -226,10 +226,10 @@ static void test_two_years_in_a_row(void) {
     for (i = 0; i < 20; i++) {
         tick(&run);
     }
-    assert(!first->has_p_);
+    assert(!first->p_);
     second = year_box(3421);
     track(&run, second);
-    assert(first->has_p_ && first->p_);
+    assert(first->p_);
     run.h = fnv(run.h, draw_hash());
     tick(&run);
     assert(!ents_listed(pico8_hv(), first));
diff --git a/tests/test_pico8_mx.c b/tests/test_pico8_mx.c
index 10e5606..9042328 100644
--- a/tests/test_pico8_mx.c
+++ b/tests/test_pico8_mx.c
@@ -375,11 +375,11 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)jo->has_fx);
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)jo->has_jh);
-    h = fnv(h, (uint32_t)jo->has_p_);
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)jo->has_kr);
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
@@ -566,7 +566,7 @@ static uint32_t fold_kf(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)ht_size(e));
     h = fold_kw(h, jo);
     h = fold_fixed_member(h, jo->ma);
-    h = jo->has_fx ? fold_bool_member(h, jo->fx) : fold_absent(h);
+    h = (jo->fx != 0) ? fold_bool_member(h, jo->fx) : fold_absent(h);
     return fnv(h, (uint32_t)ents_ptrs_size(jo->oe));
 }
 
diff --git a/tests/test_pico8_mz.c b/tests/test_pico8_mz.c
index c73ec3d..75351f1 100644
--- a/tests/test_pico8_mz.c
+++ b/tests/test_pico8_mz.c
@@ -47,8 +47,8 @@
 #define HQ_HASH 0x9DAA980Cu
 #define KZ_TICKS 21
 #define KZ_HASH 0xE16D1D52u
-#define EY_HASH 0x2D563CE9u
-#define PR_HASH 0xAC265666u
+#define EY_HASH 0x52EB8E18u
+#define PR_HASH 0x85EB1EC6u
 #define PR_EXIT_TICK 27
 #define DRAW_HASH 0xD124A50Cu
 
@@ -115,12 +115,9 @@ static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
     return fnv(h, (uint32_t)p->y);
 }
 
-/* 0 absent, 1 false, 2 true. */
+/* 0 absent or false, 2 true. */
 static uint32_t p__state(const pico8_mz_t *mz) {
-    if (!mz->has_p_) {
-        return 0;
-    }
-    return mz->p_ ? 2 : 1;
+    return mz->p_ ? 2 : 0;
 }
 
 static uint32_t fold_string(uint32_t h, const char *s) {
@@ -325,7 +322,7 @@ static void check_fresh_mz(void *e) {
     assert(jj != NULL && jj->qs != NULL && strcmp(jj->qs, "-too far-") == 0);
     assert(pico8_mz_jj_wrapper() == jj);
 
-    assert(mz->ra == NULL && mz->p == NULL && mz->probe->et == NULL && mz->ij == NULL && !mz->has_p_);
+    assert(mz->ra == NULL && mz->p == NULL && mz->probe->et == NULL && mz->ij == NULL && !mz->p_);
     assert(mz->ox != NULL);
 
     assert(ents_listed(pico8_hv(), mz) && ents_listed(pico8_cv("kj"), mz) && ents_listed(pico8_cv("ev"), mz));
@@ -383,13 +380,11 @@ static void test_ox(void) {
     jos = pico8_cg("jo");
     for (i = 0; unlit == NULL && jos != NULL && i < jos->count; i++) {
         pico8_jo_t *jo = as_jo(jos->items[i]);
-        if (!jo->has_fx) {
+        if (jo->fx == 0) {
             unlit = jo;
         }
     }
     assert(unlit != NULL);
-    unlit->fx = 0;
-    unlit->has_fx = 1;
     first = build_mz();
     h = fold_ox(h, first);
     second = build_mz();
@@ -405,8 +400,7 @@ static void test_ox(void) {
     assert(ents_listed(build_mz()->ox, probe));
 
     lit = pico8_qh_new(NULL);
-    lit->jo.fx = 1;
-    lit->jo.has_fx = 1;
+    lit->jo.fx = pico8_fixed_from_int(1);
     assert(ents_listed(build_mz()->ox, lit));
 }
 
@@ -541,7 +535,7 @@ static void test_ey(void) {
     hq(paid, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
     assert(et_kind(paid) == ET_FALSE);
     ey(paid);
-    assert(is_me(&paid->header, "pr") && paid->header.t == 0 && p__state(paid) == 1);
+    assert(is_me(&paid->header, "pr") && paid->header.t == 0 && p__state(paid) == 0);
     assert(credits() == pico8_fixed_sub(before, pico8_fixed_from_int(3)));
     h = fold_ey(h, paid);
 
diff --git a/tests/test_pico8_nl.c b/tests/test_pico8_nl.c
index 5b7144b..9c7b440 100644
--- a/tests/test_pico8_nl.c
+++ b/tests/test_pico8_nl.c
@@ -320,11 +320,11 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)jo->has_fx);
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)jo->has_jh);
-    h = fnv(h, (uint32_t)jo->has_p_);
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)jo->has_kr);
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
diff --git a/tests/test_pico8_nr.c b/tests/test_pico8_nr.c
index 1addfdb..b23d956 100644
--- a/tests/test_pico8_nr.c
+++ b/tests/test_pico8_nr.c
@@ -324,11 +324,11 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)jo->has_fx);
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)jo->has_jh);
-    h = fnv(h, (uint32_t)jo->has_p_);
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)jo->has_kr);
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
diff --git a/tests/test_pico8_os.c b/tests/test_pico8_os.c
index 14ad3f4..b3186b5 100644
--- a/tests/test_pico8_os.c
+++ b/tests/test_pico8_os.c
@@ -347,11 +347,11 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)jo->has_fx);
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)jo->has_jh);
-    h = fnv(h, (uint32_t)jo->has_p_);
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)jo->has_kr);
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
diff --git a/tests/test_pico8_pm.c b/tests/test_pico8_pm.c
index 1fa3a1c..45262ec 100644
--- a/tests/test_pico8_pm.c
+++ b/tests/test_pico8_pm.c
@@ -375,11 +375,11 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)jo->has_fx);
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)jo->has_jh);
-    h = fnv(h, (uint32_t)jo->has_p_);
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)jo->has_kr);
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
diff --git a/tests/test_pico8_qh.c b/tests/test_pico8_qh.c
index a504a66..45b9cec 100644
--- a/tests/test_pico8_qh.c
+++ b/tests/test_pico8_qh.c
@@ -116,7 +116,7 @@ static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
 
 /* 0 absent, 1 false, 2 true. */
 static uint32_t p__state(const pico8_jo_t *jo) {
-    return !jo->has_p_ ? 0u : jo->p_ ? 2u : 1u;
+    return jo->p_ ? 2u : 0u;
 }
 
 /* 1 no target, 3 a target: the bytes the frozen hashes fold. */
@@ -314,11 +314,11 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->ep != NULL));
     h = fnv(h, (uint32_t)(jo->jy != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
-    h = fnv(h, (uint32_t)jo->has_fx);
+    h = fnv(h, (uint32_t)(jo->fx != 0));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)jo->has_jh);
-    h = fnv(h, (uint32_t)jo->has_p_);
+    h = fnv(h, (uint32_t)(jo->p_ != 0));
     h = fnv(h, (uint32_t)jo->has_kr);
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
diff --git a/tests/test_pico8_struct_header.c b/tests/test_pico8_struct_header.c
index 8f9ece5..7135885 100644
--- a/tests/test_pico8_struct_header.c
+++ b/tests/test_pico8_struct_header.c
@@ -79,6 +79,8 @@ static void assert_links(const pico8_struct_header_t *h, pico8_struct_header_t w
     assert(h->fg == want.fg);
     assert(h->mt == want.mt);
     assert(h->ma == want.ma);
+    assert(h->fx == want.fx);
+    assert(h->p_ == want.p_);
 }
 
 /* Non-carriers, one per constructor that does not go through ke; the mouse
@@ -106,6 +108,7 @@ static void test_carriers_link_their_storage(void) {
     pico8_ie_t *ie = pico8_ie_new(NULL);
     pico8_js_keys_t with_fg = {.has_fg = 1};
     pico8_js_t *js_fg = pico8_js_new(&with_fg);
+    pico8_fq_t *fq = pico8_fq_new(NULL);
     pico8_ui_t *ui = pico8_ui_new();
     pico8_qb_t *qb = pico8_qb_new();
     pico8_fv_t *fv = pico8_fv_new();
@@ -119,6 +122,7 @@ static void test_carriers_link_their_storage(void) {
     assert_links(&js_fg->header,
                  (pico8_struct_header_t){.fa = &js_fg->fa, .ra = &js_fg->ra, .kq = js_fg->header.kq, .fg = &js_fg->fg});
     assert_links(&ie->header, (pico8_struct_header_t){.fa = &ie->def->fa, .ra = &ie->ra, .kq = &ie->def->kq});
+    assert_links(&fq->header, (pico8_struct_header_t){.fa = &fq->def->fa, .p_ = &fq->p_});
     assert_links(&ui->header, (pico8_struct_header_t){.fa = &ui->def->fa, .ql = &ui->def->ql});
     assert_links(&qb->header, (pico8_struct_header_t){.ra = &qb->ra, .p = &qb->p, .ma = &qb->ma});
     assert_links(&fv->g_.header, (pico8_struct_header_t){.fa = &fv->g_.def->fa,
@@ -126,18 +130,24 @@ static void test_carriers_link_their_storage(void) {
                                                          .p = &fv->g_.p,
                                                          .kq = &fv->g_.def->kq,
                                                          .ql = &fv->g_.def->ql,
-                                                         .fg = &fv->g_.def->fg});
+                                                         .fg = &fv->g_.def->fg,
+                                                         .fx = &fv->g_.def->fx,
+                                                         .p_ = &fv->g_.p_});
     assert_links(&jo->header, (pico8_struct_header_t){.fa = &jo->def->fa,
                                                       .ra = (const pico8_vec_t *const *)&jo->ra,
                                                       .p = (const pico8_vec_t *const *)&jo->p,
                                                       .kq = &jo->def->kq,
-                                                      .ma = &jo->ma});
+                                                      .ma = &jo->ma,
+                                                      .fx = &jo->fx,
+                                                      .p_ = &jo->p_});
     assert_links(&fr_jo->header, (pico8_struct_header_t){.fa = &fr_jo->def->fa,
                                                          .ra = (const pico8_vec_t *const *)&fr_jo->ra,
                                                          .p = (const pico8_vec_t *const *)&fr_jo->p,
                                                          .kq = &fr_jo->def->kq,
                                                          .mt = &fr_jo->def->mt,
-                                                         .ma = &fr_jo->ma});
+                                                         .ma = &fr_jo->ma,
+                                                         .fx = &fr_jo->fx,
+                                                         .p_ = &fr_jo->p_});
 }
 
 static pico8_jc_t *jc_dragged_over_nothing(pico8_struct_header_t *qu) {
@@ -163,9 +173,9 @@ static void test_mouse_point_has_null_links(void) {
     assert(point != NULL && point->header.cls == pico8_jc_mouse_point_struct_class());
     assert_links(&point->header, (pico8_struct_header_t){.ra = &point->ra, .p = &point->p});
     assert(!jc->has_mt && jc->fa == pico8_fixed_from_int(4));
-    assert_links(&jc->header, (pico8_struct_header_t){.fa = &jc->fa});
+    assert_links(&jc->header, (pico8_struct_header_t){.fa = &jc->fa, .p_ = &jc->p_});
     assert(from_fr->has_mt && from_fr->mt == pico8_fr_def.mt && from_fr->fa == pico8_fixed_from_int(2));
-    assert_links(&from_fr->header, (pico8_struct_header_t){.fa = &from_fr->fa, .mt = &from_fr->mt});
+    assert_links(&from_fr->header, (pico8_struct_header_t){.fa = &from_fr->fa, .mt = &from_fr->mt, .p_ = &from_fr->p_});
 }
 
 int main(int argc, char **argv) {
```

#### Diff de 2.14d2 (`git diff d1 d2`)

```diff
diff --git a/Makefile b/Makefile
index b20dcc7..7ca5c70 100644
--- a/Makefile
+++ b/Makefile
@@ -871,7 +871,7 @@ composition-residue: tools/composition_residue.sh
 	@sh tools/composition_residue.sh
 
 # Catégories du compteur de composition tombées à 0, séparées par des virgules.
-COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts
+COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6
 
 composition-guard: tools/composition_residue.sh
 	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests
diff --git a/src/pico8_bg.c b/src/pico8_bg.c
index 3d4aaf7..15fa7d9 100644
--- a/src/pico8_bg.c
+++ b/src/pico8_bg.c
@@ -41,20 +41,12 @@ static void pico8_bg_hq_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
     pico8_bg_hq((pico8_bg_t *)self, ra);
 }
 
-static void bg_set_m_(void *self, int m_) {
-    pico8_bg_t *bg = self;
-
-    bg->m_ = m_ != 0;
-    bg->has_m_ = 1;
-}
-
 static const pico8_struct_class_t g_bg_struct_class = {
     .kj = pico8_bg_kj_dispatch,
     .hd = pico8_hd_true,
     .ldown = pico8_bg_ldown_dispatch,
     .rdown = pico8_bg_rdown_dispatch,
     .hq = pico8_bg_hq_dispatch,
-    .set_m_ = bg_set_m_,
 };
 
 static pico8_bg_t *pico8_bg_struct_new(void) {
@@ -64,8 +56,8 @@ static pico8_bg_t *pico8_bg_struct_new(void) {
     self->def = &pico8_bg_def;
     self->header.fa = &self->def->fa;
     self->mq = NULL;
+    self->header.m_ = &self->m_;
     self->m_ = 0;
-    self->has_m_ = 0;
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("bg");
 #endif
diff --git a/src/pico8_bg.h b/src/pico8_bg.h
index 65455af..92d1796 100644
--- a/src/pico8_bg.h
+++ b/src/pico8_bg.h
@@ -19,7 +19,6 @@ typedef struct {
     const pico8_bg_def_t *def;
     pico8_vec_t *mq;
     int m_;
-    int has_m_;
 } pico8_bg_t;
 
 void pico8_bg_init(void);
diff --git a/src/pico8_help.c b/src/pico8_help.c
index 94842c8..896277c 100644
--- a/src/pico8_help.c
+++ b/src/pico8_help.c
@@ -100,13 +100,6 @@ void pico8_fv_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_fv_t *)self, p);
 }
 
-static void fv_set_m_(void *self, int m_) {
-    pico8_fv_t *fv = self;
-
-    fv->m_ = m_ != 0;
-    fv->has_m_ = 1;
-}
-
 static const pico8_struct_class_t g_fv_struct_class = {
     .kj = pico8_fv_kj_dispatch,
     .ev = pico8_dl_dispatch,
@@ -118,7 +111,6 @@ static const pico8_struct_class_t g_fv_struct_class = {
     .emitter_of = pico8_g__emitter_of,
     .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = fv_set_m_,
 };
 
 static pico8_fv_t *pico8_fv_struct_new(void) {
@@ -130,8 +122,8 @@ static pico8_fv_t *pico8_fv_struct_new(void) {
     self->g_.header.kq = &self->g_.def->kq;
     self->g_.header.ql = &self->g_.def->ql;
     self->g_.header.fg = &self->g_.def->fg;
+    self->g_.header.m_ = &self->m_;
     self->m_ = 0;
-    self->has_m_ = 0;
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("fv");
 #endif
diff --git a/src/pico8_help.h b/src/pico8_help.h
index e77d228..a948e8f 100644
--- a/src/pico8_help.h
+++ b/src/pico8_help.h
@@ -18,7 +18,6 @@
 typedef struct {
     pico8_g__t g_;
     int m_;
-    int has_m_; /* 0: `m_` is nil, and is then never read */
 } pico8_fv_t;
 
 /* Builds `bo`; `pico8_help_shutdown()` frees it and is called first by an
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 4b15a54..2038423 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -76,13 +76,6 @@ void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
     pico8_jo_ll((pico8_jo_t *)self, t);
 }
 
-void pico8_jo_set_m_(void *self, int m_) {
-    pico8_jo_t *jo = self;
-
-    jo->m_ = m_ != 0;
-    jo->has_m_ = 1;
-}
-
 static const pico8_struct_class_t g_jo_struct_class = {
     .kj = pico8_jo_kj_dispatch,
     .hd = pico8_jo_hd_dispatch,
@@ -94,7 +87,6 @@ static const pico8_struct_class_t g_jo_struct_class = {
     .cn = pico8_jo_cn_dispatch,
     .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = pico8_jo_set_m_,
 };
 
 void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
@@ -131,8 +123,8 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     self->by = NULL;
     self->mi = self->def->mi;
     self->has_mi = self->def->has_mi;
+    self->header.m_ = &self->m_;
     self->m_ = 0;
-    self->has_m_ = 0;
     self->kw_own = (pico8_kw_t){0};
 }
 
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index 51b7d17..395f64a 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -47,7 +47,6 @@ typedef struct {
     pico8_fixed_t mi;
     int has_mi;
     int m_;
-    int has_m_;
     pico8_kw_t kw_own;
 } pico8_jo_t;
 
@@ -98,10 +97,6 @@ void pico8_jo_kj_dispatch(void *self, const pico8_vec_t *ra);
 pico8_down_t pico8_jo_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
 void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw);
 
-/* The field accessors jo's descriptor holds, for a daughter's `static const`
- * descriptor to name. */
-void pico8_jo_set_m_(void *self, int m_);
-
 /* `e` is one of the ten classes of jo's tree: the only ones whose state
  * `ll` is jo's. */
 int pico8_jo_is_family(const pico8_struct_header_t *e);
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index df56191..3c37c2e 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -50,7 +50,6 @@ static const pico8_struct_class_t g_qh_struct_class = {
     .parent = pico8_jo_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = pico8_jo_set_m_,
 };
 
 void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
@@ -121,7 +120,6 @@ static const pico8_struct_class_t g_nl_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = pico8_jo_set_m_,
 };
 
 static pico8_nl_t *pico8_nl_struct_new(const pico8_jo_keys_t *keys) {
@@ -166,7 +164,6 @@ static const pico8_struct_class_t g_os_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = pico8_jo_set_m_,
 };
 
 static pico8_os_t *pico8_os_struct_new(const pico8_jo_keys_t *keys) {
@@ -231,7 +228,6 @@ static const pico8_struct_class_t g_hj_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = pico8_jo_set_m_,
 };
 
 void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
@@ -319,7 +315,6 @@ static const pico8_struct_class_t g_pm_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = pico8_jo_set_m_,
 };
 
 static pico8_pm_t *pico8_pm_struct_new(const pico8_jo_keys_t *keys) {
@@ -469,7 +464,6 @@ static const pico8_struct_class_t g_ga_struct_class = {
     .emitter_of = ga_emitter_of,
     /* ga's cart list of tags is empty and replaces jo's: never jo's. */
     .tags = 0,
-    .set_m_ = pico8_jo_set_m_,
 };
 
 static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index c181b32..fe8bca6 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -178,7 +178,6 @@ static const pico8_struct_class_t g_nr_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = pico8_jo_set_m_,
 };
 
 /* Initializes an allocated pico8_nr_t under `cls` and `def`, its kind's
@@ -233,7 +232,6 @@ static const pico8_struct_class_t g_fr_struct_class = {
     .parent = pico8_nr_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = pico8_jo_set_m_,
 };
 
 static pico8_fr_t *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
@@ -329,7 +327,6 @@ static const pico8_struct_class_t g_mx_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .set_m_ = pico8_jo_set_m_,
 };
 
 static pico8_mx_t *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
diff --git a/src/pico8_research_icon.c b/src/pico8_research_icon.c
index 8d365a7..f433e8c 100644
--- a/src/pico8_research_icon.c
+++ b/src/pico8_research_icon.c
@@ -175,7 +175,7 @@ void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ie_t *self, pico8_vec
     /* `dl(self.ke)` is not inert: every kind `self.ke` can hold inherits
      * `qh`'s `me="kz"`, so `dl` passes its own state check and draws the
      * kind-level build cost. */
-    if (self->has_m_ && self->m_) {
+    if (self->m_) {
         if (ie_self_ke_kind(self) != NULL) {
             pico8_probe_t pb = {.def = &pico8_probe_def_none};
 
@@ -202,19 +202,11 @@ static int pico8_ie_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_v
     return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
 }
 
-static void ie_set_m_(void *self, int m_) {
-    pico8_ie_t *ie = self;
-
-    ie->m_ = m_ != 0;
-    ie->has_m_ = 1;
-}
-
 static const pico8_struct_class_t g_ie_struct_class = {
     .kj = pico8_ie_kj_dispatch,
     .hd = pico8_ie_hd_dispatch,
     .hb = pico8_ie_hb_dispatch,
     .ldown = pico8_ie_ldown_dispatch,
-    .set_m_ = ie_set_m_,
 };
 
 static pico8_ie_t *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
@@ -231,8 +223,8 @@ static pico8_ie_t *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
     self->jb = NULL;
     self->kh = 0;
     self->has_kh = 0;
+    self->header.m_ = &self->m_;
     self->m_ = 0;
-    self->has_m_ = 0;
     if (keys != NULL) {
         self->ke = keys->ke;
         self->jb = keys->jb;
diff --git a/src/pico8_research_icon.h b/src/pico8_research_icon.h
index 7ba338f..5d4f936 100644
--- a/src/pico8_research_icon.h
+++ b/src/pico8_research_icon.h
@@ -31,7 +31,6 @@ typedef struct {
     int kh;
     int has_kh; /* 0: `kh` is nil, and is then never read */
     int m_;
-    int has_m_; /* 0: `m_` is nil, and is then never read */
 } pico8_ie_t;
 
 /* `self.ke` read as Lua reads a class: no parent, never an entity, never
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 89374a6..955039b 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -79,7 +79,8 @@ typedef struct pico8_struct_class pico8_struct_class_t;
  * embeds first. `me` and `t` are ke's (`me` NULL is nil). A component link
  * points at the field's storage, the archetype's for a field the class
  * never writes; NULL: the class has no such field, or it is nil. `ma` is
- * written through, by `pico8_ma`, and `p_`, by `d_`. `*fx` 0 is nil. */
+ * written through, by `pico8_ma`, `p_`, by `d_`, and `m_`, by `ui`'s
+ * hover. `*fx` 0 is nil. */
 typedef struct {
     const pico8_struct_class_t *cls;
     const char *me;
@@ -94,6 +95,7 @@ typedef struct {
     pico8_fixed_t *ma;
     const pico8_fixed_t *fx;
     int *p_;
+    int *m_;
 } pico8_struct_header_t;
 
 /* Every constructor calls it first: its `malloc` leaves the header
@@ -153,9 +155,6 @@ struct pico8_struct_class {
     const pico8_struct_class_t *(*parent)(void);
     struct pico8_emitter *(*emitter_of)(void *self);
     struct pico8_probe *(*probe_of)(void *self);
-    /* NULL: the class has no `m_`, for every instance. Also marks `m_`
-     * present; only ever called as `e->cls->set_m_(e, m_)`. */
-    void (*set_m_)(void *self, int m_);
 };
 
 /* The construction keys of the jo tree (the cart's `mw(ob)` in `kv:jr`).
diff --git a/src/pico8_ui.c b/src/pico8_ui.c
index 539be6a..f936d13 100644
--- a/src/pico8_ui.c
+++ b/src/pico8_ui.c
@@ -285,15 +285,15 @@ void pico8_ui_gw(pico8_mem_t *mem, pico8_ui_t *self) {
     }
 
     if (self->gj != NULL) {
-        assert(self->gj->cls->set_m_ != NULL &&
+        assert(self->gj->m_ != NULL &&
                "entity: a generic write names a field this struct has no member for -- stop, do not add a member");
-        self->gj->cls->set_m_(self->gj, 0);
+        *self->gj->m_ = 0;
     }
 
     if (self->o_ != NULL) {
-        assert(self->o_->cls->set_m_ != NULL &&
+        assert(self->o_->m_ != NULL &&
                "entity: a generic write names a field this struct has no member for -- stop, do not add a member");
-        self->o_->cls->set_m_(self->o_, 1);
+        *self->o_->m_ = 1;
     }
 
     if (self->o_ != NULL) {
diff --git a/src/pico8_ui_button.c b/src/pico8_ui_button.c
index aa401d5..1883ea3 100644
--- a/src/pico8_ui_button.c
+++ b/src/pico8_ui_button.c
@@ -58,13 +58,6 @@ static void pico8_js_hb_dispatch(void *self) {
     }
 }
 
-static void js_set_m_(void *self, int m_) {
-    pico8_js_t *js = self;
-
-    js->m_ = m_ != 0;
-    js->has_m_ = 1;
-}
-
 static const pico8_struct_class_t g_js_struct_class = {
     .kj = pico8_js_kj_dispatch,
     .ev = pico8_js_ev_dispatch,
@@ -73,7 +66,6 @@ static const pico8_struct_class_t g_js_struct_class = {
     .has_hb = pico8_js_has_hb,
     .ldown = pico8_js_ldown_dispatch,
     .states = {[PICO8_STATE_PH] = pico8_js_ph_dispatch},
-    .set_m_ = js_set_m_,
 };
 
 static pico8_js_t *pico8_js_struct_new(const pico8_js_keys_t *keys) {
@@ -89,8 +81,8 @@ static pico8_js_t *pico8_js_struct_new(const pico8_js_keys_t *keys) {
     self->qf = NULL;
     self->my = NULL;
     self->has_my = 0;
+    self->header.m_ = &self->m_;
     self->m_ = 0;
-    self->has_m_ = 0;
     self->hb = NULL;
     self->fg = 0;
     self->has_fg = 0;
@@ -222,7 +214,7 @@ void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self) {
         return;
     }
 
-    if (self->jb != NULL && self->has_m_ && self->m_) {
+    if (self->jb != NULL && self->m_) {
         pico8_jb_ol(gfx, mem, self->jb, pico8_fixed_from_int(127), pico8_fixed_from_int(1));
     }
 }
@@ -246,7 +238,7 @@ void pico8_js_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self, pic
     qt = pico8_fixed_sub(pico8_fixed_add(p.x, mb), pico8_fixed_from_int(1));
     dn = pico8_fixed_add(p.y, pico8_fixed_from_int(9));
     has_my = self->my != NULL;
-    m_flag = self->has_m_ && self->m_;
+    m_flag = self->m_;
 
     ix_p = pico8_ui_to_int(p.x);
     iy_p = pico8_ui_to_int(p.y);
@@ -299,17 +291,9 @@ static pico8_down_t pico8_fq_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_v
     return pico8_fq_ldown((pico8_fq_t *)self);
 }
 
-static void fq_set_m_(void *self, int m_) {
-    pico8_fq_t *fq = self;
-
-    fq->m_ = m_ != 0;
-    fq->has_m_ = 1;
-}
-
 static const pico8_struct_class_t g_fq_struct_class = {
     .hd = pico8_hd_true,
     .ldown = pico8_fq_ldown_dispatch,
-    .set_m_ = fq_set_m_,
 };
 
 static pico8_fq_t *pico8_fq_struct_new(const pico8_fq_keys_t *keys) {
@@ -321,8 +305,8 @@ static pico8_fq_t *pico8_fq_struct_new(const pico8_fq_keys_t *keys) {
     self->my = NULL;
     self->header.p_ = &self->p_;
     self->p_ = 0;
+    self->header.m_ = &self->m_;
     self->m_ = 0;
-    self->has_m_ = 0;
     if (keys != NULL) {
         self->my = keys->my;
     }
diff --git a/src/pico8_ui_button.h b/src/pico8_ui_button.h
index f1fd347..5ae5fd7 100644
--- a/src/pico8_ui_button.h
+++ b/src/pico8_ui_button.h
@@ -39,7 +39,6 @@ struct pico8_js {
     pico8_js_fn my;
     int has_my; /* 0: nil; 1: present, and a NULL `my` is `false` */
     int m_;
-    int has_m_; /* 0: `m_` is nil, and is then never read */
     pico8_js_fn hb;
     pico8_fixed_t fg;
     int has_fg; /* 0: `fg` is nil, and is then never read */
@@ -125,7 +124,6 @@ struct pico8_fq {
     pico8_fq_my_fn my;
     int p_;
     int m_;
-    int has_m_;
 };
 
 void pico8_fq_init(void);
diff --git a/tests/test_pico8_bg.c b/tests/test_pico8_bg.c
index 765a1d8..d2df4c9 100644
--- a/tests/test_pico8_bg.c
+++ b/tests/test_pico8_bg.c
@@ -47,7 +47,7 @@
 #define LDOWN_G_AFTER 1
 #define LDOWN_MZ_BEFORE 0
 #define LDOWN_MZ_AFTER 1
-#define HOVER_HASH 0xF751E116u
+#define HOVER_HASH 0x477E2027u
 #define DRAW_HASH 0xE18E0472u
 
 static pico8_mem_t mem;
@@ -102,12 +102,9 @@ static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
     return fnv(h, (uint32_t)p->y);
 }
 
-/* 0 absent, 1 false, 2 true. */
+/* 0 absent or false, 2 true. */
 static uint32_t m__state(const pico8_bg_t *bg) {
-    if (!bg->has_m_) {
-        return 0;
-    }
-    return bg->m_ ? 2 : 1;
+    return bg->m_ ? 2 : 0;
 }
 
 static const pico8_qb_t *mo_qb(void) {
@@ -194,7 +191,7 @@ static void check_fresh_bg(const pico8_bg_t *bg) {
     for (i = 0; i < 15; i++) {
         assert(pl->items[i] == want_pl[i]);
     }
-    assert(bg->mq == NULL && !bg->has_m_);
+    assert(bg->mq == NULL && !bg->m_);
     assert(cls->gv == NULL && cls->states[PICO8_STATE_PH] == NULL);
     assert(cls->kj != NULL && cls->ldown != NULL && cls->rdown != NULL && cls->hq != NULL);
     assert(cls->ey == NULL);
@@ -379,7 +376,7 @@ static void test_hover(void) {
     veil = (void *)pico8_fq_new(NULL);
     pico8_ui_lj(&mem, pico8_ok());
     assert((void *)pico8_ok()->o_ == veil);
-    assert(m__state(bg) == 1);
+    assert(m__state(bg) == 0);
     h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
     h = fnv(h, m__state(bg));
     check("hover", h, HOVER_HASH);
diff --git a/tests/test_pico8_fq.c b/tests/test_pico8_fq.c
index ea7aa18..df98bcb 100644
--- a/tests/test_pico8_fq.c
+++ b/tests/test_pico8_fq.c
@@ -27,7 +27,7 @@
 #define MY_TRUE_TICKS 6
 #define MY_TRUE_SEQUENCE_HASH 0xA6A43C27u
 #define BARE_SEQUENCE_HASH 0x52BE382Au
-#define HOVER_SEQUENCE_HASH 0x27C4BCB8u
+#define HOVER_SEQUENCE_HASH 0x689321F9u
 #define HOVER_CURSOR_HASH 0x04C40AB9u
 
 #define LDOWN_TICK 5
@@ -83,7 +83,7 @@ static uint32_t fold_bool_member(uint32_t h, int has, int b) {
 static uint32_t fold_veil(uint32_t h, const pico8_fq_t *e) {
     h = fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)e->header.t);
     h = fold_bool_member(h, (e->p_ != 0), e->p_);
-    h = fold_bool_member(h, e->has_m_, e->m_);
+    h = fold_bool_member(h, (e->m_ != 0), e->m_);
     h = fnv(h, (uint32_t)ents_listed(pico8_hv(), e));
     return fnv(h, (uint32_t)ents_listed(pico8_cv("hd"), e));
 }
@@ -219,7 +219,7 @@ static void test_hover_loop_on_the_veil(void) {
     pico8_ui_lj(&mem, pico8_ok());
     assert(ok_o_() == veil);
     assert(in_layer(14, veil) && !in_layer(15, veil));
-    assert(veil->has_m_ && veil->m_);
+    assert(veil->m_);
     h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
     h = fold_veil(h, veil);
 
@@ -239,7 +239,7 @@ static void test_hover_loop_on_the_veil(void) {
     pico8_he(&veil->header);
     pico8_ui_lj(&mem, pico8_ok());
     assert(ok_o_() != veil && !in_layer(14, veil));
-    assert(veil->has_m_ && !veil->m_);
+    assert(!veil->m_);
     h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
     h = fold_veil(h, veil);
     check("hover", h, HOVER_SEQUENCE_HASH);
diff --git a/tests/test_pico8_fr.c b/tests/test_pico8_fr.c
index 135ea09..bb0e184 100644
--- a/tests/test_pico8_fr.c
+++ b/tests/test_pico8_fr.c
@@ -318,7 +318,7 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)jo->has_mi);
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert(cls_of(e)->kf == cls_of(slipgate)->kf && cls_of(e)->cn == cls_of(slipgate)->cn &&
            cls_of(e)->hq == cls_of(slipgate)->hq && cls_of(e)->ey == cls_of(slipgate)->ey);
     assert(cls_of(e)->kf != NULL && cls_of(e)->cn != NULL && cls_of(e)->hq != NULL && cls_of(e)->ey != NULL &&
diff --git a/tests/test_pico8_fv.c b/tests/test_pico8_fv.c
index d2438a9..0cb4ff8 100644
--- a/tests/test_pico8_fv.c
+++ b/tests/test_pico8_fv.c
@@ -38,7 +38,7 @@
 
 #define BUILT_HASH 0x0B80F384u
 #define LIFE_HASH 0x3BFADEC0u
-#define HOVER_HASH 0x90AFEB68u
+#define HOVER_HASH 0xEE114A89u
 
 static pico8_mem_t mem;
 static pico8_gfx_t gfx;
@@ -74,12 +74,9 @@ static int ents_listed(const pico8_ents_t *list, const void *e) {
     return ents_rank_in(list, e) >= 0;
 }
 
-/* 0 absent, 1 false, 2 true. */
+/* 0 absent or false, 2 true. */
 static uint32_t m__state(const pico8_fv_t *fv) {
-    if (!fv->has_m_) {
-        return 0;
-    }
-    return fv->m_ ? 2 : 1;
+    return fv->m_ ? 2 : 0;
 }
 
 /* The words the frozen hashes fold before a value: 0xF1 a number, 0xB0 a
@@ -354,7 +351,7 @@ static void test_built(void) {
     h = fold_fixed_member(h, g_->header.t);
     h = fnv(h, (uint32_t)size_of(em->qy));
     assert(size_of(em->qy) == 0);
-    assert(!fv->has_m_ && g_->p == NULL && pr->et == NULL && !g_->p_);
+    assert(!fv->m_ && g_->p == NULL && pr->et == NULL && !g_->p_);
     assert(cls_of(fv)->hk == NULL && cls_of(fv)->hb == NULL && cls_of(fv)->states[PICO8_STATE_PH] == NULL &&
            cls_of(fv)->ldown == NULL);
 
@@ -471,7 +468,7 @@ static void test_hover(void) {
     mouse_at(64, 10);
     pico8_ui_lj(&mem, pico8_ok());
     assert((void *)pico8_ok()->o_ != fv);
-    assert(m__state(fv) == 1);
+    assert(m__state(fv) == 0);
     assert(!in_layer(10, fv));
     h = fnv(h, m__state(fv));
     h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
diff --git a/tests/test_pico8_ga.c b/tests/test_pico8_ga.c
index a06ea5d..8a56c96 100644
--- a/tests/test_pico8_ga.c
+++ b/tests/test_pico8_ga.c
@@ -366,7 +366,7 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)(jo->has_mi != 0));
-    h = fnv(h, (uint32_t)(jo->has_m_ != 0));
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert(jo->jy == NULL && !jo->has_kr);
     assert(cls_of(e)->kf == pico8_jo_struct_class()->kf && cls_of(e)->cn == pico8_jo_struct_class()->cn &&
            cls_of(e)->kf != NULL && cls_of(e)->cn != NULL && pico8_jo_is_family(e));
diff --git a/tests/test_pico8_hj.c b/tests/test_pico8_hj.c
index b570021..c76a384 100644
--- a/tests/test_pico8_hj.c
+++ b/tests/test_pico8_hj.c
@@ -390,7 +390,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)jo->has_mi);
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert(ptrs_size(em->qy) == 0);
     assert_qh_members_but_kj(cls_of(e));
     assert(cls_of(e)->kj == pico8_hj_struct_class()->kj && pico8_jo_is_family(e));
diff --git a/tests/test_pico8_ie.c b/tests/test_pico8_ie.c
index 820ecab..07bea4d 100644
--- a/tests/test_pico8_ie.c
+++ b/tests/test_pico8_ie.c
@@ -120,7 +120,7 @@ static uint32_t kh_state(const void *icon) {
 
 static uint32_t m__state(const void *icon) {
     const pico8_ie_t *ie = as_ie((void *)icon);
-    return bool_state(ie->has_m_, ie->m_);
+    return bool_state((ie->m_ != 0), ie->m_);
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -283,7 +283,7 @@ static uint32_t check_fresh_icon(uint32_t h, void *icon, const void *ke, const v
     assert(ie->def->eh.x == pico8_fixed_from_int(118) && ie->def->eh.y == pico8_fixed_from_int(120));
     assert((const void *)ie->ke == ke);
     assert((const void *)ie->jb == jb);
-    assert(!ie->has_kh && !ie->has_m_);
+    assert(!ie->has_kh && !ie->m_);
     assert(cls_of(icon)->states[PICO8_STATE_PH] == NULL && cls_of(icon)->ev == NULL && cls_of(icon)->hk == NULL &&
            cls_of(icon)->rdown == NULL && cls_of(icon)->gv == NULL);
     assert(cls_of(icon)->hd != NULL && cls_of(icon)->hb != NULL && cls_of(icon)->kj != NULL &&
@@ -480,23 +480,20 @@ static void test_draw(void) {
     hidden_px = draw_alone(hidden);
     assert(hidden_px == blank);
     as_ie(hidden)->m_ = 1;
-    as_ie(hidden)->has_m_ = 1;
     assert(m__state(hidden) == 2);
     assert(draw_alone(hidden) == blank);
     shown_px = draw_alone(shown);
     assert(shown_px != blank);
     as_ie(shown)->m_ = 1;
-    as_ie(shown)->has_m_ = 1;
     assert(m__state(shown) == 2);
     hovered_px = draw_alone(shown);
     assert(hovered_px != shown_px);
     as_ie(shown)->m_ = 0;
-    as_ie(shown)->has_m_ = 1;
     assert(draw_alone(shown) == shown_px);
-    cls_of(shown)->set_m_(shown, 1);
+    *as_ie(shown)->header.m_ = 1;
     assert(m__state(shown) == 2);
-    cls_of(shown)->set_m_(shown, 0);
-    assert(m__state(shown) == 1);
+    *as_ie(shown)->header.m_ = 0;
+    assert(m__state(shown) == 0);
 
     h = fnv(h, hidden_px);
     h = fnv(h, shown_px);
@@ -549,10 +546,8 @@ static void test_ke_view(void) {
         uint32_t plain = draw_alone(all[i]);
         uint32_t hovered;
         as_ie(all[i])->m_ = 1;
-        as_ie(all[i])->has_m_ = 1;
         hovered = draw_alone(all[i]);
         as_ie(all[i])->m_ = 0;
-        as_ie(all[i])->has_m_ = 1;
         printf("[ke view] icon %d: 0x%08Xu / 0x%08Xu\n", i, (unsigned)plain, (unsigned)hovered);
         assert(hovered != plain);
         h = fnv(h, plain);
diff --git a/tests/test_pico8_jo.c b/tests/test_pico8_jo.c
index 5ae88cb..8f177e1 100644
--- a/tests/test_pico8_jo.c
+++ b/tests/test_pico8_jo.c
@@ -246,7 +246,7 @@ static uint32_t fold_planet(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)jo->has_mi);
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert_jo_members(cls_of(e));
     assert(jo->def == &pico8_jo_def);
     assert(jo->def == &pico8_jo_def);
@@ -295,7 +295,7 @@ static uint32_t fold_live(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(jo->fo != NULL));
     h = fnv(h, (uint32_t)(jo->nk != NULL));
     h = fnv(h, (uint32_t)(jo->lq != NULL));
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     return h;
 }
 
diff --git a/tests/test_pico8_js.c b/tests/test_pico8_js.c
index 075aba4..be1e60d 100644
--- a/tests/test_pico8_js.c
+++ b/tests/test_pico8_js.c
@@ -71,7 +71,7 @@
 #define FE_MY_HASH 0x459139D2u
 #define CR_MY_HASH 0x7ED641D9u
 #define DRAW_HASH 0x92E72EEBu
-#define JS_MEMBERS_HASH 0x31CE3B93u
+#define JS_MEMBERS_HASH 0x264E8964u
 
 static pico8_mem_t mem;
 static pico8_gfx_t gfx;
@@ -150,13 +150,10 @@ static pico8_js_t *js_of(const void *e) {
     return (pico8_js_t *)e;
 }
 
-/* 0 absent, 1 false, 2 true. */
+/* 0 absent or false, 2 true. */
 static uint32_t m__state(const void *e) {
     const pico8_js_t *b = js_of(e);
-    if (!b->has_m_) {
-        return 0;
-    }
-    return b->m_ ? 2u : 1u;
+    return b->m_ ? 2u : 0u;
 }
 
 /* 0 absent, 1 false, 3 a row. */
@@ -987,12 +984,10 @@ static void test_draw(void) {
     px[0] = draw_alone(plain);
     px[1] = draw_alone(with_my);
     js_of(with_my)->m_ = 1;
-    js_of(with_my)->has_m_ = 1;
     px[2] = draw_alone(with_my);
     px[3] = draw_alone(priced);
     px[4] = draw_alone(tooltip);
     js_of(tooltip)->m_ = 1;
-    js_of(tooltip)->has_m_ = 1;
     px[5] = draw_alone(tooltip);
     px[6] = draw_alone(free_);
     assert(h__state(free_) == 1 && px[6] == px[1]);
@@ -1144,7 +1139,7 @@ static uint32_t fold_js_members(uint32_t h, const void *e) {
     h = fold_member_vec(h, b->ra);
     h = fold_member_vec(h, b->qf);
     h = fold_js_fn(h, b->has_my, b->my);
-    h = b->has_m_ ? fold_bool(fold_kind(h, BOOL_KIND), b->m_) : fnv(h, 0u);
+    h = (b->m_ != 0) ? fold_bool(fold_kind(h, BOOL_KIND), b->m_) : fnv(h, 0u);
     h = fold_js_fn(h, b->hb != NULL, b->hb);
     h = fold_member_fixed(h, b->has_fg, b->fg);
     h = b->px != NULL ? fold_string(fold_kind(h, STRING_KIND), b->px) : fnv(h, 0u);
@@ -1199,10 +1194,9 @@ static void check_le_one_pz(void) {
     check_one_pz(b, le->count);
 }
 
-/* `m_` true, false, or nil (`present` 0), which clears it. */
+/* `m_` true, false, or nil (`present` 0), which reads as false. */
 static void write_m_(pico8_js_t *b, int present, int value) {
     b->m_ = present ? value : 0;
-    b->has_m_ = present;
 }
 
 /* `m_` takes true, false and nil; `lq` a row or nil. The raw writer
@@ -1218,23 +1212,23 @@ static uint32_t fold_written(uint32_t h, void *e, void *other) {
 
     assert(kq_before != NULL);
     write_m_(b, 1, 1);
-    assert(b->has_m_ && b->m_ == 1);
+    assert(b->m_ == 1);
     h = fold_js_members(h, e);
     write_m_(b, 1, 0);
-    assert(b->has_m_ && b->m_ == 0);
+    assert(b->m_ == 0);
     h = fold_js_members(h, e);
     write_m_(b, 1, 1);
     write_m_(b, 1, 0);
-    assert(b->has_m_ && b->m_ == 0);
+    assert(b->m_ == 0);
     h = fold_js_members(h, e);
     write_m_(b, 1, 1);
     write_m_(b, 0, 0);
-    assert(!b->has_m_);
+    assert(!b->m_);
     write_m_(b, 1, 0);
     write_m_(b, 0, 0);
-    assert(!b->has_m_);
+    assert(!b->m_);
     write_m_(b, 0, 0);
-    assert(!b->has_m_);
+    assert(!b->m_);
     h = fold_js_members(h, e);
 
     row = js_of(other)->lq;
@@ -1254,7 +1248,7 @@ static uint32_t fold_written(uint32_t h, void *e, void *other) {
     for (i = 0; i < 15 + 8; i++) {
         h = fnv(h, 0u);
     }
-    assert(b->has_m_ && b->m_ == 1);
+    assert(b->m_ == 1);
     return fold_js_members(h, e);
 }
 
@@ -1396,7 +1390,7 @@ static uint32_t members_title_and_game(void) {
         assert(b->fa == pico8_fixed_from_int(11));
         assert(b->mb == pico8_fixed_add(lq->w, pico8_fixed_from_int(2)));
         assert(b->header.kq != NULL);
-        assert(!b->has_h_ && b->jb == NULL && b->ra == NULL && b->qf == NULL && !b->has_m_);
+        assert(!b->has_h_ && b->jb == NULL && b->ra == NULL && b->qf == NULL && !b->m_);
         assert(!b->has_fg && b->px == NULL && !b->has_trade && !b->has_en && b->_cr_t == NULL);
         assert(b->_jo_fe_pt == NULL && b->_jo_fe_b == NULL && !b->has__jo_fe_od);
         assert(my_state(fresh[k]) == 0 && js_of(fresh[k])->hb == NULL);
diff --git a/tests/test_pico8_mx.c b/tests/test_pico8_mx.c
index 9042328..a51eda2 100644
--- a/tests/test_pico8_mx.c
+++ b/tests/test_pico8_mx.c
@@ -384,7 +384,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)jo->has_mi);
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert(ptrs_size(em->qy) == 0);
     assert_qh_shape(cls_of(e));
     assert_members_of(cls_of(e), hj_cls, 1, 1);
diff --git a/tests/test_pico8_nl.c b/tests/test_pico8_nl.c
index 9c7b440..cc93a21 100644
--- a/tests/test_pico8_nl.c
+++ b/tests/test_pico8_nl.c
@@ -329,7 +329,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)jo->has_mi);
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert(pico8_jo_is_family(e));
     assert_qh_shape(cls_of(e));
     assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 0);
diff --git a/tests/test_pico8_nr.c b/tests/test_pico8_nr.c
index b23d956..843f454 100644
--- a/tests/test_pico8_nr.c
+++ b/tests/test_pico8_nr.c
@@ -333,7 +333,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)jo->has_mi);
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert(cls_of(e)->cn == pico8_nr_struct_class()->cn && cls_of(e)->cn != qh_cls->cn &&
            cls_of(e)->cn != pico8_jo_struct_class()->cn);
     assert(cls_of(e)->kf == qh_cls->kf && cls_of(e)->hq == qh_cls->hq && cls_of(e)->ey == qh_cls->ey &&
diff --git a/tests/test_pico8_os.c b/tests/test_pico8_os.c
index b3186b5..4475994 100644
--- a/tests/test_pico8_os.c
+++ b/tests/test_pico8_os.c
@@ -356,7 +356,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)jo->has_mi);
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert_qh_shape(cls_of(e));
     assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 1);
     assert(cls_of(e)->kf == pico8_qh_struct_class()->kf && cls_of(e)->cn == pico8_qh_struct_class()->cn &&
diff --git a/tests/test_pico8_pm.c b/tests/test_pico8_pm.c
index 45262ec..e3942bf 100644
--- a/tests/test_pico8_pm.c
+++ b/tests/test_pico8_pm.c
@@ -384,7 +384,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)jo->has_mi);
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert(ptrs_size(em->qy) == 0);
     assert_qh_shape(cls_of(e));
     assert_members_of(cls_of(e), pico8_qh_struct_class(), 0, 1);
diff --git a/tests/test_pico8_qh.c b/tests/test_pico8_qh.c
index 45b9cec..da53fe1 100644
--- a/tests/test_pico8_qh.c
+++ b/tests/test_pico8_qh.c
@@ -323,7 +323,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(jo->cc != NULL));
     h = fnv(h, (uint32_t)(jo->by != NULL));
     h = fnv(h, (uint32_t)jo->has_mi);
-    h = fnv(h, (uint32_t)jo->has_m_);
+    h = fnv(h, (uint32_t)(jo->m_ != 0));
     assert_qh_shape(cls_of(e));
     assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 1);
     assert(cls_of(e)->kf == pico8_qh_struct_class()->kf && cls_of(e)->cn == pico8_qh_struct_class()->cn &&
diff --git a/tests/test_pico8_struct_header.c b/tests/test_pico8_struct_header.c
index 7135885..48f372f 100644
--- a/tests/test_pico8_struct_header.c
+++ b/tests/test_pico8_struct_header.c
@@ -81,6 +81,7 @@ static void assert_links(const pico8_struct_header_t *h, pico8_struct_header_t w
     assert(h->ma == want.ma);
     assert(h->fx == want.fx);
     assert(h->p_ == want.p_);
+    assert(h->m_ == want.m_);
 }
 
 /* Non-carriers, one per constructor that does not go through ke; the mouse
@@ -116,13 +117,16 @@ static void test_carriers_link_their_storage(void) {
     pico8_fr_t *fr = pico8_fr_new(NULL);
     pico8_jo_t *fr_jo = &fr->nr.qh.jo;
 
-    assert_links(&bg->header, (pico8_struct_header_t){.fa = &bg->def->fa});
-    assert_links(&js->header, (pico8_struct_header_t){.fa = &js->fa, .ra = &js->ra, .kq = js->header.kq});
+    assert_links(&bg->header, (pico8_struct_header_t){.fa = &bg->def->fa, .m_ = &bg->m_});
+    assert_links(&js->header,
+                 (pico8_struct_header_t){.fa = &js->fa, .ra = &js->ra, .kq = js->header.kq, .m_ = &js->m_});
     assert(js->header.kq != NULL);
     assert_links(&js_fg->header,
-                 (pico8_struct_header_t){.fa = &js_fg->fa, .ra = &js_fg->ra, .kq = js_fg->header.kq, .fg = &js_fg->fg});
-    assert_links(&ie->header, (pico8_struct_header_t){.fa = &ie->def->fa, .ra = &ie->ra, .kq = &ie->def->kq});
-    assert_links(&fq->header, (pico8_struct_header_t){.fa = &fq->def->fa, .p_ = &fq->p_});
+                 (pico8_struct_header_t){
+                     .fa = &js_fg->fa, .ra = &js_fg->ra, .kq = js_fg->header.kq, .fg = &js_fg->fg, .m_ = &js_fg->m_});
+    assert_links(&ie->header,
+                 (pico8_struct_header_t){.fa = &ie->def->fa, .ra = &ie->ra, .kq = &ie->def->kq, .m_ = &ie->m_});
+    assert_links(&fq->header, (pico8_struct_header_t){.fa = &fq->def->fa, .p_ = &fq->p_, .m_ = &fq->m_});
     assert_links(&ui->header, (pico8_struct_header_t){.fa = &ui->def->fa, .ql = &ui->def->ql});
     assert_links(&qb->header, (pico8_struct_header_t){.ra = &qb->ra, .p = &qb->p, .ma = &qb->ma});
     assert_links(&fv->g_.header, (pico8_struct_header_t){.fa = &fv->g_.def->fa,
@@ -132,14 +136,16 @@ static void test_carriers_link_their_storage(void) {
                                                          .ql = &fv->g_.def->ql,
                                                          .fg = &fv->g_.def->fg,
                                                          .fx = &fv->g_.def->fx,
-                                                         .p_ = &fv->g_.p_});
+                                                         .p_ = &fv->g_.p_,
+                                                         .m_ = &fv->m_});
     assert_links(&jo->header, (pico8_struct_header_t){.fa = &jo->def->fa,
                                                       .ra = (const pico8_vec_t *const *)&jo->ra,
                                                       .p = (const pico8_vec_t *const *)&jo->p,
                                                       .kq = &jo->def->kq,
                                                       .ma = &jo->ma,
                                                       .fx = &jo->fx,
-                                                      .p_ = &jo->p_});
+                                                      .p_ = &jo->p_,
+                                                      .m_ = &jo->m_});
     assert_links(&fr_jo->header, (pico8_struct_header_t){.fa = &fr_jo->def->fa,
                                                          .ra = (const pico8_vec_t *const *)&fr_jo->ra,
                                                          .p = (const pico8_vec_t *const *)&fr_jo->p,
@@ -147,7 +153,8 @@ static void test_carriers_link_their_storage(void) {
                                                          .mt = &fr_jo->def->mt,
                                                          .ma = &fr_jo->ma,
                                                          .fx = &fr_jo->fx,
-                                                         .p_ = &fr_jo->p_});
+                                                         .p_ = &fr_jo->p_,
+                                                         .m_ = &fr_jo->m_});
 }
 
 static pico8_jc_t *jc_dragged_over_nothing(pico8_struct_header_t *qu) {
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index ce5ff54..663ebb2 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -130,12 +130,9 @@ static pico8_jo_t *as_jo(void *e) {
     return e;
 }
 
-/* 0 absent, 1 false, 2 true. */
+/* 0 absent or false, 2 true. */
 static uint32_t m__state(const pico8_jo_t *jo) {
-    if (!jo->has_m_) {
-        return 0;
-    }
-    return jo->m_ ? 2 : 1;
+    return jo->m_ ? 2 : 0;
 }
 
 static uint32_t fold_vec(uint32_t h, pico8_vec_t v) {
@@ -415,7 +412,7 @@ static void test_hover(void) {
     mouse_at(x + 40 > 127 ? x - 40 : x + 40, y, 0);
     pico8_ui_lj(&mem, ok);
     assert((void *)ok->o_ != planet);
-    assert(m__state(planet) == 1);
+    assert(m__state(planet) == 0);
     h = fold_entity(h, ok->o_);
     h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
     check("hover", h, HOVER_HASH);
@@ -740,20 +737,15 @@ static int giver_hd(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
     return g_giver_hovered;
 }
 
-static void giver_set_m_(void *self, int m_) {
-    (void)self;
-    (void)m_;
-}
-
 static const pico8_struct_class_t g_giver_class = {
     .hd = giver_hd,
     .ldown = giver_down,
     .rdown = giver_down,
-    .set_m_ = giver_set_m_,
 };
 
 static const pico8_fixed_t g_giver_fa = 0xf0000;
-static giver_t g_giver = {{.cls = &g_giver_class, .fa = &g_giver_fa}};
+static int g_giver_m_;
+static giver_t g_giver = {{.cls = &g_giver_class, .fa = &g_giver_fa, .m_ = &g_giver_m_}};
 
 static void press(void *ok, int buttons) {
     mouse_at(64, 100, 0);
```


### Comptes, mesurés sur la copie

| Mesure | `p` | `c` | `d1` | `d2` |
|---|---|---|---|---|
| `accesseurs-q6` (lignes / fichiers) | 123 / 21 | **80 / 19** | **32 / 12** | **0**, gardé |
| `lien-parent` | 110 / 27 | 110 / 27 | **109 / 27** (`probe.c:282` sort) | 109 / 27 |
| `void-ptr` | 8 / 4 | 8 / 4 | 8 / 4 | **7 / 3** (`planet.h:108` à `p`, `pico8_jo_set_m_`) |
| `parente` | 10 / 3 | 10 / 3 | 10 / 3 | 10 / 3 |
| `défauts` | 0, gardé | 0, gardé | 0, gardé | 0, gardé |
| `inclusion-mutuelle` (paires / modules, lignes du `--check`) | 16 / 15, 40 | 16 / 15, 40 | 16 / 15, 40 | 16 / 15, 40 |
| graphe d'usage (`usegraph.py`) | 19 | 19 | 19 | 19 |
| pied `résidu 2` | 286 / 54 | **243 / 53** | **194 / 50** | **162 / 45** |
| `--champs` | 245 ; 15 ; 20 | **249 ; 15 ; 20** | **243 ; 15 ; 20** | **238 ; 15 ; 20** |
| casts (grep de la réserve 4 de 2.13a) | 500 | **491** | **487** | **487** |
| tests de classe (`->cls`, `header.cls`, `_struct_class()`) | 677 | **669** | **661** | **656** |
| `sizeof(pico8_struct_header_t)` (x86-64) | 48 | **88** | **104** | **112** |
| `COMPOSITION_RESIDUE_GUARD` | 8 catégories | 8 | 8 | **9** (`accesseurs-q6`) |
| constantes `*_HASH` changées | — | 0 | **4** | **4** |

**Attribution d'`accesseurs-q6`** (annexe A de l'étude 2.10 ; chaque ligne sort avec le dernier emplacement qu'elle nomme). `c`, **43** : `help.c` 6, `planet_family.c` 6, `ui.c` 6, `planet_family_trade.c` 5, `struct.h` 4, `ui_button.c` 4, `slipway.c` 3, 2 dans `planet.c`, `research_icon.c`, `T:jc.c`, 1 dans `entity_geom.c`, `planet.h`, `T:ui.c` ; sortent `entity_geom.c` et `T:jc.c` (21 → 19). `d1`, **48** : `planet_family.c` 12, `planet_family_trade.c` 6, 4 dans `msgbox.c`, `planet.c`, `probe.c`, 3 dans `wormhole.c`, 2 dans `help.c`, `planet.h`, `probe.h`, `slipway.c`, `struct.h`, `ui_button.c`, `T:iv.c`, 1 dans `bootstrap.c` ; sortent `bootstrap.c`, `msgbox.c`, `probe.c`, `probe.h`, `slipway.c`, `wormhole.c`, `T:iv.c` (19 → 12). `d2`, **32** : les douze fichiers restants. 43 + 48 + 32 = 123 : l'annexe A de l'étude (43, 48, 32) à la ligne près. La ligne de commentaire de `struct.h` qui nommait `get_fx`, `get_p_` et `set_m_` sort en d1 réécrite sur une ligne qui ne nomme plus que `set_m_`, laquelle sort en d2.

**Pied** : `c` − 43 ; `d1` − 48 − 1 (`lien-parent`) ; `d2` − 32 (`planet.h:108`, `pico8_jo_set_m_`, était compté par `accesseurs-q6` **et** `void-ptr` : une seule ligne du pied).

**`--champs`** : `c` + 5 liens − `js.kq` ; `d1` + 2 liens − `has_fx` (1 type) − `has_p_` (7) ; `d2` + 1 lien − `has_m_` (6). 245 + 4 − 6 − 5 = 238.

**Casts et tests de classe** (mesurés ; motif de la réserve 4 de 2.13a) : `c`, casts 500 → **491**, tests de classe 677 → **669** — partent les casts des accesseurs retirés et les deux de `pico8_ma`, les `->cls->get_kq/ql/fg/mt` et le `pico8_qb_struct_class()` de `pico8_ma` ; le seul cast ajouté est celui du point souris dans le TU `struct_header` (`(pico8_jc_mouse_point_t *)jc->re`) ; `d1`, 491 → **487**, 669 → **661** (implémentations, aiguillage de `probe_host_raise_p_` avec `pico8_jc_struct_class()`, `pico8_mz_struct_class()` ; `planet_trade.c:63` passe de deux casts à un, en réécrivant `has_fx && fx`) ; `d2`, casts inchangés, 661 → **656** (`->cls->set_m_`). Aucun cast neuf dans `src/` (motif appliqué aux lignes `+` de chaque diff).

### Contrat pour 2.15 et la suite (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| noyau à quatorze membres (`cls`, `me`, `t`, les onze liens de 2.14 : `fa`, `ra`, `p`, `kq`, `ql`, `fg`, `mt`, `ma`, `fx`, `p_`, `m_`) | 2.15a | 112 octets ; `emitter`/`probe` se déclarent à la suite, l'init les met à NULL sans changement ; `assert_links` : une ligne de comparaison, et l'attendu des six porteurs d'émetteur et des porteurs de sonde |
| liens mutables : `ma` (`pico8_ma`), `p_` (`d_`), `m_` (`ui:gw`) | toutes | écrits à travers par un seul lecteur générique chacun ; tout autre lien est `const` |
| `probe_host_raise_p_` | 2.15a | ne lit plus que `self->p_` ; `pico8_d_` lit encore `self->cls->probe_of` (`lien-parent`) |
| `pico8_g__of` | 2.15b | `probe.c` ne l'emploie plus que dans `probe_host_set_ra_p` (`probe.c:82`) et sa définition |
| `has_p_`, `has_m_`, `has_fx` | toutes | n'existent plus ; nil et faux sont le même 0 pour `p_`, `m_`, `fx` ; un TU qui veut « faux » plie la vérité |
| TU `struct_header` : relance sous `MALLOC_PERTURB_=90` **et** `GLIBC_TUNABLES=glibc.malloc.tcache_count=0` ; `assert_malloc_fills` lit un bloc recyclé | 2.15a | un porteur neuf s'y construit ; le point souris et `jc` y sont déjà |
| `particle.c:12` (`pico8_planet.h`) | 2.17 | inclusion sans usage depuis 2.14b ; pas une paire |
| arêtes `entity_geom` → `planet` (`pico8_na` : `pico8_jo_is_family`, `pico8_jo_t`) et `entity_geom` → `scroll` (`pico8_gd` : `pico8_qb_t`) | 2.17e | ce sont les seuls symboles restants, comme l'étude 2.11 le demande au départ de 2.17e |

### Anti-tassement

**Interdit** :
- poser un lien ailleurs que dans l'init de son porteur (ou juste après l'init de la famille : `fv`, `fr`), sauf le lien `mt` de `jc`, reposé aux deux écritures de `has_mt` et nulle part ailleurs, et `kq` de `js`, écrit par `js:pq` (G3) ; écrire un lien dans un TU, sauf dans l'initialiseur ou l'init d'un faux type (`g_giver`, `fake_toast`) ;
- un lien de champ A qui vise l'instance (`kq`, `ql`, `fg` de `fv`, `kq` de `ie` et de `jo`, `ql` de `ui`, `mt` de `fr`, `fx` de `g_`/`fv`), un lien de champ écrit par la classe qui vise `def` (`fg` de `js`, `mt` de `jc`, `ma`, `fx` de `jo`, `p_`, `m_`) ;
- copier `js.kq` au lieu de le déplacer ; garder le membre `js.kq` ;
- un lien mutable autre que `ma`, `p_`, `m_` ; un cast pour écrire à travers un lien `const` ;
- garder un accesseur de champ dans `pico8_struct_class_t` « pour les TU », en ajouter un ; garder `has_fx`, `has_p_` ou `has_m_` ;
- changer une constante `*_HASH`/`FROZEN_*` autre que les huit nommées (quatre en d1, quatre en d2), ou l'une d'elles sans la réponse de l'humain (Question) ; changer l'une d'elles pour une valeur autre que celle que le parent rend avec le seul pli réécrit (preuve 6) ;
- toucher `emitter_of`, `probe_of`, `parent`, `pico8_g__of`, `pico8_jo_is_family` hors de `probe_host_raise_p_` et `pico8_ma` (2.15) ;
- retirer ou ajouter une ligne `#include` dans `src/` (2.17) ; toucher `particle.c` ;
- changer un `*.sha256`, `tests/e2e/`, `tools/` ; toucher le `Makefile` hors d2.

## Implementation Chunks

Trois commits, dans l'ordre, chacun vert. **≈ 50 min chacun**, preuves comprises : ≈ 10 min pour appliquer le diff, compiler, `make test`, `make e2e` (aussi sous `MALLOC_PERTURB_=165`), compteur ; les jumelles en arrière-plan, ≈ 10 à 17 s l'une (c : 32 lancements et 3 lignes K, ≈ 9 min ; d1 : 26 et 1, ≈ 7 min ; d2 : 16 et 1, ≈ 4 min) ; ≈ 15 min pour les preuves 2, 4 à 10 (`sanitize-net` ≈ 1 min, `line-coverage` des 26 e2e ≈ 2 min). Le plan donne ≈ 5 × 50 min pour 2.14 : a et b en ont pris deux, c, d1, d2 prennent les trois autres.

1. **2.14c** : appliquer le diff de c ; `make test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e`, compteur, `--champs`, taille ; preuves 1 à 5, 8, 9 (jumelles KQ, QL, FG, MT, MA, N, I-jc ; K-mp à `p` et à `c`, K-init-c ; réserves 1 à 4 du vérificateur de 2.14a/b) ; note « Découvert à 2.14c » ; commit. **Ne dépend pas de la question.**
2. **Question** : poser la question (Further Notes) avant d1 ; sans réponse « oui », s'arrêter après c (rapporter).
3. **2.14d1** : idem avec le diff de d1 (jumelles FX, P, N, H ; K-init-d1) et la preuve 6 (quatre constantes) ; note ; commit.
4. **2.14d2** : idem avec le diff de d2 (jumelles M, N, H ; K-init-d2 ; garde mordante) et la preuve 6 (quatre constantes) ; note ; commit.

**Repli écrit** (si un commit déborde de 1,5 fois son budget avant d'être fait), chacun un commit vert :
- c en **c1** (`kq`, `ql`, `fg` : `accesseurs-q6` → 88) puis **c2** (`mt`, `ma`, le test du point souris : → 80) ;
- d1 en **d1a** (`fx`, `has_fx` : → 62, aucune constante changée) puis **d1b** (`p_`, `has_p_`, les quatre constantes : → 32, `lien-parent` → 109) ;
- d2 ne se découpe pas (`set_m_` et `has_m_` partent ensemble, étude 2.10 §1 : un lien qui écrit `m_` sans `has_m_` rendrait faux les trois lecteurs).
Comptes du repli calculés sur l'attribution ci-dessus, non mesurés ; le repli, s'il sert, est écrit dans la note « Découvert à ».

## Testing Decisions

- **Pas de TU neuf** : `tests/test_pico8_struct_header.c` (2.14a) s'étend (dix, puis onze liens dans `assert_links` ; huit porteurs de plus ; le point souris et deux `jc` construits par `jc:hq` ; tcache coupé et bloc recyclé vérifié — réserves 1 à 4 du vérificateur de 2.14a/b) ; `tests/test_pico8_jc.c` gagne `test_hq_takes_mt_from_its_start` (G2). Le reste est réécriture : accesseurs appelés par les TU en épingles de lien, plis `has_X` en plis de vérité. Aucun rejeu d'`events.json` dans un TU.
- **Bon test ici** : une épingle compare le lien à l'adresse du stockage que 2.9 fixe (`h->kq == &ie->def->kq`, `h->fg == &js->fg`), ou à NULL pour un non-porteur ; jamais une valeur lue par le lien à la même valeur lue par le membre.
- **Prior art** : PRD 2.14ab (jumelles, lignes K, TU sous perturbation, programme `obs.c`) ; étude 2.10, annexe A (attribution d'`accesseurs-q6`) ; règle du parent (D.21, PRD 2.13).

### Preuves (parent de `c` = le commit 2.14b ; de `d1` = 2.14c ; de `d2` = 2.14d1)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `MALLOC_PERTURB_=165 make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide, aux trois commits. `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` : vide à `c` ; à `d1` et `d2`, exactement les lignes des huit constantes nommées (preuve 6), sous réserve de la réponse à la question. Mesuré (Runs 12).

2. **Preuve `const`** (`d2`) : `make test` compile tout sous `-Werror` ; preuve négative (`neg.c`, `cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only`) : `e->kq->xl = 0`, `*e->ql = 0`, `*e->fg = 0`, `*e->mt = 0`, `*e->fx = 0` sur un `pico8_struct_header_t *e` → **5** erreurs `assignment of member 'xl' in read-only object` / `assignment of read-only location` ; `*e->ma = 0`, `*e->p_ = 1`, `*e->m_ = 1` compilent. `s->header.ma = &s->def->ma` (`neg2.c`) → `assignment discards 'const' qualifier` (`-Werror=discarded-qualifiers`) : un lien mutable ne peut pas viser l'archétype. Un lien de champ A vise `def` : grep de Sortie 1. Mesuré (Runs 14).

3. **Mutations, règle du parent.** Une jumelle par porteur et par groupe (« lien laissé NULL sur un porteur ») et par non-porteur essayé (« lien posé sur un non-porteur ») : au parent, la mutation équivalente sur la table (entrée `get_X`/`set_m_` mise à `0 ? f : NULL`, qui garde la fonction nommée ; ou une entrée neuve ; pour `ma`, la branche de l'`assert` et du cast de `pico8_ma`) ; à l'enfant, sur la pose du lien. Lignes H (retrait de `has_X`) : l'état initial du champ posé vrai sans son `has_X` (au parent, invisible derrière `has_X` = 0 ; à l'enfant, la valeur seule porte la vérité). `make -k -j8 unit` puis `make -k e2e` ; restaurer par `git checkout -f`. **Vert à l'enfant et rouge au parent = ÉCHEC** ; plus précisément, ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant. Lignes K : enfant seul, `make e2e` normal et sous `MALLOC_PERTURB_=165` (K-mp aussi au parent). Script : `mut.py` + `muts.json` (hors dépôt).

**Jumelles de 2.14c** (parent `p`, enfant `c`) :

| # | Cible | Parent : mutation | Rouges au parent | Enfant : mutation | Rouges à l'enfant | e2e p / e | Verdict |
|---|---|---|---|---|---|---|---|
| KQ-fv | `kq`, `fv` (A) | `.get_kq = NULL` | `fv` | pose retirée | `fv`, `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| KQ-ie | `kq`, `ie` (A) | `.get_kq = NULL` | `ie` | pose retirée | `ie`, `struct_header` | 17 / 17 | conforme (+`struct_header`) |
| KQ-jo | `kq`, arbre `jo` | entrée NULL dans les dix tables | `ga`, `jo`, `ui` | pose retirée | `ga`, `jo`, `struct_header`, `ui` | 10 / 10 | conforme (+`struct_header`) |
| KQ-js | `kq`, `js` | `.get_kq = NULL` | `hp`, `lq`, `replay_from_title`, `ui` | pose retirée | `hp`, `js`, `lq`, `replay_from_title`, `struct_header`, `ui` | 0 / 0 | conforme (+`js`, `struct_header`) |
| QL-fv | `ql`, `fv` (A) | `.get_ql = NULL` | `fv` | pose retirée | `fv`, `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| QL-ui | `ql`, `ui` (A) | `.get_ql = NULL` | `ui` | pose retirée | `struct_header`, `ui` | 26 / 26 | conforme (+`struct_header`) |
| FG-fv | `fg`, `fv` (A) | `.get_fg = NULL` | `fv` | pose retirée | `fv`, `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| FG-js | `fg`, `js` | `.get_fg = NULL` | aucun | pose retirée | `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| MT-fr | `mt`, `fr` (A) | `.get_mt = NULL` | `fr` | pose retirée | `fr`, `jc`, `struct_header` | 25 / 25 | conforme (+`jc`, `struct_header`) |
| MT-jc | `mt`, `jc` | `.get_mt = NULL` | `jc` | pose retirée | `jc`, `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| MA-jo | `ma`, arbre `jo` | branche famille de l'`assert`/du cast de `pico8_ma` retirée | `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `jc`, `js`, `lq`, `mx`, `nr`, `planet_kj_dispatch`, `pm`, `replay_from_title` | pose retirée | `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `jc`, `js`, `lq`, `mx`, `nr`, `planet_kj_dispatch`, `pm`, `replay_from_title`, `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| MA-qb | `ma`, `qb` | branche `qb` de l'`assert` retirée | `bg`, `fq`, `fr`, `fv`, `g_`, `ga`, `headless_capture`, `hj`, `hp`, `hy`, `iv`, `jc`, `jo`, `js`, `lq`, `mh`, `mx`, `mz`, `nf`, `nl`, `nr`, `os`, `pm`, `qb`, `qh`, `replay_from_title`, `trade`, `ui` | pose retirée | `bg`, `fq`, `fr`, `fv`, `g_`, `ga`, `headless_capture`, `hj`, `hp`, `hy`, `iv`, `jc`, `jo`, `js`, `lq`, `mh`, `mx`, `mz`, `nf`, `nl`, `nr`, `os`, `pm`, `qb`, `qh`, `replay_from_title`, `struct_header`, `trade`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| N-bg-kq | `bg` reçoit `kq` | entrée `get_kq` neuve | aucun | lien vers une boîte constante | `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| N-bg-ql | `bg` reçoit `ql` = 9 | entrée `get_ql` neuve | `ui` | lien vers une constante | `struct_header`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| N-bg-fg | `bg` reçoit `fg` = 5 | entrée `get_fg` neuve | `bg`, `ui` | idem | `bg`, `struct_header`, `ui` | 26 / 26 | conforme (+`struct_header`) |
| N-jo-mt | `jo` reçoit `mt` = 1 | entrée `get_mt` neuve | `jc` | idem | `jc`, `struct_header` | 17 / 17 | conforme (+`struct_header`) |
| I-jc | `fa` de `jc` (I) | lien `&self->def->fa` | aucun | idem | `struct_header` | 25 / 25 | conforme (+`struct_header`) |

**Jumelles de 2.14d1** (parent `c`, enfant `d1`) :

| # | Cible | Parent : mutation | Rouges au parent | Enfant : mutation | Rouges à l'enfant | e2e p / e | Verdict |
|---|---|---|---|---|---|---|---|
| FX-g_ | `fx`, `g_` et `fv` (A) | entrée NULL dans les deux tables | `mz` | pose retirée | `mz`, `struct_header` | 9 / 9 | conforme (+`struct_header`) |
| FX-jo | `fx`, arbre `jo` | entrée NULL dans les dix tables | `mz` | pose retirée | `mz`, `struct_header` | 24 / 24 | conforme (+`struct_header`) |
| P-fq | `p_`, `fq` | `.get_p_ = NULL` | `fq` | pose retirée | `fq`, `struct_header` | 25 / 25 | conforme (+`struct_header`) |
| P-iv | `p_`, `iv` | `.get_p_ = NULL` | `iv`, `jc` | pose retirée | `iv`, `jc` | 16 / 16 | conforme |
| P-mh | `p_`, `mh` | `.get_p_ = NULL` | `mh` | pose retirée | `mh` | 20 / 20 | conforme |
| P-jc | `p_`, `jc` | `.get_p_ = NULL` | aucun | pose retirée | `jc`, `struct_header` | 24 / 0 | conforme (+`jc`, `struct_header`) |
| P-mz | `p_`, `mz` | `.get_p_ = NULL` | `mz` | pose retirée | `mz` | 13 / 0 | conforme |
| P-g_ | `p_`, `g_` et `fv` (A) | entrée NULL dans les deux tables | `g_` | pose retirée | `g_`, `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| P-jo | `p_`, arbre `jo` | entrée NULL dans les dix tables | `ga`, `hp`, `jo`, `qh` | pose retirée | `ga`, `hp`, `jo`, `qh`, `struct_header` | 22 / 0 | conforme (+`struct_header`) |
| N-bg-fx | `bg` reçoit `fx` vrai | entrée `get_fx` neuve | `mz` | lien vers une constante 1.0 | `mz`, `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| N-qb-p_ | `qb` reçoit `p_` vrai | entrée `get_p_` neuve | `bg`, `fv`, `g_`, `hp`, `jc`, `js`, `mz`, `nf`, `nr`, `pm`, `qb`, `replay_from_title`, `trade`, `ui` | lien vers un `int` 1 | `bg`, `fv`, `g_`, `hp`, `jc`, `js`, `mz`, `nf`, `nr`, `pm`, `qb`, `replay_from_title`, `struct_header`, `trade`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| H-fx | `fx` de `jo` | `_new` : `fx = 1` (`has_fx` 0) | aucun | `_new` : `fx` = 1.0 | `fr`, `ga`, `hj`, `hp`, `jo`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh` | 26 / 8 | conforme (+`fr`, `ga`, `hj`, `hp`, `jo`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh`) |
| H-p_ | `p_` de `js` | `_new` : `p_ = 1` (`has_p_` 0) | aucun | `_new` : `p_ = 1` | `fq`, `js` | 26 / 23 | conforme (+`fq`, `js`) |

**Jumelles de 2.14d2** (parent `d1`, enfant `d2`) :

| # | Cible | Parent : mutation | Rouges au parent | Enfant : mutation | Rouges à l'enfant | e2e p / e | Verdict |
|---|---|---|---|---|---|---|---|
| M-bg | `m_`, `bg` | `.set_m_ = NULL` | `bg`, `qb`, `replay_from_title`, `ui` | pose retirée | `bg`, `qb`, `replay_from_title`, `struct_header`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| M-fv | `m_`, `fv` (A) | `.set_m_ = NULL` | `fv` | pose retirée | `fv`, `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| M-ie | `m_`, `ie` (A) | `.set_m_ = NULL` | `ie` | pose retirée | `ie`, `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| M-js | `m_`, `js` | `.set_m_ = NULL` | `fq`, `ga`, `hp`, `js`, `lq`, `replay_from_title`, `ui` | pose retirée | `fq`, `ga`, `hp`, `js`, `lq`, `replay_from_title`, `struct_header`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| M-fq | `m_`, `fq` | `.set_m_ = NULL` | `bg`, `fq`, `js`, `replay_from_title` | pose retirée | `bg`, `fq`, `js`, `replay_from_title`, `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| M-jo | `m_`, arbre `jo` | entrée NULL dans les dix tables | `bg`, `ui` | pose retirée | `bg`, `struct_header`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| N-mz-m_ | `mz` reçoit `m_` | entrée `set_m_` neuve (sans effet) | aucun | lien vers un `int` | aucun | 26 / 26 | conforme |
| H-m_ | `m_` de `js` | `_new` : `m_ = 1` (`has_m_` 0) | aucun | `_new` : `m_ = 1` | `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `js`, `lq`, `mx`, `nr`, `pm` | 26 / 0 | conforme (+`fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `js`, `lq`, `mx`, `nr`, `pm`) |

**Lignes K** (e2e normal, puis sous `MALLOC_PERTURB_=165`) :

| # | Mutation | Commit | Rouges | e2e | e2e perturbé |
|---|---|---|---|---|---|
| K-mp@p | le point souris pose `cls` sans l'init | `p` | aucun | 26/26 | 26/26 |
| K-mp@c | idem | `c` | `struct_header` | 17/26 | 17/26 |
| K-init-c | init réduite à `cls`, `fa`, `ra`, `p` (oublie les cinq liens de c) | `c` | `bg`, `hp`, `hy`, `jc`, `js`, `lq`, `mz`, `qb`, `replay_from_title`, `struct_header`, `trade`, `ui` | 0/26 | 0/26 |
| K-init-d1 | init qui oublie `fx`, `p_` | `d1` | `bg`, `fq`, `fr`, `fv`, `g_`, `ga`, `headless_capture`, `hj`, `hp`, `hy`, `ie`, `iv`, `jc`, `jo`, `js`, `lq`, `mh`, `mx`, `mz`, `nf`, `nl`, `nr`, `os`, `pm`, `qb`, `qh`, `replay_from_title`, `spec_defaults`, `struct_header`, `trade`, `ui` | 0/26 | 0/26 |
| K-init-d2 | init qui oublie `m_` | `d2` | `struct_header` | 26/26 | 26/26 |

   Lecture : **aucun ÉCHEC** (38 jumelles, 76 lancements ; 5 lignes K). **MT-jc** était un ÉCHEC au premier essai (parent : `jc` rouge, par l'appel direct de `get_mt` dans `check_fresh_jc` ; enfant : `jc` vert, aucun `jc` du TU ne tirait d'un `fr`) : `test_hq_takes_mt_from_its_start` le corrige (G2), rouge à l'enfant sous la mutation (Runs 6, 13). `struct_header` s'ajoute à l'enfant pour les types qu'il construit. L'enfant rougit plus que le parent là où le lien remplace un accesseur que seuls des chemins de jeu lisaient : **P-jc** (e2e 24 → 0), **P-mz** (13 → 0), **P-jo** (22 → 0) — `d_` écrivait par l'aiguillage au parent, par le lien à l'enfant ; **FG-js**, **N-bg-kq** (verts au parent, `struct_header` à l'enfant). **Lignes H** : vertes au parent, rouges à l'enfant (**H-fx** : douze TU, e2e 8 ; **H-p_** : `fq`, `js`, e2e 23 ; **H-m_** : onze TU, e2e 0) — le retrait de `has_X` est observé : la valeur seule décide. **N-mz-m_** : verte aux deux bouts, e2e 26 : aucun lecteur n'observe `m_` sur `mz` (jamais survolé par `ui:gw` dans les 26 e2e) et `struct_header` ne construit pas `mz` (comme N-qb-fa, N-fq-ra à 2.14ab, G7 de ce PRD-là). **K-mp** : vert à `p` (comme à 2.14ab), **rouge à `c`** (`struct_header`, e2e 17/17) : `jc_mt_of` lit désormais `re->mt` sur le point souris, et le TU le construit. **K-init-d2** : seul `struct_header` rougit (un `m_` indéterminé n'est suivi par aucun lecteur de non-porteur) ; à `c` et `d1`, l'oubli casse le jeu (e2e 0, même sans perturbation).

4. **Chemin touché** : `make line-coverage EVENTS="<les 26 e2e>"` sur `d2` (6064 lignes) : des lignes exécutables ajoutées dans `src/` par les trois diffs (`git diff -U0 p d2 -- src`, numéros de `d2`), **47 sur 47 exécutées** : `bg.c` 59 ; `bootstrap.c` 70 ; `entity_geom.c` 37, 74, 75 ; `help.c` 122-125 ; `msgbox.c` 100, 230 ; `planet.c` 100-103, 126, 358 ; `planet_family_trade.c` 242 ; `planet_trade.c` 63, 64 ; `probe.c` 282, 390, 391 ; `research_icon.c` 178, 220, 226 ; `scroll.c` 41 ; `slipway.c` 46, 47, 50, 127, 372, 375 ; `ui.c` 90, 235, 288, 294, 300 ; `ui_button.c` 84, 125, 179, 217, 241, 306, 308 ; `wormhole.c` 44, 233 — toutes les poses, `pico8_gk`, `pico8_ma`, `jc_mt_of`, `ui:ev`/`ui:gw`, `dg`, `mz:pq`, `probe_host_raise_p_` et les lecteurs de `m_` (Runs 14).

5. **Valeurs observables, parent contre enfant** : un programme d'essai (`obs.c`, hors dépôt), lié aux objets de `src/` (sans `main`), boote le cart, crée une instance fraîche de 30 types (`bg`, `ui`, `js`, `js` avec `fg`, `fq`, `nf`, `qb`, `mh`, `iv`, `ie`, `pj`, `mz`, `g_`, `fv`, les dix sortes, `lq`, `jb`, `hp`, `hy`, `trade`, `jc`), deux `jc` tirés par `jc:hq` (d'un `jo`, d'un `fr`) et le point souris, et imprime ce que lit le code générique : `kq`, `ql`, `fg`, `mt`, `ma` (absent ou valeur), la vérité de `fx` et `p_`, la présence de `m_` ; puis trois pas de `pico8_ma` sur une planète, sur la caméra et sur NULL, et `pico8_gk` sur `js` et `jo` en six points — **46 lignes**. Compilé `-DV=0` (accesseurs) sur `p`, `-DV=1` sur `c`, `-DV=2` sur `d1`, `-DV=3` sur `d2` : sorties **identiques** (46 lignes, même `md5`), et identiques sous `MALLOC_PERTURB_=165` aux quatre arbres (Runs 15).

6. **Les huit constantes : le pli seul les change.** Au parent de chaque sous-étape, on applique **seulement** les hunks des TU concernés (les plis réécrits et les constantes neuves ; pour `test_pico8_mz.c`, sans le hunk de `test_ox`, qui dépend de `jo.fx` en fixe), puis on lance ces TU : à `c` (parent de `d1`), `fq`, `ga`, `mz` sont **verts** avec `BARE_SEQUENCE_HASH` pour « my false », `LIFE_HASH` = `0x196B693Au`, `EY_HASH` = `0x52EB8E18u`, `PR_HASH` = `0x85EB1EC6u` ; à `d1` (parent de `d2`), `bg`, `fq`, `fv`, `js` sont **verts** avec `HOVER_HASH` = `0x477E2027u` (`bg`), `HOVER_SEQUENCE_HASH` = `0x689321F9u` (`fq`), `HOVER_HASH` = `0xEE114A89u` (`fv`), `JS_MEMBERS_HASH` = `0x264E8964u` (`js`, sans le hunk de `test_draw`, qui retire deux écritures de `has_m_` que le dessin du parent lit encore) (Runs 7, 14). Le parent, qui a encore `has_p_`/`has_m_`, rend donc exactement les valeurs neuves dès que le pli ne lit plus `has_X` : l'écart vient du pli (l'état « faux présent » replié sur nil), pas du code du jeu. Les états concernés : `fq` `my` faux (`p_` faux, séquence désormais égale à `BARE`, la constante `MY_FALSE_SEQUENCE_HASH` part) ; `mz:ey` payé (`p_` faux) ; `ga` coupé (`p_` faux) ; survols quittés de `bg`, `fq`, `fv` (`m_` faux) ; `js` écrit à faux par le TU.

7. **Nil et faux, côté cart** : les lectures de `p_`, `m_`, `fx` dans `pico8/lua.lua` sont toutes en vérité — `if qc.p_ then` (`:177`, `dg`), `self.jb and self.m_ then` (`:268`), `self.m_ and 12 or 13` (`:273`), `if self.m_ then` (`:937`), `return p.fx` (`:765`, prédicat du filtre `jp` de `mz:pq`), `if (sw:eb(self).fx)` (`:466`). `fx` n'est jamais écrit à faux (`self.fx=true`, `fx=1`) ; `p_` et `m_` le sont (`self.p_=t>60`, `(self.gj or {}).m_=false`). Aucune valeur du jeu ne dépend de la distinction : preuve 5 et e2e.

8. `class-coverage-net` : **oui**, vert à `c`, `d1`, `d2` (`couverture: OK`). `sanitize-net` : **oui** (le noyau grandit : 48 → 88 → 104 → 112 octets), `sanitize: 26 propre(s)` aux trois (Runs 14).

9. **Garde mordante** (`d2`) : `make composition-guard` vert ; une ligne `int (*get_fx)(const void *self);` remise dans `pico8_struct_class_t` → `résidu gardé (…,défauts,accesseurs-q6) : 1 ligne(s)`, `make` en échec (code 2). Et le TU `struct_header` (réserve 2) : lancé avec `PICO8_MALLOC_FILLS=1` (sans relance, donc sans perturbation), il s'arrête sur `assert_malloc_fills` ; lancé avec `PICO8_MALLOC_FILLS=1 MALLOC_PERTURB_=90` sans `GLIBC_TUNABLES` (le tcache rend le bloc recyclé non rempli), il s'y arrête aussi ; par sa relance (tcache coupé), il passe : code 134 (`Aborted`) dans les deux premiers cas, vert dans le troisième (Runs 11, 14). Programme `tc.c` (hors dépôt, `malloc(64)`, mise à zéro, `free`, `malloc(64)`) : même bloc, **0/64** octets à `0xa5` sous `MALLOC_PERTURB_=90`, **64/64** avec `GLIBC_TUNABLES=glibc.malloc.tcache_count=0`.

10. **Graphe d'usage** (`usegraph.py`, annexe A du PRD 2.11) : 19 aux quatre arbres ; `inclusion-mutuelle` 16 / 15 (40 lignes) aux quatre.

### Greps de Sortie

À `d2` (numéros de `d2`) ; les lignes de `c`/`d1` sont le sous-ensemble de leurs groupes :
1. `grep -rnE 'header\.(kq|ql|fg|mt|fx) = ' src` → **13** lignes : `&…def->X` pour les champs A — `help.c:122-124` (`fv` : `kq`, `ql`, `fg`), `planet.c:100` (`kq`, arbre `jo`), `research_icon.c:220` (`kq`), `ui.c:90` (`ql`), `planet_family_trade.c:242` (`mt` de `fr`), `probe.c:390` (`fx`, `g_`/`fv`) ; l'instance pour les champs écrits par la classe — `planet.c:102` (`fx` de `jo`), `ui_button.c:125` (`fg` de `js`, sous `has_fg`), `slipway.c:127` et `:372` (`mt` de `jc`, aux deux écritures de `has_mt`), `ui_button.c:179` (`kq` de `js`, écrit par `js:pq`).
2. `grep -rnE 'header\.(ma|p_|m_) = ' src` → **15** lignes, toutes `&self->X` (`&self->m_` pour `fv`) : `ma` 2 (`planet.c`, `scroll.c`), `p_` 7 (`msgbox.c` ×2, `planet.c`, `probe.c`, `slipway.c`, `ui_button.c`, `wormhole.c`), `m_` 6 (`bg.c`, `help.c`, `planet.c`, `research_icon.c`, `ui_button.c` ×2).
3. `grep -rnE '\b(get_kq|get_ql|get_fg|get_mt|get_fx|get_p_|set_m_)\b' src tests | wc -l` → 58 à `c`, 24 à `d1`, **0** à `d2` ; `grep -rnE '\bhas_(fx|p_)\b' src tests` → vide à `d1` ; `grep -rnE '\bhas_(fx|p_|m_)\b' src tests` → vide à `d2`.
4. `grep -n kq src/pico8_ui_button.h` → vide (le membre `js.kq` est parti) ; `grep -nE 'pico8_qb_struct_class|pico8_jo_is_family|pico8_jo_t|pico8_qb_t' src/pico8_entity_geom.c` → `:46` (`pico8_qb_t`, `pico8_gd`), `:58`, `:59` (`pico8_na`) : plus rien dans `pico8_ma`.
5. `sed -n '/^static void probe_host_raise_p_/,/^}/p' src/pico8_probe.c` → l'`assert` de présence et `*self->p_ = 1;`, rien d'autre.
6. `git diff <parent> -- src | grep -E '^[-+]#include'` → vide aux trois commits ; `grep -rnE '(\.|->)cls *= [^=]' src tests | grep -v '{'` → exactement `tests/test_pico8_planet_kj_dispatch.c:91` et `:128` ; `grep -rn 'pico8_struct_header_init(' src | wc -l` → 9.
7. `make composition-residue` et `--champs` → la table « Comptes » ; `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → `…,défauts,accesseurs-q6` à `d2`.
8. `git diff --stat <parent>` : les fichiers de la section « Modules » et `docs/plan-3.md`, rien d'autre ; `git diff --stat p d2 -- tests/e2e tools '*.sha256'` vide ; le `Makefile` à `d2` seulement.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd214cd/copie`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | Premier essai (interrompu) : arbres `wtA`/`wtB` et copie inutilisables (lien `.git` vers une copie supprimée) ; `rm -rf` ; aucun résultat repris sans relance | — | 0 | supprimés | < 1 s |
| 2 | `git clone --no-hardlinks` local, `checkout 2ba060c` ; diffs du PRD 2.14ab extraits (lignes 117-982, 988-1821), `cmp` avec ceux du premier essai ; `git apply --check`/`git apply` de a, b et des trois diffs du premier essai | `2ba060c` + a, b | 0 | identiques ; les cinq s'appliquent sans reste | 3 s |
| 3 | `2859307` + diffs 2.13e2b/e2c contre `2ba060c`, `git diff -- src tests Makefile tools` | — | 0 | vide | 2 s |
| 4 | `base.sh` (test, e2e, e2e perturbé, compteur, `--champs`, `--check`, casts, tests de classe, `usegraph.py`, taille, garde) | 4 arbres sur `2ba060c` | 0 | vert, 26, 26 ; comptes égaux à ceux du premier essai | 45 s |
| 5 | `mut.py` (79 lancements, `muts.json` du premier essai relu) | idem | 0 | K-init-c **rouge** (vert au premier essai : arbre muté), K-init-d2 appliqué ; **MT-jc : ÉCHEC** (`jc` rouge au parent, vert à l'enfant) | ≈ 18 min |
| 6 | MT-jc : `assert` du lien dans `fold_jc` (vert sous la mutation : aucun `jc` du TU ne tire d'un `fr`), retiré ; `test_hq_takes_mt_from_its_start` ajouté à `c`, `d1`/`d2` repris | `c` | 0 / 2 | vert ; rouge sous la mutation (`test_pico8_jc.c:581`) | 1 min |
| 7 | 62 jumelles et lignes K relancées sur ces arbres ; preuves 2, 5, 6, 8, 9 ; `line-coverage` | idem | 0 | aucun ÉCHEC ; preuve 6 : `mz`, `js` rouges au premier jet (hunks dépendants, retirés), puis verts | ≈ 15 min |
| 8 | Message du coordinateur : base `2a91975` (2.14a `56ce4f6`, 2.14b `8fee30f`, `6e1fe46`) et quatre réserves du vérificateur ; `git fetch`, `git diff d6052ba 2a91975 -- src tests Makefile tools` | — | 0 | trois commentaires d'en-tête, une ligne de `test_pico8_pj.c` | 1 s |
| 9 | `cherry-pick` de `c`, `d1`, `d2` sur `2a91975` | `p` = `2a91975` | 0 | sans conflit | 1 s |
| 10 | `tc.c` sous `MALLOC_PERTURB_=90`, puis avec `GLIBC_TUNABLES=glibc.malloc.tcache_count=0` | — | 0 | même bloc ; 0/64 puis 64/64 à `0xa5` | < 1 s |
| 11 | réserves 2 et 4 dans le TU `struct_header` de `c` (relance, `assert_malloc_fills` sur bloc recyclé, deux commentaires) ; TU par `make`, puis `PICO8_MALLOC_FILLS=1 MALLOC_PERTURB_=90` sans tcache coupé, puis `PICO8_MALLOC_FILLS=1` seul ; `commit --amend`, `d1`/`d2` repris par `cherry-pick` ; diffs régénérés | `c`, `d1`, `d2` | 0 / 134 / 134 | vert ; `assert_malloc_fills` ×2 | 20 s |
| 12 | `base.sh` | `p`, `c`, `d1`, `d2` | 0 | vert, 26, 26 aux quatre ; la table « Comptes », égale à celle de `2ba060c` | 40 s |
| 13 | `mut.py`, 81 lancements (+ jumelle I-jc, réserve 3) | les quatre | 0 | aucun ÉCHEC (38 jumelles, 5 lignes K) ; tableaux ci-dessus | ≈ 13 min |
| 14 | preuve 6 (hunks des TU de l'enfant au parent, sans `test_ox` de `mz` ni `test_draw` de `js`) ; garde mordante ; `neg.c`, `neg2.c` ; TU sans relance ; `class-coverage-net`, `sanitize-net` ×3 ; `line-coverage` des 26 e2e | `c`, `d1`, `d2` | 0 (garde mutée 2 ; `neg` 1 ; sans relance 134) | preuve 6 : sept TU verts au parent avec les constantes neuves ; garde 0 puis 1 ligne ; 5 + 1 erreurs `const` ; `Aborted` ; `couverture: OK` et `26 propre(s)` ×3 ; 6064 lignes, 47 / 47 | ≈ 6 min |
| 15 | `obs.c` sur `p` (`-DV=0`), `c` (1), `d1` (2), `d2` (3), normal et sous `MALLOC_PERTURB_=165` ; `md5sum` | les quatre | 0 | 46 lignes, huit sorties identiques | ≈ 1 min |
| 16 | greps de Sortie (`git grep` sur les quatre étiquettes) ; lignes exécutables ajoutées contre la couverture ; `git grep -l 'has_p_\|has_m_' 2ba060c -- tests` | les quatre | 0 | ci-dessus ; 21 TU | < 5 s |
| 17 | les trois diffs extraits de ce PRD, `git apply --check`/`git apply` sur `2a91975`, `git diff` contre `d2` | — | 0 | trois `git apply` sans reste ; diff vide | 3 s |
| 18 | `rm -rf` de la copie et des fichiers d'essai | — | 0 | supprimés | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `2a91975` (ou un descendant qui ne touche que `docs/` ou `tests/test_pico8_pj.c`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si 2.14c, 2.14d1, 2.14d2 ne sont pas trois commits (hors repli écrit), dans cet ordre, chacun vert (`make test`, `make e2e` 26 conformes, et 26 sous `MALLOC_PERTURB_=165`), ou si un `*.sha256`, `tests/e2e/` ou `tools/` change.
- `FAILED:` si une constante `*_HASH`/`FROZEN_*` change à `c` ; si, à `d1`/`d2`, une constante autre que les huit nommées change, ou l'une d'elles sans la réponse « oui » de l'humain, ou pour une autre valeur que celle de la preuve 6.
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant ; si une ligne K (hors K-mp à `p`) ou une ligne H à l'enfant est verte.
- `FAILED:` si un lien est posé hors de l'init de son porteur (sauf `mt` de `jc` aux deux écritures de `has_mt`, et `kq` de `js` par `js:pq`) ; si un lien de champ A ne vise pas `def`, ou un lien de champ écrit par la classe vise `def` ; si un lien autre que `ma`, `p_`, `m_` est mutable ; si un cast écrit à travers un lien `const`.
- `FAILED:` si le membre `js.kq` reste, ou si `js:pq` copie au lieu d'écrire le noyau.
- `FAILED:` s'il reste dans `pico8_struct_class_t` un accesseur de champ (`get_kq`, `get_ql`, `get_fg`, `get_mt`, `get_fx`, `get_p_`, `set_m_`), ou un membre `has_fx`, `has_p_`, `has_m_` dans `src/` ou `tests/` à `d2`.
- `FAILED:` si `test_mouse_point_has_null_links` ou `test_hq_takes_mt_from_its_start` manque, si K-mp ou I-jc n'est pas rouge à `c`, si la relance du TU `struct_header` ne coupe pas le tcache ou si `assert_malloc_fills` ne lit pas un bloc recyclé.
- `FAILED:` si les comptes ne sont pas ceux de la table « Comptes » (`accesseurs-q6` 80 / 32 / 0, gardé à `d2` ; `lien-parent` 110 / 109 / 109 ; `void-ptr` 8 / 8 / 7 ; pied 243 / 194 / 162 ; `--champs` 249 / 243 / 238), si `défauts` n'est plus à 0 et gardé, si `inclusion-mutuelle` ou le graphe d'usage bougent, ou si une ligne `#include` de `src/` est ajoutée ou retirée.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge ; si les valeurs observables (preuve 5) diffèrent ; si la garde ne mord pas (preuve 9).
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un message de commit omet : les liens posés et leur cible (`def` ou instance), les accesseurs et `has_X` retirés, la table des jumelles avec leur résultat au parent, les lignes H et K, la preuve `const`, les comptes, les constantes changées et la preuve 6, le tableau « Runs ».
- `FAILED:` s'il manque une note « Découvert à 2.14c », « Découvert à 2.14d1 » ou « Découvert à 2.14d2 » au bout de sa ligne du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les liens `emitter`/`probe`, le masque `lineage`, `pico8_d_` (`probe_of`), `pico8_g__of` hors de `probe_host_raise_p_` (2.15).
- `particle.c:12` et les paires d'inclusion, dont `entity_geom` ↔ `planet` et `entity_geom` ↔ `scroll` (2.17, 2.17e).
- `tests/test_pico8_pj.c` (correctif annoncé, hors de ce PRD) ; la taille du code C (Q-2.10 : hors phase 2) ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. `assert_links(h, want)` prend l'attendu comme un `pico8_struct_header_t`** (littéral composé, liens non nommés à NULL) au lieu d'une liste d'arguments qui s'allongerait à chaque lien : un lien de plus = une ligne de comparaison, et l'attendu de chaque porteur ne nomme que ses liens.
- **G2. Un test neuf dans `test_pico8_jc.c`** (`test_hq_takes_mt_from_its_start`) : le TU `jc` perdait son seul rouge sur `mt` (l'appel direct de `get_mt`, parti avec l'accesseur) ; tirer un `jc` d'un `jo` puis d'un `fr` et épingler `has_mt`, `mt` et le lien rend la jumelle MT-jc conforme. Pas de commentaire : le nom le dit.
- **G3. `js.kq` monte dans le noyau** sans copie : son seul écrivain (`js:pq`) écrit `self->header.kq` ; c'est la seule pose hors d'une init, parce que c'est là que le cart pose `kq`. Le lien n'est pas `const` côté stockage (la boîte est allouée par `pico8_qz`, jamais libérée, comme avant), mais `header.kq` est `const pico8_qz_box_t *` : aucun lecteur n'écrit la boîte.
- **G4. `probe_host_raise_p_` admet tout porteur de `p_`**, plus large que l'ancien aiguillage (quatre hôtes) : `fq`, `iv`, `mh` portent `p_` mais aucun `ey` de ces trois-là n'appelle `d_` ; l'`assert` porte sur la présence du lien, comme celui de `pico8_ma` et de `ui:gw`.
- **G5. `jo.fx` en `pico8_fixed_t`, 0 pour nil** ; l'archétype de `g_`/`fv` vaut 1.0 (le `fx=1` de Lua), lié par `&def->fx`. Le lecteur `mz:pq` lit `p->fx != NULL && *p->fx != 0`.
- **G6. Le lien `mt` de `jc` est aussi posé à NULL dans `_new`**, bien que l'init du noyau l'ait déjà mis à NULL : le plan demande le lien « reposé à ses deux écritures de `has_mt` », et la ligne garde les deux écritures voisines.
- **G7. `particle.c:12` reste** (contrat de 2.14ab pour 2.17).
- **G8. Le tcache coupé par `GLIBC_TUNABLES`** plutôt qu'un `mallopt` (glibc seule, `<malloc.h>` absent sur macOS) ou un crochet sur `malloc` : c'est une variable de plus dans la relance, sans effet ailleurs ; `assert_malloc_fills` lit un bloc recyclé, donc arrête le TU si un allocateur rend un bloc recyclé non rempli (vu sous glibc 2.39 sans la variable). `MallocScribble` remplit aussi un bloc recyclé sur macOS : **non essayé** ; l'`assert` y arrêterait le TU, jamais un vert à vide.

### Surprises de mesure, pour `plan-pilot`

1. **L'étude 2.10 (§1) se trompait pour les TU** : « `has_fx`, `has_p_`, `has_m_` ne sont lus que dans `has_X && X` » est vrai dans `src/`, faux dans `tests/` : 21 TU lisent `has_p_`/`has_m_`, et six (`fq`, `ga`, `mz`, `bg`, `fv`, `js`) plient l'état « faux présent » dans une empreinte. D'où les huit constantes et la question.
2. **K-init à `c` et `d1` casse le jeu, K-init-d2 non** : seul le TU `struct_header` voit un `m_` indéterminé.
3. **Le premier essai de ce PRD (interrompu) avait des résultats faux** : une copie laissée dans un état muté a rendu K-init-c vert et K-init-d2 non appliqué ; tout a été relancé sur une copie reconstruite depuis `git`, où K-init-c est rouge (douze TU, e2e 0).
4. **Le tcache de glibc** rend un bloc libéré sans le remplir, même sous `MALLOC_PERTURB_` (0/64 octets à `0xa5` pour un bloc de 64 libéré puis repris, 64/64 tcache coupé) : le TU de 2.14a ne voyait que les blocs neufs (réserve 2 du vérificateur).
5. Les numéros du plan ont dérivé : `probe.c:283` est `:282` à `p`, `planet.h:122` est `:108`, `struct.h:139-141` est `:148-151`.

### Questions pour l'humain

**Q-2.14d** (bloque 2.14d1 et 2.14d2, pas 2.14c) : « Le plan demande de retirer `has_p_` et `has_m_`, et aussi qu'aucune empreinte de TU ne change. Les deux ne tiennent pas ensemble : sans `has_X`, "faux" et "absent" deviennent la même valeur 0. Le jeu ne voit aucune différence (le cart ne teste que la vérité ; 26 e2e identiques), mais huit constantes `*_HASH` de TU changent, parce que ces TU comptaient "faux" à part. Peut-on changer ces huit constantes (valeurs refaites au parent en changeant seulement le pli) ? Si non, on s'arrête après 2.14c. »
