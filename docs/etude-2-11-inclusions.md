# Étude 2.11 — les paires d'inclusion mutuelle

Étude C de la phase 2 (`docs/plan-3.md`, ligne 2.11), en lecture seule, faite selon le PRD `plan3-phase2-etape11-etude-inclusions.md`. Elle fixe, pour chaque paire de modules qui s'emploient l'un l'autre, le sens coupé, la technique, l'étape qui la fait tomber, et l'essai sur copie qui prouve qu'aucune coupe ne crée de paire nouvelle.

Départ : `9ca66f4` (branche `claude/plan-pilot-3-7zdft0`, pied 1073 / 85). Aucune ligne de code n'a changé depuis `ced58a6`, où le PRD a été mesuré : **les numéros de ligne sont ceux de `9ca66f4` et de `ced58a6`**. Les essais sont faits sur une seule copie jetable (`git clone` local dans le scratchpad), jamais dans l'arbre de travail ; chaque essai part de la copie remise à zéro (`git checkout -- . && git clean -fd src tests`).

**Une coupe** (lecture de Q2, PRD) : la paire {A, B} est démêlée quand, dans un sens, A → B, aucun fichier du module A (`A.c`, `A.h`) n'emploie un nom que `B.h` déclare, et que `A.c`/`A.h` n'incluent plus `B.h`. Un nom déplacé compte pour le module qui le déclare après le déplacement. Retirer une ligne `#include` qui compile encore par un détour n'est pas une coupe. Le contrôle est le graphe d'usage (`usegraph.py`), confirmé par le compilateur.

## 0. État de départ

| Hypothèse | Mesure sur `9ca66f4` | Verdict |
|---|---|---|
| H-1 | `parente` 10/3, `lien-parent` 111/27, `accesseurs-q6` 237/33, `défauts` 669/52, `void-ptr` 8/4, `inclusion-mutuelle` **17/16** ; pied `résidu 2 : 1073 ligne(s), 85 fichier(s)` ; `--check inclusion-mutuelle` : **43 lignes** | vraie |
| H-2.9 | `docs/etude-2-9-archetypes.md` existe : place des archétypes (§2, `game_data.h/.c`, sauf `hy` dans `economy.h/.c`), chemin de lecture (ii), un `const pico8_X_def_t *def` dans l'instance ; lignes 2.12 et 2.13 réécrites dans le plan | vraie |
| H-2.10 | `docs/etude-2-10-composants.md` existe, avec la table du sort des symboles pour 2.11 (§4) ; lignes 2.14, 2.15, 2.16 et 2.11r réécrites dans le plan | vraie |

Les relevés `cross.py` et `usegraph.py` (annexe A, texte du PRD inchangé) rendent sur la copie de `9ca66f4` exactement les tables du PRD : 43 lignes, mêmes symboles ; **20 paires d'usage** (les 17 du compteur, plus `economy` ↔ `economy_score`, `economy` ↔ `tech_gate`, `entity_geom` ↔ `scroll`).

**Dépendance à la question Q-2.10** (étude 2.10, §5 : une douzaine de pointeurs de composant dans le noyau ; réponse par défaut « oui »). Trois paires tombent grâce aux liens du noyau posés par 2.14 : `particle` ↔ `probe` (liens `ra`/`fa`, 2.14b), `entity_geom` ↔ `planet` et `entity_geom` ↔ `scroll` (lien `ma`, 2.14c). Si la réponse est « non », ces trois fiches sont à refaire : les lectures resteraient des tests de type suivis d'un cast, et l'arête garderait ses symboles. Aucune autre coupe n'en dépend.

## 1. Le graphe (2.11a)

### Les 43 lignes du compteur, symbole par symbole

