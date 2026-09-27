---
title: "Plan 3, étape 1b.10 : `lq` (la rangée mise en page) — six membres typés, `c`/`score` fixes avec `has_`, `bg` ({bool, fixe}) en lecture de 1b.5, `ra` en `pico8_vec_t *` emprunté, `fs` en `pico8_list_t *`, `w` fixe sans indicateur, et la table `offsetof` `g_lq_members[]` supprimée"
description: "`lq` a six membres `pico8_value_t` lus par nom. Aucun n'est écrit par nom : les écrivains sont la boucle de clés du constructeur et `pq`. Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 223 à 217. `bg` reprend telle quelle la lecture {fixe, bool} de 1b.5 : ce n'est pas un mélange nouveau. La table `offsetof` suit la forme de 1b.9. La boucle de clés est directe, sans table intermédiaire : une branche par clé (forme de 1b.5) appelle une aide `static` par genre (forme de 1b.9). Pas d'écrivain par nom, donc la moitié écrivain de R5 reste vacante. `jo.lq`, `js.lq` et `js.h_` gardent le même pointeur."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.10 — `lq` (`layout.h:26`), genres mêlés `bg`, cible de `jo.lq`/`js.lq`/`js.h_` ; `items` hors compte. Empreinte 27 (11, 16). Compte : 217.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié), le §6 (tests de régression) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 ». Il est cité ici, pas redécidé. S'appliquent aussi :

- « Découvert à 1b.3 » : le patron s'applique sans écart. R5 et R7 restent vacantes tant qu'aucune étape n'en a ;
- « à 1b.4 » : `ref:autre` vers un vecteur de `v(…)` donne un **`pico8_vec_t *` emprunté**, avec le même pointeur ;
- « à 1b.4b » : la table corrigée fait foi. Elle exige **`has_c`, `has_bg`, `has_score`** (« 1b.10 `lq.c`, `bg`, `score` ») ;
- « à 1b.5 » :
  - **lecture de R1 pour {fixe, bool}** : `pico8_fixed_t <m>` + `int has_<m>` + `int <m>_is_bool`, avec un **emballage public unique** `pico8_X_<m>` ;
  - **forme R5 de la boucle de clés** : une branche par clé, un `assert` « champ sans règle » hors genre, et l'`assert` de la clé inconnue gardé mot pour mot dans un `else` final ;
  - l'`assert` sur tag de `ref:liste` en boucle de clés ;
  - la preuve mécanique `grep -n 'pico8_value_t \*' … | grep -v 'pico8_value_t \*out_value)'` ;
- « à 1b.6 » : un fixe à nil `jamais` n'a pas d'indicateur (`w`) ;
- **conception de 1b.9** (PRD `docs/prd/triage/plan3-phase1b-etape9-hp-projet-de-recherche.md`, précondition ci-dessous) :
  - **forme R7 `offsetof`** : les entrées sont retirées, et la table vide est supprimée avec son accesseur. Les preuves `grep -rn 'offsetof(pico8_T_t' src/` et `grep -rnw '<accesseur>\|<table>' src/` doivent être vides ;
  - **aides R5 `static`, une par genre**, appelées une fois par membre. Clé absente ou `REF NULL` = nil. Un message « champ sans règle » par aide, qui nomme ses membres et leur ligne de recensement ;
  - pas d'emballage public pour un fixe à indicateur.

### État de départ

HEAD a10848c. **Précondition** : 1b.9 (`hp`) est commitée avant que 1b.10 commence. Elle est en cours d'implémentation. D'après son PRD, elle touche `research.[ch]`, `research_display.c`, `planet.c`, `economy_tiers.c`, `test_pico8_hp.c` et `test_pico8_entity_kind.c`. Elle ne touche aucun fichier modifié ici. Dans `research_display.c`, `pico8_hp_lq` n'écrit que `l->items` d'une `lq`. Les numéros de ligne cités ici, ceux de HEAD, ne devraient donc pas glisser. Chaque site est aussi désigné par son contenu. `make value-members` rend alors 223 (H1).

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H15).

### Lignes de recensement (`tools/member_census.txt:43-48`, régénéré par 1b.4b)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `c` | offset | `fixe` | observé | lu,stat | l |
| `bg` | offset | **`bool,fixe`** | observé | lu | l |
| `score` | offset | `fixe` | observé | lu | l |
| `ra` | offset | `ref:autre` | observé | lu,stat? | l |
| `fs` | offset | `ref:liste` | observé | stat | l |
| `w` | offset | `fixe` | jamais | - | l |

Six membres, donc 223 − 6 = **217**, comme le dit la ligne. `items` (`pico8_list_t`) est hors compte et hors étape. Aucun membre n'a la colonne `e` : `lq` n'est écrit par nom nulle part (voir R5). Les trois lignes qui pointent vers `lq` restent `pico8_value_t` :

- `pico8_jo_t lq` (`:79`, 1b.23c) ;
- `pico8_js_t lq` (`:235`, `le`, 1b.34) ;
- `pico8_js_t h_` (`:238`, `bool,ref:pico8_lq_t`, 1b.34).

**Ce recensement est exhaustif pour `lq`.** Les membres ne sont écrits qu'à deux endroits : la boucle de clés de `pico8_lq_struct_new`, et `pico8_lq_pq` (`fs`, `w`), appelé par ce même constructeur. Aucun `set_field` (descripteur à `NULL`), aucune méthode qui écrit, et aucune écriture directe ailleurs (grep `->(c|bg|score|ra|fs|w) =`, H3). L'observation de fin de constructeur voit donc chaque valeur qu'une `lq` tiendra jamais.

### Ce que fait le cart (`pico8/lua.lua`)

