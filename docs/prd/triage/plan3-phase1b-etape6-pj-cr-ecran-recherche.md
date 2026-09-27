---
title: "Plan 3, étape 1b.6 : `pj_cr` (en-tête de l'écran de recherche) — `hi`/`gq` deviennent des `pico8_vec_t *`, `ca` un `pico8_fixed_t` sans indicateur, `ep` un `const char *`"
description: "`pj_cr` a quatre membres `pico8_value_t` lus par nom (`hi`, `gq` : `ref:autre` ; `ca` : `fixe`, nil jamais ; `ep` : `chaine`, nil observé). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 257 à 253. Instance statique, sans constructeur, sans écrivain et sans table d'offsets : R5 et R7 restent vacantes."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.6 — `pj_cr` (`score_panel.h:29`). Empreinte 7 (3, 4). Compte : 253.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 ». Il est cité, pas redécidé. S'appliquent aussi : « Découvert à 1b.3 » (instance statique : R3 se lit sur l'initialiseur agrégé), « à 1b.4 » (`ref:autre` vers un `v(…)` → `pico8_vec_t *` emprunté, même pointeur), « à 1b.4b » (`pj_cr.ca` n'a plus de preuve de nil) et « à 1b.5 » (forme R7 d'un accesseur `pico8_X_member`).

### État de départ

HEAD b230578. **Précondition** : 1b.5 (`trade`) est commitée avant que 1b.6 commence. `make value-members` rend alors 257.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H8).

### Lignes de recensement (`tools/member_census.txt:192-195`, régénéré par 1b.4b)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `hi` | offset | `ref:autre` | jamais | - | l |
| `gq` | offset | `ref:autre` | jamais | - | l |
| `ca` | offset | `fixe` | jamais | - | l |
| `ep` | offset | `chaine` | observé | lu,stat? | l |

« offset » veut seulement dire « typedef d'un en-tête » : aucune table d'offsets ne vise `pico8_pj_cr_t` (`grep -rn 'offsetof(pico8_pj_cr_t' src/` est vide).

### Ce que fait le cart (`pico8/lua.lua`)

`pj` porte `cr=o(hi=v(0,11),gq=v(0,11),ca=10)` (`:827`). `cr()` (`:952-964`) pose `pj.cr.ep="level "..tl.." (…/…)"`, puis `pz(pj.cr,bs)`. Rien d'autre n'écrit `pj.cr`.

### Le code aujourd'hui