Relevé `cross.py` (noms que l'en-tête inclus déclare et que le fichier inclueur emploie). La colonne « Tombe » dit ce qui retire la ligne de la liste du compteur (voir §2 et §4).

| # | Ligne | En-tête | Symboles employés | Tombe |
|---|---|---|---|---|
| 1 | `economy.c:15` | `planet.h` | `pico8_jo_is_family`, `pico8_jo_t` | 2.17c (coupée) |
| 1 | `planet.c:11` | `economy.h` | `pico8_hy_nx` | 2.17c (paire démêlée, ligne gardée) |
| 2 | `economy_formulas.c:10` | `slipway.h` | `pico8_jc_end_p` | 2.17c (coupée) |
| 2 | `slipway.c:11` | `economy_formulas.h` | `pico8_dj` | 2.17c (gardée) |
| 3 | `entity_geom.c:9` | `planet.h` | `pico8_jo_is_family`, `pico8_jo_t` | 2.17e (coupée) |
| 3 | `planet.c:13` | `entity_geom.h` | `pico8_gd`, `pico8_gk` | 2.17e (gardée) |
| 4 | `particle.c:13` | `probe.h` | `pico8_g__of`, `pico8_g__t` | 2.14b (coupée) |
| 4 | `probe.c:18` | `particle.h` | `pico8_ei_dispatch`, `pico8_emitter_t`, `pico8_f_`, `pico8_qy_t` | 2.14b (gardée) |
| 4 | `probe.h:9` | `particle.h` | `pico8_emitter_t`, `pico8_qy_t` | 2.14b (gardée) |
| 5 | `planet.c:18` | `planet_display.h` | `pico8_jo_fe`, `pico8_jo_gv`, `pico8_jo_hb`, `pico8_jo_kj` | 2.17b (coupée) |
| 5 | `planet_display.c:17` | `planet.h` | `pico8_jo_cq`, `_dz`, `_gg`, `_hu`, `_ka`, `pico8_jo_t` | 2.17b (gardée) |
| 5 | `planet_display.h:7` | `planet.h` | `pico8_jo_t` | 2.17b (gardée) |
| 6 | `planet.c:19` | `planet_family.h` | `pico8_ga_kind`, `pico8_pm_kind`, `pico8_qh_of`, `pico8_qh_t` | 2.17e (coupée) |
| 6 | `planet_family.c:16` | `planet.h` | 21 symboles (dispatchers, `get_*`, `pq`, table, init) | 2.17e (gardée) |
| 6 | `planet_family.h:8` | `planet.h` | `pico8_jo_t` | 2.17e (gardée) |
| 7 | `planet.c:20` | `planet_trade.h` | `pico8_jo_cn`, `pico8_jo_kf`, `pico8_jo_trade_proposal` | 2.17b (coupée) |
| 7 | `planet_trade.c:10` | `planet.h` | `pico8_jo_is_family`, `pico8_jo_kd`, `pico8_jo_t` | 2.17b (gardée) |
| 7 | `planet_trade.h:6` | `planet.h` | `pico8_jo_t`, `pico8_jo_trade_proposal` | 2.17b (gardée) |
| 8 | `planet.c:21` | `probe.h` | `pico8_fm_hy` | 2.17d (coupée) |
| 8 | `probe.c:19` | `planet.h` | `pico8_jo_is_family`, `pico8_jo_struct_class`, `pico8_jo_t` | 2.17d (gardée) |
| 9 | `planet.c:23` | `slipway.h` | `pico8_jc_keys_t`, `pico8_jc_new`, `pico8_jc_ready` | 2.17b (coupée) |
| 9 | `slipway.c:16` | `planet.h` | `pico8_jo_is_family`, `pico8_jo_struct_class`, `pico8_jo_t` | 2.17b (gardée) |
| 10 | `planet_display.c:18` | `planet_family.h` | `pico8_nl_of`, `pico8_nl_t`, `pico8_qh_of`, `pico8_qh_t` | 2.15d (coupée) |
| 10 | `planet_family.c:17` | `planet_display.h` | `pico8_jo_kj` | 2.15d (gardée) |
| 11 | `probe.c:20` | `slipway.h` | `pico8_jc_end_p`, `pico8_jc_struct_class`, `pico8_jc_t`, `pico8_jc_too_far_struct_class`, `pico8_jc_too_far_t` | 2.17d (gardée) |
| 11 | `slipway.c:17` | `probe.h` | `pico8_d_`, `pico8_dl`, `pico8_fm_hy`, `pico8_probe_et_hang_fo`, `pico8_probe_t` | 2.17d (coupée) |
| 11 | `slipway.h:7` | `probe.h` | `pico8_probe_t` | 2.17d (coupée) |
| 12 | `probe.c:21` | `ui.h` | `pico8_je_nf`, `pico8_ui_hw` | 2.17d (gardée) |
| 12 | `ui.c:13` | `probe.h` | `pico8_g__struct_class` | 2.17d (coupée) |
| 13 | `probe.c:22` | `wormhole.h` | `pico8_mz_jj_struct_class`, `pico8_mz_jj_t`, `pico8_mz_struct_class`, `pico8_mz_t` | 2.17d (gardée) |
| 13 | `wormhole.c:13` | `probe.h` | `pico8_bv`, `pico8_d_`, `pico8_dl_dispatch`, `pico8_probe_t` | 2.17d (coupée) |
| 13 | `wormhole.h:11` | `probe.h` | `pico8_probe_t` | 2.17d (coupée) |
| 14 | `score_panel.c:21` | `score_panel_display.h` | `pico8_pj_hk`, `pico8_pj_ph`, `pico8_score_panel_display_init`, `_shutdown` | 2.17a (coupée) |
| 14 | `score_panel_display.c:14` | `score_panel.h` | 8 symboles (`pico8_pj_t`, `pico8_pj_ea`, `pico8_bz_row_t`…) | 2.17a (gardée) |
| 14 | `score_panel_display.h:6` | `score_panel.h` | `pico8_pj_t` | 2.17a (gardée) |
| 15 | `slipway.c:18` | `slipway_display.h` | `pico8_jc_kj` | 2.17a (coupée) |
| 15 | `slipway_display.c:10` | `slipway.h` | `pico8_jc_end_ra`, `pico8_jc_hg_has`, `pico8_jc_t` | 2.17a (gardée) |
| 15 | `slipway_display.h:8` | `slipway.h` | `pico8_jc_t` | 2.17a (gardée) |
| 16 | `slipway.c:20` | `ui.h` | `pico8_ok`, `pico8_ui_t` | 2.17d (gardée) |
| 16 | `ui.c:14` | `slipway.h` | `pico8_jc_struct_class` | 2.17d (coupée) |
| 17 | `wormhole.c:15` | `wormhole_display.h` | `pico8_mz_kj` | 2.17a (coupée) |
| 17 | `wormhole_display.c:9` | `wormhole.h` | `pico8_mz_ej`, `pico8_mz_t` | 2.17a (gardée) |
| 17 | `wormhole_display.h:8` | `wormhole.h` | `pico8_mz_t` | 2.17a (gardée) |

« Coupée » : la ligne est retirée (ou remplacée par un autre en-tête) par la coupe ; « gardée » : la ligne reste, elle sort de la liste parce que la paire n'existe plus. Une ligne gardée sort de la liste du compteur au même commit que la ligne coupée de sa paire.

### Paires d'usage

`usegraph.py` : A → B quand un fichier de A emploie un nom que `B.h` est seul à déclarer. 20 paires : les 17 du compteur (numérotées comme ci-dessus) et trois cachées, que le compteur ne voit pas parce qu'un sens passe par un détour.

| # | Paire | A → B | B → A |
|---|---|---|---|
| c1 | `economy` ↔ `economy_score` | `economy.c:220`, `:233`, `:236` (`pico8_hy_lj`) : `pico8_da`, `pico8_db`, `pico8_cz`, `pico8_bf` | `economy_score.c:189` (`pico8_ky`), `:232` (`pico8_bf`) : `pico8_hy_t`, reçu par `pico8_fm_hy` de `probe.h` sans inclure `economy.h` |
| c2 | `economy` ↔ `tech_gate` | `economy.c:96`, `:232`, `:280` : `pico8_ov`, `pico8_gi_nb` | `tech_gate.c:16`, `:38`, `:62` : `pico8_hy_t` (même détour) |
| c3 | `entity_geom` ↔ `scroll` | `entity_geom.c:49` (`pico8_gd`) : `pico8_qb_t` ; `:77`, `:79` (`pico8_ma`) : `pico8_qb_struct_class`, `pico8_qb_t` (par `entity.h:5` → `scroll.h`) | `scroll.c:131` (`pico8_qb_ph`) : `pico8_ma` |

Faux positifs, oublis : aucun trouvé à la lecture des sites (annexe B, `sites.py`, 324 lignes). Deux précisions : `cross.py` compte `pico8_jo_trade_proposal` pour `planet.c:20` (déclaration anticipée de `planet.h:127` employée par `pico8_jo_cn_dispatch`) que `usegraph.py` écarte (nom déclaré par deux en-têtes) ; la coupe le traite (le dispatcher part avec). `probe_of` (`planet_display.c:468-469`) est un membre de la table de classe (`struct.h`), pas un nom de `planet_family.h` : il ne pèse pas sur la paire 10.

### Détours

Les 14 lignes de `.c` que le PRD a trouvées retirables sans erreur (`economy.c:15`, `planet.c:11`, `:21`, `planet_display.c:17`, `planet_family.c:16`, `planet_trade.c:10`, `probe.c:18`, `score_panel_display.c:14`, `slipway.c:11`, `:17`, `slipway_display.c:10`, `ui.c:13`, `wormhole.c:13`, `wormhole_display.c:9`) ne sont jamais la coupe à elles seules : chaque coupe ci-dessous retire d'abord l'usage, et le graphe d'usage relancé le confirme. Contrôle : retirer `particle.c:13` seule (simulation comptable de 2.14b) donne 7 erreurs de compilation : ce n'est pas un détour, l'usage est réel jusqu'à 2.14b.

## 2. Fiches par paire (2.11a)

Techniques employées (catalogue du PRD, complété) :

- **T1** — dispatcher (ou méthode d'interaction) rangé avec l'affichage : la définition passe dans `X_display.c`, la déclaration dans `X.h` ; la table de classe reste où elle est et pointe le même nom.
- **T2** — déclaration déplacée : le prototype ou le type change d'en-tête, la définition ne bouge pas. **T2f** : en-tête feuille neuf `pico8_fm.h` (le créneau `fm` et le type `pico8_hy_t`). **T2p** : en-tête neuf `pico8_probe_comp.h` (le composant sonde et les fonctions que ses porteurs appellent).
- **T3** — signature qui prend des valeurs au lieu d'entités.
- **T4** — test de classe par le noyau. Les étiquettes ne suffisent pas (voir fiche 12) : l'identité de classe (`pico8_X_struct_class`) est déclarée dans `struct.h`, définie dans son module.
- **T5** — arête qui tombe avec 2.14/2.15 (dernier symbole retiré à une sous-étape nommée par 2.10).
- **T6** — fonction déplacée vers le module de ses seuls appelants ou de ses seules données (`pico8_na`, `jo_kv`, le pas `kr` de `hy:lj`).
- **T7** — lecture réécrite sur une fonction générique déjà là (`pico8_gd` lit `mo` par `pico8_mo()` et `pico8_entity_geom_ra`).

Les essais seuls sont faits chacun sur la copie remise à zéro : compilation `-fsyntax-only` de tous les `.c` de `src/` et `tests/` (les deux `class_coverage` sous `-DPICO8_CLASS_COVERAGE`, les trois fichiers raylib exclus), compteur (`--detail`, pied, `--check inclusion-mutuelle`), `usegraph.py`, comparés au départ par `cmp.py` (annexe A). « Aucune paire entrante » vaut pour les deux listes ; « catégories » : aucune autre catégorie ne bouge (en particulier `void-ptr` 8 / 4 partout : les dispatchers déplacés gardent leur nom en `_dispatch`).

### Fiche 1 — `economy` ↔ `planet` (2 lignes)

- **economy → planet** : `economy.c:219-226` (`pico8_hy_lj`, boucle sur `cg.jo`) : `assert(pico8_jo_is_family(p))`, cast `(pico8_jo_t *)p`, écriture de `jo->kr`, `jo->has_kr`.
- **planet → economy** : `planet.c:412` (`pico8_jo_hu`) : `pico8_hy_nx(pico8_fm_hy(), fp)`.
- **Sort des symboles (2.10)** : `pico8_jo_is_family` **reste** (corps sur le bit `JO` à 2.15b) ; le cast pour `kr` n'est pas touché (`kr` est un champ d'état de la planète) ; `pico8_hy_nx` reste. L'arête ne tombe pas seule.
- **Sens coupé : economy → planet.** `hy` est le gestionnaire, la planète l'entité : l'entité qui appelle le gestionnaire est le sens naturel ; l'autre sens n'est qu'une écriture de champ.
- **Technique : T6.** Le pas de boucle qui calcule `kr = flr(db(p) - da(p))`, l'`assert`, le cast et l'écriture deviennent `pico8_fixed_t pico8_kr(pico8_struct_header_t *p)` (nom provisoire), dans `economy_score` (qui définit déjà `pico8_da`/`pico8_db`, lit déjà `cg.jo` et inclut `planet.h`) ; `pico8_hy_lj` garde la boucle et la somme `dd`. `economy.c:15` retirée. `economy_score` → `planet` existait, `planet` → `economy_score` n'existe pas : pas de paire neuve. L'ordre d'évaluation de `db(p)` et `da(p)` reste celui d'une même expression.
- **Essai seul** (`kr_score`) : 0 erreur ; compteur 17 → **16 paires / 41 lignes** ; usage 20 → 19 ; sortante `economy` ↔ `planet` des deux listes ; aucune entrante ; pied 1071 / 84.
- **Preuve pour 2.17** : chemin touché (corps réécrit et déplacé).

### Fiche 2 — `economy_formulas` ↔ `slipway` (2 lignes)

- **economy_formulas → slipway** : `economy_formulas.c:133` (`pico8_dj`) : `pico8_jc_end_p(to)`, `pico8_jc_end_p(pb)`.
- **slipway → economy_formulas** : `slipway.c:139` (`pico8_jc_hq`) : `pico8_dj(self->qu, self->re)`.
- **Sort** : `pico8_jc_end_p` reste (2.14b ne réécrit que son corps, `slipway.c:55`).
- **Sens coupé : economy_formulas → slipway** (formule de coût, qui n'a besoin que de deux points).
- **Technique : T3.** `pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p)` ; l'appelant passe `pico8_jc_end_p(self->qu)`, `pico8_jc_end_p(self->re)` ; `economy_formulas.h` inclut `pico8_vec.h` ; `economy_formulas.c:10` retirée. `pico8_jc_end_p` lit seulement (`slipway.c:51-58`) : l'ordre d'évaluation des deux arguments, non spécifié avant comme après, est sans effet. Aucun TU n'appelle `pico8_dj`.
- **Essai seul** (`t3_dj`) : 0 erreur ; **16 / 41** ; usage 19 ; sortante `economy_formulas` ↔ `slipway` ; aucune entrante ; pied 1071 / 84.
- **Preuve pour 2.17** : chemin touché (signature changée, piège 5).

### Fiche 3 — `entity_geom` ↔ `planet` (2 lignes)

- **entity_geom → planet** : `entity_geom.c:61-62` (`pico8_na` : `assert(pico8_jo_is_family(self))`, `((pico8_jo_t *)self)->ra`) ; `:77`, `:79` (`pico8_ma` : test de famille, cast pour `ma`).
- **planet → entity_geom** : `planet.c:36` (`pico8_jo_hd_dispatch` : `pico8_gk`), `:466` (`pico8_jo_hk` : `pico8_gd`).
- **Sort (2.10)** : `:77-79` partent à **2.14c** (lien `ma`) ; `:61-62` **restent** (`pico8_na` écrit `ra->y` à travers le pointeur, que le lien `const` de 2.14b ne permet pas) ; `pico8_gk`, `pico8_gd` restent.
- **Sens coupé : entity_geom → planet** (module de géométrie générique ; `planet` → `entity_geom` est le sens naturel).
- **Technique : T6 + T5.** `pico8_na` n'a que deux appelants, `planet_family.c:26` et `:109` (`qh:ly`, `nl:ly`), et un TU (`test_pico8_fv.c:586`) ; elle ne lit que la planète : définition déplacée dans `planet.c`, déclaration dans `planet.h` (retirée de `entity_geom.h:24`). `planet_family` → `planet` existe déjà. Après 2.14c, `entity_geom` n'emploie plus aucun nom de `planet.h` ; `entity_geom.c:9` est retirée à 2.17e.
- **Essai seul** (`na_planet`) : 0 erreur ; compteur inchangé (17 / 43), usage 20 ; aucune entrante. Sites restants de l'arête (relevé `sites.py` sur la copie) : `entity_geom.c:64`, `:66`, tous deux dans `pico8_ma` (les `:77`, `:79` de départ), que 2.14c retire. **Dépend de Q-2.10** (lien `ma`).
- **Preuve pour 2.17** : corps de `pico8_na` identique à l'adresse près, `nm` ne diffère que par le module qui porte le symbole.

### Fiche 4 — `particle` ↔ `probe` (3 lignes)

- **particle → probe** : `particle.c:19` (`emitter_host_fa_ra`) : `pico8_g__of`, `pico8_g__t` (lecture de `ra`, `fa` d'un `g_`).
- **probe → particle** : `probe.c:353-360`, `:425`, `:493`, `:542`, `probe.h:44-60`, `:177` : `pico8_emitter_t`, `pico8_qy_t`, `pico8_ei_dispatch`, `pico8_f_` (le composant émetteur).
- **Sort (2.10)** : `pico8_g__of` au lecteur `particle.c:19` retiré à **2.14b** (liens `ra`, `fa`) ; `pico8_emitter_t`, `pico8_qy_t` viennent de `particle.h` même.
- **Sens coupé : particle → probe. Technique : T5**, à **2.14b**, à condition que 2.14b retire aussi `particle.c:13` (`#include "pico8_probe.h"`) dans le même commit : c'est la seule ligne de `particle` vers `probe` (`particle.h` n'inclut pas `probe.h`). Aucun autre nom de `probe.h` n'est employé par `particle` (graphe d'usage). La paire sort de 2.17.
- **Essai** : non simulé (le PRD l'exclut pour 2.14) ; retirer `particle.c:13` seule aujourd'hui donne 7 erreurs (usage réel), et `inclusion-mutuelle` 16 / 40 au compteur. **Dépend de Q-2.10.**

### Fiche 5 — `planet` ↔ `planet_display` (3 lignes)

- **planet → planet_display** : `planet.c:40` (`pico8_jo_hb_dispatch` : `pico8_jo_hb`), `:44` (`pico8_jo_gv_dispatch` : `pico8_jo_gv`), `:54` (`pico8_jo_kj_dispatch` : `pico8_jo_kj`), `:495` (`pico8_jo_ldown` : `pico8_jo_fe`, appel direct).
- **planet_display → planet** : `pico8_jo_t` partout, `pico8_jo_dz` (`:161`), `_cq` (`:240`), `_hu` (`:325`), `_gg` (`:445`), `_ka` (`:448`).
- **Sort** : rien ne part avec 2.14/2.15 (les dispatchers restent, typés `void *self` hors `hd`/`mm`, 2.16).
- **Sens coupé : planet → planet_display. Technique : T1**, élargi : les trois dispatchers et **`pico8_jo_ldown` avec son dispatcher** passent dans `planet_display.c` (ils y sont déjà déclarés par `planet.h:107-111`, `:155`). `pico8_jo_ldown` est la méthode de clic de la planète : `"known"` ouvre la fiche (`pico8_jo_fe`, affichage), `"ly"` lance une voie (`pico8_jc_new`) ; `pico8_jo_gv` (survol, même type de retour) est déjà dans `planet_display`. Le dispatcher seul ne suffisait pas (appel direct `:495`) ; déplacer `ldown` règle aussi la paire 9. `planet.c:18` retirée. `planet_display.c` inclut déjà `dispatch_context.h` et `slipway.h` ; `slipway` → `planet_display` n'existe pas.
- **Essai seul** (`t1_jo_ui`, avec la fiche 9) : 0 erreur ; **15 / 38** ; usage 18 ; sortantes `planet` ↔ `planet_display` et `planet` ↔ `slipway` des deux listes ; aucune entrante ; pied 1068 / 84.
- **Preuve pour 2.17** : corps identiques à l'adresse près ; `nm` : les cinq symboles changent de module, liaison inchangée (`T`).

### Fiche 6 — `planet` ↔ `planet_family` (3 lignes)

- **planet → planet_family** : `planet.c:269`, `:280` (`pico8_jo_pq` : `pico8_qh_t`, `pico8_qh_of`) ; `:362-363` (table `g_kv` : `pico8_pm_kind`, `pico8_ga_kind`, lue par `jo_kv`, appelée par `pico8_jo_hu`, `:394`).
- **planet_family → planet** : 21 symboles (dispatchers de `jo` dans les tables des sortes, `get_*`, `pico8_jo_pq`, `pico8_jo_struct_init`, `pico8_jo_struct_class`, `pico8_jo_ready`, `pico8_jo_t`) : couper ce sens est exclu (sous A, les sortes sont des tables de méthodes de la planète).
- **Sort (2.10)** : `pico8_qh_of` retiré à **2.15c** (bit `QH`), `pico8_qh_t` à **2.15d** (structs de sorte) ; `pico8_pm_kind`, `pico8_ga_kind` **restent** dans `planet_family`.
- **Sens coupé : planet → planet_family. Technique : T6 + T5.** Le registre `kv` (`g_kv` et `jo_kv`, `planet.c:352-376`) passe dans `planet_family.c` sous le nom `pico8_jo_kv` (provisoire), déclaré dans `planet.h` : il ne nomme que des sortes de `planet_family`. Après 2.15d, `planet.c` n'emploie plus aucun nom de `planet_family.h` ; `planet.c:19` est retirée à 2.17e. Sous A, fondre les deux modules n'est pas nécessaire.
- **Essai seul** (`kv_family`) : 0 erreur ; compteur inchangé (17 / 43), usage 20 ; aucune entrante. Sites restants de l'arête : `planet.c:269`, `:280` (`pico8_jo_pq`), que 2.15c/2.15d retirent. Ne dépend pas de Q-2.10 (le masque de lignée est un membre de la table de classe).
- **Preuve pour 2.17** : corps identiques à l'adresse près (`jo_kv` passe de `t` à `T`).

### Fiche 7 — `planet` ↔ `planet_trade` (3 lignes)

- **planet → planet_trade** : `planet.c:68` (`pico8_jo_kf_dispatch` : `pico8_jo_kf`), `:72` (`pico8_jo_cn_dispatch` : `pico8_jo_cn`, et `struct pico8_jo_trade_proposal`).
- **planet_trade → planet** : `planet_trade.c:44-72`, `:177`, `planet_trade.h:30-38` : `pico8_jo_t`, `pico8_jo_is_family`, `pico8_jo_kd`.
- **Sort** : rien ne part avec 2.14/2.15.
- **Sens coupé : planet → planet_trade. Technique : T1** : les deux dispatchers passent dans `planet_trade.c`, déjà déclarés par `planet.h:112`, `:127`. `planet.c:20` retirée.
- **Essai seul** (`t1_jo_trade`) : 0 erreur ; **16 / 40** ; usage 19 ; sortante `planet` ↔ `planet_trade` ; aucune entrante ; pied 1070 / 84.
- **Preuve pour 2.17** : corps identiques à l'adresse près ; `nm` : deux symboles changent de module.

### Fiche 8 — `planet` ↔ `probe` (2 lignes)

- **planet → probe** : `planet.c:412` (`pico8_jo_hu`) : `pico8_fm_hy` seul. `planet.c:21` est un détour (`planet_family.h:9` → `probe.h`).
- **probe → planet** : `probe.c:84-93` (`probe_host_set_ra_p`), `:108` (`g__is_target_class`), `:113-117` (`pico8_probe_et_hang_fo`), `:284-294` (`probe_host_raise_p_`) : `pico8_jo_is_family`, `pico8_jo_struct_class`, `pico8_jo_t`.
- **Sort (2.10)** : `:293` part à 2.14d1 ; `:92`, `:113` restent ; `pico8_fm_hy` reste.
- **Sens coupé : planet → probe. Technique : T2f.** `pico8_fm_hy` rend le `hy` d'`economy` mais est déclaré dans `probe.h:128`. Le déplacer vers `economy.h` recrée les paires cachées c1 et c2 (`economy_score`, `tech_gate` l'emploient) ; vers `economy_formulas.h`, il crée `planet` ↔ `economy_formulas` (`economy_formulas.c` emploie `pico8_jo_is_family`, `pico8_jo_t`) : **aucun en-tête existant ne convient**. D'où un en-tête feuille neuf, `pico8_fm.h` (nom provisoire, sans `.c`) : le type `pico8_hy_t` (reçoit l'étiquette `struct pico8_hy`, piège 3) et le créneau `fm` (`pico8_fm_placeholder_t`, `pico8_fm_placeholder_struct_class`, `pico8_fm`, `pico8_probe_set_fm`, `pico8_fm_hy`, `pico8_fm_placeholder`, avec leurs commentaires). Les définitions restent dans `probe.c`. `pico8_fm.h` inclut `economy_formulas.h` (pour `pico8_qe_map_t`), `fixed.h`, `ptrs.h`, `struct.h` ; `economy.h` l'inclut ; chaque fichier qui emploie un de ces noms l'inclut directement (12 `.c` de `src/`, 17 TU, ceux-ci en `"../src/pico8_fm.h"`). `planet.c:21` retirée. Même coupe : c1, c2 (fiche c1) et le `pico8_fm_hy` de `slipway.c:195` (fiche 11).
- **Essai seul** (`t2_fm`) : 0 erreur ; compteur **16 / 41** ; usage 20 → **17** ; sortantes : `planet` ↔ `probe` (deux listes), `economy` ↔ `economy_score`, `economy` ↔ `tech_gate` (usage) ; aucune entrante ; pied 1071 / 85.
- **Preuve pour 2.17** : refactor pur. Mesuré avec les fiches 11 et 12 (déclarations seules) : les 73 objets de `src/` sans `-g`, sous `-DNDEBUG`, ont `objdump -d` et `nm` identiques à ceux du départ ; sans `-DNDEBUG`, seuls les immédiats `__LINE__` des `assert` peuvent différer (lignes décalées), comme 2.1.

### Fiche 9 — `planet` ↔ `slipway` (2 lignes)

