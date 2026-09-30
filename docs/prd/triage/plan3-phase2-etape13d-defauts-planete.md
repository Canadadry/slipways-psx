---
title: "Plan 3, étape 2.13d : la planète lit son archétype (`jo`, `qh`, `nl`, `os`) ; `défauts` 403 → 326 → 224, pied 807 / 73 → 730 / 72 → 628 / 72 — deux commits, jeu identique"
description: "Découpée en 2.13d1 (`jo`) et 2.13d2 (`qh`, `nl`, `os`). `pico8_jo_t` reçoit `const pico8_jo_def_t *def`, passé par `pico8_jo_struct_init`, `pico8_qh_struct_init`, `pico8_hj_struct_init` et `pico8_nr_struct_init` : les dix sortes posent leur propre archétype. Quittent l'instance les seuls champs A qu'aucune sorte de la famille e n'écrit encore (`ka`, `dz`, `cq`, `gq`, `ca`, `em` en d1 ; `hs`, `ba`, `nl.rr`, `nl.nl` en d2) ; les A que `hj`/`pm`/`mx`/`ga`/`nr` réécrivent (`hf`, `fa`, `kq`, `hp`, `px`, `qh`) restent membres, copiés depuis `def`, jusqu'en 2.13e1/e2. `kw_own` est mis à zéro, `g_jo_kw_shared` posé à zéro par l'init. La sonde de l'arbre `qh` reçoit `&def->probe`, épinglé. Filets uniques trouvés par 210 jumelles et traités par des `assert` à littéral. Essayé sur copie : vert, 26 e2e."
status: needs-triage
---

## Problem Statement

**Écrit sur `b256053`** (branche `claude/plan-pilot-3-7zdft0`), pendant que 2.13c s'implémentait dans l'arbre de travail (fichiers non commités non lus). Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Essais sur une seule copie jetable (scratchpad `prd213d/`, supprimée depuis). Sur cette copie, `b256053` a reçu le diff du PRD 2.13c (`docs/prd/triage/plan3-phase2-etape13c-defauts-famille-c.md`, bloc `diff`, `git apply` sans erreur), commité `672679c`. Notation : `p` = `672679c`, l'arbre d'après 2.13c ; `d1` = `9ffd506`, `d2` = `ec90d61`, les commits de la copie qui portent les deux diffs de « Ce qui change ».

Vu du mainteneur du port : 2.13a à 2.13c ont basculé les racines, les deux racines à sonde, la sonde et l'aide. 2.13d bascule la **planète** : `jo` et les trois sortes de la famille d (`qh`, `nl`, `os`), qui partagent un seul type d'archétype (`pico8_jo_def_t`, décision A de l'étude 2.9) avec les six sortes de la famille e. C'est la plus grosse sous-étape de 2.13 (− 179 lignes `défauts`) ; elle est **découpée en deux commits** (Implementation Chunks).

### Texte du plan, recopié (`docs/plan-3.md` à `b256053`)

> - 2.13 — l'init lit l'archétype, les `pico8_X_start(_t)` et la chaîne d'init disparaissent. Chaque instance reçoit `def`, posé par son `_new`. L'init copie la part I (étude, §2), et les lectures des champs A passent par `self->def`. Les champs A quittent l'instance. Les objets statiques `g_jo_kw_shared`, `g_ga_by_shared`, `g_shared_le`, `g_nf_ra`, `g_mz_jj_wrapper` restent posés par l'init. La fonction d'une famille dans le test ponctuel part dans le commit qui retire sa chaîne, après être passée à 2.12, et le TU disparaît en 2.13e2. […] Les TU `jo_spec_defaults`, `spec_defaults` et `entity_kind` sont réécrits pour comparer l'instance à son `def`. Les écritures de TU sur un champ A sont réécrites. `jo.gq`, dans `gh_row_lq` et `ga_export_panel_by`, est retiré, et la preuve du §6 est rejouée : bug remis, test rouge. […] Les comptes d'arrivée sont attribués par les noms `pico8_X_start` de chaque ligne du `--check`. Preuves : chemin touché (`gcov` des inits, mutations : défaut d'archétype changé → e2e ou TU rouge) ; règle 1 : un champ A qui quitte l'instance a sa preuve `const`, rejouée sur la copie du commit ; `game_data.h` mesuré au commit. `class-coverage-net` : oui. `sanitize-net` : oui. Budget ≈ 7 × 50 min.
>   - 2.13d — `jo`, `qh`, `nl`, `os`. `pico8_jo_struct_init` copie le `def` de la classe concrète pour les dix sortes. Les `_new` de la famille e réécrivent encore leurs `_start` par-dessus, aux mêmes valeurs. `pico8_jo_kw_start` sort. Compte : → **224**.

Notes « Découvert à » qui s'adressent à 2.13d (même fichier) :

> 2.12d — […] Pour 2.13d : […] `kw` (`&g_jo_kw_shared`) et le `by` de `ga` restent posés par l'init, hors de `pico8_jo_def_t`.
>
> 2.12e1 — […] Remplace le « pour 2.13d » de la note 2.12d sur `kw_own` : `pico8_jo_struct_init` ne copie pas `kw_own` d'un `def` ; quand `pico8_jo_kw_start` sort, `kw_own` est mis à zéro (ou laissé à `lz`, qui l'écrit avant toute lecture), et `test_pico8_jo_spec_defaults.c:74`, `:230` sont réécrits […]. Pour 2.13d/e : `probe.has_ik` de l'arbre `qh` n'a d'autre filet que le test ponctuel (jumelle H1 […]) : un TU durable doit le garder avant que le test ponctuel parte.

Étude 2.9 (`docs/etude-2-9-archetypes.md`, corrigée à 2.12e1), arbre `jo` : `jo` A `hf`, `hs`, `fa`, `kq`, `ka`, `dz`, `cq`, `gq`, `ca`, `em` ; I `ma`, `mi`, `has_mi`, `kw` (`&g_jo_kw_shared`), `by` (NULL, `&g_ga_by_shared` pour `ga`) ; **E `kw_own`**. `qh` A `hp`, `px`, `qh`, `ba` ; I `s`. `nl` A `rr`, `nl` ; I `qw`. `os` : aucun champ propre. Sonde : A `dw`, `dq`, `fp`, `ik`, `has_ik` ; I `od`, `has_od` ; E `et`. En-tête : `me` I (réécrit par les inits de `jo` et `qh`, et par `pm`), `t` I par `pico8_ke_start`. Étude 2.10, réserve 2 : les A de la sonde **restent membres de `pico8_probe_t` jusqu'en 2.13e2**.

Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que fait 2.13d |
|---|---|
| `pico8_jo_struct_init` copie le `def` de la classe concrète pour les dix sortes (plan) | `pico8_jo_struct_init(self, cls, def)` ; `pico8_qh_struct_init`, `pico8_hj_struct_init`, `pico8_nr_struct_init` prennent aussi `def` et le passent ; chaque `_new` des dix sortes passe `&pico8_X_def` (d1). Même forme que `pico8_g__struct_init(self, cls, def)` (2.13c) |
| `kw_own` est un E : pas copié d'un `def` ; mis à zéro ou laissé à `lz` ; `jo_spec_defaults.c:74`, `:230` réécrits (2.12e1) | `self->kw_own = (pico8_kw_t){0};` dans l'init (d1) : `jo` et `ga` n'ont pas d'`hp`, `lz` ne tourne pas pour eux, et « laissé à `lz` » laisserait leur `kw_own` indéterminé (`malloc`). Les deux lignes (`:74` et `:232` à `p`, `:230` avant 2.13c) comparent maintenant `kw_own` à zéro, à littéral |
| `g_jo_kw_shared` reste posé par l'init (plan ; 2.12d) | `pico8_jo_init` fait `g_jo_kw_shared = (pico8_kw_t){0};` au lieu de `*pico8_jo_kw_start.kw` ; `pico8_jo_kw_start` sort (d1). `jo->kw == pico8_jo_shared_kw()` et `kw->mu == 0 && !kw->has_qe` à littéral dans `jo_spec_defaults` |
| `probe.has_ik` de l'arbre `qh` : `assert` à littéral par sorte avant que sa comparaison parte (2.12e1 ; table du PRD 2.13a) | les huit sortes à sonde de l'arbre perdent leur comparaison `has_ik` **en d2**, pas en e1/e2 : c'est `pico8_qh_struct_init` qui l'écrit pour les huit (littéral 0), et d2 remplace ce littéral par la copie depuis `def`. Les huit reçoivent donc leur `assert(… has_ik == 0)` en d2, dans `assert_qh_tail` de `jo_spec_defaults`, qui sert les huit (preuve 3) |
| `def` de sonde de l'arbre `qh` : `&def->probe` de la sorte concrète, épinglé par `probe->def == &pico8_X_def.probe` pour chaque sorte, lecteurs non basculés (2.13b, 2.13c) | **`&self->jo.def->probe`** dans `pico8_qh_struct_init` (d2), avec la copie des sept champs depuis `def->probe` (littéraux compris) ; épinglé dans `assert_qh_tail(qh, &pico8_X_def)` pour les huit sortes (N-sonde rouges, preuve 3) ; aucun lecteur ne lit `probe->def` (grep de Sortie) |
| Filets uniques connus (tables des PRD 2.13a à 2.13c) : `probe.has_ik` de l'arbre `qh`, `emitter.pn` de `ga`, `me`/`t`, `nf.ra`/`ui.le` | `has_ik` : ci-dessus. Les autres ne sont pas de 2.13d. Rejeu des 210 jumelles qui quittent le test ponctuel : les filets uniques trouvés sont traités (preuve 3) |
| `jo.gq` retiré de `gh_row_lq` et `ga_export_panel_by`, preuve du §6 rejouée (plan ; étude 2.9, §1) | d1 : les cinq écritures `jo.gq` des TU (`planet_gh_row_lq`, `ga_export_panel_by`, trois de `lq`) partent avec le membre ; §6 rejoué à `d1` (preuve 6) |
| Réserve de 2.13b : rien n'épingle le `def` de sonde de l'arbre `qh` | tenue (d2, ci-dessus) ; le `def` d'instance des dix sortes est épinglé aussi (`assert_jo_tail(jo, &pico8_X_def)`, N-def rouges) |

### Les sortes de la famille e en attendant 2.13e1/e2

La question est : que lit une sorte de la famille e (`hj`, `pm`, `mx`, `ga`, `nr`, `fr`) entre 2.13d et le commit qui retire son `_start` ? Règle du parent : aucune lecture par `def` ne doit rendre morte une écriture `_start` encore faite (sinon la mutation de ce `_start` serait rouge au parent et verte à l'enfant).

- **Chaque sorte e pose son propre archétype**, par le même chemin que la famille d : `pico8_hj_new` passe `&pico8_hj_def` à `pico8_hj_struct_init`, `pm` `&pico8_pm_def`, `mx` `&pico8_mx_def`, `ga` `&pico8_ga_def` à `pico8_jo_struct_init`, `nr` `&pico8_nr_def` et `fr` `&pico8_fr_def` à `pico8_nr_struct_init`. Aucune ne passe l'archétype d'un parent : un champ lu par `def` (ex. `ka`) aurait sinon sa jumelle enfant (`pico8_hj_def.ka` changé) verte.
- **Ne quittent l'instance que les champs A qu'aucune sorte e n'écrit** : `ka`, `dz`, `cq`, `gq`, `ca`, `em` (d1) ; `hs` (écrit seulement par `os`, d), `ba`, `nl.rr`, `nl.nl` (d2). Leurs lecteurs (`planet_display.c`, `economy_score.c`, `planet_trade.c`) lisent `def` pour les dix sortes.
- **Les champs A qu'une sorte e réécrit restent membres**, copiés depuis `def` par l'init puis réécrits par le `_start` de la sorte, et leurs lecteurs lisent toujours l'instance : `jo.hf`, `jo.fa`, `jo.kq` (`ga`, `nr` : jusqu'en 2.13e2) ; `qh.hp`, `qh.px`, `qh.qh` (`hj`, `pm`, `mx` : jusqu'en 2.13e1). C'est la règle que 2.13b et 2.13c ont suivie pour les A des composants (sonde, émetteur).
- Les I (`ma`, `mi`, `has_mi`, `qh.s`, `nl.qw`, `probe.od`, `probe.has_od`) et `me` sont copiés depuis `def` ; `pm` réécrit encore `me`, les six réécrivent `s`, `od`, etc.
- Contrôle mesuré (preuve 3, lignes E) : neuf mutations d'un `_start` de la famille e sur un champ gardé (`hj.hp`, `hj.qh`, `pm.px`, `pm.me`, `ga.hf`, `ga.kq`, `ga.fa`, `nr.kq`, `nr.dw`) rougissent les mêmes TU à `p` et à `d2`.

### Question ouverte chez l'humain (Q-2.10, liens du noyau)

**2.13d n'en dépend pas.** Le `def` de la planète est un membre de l'instance (`pico8_jo_t`), celui de la sonde un membre de `pico8_probe_t` que l'instance stocke (`probe_own`). Aucun n'est un lien du noyau ; aucune inclusion ne change.

### Réserves mineures reçues

L'assertion tautologique `ed_fixed(ju) == 1` de `tests/test_pico8_pj.c` et la ligne de commentaire coupée court de `src/pico8_research_icon.h` : **2.13d ne touche aucun des deux fichiers** (grep de Sortie 9) ; elles restent à la première sous-étape qui les touche (au plus tard 2.13e2 pour `research_icon.h`, si la bascule des lecteurs de sonde y passe ; sinon une ligne dans le commit de 2.13e2).

### État de départ attendu (le commit 2.13c, ou un descendant qui ne touche que `docs/`)

Mesuré sur `p` (`b256053` + diff du PRD 2.13c) :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 403 22`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 807 ligne(s), 73 fichier(s)`** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 264 ; 0 écriture : 15 ; 0 hors constructeur : 20` |
| `--check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `wc -l < src/pico8_game_data.h` | 593 |
| total asserté du test ponctuel | `g_compared == 333` |
| `make test` ; `make e2e` | vert (14 s) ; 26 conformes |
| lignes `défauts` attribuées (script du PRD 2.13a, famille = la plus tardive nommée) | d **179** (d1 **77** : ne nomment que `pico8_jo_start`/`pico8_jo_kw_start` ; d2 **102** : nomment aussi `qh`/`nl`/`os`), e1 127, e2 97 |
| casts / tests de classe (grep de la réserve 4 de 2.13a) | 532 / 707 |

Un départ autre : arrêt, rapporté. Si 2.13c n'a pas été commitée à l'identique (`git diff 672679c <commit 2.13c> -- src tests` non vide ; `672679c` se refait par `git apply` du bloc du PRD 2.13c sur `b256053`), les diffs s'appliquent par ancres de texte : ils ne touchent aucun texte de 2.13c hors la ligne du total du test ponctuel, le commentaire du membre `def` de `pico8_probe_t` (d2) et les deux `assert(… emitter->def == NULL)` de `jo_spec_defaults`, laissés tels quels.

## Solution

Deux commits, chacun vert, jeu identique.

**2.13d1 — `jo`** (`défauts` 403 → **326**) :

- `const pico8_jo_def_t *def` dans `pico8_jo_t`, premier membre après `header`. `pico8_jo_struct_init(self, cls, def)` le pose une fois, juste après `pico8_ke_struct_init`. `pico8_qh_struct_init`, `pico8_hj_struct_init` et `pico8_nr_struct_init` (`static`) prennent `def` et le passent ; les dix `_new` passent `&pico8_X_def` de leur sorte.
- **Quittent l'instance** (6 A, aucun écrivain hors de l'init de `jo`) : `ka`, `dz`, `cq`, `gq`, `ca`, `em`. Lecteurs : `planet_display.c` (`jo_pz_frame` : `ca`, `&def->gq`) ; `ka`, `dz`, `cq`, `em` n'ont pas de lecteur dans `src/` (le jeu lit `pico8_jo_dz()`, etc.) et ne vivent plus que dans l'archétype.
- **Copiés depuis `def`** : `hf`, `hs`, `fa`, `kq` (`&def->kq`) — A gardés membres (écrits encore par `qh`, `os`, `ga`, `nr`) ; I `ma`, `mi`, `has_mi` (0 et 0 écrits jusqu'ici par `jo`, puis réécrits par `qh`) ; `header.me`. `kw = &g_jo_kw_shared` inchangé ; `kw_own` mis à zéro ; `g_jo_kw_shared` posé à zéro par `pico8_jo_init`.
- `pico8_jo_start_t`, `pico8_jo_start`, `pico8_jo_kw_start` et leurs `extern` sortent.
- TU : les plis et les identités lisent `jo->def->X` ; les cinq écritures `jo.gq` des TU partent ; `archetype_chain` perd 102 comparaisons ; `jo_spec_defaults` compare à `def`, épingle les dix `def`, et reçoit les littéraux des filets uniques.

**2.13d2 — `qh`, `nl`, `os`** (`défauts` 326 → **224**) :

- `pico8_qh_struct_init` copie tout ce qu'elle écrivait depuis `def` : `hp`, `px`, `qh` (A gardés membres : `hj`, `pm`, `mx` les réécrivent jusqu'en 2.13e1), `s`, et la sonde, dont le `def` passe de NULL à `&self->jo.def->probe` et dont les littéraux d'init (`has_od = 1`, `ik = 0`, `has_ik = 0`) deviennent des copies. Ses réécritures de `mi`, `has_mi`, `hf`, `me` partent : l'init de `jo` les copie déjà du même `def`.
- **Quittent l'instance** : `qh.ba`, `nl.rr`, `nl.nl`, `jo.hs` (plus aucun écrivain : `os` ne le réécrit plus). Lecteurs : `planet_trade.c` (`ba`), `economy_score.c` (`hs`) ; `nl.rr`/`nl.nl` n'ont pas de lecteur dans `src/`.
- `pico8_nl_struct_new` ne garde que `self->qw = self->qh.jo.def->qw;` (I, réécrit par `nl:ly`) ; `pico8_os_struct_new` n'a plus rien à écrire.
- `pico8_qh_start_t`, `pico8_nl_start_t`, `pico8_os_start_t`, leurs valeurs et leurs `extern` sortent.
- TU : `archetype_chain` perd 108 comparaisons (reste **123**) ; `jo_spec_defaults` épingle la sonde des huit sortes et reçoit les littéraux de sonde ; `entity_kind`, `map_tables`, `ui`, `trade` et les TU de sorte nomment l'archétype ou lisent `def`.

Rien d'autre : ni `tools/`, ni `Makefile`, ni `glossaire-noms.md` (aucun nom minifié neuf), ni `trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`, ni les écritures `probe->dw` de `test_pico8_jc.c` (2.13e2), ni `test_pico8_pj.c`, ni `research_icon.h`.

## User Stories

1. En tant que mainteneur, je veux que `jo`, `qh`, `nl` et `os` lisent leurs défauts sur leur archétype, afin que `pico8_jo_start`, `pico8_jo_kw_start`, `pico8_qh_start`, `pico8_nl_start` et `pico8_os_start` disparaissent sans changer une valeur du jeu.
2. En tant que gardien de la règle 1, je veux que chaque champ A que plus personne n'écrit quitte l'instance, et que chaque `def` (instance, sonde) soit `const` et posé une seule fois par l'init de son porteur, afin qu'aucune écriture hors construction ne soit possible — TU compris.
3. En tant qu'auteur de 2.13e1 et 2.13e2, je veux que les six sortes de la famille e posent déjà leur propre archétype, et savoir quels champs A restent membres à cause de leurs `_start`, afin de ne ni lire un archétype faux ni rendre morte une écriture `_start` encore gardée.
4. En tant que gardien de la règle du parent, je veux qu'aucune valeur de l'arbre `jo` ne perde son dernier filet quand sa comparaison part du test ponctuel.
5. En tant que gardien du test ponctuel, je veux que seules les comparaisons dont la chaîne existe encore restent (`t` par `pico8_ke_start`, et ce que les `_start` de la famille e écrivent).
6. En tant que gardien des tests de régression (§6), je veux que `planet_gh_row_lq` et `ga_export_panel_by`, qui perdent leur écriture de `jo.gq`, rougissent encore quand leur bogue est remis.
7. En tant que gardien du compteur, je veux `défauts` 326 puis 224, sans nouvelle paire d'inclusion, et une explication chiffrée de `--champs`.

## Implementation Decisions

### Modules

- **d1** : `game_data`, `planet`, `planet_display`, `planet_family`, `planet_family_trade` (signatures seules) ; TU `archetype_chain`, `entity_kind`, `fr`, `ga`, `ga_export_panel_by` (§6), `hj`, `jo`, `jo_spec_defaults`, `lq`, `map_tables`, `mx`, `nl`, `nr`, `os`, `planet_gh_row_lq` (§6), `pm`, `qh`, `ui` ; `docs/plan-3.md`.
- **d2** : `economy_score`, `game_data`, `planet` (le membre `hs`), `planet_family`, `planet_family_trade.h` (commentaires), `planet_trade`, `probe.h` (commentaire du membre `def`) ; TU `archetype_chain`, `entity_kind`, `fr`, `hj`, `jo`, `jo_spec_defaults`, `map_tables`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `trade`, `ui` ; `docs/plan-3.md`.

### Inclusions

`planet.h` inclut déjà `pico8_game_data.h` (le type `pico8_jo_def_t` y est depuis 2.12d). Aucune inclusion neuve. Mesuré : `inclusion-mutuelle` **17 / 16, 43 lignes** à `d1` et à `d2`.

### Ce qui change

Les deux diffs ci-dessous sont le texte exact (`git diff p d1`, `git diff d1 d2` sur la copie). Lecture, en plus de la Solution :