- `src/pico8_score_panel.h:27-33` : `{ header; pico8_value_t hi, gq, ca, ep; }`.
- `src/pico8_score_panel.c` :
  - lecteur `pico8_pj_cr_field` (`:116-136`) : `strcmp` qui **copie** la valeur (ce n'est pas un accesseur `pico8_X_member` : il ne rend pas de `pico8_value_t *`), puis rend 0 sur `REF NULL` ;
  - descripteur (`:138-142`) : `set_field` NULL, aucune méthode ;
  - initialiseur agrégé `g_pj_cr` (`:148-154`) : `hi`/`gq` → `&g_pj_cr_hi`/`&g_pj_cr_gq` (deux vecteurs statiques égaux, distincts), `ca` fixe 10, `ep` `REF NULL` ;
  - `pico8_score_panel_init` (`:522`) remet `ep` à nil ; `:529` `PICO8_MEMBER_CENSUS_SEE(&g_pj_cr, pico8_pj_cr_t)`.
  - `g_pj_defaults[PJ_CR]` (`:111`, `:523`) est une entrée de la table de défauts de **`pj`** (sans membre) qui pointe sur `g_pj_cr`. Elle n'est pas concernée (1b.16).
- Écriture directe : `src/pico8_menu.c:280` `cr->ep = pico8_value_string(ep)` (`cr()`).
- Lecteurs par nom : `pz` (`msgbox.c:466-510`, `ep`, `hi` ; `pz_step` `:453-458`, `ca` (TROU-9 → `fa`), `gq`) et `test_pico8_pj.c`. Ils ne changent pas.
- Accès direct dans un test : `test_pico8_pj.c:709` `cr->ep.kind == PICO8_VALUE_STRING`.

### Nil, membre par membre

- `ca` : nil **jamais** (note 1b.4b). Seul l'initialiseur l'écrit. `grep -rn 'cr->ca\|g_pj_cr\.ca' src/` ne trouve que le lecteur. Pas de `has_ca` (R2), contrairement à `ha_pz.ca` (1b.4), que le shutdown remettait à nil.
- `hi`, `gq` : nil jamais ; pointeurs, donc sans indicateur.
- `ep` : nil observé (`lu` : `test_cr` lit `!cr_get(cr,"ep")` avant le premier `cr()`). Pointeur : NULL = nil.

### Empreinte remesurée

5 (3, 2) : `score_panel.h`, `score_panel.c`, `menu.c`, `test_pico8_pj.c`, `test_pico8_js.c` (ce dernier appelle seulement `pico8_cr()`). L'écart avec 7 vient des noms partagés (`ca`, `ep`, `hi`, `gq`) que le décompte de la liste rattache aussi à d'autres typedefs. Fichiers modifiés : `src/pico8_score_panel.h`, `src/pico8_score_panel.c`, `src/pico8_menu.c`, `tests/test_pico8_pj.c` et `docs/plan-3.md`.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** : `pico8_vec_t *hi; pico8_vec_t *gq; pico8_fixed_t ca; const char *ep;`. Aucun indicateur.
3. Effet mesurable : `make value-members` passe de 257 à **253** et ne liste plus aucune ligne `pico8_pj_cr_t`. Aucun pixel ne bouge.

## User Stories

1. En tant que lecteur de `score_panel.h`, je veux les quatre membres de `pj.cr` typés, afin que leur genre se lise sur le type.
2. En tant que `pz` et `pz_step`, je veux lire `ep`, `hi`, `gq` et `ca` par nom avec les mêmes genres, valeurs et **pointeurs** qu'en 1a.
3. En tant que `test_cr`, je veux `ep` absent jusqu'au premier `cr()`, puis gardé d'une partie à l'autre.
4. En tant que `test_cr_is_a_struct`, je veux `ep` remis à nil par l'init, `ca` toujours présent, `hi` ≠ `gq`, et l'écriture par nom qui avorte toujours.
5. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
6. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
7. En tant que `plan-pilot`, je veux `make value-members` à 253.
8. En tant qu'auteur des étapes suivantes, je veux savoir que R5 et R7 restent vacantes après 1b.6.
9. En tant que vérificateur, je veux une caractérisation par nom, lancée sur l'ancien code, et les mutations du §7 déclinées sur `pj_cr`.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline`, et marqués (G).

### R1–R9 appliquées à `pj_cr` (patron de 1b.1, cité)

| règle | sur `pj_cr` |
|---|---|
| R1 | `hi`, `gq` → `pico8_vec_t *` (lecture de 1b.4 pour `ref:autre` vers `v(…)`). `ca` → `pico8_fixed_t`. `ep` → `const char *` (`chaine`, comme `mz_jj.qs` en 1b.3). Noms et ordre gardés. |
| R2 | Aucun indicateur : `hi`/`gq`/`ep` sont des pointeurs ; `ca` est à nil `jamais`. |
| R3 | Initialiseur agrégé (note 1b.3) : `&g_pj_cr_hi`, `&g_pj_cr_gq`, `10 << PICO8_FIXED_SHIFT`, `NULL`. Mêmes vecteurs statiques. La remise à nil de l'init devient `g_pj_cr.ep = NULL`, à la même place. |
| R4 | Une branche par membre, dans l'ordre actuel. `hi`/`gq`/`ep` NULL → `return 0`, `*out_value` intact ; sinon `pico8_value_ref(hi/gq)` ou `pico8_value_string(ep)`, même pointeur. `ca` → toujours `pico8_value_fixed(ca)`. Tout autre nom rend 0. Le test final `REF NULL` disparaît. |
| R5 | **Vacante, les deux moitiés.** Pas d'écrivain (`set_field` NULL, `must_abort(write_ep)` figé) ; pas de constructeur, donc pas de boucle de clés. |
| R6 | `menu.c:280` devient `cr->ep = ep;`. Le compilateur désigne `test_pico8_pj.c:709` (voir Testing Decisions). |
| R7 | **Vacante.** Pas de `members[]`/`defaults[]`/`keys[]` sur `pico8_pj_cr_t`, et `pico8_pj_cr_field` n'est pas un accesseur `pico8_X_member` (forme 1b.5) : il copie la valeur. Preuves, vides avant et après : `grep -rn 'offsetof(pico8_pj_cr_t' src/`, `grep -n 'pico8_value_t \*' src/pico8_score_panel.c` restreint à `pj_cr`, et `grep -rnE '(cr\|g_pj_cr)(->\|\.)(hi\|gq\|ca\|ep)\.(kind\|as)' src/ tests/`. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(&g_pj_cr, pico8_pj_cr_t)` gardé. `tools/member_census.txt` n'est pas édité. |
| R9 | 257 → 253, aucune ligne `pico8_pj_cr_t`. |

- (G) **Pas de `has_ca`** : ni écrivain, ni reset, ni lecteur qui voie `ca` nil (H3). Ajouter un indicateur sans preuve de nil irait contre R2.
- (G) La chaîne de `ep` reste allouée par `cr()` et abandonnée à la réécriture suivante (TROU-7, seconde convention). Aucune libération n'est ajoutée : ce serait corriger un trou.
- Aucun site par nom ne change, et aucun n'est remplacé par un accès direct.
- Aucun commentaire nouveau. Le commentaire du typedef (« no parent, never an entity, and no writer ») et celui de `pico8_pj_cr()` restent exacts.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 257 sur l'arbre de départ (1b.5 commitée), puis 253.
- **H2** : aucun code n'écrit `hi`, `gq` ou `ca` après l'initialiseur, et aucun écrivain générique n'atteint `pj_cr` (il avorterait sur `set_field` NULL).
- **H3** : `ca` n'est jamais lu nil dans les 15 rejeux ni dans les TU (recensement : `jamais`).
- **H4** : `pz` lit `hi`, `gq`, `ca`, `ep` à l'identique ; `CR_HASH`, `REPLAY_HASH` (`test_pico8_pj`), `CR_MY_HASH` (`test_pico8_js`) et les 15 empreintes ne bougent pas.
- **H5** : `PICO8_MEMBER_CENSUS_SEE` compile sur un typedef sans plus aucun `pico8_value_t` (comme `mh` après 1b.1, `ha_pz` après 1b.4). Le build `-DPICO8_MEMBER_CENSUS` n'est pas lancé.
- **H6** : `test_pico8_pj` (dans `UNIT_TESTS`, hors `UNIT_TESTS_SLOW`) reste sous 30 s avec la caractérisation.
- **H7** : sous ASan+UBSan, `ep` en `const char *` vers une chaîne abandonnée ne signale rien (`detect_leaks=0`, TROU-7).
- **H8** : `make class-coverage-net` resterait identique à la référence. Il n'est pas lancé, voir « Cibles finales ».

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne fait aucun accès `cr->hi/gq/ca/ep` et compile sur les deux formes.
- **Caractérisation, figée sur l'ancien code** : `test_cr_members_by_name`, dans `test_pico8_pj`, lancée par `in_fresh_child`. Pas de nouveau TU.
  1. Après `start_game()` : `hi` et `gq` présents, genre `REF`, pointeurs non NULL et **distincts**, chacun valant le vecteur (0, 11) ; `ca` présent, `FIXED 10` ; `ep` absent.
  2. Après `pico8_cr()` : `ep` présent, genre `STRING`, texte égal au littéral relevé au premier passage vert sur l'ancien code ; `hi`/`gq` mêmes pointeurs qu'en 1, `ca` inchangé. Un second `pico8_cr()` rend un `ep` de même texte.
  3. Lecteur (`cls->field`) sur `"x"`, `""`, `"eo"`, `"ra"` et sur `ep` absent : rend 0, `out` égal au témoin (fixe `0x1234`).
  4. Après un nouveau `pico8_score_panel_init()` : `ep` absent ; `hi`/`gq` mêmes pointeurs ; `ca` `FIXED 10`.

  Elle est lancée verte sur l'arbre de départ **avant** le typage. Ses valeurs ne changent pas ensuite.
- **Seule autre retouche de `test_pico8_pj.c`** : `:709` `cr->ep.kind == PICO8_VALUE_STRING` devient une lecture par nom (`lookup(cr, "ep", &v) && v.kind == PICO8_VALUE_STRING`), même intention. `test_pico8_pj` n'est pas un test de régression de bug (§6). Le reste du diff du fichier est un pur ajout.
- **Mutations (§7)**, dans une copie jetable du scratchpad. Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : pas d'indicateur. Forme la plus proche : le lecteur ignore `ep == NULL` et rend `STRING NULL`. Attendu rouge : caractérisation 1 et 3, `test_cr` (`!cr_get`), `test_cr_is_a_struct` (`!has(cr,"ep")`).
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre**, aucun fixe à indicateur, `ca` n'est jamais absent (justifié par H3). Deux formes voisines : (a) défaut propre, `ca` initialisé à 0 → rouge : caractérisation 1, `test_cr_is_a_struct` (`ca` = 10) ; (b) branche `ca` qui rend 0 → rouge : `test_cr_is_a_struct` (`has(cr,"ca")`), caractérisation 1 et 4.
  3. (§7-3) *init qui saute un membre typé* : pas de boucle d'init. Formes : (a) la remise `g_pj_cr.ep = NULL` de l'init est supprimée → rouge : `test_cr_is_a_struct:713`, caractérisation 4 ; (b) l'initialiseur agrégé omet `gq` (NULL) → rouge : caractérisation 1, `test_cr_is_a_struct`, `CR_HASH`.
  4. Propre à l'étape, *aliasing invisible à l'écran* : `gq` initialisé à `&g_pj_cr_hi`. Les pixels ne bougent pas (deux vecteurs égaux). Attendu rouge : caractérisation 1 (pointeurs distincts), `test_cr_is_a_struct` (`gq` ≠ `hi`).
- **`gcov`** (`test_pico8_pj` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23) : chaque ligne modifiée du lecteur, de l'init et de `cr()` est exécutée, sauf les `return 0` sur `hi == NULL` et `gq == NULL`, que l'initialiseur rend inatteignables (H2). L'initialiseur agrégé n'est pas une ligne exécutable : les mutations 2a, 3b et 4 le prouvent. Le rapport va dans le commit.
- **Prior art** : `test_cr`, `test_cr_is_a_struct` (`test_pico8_pj`), `test_mh_members_by_name` (1b.1), `test_ha_pz_members_by_name` (1b.4).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur : `make test` 20 s à 1 min ; `make e2e` 4 à 6 min ; `make sanitize-net` ~4 min seul (on le lance seul) ; `make value-members` quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 253`**, sans aucune ligne `pico8_pj_cr_t`.

`make class-coverage-net` **ne s'applique pas** (§7). `pj_cr` n'a pas de constructeur ; le diff touche un initialiseur statique, une ligne de `pico8_score_panel_init` (module, sans `pico8_class_coverage_record`), un lecteur et `cr()`. `pico8_pj_new` (`:462`, `pico8_class_coverage_record("pj")`) et `KNOWN[]` ne bougent pas. S'il en touche un, la cible redevient obligatoire, avec une sortie identique à `tools/class_coverage_net_expected.txt`. `make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si un membre est ajouté à `pj_cr` (indicateur compris), ou si un écrivain, une méthode ou un constructeur est ajouté.
- `FAILED:` si `hi`/`gq` ne sont pas des `pico8_vec_t *` vers `g_pj_cr_hi`/`g_pj_cr_gq`, si `ca` n'est pas `pico8_fixed_t`, ou si `ep` n'est pas `const char *`.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un `ep` NULL ou un nom inconnu, ou ne rend pas le même pointeur pour `hi`/`gq`/`ep`.
- `FAILED:` si la chaîne de `ep` est libérée, ou si un autre écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si un test de régression de bug (§6) change, si une valeur figée existante change (`CR_HASH`, `REPLAY_HASH`, `CR_MY_HASH`…), ou si le diff de `test_pico8_pj.c` fait autre chose qu'ajouter la caractérisation et réécrire la ligne `:709` par nom.
- `FAILED:` si un site par nom change (`pz`, `pz_step`, lectures de `test_pico8_pj`).
- `FAILED:` si une preuve de R7 n'est pas vide, ou si `g_pj_defaults` change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 253, liste encore `pico8_pj_cr_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si le diff touche un constructeur, `pico8_class_coverage_record` ou `KNOWN[]` sans une ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit un membre directement, ou si ses valeurs changent après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 4 mutations, avec leur test ou la justification de leur trou.
- `FAILED:` s'il manque la note « Découvert à 1b.6 » ou le tableau « Runs ».

## Out of Scope

- `pj` et son entrée de défaut `cr` (`g_pj_defaults`, 1b.16).
- Libérer l'ancienne chaîne de `ep` (TROU-7) ; corriger TROU-9.
- Donner une forme à R5 (écrivain ou boucle de clés) ou à R7 sous sa forme `offsetof`.
- Régénérer `tools/member_census.txt` (R8).
- Les fichiers d'événements et `FRAMES_EVENTS`.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) : modules `score_panel.c/.h` et `menu.c` (une ligne) ; tests : `test_pico8_pj` étendu, sans nouveau TU ; un commit ; pas de `has_ca` ; `ep` en `const char *` non possédé ; R5 et R7 vacantes ; `class-coverage-net` non lancé.
- **Note « Découvert à 1b.6 »**, à écrire sur la ligne 1b.6 de `docs/plan-3.md` : « patron 1b.1 appliqué sans écart ; `fixe` à nil `jamais` sans indicateur même quand un homonyme en a un (`ha_pz.ca`) ; ni écrivain ni table d'offsets (le lecteur copie, ce n'est pas un `pico8_X_member`), R5 et R7 `offsetof` restent à fixer par la première étape qui en a. » Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 257 → 253, le résumé du tableau « Runs », le rapport `gcov`, les mutations et les réponses à H1 à H8.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