- **planet → slipway** : `planet.c:500-504` (`pico8_jo_ldown`, branche `"ly"`) : `pico8_jc_keys_t`, `pico8_jc_ready`, `pico8_jc_new`.
- **slipway → planet** : `slipway.c:55` (`pico8_jc_end_p`), `:64` (`pico8_jc_end_ra`), `:267-268` (`pico8_jc_ce`), `:358` (`jc_is_endpoint_class`) : `pico8_jo_is_family`, `pico8_jo_t`, `pico8_jo_struct_class`.
- **Sort (2.10)** : `:55`, `:64` partent à 2.14b ; `:267-268` (`oe`) et `:358` restent ; les trois noms de `slipway.h` restent.
- **Sens coupé : planet → slipway. Technique : T1 élargi** (fiche 5) : `pico8_jo_ldown` passe dans `planet_display.c`, qui inclut déjà `slipway.h`. `planet.c:23` retirée. `planet_display` → `slipway` ne fait pas de paire (`slipway` n'emploie rien de `planet_display.h`).
- **Essai seul** : avec la fiche 5 (`t1_jo_ui`), même résultat.
- **Preuve pour 2.17** : avec la fiche 5.

### Fiche 10 — `planet_display` ↔ `planet_family` (2 lignes)

- **planet_display → planet_family** : `planet_display.c:137`, `:154` (`pico8_jo_gh` : `pico8_qh_t`, `pico8_qh_of`), `:404-405`, `:429`, `:436` (`pico8_jo_kj` : `pico8_qh_t`, `pico8_nl_t`, `pico8_nl_of`, `pico8_qh_of`).
- **planet_family → planet_display** : `planet_family.c:244` (`pico8_hj_kj` : `pico8_jo_kj`).
- **Sort (2.10)** : `pico8_qh_of`, `pico8_nl_of` retirés à **2.15c** (bits `QH`, `NL`), `pico8_qh_t`, `pico8_nl_t` à **2.15d** : plus aucun symbole de `planet_family.h` après 2.15d.
- **Sens coupé : planet_display → planet_family. Technique : T5**, à **2.15d**, à condition que 2.15d retire aussi `planet_display.c:18` (`#include "pico8_planet_family.h"`) dans le même commit. La paire sort de 2.17. Ne dépend pas de Q-2.10.
- **Essai** : non simulé ; retirer `planet_display.c:18` aujourd'hui donne des erreurs (usage réel) et `inclusion-mutuelle` 15 / 38 après la simulation de 2.14b.

### Fiche 11 — `probe` ↔ `slipway` (3 lignes)

- **probe → slipway** : `probe.c:53-64` (`pico8_bv_jc_predicate`), `:109` (`g__is_target_class`), `:114-132` (`pico8_probe_et_hang_fo`), `:218-219` (`probe_et_qs`), `:285-300` (`probe_host_raise_p_`) : `pico8_jc_t`, `pico8_jc_struct_class`, `pico8_jc_end_p`, `pico8_jc_too_far_struct_class`, `pico8_jc_too_far_t`.
- **slipway → probe** : `slipway.c:148` (`pico8_jc_hq` : `pico8_probe_et_hang_fo`), `:163` (`pico8_jc_ey` : `pico8_d_`), `:195` (`pico8_fm_hy`), `:334` (`pico8_jc_ev` : `pico8_dl`), `:361` (`jc_probe_of`) et `slipway.h:52-53` (membres `probe`, `probe_own`) : `pico8_probe_t`.
- **Sort (2.10)** : `probe_host_raise_p_` réduit à 2.14d1 (lien `p_`) ; le reste de probe → slipway **reste** (casts vers `jc`, `jc_too_far`, écriture de `fo`) ; `jc_probe_of` part à 2.15a, mais les membres `probe`/`probe_own` de `jc` restent (2.10, §2).
- **Sens coupé : slipway → probe** (le porteur vers le composant). Couper `probe` → `slipway` demanderait de sortir de `probe.c` tout le code propre à chaque porteur (cinq fonctions), sans lien du noyau pour `fo` ni pour l'écriture des pointeurs `ra`/`p` (2.10 : ni l'un ni l'autre n'est un groupe à lien mutable).
- **Technique : T2p (+ T2f pour `pico8_fm_hy`).** `probe.h` mélange trois choses : le composant sonde, l'entité `g_`/`fv`, le créneau `fm`. Le composant part dans un en-tête neuf `pico8_probe_comp.h` (nom provisoire, sans `.c`) : `pico8_probe_t` (étiquette `struct pico8_probe` déjà là), `pico8_bv`, `pico8_bv_dispatch`, `pico8_probe_et_hang_fo`, `pico8_bv_et_t`, `pico8_bv_et_struct_class`, `pico8_dl`, `pico8_dl_show`, `pico8_dl_dispatch`, `pico8_d_`, `pico8_d__dispatch`, avec leurs commentaires ; il n'inclut que `fixed.h`, `gfx.h`, `mem.h`, `struct.h`, `vec.h`. `probe.h` l'inclut (`pico8_g__t` enchâsse un `pico8_probe_t`). `slipway.h:7`, `slipway.c:17`, `wormhole.h:11`, `wormhole.c:13` incluent `pico8_probe_comp.h` au lieu de `pico8_probe.h` (piège 2 : les deux `.h` comptent). Les définitions restent dans `probe.c`. 2.12b/2.13b ajouteront `def` à `pico8_probe_t` dans ce nouvel en-tête (qui inclura alors `game_data.h` ; `game_data` n'emploie rien du composant).
- **Essai seul** (`probe_comp` seul) : 3 erreurs (`slipway.c:195`, `pico8_fm_hy`) : la coupe exige T2f. **Essai `t2_fm` + `probe_comp`** : 0 erreur ; compteur **14 / 35** ; usage **15** ; sortantes : `planet` ↔ `probe`, `probe` ↔ `slipway`, `probe` ↔ `wormhole` (deux listes), c1, c2 (usage) ; aucune entrante ; pied 1065 / 83.
- **Preuve pour 2.17** : refactor pur (voir fiche 8 : objets identiques sous `-DNDEBUG`).

### Fiche 12 — `probe` ↔ `ui` (2 lignes)

- **probe → ui** : `probe.c:72` (`pico8_probe_ui_hw_call` : `pico8_ui_hw`), `:243` (`pico8_dl_show` : `pico8_je_nf`).
- **ui → probe** : `ui.c:332` (`ui_is_held_class`, lu par le seul `assert` de `ui.c:359`) : `pico8_g__struct_class`.
- **Sort** : rien ne part avec 2.14/2.15.
- **Sens coupé : ui → probe. Technique : T4.** Vérification d'équivalence par étiquette (tables de la copie, grep des `.tags =`) : l'étiquette `JC` n'est portée que par `jc` (`slipway.c:392`), `MZ` que par `mz` (`wormhole.c:233`), mais **`G_` est portée par `g_` (`probe.c:432`) et `fv` (`help.c:134`)**, avec les mêmes étiquettes ; le masque de lignée de 2.15b (bit `G_`) ne les distingue pas non plus. `tags & G_` affaiblirait l'`assert` (il accepterait `fv`, qu'aucun `ldown` ne rend aujourd'hui). Autre moyen retenu, exact : **l'identité de classe déclarée au noyau** — `const pico8_struct_class_t *pico8_g__struct_class(void);` et `pico8_jc_struct_class` (fiche 16) déclarés dans `struct.h` (en fin de fichier), retirés de `probe.h:64` et `slipway.h:80`, définis où ils sont. `struct.h` n'inclut rien de neuf (`noyau-inclut` intact). `ui.c:13` retirée.
- **Essai seul** (`t4_ui`, avec la fiche 16) : 0 erreur ; **15 / 39** ; usage 18 ; sortantes `probe` ↔ `ui`, `slipway` ↔ `ui` ; aucune entrante ; pied 1069 / 85.
- **Preuve pour 2.17** : refactor pur (fiche 8).

### Fiche 13 — `probe` ↔ `wormhole` (3 lignes)

- **probe → wormhole** : `probe.c:34` (`bv_et_is_holder_class`), `:85-100` (`probe_host_set_ra_p`), `:122` (`pico8_probe_et_hang_fo`), `:215-216` (`probe_et_qs`), `:286-307` (`probe_host_raise_p_`) : `pico8_mz_t`, `pico8_mz_struct_class`, `pico8_mz_jj_t`, `pico8_mz_jj_struct_class`.
- **wormhole → probe** : `wormhole.c:58` (`pico8_mz_hq` : `pico8_bv`), `:121` (`pico8_mz_ey` : `pico8_d_`), `:199` (`mz_probe_of`), `:224` (table : `pico8_dl_dispatch`), `wormhole.h:42-43` (membres) : `pico8_probe_t`.
- **Sort** : comme la fiche 11 (2.14d1 réduit `raise_p_`, le reste demeure).
- **Sens coupé : wormhole → probe. Technique : T2p** (fiche 11). `wormhole` n'emploie pas `pico8_fm_hy` : la coupe tient sans T2f.
- **Essai** : avec la fiche 11. `probe_comp` seul fait déjà sortir `probe` ↔ `wormhole` des deux listes (compteur 15 / 37 ; les 3 erreurs sont celles de `slipway`). Il fait aussi sortir `probe` ↔ `slipway` du compteur, mais pas du graphe d'usage (`pico8_fm_hy` reste) : exemple du piège du détour, que seul le graphe d'usage attrape.
- **Preuve pour 2.17** : refactor pur.

### Fiche 14 — `score_panel` ↔ `score_panel_display` (3 lignes)

- **score_panel → score_panel_display** : `score_panel.c:267` (`pico8_pj_hk_dispatch` : `pico8_pj_hk`), `:272` (`pico8_pj_ph_dispatch` : `pico8_pj_ph`), `:346` (`pico8_score_panel_init` : `pico8_score_panel_display_init`), `:350` (`pico8_score_panel_shutdown` : `_shutdown`).
- **score_panel_display → score_panel** : 8 symboles (`pico8_pj_t`, `pico8_pj_ea`, `pico8_pj_ea_t`, `pico8_pj_gs`, `pico8_ju`, `pico8_bz_row_t`, `pico8_pj_bz`, `pico8_init_field_value`).
- **Sens coupé : score_panel → score_panel_display. Technique : T1 + T2** : les deux dispatchers (`static`) passent dans `score_panel_display.c`, déclarés dans `score_panel.h` ; les prototypes de `pico8_score_panel_display_init`/`_shutdown` passent de `score_panel_display.h:18-19` à `score_panel.h` (l'appel d'init reste dans `pico8_score_panel_init` : l'ordre d'init ne change pas). `score_panel.c:21` retirée.
- **Essai seul** (`t1_pj`) : 0 erreur ; **16 / 40** ; usage 19 ; sortante la paire ; aucune entrante ; pied 1070 / 83.
- **Preuve pour 2.17** : corps identiques à l'adresse près ; `nm` : `t` → `T` pour les deux dispatchers.

### Fiche 15 — `slipway` ↔ `slipway_display` (3 lignes)

- **slipway → slipway_display** : `slipway.c:354` (`pico8_jc_kj_dispatch` : `pico8_jc_kj`).
- **slipway_display → slipway** : `slipway_display.c:17-86`, `.h:25-35` : `pico8_jc_t`, `pico8_jc_hg_has`, `pico8_jc_end_ra`.
- **Sens coupé : slipway → slipway_display. Technique : T1** : `pico8_jc_kj_dispatch` passe dans `slipway_display.c` (qui inclut `dispatch_context.h`), déclaré dans `slipway.h`. `slipway.c:18` retirée.
- **Essai seul** (`t1_jc`) : 0 erreur ; **16 / 40** ; usage 19 ; sortante la paire ; aucune entrante ; pied 1070 / 83.
- **Preuve pour 2.17** : corps identique à l'adresse près ; `nm` : `t` → `T`.

### Fiche 16 — `slipway` ↔ `ui` (2 lignes)

- **slipway → ui** : `slipway.c:116-123` (`pico8_jc_hq` : `pico8_ok`, `pico8_ui_t`, lecture de `ok->o_`).
- **ui → slipway** : `ui.c:332` (`ui_is_held_class`) : `pico8_jc_struct_class`.
- **Sens coupé : ui → slipway. Technique : T4** (fiche 12 : identité de classe au noyau). Pour `jc` seul, l'étiquette `JC` serait équivalente ; la même technique est prise pour les deux, un seul `assert` les lit. `ui.c:14` retirée.
- **Essai** : avec la fiche 12.

### Fiche 17 — `wormhole` ↔ `wormhole_display` (3 lignes)

- **wormhole → wormhole_display** : `wormhole.c:192` (`pico8_mz_kj_dispatch` : `pico8_mz_kj`).
- **wormhole_display → wormhole** : `wormhole_display.c:11`, `:20`, `:56`, `.h:13` : `pico8_mz_t`, `pico8_mz_ej`.
- **Sens coupé : wormhole → wormhole_display. Technique : T1** (essayé par le PRD) : `pico8_mz_kj_dispatch` dans `wormhole_display.c`, déclaré dans `wormhole.h`. `wormhole.c:15` retirée.
- **Essai seul** (`t1_mz`) : 0 erreur ; **16 / 40** ; usage 19 ; sortante la paire ; aucune entrante ; pied 1070 / 84.
- **Preuve pour 2.17** : corps identique à l'adresse près, `nm` `t` → `T` (mesuré par le PRD).

### Fiche c1 — `economy` ↔ `economy_score` (cachée)

- **economy → economy_score** : `economy.c:220`, `:233`, `:236` : `pico8_da`, `pico8_db`, `pico8_cz`, `pico8_bf` (la fiche 1 y ajoute `pico8_kr`).
- **economy_score → economy** : `pico8_hy_t` seul (`:189`, `:232`), les champs du `hy` lus par `pico8_fm_hy()`.
- **Sens coupé : economy_score → economy. Technique : T2f** (fiche 8) : `pico8_hy_t` dans `pico8_fm.h`. Coupée avec la fiche 8 ; le compteur ne la voit ni avant ni après (usage 20 → 17 avec `t2_fm`).

### Fiche c2 — `economy` ↔ `tech_gate` (cachée)

- **economy → tech_gate** : `pico8_ov` (`economy.c:96`, `:232`), `pico8_gi_nb` (`:280`).
- **tech_gate → economy** : `pico8_hy_t` (`tech_gate.c:16`, `:38`, `:62`).
- **Sens coupé : tech_gate → economy. Technique : T2f**, avec la fiche 8.

### Fiche c3 — `entity_geom` ↔ `scroll` (cachée)

- **entity_geom → scroll** (par `entity.h:5` → `scroll.h`) : `entity_geom.c:49` (`pico8_gd` : `pico8_qb_t`, lecture de `pico8_mo_qb()->ra`), `:77`, `:79` (`pico8_ma` : `pico8_qb_struct_class`, `pico8_qb_t`).
- **scroll → entity_geom** : `scroll.c:131` (`pico8_qb_ph` : `pico8_ma`).
- **Sort (2.10)** : `:77`, `:79` partent à **2.14c** (lien `ma`) ; `:49` reste.
- **Sens coupé : entity_geom → scroll. Technique : T7 + T5.** `pico8_gd` lit la caméra par `pico8_entity_geom_ra(pico8_mo())` au lieu de `pico8_mo_qb()->ra` : même valeur dans les trois cas (`mo` NULL → zéro ; le provisoire de `entity.c:44`, classe `{0}` sans `get_ra` → zéro ; un `qb` → `qb_get_ra`, `scroll.c:30`, soit `self->ra`, zéro si NULL). L'`assert` de classe de `pico8_mo_qb` (`entity.c:255`) n'est plus traversé par ce chemin. Après 2.14b, `pico8_entity_geom_ra` lit le lien `ra` du noyau ; après 2.14c, `entity_geom` n'emploie plus aucun nom de `scroll.h`. **Dépend de Q-2.10.**
- **Essai seul** (`gd_mo`) : 0 erreur ; compteur inchangé, usage 20 ; aucune entrante. Sites restants : `entity_geom.c:64`, `:66` (`pico8_ma`), que 2.14c retire.
- **Preuve pour 2.17** : chemin touché (`gcov` de `pico8_gd` ; mutations : zéro rendu pour un `qb`, `p` lu au lieu de `ra`).