- **Signatures** : `pico8_jo_struct_init(self, cls, def)`, `pico8_qh_struct_init(self, cls, def)`, `pico8_hj_struct_init(self, cls, def)`, `pico8_nr_struct_init(self, cls, def)` (`static`). C'est la forme de `pico8_g__struct_init` (2.13c) : le `def` est posé une fois, dans l'init de `jo`, sans précondition implicite ; les inits de sorte le lisent par `self->jo.def`.
- **Ordre dans l'init de `jo`** : `pico8_ke_struct_init`, `self->def`, puis les copies (`self->def->X`), puis les membres nuls, puis `kw_own` à zéro et `header.me`.
- **`kw_own` et `g_jo_kw_shared`** : `(pico8_kw_t){0}`. `jo_spec_defaults` les compare à zéro à littéral (`mu == 0`, `!has_qe`, `qe == 0` pour `kw_own`).
- **`gq`** : le membre est un pointeur ; il devient `&self->def->gq` à la lecture. Les plis de présence `(jo->gq != NULL)` des TU deviennent `fnv(h, 1u)` (`&def->gq != NULL` est refusé par `-Werror=address`) : même valeur pliée, les `BUILT_HASH` ne bougent pas.
- **Plis et identités des TU de sorte** : `jo->X` → `jo->def->X` au même rang ; `jo->gq == &pico8_jo_start.gq` devient l'épingle `jo->def == &pico8_X_def` ; `jo->kq == &pico8_jo_start.kq` devient `jo->kq == &pico8_X_def.kq` (`nr`, `fr` : `!= &pico8_jo_def.kq`). En d2, `qh->ba` → `qh->jo.def->ba`, `nl->rr`/`nl->nl` → `nl->qh.jo.def->…`, `jo->hs` → `jo->def->hs`.
- **`entity_kind`, `map_tables`, `ui`** nomment l'archétype (`pico8_jo_def.me`, `pico8_qh_def.s`, `pico8_qh_def.probe.dq`…) au lieu de `_start` : ils ne comparent pas l'archétype à lui-même (enregistrement de genre généré, identité des chaînes, identité de `me`). Les lignes qui nomment aussi un `_start` de la famille e restent comptées jusqu'à elle.
- **Filets neufs** (preuve 3), dans `jo_spec_defaults` : `assert_jo_box` (la boîte `{-7, -7, 7, 7}` à littéral, pour les sept sortes qui l'ont), `ka`/`dz`/`cq` de `jo` contre les objets générés, `mi == 0 && !has_mi` pour `jo` et `ga`, `hs == 1` pour `ga` (d1) ; `has_od == 1`, `ik == 0`, `has_ik == 0` dans `assert_qh_tail` pour les huit sortes à sonde (d2).
- **Épingles** : `assert_jo_tail(jo, &pico8_X_def)` commence par `jo->def == def` (dix sortes, d1) ; `assert_qh_tail(qh, &pico8_X_def)` commence par `qh->probe->def == &def->probe` (huit sortes, d2).
- **Commentaires** : ceux de `pico8_jo_t`, `pico8_qh_t`, `pico8_nl_t`, `pico8_os_t` et des trois inits sont réécrits pour rester vrais (contrat, sans récit) ; les mentions « `ba` at qh's » des structs `hj`, `pm`, `nr`, `fr`, `mx` partent avec le membre ; le membre `def` de `pico8_probe_t` ne nomme plus que `pb` (plus aucun porteur à NULL). `jo_pz_frame` : « `ep` and `ra` nil when NULL, `ca` and `gq` always ».
- **Test ponctuel** : voir plus bas.

#### Diff de 2.13d1 (`git diff p d1`)

```diff
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index 3563e18..d3a63e7 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -146,8 +146,6 @@ const pico8_jb_row_t pico8_jo_dz_rows[PICO8_JO_DZ_COUNT] = {
 static const int jo_cq_items[] = {12, 36, 24, 25};
 const pico8_int_seq_t pico8_jo_cq_seq = {4, jo_cq_items};
 
-const pico8_kw_t pico8_jo_kw_start = {0, 0x0, 0x0};
-
 static const pico8_jb_item_t jo_em_items[] = {{.is_false = 1}, {.n = 3}, {.n = 0}, {.n = -1}};
 const pico8_jb_row_t pico8_jo_em_row = {.count = 4, .items = jo_em_items};
 
@@ -438,22 +436,6 @@ static const char nr_rr_text[] = "slipgates";
 static const char fr_rr_text[] = "infraspace";
 static const char mx_rr_text[] = "void synthesis";
 
-const pico8_jo_start_t pico8_jo_start = {
-    .me = jo_me_text,
-    .hf = 1,
-    .hs = 0x10000,
-    .fa = 0x60000,
-    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
-    .ma = 0x0,
-    .ka = &pico8_jo_ka_seq,
-    .dz = pico8_jo_dz_rows,
-    .cq = &pico8_jo_cq_seq,
-    .kw = &pico8_jo_kw_start,
-    .gq = {0x0, 0xb0000},
-    .ca = 0x90000,
-    .em = &pico8_jo_em_row,
-};
-
 const pico8_qh_start_t pico8_qh_start = {
     .hp = 0x10000,
     .px = &pico8_qh_px_row,
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index 5cc275c..cb7c978 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -307,22 +307,6 @@ typedef struct {
 
 /* The jo tree: each class's own keys only, the inherited ones are its parent's;
  * ga.by, an empty state list, is held by its module. */
-typedef struct {
-    const char *me;
-    int hf;
-    pico8_fixed_t hs;
-    pico8_fixed_t fa;
-    pico8_qz_box_t kq;
-    pico8_fixed_t ma;
-    const pico8_int_seq_t *ka;
-    const pico8_jb_row_t *dz;
-    const pico8_int_seq_t *cq;
-    const pico8_kw_t *kw;
-    pico8_vec_t gq;
-    pico8_fixed_t ca;
-    const pico8_jb_row_t *em;
-} pico8_jo_start_t;
-
 typedef struct {
     pico8_fixed_t hp;
     const pico8_jb_row_t *px;
@@ -484,7 +468,6 @@ extern const pico8_bz_entry_t pico8_bz_table[PICO8_BZ_COUNT];
 extern const pico8_int_seq_t pico8_jo_ka_seq;
 extern const pico8_jb_row_t pico8_jo_dz_rows[PICO8_JO_DZ_COUNT];
 extern const pico8_int_seq_t pico8_jo_cq_seq;
-extern const pico8_kw_t pico8_jo_kw_start;
 extern const pico8_jb_row_t pico8_jo_em_row;
 extern const pico8_jb_row_t pico8_qh_px_row;
 extern const pico8_jb_row_t pico8_nl_px_row;
@@ -523,7 +506,6 @@ extern const pico8_jb_row_t pico8_pj_ou_row;
 /* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
  * nf.r are empty tables, held by their module. */
 extern const pico8_ke_start_t pico8_ke_start;
-extern const pico8_jo_start_t pico8_jo_start;
 extern const pico8_qh_start_t pico8_qh_start;
 extern const pico8_nl_start_t pico8_nl_start;
 extern const pico8_os_start_t pico8_os_start;
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 4ccb759..9c3ea40 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -132,21 +132,16 @@ static const pico8_struct_class_t g_jo_struct_class = {
     .set_m_ = pico8_jo_set_m_,
 };
 
-void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls) {
+void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     assert(g_jo_up && "pico8_jo_init() must run first");
     pico8_ke_struct_init(&self->header, cls);
-    self->hf = pico8_jo_start.hf;
-    self->hs = pico8_jo_start.hs;
-    self->fa = pico8_jo_start.fa;
-    self->kq = &pico8_jo_start.kq;
-    self->ma = pico8_jo_start.ma;
-    self->ka = pico8_jo_start.ka;
-    self->dz = pico8_jo_start.dz;
-    self->cq = pico8_jo_start.cq;
+    self->def = def;
+    self->hf = self->def->hf;
+    self->hs = self->def->hs;
+    self->fa = self->def->fa;
+    self->kq = &self->def->kq;
+    self->ma = self->def->ma;
     self->kw = &g_jo_kw_shared;
-    self->gq = &pico8_jo_start.gq;
-    self->ca = pico8_jo_start.ca;
-    self->em = pico8_jo_start.em;
     self->id = NULL;
     self->p = NULL;
     self->ra = NULL;
@@ -167,18 +162,18 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls) {
     self->has_kr = 0;
     self->cc = NULL;
     self->by = NULL;
-    self->mi = 0;
-    self->has_mi = 0;
+    self->mi = self->def->mi;
+    self->has_mi = self->def->has_mi;
     self->m_ = 0;
     self->has_m_ = 0;
-    self->kw_own = *pico8_jo_start.kw;
-    self->header.me = pico8_jo_start.me;
+    self->kw_own = (pico8_kw_t){0};
+    self->header.me = self->def->me;
 }
 
 static pico8_jo_t *pico8_jo_struct_new(const pico8_jo_keys_t *keys) {
     pico8_jo_t *self = malloc(sizeof *self);
 
-    pico8_jo_struct_init(self, &g_jo_struct_class);
+    pico8_jo_struct_init(self, &g_jo_struct_class, &pico8_jo_def);
     if (keys != NULL) {
         if (keys->p != NULL) {
             self->p = keys->p;
@@ -202,7 +197,7 @@ void pico8_jo_init(void) {
     pico8_jo_shutdown();
     pico8_pi_init();
     g_jo_up = 1;
-    g_jo_kw_shared = *pico8_jo_start.kw;
+    g_jo_kw_shared = (pico8_kw_t){0};
 }
 
 void pico8_jo_shutdown(void) {
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index e94350c..c2be800 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -16,27 +16,21 @@
  * for the value-returning functions), and an absent or wrong-kind field
  * reads as absent rather than crashing. */
 
-/* A bare planet: jo's own defaults (`kq`, `ka`, `dz`, `cq`, `gq` and
- * `em` point at jo's generated data in pico8_game_data.h; `kw` points at the one kw
- * every bare planet shares, written in place by `bb` and `mx:kf`, until `lz`
- * points it at the planet's own `kw_own`), its construction keys
+/* A bare planet: its kind's archetype (`kq` points into it; `kw` points at
+ * the one kw every bare planet shares, written in place by `bb` and `mx:kf`,
+ * until `lz` points it at the planet's own `kw_own`), its construction keys
  * `id`/`p`/`ra`, then every field jo's functions write on it. Every planet
  * struct begins with one, so jo's functions take it for `self`. Nothing
  * frees the struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
+    const pico8_jo_def_t *def;
     int hf;
     pico8_fixed_t hs;
     pico8_fixed_t fa;
     const pico8_qz_box_t *kq;
     pico8_fixed_t ma;
-    const pico8_int_seq_t *ka;
-    const pico8_jb_row_t *dz; /* PICO8_JO_DZ_COUNT rows */
-    const pico8_int_seq_t *cq;
     pico8_kw_t *kw;
-    const pico8_vec_t *gq;
-    pico8_fixed_t ca;
-    const pico8_jb_row_t *em;
     const char *id;
     pico8_vec_t *p;
     pico8_vec_t *ra;
@@ -94,10 +88,10 @@ pico8_kw_t *pico8_jo_shared_kw(void);
 pico8_jo_t *pico8_jo_new(const pico8_jo_keys_t *keys);
 
 /* Sets up an allocated `self` as a bare planet whose descriptor is `cls`:
- * the 31 members nil, then jo's defaults. Neither runs `jo:pq` nor `lt`:
- * a daughter's constructor does, after its own defaults. Requires
- * `pico8_jo_init()`. */
-void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls);
+ * `def` kept, the members it holds copied from it, the others nil. Neither
+ * runs `jo:pq` nor `lt`: a daughter's constructor does, after its own
+ * defaults. Requires `pico8_jo_init()`. */
+void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);
 
 void pico8_jo_pq(pico8_jo_t *self);
 
diff --git a/src/pico8_planet_display.c b/src/pico8_planet_display.c
index 1616522..39b06b7 100644
--- a/src/pico8_planet_display.c
+++ b/src/pico8_planet_display.c
@@ -115,14 +115,14 @@ static char *jo_dollar(char *s) {
     return with_suffix;
 }
 
-/* A planet's `pz` frame: `ep`, `gq` and `ra` nil when NULL, `ca` always. */
+/* A planet's `pz` frame: `ep` and `ra` nil when NULL, `ca` and `gq` always. */
 static pico8_pz_frame_t jo_pz_frame(const pico8_jo_t *self) {
     pico8_pz_frame_t frame = {0};
 
     frame.ep = self->ep;
     frame.has_ca = 1;
-    frame.ca = self->ca;
-    frame.gq = self->gq;
+    frame.ca = self->def->ca;
+    frame.gq = &self->def->gq;
     frame.ra = self->ra;
     return frame;
 }
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index 600c2d0..a5e3ce4 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -59,8 +59,8 @@ static const pico8_struct_class_t g_qh_struct_class = {
     .set_m_ = pico8_jo_set_m_,
 };
 
-void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls) {
-    pico8_jo_struct_init(&self->jo, cls);
+void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
+    pico8_jo_struct_init(&self->jo, cls, def);
     self->hp = pico8_qh_start.hp;
     self->px = pico8_qh_start.px;
     self->probe = &self->probe_own;
@@ -86,7 +86,7 @@ static pico8_qh_t *pico8_qh_struct_new(const pico8_jo_keys_t *keys) {
     pico8_qh_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_qh_struct_init(self, &g_qh_struct_class);
+    pico8_qh_struct_init(self, &g_qh_struct_class, &pico8_qh_def);
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "qh: the only construction key is id (ie:ldown); another key has no member "
@@ -153,7 +153,7 @@ static pico8_nl_t *pico8_nl_struct_new(const pico8_jo_keys_t *keys) {
     pico8_nl_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_qh_struct_init(&self->qh, &g_nl_struct_class);
+    pico8_qh_struct_init(&self->qh, &g_nl_struct_class, &pico8_nl_def);
     self->rr = pico8_nl_start.rr;
     self->nl = pico8_nl_start.nl;
     self->qw = pico8_nl_start.qw;
@@ -212,7 +212,7 @@ static pico8_os_t *pico8_os_struct_new(const pico8_jo_keys_t *keys) {
     pico8_os_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_qh_struct_init(&self->qh, &g_os_struct_class);
+    pico8_qh_struct_init(&self->qh, &g_os_struct_class, &pico8_os_def);
     self->qh.hp = pico8_os_start.hp;
     self->qh.px = pico8_os_start.px;
     self->qh.qh = pico8_os_start.qh;
@@ -285,8 +285,8 @@ static const pico8_struct_class_t g_hj_struct_class = {
     .set_m_ = pico8_jo_set_m_,
 };
 
-void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls) {
-    pico8_qh_struct_init(&self->qh, cls);
+void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
+    pico8_qh_struct_init(&self->qh, cls, def);
     self->rr = pico8_hj_start.rr;
     self->emitter = &self->emitter_own;
     self->emitter->def = NULL;
@@ -313,7 +313,7 @@ static pico8_hj_t *pico8_hj_struct_new(const pico8_jo_keys_t *keys) {
     pico8_hj_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_hj_struct_init(self, &g_hj_struct_class);
+    pico8_hj_struct_init(self, &g_hj_struct_class, &pico8_hj_def);
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "hj: the only construction key is id (ie:ldown); another key has no member "
@@ -394,7 +394,7 @@ static pico8_pm_t *pico8_pm_struct_new(const pico8_jo_keys_t *keys) {
     pico8_pm_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_hj_struct_init(&self->hj, &g_pm_struct_class);
+    pico8_hj_struct_init(&self->hj, &g_pm_struct_class, &pico8_pm_def);
     self->ng = &pico8_pm_start.ng;
     self->fz = pico8_pm_start.fz;
     self->hj.emitter->py = pico8_pm_start.py;
@@ -568,7 +568,7 @@ static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
     pico8_ga_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_jo_struct_init(&self->jo, &g_ga_struct_class);
+    pico8_jo_struct_init(&self->jo, &g_ga_struct_class, &pico8_ga_def);
     self->fz = pico8_ga_start.fz;
     self->emitter = &self->emitter_own;
     self->emitter->def = NULL;
diff --git a/src/pico8_planet_family.h b/src/pico8_planet_family.h
index 44e18ad..f107e18 100644
--- a/src/pico8_planet_family.h
+++ b/src/pico8_planet_family.h
@@ -114,10 +114,10 @@ int pico8_qh_ready(void);
 const pico8_entity_kind_t *pico8_qh_kind(void);
 const pico8_struct_class_t *pico8_qh_struct_class(void);
 
-/* Initializes an allocated pico8_qh_t under `cls`: jo's part, qh's members
- * nil, qh's defaults. Reads no key, runs neither `pq` nor `lt`, records no
+/* Initializes an allocated pico8_qh_t under `cls` and `def`, its kind's
+ * archetype: jo's part, qh's members nil, qh's defaults. Reads no key, runs neither `pq` nor `lt`, records no
  * coverage. Requires `pico8_planet_family_init()`. */
-void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls);
+void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);
 
 /* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
  * NULL. The new laboratory registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
@@ -159,10 +159,10 @@ const pico8_struct_class_t *pico8_hj_struct_class(void);
  * ever frees it. Requires `pico8_planet_family_init()`. */
 pico8_hj_t *pico8_hj_new(const pico8_jo_keys_t *keys);
 
-/* Initializes an allocated pico8_hj_t under `cls`: qh's part, hj's members
- * nil, hj's defaults. Reads no key, runs neither `pq` nor `lt`, records no
+/* Initializes an allocated pico8_hj_t under `cls` and `def`, its kind's
+ * archetype: qh's part, hj's members nil, hj's defaults. Reads no key, runs neither `pq` nor `lt`, records no
  * coverage. Requires `pico8_planet_family_init()`. */
-void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls);
+void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);
 
 /* The dispatchers hj's descriptor holds, for a daughter's `static const`
  * descriptor to name. */
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index a7fe593..67a7625 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -190,8 +190,8 @@ static const pico8_struct_class_t g_nr_struct_class = {
 /* Initializes an allocated pico8_nr_t under `cls`: qh's part, nr's members
  * nil, nr's defaults. Reads no key, runs neither `pq` nor `lt`, records no
  * coverage. Requires `pico8_planet_family_trade_init()`. */
-static void pico8_nr_struct_init(pico8_nr_t *self, const pico8_struct_class_t *cls) {
-    pico8_qh_struct_init(&self->qh, cls);
+static void pico8_nr_struct_init(pico8_nr_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
+    pico8_qh_struct_init(&self->qh, cls, def);
     self->hz = pico8_nr_start.hz;
     self->rr = pico8_nr_start.rr;
     self->qh.probe->od = pico8_nr_start.od;
@@ -205,7 +205,7 @@ static pico8_nr_t *pico8_nr_struct_new(const pico8_jo_keys_t *keys) {
     pico8_nr_t *self = malloc(sizeof *self);
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
-    pico8_nr_struct_init(self, &g_nr_struct_class);
+    pico8_nr_struct_init(self, &g_nr_struct_class, &pico8_nr_def);
     if (keys != NULL) {
         if (keys->id != NULL) {
             self->qh.jo.id = keys->id;
@@ -264,7 +264,7 @@ static pico8_fr_t *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
     pico8_fr_t *self = malloc(sizeof *self);
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
-    pico8_nr_struct_init(&self->nr, &g_fr_struct_class);
+    pico8_nr_struct_init(&self->nr, &g_fr_struct_class, &pico8_fr_def);
     self->mt = pico8_fr_start.mt;
     self->nr.hz = pico8_fr_start.hz;
     self->nr.rr = pico8_fr_start.rr;
@@ -370,7 +370,7 @@ static pico8_mx_t *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
     pico8_mx_t *self = malloc(sizeof *self);
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
-    pico8_hj_struct_init(&self->hj, &g_mx_struct_class);
+    pico8_hj_struct_init(&self->hj, &g_mx_struct_class, &pico8_mx_def);
     self->ht = pico8_mx_start.ht;
     self->hj.rr = pico8_mx_start.rr;
     self->hj.emitter->py = pico8_mx_start.py;
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index a5660ac..e92ffb1 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -182,20 +182,11 @@ static void test_fv(void) {
 }
 
 /* `kw`, `kw_own` and `by` are not the archetype's: left to each kind's test. */
+/* jo's part that qh's init still writes on its tree. */
 static void same_jo_part(const pico8_jo_t *jo, const pico8_jo_def_t *def, const char *kind) {
     same_in(same_text(jo->header.me, def->me), kind, "me");
     same_in(jo->header.t == def->t, kind, "t");
     same_in(jo->hf == def->hf, kind, "hf");
-    same_in(jo->hs == def->hs, kind, "hs");
-    same_in(jo->fa == def->fa, kind, "fa");
-    same_in(same_box(jo->kq, &def->kq), kind, "kq");
-    same_in(jo->ma == def->ma, kind, "ma");
-    same_in(jo->ka == def->ka, kind, "ka");
-    same_in(jo->dz == def->dz, kind, "dz");
-    same_in(jo->cq == def->cq, kind, "cq");
-    same_in(same_vec(jo->gq, &def->gq), kind, "gq");
-    same_in(jo->ca == def->ca, kind, "ca");
-    same_in(jo->em == def->em, kind, "em");
     same_in(jo->mi == def->mi, kind, "mi");
     same_in(jo->has_mi == def->has_mi, kind, "has_mi");
 }
@@ -227,7 +218,7 @@ static void assert_lz_ran(const pico8_jo_t *jo) {
 static void test_jo(void) {
     const pico8_jo_t *jo = pico8_jo_new(NULL);
 
-    same_jo_part(jo, &pico8_jo_def, "jo");
+    same_in(jo->header.t == pico8_jo_def.t, "jo", "t");
     assert(jo->kw == pico8_jo_shared_kw());
     assert(jo->by == NULL);
 }
@@ -255,6 +246,7 @@ static void test_os(void) {
     const pico8_os_t *os = pico8_os_new(NULL);
 
     same_jo_part(&os->qh.jo, &pico8_os_def, "os");
+    same_in(os->qh.jo.hs == pico8_os_def.hs, "os", "hs");
     same_qh_part(&os->qh, &pico8_os_def, "os");
     assert_lz_ran(&os->qh.jo);
 }
@@ -318,7 +310,10 @@ static void test_ga(void) {
     const pico8_ga_t *ga = pico8_ga_new(NULL);
     const pico8_ga_t *other = pico8_ga_new(NULL);
 
-    same_jo_part(&ga->jo, &pico8_ga_def, "ga");
+    same_in(ga->jo.header.t == pico8_ga_def.t, "ga", "t");
+    same_in(ga->jo.hf == pico8_ga_def.hf, "ga", "hf");
+    same_in(ga->jo.fa == pico8_ga_def.fa, "ga", "fa");
+    same_in(same_box(ga->jo.kq, &pico8_ga_def.kq), "ga", "kq");
     same_in(same_text(ga->fz, pico8_ga_def.fz), "ga", "fz");
     same_in(ga->iu == pico8_ga_def.iu, "ga", "iu");
     same_in(ga->ms == pico8_ga_def.ms, "ga", "ms");
@@ -332,6 +327,7 @@ static void test_nr(void) {
     const pico8_nr_t *nr = pico8_nr_new(NULL);
 
     same_jo_part(&nr->qh.jo, &pico8_nr_def, "nr");
+    same_in(same_box(nr->qh.jo.kq, &pico8_nr_def.kq), "nr", "kq");
     same_qh_part(&nr->qh, &pico8_nr_def, "nr");
     same_in(nr->hz == pico8_nr_def.hz, "nr", "hz");
     same_in(same_text(nr->rr, pico8_nr_def.rr), "nr", "rr");
@@ -342,6 +338,7 @@ static void test_fr(void) {
     const pico8_fr_t *fr = pico8_fr_new(NULL);
 
     same_jo_part(&fr->nr.qh.jo, &pico8_fr_def, "fr");
+    same_in(same_box(fr->nr.qh.jo.kq, &pico8_fr_def.kq), "fr", "kq");
     same_qh_part(&fr->nr.qh, &pico8_fr_def, "fr");
     same_in(fr->nr.hz == pico8_fr_def.hz, "fr", "hz");
     same_in(same_text(fr->nr.rr, pico8_fr_def.rr), "fr", "rr");
@@ -385,7 +382,7 @@ int main(void) {
     test_ga();
     test_nr();
     test_fr();
-    assert(g_compared == 333 && "families a to e compare 333 fields: one missed or one too many");
+    assert(g_compared == 231 && "families a to e compare 231 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_entity_kind.c b/tests/test_pico8_entity_kind.c
index 70d4e74..e323d4b 100644
--- a/tests/test_pico8_entity_kind.c
+++ b/tests/test_pico8_entity_kind.c
@@ -209,7 +209,7 @@ static void test_kind_reader_serves_the_generated_defaults(void) {
     rec = rec_of(8);
     assert(rec->rr == NULL);
     assert(!rec->has_s && rec->s == 0);
-    assert(rec->me == pico8_jo_start.me);
+    assert(rec->me == pico8_jo_def.me);
     assert(!rec->has_od && rec->od == 0);
 }
 
diff --git a/tests/test_pico8_fr.c b/tests/test_pico8_fr.c
index f0687c4..560acf9 100644
--- a/tests/test_pico8_fr.c
+++ b/tests/test_pico8_fr.c
@@ -269,8 +269,8 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fold_fixed_member(h, jo->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
-    h = fold_fixed_member(h, jo->ca);
-    h = fold_vec(h, jo->gq);
+    h = fold_fixed_member(h, jo->def->ca);
+    h = fold_vec(h, &jo->def->gq);
     h = fnv(h, (uint32_t)nr->hz->count);
     /* The 1s of `mt`, of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`,
      * `hs`, `fa`, `ma` and `ca`: constant by the struct's shape, kept so that
@@ -294,13 +294,13 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -339,9 +339,9 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     assert(jo->kq == &pico8_nr_start.kq);
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->px == &pico8_qh_px_row && qh->ba == &pico8_qh_ba_seq);
-    assert(jo->gq == &pico8_jo_start.gq);
-    assert(jo->em == &pico8_jo_em_row && jo->ka == &pico8_jo_ka_seq && jo->dz == pico8_jo_dz_rows &&
-           jo->cq == &pico8_jo_cq_seq);
+    assert(jo->def == &pico8_fr_def);
+    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
+           jo->def->cq == &pico8_jo_cq_seq);
     assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
     assert(pico8_gb_at(1) != NULL);
     assert(jo->hn == pico8_gb_at(1));
diff --git a/tests/test_pico8_ga.c b/tests/test_pico8_ga.c
index f4d9702..0e571e9 100644
--- a/tests/test_pico8_ga.c
+++ b/tests/test_pico8_ga.c
@@ -342,13 +342,13 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -391,12 +391,12 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     } else {
         assert(em->k_ == &pico8_ga_k__seq);
     }
-    assert(jo->gq == &pico8_jo_start.gq);
+    assert(jo->def == &pico8_ga_def);
     assert(jo->kw == pico8_jo_shared_kw());
-    assert(jo->em == &pico8_jo_em_row);
-    assert(jo->ka == &pico8_jo_ka_seq);
-    assert(jo->dz == pico8_jo_dz_rows);
-    assert(jo->cq == &pico8_jo_cq_seq);
+    assert(jo->def->em == &pico8_jo_em_row);
+    assert(jo->def->ka == &pico8_jo_ka_seq);
+    assert(jo->def->dz == pico8_jo_dz_rows);
+    assert(jo->def->cq == &pico8_jo_cq_seq);
     h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
     for (j = 0; j < (int)(sizeof bm / sizeof bm[0]); j++) {
         h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[j]), e));
diff --git a/tests/test_pico8_ga_export_panel_by.c b/tests/test_pico8_ga_export_panel_by.c
index 4267f40..7ee4a28 100644
--- a/tests/test_pico8_ga_export_panel_by.c
+++ b/tests/test_pico8_ga_export_panel_by.c
@@ -84,7 +84,6 @@ static void test_ga_export_line_counts_the_routes_in_by(void) {
     pico8_ga_t *planet;
     static pico8_kw_t kw;
     pico8_vec_t *ra;
-    pico8_vec_t *gq;
     pico8_ents_t *by;
     pico8_trade_t *route_a;
     pico8_trade_t *route_b;
@@ -107,9 +106,6 @@ static void test_ga_export_line_counts_the_routes_in_by(void) {
     ra = malloc(sizeof *ra);
     *ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
     planet->jo.ra = ra;
-    gq = malloc(sizeof *gq);
-    *gq = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(9));
-    planet->jo.gq = gq;
 
     /* A fresh `ga` exports nothing, against a capacity of `kw.mu`. */
     pico8_jo_gh(&planet->jo);
diff --git a/tests/test_pico8_hj.c b/tests/test_pico8_hj.c
index f8363ba..670acd9 100644
--- a/tests/test_pico8_hj.c
+++ b/tests/test_pico8_hj.c
@@ -333,8 +333,8 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fold_fixed_member(h, jo->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
-    h = fold_fixed_member(h, jo->ca);
-    h = fold_vec(h, jo->gq);
+    h = fold_fixed_member(h, jo->def->ca);
+    h = fold_vec(h, &jo->def->gq);
     /* The 1s of the ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
      * `dw`, `s`, then of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the
      * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
@@ -366,13 +366,13 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -399,10 +399,10 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->px != &pico8_qh_px_row);
     assert(qh->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_jo_start.kq);
-    assert(jo->gq == &pico8_jo_start.gq);
-    assert(jo->em == &pico8_jo_em_row && jo->ka == &pico8_jo_ka_seq && jo->dz == pico8_jo_dz_rows &&
-           jo->cq == &pico8_jo_cq_seq);
+    assert(jo->kq == &pico8_hj_def.kq);
+    assert(jo->def == &pico8_hj_def);
+    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
+           jo->def->cq == &pico8_jo_cq_seq);
     assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
     assert(pico8_gb_at(2) != NULL);
     assert(jo->hn == pico8_gb_at(2));
diff --git a/tests/test_pico8_jo.c b/tests/test_pico8_jo.c
index e2ed423..9b3539b 100644
--- a/tests/test_pico8_jo.c
+++ b/tests/test_pico8_jo.c
@@ -209,10 +209,10 @@ static uint32_t fold_planet(uint32_t h, const void *e) {
     h = fold_fixed_member(h, jo->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
-    h = fold_fixed_member(h, jo->ca);
+    h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, jo->p);
     h = fold_vec(h, jo->ra);
-    h = fold_vec(h, jo->gq);
+    h = fold_vec(h, &jo->def->gq);
     /* The 1s of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the struct's
      * shape, kept so that GALAXY_HASH and NEW_PLANET_HASH still prove the
      * typed reads. */
@@ -222,13 +222,13 @@ static uint32_t fold_planet(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -248,9 +248,9 @@ static uint32_t fold_planet(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)jo->has_mi);
     h = fnv(h, (uint32_t)jo->has_m_);
     assert_jo_members(cls_of(e));
-    assert(jo->kq == &pico8_jo_start.kq);
-    assert(jo->gq == &pico8_jo_start.gq);
-    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && jo->kw == pico8_jo_shared_kw() && jo->em == &pico8_jo_em_row);
+    assert(jo->kq == &pico8_jo_def.kq);
+    assert(jo->def == &pico8_jo_def);
+    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && jo->kw == pico8_jo_shared_kw() && jo->def->em == &pico8_jo_em_row);
     h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
     for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
         h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[i]), e));
