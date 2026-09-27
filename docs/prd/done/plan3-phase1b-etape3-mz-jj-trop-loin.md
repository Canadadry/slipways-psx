---
title: "Plan 3, étape 1b.3 : `mz_jj` (enveloppe « trop loin ») — `qs` devient un `const char *`"
description: "`mz_jj` n'a qu'un membre `pico8_value_t`, `qs` (`chaine`, jamais nil, lu par nom) ; le typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 264 à 263 sans qu'un pixel du filet ne bouge. Ni écrivain ni table d'offsets : R5 et R7 restent vacantes pour la troisième fois."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> - 1b.3 — `mz_jj` (`wormhole.h:19`), `qs`, cible de `mz.jj`. Empreinte 4 (2, 2). Compte : 263.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 » de `docs/plan-3.md`. Il est cité ici tel quel et n'est pas redécidé.

### État de départ

HEAD df413f1, avec 1b.2 (`jb`) dans l'arbre de travail : elle est implémentée, en cours de vérification, et pas encore commitée. **Précondition** : 1b.2 est commitée avant que 1b.3 commence. `make value-members` rend alors 264.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H8).

### Ligne de recensement (`tools/member_census.txt:267`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `qs` | offset | `chaine` | jamais | - | l |

Lignes voisines, **non typées ici** : `pico8_mz_t.jj` (`ref:pico8_mz_jj_t`, `:278`) et `pico8_mz_t.et` (`:282`), qui pointent sur l'enveloppe. Ce sont des membres de `mz`, typés à l'étape de `mz`.

### Ce que fait le cart (`pico8/lua.lua`)

La spec de `mz` porte `jj=o(qs="-too far-")` (`:761`). `dl` lit `t=self.et.qs` (`:563`) quand `et` vaut l'enveloppe, et affiche ce texte à la place du sprite.

### Le code aujourd'hui (`src/pico8_wormhole.c/.h`)

- Le typedef est `{ pico8_struct_header_t header; pico8_value_t qs; }` (`wormhole.h:17-20`).
- **Pas de constructeur.** L'unique instance est l'objet statique `g_mz_jj_wrapper` (`wormhole.c:42-45`). Son initialiseur agrégé pose `qs` à `{PICO8_VALUE_STRING, {.string = "-too far-"}}`. `pico8_wormhole_init` le signale au recensement (`PICO8_MEMBER_CENSUS_SEE(&g_mz_jj_wrapper, pico8_mz_jj_t)`, `:405`).
- **Lecteur** `pico8_mz_jj_field` (`:24-32`) : un `strcmp(name, "qs")` puis `*out_value = jj->qs`, sinon `return 0`. Il n'a **ni table d'offsets ni repli**.
- **Écrivain NULL** (`g_mz_jj_struct_class`, `:34-38`) : toute écriture par nom sur l'enveloppe avorte. C'est figé par `test_mz_is_a_struct`.
- `grep -rn 'offsetof(pico8_mz_jj_t' src tests` est vide. `offsetof(pico8_mz_t, jj)` (`:291`) appartient à `mz` et ne change pas.
- `mz_jj` n'a pas de fille. Aucun accès direct `->qs` à l'enveloppe hors du lecteur. Les autres `->qs`/`.qs` (`slipway.c:40`, `jc_too_far` ; `ob.c`, contexte du parseur) sont d'autres structs.
- Lecteurs par nom : `pico8_probe.c:272` (`dl`), `tests/test_pico8_mz.c:324,405,740`. Ils ne changent pas.

### Empreinte remesurée

4, comme la ligne :
- src : `pico8_wormhole.c` (accès direct) et `pico8_probe.c` (par nom) ;
- tests : `test_pico8_mz.c` et `test_pico8_jc.c`. Ce dernier est un majorant : son `"qs"` vise `jc_too_far`.

