---
title: "Plan 3, étape 1b.5 : `trade` (route d'export) — `lp`/`rl` deviennent des `pico8_list_t *`, `qe` un `pico8_fixed_t` avec `has_qe` et `qe_is_bool`"
description: "`trade` a trois membres `pico8_value_t` lus par nom (`lp`, `rl` : `ref:liste` ; `qe` : `bool,fixe`). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 260 à 257. Première étape avec un accesseur `pico8_X_member` : elle fixe la forme concrète de R7 pour cet accesseur. `qe` est à genres mêlés (fixe ou booléen, TROU-24), ce que le recensement corrigé par 1b.4b montre."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> - 1b.5 — `trade` (`trade.h:12`), `qe` tenu, lu par nom. Empreinte 6 (5, 1). Compte : 257.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 » de `docs/plan-3.md`. Il est cité ici et n'est pas redécidé. Les notes « Découvert à 1b.2 » et « à 1b.3 » s'appliquent : sans écrivain ni table d'offsets, R5 et R7 restent vacantes jusqu'à la première étape qui en a une. **Cette étape est la première à avoir une table d'offsets** (l'accesseur `trade_member`, forme `pico8_X_member` de R7). Elle n'a pas d'écrivain par nom.

### État de départ

HEAD 11d5e77 : 1b.4 (`ha_pz`) et 1b.4b (recensement des fils forkés) sont commitées. `make value-members` rend 260.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H8).

### Lignes de recensement (`tools/member_census.txt:221-223`, régénéré par 1b.4b)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `lp` | offset | `ref:liste` | observé | lu,stat | l |
| `rl` | offset | `ref:liste` | observé | lu,stat | l |
| `qe` | offset | `bool,fixe` | observé | lu | l |

`pico8_js_t.trade` (`:247`) est un membre homonyme de `js`. Il n'est pas concerné.

Par rapport à la table d'avant 1b.4b, `lp`/`rl` gagnent seulement la preuve `lu` (genres inchangés), et `qe` passe de `fixe` tenu à `bool,fixe` observé (`lu`), comme le dit la note « Découvert à 1b.4b ». Le `bool` vient de `kg`, qui pose `qe = false` quand `self.qe` est nil (TROU-24, `trade.c:151`), lu par nom dans `test_kg` (`test_pico8_trade.c:380,387`), et de `test_eq`, qui construit une route avec `qe = true` (`:414`). La ligne 1b.5 du plan dit encore « `qe` tenu » : la note de 1b.4b la corrige.

### Ce que fait le cart (`pico8/lua.lua`)

`trade=kv:jr()` (`:603`). `jo:ib` construit `trade({lp={self},rl={w},qe=qe})` (`:504`), où `qe` est `self.kw.qe` et peut valoir nil. `trade:kg` renvoie `trade({lp=la(self.lp,{np}),rl=la(self.rl,{jc}),qe=self.qe})` (`:606-607`). `jo:cn` compare `t.qe` (`:480-494`).

### Le code aujourd'hui

- `src/pico8_trade.h:10-15` : `{ header; pico8_value_t lp, rl, qe; }`.
- `src/pico8_trade.c` :
  - `trade_member` (`:15-26`) : un accesseur `pico8_X_member` à `strcmp` qui rend `&self->lp/rl/qe`. Il ne contient pas d'`offsetof`.
  - Lecteur (`:28-36`) : il passe par `trade_member`, puis rend 0 sur un membre `REF NULL`.
  - `set_field` est NULL (`:38-42`). L'écriture par nom avorte, ce qui est figé par `must_abort(write_qe)`.
  - Constructeur (`:44-67`) : il pose les trois membres à `REF NULL`, puis une boucle de clés fait `*trade_member(...) = value`, sous un `assert` si la clé est inconnue ou positionnelle. Suivent `pico8_class_coverage_record("trade")` et `PICO8_MEMBER_CENSUS_SEE`.
  - `to`, `kg` et `eq` lisent `self->lp/rl/qe` directement.
- Autres accès directs :
  - `planet_trade.c:117` : `t_qe = t->qe` (`jo:cn`) ;
  - `planet_family_trade.c:195-196` : `t->rl` (`nr:cn`).
- Lecteurs par nom : `economy_tiers.c:125-126` (`"rl"`, `"lp"`), `economy_tiers.c:302` (`"qe"`) et `test_pico8_trade.c`. Ils ne changent pas.