- `lq=kv:jr()` (`:289`) : la classe n'a pas de spec. `kv:jr` copie par `mw` **toutes** les clés propres des surcharges, puis appelle `lq:pq`.
- `lq:pq` (`:289-294`) écrit `self.fs=nv(self,…)` et `self.w=w-fs`. Un `fs` ou un `w` reçu (copie d'une `lq` par `js:pq`) est donc réécrit.
- `lq:mn` (`:300-312`) lit `self.c or 6`. `lq:ol` (`:295-299`) lit `self.w` et `self.fs`. `js:pq` (`:261`) lit `self.lq.w`. `js:kj` (`:273-275`) lit `c.w` et **`c.bg or 2`** sur `h_`.
- **Écrivains de `bg`.** L'unique écriture est `jo:hu`/`gh` (`:429`) : `{ne(self.kr).."$",bg=self.kr>=0 and 1}`. `bg` vaut donc `1` ou **`false`**. Le port fait de même (`planet_display.c:197-200` : `pico8_value_fixed(1)` ou `pico8_value_bool(0)`). `true` n'est écrit nulle part, mais la lecture de 1b.5 n'en a pas besoin (voir la décision R1).
- **Écrivains de `ra`.** `bz` (`:803-814`) : `ra=v(64,…)` sur les sept lignes à `score`. `pj:ph` (`:891`) les passe à `lq(st)`. La cible est un vecteur de `v(…)`, sans tag, d'où `ref:autre`.
- `c` vaut 13, 6, 3 ou 5, toujours un nombre (`:281`, `:419`, `:957`, `:961`, `bz`). `score` vaut de 0 à 6 (`bz`).

### Le code aujourd'hui

- `src/pico8_layout.h:20-32` : le commentaire du typedef (`items`, TROU-7), puis `{ header; pico8_list_t items; pico8_value_t c, bg, score, ra, fs, w; }`, un membre par ligne. Le commentaire de `pico8_lq_table_view` (`:57-62`) dit « `c`, `bg`, `score`, `ra` when not nil ».
- `src/pico8_layout.c` :
  - `pico8_lq_mn` (`:137-138`) : `pico8_layout_truthy(self->c) && self->c.kind == FIXED` → `self->c.as.fixed` ;
  - `pico8_lq_pq` (`:198-199`) : `self->fs = pico8_value_ref(fs_heap)`, `self->w = pico8_value_fixed(…)` ;
  - `pico8_lq_ol` (`:236-244`) : `self->w.kind == FIXED`, puis `self->fs.kind != REF || NULL` → `return`, puis `PICO8_REF_AS_LIST_CONST(self->fs.as.ref)` ;
  - **table d'offsets `g_lq_members[]`** (`:362-368`) : 6 entrées `{"m", offsetof(pico8_lq_t, m)}` ;
  - accesseur `lq_member` (`:370-378`), qui rend `pico8_value_t *` (forme `offsetof`), et `lq_is_nil` (`:380-382`) ;
  - lecteur `pico8_lq_field` (`:384-392`) : `lq_member`, nil → 0, sinon la valeur ;
  - descripteur (`:394-398`) : `OL` seul, `set_field` NULL ;
  - constructeur `pico8_lq_struct_new` (`:400-430`) :
    - `/* TROU-7 */`, puis l'en-tête ;
    - les 6 membres à `REF NULL` par `lq_member` (`:408-410`) ;
    - la **boucle de clés** (`:411-421`) : `member = NAME ? lq_member(…) : NULL`, puis `assert((INDEX || member != NULL) && "lq: every measured row passes positionals and c, bg, score, ra, fs, w; …")` et `*member = value` ;
    - `/* TROU-25 */ items = positional_values(overrides)`, `pq`, puis la couverture et `PICO8_MEMBER_CENSUS_SEE` ;
  - `pico8_lq_table_view` (`:432-454`) : `k = PICO8_REF_AS_LIST_CONST(self->fs.as.ref)->count`, puis les positionnels `1..k`, puis `c`/`bg`/`score`/`ra` par `lq_member` quand ils ne sont pas nil, puis `fs`, `w`, et les positionnels `k+1…`.
- `src/pico8_ui_button.c` :
  - `pico8_js_pq` (`:260-261`) : `new_lq->w.kind == FIXED` → `mb = w + 2` ;
  - `pico8_js_kj` (`:351`, `:355-356`) : `c->w`, et `c->bg.kind == FIXED` → `bg`, sinon 2 (`c.bg or 2`).
- **Lecteurs par nom, inchangés** :
  - `test_pico8_lq` (`lookup`, `get_ref` : `c`, `bg`, `score`, `ra`, `fs`, `w`) ;
  - `test_pico8_pj` (`w`, `c`) ;
  - `test_pico8_js` (`w`) ;
  - `test_pico8_planet_gh_row_lq` (`fs`, test de régression) ;
  - les vues `pico8_lq_table_view` de `test_pico8_ga` et `test_pico8_hp`.
- **Cibles REF.** `jo.lq` (`pico8_jo_hb`, `planet_display.c:118`), `js.lq` et `js.h_` (`pico8_js_pq`, `ui_button.c:257`, `:268`), l'`ix[2].hb` de `menu.c:358-360` (par `set_field(inst, "lq", …)`, l'écrivain de **`js`**), `jb.ls` et `pi.ld` tiennent tous `pico8_value_ref(<lq>)` ou une liste de REF. Rien ne libère une `lq` (TROU-7).

### Nil, membre par membre

- `c`, `score` : `fixe`, nil observé par une preuve **`lu` directe** (lignes sans `c`, lignes hors `bz`). Ce sont des non-pointeurs lus par nom, d'où **`has_c`** et **`has_score`** (R2, note 1b.4b). Un `score` de 0 est présent : c'est la ligne « current score » de `bz`.
- `bg` : `bool,fixe`, nil observé (`lu`). Voir la décision R1.
- `ra` : `ref:autre`. C'est un pointeur, NULL = nil.
- `fs` : `ref:liste`. C'est un pointeur, NULL = nil. `pq` le pose toujours, il n'est donc jamais NULL après le constructeur. Le nil observé (`stat`) vient du garde de `ol`.
- `w` : `fixe`, nil `jamais` : `pq` le pose toujours. Pas d'indicateur (R2, 1b.6).

### Empreinte remesurée

Le grep de la liste rend **27 (11, 16)**, la même valeur que la ligne :

- src : `entity.c`, `entity_geom.c`, `layout.c`, `menu.c`, `msgbox.c`, `particle.c`, `probe.c`, `slipway_display.c`, `ui.c`, `ui_button.c`, `wormhole_display.c` ;
- tests : `built_planet_panel_nil_row`, `empty_planet_cleared`, `ga_export_panel_by`, `ie`, `jc`, `je`, `jo`, `js`, `layout_nil_item_draws_nothing`, `lq`, `mo`, `nf`, `pj`, `planet_gh_row_lq`, `qb`, `rich_planet_exports`.

Ce grep **sur-compte largement**. `ra`, `c`, `w` et `fs` sont des noms courts partagés : `ra` est lu ou écrit par nom sur `mo`, `je`, `qc`, `js`, `jo`…, et `.c`/`.w` apparaissent partout. Les tests qui nomment `pico8_lq_t` n'y lisent que `items` (hors compte) ou passent par le nom. Personne hors de `src/` ne lit directement un des six membres.

**Fichiers modifiés : 4**, plus `docs/plan-3.md`.

- src (3) : `src/pico8_layout.h`, `src/pico8_layout.c`, `src/pico8_ui_button.c` ;
- tests (1) : `tests/test_pico8_lq.c`, la caractérisation, en pur ajout.