Fichiers modifiés : `pico8_wormhole.h`, `pico8_wormhole.c`, `tests/test_pico8_mz.c` (caractérisation seule) et `docs/plan-3.md`.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** : `qs` devient `const char *` (R1, `chaine`). Il n'a pas d'indicateur (R2 : pointeur, et nil jamais). L'initialiseur statique pose `"-too far-"` à la place de la valeur emballée (R3). Le lecteur rend `pico8_value_string(jj->qs)`, et `return 0` si `qs` est NULL (R4).
3. Effet mesurable : `make value-members` passe de 264 à **263** et ne liste plus aucune ligne `pico8_mz_jj_t`. Rien d'autre ne bouge.

## User Stories

1. En tant que lecteur de `wormhole.h`, je veux `qs` typé `const char *`, afin que le texte de l'enveloppe se lise sur le type.
2. En tant que `dl` (`probe.c`), je veux lire par nom `qs` présent, de genre chaîne, avec le même pointeur, afin d'afficher « -too far- » comme avant.
3. En tant que `mz`, je veux que mes membres `jj`/`et` pointent sur la même enveloppe, sans changer de forme.
4. En tant que `test_mz_is_a_struct`, je veux qu'une écriture par nom sur l'enveloppe avorte toujours, afin que l'écrivain NULL soit gardé.
5. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
6. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
7. En tant que `plan-pilot`, je veux `make value-members` à 263.
8. En tant qu'auteur de l'étape suivante, je veux savoir que R5 et R7 n'ont toujours pas de forme concrète, et comment R3 se lit sur une instance statique.
9. En tant que vérificateur, je veux une caractérisation par nom, lancée sur l'ancien code, et les mutations du §7 déclinées sur `mz_jj` ou leur trou justifié.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline`, et marqués (G).

### R1–R9 appliquées à `mz_jj` (patron de 1b.1, cité)

| règle | sur `mz_jj` |
|---|---|
| R1 | `qs` → `const char *`, nom et place gardés. C'est la première application de `chaine`. |
| R2 | Pas d'indicateur : c'est un pointeur, et nil n'y est jamais observé. |
| R3 | L'initialiseur de `g_mz_jj_wrapper` devient `{ {PICO8_TYPE_TAG_STRUCT, &g_mz_jj_struct_class}, "-too far-" }`. (G) Sur une instance statique sans constructeur, l'initialiseur agrégé tient lieu de l'affectation d'init. Ni clé de construction ni `pq`. |
| R4 | La branche `qs` du lecteur : si `jj->qs == NULL`, alors `return 0` et `*out_value` reste intact. Sinon `*out_value = pico8_value_string(jj->qs)` et `return 1`. Le pointeur est repassé tel quel : `pico8_value_string` ne copie pas (`value.c:15-20`). Le reste (`return 0` sur tout autre nom) ne change pas. |
| R5 | **Vacante.** L'écrivain reste NULL, et il n'y a aucune clé de construction. |
| R6 | **Vacante.** Aucune méthode ni aucun module n'écrit `qs`. Le seul accès direct est le lecteur (R4). |
| R7 | **Vacante.** Le lecteur fait un `strcmp` sans `offsetof`. La preuve `grep -n 'offsetof(pico8_mz_jj_t, \([a-z0-9_]*\.\)*qs)' src/` est vide avant comme après. Il n'y a pas de fille. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(&g_mz_jj_wrapper, pico8_mz_jj_t)` est gardé. `tools/member_census.txt` n'est pas édité. |
| R9 | 264 → 263, aucune ligne `pico8_mz_jj_t`. |

