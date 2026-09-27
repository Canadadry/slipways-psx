---
title: "Plan 3, étape 1a.37 : le `{}` que `nr:ldown` passe à `nr:gv` devient une struct sans parent ni membre, en stockage statique"
description: "À chaque clic sur une slipgate, `pico8_nr_ldown` fabrique sur la pile une instance de la couche objet sans classe (tag `INSTANCE`, table vide allouée puis libérée) pour la passer à `pico8_nr_gv`, qui n'y lit que `qu` ; elle devient un unique objet statique constant, struct sans parent à en-tête seul, lecteur qui ne trouve jamais rien, sans méthode ni écrivain."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.37 — `{}` passé à `nr:gv` : sans classe (`planet_family_trade.c:94`).

Le numéro de ligne a dérivé. À HEAD bc0551c, le `{}` est construit dans `pico8_nr_ldown`, `planet_family_trade.c:95-100` :

- `:95` `empty_fields = pico8_ob_table_new()` (table vide, `malloc`) ;
- `:96-98` une `pico8_instance_t empty` **sur la pile** : tag `INSTANCE`, `fields` = cette table, `cls` NULL ;
- `:99` `gv_result = pico8_nr_gv(self, &empty)` ;
- `:100` `pico8_ob_table_free(empty_fields)`.

C'est le port littéral de `pico8/lua.lua:1112-1113` : `function nr:ldown() return self:gv({})and jc({qu=self})end`, avec `nr:gv(s)` = `return s.qu~=self and #self.oe<3` (`:1109-1111`).

Règles du §5 qui s'appliquent :

> Chaque classe devient un `struct` […] ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. […]

§6 : aucun test de régression n'est touché par cette étape (voir plus bas). §7 : un seul commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ; preuve par `gcov` et 2 à 3 mutations ; note « Découvert à 1a.37 » réduite à ce qui change une étape suivante ; un écart port/cart découvert est préservé par un `TROU-N`, jamais corrigé.

### Précédents réutilisés tels quels

- **`je` (1a.7, 4b257ee) et `mo` (1a.9, a1a9d2a)** : struct privée à en-tête `pico8_struct_header_t` seul, sans `pico8_ke_t` ; descripteur `static const` sans méthode, lecteur qui rend 0 pour tout nom, écrivain NULL ; un unique objet statique initialisé à la compilation, jamais alloué ni libéré ; type du slot conservé par un cast. C'est exactement la forme qu'il faut ici : aucun champ n'est écrit sur le `{}`, aucun défaut.
- **`mz.jj` (1a.11, d85626d), `ie.ke` (1a.15), enveloppe de `pz` sur `pj.cr` (1a.30, c1da235), `fm` provisoire (1a.36, 7ab4129)** : instances sans classe converties en objet statique unique, même quand le cart en fabrique une par appel, dès que rien ne compare leur identité ni ne les écrit.

### Emprise mesurée (HEAD bc0551c)

**Méthode.** Grep de `src/` et `tests/`, puis sonde sur une copie `git archive HEAD` jetable du scratchpad (supprimée après usage) : `pico8_nr_ldown` journalise le résultat de `gv` sur le `{}`, `pico8_nr_gv` journalise le tag de chaque `s` non NULL. Rejoués : tous les binaires de `make unit` (verts, hors `score_panel_stars`/`planet_gh_row_lq` qui exigent leurs arguments/`ASAN_OPTIONS`, sans rapport), les 8 de `UNIT_TESTS_SLOW` (verts), et `pico8_replay_e2e` sur chaque `tests/fixtures/*events*.json`, avec et sans `--from-title` (hors les 5 `events_malformed_*`, qui avortent par conception).

