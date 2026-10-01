---
title: "Plan 3, étape 2.4 : `pk` n'est plus qu'une vérité et `fp` qu'un nombre, faux valant 0 ; `pk_is_bool`/`fp_is_bool` retirés (`un-ou-faux` 27 → 0, gardé ; `défauts` 701 → 698) — un commit, jeu identique"
description: "Deux champs de composant portaient un drapeau « je suis un booléen » à côté de leur valeur. Leur seul lecteur n'a besoin que de la vérité (`pk`) ou du nombre, faux compris comme 0 (`fp`) : les drapeaux, leurs défauts et leurs écritures partent, les deux lectures se simplifient, et la catégorie `un-ou-faux` entre dans la garde."
status: needs-triage
---

## Problem Statement

**Écrit sur `9404540`** (branche `claude/plan-pilot-3-7zdft0`), pendant que 2.3a/b/c s'implémentent en parallèle. Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les comptes et les preuves viennent de deux copies jetables de `9404540` (`p`, le parent, et `a`, 2.4 appliquée), supprimées depuis.

**Ce qui dépend de 2.3.** 2.4 part de l'arbre **après 2.3c**. Aucun site de 2.4 n'est touché par 2.3 : 2.3 réécrit ses lignes **sur place** dans `probe.c` (`:31`, `:41`, `:230`, `:367`, `:447`, `:488`, `:503`), `planet_family.c`, `planet_family_trade.c`, `slipway.c`, `wormhole.c`, `probe.h` (`:42`, `:90`) et les TU. Elle n'ajoute ni ne retire de ligne dans les fichiers de 2.4. Les suppressions de lignes de 2.3 sont ailleurs : `struct.h`, `ke.h`, `entity.h`, `entity.c`, `bootstrap.c`. Les numéros de ligne ci-dessous sont donc ceux de `9404540` et **devraient** rester valables après 2.3c. L'implémenteur les remesure. Deux chemins de TU cités ici s'écrivent `jo->ke.header.…` à `9404540` et `jo->header.…` après 2.3c : 2.4 ne les touche pas.

**Comptes.** Ceux du parent de 2.4 sont ceux que 2.3c annonce : pied **1344 / 91**, `défauts 701`, `un-ou-faux 27 14`. L'écart de 2.4, lui, est mesuré sur `9404540` : **−24 lignes, 0 fichier**, `un-ou-faux` 27 → 0, `défauts` 701 → 698. Aucune ligne de 2.4 n'est comptée par `parente` ou `inclusion-mutuelle`, les deux seules catégories que 2.3 change. L'écart reste donc le même sur l'arbre 2.3c, et l'arrivée attendue est **1320 / 91**, comme dans le plan. Si 2.3c arrive à un autre pied, l'arrivée de 2.4 est ce pied moins 24, et le message l'explique.

Vu du mainteneur du port : dans le cart, deux champs mélangent un nombre et `false` :

- **`pk`**, clé d'émetteur de particules. Il vaut `1` pour `g_`, `fv`, `hj`, `mx` et `f` (faux) pour `pm` (`lua.lua:716`, `:1006`, `:1023`, `:1043`, `:1155`). Son **seul** lecteur, `g_:mm` (`lua.lua:729`, `self.pk and v:pa() or v`), n'en lit que la **vérité**.
- **`fp`**, clé de sonde. Il vaut `f` pour `jc` et `g_` (`lua.lua:616`, `:712`) et un nombre (1 ou 2) pour `mz`, `qh`, `nr` (`:758`, `:910`, `:1103`). Son **seul** lecteur, `d_` (`lua.lua:571`, `fm:nx(self.fp or 0)`), n'en lit que le **nombre**, 0 pour faux.

Le port garde l'ambiguïté du cart sous la forme de deux drapeaux, `pk_is_bool` (`particle.h:36`) et `fp_is_bool` (`probe.h:31`). Ils sont écrits par 9 lignes d'init et par 3 défauts (`game_data.h:341`, `:362`, `:433` ; `game_data.c:565`, `:586`, `:656`), et lus seulement par `probe.c:337` et `:372`. Ce sont les 27 lignes `un-ou-faux` du compteur. Ils mourraient de toute façon à 2.12 : leur écrire un archétype serait du travail perdu (plan, « Champs qui meurent avant les archétypes »).

