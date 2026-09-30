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
- **Essai** : avec la fiche 11. `probe_comp` seul fait déjà sortir `probe` ↔ `wormhole` des deux listes (compteur 15 / 37 ; les 3 erreurs sont celles de `slipway`). Il fait aussi sortir `probe` ↔ `slipway` du compteur, mais pas du graphe d’usage (`pico8_fm_hy` reste) : exemple du piège du détour, que seul le graphe d’usage attrape.
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
