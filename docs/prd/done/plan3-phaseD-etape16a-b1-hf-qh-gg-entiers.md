---
title: "Plan 3, étape D.16a : genres mêlés B1 — `hf` et `qh` en entiers vrai/faux, `pico8_jo_gg` rend `int`, `pico8_jo_hf`/`pico8_qh_qh` et `hf_is_bool`/`qh_is_bool` retirés, quatre copies `truthy` parties ; les TU suivent aux mêmes octets ; aucune constante ne change"
description: "Deux membres de planète (`hf` du cart, `qh` du laboratoire) ne valent jamais que `1` ou `false` et ne sont lus qu'en vérité, mais le port les garde en nombre + drapeau de genre et les relit par des valeurs Lua (`pico8_jo_hf`, `pico8_qh_qh`, `pico8_jo_gg`) que quatre copies privées de `*_truthy` redéballent ; D.16a en fait des `int`, fait rendre `int` à `gg`, retire les enveloppes et les copies, et fait suivre 11 TU sans changer une constante : `valeur` baisse de 54 lignes (55 occ.), ≈ 879 → ≈ 825."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.15c est implémentée** (`plan-pilot`, étape 2b), sur l'arbre commis **`4171b9b`** (« docs: PRD de D.15c revérifié après D.15b et mis en file »), extrait par `git archive 4171b9b` dans le scratchpad. Seuls `grep`, `sed`, `awk` et `sh tools/data_residue.sh` ont tourné. **Rien n'a été compilé ni lancé**, et rien de D.15c au-delà de son PRD (`docs/prd/triage/plan3-phaseD-etape15c-tostr-nombre-dans-str.md` à `4171b9b`) n'a été lu. Tout ce qui dépend du résultat de D.15c est une hypothèse `H-D15c-n` (plus bas). **Les numéros de ligne de ce PRD sont ceux de `4171b9b`** ; dans les trois fichiers que D.15c touche aussi (`economy_score.c`, `planet_display.c`, `score_panel.c`), la colonne « après D.15c » donne le numéro attendu sous H-D15c-2 (+ 1). L'implémenteur remesure **par contenu** au parent de son commit et explique tout écart.

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c/.h` disparaissent (D.19c). L'étude D.14 (`docs/prd/done/plan3-phaseD-etape14-etude-genres-meles.md`) range en **B1** trois valeurs « à genres mêlés » au sens le plus faible : elles prennent deux genres non nil (le nombre `1` et `false`), mais **aucun lecteur ne lit le nombre** ni ne distingue nil de `false`. Le port les tient pourtant comme des valeurs Lua :

- `hf` (planète, défaut de `jo`) : `pico8_fixed_t hf` + `int hf_is_bool`, relu par `pico8_jo_hf()` qui rend une `pico8_value_t` (`FIXED 1`, `BOOL 0` ou `REF NULL`) ;
- `qh` (laboratoire et descendants) : `pico8_fixed_t qh` + `int qh_is_bool`, relu par `pico8_qh_qh()` ;
- `jo:gg()` : `pico8_jo_gg()` rend `self.hf` lui-même quand il est faux, sinon un booléen — « deliberately not collapsed to a C int » dit son contrat (`planet.h:140-141`) ;
- et quatre copies privées de `*_truthy(pico8_value_t)` (`planet.c`, `planet_display.c`, `economy_score.c`, `economy_tiers.c`) plus un prédicat en ligne (`score_panel.c`) n'existent que pour redéballer ces trois valeurs.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `4171b9b` (§5, « Phase D — Données », l. 286) :

> - D.16a — genres mêlés B1 (valeurs vraies/fausses devenues entiers). `class-coverage-net` : oui. Compte : ≈ 845.

Ligne de l'étude D.14 pour cette étape (§3, table des étapes), recopiée :

> | **D.16a** | B1 : `hf`, `qh` en `int`, `pico8_jo_gg` en `int`, `hf_is_bool`/`qh_is_bool` retirés (structs, `pico8_game_data`) ; copies `truthy` de leurs appelants ; TU : plis de `jo_gg` (`fr`, `ga`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`), `spec_defaults` | ≈ 22 fichiers | ≈ 845 |

Point B1 de l'étude (§1.3), recopié :

> **B1. `hf`, `qh` et `jo:gg` — 18 lignes.** `planet.c:348-368` (`pico8_jo_hf`, `pico8_jo_gg`), `planet.h:137,142`, `planet_family.c:38-45` (`pico8_qh_qh`), `.h:46`, le prédicat `score_panel.c:112-122`.
> - Genres : `hf` vaut `1` (défaut de `jo`, lua.lua:387) ou `false` (`qh` et `ga`, lua.lua:909, 1070) ; `qh` vaut `1` (lua.lua:909) ou `false` (lua.lua:1001, 1059, 1098). Nombre ou faux : deux genres.
> - Lecteurs : `self.hf and self.me=="ly"` (450), `not pt.hf` (1306), `oz.hf` (1329), `self.qh and …` (437), `pt.qh and #cg.qh` (1313). Tous en vérité seule. Aucune écriture hors des spécifications de classe, dans le cart comme dans le port (seules écritures : `planet.c:154-155`, `planet_family.c:103-104,619-620`, depuis `pico8_*_start`).
> - Règle (G) : **un entier vrai/faux** (`int hf`, `int qh`), `hf_is_bool`/`qh_is_bool` supprimés des structs et de `pico8_game_data.h/.c` ; `pico8_jo_gg` rend `int` (0 sur `self` NULL, même vérité que nil). Raison : aucun lecteur ne lit le nombre.
> - TROU-3 n'est pas en jeu ici : la comparaison `me=="ly"` est déjà un `strcmp`.

Règles communes de l'étude (§3), recopiées :

> - **Un TU qui nomme un symbole changé le suit dans le même commit.** Un TU peut être converti plus tôt que son symbole ; jamais plus tard.
> - **Aucune empreinte figée ne change** (G1). Pendant D.15–D.18, un TU qui plie une valeur désormais typée reproduit les octets de l'ancien pli (octet de genre + contenu) à partir du type C.
> - Aucun `*.sha256` ni e2e ne change.
> - `class-coverage-net` si l'étape touche un constructeur ; `sanitize-net` si elle change une durée de vie ou un propriétaire.