**Pourquoi le retrait ne change aucune valeur.** Aucun nombre 0 n'est jamais rangé dans `pk` ni dans `fp` par une classe qui les fait lire :

- **`pk`** vaut `0x10000` (le nombre 1) pour `g_`, `fv`, `hj`, `mx`, et 0 (faux) pour `pm`. `ga` y range 0 avec `pk_is_bool = 0` (`planet_family.c:582-583`), mais sa table a son propre `mm` (`pico8_ga_mm`, `planet_family.c:558`), qui ne lit pas `pk` : `g_:mm` n'est jamais appelé sur un `ga`.
- **`fp`** vaut 0 pour `jc` et `g_` (faux, `game_data.h:186`, `:199`, « fp is the spec's false ») et `0x10000` ou `0x20000` pour les autres.

Donc « `pk != 0` » redonne la vérité de Lua, et « `fp` tel quel » redonne `self.fp or 0`.

### Texte du plan, recopié (`docs/plan-3.md` à `9404540`, ligne 2.4)

> - 2.4 — `un-ou-faux` : `pk` devient la vérité (1 ou 0) que seul `probe.c:372` lit (`self.pk and …`, `lua.lua:729`), `fp` le nombre que seul `probe.c:337` lit (`self.fp or 0`, `lua.lua:571`), 0 pour faux ; `pk_is_bool`/`fp_is_bool` retirés des composants (`particle.h:36`, `probe.h:31`), des défauts (`game_data.h:341`, `:362`, `:433`) et des inits. Les plis de TU (`hj`, `mx`, `pm`) gardent leurs octets : chaque TU connaît le genre constant de sa classe. Modules : `particle`, `probe`, `planet_family`, `planet_family_trade`, `slipway`, `wormhole`, `game_data`, TU `hj`, `jc`, `mx`, `pm`, `jo_spec_defaults`. Compte : `un-ou-faux` 27 → **0**, gardé ; `défauts` 701 → **698** (trois copies `pk_is_bool = pico8_X_start.pk_is_bool` partent) ; pied → **1320 / 91**. Preuves : chemin touché (`gcov` de `probe.c:337`, `:372` ; mutations : `pk` de `pm` à 1, `fp` faux lu comme 1, `fp` de `qh` à 0). `class-coverage-net` : oui (inits touchées). `sanitize-net` : non. Budget ≈ 50 min.

Règles communes de la phase (plan, « Preuves communes à chaque ligne ») : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée. Une catégorie tombée à 0 entre dans `COMPOSITION_RESIDUE_GUARD`, avec une preuve de garde mordante.

### État de départ attendu (après 2.3c)

| Commande | Résultat attendu |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 262 33`, `défauts 701 52`, `nu 208 37`, `aiguillage-nom 30 8`, **`un-ou-faux 27 14`**, `void-ptr 11 7`, `ptrs-entité 7 4`, `atteinte-à-prouver 1 1`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 1344 ligne(s), 91 fichier(s)`** |
| `sh tools/composition_residue.sh --check un-ou-faux` | 27 lignes (liste ci-dessous), code 1 |
| `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` | `COMPOSITION_RESIDUE_GUARD := noyau-inclut` |
| `make test` ; `make e2e` | vert ; 26 conformes |
| `make line-coverage` sur les 26 e2e | 6343 lignes (6345 à `9404540`, moins les deux lignes que 2.3a supprime) |

## Solution

Un seul commit. Le code, les TU, le `Makefile` et le plan ne compilent ou ne se vérifient qu'ensemble.

- **`pk` reste un `pico8_fixed_t`**, qui vaut le nombre 1 (`0x10000`) ou 0 pour faux. Le lecteur `probe.c:372` devient `pk = emitter->pk != 0;`.
- **`fp` reste un `pico8_fixed_t`**, qui vaut le nombre, ou 0 pour faux. Le lecteur `probe.c:337` passe `probe->fp` tel quel.
- Les deux drapeaux, leurs trois défauts, leurs trois copies de défaut et leurs six autres écritures partent.
- Les TU cessent de lire les drapeaux. Le pli de `pk` dépend désormais de la classe, que chaque TU connaît, et non plus d'un drapeau. Il rend les mêmes octets, donc aucune constante d'empreinte ne bouge.
- `un-ou-faux` entre dans la garde : `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux`.

## User Stories

1. En tant qu'auteur des archétypes (2.12), je veux que `pk_is_bool` et `fp_is_bool` n'existent plus, afin de ne pas leur écrire un archétype qui mourrait aussitôt.
2. En tant que lecteur de `g_:mm`, je veux lire `pk` comme une vérité, sans drapeau à côté, afin de retrouver `self.pk and …` du cart en une ligne.
3. En tant que lecteur de `d_`, je veux lire `fp` comme un nombre, 0 pour faux, afin de retrouver `self.fp or 0` du cart en une ligne.
4. En tant que gardien du jeu, je veux `make e2e` à 26 conformes et aucun `*.sha256` touché, afin que le jeu reste le même image par image.
5. En tant que gardien des empreintes, je veux qu'aucune constante `*_HASH` ou `FROZEN_*` ne change, afin d'être sûr que les TU plient les mêmes valeurs après la réécriture de leur pli de `pk`.
6. En tant que vérificateur, je veux le `gcov` des deux lectures réécrites et l'ensemble `L` des lignes perdues, afin de savoir qu'aucune ligne ne perd son seul test.
7. En tant que vérificateur, je veux des mutations sur la vérité de `pk` et le nombre de `fp`, rejouées au parent, afin de prouver que les tests voient encore ces deux lectures.
8. En tant que gardien du compteur, je veux `un-ou-faux` dans `COMPOSITION_RESIDUE_GUARD`, et la preuve qu'une ligne réinjectée rougit `make test`, afin que le drapeau ne revienne pas sans bruit.
9. En tant que lecteur de `particle.h` et `probe.h`, je veux que le contrat de `pk` et `fp` (ce que vaut faux) soit écrit, afin de ne pas y ranger un nombre 0 que Lua dirait vrai.
10. En tant que lecteur du plan, je veux une note « Découvert à 2.4 » courte, afin que 2.5 parte des vrais comptes.

## Implementation Decisions

### Modules

`particle`, `probe`, `planet_family`, `planet_family_trade`, `slipway`, `wormhole`, `game_data` ; TU `hj`, `jc`, `mx`, `pm`, `jo_spec_defaults` ; `Makefile` ; `docs/plan-3.md`. Comme le plan, sans écart.

### Ce qui change, fichier par fichier (numéros de `9404540`)

**`src/pico8_particle.h`**
- `:36` `int pk_is_bool; /* … */` : ligne retirée.
- `:29`, commentaire du struct : ajouter, après « a `ga` holds 0 there. », la phrase « `pk` is the cart's 1 or false, false held as 0: only its truth is read. » C'est un piège à ne pas réintroduire (règle 3 de `CLAUDE.md`) : un nombre 0 rangé là serait vrai en Lua et faux ici.

**`src/pico8_probe.h`**
- `:31` `int fp_is_bool; /* … */` : ligne retirée.
- `:21-22`, commentaire du struct : « A fixed `fp` is truthy even at 0, and `d_` hands it to `fm:nx`; a boolean one hands 0. » → « `fp` is what `d_` hands to `fm:nx`: the cart's false is held as 0, which is what `self.fp or 0` hands. »

**`src/pico8_probe.c`**
- `:336-337` : le commentaire « A boolean `fp` hands 0 … » part, et la ligne devient `pico8_hy_nx(pico8_fm_hy(), probe->fp);`.
- `:371-372` : le commentaire « A fixed `pk` is truthy in Lua, 0 included. » part (son contenu passe à `particle.h`), et la ligne devient `pk = emitter->pk != 0;`.
- `:462` `self->probe->fp_is_bool = 1;` et `:471` `self->emitter->pk_is_bool = 0;` : retirées (init de `g_`).

**`src/pico8_planet_family.c`**
- `:72` `self->probe->fp_is_bool = 0;` (`qh`), `:299` `self->emitter->pk_is_bool = pico8_hj_start.pk_is_bool;` (`hj`), `:408` `self->hj.emitter->pk_is_bool = pico8_pm_start.pk_is_bool;` (`pm`), `:583` `self->emitter->pk_is_bool = 0;` (`ga`) : retirées. `:582` `self->emitter->pk = 0;` (`ga`) **reste**.

**`src/pico8_planet_family_trade.c`**
- `:384` `self->hj.emitter->pk_is_bool = pico8_mx_start.pk_is_bool;` (`mx`) : retirée.

**`src/pico8_slipway.c`**, **`src/pico8_wormhole.c`**
- `slipway.c:414` `self->probe->fp_is_bool = 1;` (`jc`), `wormhole.c:262` `self->probe->fp_is_bool = 0;` (`mz`) : retirées.

**`src/pico8_game_data.h`**, **`src/pico8_game_data.c`**
- `game_data.h:341` (`hj`), `:362` (`pm`), `:433` (`mx`) `int pk_is_bool;` : retirées.
- `game_data.c:565`, `:586`, `:656` `.pk_is_bool = …,` : retirées. Les valeurs `.pk` restent (`0x10000`, `0`, `0x10000`) : ce sont déjà la vérité attendue.
- Les commentaires « fp is the spec's false. » (`game_data.h:181`, `:193`) et les `int fp;` de `jc`/`g_` restent : ils disent toujours vrai, et 0 est la valeur lue.

**Tests** (aucun `*_HASH` ni `FROZEN_*` touché)
- `test_pico8_hj.c:185-187` `fold_pk` : `return ((const pico8_struct_header_t *)hj)->cls == pico8_pm_struct_class() ? fold_bool_member(h, hj->emitter->pk != 0) : fold_fixed_member(h, hj->emitter->pk);`. Ce TU plie des `hj` (`fold_ascension`, `:316`) **et** des `pm`/`mx` (`fold_heir`, `:562`) : c'est le seul des trois où le genre dépend de l'instance. Le cast vers l'en-tête s'écrit pareil avant et après 2.3c. `cls_of` (`:234`) est défini après `fold_pk` et n'est pas utilisable sans déplacer du code.
- `test_pico8_mx.c:160` : `return fold_fixed_member(h, hj->emitter->pk);`. Le seul appelant, `fold_synthesizer` (`:308`), ne plie que des `mx`.
- `test_pico8_pm.c:142` : `return fold_bool_member(h, hj->emitter->pk != 0);`. Le seul appelant, `fold_protostar` (`:305`), asserte déjà `cls_of(e) == pico8_pm_struct_class()` (`:291`).
- `test_pico8_hj.c:585-586` : `… == 0 && as_hj(pm)->emitter->pk_is_bool && as_hj(pm)->emitter->pk == 0` → `… == 0 && as_hj(pm)->emitter->pk == 0`, sur une ligne.
- `test_pico8_jc.c:444` : `assert(jc->probe->fp_is_bool && jc->probe->fp == 0);` → `assert(jc->probe->fp == 0);`.
- `test_pico8_jo_spec_defaults.c:157`, `:190`, `:300` : le second terme `&& X->emitter->pk_is_bool == pico8_X_start.pk_is_bool` part. `X->emitter->pk == pico8_X_start.pk` reste. Ces trois lignes restent comptées par `défauts`, qui compte aussi les TU (voir « Comptes »).

**`Makefile`**
- `:864` `COMPOSITION_RESIDUE_GUARD := noyau-inclut` → `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux`. Rien d'autre : la cible `composition-guard` et la légende parlent déjà de la variable.

**`docs/plan-3.md`**
- Note « Découvert à 2.4 » au bout de la ligne 2.4. Elle donne les comptes, la valeur de la garde, et ce qui change pour une étape suivante : à 2.12, les archétypes de `hj`, `pm`, `mx`, `g_`, `jc`, `mz`, `qh` n'ont plus de drapeau. Le reste va au message de commit.

Rien d'autre : ni `glossaire-noms.md` (les entrées `pk` `:543` et `fp` `:601` restent justes, et aucune ne nomme les drapeaux), ni `trous-de-portage.md` (aucun écart avec le cart n'est créé ni corrigé), ni `tools/composition_residue.sh`, ni son TU. `test_composition_residue.c:309-331` écrit déjà ses exemples `pk_` `"is_bool"` en deux morceaux, donc le TU du compteur ne compte pas pour lui-même.

