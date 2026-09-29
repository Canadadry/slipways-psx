---
title: "Plan 3, étape D.17 : genres mêlés B2 — `pico8_qe_t` (nil, faux, nombre) et `pico8_qe_equal` (le `==` de Lua, nil ≠ faux) ; `pico8_trade_qe` et `pico8_jy_qe` rendent un `pico8_qe_t` ; `jo:cn` et `dh` comparent sans valeur ; `pico8_jo_ex` prend `const pico8_fixed_t *o` ; `pico8_value_equal` n'a plus d'appelant dans `src/` ; aucune constante ne change"
description: "Le `qe` d'une route vaut nil, faux (TROU-24) ou un nombre, celui d'un besoin nil ou un nombre, et deux comparaisons du port (`jo:cn`, `dh`) séparent nil de faux comme le `==` de Lua ; D.17 remplace la valeur Lua par une petite struct étiquetée `pico8_qe_t` et `pico8_qe_equal`, type l'argument `o` de `jo.ex`, retire trois fonctions devenues sans appel, fait suivre trois TU aux mêmes octets et fixe par des tests l'écart TROU-24 qu'aucun TU ne voyait : `valeur` baisse de 52 lignes (68 occ.), ≈ 745 → ≈ 693."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.16b était implémentée** (`plan-pilot`, étape 2b) sur l'arbre commis `8533807`, puis **revérifié après la graduation de D.16b** (`2aa02d5` `refactor(D.16b1)`, `0b2fb52` `refactor(D.16b2)`, gradée par `a01b84c`) sur **`529c71a`** (HEAD de `claude/plan-pilot-3-7zdft0`, qui ne fait qu'ajouter ce PRD), extrait par `git archive HEAD` dans le scratchpad. Seuls `grep`, `sed`, `awk` et `sh tools/data_residue.sh` ont tourné ; **rien n'a été compilé ni lancé**. Les hypothèses `H-D16b-n` ont reçu leur verdict (plus bas). **Les numéros de ligne de ce PRD sont ceux de `529c71a`** : identiques à ceux de `8533807` dans les fichiers que D.16b n'a pas touchés, remesurés par contenu dans ceux qu'elle a touchés (`research.c`, `research.h`, `research_display.c`, `test_pico8_hp`). L'implémenteur remesure encore par contenu au parent de son commit et explique tout écart.