Lignes directrices de l'étude qui s'appliquent (§4) : **G1** (empreintes figées, même octet de genre et même contenu) ; **G2** (« nil et `false` confondus seulement là où aucun lecteur du cart ne les sépare (preuve par grep des lecteurs dans le PRD) : B1, B3, B4, B6 ») ; B12 (« `economy_*` ne rejette que `false` (un nil y serait lu vrai ; au passage en `int`, le PRD vérifie qu'aucun appelant ne passe un `self` NULL) »).

### Hypothèses `H-D15c-n` (D.15c en cours ; verdicts à écrire à sa graduation)

Source : le PRD de D.15c à `4171b9b` (Inventaire, « Effet sur `make data-residue` », « Includes », « `Makefile` et liens », Sortie). Rien d'autre de D.15c n'a été lu.

| Hypothèse | Énoncé | Ce qui en dépend ici | Vérification à la graduation |
|---|---|---|---|
| H-D15c-1 | Après D.15c : `valeur` **879 / 99 ; 971 occ.** (`src/` 415 / 61 / 466 occ., `tests/` 464 / 38 / 505 occ.) ; `type-tag` 243 / 58 ; résidu D **1120 / 108** ; `DATA_RESIDUE_GUARD` inchangé (`spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit`) | « État de départ », « Effet sur `make data-residue` », critère `FAILED:` de départ, Runs 1-2 | `make -s data-residue` ; recette ; note « Découvert à D.15c » |
| H-D15c-2 | Dans `economy_score.c`, `planet_display.c` et `score_panel.c`, D.15c ajoute **une** ligne `#include "pico8_str.h"` dans le bloc des includes (au-dessus de tout site de D.16a) et n'édite qu'en place ses sites `tostr` (`economy_score.c:223`, `planet_display.c:269,275,276,377`, `score_panel.c:200,216,221`) : tous les numéros D.16a de ces trois fichiers font **+ 1** ; `economy_score.c` vaut **2 / 2** (ses seules lignes `valeur` : `economy_truthy`, `:17-18` → `:18-19`), `planet_display.c` **41 / 43**, `score_panel.c` **22 / 26** | colonne « après D.15c » de l'Inventaire, mutations M9–M14, comptes par fichier, sortie d'`economy_score.c` du résidu D | `git diff 4171b9b <HEAD gradé> -- <ces trois fichiers>` ; `grep -n` par contenu |
| H-D15c-3 | D.15c ne touche ni `planet.c/.h`, ni `planet_family.c/.h`, ni `economy_tiers.c`, ni `pico8_game_data.c/.h`, ni aucun des 11 TU de l'étape (`fr`, `mx`, `os`, `ga`, `pm`, `nr`, `jo`, `hj`, `nl`, `qh`, `jo_spec_defaults`), ni `economy_score.h`, ni `docs/glossaire-noms.md:247,252,359`, ni TROU-3 | tous les numéros non marqués de l'Inventaire ; « Documentation » | `git diff --stat 4171b9b <HEAD gradé> -- <ces chemins>` vide |
| H-D15c-4 | D.15c ne retire aucune copie `*_truthy` et ne touche ni `pico8_jo_gg`, ni `pico8_jo_hf`, ni `pico8_qh_qh` ; il reste **11** copies (`economy_score`, `economy_tiers`, `layout`, `planet`, `planet_display`, `probe`, `research`, `research_display`, `tech_gate`, `ui_button`, `wormhole_display`) | « Copies `truthy` », grep 3 de la Sortie, note | `grep -nE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c` |
| H-D15c-5 | D.15c ne change dans le `Makefile` que les prérequis de `test_pico8_value` et `test_pico8_str` (`:407`, `:414`, édités en place) : `DATA_ENGINE_SRCS` (`:229-279`) et les règles `:417-553` inchangées ; `tools/class_coverage_net_expected.txt` inchangé ; aucun TU ajouté ; `make e2e` 26 conformes | « `Makefile` et liens », grep 6, `class-coverage-net` | `git diff 4171b9b <HEAD gradé> -- Makefile tools` |
| H-D15c-6 | La note « Découvert à D.15c » ne modifie pas la ligne D.16a du plan, ne déplace aucun site B1 vers D.15c (ni l'inverse) | tout le PRD | lecture du plan gradé |
| H-D15c-7 | D.15c est entière à la graduation (un commit, ou c1 + c2 tous deux commis) | tout | note de D.15c |

### Sections à revérifier quand D.15c sera gradée

- **Toujours** : « État de départ » (comptes réels, H-D15c-1) ; critère `FAILED:` de départ (HEAD, comptes) ; « Effet sur `make data-residue` » (colonnes départ et après, table par fichier) ; Runs 1-2 ; les numéros de `economy_score.c`, `planet_display.c`, `score_panel.c` dans l'Inventaire, les mutations et leurs cibles (**remesure par contenu**) ; Annexe A (recopier la ligne D.15c et sa note).
- **Si H-D15c-2 tombe** (include placé autrement, autre ligne touchée, ou `economy_score.c` ≠ 2 / 2) : Inventaire de ces fichiers ; M9–M14 ; « Includes » (si D.15c a laissé ou ajouté un include de `pico8_value.h` dans `economy_score.c`, il part ici : la règle `.c` de D.15b) ; sortie d'`economy_score.c` de `valeur` et du résidu D ; comptes.
- **Si H-D15c-3 tombe** : Inventaire et comptes du fichier touché ; « Les TU suivent » si c'est un TU.
- **Si H-D15c-4 tombe** : « Copies `truthy` », grep 3, total de l'étape, note.
- **Si H-D15c-5 tombe** : « `Makefile` et liens », grep 6, filets.
- **Si H-D15c-6 ou -7 tombe** : ne pas mettre ce PRD en file ; le reprendre.

### Preuve B1, par le cart et par le port

**Écritures du cart** (`grep -nE 'hf\s*=|qh\s*=' pico8/lua.lua`, six lignes, toutes dans des spécifications de classe passées à `jr`) :

| Champ | Ligne | Classe | Valeur |
|---|---|---|---|
| `hf` | 387 `hf=1,hs=1,` | `jo` | `1` |
| `hf` | 909 `hf=f,qh=1,` | `qh` | `false` |
| `hf` | 1070 `hf=f,` | `ga` | `false` |
| `qh` | 909 | `qh` | `1` (hérité par `nr`, `fr`, l. 1101 sans `qh=`) |
| `qh` | 1001 `hp=2,qh=f,` | `hj` | `false` (hérité par `pm`, `mx`) |
| `qh` | 1059 `…,qh=f,nl=1,` | `nl` | `false` |
| `qh` | 1098 `qh=f,s=100,` | `os` | `false` |

Le jeton `f` d'une spécification est `false` : `ob()` écrit `sc!="f" and band(…)` (l. 66). Aucune écriture dynamique ne peut atteindre ces clés : `qj(o,mr)` (l. 10-12) et le constructeur `__call` (`mw(ob)`, l. 88) ne copient que des tables écrites en littéral, et aucun littéral du cart ne porte `hf=` ou `qh=` hors des six lignes ; `ui:bh` (l. 240, 245) n'écrit que `lheld`/`rheld`. **Donc `hf` et `qh` ne valent jamais que `1` ou `false`** ; nil seulement pour `qh` sur une planète qui n'est pas de la lignée `qh` (champ absent).

**Lecteurs du cart** (`grep -nE '\bhf\b|\.qh\b|gg' pico8/lua.lua`) :

| Ligne | Lecture | Forme |
|---|---|---|
| 437 | `self.qh and {…} or self.jy` (`jo:ib`, liste « wants more ») | vérité |
| 450 | `return self.hf and self.me=="ly"` (`jo:gg`) | vérité ; rend `false` (de `hf`) ou un booléen |
| 546 | `if self:gg()then` (`jo:kj`) | vérité |
| 1173, 1320 | `jp(cg.jo,jo.gg)` (`eg`, `cz`) ; `jp` : `if (ow(e))add(f,e)` (l. 31-36) | vérité |
| 1306 | `if not pt.hf then` (`dh`) | vérité |
| 1313-1315 | `pt.qh and #cg.qh or pt:gg() and es[pt.mi] or 0` (`da`) | vérité |
| 1329 | `if oz.hf then` (`db`) | vérité |
| 1342, 1371 | `if pt:gg()then` (`ky`, `bf`) | vérité |

**Aucun lecteur ne compare `hf`, `qh` ou `gg()` par `==`/`~=`, ne les passe à `tostr` ni ne les range** : nil et `false` ne sont distingués nulle part, le nombre `1` n'est jamais lu comme nombre (G2 : B1 confirmé). `gg()` ne rend jamais un nombre ni nil (`false` de `hf`, sinon le booléen de `==`).

**Écritures du port** (`grep -rnE '(hf|qh)(_is_bool)? = ' src`) : les constructeurs seuls, depuis les constantes générées — `planet.c:154-155` (`jo`), `planet_family.c:79-80` (`qh`), `:95-96` (`hf` de `qh`), `:175-176` (`nl`), `:235-236` (`os`), `:325-326` (`hj`), `:615-616` (`ga`) ; constantes `pico8_game_data.c:500-501` (`jo` : `0x10000`, `FIXED`), `:521-524` (`qh` : `hf` `BOOL 0`, `qh` `0x10000`), `:536-537` (`nl`), `:551-552` (`os`), `:562-563` (`hj`), `:606-607` (`ga`). (L'étude cite `planet_family.c:103-104,619-620` : numéros d'avant D.15a ; mêmes lignes par contenu.) **États atteignables** : `hf` ∈ {`FIXED 0x10000`, `BOOL 0`}, `qh` ∈ {`FIXED 0x10000`, `BOOL 0`} — jamais `BOOL 1` ni `FIXED 0`.

**Lecteurs du port** — tous en vérité seule, aucun ne distingue nil de `false` :

| Site (à `4171b9b`) | Lecture | NULL possible ? |
|---|---|---|
| `economy_score.c:47` (`call_gg_truthy`) | `economy_truthy(pico8_jo_gg(pt))` | non : `pt == NULL → 0` en `:43-44`, puis l'`assert(pico8_jo_is_family(pt))` |
| `economy_score.c:53` (`da`) | `qh != NULL && economy_truthy(pico8_qh_qh(qh))` | `qh` NULL (pas de la lignée) → faux, comme le nil du cart |
| `economy_score.c:116` (`db`) | `economy_truthy(pico8_jo_hf(oz))` | non : `oz_val == NULL → continue` (`:110`) |
| `economy_tiers.c:293` (`dh`) | `!economy_truthy(pico8_jo_hf(jo))` | non : `pt == NULL → return 0` (`:225-226`) |
| `planet_display.c:227` (`jo:gh`, « wants more ») | `qh != NULL && pico8_jo_truthy(pico8_qh_qh(qh))` | `qh` NULL → faux |
| `planet_display.c:472` (`jo:kj`) | `pico8_jo_truthy(pico8_jo_gg(self))` | non : `self == NULL → return` en tête de `jo:kj` |
| `score_panel.c:111-126` (`eg`, prédicat) | `gg` rejeté si `BOOL 0` ou `REF NULL` | `e == NULL → 0` (`:114-116`) |

Les deux `economy_truthy` liraient un `REF NULL` comme **vrai** (B12) : c'est sans effet, puisqu'aucun de leurs appelants n'atteint `pico8_jo_hf`/`pico8_jo_gg` avec un `self` NULL (colonne de droite ; B12 : « le PRD vérifie qu'aucun appelant ne passe un `self` NULL » — fait). Sur les états atteignables, `economy_truthy`, `pico8_jo_truthy` et le prédicat de `eg` donnent la même vérité que l'`int` : `FIXED 0x10000` → 1, `BOOL 0` → 0, et le `BOOL` de `gg` → sa valeur.

**Conclusion : aucun champ ne sort de B1.** Rien n'est déplacé vers une autre étape.

### Lecture de la ligne pour D.16a

- **« valeurs vraies/fausses devenues entiers »** : **Tranché (G)** — `int hf;` dans `pico8_jo_t`, `int qh;` dans `pico8_qh_t`, **mêmes noms** ; `hf_is_bool`, `qh_is_bool` retirés des structs d'entité et des six structs `pico8_*_start_t` de `pico8_game_data.h` (`jo`, `qh` ×2, `nl`, `os`, `hj`, `ga`). Les constantes deviennent `1` (le `1` du cart) et `0` (son `false`) : `jo.hf = 1`, `qh.hf = 0`, `qh.qh = 1`, `nl.qh = os.qh = hj.qh = 0`, `ga.hf = 0`. Les constructeurs gardent leurs lignes `self->hf = pico8_X_start.hf;` / `…qh = …qh;` telles quelles et perdent leurs sept lignes `_is_bool`. Rejeté : garder `pico8_fixed_t` en le lisant `!= 0` (un nombre que personne ne lit, et `FIXED 0` redeviendrait un état écrivable et faux, alors que le `0` du cart est vrai) ; `_Bool` (le port écrit ses drapeaux en `int` : `has_*`, `*_is_bool`).
- **`pico8_jo_hf` et `pico8_qh_qh`** : la ligne et l'étude ne disent pas ce qu'elles deviennent. **Tranché (G)** : **retirées** (classe (a2), enveloppes, précédent `pf/mz_value_vec` de D.15b). Leurs quatre lecteurs lisent le membre : `oz->hf`, `!jo->hf`, `qh != NULL && qh->qh` (×2). Raison : une fois le membre `int`, l'accesseur ne fait plus que le relire ; sa garde `self == NULL` ne sert à aucun appelant (preuve ci-dessus). Rejeté : `int pico8_jo_hf(const pico8_jo_t *)` et `int pico8_qh_qh(const pico8_qh_t *)` (deux API sans raison d'être, gardées « au cas où »).
- **`pico8_jo_gg`** : `int pico8_jo_gg(const pico8_jo_t *self)`, 1 si `hf` est vrai et `me` vaut `"ly"`, 0 sinon, **0 pour `self` NULL** (règle de l'étude : « même vérité que nil »). Corps écrit ligne pour ligne sur l'ancien (`if (self == NULL) return 0;`, `if (!self->hf) return 0;`, `return self->ke.me != NULL && strcmp(self->ke.me, "ly") == 0;`), **sans joindre** les deux tests. Le `/* TROU-3 */` de `planet.h:139` **reste** (la comparaison par contenu de `me` est l'objet de TROU-3) ; le contrat `:140-141` (« deliberately not collapsed to a C int ») devient faux et est **réécrit** : c'est le renversement voulu par l'étude, déclaré.
- **« copies `truthy` de leurs appelants »** : les copies dont **tous** les appelants partent avec B1 deviennent des `static` inutilisées, fatales sous `-Werror` : `pico8_jo_truthy` de `planet.c:36-44` (seul appelant `:365`), `pico8_jo_truthy` de `planet_display.c:25-34` avec son commentaire (appelants `:227`, `:472`), `economy_truthy` de `economy_score.c:17-19` (appelants `:47`, `:53`, `:116`), `economy_truthy` de `economy_tiers.c:16-18` (seul appelant `:293`). **Quatre copies partent ; il en reste 7** (`layout`, `probe`, `research`, `research_display`, `tech_gate`, `ui_button`, `wormhole_display`). Le prédicat en ligne de `score_panel.c` (`:112`, `:119-124`) devient `return pico8_jo_gg(e);`. `economy_value_equal_content` (`economy_tiers.c:20-25`, B2) **reste** (D.17).
- **« TU : plis de `jo_gg` (…), `spec_defaults` »** : mesuré, l'empreinte TU est plus large que la ligne de l'étude (surprise, déclarée) : **11 TU**, `qh` en plus (voir « Les TU suivent »). Les aides `fold_hf`/`fold_qh` des dix TU de la famille planète lisent `hf_is_bool`/`qh_is_bool` : elles suivent, hors motif.
- **`fold_value` retirée de cinq TU (surprise, déclarée)** : dans `mx`, `os`, `pm`, `hj`, `nl`, le pli de `gg` est le **seul** appel de `fold_value` (mesuré : 1 appel chacun ; `fr` 7, `nr` 7, `jo` 3, `ga` 10, `qh` 0 depuis D.15b). Le pli typé la laisse sans appel, fatale sous `-Werror` (`-Wunused-function`, `CFLAGS` = `-std=c99 -Wall -Wextra -Werror -g`, `Makefile:97`) : **elle part**, 5 lignes `valeur` chacune (la signature et les quatre `case PICO8_VALUE_*`), **25 lignes, 25 occ.** C'est le précédent exact de D.15b (`test_pico8_qh`). `fold_ref` et `fnv_str`, qu'elle appelait, **restent utilisées** (plis de `d_`/`et` et `fold_string_member`, vérifié dans les cinq TU) : rien d'autre ne tombe.
- **« Compte : ≈ 845 »** : estimation de l'étude (B1 18 + copies). Mesuré à `4171b9b` : **54 lignes, 55 occ. retirées**, aucune gagnée : `src/` 28 / 28, `tests/` 26 / 27. Sous H-D15c-1 : `valeur` **879 → 825** (971 → 916 occ.). L'écart à ≈ 845 : les 25 lignes de `fold_value` (forcées par `-Werror`) et les 10 lignes des quatre copies `truthy`, que l'étude range en B12 sans les compter dans B1. « ≈ » n'est pas un critère ; « − 54 lignes, − 55 occ., exact, écart expliqué » l'est.
- **« `class-coverage-net` : oui »** : le diff touche sept constructeurs (`jo`, `qh`, `nl`, `os`, `hj`, `ga` et le `hf` de `qh`). `make class-coverage-net` exigé, **identique** à `tools/class_coverage_net_expected.txt`. `sanitize-net` : **non** (aucune durée de vie ni propriétaire ne change : deux `int` remplacent deux paires de membres, aucune allocation touchée) ; le balayage ASan+UBSan des TU est exigé (consigne de l'utilisateur).
- **§6** : aucun des trois tests de régression (`layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`) ne nomme un symbole de l'étape ; `planet_gh_row_lq` et `ga_export_panel_by` passent par `jo:gh` (`planet_display.c:227`) : recompilés, relancés, rien de plus.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes comptées, alias, renommage pour échapper aux greps, cast nouveau, cale valeur ↔ typé ; mutations restaurées par `cp` simple puis `touch` ; binaires lancés exactement comme le `Makefile` ; **jamais l'appli raylib**.

### État de départ

| Mesure | À `4171b9b` (D.15c non commencée) | Départ attendu de D.16a (D.15c gradée, H-D15c-1) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 906 / 101 ; 1025 occ. (`src/` 436 / 63 / 508, `tests/` 470 / 38 / 517) | **879 / 99 ; 971 occ.** (`src/` 415 / 61 / 466, `tests/` 464 / 38 / 505) |
| `type-tag` | 243 / 58 | 243 / 58 |
| résidu D | 1147 / 110 | **1120 / 108** |
| prototypes publics qui nomment `pico8_value_t` | 20 | 20 |
| copies `*_truthy(pico8_value_t)` | 11 | 11 |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |

Recette des occurrences (celle de D.12–D.15c, motif `valeur` de `tools/data_residue.sh:29`) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'valeur\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire (numéros à `4171b9b` ; « → » : numéro attendu après D.15c, H-D15c-2)

Classes du message de commit : **(a1)** même site, type C à la place de la valeur ; **(a2)** retirée sans remplaçant (enveloppe, copie `truthy`, local, aide de TU devenue sans appel) ; **(b)** signature publique dont le type change (table « API ») ; **(c)** même donnée, autre nom : **attendu 0**. `occ.` : occurrences du motif sur la ligne.

#### API

| Symbole | Aujourd'hui | Après D.16a | Appelants (`src/`) | TU |
|---|---|---|---|---|
| `pico8_jo_gg` (`planet.c:358`, `planet.h:142`) | rend `pico8_value_t` (`BOOL`, ou `REF NULL` sur `self` NULL) | **`int`** (1 / 0 ; 0 sur `self` NULL) — classe (b) | `economy_score.c:47`, `planet_display.c:472`, `score_panel.c:118` | plis de `fr:415`, `mx:483`, `os:454`, `ga:595`, `pm:498`, `nr:424`, `jo:367`, `hj:493`, `nl:431` ; `ga:596` |
| `pico8_jo_hf` (`planet.c:348`, `planet.h:137`) | rend `pico8_value_t` | **retirée** — (a2) | `economy_score.c:116`, `economy_tiers.c:293` → `oz->hf`, `!jo->hf` | aucun |
| `pico8_qh_qh` (`planet_family.c:30`, `planet_family.h:46`) | rend `pico8_value_t` | **retirée** — (a2) | `economy_score.c:53`, `planet_display.c:227` → `qh->qh` | aucun |

Membres : `pico8_jo_t.hf` (`planet.h:29-30`) et `pico8_qh_t.qh` (`planet_family.h:36-37`) : `pico8_fixed_t x; int x_is_bool; /* … */` → `int x;`. Structs de données (`pico8_game_data.h`) : `pico8_jo_start_t` `:276-277`, `pico8_qh_start_t` `:297-300`, `pico8_nl_start_t` `:312-313`, `pico8_os_start_t` `:327-328`, `pico8_hj_start_t` `:338-339`, `pico8_ga_start_t` `:382-383`, même changement. Prototypes publics qui nomment `pico8_value_t` : **20 → 17**. Aucune autre signature publique ne change.

#### Sites de `src/` (28 lignes, 28 occ. retirées ; aucune gagnée)

| Fichier | Ligne(s) `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `planet.c` | `:36`, `:37`, `:40` (1 chacune) | `pico8_jo_truthy` retirée entière (`:36-44`) : son seul appelant `:365` part | (a2) |
| | `:348`, `:350`, `:353`, `:355` (1 chacune) | `pico8_jo_hf` retirée entière (`:348-356`) | (a2) |
| | `:358` (1) | `int pico8_jo_gg(const pico8_jo_t *self) {` | (b) |
| | `:359` (1) | local `hf_val` retiré | (a2) |
| | `:362` (1) | `return 0;` | (a1) |
| | `:368` (1) | `return self->ke.me != NULL && strcmp(self->ke.me, "ly") == 0;` | (a1) |
| | `:364-366` (0) | `if (!self->hf) {` / `return 0;` (hors motif ; `:364` `hf_val = pico8_jo_hf(self);` part) | — |
| | `:155` (0) | `self->hf_is_bool = …;` retirée (constructeur `jo`, hors motif) | — |
| `planet.h` | `:137` (1) | prototype de `pico8_jo_hf` retiré, avec son contrat `:136` | (a2) |
| | `:142` (1) | `int pico8_jo_gg(const pico8_jo_t *self);` ; contrat `:140-141` réécrit (« 1 when `hf` is true and `me` is "ly", 0 otherwise, a NULL `self` included ») ; `/* TROU-3 */` `:139` gardé | (b) |
| | `:29-30` (0) | `int hf;` ; le commentaire de `hf_is_bool` part avec lui | — |
| `planet_family.c` | `:30`, `:32`, `:35`, `:37` (1 chacune) | `pico8_qh_qh` retirée entière (`:30-38`) | (a2) |
| | `:80`, `:96`, `:176`, `:236`, `:326`, `:616` (0) | lignes `_is_bool` des constructeurs `qh`, `qh` (`hf`), `nl`, `os`, `hj`, `ga` retirées (hors motif) | — |
| `planet_family.h` | `:46` (1) | prototype de `pico8_qh_qh` retiré, avec son contrat `:44-45` | (a2) |
| | `:36-37` (0) | `int qh;` | — |
| `score_panel.c` | `:112` → `:113` (1) | local `gg` retiré | (a2) |
| | `:119` → `:120`, `:122` → `:123` (1 chacune) | les deux tests de genre partent (`:119-124` → `:120-125`) avec `return 1;` ; `:118` → `:119` devient `return pico8_jo_gg(e);` (hors motif) | (a2) |
| `planet_display.c` | `:26` → `:27`, `:27` → `:28`, `:30` → `:31` (1 chacune) | `pico8_jo_truthy` retirée entière avec son commentaire (`:25-34` → `:26-35`) | (a2) |
| | `:227` → `:228` (0) | `int last_only = qh != NULL && qh->qh;` (hors motif) | — |
| | `:472` → `:473` (0) | `if (pico8_jo_gg(self)) {` (hors motif) | — |
| `economy_score.c` | `:17` → `:18`, `:18` → `:19` (1 chacune) | `economy_truthy` retirée entière (`:17-19` → `:18-20`) | (a2) |
| | `:47` → `:48` (0) | `return pico8_jo_gg((const pico8_jo_t *)pt);` (conversion déjà présente, non nouvelle) | — |
| | `:53` → `:54` (0) | `if (qh != NULL && qh->qh) {` | — |
| | `:116` → `:117` (0) | `if (oz->hf) {` | — |
| `economy_tiers.c` | `:16`, `:17` (1 chacune) | `economy_truthy` retirée entière (`:16-18`) | (a2) |
| | `:293` (0) | `if (!jo->hf) {` | — |
| `pico8_game_data.h` | — (0) | six structs `_start_t` : `int hf;` / `int qh;`, `_is_bool` retirés (11 lignes) | — |
| `pico8_game_data.c` | — (0) | `:500` `.hf = 1,` ; `:521` `.hf = 0,` ; `:523` `.qh = 1,` ; `:536`, `:551`, `:562` `.qh = 0,` ; `:606` `.hf = 0,` ; les sept `.x_is_bool = …,` retirées | — |

Totaux `src/` (lignes / occ. retirées) : `planet.c` 11 / 11, `planet.h` 2 / 2, `planet_family.c` 4 / 4, `planet_family.h` 1 / 1, `score_panel.c` 3 / 3, `planet_display.c` 3 / 3, `economy_score.c` 2 / 2, `economy_tiers.c` 2 / 2 : **28 lignes, 28 occ.**

#### TU (26 lignes, 27 occ. retirées ; aucune gagnée)

| TU | Ligne(s) `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `mx` | `:130`, `:132`, `:134`, `:136`, `:138` (1 chacune) | `fold_value` retirée entière (`:130-143`) : plus d'appel | (a2) |
| `os` | `:139`, `:141`, `:143`, `:145`, `:147` | idem (`:139-152`) | (a2) |
| `pm` | `:126`, `:128`, `:130`, `:132`, `:134` | idem (`:126-139`) | (a2) |
| `hj` | `:141`, `:143`, `:145`, `:147`, `:149` | idem (`:141-154`) | (a2) |
| `nl` | `:125`, `:127`, `:129`, `:131`, `:133` | idem (`:125-138`) | (a2) |
| `ga` | `:596` (2) | `assert(pico8_jo_gg(a) == 0);` | (a1) |

Lignes modifiées **hors motif**, dans le même commit (elles ne comptent pas mais doivent suivre, sans quoi rien ne compile) :

| TU | Lignes | Nouveau texte |
|---|---|---|
| `fr` `:415`, `mx` `:483`, `os` `:454`, `ga` `:595`, `pm` `:498`, `nr` `:424`, `jo` `:367`, `hj` `:493`, `nl` `:431` | pli de `gg` | `h = fnv(fnv(h, 0xB0u), pico8_jo_gg(e) ? 1u : 0u);` (`b`, `jo` selon le TU) — le pli `BOOL` de `fold_value` (`fnv(fnv(h, 0xB0u), v)`) ; `gg` n'y reçoit jamais `self` NULL (le `REF NULL` → `0xDEAD` ne se produisait pas) |
| `fold_hf` : `fr:108`, `mx:109`, `os:109`, `ga:106`, `pm:105`, `nr:106`, `jo:107`, `hj:111`, `nl:104`, `qh:111` | pli du membre | `return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);` |
| `fold_qh` : `fr:156`, `mx:163`, `os:178`, `pm:159`, `nr:154`, `hj:174`, `nl:164`, `qh:115` | pli du membre | `return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);` |
| `ga:431` | `assert(as_ga(a)->jo.hf == 0);` | |
| `os:487` | `assert(oz->hf);` | |
| `hj:575` | `assert(hj->qh.qh == 0);` | |
| `jo_spec_defaults` `:58`, `:68`, `:86`, `:105`, `:130`, `:165`, `:200`, `:219`, `:253`, `:308` | la moitié `&& x_is_bool == pico8_X_start.x_is_bool` part ; la comparaison du membre reste | |

**Pourquoi ces plis reproduisent les octets** (G1) : aujourd'hui, `fold_hf` plie `fold_fixed_member(h, 0x10000)` quand `hf_is_bool == 0` (seul état : `jo`, `hf = 0x10000`) et `fold_bool_member(h, 0)` quand `hf_is_bool == 1` (seul état : `qh`/`ga`, `hf = 0`) ; idem `fold_qh`. Les dix TU ont le même corps de `fold_bool_member` (`fnv(fnv(fnv(h, 1u), 0xB0u), (uint32_t)(b != 0))`) et de `fold_fixed_member` (`…0xF1u), (uint32_t)v)`), vérifié par grep. Le nouveau pli choisit la branche par l'`int` et écrit le même octet de genre et le même contenu. Le pli de `gg` écrivait `fnv(fnv(h, 0xB0u), b)` pour le `BOOL` rendu (toujours un `BOOL` sur un `self` non NULL : le `false` de `hf` est un `BOOL 0`, jamais un `FIXED`).

**Pourquoi `? 1u : 0u` et pas `(uint32_t)pico8_jo_gg(e)`** : aucun cast nouveau ; et une mutation qui ferait rendre 2 à `gg` ne passerait pas pour un pli juste. Rejeté aussi : `fold_value(h, pico8_value_bool(pico8_jo_gg(e)))` (neuf lignes gagneraient `valeur`, cale valeur ↔ typé) ; une aide `fold_gg` par TU (neuf aides pour neuf appels).

Totaux `tests/` : `mx` 5 / 5, `os` 5 / 5, `pm` 5 / 5, `hj` 5 / 5, `nl` 5 / 5, `ga` 1 / 2 : **26 lignes, 27 occ.** `fr`, `nr`, `jo`, `qh`, `jo_spec_defaults` : aucune ligne `valeur` touchée.

#### Includes (G, règle de D.15b)

Un **`.c`** qui ne nomme plus rien du module perd son `#include "pico8_value.h"` ; un en-tête le garde jusqu'à D.19c. Appliqué : **aucun include touché**. `planet_family.c` et `economy_score.c` se vident de `valeur` **sans** include à retirer (ils n'en ont pas : le type leur vient de `planet_family.h:11` et `planet.h:13`, gardés) ; `planet.h` et `planet_family.h` gardent leur include (en-têtes ; `planet.h` garde en plus ses lignes nommées par d'autres étapes). Aucun TU ne se vide (`mx` 7, `os` 4, `pm` 6, `hj` 8, `nl` 4, `ga` 30 lignes restantes). Aucun include ajouté ; si la compilation en exige un, il est ajouté et déclaré avec l'erreur du compilateur.

#### `Makefile` et liens

- Aucun fichier nouveau ; **aucune ligne du `Makefile` ne change**.
- Règle de lien : aucun fichier de `DATA_ENGINE_SRCS` (`Makefile:229-279`) ne nomme un symbole défini hors de cette liste. D.16a **n'ajoute aucun symbole** et en **retire deux** (`pico8_jo_hf`, `pico8_qh_qh`) : grep 1 prouve qu'aucun appelant ne reste, dans aucun fichier. `score_panel.c` (hors liste) appelle `pico8_jo_gg` (`planet.c`, dans la liste) : permis. Les binaires liés sur `DATA_ENGINE_SRCS` seule (`data_init`, `map_tables`, `jb_ol`, `planet_kj_dispatch`, `bg_kj`), sous ASan (`planet_gh_row_lq`, `Makefile:550`) ou avec `planet_family.c` en plus (`ga_export_panel_by`, `:553`) doivent se lier (grep 6).

## Solution

**Un commit** (22 chemins : 10 de `src/`, 11 TU, `docs/plan-3.md` pour la note), vert seul, vérifié et poussé :

1. `hf` et `qh` deviennent des `int` vrai/faux dans les structs d'entité et les structs de données ; `hf_is_bool`, `qh_is_bool` disparaissent ; les constantes valent 1 ou 0.
2. `pico8_jo_gg` rend `int` ; `pico8_jo_hf` et `pico8_qh_qh` disparaissent ; leurs lecteurs lisent le membre.
3. Les quatre copies `truthy` qui n'avaient plus que ces valeurs pour argument disparaissent ; le prédicat de `eg` rend `gg`.
4. Les onze TU suivent : plis de `gg`, `fold_hf`, `fold_qh` aux mêmes octets ; `fold_value` retirée des cinq TU où elle n'a plus d'appel ; assertions de `ga`, `os`, `hj`, `jo_spec_defaults` sur le membre seul. Aucune constante ne change.

**Sortie de l'étape**, constatée par commande sur le commit :

- `make test` vert (dont `make data-guard`, `constants-check`, `float-grep`, `float-guard`) ; `make e2e` : **26 conformes** ; `make class-coverage-net` : **identique** à `tools/class_coverage_net_expected.txt` ; balayage ASan+UBSan des TU propre ;
- `make data-residue` : `valeur` **879 → 825** lignes (**971 → 916** occ.), fichiers **99 → 97** (`planet_family.c` sort de `valeur`, reste au résidu D par `type-tag` ; `economy_score.c` sort de `valeur` **et** du résidu D) ; `type-tag` 243 / 58 inchangé ; résidu D **1120 / 108 → 1066 / 107** (sous H-D15c-1 ; sinon départ − 54 / départ − 1) ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/`, rien dans `tools/` (`float_guard.txt` réécrit à l'identique par `make test`), rien dans le `Makefile` ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. drapeaux de genre et enveloppes, nulle part : vide
  grep -rnE 'hf_is_bool|qh_is_bool|pico8_jo_hf|pico8_qh_qh' src tests
  # 2. gg typé : une ligne, 'int pico8_jo_gg(const pico8_jo_t *self);'
  grep -nE 'pico8_jo_gg\(' src/*.h
  # 3. copies truthy restantes : 7 fichiers (layout, probe, research, research_display,
  #    tech_gate, ui_button, wormhole_display) ; aucune dans planet, planet_display, economy_*
  grep -lE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c
  # 4. plus aucun pli de gg par la valeur : vide ; fold_value absente de mx, os, pm, hj, nl
  grep -nE 'fold_value\(h, pico8_jo_gg' tests/*.c
  grep -c 'static uint32_t fold_value' tests/test_pico8_mx.c tests/test_pico8_os.c \
       tests/test_pico8_pm.c tests/test_pico8_hj.c tests/test_pico8_nl.c        # 0 partout
  # 5. constantes de données : .hf = 1 (jo), .hf = 0 (qh, ga), .qh = 1 (qh), .qh = 0 (nl, os, hj)
  grep -nE '^\s*\.(hf|qh) = ' src/pico8_game_data.c
  # 6. règle de lien
  make tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_bg_kj tmp/test_pico8_planet_gh_row_lq \
       tmp/test_pico8_ga_export_panel_by
  # 7. aucune constante de TU changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 8. aucun include de pico8_value.h touché : vide
  git diff <parent> | grep -E '^[-+]\s*#include "(\.\./src/)?pico8_value\.h"'
  # 9. les deux fichiers vidés ne nomment plus le module : 0, 0
  grep -cE "$P" src/pico8_planet_family.c src/pico8_economy_score.c
  # 10. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^valeur'
  # 11. filet de classes
  make class-coverage-net
  ```

## User Stories

1. En tant que mainteneur du port, je veux que `hf` d'une planète soit un entier vrai/faux, afin que son type dise ce que le cart en lit : sa vérité, jamais un nombre.
2. En tant que mainteneur du port, je veux que `qh` d'un laboratoire soit un entier vrai/faux, afin que `da` et la liste « wants more » le lisent sans déballer une valeur.
3. En tant que mainteneur du port, je veux que `pico8_jo_gg` rende 1 ou 0, afin que `jo:kj`, `eg`, `cz`, `ky`, `bf` et `da` testent une vérité C.
4. En tant que mainteneur du port, je veux que `gg` sur une planète NULL rende 0, afin que l'absence se lise comme le nil du cart.
5. En tant que mainteneur du port, je veux que `pico8_jo_hf` et `pico8_qh_qh` disparaissent, afin qu'aucun accesseur ne subsiste pour relire un entier.
6. En tant que mainteneur du port, je veux que les drapeaux `hf_is_bool`/`qh_is_bool` disparaissent des structs et des données générées, afin qu'aucun état « vrai booléen » ou « nombre 0 », jamais écrit par le cart, ne reste représentable.
7. En tant que mainteneur du port, je veux que les quatre copies privées de `*_truthy` qui n'avaient plus que ces valeurs pour argument disparaissent, afin que chaque lecteur lise la vérité du type.
8. En tant que mainteneur du port, je veux que le contrat de `gg` qui refusait « deliberately » l'entier soit réécrit, et que le `TROU-3` de sa comparaison par contenu reste, afin que les commentaires disent le code.
9. En tant que gardien des tests, je veux que les neuf TU qui plient `gg` le plient aux mêmes octets, afin qu'aucune constante ne change et qu'aucune ligne ne gagne `valeur`.
10. En tant que gardien des tests, je veux que `fold_hf` et `fold_qh` des dix TU de la famille planète écrivent le même octet de genre et le même contenu à partir de l'entier, afin que les empreintes figées prouvent encore la lecture du membre.
11. En tant que gardien des tests, je veux que `fold_value`, devenue sans appel dans cinq TU, soit retirée plutôt que maintenue en vie par un pli artificiel, afin que `-Werror` passe sans cale.
12. En tant que gardien des tests, je veux que `jo_spec_defaults` compare chaque membre à sa constante, sans la moitié de drapeau disparue, afin que le test garde son intention.
13. En tant que gardien des filets, je veux `make test`, `make e2e`, `make class-coverage-net` identique et le balayage ASan+UBSan des TU verts, afin de prouver qu'aucun constructeur, aucun pixel, aucun score ni aucune lecture mémoire ne change.
14. En tant que gardien des tests, je veux `gcov` avant/après et des mutations ciblées restaurées par `cp`, afin que chaque ligne modifiée soit exécutée et qu'aucune ligne survivante ne cesse de l'être.
15. En tant que gardien du compteur, je veux au message du commit les comptes par catégorie et par fichier en lignes et en occurrences, la classification (a1)/(a2)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris) et chaque Δlignes > Δocc justifié, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
16. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe (les deux tests de `gg`), qu'aucun alias, renommage d'évitement, cast nouveau ni cale ne soit ajouté, et qu'aucun include ne soit touché, afin que « − 54 » veuille dire « retiré ».
17. En tant que `plan-pilot`, je veux une étape d'environ vingt minutes, un repli écrit, et une note « Découvert à D.16a » qui dise les comptes réels, l'écart à ≈ 845 et ce que D.16b, D.19a et D.19c trouvent, afin que la suite parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)** ; les rejets sont notés à chaque décision. Q12 (oui) et Q13 (A) sont des décisions humaines ; l'étude D.14 (G1–G7) fait foi sauf écart déclaré ici.

### Modules

- **Planète** : `pico8_planet.c/.h` (membre `hf`, `gg`, constructeur `jo`, copie `truthy`).
- **Famille planète** : `pico8_planet_family.c/.h` (membre `qh`, constructeurs `qh`, `nl`, `os`, `hj`, `ga`).
- **Données générées** : `pico8_game_data.c/.h` (six structs `_start_t` et leurs constantes).
- **Lecteurs** : `pico8_economy_score.c` (`da`, `db`, `call_gg_truthy`), `pico8_economy_tiers.c` (`dh`), `pico8_planet_display.c` (`jo:gh`, `jo:kj`), `pico8_score_panel.c` (`eg`).
- **TU** : `fr`, `mx`, `os`, `ga`, `pm`, `nr`, `jo`, `hj`, `nl`, `qh`, `jo_spec_defaults` (11). **Docs** : `plan-3.md` (note).
- Inchangés : `Makefile`, `tools/`, `pico8_value.c/.h`, `pico8_type_tag.h`, `pico8_economy_score.h` (son commentaire sur `jo.gg` reste vrai), tout en-tête hors `planet.h`, `planet_family.h`, `game_data.h` ; les trois §6 (recompilés, relancés).

### Représentation (G)

- `int hf;` (`pico8_jo_t`), `int qh;` (`pico8_qh_t`), et `int hf;` / `int qh;` dans les structs `_start_t`. Valeurs : 1 et 0 seulement. **Pas de commentaire de membre** nouveau : le nom, le type et le contrat de `gg` suffisent ; les commentaires `/* 1: `x` holds a boolean (0/1), 0: a fixed */` partent avec leurs drapeaux.
- Aucun nom neuf : pas de `has_hf`, pas de renommage. **Piège de motif** : aucun nom ne doit correspondre à `\b[a-z0-9_]+_default_(…|hf)\b` (motif `spec`, gardé à 0 dans `DATA_RESIDUE_GUARD`) ni à `*field_truthy(` (`par-nom`).
- Constructeurs : `self->hf = pico8_jo_start.hf;` etc. **inchangés** ; seules les sept lignes `_is_bool` partent. L'ordre des affectations restantes ne bouge pas.

### `gg` (G)

```c
int pico8_jo_gg(const pico8_jo_t *self) {
    if (self == NULL) {
        return 0;
    }
    if (!self->hf) {
        return 0;
    }
    return self->ke.me != NULL && strcmp(self->ke.me, "ly") == 0;
}
```

(Forme décidée ici, calquée ligne pour ligne sur l'actuelle : le `A and B` du cart reste deux tests ; les joindre est interdit.) Contrat de `planet.h` : « 1 when `hf` is true and `me` is "ly", 0 otherwise, a NULL `self` included » précédé du `/* TROU-3 */` inchangé.

### Lecteurs (G)

- `call_gg_truthy` (`economy_score.c:42-48`) : garde sa garde NULL et son `assert`, rend `pico8_jo_gg(…)`. Le nom de la fonction reste (il ne correspond à aucun motif ; le renommer serait hors étape).
- `da` : `qh != NULL && qh->qh` ; `db` : `oz->hf` ; `dh` : `!jo->hf`.
- `jo:gh` : `int last_only = qh != NULL && qh->qh;` ; `jo:kj` : `if (pico8_jo_gg(self))`.
- `eg` (prédicat, `score_panel.c:111-126`) : `(void)ctx;`, garde `e == NULL`, `assert` d'étiquette (`type-tag`, **mot pour mot**, D.20), puis `return pico8_jo_gg(e);`.

### Les TU suivent (G, règle commune de l'étude)

- **Tranché** : chaque TU qui nomme un symbole ou un membre changé suit dans ce commit ; aucun pont.
- Plis de `gg` (neuf TU), `fold_hf` (dix), `fold_qh` (huit), assertions `ga:431`, `ga:596`, `os:487`, `hj:575`, dix lignes de `jo_spec_defaults` : table de l'Inventaire.
- `fold_value` retirée de `mx`, `os`, `pm`, `hj`, `nl` (sans appel) ; gardée dans `fr`, `nr`, `jo`, `ga` (autres appels : `gv(…).stop`, `hd`, cases).
- Commentaires des TU : `os:448`, `hj:487` (« `pt.hf` false »), `fr:278`, `os:328`… (« the 1s of … `hf` ») restent vrais : inchangés.
- **Constantes** : aucune `*_HASH`/`identity[]` ne change. Une constante qui rougit est un bug de conversion.
- **Assertion nouvelle** : admise seulement pour couvrir une ligne modifiée non exécutée ou pour faire rougir une mutation qui ne rougit rien (trou), dans un test du module, listée au message avec la ligne qu'elle couvre ; rien de lent, aucun fichier d'événements.

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la valeur est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs (les deux tests de `gg` nommément) ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours pour échapper au motif ; alias (macro ou `typedef` qui renomme `pico8_value_t` ou une de ses fonctions, `#define pico8_jo_hf(x) …`) ; cast nouveau (les plis écrivent `? 1u : 0u`) ; **cale valeur ↔ typé** (`pico8_jo_gg_value`, `pico8_value_bool(pico8_jo_gg(…))` dans un TU, `fold_value` gardée en vie par un appel artificiel) ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; toute modification d'une constante de hachage, de `tests/e2e/`, d'un `*.sha256`, de `tools/`, du `Makefile`, d'un `#include "pico8_value.h"`.
- **Classification exigée** au message, pour chaque ligne `valeur` retirée ou modifiée (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) : attendu **(a1) 3** (`planet.c:362`, `:368`, `ga:596`), **(a2) 49**, **(b) 2** (`planet.c:358`, `planet.h:142`), **(c) 0** — à `4171b9b`, recompté au parent.
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et **par fichier** du diff : lignes (`--detail`) **et** occurrences (recette).
- **Lignes qui gagnent une catégorie**, par fichier et par catégorie, « aucune » compris. Attendu : **aucune** dans aucune catégorie. Toute autre est à justifier.
- **Lignes comptées modifiées, non retirées** : attendu **aucune**.
- **Δlignes > Δoccurrences** : chaque couple justifié. Attendu : aucun (Δocc ≥ Δlignes dans chaque fichier ; `ga` 1 / 2).
- **Numéros de ligne** : ceux du **parent du commit**, jamais ceux de ce PRD sans remesure (trois fichiers décalés par D.15c, H-D15c-2).

### Empreinte et découpage (G)

| Commit | `src/` | `tests/` | autres | Chemins |
|---|---|---|---|---|
| **D.16a** | `planet.c`, `planet.h`, `planet_family.c`, `planet_family.h`, `game_data.c`, `game_data.h`, `economy_score.c`, `economy_tiers.c`, `planet_display.c`, `score_panel.c` (10) | `fr`, `mx`, `os`, `ga`, `pm`, `nr`, `jo`, `hj`, `nl`, `qh`, `jo_spec_defaults` (11) | `plan-3.md` | 22 |

(L'étude disait ≈ 22 fichiers : même nombre, pas la même liste — `qh` en plus, `economy_*` et `game_data` comptés, le plan en plus.)

- **Pourquoi un seul commit** : 22 chemins, changements mécaniques qui se tiennent (changer le type du membre casse d'un coup les dix `fold_hf`/`fold_qh`, les constructeurs et les données). Rythme mesuré : D.15a et D.15b ≈ 10 à 12 min chacune de la mise en file à la graduation. Estimation : éditions ≈ 6 min ; `gcov` départ/arrivée ≈ 4 min ; 16 mutations ≈ 5 min ; filets (`test`, `e2e`, `class-coverage-net`, balayage) ≈ 3 min ; mesures, classification, message, note ≈ 3 min. **Budget ≈ 20 min, plafond 40 min.**
- **Repli écrit** (G) : si le plafond est atteint sans commit, rien n'est commis et `plan-pilot` relance en deux commits, même PRD, chacun vert seul : **a1** — membres, données, constructeurs, `pico8_jo_hf`/`pico8_qh_qh` retirées et leurs lecteurs, copies `truthy` de `planet.c` et `economy_tiers.c`, `fold_hf`/`fold_qh`, `jo_spec_defaults`, asserts `ga:431`, `os:487`, `hj:575` (`gg` garde sa signature, sa branche fausse rend `pico8_value_bool(0)`, mêmes octets) ; **a2** — `gg` en `int`, prédicat de `eg`, copies de `planet_display.c` et `economy_score.c`, plis de `gg`, `ga:596`, `fold_value` des cinq TU. Comptes recomptés par commit. Rejeté comme plan principal : deux coûts fixes, `planet.c` et dix TU édités deux fois.

### Effet sur `make data-residue`

| Catégorie | À `4171b9b` | Départ (après D.15c, H-D15c-1) | Après D.16a | Pourquoi |
|---|---|---|---|---|
| `valeur` | 906 / 101 ; 1025 occ. | 879 / 99 ; 971 occ. | **825 / 97 ; 916 occ.** (− 54, − 55) | Inventaire : `src/` − 28 / − 28, `tests/` − 26 / − 27 |
| `valeur` `src/` | 436 / 63 ; 508 occ. | 415 / 61 ; 466 occ. | **387 / 59 ; 438 occ.** | `planet_family.c`, `economy_score.c` sortent |
| `valeur` `tests/` | 470 / 38 ; 517 occ. | 464 / 38 ; 505 occ. | **438 / 38 ; 478 occ.** | aucun TU ne se vide |
| `type-tag` | 243 / 58 | 243 / 58 | **inchangé** | aucune ligne `type-tag` touchée (l'`assert` de `eg` reste) |
| résidu D | 1147 / 110 | 1120 / 108 | **1066 / 107** | `economy_score.c` n'a aucune ligne `type-tag` ; `planet_family.c` en a une |
| `DATA_RESIDUE_GUARD` | 8 catégories | inchangé | inchangé | `valeur` ≠ 0 |

Par fichier (lignes / occ. ; départ attendu → après D.16a) : `planet.c` 13/13 → 2/2 ; `planet.h` 3/3 → 1/1 ; `planet_family.c` 4/4 → 0/0 ; `planet_family.h` 2/2 → 1/1 ; `score_panel.c` 22/26 → 19/23 ; `planet_display.c` 41/43 → 38/40 ; `economy_score.c` 2/2 → 0/0 ; `economy_tiers.c` 6/8 → 4/6 ; `test_pico8_mx` 12/12 → 7/7 ; `test_pico8_os` 9/9 → 4/4 ; `test_pico8_pm` 11/11 → 6/6 ; `test_pico8_hj` 13/13 → 8/8 ; `test_pico8_nl` 9/9 → 4/4 ; `test_pico8_ga` 31/32 → 30/30 ; `fr` 15/15, `nr` 17/17, `jo` 12/12, `qh` 5/5, `jo_spec_defaults` 0/0, `game_data.c/.h` 0/0 : inchangés.

« − 54 », « − 55 occ. », « 825 / 916 » (ou départ réel − 54 / − 55), « inchangé » sont des critères (écart = explication chiffrée au message).

### Documentation (G)

- `docs/plan-3.md` : note « Découvert à D.16a » en fin de ligne D.16a, rien d'autre. Elle dit : le commit ; les comptes réels et l'écart à ≈ 845 (`fold_value` de cinq TU, quatre copies `truthy`) ; les lectures de la ligne (`pico8_jo_hf`/`pico8_qh_qh` retirées ; `gg` NULL → 0 ; aucun champ sorti de B1) ; `economy_score.c` sorti du résidu D ; **pour D.16b** : copies `truthy` restantes 7 (liste) ; **pour D.19a** : `fold_value` déjà retirée de `mx`, `os`, `pm`, `hj`, `nl` (et `qh` depuis D.15b) ; **pour D.19c** : `planet_family.h`, `planet.h` gardent leur include.
- `docs/glossaire-noms.md` (`hf` l. 247, `gg()` l. 252, `jo:gg()` → `pico8_jo_gg` l. 359, « `hf` vrai et état `ly` ») : reste vrai, **inchangé**.
- `docs/trous-de-portage.md` : TROU-3 (« `jo:gg` » parmi les comparaisons par contenu) reste vrai, **inchangé**.
- Commentaires : quatre sortes de `CLAUDE.md` ; contrat de `gg` réécrit (`planet.h:140-141`) ; contrats de `pico8_jo_hf` (`planet.h:136`) et `pico8_qh_qh` (`planet_family.h:44-45`), commentaire de `pico8_jo_truthy` (`planet_display.c:25`) et des deux drapeaux retirés avec leur objet ; aucun renvoi à D.16a ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile`, tout `#include "pico8_value.h"` ; aucun e2e ni TU ajouté ou supprimé.
- Toute constante `*_HASH`/`identity[]` de TU ; `tools/class_coverage_net_expected.txt`.
- Le comportement : mêmes planètes comptées par `eg`, `cz`, `ky`, `bf`, `da`, mêmes `dh`/`db`, même anneau de `jo:kj`, même liste « wants more », mêmes durées de vie ; `make e2e` conforme au bit près.
- `pk`/`pk_is_bool` de `hj`/`pm`, `fp_is_bool` de la sonde (hors B1, hors motif), les sept autres copies `truthy`, `economy_value_equal_content` (B2), `pico8_rn` (B5), les `assert` d'étiquette.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : sous `-Wall -Wextra -Werror`, `pico8_jo_truthy` (`planet.c`, `planet_display.c`), `economy_truthy` (×2) et `fold_value` (`mx`, `os`, `pm`, `hj`, `nl`) deviennent fatales une fois leurs appels partis : d'où leur retrait dans ce commit. `fold_ref`, `fnv_str`, `fold_bool_member`, `fold_fixed_member` restent utilisées partout.
- **H2** : aucun autre lecteur ou écrivain de `hf`, `qh`, `hf_is_bool`, `qh_is_bool`, `pico8_jo_hf`, `pico8_qh_qh`, `pico8_jo_gg` n'existe hors ceux de l'Inventaire (grep à `4171b9b`, `src/`, `tests/`, `tools/`) ; un oublié se révèle à la compilation et suit (listé).
- **H3** : `pico8_fixed_from_int` est visible dans les dix TU qui l'écrivent dans `fold_hf`/`fold_qh` (déjà appelée dans chacun, par grep) ; les TU se lient déjà avec `pico8_fixed.c`.
- **H4** : la branche `self == NULL` de `gg` (`return 0;`) n'est exécutée par personne, avant (`:362`) comme après : justifiée au rapport `gcov`, pas dans `L`.
- **H5** : le prédicat de `eg` est exécuté par `test_pico8_pj` (`hb` → `eg` avec des planètes dans `cg("jo")`) ou, sinon, par les e2e ; `jo:gh` sur un laboratoire (`last_only` vrai) par les e2e au moins.

## Implementation Chunks

Un seul commit. Ordre de travail :

1. Mesure de départ (HEAD de graduation de D.15c : `valeur` 879 / 99 / 971 occ., `type-tag` 243 / 58, résidu D 1120 / 108 sous H-D15c-1) ; remesure par contenu des numéros ; `gcov` de départ.
2. `game_data.h/.c` ; `planet.h` (membre, prototypes, contrat) ; `planet_family.h` (membre, prototype).
3. `planet.c` (constructeur, `gg`, retrait de `pico8_jo_hf` et `pico8_jo_truthy`) ; `planet_family.c` (constructeurs, retrait de `pico8_qh_qh`).
4. Lecteurs : `economy_score.c`, `economy_tiers.c`, `planet_display.c`, `score_panel.c`.
5. TU : `fold_hf`/`fold_qh` (10), plis de `gg` (9), `fold_value` (5), asserts, `jo_spec_defaults` ; `make tmp/test_pico8_<nom>` et lancement de chacun.
6. `make test`, `make e2e`, `make class-coverage-net`, balayage ASan+UBSan ; `gcov` d'arrivée ; mutations M1–M16 ; mesure, classification ; note « Découvert à D.16a » ; **commit**.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (planètes comptées, score, anneau dessiné, liste « wants more », valeur par défaut d'une classe), pas la présence d'une ligne. D.16a change deux représentations et une signature sans changer un résultat. La preuve : les TU existants, qui plient ces membres et `gg` dans des constantes inchangées ; les 26 e2e au bit près ; `class-coverage-net` identique ; `gcov` et mutations ; le balayage ASan+UBSan.

### TU touchés (G)

1. `fr`, `mx`, `os`, `ga`, `pm`, `nr`, `jo`, `hj`, `nl`, `qh` (plis des membres ; `gg` pour neuf d'entre eux), `jo_spec_defaults` (défauts).
2. Prouvent sans suivre : `pj` (`ky` par `call_gg_truthy`, `eg` par `hb`), `planet_kj_dispatch` (`jo:kj`), `planet_gh_row_lq`, `lq`, `js`, `ga_export_panel_by` (`jo:gh`).
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change.
4. **Prior art** : plis typés aux mêmes octets (`pico8_d_(…) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL)` de D.15b ; `fold_value(h, pico8_value_bool(gv(...).stop))` de D.10–D.11) ; `fold_value` retirée de `test_pico8_qh` à D.15b pour la même raison ; TU suiveurs dans le même commit (D.10–D.15c).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`, `Makefile:118`), **les TU qui forkent** (enfants de `must_abort`, qui sortent par `_exit`) compilés en plus avec **`-D_exit=exit`** pour que l'enfant écrive ses compteurs ; lancés comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour `test_pico8_planet_gh_row_lq`, lié sous ASan) ; union des lignes de `src/` exécutées, départ et arrivée.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée, sinon justifiée au message (H4 ; déclarations). Attendus : `gg` (`if (!self->hf)` et les deux `return` : `jo` pour vrai, `os`/`hj` pour faux) ; `da` (`fr`, `nr` : `qh` vrai ; `os`, `hj`, `nl` : faux) ; `dh` (`os`, `hj` : `!jo->hf`) ; `db` (`os`, `fold_db`) ; `call_gg_truthy` (`pj`, TU `da`) ; `eg` (`pj`, sinon e2e) ; `jo:kj` (`planet_kj_dispatch`) ; `jo:gh` (`planet_gh_row_lq`, `lq`, `js` ; `last_only` vrai : e2e) ; constructeurs (tous les TU de la famille).
- **`L`** = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide**. Partantes, hors `L` : corps de `pico8_jo_truthy` (×2), `economy_truthy` (×2), `pico8_jo_hf`, `pico8_qh_qh`, les tests de genre du prédicat de `eg`, les sept lignes `_is_bool` des constructeurs, `planet.c:364` (`hf_val = …`). Une ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail : fichier sauvegardé par `cp` avant, restauré par `cp` simple puis `touch` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance ; chacune doit rougir au moins le TU nommé, sinon c'est un trou, couvert par une assertion ou justifié (mutation équivalente, démontrée). **Astuces** : retirer un appel peut échouer sous `-Werror` (fonction `static` ou variable inutilisée) : neutraliser par `0 ? f(…) : 0`, `if (0)` ou une condition toujours fausse plutôt que supprimer ; si un `assert` de `src/` attrape la mutation avant le TU (abandon), relancer une seconde fois avec cet `assert` relâché dans la copie mutée, pour montrer que le TU l'attrape aussi, et rapporter les deux.

- **M1** `game_data.c` : `jo.hf = 0` → `test_pico8_jo` (`fold_hf`, pli de `gg`) ; `jo_spec_defaults` reste vert (il compare à la constante) : attendu, rapporté ;
- **M2** `game_data.c` : `ga.hf = 1` → `test_pico8_ga` (`:431`, `fold_hf`, pli de `gg`) ;
- **M3** `game_data.c` : `qh.qh = 0` → `test_pico8_qh` (`fold_qh`), `fr`/`nr` (`fold_qh`, `da`) ;
- **M4** `game_data.c` : `hj.qh = 1` → `test_pico8_hj` (`:575`), `mx`, `pm` ;
- **M5** constructeur `qh` n'écrit plus `hf` (`if (0)` sur `self->jo.hf = pico8_qh_start.hf;` : garde le 1 de `jo`) → `test_pico8_qh`, `jo_spec_defaults` (`:58`), constantes `os`/`hj` (`gg` d'une planète « ly ») ;
- **M6** constructeur `nl` n'écrit plus `qh` (garde le 1 de `qh`) → `test_pico8_nl` (`fold_qh`, `da`), `jo_spec_defaults` (`:105`) ;
- **M7** `gg` ignore `hf` (`if (0 && !self->hf)`) → `test_pico8_os`, `test_pico8_hj` (planète « ly » à `hf` faux, `:525`, `:535`) ;
- **M8** `gg` ignore `me` (`return 1;` à la place du `strcmp`) → `test_pico8_jo` (planète nue hors « ly »), `pj` ;
- **M9** `eg` rend `!pico8_jo_gg(e)` → `test_pico8_pj`, sinon `make e2e` (drapeaux de sprites de `eg`), sinon assertion ;
- **M10** `db` lit `!oz->hf` → `test_pico8_os` (`fold_db`) ;
- **M11** `dh` lit `jo->hf` au lieu de `!jo->hf` → `test_pico8_os`, `test_pico8_hj` (`fold_economy`) ;
- **M12** `da` ignore `qh` (`qh != NULL && 0`) → `test_pico8_fr`, `test_pico8_nr` (`fold_economy`) ;
- **M13** `jo:gh` : `last_only` forcé à 0 → TU `lq`/`js`/`planet_gh_row_lq` s'ils montrent un laboratoire, sinon `make e2e`, sinon trou justifié ;
- **M14** `jo:kj` : `if (0 && pico8_jo_gg(self))` → `test_pico8_planet_kj_dispatch`, sinon `make e2e` ;
- **M15** (preuve du pli, dans le TU) `test_pico8_jo` : `fold_hf` plie `pico8_fixed_from_int(2)` → `test_pico8_jo` rouge (sa constante plie bien le membre), restauré, vert ;
- **M16** (preuve du pli, dans le TU) `test_pico8_hj` : pli de `gg` `? 0u : 1u` → `test_pico8_hj` rouge, restauré, vert.

Cibles (numéros à `4171b9b` ; « + 1 » après D.15c pour les trois fichiers de H-D15c-2) : `pico8_game_data.c:495-620`, `planet.c:150-160`, `:345-370`, `planet_family.c:75-100`, `:170-180`, `:230-240`, `:320-330`, `:610-620`, `economy_score.c:42-60`, `:105-125`, `economy_tiers.c:290-300`, `planet_display.c:225-230`, `:470-475`, `score_panel.c:111-126`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes) au commit. Aucun e2e ajouté.
- **`make class-coverage-net`** (exigé par la ligne) : sortie **identique** à `tools/class_coverage_net_expected.txt` (`jo 2430`, `qh 12`, `ga 2`, `nl 1`, `os 1`, `hj 1`, `pm 1`, `mx 2`, `nr 2`, `fr 1`…).
- **Balayage ASan+UBSan des TU** : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.12a–D.15c.
- `make sanitize-net` : non exigé ; s'il est lancé, résultat rapporté.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 min de temps réel**, mesures, `gcov`, mutations et filets compris ; **plafond 40 min** (au-delà : repli a1/a2). Base : D.15a et D.15b, ≈ 10 à 12 min chacune ; D.16a a 22 chemins (comme D.15b) et 16 mutations d'un `make` ciblé chacune. La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute (`planet.h`, `planet_family.h`, `game_data.h` changent : recompilation large) |
| `make e2e` | ~1 s |
| `make class-coverage-net` | < 1 min |
| `make line-coverage EVENTS="tests/e2e/*.json"` | < 1 min |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes |
| mutations (16) | < 6 min |
| balayage ASan+UBSan des TU | quelques minutes |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; rien d'inachevé n'est commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | HEAD de graduation de D.15c, avant D.16a | 0 | `valeur 879 99` (H-D15c-1) | … |
| 2 | recette `valeur` | idem | 0 | `valeur 879 lignes 99 fichiers 971 occ.` | … |
| 3 | `grep -rnE 'hf_is_bool\|qh_is_bool\|pico8_jo_hf\|pico8_qh_qh' src tests \| wc -l` | idem | 0 | nombre de départ, rapporté | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les 22 chemins de « Empreinte » (plus un TU suiveur listé au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`.
2. **Greps** de la « Sortie », exactement.
3. **Formes** : `int hf`, `int qh` (structs d'entité et `_start_t`), aucun `_is_bool` pour eux ; `int pico8_jo_gg` ; `pico8_jo_hf`, `pico8_qh_qh` absentes ; quatre copies `truthy` absentes ; `gg` en deux tests, non joints.
4. **Casts** : aucun cast nouveau (la conversion `(const pico8_jo_t *)pt` de `call_gg_truthy` était déjà là).
5. **API** : seule `pico8_jo_gg` change de signature ; seules `pico8_jo_hf`, `pico8_qh_qh` disparaissent.
6. **Liens** : grep 6.
7. **Constantes** : grep 7 vide ; plis aux mêmes octets (grep 4) ; M15, M16 rapportés.
8. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste ((a1) 3, (a2) 49, (b) 2, (c) 0) ; aucune ligne gagnée ; aucun include touché (grep 8) ; aucun alias, renommage, cale.
9. **Écarts déclarés** : `pico8_jo_hf`/`pico8_qh_qh` retirées ; `fold_value` retirée de cinq TU ; TU `qh` dans l'empreinte ; − 54 au lieu de ≈ − 34 ; contrat de `gg` renversé.
10. **Preuve** : `gcov` (lignes modifiées exécutées ou justifiées ; `L` vide), mutations restaurées par `cp`, `class-coverage-net` et balayage ASan+UBSan rapportés.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; contrat de `gg` réécrit, `TROU-3` gardé ; aucun renvoi à D.16a ni au plan dans `src/` ou `tests/`.
12. **Docs** : note « Découvert à D.16a » en fin de ligne D.16a ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le commit de graduation de D.15c (ou un descendant qui ne touche que `docs/`), propre ; ou si `valeur` n'y vaut pas 879 / 99 / 971 occ., `type-tag` 243 / 58, résidu D 1120 / 108, sans explication (H-D15c-1 revérifiée à la graduation remplace ces nombres).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change.
- `FAILED:` si un site ne tombe sous aucune règle de ce PRD ni de l'étude (arrêt « champ sans règle »), si un lecteur de `hf`, `qh` ou `gg` qui distingue nil de `false` est découvert (il sort de B1 : arrêt, rapporté), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si `make class-coverage-net` diffère de `tools/class_coverage_net_expected.txt`, ou si le balayage ASan+UBSan des TU n'est pas propre.

**Périmètre et forme**

- `FAILED:` si `tools/`, le `Makefile` ou un `#include "pico8_value.h"` change ; si le commit touche un chemin hors « Empreinte » sans le lister comme TU suiveur.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si une signature autre que `pico8_jo_gg` change, si une fonction autre que `pico8_jo_hf`, `pico8_qh_qh`, les quatre copies `truthy` et les cinq `fold_value` disparaît, si `pk`/`fp`, `et`, `rn`, `oj`, `bg`, `ed`, `qe` changent (D.16b, D.17), ou si une durée de vie change.
- `FAILED:` si un cast nouveau est ajouté, si les deux tests de `gg` sont joints, ou si une conversion vers une struct précise n'a pas son test.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne baisse pas de **54 lignes et 55 occurrences exactement** depuis le départ réel (879 / 971 → 825 / 916 sous H-D15c-1), sans explication chiffrée ligne à ligne ; si `type-tag` change.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, renommage d'un identifiant vivant pour échapper aux greps, commentaire compté réécrit hors de son sujet, cale valeur ↔ typé, (c) non nulle, (b) hors de `pico8_jo_gg`.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées (« aucune ») ; la justification de chaque Δlignes > Δoccurrences ; les deux fonctions publiques retirées et leurs lecteurs ; les quatre copies `truthy` et les cinq `fold_value` retirées, avec la raison `-Werror` ; la preuve B1 (écritures et lecteurs, cart et port) ; le rapport `gcov` ; les mutations ; `class-coverage-net` ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne modifiée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` n'est pas vide.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si le plafond est dépassé sans note (et sans repli a1/a2), ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.16a », ou si elle omet : les comptes réels et l'écart à ≈ 845, les lectures de la ligne, `economy_score.c` sorti du résidu D, les copies `truthy` restantes, les `fold_value` retirées.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- B3 (`et`, `has_et`, `pico8_mz_et`, `pico8_probe_truthy`), B4 (`oj`, `il`, `hp_pose`), B5 (`rn` : `planet.c:445,447` restent), B6 (`bg`), B7 (`pico8_pj_ed`) et les sept copies `truthy` restantes : D.16b.
- B2 (`qe`, `economy_value_equal_content`, `jo_kw_qe`, TROU-3/TROU-24) : D.17. B8–B11 (cases, sources de bouton, `pi`, `s[i]`, `ou[]` de `score_panel.c`) : D.18.
- `pk`/`pk_is_bool` (`hj`, `pm`) et `fp_is_bool` (sonde) : même forme « 1 ou faux » mais hors B1, hors motif `valeur` ; aucune étape ne les nomme (signalé, non traité).
- Les autres aides d'empreinte des TU (`fold_value` de `fr`, `nr`, `jo`, `ga`, `vec_ref`, `fold_particle`…) : D.19a/b. Les includes de `pico8_value.h` (`planet.h:13`, `planet_family.h:11`) : D.19c. Les `assert` d'étiquette : D.20.
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **Types** : `int hf`, `int qh` (mêmes noms), valeurs 1 / 0, dans les structs d'entité et `_start_t` ; `int pico8_jo_gg`, NULL → 0.
  - **Portée** : `pico8_jo_hf`, `pico8_qh_qh` retirées (enveloppes) ; quatre copies `truthy` retirées (imposé par `-Werror`) ; `fold_value` retirée de cinq TU (imposé par `-Werror`, précédent D.15b) ; TU `qh` inclus.
  - **Preuve B1** : faite par le cart (six écritures, onze lectures) et le port (sept constructeurs, sept lecteurs) ; aucun champ ne sort de B1.
  - **TU** : plis aux mêmes octets sans cast (`? 1u : 0u`, `fold_fixed_member(h, pico8_fixed_from_int(1))` / `fold_bool_member(h, 0)`).
  - **Drapeaux** : `class-coverage-net` (ligne) ; balayage ASan+UBSan des TU ; pas de `sanitize-net`.
  - **Découpage** : un commit (≈ 20 min, plafond 40), repli a1/a2 écrit.
- **Surprises de mesure, pour `plan-pilot`** : (1) `fold_value` n'a plus que le pli de `gg` pour appel dans `mx`, `os`, `pm`, `hj`, `nl` : elle part (25 lignes) ; (2) le compte (− 54) dépasse l'estimation (≈ − 34 ou − 35) de 20 lignes, par (1) et par les quatre copies `truthy` (10 lignes) que B1 ne comptait pas ; (3) le TU `qh` doit suivre (`fold_hf`, `fold_qh`), absent de la ligne de l'étude ; (4) l'étude ne dit pas le sort de `pico8_jo_hf`/`pico8_qh_qh` : retirées ; (5) `economy_score.c` sort du résidu D (ses deux dernières lignes, sous H-D15c-2) et `planet_family.c` de `valeur` (il garde une ligne `type-tag`) ; (6) le contrat de `gg` (« deliberately not collapsed to a C int ») est renversé à dessein, `TROU-3` gardé ; (7) les numéros `planet_family.c:103-104,619-620` de l'étude sont d'avant D.15a : `:95-96`, `:615-616` à `4171b9b` ; (8) `pk`/`pk_is_bool` et `fp_is_bool` ont la même forme « 1 ou faux » sans être nommés par aucune étape (hors motif) ; (9) prototypes publics qui nomment `pico8_value_t` : 20 → 17 ; copies `truthy` : 11 → 7.
- **Pour D.16b** : copies `truthy` restantes 7 (`layout`, `probe`, `research`, `research_display`, `tech_gate`, `ui_button`, `wormhole_display`) ; `planet.c` n'a plus que `rn` (`:445`, `:447` à `4171b9b`, numéros décalés par D.16a).
- **Pour D.19a** : `fold_value` déjà absente de `mx`, `os`, `pm`, `hj`, `nl`, `qh` ; les plis de `gg`, `hf`, `qh` sont typés.

## Annexe A — textes recopiés (`docs/plan-3.md` à `4171b9b`, par extraction de lignes)

### Lignes D.15c à D.16b (l. 285-287)

> - D.15c — `valeur` : `tostr` déplacé dans `pico8_str.c` avec ses tests. `sanitize-net` : oui. Compte : ≈ 880.
> - D.16a — genres mêlés B1 (valeurs vraies/fausses devenues entiers). `class-coverage-net` : oui. Compte : ≈ 845.
> - D.16b — genres mêlés B3–B7 (pointeur NULL pour nil/faux, chaînes, `hp_pose` en affectation par position, `rn`, `bg`, `pj_ed`), en deux sous-étapes b1/b2. `class-coverage-net` : oui. Compte : ≈ 765.

(À la graduation de D.15c : recopier la ligne D.15c avec sa note « Découvert à D.15c », et vérifier que la ligne D.16a est identique, H-D15c-6.)

### §7, extrait (« En D et en 2 »)

> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.

## Annexe B — reproduire

```sh
grep -nE 'hf\s*=|qh\s*=' pico8/lua.lua                                  # 6 écritures
grep -nE '\bhf\b|\.qh\b|:gg\(|jo\.gg' pico8/lua.lua                     # lecteurs
grep -rnE '(hf|qh)(_is_bool)? = pico8_[a-z]+_start' src                 # 14 lignes de constructeurs
grep -rnE 'hf_is_bool|qh_is_bool|pico8_jo_hf|pico8_qh_qh|pico8_jo_gg' src tests
grep -nE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c           # 11 à 4171b9b
for t in fr mx os ga pm nr jo hj nl qh; do printf '%s ' $t; grep -c 'fold_value(h' tests/test_pico8_$t.c; done
```