### Contrôles de complétude (2.11a)

1. **Lignes** : les 43 lignes de `--check inclusion-mutuelle` sont dans la table du §1 et chacune dans la fiche de sa paire ; `comm -3` entre la liste du compteur et la liste des lignes citées au §1 : vide (commande et sortie en annexe B).
2. **Paires** : 17 fiches pour les 17 paires de `--detail`, trois fiches (c1 à c3) pour les trois paires d'usage hors compteur ; 20 = 20.
3. **Symboles** : chaque arête coupée a son sort. Arêtes coupées sans attendre 2.14/2.15 : le graphe d'usage relancé sur l'essai ne les contient plus (aucun symbole restant). Arêtes qui attendent 2.14/2.15 (fiches 3, 4, 6, 10, c3) : les symboles restants après les coupes préparatoires sont listés avec leur site et leur sous-étape de retrait (2.14b, 2.14c, 2.15c, 2.15d).
4. **Tombées d'elles-mêmes** : fiche 4 → 2.14b ; fiche 10 → 2.15d. Les sous-étapes sont celles qui retirent le dernier symbole selon 2.10, §4.
5. **Essais seuls** : chaque coupe de 2.17 a le sien (tableau « Runs ») ; aucune paire entrante, aucune catégorie autre qu'`inclusion-mutuelle` ne bouge.

## 3. Essai cumulé (2.11b)

Toutes les coupes que 2.17 fait sans attendre 2.14/2.15 (fiches 1, 2, 5, 7, 8, 9, 11 à 17, c1, c2), plus les parties préparatoires de 2.17e qui n'en dépendent pas (`pico8_na` vers `planet`, `pico8_jo_kv` vers `planet_family`, `pico8_gd` sur `pico8_mo()`), appliquées ensemble sur la copie : `bash trial.sh t3_dj t1_mz t1_jc t1_pj t1_jo_ui t1_jo_trade t2_fm kr_score probe_comp t4_ui na_planet kv_family gd_mo`.

| Critère | Attendu | Mesuré |
|---|---|---|
| compilation `-fsyntax-only`, `src/` et `tests/` (gcc 13.3) | 0 erreur | 0 |
| même compilation sous `clang` (piège 4) | 0 erreur | 0 |
| compteur | 17 − 13 = 4 paires | **4 paires / 10 lignes** (`entity_geom` ↔ `planet`, `particle` ↔ `probe`, `planet` ↔ `planet_family`, `planet_display` ↔ `planet_family`) |
| graphe d'usage | 20 − 13 − 2 (c1, c2) = 5 | **5** (les quatre ci-dessus et c3) |
| paires entrantes | aucune | aucune, au compteur comme à l'usage |
| autres catégories | inchangées | inchangées (`parente` 10, `lien-parent` 111, `accesseurs-q6` 237, `défauts` 669, `void-ptr` 8 / 4) ; pied 1073 / 85 → 1040 / 74 |
| `make -j4 test` | vert | vert (8 s), après correction du chemin d'include des TU (Runs 21) |
| `make e2e` | 26 conformes | `e2e: 26 e2e`, 26 `conforme` (1 s) |
| objets des coupes « déclaration seule » (`t2_fm`, `probe_comp`, `t4_ui`) | identiques sous `-DNDEBUG` | 73 / 73 objets de `src/` sans `-g` : `objdump -d` et `nm` identiques |

Le diff cumulé (`src/`, 33 fichiers, 2 neufs) est en annexe D ; les 17 TU ne gagnent qu'une ligne chacun, `#include "../src/pico8_fm.h"`.

**Cycles plus longs** (hors périmètre, signalés) : le graphe d'usage orienté a au départ deux composantes fortement connexes, de 26 et 2 modules ; après l'essai cumulé, une de 25 (`scc.py`). Démêler les paires ne rend pas le graphe acyclique : la Sortie de la phase (`inclusion-mutuelle` à 0) n'en dit rien.

### Comptes d'arrivée, sous-étape par sous-étape

Mesurés sur la copie en appliquant les coupes dans l'ordre de §4. Les retraits d'usage de 2.14b, 2.14c, 2.15c et 2.15d ne sont pas simulés : leurs lignes `#include` sont retirées pour le seul compte (`sim_214b`, `sim_215d`, `sim_e`, qui ne compilent pas avant ces étapes). Pied à 2.17 : toutes les autres catégories sont à 0 après 2.16, il ne compte plus que les lignes d'`inclusion-mutuelle`.

| Après | Compteur (paires / lignes / fichiers) | Usage |
|---|---|---|
| départ | 17 / 43 / 25 | 20 |
| 2.14b (`particle.c:13` retirée) | 16 / 40 / 23 | 19 |
| 2.15d (`planet_display.c:18` retirée) | 15 / 38 / 23 | 18 |
| 2.17a | 12 / 29 / 16 | 15 |
| 2.17b | 9 / 21 / 12 | 12 |
| 2.17c | 7 / 17 / 10 | 10 |
| 2.17d | 2 / 5 / 4 | 3 |
| 2.17e | **0 / 0 / 0** | **0** |

## 4. Liste ordonnée de 2.17 (2.11b)

**Ordre : tout 2.17 reste après 2.16.** Les coupes de 2.17a à 2.17d ne dépendent d'aucun symbole que 2.14/2.15 retirent, mais `pico8_hy_t` (2.12a, 2.13a1 : archétype et `def` de `hy`) et `pico8_probe_t` (2.12b, 2.13b : `def` du composant) changent avant, et les coupes déplacent des corps de `planet.c`, `slipway.c`, `probe.c`, dont les lignes 2.12 à 2.16 citent les numéros. La garde ne mord qu'à 0 : rien ne se gagne à couper plus tôt. 2.17e attend 2.14c et 2.15d. 2.14b et 2.15d font chacune tomber une paire (incises sur ces lignes du plan).

