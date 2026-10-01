---
title: "Plan 3, étape 2.13c : la famille c lit son archétype (`g_`, `fv`) et le composant émetteur reçoit son `def` ; `défauts` 507 → 403, pied 911 / 74 → 807 / 73 — un commit, jeu identique"
description: "`pico8_g__t` reçoit un `const pico8_g__def_t *def`, passé à `pico8_g__struct_init` par ses deux `_new` (`&pico8_g__def`, `&pico8_fv_def`) ; les onze champs A (`g_.fa`/`fx`, `fv.hi`/`ca`/`gq`/`eo`/`ff`/`kq`/`ql`/`fg`/`rb`) quittent l'instance, `ra`, l'émetteur et la sonde sont copiés depuis `def`, `pico8_g__start` et `pico8_fv_start` disparaissent. `pico8_emitter_t` gagne `def` (`&def->emitter` sur `g_`/`fv`, NULL chez `hj` et `ga`) ; le `def` de sonde de `g_`/`fv` passe de NULL à `&def->probe`. Aucun lecteur ne lit `emitter->def`/`probe->def` avant 2.13e2. Six filets uniques (sonde `has_od`, `ik`, `has_ik` de `g_` et de `fv`) reçoivent un `assert` à littéral ; les poses de `def` sont épinglées. Essayé sur copie : vert, 26 e2e, 51 jumelles de chaîne sans ÉCHEC."
status: needs-triage
---

## Problem Statement

**Écrit sur `b925a63`** (branche `claude/plan-pilot-3-7zdft0`), pendant que 2.13b s'implémentait dans l'arbre de travail (fichiers non commités non lus). Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Essais sur une seule copie jetable (scratchpad `prd213c/`, supprimée depuis). Sur cette copie, `b925a63` a reçu le diff du PRD 2.13b (`docs/prd/triage/plan3-phase2-etape13b-defauts-famille-b.md`, bloc `diff`, `git apply` sans erreur), commité `30d074a`. Notation : `p` = `30d074a`, l'arbre d'après 2.13b ; `c` = `bebe104`, le commit de la copie qui porte le texte de « Ce qui change ».

Vu du mainteneur du port : 2.13a et 2.13b ont basculé les racines simples et les deux racines à sonde. 2.13c fait de même pour la sonde (`g_`) et sa fille l'aide (`fv`), qui partagent un type d'archétype (`pico8_g__def_t`, décision A de l'étude 2.9) et une init (`pico8_g__struct_init`). C'est aussi l'étape où le **composant émetteur** reçoit son `def` (étude 2.10, §2, réserves 1 et 2 ; contrat de 2.12c).

### Texte du plan, recopié (`docs/plan-3.md` à `b925a63`, ligne 2.13)

> - 2.13 — l'init lit l'archétype, les `pico8_X_start(_t)` et la chaîne d'init disparaissent. Chaque instance reçoit `def`, posé par son `_new`. L'init copie la part I (étude, §2), et les lectures des champs A passent par `self->def`. Les champs A quittent l'instance. […] La fonction d'une famille dans le test ponctuel part dans le commit qui retire sa chaîne […]. Les écritures de TU sur un champ A sont réécrites. […] `g_.fa` et la sonde `dw` passent par un archétype de test pointé par `def`. Les comptes d'arrivée sont attribués par les noms `pico8_X_start` de chaque ligne du `--check`. Preuves : chemin touché (`gcov` des inits, mutations : défaut d'archétype changé → e2e ou TU rouge) ; règle 1 : un champ A qui quitte l'instance a sa preuve `const`, rejouée sur la copie du commit ; `game_data.h` mesuré au commit. `class-coverage-net` : oui. `sanitize-net` : oui. Budget ≈ 7 × 50 min.
>   - 2.13c — `g_`, `fv`. Compte : → **403**.

Note « Découvert à 2.12c » : « Pour 2.13c : le membre `def` de `pico8_emitter_t` n'ajoute aucune inclusion (`particle.h` inclut déjà `game_data.h`), et `hj_struct_init`, `ga_struct_new` doivent le poser (NULL ou archétype) ; pas de sous-archétype émetteur nul (aucun émetteur hors d'un porteur) ; un TU durable doit garder le `has_od` de `g_` quand son test ponctuel part avec la chaîne. »

Étude 2.9 (`docs/etude-2-9-archetypes.md`), famille c : `g_` A `fa`, `fx` ; I `ra` (NULL pour `g_`, `&def->ra` pour `fv`). `fv` A `ca`, `eo`, `ff`, `fg`, `gq`, `hi`, `kq`, `ql`, `rb`. Émetteur : A `ki`, `pk`, `fc` ; I `pn`, `du`, `py`, `k_`, `qo`, `ro`, `qg`, `q_` ; E `qy`. Sonde : A `dw`, `dq`, `fp`, `ik`, `has_ik` ; I `od`, `has_od` ; E `et`. En-tête `me`/`t` : I, par `pico8_ke_start` (ni `g_` ni `fv` ne réécrit `me`). Étude 2.10, §2 : `def` dans le composant, posé par l'init du porteur à `&def->emitter` ; réserve 2 : les champs A de l'émetteur **restent membres de `pico8_emitter_t` jusqu'en 2.13e2** (familles c et e les écrivent).

Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que fait 2.13c |
|---|---|
| `def` de sonde de `g_`/`fv` : NULL → `def` de la classe concrète, copier la sonde depuis ce `def`, ne pas basculer les lecteurs (PRD 2.13b) | `self->probe->def = &self->def->probe;` dans `pico8_g__struct_init`, qui reçoit le `def` en paramètre ; les sept champs de sonde sont copiés depuis `self->def->probe`, littéraux compris (`has_od = 1`, `ik = 0`, `has_ik = 0`) |
| `def` de `pico8_emitter_t`, posé chez `hj`, `ga` (NULL ou archétype), pas d'émetteur nul (2.12c) | membre en tête de `pico8_emitter_t` ; `&self->def->emitter` sur `g_`/`fv`, **NULL** dans `pico8_hj_struct_init` (sert `hj`, `pm`, `mx`) et `pico8_ga_struct_new` ; pas de `pico8_emitter_def_none` : tout `pico8_emitter_t` est un `emitter_own` d'un de ces trois porteurs (grep de Sortie 4) |
| `has_od` de `g_` gardé par un TU durable (2.12c, table du PRD 2.13a) | `assert(g_->probe->has_od == 1)` à littéral dans `test_g__defaults_and_ready` ; cinq autres filets uniques trouvés par le rejeu des jumelles (preuve 3), traités de même |
| `g_.fa` par un archétype de test (plan ; PRD 2.13b, section `test_pico8_jc.c:853`) | `test_pico8_g_.c:751` : une copie locale `fa_zero` de `pico8_g__def` à `fa = 0`, passée à `pico8_g__struct_init` sur une sonde locale — le `def` n'est écrit que par l'init, même dans les TU (preuve 2) |
| Réserve du vérificateur de 2.13b : rien n'épingle le `def` de sonde de `g_`/`fv`, de l'arbre `qh`, ni de `pb` | `g_`/`fv` : `assert` de pose `probe->def == &pico8_X_def.probe` et `emitter->def == &pico8_X_def.emitter` dans `spec_defaults` (N2, N3 rouges) ; émetteur de `hj`/`ga` : `assert(…->emitter->def == NULL)` dans `jo_spec_defaults` (N1, N4 rouges) ; arbre `qh` : **2.13d**, qui remplace ce NULL (contrat plus bas) ; `pb` : local de `pico8_ie_kj`, aucun TU ne l'atteint, épinglé par le grep de Sortie 4 (texte exact) ; la valeur de `pico8_probe_def_none` n'a pas de lecteur et n'en aura pas. Le commentaire du membre `def` de `pico8_probe_t` nomme maintenant `pb` |

**Quand les lecteurs de l'émetteur basculent sur `emitter->def` : en 2.13e2, pas avant — même règle que la sonde.** Les lecteurs des champs A de l'émetteur sont `probe.c:368` (`ki`), `:369` (`pk`), dans `pico8_g__mm`, et `particle.c:149` (`fc`, dans `pico8_ei`), numéros de `c`. Ils passent à `emitter->def->X` dans le commit **2.13e2**, celui qui retire `pico8_ga_start` : `ga` est la dernière famille dont un `_new` écrit un champ A de l'émetteur (`ki = 0`, `pk = 0`, `fc = pico8_ga_start.fc`), et `hj`/`pm`/`mx` écrivent encore les leurs jusqu'en 2.13e1. Basculer plus tôt rendrait morte une écriture de `_start` encore gardée par une jumelle (ÉCHEC par la règle du parent en 2.13e1/e2). C'est aussi en 2.13e2 que `ki`, `pk`, `fc` quittent `pico8_emitter_t`, que leurs copies dans les inits partent (`g_` ici, `hj`, `ga`) et que le commentaire du membre `def` part. Les I (`pn`, `du`, `py`, `k_`, `qo`, `ro`, `qg`, `q_`) restent membres : `pico8_g__struct_new` réécrit `pn`/`du`/`py` par clé, `ga` réécrit `py`, `k_`, `qo`, `ro`, `qg`, `q_` par `keys->pq`. Mesuré ici : N1 à N4 (un `def` d'émetteur ou de sonde pointé ailleurs) ne rougissent que les `assert` de pose, 26 e2e conformes : **personne ne lit `emitter->def` ni `probe->def` à `c`**.

### Question ouverte chez l'humain (Q-2.10, liens du noyau)

**2.13c n'en dépend pas.** Le `def` de `g_` est un membre de l'instance ; celui de l'émetteur est un membre de `pico8_emitter_t`, que l'instance stocke (`emitter_own`). Aucun n'est un lien du noyau. Le lien `emitter` du noyau (2.15a) visera `&self->emitter_own` quelle que soit la réponse, ou n'existera pas ; le `def` du composant est lu à travers lui dans les deux cas. La paire `particle` ↔ `probe`, l'une des trois qui dépendent de Q-2.10 (plan, note 2.11), n'est pas touchée : aucune inclusion ne change.

### Réserves mineures reçues

L'assertion tautologique `ed_fixed(ju) == 1` de `tests/test_pico8_pj.c` et la ligne de commentaire coupée court de `src/pico8_research_icon.h` : **2.13c ne touche aucun des deux fichiers** ; elles restent à reprendre par la première sous-étape qui les touche (au plus tard 2.13e2 pour `research_icon.h`, si la bascule des lecteurs de sonde y passe ; sinon une ligne dans le commit de 2.13e2).

### État de départ attendu (le commit 2.13b, ou un descendant qui ne touche que `docs/`)

Mesuré sur `p` (`b925a63` + diff du PRD 2.13b) :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 507 29`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 911 ligne(s), 74 fichier(s)`** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 274 ; 0 écriture : 15 ; 0 hors constructeur : 20` |
| `--check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `wc -l < src/pico8_game_data.h` | 642 |
| total asserté du test ponctuel | `g_compared == 384` |
| `make test` ; `make e2e` | vert (16 s) ; 26 conformes |
| lignes `défauts` qui nomment `pico8_g__start` ou `pico8_fv_start` | **104**, toutes attribuées à c (aucune ne nomme un `_start` plus tardif) |
| casts / tests de classe (grep de la réserve 4 de 2.13a) | 532 / 707 |

Un départ autre : arrêt, rapporté. Si 2.13b n'a pas été commitée à l'identique (`git diff 30d074a <commit 2.13b> -- src tests` non vide, `30d074a` se refait par `git apply` du bloc du PRD 2.13b sur `b925a63`), le diff s'applique par ancres de texte : il ne touche aucun texte de 2.13b hors le commentaire du membre `def` de `pico8_probe_t`, la pose `self->probe->def = NULL;` de `pico8_g__struct_init` et la ligne du total du test ponctuel.

## Solution

Un commit.

- **`def` d'instance** : `const pico8_g__def_t *def` dans `pico8_g__t`, premier membre après `header`. `pico8_g__struct_init` prend un troisième paramètre `def` et le pose une seule fois, juste après `pico8_ke_struct_init` ; `pico8_g__struct_new` passe `&pico8_g__def`, `pico8_fv_struct_new` `&pico8_fv_def`. `fv` n'a pas de `def` propre : c'est celui de son `g_` (un type d'archétype pour les deux, décision A).
- **Champs A qui quittent l'instance** (11) : `g_.fa`, `g_.fx` ; `fv.hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb`. Lecteurs : `pico8_g__get_fa`, `emitter_host_fa_ra` (`particle.c:24`), `pico8_fv_pq` (`eo`, `hi`, `ca`, `gq`), `pico8_fv_kj` (`rb`), `fv_get_kq`/`ql`/`fg` lisent `…def->X` (ou `&…def->X` pour les vecteurs et la boîte, que l'instance pointait déjà dans `pico8_fv_start`). `fx` et `ff` n'ont pas de lecteur dans `src/` (`get_fx` rend 1 ; `frame.ff = 1`) : ils ne vivent plus que dans l'archétype, lus par les empreintes des TU.
- **Copiés depuis `def`** : `ra` (`def->has_ra ? &def->ra : NULL`) ; les onze champs de l'émetteur et les sept de la sonde, A compris, puisqu'ils restent membres des composants jusqu'en 2.13e2 ; les littéraux d'init de la sonde (`has_od = 1`, `ik = 0`, `has_ik = 0`) deviennent des copies. Les 22 réécritures de `pico8_fv_struct_new` disparaissent : l'init copie directement l'archétype de `fv`.
- **Composant émetteur** : `const pico8_emitter_def_t *def` en tête de `pico8_emitter_t`, posé comme au tableau des contrats.
- `pico8_g__start_t`, `pico8_fv_start_t`, leurs valeurs et leurs `extern` sortent.
- **TU** : `archetype_chain` perd 51 comparaisons (garde `me`, `t` de `g_` et de `fv`) ; `spec_defaults` compare `g_`/`fv` à leur `def`, épingle les trois poses et garde six littéraux (filets uniques) ; `jo_spec_defaults` épingle le NULL de `hj`/`ga` ; `g_`, `fv`, `js`, `map_tables`, `ui` lisent les A par `def` ou nomment l'archétype à la place de `_start`.