@@ -347,9 +347,9 @@ static uint32_t fold_daughter(uint32_t h, void *d) {
     const pico8_jo_t *jo = as_jo(d);
     pico8_vec_t ra;
 
-    h = fold_fixed_member(h, jo->ca);
-    h = fold_vec(h, jo->gq);
-    assert(jo->em == &pico8_jo_em_row);
+    h = fold_fixed_member(h, jo->def->ca);
+    h = fold_vec(h, &jo->def->gq);
+    assert(jo->def->em == &pico8_jo_em_row);
     h = fnv(fnv(h, 0xB0u), pico8_jo_gg(jo) ? 1u : 0u);
     assert(jo->ra != NULL);
     ra = *jo->ra;
diff --git a/tests/test_pico8_jo_spec_defaults.c b/tests/test_pico8_jo_spec_defaults.c
index 8c86fa8..89a760f 100644
--- a/tests/test_pico8_jo_spec_defaults.c
+++ b/tests/test_pico8_jo_spec_defaults.c
@@ -40,16 +40,19 @@ static void assert_same_box(const pico8_qz_box_t *a, const pico8_qz_box_t *b) {
     assert(a != NULL && a == b);
 }
 
-/* The members of jo every class of the tree inherits; kw aside, which `lz`
- * points at the planet's own when it has an `hp`. */
-static void assert_jo_tail(const pico8_jo_t *jo) {
-    assert(jo->ma == pico8_jo_start.ma);
-    assert(jo->ka == pico8_jo_start.ka);
-    assert(jo->dz == pico8_jo_start.dz);
-    assert(jo->cq == pico8_jo_start.cq);
-    assert_same_vec(jo->gq, &pico8_jo_start.gq);
-    assert(jo->ca == pico8_jo_start.ca);
-    assert(jo->em == pico8_jo_start.em);
+/* The members of jo every class of the tree inherits, and its kind's
+ * archetype; kw aside, which `lz` points at the planet's own when it has an
+ * `hp`. */
+static void assert_jo_tail(const pico8_jo_t *jo, const pico8_jo_def_t *def) {
+    assert(jo->def == def);
+    assert(jo->ma == def->ma);
+    assert(jo->mi == def->mi && jo->has_mi == def->has_mi);
+}
+
+/* jo's box, which ga and nr replace. */
+static void assert_jo_box(const pico8_qz_box_t *kq) {
+    assert(kq->xl == pico8_fixed_from_int(-7) && kq->yt == pico8_fixed_from_int(-7) &&
+           kq->xr == pico8_fixed_from_int(7) && kq->yb == pico8_fixed_from_int(7));
 }
 
 /* The members of qh every class below it inherits. */
@@ -63,15 +66,18 @@ static void test_jo_defaults(void) {
     pico8_jo_t *jo = pico8_jo_new(NULL);
 
     assert(pico8_jo_ready());
-    assert_same_string(jo->header.me, pico8_jo_start.me);
-    assert(jo->hf == pico8_jo_start.hf);
-    assert(jo->hs == pico8_jo_start.hs);
-    assert(jo->fa == pico8_jo_start.fa);
-    assert_same_box(jo->kq, &pico8_jo_start.kq);
-    assert_jo_tail(jo);
+    assert_same_string(jo->header.me, jo->def->me);
+    assert(jo->hf == jo->def->hf);
+    assert(jo->hs == jo->def->hs);
+    assert(jo->fa == jo->def->fa);
+    assert_same_box(jo->kq, &jo->def->kq);
+    assert_jo_box(jo->kq);
+    assert_jo_tail(jo, &pico8_jo_def);
+    assert(jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows && jo->def->cq == &pico8_jo_cq_seq);
+    assert(jo->mi == 0 && !jo->has_mi);
     assert(jo->kw == pico8_jo_shared_kw());
-    assert(jo->kw->mu == pico8_jo_start.kw->mu && jo->kw->has_qe == pico8_jo_start.kw->has_qe);
-    assert(jo->kw_own.mu == pico8_jo_start.kw->mu && jo->kw_own.has_qe == pico8_jo_start.kw->has_qe);
+    assert(jo->kw->mu == 0 && !jo->kw->has_qe);
+    assert(jo->kw_own.mu == 0 && !jo->kw_own.has_qe && jo->kw_own.qe == 0);
 }
 
 static void test_qh_defaults(void) {
@@ -88,10 +94,11 @@ static void test_qh_defaults(void) {
     assert(qh->s == pico8_qh_start.s);
     assert(qh->ba == pico8_qh_start.ba);
     assert_qh_tail(qh);
-    assert(qh->jo.hs == pico8_jo_start.hs);
-    assert(qh->jo.fa == pico8_jo_start.fa);
-    assert_same_box(qh->jo.kq, &pico8_jo_start.kq);
-    assert_jo_tail(&qh->jo);
+    assert(qh->jo.hs == qh->jo.def->hs);
+    assert(qh->jo.fa == qh->jo.def->fa);
+    assert_same_box(qh->jo.kq, &qh->jo.def->kq);
+    assert_jo_box(qh->jo.kq);
+    assert_jo_tail(&qh->jo, &pico8_qh_def);
 }
 
 static void test_nl_defaults(void) {
@@ -111,10 +118,11 @@ static void test_nl_defaults(void) {
     assert(nl->qh.probe->fp == pico8_qh_start.fp);
     assert(nl->qh.probe->dw == pico8_qh_start.dw);
     assert_qh_tail(&nl->qh);
-    assert(nl->qh.jo.hs == pico8_jo_start.hs);
-    assert(nl->qh.jo.fa == pico8_jo_start.fa);
-    assert_same_box(nl->qh.jo.kq, &pico8_jo_start.kq);
-    assert_jo_tail(&nl->qh.jo);
+    assert(nl->qh.jo.hs == nl->qh.jo.def->hs);
+    assert(nl->qh.jo.fa == nl->qh.jo.def->fa);
+    assert_same_box(nl->qh.jo.kq, &nl->qh.jo.def->kq);
+    assert_jo_box(nl->qh.jo.kq);
+    assert_jo_tail(&nl->qh.jo, &pico8_nl_def);
 }
 
 static void test_os_defaults(void) {
@@ -132,9 +140,10 @@ static void test_os_defaults(void) {
     assert(os->qh.probe->dw == pico8_qh_start.dw);
     assert(os->qh.ba == pico8_qh_start.ba);
     assert_qh_tail(&os->qh);
-    assert(os->qh.jo.fa == pico8_jo_start.fa);
-    assert_same_box(os->qh.jo.kq, &pico8_jo_start.kq);
-    assert_jo_tail(&os->qh.jo);
+    assert(os->qh.jo.fa == os->qh.jo.def->fa);
+    assert_same_box(os->qh.jo.kq, &os->qh.jo.def->kq);
+    assert_jo_box(os->qh.jo.kq);
+    assert_jo_tail(&os->qh.jo, &pico8_os_def);
 }
 
 static void test_hj_defaults(void) {
@@ -164,10 +173,11 @@ static void test_hj_defaults(void) {
     assert(hj->qh.probe->dw == pico8_qh_start.dw);
     assert(hj->qh.ba == pico8_qh_start.ba);
     assert_qh_tail(&hj->qh);
-    assert(hj->qh.jo.hs == pico8_jo_start.hs);
-    assert(hj->qh.jo.fa == pico8_jo_start.fa);
-    assert_same_box(hj->qh.jo.kq, &pico8_jo_start.kq);
-    assert_jo_tail(&hj->qh.jo);
+    assert(hj->qh.jo.hs == hj->qh.jo.def->hs);
+    assert(hj->qh.jo.fa == hj->qh.jo.def->fa);
+    assert_same_box(hj->qh.jo.kq, &hj->qh.jo.def->kq);
+    assert_jo_box(hj->qh.jo.kq);
+    assert_jo_tail(&hj->qh.jo, &pico8_hj_def);
 }
 
 static void test_pm_defaults(void) {
@@ -198,10 +208,11 @@ static void test_pm_defaults(void) {
     assert(pm->hj.qh.probe->dw == pico8_qh_start.dw);
     assert(pm->hj.qh.ba == pico8_qh_start.ba);
     assert_qh_tail(&pm->hj.qh);
-    assert(pm->hj.qh.jo.hs == pico8_jo_start.hs);
-    assert(pm->hj.qh.jo.fa == pico8_jo_start.fa);
-    assert_same_box(pm->hj.qh.jo.kq, &pico8_jo_start.kq);
-    assert_jo_tail(&pm->hj.qh.jo);
+    assert(pm->hj.qh.jo.hs == pm->hj.qh.jo.def->hs);
+    assert(pm->hj.qh.jo.fa == pm->hj.qh.jo.def->fa);
+    assert_same_box(pm->hj.qh.jo.kq, &pm->hj.qh.jo.def->kq);
+    assert_jo_box(pm->hj.qh.jo.kq);
+    assert_jo_tail(&pm->hj.qh.jo, &pico8_pm_def);
 }
 
 static void test_ga_defaults(void) {
@@ -224,12 +235,14 @@ static void test_ga_defaults(void) {
     assert(ga->iu == pico8_ga_start.iu);
     assert(ga->emitter->py == pico8_ga_start.py);
     assert(ga->ms == pico8_ga_start.ms);
-    assert_same_string(ga->jo.header.me, pico8_jo_start.me);
-    assert(ga->jo.hs == pico8_jo_start.hs);
-    assert_jo_tail(&ga->jo);
+    assert_same_string(ga->jo.header.me, ga->jo.def->me);
+    assert(ga->jo.hs == ga->jo.def->hs);
+    assert_jo_tail(&ga->jo, &pico8_ga_def);
+    assert(ga->jo.def->hs == pico8_fixed_from_int(1));
+    assert(ga->jo.mi == 0 && !ga->jo.has_mi);
     assert(ga->jo.kw == pico8_jo_shared_kw());
-    assert(ga->jo.kw->mu == pico8_jo_start.kw->mu && ga->jo.kw->has_qe == pico8_jo_start.kw->has_qe);
-    assert(ga->jo.kw_own.mu == pico8_jo_start.kw->mu && ga->jo.kw_own.has_qe == pico8_jo_start.kw->has_qe);
+    assert(ga->jo.kw->mu == 0 && !ga->jo.kw->has_qe);
+    assert(ga->jo.kw_own.mu == 0 && !ga->jo.kw_own.has_qe && ga->jo.kw_own.qe == 0);
 }
 
 /* nr's own keys on a struct built by nr_struct_init, under nr or fr. */
@@ -240,16 +253,16 @@ static void assert_nr_own(const pico8_nr_t *nr) {
 }
 
 /* What nr and fr inherit from qh and jo. */
-static void assert_nr_inherited(const pico8_nr_t *nr) {
+static void assert_nr_inherited(const pico8_nr_t *nr, const pico8_jo_def_t *def) {
     assert(nr->qh.hp == pico8_qh_start.hp);
     assert(nr->qh.px == pico8_qh_start.px);
     assert_same_string(nr->qh.jo.header.me, pico8_qh_start.me);
     assert(nr->qh.qh == pico8_qh_start.qh);
     assert(nr->qh.ba == pico8_qh_start.ba);
     assert_qh_tail(&nr->qh);
-    assert(nr->qh.jo.hs == pico8_jo_start.hs);
-    assert(nr->qh.jo.fa == pico8_jo_start.fa);
-    assert_jo_tail(&nr->qh.jo);
+    assert(nr->qh.jo.hs == nr->qh.jo.def->hs);
+    assert(nr->qh.jo.fa == nr->qh.jo.def->fa);
+    assert_jo_tail(&nr->qh.jo, def);
 }
 
 static void test_nr_defaults(void) {
@@ -261,7 +274,7 @@ static void test_nr_defaults(void) {
     assert(nr->qh.s == pico8_nr_start.s);
     assert(nr->hz == pico8_nr_start.hz);
     assert_same_string(nr->rr, pico8_nr_start.rr);
-    assert_nr_inherited(nr);
+    assert_nr_inherited(nr, &pico8_nr_def);
 }
 
 static void test_fr_defaults(void) {
@@ -274,7 +287,7 @@ static void test_fr_defaults(void) {
     assert(fr->nr.qh.s == pico8_fr_start.s);
     assert(fr->nr.hz == pico8_fr_start.hz);
     assert_nr_own(&fr->nr);
-    assert_nr_inherited(&fr->nr);
+    assert_nr_inherited(&fr->nr, &pico8_fr_def);
 }
 
 static void test_mx_defaults(void) {
@@ -304,10 +317,11 @@ static void test_mx_defaults(void) {
     assert(mx->hj.qh.probe->dw == pico8_qh_start.dw);
     assert(mx->hj.qh.ba == pico8_qh_start.ba);
     assert_qh_tail(&mx->hj.qh);
-    assert(mx->hj.qh.jo.hs == pico8_jo_start.hs);
-    assert(mx->hj.qh.jo.fa == pico8_jo_start.fa);
-    assert_same_box(mx->hj.qh.jo.kq, &pico8_jo_start.kq);
-    assert_jo_tail(&mx->hj.qh.jo);
+    assert(mx->hj.qh.jo.hs == mx->hj.qh.jo.def->hs);
+    assert(mx->hj.qh.jo.fa == mx->hj.qh.jo.def->fa);
+    assert_same_box(mx->hj.qh.jo.kq, &mx->hj.qh.jo.def->kq);
+    assert_jo_box(mx->hj.qh.jo.kq);
+    assert_jo_tail(&mx->hj.qh.jo, &pico8_mx_def);
 }
 
 /* The second `jl()` (the one `eu` runs) finds `ke` ready and leaves it
diff --git a/tests/test_pico8_lq.c b/tests/test_pico8_lq.c
index 9102c64..76a73f0 100644
--- a/tests/test_pico8_lq.c
+++ b/tests/test_pico8_lq.c
@@ -761,7 +761,6 @@ static void test_gh_wants_more(void) {
     pico8_jo_t *planet;
     const pico8_lq_t *wants;
     const pico8_lq_t *receives;
-    pico8_vec_t *gq;
 
     start_game();
     a->has_gc = 1;
@@ -777,9 +776,6 @@ static void test_gh_wants_more(void) {
     planet->mi = pico8_fixed_from_int(1);
     planet->ra = malloc(sizeof *planet->ra);
     *planet->ra = vec_xy(64, 64);
-    gq = malloc(sizeof *gq);
-    *gq = vec_xy(0, 9);
-    planet->gq = gq;
     pico8_jo_gh(planet);
 
     wants = panel_row("wants more:");
@@ -801,7 +797,6 @@ static void test_gh_w_maker_wants_no_more(void) {
     pico8_ptrs_t *jy = malloc(sizeof *jy);
     pico8_kw_t *kw = calloc(1, sizeof *kw);
     pico8_jo_t *planet;
-    pico8_vec_t *gq;
     int r_w;
 
     start_game();
@@ -819,9 +814,6 @@ static void test_gh_w_maker_wants_no_more(void) {
     planet->mi = pico8_fixed_from_int(1);
     planet->ra = malloc(sizeof *planet->ra);
     *planet->ra = vec_xy(64, 64);
-    gq = malloc(sizeof *gq);
-    *gq = vec_xy(0, 9);
-    planet->gq = gq;
     pico8_jo_gh(planet);
 
     assert(panel_row("wants more:") == NULL && panel_row("needs:") == NULL);
@@ -834,7 +826,6 @@ static void test_gh_lab_wants_its_last(void) {
     pico8_ptrs_t *jy = malloc(sizeof *jy);
     pico8_qh_t *lab;
     const pico8_lq_t *wants;
-    pico8_vec_t *gq;
 
     start_game();
     a->has_gc = 1;
@@ -850,9 +841,6 @@ static void test_gh_lab_wants_its_last(void) {
     lab->jo.mi = pico8_fixed_from_int(1);
     lab->jo.ra = malloc(sizeof *lab->jo.ra);
     *lab->jo.ra = vec_xy(64, 64);
-    gq = malloc(sizeof *gq);
-    *gq = vec_xy(0, 9);
-    lab->jo.gq = gq;
     pico8_jo_gh(&lab->jo);
 
     wants = panel_row("wants more:");
diff --git a/tests/test_pico8_map_tables.c b/tests/test_pico8_map_tables.c
index 0b0d40f..37e262c 100644
--- a/tests/test_pico8_map_tables.c
+++ b/tests/test_pico8_map_tables.c
@@ -150,7 +150,7 @@ static void test_gn_pools_df_entries_by_their_r(void) {
 /* ob() gave each string token its own allocation: no string field of the jo
  * tree shares a pointer with another string of the generated data. */
 static void test_jo_tree_strings_are_distinct_objects(void) {
-    const char *const tree[] = {pico8_jo_start.me, pico8_qh_start.me, pico8_qh_start.dq, pico8_nl_start.rr,
+    const char *const tree[] = {pico8_jo_def.me, pico8_qh_start.me, pico8_qh_start.dq, pico8_nl_start.rr,
         pico8_hj_start.rr, pico8_pm_start.fz, pico8_pm_start.me, pico8_ga_start.fz, pico8_nr_start.rr,
         pico8_fr_start.rr, pico8_mx_start.rr};
     const char *const others[] = {pico8_ke_start.me, pico8_mz_def.me, pico8_mz_def.probe.dq, pico8_jc_def.me,
diff --git a/tests/test_pico8_mx.c b/tests/test_pico8_mx.c
index 8c7915d..7a8c517 100644
--- a/tests/test_pico8_mx.c
+++ b/tests/test_pico8_mx.c
@@ -324,8 +324,8 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fold_fixed_member(h, jo->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
-    h = fold_fixed_member(h, jo->ca);
-    h = fold_vec(h, jo->gq);
+    h = fold_fixed_member(h, jo->def->ca);
+    h = fold_vec(h, &jo->def->gq);
     h = fnv(h, (uint32_t)(ht_of(e) != NULL));
     h = fnv(h, (uint32_t)ht_size(e));
     /* The 1s of hj's ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
@@ -360,13 +360,13 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -396,10 +396,10 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(em->k_ != &pico8_hj_k__seq);
     assert(qh->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_jo_start.kq);
-    assert(jo->gq == &pico8_jo_start.gq);
-    assert(jo->em == &pico8_jo_em_row && jo->ka == &pico8_jo_ka_seq && jo->dz == pico8_jo_dz_rows &&
-           jo->cq == &pico8_jo_cq_seq);
+    assert(jo->kq == &pico8_mx_def.kq);
+    assert(jo->def == &pico8_mx_def);
+    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
+           jo->def->cq == &pico8_jo_cq_seq);
     assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
     assert(pico8_gb_at(5) != NULL);
     assert(jo->hn == pico8_gb_at(5));
diff --git a/tests/test_pico8_nl.c b/tests/test_pico8_nl.c
index 4bbe180..c1526bc 100644
--- a/tests/test_pico8_nl.c
+++ b/tests/test_pico8_nl.c
@@ -281,8 +281,8 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fold_fixed_member(h, jo->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
-    h = fold_fixed_member(h, jo->ca);
-    h = fold_vec(h, jo->gq);
+    h = fold_fixed_member(h, jo->def->ca);
+    h = fold_vec(h, &jo->def->gq);
     /* The 1s of `nl` and `qw`, of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of
      * `hf`, `hs`, `fa`, `ma` and `ca`: constant by the struct's shape, kept
      * so that BUILT_HASH still proves the typed reads. */
@@ -305,13 +305,13 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -339,10 +339,10 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->px == &pico8_nl_px_row && qh->px != &pico8_qh_px_row);
     assert(qh->ba == &pico8_nl_ba_seq && qh->ba != &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_jo_start.kq);
-    assert(jo->gq == &pico8_jo_start.gq);
-    assert(jo->em == &pico8_jo_em_row && jo->ka == &pico8_jo_ka_seq && jo->dz == pico8_jo_dz_rows &&
-           jo->cq == &pico8_jo_cq_seq);
+    assert(jo->kq == &pico8_nl_def.kq);
+    assert(jo->def == &pico8_nl_def);
+    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
+           jo->def->cq == &pico8_jo_cq_seq);
     assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
     assert(pico8_gb_at(6) != NULL);
     assert(jo->hn == pico8_gb_at(6));
diff --git a/tests/test_pico8_nr.c b/tests/test_pico8_nr.c
index 9aebc56..3039ab1 100644
--- a/tests/test_pico8_nr.c
+++ b/tests/test_pico8_nr.c
@@ -285,8 +285,8 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fold_fixed_member(h, jo->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
-    h = fold_fixed_member(h, jo->ca);
-    h = fold_vec(h, jo->gq);
+    h = fold_fixed_member(h, jo->def->ca);
+    h = fold_vec(h, &jo->def->gq);
     h = fnv(h, (uint32_t)nr->hz->count);
     /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
      * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
@@ -309,13 +309,13 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -349,11 +349,11 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     assert(jo->kq == &pico8_nr_start.kq);
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && nr->hz == &pico8_nr_hz_seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
-    assert(jo->kq != &pico8_jo_start.kq);
+    assert(jo->kq != &pico8_jo_def.kq);
     assert(qh->px == &pico8_qh_px_row && qh->ba == &pico8_qh_ba_seq);
-    assert(jo->gq == &pico8_jo_start.gq);
-    assert(jo->em == &pico8_jo_em_row && jo->ka == &pico8_jo_ka_seq && jo->dz == pico8_jo_dz_rows &&
-           jo->cq == &pico8_jo_cq_seq);
+    assert(jo->def == &pico8_nr_def);
+    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
+           jo->def->cq == &pico8_jo_cq_seq);
     assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
     assert(pico8_gb_at(1) != NULL);
     assert(jo->hn == pico8_gb_at(1));
diff --git a/tests/test_pico8_os.c b/tests/test_pico8_os.c
index dd6178e..5b1c0d4 100644
--- a/tests/test_pico8_os.c
+++ b/tests/test_pico8_os.c
@@ -311,8 +311,8 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
-    h = fold_fixed_member(h, jo->ca);
-    h = fold_vec(h, jo->gq);
+    h = fold_fixed_member(h, jo->def->ca);
+    h = fold_vec(h, &jo->def->gq);
     /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
      * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
      * still proves the typed reads. */
@@ -332,13 +332,13 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -364,10 +364,10 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->px == &pico8_os_px_row && qh->px != &pico8_qh_px_row);
     assert(qh->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_jo_start.kq);
-    assert(jo->gq == &pico8_jo_start.gq);
-    assert(jo->em == &pico8_jo_em_row && jo->ka == &pico8_jo_ka_seq && jo->dz == pico8_jo_dz_rows &&
-           jo->cq == &pico8_jo_cq_seq);
+    assert(jo->kq == &pico8_os_def.kq);
+    assert(jo->def == &pico8_os_def);
+    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
+           jo->def->cq == &pico8_jo_cq_seq);
     assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
     assert(pico8_gb_at(4) != NULL);
     assert(jo->hn == pico8_gb_at(4));
diff --git a/tests/test_pico8_planet_gh_row_lq.c b/tests/test_pico8_planet_gh_row_lq.c
index eed6973..8917522 100644
--- a/tests/test_pico8_planet_gh_row_lq.c
+++ b/tests/test_pico8_planet_gh_row_lq.c
@@ -93,7 +93,6 @@ static void test_gh_exports_row_lq_receives_its_four_items(void) {
     static pico8_kw_t kw;
     const pico8_jb_row_t *kw_row;
     pico8_vec_t *ra;
-    pico8_vec_t *gq;
     const pico8_ents_t *le;
     const pico8_lq_t *exports_lq = NULL;
     pico8_lq_cell_t entry;
@@ -112,13 +111,10 @@ static void test_gh_exports_row_lq_receives_its_four_items(void) {
     kw.mu = pico8_fixed_from_int(1);
     planet->kw = &kw;
 
-    /* `jo:gh` reads these for `pz()`'s row placement. */
+    /* `jo:gh` reads it, and the archetype's `gq`, for `pz()`'s row placement. */
     ra = malloc(sizeof *ra);
     *ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
     planet->ra = ra;
-    gq = malloc(sizeof *gq);
-    *gq = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(9));
-    planet->gq = gq;
 
     /* The call under test: this is where the overflow happened. */
     pico8_jo_gh(planet);
diff --git a/tests/test_pico8_pm.c b/tests/test_pico8_pm.c
index b3238fc..8b43bb0 100644
--- a/tests/test_pico8_pm.c
+++ b/tests/test_pico8_pm.c
@@ -321,9 +321,9 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fold_fixed_member(h, jo->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
-    h = fold_fixed_member(h, jo->ca);
+    h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, pm->ng);
-    h = fold_vec(h, jo->gq);
+    h = fold_vec(h, &jo->def->gq);
     h = fold_vec(h, jo->ra);
     h = fold_vec(h, jo->p);
     /* The 1s of hj's ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
@@ -360,13 +360,13 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -397,10 +397,10 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(em->k_ != &pico8_hj_k__seq);
     assert(qh->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_jo_start.kq);
-    assert(jo->gq == &pico8_jo_start.gq);
-    assert(jo->em == &pico8_jo_em_row && jo->ka == &pico8_jo_ka_seq && jo->dz == pico8_jo_dz_rows &&
-           jo->cq == &pico8_jo_cq_seq);
+    assert(jo->kq == &pico8_pm_def.kq);
+    assert(jo->def == &pico8_pm_def);
+    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
+           jo->def->cq == &pico8_jo_cq_seq);
     assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
     assert(pico8_gb_at(3) != NULL);
     assert(jo->hn == pico8_gb_at(3));
diff --git a/tests/test_pico8_qh.c b/tests/test_pico8_qh.c
index b8a3b3a..fd9f907 100644
--- a/tests/test_pico8_qh.c
+++ b/tests/test_pico8_qh.c
@@ -278,8 +278,8 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fold_fixed_member(h, jo->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
-    h = fold_fixed_member(h, jo->ca);
-    h = fold_vec(h, jo->gq);
+    h = fold_fixed_member(h, jo->def->ca);
+    h = fold_vec(h, &jo->def->gq);
     /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
      * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
      * still proves the typed reads. */
@@ -299,13 +299,13 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->kq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->ka != NULL));
-    h = fnv(h, (uint32_t)(jo->dz != NULL));
-    h = fnv(h, (uint32_t)(jo->cq != NULL));
+    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
+    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
+    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
     h = fnv(h, (uint32_t)(jo->kw != NULL));
-    h = fnv(h, (uint32_t)(jo->gq != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->em != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(jo->def->em != NULL));
     h = fnv(h, (uint32_t)(jo->id != NULL));
     h = fnv(h, (uint32_t)(jo->p != NULL));
     h = fnv(h, (uint32_t)(jo->ra != NULL));
@@ -329,10 +329,10 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     assert(cls_of(e)->kf == pico8_qh_struct_class()->kf && cls_of(e)->cn == pico8_qh_struct_class()->cn &&
            cls_of(e)->mm == NULL && pico8_jo_is_family(e));
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->px == &pico8_qh_px_row && qh->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_jo_start.kq);
-    assert(jo->gq == &pico8_jo_start.gq);
-    assert(jo->em == &pico8_jo_em_row && jo->ka == &pico8_jo_ka_seq && jo->dz == pico8_jo_dz_rows &&
-           jo->cq == &pico8_jo_cq_seq);
+    assert(jo->kq == &pico8_qh_def.kq);
+    assert(jo->def == &pico8_qh_def);
+    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
+           jo->def->cq == &pico8_jo_cq_seq);
     assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
     assert(pico8_gb_at(1) != NULL);
     assert(jo->hn == pico8_gb_at(1));
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index 22ddc9f..7ce7949 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -832,7 +832,7 @@ static int identity_of(const char *s) {
     if (s == pico8_ke_start.me) {
         return 1;
     }
-    if (s == pico8_jo_start.me) {
+    if (s == pico8_jo_def.me) {
         return 2;
     }
     if (s == pico8_qh_start.me) {
```

#### Diff de 2.13d2 (`git diff d1 d2`)

```diff
diff --git a/src/pico8_economy_score.c b/src/pico8_economy_score.c
index 7bfde0a..681f369 100644
--- a/src/pico8_economy_score.c
+++ b/src/pico8_economy_score.c
@@ -109,7 +109,7 @@ pico8_fixed_t pico8_db(const pico8_struct_header_t *pt) {
                 oz = (const pico8_jo_t *)oz_val;
                 if (oz->hf) {
                     pico8_fixed_t trade_mult = pico8_fixed_from_int(0);
-                    pico8_fixed_t hs = jo->hs;
+                    pico8_fixed_t hs = jo->def->hs;
                     pico8_fixed_t ct_pt = pico8_fixed_from_int(0);
                     pico8_fixed_t ct_oz = pico8_fixed_from_int(0);
                     pico8_fixed_t bonus;
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index d3a63e7..b620d17 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -436,42 +436,6 @@ static const char nr_rr_text[] = "slipgates";
 static const char fr_rr_text[] = "infraspace";
 static const char mx_rr_text[] = "void synthesis";
 
-const pico8_qh_start_t pico8_qh_start = {
-    .hp = 0x10000,
-    .px = &pico8_qh_px_row,
-    .me = qh_me_text,
-    .dq = qh_dq_text,
-    .hf = 0,
-    .qh = 1,
-    .od = 0xf0000,
-    .fp = 0x20000,
-    .dw = 0x666,
-    .mi = 0x10000,
-    .s = 0xae0000,
-    .ba = &pico8_qh_ba_seq,
-};
-
-const pico8_nl_start_t pico8_nl_start = {
-    .rr = nl_rr_text,
-    .qh = 0,
-    .nl = 0x10000,
-    .hp = 0x60000,
-    .s = 0x440000,
-    .px = &pico8_nl_px_row,
-    .od = 0x140000,
-    .qw = 0xf0000,
-    .ba = &pico8_nl_ba_seq,
-};
-
-const pico8_os_start_t pico8_os_start = {
-    .hp = 0x40000,
-    .px = &pico8_os_px_row,
-    .qh = 0,
-    .s = 0x640000,
-    .od = 0xf0000,
-    .hs = 0x8000,
-};
-
 const pico8_hj_start_t pico8_hj_start = {
     .rr = hj_rr_text,
     .hp = 0x20000,
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index cb7c978..b4d0b0a 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -307,42 +307,6 @@ typedef struct {
 
 /* The jo tree: each class's own keys only, the inherited ones are its parent's;
  * ga.by, an empty state list, is held by its module. */
-typedef struct {
-    pico8_fixed_t hp;
-    const pico8_jb_row_t *px;
-    const char *me;
-    const char *dq;
-    int hf;
-    int qh;
-    pico8_fixed_t od;
-    pico8_fixed_t fp;
-    pico8_fixed_t dw;
-    pico8_fixed_t mi;
-    pico8_fixed_t s;
-    const pico8_int_seq_t *ba;
-} pico8_qh_start_t;
-
-typedef struct {
-    const char *rr;
-    int qh;
-    pico8_fixed_t nl;
-    pico8_fixed_t hp;
-    pico8_fixed_t s;
-    const pico8_jb_row_t *px;
-    pico8_fixed_t od;
-    pico8_fixed_t qw;
-    const pico8_int_seq_t *ba;
-} pico8_nl_start_t;
-
-typedef struct {
-    pico8_fixed_t hp;
-    const pico8_jb_row_t *px;
-    int qh;
-    pico8_fixed_t s;
-    pico8_fixed_t od;
-    pico8_fixed_t hs;
-} pico8_os_start_t;
-
 typedef struct {
     const char *rr;
     pico8_fixed_t hp;
@@ -506,9 +470,6 @@ extern const pico8_jb_row_t pico8_pj_ou_row;
 /* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
  * nf.r are empty tables, held by their module. */
 extern const pico8_ke_start_t pico8_ke_start;
-extern const pico8_qh_start_t pico8_qh_start;
-extern const pico8_nl_start_t pico8_nl_start;
-extern const pico8_os_start_t pico8_os_start;
 extern const pico8_hj_start_t pico8_hj_start;
 extern const pico8_pm_start_t pico8_pm_start;
 extern const pico8_ga_start_t pico8_ga_start;
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 9c3ea40..4286cea 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -137,7 +137,6 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     pico8_ke_struct_init(&self->header, cls);
     self->def = def;
     self->hf = self->def->hf;
-    self->hs = self->def->hs;
     self->fa = self->def->fa;
     self->kq = &self->def->kq;
     self->ma = self->def->ma;
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index c2be800..46456d7 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -26,7 +26,6 @@ typedef struct {
     pico8_struct_header_t header;
     const pico8_jo_def_t *def;
     int hf;
-    pico8_fixed_t hs;
     pico8_fixed_t fa;
     const pico8_qz_box_t *kq;
     pico8_fixed_t ma;
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index a5e3ce4..8fef042 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -61,25 +61,20 @@ static const pico8_struct_class_t g_qh_struct_class = {
 
 void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     pico8_jo_struct_init(&self->jo, cls, def);
-    self->hp = pico8_qh_start.hp;
-    self->px = pico8_qh_start.px;
+    self->hp = self->jo.def->hp;
+    self->px = self->jo.def->px;
     self->probe = &self->probe_own;
-    self->probe->def = NULL;
-    self->probe->dq = pico8_qh_start.dq;
-    self->qh = pico8_qh_start.qh;
-    self->probe->od = pico8_qh_start.od;
-    self->probe->has_od = 1;
-    self->probe->fp = pico8_qh_start.fp;
-    self->probe->ik = 0;
-    self->probe->has_ik = 0;
-    self->probe->dw = pico8_qh_start.dw;
-    self->s = pico8_qh_start.s;
-    self->ba = pico8_qh_start.ba;
+    self->probe->def = &self->jo.def->probe;
+    self->probe->dq = self->jo.def->probe.dq;
+    self->qh = self->jo.def->qh;
+    self->probe->od = self->jo.def->probe.od;
+    self->probe->has_od = self->jo.def->probe.has_od;
+    self->probe->fp = self->jo.def->probe.fp;
+    self->probe->ik = self->jo.def->probe.ik;
+    self->probe->has_ik = self->jo.def->probe.has_ik;
+    self->probe->dw = self->jo.def->probe.dw;
+    self->s = self->jo.def->s;
     self->probe->et = NULL;
-    self->jo.mi = pico8_qh_start.mi;
-    self->jo.has_mi = 1;
-    self->jo.hf = pico8_qh_start.hf;
-    self->jo.header.me = pico8_qh_start.me;
 }
 
 static pico8_qh_t *pico8_qh_struct_new(const pico8_jo_keys_t *keys) {
@@ -154,15 +149,7 @@ static pico8_nl_t *pico8_nl_struct_new(const pico8_jo_keys_t *keys) {
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
     pico8_qh_struct_init(&self->qh, &g_nl_struct_class, &pico8_nl_def);
-    self->rr = pico8_nl_start.rr;
-    self->nl = pico8_nl_start.nl;
-    self->qw = pico8_nl_start.qw;
-    self->qh.qh = pico8_nl_start.qh;
-    self->qh.hp = pico8_nl_start.hp;
-    self->qh.s = pico8_nl_start.s;
-    self->qh.px = pico8_nl_start.px;
-    self->qh.probe->od = pico8_nl_start.od;
-    self->qh.ba = pico8_nl_start.ba;
+    self->qw = self->qh.jo.def->qw;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "nl: the only construction key is id (ie:ldown); another key has no member "
@@ -213,12 +200,6 @@ static pico8_os_t *pico8_os_struct_new(const pico8_jo_keys_t *keys) {
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
     pico8_qh_struct_init(&self->qh, &g_os_struct_class, &pico8_os_def);
-    self->qh.hp = pico8_os_start.hp;
-    self->qh.px = pico8_os_start.px;
-    self->qh.qh = pico8_os_start.qh;
-    self->qh.s = pico8_os_start.s;
-    self->qh.probe->od = pico8_os_start.od;
-    self->qh.jo.hs = pico8_os_start.hs;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "os: the only construction key is id (ie:ldown); another key has no member "
diff --git a/src/pico8_planet_family.h b/src/pico8_planet_family.h
index f107e18..e0b3538 100644
--- a/src/pico8_planet_family.h
+++ b/src/pico8_planet_family.h
@@ -22,19 +22,16 @@ void pico8_planet_family_init(void);
  * before the first `pico8_planet_family_init()`. */
 void pico8_planet_family_shutdown(void);
 
-/* A laboratory: a bare planet, then qh's own defaults, `dq`, `od`, `fp`
- * and `dw` in the probe with `et`, which its own `hq` (`bv`) writes. A
- * default of qh whose name is a member of `jo` or `ke` (`me`, `hf`,
- * `mi`) lives in that member, never in a second copy here. `px` and
- * `ba` point at qh's generated data (pico8_game_data.h). `probe` points at
- * `probe_own`: never copy the struct. Nothing frees the struct: TROU-7. */
+/* A laboratory: a bare planet, then qh's members copied from its kind's
+ * archetype, the probe's with `et`, which its own `hq` (`bv`) writes.
+ * `probe` points at `probe_own`: never copy the struct. Nothing frees the
+ * struct: TROU-7. */
 typedef struct {
     pico8_jo_t jo;
     pico8_fixed_t hp;
     const pico8_jb_row_t *px;
     int qh;
     pico8_fixed_t s;
-    const pico8_int_seq_t *ba;
     pico8_probe_t *probe;
     pico8_probe_t probe_own;
 } pico8_qh_t;
@@ -42,22 +39,15 @@ typedef struct {
 /* The `probe_of` of qh's descriptor, for its daughters' to name. */
 pico8_probe_t *pico8_qh_probe_of(void *self);
 
-/* A trading hub: a laboratory, then nl's own defaults `rr`, `nl` and `qw`
- * (`qw` also written by its own `ly`). A default of nl whose name is a member of `qh`
- * or `jo` (`qh`, `hp`, `s`, `px`, `od`, `ba`) lives in that member.
- * `px`, `ba` and `rr` point at nl's generated data. Nothing frees
- * the struct: TROU-7. */
+/* A trading hub: a laboratory, then `qw`, copied from its archetype and
+ * written by its own `ly`. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_qh_t qh;
-    const char *rr;
-    pico8_fixed_t nl;
     pico8_fixed_t qw;
 } pico8_nl_t;
 
-/* A processor: a laboratory and nothing of its own. Each default of os
- * lives in the member of that name, qh's (`hp`, `px`, `qh`, `s`, `od`) or
- * jo's (`hs`). `px` points at os's generated data, `ba` at
- * qh's. Nothing frees the struct: TROU-7. */
+/* A processor: a laboratory and nothing of its own; its defaults are its
+ * archetype's. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_qh_t qh;
 } pico8_os_t;
@@ -66,7 +56,7 @@ typedef struct {
  * in the emitter with `qy`, which its own `pq` (`f_`) writes. A default of
  * hj whose name is a member of `qh` or `jo` (`hp`, `qh`, `px`, `od`,
  * `s`) lives in that member. `px`, the emitter's `k_` and `rr` point
- * at hj's generated data, `ba` at qh's. `emitter` points at `emitter_own`:
+ * at hj's generated data. `emitter` points at `emitter_own`:
  * never copy the struct. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_qh_t qh;
@@ -82,7 +72,7 @@ pico8_emitter_t *pico8_hj_emitter_of(void *self);
  * ancestor has a member for. A default of pm whose name is a member of
  * `hj`, `qh`, `jo` or `ke` (`me`, `hp`, `px`, `s` and the 11 emitter
  * defaults) lives in that member. `px`, `k_`, `ng` and `fz` point
- * at pm's generated data, `ba` at qh's; `qy` is the protostar's own.
+ * at pm's generated data; `qy` is the protostar's own.
  * Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_hj_t hj;
diff --git a/src/pico8_planet_family_trade.h b/src/pico8_planet_family_trade.h
index 99bc6de..bddc7cd 100644
--- a/src/pico8_planet_family_trade.h
+++ b/src/pico8_planet_family_trade.h
@@ -17,7 +17,7 @@ void pico8_planet_family_trade_shutdown(void);
 /* A slipgate: a laboratory, then nr's own defaults `hz` and `rr`, which no
  * ancestor has a member for. A default of nr whose name is a member of `qh`
  * or `jo` (`kq`, `od`, `fp`, `s`, `dw`) lives in that member.
- * `hz`, `kq` and `rr` point at nr's generated data, `px` and `ba` at qh's.
+ * `hz`, `kq` and `rr` point at nr's generated data, `px` at qh's.
  * Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_qh_t qh;
@@ -28,7 +28,7 @@ typedef struct {
 /* An infraspace gate: a slipgate, then fr's own default `mt`, which no
  * ancestor has a member for. A default of fr whose name is a member of `nr`
  * or `qh` (`rr`, `hz`, `od`, `s`) lives in that member. `hz` and `rr` point
- * at fr's generated data, `kq` at nr's, `px` and `ba` at qh's.
+ * at fr's generated data, `kq` at nr's, `px` at qh's.
  * Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_nr_t nr;
@@ -39,7 +39,7 @@ typedef struct {
  * ancestor has a member for. A default of mx whose name is a member of
  * `hj`, `qh` or `jo` (`rr`, `hp`, `s`, `px`, `od` and the 11 emitter
  * defaults) lives in that member. `px`, `ht` and `k_` point at mx's
- * generated data, `ba` at qh's; `mx:kf` points `ht` at an empty sequence
+ * generated data; `mx:kf` points `ht` at an empty sequence
  * once it accepts a resource. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_hj_t hj;
diff --git a/src/pico8_planet_trade.c b/src/pico8_planet_trade.c
index 9538921..54d3f7b 100644
--- a/src/pico8_planet_trade.c
+++ b/src/pico8_planet_trade.c
@@ -152,9 +152,9 @@ void pico8_jo_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw) {
             const pico8_qh_t *qh = pico8_qh_of(&self->header);
             int accepted = 0;
 
-            if (qh != NULL && qh->ba != NULL) {
+            if (qh != NULL && qh->jo.def->ba != NULL) {
                 accepted = t_qe.kind == PICO8_QE_FIXED &&
-                           pico8_int_seq_has_fixed(qh->ba, t_qe.n);
+                           pico8_int_seq_has_fixed(qh->jo.def->ba, t_qe.n);
             }
             if (accepted) {
                 assert(t_qe.kind == PICO8_QE_FIXED && "a want's qe is a resource of r, never a boolean");
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index 81b879a..d6be829 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -22,8 +22,8 @@
  * nil. `fp` is what `d_` hands to `fm:nx`: the cart's false is held as 0,
  * which is what `self.fp or 0` hands. */
 typedef struct pico8_probe {
-    /* NULL on the jo tree, `pico8_probe_def_none` on research_icon's `pb`:
-     * no generic reader may use it. */
+    /* `pico8_probe_def_none` on research_icon's `pb`; no generic reader may
+     * use it. */
     const pico8_probe_def_t *def;
     pico8_struct_header_t *et;
     pico8_fixed_t dw;
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index e92ffb1..1a17839 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -56,10 +56,6 @@ static int same_text(const char *got, const char *want) {
     return got != NULL && want != NULL && strcmp(got, want) == 0;
 }
 
-static int same_text_or_nil(const char *got, const char *want) {
-    return (got == NULL && want == NULL) || same_text(got, want);
-}
-
 static int same_vec(const pico8_vec_t *got, const pico8_vec_t *want) {
     return got != NULL && got->x == want->x && got->y == want->y;
 }
@@ -182,32 +178,6 @@ static void test_fv(void) {
 }
 
 /* `kw`, `kw_own` and `by` are not the archetype's: left to each kind's test. */
-/* jo's part that qh's init still writes on its tree. */
-static void same_jo_part(const pico8_jo_t *jo, const pico8_jo_def_t *def, const char *kind) {
-    same_in(same_text(jo->header.me, def->me), kind, "me");
-    same_in(jo->header.t == def->t, kind, "t");
-    same_in(jo->hf == def->hf, kind, "hf");
-    same_in(jo->mi == def->mi, kind, "mi");
-    same_in(jo->has_mi == def->has_mi, kind, "has_mi");
-}
-
-static void same_qh_part(const pico8_qh_t *qh, const pico8_jo_def_t *def, const char *kind) {
-    same_in(qh->hp == def->hp, kind, "hp");
-    same_in(qh->px == def->px, kind, "px");
-    same_in(qh->qh == def->qh, kind, "qh");
-    same_in(qh->s == def->s, kind, "s");
-    same_in(qh->ba == def->ba, kind, "ba");
-    same_in(qh->probe->dw == def->probe.dw, kind, "probe.dw");
-    same_in(qh->probe->od == def->probe.od, kind, "probe.od");
-    same_in(qh->probe->has_od == def->probe.has_od, kind, "probe.has_od");
-    same_in(same_text_or_nil(qh->probe->dq, def->probe.dq), kind, "probe.dq");
-    same_in(qh->probe->fp == def->probe.fp, kind, "probe.fp");
-    same_in(qh->probe->ik == def->probe.ik, kind, "probe.ik");
-    same_in(qh->probe->has_ik == def->probe.has_ik, kind, "probe.has_ik");
-    assert(qh->probe->et == NULL);
-    assert(qh->jo.by == NULL);
-}
-
 /* A kind of the qh tree has an `hp`: its `_new` runs `lz`, which fills
  * `kw_own` from that research and points `kw` at it. */
 static void assert_lz_ran(const pico8_jo_t *jo) {
@@ -226,31 +196,49 @@ static void test_jo(void) {
 static void test_qh(void) {
     const pico8_qh_t *qh = pico8_qh_new(NULL);
 
-    same_jo_part(&qh->jo, &pico8_qh_def, "qh");
-    same_qh_part(qh, &pico8_qh_def, "qh");
+    same_in(qh->jo.header.t == pico8_qh_def.t, "qh", "t");
+    assert(qh->probe->et == NULL && qh->jo.by == NULL);
     assert_lz_ran(&qh->jo);
 }
 
 static void test_nl(void) {
     const pico8_nl_t *nl = pico8_nl_new(NULL);
 
-    same_jo_part(&nl->qh.jo, &pico8_nl_def, "nl");
-    same_qh_part(&nl->qh, &pico8_nl_def, "nl");
-    same_in(same_text(nl->rr, pico8_nl_def.rr), "nl", "rr");
-    same_in(nl->nl == pico8_nl_def.nl, "nl", "nl");
-    same_in(nl->qw == pico8_nl_def.qw, "nl", "qw");
+    same_in(nl->qh.jo.header.t == pico8_nl_def.t, "nl", "t");
+    assert(nl->qh.probe->et == NULL && nl->qh.jo.by == NULL);
     assert_lz_ran(&nl->qh.jo);
 }
 
 static void test_os(void) {
     const pico8_os_t *os = pico8_os_new(NULL);
 
-    same_jo_part(&os->qh.jo, &pico8_os_def, "os");
-    same_in(os->qh.jo.hs == pico8_os_def.hs, "os", "hs");
-    same_qh_part(&os->qh, &pico8_os_def, "os");
+    same_in(os->qh.jo.header.t == pico8_os_def.t, "os", "t");
+    assert(os->qh.probe->et == NULL && os->qh.jo.by == NULL);
     assert_lz_ran(&os->qh.jo);
 }
 
+/* What hj's init still writes on qh's part, for hj, pm and mx. */
+static void same_hj_part(const pico8_qh_t *qh, const pico8_jo_def_t *def, const char *kind) {
+    same_in(qh->jo.header.t == def->t, kind, "t");
+    same_in(qh->hp == def->hp, kind, "hp");
+    same_in(qh->px == def->px, kind, "px");
+    same_in(qh->qh == def->qh, kind, "qh");
+    same_in(qh->s == def->s, kind, "s");
+    same_in(qh->probe->od == def->probe.od, kind, "probe.od");
+    assert(qh->probe->et == NULL && qh->jo.by == NULL);
+}
+
+/* What nr's init still writes, for nr and fr. */
+static void same_nr_part(const pico8_qh_t *qh, const pico8_jo_def_t *def, const char *kind) {
+    same_in(qh->jo.header.t == def->t, kind, "t");
+    same_in(same_box(qh->jo.kq, &def->kq), kind, "kq");
+    same_in(qh->s == def->s, kind, "s");
+    same_in(qh->probe->dw == def->probe.dw, kind, "probe.dw");
+    same_in(qh->probe->od == def->probe.od, kind, "probe.od");
+    same_in(qh->probe->fp == def->probe.fp, kind, "probe.fp");
+    assert(qh->probe->et == NULL && qh->jo.by == NULL);
+}
+
 /* `qy` is left to each kind's test. */
 static void same_emitter_part(const pico8_emitter_t *emitter, const pico8_jo_def_t *def, const char *kind) {
     same_in(emitter->pn == def->emitter.pn, kind, "emitter.pn");
@@ -270,8 +258,7 @@ static void same_emitter_part(const pico8_emitter_t *emitter, const pico8_jo_def
 static void test_hj(void) {
     const pico8_hj_t *hj = pico8_hj_new(NULL);
 
-    same_jo_part(&hj->qh.jo, &pico8_hj_def, "hj");
-    same_qh_part(&hj->qh, &pico8_hj_def, "hj");
+    same_hj_part(&hj->qh, &pico8_hj_def, "hj");
     same_in(same_text(hj->rr, pico8_hj_def.rr), "hj", "rr");
     same_emitter_part(hj->emitter, &pico8_hj_def, "hj");
     assert(hj->emitter->qy != NULL && hj->emitter->qy->count == 0);
@@ -281,8 +268,8 @@ static void test_hj(void) {
 static void test_pm(void) {
     const pico8_pm_t *pm = pico8_pm_new(NULL);
 
-    same_jo_part(&pm->hj.qh.jo, &pico8_pm_def, "pm");
-    same_qh_part(&pm->hj.qh, &pico8_pm_def, "pm");
+    same_in(same_text(pm->hj.qh.jo.header.me, pico8_pm_def.me), "pm", "me");
+    same_hj_part(&pm->hj.qh, &pico8_pm_def, "pm");
     same_in(same_text(pm->hj.rr, pico8_pm_def.rr), "pm", "rr");
     same_in(same_vec(pm->ng, &pico8_pm_def.ng), "pm", "ng");
     same_in(same_text(pm->fz, pico8_pm_def.fz), "pm", "fz");
@@ -294,8 +281,7 @@ static void test_pm(void) {
 static void test_mx(void) {
     const pico8_mx_t *mx = pico8_mx_new(NULL);
 
-    same_jo_part(&mx->hj.qh.jo, &pico8_mx_def, "mx");
-    same_qh_part(&mx->hj.qh, &pico8_mx_def, "mx");
+    same_hj_part(&mx->hj.qh, &pico8_mx_def, "mx");
     same_in(same_text(mx->hj.rr, pico8_mx_def.rr), "mx", "rr");
     same_in(mx->ht == pico8_mx_def.ht, "mx", "ht");
     same_emitter_part(mx->hj.emitter, &pico8_mx_def, "mx");
@@ -326,9 +312,7 @@ static void test_ga(void) {
 static void test_nr(void) {
     const pico8_nr_t *nr = pico8_nr_new(NULL);
 
-    same_jo_part(&nr->qh.jo, &pico8_nr_def, "nr");
-    same_in(same_box(nr->qh.jo.kq, &pico8_nr_def.kq), "nr", "kq");
-    same_qh_part(&nr->qh, &pico8_nr_def, "nr");
+    same_nr_part(&nr->qh, &pico8_nr_def, "nr");
     same_in(nr->hz == pico8_nr_def.hz, "nr", "hz");
     same_in(same_text(nr->rr, pico8_nr_def.rr), "nr", "rr");
     assert_lz_ran(&nr->qh.jo);
@@ -337,9 +321,7 @@ static void test_nr(void) {
 static void test_fr(void) {
     const pico8_fr_t *fr = pico8_fr_new(NULL);
 
-    same_jo_part(&fr->nr.qh.jo, &pico8_fr_def, "fr");
-    same_in(same_box(fr->nr.qh.jo.kq, &pico8_fr_def.kq), "fr", "kq");
-    same_qh_part(&fr->nr.qh, &pico8_fr_def, "fr");
+    same_nr_part(&fr->nr.qh, &pico8_fr_def, "fr");
     same_in(fr->nr.hz == pico8_fr_def.hz, "fr", "hz");
     same_in(same_text(fr->nr.rr, pico8_fr_def.rr), "fr", "rr");
     same_in(fr->mt == pico8_fr_def.mt, "fr", "mt");
@@ -382,7 +364,7 @@ int main(void) {
     test_ga();
     test_nr();
     test_fr();
-    assert(g_compared == 231 && "families a to e compare 231 fields: one missed or one too many");
+    assert(g_compared == 123 && "families a to e compare 123 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_entity_kind.c b/tests/test_pico8_entity_kind.c
index e323d4b..fdd8585 100644
--- a/tests/test_pico8_entity_kind.c
+++ b/tests/test_pico8_entity_kind.c
@@ -160,44 +160,44 @@ static void test_kind_reader_serves_the_generated_defaults(void) {
 
     rec = rec_of(0);
     assert(rec->rr == NULL);
-    assert(rec->has_s && rec->s == pico8_qh_start.s);
-    assert(rec->me == pico8_qh_start.me);
-    assert(rec->has_od && rec->od == pico8_qh_start.od);
+    assert(rec->has_s && rec->s == pico8_qh_def.s);
+    assert(rec->me == pico8_qh_def.me);
+    assert(rec->has_od && rec->od == pico8_qh_def.probe.od);
 
     rec = rec_of(1);
     assert(rec->rr == NULL);
-    assert(rec->has_s && rec->s == pico8_os_start.s);
-    assert(rec->me == pico8_qh_start.me);
-    assert(rec->has_od && rec->od == pico8_os_start.od);
+    assert(rec->has_s && rec->s == pico8_os_def.s);
+    assert(rec->me == pico8_qh_def.me);
+    assert(rec->has_od && rec->od == pico8_os_def.probe.od);
 
     rec = rec_of(2);
     assert(rec->rr == pico8_nr_start.rr);
     assert(rec->has_s && rec->s == pico8_nr_start.s);
-    assert(rec->me == pico8_qh_start.me);
+    assert(rec->me == pico8_qh_def.me);
     assert(rec->has_od && rec->od == pico8_nr_start.od);
 
     rec = rec_of(3);
     assert(rec->rr == pico8_fr_start.rr);
     assert(rec->has_s && rec->s == pico8_fr_start.s);
-    assert(rec->me == pico8_qh_start.me);
+    assert(rec->me == pico8_qh_def.me);
     assert(rec->has_od && rec->od == pico8_fr_start.od);
 
     rec = rec_of(4);
-    assert(rec->rr == pico8_nl_start.rr);
-    assert(rec->has_s && rec->s == pico8_nl_start.s);
-    assert(rec->me == pico8_qh_start.me);
-    assert(rec->has_od && rec->od == pico8_nl_start.od);
+    assert(rec->rr == pico8_nl_def.rr);
+    assert(rec->has_s && rec->s == pico8_nl_def.s);
+    assert(rec->me == pico8_qh_def.me);
+    assert(rec->has_od && rec->od == pico8_nl_def.probe.od);
 
     rec = rec_of(5);
     assert(rec->rr == pico8_mx_start.rr);
     assert(rec->has_s && rec->s == pico8_mx_start.s);
-    assert(rec->me == pico8_qh_start.me);
+    assert(rec->me == pico8_qh_def.me);
     assert(rec->has_od && rec->od == pico8_mx_start.od);
 
     rec = rec_of(6);
     assert(rec->rr == pico8_hj_start.rr);
     assert(rec->has_s && rec->s == pico8_hj_start.s);
-    assert(rec->me == pico8_qh_start.me);
+    assert(rec->me == pico8_qh_def.me);
     assert(rec->has_od && rec->od == pico8_hj_start.od);
 
     rec = rec_of(7);
diff --git a/tests/test_pico8_fr.c b/tests/test_pico8_fr.c
index 560acf9..d699b8d 100644
--- a/tests/test_pico8_fr.c
+++ b/tests/test_pico8_fr.c
@@ -266,7 +266,7 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fold_qh(h, qh);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
-    h = fold_fixed_member(h, jo->hs);
+    h = fold_fixed_member(h, jo->def->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
@@ -286,7 +286,7 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->ba != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
     h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -338,7 +338,7 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     assert(nr->hz != &pico8_nr_hz_seq);
     assert(jo->kq == &pico8_nr_start.kq);
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
-    assert(qh->px == &pico8_qh_px_row && qh->ba == &pico8_qh_ba_seq);
+    assert(qh->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->def == &pico8_fr_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
diff --git a/tests/test_pico8_hj.c b/tests/test_pico8_hj.c
index 670acd9..2ce77ac 100644
--- a/tests/test_pico8_hj.c
+++ b/tests/test_pico8_hj.c
@@ -330,7 +330,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->probe->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
-    h = fold_fixed_member(h, jo->hs);
+    h = fold_fixed_member(h, jo->def->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
@@ -358,7 +358,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->ba != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
     h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -398,7 +398,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->px == &pico8_hj_px_row && em->k_ == &pico8_hj_k__seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->px != &pico8_qh_px_row);
-    assert(qh->ba == &pico8_qh_ba_seq);
+    assert(qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_hj_def.kq);
     assert(jo->def == &pico8_hj_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
diff --git a/tests/test_pico8_jo.c b/tests/test_pico8_jo.c
index 9b3539b..1af2d65 100644
--- a/tests/test_pico8_jo.c
+++ b/tests/test_pico8_jo.c
@@ -206,7 +206,7 @@ static uint32_t fold_planet(uint32_t h, const void *e) {
     h = fold_fixed_member(h, jo->header.t);
     h = fold_string_member(h, jo->id);
     h = fold_hf(h, jo);
-    h = fold_fixed_member(h, jo->hs);
+    h = fold_fixed_member(h, jo->def->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
diff --git a/tests/test_pico8_jo_spec_defaults.c b/tests/test_pico8_jo_spec_defaults.c
index 89a760f..1a08288 100644
--- a/tests/test_pico8_jo_spec_defaults.c
+++ b/tests/test_pico8_jo_spec_defaults.c
@@ -1,7 +1,6 @@
 /* The defaults of the ten classes of the `jo` tree, on a fresh struct of
- * each: its own keys are its class's generated data (pico8_game_data.h),
- * the others its parent's; and each module's `_ready` over init and
- * shutdown. */
+ * each, against its kind's archetype; and each module's `_ready` over init
+ * and shutdown. */
 
 #include <assert.h>
 #include <stdio.h>
@@ -55,11 +54,14 @@ static void assert_jo_box(const pico8_qz_box_t *kq) {
            kq->xr == pico8_fixed_from_int(7) && kq->yb == pico8_fixed_from_int(7));
 }
 
-/* The members of qh every class below it inherits. */
-static void assert_qh_tail(const pico8_qh_t *qh) {
-    assert_same_string(qh->probe->dq, pico8_qh_start.dq);
-    assert(qh->jo.hf == pico8_qh_start.hf);
-    assert(qh->jo.mi == pico8_qh_start.mi && qh->jo.has_mi == 1);
+/* The members of qh every class below it inherits, its probe's archetype, and
+ * the probe's init literals. */
+static void assert_qh_tail(const pico8_qh_t *qh, const pico8_jo_def_t *def) {
+    assert(qh->probe->def == &def->probe);
+    assert_same_string(qh->probe->dq, def->probe.dq);
+    assert(qh->jo.hf == def->hf);
+    assert(qh->jo.mi == def->mi && qh->jo.has_mi == 1);
+    assert(qh->probe->has_od == 1 && qh->probe->ik == 0 && qh->probe->has_ik == 0);
 }
 
 static void test_jo_defaults(void) {
@@ -68,7 +70,6 @@ static void test_jo_defaults(void) {
     assert(pico8_jo_ready());
     assert_same_string(jo->header.me, jo->def->me);
     assert(jo->hf == jo->def->hf);
-    assert(jo->hs == jo->def->hs);
     assert(jo->fa == jo->def->fa);
     assert_same_box(jo->kq, &jo->def->kq);
     assert_jo_box(jo->kq);
@@ -84,17 +85,15 @@ static void test_qh_defaults(void) {
     pico8_qh_t *qh = pico8_qh_new(NULL);
 
     assert(pico8_qh_ready());
-    assert(qh->hp == pico8_qh_start.hp);
-    assert(qh->px == pico8_qh_start.px);
-    assert_same_string(qh->jo.header.me, pico8_qh_start.me);
-    assert(qh->qh == pico8_qh_start.qh);
-    assert(qh->probe->od == pico8_qh_start.od);
-    assert(qh->probe->fp == pico8_qh_start.fp);
-    assert(qh->probe->dw == pico8_qh_start.dw);
-    assert(qh->s == pico8_qh_start.s);
-    assert(qh->ba == pico8_qh_start.ba);
-    assert_qh_tail(qh);
-    assert(qh->jo.hs == qh->jo.def->hs);
+    assert(qh->hp == pico8_qh_def.hp);
+    assert(qh->px == pico8_qh_def.px);
+    assert_same_string(qh->jo.header.me, pico8_qh_def.me);
+    assert(qh->qh == pico8_qh_def.qh);
+    assert(qh->probe->od == pico8_qh_def.probe.od);
+    assert(qh->probe->fp == pico8_qh_def.probe.fp);
+    assert(qh->probe->dw == pico8_qh_def.probe.dw);
+    assert(qh->s == pico8_qh_def.s);
+    assert_qh_tail(qh, &pico8_qh_def);
     assert(qh->jo.fa == qh->jo.def->fa);
     assert_same_box(qh->jo.kq, &qh->jo.def->kq);
     assert_jo_box(qh->jo.kq);
@@ -105,20 +104,16 @@ static void test_nl_defaults(void) {
     pico8_nl_t *nl = pico8_nl_new(NULL);
 
     assert(pico8_nl_ready());
-    assert_same_string(nl->rr, pico8_nl_start.rr);
-    assert(nl->qh.qh == pico8_nl_start.qh);
-    assert(nl->nl == pico8_nl_start.nl);
-    assert(nl->qh.hp == pico8_nl_start.hp);
-    assert(nl->qh.s == pico8_nl_start.s);
-    assert(nl->qh.px == pico8_nl_start.px);
-    assert(nl->qh.probe->od == pico8_nl_start.od);
-    assert(nl->qw == pico8_nl_start.qw);
-    assert(nl->qh.ba == pico8_nl_start.ba);
-    assert_same_string(nl->qh.jo.header.me, pico8_qh_start.me);
-    assert(nl->qh.probe->fp == pico8_qh_start.fp);
-    assert(nl->qh.probe->dw == pico8_qh_start.dw);
-    assert_qh_tail(&nl->qh);
-    assert(nl->qh.jo.hs == nl->qh.jo.def->hs);
+    assert(nl->qh.qh == pico8_nl_def.qh);
+    assert(nl->qh.hp == pico8_nl_def.hp);
+    assert(nl->qh.s == pico8_nl_def.s);
+    assert(nl->qh.px == pico8_nl_def.px);
+    assert(nl->qh.probe->od == pico8_nl_def.probe.od);
+    assert(nl->qw == pico8_nl_def.qw);
+    assert_same_string(nl->qh.jo.header.me, pico8_nl_def.me);
+    assert(nl->qh.probe->fp == pico8_nl_def.probe.fp);
+    assert(nl->qh.probe->dw == pico8_nl_def.probe.dw);
+    assert_qh_tail(&nl->qh, &pico8_nl_def);
     assert(nl->qh.jo.fa == nl->qh.jo.def->fa);
     assert_same_box(nl->qh.jo.kq, &nl->qh.jo.def->kq);
     assert_jo_box(nl->qh.jo.kq);
@@ -129,17 +124,15 @@ static void test_os_defaults(void) {
     pico8_os_t *os = pico8_os_new(NULL);
 
     assert(pico8_os_ready());
-    assert(os->qh.hp == pico8_os_start.hp);
-    assert(os->qh.px == pico8_os_start.px);
-    assert(os->qh.qh == pico8_os_start.qh);
-    assert(os->qh.s == pico8_os_start.s);
-    assert(os->qh.probe->od == pico8_os_start.od);
-    assert(os->qh.jo.hs == pico8_os_start.hs);
-    assert_same_string(os->qh.jo.header.me, pico8_qh_start.me);
-    assert(os->qh.probe->fp == pico8_qh_start.fp);
-    assert(os->qh.probe->dw == pico8_qh_start.dw);
-    assert(os->qh.ba == pico8_qh_start.ba);
-    assert_qh_tail(&os->qh);
+    assert(os->qh.hp == pico8_os_def.hp);
+    assert(os->qh.px == pico8_os_def.px);
+    assert(os->qh.qh == pico8_os_def.qh);
+    assert(os->qh.s == pico8_os_def.s);
+    assert(os->qh.probe->od == pico8_os_def.probe.od);
+    assert_same_string(os->qh.jo.header.me, pico8_os_def.me);
+    assert(os->qh.probe->fp == pico8_os_def.probe.fp);
+    assert(os->qh.probe->dw == pico8_os_def.probe.dw);
+    assert_qh_tail(&os->qh, &pico8_os_def);
     assert(os->qh.jo.fa == os->qh.jo.def->fa);
     assert_same_box(os->qh.jo.kq, &os->qh.jo.def->kq);
     assert_jo_box(os->qh.jo.kq);
@@ -168,12 +161,10 @@ static void test_hj_defaults(void) {
     assert(hj->emitter->qg == pico8_hj_start.qg);
     assert(hj->emitter->q_ == pico8_hj_start.q_);
     assert(hj->qh.qh == pico8_hj_start.qh);
-    assert_same_string(hj->qh.jo.header.me, pico8_qh_start.me);
-    assert(hj->qh.probe->fp == pico8_qh_start.fp);
-    assert(hj->qh.probe->dw == pico8_qh_start.dw);
-    assert(hj->qh.ba == pico8_qh_start.ba);
-    assert_qh_tail(&hj->qh);
-    assert(hj->qh.jo.hs == hj->qh.jo.def->hs);
+    assert_same_string(hj->qh.jo.header.me, pico8_hj_def.me);
+    assert(hj->qh.probe->fp == pico8_hj_def.probe.fp);
+    assert(hj->qh.probe->dw == pico8_hj_def.probe.dw);
+    assert_qh_tail(&hj->qh, &pico8_hj_def);
     assert(hj->qh.jo.fa == hj->qh.jo.def->fa);
     assert_same_box(hj->qh.jo.kq, &hj->qh.jo.def->kq);
     assert_jo_box(hj->qh.jo.kq);
@@ -204,11 +195,9 @@ static void test_pm_defaults(void) {
     assert_same_string(pm->hj.rr, pico8_hj_start.rr);
     assert(pm->hj.qh.qh == pico8_hj_start.qh);
     assert(pm->hj.qh.probe->od == pico8_hj_start.od);
-    assert(pm->hj.qh.probe->fp == pico8_qh_start.fp);
-    assert(pm->hj.qh.probe->dw == pico8_qh_start.dw);
-    assert(pm->hj.qh.ba == pico8_qh_start.ba);
-    assert_qh_tail(&pm->hj.qh);
-    assert(pm->hj.qh.jo.hs == pm->hj.qh.jo.def->hs);
+    assert(pm->hj.qh.probe->fp == pico8_pm_def.probe.fp);
+    assert(pm->hj.qh.probe->dw == pico8_pm_def.probe.dw);
+    assert_qh_tail(&pm->hj.qh, &pico8_pm_def);
     assert(pm->hj.qh.jo.fa == pm->hj.qh.jo.def->fa);
     assert_same_box(pm->hj.qh.jo.kq, &pm->hj.qh.jo.def->kq);
     assert_jo_box(pm->hj.qh.jo.kq);
@@ -236,7 +225,6 @@ static void test_ga_defaults(void) {
     assert(ga->emitter->py == pico8_ga_start.py);
     assert(ga->ms == pico8_ga_start.ms);
     assert_same_string(ga->jo.header.me, ga->jo.def->me);
-    assert(ga->jo.hs == ga->jo.def->hs);
     assert_jo_tail(&ga->jo, &pico8_ga_def);
     assert(ga->jo.def->hs == pico8_fixed_from_int(1));
     assert(ga->jo.mi == 0 && !ga->jo.has_mi);
@@ -254,13 +242,11 @@ static void assert_nr_own(const pico8_nr_t *nr) {
 
 /* What nr and fr inherit from qh and jo. */
 static void assert_nr_inherited(const pico8_nr_t *nr, const pico8_jo_def_t *def) {
-    assert(nr->qh.hp == pico8_qh_start.hp);
-    assert(nr->qh.px == pico8_qh_start.px);
-    assert_same_string(nr->qh.jo.header.me, pico8_qh_start.me);
-    assert(nr->qh.qh == pico8_qh_start.qh);
-    assert(nr->qh.ba == pico8_qh_start.ba);
-    assert_qh_tail(&nr->qh);
-    assert(nr->qh.jo.hs == nr->qh.jo.def->hs);
+    assert(nr->qh.hp == def->hp);
+    assert(nr->qh.px == def->px);
+    assert_same_string(nr->qh.jo.header.me, def->me);
+    assert(nr->qh.qh == def->qh);
+    assert_qh_tail(&nr->qh, def);
     assert(nr->qh.jo.fa == nr->qh.jo.def->fa);
     assert_jo_tail(&nr->qh.jo, def);
 }
@@ -312,12 +298,10 @@ static void test_mx_defaults(void) {
     assert(mx->hj.emitter->qg == pico8_mx_start.qg);
     assert(mx->hj.emitter->q_ == pico8_mx_start.q_);
     assert(mx->hj.qh.qh == pico8_hj_start.qh);
-    assert_same_string(mx->hj.qh.jo.header.me, pico8_qh_start.me);
-    assert(mx->hj.qh.probe->fp == pico8_qh_start.fp);
-    assert(mx->hj.qh.probe->dw == pico8_qh_start.dw);
-    assert(mx->hj.qh.ba == pico8_qh_start.ba);
-    assert_qh_tail(&mx->hj.qh);
-    assert(mx->hj.qh.jo.hs == mx->hj.qh.jo.def->hs);
+    assert_same_string(mx->hj.qh.jo.header.me, pico8_mx_def.me);
+    assert(mx->hj.qh.probe->fp == pico8_mx_def.probe.fp);
+    assert(mx->hj.qh.probe->dw == pico8_mx_def.probe.dw);
+    assert_qh_tail(&mx->hj.qh, &pico8_mx_def);
     assert(mx->hj.qh.jo.fa == mx->hj.qh.jo.def->fa);
     assert_same_box(mx->hj.qh.jo.kq, &mx->hj.qh.jo.def->kq);
     assert_jo_box(mx->hj.qh.jo.kq);
diff --git a/tests/test_pico8_map_tables.c b/tests/test_pico8_map_tables.c
index 37e262c..a626500 100644
--- a/tests/test_pico8_map_tables.c
+++ b/tests/test_pico8_map_tables.c
@@ -150,7 +150,7 @@ static void test_gn_pools_df_entries_by_their_r(void) {
 /* ob() gave each string token its own allocation: no string field of the jo
  * tree shares a pointer with another string of the generated data. */
 static void test_jo_tree_strings_are_distinct_objects(void) {
-    const char *const tree[] = {pico8_jo_def.me, pico8_qh_start.me, pico8_qh_start.dq, pico8_nl_start.rr,
+    const char *const tree[] = {pico8_jo_def.me, pico8_qh_def.me, pico8_qh_def.probe.dq, pico8_nl_def.rr,
         pico8_hj_start.rr, pico8_pm_start.fz, pico8_pm_start.me, pico8_ga_start.fz, pico8_nr_start.rr,
         pico8_fr_start.rr, pico8_mx_start.rr};
     const char *const others[] = {pico8_ke_start.me, pico8_mz_def.me, pico8_mz_def.probe.dq, pico8_jc_def.me,
diff --git a/tests/test_pico8_mx.c b/tests/test_pico8_mx.c
index 7a8c517..b76b027 100644
--- a/tests/test_pico8_mx.c
+++ b/tests/test_pico8_mx.c
@@ -321,7 +321,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->probe->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
-    h = fold_fixed_member(h, jo->hs);
+    h = fold_fixed_member(h, jo->def->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
@@ -352,7 +352,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->ba != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
     h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -395,7 +395,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
            em->k_ == &pico8_mx_k__seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(em->k_ != &pico8_hj_k__seq);
-    assert(qh->ba == &pico8_qh_ba_seq);
+    assert(qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_mx_def.kq);
     assert(jo->def == &pico8_mx_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
diff --git a/tests/test_pico8_nl.c b/tests/test_pico8_nl.c
index c1526bc..3689c63 100644
--- a/tests/test_pico8_nl.c
+++ b/tests/test_pico8_nl.c
@@ -247,7 +247,7 @@ static void *build_plate(void) {
 /* A plate is the only entity whose chain holds `nl`, and its `nl` is 1. */
 static int is_plate(const void *e) {
     const pico8_nl_t *nl = pico8_nl_of(e);
-    return nl != NULL && nl->nl == pico8_fixed_from_int(1);
+    return nl != NULL && nl->qh.jo.def->nl == pico8_fixed_from_int(1);
 }
 
 /* A plate as `ie:ldown` and `jo:pq` left it: its scalars, which of nl's,
@@ -266,8 +266,8 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fold_string_member(h, jo->id);
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
-    h = fold_string_member(h, nl->rr);
-    h = fold_fixed_member(h, nl->nl);
+    h = fold_string_member(h, nl->qh.jo.def->rr);
+    h = fold_fixed_member(h, nl->qh.jo.def->nl);
     h = fold_qh(h, qh);
     h = fold_fixed_member(h, qh->hp);
     h = fold_fixed_member(h, qh->s);
@@ -278,7 +278,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fold_fixed_member(h, qh->probe->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
-    h = fold_fixed_member(h, jo->hs);
+    h = fold_fixed_member(h, jo->def->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
@@ -286,7 +286,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     /* The 1s of `nl` and `qw`, of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of
      * `hf`, `hs`, `fa`, `ma` and `ca`: constant by the struct's shape, kept
      * so that BUILT_HASH still proves the typed reads. */
-    h = fnv(h, (uint32_t)(nl->rr != NULL));
+    h = fnv(h, (uint32_t)(nl->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -297,7 +297,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->ba != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
     h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -338,7 +338,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     assert(cls_of(e)->states[PICO8_STATE_LY] != pico8_qh_struct_class()->states[PICO8_STATE_LY]);
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->px == &pico8_nl_px_row && qh->px != &pico8_qh_px_row);
-    assert(qh->ba == &pico8_nl_ba_seq && qh->ba != &pico8_qh_ba_seq);
+    assert(qh->jo.def->ba == &pico8_nl_ba_seq && qh->jo.def->ba != &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_nl_def.kq);
     assert(jo->def == &pico8_nl_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
diff --git a/tests/test_pico8_nr.c b/tests/test_pico8_nr.c
index 3039ab1..88f7367 100644
--- a/tests/test_pico8_nr.c
+++ b/tests/test_pico8_nr.c
@@ -282,7 +282,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fold_qh(h, qh);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
-    h = fold_fixed_member(h, jo->hs);
+    h = fold_fixed_member(h, jo->def->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
@@ -301,7 +301,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->ba != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
     h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -350,7 +350,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && nr->hz == &pico8_nr_hz_seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(jo->kq != &pico8_jo_def.kq);
-    assert(qh->px == &pico8_qh_px_row && qh->ba == &pico8_qh_ba_seq);
+    assert(qh->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->def == &pico8_nr_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
diff --git a/tests/test_pico8_os.c b/tests/test_pico8_os.c
index 5b1c0d4..2f9d918 100644
--- a/tests/test_pico8_os.c
+++ b/tests/test_pico8_os.c
@@ -303,7 +303,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
-    h = fold_fixed_member(h, jo->hs);
+    h = fold_fixed_member(h, jo->def->hs);
     h = fold_string_member(h, qh->probe->dq);
     h = fold_fixed_member(h, qh->probe->fp);
     h = fold_fixed_member(h, qh->probe->dw);
@@ -324,7 +324,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->ba != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
     h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -363,7 +363,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
            cls_of(e)->mm == NULL && pico8_jo_is_family(e));
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->px == &pico8_os_px_row && qh->px != &pico8_qh_px_row);
-    assert(qh->ba == &pico8_qh_ba_seq);
+    assert(qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_os_def.kq);
     assert(jo->def == &pico8_os_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
diff --git a/tests/test_pico8_pm.c b/tests/test_pico8_pm.c
index 8b43bb0..f3ee78e 100644
--- a/tests/test_pico8_pm.c
+++ b/tests/test_pico8_pm.c
@@ -318,7 +318,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->probe->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
-    h = fold_fixed_member(h, jo->hs);
+    h = fold_fixed_member(h, jo->def->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
@@ -352,7 +352,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->ba != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
     h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -396,7 +396,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->px == &pico8_pm_px_row && em->k_ == &pico8_pm_k__seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(em->k_ != &pico8_hj_k__seq);
-    assert(qh->ba == &pico8_qh_ba_seq);
+    assert(qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_pm_def.kq);
     assert(jo->def == &pico8_pm_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
diff --git a/tests/test_pico8_qh.c b/tests/test_pico8_qh.c
index fd9f907..8c4de33 100644
--- a/tests/test_pico8_qh.c
+++ b/tests/test_pico8_qh.c
@@ -275,7 +275,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_fixed_member(h, qh->s);
     h = fold_hf(h, jo);
-    h = fold_fixed_member(h, jo->hs);
+    h = fold_fixed_member(h, jo->def->hs);
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
@@ -291,7 +291,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->ba != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
     h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -328,7 +328,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 1);
     assert(cls_of(e)->kf == pico8_qh_struct_class()->kf && cls_of(e)->cn == pico8_qh_struct_class()->cn &&
            cls_of(e)->mm == NULL && pico8_jo_is_family(e));
-    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->px == &pico8_qh_px_row && qh->ba == &pico8_qh_ba_seq);
+    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_qh_def.kq);
     assert(jo->def == &pico8_qh_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
@@ -514,7 +514,7 @@ static uint32_t fold_descendant(uint32_t h, void *d) {
     h = qh->jo.has_mi ? fold_fixed_member(h, qh->jo.mi) : fold_absent(h);
     h = fnv(h, (uint32_t)(qh->jo.header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER))));
     h = fnv(h, (uint32_t)(qh->px == &pico8_qh_px_row));
-    h = fnv(h, (uint32_t)(qh->ba == &pico8_qh_ba_seq));
+    h = fnv(h, (uint32_t)(qh->jo.def->ba == &pico8_qh_ba_seq));
     assert(cls_of(d)->hq == pico8_qh_struct_class()->hq && cls_of(d)->ey == pico8_qh_struct_class()->ey);
     assert(cls_of(d)->ev == pico8_qh_struct_class()->ev);
     if (qh->jo.ra != NULL) {
diff --git a/tests/test_pico8_trade.c b/tests/test_pico8_trade.c
index c79fe19..0308cdc 100644
--- a/tests/test_pico8_trade.c
+++ b/tests/test_pico8_trade.c
@@ -501,7 +501,7 @@ static void test_cn_wildcard_want(void) {
     qh = pico8_qh_new(NULL);
     qh->jo.jy = jy;
     keys.has_qe = 1;
-    keys.qe = pico8_fixed_from_int(qh->ba->items[0]);
+    keys.qe = pico8_fixed_from_int(qh->jo.def->ba->items[0]);
     lw.t = pico8_trade_new(&keys);
     lw.kf = count_kf;
     pico8_jo_cn(&qh->jo, &lw);
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index 7ce7949..98607b9 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -835,7 +835,7 @@ static int identity_of(const char *s) {
     if (s == pico8_jo_def.me) {
         return 2;
     }
-    if (s == pico8_qh_start.me) {
+    if (s == pico8_qh_def.me) {
         return 3;
     }
     if (s == pico8_pm_start.me) {
```

**`docs/plan-3.md`** — deux notes « Découvert à 2.13d1 » et « Découvert à 2.13d2 », au bout de la ligne 2.13d, qui reçoit aussi la découpe (`2.13d1 — jo → 326 ; 2.13d2 — qh, nl, os → 224`) : comptes (`défauts` **326 / 15** puis **224 / 15**, pied **730 / 72** puis **628 / 72**, `--champs` **259 ; 15 ; 20** puis **255 ; 15 ; 20**, `inclusion-mutuelle` 17 / 16), `game_data.h` **575** puis **536** lignes, total du test ponctuel **231** puis **123**, les filets neufs, les signatures des quatre inits, le `def` de sonde de l'arbre `qh` posé, les champs A gardés membres pour la famille e (`hf`, `fa`, `kq` → 2.13e2 ; `hp`, `px`, `qh` → 2.13e1), le repli s'il a servi. Le reste va au message de commit.

### Test ponctuel : ce qui part, ce qui reste

Même règle qu'en 2.13a à 2.13c : une comparaison part dans le commit qui retire **sa** chaîne, jamais réécrite vers `def`. La chaîne d'un champ est la dernière écriture qui le pose à la construction. Elle part quand cette écriture devient une copie depuis `def` ; elle reste quand c'est encore un `pico8_X_start` (famille e) ou `pico8_ke_start` (`t`, les dix sortes). Pour l'arbre `jo`, cela se lit champ par champ, sorte par sorte :

| Sorte | Reste à `d2` (chaîne vivante) | Part en d1 | Part en d2 |
|---|---|---|---|
| `jo` | `t` | 14 : `me`, `hf`, `hs`, `fa`, `kq`, `ma`, `ka`, `dz`, `cq`, `gq`, `ca`, `em`, `mi`, `has_mi` | — |
| `qh` | `t` | 10 : `hs`, `fa`, `kq`, `ma`, `ka`, `dz`, `cq`, `gq`, `ca`, `em` | 16 : `me`, `hf`, `mi`, `has_mi`, `hp`, `px`, `qh`, `s`, `ba`, sonde ×7 |
| `nl` | `t` | 10 (idem) | 19 : les 16 de `qh`, `rr`, `nl`, `qw` |
| `os` | `t` | 9 (sans `hs`, que `os` écrit) | 17 : les 16, `hs` |
| `hj` | `t`, `hp`, `px`, `qh`, `s`, `probe.od`, `rr`, émetteur ×11 (18) | 10 | 11 : `me`, `hf`, `mi`, `has_mi`, `ba`, `dw`, `has_od`, `dq`, `fp`, `ik`, `has_ik` |
| `pm` | `me`, `t`, `hp`, `px`, `s`, `qh`, `probe.od`, `rr`, `ng`, `fz`, émetteur ×11 (21) | 10 | 10 : ceux de `hj` sans `me` |
| `mx` | `t`, `hp`, `s`, `px`, `probe.od`, `qh`, `rr`, `ht`, émetteur ×11 (19) | 10 | 11 : ceux de `hj` |
| `ga` | `t`, `hf`, `fa`, `kq`, `fz`, `iu`, `ms`, émetteur ×11 (18) | 11 : `me`, `hs`, `ma`, `ka`, `dz`, `cq`, `gq`, `ca`, `em`, `mi`, `has_mi` | — |
| `nr` | `t`, `kq`, `s`, `probe.dw`, `od`, `fp`, `hz`, `rr` (8) | 9 (sans `kq`) | 12 : `me`, `hf`, `mi`, `has_mi`, `hp`, `px`, `qh`, `ba`, `has_od`, `dq`, `ik`, `has_ik` |
| `fr` | idem `nr`, plus `mt` (9) | 9 | 12 (idem `nr`) |
| **total** | 97 + 26 hors arbre = **123** | **102** (333 → **231**) | **108** (231 → **123**) |

Écriture dans le TU : `same_jo_part`/`same_qh_part` disparaissent ; d1 garde un `same_jo_part` réduit à ce que `qh` écrit encore (`me`, `t`, `hf`, `mi`, `has_mi`) ; d2 le remplace par `same_hj_part` (ce que `pico8_hj_struct_init` écrit, pour `hj`, `pm`, `mx`) et `same_nr_part` (ce que `pico8_nr_struct_init` écrit, pour `nr`, `fr`) ; `jo`, `qh`, `nl`, `os` n'y gardent que `t`. Les `assert` nus (`et` NULL, `by` NULL, `lz` a tourné, `kw` partagé) restent. `same_text_or_nil` n'a plus d'appelant en d2 : retiré (`-Werror=unused-function`). Le message du total garde sa forme.

### Comptes, mesurés sur la copie

| Mesure | `p` | `d1` | `d2` |
|---|---|---|---|
| `défauts` (lignes / fichiers) | 403 / 22 | **326 / 15** | **224 / 15** |
| pied `résidu 2` | 807 / 73 | **730 / 72** | **628 / 72** |
| `parente`, `lien-parent`, `accesseurs-q6`, `void-ptr` | 10/3, 111/27, 237/33, 8/4 | identiques | identiques |
| `inclusion-mutuelle` (`--check`) | 17 / 16, 43 lignes | **17 / 16, 43** | **17 / 16, 43** |
| `--champs` | 264 ; 15 ; 20 | **259 ; 15 ; 20** | **255 ; 15 ; 20** |
| `wc -l src/pico8_game_data.h` | 593 | **575** | **536** |
| total du test ponctuel | 333 | **231** | **123** |
| casts / tests de classe | 532 / 707 | 532 / 707 | 532 / 707 |

**Attribution** (script du PRD 2.13a, famille = la plus tardive nommée ; `pico8_jo_kw_start` compté avec `jo`) : les 179 lignes de d se partagent en **77** (d1 : `jo_spec_defaults.c` 37, `planet.c` 14, `game_data.h` 3, `game_data.c` 3, deux par TU de sorte `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, une par `fr`, `ga`, `entity_kind`, `ui`) et **102** (d2 : `jo_spec_defaults.c` 50, `planet_family.c` 27, `entity_kind.c` 14, `game_data.h` 6, `game_data.c` 3, `map_tables.c` 1, `ui.c` 1). À `d2`, les 224 restantes : e1 127, e2 97, rien d'autre. Fichiers : d1 fait sortir de `défauts` `planet.c` et les TU `hj`, `jo`, `mx`, `nl`, `os`, `qh` (22 → 15) ; le pied perd **un** fichier, `test_pico8_jo.c`, que plus aucune catégorie ne compte. d2 ne fait sortir aucun fichier (chacun nomme encore un `_start` de la famille e).

**`--champs`, lu** (`diff` des sorties complètes) : d1 retire six A de `pico8_jo_t` (`ka`, `dz`, `cq`, `gq`, `ca`, `em`) et ajoute `pico8_jo_t def`, 264 → **259** ; « 0 hors constructeur » reste **20** (le nom `def` a déjà une écriture hors constructeur, l'initialiseur de `pb`) ; les quinze couples `<type> def` passent de 23 à 24 écritures par nom ; les couples de même nom perdent une écriture chacun (`fa`, `hf`, `hs`, `kq`, `ma`, `kw`, `me`, et les `ca`/`gq` de `pj_cr`), parce que les copies depuis `def` remplacent des lignes, sans changer de groupe. d2 retire `jo.hs`, `qh.ba`, `nl.rr`, `nl.nl`, 259 → **255** ; mêmes baisses par nom (`hp`, `px`, `qh`, `s`, `od`, `rr`, `me`, `mi`, `has_mi`, `hf`, `qw`), aucun changement de groupe.

Pied, lu : d1, 77 lignes et un fichier ; d2, 102 lignes ; les autres catégories ne bougent pas.

### Contrat pour 2.13e1 et 2.13e2 (à recopier dans leurs PRD ; complète ceux de 2.13a à 2.13c)

| Point | Sous-étape | Ce qu'elle doit faire |
|---|---|---|
| `qh.hp`, `qh.px`, `qh.qh` | 2.13e1 | quand `hj`/`pm`/`mx` ne les écrivent plus, ils quittent `pico8_qh_t` ; lecteurs sur `qh->jo.def->X` ; leurs copies dans `pico8_qh_struct_init` partent |
| `jo.hf`, `jo.fa`, `jo.kq` | 2.13e2 | idem quand `ga`/`nr` ne les écrivent plus : ils quittent `pico8_jo_t` (lecteurs : `pico8_jo_get_fa`, `pico8_jo_get_kq`, `jo->hf`…) ; `kq` se lit `&def->kq` |
| `def` passé à l'init | 2.13e1/e2 | déjà posé : `pico8_hj_struct_init(self, cls, def)`, `pico8_nr_struct_init(self, cls, def)`, `pico8_jo_struct_init(&ga->jo, cls, &pico8_ga_def)` ; les `_new` passent l'archétype de leur sorte. Ne rien réécrire de ce chemin ; seules les écritures `_start` partent |
| `def` d'émetteur de `hj`/`pm`/`mx`, `ga` | 2.13e1, 2.13e2 | inchangé (contrat de 2.13c) : `&self->qh.jo.def->emitter` (ou `&self->jo.def->emitter`), épinglé |
| comparaisons du test ponctuel | 2.13e1, 2.13e2 | celles de la colonne « Reste à `d2` » de la table ci-dessus, sorte par sorte, avec leur `_start` ; `t` des dix sortes avec `pico8_ke_start` (2.13e2) |
| `me` de `pm` | 2.13e1 | `pm` le réécrit (`pico8_pm_start.me`, `"ly"`) : la copie de l'init de `jo` le pose déjà depuis `pico8_pm_def.me` |
| bascule des lecteurs de sonde | **2.13e2** | inchangé (PRD 2.13b) ; à `d2` le `def` de sonde est posé sur **tous** les porteurs (`&pico8_probe_def_none` sur `pb`) |
| filets uniques connus | 2.13e1, 2.13e2 | table du PRD 2.13a moins `probe.has_ik` de l'arbre `qh` (fait ici, pour les huit sortes) ; plus ceux trouvés ici qui touchent la famille e (preuve 3) : ils sont déjà gardés par un littéral de `jo_spec_defaults`, qui doit rester quand 2.13e1/e2 réécrit ce TU |

### Anti-tassement

**Interdit** :
- réécrire une comparaison du test ponctuel vers `def` au lieu de la retirer ; retirer une comparaison dont la chaîne vit encore (colonne « Reste à `d2` », `t` des dix sortes) ;
- faire quitter l'instance à un champ A qu'une sorte de la famille e écrit encore (`hf`, `fa`, `kq`, `hp`, `px`, `qh`), ou faire lire `def` à un de ses lecteurs : l'écriture `_start` deviendrait morte (lignes E de la preuve 3) ;
- passer à l'init l'archétype d'un parent (`&pico8_qh_def` pour `hj`…) au lieu de celui de la sorte ;
- basculer un lecteur (`probe.c`, `particle.c`, `planet_display.c`) sur `probe->def` ou `emitter->def` ; retirer un champ A de `pico8_probe_t` ;
- laisser un `def` (instance, sonde) indéterminé, ou l'écrire ailleurs que dans l'init de son porteur — **TU compris** ;
- garder un littéral d'init de sonde (`has_od = 1`, `ik = 0`, `has_ik = 0`) dans `pico8_qh_struct_init` au lieu de la copie depuis `def` ;
- laisser `kw_own` indéterminé (le « laisser à `lz` » ne vaut pas pour `jo` ni `ga`) ; mettre `kw`, `kw_own` ou `by` dans l'archétype ; changer un type `pico8_X_def_t` ;
- un cast, un `void *`, un aiguillage par classe pour atteindre `def` ;
- nommer un `pico8_X_start` retiré, même en commentaire ;
- toucher `test_pico8_jc.c:853`/`:893`, `test_pico8_pj.c`, `research_icon.h` ;
- changer une constante `*_HASH`/`FROZEN_*`, un `*.sha256`, `tests/e2e/`, `tools/`, le `Makefile`.

## Implementation Chunks

Deux commits, dans l'ordre, chacun vert. **≈ 50 min chacun**, preuves comprises (essai : ≈ 15 min d'écriture et de compilation par commit ; jumelles 102 puis 108, à ≈ 7,5 s l'une, soit ≈ 13 min par côté et ≈ 26 min de machine par commit, lancées en arrière-plan pendant la note et les autres preuves ; ≈ 10 min pour les preuves 2, 4 à 8). La sous-étape entière aurait pris ≈ 1 h 45 : au-dessus du seuil de 50 min, d'où la découpe.

1. **2.13d1** : appliquer le diff de d1 ; `make test`, `make e2e`, compteur, `--champs` ; preuves 1 à 8 (jumelles A, N-def, K, §6) ; note du plan ; commit.
2. **2.13d2** : idem avec le diff de d2 (jumelles B, N-sonde, E).

**Repli écrit** (si un commit déborde de 1,5 fois son budget avant d'être fait), chacun un commit vert :
- d1 en **2.13d1a** (le membre `def`, les quatre signatures, les dix poses, les six A qui quittent l'instance, leurs lecteurs et les TU qui ne compilent plus ; les copies restent lues sur `pico8_jo_start`) puis **2.13d1b** (`pico8_jo_start`, `pico8_jo_kw_start`, `kw_own`/`g_jo_kw_shared`, le test ponctuel et `jo_spec_defaults`) → **326** ;
- d2 en **2.13d2a** (`qh` et la sonde) puis **2.13d2b** (`nl`, `os`) → **224**.
Comptes intermédiaires non mesurés ; le repli, s'il sert, est écrit dans la note « Découvert à ».

## Testing Decisions

- **Aucun TU neuf.** Des littéraux, deux aides (`assert_jo_box`, paramètre `def` de `assert_jo_tail`/`assert_qh_tail`) et des épingles dans `jo_spec_defaults` ; le reste est réécriture. Aucun rejeu d'`events.json` dans un TU.
- **Bon test ici** : un TU durable compare une valeur à un **littéral du cart** ou à l'objet généré qu'elle doit désigner (`kq->xl == pico8_fixed_from_int(-7)`, `jo->def->ka == &pico8_jo_ka_seq`), jamais l'archétype à lui-même. `jo_spec_defaults` compare l'instance à son `def` : cela prouve la pose et la copie, pas la valeur.
- **Prior art** : preuves 1 à 8 du PRD 2.13c ; jumelles « par ligne ajoutée » (G5 de 2.13c).

### Preuves (parent de d1 = le commit 2.13c ; parent de d2 = le commit 2.13d1)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré à `d1` et `d2`.

2. **Règle 1, preuve `const`** (copie du commit ; le membre `def` rendu `*const def`, un à la fois, puis `cc -std=c99 -fsyntax-only -Isrc` de chaque `.c` de `src/` et `tests/`, `-DPICO8_CLASS_COVERAGE` pour les deux `class_coverage`, l'erreur `raylib.h` ôtée) : `pico8_jo_t.def` → **une** erreur, `planet.c:138` (l'init ; aucune dans `tests/`), à `d1` et à `d2` ; `pico8_probe_t.def` → **quatre** à `d2`, `planet_family.c:67` (`&self->jo.def->probe`), `probe.c:448`, `slipway.c:403`, `wormhole.c:246` (l'initialiseur de `pb` n'est pas une affectation). Preuve négative : `j->def->ca = 0;` sur un `pico8_jo_t *` et `n->qh.jo.def->probe.dw = 0;` sur un `pico8_nl_t *` → `assignment of member … in read-only object`. Les A retirés n'ont plus de membre (`--champs`, compilation).

3. **Mutations, règle du parent.** Une jumelle par comparaison qui quitte le test ponctuel (102 en d1, 108 en d2). Parent : **une ligne ajoutée** dans le `_new` de la sorte mesurée, juste avant `#ifdef PICO8_CLASS_COVERAGE` (après toute l'init), pour ne muter qu'elle (l'init de `jo` ou de `qh` sert plusieurs sortes) ; enfant : le membre dans `pico8_X_def`, même valeur neuve. Valeurs : fixes + `0x10000` (`dw` + 1), drapeaux inversés, pointeurs vers un autre objet généré du même type (`&pico8_nl_ba_seq`, `&pico8_os_px_row`…), `me` échangé entre `jo_me_text` et `qh_me_text`, `dq` → `"pr"`, `rr` → `hj_rr_text`, boîte et vecteur décalés d'une unité. `make -k -j8 unit` puis `make -k e2e` ; restaurer par `git checkout -- <fichier>`. **Vert à l'enfant et rouge au parent (hors `archetype_chain`) = ÉCHEC** ; une comparaison de `jo_spec_defaults` à un `_start` compte comme rouge au parent (le filet qu'elle portait part avec la chaîne : l'enfant doit rougir ailleurs). Chaque mutation parente rougit aussi `archetype_chain` ; la colonne l'omet. e2e = conformes sur 26. La colonne enfant est mesurée avec les filets neufs ; « (filet neuf) » marque les jumelles vertes au premier passage.

**Jumelles de d1** (parent `p`, enfant `d1`) :

| # | Valeur | Parent : ligne ajoutée avant `#ifdef PICO8_CLASS_COVERAGE` du `_new` | Rouges au parent (hors `archetype_chain`) | Enfant : membre de `pico8_X_def` | Rouges à l'enfant | e2e p / c |
|---|---|---|---|---|---|---|
| A1 | `jo.me` | `self->header.me = pico8_qh_def.me;` | `hp`, `jo`, `jo_spec_defaults`, `mz`, `ui` | `.me = jo_me_text -> qh_me_text` | `entity_kind`, `hp`, `jo`, `map_tables`, `mz`, `ui` | 9 / 9 |
| A2 | `jo.hf` | `self->hf = 0;` | `hp`, `hy`, `jc`, `jo`, `jo_spec_defaults`, `js`, `lq`, `os` | `.hf = 1 -> 0` | `hp`, `hy`, `jc`, `jo`, `js`, `lq`, `os` | 15 / 15 |
| A3 | `jo.hs` | `self->hs = 0x20000;` | `jo`, `jo_spec_defaults`, `os` | `.hs = 0x10000 -> 0x20000` | `jo`, `os` | 21 / 21 |
| A4 | `jo.fa` | `self->fa = 0x70000;` | `jo`, `jo_spec_defaults`, `ui` | `.fa = 0x60000 -> 0x70000` | `jo`, `ui` | 17 / 17 |
| A5 | `jo.kq` | `{ static const pico8_qz_box_t mut = {-0x80000, -0x70000, 0x70000, 0x70000}; self->kq = &mut; }` | `jo`, `jo_spec_defaults` | `.kq = {-0x70000, -0x70000, 0x70000, 0x70000} -> {-0x80000, -0x70000, 0x70000, 0x70000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A6 | `jo.ma` | `self->ma = 0x10000;` | `fr`, `g_`, `ga`, `hj`, `hp`, `jo`, `jo_spec_defaults`, `lq`, `mx`, `nr`, `pm` | `.ma = 0x0 -> 0x10000` | `fr`, `g_`, `ga`, `hj`, `hp`, `jo`, `lq`, `mx`, `nr`, `pm` | 0 / 0 |
| A7 | `jo.ka` | `self->ka = &pico8_nl_ba_seq;` | `jo_spec_defaults` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A8 | `jo.dz` | `self->dz = &pico8_os_px_row;` | `jo_spec_defaults` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A9 | `jo.cq` | `self->cq = &pico8_nl_ba_seq;` | `jo_spec_defaults` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A10 | `jo.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->gq = &mut; }` | `jo`, `jo_spec_defaults`, `js`, `lq` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `jo`, `js`, `lq` | 14 / 14 |
| A11 | `jo.ca` | `self->ca = 0xa0000;` | `jo`, `jo_spec_defaults`, `js`, `lq` | `.ca = 0x90000 -> 0xa0000` | `jo`, `js`, `lq` | 10 / 10 |
| A12 | `jo.em` | `self->em = &pico8_os_px_row;` | `jo`, `jo_spec_defaults` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `jo` | 26 / 26 |
| A13 | `jo.mi` | `self->mi = 0x10000;` | **aucun** | `.mi = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A14 | `jo.has_mi` | `self->has_mi = 1;` | `jo` | `.has_mi = 0 -> 1` | `jo` | 26 / 26 |
| A15 | `qh.hs` | `self->jo.hs = 0x20000;` | `jo_spec_defaults`, `qh` | `.hs = 0x10000 -> 0x20000` | `qh` | 26 / 26 |
| A16 | `qh.fa` | `self->jo.fa = 0x70000;` | `jo_spec_defaults`, `qh` | `.fa = 0x60000 -> 0x70000` | `qh` | 26 / 26 |
| A17 | `qh.kq` | `{ static const pico8_qz_box_t mut = {-0x80000, -0x70000, 0x70000, 0x70000}; self->jo.kq = &mut; }` | `jo_spec_defaults`, `qh` | `.kq = {-0x70000, -0x70000, 0x70000, 0x70000} -> {-0x80000, -0x70000, 0x70000, 0x70000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A18 | `qh.ma` | `self->jo.ma = 0x10000;` | `jo_spec_defaults`, `qh` | `.ma = 0x0 -> 0x10000` | `qh` | 23 / 23 |
| A19 | `qh.ka` | `self->jo.ka = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `qh` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `qh` | 26 / 26 |
| A20 | `qh.dz` | `self->jo.dz = &pico8_os_px_row;` | `jo_spec_defaults`, `qh` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `qh` | 26 / 26 |
| A21 | `qh.cq` | `self->jo.cq = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `qh` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `qh` | 26 / 26 |
| A22 | `qh.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->jo.gq = &mut; }` | `jo`, `jo_spec_defaults`, `qh` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `jo`, `qh` | 25 / 25 |
| A23 | `qh.ca` | `self->jo.ca = 0xa0000;` | `jo`, `jo_spec_defaults`, `qh` | `.ca = 0x90000 -> 0xa0000` | `jo`, `qh` | 25 / 25 |
| A24 | `qh.em` | `self->jo.em = &pico8_os_px_row;` | `jo`, `jo_spec_defaults`, `qh` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `jo`, `qh` | 26 / 26 |
| A25 | `nl.hs` | `self->qh.jo.hs = 0x20000;` | `jo_spec_defaults`, `nl` | `.hs = 0x10000 -> 0x20000` | `nl` | 26 / 26 |
| A26 | `nl.fa` | `self->qh.jo.fa = 0x70000;` | `jo_spec_defaults`, `nl` | `.fa = 0x60000 -> 0x70000` | `nl` | 26 / 26 |
| A27 | `nl.kq` | `{ static const pico8_qz_box_t mut = {-0x80000, -0x70000, 0x70000, 0x70000}; self->qh.jo.kq = &mut; }` | `jo_spec_defaults`, `nl` | `.kq = {-0x70000, -0x70000, 0x70000, 0x70000} -> {-0x80000, -0x70000, 0x70000, 0x70000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A28 | `nl.ma` | `self->qh.jo.ma = 0x10000;` | `jo_spec_defaults`, `nl` | `.ma = 0x0 -> 0x10000` | `nl` | 25 / 25 |
| A29 | `nl.ka` | `self->qh.jo.ka = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `nl` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `nl` | 26 / 26 |
| A30 | `nl.dz` | `self->qh.jo.dz = &pico8_os_px_row;` | `jo_spec_defaults`, `nl` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `nl` | 26 / 26 |
| A31 | `nl.cq` | `self->qh.jo.cq = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `nl` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `nl` | 26 / 26 |
| A32 | `nl.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->qh.jo.gq = &mut; }` | `jo`, `jo_spec_defaults`, `nl` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `jo`, `nl` | 26 / 26 |
| A33 | `nl.ca` | `self->qh.jo.ca = 0xa0000;` | `jo`, `jo_spec_defaults`, `nl` | `.ca = 0x90000 -> 0xa0000` | `jo`, `nl` | 26 / 26 |
| A34 | `nl.em` | `self->qh.jo.em = &pico8_os_px_row;` | `jo`, `jo_spec_defaults`, `nl` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `jo`, `nl` | 26 / 26 |
| A35 | `hj.hs` | `self->qh.jo.hs = 0x20000;` | `hj`, `jo_spec_defaults` | `.hs = 0x10000 -> 0x20000` | `hj` | 26 / 26 |
| A36 | `hj.fa` | `self->qh.jo.fa = 0x70000;` | `hj`, `jo_spec_defaults` | `.fa = 0x60000 -> 0x70000` | `hj` | 26 / 26 |
| A37 | `hj.kq` | `{ static const pico8_qz_box_t mut = {-0x80000, -0x70000, 0x70000, 0x70000}; self->qh.jo.kq = &mut; }` | `hj`, `jo_spec_defaults` | `.kq = {-0x70000, -0x70000, 0x70000, 0x70000} -> {-0x80000, -0x70000, 0x70000, 0x70000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A38 | `hj.ma` | `self->qh.jo.ma = 0x10000;` | `hj`, `jo_spec_defaults` | `.ma = 0x0 -> 0x10000` | `hj` | 25 / 25 |
| A39 | `hj.ka` | `self->qh.jo.ka = &pico8_nl_ba_seq;` | `hj`, `jo_spec_defaults` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `hj` | 26 / 26 |
| A40 | `hj.dz` | `self->qh.jo.dz = &pico8_os_px_row;` | `hj`, `jo_spec_defaults` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `hj` | 26 / 26 |
| A41 | `hj.cq` | `self->qh.jo.cq = &pico8_nl_ba_seq;` | `hj`, `jo_spec_defaults` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `hj` | 26 / 26 |
| A42 | `hj.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->qh.jo.gq = &mut; }` | `hj`, `jo`, `jo_spec_defaults` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `hj`, `jo` | 26 / 26 |
| A43 | `hj.ca` | `self->qh.jo.ca = 0xa0000;` | `hj`, `jo`, `jo_spec_defaults` | `.ca = 0x90000 -> 0xa0000` | `hj`, `jo` | 26 / 26 |
| A44 | `hj.em` | `self->qh.jo.em = &pico8_os_px_row;` | `hj`, `jo`, `jo_spec_defaults` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `hj`, `jo` | 26 / 26 |
| A45 | `pm.hs` | `self->hj.qh.jo.hs = 0x20000;` | `jo_spec_defaults`, `pm` | `.hs = 0x10000 -> 0x20000` | `pm` | 25 / 25 |
| A46 | `pm.fa` | `self->hj.qh.jo.fa = 0x70000;` | `jo_spec_defaults`, `pm` | `.fa = 0x60000 -> 0x70000` | `pm` | 26 / 26 |
| A47 | `pm.kq` | `{ static const pico8_qz_box_t mut = {-0x80000, -0x70000, 0x70000, 0x70000}; self->hj.qh.jo.kq = &mut; }` | `jo_spec_defaults`, `pm` | `.kq = {-0x70000, -0x70000, 0x70000, 0x70000} -> {-0x80000, -0x70000, 0x70000, 0x70000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A48 | `pm.ma` | `self->hj.qh.jo.ma = 0x10000;` | `jo_spec_defaults`, `pm` | `.ma = 0x0 -> 0x10000` | `pm` | 26 / 26 |
| A49 | `pm.ka` | `self->hj.qh.jo.ka = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `pm` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `pm` | 26 / 26 |
| A50 | `pm.dz` | `self->hj.qh.jo.dz = &pico8_os_px_row;` | `jo_spec_defaults`, `pm` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `pm` | 26 / 26 |
| A51 | `pm.cq` | `self->hj.qh.jo.cq = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `pm` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `pm` | 26 / 26 |
| A52 | `pm.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->hj.qh.jo.gq = &mut; }` | `jo`, `jo_spec_defaults`, `pm` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `jo`, `pm` | 26 / 26 |
| A53 | `pm.ca` | `self->hj.qh.jo.ca = 0xa0000;` | `jo`, `jo_spec_defaults`, `pm` | `.ca = 0x90000 -> 0xa0000` | `jo`, `pm` | 26 / 26 |
| A54 | `pm.em` | `self->hj.qh.jo.em = &pico8_os_px_row;` | `jo`, `jo_spec_defaults`, `pm` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `jo`, `pm` | 26 / 26 |
| A55 | `mx.hs` | `self->hj.qh.jo.hs = 0x20000;` | `jo_spec_defaults`, `mx` | `.hs = 0x10000 -> 0x20000` | `mx` | 25 / 25 |
| A56 | `mx.fa` | `self->hj.qh.jo.fa = 0x70000;` | `jo_spec_defaults`, `mx` | `.fa = 0x60000 -> 0x70000` | `mx` | 26 / 26 |
| A57 | `mx.kq` | `{ static const pico8_qz_box_t mut = {-0x80000, -0x70000, 0x70000, 0x70000}; self->hj.qh.jo.kq = &mut; }` | `jo_spec_defaults`, `mx` | `.kq = {-0x70000, -0x70000, 0x70000, 0x70000} -> {-0x80000, -0x70000, 0x70000, 0x70000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A58 | `mx.ma` | `self->hj.qh.jo.ma = 0x10000;` | `jo_spec_defaults`, `mx` | `.ma = 0x0 -> 0x10000` | `mx` | 25 / 25 |
| A59 | `mx.ka` | `self->hj.qh.jo.ka = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `mx` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `mx` | 26 / 26 |
| A60 | `mx.dz` | `self->hj.qh.jo.dz = &pico8_os_px_row;` | `jo_spec_defaults`, `mx` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `mx` | 26 / 26 |
| A61 | `mx.cq` | `self->hj.qh.jo.cq = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `mx` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `mx` | 26 / 26 |
| A62 | `mx.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->hj.qh.jo.gq = &mut; }` | `jo`, `jo_spec_defaults`, `mx` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `jo`, `mx` | 26 / 26 |
| A63 | `mx.ca` | `self->hj.qh.jo.ca = 0xa0000;` | `jo`, `jo_spec_defaults`, `mx` | `.ca = 0x90000 -> 0xa0000` | `jo`, `mx` | 26 / 26 |
| A64 | `mx.em` | `self->hj.qh.jo.em = &pico8_os_px_row;` | `jo`, `jo_spec_defaults`, `mx` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `jo`, `mx` | 26 / 26 |
| A65 | `os.fa` | `self->qh.jo.fa = 0x70000;` | `jo_spec_defaults`, `os` | `.fa = 0x60000 -> 0x70000` | `os` | 26 / 26 |
| A66 | `os.kq` | `{ static const pico8_qz_box_t mut = {-0x80000, -0x70000, 0x70000, 0x70000}; self->qh.jo.kq = &mut; }` | `jo_spec_defaults`, `os` | `.kq = {-0x70000, -0x70000, 0x70000, 0x70000} -> {-0x80000, -0x70000, 0x70000, 0x70000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A67 | `os.ma` | `self->qh.jo.ma = 0x10000;` | `jo_spec_defaults`, `os` | `.ma = 0x0 -> 0x10000` | `os` | 25 / 25 |
| A68 | `os.ka` | `self->qh.jo.ka = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `os` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `os` | 26 / 26 |
| A69 | `os.dz` | `self->qh.jo.dz = &pico8_os_px_row;` | `jo_spec_defaults`, `os` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `os` | 26 / 26 |
| A70 | `os.cq` | `self->qh.jo.cq = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `os` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `os` | 26 / 26 |
| A71 | `os.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->qh.jo.gq = &mut; }` | `jo`, `jo_spec_defaults`, `os` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `jo`, `os` | 26 / 26 |
| A72 | `os.ca` | `self->qh.jo.ca = 0xa0000;` | `jo`, `jo_spec_defaults`, `os` | `.ca = 0x90000 -> 0xa0000` | `jo`, `os` | 26 / 26 |
| A73 | `os.em` | `self->qh.jo.em = &pico8_os_px_row;` | `jo`, `jo_spec_defaults`, `os` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `jo`, `os` | 26 / 26 |
| A74 | `nr.hs` | `self->qh.jo.hs = 0x20000;` | `jo_spec_defaults`, `nr` | `.hs = 0x10000 -> 0x20000` | `nr` | 26 / 26 |
| A75 | `nr.fa` | `self->qh.jo.fa = 0x70000;` | `jo_spec_defaults`, `nr` | `.fa = 0x60000 -> 0x70000` | `nr` | 26 / 26 |
| A76 | `nr.ma` | `self->qh.jo.ma = 0x10000;` | `jo_spec_defaults`, `nr` | `.ma = 0x0 -> 0x10000` | `nr` | 24 / 24 |
| A77 | `nr.ka` | `self->qh.jo.ka = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `nr` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `nr` | 26 / 26 |
| A78 | `nr.dz` | `self->qh.jo.dz = &pico8_os_px_row;` | `jo_spec_defaults`, `nr` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `nr` | 26 / 26 |
| A79 | `nr.cq` | `self->qh.jo.cq = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `nr` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `nr` | 26 / 26 |
| A80 | `nr.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->qh.jo.gq = &mut; }` | `jo`, `jo_spec_defaults`, `nr` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `jo`, `nr` | 26 / 26 |
| A81 | `nr.ca` | `self->qh.jo.ca = 0xa0000;` | `jo`, `jo_spec_defaults`, `nr` | `.ca = 0x90000 -> 0xa0000` | `jo`, `nr` | 26 / 26 |
| A82 | `nr.em` | `self->qh.jo.em = &pico8_os_px_row;` | `jo`, `jo_spec_defaults`, `nr` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `jo`, `nr` | 26 / 26 |
| A83 | `fr.hs` | `self->nr.qh.jo.hs = 0x20000;` | `fr`, `jo_spec_defaults` | `.hs = 0x10000 -> 0x20000` | `fr` | 26 / 26 |
| A84 | `fr.fa` | `self->nr.qh.jo.fa = 0x70000;` | `fr`, `jo_spec_defaults` | `.fa = 0x60000 -> 0x70000` | `fr` | 26 / 26 |
| A85 | `fr.ma` | `self->nr.qh.jo.ma = 0x10000;` | `fr`, `jo_spec_defaults` | `.ma = 0x0 -> 0x10000` | `fr` | 25 / 25 |
| A86 | `fr.ka` | `self->nr.qh.jo.ka = &pico8_nl_ba_seq;` | `fr`, `jo_spec_defaults` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `fr` | 26 / 26 |
| A87 | `fr.dz` | `self->nr.qh.jo.dz = &pico8_os_px_row;` | `fr`, `jo_spec_defaults` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `fr` | 26 / 26 |
| A88 | `fr.cq` | `self->nr.qh.jo.cq = &pico8_nl_ba_seq;` | `fr`, `jo_spec_defaults` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `fr` | 26 / 26 |
| A89 | `fr.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->nr.qh.jo.gq = &mut; }` | `fr`, `jo`, `jo_spec_defaults` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `fr`, `jo` | 26 / 26 |
| A90 | `fr.ca` | `self->nr.qh.jo.ca = 0xa0000;` | `fr`, `jo`, `jo_spec_defaults` | `.ca = 0x90000 -> 0xa0000` | `fr`, `jo` | 26 / 26 |
| A91 | `fr.em` | `self->nr.qh.jo.em = &pico8_os_px_row;` | `fr`, `jo`, `jo_spec_defaults` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `fr`, `jo` | 26 / 26 |
| A92 | `ga.me` | `self->jo.header.me = pico8_qh_def.me;` | `ga`, `hp`, `jo_spec_defaults`, `ui` | `.me = jo_me_text -> qh_me_text` | `ga`, `hp`, `ui` | 26 / 26 |
| A93 | `ga.hs` | `self->jo.hs = 0x20000;` | `jo_spec_defaults` | `.hs = 0x10000 -> 0x20000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A94 | `ga.ma` | `self->jo.ma = 0x10000;` | `ga`, `jo_spec_defaults` | `.ma = 0x0 -> 0x10000` | `ga` | 26 / 26 |
| A95 | `ga.ka` | `self->jo.ka = &pico8_nl_ba_seq;` | `ga`, `jo_spec_defaults` | `.ka = &pico8_jo_ka_seq -> &pico8_nl_ba_seq` | `ga` | 26 / 26 |
| A96 | `ga.dz` | `self->jo.dz = &pico8_os_px_row;` | `ga`, `jo_spec_defaults` | `.dz = pico8_jo_dz_rows -> &pico8_os_px_row` | `ga` | 26 / 26 |
| A97 | `ga.cq` | `self->jo.cq = &pico8_nl_ba_seq;` | `ga`, `jo_spec_defaults` | `.cq = &pico8_jo_cq_seq -> &pico8_nl_ba_seq` | `ga` | 26 / 26 |
| A98 | `ga.gq` | `{ static const pico8_vec_t mut = {0x10000, 0xb0000}; self->jo.gq = &mut; }` | `ga`, `jo`, `jo_spec_defaults` | `.gq = {0x0, 0xb0000} -> {0x10000, 0xb0000}` | `jo` | 26 / 26 |
| A99 | `ga.ca` | `self->jo.ca = 0xa0000;` | `jo`, `jo_spec_defaults` | `.ca = 0x90000 -> 0xa0000` | `jo` | 26 / 26 |
| A100 | `ga.em` | `self->jo.em = &pico8_os_px_row;` | `ga`, `jo`, `jo_spec_defaults` | `.em = &pico8_jo_em_row -> &pico8_os_px_row` | `ga`, `jo` | 26 / 26 |
| A101 | `ga.mi` | `self->jo.mi = 0x10000;` | **aucun** | `.mi = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| A102 | `ga.has_mi` | `self->jo.has_mi = 1;` | `ga` | `.has_mi = 0 -> 1` | `ga` | 26 / 26 |

**Jumelles de d2** (parent `d1`, enfant `d2`) :

| # | Valeur | Parent : ligne ajoutée avant `#ifdef PICO8_CLASS_COVERAGE` du `_new` | Rouges au parent (hors `archetype_chain`) | Enfant : membre de `pico8_X_def` | Rouges à l'enfant | e2e p / c |
|---|---|---|---|---|---|---|
| B1 | `qh.me` | `self->jo.header.me = pico8_jo_def.me;` | `jo_spec_defaults`, `qh`, `ui` | `.me = qh_me_text -> jo_me_text` | `entity_kind`, `map_tables`, `qh`, `ui` | 23 / 23 |
| B2 | `qh.hf` | `self->jo.hf = 1;` | `jo_spec_defaults`, `qh` | `.hf = 0 -> 1` | `qh` | 23 / 23 |
| B3 | `qh.mi` | `self->jo.mi = 0x20000;` | `jo_spec_defaults`, `qh` | `.mi = 0x10000 -> 0x20000` | `qh` | 23 / 23 |
| B4 | `qh.has_mi` | `self->jo.has_mi = 0;` | `jo_spec_defaults`, `qh` | `.has_mi = 1 -> 0` | `jo_spec_defaults`, `qh` | 26 / 26 |
| B5 | `qh.hp` | `self->hp = 0x20000;` | `jo_spec_defaults`, `qh` | `.hp = 0x10000 -> 0x20000` | `qh` | 23 / 23 |
| B6 | `qh.px` | `self->px = &pico8_os_px_row;` | `jo_spec_defaults`, `qh` | `.px = &pico8_qh_px_row -> &pico8_os_px_row` | `qh` | 25 / 25 |
| B7 | `qh.qh` | `self->qh = 0;` | `jo_spec_defaults`, `lq`, `qh` | `.qh = 1 -> 0` | `lq`, `qh` | 23 / 23 |
| B8 | `qh.s` | `self->s = 0xaf0000;` | `jo_spec_defaults`, `qh` | `.s = 0xae0000 -> 0xaf0000` | `entity_kind`, `qh` | 23 / 23 |
| B9 | `qh.ba` | `self->ba = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `qh` | `.ba = &pico8_qh_ba_seq -> &pico8_nl_ba_seq` | `qh` | 26 / 26 |
| B10 | `qh.dw` | `self->probe->dw = 0x667;` | `jo_spec_defaults`, `qh` | `.dw = 0x666 -> 0x667` | `qh` | 26 / 26 |
| B11 | `qh.od` | `self->probe->od = 0x100000;` | `jo_spec_defaults`, `qh` | `.od = 0xf0000 -> 0x100000` | `entity_kind`, `qh` | 23 / 23 |
| B12 | `qh.has_od` | `self->probe->has_od = 0;` | `qh` | `.has_od = 1 -> 0` | `qh` | 23 / 23 |
| B13 | `qh.dq` | `self->probe->dq = "pr";` | `jo_spec_defaults`, `qh` | `.dq = qh_dq_text -> "pr"` | `map_tables`, `qh` | 24 / 24 |
| B14 | `qh.fp` | `self->probe->fp = 0x30000;` | `jo_spec_defaults`, `qh` | `.fp = 0x20000 -> 0x30000` | `qh` | 23 / 23 |
| B15 | `qh.ik` | `self->probe->ik = 0x10000;` | **aucun** | `.ik = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B16 | `qh.has_ik` | `self->probe->has_ik = 1;` | **aucun** | `.has_ik = 0 -> 1` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B17 | `nl.me` | `self->qh.jo.header.me = pico8_jo_def.me;` | `jo_spec_defaults`, `nl`, `ui` | `.me = qh_me_text -> jo_me_text` | `nl`, `ui` | 25 / 25 |
| B18 | `nl.hf` | `self->qh.jo.hf = 1;` | `jo_spec_defaults`, `nl`, `qh` | `.hf = 0 -> 1` | `nl`, `qh` | 25 / 25 |
| B19 | `nl.mi` | `self->qh.jo.mi = 0x20000;` | `jo_spec_defaults`, `nl`, `qh` | `.mi = 0x10000 -> 0x20000` | `nl`, `qh` | 25 / 25 |
| B20 | `nl.has_mi` | `self->qh.jo.has_mi = 0;` | `jo_spec_defaults`, `nl`, `qh` | `.has_mi = 1 -> 0` | `jo_spec_defaults`, `nl`, `qh` | 26 / 26 |
| B21 | `nl.hp` | `self->qh.hp = 0x70000;` | `jo_spec_defaults`, `nl` | `.hp = 0x60000 -> 0x70000` | `nl` | 25 / 25 |
| B22 | `nl.px` | `self->qh.px = &pico8_os_px_row;` | `jo_spec_defaults`, `nl` | `.px = &pico8_nl_px_row -> &pico8_os_px_row` | `nl` | 26 / 26 |
| B23 | `nl.qh` | `self->qh.qh = 1;` | `jo_spec_defaults`, `nl` | `.qh = 0 -> 1` | `nl` | 26 / 26 |
| B24 | `nl.s` | `self->qh.s = 0x450000;` | `jo_spec_defaults`, `nl` | `.s = 0x440000 -> 0x450000` | `entity_kind`, `nl` | 25 / 25 |
| B25 | `nl.ba` | `self->qh.ba = &pico8_qh_ba_seq;` | `jo_spec_defaults`, `nl`, `qh` | `.ba = &pico8_nl_ba_seq -> &pico8_qh_ba_seq` | `nl`, `qh` | 26 / 26 |
| B26 | `nl.dw` | `self->qh.probe->dw = 0x667;` | `jo_spec_defaults`, `nl`, `qh` | `.dw = 0x666 -> 0x667` | `nl`, `qh` | 26 / 26 |
| B27 | `nl.od` | `self->qh.probe->od = 0x150000;` | `jo_spec_defaults`, `nl` | `.od = 0x140000 -> 0x150000` | `entity_kind`, `nl` | 25 / 25 |
| B28 | `nl.has_od` | `self->qh.probe->has_od = 0;` | `nl` | `.has_od = 1 -> 0` | `nl` | 25 / 25 |
| B29 | `nl.dq` | `self->qh.probe->dq = "pr";` | `jo_spec_defaults`, `nl`, `qh` | `.dq = qh_dq_text -> "pr"` | `nl`, `qh` | 25 / 25 |
| B30 | `nl.fp` | `self->qh.probe->fp = 0x30000;` | `jo_spec_defaults`, `nl`, `qh` | `.fp = 0x20000 -> 0x30000` | `nl`, `qh` | 25 / 25 |
| B31 | `nl.ik` | `self->qh.probe->ik = 0x10000;` | **aucun** | `.ik = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B32 | `nl.has_ik` | `self->qh.probe->has_ik = 1;` | **aucun** | `.has_ik = 0 -> 1` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B33 | `nl.rr` | `self->rr = pico8_hj_def.rr;` | `jo_spec_defaults`, `nl` | `.rr = nl_rr_text -> hj_rr_text` | `entity_kind`, `map_tables`, `nl` | 26 / 26 |
| B34 | `nl.nl` | `self->nl = 0x20000;` | `jo_spec_defaults`, `nl` | `.nl = 0x10000 -> 0x20000` | `nl` | 26 / 26 |
| B35 | `nl.qw` | `self->qw = 0x100000;` | `jo_spec_defaults`, `nl`, `qh` | `.qw = 0xf0000 -> 0x100000` | `nl`, `qh` | 26 / 26 |
| B36 | `os.me` | `self->qh.jo.header.me = pico8_jo_def.me;` | `jo_spec_defaults`, `os`, `ui` | `.me = qh_me_text -> jo_me_text` | `os`, `ui` | 25 / 25 |
| B37 | `os.hf` | `self->qh.jo.hf = 1;` | `jo_spec_defaults`, `os`, `qh` | `.hf = 0 -> 1` | `os`, `qh` | 25 / 25 |
| B38 | `os.mi` | `self->qh.jo.mi = 0x20000;` | `jo_spec_defaults`, `os`, `qh` | `.mi = 0x10000 -> 0x20000` | `os`, `qh` | 25 / 25 |
| B39 | `os.has_mi` | `self->qh.jo.has_mi = 0;` | `jo_spec_defaults`, `os`, `qh` | `.has_mi = 1 -> 0` | `jo_spec_defaults`, `os`, `qh` | 26 / 26 |
| B40 | `os.hp` | `self->qh.hp = 0x50000;` | `jo_spec_defaults`, `os` | `.hp = 0x40000 -> 0x50000` | `os` | 25 / 25 |
| B41 | `os.px` | `self->qh.px = &pico8_nl_px_row;` | `jo_spec_defaults`, `os` | `.px = &pico8_os_px_row -> &pico8_nl_px_row` | `os` | 26 / 26 |
| B42 | `os.qh` | `self->qh.qh = 1;` | `jo_spec_defaults`, `os` | `.qh = 0 -> 1` | `os` | 26 / 26 |
| B43 | `os.s` | `self->qh.s = 0x650000;` | `jo_spec_defaults`, `os` | `.s = 0x640000 -> 0x650000` | `entity_kind`, `os` | 25 / 25 |
| B44 | `os.ba` | `self->qh.ba = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `os`, `qh` | `.ba = &pico8_qh_ba_seq -> &pico8_nl_ba_seq` | `os`, `qh` | 26 / 26 |
| B45 | `os.dw` | `self->qh.probe->dw = 0x667;` | `jo_spec_defaults`, `os`, `qh` | `.dw = 0x666 -> 0x667` | `os`, `qh` | 26 / 26 |
| B46 | `os.od` | `self->qh.probe->od = 0x100000;` | `jo_spec_defaults`, `os` | `.od = 0xf0000 -> 0x100000` | `entity_kind`, `os` | 25 / 25 |
| B47 | `os.has_od` | `self->qh.probe->has_od = 0;` | `os` | `.has_od = 1 -> 0` | `os` | 25 / 25 |
| B48 | `os.dq` | `self->qh.probe->dq = "pr";` | `jo_spec_defaults`, `os`, `qh` | `.dq = qh_dq_text -> "pr"` | `os`, `qh` | 25 / 25 |
| B49 | `os.fp` | `self->qh.probe->fp = 0x30000;` | `jo_spec_defaults`, `os`, `qh` | `.fp = 0x20000 -> 0x30000` | `os`, `qh` | 25 / 25 |
| B50 | `os.ik` | `self->qh.probe->ik = 0x10000;` | **aucun** | `.ik = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B51 | `os.has_ik` | `self->qh.probe->has_ik = 1;` | **aucun** | `.has_ik = 0 -> 1` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B52 | `os.hs` | `self->qh.jo.hs = 0x18000;` | `jo_spec_defaults`, `os` | `.hs = 0x8000 -> 0x18000` | `os` | 26 / 26 |
| B53 | `hj.me` | `self->qh.jo.header.me = pico8_jo_def.me;` | `hj`, `jo_spec_defaults`, `ui` | `.me = qh_me_text -> jo_me_text` | `hj`, `ui` | 25 / 25 |
| B54 | `hj.hf` | `self->qh.jo.hf = 1;` | `hj`, `jo_spec_defaults`, `qh` | `.hf = 0 -> 1` | `hj`, `qh` | 25 / 25 |
| B55 | `hj.mi` | `self->qh.jo.mi = 0x20000;` | `hj`, `jo_spec_defaults`, `mx`, `qh` | `.mi = 0x10000 -> 0x20000` | `hj`, `mx`, `qh` | 25 / 25 |
| B56 | `hj.has_mi` | `self->qh.jo.has_mi = 0;` | `hj`, `jo_spec_defaults`, `qh` | `.has_mi = 1 -> 0` | `hj`, `jo_spec_defaults`, `qh` | 26 / 26 |
| B57 | `hj.ba` | `self->qh.ba = &pico8_nl_ba_seq;` | `hj`, `jo_spec_defaults`, `qh` | `.ba = &pico8_qh_ba_seq -> &pico8_nl_ba_seq` | `hj`, `qh` | 26 / 26 |
| B58 | `hj.dw` | `self->qh.probe->dw = 0x667;` | `hj`, `jo_spec_defaults`, `qh` | `.dw = 0x666 -> 0x667` | `hj`, `qh` | 26 / 26 |
| B59 | `hj.has_od` | `self->qh.probe->has_od = 0;` | `hj` | `.has_od = 1 -> 0` | `hj` | 25 / 25 |
| B60 | `hj.dq` | `self->qh.probe->dq = "pr";` | `hj`, `jo_spec_defaults`, `qh` | `.dq = qh_dq_text -> "pr"` | `hj`, `qh` | 25 / 25 |
| B61 | `hj.fp` | `self->qh.probe->fp = 0x30000;` | `hj`, `jo_spec_defaults`, `qh` | `.fp = 0x20000 -> 0x30000` | `hj`, `qh` | 25 / 25 |
| B62 | `hj.ik` | `self->qh.probe->ik = 0x10000;` | **aucun** | `.ik = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B63 | `hj.has_ik` | `self->qh.probe->has_ik = 1;` | **aucun** | `.has_ik = 0 -> 1` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B64 | `mx.me` | `self->hj.qh.jo.header.me = pico8_jo_def.me;` | `hj`, `jo_spec_defaults`, `mx`, `ui` | `.me = qh_me_text -> jo_me_text` | `hj`, `mx`, `ui` | 25 / 25 |
| B65 | `mx.hf` | `self->hj.qh.jo.hf = 1;` | `jo_spec_defaults`, `mx`, `qh` | `.hf = 0 -> 1` | `mx`, `qh` | 25 / 25 |
| B66 | `mx.mi` | `self->hj.qh.jo.mi = 0x20000;` | `jo_spec_defaults`, `mx`, `qh` | `.mi = 0x10000 -> 0x20000` | `mx`, `qh` | 25 / 25 |
| B67 | `mx.has_mi` | `self->hj.qh.jo.has_mi = 0;` | `jo_spec_defaults`, `mx`, `qh` | `.has_mi = 1 -> 0` | `jo_spec_defaults`, `mx`, `qh` | 26 / 26 |
| B68 | `mx.ba` | `self->hj.qh.ba = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `mx`, `qh` | `.ba = &pico8_qh_ba_seq -> &pico8_nl_ba_seq` | `mx`, `qh` | 26 / 26 |
| B69 | `mx.dw` | `self->hj.qh.probe->dw = 0x667;` | `jo_spec_defaults`, `mx`, `qh` | `.dw = 0x666 -> 0x667` | `mx`, `qh` | 26 / 26 |
| B70 | `mx.has_od` | `self->hj.qh.probe->has_od = 0;` | `mx` | `.has_od = 1 -> 0` | `mx` | 25 / 25 |
| B71 | `mx.dq` | `self->hj.qh.probe->dq = "pr";` | `jo_spec_defaults`, `mx`, `qh` | `.dq = qh_dq_text -> "pr"` | `mx`, `qh` | 25 / 25 |
| B72 | `mx.fp` | `self->hj.qh.probe->fp = 0x30000;` | `jo_spec_defaults`, `mx`, `qh` | `.fp = 0x20000 -> 0x30000` | `mx`, `qh` | 25 / 25 |
| B73 | `mx.ik` | `self->hj.qh.probe->ik = 0x10000;` | **aucun** | `.ik = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B74 | `mx.has_ik` | `self->hj.qh.probe->has_ik = 1;` | **aucun** | `.has_ik = 0 -> 1` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B75 | `pm.hf` | `self->hj.qh.jo.hf = 1;` | `ga`, `hp`, `jo`, `jo_spec_defaults`, `pm`, `qh` | `.hf = 0 -> 1` | `ga`, `hp`, `jo`, `pm`, `qh` | 25 / 25 |
| B76 | `pm.mi` | `self->hj.qh.jo.mi = 0x20000;` | `jo_spec_defaults`, `pm` | `.mi = 0x10000 -> 0x20000` | `pm` | 26 / 26 |
| B77 | `pm.has_mi` | `self->hj.qh.jo.has_mi = 0;` | `jo_spec_defaults`, `pm` | `.has_mi = 1 -> 0` | `jo_spec_defaults`, `pm` | 26 / 26 |
| B78 | `pm.ba` | `self->hj.qh.ba = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `pm`, `qh` | `.ba = &pico8_qh_ba_seq -> &pico8_nl_ba_seq` | `pm`, `qh` | 26 / 26 |
| B79 | `pm.dw` | `self->hj.qh.probe->dw = 0x667;` | `jo_spec_defaults`, `pm`, `qh` | `.dw = 0x666 -> 0x667` | `pm`, `qh` | 26 / 26 |
| B80 | `pm.has_od` | `self->hj.qh.probe->has_od = 0;` | **aucun** | `.has_od = 1 -> 0` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B81 | `pm.dq` | `self->hj.qh.probe->dq = "pr";` | `jo_spec_defaults`, `pm`, `qh` | `.dq = qh_dq_text -> "pr"` | `pm`, `qh` | 26 / 26 |
| B82 | `pm.fp` | `self->hj.qh.probe->fp = 0x30000;` | `jo_spec_defaults`, `pm`, `qh` | `.fp = 0x20000 -> 0x30000` | `pm`, `qh` | 26 / 26 |
| B83 | `pm.ik` | `self->hj.qh.probe->ik = 0x10000;` | **aucun** | `.ik = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B84 | `pm.has_ik` | `self->hj.qh.probe->has_ik = 1;` | **aucun** | `.has_ik = 0 -> 1` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B85 | `nr.me` | `self->qh.jo.header.me = pico8_jo_def.me;` | `jo_spec_defaults`, `nr`, `ui` | `.me = qh_me_text -> jo_me_text` | `nr`, `ui` | 24 / 24 |
| B86 | `nr.hf` | `self->qh.jo.hf = 1;` | `jo_spec_defaults`, `nr`, `qh` | `.hf = 0 -> 1` | `nr`, `qh` | 24 / 24 |
| B87 | `nr.mi` | `self->qh.jo.mi = 0x20000;` | `jo_spec_defaults`, `nr`, `qh` | `.mi = 0x10000 -> 0x20000` | `nr`, `qh` | 26 / 26 |
| B88 | `nr.has_mi` | `self->qh.jo.has_mi = 0;` | `jo_spec_defaults`, `nr`, `qh` | `.has_mi = 1 -> 0` | `jo_spec_defaults`, `nr`, `qh` | 26 / 26 |
| B89 | `nr.hp` | `self->qh.hp = 0x20000;` | `jo_spec_defaults`, `nr` | `.hp = 0x10000 -> 0x20000` | `nr` | 26 / 26 |
| B90 | `nr.px` | `self->qh.px = &pico8_os_px_row;` | `jo_spec_defaults`, `nr`, `qh` | `.px = &pico8_qh_px_row -> &pico8_os_px_row` | `nr`, `qh` | 26 / 26 |
| B91 | `nr.qh` | `self->qh.qh = 0;` | `jo_spec_defaults`, `nr` | `.qh = 1 -> 0` | `nr` | 26 / 26 |
| B92 | `nr.ba` | `self->qh.ba = &pico8_nl_ba_seq;` | `jo_spec_defaults`, `nr`, `qh` | `.ba = &pico8_qh_ba_seq -> &pico8_nl_ba_seq` | `nr`, `qh` | 26 / 26 |
| B93 | `nr.has_od` | `self->qh.probe->has_od = 0;` | `nr` | `.has_od = 1 -> 0` | `nr` | 24 / 24 |
| B94 | `nr.dq` | `self->qh.probe->dq = "pr";` | `jo_spec_defaults`, `nr`, `qh` | `.dq = qh_dq_text -> "pr"` | `nr`, `qh` | 24 / 24 |
| B95 | `nr.ik` | `self->qh.probe->ik = 0x10000;` | **aucun** | `.ik = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B96 | `nr.has_ik` | `self->qh.probe->has_ik = 1;` | **aucun** | `.has_ik = 0 -> 1` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B97 | `fr.me` | `self->nr.qh.jo.header.me = pico8_jo_def.me;` | `fr`, `jo_spec_defaults`, `ui` | `.me = qh_me_text -> jo_me_text` | `fr`, `ui` | 25 / 25 |
| B98 | `fr.hf` | `self->nr.qh.jo.hf = 1;` | `fr`, `jo_spec_defaults`, `qh` | `.hf = 0 -> 1` | `fr`, `qh` | 25 / 25 |
| B99 | `fr.mi` | `self->nr.qh.jo.mi = 0x20000;` | `fr`, `jo_spec_defaults`, `qh` | `.mi = 0x10000 -> 0x20000` | `fr`, `qh` | 26 / 26 |
| B100 | `fr.has_mi` | `self->nr.qh.jo.has_mi = 0;` | `fr`, `jo_spec_defaults`, `qh` | `.has_mi = 1 -> 0` | `fr`, `jo_spec_defaults`, `qh` | 26 / 26 |
| B101 | `fr.hp` | `self->nr.qh.hp = 0x20000;` | `fr`, `jo_spec_defaults` | `.hp = 0x10000 -> 0x20000` | `fr` | 26 / 26 |
| B102 | `fr.px` | `self->nr.qh.px = &pico8_os_px_row;` | `fr`, `jo_spec_defaults`, `qh` | `.px = &pico8_qh_px_row -> &pico8_os_px_row` | `fr`, `qh` | 26 / 26 |
| B103 | `fr.qh` | `self->nr.qh.qh = 0;` | `fr`, `jo_spec_defaults` | `.qh = 1 -> 0` | `fr` | 26 / 26 |
| B104 | `fr.ba` | `self->nr.qh.ba = &pico8_nl_ba_seq;` | `fr`, `jo_spec_defaults`, `qh` | `.ba = &pico8_qh_ba_seq -> &pico8_nl_ba_seq` | `fr`, `qh` | 26 / 26 |
| B105 | `fr.has_od` | `self->nr.qh.probe->has_od = 0;` | `fr` | `.has_od = 1 -> 0` | `fr` | 25 / 25 |
| B106 | `fr.dq` | `self->nr.qh.probe->dq = "pr";` | `fr`, `jo_spec_defaults`, `qh` | `.dq = qh_dq_text -> "pr"` | `fr`, `qh` | 25 / 25 |
| B107 | `fr.ik` | `self->nr.qh.probe->ik = 0x10000;` | **aucun** | `.ik = 0x0 -> 0x10000` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| B108 | `fr.has_ik` | `self->nr.qh.probe->has_ik = 1;` | **aucun** | `.has_ik = 0 -> 1` | `jo_spec_defaults` (filet neuf) | 26 / 26 |


   **Lignes N, K, E** (N-def et K à `d1`, N-sonde à `d2` ; E à `p` et à `d2`, rouges hors `archetype_chain` sauf mention) :

   | # | Valeur | Mutation | Rouges | e2e |
   |---|---|---|---|---|
   | N1–N10 | `def` d'instance de chaque sorte | le `_new` passe l'archétype d'une sœur (`jo`→`qh`, `qh`→`os`, `nl`→`qh`, `os`→`qh`, `hj`→`pm`, `pm`→`hj`, `mx`→`hj`, `ga`→`jo`, `nr`→`fr`, `fr`→`nr`) | `jo_spec_defaults` et le TU de la sorte, les dix ; `jo`→`qh` rougit aussi `hp`, `hy`, `jc`, `js`, `lq`, `mz`, `os`, `ui` | 26 (9 pour `jo`) |
   | N11 | `def` de sonde de l'arbre `qh` | `&self->jo.def->probe` → `&pico8_qh_def.probe` | `jo_spec_defaults` seul | 26 |
   | N12 | idem | → `&pico8_nl_def.probe` | `jo_spec_defaults` seul | 26 |
   | K1 | parent : `pico8_jo_kw_start.mu` → 1 | `{0, 0x0, 0x10000}` | `hp`, `jo` | 26 |
   | K2 | enfant : `g_jo_kw_shared` posé par l'init | `(pico8_kw_t){0, 0x0, 0x10000}` | `hp`, `jo`, `jo_spec_defaults` | 26 |
   | K3 | enfant : `kw_own` mis à zéro | idem sur `kw_own` | `jo_spec_defaults` (littéral ; `lz` réécrit la valeur avant toute lecture) | 26 |
   | E1 | `pico8_hj_start.hp` → `0x30000` (à `p` et à `d2`) | `hj`, `mx` aux deux | 25 / 25 |
   | E2 | `pico8_hj_start.qh` → 1 | `hj`, `mx`, `pm` aux deux | 26 / 26 |
   | E3 | `pico8_pm_start.px` → `&pico8_os_px_row` | `hj`, `pm` aux deux | 26 / 26 |
   | E4 | `pico8_pm_start.me` → `qh_me_text` | `entity_kind`, `hj`, `hp`, `map_tables`, `pm`, `qh`, `ui` aux deux | 25 / 25 |
   | E5 | `pico8_ga_start.hf` → 1 | `ga` aux deux | 26 / 26 |
   | E6 | `pico8_ga_start.kq` : `xl` → `0x10000` | **`archetype_chain` seul**, aux deux | 26 / 26 |
   | E7 | `pico8_ga_start.fa` → `0x40000` | `ga` aux deux | 26 / 26 |
   | E8 | `pico8_nr_start.kq` : `xl` → `-0x50000` | **`archetype_chain` seul**, aux deux | 26 / 26 |
   | E9 | `pico8_nr_start.dw` → `0x28f` | `fr`, `nr`, `qh` aux deux | 26 / 26 |

   Lecture : **aucun ÉCHEC** à `d1` ni à `d2`, filets neufs compris. **Filets uniques trouvés** — au sens large (verts à l'enfant avant les littéraux) : d1, la boîte `kq` des sept sortes qui gardent celle de `jo` (A5, A17, A27, A37, A47, A57, A66 : au parent, seuls l'identité `jo->kq == &pico8_jo_start.kq` et `jo_spec_defaults` la voyaient ; le jeu ne lit que son bord, que le décalage ne change pas), `ka`/`dz`/`cq` de `jo` (A7–A9 : aucun lecteur dans `src/`), `mi` de `jo` et de `ga` (A13, A101 : `has_mi` nul, valeur morte, rouge au seul `archetype_chain` au parent), `hs` de `ga` (A93 : aucun lecteur pour `ga`) ; d2, `ik`/`has_ik` des huit sortes (B15… : valeurs mortes, lues seulement si `has_ik`) et `has_od` de `pm` (B80 ; celui des sept autres est vu par `qh`/`nl`… ou `entity_kind`). Tous rougissent `jo_spec_defaults` avec les littéraux. **Contrôles E** : aucune écriture `_start` de la famille e n'est devenue morte ; `ga.kq` et `nr.kq` n'ont que le test ponctuel pour filet (E6, E8), aux deux commits : leur comparaison reste (chaîne vivante), **à garder par un littéral en 2.13e2** (contrat).

4. **Chemin touché** : `make line-coverage EVENTS="<les 26 e2e>"` sur `d2` (6279 lignes) : la pose `planet.c:138`, le `_new` de `jo` (`planet.c:175`), `kw_own` (`:168`), `g_jo_kw_shared` (`:199`), la sonde (`planet_family.c:67`), les dix appels d'init (`planet_family.c:84`, `:151`, `:202`, `:297`, `:378`, `:552` ; `planet_family_trade.c:208`, `:267`, `:373` ; `planet.c:175`), la copie de `qw` (`planet_family.c:152`) et les lectures par `def` (`planet_display.c:124`, `:125`, `economy_score.c:112`, `planet_trade.c:155`, `:157`) sont exécutés.

5. **Valeurs observables, parent contre enfant** : un programme d'essai (`obs.c`, hors dépôt) lié aux objets de `src/` crée une instance fraîche de chacune des dix sortes et imprime 19 lignes : `me`, `t`, `hf`, `hs`, `fa`, `get_fa`, `kq`, `get_kq`, `ma`, `ka`, `cq`, `dz`, `em`, `gq`, `ca`, `mi`, `has_mi`, `kw` (partagé ou non, `mu`, `has_qe`), `by` ; pour l'arbre `qh`, `hp`, `px`, `qh`, `s`, `ba` et les huit champs de sonde ; pour `nl`, `rr`, `nl`, `qw`. Compilé avec `-DV=0` sur `p` (lecture de l'instance), `-DV=1` sur `d1` et `-DV=2` sur `d2` (lecture par `def` de ce qui a quitté l'instance) : sorties **identiques**.

6. **§6, rejoué à `d1`** : `planet_gh_row_lq` — la copie clés → cases prend une case de trop (`pico8_lq_struct_alloc(keys->count + 1)`, boucle jusqu'à `keys->count` inclus, M3 de D.18a) → rouge (ASan : `heap-buffer-overflow`), lancé comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0`) ; restauré, vert. `ga_export_panel_by` — le constructeur de `ga` n'installe plus `g_ga_by_shared` (`self->jo.by = NULL;`, M22 de D.12b) → rouge (`by != NULL`) ; restauré, vert. Les deux tests perdent leur écriture de `jo.gq` (placement de la ligne seulement, jamais asserté : étude 2.9, §1) ; leur intention est inchangée.

7. `class-coverage-net` : **oui**, vert à `d1` et `d2` (`couverture: OK`, sortie égale à `tools/class_coverage_net_expected.txt`). `sanitize-net` : **oui** (`pico8_jo_t`, `pico8_qh_t`, `pico8_nl_t` changent de taille), `sanitize: 26 propre(s)` aux deux.

8. `game_data.h` mesuré au commit : **575** à `d1`, **536** à `d2`.

### Greps de Sortie

À `d1` :
1. `grep -rnE 'pico8_jo_start|pico8_jo_kw_start' src tests` → vide.
2. `grep -rn 'const pico8_[a-z_]*_def_t \*def;' src | wc -l` → **18** (17 de 2.13c, `planet.h`).
3. `grep -rnE 'self->def = (&pico8_[a-z_]*_def|def);' src | wc -l` → **16** (15 de 2.13c, `planet.c:138`).
4. `grep -rnE 'emitter->def = |probe->def = |\.def = ' src` → ceux de 2.13c, inchangés (`planet_family.c:67` encore NULL).
5. `grep -rnE 'probe->def->|emitter->def->|pb\.def' src` → vide.

À `d2` :
1. `grep -rnE 'pico8_(jo|qh|nl|os)_start|pico8_jo_kw_start' src tests` → vide.
2. et 3. → **18**, **16**.
4. `grep -rnE 'emitter->def = |probe->def = |\.def = ' src` → exactement : `probe.c:443` (`&self->def->emitter`), `probe.c:448`, `wormhole.c:246`, `slipway.c:403` (`&self->def->probe`), **`planet_family.c:67` (`&self->jo.def->probe`)**, `planet_family.c:273`, `:555` (émetteur, NULL), `research_icon.c:180` (`{.def = &pico8_probe_def_none}`).
5. `grep -rnE 'probe->def->|emitter->def->|pb\.def' src` → vide.

Aux deux :
6. Casts et tests de classe : `grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests | wc -l` → 532 ; `grep -rEo '(->cls|header\.cls|_struct_class\(\))' src tests | wc -l` → 707.
7. `grep -n 'g_compared ==' tests/test_pico8_archetype_chain.c` → **231**, puis **123**.
8. `make composition-residue` → pied **730 / 72**, `défauts 326 15`, puis **628 / 72**, `défauts 224 15` ; `inclusion-mutuelle 17 16` ; `--champs` → `259 ; 15 ; 20`, puis `255 ; 15 ; 20`.
9. `git diff --stat <parent>` : les fichiers de la section « Modules » et `docs/plan-3.md`, rien d'autre ; en particulier ni `tests/test_pico8_pj.c`, ni `src/pico8_research_icon.h`, ni `tests/test_pico8_jc.c`, ni `tools/`, ni `Makefile`.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`p` = `672679c`, `d1` = `9ffd506`, `d2` = `ec90d61`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout b256053`, `git apply` du diff du PRD 2.13c, commit | `p` | 0 | `672679c` | 3 s |
| 2 | `timeout 900 make -j8 test` ; `timeout 600 make e2e` | `p` | 0 | vert ; 26 conformes | 14 s ; < 30 s |
| 3 | compteur, `--champs`, `--check inclusion-mutuelle`, `wc -l`, total, casts, `--check défauts` + attribution | `p` | 0 | 807 / 73 ; 403 / 22 ; 264 / 15 / 20 ; 43 ; 593 ; 333 ; 532 / 707 ; d1 77, d2 102 | < 10 s |
| 4 | texte de d1 (scripts d'ancres) ; `make -k -j8 test` | `d1` | 2 | erreurs de compilation dans 18 TU (membres retirés, `_start` nommés) ; `src/` compile | < 1 min |
| 5 | TU réécrits ; `make -j8 test`, `make e2e`, compteur, `--champs`, `wc -l` ; commit | `d1` | 0 | vert ; 26 ; 730 / 72 ; 326 / 15 ; 259 ; 575 ; 231 | < 1 min |
| 6 | texte de d2 ; `make -k -j8 test` | `d2` | 2 | erreurs dans 15 TU ; `src/` compile | < 1 min |
| 7 | TU réécrits, `same_text_or_nil` retiré ; `make -j8 test`, `make e2e`, compteur ; commit | `d2` | 0 | vert ; 26 ; 628 / 72 ; 224 / 15 ; 255 ; 536 ; 123 | < 1 min |
| 8 | jumelles A (`mut.py`, 102) au parent | `p` | 0 | tableau, preuve 3 ; `jo.mi`, `ga.mi` : `archetype_chain` seul | ≈ 13 min |
| 9 | jumelles A à l'enfant, N1–N10 | `d1` (sans filets) | 0 | 13 vertes (7 `kq`, `jo.ka`/`dz`/`cq`/`mi`, `ga.hs`/`mi`) = ÉCHEC ; N rouges | ≈ 14 min |
| 10 | jumelles B au parent | `d1` (sans filets) | 0 | 17 à `archetype_chain` seul (`ik`, `has_ik` ×8, `pm.has_od`) | ≈ 13 min |
| 11 | jumelles B à l'enfant, N11–N12 | `d2` (sans filets) | 0 | les 17 vertes = ÉCHEC ; N rouges | ≈ 14 min |
| 12 | littéraux de d1 (`assert_jo_box`, `ka`/`dz`/`cq`, `mi`, `ga.hs`) ; `make test`, `make e2e` ; d1 refait, d2 rejoué dessus (`cherry-pick`), littéraux de sonde ; `make test`, `make e2e` | `d1`, `d2` | 0 | vert ; 26 ; `9ffd506`, `ec90d61` | 2 min |
| 13 | rejeu des 13 + 17 jumelles, E1–E9 et K aux deux bouts | `d1`, `d2`, `p` | 0 | toutes rouges (`jo_spec_defaults`) ; E identiques ; K ci-dessus | ≈ 6 min |
| 14 | `make test`, `make e2e`, `class-coverage-net`, `sanitize-net`, compteur | `d1`, `d2` | 0 | vert ; 26 ; `couverture: OK` ; `26 propre(s)` ; 730 / 72, 628 / 72 | < 1 min chacun |
| 15 | preuve `const` (2 membres) ; preuve négative | `d1`, `d2` | 0 | 1 et 4 erreurs aux poses ; `read-only object` ×2 | < 1 min |
| 16 | §6 : M3 (`gh_row_lq`, ASan), M22 (`ga_export_panel_by`) | `d1` | 0 | rouges ; restaurés, verts | < 1 min |
| 17 | `obs.c` (`-DV=0/1/2`) sur `p`, `d1`, `d2`, `diff` | les trois | 0 | 19 lignes, identiques | < 2 min |
| 18 | `make line-coverage EVENTS=<26 e2e>` | `d2` | 0 | 6279 lignes ; lignes de la preuve 4 présentes | 5 s |
| 19 | `--champs` complets, `diff` ; greps de Sortie | `p`, `d1`, `d2` | 0 | ci-dessus | < 10 s |
| 20 | `rm -rf` de la copie | — | 0 | supprimée | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit 2.13c (ou un descendant qui ne touche que `docs/`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si d1 et d2 ne sont pas deux commits, dans cet ordre, chacun vert (`make test`, `make e2e` 26 conformes), ou si un `*.sha256`, `tests/e2e/` ou une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si une jumelle de la preuve 3 est verte à l'enfant et rouge au parent (hors `archetype_chain`, une comparaison de `jo_spec_defaults` à un `_start` comptant comme rouge au parent), notamment si un des littéraux de la preuve 3 manque.
- `FAILED:` si une comparaison dont la chaîne vit encore quitte le test ponctuel (colonne « Reste à `d2` »), si une autre reste ou est réécrite vers `def`, ou si le total n'est pas 231 puis 123.
- `FAILED:` si un champ A qu'aucune sorte e n'écrit reste membre à la fin de sa sous-étape, si un champ que la famille e écrit encore (`hf`, `fa`, `kq`, `hp`, `px`, `qh`) quitte l'instance ou est lu par `def`, ou si une ligne E devient verte à `d2`.
- `FAILED:` si une sorte pose un autre archétype que le sien, si un `def` (instance, sonde) est écrit hors de l'init de son porteur, TU compris (preuve 2), ou n'est pas `const pico8_X_def_t *` ; si une pose n'est pas épinglée (N1 à N12 verts).
- `FAILED:` si `kw_own` n'est pas mis à zéro par l'init, si `g_jo_kw_shared` n'est pas posé par `pico8_jo_init`, ou si `kw`/`kw_own`/`by` entre dans un archétype.
- `FAILED:` si un lecteur lit `probe->def` ou `emitter->def` (grep 5), ou si un champ A quitte `pico8_probe_t`.
- `FAILED:` si le §6 n'est pas rejoué (preuve 6) ou si l'un des deux tests reste vert avec son bogue.
- `FAILED:` si un cast ou un test de classe est ajouté, si `inclusion-mutuelle` n'est pas 17 / 16 (43 lignes).
- `FAILED:` si `défauts` n'est pas 326 puis 224, si le pied n'est pas 730 / 72 puis 628 / 72, si `--champs` n'est pas `259 ; 15 ; 20` puis `255 ; 15 ; 20`.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge ; si les valeurs observables (preuve 5) diffèrent.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un message de commit omet : les classes, les champs A retirés et ceux gardés membres (avec leur sous-étape de sortie), les I copiés, les signatures, les poses de `def`, `kw_own`/`g_jo_kw_shared`, la table des jumelles avec leur résultat au parent, la preuve `const`, le §6, les comptes, le tableau « Runs ».
- `FAILED:` s'il manque une note « Découvert à 2.13d1 » ou « 2.13d2 », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- `hj`, `pm`, `mx`, `ga`, `nr`, `fr` (2.13e1, 2.13e2), sauf le contrat plus haut : leurs `_start`, leurs écritures des champs A gardés membres, le `def` de leur émetteur.
- La bascule des lecteurs de la sonde et de l'émetteur et le retrait de leurs champs A (2.13e2) ; `test_pico8_jc.c:853`/`:893` (2.13e2).
- `pico8_ke_start` et la lecture de `t` sur `def` (2.13e2).
- Les réserves mineures sur `test_pico8_pj.c` et `research_icon.h` (fichiers non touchés).
- Les liens du noyau (2.14, 2.15), la question Q-2.10 ; les structs de sorte (2.15d).
- `tools/composition_residue.sh` et son TU (la ligne `"pico8_" "jo_kw_start;\n"` de `test_composition_residue.c` est une fixture coupée, non comptée) ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Découpe en d1 (`jo`) et d2 (`qh`, `nl`, `os`)** : ≈ 1 h 45 d'un bloc, dont ≈ 50 min de jumelles ; la frontière suit l'attribution (77 / 102 lignes) et chaque moitié compile et passe seule.
- **G2. Chaque sorte e pose son propre archétype dès d1**, et les champs A qu'elle réécrit restent membres (copiés depuis `def`) jusqu'à sa sous-étape : c'est la seule forme où aucune écriture `_start` ne devient morte (lignes E) et où aucune jumelle enfant d'un champ lu par `def` n'est verte.
- **G3. `def` passé à l'init**, comme `pico8_g__struct_init` en 2.13c ; `pico8_nr_struct_init` (`static`) le prend aussi.
- **G4. `kw_own` mis à zéro**, pas « laissé à `lz` » : `jo` et `ga` n'ont pas d'`hp`, leur `kw_own` resterait indéterminé.
- **G5. `has_ik` gardé pour les huit sortes en d2**, pas étalé sur e1/e2 comme le disait la table du PRD 2.13a : c'est `pico8_qh_struct_init` qui écrit ce littéral pour les huit, et d2 en fait une copie depuis `def`.
- **G6. Une comparaison de `jo_spec_defaults` à un `_start` compte comme rouge au parent** : le filet qu'elle portait part avec la chaîne ; c'est ce qui a trouvé la boîte `kq` et `ka`/`dz`/`cq`/`ga.hs`.
- **G7. `entity_kind`, `map_tables`, `ui`** nomment l'archétype au lieu de `_start` (précédent G7 de 2.13c).

### Surprises de mesure, pour `plan-pilot`

1. **30 filets uniques**, dont un seul connu (`has_ik` de l'arbre `qh`, H1 de 2.12e1) : la boîte `kq` de sept sortes, `ka`/`dz`/`cq` de `jo`, `mi` de `jo` et `ga`, `hs` de `ga`, `ik`/`has_ik` des huit sortes à sonde, `has_od` de `pm`. Tous sont des valeurs mortes ou lues seulement par identité de pointeur.
2. **`ga.kq` et `nr.kq` n'ont que le test ponctuel pour filet** (E6, E8), au parent comme à `d2` : à garder par un littéral en 2.13e2.
3. **d2 ne fait sortir aucun fichier de `défauts`** : chaque fichier touché nomme encore un `_start` de la famille e.
4. **Six A de l'arbre restent membres après 2.13d** (`hf`, `fa`, `kq`, `hp`, `px`, `qh`) : ils sortiront avec la famille e.

### Questions pour l'humain

Aucune.