Preuves communes : celles du plan (`make test`, 26 e2e conformes, aucun `*.sha256` ni constante d'empreinte changés). Chaque sous-étape relance, au départ puis au commit, `usegraph.py` (annexe A) : les paires d'usage attendues sortent, aucune n'entre. Une ligne `#include` retirée sans que l'usage ait disparu est un `FAILED:` (piège du détour).

- **2.17a — T1, trois modules d'affichage** (fiches 17, 15, 14). `pico8_mz_kj_dispatch` vers `wormhole_display.c`, `pico8_jc_kj_dispatch` vers `slipway_display.c`, `pico8_pj_hk_dispatch`/`pico8_pj_ph_dispatch` vers `score_panel_display.c` (déclarés dans `wormhole.h`, `slipway.h`, `score_panel.h`) ; prototypes de `pico8_score_panel_display_init`/`_shutdown` dans `score_panel.h`. Retire `wormhole.c:15`, `slipway.c:18`, `score_panel.c:21`. Compte : `inclusion-mutuelle` 15 / 38 → **12 / 29** ; usage 18 → 15. Preuves : corps déplacés identiques à l'adresse près (`objdump -d` du symbole, seules les cibles relatives diffèrent), `nm` ne diffère que par la liaison des quatre dispatchers (`t` → `T`) ; autres objets identiques sous `-DNDEBUG`. `class-coverage-net` : non (aucune table ni constructeur ne bouge, les tables pointent les mêmes noms). `sanitize-net` : non. Budget ≈ 50 min.
- **2.17b — T1, planète** (fiches 5, 9, 7). `pico8_jo_hb_dispatch`, `pico8_jo_gv_dispatch`, `pico8_jo_kj_dispatch`, `pico8_jo_ldown_dispatch` et `pico8_jo_ldown` vers `planet_display.c` ; `pico8_jo_kf_dispatch`, `pico8_jo_cn_dispatch` vers `planet_trade.c` (tous déjà déclarés dans `planet.h`). Retire `planet.c:18`, `:20`, `:23`. Compte : → **9 / 21** ; usage → 12. Preuves : corps identiques à l'adresse près ; `nm` : sept symboles changent de module, liaison `T` inchangée. `class-coverage-net` : non. `sanitize-net` : non. Budget ≈ 50 min.
- **2.17c — T3 et T6, économie** (fiches 2, 1). `pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p)`, appelant `slipway.c:139` ; le pas de boucle de `hy:lj` (`economy.c:219-226`) devient `pico8_kr` (nom provisoire) dans `economy_score`. Retire `economy_formulas.c:10`, `economy.c:15`. Compte : → **7 / 17** ; usage → 10. Preuves : chemin touché, `gcov` de `pico8_dj`, `pico8_jc_hq`, `pico8_hy_lj`, `pico8_kr` au parent puis après ; mutations : `pico8_dj` qui reçoit deux fois l'extrémité `qu` (échanger les deux arguments ne change rien, la distance est au carré : mutation inutile, déclarée), `has_kr` laissé à 0, `flr` retiré de `kr`. `class-coverage-net` : non. `sanitize-net` : non. Budget ≈ 50 min.
- **2.17d — T2 et T4, déclarations** (fiches 8, 11, 13, 12, 16, c1, c2). En-tête neuf `pico8_fm.h` (`pico8_hy_t` avec l'étiquette `struct pico8_hy`, créneau `fm`) inclus par `economy.h` et par chaque fichier qui en emploie un nom (12 `.c`, 17 TU) ; en-tête neuf `pico8_probe_comp.h` (le composant sonde, onze déclarations) inclus par `probe.h`, `slipway.h`, `slipway.c`, `wormhole.h`, `wormhole.c` à la place de `probe.h` ; `pico8_jc_struct_class` et `pico8_g__struct_class` déclarés dans `struct.h`. Retire `planet.c:21`, `ui.c:13`, `:14` ; remplace `slipway.c:17`, `slipway.h:7`, `wormhole.c:13`, `wormhole.h:11`. Les noms des deux en-têtes sont provisoires (phase 3). Compte : → **2 / 5** ; usage → 3 (c1, c2 sortent). Preuves : refactor pur, objets de `src/` sans `-g` identiques sous `-DNDEBUG` (essai : 73 / 73), sans `-DNDEBUG` seuls les immédiats `__LINE__` des `assert` peuvent différer. `class-coverage-net` : non. `sanitize-net` : non. Budget ≈ 50 min.
- **2.17e — T6, T7 et fin, après 2.14c et 2.15d** (fiches 3, 6, c3). Départ : `sites.py` rejoué ; les arêtes `entity_geom` → `planet`, `entity_geom` → `scroll`, `planet` → `planet_family` ne gardent que `pico8_na`, `pico8_gd`, le registre `kv` (sinon arrêt : 2.14c ou 2.15d n'a pas retiré ce que 2.10 prévoit). `pico8_na` vers `planet.c`/`planet.h` ; `pico8_gd` lit `pico8_entity_geom_ra(pico8_mo())` ; `g_kv`/`jo_kv` vers `planet_family.c` en `pico8_jo_kv`, déclaré dans `planet.h`. Retire `entity_geom.c:9`, `planet.c:19`. Compte : → **0 paire, 0 ligne**, usage **0** ; `inclusion-mutuelle` entre dans `COMPOSITION_RESIDUE_GUARD`, prouvée mordante (`#include "pico8_planet.h"` remis dans une copie d'`entity_geom.c` → `make test` rouge). Preuves : `pico8_na`, `pico8_jo_kv` identiques à l'adresse près ; `pico8_gd` chemin touché (`gcov` ; mutations : zéro rendu pour un `qb`, `p` lu au lieu de `ra`). `class-coverage-net` : non. `sanitize-net` : non. Budget ≈ 50 min.
- **2.17f — garde de l'usage (G4).** La garde d'`inclusion-mutuelle` ne voit pas un usage revenu par un détour (14 lignes de `.c` sur 34 l'étaient au départ ; la phase 3 renomme tout). Catégorie neuve `usage-mutuel` dans `composition_residue.sh` : portage `awk` de `usegraph.py` (noms déclarés au niveau supérieur des `.h` de `src/` par motif, un nom déclaré par deux en-têtes écarté, A → B si un fichier de A emploie un nom de B, une ligne par paire). Fixture : deux modules dont un sens passe par un détour (rouge à `usage-mutuel`, vert à `inclusion-mutuelle`) ; deux mutations du script (nom partagé compté, commentaires non retirés). Entre dans `CATEGORIES` et `SORTIE` à 0, et dans la garde. Compte : `usage-mutuel` **0**, gardé ; pied inchangé. Preuves : fixture rouge puis verte, deux mutations, garde mordante (sur une copie, `planet.c` appelle `pico8_g__ready()` de `probe.h`, reçu par le détour `planet_family.h` → `probe.h`, sans include direct : `usage-mutuel` rouge, `inclusion-mutuelle` vert). Budget ≈ 50 min.

Si une sous-étape dépasse 1,5 fois son budget, repli : 2.17d en deux (d1 : `pico8_fm.h` ; d2 : `pico8_probe_comp.h` et identités au noyau, qui exige d1 pour `slipway`).

## 5. Questions à l'humain

Aucune. Les choix de sens et de technique sont tranchés par les critères du PRD (coupe d'usage, aucune paire neuve, équivalence d'un test de classe) ; les deux en-têtes neufs sont imposés par le critère « aucune paire neuve » (fiche 8 : aucun en-tête existant ne convient pour le créneau `fm`).

La réponse attendue à la question Q-2.10 de l'étude 2.10 (« oui ») est supposée : trois fiches en dépendent (4, 3, c3 ; voir §0). Si la réponse est « non », 2.17 garde la paire `particle` ↔ `probe` et les deux paires de `entity_geom`, et leurs fiches sont à refaire avant 2.14.

## Annexe A — scripts (texte exact essayé)

Tous lancés depuis le dossier des essais, la copie de `9ca66f4` dans `tree/`. `cross.py` et `usegraph.py` sont ceux de l'annexe A du PRD (la copie du scratchpad n'en diffère que par deux commentaires et une variable inutilisée ; sorties identiques aux tables du PRD). `usegraph_e.py` est `usegraph.py` qui imprime aussi chaque arête orientée (`E<TAB>A<TAB>B<TAB>symboles`, sortie d'erreur), obtenu par `sed` ; `scc.py` en tire les composantes fortement connexes.

Une coupe est un script `cuts/<nom>.py <copie>` ; `bash trial.sh <nom>...` remet la copie à zéro, applique les coupes dans l'ordre, compile, relance compteur et graphe d'usage et compare au départ (`base`).

`syn.sh` :

```sh
#!/bin/sh
# compile tout l'arbre en -fsyntax-only ; rend les lignes error:
cd "$1"
ls src/*.c tests/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend|class_coverage' \
  | xargs -P4 -n8 cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only 2>&1
for f in src/main_class_coverage.c tests/test_class_coverage.c; do
  cc -std=c99 -Wall -Wextra -Werror -Isrc -DPICO8_CLASS_COVERAGE -fsyntax-only "$f" 2>&1
done
```

`trial.sh` :

```sh
#!/bin/bash
# trial.sh NAME... : copie remise a zero, applique cuts/NAME.py (dans l'ordre), compile, compteur, usegraph, compare a base
E=$(cd "$(dirname "$0")" && pwd); cd "$E"; T=$E/tree; N=$(echo "$@" | tr ' ' '+')
(cd $T && git checkout -q -- . && git clean -qfd src tests)
t0=$(date +%s.%N)
for c in "$@"; do python3 $E/cuts/$c.py $T || { echo "PATCH FAILED $c"; exit 1; }; done
bash $E/syn.sh $T > logs/$N.syn 2>&1; nerr=$(grep -c 'error' logs/$N.syn)
(cd $T && sh tools/composition_residue.sh --detail) > logs/$N.detail 2>&1
(cd $T && sh tools/composition_residue.sh) > logs/$N.foot 2>&1
(cd $T && sh tools/composition_residue.sh --check inclusion-mutuelle) > logs/$N.lines 2>&1
python3 usegraph.py $T > logs/$N.use 2> logs/$N.useerr
(cd $T && git diff; git status --short) > logs/$N.diff
t1=$(date +%s.%N)
python3 $E/cmp.py base "$N" "$nerr" "$(echo "$t1 - $t0" | bc)"
```

`cmp.py` :

```python
import sys
b,n,nerr,dt=sys.argv[1:5]; L='logs/'
def cp(x): return {tuple(l.split('\t')[1:3]) for l in open(L+x+'.detail') if l.startswith('inclusion-mutuelle\t')}
def up(x): return {tuple(l.split('\t')[:2]) for l in open(L+x+'.use')}
def ft(x): return {l.split('\t')[0]:l.strip().split('\t')[1:] for l in open(L+x+'.foot') if '\t' in l}
def foot(x): return [l.strip() for l in open(L+x+'.foot')][-1]
nl=lambda x: sum(1 for l in open(L+x+'.lines') if l.startswith('inclusion-mutuelle:'))
s=lambda p: ' '.join(f"{a[6:]}<->{c[6:]}" for a,c in sorted((x.strip(),y.strip()) for x,y in p)) or '-'
fb,fn=ft(b),ft(n)
up_=[k+' '+'/'.join(fb[k])+'->'+'/'.join(fn.get(k,['?'])) for k in fb if fb[k]!=fn.get(k)]
print(f"== {n}: erreurs {nerr} | compteur {len(cp(n))} paires/{nl(n)} lignes | usage {len(up(n))} | {foot(n)} | {dt[:4]} s")
print(f"   compteur sortantes: {s(cp(b)-cp(n))} ; entrantes: {s(cp(n)-cp(b))}")
print(f"   usage    sortantes: {s(up(b)-up(n))} ; entrantes: {s(up(n)-up(b))}")
print(f"   categories changees: {'; '.join(up_) or 'aucune'}")
```

`sites.py` :

```python
#!/usr/bin/env python3
# Pour chaque paire d'usage et chaque sens : sites (fichier:ligne, fonction) des symboles.
import re,sys,os,glob
os.chdir(sys.argv[1])
def strip(s):
    s=re.sub(r'/\*.*?\*/',lambda m:'\n'*m.group(0).count('\n'),s,flags=re.S); s=re.sub(r'//[^\n]*','',s)
    return re.sub(r'"(\\.|[^"\\])*"','""',s)
for line in open(sys.argv[2]):
    a,b,ab,ba=line.rstrip('\n').split('\t')
    for (x,y,syms) in ((a,b,ab),(b,a,ba)):
        names=syms.split(':',1)[1].split()
        print(f"## {x} -> {y}")
        for f in sorted(glob.glob(f'src/{x}.[ch]')):
            L=strip(open(f).read()).split('\n'); fn='-'
            for i,l in enumerate(L,1):
                m=re.match(r'^[A-Za-z_][\w \*]*?\b(\w+)\s*\(',l)
                if m and not l.rstrip().endswith(';'): fn=m.group(1)
                hit=[n for n in names if re.search(r'\b'+n+r'\b',l)]
                if hit: print(f"  {os.path.basename(f)}:{i}\t{fn}\t{' '.join(hit)}")
```

`objs.sh` :

```sh
#!/bin/bash
# objs.sh OUT : objets sans -g, -DNDEBUG, de src/ (hors raylib), desassembles
cd tree; mkdir -p ../$1
for f in $(ls src/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend|class_coverage'); do
  b=$(basename $f .c); cc -std=c99 -O0 -DNDEBUG -c -o ../$1/$b.o $f && objdump -d ../$1/$b.o | tail -n +3 > ../$1/$b.dis && nm ../$1/$b.o > ../$1/$b.nm
done
```

`scc.py` :

```python
import sys,collections
def g(f):
    G=collections.defaultdict(set)
    for l in open(f):
        _,a,b,_=l.rstrip('\n').split('\t'); G[a].add(b)
    return G
def scc(G):
    idx={};low={};st=[];on=set();out=[];c=[0]
    sys.setrecursionlimit(10000)
    def v(x):
        idx[x]=low[x]=c[0];c[0]+=1;st.append(x);on.add(x)
        for y in G.get(x,()):
            if y not in idx: v(y);low[x]=min(low[x],low[y])
            elif y in on: low[x]=min(low[x],idx[y])
        if low[x]==idx[x]:
            comp=[]
            while True:
                y=st.pop();on.discard(y);comp.append(y)
                if y==x:break
            if len(comp)>1: out.append(sorted(comp))
    for x in list(G):
        if x not in idx: v(x)
    return out
for f in sys.argv[1:]:
    S=scc(g(f)); print(f, 'composantes >1 :', [len(s) for s in S], 'modules en cycle :', sum(len(s) for s in S))
```

`cuts/_h.py` :

```python
import os, sys, re
sys.path.insert(0, os.path.dirname(__file__))
T = sys.argv[1]
def p(f): return os.path.join(T, f if '/' in f else 'src/' + f)
def rd(f): return open(p(f)).read()
def wr(f, s): open(p(f), 'w').write(s)
def sub(f, old, new, n=1):
    s = rd(f); assert s.count(old) >= 1, (f, old[:60]); wr(f, s.replace(old, new, n))
def cut_fn(f, start):
    """retire la definition qui commence par `start` (jusqu'a la premiere ligne '}'), la rend"""
    s = rd(f); i = s.index(start); j = s.index('\n}\n', i) + 3
    if s[j:j+1] == '\n': j += 1
    wr(f, s[:i] + s[j:]); return s[i:j]
def add_before_endif(h, text):
    s = rd(h); i = s.rindex('#endif'); wr(h, s[:i] + text + '\n' + s[i:])
def append(f, text): wr(f, rd(f).rstrip('\n') + '\n\n' + text.rstrip('\n') + '\n')
def del_include(f, h): sub(f, '#include "%s"\n' % h, '')
def add_include(f, h, after):
    sub(f, '#include "%s"\n' % after, '#include "%s"\n#include "%s"\n' % (after, h))
def cut_decl(h, decl):
    """retire une declaration (et le commentaire qui la precede immediatement), rend le tout"""
    s = rd(h); i = s.index(decl); j = i + len(decl)
    k = s.rfind('\n\n', 0, i); blk_start = k + 2 if k >= 0 else i
    pre = s[blk_start:i]
    if not pre.lstrip().startswith('/*'): blk_start = i; pre = ''
    if s[j:j+1] == '\n': j += 1
    wr(h, s[:blk_start] + s[j:]); return pre + decl + '\n'
```

`cuts/t1_mz.py` :

```python
from _h import *
fn = cut_fn('pico8_wormhole.c', 'static void pico8_mz_kj_dispatch(')
fn = fn.replace('static void', 'void', 1)
append('pico8_wormhole_display.c', fn)
add_include('pico8_wormhole_display.c', 'pico8_dispatch_context.h', 'pico8_entity.h')
add_before_endif('pico8_wormhole.h', 'void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra);\n')
del_include('pico8_wormhole.c', 'pico8_wormhole_display.h')
```

`cuts/t1_jc.py` :

```python
from _h import *
fn = cut_fn('pico8_slipway.c', 'static void pico8_jc_kj_dispatch(')
fn = fn.replace('static void', 'void', 1)
append('pico8_slipway_display.c', fn)
add_include('pico8_slipway_display.c', 'pico8_dispatch_context.h', 'pico8_entity.h')
add_before_endif('pico8_slipway.h', 'void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra);\n')
del_include('pico8_slipway.c', 'pico8_slipway_display.h')
```

`cuts/t1_pj.py` :

```python
from _h import *
a = cut_fn('pico8_score_panel.c', 'static void pico8_pj_hk_dispatch(').replace('static void', 'void', 1)
b = cut_fn('pico8_score_panel.c', 'static void pico8_pj_ph_dispatch(').replace('static void', 'void', 1)
append('pico8_score_panel_display.c', a + '\n' + b)
d = cut_decl('pico8_score_panel_display.h', 'void pico8_score_panel_display_init(void);\nvoid pico8_score_panel_display_shutdown(void);')
add_before_endif('pico8_score_panel.h', 'void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr);\nvoid pico8_pj_ph_dispatch(void *self, pico8_fixed_t t);\n\n' + d)
del_include('pico8_score_panel.c', 'pico8_score_panel_display.h')
```

`cuts/t1_jo_ui.py` :

```python
from _h import *
fs = [cut_fn('pico8_planet.c', s) for s in (
    'void pico8_jo_hb_dispatch(', 'pico8_down_t pico8_jo_gv_dispatch(', 'void pico8_jo_kj_dispatch(',
    'pico8_down_t pico8_jo_ldown_dispatch(', 'pico8_down_t pico8_jo_ldown(')]
append('pico8_planet_display.c', '\n'.join(fs))
del_include('pico8_planet.c', 'pico8_planet_display.h')
del_include('pico8_planet.c', 'pico8_slipway.h')
```

`cuts/t1_jo_trade.py` :

```python
from _h import *
fs = [cut_fn('pico8_planet.c', s) for s in ('void pico8_jo_kf_dispatch(', 'void pico8_jo_cn_dispatch(')]
append('pico8_planet_trade.c', '\n'.join(fs))
del_include('pico8_planet.c', 'pico8_planet_trade.h')
```

`cuts/t3_dj.py` :

```python
from _h import *
sub('pico8_economy_formulas.h', 'pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to);',
    'pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p);')
sub('pico8_economy_formulas.h', '#include "pico8_map_tables.h"\n', '#include "pico8_map_tables.h"\n#include "pico8_vec.h"\n')
sub('pico8_economy_formulas.c', 'pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to) {\n    pico8_vec_t diff = pico8_vec_sub(pico8_jc_end_p(to), pico8_jc_end_p(pb));',
    'pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p) {\n    pico8_vec_t diff = pico8_vec_sub(to_p, pb_p);')
del_include('pico8_economy_formulas.c', 'pico8_slipway.h')
sub('pico8_slipway.c', 'pico8_dj(self->qu, self->re)', 'pico8_dj(pico8_jc_end_p(self->qu), pico8_jc_end_p(self->re))')
```

`cuts/kr_score.py` :

```python
from _h import *
sub('pico8_economy.c', '''                pico8_struct_header_t *p = e;
                pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
                pico8_jo_t *jo;

                assert(pico8_jo_is_family(p) && "hy:lj: cg.jo holds planets only");
                jo = (pico8_jo_t *)p;
                jo->kr = kr;
                jo->has_kr = 1;
                dd = pico8_fixed_add(dd, kr);''', '''                pico8_fixed_t kr = pico8_kr(e);

                dd = pico8_fixed_add(dd, kr);''')
append('pico8_economy_score.c', '''pico8_fixed_t pico8_kr(pico8_struct_header_t *p) {
    pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
    pico8_jo_t *jo;

    assert(pico8_jo_is_family(p) && "hy:lj: cg.jo holds planets only");
    jo = (pico8_jo_t *)p;
    jo->kr = kr;
    jo->has_kr = 1;
    return kr;
}''')
add_before_endif('pico8_economy_score.h', '/* `kr` of planet `p`, written into it and returned. */\npico8_fixed_t pico8_kr(pico8_struct_header_t *p);\n')
del_include('pico8_economy.c', 'pico8_planet.h')
```

`cuts/t2_fm.py` :

```python
from _h import *
import subprocess
def cut_span(f, start, end):
    s = rd(f); i = s.index(start); j = s.index(end, i) + len(end)
    while s[j:j+1] == '\n': j += 1
    wr(f, s[:i] + s[j:]); return s[i:j].rstrip('\n') + '\n'
hy = cut_span('pico8_economy.h', '/* The economy, one per game.', '} pico8_hy_t;')
hy = hy.replace('typedef struct {', 'typedef struct pico8_hy {', 1)
fm = cut_span('pico8_probe.h', '/* The provisional `fm`:', 'const pico8_fm_placeholder_t *pico8_fm_placeholder(void);')
wr('pico8_fm.h', '#ifndef PICO8_FM_H\n#define PICO8_FM_H\n\n#include "pico8_economy_formulas.h"\n#include "pico8_fixed.h"\n'
   '#include "pico8_ptrs.h"\n#include "pico8_struct.h"\n\n' + hy + '\n' + fm + '\n#endif\n')
sub('pico8_economy.h', '#include "pico8_economy_formulas.h"\n', '#include "pico8_economy_formulas.h"\n#include "pico8_fm.h"\n')
del_include('pico8_planet.c', 'pico8_probe.h')
# chaque .c/.h qui emploie un nom du creneau fm l'inclut directement
names = re.compile(r'\b(pico8_hy_t|pico8_fm|pico8_fm_hy|pico8_fm_placeholder|pico8_fm_placeholder_t|pico8_fm_placeholder_struct_class|pico8_probe_set_fm)\b')
import glob
for f in sorted(glob.glob(p('*.c')) + glob.glob(p('*.h')) + glob.glob(os.path.join(T, 'tests/*.c'))):
    b = os.path.basename(f)
    if b in ('pico8_fm.h', 'pico8_economy.h'): continue
    s = open(f).read()
    body = re.sub(r'/\*.*?\*/', '', s, flags=re.S)
    if names.search(body) and '#include "pico8_fm.h"' not in s:
        pre = '../src/' if '/tests/' in f else ''
        inc = '#include "%spico8_fm.h"\n' % pre
        m = list(re.finditer(r'#include "%spico8_[^"]*"\n' % re.escape(pre), s))
        if not m: print('SANS INCLUDE', f); continue
        own = m[0].end() if b.endswith('.c') and not pre else 0
        ins = next((x for x in m if x.group(0) > inc and x.start() >= own), None)
        pos = ins.start() if ins else m[-1].end()
        s = s[:pos] + inc + s[pos:]
        open(f, 'w').write(s)
```

`cuts/probe_comp.py` :

```python
from _h import *
import glob
def cut_span(f, start, end):
    s = rd(f); i = s.index(start); j = s.index(end, i) + len(end)
    while s[j:j+1] == '\n': j += 1
    wr(f, s[:i] + s[j:]); return s[i:j].rstrip('\n') + '\n'
t = cut_span('pico8_probe.h', '/* What `bv`, `dl` and `d_` read and write', '} pico8_probe_t;')
fns = cut_span('pico8_probe.h', '/* TROU-7 */\n/* `mp` is stored into `ra`', 'void pico8_d__dispatch(void *self, pico8_struct_header_t *kv);')
wr('pico8_probe_comp.h', '#ifndef PICO8_PROBE_COMP_H\n#define PICO8_PROBE_COMP_H\n\n#include "pico8_fixed.h"\n#include "pico8_gfx.h"\n'
   '#include "pico8_mem.h"\n#include "pico8_struct.h"\n#include "pico8_vec.h"\n\n' + t + '\n' + fns + '\n#endif\n')
sub('pico8_probe.h', '#include "pico8_particle.h"\n', '#include "pico8_particle.h"\n#include "pico8_probe_comp.h"\n')
for f in ('pico8_slipway.h', 'pico8_wormhole.h', 'pico8_slipway.c', 'pico8_wormhole.c'):
    sub(f, '#include "pico8_probe.h"\n', '#include "pico8_probe_comp.h"\n')
```

`cuts/t4_ui.py` :

```python
from _h import *
sub('pico8_slipway.h', 'const pico8_struct_class_t *pico8_jc_struct_class(void);\n', '')
sub('pico8_probe.h', 'const pico8_struct_class_t *pico8_g__struct_class(void);\n', '')
add_before_endif('pico8_struct.h', '/* Class identities that a module compares against without including the\n * class\'s own module. */\n'
    'const pico8_struct_class_t *pico8_jc_struct_class(void);\nconst pico8_struct_class_t *pico8_g__struct_class(void);\n')
del_include('pico8_ui.c', 'pico8_probe.h')
del_include('pico8_ui.c', 'pico8_slipway.h')
```

`cuts/na_planet.py` :

```python
from _h import *
fn = cut_fn('pico8_entity_geom.c', 'void pico8_na(')
d = cut_decl('pico8_entity_geom.h', 'void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t);')
append('pico8_planet.c', fn)
add_before_endif('pico8_planet.h', d)
```

`cuts/kv_family.py` :

```python
from _h import *
s = rd('pico8_planet.c')
i = s.index('/* `kv`, indexed by `fz`'); j = s.index('\n}\n', s.index('static const pico8_entity_kind_t *jo_kv(')) + 3
blk = s[i:j]; wr('pico8_planet.c', s[:i] + s[j:].lstrip('\n'))
blk = blk.replace('static const pico8_entity_kind_t *jo_kv(', 'const pico8_entity_kind_t *pico8_jo_kv(').replace('/* NULL when no kind has `fz`. */\n', '')
append('pico8_planet_family.c', blk)
sub('pico8_planet.c', 'jo_kv(hp_instance->cw)', 'pico8_jo_kv(hp_instance->cw)')
add_before_endif('pico8_planet.h', '/* NULL when no kind has `fz`. */\nconst pico8_entity_kind_t *pico8_jo_kv(const char *fz);\n')
```

`cuts/gd_mo.py` :

```python
from _h import *
sub('pico8_entity_geom.c', '''    const pico8_qb_t *mo = pico8_mo_qb();
    pico8_vec_t mo_ra = mo != NULL && mo->ra != NULL ? *mo->ra : pico8_entity_geom_zero();''',
'''    pico8_vec_t mo_ra = pico8_entity_geom_ra(pico8_mo());''')
```

`cuts/sim_214b.py` :

```python
from _h import *
del_include('pico8_particle.c', 'pico8_probe.h')
```

`cuts/sim_215d.py` :

```python
from _h import *
del_include('pico8_planet_display.c', 'pico8_planet_family.h')
```

`cuts/sim_e.py` :

```python
from _h import *
del_include('pico8_entity_geom.c', 'pico8_planet.h')
del_include('pico8_planet.c', 'pico8_planet_family.h')
```

## Annexe B — contrôles

Contrôle 1 (lignes), depuis le dossier des essais :

```sh
sed -n '/^### Les 43 lignes/,/^### Paires d/p' docs/etude-2-11-inclusions.md \
  | awk -F'|' '/^\| [0-9]/{gsub(/[` ]/,"",$3); print "src/pico8_"$3}' | sort > logs/ctl1_fiches.txt
