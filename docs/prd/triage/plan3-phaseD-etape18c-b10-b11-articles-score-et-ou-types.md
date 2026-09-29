---
title: "Plan 3, étape D.18c : genres mêlés B10, B11 et `ou[]` — les articles de `bl` se lisent sur l'item généré (`pico8_pi_arg` rend `const pico8_jb_item_t *`), `s[i]` du panneau de fin est fondu dans son seul lecteur (`pico8_pj_ph_s_str`, TROU-13 écrit au site), `ou[]` de `pj:hb` s'écrit en `pico8_jb_item_t` et `pico8_pj_item_of` disparaît ; aucune constante ne change"
description: "Trois derniers groupes de `src/` fabriquent encore une valeur Lua pour la défaire aussitôt (les articles de `bl` dans `pi.c`, `s[i]` du panneau de fin, le tableau `ou` de `pj:hb`) ; D.18c les écrit sur leur type C, déplace TROU-13 à son site, retire la dernière fonction publique qui nomme `pico8_value_t` hors du module, et laisse `src/` sans aucun appelant de `pico8_value_*` hors du module lui-même : `valeur` baisse de 59 lignes (68 occ.), ≈ 494 → ≈ 435."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.18b est implémentée** (`plan-pilot`, étape 2b), sur l'arbre commis **`b4bfbfa`** (HEAD de `claude/plan-pilot-3-7zdft0` : `ab56a93` plus la version revérifiée du PRD de D.18b, `docs/` seul), extrait par `git archive b4bfbfa` dans le scratchpad. Seuls `grep`, `sed`, `awk` et `sh tools/data_residue.sh` ont tourné ; **rien n'a été compilé ni lancé**. Tout ce que ce PRD suppose de l'issue de D.18b est une hypothèse `H-D18b-n` (plus bas), à vérifier à la graduation de D.18b avant la mise en file. **Numéros de ligne : ceux de `b4bfbfa`** (identiques à `ab56a93` dans `src/` et `tests/`). D.18b ne touche aucun fichier de l'empreinte de D.18c sauf `score_panel_display.c`, où elle retire deux lignes (`:262`, `:277`) **après** toutes les lignes B11 (`:127-180`) : leurs numéros ne bougent pas (H-D18b-2). L'implémenteur remesure par contenu au parent de son commit et explique tout écart.

Mesuré à `b4bfbfa` (départ de D.18b) : `valeur` 560 / 83 ; 623 occ. (`src/` 181 / 45 / 212, `tests/` 379 / 38 / 411), `type-tag` 242 / 58, résidu D 800 / 101. **Attendu au départ de D.18c** (H-D18b-1) : `valeur` **494 / 78 ; 547 occ.** (`src/` 140 / 40 / 165, `tests/` 354 / 38 / 382), `type-tag` **241 / 57**, résidu D **733 / 99**.

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c/.h` disparaissent (D.19c). Après D.18b, trois fichiers de `src/` fabriquent encore une `pico8_value_t` pour la défaire trois lignes plus loin :

- **B10, `pi.c`** : `pico8_pi_arg` traduit chaque item généré d'un article de `bl` (`pico8_jb_item_t`, déjà une union typée) en valeur, que les cinq fonctions de dessin (`br`, `qr`, `rf`, `s`, `ld`) re-trient par `.kind` ; `qr` passe un nombre à `pico8_value_tostr`.
- **B11, `score_panel_display.c`** : `pico8_pj_ph_s_at` range `s[i]` de `ky()` dans une valeur (nombre, chaîne, ou « absent » hors de 1..6) dont l'unique lecteur est `pico8_value_tostr` ; TROU-13 (l'absent rendu `""`) vit dans la branche `REF` de `pico8_value_tostr`, dans le module qui doit disparaître.
- **`ou[]`, `score_panel.c`** : `pj:hb` remplit un tableau `pico8_value_t ou[7]` que `pico8_pj_item_of` (publique, dernière fonction hors du module à nommer `pico8_value_t`) retraduit aussitôt en `pico8_jb_item_t`.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `b4bfbfa` (§5, l. 291) :

> - D.18c — B10, B11, `ou[]`. `sanitize-net` : oui. Compte : ≈ 435.

Ligne de l'étude D.14 (§3), recopiée :

> | **D.18c** | B10 (`pico8_pi_arg` → item généré), B11 (`pico8_pj_ph_s_str`, TROU-13 au site), `ou[]`/`pico8_pj_item_of` en `pico8_jb_item_t` ; TU `pj` | ≈ 6 fichiers | ≈ 435 |

Points de l'étude (§1.3), recopiés :

> **B10. Articles de `bl` (`pi.c`) — 32 lignes.** `pico8_pi_arg` (`pi.c:58-78`) et ses lecteurs `io` (`:100-110`), `eq`, `spr` (`:126-131`), `ld` (`:148-155`).
> - Genres : le premier argument de `io` est une chaîne ou un nombre (année, compteur de techs de la barre de menu, commentaire `pi.c:95-97`) ; les autres, nombre ou nil ; `ld` reçoit un `lq` (référence) et un vecteur.
> - Règle (G) : **lire l'item généré** : `pico8_pi_arg` rend `const pico8_jb_item_t *` (NULL au-delà de la fin) et trois petites lectures typées (`nombre ou défaut`, `chaîne ou NULL`, `pointeur`) ; `io` formate un nombre par `pico8_tostr_fixed`. Raison : `pico8_jb_item_t` est l'union de ces articles ; la valeur n'était qu'une traduction intermédiaire.
>
> **B11. `s[i]` du panneau de fin — 12 lignes.** `score_panel_display.c:143-158` (`pico8_pj_ph_s_at`), `:178-196`.
> - Genres : nombre (`di`, `gl`, `el`, `total`, `tier`), chaîne (`happiness`), nil hors de 1..6 (score 0, TROU-13 et TROU-25) ; l'unique lecteur est `tostr`.
> - Règle (G) : **fusion avec son lecteur** : `char *pico8_pj_ph_s_str(s, i)` rend directement la chaîne possédée (`pico8_tostr_fixed`, copie de `happiness`, `""` hors bornes — TROU-13 préservé et écrit au site). Raison : la valeur n'a pas d'autre lecteur que sa mise en chaîne.
>
> (B8, extrait) `ou[]` et `pico8_pj_item_of` deviennent des `pico8_jb_item_t` écrits directement (l'item généré est déjà l'union typée : `s`, `n`, `row`, `is_false`, `is_fixed`, `is_ref`).

Lignes directrices de l'étude (§4) qui s'appliquent, en extrait : **G1** aucune constante figée ne change ; **G3** TROU-13 et TROU-25 restent à l'identique, leur texte est réécrit quand il nomme `pico8_value_*` ; **G4** un cas de TU d'un genre que `src/` n'écrit pas est supprimé avec le type qui le permettait. Règles communes (§3) : un TU qui nomme un symbole changé le suit dans le même commit ; aucun `*.sha256` ni e2e ne change ; `sanitize-net` pour D.18c (« la chaîne de `s[i]` change de propriétaire »).

Legs reçus, relus ici :

- **D.15c** (note `docs/plan-3.md:285` et PRD, l. 81, 484, 502) : `pico8_value_tostr` garde ses appelants à genres mêlés `pi.c:105` et `score_panel_display.c:197` (`:180` à `b4bfbfa`, remesuré par contenu), pour D.18c ; **TROU-13 « reste dans `pico8_value.c` » jusqu'à D.18c, « qui écrit `""` au site »** ; surprise (9) : « aucun cas `REF` de `pico8_value_tostr` n'est testé » (TROU-13 n'est gardé que par les e2e et, peut-être, un pli de `test_pico8_pj`).
- **D.17** (note, l. 288) : `pico8_value_tostr` garde deux appelants dans `src/` (D.18c) ; `pico8_value_equal` n'a plus que `test_pico8_value` et **reste pour D.19c** (précédent, voir « Lecture »).
- **D.18a** (note, l. 289) : « Ligne prise à B11 : `score_panel_display.c:181` (`items[5]`, les clés de `pj:ph`) » — l'étude comptait 12 lignes B11, il en reste **11** ; « Pour D.18c : `score_panel_display.c` garde `s[i]` (`pico8_pj_ph_s_at`, `s_val`, `pico8_value_tostr`) ».
- **D.18b** (PRD, « Pour D.18c ») : `score_panel_display.c` garde 11 lignes (B11) ; `pico8_pj_item_of` est la dernière fonction publique qui nomme `pico8_value_t` ; `pico8_value_ref`/`pico8_value_bool` n'ont plus d'appelant de `src/` que `pi.c`, `score_panel.c`, `score_panel_display.c`.

### Hypothèses `H-D18b-n` (D.18b non gradée)

Source : le PRD de D.18b (`docs/prd/triage/plan3-phaseD-etape18b-b9-sources-de-bouton-en-champs.md` à `b4bfbfa`), sa Sortie et son « Effet sur `make data-residue` ». À vérifier contre la note « Découvert à D.18b » et `git diff --stat b4bfbfa <graduation> -- src tests Makefile tools docs/trous-de-portage.md docs/glossaire-noms.md`.

| Hypothèse | Énoncé | Effet si elle tombe |
|---|---|---|
| H-D18b-1 | Après D.18b : `valeur` **494 / 78 ; 547 occ.** (`src/` 140 / 40 / 165, `tests/` 354 / 38 / 382) ; `type-tag` **241 / 57** ; résidu D **733 / 99** | départ, arrivée (435 / 75 / 479 occ.), critères `FAILED:` de départ et lignes 1–3 du tableau « Runs » à remettre ; les deltas de D.18c (− 59 / − 68) ne dépendent pas de D.18b |
| H-D18b-2 | D.18b ne change `score_panel_display.c` qu'en retirant `:262` et `:277` (écritures `spec1.lq`/`spec2.lq`) ; après D.18b le fichier compte **11 / 11**, lignes B11 aux mêmes numéros (`:127-142`, `:162`, `:178`, `:180`) | Inventaire B11 à remesurer ; si une ligne B9 reste, elle n'est pas à D.18c (arrêt, rapporté) |
| H-D18b-3 | D.18b ne touche ni `pi.c`, ni `score_panel.c/.h`, ni `pico8_value.c/.h`, ni `test_pico8_pj`, `test_pico8_fm`, `test_pico8_value` ; dans `test_pico8_lq` elle modifie `:533` sur place et retire `:900-904`, `:958` (le cas `ld` de `:600-612` garde ses numéros) | numéros et cibles de mutation à remesurer |
| H-D18b-4 | Copies `*_truthy(pico8_value_t)` : **0** ; fonctions publiques hors du module qui nomment `pico8_value_t` : **1** (`pico8_pj_item_of`) ; `pico8_js_pq` n'en nomme plus | la Sortie « 0 fonction publique » de D.18c suppose ce 1 |
| H-D18b-5 | Appelants de `pico8_value_*` dans `src/` hors du module après D.18b : **`pi.c`, `score_panel.c`, `score_panel_display.c` seuls** (les autres lignes de `src/` sont des includes et trois commentaires) ; `pico8_value_tostr` : `pi.c:105`, `score_panel_display.c:180` | si un autre appelant reste, il n'est pas à D.18c : l'inventaire ne l'absorbe pas, la note le lègue |
| H-D18b-6 | `ui_button.h:13` garde son include (en-tête vidé, D.19c) ; D.18c ne touche pas `ui_button.*` | aucun |
| H-D18b-7 | `Makefile`, `tools/`, `DATA_RESIDUE_GUARD` inchangés par D.18b ; `DATA_ENGINE_SRCS` contient `pi.c` et `str.c` | règle de lien à revoir |
| H-D18b-8 | TROU-13 (`docs/trous-de-portage.md:190-205`) inchangé ; glossaire : D.18b ne réécrit que l'entrée `js` (`:444`), sur une ligne : `:294` (`tostr(v)`) et `:657` (`pico8_value_ref(NULL)`) gardent leur texte et leur numéro | textes « avant » de la Documentation à relire |
| H-D18b-9 | D.18b en un commit, ≈ 15 min de la mise en file au commit | budget à recalibrer sur le temps réel |
| H-D18b-10 | La ligne D.18c du plan reste `- D.18c — B10, B11, \`ou[]\`. \`sanitize-net\` : oui. Compte : ≈ 435.` ; la note de D.18b dit « pour D.18c : `score_panel_display.c` n'a plus que B11 (11 lignes) » | Lecture à refaire si la ligne change |
| H-D18b-11 | D.18b ne crée ni ne supprime de TU ; les TU qui prouvent D.18c (`pj`, `fm`, `lq`, `fv`, `mh`, `ga`) et leurs constantes sont intacts | liste « Prouvent sans suivre » à revoir |