### Disposition mémoire

`pico8_emitter_t` et `pico8_probe_t` perdent un `int` chacun, et les structs qui embarquent `emitter_own`/`probe_own` rapetissent. Rien ne dépend de ces offsets : aucun `offsetof` dans `src/` ni `tests/`, aucun `sizeof`/`memcpy` d'un émetteur ou d'une sonde (`git grep` mesuré), et tout accès passe par `emitter_of`/`probe_of` ou le membre nommé. `sanitize-net` n'est pas exigé (durée de vie et propriétaire inchangés, comme le dit le plan). Aucune sonde de taille n'est exigée.

### Comptes, mesurés sur copie de `9404540`, transposés à l'arbre 2.3c

| Catégorie | parent `9404540` → 2.4 | attendu : parent 2.3c → 2.4 |
|---|---|---|
| `un-ou-faux` (lignes / fichiers) | 27 / 14 → **0**, disparue de la sortie | 27 / 14 → **0**, gardée |
| `défauts` | 701 / 52 → **698 / 52** (`planet_family.c` 82 → 80, `planet_family_trade.c` 32 → 31) | 701 → **698** |
| les dix autres | inchangées | inchangées (`parente 10 3`, `inclusion-mutuelle 17 16`, …) |
| pied | 1362 / 93 → **1338 / 93** (−24, 0 fichier) | 1344 / 91 → **1320 / 91** |