**Aucun test de `UNIT_TESTS_SLOW` n'est touché** (`ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`). `test_pico8_ie` et `test_pico8_jo` n'entrent dans l'empreinte que par l'homonyme `ra` (`set_field(e, "ra", …)` sur leur propre struct). Leur diff est vide. **Aucun test de régression (§6) n'est touché** (`built_planet_panel_nil_row`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`, `rich_planet_exports`) : ils ne lisent que `items` et `fs` par nom, et leur diff est vide (H14).

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** (un membre par ligne, ordre et noms gardés, `items` inchangé) :

   ```c
   pico8_list_t items;
   pico8_fixed_t c;      int has_c;
   pico8_fixed_t bg;     int has_bg;  int bg_is_bool;
   pico8_fixed_t score;  int has_score;
   pico8_vec_t *ra;
   pico8_list_t *fs;
   pico8_fixed_t w;
   ```

   La table `g_lq_members[]`, l'accesseur `lq_member` et `lq_is_nil` disparaissent. Un emballage public, `pico8_lq_bg`, est ajouté.
3. Effet mesurable : `make value-members` passe de 223 à **217** et ne liste plus aucune ligne `pico8_lq_t`. Aucun pixel ne bouge. `jo.lq`, `js.lq` et `js.h_` tiennent le même pointeur qu'avant.

**Pas de découpage** (G). L'empreinte de 27 est presque entièrement faite d'homonymes. Le diff réel tient dans un fichier réécrit (`layout.c` : lecteur, constructeur, `mn`, `pq`, `ol`, vue), quatre lignes de `ui_button.c` et une caractérisation. Il est plus petit que celui de 1b.9. La table `g_lq_members[]` et `lq_member` ne disparaissent qu'une fois toutes leurs entrées retirées. **Découpage de repli**, à ne prendre que si le premier passage dépasse le budget avant les fixes à indicateur :

- 1b.10a : `ra`, `fs`, `w`, compte **220**. `g_lq_members[]` est réduite à `c`, `bg`, `score` ;
- 1b.10b : `c`, `bg`, `score`, compte **217**. `g_lq_members[]`, `lq_member` et `lq_is_nil` sont supprimés.

L'implémenteur écrit alors ce découpage dans la liste du plan, dans le commit de 1b.10a.

## User Stories

1. En tant que lecteur de `layout.c`, je veux les six membres de `lq` typés, afin que `mn`, `pq`, `ol` et la vue ne déballent plus de `pico8_value_t` ni ne passent par `PICO8_REF_AS_LIST_CONST`.
2. En tant que lecteur de `ui_button.c`, je veux lire `w` et `bg` d'une rangée par leur type C dans `js:pq` et `js:kj`.
3. En tant que `test_pico8_lq`, je veux lire par nom chaque membre de chaque rangée construite avec le même genre, la même valeur et **le même pointeur** qu'en 1a.
4. En tant que `js:kj`, je veux qu'un `h_` dont `bg` vaut `false` soit peint en couleur 2, et un `h_` dont `bg` vaut 1 en couleur 1, comme en 1a.
5. En tant que lecteur par nom, je veux un `bg` `false` lu `BOOL` 0 présent, et non absent, et un `score` 0 lu `FIXED` 0 présent.
6. En tant que `js:pq`, qui recopie une rangée par sa vue en table, je veux que la copie ait les mêmes `c`, `bg` et `score`, **le même pointeur `ra`**, et un `fs` neuf.
7. En tant que `jo.lq`, `js.lq` et `js.h_`, je veux que la rangée pointée reste à la même adresse, passe l'`assert` de `lq:ol` et soit dessinée pareil.
8. En tant que `bare` (`pico8_lq_new(NULL)`), je veux exactement `fs` (liste vide) et `w` (0) présents, et les quatre autres absents.
9. En tant que `test_lq_is_a_struct`, je veux que l'écriture par nom (`write_w`) et une clé nommée inconnue avortent toujours, avec les mêmes messages.
10. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
11. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux, sans nouvelle allocation ni libération (TROU-7 préservé pour la rangée, `items` et `fs`, `ra` toujours emprunté).
12. En tant que `plan-pilot`, je veux `make value-members` à 217 et `make class-coverage-net` identique à la référence (`lq  10718  15/15`).
13. En tant qu'auteur de 1b.23c (`jo.lq`) et de 1b.34 (`js.lq`, `js.h_` en `bool,ref:pico8_lq_t`), je veux trouver `pico8_lq_t` complètement typé quand je type le pointeur qui le vise.
14. En tant qu'auteur de 1b.13 (`jc_too_far.fo`), je veux savoir que 1b.10 **ne** fixe **pas** la moitié écrivain de R5.
15. En tant qu'auteur des étapes à `ref:autre` en boucle de clés (1b.19 `qb.v`/`ma`, 1b.23b `jo.ra`, 1b.17 `nf.ra`…), je veux savoir quel `assert` de tag s'y applique.
16. En tant que vérificateur, je veux une caractérisation par nom lancée sur l'ancien code, les mutations du §7 déclinées sur `lq`, la justification de chaque trou, et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline`, et marqués (G).

### Décision R1 pour `bg` : lecture {fixe, bool} de 1b.5, appliquée sans écart

La ligne est `bool,fixe`. C'est exactement le mélange de `trade.qe` (1b.5), **pas un mélange nouveau** : aucune lecture nouvelle de R1 n'est introduite. (G) La forme de 1b.5 est reprise telle quelle :

```c
pico8_fixed_t bg;
int has_bg;     /* 0: `bg` is nil, and `bg` is then never read */
int bg_is_bool; /* 1: `bg` holds a boolean (0/1), 0: a fixed; read only when `has_bg` */
```

| état 1a | `has_bg` | `bg_is_bool` | `bg` | lu par nom |
|---|---|---|---|---|
| nil (clé absente ou `REF NULL`) | 0 | 0 | 0 (non lu) | absent, `*out_value` intact |
| `FIXED` n | 1 | 0 | n | `FIXED` n |
| `BOOL` b | 1 | 1 | b (0/1) | `BOOL` b |

- **Pourquoi pas la lecture {bool, chaine} de 1b.7**, alors que le seul booléen écrit est `false` : l'autre genre est un fixe, pas un pointeur. Un fixe n'a pas de NULL qui pourrait porter `false`. Un indicateur `bg_is_bool` coûte moins qu'une sentinelle (interdite par la puce **Nil**). La lecture de 1b.5 couvre aussi `true`. Rien n'est donc à prouver sur `true`, et la décision ne dépend pas de la ligne `:429` du cart.
- **Emballage** : `pico8_value_t pico8_lq_bg(const pico8_lq_t *self)`, déclaré dans `layout.h`. Il rend `REF NULL` si `self` est NULL ou si `has_bg` vaut 0, `pico8_value_bool(bg)` si `bg_is_bool`, `pico8_value_fixed(bg)` sinon. Son contrat tient en une phrase, au format de `pico8_trade_qe`. **Ses appelants** sont le lecteur (R4) et `pico8_lq_table_view`, une lecture directe qui garde la valeur de 1a pour la recopie. Le `bg` d'une copie reste ainsi `BOOL` 0 quand la source l'avait.
- `pico8_js_kj` n'a pas besoin de la valeur de 1a. (G) Il lit les indicateurs : `if (c->has_bg && !c->bg_is_bool) bg = pico8_ui_to_int(c->bg);`. C'est `c.bg or 2` : `false` et nil donnent 2.