- **Ni R5 ni R7 ne sont exercées ici.** `mz_jj` n'a ni écrivain (`set_field` NULL) ni table d'offsets (`strcmp` seul). La réserve de la note 1b.1 passe telle quelle à l'étape suivante (G : pas de règle sans site pour l'exercer).
- (G) La branche NULL de R4 est gardée, alors que `qs` n'est jamais NULL (H3). C'est la lecture uniforme de R4 pour un pointeur. La retirer, ou la remplacer par un `assert`, serait une lecture nouvelle de la règle.
- (G) `const char *` et non `char *` : R1 le dit pour `chaine`. La clause « pointeurs non `const` » vise les REF de struct. Le littéral n'est ni possédé ni libéré (TROU-7 ne s'applique pas à une instance statique).
- Aucun site par nom ne change, `dl` compris. Aucun site par nom n'est remplacé par un accès direct.
- Les commentaires du typedef et de l'instance statique restent. Rien n'est ajouté (CLAUDE.md).

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 264 sur l'arbre de départ (1b.2 commitée), puis 263.
- **H2** : `test_pico8_mz` (dans `UNIT_TESTS`, hors `UNIT_TESTS_SLOW`) reste sous 30 s avec la caractérisation.
- **H3** : `qs` n'est jamais NULL : seul l'initialiseur statique l'écrit, et il n'a pas d'écrivain. La branche NULL de R4 n'est jamais exécutée.
- **H4** : `DRAW_HASH` (cas `far`, étiquette « -too far- » de `dl`) et `HQ_HASH` (`fnv_str` de `qs` dans le pli de `hq`) dépendent du `qs` lu par nom. Une lecture absente rougit les deux.
- **H5** : `STRUCT_HASH`/`SPEC_HASH` de `test_pico8_spec_defaults` ne bougent pas. Le lecteur existe déjà, et l'écrivain reste NULL.
- **H6** : `test_pico8_mz` ne fait pas partie des tests de régression de bug du §6. Il n'y reçoit qu'un ajout.
- **H7** : le build `-DPICO8_MEMBER_CENSUS` compile encore. La table générée perd la ligne `pico8_mz_jj_t`, comme `pico8_mh_t` à 1b.1. Ce n'est pas lancé : `make member-census` ne fait pas partie des cibles.
- **H8** : `make class-coverage-net` reste identique à la référence. Il n'est pas lancé, voir « Cibles finales ».

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne fait aucun accès `->qs` et compile sur les deux formes.
- **Caractérisation, figée sur l'ancien code** : `test_jj_qs_by_name`, dans `test_pico8_mz`, sans nouveau TU. Elle est appelée dans `main` comme les autres.
  1. `cls = pico8_mz_jj_struct_class()` : `cls->field != NULL`, `cls->set_field == NULL`.
  2. `out` est posé à un témoin, un fixe `0x1234`. `pico8_entity_get_field(wrapper, "qs", &out)` rend 1, de genre `STRING`, égal à `"-too far-"`. Une seconde lecture, puis la lecture via le `jj` de deux `mz` distincts, rendent le **même pointeur** `as.string`.
  3. `cls->field(wrapper, n, &out)` rend 0 pour `n` ∈ {`"fo"`, `"jj"`, `""`}, et `out` reste égal au témoin.

  Lancée verte sur l'arbre de départ **avant** le typage. Ses valeurs ne changent pas ensuite. L'écran est déjà figé par `DRAW_HASH` et `HQ_HASH` (H4). On n'ajoute pas d'empreinte.
- **Mutations (§7)**, dans une copie jetable du scratchpad. Chacune est notée dans le commit avec le test qui la rougit.
  1. (§7-1) *indicateur forcé vrai* : **sans objet**. `qs` n'a pas d'indicateur et nil n'y est jamais observé. Il n'y a pas de forme proche. Ce trou est justifié dans le commit.
  2. (§7-2) *lecture emballée qui change la présence* : pas de fixe typé ici. On prend la forme inverse, la branche `qs` rend 0. Attendu rouge : `test_construction:324`, `check_jj_untouched`, l'étape 2 de la caractérisation, `DRAW_HASH`, `HQ_HASH`.
  3. (§7-3) *init qui saute un membre typé* : l'initialiseur statique omet `qs`, qui reste NULL. Attendu rouge : les mêmes tests qu'en 2, par la branche NULL.
  4. Propre à l'étape, *valeur du défaut* : l'initialiseur pose `"-too far"`. Attendu rouge : `is_string(…, "-too far-")`, `DRAW_HASH`.