### Nil de `qe` vu par nom

Oui, et la table le confirme (`lu`). `test_construction` lit par nom `!has(t, "qe")` sur `pico8_trade_new(NULL)` et sur une route sans `qe` (`:294,296`). En jeu, `jo:ib` passe un `qe` nil quand `kw` n'a pas de `qe` (`planet_trade.c:242`). R2 s'applique, et `has_qe` est obligatoire.

### Empreinte remesurée

5 (4, 1) : `trade.c`, `planet_trade.c`, `planet_family_trade.c`, `economy_tiers.c` et `test_pico8_trade.c`. La ligne dit 6 parce qu'elle compte aussi `trade.h`, qui déclare le typedef et change. Fichiers modifiés : `src/pico8_trade.h`, `src/pico8_trade.c`, `src/pico8_planet_trade.c`, `src/pico8_planet_family_trade.c`, `tests/test_pico8_trade.c` (caractérisation seule) et `docs/plan-3.md`.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** :
   - `lp` et `rl` deviennent `pico8_list_t *`. Ce sont des pointeurs : NULL = nil.
   - `qe` devient `pico8_fixed_t qe; int has_qe; int qe_is_bool;`.
   - `trade_member` est supprimé (R7).
3. Effet mesurable : `make value-members` passe de 260 à **257** et ne liste plus aucune ligne `pico8_trade_t`. Aucun pixel ne bouge.

## User Stories

1. En tant que lecteur de `trade.c`, je veux `lp`/`rl` en `pico8_list_t *` et `qe` en fixe typé, afin que `to`, `kg` et `eq` ne déballent plus de REF.
2. En tant que `dh`, `jo:cn` et `nr:cn`, je veux lire `lp`, `rl` et `qe` avec les mêmes genres, les mêmes valeurs et **les mêmes pointeurs** qu'en 1a.
3. En tant que `test_kg`, je veux qu'une route allongée depuis un `qe` nil lise toujours `BOOL` false (TROU-24 préservé).
4. En tant que `test_construction`, je veux un `qe` absent ou nil lu absent, et un `qe` fixe 0 lu présent.
5. En tant que `test_trade_is_a_struct`, je veux que l'écriture par nom et une clé inconnue ou positionnelle avortent toujours.
6. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
7. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
8. En tant que `plan-pilot`, je veux `make value-members` à 257 et `make class-coverage-net` identique à la référence.
9. En tant qu'auteur des étapes suivantes (`hy`, `hp`, `lq`, `js`, `qh`, qui ont aussi un `pico8_X_member`), je veux la forme concrète de R7 pour un tel accesseur, et la forme retenue pour des genres scalaires mêlés.
10. En tant que vérificateur, je veux une caractérisation par nom, lancée sur l'ancien code, et les mutations du §7 déclinées sur `trade`.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline`, et marqués (G).

### R1–R9 appliquées à `trade` (patron de 1b.1, cité)