Pourquoi −24 et pas −27 : les trois lignes `test_pico8_jo_spec_defaults.c:157`, `:190`, `:300` sortent de `un-ou-faux` mais restent dans `défauts`, qui compte les TU (`pico8_X_start.pk`). Les trois lignes de `défauts` qui partent sont les trois copies de `src/` (`planet_family.c:299`, `:408`, `planet_family_trade.c:384`), qui étaient aussi `un-ou-faux`. Aucun fichier ne quitte le pied : chacun des 14 garde une ligne d'une autre catégorie.

Les 27 lignes, classées pour le message (numéros du parent) :
- **Retirées (19)** : `particle.h:36` ; `probe.h:31` ; `game_data.h:341`, `:362`, `:433` ; `game_data.c:565`, `:586`, `:656` ; `probe.c:462`, `:471` ; `planet_family.c:72`, `:299`, `:408`, `:583` ; `planet_family_trade.c:384` ; `slipway.c:414` ; `wormhole.c:262` ; `test_pico8_hj.c:585`, fusionnée avec `:586`.
- **Réécrites hors motif (8)** : `probe.c:337`, `:372` ; `test_pico8_hj.c:186` ; `test_pico8_mx.c:160` ; `test_pico8_pm.c:142` ; `test_pico8_jc.c:444` ; `test_pico8_jo_spec_defaults.c:157`, `:190`, `:300` (ces trois restent en `défauts`).

