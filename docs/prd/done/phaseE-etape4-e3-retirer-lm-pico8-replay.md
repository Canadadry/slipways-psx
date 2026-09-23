---
title: "Plan 2, Phase E3 : -lm retiré du lien de pico8_replay"
description: "La cible de lien du cœur ($(TMP_DIR)/pico8_replay) passe encore -lm alors qu'aucun de ses objets n'appelle plus libm ; tant que -lm y est, le point 6.2 n'est vrai que dans le lien d'essai de float-link, pas dans le build réel."
status: needs-triage
---

## Problem Statement

Texte de l'étape, recopié de `docs/plan-2.md` (§7, Phase E) :

> ### Phase E — Fermeture
> Seule porte de E : les quatre points du §6. Décisions humaines prises en fin de Phase D :
> - **§6.1 porte sur le cœur seul**, conformément au §4 : `CORE_SRCS` et les en-têtes qu'ils incluent par défaut (sans les définitions `PICO8_FLOAT_*`). Le reste de `src/` (backend raylib, visualiseur, builds d'audit) est exclu par une liste versionnée, une ligne de justification par fichier.
> - **§6.4 se lit « toute divergence justifiée par une ligne du relevé »** : C2b (bloc A) et D1 (bloc B) sont admis.
> - **Le build d'observation est retiré** (`make float-observe`, `src/pico8_float_observe.*`) : plus aucun cast à observer, l'audit avant/après le remplace. ✅ fait en E1.
> - **TROU-1 est restreint, pas fermé** : il ne décrit plus que `sin`/`cos` hors des 64 points du relevé.
>
> Étapes :
> - **E3 — `-lm` retiré de `$(TMP_DIR)/pico8_replay`** seulement ; il reste sur les variantes couverture/audit, les tests qui incluent `math.h`, le générateur et `main_raylib`.

