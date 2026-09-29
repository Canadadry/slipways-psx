---
title: "Plan 3, étape D.18a : genres mêlés B8 — les clés d'une rangée `lq` (`pico8_lq_keys_t.items`) deviennent des cases `pico8_lq_cell_t` écrites par leurs producteurs au moyen de cinq constructeurs ; `pico8_lq_add` et `pico8_lq_keys_of_row` prennent et rendent des cases ; le classement valeur → case disparaît de `layout` ; §6 `layout_nil_item_draws_nothing` réécrit dans sa lecture et `planet_gh_row_lq` reprouvé, preuves rouges ; aucune constante ne change"
description: "Les positionnels d'une rangée `lq` passent encore par la valeur Lua, triée en case à la construction ; D.18a fait écrire les cases directement par les quinze sites producteurs (`layout`, `menu`, `help`, `msgbox`, `planet_display`, `research_display`, `score_panel_display`) et par trois TU, garde la distinction nil / faux que l'union a fondue là où `CONSTRUCTIONS_HASH` la plie (masque `nil_at` du TU, vérifié par M0 et par une mutation exigée), et fait passer le test du bogue « item nil dessiné » par le tri d'un item généré : `valeur` baisse de 133 lignes (140 occ.), ≈ 693 → ≈ 560."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.17 est implémentée** (`plan-pilot`, étape 2b) sur l'arbre commis **`b4ec4b3`** (HEAD de `claude/plan-pilot-3-7zdft0` : PRD de D.17 mis en file, D.17 non commencée), extrait par `git archive b4ec4b3` dans le scratchpad. Seuls `grep`, `sed`, `awk` et `sh tools/data_residue.sh` ont tourné ; **rien n'a été compilé ni lancé**, rien de non commis n'a été lu. Tout ce que ce PRD suppose de l'issue de D.17 est une hypothèse **`H-D17-n`** (table plus bas), à confronter à la note « Découvert à D.17 » quand D.17 sera gradée ; les sections à revérifier sont listées. **Les numéros de ligne sont ceux de `b4ec4b3`** ; pour les trois fichiers que D.17 touche aussi (`planet_display.c`, `research_display.c`, `test_pico8_lq.c`), une colonne donne le numéro attendu après D.17 (H-D17-3 à H-D17-5). L'implémenteur remesure par contenu au parent de son commit et explique tout écart.