### Les autres membres

- `c`, `score` → `pico8_fixed_t` + `has_c`, `has_score` (R2). Pas d'emballage public (1b.9).
- `ra` → `pico8_vec_t *` (lecture de 1b.4), **le pointeur de la REF reçue**, jamais copié ni libéré. Il appartient au vecteur de `bz` (`pj:ph`), à celui d'un test, ou à la source d'une copie (vue en table). La copie **partage** donc le `ra` de sa source, comme le `mw` du cart.
- `fs` → `pico8_list_t *` (`ref:liste`), la liste que `pq` alloue (TROU-7). Un `fs` reçu par clé (copie) est déballé puis réécrit par `pq`, comme dans le cart.
- `w` → `pico8_fixed_t`, sans indicateur. `pq` le pose toujours.

### Cibles REF : `jo.lq`, `js.lq`, `js.h_` inchangés

Le typage ne touche ni l'allocation ni l'adresse d'une `lq` : un `malloc(sizeof *self)` par construction, jamais libéré. `sizeof(pico8_lq_t)` change, mais aucun code ne copie une `lq` par valeur (grep `pico8_lq_t <nom> =` sans `*` : vide). Les écrivains et lecteurs des trois cibles ne changent pas et restent `pico8_value_t` jusqu'à 1b.23c et 1b.34 : `pico8_jo_hb`, `pico8_js_pq`, `menu.c:360`, l'`assert` « jo.lq must be the row jo.ex built », l'`assert` de `lq:ol` (message inchangé) et `pico8_js_is_lq`. La caractérisation (cas 4) le vérifie.

### R1–R9 appliquées à `lq` (patron de 1b.1, cité)

