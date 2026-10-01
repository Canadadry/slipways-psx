---
title: "Plan 3, étape 1b.2 : `jb` (bulle d'info) — `ls` devient un `pico8_list_t *`"
description: "`jb` n'a qu'un membre `pico8_value_t`, `ls` (`ref:liste`, nil observé par grep, jamais lu par nom) ; le typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 265 à 264 sans qu'un pixel du filet ne bouge. Ni écrivain ni table d'offsets : R5 et R7 restent vacantes, et leur première forme concrète reste à écrire par une étape suivante."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> - 1b.2 — `jb` (`layout.h:16`), `ls`, cible de `ie.jb`/`js.jb`. Empreinte 2 (1, 1). Compte : 264.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 » de `docs/plan-3.md`. Il est cité ici tel quel et n'est pas redécidé.

### État de départ

HEAD 85c4534, avec 1b.1 (`mh`) dans l'arbre de travail : elle est implémentée, en cours de vérification, et pas encore commitée. **Précondition** : 1b.1 est commitée avant que 1b.2 commence. `make value-members` rend alors 265.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H7).

### Ligne de recensement (`tools/member_census.txt:43`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `ls` | offset | `ref:liste` | observé | stat | - |

La preuve `stat` est le test `self->ls.as.ref == NULL` de `pico8_jb_ol` (`layout.c:304`). `ls` n'est jamais lu ni écrit par nom.

### Ce que fait le cart (`pico8/lua.lua:314-322`)

`jb:pq` pose `self.ls = nv(self, …)` : une `lq` par positionnel (une chaîne nue devient `{ln}`). `jb:ol` dessine un cadre de hauteur `#self*8+2`, puis chaque `l:ol(...)`.

### Le code aujourd'hui (`src/pico8_layout.c/.h`)

- Le typedef est `{ pico8_struct_header_t header; pico8_value_t ls; }` (`layout.h:14-17`).
- Le constructeur `pico8_jb_struct_new` (`:462-480`) enchaîne :
  1. la couverture ;
  2. un assert « tout override est positionnel » ;
  3. `self->ls = pico8_value_ref(NULL)` ;
  4. `pico8_jb_pq` (`:284-293`), qui fait `self->ls = pico8_value_ref(ls_heap)`, où `ls_heap` est toujours non NULL ;
  5. `PICO8_MEMBER_CENSUS_SEE`.
- `pico8_jb_ol` (`:297-330`) teste `ls` puis le lit par `PICO8_REF_AS_LIST_CONST(self->ls.as.ref)`. C'est un site de `pico8_ref_audit.h`, qui disparaît (§5, REF).
- Le descripteur `g_jb_struct_class` (`:456-460`) porte `OL` seul, et un **lecteur NULL** et un **écrivain NULL**. `pico8_entity_get_field` n'accepte sur un `jb` que le nom de méthode `ol`. Tout autre nom avorte (`entity.c:186`).
- `grep -rn 'offsetof(pico8_jb_t' src tests` est vide.
- Hors de `layout.c`, un seul accès direct : `tests/test_pico8_jb_ol.c:94-95` (`direct->ls.kind`, `direct->ls.as.ref`).
- `ie.jb`/`js.jb` (`research_icon.c:241,276` ; `ui_button.c:106,278,310`) sont des membres de `ie`/`js`, typés à 1b.20/1b.34. Ils ne changent pas ici. Leur cast `(const pico8_jb_t *)…as.ref` reste valide.

### Empreinte remesurée

2 : `src/pico8_layout.c` et `tests/test_pico8_jb_ol.c`. S'y ajoute `src/pico8_layout.h` (le typedef ; le grep de l'empreinte ne compte que les accès). Fichiers modifiés : ces 3 fichiers, plus `docs/plan-3.md`.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** : `ls` devient `pico8_list_t *` (R1, `ref:liste`). Il n'a pas d'indicateur (R2 : pointeur, NULL = nil). Le constructeur pose `self->ls = NULL` à la place de `pico8_value_ref(NULL)` (R3). `jb:pq` pose `self->ls = ls_heap` (R6). `jb:ol` teste `self->ls == NULL` et lit `self->ls` sans passer par `PICO8_REF_AS_LIST_CONST`.
3. Effet mesurable : `make value-members` passe de 265 à **264** et ne liste plus aucune ligne `pico8_jb_t`. Rien d'autre ne bouge.

## User Stories

1. En tant que lecteur de `layout.c`, je veux `ls` typé `pico8_list_t *`, afin que « la bulle a ses rangées » se lise sur le type, sans cast audité.
2. En tant que `jb:ol`, je veux ne rien dessiner quand `ls` est NULL et dessiner les mêmes pixels sinon, afin que les infobulles de `js` et de `ie` soient identiques.
3. En tant que `ie`/`js`, je veux que le `pico8_jb_t *` que je tiens se dessine comme avant, sans changer mon propre membre `jb`.
4. En tant que `test_pico8_spec_defaults`, je veux que `jb` reste une struct sans lecteur, afin que `STRUCT_HASH` ne bouge pas.
5. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
6. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
7. En tant que `plan-pilot`, je veux `make value-members` à 264.
8. En tant qu'auteur de l'étape suivante, je veux savoir que R5 et R7 n'ont toujours pas de forme concrète, afin que la première struct à écrivain ou à table d'offsets l'écrive.
9. En tant que vérificateur, je veux une caractérisation par nom, lancée sur l'ancien code, et les mutations du §7 déclinées sur `jb` ou leur trou justifié.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline`, et marqués (G).