| Aspect | Mesure |
|---|---|
| Construction | 1 site : `pico8_nr_ldown` (`:95-98`), une fois par appel. Pas de classe, pas de `pico8_class_instantiate`, pas de `lt` : jamais dans `hv`/`cv`/`cg`, jamais compté par le crochet de couverture. |
| Qui appelle `pico8_nr_ldown` | seulement `pico8_nr_ldown_dispatch` (entrée `LDOWN` des descripteurs de `nr` et `fr`), atteint par le clic (`ui`, `bt.."down"`) et par les tests qui appellent `ldown` par le descripteur. |
| Où il va | **seulement** en argument `s` de l'appel direct `pico8_nr_gv(self, &empty)`. Il n'est ni rendu, ni rangé, ni passé ailleurs. |
| Lecteurs | un seul : `pico8_nr_gv` lit `qu` par `pico8_entity_get_field` (`:62-64`). Toujours absent, donc `qu_not_self` vrai, donc `gv` = `#oe<3`. Aucun autre nom lu. `jo:gv` (`planet_display.c`) ne le reçoit jamais (l'appel est direct, pas par `self:gv`). |
| Écrivains | aucun. |
| Méthodes | aucune lue. |
| Identité | jamais comparée : `nr:gv` compare `s.qu` à `self`, jamais `s` à quoi que ce soit. |
| Vie / propriété | pile de `pico8_nr_ldown` ; la table est allouée à `:95` et libérée à `:100`, avant tout retour. Aucune fuite, aucun pointeur conservé (contrat de `pico8_nr_gv` dans le `.h` : « never stores `s` »). |
| `s` reçu par `pico8_nr_gv` | le `{}` (`INSTANCE`) est **le seul `s` non-struct jamais observé** : dans chaque exécution, compte « `gv` sur `INSTANCE` » = compte de `ldown`. Les autres `s` sont des structs (le `jc` tenu, par `jz("gv",held)` dans `ui:bh`, et `mz`/`bg` passés directement par `test_pico8_mz:726` et `test_pico8_bg:506`) ou NULL (`test_pico8_nr`, `test_pico8_fr`). |
| Filet | `session_events.json --from-title` : 15 `ldown`, tous acceptés ; `trading_hub_session_events.json` (avec et sans `--from-title`) : 2, acceptés. Aucun autre rejeu du filet n'atteint `nr:ldown`. Le refus (`#oe>=3`) n'est **jamais** sur le filet. |
| Tests | `ldown` accepté : `fr` 3, `jc` 2, `nl` 2, `trade` 2, `ie` 2, `jo` 17, `qh` 17, `pm` 15, `nr` 18. **Refusé** : `fr` 1, `nr` 1 (`test_life`, `oe` = 3 liens). `fr`, `jc`, `nl`, `trade` sont dans `make test` ; `nr`, `ie`, `jo`, `qh`, `pm` dans `UNIT_TESTS_SLOW`. |
| Couverture | aucun nom : absent de `KNOWN[]` et de `tools/class_coverage_net_expected.txt`, qui ne doivent pas bouger. |
| Écart port/cart | aucun : `qu_not_self && oe_count<3` rend `false` ou le booléen de `#oe<3`, comme `s.qu~=self and #self.oe<3`. Pas de `TROU` nouveau. |

### Ce que les tests figent déjà de ce chemin

`test_pico8_nr` (`test_life`, `:494-509`, `LIFE_HASH`) et `test_pico8_fr` (`test_life`, `:461-476`) appellent `ldown` par le descripteur sur une slipgate/porte posée : sans lien, ils assertent un `jc` non NULL dont `qu` est la porte ; avec 3 liens, ils plient le `false` rendu dans leur empreinte. `test_pico8_jc`, `test_pico8_nl`, `test_pico8_trade` construisent leurs voies par `ldown` et plient le résultat dans des empreintes figées.

**Preuve qu'ils gardent le `{}`** (mesurée sur l'ancien code, copie jetable) : mutation « le `{}` porte `qu` = `self` » (une ligne `pico8_ob_table_set_name(empty_fields,"qu",self)`) → `ldown` refuse toujours → **rouges** : `test_pico8_fr` (`:462`, assert `jc` non NULL), `test_pico8_nr` (`:495`), `test_pico8_jc` (`:337`, empreinte), `test_pico8_trade` (`:160`), `test_pico8_nl` (`:215`). Le refus reste gardé par `LIFE_HASH` des tests `nr`/`fr`. Un lecteur absent ou qui asserte sur `qu` avorterait dans tous les mêmes tests.

Ces tests sont donc réutilisés comme caractérisation (voir Testing Decisions) : aucun nouveau fichier de test.

## Solution