Lignes qui gagnent une catégorie : aucune (`--detail` avant/après, seules `défauts` et `un-ou-faux` bougent).

### Anti-tassement

- **Interdit** :
  - renommer un drapeau pour échapper au motif `\b(pk|fp)_is_bool\b` (par exemple `pk_bool`, `pk_kind`, `pk_is_true`), ou le ranger dans un autre champ, un bit, une table à côté, une union ;
  - lire la classe dans `probe.c` pour savoir si `pk`/`fp` est un booléen : le lecteur n'a plus de genre à connaître ;
  - changer `.pk`/`.fp` d'un défaut, `tools/composition_residue.sh`, `tests/test_composition_residue.c`, `tests/e2e/`, un `*.sha256` ou une constante d'empreinte ;
  - retirer `un-ou-faux` de la garde, ou garder une ligne dans un fichier exclu du compteur.
- Le pli de TU par classe (`hj`) n'est **pas** un drapeau déguisé : c'est le TU qui plie, pas le jeu, et la garde ne compte pas un nom de classe.

## Implementation Chunks

Un commit, ≈ 40 min preuves comprises, sous le budget de 50 min. Il n'y a donc pas de découpage.

1. Retirer les 19 lignes et réécrire les 8 (liste ci-dessus), plus les deux commentaires de struct.
2. Changer la garde du `Makefile`.
3. Lancer les preuves (ci-dessous), écrire la note du plan, commiter, pousser.

**Repli écrit (si le budget déborde avant le commit).** Le code ne se coupe pas utilement : un drapeau retiré du composant casse la compilation de toutes ses écritures. On peut en revanche couper par drapeau, et chaque moitié compile seule :
- **2.4a** : `fp` (`probe.h:31`, `probe.c:337`, `:462`, `planet_family.c:72`, `slipway.c:414`, `wormhole.c:262`, `test_pico8_jc.c:444`) ; `un-ou-faux` 27 → 20, pied → 1337 / 91 (à remesurer), sans toucher la garde ;
- **2.4b** : `pk` (le reste), `un-ou-faux` → 0, `défauts` → 698, garde, pied → 1320 / 91.