### R1–R9 appliquées à `jb` (patron de 1b.1, cité)

| règle | sur `jb` |
|---|---|
| R1 | `ls` → `pico8_list_t *`, non `const`, nom et place gardés. |
| R2 | Pas d'indicateur : c'est un pointeur, et il n'est pas lu par nom. |
| R3 | `self->ls = NULL;` à la place de `self->ls = pico8_value_ref(NULL);`, avant `jb:pq`. |
| R4 | **Vacante.** Le lecteur reste NULL. (G) Aucun lecteur n'est ajouté : il n'aurait pas d'appelant, et il changerait `STRUCT_HASH` (`readable`). |
| R5 | **Vacante.** L'écrivain reste NULL, et il n'y a aucune clé de construction nommée (l'assert existant la refuse). Les positionnels vont à `jb:pq`, pas dans un membre. |
| R6 | Dans `jb:pq`, `self->ls = pico8_value_ref(ls_heap)` devient `self->ls = ls_heap`. Dans `jb:ol`, le test `kind`/`as.ref == NULL` devient `self->ls == NULL`, et `ls = self->ls` remplace `PICO8_REF_AS_LIST_CONST(...)`. |
| R7 | **Vacante.** Il n'y a aucune table d'offsets sur `jb`. La preuve `grep -n 'offsetof(pico8_jb_t, \([a-z0-9_]*\.\)*ls)' src/` est vide avant comme après. `jb` n'a pas de fille. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_jb_t)` est gardé. `tools/member_census.txt` n'est pas édité. |
| R9 | 265 → 264, aucune ligne `pico8_jb_t`. |

- **R5 et R7 restent sans forme concrète.** La note de 1b.1 réserve cette forme à « la première étape dont la struct a un écrivain », respectivement « une table d'offsets ». `jb` n'a ni l'un ni l'autre. Ce PRD ne l'invente pas (G : pas de règle sans site pour l'exercer), et la réserve passe telle quelle aux étapes suivantes.
- Le contrat du header est gardé : « A NULL self, or one whose `ls` is nil, draws nothing ». « nil » se lit désormais NULL. (G) La branche est gardée même si `ls` n'est jamais NULL après construction (H3) : elle porte la preuve `stat`, et la retirer serait un changement de comportement hors plan.
- Aucun site par nom ne change. Aucun n'existe sur `ls`, et `mv(jb,"ol")` passe par la table de méthodes.
- `tests/test_pico8_jb_ol.c:94-95` lit `ls` directement. (G) Seule la façon de lire change : l'assert lit `direct->ls != NULL && pico8_type_tag_of(direct->ls) == PICO8_TYPE_TAG_LIST && direct->ls->count == 0`. L'intention (liste vide, étiquetée liste) et les 3 empreintes restent identiques. Ce n'est pas un test de régression de bug du §6 (H6).
- Commentaires : le commentaire TROU-7 du typedef reste. Rien n'est ajouté (CLAUDE.md).

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 265 sur l'arbre de départ (1b.1 commitée), puis 264.
- **H2** : `test_pico8_jb_ol` avec la caractérisation dure moins de 30 s.
- **H3** : `ls` n'est NULL sur aucun `jb` construit : `jb:pq` pose toujours un `malloc` non NULL. La branche NULL de `jb:ol` n'est atteinte que par `self == NULL`.
- **H4** : le binaire de `test_pico8_jb_ol` (`DATA_ENGINE_SRCS`) lie `pico8_entity.c`, ce qui rend `pico8_mv` et `pico8_entity_get_field` disponibles. Sinon, la règle Makefile du test ajoute l'objet ; ce n'est pas une cible du filet.
- **H5** : `STRUCT_HASH`, `SPEC_HASH` et les 3 empreintes de `test_pico8_jb_ol` ne bougent pas.
- **H6** : `test_pico8_jb_ol` ne fait pas partie des 17 tests de régression de bug du §6. Il fige des pixels.
- **H7** : `make class-coverage-net` rend `jb  831  15/15`, identique à la référence.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** Sur `jb`, le seul accès par nom est la méthode `ol`. La caractérisation lit donc l'état par nom via `pico8_entity_get_field(jb, "ol")`, puis `pico8_mv(jb, "ol", 2, om, a)`, et plie les pixels. Elle ne fait aucun accès `->ls` et doit compiler sur les deux formes.
- **Caractérisation, figée sur l'ancien code** : `test_jb_state_by_name`, dans `test_pico8_jb_ol`, sans nouveau TU.
  1. `get_field(jb, "ol")` est présent, de genre `FUNCTION`, et égal à `cls->methods[PICO8_METHOD_OL]`. `cls->field` et `cls->set_field` sont NULL.
  2. Construction des 7 `jb(dr[i])` de `cart.c:224-230`. Pour chacun, `pico8_mv(…, "ol", 2, 127, 1)` sur un écran effacé, et l'empreinte de l'écran pliée dans `DR_ALL_BY_NAME_HASH`.
  3. Une ligne mêlée (chaîne nue, table REF, fixe → `lq` vide, trou d'index), dessinée par `mv` à `(0, 0.5)`, et figée dans `MIXED_BY_NAME_HASH`.
  4. Pour chaque cas, l'écran est différent de l'écran vide.

  Lancée verte sur l'arbre de départ **avant** le typage. Ses valeurs ne changent pas ensuite.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune notée dans le commit avec le test qui la rougit :
  1. (§7-1) *indicateur forcé vrai* : sans objet, `ls` n'a pas d'indicateur. La forme la plus proche, retirer la garde `ls == NULL` de `jb:ol`, n'est pas détectable (H3). Ce trou est justifié dans le commit.
  2. (§7-2) *« présent 0 » là où c'était absent* : sans objet. Il n'y a ni lecteur ni fixe typé, et c'est justifié.
  3. (§7-3) *init qui saute un membre typé* : `jb:pq` ne pose pas `self->ls`, qui reste NULL. Attendu rouge : les étapes 2 et 3 (écran vide) et les 3 tests existants.
  4. Propre à l'étape : `jb:ol` itère un `ls` décalé d'une entrée (`count - 1`). Attendu rouge : `DR_ALL_BY_NAME_HASH`, `DR_FIRST_ENTRY_HASH`.
- **`gcov`** (§7, `test_pico8_jb_ol` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23) : chaque ligne modifiée de `layout.c` est exécutée, sauf `return` sur `ls == NULL`, justifié par H3. Le rapport va dans le commit.
- **Prior art** : les tests existants de `test_pico8_jb_ol` (empreintes d'écran) et la caractérisation de 1b.1 (`test_mh_members_by_name`).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur : `make test` 20 s à 1 min ; `make e2e` 4 à 6 min ; `make sanitize-net` ~4 min seul (le double sous charge : on le lance seul) ; `make class-coverage-net` 3 à 4 min ; `make value-members` quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 264`**, sans aucune ligne `pico8_jb_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**. Le diff touche le corps du constructeur `pico8_jb_struct_new` (l'affectation typée de R3), même si `pico8_class_coverage_record` et `KNOWN[]` ne bougent pas (§7, même lecture qu'à 1b.1). La sortie doit être identique à `tools/class_coverage_net_expected.txt`.

`make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si une entrée de `FRAMES_EVENTS`/`RECORDINGS` est retirée.
- `FAILED:` (« champ sans règle ») si un membre, un indicateur, un lecteur ou un écrivain est ajouté à `jb`, ou si une clé de construction nommée est acceptée.
- `FAILED:` si un test de régression de bug (§6) change, ou si une valeur figée existante change (`DR_FIRST_ENTRY_HASH`, `BARE_STRINGS_HASH`, `NO_OVERRIDES_HASH`, `STRUCT_HASH`, `SPEC_HASH`…). Dans `test_pico8_jb_ol.c`, seule la lecture de `ls` des lignes 94-95 peut changer.
- `FAILED:` si `make class-coverage-net` n'est pas identique à la référence, ou si cette référence, les exemptions ou `KNOWN[]` changent.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 264, liste encore `pico8_jb_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si `ls` n'est pas `pico8_list_t *`, si `jb:ol` ne dessine plus rien sur `ls == NULL`, ou si `PICO8_REF_AS_LIST_CONST` reste sur `ls`.
- `FAILED:` si `ie`, `js`, `lq` ou tout autre typedef change de forme, ou si un site par nom change.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si la caractérisation n'a pas une ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit `->ls`, ou si ses valeurs changent après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 4 mutations, avec leur test ou la justification de leur trou.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.2 », ou si elle écrit une forme concrète pour R5 ou R7.
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- `ie.jb` (1b.20), `js.jb` (1b.34), `lq` (1b.10), dans le même fichier.
- Donner une forme concrète à R5 ou à R7.
- Libérer `jb` ou `ls` (TROU-7) ; TROU-23.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `layout.h/.c` seuls ;
  - tests : `test_pico8_jb_ol` étendu, sans nouveau TU ;
  - un commit, sans découpage.
- **Note « Découvert à 1b.2 »** à écrire sur la ligne 1b.2 de `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante : « patron 1b.1 appliqué sans écart ; ni écrivain ni table d'offsets, R5 et R7 restent à fixer par la première étape qui en a ». Le reste (site `PICO8_REF_AS_LIST_CONST` retiré, lecture de `test_pico8_jb_ol.c:94` adaptée) va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le compte 265 → 264 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov` ;
  - les mutations ;
  - les réponses à H1 à H7.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