### Sections à revérifier à la graduation de D.18b

1. « État de départ » et critères `FAILED:` de départ (H-D18b-1) ; lignes 1–3 du tableau « Runs ».
2. Inventaire B11 : numéros de `score_panel_display.c` (H-D18b-2) ; totaux par fichier.
3. Cibles des mutations dans `test_pico8_lq` (`:600-612`) et `score_panel_display.c` (H-D18b-3).
4. « Effet sur `make data-residue` » : arrivée absolue (H-D18b-1), fichiers qui sortent (H-D18b-5).
5. Documentation : textes « avant » de TROU-13 et des lignes `:294`, `:657` du glossaire (H-D18b-8).
6. Budget (H-D18b-9) ; découpage (inchangé sauf surprise).
7. Annexe A : ligne D.18b avec sa note, ligne D.18c.

### Preuve B10, par le cart

`oo` (lua.lua:51-53) appelle `fn(a[1],a[2],a[3],a[4],a[5])` : un item au-delà de la fin est nil. `pi={b=qz,v=v,br=rk,qr=io,rf=eq,s=spr}` (l. 133) et `ld` (le `lq:ol` non lié, `score_panel_display.c:231`). Lecteurs : `rk(y1,y2,...)` (l. 24 : `rectfill(0,y1,127,y2 or y1,...)`), `eq(x1,y1,x2)` (l. 25), `io(t,x,y,c,a)` (l. 26-30 : `print(t,…)`, `if (a) x-=a*4*#t`), `spr`, `lq:ol(p,a)` (l. 295-298).

### Preuve B10, par le port : producteurs d'articles

Grep complet à `b4bfbfa` : `grep -rn '"br"\|"rf"\|"qr"\|"ld"\|fn = "s"' src/*.c` (hors enregistrement), plus les réécritures d'items (`pj:hb`, `pj:hk`) et le seul TU qui appelle `pico8_bl*` (`test_pico8_lq:612`). Appelants de `bl` : `help.c:96`, `msgbox.c:76`, `score_panel.c:260`.

| `fn` | Producteurs (`b4bfbfa`) | Items |
|---|---|---|
| `br` | `game_data.c` : `bz` 1–5, `pj_i__1-2`, `pj_ea_1` | 3 entiers, toujours |
| `rf` | `pj_i__3-5` | 3 entiers |
| `s` | `fv_rb_1-2` (aide) | 5 entiers |
| `qr` | `fv_rb_3-4` (chaîne + 3 entiers) ; `pj_gs_1/2/5` (chaîne, 3 entiers, `0.5` en `is_fixed`) ; `pj_ea_2` (item 1 réécrit par `pj:hk`, chaîne « year N ») ; `pj_ou_1-7` (4 items ; item 1 `false` à l'init, puis réécrit par `pj:hb` : chaîne, nombre `is_fixed`, ou nil `is_ref` NULL) | a1 : chaîne, `false`, nombre ou nil ; a2–a4 entiers ; a5 `0.5` ou absent |
| `ld` | `pico8_pj_ph_map` (`score_panel_display.c:228-231`) ; `test_pico8_lq:602-612` | `is_ref` `lq`, `is_ref` vecteur (non NULL : toutes les rangées à score ont `ra`), `is_fixed` 0.5 |

**Aucun producteur n'écrit** : une rangée imbriquée dans un article (l'`assert` de `pi.c:65` reste), un non-nombre en position lue sans test de genre (`rk` a1, `eq` a1–a3, `spr` a1–a3), un `br` à moins de deux nombres, un `s` sans `w`/`h`. Conséquences : les lectures inconditionnelles `.as.fixed` d'aujourd'hui ne lisent jamais qu'un nombre (ou, au-delà de la fin, les bits d'une référence NULL, soit 0) ; **les branches `pico8_rk_y1` et `pico8_spr_default` ne sont atteintes par aucune donnée de `src/`** (justifiées au `gcov`, pas couvertes par un test nouveau).

**Table de vérité** (item → lecture), aujourd'hui par `pico8_pi_arg` puis `.kind`, après D.18c par les lectures typées :

| Item (ordre de priorité de `pico8_pi_arg`) | Aujourd'hui | Après D.18c |
|---|---|---|
| au-delà de la fin | `REF` NULL : pas un nombre, pas une chaîne ; `.as.fixed` lu = 0 | `pico8_pi_arg` NULL : `pico8_pi_is_number` 0, `pico8_pi_number` 0, `pico8_pi_ref` NULL |
| `s != NULL` | `STRING` | chaîne (`io` seul la lit) ; ni nombre ni pointeur |
| `is_false` | `BOOL` 0 : ni nombre, ni chaîne, ni `REF` | ni nombre, ni chaîne, ni pointeur |
| `is_ref` | `REF(ref)` | `pico8_pi_ref` = `ref` ; pas un nombre |
| `is_fixed` | `FIXED(fixed)` | nombre `fixed` |
| sinon | `FIXED(n)` | nombre `n` |

Même membre lu pour chaque genre que `src/` écrit ; aucune ligne ne diverge.

### Preuve B11 et TROU-13, par le cart et le port

`pj:ph` (lua.lua:883-884) : `st[3]=tostr(s[st.score])` ; `ky()` (l. 1339-1362) rend `{di, gl, el, fm.happiness.."%", total, min(flr(total/2000),5)}`. `s[0]` (la rangée `o("",score=0,…)`, l. 803) est nil : PICO-8 écrirait `"[nil]"`, le port écrit `""` (TROU-13, `docs/trous-de-portage.md:190-205`). Port : `pico8_ky_t` (`economy_score.h:22-29`) : cinq `pico8_fixed_t` et `char *happiness` (tas, jamais libéré, TROU-7, jamais NULL : `ky` l'écrit toujours). Le seul appelant de `pico8_pj_ph_s_at` est `pico8_pj_ph_map` (`:176`), le seul lecteur de `s_val` est `pico8_value_tostr` (`:180`), qui rend une chaîne possédée (copie pour `STRING`, `pico8_tostr_fixed` pour `FIXED`, `""` pour `REF`) rangée dans `st->items[2].s` et jamais libérée (`/* TROU-7 */`, `:181`). `s[6]` (`tier`) est aussi lu directement pour les icônes (`:201`), hors valeur.

### Preuve `ou[]`, par le cart et le port

`pj:hb` (lua.lua:847-850) : `local ou={fm.nz.."$",cf,fm.jk,"+"..fm.po[r.k]+1,fm.happiness.."%",ef[fm.lo],fm.yr}` puis `for i,p in pairs(ou) do pj.ou[i][1]=p end`. Port (`score_panel.c:160-232`) : sept `pico8_value_t` — chaînes (`ou[0]`, `[1]`, `[3]`, `[4]`, `[5]` quand `lo` est dans 1..`PICO8_EF_COUNT`), nombres (`jk`, `yr`), nil (`jk` du `fm` provisoire sans `has_jk`, `yr` sans `fm`, `ef` hors bornes) — puis `pico8_pj_item_of` : `STRING` → `{.s}` (empruntée), `FIXED` → `{.is_fixed = 1, .fixed}`, `BOOL` faux → `{.is_false = 1}` (jamais écrit par `hb`), `REF` NULL → `{.is_ref = 1}`, autre → `assert`. **Seul appelant de `pico8_pj_item_of` : `score_panel.c:229`** ; aucun TU ne l'appelle (grep à `b4bfbfa`). Lecteurs des items : `pi.c` (`io`, table ci-dessus), `test_pico8_fm:104,108` (`sub_items[2][0].is_ref`, `sub_items[0][0].s`), `test_pico8_pj` (`fold_ou_heads` par `item_value`, qui plie `is_fixed` et `n` en un même `FIXED` : le drapeau `is_fixed` n'est pas vu par les constantes, il est quand même gardé).

**Constat hors ligne** (pas un écart introduit, rien à faire ici) : le `pairs` du cart saute un nil (`pj.ou[i][1]` garde son ancienne valeur) ; le port écrit nil. Les nils d'`ou` ne viennent que de chemins où le cart planterait avant (`fm` provisoire : `ne(fm.ge)`, `fm.po[r.k]`, TROU-15) ou de `ef[fm.lo]` hors bornes ; `io` rend `""` pour nil comme pour le `false` initial. D.18c préserve le comportement du port à l'identique ; signalé à `plan-pilot` (voir Further Notes), sans entrée TROU ajoutée ici (la preuve d'atteignabilité n'est pas faite).

### Lecture de la ligne pour D.18c