| règle | sur `trade` |
|---|---|
| R1 | `lp`, `rl` → `pico8_list_t *` (`ref:liste`). **`qe` : genres mêlés {`fixe`, `bool`}**, lus dans la table (`bool,fixe`). (G) Le type retenu est `pico8_fixed_t qe` plus `int qe_is_bool`, déclaré après `has_qe`. `qe_is_bool` vaut 1 quand `qe` tient un booléen (0/1), et 0 quand `qe` tient un fixe. C'est la plus petite forme qui rend `BOOL` false et `FIXED 0` distincts sans sentinelle. Noms et ordre gardés. |
| R2 | `lp`, `rl` : pas d'indicateur (pointeurs). `qe` : `int has_qe` juste après `qe`. C'est un non-pointeur lu par nom, à nil observé (`lu`, `test_construction`). `qe_is_bool` n'est lu que si `has_qe` vaut 1. |
| R3 | Au début du constructeur, avant la boucle de clés : `lp = NULL`, `rl = NULL`, `qe = 0`, `has_qe = 0`, `qe_is_bool = 0`. Pas de défaut de spec (spec vide). `pico8_class_coverage_record` et `PICO8_MEMBER_CENSUS_SEE` restent à leur place. |
| R4 | Une branche `strcmp` par membre. `lp`/`rl` NULL → `return 0`, sinon `pico8_value_ref(ptr)`, même pointeur. `qe` : `has_qe == 0` → `return 0`, sinon `pico8_value_bool(qe)` si `qe_is_bool`, `pico8_value_fixed(qe)` sinon. Tout autre nom rend 0. Dans tous ces cas, `*out_value` reste intact. |
| R5 | **Boucle des clés, sous la forme de `mh` (1b.1)** : une branche par nom. Une clé non nommée, ou un nom autre que `lp`/`rl`/`qe`, déclenche l'`assert` existant, **message inchangé**. `lp`/`rl` : `REF NULL` → NULL ; `REF` d'étiquette `LIST` → le pointeur ; tout autre cas → `assert` qui nomme le membre et l'arrêt « champ sans règle ». (G) L'étiquette `LIST` tient lieu de l'« `assert` sur tag » de `ref:pico8_X_t`. `qe` : `REF NULL` → `has_qe = 0` ; `FIXED` → `qe = as.fixed`, `qe_is_bool = 0`, `has_qe = 1` ; `BOOL` → `qe = as.boolean`, `qe_is_bool = 1`, `has_qe = 1` ; tout autre genre → `assert` « champ sans règle ». **L'écrivain par nom reste vacant** (`set_field` NULL) : sa forme n'est pas écrite ici. |
| R6 | Méthodes, désignées par le compilateur : `to`/`eq` testent `self->lp == NULL` et lisent `self->lp` sans `PICO8_REF_AS_LIST_CONST` ; `kg` fait de même pour `lp`/`rl`. `planet_family_trade.c:195-196` teste `t->rl != NULL`, puis `t_rl = t->rl`. Pour `qe`, voir la puce qui suit ce tableau. |
| R7 | **Première forme concrète (accesseur `pico8_X_member`)**, fixée ici : dans un accesseur qui rend `pico8_value_t *` par `strcmp`, la branche du membre typé est retirée. Ses appelants (lecteur, boucle de clés) prennent pour ce nom la branche R4/R5 explicite, placée avant l'appel. Un accesseur sans branche restante est supprimé. Ici, `trade_member` disparaît. Preuves, vides après : `grep -n 'trade_member\|pico8_value_t \*' src/pico8_trade.c`, `grep -rn 'offsetof(pico8_trade_t' src/` (déjà vide) et `grep -rnE '(->\|\.)(lp\|rl\|qe)\.(kind\|as)' src/` sur un `trade`. La forme `offsetof` (`members[]`/`defaults[]`/`keys[]`) reste à fixer par la première étape qui en a une. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_trade_t)` est gardé. `tools/member_census.txt` n'est pas édité : sa ligne `qe` dit déjà `bool,fixe`. |
| R9 | 260 → 257, aucune ligne `pico8_trade_t`. |

- (G) **Un seul emballage de `qe`** : `pico8_value_t pico8_trade_qe(const pico8_trade_t *self)`, déclaré dans `trade.h`, rend le `qe` du genre de 1a. Il rend `pico8_value_ref(NULL)` pour un `qe` nil ou un `self` NULL. Il sert :
  - au lecteur ;
  - à `kg` : `qe_val = has_qe ? pico8_trade_qe(self) : pico8_value_bool(0)`, avec `/* TROU-24 */` gardé ;
  - à `jo:cn` : `t_qe = pico8_trade_qe(t)` remplace `t->qe`.

  Trois sites, une règle : on n'écrit pas trois fois le choix fixe/booléen. Son contrat va dans `trade.h`. C'est un contrat C.
- Aucun site par nom ne change, et aucun n'est remplacé par un accès direct : `dh` (`economy_tiers.c`) et le test restent tels quels.
- Le commentaire de `trade.h` sur `lp`/`rl` (« borrowed, not copied ») et celui de `kg` (TROU-24) restent exacts. Le contrat de `has_qe`/`qe_is_bool` s'écrit au format de `mh.h`. On n'écrit pas d'autre commentaire.
- `#include "pico8_ref_audit.h"` est retiré de `trade.c` s'il n'y sert plus.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 260 sur l'arbre de départ, puis 257.
- **H2** : aucun rejeu du filet ni aucun TU ne passe à `pico8_trade_new` un `qe` d'un genre autre que fixe, booléen ou nil, ni un `lp`/`rl` qui ne soit pas une liste étiquetée. Sinon, l'`assert` de R5 rougit `make test`, et c'est l'arrêt « champ sans règle ». Les `kw.qe = "x"` de `ga_export_panel_by`/`gh_row_lq` ne passent pas par `jo:ib`.
- **H3** : `pico8_ob_table_set_name(t, "qe", REF NULL)` garde une entrée `REF NULL` (c'est ainsi que `jo:ib` passe un `qe` nil). La caractérisation l'utilise pour atteindre la branche nil de R5.
- **H4** : en jeu, `qe` n'est jamais booléen dans les 15 rejeux (TROU-24 : non atteint). Les empreintes et les `*_REPLAY_HASH` de `test_pico8_trade` ne bougent pas.
- **H5** : `test_pico8_trade` (dans `UNIT_TESTS`, hors `UNIT_TESTS_SLOW`) reste sous 30 s avec la caractérisation.
- **H6** : `make class-coverage-net` rend `trade  21032  6/15`, identique à la référence.
- **H7** : `test_pico8_spec_defaults` (`:162`, `pico8_trade_new(NULL)`) ne hache aucun membre de `trade` qui changerait.
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile encore. Ce n'est pas lancé.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne fait aucun accès `->lp/rl/qe` et compile sur les deux formes.
- **Caractérisation, figée sur l'ancien code** : `test_trade_members_by_name`, dans `test_pico8_trade`, lancée par `in_fresh_child`. Pas de nouveau TU.
  1. Sur `game_route()` : `lp`/`rl` présents, de genre `REF`, d'étiquette `LIST` ; `qe` présent, de genre `FIXED`.
  2. On construit des tables explicites (`pico8_ob_table_set_name`, sans le helper `route`, qui omet nil), avec chaque `qe` : `FIXED 14` → lu `FIXED 14` ; **`FIXED 0` → présent `FIXED 0`** ; `BOOL` true → `BOOL` 1 ; `BOOL` false → **présent** `BOOL` 0 ; `REF NULL` explicite → absent ; clé omise → absent.
  3. `lp`/`rl` : une liste passée → **même pointeur** ; `REF NULL` explicite ou clé omise → absent.
  4. Lecteur (`cls->field`) sur `"to"`, `"x"`, `""`, et sur chaque membre absent : rend 0, et `out` reste égal au témoin (fixe `0x1234`).
  5. `kg` : depuis un `qe` `FIXED 0` → `FIXED 0` ; depuis un `qe` `BOOL` true → `BOOL` 1 ; depuis un `qe` explicitement nil → `BOOL` false.

  Elle est lancée verte sur l'arbre de départ **avant** le typage. Ses valeurs ne changent pas ensuite. Le diff de `test_pico8_trade.c` est un pur ajout.
- **Mutations (§7)**, dans une copie jetable du scratchpad. Chacune est notée dans le commit avec le test qui la rougit.
  1. (§7-1) *indicateur forcé vrai* : le lecteur ignore `has_qe`. Attendu rouge : caractérisation 2 et 4, `test_construction` (`!has(t,"qe")`).
  2. (§7-2) *« présent 0 » là où c'était absent* : la branche `REF NULL` de R5 pose `has_qe = 1`. Attendu rouge : caractérisation 2.
  3. (§7-3) *boucle d'init qui saute un membre typé* : la branche `rl` ne pose rien. Attendu rouge : caractérisation 3, `test_construction` (`get_ref(t,"rl") == rl`), `GAME_ROUTE_HASH`, les `*_REPLAY_HASH`.
  4. Propre à l'étape, *genre perdu* : le lecteur ignore `qe_is_bool`. Attendu rouge : caractérisation 2 et 5, `test_kg:380`.
  5. Propre à l'étape, *TROU-24 corrigé* : `kg` recopie un `qe` nil au lieu de poser false. Attendu rouge : `test_kg:380,387`, caractérisation 5.
- **`gcov`** (`test_pico8_trade` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23) : chaque ligne modifiée de `trade.c` est exécutée par la caractérisation, y compris les branches R5 `REF NULL`/`FIXED`/`BOOL` et les branches nil de R4. Les branches `assert` « champ sans règle » de R5 ne sont pas exécutées : ce trou est justifié, un `assert` qui tire tue le fils. Les lignes de `planet_trade.c:117` et `planet_family_trade.c:195` sont exécutées par `test_hub_replay` (H4). **Aucun autre trou n'est attendu.** Le rapport va dans le commit.
- **Prior art** : `test_construction`, `test_kg` et `test_trade_is_a_struct` (`test_pico8_trade`), `test_mh_members_by_name` (1b.1) et `test_ha_pz_members_by_name` (1b.4).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make sanitize-net` : ~4 min seul, le double sous charge (on le lance seul) ;
- `make class-coverage-net` : 3 à 4 min ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 257`**, sans aucune ligne `pico8_trade_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**. Le diff touche le corps du constructeur `pico8_trade_struct_new` (R3, R5), même si `pico8_class_coverage_record("trade")` et `KNOWN[]` ne bougent pas (§7, même lecture qu'à 1b.1 et 1b.2). La sortie doit être identique à `tools/class_coverage_net_expected.txt`.

`make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). L'utilisateur y travaille : ses changements non commités sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si un `assert` de R5 tire dans `make test` ou `make e2e` (H2), si un membre est ajouté à `trade` (sauf `has_qe` et `qe_is_bool`), si un écrivain ou une méthode est ajouté, ou si le lecteur ou la boucle de clés accepte un autre nom.
- `FAILED:` si `lp`/`rl` ne sont pas des `pico8_list_t *` qui gardent la même REF, ou si `qe`/`has_qe`/`qe_is_bool` ne sont pas `pico8_fixed_t`/`int`/`int`.
- `FAILED:` si `trade_member` survit, ou si une preuve de R7 n'est pas vide.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un membre nil, ou si un `qe` `FIXED 0` ou `BOOL` false est lu absent.
- `FAILED:` si TROU-24 est corrigé, si son `/* TROU-24 */` disparaît, ou si un autre écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le message de l'`assert` de clé inconnue change.
- `FAILED:` si un test de régression de bug (§6) change, si une valeur figée existante change (`GAME_ROUTE_HASH`, `*_REPLAY_HASH`, `*_ROUTES`), ou si le diff de `test_pico8_trade.c` n'est pas un pur ajout.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 257, liste encore `pico8_trade_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si `make class-coverage-net` n'est pas identique à la référence, ou si cette référence, les exemptions ou `KNOWN[]` changent.
- `FAILED:` si un site par nom change (`economy_tiers.c`, les lectures de `test_pico8_trade`).
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit un membre directement, ou si ses valeurs changent après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 5 mutations avec le test qui rougit chacune.
- `FAILED:` s'il manque la note « Découvert à 1b.5 », ou si elle omet la forme R7 `pico8_X_member`, ou la forme `qe_is_bool`.
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- Corriger TROU-24 ou TROU-3 (`dh`).
- Régénérer `tools/member_census.txt` (R8).
- `js.trade` et les `qe` des tables `kw`/`jy` (des `pico8_ob_table_t`, phase D).
- Donner une forme à l'écrivain de R5, ou à R7 sous sa forme `offsetof`.
- Les fichiers d'événements et `FRAMES_EVENTS`.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `trade.c/.h`, plus deux lectures directes (`jo:cn`, `nr:cn`) ;
  - tests : `test_pico8_trade` étendu, sans nouveau TU ;
  - un commit ;
  - `qe` à genres mêlés tranché en `pico8_fixed_t` + `has_qe` + `qe_is_bool` ;
  - `has_qe` gardé, parce qu'un lecteur voit nil ;
  - un seul emballage public, `pico8_trade_qe` ;
  - R7 fixée pour `pico8_X_member` ;
  - l'écrivain de R5 reste vacant ;
  - `class-coverage-net` lancé, parce que le constructeur change.
- **Note « Découvert à 1b.5 »**, à écrire sur la ligne 1b.5 de `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante :
  - « **R7, forme `pico8_X_member`** : la branche du membre typé est retirée de l'accesseur ; lecteur et boucle de clés prennent une branche R4/R5 explicite ; un accesseur vide est supprimé ; preuve `grep` du nom de l'accesseur vide. La forme `offsetof` reste à fixer. » ;
  - « **lecture nouvelle de R1** : genres scalaires mêlés {fixe, bool} → `pico8_fixed_t <m>` + `int has_<m>` + `int <m>_is_bool` ; un emballage unique par membre » ;
  - « l'écrivain de R5 reste à fixer ».

  Le reste va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le compte 260 → 257 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov` ;
  - les mutations ;
  - les réponses à H1 à H8.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