Rien d'autre : ni `tools/`, ni `Makefile`, ni `glossaire-noms.md` (aucun nom minifié neuf), ni `trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`, ni les écritures `probe->dw` de `test_pico8_jc.c` (2.13e2).

## User Stories

1. En tant que mainteneur, je veux que `g_` et `fv` lisent leurs défauts sur leur archétype, afin que `pico8_g__start` et `pico8_fv_start` disparaissent sans changer une valeur du jeu.
2. En tant que gardien de la règle 1, je veux que les onze champs A de `g_`/`fv` quittent l'instance et que chaque `def` (instance, émetteur, sonde) soit `const` et posé une seule fois par l'init de son porteur, afin qu'aucune écriture hors construction ne soit possible — TU compris.
3. En tant qu'auteur de 2.13d à 2.13e2, je veux un `def` déjà dans `pico8_emitter_t`, déterminé sur chaque porteur, et la date précise où les lecteurs basculent, afin de ne ni lire un pointeur indéterminé ni rendre morte une écriture `_start` encore gardée.
4. En tant que gardien de la règle du parent, je veux qu'aucune valeur de la famille c ne perde son dernier filet quand sa comparaison part du test ponctuel.
5. En tant que gardien du test ponctuel, je veux que seules les comparaisons dont la chaîne existe encore restent (`me`, `t`, par `pico8_ke_start`).
6. En tant que gardien du compteur, je veux `défauts` 403, pied 807 / 73, sans nouvelle paire d'inclusion, et une explication chiffrée de `--champs`.

## Implementation Decisions

### Modules

`game_data`, `probe`, `help`, `particle`, `planet_family` (deux lignes) ; TU `archetype_chain`, `fv`, `g_`, `jo_spec_defaults`, `js`, `map_tables`, `spec_defaults`, `ui` ; `docs/plan-3.md`.

### Inclusions

`probe.h` et `particle.h` incluent déjà `pico8_game_data.h` ; `help.h` inclut `probe.h`. Aucune inclusion neuve. Mesuré : `inclusion-mutuelle` **17 / 16, 43 lignes** à `c`.

### Ce qui change

Le diff ci-dessous est le texte exact (`git diff 30d074a bebe104`). Lecture, en plus de la Solution :