- **B10 « lire l'item généré »** (G, règle de l'étude) : `static const pico8_jb_item_t *pico8_pi_arg(const pico8_jb_row_t *e, int i)` (NULL au-delà de la fin ; l'`assert` « an article of bl holds no nested row » reste) ; trois lectures `static` : **`pico8_pi_is_number(e, i)`** (1 si l'item existe et n'est ni chaîne, ni faux, ni pointeur — l'ordre de priorité d'aujourd'hui), **`pico8_pi_number(e, i)`** (le nombre, `is_fixed ? fixed : n`, sinon 0), **`pico8_pi_ref(e, i)`** (`const void *`, le pointeur si l'item en est un, sinon NULL). La lecture « chaîne ou NULL » n'a qu'un lecteur (`io` a1) : elle s'écrit au site (`a1 != NULL && a1->s != NULL`), sans aide. **Écart à l'étude, déclaré** : quatre aides et non trois ; le prédicat est exigé par les quatre sites qui choisissent une branche sur le genre (`rk` a2, `io` a1 et a5, `spr` a4 et a5), et **tous les défauts d'aujourd'hui valent 0** : « nombre ou défaut » devient « nombre ou 0 », sans paramètre. **Rejeté** : une lecture `int f(e, i, pico8_fixed_t *out)` (trois locales de plus par fonction de dessin) ; garder `pico8_pi_arg` rendant une petite union locale (ce serait refaire `pico8_value_t` sous un autre nom : cale interdite).
- **`io` formate un nombre par `pico8_tostr_fixed`** : `char *owned = pico8_pi_is_number(e, 1) ? pico8_tostr_fixed(pico8_pi_number(e, 1)) : NULL;` ; `pi.c` gagne `#include "pico8_str.h"` (il avait `pico8_value_tostr` par `layout.h` → `pico8_value.h`). Le commentaire-piège `:96-98` reste.
- **`spr`** (G) : `pico8_pi_spr_call` (cinq paramètres valeur, un seul appelant depuis que le `'s'` à clé caractère est parti) est **fondue dans `pico8_pi_spr_draw`**, qui lit ses cinq items comme `rk`, `io`, `eq`, `ld` ; son commentaire garde le seul piège (« w/h are supplied both or neither, as in real spr() »). **Rejeté** : garder `pico8_pi_spr_call(const pico8_jb_row_t *e)` derrière un `spr_draw` d'une ligne (enveloppe sans objet ; et la signature compte deux lignes qu'il faudrait joindre).
- **`ld`** : `const pico8_lq_t *self = pico8_pi_ref(e, 1);`, `const pico8_vec_t *at = pico8_pi_ref(e, 2);`, `p = at != NULL ? *at : pico8_vec_v(0, 0)` : **les casts `(void *)item->ref` (`:73`), `(const pico8_lq_t *)` (`:151`) et `*(const pico8_vec_t *)` (`:153`) disparaissent** (conversion implicite depuis `const void *`), aucun n'est remplacé.
- **B11 « fusion avec son lecteur »** (G, règle et nom de l'étude) : `static char *pico8_pj_ph_s_str(const pico8_ky_t *s, int i)`, contrat « returns a newly allocated string the caller owns » : `pico8_tostr_fixed(s->di)` … ; **`pj_concat2(s->happiness, "")`** pour la chaîne (copie, par l'aide possédante déjà dans le fichier) ; hors de 1..6, **`/* TROU-13 */`** nu puis `return pj_concat2("", "");`. L'appel `:176` et la mise en chaîne `:180` deviennent **`s_str = pico8_pj_ph_s_str(ctx->s, score_i);`**, `s_val` part, le commentaire `:177-179` part (son contenu est TROU-13, désormais marqué au site). **Rejeté** : emprunter `s->happiness` et un littéral `""` (propriété mêlée dans une même fonction : l'une des branches rendrait une chaîne que l'appelant ne doit pas libérer) ; une aide `strdup` nouvelle (la seule du dépôt, `pico8_strdup_owned`, est `static` dans `pico8_value.c`, qui part en D.19c). Le nom `pico8_pj_ph_s_str` est celui de l'étude : le contrat change (valeur → chaîne possédée), l'ancien nom `s_at` ne correspond à aucun motif, ce n'est pas un renommage d'évitement ; déclaré (a1).
- **`ou[]`** (G) : `pico8_jb_item_t ou[7]` écrit directement, par littéraux composés (prior art : `pico8_pj_item_of` elle-même, `score_panel_display.c:228-230`) ; `fm_jk` et `fm_yr` en `pico8_jb_item_t` (nil : `{.is_ref = 1}`, nombre : `{.is_fixed = 1, .fixed = …}`, **les octets exacts de `pico8_pj_item_of`**) ; la boucle `:229` devient `ou_state->sub_items[i][0] = ou[i];`. **`pico8_pj_item_of` disparaît** (définition `score_panel.c:144-158`, déclaration et contrat `score_panel.h:120-122`) : son `assert` de genre tombe avec le type qui le permettait (plus rien ne peut passer un `BOOL` vrai ou une référence non NULL). Les littéraux composés ne sont pas des casts (la grep du contrôle 4 exclut `(type){`).
- **`pico8_value_tostr` : gardée jusqu'à D.19c** (G). Après D.18c elle n'a plus d'appelant dans `src/`, seulement `test_pico8_value:69,75,76`. Raisons : précédent de D.17 (`pico8_value_equal`, sans appelant de `src/`, gardée pour D.19c avec le test de la couche) ; `test_pico8_value` est un test de la couche que D.19c supprime entier ; supprimer maintenant coûterait 10 lignes de plus dans trois fichiers que D.19c supprime de toute façon (`pico8_value.h:68,70`, `pico8_value.c:61,63,65,67,69`, `test_pico8_value.c:69,75,76`, plus `pico8_strdup_owned`, `<stdlib.h>`, `<string.h>`, l'include de `pico8_str.h` et les deux tests), sans rien changer à la Sortie. **Mais ses deux marqueurs `/* TROU-13 */` (`pico8_value.h:69`, `pico8_value.c:70`) partent** : l'écart ne vit plus dans cette branche (plus aucun appelant de `src/` ne l'atteint), un marqueur TROU se met où l'écart se produit. Lignes non comptées ; le contrat de `pico8_value.h:60-68` (dont `:68`, comptée) ne change pas. **Rejeté** : supprimer la fonction ici (ci-dessus) ; laisser les marqueurs (TROU-13 nommerait deux lieux dont un mort).
- **Includes** : `pi.c` gagne `"pico8_str.h"` ; aucun retiré (`pi.c`, `score_panel.c`, `score_panel_display.c` n'ont pas d'include propre de `pico8_value.h`, ils l'ont par `layout.h`/`score_panel.h`). **`score_panel.h:12` gardé** (en-tête, D.19c, précédent `ui_button.h:13`).
- **`sanitize-net` : oui** (ligne ; la chaîne de `s[i]` passe d'une allocation de `pico8_value.c` à celles de `score_panel_display.c`) ; **balayage ASan+UBSan des TU** exigé (consigne). **`class-coverage-net`** : non exigé (aucun constructeur touché) ; s'il est lancé, rapporté.
- **« Compte : ≈ 435 »** : mesuré ici **− 59 lignes / − 68 occ.** depuis 494 (H-D18b-1) : **435**, la cible du plan exactement. L'étude comptait B10 32 + B11 12 + `ou[]` (dans B8) ; B11 a perdu `:181` au profit de D.18a. « ≈ » n'est pas un critère ; les deltas exacts le sont.
- **Aucun TU ne suit par symbole** : aucun ne nomme `pico8_pj_item_of`, `pico8_pi_arg`, `pico8_pj_ph_s_at` (grep à `b4bfbfa`) ; les lignes `valeur` de `test_pico8_pj` sont des plis (D.19b). L'étude disait « TU `pj » : `test_pico8_pj` ne change que si une mutation exigée ne rougit aucun TU (assertion ajoutée, « Testing Decisions »).
- **Découpage** : **(G) un commit** (`refactor(D.18c)`). 59 lignes dans quatre fichiers, deux blocs indépendants (`pi.c` ; `score_panel*`). Repli écrit plus bas.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; jamais l'appli raylib ; interdits : jonction de lignes comptées (sauf contrefactuel déclaré), alias, renommage pour échapper aux greps, cast nouveau, cale valeur ↔ typé ou pont, aide laissée sans appel (`-Werror`) ; mutations dans une copie jetable ou restaurées par `cp` simple puis `touch` ; binaires lancés exactement comme le `Makefile`.

### État de départ

| Mesure | À `b4bfbfa` (D.18b non commencée) | Départ de D.18c (H-D18b-1) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 560 / 83 ; 623 occ. | **494 / 78 ; 547 occ.** (`src/` 140 / 40 / 165, `tests/` 354 / 38 / 382) |
| `type-tag` | 242 / 58 | **241 / 57** |
| résidu D | 800 / 101 | **733 / 99** |
| fonctions publiques hors du module qui nomment `pico8_value_t` | 2 | **1** (`pico8_pj_item_of`) |
| copies `*_truthy(pico8_value_t)` | 1 | **0** |
| appelants de `pico8_value_tostr` dans `src/` | 2 | **2** (`pi.c:105`, `score_panel_display.c:180`) |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |

Recette des occurrences (celle de D.12–D.18b, motif `valeur` de `tools/data_residue.sh:29`) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'valeur\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire

Classes du message de commit : **(a1)** même site, type C à la place de la valeur ; **(a2)** retirée sans remplaçant au site (locale dont la lecture passe par une lecture typée, tri déplacé dans une aide neuve hors motif, fonction ou aide sans appel, commentaire dont l'objet part) ; **(b)** signature ou membre public dont le type change ou qui disparaît ; **(c)** même donnée, autre nom : **attendu 0**. `occ.` : occurrences du motif. Chaque ligne listée **sort** de `valeur` ; aucune n'en gagne. Numéros de `b4bfbfa`.

#### API

| Symbole | Aujourd'hui | Après D.18c | Classe |
|---|---|---|---|
| `pico8_pj_item_of` (`score_panel.h:122`, `.c:144`) | `pico8_jb_item_t pico8_pj_item_of(pico8_value_t v)` | **retirée** (seul appelant `score_panel.c:229`, aucun TU) ; contrat `.h:120-121` retiré | (b) ×2 |
| `pico8_pi_arg` (`static`) | `pico8_value_t (e, i)` | `const pico8_jb_item_t *(e, i)`, NULL au-delà de la fin | (a1) |
| `pico8_pi_is_number`, `pico8_pi_number`, `pico8_pi_ref` (`static`, nouvelles) | — | voir « Lecture » ; hors de tout motif | — |
| `pico8_pi_spr_call` (`static`) | cinq `pico8_value_t` | **fondue** dans `pico8_pi_spr_draw` | (a2) |
| `pico8_pj_ph_s_at` → `pico8_pj_ph_s_str` (`static`) | `pico8_value_t (s, i)` | `char *(s, i)`, chaîne possédée | (a1) |

#### Sites de `src/` (59 lignes, 68 occ. retirées ; aucune gagnée)

| Fichier | Lignes `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `pi.c` | `:58` (1) | `static const pico8_jb_item_t *pico8_pi_arg(const pico8_jb_row_t *e, int i) {` ; contrat `:56-57` réécrit (« NULL past the end, where the cart's `oo` unpacked nil ») | (a1) 1 |
| | `:62` (1) | `return NULL;` | (a1) 1 |
| | `:67`, `:70`, `:73`, `:76`, `:78` (1 chacune) | tri retiré : `pico8_pi_arg` rend `item` après l'`assert` ; le tri vit dans les trois lectures (lignes neuves) ; cast `(void *)` de `:73` parti | (a2) 5 |
| | `:84` (1) | `pico8_fixed_t y1 = pico8_pi_number(e, 1);` | (a1) 1 |
| | `:85`, `:86` (1 chacune) | locales retirées | (a2) 2 |
| | `:88` (1) | `col = pico8_fixed_flr_int(pico8_pi_number(e, 3));` (défaut 0 : `flr(0) = 0`) | (a1) 1 |
| | `:89` (1) | `if (pico8_pi_is_number(e, 2)) {` ; `:90`, `:92` (0) lisent `y1`, `pico8_pi_number(e, 2)` | (a1) 1 |
| | `:100` (1) | `const pico8_jb_item_t *a1 = pico8_pi_arg(e, 1);` | (a1) 1 |
| | `:101`–`:104` (1 chacune) | locales `a2`–`a5` retirées | (a2) 4 |
| | `:105` (2) | `char *owned = pico8_pi_is_number(e, 1) ? pico8_tostr_fixed(pico8_pi_number(e, 1)) : NULL;` | (a1) 1 |
| | `:106` (1) | `const char *t = a1 != NULL && a1->s != NULL ? a1->s : (owned != NULL ? owned : "");` | (a1) 1 |
| | `:107`, `:108` (1 chacune) | `pico8_fixed_t x = pico8_pi_number(e, 2);`, `y = …(e, 3)` | (a1) 2 |
| | `:109` (1) | `int c = pico8_fixed_flr_int(pico8_pi_number(e, 4));` | (a1) 1 |
| | `:110` (1) | `if (pico8_pi_is_number(e, 5)) {` ; `:111` (0) `a_val = pico8_pi_number(e, 5)` | (a1) 1 |
| | `:120-121` (0) | `pico8_eq(…, pico8_pi_number(e, 1), pico8_pi_number(e, 2), pico8_pi_number(e, 3));` (deux lignes non comptées jointes, déclaré) | — |
| | `:126`, `:127` (4, 1) | signature de `pico8_pi_spr_call` retirée (fondue dans `spr_draw`, `:140-143`) ; `:128-130` (0) lisent `pico8_pi_number(e, 1..3)` | (a2) 2 |
| | `:131` (2) | `if (pico8_pi_is_number(e, 4) && pico8_pi_is_number(e, 5)) {` | (a1) 1 |
| | `:148`, `:149` (1 chacune) | `const pico8_lq_t *self = pico8_pi_ref(e, 1);`, `const pico8_vec_t *at = pico8_pi_ref(e, 2);` | (a1) 2 |
| | `:150`, `:151` (1 chacune) | locale `a3` et tri de `self` retirés (cast `(const pico8_lq_t *)` parti) | (a2) 2 |
| | `:152` (1) | `pico8_vec_t p = at != NULL` ; `:153` (0) `? *at` (cast parti) | (a1) 1 |
| | `:155` (1) | `pico8_fixed_t a = pico8_pi_number(e, 3);` | (a1) 1 |
| `score_panel.c` | `:144` (1) | définition de `pico8_pj_item_of` retirée (`:144-158`) | (b) 1 |
| | `:146`, `:148`, `:150`, `:154` (1 chacune) | corps retiré (avec `:155`, 0) | (a2) 4 |
| | `:165` (2) | `pico8_jb_item_t fm_jk = {.is_ref = 1};` | (a1) 1 |
| | `:168` (3) | `pico8_jb_item_t fm_yr = fm != NULL ? (pico8_jb_item_t){.is_fixed = 1, .fixed = fm->yr} : (pico8_jb_item_t){.is_ref = 1};` (coupée en deux lignes si > 120 colonnes : la suite ne porte pas le motif) | (a1) 1 |
| | `:171` (1) | `pico8_jb_item_t ou[7];` | (a1) 1 |
| | `:181`, `:187` (1 chacune) | `fm_jk = (pico8_jb_item_t){.is_fixed = 1, .fixed = fm->jk};` (resp. `provisional->jk`) | (a1) 2 |
| | `:194`, `:197`, `:210`, `:215` (1 chacune) | `ou[k] = (pico8_jb_item_t){.s = …};` (même chaîne, empruntée comme aujourd'hui) | (a1) 4 |
| | `:220` (2) | `ou[5] = (lo >= 1 && lo <= PICO8_EF_COUNT) ? (pico8_jb_item_t){.s = pico8_ef_table[lo - 1]}` / `: (pico8_jb_item_t){.is_ref = 1};` (deux lignes, la suite hors motif) | (a1) 1 |
| | `:229` (0) | `ou_state->sub_items[i][0] = ou[i];` | — |
| `score_panel.h` | `:122` (1) | déclaration retirée, contrat `:120-121` avec ; include `:12` **gardé** (D.19c) | (b) 1 |
| `score_panel_display.c` | `:127` (1) | `static char *pico8_pj_ph_s_str(const pico8_ky_t *s, int i) {` ; contrat `:126` : « `tostr(s[i])`, a newly allocated string the caller owns » | (a1) 1 |
| | `:130`, `:132`, `:134`, `:138`, `:140` (1 chacune) | `return pico8_tostr_fixed(s->di);` … `(s->tier)` | (a1) 5 |
| | `:136` (1) | `return pj_concat2(s->happiness, "");` | (a1) 1 |
| | `:142` (1) | `/* TROU-13 */` (nu, ligne neuve) puis `return pj_concat2("", "");` | (a1) 1 |
| | `:162` (1) | `pico8_value_t s_val;` retirée | (a2) 1 |
| | `:178` (1) | commentaire `:177-179` retiré (TROU-13 marqué à `:142`) | (a2) 1 |
| | `:180` (1) | `s_str = pico8_pj_ph_s_str(ctx->s, score_i);` ; `:176` (0) part | (a1) 1 |
| `pico8_value.h`, `pico8_value.c` | `:69`, `:70` (0) | marqueurs `/* TROU-13 */` retirés ; rien d'autre | — |

Totaux (lignes / occ.) : `pi.c` 32 / 37, `score_panel.c` 15 / 19, `score_panel.h` 1 / 1, `score_panel_display.c` 11 / 11 : **59 lignes, 68 occ.** Classes : **(a1) 36** (`pi.c` 17, `score_panel.c` 10, `score_panel_display.c` 9), **(a2) 21** (15, 4, 2), **(b) 2**, **(c) 0**.

**Contrefactuel des locales (anti-tassement).** Dans `pi.c`, chaque locale `pico8_value_t aK = pico8_pi_arg(e, K);` part, et sa lecture passe dans la ligne qui l'utilisait (déjà comptée : `(aK.kind == PICO8_VALUE_FIXED) ? …`). Forme contrefactuelle : garder chaque locale en `const pico8_jb_item_t *aK = pico8_pi_arg(e, K);` (hors motif). **Dans les deux formes, les 32 lignes comptées de `pi.c` sortent toutes et aucune n'est gagnée** ; seule la répartition (a1)/(a2) change. Aucune ligne comptée n'est jointe à une autre ligne comptée. Jointures et coupes de lignes **non comptées**, déclarées : `eq` `:120-121` (une ligne) ; `score_panel.c:168`, `:220` (coupées si > 120 colonnes) ; `score_panel_display.c:176` et `:180` (une ligne, `:176` non comptée).

**Δlignes > Δocc.** : attendu aucun fichier (`pi.c` 32 ≤ 37, `score_panel.c` 15 ≤ 19, les autres égaux).

#### Les TU

**Aucun ne suit par symbole** (0 ligne `valeur` changée dans `tests/`). Assertions ajoutées seulement si une mutation exigée ne rougit aucun TU (voir « Testing Decisions ») : dans `test_ph_rows_show_ky` de `test_pico8_pj` (lignes hors motif, `pico8_lq_cell_t`), nommées au message.

#### Includes

Ajouté : `pi.c` `#include "pico8_str.h"`. Retiré : aucun. Aucun include `pico8_value.h` ne change (en-têtes et TU : D.19c).

#### `Makefile` et liens

- Aucun fichier nouveau ; **aucune ligne du `Makefile` ne change** (`test_pico8_value` garde `pico8_str.c`, D.19c).
- **Règle de lien** (aucun fichier de `DATA_ENGINE_SRCS`, `Makefile:229-279`, n'appelle un symbole défini hors de cette liste) : `pi.c` (dans la liste) appelle désormais `pico8_tostr_fixed` (`pico8_str.c`, dans la liste) et plus `pico8_value_tostr` ; `score_panel.c` et `score_panel_display.c` ne sont pas dans la liste. Vérifié par la construction des binaires à liste courte (grep 6).

## Solution

**Un commit** `refactor(D.18c)` (9 chemins : `pi.c`, `score_panel.c`, `score_panel.h`, `score_panel_display.c`, `pico8_value.c`, `pico8_value.h`, `docs/plan-3.md`, `docs/trous-de-portage.md`, `docs/glossaire-noms.md` ; plus `tests/test_pico8_pj.c` si une assertion est ajoutée), vert seul, vérifié, poussé :

1. Les fonctions de dessin de `bl` lisent l'item généré par trois lectures typées ; `io` formate un nombre par `pico8_tostr_fixed`.
2. `s[i]` du panneau de fin est mis en chaîne là où il est choisi ; TROU-13 est écrit au site.
3. `pj:hb` écrit ses sept items directement ; `pico8_pj_item_of` disparaît.
4. `src/` n'appelle plus rien de `pico8_value_*` hors du module lui-même.

**Sortie**, constatée par commande :

- `make test` vert (dont `data-guard`, `constants-check`, `float-grep`, `float-guard`) ; `make e2e` **26 conformes** ; **`make sanitize-net` propre** ; balayage ASan+UBSan des TU propre ;
- `make data-residue` : `valeur` **494 → 435** lignes (**547 → 479** occ.), fichiers **78 → 75** (sortent `pi.c`, `score_panel.c`, `score_panel_display.c`) ; `valeur` `src/` **140 / 40 / 165 → 81 / 37 / 97** ; `tests/` inchangé (354 / 38 / 382) ; `type-tag` **241 / 57 inchangé** ; résidu D **733 / 99 → 674 / 97** (sortent `pi.c`, `score_panel_display.c` ; `score_panel.c` reste par `type-tag`) ; départs de H-D18b-1 (si le départ réel diffère : départ réel − 59 / − 68 occ.) ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. vidés : 0 pour chacun
  grep -cE "$P" src/pico8_pi.c src/pico8_score_panel.c src/pico8_score_panel_display.c
  # 2. score_panel.h : une seule ligne, l'include
  grep -nE "$P" src/pico8_score_panel.h
  # 3. src/ n'appelle plus le module : vide (hors du module, seuls includes et commentaires)
  grep -nE "$P" src/*.c src/*.h | grep -vE '^src/pico8_value\.(c|h):' | grep -vE '#include|:\s*(\*|/\*)'
  # 4. plus aucune fonction publique hors du module ne nomme pico8_value_t : vide
  #    (type_tag.h:10, un commentaire, est écarté par le second filtre ; D.19c/D.20b)
  grep -nE 'pico8_value_t' src/*.h | grep -v '^src/pico8_value.h:' | grep -vE ':\s*(\*|/\*)'
  # 5. pico8_value_tostr : appelée par test_pico8_value seul
  grep -rnE 'pico8_value_tostr\(' src tests | grep -vE '^src/pico8_value\.(c|h):'
  # 6. règle de lien
  make tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_bg_kj tmp/test_pico8_planet_gh_row_lq \
       tmp/test_pico8_ga_export_panel_by tmp/test_pico8_layout_nil_item_draws_nothing
  # 7. aucune constante de TU changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 8. includes : exactement une ligne '+' (pi.c pico8_str.h), aucune '-'
  git diff <parent> -- src tests | grep -E '^[-+]\s*#include'
  # 9. TROU-13 : un marqueur nu dans score_panel_display.c, aucun dans pico8_value.c/.h,
  #    texte de trous-de-portage sans pico8_value_
  grep -c 'TROU-13 \*/' src/pico8_score_panel_display.c; grep -c 'TROU-13' src/pico8_value.c src/pico8_value.h
  awk '/^## TROU-13/,/^## TROU-14/' docs/trous-de-portage.md | grep -c 'pico8_value_'
  # 10. plus de pico8_pj_item_of ni de pico8_pi_spr_call : vide
  grep -rnE 'pico8_pj_item_of|pico8_pi_spr_call|pico8_pj_ph_s_at' src tests docs/glossaire-noms.md docs/trous-de-portage.md
  # 11. comptes ; 12. filets
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^(valeur|type-tag)'
  make sanitize-net
  ```

  (grep 2 : `12:#include "pico8_value.h"` ; grep 5 : trois lignes de `tests/test_pico8_value.c` ; grep 9 : `1`, puis `0` et `0`, puis `0`.)

## User Stories

1. En tant que mainteneur du port, je veux que les fonctions de dessin de `bl` lisent l'item généré de leur article, afin qu'aucune valeur Lua ne soit fabriquée pour dessiner un rectangle, un texte, un sprite ou une rangée.
2. En tant que mainteneur du port, je veux que `pico8_pi_arg` rende l'item ou NULL au-delà de la fin, afin que le nil de `oo` s'écrive comme un pointeur absent.
3. En tant que mainteneur du port, je veux trois lectures typées (nombre ?, nombre ou 0, pointeur ou NULL) qui gardent l'ordre de priorité d'aujourd'hui (chaîne, faux, pointeur, nombre), afin qu'aucun genre écrit par `src/` ne change de lecture.
4. En tant que mainteneur du port, je veux que `io` formate un nombre par `pico8_tostr_fixed`, afin que l'année et le compteur de techs de la barre de menu ne passent plus par le module.
5. En tant que mainteneur du port, je veux que les casts de `void *` de `pi.c` disparaissent sans remplaçant, afin que les pointeurs d'un article `ld` gardent leur type par conversion implicite.
6. En tant que mainteneur du port, je veux que `pico8_pi_spr_call` soit fondue dans son seul appelant, afin qu'aucune fonction ne prenne cinq valeurs pour un seul site.
7. En tant que mainteneur du port, je veux que `s[i]` soit mis en chaîne là où il est choisi (`pico8_pj_ph_s_str`), afin que sa valeur n'ait plus de lecteur intermédiaire.
8. En tant que mainteneur du port, je veux que la chaîne de `s[i]` soit toujours possédée par l'appelant (nombre formaté, copie de `happiness`, `""`), afin que le contrat de propriété soit le même dans toutes les branches.
9. En tant que lecteur de `docs/trous-de-portage.md`, je veux que TROU-13 soit marqué à la branche qui rend `""` pour `s[0]`, afin que l'écart soit écrit là où il se produit et plus dans un module qui part.
10. En tant que mainteneur du port, je veux que `pj:hb` écrive ses sept items en `pico8_jb_item_t`, afin que `pico8_pj_item_of`, dernière fonction publique hors du module à nommer `pico8_value_t`, disparaisse.
11. En tant que mainteneur du port, je veux que les items d'`ou` gardent les octets de `pico8_pj_item_of` (`is_fixed` pour un nombre, `is_ref` NULL pour nil, chaîne empruntée), afin que `test_pico8_fm` et `fold_ou_heads` voient les mêmes items.
12. En tant que mainteneur du port, je veux que `pico8_value_tostr` reste jusqu'à D.19c avec le test de la couche, afin de ne pas éditer maintenant un fichier que D.19c supprime entier.
13. En tant que gardien des tests, je veux que `LIFE_HASH`, `SHARED_HASH`, `BUILT_HASH`, les constantes de `fm`, `lq`, `fv`, `mh`, `ga` et toute constante restent égales, afin de prouver que pixels et items n'ont pas changé.
14. En tant que gardien des tests, je veux une mutation exigée sur TROU-13 qui rougisse un TU (sinon une assertion ajoutée sur la case de `s[0]`), afin que l'écart soit fixé par un test rapide et plus seulement par les e2e.
15. En tant que gardien des tests, je veux des mutations exigées sur le formatage des nombres de `io`, sur la lecture `fixed`/`n` et sur le nil de `jk`, afin qu'on ne puisse pas casser les lectures typées sans rougir.
16. En tant que gardien des filets, je veux `make test`, `make e2e`, `make sanitize-net` et le balayage ASan+UBSan verts, afin de prouver qu'aucun pixel ni accès mémoire ne change quand la chaîne de `s[i]` change de propriétaire.
17. En tant que gardien des tests, je veux `gcov` avant/après et des mutations restaurées par `cp`, afin que chaque ligne modifiée soit exécutée (ou sa non-exécution justifiée par les données) et qu'aucune ligne survivante ne cesse de l'être.
18. En tant que gardien du compteur, je veux au message du commit les comptes par catégorie et par fichier, en lignes et occurrences, la classification (a1)/(a2)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris), le contrefactuel des locales de `pi.c` et les jointures non comptées, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
19. En tant que lecteur du glossaire, je veux que l'entrée `tostr(v)` et celle de `pico8_value_ref(NULL)` ne nomment plus `pico8_value_tostr` ni TROU-13, afin qu'elles restent vraies.
20. En tant que `plan-pilot`, je veux une étape d'environ quinze minutes, un plafond, un repli, une note « Découvert à D.18c » et une prévision chiffrée pour D.19a/b/c, afin que D.19 parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules, tests et découpage ; ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, orphelins du diff retirés, critères vérifiables) et marqués **(G)** ; les rejets sont notés. Q12 (oui) et Q13 (A) sont des décisions humaines ; l'étude D.14 (B10, B11, B8 pour `ou[]`, G1–G4) fait foi sauf écart déclaré ici : quatre aides de lecture et non trois ; `spr_call` fondue ; `pico8_value_tostr` gardée jusqu'à D.19c mais ses marqueurs TROU-13 retirés ; B11 compte 11 lignes (D.18a a pris `:181`) ; aucun TU ne suit par symbole.

### Modules

- **Articles de `bl`** : `pico8_pi.c`.
- **Panneau de score** : `pico8_score_panel.c/.h` (`ou`, `pico8_pj_item_of`), `pico8_score_panel_display.c` (`s[i]`).
- **Module valeur** : `pico8_value.c/.h`, deux marqueurs retirés, rien d'autre.
- **Docs** : `plan-3.md` (note), `trous-de-portage.md` (TROU-13), `glossaire-noms.md` (`:294`, `:657`).
- Inchangés : `Makefile`, `tools/`, `pico8_pi.h`, `pico8_jb_item_t`/`pico8_jb_row_t`, `pico8_pj_ou_t`, `pico8_ky_t`, `pico8_lq_*`, `pico8_str.c/.h`, les TU (sauf assertion ajoutée), les §6, `tests/e2e/`.

### Représentation (G)

- Aucun type nouveau, aucune union : l'item généré est déjà l'union (étude).
- `pico8_pi_arg(e, i)` : `const pico8_jb_item_t *`, NULL si `i > e->count` ; `assert(item->row == NULL …)` gardé.
- `pico8_pi_is_number(e, i)` : `item != NULL && item->s == NULL && !item->is_false && !item->is_ref`.
- `pico8_pi_number(e, i)` : le nombre si `pico8_pi_is_number`, sinon 0 (contrat écrit : « 0 when `e[i]` is not a number »).
- `pico8_pi_ref(e, i)` : `const void *`, `item->ref` si `item != NULL && item->s == NULL && !item->is_false && item->is_ref`, sinon NULL.
- `pico8_pj_ph_s_str(s, i)` : chaîne possédée, jamais NULL.
- **Piège de motif** : aucun nom neuf ne correspond à un motif (`pico8_pi_number` etc. : ni `valeur`, ni `spec` — pas de `_default` —, ni `par-nom` — pas de `_take_`, pas de `field` —, ni `liste`) ; `pico8_io_default`, `pico8_spr_default` (déplacé par la fusion de `spr_call`) sont exclus de `spec` par `SPEC_EXCLUSION`.
- **Piège de forme** : une union locale « genre + contenu » dans `pi.c`, ou une aide item → valeur, serait une cale ; interdit.

### Anti-tassement et message de commit (liant)

- **Interdit** : joindre des lignes comptées (aucune jointure de lignes comptées n'est prévue ; contrefactuel des locales ci-dessus) ; renommer un identifiant vivant pour échapper au motif (`s_at` → `s_str` : contrat changé, nom de l'étude, déclaré) ; alias (`#define`, `typedef`) ; cast nouveau (les trois casts de `pi.c` retirés ne sont pas remplacés ; les littéraux composés ne sont pas des casts) ; cale ou pont ; toucher une constante de hachage, `tests/e2e/`, un `*.sha256`, `tools/`, le `Makefile`, un include hors de celui listé.
- **Classification exigée** (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif en regard des `+`) : **(a1) 36, (a2) 21, (b) 2, (c) 0**, recomptée au parent.
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et par fichier du diff, en lignes (`--detail`) **et** occurrences.
- **Lignes qui gagnent une catégorie**, par fichier et catégorie, « aucune » compris : attendu **aucune**.
- **Lignes comptées modifiées, non retirées** : attendu **aucune** (`score_panel.h:12`, `pico8_value.h:68` inchangées).
- **Δlignes > Δoccurrences** : attendu aucun fichier.
- **Lignes prises à une autre étape** : aucune. (B11 a déjà cédé `:181` à D.18a.)
- **Numéros de ligne** : ceux du parent.

### Empreinte et découpage (G)

| Commit | `src/` | `tests/` | docs | Chemins |
|---|---|---|---|---|
| **D.18c** | `pi.c`, `score_panel.c`, `score_panel.h`, `score_panel_display.c`, `pico8_value.c`, `pico8_value.h` (6) | aucun (ou `test_pico8_pj` si assertion) | `plan-3.md`, `trous-de-portage.md`, `glossaire-noms.md` | 9 (10) |

(L'étude disait ≈ 6 fichiers et TU `pj` : conforme, plus les docs et les deux marqueurs du module.)

- **Rythme mesuré** : D.18a, 133 lignes, ≈ 10 min de la mise en file au commit, gradée à ≈ 18 min ; D.18b : budget écrit ≈ 15 min (réel à relever, H-D18b-9). **D.18c** : éditions ≈ 5 (`pi.c` réécrit en cinq fonctions et trois aides, `score_panel*` mécaniques, docs) ; `gcov` départ/arrivée ≈ 3 ; mutations (≈ 12) ≈ 3 ; filets (`sanitize-net`, balayage ASan+UBSan) ≈ 3 ; mesures, classification, message, note ≈ 2 : **budget ≈ 15 min jusqu'au commit**, **plafond 50 min**.
- **Repli écrit** (G) : au plafond sans commit, rien n'est commis et `plan-pilot` relance en **D.18c1** (`pi.c` seul, B10, 32 lignes / 37 occ.) puis **D.18c2** (`score_panel*`, B11, `ou[]`, marqueurs, docs ; 27 lignes / 31 occ.). Les deux blocs sont indépendants : **aucun pont**. Comptes recomptés par commit. Rejeté comme plan principal : deux coûts fixes pour 59 lignes.

### Effet sur `make data-residue`

| Catégorie | Départ (H-D18b-1) | Après D.18c |
|---|---|---|
| `valeur` | 494 / 78 ; 547 occ. | **435 / 75 ; 479 occ.** (− 59, − 68) |
| `valeur` `src/` | 140 / 40 ; 165 occ. | **81 / 37 ; 97 occ.** |
| `valeur` `tests/` | 354 / 38 ; 382 occ. | 354 / 38 ; 382 occ. |
| `type-tag` | 241 / 57 | **241 / 57** |
| résidu D | 733 / 99 | **674 / 97** |
| `DATA_RESIDUE_GUARD` | inchangé | inchangé |

Par fichier (lignes / occ., départ → après) : `pi.c` 32/37 → 0/0 ; `score_panel.c` 15/19 → 0/0 ; `score_panel.h` 2/2 → 1/1 ; `score_panel_display.c` 11/11 → 0/0 ; `pico8_value.c` 23/30, `pico8_value.h` 22/30 inchangés.

Fonctions publiques hors du module qui nomment `pico8_value_t` : 1 → **0**. Appelants de `pico8_value_*` dans `src/` hors du module : **aucun** (grep 3).

« − 59 / − 68 », « `type-tag` inchangé » (depuis le départ réel) sont des critères (écart = explication chiffrée au message).

### Prévision pour D.19a/b et D.19c (mesurée à `b4bfbfa`, corrigée de D.18b et D.18c)

**Après D.18c : `valeur` 435 lignes / 479 occ. — `src/` 81 / 37 fichiers / 97 occ., `tests/` 354 / 38 / 382.**

`src/` (81 lignes), tout pour D.19c :

- le module : `pico8_value.c` 23 / 30, `pico8_value.h` 22 / 30 — **45 lignes**, deux fichiers supprimés (avec `pico8_value_tostr`, `pico8_value_equal`, le cas `REF` qui rendait TROU-13, `pico8_strdup_owned`) ;
- **33 includes** : 3 `.c` (`bootstrap.c:12`, `cart.c:42`, `scroll.c:11`) et 30 en-têtes (`audio`, `bg`, `cart`, `collect`, `economy_formulas`, `entity`, `entity_geom`, `help`, `ke`, `layout`, `mouse_reader`, `msgbox`, `particle`, `planet`, `planet_display`, `planet_family`, `planet_family_trade`, `planet_trade`, `probe`, `research`, `research_display`, `research_icon`, `score_panel`, `scroll`, `slipway`, `tech_gate`, `trade`, `ui`, `ui_button`, `wormhole`) — à retirer en vérifiant que chaque inclusion transitive (`pico8_fixed.h` surtout) reste fournie ;
- **3 commentaires** : `geom.h:37-38` (« a PICO8_VALUE_REF », « pico8_value.h's borrowed-pointer convention »), `type_tag.h:10` (ce dernier part avec D.20b s'il n'est pas réécrit avant).

`tests/` (354 lignes), par groupe de l'étude :

| Groupe | TU | Lignes (occ.) | dont includes | Plis et aides |
|---|---|---|---|---|
| D.19a (famille planète) | `fr` 15, `fv` 16, `g_` 16, `ga` 28, `hj` 8, `jo` 12, `mx` 7, `nl` 4, `nr` 17, `os` 4, `pm` 6, `qh` 5 | 138 (138) | 12 | 126 |
| D.19b (autres) | `hp` 34, `lq` 28, `jc` 26, `pj` 17, `js` 16, `hy` 15, `mo` 7, `iv` 6, `mz` 6, `bg` 5, `trade` 4, `ui` 4, `entity_kind` 3, `fq` 3, `ie` 3, `mh` 3, et 9 TU à include seul (`ga_export_panel_by`, `jb_ol`, `je`, `layout_nil_item_draws_nothing`, `nf`, `planet_gh_row_lq`, `planet_kj_dispatch`, `probe_mm_rnd_order`, `qb`) | 189 (192) | 25 | 164 |
| D.19c | `test_pico8_value` (test de la couche, supprimé) | 27 (52) | 1 | — |

Deux lectures de D.19a/b, que leurs PRD trancheront :

- **les includes des TU partent avec leur dernier usage** (règle de D.15b appliquée aux TU) : D.19a − 138, D.19b − 180 (164 plis + les 16 includes des TU pliés) → **≈ 117 après D.19b** (`src/` 81, `tests/` 36 : 9 includes seuls, `test_pico8_value` 27) — c'est le ≈ 115 du plan ;
- **les includes des TU attendent D.19c** (précédent de D.18b : « includes `pico8_value.h` des TU gardés ») : D.19a − 126, D.19b − 164 → **≈ 145 après D.19b** (`src/` 81, `tests/` 64 : 37 includes, `test_pico8_value` 27).

D.19c retire alors, dans `src/`, les 81 lignes ci-dessus (45 du module, 33 includes, 3 commentaires), et dans `tests/` `test_pico8_value.c` et les includes restants (36 ou 64 lignes) ; avec le `Makefile` (règle `test_pico8_value`, `src/pico8_value.c` de `DATA_ENGINE_SRCS`, `ENGINE_SRCS`, de la ligne de `test_pico8_layout_nil_item_draws_nothing` et de `main_raylib`), `glossaire-noms.md` (`:294`, `:657`), TROU-3 (qui nomme `pico8_value_equal`, `src/pico8_value.c`), TROU-4 (`pico8_value_ref(NULL)`, l. 72). Le compte à D.19a/b de l'étude (≈ 260, ≈ 115) supposait 12 lignes de plus à `src/` ; l'écart est dans le sens favorable.

### Documentation (G)

- `docs/plan-3.md` : une note « Découvert à D.18c » en fin de ligne D.18c, rien d'autre. Elle dit : le commit ; les comptes réels et l'écart à ≈ 435 ; les lectures (quatre aides de `pi.c`, `spr_call` fondue, `pico8_pj_ph_s_str` et sa propriété, `ou` en items aux octets de `pico8_pj_item_of`, `pico8_value_tostr` gardée mais TROU-13 déplacé) ; les assertions ajoutées s'il y en a ; **pour D.19a/b** : la prévision ci-dessus (et laquelle des deux lectures des includes le PRD de D.19a doit trancher) ; **pour D.19c** : `src/` n'a plus que le module, 33 includes et 3 commentaires ; `pico8_value_tostr` et `pico8_value_equal` n'ont plus que `test_pico8_value` ; `score_panel.h:12` n'a plus que son include ; TROU-3 et TROU-4 nomment encore le module.
- `docs/trous-de-portage.md`, **TROU-13** (l. 190-205), réécrit (G3), titre et premier paragraphe inchangés :
  > Ce port n'a pas de nil à mettre en chaîne : `pico8_pj_ph_s_str` rend `""` pour tout indice hors de 1..6. PICO-8, lui, a une représentation dédiée pour nil dans `tostr` — donc la ligne ne s'affiche pas pareil.
  >
  > Fixé par `<test>` de `test_pico8_pj` (si une assertion est ajoutée ou si une assertion existante rougit sous la mutation M1 ; sinon : « Atteint par le filet : `game_end_final_score`, `game_end_bankrupt` »).
  >
  > Fichier : `src/pico8_score_panel_display.c`.

  Le paragraphe « Corollaire » (sur le commentaire de `pico8_value_tostr` et ses branches exercées « à chaque rafraîchissement ») **part** : après D.18c, aucune branche `REF` de `pico8_value_tostr` n'est exercée par `src/`, son affirmation devient fausse. (D.10b avait refusé de le réécrire parce qu'il était encore vrai.)
- `docs/glossaire-noms.md:294` (`tostr(v)`) : « `pico8_tostr_fixed` (`pico8_str.h`) pour un nombre ; `s[i]` du panneau de fin par `pico8_pj_ph_s_str` (chaîne possédée, `""` hors de 1..6 : TROU-13) ». `:657` (`pico8_value_ref(NULL)`) : « C'est lui qui fait exister TROU-11 » (TROU-13 retiré de la phrase). Rien d'autre (les entrées `pi`, `bl`, `ou`, `pico8_jb_item_t` restent vraies).
- Commentaires de `src/` : quatre sortes de `CLAUDE.md` ; contrats réécrits (`pico8_pi_arg` `:56-57`, `pico8_pj_ph_s_str` `:126`), contrats neufs (`pico8_pi_number` : « 0 when … not a number » ; `pico8_pi_ref` : « NULL when … not a pointer ») ; piège gardé (`io` `:96-98`, `spr` « w/h both or neither ») ; commentaire `score_panel_display.c:177-179` retiré (son objet est TROU-13, marqué nu) ; contrat `score_panel.h:120-121` retiré avec la déclaration ; aucun renvoi à D.18c ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile` ; aucun e2e ajouté ou supprimé ; toute constante `*_HASH`/`identity[]` ; `tools/class_coverage_net_expected.txt`.
- Le comportement : mêmes pixels pour chaque article (`br`, `qr`, `rf`, `s`, `ld`) ; mêmes items d'`ou` (octets de `pico8_pj_item_of`) ; même chaîne dans `st->items[2]` ; mêmes durées de vie (TROU-7 : la chaîne de `s[i]` n'est toujours pas libérée, `owned` de `io` l'est toujours) ; TROU-13 et TROU-25 préservés ; `make e2e` conforme au bit près.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : sous `-Wall -Wextra -Werror`, `const void *` s'affecte à `const pico8_lq_t *` et `const pico8_vec_t *` sans cast ; les aides `static` neuves sont toutes appelées ; `pico8_pi_spr_call` retirée n'est plus appelée.
- **H2** : aucun autre écrivain ni lecteur de `pico8_pj_item_of`, `pico8_pi_arg`, `pico8_pj_ph_s_at` n'existe (grep à `b4bfbfa` : `src/`, `tests/`, `tools/`, glossaire, TROU) ; un oublié se révèle à la compilation.
- **H3** : les producteurs de la table « Preuve B10 » sont tous les producteurs d'articles ; si ASan/UBSan ou `gcov` montre un article `br`/`rf`/`s` lu en position non numérique, H3 est fausse : arrêt, rapporté.
- **H4** : `pico8_tostr_fixed` rend, pour les nombres de `io` et de `s[i]`, la même chaîne que `pico8_value_tostr` (qui lui délègue depuis D.15c) ; `pj_concat2(x, "")` rend une copie de `x`.
- **H5** : `test_pico8_pj` (`fold_ou_heads`, `LIFE_HASH`, `test_ph_rows_show_ky`), `test_pico8_fm` (`sub_items`), `test_pico8_lq` (`ld` par `bl_default`) jugent les trois blocs.

## Implementation Chunks

**D.18c** (un commit)

1. Mesure de départ (graduation de D.18b ou un descendant `docs/` seul : H-D18b-1) ; remesure par contenu des numéros ; `gcov` de départ.
2. `pi.c` : `pico8_pi_arg`, trois lectures, `rk`, `io` (+ include), `eq`, `spr` (fusion), `ld`.
3. `score_panel.c/.h` : `ou`, `fm_jk`, `fm_yr`, retrait de `pico8_pj_item_of`.
4. `score_panel_display.c` : `pico8_pj_ph_s_str`, TROU-13 au site ; `pico8_value.c/.h` : marqueurs retirés.
5. `make tmp/test_pico8_pj tmp/test_pico8_fm tmp/test_pico8_lq` et lancement de chacun.
6. Docs : TROU-13, glossaire.
7. `make test`, `make e2e`, `make sanitize-net`, balayage ASan+UBSan ; `gcov` d'arrivée ; mutations (assertion ajoutée si M1 ou M3 l'exige) ; mesures, classification ; note « Découvert à D.18c » ; **commit**, vérification, push.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (pixels, items écrits, cases de rangée), pas la présence d'une ligne. D.18c change la représentation intermédiaire sans changer un pixel ni un item pour les genres que `src/` écrit. La preuve : les TU existants, qui plient items et écrans dans des constantes inchangées et assertent des items (`test_pico8_fm:104,108`, `test_ph_rows_show_ky`) ; les 26 e2e au bit près (`game_end_final_score`, `game_end_bankrupt`, `message_box_hold`… dessinent `ou`, `bz`, `ld`) ; `sanitize-net` ; le balayage ASan+UBSan ; `gcov` et mutations.

### TU touchés (G)

1. Suivent : **aucun**.
2. Prouvent sans suivre : `pj` (`fold_ou_heads`, `BUILT/SHARED/LIFE/CR_HASH`, `test_ph_rows_show_ky`), `fm` (items d'`ou` du `fm` provisoire), `lq` (`ld` par `pico8_bl_default`, `:600-612`), `fv` (articles `s`, `qr` de l'aide), `mh` (articles d'une boîte), `ga` (écran).
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change ; une constante qui rougit est un bug de conversion.
4. **Assertion nouvelle** : seulement pour faire rougir une mutation exigée qui ne rougit aucun TU (M1, M3), dans `test_ph_rows_show_ky` de `test_pico8_pj` : la case de `s[0]` de la rangée « score 0 » (`ph_row_item(mh, 5, 2)` attendu ; TROU-25 resserre les indices 1 et 3 en deux cases — à confirmer à l'exécution) est une chaîne vide ; la case 3 de la rangée « score 4 » (`ph_row_item(mh, 9, 3)` attendu) est `"1%"`. Rien de lent, aucun fichier d'événements ; listées au message ; TROU-13 nomme alors ce test.
5. **Prior art** : lectures typées d'un item généré (`pico8_lq_cell_item`, `layout.c:43-60`, D.18a) ; items écrits par littéraux composés (`score_panel_display.c:228-230`, `pico8_pj_item_of`) ; fonction fondue avec son lecteur (étude, B11) ; fonctions `static` retirées avec leur dernier appel (D.15b–D.18b) ; TROU marqué au site (TROU-23 dans `pq`, D.18b) ; fonction publique sans appelant de `src/` gardée pour D.19c (`pico8_value_equal`, D.17).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`), **les TU qui forkent** (`in_fresh_child`, dont `pj`, `fm`, `lq` ; sortie par `_exit`) compilés en plus avec **`-D_exit=exit`** ; lancés comme le `Makefile` (depuis la racine, `ASAN_OPTIONS=detect_leaks=0` pour `test_pico8_planet_gh_row_lq`) ; union des lignes de `src/` exécutées, départ et arrivée.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon justifiée au message. Attendus : les trois lectures (tous genres : chaîne, faux initial d'`ou`, nombre `n` et `is_fixed`, pointeur, au-delà de la fin — `fv_rb_3-4` et `pj_ou_*` ont 4 items, `io` lit a5) ; `io` nombre (`jk`, `yr` d'`ou` : `pj`) ; `ld` (`lq:612`, `pj` `ph`) ; `ou` nil (`fm` provisoire : `jk`, `yr`, `ef`) ; `s_str` cas 1–6 et `default` (`pj` : `test_life`, `test_ph_rows_show_ky` ; score 0 atteint par tout `ph`). **Justifiées d'avance** (données de `src/`, preuve B10) : `pico8_rk_y1` (`:92`, aucun `br` à moins de deux nombres) et `pico8_spr_default` (aucun `s` sans `w`/`h`), lignes modifiées seulement par le nom de leurs arguments.
- **`L`** = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : attendu **seulement dans `pico8_value.c`** — la branche `REF` de `pico8_value_tostr` (`:69-71`, `test_pico8_value` ne la teste pas, surprise (9) de D.15c) et toute ligne de constructeur que seuls `pi.c`/`score_panel*.c` atteignaient ; justifiées par « sans appelant de `src/`, D.19c ». Toute autre ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, **dans une copie jetable** (ou dans l'arbre de travail : fichier sauvegardé par `cp` avant, restauré par `cp` simple puis `touch`, jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance ; chacune doit rougir au moins le TU nommé, sinon trou couvert par une assertion ou justifié (mutation équivalente démontrée). **Astuces** : neutraliser par `0 ? x : …`, `0 &&`, une condition toujours fausse, plutôt que supprimer (`-Werror`) ; si un `assert` de `src/` attrape la mutation avant le TU, relancer avec cet `assert` relâché dans la copie mutée, et rapporter les deux.

- **M1 (exigée) — TROU-13** : le `default` de `pico8_pj_ph_s_str` rend `pj_concat2("[nil]", "")` → **un TU rouge** (`pj` : `LIFE_HASH` si un écran plie la boîte de fin, sinon l'assertion ajoutée sur la case de `s[0]`) ; e2e `game_end_*` rouges attendus.
- **M2 (exigée) — nombre de `io`** : `owned` neutralisé (`0 ? … : NULL`) → **`pj` rouge** (`LIFE_HASH` ou `SHARED_HASH` : l'année et `jk` d'`ou` blanchis, le piège de `:96-98`).
- **M3 (exigée) — copie de `happiness`** : le cas 4 rend `pj_concat2("", "")` → **un TU rouge** (`pj`, sinon l'assertion ajoutée sur la rangée « score 4 »).
- **M4 (exigée) — nil de `jk`** : `fm_jk = {.is_false = 1}` à la place de `{.is_ref = 1}` → **`fm` rouge** (`:104`, `sub_items[2][0].is_ref`).
- **M5 (exigée) — `fixed` contre `n`** : `pico8_pi_number` rend `pico8_fixed_from_int(item->n)` même quand `is_fixed` → **`lq` rouge** (`:612`, `ld` à `a = 0.5` : `screen_hash() == want`), `pj` possible (`gs`/`ea` à `0.5`). **Si M1, M2, M3, M4 ou M5 ne rougit pas un TU (assertion ajoutée comprise) : `FAILED:`.**
- **M6** `pico8_pi_is_number` sans `!item->is_false` (le `false` initial d'`ou` devient « 0 ») → `pj` (écran avant le premier `hb`, si plié) sinon e2e, sinon justification.
- **M7** `pico8_pi_is_number` sans `!item->is_ref` (un nil d'`ou` devient « 0 ») → `fm`/`pj` si un écran plie un `ou` nil, sinon e2e, sinon justification (nil d'`ou` seulement sur des chemins où le cart planterait).
- **M8** `pico8_pi_ref` rend toujours NULL → `lq` (`:612`), `pj` (boîte de fin).
- **M9** `ld` : `at` ignoré (`p` à `(0, 0)`) → `lq` (`:612`).
- **M10** `rk` : `pico8_pi_is_number(e, 2)` neutralisé (toujours `rk_y1`) → `pj` (cadre `i_`, `BUILT_HASH`/`LIFE_HASH`).
- **M11** `eq` : `pico8_pi_number(e, 3)` → `(e, 2)` → `pj` (cadres `rf` de `i_`).
- **M12** `spr` : condition `w`/`h` neutralisée → `fv` (articles `s` de l'aide), sinon e2e.
- **M13** `ou` : `ou[i]` écrit à `sub_items[i][1]` au lieu de `[0]` → `pj` (`fold_ou_heads`), `fm`.
- **M14** `ou[5]` : la branche nil toujours prise → `pj` (`fold_ou_heads`).

Cibles (numéros de `b4bfbfa`) : `pi.c:56-157` ; `score_panel.c:144-232` ; `score_panel_display.c:126-181`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes). Aucun e2e ajouté.
- **`make sanitize-net`** (exigé par la ligne) : propre.
- **Balayage ASan+UBSan des TU** : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`.
- `make class-coverage-net` : non exigé (aucun constructeur touché) ; s'il est lancé, rapporté.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 15 min de temps réel jusqu'au commit**, mesures, `gcov`, mutations et filets compris ; **plafond 50 min** (au-delà : repli D.18c1/D.18c2). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_pj tmp/test_pico8_fm tmp/test_pico8_lq` + lancement | quelques secondes |
| `make test` | de l'ordre de la minute (`score_panel.h` change : recompilation large) |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 1 min |
| `make line-coverage EVENTS="tests/e2e/*.json"` | < 1 min |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes |
| mutations (M1–M14) | < 6 min |
| balayage ASan+UBSan des TU | quelques minutes |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; rien d'inachevé n'est commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »**, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + état) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | graduation de D.18b, avant D.18c | 0 | `valeur 494 78`, `type-tag 241 57` (H-D18b-1) | … |
| 2 | recette `valeur` | idem | 0 | `valeur 494 lignes 78 fichiers 547 occ.` | … |
| 3 | `grep -cE "$P" src/pico8_pi.c src/pico8_score_panel.c src/pico8_score_panel.h src/pico8_score_panel_display.c` | idem | 0 | `32`, `15`, `2`, `11` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les 9 chemins de « Empreinte » (plus `tests/test_pico8_pj.c` si une assertion est ajoutée, listée au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`, ni un §6.
2. **Greps** de la Sortie, exactement.
3. **Formes** : `pico8_pi_arg` en `const pico8_jb_item_t *` ; trois lectures `static` ; `io` par `pico8_tostr_fixed` ; `spr_call` fondue ; `pico8_pj_ph_s_str` possédante, un marqueur TROU-13 nu dans sa branche hors bornes ; `ou` en `pico8_jb_item_t` aux octets de `pico8_pj_item_of` ; `pico8_value_tostr` présente, sans marqueur.
4. **Casts** : aucun nouveau (`grep -nE '\(\s*(const\s+)?[a-z0-9_]+_t\s*\**\s*\)\s*[^{ ]'` et `grep -nE '\(\s*(const\s+)?void\s*\*\s*\)'` sur les lignes `+` du diff : vides) ; trois casts de `pi.c` retirés.
5. **API** : seule `pico8_pj_item_of` disparaît de l'API publique ; aucun symbole public nouveau.
6. **Liens** : grep 6.
7. **Constantes** : grep 7 vide ; M1–M5 rapportées.
8. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste ; aucune ligne gagnée ; contrefactuel des locales et jointures non comptées déclarés ; includes (grep 8).
9. **Écarts déclarés** : quatre aides et non trois ; `spr_call` fondue ; `pico8_value_tostr` gardée, marqueurs retirés ; corollaire de TROU-13 retiré ; B11 à 11 lignes ; aucun TU ne suit.
10. **Preuve** : `gcov` (lignes modifiées exécutées ou justifiées ; `L` limité à `pico8_value.c`), mutations restaurées, `sanitize-net` et ASan+UBSan rapportés.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.18c ni au plan dans `src/` ou `tests/`.
12. **Docs** : note « Découvert à D.18c » en fin de ligne D.18c, rien d'autre dans le plan ; TROU-13 et glossaire (`:294`, `:657`) réécrits.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas la graduation de D.18b (ou un descendant qui ne touche que `docs/`), propre ; ou si `valeur` n'y vaut pas 494 / 78 / 547 occ., `type-tag` 241 / 57, résidu D 733 / 99, sans explication (ces valeurs sont remises à la graduation de D.18b).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change ; si un site ne tombe sous aucune règle de ce PRD ni de l'étude (arrêt « champ sans règle ») — en particulier un article lu en position non numérique (H3) ; ou si un écart port/cart découvert n'est pas préservé et décrit.
- `FAILED:` si `make sanitize-net` ou le balayage ASan+UBSan des TU n'est pas propre.

**Périmètre et forme**

- `FAILED:` si `tools/` ou le `Makefile` change, ou si un include change hors de celui listé ; si le commit touche un chemin hors de son « Empreinte » sans le lister.
- `FAILED:` si une grep de la Sortie ne rend pas ce qui y est écrit.
- `FAILED:` si une fonction publique autre que `pico8_pj_item_of` change ou disparaît ; si `pico8_jb_item_t`, `pico8_pj_ou_t`, `pico8_ky_t`, `pico8_pi.h`, `pico8_value_tostr` ou `pico8_value_equal` changent ; si une durée de vie change.
- `FAILED:` si TROU-13 n'est plus préservé à l'identique (`""` pour `s[0]`), ou si TROU-25 change.
- `FAILED:` si un cast nouveau est ajouté, si des lignes comptées sont jointes, ou si une cale valeur ↔ typé, une union locale « genre + contenu » ou un pont apparaît.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne baisse pas de **59 lignes / 68 occ.** exactement depuis le départ réel, sans explication chiffrée ligne à ligne ; si `type-tag` change.
- `FAILED:` **si un compte baisse par tassement** : lignes comptées jointes, alias, renommage d'un identifiant vivant pour le motif, commentaire compté réécrit hors de son sujet, cale ou pont, (c) non nulle, (b) hors de `pico8_pj_item_of`.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; les lignes qui gagnent une catégorie (« aucune » compris) ; la justification de chaque Δlignes > Δoccurrences (attendu : aucune) ; le contrefactuel des locales de `pi.c` et les jointures/coupes non comptées ; les fonctions retirées et leurs lecteurs (`pico8_pj_item_of`, `pico8_pi_spr_call`) ; la table de vérité de `pi.c` ; la décision sur `pico8_value_tostr` ; le rapport `gcov` ; les mutations (M1–M5 en tête) ; `sanitize-net`, ASan+UBSan ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne modifiée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` contient une ligne hors de `pico8_value.c`.
- `FAILED:` si une mutation manque, ne se restaure pas, ou ne rougit rien sans justification ; **si M1, M2, M3, M4 ou M5 ne rougit pas un TU**.
- `FAILED:` si le plafond est dépassé sans note (et sans repli), ou s'il manque un tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.18c », ou si elle omet : les comptes réels, les lectures, les assertions ajoutées, la prévision pour D.19a/b, les legs pour D.19c.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`, ou si le marqueur TROU-13 n'est pas dans `src/pico8_score_panel_display.c`.

## Out of Scope

- Supprimer `pico8_value_tostr`, `pico8_value_equal`, le module, `test_pico8_value`, les includes des en-têtes et des TU : D.19c.
- Les plis `fold_value`/`item_value` de `test_pico8_pj` et des autres TU : D.19a/b.
- Modéliser le `pairs` de `pj:hb` qui saute les nils (constat, Further Notes) ; corriger TROU-13 ou TROU-25 (interdit par le plan).
- TROU-3, TROU-4 (nomment le module, D.19c).
- Les lectures d'étiquette (`score_panel.c:67`, `:117` ; `test_pico8_pj`) : D.20.
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Découpage** : un commit ; repli D.18c1 (`pi.c`) / D.18c2 (`score_panel*`, docs), sans pont.
  - **Lectures de `pi.c`** : `pico8_pi_arg` + prédicat + nombre ou 0 + pointeur ; la chaîne lue au site ; `spr_call` fondue.
  - **`s[i]`** : `pico8_pj_ph_s_str` possédante dans toutes ses branches, copies par `pj_concat2` existante ; TROU-13 marqué nu à la branche hors bornes.
  - **`ou`** : littéraux composés aux octets de `pico8_pj_item_of`.
  - **`pico8_value_tostr`** : gardée pour D.19c (précédent `pico8_value_equal`), marqueurs TROU-13 retirés, corollaire de TROU-13 retiré.
  - **Tests** : aucun TU ne suit ; assertions seulement pour M1/M3 si nécessaire.
  - **Drapeaux** : `sanitize-net` (ligne) ; balayage ASan+UBSan (consigne) ; `class-coverage-net` non exigé.
- **Surprises de mesure, pour `plan-pilot`** : (1) **aucun TU ne nomme un symbole de D.18c** : l'étude disait « TU `pj` », mais `pico8_pj_item_of` n'a aucun appelant de TU et les lignes `valeur` de `test_pico8_pj` sont des plis (D.19b) ; (2) **B11 compte 11 lignes et non 12** (D.18a a pris `:181`) ; (3) **arrivée 435, la cible du plan exactement** ; (4) **`pi.c` lit aujourd'hui `.as.fixed` sans test de genre** en neuf positions (`rk` a1, `eq`, `spr` a1–a3) : sans conséquence parce qu'aucun producteur n'y met autre chose qu'un nombre (table B10), mais c'est pourquoi la lecture « nombre ou 0 » est équivalente ; (5) **`pico8_rk_y1` et `pico8_spr_default` ne sont atteintes par aucune donnée de `src/`** ; (6) **le `pairs` de `pj:hb` saute les nils** (lua.lua:848-850) alors que le port écrit nil dans `pj.ou[i][1]` : écart non documenté, atteint seulement quand le cart planterait avant (`fm` provisoire) ou par `ef[fm.lo]` hors bornes (à vérifier : `fm.lo` avance de `hx`, peut-être non entier, et le port lit `flr(lo)` là où le cart indexe `ef[fm.lo]`) ; préservé, rien à faire ici, **candidat à une entrée TROU** par une étape qui en fait la preuve ; (7) **la branche `REF` de `pico8_value_tostr` n'est plus atteinte par `src/`** : TROU-13 quitte le module, son corollaire devient faux ; (8) après D.18c, **`src/` n'appelle plus rien du module** : D.19c n'a plus que 45 lignes du module, 33 includes et 3 commentaires à retirer de `src/`.
- **Pour D.19a/b** : voir « Prévision » ; le PRD de D.19a tranche si les includes des TU partent avec leur dernier pli (≈ 117 après D.19b, le ≈ 115 du plan) ou attendent D.19c (≈ 145).
- **Pour D.19c** : `pico8_value_tostr` et `pico8_value_equal` n'ont plus que `test_pico8_value` ; `score_panel.h:12`, `ui_button.h:13` et 28 autres en-têtes n'ont plus que leur include ; `geom.h:37-38`, `type_tag.h:10` nomment le module ; TROU-3, TROU-4, glossaire `:657` aussi.

## Annexe A — textes recopiés (`b4bfbfa`, par extraction de lignes)

### Lignes D.18b et D.18c (`docs/plan-3.md`, l. 290-291)

> - D.18b — B9 : sources des boutons en champs séparés. `class-coverage-net` : oui. Compte : ≈ 505.
> - D.18c — B10, B11, `ou[]`. `sanitize-net` : oui. Compte : ≈ 435.

(La ligne D.18b portera sa note « Découvert à D.18b » : à réextraire à la graduation, H-D18b-10.)

### TROU-13 (`docs/trous-de-portage.md:190-205`), texte à `b4bfbfa`

> ## TROU-13 — `tostr` d'un champ absent rend la chaîne vide
>
> La rangée « score 0 » du panneau lit `s[st.score]` avec `score == 0`, c'est-à-dire un indice que `ky()` ne pose jamais. Lua y verrait nil.
>
> Ce port n'a pas de nil : il passe son emplissage « absent » (`pico8_value_ref(NULL)`) à `pico8_value_tostr`, qui rend `""`. PICO-8, lui, a une représentation dédiée pour nil dans `tostr` — donc la ligne ne s'affiche pas pareil.
>
> Corollaire : le commentaire de `pico8_value_tostr` affirmait que ses branches `REF`/`FUNCTION` ne sont exercées par aucun site d'appel. C'est faux depuis que le panneau de score existe : elles le sont à chaque rafraîchissement.
>
> Fichiers : `src/pico8_score_panel_display.c`, `src/pico8_value.c`.

### Glossaire (`docs/glossaire-noms.md`), textes à `b4bfbfa`

> `:294` | `tostr(v)` | `pico8_tostr_fixed` (`pico8_str.h`) pour un nombre ; `pico8_value_tostr` pour une valeur à genres mêlés (jusqu'à D.19c) | conversion en chaîne ; un nombre : 4 décimales puis zéros de fin élagués |
>
> `:657` | `pico8_value_ref(NULL)` | l'emplissage « valeur absente ». C'est lui qui fait exister TROU-11 et TROU-13 : il traverse les dispatches et les conversions sans se distinguer d'une vraie valeur |

## Annexe B — reproduire

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
sed -n 24,30p pico8/lua.lua; sed -n 51,53p pico8/lua.lua; sed -n 133p pico8/lua.lua   # rk, eq, io, oo, pi
sed -n 847,850p pico8/lua.lua; sed -n 882,892p pico8/lua.lua; sed -n 1339,1362p pico8/lua.lua  # pj:hb, pj:ph, ky
grep -nE "$P" src/pico8_pi.c src/pico8_score_panel.c src/pico8_score_panel.h src/pico8_score_panel_display.c
grep -rn 'pico8_pj_item_of\|pico8_pi_arg\|pico8_pj_ph_s_at\|pico8_value_tostr' src tests
grep -rn '"br"\|"rf"\|"qr"\|"ld"\|fn = "s"' src/*.c | grep -v dispatch_register
grep -rn 'pico8_bl(\|pico8_bl_default(' src tests
grep -nE 'pico8_value_t' src/*.h | grep -v '^src/pico8_value.h:' | grep -vE ':\s*(\*|/\*)'
sh tools/data_residue.sh --detail | grep -E 'pi\.c|score_panel|pico8_value'
```
