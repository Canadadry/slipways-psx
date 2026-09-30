---
title: "Plan 3, étape 2.3 (a, b, c) : `me` et `t` montent dans l'en-tête, qui devient le noyau ; `pico8_ke_t` fond puis disparaît (`inclusion-mutuelle` 18/45 → 17/43, `parente` 26 → 12 → 10) — trois commits, jeu identique"
description: "Le noyau d'entité doit être l'en-tête `pico8_struct_header_t` lui-même, pas une struct `ke` qui l'enveloppe : `me` et `t` y montent, les six casts non vérifiés vers `pico8_ke_t` deviennent des lectures directes, puis les quatorze racines et les deux racines à descendance embarquent l'en-tête directement, jusqu'à ce que `pico8_ke_t` n'existe plus."
status: needs-triage
---

## Problem Statement

**Écrit sur `ee2d00f`** (branche `claude/plan-pilot-3-7zdft0`). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les comptes et les preuves viennent d'essais sur quatre copies jetables de l'arbre (`git archive HEAD`, puis 2.3a, 2.3b, 2.3c appliquées l'une sur l'autre), supprimées depuis. Les numéros de ligne sont ceux de `ee2d00f`, sauf mention. L'implémenteur remesure tout sur le vrai arbre. Un écart s'explique dans le message de commit.

Vu du mainteneur du port : la phase 2 veut un noyau d'entité commun (`docs/plan-3.md`, « Phase 2 — Composition »). Aujourd'hui, ce noyau est coupé en deux :

- `pico8_struct_header_t` (`struct.h:44-46`) ne porte que `cls` ;
- `pico8_ke_t` (`ke.h:9-13`) l'enveloppe et y ajoute `me` (l'état courant) et `t` (le compteur de ticks).

Conséquences :

1. **Six lectures passent par un cast non vérifié** vers `pico8_ke_t` : `ke.c:30-31`, `probe.c:230`, `:367`, `entity_geom.c:84`, `bootstrap.c:58`. Elles supposent que le pointeur reçu est celui d'une struct qui commence par un `pico8_ke_t`. Rien ne le vérifie.
2. **`ke` ne peut pas devenir un alias de l'en-tête** : le lecteur de structs du compteur (`composition_residue.sh:188`, `:238`) part de `pico8_struct_header_t` ; un `typedef pico8_struct_header_t pico8_ke_t;` sortirait `jo` et tout son arbre de la fermeture, et `parente`, `ptrs-entité` et `--champs` compteraient trop peu (plan, « Pas d'alias de type pour le noyau »). Le noyau doit donc être l'en-tête lui-même, en trois temps.
3. **La paire `entity` ↔ `ke`** (`entity.h:4` ↔ `ke.c:7`) n'existe que parce que `pico8_lt` prend un `pico8_ke_t *`.
4. **26 lignes `parente`**, dont 16 ne sont qu'un `pico8_ke_t ke;` en premier membre.

### Texte du plan, recopié (`docs/plan-3.md` à `ee2d00f`, lignes 2.3 à 2.3c)

> - 2.3 — **noyau** : `pico8_ke_t` (`ke.h:9-13`) fondu dans `pico8_struct_header_t` (`struct.h:43-45`), qui devient le noyau `{cls, me, t}` ; les casts non vérifiés `((pico8_ke_t *)x)->me`/`->t` (`ke.c:30-31`, `probe.c:230`, `:367`, `entity_geom.c:84`, `bootstrap.c:58`) deviennent des lectures directes. Lecture : les entités qui ne descendent pas de `ke` (`trade`, `hp`, `lq`, `jb`, les enveloppes de `probe`/`slipway`/`wormhole`/`menu`, les remplaçants…) portent désormais `me`/`t` sans les lire ; c'est le prix d'un noyau unique, imposé par la fermeture du compteur (dépendance ci-dessus). La garantie « seul un `ke` entre dans `hv` » (`entity.h:39`) passait par le type de `pico8_lt` : elle reste portée par son seul appelant, `pico8_ke_struct_pq` (`ke.c:41`), et par un `assert` si la PRD en trouve un sûr ; déclarée au commit. `class-coverage-net` : oui (2.3a touche `pico8_ke_struct_init`, que chaque constructeur appelle). `sanitize-net` : 2.3a seulement (disposition changée).
>   - 2.3a — `me`/`t` montent dans l'en-tête ; `pico8_ke_t` n'a plus que `header` ; `x.ke.me`/`x.ke.t` → `x.ke.header.me`/`.t` (172 lignes, `sed`) ; `pico8_lt`, `pico8_ke_ks`, `pico8_ke_struct_init`, `pico8_ke_struct_pq`, `pico8_dg` prennent l'en-tête ; `entity.h:4` n'inclut plus `ke.h`. […] Compte : `inclusion-mutuelle` 18 / 45 → **17 / 43** (paire `entity` ↔ `ke`) ; `parente` inchangé (26) ; pied → **1360 / 93**. Preuves : chemin touché (`gcov` de `ke`, `entity`, `bootstrap` ; mutations : `ke_ks` qui n'écrit plus `me`, `t` non remis à 0, `dg` qui lit `t` d'un autre membre). Budget ≈ 50 min.
>   - 2.3b — les 12 racines filles de `ke` sans descendance (`bg.h:18`, `mouse_reader.h:18`, `msgbox.h:29`, `:85`, `research_icon.h:26`, `score_panel.h:20`, `scroll.h:20`, `slipway.h:38`, `ui.h:32`, `ui_button.h:28`, `:122`, `wormhole.h:29`) et les deux structs de TU (`test_pico8_iv.c:172`, `test_pico8_ui.c:718`) : `pico8_ke_t ke;` → `pico8_struct_header_t header;`, `.ke.header` → `.header`. Compte : `parente` 26 → **12** ; pied → **1346 / 91**. Preuves : refactor pur (disposition identique depuis 2.3a : objets sans `-g` identiques). Budget ≈ 40 min.
>   - 2.3c — `jo` (`planet.h:26`) et `g_` (`probe.h:42`), avec les chemins de leurs descendants (`.qh.jo.ke.header` → `.qh.jo.header`…) ; `pico8_ke_t` et son nom disparaissent. Compte : `parente` 12 → **10** (`jo`→`qh`→…, `g_`→`fv`) ; pied → **1344 / 91**. Preuves : refactor pur (objets sans `-g` identiques). Budget ≈ 40 min.

Règles communes de la phase (plan, « Preuves communes à chaque ligne ») : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée. `class-coverage-net` et `sanitize-net` : règle du §7, « En D et en 2 ».