- **`gcov`** (§7, `test_pico8_mz` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23). Chaque ligne modifiée du lecteur est exécutée, sauf le `return 0` sur `qs == NULL`, justifié par H3. L'initialiseur statique n'est pas une ligne exécutable : il est prouvé par la mutation 3. Le rapport va dans le commit.
- **Prior art** : `test_mh_members_by_name` (1b.1), `test_jb_state_by_name` (1b.2), `check_jj_untouched` et `test_mz_is_a_struct` (`test_pico8_mz`).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make sanitize-net` : ~4 min seul, le double sous charge (on le lance seul) ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 263`**, sans aucune ligne `pico8_mz_jj_t`.

`make class-coverage-net` **ne s'applique pas** (§7). `mz_jj` n'a pas de constructeur. Le diff ne touche ni `pico8_mz_struct_new` (où `self->jj = pico8_value_ref(&g_mz_jj_wrapper)` reste tel quel), ni un appel `pico8_class_coverage_record`, ni `KNOWN[]`. Il ne change que l'initialiseur d'un objet statique et un lecteur. S'il touche l'un des trois, la cible redevient obligatoire (sortie identique à `tools/class_coverage_net_expected.txt`).

`make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). L'utilisateur y travaille : ses changements non commités sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si un membre, un indicateur ou un écrivain est ajouté à `mz_jj`, ou si le lecteur accepte un autre nom que `qs`.
- `FAILED:` si un test de régression de bug (§6) change, ou si une valeur figée existante change (`DRAW_HASH`, `HQ_HASH`, les autres `*_HASH` de `test_pico8_mz`, `STRUCT_HASH`, `SPEC_HASH`…). Dans `test_pico8_mz.c`, le diff n'est qu'un ajout.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 263, liste encore `pico8_mz_jj_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si `qs` n'est pas `const char *`, si le lecteur ne rend pas `pico8_value_string` avec le même pointeur, ou si le pointeur NULL n'y rend pas 0 avec `*out_value` intact.
- `FAILED:` si `mz` (`jj`, `et`, `members[]`), `jc_too_far` ou tout autre typedef change de forme, ou si un site par nom change.
- `FAILED:` si le diff touche un constructeur, `pico8_class_coverage_record` ou `KNOWN[]` sans une ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si la caractérisation n'a pas une ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit `->qs`, ou si ses valeurs changent après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 4 mutations, avec leur test ou la justification de leur trou.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.3 », ou si elle écrit une forme concrète pour R5 ou R7.
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- `mz.jj`, `mz.et` et tout autre membre de `mz` (étape de `mz`).
- `jc_too_far.qs` (`slipway.c`), une autre struct.
- Donner une forme concrète à R5 ou à R7.
- Le spec `jj=o(qs="-too far-")` de `g_mz_spec`, qui reste une table lue par `pico8_mz_default`.
- Les fichiers d'événements et `FRAMES_EVENTS`.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `wormhole.h/.c` seuls ;
  - tests : `test_pico8_mz` étendu, sans nouveau TU ;
  - un commit, sans découpage ;
  - `class-coverage-net` non lancé (pas de constructeur) ;
  - R3 lue sur l'initialiseur statique.
- **Note « Découvert à 1b.3 »** à écrire sur la ligne 1b.3 de `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante : « patron 1b.1 appliqué sans écart ; sur une instance statique sans constructeur, R3 se lit sur l'initialiseur agrégé ; ni écrivain ni table d'offsets, R5 et R7 restent à fixer par la première étape qui en a ». Le reste (première application de `chaine`, mutations, comptes) va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le compte 264 → 263 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov` ;
  - les mutations ;
  - les réponses à H1 à H8.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