| règle | sur `lq` |
|---|---|
| R1 | Voir la Solution. `c`, `score`, `w` → `pico8_fixed_t`. `bg` : lecture 1b.5 ci-dessus. `ra` → `pico8_vec_t *` (1b.4). `fs` → `pico8_list_t *`. Noms et ordre gardés, un membre par ligne, `items` en tête inchangé. |
| R2 | `has_c`, `has_bg` (+ `bg_is_bool`), `has_score`, chacun juste après son membre. Chaque preuve est `lu` directe. Pas d'indicateur pour `ra`/`fs` (NULL = nil) ni pour `w` (nil `jamais`). |
| R3 | Le bloc d'ouverture (`:408-410`) garde sa place, avant la boucle de clés. La boucle `lq_member` devient des affectations typées : fixes et indicateurs à 0, `ra` et `fs` à NULL, `w` à 0. `pq` pose ensuite `self->fs = fs_heap` et `self->w = pico8_fixed_sub(ctx.w, ctx.fs)` (R6). L'ordre est gardé : ouverture, clés, `/* TROU-25 */ items`, `pq`, couverture, recensement. `/* TROU-7 */` est gardé. |
| R4 | Le lecteur a une branche `strcmp` par membre, dans l'ordre du typedef, avant le `return 0` final. `c`, `score` : `has_<m>` = 0 → `return 0`, sinon `pico8_value_fixed`. `bg` : `v = pico8_lq_bg(self)`, `return 0` si `v` est `REF NULL`. `ra`, `fs` : NULL → `return 0`, sinon `pico8_value_ref(ptr)`, même pointeur. `w` : toujours `pico8_value_fixed(w)`. Nom inconnu (`items`, `ol`, `has_c`…) → `return 0`. Dans tous les `return 0`, `*out_value` reste intact. |
| R5 | **Écrivain : vacant** (voir plus bas). **Boucle de clés : exercée, directe** (forme fixée ici, voir plus bas). |
| R6 | Lectures directes, désignées par le compilateur (voir le tableau plus bas). Les seules écritures hors de la boucle de clés sont celles de `pq`. |
| R7 | **Forme `offsetof` de 1b.9, appliquée sans écart**, voir plus bas. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_lq_t)` est gardé. `tools/member_census.txt` n'est pas édité. |
| R9 | 223 → 217, aucune ligne `pico8_lq_t`. Les trois `has_`, `bg_is_bool` et `items` ne comptent pas. |

### R5, moitié boucle de clés : branche par clé, aide par genre (forme fixée ici)

La boucle de clés de `lq` est **directe** : les clés vont de `overrides` aux membres, sans table intermédiaire, comme pour `mh` et `trade` et à la différence de `hp`. (G) La forme combine les deux précédents :

- **une branche par clé** (forme de 1b.5), dans l'ordre du typedef : `key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "<m>") == 0` ;
- **chaque branche appelle une aide `static` par genre** (forme de 1b.9), qui prend la `pico8_value_t` de la clé (pas de nom à chercher, puisqu'il n'y a pas de table intermédiaire). Il y a cinq aides pour six membres :
  - `lq_take_fixed(value, &self->c, &self->has_c)`, et de même pour `score` ;
  - `lq_take_bg(value, self)` : nil → `has_bg = 0`, `FIXED` → `bg_is_bool = 0`, `BOOL` → `bg_is_bool = 1`. C'est le corps de la branche `qe` de `trade` ;
  - `lq_take_vec(value, &self->ra)` ;
  - `lq_take_list(value, &self->fs)` ;
  - `lq_take_w(value, &self->w)`.
- **Chaque aide applique R5 de 1b.1** :
  - `REF NULL` → nil, soit pointeur NULL, soit `has_<m>` = 0. **Pour `w`, sans indicateur : `assert`** (R5 : « `assert` sur un membre sans indicateur ») ;
  - genre de la ligne → `<m> = value.as.<champ>`, et `has_<m> = 1` s'il existe ;
  - `ref:liste` : l'`assert` sur tag de 1b.5, `pico8_type_tag_of(ref) == PICO8_TYPE_TAG_LIST` ;
  - **`ref:autre` (lecture de R5, première en boucle de clés)** : une REF non NULL **dont `pico8_type_tag_of` rend `PICO8_TYPE_TAG_UNKNOWN`**. C'est exactement la définition de `ref:autre` dans l'en-tête du recensement. Une liste, une table ou une struct taguée à cette place est hors de la ligne ;
  - tout autre genre → `assert` « champ sans règle ». Son message nomme les membres de l'aide et leur ligne de recensement, au format de `trade`/`hp`. Exemple : `"lq.c/score: a key neither nil nor fixed is off its census row (fixe) -- a field under no rule of plan-3 section 5: stop, do not widen the member"`.
- **`else` final** : une clé positionnelle passe (elle va à `items`, TROU-25). Une clé nommée inconnue avorte par `assert(key.kind == PICO8_OB_KEY_INDEX && "<message actuel, mot pour mot>")`. Le message `"lq: every measured row passes positionals and c, bg, score, ra, fs, w; a named key other than c/bg/score/ra/fs/w would be a field under no rule of plan-3 section 5 -- stop, do not add a member"` ne change pas.
- `fs` et `w` reçus (copie par `js:pq`) sont déballés **puis** réécrits par `pq`. Ce n'est pas du code mort : c'est le `mw` puis `pq` du cart, et la branche est exécutée par chaque copie (H4).

### R5, moitié écrivain : toujours vacante

`lq` n'a **pas** d'écrivain par nom. `g_lq_struct_class.set_field` vaut NULL (`layout.c:397`), le recensement dit `l` sans `e` pour les six membres, et `test_lq_is_a_struct` le fige (`cls->set_field == NULL`, `must_abort(write_w)`). L'écriture `set_field(inst, "lq", …)` de `menu.c:360` écrit `js.lq`. Elle relève de 1b.34, pas de `lq`. (G) En ajouter un pour « exercer R5 » serait un écrivain non mesuré : c'est un critère d'échec. **Première candidate : 1b.13** (`jc_too_far.fo`), comme l'écrivent 1b.8 et 1b.9.

### R7, forme `offsetof` : lecture de 1b.9, sans écart

- Les six entrées de `g_lq_members[]` sont retirées. `lq` n'a pas de fille, donc aucune `offsetof(pico8_F_t, parent.m)` n'est à toucher. **La table vide est supprimée avec son accesseur `lq_member`**, et avec `lq_is_nil`, qui n'a plus d'appelant (`-Werror` sur une `static` inutilisée).
- Les quatre appelants de `lq_member` prennent une forme typée : branches R4 du lecteur, affectations R3 du bloc d'ouverture, branches R5 de la boucle de clés, et lectures R6 de `pico8_lq_table_view`.
- La liste `named[]` de `pico8_lq_table_view` (`:433`) est remplacée par quatre blocs en ligne, dans le même ordre, parce que chaque membre a désormais son propre genre : `has_c` → `set_name(view, "c", fixed(c))` ; `bg` = `pico8_lq_bg(self)` s'il n'est pas `REF NULL` ; `has_score` ; `ra != NULL` → `ref(ra)`. Puis `fs` → `pico8_value_ref(self->fs)` et `w` → `pico8_value_fixed(self->w)`. L'ordre des clés de la vue est inchangé (`test_lq_is_a_struct` le fige).
- **Preuves**, toutes vides après l'étape :
  - `grep -rn 'offsetof(pico8_lq_t' src/` ;
  - `grep -rnw 'lq_member\|g_lq_members\|lq_is_nil' src/` ;
  - `grep -nE '(->|\.)(c|bg|score|ra|fs|w)\.(kind|as)' src/pico8_layout.c`, qui rend aujourd'hui 7 lignes, toutes sur une `lq` ;
  - `grep -nE '(new_lq|c)->(w|bg)\.(kind|as)' src/pico8_ui_button.c`, qui rend aujourd'hui 5 lignes. La restriction aux variables `new_lq`/`c` évite l'homonyme `self->ra` de `js` (`:289-290`, 1b.34) ;
  - `grep -n 'pico8_value_t \*' src/pico8_layout.c | grep -v 'pico8_value_t \*out_value)'`, qui rend aujourd'hui 4 lignes, toutes `lq_member`.

### R6 : lectures directes hors du constructeur (le compilateur les désigne)

| site | aujourd'hui | après |
|---|---|---|
| `layout.c` `mn` (`:137-138`) | `truthy(self->c) && self->c.kind == FIXED` → `self->c.as.fixed` | `self != NULL && self->has_c` → `self->c`. Un fixe présent est vrai en Lua |
| `layout.c` `pq` (`:198-199`) | `self->fs = pico8_value_ref(fs_heap)` ; `self->w = pico8_value_fixed(…)` | `self->fs = fs_heap` ; `self->w = pico8_fixed_sub(ctx.w, ctx.fs)` |
| `layout.c` `ol` (`:236-244`) | `self->w.kind == FIXED` → `w` ; `fs` non REF ou NULL → `return` ; `PICO8_REF_AS_LIST_CONST(self->fs.as.ref)` | `w = self->w` ; `self->fs == NULL` → `return` (garde gardé) ; `pico8_list_all(self->fs)` |
| `layout.c` `table_view` (`:435-449`) | `PICO8_REF_AS_LIST_CONST(self->fs.as.ref)->count` ; `named[]` par `lq_member` ; `self->fs`, `self->w` | `self->fs->count` ; blocs en ligne (R7) ; `pico8_value_ref(self->fs)`, `pico8_value_fixed(self->w)` |
| `ui_button.c` `js_pq` (`:260-261`) | `new_lq->w.kind == FIXED` → `mb = w + 2` | `mb = pico8_fixed_add(new_lq->w, mb)`, sans condition (`w` toujours posé) |
| `ui_button.c` `js_kj` (`:351`) | `c->w.kind == FIXED ? c->w.as.fixed : 0` | `c->w` |
| `ui_button.c` `js_kj` (`:355-356`) | `c->bg.kind == FIXED` → `pico8_ui_to_int(c->bg.as.fixed)` | `c->has_bg && !c->bg_is_bool` → `pico8_ui_to_int(c->bg)` |

- `#include "pico8_ref_audit.h"` est retiré de `layout.c`, qui n'en a plus d'usage (grep avant de retirer). Il reste dans `ui_button.c` (`:41`, `h_` en liste).
- Aucun site par nom ne change, et aucun n'est remplacé par un accès direct.
- **Commentaires** (règle de `CLAUDE.md`) :
  - les contrats de `has_c`, `has_bg`, `bg_is_bool`, `has_score` s'écrivent au format de `trade.h` ;
  - le commentaire du typedef gagne une phrase de contrat : « `ra` is the vector the overrides held, borrowed, never copied » ;
  - `pico8_lq_bg` a un contrat d'une phrase, au format de `pico8_trade_qe` ;
  - le commentaire de `pico8_lq_table_view` (« when not nil »), `/* TROU-7 */` et `/* TROU-25 */` restent exacts ;
  - aucun autre commentaire.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 223 sur l'arbre de départ (1b.9 commitée), puis 217.
- **H2** : aucun `pico8_entity_set_field` n'atteint une `lq`, hors `must_abort(write_w)` (recensement `l` sans `e`).
- **H3** : aucun membre de `lq` n'est écrit hors de la boucle de clés et de `pq`. Grep `->(c|bg|score|ra|fs|w) =` sur `layout.c`, `ui_button.c` et `planet_display.c` : seules les lignes de `pq` sur `lq`, les autres sont des homonymes sur `js`/`jo`.
- **H4** : dans les 15 rejeux et les TU, les clés nommées reçues sont :
  - `c`, `score` : `FIXED` ;
  - `bg` : `FIXED` ou `BOOL` 0 ;
  - `ra` : une REF de tag `UNKNOWN` (vecteur) ;
  - `fs` : une REF de tag `LIST` ;
  - `w` : `FIXED`, jamais nil ;
  - aucune `REF NULL`, sauf celles que la caractérisation passe.

  Chaque copie par `js:pq` (vue en table) passe `fs` et `w`. Sinon, une aide R5 avorte : c'est l'arrêt « champ sans règle ».