1. **Struct sans parent, en-tête seul** : `pico8_struct_header_t` et rien d'autre. Aucun membre : aucun champ n'est écrit sur le `{}` ni défini par défaut ; `qu` est **lu**, pas écrit, et le §5 ne fait membre que les défauts et les champs écrits (précédent `je`/`mo`). Pas de `pico8_ke_t`.
2. **Descripteur `static const`** : aucune méthode ; un lecteur qui rend 0 pour tout nom (« absent », comme une table vide) ; écrivain NULL.
3. **Un unique objet `static const`**, tag `STRUCT` et descripteur fixés à la compilation, privé à `pico8_planet_family_trade.c`. `pico8_nr_ldown` lui passe son adresse (cast vers le type du paramètre) au lieu de la pile + `pico8_ob_table_new`/`free`. Plus aucune allocation par clic.
4. **`pico8_nr_gv`** : inchangé dans son corps (il lit `qu` par `pico8_entity_get_field`, qui aiguille déjà sur le tag, et reçoit aussi des structs `jc`/`mz`/`bg`) et dans sa signature (`const pico8_instance_t *s`, type qui tombe à 1a.42).
5. **Contrat d'en-tête corrigé**, le seul que ce commit rend faux : `pico8_nr_gv` dans `pico8_planet_family_trade.h` — « `nr:ldown` passes a stack instance it frees as soon as the call returns » devient : `nr:ldown` passe une struct statique sans champ. « Never stores `s`, which may be NULL » reste.
6. **Plan** : à la ligne 1a.37 de `docs/plan-3.md`, corriger le site (`planet_family_trade.c:95`) et ajouter « Découvert à 1a.37 : » réduit à ce qui change une étape suivante (§7) — a priori : aucune entrée d'enum, `pico8_nr_gv` reste lu par `pico8_entity_get_field` (son `s` est aussi un `jc`, un `mz` ou un `bg`), le type `pico8_instance_t *` du paramètre tombe à 1a.42. Sites, comptes et mesures vont dans le message de commit.
7. **Rien d'autre** : `nr`, `fr`, `jc`, `jo:gv`, l'enum de méthodes, `KNOWN[]`, les includes encore utilisés (`pico8_ob.h`, `pico8_class.h` servent à `jc({qu=self})`) ne bougent pas.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que le `{}` de `nr:ldown` soit la seule chose convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux l'emprise du `{}` mesurée (construction, destination, lecteurs, écrivains, vie, filet, tests), afin que l'absence de tout membre soit justifiée par une mesure.
3. En tant que lecteur du plan, je veux que le `{}` devienne une struct sans parent, afin d'appliquer « converties comme des classes à part entière, sans parent » à la lettre.
4. En tant que mainteneur, je veux que la struct n'ait aucun membre, afin de ne pas inventer un `qu` que le cart ne pose jamais sur ce `{}`.
5. En tant que joueur qui clique sur une slipgate à moins de 3 liens, je veux qu'une slipway parte toujours de cette slipgate, afin que la conversion ne change rien.
6. En tant que joueur qui clique sur une slipgate à 3 liens, je veux que le clic soit toujours refusé, afin que la conversion ne change rien.
7. En tant que joueur qui clique sur une porte infraspace (`fr`), je veux le même comportement, puisque `fr` hérite de `nr:ldown`.
8. En tant que gardien du filet, je veux que la lecture de `qu` sur le `{}` rende toujours « absent », afin que `nr:gv` compare comme aujourd'hui.
9. En tant que gardien du filet, je veux que le `{}` n'ait aucune méthode, afin qu'une lecture de `gv`/`ldown`/… sur lui rende « absent » comme sur une table vide.
10. En tant que mainteneur, je veux que le `{}` n'ait pas d'écrivain générique, afin qu'une écriture jamais mesurée arrête le programme au lieu d'être rangée en silence.
11. En tant que gardien de la mémoire, je veux que le `{}` ne soit ni alloué ni libéré, afin que chaque clic sur une slipgate cesse d'allouer une table.
12. En tant que gardien de la mémoire, je veux que l'objet soit `const`, afin qu'aucune écriture ne puisse faire survivre un état d'un clic au suivant (le cart fabrique un `{}` neuf à chaque appel).
13. En tant que mainteneur, je veux que `pico8_nr_gv` garde sa signature et son corps, afin que ses autres appelants (le `jc` tenu par `ui`, `test_pico8_mz`, `test_pico8_bg`) ne soient pas touchés.
14. En tant que gardien de la couverture, je veux qu'aucun nom ne soit ajouté à `KNOWN[]` ni compté, afin que `make class-coverage-net` reste identique à la référence.
15. En tant que gardien des tests, je veux que `test_pico8_nr`, `test_pico8_fr`, `test_pico8_jc`, `test_pico8_nl`, `test_pico8_trade` passent avec un diff vide, puisqu'ils figent déjà les deux issues de `ldown`.
16. En tant que vérificateur, je veux la preuve qu'une mutation de la conversion (lecteur qui sert `qu` = `self`) rend ces tests rouges, afin de juger que leur réutilisation comme caractérisation est fondée.
17. En tant que vérificateur, je veux un rapport `gcov` qui montre chaque ligne ajoutée ou modifiée exécutée, plus 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
18. En tant que gardien du filet, je veux qu'aucun `.sha256` ni aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
19. En tant qu'auteur des étapes 1a.38 à 1a.42, je veux que `et={}`, `{qs="-too far-"}`, `re={ra,p}`, l'enveloppe de `g_ha` et les `pico8_class_t` restent intacts, afin que leurs étapes restent séparées.
20. En tant que lecteur du code, je veux que le contrat de `pico8_nr_gv` dans l'en-tête dise vrai après ce commit (CLAUDE.md).
21. En tant que lecteur du code, je veux qu'aucun commentaire de récit de chantier ne soit ajouté (règle des quatre destinations).
22. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.

