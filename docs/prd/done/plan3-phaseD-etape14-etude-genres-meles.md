# Plan 3, étape D.14 — étude D.9b sans conversion : valeurs à genres mêlés, `valeur`, `type-tag`

Réponse à Q12, tranchée oui par l'humain (2026-09-29) : « fais le tri d'abord ». Le but est de supprimer la couche objet Lua. Aucun code n'est changé, et `docs/plan-3.md` non plus : ce fichier propose les étapes à y ajouter après D.14. L'humain tranche la question de la dernière partie avant la première étape qui l'applique.

**Mesuré sur `b7f6c02`**, arbre propre, avant que D.13 (suppression de `pico8_ref_audit.h`, menée en parallèle) ne touche l'arbre. Seuls `sh tools/data_residue.sh` (avec `--detail`) et des `grep` ont tourné. Rien n'a été compilé ni lancé. Le classement vient de la lecture des sources et, pour les genres, des écritures de `pico8/lua.lua`.

Départ (`make data-residue`, lignes / fichiers) : **`valeur` 1014 / 105** (`src/` 518 / 66, `tests/` 496 / 39) ; **`type-tag` 249 / 59** (`src/` 78 / 25, `tests/` 171 / 34) ; `ref-audit` 23 / 13, hors sujet ici. D.13 retire les 6 lignes `type-tag` de `pico8_ref_audit.h` : `type-tag` passe à **243 / 58** (remesuré en fin d'étude, arbre de D.13 en cours) et quelques numéros de ligne cités ici se décalent d'une ligne dans les fichiers dont D.13 retire l'include.

Unités. Une **ligne** est une ligne comptée par `data_residue.sh`. Le classement est fait ligne à ligne (script en annexe B) ; les comptes par groupe sont exacts pour ce classement. Les comptes par étape (§3) sont des estimations « ≈ », chaque PRD remesure.

**Critère (a) / (b).** Une valeur est **(a) mécanique** si elle ne prend qu'un seul genre non nil, et qu'aucun lecteur ne distingue nil de `false` : elle devient son type C, avec `NULL` ou un drapeau `has_x` pour nil (la règle de 1b). Elle est **(b) à genres mêlés** si elle prend deux genres non nil (nombre et `false`, chaîne et nombre…), ou si un lecteur distingue nil de `false`. Les 13 copies privées de `*_truthy(pico8_value_t)` sont classées (b) : elles n'existent que parce que leur argument mêle les genres, et meurent avec lui.

Ce que Q12 nommait et qui est **déjà réglé** par D.10–D.12 : `js.my` (drapeau `has_my` + pointeur de fonction), les retours de `ldown`/`rdown`/`gv` (struct `stop`… de `pico8_struct.h`). Ils ne comptent plus dans `valeur`.

---

## 1. Inventaire

### 1.1 Vue d'ensemble

| | `src/` | `tests/` | total |
|---|---|---|---|
| **(a) mécanique** | 154 | 406 | **560** |
| **(b) genres mêlés** | 319 | 57 | **376** |
| module lui-même (`pico8_value.c/.h`, `test_pico8_value.c`) | 45 | 33 | 78 |
| **total** | 518 | 496 | **1014** |

Détail de (a), `src/` : 38 includes (dont une vingtaine sans aucun usage), 20 appels `pico8_value_tostr(pico8_value_fixed(x))`, 96 autres lignes (§1.2). Détail de (a), `tests/` : 38 includes, 226 lignes d'aides d'empreinte (`fold_value`, `fold_cell`, `fold_kind`, octets `PICO8_VALUE_*` pliés dans les empreintes), 129 aides locales (`vec_ref`, `nil()`, `mouse_at`, `zero_union`…), 13 appels d'API typées en (a).

### 1.2 (a) — mécanique, 154 lignes de `src/`

| Site | Genre réel (preuve) | Remplacement |
|---|---|---|
| `pico8_ov(px, lk, mc)` — `tech_gate.c:45`, `.h:15` ; appelants `economy.c:90-101,196-240`, `planet.c:435-442`, `slipway.c:271,283-285`, `wormhole.c:76-90,103-117` | `px` toujours un littéral chaîne, `lk`/`mc` toujours des littéraux nombre (lua.lua:1467, `ov("space folding",0.5,1.08)`…) ; rend donc un nombre | `pico8_fixed_t pico8_ov(const char *px, pico8_fixed_t lk, pico8_fixed_t mc)` ; les replis `kind == FIXED ? … : défaut` des appelants meurent |
| `pico8_cb(bi)` — `tech_gate.c:77,106`, `.h:29` ; `menu.c:177,283` | `true` ou nil (`cb(true)`, `cb()`), lu en vérité seule (lua.lua:1455-1461) | `int bi` |
| `pico8_ij(…, out_nearest)` — `entity_geom.c:96-118`, `.h:10,29,31` ; `galaxy.c:53`, `wormhole.c:74` | nil ou une entité | `void **out_nearest` (NULL : rien) |
| `pico8_jc_ce` — `slipway.c:266-339`, `.h:119` ; `pico8_jc_value_instance` `slipway.c:59-60` | nil, la vue « -too far- » ou une entité (lua.lua:641-652) : toujours une référence | `pico8_struct_header_t *` ; `jc_value_instance` disparaît |
| `pico8_d_` — `probe.c:328-364`, `.h:175` | `true` ou nil (lua.lua:569-575), lu en vérité seule | `int` |
| `pico8_jc_hg_add/has(hg, id)` — `slipway.c:94-108`, `.h:28,31` ; `economy_tiers.c:134`, `slipway_display.c:31` | `src/` ne passe que des chaînes (`planet->id`, `t_id`) | `const char *id` ; les cas nombre et nil de `test_pico8_jc` sont des genres que `src/` ne produit pas (voir G4) |
| `pico8_pf_value_vec`, `pico8_mz_value_vec` — `planet_family.c:27-28,373-374`, `wormhole.c:40-41,84,173` | enveloppent un `pico8_vec_t *` pour le relire | lecture directe du pointeur (NULL : vecteur nul) |
| `g_ie_id` — `research_icon.c:24,127-129,267,297-300` | nil ou nombre (l'`assert` de `:300` le dit déjà) | `pico8_fixed_t` + `has_ie_id` |
| `pico8_value_tostr(pico8_value_fixed(x))` — 20 sites (`collect`, `economy_score`, `galaxy`, `menu` ×6, `planet_display` ×4, `probe`, `research_display` ×2, `research_icon`, `score_panel` ×3, `score_panel_display`) | toujours un nombre | `char *pico8_tostr_fixed(pico8_fixed_t)` dans `pico8_str.c`, à côté de `pico8_sub` (même convention : l'appelant libère) ; TROU-22 et TROU-13 suivent la fonction (G5) |
| `kw.qe` et l'argument `o` de `pico8_jo_ex` — `planet_display.c:38-42,59,220-225,261-264`, `research_display.c:47,57,142,164,194-205,303`, `.h:27` | nil ou nombre (`hp.o`, `kw={qe=hp.o}`, lua.lua:463) | drapeau + nombre ; rangés avec le point `qe` de (b) parce qu'ils passent par la même comparaison |
| commentaires (`geom.h:37-38`, `entity_geom.h:10,29`, `type_tag.h:10`, `collect.c:90`) | — | réécrits ou supprimés avec leur objet |
| 38 `#include "pico8_value.h"` | — | supprimés au fil des étapes ; les derniers avec le module |

### 1.3 (b) — genres mêlés, 319 lignes de `src/` et 57 de `tests/`

Chaque point donne : les sites, les genres, la preuve, la règle C retenue et sa raison. « (G) » : tranché ici, la lecture du code suffit.

**B1. `hf`, `qh` et `jo:gg` — 18 lignes.** `planet.c:348-368` (`pico8_jo_hf`, `pico8_jo_gg`), `planet.h:137,142`, `planet_family.c:38-45` (`pico8_qh_qh`), `.h:46`, le prédicat `score_panel.c:112-122`.
- Genres : `hf` vaut `1` (défaut de `jo`, lua.lua:387) ou `false` (`qh` et `ga`, lua.lua:909, 1070) ; `qh` vaut `1` (lua.lua:909) ou `false` (lua.lua:1001, 1059, 1098). Nombre ou faux : deux genres.
- Lecteurs : `self.hf and self.me=="ly"` (450), `not pt.hf` (1306), `oz.hf` (1329), `self.qh and …` (437), `pt.qh and #cg.qh` (1313). Tous en vérité seule. Aucune écriture hors des spécifications de classe, dans le cart comme dans le port (seules écritures : `planet.c:154-155`, `planet_family.c:103-104,619-620`, depuis `pico8_*_start`).
- Règle (G) : **un entier vrai/faux** (`int hf`, `int qh`), `hf_is_bool`/`qh_is_bool` supprimés des structs et de `pico8_game_data.h/.c` ; `pico8_jo_gg` rend `int` (0 sur `self` NULL, même vérité que nil). Raison : aucun lecteur ne lit le nombre.
- TROU-3 n'est pas en jeu ici : la comparaison `me=="ly"` est déjà un `strcmp`.

**B2. `qe` d'une route et d'un besoin — 23 lignes (+ 17 lignes (a) rangées avec).** `trade.c:13-17` (`pico8_trade_qe`), `trade.h:50`, `research.c:137-138` (`pico8_jy_qe`), `research.h:21`, tout `planet_trade.c` (12 lignes), `economy_tiers.c:21-25,241`.
- Genres : `trade.qe` vaut nil, `false` ou un nombre. `false` vient de TROU-24 (`trade.c:117`, `kg` pose `false` quand `self.qe` est nil) ; nil vient de `jo:ib` avec un `kw` sans `qe` (lua.lua:502-504). `jy.qe` (`i.qe`) vaut nil ou un nombre.
- Lecteurs qui distinguent nil de `false` : `t.qe==i.qe` (lua.lua:494, 1297) — en Lua, `false==nil` est faux, `nil==nil` est vrai. Le port le reproduit aujourd'hui par `pico8_value_equal` (identité de `REF NULL`, genre `BOOL` à part).
- Règle (G) : **petite struct étiquetée** `pico8_qe_t { enum { PICO8_QE_NIL, PICO8_QE_FALSE, PICO8_QE_FIXED } kind; pico8_fixed_t n; }` dans `pico8_trade.h`, et `pico8_qe_equal(a, b)` : même genre, et même `n` si `FIXED`. `pico8_trade_qe` et `pico8_jy_qe` rendent un `pico8_qe_t`. Les membres de `pico8_trade_t` (`has_qe`, `qe_is_bool`, `qe`) restent tels quels. Raison : trois états qu'un lecteur distingue ; un drapeau seul perdrait TROU-24.
- `economy_value_equal_content` (`economy_tiers.c:21-25`) ne compare jamais deux chaînes (ses arguments sont des `qe`) : il devient `pico8_qe_equal`.

**B3. `et` de la sonde — 16 lignes.** `probe.c:157-214` (locales `c` et `et` de `pico8_bv`), `wormhole.c:222-229` (`pico8_mz_et`), `wormhole.h:49`.
- Genres : `et` vaut nil (avant le premier `bv`), `false` (`probe.c:203`), une entité ou la vue statique `g_bv_et`. Le membre `probe->et` est déjà un pointeur, avec `has_et` toujours posé à 1 par `bv` (`probe.c:215`).
- Lecteurs : `not self.et` (lua.lua:569, via `has_et && et != NULL`, `probe.c:343`), `pico8_mzd_truthy(pico8_mz_et(self))` (`wormhole_display.c`), `jc_ce` → `probe->et`. Aucun ne distingue nil de `false`.
- Règle (G) : **pointeur, NULL pour nil et `false`** ; `has_et` supprimé ; `pico8_mz_et` rend `const void *` (ou disparaît au profit de `self->probe->et`). Raison : aucun lecteur ne sépare les deux absences.

**B4. `il` d'un projet et l'argument de `oj` — 14 lignes.** `research.c:251-258` (`pico8_hp_il`), `:345,369,381`, `research.h:84`, `tech_gate.c:28-36,99` (`pico8_oj`), `.h:10`, `research_icon.c:91`, `research_display.c:324`.
- Genres : `hp.il` vaut nil, `false` (position 7 écrite `f` dans `fd`, `il_false`) ou une chaîne. `oj(px)` reçoit une chaîne, nil ou `false` (lua.lua:1464-1466 : `not px or gu(fm.invented,px)`).
- Lecteurs : `self.il and oj(self.il)` (lua.lua:1423, 1427), `not px` : vérité seule.
- Règle (G) : **pointeur de chaîne, NULL pour nil et `false`** : `int pico8_oj(const char *px)`, `const char *pico8_hp_il(…)`. Le cas « nombre » de `test_pico8_hp` (`pico8_oj(fixed 1)`) est un genre que `src/` ne produit pas (G4).
- `hp_pose` (`research.c:73-107`, nommé au « Reste de D ») suit dans la même étape, sans compte : `mr` est la constante de classe `hp.mr` (lua.lua:1400, `pico8_hp_mr_names`), donc la position fixe le membre ; l'aiguillage par `strcmp` devient une affectation par position (`switch (p)`), les `assert` de genre restent.

**B5. `rn(no)` — 16 lignes.** `audio.c:24-28`, `audio.h:17,19`, dix appels (`planet.c:450,452`, `probe.c:357,362`, `slipway.c:126,195`, `wormhole.c:64`, `msgbox.c:339,361,363`, `ui_button.c:343`).
- Genres : un nombre (id de son) ou `false` (`rn(self.my and 60)`, lua.lua ; `ui_button.c:343`).
- Règle (G) : **`void pico8_rn(pico8_fixed_t n)`**, et l'unique appelant à `false` appelle sous condition (`if (has_my) pico8_rn(60)`). Raison : `rn(false)` ne fait rien ; l'audio est hors périmètre (TROU-10), `pico8_sfx` est vide.

**B6. `bg` d'une rangée — 4 lignes (+ 3 de `tests/`).** `layout.c:301-305` (`pico8_lq_bg`), `layout.h:130`.
- Genres : `bg=self.kr>=0 and 1` (lua.lua:429) : `1` ou `false` ; nil ailleurs. Le membre est déjà typé (`has_bg`, `bg_is_bool`, `bg`).
- Lecteur unique : `c.bg or 2` (lua.lua:275) : `false` et nil donnent 2.
- Règle (G) : **drapeau + nombre**, `false` rangé en absent (`has_bg = 0`), `bg_is_bool` supprimé ; `pico8_lq_bg` supprimé (appelants : deux TU seulement, `ga`, `hp`).

**B7. `ed` du panneau de score — 5 lignes.** `score_panel.c:298-305` (`pico8_pj_ed`), `.h:106`.
- Genres : `ed=1` (défaut de classe, lua.lua:846) ou un `mh` (lua.lua:899). Le membre est déjà un pointeur, NULL voulant dire « le défaut 1 » ; le seul lecteur (`kc~=self.ed`, lua.lua:882, `score_panel_display.c:317`) compare des identités.
- Règle (G) : **pointeur, NULL = défaut de classe** (déjà en place) ; `pico8_pj_ed` supprimé (un seul appelant, `test_pico8_spec_defaults`).

**B8. Cases d'une rangée (`lq`) — 104 lignes.** `layout.h:100,118,126` (`pico8_lq_keys_t.items`, `pico8_lq_keys_of_row`, `pico8_lq_add`), `layout.c:30-113` (`pico8_jb_item_value`, `pico8_lq_scalar_cell`, `pico8_lq_cell_of`, lecture des tuples dans `lq:mn`), `:222-227,375-382,411` ; tableaux d'items de `menu.c` (14), `help.c:46`, `msgbox.c:334,351`, `planet_display.c` (≈ 21), `research_display.c` (≈ 15), `score_panel.c:151-227` (`pico8_pj_item_of`, `ou[]`), `score_panel.h:125`, `score_panel_display.c:126-139,261-285`.
- Genres : une case vaut nil, `false`, un nombre, une chaîne, une rangée (tuple `{f,w,qw,qv}` ou `{qe=x}`), ou autre chose de vrai ; `lq:mn` les distingue (lua.lua, `lq:mn`).
- L'union existe déjà : `pico8_lq_cell_t` (`layout.h:29-49`, D.12c), ses cinq genres (`NOTHING`, `FIXED`, `STRING`, `ROW`, `TRUE`). Les clés passent encore par `pico8_value_t` puis sont triées par `pico8_lq_cell_of`.
- Toute `REF` passée comme case pointe un `pico8_jb_row_t` : tuples générés, `&tuple->row`, `&jy->row` par premier membre (`planet_display.c:215-237`), `pico8_hp_fk()`, `pico8_jo_em()`, `&gap`, `&back`. `pico8_lq_cell_of` le suppose déjà.
- Règle (G) : **`pico8_lq_keys_t.items` devient `const pico8_lq_cell_t *`**, avec des constructeurs `pico8_lq_cell_fixed/string/row/nothing()` ; `pico8_lq_add(self, pico8_lq_cell_t)` ; `pico8_lq_keys_of_row(row, pico8_lq_cell_t *cells, cap)` trie chaque `pico8_jb_item_t` en case directement (`is_ref` à NULL et `is_false` → `NOTHING`) ; `pico8_jb_item_value` et `pico8_lq_scalar_cell` disparaissent ; les tuples de `lq:mn` se lisent sur leurs `pico8_jb_item_t`. `ou[]` et `pico8_pj_item_of` deviennent des `pico8_jb_item_t` écrits directement (l'item généré est déjà l'union typée : `s`, `n`, `row`, `is_false`, `is_fixed`, `is_ref`). Raison : l'union locale de Q10 est déjà là, il ne reste qu'à y entrer sans passer par la valeur.

**B9. Sources d'un bouton (`js` : `lq`, `h_`, `jb`) — 38 lignes (+ 12 de `tests/`).** `ui_button.h:73,76,79,107-108`, `ui_button.c:119-166` (constructeur, TROU-23), `:218-252` (`pico8_js_lq`, `pico8_js_pq`), écritures `.lq/.h_/.jb = pico8_value_ref(NULL)` de `menu.c` (7), `help.c:58`, `msgbox.c:356`, `planet_display.c:184,190,303,370-395`, `score_panel_display.c:278,293`.
- Genres, preuve par les écritures de `src/` : `lq` vaut une rangée (`lq_row`), un `lq` existant (`pico8_hp_lq(b)`, `planet_display.c:372`) ou nil ; `h_` vaut une rangée, `false` (`h_=od>0 and {…}`, lua.lua:419 ; `planet_display.c:376`) ou nil — nil présent devient `true` (TROU-23) ; `jb` vaut une rangée ou nil (nil présent → `true`, TROU-23 ; `false` s'arrête sur un `assert`).
- Lecteurs : `js:pq` (lua.lua:261-262) distingue `false` (pas de `h_`, `has_h_=1`) de nil (pas de `h_`, `has_h_=0`) et de `true` (rangée vide).
- Règle (G) : **champs séparés** : `const pico8_lq_t *lq_copy` (NULL : aucun) à côté de `lq_row` ; `has_h_` + `h_is_false` à côté de `h_row` ; `has_jb` à côté de `jb_row`. TROU-23 s'écrit « présent, sans rangée, pas faux → rangée vide ». `pico8_js_pq` prend ces champs ; `pico8_js_is_lq` (qui lisait l'étiquette de type) disparaît. Raison : les quatre états sont déjà des drapeaux au site d'écriture ; une union ne ferait que les réemballer.

**B10. Articles de `bl` (`pi.c`) — 32 lignes.** `pico8_pi_arg` (`pi.c:58-78`) et ses lecteurs `io` (`:100-110`), `eq`, `spr` (`:126-131`), `ld` (`:148-155`).
- Genres : le premier argument de `io` est une chaîne ou un nombre (année, compteur de techs de la barre de menu, commentaire `pi.c:95-97`) ; les autres, nombre ou nil ; `ld` reçoit un `lq` (référence) et un vecteur.
- Règle (G) : **lire l'item généré** : `pico8_pi_arg` rend `const pico8_jb_item_t *` (NULL au-delà de la fin) et trois petites lectures typées (`nombre ou défaut`, `chaîne ou NULL`, `pointeur`) ; `io` formate un nombre par `pico8_tostr_fixed`. Raison : `pico8_jb_item_t` est l'union de ces articles ; la valeur n'était qu'une traduction intermédiaire.

**B11. `s[i]` du panneau de fin — 12 lignes.** `score_panel_display.c:143-158` (`pico8_pj_ph_s_at`), `:178-196`.
- Genres : nombre (`di`, `gl`, `el`, `total`, `tier`), chaîne (`happiness`), nil hors de 1..6 (score 0, TROU-13 et TROU-25) ; l'unique lecteur est `tostr`.
- Règle (G) : **fusion avec son lecteur** : `char *pico8_pj_ph_s_str(s, i)` rend directement la chaîne possédée (`pico8_tostr_fixed`, copie de `happiness`, `""` hors bornes — TROU-13 préservé et écrit au site). Raison : la valeur n'a pas d'autre lecteur que sa mise en chaîne.

**B12. Les 13 copies privées de `*_truthy(pico8_value_t)` — 37 lignes.** `economy_score.c:18`, `economy_tiers.c:17`, `layout.c:19`, `planet.c:36`, `planet_display.c:27`, `probe.c:46`, `research.c:34`, `research_display.c:19`, `slipway.c:49`, `tech_gate.c:15`, `ui_button.c:28`, `wormhole.c:30`, `wormhole_display.c:12` (et `score_panel.c:161`, l'`assert` de `pico8_pj_item_of`).
- Deux variantes : `economy_*` ne rejette que `false` (un nil y serait lu vrai ; au passage en `int`, le PRD vérifie qu'aucun appelant ne passe un `self` NULL), les autres rejettent `false` et nil.
- Règle (G) : **supprimées avec leur argument** ; chaque appel devient la vérité C du type (`int`, `ptr != NULL`, `has_x`). Aucune n'est gardée en commun.

Parts `tests/` de (b), 57 lignes : `test_pico8_lq` (cases, 35 environ), `test_pico8_js` (sources de bouton, 12), `test_pico8_hp` (`oj`, 5), `bg` dans `ga`/`hp` (3), `jo_gg` et `mz_et` (2).

### 1.4 `type-tag` — 249 lignes (243 après D.13)

| Nature | `src/` | `tests/` |
|---|---|---|
| définition `pico8_type_tag.h` | 12 | — |
| `#include` | 15 | 34 |
| écritures de l'étiquette (`header.tag = …`, initialiseurs `{PICO8_TYPE_TAG_STRUCT, &cls}` des vues statiques) | 17 | 3 |
| `pico8_type_tag_of(x) == PICO8_TYPE_TAG_STRUCT` | 26 | 133 |
| membre `tag` et commentaire de `pico8_struct_header_t` (`struct.h:44,46`) | 2 | 1 |
| `pico8_ref_audit.h` (supprimé par D.13) | 6 | — |

- **Aucune ligne de `src/` ne discrimine réellement** : `PICO8_TYPE_TAG_UNKNOWN` n'est lu nulle part dans `src/`. Les 26 `pico8_type_tag_of` sont des `assert` qui gardent un cast de `void *` — éléments d'un conteneur de pointeurs (Q9 : « `cv` holds entities only », « `cg("jo")` only ever holds entities », « `qu.oe` must hold jc structs ») ou `self` d'une méthode. Deux dépendent de `valeur` : `ui_button.c:20` (`pico8_js_is_lq`, meurt avec B9) et `probe.c:210` (sur `et`, meurt avec B3).
- `tests/` : les 133 lignes sont les mêmes `assert` et les aides d'empreinte qui plient l'étiquette dans les empreintes figées (`fnv(h, pico8_type_tag_of(ref))` : `0x9E5C57C7` pour une entité, 0 pour NULL ou autre chose). Neuf TU vérifient qu'une sorte (`pico8_entity_kind_t`) lit `UNKNOWN` : c'est un test de la couche.
- Raison d'être de l'étiquette (`type_tag.h:9-12`) : rendre répondable « qu'est-ce que cette `REF` » pour une `pico8_value_t REF`. Elle tombe avec `valeur`.

---

## 2. État d'arrivée

- **`pico8_value.c` / `pico8_value.h` : supprimés**, avec `test_pico8_value.c` (test de la couche, §6). Rien ne reste sous ce nom.
- **Ce qui survit ailleurs** :
  - `pico8_tostr_fixed` dans `pico8_str.c/.h` (formatage à 4 décimales, arrondi pair ; TROU-22) ; ses cas « nombre » de `test_pico8_value` passent au TU de `pico8_str` (ils portent les 13 témoins de `tools/probe_out.txt`, ce n'est pas un test de la couche) ; ses cas chaîne, booléen et `REF` sont supprimés avec leur genre.
  - `pico8_lq_cell_t` (déjà là) et ses constructeurs ; `pico8_qe_t` et `pico8_qe_equal` dans `pico8_trade.h`.
  - `pico8_value_equal` n'a plus d'appelant : TROU-3 et TROU-13 de `docs/trous-de-portage.md` sont réécrits pour nommer les sites qui restent (comparaisons de chaînes par contenu ; `""` écrit au site du score 0).
- **`pico8_type_tag.h` : supprimé.** `pico8_struct_header_t` garde `cls` seul, en premier membre. Les `assert` sur un `void *` deviennent des tests de classe (`->cls == &g_x_struct_class`, `pico8_X_of`) ; là où le pointeur est déjà typé, l'`assert` disparaît. Les vues statiques s'initialisent `{&cls}`.
- **« Plus aucun `malloc` pour des données »** : l'allocation de `pico8_value_tostr` part avec le module ; celle de `pico8_tostr_fixed` rend une chaîne à l'appelant, comme `pico8_sub` — ce n'est pas une table de données.
- **`valeur` et `type-tag` atteignent 0** et entrent dans `DATA_RESIDUE_GUARD`. Le motif `valeur` ne doit rien toucher de neuf : `pico8_tostr_fixed`, `pico8_qe_t`, `pico8_lq_cell_*` n'y correspondent pas (règle commune : un nom nouveau ne correspond à aucun motif de la catégorie qu'il remplace). Avec `ref-audit` gardé par D.13, **le résidu D est vide : c'est la Sortie de D.**

---

## 3. Étapes proposées

Ordre : les API mécaniques d'abord (elles réduisent les appelants des points (b)), puis les scalaires (b), `qe`, les rangées et boutons, la suppression du module, enfin `type-tag` (ses deux `assert` liés à `valeur` doivent être partis).

Règles communes, pour chaque PRD :
- **Un TU qui nomme un symbole changé le suit dans le même commit.** Un TU peut être converti plus tôt que son symbole ; jamais plus tard.
- **Aucune empreinte figée ne change** (G1). Pendant D.15–D.18, un TU qui plie une valeur désormais typée reproduit les octets de l'ancien pli (octet de genre + contenu) à partir du type C.
- Aucun `*.sha256` ni e2e ne change.
- `class-coverage-net` si l'étape touche un constructeur ; `sanitize-net` si elle change une durée de vie ou un propriétaire.

| Étape | Contenu | Empreinte estimée | Compte après |
|---|---|---|---|
| **D.15a** | `pico8_ov` en nombres, `pico8_cb(int)` ; appelants `economy`, `planet`, `slipway`, `wormhole`, `menu` ; TU `fm`, `js` | ≈ 8 fichiers | `valeur` ≈ 979 |
| **D.15b** | pointeurs : `pico8_ij` (sortie `void **`), `pico8_jc_ce` (pointeur, `jc_value_instance` retiré), `pico8_d_` (`int`), `pico8_jc_hg_add/has` (`const char *`), `pf/mz_value_vec` retirés, `g_ie_id` en drapeau + nombre ; TU `jc`, `g_`, et les plis de `d_` (`fr`, `fv`, `hj`, `mx`, `nl`, `nr`, `os`, `qh`) | ≈ 20 fichiers | ≈ 910 |
| **D.15c** | `pico8_tostr_fixed` dans `pico8_str.c`, 20 sites ; cas « nombre » de `test_pico8_value` déplacés au TU de `pico8_str` (G5) | ≈ 16 fichiers | ≈ 880 |
| **D.16a** | B1 : `hf`, `qh` en `int`, `pico8_jo_gg` en `int`, `hf_is_bool`/`qh_is_bool` retirés (structs, `pico8_game_data`) ; copies `truthy` de leurs appelants ; TU : plis de `jo_gg` (`fr`, `ga`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`), `spec_defaults` | ≈ 22 fichiers | ≈ 845 |
| **D.16b** | B3 (`et`, `has_et` retiré), B4 (`oj`, `il`) et `hp_pose` par position, B5 (`rn`), B6 (`bg`, `bg_is_bool` retiré), B7 (`pico8_pj_ed` retiré) ; copies `truthy` restantes de ces modules ; TU `hp`, `mz`, `g_`, `ga`, `spec_defaults`, `lq` (bg) | ≈ 25 fichiers : **découper** b1 (B3, B5) / b2 (B4, B6, B7, `hp_pose`) | ≈ 765 |
| **D.17** | B2 : `pico8_qe_t`, `pico8_qe_equal` ; `pico8_trade_qe`, `pico8_jy_qe` typés ; `planet_trade`, `economy_tiers`, `kw.qe` et `o` de `pico8_jo_ex` (`planet_display`, `research_display`) ; `pico8_value_equal` n'a plus d'appelant dans `src/` ; TROU-3 et TROU-24 relus ; TU `trade`, `hp`, `nr`, `lq` (`jo_ex`) | ≈ 14 fichiers | ≈ 720 |
| **D.18a** | B8 : `pico8_lq_keys_t.items` en `pico8_lq_cell_t`, constructeurs, `pico8_lq_add(cell)`, `pico8_lq_keys_of_row` ; `layout` et tous les tableaux d'items (`menu`, `help`, `msgbox`, `planet_display`, `research_display`, `score_panel_display`) ; §6 : `layout_nil_item_draws_nothing` et `planet_gh_row_lq`, avec leur preuve rouge (G6) ; TU `lq`, `js`, `ga`, `hp` | ≈ 18 fichiers ; si trop gros, a1/a2 avec un pont compté `pico8_lq_cell_of_value`, retiré en a2 (le PRD tranche) | ≈ 560 |
| **D.18b** | B9 : sources de bouton en champs séparés (`lq_copy`, `h_is_false`), TROU-23 réécrit, `pico8_js_pq` typé, `pico8_js_is_lq` retiré ; TU `js`, `lq` | ≈ 10 fichiers | ≈ 505 |
| **D.18c** | B10 (`pico8_pi_arg` → item généré), B11 (`pico8_pj_ph_s_str`, TROU-13 au site), `ou[]`/`pico8_pj_item_of` en `pico8_jb_item_t` ; TU `pj` | ≈ 6 fichiers | ≈ 435 |
| **D.19a** | TU seuls : aides d'empreinte et aides locales (`fold_value`, `fold_cell`, `fold_kind`, `vec_ref`, `nil()`, `mouse_at`…) en plis typés aux mêmes octets, TU de la famille planète (`fr`, `fv`, `g_`, `ga`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`) | ≈ 12 fichiers | ≈ 260 |
| **D.19b** | idem, autres TU (`bg`, `hp`, `hy`, `ie`, `iv`, `jc`, `js`, `lq`, `mo`, `mz`, `pj`, `trade`, `ui`…) | ≈ 20 fichiers | ≈ 115 |
| **D.19c** | `pico8_value.c/.h`, `test_pico8_value.c` supprimés (§6) ; les includes restants (≈ 76 lignes, `src/` et `tests/`) ; commentaires qui nomment le module ; `Makefile`, `tools/float_guard.txt` si `make test` le réécrit, `glossaire-noms.md` | ≈ 75 chemins, surtout des includes | `valeur` = 0, dans `DATA_RESIDUE_GUARD` |
| **D.20a** | TU seuls, `type-tag` : `assert(pico8_type_tag_of(x) == …STRUCT)` en tests de classe ; plis d'étiquette en constante figée `FROZEN_STRUCT_TAG 0x9E5C57C7u` (0 pour NULL et les non-entités, comme aujourd'hui) ; tests « une sorte lit UNKNOWN » supprimés (test de la couche, §6) ; l'étiquette reste dans `src/` | 34 fichiers : **découper** a1 (famille planète, ≈ 80 lignes) / a2 (le reste, ≈ 90) | `type-tag` ≈ 72 (`src/` seul) |
| **D.20b** | `src/` : membre `tag` retiré de `pico8_struct_header_t`, 17 écritures, 26 `assert` en tests de classe (ou retirés si le pointeur est typé), 15 includes, `pico8_type_tag.h` supprimé | ≈ 25 fichiers | `type-tag` = 0, dans `DATA_RESIDUE_GUARD` ; résidu D vide |

Drapeaux :
- `class-coverage-net` : D.16a (constructeurs `jo`, `qh`, `ga`), D.16b (constructeur `hp`), D.17 (constructeur `trade`), D.18b (constructeur `js`), D.20b (en-tête de tous les constructeurs), et la sortie de phase.
- `sanitize-net` : D.15c (l'allocation de `tostr` change de module), D.18a (tableaux de cases, dont ceux de `jo_gh_row_items`, TROU-7), D.18c (la chaîne de `s[i]` change de propriétaire), D.19c (dernière allocation du module), et la sortie de phase.
- §6 : D.18a réécrit `layout_nil_item_draws_nothing` et `planet_gh_row_lq` (preuve rouge, G6) ; `ga_export_panel_by` n'a qu'un include (D.19c) ; D.19c supprime `test_pico8_value` ; D.20a supprime les tests de la couche sur l'étiquette.
- Arrêt « champ sans règle » : aucun site recensé n'est sans règle ; un site nouveau tombe sous B1–B12 ou sous (a).

---

## 4. Lignes directrices tranchées ici (G)

- **G1. Empreintes figées.** Aucune constante figée ne change dans D.15–D.20. Un pli typé écrit le même octet de genre (`0`/`1`/`2`/`3`, les valeurs de l'actuel `pico8_value_kind_t`, ou les octets propres au TU : `0xB0`, `0xF1`, `0x57`) et le même contenu. Une empreinte qui bouge est une régression, pas une mise à jour.
- **G2. nil et `false` confondus** seulement là où aucun lecteur du cart ne les sépare (preuve par grep des lecteurs dans le PRD) : B1, B3, B4, B6. Là où un lecteur les sépare, ils restent deux états : B2 (`qe`, TROU-24), B9 (`h_`, TROU-23).
- **G3. Écart préservé.** TROU-13, TROU-23, TROU-24 et TROU-25 restent à l'identique ; leur texte est réécrit quand il nomme `pico8_value_*`.
- **G4. Genres que `src/` ne produit pas.** Un cas de TU qui passe un genre qu'aucun appelant de `src/` n'écrit (`pico8_oj` d'un nombre, `pico8_jc_hg_has` d'un nombre ou de nil, `pico8_jo_ex` d'un `false`) est supprimé avec le type qui le permettait ; le message de commit le nomme. Ce n'est pas un test de régression (§6, deuxième point).
- **G5. `tostr`.** `pico8_tostr_fixed` va dans `pico8_str.c` (même convention de propriété que `pico8_sub`) ; ses 13 témoins restent testés.
- **G6. `layout_nil_item_draws_nothing`.** Le bogue d'origine (un item nil classé « vrai », dessiné comme sprite 0) se réintroduit désormais dans le tri d'un item généré (`is_ref` à NULL, ou `is_false`) en case ; le test passe par `pico8_lq_keys_of_row` sur un tel item, et la preuve rouge remet ce tri fautif.
- **G7. `hp_pose`.** Positionnel en D.16b, sans compte : `mr` est une constante de classe.

---

## 5. Question à l'humain

**Q13. Le nombre de contrôle des objets du jeu.**
Chaque objet du jeu porte, au début de sa mémoire, un nombre de contrôle. Il sert à vérifier, quand on retire un objet d'une liste fourre-tout, qu'il s'agit bien d'un objet du jeu. Une fois ce ménage fini, plus aucune liste ne mélange les objets du jeu avec autre chose : ce contrôle devient une simple précaution.
(A) Le retirer ; on vérifie seulement la catégorie de l'objet, un peu moins robuste si la mémoire est abîmée.
(B) Le garder à part, comme précaution.
Recommandation : A. C'est ce que le plan prévoyait déjà, et les vérifications mémoire automatiques couvrent ce risque.

---

## Annexe A — sites (b) de `src/`, ligne à ligne

Groupes du classement (`b-*`), numéros de ligne à `b7f6c02` :

- **truthy (B12)** : `ui_button.c:28,29` ; `slipway.c:49,50,53` ; `wormhole_display.c:12,13,16` ; `planet.c:36,37,40` ; `economy_tiers.c:17,18` ; `wormhole.c:30,31,34` ; `tech_gate.c:15,16,19` ; `research_display.c:19,20,23` ; `layout.c:19,20,23` ; `probe.c:46,47,50` ; `score_panel.c:161` ; `planet_display.c:27,28,31` ; `economy_score.c:18,19` ; `research.c:34,35,38`.
- **hf/qh (B1)** : `planet.c:348,350,353,355,358,359,362,368` ; `planet.h:137,142` ; `planet_family.c:38,40,43,45` ; `planet_family.h:46` ; `score_panel.c:112,119,122`.
- **qe (B2)** : `trade.c:13,15,17` ; `trade.h:50` ; `economy_tiers.c:21,22,25,241` ; `planet_trade.c:75,76,80,95,99,104,112,144,152,157,161,168` ; `research.c:137,138` ; `research.h:21`.
- **et (B3)** : `probe.c:157,158,175,178,186,191,201,203,208,209,214` ; `wormhole.c:222,224,227,229` ; `wormhole.h:49`.
- **oj/il (B4)** : `research.c:251,253,256,258,345,369,381` ; `research.h:84` ; `tech_gate.c:28,35,99` ; `tech_gate.h:10` ; `research_icon.c:91` ; `research_display.c:324`.
- **rn (B5)** : `audio.c:24,25,28` ; `audio.h:17,19` ; `ui_button.c:343` ; `slipway.c:126,195` ; `planet.c:450,452` ; `wormhole.c:64` ; `msgbox.c:339,361,363` ; `probe.c:357,362`.
- **bg (B6)** : `layout.c:301,303,305` ; `layout.h:130`.
- **ed (B7)** : `score_panel.c:298,300,303,305` ; `score_panel.h:106`.
- **cases (B8)** : `layout.c:30-113` (19 lignes), `:222,223,227,375,382,411` ; `layout.h:100,118,126` ; `menu.c:86,88,100,101,112,113,190,191,275,286,351,382` ; `help.c:46` ; `msgbox.c:334,351` ; `planet_display.c:117,118,153,155,160,194,213,215,232,234,237,250,252,271,272,286,287,289,301,358,360` ; `research_display.c:144,150,167,168,211,222,231,239,240,250,260,265,268,308,310` ; `score_panel.c:151,153,155,157,172,175,178,188,194,201,204,217,222,227` ; `score_panel.h:125` ; `score_panel_display.c:126,128,131,134,137,139,261,262,270,285`.
- **sources de bouton (B9)** : `ui_button.c:119,120,121,162,163,165,166,218,222,229,230,242,250,252` ; `ui_button.h:73,76,79,107,108` ; `menu.c:106,117,121,196,199,364,387` ; `help.c:58` ; `msgbox.c:356` ; `planet_display.c:184,190,303,370,372,376,384,395` ; `score_panel_display.c:278,293`.
- **articles de `bl` (B10)** : `pi.c:58-155` (32 lignes).
- **`s[i]` (B11)** : `score_panel_display.c:143,146,148,150,152,154,156,158,178,180,194,196`.

## Annexe B — reproduire

```sh
sh tools/data_residue.sh; sh tools/data_residue.sh --detail | grep -E '^(valeur|type-tag)'
grep -nE '\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h' src/*.c src/*.h tests/*.c   # 1014 lignes, 518 dans src/
grep -nE 'pico8_value_t' src/*.h | grep -vE ':\s*(\*|/\*)'      # 35 lignes : 29 déclarations (23 hors pico8_value.h + 6), 4 membres, 1 typedef, 1 suite de ligne
grep -n 'TYPE_TAG_UNKNOWN\|!= PICO8_TYPE_TAG_STRUCT' src/*.c   # vide : aucune discrimination réelle dans src/
grep -n 'hf=\|qh=\|bg=\|ed=\|il=' pico8/lua.lua                # genres écrits par le cart
```

Le classement ligne à ligne est un `awk` sur la sortie du premier `grep` : plages de lignes par groupe B1–B12 (annexe A), le reste de `src/` en (a) ; dans `tests/`, les lignes qui nomment une API des groupes (b) ou les clés de rangée en (b), les aides d'empreinte et aides locales en (a).