grep '^inclusion-mutuelle:' logs/base.lines | sed 's/^inclusion-mutuelle: //; s/:#include.*//' | sort > logs/ctl1_compteur.txt
comm -3 logs/ctl1_compteur.txt logs/ctl1_fiches.txt
```

Sortie : 43 lignes de part et d'autre, `comm -3` vide.

Contrôle 2 (paires) : `cut -f2,3 logs/base.detail | grep …` rend les 17 paires, `logs/base.use` les 20 ; chacune a sa fiche (1 à 17, c1 à c3).

Sites : `python3 sites.py tree logs/use0.tsv` (324 lignes, fichier:ligne, fonction englobante, symboles) ; relancé sur l'essai `na_planet` + `gd_mo` + `kv_family`, il ne laisse, pour les arêtes qui attendent 2.14/2.15, que :

```
## pico8_entity_geom -> pico8_planet
  pico8_entity_geom.c:64	pico8_ma	pico8_jo_is_family
  pico8_entity_geom.c:66	pico8_ma	pico8_jo_is_family pico8_jo_t
## pico8_entity_geom -> pico8_scroll
  pico8_entity_geom.c:64	pico8_ma	pico8_qb_struct_class
  pico8_entity_geom.c:66	pico8_ma	pico8_qb_t
## pico8_planet -> pico8_planet_family
  pico8_planet.c:269	pico8_jo_pq	pico8_qh_t
  pico8_planet.c:280	pico8_jo_pq	pico8_qh_of
```

## Annexe C — tableau « Runs »

Copie de `9ca66f4` ; chaque essai part de la copie remise à zéro.

| # | Commande | Code | Ligne clé | Durée |
|---|---|---|---|---|
| 1 | `git clone` local ; `sh tools/composition_residue.sh` | 0 | `inclusion-mutuelle 17 16`, `void-ptr 8 4` ; `résidu 2 : 1073 ligne(s), 85 fichier(s)` | < 1 s |
| 2 | `cross.py`, `usegraph.py`, `usegraph_e.py` | 0 | 43 lignes, symboles du PRD ; `paires 20` ; 636 arêtes orientées | < 2 s |
| 3 | `sites.py` | 0 | 324 lignes de sites | < 1 s |
| 4 | `trial.sh base` | 0 | 0 erreur ; 17 / 43 ; usage 20 ; pied 1073 / 85 | 2,1 s |
| 5 | `trial.sh t1_mz` | 0 | 0 erreur ; 16 / 40 ; usage 19 ; −`wormhole` ↔ `wormhole_display` | 2,2 s |
| 6 | `trial.sh t1_jc` | 0 | 0 erreur ; 16 / 40 ; usage 19 ; −`slipway` ↔ `slipway_display` | 2,2 s |
| 7 | `trial.sh t1_pj` | 0 | 0 erreur ; 16 / 40 ; usage 19 ; −`score_panel` ↔ `score_panel_display` | 2,3 s |
| 8 | `trial.sh t3_dj` | 0 | 0 erreur ; 16 / 41 ; usage 19 ; −`economy_formulas` ↔ `slipway` | 2,2 s |
| 9 | `trial.sh t1_jo_ui` | 0 | 0 erreur ; 15 / 38 ; usage 18 ; −`planet` ↔ `planet_display`, −`planet` ↔ `slipway` | 2,2 s |
| 10 | `trial.sh t1_jo_trade` | 0 | 0 erreur ; 16 / 40 ; usage 19 ; −`planet` ↔ `planet_trade` | 2,2 s |
| 11 | `trial.sh t2_fm` (1re version du script) | 1 | `IndexError` : un TU sans include `"pico8_…"` (les TU incluent `"../src/pico8_…"`) ; script corrigé | < 1 s |
| 12 | `trial.sh t2_fm` | 0 | 0 erreur ; 16 / 41 ; usage **17** ; −`planet` ↔ `probe`, −c1, −c2 | 2,4 s |
| 13 | `trial.sh kr_score` | 0 | 0 erreur ; 16 / 41 ; usage 19 ; −`economy` ↔ `planet` | 3,0 s |
| 14 | `trial.sh probe_comp` | 0 | **3 erreurs** (`slipway.c:195`, `pico8_fm_hy`) ; 15 / 37 ; usage 19 : `probe` ↔ `slipway` sort du compteur, pas de l'usage | 3,3 s |
| 15 | `trial.sh t2_fm probe_comp` | 0 | 0 erreur ; 14 / 35 ; usage 15 ; −`planet` ↔ `probe`, −`probe` ↔ `slipway`, −`probe` ↔ `wormhole`, −c1, −c2 | 2,3 s |
| 16 | `trial.sh t4_ui` | 0 | 0 erreur ; 15 / 39 ; usage 18 ; −`probe` ↔ `ui`, −`slipway` ↔ `ui` | 2,2 s |
| 17 | `trial.sh na_planet` ; `kv_family` ; `gd_mo` | 0 | 0 erreur chacun ; 17 / 43 ; usage 20 ; aucune entrante | 3 × 2,2 s |
| 18 | `sites.py` sur `na_planet` + `gd_mo` + `kv_family` | 0 | restent `entity_geom.c:64`, `:66` (`pico8_ma`), `planet.c:269`, `:280` (`pico8_jo_pq`) | < 1 s |
| 19 | contrôle 1 (`comm -3`) | 0 | vide ; 43 = 43 | < 1 s |
| 20 | `trial.sh` des 13 coupes ensemble (essai cumulé) | 0 | 0 erreur ; **4 / 10** ; usage **5** ; 15 paires d'usage sortantes, aucune entrante ; pied 1040 / 74 | 2,6 s |
| 21 | `make -j4 test` sur l'essai cumulé (1re fois) | 2 | `tests/test_pico8_mz.c:9: fatal error: pico8_fm.h` : les TU compilent sans `-Isrc` et incluent `"../src/…"` (`syn.sh` ajoute `-Isrc` et ne le voyait pas) ; `t2_fm.py` corrigé | 3 s |
| 22 | `trial.sh` cumulé rejoué ; `make -j4 test` ; `make e2e` | 0 / 0 | `test_composition_residue: all tests passed`, `test_class_coverage: all tests passed` ; `e2e: 26 e2e`, 26 `conforme` | 8 s + 1 s |
| 23 | `clang -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only` sur l'essai cumulé | 0 | 0 erreur | < 30 s |
| 24 | `objs.sh` (1re version, `-Werror` avec `-DNDEBUG`) | 1 | variables d'`assert` inutilisées sous `-DNDEBUG` ; drapeaux d'avertissement retirés | ≈ 10 s |
| 25 | `objs.sh` au départ puis sur `t2_fm` + `probe_comp` + `t4_ui` ; `cmp` des `.dis` et `.nm` | 0 | 73 objets, aucun écart | ≈ 20 s |
| 26 | `usegraph_e.py` départ et cumulé ; `scc.py` | 0 | composantes 26 + 2 → 25 | < 1 s |
| 27 | `trial.sh` en séquence (`sim_214b`, `sim_215d`, 2.17a à 2.17e, `sim_e`) | 0 | 16/40, 15/38, 12/29, 9/21, 7/17, 2/5, **0/0** (les `sim_*` ne compilent pas avant 2.14/2.15, attendu) | 7 × 2,3 s |
| 28 | copie supprimée (`rm -rf tree obj_*`) | 0 | `ls` : scripts et `logs/` seuls | < 1 s |

Chaque essai a été précédé de `git checkout -- . && git clean -fd src tests` sur la copie. Arbre de travail : aucun fichier de `src/`, `tests/`, `tools/` ni le `Makefile` n'a changé ; `make composition-residue` y rend le même pied qu'au départ (1073 / 85).

## Annexe D — diff cumulé de l'essai (§3), `src/` seul

Produit par `git diff -U1 -- src` sur la copie après les 13 coupes (33 fichiers, dont les deux en-têtes neufs ; +300 / −248). Les 17 TU ne gagnent chacun que `#include "../src/pico8_fm.h"`. Les scripts de l'annexe A le reproduisent exactement depuis `9ca66f4`.

<details><summary>diff (865 lignes)</summary>

```diff
diff --git a/src/pico8_cart.c b/src/pico8_cart.c
index 1de9bbc..015bacb 100644
--- a/src/pico8_cart.c
+++ b/src/pico8_cart.c
@@ -9,2 +9,3 @@
 #include "pico8_bootstrap.h"
+#include "pico8_fm.h"
 #include "pico8_struct.h"
diff --git a/src/pico8_cheat.c b/src/pico8_cheat.c
index ee515af..63ec4e5 100644
--- a/src/pico8_cheat.c
+++ b/src/pico8_cheat.c
@@ -3,2 +3,3 @@
 #include "pico8_fixed.h"
+#include "pico8_fm.h"
 #include "pico8_probe.h"
diff --git a/src/pico8_economy.c b/src/pico8_economy.c
index bbfa5be..f525a26 100644
--- a/src/pico8_economy.c
+++ b/src/pico8_economy.c
@@ -11,2 +11,3 @@
 #include "pico8_entity.h"
+#include "pico8_fm.h"
 #include "pico8_game_data.h"
@@ -14,3 +15,2 @@
 #include "pico8_msgbox.h"
-#include "pico8_planet.h"
 #include "pico8_tech_gate.h"
@@ -218,10 +218,4 @@ void pico8_hy_lj(pico8_hy_t *self) {
             if (e != NULL) {
-                pico8_struct_header_t *p = e;
-                pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
-                pico8_jo_t *jo;
-
-                assert(pico8_jo_is_family(p) && "hy:lj: cg.jo holds planets only");
-                jo = (pico8_jo_t *)p;
-                jo->kr = kr;
-                jo->has_kr = 1;
+                pico8_fixed_t kr = pico8_kr(e);
+
                 dd = pico8_fixed_add(dd, kr);
diff --git a/src/pico8_economy.h b/src/pico8_economy.h
index c08ac20..7651444 100644
--- a/src/pico8_economy.h
+++ b/src/pico8_economy.h
@@ -4,2 +4,3 @@
 #include "pico8_economy_formulas.h"
+#include "pico8_fm.h"
 #include "pico8_fixed.h"
@@ -11,26 +12,2 @@
 
-/* The economy, one per game. Nothing here frees it: pico8_probe_set_fm()
- * and pico8_probe_shutdown() free the struct alone, never `invented`
- * (TROU-7). `po`/`fu` are NULL before the first pico8_hy_lj(), then point
- * at `po_own`/`fu_own`, which every pico8_hy_lj() refills in place. `hl` is
- * absent before the first pico8_hy_lj(). */
-typedef struct {
-    pico8_struct_header_t header;
-    pico8_fixed_t nz;
-    pico8_fixed_t ge;
-    pico8_fixed_t jk;
-    pico8_fixed_t ew;
-    pico8_ptrs_t *invented;
-    pico8_fixed_t lo;
-    pico8_fixed_t yr;
-    pico8_qe_map_t *po;
-    pico8_qe_map_t *fu;
-    pico8_qe_map_t po_own;
-    pico8_qe_map_t fu_own;
-    pico8_fixed_t happiness;
-    int has_happiness; /* 0: `happiness` is nil, and is then never read */
-    const char *hl;
-    int has_hl; /* 0: `hl` is nil; 1 with `hl` NULL: `hl` is false */
-} pico8_hy_t;
-
 /* Marks the module ready only. The singleton `fm=hy()` is built later, by
diff --git a/src/pico8_economy_formulas.c b/src/pico8_economy_formulas.c
index f174ae7..bbab6ed 100644
--- a/src/pico8_economy_formulas.c
+++ b/src/pico8_economy_formulas.c
@@ -9,3 +9,2 @@
 #include "pico8_research.h"
-#include "pico8_slipway.h"
 #include "pico8_vec.h"
@@ -131,4 +130,4 @@ void pico8_ck(pico8_qe_map_t *out) {
 
-pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to) {
-    pico8_vec_t diff = pico8_vec_sub(pico8_jc_end_p(to), pico8_jc_end_p(pb));
+pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p) {
+    pico8_vec_t diff = pico8_vec_sub(to_p, pb_p);
     pico8_fixed_t len_sq = pico8_vec_len_sq(diff);
diff --git a/src/pico8_economy_formulas.h b/src/pico8_economy_formulas.h
index 8c21af1..b544c50 100644
--- a/src/pico8_economy_formulas.h
+++ b/src/pico8_economy_formulas.h
@@ -6,2 +6,3 @@
 #include "pico8_map_tables.h"
+#include "pico8_vec.h"
 
@@ -37,3 +38,3 @@ void pico8_ck(pico8_qe_map_t *out);
 /* `#(to.p-pb.p)` is the SQUARED distance, not the Euclidean one. */
-pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to);
+pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p);
 
diff --git a/src/pico8_economy_score.c b/src/pico8_economy_score.c
index b32615c..0bd520e 100644
--- a/src/pico8_economy_score.c
+++ b/src/pico8_economy_score.c
@@ -9,2 +9,3 @@
 #include "pico8_entity.h"
+#include "pico8_fm.h"
 #include "pico8_game_data.h"
@@ -268 +269,12 @@ void pico8_economy_set_dk(const pico8_js_t *dk) {
 }
+
+pico8_fixed_t pico8_kr(pico8_struct_header_t *p) {
+    pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
+    pico8_jo_t *jo;
+
+    assert(pico8_jo_is_family(p) && "hy:lj: cg.jo holds planets only");
+    jo = (pico8_jo_t *)p;
+    jo->kr = kr;
+    jo->has_kr = 1;
+    return kr;
+}
diff --git a/src/pico8_economy_score.h b/src/pico8_economy_score.h
index 8ac1a7f..efd507e 100644
--- a/src/pico8_economy_score.h
+++ b/src/pico8_economy_score.h
@@ -39,2 +39,5 @@ void pico8_economy_set_dk(const pico8_js_t *dk);
 
+/* `kr` of planet `p`, written into it and returned. */
+pico8_fixed_t pico8_kr(pico8_struct_header_t *p);
+
 #endif
diff --git a/src/pico8_entity_geom.c b/src/pico8_entity_geom.c
index a31edd0..f510c38 100644
--- a/src/pico8_entity_geom.c
+++ b/src/pico8_entity_geom.c
@@ -48,4 +48,3 @@ int pico8_gd(const pico8_struct_header_t *e) {
     pico8_vec_t e_ra = pico8_entity_geom_ra(e);
-    const pico8_qb_t *mo = pico8_mo_qb();
-    pico8_vec_t mo_ra = mo != NULL && mo->ra != NULL ? *mo->ra : pico8_entity_geom_zero();
+    pico8_vec_t mo_ra = pico8_entity_geom_ra(pico8_mo());
     pico8_fixed_t sixty_four = pico8_fixed_from_int(64);
@@ -57,14 +56,2 @@ int pico8_gd(const pico8_struct_header_t *e) {
 
-void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t) {
-    pico8_vec_t *ra_vec;
-
-    assert((self == NULL || pico8_jo_is_family(self)) && "na: self is a planet (qh:ly, nl:ly) -- stop");
-    ra_vec = self != NULL ? ((pico8_jo_t *)self)->ra : NULL;
-    if (ra_vec != NULL) {
-        pico8_fixed_t delta = pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000147, 0.005))),
-                                               PICO8_FIXED_LIT(0x00000f5c, 0.06));
-        ra_vec->y = pico8_fixed_add(ra_vec->y, delta);
-    }
-}
-
 pico8_vec_t pico8_ma(pico8_vec_t p, pico8_struct_header_t *e) {
diff --git a/src/pico8_entity_geom.h b/src/pico8_entity_geom.h
index 0628b7d..66a143b 100644
--- a/src/pico8_entity_geom.h
+++ b/src/pico8_entity_geom.h
@@ -20,6 +20,2 @@ int pico8_gd(const pico8_struct_header_t *e);
 
-/* Mutates the pico8_vec_t `self.ra` points at, in place: every other holder
- * of that vector -- the `iv` toast `jo:hk` builds from `self.ra` -- sees the
- * bob. Rebinding the field to a fresh vector, as `qb:kj` does, would not. */
-void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t);
 
diff --git a/src/pico8_fm.h b/src/pico8_fm.h
new file mode 100644
index 0000000..32c2ac6
--- /dev/null
+++ b/src/pico8_fm.h
@@ -0,0 +1,61 @@
+#ifndef PICO8_FM_H
+#define PICO8_FM_H
+
+#include "pico8_economy_formulas.h"
+#include "pico8_fixed.h"
+#include "pico8_ptrs.h"
+#include "pico8_struct.h"
+
+/* The economy, one per game. Nothing here frees it: pico8_probe_set_fm()
+ * and pico8_probe_shutdown() free the struct alone, never `invented`
+ * (TROU-7). `po`/`fu` are NULL before the first pico8_hy_lj(), then point
+ * at `po_own`/`fu_own`, which every pico8_hy_lj() refills in place. `hl` is
+ * absent before the first pico8_hy_lj(). */
+typedef struct pico8_hy {
+    pico8_struct_header_t header;
+    pico8_fixed_t nz;
+    pico8_fixed_t ge;
+    pico8_fixed_t jk;
+    pico8_fixed_t ew;
+    pico8_ptrs_t *invented;
+    pico8_fixed_t lo;
+    pico8_fixed_t yr;
+    pico8_qe_map_t *po;
+    pico8_qe_map_t *fu;
+    pico8_qe_map_t po_own;
+    pico8_qe_map_t fu_own;
+    pico8_fixed_t happiness;
+    int has_happiness; /* 0: `happiness` is nil, and is then never read */
+    const char *hl;
+    int has_hl; /* 0: `hl` is nil; 1 with `hl` NULL: `hl` is false */
+} pico8_hy_t;
+
+/* The provisional `fm`: one static struct, never allocated nor freed, with
+ * no method and no writer; `jk`/`nz` are written only by the cheat code, and
+ * set back to nil by every `pico8_probe_init()`. */
+typedef struct {
+    pico8_struct_header_t header;
+    pico8_fixed_t jk;
+    int has_jk; /* 0: `jk` is nil, and is then never read */
+    pico8_fixed_t nz;
+    int has_nz; /* 0: `nz` is nil, and is then never read */
+} pico8_fm_placeholder_t;
+
+const pico8_struct_class_t *pico8_fm_placeholder_struct_class(void);
+
+/* `fm`: the provisional, a static struct that has only `jk`/`nz` (written by
+ * the cheat code), which `pico8_probe_init()` installs, then, from `eu()` on,
+ * a `hy` struct; read it through pico8_fm_hy() and pico8_fm_placeholder().
+ * NULL before init and after shutdown, which several callers test for. The
+ * setter takes ownership of the new occupant and frees a previous `hy` as
+ * the struct alone (TROU-7); never the provisional. */
+pico8_struct_header_t *pico8_fm(void);
+void pico8_probe_set_fm(pico8_struct_header_t *instance);
+
+/* The `hy` of `fm`, NULL when the slot is empty and on the provisional. */
+pico8_hy_t *pico8_fm_hy(void);
+
+/* The provisional `fm`, NULL when the slot is empty or holds a `hy`. */
+const pico8_fm_placeholder_t *pico8_fm_placeholder(void);
+
+#endif
diff --git a/src/pico8_menu.c b/src/pico8_menu.c
index df07069..a59fab2 100644
--- a/src/pico8_menu.c
+++ b/src/pico8_menu.c
@@ -7,2 +7,3 @@
 #include "pico8_audio.h"
+#include "pico8_fm.h"
 #include "pico8_struct.h"
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index ab73752..0615554 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -13,2 +13,3 @@
 #include "pico8_entity_geom.h"
+#include "pico8_fm.h"
 #include "pico8_gen.h"
@@ -17,8 +18,4 @@
 #include "pico8_pi.h"
-#include "pico8_planet_display.h"
 #include "pico8_planet_family.h"
-#include "pico8_planet_trade.h"
-#include "pico8_probe.h"
 #include "pico8_research.h"
-#include "pico8_slipway.h"
 #include "pico8_tech_gate.h"
@@ -38,10 +35,2 @@ int pico8_jo_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *r
 
-void pico8_jo_hb_dispatch(void *self) {
-    pico8_jo_hb((pico8_jo_t *)self);
-}
-
-pico8_down_t pico8_jo_gv_dispatch(void *self, const pico8_struct_header_t *s) {
-    return pico8_jo_gv((const pico8_jo_t *)self, s);
-}
-
 void pico8_jo_hk_dispatch(void *self, pico8_fixed_t yr) {
@@ -51,13 +40,2 @@ void pico8_jo_hk_dispatch(void *self, pico8_fixed_t yr) {
 
-void pico8_jo_kj_dispatch(void *self, const pico8_vec_t *ra) {
-    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-    pico8_jo_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
-}
-
-pico8_down_t pico8_jo_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
-    (void)qa;
-    (void)ra;
-    return pico8_jo_ldown((pico8_jo_t *)self);
-}
-
 int pico8_jo_is_family(const pico8_struct_header_t *e) {
@@ -66,10 +44,2 @@ int pico8_jo_is_family(const pico8_struct_header_t *e) {
 
-void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw) {
-    pico8_jo_kf((pico8_jo_t *)self, sw);
-}
-
-void pico8_jo_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw) {
-    pico8_jo_cn((pico8_jo_t *)self, lw);
-}
-
 void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
@@ -355,24 +325,2 @@ void pico8_jo_ll(pico8_jo_t *self, pico8_fixed_t t) {
 
-/* `kv`, indexed by `fz`: `pm` and `ga` are the only kinds whose spec sets
- * one. */
-static const struct {
-    const char *fz;
-    const pico8_entity_kind_t *(*kind)(void);
-} g_kv[] = {
-    {"pm", pico8_pm_kind},
-    {"ga", pico8_ga_kind},
-};
-
-/* NULL when no kind has `fz`. */
-static const pico8_entity_kind_t *jo_kv(const char *fz) {
-    size_t i;
-
-    for (i = 0; i < sizeof g_kv / sizeof g_kv[0]; i++) {
-        if (strcmp(g_kv[i].fz, fz) == 0) {
-            return g_kv[i].kind();
-        }
-    }
-    return NULL;
-}
-
 void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance) {
@@ -394,3 +342,3 @@ void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance) {
 
-        target_kind = jo_kv(hp_instance->cw);
+        target_kind = pico8_jo_kv(hp_instance->cw);
 
@@ -485,24 +433,12 @@ void pico8_jo_hk(const pico8_jo_t *self) {
 
-pico8_down_t pico8_jo_ldown(pico8_jo_t *self) {
-    const char *me_val;
-    pico8_down_t out = {0, NULL};
-
-    if (self == NULL || self->header.me == NULL) {
-        return out;
-    }
-    me_val = self->header.me;
-    if (strcmp(me_val, "known") == 0) {
-        pico8_jo_fe(self);
-        out.stop = 1;
-        return out;
-    }
-    if (strcmp(me_val, "ly") == 0) {
-        pico8_jc_keys_t keys = {0};
+void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t) {
+    pico8_vec_t *ra_vec;
 
-        assert(pico8_jc_ready() && "pico8_jc_init() must run before jo:ldown builds a jc");
-        keys.qu = (pico8_struct_header_t *)self;
-        out.held = (pico8_struct_header_t *)pico8_jc_new(&keys);
-        out.stop = out.held != NULL;
+    assert((self == NULL || pico8_jo_is_family(self)) && "na: self is a planet (qh:ly, nl:ly) -- stop");
+    ra_vec = self != NULL ? ((pico8_jo_t *)self)->ra : NULL;
+    if (ra_vec != NULL) {
+        pico8_fixed_t delta = pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000147, 0.005))),
+                                               PICO8_FIXED_LIT(0x00000f5c, 0.06));
+        ra_vec->y = pico8_fixed_add(ra_vec->y, delta);
     }
-    return out;
 }
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index e94350c..2b8d802 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -162,2 +162,10 @@ void pico8_jo_set_ir(pico8_jf_state_t *ir);
 
+/* Mutates the pico8_vec_t `self.ra` points at, in place: every other holder
+ * of that vector -- the `iv` toast `jo:hk` builds from `self.ra` -- sees the
+ * bob. Rebinding the field to a fresh vector, as `qb:kj` does, would not. */
+void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t);
+
+/* NULL when no kind has `fz`. */
+const pico8_entity_kind_t *pico8_jo_kv(const char *fz);
+
 #endif
diff --git a/src/pico8_planet_display.c b/src/pico8_planet_display.c
index 1616522..131f1ae 100644
--- a/src/pico8_planet_display.c
+++ b/src/pico8_planet_display.c
@@ -13,2 +13,3 @@
 #include "pico8_entity_geom.h"
+#include "pico8_fm.h"
 #include "pico8_hud.h"
@@ -483 +484,48 @@ void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec
 }
+
+void pico8_jo_hb_dispatch(void *self) {
+    pico8_jo_hb((pico8_jo_t *)self);
+}
+
+
+pico8_down_t pico8_jo_gv_dispatch(void *self, const pico8_struct_header_t *s) {
+    return pico8_jo_gv((const pico8_jo_t *)self, s);
+}
+
+
+void pico8_jo_kj_dispatch(void *self, const pico8_vec_t *ra) {
+    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
+    pico8_jo_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
+}
+
+
+pico8_down_t pico8_jo_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
+    (void)qa;
+    (void)ra;
+    return pico8_jo_ldown((pico8_jo_t *)self);
+}
+
+
+pico8_down_t pico8_jo_ldown(pico8_jo_t *self) {
+    const char *me_val;
+    pico8_down_t out = {0, NULL};
+
+    if (self == NULL || self->header.me == NULL) {
+        return out;
+    }
+    me_val = self->header.me;
+    if (strcmp(me_val, "known") == 0) {
+        pico8_jo_fe(self);
+        out.stop = 1;
+        return out;
+    }
+    if (strcmp(me_val, "ly") == 0) {
+        pico8_jc_keys_t keys = {0};
+
+        assert(pico8_jc_ready() && "pico8_jc_init() must run before jo:ldown builds a jc");
+        keys.qu = (pico8_struct_header_t *)self;
+        out.held = (pico8_struct_header_t *)pico8_jc_new(&keys);
+        out.stop = out.held != NULL;
+    }
+    return out;
+}
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index 6c20d8e..7fbf359 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -768 +768,22 @@ pico8_ga_t *pico8_ga_new(const pico8_ga_keys_t *keys) {
 }
+
+/* `kv`, indexed by `fz`: `pm` and `ga` are the only kinds whose spec sets
+ * one. */
+static const struct {
+    const char *fz;
+    const pico8_entity_kind_t *(*kind)(void);
+} g_kv[] = {
+    {"pm", pico8_pm_kind},
+    {"ga", pico8_ga_kind},
+};
+
+const pico8_entity_kind_t *pico8_jo_kv(const char *fz) {
+    size_t i;
+
+    for (i = 0; i < sizeof g_kv / sizeof g_kv[0]; i++) {
+        if (strcmp(g_kv[i].fz, fz) == 0) {
+            return g_kv[i].kind();
+        }
+    }
+    return NULL;
+}
diff --git a/src/pico8_planet_trade.c b/src/pico8_planet_trade.c
index 9538921..643c30e 100644
--- a/src/pico8_planet_trade.c
+++ b/src/pico8_planet_trade.c
@@ -250 +250,10 @@ pico8_ents_t *pico8_jo_ib(pico8_jo_t *self) {
 }
+
+void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw) {
+    pico8_jo_kf((pico8_jo_t *)self, sw);
+}
+
+
+void pico8_jo_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw) {
+    pico8_jo_cn((pico8_jo_t *)self, lw);
+}
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index 55af84a..4bfeacc 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -11,2 +11,3 @@
 #include "pico8_fixed.h"
