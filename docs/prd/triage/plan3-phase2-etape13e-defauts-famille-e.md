---
title: "Plan 3, étape 2.13e : la famille e lit son archétype (`hj`, `pm`, `mx` ; `ga`, `nr`, `fr` ; composants ; `ke`) ; `défauts` 224 → 97 → 27 → 27 → 0, gardé — quatre commits, jeu identique"
description: "Découpée en 2.13e1 (`hj`, `pm`, `mx`), 2.13e2a (`ga`, `nr`, `fr`), 2.13e2b (bascule des lecteurs de la sonde et de l'émetteur, retrait de leurs champs A) et 2.13e2c (`pico8_ke_start` sort, `me`/`t` lus sur l'archétype, test ponctuel retiré, `défauts` gardé). Les `_start` de la famille e disparaissent ; `qh.hp`/`px`/`qh`, `hj.rr`, `pm.ng`/`fz` (e1), `jo.hf`/`fa`/`kq`, `ga.fz`, `nr.hz`/`rr`, `fr.mt` (e2a), sonde `dw`/`dq`/`fp`/`ik`/`has_ik` et émetteur `ki`/`pk`/`fc` (e2b) quittent l'instance. Essayé sur copie : vert, 26 e2e à chaque commit ; 55 + 32 + 78 + 42 jumelles, aucun ÉCHEC."
status: needs-triage
---

## Problem Statement

**Écrit sur `689d4fd`, relu sur `bde509f`** (branche `claude/plan-pilot-3-7zdft0`). Pendant l'écriture, 2.13d s'implémentait dans l'arbre de travail (fichiers non commités non lus) ; elle a depuis été commitée (`5ed4b31` d1, `df3878c` d2) et notée (`bde509f`). Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Essais sur une seule copie jetable (scratchpad `prd213e/`, supprimée depuis). Sur cette copie, `689d4fd` a reçu les deux diffs du PRD 2.13d (`git apply` sans erreur), commités `28cf5fd` (d1) et `59f7d2d` (d2) ; `git diff 59f7d2d df3878c -- src tests Makefile tools` est **vide** : l'arbre de départ est exactement le commit 2.13d2 réel. Notation : `p` = `59f7d2d` (= `df3878c` pour `src/`, `tests/`) ; `e1` = `deaaaa5`, `e2a` = `6d01a04`, `e2b` = `4a1343a`, `e2c` = `ef5b52c`, les commits de la copie qui portent les quatre diffs de « Ce qui change ». Les quatre diffs ont été rejoués sur `bde509f` (`git apply --check` puis `git apply`, `git rm` du test ponctuel avant le quatrième) : même arbre que `e2c` pour `src/`, `tests/`, `Makefile`, `tools/`.

Vu du mainteneur du port : 2.13a à 2.13d ont basculé les racines, les deux racines à sonde, la sonde et l'aide, puis la planète et la famille d. Il reste la famille e (`hj`, `pm`, `mx`, `ga`, `nr`, `fr`), dont les `_new` réécrivent encore leurs `_start` par-dessus l'archétype, les deux composants (sonde, émetteur), dont les champs A restent membres parce que la famille e les écrivait, et `pico8_ke_start`, la chaîne de `me`/`t` pour tout le monde. 2.13e les fait tomber dans cet ordre, et avec eux le test ponctuel : `défauts` passe à **0** et entre dans `COMPOSITION_RESIDUE_GUARD`.

### Texte du plan, recopié (`docs/plan-3.md` à `bde509f`)

> - 2.13 — l'init lit l'archétype, les `pico8_X_start(_t)` et la chaîne d'init disparaissent. Chaque instance reçoit `def`, posé par son `_new`. L'init copie la part I (étude, §2), et les lectures des champs A passent par `self->def`. Les champs A quittent l'instance. Les objets statiques `g_jo_kw_shared`, `g_ga_by_shared`, `g_shared_le`, `g_nf_ra`, `g_mz_jj_wrapper` restent posés par l'init. La fonction d'une famille dans le test ponctuel part dans le commit qui retire sa chaîne, après être passée à 2.12, et le TU disparaît en 2.13e2. […] Les écritures de TU sur un champ A sont réécrites. […] `g_.fa` et la sonde `dw` passent par un archétype de test pointé par `def`. Les comptes d'arrivée sont attribués par les noms `pico8_X_start` de chaque ligne du `--check`. Preuves : chemin touché (`gcov` des inits, mutations : défaut d'archétype changé → e2e ou TU rouge) ; règle 1 : un champ A qui quitte l'instance a sa preuve `const`, rejouée sur la copie du commit ; `game_data.h` mesuré au commit. `class-coverage-net` : oui. `sanitize-net` : oui. Budget ≈ 7 × 50 min.
>   - 2.13e1 — `hj`, `pm`, `mx`. Compte : → **97**.
>   - 2.13e2 — `ga`, `nr`, `fr`, puis `pico8_ke_start` (`me`/`t` lus sur `def`). Compte : → **0**, gardé.

Notes « Découvert à » qui s'adressent à 2.13e (même fichier) : 2.12c (`has_od` de `g_`, fait en 2.13c) ; 2.12e1 (`probe.has_ik` de l'arbre `qh`, fait en 2.13d2) ; 2.12e2 (jumelle C4, `emitter.pn` de `ga` : rouge au seul test ponctuel, « à noter pour 2.13e2, pas à garder ») ; 2.13a1 (`me`/`t` de la famille a, seul filet le test ponctuel tant que `pico8_ke_start` vit) ; 2.13b (bascule des lecteurs de sonde et `test_pico8_jc.c:853`/`:893` en 2.13e2) ; 2.13c (bascule des lecteurs d'émetteur en 2.13e2, `def` d'émetteur NULL chez `hj`/`ga` à remplacer en 2.13e1/e2) ; 2.13d (contrat recopié plus bas).

Étude 2.9 (`docs/etude-2-9-archetypes.md`), famille e : `hj` A `rr` ; `pm` A `fz`, `ng` ; `mx` I `ht` ; `ga` A `fz`, I `iu`, `ms`, `jo.by` (objet statique) ; `nr` A `hz`, `rr` ; `fr` A `mt` ; `qh` A `hp`, `px`, `qh`, I `s` ; `jo` A `hf`, `fa`, `kq`. Émetteur : A `ki`, `pk`, `fc` ; I `pn`, `du`, `py`, `k_`, `qo`, `ro`, `qg`, `q_` (des clés les réécrivent : `g_`, `ga`) ; E `qy`. Sonde : A `dw`, `dq`, `fp`, `ik`, `has_ik` ; I `od`, `has_od` ; E `et`. En-tête : `me`, `t` I, résolus le long de la chaîne, `ke` en tête. Étude 2.10, §2 : `def` dans le composant ; réserve 2 : les A de la sonde restent membres jusqu'en 2.13e2.

Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée.

### Contrats reçus et ce que ce PRD en fait

Recopiés des PRD 2.13a (§ « Contrat pour 2.13b à 2.13e2 »), 2.13b, 2.13c, 2.13d (§ « Contrat pour 2.13e1 et 2.13e2 »), du message de `05daecc` et des réserves du vérificateur de 2.13d (`bde509f`).

| Point (source) | Ce que fait 2.13e |
|---|---|
| `qh.hp`, `qh.px`, `qh.qh` quittent `pico8_qh_t` quand `hj`/`pm`/`mx` ne les écrivent plus ; lecteurs sur `qh->jo.def->X` ; copies de `pico8_qh_struct_init` retirées (2.13d) | **e1**. Lecteurs : `economy_score.c:49` (`qh`), `planet.c:276` (`hp`), `planet_display.c:155-156` (`px`), `:205` (`qh`) |
| `jo.hf`, `jo.fa`, `jo.kq` quittent `pico8_jo_t` quand `ga`/`nr` ne les écrivent plus ; `kq` se lit `&def->kq` (2.13d) | **e2a**. Lecteurs : `pico8_jo_get_fa`, `pico8_jo_get_kq` (`planet.c:80`, `:93`), `planet.c:326`, `economy_score.c:110`, `economy_tiers.c:279`, `particle.c:30` |
| `def` passé à l'init, déjà posé pour les six sortes : ne rien réécrire de ce chemin (2.13d) | tenu : aucune signature ne change en e1/e2a ; seules les écritures `_start` partent. `pico8_nr_struct_init` garde sa forme (elle n'a plus que l'appel à `pico8_qh_struct_init`) |
| `def` d'émetteur de `hj`/`pm`/`mx` : `&def->emitter` de la sorte, `assert` de pose, émetteur copié depuis ce `def` (2.13c) | **e1** : `self->emitter->def = &self->qh.jo.def->emitter;` dans `pico8_hj_struct_init`, les onze membres copiés ; épinglé par `assert_emitter(…, &pico8_X_def)` de `jo_spec_defaults` (N1 rouge) |
| `def` d'émetteur de `ga` ; `emitter.pn`/`du`/`ki`/`pk` de `ga` morts : l'init de `ga` cesse de les écrire, le zéro reste dans la valeur, pas de TU durable (`05daecc`, remplace la ligne « un littéral `emitter->pn == 0` » du contrat de 2.13a) | **e2a** : `&self->jo.def->emitter`, épinglé (N2 rouge) ; `pn`, `du`, `ki`, `pk` ne sont plus écrits par `pico8_ga_struct_new` (`ki`, `pk` quittent la struct en e2b ; `pn`, `du` restent, non écrits sur `ga`, lus par `g_:mm` seul) ; jumelles D7, D8, D10, D11 : rouges au seul test ponctuel au parent, vertes à l'enfant — pas un ÉCHEC (preuve 3) |
| `ga.kq` et `nr.kq` n'ont que le test ponctuel pour filet (E6, E8 de 2.13d ; `ga_start.kq.xl` signalé à `bde509f`) : épingler par un littéral en 2.13e2 | **e2a** : `assert_box(&ga->jo.def->kq, 0, 0, -1, -1)` et `assert_box(&nr->qh.jo.def->kq, -4, -4, 5, 5)` (pour `nr` et `fr`) dans `jo_spec_defaults` ; D3, D18, D25 rouges à l'enfant par eux seuls |
| bascule des lecteurs de sonde (`probe.c:168`, `:333`, `:335`, `:337`, `planet_display.c:469`) et d'émetteur (`probe.c:368`, `:369`, `particle.c:149`) ; `dw`, `dq`, `fp`, `ik`, `has_ik` quittent `pico8_probe_t`, `ki`, `pk`, `fc` quittent `pico8_emitter_t` ; copies retirées des inits (`g_`, `mz`, `jc`, `qh`, `hj`, `ga`) ; commentaires des membres `def` réécrits (2.13b, 2.13c) | **e2b**, après e2a : plus aucun `_new` n'écrit un champ A de composant. `od`/`has_od`, `pn`/`du`/`py`/`k_`/`qo`/`ro`/`qg`/`q_` restent membres (I) |
| littéraux `ik == 0`, `has_ik == 0` des sondes `g_`/`fv` (`spec_defaults`) et de l'arbre `qh` (`jo_spec_defaults`) deviennent des lectures `probe->def->…` aux mêmes littéraux (2.13c, 2.13d) | **e2b** ; F14, F15, F19, F20, F24… rouges par eux seuls, au parent comme à l'enfant |
| `test_pico8_jc.c:853`/`:893` : un archétype de test (la sonde de `pico8_g__def` à `dw` nul) pointé par `probe->def`, posé par l'init (2.13b, 2.13c) | **e2b** : `g_dw_zero` (copie de `pico8_g__def`, `probe.dw = 0`) et `probe_with_zero_dw()`, qui fait ce que fait `pico8_g__new(NULL)` (`pico8_g__struct_init`, `pico8_f_`, `pico8_ke_struct_pq`) ; `BV_HASH` inchangé |
| `pb` : `&pico8_probe_def_none`, aucun lecteur, même après 2.13e2 (2.13b) ; épingle du `def` de `pb` (demande de 2.13e) | **e2b** : rien ne lit `pb.def` (`pico8_dl_show` ne lit que `et`, `has_od`, `od`) ; N3 (`pb.def` NULL) et N4 (`&pico8_qh_def.probe`) **verts partout**, mesurés. Épingle : le texte exact au grep de Sortie 4 et l'initialiseur dans la preuve `const` (G5) |
| `me`/`t` des dix classes de la famille a, `t` de `mz`/`jc`, `me`/`t` de `g_`/`fv`, `t` des dix sortes : quand `pico8_ke_start` part, leurs comparaisons partent ; vérifier par jumelle qu'aucune ne manque de filet (`js`, `nf`, `mh`, `qb` à voir) (2.13a à 2.13d) | **e2c** : 36 jumelles G, toutes rouges à l'enfant hors test ponctuel (preuve 3) ; les huit `assert(… == pico8_ke_start.me …)` de `spec_defaults` deviennent des littéraux (`"ph"`, `0`) |
| `nf.ra` partagé : déplacer l'`assert` de `test_nf` dans `spec_defaults` avant la suppression du fichier (2.13a) | **e2c** : `test_nf_ra_is_shared` dans `test_pico8_spec_defaults.c` ; S1 rouge au seul test ponctuel au parent, rouge dans `spec_defaults` à l'enfant. Les cinq autres `assert` nus du test ponctuel (S2 à S6) ont déjà un filet durable |
| `ke` entre dans chaque archétype (demande de 2.13e) | **e2c** : `pico8_ke_struct_init(self, cls, me, t)` reçoit `me`/`t` de l'archétype de la classe (`def->me`, `def->t` ou `pico8_X_def.me`, `.t`) ; les trois réécritures `self->header.me = self->def->me` (`jo`, `mz`, `jc`) partent |
| réserves du vérificateur de 2.13d (`bde509f`) : lignes ≈ 110 colonnes des commentaires de `pico8_qh_struct_init`/`pico8_hj_struct_init` ; `pico8_nr_struct_init` ne nomme pas `def` ; le commentaire de `pico8_jo_t` dit que `kq` pointe dans `def` | e1 (les deux commentaires, réécrits à 80 colonnes et rendus vrais), e2a (`pico8_nr_struct_init` nomme `def` ; la parenthèse sur `kq` part avec le membre) |
| réserve du vérificateur de 2.11r-e (`7d7f194`) : mutation X2 du compteur qui survit à la fixture ; `tests/test_composition_residue.c:335` à 81 colonnes | **e2c** (le PRD touche la garde) : X2 reconstruite et tuée par une fixture neuve (preuve 9) ; `:335` recoupée |
| réserve : ligne de commentaire coupée court de `src/pico8_research_icon.h` (au plus tard 2.13e2) | **e2b** : recoupée (commentaire de `pico8_ie_t`) |
| réserve : assertion tautologique `ed_fixed(ju) == 1` de `tests/test_pico8_pj.c` (si le fichier est touché) | **non touché** par 2.13e (les `me`/`t` de `pj` ont déjà leurs littéraux : `test_pico8_pj.c:366`, `spec_defaults`) ; une ligne dans le message de commit de 2.13e2c la reporte |

### Question Q-2.10 (liens du noyau) : tranchée « oui » (`259fabe`) ; 2.13e n'en dépend pas

Le `def` de chaque sorte est un membre de l'instance (`pico8_jo_t`), celui de chaque composant un membre du composant (`pico8_probe_t`, `pico8_emitter_t`), que l'instance stocke (`probe_own`, `emitter_own`). Aucun n'est un lien du noyau, aucune inclusion ne change (`inclusion-mutuelle` 17 / 16, 43 lignes aux quatre commits) ; les liens de composant que 2.14/2.15 poseront viseront `&self->probe_own`/`&self->emitter_own`, à travers lesquels `def` se lit.

### État de départ attendu (le commit 2.13d2, `df3878c`, ou un descendant qui ne touche que `docs/`, comme `bde509f`)

Mesuré sur `p` :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 224 15`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 628 ligne(s), 72 fichier(s)`** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 255 ; 0 écriture : 15 ; 0 hors constructeur : 20` |
| `--check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `wc -l < src/pico8_game_data.h` | 536 |
| total asserté du test ponctuel | `g_compared == 123` |
| `make test` ; `make e2e` | vert (13 s) ; 26 conformes |
| lignes `défauts` par fichier (`git grep -cE '\bpico8_[a-z0-9_]+_start(_t)?\b'`, égal au `--check`) | `jo_spec_defaults.c` 91, `planet_family.c` 47, `planet_family_trade.c` 29, `entity_kind.c` 16, `game_data.h` 14, `spec_defaults.c` 8, `game_data.c` 7, `map_tables.c` 3, `ke.c` 2, `ui.c` 2, `ke.h`, `fr.c`, `ga.c`, `nr.c`, `pm.c` 1 chacun |
| casts / tests de classe (grep de la réserve 4 de 2.13a) | 532 / 707 |

Un départ autre : arrêt, rapporté.

## Solution

Quatre commits, chacun vert, jeu identique. Le plan découpe 2.13e en e1 et e2 ; e2 est découpée ici en **e2a**, **e2b**, **e2c** (G1).

**2.13e1 — `hj`, `pm`, `mx`** (`défauts` 224 → **97**) :