- **H5** : au moins une rangée du rejeu `BUILT`, en dernière frame, a `bg` présent. Au moins un `h_` porte `bg` = `false` (planète à `kr` < 0) ; sinon `BG_FALSE_COUNT` vaut 0, et l'état `false` n'est figé que par les constructions de la caractérisation et `HP_LQ_HASH` (déjà `bool(0)`).
- **H6** : les empreintes et valeurs figées ne bougent pas : `CONSTRUCTIONS_HASH`, `JO_EX_HASH`, `HP_LQ_HASH`, `CALLS_HASH`, `REPLAY_HASH` (`test_pico8_lq`), et celles de `test_pico8_js`, `test_pico8_pj`, `test_pico8_ga` et `test_pico8_hp` qui lisent une rangée par nom ou par sa vue.
- **H7** : `test_pico8_lq` (dans `UNIT_TESTS`, hors `UNIT_TESTS_SLOW`) reste sous 30 s. La caractérisation rejoue au plus une fois `BUILT` de plus (cas 4). Sa durée n'est pas mesurée ici.
- **H8** : `make class-coverage-net` rend `lq  10718  15/15`, identique à la référence.
- **H9** : `make float-audit` retrouve ses sites C1 de `layout.c` (`pico8_lq_ol_item_draw | item->color`, `item->sprite_id`) inchangés. `js_kj` passe par `pico8_ui_to_int`, dont l'extrait (`x`) ne change pas.
- **H10** : le build `-DPICO8_MEMBER_CENSUS` compile encore sur un `pico8_lq_t` sans `pico8_value_t`. Il n'est pas lancé.
- **H11** : en dernière frame du rejeu `BUILT`, `hv` contient au moins une entité dont `lq`, lu par nom, est une rangée (`jo.lq` ou `js.lq`), et `ok.le` au moins un bouton à `h_`. Sinon, le cas 4 se réduit à ce qui est atteint, et c'est noté dans le commit.
- **H12** : `pico8_type_tag_of` rend `UNKNOWN` sur un vecteur de `v(…)` (`bz`) et sur les `vec_ref` des tests (hypothèse H4 de 1b.4).
- **H13** : sous ASan+UBSan, rien de nouveau (`detect_leaks=0`, `fs` et la rangée toujours perdus, `ra` emprunté).
- **H14** : les cinq tests de régression de `lq` compilent et passent sans diff : ils ne lisent que `items`, et `fs` par nom.
- **H15** : 1b.9, commitée, ne touche aucun des quatre fichiers de cette étape. Les lignes citées ici restent justes à ±0.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne fait aucun accès `->m` à l'un des six membres (`items` reste lu par `row_count`/`row_item`, comme aujourd'hui). Elle lit par le **lecteur brut** (`pico8_lq_struct_class()->field`, avec un témoin) et par `pico8_entity_get_field`/`get_ref`. Elle compile sur les deux formes.
- **Caractérisation, figée sur l'ancien code** : `test_lq_members_by_name`, dans `test_pico8_lq`, lancée par `in_fresh_child` après `test_lq_is_a_struct`. Pas de nouveau TU. Le témoin est un fixe `0x1234` posé dans `out` avant chaque lecture. Les noms lus sont les six membres, plus les intrus `""`, `"x"`, `"lq"`, `"items"`, `"ol"`, `"has_c"`, `"has_bg"`, `"bg_is_bool"`.
  1. **`bare`** (`pico8_lq_new(NULL)`) :
     - `fs` : lecteur à 1, `REF` de tag `LIST`, `count` 0 ;
     - `w` : lecteur à 1, `FIXED` 0 ;
     - `c`, `bg`, `score`, `ra` et chaque intrus (`"ol"` compris : c'est une méthode, pas un membre) : lecteur à 0, `out` égal au témoin.
  2. **États par membre**, sur des rangées construites à la main :
     - `c` = 9 ; `score` = **0** (présent 0) ;
     - `bg` = 8, `bg` = `BOOL` 0 (présent faux), `bg` absent ;
     - `ra` = `vec_ref(…)` ;
     - une rangée **« clés nil »** qui passe `c`, `bg`, `score`, `ra` et `fs` à `REF NULL`. Sur l'ancien code, ces membres sont lus absents, sauf `fs`, réécrit par `pq`. Pour chaque rangée et chaque membre, le lecteur brut rend 1 avec un genre de la ligne, ou 0 avec `out` intact. Il ne rend **jamais** 1 avec `REF NULL`. On plie `(rangée, membre, retour, genre, valeur fixe ou booléenne)` dans une empreinte nouvelle, **`RAW_PRESENCE_HASH`**.

     Puis, sur le rejeu `BUILT` en dernière frame, les mêmes lectures sur chaque rangée atteinte (cas 4), pliées dans **`REPLAY_PRESENCE_HASH`**, avec les comptes **`BG_FIXED_COUNT`**, **`BG_FALSE_COUNT`** (H5) et **`BG_ABSENT_COUNT`**. Les cinq valeurs sont relevées sur l'arbre de départ et écrites en `#define` avant le typage.
  3. **Identité des pointeurs** :
     - le `ra` lu par nom est, **par adresse**, la REF passée ;
     - deux lectures successives de `fs` rendent le même pointeur ;
     - une copie par `pico8_js_new(spec lq = rangée)` a un `ra` **à la même adresse** que celui de la source, un `fs` différent, et les mêmes `c`, `bg` (genre compris, `BOOL` 0 gardé) et `score`. La même chose vaut pour `pico8_lq_new(pico8_lq_table_view(rangée))`.
  4. **Cibles REF**, sur le rejeu `BUILT` en dernière frame (H11) :
     - pour chaque entité de `hv` dont `lq`, lu par nom, est une struct de descripteur `pico8_lq_struct_class()` (`jo.lq`, `js.lq`), et pour chaque `h_` d'un bouton de `ok.le` : deux lectures par nom rendent la même adresse ;
     - `pico8_mv(rangée, "ol", …)` réussit ;
     - la rangée entre dans `REPLAY_PRESENCE_HASH`.

  Elle est lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite. Le diff de `test_pico8_lq.c` est un pur ajout.
- **Tests existants qui figent déjà beaucoup**, inchangés :
  - `test_constructions` (dont `bg` 8, `bg false`, `score` 0 et 3, `ra`) ;
  - `test_jo_ex` (`c` présent si `jq > 5`) ;
  - `test_hp_lq_and_copy` (copie par `js:pq`, deux `h_` à `bg` nombre et `false`, pixels de `js:kj`) ;
  - `test_calls` ;
  - `test_replay` ;
  - `test_lq_is_a_struct` (ordre des clés de la vue, `must_abort`).
- **Tests de régression (§6)** : aucun n'est touché, leurs diffs sont vides (H14). Il n'y a pas de preuve rouge à refaire.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune lancée par `make tmp/test_pico8_lq && ./tmp/test_pico8_lq`. Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : la branche `c` du lecteur ignore `has_c`. Attendu rouge :
     - caractérisation 1 et 2 ;
     - `test_jo_ex` (`lookup(lq,"c") == (jqs > 5)`) ;
     - `test_lq_is_a_struct` ;
     - `CONSTRUCTIONS_HASH`.
  2. (§7-2) *« présent 0 » là où c'était absent* : `pico8_lq_bg` rend `pico8_value_fixed(0)` quand `has_bg` vaut 0. Attendu rouge :
     - caractérisation 1 et 2 ;
     - `test_lq_is_a_struct` (`!lookup(lq,"bg")`, et `view->count == row_count + 2` sur la rangée de `hp:lq`, qui gagnerait `bg`) ;
     - `CONSTRUCTIONS_HASH`.

     Les pixels de `js:kj` ne bougent pas, puisque ce site lit les indicateurs.
  3. (§7-3) *boucle d'init qui saute un membre typé* : la branche `score` de la boucle de clés garde son test de clé mais n'appelle plus son aide, et `has_score` reste à 0. Retirer la branche entière ne conviendrait pas : la clé tomberait dans l'`else` final et avorterait. Attendu rouge : caractérisation 2, `CONSTRUCTIONS_HASH` (rangées `hole`, `disorder`), `test_pico8_pj` si ses rangées de `bz` lisent `score`. **Variante non retenue** : sauter `has_score = 0` dans le bloc d'ouverture laisse un octet de `malloc` indéterminé, donc un résultat non déterministe. C'est justifié dans le commit.
  4. Propre à l'étape, *faux devenu nil* : `pico8_lq_bg` rend `REF NULL` quand `bg_is_bool`. Les pixels de `js:kj` ne bougent pas : `false` et nil y donnent tous deux 2. Attendu rouge :
     - caractérisation 2 (`BG_FALSE_COUNT` si H5 tient, `RAW_PRESENCE_HASH`) et 3 (copie) ;
     - `CONSTRUCTIONS_HASH` (rangée `bg false`) ;
     - `test_lq_is_a_struct` (`view->count == 7`).
  5. Propre à l'étape, *`false` lu comme fixe 0 dans `js:kj`* : la condition devient `c->has_bg` seul. La couleur 0 remplace la couleur 2. Attendu rouge : `HP_LQ_HASH` (bouton à `h_` `false`), et `REPLAY_HASH`/`make e2e` si H5 tient. Cette mutation prouve la seule lecture directe de `bg`.
  6. Propre à l'étape, *vecteur copié au lieu d'emprunté* : `lq_take_vec` alloue une copie de `*ra`. Les empreintes par valeur ne bougent pas. Attendu rouge : **caractérisation 3 seule**. C'est ce qui justifie ce cas.
  - **Trou justifié** : les `assert` « champ sans règle » des cinq aides, dont celui de `w` nil, ne sont atteints par aucune entrée (H4). Un `must_abort` qui les viserait (par exemple `c` = `BOOL` 1, ou `w` = `REF NULL`) serait rouge sur l'ancien code, qui accepte ces clés. Il ne peut donc pas faire partie de la caractérisation figée. (G) Ce PRD n'en ajoute pas, comme 1b.5, 1b.7 et 1b.9. Le commit note la mutation « `assert` retiré de `lq_take_vec` » et l'absence de rouge.
- **`gcov`** : `test_pico8_lq` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, puis `gcov` sur `pico8_layout.c` et `pico8_ui_button.c`. Chaque ligne ajoutée ou modifiée doit être exécutée :
  - les six branches du lecteur, présent comme absent ;
  - les cinq aides, avec leur branche nil (rangée « clés nil ») et chaque genre accepté ;
  - le bloc d'ouverture ;
  - l'`else` final sur une clé positionnelle ;
  - `mn`, `pq`, `ol`, la vue (les quatre blocs, présent et absent) ;
  - `js_pq` et les deux branches de `bg` dans `js_kj`.

  Trous attendus et justifiés :
  - la branche `self == NULL` de `pico8_lq_bg` : ses deux appelants ne passent jamais NULL, et la caractérisation, qui doit compiler sur l'ancien code, ne peut pas l'appeler ;
  - la branche `fs == NULL` du lecteur et le `return` sur `fs == NULL` de `ol` : inatteignables, puisque `pq` alloue toujours `fs` ;
  - la branche d'échec des `assert` R5 (voir le trou ci-dessus).

  Aucun autre trou n'est attendu. Le rapport va dans le commit.
- **Prior art** :
  - `test_lq_is_a_struct` et `test_constructions` (1a.34) ;
  - `test_trade_members_by_name` (1b.5 : lecteur brut, témoin `0x1234`, {fixe, bool}) ;
  - `test_hy_members_by_name` (1b.7) ;
  - `test_hp_members_by_name` (1b.9 : identité par adresse, comptes par état).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make sanitize-net` : ~4 min seul, le double sous charge ; on ne le lance **jamais** en même temps que `e2e` ;
- `make class-coverage-net` : 3 à 4 min ;
- `make value-members` : quelques secondes.

Caractérisation, mutations et `gcov` passent par un binaire seul (`make tmp/test_pico8_lq`), jamais par `make test`. Il y a 6 mutations, à quelques secondes chacune (H7).

**TU de `UNIT_TESTS_SLOW` touchés : aucun.** Rien à compiler en avance. Les TU lents lient `layout.c` et `ui_button.c`, mais aucun ne nomme un membre de `lq` : une erreur de compilation n'y est pas attendue. `make e2e` les construit et les lance.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` sans erreur de liste C1, et les `UNIT_TESTS_SLOW` qui passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 217`**, sans aucune ligne `pico8_lq_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**. Le diff touche le corps du constructeur `pico8_lq_struct_new` (R3, R5), même si `pico8_class_coverage_record("lq")` et `KNOWN[]` ne bougent pas. C'est la même lecture du §7 qu'en 1b.5, 1b.7 et 1b.9. La sortie doit être identique à `tools/class_coverage_net_expected.txt` (`lq  10718  15/15`).

`make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si :
  - une aide R5 avorte dans un rejeu ou un TU (H4) ;
  - un membre est ajouté à `lq`, sauf `has_c`, `has_bg`, `bg_is_bool` et `has_score` ;
  - un `set_field`, une méthode de descripteur ou une clé de construction acceptée est ajouté ;
  - le lecteur accepte un autre nom que les six membres.
- `FAILED:` si un type diffère de la Solution, si un indicateur n'est pas un `int` déclaré juste après son membre (`bg_is_bool` après `has_bg`), si `w`, `ra` ou `fs` ont un indicateur, ou si `items` change.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un membre nil ou un nom inconnu, ne rend pas le même pointeur pour `ra` et `fs`, lit absent un `bg` `false` ou un `score` 0, ou lit un `bg` `false` autrement qu'en `BOOL` 0.
- `FAILED:` si `g_lq_members[]`, `lq_member` ou `lq_is_nil` survivent, si `pico8_lq_bg` manque à `layout.h`, ou si une preuve de R7 n'est pas vide.
- `FAILED:` si `ra` est copié, si une libération est ajoutée (rangée, `items`, `fs`, `ra` : TROU-7), si l'allocation d'une `lq` change, ou si un autre écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si `jo.lq`, `js.lq`, `js.h_`, leurs écrivains (`pico8_jo_hb`, `pico8_js_pq`, `menu.c` `ix[2].hb`) ou leurs lecteurs changent autrement que par les lignes R6 de `js_pq`/`js_kj` ci-dessus, ou si `pico8_js_is_lq` change.
- `FAILED:` si le message d'un `assert` existant change (`lq: every measured row passes positionals and c, bg, score, ra, fs, w; …`, `lq:ol: every row reaching it …`, `jo.lq must be the row jo.ex built`).
- `FAILED:` si la signature de `pico8_lq_new`, `pico8_lq_ol`, `pico8_lq_ol_default`, `pico8_lq_table_view`, `pico8_lq_default` ou `pico8_lq_struct_class` change.
- `FAILED:` si un extrait C1 de `tools/float_audit_c1.txt` ou ce fichier changent.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation à `test_pico8_lq.c` (et ses `#define`). Également si une lecture par nom change, ou si une valeur figée existante change (`*_HASH` de `test_pico8_lq`, empreintes des autres TU).
- `FAILED:` si un test de régression de bug (§6) a un diff non vide.
- `FAILED:` si un site par nom de `src/` change ou est remplacé par un accès direct.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 217 (ou 220 puis 217 en découpage de repli), liste encore `pico8_lq_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si `make class-coverage-net` n'a pas de ligne « Runs » identique à la référence, ou si cette référence, les exemptions ou `KNOWN[]` changent.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit un des six membres directement, ou si ses valeurs changent après le typage (`RAW_PRESENCE_HASH`, `REPLAY_PRESENCE_HASH`, les trois comptes de `bg`).
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 6 mutations, chacune avec son test ou la justification de son trou, plus la note sur les `assert` R5 non atteints.
- `FAILED:` s'il manque la note « Découvert à 1b.10 », ou si elle omet la lecture de 1b.5 appliquée à `bg`, la forme R5 de la boucle de clés directe, l'`assert` de tag `UNKNOWN` pour `ref:autre`, ou le constat « R5 écrivain toujours vacante ».
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- `jo.lq` (1b.23c), `js.lq` et `js.h_` (1b.34, dont le mélange `bool,ref:pico8_lq_t`), et l'écrivain par nom de `js.lq` (`menu.c`).
- `items` : sa forme, son remplissage (TROU-25) et ses lecteurs dans les tests.
- `jb` (typé en 1b.2) et `pico8_lq_item_t`.
- Libérer la rangée, `items`, `fs` ou ses entrées (TROU-7).
- Donner une forme à la moitié écrivain de R5 (1b.13), ou à une table `offsetof` qui garde des entrées ou a des filles (1b.23, 1b.32).
- Remplacer une lecture par nom par un accès direct.
- Régénérer `tools/member_census.txt` (R8).
- Les fichiers d'événements et `FRAMES_EVENTS`.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `layout.c/.h` réécrits, plus trois lectures directes de `ui_button.c` ;
  - tests : `test_pico8_lq` étendu d'une caractérisation au lecteur brut, sans nouveau TU ;
  - un commit, avec le découpage de repli écrit (220 puis 217) ;
  - `bg` en lecture {fixe, bool} de 1b.5, pas en {bool, chaine} de 1b.7, et pas de lecture nouvelle de R1 ;
  - un emballage public `pico8_lq_bg`, appelé par le lecteur et la vue. `js_kj` lit les indicateurs ;
  - aucun emballage pour `c`/`score` ;
  - `ra` en `pico8_vec_t *` emprunté, partagé par les copies ;
  - boucle de clés : une branche par clé et une aide par genre ;
  - `ref:autre` en boucle de clés : `assert` tag `UNKNOWN` ;
  - `w` nil par clé : `assert` ;
  - aucun `set_field`, donc R5 écrivain vacante ;
  - `g_lq_members[]`, `lq_member` et `lq_is_nil` supprimés ;
  - aucun `must_abort` nouveau pour les `assert` R5 ;
  - `class-coverage-net` lancé ;
  - pas de découpage malgré l'empreinte de 27 (homonymes).
- **Note « Découvert à 1b.10 »**, à écrire sur la ligne 1b.10 de `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante :
  - « lecture de R1 {fixe, bool} de 1b.5 appliquée à `bg` sans écart, même si seul `false` est écrit (un fixe n'a pas de NULL pour porter `false`, donc pas de lecture 1b.7) ; emballage `pico8_lq_bg` appelé aussi par la vue en table qui sert à la recopie » ;
  - « R7 `offsetof` de 1b.9 appliquée sans écart (`g_lq_members[]`, `lq_member`, `lq_is_nil` supprimés) » ;
  - « **R5, boucle de clés directe** (sans table intermédiaire) : une branche par clé (forme 1b.5) qui appelle une aide `static` par genre (forme 1b.9), la clé inconnue dans l'`else` final avec son message ; un membre reçu par clé puis réécrit par `pq` (`fs`, `w`) est déballé quand même ; `w` sans indicateur : nil → `assert` » ;
  - « **lecture de R5 pour `ref:autre`** en boucle de clés : REF non NULL dont `pico8_type_tag_of` rend `UNKNOWN`, `assert` sinon (sert à `nf.ra`, `jo.ra`, `qb.v`/`ma`…) » ;
  - « **R5 écrivain toujours vacante** (`lq.set_field` NULL) ; première candidate : 1b.13 ».

  Le reste va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le compte 223 → 217 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov` ;
  - les 6 mutations et le trou justifié des `assert` R5 ;
  - les valeurs relevées de la caractérisation (`RAW_PRESENCE_HASH`, `REPLAY_PRESENCE_HASH`, les comptes de `bg`) ;
  - les réponses à H1 à H15 ;
  - l'empreinte remesurée (27, dont 23 homonymes).
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