## Implementation Decisions

Pas d'outil pour lancer un agent `guideline` frais dans cette session : j'ai appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Décisions tranchées ainsi marquées (G).

- **En-tête seul, aucun membre** (G). `qu` est seulement lu ; le §5 range en membre les défauts et les champs écrits, rien d'autre. Rejeté : un membre `qu` nil — un membre que rien n'écrit, sans règle, et que le lecteur servirait comme absent de toute façon.
- **Objet statique `const`, pas pile** (G). La pile marcherait aussi (le `{}` ne sort pas de l'appel), mais le statique suit tous les précédents (`je`, `mo`, `mz.jj`, `fm`), n'a pas d'état et supprime la table allouée à chaque clic. `const` parce que rien ne l'écrit et que le cart en fabrique un neuf par appel : aucun état ne doit survivre d'un clic à l'autre. Si `const` bute sur un cast vers le type du paramètre, un `static` non `const` est acceptable (précédent `je`), sans écrivain.
- **Lecteur qui rend 0 pour tout nom, sans assert** (G, précédent `je`/`mo`). Le seul nom lu est `qu`, et `nr:gv` tolère son absence. Le lecteur ne peut pas être NULL : `pico8_entity_get_field` l'asserte.
- **Écrivain NULL, aucune méthode** : mesuré, aucun écrivain ni appel de méthode ne l'atteint.
- **Pas de passage de NULL à la place** (G). `pico8_nr_gv(self, NULL)` rendrait le même résultat (mutant équivalent), mais le cart passe une table, pas nil, et la ligne dit « sans classe », pas « supprimée ».
- **Pas d'accesseur public du descripteur** (G). À la différence de `je`/`mo`/`fm`, l'objet ne sort jamais de `pico8_nr_ldown` : aucun slot, aucun lecteur hors de ce fichier, aucun test ne peut l'atteindre sans un accesseur ajouté pour lui seul. Sa forme est prouvée par `gcov` (lecteur exécuté à chaque `ldown`) et par les mutations ci-dessous. La décision est rapportée dans le message de commit.
- **Nom** : le cart n'a pas de nom pour ce `{}`. Nom C descriptif rattaché à `nr:gv` (ex. « table vide de `nr:ldown` »), au choix de l'implémenteur, sans inventer de nom minifié ni d'entrée de glossaire.
- **Signature de `pico8_nr_gv` conservée** (`const pico8_instance_t *s`) : son `s` est aussi un `jc`, un `mz`, un `bg` ou NULL ; le type disparaît à 1a.42 avec les autres `pico8_instance_t *`.
- **Écarts port/cart** : aucun connu. Un écart découvert est préservé et décrit par `TROU-N`.
- **Commentaires** (CLAUDE.md) : au plus un contrat C sur l'objet (statique, jamais libéré, sans champ) ; le contrat de `pico8_nr_gv` corrigé ; tout le reste dans le message de commit.

## Testing Decisions

- **Caractérisation : réutilisation, pas de nouveau fichier.** Justification mesurée ci-dessus : `test_pico8_fr` (rapide, dans `make test`) et `test_pico8_nr` (`UNIT_TESTS_SLOW`) figent les deux issues de `ldown` (accepté : `jc` dont `qu` est la porte ; refusé à 3 liens : `false` plié dans `LIFE_HASH`) ; `test_pico8_jc`, `test_pico8_nl`, `test_pico8_trade` figent en plus des voies construites par `ldown`. La mutation « le `{}` voit `qu` = `self` » les rend tous rouges sur l'ancien code. Ce qui est observable du `{}` se réduit à « `qu` n'est pas `self` » : il n'y a rien d'autre à figer. **Aucune valeur figée ni empreinte (`LIFE_HASH`, etc.) n'est modifiée** ; diff vide sur ces cinq fichiers.
- **Si l'implémenteur juge nécessaire un test dédié**, il est écrit et vu vert **sur l'ancien code d'abord**, puis gardé inchangé ; il appelle `ldown` par le descripteur (0 à 3 liens, porte `nr` puis `fr`) et ne teste que le comportement externe.
- **`gcov`** (test compilé en `--coverage`, copie jetable du scratchpad, supprimée après) sur `test_pico8_fr` : chaque ligne ajoutée ou modifiée de `pico8_planet_family_trade.c` exécutée (le lecteur, l'appel de `pico8_nr_gv` avec l'objet statique).
- **Mutations ciblées sur le nouveau code** (au moins deux, rapportées dans le message de commit) :
  1. lecteur qui sert `qu` = l'argument `self` de `ldown` (ou tout lecteur qui rend présent un `qu` égal à la porte) → `test_pico8_fr`/`nr`/`jc`/`nl`/`trade` rouges ;
  2. descripteur au lecteur NULL → abort de `pico8_entity_get_field` dans les mêmes tests ;
  3. mutants équivalents, nommés comme tels : passer NULL au lieu de l'objet ; lecteur qui rend un `qu` présent nil ou différent de `self`.
- **Tests de régression du §6** : aucun n'est touché par ce chemin (la ligne du plan n'en nomme aucun) ; aucune réécriture, aucune preuve rouge supplémentaire exigée.
- **Commandes** : `timeout 10m make test` ; `timeout 15m make e2e` (filet + `UNIT_TESTS_SLOW`, dont `test_pico8_nr`) ; `timeout 15m make class-coverage-net`, sortie identique à `tools/class_coverage_net_expected.txt`. Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- le filet reste rouge après 3 tentatives de correction sur le même commit ;
- le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS`/`RECORDINGS` ;
- `make class-coverage-net` n'est pas identique à la référence, ou un nom est ajouté à `KNOWN[]` ;
- un champ ne tombe sous aucune règle du §5 (par exemple, un écrivain découvert sur le `{}`) ;
- un test de régression devrait changer d'intention pour passer ;
- une autre classe ou instance sans classe est convertie (`et={}`, `{qs="-too far-"}`, `re={ra,p}`, enveloppe de `g_ha`, `jc`, …) ;
- une valeur ou empreinte figée de `test_pico8_nr`/`fr`/`jc`/`nl`/`trade` est éditée ;
- un écart port/cart est corrigé au lieu d'être préservé.

## Out of Scope

- 1a.38 (`et={}`, `probe.c`), 1a.39 (`{qs="-too far-"}`), 1a.40 (`re={ra,p}`), 1a.41 (enveloppe de `pz()` sur `g_ha`), 1a.42 (suppression des `pico8_class_t`, du type `pico8_instance_t *` des paramètres et slots).
- La construction du `jc` par `pico8_class_lookup_named("jc")` + `pico8_class_instantiate` dans `pico8_nr_ldown` (le `{qu=self}` passé à `jc`) : ce sont des surcharges de constructeur d'une classe déjà convertie, pas l'instance de cette ligne.
- L'appel direct `pico8_nr_gv` au lieu de `self:gv` par le descripteur (équivalent : `fr` ne surcharge pas `gv`).
- Tout changement de rendu ou de comportement ; l'audio.

## Further Notes

- Étape d'un seul commit, sans morceaux.
- Mesures faites à HEAD bc0551c sur une copie jetable supprimée ensuite ; les numéros de ligne cités sont ceux de ce HEAD.
- Le message de commit porte : sites, comptes de la sonde, mutations tuées et équivalentes, rapport `gcov`, décision « pas d'accesseur », « Découvert à 1a.37 ».