+#include "pico8_fm.h"
 #include "pico8_geom.h"
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index 7b9fc95..1525e21 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -9,2 +9,3 @@
 #include "pico8_particle.h"
+#include "pico8_probe_comp.h"
 #include "pico8_struct.h"
@@ -18,18 +19,2 @@
 
-/* What `bv`, `dl` and `d_` read and write of their host, the probe's own
- * fields, stored in the host (`probe_own`) and never copied. `et` is
- * borrowed; NULL is nil or false, which no reader tells apart. `dq` NULL is
- * nil. `fp` is what `d_` hands to `fm:nx`: the cart's false is held as 0,
- * which is what `self.fp or 0` hands. */
-typedef struct pico8_probe {
-    pico8_struct_header_t *et;
-    pico8_fixed_t dw;
-    pico8_fixed_t od;
-    int has_od; /* 0: `od` is nil, and is then never read */
-    const char *dq;
-    pico8_fixed_t fp;
-    pico8_fixed_t ik;
-    int has_ik; /* 0: `ik` is nil, and is then never read */
-} pico8_probe_t;
-
 /* TROU-7 */
@@ -78,3 +63,2 @@ void pico8_probe_shutdown(void);
 int pico8_g__ready(void);
-const pico8_struct_class_t *pico8_g__struct_class(void);
 
@@ -104,71 +88,2 @@ pico8_g__t *pico8_g__new(const pico8_g__keys_t *keys);
 
-/* The provisional `fm`: one static struct, never allocated nor freed, with
- * no method and no writer; `jk`/`nz` are written only by the cheat code, and
- * set back to nil by every `pico8_probe_init()`. */
-typedef struct {
-    pico8_struct_header_t header;
-    pico8_fixed_t jk;
-    int has_jk; /* 0: `jk` is nil, and is then never read */
-    pico8_fixed_t nz;
-    int has_nz; /* 0: `nz` is nil, and is then never read */
-} pico8_fm_placeholder_t;
-
-const pico8_struct_class_t *pico8_fm_placeholder_struct_class(void);
-
-/* `fm`: the provisional, a static struct that has only `jk`/`nz` (written by
- * the cheat code), which `pico8_probe_init()` installs, then, from `eu()` on,
- * a `hy` struct; read it through pico8_fm_hy() and pico8_fm_placeholder().
- * NULL before init and after shutdown, which several callers test for. The
- * setter takes ownership of the new occupant and frees a previous `hy` as
- * the struct alone (TROU-7); never the provisional. */
-pico8_struct_header_t *pico8_fm(void);
-void pico8_probe_set_fm(pico8_struct_header_t *instance);
-
-/* The `hy` of `fm`, NULL when the slot is empty and on the provisional. */
-pico8_hy_t *pico8_fm_hy(void);
-
-/* The provisional `fm`, NULL when the slot is empty or holds a `hy`. */
-const pico8_fm_placeholder_t *pico8_fm_placeholder(void);
-
-/* TROU-7 */
-/* `mp` is stored into `ra` by REF identity, not copied: the caller's vector
- * stays aliased. `et` ends in one of three states -- a real target, a
- * static struct with no readable field but `fo`, or literal `false` -- of
- * which only `false` is falsy; collapsing the throwaway to nil would change
- * `d_`. */
-void pico8_bv(pico8_struct_header_t *self, pico8_vec_t *mp, pico8_vec_t *rp);
-
-void pico8_bv_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
-
-/* Points the `fo` of `et` back at `self`, which it borrows: `et` is a planet
- * of jo's family, a `jc`, the `jc_too_far` struct or `bv`'s throwaway target
- * (asserted, as is the class of `self` each accepts). */
-void pico8_probe_et_hang_fo(pico8_struct_header_t *et, pico8_struct_header_t *self);
-
-/* The `{}` `bv` puts in `et` over the void: one static struct, never
- * allocated nor freed; only `fo` is written, by pico8_probe_et_hang_fo(),
- * and the next `bv` overwrites it. */
-typedef struct {
-    pico8_struct_header_t header;
-    pico8_struct_header_t *fo;
-} pico8_bv_et_t;
-
-const pico8_struct_class_t *pico8_bv_et_struct_class(void);
-
-/* The tooltip of a probe host: `self` must carry a probe (asserted), and is
- * read, never retained. */
-void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self);
-
-/* The same, for a caller that is not a host (the research icon): `me` NULL
- * is nil, `pb` is read during the call only. */
-void pico8_dl_show(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *me, const pico8_probe_t *pb);
-
-void pico8_dl_dispatch(void *self, const pico8_vec_t *ra);
-
-/* Returns 1 on the success branch (Lua true), 0 on the other (nil); `jc:ey`
- * and `mz:ey` consume it. */
-int pico8_d_(pico8_struct_header_t *self);
-
-void pico8_d__dispatch(void *self, pico8_struct_header_t *kv);
-
 /* TROU-7 */
diff --git a/src/pico8_probe_comp.h b/src/pico8_probe_comp.h
new file mode 100644
index 0000000..b42dde0
--- /dev/null
+++ b/src/pico8_probe_comp.h
@@ -0,0 +1,67 @@
+#ifndef PICO8_PROBE_COMP_H
+#define PICO8_PROBE_COMP_H
+
+#include "pico8_fixed.h"
+#include "pico8_gfx.h"
+#include "pico8_mem.h"
+#include "pico8_struct.h"
+#include "pico8_vec.h"
+
+/* What `bv`, `dl` and `d_` read and write of their host, the probe's own
+ * fields, stored in the host (`probe_own`) and never copied. `et` is
+ * borrowed; NULL is nil or false, which no reader tells apart. `dq` NULL is
+ * nil. `fp` is what `d_` hands to `fm:nx`: the cart's false is held as 0,
+ * which is what `self.fp or 0` hands. */
+typedef struct pico8_probe {
+    pico8_struct_header_t *et;
+    pico8_fixed_t dw;
+    pico8_fixed_t od;
+    int has_od; /* 0: `od` is nil, and is then never read */
+    const char *dq;
+    pico8_fixed_t fp;
+    pico8_fixed_t ik;
+    int has_ik; /* 0: `ik` is nil, and is then never read */
+} pico8_probe_t;
+
+/* TROU-7 */
+/* `mp` is stored into `ra` by REF identity, not copied: the caller's vector
+ * stays aliased. `et` ends in one of three states -- a real target, a
+ * static struct with no readable field but `fo`, or literal `false` -- of
+ * which only `false` is falsy; collapsing the throwaway to nil would change
+ * `d_`. */
+void pico8_bv(pico8_struct_header_t *self, pico8_vec_t *mp, pico8_vec_t *rp);
+
+void pico8_bv_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
+
+/* Points the `fo` of `et` back at `self`, which it borrows: `et` is a planet
+ * of jo's family, a `jc`, the `jc_too_far` struct or `bv`'s throwaway target
+ * (asserted, as is the class of `self` each accepts). */
+void pico8_probe_et_hang_fo(pico8_struct_header_t *et, pico8_struct_header_t *self);
+
+/* The `{}` `bv` puts in `et` over the void: one static struct, never
+ * allocated nor freed; only `fo` is written, by pico8_probe_et_hang_fo(),
+ * and the next `bv` overwrites it. */
+typedef struct {
+    pico8_struct_header_t header;
+    pico8_struct_header_t *fo;
+} pico8_bv_et_t;
+
+const pico8_struct_class_t *pico8_bv_et_struct_class(void);
+
+/* The tooltip of a probe host: `self` must carry a probe (asserted), and is
+ * read, never retained. */
+void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self);
+
+/* The same, for a caller that is not a host (the research icon): `me` NULL
+ * is nil, `pb` is read during the call only. */
+void pico8_dl_show(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *me, const pico8_probe_t *pb);
+
+void pico8_dl_dispatch(void *self, const pico8_vec_t *ra);
+
+/* Returns 1 on the success branch (Lua true), 0 on the other (nil); `jc:ey`
+ * and `mz:ey` consume it. */
+int pico8_d_(pico8_struct_header_t *self);
+
+void pico8_d__dispatch(void *self, pico8_struct_header_t *kv);
+
+#endif
diff --git a/src/pico8_score_panel.c b/src/pico8_score_panel.c
index cae50f6..472342a 100644
--- a/src/pico8_score_panel.c
+++ b/src/pico8_score_panel.c
@@ -12,2 +12,3 @@
 #include "pico8_entity.h"
+#include "pico8_fm.h"
 #include "pico8_game_data.h"
@@ -20,3 +21,2 @@
 #include "pico8_probe.h"
-#include "pico8_score_panel_display.h"
 #include "pico8_str.h"
@@ -264,12 +264,2 @@ static void pico8_pj_kj_dispatch(void *self, const pico8_vec_t *ra) {
 
-static void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr) {
-    (void)self;
-    pico8_pj_hk(yr);
-}
-
-static void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t) {
-    (void)t;
-    pico8_pj_ph((pico8_pj_t *)self);
-}
-
 static int pj_get_fa(const void *self, pico8_fixed_t *out) {
diff --git a/src/pico8_score_panel.h b/src/pico8_score_panel.h
index 15209e6..07470d4 100644
--- a/src/pico8_score_panel.h
+++ b/src/pico8_score_panel.h
@@ -124,2 +124,8 @@ void pico8_eg(void);
 
+void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr);
+void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t);
+
+void pico8_score_panel_display_init(void);
+void pico8_score_panel_display_shutdown(void);
+
 #endif
diff --git a/src/pico8_score_panel_display.c b/src/pico8_score_panel_display.c
index d87ee53..f6af832 100644
--- a/src/pico8_score_panel_display.c
+++ b/src/pico8_score_panel_display.c
@@ -10,2 +10,3 @@
 #include "pico8_entity.h"
+#include "pico8_fm.h"
 #include "pico8_layout.h"
@@ -336 +337,12 @@ pico8_fq_my_fn pico8_de_my(void) {
 }
+
+void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr) {
+    (void)self;
+    pico8_pj_hk(yr);
+}
+
+
+void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t) {
+    (void)t;
+    pico8_pj_ph((pico8_pj_t *)self);
+}
diff --git a/src/pico8_score_panel_display.h b/src/pico8_score_panel_display.h
index 4ff2b39..a4cc883 100644
--- a/src/pico8_score_panel_display.h
+++ b/src/pico8_score_panel_display.h
@@ -17,4 +17,2 @@ void pico8_pj_hk(pico8_fixed_t yr);
 
-void pico8_score_panel_display_init(void);
-void pico8_score_panel_display_shutdown(void);
 
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index 182a75c..21cb979 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -12,2 +12,3 @@
 #include "pico8_entity.h"
+#include "pico8_fm.h"
 #include "pico8_geom.h"
@@ -16,4 +17,3 @@
 #include "pico8_planet.h"
-#include "pico8_probe.h"
-#include "pico8_slipway_display.h"
+#include "pico8_probe_comp.h"
 #include "pico8_tech_gate.h"
@@ -138,3 +138,3 @@ void pico8_jc_hq(pico8_jc_t *self, pico8_vec_t *mp) {
 
-    self->probe->od = pico8_dj(self->qu, self->re);
+    self->probe->od = pico8_dj(pico8_jc_end_p(self->qu), pico8_jc_end_p(self->re));
     self->probe->has_od = 1;
@@ -351,7 +351,2 @@ static void pico8_jc_ev_dispatch(void *self, const pico8_vec_t *ra) {
 
-static void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra) {
-    (void)ra;
-    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jc_t *)self);
-}
-
 static int jc_is_endpoint_class(const pico8_struct_class_t *cls) {
diff --git a/src/pico8_slipway.h b/src/pico8_slipway.h
index 1e7dc07..bbbfc7a 100644
--- a/src/pico8_slipway.h
+++ b/src/pico8_slipway.h
@@ -6,3 +6,3 @@
 #include "pico8_ke.h"
-#include "pico8_probe.h"
+#include "pico8_probe_comp.h"
 #include "pico8_ptrs.h"
@@ -79,3 +79,2 @@ void pico8_jc_shutdown(void);
 int pico8_jc_ready(void);
-const pico8_struct_class_t *pico8_jc_struct_class(void);
 
@@ -127,2 +126,4 @@ void pico8_jc_ev(pico8_jc_t *self);
 
+void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra);
+
 #endif
diff --git a/src/pico8_slipway_display.c b/src/pico8_slipway_display.c
index 3b61d85..3bf1806 100644
--- a/src/pico8_slipway_display.c
+++ b/src/pico8_slipway_display.c
@@ -7,2 +7,3 @@
 #include "pico8_entity.h"
+#include "pico8_dispatch_context.h"
 #include "pico8_entity_geom.h"
@@ -172 +173,6 @@ void pico8_jc_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jc_t *self) {
 }
+
+void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra) {
+    (void)ra;
+    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jc_t *)self);
+}
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 0cd217b..7ec1273 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -173,2 +173,7 @@ typedef struct {
 
+/* Class identities that a module compares against without including the
+ * class's own module. */
+const pico8_struct_class_t *pico8_jc_struct_class(void);
+const pico8_struct_class_t *pico8_g__struct_class(void);
+
 #endif
diff --git a/src/pico8_tech_gate.c b/src/pico8_tech_gate.c
index 4ee1ea7..32ed86f 100644
--- a/src/pico8_tech_gate.c
+++ b/src/pico8_tech_gate.c
@@ -9,2 +9,3 @@
 #include "pico8_entity.h"
+#include "pico8_fm.h"
 #include "pico8_probe.h"
diff --git a/src/pico8_ui.c b/src/pico8_ui.c
index b830c6e..e5fa336 100644
--- a/src/pico8_ui.c
+++ b/src/pico8_ui.c
@@ -12,4 +12,2 @@
 #include "pico8_planet_family.h"
-#include "pico8_probe.h"
-#include "pico8_slipway.h"
 #include "pico8_vec.h"
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index 612cf57..a0e8f29 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -12,5 +12,4 @@
 #include "pico8_planet.h"
-#include "pico8_probe.h"
+#include "pico8_probe_comp.h"
 #include "pico8_tech_gate.h"
-#include "pico8_wormhole_display.h"
 
@@ -189,7 +188,2 @@ pico8_fixed_t pico8_mz_ej(pico8_mz_t *self, pico8_fixed_t t) {
 
-static void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra) {
-    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mz_t *)self, p);
-}
-
 /* Lua's `mz.hq=bv` is rebound three statements later by `function mz:hq`,
diff --git a/src/pico8_wormhole.h b/src/pico8_wormhole.h
index 2d8b31b..05bd574 100644
--- a/src/pico8_wormhole.h
+++ b/src/pico8_wormhole.h
@@ -10,3 +10,3 @@
 #include "pico8_mem.h"
-#include "pico8_probe.h"
+#include "pico8_probe_comp.h"
 #include "pico8_struct.h"
@@ -81,2 +81,4 @@ pico8_fixed_t pico8_mz_ej(pico8_mz_t *self, pico8_fixed_t t);
 
+void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra);
+
 #endif
diff --git a/src/pico8_wormhole_display.c b/src/pico8_wormhole_display.c
index 859e316..dc05586 100644
--- a/src/pico8_wormhole_display.c
+++ b/src/pico8_wormhole_display.c
@@ -7,2 +7,3 @@
 #include "pico8_entity.h"
+#include "pico8_dispatch_context.h"
 #include "pico8_fixed.h"
@@ -75 +76,6 @@ void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mz_t *self, pico8_vec
 }
+
+void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra) {
+    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
+    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mz_t *)self, p);
+}
```

</details>