Un repli est écrit dans la note « Découvert à ». Les comptes finaux ne changent pas.

## Testing Decisions

- **Aucun TU nouveau.** Le comportement ne change pas. Les TU existants lisent `pk` et `fp` par le jeu (`g_:mm`, `d_`) et par le pli, et les mutations prouvent qu'ils les voient. Les TU ne rejouent jamais un `events.json`. Les e2e partent de l'écran titre (`make e2e`), comme toujours.
- **Bon test ici** : il lit la vérité de `pk` et le nombre de `fp` par ce que le jeu en fait (particule de `g_:mm`, appel à `fm:nx`) ou par la valeur du membre, jamais par un drapeau.
- **Prior art** : ensemble `L` de D.20a et 2.3a ; mutations avec la règle du parent de D.21 ; garde mordante de 2.2 (M1 de `plan3-phase2-etape1-2-includes-morts-noyau.md`).

### Preuves (chemin touché)

1. `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide.
2. **`gcov` des e2e** (`make line-coverage EVENTS="$(ls tests/e2e/*.json | tr '\n' ' ')"`, dans une copie) au parent puis après. `L` est calculé en appariant les lignes par `diff`. Mesuré sur `9404540` : 6345 → **6336** lignes. **`L` = les 9 écritures de drapeau retirées** : `probe.c:462`, `:471` ; `planet_family.c:72`, `:299`, `:408`, `:583` ; `planet_family_trade.c:384` ; `slipway.c:414` ; `wormhole.c:262`. Les deux lectures réécrites sont **exécutées** au parent (`probe.c:337`, `:372`) et après (`:336`, `:370`, numéros de l'enfant). Sur l'arbre 2.3c, on attend 6343 → 6334, avec le même `L`. Toute autre ligne dans `L` : arrêt, rapporté.
3. **Mutations**, une à la fois, à l'enfant puis la même au parent (règle du parent). Mesuré sur copie : les résultats sont identiques au parent et à l'enfant, sauf M5, qui est rouge aux deux pour les TU. **ÉCHEC** si une mutation est verte à l'enfant et rouge au parent. Une mutation verte aux deux est déclarée « chemin jamais atteint ». Restaurer par `cp` puis `touch` (piège de 2.3 : `mv` rend un `mtime` ancien).

| # | Mutation (enfant ; au parent, la même sur l'ancienne forme) | TU rouges (`make -k unit`) | e2e conformes |
|---|---|---|---|
| M1 | `pk` de `pm` à 1 : `game_data.c` `pico8_pm_start` `.pk = 0` → `0x10000` (au parent, `.pk_is_bool = 1` gardé, donc `pk != 0` vrai aussi) | `hj`, `pm` | 26 / 26 (enfant et parent) |
| M2 | `fp` faux lu comme 1 : `probe.c:336` `probe->fp` → `probe->fp != 0 ? probe->fp : pico8_fixed_from_int(1)` ; au parent, `:337` `fp_is_bool ? pico8_fixed_from_int(0)` → `…from_int(1)` | `hp`, `hy`, `lq` | 8 / 26 (enfant et parent) |
| M3 | `fp` de `qh` à 0 : `game_data.c` `pico8_qh_start` `.fp = 0x20000` → `0` | `hj`, `mx`, `nl`, `os`, `pm`, `qh` | 20 / 26 (enfant et parent) |
| M4 | vérité de `pk` forcée : `probe.c:370` `pk = emitter->pk != 0;` → `pk = 1;` ; au parent, `:372` → `pk = 1;` | `hj`, `pm` | 26 / 26 (enfant et parent) |
| M5 | `fp` de `g_` à 1 : `game_data.c` `pico8_g__start` `.fp = 0` → `0x10000` | `fv`, `g_` (enfant et parent) | **8 / 26** à l'enfant ; 26 / 26 au parent |

   Lecture : M1 et M4 ne sont vues que par les TU `hj`/`pm`, parce que la vérité de `pk` de `pm` ne change aucune image des 26 e2e. C'est le même filet qu'au parent, donc pas de perte. M5 montre ce qui change vraiment : au parent, le drapeau **masquait** le nombre d'un `fp` faux ; à l'enfant, le nombre est lu. Les TU `fv`/`g_` rougissent aux deux, par le pli `fp != 0`. Ce n'est pas un ÉCHEC (rouge à l'enfant). M5 est une mutation de donnée : aucune classe du cart ne range 1 dans le `fp` de `g_`.
4. **Garde mordante** (dans une copie) : `int fp_is_bool;` réinjecté après `pico8_fixed_t fp;` dans `probe.h` → `make test` code 2, avec `un-ou-faux: src/pico8_probe.h:31:    int fp_is_bool;`, `résidu gardé (noyau-inclut,un-ou-faux) : 1 ligne(s)`, `make: *** [Makefile:867: composition-guard] Error 1`. Mesuré.
5. `make class-coverage-net` : **OK**, identique à `tools/class_coverage_net_expected.txt` (les inits de `g_`, `qh`, `hj`, `pm`, `mx`, `ga`, `jc`, `mz` sont touchées). Mesuré : « couverture: OK », 26 rejeux. `make sanitize-net` : non exigé.
6. `--champs` : inchangé (`289 ; 15 ; 27`). `pico8_emitter_t` et `pico8_probe_t` sont des composants, pas des types d'entité de `--champs`. Mesuré.

### Greps de Sortie (sur l'arbre du commit)

1. `grep -rnE '\b(pk|fp)_is_bool\b' src tests` → **vide**.
2. `sh tools/composition_residue.sh --check un-ou-faux` → `résidu gardé (un-ou-faux) : 0 ligne(s)`, code 0.
3. `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux`.
4. `grep -n 'pk = emitter->pk != 0;' src/pico8_probe.c` → 1 ligne ; `grep -n 'pico8_hy_nx(pico8_fm_hy(), probe->fp);' src/pico8_probe.c` → 1 ligne.
5. `grep -rnE '(->|\.)pk *=' src | grep -v game_data.c` → 6 lignes, les mêmes qu'au parent (`probe.c` init de `g_`, `help.c:165`, `planet_family.c` `hj`/`pm`/`ga`, `planet_family_trade.c` `mx`).
6. `make composition-residue` → `défauts 698`, pas de ligne `un-ou-faux`, `parente 10 3`, `inclusion-mutuelle 17 16`, pied **`1320 / 91`**.
7. `git diff --stat <parent>` : seulement les 9 fichiers de `src/` (7 modules), les 5 TU, `Makefile`, `docs/plan-3.md`.

### Budget et tableau « Runs »

Budget : **≈ 50 min**, preuves comprises. Au-delà de 1,5 fois, on passe au repli, sans rien commettre d'incomplet. Jamais l'appli raylib ; `make raylib` n'est pas exigé.

L'implémenteur tient un tableau « Runs », une ligne par lancement. Voici celui des essais de ce PRD, sur deux copies de `9404540` (`p` parent, `a` 2.4 appliquée). Il est à refaire sur l'arbre 2.3c.

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make composition-residue` ; `--check un-ou-faux` ; `--detail` | `p` | 0 ; 1 ; 0 | pied 1362 / 93 ; 27 lignes | < 1 s |
| 2 | `make test` | `p` | 0 | vert | ≈ 10 s |
| 3 | `make test` ; `make e2e` | `a` | 0 ; 0 | vert ; 26 conformes | ≈ 10 s ; ≈ 1 s |
| 4 | `make composition-residue` ; `--detail` comparé | `a` | 0 | `défauts 698 52`, `un-ou-faux` absent, pied 1338 / 93 | < 1 s |
| 5 | `make composition-guard` avec la garde élargie | `a` | 0 | `résidu gardé (noyau-inclut,un-ou-faux) : 0 ligne(s)` | < 1 s |
| 6 | `make test` avec `int fp_is_bool;` réinjecté | `a` | 2 | `composition-guard` Error 1, 1 ligne | ≈ 10 s |
| 7 | `make line-coverage` sur les 26 e2e | `p` / `a` | 0 | 6345 → 6336 ; `L` = 9 écritures retirées | ≈ 30 s × 2 |
| 8 | M1 à M5, `make -k unit` + `make -k e2e`, enfant puis parent | `a` / `p` | 2 | tableau des mutations | ≈ 20 s × 10 |
| 9 | `make class-coverage-net` | `a` | 0 | `couverture: OK` | ≈ 5 s |
| 10 | `--champs` | `p` / `a` | 0 | 289 / 15 / 27 aux deux | < 1 s |
| 11 | `git grep` `offsetof`, `sizeof`/`memcpy` d'émetteur ou de sonde | `9404540` | 0 | rien | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit 2.3c (ou un descendant qui ne touche que `docs/`), ou si les comptes de départ diffèrent de 1344 / 91 et `un-ou-faux 27 14` sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) reste rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si `L` contient autre chose que les 9 écritures de drapeau retirées, ou si l'une des deux lectures réécrites n'est pas exécutée par les e2e.
- `FAILED:` si une mutation M1 à M5 est verte à l'enfant et rouge au parent. Une mutation verte aux deux est déclarée « chemin jamais atteint ».
- `FAILED:` si la garde ne vaut pas `noyau-inclut,un-ou-faux`, ou si la ligne réinjectée ne rougit pas `make test`.
- `FAILED:` si `class-coverage-net` n'est pas OK.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit, ou si les comptes d'arrivée ne sont pas `un-ou-faux` 0, `défauts` 698, pied 1320 / 91 (ou pied du parent − 24, même nombre de fichiers), sans explication.
- `FAILED:` en cas de tassement (voir « Anti-tassement »).
- `FAILED:` si le message du commit omet : les 27 sites `fichier:ligne` du parent classés, `L`, le `gcov` des deux lectures, les mutations avec leur résultat au parent, la garde mordante, `class-coverage-net`, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.4 », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Changer le type de `pk` en `int` ou en booléen C, ou ranger `1` au lieu de `0x10000` dans les défauts : ce changerait des valeurs de départ et le pli de `fv`/`g_` (`fold_fixed_member(em->pk)`), sans rien faire tomber (G1).
- Changer `int fp` des défauts de `jc`/`g_` en `pico8_fixed_t`, ou leur commentaire « the spec's false » (G2).
- Le drapeau `bg_is_bool` de `lq` et `qe_is_bool` de `trade` : le compteur ne les voit pas, et le plan ne les nomme pas.
- `pk = 0` de `ga` (`planet_family.c:582`) : c'est un défaut jamais lu, qui partira avec l'archétype de `ga` (2.12).
- `tools/composition_residue.sh`, son TU ; `sanitize-net` ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. `pk` reste un `pico8_fixed_t`, et « vrai » s'y écrit `0x10000`.** Le plan dit « la vérité (1 ou 0) ». Tous les `pk` vrais valent déjà le nombre 1 du cart, et le faux de `pm` vaut déjà 0 : aucun défaut ne change, et les plis de `fv`, `g_`, `hj`, `mx` (`fold_fixed_member`) restent justes octet à octet. Passer à `int` changerait les défauts `.pk` de `g_`, `fv`, `hj`, `mx`, leurs types, et les plis de `fv` et `g_`, sans rien gagner à la garde.
- **G2. `fp` garde ses types de défaut** (`int` pour `jc`/`g_`, où il vaut 0, faux ; `pico8_fixed_t` ailleurs). L'affectation `probe->fp = pico8_g__start.fp` convertit 0 en 0.
- **G3. Le pli de `pk` dans le TU `hj` lit la classe** par un cast vers l'en-tête, pas par `cls_of`, défini plus bas dans le fichier. Il s'écrit pareil avant et après 2.3c.
- **G4. Le contrat de `pk` (faux rangé 0, seule la vérité est lue) va dans `particle.h`**, où l'on écrit un nouveau défaut, et non plus au lecteur, qui n'a plus rien à expliquer.
- **G5. Pas de sous-commits** : l'essai tient dans le budget. Le repli par drapeau est écrit plus haut.

### Surprises de mesure, pour `plan-pilot`

1. **Pied −24, pas −27** : `défauts` compte aussi les TU, et les trois `assert` de `jo_spec_defaults` y restent.
2. **Aucun fichier ne quitte le pied** : les 14 fichiers `un-ou-faux` ont tous une autre ligne comptée.
3. **Le drapeau masquait une valeur** (M5) : au parent, le nombre rangé dans un `fp` faux n'était jamais lu ; après 2.4, il l'est. Aucune classe n'y range autre chose que 0 : c'est la preuve de l'égalité, et elle est dans les défauts.
4. **M1 et M4 ne sont vues que par deux TU** (`hj`, `pm`) : aucun e2e ne dépend de la vérité de `pk` de `pm`. C'est le même filet qu'au parent.
5. **`test_pico8_hj.c`** plie trois classes avec un seul `fold_pk` : c'est le seul TU où « le genre constant de la classe » demande un test de classe.

### Questions pour l'humain

Aucune.