**Où en est-on.** E0 a nommé « la cible de lien du cœur » du §6.2 : `$(TMP_DIR)/pico8_replay`, dont les sources sont `REPLAY_SRCS` (`src/main.c`, `src/pico8_screencast.c`, `ENGINE_SRCS`). Sa règle se termine par `-lm`. La cible `make float-link` lie **les mêmes objets** sans `-lm` dans un binaire jetable (`tmp/float-link/pico8_replay`, jamais lancé) : elle passe depuis E0 (`float-link objets 62 non-resolus 0`). Mesuré en préparant ce PRD : ni le binaire réel ni le binaire d'essai n'ont `libm.so` en `NEEDED` (`readelf -d` : seulement `libc.so.6`, l'éditeur de liens retire `-lm` inutilisé) — le `-lm` de la règle est donc déjà mort, mais il **autorise** en silence tout futur appel à `libm` depuis le cœur.

Le problème, pour celui qui ferme le chantier : le §6.2 dit « la cible de lien du cœur perd `-lm`, et le build passe ». Aujourd'hui c'est vrai du lien d'essai, pas de la règle que `make test` exécute ; le critère n'est donc pas tenu par le build lui-même, et un appel à `sin()` réintroduit dans le cœur se lierait sans bruit.

## Solution

Retirer `-lm` de la **seule** ligne de lien de `$(TMP_DIR)/pico8_replay`. Rien d'autre ne change dans le `Makefile` : `-lm` reste, comme le plan l'énumère, sur

- les variantes couverture et audit (`pico8_replay_coverage`, `pico8_replay_audit` — ce dernier lie `pico8_float_audit.c`, qui inclut `math.h`) ;
- les tests (dont ceux qui incluent `math.h` : `test_pico8_fixed`, sa variante UBSan, `test_pico8_vec`, `test_pico8_playability`, `test_pico8_slipway_drag`, `test_pico8_empty_planet_cleared`, `test_float_link`) — les autres tests qui portent `-lm` sans en avoir besoin ne sont pas touchés non plus (« seulement ») ;
- le générateur de table (`gen_quarter_wave`) ;
- `main_raylib` (`RAYLIB_LDLIBS`, dont raylib lui-même a besoin).

Après E3, la ligne de lien réelle et celle de `float-link` sont la même commande sur les mêmes objets, sans `-lm` : les deux résultats doivent concorder.

Résultat attendu, à mesurer et consigner (pas à supposer) :

- `make replay` (donc `make test`) construit `tmp/pico8_replay` sans `-lm` : lien réussi.
- `readelf -d tmp/pico8_replay` : aucune entrée `NEEDED` vers `libm`. `nm -u tmp/pico8_replay` : aucun symbole de `libm` (`sin`, `cos`, `pow`, `sqrt`…).
- `make float-link` → code 0, `float-link objets 62 non-resolus 0` — concordant avec le lien réel.
- `make frames-check` identique sur les 12 fichiers de `FRAMES_EVENTS`, **sans** `frames-baseline`.
- `make float-grep`, `make float-guard`, `make frames-ledger` : codes et lignes inchangés par rapport à l'« État de E2 ».
- `make test` vert.

## User Stories

1. En tant qu'agent qui ferme la Phase E, je veux que la règle de lien de `pico8_replay` ne passe plus `-lm`, afin que le point 6.2 soit vrai du build que `make test` exécute et pas seulement d'un lien d'essai.
2. En tant que mainteneur du cœur, je veux qu'un appel à une fonction de `libm` réintroduit dans le cœur fasse échouer le lien de `pico8_replay` (référence non résolue), afin que la régression se voie au prochain `make test`.
3. En tant qu'agent, je veux que `make float-link` et le lien réel concordent (même commande, mêmes objets, même résultat), afin qu'aucun des deux ne contredise l'autre dans l'« État ».
4. En tant qu'agent, je veux vérifier par `readelf -d` et `nm -u` que le binaire n'a ni dépendance ni symbole `libm`, afin que « le build passe » ne repose pas seulement sur le code de retour de l'éditeur de liens.
5. En tant que mainteneur des outils d'audit, je veux que `pico8_replay_audit` et `pico8_replay_coverage` gardent `-lm`, afin que l'audit avant/après (qui recalcule les anciennes expressions `double`) et la couverture restent constructibles.
6. En tant qu'auteur de tests, je veux que les tests gardent `-lm`, afin que ceux qui incluent `math.h` pour leurs références se lient toujours.
7. En tant que mainteneur, je veux que le générateur de table de sinus garde `-lm`, afin que `make sin-table` reste possible.
8. En tant que mainteneur du binaire interactif, je veux que `main_raylib` garde `-lm`, afin que `make raylib` lie toujours (sans jamais le lancer).
9. En tant que mainteneur, je veux que les tests qui portent `-lm` sans inclure `math.h` ne soient pas touchés, afin que l'étape reste d'une seule ligne, comme le plan le dit (« seulement »).
10. En tant qu'agent, je veux constater `make frames-check` identique sans `frames-baseline`, afin de prouver que le binaire se comporte à l'identique.
11. En tant qu'agent, je veux relancer les quatre cibles du §6 et consigner leur ligne et leur code dans un « État de E3 » de `docs/plan-2.md`, et marquer la puce E3 « ✅ faite », afin qu'E4 parte d'un état mesuré.
12. En tant que lecteur du plan, je veux que l'« État de E3 » dise que 6.2 est désormais tenu par le lien de `pico8_replay` construit par `make test`, afin qu'E4 puisse décider de la place de `float-link` en connaissance de cause.

## Implementation Decisions

- **Un seul changement de code** : le jeton `-lm` de la ligne de lien de `$(TMP_DIR)/pico8_replay` dans le `Makefile`. `REPLAY_SRCS`, la règle de `float-link`, `tools/float_link.sh`, `RAYLIB_LDLIBS` et toutes les autres lignes de lien sont inchangés.
- **Pas de commentaire ajouté dans le `Makefile`** : la règle de CLAUDE.md n'y admet que l'aide à la lecture d'une commande, et une ligne sans `-lm` se lit seule. La justification va dans le message de commit et dans l'« État de E3 ».
- **La légende du `Makefile`** (`make float-link -> lie les objets de tmp/pico8_replay sans -lm (§6.2)…`) reste vraie et n'est pas touchée ; la question de sa place dans la suite est pour E4.
- **Aucun `*.frames.hash` ne change** : pas de rapport, pas de ligne de registre.
- **Aucune modification de `docs/plan-2.md` hors de l'« État de E3 » et de la puce E3** : la réécriture du §6.2 relève d'E4.

## Testing Decisions

- **Un bon test ici est un fait de lien, observable sans lancer le binaire au-delà de ce que `make test` fait déjà** (le rejeu headless) : lien réussi, pas de `libm` en `NEEDED` ni en symbole non défini, `float-link` concordant, `frames-check` identique, `make test` vert.
- **Aucun nouveau test automatisé** : le lien de `pico8_replay`, construit par `make test` via `replay`, **est** le verrou du 6.2 après cette étape (c'est ce que dit la puce E4).
- **Vérification ponctuelle du verrou, consignée et non versionnée** (optionnelle mais recommandée) : sur une copie jetable d'un fichier du cœur, ajouter un appel à une fonction de `libm` échoue déjà à la compilation sans `math.h` (déclaration implicite, `-Werror`), et sous la garde avec `math.h` (retour `double`) ; le lien sans `-lm` est la ligne de défense qui ne dépend d'aucun de ces deux mécanismes. Si l'implémenteur la fait, il consigne la commande et le message.
- **Prior art** : `make float-link` (E0), et la mesure `nm tmp/pico8_replay` de l'« État de E1 ».

## Out of Scope

- Retirer `-lm` de toute autre cible (tests, audit, couverture, générateur, `main_raylib`, `test_float_link`), même quand il y est inutile.
- Supprimer `make float-link` ou le faire entrer dans `make test` : E4.
- Toute modification de source (`src/`, `tests/`, `tools/`).
- La vérification `mipsel` soft-float (hors périmètre, décision humaine en Phase D) ; le volet `malloc` (§8).
- Lancer l'appli raylib : jamais (`make raylib` peut être construit pour vérifier que son lien garde `-lm`, jamais exécuté).

## Further Notes

- `-lm` était déjà sans effet sur le binaire (l'éditeur de liens de l'hôte ne garde pas une bibliothèque inutilisée) ; l'étape ne change donc pas le binaire produit, elle change ce que la règle **tolère**. Si l'implémenteur compare l'ancien et le nouveau `tmp/pico8_replay` (`cmp`), un binaire identique est un bonus consignable, pas une exigence.
- Un clone où `make raylib-vendor-setup` n'a pas été fait ne peut pas construire `main_raylib` ; ce n'est pas une porte de l'étape.