### État de départ (`ee2d00f`, remesuré)

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 26 17`, `lien-parent 111 27`, `accesseurs-q6 262 33`, `défauts 701 52`, `nu 208 37`, `aiguillage-nom 30 8`, `un-ou-faux 27 14`, `void-ptr 11 7`, `ptrs-entité 7 4`, `atteinte-à-prouver 1 1`, `inclusion-mutuelle 18 18` ; **`résidu 2 : 1362 ligne(s), 93 fichier(s)`** |
| `--check parente,inclusion-mutuelle` | 26 + 45 = **71** lignes |
| `--champs` | `champs : 289 ; 0 écriture : 15 ; 0 hors constructeur : 27` |
| `make test` | vert |
| `make e2e` | 26 conformes |
| `make sanitize-net` / `make class-coverage-net` | 26 propres / OK |
| `make line-coverage` sur les 26 e2e | 6345 lignes de `src/` |

## Solution

Trois commits, dans cet ordre. Chacun est vérifié et poussé avant le suivant.

**2.3a : le noyau est l'en-tête.** `pico8_struct_header_t` devient `{cls, me, t}` ; `pico8_ke_t` n'est plus que `{header}`. Pour toute struct qui descend de `ke`, la disposition mémoire est **identique** (mêmes offsets, même taille). Pour les 16 structs de `src/` qui embarquent l'en-tête sans passer par `ke`, l'en-tête grandit de 16 octets, et leurs champs glissent d'autant. Aucun code ne dépend de ces offsets (voir « Disposition mémoire »). Les six casts vers `pico8_ke_t` deviennent des lectures de l'en-tête. `pico8_lt`, `pico8_ke_struct_init` et `pico8_ke_struct_pq` prennent l'en-tête. `entity.h` n'inclut plus `ke.h`. Résultat : `inclusion-mutuelle` **17 paires / 43 lignes**, pied **1360 / 93**.

**2.3b : les douze racines sans descendance et les deux structs de TU embarquent l'en-tête directement.** `pico8_ke_t ke;` (ou `ke_base`) → `pico8_struct_header_t header;`, et chaque chemin `ke.header` de ces types → `header`. Disposition identique à 2.3a : objets de `src/` identiques octet à octet. Résultat : `parente` **12**, pied **1346 / 91**.

**2.3c : `jo` et `g_` font de même, et `pico8_ke_t` disparaît.** Même opération sur `planet.h:26` et `probe.h:42`. Tous les chemins `ke.header` qui restent sont ceux de l'arbre `jo` et de `g_`/`fv` : ils deviennent `header`. Le `typedef` et son commentaire sortent de `ke.h`. Résultat : `parente` **10**, pied **1344 / 91**.

À l'arrivée, le seul noyau est `pico8_struct_header_t {cls, me, t}`. `ke` n'est plus un type, seulement un module : ses fonctions `pico8_ke_ks`, `pico8_ke_struct_init`, `pico8_ke_struct_pq`, `pico8_ke_init`, etc.

## User Stories

1. En tant qu'auteur des archétypes (2.12), je veux un seul noyau d'entité, l'en-tête, afin qu'un archétype ne nomme qu'un type du noyau.
2. En tant qu'auteur de la liste typée (2.8), je veux que les éléments de la liste soient des `pico8_struct_header_t *` qui portent `me` et `t`, afin de ne plus caster vers `pico8_ke_t` pour les lire.
3. En tant que relecteur, je veux que chacun des six casts non vérifiés vers `pico8_ke_t` soit nommé par `fichier:ligne`, avec ce qui le remplace, afin de relire le diff sans le deviner.
4. En tant que gardien de la mémoire, je veux la taille et les offsets de chaque struct d'entité avant et après chaque commit, afin de savoir quelles structs changent de disposition et lesquelles non.
5. En tant que gardien de la mémoire, je veux la preuve qu'aucun code ne dépend des offsets qui glissent (`offsetof`, `memcmp`, `memcpy`, cast entre types qui supposerait un préfixe commun), afin que le glissement de 16 octets ne casse rien en silence.
6. En tant que gardien du jeu, je veux `make e2e` à 26 conformes après chaque commit, et aucun `*.sha256` touché, afin que le jeu reste le même image par image.
7. En tant que gardien des empreintes, je veux qu'aucune constante `*_HASH` ou `FROZEN_*` ne change, afin d'être sûr que les TU plient les mêmes valeurs.
8. En tant que vérificateur, je veux l'ensemble `L` (lignes de `src/` exécutées par les e2e au départ et plus à l'arrivée) de chaque commit, afin qu'aucune ligne ne perde son seul test.
9. En tant que vérificateur, je veux des mutations sur chaque chemin touché de 2.3a, avec la règle du parent, afin de prouver que les tests voient encore les lectures de `me` et `t`.
10. En tant que vérificateur, je veux un balayage ASan+UBSan de tous les TU, `sanitize-net` et `class-coverage-net`, afin d'écarter une lecture hors bornes ou un constructeur oublié.
11. En tant que relecteur de 2.3b et 2.3c, je veux les objets de `src/` identiques octet à octet, afin de savoir que ce sont des renommages de chemin et rien d'autre.
12. En tant que relecteur des TU, je veux savoir pourquoi des objets de `tests/` diffèrent en 2.3b et 2.3c, afin de ne pas prendre le texte d'un `assert` pour un changement de code.
13. En tant qu'auteur de l'étude C (2.11), je veux que la paire `entity` ↔ `ke` soit tombée, afin de n'étudier que les 17 paires qui restent.
14. En tant que gardien du compteur, je veux qu'aucun alias de type, aucun renommage et aucun déplacement ne fasse sortir une struct de la fermeture, afin que `parente` baisse pour de vrai.
15. En tant que lecteur de `entity.h`, je veux que le contrat « tout élément de `hv` a eu `me` et `t` posés par `pico8_ke_struct_init` » soit écrit, afin de savoir ce que le type de `pico8_lt` ne dit plus.
16. En tant que lecteur de `struct.h`, je veux savoir que `me` et `t` restent non posés sur une entité qui ne descend pas de `ke`, et que personne ne les y lit, afin de ne pas en lire un.
17. En tant que lecteur du glossaire, je veux que `ke` et `lt` y soient à jour, afin que le glossaire ne nomme pas un type disparu.
18. En tant que `plan-pilot`, je veux un repli écrit pour chaque commit, afin de découper sans improviser si le budget déborde.
19. En tant que lecteur du plan, je veux une note « Découvert à 2.3a/b/c » courte au bout de chaque ligne, afin que 2.4 parte des vrais comptes.

## Implementation Decisions

### Modules

- **2.3a** : `struct`, `ke`, `entity`, `bootstrap`, `probe`, `entity_geom` ; les lecteurs de `me`/`t` (`planet`, `planet_display`, `planet_family`, `wormhole`, `wormhole_display`, `slipway`, `slipway_display`, `economy_tiers`, `msgbox`) ; les appelants de `pico8_ke_struct_init`/`_pq` (`bg`, `help`, `menu`, `mouse_reader`, `msgbox`, `planet`, `planet_family`, `planet_family_trade`, `probe`, `research_icon`, `score_panel`, `scroll`, `slipway`, `ui`, `ui_button`, `wormhole`) ; 30 TU ; `docs/glossaire-noms.md` ; `docs/plan-3.md`.
- **2.3b** : les 12 en-têtes des racines et leurs `.c` (22 fichiers de `src/`), 16 TU, `docs/plan-3.md`.
- **2.3c** : `planet`, `probe`, `ke.h`, `struct.h` (commentaire), et les lecteurs de l'arbre `jo` et de `g_` (12 fichiers de `src/` en tout), 21 TU, `docs/glossaire-noms.md`, `docs/plan-3.md`.

### 2.3a : ce qui change, site par site (numéros de `ee2d00f`)

**Types et prototypes**

| Site | Avant | Après |
|---|---|---|
| `struct.h:44-46` | `{ cls; }` | `{ const pico8_struct_class_t *cls; const char *me; pico8_fixed_t t; }`, dans cet ordre (c'est l'ordre de `pico8_ke_t` : `cls` à 0, `me` à 8, `t` à 16) |
| `ke.h:9-13` | `{ header; me; t; } pico8_ke_t` | `{ pico8_struct_header_t header; } pico8_ke_t` |
| `ke.h:31` | `pico8_ke_struct_init(pico8_ke_t *self, …)` | `pico8_ke_struct_init(pico8_struct_header_t *self, …)` |
| `ke.h:36` | `pico8_ke_struct_pq(pico8_ke_t *self)` | `pico8_ke_struct_pq(pico8_struct_header_t *self)` |
| `entity.h:4` | `#include "pico8_ke.h"` | retiré |
| `entity.h:40` | `pico8_lt(pico8_ke_t *ke)` | `pico8_lt(pico8_struct_header_t *e)` |
| `pico8_ke_ks` (`ke.h:27`), `pico8_dg` (`bootstrap.h:21`) | prennent déjà l'en-tête, ou rien | inchangés |