Mesuré à `8533807` : `valeur` 825 / 97 ; 916 occ., `type-tag` 242 / 58, résidu D 1065 / 107 (D.16b non commencée). **Mesuré à `529c71a` (départ de D.17) : `valeur` 745 / 89 ; 831 occ. (`src/` 318 / 51 / 364, `tests/` 427 / 38 / 467), `type-tag` 242 / 58, résidu D 985 / 104.**

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c/.h` disparaissent (D.19c). L'étude D.14 (`docs/prd/done/plan3-phaseD-etape14-etude-genres-meles.md`, B2) range à part le `qe` des routes et des besoins : c'est **le seul groupe où un lecteur sépare nil de `false`**. Le port le tient encore en `pico8_value_t` :

- `pico8_trade_qe` rend `REF NULL`, `BOOL 0` ou `FIXED` ; `pico8_jy_qe` rend `REF NULL` ou `FIXED` ;
- `jo:cn` (`planet_trade.c`, 12 lignes) les compare par `pico8_value_equal` ; `dh` (`economy_tiers.c`) par une copie privée, `economy_value_equal_content`, qui ne compare jamais de chaînes ;
- l'argument `o` de `pico8_jo_ex` est une `pico8_value_t` que seuls `jo:hb` (`kw.qe`) et `hp:lq` (`hp.o`) fabriquent, pour une copie `truthy` et une comparaison à `r.w` ;
- `planet_display.c` refabrique `kw.qe` en valeur (`jo_kw_qe`) pour trois lecteurs ; `hp:jb` passe un `qe` enveloppé à `pico8_value_tostr` (appelant que le PRD de D.15c a confié à D.17).

Ligne de l'étape, recopiée de `docs/plan-3.md` à `529c71a` (§5, l. 288, inchangée depuis `8533807`) :

> - D.17 — genres mêlés B2 : `pico8_qe_t` et `pico8_qe_equal` (nil ≠ faux conservé). `class-coverage-net` : oui. Compte : ≈ 720.

Ligne de l'étude D.14 (§3), recopiée :

> | **D.17** | B2 : `pico8_qe_t`, `pico8_qe_equal` ; `pico8_trade_qe`, `pico8_jy_qe` typés ; `planet_trade`, `economy_tiers`, `kw.qe` et `o` de `pico8_jo_ex` (`planet_display`, `research_display`) ; `pico8_value_equal` n'a plus d'appelant dans `src/` ; TROU-3 et TROU-24 relus ; TU `trade`, `hp`, `nr`, `lq` (`jo_ex`) | ≈ 14 fichiers | ≈ 720 |

Point B2 et ligne (a) rangée avec lui (étude §1.2, §1.3), recopiés :

> **B2. `qe` d'une route et d'un besoin — 23 lignes (+ 17 lignes (a) rangées avec).** `trade.c:13-17` (`pico8_trade_qe`), `trade.h:50`, `research.c:137-138` (`pico8_jy_qe`), `research.h:21`, tout `planet_trade.c` (12 lignes), `economy_tiers.c:21-25,241`.
> - Genres : `trade.qe` vaut nil, `false` ou un nombre. `false` vient de TROU-24 (`trade.c:117`, `kg` pose `false` quand `self.qe` est nil) ; nil vient de `jo:ib` avec un `kw` sans `qe` (lua.lua:502-504). `jy.qe` (`i.qe`) vaut nil ou un nombre.
> - Lecteurs qui distinguent nil de `false` : `t.qe==i.qe` (lua.lua:494, 1297) — en Lua, `false==nil` est faux, `nil==nil` est vrai. Le port le reproduit aujourd'hui par `pico8_value_equal` (identité de `REF NULL`, genre `BOOL` à part).
> - Règle (G) : **petite struct étiquetée** `pico8_qe_t { enum { PICO8_QE_NIL, PICO8_QE_FALSE, PICO8_QE_FIXED } kind; pico8_fixed_t n; }` dans `pico8_trade.h`, et `pico8_qe_equal(a, b)` : même genre, et même `n` si `FIXED`. `pico8_trade_qe` et `pico8_jy_qe` rendent un `pico8_qe_t`. Les membres de `pico8_trade_t` (`has_qe`, `qe_is_bool`, `qe`) restent tels quels. Raison : trois états qu'un lecteur distingue ; un drapeau seul perdrait TROU-24.
> - `economy_value_equal_content` (`economy_tiers.c:21-25`) ne compare jamais deux chaînes (ses arguments sont des `qe`) : il devient `pico8_qe_equal`.
>
> | `kw.qe` et l'argument `o` de `pico8_jo_ex` — `planet_display.c:38-42,59,220-225,261-264`, `research_display.c:47,57,142,164,194-205,303`, `.h:27` | nil ou nombre (`hp.o`, `kw={qe=hp.o}`, lua.lua:463) | drapeau + nombre ; rangés avec le point `qe` de (b) parce qu'ils passent par la même comparaison |

Lignes directrices de l'étude qui s'appliquent (§4), recopiées en extrait :

> **G1.** Aucune constante figée ne change dans D.15–D.20. Un pli typé écrit le même octet de genre (`0`/`1`/`2`/`3`, les valeurs de l'actuel `pico8_value_kind_t` […]) et le même contenu. Une empreinte qui bouge est une régression, pas une mise à jour.
> **G2.** […] Là où un lecteur les sépare, ils restent deux états : B2 (`qe`, TROU-24), B9 (`h_`, TROU-23).
> **G3.** TROU-13, TROU-23, TROU-24 et TROU-25 restent à l'identique ; leur texte est réécrit quand il nomme `pico8_value_*`.
> **G4.** Un cas de TU qui passe un genre qu'aucun appelant de `src/` n'écrit (`pico8_oj` d'un nombre, `pico8_jc_hg_has` d'un nombre ou de nil, `pico8_jo_ex` d'un `false`) est supprimé avec le type qui le permettait ; le message de commit le nomme.

G5 (`tostr` → `pico8_str.c`, fait à D.15c), G6 (D.18a) et G7 (`hp_pose`, D.16b) ne s'appliquent pas ici.

Legs des étapes précédentes, recopiés (`docs/plan-3.md` l. 285 et PRD de D.16b, « Further Notes ») :

> `pico8_value_tostr` est gardée et délègue ses nombres : trois appelants à genres mêlés vivent (`research_display.c:58` `qe`, D.17 ; `pi.c:105` et `score_panel_display.c:197`, D.18c).
> **Pour D.17** : la copie `truthy` de `research_display.c` n'a plus que `o` de `jo_ex` pour appelant ; `research.c` garde `pico8_jy_qe` (`:137-138`).

Règles communes de l'étude (§3) : un TU qui nomme un symbole changé le suit dans le même commit ; aucune empreinte figée ne change ; aucun `*.sha256` ni e2e ne change ; `class-coverage-net` si l'étape touche un constructeur.

### Hypothèses `H-D16b-n` — verdicts (D.16b gradée, mesurés à `529c71a`)

Source des hypothèses : ce PRD à `8533807`. Verdicts : `git archive HEAD` (`529c71a`), `git diff --stat 8533807 HEAD -- src tests Makefile tools docs/trous-de-portage.md docs/glossaire-noms.md` (37 chemins de code ; ni `Makefile`, ni `tools/`, ni les deux docs), la note « Découvert à D.16b1/b2 » (`docs/plan-3.md:287`).

| Hypothèse | Énoncé (résumé) | Verdict | Mesure à `529c71a` ; effet sur ce PRD |
|---|---|---|---|
| H-D16b-1 | `valeur` 745 / 89 ; 831 occ. (`src/` 318 / 51 / 364, `tests/` 427 / 38 / 467), `type-tag` 242 / 58, résidu D 985 / 104 | **tient** | exact ; départ, effet et critères inchangés |
| H-D16b-2 | D.16b ne touche ni `trade.c/.h`, `planet_trade.c`, `economy_tiers.c`, `economy_score.c`, `planet_display.c/.h`, `research_display.h`, ni les TU `trade`, `lq` | **tient** | absents du `diff --stat` ; numéros inchangés |
| H-D16b-3 | `research.c` 2 / 3, seules les lignes de `pico8_jy_qe`, remontées vers ≈ `:110–:125` | **fausse en partie** | 2 / 3 exact, mais `pico8_jy_qe` est en **`:134-135`** (− 3 seulement : `hp_pose` réécrite a presque rendu les lignes que la copie `truthy` a libérées) ; numéros corrigés dans l'Inventaire, sans autre effet |
| H-D16b-4 | `research.h` 2 / 2, include `:9`, `pico8_jy_qe` `:21` | **tient** | `:9`, `:21` ; contrat `:20` inchangé |
| H-D16b-5 | `research_display.c` 26 / 31, seul `:325` changé en place | **tient** | `:325` est `if (!pico8_oj(b->rr)) {` ; toutes les lignes de D.17 à leur numéro |
| H-D16b-6 | 3 copies `truthy`, celle de `research_display` appelée seulement en `:156` | **tient** | `layout.c:19`, `research_display.c:20`, `ui_button.c:27` |
| H-D16b-7 | `test_pico8_hp` 35 / 36 ; `:453-455` → `:449-451`, `:627` → `:622` ; `fold_value` toujours appelée | **tient** | `:449` (2 occ.), `:450`, `:451`, `:622` ; `fold_value` sur 20 lignes (définition comprise) |
| H-D16b-8 | 7 fonctions publiques à `pico8_value_t` | **tient** | `lq_keys_of_row`, `lq_add`, `jy_qe`, `jo_ex`, `pj_item_of` (`score_panel.h:122`), `js_pq`, `trade_qe` |
| H-D16b-9 | 7 lignes d'appel de `pico8_value_equal`, 3 de `pico8_value_tostr` dans `src/` | **tient** | mêmes sites (`pi.c:105`, `score_panel_display.c:197`, `research_display.c:58`) |
| H-D16b-10 | `Makefile`, `tools/`, `DATA_ENGINE_SRCS`, attendu de `class-coverage-net` inchangés ; 26 e2e | **tient** | aucun chemin de `Makefile`/`tools/` dans le diff |
| H-D16b-11 | `o` toujours un nombre (`hp_pose`, `case 5`) ; TROU-3, TROU-24, glossaire intacts | **tient** | `research.c:90-92` (`self->o`, `has_o = 1`) ; docs absents du diff |
| H-D16b-12 | ligne D.17 inchangée ; D.16b en deux commits, sans repli | **tient** | `2aa02d5`, `0b2fb52` ; ligne D.17 identique (Annexe A) |

### Revérification faite à la graduation de D.16b

- « État de départ », critères `FAILED:` de départ, Runs 1 et 3 : remis sur `529c71a` (mesures identiques à H-D16b-1).
- Numéros de `research.c` (`:134-135`), `research.h`, `research_display.c`, `test_pico8_hp` : remesurés par contenu ; les autres fichiers sont inchangés depuis `8533807`.
- Copies `truthy`, greps 1, 3, 4 : constatés.
- Découpage en un commit : **gardé** (aucune mesure ne le change).
- Annexe A : lignes D.16b–D.18a réextraites de `529c71a`, avec les notes « Découvert à D.16b1/b2 ».

### Preuve B2, par le cart

**Écritures du cart** (`grep -n 'qe' pico8/lua.lua`) :

| Ligne | Site | `qe` écrit | Genres |
|---|---|---|---|
| 463 | `jo:lz` | `self.kw={qe=hp.o,mu=0}` ; `hp.o` passe par `gp` (`ho`, l. 17-22 : `r[ri[i]]`) | nil (projet sans `o`) ou nombre |
| 1052 | `mx:kf` | `self.kw.qe=i.qe` sous `gu(self.ht,i.qe)` | nombre |
| 504 | `jo:ib` | `trade({…,qe=qe})`, `qe=self.kw.qe` | nil ou nombre |
| 607 | `trade:kg` | `qe=self.qe` : recopie | nil ou nombre |
| 1407 | `hp:pq` | `{qe=r[c]}` pour chaque caractère de `i` | nil (`c` hors de `r`) ou nombre |
| 488 | `jo:cn` | `{qe=r.e}` | nombre |
| 492 | `jo:cn` | `i.qe=t.qe` sous `gu(self.ba,t.qe)` | nombre |
| 1280 | `bb` | `{qe=pt.hn.dm}` sous `if pt.hn.dm` | nombre |

**Le cart n'écrit jamais `false` dans un `qe`.** `trade.qe` et `kw.qe` : nil ou nombre ; `i.qe` : nil ou nombre.

**Lecteurs du cart** :

| Ligne | Lecture | Deux côtés absents possibles ? |
|---|---|---|
| 436 | `self.kw.qe~=r.w` (`jo:gh`, « wants more ») | non : `r.w` est un nombre |
| 443 | `gu(jo.cq,self.kw.qe)` (`jo:gh`, « makes ») | non : appartenance d'un nombre |
| 480 | `t.qe==r.e` (`jo:cn`) | non |
| 483, 491, 494 | `i.qe~=r["?"]`, `i.qe==r["?"]` (`jo:cn`) | non |
| 491 | `gu(self.ba,t.qe)` | non |
| **494** | **`t.qe==i.qe`** (`jo:cn`) | **oui** : route et besoin tous deux sans `qe` → vrai dans le cart |
| **1297** | **`im.qe==i.qe`** (`dh`) | **oui** : idem → la route compte dans le cart |
| 525 | `{i.qe,3-lc,…}` (`jo.ex`) | — (item de rangée) |
| 530-532, 539 | `if (not o)return`, `o==r.w`, `{o}` ; `jo.ex(self.jy,self.kw.qe,…)` | `o` : nil ou nombre, vérité puis nombre |
| 1418 | `iz[tostr(i.qe)]` (`hp:jb`) | — (`tostr(nil)` vaut `"[nil]"`, clé absente de `iz`) |
| 302, 1051, 1240-1253, 1333 | `lq:mn`, `mx:kf`, `bx`/`ck`, `dh` (score) | déjà typés dans le port (hors de ce PRD) |

### Preuve B2, par le port

**Écritures** :

| Site (à `8533807`) | `qe` écrit | État `pico8_qe_t` lu ensuite |
|---|---|---|
| `trade.c:22-47` (constructeur) | recopie `keys` ; les deux `assert` `:36-39` refusent un booléen autre que 0/1 et un nil porteur d'une valeur | — |
| `planet_trade.c:196-200` (`jo:ib`) | `has_qe = 1, qe = kw->qe` si `kw->has_qe`, sinon rien | NIL ou FIXED |
| `trade.c:115-118` (`kg`, `/* TROU-24 */`) | `has_qe = 1` toujours ; `qe_is_bool = 1, qe = 0` si `self` n'a pas de `qe` | FIXED recopié ou **FALSE** |
| `research.c:138-146` (`pico8_jy_new`) ; appelants `research.c:233-237` (NULL si le caractère est hors de `r`), `planet_trade.c:121` (`r.e`) | `row.has_qe` sous `qe != NULL` | NIL ou FIXED |
| `planet_trade.c:161-162` (`i.qe=t.qe`) | nombre (l'`assert` `:160` le garde) | FIXED |
| TU : `test_pico8_trade:191-199` (`route()`), `:381` | un `qe` **vrai** (`1, 1, 1`) seulement pour `pico8_trade_eq`, qui ne lit pas `qe` ; aucun appel de `pico8_trade_qe` dans `tests/` | — |

**Aucun site de `src/` n'écrit un `qe` vrai**, et le seul `false` est celui de `kg` (TROU-24). `pico8_trade_qe` n'a donc que trois états à rendre ; un `qe` vrai (TU seul) s'arrête sur un `assert` (décision plus bas).

**Lecteurs**, avec ce qu'ils deviennent :

| Site | Lecture aujourd'hui | Côtés | nil ≠ faux compte ? |
|---|---|---|---|
| `planet_trade.c:103` | `have_r_e && pico8_value_equal(t_qe, r_e)` | route vs nombre | non |
| `planet_trade.c:111` | `have_r_q && pico8_value_equal(pico8_jy_qe(i), r_q)` | besoin vs nombre | non |
| `planet_trade.c:151` | `have_r_q && pico8_value_equal(qe_val, r_q)` | besoin vs nombre | non |
| `planet_trade.c:156`, `:160` | `t_qe.kind == PICO8_VALUE_FIXED` | genre seul | non |
| **`planet_trade.c:167`** | **`pico8_value_equal(t_qe, qe_val)`** | **route vs besoin** | **oui** |
| **`economy_tiers.c:256`** | **`im->has_qe && economy_value_equal_content(pico8_trade_qe(im), qe_val)`** | **route vs besoin** | **oui** |
| `research_display.c:156`, `:165` | `truthy(o)`, `pico8_value_equal(o, FIXED r.w)` | `o` vs nombre | non |
| `planet_display.c:214`, `:253` | `pico8_value_equal(kw_qe, FIXED r.w)`, `kind == FIXED && gu(cq, …)` | `kw.qe` vs nombre | non |
| `research_display.c:58` | `pico8_value_tostr(qe)` puis `pico8_iz_lookup` | besoin | non (voir `hp:jb`) |
| `research_display.c:204-207` | `assert` FIXED ou `REF NULL`, puis item | besoin | non |

**Où nil contre faux décide** (G2, TROU-24) :

1. **`jo:cn`, `planet_trade.c:167`** (`t.qe==i.qe`, lua.lua:494). Une route venue directement de `jo:ib` sans `qe` est NIL ; face à un besoin NIL, `pico8_value_equal(REF NULL, REF NULL)` vaut 1 : elle répond, comme dans le cart. La même route passée par `kg` (`nr:cn`, `planet_family_trade.c:145`, qui la transmet au `cn` suivant) est FALSE : `pico8_value_equal(BOOL 0, REF NULL)` vaut 0 ; elle ne répond plus, alors que le cart (nil == nil) répond. **C'est l'effet de TROU-24.**
2. **`dh`, `economy_tiers.c:256`** (`im.qe==i.qe`, lua.lua:1297). La garde `im->has_qe` écarte une route NIL ; il ne reste que FALSE et FIXED face à un besoin NIL ou FIXED. `BOOL 0` contre `REF NULL` vaut 0. **Confondre nil et faux ferait compter une route FALSE pour un besoin NIL** : `gc` changerait. La garde elle-même est l'autre moitié de TROU-24 (« même effondrement nil/absent dans `dh` ») : dans le cart, une route sans `qe` compte pour un besoin sans `qe`.

Partout ailleurs, un côté est un nombre ou la lecture ne teste que le genre : nil et faux n'y sont pas distingués.

**Table exigée de `pico8_qe_equal`** (reproduit `pico8_value_equal` sur les genres que `src/` produit) :

| a \ b | NIL | FALSE | FIXED `m` |
|---|---|---|---|
| NIL | 1 | 0 | 0 |
| FALSE | 0 | 1 | 0 |
| FIXED `n` | 0 | 0 | `n == m` |

(`n` d'un NIL ou d'un FALSE n'est jamais lu.)

**Atteinte** : TROU-24 le dit, aucun e2e ne construit de route sans `qe`. Aucun TU non plus ne fait se rencontrer une route FALSE et un besoin NIL (grep : `pico8_jo_cn` n'est appelé que par `test_pico8_trade:498`, route à nombre ; les besoins NIL de `tests/` ne sont que dans `test_pico8_lq:762,801-802`, hors `cn`/`dh`). **Une mutation qui confondrait nil et faux ne rougirait rien aujourd'hui** : D.17 ajoute les tests qui la font rougir (« Tests ajoutés »).

### Lecture de la ligne pour D.17

- **« `pico8_qe_t` »** : **forme de l'étude**, dans `pico8_trade.h` (le type des routes, où naît le FALSE) :

  ```c
  typedef struct {
      enum { PICO8_QE_NIL, PICO8_QE_FALSE, PICO8_QE_FIXED } kind;
      pico8_fixed_t n;
  } pico8_qe_t;
  ```

  Valeurs construites par initialiseurs désignés (`{.kind = PICO8_QE_NIL}`) et littéraux composés (`(pico8_qe_t){.kind = PICO8_QE_FIXED, .n = x}`), la forme déjà employée par `research_display.c:140,207` pour `pico8_jb_item_t` ; **ce ne sont pas des casts**. **(G)** Rejetés : des constructeurs `pico8_qe_nil()`/`pico8_qe_fixed()` (trois fonctions pour des littéraux d'une ligne, et un symbole de plus pour la règle de lien) ; un genre `TRUE` (G4 : aucun site de `src/` n'écrit un `qe` vrai) ; un drapeau seul (perdrait TROU-24, étude).
- **« `pico8_qe_equal` »** : `int pico8_qe_equal(pico8_qe_t a, pico8_qe_t b)`, défini dans `trade.c` : `return a.kind == b.kind && (a.kind != PICO8_QE_FIXED || a.n == b.n);`. Contrat au prototype : « Lua's `==` on two `qe`: nil equals nil, false equals false, nil is not false; numbers by value. »
- **`pico8_trade_qe`** : `pico8_qe_t pico8_trade_qe(const pico8_trade_t *self)` ; NIL pour `self` NULL ou `!has_qe`, FALSE si `qe_is_bool`, sinon FIXED. **(G)** Un `qe` vrai (possible par le constructeur, écrit seulement par `test_pico8_trade:381`) s'arrête sur un `assert` porteur de message (« no route of src/ holds true: `pico8_qe_t` has no kind for it »), et un test `must_abort` le fixe. Rejeté : resserrer l'`assert` du constructeur (il changerait le constructeur et le test `:381`, pour un état que `pico8_trade_eq` ne lit pas).
- **`pico8_jy_qe`** : `pico8_qe_t pico8_jy_qe(const pico8_jy_t *i)`, NIL ou FIXED, jamais FALSE (contrat réécrit). `research.h` gagne **`#include "pico8_trade.h"`** (seul include ajouté ; pas de cycle : `trade.h` n'inclut que `fixed`, `ptrs`, `struct`, `value`).
- **`jo:cn`** (`planet_trade.c`) : `r_e`, `r_q`, `t_qe`, `qe_val` deviennent des `pico8_qe_t` ; chaque `pico8_value_equal` devient `pico8_qe_equal` ; les gardes `have_r_e &&`/`have_r_q &&` **restent** (un `r_e` NIL serait égal à une route NIL). `r_e.as.fixed` (`:120`), `t_qe.as.fixed` (`:157`, `:162`) deviennent `.n`.
- **`dh`** (`economy_tiers.c`) : `economy_value_equal_content` part (sans appel, `-Werror`) ; `:256` devient `im->has_qe && pico8_qe_equal(pico8_trade_qe(im), qe_val)` ; **la garde `has_qe` reste** (TROU-24) et son marqueur `/* TROU-3 */` (`:255`) devient `/* TROU-24 */` : la ligne ne compare plus de chaînes, elle porte l'écart nil/absent.
- **Argument `o` de `pico8_jo_ex`** : **(G)** `const pico8_fixed_t *o`, NULL pour nil — le « drapeau + nombre » de l'étude passé comme `pico8_jy_new(const pico8_fixed_t *qe)` (`research.h:24`), voisin du même en-tête ; les appelants passent `has ? &membre : NULL`. Rejeté : deux paramètres `int has_o, pico8_fixed_t o` (un état incohérent de plus, une signature plus longue) ; `pico8_qe_t` (`o` n'est jamais FALSE ni comparé à un `qe` de route). `pico8_research_display_truthy` part (dernier appelant `:156`, `-Werror`) ; `:156` devient `if (o == NULL) {`.
- **`ml` de `jo_ex`** : **(G, écart déclaré à l'étude)** `ml` tient `{25,23}` ou `{o}` (lua.lua:531) : des numéros de sprite. Une fois `o` typé, `ml[0] = *o` exige **`pico8_fixed_t ml[2]`** ; garder `ml` en valeur obligerait à écrire `ml[0] = pico8_value_fixed(*o)`, une ligne gagnée par `valeur` (cale). L'étude rangeait ces 5 lignes (`:151`, `:168`, `:169`, `:251`, `:261`) dans B8 (D.18a) : elles passent à D.17. `assert(qq.kind == …)` (`:261`) part, le type le dit.
- **`jo_ex`, tuple de besoin** (`:195-207`) : `qe` local en `pico8_qe_t` ; l'`assert` `:204-205` part (`pico8_jy_qe` ne rend jamais FALSE : son contrat le dit) ; `:206` teste `qe.kind == PICO8_QE_FIXED`, lit `qe.n`.
- **`hp:lq`** (`:304`) : `self->has_o ? &self->o : NULL`.
- **`jo:hb`, `jo:gh`** (`planet_display.c`) : `jo_kw_qe` part (sans appel) ; `jo:hb` passe `self->kw->has_qe ? &self->kw->qe : NULL` ; `jo:gh` lit `kw->has_qe`/`kw->qe` (« wants more » : `kw != NULL && kw->has_qe && … && kw->qe == pico8_fixed_from_int(r_w)`, la garde NULL de `jo_kw_qe` gardée ; « makes » : `kw->has_qe && pico8_int_seq_has_fixed(pico8_jo_cq(), kw->qe)`, `kw` non NULL sous `mu > 0`). Le marqueur `/* TROU-3 */` de `:213` part (comparaison de nombres).
- **`hp:jb`** (`research_display.c:48-63`) : `num_str = qe.kind == PICO8_QE_FIXED ? pico8_tostr_fixed(qe.n) : NULL;` puis `iz_name = num_str != NULL ? pico8_iz_lookup(num_str) : NULL;` ; `free(NULL)` est permis. Même morceau qu'aujourd'hui pour un besoin NIL : l'ancien `tostr` rendait `""`, que `iz` n'a pas (clés `"28"`…`"25"`, `map_tables.c:12-15`), et le cart cherche `"[nil]"`, qu'il n'a pas non plus ; aucune chaîne n'est affichée, **TROU-13 n'est pas en jeu à ce site**. Le legs de D.15c (`research_display.c:58`) est soldé : `pico8_value_tostr` garde deux appelants, D.18c.
- **« nil ≠ faux conservé »** : par `pico8_qe_equal` (table), par la garde `has_qe` de `dh`, et **par des tests** qui fixent les deux lectures de TROU-24 (aujourd'hui non observées).
- **TU** : `hp` suit `pico8_jy_qe` (pli typé `fold_qe`, mêmes octets) ; `lq` suit `pico8_jo_ex` (G4 : le cas `false` devient le cas nil) ; `trade` reçoit les tests. **`nr` ne suit pas** (l'étude le nommait ; mesuré : il ne nomme ni `pico8_trade_qe`, ni `pico8_jy_qe`, ni `pico8_jo_ex`, ni `pico8_value_equal` ; ses 17 lignes `valeur` sont des plis de D.19).
- **« TROU-3 et TROU-24 relus »** : TROU-3 nomme « un `pico8_value_equal` dans `jo:gh` » que D.17 retire, et décrit `kw.qe` comme une chaîne partageant le pointeur de `r` — faux depuis que `ho` route `o`/`dm` en nombres (`research.c:119-128`). Son paragraphe d'exception est réécrit ; les marqueurs `/* TROU-3 */` des comparaisons de `qe` partent (`planet_display.c:213`, `research_display.c:164`, `economy_score.c:141`) ou deviennent `/* TROU-24 */` (`economy_tiers.c:255`) ; la clause de `glossaire-noms.md:630` qui « justifie l'exception de TROU-3 » est réécrite. TROU-24 est réécrit pour nommer des fonctions au lieu de numéros de ligne périmés (`planet_trade.c:242`, `economy_tiers.c:302`) et dire où l'écart se voit et quel TU le fixe (textes plus bas).
- **« Compte : ≈ 720 »** : estimation de l'étude sur une base de 765 (− 45). Mesuré : **− 52 lignes / − 68 occ.** = 23 (B2) + 17 ((a) rangées avec B2) + 3 (copie `truthy`, B12) + 5 (`ml`, B8 dans l'étude) + 4 (TU `hp` 1, `lq` 3) ; l'arrivée est **≈ 693** parce que D.16a/D.16b finissent à 745 et non à 765. « ≈ » n'est pas un critère ; les deltas exacts le sont.
- **« `class-coverage-net` : oui »** : le constructeur `trade` n'est **pas** modifié (`trade.c:22-47` intact) ; `jo:ib` et `kg`, qui construisent les routes, gardent leurs lignes de construction ; `jo_ex` construit des `lq`. Exigé par la ligne : sortie **identique** à `tools/class_coverage_net_expected.txt`. `sanitize-net` : **non** (aucune durée de vie ni propriétaire ne change : `o` est emprunté le temps de l'appel ; `hp:jb` n'alloue plus de `""` pour un besoin NIL, `free(NULL)`). Balayage ASan+UBSan des TU exigé (consigne).
- **§6** : `planet_gh_row_lq` et `ga_export_panel_by` exécutent `jo:gh` avec `kw.qe = 99` (branche « exports ») : **inchangés**, recompilés, relancés ; `layout_nil_item_draws_nothing` ne lie pas les fichiers de D.17.
- **Découpage** : **(G) un commit** (`refactor(D.17)`) : 10 fichiers de `src/`, 3 TU, 3 docs, ≈ 52 lignes comptées — la taille de D.16a (un commit, ≈ 11 min jusqu'au commit). Repli écrit plus bas.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes comptées, alias, renommage pour échapper aux greps, cast nouveau, cale valeur ↔ typé ; mutations restaurées par `cp` simple puis `touch` ; binaires lancés exactement comme le `Makefile` ; **jamais l'appli raylib**.

### État de départ

| Mesure | À `8533807` (D.16b non commencée) | Départ de D.17, mesuré à `529c71a` (D.16b gradée) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 825 / 97 ; 916 occ. | **745 / 89 ; 831 occ.** (`src/` 318 / 51 / 364, `tests/` 427 / 38 / 467) |
| `type-tag` | 242 / 58 | **242 / 58** |
| résidu D | 1065 / 107 | **985 / 104** |
| fonctions publiques qui nomment `pico8_value_t` (Annexe B) | 13 | **7** |
| copies `*_truthy(pico8_value_t)` | 7 | **3** |
| appelants de `pico8_value_equal` dans `src/` (hors module) | 7 lignes | 7 |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |

Recette des occurrences (celle de D.12–D.16b, motif `valeur` de `tools/data_residue.sh:29`) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'valeur\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire (numéros à `529c71a`, remesurés par contenu après D.16b)

Classes du message de commit : **(a1)** même site, type C à la place de la valeur ; **(a2)** retirée sans remplaçant (fonction sans appel, copie `truthy`, local, `assert` que le type rend vrai) ; **(b)** signature publique dont le type change ; **(c)** même donnée, autre nom : **attendu 0**. `occ.` : occurrences du motif sur la ligne.

#### API

| Symbole | Aujourd'hui | Après D.17 | Appelants (`src/`) | TU |
|---|---|---|---|---|
| `pico8_qe_t`, `pico8_qe_equal` (`trade.h`, `trade.c`) | — | **nouveaux** (hors motif) | `planet_trade.c`, `economy_tiers.c` | `trade` (tests ajoutés) |
| `pico8_trade_qe` (`trade.c:13`, `.h:50`) | rend `pico8_value_t` | **rend `pico8_qe_t`** — (b) | `planet_trade.c:89`, `economy_tiers.c:256` | `trade` (tests ajoutés) |
| `pico8_jy_qe` (`research.c:134`, `.h:21`) | rend `pico8_value_t` | **rend `pico8_qe_t`** — (b) | `planet_trade.c:111,149`, `economy_tiers.c:244`, `research_display.c:56,201` | `hp:450`, `:622` |
| `pico8_jo_ex` (`research_display.c:143`, `.h:27`) | `(…, pico8_value_t o, …)` | **`(…, const pico8_fixed_t *o, …)`** — (b) | `planet_display.c:63`, `research_display.c:304` | `lq:484`, `:495`, `:499` |
| `pico8_research_display_truthy`, `jo_kw_qe`, `economy_value_equal_content` (`static`) | — | **retirées** (sans appel, `-Werror`) — (a2) | — | — |

#### Sites de `src/` (48 lignes, 63 occ. retirées ; aucune gagnée)

| Fichier | Ligne(s) `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `trade.c` | `:13` (1) | `pico8_qe_t pico8_trade_qe(const pico8_trade_t *self) {` | (b) |
| | `:15` (1) | `return (pico8_qe_t){.kind = PICO8_QE_NIL};` | (a1) |
| | `:17` (2) | `return self->qe_is_bool ? (pico8_qe_t){.kind = PICO8_QE_FALSE} : (pico8_qe_t){.kind = PICO8_QE_FIXED, .n = self->qe};` (coupée en deux lignes si le style l'exige, non comptées) ; précédée de l'`assert` du `qe` vrai (ligne ajoutée, hors motif) | (a1) |
| | (ajout) | `int pico8_qe_equal(pico8_qe_t a, pico8_qe_t b)`, après `pico8_trade_qe` | — |
| `trade.h` | `:50` (1) | `pico8_qe_t pico8_trade_qe(const pico8_trade_t *self);` ; contrat `:48-49` réécrit : « `qe` as the cart compares it: nil for a nil `qe` or a NULL self, false only as `kg` writes it (TROU-24); a true `qe` stops the run. » | (b) |
| | (ajout) | `pico8_qe_t` et le prototype de `pico8_qe_equal` avec son contrat, avant `pico8_trade_keys_t` | — |
| `research.c` | `:134` (1) | `pico8_qe_t pico8_jy_qe(const pico8_jy_t *i) {` | (b) |
| | `:135` (2) | `return i->row.has_qe ? (pico8_qe_t){.kind = PICO8_QE_FIXED, .n = i->row.qe} : (pico8_qe_t){.kind = PICO8_QE_NIL};` | (a1) |
| `research.h` | `:21` (1) | `pico8_qe_t pico8_jy_qe(const pico8_jy_t *i);` ; contrat `:20` : « `i.qe`: nil or a number, never false. » ; `#include "pico8_trade.h"` ajouté après `pico8_struct.h` (hors motif) | (b) |
| `planet_trade.c` | `:74`, `:75` (2 chacune) | `pico8_qe_t r_e = {.kind = PICO8_QE_NIL};`, idem `r_q` | (a1) |
| | `:79` (2) | `pico8_qe_t t_qe = {.kind = PICO8_QE_NIL};` | (a1) |
| | `:94`, `:98` (1 chacune) | `r_e = (pico8_qe_t){.kind = PICO8_QE_FIXED, .n = pico8_fixed_from_int(qe)};`, idem `r_q` | (a1) |
| | `:103` (1) | `if (have_r_e && pico8_qe_equal(t_qe, r_e) && !self->has_jh) {` | (a1) |
| | `:111` (1) | `qe_is_r_q = have_r_q && pico8_qe_equal(pico8_jy_qe(i), r_q);` | (a1) |
| | `:120` (0) | `pico8_fixed_t e_qe = r_e.n;` | — |
| | `:143` (1) | `pico8_qe_t qe_val;` | (a1) |
| | `:151` (1) | `if (have_r_q && pico8_qe_equal(qe_val, r_q)) {` | (a1) |
| | `:156` (1) | `accepted = t_qe.kind == PICO8_QE_FIXED &&` | (a1) |
| | `:157`, `:162` (0) | `t_qe.n` à la place de `t_qe.as.fixed` | — |
| | `:160` (1) | `assert(t_qe.kind == PICO8_QE_FIXED && "a want's qe is a resource of r, never a boolean");` | (a1) |
| | `:167` (2) | `if (pico8_qe_equal(t_qe, qe_val) && !(have_r_q && pico8_qe_equal(qe_val, r_q))) {` | (a1) |
| `economy_tiers.c` | `:16`, `:17` (2 chacune), `:20` (1) | `economy_value_equal_content` retirée entière (`:16-21` et la blanche) | (a2) |
| | `:236` (1) | `pico8_qe_t qe_val;` | (a1) |
| | `:255` (0) | `/* TROU-24 */` à la place de `/* TROU-3 */` | — |
| | `:256` (0) | `if (im->has_qe && pico8_qe_equal(pico8_trade_qe(im), qe_val)) {` | — |
| `planet_display.c` | `:27`, `:29`, `:31` (1 chacune) | `jo_kw_qe` retirée entière avec son commentaire (`:26-32` et la blanche) | (a2) |
| | `:48` (2) | `const pico8_fixed_t *qe = NULL;` | (a1) |
| | `:58` (0) | `qe = self->kw->has_qe ? &self->kw->qe : NULL;` | — |
| | `:209` (1) | `const pico8_kw_t *kw = self->kw;` | (a1) |
| | `:213` (0) | `/* TROU-3 */` retiré | — |
| | `:214` (2) | `qe_is_r_w = kw != NULL && kw->has_qe && pico8_r_lookup("w", &r_w) && kw->qe == pico8_fixed_from_int(r_w);` | (a1) |
| | `:250` (1) | local `qe_val` retiré | (a2) |
| | `:253` (1) | `int makes = kw->has_qe && pico8_int_seq_has_fixed(pico8_jo_cq(), kw->qe);` | (a1) |
| `research_display.c` | `:20`, `:21`, `:24` (1 chacune) | `pico8_research_display_truthy` retirée entière avec son commentaire (`:18-28` et la blanche) | (a2) |
| | `:48` (2) | `pico8_qe_t qe = {.kind = PICO8_QE_NIL};` | (a1) |
| | `:58` (1) | `num_str = qe.kind == PICO8_QE_FIXED ? pico8_tostr_fixed(qe.n) : NULL;` | (a1) |
| | `:59` (0) | `iz_name = num_str != NULL ? pico8_iz_lookup(num_str) : NULL;` | — |
| | `:143` (1) | `pico8_lq_t *pico8_jo_ex(const pico8_ptrs_t *is, const pico8_fixed_t *o, pico8_fixed_t jq, pico8_fixed_t mj,` | (b) |
| | `:151` (1) | `pico8_fixed_t ml[2];` | (a1) |
| | `:156` (0) | `if (o == NULL) {` | — |
| | `:164` (0) | `/* TROU-3 */` retiré | — |
| | `:165` (2) | `int o_is_r_w = pico8_r_lookup("w", &r_w) && *o == pico8_fixed_from_int(r_w);` | (a1) |
| | `:168`, `:169` (1 chacune) | `ml[0] = pico8_fixed_from_int(25);`, `ml[1] = pico8_fixed_from_int(23);` | (a1) |
| | `:172` (0) | `ml[0] = *o;` | — |
| | `:195` (2) | `pico8_qe_t qe = {.kind = PICO8_QE_NIL};` | (a1) |
| | `:204` (2) | `assert` retiré avec son message `:205` | (a2) |
| | `:206` (1) | `tuple->items[0] = qe.kind == PICO8_QE_FIXED ? pico8_jo_ex_fixed(qe.n)` (`:207` inchangée) | (a1) |
| | `:251` (1) | `pico8_fixed_t qq = ml[qq_i];` | (a1) |
| | `:261` (1) | `assert` retiré (le type dit « numéro de sprite ») | (a2) |
| | `:262` (0) | `t4->items[0] = pico8_jo_ex_fixed(qq);` | — |
| | `:304` (2) | `l = pico8_jo_ex(is, self->has_o ? &self->o : NULL, jq, pico8_fixed_from_int(0), pico8_fixed_from_int(1));` | (a1) |
| `research_display.h` | `:27` (1) | `pico8_lq_t *pico8_jo_ex(const pico8_ptrs_t *is, const pico8_fixed_t *o, pico8_fixed_t jq, pico8_fixed_t mj,` ; contrat `:24-26` : « … or NULL when `o` is NULL (nil) -- which `jo:hb` does reach, for a planet whose `kw` carries no `qe`. `o` is read during the call only. » | (b) |
| `economy_score.c` | `:141` (0) | `/* TROU-3 */` retiré (même comparaison de nombres que `jo:gh`) | — |

Totaux `src/` (lignes / occ. retirées) : `trade.c` 3 / 4, `trade.h` 1 / 1, `research.c` 2 / 3, `research.h` 1 / 1, `planet_trade.c` 12 / 16, `economy_tiers.c` 4 / 6, `planet_display.c` 8 / 10, `research_display.c` 16 / 21, `research_display.h` 1 / 1 : **48 lignes, 63 occ.** Classes : (a1) 30, (a2) 12, (b) 6.

#### Les TU suivent (4 lignes, 5 occ. retirées)

| TU | Ligne(s) (occ.) | Nouveau texte | Classe |
|---|---|---|---|
| `hp` | `:449` (2) (`:453` à `8533807`) | `pico8_qe_t qe;` (l'initialisation morte part) | (a1) |
| | `:451` (0) | `h = fold_qe(h, qe);` | — |
| | `:622` (0) (`:627` à `8533807`) | `h = fold_qe(fold_view_name(h, "qe"), pico8_jy_qe(want));` | — |
| | (ajout, après `fold_value`) | `fold_qe`, voir ci-dessous | — |
| `lq` | `:476` (1) | `pico8_fixed_t o;` | (a1) |
| | `:482` (1) | `o = p->o;` | (a1) |
| | `:484`, `:495` (0) | `pico8_jo_ex(is, &o, …)`, `pico8_jo_ex(NULL, &o, …)` | — |
| | `:499` (1) | `assert(pico8_jo_ex(NULL, NULL, pico8_fixed_from_int(1), 0, 0) == NULL);` — **G4** : le cas `false` part avec le type ; la branche « `o` absent » reste sous test par nil, le genre que `jo:hb` passe | (a1) |

Totaux `tests/` : `hp` 1 / 2, `lq` 3 / 3. **Total D.17 : 52 lignes, 68 occ.** ; **(a1) 34, (a2) 12, (b) 6, (c) 0.**

**`fold_qe` de `test_pico8_hp`** (G1) — les octets que `fold_value` pliait pour ces deux genres (`fold_value`, `hp:114-140` : genre, puis contenu ; `PICO8_VALUE_FIXED` vaut 0, `PICO8_VALUE_REF` 3, `pico8_value.h:14-19`) :

```c
/* A want's `qe` with the bytes fold_value folded for it, which the frozen
 * hashes hold: kind word 0 then the number; kind word 3 then 0xDEAD for nil. */
static uint32_t fold_qe(uint32_t h, pico8_qe_t qe) {
    assert(qe.kind != PICO8_QE_FALSE && "a want's qe is never false");
    return qe.kind == PICO8_QE_FIXED ? fnv(fnv(h, 0u), qe.n) : fnv(fnv(h, 3u), 0xDEADu);
}
```

`fnv(…, qe.n)` : conversion implicite de `pico8_fixed_t` en `uint32_t` à l'argument, sans cast (`-Wsign-conversion` n'est pas dans `CFLAGS`, `Makefile:97`), même valeur que l'ancien `(uint32_t)v.as.fixed`. Les octets ne dépendent pas de `depth` pour ces deux genres (`fold_value` ne lit `depth` que pour une `REF` non nulle). La branche nil n'est probablement prise par aucun projet de `gb` (tous les caractères de `i` sont dans `r`) : elle reproduit l'ancien pli et n'est pas un critère `gcov` (lignes de `tests/`). **Rejeté** : `fold_value(h, qe.kind == … ? pico8_value_fixed(qe.n) : pico8_value_ref(NULL), 0)` (cale valeur ↔ typé, et lignes gagnées) ; écrire `PICO8_VALUE_FIXED`/`PICO8_VALUE_REF` (lignes gagnées par `valeur`).

#### Tests ajoutés (`test_pico8_trade`, hors motif)

Les tests de régression de TROU-24 n'existent pas (« Atteinte ») ; sans eux, la mutation qui confond nil et faux ne rougit rien. Chacun construit son état directement (prior art : `test_cn_wildcard_want`, `test_pico8_trade:475-500` ; `pico8_ke_ks(…, "ly")`, `test_pico8_jo:479`), dans un enfant de `in_fresh_child`, sans fichier d'événements :

1. **`test_cn_nil_want_after_kg`** : un laboratoire (`pico8_qh_new(NULL)`) dont `jy` tient un seul besoin NIL (`pico8_jy_new(NULL)`) ; une proposition dont la route n'a pas de `qe` (`pico8_trade_new` avec `lp`, sans `qe`) → `pico8_jo_cn` appelle `kf` **une fois** (nil == nil, comme le cart) ; la même proposition avec `pico8_trade_kg(route, …)` (FALSE) → `kf` **n'est pas appelé** (TROU-24 : le cart y répondrait). Commentaire de tête : le piège (« a route through `kg` holds false, not nil: it no longer answers a want with no `qe` -- TROU-24 »).
2. **`test_dh_counts_numbered_routes_only`** : une planète `ly` dont `jy` tient un besoin NIL et un besoin de nombre `x`, et `cc` trois routes : sans `qe` (NIL), passée par `kg` (FALSE), de nombre `x` → après `pico8_dh`, `gc` du besoin NIL vaut **0** (la garde `has_qe` écarte la route NIL, `pico8_qe_equal` sépare FALSE de NIL ; TROU-24 : le cart compterait la route NIL), `gc` du besoin `x` vaut **1**.
3. **`test_qe_equal`** : la table ci-dessus, dans les deux sens, `n` ignoré hors FIXED (un NIL et un FALSE porteurs d'un `n` non nul restent égaux à eux-mêmes), FIXED 0 différent de NIL et de FALSE ; puis `pico8_trade_qe` sur `NULL`, sur une route sans `qe` (NIL), après `kg` (FALSE), sur une route de nombre (FIXED, même `n`).
4. **`must_abort(qe_of_true_route, NULL)`** dans `test_trade_is_a_struct` : `pico8_trade_qe` d'une route au `qe` vrai s'arrête.

**Ordre dans `main`** : 1 et 2 **avant** 3, pour que la mutation M1 rougisse d'abord sur un comportement (`jo:cn`, `dh`), pas sur la table. `#include "../src/pico8_economy_tiers.h"` ajouté au TU (pour `pico8_dh`). Aucune constante nouvelle, aucune empreinte : des `assert`.

#### Copies `truthy`

`pico8_research_display_truthy` part (dernier appelant `:156`). **3 → 2** : restent `layout` (B8, D.18a) et `ui_button` (B9, D.18b).

#### Includes (règle de D.15b)

Un `.c` qui ne nomme plus rien du module perd son `#include "pico8_value.h"` ; un en-tête le garde jusqu'à D.19c. D.17 : **aucun `.c` de l'empreinte n'a d'include propre** (`trade.c`, `research.c`, `planet_trade.c`, `economy_tiers.c`, `research_display.c`, `planet_display.c` l'ont par leur en-tête) : aucun retiré. En-têtes gardés : `trade.h:7`, `research.h:9`, `research_display.h:9` (seules lignes restantes après D.17, pour D.19c), `planet_trade.h:9`, `planet_display.h:8`. **Un include ajouté** : `pico8_trade.h` dans `research.h` (hors motif). Les TU gardent leurs includes.

#### `Makefile` et liens

- Aucun fichier nouveau ; **aucune ligne du `Makefile` ne change** (dépendances d'en-têtes par `-MMD -MP`).
- Règle de lien : aucun fichier de `DATA_ENGINE_SRCS` (`Makefile:229-279`) ne nomme un symbole défini hors de cette liste. Symbole ajouté : `pico8_qe_equal`, défini dans `trade.c` (dans la liste), appelé par `planet_trade.c` et `economy_tiers.c` (dans la liste) et par `test_pico8_trade` (`ENGINE_SRCS`). `pico8_qe_t` se construit par littéraux : aucun symbole pour `research.c`, `research_display.c`. Symboles retirés : trois `static`. Les binaires liés sur `DATA_ENGINE_SRCS` seule (`data_init`, `map_tables`, `jb_ol`, `planet_kj_dispatch`, `bg_kj`), sous ASan (`planet_gh_row_lq`, `Makefile:550`), avec `planet_family.c` (`ga_export_panel_by`, `:553`) et la liste courte de `layout_nil_item_draws_nothing` (`:561`) doivent se lier (grep 6).

## Solution

**Un commit** `refactor(D.17)` (16 chemins : 10 de `src/`, 3 TU, `docs/plan-3.md`, `docs/trous-de-portage.md`, `docs/glossaire-noms.md`), vert seul, vérifié, poussé :

1. Le `qe` d'une route ou d'un besoin est un `pico8_qe_t` : nil, faux ou un nombre ; `pico8_qe_equal` est le `==` de Lua, nil ≠ faux.
2. `jo:cn` et `dh` comparent des `pico8_qe_t` ; la garde `has_qe` de `dh` reste et porte TROU-24.
3. `pico8_jo_ex` reçoit `o` en `const pico8_fixed_t *` (NULL : nil) ; `ml` tient des nombres ; `jo:hb`, `jo:gh`, `hp:lq`, `hp:jb` lisent les membres.
4. `pico8_value_equal` n'a plus d'appelant dans `src/` ; `pico8_value_tostr` en garde deux (D.18c).
5. `hp` et `lq` suivent aux mêmes octets ; `trade` fixe TROU-24 par quatre tests.
6. TROU-3 et TROU-24 relus et réécrits ; le glossaire nomme `pico8_qe_t`.

**Sortie**, constatée par commande :

- `make test` vert (dont `data-guard`, `constants-check`, `float-grep`, `float-guard`) ; `make e2e` **26 conformes** ; `make class-coverage-net` **identique** à l'attendu ; balayage ASan+UBSan des TU propre ;
- `make data-residue` : `valeur` **745 → 693** lignes (**831 → 763** occ.), fichiers **89 → 85** (sortent : `trade.c`, `research.c`, `planet_trade.c`, `economy_tiers.c`) ; `type-tag` 242 / 58 inchangé ; résidu D **985 / 104 → 933 / 102** (`planet_trade.c`, `economy_tiers.c` sortent ; `trade.c`, `research.c` restent par `type-tag`) ; départs mesurés à `529c71a` (si un commit `docs/` s'intercale : départ réel − 52 / − 68 occ.) ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. pico8_value_equal : plus aucun appelant dans src/ hors du module : vide
  grep -rn 'pico8_value_equal' src | grep -v '^src/pico8_value\.'
  # 2. signatures : trois lignes, 'pico8_qe_t pico8_trade_qe(', 'pico8_qe_t pico8_jy_qe(',
  #    'pico8_lq_t *pico8_jo_ex(const pico8_ptrs_t *is, const pico8_fixed_t *o,'
  grep -hnE 'pico8_trade_qe\(|pico8_jy_qe\(|pico8_jo_ex\(' src/*.h
  # 3. copies truthy : 2 fichiers (layout, ui_button)
  grep -lE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c
  # 4. pico8_value_tostr : 2 appelants (pi.c, score_panel_display.c)
  grep -rn 'pico8_value_tostr(' src | grep -v '^src/pico8_value\.'
  # 5. fichiers vidés : 0 partout
  grep -cE "$P" src/pico8_trade.c src/pico8_research.c src/pico8_planet_trade.c src/pico8_economy_tiers.c
  # 6. règle de lien
  make tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_bg_kj tmp/test_pico8_planet_gh_row_lq \
       tmp/test_pico8_ga_export_panel_by tmp/test_pico8_layout_nil_item_draws_nothing
  # 7. aucune constante de TU changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 8. includes : exactement une ligne '+' ('#include "pico8_trade.h"' dans research.h), aucune '-'
  git diff <parent> -- src tests | grep -E '^[-+]\s*#include "(\.\./src/)?pico8_(value|trade)\.h"'
  # 9. marqueurs : TROU-3 absent des sites qe ; TROU-24 à kg et à dh
  grep -n 'TROU-3' src/pico8_planet_display.c src/pico8_research_display.c src/pico8_economy_tiers.c src/pico8_economy_score.c   # vide
  grep -rln 'TROU-24' src   # pico8_trade.c, pico8_trade.h, pico8_economy_tiers.c
  # 10. comptes ; 11. filet de classes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^valeur'
  make class-coverage-net
  ```

## User Stories

1. En tant que mainteneur du port, je veux que le `qe` d'une route et d'un besoin soit une petite struct à trois états (nil, faux, nombre), afin que le type dise ce que le cart et TROU-24 écrivent.
2. En tant que mainteneur du port, je veux une seule fonction `pico8_qe_equal` qui soit le `==` de Lua (nil égal à nil, faux à faux, nil différent de faux, nombres par valeur), afin que `jo:cn` et `dh` comparent comme le cart sans passer par la valeur Lua.
3. En tant que mainteneur du port, je veux que `pico8_trade_qe` rende un `pico8_qe_t` et s'arrête sur un `qe` vrai qu'aucune route n'écrit, afin que l'état impossible ne soit pas lu en silence.
4. En tant que mainteneur du port, je veux que `pico8_jy_qe` rende nil ou un nombre, jamais faux, afin que les lecteurs d'un besoin n'aient pas d'`assert` de genre à tenir.
5. En tant que mainteneur du port, je veux que `jo:cn` compare route, besoin, `r.e` et `r["?"]` en `pico8_qe_t` en gardant ses gardes `have_r_*`, afin qu'un `r` sans la clé reste « jamais égal ».
6. En tant que mainteneur du port, je veux que `dh` garde sa garde `has_qe` marquée TROU-24, afin que l'écart nil/absent reste visible au site qui le porte.
7. En tant que mainteneur du port, je veux que `economy_value_equal_content`, `jo_kw_qe` et la copie `truthy` de `research_display.c` disparaissent avec leurs derniers appels, afin qu'aucune fonction ne reste pour une valeur qui n'existe plus.
8. En tant que mainteneur du port, je veux que `jo.ex` reçoive `o` comme un nombre ou NULL, afin que `if (not o)return` et `o==r.w` s'écrivent en C sur le nombre.
9. En tant que mainteneur du port, je veux que `ml` tienne des numéros de sprite, afin qu'aucune valeur ne soit fabriquée pour être aussitôt défaite.
10. En tant que mainteneur du port, je veux que `jo:hb`, `jo:gh`, `hp:lq` lisent `kw.qe`/`hp.o` par leurs membres, afin que `planet_display.c` ne refabrique plus de valeur.
11. En tant que mainteneur du port, je veux que `hp:jb` ne passe plus par `pico8_value_tostr`, avec le même morceau vide pour un besoin sans `qe`, afin que le legs de D.15c soit soldé.
12. En tant que mainteneur du port, je veux que `pico8_value_equal` n'ait plus d'appelant dans `src/`, afin que D.19c n'ait plus qu'à supprimer le module.
13. En tant que lecteur de `docs/trous-de-portage.md`, je veux que TROU-24 nomme les fonctions où l'écart se voit et le TU qui le fixe, et que TROU-3 cesse de décrire `kw.qe` comme une chaîne, afin que les deux entrées disent vrai.
14. En tant que lecteur du glossaire, je veux une ligne pour `pico8_qe_t`/`pico8_qe_equal`, afin de retrouver le type depuis le nom `qe`.
15. En tant que gardien des tests, je veux que `hp` plie les `qe` des besoins aux mêmes octets par un pli typé sans cast, afin qu'aucune constante ne change.
16. En tant que gardien des tests, je veux que le cas `jo.ex` d'un `false` devienne le cas nil (G4), afin que la branche « `o` absent » reste sous test avec le genre que `src/` passe.
17. En tant que gardien des tests, je veux des tests qui fixent les deux lectures de TROU-24 (`jo:cn` après `kg`, `dh` sur des routes sans nombre) et la table de `pico8_qe_equal`, afin qu'une mutation qui confond nil et faux rougisse.
18. En tant que gardien des filets, je veux `make test`, `make e2e`, `make class-coverage-net` identique et le balayage ASan+UBSan verts, afin de prouver qu'aucun constructeur, pixel ou accès mémoire ne change.
19. En tant que gardien des tests, je veux `gcov` avant/après et des mutations restaurées par `cp`, afin que chaque ligne modifiée soit exécutée et qu'aucune ligne survivante ne cesse de l'être.
20. En tant que gardien du compteur, je veux au message du commit les comptes par catégorie et par fichier en lignes et occurrences, la classification (a1)/(a2)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris), les `ml` pris à B8, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
21. En tant que `plan-pilot`, je veux une étape d'environ vingt minutes, un plafond, un repli et une note « Découvert à D.17 », afin que D.18a parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules, tests et découpage ; ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)** ; les rejets sont notés. Q12 (oui) et Q13 (A) sont des décisions humaines ; l'étude D.14 (B2, G1–G4) fait foi sauf écart déclaré ici (`ml` pris à B8 ; `o` en pointeur plutôt qu'en deux paramètres ; `nr` hors empreinte ; assertion du `qe` vrai).

### Modules

- **Route** : `pico8_trade.c/.h` (`pico8_qe_t`, `pico8_qe_equal`, `pico8_trade_qe`).
- **Besoin** : `pico8_research.c/.h` (`pico8_jy_qe`).
- **Échanges** : `pico8_planet_trade.c` (`jo:cn`), `pico8_economy_tiers.c` (`dh`), `pico8_economy_score.c` (marqueur seul).
- **Affichage** : `pico8_research_display.c/.h` (`jo.ex`, `hp:lq`, `hp:jb`), `pico8_planet_display.c` (`jo:hb`, `jo:gh`).
- **TU** : `hp`, `lq` (suivent), `trade` (tests ajoutés). **Docs** : `plan-3.md` (note), `trous-de-portage.md` (TROU-3, TROU-24), `glossaire-noms.md` (une ligne ajoutée, une clause réécrite).
- Inchangés : `Makefile`, `tools/`, `pico8_value.c/.h`, `pico8_type_tag.h`, les membres de `pico8_trade_t`, `pico8_trade_keys_t` et le constructeur `trade`, `kg`, `jo:ib`, `nr:cn`, `mx:kf`, `pico8_kw_t`, `pico8_jb_row_t`, `test_pico8_nr`, `test_pico8_value`, les trois tests du §6.

### Représentation (G)

- `pico8_qe_t` (forme de l'étude, `trade.h`) ; `pico8_qe_equal` dans `trade.c`.
- Route : NIL / FALSE / FIXED ; besoin : NIL / FIXED ; `o` et `kw.qe` : `const pico8_fixed_t *` au passage, membres `has_*` + nombre inchangés.
- **Piège de motif** : aucun nom neuf ne correspond à un motif (`PICO8_QE_*` n'est pas `PICO8_VALUE_*` ; `pico8_qe_equal`, `fold_qe` ne touchent ni `spec` ni `par-nom`).

### Anti-tassement et message de commit (liant)

- **Interdit** : joindre des lignes comptées (`jo_ex` garde sa signature sur deux lignes ; `:214` de `planet_display.c` n'absorbe pas `:209`) ; renommer pour échapper au motif ; alias (`#define`, `typedef` d'un nom existant) ; cast nouveau (littéraux composés et initialiseurs désignés ne sont pas des casts ; `fold_qe` passe `qe.n` sans cast) ; **cale valeur ↔ typé** (une `pico8_qe_of_value`, un `fold_value(…, pico8_value_fixed(qe.n))` dans un TU, `ml` gardé en valeur, une `jo_kw_qe` gardée) ; toucher une constante de hachage, `tests/e2e/`, un `*.sha256`, `tools/`, le `Makefile`, un include autre que celui de `research.h`.
- **Classification exigée** (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif en regard des `+`) : **(a1) 34, (a2) 12, (b) 6, (c) 0**, recomptée au parent.
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et par fichier du diff, en lignes (`--detail`) **et** occurrences.
- **Lignes qui gagnent une catégorie**, par fichier et catégorie, « aucune » compris : attendu **aucune**.
- **Lignes comptées modifiées, non retirées** : attendu **aucune** (toute ligne `valeur` touchée sort de la catégorie).
- **Δlignes > Δoccurrences** : attendu aucun fichier (Δocc ≥ Δlignes partout).
- **Lignes prises à une autre étape** : les 5 lignes `ml`/`qq` (`research_display.c:151,168,169,251,261`), rangées B8 (D.18a) par l'étude, déclarées au message et dans la note.
- **Numéros de ligne** : ceux du parent.

### Empreinte et découpage (G)

| Commit | `src/` | `tests/` | docs | Chemins |
|---|---|---|---|---|
| **D.17** | `trade.c`, `trade.h`, `research.c`, `research.h`, `planet_trade.c`, `economy_tiers.c`, `economy_score.c`, `planet_display.c`, `research_display.c`, `research_display.h` (10) | `hp`, `lq`, `trade` (3) | `plan-3.md`, `trous-de-portage.md`, `glossaire-noms.md` | 16 |

(L'étude disait ≈ 14 fichiers et nommait les TU `trade`, `hp`, `nr`, `lq` : `nr` sort, `economy_score.c` et deux docs entrent.)

- **Rythme mesuré** (horodatage des commits) : D.15a–D.16a, ≈ 7 à 11 min de la mise en file au commit `refactor`, ≈ 13 à 18 min jusqu'à la graduation. **D.17** : éditions ≈ 6 min ; tests ajoutés ≈ 4 ; `gcov` départ/arrivée ≈ 4 ; 21 mutations ≈ 6 ; filets ≈ 3 ; mesures, message, note ≈ 3 : **budget ≈ 20 min jusqu'au commit, plafond 45 min** (≤ 50).
- **Repli écrit** (G) : au plafond sans commit, rien n'est commis et `plan-pilot` relance en **D.17a** (`pico8_qe_t`, `pico8_qe_equal`, `pico8_trade_qe`, `pico8_jy_qe`, `jo:cn`, `dh`, et les lecteurs de `pico8_jy_qe` de `research_display.c` — `hp:jb` `:48-58`, tuple `:195-207` — qui doivent suivre son type ; TU `hp`, `trade`) puis **D.17b** (`o` de `jo_ex`, `ml`, copie `truthy`, `hp:lq`, `planet_display.c`, `economy_score.c` ; TU `lq` ; TROU-3, glossaire). Comptes recomptés par commit. Rejeté comme plan principal : deux coûts fixes pour ≈ 52 lignes.

### Effet sur `make data-residue`

| Catégorie | À `8533807` | Départ (mesuré à `529c71a`) | Après D.17 |
|---|---|---|---|
| `valeur` | 825 / 97 ; 916 occ. | 745 / 89 ; 831 occ. | **693 / 85 ; 763 occ.** (− 52, − 68) |
| `valeur` `src/` | 387 / 59 ; 438 occ. | 318 / 51 ; 364 occ. | 270 / 47 ; 301 occ. |
| `valeur` `tests/` | 438 / 38 ; 478 occ. | 427 / 38 ; 467 occ. | 423 / 38 ; 462 occ. |
| `type-tag` | 242 / 58 | 242 / 58 | inchangé |
| résidu D | 1065 / 107 | 985 / 104 | **933 / 102** |
| `DATA_RESIDUE_GUARD` | 8 catégories | inchangé | inchangé |

Par fichier (lignes / occ. ; départ attendu → après) : `trade.c` 3/4 → 0/0 ; `trade.h` 2/2 → 1/1 ; `research.c` 2/3 → 0/0 ; `research.h` 2/2 → 1/1 ; `planet_trade.c` 12/16 → 0/0 ; `economy_tiers.c` 4/6 → 0/0 ; `planet_display.c` 38/40 → 30/30 ; `research_display.c` 26/31 → 10/10 ; `research_display.h` 2/2 → 1/1 ; `test_pico8_hp` 35/36 → 34/34 ; `test_pico8_lq` 69/74 → 66/71 ; `test_pico8_trade` 4/4 → 4/4 ; `economy_score.c` 0/0.

Fonctions publiques qui nomment `pico8_value_t` (Annexe B) : 7 → **4** (`pico8_lq_keys_of_row`, `pico8_lq_add`, `pico8_pj_item_of`, `pico8_js_pq`).

« − 52 / − 68 » (ou départ réel moins ces deltas) et « inchangé » sont des critères (écart = explication chiffrée au message).

### Documentation (G)

- `docs/plan-3.md` : une note « Découvert à D.17 » en fin de ligne D.17, rien d'autre. Elle dit : le commit ; les comptes réels et l'écart à ≈ 720 (base 745) ; les lectures (`pico8_qe_t` dans `trade.h`, littéraux ; `o` en `const pico8_fixed_t *` ; `assert` du `qe` vrai ; `has_qe` de `dh` porte TROU-24 ; tests TROU-24 ajoutés à `test_pico8_trade`) ; **pour D.18a** : les 5 lignes `ml` sont parties, B8 compte 5 de moins ; `research_display.c` n'a plus que ses 10 lignes de cases ; **pour D.19b** : `test_pico8_trade` (`fold_qe`, `fold_ref`, octets `PICO8_VALUE_*`) et `hp` (`fold_qe` écrit ici en octets) ; **pour D.19c** : `trade.h`, `research.h`, `research_display.h` n'ont plus que leur include, `pico8_value_equal` n'a plus que `test_pico8_value`.
- `docs/trous-de-portage.md`, **TROU-24** réécrit (brouillon ; l'implémenteur peut resserrer) :

  > Dans le cart, `trade:kg` recopie `self.qe` tel quel : nil reste nil, et aucune route n'a jamais `qe=false`. Ce port pose `false` quand la route allongée n'a pas de `qe`. Dans la struct `trade`, nil et absent ne font qu'un : un `qe` nil, passé explicitement ou omis, donne `false`.
  >
  > `pico8_qe_t` garde les trois états et `pico8_qe_equal` est le `==` de Lua : l'écart se voit là où une route rencontre un besoin sans `qe`. Dans `jo:cn`, une route venue de `jo:ib` sans `qe` (nil) répond à un tel besoin, comme dans le cart ; passée par `kg` (`nr:cn`), elle vaut `false` et n'y répond plus. Dans `dh`, la garde `has_qe` écarte en plus la route nil que le cart compterait : une route sans nombre n'y compte jamais.
  >
  > Non atteint par les e2e : toute route qu'ils construisent a un `qe` non nil. `jo:ib` passe pourtant un `qe` nil quand la table `kw` de la planète n'a pas de `qe`. `test_pico8_trade` fixe les deux lectures.
  >
  > Fichiers : `src/pico8_trade.c`, `src/pico8_planet_trade.c`, `src/pico8_economy_tiers.c`.

- **TROU-3** : le paragraphe « Une exception, qui explique deux sites voisins… » est remplacé par :

  > **Hors de TROU-3 : les `qe`.** Ce que `ho()` route à travers `r` (`hp.o`, `hp.dm`, donc `kw.qe`) et les besoins `{qe=r[c]}` sont des nombres — l'indice d'une ressource —, pas des chaînes : ils se comparent par valeur (`==` sur `pico8_fixed_t`, `pico8_qe_equal`). `db()`, qui compare une chaîne, passe par le contenu.

  Le reste de TROU-3 (dont « Fichiers : `src/pico8_value.c`, `src/pico8_collect.c` ») reste pour D.19c.
- `docs/glossaire-noms.md` : ligne ajoutée après `pico8_trade_keys_t` (l. 25) : « | — | `pico8_qe_t`, `pico8_qe_equal` (`pico8_trade.h`) | un `qe` de route ou de besoin tel que le cart le compare : nil, faux (seulement par TROU-24) ou un nombre ; `pico8_qe_equal` est le `==` de Lua (nil ≠ faux) | » ; l. 630, « c'est `gp` qui rend `o`/`dm` identiques au pointeur de `r`, donc qui justifie l'exception de TROU-3 » devient « c'est `gp` qui fait de `o`/`dm` le nombre que `r` donne ».
- Commentaires : quatre sortes de `CLAUDE.md` ; contrats réécrits (`trade.h:48-49`, `research.h:20`, `research_display.h:24-26`) ; contrats nouveaux (`pico8_qe_t` : `n` lu seulement pour FIXED ; `pico8_qe_equal` : nil ≠ faux, le piège) ; commentaire de `fold_qe` (octets figés) ; commentaires retirés avec leur objet (`planet_display.c:26`, `research_display.c:18-19`) ; aucun renvoi à D.17 ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile` ; aucun e2e ni TU ajouté ou supprimé ; toute constante `*_HASH`/`identity[]` ; `tools/class_coverage_net_expected.txt`.
- Le comportement : mêmes routes proposées et acceptées (`jo:cn`), mêmes `gc` et paliers (`dh`), mêmes rangées `jo.ex`/`jo:gh`/`hp:lq`, même infobulle `hp:jb`, mêmes durées de vie ; TROU-24 à l'identique ; `make e2e` conforme au bit près.
- Les membres `has_qe`, `qe_is_bool`, `qe` de `pico8_trade_t` et de ses clés (étude) ; `pico8_kw_t`, `pico8_jb_row_t` ; les cases de `lq` (`pico8_value_t *l` de `jo_ex`, `pico8_lq_add`), sources de bouton, `pi`, `s[i]` (D.18) ; les plis de `test_pico8_trade`, `ga`, `lq`, `nr` (D.19).

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : sous `-Wall -Wextra -Werror`, les trois `static` retirées seraient fatales si gardées (retirées dans le même commit) ; aucune aide de TU ne devient sans appel (`fold_value` de `hp` garde ≥ 15 appels) ; `<string.h>` reste utilisé par `economy_tiers.c` (`strcmp` de `me`) ; les initialiseurs désignés ne déclenchent pas `-Wmissing-field-initializers`.
- **H2** : aucun autre lecteur ou écrivain de `pico8_trade_qe`, `pico8_jy_qe`, `pico8_jo_ex`, `jo_kw_qe`, `pico8_value_equal` (dans `src/`) n'existe hors de l'Inventaire (grep à `8533807`, `src/`, `tests/`, `tools/`) ; un oublié se révèle à la compilation et suit (listé).
- **H3** : aucun site de `src/` n'écrit un `qe` vrai ; si l'`assert` de `pico8_trade_qe` tombe dans un TU ou un e2e, H3 est fausse : arrêt, rapporté (un genre sans règle).
- **H4** : `pico8_jy_qe` ne rend jamais FALSE ; `fold_qe` de `hp` l'affirme.
- **H5** : les quatre tests ajoutés passent sur le code converti **et** sur le code de départ (ils fixent le comportement existant) : l'implémenteur les écrit d'abord contre l'arbre de départ, avec `pico8_value_equal` (s'ils rougissent au départ, l'analyse de TROU-24 est fausse : arrêt, rapporté).
- **H6** : `research.h` compile avec `pico8_trade.h` inclus, sans cycle ni double définition.

## Implementation Chunks

**D.17** (un commit)

1. Mesure de départ (`529c71a` ou un descendant `docs/` seul : 745 / 89 / 831 occ., `type-tag` 242 / 58, résidu D 985 / 104, mesurés à `529c71a`) ; remesure par contenu ; `gcov` de départ.
2. **Tests ajoutés d'abord** (`test_pico8_trade`, 1 à 4 moins le `must_abort` du `qe` vrai, qui n'existe qu'avec l'`assert`) contre l'arbre de départ : verts (H5). Ils fixent le comportement que la conversion doit garder.
3. `trade.h/.c` (`pico8_qe_t`, `pico8_qe_equal`, `pico8_trade_qe` et son `assert`) ; `research.h/.c` (`pico8_jy_qe`, include) ; `planet_trade.c` ; `economy_tiers.c`.
4. `research_display.h/.c` (`jo_ex`, `ml`, `hp:lq`, `hp:jb`, copie `truthy`) ; `planet_display.c` ; marqueur de `economy_score.c`.
5. TU `hp` (`fold_qe`), `lq` ; `must_abort` du `qe` vrai dans `trade` ; `make tmp/test_pico8_<nom>` et lancement de chacun.
6. Docs : TROU-24, TROU-3, glossaire.
7. `make test`, `make e2e`, `make class-coverage-net`, balayage ASan+UBSan ; `gcov` d'arrivée ; mutations M1–M21 ; mesures, classification ; note « Découvert à D.17 » ; **commit**, vérification, push.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (routes proposées et acceptées, `gc` d'un besoin, rangée dessinée, infobulle), pas la présence d'une ligne. D.17 change une représentation et trois signatures sans changer un résultat. La preuve : les TU existants, qui plient ces états dans des constantes inchangées ; les quatre tests ajoutés, qui fixent TROU-24 et la table d'égalité ; les 26 e2e au bit près ; `class-coverage-net` identique ; `gcov` et mutations ; le balayage ASan+UBSan.

### TU touchés (G)

1. Suivent : `hp` (`pico8_jy_qe`), `lq` (`pico8_jo_ex`). Reçoit des tests : `trade`.
2. Prouvent sans suivre : `fr`, `mx`, `os`, `pm`, `nr`, `hj`, `nl` (`jo:ib` puis `dh` pliés dans leurs empreintes : `jo:cn`, `dh`, `pico8_trade_qe`, `pico8_jy_qe`) ; `ga`, `jo` (`jo:hb`, `jo:gh`) ; `js` (`hp:lq`) ; `hy` (`bb`) ; `planet_gh_row_lq`, `ga_export_panel_by` (§6, `jo:gh` « exports »).
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change ; une constante qui rougit est un bug de conversion.
4. **Assertion nouvelle** : les quatre tests de `trade` (décidés), ou pour couvrir une ligne modifiée non exécutée, ou pour faire rougir une mutation qui ne rougit rien, dans un test du module, listée au message ; rien de lent, aucun fichier d'événements.
5. **Prior art** : plis typés aux mêmes octets (`? 1u : 0u`, D.16a/D.16b ; `fnv(fnv(h, 0xB0u), 1u)`, D.15b) ; fonctions `static` retirées avec leur dernier appel (`pico8_jo_hf`, D.16a ; copies `truthy`, D.15b–D.16b) ; cas G4 retirés (`hg` d'un nombre, D.15b ; `oj` d'un nombre, D.16b) ; tests ajoutés pour une mutation sans rouge (trois à D.16a) ; `test_cn_wildcard_want` (construction directe d'un `cn`).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`, `Makefile:118`), **les TU qui forkent** (26, dont `trade`, `hp`, `lq`, `fr`, `mx`, `nr`, `os` : enfants de `in_fresh_child`/`must_abort`, sortie par `_exit`) compilés en plus avec **`-D_exit=exit`** ; lancés comme le `Makefile` (depuis la racine, `ASAN_OPTIONS=detect_leaks=0` pour `test_pico8_planet_gh_row_lq`) ; union des lignes de `src/` exécutées, départ et arrivée.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée, sinon justifiée au message. Attendus : `pico8_qe_equal` et les trois retours de `pico8_trade_qe` (tests de `trade`) ; `pico8_jy_qe` (`hp`, `trade`) ; `jo:cn` `:103-167` (`trade`, TU de la famille) ; `dh` `:236-256` (`trade`, famille) ; `jo:hb` (`ga`, `jo`, famille) ; `jo:gh` `:209-214` (§6, `lq` « wants more »), `:250-253` (§6 pour « exports » ; « makes » par un TU qui en plie la rangée, sinon e2e) ; `jo_ex` `:156-172`, `:195-207`, `:251-262` (`lq`, `hp`) ; `hp:lq` `:304` (`hp`, `lq`, `js`) ; `hp:jb` `:48-59` (`hp`).
- **`L`** = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide**. Partantes, hors `L` : corps de `pico8_research_display_truthy`, `jo_kw_qe`, `economy_value_equal_content` ; les `assert` `:204-205`, `:261` de `research_display.c`. **Surveillé** : `pico8_value_equal` (`pico8_value.c:36-51`) perd ses appelants de `src/` ; ses branches restent exécutées par `test_pico8_value` (FIXED, BOOL, REF, STRING, genres différents) : si une ligne tombe dans `L`, elle est justifiée par ce seul appelant (D.19c) ; de même `pico8_value_tostr` (encore appelée par `pi.c`, `score_panel_display.c`, `test_pico8_value`). Toute autre ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail : fichier sauvegardé par `cp` avant, restauré par `cp` simple puis `touch` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance ; chacune doit rougir au moins le TU nommé, sinon trou couvert par une assertion ou justifié (mutation équivalente démontrée). **Astuces** : retirer un appel ou une affectation peut échouer sous `-Werror` : neutraliser par `0 ? x : …`, `0 &&` ou une condition toujours fausse plutôt que supprimer ; si un `assert` de `src/` attrape la mutation avant le TU, relancer avec cet `assert` relâché dans la copie mutée, et rapporter les deux.

- **M1 (exigée) — confondre nil et faux dans `pico8_qe_equal`** : `return (a.kind == b.kind || (a.kind != PICO8_QE_FIXED && b.kind != PICO8_QE_FIXED)) && (a.kind != PICO8_QE_FIXED || a.n == b.n);` → **`test_pico8_trade` rouge**, d'abord sur `test_cn_nil_want_after_kg` (`kf` appelé pour la route FALSE), puis, relancée avec ce seul test neutralisé dans une copie du TU, sur `test_dh_counts_numbered_routes_only` (`gc` 1 au lieu de 0) et sur `test_qe_equal` ; les trois rougeurs rapportées. **Si M1 ne rougit pas : `FAILED:`.**
- **M2** `pico8_qe_equal` sans le nombre (`(a.kind != PICO8_QE_FIXED || 1)`) → `trade` (`test_qe_equal`), TU de la famille (`dh`, `jo:cn`) ;
- **M3** nil ≠ nil (`a.kind == b.kind && a.kind != PICO8_QE_NIL && …`) → `trade` (`test_cn_nil_want_after_kg` : `kf` 0 au lieu de 1) ;
- **M4** `pico8_trade_qe` : FALSE rendu NIL → `trade` (`kf` appelé après `kg`) ;
- **M5** `pico8_trade_qe` : nil rendu FALSE → `trade` (`kf` non appelé pour la route de `jo:ib`) ;
- **M6** `pico8_trade_qe` : `assert` du `qe` vrai relâché (`assert(1 || …)`) → `trade` (`must_abort` du `qe` vrai) ;
- **M7** `pico8_jy_qe` : nil rendu FALSE → `trade` (`jo:cn` : `kf` 0), `lq`/`hp` s'ils plient un besoin nil, sinon l'`assert` de `fold_qe` ;
- **M8** `jo:cn` `:103` : `have_r_e && 0 &&` → TU de la famille s'ils plient un `jh`, sinon e2e, sinon trou justifié ;
- **M9** `jo:cn` `:156` : `accepted = 0 &&` → `trade` (`test_cn_wildcard_want`) ;
- **M10** `jo:cn` `:162` : `i->row.qe = 0 ? t_qe.n : 0;` → `trade` (`test_cn_wildcard_want`) ;
- **M11** `jo:cn` `:167` : `if (0 && pico8_qe_equal(t_qe, qe_val) && …)` → `trade`, famille ;
- **M12** `dh` `:256` : garde `has_qe` retirée (`1 &&`) → `trade` (`test_dh_…` : `gc` 1 au lieu de 0 pour le besoin nil) ;
- **M13** `jo:hb` `:58` : `qe = 0 ? … : NULL;` → TU qui plient `jo.lq` (`ga`, famille), sinon e2e ;
- **M14** `jo:gh` `:214` : `qe_is_r_w = 0 && …` → `lq` (« wants more »), sinon e2e ;
- **M15** `jo:gh` `:253` : `int makes = 0 && …` → TU qui plie la rangée « makes », sinon e2e ;
- **M16** `jo_ex` `:156` : `if (0 && o == NULL)` → `lq` (cas nil : déréférence, rouge par signal) ;
- **M17** `jo_ex` `:165` : `o_is_r_w = 0 && …` → `lq` (`JO_EX_HASH`) si un projet de `ks` a `o == r.w`, sinon `hp`, sinon e2e ;
- **M18** `jo_ex` `:172` : `ml[0] = 0 ? *o : 0;` → `lq` (`JO_EX_HASH`) ;
- **M19** `jo_ex` `:206` : `qe.kind == PICO8_QE_FIXED && 0 ?` → `lq` (`JO_EX_HASH`), `hp` ;
- **M20** `hp:jb` `:58` : branche FIXED neutralisée (`… && 0 ? … : NULL`) → `hp` (infobulle de `jb`) ;
- **M21** (preuve du pli, TU) `test_pico8_hp` : `fold_qe` plie `1u` au lieu de `0u` pour FIXED → `hp` rouge (`GB_HASH`), restauré, vert.
- `hp:lq` `:304` (`NULL` toujours) est couvert par M16/M19 et `HP_LQ_HASH` ; ajouté comme M22 si le temps le permet.

Cibles (numéros à `529c71a`) : `trade.c:13-18`, `planet_trade.c:72-175`, `economy_tiers.c:236-260`, `planet_display.c:45-66`, `:207-254`, `research_display.c:48-63`, `:143-207`, `:247-270`, `:304`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes). Aucun e2e ajouté.
- **`make class-coverage-net`** (exigé par la ligne) : sortie **identique** à `tools/class_coverage_net_expected.txt` (`trade 247`, `lq 2742`, `hp 884`, `jo 2430`…).
- **Balayage ASan+UBSan des TU** : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`.
- `make sanitize-net` : non exigé ; s'il est lancé, résultat rapporté.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 min de temps réel jusqu'au commit**, mesures, tests ajoutés, `gcov`, mutations et filets compris ; **plafond 45 min** (au-delà : repli D.17a/D.17b). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute (`trade.h`, `research.h` changent : recompilation large) |
| `make e2e` | ~1 s |
| `make class-coverage-net` | < 1 min |
| `make line-coverage EVENTS="tests/e2e/*.json"` | < 1 min |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes |
| mutations (21) | < 7 min |
| balayage ASan+UBSan des TU | quelques minutes |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; rien d'inachevé n'est commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »**, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + état) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | `529c71a` (D.16b gradée), avant D.17 | 0 | `valeur 745 89` (mesuré à `529c71a`) | … |
| 2 | recette `valeur` | idem | 0 | `valeur 745 lignes 89 fichiers 831 occ.` | … |
| 3 | `grep -rn 'pico8_value_equal' src \| grep -v '^src/pico8_value\.' \| wc -l` | idem | 0 | 7 (mesuré à `529c71a`) | … |
| 4 | `make tmp/test_pico8_trade && ./tmp/test_pico8_trade` | idem + tests ajoutés (H5) | 0 | `test_pico8_trade: all tests passed` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les 16 chemins de « Empreinte » (plus un TU suiveur listé au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`.
2. **Greps** de la Sortie, exactement.
3. **Formes** : `pico8_qe_t` à trois genres dans `trade.h` ; `pico8_qe_equal` conforme à la table ; `pico8_trade_qe`/`pico8_jy_qe` rendent `pico8_qe_t` ; `pico8_jo_ex(…, const pico8_fixed_t *o, …)` ; `ml` en `pico8_fixed_t` ; garde `has_qe` de `dh` présente sous `/* TROU-24 */` ; gardes `have_r_e`/`have_r_q` de `jo:cn` présentes ; membres de `pico8_trade_t` inchangés.
4. **Casts** : aucun nouveau (`grep -nE '\(\s*(const\s+)?[a-z0-9_]+_t\s*\**\s*\)\s*[^{ ]'` sur les lignes `+` du diff : vide ; les littéraux composés `(pico8_qe_t){…}` en sont exclus par le `{`).
5. **API** : seules `pico8_trade_qe`, `pico8_jy_qe`, `pico8_jo_ex` changent de signature ; seules les trois `static` disparaissent ; seuls `pico8_qe_t` et `pico8_qe_equal` apparaissent.
6. **Liens** : grep 6.
7. **Constantes** : grep 7 vide ; M1 et M21 rapportées.
8. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste ; aucune ligne gagnée ; les 5 lignes `ml` déclarées ; includes (grep 8).
9. **Écarts déclarés** : `ml` pris à B8 ; `o` en pointeur ; `nr` hors empreinte ; `assert` du `qe` vrai ; `economy_score.c` (marqueur seul) ; tests TROU-24 ajoutés.
10. **Preuve** : `gcov` (lignes modifiées exécutées ou justifiées ; `L` vide ou justifié par `test_pico8_value` seul), mutations restaurées par `cp`, `class-coverage-net` et ASan+UBSan rapportés.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.17 ni au plan dans `src/` ou `tests/` ; marqueurs (grep 9).
12. **Docs** : note « Découvert à D.17 » en fin de ligne D.17, rien d'autre dans le plan ; TROU-24 et TROU-3 réécrits sans nommer `pico8_value_equal` ni un numéro de ligne ; glossaire (une ligne ajoutée, une clause réécrite).

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `529c71a` (ou un descendant qui ne touche que `docs/`), propre ; ou si `valeur` n'y vaut pas 745 / 89 / 831 occ., `type-tag` 242 / 58, résidu D 985 / 104 (mesurés à `529c71a`), sans explication.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change.
- `FAILED:` si un site ne tombe sous aucune règle de ce PRD ni de l'étude (arrêt « champ sans règle ») — en particulier un `qe` vrai écrit par `src/` (H3) —, ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si un test ajouté rougit sur l'arbre de départ (H5 : l'analyse de TROU-24 est fausse) sans arrêt rapporté.
- `FAILED:` si `make class-coverage-net` diffère de l'attendu, ou si le balayage ASan+UBSan des TU n'est pas propre.

**Périmètre et forme**

- `FAILED:` si `tools/`, le `Makefile` ou un include autre que `#include "pico8_trade.h"` de `research.h` change ; si le commit touche un chemin hors de son « Empreinte » sans le lister comme TU suiveur.
- `FAILED:` si une grep de la Sortie ne rend pas ce qui y est écrit.
- `FAILED:` si une signature ou une fonction hors de la liste du contrôle 5 change ou disparaît ; si les membres de `pico8_trade_t`/`pico8_trade_keys_t`, le constructeur `trade`, `kg`, `jo:ib`, les cases de `lq`, les sources de bouton ou `pk`/`fp_is_bool`/`bg_is_bool` changent ; si une durée de vie change.
- `FAILED:` si `pico8_qe_equal` ne sépare pas nil de faux (table), si la garde `has_qe` de `dh` ou une garde `have_r_*` de `jo:cn` disparaît, ou si TROU-24 n'est plus reproduit à l'identique.
- `FAILED:` si un cast nouveau est ajouté, si des lignes comptées sont jointes, ou si une cale valeur ↔ typé apparaît (dans `src/` ou un TU).

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne baisse pas de **52 lignes / 68 occ.** exactement depuis le départ réel, sans explication chiffrée ligne à ligne ; si `type-tag` change de compte.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, cale valeur ↔ typé, (c) non nulle, (b) hors de `pico8_trade_qe`/`pico8_jy_qe`/`pico8_jo_ex`.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; les lignes qui gagnent une catégorie (« aucune » compris) ; les 5 lignes `ml` prises à B8 ; la justification de chaque Δlignes > Δoccurrences (attendu : aucune) ; les fonctions retirées et leurs lecteurs (raison `-Werror`) ; la preuve B2 (écritures et lecteurs, cart et port ; les deux sites où nil ≠ faux décide) ; le cas G4 de `lq` ; l'`include` ajouté ; le rapport `gcov` ; les mutations (M1 en tête) ; `class-coverage-net` ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne modifiée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` contient une ligne hors de `pico8_value.c` justifiée par `test_pico8_value`.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification ; **si M1 ne rougit pas**.
- `FAILED:` si le plafond est dépassé sans note (et sans repli), ou s'il manque un tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.17 », ou si elle omet : les comptes réels et l'écart à ≈ 720, les lectures, les 5 lignes `ml` pour D.18a, les en-têtes réduits à leur include pour D.19c.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`, ou si TROU-3/TROU-24 nomment encore `pico8_value_equal` ou un numéro de ligne.

## Out of Scope

- Les membres `has_qe`/`qe_is_bool`/`qe` de `pico8_trade_t` et de `pico8_trade_keys_t` (gardés par l'étude) ; la correction de TROU-24 (interdite par §7 : l'écart est préservé).
- B8–B11 : les cases de `lq` (dont `pico8_value_t *l` de `jo_ex`, `pico8_lq_add` de `hp:lq`, `jo_kw_tuple`), sources de bouton, `pi`, `s[i]`, `ou[]`, et les copies `truthy` de `layout.c`, `ui_button.c` : D.18. `pico8_value_tostr` de `pi.c`/`score_panel_display.c` : D.18c.
- Les plis de `test_pico8_trade` (`fold_qe` sur les membres, `fold_ref`, octets `PICO8_VALUE_*`), `ga`, `lq`, `nr`, `hy`, `mx`, `os`, `pm` : D.19a/b. Les includes de `trade.h`, `research.h`, `research_display.h`, `planet_trade.h`, `planet_display.h` et le module lui-même : D.19c.
- Les autres marqueurs `/* TROU-3 */` (`collect`, `tech_gate`, `planet.h:135`, `wormhole*.h`) : ils portent des comparaisons de chaînes.
- `bg_is_bool`, `pk`/`fp_is_bool`, `has_il` (note du plan) ; les `assert` d'étiquette (D.20).
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Découpage** : un commit ; repli D.17a (`qe` et ses lecteurs) / D.17b (`o`, `ml`, `planet_display`, docs de TROU-3).
  - **Types** : `pico8_qe_t` de l'étude, sans constructeurs (littéraux) ; `pico8_qe_equal` dans `trade.c` ; `o` en `const pico8_fixed_t *` ; `ml` en `pico8_fixed_t[2]` ; `assert` sur un `qe` vrai.
  - **Portée** : trois `static` retirées ; `ml` pris à B8 ; `nr` hors empreinte ; `economy_score.c` pour un marqueur ; TROU-3, TROU-24, glossaire réécrits.
  - **Tests** : quatre tests ajoutés à `trade` (TROU-24 dans `jo:cn`, dans `dh`, table, `qe` vrai), écrits d'abord contre l'arbre de départ ; `fold_qe` typé dans `hp` ; cas G4 de `lq` rendu nil.
  - **Drapeaux** : `class-coverage-net` (ligne du plan) ; balayage ASan+UBSan ; pas de `sanitize-net`.
- **Surprises de mesure, pour `plan-pilot`** : (1) **le cart n'écrit jamais `false` dans un `qe`** : le troisième état n'existe que par TROU-24, et l'écart nil/faux ne se voit qu'à deux comparaisons (`jo:cn` après `kg`, `dh`) ; (2) **aucun TU ni e2e n'atteint ces deux lectures** : sans les tests ajoutés, confondre nil et faux passerait tous les filets ; (3) la garde `has_qe` de `dh` est l'autre moitié de TROU-24 et portait un marqueur TROU-3 ; (4) le paragraphe d'exception de TROU-3 (et la ligne 630 du glossaire) décrivait `kw.qe` comme une chaîne partageant le pointeur de `r` : faux dans le port actuel (`ho` route `o`/`dm` en nombres) ; (5) `ml` de `jo_ex` doit être typé avec `o` : 5 lignes de B8 passent à D.17 ; (6) `nr` n'a rien à suivre ; (7) un `qe` vrai n'est écrit que par `test_pico8_trade:381`, pour `pico8_trade_eq` ; (8) `hp:jb` d'un besoin nil ne rend ni `""` ni `"[nil]"` à l'écran : TROU-13 n'y est pas en jeu ; (9) `research.h` doit inclure `pico8_trade.h` ; (10) arrivée ≈ 693 et non ≈ 720 : base 745, delta − 52 (et non − 45) ; (11) fonctions publiques à `pico8_value_t` 7 → 4, copies `truthy` 3 → 2, `pico8_value_equal` sans appelant dans `src/`.
- **Révision après la graduation de D.16b** (`529c71a`) : toutes les hypothèses tiennent sauf la position estimée de `pico8_jy_qe` dans `research.c` (H-D16b-3 : `:134-135`, et non ≈ `:110–:125`) ; numéros remesurés (`research.c`, `test_pico8_hp` `:449-451`, `:622`) ; départ, inventaire, deltas (− 52 / − 68), classification et découpage inchangés.
- **Pour D.18a** : `research_display.c` garde 10 lignes, toutes des cases (`l`, `pico8_lq_add`) ; B8 compte 5 lignes de moins que l'étude.
- **Pour D.19b** : `test_pico8_trade` garde `fold_qe` (membres), `fold_ref`, `same_qe` ; `hp` plie les `qe` en octets par `fold_qe`.
- **Pour D.19c** : `trade.h:7`, `research.h:9`, `research_display.h:9` n'ont plus que leur include ; `pico8_value_equal` n'a plus que `test_pico8_value`.

## Annexe A — textes recopiés (`docs/plan-3.md` à `529c71a`, par extraction de lignes)

### Lignes D.16b à D.18a (l. 287-289)

> - D.16b — genres mêlés B3–B7 (pointeur NULL pour nil/faux, chaînes, `hp_pose` en affectation par position, `rn`, `bg`, `pj_ed`), en deux sous-étapes b1/b2. `class-coverage-net` : oui. Compte : ≈ 765. Découvert à D.16b1 : un commit (`refactor(D.16b1)`, B3 + B5) ; `valeur` 825 / 97 (916 occ.) → **785 / 91 (874 occ.)**, − 40 lignes, − 42 occ., le delta du PRD exactement (≈ 765 visait une base de 845 : D.16a a fini à 825) ; `type-tag` 242 / 58 inchangé (`probe.c:211` modifiée sur place) ; résidu D 1065 / 107 → **1025 / 105** (`wormhole_display.c`, `audio.c` sortent). Lectures : `et` pointeur, NULL pour nil et `false`, `has_et` retiré de `pico8_probe_t` et de ses sept écritures ; `pico8_mz_et` retirée, `mz:kj` lit le membre ; le pont `probe.c:182` de D.15b part ; `has_et` était plié par onze TU, en « built » (avant tout `bv` : `et != NULL ? 1u : 0u`) ou après un `bv` qui le posait toujours (branche absente retirée) : aucune constante ne bouge ; le local `has_et` de `d_` garde son nom (grep 1 de la Sortie le rend, cinq lignes de `d_`) ; `pico8_rn(pico8_fixed_t)`, `js:ldown` l'appelle dans son bloc `has_my`. Copies `truthy` 7 → 5 (`layout`, `research`, `research_display`, `tech_gate`, `ui_button`). Pour D.19c : `audio.h` (qui fournit aussi `pico8_fixed_t` à `rn`), `wormhole.h`, `probe.h` n'ont plus que leur include. Pour D.20b : `probe.c:199-201` porte encore l'`assert` d'étiquette sur `et`. Découvert à D.16b2 : un commit (`refactor(D.16b2)`, B4 + B6 + B7 + `hp_pose`) ; `valeur` 785 / 91 (874 occ.) → **745 / 89 (831 occ.)**, − 40 lignes, − 43 occ., le delta du PRD exactement ; D.16b entière : 825 → 745 (− 80, − 85 occ.), et non ≈ 765 (base 825, non 845) ; `type-tag` 242 / 58 inchangé ; résidu D 1025 / 105 → **985 / 104** (`tech_gate.c` sort). Lectures : `int pico8_oj(const char *)` (NULL : 1), le pont `tech_gate.c:45` de D.15a part ; `const char *pico8_hp_il`, **`has_il` gardé** (`test_pico8_hp` plie nil ≠ `false`) ; le cas `pico8_oj` d'un nombre de `test_pico8_hp` retiré (G4) ; `hp_pose` par position (`switch (p)`, trois `assert` de genre gardés) ; `pico8_lq_bg` et `pico8_pj_ed` retirées ; **`bg_is_bool` gardé** par G1 (`test_pico8_lq` sépare `bg` faux de `bg` absent dans deux empreintes figées) : il rejoint `pk_is_bool` et `fp_is_bool` dans la forme « 1 ou faux » sans étape nommée ; les branches `bg` de `ga`/`hp` étaient mortes, devenues `assert(!lq->has_bg)`. Un test ajouté pour une mutation sans rouge de TU (seul l'e2e rougissait) : `cb` écarte une tech inventée du palier courant (`test_pico8_fm`). Copies `truthy` 5 → 3 (`layout`, `research_display`, `ui_button`). Pour D.17 : `research_display` garde sa copie (argument `o` de `jo_ex`). Pour D.19c : `tech_gate.h` n'a plus que son include (avec `audio.h`, `wormhole.h`, `probe.h`).
> - D.17 — genres mêlés B2 : `pico8_qe_t` et `pico8_qe_equal` (nil ≠ faux conservé). `class-coverage-net` : oui. Compte : ≈ 720.
> - D.18a — B8 : `pico8_lq_keys_t.items` en cases de l'union de `layout` ; §6 `layout_nil_item_draws_nothing` et `planet_gh_row_lq` réécrits dans leur lecture, preuve rouge. `sanitize-net` : oui. Compte : ≈ 560.

(Réextraites de `529c71a` : la ligne D.16b porte ses notes « Découvert à D.16b1 » et « Découvert à D.16b2 » ; les lignes D.17 et D.18a sont identiques à celles de `8533807`, H-D16b-12.)

### TROU-24 à `529c71a` (inchangé depuis `8533807`) (`docs/trous-de-portage.md:395-412`)

> Dans le cart, `trade:kg` recopie `self.qe` tel quel : nil reste nil. Ce port pose `false` quand la route allongée n'a pas de `qe`. Dans la struct `trade`, nil et absent ne font qu'un : un `qe` nil, passé explicitement ou omis, donne `false`.
> Non atteint par le filet : toute route construite par les rejeux a un `qe` non nil. `jo:ib` passe pourtant un `qe` nil quand la table `kw` de la planète n'a pas de `qe` (`src/pico8_planet_trade.c:242`), jusqu'à `nr:cn` → `kg`. Même effondrement nil/absent dans `dh` (`src/pico8_economy_tiers.c:302`) : une route au `qe` nil y compte comme une route sans `qe`.
> Fichier : `src/pico8_trade.c`.

### TROU-3, paragraphe d'exception à `529c71a` (inchangé)

> **Une exception, qui explique deux sites voisins qui ne se ressemblent pas.** Les valeurs que `ho()` route à travers `r` partagent littéralement le pointeur de `r` : là, la comparaison par identité est correcte. C'est le cas de `kw.qe` (routé depuis `hp.o`), d'où un `pico8_value_equal` dans `jo:gh` là où `db()` — qui compare une chaîne d'une autre provenance — doit passer par le contenu. Aucun des deux n'est à « corriger » d'après l'autre.

### §7, extrait (« En D et en 2 »)

> - **En D et en 2** : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée […], et à la sortie de phase.

## Annexe B — reproduire

```sh
grep -n 'qe' pico8/lua.lua                                        # écritures et lecteurs de qe
sed -n 478,506p pico8/lua.lua; sed -n 1293,1300p pico8/lua.lua     # jo:cn, jo:ib, dh
grep -rnE 'pico8_trade_qe|pico8_jy_qe|pico8_jo_ex\(|pico8_value_equal|economy_value_equal_content|jo_kw_qe' src tests
grep -rnE 'pico8_trade_new|pico8_trade_kg|qe_is_bool|pico8_jy_new' src tests   # écritures de qe
grep -rn 'pico8_jo_cn\|pico8_dh(' tests                          # TU qui atteignent cn/dh
grep -rn 'TROU-3\|TROU-24' src                                    # marqueurs
grep -nE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c     # 3 à 529c71a (7 à 8533807)
grep -nE 'pico8_value_t' src/*.h | grep -vE ':\s*(\*|/\*)' | grep -v 'src/pico8_value.h' \
  | grep -vE '^\S+:[0-9]+:\s+(const )?pico8_value_t \*?[a-z_]+;|^\S+:[0-9]+:\s+const pico8_lq_keys_t'  # 7 à 529c71a (13 à 8533807)
sed -n 12,15p src/pico8_map_tables.c                             # clés de iz : ni "" ni "[nil]"
```