Mesuré à `b4ec4b3` (départ de D.17) : `valeur` 745 / 89 ; 831 occ., `type-tag` 242 / 58, résidu D 985 / 104. **Départ supposé de D.18a (H-D17-1) : `valeur` 693 / 85 ; 763 occ. (`src/` 270 / 47 / 301, `tests/` 423 / 38 / 462), `type-tag` 242 / 58, résidu D 933 / 102.**

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c/.h` disparaissent (D.19c). Le plus gros groupe à genres mêlés de l'étude D.14 (`docs/prd/done/plan3-phaseD-etape14-etude-genres-meles.md`, B8, 104 lignes à `b7f6c02`) est celui des **cases d'une rangée `lq`** : ses positionnels valent nil, faux, un nombre, une chaîne, une rangée (tuple `{f,w,qw,qv}` ou `{qe=x}`) ; `lq:mn` les distingue. D.12c a déjà créé l'union qui les range (`pico8_lq_cell_t`, `layout.h:29-49`, cinq genres) et l'a mise dans `lq.items` ; mais **les clés `pico8_lq_keys_t` gardent `const pico8_value_t *items`** (Q12 alors ouverte), si bien que chaque producteur fabrique des valeurs que `layout.c` retrie aussitôt en cases (`pico8_lq_cell_of`, `pico8_lq_scalar_cell`, `pico8_layout_truthy`), et qu'un item généré (`pico8_jb_item_t`) passe par une valeur (`pico8_jb_item_value`, `pico8_pj_ph_item_value`) pour devenir une case.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `b4ec4b3` (§5, l. 289) :

> - D.18a — B8 : `pico8_lq_keys_t.items` en cases de l'union de `layout` ; §6 `layout_nil_item_draws_nothing` et `planet_gh_row_lq` réécrits dans leur lecture, preuve rouge. `sanitize-net` : oui. Compte : ≈ 560.

Ligne de l'étude D.14 (§3), recopiée :

> | **D.18a** | B8 : `pico8_lq_keys_t.items` en `pico8_lq_cell_t`, constructeurs, `pico8_lq_add(cell)`, `pico8_lq_keys_of_row` ; `layout` et tous les tableaux d'items (`menu`, `help`, `msgbox`, `planet_display`, `research_display`, `score_panel_display`) ; §6 : `layout_nil_item_draws_nothing` et `planet_gh_row_lq`, avec leur preuve rouge (G6) ; TU `lq`, `js`, `ga`, `hp` | ≈ 18 fichiers ; si trop gros, a1/a2 avec un pont compté `pico8_lq_cell_of_value`, retiré en a2 (le PRD tranche) | ≈ 560 |

Point B8 de l'étude (§1.3), recopié :

> **B8. Cases d'une rangée (`lq`) — 104 lignes.** `layout.h:100,118,126` (`pico8_lq_keys_t.items`, `pico8_lq_keys_of_row`, `pico8_lq_add`), `layout.c:30-113` (`pico8_jb_item_value`, `pico8_lq_scalar_cell`, `pico8_lq_cell_of`, lecture des tuples dans `lq:mn`), `:222-227,375-382,411` ; tableaux d'items de `menu.c` (14), `help.c:46`, `msgbox.c:334,351`, `planet_display.c` (≈ 21), `research_display.c` (≈ 15), `score_panel.c:151-227` (`pico8_pj_item_of`, `ou[]`), `score_panel.h:125`, `score_panel_display.c:126-139,261-285`.
> - Genres : une case vaut nil, `false`, un nombre, une chaîne, une rangée (tuple `{f,w,qw,qv}` ou `{qe=x}`), ou autre chose de vrai ; `lq:mn` les distingue (lua.lua, `lq:mn`).
> - L'union existe déjà : `pico8_lq_cell_t` (`layout.h:29-49`, D.12c), ses cinq genres (`NOTHING`, `FIXED`, `STRING`, `ROW`, `TRUE`). Les clés passent encore par `pico8_value_t` puis sont triées par `pico8_lq_cell_of`.
> - Toute `REF` passée comme case pointe un `pico8_jb_row_t` : tuples générés, `&tuple->row`, `&jy->row` par premier membre (`planet_display.c:215-237`), `pico8_hp_fk()`, `pico8_jo_em()`, `&gap`, `&back`. `pico8_lq_cell_of` le suppose déjà.
> - Règle (G) : **`pico8_lq_keys_t.items` devient `const pico8_lq_cell_t *`**, avec des constructeurs `pico8_lq_cell_fixed/string/row/nothing()` ; `pico8_lq_add(self, pico8_lq_cell_t)` ; `pico8_lq_keys_of_row(row, pico8_lq_cell_t *cells, cap)` trie chaque `pico8_jb_item_t` en case directement (`is_ref` à NULL et `is_false` → `NOTHING`) ; `pico8_jb_item_value` et `pico8_lq_scalar_cell` disparaissent ; les tuples de `lq:mn` se lisent sur leurs `pico8_jb_item_t`. `ou[]` et `pico8_pj_item_of` deviennent des `pico8_jb_item_t` écrits directement (l'item généré est déjà l'union typée : `s`, `n`, `row`, `is_false`, `is_fixed`, `is_ref`). Raison : l'union locale de Q10 est déjà là, il ne reste qu'à y entrer sans passer par la valeur.

(`ou[]`/`pico8_pj_item_of`, `score_panel.c`, `score_panel.h` sont rangés par le tableau de l'étude dans **D.18c** : ce ne sont pas des producteurs de clés de `lq` — leurs items sont des articles de `bl`. Hors de D.18a.)

Lignes directrices de l'étude qui s'appliquent (§4), recopiées en extrait :

> **G1.** Aucune constante figée ne change dans D.15–D.20. Un pli typé écrit le même octet de genre (`0`/`1`/`2`/`3`, les valeurs de l'actuel `pico8_value_kind_t`, ou les octets propres au TU : `0xB0`, `0xF1`, `0x57`) et le même contenu. Une empreinte qui bouge est une régression, pas une mise à jour.
> **G2.** nil et `false` confondus seulement là où aucun lecteur du cart ne les sépare […] : B1, B3, B4, B6. […]
> **G4.** Un cas de TU qui passe un genre qu'aucun appelant de `src/` n'écrit […] est supprimé avec le type qui le permettait ; le message de commit le nomme.
> **G6.** `layout_nil_item_draws_nothing`. Le bogue d'origine (un item nil classé « vrai », dessiné comme sprite 0) se réintroduit désormais dans le tri d'un item généré (`is_ref` à NULL, ou `is_false`) en case ; le test passe par `pico8_lq_keys_of_row` sur un tel item, et la preuve rouge remet ce tri fautif.

Règles communes de l'étude (§3) : un TU qui nomme un symbole changé le suit dans le même commit ; aucune empreinte figée ne change ; aucun `*.sha256` ni e2e ne change ; `sanitize-net` si l'étape change une durée de vie ou un propriétaire (drapeau de l'étude pour D.18a : « tableaux de cases, dont ceux de `jo_gh_row_items`, TROU-7 »).

Legs de D.12c (`docs/plan-3.md` l. 279, note « Découvert à D.12c2 », recopiée en extrait) :

> Piège « rien » : M0 (instrumentation temporaire des plis de positionnels de `lq`, `js`, `pj`, `hp`, `ga`, restaurée par `cp`) : seule `CONSTRUCTIONS_HASH` plie des nil (2) et des faux (2), dont une rangée mêlée (« mixed c »), famille (1) ; toutes les autres constantes, 0 : conforme. `test_pico8_lq` plie une case « rien » du genre de sa clé (`fold_row_from`, `assert_sorted` asserte la case de chaque clé), les autres TU la plient comme faux.

Leg de D.17 (son PRD, « Further Notes », à confirmer par sa note — H-D17-3) :

> **Pour D.18a** : `research_display.c` garde 10 lignes, toutes des cases (`l`, `pico8_lq_add`) ; B8 compte 5 lignes de moins que l'étude.

### Hypothèses `H-D17-n` (D.17 en cours d'implémentation)

Source : le PRD de D.17 (`docs/prd/triage/plan3-phaseD-etape17-b2-qe-nil-faux-nombre.md` à `b4ec4b3`), son Inventaire et ses Sorties. À la graduation de D.17, chaque ligne reçoit un verdict (`tient` / `fausse`, mesure, effet), comme le PRD de D.17 l'a fait pour `H-D16b-n`.

| Hypothèse | Énoncé | Vérification à la graduation |
|---|---|---|
| H-D17-1 | Après D.17 : `valeur` **693 / 85 ; 763 occ.** (`src/` 270 / 47 / 301, `tests/` 423 / 38 / 462) ; `type-tag` 242 / 58 ; résidu D **933 / 102** ; `DATA_RESIDUE_GUARD` inchangé | `make -s data-residue` ; recette des occurrences |
| H-D17-2 | D.17 ne touche, dans l'empreinte de D.18a, que `planet_display.c`, `research_display.c/.h` et `test_pico8_lq.c` ; ni `layout.c/.h`, `menu.c`, `help.c`, `msgbox.c`, `score_panel_display.c`, ni `test_pico8_js.c`, `test_pico8_layout_nil_item_draws_nothing.c`, `test_pico8_planet_gh_row_lq.c`, `test_pico8_ga_export_panel_by.c` | `git diff --stat b4ec4b3 <HEAD gradé> -- src tests` |
| H-D17-3 | `research_display.c` : **10 / 10**, exactement les lignes de cases (`l`, `pico8_lq_add`) : `:145,212,223,232,240,241,266,269,309,311` à `b4ec4b3` → **`:133,197,208,217,225,226,250,253,293,295`** (D.17 retire `:18-29`, `:164`, `:204-205`, `:261`) ; `ml` en `pico8_fixed_t[2]` ; `pico8_jo_ex(…, const pico8_fixed_t *o, …)` ; `keys` (`:148` → `:136`), `malloc` de `l` (`:186` → `:173`), `keys.items = l` (`:275` → `:259`) inchangés | `sh tools/data_residue.sh --detail` ; `grep -nE "$P"` |
| H-D17-4 | `planet_display.c` : **30 / 30** ; ses 22 lignes B8 intactes, décalées de − 8 (avant `:213`), − 9 (`:214`–`:250`), − 10 (après `:250`) (D.17 retire `jo_kw_qe` `:26-33`, le marqueur `:213`, le local `:250`) ; ses 8 lignes B9 intactes ; `jo_kw_tuple` (`:36-43`) gardée | idem |
| H-D17-5 | `test_pico8_lq` : **66 / 71** ; D.17 change `:476`, `:482`, `:499` en place (numéros inchangés) ; aucune aide nouvelle ne nomme les clés ; `CONSTRUCTIONS_HASH`, `JO_EX_HASH`, `HP_LQ_HASH`, `CALLS_HASH`, `PANEL_HASH` inchangées | `git diff b4ec4b3 <HEAD> -- tests/test_pico8_lq.c` |
| H-D17-6 | Copies `*_truthy(pico8_value_t)` : **2** (`layout.c:19`, `ui_button.c:27`) | grep 3 de la Sortie de D.17 |
| H-D17-7 | Fonctions publiques qui nomment `pico8_value_t` : **4** (`pico8_lq_keys_of_row`, `pico8_lq_add`, `pico8_pj_item_of`, `pico8_js_pq`) | grep de l'Annexe B |
| H-D17-8 | `Makefile`, `tools/`, `DATA_ENGINE_SRCS`, attendus de `class-coverage-net` inchangés ; 26 e2e | `git diff --stat b4ec4b3 <HEAD> -- Makefile tools` |
| H-D17-9 | D.17 en un commit (`refactor(D.17)`), sans repli D.17a/b ; un repli ne changerait que des numéros de ligne | journal |
| H-D17-10 | Ligne D.18a du plan inchangée ; la note « Découvert à D.17 » dit : 5 lignes `ml` parties, `research_display.c` réduit à ses 10 lignes de cases | `docs/plan-3.md` |
| H-D17-11 | `pico8_value_tostr` garde deux appelants (`pi.c`, `score_panel_display.c`), que D.18a ne touche pas ; `pico8_value_equal` n'a plus d'appelant dans `src/` | greps 1 et 4 de la Sortie de D.17 |

**Sections à revérifier à la graduation de D.17** : « État de départ » (chiffres et critères `FAILED:` de départ) ; « Inventaire » (colonnes « après D.17 » de `planet_display.c`, `research_display.c`, `test_pico8_lq.c`) ; « Effet sur `make data-residue` » ; tableau « Runs », lignes 1 à 3 ; cibles des mutations dans ces trois fichiers ; Annexe A (ligne D.17 avec sa note, ligne D.18a) ; « Budget » (si le temps réel de D.17 s'écarte de ≈ 20 min, recalibrer). Si une hypothèse tombe, `plan-pilot` renvoie ce PRD à son auteur avant de le mettre en file.

### Preuve B8, par le cart

`lq:mn` (lua.lua:300-312), recopiée :

```lua
function lq:mn(f)local w,qw,qv,fn=6,0
if type(f)=="table" then
if (f.qe)f={f.qe,5}
f,w,qw,qv=f[1],f[2],f[3]or qw,f[4]
end
if type(f)=="string" then
…
elseif f then
fn=function(p)dv(qw)spr(f,p.x,p.y)dv()end
end
return fn or function()end,w,qv or 2
end
```

- Un positionnel est une table (tuple ou `{qe=x}`), une chaîne, ou autre chose ; `elseif f` range **nil et `false` ensemble** (« rien ») ; tout autre vrai qui n'est pas un nombre dessine `spr(f)`, soit le sprite 0 dans le port. C'est l'union de D.12c, et **aucun lecteur de `lq:mn` ne sépare nil de `false`** (G2) : la case `NOTHING` les confond à bon droit.
- Dans un tuple, `f[1]` suit la même règle ; `f[2]` n'est lu que nombre (le port : sinon 6), `f[3]` que nombre (`or qw`), `f[4]` en vérité puis nombre (sinon 0).

### Preuve B8, par le port : où naissent les clés

Un **producteur** est un site qui écrit les positionnels d'un `pico8_lq_keys_t` (ou d'un `pico8_lq_add`). Inventaire complet par `grep -rn 'pico8_lq_keys_t\|pico8_lq_keys_of_row\|pico8_lq_add\|pico8_lq_new(' src tests` à `b4ec4b3` (le PRD de D.12c en comptait « dix » ; il y en a quinze dans `src/`, trois TU) :

| # | Site (`b4ec4b3`) | Ce que le cart écrit | Genres écrits aujourd'hui | Case écrite après D.18a |
|---|---|---|---|---|
| P1 | `layout.c:220-246` `pico8_jb_line` (`jb:pq`) | `lq(type(ln)=="string" and {ln} or ln)` | chaîne ; items d'une ligne (chaîne, nombre, faux) | `pico8_lq_cell_string(item->s)` ; items par `pico8_lq_scalar_item` (voir « Représentation ») |
| P2 | `layout.c:368-383` `pico8_lq_keys_of_row` | clés d'une rangée générée | sous-rangée → `REF`, sinon `pico8_jb_item_value` | `pico8_lq_cell_item(&row->items[i])` ; appelée par P6, P7, P8, P10 (icône), P11 |
| P3 | `menu.c:94-122` `pico8_cr_step` | `{t.nk, t.px, c=6}`, `h_={{24,5}, gi}` | nombre, chaîne ; tuple, chaîne | `pico8_lq_cell_fixed`, `_string`, `_row(&pair)` |
| P4 | `menu.c:185-200` rangée « n invented » | `{30, gr.." invented", c=3}` | nombre, chaîne | `_fixed`, `_string` |
| P5 | `menu.c:272-292` `pico8_ix2_hb_dispatch` | `lq({#cb()>0 and 24 or 37})` | nombre | `_fixed` |
| P6 | `menu.c:342-376` `pico8_ix4_my_dispatch` (`ha`) | rangées de `ha` | par P2 | tableau `pico8_lq_cell_t items[…][…]` |
| P7 | `menu.c:378-394` `pico8_ix_button` (`ix`) | rangées de `ix` | par P2 | idem |
| P8 | `help.c:43-62` `pico8_fv_pq` (`bo`) | rangées de `bo` | par P2 | idem |
| P9 | `msgbox.c:333-358` `pico8_pz` (`ep`) | `{p.ep, c=13}` sous `if p.ep` | chaîne (non NULL) | `_string` |
| P10 | `planet_display.c:139-298` `pico8_jo_gh` | icône (P2) ; `h_` `{"$…", bg=…}` ; rangées « needs: », « wants more: », « receives: », « makes: »/« exports: » + `{qe=kw.qe}`, écart `{false,1}` | chaîne, `&jy->row` par premier membre (`jx.items[i]`, `jy->items[i]`, `ctx.ji.items[i]`, `void *`), tuple (`jo_kw_tuple`, `&gap`) | `_string`, `_row(…)` ; `jo_gh_row_items` alloue des cases |
| P11 | `planet_display.c:352-386` `pico8_jo_fe_keys` | rangée `b.lq` (P2) ; `h_` `{"$…", c=13}` | par P2 ; chaîne | tableau de cases ; `_string` |
| P12 | `research_display.c:143-280` `pico8_jo_ex` | `l` : tuples de besoins, `{false,d+lc}`, `{35|32,…}`, `tostr(jq)`, `{false,-4}`, tuples `{qq,6,…}`, `em` | `&tuple->row`, chaîne, `&t3`, `pico8_jo_em()` | `_row`, `_string` ; `l` alloué en cases |
| P13 | `research_display.c:309-311` `hp:lq` (`add`) | `add(l, hp.fk)`, `add(l, self.px)` | `pico8_hp_fk()`, chaîne | `pico8_lq_add(l, pico8_lq_cell_row(pico8_hp_fk()))`, `_string` |
| P14 | `score_panel_display.c:176-253` `pico8_pj_ph_map` | positions d'une rangée de `bz` (trous fermés, TROU-25) | par `pico8_pj_ph_item_value` : chaîne, sous-rangée, **faux**, nombre | `pico8_lq_cell_item(&st->items[k])` |
| P15 | `score_panel_display.c:256-296` `pico8_pj_ph` (boutons) | `{" again? "}`, `{{10,7}}` | chaîne, `&back` | `_string`, `_row(&back)` |

TU producteurs : `test_pico8_lq` (`positionals`, `str`, `num`, `tuple_of`, tableaux de `test_constructions`, `test_hp_lq_and_copy`, `test_calls`, `test_lq_is_a_struct`, `keys_of_row`), `test_pico8_js` (`build`, `fold_keys`), `test_pico8_layout_nil_item_draws_nothing` (`render_one_item`). **Aucun autre TU** ne nomme `pico8_lq_keys_t`, `pico8_lq_keys_of_row` ou `pico8_lq_add` (`ga`, `hp`, `pj` lisent les cases stockées, inchangées : ils prouvent sans suivre ; l'étude nommait `ga` et `hp` : mesuré, rien à suivre).

**Genres réellement produits dans `src/` au premier niveau** : nombre, chaîne non NULL, rangée (non NULL, sauf peut-être `jx.items[i]`/`jy->items[i]` — le port traite déjà un NULL en « rien »), faux (P14 seul, positions `f` de `bz`), nil (jamais au premier niveau à `b4ec4b3` : `pico8_jb_item_value` sur `is_ref` NULL n'apparaît que **dans** un tuple, P12 `:207`). **Aucun producteur n'écrit `TRUE`** : l'ancien chemin ne le donnait que pour un `BOOL` vrai, une `STRING` NULL, ou un `is_ref` non NULL dans un tuple, qu'aucun site n'écrit. `PICO8_LQ_CELL_TRUE` reste dans l'énumération (lue par les plis de TU, D.19b) ; rien ne le construit plus (« Surprises »).

**Chaînes : aucun producteur ne passe NULL** (preuve par site) : littéraux ; `pico8_tostr_fixed`, `jo_dollar`, `cr_concat2` rendent une chaîne allouée ; `t->px` de `nb` (table générée) ; `frame->ep` sous `if (has_ep)` ; `self->px` sous `if` ; `item->s` sous `if (item->s != NULL)`. Seul piège : `test_pico8_js.c:781` calcule `pico8_value_string(h_text)` même quand `h_text` est NULL (valeur jetée) — voir « Les TU suivent ».

### Le piège « rien » (décisif pour ce PRD)

L'union range nil **et** faux dans `NOTHING` (conforme au cart). Or **`CONSTRUCTIONS_HASH` de `test_pico8_lq` plie le genre de la clé d'où vient chaque case « rien »** (`fold_cell`, `:193-206` : `fold_value(h, *from, 2)`, soit les octets `fnv(fnv(h, 2u), 0u)` pour faux — `PICO8_VALUE_BOOL` vaut 2 — et `fnv(fnv(h, 3u), 0xDEADu)` pour nil — `PICO8_VALUE_REF` vaut 3, `pico8_value.h:14-19`). Ses rangées « false » (`:421-425`), « nil » (`:426-430`) et « mixed c » (`:431-438`) ne diffèrent que par là. Quand les clés deviennent des cases, **la clé ne sait plus si elle était nil ou faux** : un pli qui lirait la clé ferait rougir la constante sans bug.

Où l'information compte, mesuré (M0 de D.12c, confirmé par lecture à `b4ec4b3`) :

- **Seul `fold_built_from` → `fold_row_from(…, from = k)` → `fold_positionals` lit les clés** (`:209-222`), et seul `test_constructions` l'appelle avec des clés (14 appels, `:392-458`). `fold_row` (tous les autres plis de `lq`) passe `from = NULL` : case « rien » pliée comme faux.
- Les autres TU (`ga:489`, `hp:261`, `js:283`, `pj:521`) plient les **cases stockées**, que D.18a ne change pas (mêmes genres, même contenu) : ils plient « rien » comme faux, aujourd'hui comme demain.
- Dans `test_constructions`, les clés « rien » sont : « false » position 0 (faux) ; « nil » position 0 (nil) ; « mixed c » position 3 (faux) et position 5 (nil) — la position 2, écrite nil, est **remplacée** par `tuple_of(2, f)` (`:435`) avant la construction, donc pliée comme rangée.

**Décision (G) : le producteur ne garde pas l'information ; le TU la tient à côté de ses clés.** `fold_built_from`, `fold_row_from` et `fold_positionals` reçoivent un masque **`unsigned nil_at`** (bit `i` : la clé `i` était nil, pas faux) ; une case « rien » se plie avec les octets de nil si son bit est posé, de faux sinon ; `fold_row` passe `NULL, 0u`. Masques : « nil » `1u << 0`, « mixed c » `1u << 5`, tous les autres `0u`. Trois `assert` dans `fold_positionals` empêchent le masque de mentir : `from != NULL || nil_at == 0` ; `(nil_at >> n) == 0` ; un bit posé tombe sur une case `NOTHING`. `CONSTRUCTIONS_HASH` ne bouge pas.

Rejetés :
- **Séparer nil et faux dans l'union** (`PICO8_LQ_CELL_NIL`/`FALSE`, ou un drapeau dans `NOTHING`) : un état que `src/` ne lit jamais, ajouté pour un TU ; contredit le contrat de D.12c et `lq:mn` (G2).
- **Passer l'information par la charge `as` d'une case `NOTHING`** (le TU écrirait un marqueur que la construction recopie) : lit un membre que le contrat déclare sans objet ; canal caché.
- **Garder les tableaux de valeurs dans le TU et les convertir en cases** (une `cell_of_value` de TU) : recopie le tri retiré de `src/`, cale valeur ↔ typé interdite.
- **Un tableau parallèle `{case, mot de genre}` par rangée** : deux tableaux par rangée pour quatre positions ; le masque dit la même chose en un entier.

**Vérification obligatoire M0′, avant toute conversion** (sur le modèle de M0 de D.12c) : copie `cp` de `test_pico8_lq.c`, instrumentée dans `fold_positionals` (quand `from != NULL`) pour imprimer, par rangée (`label`, que `fold_built_from` passe déjà à `printf`) et par position, le genre de chaque clé qui donne une case `NOTHING` (nil = `REF` NULL, faux = `BOOL` 0) ; un lancement, restauration par `cp`, `touch`. **Attendu exactement** : `false: 0 faux` ; `nil: 0 nil` ; `mixed c: 3 faux, 5 nil` ; rien d'autre. Les masques sont ceux que M0′ imprime ; **tout écart avec ce PRD est rapporté au message** (le masque suit M0′). Consigné au tableau « Runs ». Après conversion, la mutation **M1 (exigée)** intervertit les masques et doit faire rougir `CONSTRUCTIONS_HASH`.

### Lecture de la ligne pour D.18a

- **« `pico8_lq_keys_t.items` en cases de l'union de `layout` »** : `const pico8_lq_cell_t *items` ; le constructeur copie les cases (`self->items[k] = keys->items[k]`), plus de tri à la construction.
- **Constructeurs** (étude, règle B8 ; **(G)** cinq, publics, dans `layout.c`, déclarés dans `layout.h`) :

  ```c
  /* Cells as the producers of keys write them. */
  pico8_lq_cell_t pico8_lq_cell_nothing(void);
  pico8_lq_cell_t pico8_lq_cell_fixed(pico8_fixed_t x);
  /* `s` borrowed; a NULL `s` stops the run: no producer writes one. */
  pico8_lq_cell_t pico8_lq_cell_string(const char *s);
  /* `row` borrowed; NULL is nil: NOTHING. */
  pico8_lq_cell_t pico8_lq_cell_row(const pico8_jb_row_t *row);
  /* A generated item as `lq:mn` tells it apart: a row, a string, a number;
   * false and a nil `ref` are NOTHING. */
  pico8_lq_cell_t pico8_lq_cell_item(const pico8_jb_item_t *item);
  ```

  (Forme de décision, pas code à recopier ; les contrats en commentaire sont ceux que `CLAUDE.md` admet.) **Pourquoi des constructeurs ici et des littéraux à D.17** : une case porte un genre **et** un membre d'union qui doivent s'accorder ; le constructeur l'impose en un point (`(pico8_lq_cell_t){.kind = PICO8_LQ_CELL_FIXED, .as.s = …}` compilerait) ; il y a quinze producteurs, pas deux. Rejeté : un constructeur `TRUE` (aucun producteur, G4 ; il serait sans appel et hors `gcov`).
- **`pico8_lq_cell_item`** remplace à la fois `pico8_jb_item_value`, `pico8_lq_scalar_cell`, `pico8_lq_cell_of`, `pico8_layout_truthy` et `pico8_pj_ph_item_value` : `row` → `ROW` ; `s` → `STRING` ; `is_false` → `NOTHING` ; `is_ref` → `pico8_lq_cell_row(item->ref)` (NULL : `NOTHING`, non NULL : `ROW`, conversion implicite de `const void *`, **sans cast**, comme l'affectation `cell.as.row = v.as.ref` d'aujourd'hui) ; sinon `FIXED` de `fixed` (si `is_fixed`) ou de `n`. Écrite **au site de `pico8_jb_item_value`** (`layout.c:29-45`).
- **Garde « scalaires seulement »** (`assert(item->row == NULL && "a tuple or a line holds scalar items only; …")`, `layout.c:31`) : gardée, au site de `pico8_lq_scalar_cell`, dans une `static pico8_lq_cell_t pico8_lq_scalar_item(const pico8_jb_item_t *item)` (garde puis `pico8_lq_cell_item`) qu'appellent `lq:mn` (tuples) et `pico8_jb_line` (items d'une ligne). **Écart mesuré, sans effet** : un `is_ref` non NULL dans un tuple devenait `TRUE`, il devient `ROW` ; `lq:mn` n'entre plus dans la branche tuple une fois `f` réaffectée (`:104`), et `TRUE` comme `ROW` donnent le sprite 0 (`:131-133`), `f[2..4]` ne lisant que `FIXED` et `≠ NOTHING` : dessin identique ; aucun producteur n'écrit un tel item (« never set by generated data », `map_tables.h:35`).
- **`lq:mn`, tuples** (`layout.c:93-114`) : `f1..f4` en `pico8_lq_cell_t` (`pico8_lq_scalar_item(&items[k])` ou `pico8_lq_cell_nothing()` au-delà du compte) ; `f = f1` ; `w` si `f2.kind == PICO8_LQ_CELL_FIXED` ; `qw` si `f3.kind == PICO8_LQ_CELL_FIXED` (« vrai et nombre » = « nombre ») ; `have_qv` si `f4.kind != PICO8_LQ_CELL_NOTHING`, `qv` = `f4.as.fixed` si `FIXED`, sinon 0. `pico8_layout_truthy` part (dernier appelant, `-Werror`) : **copies `truthy` 2 → 1** (reste `ui_button`, D.18b).
- **Le piège de `layout.c:14-18`** (« La clause REF-NULL n'est pas optionnelle… ») suit la clause qui le porte désormais : la branche NULL de `pico8_lq_cell_row` (celle par laquelle un `is_ref` nil devient « rien »). Reformulé (quatre sortes de `CLAUDE.md`, ≤ 40 mots), nommant `layout_nil_item_draws_nothing`.
- **`pico8_lq_keys_of_row(const pico8_jb_row_t *row, pico8_lq_cell_t *cells, int cap)`** : `cells[i] = pico8_lq_cell_item(&row->items[i])` (une sous-rangée devient `ROW` par la même fonction ; l'`assert` `:372-373` gardé).
- **`pico8_lq_add(pico8_lq_t *self, pico8_lq_cell_t cell)`** : range la case telle quelle.
- **§6 `layout_nil_item_draws_nothing`, « réécrit dans sa lecture »** (G6) : `render_one_item` prend un **`pico8_jb_item_t`** (item généré), le met dans une rangée d'un item et passe par **`pico8_lq_keys_of_row`** (`pico8_lq_cell_t cells[1]`) puis `pico8_lq_new` ; sujet `(pico8_jb_item_t){.is_ref = 1}` (nil), contrôle `(pico8_jb_item_t){.n = 0}` (le nombre 0) ; assertions de case (`NOTHING` ; `FIXED` 0) et de pixels **inchangées**. Même intention (nil ne dessine rien ; 0 dessine le sprite 0 là où la mise en page le place). Commentaire de `render_one_item` (`:103-105`) réécrit ; commentaire de tête (`:1-21`) vrai tel quel (il date la mesure et nomme `layout.c`). **Preuve rouge M2 (exigée)** : la branche NULL de `pico8_lq_cell_row` rend `TRUE` → rouge sur l'assertion de case ; relancé avec cette seule assertion neutralisée dans une copie du TU → rouge sur les pixels (sprite 0 trouvé) ; les deux rapportées.
- **§6 `planet_gh_row_lq`, « réécrit dans sa lecture »** : **rien à réécrire, déclaré** (écart à la lettre de la ligne) : il lit déjà des cases stockées (D.12c2 : `lq_item`, `lq_index_is_string`, genres `STRING`/`ROW`, `fs_count`), et D.18a ne change ni le type ni le contenu de `lq.items`. Ce qui change sous lui, c'est exactement ce qu'il garde : la rangée « exports: » de `jo:gh` (`jo_gh_row_items` alloue désormais des cases — le pas d'élément du bogue d'origine) et la copie clés → cases de `pico8_lq_struct_new`. Texte inchangé (son include `pico8_value.h:35`, seule ligne `valeur`, reste pour D.19c) ; **preuves rouges refaites, exigées** : **M3** la copie prend une case de trop (`pico8_lq_struct_alloc(keys->count + 1)`, boucle jusqu'à `keys->count` inclus) → rouge (ASan : lecture hors de `row_items` ; et « pas de cinquième » `:157`) ; **M4** `jo_gh_row_items` alloue une case de moins → rouge (ASan : écriture de l'écart hors bloc). Lancé comme le `Makefile` : `ASAN_OPTIONS=detect_leaks=0 ./tmp/test_pico8_planet_gh_row_lq`.
- **§6 `ga_export_panel_by`** : lit des cases stockées, inchangé, recompilé, relancé ; aucune preuve exigée.
- **« `sanitize-net` : oui »** : exigé, résultat rapporté. Les durées de vie ne changent pas (les cases empruntent chaînes et rangées comme les valeurs le faisaient ; `jo_gh_row_items` et les tuples jamais libérés, TROU-7 ; `l` de `jo.ex` libéré après `pico8_lq_new`, qui a copié les cases), mais la taille des blocs alloués change (`sizeof *items` : une case au lieu d'une valeur) : c'est l'objet du filet.
- **`class-coverage-net`** : non demandé par la ligne ; **(G) exigé quand même**, sortie identique : le diff touche le constructeur `lq` (`pico8_lq_struct_new`, la ligne de copie) — §7, « aux étapes dont le diff touche un constructeur ».
- **« Compte : ≈ 560 »** : mesuré **− 133 lignes / − 140 occ.** depuis ≈ 693 (H-D17-1) : **≈ 560** (la coïncidence avec l'estimation est réelle : l'étude comptait B8 plus les autres parts de D.18a sur une base de 720). « ≈ » n'est pas un critère ; les deltas exacts le sont.
- **Lignes prises à une autre étape** : `score_panel_display.c:181` (`pico8_value_t items[5];`, les clés de `pj:ph`) était rangée **B11** (D.18c) par l'Annexe A de l'étude (`:180` à `b7f6c02`) ; son type suit celui des clés : prise ici, déclarée. Aucune autre.
- **Découpage** : **(G) un commit** (`refactor(D.18a)`). Le type des clés change d'un coup pour tous les producteurs ; le pont compté `pico8_lq_cell_of_value` de l'étude n'éviterait aucune ligne (chaque site de producteur changerait deux fois). Repli écrit plus bas.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes comptées, alias, renommage pour échapper aux greps, cast nouveau, cale valeur ↔ typé (dont un pont), aide de TU laissée sans appel (`-Werror`) ; mutations restaurées par `cp` simple puis `touch` ; binaires lancés exactement comme le `Makefile` ; **jamais l'appli raylib**.

### État de départ

| Mesure | À `b4ec4b3` (D.17 non commencée) | Départ de D.18a (H-D17-1, à confirmer) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 745 / 89 ; 831 occ. | **693 / 85 ; 763 occ.** (`src/` 270 / 47 / 301, `tests/` 423 / 38 / 462) |
| `type-tag` | 242 / 58 | **242 / 58** |
| résidu D | 985 / 104 | **933 / 102** |
| fonctions publiques qui nomment `pico8_value_t` (Annexe B) | 7 | **4** |
| copies `*_truthy(pico8_value_t)` | 3 | **2** |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |

Recette des occurrences (celle de D.12–D.17, motif `valeur` de `tools/data_residue.sh:29`) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'valeur\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire

Classes du message de commit : **(a1)** même site, type C (case) à la place de la valeur ; **(a2)** retirée sans remplaçant au site (fonction sans appel, copie `truthy`) ; **(b)** signature ou membre public dont le type change ; **(c)** même donnée, autre nom : **attendu 0**. `occ.` : occurrences du motif. Chaque ligne listée **sort** de `valeur` ; aucune n'en gagne.

#### API (`layout.h`, `layout.c`)

| Symbole | Aujourd'hui | Après D.18a | Classe |
|---|---|---|---|
| `pico8_lq_keys_t.items` (`layout.h:100`) | `const pico8_value_t *items;` | **`const pico8_lq_cell_t *items;`** ; contrat `:95-97` réécrit (« each a cell, copied into `lq.items` ») | (b) |
| `pico8_lq_keys_of_row` (`.h:118`, `.c:368`) | `(…, pico8_value_t *items, int cap)` | **`(…, pico8_lq_cell_t *cells, int cap)`** ; contrat `.h:114-117` (« a nested row, a tuple, as a ROW cell ») | (b) ×2 |
| `pico8_lq_add` (`.h:126`, `.c:404`) | `(pico8_lq_t *self, pico8_value_t v)` | **`(pico8_lq_t *self, pico8_lq_cell_t cell)`** ; contrat `.h:123-125` | (b) ×2 |
| `pico8_lq_cell_nothing/fixed/string/row/item` | — | **nouveaux**, hors motif | — |
| `pico8_layout_truthy`, `pico8_lq_scalar_cell`, `pico8_lq_cell_of` (`static`) | — | **retirées** (sans appel, `-Werror`) | (a2) |
| `pico8_jb_item_value` (`static`) | rend une valeur | **devient `pico8_lq_cell_item`** au même site | (a1) |
| `pico8_pj_ph_item_value` (`static`, `score_panel_display.c`) | rend une valeur | **retirée** ; son appel devient `pico8_lq_cell_item` | (a2) |
| `pico8_lq_cell_t`, `pico8_lq_cell_kind_t` | — | **inchangés** ; commentaire `:29-32` (« set once when the row is built ») réécrit : les cases sont écrites par les producteurs de clés | — |

#### Sites de `src/` (89 lignes, 89 occ. retirées ; aucune gagnée)

| Fichier | Lignes `valeur` à `b4ec4b3` (après D.17) | Remplacement | Classe |
|---|---|---|---|
| `layout.c` | `:19`, `:20`, `:23` | `pico8_layout_truthy` retirée entière (commentaire `:14-18` déplacé sur la branche NULL de `pico8_lq_cell_row`) | (a2) 3 |
| | `:30`, `:33`, `:36`, `:39`, `:42`, `:44` | `pico8_lq_cell_t pico8_lq_cell_item(const pico8_jb_item_t *item)` et ses retours (`_row`, `_string`, `_nothing`, `_row(item->ref)`, `_fixed`) ; l'`assert` `:31` passe à `pico8_lq_scalar_item` | (a1) 6 |
| | `:49` | `static pico8_lq_cell_t pico8_lq_scalar_item(const pico8_jb_item_t *item) {` (garde `:31`, puis `pico8_lq_cell_item`) | (a1) 1 |
| | `:52`, `:55` | corps de `pico8_lq_scalar_cell` retiré | (a2) 2 |
| | `:67`, `:70` | `pico8_lq_cell_of` retirée entière avec son commentaire `:64-66` (le contrat « a REF is read as a row » est déjà dans `layout.h:43-44`) | (a2) 2 |
| | `:93` | `pico8_lq_cell_t f1, f2, f3, f4;` | (a1) 1 |
| | `:99`–`:102` | `fk = have_fk ? pico8_lq_scalar_item(&items[k-1]) : pico8_lq_cell_nothing();` | (a1) 4 |
| | `:104` (0) | `f = f1;` | — |
| | `:105`, `:108`, `:113` | `f2.kind == PICO8_LQ_CELL_FIXED` ; `f3.kind == PICO8_LQ_CELL_FIXED` ; `(f4.kind == PICO8_LQ_CELL_FIXED) ? f4.as.fixed : …` ; `:111` (0) devient `have_f4 && f4.kind != PICO8_LQ_CELL_NOTHING` | (a1) 3 |
| | `:222`, `:223` | `pico8_lq_cell_t s;`, `pico8_lq_cell_t items[PICO8_JB_LINE_CAP];` | (a1) 2 |
| | `:227` | `s = pico8_lq_cell_string(item->s);` | (a1) 1 |
| | `:239` (0) | `items[i] = pico8_lq_scalar_item(&item->row->items[i]);` | — |
| | `:348` (0) | `self->items[k] = keys->items[k];` | — |
| | `:368` | signature (b) | (b) 1 |
| | `:375` (`:376` 0) | `cells[i] = pico8_lq_cell_item(&row->items[i]);` (deux lignes jointes en une : **permis**, `:376` ne porte pas le motif) | (a1) 1 |
| | `:378` (0) | `keys.items = cells;` | — |
| | `:404` | signature (b) | (b) 1 |
| | `:409` (0) | `self->items[self->items_count] = cell;` | — |
| | (ajout) | les cinq constructeurs, après `pico8_lq_mn`/avant `pico8_lq_pq` ou en tête (au choix, un bloc) | — |
| `layout.h` | `:100`, `:118`, `:126` | voir API ; include `:9` **gardé** (règle des en-têtes, D.19c) | (b) 3 |
| `menu.c` | `:86`, `:88` | `pico8_lq_cell_t lq_items[2];`, `pico8_lq_cell_t h_items[2];` | (a1) 2 |
| | `:100`, `:101` | `pico8_lq_cell_fixed(pico8_fixed_from_int(t->nk))`, `pico8_lq_cell_string(t->px)` | (a1) 2 |
| | `:112`, `:113` | `pico8_lq_cell_row(&pair)` (cast `(void *)` retiré), `pico8_lq_cell_string(gi_str)` | (a1) 2 |
| | `:190`, `:191` | `_fixed(… 30)`, `_string(label)` | (a1) 2 |
| | `:275`, `:286` | `pico8_lq_cell_t item;`, `item = pico8_lq_cell_fixed(…);` | (a1) 2 |
| | `:351`, `:382` | `pico8_lq_cell_t items[…][PICO8_MENU_ROW_CAP];`, `pico8_lq_cell_t items[PICO8_MENU_ROW_CAP];` | (a1) 2 |
| | B9 `:106,117,121,196,199,364,387` ; include `:24` | **inchangés** (D.18b ; l'include reste : le fichier nomme encore le module) | — |
| `help.c` | `:46` | `pico8_lq_cell_t items[PICO8_BO_COUNT][PICO8_BO_ROW_CAP];` ; B9 `:58` inchangée | (a1) 1 |
| `msgbox.c` | `:334`, `:351` | `pico8_lq_cell_t ep_cell;`, `ep_cell = pico8_lq_cell_string(frame->ep);` (le local change de nom avec son type : `ep_val` → `ep_cell`, (a1) et non (c) — la donnée est une case, plus une valeur) ; B9 `:356` inchangée | (a1) 2 |
| `planet_display.c` | `:106`, `:107` (`:98`, `:99`) | `static pico8_lq_cell_t *jo_gh_row_items(int n) {`, `pico8_lq_cell_t *items = malloc((size_t)n * sizeof *items);` | (a1) 2 |
| | `:142`, `:144`, `:149` (`:134`, `:136`, `:141`) | `pico8_lq_cell_t icon_items[…]`, `pico8_lq_cell_t h_item;`, `pico8_lq_cell_t *row_items[…]` | (a1) 3 |
| | `:183` (`:175`) | `h_item = pico8_lq_cell_string(jo_dollar(…));` | (a1) 1 |
| | `:202`, `:221`, `:239`, `:260`, `:261`, `:275`, `:276` (`:194`, `:212`, `:230`, `:250`, `:251`, `:265`, `:266`) | `pico8_lq_cell_string(…)` | (a1) 7 |
| | `:204`, `:223`, `:226`, `:241` (`:196`, `:214`, `:217`, `:232`) | `pico8_lq_cell_row(jx.items[i])` etc. (`void *` → `const pico8_jb_row_t *`, implicite, **sans cast**) | (a1) 4 |
| | `:278`, `:290` (`:268`, `:280`) | `pico8_lq_cell_row(jo_kw_tuple(kw))`, `pico8_lq_cell_row(&gap)` (casts `(void *)` retirés) | (a1) 2 |
| | `:347`, `:349`, `:367` (`:337`, `:339`, `:357`) | `pico8_lq_cell_t lq_items[…]`, `pico8_lq_cell_t h_item;`, `row->h_item = pico8_lq_cell_string(jo_dollar(…));` | (a1) 3 |
| | B9 `:173,179,292,359,361,365,373,384` (`:165,171,282,349,351,355,363,374`) | **inchangés** (D.18b) | — |
| `research_display.c` | `:145` (`:133`) | `pico8_lq_cell_t *l;` (`malloc(… sizeof *l)` inchangé) | (a1) 1 |
| | `:212`, `:223`, `:232`, `:266` (`:197`, `:208`, `:217`, `:250`) | `l[l_count++] = pico8_lq_cell_row(&tupleX->row);` | (a1) 4 |
| | `:240`, `:241` (`:225`, `:226`) | `pico8_lq_cell_string(jq_str)`, `pico8_lq_cell_row(&t3)` | (a1) 2 |
| | `:269` (`:253`) | `pico8_lq_cell_row(pico8_jo_em())` | (a1) 1 |
| | `:309`, `:311` (`:293`, `:295`) | `pico8_lq_add(l, pico8_lq_cell_row(pico8_hp_fk()));`, `pico8_lq_add(l, pico8_lq_cell_string(self->px));` | (a1) 2 |
| `score_panel_display.c` | `:127`, `:129`, `:132`, `:135`, `:138`, `:140` | `pico8_pj_ph_item_value` retirée entière avec son commentaire `:126` | (a2) 6 |
| | `:181` (**pris à B11**) | `pico8_lq_cell_t items[5];` | (a1) 1 |
| | `:232` (0) | `items[keys.count++] = pico8_lq_cell_item(&st->items[k]);` | — |
| | `:262`, `:263` | `pico8_lq_cell_t lq1_item;`, `pico8_lq_cell_t lq2_item;` | (a1) 2 |
| | `:271`, `:286` | `pico8_lq_cell_string(" again? ")`, `pico8_lq_cell_row(&back)` | (a1) 2 |
| | B11 `:144-159`, `:179`, `:195`, `:197` ; B9 `:279`, `:294` | **inchangés** (D.18c, D.18b) | — |

Totaux `src/` (lignes / occ.) : `layout.c` 28 / 28, `layout.h` 3 / 3, `menu.c` 12 / 12, `help.c` 1 / 1, `msgbox.c` 2 / 2, `planet_display.c` 22 / 22, `research_display.c` 10 / 10, `score_panel_display.c` 11 / 11 : **89 lignes, 89 occ.** Classes : (a1) 71, (a2) 13, (b) 5.

**Écart de `pico8_pj_ph_item_value`, sans effet** : elle ne traitait pas `is_ref` (un tel item tombait sur `n` : fixe 0) ; `pico8_lq_cell_item` le rend `NOTHING`/`ROW`. Les rangées de `bz` sont générées (`is_ref` jamais posé) et `pj:ph` n'y écrit que `s` et `n` (`:196`, `:214`, `:222`) : inatteignable, déclaré.

#### Les TU suivent (44 lignes, 51 occ. retirées)

| TU | Lignes à `b4ec4b3` (après D.17 : identiques, H-D17-5) (occ.) | Nouveau texte | Classe |
|---|---|---|---|
| `lq` | `:175`, `:176`, `:178`, `:180`, `:182` (5) | `assert_sorted` devient `static int same_cell(pico8_lq_cell_t a, pico8_lq_cell_t b)` (même genre ; même `fixed`, même pointeur `s` ou `row` selon le genre) ; l'appel `:217` devient `assert(same_cell(c, from->items[i]));` : la construction copie la case, le TU le vérifie | (a1) 5 |
| | `:193` (1) | `static uint32_t fold_cell(uint32_t h, pico8_lq_cell_t c, int nil) {` | (a1) 1 |
| | `:202` (1) | `return nil ? fnv(fnv(h, 3u), 0xDEADu) : fnv(fnv(h, 2u), 0u);` sous un commentaire (piège) : « the bytes fold_value folded for a nil and a false key, which CONSTRUCTIONS_HASH holds apart » | (a1) 1 |
| | `:209`, `:219`, `:324`, `:328`, `:333`, `:337`, `:339` (0) | `nil_at` passé de `fold_built_from` à `fold_row_from` à `fold_positionals` ; `fold_row` passe `NULL, 0u` ; trois `assert` du masque (voir « Le piège ») | — |
| | `:289`, `:290`, `:293`, `:294` (4) | `num`, `str` rendent `pico8_lq_cell_t` (`pico8_lq_cell_fixed`, `pico8_lq_cell_string`) | (a1) 4 |
| | `:298`, `:307` (2) | `tuple_of` rend `pico8_lq_cell_t` (`pico8_lq_cell_row(t)`) | (a1) 2 |
| | `:349` (1) | `positionals(int n, const pico8_lq_cell_t *items)` | (a1) 1 |
| | `:390`, `:395`, `:400`, `:406`, `:414`, `:441`, `:446`, `:451`, `:456` (9) | `pico8_lq_cell_t a[…]` | (a1) 9 |
| | `:417` (1) | `a[0] = pico8_lq_cell_row(qe);` | (a1) 1 |
| | `:422`, `:427` (2 chacune) | `pico8_lq_cell_t a[] = {pico8_lq_cell_nothing()};` ; appels `:424` masque `0u`, `:429` masque `1u << 0` | (a1) 2 |
| | `:433` (3), `:434` (1) | `pico8_lq_cell_t a[] = {str("a"), num(12), pico8_lq_cell_nothing(), pico8_lq_cell_nothing(), str("bc"),` / `pico8_lq_cell_nothing(), num(40)};` (deux lignes gardées) ; appel `:437` masque `1u << 5` | (a1) 2 |
| | `:538`, `:539` (1 chacune) | `pico8_lq_cell_t lq_items[] = …`, `pico8_lq_cell_t h_items[] = …` | (a1) 2 |
| | `:574` (2) | `pico8_lq_cell_t a[] = {str("hi"), num(12), pico8_lq_cell_nothing()};` (la position 2 est remplacée par `tuple_of`, `:583`) | (a1) 1 |
| | `:849` (1) | `pico8_lq_cell_t items[2];` (`keys_of_row`) | (a1) 1 |
| | `:875`, `:876` (1 chacune) | `pico8_lq_cell_t a[] = …`, `pico8_lq_cell_t items[2];` | (a1) 2 |
| | `:903`, `:904` (1 chacune) | `assert(items[0].kind == PICO8_LQ_CELL_FIXED && items[0].as.fixed == 0x8000);`, `assert(items[1].kind == PICO8_LQ_CELL_ROW && items[1].as.row == &pico8_dr_table[0]);` | (a1) 2 |
| | **inchangés** : `fold_value`, `tuple_item_value`, `fold_tuple`, `cell_value_kind` (`:703`), `fold_fixed_member`, `fold_bg`, `vec_ref`, `with_ra`, `:198`, `:200`, `:204`, `:524`, `:859` (B9), D.17 `:476,482,499` | plis et aides de D.19b ; sources de bouton de D.18b | — |
| `js` | `:780` (2) | `pico8_lq_cell_t lq_item = pico8_lq_cell_string(lq_text);` | (a1) 1 |
| | `:781` (2) | **`pico8_lq_cell_t h_item;`**, et `h_item = pico8_lq_cell_string(h_text);` écrite **dans** le `if (h_text != NULL)` (`:789`) : aujourd'hui la valeur d'un `h_text` NULL est calculée puis jetée ; le constructeur l'arrêterait (piège) | (a1) 1 |
| | `:1319` (1) | `pico8_lq_cell_t from_keys[2];` | (a1) 1 |
| | `:1388`, `:1389` (1 chacune) | `pico8_lq_cell_string("from a list")`, `pico8_lq_cell_fixed(pico8_fixed_from_int(3))` | (a1) 2 |
| `layout_nil_item_draws_nothing` (§6) | `:106` (1) | `static pico8_lq_cell_t render_one_item(pico8_jb_item_t item) {` ; corps `:107-112` : `const pico8_jb_row_t row = {.count = 1, .items = &item};`, `pico8_lq_cell_t cells[1];`, `pico8_lq_keys_t keys = pico8_lq_keys_of_row(&row, cells, 1);`, `line = pico8_lq_new(&keys);` | (a1) 1 |
| | `:126` (1) | `cell = render_one_item((pico8_jb_item_t){.is_ref = 1});` | (a1) 1 |
| | `:153` (1) | `cell = render_one_item((pico8_jb_item_t){.n = 0});` | (a1) 1 |
| | include `:34` | **gardé** (les TU gardent leurs includes jusqu'à D.19c, règle de D.17) | — |

Totaux `tests/` (lignes / occ.) : `lq` 36 / 41, `js` 5 / 7, `layout_nil_item_draws_nothing` 3 / 3. **Total D.18a : 133 lignes, 140 occ.** ; **(a1) 115, (a2) 13, (b) 5, (c) 0.**

**Aides de TU et `-Werror`** : aucune aide ne perd son dernier appel (`fold_value`, `tuple_item_value`, `cell_value_kind`, `vec_ref` gardent leurs appels) ; `assert_sorted` devient `same_cell` (même site, un appel) ; aucune aide créée hors de `same_cell`.

#### Includes (règle de D.15b)

Un `.c` de `src/` qui ne nomme plus rien du module perd son include ; aucun n'est dans ce cas avec un include propre : `layout.c`, `research_display.c` sortent de `valeur` sans en avoir (include par leur en-tête) ; `menu.c:24` reste (B9). En-têtes gardés : `layout.h:9`, `research_display.h:9`, `help.h`, `msgbox.h`, `planet_display.h`. **Aucun include ajouté ni retiré**, TU compris.

#### `Makefile` et liens

- Aucun fichier nouveau ; **aucune ligne du `Makefile` ne change**.
- Règle de lien : aucun fichier de `DATA_ENGINE_SRCS` (`Makefile:229-279`) ne nomme un symbole défini hors de cette liste. Symboles ajoutés : les cinq constructeurs, définis dans `layout.c` (dans la liste), appelés par `layout.c`, `msgbox.c`, `planet_display.c`, `research_display.c` (dans la liste), `menu.c`, `help.c`, `score_panel_display.c` (hors liste : permis) et par les TU `lq`, `js`, `layout_nil_item_draws_nothing`. La liste courte de `layout_nil_item_draws_nothing` (`Makefile:561`) contient `layout.c` ; `planet_gh_row_lq` (ASan, `:550`) et `ga_export_panel_by` (`:553`) lient `DATA_ENGINE_SRCS` (grep 6).

## Solution

**Un commit** `refactor(D.18a)` (13 chemins : 8 de `src/`, 3 TU, `docs/plan-3.md`, `docs/glossaire-noms.md`), vert seul, vérifié, poussé :

1. Les clés d'une rangée `lq` sont des cases ; chaque producteur écrit sa case par un constructeur ; un item généré devient une case par `pico8_lq_cell_item`.
2. `layout` ne classe plus de valeurs : `pico8_layout_truthy`, `pico8_lq_scalar_cell`, `pico8_lq_cell_of` disparaissent ; `lq:mn` lit ses tuples en cases.
3. `pico8_lq_add` et `pico8_lq_keys_of_row` prennent et rendent des cases.
4. `test_pico8_lq` garde nil et faux séparés dans `CONSTRUCTIONS_HASH` par un masque vérifié ; aucune constante ne change.
5. `layout_nil_item_draws_nothing` passe son item nil par le tri d'un item généré ; `planet_gh_row_lq` garde la copie retypée ; preuves rouges refaites.

**Sortie**, constatée par commande :

- `make test` vert (dont `data-guard`, `constants-check`, `float-grep`, `float-guard`) ; `make e2e` **26 conformes** ; `make sanitize-net` **propre** ; `make class-coverage-net` **identique** à l'attendu ; balayage ASan+UBSan des TU propre ;
- `make data-residue` : `valeur` **693 → 560** lignes (**763 → 623** occ.), fichiers **85 → 83** (sortent `layout.c`, `research_display.c`) ; `type-tag` 242 / 58 inchangé ; résidu D **933 / 102 → 800 / 101** (`research_display.c` sort ; `layout.c` reste par `type-tag`) ; départs de H-D17-1 (sinon : départ réel − 133 / − 140 occ.) ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. layout.c et research_display.c vidés : 0 et 0
  grep -cE "$P" src/pico8_layout.c src/pico8_research_display.c
  # 2. layout.h : une seule ligne, l'include
  grep -nE "$P" src/pico8_layout.h
  # 3. copies truthy : 1 fichier (ui_button)
  grep -lE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c
  # 4. plus de tri valeur -> case : vide
  grep -rnE 'pico8_lq_cell_of\b|pico8_lq_scalar_cell|pico8_jb_item_value|pico8_pj_ph_item_value|pico8_layout_truthy' src tests
  # 5. clés et API : trois lignes, 'const pico8_lq_cell_t *items;',
  #    '… pico8_lq_keys_of_row(const pico8_jb_row_t *row, pico8_lq_cell_t *cells, int cap);',
  #    'void pico8_lq_add(pico8_lq_t *self, pico8_lq_cell_t cell);'
  grep -nE 'const pico8_lq_cell_t \*items;|pico8_lq_keys_of_row\(|void pico8_lq_add\(' src/pico8_layout.h
  # 6. règle de lien
  make tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_bg_kj tmp/test_pico8_planet_gh_row_lq \
       tmp/test_pico8_ga_export_panel_by tmp/test_pico8_layout_nil_item_draws_nothing
  # 7. aucune constante de TU changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 8. includes : vide
  git diff <parent> -- src tests | grep -E '^[-+]\s*#include'
  # 9. §6 : planet_gh_row_lq et ga_export_panel_by inchangés : vide
  git diff <parent> -- tests/test_pico8_planet_gh_row_lq.c tests/test_pico8_ga_export_panel_by.c
  # 10. masques du piège « rien » : deux lignes (« nil » 1u << 0, « mixed c » 1u << 5), ou celles de M0′
  grep -nE 'fold_built_from\(h, "[^"]*", &k, 1u' tests/test_pico8_lq.c
  # 11. comptes ; 12. filets
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^valeur'
  make class-coverage-net; make sanitize-net
  ```

## User Stories

1. En tant que mainteneur du port, je veux que les clés d'une rangée `lq` soient des cases de l'union de `layout`, afin qu'aucune valeur Lua ne soit fabriquée pour être aussitôt retriée.
2. En tant que mainteneur du port, je veux cinq constructeurs de case (rien, nombre, chaîne, rangée, item généré), afin qu'un genre et son membre d'union s'accordent toujours.
3. En tant que mainteneur du port, je veux qu'une rangée NULL donne une case « rien », afin que nil reste nil comme dans le cart.
4. En tant que mainteneur du port, je veux qu'une chaîne NULL arrête le programme, afin qu'un état qu'aucun producteur n'écrit ne soit pas lu en silence comme « vrai ».
5. En tant que mainteneur du port, je veux une seule fonction qui range un item généré en case, afin que `keys_of_row`, les lignes de `jb`, les tuples de `lq:mn` et `pj:ph` classent de la même façon.
6. En tant que mainteneur du port, je veux que la garde « un tuple ou une ligne ne tient que des scalaires » reste, afin qu'une sous-rangée inattendue ne soit pas dessinée en sprite 0.
7. En tant que mainteneur du port, je veux que `lq:mn` lise `f[1..4]` d'un tuple en cases, avec les mêmes règles de largeur, de palette et d'espacement, afin que chaque rangée soit dessinée au pixel près.
8. En tant que mainteneur du port, je veux que `pico8_layout_truthy`, `pico8_lq_scalar_cell`, `pico8_lq_cell_of` et `pico8_pj_ph_item_value` disparaissent avec leur dernier appel, afin qu'aucune fonction ne reste pour une valeur qui n'existe plus.
9. En tant que mainteneur du port, je veux que `pico8_lq_add` prenne une case, afin que `hp:lq` ajoute `fk` et `px` sans passer par la valeur.
10. En tant que mainteneur du port, je veux que `pico8_lq_keys_of_row` écrive des cases, afin que `menu`, `help` et `planet_display` fournissent des tableaux de cases.
11. En tant que mainteneur du port, je veux que `jo:gh`, `jo:fe`, `jo.ex`, `pj:ph`, `pz`, `cr`, `ix` et `fv:pq` écrivent leurs cases par constructeur, sans cast, afin que chaque producteur dise le genre de ce qu'il pose.
12. En tant que mainteneur du port, je veux que le piège « la clause nil n'est pas optionnelle » soit écrit là où nil devient « rien », afin qu'on ne le réintroduise pas en réécrivant ce tri.
13. En tant que gardien des tests, je veux que `test_pico8_lq` sépare nil et faux par un masque vérifié par trois `assert`, afin que `CONSTRUCTIONS_HASH` ne change pas et que le masque ne puisse pas mentir.
14. En tant que gardien des tests, je veux une mesure M0′ des clés « rien » avant la conversion, afin que les masques viennent d'une mesure et pas d'une lecture.
15. En tant que gardien des tests, je veux une mutation exigée qui intervertit les masques et fait rougir `CONSTRUCTIONS_HASH`, afin de prouver que le piège est tenu.
16. En tant que gardien des tests de régression, je veux que `layout_nil_item_draws_nothing` fasse passer un item nil généré par `pico8_lq_keys_of_row`, avec les mêmes assertions de case et de pixels, afin que le bogue d'origine soit gardé là où il se réintroduirait désormais.
17. En tant que gardien des tests de régression, je veux que `planet_gh_row_lq`, inchangé, rougisse quand la copie des cases ou l'allocation de `jo_gh_row_items` prend un élément de trop ou de moins, afin que le bogue de pas d'élément reste gardé après le changement de type.
18. En tant que gardien des tests, je veux que `test_pico8_js` construise l'`h_` de `build` seulement quand il existe, afin que le constructeur de chaîne ne s'arrête pas sur une valeur jetée.
19. En tant que gardien des filets, je veux `make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net` et le balayage ASan+UBSan verts, afin de prouver qu'aucun pixel, accès mémoire ou constructeur ne change.
20. En tant que gardien des tests, je veux `gcov` avant/après et des mutations restaurées par `cp`, afin que chaque ligne modifiée soit exécutée et qu'aucune ligne survivante ne cesse de l'être.
21. En tant que gardien du compteur, je veux au message du commit les comptes par catégorie et par fichier, en lignes et occurrences, la classification (a1)/(a2)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris), la ligne prise à B11, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
22. En tant que lecteur du glossaire, je veux que l'entrée `lq` dise que les clés sont des cases et nomme les constructeurs, afin qu'elle reste vraie.
23. En tant que `plan-pilot`, je veux une étape d'environ une demi-heure, un plafond, un repli et une note « Découvert à D.18a », afin que D.18b parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules, tests et découpage ; ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)** ; les rejets sont notés. Q12 (oui) et Q13 (A) sont des décisions humaines ; l'étude D.14 (B8, G1, G2, G4, G6) fait foi sauf écart déclaré ici (constructeurs `nothing`/`item` en plus ; pas de pont ; `planet_gh_row_lq` sans réécriture ; `ga`, `hp` hors empreinte ; `:181` de `score_panel_display.c` prise à B11 ; `class-coverage-net` exigé).

### Modules

- **Mise en page** : `pico8_layout.c/.h` (clés, constructeurs, tri d'item, `lq:mn`, `jb:pq`, `keys_of_row`, `add`).
- **Producteurs** : `pico8_menu.c`, `pico8_help.c`, `pico8_msgbox.c`, `pico8_planet_display.c`, `pico8_research_display.c`, `pico8_score_panel_display.c`.
- **TU** : `lq`, `js` (suivent), `layout_nil_item_draws_nothing` (§6, suit). **Docs** : `plan-3.md` (note), `glossaire-noms.md` (entrées `lq` et `pico8_lq_keys_of_row`, `:433-434`).
- Inchangés : `Makefile`, `tools/`, `pico8_value.c/.h`, `pico8_type_tag.h`, `pico8_lq_cell_t` et ses genres, `pico8_lq_t`, `pico8_lq_copy`, `pico8_jb_t`, `pico8_js_keys_t` et `pico8_js_pq` (B9), `score_panel.c/.h` (D.18c), `pi.c` (D.18c), `ui_button.c/.h`, les TU `ga`, `hp`, `pj`, `jb_ol`, `planet_gh_row_lq`, `ga_export_panel_by`, `test_pico8_value`, `docs/trous-de-portage.md`.

### Représentation (G)

- Clés : `const pico8_lq_cell_t *items` ; construction par copie.
- Cinq constructeurs publics (`layout.c`) ; pas de constructeur `TRUE`.
- `pico8_lq_cell_item` au site de `pico8_jb_item_value` ; `pico8_lq_scalar_item` (garde + tri) au site de `pico8_lq_scalar_cell`.
- `lq:mn` : tuples en cases, mêmes règles.
- **Piège de motif** : aucun nom neuf ne correspond à un motif (`pico8_lq_cell_*`, `pico8_lq_scalar_item`, `same_cell`, `nil_at` : ni `valeur`, ni `spec` — pas de suffixe `_default` —, ni `par-nom`, ni `liste`).
- **Piège de forme** : garder `f2`/`f3`/`f4` en valeurs, ou écrire une `pico8_lq_cell_of_value`, serait une cale ; interdit.

### Anti-tassement et message de commit (liant)

- **Interdit** : joindre des lignes comptées (`:433-434` de `test_pico8_lq` restent deux lignes ; `:222-223` de `layout.c` restent deux déclarations) ; la jonction de `layout.c:375-376` est **permise et déclarée** (`:376` ne porte pas le motif ; la ligne comptée sort) ; renommer pour échapper au motif ; alias (`#define`, `typedef` d'un nom existant) ; cast nouveau (les littéraux composés `(pico8_jb_item_t){…}` ne sont pas des casts ; les `(void *)` retirés ne sont pas remplacés) ; cale valeur ↔ typé ou pont ; toucher une constante de hachage, `tests/e2e/`, un `*.sha256`, `tools/`, le `Makefile`, un include.
- **Classification exigée** (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif en regard des `+`) : **(a1) 115, (a2) 13, (b) 5, (c) 0**, recomptée au parent.
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et par fichier du diff, en lignes (`--detail`) **et** occurrences.
- **Lignes qui gagnent une catégorie**, par fichier et catégorie, « aucune » compris : attendu **aucune**.
- **Lignes comptées modifiées, non retirées** : attendu **aucune**.
- **Δlignes > Δoccurrences** : attendu aucun fichier (Δocc ≥ Δlignes partout).
- **Lignes prises à une autre étape** : `score_panel_display.c:181` (B11 de l'étude), déclarée au message et dans la note.
- **Numéros de ligne** : ceux du parent.

### Empreinte et découpage (G)

| Commit | `src/` | `tests/` | docs | Chemins |
|---|---|---|---|---|
| **D.18a** | `layout.c`, `layout.h`, `menu.c`, `help.c`, `msgbox.c`, `planet_display.c`, `research_display.c`, `score_panel_display.c` (8) | `lq`, `js`, `layout_nil_item_draws_nothing` (3) | `plan-3.md`, `glossaire-noms.md` | 13 |

(L'étude disait ≈ 18 fichiers et nommait les TU `lq`, `js`, `ga`, `hp` : `ga` et `hp` sortent — ils ne nomment ni les clés ni leurs fonctions —, `layout_nil_item_draws_nothing` entre ; `planet_gh_row_lq` est relancé et reprouvé sans changer.)

- **Rythme mesuré** (horodatage des commits) : D.15a–D.16b, ≈ 7 à 11 min de la mise en file au commit `refactor` pour 40 à 55 lignes comptées ; D.12c2 (≈ 63 lignes `liste`, 20 chemins) a tenu sous ≈ 55 min. **D.18a** : M0′ ≈ 2 min ; éditions ≈ 12 (13 chemins, 133 lignes presque toutes mécaniques) ; `gcov` départ/arrivée ≈ 4 ; mutations (≈ 22) ≈ 8 ; filets (dont `sanitize-net`, `class-coverage-net`, balayage ASan+UBSan) ≈ 5 ; mesures, classification, message, note ≈ 4 : **budget ≈ 35 min jusqu'au commit, plafond 50 min.**
- **Repli écrit** (G) : au plafond sans commit, rien n'est commis et `plan-pilot` relance en **D.18a1** (constructeurs, `pico8_lq_cell_item`/`pico8_lq_scalar_item`, `lq:mn` en cases, `pico8_lq_add(cell)` et `hp:lq` ; les clés restent des valeurs, `pico8_lq_cell_of` garde son tri ; TU aucun ; ≈ 25 lignes) puis **D.18a2** (le type des clés et tous les producteurs, `pico8_lq_keys_of_row`, `pico8_pj_ph_item_value`, TU `lq`, `js`, §6, piège « rien », M0′ ; ≈ 108 lignes). Sans pont compté. Comptes recomptés par commit. Rejeté comme plan principal : deux coûts fixes pour un changement de type qui n'a qu'un seul point de bascule.

### Effet sur `make data-residue`

| Catégorie | À `b4ec4b3` | Départ (H-D17-1) | Après D.18a |
|---|---|---|---|
| `valeur` | 745 / 89 ; 831 occ. | 693 / 85 ; 763 occ. | **560 / 83 ; 623 occ.** (− 133, − 140) |
| `valeur` `src/` | 318 / 51 ; 364 occ. | 270 / 47 ; 301 occ. | 181 / 45 ; 212 occ. |
| `valeur` `tests/` | 427 / 38 ; 467 occ. | 423 / 38 ; 462 occ. | 379 / 38 ; 411 occ. |
| `type-tag` | 242 / 58 | 242 / 58 | inchangé |
| résidu D | 985 / 104 | 933 / 102 | **800 / 101** |
| `DATA_RESIDUE_GUARD` | 8 catégories | inchangé | inchangé |

Par fichier (lignes / occ. ; départ attendu → après) : `layout.c` 28/28 → 0/0 ; `layout.h` 4/4 → 1/1 ; `menu.c` 20/20 → 8/8 ; `help.c` 2/2 → 1/1 ; `msgbox.c` 3/3 → 1/1 ; `planet_display.c` 30/30 → 8/8 ; `research_display.c` 10/10 → 0/0 ; `score_panel_display.c` 24/24 → 13/13 ; `test_pico8_lq` 66/71 → 30/30 ; `test_pico8_js` 44/51 → 39/44 ; `test_pico8_layout_nil_item_draws_nothing` 4/4 → 1/1.

Fonctions publiques qui nomment `pico8_value_t` (Annexe B) : 4 → **2** (`pico8_pj_item_of`, `pico8_js_pq`) ; membre public `pico8_lq_keys_t.items` sorti. Copies `truthy` : 2 → **1** (`ui_button`).

« − 133 / − 140 » (ou départ réel moins ces deltas) et « inchangé » sont des critères (écart = explication chiffrée au message).

### Documentation (G)

- `docs/plan-3.md` : une note « Découvert à D.18a » en fin de ligne D.18a, rien d'autre. Elle dit : le commit ; les comptes réels et l'écart éventuel à ≈ 560 ; les lectures (clés en cases, cinq constructeurs, `pico8_lq_cell_item` unique, piège « rien » tenu par le masque `nil_at` de `test_pico8_lq` et M0′, `planet_gh_row_lq` sans réécriture, reprouvé ; `class-coverage-net` lancé) ; la ligne prise à B11 ; `PICO8_LQ_CELL_TRUE` plus produit ; **pour D.18b** : les sources de bouton restent (`menu.c` 7, `help.c` 1, `msgbox.c` 1, `planet_display.c` 8, `score_panel_display.c` 2, `ui_button.c/.h`), la dernière copie `truthy` est celle de `ui_button` ; **pour D.18c** : `score_panel_display.c` garde `s[i]` (`:144-159`, `:179`, `:195`, `:197`) ; **pour D.19b** : `test_pico8_lq` plie encore par `fold_value` (`fold_cell`, `fold_tuple`, `cell_value_kind`), `test_pico8_js` par son `fold_value` ; **pour D.19c** : `layout.h:9` et `test_pico8_layout_nil_item_draws_nothing.c:34` n'ont plus que leur include.
- `docs/glossaire-noms.md:433` : « ses clés : `pico8_lq_keys_t` (positionnels `pico8_value_t` dans l'ordre, …) ; ses positionnels, `items` (`items_count`), classés une fois à la construction en `pico8_lq_cell_t` … » devient « ses clés : `pico8_lq_keys_t` (positionnels en cases `pico8_lq_cell_t`, dans l'ordre, écrites par `pico8_lq_cell_nothing/fixed/string/row/item`, `c`, `bg`, `score`, `ra`) ; ses positionnels, `items` (`items_count`), copiés des clés : rien (nil et faux, confondus), fixe, chaîne, rangée (tuple), autre-vrai (`pico8_lq_cell_kind_t`, `PICO8_LQ_CELL_*`) » ; `:434` : « (une sous-rangée, un tuple, en case rangée) ».
- `docs/trous-de-portage.md` : **inchangé** (aucun écart port/cart n'est touché ; TROU-7 et TROU-25 gardent leurs marqueurs).
- Commentaires : quatre sortes de `CLAUDE.md` ; contrats réécrits (`layout.h` : union `:29-32`, clés `:95-97`, `keys_of_row` `:114-117`, `add` `:123-125`) ; contrats nouveaux (cinq constructeurs : qui emprunte, ce que rend NULL) ; piège déplacé (clause NULL de `pico8_lq_cell_row`, nommant `layout_nil_item_draws_nothing`) ; piège du TU (`fold_cell`, octets figés ; le masque `nil_at`) ; commentaire de `render_one_item` ; commentaires retirés avec leur objet (`layout.c:47-48`, `:64-66`, `score_panel_display.c:126`) ; aucun renvoi à D.18a ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile` ; aucun e2e ni TU ajouté ou supprimé ; toute constante `*_HASH`/`identity[]` ; `tools/class_coverage_net_expected.txt`.
- Le comportement : mêmes cases stockées, mêmes `fs`, mêmes largeurs, mêmes pixels ; mêmes durées de vie (TROU-7) ; `make e2e` conforme au bit près.
- `pico8_lq_cell_t`, ses cinq genres et le contrat « l'union ne distingue pas nil et faux ».

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : sous `-Wall -Wextra -Werror`, les fonctions `static` retirées seraient fatales si gardées (retirées dans le même commit) ; aucune aide de TU ne devient sans appel ; les initialiseurs d'un tableau automatique par appels de fonction (`{str("a"), …}`) restent permis (déjà employés) ; `const pico8_jb_row_t row = {.count = 1, .items = &item};` compile (adresse d'un paramètre, durée automatique).
- **H2** : aucun autre producteur ni lecteur de `pico8_lq_keys_t.items`, `pico8_lq_keys_of_row`, `pico8_lq_add` n'existe hors de l'Inventaire (grep à `b4ec4b3`, `src/`, `tests/`, `tools/`) ; un oublié se révèle à la compilation et suit (listé).
- **H3** : aucun producteur ne passe une chaîne NULL (preuve par site ci-dessus) ; si l'`assert` de `pico8_lq_cell_string` tombe dans un TU ou un e2e, H3 est fausse : arrêt, rapporté (un genre sans règle).
- **H4** : M0′ rend exactement les quatre positions « rien » attendues ; sinon les masques suivent M0′ et l'écart est rapporté.
- **H5** : `lq:mn` dessine au pixel près avec ses tuples lus en cases (`JO_EX_HASH`, `PANEL_HASH`, `HP_LQ_HASH`, `CALLS_HASH`, e2e le jugent).

## Implementation Chunks

**D.18a** (un commit)

1. Mesure de départ (H-D17-1, sinon départ réel) ; remesure par contenu ; `gcov` de départ ; **M0′** (instrumentation de `test_pico8_lq`, un lancement, `cp`).
2. `layout.h/.c` : clés, constructeurs, `pico8_lq_cell_item`, `pico8_lq_scalar_item`, `lq:mn`, `jb_line`, `lq_struct_new`, `keys_of_row`, `add` ; retraits.
3. Producteurs : `menu.c`, `help.c`, `msgbox.c`, `planet_display.c`, `research_display.c`, `score_panel_display.c`.
4. TU `lq` (cases, `same_cell`, `fold_cell`, masques de M0′), `js` (`build`, `fold_keys`), `layout_nil_item_draws_nothing` ; `make tmp/test_pico8_<nom>` et lancement de chacun, dont les §6 (`planet_gh_row_lq` avec `ASAN_OPTIONS=detect_leaks=0`).
5. Glossaire.
6. `make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`, balayage ASan+UBSan ; `gcov` d'arrivée ; mutations M1–M22 ; mesures, classification ; note « Découvert à D.18a » ; **commit**, vérification, push.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (cases stockées, pixels, largeurs, rangées de boutons), pas la présence d'une ligne. D.18a change le type des clés sans changer une case stockée ni un pixel. La preuve : les TU existants, qui plient cases et pixels dans des constantes inchangées ; le masque vérifié de `test_pico8_lq` ; les deux tests §6 et leurs preuves rouges ; les 26 e2e au bit près ; `sanitize-net` et le balayage ASan+UBSan ; `class-coverage-net` identique ; `gcov` et mutations.

### TU touchés (G)

1. Suivent : `lq`, `js`, `layout_nil_item_draws_nothing`.
2. Prouvent sans suivre : `ga`, `hp`, `pj`, `jb_ol`, `fm`, `mh`, `jo`, et tous ceux qui construisent des boutons ou des rangées (`jo:gh`, `jo:fe`, `hp:lq`, `pz`, `pj:ph`, `jb:pq`) ; §6 `planet_gh_row_lq`, `ga_export_panel_by`.
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change ; une constante qui rougit est un bug de conversion (ou le piège « rien » oublié).
4. **Assertion nouvelle** : seulement pour couvrir une ligne modifiée non exécutée ou faire rougir une mutation qui ne rougit rien, dans un test du module, listée au message ; rien de lent, aucun fichier d'événements. Les trois `assert` du masque et `same_cell` font partie de la conversion, pas des ajouts.
5. **Prior art** : piège « rien » et M0 de D.12c ; plis aux mêmes octets sans nommer le module (`fold_qe` de D.17, `fnv(fnv(h, 0xB0u), 1u)` de D.15b) ; §6 réécrits dans leur lecture avec preuve rouge (D.12c2 : M17, M19, M20) ; fonctions `static` retirées avec leur dernier appel (D.15b–D.17).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`, `Makefile:118`), **les TU qui forkent** (`in_fresh_child`/`must_abort`, dont `lq`, `js`, sortie par `_exit`) compilés en plus avec **`-D_exit=exit`** ; lancés comme le `Makefile` (depuis la racine, `ASAN_OPTIONS=detect_leaks=0` pour `test_pico8_planet_gh_row_lq`) ; union des lignes de `src/` exécutées, départ et arrivée.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée, sinon justifiée au message. Attendus : les cinq constructeurs (la branche NULL de `pico8_lq_cell_row` par `layout_nil_item_draws_nothing` ; l'`assert` de `pico8_lq_cell_string` n'est pas une branche) ; chaque branche de `pico8_lq_cell_item` (`row` : `lq:903-904`, rangées générées de `menu`/`help` ; `s` ; `is_false` : écart `{false,1}` de `jo:gh`, `{false,…}` de `jo.ex`, faux de `bz` ; `is_ref` : `layout_nil` ; nombre : partout) ; `lq:mn` `:93-114` (`lq`) ; `jb_line` (`js:679`, `jb_ol`) ; producteurs (`menu` par `js`/`fm`, `help` par `fv`/`js`, `msgbox` `pz` d'une planète à `ep` par `lq`/`ga` sinon e2e `colony_panel`, `jo:gh`/`jo:fe` par `lq`/`ga`/§6, `jo.ex`/`hp:lq` par `lq`/`hp`, `pj:ph` par `pj`).
- **`L`** = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide**. Partantes, hors `L` : `pico8_layout_truthy`, `pico8_lq_scalar_cell`, `pico8_lq_cell_of`, `pico8_pj_ph_item_value`. **Surveillé** : `pico8_value_string`, `_fixed`, `_ref`, `_bool` (`pico8_value.c`) perdent des appelants ; elles restent appelées par `test_pico8_value` et par les sites B9–B11 ; si une ligne de `pico8_value.c` tombe dans `L`, elle est justifiée par ce seul appelant (D.19c). Toute autre ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail : fichier sauvegardé par `cp` avant, restauré par `cp` simple puis `touch` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance ; chacune doit rougir au moins le TU nommé, sinon trou couvert par une assertion ou justifié (mutation équivalente démontrée). **Astuces** : neutraliser par `0 ? x : …`, `0 &&`, une condition toujours fausse, ou `pico8_lq_cell_nothing()` à la place d'un constructeur, plutôt que supprimer (`-Werror`) ; si un `assert` de `src/` attrape la mutation avant le TU, relancer avec cet `assert` relâché dans la copie mutée, et rapporter les deux.

- **M0′ (exigée, avant conversion)** : voir « Le piège “rien” » ; rapportée au tableau « Runs » et au message.
- **M1 (exigée) — piège « rien »** : dans `test_pico8_lq`, masque de « mixed c » `1u << 3` au lieu de `1u << 5` (nil et faux intervertis) → **`test_pico8_lq` rouge** (`CONSTRUCTIONS_HASH`) ; **M1b** masque de « nil » à `0u` → rouge. **Si M1 ou M1b ne rougit pas : `FAILED:`.**
- **M2 (exigée) — G6, preuve rouge de `layout_nil_item_draws_nothing`** : branche NULL de `pico8_lq_cell_row` rendant `TRUE` (`(pico8_lq_cell_t){.kind = PICO8_LQ_CELL_TRUE}`) → rouge sur l'assertion de case ; relancé avec cette seule assertion neutralisée dans une copie du TU → rouge sur les pixels (sprite 0 trouvé, `hits == 1`). **M2b** : `is_false` rangé `TRUE` dans `pico8_lq_cell_item` → `lq` rouge (`PANEL_HASH`, `JO_EX_HASH` : l'écart `{false,1}` dessinerait un sprite). **Si M2 ne rougit pas : `FAILED:`.**
- **M3 (exigée) — preuve rouge de `planet_gh_row_lq`** : `pico8_lq_struct_new` alloue et copie `keys->count + 1` cases → rouge (ASan, lecture hors de `row_items` ; « pas de cinquième »). **M4 (exigée)** : `jo_gh_row_items` alloue `n - 1` cases → rouge (ASan, écriture de l'écart). **Si M3 ou M4 ne rougit pas : `FAILED:`.**
- **M5** `lq:mn` : `w` ignoré (`have_f2 && 0 &&`) → `lq` (`CONSTRUCTIONS_HASH` « spr w qw qv », `JO_EX_HASH`) ;
- **M6** `lq:mn` : `qw` ignoré → `lq` (assertion `sprite_qw`, `:410`) ;
- **M7** `lq:mn` : `f4.kind == PICO8_LQ_CELL_NOTHING` à la place de `!=` → `lq` (`JO_EX_HASH`, `PANEL_HASH`) ;
- **M8** `lq:mn` : `f1` pris `pico8_lq_cell_nothing()` → `lq`, `hp` ;
- **M9** `pico8_lq_cell_item` : `is_fixed` ignoré (toujours `n`) → `lq` (`:903`, `JO_EX_HASH`) ;
- **M10** `pico8_lq_cell_item` : branche `row` neutralisée → `lq` (`:904`), `js`/`fm` (rangées de `ix`/`ha`) ;
- **M11** `pico8_lq_cell_string` rend `NOTHING` → `lq`, `js`, `pj`, `ga` ;
- **M12** `pico8_lq_cell_fixed` rend `x + 1` → `lq`, `js` ;
- **M13** `pico8_lq_struct_new` copie toujours `keys->items[0]` → `lq` (`same_cell`, `CONSTRUCTIONS_HASH`) ;
- **M14** `pico8_lq_add` range `pico8_lq_cell_nothing()` → `lq` (`HP_LQ_HASH`), `hp` ;
- **M15** `pico8_lq_keys_of_row` n'écrit que `cells[0]` (les autres `nothing`) → `lq` (`:904`), `js`, `ga` ;
- **M16** `jb_line` : chaîne → `nothing` → `js` (`:679`), `jb_ol` ;
- **M17** `jo:gh` : rangée « needs: »/« wants more: » en `nothing` au lieu de `_row(…)` → `lq` (`test_gh_wants_more`, `PANEL_HASH`), `ga` ;
- **M18** `jo.ex` : `l[l_count++] = pico8_lq_cell_nothing()` pour `t1` → `lq` (`JO_EX_HASH`), `hp` ;
- **M19** `hp:lq` : `px` ajouté en `nothing` → `lq` (`HP_LQ_HASH`), `hp` ;
- **M20** `pj:ph` : `pico8_lq_cell_item` remplacé par `nothing` → `pj` ;
- **M21** `pz` : `ep` en `nothing` → TU qui plie un `pz` à `ep`, sinon e2e (`colony_panel`) ;
- **M22** `cr` : `lq_items[0]` (`nk`) en `nothing` → `js`/`fm` (`RESEARCH_HASH`…), sinon e2e.

Cibles (numéros à `b4ec4b3` ; `planet_display.c`/`research_display.c` après D.17 : colonnes de l'Inventaire) : `layout.c:29-114`, `:220-246`, `:342-411` ; `menu.c:94-122`, `:185-200`, `:272-292` ; `msgbox.c:333-358` ; `planet_display.c:104-111`, `:139-298`, `:345-386` ; `research_display.c:143-280`, `:309-311` ; `score_panel_display.c:176-296`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes). Aucun e2e ajouté.
- **`make sanitize-net`** (exigé par la ligne) : propre.
- **`make class-coverage-net`** (exigé ici, (G)) : sortie **identique** à `tools/class_coverage_net_expected.txt`.
- **Balayage ASan+UBSan des TU** : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 35 min de temps réel jusqu'au commit**, M0′, mesures, `gcov`, mutations et filets compris ; **plafond 50 min** (au-delà : repli D.18a1/D.18a2). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| M0′ (une instrumentation, un lancement) | < 2 min |
| `make test` | de l'ordre de la minute (`layout.h` change : recompilation large) |
| `make e2e` | ~1 s |
| `make sanitize-net` | quelques minutes (rejeux sous ASan+UBSan, `-O2`) |
| `make class-coverage-net` | < 1 min |
| `make line-coverage EVENTS="tests/e2e/*.json"` | < 1 min |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes |
| mutations (≈ 22, plus M1b, M2b) | < 9 min |
| balayage ASan+UBSan des TU | quelques minutes |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; rien d'inachevé n'est commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »**, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + état) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | D.17 gradée (HEAD à noter), avant D.18a | 0 | `valeur 693 85` (H-D17-1) | … |
| 2 | recette `valeur` | idem | 0 | `valeur 693 lignes 85 fichiers 763 occ.` | … |
| 3 | `grep -cE "$P" src/pico8_research_display.c src/pico8_planet_display.c tests/test_pico8_lq.c` | idem | 0 | `10`, `30`, `66` (H-D17-3 à 5) | … |
| 4 | M0′ : `make tmp/test_pico8_lq && ./tmp/test_pico8_lq` (copie instrumentée) | idem + instrumentation | 0 | `false: 0 faux / nil: 0 nil / mixed c: 3 faux, 5 nil` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les 13 chemins de « Empreinte » (plus un TU suiveur listé au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`, ni `planet_gh_row_lq`, ni `ga_export_panel_by`.
2. **Greps** de la Sortie, exactement.
3. **Formes** : `pico8_lq_keys_t.items` en `const pico8_lq_cell_t *` ; cinq constructeurs, contrats écrits ; `pico8_lq_cell_row(NULL)` rend `NOTHING`, `pico8_lq_cell_string(NULL)` s'arrête ; `pico8_lq_cell_item` unique ; garde « scalaires seulement » présente ; `lq:mn` sans valeur ; `pico8_lq_cell_t` et ses genres inchangés.
4. **Casts** : aucun nouveau (`grep -nE '\(\s*(const\s+)?[a-z0-9_]+_t\s*\**\s*\)\s*[^{ ]'` et `grep -nE '\(\s*(const\s+)?void\s*\*\s*\)'` sur les lignes `+` du diff : vides ; les littéraux composés en sont exclus par le `{`).
5. **API** : seuls `pico8_lq_keys_t.items`, `pico8_lq_keys_of_row`, `pico8_lq_add` changent de type ; seules les quatre `static` disparaissent (et `pico8_jb_item_value` devient `pico8_lq_cell_item`) ; seuls les cinq constructeurs apparaissent.
6. **Liens** : grep 6.
7. **Constantes** : grep 7 vide ; M0′, M1/M1b, M2/M2b, M3, M4 rapportées.
8. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste ; aucune ligne gagnée ; la ligne prise à B11 déclarée ; includes (grep 8) ; jonction `layout.c:375-376` déclarée.
9. **Écarts déclarés** : `planet_gh_row_lq` sans réécriture ; `ga`, `hp` hors empreinte ; constructeurs `nothing`/`item` ; pas de pont ; `is_ref` non NULL dans un tuple rendu `ROW` ; `is_ref` de `bz` ; `PICO8_LQ_CELL_TRUE` plus produit ; `class-coverage-net` lancé ; piège de `js:781`.
10. **Preuve** : `gcov` (lignes modifiées exécutées ou justifiées ; `L` vide ou justifié par `test_pico8_value` seul), mutations restaurées par `cp`, `sanitize-net`, `class-coverage-net` et ASan+UBSan rapportés.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; piège de la clause nil présent à la branche NULL de `pico8_lq_cell_row` ; aucun renvoi à D.18a ni au plan dans `src/` ou `tests/`.
12. **Docs** : note « Découvert à D.18a » en fin de ligne D.18a, rien d'autre dans le plan ; glossaire `:433-434` réécrit.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le HEAD où D.17 est gradée (ou un descendant qui ne touche que `docs/`), propre ; ou si `valeur` n'y vaut pas 693 / 85 / 763 occ., `type-tag` 242 / 58, résidu D 933 / 102, sans que ce PRD ait été revérifié (H-D17-1).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change **dans son intention** (valeurs attendues, cas, garde, assertion retirée ou affaiblie) ; si `planet_gh_row_lq` ou `ga_export_panel_by` change ; si l'une des preuves rouges M2, M3, M4 manque.
- `FAILED:` si M0′ n'est pas fait avant la conversion, ou si un site ne tombe sous aucune règle de ce PRD ni de l'étude (arrêt « champ sans règle ») — en particulier une chaîne NULL passée par un producteur (H3) —, ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si `make sanitize-net` ou le balayage ASan+UBSan des TU n'est pas propre, ou si `make class-coverage-net` diffère de l'attendu.

**Périmètre et forme**

- `FAILED:` si `tools/`, le `Makefile` ou un include change ; si le commit touche un chemin hors de son « Empreinte » sans le lister comme TU suiveur.
- `FAILED:` si une grep de la Sortie ne rend pas ce qui y est écrit.
- `FAILED:` si une signature ou une fonction hors de la liste du contrôle 5 change ou disparaît ; si `pico8_lq_cell_t`, ses genres, `pico8_lq_t`, `pico8_lq_copy`, les sources de bouton (B9), `pico8_pj_item_of`/`ou[]` (D.18c), `pi.c` ou `s[i]` changent ; si une durée de vie change.
- `FAILED:` si l'union se met à distinguer nil de faux, ou si un canal caché (charge d'une case `NOTHING`) porte l'information du TU.
- `FAILED:` si un cast nouveau est ajouté, si des lignes comptées sont jointes (hors `layout.c:375-376`, déclarée), ou si une cale valeur ↔ typé ou un pont apparaît (dans `src/` ou un TU).

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne baisse pas de **133 lignes / 140 occ.** exactement depuis le départ réel, sans explication chiffrée ligne à ligne ; si `type-tag` change de compte.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, cale ou pont, (c) non nulle, (b) hors de `pico8_lq_keys_t.items`/`pico8_lq_keys_of_row`/`pico8_lq_add`.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; les lignes qui gagnent une catégorie (« aucune » compris) ; la ligne prise à B11 ; la justification de chaque Δlignes > Δoccurrences (attendu : aucune) ; la jonction `layout.c:375-376` ; les fonctions retirées et leurs lecteurs (raison `-Werror`) ; l'inventaire des producteurs (P1–P15) ; M0′ et les masques ; le piège de `js:781` ; les écarts déclarés (contrôle 9) ; le rapport `gcov` ; les mutations (M1, M2, M3, M4 en tête) ; `sanitize-net`, `class-coverage-net`, ASan+UBSan ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne modifiée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` contient une ligne hors de `pico8_value.c` justifiée par `test_pico8_value`.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification ; **si M1, M1b, M2, M3 ou M4 ne rougit pas**.
- `FAILED:` si le plafond est dépassé sans note (et sans repli), ou s'il manque un tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.18a », ou si elle omet : les comptes réels, les lectures, la ligne prise à B11, les legs pour D.18b, D.18c, D.19b, D.19c.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`, ou si le piège de la clause nil disparaît de `src/`.

## Out of Scope

- B9 (sources de bouton `lq`/`h_`/`jb` en `pico8_value_t`, `pico8_js_pq`, `pico8_js_is_lq`, TROU-23, dernière copie `truthy`) : D.18b.
- B10 (`pico8_pi_arg`), B11 (`s[i]`, `pico8_pj_ph_s_at`, `pico8_value_tostr` de `score_panel_display.c`), `ou[]` et `pico8_pj_item_of` : D.18c.
- Le retrait de `PICO8_LQ_CELL_TRUE` (plus produit) et des plis qui le lisent (`lq`, `ga`, `hp`, `js`, `pj`) ; les plis `fold_value`, `fold_tuple`, `tuple_item_value`, `cell_value_kind`, `vec_ref`, `with_ra` : D.19b (ou une étape que `plan-review` proposerait).
- Les includes de `layout.h`, `help.h`, `msgbox.h`, `planet_display.h`, `research_display.h`, `menu.c` et des TU ; le module lui-même : D.19c.
- Les `assert` d'étiquette (`layout.c:198`, TU) : D.20.
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Découpage** : un commit ; repli D.18a1 (constructeurs, tri d'item, `lq:mn`, `add`) / D.18a2 (type des clés, producteurs, TU, §6), sans pont.
  - **Types** : clés en `const pico8_lq_cell_t *` ; cinq constructeurs publics (`nothing`, `fixed`, `string` — NULL s'arrête —, `row` — NULL est nil —, `item`) ; pas de constructeur `TRUE`.
  - **Piège « rien »** : l'union ne change pas ; `test_pico8_lq` tient nil / faux par un masque `nil_at` gardé par trois `assert`, mesuré par M0′, prouvé par M1/M1b ; octets figés écrits sans nommer le module.
  - **§6** : `layout_nil_item_draws_nothing` passe par `pico8_lq_keys_of_row` sur un item nil généré (G6) ; `planet_gh_row_lq` n'a rien à réécrire, reprouvé par M3, M4.
  - **Portée** : `ga`, `hp` hors empreinte ; `:181` de `score_panel_display.c` prise à B11 ; glossaire réécrit, `trous-de-portage.md` intact.
  - **Drapeaux** : `sanitize-net` (ligne) ; `class-coverage-net` (constructeur `lq` touché) ; balayage ASan+UBSan.
- **Surprises de mesure, pour `plan-pilot`** : (1) **quinze producteurs dans `src/`**, et non « dix » (le PRD de D.12c ne comptait pas `jb:pq`, `keys_of_row`, `hp:lq`, `ix2_hb`, `pj:ph` boutons) ; (2) **le piège « rien » ne touche qu'une constante** (`CONSTRUCTIONS_HASH`, quatre positions dans trois rangées), parce que tous les autres plis lisent les cases stockées, que D.18a ne change pas ; (3) **`planet_gh_row_lq` n'a rien à réécrire** : il lit des cases depuis D.12c2 ; ce qu'il garde (la copie, l'allocation de `jo_gh_row_items`) change de type sous lui, d'où deux preuves rouges refaites plutôt qu'une réécriture ; (4) **aucun producteur n'écrit `TRUE`** : le genre n'était produit que par des états inatteignables (`BOOL` vrai, chaîne NULL, `is_ref` non NULL dans un tuple) ; (5) **`test_pico8_js.c:781` calcule une chaîne NULL jetée** : un constructeur strict l'arrêterait ; (6) `pico8_pj_ph_item_value` ignorait `is_ref` (inatteignable) ; (7) `ga` et `hp`, nommés par l'étude, n'ont rien à suivre ; (8) `score_panel_display.c:181` était rangée B11 ; (9) arrivée **≈ 560** comme l'estimation : − 133 depuis 693.
- **Pour D.18b** : `menu.c` 8 (7 B9 + include), `help.c` 1, `msgbox.c` 1, `planet_display.c` 8, `score_panel_display.c` 2 lignes B9, plus `ui_button.c/.h` ; dernière copie `truthy` : `ui_button.c:27`.
- **Pour D.18c** : `score_panel_display.c` garde 13 lignes (B11 11, B9 2) ; `pico8_value_tostr` y garde son appel.
- **Pour D.19b** : `test_pico8_lq` garde 30 lignes (`fold_value`, `fold_tuple`, `tuple_item_value`, `cell_value_kind`, `fold_fixed_member`, `fold_bg`, `vec_ref`, `with_ra`, `:198/200/204`, B9 `:524`, `:859`, include) ; `test_pico8_js` 39.
- **Pour D.19c** : `layout.h:9`, `test_pico8_layout_nil_item_draws_nothing.c:34` n'ont plus que leur include.

## Annexe A — textes recopiés (`docs/plan-3.md` à `b4ec4b3`, par extraction de lignes)

### Lignes D.17 à D.18c (l. 288-291)

> - D.17 — genres mêlés B2 : `pico8_qe_t` et `pico8_qe_equal` (nil ≠ faux conservé). `class-coverage-net` : oui. Compte : ≈ 720.
> - D.18a — B8 : `pico8_lq_keys_t.items` en cases de l'union de `layout` ; §6 `layout_nil_item_draws_nothing` et `planet_gh_row_lq` réécrits dans leur lecture, preuve rouge. `sanitize-net` : oui. Compte : ≈ 560.
> - D.18b — B9 : sources des boutons en champs séparés. `class-coverage-net` : oui. Compte : ≈ 505.
> - D.18c — B10, B11, `ou[]`. `sanitize-net` : oui. Compte : ≈ 435.

(À réextraire à la graduation de D.17 : la ligne D.17 portera sa note « Découvert à D.17 » — H-D17-10.)

### `lq:mn` et `jb:pq` du cart (lua.lua:300-315)

Recopiés dans « Preuve B8, par le cart ».

### Glossaire à `b4ec4b3` (`docs/glossaire-noms.md:433-434`)

> | `lq` | `pico8_lq_t` | une rangée d'icônes mise en page ; ses clés : `pico8_lq_keys_t` (positionnels `pico8_value_t` dans l'ordre, `c`, `bg`, `score`, `ra`) ; ses positionnels, `items` (`items_count`), classés une fois à la construction en `pico8_lq_cell_t` : rien (nil et faux, confondus), fixe, chaîne, rangée (tuple), autre-vrai (`pico8_lq_cell_kind_t`, `PICO8_LQ_CELL_*`) |
> | — | `pico8_lq_keys_of_row` | les clés d'une rangée générée : ses items (une sous-rangée, un tuple, en REF vers elle) et son `c` |

### §7, extrait (« En D et en 2 »)

> - **En D et en 2** : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée […], et à la sortie de phase.

## Annexe B — reproduire

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
sed -n 300,315p pico8/lua.lua                                           # lq:mn, jb:pq
grep -rn 'pico8_lq_keys_t\|pico8_lq_keys_of_row\|pico8_lq_add\|pico8_lq_new(' src tests   # producteurs
grep -rn 'PICO8_LQ_CELL_\|row_item' tests                                 # lecteurs de cases stockées
grep -n 'fold_built_from\|fold_row_from\|fold_positionals\|fold_cell' tests/test_pico8_lq.c   # piège « rien »
grep -rn 'is_ref' src | grep -v pico8_layout.c                            # items nil ou référence
grep -nE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c             # 2 après D.17 (H-D17-6)
grep -nE 'pico8_value_t' src/*.h | grep -vE ':\s*(\*|/\*)' | grep -v 'src/pico8_value.h' \
  | grep -vE '^\S+:[0-9]+:\s+(const )?pico8_value_t \*?[a-z_]+;|^\S+:[0-9]+:\s+const pico8_lq_keys_t'  # 4 après D.17
sh tools/data_residue.sh --detail | grep -E 'layout|menu|help|msgbox|planet_display|research_display|score_panel_display|test_pico8_(lq|js|layout)'
```