**Les six casts non vérifiés, et ce qui les remplace**

| Site | Avant | Après | Pourquoi le receveur porte `me`/`t` |
|---|---|---|---|
| `ke.c:30` | `((pico8_ke_t *)self)->me = me;` | `self->me = me;` | `self` est un en-tête ; tout en-tête a désormais `me` |
| `ke.c:31` | `((pico8_ke_t *)self)->t = …(0);` | `self->t = …(0);` | idem |
| `ke.c:34-35` | `self->header.cls = cls;` | `self->cls = cls;` | le paramètre est l'en-tête |
| `ke.c:40-41` | `pico8_lt(self)` sur un `pico8_ke_t *` | idem sur l'en-tête | — |
| `entity.c:188-189` | `pico8_lt(pico8_ke_t *ke) { … *e = &ke->header;` | `pico8_lt(pico8_struct_header_t *e) {` (la ligne `:189` part) | — |
| `bootstrap.c:58` | `pico8_ke_t *ke = hv->items[i];` (conversion implicite de `void *`) | ligne retirée ; `ke->me`, `ke->t` (`:63`, `:64`, `:67`, `:75`) → `qc->me`, `qc->t` | `qc` est l'en-tête du même élément |
| `probe.c:230` | `((const pico8_ke_t *)self)->me` | `self->me` | `self` a un `probe_of` non NULL (asserté `:228`) : `g_`, `fv`, l'arbre `qh`, `jc`, `mz`, tous de `ke` |
| `probe.c:367` | `((const pico8_ke_t *)self)->t` | `self->t` | `self` a un `emitter_of` non NULL (asserté `:365`) : `g_`, `fv`, `hj`, `pm`, `mx`, `ga`, tous de `ke` |
| `entity_geom.c:84` | `((const pico8_ke_t *)e)->t` | `e->t` | `e` est de la famille `jo` ou `qb` (asserté `:77-78`), tous deux de `ke` |

Chaque ligne « Après » lit le même mot mémoire que la ligne « Avant », puisque `me` et `t` gardent leurs offsets 8 et 16 dans toute struct de `ke`. Le comportement ne change donc pas. La différence : le compilateur voit désormais un membre de l'en-tête, pas une réinterprétation.

**Lectures par chemin de membre (`sed`)**

- Motif : `\b(ke(_base)?)\.(me|t)\b` → `\1.header.\3`. Mesuré : **173 lignes** (le plan dit 172 ; la 173ᵉ est le commentaire `probe.h:90`), 198 occurrences.
  - `src/` (21 lignes) : `planet.c:180`, `:343`, `:495`, `:498` ; `planet_display.c:293`, `:415`, `:417` ; `planet_family.c:83`, `:420` ; `probe.c:488` ; `probe.h:90` (commentaire) ; `wormhole.c:116`, `:161`, `:250` ; `wormhole_display.c:31`, `:49` ; `slipway.c:408` ; `slipway_display.c:67` ; `economy_tiers.c:219` ; `msgbox.c:59`, `:65`.
  - `tests/` (152 lignes, 30 TU) : `bg` 1, `fq` 2, `fr` 6, `fv` 4, `g_` 14, `ga` 2, `hj` 8, `hp` 2, `hy` 1, `ie` 5, `iv` 6, `jc` 5, `jo` 5, `jo_spec_defaults` 19, `js` 6, `lq` 1, `mh` 2, `mx` 6, `mz` 7, `nl` 7, `nr` 6, `os` 7, `pj` 2, `pm` 4, `qb` 3, `qh` 8, `spec_defaults` 8, `trade` 1, `ui` 4.
- Motif : `(pico8_ke_struct_(init|pq)\(&[^,)]*\bke(_base)?)(?=[,)])` → `\1.header`. 45 lignes : les 44 appels de `src/` listés par `grep -rn 'pico8_ke_struct_\(init\|pq\)(' src` et `test_pico8_iv.c:204`, `:206`.
- `test_pico8_ui.c:782` : `pico8_lt(&g_giver.ke)` → `pico8_lt(&g_giver.ke.header)`.