- `pico8_hj_struct_init` ne lit plus `pico8_hj_start` : l'émetteur reçoit `def = &self->qh.jo.def->emitter` et ses onze membres copiés depuis ce `def` (A compris, membres jusqu'en e2b) ; ses réécritures de `hp`, `qh`, `px`, `probe.od`, `s` partent (l'init de `qh` les copie déjà du même `def`).
- `pico8_pm_struct_new` et `pico8_mx_struct_new` n'écrivent plus rien de leur `_start` ; `mx` copie `ht` (I) depuis `def` ; le `me` de `pm` vient de la copie de l'init de `jo`.
- **Quittent l'instance** : `qh.hp`, `qh.px`, `qh.qh` (plus aucun écrivain), `hj.rr`, `pm.ng`, `pm.fz` (A). Lecteurs sur `def` ; `pico8_pm_pq` lit `self->hj.qh.jo.def->ng` par valeur. `pico8_pm_t` n'a plus que `hj`.
- `pico8_hj_start_t`, `pico8_pm_start_t`, `pico8_mx_start_t`, leurs valeurs et leurs `extern` sortent.
- TU : les plis lisent `def` ; `jo_spec_defaults` compare `hj`/`pm`/`mx` à leur `def` (`assert_emitter`, épingle du `def` d'émetteur) ; le test ponctuel perd 55 comparaisons (reste **68**).

**2.13e2a — `ga`, `nr`, `fr`** (`défauts` 97 → **27**) :

- `pico8_ga_struct_new` : émetteur `def = &self->jo.def->emitter`, copie de `fc`, `qo`, `ro`, `qg`, `q_`, `k_`, `py` (puis les clés `pq` comme avant), `iu`, `ms` depuis `def` ; `pn`, `du`, `ki`, `pk` ne sont plus écrits ; ses réécritures de `jo.hf`, `jo.kq`, `jo.fa` partent.
- `pico8_nr_struct_init` se réduit à l'appel à `pico8_qh_struct_init` ; `pico8_fr_struct_new` n'écrit plus rien de son `_start`.
- **Quittent l'instance** : `jo.hf`, `jo.fa`, `jo.kq` (plus aucun écrivain), `ga.fz`, `nr.hz`, `nr.rr`, `fr.mt`. `pico8_jo_get_kq` rend `&def->kq` ; `pico8_fr_get_mt` lit `def->mt`. `pico8_nr_t` n'a plus que `qh`, `pico8_fr_t` plus que `nr`.
- `pico8_ga_start_t`, `pico8_nr_start_t`, `pico8_fr_start_t`, leurs valeurs et leurs `extern` sortent (`pico8_ga_pq_keys_t`, les clés de `pm:pq`, reste : ce n'est pas un `_start`).
- TU : plis sur `def` ; les identités `jo->kq == &pico8_X_def.kq` deviennent l'épingle `jo->def == &pico8_X_def` ; `jo_spec_defaults` reçoit `assert_box` et les boîtes de `ga`/`nr` à littéral ; le test ponctuel perd 32 comparaisons (reste **36**, `me`/`t`).

**2.13e2b — composants** (`défauts` 27, inchangé) :

- Lecteurs sur `def` : `probe.c:168` (`dw`), `:333` (`dq`), `:335` (`fp`), `:337` (`has_ik`, `ik`), `:368` (`ki`), `:369` (`pk`), `particle.c:149` (`fc`), `planet_display.c:469` (`dw`).
- `pico8_probe_t` perd `dw`, `dq`, `fp`, `ik`, `has_ik` ; `pico8_emitter_t` perd `ki`, `pk`, `fc`. Leurs copies partent de `pico8_g__struct_init`, `pico8_mz_struct_new`, `pico8_jc_struct_new`, `pico8_qh_struct_init`, `pico8_hj_struct_init`, `pico8_ga_struct_new`.
- Commentaires : les membres `def` des deux composants ne disent plus « no generic reader may use it » ; les notes sur `dq`, `fp` et `pk` passent sur `pico8_probe_def_t`/`pico8_emitter_def_t` ; `wormhole.h`, `slipway.h`, `planet_family.h` (`ga`) ne nomment plus que les membres qui restent ; `research_icon.h` recoupé (réserve).
- TU : plis sur `probe->def`/`emitter->def` ; les comparaisons instance/`def` des champs partis sortent de `spec_defaults`/`jo_spec_defaults` (vraies par construction) ; `test_pico8_jc.c:853`/`:893` passent par `probe_with_zero_dw()`.

**2.13e2c — `ke`, test ponctuel, garde** (`défauts` 27 → **0**, gardé) :

- `pico8_ke_struct_init(self, cls, me, t)` : chacun des quatorze appelants de `src/` passe le `me`/`t` de son archétype ; `pico8_ke_start_t`, `pico8_ke_start` et leur `extern` sortent ; `jo`, `mz`, `jc` perdent leur réécriture de `header.me`.
- `tests/test_pico8_archetype_chain.c` est retiré (`git rm`), avec sa règle et sa ligne `UNIT_TESTS` du `Makefile`, après que `test_nf_ra_is_shared` a relogé son `assert` du `ra` partagé dans `spec_defaults`.
- `défauts` entre dans `COMPOSITION_RESIDUE_GUARD`.
- TU : `spec_defaults` compare `me`/`t` à `"ph"`/`0` ; `jo_spec_defaults` compare `t` à `0` ; `map_tables`, `ui` nomment `pico8_bg_def.me` ; le faux type de `test_pico8_iv.c` passe `"ph", 0`. `test_composition_residue.c` gagne la fixture qui tue X2 et recoupe `:335`.
- `game_data.h:42-44` (commentaire de `pico8_kind_rec_t`) ne nomme plus `pico8_X_start`.

Rien d'autre : ni `glossaire-noms.md` (aucun nom minifié neuf), ni `trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `test_pico8_pj.c`.

## User Stories

1. En tant que mainteneur, je veux que `hj`, `pm`, `mx`, `ga`, `nr`, `fr` lisent leurs défauts sur leur archétype, afin que les six derniers `pico8_X_start` disparaissent sans changer une valeur du jeu.
2. En tant que gardien de la règle 1, je veux que chaque champ A que plus personne n'écrit quitte l'instance, y compris ceux des composants, et que chaque `def` reste `const` et posé une seule fois par l'init de son porteur — TU compris.
3. En tant que gardien de la règle du parent, je veux qu'aucune valeur ne perde son dernier filet quand sa comparaison part du test ponctuel, ni quand un lecteur bascule sur `def`.
4. En tant que gardien du test ponctuel, je veux qu'il disparaisse avec la dernière chaîne (`pico8_ke_start`), après que ses `assert` nus utiles ont trouvé une place durable.
5. En tant que gardien du compteur, je veux `défauts` à 0 et gardé, et la mutation X2 du compteur tuée par une fixture.
6. En tant qu'auteur de 2.14, je veux des composants dont le `def` est lu par les lecteurs génériques, pour que le lien de composant du noyau (Q-2.10, « oui ») n'ait plus qu'à viser `probe_own`/`emitter_own`.

## Implementation Decisions

### Modules

- **e1** : `economy_score`, `game_data`, `particle.h` (commentaire du membre `def`), `planet` (`hp`), `planet_display`, `planet_family`, `planet_family_trade` ; TU `archetype_chain`, `entity_kind`, `fr`, `hj`, `jo_spec_defaults`, `map_tables`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `ui` ; `docs/plan-3.md`.
- **e2a** : `economy_score`, `economy_tiers`, `game_data`, `particle.c` (`fa`), `planet`, `planet_family`, `planet_family_trade` ; TU `archetype_chain`, `entity_kind`, `fr`, `ga`, `hj`, `jo`, `jo_spec_defaults`, `map_tables`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` ; `docs/plan-3.md`.
- **e2b** : `game_data.h` (commentaires), `particle`, `planet_display`, `planet_family`, `probe`, `research_icon.h` (commentaire), `slipway`, `wormhole` ; TU `fr`, `fv`, `g_`, `ga`, `hj`, `jc`, `jo_spec_defaults`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh`, `spec_defaults` ; `docs/plan-3.md`.
- **e2c** : `Makefile`, `ke`, `game_data`, et les quatorze appelants de `pico8_ke_struct_init` dans `src/` (`bg`, `mouse_reader`, `msgbox`, `planet`, `probe`, `research_icon`, `score_panel`, `scroll`, `slipway`, `ui`, `ui_button`, `wormhole`) ; TU `composition_residue`, `iv`, `jo_spec_defaults`, `map_tables`, `spec_defaults`, `ui`, `archetype_chain` (retiré) ; `docs/plan-3.md`.

### Inclusions

Aucune inclusion neuve : `ke.h` connaît déjà `pico8_fixed_t` (par `pico8_struct.h`) ; `particle.h` et `probe.h` incluent déjà `game_data.h`. Mesuré : `inclusion-mutuelle` **17 / 16, 43 lignes** aux quatre commits.

### Ce qui change

Les quatre diffs ci-dessous sont le texte exact (`git diff p e1`, `git diff e1 e2a`, `git diff e2a e2b`, puis `git rm tests/test_pico8_archetype_chain.c` et `git diff e2b e2c` sans ce fichier). Lecture, en plus de la Solution :

- **Même forme partout** : un champ A qui quitte l'instance se lit `self->…def->X` (pointeur : `&def->X` si le membre était une adresse dans le `_start`, comme `kq`, `ng`) ; un I reste membre, copié depuis `def` à l'init. Aucune signature ne change avant e2c ; e2c change la seule `pico8_ke_struct_init`.
- **`pm.ng`** : `pico8_pm_pq` ajoutait `ng != NULL ? *ng : {0,0}` ; `def->ng` est une valeur, la branche morte part.
- **Plis de présence** : `(pm->ng != NULL)` et `(jo->kq != NULL)` deviennent `fnv(h, 1u)` (`&def->X != NULL` est refusé par `-Werror=address`) : même valeur pliée, les `*_HASH` ne bougent pas.
- **Identités** : `jo->kq == &pico8_X_def.kq` (sept TU de sorte : `hj`, `jo`, `mx`, `nl`, `os`, `pm`, `qh`) devient `jo->def == &pico8_X_def` ; `nr`/`fr` : `jo->kq == &pico8_nr_start.kq` devient l'épingle du `def` de la sorte, et `jo->kq != &pico8_jo_def.kq` devient `jo->def->kq.xl != pico8_jo_def.kq.xl` ; `pm->ng == &pico8_pm_start.ng` devient le littéral `{1, 0}`.
- **`entity_kind`, `map_tables`, `ui`** nomment l'archétype (`pico8_hj_def.rr`, `pico8_pm_def.s`, `pico8_nr_def.probe.od`, `pico8_bg_def.me`…) au lieu de `_start` (précédent G7 de 2.13c). `ui.c:identity_of` compare à `pico8_bg_def.me` : les dix `"ph"` de `game_data.c` sont un seul objet (même unité de traduction), comme l'était `pico8_ke_start.me` ; vert, empreintes inchangées.
- **Filets neufs** : e2a, `assert_box` de `ga` et de `nr`/`fr` (contrat) ; e2c, `test_nf_ra_is_shared` (contrat) et les littéraux `"ph"`/`0` des huit `assert` de `spec_defaults`. e1 et e2b n'en demandent aucun (preuve 3).
- **Épingles** : e1, `assert_emitter(em, &pico8_X_def)` commence par `em->def == &def->emitter` (`hj`, `pm`, `mx`) ; e2a, `ga->emitter->def == &pico8_ga_def.emitter` ; les `def` d'instance et de sonde restent épinglés par `assert_jo_tail`/`assert_qh_tail` (2.13d).
- **Commentaires** : ceux des structs qui perdent leurs membres (`pico8_hj_t`, `pico8_pm_t`, `pico8_ga_t`, `pico8_nr_t`, `pico8_fr_t`, `pico8_mx_t`, `pico8_jo_t`, `pico8_probe_t`, `pico8_emitter_t`, `pico8_mz_t`, `pico8_jc_t`) et des inits (`qh`, `hj`, `nr`, `ke`) sont réécrits pour rester vrais (contrat, sans récit) ; aucun ne nomme un `_start` retiré.
- **Test ponctuel** : voir plus bas.

#### Diff de 2.13e1 (`git diff p e1`)

```diff
diff --git a/src/pico8_economy_score.c b/src/pico8_economy_score.c
index 681f369..bbf47d9 100644
--- a/src/pico8_economy_score.c
+++ b/src/pico8_economy_score.c
@@ -46,7 +46,7 @@ static int call_gg_truthy(pico8_struct_header_t *pt) {
 pico8_fixed_t pico8_da(pico8_struct_header_t *pt) {
     const pico8_qh_t *qh = pico8_qh_of(pt);
 
-    if (qh != NULL && qh->qh) {
+    if (qh != NULL && qh->jo.def->qh) {
         const pico8_ents_t *qh_list = pico8_cg("qh");
         return pico8_fixed_from_int(qh_list != NULL ? qh_list->count : 0);
     }
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index b620d17..3822e30 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -436,46 +436,6 @@ static const char nr_rr_text[] = "slipgates";
 static const char fr_rr_text[] = "infraspace";
 static const char mx_rr_text[] = "void synthesis";
 
-const pico8_hj_start_t pico8_hj_start = {
-    .rr = hj_rr_text,
-    .hp = 0x20000,
-    .qh = 0,
-    .px = &pico8_hj_px_row,
-    .od = 0x3c0000,
-    .s = 0xa60000,
-    .py = 0x1eb8,
-    .ki = 0x90000,
-    .pn = 0x50000,
-    .pk = 0x10000,
-    .fc = 0x8000,
-    .k_ = &pico8_hj_k__seq,
-    .du = 0x78000,
-    .qo = 0xfae0,
-    .ro = -0x11ec,
-    .qg = 0x11ea,
-    .q_ = 0xfae0,
-};
-
-const pico8_pm_start_t pico8_pm_start = {
-    .fz = pm_fz_text,
-    .hp = 0x30000,
-    .px = &pico8_pm_px_row,
-    .me = pm_me_text,
-    .s = 0x620000,
-    .py = 0xf5c,
-    .ki = 0x90000,
-    .pn = 0x80000,
-    .pk = 0,
-    .ng = {0x10000, 0x0},
-    .du = 0x20000,
-    .fc = 0x10000,
-    .k_ = &pico8_pm_k__seq,
-    .qo = 0x1051e,
-    .ro = -0xd70,
-    .qg = 0xd70,
-    .q_ = 0x1051e,
-};
-
 const pico8_ga_start_t pico8_ga_start = {
     .fz = ga_fz_text,
     .hf = 0,
@@ -521,26 +481,6 @@ const pico8_fr_start_t pico8_fr_start = {
     .hz = &pico8_fr_hz_seq,
 };
 
-const pico8_mx_start_t pico8_mx_start = {
-    .rr = mx_rr_text,
-    .hp = 0x50000,
-    .s = 0x420000,
-    .px = &pico8_mx_px_row,
-    .od = 0x280000,
-    .ht = &pico8_mx_ht_seq,
-    .py = 0x1eb8,
-    .ki = 0x90000,
-    .pn = 0x50000,
-    .pk = 0x10000,
-    .fc = 0x4ccc,
-    .k_ = &pico8_mx_k__seq,
-    .du = 0x50000,
-    .qo = 0xfae0,
-    .ro = -0x11ec,
-    .qg = 0x11ea,
-    .q_ = 0xfae0,
-};
-
 const pico8_jo_def_t pico8_jo_def = {
     .me = jo_me_text,
     .t = 0x0,
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index b4d0b0a..cb582e5 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -307,46 +307,6 @@ typedef struct {
 
 /* The jo tree: each class's own keys only, the inherited ones are its parent's;
  * ga.by, an empty state list, is held by its module. */
-typedef struct {
-    const char *rr;
-    pico8_fixed_t hp;
-    int qh;
-    const pico8_jb_row_t *px;
-    pico8_fixed_t od;
-    pico8_fixed_t s;
-    pico8_fixed_t py;
-    pico8_fixed_t ki;
-    pico8_fixed_t pn;
-    pico8_fixed_t pk;
-    pico8_fixed_t fc;
-    const pico8_int_seq_t *k_;
-    pico8_fixed_t du;
-    pico8_fixed_t qo;
-    pico8_fixed_t ro;
-    pico8_fixed_t qg;
-    pico8_fixed_t q_;
-} pico8_hj_start_t;
-
-typedef struct {
-    const char *fz;
-    pico8_fixed_t hp;
-    const pico8_jb_row_t *px;
-    const char *me;
-    pico8_fixed_t s;
-    pico8_fixed_t py;
-    pico8_fixed_t ki;
-    pico8_fixed_t pn;
-    pico8_fixed_t pk;
-    pico8_vec_t ng;
-    pico8_fixed_t du;
-    pico8_fixed_t fc;
-    const pico8_int_seq_t *k_;
-    pico8_fixed_t qo;
-    pico8_fixed_t ro;
-    pico8_fixed_t qg;
-    pico8_fixed_t q_;
-} pico8_pm_start_t;
-
 typedef struct {
     const char *fz;
     int hf;
@@ -393,26 +353,6 @@ typedef struct {
     const pico8_int_seq_t *hz;
 } pico8_fr_start_t;
 
-typedef struct {
-    const char *rr;
-    pico8_fixed_t hp;
-    pico8_fixed_t s;
-    const pico8_jb_row_t *px;
-    pico8_fixed_t od;
-    const pico8_int_seq_t *ht;
-    pico8_fixed_t py;
-    pico8_fixed_t ki;
-    pico8_fixed_t pn;
-    pico8_fixed_t pk;
-    pico8_fixed_t fc;
-    const pico8_int_seq_t *k_;
-    pico8_fixed_t du;
-    pico8_fixed_t qo;
-    pico8_fixed_t ro;
-    pico8_fixed_t qg;
-    pico8_fixed_t q_;
-} pico8_mx_start_t;
-
 extern const int pico8_es_table[PICO8_ES_COUNT];
 extern const pico8_fixed_t pico8_ct_table[PICO8_CT_COUNT];
 extern const int pico8_b__table[PICO8_B__COUNT];
@@ -470,13 +410,10 @@ extern const pico8_jb_row_t pico8_pj_ou_row;
 /* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
  * nf.r are empty tables, held by their module. */
 extern const pico8_ke_start_t pico8_ke_start;
-extern const pico8_hj_start_t pico8_hj_start;
-extern const pico8_pm_start_t pico8_pm_start;
 extern const pico8_ga_start_t pico8_ga_start;
 extern const pico8_ga_pq_keys_t pico8_pm_ga_pq_keys;
 extern const pico8_nr_start_t pico8_nr_start;
 extern const pico8_fr_start_t pico8_fr_start;
-extern const pico8_mx_start_t pico8_mx_start;
 extern const pico8_bg_def_t pico8_bg_def;
 extern const pico8_ui_def_t pico8_ui_def;
 extern const pico8_js_def_t pico8_js_def;
diff --git a/src/pico8_particle.h b/src/pico8_particle.h
index 5f1a783..a423936 100644
--- a/src/pico8_particle.h
+++ b/src/pico8_particle.h
@@ -29,7 +29,7 @@ typedef struct pico8_qy {
  * `pn`, `du`, `ki`, `pk` are read by `g_:mm` only: a `ga` holds 0 there.
  * `pk` is the cart's 1 or false, false held as 0: only its truth is read. */
 typedef struct pico8_emitter {
-    const pico8_emitter_def_t *def; /* NULL on the jo tree: no generic reader may use it */
+    const pico8_emitter_def_t *def; /* NULL on ga: no generic reader may use it */
     pico8_fixed_t pn;
     pico8_fixed_t du;
     pico8_fixed_t py;
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 4286cea..b3aaace 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -273,7 +273,7 @@ void pico8_jo_pq(pico8_jo_t *self) {
 
     qh = pico8_qh_of(&self->header);
     if (qh != NULL) {
-        pico8_hp_t *target = pico8_gb_at(pico8_fixed_flr_int(qh->hp));
+        pico8_hp_t *target = pico8_gb_at(pico8_fixed_flr_int(qh->jo.def->hp));
 
         if (target != NULL) {
             pico8_jo_lz(self, target);
diff --git a/src/pico8_planet_display.c b/src/pico8_planet_display.c
index 39b06b7..0516c31 100644
--- a/src/pico8_planet_display.c
+++ b/src/pico8_planet_display.c
@@ -152,8 +152,8 @@ void pico8_jo_gh(const pico8_jo_t *self) {
     memset(bs, 0, sizeof bs);
     memset(row_keys, 0, sizeof row_keys);
     qh = pico8_qh_of(&self->header);
-    if (qh != NULL && qh->px != NULL) {
-        icon_row = qh->px;
+    if (qh != NULL && qh->jo.def->px != NULL) {
+        icon_row = qh->jo.def->px;
     } else if (self->has_mi) {
         int mi = pico8_fixed_flr_int(self->mi);
 
@@ -202,7 +202,7 @@ void pico8_jo_gh(const pico8_jo_t *self) {
         qe_is_r_w = kw != NULL && kw->has_qe && pico8_r_lookup("w", &r_w) && kw->qe == pico8_fixed_from_int(r_w);
 
         if (mi_fixed < pico8_fixed_from_int(4) && jy != NULL && jy->count > 0 && !qe_is_r_w) {
-            int last_only = qh != NULL && qh->qh;
+            int last_only = qh != NULL && qh->jo.def->qh;
 
             row_cap[n] = 1 + (last_only ? 1 : jy->count) + 1;
             row_items[n] = jo_gh_row_items(row_cap[n]);
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index 8fef042..12c2a70 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -61,12 +61,9 @@ static const pico8_struct_class_t g_qh_struct_class = {
 
 void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     pico8_jo_struct_init(&self->jo, cls, def);
-    self->hp = self->jo.def->hp;
-    self->px = self->jo.def->px;
     self->probe = &self->probe_own;
     self->probe->def = &self->jo.def->probe;
     self->probe->dq = self->jo.def->probe.dq;
-    self->qh = self->jo.def->qh;
     self->probe->od = self->jo.def->probe.od;
     self->probe->has_od = self->jo.def->probe.has_od;
     self->probe->fp = self->jo.def->probe.fp;
@@ -268,26 +265,20 @@ static const pico8_struct_class_t g_hj_struct_class = {
 
 void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     pico8_qh_struct_init(&self->qh, cls, def);
-    self->rr = pico8_hj_start.rr;
     self->emitter = &self->emitter_own;
-    self->emitter->def = NULL;
-    self->emitter->py = pico8_hj_start.py;
-    self->emitter->ki = pico8_hj_start.ki;
-    self->emitter->pn = pico8_hj_start.pn;
-    self->emitter->pk = pico8_hj_start.pk;
-    self->emitter->fc = pico8_hj_start.fc;
-    self->emitter->k_ = pico8_hj_start.k_;
-    self->emitter->du = pico8_hj_start.du;
-    self->emitter->qo = pico8_hj_start.qo;
-    self->emitter->ro = pico8_hj_start.ro;
-    self->emitter->qg = pico8_hj_start.qg;
-    self->emitter->q_ = pico8_hj_start.q_;
+    self->emitter->def = &self->qh.jo.def->emitter;
+    self->emitter->py = self->qh.jo.def->emitter.py;
+    self->emitter->ki = self->qh.jo.def->emitter.ki;
+    self->emitter->pn = self->qh.jo.def->emitter.pn;
+    self->emitter->pk = self->qh.jo.def->emitter.pk;
+    self->emitter->fc = self->qh.jo.def->emitter.fc;
+    self->emitter->k_ = self->qh.jo.def->emitter.k_;
+    self->emitter->du = self->qh.jo.def->emitter.du;
+    self->emitter->qo = self->qh.jo.def->emitter.qo;
+    self->emitter->ro = self->qh.jo.def->emitter.ro;
+    self->emitter->qg = self->qh.jo.def->emitter.qg;
+    self->emitter->q_ = self->qh.jo.def->emitter.q_;
     self->emitter->qy = NULL;
-    self->qh.hp = pico8_hj_start.hp;
-    self->qh.qh = pico8_hj_start.qh;
-    self->qh.px = pico8_hj_start.px;
-    self->qh.probe->od = pico8_hj_start.od;
-    self->qh.s = pico8_hj_start.s;
 }
 
 static pico8_hj_t *pico8_hj_struct_new(const pico8_jo_keys_t *keys) {
@@ -318,16 +309,14 @@ void pico8_pm_pq(pico8_pm_t *self) {
     pico8_ga_keys_t keys;
     pico8_ga_t *spawned;
     const pico8_vec_t *ra;
-    const pico8_vec_t *ng;
 
     if (self == NULL) {
         return;
     }
 
     ra = self->hj.qh.jo.ra;
-    ng = self->ng;
     spawn_ra = pico8_vec_add(ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)),
-                             ng != NULL ? *ng : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)));
+                             self->hj.qh.jo.def->ng);
     spawn_ra_heap = malloc(sizeof *spawn_ra_heap);
     *spawn_ra_heap = spawn_ra;
 
@@ -376,23 +365,6 @@ static pico8_pm_t *pico8_pm_struct_new(const pico8_jo_keys_t *keys) {
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
     pico8_hj_struct_init(&self->hj, &g_pm_struct_class, &pico8_pm_def);
-    self->ng = &pico8_pm_start.ng;
-    self->fz = pico8_pm_start.fz;
-    self->hj.emitter->py = pico8_pm_start.py;
-    self->hj.emitter->ki = pico8_pm_start.ki;
-    self->hj.emitter->pn = pico8_pm_start.pn;
-    self->hj.emitter->pk = pico8_pm_start.pk;
-    self->hj.emitter->du = pico8_pm_start.du;
-    self->hj.emitter->fc = pico8_pm_start.fc;
-    self->hj.emitter->k_ = pico8_pm_start.k_;
-    self->hj.emitter->qo = pico8_pm_start.qo;
-    self->hj.emitter->ro = pico8_pm_start.ro;
-    self->hj.emitter->qg = pico8_pm_start.qg;
-    self->hj.emitter->q_ = pico8_pm_start.q_;
-    self->hj.qh.hp = pico8_pm_start.hp;
-    self->hj.qh.px = pico8_pm_start.px;
-    self->hj.qh.s = pico8_pm_start.s;
-    self->hj.qh.jo.header.me = pico8_pm_start.me;
     if (keys != NULL) {
         if (keys->p != NULL) {
             self->hj.qh.jo.p = keys->p;
diff --git a/src/pico8_planet_family.h b/src/pico8_planet_family.h
index e0b3538..5312ee7 100644
--- a/src/pico8_planet_family.h
+++ b/src/pico8_planet_family.h
@@ -28,9 +28,6 @@ void pico8_planet_family_shutdown(void);
  * struct: TROU-7. */
 typedef struct {
     pico8_jo_t jo;
-    pico8_fixed_t hp;
-    const pico8_jb_row_t *px;
-    int qh;
     pico8_fixed_t s;
     pico8_probe_t *probe;
     pico8_probe_t probe_own;
@@ -52,15 +49,12 @@ typedef struct {
     pico8_qh_t qh;
 } pico8_os_t;
 
-/* An ascension: a laboratory, then `rr` and hj's own emitter defaults,
- * in the emitter with `qy`, which its own `pq` (`f_`) writes. A default of
- * hj whose name is a member of `qh` or `jo` (`hp`, `qh`, `px`, `od`,
- * `s`) lives in that member. `px`, the emitter's `k_` and `rr` point
- * at hj's generated data. `emitter` points at `emitter_own`:
- * never copy the struct. Nothing frees the struct: TROU-7. */
+/* An ascension: a laboratory, then an emitter copied from its kind's
+ * archetype, with `qy`, which its own `pq` (`f_`) writes. `emitter` points
+ * at `emitter_own`: never copy the struct. Nothing frees the struct:
+ * TROU-7. */
 typedef struct {
     pico8_qh_t qh;
-    const char *rr;
     pico8_emitter_t *emitter;
     pico8_emitter_t emitter_own;
 } pico8_hj_t;
@@ -68,16 +62,10 @@ typedef struct {
 /* The `emitter_of` of hj's descriptor, for `mx`'s to name (`pm`'s too). */
 pico8_emitter_t *pico8_hj_emitter_of(void *self);
 
-/* A protostar: an ascension, then pm's own defaults `ng` and `fz`, which no
- * ancestor has a member for. A default of pm whose name is a member of
- * `hj`, `qh`, `jo` or `ke` (`me`, `hp`, `px`, `s` and the 11 emitter
- * defaults) lives in that member. `px`, `k_`, `ng` and `fz` point
- * at pm's generated data; `qy` is the protostar's own.
- * Nothing frees the struct: TROU-7. */
+/* A protostar: an ascension and nothing of its own; its defaults are its
+ * archetype's. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_hj_t hj;
-    const pico8_vec_t *ng;
-    const char *fz;
 } pico8_pm_t;
 
 /* A particle burst: a bare planet, then ga's own defaults, those of the
@@ -105,8 +93,9 @@ const pico8_entity_kind_t *pico8_qh_kind(void);
 const pico8_struct_class_t *pico8_qh_struct_class(void);
 
 /* Initializes an allocated pico8_qh_t under `cls` and `def`, its kind's
- * archetype: jo's part, qh's members nil, qh's defaults. Reads no key, runs neither `pq` nor `lt`, records no
- * coverage. Requires `pico8_planet_family_init()`. */
+ * archetype: jo's part, then `s` and the probe copied from `def`. Reads no
+ * key, runs neither `pq` nor `lt`, records no coverage. Requires
+ * `pico8_planet_family_init()`. */
 void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);
 
 /* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
@@ -150,8 +139,9 @@ const pico8_struct_class_t *pico8_hj_struct_class(void);
 pico8_hj_t *pico8_hj_new(const pico8_jo_keys_t *keys);
 
 /* Initializes an allocated pico8_hj_t under `cls` and `def`, its kind's
- * archetype: qh's part, hj's members nil, hj's defaults. Reads no key, runs neither `pq` nor `lt`, records no
- * coverage. Requires `pico8_planet_family_init()`. */
+ * archetype: qh's part, then the emitter copied from `def`, `qy` nil. Reads
+ * no key, runs neither `pq` nor `lt`, records no coverage. Requires
+ * `pico8_planet_family_init()`. */
 void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);
 
 /* The dispatchers hj's descriptor holds, for a daughter's `static const`
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index 67a7625..cf3a4f1 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -371,23 +371,7 @@ static pico8_mx_t *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
     pico8_hj_struct_init(&self->hj, &g_mx_struct_class, &pico8_mx_def);
-    self->ht = pico8_mx_start.ht;
-    self->hj.rr = pico8_mx_start.rr;
-    self->hj.emitter->py = pico8_mx_start.py;
-    self->hj.emitter->ki = pico8_mx_start.ki;
-    self->hj.emitter->pn = pico8_mx_start.pn;
-    self->hj.emitter->pk = pico8_mx_start.pk;
-    self->hj.emitter->fc = pico8_mx_start.fc;
-    self->hj.emitter->k_ = pico8_mx_start.k_;
-    self->hj.emitter->du = pico8_mx_start.du;
-    self->hj.emitter->qo = pico8_mx_start.qo;
-    self->hj.emitter->ro = pico8_mx_start.ro;
-    self->hj.emitter->qg = pico8_mx_start.qg;
-    self->hj.emitter->q_ = pico8_mx_start.q_;
-    self->hj.qh.hp = pico8_mx_start.hp;
-    self->hj.qh.s = pico8_mx_start.s;
-    self->hj.qh.px = pico8_mx_start.px;
-    self->hj.qh.probe->od = pico8_mx_start.od;
+    self->ht = self->hj.qh.jo.def->ht;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "mx: the only construction key is id (ie:ldown); another key has no member "
diff --git a/src/pico8_planet_family_trade.h b/src/pico8_planet_family_trade.h
index bddc7cd..53ed862 100644
--- a/src/pico8_planet_family_trade.h
+++ b/src/pico8_planet_family_trade.h
@@ -35,12 +35,9 @@ typedef struct {
     pico8_fixed_t mt;
 } pico8_fr_t;
 
-/* A synthesizer: an ascension, then mx's own default `ht`, which no
- * ancestor has a member for. A default of mx whose name is a member of
- * `hj`, `qh` or `jo` (`rr`, `hp`, `s`, `px`, `od` and the 11 emitter
- * defaults) lives in that member. `px`, `ht` and `k_` point at mx's
- * generated data; `mx:kf` points `ht` at an empty sequence
- * once it accepts a resource. Nothing frees the struct: TROU-7. */
+/* A synthesizer: an ascension, then `ht`, copied from its archetype;
+ * `mx:kf` points it at an empty sequence once it accepts a resource.
+ * Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_hj_t hj;
     const pico8_int_seq_t *ht;
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index 1a17839..2ef913e 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -56,10 +56,6 @@ static int same_text(const char *got, const char *want) {
     return got != NULL && want != NULL && strcmp(got, want) == 0;
 }
 
-static int same_vec(const pico8_vec_t *got, const pico8_vec_t *want) {
-    return got != NULL && got->x == want->x && got->y == want->y;
-}
-
 static int same_box(const pico8_qz_box_t *got, const pico8_qz_box_t *want) {
     return got != NULL && got->xl == want->xl && got->yt == want->yt && got->xr == want->xr && got->yb == want->yb;
 }
@@ -217,17 +213,6 @@ static void test_os(void) {
     assert_lz_ran(&os->qh.jo);
 }
 
-/* What hj's init still writes on qh's part, for hj, pm and mx. */
-static void same_hj_part(const pico8_qh_t *qh, const pico8_jo_def_t *def, const char *kind) {
-    same_in(qh->jo.header.t == def->t, kind, "t");
-    same_in(qh->hp == def->hp, kind, "hp");
-    same_in(qh->px == def->px, kind, "px");
-    same_in(qh->qh == def->qh, kind, "qh");
-    same_in(qh->s == def->s, kind, "s");
-    same_in(qh->probe->od == def->probe.od, kind, "probe.od");
-    assert(qh->probe->et == NULL && qh->jo.by == NULL);
-}
-
 /* What nr's init still writes, for nr and fr. */
 static void same_nr_part(const pico8_qh_t *qh, const pico8_jo_def_t *def, const char *kind) {
     same_in(qh->jo.header.t == def->t, kind, "t");
@@ -254,13 +239,11 @@ static void same_emitter_part(const pico8_emitter_t *emitter, const pico8_jo_def
     same_in(emitter->q_ == def->emitter.q_, kind, "emitter.q_");
 }
 
-/* `qy` is the fresh particle list `f_` gives every emitter. */
 static void test_hj(void) {
     const pico8_hj_t *hj = pico8_hj_new(NULL);
 
-    same_hj_part(&hj->qh, &pico8_hj_def, "hj");
-    same_in(same_text(hj->rr, pico8_hj_def.rr), "hj", "rr");
-    same_emitter_part(hj->emitter, &pico8_hj_def, "hj");
+    same_in(hj->qh.jo.header.t == pico8_hj_def.t, "hj", "t");
+    assert(hj->qh.probe->et == NULL && hj->qh.jo.by == NULL);
     assert(hj->emitter->qy != NULL && hj->emitter->qy->count == 0);
     assert_lz_ran(&hj->qh.jo);
 }
@@ -268,12 +251,8 @@ static void test_hj(void) {
 static void test_pm(void) {
     const pico8_pm_t *pm = pico8_pm_new(NULL);
 
-    same_in(same_text(pm->hj.qh.jo.header.me, pico8_pm_def.me), "pm", "me");
-    same_hj_part(&pm->hj.qh, &pico8_pm_def, "pm");
-    same_in(same_text(pm->hj.rr, pico8_pm_def.rr), "pm", "rr");
-    same_in(same_vec(pm->ng, &pico8_pm_def.ng), "pm", "ng");
-    same_in(same_text(pm->fz, pico8_pm_def.fz), "pm", "fz");
-    same_emitter_part(pm->hj.emitter, &pico8_pm_def, "pm");
+    same_in(pm->hj.qh.jo.header.t == pico8_pm_def.t, "pm", "t");
+    assert(pm->hj.qh.probe->et == NULL && pm->hj.qh.jo.by == NULL);
     assert(pm->hj.emitter->qy != NULL && pm->hj.emitter->qy->count == 0);
     assert_lz_ran(&pm->hj.qh.jo);
 }
@@ -281,10 +260,8 @@ static void test_pm(void) {
 static void test_mx(void) {
     const pico8_mx_t *mx = pico8_mx_new(NULL);
 
-    same_hj_part(&mx->hj.qh, &pico8_mx_def, "mx");
-    same_in(same_text(mx->hj.rr, pico8_mx_def.rr), "mx", "rr");
-    same_in(mx->ht == pico8_mx_def.ht, "mx", "ht");
-    same_emitter_part(mx->hj.emitter, &pico8_mx_def, "mx");
+    same_in(mx->hj.qh.jo.header.t == pico8_mx_def.t, "mx", "t");
+    assert(mx->hj.qh.probe->et == NULL && mx->hj.qh.jo.by == NULL);
     assert(mx->hj.emitter->qy != NULL && mx->hj.emitter->qy->count == 0);
     assert_lz_ran(&mx->hj.qh.jo);
 }
@@ -364,7 +341,7 @@ int main(void) {
     test_ga();
     test_nr();
     test_fr();
-    assert(g_compared == 123 && "families a to e compare 123 fields: one missed or one too many");
+    assert(g_compared == 68 && "families a to e compare 68 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_entity_kind.c b/tests/test_pico8_entity_kind.c
index fdd8585..11c8927 100644
--- a/tests/test_pico8_entity_kind.c
+++ b/tests/test_pico8_entity_kind.c
@@ -189,22 +189,22 @@ static void test_kind_reader_serves_the_generated_defaults(void) {
     assert(rec->has_od && rec->od == pico8_nl_def.probe.od);
 
     rec = rec_of(5);
-    assert(rec->rr == pico8_mx_start.rr);
-    assert(rec->has_s && rec->s == pico8_mx_start.s);
+    assert(rec->rr == pico8_mx_def.rr);
+    assert(rec->has_s && rec->s == pico8_mx_def.s);
     assert(rec->me == pico8_qh_def.me);
-    assert(rec->has_od && rec->od == pico8_mx_start.od);
+    assert(rec->has_od && rec->od == pico8_mx_def.probe.od);
 
     rec = rec_of(6);
-    assert(rec->rr == pico8_hj_start.rr);
-    assert(rec->has_s && rec->s == pico8_hj_start.s);
+    assert(rec->rr == pico8_hj_def.rr);
+    assert(rec->has_s && rec->s == pico8_hj_def.s);
     assert(rec->me == pico8_qh_def.me);
-    assert(rec->has_od && rec->od == pico8_hj_start.od);
+    assert(rec->has_od && rec->od == pico8_hj_def.probe.od);
 
     rec = rec_of(7);
-    assert(rec->rr == pico8_hj_start.rr);
-    assert(rec->has_s && rec->s == pico8_pm_start.s);
-    assert(rec->me == pico8_pm_start.me);
-    assert(rec->has_od && rec->od == pico8_hj_start.od);
+    assert(rec->rr == pico8_pm_def.rr);
+    assert(rec->has_s && rec->s == pico8_pm_def.s);
+    assert(rec->me == pico8_pm_def.me);
+    assert(rec->has_od && rec->od == pico8_pm_def.probe.od);
 
     rec = rec_of(8);
     assert(rec->rr == NULL);
diff --git a/tests/test_pico8_fr.c b/tests/test_pico8_fr.c
index d699b8d..4b84178 100644
--- a/tests/test_pico8_fr.c
+++ b/tests/test_pico8_fr.c
@@ -150,7 +150,7 @@ static pico8_qh_t *as_qh(void *e) {
 }
 
 static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
-    return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return qh->jo.def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static void check(const char *label, uint32_t got, uint32_t want) {
@@ -257,7 +257,7 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fold_fixed_member(h, jo->header.t);
     h = fold_string_member(h, nr->rr);
     h = fold_fixed_member(h, fr->mt);
-    h = fold_fixed_member(h, qh->hp);
+    h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, qh->probe->fp);
@@ -279,7 +279,7 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fnv(h, (uint32_t)(nr->hz != NULL));
     h = fnv(h, (uint32_t)(nr->rr != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->px != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -338,7 +338,7 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     assert(nr->hz != &pico8_nr_hz_seq);
     assert(jo->kq == &pico8_nr_start.kq);
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
-    assert(qh->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
+    assert(qh->jo.def->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->def == &pico8_fr_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
diff --git a/tests/test_pico8_hj.c b/tests/test_pico8_hj.c
index 2ce77ac..f51913f 100644
--- a/tests/test_pico8_hj.c
+++ b/tests/test_pico8_hj.c
@@ -161,7 +161,7 @@ static uint32_t fold_kw(uint32_t h, const pico8_jo_t *jo) {
 }
 
 static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
-    return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return qh->jo.def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 /* 1 no target, 3 a target: the bytes the frozen hashes fold. */
@@ -292,7 +292,7 @@ static void *build_ascension(void) {
  * row `px` is one of that kind. */
 static int is_of(const void *e, const pico8_jb_row_t *px) {
     const pico8_qh_t *qh = pico8_qh_of(e);
-    return qh != NULL && qh->px == px;
+    return qh != NULL && qh->jo.def->px == px;
 }
 
 /* An ascension as `ie:ldown`, `f_` and `jo:pq` left it: its scalars (the 12
@@ -310,9 +310,9 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fold_string_member(h, jo->id);
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
-    h = fold_string_member(h, hj->rr);
+    h = fold_string_member(h, hj->qh.jo.def->rr);
     h = fold_qh(h, qh);
-    h = fold_fixed_member(h, qh->hp);
+    h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, em->py);
@@ -338,7 +338,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     /* The 1s of the ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
      * `dw`, `s`, then of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the
      * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
-    h = fnv(h, (uint32_t)(hj->rr != NULL));
+    h = fnv(h, (uint32_t)(hj->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -351,7 +351,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->px != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -395,9 +395,9 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     assert_qh_members_but_kj(cls_of(e));
     assert(cls_of(e)->kj == pico8_hj_struct_class()->kj && pico8_jo_is_family(e));
     assert(cls_of(e)->mm == pico8_g__mm);
-    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->px == &pico8_hj_px_row && em->k_ == &pico8_hj_k__seq);
+    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->jo.def->px == &pico8_hj_px_row && em->k_ == &pico8_hj_k__seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
-    assert(qh->px != &pico8_qh_px_row);
+    assert(qh->jo.def->px != &pico8_qh_px_row);
     assert(qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_hj_def.kq);
     assert(jo->def == &pico8_hj_def);
@@ -558,14 +558,14 @@ static uint32_t fold_heir(uint32_t h, void *e, const pico8_struct_class_t *cls)
     assert(cls_of(e)->kj == pico8_hj_struct_class()->kj);
     assert(cls_of(e)->mm == pico8_hj_struct_class()->mm);
     assert(ptrs_size(hj->emitter->qy) == 0);
-    assert(hj->qh.qh == 0);
+    assert(hj->qh.jo.def->qh == 0);
     h = fold_string_member(h, hj->qh.jo.id);
     h = fold_string_member(h, hj->qh.jo.header.me);
     h = fold_qh(h, &hj->qh);
-    h = fold_string_member(h, hj->rr);
+    h = fold_string_member(h, hj->qh.jo.def->rr);
     h = fold_fixed_member(h, hj->qh.probe->od);
     h = fold_pk(h, hj);
-    h = fold_fixed_member(h, hj->qh.hp);
+    h = fold_fixed_member(h, hj->qh.jo.def->hp);
     h = fold_fixed_member(h, hj->qh.s);
     h = fold_fixed_member(h, hj->emitter->py);
     h = fold_fixed_member(h, hj->emitter->pn);
@@ -587,9 +587,9 @@ static void test_heirs(void) {
     keys.id = "s77";
     pm = pico8_pm_new(&keys);
 
-    assert(strcmp(as_hj(pm)->rr, "ascension") == 0 && as_hj(pm)->emitter->pk == 0);
+    assert(strcmp(as_hj(pm)->qh.jo.def->rr, "ascension") == 0 && as_hj(pm)->emitter->pk == 0);
     assert(as_hj(pm)->qh.probe->od == pico8_fixed_from_int(60));
-    assert(strcmp(as_hj(mx)->rr, "void synthesis") == 0);
+    assert(strcmp(as_hj(mx)->qh.jo.def->rr, "void synthesis") == 0);
     h = fold_heir(h, pm, pico8_pm_struct_class());
     h = fold_heir(h, mx, pico8_mx_struct_class());
     check("heirs", h, HEIRS_HASH);
diff --git a/tests/test_pico8_jo_spec_defaults.c b/tests/test_pico8_jo_spec_defaults.c
index 1a08288..0e039f5 100644
--- a/tests/test_pico8_jo_spec_defaults.c
+++ b/tests/test_pico8_jo_spec_defaults.c
@@ -31,10 +31,6 @@ static void assert_same_string(const char *a, const char *b) {
     assert(a != NULL && a == b && strcmp(a, b) == 0);
 }
 
-static void assert_same_vec(const pico8_vec_t *a, const pico8_vec_t *b) {
-    assert(a != NULL && a == b);
-}
-
 static void assert_same_box(const pico8_qz_box_t *a, const pico8_qz_box_t *b) {
     assert(a != NULL && a == b);
 }
@@ -64,6 +60,15 @@ static void assert_qh_tail(const pico8_qh_t *qh, const pico8_jo_def_t *def) {
     assert(qh->probe->has_od == 1 && qh->probe->ik == 0 && qh->probe->has_ik == 0);
 }
 
+/* The emitter of an ascension, pm or mx, and its archetype. */
+static void assert_emitter(const pico8_emitter_t *em, const pico8_jo_def_t *def) {
+    assert(em->def == &def->emitter);
+    assert(em->py == def->emitter.py && em->ki == def->emitter.ki && em->pn == def->emitter.pn);
+    assert(em->pk == def->emitter.pk && em->fc == def->emitter.fc && em->k_ == def->emitter.k_);
+    assert(em->du == def->emitter.du && em->qo == def->emitter.qo && em->ro == def->emitter.ro);
+    assert(em->qg == def->emitter.qg && em->q_ == def->emitter.q_);
+}
+
 static void test_jo_defaults(void) {
     pico8_jo_t *jo = pico8_jo_new(NULL);
 
@@ -85,10 +90,7 @@ static void test_qh_defaults(void) {
     pico8_qh_t *qh = pico8_qh_new(NULL);
 
     assert(pico8_qh_ready());
-    assert(qh->hp == pico8_qh_def.hp);
-    assert(qh->px == pico8_qh_def.px);
     assert_same_string(qh->jo.header.me, pico8_qh_def.me);
-    assert(qh->qh == pico8_qh_def.qh);
     assert(qh->probe->od == pico8_qh_def.probe.od);
     assert(qh->probe->fp == pico8_qh_def.probe.fp);
     assert(qh->probe->dw == pico8_qh_def.probe.dw);
@@ -104,10 +106,7 @@ static void test_nl_defaults(void) {
     pico8_nl_t *nl = pico8_nl_new(NULL);
 
     assert(pico8_nl_ready());
-    assert(nl->qh.qh == pico8_nl_def.qh);
-    assert(nl->qh.hp == pico8_nl_def.hp);
     assert(nl->qh.s == pico8_nl_def.s);
-    assert(nl->qh.px == pico8_nl_def.px);
     assert(nl->qh.probe->od == pico8_nl_def.probe.od);
     assert(nl->qw == pico8_nl_def.qw);
     assert_same_string(nl->qh.jo.header.me, pico8_nl_def.me);
@@ -124,9 +123,6 @@ static void test_os_defaults(void) {
     pico8_os_t *os = pico8_os_new(NULL);
 
     assert(pico8_os_ready());
-    assert(os->qh.hp == pico8_os_def.hp);
-    assert(os->qh.px == pico8_os_def.px);
-    assert(os->qh.qh == pico8_os_def.qh);
     assert(os->qh.s == pico8_os_def.s);
     assert(os->qh.probe->od == pico8_os_def.probe.od);
     assert_same_string(os->qh.jo.header.me, pico8_os_def.me);
@@ -143,24 +139,9 @@ static void test_hj_defaults(void) {
     pico8_hj_t *hj = pico8_hj_new(NULL);
 
     assert(pico8_hj_ready());
-    assert_same_string(hj->rr, pico8_hj_start.rr);
-    assert(hj->qh.hp == pico8_hj_start.hp);
-    assert(hj->qh.px == pico8_hj_start.px);
-    assert(hj->qh.probe->od == pico8_hj_start.od);
-    assert(hj->qh.s == pico8_hj_start.s);
-    assert(hj->emitter->def == NULL);
-    assert(hj->emitter->py == pico8_hj_start.py);
-    assert(hj->emitter->ki == pico8_hj_start.ki);
-    assert(hj->emitter->pn == pico8_hj_start.pn);
-    assert(hj->emitter->pk == pico8_hj_start.pk);
-    assert(hj->emitter->fc == pico8_hj_start.fc);
-    assert(hj->emitter->k_ == pico8_hj_start.k_);
-    assert(hj->emitter->du == pico8_hj_start.du);
-    assert(hj->emitter->qo == pico8_hj_start.qo);
-    assert(hj->emitter->ro == pico8_hj_start.ro);
-    assert(hj->emitter->qg == pico8_hj_start.qg);
-    assert(hj->emitter->q_ == pico8_hj_start.q_);
-    assert(hj->qh.qh == pico8_hj_start.qh);
+    assert(hj->qh.probe->od == pico8_hj_def.probe.od);
+    assert(hj->qh.s == pico8_hj_def.s);
+    assert_emitter(hj->emitter, &pico8_hj_def);
     assert_same_string(hj->qh.jo.header.me, pico8_hj_def.me);
     assert(hj->qh.probe->fp == pico8_hj_def.probe.fp);
     assert(hj->qh.probe->dw == pico8_hj_def.probe.dw);
@@ -175,26 +156,10 @@ static void test_pm_defaults(void) {
     pico8_pm_t *pm = pico8_pm_new(NULL);
 
     assert(pico8_pm_ready());
-    assert_same_string(pm->fz, pico8_pm_start.fz);
-    assert(pm->hj.qh.hp == pico8_pm_start.hp);
-    assert(pm->hj.qh.px == pico8_pm_start.px);
-    assert_same_string(pm->hj.qh.jo.header.me, pico8_pm_start.me);
-    assert(pm->hj.qh.s == pico8_pm_start.s);
-    assert(pm->hj.emitter->py == pico8_pm_start.py);
-    assert(pm->hj.emitter->ki == pico8_pm_start.ki);
-    assert(pm->hj.emitter->pn == pico8_pm_start.pn);
-    assert(pm->hj.emitter->pk == pico8_pm_start.pk);
-    assert_same_vec(pm->ng, &pico8_pm_start.ng);
-    assert(pm->hj.emitter->du == pico8_pm_start.du);
-    assert(pm->hj.emitter->fc == pico8_pm_start.fc);
-    assert(pm->hj.emitter->k_ == pico8_pm_start.k_);
-    assert(pm->hj.emitter->qo == pico8_pm_start.qo);
-    assert(pm->hj.emitter->ro == pico8_pm_start.ro);
-    assert(pm->hj.emitter->qg == pico8_pm_start.qg);
-    assert(pm->hj.emitter->q_ == pico8_pm_start.q_);
-    assert_same_string(pm->hj.rr, pico8_hj_start.rr);
-    assert(pm->hj.qh.qh == pico8_hj_start.qh);
-    assert(pm->hj.qh.probe->od == pico8_hj_start.od);
+    assert_same_string(pm->hj.qh.jo.header.me, pico8_pm_def.me);
+    assert(pm->hj.qh.s == pico8_pm_def.s);
+    assert_emitter(pm->hj.emitter, &pico8_pm_def);
+    assert(pm->hj.qh.probe->od == pico8_pm_def.probe.od);
     assert(pm->hj.qh.probe->fp == pico8_pm_def.probe.fp);
     assert(pm->hj.qh.probe->dw == pico8_pm_def.probe.dw);
     assert_qh_tail(&pm->hj.qh, &pico8_pm_def);
@@ -242,10 +207,7 @@ static void assert_nr_own(const pico8_nr_t *nr) {
 
 /* What nr and fr inherit from qh and jo. */
 static void assert_nr_inherited(const pico8_nr_t *nr, const pico8_jo_def_t *def) {
-    assert(nr->qh.hp == def->hp);
-    assert(nr->qh.px == def->px);
     assert_same_string(nr->qh.jo.header.me, def->me);
-    assert(nr->qh.qh == def->qh);
     assert_qh_tail(&nr->qh, def);
     assert(nr->qh.jo.fa == nr->qh.jo.def->fa);
     assert_jo_tail(&nr->qh.jo, def);
@@ -280,24 +242,10 @@ static void test_mx_defaults(void) {
     pico8_mx_t *mx = pico8_mx_new(NULL);
 
     assert(pico8_mx_ready());
-    assert_same_string(mx->hj.rr, pico8_mx_start.rr);
-    assert(mx->hj.qh.hp == pico8_mx_start.hp);
-    assert(mx->hj.qh.s == pico8_mx_start.s);
-    assert(mx->hj.qh.px == pico8_mx_start.px);
-    assert(mx->hj.qh.probe->od == pico8_mx_start.od);
-    assert(mx->ht == pico8_mx_start.ht);
-    assert(mx->hj.emitter->py == pico8_mx_start.py);
-    assert(mx->hj.emitter->ki == pico8_mx_start.ki);
-    assert(mx->hj.emitter->pn == pico8_mx_start.pn);
-    assert(mx->hj.emitter->pk == pico8_mx_start.pk);
-    assert(mx->hj.emitter->fc == pico8_mx_start.fc);
-    assert(mx->hj.emitter->k_ == pico8_mx_start.k_);
-    assert(mx->hj.emitter->du == pico8_mx_start.du);
-    assert(mx->hj.emitter->qo == pico8_mx_start.qo);
-    assert(mx->hj.emitter->ro == pico8_mx_start.ro);
-    assert(mx->hj.emitter->qg == pico8_mx_start.qg);
-    assert(mx->hj.emitter->q_ == pico8_mx_start.q_);
-    assert(mx->hj.qh.qh == pico8_hj_start.qh);
+    assert(mx->hj.qh.s == pico8_mx_def.s);
+    assert(mx->hj.qh.probe->od == pico8_mx_def.probe.od);
+    assert(mx->ht == pico8_mx_def.ht);
+    assert_emitter(mx->hj.emitter, &pico8_mx_def);
     assert_same_string(mx->hj.qh.jo.header.me, pico8_mx_def.me);
     assert(mx->hj.qh.probe->fp == pico8_mx_def.probe.fp);
     assert(mx->hj.qh.probe->dw == pico8_mx_def.probe.dw);
diff --git a/tests/test_pico8_map_tables.c b/tests/test_pico8_map_tables.c
index a626500..0415ee4 100644
--- a/tests/test_pico8_map_tables.c
+++ b/tests/test_pico8_map_tables.c
@@ -151,8 +151,8 @@ static void test_gn_pools_df_entries_by_their_r(void) {
  * tree shares a pointer with another string of the generated data. */
 static void test_jo_tree_strings_are_distinct_objects(void) {
     const char *const tree[] = {pico8_jo_def.me, pico8_qh_def.me, pico8_qh_def.probe.dq, pico8_nl_def.rr,
-        pico8_hj_start.rr, pico8_pm_start.fz, pico8_pm_start.me, pico8_ga_start.fz, pico8_nr_start.rr,
-        pico8_fr_start.rr, pico8_mx_start.rr};
+        pico8_hj_def.rr, pico8_pm_def.fz, pico8_pm_def.me, pico8_ga_start.fz, pico8_nr_start.rr,
+        pico8_fr_start.rr, pico8_mx_def.rr};
     const char *const others[] = {pico8_ke_start.me, pico8_mz_def.me, pico8_mz_def.probe.dq, pico8_jc_def.me,
         pico8_g__def.probe.dq, pico8_mz_jj_qs};
     int n = (int)(sizeof tree / sizeof tree[0]);
diff --git a/tests/test_pico8_mx.c b/tests/test_pico8_mx.c
index b76b027..80ea2a0 100644
--- a/tests/test_pico8_mx.c
+++ b/tests/test_pico8_mx.c
@@ -150,7 +150,7 @@ static uint32_t fold_kw(uint32_t h, const pico8_jo_t *jo) {
 }
 
 static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
-    return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return qh->jo.def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 /* `e` as an ascension, a `pm` or an `mx`: an `hj` first. */
@@ -267,7 +267,7 @@ static void *build_synthesizer(void) {
  * class is a synthesizer. */
 static int is_mx(const void *e) {
     const pico8_qh_t *qh = pico8_qh_of(e);
-    return qh != NULL && qh->px == &pico8_mx_px_row;
+    return qh != NULL && qh->jo.def->px == &pico8_mx_px_row;
 }
 
 /* `ht` of an `mx`; NULL on any other struct, which has no `ht`. */
@@ -301,9 +301,9 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fold_string_member(h, jo->id);
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
-    h = fold_string_member(h, hj->rr);
+    h = fold_string_member(h, hj->qh.jo.def->rr);
     h = fold_qh(h, qh);
-    h = fold_fixed_member(h, qh->hp);
+    h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, em->py);
@@ -331,7 +331,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     /* The 1s of hj's ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
      * `dw`, `s`, then of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the
      * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
-    h = fnv(h, (uint32_t)(hj->rr != NULL));
+    h = fnv(h, (uint32_t)(hj->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -345,7 +345,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(em->qy != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->px != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -391,7 +391,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     assert(cls_of(e)->kf != hj_cls->kf && cls_of(e)->kf != pico8_jo_struct_class()->kf);
     assert(cls_of(e)->cn == hj_cls->cn && cls_of(e)->mm == hj_cls->mm && cls_of(e)->mm != NULL &&
            pico8_jo_is_family(e));
-    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->px == &pico8_mx_px_row && ht_of(e) == &pico8_mx_ht_seq &&
+    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->jo.def->px == &pico8_mx_px_row && ht_of(e) == &pico8_mx_ht_seq &&
            em->k_ == &pico8_mx_k__seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(em->k_ != &pico8_hj_k__seq);
diff --git a/tests/test_pico8_nl.c b/tests/test_pico8_nl.c
index 3689c63..45a99d9 100644
--- a/tests/test_pico8_nl.c
+++ b/tests/test_pico8_nl.c
@@ -151,7 +151,7 @@ static pico8_qh_t *as_qh(void *e) {
 }
 
 static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
-    return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return qh->jo.def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 /* 1 no target, 3 a target: the bytes the frozen hashes fold. */
@@ -269,7 +269,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fold_string_member(h, nl->qh.jo.def->rr);
     h = fold_fixed_member(h, nl->qh.jo.def->nl);
     h = fold_qh(h, qh);
-    h = fold_fixed_member(h, qh->hp);
+    h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, nl->qw);
@@ -290,7 +290,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->px != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -337,7 +337,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
            cls_of(e)->mm == NULL);
     assert(cls_of(e)->states[PICO8_STATE_LY] != pico8_qh_struct_class()->states[PICO8_STATE_LY]);
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
-    assert(qh->px == &pico8_nl_px_row && qh->px != &pico8_qh_px_row);
+    assert(qh->jo.def->px == &pico8_nl_px_row && qh->jo.def->px != &pico8_qh_px_row);
     assert(qh->jo.def->ba == &pico8_nl_ba_seq && qh->jo.def->ba != &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_nl_def.kq);
     assert(jo->def == &pico8_nl_def);
diff --git a/tests/test_pico8_nr.c b/tests/test_pico8_nr.c
index 88f7367..4105a66 100644
--- a/tests/test_pico8_nr.c
+++ b/tests/test_pico8_nr.c
@@ -148,7 +148,7 @@ static pico8_qh_t *as_qh(void *e) {
 }
 
 static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
-    return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return qh->jo.def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static void check(const char *label, uint32_t got, uint32_t want) {
@@ -273,7 +273,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
     h = fold_string_member(h, nr->rr);
-    h = fold_fixed_member(h, qh->hp);
+    h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, qh->probe->fp);
@@ -294,7 +294,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(nr->hz != NULL));
     h = fnv(h, (uint32_t)(nr->rr != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->px != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -350,7 +350,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && nr->hz == &pico8_nr_hz_seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(jo->kq != &pico8_jo_def.kq);
-    assert(qh->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
+    assert(qh->jo.def->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->def == &pico8_nr_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
diff --git a/tests/test_pico8_os.c b/tests/test_pico8_os.c
index 2f9d918..31de6a1 100644
--- a/tests/test_pico8_os.c
+++ b/tests/test_pico8_os.c
@@ -165,7 +165,7 @@ static pico8_qh_t *as_qh(void *e) {
 }
 
 static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
-    return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return qh->jo.def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 /* 1 no target, 3 a target: the bytes the frozen hashes fold. */
@@ -283,7 +283,7 @@ static void *build_processor(void) {
 /* A processor is the only entity whose `px` is os's own table. */
 static int is_processor(const void *e) {
     const pico8_qh_t *qh = pico8_qh_of(e);
-    return qh != NULL && qh->px == &pico8_os_px_row;
+    return qh != NULL && qh->jo.def->px == &pico8_os_px_row;
 }
 
 /* A processor as `ie:ldown` and `jo:pq` left it: its scalars (`hs` is jo's
@@ -300,7 +300,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
     h = fold_qh(h, qh);
-    h = fold_fixed_member(h, qh->hp);
+    h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, jo->def->hs);
@@ -317,7 +317,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
      * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
      * still proves the typed reads. */
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->px != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -362,7 +362,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     assert(cls_of(e)->kf == pico8_qh_struct_class()->kf && cls_of(e)->cn == pico8_qh_struct_class()->cn &&
            cls_of(e)->mm == NULL && pico8_jo_is_family(e));
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
-    assert(qh->px == &pico8_os_px_row && qh->px != &pico8_qh_px_row);
+    assert(qh->jo.def->px == &pico8_os_px_row && qh->jo.def->px != &pico8_qh_px_row);
     assert(qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_os_def.kq);
     assert(jo->def == &pico8_os_def);
diff --git a/tests/test_pico8_pm.c b/tests/test_pico8_pm.c
index f3ee78e..295b607 100644
--- a/tests/test_pico8_pm.c
+++ b/tests/test_pico8_pm.c
@@ -132,7 +132,7 @@ static uint32_t fold_kw(uint32_t h, const pico8_jo_t *jo) {
 }
 
 static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
-    return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return qh->jo.def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 /* `e` as an ascension, a `pm` or an `mx`: an `hj` first. */
@@ -240,7 +240,7 @@ static void *build_protostar(const pico8_vec_t *p, const pico8_vec_t *ra, const
  * class is a protostar. */
 static int is_pm(const void *e) {
     const pico8_qh_t *qh = pico8_qh_of(e);
-    return qh != NULL && qh->px == &pico8_pm_px_row;
+    return qh != NULL && qh->jo.def->px == &pico8_pm_px_row;
 }
 
 static int is_ga(const void *e) {
@@ -262,7 +262,7 @@ static uint32_t fold_spawned(uint32_t h, void *e) {
     assert(at >= 1 && cls_of(e) == pico8_pm_struct_class());
     pm = e;
     ra = pm->hj.qh.jo.ra;
-    ng = pm->ng;
+    ng = &pm->hj.qh.jo.def->ng;
     assert(is_ga(hv->items[at - 1]));
     g = (const pico8_ga_t *)hv->items[at - 1];
     gra = g->jo.ra;
@@ -297,10 +297,10 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fold_string_member(h, jo->id);
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
-    h = fold_string_member(h, pm->fz);
-    h = fold_string_member(h, hj->rr);
+    h = fold_string_member(h, pm->hj.qh.jo.def->fz);
+    h = fold_string_member(h, hj->qh.jo.def->rr);
     h = fold_qh(h, qh);
-    h = fold_fixed_member(h, qh->hp);
+    h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, em->py);
@@ -322,16 +322,16 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fold_fixed_member(h, jo->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
-    h = fold_vec(h, pm->ng);
+    h = fold_vec(h, &pm->hj.qh.jo.def->ng);
     h = fold_vec(h, &jo->def->gq);
     h = fold_vec(h, jo->ra);
     h = fold_vec(h, jo->p);
     /* The 1s of hj's ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
      * `dw`, `s`, then of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the
      * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
-    h = fnv(h, (uint32_t)(pm->ng != NULL));
-    h = fnv(h, (uint32_t)(pm->fz != NULL));
-    h = fnv(h, (uint32_t)(hj->rr != NULL));
+    h = fnv(h, 1u);
+    h = fnv(h, (uint32_t)(pm->hj.qh.jo.def->fz != NULL));
+    h = fnv(h, (uint32_t)(hj->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -345,7 +345,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(em->qy != NULL));
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->px != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -392,8 +392,8 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
            pico8_jo_is_family(e));
     assert(cls_of(e)->mm == pico8_hj_struct_class()->mm && cls_of(e)->mm != NULL);
     assert(cls_of(e)->kj == pico8_hj_struct_class()->kj);
-    assert(pm->ng == &pico8_pm_start.ng);
-    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->px == &pico8_pm_px_row && em->k_ == &pico8_pm_k__seq);
+    assert(pm->hj.qh.jo.def->ng.x == pico8_fixed_from_int(1) && pm->hj.qh.jo.def->ng.y == 0);
+    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->jo.def->px == &pico8_pm_px_row && em->k_ == &pico8_pm_k__seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(em->k_ != &pico8_hj_k__seq);
     assert(qh->jo.def->ba == &pico8_qh_ba_seq);
diff --git a/tests/test_pico8_qh.c b/tests/test_pico8_qh.c
index 8c4de33..13ff10b 100644
--- a/tests/test_pico8_qh.c
+++ b/tests/test_pico8_qh.c
@@ -111,7 +111,7 @@ static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
 }
 
 static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
-    return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return qh->jo.def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 /* 0 absent, 1 false, 2 true. */
@@ -266,7 +266,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fold_string_member(h, jo->id);
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
-    h = fold_fixed_member(h, qh->hp);
+    h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_string_member(h, qh->probe->dq);
     h = fold_qh(h, qh);
     h = fold_fixed_member(h, qh->probe->od);
@@ -284,7 +284,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
      * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
      * still proves the typed reads. */
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(qh->px != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -328,7 +328,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 1);
     assert(cls_of(e)->kf == pico8_qh_struct_class()->kf && cls_of(e)->cn == pico8_qh_struct_class()->cn &&
            cls_of(e)->mm == NULL && pico8_jo_is_family(e));
-    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
+    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->jo.def->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->kq == &pico8_qh_def.kq);
     assert(jo->def == &pico8_qh_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
@@ -513,7 +513,7 @@ static uint32_t fold_descendant(uint32_t h, void *d) {
     h = fold_hf(h, &qh->jo);
     h = qh->jo.has_mi ? fold_fixed_member(h, qh->jo.mi) : fold_absent(h);
     h = fnv(h, (uint32_t)(qh->jo.header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER))));
-    h = fnv(h, (uint32_t)(qh->px == &pico8_qh_px_row));
+    h = fnv(h, (uint32_t)(qh->jo.def->px == &pico8_qh_px_row));
     h = fnv(h, (uint32_t)(qh->jo.def->ba == &pico8_qh_ba_seq));
     assert(cls_of(d)->hq == pico8_qh_struct_class()->hq && cls_of(d)->ey == pico8_qh_struct_class()->ey);
     assert(cls_of(d)->ev == pico8_qh_struct_class()->ev);
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index 98607b9..89105a9 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -838,7 +838,7 @@ static int identity_of(const char *s) {
     if (s == pico8_qh_def.me) {
         return 3;
     }
-    if (s == pico8_pm_start.me) {
+    if (s == pico8_pm_def.me) {
         return 4;
     }
     return 0;
```

#### Diff de 2.13e2a (`git diff e1 e2a`)

```diff
diff --git a/src/pico8_economy_score.c b/src/pico8_economy_score.c
index bbf47d9..32db7e6 100644
--- a/src/pico8_economy_score.c
+++ b/src/pico8_economy_score.c
@@ -107,7 +107,7 @@ pico8_fixed_t pico8_db(const pico8_struct_header_t *pt) {
                 const pico8_jo_t *oz;
                 assert(pico8_jo_is_family(oz_val) && "db: a trade leads to a planet");
                 oz = (const pico8_jo_t *)oz_val;
-                if (oz->hf) {
+                if (oz->def->hf) {
                     pico8_fixed_t trade_mult = pico8_fixed_from_int(0);
                     pico8_fixed_t hs = jo->def->hs;
                     pico8_fixed_t ct_pt = pico8_fixed_from_int(0);
diff --git a/src/pico8_economy_tiers.c b/src/pico8_economy_tiers.c
index 3fd648b..9c8fc96 100644
--- a/src/pico8_economy_tiers.c
+++ b/src/pico8_economy_tiers.c
@@ -276,7 +276,7 @@ pico8_fixed_t pico8_dh(const pico8_struct_header_t *pt) {
         return fixed_min(pico8_fixed_from_int(count), pico8_fixed_from_int(3));
     }
 
-    if (!jo->hf) {
+    if (!jo->def->hf) {
         if (jy != NULL && jy->count > 0) {
             void *last_val = jy->items[jy->count - 1];
             if (last_val != NULL) {
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index 3822e30..3838fe6 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -436,22 +436,6 @@ static const char nr_rr_text[] = "slipgates";
 static const char fr_rr_text[] = "infraspace";
 static const char mx_rr_text[] = "void synthesis";
 
-const pico8_ga_start_t pico8_ga_start = {
-    .fz = ga_fz_text,
-    .hf = 0,
-    .kq = {0x0, 0x0, -0x10000, -0x10000},
-    .fa = 0x30000,
-    .fc = 0x0,
-    .qo = 0xe666,
-    .ro = 0x0,
-    .qg = 0x0,
-    .q_ = 0xe666,
-    .k_ = &pico8_ga_k__seq,
-    .iu = -0xf5c,
-    .py = 0x60000,
-    .ms = 0x30000,
-};
-
 const pico8_ga_pq_keys_t pico8_pm_ga_pq_keys = {
     .qo = 0xf850,
     .ro = -0x2020,
@@ -463,24 +447,6 @@ const pico8_ga_pq_keys_t pico8_pm_ga_pq_keys = {
     .k_ = &pico8_pm_ga_k__seq,
 };
 
-const pico8_nr_start_t pico8_nr_start = {
-    .od = 0xa0000,
-    .fp = 0x10000,
-    .kq = {-0x40000, -0x40000, 0x50000, 0x50000},
-    .s = 0xaa0000,
-    .hz = &pico8_nr_hz_seq,
-    .dw = 0x28e,
-    .rr = nr_rr_text,
-};
-
-const pico8_fr_start_t pico8_fr_start = {
-    .mt = 0x10000,
-    .rr = fr_rr_text,
-    .od = 0x190000,
-    .s = 0x4e0000,
-    .hz = &pico8_fr_hz_seq,
-};
-
 const pico8_jo_def_t pico8_jo_def = {
     .me = jo_me_text,
     .t = 0x0,
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index cb582e5..92b6263 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -305,24 +305,6 @@ typedef struct {
     pico8_emitter_def_t emitter;
 } pico8_jo_def_t;
 
-/* The jo tree: each class's own keys only, the inherited ones are its parent's;
- * ga.by, an empty state list, is held by its module. */
-typedef struct {
-    const char *fz;
-    int hf;
-    pico8_qz_box_t kq;
-    pico8_fixed_t fa;
-    pico8_fixed_t fc;
-    pico8_fixed_t qo;
-    pico8_fixed_t ro;
-    pico8_fixed_t qg;
-    pico8_fixed_t q_;
-    const pico8_int_seq_t *k_;
-    pico8_fixed_t iu;
-    pico8_fixed_t py;
-    pico8_fixed_t ms;
-} pico8_ga_start_t;
-
 /* The keys pm:pq passes to the ga it spawns, besides `ra`. */
 typedef struct {
     pico8_fixed_t qo;
@@ -335,24 +317,6 @@ typedef struct {
     const pico8_int_seq_t *k_;
 } pico8_ga_pq_keys_t;
 
-typedef struct {
-    pico8_fixed_t od;
-    pico8_fixed_t fp;
-    pico8_qz_box_t kq;
-    pico8_fixed_t s;
-    const pico8_int_seq_t *hz;
-    pico8_fixed_t dw;
-    const char *rr;
-} pico8_nr_start_t;
-
-typedef struct {
-    pico8_fixed_t mt;
-    const char *rr;
-    pico8_fixed_t od;
-    pico8_fixed_t s;
-    const pico8_int_seq_t *hz;
-} pico8_fr_start_t;
-
 extern const int pico8_es_table[PICO8_ES_COUNT];
 extern const pico8_fixed_t pico8_ct_table[PICO8_CT_COUNT];
 extern const int pico8_b__table[PICO8_B__COUNT];
@@ -410,10 +374,7 @@ extern const pico8_jb_row_t pico8_pj_ou_row;
 /* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
  * nf.r are empty tables, held by their module. */
 extern const pico8_ke_start_t pico8_ke_start;
-extern const pico8_ga_start_t pico8_ga_start;
 extern const pico8_ga_pq_keys_t pico8_pm_ga_pq_keys;
-extern const pico8_nr_start_t pico8_nr_start;
-extern const pico8_fr_start_t pico8_fr_start;
 extern const pico8_bg_def_t pico8_bg_def;
 extern const pico8_ui_def_t pico8_ui_def;
 extern const pico8_js_def_t pico8_js_def;
diff --git a/src/pico8_particle.c b/src/pico8_particle.c
index 6e5510f..5fab564 100644
--- a/src/pico8_particle.c
+++ b/src/pico8_particle.c
@@ -27,7 +27,7 @@ static pico8_fixed_t emitter_host_fa_ra(pico8_struct_header_t *self, pico8_vec_t
            "ei: a host neither of g_'s line nor of jo's family, none of whose kj runs ei -- stop");
     jo = (const pico8_jo_t *)self;
     *ra = jo->ra != NULL ? *jo->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-    return jo->fa;
+    return jo->def->fa;
 }
 
 static int pico8_particle_to_int(pico8_fixed_t x) {
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index b3aaace..7eb6692 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -77,7 +77,7 @@ void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
 }
 
 int pico8_jo_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_jo_t *)self)->fa;
+    *out = ((const pico8_jo_t *)self)->def->fa;
     return 1;
 }
 
@@ -90,7 +90,7 @@ const pico8_vec_t *pico8_jo_get_p(const void *self) {
 }
 
 const pico8_qz_box_t *pico8_jo_get_kq(const void *self) {
-    return ((const pico8_jo_t *)self)->kq;
+    return &((const pico8_jo_t *)self)->def->kq;
 }
 
 int pico8_jo_get_fx(const void *self) {
@@ -136,9 +136,6 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     assert(g_jo_up && "pico8_jo_init() must run first");
     pico8_ke_struct_init(&self->header, cls);
     self->def = def;
-    self->hf = self->def->hf;
-    self->fa = self->def->fa;
-    self->kq = &self->def->kq;
     self->ma = self->def->ma;
     self->kw = &g_jo_kw_shared;
     self->id = NULL;
@@ -326,7 +323,7 @@ int pico8_jo_gg(const pico8_jo_t *self) {
     if (self == NULL) {
         return 0;
     }
-    if (!self->hf) {
+    if (!self->def->hf) {
         return 0;
     }
     return self->header.me != NULL && strcmp(self->header.me, "ly") == 0;
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index 46456d7..d3193cd 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -16,7 +16,7 @@
  * for the value-returning functions), and an absent or wrong-kind field
  * reads as absent rather than crashing. */
 
-/* A bare planet: its kind's archetype (`kq` points into it; `kw` points at
+/* A bare planet: its kind's archetype (`kw` points at
  * the one kw every bare planet shares, written in place by `bb` and `mx:kf`,
  * until `lz` points it at the planet's own `kw_own`), its construction keys
  * `id`/`p`/`ra`, then every field jo's functions write on it. Every planet
@@ -25,9 +25,6 @@
 typedef struct {
     pico8_struct_header_t header;
     const pico8_jo_def_t *def;
-    int hf;
-    pico8_fixed_t fa;
-    const pico8_qz_box_t *kq;
     pico8_fixed_t ma;
     pico8_kw_t *kw;
     const char *id;
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index 12c2a70..d58e0e5 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -522,27 +522,19 @@ static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
     pico8_jo_struct_init(&self->jo, &g_ga_struct_class, &pico8_ga_def);
-    self->fz = pico8_ga_start.fz;
     self->emitter = &self->emitter_own;
-    self->emitter->def = NULL;
-    self->emitter->pn = 0;
-    self->emitter->du = 0;
-    self->emitter->ki = 0;
-    self->emitter->pk = 0;
-    self->emitter->fc = pico8_ga_start.fc;
-    self->emitter->qo = pico8_ga_start.qo;
-    self->emitter->ro = pico8_ga_start.ro;
-    self->emitter->qg = pico8_ga_start.qg;
-    self->emitter->q_ = pico8_ga_start.q_;
-    self->emitter->k_ = pico8_ga_start.k_;
-    self->iu = pico8_ga_start.iu;
-    self->emitter->py = pico8_ga_start.py;
-    self->ms = pico8_ga_start.ms;
+    self->emitter->def = &self->jo.def->emitter;
+    self->emitter->fc = self->jo.def->emitter.fc;
+    self->emitter->qo = self->jo.def->emitter.qo;
+    self->emitter->ro = self->jo.def->emitter.ro;
+    self->emitter->qg = self->jo.def->emitter.qg;
+    self->emitter->q_ = self->jo.def->emitter.q_;
+    self->emitter->k_ = self->jo.def->emitter.k_;
+    self->iu = self->jo.def->iu;
+    self->emitter->py = self->jo.def->emitter.py;
+    self->ms = self->jo.def->ms;
     self->emitter->qy = NULL;
     self->jo.by = &g_ga_by_shared;
-    self->jo.hf = pico8_ga_start.hf;
-    self->jo.kq = &pico8_ga_start.kq;
-    self->jo.fa = pico8_ga_start.fa;
     if (keys != NULL) {
         if (keys->jo.p != NULL) {
             self->jo.p = keys->jo.p;
diff --git a/src/pico8_planet_family.h b/src/pico8_planet_family.h
index 5312ee7..fa71fe0 100644
--- a/src/pico8_planet_family.h
+++ b/src/pico8_planet_family.h
@@ -68,17 +68,16 @@ typedef struct {
     pico8_hj_t hj;
 } pico8_pm_t;
 
-/* A particle burst: a bare planet, then ga's own defaults, those of the
- * emitter in it with `qy`, which its own `pq` (`f_`) writes. A default of
- * ga whose name is a member of `jo` (`hf`, `by`, `kq`, `fa`) lives in
- * that member. The emitter's `k_`, `kq` and `fz` point at ga's
- * generated data; `by` is the one list every burst shares, which `ib`
- * extends in place and each init empties. A burst spawned by `pm:pq` points
- * `k_` at pico8_pm_ga_k__seq instead. `emitter` points at `emitter_own`:
- * never copy the struct. Nothing frees the struct: TROU-7. */
+/* A particle burst: a bare planet, then `iu`, `ms` and an emitter copied
+ * from its archetype, with `qy`, which its own `pq` (`f_`) writes; the
+ * emitter's `pn`, `du`, `ki`, `pk` are left unwritten, read by `g_:mm`
+ * only. `jo.by` is the one list every burst shares, which `ib` extends in
+ * place and each init empties. A burst spawned by `pm:pq` takes `iu`, `ms`,
+ * `py`, `k_`, `qo`, `ro`, `qg`, `q_` from its keys instead. `emitter` points
+ * at `emitter_own`: never copy the struct. Nothing frees the struct:
+ * TROU-7. */
 typedef struct {
     pico8_jo_t jo;
-    const char *fz;
     pico8_fixed_t iu;
     pico8_fixed_t ms;
     pico8_emitter_t *emitter;
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index cf3a4f1..757a908 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -90,9 +90,9 @@ void pico8_nr_hb(pico8_nr_t *self) {
 
     oe_count = self->qh.jo.oe != NULL ? self->qh.jo.oe->count : 0;
 
-    if (self->hz != NULL) {
+    if (self->qh.jo.def->hz != NULL) {
         int entry;
-        if (pico8_int_seq_at(self->hz->items, self->hz->count, oe_count + 1, &entry)) {
+        if (pico8_int_seq_at(self->qh.jo.def->hz->items, self->qh.jo.def->hz->count, oe_count + 1, &entry)) {
             s_val = pico8_fixed_from_int(entry);
         }
     }
@@ -187,18 +187,12 @@ static const pico8_struct_class_t g_nr_struct_class = {
     .set_m_ = pico8_jo_set_m_,
 };
 
-/* Initializes an allocated pico8_nr_t under `cls`: qh's part, nr's members
- * nil, nr's defaults. Reads no key, runs neither `pq` nor `lt`, records no
- * coverage. Requires `pico8_planet_family_trade_init()`. */
+/* Initializes an allocated pico8_nr_t under `cls` and `def`, its kind's
+ * archetype: qh's part, nr having no member of its own. Reads no key, runs
+ * neither `pq` nor `lt`, records no coverage. Requires
+ * `pico8_planet_family_trade_init()`. */
 static void pico8_nr_struct_init(pico8_nr_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     pico8_qh_struct_init(&self->qh, cls, def);
-    self->hz = pico8_nr_start.hz;
-    self->rr = pico8_nr_start.rr;
-    self->qh.probe->od = pico8_nr_start.od;
-    self->qh.probe->fp = pico8_nr_start.fp;
-    self->qh.s = pico8_nr_start.s;
-    self->qh.probe->dw = pico8_nr_start.dw;
-    self->qh.jo.kq = &pico8_nr_start.kq;
 }
 
 static pico8_nr_t *pico8_nr_struct_new(const pico8_jo_keys_t *keys) {
@@ -226,7 +220,7 @@ static pico8_nr_t *pico8_nr_struct_new(const pico8_jo_keys_t *keys) {
 }
 
 static int pico8_fr_get_mt(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_fr_t *)self)->mt;
+    *out = ((const pico8_fr_t *)self)->nr.qh.jo.def->mt;
     return 1;
 }
 
@@ -265,11 +259,6 @@ static pico8_fr_t *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
     pico8_nr_struct_init(&self->nr, &g_fr_struct_class, &pico8_fr_def);
-    self->mt = pico8_fr_start.mt;
-    self->nr.hz = pico8_fr_start.hz;
-    self->nr.rr = pico8_fr_start.rr;
-    self->nr.qh.probe->od = pico8_fr_start.od;
-    self->nr.qh.s = pico8_fr_start.s;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "fr: the only construction key is id (ie:ldown); another key has no member "
diff --git a/src/pico8_planet_family_trade.h b/src/pico8_planet_family_trade.h
index 53ed862..130f769 100644
--- a/src/pico8_planet_family_trade.h
+++ b/src/pico8_planet_family_trade.h
@@ -14,25 +14,16 @@ void pico8_planet_family_trade_init(void);
  * `pico8_planet_family_trade_init()`. */
 void pico8_planet_family_trade_shutdown(void);
 
-/* A slipgate: a laboratory, then nr's own defaults `hz` and `rr`, which no
- * ancestor has a member for. A default of nr whose name is a member of `qh`
- * or `jo` (`kq`, `od`, `fp`, `s`, `dw`) lives in that member.
- * `hz`, `kq` and `rr` point at nr's generated data, `px` at qh's.
- * Nothing frees the struct: TROU-7. */
+/* A slipgate: a laboratory and nothing of its own; its defaults are its
+ * archetype's. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_qh_t qh;
-    const pico8_int_seq_t *hz;
-    const char *rr;
 } pico8_nr_t;
 
-/* An infraspace gate: a slipgate, then fr's own default `mt`, which no
- * ancestor has a member for. A default of fr whose name is a member of `nr`
- * or `qh` (`rr`, `hz`, `od`, `s`) lives in that member. `hz` and `rr` point
- * at fr's generated data, `kq` at nr's, `px` at qh's.
- * Nothing frees the struct: TROU-7. */
+/* An infraspace gate: a slipgate and nothing of its own; its defaults are
+ * its archetype's. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_nr_t nr;
-    pico8_fixed_t mt;
 } pico8_fr_t;
 
 /* A synthesizer: an ascension, then `ht`, copied from its archetype;
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index 2ef913e..d2e34db 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -56,10 +56,6 @@ static int same_text(const char *got, const char *want) {
     return got != NULL && want != NULL && strcmp(got, want) == 0;
 }
 
-static int same_box(const pico8_qz_box_t *got, const pico8_qz_box_t *want) {
-    return got != NULL && got->xl == want->xl && got->yt == want->yt && got->xr == want->xr && got->yb == want->yb;
-}
-
 static void test_bg(void) {
     const pico8_bg_t *bg = pico8_bg_new();
 
@@ -213,32 +209,6 @@ static void test_os(void) {
     assert_lz_ran(&os->qh.jo);
 }
 
-/* What nr's init still writes, for nr and fr. */
-static void same_nr_part(const pico8_qh_t *qh, const pico8_jo_def_t *def, const char *kind) {
-    same_in(qh->jo.header.t == def->t, kind, "t");
-    same_in(same_box(qh->jo.kq, &def->kq), kind, "kq");
-    same_in(qh->s == def->s, kind, "s");
-    same_in(qh->probe->dw == def->probe.dw, kind, "probe.dw");
-    same_in(qh->probe->od == def->probe.od, kind, "probe.od");
-    same_in(qh->probe->fp == def->probe.fp, kind, "probe.fp");
-    assert(qh->probe->et == NULL && qh->jo.by == NULL);
-}
-
-/* `qy` is left to each kind's test. */
-static void same_emitter_part(const pico8_emitter_t *emitter, const pico8_jo_def_t *def, const char *kind) {
-    same_in(emitter->pn == def->emitter.pn, kind, "emitter.pn");
-    same_in(emitter->du == def->emitter.du, kind, "emitter.du");
-    same_in(emitter->py == def->emitter.py, kind, "emitter.py");
-    same_in(emitter->ki == def->emitter.ki, kind, "emitter.ki");
-    same_in(emitter->pk == def->emitter.pk, kind, "emitter.pk");
-    same_in(emitter->fc == def->emitter.fc, kind, "emitter.fc");
-    same_in(emitter->k_ == def->emitter.k_, kind, "emitter.k_");
-    same_in(emitter->qo == def->emitter.qo, kind, "emitter.qo");
-    same_in(emitter->ro == def->emitter.ro, kind, "emitter.ro");
-    same_in(emitter->qg == def->emitter.qg, kind, "emitter.qg");
-    same_in(emitter->q_ == def->emitter.q_, kind, "emitter.q_");
-}
-
 static void test_hj(void) {
     const pico8_hj_t *hj = pico8_hj_new(NULL);
 
@@ -274,13 +244,6 @@ static void test_ga(void) {
     const pico8_ga_t *other = pico8_ga_new(NULL);
 
     same_in(ga->jo.header.t == pico8_ga_def.t, "ga", "t");
-    same_in(ga->jo.hf == pico8_ga_def.hf, "ga", "hf");
-    same_in(ga->jo.fa == pico8_ga_def.fa, "ga", "fa");
-    same_in(same_box(ga->jo.kq, &pico8_ga_def.kq), "ga", "kq");
-    same_in(same_text(ga->fz, pico8_ga_def.fz), "ga", "fz");
-    same_in(ga->iu == pico8_ga_def.iu, "ga", "iu");
-    same_in(ga->ms == pico8_ga_def.ms, "ga", "ms");
-    same_emitter_part(ga->emitter, &pico8_ga_def, "ga");
     assert(ga->emitter->qy != NULL);
     assert(ga->jo.kw == pico8_jo_shared_kw());
     assert(ga->jo.by != NULL && ga->jo.by == other->jo.by);
@@ -289,19 +252,16 @@ static void test_ga(void) {
 static void test_nr(void) {
     const pico8_nr_t *nr = pico8_nr_new(NULL);
 
-    same_nr_part(&nr->qh, &pico8_nr_def, "nr");
-    same_in(nr->hz == pico8_nr_def.hz, "nr", "hz");
-    same_in(same_text(nr->rr, pico8_nr_def.rr), "nr", "rr");
+    same_in(nr->qh.jo.header.t == pico8_nr_def.t, "nr", "t");
+    assert(nr->qh.probe->et == NULL && nr->qh.jo.by == NULL);
     assert_lz_ran(&nr->qh.jo);
 }
 
 static void test_fr(void) {
     const pico8_fr_t *fr = pico8_fr_new(NULL);
 
-    same_nr_part(&fr->nr.qh, &pico8_fr_def, "fr");
-    same_in(fr->nr.hz == pico8_fr_def.hz, "fr", "hz");
-    same_in(same_text(fr->nr.rr, pico8_fr_def.rr), "fr", "rr");
-    same_in(fr->mt == pico8_fr_def.mt, "fr", "mt");
+    same_in(fr->nr.qh.jo.header.t == pico8_fr_def.t, "fr", "t");
+    assert(fr->nr.qh.probe->et == NULL && fr->nr.qh.jo.by == NULL);
     assert_lz_ran(&fr->nr.qh.jo);
 }
 
@@ -341,7 +301,7 @@ int main(void) {
     test_ga();
     test_nr();
     test_fr();
-    assert(g_compared == 68 && "families a to e compare 68 fields: one missed or one too many");
+    assert(g_compared == 36 && "families a to e compare 36 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_entity_kind.c b/tests/test_pico8_entity_kind.c
index 11c8927..eb8aacc 100644
--- a/tests/test_pico8_entity_kind.c
+++ b/tests/test_pico8_entity_kind.c
@@ -171,16 +171,16 @@ static void test_kind_reader_serves_the_generated_defaults(void) {
     assert(rec->has_od && rec->od == pico8_os_def.probe.od);
 
     rec = rec_of(2);
-    assert(rec->rr == pico8_nr_start.rr);
-    assert(rec->has_s && rec->s == pico8_nr_start.s);
+    assert(rec->rr == pico8_nr_def.rr);
+    assert(rec->has_s && rec->s == pico8_nr_def.s);
     assert(rec->me == pico8_qh_def.me);
-    assert(rec->has_od && rec->od == pico8_nr_start.od);
+    assert(rec->has_od && rec->od == pico8_nr_def.probe.od);
 
     rec = rec_of(3);
-    assert(rec->rr == pico8_fr_start.rr);
-    assert(rec->has_s && rec->s == pico8_fr_start.s);
+    assert(rec->rr == pico8_fr_def.rr);
+    assert(rec->has_s && rec->s == pico8_fr_def.s);
     assert(rec->me == pico8_qh_def.me);
-    assert(rec->has_od && rec->od == pico8_fr_start.od);
+    assert(rec->has_od && rec->od == pico8_fr_def.probe.od);
 
     rec = rec_of(4);
     assert(rec->rr == pico8_nl_def.rr);
diff --git a/tests/test_pico8_fr.c b/tests/test_pico8_fr.c
index 4b84178..3ad5770 100644
--- a/tests/test_pico8_fr.c
+++ b/tests/test_pico8_fr.c
@@ -107,7 +107,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -218,7 +218,7 @@ static void *build(const pico8_entity_kind_t *kind) {
 /* `hz` of a slipgate or a gate; NULL on any other struct, which has none. */
 static const pico8_int_seq_t *hz_of(const void *e) {
     if (cls_of(e) == pico8_nr_struct_class() || cls_of(e) == pico8_fr_struct_class()) {
-        return ((const pico8_nr_t *)e)->hz;
+        return ((const pico8_nr_t *)e)->qh.jo.def->hz;
     }
     return NULL;
 }
@@ -255,8 +255,8 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fold_string_member(h, jo->id);
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
-    h = fold_string_member(h, nr->rr);
-    h = fold_fixed_member(h, fr->mt);
+    h = fold_string_member(h, nr->qh.jo.def->rr);
+    h = fold_fixed_member(h, fr->nr.qh.jo.def->mt);
     h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
@@ -267,17 +267,17 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, &jo->def->gq);
-    h = fnv(h, (uint32_t)nr->hz->count);
+    h = fnv(h, (uint32_t)nr->qh.jo.def->hz->count);
     /* The 1s of `mt`, of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`,
      * `hs`, `fa`, `ma` and `ca`: constant by the struct's shape, kept so that
      * BUILT_HASH still proves the typed reads. */
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(nr->hz != NULL));
-    h = fnv(h, (uint32_t)(nr->rr != NULL));
+    h = fnv(h, (uint32_t)(nr->qh.jo.def->hz != NULL));
+    h = fnv(h, (uint32_t)(nr->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
@@ -292,7 +292,7 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -335,8 +335,8 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     assert(cls_of(e)->ol == NULL && cls_of(e)->mm == NULL);
     assert(cls_of(e)->states[PICO8_STATE_PH] == NULL && cls_of(e)->states[PICO8_STATE_KZ] == NULL &&
            cls_of(e)->states[PICO8_STATE_PR] == NULL && cls_of(e)->rdown == NULL);
-    assert(nr->hz != &pico8_nr_hz_seq);
-    assert(jo->kq == &pico8_nr_start.kq);
+    assert(nr->qh.jo.def->hz != &pico8_nr_hz_seq);
+    assert(jo->def == &pico8_fr_def);
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->jo.def->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->def == &pico8_fr_def);
@@ -520,7 +520,7 @@ static void test_life(void) {
     h = fold_economy(h, placed);
 
     h = fold_route(h, placed);
-    assert(is_a_gate(placed) && ((const pico8_fr_t *)placed)->mt == pico8_fixed_from_int(1));
+    assert(is_a_gate(placed) && ((const pico8_fr_t *)placed)->nr.qh.jo.def->mt == pico8_fixed_from_int(1));
     slipgate = build(pico8_nr_kind());
     place(slipgate, 40, 108);
     h = fold_route(h, slipgate);
diff --git a/tests/test_pico8_ga.c b/tests/test_pico8_ga.c
index 0e571e9..c356dee 100644
--- a/tests/test_pico8_ga.c
+++ b/tests/test_pico8_ga.c
@@ -110,7 +110,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static uint32_t fold_p_(uint32_t h, const pico8_jo_t *jo) {
@@ -210,8 +210,8 @@ static const pico8_struct_class_t *cls_of(const void *e) {
 
 /* `fz` is "ga" on a burst and on nothing else. */
 static int is_ga(const void *e) {
-    return e != NULL && cls_of(e) == pico8_ga_struct_class() && ((const pico8_ga_t *)e)->fz != NULL &&
-           strcmp(((const pico8_ga_t *)e)->fz, "ga") == 0;
+    return e != NULL && cls_of(e) == pico8_ga_struct_class() && ((const pico8_ga_t *)e)->jo.def->fz != NULL &&
+           strcmp(((const pico8_ga_t *)e)->jo.def->fz, "ga") == 0;
 }
 
 static pico8_ga_t *as_ga(void *e) {
@@ -297,8 +297,8 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     int j;
 
     assert(is_ga(e));
-    h = fold_string_member(h, g->fz);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_string_member(h, g->jo.def->fz);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, em->fc);
     h = fold_fixed_member(h, em->qo);
     h = fold_fixed_member(h, em->ro);
@@ -325,7 +325,7 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     /* The 1s of `fc`, `qo`, `ro`, `qg`, `q_`, `iu`, `py`, `ms` (ga's own)
      * and of `hf`, `hs`, `fa`, `ma`, `ca` (jo's) are constant by the
      * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
-    h = fnv(h, (uint32_t)(g->fz != NULL));
+    h = fnv(h, (uint32_t)(g->jo.def->fz != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -340,7 +340,7 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -383,7 +383,7 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
            cls_of(e)->rdown == NULL && cls_of(e)->states[PICO8_STATE_LY] == NULL);
     assert(cls_of(e)->ol == NULL && cls_of(e)->hq == NULL && cls_of(e)->ey == NULL);
     assert(jo->header.cls->tags == 0);
-    assert(jo->kq == &pico8_ga_start.kq);
+    assert(jo->def == &pico8_ga_def);
     assert(jo->by != NULL);
     if (from_pm) {
         assert(em->k_ != &pico8_ga_k__seq);
@@ -427,8 +427,8 @@ static void test_built(void) {
     assert(as_ga(a)->jo.oe != as_ga(b)->jo.oe);
     assert(as_ga(a)->jo.by == as_ga(b)->jo.by && as_ga(b)->jo.by == as_ga(c)->jo.by);
     assert(as_ga(c)->jo.ra == NULL && as_ga(c)->jo.p == NULL && as_ga(c)->jo.id == NULL);
-    assert(as_ga(a)->jo.fa == pico8_fixed_from_int(3) && as_ga(a)->ms == pico8_fixed_from_int(3));
-    assert(as_ga(a)->jo.hf == 0);
+    assert(as_ga(a)->jo.def->fa == pico8_fixed_from_int(3) && as_ga(a)->ms == pico8_fixed_from_int(3));
+    assert(as_ga(a)->jo.def->hf == 0);
     assert(as_ga(b)->ms == PICO8_FIXED_LIT(0x00018000, 1.5));
     check("built", h, BUILT_HASH);
 }
diff --git a/tests/test_pico8_hj.c b/tests/test_pico8_hj.c
index f51913f..a5de5eb 100644
--- a/tests/test_pico8_hj.c
+++ b/tests/test_pico8_hj.c
@@ -110,7 +110,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -331,7 +331,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, &jo->def->gq);
@@ -364,7 +364,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -399,7 +399,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->jo.def->px != &pico8_qh_px_row);
     assert(qh->jo.def->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_hj_def.kq);
+    assert(jo->def == &pico8_hj_def);
     assert(jo->def == &pico8_hj_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
diff --git a/tests/test_pico8_jo.c b/tests/test_pico8_jo.c
index 1af2d65..9fa1cba 100644
--- a/tests/test_pico8_jo.c
+++ b/tests/test_pico8_jo.c
@@ -102,7 +102,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 /* 0 absent, 1 false, 2 true. */
@@ -207,7 +207,7 @@ static uint32_t fold_planet(uint32_t h, const void *e) {
     h = fold_string_member(h, jo->id);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, jo->p);
@@ -220,7 +220,7 @@ static uint32_t fold_planet(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -248,7 +248,7 @@ static uint32_t fold_planet(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)jo->has_mi);
     h = fnv(h, (uint32_t)jo->has_m_);
     assert_jo_members(cls_of(e));
-    assert(jo->kq == &pico8_jo_def.kq);
+    assert(jo->def == &pico8_jo_def);
     assert(jo->def == &pico8_jo_def);
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && jo->kw == pico8_jo_shared_kw() && jo->def->em == &pico8_jo_em_row);
     h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
diff --git a/tests/test_pico8_jo_spec_defaults.c b/tests/test_pico8_jo_spec_defaults.c
index 0e039f5..b2dc19c 100644
--- a/tests/test_pico8_jo_spec_defaults.c
+++ b/tests/test_pico8_jo_spec_defaults.c
@@ -26,15 +26,11 @@ static pico8_mem_t mem;
 static pico8_gfx_t gfx;
 static pico8_input_t input;
 
-/* The very string, vector or box of the generated data, not a copy. */
+/* The very string of the generated data, not a copy. */
 static void assert_same_string(const char *a, const char *b) {
     assert(a != NULL && a == b && strcmp(a, b) == 0);
 }
 
-static void assert_same_box(const pico8_qz_box_t *a, const pico8_qz_box_t *b) {
-    assert(a != NULL && a == b);
-}
-
 /* The members of jo every class of the tree inherits, and its kind's
  * archetype; kw aside, which `lz` points at the planet's own when it has an
  * `hp`. */
@@ -44,10 +40,14 @@ static void assert_jo_tail(const pico8_jo_t *jo, const pico8_jo_def_t *def) {
     assert(jo->mi == def->mi && jo->has_mi == def->has_mi);
 }
 
+static void assert_box(const pico8_qz_box_t *kq, int xl, int yt, int xr, int yb) {
+    assert(kq->xl == pico8_fixed_from_int(xl) && kq->yt == pico8_fixed_from_int(yt) &&
+           kq->xr == pico8_fixed_from_int(xr) && kq->yb == pico8_fixed_from_int(yb));
+}
+
 /* jo's box, which ga and nr replace. */
 static void assert_jo_box(const pico8_qz_box_t *kq) {
-    assert(kq->xl == pico8_fixed_from_int(-7) && kq->yt == pico8_fixed_from_int(-7) &&
-           kq->xr == pico8_fixed_from_int(7) && kq->yb == pico8_fixed_from_int(7));
+    assert_box(kq, -7, -7, 7, 7);
 }
 
 /* The members of qh every class below it inherits, its probe's archetype, and
@@ -55,7 +55,6 @@ static void assert_jo_box(const pico8_qz_box_t *kq) {
 static void assert_qh_tail(const pico8_qh_t *qh, const pico8_jo_def_t *def) {
     assert(qh->probe->def == &def->probe);
     assert_same_string(qh->probe->dq, def->probe.dq);
-    assert(qh->jo.hf == def->hf);
     assert(qh->jo.mi == def->mi && qh->jo.has_mi == 1);
     assert(qh->probe->has_od == 1 && qh->probe->ik == 0 && qh->probe->has_ik == 0);
 }
@@ -74,10 +73,7 @@ static void test_jo_defaults(void) {
 
     assert(pico8_jo_ready());
     assert_same_string(jo->header.me, jo->def->me);
-    assert(jo->hf == jo->def->hf);
-    assert(jo->fa == jo->def->fa);
-    assert_same_box(jo->kq, &jo->def->kq);
-    assert_jo_box(jo->kq);
+    assert_jo_box(&jo->def->kq);
     assert_jo_tail(jo, &pico8_jo_def);
     assert(jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows && jo->def->cq == &pico8_jo_cq_seq);
     assert(jo->mi == 0 && !jo->has_mi);
@@ -96,9 +92,7 @@ static void test_qh_defaults(void) {
     assert(qh->probe->dw == pico8_qh_def.probe.dw);
     assert(qh->s == pico8_qh_def.s);
     assert_qh_tail(qh, &pico8_qh_def);
-    assert(qh->jo.fa == qh->jo.def->fa);
-    assert_same_box(qh->jo.kq, &qh->jo.def->kq);
-    assert_jo_box(qh->jo.kq);
+    assert_jo_box(&qh->jo.def->kq);
     assert_jo_tail(&qh->jo, &pico8_qh_def);
 }
 
@@ -113,9 +107,7 @@ static void test_nl_defaults(void) {
     assert(nl->qh.probe->fp == pico8_nl_def.probe.fp);
     assert(nl->qh.probe->dw == pico8_nl_def.probe.dw);
     assert_qh_tail(&nl->qh, &pico8_nl_def);
-    assert(nl->qh.jo.fa == nl->qh.jo.def->fa);
-    assert_same_box(nl->qh.jo.kq, &nl->qh.jo.def->kq);
-    assert_jo_box(nl->qh.jo.kq);
+    assert_jo_box(&nl->qh.jo.def->kq);
     assert_jo_tail(&nl->qh.jo, &pico8_nl_def);
 }
 
@@ -129,9 +121,7 @@ static void test_os_defaults(void) {
     assert(os->qh.probe->fp == pico8_os_def.probe.fp);
     assert(os->qh.probe->dw == pico8_os_def.probe.dw);
     assert_qh_tail(&os->qh, &pico8_os_def);
-    assert(os->qh.jo.fa == os->qh.jo.def->fa);
-    assert_same_box(os->qh.jo.kq, &os->qh.jo.def->kq);
-    assert_jo_box(os->qh.jo.kq);
+    assert_jo_box(&os->qh.jo.def->kq);
     assert_jo_tail(&os->qh.jo, &pico8_os_def);
 }
 
@@ -146,9 +136,7 @@ static void test_hj_defaults(void) {
     assert(hj->qh.probe->fp == pico8_hj_def.probe.fp);
     assert(hj->qh.probe->dw == pico8_hj_def.probe.dw);
     assert_qh_tail(&hj->qh, &pico8_hj_def);
-    assert(hj->qh.jo.fa == hj->qh.jo.def->fa);
-    assert_same_box(hj->qh.jo.kq, &hj->qh.jo.def->kq);
-    assert_jo_box(hj->qh.jo.kq);
+    assert_jo_box(&hj->qh.jo.def->kq);
     assert_jo_tail(&hj->qh.jo, &pico8_hj_def);
 }
 
@@ -163,9 +151,7 @@ static void test_pm_defaults(void) {
     assert(pm->hj.qh.probe->fp == pico8_pm_def.probe.fp);
     assert(pm->hj.qh.probe->dw == pico8_pm_def.probe.dw);
     assert_qh_tail(&pm->hj.qh, &pico8_pm_def);
-    assert(pm->hj.qh.jo.fa == pm->hj.qh.jo.def->fa);
-    assert_same_box(pm->hj.qh.jo.kq, &pm->hj.qh.jo.def->kq);
-    assert_jo_box(pm->hj.qh.jo.kq);
+    assert_jo_box(&pm->hj.qh.jo.def->kq);
     assert_jo_tail(&pm->hj.qh.jo, &pico8_pm_def);
 }
 
@@ -174,21 +160,18 @@ static void test_ga_defaults(void) {
     pico8_ga_t *other = pico8_ga_new(NULL);
 
     assert(pico8_ga_ready());
-    assert_same_string(ga->fz, pico8_ga_start.fz);
-    assert(ga->jo.hf == pico8_ga_start.hf);
     assert(ga->jo.by != NULL && ga->jo.by == other->jo.by);
-    assert_same_box(ga->jo.kq, &pico8_ga_start.kq);
-    assert(ga->jo.fa == pico8_ga_start.fa);
-    assert(ga->emitter->def == NULL);
-    assert(ga->emitter->fc == pico8_ga_start.fc);
-    assert(ga->emitter->qo == pico8_ga_start.qo);
-    assert(ga->emitter->ro == pico8_ga_start.ro);
-    assert(ga->emitter->qg == pico8_ga_start.qg);
-    assert(ga->emitter->q_ == pico8_ga_start.q_);
-    assert(ga->emitter->k_ == pico8_ga_start.k_);
-    assert(ga->iu == pico8_ga_start.iu);
-    assert(ga->emitter->py == pico8_ga_start.py);
-    assert(ga->ms == pico8_ga_start.ms);
+    assert(ga->emitter->def == &pico8_ga_def.emitter);
+    assert(ga->emitter->fc == pico8_ga_def.emitter.fc);
+    assert(ga->emitter->qo == pico8_ga_def.emitter.qo);
+    assert(ga->emitter->ro == pico8_ga_def.emitter.ro);
+    assert(ga->emitter->qg == pico8_ga_def.emitter.qg);
+    assert(ga->emitter->q_ == pico8_ga_def.emitter.q_);
+    assert(ga->emitter->k_ == pico8_ga_def.emitter.k_);
+    assert(ga->iu == pico8_ga_def.iu);
+    assert(ga->emitter->py == pico8_ga_def.emitter.py);
+    assert(ga->ms == pico8_ga_def.ms);
+    assert_box(&ga->jo.def->kq, 0, 0, -1, -1);
     assert_same_string(ga->jo.header.me, ga->jo.def->me);
     assert_jo_tail(&ga->jo, &pico8_ga_def);
     assert(ga->jo.def->hs == pico8_fixed_from_int(1));
@@ -198,18 +181,15 @@ static void test_ga_defaults(void) {
     assert(ga->jo.kw_own.mu == 0 && !ga->jo.kw_own.has_qe && ga->jo.kw_own.qe == 0);
 }
 
-/* nr's own keys on a struct built by nr_struct_init, under nr or fr. */
-static void assert_nr_own(const pico8_nr_t *nr) {
-    assert(nr->qh.probe->fp == pico8_nr_start.fp);
-    assert_same_box(nr->qh.jo.kq, &pico8_nr_start.kq);
-    assert(nr->qh.probe->dw == pico8_nr_start.dw);
-}
-
-/* What nr and fr inherit from qh and jo. */
+/* What nr and fr inherit from qh and jo, and nr's box, which fr keeps. */
 static void assert_nr_inherited(const pico8_nr_t *nr, const pico8_jo_def_t *def) {
+    assert(nr->qh.probe->fp == def->probe.fp);
+    assert(nr->qh.probe->dw == def->probe.dw);
+    assert(nr->qh.probe->od == def->probe.od);
+    assert(nr->qh.s == def->s);
+    assert_box(&nr->qh.jo.def->kq, -4, -4, 5, 5);
     assert_same_string(nr->qh.jo.header.me, def->me);
     assert_qh_tail(&nr->qh, def);
-    assert(nr->qh.jo.fa == nr->qh.jo.def->fa);
     assert_jo_tail(&nr->qh.jo, def);
 }
 
@@ -217,11 +197,6 @@ static void test_nr_defaults(void) {
     pico8_nr_t *nr = pico8_nr_new(NULL);
 
     assert(pico8_nr_ready());
-    assert_nr_own(nr);
-    assert(nr->qh.probe->od == pico8_nr_start.od);
-    assert(nr->qh.s == pico8_nr_start.s);
-    assert(nr->hz == pico8_nr_start.hz);
-    assert_same_string(nr->rr, pico8_nr_start.rr);
     assert_nr_inherited(nr, &pico8_nr_def);
 }
 
@@ -229,12 +204,6 @@ static void test_fr_defaults(void) {
     pico8_fr_t *fr = pico8_fr_new(NULL);
 
     assert(pico8_fr_ready());
-    assert(fr->mt == pico8_fr_start.mt);
-    assert_same_string(fr->nr.rr, pico8_fr_start.rr);
-    assert(fr->nr.qh.probe->od == pico8_fr_start.od);
-    assert(fr->nr.qh.s == pico8_fr_start.s);
-    assert(fr->nr.hz == pico8_fr_start.hz);
-    assert_nr_own(&fr->nr);
     assert_nr_inherited(&fr->nr, &pico8_fr_def);
 }
 
@@ -250,9 +219,7 @@ static void test_mx_defaults(void) {
     assert(mx->hj.qh.probe->fp == pico8_mx_def.probe.fp);
     assert(mx->hj.qh.probe->dw == pico8_mx_def.probe.dw);
     assert_qh_tail(&mx->hj.qh, &pico8_mx_def);
-    assert(mx->hj.qh.jo.fa == mx->hj.qh.jo.def->fa);
-    assert_same_box(mx->hj.qh.jo.kq, &mx->hj.qh.jo.def->kq);
-    assert_jo_box(mx->hj.qh.jo.kq);
+    assert_jo_box(&mx->hj.qh.jo.def->kq);
     assert_jo_tail(&mx->hj.qh.jo, &pico8_mx_def);
 }
 
diff --git a/tests/test_pico8_map_tables.c b/tests/test_pico8_map_tables.c
index 0415ee4..1a3172d 100644
--- a/tests/test_pico8_map_tables.c
+++ b/tests/test_pico8_map_tables.c
@@ -151,8 +151,8 @@ static void test_gn_pools_df_entries_by_their_r(void) {
  * tree shares a pointer with another string of the generated data. */
 static void test_jo_tree_strings_are_distinct_objects(void) {
     const char *const tree[] = {pico8_jo_def.me, pico8_qh_def.me, pico8_qh_def.probe.dq, pico8_nl_def.rr,
-        pico8_hj_def.rr, pico8_pm_def.fz, pico8_pm_def.me, pico8_ga_start.fz, pico8_nr_start.rr,
-        pico8_fr_start.rr, pico8_mx_def.rr};
+        pico8_hj_def.rr, pico8_pm_def.fz, pico8_pm_def.me, pico8_ga_def.fz, pico8_nr_def.rr,
+        pico8_fr_def.rr, pico8_mx_def.rr};
     const char *const others[] = {pico8_ke_start.me, pico8_mz_def.me, pico8_mz_def.probe.dq, pico8_jc_def.me,
         pico8_g__def.probe.dq, pico8_mz_jj_qs};
     int n = (int)(sizeof tree / sizeof tree[0]);
diff --git a/tests/test_pico8_mx.c b/tests/test_pico8_mx.c
index 80ea2a0..67f30ee 100644
--- a/tests/test_pico8_mx.c
+++ b/tests/test_pico8_mx.c
@@ -108,7 +108,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -322,7 +322,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, &jo->def->gq);
@@ -358,7 +358,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -396,7 +396,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(em->k_ != &pico8_hj_k__seq);
     assert(qh->jo.def->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_mx_def.kq);
+    assert(jo->def == &pico8_mx_def);
     assert(jo->def == &pico8_mx_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
diff --git a/tests/test_pico8_nl.c b/tests/test_pico8_nl.c
index 45a99d9..41b1148 100644
--- a/tests/test_pico8_nl.c
+++ b/tests/test_pico8_nl.c
@@ -103,7 +103,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -279,7 +279,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, &jo->def->gq);
@@ -303,7 +303,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -339,7 +339,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->jo.def->px == &pico8_nl_px_row && qh->jo.def->px != &pico8_qh_px_row);
     assert(qh->jo.def->ba == &pico8_nl_ba_seq && qh->jo.def->ba != &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_nl_def.kq);
+    assert(jo->def == &pico8_nl_def);
     assert(jo->def == &pico8_nl_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
diff --git a/tests/test_pico8_nr.c b/tests/test_pico8_nr.c
index 4105a66..fc9ca9e 100644
--- a/tests/test_pico8_nr.c
+++ b/tests/test_pico8_nr.c
@@ -105,7 +105,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -239,7 +239,7 @@ static int gate_kind(const void *e) {
     const void *hz = NULL;
 
     if (e != NULL && (cls_of(e) == pico8_nr_struct_class() || cls_of(e) == pico8_fr_struct_class())) {
-        hz = ((const pico8_nr_t *)e)->hz;
+        hz = ((const pico8_nr_t *)e)->qh.jo.def->hz;
     }
     if (hz == NULL) {
         return -1;
@@ -272,7 +272,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fold_string_member(h, jo->id);
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
-    h = fold_string_member(h, nr->rr);
+    h = fold_string_member(h, nr->qh.jo.def->rr);
     h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
@@ -283,16 +283,16 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, &jo->def->gq);
-    h = fnv(h, (uint32_t)nr->hz->count);
+    h = fnv(h, (uint32_t)nr->qh.jo.def->hz->count);
     /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
      * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
      * still proves the typed reads. */
-    h = fnv(h, (uint32_t)(nr->hz != NULL));
-    h = fnv(h, (uint32_t)(nr->rr != NULL));
+    h = fnv(h, (uint32_t)(nr->qh.jo.def->hz != NULL));
+    h = fnv(h, (uint32_t)(nr->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
     h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
@@ -307,7 +307,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -346,10 +346,10 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     assert(cls_of(e)->gv != qh_cls->gv && cls_of(e)->ldown != qh_cls->ldown && cls_of(e)->hb != qh_cls->hb);
     assert(cls_of(e)->gv != pico8_jo_struct_class()->gv && cls_of(e)->ldown != pico8_jo_struct_class()->ldown &&
            cls_of(e)->hb != pico8_jo_struct_class()->hb);
-    assert(jo->kq == &pico8_nr_start.kq);
-    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && nr->hz == &pico8_nr_hz_seq);
+    assert(jo->def == &pico8_nr_def);
+    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && nr->qh.jo.def->hz == &pico8_nr_hz_seq);
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
-    assert(jo->kq != &pico8_jo_def.kq);
+    assert(jo->def->kq.xl != pico8_jo_def.kq.xl);
     assert(qh->jo.def->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
     assert(jo->def == &pico8_nr_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
diff --git a/tests/test_pico8_os.c b/tests/test_pico8_os.c
index 31de6a1..bbad31b 100644
--- a/tests/test_pico8_os.c
+++ b/tests/test_pico8_os.c
@@ -108,7 +108,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -309,7 +309,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->probe->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, &jo->def->gq);
@@ -330,7 +330,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -364,7 +364,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(qh->jo.def->px == &pico8_os_px_row && qh->jo.def->px != &pico8_qh_px_row);
     assert(qh->jo.def->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_os_def.kq);
+    assert(jo->def == &pico8_os_def);
     assert(jo->def == &pico8_os_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
@@ -472,7 +472,7 @@ static uint32_t fold_db(uint32_t h, void *processor) {
     pico8_fixed_t os_db;
     pico8_fixed_t bare_db;
 
-    assert(oz->hf);
+    assert(oz->def->hf);
     bare->mi = from->has_mi ? from->mi : 0;
     bare->has_mi = from->has_mi;
     export_to(processor, oz);
diff --git a/tests/test_pico8_pm.c b/tests/test_pico8_pm.c
index 295b607..47d1bf3 100644
--- a/tests/test_pico8_pm.c
+++ b/tests/test_pico8_pm.c
@@ -100,7 +100,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -244,8 +244,8 @@ static int is_pm(const void *e) {
 }
 
 static int is_ga(const void *e) {
-    return e != NULL && cls_of(e) == pico8_ga_struct_class() && ((const pico8_ga_t *)e)->fz != NULL &&
-           strcmp(((const pico8_ga_t *)e)->fz, "ga") == 0;
+    return e != NULL && cls_of(e) == pico8_ga_struct_class() && ((const pico8_ga_t *)e)->jo.def->fz != NULL &&
+           strcmp(((const pico8_ga_t *)e)->jo.def->fz, "ga") == 0;
 }
 
 /* The `ga` that `pm:pq` spawned: registered in `hv` just before the
@@ -319,7 +319,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, &pm->hj.qh.jo.def->ng);
@@ -358,7 +358,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -397,7 +397,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(em->k_ != &pico8_hj_k__seq);
     assert(qh->jo.def->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_pm_def.kq);
+    assert(jo->def == &pico8_pm_def);
     assert(jo->def == &pico8_pm_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
diff --git a/tests/test_pico8_qh.c b/tests/test_pico8_qh.c
index 13ff10b..4a37eec 100644
--- a/tests/test_pico8_qh.c
+++ b/tests/test_pico8_qh.c
@@ -107,7 +107,7 @@ static uint32_t fold_string_member(uint32_t h, const char *s) {
 }
 
 static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
-    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
+    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
 }
 
 static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
@@ -276,7 +276,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->s);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_fixed_member(h, jo->fa);
+    h = fold_fixed_member(h, jo->def->fa);
     h = fold_fixed_member(h, jo->ma);
     h = fold_fixed_member(h, jo->def->ca);
     h = fold_vec(h, &jo->def->gq);
@@ -297,7 +297,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(jo->kq != NULL));
+    h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(jo->def->ka != NULL));
     h = fnv(h, (uint32_t)(jo->def->dz != NULL));
@@ -329,7 +329,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     assert(cls_of(e)->kf == pico8_qh_struct_class()->kf && cls_of(e)->cn == pico8_qh_struct_class()->cn &&
            cls_of(e)->mm == NULL && pico8_jo_is_family(e));
     assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->jo.def->px == &pico8_qh_px_row && qh->jo.def->ba == &pico8_qh_ba_seq);
-    assert(jo->kq == &pico8_qh_def.kq);
+    assert(jo->def == &pico8_qh_def);
     assert(jo->def == &pico8_qh_def);
     assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
            jo->def->cq == &pico8_jo_cq_seq);
```

#### Diff de 2.13e2b (`git diff e2a e2b`)

```diff
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index 92b6263..b373e0c 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -202,6 +202,8 @@ typedef struct {
     const pico8_jb_row_t *i_;
 } pico8_pj_def_t;
 
+/* `dq` NULL is nil. `fp` is what `d_` hands to `fm:nx`: the cart's false is
+ * held as 0, which is what `self.fp or 0` hands. */
 typedef struct {
     pico8_fixed_t dw;
     pico8_fixed_t od;
@@ -231,6 +233,7 @@ typedef struct {
     pico8_probe_def_t probe;
 } pico8_jc_def_t;
 
+/* `pk` is the cart's 1 or false, false held as 0: only its truth is read. */
 typedef struct {
     pico8_fixed_t pn;
     pico8_fixed_t du;
diff --git a/src/pico8_particle.c b/src/pico8_particle.c
index 5fab564..5b57568 100644
--- a/src/pico8_particle.c
+++ b/src/pico8_particle.c
@@ -146,7 +146,7 @@ void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *s
         }
     }
 
-    pico8_ez(self, emitter->fc);
+    pico8_ez(self, emitter->def->fc);
 }
 
 void pico8_ei_dispatch(void *self, const pico8_vec_t *ra) {
diff --git a/src/pico8_particle.h b/src/pico8_particle.h
index a423936..11c0c01 100644
--- a/src/pico8_particle.h
+++ b/src/pico8_particle.h
@@ -24,18 +24,15 @@ typedef struct pico8_qy {
 } pico8_qy_t;
 
 /* What `ei`, `ez`, `f_` and `g_:mm` read of their host, the particle
- * emitter's own fields, stored in the host (`emitter_own`) and never copied.
- * `qy` is NULL until `f_`, then owned by the host and never freed (TROU-7).
- * `pn`, `du`, `ki`, `pk` are read by `g_:mm` only: a `ga` holds 0 there.
- * `pk` is the cart's 1 or false, false held as 0: only its truth is read. */
+ * emitter's own fields, stored in the host (`emitter_own`) and never copied;
+ * the rest is read on `def`, the host's archetype's emitter. `qy` is NULL
+ * until `f_`, then owned by the host and never freed (TROU-7). `pn`, `du`
+ * are read by `g_:mm` only: a `ga` leaves them unwritten. */
 typedef struct pico8_emitter {
-    const pico8_emitter_def_t *def; /* NULL on ga: no generic reader may use it */
+    const pico8_emitter_def_t *def;
     pico8_fixed_t pn;
     pico8_fixed_t du;
     pico8_fixed_t py;
-    pico8_fixed_t ki;
-    pico8_fixed_t pk;
-    pico8_fixed_t fc;
     const pico8_int_seq_t *k_; /* NULL: absent, every particle drawn in colour 0 */
     pico8_fixed_t qo;
     pico8_fixed_t ro;
diff --git a/src/pico8_planet_display.c b/src/pico8_planet_display.c
index 0516c31..ed34f76 100644
--- a/src/pico8_planet_display.c
+++ b/src/pico8_planet_display.c
@@ -466,7 +466,7 @@ void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec
             int radius;
 
             if (self->fo->cls->probe_of != NULL) {
-                dw = self->fo->cls->probe_of(self->fo)->dw;
+                dw = self->fo->cls->probe_of(self->fo)->def->dw;
             }
             radius_f = pico8_fixed_mul(pico8_fixed_sqrt(dw), pico8_fixed_from_int(128));
             radius = pico8_fixed_flr_int(radius_f);
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index d58e0e5..65a0ce4 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -63,13 +63,8 @@ void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, con
     pico8_jo_struct_init(&self->jo, cls, def);
     self->probe = &self->probe_own;
     self->probe->def = &self->jo.def->probe;
-    self->probe->dq = self->jo.def->probe.dq;
     self->probe->od = self->jo.def->probe.od;
     self->probe->has_od = self->jo.def->probe.has_od;
-    self->probe->fp = self->jo.def->probe.fp;
-    self->probe->ik = self->jo.def->probe.ik;
-    self->probe->has_ik = self->jo.def->probe.has_ik;
-    self->probe->dw = self->jo.def->probe.dw;
     self->s = self->jo.def->s;
     self->probe->et = NULL;
 }
@@ -268,10 +263,7 @@ void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, con
     self->emitter = &self->emitter_own;
     self->emitter->def = &self->qh.jo.def->emitter;
     self->emitter->py = self->qh.jo.def->emitter.py;
-    self->emitter->ki = self->qh.jo.def->emitter.ki;
     self->emitter->pn = self->qh.jo.def->emitter.pn;
-    self->emitter->pk = self->qh.jo.def->emitter.pk;
-    self->emitter->fc = self->qh.jo.def->emitter.fc;
     self->emitter->k_ = self->qh.jo.def->emitter.k_;
     self->emitter->du = self->qh.jo.def->emitter.du;
     self->emitter->qo = self->qh.jo.def->emitter.qo;
@@ -524,7 +516,6 @@ static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
     pico8_jo_struct_init(&self->jo, &g_ga_struct_class, &pico8_ga_def);
     self->emitter = &self->emitter_own;
     self->emitter->def = &self->jo.def->emitter;
-    self->emitter->fc = self->jo.def->emitter.fc;
     self->emitter->qo = self->jo.def->emitter.qo;
     self->emitter->ro = self->jo.def->emitter.ro;
     self->emitter->qg = self->jo.def->emitter.qg;
diff --git a/src/pico8_planet_family.h b/src/pico8_planet_family.h
index fa71fe0..ea30bf0 100644
--- a/src/pico8_planet_family.h
+++ b/src/pico8_planet_family.h
@@ -70,9 +70,9 @@ typedef struct {
 
 /* A particle burst: a bare planet, then `iu`, `ms` and an emitter copied
  * from its archetype, with `qy`, which its own `pq` (`f_`) writes; the
- * emitter's `pn`, `du`, `ki`, `pk` are left unwritten, read by `g_:mm`
- * only. `jo.by` is the one list every burst shares, which `ib` extends in
- * place and each init empties. A burst spawned by `pm:pq` takes `iu`, `ms`,
+ * emitter's `pn`, `du` are left unwritten, read by `g_:mm` only. `jo.by`
+ * is the one list every burst shares, which `ib` extends in place and each
+ * init empties. A burst spawned by `pm:pq` takes `iu`, `ms`,
  * `py`, `k_`, `qo`, `ro`, `qg`, `q_` from its keys instead. `emitter` points
  * at `emitter_own`: never copy the struct. Nothing frees the struct:
  * TROU-7. */
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index 5213568..097665e 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -165,7 +165,7 @@ void pico8_bv(pico8_struct_header_t *self, pico8_vec_t *mp, pico8_vec_t *rp) {
 
     et = NULL;
 
-    if (nq < probe->dw && c != NULL) {
+    if (nq < probe->def->dw && c != NULL) {
         et = c;
     } else {
         pico8_bv_ctx_t ctx;
@@ -330,11 +330,11 @@ int pico8_d_(pico8_struct_header_t *self) {
     assert((!has_et || !lu_ok) && "d_: fm:lu must stay short-circuited when et is set");
 
     if (!has_et && lu_ok) {
-        pico8_ke_ks(self, probe->dq != NULL ? probe->dq : "ph");
+        pico8_ke_ks(self, probe->def->dq != NULL ? probe->def->dq : "ph");
 
-        pico8_hy_nx(pico8_fm_hy(), probe->fp);
+        pico8_hy_nx(pico8_fm_hy(), probe->def->fp);
 
-        pico8_rn(probe->has_ik ? probe->ik : pico8_fixed_from_int(54));
+        pico8_rn(probe->def->has_ik ? probe->def->ik : pico8_fixed_from_int(54));
 
         return 1;
     }
@@ -365,8 +365,8 @@ pico8_qy_t *pico8_g__mm(void *self_ptr) {
         t = self->t;
         du = emitter->du;
         py = emitter->py;
-        ki = emitter->ki;
-        pk = emitter->pk != 0;
+        ki = emitter->def->ki;
+        pk = emitter->def->pk != 0;
     }
 
     /* Two statements, not two arguments: C leaves argument order unspecified
@@ -449,18 +449,10 @@ void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, con
     self->probe->et = NULL;
     self->p_ = 0;
     self->has_p_ = 0;
-    self->probe->dq = self->def->probe.dq;
     self->probe->od = self->def->probe.od;
     self->probe->has_od = self->def->probe.has_od;
-    self->probe->fp = self->def->probe.fp;
-    self->probe->ik = self->def->probe.ik;
-    self->probe->has_ik = self->def->probe.has_ik;
-    self->probe->dw = self->def->probe.dw;
     self->emitter->py = self->def->emitter.py;
-    self->emitter->ki = self->def->emitter.ki;
     self->emitter->pn = self->def->emitter.pn;
-    self->emitter->pk = self->def->emitter.pk;
-    self->emitter->fc = self->def->emitter.fc;
     self->emitter->k_ = self->def->emitter.k_;
     self->emitter->qo = self->def->emitter.qo;
     self->emitter->ro = self->def->emitter.ro;
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index d6be829..bb3e989 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -17,22 +17,15 @@
  * `gfx`/`mem` skips the draw calls while the rest of the body still runs. */
 
 /* What `bv`, `dl` and `d_` read and write of their host, the probe's own
- * fields, stored in the host (`probe_own`) and never copied. `et` is
- * borrowed; NULL is nil or false, which no reader tells apart. `dq` NULL is
- * nil. `fp` is what `d_` hands to `fm:nx`: the cart's false is held as 0,
- * which is what `self.fp or 0` hands. */
+ * fields, stored in the host (`probe_own`) and never copied; the rest is
+ * read on `def`, the host's archetype's probe (`pico8_probe_def_none` on
+ * research_icon's `pb`). `et` is borrowed; NULL is nil or false, which no
+ * reader tells apart. */
 typedef struct pico8_probe {
-    /* `pico8_probe_def_none` on research_icon's `pb`; no generic reader may
-     * use it. */
     const pico8_probe_def_t *def;
     pico8_struct_header_t *et;
-    pico8_fixed_t dw;
     pico8_fixed_t od;
     int has_od; /* 0: `od` is nil, and is then never read */
-    const char *dq;
-    pico8_fixed_t fp;
-    pico8_fixed_t ik;
-    int has_ik; /* 0: `ik` is nil, and is then never read */
 } pico8_probe_t;
 
 /* TROU-7 */
diff --git a/src/pico8_research_icon.h b/src/pico8_research_icon.h
index 7b8caf3..7ba338f 100644
--- a/src/pico8_research_icon.h
+++ b/src/pico8_research_icon.h
@@ -20,9 +20,8 @@
  * pointer. */
 
 /* `ra` starts on the archetype's, shared by every `ie`; `hb` rebinds it to
- * the cursor. `gy` and `id` are no members: the
- * module's shared state, read by pico8_ie_gy()/pico8_ie_id(). Nothing frees
- * the struct: TROU-7. */
+ * the cursor. `gy` and `id` are no members: the module's shared state, read
+ * by pico8_ie_gy()/pico8_ie_id(). Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
     const pico8_ie_def_t *def;
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index 05c8d81..2566224 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -401,11 +401,6 @@ static pico8_jc_t *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
     self->fa = self->def->fa;
     self->probe = &self->probe_own;
     self->probe->def = &self->def->probe;
-    self->probe->dq = self->def->probe.dq;
-    self->probe->fp = self->def->probe.fp;
-    self->probe->ik = self->def->probe.ik;
-    self->probe->has_ik = self->def->probe.has_ik;
-    self->probe->dw = self->def->probe.dw;
     self->nh = self->def->nh;
     self->qu = NULL;
     self->ts = NULL;
diff --git a/src/pico8_slipway.h b/src/pico8_slipway.h
index 93ff90e..cbda0c0 100644
--- a/src/pico8_slipway.h
+++ b/src/pico8_slipway.h
@@ -30,9 +30,8 @@ void pico8_jc_hg_add(pico8_jc_hg_t *hg, const char *id);
 int pico8_jc_hg_has(const pico8_jc_hg_t *hg, const char *id);
 
 /* `ts` is this slipway's own, made by `pico8_jc_pq`, which also empties
- * `hg_own` and points `hg` at it (NULL before). `fp`, `ik`, `dw`, `od` and
- * `et` (NULL is nil) live in the probe; `probe` points at `probe_own`: never
- * copy the struct. Nothing frees the struct: TROU-7. */
+ * `hg_own` and points `hg` at it (NULL before). `od` and `et` (NULL is nil)
+ * live in the probe; `probe` points at `probe_own`: never copy the struct. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
     const pico8_jc_def_t *def;
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index 17186ff..cd38d62 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -244,14 +244,9 @@ static pico8_mz_t *pico8_mz_struct_new(void) {
     self->header.me = self->def->me;
     self->probe = &self->probe_own;
     self->probe->def = &self->def->probe;
-    self->probe->dw = self->def->probe.dw;
-    self->probe->dq = self->def->probe.dq;
-    self->probe->ik = self->def->probe.ik;
-    self->probe->has_ik = self->def->probe.has_ik;
     self->oi = self->def->oi;
     self->probe->od = self->def->probe.od;
     self->probe->has_od = self->def->probe.has_od;
-    self->probe->fp = self->def->probe.fp;
     self->jj = &g_mz_jj_wrapper;
     self->ox = NULL;
     self->ra = NULL;
diff --git a/src/pico8_wormhole.h b/src/pico8_wormhole.h
index f942967..9f0a06c 100644
--- a/src/pico8_wormhole.h
+++ b/src/pico8_wormhole.h
@@ -23,9 +23,8 @@ typedef struct {
 } pico8_mz_jj_t;
 
 /* `jj` points at the one `pico8_mz_jj_t`, shared by every `mz` and never
- * modified in place. `dw`, `dq`, `ik`, `od`, `fp` and `et` live in the probe;
- * `probe` points at `probe_own`: never copy the struct. Nothing frees the
- * struct: TROU-7. */
+ * modified in place. `od` and `et` live in the probe; `probe` points at
+ * `probe_own`: never copy the struct. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
     const pico8_mz_def_t *def;
diff --git a/tests/test_pico8_fr.c b/tests/test_pico8_fr.c
index 3ad5770..b99fbdb 100644
--- a/tests/test_pico8_fr.c
+++ b/tests/test_pico8_fr.c
@@ -260,9 +260,9 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
-    h = fold_fixed_member(h, qh->probe->fp);
-    h = fold_fixed_member(h, qh->probe->dw);
-    h = fold_string_member(h, qh->probe->dq);
+    h = fold_fixed_member(h, qh->probe->def->fp);
+    h = fold_fixed_member(h, qh->probe->def->dw);
+    h = fold_string_member(h, qh->probe->def->dq);
     h = fold_qh(h, qh);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
@@ -280,7 +280,7 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fnv(h, (uint32_t)(nr->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
diff --git a/tests/test_pico8_fv.c b/tests/test_pico8_fv.c
index e9bcec6..f95c8fc 100644
--- a/tests/test_pico8_fv.c
+++ b/tests/test_pico8_fv.c
@@ -309,11 +309,11 @@ static void test_built(void) {
     hv = pico8_hv();
 
     assert(g_->def->fa == pico8_fixed_from_int(10));
-    assert(em->ki == pico8_fixed_from_int(17));
+    assert(em->def->ki == pico8_fixed_from_int(17));
     assert(g_->def->ca == pico8_fixed_from_int(11));
     assert(g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(g_->ra != NULL && em->k_ != NULL);
-    assert(g_->def->rb != NULL && pr->dq != NULL);
+    assert(g_->def->rb != NULL && pr->def->dq != NULL);
     h = fold_fixed_member(h, g_->def->fa);
     h = fold_ref_member(h, g_->ra);
     h = fold_ref_member(h, &g_->def->hi);
@@ -321,7 +321,7 @@ static void test_built(void) {
     h = fold_ref_member(h, &g_->def->gq);
     h = fold_fixed_member(h, g_->def->eo);
     h = fold_fixed_member(h, g_->def->ff);
-    h = fold_fixed_member(h, em->fc);
+    h = fold_fixed_member(h, em->def->fc);
     h = fold_ref_member(h, em->k_);
     h = fold_fixed_member(h, em->qo);
     h = fold_fixed_member(h, em->ro);
@@ -330,18 +330,18 @@ static void test_built(void) {
     h = fold_fixed_member(h, em->pn);
     h = fold_fixed_member(h, em->du);
     h = fold_fixed_member(h, em->py);
-    h = fold_fixed_member(h, em->pk);
-    h = fold_fixed_member(h, em->ki);
+    h = fold_fixed_member(h, em->def->pk);
+    h = fold_fixed_member(h, em->def->ki);
     h = fold_ref_member(h, &g_->def->kq);
     h = fold_fixed_member(h, g_->def->ql);
     h = fold_fixed_member(h, g_->def->fg);
     h = fold_ref_member(h, g_->def->rb);
     h = fold_tags(fnv(fnv(h, 1u), 0x7Au), g_->header.cls->tags);
     h = fold_fixed_member(h, g_->def->fx);
-    h = fold_string_member(h, pr->dq);
+    h = fold_string_member(h, pr->def->dq);
     h = fold_fixed_member(h, pr->od);
-    h = fold_bool_member(h, pr->fp != 0);
-    h = fold_fixed_member(h, pr->dw);
+    h = fold_bool_member(h, pr->def->fp != 0);
+    h = fold_fixed_member(h, pr->def->dw);
     h = fold_vec(h, g_->ra);
     h = fold_vec(h, &g_->def->hi);
     h = fold_vec(h, &g_->def->gq);
diff --git a/tests/test_pico8_g_.c b/tests/test_pico8_g_.c
index 6a7d93b..04d68be 100644
--- a/tests/test_pico8_g_.c
+++ b/tests/test_pico8_g_.c
@@ -175,16 +175,16 @@ static uint32_t fold_defaults(uint32_t h, const pico8_g__t *g_) {
 
     h = fold_ref_member(h, &g_->header.cls->tags);
     h = fold_fixed_member(h, g_->def->fx);
-    h = fold_string_member(h, pr->dq);
+    h = fold_string_member(h, pr->def->dq);
     h = fold_fixed_member(h, pr->od);
-    h = fold_bool_member(h, pr->fp != 0);
+    h = fold_bool_member(h, pr->def->fp != 0);
     h = fold_fixed_member(h, g_->def->fa);
-    h = fold_fixed_member(h, pr->dw);
+    h = fold_fixed_member(h, pr->def->dw);
     h = fold_fixed_member(h, em->py);
-    h = fold_fixed_member(h, em->ki);
+    h = fold_fixed_member(h, em->def->ki);
     h = fold_fixed_member(h, em->pn);
-    h = fold_fixed_member(h, em->pk);
-    h = fold_fixed_member(h, em->fc);
+    h = fold_fixed_member(h, em->def->pk);
+    h = fold_fixed_member(h, em->def->fc);
     h = fold_ref_member(h, em->k_);
     h = fold_fixed_member(h, em->qo);
     h = fold_fixed_member(h, em->ro);
@@ -349,16 +349,16 @@ static uint32_t fold_probe(uint32_t h, pico8_g__t *e) {
     assert(is_probe(&e->header));
     assert(e->header.cls->get_p(e) == e->p);
     h = fold_fixed_member(h, e->def->fx);
-    h = fold_string_member(h, pr->dq);
+    h = fold_string_member(h, pr->def->dq);
     h = fold_fixed_member(h, pr->od);
-    h = fold_bool_member(h, pr->fp != 0);
+    h = fold_bool_member(h, pr->def->fp != 0);
     h = fold_fixed_member(h, e->def->fa);
-    h = fold_fixed_member(h, pr->dw);
+    h = fold_fixed_member(h, pr->def->dw);
     h = fold_fixed_member(h, em->py);
-    h = fold_fixed_member(h, em->ki);
+    h = fold_fixed_member(h, em->def->ki);
     h = fold_fixed_member(h, em->pn);
-    h = fold_fixed_member(h, em->pk);
-    h = fold_fixed_member(h, em->fc);
+    h = fold_fixed_member(h, em->def->pk);
+    h = fold_fixed_member(h, em->def->fc);
     h = fold_fixed_member(h, em->qo);
     h = fold_fixed_member(h, em->ro);
     h = fold_fixed_member(h, em->qg);
@@ -366,7 +366,7 @@ static uint32_t fold_probe(uint32_t h, pico8_g__t *e) {
     h = fold_fixed_member(h, em->du);
     h = fold_string_member(h, e->header.me);
     h = fold_fixed_member(h, e->header.t);
-    assert(pr->dq != NULL && em->k_ != NULL);
+    assert(pr->def->dq != NULL && em->k_ != NULL);
     h = fold_tags(h, e->header.cls->tags);
     h = fold_seq(h, em->k_);
     assert(e->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)));
@@ -409,8 +409,8 @@ static void test_built(void) {
     h = fold_probe(h, c);
     assert(a != b && b != c && a != c);
     assert(a->emitter->qy != b->emitter->qy && b->emitter->qy != c->emitter->qy);
-    assert(a->def->fa == pico8_fixed_from_int(2) && a->emitter->ki == pico8_fixed_from_int(9));
-    assert(a->probe->dw == 0x666);
+    assert(a->def->fa == pico8_fixed_from_int(2) && a->emitter->def->ki == pico8_fixed_from_int(9));
+    assert(a->probe->def->dw == 0x666);
     assert(c->emitter->pn == pico8_fixed_from_int(7) && c->header.t == pico8_fixed_from_int(3) &&
            c->emitter->du == pico8_fixed_from_int(10) && c->emitter->py == pico8_fixed_from_int(1));
     assert(b->emitter->pn == pico8_fixed_from_int(3) && b->header.t == pico8_fixed_from_int(0));
@@ -689,7 +689,7 @@ static void test_fv(void) {
     fv = pico8_g__of(found);
     assert(fv->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)));
     assert(fv->def->fx == pico8_fixed_from_int(1));
-    assert(fv->probe->dq != NULL && strcmp(fv->probe->dq, "ly") == 0);
+    assert(fv->probe->def->dq != NULL && strcmp(fv->probe->def->dq, "ly") == 0);
     assert(fv->def->fa == pico8_fixed_from_int(10));
     assert(fv->emitter->k_ == &pico8_fv_k__seq);
     assert(fv->emitter->k_ != &pico8_g__k__seq);
diff --git a/tests/test_pico8_ga.c b/tests/test_pico8_ga.c
index c356dee..7aafbb8 100644
--- a/tests/test_pico8_ga.c
+++ b/tests/test_pico8_ga.c
@@ -299,7 +299,7 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     assert(is_ga(e));
     h = fold_string_member(h, g->jo.def->fz);
     h = fold_fixed_member(h, jo->def->fa);
-    h = fold_fixed_member(h, em->fc);
+    h = fold_fixed_member(h, em->def->fc);
     h = fold_fixed_member(h, em->qo);
     h = fold_fixed_member(h, em->ro);
     h = fold_fixed_member(h, em->qg);
diff --git a/tests/test_pico8_hj.c b/tests/test_pico8_hj.c
index a5de5eb..67ba7bd 100644
--- a/tests/test_pico8_hj.c
+++ b/tests/test_pico8_hj.c
@@ -187,8 +187,8 @@ static pico8_hj_t *as_hj(void *e) {
 }
 
 static uint32_t fold_pk(uint32_t h, const pico8_hj_t *hj) {
-    return hj->qh.jo.header.cls == pico8_pm_struct_class() ? fold_bool_member(h, hj->emitter->pk != 0)
-                                                           : fold_fixed_member(h, hj->emitter->pk);
+    return hj->qh.jo.header.cls == pico8_pm_struct_class() ? fold_bool_member(h, hj->emitter->def->pk != 0)
+                                                           : fold_fixed_member(h, hj->emitter->def->pk);
 }
 
 /* A particle record as `g_.mm` returns it: `p`, `v`, `l`. */
@@ -316,18 +316,18 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, em->py);
-    h = fold_fixed_member(h, em->ki);
+    h = fold_fixed_member(h, em->def->ki);
     h = fold_fixed_member(h, em->pn);
     h = fold_pk(h, hj);
-    h = fold_fixed_member(h, em->fc);
+    h = fold_fixed_member(h, em->def->fc);
     h = fold_fixed_member(h, em->du);
     h = fold_fixed_member(h, em->qo);
     h = fold_fixed_member(h, em->ro);
     h = fold_fixed_member(h, em->qg);
     h = fold_fixed_member(h, em->q_);
-    h = fold_string_member(h, qh->probe->dq);
-    h = fold_fixed_member(h, qh->probe->fp);
-    h = fold_fixed_member(h, qh->probe->dw);
+    h = fold_string_member(h, qh->probe->def->dq);
+    h = fold_fixed_member(h, qh->probe->def->fp);
+    h = fold_fixed_member(h, qh->probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
@@ -352,7 +352,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -587,7 +587,7 @@ static void test_heirs(void) {
     keys.id = "s77";
     pm = pico8_pm_new(&keys);
 
-    assert(strcmp(as_hj(pm)->qh.jo.def->rr, "ascension") == 0 && as_hj(pm)->emitter->pk == 0);
+    assert(strcmp(as_hj(pm)->qh.jo.def->rr, "ascension") == 0 && as_hj(pm)->emitter->def->pk == 0);
     assert(as_hj(pm)->qh.probe->od == pico8_fixed_from_int(60));
     assert(strcmp(as_hj(mx)->qh.jo.def->rr, "void synthesis") == 0);
     h = fold_heir(h, pm, pico8_pm_struct_class());
diff --git a/tests/test_pico8_jc.c b/tests/test_pico8_jc.c
index 3f969e7..28e65e1 100644
--- a/tests/test_pico8_jc.c
+++ b/tests/test_pico8_jc.c
@@ -445,10 +445,10 @@ static void check_fresh_jc(const void *e, const void *qu) {
     assert(jc->header.me != NULL && strcmp(jc->header.me, "kz") == 0);
     assert(jc->header.t == 0);
     assert(jc->fa == pico8_fixed_from_int(2));
-    assert(jc->probe->fp == 0);
-    assert(jc->probe->ik == pico8_fixed_from_int(48) && jc->probe->has_ik == 1);
-    printf("[dw] 0x%08X\n", (unsigned)jc->probe->dw);
-    assert(jc->probe->dw == (pico8_fixed_t)DW_BITS);
+    assert(jc->probe->def->fp == 0);
+    assert(jc->probe->def->ik == pico8_fixed_from_int(48) && jc->probe->def->has_ik == 1);
+    printf("[dw] 0x%08X\n", (unsigned)jc->probe->def->dw);
+    assert(jc->probe->def->dw == (pico8_fixed_t)DW_BITS);
     assert(jc->nh == pico8_fixed_from_int(20));
 
     k_ = jc->def->k_;
@@ -836,9 +836,24 @@ static void test_ce_crosses_a_slipway(void) {
     assert(crossed);
 }
 
-/* A probe over the slipway targets it and hangs `fo` on it. Its `dw` is
- * zeroed, as `mz`'s is: every point of this short slipway lies within a
- * `g_`'s `dw` of one of its own planets, which would win. */
+/* g_'s archetype with the probe's `dw` zeroed, as `mz`'s is: every point of
+ * this short slipway lies within a `g_`'s `dw` of one of its own planets,
+ * which would win. */
+static pico8_g__def_t g_dw_zero;
+
+/* A `g_` built as `pico8_g__new(NULL)` builds one, on `g_dw_zero`. */
+static pico8_g__t *probe_with_zero_dw(void) {
+    pico8_g__t *probe = malloc(sizeof *probe);
+
+    g_dw_zero = pico8_g__def;
+    g_dw_zero.probe.dw = 0;
+    pico8_g__struct_init(probe, pico8_g__struct_class(), &g_dw_zero);
+    pico8_f_(&probe->header);
+    pico8_ke_struct_pq(&probe->header);
+    return probe;
+}
+
+/* A probe over the slipway targets it and hangs `fo` on it. */
 static void test_bv(void) {
     uint32_t h = 2166136261u;
     void *old;
@@ -849,9 +864,7 @@ static void test_bv(void) {
 
     world();
     old = the_slipway();
-    probe = pico8_g__new(NULL);
-    assert(probe != NULL);
-    ((pico8_g__t *)probe)->probe->dw = 0;
+    probe = probe_with_zero_dw();
     qu = pos_of(as_jc(old)->qu);
     re = pos_of(as_jc(old)->re);
     for (k = 1; k < 8; k++) {
@@ -890,8 +903,7 @@ static void test_bv_takes_the_first_slipway_of_cg(void) {
     as_jc(twin)->re = as_jc(old)->qu;
     assert(pico8_cg("jc")->count == 2 && pico8_cg("jc")->items[0] == old &&
            pico8_cg("jc")->items[1] == twin);
-    probe = pico8_g__new(NULL);
-    ((pico8_g__t *)probe)->probe->dw = 0;
+    probe = probe_with_zero_dw();
     qu = pos_of(as_jc(old)->qu);
     re = pos_of(as_jc(old)->re);
     at = pico8_vec_div(pico8_vec_add(qu, re), pico8_fixed_from_int(2));
diff --git a/tests/test_pico8_jo_spec_defaults.c b/tests/test_pico8_jo_spec_defaults.c
index b2dc19c..98fdf15 100644
--- a/tests/test_pico8_jo_spec_defaults.c
+++ b/tests/test_pico8_jo_spec_defaults.c
@@ -54,16 +54,14 @@ static void assert_jo_box(const pico8_qz_box_t *kq) {
  * the probe's init literals. */
 static void assert_qh_tail(const pico8_qh_t *qh, const pico8_jo_def_t *def) {
     assert(qh->probe->def == &def->probe);
-    assert_same_string(qh->probe->dq, def->probe.dq);
     assert(qh->jo.mi == def->mi && qh->jo.has_mi == 1);
-    assert(qh->probe->has_od == 1 && qh->probe->ik == 0 && qh->probe->has_ik == 0);
+    assert(qh->probe->has_od == 1 && qh->probe->def->ik == 0 && qh->probe->def->has_ik == 0);
 }
 
 /* The emitter of an ascension, pm or mx, and its archetype. */
 static void assert_emitter(const pico8_emitter_t *em, const pico8_jo_def_t *def) {
     assert(em->def == &def->emitter);
-    assert(em->py == def->emitter.py && em->ki == def->emitter.ki && em->pn == def->emitter.pn);
-    assert(em->pk == def->emitter.pk && em->fc == def->emitter.fc && em->k_ == def->emitter.k_);
+    assert(em->py == def->emitter.py && em->pn == def->emitter.pn && em->k_ == def->emitter.k_);
     assert(em->du == def->emitter.du && em->qo == def->emitter.qo && em->ro == def->emitter.ro);
     assert(em->qg == def->emitter.qg && em->q_ == def->emitter.q_);
 }
@@ -88,8 +86,6 @@ static void test_qh_defaults(void) {
     assert(pico8_qh_ready());
     assert_same_string(qh->jo.header.me, pico8_qh_def.me);
     assert(qh->probe->od == pico8_qh_def.probe.od);
-    assert(qh->probe->fp == pico8_qh_def.probe.fp);
-    assert(qh->probe->dw == pico8_qh_def.probe.dw);
     assert(qh->s == pico8_qh_def.s);
     assert_qh_tail(qh, &pico8_qh_def);
     assert_jo_box(&qh->jo.def->kq);
@@ -104,8 +100,6 @@ static void test_nl_defaults(void) {
     assert(nl->qh.probe->od == pico8_nl_def.probe.od);
     assert(nl->qw == pico8_nl_def.qw);
     assert_same_string(nl->qh.jo.header.me, pico8_nl_def.me);
-    assert(nl->qh.probe->fp == pico8_nl_def.probe.fp);
-    assert(nl->qh.probe->dw == pico8_nl_def.probe.dw);
     assert_qh_tail(&nl->qh, &pico8_nl_def);
     assert_jo_box(&nl->qh.jo.def->kq);
     assert_jo_tail(&nl->qh.jo, &pico8_nl_def);
@@ -118,8 +112,6 @@ static void test_os_defaults(void) {
     assert(os->qh.s == pico8_os_def.s);
     assert(os->qh.probe->od == pico8_os_def.probe.od);
     assert_same_string(os->qh.jo.header.me, pico8_os_def.me);
-    assert(os->qh.probe->fp == pico8_os_def.probe.fp);
-    assert(os->qh.probe->dw == pico8_os_def.probe.dw);
     assert_qh_tail(&os->qh, &pico8_os_def);
     assert_jo_box(&os->qh.jo.def->kq);
     assert_jo_tail(&os->qh.jo, &pico8_os_def);
@@ -133,8 +125,6 @@ static void test_hj_defaults(void) {
     assert(hj->qh.s == pico8_hj_def.s);
     assert_emitter(hj->emitter, &pico8_hj_def);
     assert_same_string(hj->qh.jo.header.me, pico8_hj_def.me);
-    assert(hj->qh.probe->fp == pico8_hj_def.probe.fp);
-    assert(hj->qh.probe->dw == pico8_hj_def.probe.dw);
     assert_qh_tail(&hj->qh, &pico8_hj_def);
     assert_jo_box(&hj->qh.jo.def->kq);
     assert_jo_tail(&hj->qh.jo, &pico8_hj_def);
@@ -148,8 +138,6 @@ static void test_pm_defaults(void) {
     assert(pm->hj.qh.s == pico8_pm_def.s);
     assert_emitter(pm->hj.emitter, &pico8_pm_def);
     assert(pm->hj.qh.probe->od == pico8_pm_def.probe.od);
-    assert(pm->hj.qh.probe->fp == pico8_pm_def.probe.fp);
-    assert(pm->hj.qh.probe->dw == pico8_pm_def.probe.dw);
     assert_qh_tail(&pm->hj.qh, &pico8_pm_def);
     assert_jo_box(&pm->hj.qh.jo.def->kq);
     assert_jo_tail(&pm->hj.qh.jo, &pico8_pm_def);
@@ -162,7 +150,6 @@ static void test_ga_defaults(void) {
     assert(pico8_ga_ready());
     assert(ga->jo.by != NULL && ga->jo.by == other->jo.by);
     assert(ga->emitter->def == &pico8_ga_def.emitter);
-    assert(ga->emitter->fc == pico8_ga_def.emitter.fc);
     assert(ga->emitter->qo == pico8_ga_def.emitter.qo);
     assert(ga->emitter->ro == pico8_ga_def.emitter.ro);
     assert(ga->emitter->qg == pico8_ga_def.emitter.qg);
@@ -183,8 +170,6 @@ static void test_ga_defaults(void) {
 
 /* What nr and fr inherit from qh and jo, and nr's box, which fr keeps. */
 static void assert_nr_inherited(const pico8_nr_t *nr, const pico8_jo_def_t *def) {
-    assert(nr->qh.probe->fp == def->probe.fp);
-    assert(nr->qh.probe->dw == def->probe.dw);
     assert(nr->qh.probe->od == def->probe.od);
     assert(nr->qh.s == def->s);
     assert_box(&nr->qh.jo.def->kq, -4, -4, 5, 5);
@@ -216,8 +201,6 @@ static void test_mx_defaults(void) {
     assert(mx->ht == pico8_mx_def.ht);
     assert_emitter(mx->hj.emitter, &pico8_mx_def);
     assert_same_string(mx->hj.qh.jo.header.me, pico8_mx_def.me);
-    assert(mx->hj.qh.probe->fp == pico8_mx_def.probe.fp);
-    assert(mx->hj.qh.probe->dw == pico8_mx_def.probe.dw);
     assert_qh_tail(&mx->hj.qh, &pico8_mx_def);
     assert_jo_box(&mx->hj.qh.jo.def->kq);
     assert_jo_tail(&mx->hj.qh.jo, &pico8_mx_def);
diff --git a/tests/test_pico8_mx.c b/tests/test_pico8_mx.c
index 67f30ee..10e5606 100644
--- a/tests/test_pico8_mx.c
+++ b/tests/test_pico8_mx.c
@@ -161,7 +161,7 @@ static pico8_hj_t *as_hj(void *e) {
 }
 
 static uint32_t fold_pk(uint32_t h, const pico8_hj_t *hj) {
-    return fold_fixed_member(h, hj->emitter->pk);
+    return fold_fixed_member(h, hj->emitter->def->pk);
 }
 
 /* A particle record as `g_.mm` returns it: `p`, `v`, `l`. */
@@ -307,18 +307,18 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, em->py);
-    h = fold_fixed_member(h, em->ki);
+    h = fold_fixed_member(h, em->def->ki);
     h = fold_fixed_member(h, em->pn);
     h = fold_pk(h, hj);
-    h = fold_fixed_member(h, em->fc);
+    h = fold_fixed_member(h, em->def->fc);
     h = fold_fixed_member(h, em->du);
     h = fold_fixed_member(h, em->qo);
     h = fold_fixed_member(h, em->ro);
     h = fold_fixed_member(h, em->qg);
     h = fold_fixed_member(h, em->q_);
-    h = fold_string_member(h, qh->probe->dq);
-    h = fold_fixed_member(h, qh->probe->fp);
-    h = fold_fixed_member(h, qh->probe->dw);
+    h = fold_string_member(h, qh->probe->def->dq);
+    h = fold_fixed_member(h, qh->probe->def->fp);
+    h = fold_fixed_member(h, qh->probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
@@ -346,7 +346,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(em->qy != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
diff --git a/tests/test_pico8_mz.c b/tests/test_pico8_mz.c
index f6e52b4..357a5a1 100644
--- a/tests/test_pico8_mz.c
+++ b/tests/test_pico8_mz.c
@@ -306,13 +306,13 @@ static void check_fresh_mz(void *e) {
 
     assert(is_me(&mz->header, "kz"));
     assert(mz->header.t == 0);
-    assert(mz->probe->dw == 0);
-    assert(mz->probe->dq != NULL && strcmp(mz->probe->dq, "pr") == 0);
+    assert(mz->probe->def->dw == 0);
+    assert(mz->probe->def->dq != NULL && strcmp(mz->probe->def->dq, "pr") == 0);
     assert(mz->def->spawn_snd == pico8_fixed_from_int(52));
-    assert(mz->probe->ik == pico8_fixed_from_int(49) && mz->probe->has_ik == 1);
+    assert(mz->probe->def->ik == pico8_fixed_from_int(49) && mz->probe->def->has_ik == 1);
     assert(mz->oi == 0);
     assert(mz->probe->od == pico8_fixed_from_int(3));
-    assert(mz->probe->fp == pico8_fixed_from_int(1));
+    assert(mz->probe->def->fp == pico8_fixed_from_int(1));
     assert(mz->def->fa == pico8_fixed_from_int(5));
     assert(mz->header.cls->get_fa(mz, &fa) && fa == mz->def->fa);
 
diff --git a/tests/test_pico8_nl.c b/tests/test_pico8_nl.c
index 41b1148..5b7144b 100644
--- a/tests/test_pico8_nl.c
+++ b/tests/test_pico8_nl.c
@@ -273,9 +273,9 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, nl->qw);
-    h = fold_string_member(h, qh->probe->dq);
-    h = fold_fixed_member(h, qh->probe->fp);
-    h = fold_fixed_member(h, qh->probe->dw);
+    h = fold_string_member(h, qh->probe->def->dq);
+    h = fold_fixed_member(h, qh->probe->def->fp);
+    h = fold_fixed_member(h, qh->probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
@@ -291,7 +291,7 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
diff --git a/tests/test_pico8_nr.c b/tests/test_pico8_nr.c
index fc9ca9e..1addfdb 100644
--- a/tests/test_pico8_nr.c
+++ b/tests/test_pico8_nr.c
@@ -276,9 +276,9 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fold_fixed_member(h, qh->jo.def->hp);
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
-    h = fold_fixed_member(h, qh->probe->fp);
-    h = fold_fixed_member(h, qh->probe->dw);
-    h = fold_string_member(h, qh->probe->dq);
+    h = fold_fixed_member(h, qh->probe->def->fp);
+    h = fold_fixed_member(h, qh->probe->def->dw);
+    h = fold_string_member(h, qh->probe->def->dq);
     h = fold_qh(h, qh);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
@@ -295,7 +295,7 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(nr->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
diff --git a/tests/test_pico8_os.c b/tests/test_pico8_os.c
index bbad31b..14ad3f4 100644
--- a/tests/test_pico8_os.c
+++ b/tests/test_pico8_os.c
@@ -304,9 +304,9 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_string_member(h, qh->probe->dq);
-    h = fold_fixed_member(h, qh->probe->fp);
-    h = fold_fixed_member(h, qh->probe->dw);
+    h = fold_string_member(h, qh->probe->def->dq);
+    h = fold_fixed_member(h, qh->probe->def->fp);
+    h = fold_fixed_member(h, qh->probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->fa);
@@ -318,7 +318,7 @@ static uint32_t fold_processor(uint32_t h, void *e) {
      * still proves the typed reads. */
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
diff --git a/tests/test_pico8_pm.c b/tests/test_pico8_pm.c
index 47d1bf3..1fa3a1c 100644
--- a/tests/test_pico8_pm.c
+++ b/tests/test_pico8_pm.c
@@ -143,7 +143,7 @@ static pico8_hj_t *as_hj(void *e) {
 }
 
 static uint32_t fold_pk(uint32_t h, const pico8_hj_t *hj) {
-    return fold_bool_member(h, hj->emitter->pk != 0);
+    return fold_bool_member(h, hj->emitter->def->pk != 0);
 }
 
 /* A particle record as `g_.mm` returns it: `p`, `v`, `l`. */
@@ -304,18 +304,18 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fold_fixed_member(h, qh->s);
     h = fold_fixed_member(h, qh->probe->od);
     h = fold_fixed_member(h, em->py);
-    h = fold_fixed_member(h, em->ki);
+    h = fold_fixed_member(h, em->def->ki);
     h = fold_fixed_member(h, em->pn);
     h = fold_pk(h, hj);
-    h = fold_fixed_member(h, em->fc);
+    h = fold_fixed_member(h, em->def->fc);
     h = fold_fixed_member(h, em->du);
     h = fold_fixed_member(h, em->qo);
     h = fold_fixed_member(h, em->ro);
     h = fold_fixed_member(h, em->qg);
     h = fold_fixed_member(h, em->q_);
-    h = fold_string_member(h, qh->probe->dq);
-    h = fold_fixed_member(h, qh->probe->fp);
-    h = fold_fixed_member(h, qh->probe->dw);
+    h = fold_string_member(h, qh->probe->def->dq);
+    h = fold_fixed_member(h, qh->probe->def->fp);
+    h = fold_fixed_member(h, qh->probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
@@ -346,7 +346,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(em->qy != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
diff --git a/tests/test_pico8_qh.c b/tests/test_pico8_qh.c
index 4a37eec..a504a66 100644
--- a/tests/test_pico8_qh.c
+++ b/tests/test_pico8_qh.c
@@ -267,11 +267,11 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
     h = fold_fixed_member(h, qh->jo.def->hp);
-    h = fold_string_member(h, qh->probe->dq);
+    h = fold_string_member(h, qh->probe->def->dq);
     h = fold_qh(h, qh);
     h = fold_fixed_member(h, qh->probe->od);
-    h = fold_fixed_member(h, qh->probe->fp);
-    h = fold_fixed_member(h, qh->probe->dw);
+    h = fold_fixed_member(h, qh->probe->def->fp);
+    h = fold_fixed_member(h, qh->probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_fixed_member(h, qh->s);
     h = fold_hf(h, jo);
@@ -285,7 +285,7 @@ static uint32_t fold_lab(uint32_t h, void *e) {
      * still proves the typed reads. */
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -507,9 +507,9 @@ static uint32_t fold_descendant(uint32_t h, void *d) {
     pico8_nl_t *nl = cls_of(d) == pico8_nl_struct_class() ? (pico8_nl_t *)d : NULL;
     pico8_fixed_t qw = nl != NULL ? nl->qw : 0;
 
-    h = fold_string_member(h, qh->probe->dq);
-    h = fold_fixed_member(h, qh->probe->fp);
-    h = fold_fixed_member(h, qh->probe->dw);
+    h = fold_string_member(h, qh->probe->def->dq);
+    h = fold_fixed_member(h, qh->probe->def->fp);
+    h = fold_fixed_member(h, qh->probe->def->dw);
     h = fold_hf(h, &qh->jo);
     h = qh->jo.has_mi ? fold_fixed_member(h, qh->jo.mi) : fold_absent(h);
     h = fnv(h, (uint32_t)(qh->jo.header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER))));
diff --git a/tests/test_pico8_spec_defaults.c b/tests/test_pico8_spec_defaults.c
index 5a7bd31..7a0d007 100644
--- a/tests/test_pico8_spec_defaults.c
+++ b/tests/test_pico8_spec_defaults.c
@@ -109,11 +109,9 @@ static void test_mz_defaults_and_ready(void) {
 
     assert(mz->def == &pico8_mz_def && mz->probe->def == &pico8_mz_def.probe);
     assert(mz->header.me == mz->def->me && strcmp(mz->header.me, "kz") == 0 && mz->header.t == pico8_ke_start.t);
-    assert(mz->probe->dw == mz->def->probe.dw && mz->probe->dq == mz->def->probe.dq);
-    assert(mz->probe->ik == mz->def->probe.ik && mz->probe->has_ik == mz->def->probe.has_ik);
     assert(mz->oi == mz->def->oi);
     assert(mz->probe->od == mz->def->probe.od && mz->probe->od == pico8_fixed_from_int(3));
-    assert(mz->probe->has_od == mz->def->probe.has_od && mz->probe->fp == mz->def->probe.fp);
+    assert(mz->probe->has_od == mz->def->probe.has_od);
     assert(mz->jj == pico8_mz_jj_wrapper() && mz->jj->qs == pico8_mz_jj_qs);
     assert(pico8_mz_ready());
     pico8_wormhole_shutdown();
@@ -134,17 +132,14 @@ static void test_fv_defaults_and_ready(void) {
     assert(g_->def == &pico8_fv_def && g_->emitter->def == &pico8_fv_def.emitter &&
            g_->probe->def == &pico8_fv_def.probe);
     assert(g_->ra == &g_->def->ra);
-    assert(g_->emitter->fc == g_->def->emitter.fc && g_->emitter->k_ == g_->def->emitter.k_);
+    assert(g_->emitter->k_ == g_->def->emitter.k_);
     assert(g_->emitter->qo == g_->def->emitter.qo && g_->emitter->ro == g_->def->emitter.ro);
     assert(g_->emitter->qg == g_->def->emitter.qg && g_->emitter->q_ == g_->def->emitter.q_);
     assert(g_->emitter->pn == g_->def->emitter.pn && g_->emitter->du == g_->def->emitter.du);
-    assert(g_->emitter->py == g_->def->emitter.py && g_->emitter->pk == g_->def->emitter.pk);
-    assert(g_->emitter->ki == g_->def->emitter.ki);
-    assert(g_->probe->dq == g_->def->probe.dq && g_->probe->od == g_->def->probe.od);
-    assert(g_->probe->has_od == g_->def->probe.has_od && g_->probe->fp == g_->def->probe.fp);
-    assert(g_->probe->ik == g_->def->probe.ik && g_->probe->has_ik == g_->def->probe.has_ik);
-    assert(g_->probe->dw == g_->def->probe.dw);
-    assert(g_->probe->has_od == 1 && g_->probe->ik == 0 && g_->probe->has_ik == 0);
+    assert(g_->emitter->py == g_->def->emitter.py);
+    assert(g_->probe->od == g_->def->probe.od);
+    assert(g_->probe->has_od == g_->def->probe.has_od);
+    assert(g_->probe->has_od == 1 && g_->probe->def->ik == 0 && g_->probe->def->has_ik == 0);
     assert(g_->header.me == pico8_ke_start.me && g_->header.t == pico8_ke_start.t);
     assert(pico8_fv_ready());
     pico8_help_shutdown();
@@ -158,14 +153,12 @@ static void test_g__defaults_and_ready(void) {
 
     assert(g_->def == &pico8_g__def && g_->emitter->def == &pico8_g__def.emitter &&
            g_->probe->def == &pico8_g__def.probe);
-    assert(g_->probe->dq == g_->def->probe.dq && g_->probe->od == g_->def->probe.od);
+    assert(g_->probe->od == g_->def->probe.od);
     assert(g_->probe->has_od == g_->def->probe.has_od && g_->probe->has_od == 1);
-    assert(g_->probe->fp == g_->def->probe.fp && g_->probe->dw == g_->def->probe.dw);
-    assert(g_->probe->ik == g_->def->probe.ik && g_->probe->has_ik == g_->def->probe.has_ik);
-    assert(g_->probe->ik == 0 && g_->probe->has_ik == 0);
-    assert(g_->emitter->py == g_->def->emitter.py && g_->emitter->ki == g_->def->emitter.ki);
-    assert(g_->emitter->pn == g_->def->emitter.pn && g_->emitter->pk == g_->def->emitter.pk);
-    assert(g_->emitter->fc == g_->def->emitter.fc && g_->emitter->k_ == g_->def->emitter.k_);
+    assert(g_->probe->def->ik == 0 && g_->probe->def->has_ik == 0);
+    assert(g_->emitter->py == g_->def->emitter.py);
+    assert(g_->emitter->pn == g_->def->emitter.pn);
+    assert(g_->emitter->k_ == g_->def->emitter.k_);
     assert(g_->emitter->qo == g_->def->emitter.qo && g_->emitter->ro == g_->def->emitter.ro);
     assert(g_->emitter->qg == g_->def->emitter.qg && g_->emitter->q_ == g_->def->emitter.q_);
     assert(g_->emitter->du == g_->def->emitter.du);
```

#### Diff de 2.13e2c (`git rm tests/test_pico8_archetype_chain.c`, puis `git diff e2b e2c` sans ce fichier)

```diff
diff --git a/Makefile b/Makefile
index 7ebf0f8..9ff7838 100644
--- a/Makefile
+++ b/Makefile
@@ -478,9 +478,6 @@ $(TMP_DIR)/test_pico8_spec_defaults: $(call objs,tests/test_pico8_spec_defaults.
 $(TMP_DIR)/test_pico8_jo_spec_defaults: $(call objs,tests/test_pico8_jo_spec_defaults.c $(ENGINE_SRCS)) | $(TMP_DIR)
 	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm
 
-$(TMP_DIR)/test_pico8_archetype_chain: $(call objs,tests/test_pico8_archetype_chain.c $(ENGINE_SRCS)) | $(TMP_DIR)
-	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm
-
 $(TMP_DIR)/test_pico8_nl: $(call objs,tests/test_pico8_nl.c $(ENGINE_SRCS)) | $(TMP_DIR)
 	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm
 
@@ -632,7 +629,6 @@ UNIT_TESTS := \
     test_pico8_entity_kind \
     test_pico8_spec_defaults \
     test_pico8_jo_spec_defaults \
-    test_pico8_archetype_chain \
     test_pico8_os \
     test_pico8_hj \
     test_pico8_pm \
@@ -871,7 +867,7 @@ composition-residue: tools/composition_residue.sh
 	@sh tools/composition_residue.sh
 
 # Catégories du compteur de composition tombées à 0, séparées par des virgules.
-COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres
+COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts
 
 composition-guard: tools/composition_residue.sh
 	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests
diff --git a/src/pico8_bg.c b/src/pico8_bg.c
index 55de6d8..9be6afc 100644
--- a/src/pico8_bg.c
+++ b/src/pico8_bg.c
@@ -66,7 +66,7 @@ static const pico8_struct_class_t g_bg_struct_class = {
 static pico8_bg_t *pico8_bg_struct_new(void) {
     pico8_bg_t *self = malloc(sizeof *self);
 
-    pico8_ke_struct_init(&self->header, &g_bg_struct_class);
+    pico8_ke_struct_init(&self->header, &g_bg_struct_class, pico8_bg_def.me, pico8_bg_def.t);
     self->def = &pico8_bg_def;
     self->mq = NULL;
     self->m_ = 0;
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index 3838fe6..52b8002 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -285,11 +285,6 @@ const pico8_jb_row_t pico8_pj_ou_row = {.count = 7, .items = pj_ou_items};
 
 const char pico8_mz_jj_qs[] = "-too far-";
 
-const pico8_ke_start_t pico8_ke_start = {
-    .me = "ph",
-    .t = 0x0,
-};
-
 const pico8_bg_def_t pico8_bg_def = {
     .me = "ph",
     .t = 0x0,
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index b373e0c..cb927a7 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -40,8 +40,8 @@ typedef struct {
 } pico8_vec_seq_t;
 
 /* A kind's `rr`, `s`, `me`, `od` as its chain of defaults resolves them: a
- * NULL string, or has_* 0, when absent. The strings are the ones the
- * pico8_X_start of the class that sets each points at. */
+ * NULL string, or has_* 0, when absent. The strings are the ones its
+ * archetype points at. */
 typedef struct pico8_kind_rec {
     const char *rr;
     int has_s;
@@ -112,13 +112,6 @@ typedef struct {
     int ra_y;
 } pico8_bz_entry_t;
 
-/* The values a new instance of a class starts with: exactly the keys of the
- * class's spec, ke's left out, under the struct's own member types. */
-typedef struct {
-    const char *me;
-    pico8_fixed_t t;
-} pico8_ke_start_t;
-
 /* A class's archetype: its defaults resolved along its init chain, ke's
  * `me`/`t` first. A default that is the address of a module's static object
  * (ui.le, nf.ra, mz.jj) is not here: the module's init still sets it. */
@@ -376,7 +369,6 @@ extern const pico8_jb_row_t pico8_pj_ou_row;
 
 /* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
  * nf.r are empty tables, held by their module. */
-extern const pico8_ke_start_t pico8_ke_start;
 extern const pico8_ga_pq_keys_t pico8_pm_ga_pq_keys;
 extern const pico8_bg_def_t pico8_bg_def;
 extern const pico8_ui_def_t pico8_ui_def;
diff --git a/src/pico8_ke.c b/src/pico8_ke.c
index 67b4357..74fdcb8 100644
--- a/src/pico8_ke.c
+++ b/src/pico8_ke.c
@@ -31,10 +31,11 @@ void pico8_ke_ks(pico8_struct_header_t *self, const char *me) {
     self->t = pico8_fixed_from_int(0);
 }
 
-void pico8_ke_struct_init(pico8_struct_header_t *self, const pico8_struct_class_t *cls) {
+void pico8_ke_struct_init(pico8_struct_header_t *self, const pico8_struct_class_t *cls, const char *me,
+                          pico8_fixed_t t) {
     self->cls = cls;
-    self->me = pico8_ke_start.me;
-    self->t = pico8_ke_start.t;
+    self->me = me;
+    self->t = t;
 }
 
 void pico8_ke_struct_pq(pico8_struct_header_t *self) {
diff --git a/src/pico8_ke.h b/src/pico8_ke.h
index be15dbc..af8126a 100644
--- a/src/pico8_ke.h
+++ b/src/pico8_ke.h
@@ -17,9 +17,10 @@ int pico8_ke_ready(void);
  * borrowed. */
 void pico8_ke_ks(pico8_struct_header_t *self, const char *me);
 
-/* Tags `self` as a struct of class `cls` and sets ke's defaults,
- * `pico8_ke_start`. */
-void pico8_ke_struct_init(pico8_struct_header_t *self, const pico8_struct_class_t *cls);
+/* Tags `self` as a struct of class `cls` and sets `me` and `t`, its class's
+ * archetype's. `me` is borrowed. */
+void pico8_ke_struct_init(pico8_struct_header_t *self, const pico8_struct_class_t *cls, const char *me,
+                          pico8_fixed_t t);
 
 /* ke's pq for a struct. A child calls it exactly once, after its own pq,
  * or alone when it has none: the class walk's derived-to-base order and
diff --git a/src/pico8_mouse_reader.c b/src/pico8_mouse_reader.c
index 52eb2ac..5d645bf 100644
--- a/src/pico8_mouse_reader.c
+++ b/src/pico8_mouse_reader.c
@@ -43,7 +43,7 @@ static void pico8_nf_pq(pico8_mem_t *mem) {
 static pico8_nf_t *pico8_nf_struct_new(void) {
     pico8_nf_t *self = malloc(sizeof *self);
 
-    pico8_ke_struct_init(&self->header, &g_nf_struct_class);
+    pico8_ke_struct_init(&self->header, &g_nf_struct_class, pico8_nf_def.me, pico8_nf_def.t);
     self->def = &pico8_nf_def;
     self->ra = &g_nf_ra;
 #ifdef PICO8_CLASS_COVERAGE
diff --git a/src/pico8_msgbox.c b/src/pico8_msgbox.c
index fb95c6e..7f9f9e7 100644
--- a/src/pico8_msgbox.c
+++ b/src/pico8_msgbox.c
@@ -107,7 +107,7 @@ static pico8_mh_t *pico8_mh_struct_new(const pico8_mh_keys_t *keys) {
     pico8_mh_t *self = malloc(sizeof *self);
     int i;
 
-    pico8_ke_struct_init(&self->header, &g_mh_struct_class);
+    pico8_ke_struct_init(&self->header, &g_mh_struct_class, pico8_mh_def.me, pico8_mh_def.t);
     self->def = &pico8_mh_def;
     self->rh = self->def->rh;
     self->lh = NULL;
@@ -254,7 +254,7 @@ static const pico8_struct_class_t g_iv_struct_class = {
 static pico8_iv_t *pico8_iv_struct_new(const pico8_iv_keys_t *keys) {
     pico8_iv_t *self = malloc(sizeof *self);
 
-    pico8_ke_struct_init(&self->header, &g_iv_struct_class);
+    pico8_ke_struct_init(&self->header, &g_iv_struct_class, pico8_iv_def.me, pico8_iv_def.t);
     self->def = &pico8_iv_def;
     self->l = self->def->l;
     self->ra = NULL;
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 7eb6692..713fab1 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -134,7 +134,7 @@ static const pico8_struct_class_t g_jo_struct_class = {
 
 void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     assert(g_jo_up && "pico8_jo_init() must run first");
-    pico8_ke_struct_init(&self->header, cls);
+    pico8_ke_struct_init(&self->header, cls, def->me, def->t);
     self->def = def;
     self->ma = self->def->ma;
     self->kw = &g_jo_kw_shared;
@@ -163,7 +163,6 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     self->m_ = 0;
     self->has_m_ = 0;
     self->kw_own = (pico8_kw_t){0};
-    self->header.me = self->def->me;
 }
 
 static pico8_jo_t *pico8_jo_struct_new(const pico8_jo_keys_t *keys) {
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index 097665e..43c2dc0 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -437,7 +437,7 @@ static const pico8_struct_class_t g_g__struct_class = {
 };
 
 void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, const pico8_g__def_t *def) {
-    pico8_ke_struct_init(&self->header, cls);
+    pico8_ke_struct_init(&self->header, cls, def->me, def->t);
     self->def = def;
     self->emitter = &self->emitter_own;
     self->emitter->def = &self->def->emitter;
diff --git a/src/pico8_research_icon.c b/src/pico8_research_icon.c
index 5acf841..4eb4383 100644
--- a/src/pico8_research_icon.c
+++ b/src/pico8_research_icon.c
@@ -237,7 +237,7 @@ static pico8_ie_t *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
     pico8_ie_t *self = malloc(sizeof *self);
 
     assert(g_ie_up && "pico8_research_icon_init() must run before an ie is built");
-    pico8_ke_struct_init(&self->header, &g_ie_struct_class);
+    pico8_ke_struct_init(&self->header, &g_ie_struct_class, pico8_ie_def.me, pico8_ie_def.t);
     self->def = &pico8_ie_def;
     self->ra = &self->def->ra;
     self->ke = NULL;
diff --git a/src/pico8_score_panel.c b/src/pico8_score_panel.c
index 1781501..8f0b86d 100644
--- a/src/pico8_score_panel.c
+++ b/src/pico8_score_panel.c
@@ -290,7 +290,7 @@ static pico8_pj_t *pico8_pj_struct_new(void) {
 
     assert(g_pj_up && "pico8_score_panel_init() must run first");
     self = malloc(sizeof *self);
-    pico8_ke_struct_init(&self->header, &g_pj_struct_class);
+    pico8_ke_struct_init(&self->header, &g_pj_struct_class, pico8_pj_def.me, pico8_pj_def.t);
     self->def = &pico8_pj_def;
     self->fa = self->def->fa;
     self->ed = NULL;
diff --git a/src/pico8_scroll.c b/src/pico8_scroll.c
index 128b29b..f21b845 100644
--- a/src/pico8_scroll.c
+++ b/src/pico8_scroll.c
@@ -44,7 +44,7 @@ static const pico8_struct_class_t g_qb_struct_class = {
 static pico8_qb_t *pico8_qb_struct_new(void) {
     pico8_qb_t *self = malloc(sizeof *self);
 
-    pico8_ke_struct_init(&self->header, &g_qb_struct_class);
+    pico8_ke_struct_init(&self->header, &g_qb_struct_class, pico8_qb_def.me, pico8_qb_def.t);
     self->def = &pico8_qb_def;
     self->v = &self->def->v;
     self->ma = self->def->ma;
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index 2566224..7263225 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -395,9 +395,8 @@ static const pico8_struct_class_t g_jc_struct_class = {
 static pico8_jc_t *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
     pico8_jc_t *self = malloc(sizeof *self);
 
-    pico8_ke_struct_init(&self->header, &g_jc_struct_class);
+    pico8_ke_struct_init(&self->header, &g_jc_struct_class, pico8_jc_def.me, pico8_jc_def.t);
     self->def = &pico8_jc_def;
-    self->header.me = self->def->me;
     self->fa = self->def->fa;
     self->probe = &self->probe_own;
     self->probe->def = &self->def->probe;
diff --git a/src/pico8_ui.c b/src/pico8_ui.c
index c8e4c1e..47f4175 100644
--- a/src/pico8_ui.c
+++ b/src/pico8_ui.c
@@ -96,7 +96,7 @@ static pico8_ui_t *pico8_ui_struct_new(void) {
 
     assert(g_ui_up && "pico8_ui_init() must run first");
     self = malloc(sizeof *self);
-    pico8_ke_struct_init(&self->header, &g_ui_struct_class);
+    pico8_ke_struct_init(&self->header, &g_ui_struct_class, pico8_ui_def.me, pico8_ui_def.t);
     self->def = &pico8_ui_def;
     self->le = &g_shared_le;
     self->qa = NULL;
diff --git a/src/pico8_ui_button.c b/src/pico8_ui_button.c
index 1ba9241..96b53af 100644
--- a/src/pico8_ui_button.c
+++ b/src/pico8_ui_button.c
@@ -106,7 +106,7 @@ static const pico8_struct_class_t g_js_struct_class = {
 static pico8_js_t *pico8_js_struct_new(const pico8_js_keys_t *keys) {
     pico8_js_t *self = malloc(sizeof *self);
 
-    pico8_ke_struct_init(&self->header, &g_js_struct_class);
+    pico8_ke_struct_init(&self->header, &g_js_struct_class, pico8_js_def.me, pico8_js_def.t);
     self->lq = NULL;
     self->mb = 0;
     self->kq = NULL;
@@ -353,7 +353,7 @@ static const pico8_struct_class_t g_fq_struct_class = {
 static pico8_fq_t *pico8_fq_struct_new(const pico8_fq_keys_t *keys) {
     pico8_fq_t *self = malloc(sizeof *self);
 
-    pico8_ke_struct_init(&self->header, &g_fq_struct_class);
+    pico8_ke_struct_init(&self->header, &g_fq_struct_class, pico8_fq_def.me, pico8_fq_def.t);
     self->def = &pico8_fq_def;
     self->my = NULL;
     self->p_ = 0;
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index cd38d62..c044514 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -239,9 +239,8 @@ static const pico8_struct_class_t g_mz_struct_class = {
 static pico8_mz_t *pico8_mz_struct_new(void) {
     pico8_mz_t *self = malloc(sizeof *self);
 
-    pico8_ke_struct_init(&self->header, &g_mz_struct_class);
+    pico8_ke_struct_init(&self->header, &g_mz_struct_class, pico8_mz_def.me, pico8_mz_def.t);
     self->def = &pico8_mz_def;
-    self->header.me = self->def->me;
     self->probe = &self->probe_own;
     self->probe->def = &self->def->probe;
     self->oi = self->def->oi;
diff --git a/tests/test_composition_residue.c b/tests/test_composition_residue.c
index 8004be7..13e6704 100644
--- a/tests/test_composition_residue.c
+++ b/tests/test_composition_residue.c
@@ -332,7 +332,8 @@ static void test_registres_sees_hidden_forms_and_not_its_named_limits(void) {
 
 /* ptrs-entité lit les membres d'un type d'entité comme registres lit ceux des
  * autres structs, avec les mêmes limites (void *owner, :15). jy est exempté
- * seul, pas other_jy sur sa ligne (:16) ; (*mk) n'a pas de nom lisible (:17). */
+ * seul, pas other_jy sur sa ligne (:16) ; (*mk) n'a pas de nom lisible
+ * (:17). */
 static void test_ptrs_entite_reads_entity_members_like_registres(void) {
     setup();
     write_file(SRC "/a.h", "typedef pico8_" "ptrs_t reg17_t;\n");
@@ -376,6 +377,20 @@ static void test_ptrs_entite_reads_entity_members_like_registres(void) {
                                   "résidu gardé (ptrs-entité) : 13 ligne(s)\n") == 0);
 }
 
+/* Deux types d'entité de deux fichiers, un membre au même numéro de ligne :
+ * deux lignes, la clé qui les dédoublonne tient au type, pas au numéro. */
+static void test_ptrs_entite_keeps_one_line_number_in_two_files(void) {
+    setup();
+    write_file(SRC "/e.h", "typedef struct {\n    pico8_struct_" "header_t header;\n} pico8_" "ke_t;\n"
+                           "typedef struct {\n    pico8_" "ke_t ke;\n    void **items;\n} pico8_" "jo_t;\n");
+    write_file(SRC "/g.h", "/* g */\n\n\n"
+                           "typedef struct {\n    pico8_" "ke_t ke;\n    void **more;\n} pico8_" "qh_t;\n");
+    assert(run_residue("--check ptrs-entité " TREE) == 1);
+    assert(strcmp(read_file(OUT), "ptrs-entité: " SRC "/e.h:6:    void **items;\n"
+                                  "ptrs-entité: " SRC "/g.h:6:    void **more;\n"
+                                  "résidu gardé (ptrs-entité) : 2 ligne(s)\n") == 0);
+}
+
 /* const et volatile autour de void et des *, plusieurs déclarations sur une
  * ligne (comptée une fois), déclarateur à la ligne suivante, attribut à deux
  * niveaux : vus. Limites (attendu 0) : attribut à trois niveaux (e.h:18),
@@ -663,6 +678,7 @@ int main(void) {
     test_registres_reads_declaration_points_outside_entity_types();
     test_registres_sees_hidden_forms_and_not_its_named_limits();
     test_ptrs_entite_reads_entity_members_like_registres();
+    test_ptrs_entite_keeps_one_line_number_in_two_files();
     test_readers_see_qualifiers_split_declarations_and_nested_attributes();
     test_registres_sees_split_braces_unions_and_attributes();
     test_void_ptr_reads_joined_public_prototypes();
diff --git a/tests/test_pico8_iv.c b/tests/test_pico8_iv.c
index d19f4b4..5db0f3c 100644
--- a/tests/test_pico8_iv.c
+++ b/tests/test_pico8_iv.c
@@ -201,7 +201,7 @@ static void test_dg_drives_a_ke_struct(void) {
     int i;
 
     fresh_world();
-    pico8_ke_struct_init(&f->header, &g_fake_toast_class);
+    pico8_ke_struct_init(&f->header, &g_fake_toast_class, "ph", 0);
     f->p_ = 0;
     pico8_ke_struct_pq(&f->header);
     assert(ents_listed(pico8_hv(), f));
diff --git a/tests/test_pico8_jo_spec_defaults.c b/tests/test_pico8_jo_spec_defaults.c
index 98fdf15..1cf20e2 100644
--- a/tests/test_pico8_jo_spec_defaults.c
+++ b/tests/test_pico8_jo_spec_defaults.c
@@ -207,22 +207,21 @@ static void test_mx_defaults(void) {
 }
 
 /* The second `jl()` (the one `eu` runs) finds `ke` ready and leaves it
- * alone: a fresh struct of each module still holds ke's `t`. */
+ * alone: a fresh struct of each module still holds `t` 0. */
 static void test_second_jl_keeps_ke(void) {
     assert(pico8_ke_ready());
     pico8_jl();
     assert(pico8_ke_ready());
-    assert(pico8_jo_new(NULL)->header.t == pico8_ke_start.t);
-    assert(pico8_qh_new(NULL)->jo.header.t == pico8_ke_start.t);
-    assert(pico8_nl_new(NULL)->qh.jo.header.t == pico8_ke_start.t);
-    assert(pico8_os_new(NULL)->qh.jo.header.t == pico8_ke_start.t);
-    assert(pico8_hj_new(NULL)->qh.jo.header.t == pico8_ke_start.t);
-    assert(pico8_pm_new(NULL)->hj.qh.jo.header.t == pico8_ke_start.t);
-    assert(pico8_ga_new(NULL)->jo.header.t == pico8_ke_start.t);
-    assert(pico8_nr_new(NULL)->qh.jo.header.t == pico8_ke_start.t);
-    assert(pico8_fr_new(NULL)->nr.qh.jo.header.t == pico8_ke_start.t);
-    assert(pico8_mx_new(NULL)->hj.qh.jo.header.t == pico8_ke_start.t);
-    assert(strcmp(pico8_ke_start.me, "ph") == 0 && pico8_ke_start.t == 0);
+    assert(pico8_jo_new(NULL)->header.t == 0);
+    assert(pico8_qh_new(NULL)->jo.header.t == 0);
+    assert(pico8_nl_new(NULL)->qh.jo.header.t == 0);
+    assert(pico8_os_new(NULL)->qh.jo.header.t == 0);
+    assert(pico8_hj_new(NULL)->qh.jo.header.t == 0);
+    assert(pico8_pm_new(NULL)->hj.qh.jo.header.t == 0);
+    assert(pico8_ga_new(NULL)->jo.header.t == 0);
+    assert(pico8_nr_new(NULL)->qh.jo.header.t == 0);
+    assert(pico8_fr_new(NULL)->nr.qh.jo.header.t == 0);
+    assert(pico8_mx_new(NULL)->hj.qh.jo.header.t == 0);
 }
 
 int main(void) {
diff --git a/tests/test_pico8_map_tables.c b/tests/test_pico8_map_tables.c
index 1a3172d..abd5a98 100644
--- a/tests/test_pico8_map_tables.c
+++ b/tests/test_pico8_map_tables.c
@@ -153,7 +153,7 @@ static void test_jo_tree_strings_are_distinct_objects(void) {
     const char *const tree[] = {pico8_jo_def.me, pico8_qh_def.me, pico8_qh_def.probe.dq, pico8_nl_def.rr,
         pico8_hj_def.rr, pico8_pm_def.fz, pico8_pm_def.me, pico8_ga_def.fz, pico8_nr_def.rr,
         pico8_fr_def.rr, pico8_mx_def.rr};
-    const char *const others[] = {pico8_ke_start.me, pico8_mz_def.me, pico8_mz_def.probe.dq, pico8_jc_def.me,
+    const char *const others[] = {pico8_bg_def.me, pico8_mz_def.me, pico8_mz_def.probe.dq, pico8_jc_def.me,
         pico8_g__def.probe.dq, pico8_mz_jj_qs};
     int n = (int)(sizeof tree / sizeof tree[0]);
     int i;
diff --git a/tests/test_pico8_spec_defaults.c b/tests/test_pico8_spec_defaults.c
index 7a0d007..26c76be 100644
--- a/tests/test_pico8_spec_defaults.c
+++ b/tests/test_pico8_spec_defaults.c
@@ -75,7 +75,7 @@ static void test_bg_defaults_and_ready(void) {
     pico8_bg_t *bg = pico8_bg_new();
 
     assert(bg->def == &pico8_bg_def && bg->def->pl == &pico8_bg_pl_seq);
-    assert(bg->header.me == pico8_ke_start.me && bg->header.t == pico8_ke_start.t);
+    assert(bg->header.me == bg->def->me && strcmp(bg->header.me, "ph") == 0 && bg->header.t == 0);
     assert(pico8_bg_ready());
     pico8_bg_shutdown();
     assert(!pico8_bg_ready());
@@ -85,19 +85,29 @@ static void test_js_defaults_and_ready(void) {
     pico8_js_t *js = pico8_js_new(NULL);
 
     assert(js->def == &pico8_js_def && js->fa == js->def->fa);
-    assert(js->header.me == pico8_ke_start.me && js->header.t == pico8_ke_start.t);
+    assert(js->header.me == js->def->me && strcmp(js->header.me, "ph") == 0 && js->header.t == 0);
     assert(pico8_js_ready());
     pico8_js_shutdown();
     assert(!pico8_js_ready());
 }
 
+/* `ra` is the module's shared vector, not the archetype's: two fresh
+ * readers point at the same one. */
+static void test_nf_ra_is_shared(void) {
+    const pico8_nf_t *nf = pico8_nf_new();
+    const pico8_nf_t *other = pico8_nf_new();
+
+    assert(nf->def == &pico8_nf_def);
+    assert(nf->ra != NULL && nf->ra == other->ra);
+}
+
 /* `v`, `p` and `ra` start on the archetype's vectors. */
 static void test_qb_defaults_and_ready(void) {
     pico8_qb_t *qb = pico8_qb_new();
 
     assert(qb->def == &pico8_qb_def && qb->def->pc == &pico8_qb_pc_vecs);
     assert(qb->v == &qb->def->v && qb->p == &qb->def->p && qb->ra == &qb->def->ra && qb->ma == qb->def->ma);
-    assert(qb->header.me == pico8_ke_start.me && qb->header.t == pico8_ke_start.t);
+    assert(qb->header.me == qb->def->me && strcmp(qb->header.me, "ph") == 0 && qb->header.t == 0);
     assert(pico8_qb_ready());
     pico8_qb_shutdown();
     assert(!pico8_qb_ready());
@@ -108,7 +118,7 @@ static void test_mz_defaults_and_ready(void) {
     pico8_mz_t *mz = pico8_mz_new();
 
     assert(mz->def == &pico8_mz_def && mz->probe->def == &pico8_mz_def.probe);
-    assert(mz->header.me == mz->def->me && strcmp(mz->header.me, "kz") == 0 && mz->header.t == pico8_ke_start.t);
+    assert(mz->header.me == mz->def->me && strcmp(mz->header.me, "kz") == 0 && mz->header.t == 0);
     assert(mz->oi == mz->def->oi);
     assert(mz->probe->od == mz->def->probe.od && mz->probe->od == pico8_fixed_from_int(3));
     assert(mz->probe->has_od == mz->def->probe.has_od);
@@ -140,7 +150,7 @@ static void test_fv_defaults_and_ready(void) {
     assert(g_->probe->od == g_->def->probe.od);
     assert(g_->probe->has_od == g_->def->probe.has_od);
     assert(g_->probe->has_od == 1 && g_->probe->def->ik == 0 && g_->probe->def->has_ik == 0);
-    assert(g_->header.me == pico8_ke_start.me && g_->header.t == pico8_ke_start.t);
+    assert(g_->header.me == g_->def->me && strcmp(g_->header.me, "ph") == 0 && g_->header.t == 0);
     assert(pico8_fv_ready());
     pico8_help_shutdown();
     assert(!pico8_fv_ready());
@@ -163,7 +173,7 @@ static void test_g__defaults_and_ready(void) {
     assert(g_->emitter->qg == g_->def->emitter.qg && g_->emitter->q_ == g_->def->emitter.q_);
     assert(g_->emitter->du == g_->def->emitter.du);
     assert(g_->ra == NULL);
-    assert(g_->header.me == pico8_ke_start.me && g_->header.t == pico8_ke_start.t);
+    assert(g_->header.me == g_->def->me && strcmp(g_->header.me, "ph") == 0 && g_->header.t == 0);
     assert(pico8_g__ready());
     pico8_probe_shutdown();
     assert(!pico8_g__ready());
@@ -183,7 +193,7 @@ static void test_ie_defaults_and_ready(void) {
     pico8_ie_t *ie = pico8_ie_new(NULL);
 
     assert(ie->def == &pico8_ie_def && ie->ra == &ie->def->ra);
-    assert(ie->header.me == pico8_ke_start.me && ie->header.t == pico8_ke_start.t);
+    assert(ie->header.me == ie->def->me && strcmp(ie->header.me, "ph") == 0 && ie->header.t == 0);
     assert(pico8_ie_ready());
     pico8_research_icon_shutdown();
     assert(!pico8_ie_ready());
@@ -199,7 +209,7 @@ static void test_pj_defaults_and_ready(void) {
     assert(cr->hi->x == 0 && cr->hi->y == pico8_fixed_from_int(11));
     assert(cr->gq->x == 0 && cr->gq->y == pico8_fixed_from_int(11));
     assert(cr->ca == pico8_fixed_from_int(10));
-    assert(pj->header.me == pico8_ke_start.me && pj->header.t == pico8_ke_start.t);
+    assert(pj->header.me == pj->def->me && strcmp(pj->header.me, "ph") == 0 && pj->header.t == 0);
     assert(pico8_pj_ready());
     pico8_score_panel_shutdown();
     assert(!pico8_pj_ready());
@@ -248,6 +258,7 @@ int main(void) {
     test_bg_defaults_and_ready();
     test_js_defaults_and_ready();
     test_qb_defaults_and_ready();
+    test_nf_ra_is_shared();
     test_mz_defaults_and_ready();
     test_layout_ready();
     test_fv_defaults_and_ready();
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index 89105a9..ba0a348 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -829,7 +829,7 @@ static uint32_t fold_string(uint32_t h, const char *s) {
 }
 
 static int identity_of(const char *s) {
-    if (s == pico8_ke_start.me) {
+    if (s == pico8_bg_def.me) {
         return 1;
     }
     if (s == pico8_jo_def.me) {
```

### Test ponctuel : ce qui part, ce qui reste

Même règle qu'en 2.13a à 2.13d : une comparaison part dans le commit qui retire **sa** chaîne (la dernière écriture qui pose le champ à la construction), jamais réécrite vers `def`. À `p`, il en reste 123 : `me`/`t` de la famille a (20), `t` de `mz`/`jc` (2), `me`/`t` de `g_`/`fv` (4), et la colonne « Reste à `d2` » du PRD 2.13d (97, dont un `t` par sorte).

| Commit | Part | Reste |
|---|---|---|
| e1 | 55 : `hj` 17 (`hp`, `px`, `qh`, `s`, `probe.od`, `rr`, émetteur ×11), `pm` 20 (les mêmes, plus `me`, `ng`, `fz`), `mx` 18 (ceux de `hj`, plus `ht`) — leur chaîne était `pico8_hj_start`/`pm`/`mx` | **68** |
| e2a | 32 : `ga` 17 (`hf`, `fa`, `kq`, `fz`, `iu`, `ms`, émetteur ×11), `nr` 7 (`kq`, `s`, `probe.dw`, `od`, `fp`, `hz`, `rr`), `fr` 8 (ceux de `nr`, plus `mt`) | **36** : `me`/`t` de ke, un `t` par sorte |
| e2b | rien (aucune chaîne ne change : les copies des champs A de composant étaient déjà lues sur `def`) | 36 |
| e2c | les 36 (`pico8_ke_start` part) ; le fichier est retiré | — |

Écriture dans le TU (e1, e2a) : `same_hj_part`, `same_nr_part`, `same_emitter_part`, `same_vec`, `same_box` disparaissent quand ils n'ont plus d'appelant (`-Werror=unused-function`) ; chaque sorte ne garde que son `t` et ses `assert` nus (`et` NULL, `by`, `lz` a tourné, `qy` frais, `kw`/`by` partagés de `ga`). Le message du total garde sa forme (`68`, puis `36`).

**Les `assert` nus, avant le retrait** (e2c, lignes S de la preuve 3) : `nf.ra` partagé n'avait que le test ponctuel (S1) et va dans `spec_defaults` (`test_nf_ra_is_shared`) ; `mz.jj` partagé (`spec_defaults`, `mz`), `ui.le` partagé (`ui`), `kw` partagé (`jo`, `ga`, `jo_spec_defaults`), `by` NULL et `et` NULL de l'arbre `qh` (les TU de sorte) ont déjà un filet durable ; `qy` frais est vu par les plis des TU `hj`, `pm`, `mx` (`ptrs_size(em->qy) == 0`), celui de `ga` par `test_pico8_ga.c` (non nul après `ga_pq`) ; `lz` a tourné, par `jo->kw != pico8_jo_shared_kw()` des TU `hj`, `fr`.

### Comptes, mesurés sur la copie

| Mesure | `p` | `e1` | `e2a` | `e2b` | `e2c` |
|---|---|---|---|---|---|
| `défauts` (lignes / fichiers) | 224 / 15 | **97 / 14** | **27 / 8** | **27 / 8** | **0** (ligne absente ; gardé) |
| pied `résidu 2` | 628 / 72 | **501 / 72** | **431 / 70** | **431 / 70** | **404 / 63** |
| `parente`, `lien-parent`, `accesseurs-q6`, `void-ptr` | 10/3, 111/27, 237/33, 8/4 | identiques | identiques | identiques | identiques |
| `inclusion-mutuelle` (`--check`) | 17 / 16, 43 lignes | **17 / 16, 43** | **17 / 16, 43** | **17 / 16, 43** | **17 / 16, 43** |
| `--champs` | 255 ; 15 ; 20 | **249 ; 15 ; 20** | **242 ; 15 ; 20** | **242 ; 15 ; 20** | **242 ; 15 ; 20** |
| `wc -l src/pico8_game_data.h` | 536 | **473** | **434** | **437** | **429** |
| total du test ponctuel | 123 | **68** | **36** | **36** | fichier retiré |
| casts / tests de classe | 532 / 707 | 532 / 707 | 532 / 707 | **530 / 708** | 530 / 708 |
| `COMPOSITION_RESIDUE_GUARD` | 7 catégories | 7 | 7 | 7 | **8** (`défauts`) |

**Attribution des lignes `défauts`** (`git grep -cE '\bpico8_[a-z0-9_]+_start(_t)?\b'` par fichier, égal au `--check`) :
- e1, **127** : `jo_spec_defaults.c` 55, `planet_family.c` 34, `planet_family_trade.c` 17, `entity_kind.c` 10, `game_data.h` 6, `game_data.c` 3, `pm.c` 1, `ui.c` 1. Sort de `défauts` : `test_pico8_pm.c` (15 → 14 fichiers) ; le pied garde ses 72 fichiers (`test_pico8_pm.c` reste compté par une autre catégorie).
- e2a, **70** : `jo_spec_defaults.c` 25, `planet_family.c` 13, `planet_family_trade.c` 12, `entity_kind.c` 6, `game_data.h` 6, `game_data.c` 3, `map_tables.c` 2, `fr.c`, `ga.c`, `nr.c` 1 chacun. Sortent : `planet_family.c`, `planet_family_trade.c`, `entity_kind.c`, `fr.c`, `ga.c`, `nr.c` (14 → 8) ; le pied perd deux fichiers, `test_pico8_entity_kind.c` et `test_pico8_ga.c`, que plus aucune catégorie ne compte (70).
- e2c, **27** : `jo_spec_defaults.c` 11, `spec_defaults.c` 8, `game_data.h` 2, `ke.c` 2, `game_data.c` 1, `ke.h` 1, `map_tables.c` 1, `ui.c` 1 ; les huit fichiers sortent, le pied en perd sept (63 ; `test_pico8_ui.c` reste compté par une autre catégorie).

**`--champs`, lu** : e1 retire six membres de types d'entité (`pico8_qh_t` `hp`, `px`, `qh` ; `pico8_hj_t` `rr` ; `pico8_pm_t` `ng`, `fz`), 255 → **249** ; e2a en retire sept (`pico8_jo_t` `hf`, `fa`, `kq` ; `pico8_ga_t` `fz` ; `pico8_nr_t` `hz`, `rr` ; `pico8_fr_t` `mt`), → **242** ; e2b ne touche que `pico8_probe_t`/`pico8_emitter_t`, qui ne sont pas des types d'entité (inchangé) ; e2c n'enlève aucun membre. « 0 écriture » et « 0 hors constructeur » ne bougent à aucun commit.

**Casts et tests de classe à e2b** : les deux `((pico8_g__t *)probe)->probe->dw = 0;` de `test_pico8_jc.c` partent (532 → **530**) ; `probe_with_zero_dw()` appelle `pico8_g__struct_class()` pour construire (707 → **708**). Ce n'est pas un aiguillage par classe : l'accesseur rend la classe à poser, comme dans `test_pico8_g_.c` (2.13c) ; le grep le compte (G6).

**`game_data.h`** : e1 − 63 (trois types `_start`, trois `extern`, `pico8_pm_start_t` le plus long) ; e2a − 39 (trois types, trois `extern`, le commentaire « The jo tree » qui les précédait) ; e2b + 3 (deux commentaires de contrat déplacés sur `pico8_probe_def_t`/`pico8_emitter_def_t`) ; e2c − 8 (`pico8_ke_start_t`, son commentaire, son `extern`).

### Contrat pour 2.14 et la suite (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| `def` des composants | 2.14, 2.15 (liens de composant, Q-2.10 « oui ») | posé une fois par l'init du porteur à `&def->probe`/`&def->emitter` (`pb` : `&pico8_probe_def_none`) ; les lecteurs génériques lisent `probe->def->X`/`emitter->def->X` à travers `probe_of`/`emitter_of` ; un lien du noyau vers `&self->probe_own`/`&self->emitter_own` n'a rien à changer au `def` |
| `emitter.pn`/`du` de `ga` | toute sous-étape qui lit l'émetteur de `ga` | **non écrits** par l'init de `ga` (valeurs mortes, lues par `g_:mm` seul) : un lecteur neuf de `pn`/`du` sur `ga` lirait de la mémoire indéterminée (`malloc`) ; l'écrire depuis `def` avant de le lire |
| `pico8_nr_struct_init` | 2.15d (structs de sorte) | réduite à l'appel de `pico8_qh_struct_init` ; `pico8_nr_t`, `pico8_fr_t`, `pico8_pm_t` n'ont plus de membre propre |
| `pico8_ke_struct_init(self, cls, me, t)` | tout constructeur neuf | reçoit `me`/`t` de l'archétype ; un type sans archétype passe ses littéraux (le faux type de `test_pico8_iv.c` : `"ph", 0`) |
| `défauts` | toutes | dans `COMPOSITION_RESIDUE_GUARD` : un `pico8_X_start` neuf, même dans un TU, rougit `make test` |
| `test_pico8_pj.c`, `ed_fixed(ju) == 1` | la première sous-étape qui touche ce fichier | tautologie toujours là (non touché par 2.13e) |

### Anti-tassement

**Interdit** :
- réécrire une comparaison du test ponctuel vers `def` au lieu de la retirer ; retirer une comparaison dont la chaîne vit encore (e1 : celles de `ga`, `nr`, `fr` et les `me`/`t` ; e2a : les `me`/`t`) ; retirer le fichier avant e2c ;
- basculer un lecteur de composant sur `def`, ou retirer un champ A d'un composant, avant e2b (avant, `ga` écrit encore `ki`, `pk`, `fc`) ;
- garder un champ A dans l'instance « pour les TU » après son commit ; faire lire `def` à un lecteur de `hf`, `fa`, `kq` avant e2a ;
- faire écrire par l'init de `ga` `pn`, `du`, `ki` ou `pk` (avis de `05daecc`) ; retirer le zéro de ces membres dans `pico8_ga_def` ;
- écrire un `def` (instance, sonde, émetteur) ailleurs que dans l'init de son porteur — **TU compris** (`probe_with_zero_dw` passe par `pico8_g__struct_init`) ; laisser un `def` NULL après e2a ;
- un cast, un `void *`, un aiguillage par classe pour atteindre `def` ;
- nommer un `pico8_X_start` retiré, même en commentaire ;
- supprimer le test ponctuel avant d'avoir relogé l'`assert` du `ra` partagé de `nf` ;
- changer une constante `*_HASH`/`FROZEN_*`, un `*.sha256`, `tests/e2e/`, `tools/` ; toucher le `Makefile` hors e2c.

## Implementation Chunks

Quatre commits, dans l'ordre, chacun vert. **≈ 50 min chacun**, preuves comprises : ≈ 15 min d'écriture et de compilation (les diffs sont donnés), les jumelles en arrière-plan à ≈ 13 s l'une (e1 : 110, ≈ 24 min ; e2a : 64, ≈ 14 min ; e2b : 156, ≈ 34 min ; e2c : 84, ≈ 18 min), ≈ 10 min pour les preuves 2, 4 à 9. 2.13e d'un bloc aurait pris ≈ 3 h 20 : au-dessus du seuil, d'où la découpe en quatre (G1). Budget du plan pour 2.13 (≈ 7 × 50 min) : 2.13 compte dix commits au lieu de sept (d et e2 découpées), déclaré.

1. **2.13e1** : appliquer le diff de e1 ; `make test`, `make e2e`, compteur, `--champs` ; preuves 1 à 8 (jumelles C, N1) ; note du plan ; commit.
2. **2.13e2a** : idem avec le diff de e2a (jumelles D, N2).
3. **2.13e2b** : idem avec le diff de e2b (jumelles F, N3, N4).
4. **2.13e2c** : `git rm tests/test_pico8_archetype_chain.c`, diff de e2c ; preuves 1 à 9 (jumelles G, S ; X2) ; note du plan ; commit.

**Repli écrit** (si un commit déborde de 1,5 fois son budget avant d'être fait), chacun un commit vert :
- e1 en **e1a** (`hj` et `mx`) puis **e1b** (`pm`) → 97 ;
- e2b en **e2b-sonde** (lecteurs `probe.c:168`, `:333`, `:335`, `:337`, `planet_display.c:469`, cinq membres, `test_pico8_jc.c`) puis **e2b-émetteur** (`probe.c:368`, `:369`, `particle.c:149`, trois membres) ;
- e2c en **e2c-ke** (`pico8_ke_start` sort, `défauts` → 0, sans garde, le test ponctuel réduit à zéro comparaison et son total à 0) puis **e2c-garde** (retrait du fichier, `COMPOSITION_RESIDUE_GUARD`, X2).
Comptes intermédiaires non mesurés ; le repli, s'il sert, est écrit dans la note « Découvert à ».

## Testing Decisions

- **Aucun TU neuf.** Une aide (`assert_emitter`, e1 ; `assert_box`, e2a) et des littéraux dans `jo_spec_defaults`, une fonction dans `spec_defaults` (`test_nf_ra_is_shared`, e2c), un archétype de test dans `test_pico8_jc.c` (e2b), une fixture dans `test_composition_residue.c` (e2c) ; le reste est réécriture. Aucun rejeu d'`events.json` dans un TU.
- **Bon test ici** : un TU durable compare une valeur à un **littéral du cart** (`assert_box(&ga->jo.def->kq, 0, 0, -1, -1)`, `strcmp(bg->header.me, "ph") == 0`) ou épingle une pose (`em->def == &def->emitter`), jamais l'archétype à lui-même. Les comparaisons instance/`def` d'un champ qui ne vit plus que dans `def` sont retirées, pas gardées vraies par construction.
- **Prior art** : preuves 1 à 8 du PRD 2.13d ; jumelles « par ligne ajoutée » (G5 de 2.13c).

### Preuves (parents : `p` pour e1, puis chaque commit pour le suivant)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré aux quatre commits.

2. **Règle 1, preuve `const`** (copie du commit ; le membre `def` du composant rendu `*const def`, un à la fois, puis `cc -std=c99 -fsyntax-only -Isrc` de chaque `.c` de `src/` et `tests/`, `-DPICO8_CLASS_COVERAGE` pour les deux `class_coverage`, l'erreur `raylib.h` ôtée) : `pico8_emitter_t.def` → **trois** erreurs aux quatre commits, les poses (`probe.c:443`, `planet_family.c:269` et `:527` à e1 — cette dernière encore `= NULL` —, `:269`/`:526` à e2a, `:264`/`:518` à e2b et e2c) ; `pico8_probe_t.def` → **quatre** aux quatre commits (`planet_family.c:65`, `probe.c:448`, `slipway.c:403`, `wormhole.c:246` ; `:402`/`:245` à e2c) ; aucune dans `tests/` (`probe_with_zero_dw` pose par `pico8_g__struct_init`) ; l'initialiseur de `pb` n'est pas une affectation. `pico8_jo_t.def` → une (inchangé depuis 2.13d). Preuve négative à e2c : `h->emitter->def->ki = 0;` (`pico8_hj_t *`), `q->probe->def->dw = 0;` (`pico8_qh_t *`), `r->nr.qh.jo.def->mt = 0;` (`pico8_fr_t *`) → `assignment of member … in read-only object` ×3. Les A retirés n'ont plus de membre (`--champs`, compilation).

3. **Mutations, règle du parent.**
   - **C (e1), D (e2a), G (e2c)** : une jumelle par comparaison qui quitte le test ponctuel (55, 32, 36). Parent : **une ligne ajoutée** dans le `_new` de la sorte mesurée, juste avant `#ifdef PICO8_CLASS_COVERAGE` (après toute l'init) ; enfant : le membre dans `pico8_X_def`, même valeur neuve. `make -k -j8 unit` puis `make -k e2e` ; restaurer par `git checkout -- <fichier>`. **Vert à l'enfant et rouge au parent (hors `archetype_chain`) = ÉCHEC** ; une comparaison de `jo_spec_defaults` à un `_start` compte comme rouge au parent. e2e = conformes sur 26.
   - **F (e2b)** : aucune comparaison ne part en e2b ; ce qui change est le chemin de lecture. Une jumelle par champ A de composant et par porteur (sonde : 5 champs × 12 porteurs ; émetteur : 3 × 6), **la même mutation de l'archétype aux deux bouts** (parent : la copie de l'init la porte dans l'instance, que le lecteur lit ; enfant : le lecteur lit `def`). ÉCHEC si l'ensemble des rouges diffère (un filet qui ne voyait que l'instance, comme `test_pico8_jc.c:853`, serait perdu).
   - **S (e2c)** : les six `assert` nus du test ponctuel, la même mutation de l'init aux deux bouts.
   - **N, E** : épingles et poses.

   **Jumelles de e1** (parent `p`, enfant `e1`) :

| # | Valeur | Parent : ligne ajoutée avant `#ifdef PICO8_CLASS_COVERAGE` du `_new` | Rouges au parent (hors `archetype_chain`) | Enfant : membre de `pico8_X_def` | Rouges à l'enfant | e2e p / c |
|---|---|---|---|---|---|---|
| C1 | `hj.hp` | `self->qh.hp = 0x30000;` | `hj`, `jo_spec_defaults`, `mx` | `.hp = 0x20000` → `.hp = 0x30000` | `hj`, `mx` | 25 / 25 |
| C2 | `hj.px` | `self->qh.px = &pico8_os_px_row;` | `hj`, `jo_spec_defaults` | `.px = &pico8_hj_px_row` → `.px = &pico8_os_px_row` | `hj` | 26 / 26 |
| C3 | `hj.qh` | `self->qh.qh = 1;` | `hj`, `jo_spec_defaults` | `.qh = 0` → `.qh = 1` | `hj` | 26 / 26 |
| C4 | `hj.s` | `self->qh.s = 0xa70000;` | `hj`, `jo_spec_defaults` | `.s = 0xa60000` → `.s = 0xa70000` | `entity_kind`, `hj` | 25 / 25 |
| C5 | `hj.probe.od` | `self->qh.probe->od = 0x3d0000;` | `hj`, `jo_spec_defaults` | `.od = 0x3c0000` → `.od = 0x3d0000` | `entity_kind`, `hj` | 25 / 25 |
| C6 | `hj.rr` | `self->rr = pico8_nl_def.rr;` | `hj`, `jo_spec_defaults` | `.rr = hj_rr_text` → `.rr = nl_rr_text` | `entity_kind`, `hj`, `map_tables` | 26 / 26 |
| C7 | `hj.emitter.pn` | `self->emitter->pn = 0x60000;` | `hj`, `jo_spec_defaults` | `.pn = 0x50000` → `.pn = 0x60000` | `hj` | 26 / 26 |
| C8 | `hj.emitter.du` | `self->emitter->du = 0x88000;` | `hj`, `jo_spec_defaults` | `.du = 0x78000` → `.du = 0x88000` | `hj` | 26 / 26 |
| C9 | `hj.emitter.py` | `self->emitter->py = 0x11eb8;` | `hj`, `jo_spec_defaults` | `.py = 0x1eb8` → `.py = 0x11eb8` | `hj` | 26 / 26 |
| C10 | `hj.emitter.ki` | `self->emitter->ki = 0xa0000;` | `hj`, `jo_spec_defaults` | `.ki = 0x90000` → `.ki = 0xa0000` | `hj` | 26 / 26 |
| C11 | `hj.emitter.pk` | `self->emitter->pk = 0x0;` | `hj`, `jo_spec_defaults` | `.pk = 0x10000` → `.pk = 0x0` | `hj` | 26 / 26 |
| C12 | `hj.emitter.fc` | `self->emitter->fc = 0x18000;` | `hj`, `jo_spec_defaults` | `.fc = 0x8000` → `.fc = 0x18000` | `hj` | 26 / 26 |
| C13 | `hj.emitter.k_` | `self->emitter->k_ = &pico8_mx_k__seq;` | `hj`, `jo_spec_defaults` | `.k_ = &pico8_hj_k__seq` → `.k_ = &pico8_mx_k__seq` | `hj` | 26 / 26 |
| C14 | `hj.emitter.qo` | `self->emitter->qo = 0x1fae0;` | `hj`, `jo_spec_defaults` | `.qo = 0xfae0` → `.qo = 0x1fae0` | `hj` | 26 / 26 |
| C15 | `hj.emitter.ro` | `self->emitter->ro = 0xee14;` | `hj`, `jo_spec_defaults` | `.ro = -0x11ec` → `.ro = 0xee14` | `hj` | 26 / 26 |
| C16 | `hj.emitter.qg` | `self->emitter->qg = 0x111ea;` | `hj`, `jo_spec_defaults` | `.qg = 0x11ea` → `.qg = 0x111ea` | `hj` | 26 / 26 |
| C17 | `hj.emitter.q_` | `self->emitter->q_ = 0x1fae0;` | `hj`, `jo_spec_defaults` | `.q_ = 0xfae0` → `.q_ = 0x1fae0` | `hj` | 26 / 26 |
| C18 | `pm.me` | `self->hj.qh.jo.header.me = pico8_qh_def.me;` | `hj`, `hp`, `jo_spec_defaults`, `pm`, `qh`, `ui` | `.me = pm_me_text` → `.me = qh_me_text` | `entity_kind`, `hj`, `hp`, `map_tables`, `pm`, `qh`, `ui` | 25 / 25 |
| C19 | `pm.hp` | `self->hj.qh.hp = 0x40000;` | `hj`, `hp`, `jo_spec_defaults`, `pm`, `qh` | `.hp = 0x30000` → `.hp = 0x40000` | `hj`, `hp`, `pm`, `qh` | 25 / 25 |
| C20 | `pm.px` | `self->hj.qh.px = &pico8_os_px_row;` | `hj`, `jo_spec_defaults`, `pm` | `.px = &pico8_pm_px_row` → `.px = &pico8_os_px_row` | `hj`, `pm` | 26 / 26 |
| C21 | `pm.qh` | `self->hj.qh.qh = 1;` | `hj`, `jo_spec_defaults`, `pm` | `.qh = 0` → `.qh = 1` | `hj`, `pm` | 26 / 26 |
| C22 | `pm.s` | `self->hj.qh.s = 0x630000;` | `ga`, `hj`, `jo_spec_defaults`, `pm` | `.s = 0x620000` → `.s = 0x630000` | `entity_kind`, `ga`, `hj`, `pm` | 25 / 25 |
| C23 | `pm.probe.od` | `self->hj.qh.probe->od = 0x3d0000;` | `hj`, `jo_spec_defaults`, `pm` | `.od = 0x3c0000` → `.od = 0x3d0000` | `entity_kind`, `hj`, `pm` | 26 / 26 |
| C24 | `pm.rr` | `self->hj.rr = pico8_nl_def.rr;` | `hj`, `jo_spec_defaults`, `pm` | `.rr = hj_rr_text` → `.rr = nl_rr_text` | `entity_kind`, `hj`, `pm` | 26 / 26 |
| C25 | `pm.ng` | `{ static const pico8_vec_t mut = {0x20000, 0x0}; self->ng = &mut; }` | `ga`, `jo_spec_defaults`, `pm` | `.ng = {0x10000, 0x0}` → `.ng = {0x20000, 0x0}` | `ga`, `pm` | 25 / 25 |
| C26 | `pm.fz` | `self->fz = pico8_pm_start.fz + 1;` | `jo_spec_defaults`, `pm` | `.fz = pm_fz_text` → `.fz = pm_fz_text + 1` | `pm` | 26 / 26 |
| C27 | `pm.emitter.pn` | `self->hj.emitter->pn = 0x90000;` | `hj`, `jo_spec_defaults`, `pm` | `.pn = 0x80000` → `.pn = 0x90000` | `hj`, `pm` | 26 / 26 |
| C28 | `pm.emitter.du` | `self->hj.emitter->du = 0x30000;` | `hj`, `jo_spec_defaults`, `pm` | `.du = 0x20000` → `.du = 0x30000` | `hj`, `pm` | 25 / 25 |
| C29 | `pm.emitter.py` | `self->hj.emitter->py = 0x10f5c;` | `hj`, `jo_spec_defaults`, `pm` | `.py = 0xf5c` → `.py = 0x10f5c` | `hj`, `pm` | 25 / 25 |
| C30 | `pm.emitter.ki` | `self->hj.emitter->ki = 0xa0000;` | `hj`, `jo_spec_defaults`, `pm` | `.ki = 0x90000` → `.ki = 0xa0000` | `hj`, `pm` | 26 / 26 |
| C31 | `pm.emitter.pk` | `self->hj.emitter->pk = 0x10000;` | `hj`, `jo_spec_defaults`, `pm` | `.pk = 0x0` → `.pk = 0x10000` | `hj`, `pm` | 26 / 26 |
| C32 | `pm.emitter.fc` | `self->hj.emitter->fc = 0x20000;` | `jo_spec_defaults`, `pm` | `.fc = 0x10000` → `.fc = 0x20000` | `pm` | 25 / 25 |
| C33 | `pm.emitter.k_` | `self->hj.emitter->k_ = &pico8_hj_k__seq;` | `jo_spec_defaults`, `pm` | `.k_ = &pico8_pm_k__seq` → `.k_ = &pico8_hj_k__seq` | `pm` | 26 / 26 |
| C34 | `pm.emitter.qo` | `self->hj.emitter->qo = 0x2051e;` | `jo_spec_defaults`, `pm` | `.qo = 0x1051e` → `.qo = 0x2051e` | `pm` | 25 / 25 |
| C35 | `pm.emitter.ro` | `self->hj.emitter->ro = 0xf290;` | `jo_spec_defaults`, `pm` | `.ro = -0xd70` → `.ro = 0xf290` | `pm` | 25 / 25 |
| C36 | `pm.emitter.qg` | `self->hj.emitter->qg = 0x10d70;` | `jo_spec_defaults`, `pm` | `.qg = 0xd70` → `.qg = 0x10d70` | `pm` | 26 / 26 |
| C37 | `pm.emitter.q_` | `self->hj.emitter->q_ = 0x2051e;` | `jo_spec_defaults`, `pm` | `.q_ = 0x1051e` → `.q_ = 0x2051e` | `pm` | 25 / 25 |
| C38 | `mx.hp` | `self->hj.qh.hp = 0x60000;` | `hj`, `jo_spec_defaults`, `mx` | `.hp = 0x50000` → `.hp = 0x60000` | `hj`, `mx` | 25 / 25 |
| C39 | `mx.s` | `self->hj.qh.s = 0x430000;` | `hj`, `jo_spec_defaults`, `mx` | `.s = 0x420000` → `.s = 0x430000` | `entity_kind`, `hj`, `mx` | 25 / 25 |
| C40 | `mx.px` | `self->hj.qh.px = &pico8_os_px_row;` | `hj`, `jo_spec_defaults`, `mx` | `.px = &pico8_mx_px_row` → `.px = &pico8_os_px_row` | `hj`, `mx` | 26 / 26 |
| C41 | `mx.probe.od` | `self->hj.qh.probe->od = 0x290000;` | `hj`, `jo_spec_defaults`, `mx` | `.od = 0x280000` → `.od = 0x290000` | `entity_kind`, `hj`, `mx` | 25 / 25 |
| C42 | `mx.qh` | `self->hj.qh.qh = 1;` | `hj`, `jo_spec_defaults`, `mx` | `.qh = 0` → `.qh = 1` | `hj`, `mx` | 26 / 26 |
| C43 | `mx.rr` | `self->hj.rr = pico8_nl_def.rr;` | `hj`, `jo_spec_defaults`, `mx` | `.rr = mx_rr_text` → `.rr = nl_rr_text` | `entity_kind`, `hj`, `map_tables`, `mx` | 26 / 26 |
| C44 | `mx.ht` | `self->ht = &pico8_nr_hz_seq;` | `jo_spec_defaults`, `mx` | `.ht = &pico8_mx_ht_seq` → `.ht = &pico8_nr_hz_seq` | `mx` | 25 / 25 |
| C45 | `mx.emitter.pn` | `self->hj.emitter->pn = 0x60000;` | `hj`, `jo_spec_defaults`, `mx` | `.pn = 0x50000` → `.pn = 0x60000` | `hj`, `mx` | 25 / 25 |
| C46 | `mx.emitter.du` | `self->hj.emitter->du = 0x60000;` | `hj`, `jo_spec_defaults`, `mx` | `.du = 0x50000` → `.du = 0x60000` | `hj`, `mx` | 25 / 25 |
| C47 | `mx.emitter.py` | `self->hj.emitter->py = 0x11eb8;` | `hj`, `jo_spec_defaults`, `mx` | `.py = 0x1eb8` → `.py = 0x11eb8` | `hj`, `mx` | 25 / 25 |
| C48 | `mx.emitter.ki` | `self->hj.emitter->ki = 0xa0000;` | `hj`, `jo_spec_defaults`, `mx` | `.ki = 0x90000` → `.ki = 0xa0000` | `hj`, `mx` | 26 / 26 |
| C49 | `mx.emitter.pk` | `self->hj.emitter->pk = 0x0;` | `hj`, `jo_spec_defaults`, `mx` | `.pk = 0x10000` → `.pk = 0x0` | `hj`, `mx` | 25 / 25 |
| C50 | `mx.emitter.fc` | `self->hj.emitter->fc = 0x14ccc;` | `jo_spec_defaults`, `mx` | `.fc = 0x4ccc` → `.fc = 0x14ccc` | `mx` | 25 / 25 |
| C51 | `mx.emitter.k_` | `self->hj.emitter->k_ = &pico8_hj_k__seq;` | `jo_spec_defaults`, `mx` | `.k_ = &pico8_mx_k__seq` → `.k_ = &pico8_hj_k__seq` | `mx` | 26 / 26 |
| C52 | `mx.emitter.qo` | `self->hj.emitter->qo = 0x1fae0;` | `jo_spec_defaults`, `mx` | `.qo = 0xfae0` → `.qo = 0x1fae0` | `mx` | 25 / 25 |
| C53 | `mx.emitter.ro` | `self->hj.emitter->ro = 0xee14;` | `jo_spec_defaults`, `mx` | `.ro = -0x11ec` → `.ro = 0xee14` | `mx` | 25 / 25 |
| C54 | `mx.emitter.qg` | `self->hj.emitter->qg = 0x111ea;` | `jo_spec_defaults`, `mx` | `.qg = 0x11ea` → `.qg = 0x111ea` | `mx` | 25 / 25 |
| C55 | `mx.emitter.q_` | `self->hj.emitter->q_ = 0x1fae0;` | `jo_spec_defaults`, `mx` | `.q_ = 0xfae0` → `.q_ = 0x1fae0` | `mx` | 25 / 25 |

   **Jumelles de e2a** (parent `e1`, enfant `e2a`) :

| # | Valeur | Parent : ligne ajoutée avant `#ifdef PICO8_CLASS_COVERAGE` du `_new` | Rouges au parent (hors `archetype_chain`) | Enfant : membre de `pico8_X_def` | Rouges à l'enfant | e2e p / c |
|---|---|---|---|---|---|---|
| D1 | `ga.hf` | `self->jo.hf = 1;` | `ga`, `jo_spec_defaults` | `.hf = 0` → `.hf = 1` | `ga` | 26 / 26 |
| D2 | `ga.fa` | `self->jo.fa = 0x40000;` | `ga`, `jo_spec_defaults` | `.fa = 0x30000` → `.fa = 0x40000` | `ga` | 26 / 26 |
| D3 | `ga.kq` | `{ static const pico8_qz_box_t mut = {0x10000, 0x0, -0x10000, -0x10000}; self->jo.kq = &mut; }` | `ga`, `jo_spec_defaults` | `.kq = {0x0, 0x0, -0x10000, -0x10000}` → `.kq = {0x10000, 0x0, -0x10000, -0x10000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| D4 | `ga.fz` | `self->fz = pico8_ga_start.fz + 1;` | `ga`, `jo_spec_defaults`, `pm` | `.fz = ga_fz_text` → `.fz = ga_fz_text + 1` | `ga`, `pm` | 26 / 26 |
| D5 | `ga.iu` | `self->iu = 0xf0a4;` | `ga`, `jo_spec_defaults`, `pm` | `.iu = -0xf5c` → `.iu = 0xf0a4` | `ga` | 24 / 25 |
| D6 | `ga.ms` | `self->ms = 0x40000;` | `ga`, `hp`, `jo_spec_defaults`, `pm` | `.ms = 0x30000` → `.ms = 0x40000` | `ga`, `hp` | 24 / 25 |
| D7 | `ga.emitter.pn` | `self->emitter->pn = 0x10000;` | **aucun** | `.pn = 0x0` → `.pn = 0x10000` | **aucun** | 26 / 26 |
| D8 | `ga.emitter.du` | `self->emitter->du = 0x10000;` | **aucun** | `.du = 0x0` → `.du = 0x10000` | **aucun** | 26 / 26 |
| D9 | `ga.emitter.py` | `self->emitter->py = 0x70000;` | `ga`, `hp`, `jo_spec_defaults`, `pm` | `.py = 0x60000` → `.py = 0x70000` | `ga`, `hp` | 24 / 25 |
| D10 | `ga.emitter.ki` | `self->emitter->ki = 0x10000;` | **aucun** | `.ki = 0x0` → `.ki = 0x10000` | **aucun** | 26 / 26 |
| D11 | `ga.emitter.pk` | `self->emitter->pk = 0x10000;` | **aucun** | `.pk = 0x0` → `.pk = 0x10000` | **aucun** | 26 / 26 |
| D12 | `ga.emitter.fc` | `self->emitter->fc = 0x10000;` | `ga`, `jo_spec_defaults`, `pm` | `.fc = 0x0` → `.fc = 0x10000` | `ga`, `pm` | 24 / 24 |
| D13 | `ga.emitter.k_` | `self->emitter->k_ = &pico8_hj_k__seq;` | `ga`, `hp`, `jo_spec_defaults`, `pm` | `.k_ = &pico8_ga_k__seq` → `.k_ = &pico8_hj_k__seq` | `ga`, `hp` | 24 / 25 |
| D14 | `ga.emitter.qo` | `self->emitter->qo = 0x1e666;` | `ga`, `jo_spec_defaults`, `pm` | `.qo = 0xe666` → `.qo = 0x1e666` | `ga` | 24 / 25 |
| D15 | `ga.emitter.ro` | `self->emitter->ro = 0x10000;` | `ga`, `jo_spec_defaults`, `pm` | `.ro = 0x0` → `.ro = 0x10000` | `ga` | 24 / 25 |
| D16 | `ga.emitter.qg` | `self->emitter->qg = 0x10000;` | `ga`, `jo_spec_defaults`, `pm` | `.qg = 0x0` → `.qg = 0x10000` | `ga` | 24 / 25 |
| D17 | `ga.emitter.q_` | `self->emitter->q_ = 0x1e666;` | `ga`, `jo_spec_defaults`, `pm` | `.q_ = 0xe666` → `.q_ = 0x1e666` | `ga` | 24 / 25 |
| D18 | `nr.kq` | `{ static const pico8_qz_box_t mut = {-0x50000, -0x40000, 0x50000, 0x50000}; self->qh.jo.kq = &mut; }` | `jo_spec_defaults`, `nr` | `.kq = {-0x40000, -0x40000, 0x50000, 0x50000}` → `.kq = {-0x50000, -0x40000, 0x50000, 0x50000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| D19 | `nr.s` | `self->qh.s = 0xab0000;` | `jo_spec_defaults`, `nr` | `.s = 0xaa0000` → `.s = 0xab0000` | `entity_kind`, `nr` | 24 / 24 |
| D20 | `nr.probe.dw` | `self->qh.probe->dw = 0x28f;` | `jo_spec_defaults`, `nr`, `qh` | `.dw = 0x28e` → `.dw = 0x28f` | `nr`, `qh` | 26 / 26 |
| D21 | `nr.probe.od` | `self->qh.probe->od = 0xb0000;` | `jo_spec_defaults`, `nr` | `.od = 0xa0000` → `.od = 0xb0000` | `entity_kind`, `nr` | 24 / 24 |
| D22 | `nr.probe.fp` | `self->qh.probe->fp = 0x20000;` | `jo_spec_defaults`, `nr`, `qh` | `.fp = 0x10000` → `.fp = 0x20000` | `nr`, `qh` | 24 / 24 |
| D23 | `nr.hz` | `self->hz = &pico8_fr_hz_seq;` | `fr`, `jo_spec_defaults`, `nr` | `.hz = &pico8_nr_hz_seq` → `.hz = &pico8_fr_hz_seq` | `fr`, `nr` | 24 / 24 |
| D24 | `nr.rr` | `self->rr = pico8_nr_start.rr + 1;` | `jo_spec_defaults`, `nr` | `.rr = nr_rr_text` → `.rr = nr_rr_text + 1` | `entity_kind`, `nr` | 26 / 26 |
| D25 | `fr.kq` | `{ static const pico8_qz_box_t mut = {-0x50000, -0x40000, 0x50000, 0x50000}; self->nr.qh.jo.kq = &mut; }` | `fr`, `jo_spec_defaults` | `.kq = {-0x40000, -0x40000, 0x50000, 0x50000}` → `.kq = {-0x50000, -0x40000, 0x50000, 0x50000}` | `jo_spec_defaults` (filet neuf) | 26 / 26 |
| D26 | `fr.s` | `self->nr.qh.s = 0x4f0000;` | `fr`, `jo_spec_defaults`, `nr` | `.s = 0x4e0000` → `.s = 0x4f0000` | `entity_kind`, `fr`, `nr` | 25 / 25 |
| D27 | `fr.probe.dw` | `self->nr.qh.probe->dw = 0x28f;` | `fr`, `jo_spec_defaults`, `qh` | `.dw = 0x28e` → `.dw = 0x28f` | `fr`, `qh` | 26 / 26 |
| D28 | `fr.probe.od` | `self->nr.qh.probe->od = 0x1a0000;` | `fr`, `jo_spec_defaults` | `.od = 0x190000` → `.od = 0x1a0000` | `entity_kind`, `fr` | 25 / 25 |
| D29 | `fr.probe.fp` | `self->nr.qh.probe->fp = 0x20000;` | `fr`, `jo_spec_defaults`, `qh` | `.fp = 0x10000` → `.fp = 0x20000` | `fr`, `qh` | 25 / 25 |
| D30 | `fr.hz` | `self->nr.hz = &pico8_nr_hz_seq;` | `fr`, `jo_spec_defaults`, `nr` | `.hz = &pico8_fr_hz_seq` → `.hz = &pico8_nr_hz_seq` | `fr`, `nr` | 25 / 25 |
| D31 | `fr.rr` | `self->nr.rr = pico8_fr_start.rr + 1;` | `fr`, `jo_spec_defaults` | `.rr = fr_rr_text` → `.rr = fr_rr_text + 1` | `entity_kind`, `fr` | 26 / 26 |
| D32 | `fr.mt` | `self->mt = 0x20000;` | `fr`, `jo_spec_defaults` | `.mt = 0x10000` → `.mt = 0x20000` | `fr` | 26 / 26 |

   **Jumelles de e2b** (parent `e2a`, enfant `e2b`, même mutation) :

| # | Valeur | Mutation (dans l'archétype, aux deux bouts) | Rouges au parent | Rouges à l'enfant | e2e p / c |
|---|---|---|---|---|---|
| F1 | `mz.probe.dw` | `.dw = 0x0` → `.dw = 0x1` | `mz` | `mz` | 26 / 26 |
| F2 | `mz.probe.dq` | `.dq = "pr"` → `.dq = "ly"` | `mz` | `mz` | 9 / 9 |
| F3 | `mz.probe.fp` | `.fp = 0x10000` → `.fp = 0x20000` | `mz` | `mz` | 9 / 9 |
| F4 | `mz.probe.ik` | `.ik = 0x310000` → `.ik = 0x320000` | `mz` | `mz` | 26 / 26 |
| F5 | `mz.probe.has_ik` | `.has_ik = 1` → `.has_ik = 0` | `mz` | `mz` | 26 / 26 |
| F6 | `jc.probe.dw` | `.dw = 0x106` → `.dw = 0x107` | `jc` | `jc` | 26 / 26 |
| F7 | `jc.probe.dq` | `.dq = NULL` → `.dq = "pr"` | `jc` | `jc` | 26 / 26 |
| F8 | `jc.probe.fp` | `.fp = 0x0` → `.fp = 0x10000` | `hp`, `hy`, `jc`, `lq` | `hp`, `hy`, `jc`, `lq` | 17 / 17 |
| F9 | `jc.probe.ik` | `.ik = 0x300000` → `.ik = 0x310000` | `jc` | `jc` | 26 / 26 |
| F10 | `jc.probe.has_ik` | `.has_ik = 1` → `.has_ik = 0` | `jc` | `jc` | 26 / 26 |
| F11 | `g_.probe.dw` | `.dw = 0x666` → `.dw = 0x667` | `g_` | `g_` | 26 / 26 |
| F12 | `g_.probe.dq` | `.dq = "ly"` → `.dq = "pr"` | `g_` | `g_` | 26 / 26 |
| F13 | `g_.probe.fp` | `.fp = 0x0` → `.fp = 0x10000` | `g_` | `g_` | 8 / 8 |
| F14 | `g_.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `spec_defaults` | `spec_defaults` | 26 / 26 |
| F15 | `g_.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `spec_defaults` | `spec_defaults` | 26 / 26 |
| F16 | `fv.probe.dw` | `.dw = 0x666` → `.dw = 0x667` | `fv`, `g_` | `fv`, `g_` | 26 / 26 |
| F17 | `fv.probe.dq` | `.dq = "ly"` → `.dq = "pr"` | `fv`, `g_` | `fv`, `g_` | 26 / 26 |
| F18 | `fv.probe.fp` | `.fp = 0x0` → `.fp = 0x10000` | `fv`, `g_` | `fv`, `g_` | 26 / 26 |
| F19 | `fv.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `spec_defaults` | `spec_defaults` | 26 / 26 |
| F20 | `fv.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `spec_defaults` | `spec_defaults` | 26 / 26 |
| F21 | `qh.probe.dw` | `.dw = 0x666` → `.dw = 0x667` | `qh` | `qh` | 26 / 26 |
| F22 | `qh.probe.dq` | `.dq = qh_dq_text` → `.dq = "pr"` | `map_tables`, `qh` | `map_tables`, `qh` | 24 / 24 |
| F23 | `qh.probe.fp` | `.fp = 0x20000` → `.fp = 0x30000` | `qh` | `qh` | 23 / 23 |
| F24 | `qh.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F25 | `qh.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F26 | `nl.probe.dw` | `.dw = 0x666` → `.dw = 0x667` | `nl`, `qh` | `nl`, `qh` | 26 / 26 |
| F27 | `nl.probe.dq` | `.dq = qh_dq_text` → `.dq = "pr"` | `nl`, `qh` | `nl`, `qh` | 25 / 25 |
| F28 | `nl.probe.fp` | `.fp = 0x20000` → `.fp = 0x30000` | `nl`, `qh` | `nl`, `qh` | 25 / 25 |
| F29 | `nl.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F30 | `nl.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F31 | `os.probe.dw` | `.dw = 0x666` → `.dw = 0x667` | `os`, `qh` | `os`, `qh` | 26 / 26 |
| F32 | `os.probe.dq` | `.dq = qh_dq_text` → `.dq = "pr"` | `os`, `qh` | `os`, `qh` | 25 / 25 |
| F33 | `os.probe.fp` | `.fp = 0x20000` → `.fp = 0x30000` | `os`, `qh` | `os`, `qh` | 25 / 25 |
| F34 | `os.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F35 | `os.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F36 | `hj.probe.dw` | `.dw = 0x666` → `.dw = 0x667` | `hj`, `qh` | `hj`, `qh` | 26 / 26 |
| F37 | `hj.probe.dq` | `.dq = qh_dq_text` → `.dq = "pr"` | `hj`, `qh` | `hj`, `qh` | 25 / 25 |
| F38 | `hj.probe.fp` | `.fp = 0x20000` → `.fp = 0x30000` | `hj`, `qh` | `hj`, `qh` | 25 / 25 |
| F39 | `hj.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F40 | `hj.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F41 | `pm.probe.dw` | `.dw = 0x666` → `.dw = 0x667` | `pm`, `qh` | `pm`, `qh` | 26 / 26 |
| F42 | `pm.probe.dq` | `.dq = qh_dq_text` → `.dq = "pr"` | `pm`, `qh` | `pm`, `qh` | 26 / 26 |
| F43 | `pm.probe.fp` | `.fp = 0x20000` → `.fp = 0x30000` | `pm`, `qh` | `pm`, `qh` | 26 / 26 |
| F44 | `pm.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F45 | `pm.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F46 | `mx.probe.dw` | `.dw = 0x666` → `.dw = 0x667` | `mx`, `qh` | `mx`, `qh` | 26 / 26 |
| F47 | `mx.probe.dq` | `.dq = qh_dq_text` → `.dq = "pr"` | `mx`, `qh` | `mx`, `qh` | 25 / 25 |
| F48 | `mx.probe.fp` | `.fp = 0x20000` → `.fp = 0x30000` | `mx`, `qh` | `mx`, `qh` | 25 / 25 |
| F49 | `mx.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F50 | `mx.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F51 | `nr.probe.dw` | `.dw = 0x28e` → `.dw = 0x28f` | `nr`, `qh` | `nr`, `qh` | 26 / 26 |
| F52 | `nr.probe.dq` | `.dq = qh_dq_text` → `.dq = "pr"` | `nr`, `qh` | `nr`, `qh` | 24 / 24 |
| F53 | `nr.probe.fp` | `.fp = 0x10000` → `.fp = 0x20000` | `nr`, `qh` | `nr`, `qh` | 24 / 24 |
| F54 | `nr.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F55 | `nr.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F56 | `fr.probe.dw` | `.dw = 0x28e` → `.dw = 0x28f` | `fr`, `qh` | `fr`, `qh` | 26 / 26 |
| F57 | `fr.probe.dq` | `.dq = qh_dq_text` → `.dq = "pr"` | `fr`, `qh` | `fr`, `qh` | 25 / 25 |
| F58 | `fr.probe.fp` | `.fp = 0x10000` → `.fp = 0x20000` | `fr`, `qh` | `fr`, `qh` | 25 / 25 |
| F59 | `fr.probe.ik` | `.ik = 0x0` → `.ik = 0x10000` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F60 | `fr.probe.has_ik` | `.has_ik = 0` → `.has_ik = 1` | `jo_spec_defaults` | `jo_spec_defaults` | 26 / 26 |
| F61 | `g_.emitter.ki` | `.ki = 0x90000` → `.ki = 0xa0000` | `g_` | `g_` | 9 / 9 |
| F62 | `g_.emitter.pk` | `.pk = 0x10000` → `.pk = 0x0` | `g_` | `g_` | 9 / 9 |
| F63 | `g_.emitter.fc` | `.fc = 0x10000` → `.fc = 0x20000` | `g_` | `g_` | 8 / 8 |
| F64 | `fv.emitter.ki` | `.ki = 0x110000` → `.ki = 0x120000` | `fv`, `g_` | `fv`, `g_` | 26 / 26 |
| F65 | `fv.emitter.pk` | `.pk = 0x10000` → `.pk = 0x0` | `fv`, `g_` | `fv`, `g_` | 26 / 26 |
| F66 | `fv.emitter.fc` | `.fc = 0x50000` → `.fc = 0x60000` | `fv`, `g_` | `fv`, `g_` | 25 / 25 |
| F67 | `hj.emitter.ki` | `.ki = 0x90000` → `.ki = 0xa0000` | `hj` | `hj` | 26 / 26 |
| F68 | `hj.emitter.pk` | `.pk = 0x10000` → `.pk = 0x0` | `hj` | `hj` | 26 / 26 |
| F69 | `hj.emitter.fc` | `.fc = 0x8000` → `.fc = 0x18000` | `hj` | `hj` | 26 / 26 |
| F70 | `pm.emitter.ki` | `.ki = 0x90000` → `.ki = 0xa0000` | `hj`, `pm` | `hj`, `pm` | 26 / 26 |
| F71 | `pm.emitter.pk` | `.pk = 0x0` → `.pk = 0x10000` | `hj`, `pm` | `hj`, `pm` | 26 / 26 |
| F72 | `pm.emitter.fc` | `.fc = 0x10000` → `.fc = 0x20000` | `pm` | `pm` | 25 / 25 |
| F73 | `mx.emitter.ki` | `.ki = 0x90000` → `.ki = 0xa0000` | `hj`, `mx` | `hj`, `mx` | 26 / 26 |
| F74 | `mx.emitter.pk` | `.pk = 0x10000` → `.pk = 0x0` | `hj`, `mx` | `hj`, `mx` | 25 / 25 |
| F75 | `mx.emitter.fc` | `.fc = 0x4ccc` → `.fc = 0x14ccc` | `mx` | `mx` | 25 / 25 |
| F76 | `ga.emitter.ki` | `.ki = 0x0` → `.ki = 0x10000` | **aucun** | **aucun** | 26 / 26 |
| F77 | `ga.emitter.pk` | `.pk = 0x0` → `.pk = 0x10000` | **aucun** | **aucun** | 26 / 26 |
| F78 | `ga.emitter.fc` | `.fc = 0x0` → `.fc = 0x10000` | `ga`, `pm` | `ga`, `pm` | 24 / 24 |

   **Jumelles de e2c** (parent `e2b`, enfant `e2c`) :

| # | Valeur | Parent : ligne ajoutée avant `#ifdef PICO8_CLASS_COVERAGE` du `_new` | Rouges au parent (hors `archetype_chain`) | Enfant : membre de `pico8_X_def` | Rouges à l'enfant | e2e p / c |
|---|---|---|---|---|---|---|
| G1 | `bg.me` | `self->header.me = "pq";` | `bg`, `spec_defaults`, `ui` | `.me = "ph"` → `.me = "pq"` | `bg`, `spec_defaults`, `ui` | 26 / 26 |
| G2 | `bg.t` | `self->header.t = 0x10000;` | `bg`, `spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `bg`, `spec_defaults`, `ui` | 26 / 26 |
| G3 | `ui.me` | `self->header.me = "pq";` | `ui` | `.me = "ph"` → `.me = "pq"` | `ui` | 26 / 26 |
| G4 | `ui.t` | `self->header.t = 0x10000;` | `ui` | `.t = 0x0` → `.t = 0x10000` | `ui` | 26 / 26 |
| G5 | `js.me` | `self->header.me = "pq";` | `js`, `lq`, `spec_defaults`, `ui` | `.me = "ph"` → `.me = "pq"` | `js`, `lq`, `spec_defaults`, `ui` | 0 / 0 |
| G6 | `js.t` | `self->header.t = 0x10000;` | `js`, `spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `js`, `spec_defaults`, `ui` | 26 / 26 |
| G7 | `fq.me` | `self->header.me = "pq";` | `fq`, `ui` | `.me = "ph"` → `.me = "pq"` | `fq`, `ui` | 26 / 26 |
| G8 | `fq.t` | `self->header.t = 0x10000;` | `fq`, `ui` | `.t = 0x0` → `.t = 0x10000` | `fq`, `ui` | 26 / 26 |
| G9 | `qb.me` | `self->header.me = "pq";` | `pm`, `qb`, `spec_defaults`, `ui` | `.me = "ph"` → `.me = "pq"` | `pm`, `qb`, `spec_defaults`, `ui` | 22 / 22 |
| G10 | `qb.t` | `self->header.t = 0x10000;` | `pm`, `qb`, `spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `pm`, `qb`, `spec_defaults`, `ui` | 24 / 24 |
| G11 | `nf.me` | `self->header.me = "pq";` | `bg`, `fv`, `nf`, `qb`, `replay_from_title`, `ui` | `.me = "ph"` → `.me = "pq"` | `bg`, `fv`, `nf`, `qb`, `replay_from_title`, `ui` | 0 / 0 |
| G12 | `nf.t` | `self->header.t = 0x10000;` | `ui` | `.t = 0x0` → `.t = 0x10000` | `ui` | 26 / 26 |
| G13 | `mh.me` | `self->header.me = "pq";` | `mh`, `ui` | `.me = "ph"` → `.me = "pq"` | `mh`, `ui` | 26 / 26 |
| G14 | `mh.t` | `self->header.t = 0x10000;` | `mh`, `ui` | `.t = 0x0` → `.t = 0x10000` | `mh`, `ui` | 15 / 15 |
| G15 | `iv.me` | `self->header.me = "pq";` | `iv`, `jc`, `ui` | `.me = "ph"` → `.me = "pq"` | `iv`, `jc`, `ui` | 7 / 7 |
| G16 | `iv.t` | `self->header.t = 0x10000;` | `iv`, `ui` | `.t = 0x0` → `.t = 0x10000` | `iv`, `ui` | 26 / 26 |
| G17 | `ie.me` | `self->header.me = "pq";` | `ie`, `spec_defaults`, `ui` | `.me = "ph"` → `.me = "pq"` | `ie`, `spec_defaults`, `ui` | 26 / 26 |
| G18 | `ie.t` | `self->header.t = 0x10000;` | `ie`, `spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `ie`, `spec_defaults`, `ui` | 26 / 26 |
| G19 | `pj.me` | `self->header.me = "pq";` | `pj`, `replay_from_title`, `spec_defaults`, `ui` | `.me = "ph"` → `.me = "pq"` | `pj`, `replay_from_title`, `spec_defaults`, `ui` | 23 / 23 |
| G20 | `pj.t` | `self->header.t = 0x10000;` | `pj`, `spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `pj`, `spec_defaults`, `ui` | 26 / 26 |
| G21 | `mz.t` | `self->header.t = 0x10000;` | `mz`, `spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `mz`, `spec_defaults`, `ui` | 9 / 9 |
| G22 | `jc.t` | `self->header.t = 0x10000;` | `hp`, `jc`, `lq`, `ui` | `.t = 0x0` → `.t = 0x10000` | `hp`, `jc`, `lq`, `ui` | 18 / 18 |
| G23 | `g_.me` | `self->header.me = "pq";` | `g_`, `spec_defaults`, `ui` | `.me = "ph"` → `.me = "pq"` | `g_`, `spec_defaults`, `ui` | 26 / 26 |
| G24 | `g_.t` | `self->header.t = 0x10000;` | `g_`, `probe_mm_rnd_order`, `spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `g_`, `spec_defaults`, `ui` | 11 / 11 |
| G25 | `fv.me` | `self->g_.header.me = "pq";` | `fv`, `spec_defaults`, `ui` | `.me = "ph"` → `.me = "pq"` | `fv`, `spec_defaults`, `ui` | 26 / 26 |
| G26 | `fv.t` | `self->g_.header.t = 0x10000;` | `fv`, `g_`, `spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `fv`, `g_`, `spec_defaults`, `ui` | 26 / 26 |
| G27 | `jo.t` | `self->header.t = 0x10000;` | `jo`, `jo_spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `jo`, `jo_spec_defaults`, `ui` | 26 / 26 |
| G28 | `qh.t` | `self->jo.header.t = 0x10000;` | `jo_spec_defaults`, `qh`, `ui` | `.t = 0x0` → `.t = 0x10000` | `jo_spec_defaults`, `qh`, `ui` | 26 / 26 |
| G29 | `nl.t` | `self->qh.jo.header.t = 0x10000;` | `jo_spec_defaults`, `nl`, `ui` | `.t = 0x0` → `.t = 0x10000` | `jo_spec_defaults`, `nl`, `ui` | 26 / 26 |
| G30 | `os.t` | `self->qh.jo.header.t = 0x10000;` | `jo_spec_defaults`, `os`, `ui` | `.t = 0x0` → `.t = 0x10000` | `jo_spec_defaults`, `os`, `ui` | 26 / 26 |
| G31 | `hj.t` | `self->qh.jo.header.t = 0x10000;` | `hj`, `jo_spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `hj`, `jo_spec_defaults`, `ui` | 26 / 26 |
| G32 | `pm.t` | `self->hj.qh.jo.header.t = 0x10000;` | `hj`, `jo_spec_defaults`, `pm`, `ui` | `.t = 0x0` → `.t = 0x10000` | `hj`, `jo_spec_defaults`, `pm`, `ui` | 25 / 25 |
| G33 | `mx.t` | `self->hj.qh.jo.header.t = 0x10000;` | `hj`, `jo_spec_defaults`, `mx`, `ui` | `.t = 0x0` → `.t = 0x10000` | `hj`, `jo_spec_defaults`, `mx`, `ui` | 26 / 26 |
| G34 | `ga.t` | `self->jo.header.t = 0x10000;` | `ga`, `jo_spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `ga`, `jo_spec_defaults`, `ui` | 26 / 26 |
| G35 | `nr.t` | `self->qh.jo.header.t = 0x10000;` | `jo_spec_defaults`, `nr`, `ui` | `.t = 0x0` → `.t = 0x10000` | `jo_spec_defaults`, `nr`, `ui` | 26 / 26 |
| G36 | `fr.t` | `self->nr.qh.jo.header.t = 0x10000;` | `fr`, `jo_spec_defaults`, `ui` | `.t = 0x0` → `.t = 0x10000` | `fr`, `jo_spec_defaults`, `ui` | 26 / 26 |

   **Lignes S de e2c** (parent `e2b`, enfant `e2c`, même mutation) :

| # | Valeur (`assert` nu du test ponctuel) | Mutation (aux deux bouts) | Rouges au parent (hors `archetype_chain`) | Rouges à l'enfant | e2e p / c |
|---|---|---|---|---|---|
| S1 | nf.ra partagé | `self->ra = &g_nf_ra;` → `self->ra = calloc(1, sizeof *self->ra);` | **aucun** | `spec_defaults` (filet neuf) | 26 / 26 |
| S2 | mz.jj partagé | `self->jj = &g_mz_jj_wrapper;` → `{ pico8_mz_jj_t *o = malloc(sizeof *o); *o = g_mz_jj_wrapper; self->jj = o; }` | `mz`, `spec_defaults` | `mz`, `spec_defaults` | 26 / 26 |
| S3 | ui.le partagé | `self->le = &g_shared_le;` → `self->le = calloc(1, sizeof *self->le);` | `ui` | `ui` | 26 / 26 |
| S4 | jo.kw partagé | `self->kw = &g_jo_kw_shared;` → `self->kw = &self->kw_own;` | `ga`, `jo`, `jo_spec_defaults` | `ga`, `jo`, `jo_spec_defaults` | 26 / 26 |
| S5 | by NULL | `self->by = NULL;` → `self->by = calloc(1, sizeof *self->by);` | `fr`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` | `fr`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` | 26 / 26 |
| S6 | et NULL (arbre qh) | `self->probe->et = NULL;` → `self->probe->et = &self->jo.header;` | `fr`, `hj`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` | `fr`, `hj`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` | 26 / 26 |

   **Lignes N** :

   | # | Valeur | Mutation | Commit | Rouges | e2e |
   |---|---|---|---|---|---|
   | N1 | `def` d'émetteur de `hj`/`pm`/`mx` | `&self->qh.jo.def->emitter` → `&pico8_qh_def.emitter` (`pico8_hj_struct_init`) | e1 | `jo_spec_defaults` seul | 26 |
   | N2 | `def` d'émetteur de `ga` | `&self->jo.def->emitter` → `&pico8_hj_def.emitter` | e2a | `jo_spec_defaults` seul | 26 |
   | N3 | `def` de `pb` | `{.def = &pico8_probe_def_none}` → `{.def = NULL}` | e2b | **aucun** (attendu : aucun lecteur, G5) | 26 |
   | N4 | idem | → `{.def = &pico8_qh_def.probe}` | e2b | **aucun** (idem) | 26 |

   Lecture : **aucun ÉCHEC** aux quatre commits. **Filets uniques trouvés** : e1, aucun (les 55 jumelles rougissent leur TU de sorte à l'enfant) ; e2a, la boîte `kq` de `ga`, `nr`, `fr` (D3, D18, D25 : au parent, l'identité `jo->kq == &pico8_X_start.kq` de leur TU ; à l'enfant, seuls les littéraux neufs de `jo_spec_defaults`, prévus par le contrat) ; `emitter.pn`, `du`, `ki`, `pk` de `ga` (D7, D8, D10, D11) : rouges au **seul** test ponctuel au parent, verts à l'enfant — valeurs mortes, sans filet par décision (`05daecc`), pas un ÉCHEC par la règle (rouge au parent hors `archetype_chain` : aucun) ; e2b, aucun (78 paires identiques ; les `ik`/`has_ik` sont vus par leurs littéraux, aux deux bouts) ; e2c, `nf.ra` partagé (S1, relogé). **G24** (`g_.t`) : au parent, `probe_mm_rnd_order` rougit en plus, parce que la ligne ajoutée écrase la clé `t` que ce TU passe à `pico8_g__new` ; l'archétype, lui, est réécrit par la clé : l'écart vient de la ligne ajoutée, pas d'un filet perdu (G7).

4. **Chemin touché** : `make line-coverage EVENTS="<les 26 e2e>"` sur `e2c` (6184 lignes) : les poses d'émetteur (`planet_family.c:264`, `:518`), `pico8_ke_struct_init` (`ke.c:37`), les lecteurs basculés (`probe.c:168`, `:333`, `:335`, `:337`, `:368`, `:369`, `particle.c:30`, `:149`, `planet_display.c:155`, `:205`, `:469`, `planet.c:93`, `:272`, `:325`, `economy_score.c:49`, `:110`, `economy_tiers.c:279`, `planet_family_trade.c:93`, `:223`), la copie de `ht` (`planet_family_trade.c:315`) et la lecture de `ng` (`planet_family.c:311`) sont exécutés.

5. **Valeurs observables, parent contre enfant** : un programme d'essai (`obs.c`, hors dépôt) lié aux sources de `src/` (sans `main`, sans raylib) crée une instance fraîche des dix sortes, de `g_`, `fv`, `mz`, `jc` et des dix classes de la famille a, et imprime **348** lignes : `me`, `t` de chacune ; pour l'arbre `jo`, `hf`, `fa`, `get_fa`, `kq` et `get_kq`, `hs`, `ma`, `mi`/`has_mi`, `kw` partagé, `by` ; pour l'arbre `qh`, `hp`, `px`, `qh`, `s`, `ba`, la sonde (`dw`, `dq`, `fp`, `has_ik`/`ik`, `has_od`/`od`, `et`) ; les émetteurs (`pn`, `du`, `ki`, `pk` sauf pour `ga`, `py`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`) ; `rr`, `ng`, `fz`, `ht`, `hz`, `mt`, `iu`, `ms`, `qw`. Compilé `-DV=0` sur `p`, `-DV=1` sur `e1`, `-DV=2` sur `e2a`, `-DV=3` sur `e2b` et `e2c` (lecture par `def` de ce qui a quitté l'instance) : sorties **identiques**. `make e2e` sous `MALLOC_PERTURB_=165` à `e2c` : 26 conformes (les `pn`/`du` non écrits de `ga` ne sont lus par personne).

6. **§6 de l'étude 2.9** : non rejoué, `jo.gq` n'existe plus depuis 2.13d1 et aucun des deux TU (`planet_gh_row_lq`, `ga_export_panel_by`) n'est touché.

7. `class-coverage-net` : **oui**, vert aux quatre commits (`couverture: OK`, sortie égale à `tools/class_coverage_net_expected.txt`). `sanitize-net` : **oui** (`pico8_qh_t`, `pico8_hj_t`, `pico8_pm_t`, `pico8_jo_t`, `pico8_ga_t`, `pico8_nr_t`, `pico8_fr_t`, `pico8_probe_t`, `pico8_emitter_t` changent de taille), `sanitize: 26 propre(s)` aux quatre.

8. `game_data.h` mesuré au commit : **473**, **434**, **437**, **429**.

9. **Garde et compteur (e2c)** : `make composition-guard` rend `résidu gardé (noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts) : 0 ligne(s)` ; **garde mordante** : `int pico8_foo_start;` ajouté à `tests/test_pico8_iv.c` sur la copie rougit `make composition-guard`, donc `make test` (`défauts: tests/test_pico8_iv.c:288`, `1 ligne(s)`) ; retiré, vert. **X2**, reconstruite (la réserve de `7d7f194` n'en garde que la description) : dans `tools/composition_residue.sh`, la clé de dédoublonnage des lignes `ptrs-entité`, `(k, rline[k, j])` (le type, donc son fichier, et la ligne), réduite au numéro de ligne (`rline[k, j]`) ; elle **survit** à `test_composition_residue` au parent (vert), et `test_ptrs_entite_keeps_one_line_number_in_two_files` (deux types d'entité de deux fichiers, un membre `void **` à la ligne 6 de chacun) la **tue** (rouge : `résidu gardé (ptrs-entité) : 1 ligne(s)` au lieu de 2) ; restaurée, verte. Deux autres lectures de « clé sans le fichier » essayées (`cut -d: -f2` au lieu de `-f1,2` pour le pied et pour `--check`) sont déjà tuées par la fixture existante.

### Greps de Sortie

À chaque commit :
1. `grep -rnE 'pico8_(hj|pm|mx)_start' src tests` → vide à e1 ; `grep -rnE 'pico8_(ga|nr|fr)_start' src tests` → vide à e2a ; `grep -rnE '\bpico8_[a-z0-9_]+_start(_t)?\b' src tests | wc -l` → **97**, **27**, **27**, **0**.
2. `grep -rn 'const pico8_[a-z_]*_def_t \*def;' src | wc -l` → **18** ; `grep -rnE 'self->def = (&pico8_[a-z_]*_def|def);' src | wc -l` → **16** (inchangés depuis 2.13d).
3. `grep -rnE 'emitter->def = |probe->def = |\.def = ' src` → à e1 : `planet_family.c:65` (`&self->jo.def->probe`), `:269` (`&self->qh.jo.def->emitter`), `:527` (`NULL`), `probe.c:443`, `:448`, `research_icon.c:180`, `slipway.c:403`, `wormhole.c:246` ; à e2a, `:526` devient `&self->jo.def->emitter` (plus aucun `NULL`) ; à e2b, `:264`, `:518` ; à e2c, `slipway.c:402`, `wormhole.c:245`.
4. `grep -n 'pico8_probe_t pb = ' src/pico8_research_icon.c` → exactement `pico8_probe_t pb = {.def = &pico8_probe_def_none};` (épingle de `pb`, G5).
5. `grep -rnE 'probe->def->|emitter->def->|\)->def->dw' src` → vide à e1 et e2a ; à e2b et e2c exactement : `particle.c:149`, `probe.c:168`, `:333`, `:335`, `:337`, `:368`, `:369`, `planet_display.c:469`.
6. `grep -rnE 'probe->(dw|dq|fp|ik|has_ik) =|emitter->(ki|pk|fc) =' src tests | wc -l` → 81, 72, **0**, 0 (96 à `p`).
7. Casts et tests de classe : `grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests | wc -l` → 532, 532, **530**, 530 ; `grep -rEo '(->cls|header\.cls|_struct_class\(\))' src tests | wc -l` → 707, 707, **708**, 708.
8. `grep -n 'g_compared ==' tests/test_pico8_archetype_chain.c` → **68**, **36**, 36 ; à e2c le fichier n'existe plus, et `grep -rn archetype_chain Makefile tests src` → vide.
9. `make composition-residue` → pied **501 / 72**, **431 / 70**, **431 / 70**, **404 / 63** ; `défauts` 97 14, 27 8, 27 8, absent ; `inclusion-mutuelle 17 16` ; `--champs` → `249 ; 15 ; 20`, puis `242 ; 15 ; 20` trois fois.
10. `git diff --stat <parent>` : les fichiers de la section « Modules » et `docs/plan-3.md`, rien d'autre ; en particulier ni `tools/`, ni `tests/e2e/`, ni `test_pico8_pj.c` ; le `Makefile` à e2c seulement.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable. Les jumelles ont tourné sur une première version des commits (`b4d6640`, `d65bca0`, `6b6d635`) ; les commits finaux `deaaaa5`, `6d01a04`, `4a1343a` n'en diffèrent que par trois commentaires (réserves de `bde509f`, `git diff` : `planet_family.h`, `planet_family_trade.c`, 10 + / 7 −), rejoués verts (ligne 20).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 689d4fd`, `git apply` des deux diffs du PRD 2.13d, deux commits | `p` | 0 | `28cf5fd`, `59f7d2d` | 3 s |
| 2 | `timeout 900 make -j8 test` ; `timeout 600 make e2e` ; compteur, `--champs`, `wc -l`, total, `--check défauts` | `p` | 0 | vert ; 26 ; 628 / 72 ; 224 / 15 ; 255 ; 536 ; 123 | 13 s ; 2 s |
| 3 | texte de e1 ; `make -k -j8 unit` | `e1` | 2 | erreurs dans 13 TU (membres retirés, `_start` nommés) ; `src/` compile | < 1 min |
| 4 | TU réécrits ; `make -j8 test`, `make e2e`, compteur ; commit | `e1` | 0 | vert ; 26 ; 501 / 72 ; 97 / 14 ; 249 ; 473 ; 68 | < 1 min |
| 5 | jumelles C au parent puis à l'enfant (`mut.py`, 110) | `p`, `e1` | 0 | tableau C ; C26 refaite (`pm_fz_text` inutilisé : `-Werror`, valeur `+ 1`) | ≈ 25 min |
| 6 | texte de e2a ; `make -k -j8 unit` | `e2a` | 2 | erreurs dans 14 TU | < 1 min |
| 7 | TU réécrits ; `make -j8 test`, `make e2e`, compteur ; commit | `e2a` | 0 | vert ; 26 ; 431 / 70 ; 27 / 8 ; 242 ; 434 ; 36 | < 1 min |
| 8 | jumelles D au parent et à l'enfant, N1 (`e1`), N2 (`e2a`) | `e1`, `e2a` | 0 | tableau D ; N1, N2 rouges | ≈ 15 min |
| 9 | texte de e2b ; `make -k -j8 unit` ; TU réécrits, `test_pico8_jc.c` ; `make -j8 test`, `make e2e`, compteur ; commit | `e2b` | 0 | erreurs dans 15 TU, puis vert ; 26 ; `BV_HASH` inchangé ; 530 / 708 | < 2 min |
| 10 | jumelles F aux deux bouts (156), N3, N4 | `e2a`, `e2b` | 0 | 78 paires identiques ; N3, N4 verts | ≈ 35 min |
| 11 | réserves de `bde509f` : trois commentaires, `commit --amend` de e1 et e2a, `cherry-pick` de e2b | `e1`…`e2b` | 0 | `deaaaa5`, `6d01a04`, `4a1343a` | < 1 min |
| 12 | texte de e2c, `git rm` du test ponctuel, `Makefile` ; `make -j8 test`, `make e2e`, compteur ; commit | `e2c` | 0 | vert ; 26 ; 404 / 63 ; `défauts` gardé, 0 | < 1 min |
| 13 | X2 : trois lectures de la clé essayées ; fixture ; mutant rejoué | `e2c` | 0 | `cut -f2` ×2 tuées ; `shown[rline]` survit, puis tuée | < 2 min |
| 14 | jumelles G aux deux bouts (72), lignes S (12) | `e2b`, `e2c` | 0 | tableaux G, S | ≈ 19 min |
| 15 | preuve `const` (2 membres × 4 commits) ; preuve négative | les quatre | 0 | 3 et 4 erreurs aux poses ; `read-only object` ×3 | < 2 min |
| 16 | `obs.c` (`-DV=0…3`) sur `p` et les quatre commits, `cmp` | les cinq | 0 | 348 lignes, identiques | < 3 min |
| 17 | `class-coverage-net`, `sanitize-net` | les quatre | 0 | `couverture: OK` ; `26 propre(s)` | ≈ 1 min chacun |
| 18 | `MALLOC_PERTURB_=165 make e2e` ; `make line-coverage EVENTS=<26 e2e>` | `e2c` | 0 | 26 ; 6184 lignes, celles de la preuve 4 présentes | < 1 min |
| 19 | greps de Sortie, attribution (`git grep -c`) | les cinq | 0 | ci-dessus | < 10 s |
| 20 | `git fetch` de `bde509f` ; `git apply --check` et `git apply` des quatre diffs (`git rm` avant le dernier) ; `git diff ef5b52c` | `bde509f` | 0 | quatre commits ; diff vide sur `src/`, `tests/`, `Makefile`, `tools/` | < 1 min |
| 21 | `rm -rf` de la copie | — | 0 | supprimée | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit 2.13d2 (ou un descendant qui ne touche que `docs/`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si e1, e2a, e2b, e2c ne sont pas quatre commits (hors repli écrit), dans cet ordre, chacun vert (`make test`, `make e2e` 26 conformes), ou si un `*.sha256`, `tests/e2e/`, `tools/` ou une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si une jumelle C, D ou G est verte à l'enfant et rouge au parent hors `archetype_chain` (une comparaison de `jo_spec_defaults` à un `_start` comptant comme rouge), si une paire F ou S n'a pas les mêmes rouges aux deux bouts, ou si un littéral de la preuve 3 manque (`assert_box` de `ga`/`nr`, `test_nf_ra_is_shared`, les `"ph"`/`0` de `spec_defaults`, les `ik`/`has_ik` sur `def`).
- `FAILED:` si une comparaison dont la chaîne vit encore quitte le test ponctuel, si une autre reste ou est réécrite vers `def`, si le total n'est pas 68 puis 36, ou si le fichier existe encore après e2c.
- `FAILED:` si un champ A que plus personne n'écrit reste membre à la fin de son commit, ou si un champ ou un lecteur bascule avant son commit (Anti-tassement).
- `FAILED:` si une sorte pose un autre archétype que le sien, si un `def` (instance, sonde, émetteur) est écrit hors de l'init de son porteur, TU compris (preuve 2), si un `def` reste NULL après e2a, ou si une pose n'est pas épinglée (N1, N2 verts).
- `FAILED:` si l'init de `ga` écrit `pn`, `du`, `ki` ou `pk`.
- `FAILED:` si un cast est ajouté, ou un test de classe autre que l'appel à `pico8_g__struct_class()` de `probe_with_zero_dw` ; si `inclusion-mutuelle` n'est pas 17 / 16 (43 lignes).
- `FAILED:` si `défauts` n'est pas 97, 27, 27, 0 ; si le pied n'est pas 501 / 72, 431 / 70, 431 / 70, 404 / 63 ; si `--champs` n'est pas `249 ; 15 ; 20` puis `242 ; 15 ; 20` ; si `défauts` n'est pas dans `COMPOSITION_RESIDUE_GUARD` à e2c.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge ; si les valeurs observables (preuve 5) diffèrent.
- `FAILED:` si X2 survit à e2c, ou si `tests/test_composition_residue.c:335` dépasse 80 colonnes.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un message de commit omet : les classes, les champs A retirés, les I copiés, les poses de `def`, la table des jumelles avec leur résultat au parent, la preuve `const`, les comptes, le tableau « Runs » ; pour e2c, la réserve `ed_fixed(ju) == 1` de `test_pico8_pj.c` reportée.
- `FAILED:` s'il manque une note « Découvert à 2.13e1 », « 2.13e2a », « 2.13e2b » ou « 2.13e2c » au bout des lignes 2.13e1/2.13e2 du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les liens du noyau (2.14, 2.15) ; les structs de sorte (2.15d), même devenues vides (`pico8_pm_t`, `pico8_nr_t`, `pico8_fr_t`, `pico8_os_t`).
- `tools/composition_residue.sh` (seul son TU change) ; la ligne `:261` de `test_composition_residue.c`, aussi à 81 colonnes, que la réserve ne nomme pas.
- `test_pico8_pj.c` (réserve reportée) ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. e2 découpée en trois** (e2a `ga`/`nr`/`fr`, e2b composants, e2c `ke`), chacune ≈ 50 min : d'un bloc, e2 prendrait ≈ 2 h 30. L'ordre est forcé : les lecteurs de composant ne basculent qu'une fois qu'aucun `_new` n'écrit plus un de leurs champs A (après e2a) ; `ke` part en dernier, avec le test ponctuel, dont les 36 dernières comparaisons ne dépendent que de lui.
- **G2. `pico8_ke_struct_init(self, cls, me, t)`** plutôt qu'une copie de `me`/`t` dans chaque init : une ligne par constructeur, et les trois réécritures de `header.me` (`jo`, `mz`, `jc`) partent. C'est la lecture de « `ke` entre dans chaque archétype » : `me`/`t` de `ke` sont déjà résolus dans les 24 archétypes qui les portent (`hp` et `hy` n'ont pas d'en-tête `ke`).
- **G3. `pn`/`du` de `ga` non écrits** (avis de `05daecc`) : valeurs mortes, restent membres parce que `g_` les écrit par clé ; le contrat pour la suite le signale.
- **G4. `pico8_nr_struct_init` gardée**, réduite à l'appel de `pico8_qh_struct_init` : le contrat de 2.13d interdit de réécrire le chemin du `def` ; 2.15d décidera.
- **G5. Épingle de `pb` par le texte** (grep de Sortie 4) : `pb` est une locale de `pico8_ie_kj` qu'aucun TU n'atteint, et aucun lecteur ne lit son `def` (N3, N4 verts, mesuré) ; un `assert` n'aurait rien à voir. La preuve `const` compte son initialiseur comme la seule pose hors affectation.
- **G6. 708 tests de classe à e2b** : `probe_with_zero_dw` a besoin de la classe de `g_` pour appeler `pico8_g__struct_init`, comme `test_pico8_g_.c` en 2.13c ; deux casts partent en échange.
- **G7. G24 lue comme conforme** : la ligne ajoutée au parent écrase aussi une clé de construction, ce que la mutation de l'archétype ne fait pas ; l'enfant rougit trois TU.
- **G8. Réserves du vérificateur de 2.13d traitées là où les fichiers sont touchés** (e1, e2a), et X2/`:335` en e2c, qui touche la garde.

### Surprises de mesure, pour `plan-pilot`

1. **Aucun filet unique neuf en e1 ni en e2b** : les 55 valeurs de `hj`/`pm`/`mx` et les 78 paires de composants sont vues par leurs TU de sorte ; seuls `kq` de `ga`/`nr`/`fr` (prévu) et `nf.ra` (prévu) demandent un littéral.
2. **`pn`, `du`, `ki`, `pk` de `ga`** : rouges au seul test ponctuel au parent, verts à l'enfant, conformes à l'avis de `05daecc`.
3. **La mutation X2 n'était décrite nulle part dans le dépôt** : reconstruite par essai (trois clés candidates, une seule survit).
4. **`pico8_pm_t`, `pico8_nr_t`, `pico8_fr_t` n'ont plus de membre propre** après 2.13e : matière pour 2.15d.

### Questions pour l'humain

Aucune.