- **Signature de l'init** : `pico8_g__struct_init(self, cls, def)`. C'est l'option « passer le `def` à l'init » du contrat de 2.13b, et la forme que 2.13d prend pour `pico8_jo_struct_init` (plan). Poser `def` dans chaque `_new` avant l'init aurait donné une précondition implicite (l'init lirait `self->def` sans le recevoir), et le poser dans l'init puis le réécrire dans `fv` deux écritures.
- **Ordre dans l'init** : `self->def`, puis `emitter` et `emitter->def`, puis `ra`, puis `probe` et `probe->def` ; les copies lisent `self->def->emitter.X` / `self->def->probe.X` (même adresse que `emitter->def`, et la ligne ne dépend pas du composant).
- **`test_pico8_g_.c:751`** (`roads[1]->fa = 0;`) devient un bloc : `pico8_g__def_t fa_zero = pico8_g__def; fa_zero.fa = 0;` puis `pico8_g__struct_init(&bare, cls, &fa_zero)` sur une sonde locale, et `assert(is_a_g__struct(&bare) && !is_probe(&bare.header))`. Même prémisse (une sonde de la classe `g_` dont `fa` n'est pas 2), sans écrire `def` hors de l'init ; la sonde locale n'est ni enregistrée (`pq` non lancé) ni libérée.
- **Empreintes** : les plis de `test_pico8_g_.c` et `test_pico8_fv.c` lisent `…->def->X` au même rang ; `fold_ref_member(&def->hi)` rend la même valeur que `fold_ref_member(fv->hi)` (l'adresse n'entre pas dans le pli, seul son genre) : `BUILT_HASH` inchangés, verts.
- **Commentaires** : `help.h` ne garde que le contrat de `g_.ra` (partagé) ; `probe.h` décrit l'init en une phrase de contrat ; le membre `def` de `pico8_probe_t` : « NULL on the jo tree, `pico8_probe_def_none` on research_icon's `pb`: no generic reader may use it » (réserve de 2.13b ; 2.13d le réécrit, 2.13e2 le retire) ; le membre `def` de `pico8_emitter_t` : « NULL on the jo tree: no generic reader may use it » (2.13e1 le réécrit si `hj` pose son archétype, 2.13e2 le retire). Les deux en-têtes de TU qui nommaient `pico8_X_start` disent « read on its archetype ».
- `same_vec_or_nil` de `archetype_chain` n'a plus d'appelant (seuls `g_.ra`/`fv.ra` s'en servaient) : retiré, sinon `-Werror=unused-function`.
- **Test ponctuel** : voir plus bas.

```diff
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index 51014e1..3563e18 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -292,51 +292,6 @@ const pico8_ke_start_t pico8_ke_start = {
     .t = 0x0,
 };
 
-const pico8_g__start_t pico8_g__start = {
-    .fx = 0x10000,
-    .dq = "ly",
-    .od = 0x0,
-    .fp = 0,
-    .fa = 0x20000,
-    .dw = 0x666,
-    .py = 0xf5c,
-    .ki = 0x90000,
-    .pn = 0x30000,
-    .pk = 0x10000,
-    .fc = 0x10000,
-    .k_ = &pico8_g__k__seq,
-    .qo = 0x1051e,
-    .ro = -0xd70,
-    .qg = 0xd70,
-    .q_ = 0x1051e,
-    .du = 0x20000,
-};
-
-const pico8_fv_start_t pico8_fv_start = {
-    .fa = 0xa0000,
-    .ra = {0x400000, 0x1c0000},
-    .hi = {0x220000, 0x380000},
-    .ca = 0xb0000,
-    .gq = {0x0, 0xb0000},
-    .eo = -0x10000,
-    .ff = 0x10000,
-    .fc = 0x50000,
-    .k_ = &pico8_fv_k__seq,
-    .qo = 0x1051e,
-    .ro = -0x1ca,
-    .qg = 0x1ca,
-    .q_ = 0x1051e,
-    .pn = 0x50000,
-    .du = 0xf0000,
-    .py = 0x1b6,
-    .pk = 0x10000,
-    .ki = 0x110000,
-    .kq = {-0x280000, 0x4e0000, 0x280000, 0x620000},
-    .ql = 0xb0000,
-    .fg = 0x50000,
-    .rb = &pico8_fv_rb_row,
-};
-
 const pico8_bg_def_t pico8_bg_def = {
     .me = "ph",
     .t = 0x0,
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index 9434509..5cc275c 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -119,53 +119,6 @@ typedef struct {
     pico8_fixed_t t;
 } pico8_ke_start_t;
 
-/* fp is the spec's false. */
-typedef struct {
-    pico8_fixed_t fx;
-    const char *dq;
-    pico8_fixed_t od;
-    int fp;
-    pico8_fixed_t fa;
-    pico8_fixed_t dw;
-    pico8_fixed_t py;
-    pico8_fixed_t ki;
-    pico8_fixed_t pn;
-    pico8_fixed_t pk;
-    pico8_fixed_t fc;
-    const pico8_int_seq_t *k_;
-    pico8_fixed_t qo;
-    pico8_fixed_t ro;
-    pico8_fixed_t qg;
-    pico8_fixed_t q_;
-    pico8_fixed_t du;
-} pico8_g__start_t;
-
-/* fv's own keys only: the others are g_'s. */
-typedef struct {
-    pico8_fixed_t fa;
-    pico8_vec_t ra;
-    pico8_vec_t hi;
-    pico8_fixed_t ca;
-    pico8_vec_t gq;
-    pico8_fixed_t eo;
-    pico8_fixed_t ff;
-    pico8_fixed_t fc;
-    const pico8_int_seq_t *k_;
-    pico8_fixed_t qo;
-    pico8_fixed_t ro;
-    pico8_fixed_t qg;
-    pico8_fixed_t q_;
-    pico8_fixed_t pn;
-    pico8_fixed_t du;
-    pico8_fixed_t py;
-    pico8_fixed_t pk;
-    pico8_fixed_t ki;
-    pico8_qz_box_t kq;
-    pico8_fixed_t ql;
-    pico8_fixed_t fg;
-    const pico8_jb_row_t *rb;
-} pico8_fv_start_t;
-
 /* A class's archetype: its defaults resolved along its init chain, ke's
  * `me`/`t` first. A default that is the address of a module's static object
  * (ui.le, nf.ra, mz.jj) is not here: the module's init still sets it. */
@@ -570,8 +523,6 @@ extern const pico8_jb_row_t pico8_pj_ou_row;
 /* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
  * nf.r are empty tables, held by their module. */
 extern const pico8_ke_start_t pico8_ke_start;
-extern const pico8_g__start_t pico8_g__start;
-extern const pico8_fv_start_t pico8_fv_start;
 extern const pico8_jo_start_t pico8_jo_start;
 extern const pico8_qh_start_t pico8_qh_start;
 extern const pico8_nl_start_t pico8_nl_start;
diff --git a/src/pico8_help.c b/src/pico8_help.c
index 742a7ad..8a89308 100644
--- a/src/pico8_help.c
+++ b/src/pico8_help.c
@@ -64,12 +64,12 @@ void pico8_fv_pq(pico8_fv_t *self) {
         bs[i].my = pico8_eu_field_value();
     }
     frame.has_eo = 1;
-    frame.eo = self->eo;
-    frame.hi = self->hi;
+    frame.eo = self->g_.def->eo;
+    frame.hi = &self->g_.def->hi;
     frame.ff = 1;
     frame.has_ca = 1;
-    frame.ca = self->ca;
-    frame.gq = self->gq;
+    frame.ca = self->g_.def->ca;
+    frame.gq = &self->g_.def->gq;
     frame.ra = self->g_.ra;
     pico8_pz(&frame, bs, PICO8_BO_COUNT);
 }
@@ -87,9 +87,9 @@ void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec
         int n = 0;
         int i;
 
-        for (i = 0; self != NULL && self->rb != NULL && i < self->rb->count; i++) {
+        for (i = 0; self != NULL && self->g_.def->rb != NULL && i < self->g_.def->rb->count; i++) {
             assert(n < PICO8_FV_RB_CAP && "fv.rb has four articles in the cart");
-            items[n++] = self->rb->items[i].row;
+            items[n++] = self->g_.def->rb->items[i].row;
         }
         pico8_bl_default(gfx, mem, items, n);
     }
@@ -101,16 +101,16 @@ void pico8_fv_kj_dispatch(void *self, const pico8_vec_t *ra) {
 }
 
 static const pico8_qz_box_t *fv_get_kq(const void *self) {
-    return ((const pico8_fv_t *)self)->kq;
+    return &((const pico8_fv_t *)self)->g_.def->kq;
 }
 
 static int fv_get_ql(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_fv_t *)self)->ql;
+    *out = ((const pico8_fv_t *)self)->g_.def->ql;
     return 1;
 }
 
 static int fv_get_fg(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_fv_t *)self)->fg;
+    *out = ((const pico8_fv_t *)self)->g_.def->fg;
     return 1;
 }
 
@@ -148,31 +148,9 @@ static pico8_fv_t *pico8_fv_struct_new(void) {
 
     assert(g_fv_up && "pico8_help_init() must run first");
     self = malloc(sizeof *self);
-    pico8_g__struct_init(&self->g_, &g_fv_struct_class);
+    pico8_g__struct_init(&self->g_, &g_fv_struct_class, &pico8_fv_def);
     self->m_ = 0;
     self->has_m_ = 0;
-    self->g_.fa = pico8_fv_start.fa;
-    self->g_.ra = &pico8_fv_start.ra;
-    self->g_.emitter->fc = pico8_fv_start.fc;
-    self->g_.emitter->k_ = pico8_fv_start.k_;
-    self->g_.emitter->qo = pico8_fv_start.qo;
-    self->g_.emitter->ro = pico8_fv_start.ro;
-    self->g_.emitter->qg = pico8_fv_start.qg;
-    self->g_.emitter->q_ = pico8_fv_start.q_;
-    self->g_.emitter->pn = pico8_fv_start.pn;
-    self->g_.emitter->du = pico8_fv_start.du;
-    self->g_.emitter->py = pico8_fv_start.py;
-    self->g_.emitter->pk = pico8_fv_start.pk;
-    self->g_.emitter->ki = pico8_fv_start.ki;
-    self->hi = &pico8_fv_start.hi;
-    self->ca = pico8_fv_start.ca;
-    self->gq = &pico8_fv_start.gq;
-    self->eo = pico8_fv_start.eo;
-    self->ff = pico8_fv_start.ff;
-    self->kq = &pico8_fv_start.kq;
-    self->ql = pico8_fv_start.ql;
-    self->fg = pico8_fv_start.fg;
-    self->rb = pico8_fv_start.rb;
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("fv");
 #endif
diff --git a/src/pico8_help.h b/src/pico8_help.h
index 1e6b7ef..e77d228 100644
--- a/src/pico8_help.h
+++ b/src/pico8_help.h
@@ -14,19 +14,9 @@
  * no-op, and an absent or wrong-kind field reads as absent. */
 
 /* TROU-7 */
-/* `ra`, `hi`, `gq`, `kq` point at `pico8_fv_start`'s, shared by every help;
- * `k_` points at `pico8_fv_k__seq`, `rb` at `pico8_fv_rb_row`. */
+/* `g_.ra` points at `pico8_fv_def`'s, shared by every help. */
 typedef struct {
     pico8_g__t g_;
-    const pico8_vec_t *hi;
-    pico8_fixed_t ca;
-    const pico8_vec_t *gq;
-    pico8_fixed_t eo;
-    pico8_fixed_t ff;
-    const pico8_qz_box_t *kq;
-    pico8_fixed_t ql;
-    pico8_fixed_t fg;
-    const pico8_jb_row_t *rb;
     int m_;
     int has_m_; /* 0: `m_` is nil, and is then never read */
 } pico8_fv_t;
diff --git a/src/pico8_particle.c b/src/pico8_particle.c
index 9c4e66b..6e5510f 100644
--- a/src/pico8_particle.c
+++ b/src/pico8_particle.c
@@ -21,7 +21,7 @@ static pico8_fixed_t emitter_host_fa_ra(pico8_struct_header_t *self, pico8_vec_t
 
     if (g_ != NULL) {
         *ra = g_->ra != NULL ? *g_->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-        return g_->fa;
+        return g_->def->fa;
     }
     assert(pico8_jo_is_family(self) &&
            "ei: a host neither of g_'s line nor of jo's family, none of whose kj runs ei -- stop");
diff --git a/src/pico8_particle.h b/src/pico8_particle.h
index a34973f..5f1a783 100644
--- a/src/pico8_particle.h
+++ b/src/pico8_particle.h
@@ -29,6 +29,7 @@ typedef struct pico8_qy {
  * `pn`, `du`, `ki`, `pk` are read by `g_:mm` only: a `ga` holds 0 there.
  * `pk` is the cart's 1 or false, false held as 0: only its truth is read. */
 typedef struct pico8_emitter {
+    const pico8_emitter_def_t *def; /* NULL on the jo tree: no generic reader may use it */
     pico8_fixed_t pn;
     pico8_fixed_t du;
     pico8_fixed_t py;
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index 834edcb..600c2d0 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -289,6 +289,7 @@ void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls) {
     pico8_qh_struct_init(&self->qh, cls);
     self->rr = pico8_hj_start.rr;
     self->emitter = &self->emitter_own;
+    self->emitter->def = NULL;
     self->emitter->py = pico8_hj_start.py;
     self->emitter->ki = pico8_hj_start.ki;
     self->emitter->pn = pico8_hj_start.pn;
@@ -570,6 +571,7 @@ static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
     pico8_jo_struct_init(&self->jo, &g_ga_struct_class);
     self->fz = pico8_ga_start.fz;
     self->emitter = &self->emitter_own;
+    self->emitter->def = NULL;
     self->emitter->pn = 0;
     self->emitter->du = 0;
     self->emitter->ki = 0;
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index 020aac6..5213568 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -396,7 +396,7 @@ pico8_qy_t *pico8_g__mm(void *self_ptr) {
 }
 
 int pico8_g__get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_g__t *)self)->fa;
+    *out = ((const pico8_g__t *)self)->def->fa;
     return 1;
 }
 
@@ -436,37 +436,37 @@ static const pico8_struct_class_t g_g__struct_class = {
     .get_p_ = pico8_g__get_p_,
 };
 
-void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls) {
+void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, const pico8_g__def_t *def) {
     pico8_ke_struct_init(&self->header, cls);
+    self->def = def;
     self->emitter = &self->emitter_own;
+    self->emitter->def = &self->def->emitter;
     self->emitter->qy = NULL;
-    self->ra = NULL;
+    self->ra = self->def->has_ra ? &self->def->ra : NULL;
     self->p = NULL;
     self->probe = &self->probe_own;
-    self->probe->def = NULL;
+    self->probe->def = &self->def->probe;
     self->probe->et = NULL;
     self->p_ = 0;
     self->has_p_ = 0;
-    self->fx = pico8_g__start.fx;
-    self->probe->dq = pico8_g__start.dq;
-    self->probe->od = pico8_g__start.od;
-    self->probe->has_od = 1;
-    self->probe->fp = pico8_g__start.fp;
-    self->probe->ik = 0;
-    self->probe->has_ik = 0;
-    self->fa = pico8_g__start.fa;
-    self->probe->dw = pico8_g__start.dw;
-    self->emitter->py = pico8_g__start.py;
-    self->emitter->ki = pico8_g__start.ki;
-    self->emitter->pn = pico8_g__start.pn;
-    self->emitter->pk = pico8_g__start.pk;
-    self->emitter->fc = pico8_g__start.fc;
-    self->emitter->k_ = pico8_g__start.k_;
-    self->emitter->qo = pico8_g__start.qo;
-    self->emitter->ro = pico8_g__start.ro;
-    self->emitter->qg = pico8_g__start.qg;
-    self->emitter->q_ = pico8_g__start.q_;
-    self->emitter->du = pico8_g__start.du;
+    self->probe->dq = self->def->probe.dq;
+    self->probe->od = self->def->probe.od;
+    self->probe->has_od = self->def->probe.has_od;
+    self->probe->fp = self->def->probe.fp;
+    self->probe->ik = self->def->probe.ik;
+    self->probe->has_ik = self->def->probe.has_ik;
+    self->probe->dw = self->def->probe.dw;
+    self->emitter->py = self->def->emitter.py;
+    self->emitter->ki = self->def->emitter.ki;
+    self->emitter->pn = self->def->emitter.pn;
+    self->emitter->pk = self->def->emitter.pk;
+    self->emitter->fc = self->def->emitter.fc;
+    self->emitter->k_ = self->def->emitter.k_;
+    self->emitter->qo = self->def->emitter.qo;
+    self->emitter->ro = self->def->emitter.ro;
+    self->emitter->qg = self->def->emitter.qg;
+    self->emitter->q_ = self->def->emitter.q_;
+    self->emitter->du = self->def->emitter.du;
 }
 
 static pico8_g__t *pico8_g__struct_new(const pico8_g__keys_t *keys) {
@@ -474,7 +474,7 @@ static pico8_g__t *pico8_g__struct_new(const pico8_g__keys_t *keys) {
 
     assert(g_g__up && "pico8_probe_init() must run first");
     self = malloc(sizeof *self);
-    pico8_g__struct_init(self, &g_g__struct_class);
+    pico8_g__struct_init(self, &g_g__struct_class, &pico8_g__def);
     if (keys != NULL) {
         self->header.t = keys->t;
         if (keys->has_pn) {
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index f6d5e88..81b879a 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -22,7 +22,9 @@
  * nil. `fp` is what `d_` hands to `fm:nx`: the cart's false is held as 0,
  * which is what `self.fp or 0` hands. */
 typedef struct pico8_probe {
-    const pico8_probe_def_t *def; /* NULL on g_ and the jo tree: no generic reader may use it */
+    /* NULL on the jo tree, `pico8_probe_def_none` on research_icon's `pb`:
+     * no generic reader may use it. */
+    const pico8_probe_def_t *def;
     pico8_struct_header_t *et;
     pico8_fixed_t dw;
     pico8_fixed_t od;
@@ -40,8 +42,7 @@ typedef struct pico8_probe {
  * `probe_own`: never copy the struct. */
 typedef struct {
     pico8_struct_header_t header;
-    pico8_fixed_t fx;
-    pico8_fixed_t fa;
+    const pico8_g__def_t *def;
     pico8_emitter_t *emitter;
     pico8_probe_t *probe;
     const pico8_vec_t *ra;
@@ -79,11 +80,10 @@ void pico8_probe_shutdown(void);
 int pico8_g__ready(void);
 const pico8_struct_class_t *pico8_g__struct_class(void);
 
-/* Initializes an allocated pico8_g__t under `cls`: ke's part, g_'s 5
- * written members nil, g_'s 18 defaults from `pico8_g__start`, 12 of them
- * in the emitter, 4 in the probe. Reads no
- * key, runs neither `pq` nor `lt`, records no coverage. */
-void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls);
+/* Initializes an allocated pico8_g__t under `cls`: ke's part, g_'s 4
+ * written members nil, `def` kept, `ra`, the emitter and the probe copied
+ * from it. Reads no key, runs neither `pq` nor `lt`, records no coverage. */
+void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, const pico8_g__def_t *def);
 
 /* The keys of `pico8_g__new`, read, not kept. `t` is `header.t`, whose default
  * is 0: no flag. `pn`, `du`, `py` have non-zero defaults, so a key at 0
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index a7d205f..a5660ac 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -64,10 +64,6 @@ static int same_vec(const pico8_vec_t *got, const pico8_vec_t *want) {
     return got != NULL && got->x == want->x && got->y == want->y;
 }
 
-static int same_vec_or_nil(const pico8_vec_t *got, const pico8_vec_t *want) {
-    return (got == NULL && want == NULL) || (want != NULL && same_vec(got, want));
-}
-
 static int same_box(const pico8_qz_box_t *got, const pico8_qz_box_t *want) {
     return got != NULL && got->xl == want->xl && got->yt == want->yt && got->xr == want->xr && got->yb == want->yb;
 }
@@ -172,27 +168,6 @@ static void test_g_(void) {
 
     same(same_text(g_->header.me, pico8_g__def.me), "g_.me");
     same(g_->header.t == pico8_g__def.t, "g_.t");
-    same(g_->fx == pico8_g__def.fx, "g_.fx");
-    same(g_->fa == pico8_g__def.fa, "g_.fa");
-    same(same_vec_or_nil(g_->ra, pico8_g__def.has_ra ? &pico8_g__def.ra : NULL), "g_.ra");
-    same(g_->emitter->pn == pico8_g__def.emitter.pn, "g_.emitter.pn");
-    same(g_->emitter->du == pico8_g__def.emitter.du, "g_.emitter.du");
-    same(g_->emitter->py == pico8_g__def.emitter.py, "g_.emitter.py");
-    same(g_->emitter->ki == pico8_g__def.emitter.ki, "g_.emitter.ki");
-    same(g_->emitter->pk == pico8_g__def.emitter.pk, "g_.emitter.pk");
-    same(g_->emitter->fc == pico8_g__def.emitter.fc, "g_.emitter.fc");
-    same(g_->emitter->k_ == pico8_g__def.emitter.k_, "g_.emitter.k_");
-    same(g_->emitter->qo == pico8_g__def.emitter.qo, "g_.emitter.qo");
-    same(g_->emitter->ro == pico8_g__def.emitter.ro, "g_.emitter.ro");
-    same(g_->emitter->qg == pico8_g__def.emitter.qg, "g_.emitter.qg");
-    same(g_->emitter->q_ == pico8_g__def.emitter.q_, "g_.emitter.q_");
-    same(g_->probe->dw == pico8_g__def.probe.dw, "g_.probe.dw");
-    same(g_->probe->od == pico8_g__def.probe.od, "g_.probe.od");
-    same(g_->probe->has_od == pico8_g__def.probe.has_od, "g_.probe.has_od");
-    same(same_text_or_nil(g_->probe->dq, pico8_g__def.probe.dq), "g_.probe.dq");
-    same(g_->probe->fp == pico8_g__def.probe.fp, "g_.probe.fp");
-    same(g_->probe->ik == pico8_g__def.probe.ik, "g_.probe.ik");
-    same(g_->probe->has_ik == pico8_g__def.probe.has_ik, "g_.probe.has_ik");
     assert(g_->emitter->qy != NULL && g_->emitter->qy->count == 0);
     assert(g_->probe->et == NULL);
 }
@@ -202,36 +177,6 @@ static void test_fv(void) {
 
     same(same_text(fv->g_.header.me, pico8_fv_def.me), "fv.me");
     same(fv->g_.header.t == pico8_fv_def.t, "fv.t");
-    same(fv->g_.fx == pico8_fv_def.fx, "fv.fx");
-    same(fv->g_.fa == pico8_fv_def.fa, "fv.fa");
-    same(same_vec_or_nil(fv->g_.ra, pico8_fv_def.has_ra ? &pico8_fv_def.ra : NULL), "fv.ra");
-    same(same_vec(fv->hi, &pico8_fv_def.hi), "fv.hi");
-    same(fv->ca == pico8_fv_def.ca, "fv.ca");
-    same(same_vec(fv->gq, &pico8_fv_def.gq), "fv.gq");
-    same(fv->eo == pico8_fv_def.eo, "fv.eo");
-    same(fv->ff == pico8_fv_def.ff, "fv.ff");
-    same(same_box(fv->kq, &pico8_fv_def.kq), "fv.kq");
-    same(fv->ql == pico8_fv_def.ql, "fv.ql");
-    same(fv->fg == pico8_fv_def.fg, "fv.fg");
-    same(fv->rb == pico8_fv_def.rb, "fv.rb");
-    same(fv->g_.emitter->pn == pico8_fv_def.emitter.pn, "fv.emitter.pn");
-    same(fv->g_.emitter->du == pico8_fv_def.emitter.du, "fv.emitter.du");
-    same(fv->g_.emitter->py == pico8_fv_def.emitter.py, "fv.emitter.py");
-    same(fv->g_.emitter->ki == pico8_fv_def.emitter.ki, "fv.emitter.ki");
-    same(fv->g_.emitter->pk == pico8_fv_def.emitter.pk, "fv.emitter.pk");
-    same(fv->g_.emitter->fc == pico8_fv_def.emitter.fc, "fv.emitter.fc");
-    same(fv->g_.emitter->k_ == pico8_fv_def.emitter.k_, "fv.emitter.k_");
-    same(fv->g_.emitter->qo == pico8_fv_def.emitter.qo, "fv.emitter.qo");
-    same(fv->g_.emitter->ro == pico8_fv_def.emitter.ro, "fv.emitter.ro");
-    same(fv->g_.emitter->qg == pico8_fv_def.emitter.qg, "fv.emitter.qg");
-    same(fv->g_.emitter->q_ == pico8_fv_def.emitter.q_, "fv.emitter.q_");
-    same(fv->g_.probe->dw == pico8_fv_def.probe.dw, "fv.probe.dw");
-    same(fv->g_.probe->od == pico8_fv_def.probe.od, "fv.probe.od");
-    same(fv->g_.probe->has_od == pico8_fv_def.probe.has_od, "fv.probe.has_od");
-    same(same_text_or_nil(fv->g_.probe->dq, pico8_fv_def.probe.dq), "fv.probe.dq");
-    same(fv->g_.probe->fp == pico8_fv_def.probe.fp, "fv.probe.fp");
-    same(fv->g_.probe->ik == pico8_fv_def.probe.ik, "fv.probe.ik");
-    same(fv->g_.probe->has_ik == pico8_fv_def.probe.has_ik, "fv.probe.has_ik");
     assert(fv->g_.emitter->qy != NULL && fv->g_.emitter->qy->count == 0);
     assert(fv->g_.probe->et == NULL);
 }
@@ -440,7 +385,7 @@ int main(void) {
     test_ga();
     test_nr();
     test_fr();
-    assert(g_compared == 384 && "families a to e compare 384 fields: one missed or one too many");
+    assert(g_compared == 333 && "families a to e compare 333 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_fv.c b/tests/test_pico8_fv.c
index 04d66f0..e9bcec6 100644
--- a/tests/test_pico8_fv.c
+++ b/tests/test_pico8_fv.c
@@ -1,6 +1,6 @@
 /* The help screen `fv`, built by its only road (`pico8__init()` on the
  * title screen, `pico8_fv_new()`), read only through the dual-form field
- * API, its defaults compared to `pico8_fv_start`, and driven through the
+ * API, its defaults read on its archetype, and driven through the
  * real `mv`/`dc`/`dg`/`lj`. Its
  * methods are compared to the public functions. No events file. */
 
@@ -252,7 +252,7 @@ static void start_title(void) {
 /* The help, whatever its form: `g_`'s tags and `fa`=10 (a probe has 2). */
 static int is_help(pico8_struct_header_t *e) {
     const pico8_g__t *g_ = e != NULL ? pico8_g__of(e) : NULL;
-    return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->fa == pico8_fixed_from_int(10);
+    return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->def->fa == pico8_fixed_from_int(10);
 }
 
 static pico8_fv_t *the_help(void) {
@@ -308,19 +308,19 @@ static void test_built(void) {
     pr = g_->probe;
     hv = pico8_hv();
 
-    assert(g_->fa == pico8_fixed_from_int(10));
+    assert(g_->def->fa == pico8_fixed_from_int(10));
     assert(em->ki == pico8_fixed_from_int(17));
-    assert(fv->ca == pico8_fixed_from_int(11));
+    assert(g_->def->ca == pico8_fixed_from_int(11));
     assert(g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)));
-    assert(g_->ra != NULL && fv->hi != NULL && fv->gq != NULL && em->k_ != NULL && fv->kq != NULL);
-    assert(fv->rb != NULL && pr->dq != NULL);
-    h = fold_fixed_member(h, g_->fa);
+    assert(g_->ra != NULL && em->k_ != NULL);
+    assert(g_->def->rb != NULL && pr->dq != NULL);
+    h = fold_fixed_member(h, g_->def->fa);
     h = fold_ref_member(h, g_->ra);
-    h = fold_ref_member(h, fv->hi);
-    h = fold_fixed_member(h, fv->ca);
-    h = fold_ref_member(h, fv->gq);
-    h = fold_fixed_member(h, fv->eo);
-    h = fold_fixed_member(h, fv->ff);
+    h = fold_ref_member(h, &g_->def->hi);
+    h = fold_fixed_member(h, g_->def->ca);
+    h = fold_ref_member(h, &g_->def->gq);
+    h = fold_fixed_member(h, g_->def->eo);
+    h = fold_fixed_member(h, g_->def->ff);
     h = fold_fixed_member(h, em->fc);
     h = fold_ref_member(h, em->k_);
     h = fold_fixed_member(h, em->qo);
@@ -332,27 +332,24 @@ static void test_built(void) {
     h = fold_fixed_member(h, em->py);
     h = fold_fixed_member(h, em->pk);
     h = fold_fixed_member(h, em->ki);
-    h = fold_ref_member(h, fv->kq);
-    h = fold_fixed_member(h, fv->ql);
-    h = fold_fixed_member(h, fv->fg);
-    h = fold_ref_member(h, fv->rb);
+    h = fold_ref_member(h, &g_->def->kq);
+    h = fold_fixed_member(h, g_->def->ql);
+    h = fold_fixed_member(h, g_->def->fg);
+    h = fold_ref_member(h, g_->def->rb);
     h = fold_tags(fnv(fnv(h, 1u), 0x7Au), g_->header.cls->tags);
-    h = fold_fixed_member(h, g_->fx);
+    h = fold_fixed_member(h, g_->def->fx);
     h = fold_string_member(h, pr->dq);
     h = fold_fixed_member(h, pr->od);
     h = fold_bool_member(h, pr->fp != 0);
     h = fold_fixed_member(h, pr->dw);
     h = fold_vec(h, g_->ra);
-    h = fold_vec(h, fv->hi);
-    h = fold_vec(h, fv->gq);
-    h = fold_box(h, fv->kq);
-    assert(g_->ra == &pico8_fv_start.ra);
-    assert(fv->hi == &pico8_fv_start.hi);
-    assert(fv->gq == &pico8_fv_start.gq);
-    assert(fv->kq == &pico8_fv_start.kq);
+    h = fold_vec(h, &g_->def->hi);
+    h = fold_vec(h, &g_->def->gq);
+    h = fold_box(h, &g_->def->kq);
+    assert(g_->def == &pico8_fv_def && g_->ra == &pico8_fv_def.ra);
     assert(em->k_ == &pico8_fv_k__seq);
     assert(em->k_ != &pico8_g__k__seq);
-    assert(fv->rb == &pico8_fv_rb_row);
+    assert(g_->def->rb == &pico8_fv_rb_row);
     h = fold_string_member(h, g_->header.me);
     h = fold_fixed_member(h, g_->header.t);
     h = fnv(h, (uint32_t)size_of(em->qy));
diff --git a/tests/test_pico8_g_.c b/tests/test_pico8_g_.c
index f292cc0..6a7d93b 100644
--- a/tests/test_pico8_g_.c
+++ b/tests/test_pico8_g_.c
@@ -1,5 +1,5 @@
 /* The probe `g_`, read and written only through the dual-form field API,
- * its defaults compared to `pico8_g__start`/`pico8_fv_start`, built by
+ * its defaults read on its archetype, built by
  * its three roads (`bg:ldown`, `pico8_g__new(NULL)`, `pico8_g__new` with the
  * 4 keys of probe_mm_rnd_order) and driven through the real `mv`/`dc`/`dg`.
  * Its methods are compared to the public functions. Its daughter `fv` is
@@ -174,11 +174,11 @@ static uint32_t fold_defaults(uint32_t h, const pico8_g__t *g_) {
     const pico8_probe_t *pr = g_->probe;
 
     h = fold_ref_member(h, &g_->header.cls->tags);
-    h = fold_fixed_member(h, g_->fx);
+    h = fold_fixed_member(h, g_->def->fx);
     h = fold_string_member(h, pr->dq);
     h = fold_fixed_member(h, pr->od);
     h = fold_bool_member(h, pr->fp != 0);
-    h = fold_fixed_member(h, g_->fa);
+    h = fold_fixed_member(h, g_->def->fa);
     h = fold_fixed_member(h, pr->dw);
     h = fold_fixed_member(h, em->py);
     h = fold_fixed_member(h, em->ki);
@@ -304,7 +304,7 @@ static pico8_bg_t *the_bg(void) {
 /* A probe, whatever its form: `g_`'s tags and `fa`=2 (an `fv` has 10). */
 static int is_probe(pico8_struct_header_t *e) {
     const pico8_g__t *g_ = e != NULL ? pico8_g__of(e) : NULL;
-    return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->fa == pico8_fixed_from_int(2);
+    return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->def->fa == pico8_fixed_from_int(2);
 }
 
 static pico8_g__t *as_g_(void *e) {
@@ -348,11 +348,11 @@ static uint32_t fold_probe(uint32_t h, pico8_g__t *e) {
 
     assert(is_probe(&e->header));
     assert(e->header.cls->get_p(e) == e->p);
-    h = fold_fixed_member(h, e->fx);
+    h = fold_fixed_member(h, e->def->fx);
     h = fold_string_member(h, pr->dq);
     h = fold_fixed_member(h, pr->od);
     h = fold_bool_member(h, pr->fp != 0);
-    h = fold_fixed_member(h, e->fa);
+    h = fold_fixed_member(h, e->def->fa);
     h = fold_fixed_member(h, pr->dw);
     h = fold_fixed_member(h, em->py);
     h = fold_fixed_member(h, em->ki);
@@ -409,8 +409,8 @@ static void test_built(void) {
     h = fold_probe(h, c);
     assert(a != b && b != c && a != c);
     assert(a->emitter->qy != b->emitter->qy && b->emitter->qy != c->emitter->qy);
-    assert(a->fa == pico8_fixed_from_int(2) && a->emitter->ki == pico8_fixed_from_int(9));
-    assert(a->probe->dw == pico8_g__start.dw);
+    assert(a->def->fa == pico8_fixed_from_int(2) && a->emitter->ki == pico8_fixed_from_int(9));
+    assert(a->probe->dw == 0x666);
     assert(c->emitter->pn == pico8_fixed_from_int(7) && c->header.t == pico8_fixed_from_int(3) &&
            c->emitter->du == pico8_fixed_from_int(10) && c->emitter->py == pico8_fixed_from_int(1));
     assert(b->emitter->pn == pico8_fixed_from_int(3) && b->header.t == pico8_fixed_from_int(0));
@@ -662,7 +662,7 @@ static void test_ey_void(void) {
 /* The help: `g_`'s tags and `fa`=10. */
 static int is_help(pico8_struct_header_t *e) {
     const pico8_g__t *g_ = e != NULL ? pico8_g__of(e) : NULL;
-    return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->fa == pico8_fixed_from_int(10);
+    return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->def->fa == pico8_fixed_from_int(10);
 }
 
 /* The title screen's `fv`, whose class's parent is `g_`'s class: its
@@ -688,9 +688,9 @@ static void test_fv(void) {
     assert(found != NULL && !is_probe(found));
     fv = pico8_g__of(found);
     assert(fv->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)));
-    assert(fv->fx == pico8_fixed_from_int(1));
+    assert(fv->def->fx == pico8_fixed_from_int(1));
     assert(fv->probe->dq != NULL && strcmp(fv->probe->dq, "ly") == 0);
-    assert(fv->fa == pico8_fixed_from_int(10));
+    assert(fv->def->fa == pico8_fixed_from_int(10));
     assert(fv->emitter->k_ == &pico8_fv_k__seq);
     assert(fv->emitter->k_ != &pico8_g__k__seq);
     assert(cls_of(fv)->ev == pico8_dl_dispatch);
@@ -748,8 +748,14 @@ static void test_g__is_a_struct(void) {
     }
     assert(pico8_g__ready());
 
-    roads[1]->fa = 0;
-    assert(!is_probe(&roads[1]->header));
+    {
+        pico8_g__def_t fa_zero = pico8_g__def;
+        pico8_g__t bare;
+
+        fa_zero.fa = 0;
+        pico8_g__struct_init(&bare, cls, &fa_zero);
+        assert(is_a_g__struct(&bare) && !is_probe(&bare.header));
+    }
 
     pico8__init();
     hv = pico8_hv();
diff --git a/tests/test_pico8_jo_spec_defaults.c b/tests/test_pico8_jo_spec_defaults.c
index 721ec61..8c86fa8 100644
--- a/tests/test_pico8_jo_spec_defaults.c
+++ b/tests/test_pico8_jo_spec_defaults.c
@@ -146,6 +146,7 @@ static void test_hj_defaults(void) {
     assert(hj->qh.px == pico8_hj_start.px);
     assert(hj->qh.probe->od == pico8_hj_start.od);
     assert(hj->qh.s == pico8_hj_start.s);
+    assert(hj->emitter->def == NULL);
     assert(hj->emitter->py == pico8_hj_start.py);
     assert(hj->emitter->ki == pico8_hj_start.ki);
     assert(hj->emitter->pn == pico8_hj_start.pn);
@@ -213,6 +214,7 @@ static void test_ga_defaults(void) {
     assert(ga->jo.by != NULL && ga->jo.by == other->jo.by);
     assert_same_box(ga->jo.kq, &pico8_ga_start.kq);
     assert(ga->jo.fa == pico8_ga_start.fa);
+    assert(ga->emitter->def == NULL);
     assert(ga->emitter->fc == pico8_ga_start.fc);
     assert(ga->emitter->qo == pico8_ga_start.qo);
     assert(ga->emitter->ro == pico8_ga_start.ro);
diff --git a/tests/test_pico8_js.c b/tests/test_pico8_js.c
index 4a81a51..efae9e1 100644
--- a/tests/test_pico8_js.c
+++ b/tests/test_pico8_js.c
@@ -1367,7 +1367,7 @@ static uint32_t members_title_and_game(void) {
             assert(n < 4);
             title[n++] = e;
             assert(js_of(e)->px != NULL);
-            assert(js_of(e)->fa == pico8_fv_start.ca);
+            assert(js_of(e)->fa == pico8_fv_def.ca);
             h = fold_js_members(h, e);
         }
     }
diff --git a/tests/test_pico8_map_tables.c b/tests/test_pico8_map_tables.c
index 47cffc4..0b0d40f 100644
--- a/tests/test_pico8_map_tables.c
+++ b/tests/test_pico8_map_tables.c
@@ -154,7 +154,7 @@ static void test_jo_tree_strings_are_distinct_objects(void) {
         pico8_hj_start.rr, pico8_pm_start.fz, pico8_pm_start.me, pico8_ga_start.fz, pico8_nr_start.rr,
         pico8_fr_start.rr, pico8_mx_start.rr};
     const char *const others[] = {pico8_ke_start.me, pico8_mz_def.me, pico8_mz_def.probe.dq, pico8_jc_def.me,
-        pico8_g__start.dq, pico8_mz_jj_qs};
+        pico8_g__def.probe.dq, pico8_mz_jj_qs};
     int n = (int)(sizeof tree / sizeof tree[0]);
     int i;
     int j;
diff --git a/tests/test_pico8_spec_defaults.c b/tests/test_pico8_spec_defaults.c
index 2240c3d..5a7bd31 100644
--- a/tests/test_pico8_spec_defaults.c
+++ b/tests/test_pico8_spec_defaults.c
@@ -126,65 +126,49 @@ static void test_layout_ready(void) {
     assert(!pico8_lq_ready() && !pico8_jb_ready());
 }
 
-/* A fresh fv: fv's own generated defaults over g_'s, ke's under both. */
+/* A fresh fv: its archetype's defaults copied, ke's under them. */
 static void test_fv_defaults_and_ready(void) {
     pico8_fv_t *fv = pico8_fv_new();
-
-    assert(fv->g_.fa == pico8_fv_start.fa);
-    assert(fv->g_.ra->x == pico8_fv_start.ra.x && fv->g_.ra->y == pico8_fv_start.ra.y);
-    assert(fv->g_.emitter->fc == pico8_fv_start.fc);
-    assert(fv->g_.emitter->k_ == pico8_fv_start.k_);
-    assert(fv->g_.emitter->qo == pico8_fv_start.qo);
-    assert(fv->g_.emitter->ro == pico8_fv_start.ro);
-    assert(fv->g_.emitter->qg == pico8_fv_start.qg);
-    assert(fv->g_.emitter->q_ == pico8_fv_start.q_);
-    assert(fv->g_.emitter->pn == pico8_fv_start.pn);
-    assert(fv->g_.emitter->du == pico8_fv_start.du);
-    assert(fv->g_.emitter->py == pico8_fv_start.py);
-    assert(fv->g_.emitter->pk == pico8_fv_start.pk);
-    assert(fv->g_.emitter->ki == pico8_fv_start.ki);
-    assert(fv->g_.fx == pico8_g__start.fx);
-    assert(strcmp(fv->g_.probe->dq, pico8_g__start.dq) == 0);
-    assert(fv->g_.probe->od == pico8_g__start.od);
-    assert(fv->g_.probe->fp == pico8_g__start.fp);
-    assert(fv->g_.probe->dw == pico8_g__start.dw);
-    assert(fv->hi->x == pico8_fv_start.hi.x && fv->hi->y == pico8_fv_start.hi.y);
-    assert(fv->ca == pico8_fv_start.ca);
-    assert(fv->gq->x == pico8_fv_start.gq.x && fv->gq->y == pico8_fv_start.gq.y);
-    assert(fv->eo == pico8_fv_start.eo);
-    assert(fv->ff == pico8_fv_start.ff);
-    assert(fv->kq->xl == pico8_fv_start.kq.xl && fv->kq->yt == pico8_fv_start.kq.yt);
-    assert(fv->kq->xr == pico8_fv_start.kq.xr && fv->kq->yb == pico8_fv_start.kq.yb);
-    assert(fv->ql == pico8_fv_start.ql);
-    assert(fv->fg == pico8_fv_start.fg);
-    assert(fv->rb == pico8_fv_start.rb);
-    assert(fv->g_.header.me == pico8_ke_start.me && fv->g_.header.t == pico8_ke_start.t);
+    const pico8_g__t *g_ = &fv->g_;
+
+    assert(g_->def == &pico8_fv_def && g_->emitter->def == &pico8_fv_def.emitter &&
+           g_->probe->def == &pico8_fv_def.probe);
+    assert(g_->ra == &g_->def->ra);
+    assert(g_->emitter->fc == g_->def->emitter.fc && g_->emitter->k_ == g_->def->emitter.k_);
+    assert(g_->emitter->qo == g_->def->emitter.qo && g_->emitter->ro == g_->def->emitter.ro);
+    assert(g_->emitter->qg == g_->def->emitter.qg && g_->emitter->q_ == g_->def->emitter.q_);
+    assert(g_->emitter->pn == g_->def->emitter.pn && g_->emitter->du == g_->def->emitter.du);
+    assert(g_->emitter->py == g_->def->emitter.py && g_->emitter->pk == g_->def->emitter.pk);
+    assert(g_->emitter->ki == g_->def->emitter.ki);
+    assert(g_->probe->dq == g_->def->probe.dq && g_->probe->od == g_->def->probe.od);
+    assert(g_->probe->has_od == g_->def->probe.has_od && g_->probe->fp == g_->def->probe.fp);
+    assert(g_->probe->ik == g_->def->probe.ik && g_->probe->has_ik == g_->def->probe.has_ik);
+    assert(g_->probe->dw == g_->def->probe.dw);
+    assert(g_->probe->has_od == 1 && g_->probe->ik == 0 && g_->probe->has_ik == 0);
+    assert(g_->header.me == pico8_ke_start.me && g_->header.t == pico8_ke_start.t);
     assert(pico8_fv_ready());
     pico8_help_shutdown();
     assert(!pico8_fv_ready());
 }
 
-/* A fresh probe: g_'s generated defaults and ke's; no `ra` yet. */
+/* A fresh probe: its archetype's defaults copied, ke's under them; no `ra`
+ * yet. */
 static void test_g__defaults_and_ready(void) {
     pico8_g__t *g_ = pico8_g__new(NULL);
 
-    assert(g_->fx == pico8_g__start.fx);
-    assert(strcmp(g_->probe->dq, pico8_g__start.dq) == 0);
-    assert(g_->probe->od == pico8_g__start.od);
-    assert(g_->probe->fp == pico8_g__start.fp);
-    assert(g_->fa == pico8_g__start.fa);
-    assert(g_->probe->dw == pico8_g__start.dw);
-    assert(g_->emitter->py == pico8_g__start.py);
-    assert(g_->emitter->ki == pico8_g__start.ki);
-    assert(g_->emitter->pn == pico8_g__start.pn);
-    assert(g_->emitter->pk == pico8_g__start.pk);
-    assert(g_->emitter->fc == pico8_g__start.fc);
-    assert(g_->emitter->k_ == pico8_g__start.k_);
-    assert(g_->emitter->qo == pico8_g__start.qo);
-    assert(g_->emitter->ro == pico8_g__start.ro);
-    assert(g_->emitter->qg == pico8_g__start.qg);
-    assert(g_->emitter->q_ == pico8_g__start.q_);
-    assert(g_->emitter->du == pico8_g__start.du);
+    assert(g_->def == &pico8_g__def && g_->emitter->def == &pico8_g__def.emitter &&
+           g_->probe->def == &pico8_g__def.probe);
+    assert(g_->probe->dq == g_->def->probe.dq && g_->probe->od == g_->def->probe.od);
+    assert(g_->probe->has_od == g_->def->probe.has_od && g_->probe->has_od == 1);
+    assert(g_->probe->fp == g_->def->probe.fp && g_->probe->dw == g_->def->probe.dw);
+    assert(g_->probe->ik == g_->def->probe.ik && g_->probe->has_ik == g_->def->probe.has_ik);
+    assert(g_->probe->ik == 0 && g_->probe->has_ik == 0);
+    assert(g_->emitter->py == g_->def->emitter.py && g_->emitter->ki == g_->def->emitter.ki);
+    assert(g_->emitter->pn == g_->def->emitter.pn && g_->emitter->pk == g_->def->emitter.pk);
+    assert(g_->emitter->fc == g_->def->emitter.fc && g_->emitter->k_ == g_->def->emitter.k_);
+    assert(g_->emitter->qo == g_->def->emitter.qo && g_->emitter->ro == g_->def->emitter.ro);
+    assert(g_->emitter->qg == g_->def->emitter.qg && g_->emitter->q_ == g_->def->emitter.q_);
+    assert(g_->emitter->du == g_->def->emitter.du);
     assert(g_->ra == NULL);
     assert(g_->header.me == pico8_ke_start.me && g_->header.t == pico8_ke_start.t);
     assert(pico8_g__ready());
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index a16c2d4..22ddc9f 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -1107,9 +1107,9 @@ static void test_ke_t_by_name(void) {
     zero_keys.t = 0;
     zero = pico8_g__new(&zero_keys);
     assert_t(zero, 0);
-    assert(zero->emitter->pn == pico8_g__start.pn);
-    assert(zero->emitter->du == pico8_g__start.du);
-    assert(zero->emitter->py == pico8_g__start.py);
+    assert(zero->emitter->pn == pico8_g__def.emitter.pn);
+    assert(zero->emitter->du == pico8_g__def.emitter.du);
+    assert(zero->emitter->py == pico8_g__def.emitter.py);
     h = fold_t(h, zero);
 
     pico8_ke_ks(&keyed->header, "ll");
```

**`docs/plan-3.md`** — note « Découvert à 2.13c » au bout de la ligne 2.13c : comptes (`défauts` **403 / 22**, pied **807 / 73**, `--champs` **264 ; 15 ; 20**, `inclusion-mutuelle` 17 / 16), `game_data.h` **593** lignes, total du test ponctuel **333** (`me`/`t` gardés), les six filets neufs, la signature `pico8_g__struct_init(self, cls, def)`, les poses du `def` d'émetteur (NULL chez `hj`/`ga`, à remplacer en 2.13e1/e2), la bascule des lecteurs d'émetteur et de sonde en 2.13e2, le repli s'il a servi. Le reste va au message de commit.

### Test ponctuel : ce qui part, ce qui reste

Même règle qu'en 2.13a/b : une comparaison part dans le commit qui retire sa chaîne, jamais réécrite vers `def`. `g_` et `fv` ne réécrivent pas `me` : `me` et `t` viennent encore de `pico8_ke_start` (`pico8_ke_struct_init`) et leurs comparaisons restent, seul filet de `pico8_g__def.me`/`.t` et `pico8_fv_def.me`/`.t` (jumelles 52 à 55 : `archetype_chain` seul).

| Fonction | Garde | Retire | Total asserté |
|---|---|---|---|
| départ (`p`) | — | — | 384 |
| `test_g_` | `g_.me`, `g_.t` ; `assert` nus `qy` vide, `et` NULL | 21 : `fx`, `fa`, `ra`, émetteur ×11, sonde ×7 | |
| `test_fv` | `fv.me`, `fv.t` ; `assert` nus `qy` vide, `et` NULL | 30 : `fx`, `fa`, `ra`, `hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb`, émetteur ×11, sonde ×7 | **333** |

`same_text_or_nil` reste employé (arbre `qh`), `same_vec` et `same_box` aussi ; `same_vec_or_nil` part. Le message du total garde sa forme (`"families a to e compare 333 fields: …"`).

### Comptes, mesurés sur la copie

| Mesure | `p` | `c` |
|---|---|---|
| `défauts` (lignes / fichiers) | 507 / 29 | **403 / 22** |
| pied `résidu 2` | 911 / 74 | **807 / 73** |
| `parente`, `lien-parent`, `accesseurs-q6`, `void-ptr` | 10/3, 111/27, 237/33, 8/4 | identiques |
| `inclusion-mutuelle` (`--check`) | 17 / 16, 43 lignes | **17 / 16, 43** |
| `--champs` | 274 ; 15 ; 20 | **264 ; 15 ; 20** |
| `wc -l src/pico8_game_data.h` | 642 | **593** |
| total du test ponctuel | 384 | **333** |
| casts / tests de classe | 532 / 707 | 532 / 707 |

Attribution des 104 lignes (script du PRD 2.13a : `re.findall(r'\bpico8_([a-z0-9_]+?)_start(?:_t)?\b', ligne)`, famille = la plus tardive) : `spec_defaults.c` 45, `help.c` 22, `probe.c` 17, `fv.c` 5, `game_data.h` 4 (deux types, deux `extern`), `ui.c` 3, `game_data.c` 2, `g_.c` 2, `help.h` 1, `probe.h` 1, `js.c` 1, `map_tables.c` 1. À `c`, les 403 restantes : d 179, e1 127, e2 97, rien d'autre. Fichiers : `help.c`, `help.h`, `probe.c`, `probe.h`, `test_pico8_fv.c`, `test_pico8_g_.c`, `test_pico8_js.c` sortent de `défauts` (29 → 22) ; le pied perd **un** fichier (74 → 73), `test_pico8_js.c`, que plus aucune catégorie ne compte (les six autres restent comptés par `lien-parent`, `accesseurs-q6` ou `void-ptr`).

`--champs`, lu (`diff` des sorties complètes à `p` et `c`) : onze A retirés (`pico8_g__t` `fa`, `fx` ; `pico8_fv_t` les neuf) et un `def` ajouté (`pico8_g__t def`), 274 → **264** ; `pico8_emitter_t` n'est pas un type d'entité, son `def` n'y figure pas. « 0 hors constructeur » reste **20** : `pico8_g__t def` n'y entre pas, le relevé comptant par nom et le nom `def` ayant déjà une écriture hors constructeur (l'initialiseur de `pb`, 2.13b) ; les quinze couples `<type> def` passent de 19 à 23 écritures par nom (les quatre poses neuves : `g_`, émetteur de `g_`, `hj`, `ga`), sans changer de groupe. `pico8_g__t ra` passe de 49 / 32 à 47 / 31 (les écritures `ra = NULL` et `g_.ra = &pico8_fv_start.ra` fondues en une). Aucun autre couple ne bouge.

Pied, lu : 104 lignes et un fichier ; les autres catégories ne bougent pas.

### Contrat pour 2.13d à 2.13e2 (à recopier dans leurs PRD ; complète ceux de 2.13a et 2.13b)

| Point | Sous-étape | Ce qu'elle doit faire |
|---|---|---|
| `def` de sonde de l'arbre `qh` | 2.13d | remplacer `self->probe->def = NULL;` de `pico8_qh_struct_init` par le `def` de planète de la sorte concrète (le passer à l'init comme ici) ; **épingler** la pose par un `assert` `probe->def == &pico8_X_def.probe` pour chaque sorte (réserve de 2.13b) ; ne pas basculer les lecteurs |
| `def` d'émetteur de `hj`/`pm`/`mx` | 2.13e1 | remplacer `self->emitter->def = NULL;` de `pico8_hj_struct_init` par `&def->emitter` de la sorte ; réécrire l'`assert(hj->emitter->def == NULL)` de `jo_spec_defaults` en `assert` de pose ; copier l'émetteur depuis ce `def` |
| `def` d'émetteur de `ga` | 2.13e2 | idem dans `pico8_ga_struct_new` ; `emitter.pn`/`du`/`ki`/`pk` de `ga` (0, morts) gardés par un littéral avant que leur comparaison parte (table du PRD 2.13a) |
| bascule des lecteurs d'émetteur | **2.13e2** | `probe.c:368` (`ki`), `:369` (`pk`), `particle.c:149` (`fc`) lisent `emitter->def->X` ; `ki`, `pk`, `fc` quittent `pico8_emitter_t` ; leurs copies dans les inits (`g_` ici, `hj`, `ga`) partent ; le commentaire du membre `def` part |
| bascule des lecteurs de sonde | **2.13e2** | inchangé (PRD 2.13b) ; les copies de `dw`, `dq`, `fp`, `ik`, `has_ik` de `pico8_g__struct_init` partent aussi ; les littéraux `ik == 0`, `has_ik == 0` des sondes `g_`/`fv` dans `spec_defaults` deviennent des lectures `probe->def->…` comparées aux mêmes littéraux (ils gardent un filet) |
| `me`/`t` de `g_`, `fv` | 2.13e2 | avec ceux des familles a et b, quand `pico8_ke_start` part |
| `pico8_g__struct_init` sur une sonde locale (`test_pico8_g_.c`) | — | reste le seul chemin d'un archétype de test : on ne réécrit pas `def` hors de l'init ; `test_pico8_jc.c:853`/`:893` (2.13e2) peuvent faire de même avec une copie de `pico8_g__def` à `probe.dw = 0` |
| filets uniques connus | 2.13d à 2.13e2 | table du PRD 2.13a moins `probe.has_od` de `g_` (fait ici) : `probe.has_ik` de l'arbre `qh`, `emitter.pn` de `ga`, `me`/`t`, `nf.ra`/`ui.le` |

### Anti-tassement

**Interdit** :
- réécrire une comparaison du test ponctuel vers `def` au lieu de la retirer ; retirer `me`/`t` de `g_`/`fv` avant 2.13e2 ;
- basculer un lecteur (`probe.c`, `particle.c`, `planet_display.c`) sur `emitter->def` ou `probe->def` ; retirer un champ A de `pico8_emitter_t` ou de `pico8_probe_t` ;
- laisser un `def` (instance, émetteur, sonde) indéterminé sur un porteur, ou l'écrire ailleurs que dans l'init de son porteur — **TU compris** (`test_pico8_g_.c:751` passe par l'init) ;
- un `pico8_emitter_def_none` ; un `def` propre à `pico8_fv_t` ; un cast, un `void *`, un aiguillage par classe pour atteindre `def` ;
- garder un littéral d'init de sonde (`has_od = 1`, `ik = 0`, `has_ik = 0`) au lieu de la copie depuis `def` ;
- laisser un champ A de `g_`/`fv` dans l'instance « pour les TU » ; changer un type `pico8_X_def_t` ;
- nommer `pico8_g__start` ou `pico8_fv_start`, même en commentaire ;
- toucher `test_pico8_jc.c:853`/`:893`, `test_pico8_pj.c`, `research_icon.h` ;
- changer une constante `*_HASH`/`FROZEN_*`, un `*.sha256`, `tests/e2e/`, `tools/`, le `Makefile`.

## Implementation Chunks

Un commit, **≈ 45 min** preuves comprises (essai : ≈ 12 min d'écriture et de compilation, deux passes de `make test` ; ≈ 13 min de jumelles, 51 au parent en 369 s et 59 à l'enfant en 400 s, plus 7 rejouées après les filets en 49 s ; ≈ 10 min pour les preuves 2, 4 à 8 ; note et message ≈ 10 min). Budget 50 min : la sous-étape tient sans découpage malgré ses − 104 lignes, parce que 91 d'entre elles sont des suppressions mécaniques (types, valeurs, 22 réécritures de `fv`, 45 lignes de `spec_defaults`) ; le coût est dans les jumelles.

1. appliquer le diff ; `make test`, `make e2e`, compteur, `--champs` ;
2. preuves 1 à 8 ; note du plan ; commit.

**Repli écrit** (si l'étape déborde de 1,5 fois son budget avant le commit), chacun un commit vert :
- **2.13c1** : `g_`, la signature à trois paramètres (`fv` passe déjà `&pico8_fv_def`, mais garde ses réécritures depuis `pico8_fv_start` et ses neuf membres A), le `def` d'émetteur (poses de `g_`, `hj`, `ga`) → `défauts` **459** (48 lignes qui ne nomment que `pico8_g__start`), total **355** (les 21 de `g_`, plus `fv.fx` et les sept de sa sonde, qui ne viennent plus que de l'init), filets `has_od`/`ik`/`has_ik` des deux sondes ;
- **2.13c2** : `fv` → **403**, **333**.
Comptes du repli calculés par l'attribution (48 + 55, la ligne `test_pico8_g_.c:2` qui nomme les deux part en c2), pas mesurés. Le repli est écrit dans la note « Découvert à ».

## Testing Decisions

- **Aucun TU neuf.** Six littéraux et trois `assert` de pose dans `spec_defaults`, deux `assert` NULL dans `jo_spec_defaults`, le reste est réécriture. Aucun rejeu d'`events.json` dans un TU.
- **Bon test ici** : un TU durable compare une valeur à un **littéral du cart** (`g_->probe->has_od == 1`, `g_->probe->ik == 0`), jamais l'archétype à lui-même. `spec_defaults` compare l'instance à son `def` (`g_->emitter->ki == g_->def->emitter.ki`) : cela prouve la pose et la copie, pas la valeur ; les valeurs sont gardées par les empreintes de `test_pico8_g_`/`test_pico8_fv`, leurs littéraux et les e2e (preuve 3).
- **Prior art** : preuves 1 à 8 du PRD 2.13b ; jumelles C1 à C6 de 2.12c.

### Preuves (parent = le commit 2.13b)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré à `c`.

2. **Règle 1, preuve `const`** (copie du commit ; chaque membre `def` rendu `*const def`, un à la fois, puis `cc -std=c99 -fsyntax-only -Isrc` de chaque `.c` de `src/` et `tests/`, `-DPICO8_CLASS_COVERAGE` pour les deux `class_coverage`, l'erreur `raylib.h` ôtée) : `pico8_g__t.def` → **une** erreur, `probe.c:441` (l'init ; aucune dans `tests/`, `test_pico8_g_.c:756` passe par l'init) ; `pico8_emitter_t.def` → **trois**, `probe.c:443`, `planet_family.c:292` (`hj`), `:574` (`ga`) ; `pico8_probe_t.def` → **quatre**, `probe.c:448`, `planet_family.c:67`, `slipway.c:403`, `wormhole.c:246` (l'initialiseur de `pb` n'est pas une affectation). Aucune écriture de `def` hors construction. Preuve négative : `g->def->fa = 0;` sur un `pico8_g__t *` et `v->g_.emitter->def->ki = 0;` sur un `pico8_fv_t *` → `assignment of member … in read-only object`. Les A retirés n'ont plus de membre (grep de Sortie 1 ; `--champs`).

3. **Mutations, règle du parent.** « Défaut de classe changé » : au parent, sur `pico8_g__start`/`pico8_fv_start` ou un littéral d'init ; pour une valeur que `g_` et `fv` tiennent de la même ligne d'init (`fx` et la sonde), la jumelle parente est **une ligne ajoutée** juste après l'init, dans `pico8_g__struct_new` (pour `g_`) ou `pico8_fv_struct_new` (pour `fv`), pour ne muter que la classe mesurée ; à l'enfant, sur `pico8_g__def`/`pico8_fv_def`. `make -k -j8 unit` puis `make -k e2e` ; restaurer par `git checkout -- <fichier>`. **Vert à l'enfant et rouge au parent = ÉCHEC.** Au parent, chaque mutation rougit aussi `archetype_chain` ; la colonne l'omet. e2e = conformes sur 26. N1 à N4 : `def` d'un composant pointé ailleurs (aucun lecteur attendu).


   | # | Valeur | Parent : mutation | Rouges au parent (hors `archetype_chain`) | Enfant : mutation | Rouges à l'enfant | e2e p / c |
   |---|---|---|---|---|---|---|
   | 1 | `g_.fx` | ajout `self->fx = 0x20000;` | `g_`, `spec_defaults` | `.fx = 0x10000,` → `.fx = 0x20000,` | `g_` | 26 / 26 |
   | 2 | `g_.fa` | `.fa = 0x20000,` → `.fa = 0x30000,` | `g_` | `.fa = 0x20000,` → `.fa = 0x30000,` | `g_` | 26 / 26 |
   | 3 | `g_.ra` | `self->ra = NULL;` → `self->ra = &pico8_fv_start.gq;` | `g_`, `spec_defaults` | `.has_ra = 0,` → `.has_ra = 1, .ra = {0x0, 0xb0000},` | `g_`, `spec_defaults` | 26 / 26 |
   | 4 | `g_.emitter.pn` | `.pn = 0x30000,` → `.pn = 0x40000,` | `g_` | `.pn = 0x30000` → `.pn = 0x40000` | `g_` | 9 / 9 |
   | 5 | `g_.emitter.du` | `.du = 0x20000,` → `.du = 0x30000,` | `g_` | `.du = 0x20000` → `.du = 0x30000` | `g_` | 9 / 9 |
   | 6 | `g_.emitter.py` | `.py = 0xf5c,` → `.py = 0xf5d,` | `g_` | `.py = 0xf5c` → `.py = 0xf5d` | `g_` | 17 / 17 |
   | 7 | `g_.emitter.ki` | `.ki = 0x90000,` → `.ki = 0xa0000,` | `g_` | `.ki = 0x90000` → `.ki = 0xa0000` | `g_` | 9 / 9 |
   | 8 | `g_.emitter.pk` | `.pk = 0x10000,` → `.pk = 0x0,` | `g_` | `.pk = 0x10000` → `.pk = 0x0` | `g_` | 9 / 9 |
   | 9 | `g_.emitter.fc` | `.fc = 0x10000,` → `.fc = 0x20000,` | `g_` | `.fc = 0x10000` → `.fc = 0x20000` | `g_` | 8 / 8 |
   | 10 | `g_.emitter.k_` | `.k_ = &pico8_g__k__seq,` → `.k_ = &pico8_fv_k__seq,` | `g_` | `.k_ = &pico8_g__k__seq` → `.k_ = &pico8_fv_k__seq` | `g_` | 8 / 8 |
   | 11 | `g_.emitter.qo` | `.qo = 0x1051e,` → `.qo = 0x1061e,` | `g_` | `.qo = 0x1051e` → `.qo = 0x1061e` | `g_` | 9 / 9 |
   | 12 | `g_.emitter.ro` | `.ro = -0xd70,` → `.ro = -0xe70,` | `g_` | `.ro = -0xd70` → `.ro = -0xe70` | `g_` | 9 / 9 |
   | 13 | `g_.emitter.qg` | `.qg = 0xd70,` → `.qg = 0xe70,` | `g_` | `.qg = 0xd70` → `.qg = 0xe70` | `g_` | 9 / 9 |
   | 14 | `g_.emitter.q_` | `.q_ = 0x1051e,` → `.q_ = 0x1061e,` | `g_` | `.q_ = 0x1051e` → `.q_ = 0x1061e` | `g_` | 9 / 9 |
   | 15 | `g_.probe.dw` | ajout `self->probe->dw = 0x667;` | `g_`, `spec_defaults` | `.dw = 0x666` → `.dw = 0x667` | `g_` | 26 / 26 |
   | 16 | `g_.probe.od` | ajout `self->probe->od = 0x10000;` | `g_`, `spec_defaults` | `.od = 0x0` → `.od = 0x10000` | `g_` | 8 / 8 |
   | 17 | `g_.probe.has_od` | ajout `self->probe->has_od = 0;` | **aucun** | `.has_od = 1` → `.has_od = 0` | **`spec_defaults`** (filet neuf) | 26 / 26 |
   | 18 | `g_.probe.dq` | ajout `self->probe->dq = "pr";` | `g_`, `spec_defaults` | `.dq = "ly"` → `.dq = "pr"` | `g_` | 26 / 26 |
   | 19 | `g_.probe.fp` | ajout `self->probe->fp = 0x1;` | `g_`, `spec_defaults` | `.fp = 0x0` → `.fp = 0x1` | `g_` | 24 / 24 |
   | 20 | `g_.probe.ik` | ajout `self->probe->ik = 0x10000;` | **aucun** | `.ik = 0x0` → `.ik = 0x10000` | **`spec_defaults`** (filet neuf) | 26 / 26 |
   | 21 | `g_.probe.has_ik` | ajout `self->probe->has_ik = 1;` | **aucun** | `.has_ik = 0` → `.has_ik = 1` | **`spec_defaults`** (filet neuf) | 26 / 26 |
   | 22 | `fv.fx` | ajout `self->g_.fx = 0x20000;` | `fv`, `g_`, `spec_defaults` | `.fx = 0x10000,` → `.fx = 0x20000,` | `fv`, `g_` | 26 / 26 |
   | 23 | `fv.fa` | `.fa = 0xa0000,` → `.fa = 0xb0000,` | `fv`, `g_` | `.fa = 0xa0000,` → `.fa = 0xb0000,` | `fv`, `g_` | 26 / 26 |
   | 24 | `fv.ra` | `.ra = {0x400000, 0x1c0000},` → `.ra = {0x410000, 0x1c0000},` | `fv`, `g_` | `.ra = {0x400000, 0x1c0000},` → `.ra = {0x410000, 0x1c0000},` | `fv`, `g_` | 26 / 26 |
   | 25 | `fv.hi` | `.hi = {0x220000, 0x380000},` → `.hi = {0x230000, 0x380000},` | `fv`, `g_`, `js` | `.hi = {0x220000, 0x380000},` → `.hi = {0x230000, 0x380000},` | `fv`, `g_`, `js` | 24 / 24 |
   | 26 | `fv.ca` | `.ca = 0xb0000,` → `.ca = 0xc0000,` | `fv`, `js` | `.ca = 0xb0000,` → `.ca = 0xc0000,` | `fv`, `js` | 26 / 26 |
   | 27 | `fv.gq` | `.gq = {0x0, 0xb0000},` → `.gq = {0x10000, 0xb0000},` | `fv`, `js` | `.gq = {0x0, 0xb0000},` → `.gq = {0x10000, 0xb0000},` | `fv`, `js` | 24 / 24 |
   | 28 | `fv.eo` | `.eo = -0x10000,` → `.eo = -0x20000,` | `fv` | `.eo = -0x10000,` → `.eo = -0x20000,` | `fv` | 26 / 26 |
   | 29 | `fv.ff` | `.ff = 0x10000,` → `.ff = 0x20000,` | `fv` | `.ff = 0x10000,` → `.ff = 0x20000,` | `fv` | 26 / 26 |
   | 30 | `fv.kq` | `.kq = {-0x280000,` → `.kq = {-0x290000,` | `fv` | `.kq = {-0x280000,` → `.kq = {-0x290000,` | `fv` | 26 / 26 |
   | 31 | `fv.ql` | `.ql = 0xb0000,` → `.ql = 0xc0000,` | `fv` | `.ql = 0xb0000,` → `.ql = 0xc0000,` | `fv` | 26 / 26 |
   | 32 | `fv.fg` | `.fg = 0x50000,` → `.fg = 0x60000,` | `fv` | `.fg = 0x50000,` → `.fg = 0x60000,` | `fv` | 26 / 26 |
   | 33 | `fv.rb` | `.rb = &pico8_fv_rb_row,` → `.rb = &pico8_pj_i__row,` | `fv`, `g_`, `headless_capture`, `replay_from_title` | `.rb = &pico8_fv_rb_row,` → `.rb = &pico8_pj_i__row,` | `fv`, `g_`, `headless_capture`, `replay_from_title` | 0 / 0 |
   | 34 | `fv.emitter.pn` | `.pn = 0x50000,` → `.pn = 0x60000,` | `fv`, `g_` | `.pn = 0x50000` → `.pn = 0x60000` | `fv`, `g_` | 26 / 26 |
   | 35 | `fv.emitter.du` | `.du = 0xf0000,` → `.du = 0x100000,` | `fv`, `g_` | `.du = 0xf0000` → `.du = 0x100000` | `fv`, `g_` | 26 / 26 |
   | 36 | `fv.emitter.py` | `.py = 0x1b6,` → `.py = 0x1b7,` | `fv`, `g_` | `.py = 0x1b6` → `.py = 0x1b7` | `fv`, `g_` | 26 / 26 |
   | 37 | `fv.emitter.ki` | `.ki = 0x110000,` → `.ki = 0x120000,` | `fv`, `g_` | `.ki = 0x110000` → `.ki = 0x120000` | `fv`, `g_` | 26 / 26 |
   | 38 | `fv.emitter.pk` | `.pk = 0x10000,` → `.pk = 0x0,` | `fv`, `g_` | `.pk = 0x10000` → `.pk = 0x0` | `fv`, `g_` | 26 / 26 |
   | 39 | `fv.emitter.fc` | `.fc = 0x50000,` → `.fc = 0x60000,` | `fv`, `g_` | `.fc = 0x50000` → `.fc = 0x60000` | `fv`, `g_` | 25 / 25 |
   | 40 | `fv.emitter.k_` | `.k_ = &pico8_fv_k__seq,` → `.k_ = &pico8_g__k__seq,` | `fv`, `g_` | `.k_ = &pico8_fv_k__seq` → `.k_ = &pico8_g__k__seq` | `fv`, `g_` | 26 / 26 |
   | 41 | `fv.emitter.qo` | `.qo = 0x1051e,` → `.qo = 0x1061e,` | `fv`, `g_` | `.qo = 0x1051e` → `.qo = 0x1061e` | `fv`, `g_` | 26 / 26 |
   | 42 | `fv.emitter.ro` | `.ro = -0x1ca,` → `.ro = -0x2ca,` | `fv`, `g_` | `.ro = -0x1ca` → `.ro = -0x2ca` | `fv`, `g_` | 26 / 26 |
   | 43 | `fv.emitter.qg` | `.qg = 0x1ca,` → `.qg = 0x2ca,` | `fv`, `g_` | `.qg = 0x1ca` → `.qg = 0x2ca` | `fv`, `g_` | 26 / 26 |
   | 44 | `fv.emitter.q_` | `.q_ = 0x1051e,` → `.q_ = 0x1061e,` | `fv`, `g_` | `.q_ = 0x1051e` → `.q_ = 0x1061e` | `fv`, `g_` | 26 / 26 |
   | 45 | `fv.probe.dw` | ajout `self->g_.probe->dw = 0x667;` | `fv`, `g_`, `spec_defaults` | `.dw = 0x666` → `.dw = 0x667` | `fv`, `g_` | 26 / 26 |
   | 46 | `fv.probe.od` | ajout `self->g_.probe->od = 0x10000;` | `fv`, `g_`, `spec_defaults` | `.od = 0x0` → `.od = 0x10000` | `fv`, `g_` | 26 / 26 |
   | 47 | `fv.probe.has_od` | ajout `self->g_.probe->has_od = 0;` | **aucun** | `.has_od = 1` → `.has_od = 0` | **`spec_defaults`** (filet neuf) | 26 / 26 |
   | 48 | `fv.probe.dq` | ajout `self->g_.probe->dq = "pr";` | `fv`, `g_`, `spec_defaults` | `.dq = "ly"` → `.dq = "pr"` | `fv`, `g_` | 26 / 26 |
   | 49 | `fv.probe.fp` | ajout `self->g_.probe->fp = 0x1;` | `fv`, `g_`, `spec_defaults` | `.fp = 0x0` → `.fp = 0x1` | `fv`, `g_` | 26 / 26 |
   | 50 | `fv.probe.ik` | ajout `self->g_.probe->ik = 0x10000;` | **aucun** | `.ik = 0x0` → `.ik = 0x10000` | **`spec_defaults`** (filet neuf) | 26 / 26 |
   | 51 | `fv.probe.has_ik` | ajout `self->g_.probe->has_ik = 1;` | **aucun** | `.has_ik = 0` → `.has_ik = 1` | **`spec_defaults`** (filet neuf) | 26 / 26 |
   | 52 | `g_.me` | — | — | `.me = "ph",` → `.me = "kz",` | `archetype_chain` | — / 26 |
   | 53 | `g_.t` | — | — | `.t = 0x0,` → `.t = 0x10000,` | `archetype_chain` | — / 26 |
   | 54 | `fv.me` | — | — | `.me = "ph",` → `.me = "kz",` | `archetype_chain` | — / 26 |
   | 55 | `fv.t` | — | — | `.t = 0x0,` → `.t = 0x10000,` | `archetype_chain` | — / 26 |
   | 56 | `N1 hj emitter.def` | — | — | `self->emitter->def = NULL;` → `self->emitter->def = &pico8_g__def.emitter;` | `jo_spec_defaults` | — / 26 |
   | 57 | `N2 g_ probe.def` | — | — | `self->probe->def = &self->def->probe;` → `self->probe->def = &pico8_mz_def.probe;` | `spec_defaults` | — / 26 |
   | 58 | `N3 g_ emitter.def` | — | — | `self->emitter->def = &self->def->emitter;` → `self->emitter->def = &pico8_jo_def.emitter;` | `spec_defaults` | — / 26 |
   | 59 | `N4 ga emitter.def` | — | — | `self->emitter->def = NULL;` → `self->emitter->def = &pico8_g__def.emitter;` | `jo_spec_defaults` | — / 26 |

   Lecture : **aucun ÉCHEC** (colonne « enfant » mesurée sur `c`, donc avec les filets neufs). Hors `archetype_chain`, chaque jumelle rougit à l'enfant les TU de classe et les e2e qu'elle rougit au parent, au conforme près. **Une seule différence, attendue** : au parent, `spec_defaults` rougit en plus pour `fx`, `dw`, `od`, `dq`, `fp` (lignes 1, 15, 16, 18, 19, 22, 45, 46, 48, 49), parce qu'il comparait l'instance à `pico8_g__start` — une comparaison de chaîne, qui part avec elle ; à l'enfant, il compare l'instance à `def`, que la mutation change des deux côtés. Ces lignes restent rouges à l'enfant par `g_`/`fv` (empreintes), ce n'est donc pas un ÉCHEC. **Filets uniques trouvés** (rejeu des 51 jumelles au parent, sans les filets) : sonde `has_od`, `ik`, `has_ik` de `g_` (17, 20, 21) et de `fv` (47, 50, 51) ; `has_od` de `g_` était connu (C3 de 2.12c), les cinq autres non — `ik`/`has_ik` sont des valeurs mortes (lues seulement si `has_ik`, nul), gardées quand même par la règle du parent. Premier passage à l'enfant sans les littéraux : les six vertes (sauf `g_.has_od`, déjà gardé) = ÉCHEC ; avec les littéraux de `spec_defaults` (`has_od == 1`, `ik == 0`, `has_ik == 0`, pour chaque sonde), les six rougissent `spec_defaults`. 52 à 55 montrent que `me`/`t` doivent rester. N1 à N4 ne rougissent que leurs `assert` de pose (26 e2e) : aucun lecteur de `emitter->def`/`probe->def`.

4. **Chemin touché** : `make line-coverage EVENTS="<les 26 e2e>"` sur `c` (6304 lignes) : l'init et ses poses (`probe.c:440`, `:441`, `:443`, `:445`, `:448`), les deux appels (`probe.c:477`, `help.c:151`), les poses NULL (`planet_family.c:292`, `:574`) et les lectures par `def` (`probe.c:399`, `particle.c:24`, `help.c:67`, `:68`, `:90`, `:92`, `:104`) sont exécutés. `fv_get_ql`/`fv_get_fg` (`help.c:108`, `:113`) ne sont pas exécutés par les e2e, **au parent non plus** (mesuré) ; ils sont gardés par `test_pico8_fv` (jumelles 31, 32).

5. **Valeurs observables, parent contre enfant** : un programme d'essai (`obs.c`, hors dépôt) lié aux objets de `src/` crée un `g_` et un `fv` frais et imprime 18 lignes : `me`, `t`, `fx`, `fa`, `get_fa`, `get_fx`, `ra`, les onze champs d'émetteur (`k_` par son compte et son premier élément), `qy`, les huit de sonde, puis pour `fv` `hi`, `gq`, `ca`, `eo`, `ff`, `ql`, `fg`, `kq`, `get_kq`, `get_ql`, `get_fg` et les comptes des quatre rangées de `rb`. Compilé sans `-DCHILD` sur `p` (lecture de l'instance) et avec sur `c` (lecture par `def`) : sorties **identiques**.

6. **Empreintes, inclusion, casts** : aucune constante ne change (`BUILT_HASH` de `g_` et `fv` verts) ; `inclusion-mutuelle` **17 / 16, 43 lignes** ; casts 532, tests de classe 707, comme au parent. Les lignes « cast » du diff sont les accesseurs existants (`((const pico8_g__t *)self)->fa` → `…->def->fa`, trois de `fv`).

7. `class-coverage-net` : **oui**, vert à `c` (`couverture: OK`, sortie égale à `tools/class_coverage_net_expected.txt`). `sanitize-net` : **oui** (`pico8_g__t`, `pico8_fv_t` rétrécissent, `pico8_emitter_t` grandit d'un pointeur), `sanitize: 26 propre(s)` à `c`.

8. `game_data.h` mesuré au commit : **593**.

### Greps de Sortie (arbre du commit)

1. `grep -rnE 'pico8_(g_|fv)_start' src tests` → vide.
2. `grep -rn 'const pico8_[a-z_]*_def_t \*def;' src | wc -l` → **17** (15 de 2.13b, `probe.h` pour `pico8_g__t`, `particle.h`).
3. `grep -rnE 'self->def = (&pico8_[a-z_]*_def|def);' src | wc -l` → **15** (14 de 2.13b, plus `probe.c:441` `self->def = def;`).
4. `grep -rnE 'emitter->def = |probe->def = |\.def = ' src` → exactement : `probe.c:443` (`&self->def->emitter`), `probe.c:448`, `wormhole.c:246`, `slipway.c:403` (`&self->def->probe`), `planet_family.c:67` (sonde, NULL), `planet_family.c:292`, `:574` (émetteur, NULL), `research_icon.c:180` (`{.def = &pico8_probe_def_none}`).
5. `grep -rnE 'probe->def->|emitter->def->|pb\.def' src` → vide.
6. Casts et tests de classe : `grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests | wc -l` → 532 ; `grep -rEo '(->cls|header\.cls|_struct_class\(\))' src tests | wc -l` → 707.
7. `grep -n 'g_compared ==' tests/test_pico8_archetype_chain.c` → **333**.
8. `make composition-residue` → pied **807 / 73**, `défauts 403 22`, `inclusion-mutuelle 17 16` ; `--champs` → `264 ; 15 ; 20`.
9. `git diff --stat <parent>` : les fichiers de la section « Modules » et `docs/plan-3.md`, rien d'autre.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`p` = `30d074a`, `c` = `bebe104`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout b925a63`, `git apply` du diff du PRD 2.13b, commit | `p` | 0 | `30d074a` | 3 s |
| 2 | `timeout 900 make -j8 test` ; `timeout 600 make e2e` | `p` | 0 | vert ; 26 conformes | 16 s ; < 30 s |
| 3 | compteur, `--champs`, `--check inclusion-mutuelle`, `wc -l`, total, casts, `--check défauts` + attribution | `p` | 0 | 911 / 74 ; 507 / 29 ; 274 / 15 / 20 ; 43 ; 642 ; 384 ; 532 / 707 ; c 104 | < 10 s |
| 4 | texte `c` écrit (scripts d'ancres) ; `make -k -j8 test` | `c` | 2 | 78 erreurs de compilation dans 8 TU (membres retirés, `_start` nommés) | < 1 min |
| 5 | TU réécrits ; `make -j8 test` | `c` | 2 | `same_vec_or_nil` sans appelant (`-Werror`) | < 1 min |
| 6 | helper retiré ; `make -j8 test`, `make e2e`, compteur, `--champs`, `wc -l`, total ; commit | `c` | 0 | vert ; 26 ; 807 / 73 ; 403 / 22 ; 264 / 15 / 20 ; 593 ; 333 | < 1 min |
| 7 | P1 à P51 (`mut.py` : `make -k -j8 unit` + `make -k e2e` par mutation) | `p` | 0 | tableau, preuve 3 ; six jumelles `archetype_chain` seul | 369 s |
| 8 | C1 à C55, N1 à N3 | `c` (sans filets) | 0 | `g_`/`fv` `ik`, `has_ik`, `fv.has_od` : **vertes** (ÉCHEC) ; N1 vert | 400 s |
| 9 | six littéraux (`spec_defaults`), deux `assert` NULL (`jo_spec_defaults`), commentaire de `pico8_probe_t.def` (réserve 2.13b) ; `make test`, `make e2e` ; commit amendé | `c` | 0 | vert ; 26 | < 1 min |
| 10 | rejeu des six filets et de N1, puis N4 | `c` | 0 | `spec_defaults` rouge ×6 ; `jo_spec_defaults` rouge ×2 | 56 s |
| 11 | compteur, `--champs` (et `diff` avec `p`), greps | `c`, `p` | 0 | inchangés ; 11 A − 1 `def` ; `def` 19 → 23 écritures | < 10 s |
| 12 | preuve `const` (3 membres) ; preuve négative | `c` | 0 | 1, 3, 4 erreurs aux poses ; `read-only object` ×2 | < 1 min |
| 13 | `obs.c` sur `p` et sur `c` (`-DCHILD`), `diff` | `p`, `c` | 0 | 18 lignes, identiques | < 1 min |
| 14 | `make class-coverage-net` ; `make sanitize-net` | `c` | 0 | `couverture: OK` ; `sanitize: 26 propre(s)` | 2 s ; 3 s |
| 15 | `make line-coverage EVENTS=<26 e2e>` sur `c`, puis sur `p` pour `help.c:108`/`:113` | `c`, `p` | 0 | 6304 lignes ; lignes de la preuve 4 présentes ; 108/113 absentes aux deux | 5 s ×2 |
| 16 | `rm -rf` de la copie | — | 0 | supprimée | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit 2.13b (ou un descendant qui ne touche que `docs/`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256`, `tests/e2e/` ou une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si une jumelle de la preuve 3 est verte à l'enfant et rouge au parent (hors `archetype_chain` et hors les comparaisons de `spec_defaults` à `_start`), notamment si `has_od`, `ik` ou `has_ik` d'une des deux sondes n'a pas d'`assert` à littéral dans un TU durable.
- `FAILED:` si `me`/`t` de `g_` ou de `fv` quitte le test ponctuel, si une autre comparaison de `g_`/`fv` y reste (ou y est réécrite vers `def`), ou si le total n'est pas 333.
- `FAILED:` si un champ A de `g_`/`fv` reste membre, si un `def` (instance, émetteur, sonde) est écrit hors de l'init de son porteur, TU compris (preuve 2), ou n'est pas `const pico8_X_def_t *`.
- `FAILED:` si un porteur d'émetteur ou de sonde laisse son `def` indéterminé (grep 4), si un lecteur lit `emitter->def` ou `probe->def` (grep 5), si un champ A quitte `pico8_emitter_t` ou `pico8_probe_t`, ou si une pose n'est pas épinglée (N1 à N4 verts).
- `FAILED:` si un cast ou un test de classe est ajouté, si `inclusion-mutuelle` n'est pas 17 / 16 (43 lignes).
- `FAILED:` si `défauts` n'est pas 403 (ou 459 puis 403 au repli), si le pied n'est pas 807 / 73, si `--champs` n'est pas `264 ; 15 ; 20`.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge ; si les valeurs observables (preuve 5) diffèrent.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le message du commit omet : les classes, les champs A retirés et les I copiés, la signature de l'init, les poses des `def` d'émetteur et de sonde, la table des jumelles avec leur résultat au parent, la preuve `const`, les comptes, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.13c », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- `jo`, `qh`, `nl`, `os` et la suite (2.13d à 2.13e2), sauf le contrat plus haut ; le `def` de sonde de l'arbre `qh` (2.13d).
- La bascule des lecteurs de l'émetteur et de la sonde et le retrait de leurs champs A (2.13e2) ; `test_pico8_jc.c:853`/`:893` (2.13e2).
- `pico8_ke_start` et la lecture de `me`/`t` sur `def` (2.13e2).
- Les réserves mineures sur `test_pico8_pj.c` et `research_icon.h` (fichiers non touchés).
- Les liens du noyau (2.14, 2.15), la question Q-2.10.
- `tools/composition_residue.sh` (le relevé par nom de `--champs`), son TU ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Le `def` passe par l'init** (`pico8_g__struct_init(self, cls, def)`) : une pose, dans l'init ; `fv` perd ses 22 réécritures. Même forme que 2.13d pour `pico8_jo_struct_init`.
- **G2. Tout l'émetteur et toute la sonde sont copiés depuis `def`, A compris**, et les littéraux d'init aussi : les A des composants restent membres jusqu'en 2.13e2.
- **G3. NULL, pas un émetteur nul, chez `hj`/`ga`** : même raison que le NULL de sonde de 2.13b ; pas d'émetteur hors d'un porteur, donc pas de `pico8_emitter_def_none` (contrat de 2.12c).
- **G4. L'archétype de test de `g_.fa` passe par l'init sur une sonde locale**, pas par `roads[1]->def = &fa_zero` : la preuve `const` reste à une erreur, sans exception pour les TU.
- **G5. Jumelles parentes par ligne ajoutée** pour les valeurs que `g_` et `fv` tiennent de la même ligne d'init : une mutation de `pico8_g__start.dw` changerait les deux classes, sa jumelle enfant n'en change qu'une.
- **G6. Réserve de 2.13b tenue** là où 2.13c touche : poses de `g_`/`fv` et NULL de `hj`/`ga` épinglés ; commentaire du membre `def` de sonde complété pour `pb` ; arbre `qh` renvoyé à 2.13d dans le contrat.
- **G7. `test_pico8_js.c:1370`, `test_pico8_ui.c:1110-1112`, `test_pico8_map_tables.c:157`** nomment l'archétype (`pico8_fv_def.ca`, `pico8_g__def.emitter.pn`, `pico8_g__def.probe.dq`) au lieu de `_start` : ils ne comparent pas l'archétype à lui-même (le `fa` des boutons de `pz`, l'émetteur d'une sonde à clés nulles, l'identité des chaînes) ; `test_pico8_g_.c:413` compare `dw` au littéral `0x666`.

### Surprises de mesure, pour `plan-pilot`

1. **Six filets uniques** au lieu d'un : `has_od` de `g_` (connu) et cinq non signalés — `ik`, `has_ik` des deux sondes et `has_od` de `fv` — tous des littéraux d'init ou des valeurs héritées de la ligne d'init de `g_`. Les jumelles « par ligne ajoutée » sont ce qui les trouve.
2. **Le pied perd un fichier** (`test_pico8_js.c`), pas zéro comme en 2.13b.
3. **`spec_defaults` au parent rougit plus que l'enfant** sur dix jumelles : ce sont ses comparaisons à `_start`, qui partent avec la chaîne ; les TU de classe gardent les valeurs.
4. `fv_get_ql`/`fv_get_fg` ne sont atteints par aucun e2e, au parent comme à l'enfant.

### Questions pour l'humain

Aucune.