**Initialiseurs (découvert à l'essai, absent du plan)**

`CFLAGS` contient `-Wextra -Werror`, donc `-Wmissing-field-initializers`. Un initialiseur positionnel `{&cls}` d'un en-tête à trois membres est **refusé** (« missing initializer for field 'me' »). Onze sites l'écrivent : `entity.c:47`, `menu.c:52`, `planet_family_trade.c:55`, `probe.c:31`, `:41`, `score_panel.c:66`, `slipway.c:30`, `ui.c:39`, `wormhole.c:25`, `test_pico8_mo.c:95`, `:96`. Chacun passe à l'initialiseur désigné **`{.cls = &…}`**, qui ne déclenche pas l'avertissement et met `me`/`t` à zéro, comme les objets statiques l'étaient déjà (voir G2). Le douzième, `test_pico8_ui.c:761` (`{{{&g_giver_class}, NULL, 0}}`, un `giver_t` qui embarque un `pico8_ke_t`), devient `{{{.cls = &g_giver_class}}}` : `NULL` et `0` étaient `me` et `t`, que le zéro implicite redonne.

Tous ces initialiseurs gardent l'accolade de l'en-tête (`{{…}, …}`) : aucune élision d'accolades ne peut faire glisser un membre suivant dans `me`.

**Aides de TU qui castaient vers `pico8_ke_t`**

- `is_me(const pico8_ke_t *ke, …)` (`test_pico8_mz.c:89`, `test_pico8_g_.c:95`, `test_pico8_bg.c:93`) prend `const pico8_struct_header_t *` ; ses 11 appels `is_me(&X->ke, …)` passent `&X->ke.header` (`mz:284`, `:308`, `:547`, `:556`, `:567`, `:586` ; `g_:615` ; `bg:187`, `:326`, `:394`, `:417`).
- `ke_of` (`test_pico8_jc.c:413`, `test_pico8_ui.c:817`) rend `pico8_struct_header_t *` ; la locale `test_pico8_jc.c:422` suit. Le nom `ke_of` reste : il n'est compté par aucune catégorie, et le renommer ne ferait rien tomber (G3).

**Commentaires (règle des quatre destinations de `CLAUDE.md`)**

- `struct.h:42-43` : le contrat C de l'en-tête gagne une phrase, par exemple « `me` and `t` are ke's (`me` NULL is nil): pico8_ke_struct_init sets them; on an entity that never went through it they are left unset and never read. »
- `ke.h:6-8` : ne garde que « First member of every entity struct of a class descended from ke. » La phrase sur `lt` part vers `entity.h`.
- `ke.h:24-26` : « written into its pico8_ke_t » → « written into its header ».
- `entity.h:39` (qui devient `:38`) : le contrat que le type ne porte plus, par exemple « Only pico8_ke_struct_pq calls it outside tests: every entity of `hv` has had `me` and `t` set by pico8_ke_struct_init, and pico8_dg() reads them. »
- `test_pico8_jc.c:412`, `test_pico8_ui.c:816` : « begins with its `pico8_ke_t` » → « begins with its header ».
- `probe.h:90` : transformé par le `sed` (`ke.header.t`), puis `header.t` à 2.3c.
- `struct.h:74` (« A class that sets one embeds pico8_ke_t first ») reste vrai jusqu'à 2.3c, et change alors.
- `docs/glossaire-noms.md:35` (`lt`) : « et ne prend qu'un `pico8_ke_t` » → « et prend l'en-tête `pico8_struct_header_t` ».

**Sur l'`assert` de `pico8_lt`.** Le plan demande un `assert` « si la PRD en trouve un sûr ». **Il n'y en a pas** : `ke` n'a aucune table de classe (`grep ke_struct_class src` est vide), et aucune chaîne `parent()` ne remonte jusqu'à lui. Rien, dans un en-tête, ne dit qu'il est passé par `pico8_ke_struct_init`. La garantie reste donc portée par le seul appelant de `src/`, `pico8_ke_struct_pq`, et elle est écrite au contrat de `entity.h`. Elle compte d'ailleurs moins qu'avant : `pico8_dg` lit `me`/`t` dans l'en-tête, qui les porte sur toute entité ; une entité hors `ke` qui entrerait dans `hv` ne ferait plus lire hors de la struct, seulement lire un champ jamais posé. À déclarer au commit.

**Inclusions.** `entity.h` ne lit plus rien de `ke.h`. La paire `entity` ↔ `ke` tombe, et `entity.h:4` et `ke.c:7` sortent du compte. `ke.c:7` reste dans le fichier (`ke.c` appelle `pico8_lt`). Règle de D.13 : aucun fichier ne perd `ke.h` de sa fermeture (`cc -MM -MG -DPICO8_CLASS_COVERAGE` : 103 fichiers la contiennent, avant comme après), donc **aucun include direct gagné**.

### Disposition mémoire (sonde `gdb ptype /o`, sur copie)

Sonde : un `.c` jetable qui inclut tous les `src/*.h` (hors raylib) et déclare un pointeur de chacun des 37 types de `--champs`, compilé en `-g`, puis `gdb -batch -ex 'ptype /o <type>'` ; les types locaux à un `.c` (`pico8_ha_pz_t`, `pico8_mo_placeholder_t`, `pico8_nr_ldown_empty_t`, `pico8_je_placeholder_t`, `giver_t`, `ra_only_t`) sont lus sur l'objet `-g` de leur fichier.

| Types | `ee2d00f` | après 2.3a | après 2.3b | après 2.3c |
|---|---|---|---|---|
| `pico8_struct_header_t` | 8 | **24** (`cls` 0, `me` 8, `t` 16, 4 de bourrage) | 24 | 24 |
| `pico8_ke_t` | 24 (`header` 0, `me` 8, `t` 16) | 24 (`header` seul) | 24 | — (supprimé) |
| 26 types de `ke` : `bg` 56, `fq` 56, `fr` 392, `fv` 256, `g_` 192, `ga` 368, `hj` 448, `ie` 88, `iv` 72, `jc` 896, `jo` 280, `js` 192, `mh` 160, `mx` 456, `mz` 160, `nf` 32, `nl` 384, `nr` 384, `pj` 72, `pm` 464, `qb` 64, `qh` 368, `ui` 360, `giver_t` 24 | = | **=** (offsets identiques) | = | = |
| 16 types de `src/` à en-tête direct : `bv_et` 16, `fm_placeholder` 24, `ha_pz` 32, `hp` 472, `hy` 344, `ie_ke` 40, `jb` 24, `jc_mouse_point` 24, `jc_too_far` 24, `je_placeholder` 8, `lq` 72, `mo_placeholder` 8, `mz_jj` 16, `nr_ldown_empty` 8, `pj_cr` 40, `trade` 40 ; et `ra_only_t` 16 (TU) | = | **+16** chacun, tous leurs champs glissent de 16 | = | = |

Les structs de TU `fake_toast_t` (`iv`) et `giver_t` (`ui`) suivent leur racine.

**Ce qui pourrait dépendre de ces offsets, cherché et écarté :**

- `offsetof` : **aucun** dans `src/` ni `tests/` (et `make data-guard` refuse déjà tout `offsetof` dans `src/`).
- `memcmp` / `memcpy` : **aucun** sur une struct d'entité. Les 20 `memcpy` de `src/` copient des chaînes ou des tampons d'écran (`score_panel.c:39`, `probe.c:260`, `gif.c:152`…) ; les `memcmp` sont dans `minimize.c` (`dpad`) et dans les TU de capture.
- `sizeof` d'un type d'entité hors `malloc` : seulement `test_pico8_ui.c:912-925` (`assert_own_emitter`/`assert_own_probe`), qui vérifie qu'un pointeur tombe **dans** la struct. Le test ne dépend d'aucun offset.
- Casts entre types : les ≈ 170 casts `(pico8_X_t *)` de `src/` partent tous d'un `pico8_struct_header_t *` ou d'un `void *`. Ils supposent seulement que l'en-tête est à l'offset 0 de `X`. C'est vrai avant, et après chaque sous-étape : l'en-tête reste premier membre, directement ou par la chaîne des premiers membres (`qh` → `jo` → en-tête). Les descentes `pico8_qh_of`, `pico8_nl_of` (`planet_family.c:679-695`) et `pico8_g__of` (`probe.h:58`) castent l'en-tête vers `qh`/`nl`/`g_` sous la même hypothèse ; 2.3c retire le niveau `ke` **au-dessus** de l'en-tête, pas entre `qh` et `jo`, et l'adresse ne change pas. Elles restent pour 2.15.
- Seul cast qui supposait un **préfixe plus long** que l'en-tête : les six vers `pico8_ke_t` du tableau plus haut, plus `ke_of` des deux TU. Tous remplacés par des lectures de l'en-tête.
- Empreintes de TU : elles plient des valeurs de membres, pas des octets. `make test` vert sur les quatre arbres, sans aucune constante changée, le prouve.

**Champs jamais posés.** Sur les cinq entités hors `ke` allouées par `malloc` (`trade.c:28`, `layout.c:303` (`lq`), `:407` (`jb`), `economy.c` (`hy`), `research.c` (`hp`)) et sur la vue `pico8_ie_ke_view` (`research_icon.c:55`, une locale rendue par valeur), `me` et `t` restent **indéterminés** : leur constructeur ne pose que `header.cls`. Les objets statiques (les onze initialiseurs ci-dessus) les ont à 0. Les seuls lecteurs de `me`/`t` sur un en-tête après 2.3a sont `bootstrap.c` (`dg`, sur `hv`), `probe.c:230`, `:367`, `entity_geom.c:84`, plus les chemins typés `X.ke.header.me` d'un type de `ke`. Aucun n'atteint une entité hors `ke` (voir la colonne « Pourquoi » du tableau des casts). Relevé : `grep -nE '(->|\.)(me|t)\b' src/*.c` sur l'arbre 2.3a. Décision G2 : on ne les met pas à zéro.

### 2.3b : ce qui change

- **14 déclarations** : `pico8_ke_t ke;` → `pico8_struct_header_t header;` à `bg.h:18` (`bg`), `mouse_reader.h:18` (`nf`), `msgbox.h:29` (`mh`), `:85` (`iv`), `score_panel.h:20` (`pj`), `scroll.h:20` (`qb`), `slipway.h:38` (`jc`), `ui.h:32` (`ui`), `ui_button.h:28` (`js`), `:122` (`fq`), `wormhole.h:29` (`mz`), `test_pico8_iv.c:172` (`fake_toast_t`), `test_pico8_ui.c:718` (`giver_t`) ; et `pico8_ke_t ke_base;` → `pico8_struct_header_t header;` à `research_icon.h:26` (`ie`). Aucun de ces 13 types n'a déjà un membre `header`. Le membre `ke` de `pico8_ie_t` (un `pico8_entity_kind_t *`, `research_icon.c:46`) n'est **pas** touché.
- **125 occurrences de chemin** `ke.header` / `ke_base.header` → `header`, sur les seuls types ci-dessus. On les trouve par le compilateur, pas par un `sed` global : l'arbre `jo` et `g_` gardent `ke.header` jusqu'à 2.3c. Méthode de l'essai : compiler chaque `.c` en `-fsyntax-only`, prendre chaque erreur « has no member named 'ke' » (ou `'ke_base'`), réécrire le `ke(_base)?\.header` de sa colonne. Par ligne, on traite les colonnes de droite à gauche, sinon la première réécriture décale la seconde. Une passe suffit (125 erreurs, 125 réécritures ; la deuxième passe ne trouve rien).
  - `src/` (49 lignes) : `bg.c:69`, `:80`, `:201` ; `mouse_reader.c:46`, `:52` ; `msgbox.c:59`, `:65`, `:114`, `:137`, `:262`, `:280` ; `research_icon.c:240`, `:258` ; `score_panel.c:293`, `:305` ; `scroll.c:47`, `:56` ; `slipway.c:148`, `:185`, `:189`, `:407`, `:408`, `:442` ; `slipway_display.c:67` ; `ui.c:100`, `:118`, `:371` ; `ui_button.c:109`, `:162`, `:355`, `:369` ; `wormhole.c:116`, `:249`, `:250`, `:277` ; `wormhole_display.c:31`, `:49` ; plus les 14 déclarations.
  - `tests/` (71 lignes, 16 TU) : `bg`, `fq`, `hp`, `hy`, `ie`, `iv`, `jc`, `js`, `lq`, `mh`, `mz`, `pj`, `qb`, `spec_defaults`, `trade`, `ui` (liste `fichier:ligne` au message de commit).
- `test_pico8_ui.c:761` : `{{{.cls = &g_giver_class}}}` → `{{.cls = &g_giver_class}}` (un niveau d'accolades de moins).
- **Pourquoi rien ne change** : `ke` était `{header}` depuis 2.3a, donc `x.ke.header.m` et `x.header.m` sont au même offset. Les **72 objets de `src/` sans `-g` sont identiques octet à octet**. Les 12 objets de TU qui diffèrent (`bg`, `fq`, `ie`, `iv`, `jc`, `js`, `mh`, `mz`, `pj`, `qb`, `spec_defaults`, `ui`) ne diffèrent que par le texte de leurs `assert` (la chaîne `#expr` embarquée) ; sous `-DNDEBUG`, ils sont identiques. Mesuré.

### 2.3c : ce qui change

- `planet.h:26` (`jo`) et `probe.h:42` (`g_`) : `pico8_ke_t ke;` → `pico8_struct_header_t header;`.
- **166 lignes** `ke.header` → `header`. Après 2.3b, toutes appartiennent à l'arbre `jo` ou à `g_`/`fv`, donc ici un `sed` global sur `\bke\.header\b` est juste (essai : 0 erreur de compilation ensuite).
  - `src/` (36 lignes) : `economy_tiers.c:219` ; `help.c:181` ; `planet.c:141`, `:180`, `:202`, `:285`, `:343`, `:495`, `:498` ; `planet_display.c:154`, `:293`, `:298`, `:415`, `:417`, `:429`, `:436` ; `planet_family.c:83`, `:103`, `:180`, `:237`, `:334`, `:420`, `:438`, `:627` ; `planet_family_trade.c:35`, `:144`, `:228`, `:289`, `:410` ; `planet_trade.c:152`, `:168` ; `probe.c:447`, `:488`, `:503` ; `probe.h:90` (commentaire) ; `wormhole.c:161`.
  - `tests/` (130 lignes, 21 TU) : `bg` 2, `fr` 6, `fv` 5, `g_` 24, `ga` 2, `hj` 8, `hp` 1, `jo` 6, `jo_spec_defaults` 19, `js` 3, `mx` 6, `mz` 2, `nl` 8, `nr` 8, `os` 8, `planet_kj_dispatch` 2, `pm` 4, `qh` 8, `spec_defaults` 2, `trade` 2, `ui` 4.
- `ke.h:6-13` : le commentaire et le `typedef` de `pico8_ke_t` partent (7 lignes). `ke.h` garde `#include "pico8_struct.h"` et ses six prototypes.
- `struct.h:76` (`:74` à `ee2d00f`) : « A class that sets one embeds pico8_ke_t first » → « A class that sets one descends from ke ».
- `docs/glossaire-noms.md:32` : la colonne C de `ke` devient `pico8_ke_*` (module, sans type) ; le sens ajoute « son état `me`/`t` vit dans l'en-tête `pico8_struct_header_t` ».
- **Pourquoi rien ne change** : même raisonnement qu'en 2.3b. **Les 72 objets de `src/` sont identiques octet à octet** ; les 14 TU qui diffèrent (`bg`, `fr`, `g_`, `hj`, `jo`, `jo_spec_defaults`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `spec_defaults`, `ui`) sont identiques sous `-DNDEBUG`. Mesuré.
- **Inclusions laissées** : `planet.h`, `probe.h` et les 12 en-têtes de 2.3b gardent leur `#include "pico8_ke.h"`, même si l'en-tête lui-même n'en lit plus rien : leurs `.c` appellent `pico8_ke_struct_init`/`_pq` par ce chemin. Les retirer toucherait des fermetures sans rien faire tomber (G4).

### `--champs` (règle 1), relevé déclaré

Après 2.3a : `champs : 289 ; 0 écriture : 15 ; 0 hors constructeur : 27`, inchangé. Mais deux choses bougent :
- `pico8_ke_t me 9 9` et `pico8_ke_t t 7 6` deviennent `pico8_struct_header_t me 9 9` et `t 7 6` ;
- `pico8_struct_header_t cls` passe de `8 4` à **`17 13`** : les neuf initialiseurs désignés `.cls = &…` de `src/` comptent comme écritures hors constructeur (motif `(->|\.)cls =`).

2.3b et 2.3c ne changent pas `--champs`.

### Comptes, mesurés sur copie

| Catégorie | `ee2d00f` | 2.3a | 2.3b | 2.3c |
|---|---|---|---|---|
| `parente` (lignes / fichiers) | 26 / 17 | 26 / 17 | **12 / 5** | **10 / 3** |
| `inclusion-mutuelle` (paires / modules ; lignes) | 18 / 18 ; 45 | **17 / 16 ; 43** | 17 / 16 ; 43 | 17 / 16 ; 43 |
| les dix autres | inchangées | inchangées | inchangées | inchangées |
| pied | 1362 / 93 | **1360 / 93** | **1346 / 91** | **1344 / 91** |

En 2.3b, `bg.h` et `mouse_reader.h` quittent le pied : `parente` était leur seule ligne comptée. Les dix lignes `parente` qui restent sont `help.h:21`, `planet_family.h:32`, `:51`, `:62`, `:72`, `:88`, `:102`, `planet_family_trade.h:23`, `:34`, `:45` : les enchâssements entre classes, pour 2.15. `COMPOSITION_RESIDUE_GUARD` ne change pas (`noyau-inclut`) : aucune catégorie ne tombe à 0.

### Anti-tassement

- **Interdit** :
  - un alias de type ou une macro pour `pico8_ke_t` : `typedef pico8_struct_header_t pico8_ke_t;`, `#define pico8_ke_t …`, ou un `typedef struct X pico8_ke_t;` (le lecteur du compteur suit ces alias et sortirait l'arbre `jo` de la fermeture) ;
  - renommer un membre enchâssé pour échapper au motif `parente` (par exemple `pico8_jo_t jo;` en `pico8_jo_t base;`), ou déplacer un enchâssement hors de la première ligne ;
  - changer `tools/composition_residue.sh`, `tests/test_composition_residue.c`, `COMPOSITION_RESIDUE_GUARD`, `tests/e2e/`, un `*.sha256` ou une constante d'empreinte ;
  - un cast neuf vers un type qui supposerait un préfixe plus long que l'en-tête ;
  - mettre un `me`/`t` hors de l'en-tête (un deuxième `me` dans une struct, sauf `pico8_ie_ke_t.me`, qui est un autre champ : l'icône de la vue de classe).
- **Classification au message**, pour chaque ligne comptée retirée (numéros du parent) : 2.3a, (a) `entity.h:4` retirée ; `ke.c:7` sort du compte sans être touchée. 2.3b, les 14 lignes `parente`, réécrites hors motif. 2.3c, les 2 lignes `parente`, réécrites hors motif. Lignes qui gagnent une catégorie : aucune (mesuré par `--detail` avant/après).

## Implementation Chunks

1. **2.3a** (un commit, ≈ 50 min) : types et prototypes ; les six casts ; `sed` des 173 lignes et des 45 appels ; les 12 initialiseurs ; les aides de TU ; les commentaires et la ligne `lt` du glossaire ; note du plan ; preuves ; push.
2. **2.3b** (un commit, ≈ 40 min) : 14 déclarations ; 125 chemins trouvés par le compilateur ; `test_pico8_ui.c:761` ; note du plan ; preuves ; push.
3. **2.3c** (un commit, ≈ 40 min) : 2 déclarations ; `sed` global des 166 lignes ; `typedef` retiré ; `struct.h:74` ; glossaire `ke` ; note du plan ; preuves ; push.

Chaque commit part de l'arbre du précédent. Ses preuves d'objets et de couverture comparent au commit parent.

**Replis écrits (si le budget déborde avant le commit).**

- **2.3a** ne se coupe pas dans le code : le changement de l'en-tête, les `sed` et les initialiseurs ne compilent qu'ensemble. Repli : **2.3a1**, tout le code et les preuves, commentaires laissés tels quels sauf `entity.h:39` (le contrat qui remplace le type ne peut pas attendre) ; puis **2.3a2**, les autres commentaires et le glossaire (préprocesseur identique hors `__LINE__` et commentaires). Les comptes de 2.3a sont atteints dès 2.3a1.
- **2.3b** se coupe par racine, chaque moitié compile seule : **2.3b1** `bg`, `nf`, `mh`, `iv`, `ie`, `pj` et `fake_toast_t` (`parente` 26 → 19) ; **2.3b2** `qb`, `jc`, `ui`, `js`, `fq`, `mz` et `giver_t` (19 → 12). Comptes de chaque moitié à remesurer par `--check parente`.
- **2.3c** se coupe en **2.3c1** `g_` (`probe.h:42`, les chemins de `g_`/`fv`, `parente` 12 → 11) puis **2.3c2** `jo` et son arbre, et le retrait du `typedef` (11 → 10).
- Un repli est écrit dans la note « Découvert à » ; les comptes finaux ne changent pas.

## Testing Decisions

- **Aucun TU nouveau.** 2.3a change la disposition et le chemin de six lectures, pas un comportement : les tests existants lisent `me` et `t` par ces chemins, et les mutations prouvent qu'ils les voient. 2.3b et 2.3c sont des renommages de chemin : la preuve est l'identité des objets.
- **Bon test ici** : il lit `me`/`t` par l'API ou par le membre (`is_me`, `ke_of`, `x.header.t`), jamais par un offset.
- **Prior art** : preuve d'objets de D.19c1 et 2.1/2.2 ; mutations avec règle du parent de D.21 ; ensemble `L` de D.20a ; balayage ASan+UBSan de D.12d ; sonde de disposition de D.20b (en-tête `{tag, cls}` → `{cls}`, même genre de changement).

### Preuves par commit

**2.3a (chemin touché)**

1. `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e` vide.
2. **Sonde de taille** avant/après (tableau ci-dessus) : 26 types de `ke` inchangés, 17 types à en-tête direct à +16, en-tête 8 → 24, `pico8_ke_t` 24 → 24.
3. **`gcov` des e2e** (`make line-coverage EVENTS="$(ls tests/e2e/*.json)"`) au parent et après ; `L` calculé avec un appariement des lignes par `diff`. Attendu, mesuré : **`L` = {`bootstrap.c:58`, `entity.c:189`}**, les deux lignes supprimées (6345 → 6343 lignes). La nouvelle signature `entity.c:188` est exécutée. `ke.c:26-42`, `bootstrap.c:51-76` (lignes exécutables), `probe.c:230`, `:367`, `entity_geom.c:84` sont exécutées à l'arrivée. Toute autre ligne dans `L` : arrêt, rapporté.
4. **Mutations**, une à la fois, rejouées au parent (règle du parent). Mesuré sur copie, résultats identiques au parent et à l'enfant :

| # | Mutation (enfant ; au parent, la même sur l'ancienne forme) | TU rouges | e2e conformes |
|---|---|---|---|
| M1 | `ke.c:30` `self->me = me;` → `(void)me;` | 18 : `bg fr g_ hj hp hy jc jo js lq mx mz nl nr os qh trade ui` | 7 / 26 |
| M2 | `ke.c:31` `t` non remis à 0 (ligne vidée) | 4 : `jc jo mz ui` | 6 / 26 |
| M3 | `bootstrap.c` `dg` passe `hv->items[0]->t` au lieu de `qc->t` | 11 : `fr hj iv jo mx mz nl nr os pm qh` | 7 / 26 |
| M4 | `probe.c:367` `t = self->t` → `t = 0` | 4 : `hj mx pm probe_mm_rnd_order` | 9 / 26 |
| M5 | `entity_geom.c:84` `t = e->t` → `t = 0` | 4 : `hp lq pm qb` | 9 / 26 |
| M6 | `probe.c:230` `self->me` → `NULL` | 1 : `jc` | 6 / 26 |

   Attention, piège de l'essai : restaurer par `mv fichier.orig fichier` rend au fichier un `mtime` antérieur à l'objet muté, et `make` ne recompile pas. Restaurer par `cp`, puis `touch`.
5. **ASan+UBSan sur tous les TU** : `ASAN_OPTIONS=detect_leaks=0 make -k unit TMP_DIR=tmp-san CFLAGS='-std=c99 -Wall -Wextra -Werror -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer'` (à faire dans une copie ou avec un `TMP_DIR` jetable). Mesuré : code 0, 76 lignes « passed/ok » sur 78 lancements, 0 `ERROR: AddressSanitizer`, 0 `runtime error`, au parent comme à l'enfant. Sans `detect_leaks=0`, des TU fuient déjà (mesuré à l'enfant) : fuites connues, hors sujet, et `sanitize-net` les coupe de même.
6. `make sanitize-net` : **26 propres** ; `make class-coverage-net` : OK, identique à `tools/class_coverage_net_expected.txt`.
7. **Objets** : 58 des 148 objets sans `-g` changent (23 de `src/`, 35 TU) (attendu : disposition des 17 types, `__LINE__` de `bootstrap.c` et `entity.c`). Aucune exigence d'identité ; la liste va au message.
8. `--champs` : relevé déclaré ci-dessus.

**2.3b et 2.3c (refactor pur)**

1. `make test`, `make e2e` 26 conformes, aucun `*.sha256` touché.
2. **Objets sans `-g`** des 148 `.c` compilables sans flag (recette de 2.1/2.2, depuis la racine de chaque arbre) : **72 objets de `src/` identiques octet à octet** ; les TU qui diffèrent sont identiques sous `-DNDEBUG` (`test_pico8_js.c` a besoin de `-w` sous `-DNDEBUG` : `ix` non initialisé, au parent comme à l'enfant). Les deux objets sous `-DPICO8_CLASS_COVERAGE` (`main_class_coverage.c`, `test_class_coverage.c`) : identiques.
3. **Sonde de taille** : identique à 2.3a, type par type.
4. `gcov` des e2e : **`L` vide** (6343 = 6343), mesuré.
5. Mutations : aucune exigée (objets identiques). L'essai en a fait une par commit, pour montrer que les tests lisent le nouveau chemin : 2.3b, `msgbox.c:65` facteur 10 → 9 : `mh` rouge, 25 / 26, au parent comme à l'enfant ; 2.3c, `planet.c:343` `"ly"` → `"ll"` : `hp hy jc js lq` rouges, 14 / 26, au parent comme à l'enfant.
6. `class-coverage-net` : oui (le diff touche les lignes `pico8_ke_struct_init` de constructeurs), OK. `sanitize-net` : non exigé (disposition inchangée) ; mesuré propre quand même. ASan+UBSan des TU : mesuré propre après 2.3c.

### Greps de Sortie (sur l'arbre final de chaque commit)

Après 2.3a :

1. `grep -rn '(const pico8_ke_t \*)\|(pico8_ke_t \*)' src tests` → **vide**.
2. `grep -rnE 'pico8_ke_t \*' src tests` → **vide** (aucun prototype ni local ne prend plus un `pico8_ke_t *`).
3. `grep -rnE '\bke(_base)?\.(me|t)\b' src tests` → **vide**.
4. `grep -n '#include "pico8_ke.h"' src/pico8_entity.h` → **vide**.
5. `grep -rnE '\{\{?&g_[a-z0-9_]+\}' src tests` → **vide** (plus d'initialiseur positionnel d'en-tête).
6. `grep -rn 'pico8_ke_t' src tests` → exactement 18 lignes : `ke.h`, `} pico8_ke_t;` ; les 16 déclarations `pico8_ke_t ke;`/`ke_base;` (les 16 lignes `parente` qui nomment `pico8_ke_t`) ; `struct.h:76` (l'ancien `:74`, commentaire, décalé par les deux membres ajoutés).
7. `sh tools/composition_residue.sh --detail | grep -c '^inclusion-mutuelle.*pico8_entity.*pico8_ke'` → **0** ; `--check inclusion-mutuelle` → `43 ligne(s)` ; pied `1360 / 93`.

Après 2.3b :

8. `sh tools/composition_residue.sh --check parente` → 12 lignes, **exactement** `help.h:21`, `planet.h:26`, `planet_family.h:32`, `:51`, `:62`, `:72`, `:88`, `:102`, `planet_family_trade.h:23`, `:34`, `:45`, `probe.h:42`.
9. `grep -rn 'ke_base' src tests` → **vide**.
10. `grep -rnE '\bke\.header\b' src tests` → seulement des chemins de l'arbre `jo` ou de `g_`/`fv` (les 166 lignes de 2.3c) ; exclusion nommée : rien d'autre.

Après 2.3c :

11. `grep -rn 'pico8_ke_t' src tests docs/glossaire-noms.md` → **vide**.
12. `grep -rnE '\bke\.header\b' src tests` → **vide**.
13. `sh tools/composition_residue.sh --check parente` → 10 lignes, celles de 8 sans `planet.h:26` ni `probe.h:42`.
14. `grep -rn 'pico8_struct_header_t header;' src tests | wc -l` → le nombre du parent + 1 (`planet.h:26` et `probe.h:42` entrent, `ke.h:10` part avec le `typedef`).
15. `make composition-residue` → `parente 10 3`, `inclusion-mutuelle 17 16`, dix autres inchangées, pied `1344 / 91`.
16. Pour chaque commit, `git diff --stat <parent>` : seulement `src/`, `tests/` (hors `tests/e2e/`), `docs/plan-3.md` et, pour 2.3a et 2.3c, `docs/glossaire-noms.md`.

### Budget et tableau « Runs »

Budget : **≈ 50 min pour 2.3a, ≈ 40 min pour 2.3b et 2.3c**, preuves comprises. Au-delà de 1,5 fois le budget, on passe au repli, sans rien commettre d'incomplet. Jamais l'appli raylib ; `make raylib` non exigé.

L'implémenteur tient un tableau « Runs », une ligne par lancement. Voici celui des essais de ce PRD, sur copie (à refaire sur le vrai arbre) :

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make composition-residue` | `ee2d00f` | 0 | pied 1362 / 93 ; `inclusion-mutuelle 18 18` | < 1 s |
| 2 | `--check parente,inclusion-mutuelle` | `ee2d00f` | 1 | 71 lignes | < 1 s |
| 3 | `make test`, `make e2e` | `ee2d00f` | 0 | vert ; 26 conformes | 8 s ; 1 s |
| 4 | `make -k test` | 2.3a sans initialiseurs | 2 | 11 × `missing initializer for field 'me'` | 5 s |
| 5 | `make test` | 2.3a | 0 | vert | 8 s |
| 6 | `make e2e` | 2.3a | 0 | 26 conformes | 1 s |
| 7 | `make composition-residue` | 2.3a | 0 | `inclusion-mutuelle 17 16`, pied 1360 / 93 ; `--check` 43 | < 1 s |
| 8 | sonde `ptype /o`, 43 types | `ee2d00f` / 2.3a | 0 | 26 types `ke` =, 17 à +16, en-tête 8 → 24 | 10 s |
| 9 | 148 objets sans `-g` | `ee2d00f` / 2.3a | 0 | 58 diffèrent (attendu) | 30 s |
| 10 | `make line-coverage` 26 e2e | `ee2d00f` / 2.3a | 0 | 6345 → 6343 ; `L` = `bootstrap.c:58`, `entity.c:189` | 5 s ×2 |
| 11 | M1 à M6, enfant puis parent | 2.3a / `ee2d00f` | 2 | rouges identiques (tableau) | 12 × 10 s |
| 12 | ASan+UBSan, `make -k unit` | `ee2d00f` / 2.3a | 0 | 76 ok / 78, 0 erreur | 13 s ×2 |
| 13 | `sanitize-net`, `class-coverage-net` | `ee2d00f` / 2.3a | 0 | 26 propres ; OK | 3 s ; 2 s |
| 14 | `--champs` | 2.3a | 0 | 289 / 15 / 27 ; `cls 17 13` | < 1 s |
| 15 | `-MM -MG`, fermetures de `ke.h` | `ee2d00f` / 2.3a | 0 | 103 = 103 | 10 s |
| 16 | `-fsyntax-only` itératif | 2.3b | 0 | 125 erreurs, 125 réécritures, 2ᵉ passe vide | 40 s |
| 17 | `make test`, `make e2e` | 2.3b | 0 | vert ; 26 conformes | 9 s |
| 18 | `make composition-residue` | 2.3b | 0 | `parente 12 5`, pied 1346 / 91 | < 1 s |
| 19 | objets sans `-g` ; TU sous `-DNDEBUG` | 2.3a / 2.3b | 0 | 72 `src/` identiques ; 12 TU diffèrent, identiques sous `NDEBUG` | 40 s |
| 20 | `line-coverage` ; sonde ; `ccn` ; `sanitize-net` | 2.3b | 0 | `L` vide ; tailles = ; OK ; 26 propres | 15 s |
| 21 | M7 enfant / parent | 2.3b / 2.3a | 2 | `mh` ; 25 / 26 les deux | 20 s |
| 22 | `sed` global, `make test`, `make e2e` | 2.3c | 0 | 166 lignes ; vert ; 26 conformes | 10 s |
| 23 | `make composition-residue` | 2.3c | 0 | `parente 10 3`, pied 1344 / 91 | < 1 s |
| 24 | objets ; TU sous `-DNDEBUG` | 2.3b / 2.3c | 0 | 72 `src/` identiques ; 14 TU identiques sous `NDEBUG` | 40 s |
| 25 | `line-coverage` ; sonde ; `ccn` ; `sanitize-net` ; ASan+UBSan | 2.3c | 0 | `L` vide ; = ; OK ; 26 propres ; 76 ok | 30 s |
| 26 | M8 enfant / parent | 2.3c / 2.3b | 2 | `hp hy jc js lq` ; 14 / 26 les deux | 20 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `ee2d00f` ou un descendant qui ne touche que `docs/`, ou si les comptes de départ diffèrent sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) reste rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si un type de `ke` change de taille ou d'offset à 2.3a, ou si un type quelconque change de taille à 2.3b ou 2.3c.
- `FAILED:` si un objet de `src/` sans `-g` diffère à 2.3b ou 2.3c, ou si un objet de TU diffère sous `-DNDEBUG`.
- `FAILED:` si `L` de 2.3a contient autre chose que `bootstrap.c:58` et `entity.c:189`, ou si `L` de 2.3b/2.3c n'est pas vide.
- `FAILED:` si une mutation M1 à M6 reste verte à l'enfant alors qu'elle est rouge au parent ; une mutation verte aux deux est déclarée « chemin jamais atteint ».
- `FAILED:` si ASan+UBSan, `sanitize-net` ou `class-coverage-net` signale quoi que ce soit.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit, ou si les comptes d'arrivée ne sont pas 17 / 43 et 1360 / 93 (2.3a), 12 et 1346 / 91 (2.3b), 10 et 1344 / 91 (2.3c), sans explication.
- `FAILED:` en cas de tassement (voir « Anti-tassement »).
- `FAILED:` si le message d'un commit omet : les sites `fichier:ligne` du parent, la classification des lignes comptées, la sonde avant/après, `L`, les mutations (2.3a), le rapport objets, les filets, le relevé `--champs` (2.3a), l'`assert` non trouvé et son pourquoi (2.3a), le tableau « Runs ».
- `FAILED:` s'il manque une note « Découvert à », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les dix lignes `parente` restantes (enchâssements entre classes) et `lien-parent` : 2.15, après les études 2.9 et 2.10.
- Les descentes `pico8_qh_of`, `pico8_nl_of`, `pico8_g__of` et les casts de famille : ils ne dépendent que de l'offset 0 de l'en-tête, que 2.3 garde. 2.14 et 2.15.
- Mettre `me`/`t` à zéro dans les constructeurs hors `ke` (G2).
- Retirer les `#include "pico8_ke.h"` devenus inutiles aux en-têtes (G4).
- Les 17 paires d'inclusion restantes : 2.11 puis 2.17.
- Le masque `nu` que le noyau portera à 2.7 : 2.7 ajoutera un membre à l'en-tête. Les initialiseurs désignés de 2.3a le supportent sans retouche.
- `tools/composition_residue.sh`, son TU, `COMPOSITION_RESIDUE_GUARD`.
- `make raylib`, l'appli raylib, l'audio, PSX (l'en-tête grandit de 16 octets par entité hors `ke`, à mesurer quand PSX reprendra).

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Initialiseurs désignés `{.cls = &…}`** plutôt que positionnels `{&cls, NULL, 0}`. Les deux passent `-Wextra` et donnent les mêmes octets. Le désigné ne nomme pas la forme de l'en-tête : 2.7 pourra lui ajouter le masque sans toucher ces onze sites. Coût : `--champs` compte neuf écritures de `cls` de plus (déclaré).
- **G2. `me`/`t` non posés sur les entités hors `ke`.** Le plan le dit (« portent désormais `me`/`t` sans les lire »). Les mettre à zéro changerait le code de cinq constructeurs et d'une vue pour un champ que personne ne lit ; ce n'est pas un refactor. C'est écrit au contrat de `struct.h`.
- **G3. `ke_of` et `is_me` des TU gardent leur nom** : ils prennent l'en-tête, et aucun compteur ne les voit.
- **G4. Les `#include "pico8_ke.h"` des en-têtes restent** après 2.3b/2.3c : les `.c` s'en servent par ce chemin, et les couper n'est pas l'objet de l'étape.
- **G5. Pas d'`assert` dans `pico8_lt`** : aucun n'est sûr (voir « Sur l'`assert` de `pico8_lt` »). Le contrat écrit à `entity.h` le remplace.
- **G6. Méthode de 2.3b par le compilateur**, pas par un `sed` : un `sed` de `ke.header` toucherait l'arbre `jo`, qui ne change qu'en 2.3c.

### Surprises de mesure, pour `plan-pilot`

1. **`-Wmissing-field-initializers`** : le plan ne prévoyait pas les douze initialiseurs. Toute étape qui ajoute un membre à l'en-tête (2.7) les retrouvera si elle revient au positionnel.
2. **173 lignes, pas 172** : la 173ᵉ est le commentaire `probe.h:90`.
3. **Objets de TU « différents » en refactor pur** : un `assert` embarque son expression en texte. Toute étape qui renomme un chemin lu dans un `assert` de TU aura cet écart ; la preuve est la compilation sous `-DNDEBUG`.
4. **`--champs`** : `me`/`t` passent de `pico8_ke_t` à `pico8_struct_header_t` ; `cls` gagne neuf écritures hors constructeur.
5. **Piège de mutation** : `mv` d'une copie de sauvegarde rend un `mtime` ancien, et `make` garde l'objet muté. Il faut `cp` puis `touch`.
6. **ASan des TU** : sans `detect_leaks=0`, des TU fuient. L'implémenteur vérifie au parent que ce n'est pas un effet de 2.3.

### Questions pour l'humain

Aucune qui change le plan. Les six choix ci-dessus sont pris par l'agent et déclarés au commit.
