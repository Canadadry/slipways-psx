---
title: "Plan 3, étape D.19c : `pico8_value.c/.h` et `test_pico8_value.c` supprimés, 42 includes retirés (un remplacé par `pico8_fixed.h`), trois commentaires réécrits ; `valeur` = 0 et gardée"
description: "Après D.19a/b, plus aucun fichier ne se sert du module de valeur Lua : 32 en-têtes et 3 `.c` de `src/` ainsi que 9 TU l'incluent sans rien lui emprunter, et seul `test_pico8_value` appelle encore ses fonctions. D.19c le supprime en deux commits (includes de `src/`, puis suppression et garde). `pico8_audio.h` reçoit `pico8_fixed.h` en direct. La garde et le build prouvent qu'aucun reste ne peut revenir, et aucun comportement ne change."
status: needs-triage
---

## Problem Statement

**Écrit pendant l'implémentation de D.19a/b** (`plan-pilot`, étape 2b), sur l'arbre commis **`b85e94d`** (« docs: PRD de D.19a/b (TU en plis typés) en file, trois sous-étapes »). L'arbre a été extrait par `git archive b85e94d` dans le scratchpad. Seuls `grep`, `sed`, `awk`, un calcul de fermeture d'includes en Python (lecture seule), `sh tools/data_residue.sh` et une simulation des éditions sur une copie de `src/` et de 10 TU ont tourné. **Rien n'a été compilé ni lancé.** Tout ce que ce PRD suppose de l'issue de D.19a/b est une hypothèse `H-D19ab-n` (plus bas), à vérifier à la graduation de D.19b2, avant la mise en file. **Numéros de ligne : ceux de `b85e94d`.** D.19a/b ne touche ni `src/`, ni le `Makefile`, ni `tools/`, ni les 10 TU de ce PRD (H-D19ab-3). Ces numéros sont donc ceux du parent de chaque commit, et l'implémenteur remesure par contenu.

Mesuré à `b85e94d` (D.19a/b non commencée) : `valeur` 435 / 75 ; 479 occ. (`src/` 81 / 37 / 97, `tests/` 354 / 38 / 382), `type-tag` 241 / 57, résidu D 674 / 97. **Attendu au départ de D.19c** (H-D19ab-1) : `valeur` **117 / 47 ; 158 occ.** (`src/` 81 / 37 / 97, `tests/` 36 / 10 / 61), `type-tag` **239 / 57** (`src/` 71 / 23, `tests/` 168 / 34), résidu D **356 / 97**, `DATA_RESIDUE_GUARD` = `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit`.

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c` disparaisse (« **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données »). Après D.19a/b, le module est mort :

- `pico8_value_fixed/_string/_bool/_ref`, `pico8_value_equal` et `pico8_value_tostr` n'ont plus d'appelant hors de `test_pico8_value.c`, qui est un test de la couche (§6, premier point) ;
- **33 fichiers de `src/`** (30 en-têtes et `pico8_cart.c`, `pico8_scroll.c`, `pico8_bootstrap.c`) et **9 TU** incluent `pico8_value.h`. Ils n'en tirent au plus que `pico8_fixed.h`, que l'en-tête inclut ;
- trois lignes de commentaire (`pico8_geom.h:37-38`, `pico8_type_tag.h:10`) renvoient au module ;
- le `Makefile` le compile dans `ENGINE_SRCS`, `DATA_ENGINE_SRCS`, la liste explicite de `test_pico8_layout_nil_item_draws_nothing` et celle de `main_raylib`, et garde sa cible `test_pico8_value` ;
- `pico8_value_tostr` porte la **dernière allocation** du module (`pico8_strdup_owned`), le dernier `malloc` de la Sortie ;
- TROU-3, TROU-4 et le glossaire (`:655`, `:657`) nomment encore le module.

Tant que ces lignes existent, `valeur` ne peut pas entrer dans `DATA_RESIDUE_GUARD`. Un `pico8_value_t` pourrait alors revenir sans que `make test` le voie. **Piège** (surprise 5 du PRD de D.19a/b) : tant que les en-têtes de `src/` incluent `pico8_value.h`, un TU qui nomme encore `pico8_value_t` compile sans son propre include. Le compilateur ne voit donc pas un reste, et seule la grep le voit.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `b85e94d` (§5, l. 293) :

> - D.19c — `pico8_value.c/.h` et `test_pico8_value.c` supprimés. Compte : `valeur` = 0, ajouté à `DATA_RESIDUE_GUARD`. `sanitize-net` : oui.

Legs reçus, relus ici :

- **Étude D.14** (§2, « État d'arrivée ») : « `pico8_value.c` / `pico8_value.h` : supprimés, avec `test_pico8_value.c` (test de la couche, §6). Rien ne reste sous ce nom. » ; `pico8_tostr_fixed` survit dans `pico8_str.c`, avec ses cas « nombre » déjà déplacés à D.15c (G5), et « ses cas chaîne, booléen et `REF` sont supprimés avec leur genre » ; « `pico8_value_equal` n'a plus d'appelant : TROU-3 et TROU-13 […] sont réécrits pour nommer les sites qui restent ». TROU-13 a été réécrit à D.18c. **« l'allocation de `pico8_value_tostr` part avec le module »**. Table (§3) : « D.19c | […] les includes restants ; commentaires qui nomment le module ; `Makefile`, `tools/float_guard.txt` si `make test` le réécrit, `glossaire-noms.md` | ≈ 75 chemins, surtout des includes ». §3 : « `sanitize-net` : […] D.19c (dernière allocation du module) ».
- **D.18c** (note, PRD) : « `src/` n'a plus que le module (45 lignes), 33 includes et 3 commentaires (`geom.h:37-38`, `type_tag.h:10`) ; `pico8_value_tostr` et `pico8_value_equal` n'ont plus que `test_pico8_value` ; `score_panel.h:12` n'a plus que son include ; TROU-3, TROU-4 et le glossaire (`pico8_value_ref(NULL)`) nomment encore le module. » Le PRD de D.18c énumère aussi les lignes du `Makefile` (règle `test_pico8_value`, `DATA_ENGINE_SRCS`, `ENGINE_SRCS`, la ligne de `layout_nil_item_draws_nothing`, `main_raylib`).
- **D.19a/b** (PRD) : « Les 9 TU à include seul attendent D.19c : `ga_export_panel_by`, `jb_ol`, `je`, `layout_nil_item_draws_nothing`, `nf`, `planet_gh_row_lq`, `planet_kj_dispatch`, `probe_mm_rnd_order`, `qb` » (trois sont des §6) ; « D.19c retire les includes d'en-tête en vérifiant que chaque TU garde `pico8_fixed_t` par un autre chemin » ; « `test_pico8_layout_nil_item_draws_nothing` (§6) garde `src/pico8_value.c` dans sa ligne du `Makefile` » ; énumérations `FIXED_KIND … REF_KIND` locales aux TU, définies par des littéraux.
- **Point relevé pour D.19c** : l'include de `pico8_value.h` par `pico8_audio.h` est aussi **le seul chemin par lequel ce fichier reçoit `pico8_fixed_t`**. Confirmé ici par la fermeture : c'est le seul des 42 fichiers dans ce cas.

### Hypothèses `H-D19ab-n` (D.19a/b non gradée)

Source : le PRD de D.19a/b (`docs/prd/triage/plan3-phaseD-etape19ab-tu-plis-types-mots-de-genre-figes.md` à `b85e94d`), sa « Prévision pour D.19c » et son « Effet sur `make data-residue` ». À vérifier contre les notes « Découvert à D.19a/b1/b2 » et `git diff --stat b85e94d <graduation> -- src tests Makefile tools`.

| Hypothèse | Énoncé | Effet si elle tombe |
|---|---|---|
| H-D19ab-1 | Après D.19b2 : `valeur` **117 / 47 ; 158 occ.** (`src/` 81 / 37 / 97, `tests/` 36 / 10 / 61) ; `type-tag` **239 / 57** (`src/` 71 / 23, `tests/` 168 / 34) ; résidu D **356 / 97** ; `DATA_RESIDUE_GUARD` inchangé | départs, arrivées absolues, lignes 1–3 des tableaux « Runs », critères `FAILED:` de départ ; les deltas de c1 (− 33) ne dépendent que de H-D19ab-3 |
| H-D19ab-2 | Dans `tests/`, seuls `test_pico8_value.c` (27 lignes / 52 occ., inchangé) et les 9 TU à include seul nomment le module. Chacun de ces 9 TU en a **une ligne, l'include**, aux numéros de `b85e94d` : `ga_export_panel_by:24`, `jb_ol:21`, `je:24`, `layout_nil_item_draws_nothing:34`, `nf:25`, `planet_gh_row_lq:35`, `planet_kj_dispatch:16`, `probe_mm_rnd_order:23`, `qb:31`. Les 28 TU pliés ont perdu leur include | un TU plié qui garde son include (ou une ligne comptée) entre dans l'empreinte de c2 ; un reste qui n'est pas un include est un travail de D.19a/b : arrêt, rapporté |
| H-D19ab-3 | D.19a/b ne touche **aucune ligne de `src/`, du `Makefile`, de `tools/`**, ni ces 10 TU ; aucun fichier créé ni supprimé | inventaire de `src/`, table du `Makefile` et numéros à remesurer |
| H-D19ab-4 | Les énumérations locales `FIXED_KIND = 0`, `STRING_KIND = 1`, `BOOL_KIND = 2`, `REF_KIND = 3` (et `FUNCTION_KIND = 4`) sont **définies par des littéraux**, jamais par `PICO8_VALUE_*`, et ne correspondent à aucun des dix motifs du compteur. Vérifié à `b85e94d` sur les motifs de `tools/data_residue.sh` : un identifiant en capitales `*_KIND` ne touche ni `valeur` (`\bPICO8_VALUE_…`), ni `type-tag`, ni `par-nom`/`spec`/`dispatch`. Leurs bits ne dépendent donc pas de l'en-tête supprimé | si un énumérateur est défini à partir d'un `PICO8_VALUE_*`, c2 ne compile pas : c'est un reste de D.19a/b (alias), arrêt, rapporté |
| H-D19ab-5 | Chaque TU reçoit `pico8_fixed_t` et `<stdint.h>` par un autre en-tête que `pico8_value.h`. Vérifié à `b85e94d` par fermeture sur **tous** les `.c`/`.h` de `src/` et `tests/` (table « Fermeture »). D.19a/b n'ajoute ni ne retire d'autre include | fermeture à refaire sur l'arbre gradué (recette en Annexe B) |
| H-D19ab-6 | D.19a/b en trois commits, ≈ 25 min chacun (plafond 50) | budgets de c1/c2 à recalibrer sur le temps réel |
| H-D19ab-7 | La ligne D.19c du plan reste mot pour mot celle recopiée ci-dessus. La note de D.19b2 porte la prévision « pour D.19c » de son PRD (117, 9 includes, `test_pico8_value`, `src/` 81) et « pour D.20a » `type-tag` `tests/` 168 | Annexe A et note à reprendre |
| H-D19ab-8 | Aucun TU ne plie ni n'appelle `pico8_value_tostr`/`_equal` ; aucun commentaire neuf de TU ne nomme `pico8_value_*` (règle du PRD de D.19a/b) | une ligne comptée hors des 10 TU : arrêt, rapporté |

### Sections à revérifier à la graduation de D.19a/b

1. « État de départ », critères `FAILED:` de départ, lignes 1–3 des deux tableaux « Runs » (H-D19ab-1).
2. « Inventaire », partie `tests/` : les 9 numéros d'include et `test_pico8_value` 27 / 52 (H-D19ab-2, H-D19ab-8) ; `git grep -lE "$P" -- tests` doit rendre exactement 10 fichiers.
3. « Fermeture » : relancer la recette de l'Annexe B sur l'arbre gradué (H-D19ab-5). Attendu : seul `pico8_audio.h` (et `pico8_audio.c`) perd `pico8_fixed.h`.
4. H-D19ab-4 : `git grep -nE '\b(FIXED|STRING|BOOL|REF|FUNCTION)_KIND\b *=' -- tests` ne montre que des littéraux ; `git grep -n 'PICO8_VALUE_' -- tests` ne rend que `test_pico8_value.c`.
5. « Effet sur `make data-residue` » : arrivées absolues (H-D19ab-1).
6. « `Makefile` » : numéros (H-D19ab-3).
7. Budget (H-D19ab-6), Annexe A (H-D19ab-7).

### Lecture de la ligne pour D.19c

1. **« `pico8_value.c/.h` et `test_pico8_value.c` supprimés »** : les trois fichiers partent entiers. Rien ne reste sous ce nom ni sous un autre (étude D.14 : « Rien ne reste sous ce nom »). Aucune fonction n'est déplacée, parce que toutes sont sans appelant (table « Ce que le module définit »).
2. **Les includes** : la ligne ne les nomme pas, mais `valeur` = 0 les exige, et l'étude les range ici (« les includes restants »). Règle appliquée depuis D.15b (« un fichier qui ne nomme plus rien du module perd son include »), dont les en-têtes étaient jusqu'ici l'exception déclarée (« en-têtes vidés gardés pour D.19c ») : **chaque include est retiré**. Là où l'en-tête supprimé était le seul chemin vers ce que le fichier emploie, il est **remplacé par l'include direct de cet en-tête**. C'est la règle de D.13 (« le fichier gagne l'include direct de ce qu'il emploie »), appliquée ici d'avance, puisque la fermeture le montre (un seul cas : `pico8_audio.h`).
3. **« `sanitize-net` : oui »** : la dernière allocation du module (`pico8_value_tostr`) part (étude, §3). `make sanitize-net` tourne au commit de suppression.
4. **Le découpage** : 53 chemins, au-delà de la borne de ~40 d'un commit (règle de découpage de `plan-pilot`). **Deux sous-étapes, même PRD, chacune vérifiée et poussée** : **D.19c1** retire les 33 includes de `src/` (le module reste, compilé) ; **D.19c2** supprime le module, son test et les 9 includes de TU, et pose la garde. Voir « Empreinte et découpage ».
5. **§6** : `test_pico8_value` est supprimé comme test de la couche (§6, premier point : « supprimés dans le commit qui supprime le code testé »). Les trois tests de régression parmi les 9 TU (`ga_export_panel_by`, `planet_gh_row_lq`, `layout_nil_item_draws_nothing`) ne perdent que leur ligne d'include. Leur intention et leur façon de lire l'état ne changent pas. C'est le précédent de D.12d (`planet_gh_row_lq`, « include seul », sans preuve rouge, avec un diff contrôlé ligne à ligne), et ce n'est **pas** un arrêt §7. La ligne du `Makefile` de `layout_nil_item_draws_nothing` perd ` src/pico8_value.c` ; le test lui-même ne change pas.
6. **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; jamais l'appli raylib. Sont interdits : la jonction de lignes comptées, l'alias, le renommage pour échapper aux greps, le cast nouveau, la cale (en-tête vide ou renommé, type redéfini ailleurs, pont), l'aide laissée sans appel. Les mutations se font dans l'arbre avec restauration par `cp` simple, jamais `git checkout`/`stash`. Les binaires sont lancés exactement comme le `Makefile` les lance.

### État de départ

| Mesure | À `b85e94d` | Départ de D.19c1 (H-D19ab-1) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 435 / 75 ; 479 | **117 / 47 ; 158** (`src/` 81 / 37 / 97, `tests/` 36 / 10 / 61) |
| `type-tag` | 241 / 57 | **239 / 57** (`src/` 71 / 23, `tests/` 168 / 34) |
| résidu D | 674 / 97 | **356 / 97** |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |
| `tools/float_guard.txt` | `garde fichiers 58 en-echec 0` | inchangé |
| `tools/line_coverage_reference.txt` | 5968 lignes, aucun fichier absent (D.13), dont 58 de `src/pico8_value.c` | inchangé |

Recette des occurrences (motif `valeur` de `tools/data_residue.sh:29`, celle de D.12 à D.19a/b) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
T='\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h'
for d in src tests; do F=$(find $d \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in "valeur:$P" "type-tag:$T"; do n=${c%%:*}; p=${c#*:}
    printf '%s\t%s\t%s lignes\t%s fichiers\t%s occ.\n' $d $n "$(cat $F | grep -cE "$p")" \
      "$(grep -lE "$p" $F | wc -l)" "$(cat $F | grep -oE "$p" | wc -l)"; done; done
```

### Inventaire (`b85e94d`, 117 lignes après D.19a/b)

#### Ce que le module définit, et qui l'appelle

| Symbole | Appelants hors `pico8_value.c` et `test_pico8_value.c` | Sort |
|---|---|---|
| `pico8_value_kind_t`, `pico8_value_t`, `struct pico8_value` | aucun (H-D19ab-2, -4) | supprimés |
| `pico8_value_fixed`, `_string`, `_bool`, `_ref` | aucun | supprimés |
| `pico8_value_equal` (porte `/* TROU-3 */`, `value.h:57`) | aucun depuis D.17 | supprimée ; TROU-3 garde ses 6 autres sites (`collect.c:70`, `collect.h:29`, `tech_gate.c:25`, `planet.h:135`, `wormhole.h:77`, `wormhole_display.h:10`), texte réécrit |
| `pico8_value_tostr` | aucun depuis D.18c | supprimée, avec la dernière allocation du module |
| `static pico8_strdup_owned` | `pico8_value_tostr` seul | supprimée |

`pico8_tostr_fixed` n'est pas dans le module : elle est dans `pico8_str.c` depuis D.15c. Ni `pico8_value.c` ni `pico8_value.h` ne sont inclus par `pico8_str.*`.

#### `src/` — 81 lignes / 37 fichiers / 97 occ.

| Groupe | Lignes (occ.) | Sous-étape | Classe |
|---|---|---|---|
| `pico8_value.h` (fichier, 71 lignes) | 22 (30) | c2 | (a) fichier supprimé |
| `pico8_value.c` (fichier, 73 lignes) | 23 (30) | c2 | (a) fichier supprimé |
| 32 includes (30 en-têtes sauf `audio.h`, 3 `.c`) | 32 (32) | c1 | (a) include retiré |
| `pico8_audio.h:4` | 1 (1) | c1 | (a) include **remplacé** par `#include "pico8_fixed.h"` (voir « Fermeture ») |
| `pico8_geom.h:37`, `:38` | 2 (2) | c2 | (a) commentaire réécrit, son sujet disparaît |
| `pico8_type_tag.h:10` | 1 (2) | c2 | (a) commentaire réécrit, son sujet disparaît |

Les 33 includes de `src/`, avec leurs numéros à `b85e94d` : `audio.h:4`, `bg.h:9`, `bootstrap.c:12`, `cart.c:42`, `cart.h:6`, `collect.h:7`, `economy_formulas.h:7`, `entity.h:8`, `entity_geom.h:6`, `help.h:11`, `ke.h:5`, `layout.h:9`, `mouse_reader.h:9`, `msgbox.h:10`, `particle.h:9`, `planet.h:13`, `planet_display.h:8`, `planet_family.h:11`, `planet_family_trade.h:8`, `planet_trade.h:9`, `probe.h:11`, `research.h:10`, `research_display.h:9`, `research_icon.h:10`, `score_panel.h:12`, `scroll.c:11`, `scroll.h:10`, `slipway.h:10`, `tech_gate.h:6`, `trade.h:7`, `ui.h:12`, `ui_button.h:13`, `wormhole.h:12` (préfixe `src/pico8_`). La 34e ligne, `pico8_value.c:1`, est comptée dans le module.

#### `tests/` — 36 lignes / 10 fichiers / 61 occ. (H-D19ab-2)

| Fichier | Lignes (occ.) | Sous-étape | Classe |
|---|---|---|---|
| `test_pico8_value.c` (fichier, 98 lignes, 11 cas) | 27 (52) | c2 | (a) fichier supprimé (§6, test de la couche) |
| 9 TU, une ligne d'include chacun (numéros dans H-D19ab-2) | 9 (9) | c2 | (a) include retiré |

#### Fermeture : ce que chaque includer tenait de `pico8_value.h` (calculée à `b85e94d`, lecture seule)

`pico8_value.h` n'inclut que `pico8_fixed.h`, qui inclut `<stdint.h>`. Le calcul (Annexe B) retire l'en-tête et recalcule, pour **chaque** `.c`/`.h` de `src/` et `tests/` (pas seulement les includers), la fermeture de ses includes entre guillemets. Il vérifie deux choses : `pico8_fixed.h` y reste si le fichier nomme un symbole de `pico8_fixed.h` (`pico8_fixed_t`, `pico8_fixed_*`, `PICO8_FIXED_*`), et `<stdint.h>` y reste s'il nomme un type `u?intN_t`. Pour un en-tête, la fermeture est la sienne propre : chaque en-tête doit rester autonome, quel que soit l'ordre d'inclusion. Un contrôle vérifie enfin qu'aucun includer n'emploie un nom de `pico8_fixed.h` avant la dernière ligne de ses includes (résultat : aucun).

**Résultat : un seul fichier perd ce qu'il emploie, `pico8_audio.h`** (`void pico8_rn(pico8_fixed_t no);`, `:16`). Son unique include est `pico8_value.h`, et `pico8_audio.c` n'inclut que lui. Il reçoit donc `#include "pico8_fixed.h"` **à la place** de sa ligne `:4`. Les 41 autres ne gagnent rien. Simulé sur une copie après les éditions : la fermeture de chaque `.c`/`.h` de `src/` et des 10 TU contient `pico8_fixed.h` et `<stdint.h>` partout où ils servent, et ne contient plus `pico8_value.h` (0 fichier en défaut).

| Includer (préfixe `src/pico8_`) | Emploie | Reçu après retrait par | Action |
|---|---|---|---|
| `audio.h:4` | `pico8_fixed_t` | **rien** | **remplacé** par `#include "pico8_fixed.h"` |
| `cart.h:6`, `entity.h:8`, `mouse_reader.h:9`, `planet_display.h:8` | rien de `pico8_fixed.h` | (sans objet ; `struct.h:4`, `ke.h:4`, `ke.h:5`, `struct.h:4` le fournissent de toute façon) | retiré |
| `help.h:11`, `layout.h:9`, `planet.h:13`, `planet_family.h:11`, `planet_family_trade.h:8`, `research.h:10`, `score_panel.h:12`, `scroll.h:10`, `slipway.h:10`, `tech_gate.h:6`, `trade.h:7`, `wormhole.h:12` | `pico8_fixed_t` | `#include "pico8_fixed.h"` **direct**, `:4` | retiré |
| `economy_formulas.h:7`, `entity_geom.h:6`, `ke.h:5`, `msgbox.h:10`, `particle.h:9`, `planet_trade.h:9`, `research_display.h:9` | `pico8_fixed_t` | `pico8_struct.h:4` | retiré |
| `bg.h:9`, `collect.h:7` | `pico8_fixed_t` | `pico8_game_data.h:4` | retiré |
| `probe.h:11` | `pico8_fixed_t` | `pico8_economy.h:4` | retiré |
| `research_icon.h:10`, `ui_button.h:13` | `pico8_fixed_t` | `pico8_geom.h:4` | retiré |
| `ui.h:12` | `pico8_fixed_t` | `pico8_bg.h:4` | retiré |
| `bootstrap.c:12`, `cart.c:42`, `scroll.c:11` | `pico8_fixed_*` | leur propre en-tête, `:1` | retiré |
| TU (préfixe `tests/test_pico8_`) : `ga_export_panel_by:24`, `je:24`, `nf:25`, `planet_gh_row_lq:35`, `qb:31` | `pico8_fixed_*` ou `uint32_t` (`je` : rien) | `pico8_bootstrap.h` | retiré |
| `jb_ol:21`, `probe_mm_rnd_order:23` | `pico8_fixed_*`, `uint32_t` | `pico8_entity.h` | retiré |
| `layout_nil_item_draws_nothing:34` | `pico8_fixed_*` | `pico8_struct.h` (`:28`) | retiré |
| `planet_kj_dispatch:16` | `pico8_fixed_*` | `pico8_planet.h` (`:1`) | retiré |

Les 28 TU pliés par D.19a/b ont perdu leur include dans le commit de leur dernier pli (H-D19ab-2). Leur fermeture sans `pico8_value.h`, calculée à `b85e94d`, contient elle aussi `pico8_fixed.h` et `<stdint.h>` (par `pico8_bootstrap.h`, `pico8_bg.h`, `pico8_entity.h` ; H-D19ab-5). Les deux autres faux positifs du calcul (`test_constants_check.c`, `test_float_grep.c`) nomment `pico8_fixed_*` dans des chaînes de fixture. Ils n'ont jamais reçu `pico8_fixed.h`, ni avant ni après.

#### Commentaires

| Ligne | Aujourd'hui | Après (sens, pas le mot à mot) |
|---|---|---|
| `pico8_geom.h:36-38` | « The box is heap-allocated and belongs to the caller, / which usually parks it in a field as a PICO8_VALUE_REF and never frees / it (see pico8_value.h's borrowed-pointer convention). */ » | le contrat de propriété reste (sorte « contrat C » de `CLAUDE.md`) : la boîte est allouée, appartient à l'appelant, qui la garde dans un membre (`js.kq`, seul appelant, `ui_button.c:211`) et ne la libère pas. Le genre de valeur et le renvoi à l'en-tête partent. Deux lignes comptées sortent pour une ou deux lignes non comptées |
| `pico8_type_tag.h:10` | « A pico8_value_t REF (pico8_value.h) is a bare \`void *\` and carries no » | **une ligne pour une ligne**, sans nom compté, même sens : un REF est un `void *` opaque, sans type propre (par exemple « A REF, an opaque \`void *\`, carries no »). Les lignes `:11-12` ne changent pas. Le nombre de lignes de l'en-tête ne bouge pas, parce que ses fonctions `inline` sont couvertes par `gcov` |

Aucun autre commentaire de `src/` ou `tests/` ne nomme le module (grep sur `value\.h`, `tagged value`, `PICO8_VALUE`, hors motif : rien hors D.19a/b).

#### `Makefile` (numéros de `b85e94d`, H-D19ab-3)

| Ligne(s) | Aujourd'hui | Après | Sous-étape |
|---|---|---|---|
| `:217` | `    src/pico8_value.c \` (`ENGINE_SRCS`) | supprimée | c2 |
| `:276` | `    src/pico8_value.c \` (`DATA_ENGINE_SRCS`) | supprimée | c2 |
| `:407-409` | cible `$(TMP_DIR)/test_pico8_value: $(call objs,tests/test_pico8_value.c src/pico8_value.c src/pico8_str.c src/pico8_fixed.c) \| $(TMP_DIR)`, sa recette et la ligne vide qui suit | supprimées ; une seule ligne vide reste entre `test_pico8_geom` et `test_pico8_ptrs` | c2 |
| `:561` | liste explicite de `test_pico8_layout_nil_item_draws_nothing` | ` src/pico8_value.c` retiré de la ligne, rien d'autre | c2 |
| `:574` | `    test_pico8_value \` (`UNIT_TESTS`) | supprimée | c2 |
| `:848` | `DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | `…,liste,ref-audit,valeur` | c2 |
| `:944`, `:945` | prérequis et commande de `main_raylib` | ` src/pico8_value.c` retiré de chacune, rien d'autre | c2 |

Six lignes sont retirées et quatre modifiées, et **rien d'autre ne change** : ni la légende (`:59`, « make value-members -> liste les membres pico8_value_t », décrit toujours l'outil), ni une autre cible. `CORE_SRCS` perd `pico8_value.c` par `ENGINE_SRCS`. c1 ne touche pas le `Makefile`.

#### `tools/`, `docs/`

| Chemin | Sort | Sous-étape |
|---|---|---|
| `tools/float_guard.txt` | réécrit par `make test` (`float-guard` sur `CORE_SRCS`) : `garde fichiers 58` → **57**, jamais à la main (précédents D.2, D.8, D.12d) | c2 |
| `tools/line_coverage_reference.txt` | les **58** lignes `src/pico8_value.c:*` retirées (5968 → **5910**), rien d'autre ; l'invariant posé par D.13 (« plus aucun fichier absent ») tient | c2 |
| `tools/data_residue.sh`, `tests/test_data_residue.c` | **inchangés** : le motif `valeur` est la garde, et les fixtures coupent leurs chaînes (`"pico8_" "value_t"`) | — |
| `tools/entity_value_members.sh` | **inchangé** : `make value-members` (dans `data-guard`) cherche un membre `pico8_value_t` et en trouve 0 ; le type n'existe plus, la garde reste vraie | — |
| `docs/trous-de-portage.md` | TROU-3 (premier paragraphe, ligne « Fichiers »), TROU-4 (une phrase) | c2 |
| `docs/glossaire-noms.md` | `:655` réécrite, `:657` supprimée | c2 |
| `docs/plan-3.md` | notes « Découvert à D.19c1 », « à D.19c2 » ; bloc « Reste de D » (c2) | c1, c2 |
| `CLAUDE.md`, `docs/plan.md`, `docs/plan-2.md`, `docs/audit-ref-cast-reverse.md`, `docs/prd/done/*` | **inchangés** (configuration, ou textes historiques) | — |

## Solution

**Deux sous-étapes, deux commits, chacun vert seul, vérifié et poussé :**

- **D.19c1 — includes de `src/`** : les 33 lignes `#include "pico8_value.h"` des 30 en-têtes et 3 `.c` sont retirées. Celle de `pico8_audio.h` devient `#include "pico8_fixed.h"`. Rien d'autre ne change. Le module reste compilé, et seuls `pico8_value.c`, `test_pico8_value.c` et les 9 TU l'incluent encore. Dès ce commit, un fichier de `src/` ou un TU plié qui nommerait encore `pico8_value_t` **ne compile plus**, ce qui prouve H-D19ab-2 par le compilateur.
- **D.19c2 — suppression et garde** : `src/pico8_value.c`, `src/pico8_value.h` et `tests/test_pico8_value.c` sont supprimés. Les 9 TU perdent leur include. Les trois commentaires sont réécrits. Le `Makefile` suit sa table. `tools/float_guard.txt` est réécrit par `make test`, et `tools/line_coverage_reference.txt` perd ses 58 lignes. TROU-3, TROU-4 et le glossaire sont mis à jour. `valeur` entre dans `DATA_RESIDUE_GUARD`.

**Sortie de l'étape**, constatée par commande sur chaque commit :

- `make test` vert (dont `make data-guard`, avec `valeur` gardée à c2), `make e2e` : **26 conformes** ; `make sanitize-net` propre (c2) ;
- `make data-residue` : après c1, `valeur` **84 / 14** ; après c2, **plus de ligne `valeur`**, `type-tag` **239 / 57** et résidu D **239 / 57** (la seule catégorie restante) ;
- la garde mord (M1) ; l'include de `audio.h` est nécessaire (M2) ;
- aucune ligne exécutée perdue par les e2e, hors `pico8_value.c` (`L`, `gcov`) ; aucun objet ne référence `pico8_value_*` (`nm`) ; la sortie du préprocesseur de chaque `.c` ne perd que les déclarations de `pico8_value.h` ;
- ces commandes rendent exactement ce qui est écrit (`<parent>` = le parent du commit) :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # --- après D.19c1 ---
  # 1. src/ : includes de pico8_value.h, seulement dans le module
  git grep -n 'pico8_value\.h' -- src                # src/pico8_value.c:1 et les lignes de commentaire geom.h:38, type_tag.h:10
  git grep -nE '^#include "pico8_value\.h"' -- src   # src/pico8_value.c:1 seul
  # 2. src/ : 33 lignes retirées, une ajoutée
  git diff <parent> -- src | grep -c '^-[^-]'        # 33
  git diff <parent> -- src | grep '^+[^+]'           # +#include "pico8_fixed.h" (src/pico8_audio.h) seul
  # 3. périmètre
  git diff --stat <parent>                           # 33 fichiers de src/, docs/plan-3.md
  # --- après D.19c2 ---
  # 4. plus aucune trace du module dans les fichiers suivis (hors motif du compteur, fixtures, légende)
  git grep -nE "$P|test_pico8_value" -- src tests Makefile tools ':!tools/data_residue.sh' ':!tools/entity_value_members.sh'   # vide
  git grep -n 'pico8_value' -- Makefile              # la seule légende :59 (value-members)
  grep -n '^DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur$' Makefile
  ls src/pico8_value.c src/pico8_value.h tests/test_pico8_value.c   # trois « No such file »
  # 5. tests/ : 9 lignes retirées hors du fichier supprimé, aucune ajoutée
  git diff <parent> -- tests ':!tests/test_pico8_value.c' | grep -c '^-[^-]'   # 9
  git diff <parent> -- tests | grep -c '^+[^+]'      # 0
  # 6. référence de couverture : 58 retraits, tous de pico8_value.c ; plus aucun fichier absent
  git diff <parent> -- tools/line_coverage_reference.txt | grep '^-[^-]' | grep -vc '^-src/pico8_value\.c:'   # 0
  git diff <parent> -- tools/line_coverage_reference.txt | grep -c '^+[^+]'  # 0
  wc -l < tools/line_coverage_reference.txt          # 5910
  cut -d: -f1 tools/line_coverage_reference.txt | sort -u | while read f; do [ -f "$f" ] || echo "$f"; done   # vide
  grep '^garde ' tools/float_guard.txt               # garde fichiers 57 en-echec 0
  # 7. comptes
  make -s data-residue                                # type-tag 239 57 ; résidu D : 239 ligne(s), 57 fichier(s)
  ```

## User Stories

1. En tant que mainteneur du port, je veux que `pico8_value.c/.h` disparaissent, afin que la Sortie de D n'ait plus que `pico8_type_tag.h` à supprimer.
2. En tant que mainteneur du port, je veux que `test_pico8_value` parte avec le code qu'il teste, afin qu'aucun test ne garde en vie une couche supprimée (§6).
3. En tant que mainteneur du port, je veux que chaque fichier qui incluait l'en-tête le perde, et ne gagne l'include direct de `pico8_fixed.h` que s'il en a besoin, afin que les includes disent ce que chaque fichier emploie.
4. En tant que mainteneur du port, je veux la fermeture d'includes de chaque fichier avant d'éditer, afin de savoir d'avance que seul `pico8_audio.h` a besoin d'un remplaçant.
5. En tant que mainteneur du port, je veux que tout compile sous `-std=c99 -Wall -Wextra -Werror` à chaque commit, afin que la suppression ne casse aucune dépendance passée par l'en-tête.
6. En tant que mainteneur du port, je veux que le retrait des includes de `src/` vienne dans un commit à part, avant la suppression, afin que le compilateur voie tout reste de `pico8_value_t` dans `src/` ou dans un TU plié.
7. En tant que mainteneur du port, je veux que les 9 TU à include seul ne perdent que cette ligne, afin que les trois tests de régression parmi eux gardent leur intention mot pour mot.
8. En tant que mainteneur du port, je veux que le `Makefile` ne nomme plus `pico8_value.c` ni `test_pico8_value`, dans aucune liste de sources ni cible, et ne change nulle part ailleurs, afin qu'aucun lien n'attende un objet absent.
9. En tant que mainteneur du port, je veux que `valeur` entre dans `DATA_RESIDUE_GUARD`, afin qu'un `pico8_value_*` ou un include du module qui revient fasse échouer `make test`.
10. En tant que vérificateur, je veux la preuve que la garde mord (une ligne plantée, la garde rouge ; restaurée, verte), afin de ne pas croire une garde vide.
11. En tant que vérificateur, je veux la preuve que l'include direct de `pico8_audio.h` est nécessaire (retiré, `pico8_audio.c` ne compile plus), afin qu'il ne soit pas un ajout gratuit.
12. En tant que vérificateur, je veux qu'aucun objet compilé ne référence un symbole `pico8_value_*` avant la suppression (hors le module et son test), afin que la règle de lien soit une mesure.
13. En tant que vérificateur, je veux que la sortie du préprocesseur de chaque `.c` ne perde que les déclarations de `pico8_value.h`, à `__LINE__` près, afin que « aucun comportement ne change » soit prouvé avant tout lancement.
14. En tant que vérificateur, je veux `gcov` sur les 26 e2e avant et après chaque commit, avec `L` ne contenant au plus que des lignes de `pico8_value.c`, afin de constater qu'aucune ligne exécutée par le jeu n'est perdue.
15. En tant que vérificateur, je veux un balayage ASan+UBSan de tous les TU et `make sanitize-net` sur l'état final, afin que la disparition de la dernière allocation du module soit contrôlée.
16. En tant que mainteneur du port, je veux que `make e2e` reste à 26 conformes au bit près, sans `.sha256` ni fichier de `tests/e2e/` touché, afin que le filet juge et ne soit pas ajusté.
17. En tant que relecteur, je veux la classification de chaque ligne retirée ou modifiée, avec comptes avant/après par fichier en lignes et en occurrences, afin de vérifier qu'aucun compte ne baisse par tassement.
18. En tant que relecteur, je veux savoir où vit la couverture de chacun des 11 cas de `test_pico8_value`, ou pourquoi elle n'appartient qu'à la couche, afin que la suppression ne perde rien de ce que le jeu fait.
19. En tant que lecteur de `docs/trous-de-portage.md`, je veux que TROU-3 nomme les comparaisons de chaînes qui restent et que TROU-4 ne nomme plus une fonction supprimée, afin que chaque entrée reste vraie.
20. En tant que lecteur du glossaire, je veux que les conventions ne parlent plus d'un type supprimé, afin qu'elles décrivent le code tel qu'il est.
21. En tant que lecteur du code, je veux que les commentaires de `pico8_geom.h` et `pico8_type_tag.h` gardent leur contrat sans renvoyer au module, afin qu'aucun commentaire ne pointe vers un fichier absent.
22. En tant que `plan-pilot`, je veux des notes « Découvert à D.19c1/c2 » courtes et un bloc « Reste de D » sans le point `valeur`, afin que D.20a parte des bons comptes.
23. En tant qu'humain, je veux qu'aucun test ne rejoue un `events.json`, que rien ne soit lent et que l'appli raylib ne soit jamais lancée, afin que la vérification reste dans les règles du dépôt.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules, tests et découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés. La suppression elle-même est dans la ligne du plan (étude D.14, Q12 = oui, décision humaine).

### Modules

- **Supprimés** (c2) : `src/pico8_value.c`, `src/pico8_value.h`, `tests/test_pico8_value.c`.
- **Modifiés, `src/`** : c1 touche les 33 includers, une ligne chacun (32 retirées, 1 remplacée). c2 touche `pico8_geom.h` (commentaire, `:37-38`) et `pico8_type_tag.h` (commentaire, `:10`).
- **Modifiés, `tests/`** (c2) : les 9 TU à include seul, une ligne retirée chacun.
- **Build** (c2) : `Makefile` (table).
- **Outils** (c2) : `tools/float_guard.txt` (par `make test`), `tools/line_coverage_reference.txt` (retraits seuls).
- **Docs** : `docs/plan-3.md` (c1, c2) ; `docs/trous-de-portage.md`, `docs/glossaire-noms.md` (c2).
- **Inchangés** : tout autre fichier de `src/` et `tests/`, dont `pico8_str.c/.h` (`pico8_tostr_fixed`), `pico8_fixed.h`, `test_data_residue.c` ; `tools/data_residue.sh` (motif `valeur` = la garde) ; `tools/entity_value_members.sh` ; `tests/e2e/` ; tout `*.sha256` ; `tools/class_coverage_net_expected.txt`.

### Includes (G)

- **Retirés, sans remplaçant**, partout où la fermeture montre que le fichier reçoit `pico8_fixed.h` et `<stdint.h>` par un autre chemin, ou n'en emploie rien (41 lignes : 32 de `src/` à c1, 9 de TU à c2).
- **`pico8_audio.h:4` : remplacé par `#include "pico8_fixed.h"`** (même position). L'en-tête déclare `pico8_rn(pico8_fixed_t)` et n'a pas d'autre include, et `pico8_audio.c` n'inclut que lui. L'include va dans l'en-tête et non dans `pico8_audio.c`, parce qu'un en-tête doit rester autonome (ses 10 includers : `planet.c`, `probe.c`, `cart.c`, `ui_button.c`, `menu.c`, `wormhole.c`, `msgbox.c`, `slipway.c`, `audio.c`, `test_pico8_ui.c`). Ce n'est ni un alias ni une cale : `pico8_fixed.h` est l'en-tête qui définit ce que `audio.h` emploie, et il existait avant ce plan. Déclaré au message, prouvé par M2.
- **Aucun autre include ajouté.** Si la compilation dément la fermeture, le fichier gagne l'include **direct** de ce qu'il emploie (`pico8_fixed.h` ou `<stdint.h>`, jamais un en-tête de confort), déclaré avec l'erreur du compilateur. Rejeté : réordonner ou regrouper des includes (hors sujet) ; ajouter `pico8_fixed.h` par principe aux 29 autres en-têtes qui le reçoivent déjà (bruit, hors besoin).
- **Rejeté : un seul commit** (53 chemins, au-delà de ~40). **Rejeté : c1 = tous les includes (src et TU, 42 chemins + plan)**. Ce découpage dépasse aussi la borne, et il toucherait les trois §6 un commit avant la suppression de l'en-tête qu'ils cessent d'inclure. Les 9 includes de TU partent donc au commit qui supprime l'en-tête (précédent D.12d : `planet_gh_row_lq` « include seul » dans le commit de suppression).

### `test_pico8_value` : où vit la couverture de chaque cas (G)

§6, premier point : « Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, …) sont supprimés dans le commit qui supprime le code testé. » Le plan nomme `test_pico8_value` en toutes lettres. Ses 11 cas, lus un par un :

| Cas (`test_pico8_value.c`) | Ce qu'il teste | Où vit ce que le jeu en garde |
|---|---|---|
| `fixed_constructs_with_fixed_kind` (`:8`) | constructeur, genre 0 | couche seule. Le mot de genre 0 est figé en `FIXED_KIND = 0` dans les TU à mot de genre (D.19b1) et plié par leurs `*_HASH` |
| `string_constructs_with_string_kind` | constructeur, genre 1, pointeur emprunté | couche seule ; `STRING_KIND = 1` figé de même |
| `bool_constructs_with_bool_kind` | constructeur, genre 2, **normalisation `b ? 1 : 0`** | couche seule. La normalisation est reproduite par les `fold_bool` des TU (`b ? 1u : 0u`, D.19a/b), et `BOOL_KIND = 2` y est figé |
| `ref_constructs_with_ref_kind` | constructeur, genre 3 | couche seule ; `REF_KIND = 3` figé |
| `equal_fixed_by_numeric_value` | `pico8_value_equal` sur deux nombres | couche seule. Le jeu compare des `pico8_fixed_t` par `==`, et `pico8_qe_equal` (D.17) est testé par les TU de `trade`, `hp` et de l'arbre `jo` |
| `equal_bool_by_value` | idem, booléens | couche seule. Les booléens sont des `int` depuis D.16a |
| `equal_ref_by_pointer_identity` | idem, pointeurs | couche seule. L'identité de pointeur du jeu (`pico8_ptrs_del`) est tenue par `test_pico8_ptrs` |
| `equal_string_by_pointer_identity_not_content` | TROU-3, comparaison par pointeur | couche seule : plus aucune comparaison de chaînes ne passe par un pointeur. Les sites de TROU-3 comparent par contenu (`pico8_gu_content_ptrs` pour `oj`, `pico8_jo_gg`), et les TU de leurs modules les couvrent (`hp`, arbre `jo`) |
| `equal_different_kinds_never_equal` | genres distincts | couche seule : aucun genre ne reste |
| `tostr_string_is_content_passthrough` | `tostr` d'une chaîne, copie possédée | couche seule. La seule chaîne copiée pour `tostr` dans le jeu est `happiness` (`pico8_pj_ph_s_str`, D.18c), tenue par `SCORES_SEQUENCE_HASH` de `test_pico8_mh` (M3 de D.18c) |
| `tostr_bool` | `"true"` / `"false"` | couche seule. Aucun appelant de `src/` ne met un booléen en chaîne (grep : les seuls `"true"`/`"false"` hors du module sont ceux du format d'`events.json`, `pico8_input.c`, sans rapport) |

Les cas « nombre » (`tostr` d'un fixe, 13 témoins de `tools/probe_out.txt`) ont déjà rejoint `test_pico8_str` à D.15c (G5 de l'étude). **Aucun cas n'est déplacé ni réécrit. Aucun test n'est ajouté.** Rejeté : garder un test des genres pour mémoire (il testerait un type qui n'existe plus).

### Commentaires (G)

Les quatre sortes de `CLAUDE.md` s'appliquent. Il n'y a aucun renvoi à D.19 ni au plan dans `src/`, `tests/` ou le `Makefile`. **Réécrits parce que leur sujet disparaît, et seulement eux** : `pico8_geom.h:37-38` (le contrat de propriété reste, table « Commentaires ») et `pico8_type_tag.h:10` (une ligne pour une ligne). Rejeté : réécrire l'en-tête de `pico8_type_tag.h` au-delà de `:10` (D.20b le supprime) ; nommer « la valeur étiquetée » ou « l'ancien en-tête » à la place du module (ce serait un renommage d'évitement : le commentaire ne doit plus parler du module du tout).

### `Makefile` (G)

Le diff exact est la table « `Makefile` ». Les fichiers `.d` des arbres d'objets nomment `src/pico8_value.h` comme prérequis. `-MP` (`DEPFLAGS`) leur donne une cible factice, et un en-tête supprimé ne casse donc pas `make` (H1). Les objets `tmp/obj*/src/pico8_value.o` et le binaire `tmp/test_pico8_value` restent sur le disque (non suivis). Ils ne sont pas nettoyés à la main.

**Règle de lien** : aucun objet d'une liste de lien ne référence un symbole défini hors de cette liste. Elle tient par construction, puisque plus rien n'appelle `pico8_value_*`, et elle est **mesurée** au départ de c2 : après c1 et `make test`, `nm -u tmp/obj/src/*.o tmp/obj/tests/*.o | grep pico8_value_` ne rend que des références de `test_pico8_value.o`. Elle est vérifiée ensuite en liant : `make test` (79 → 78 TU, dont `layout_nil_item_draws_nothing` sur sa liste explicite), `make e2e` (`pico8_replay` sur `ENGINE_SRCS`), `make sanitize-net` (arbre `obj-sanitize`), `make line-coverage` (arbre `obj-gcov`). `main_raylib` (`:944-945`) est contrôlé par le diff exact. `make raylib` (compilation et lien, **jamais d'exécution**) n'est lancé que si `third_party/raylib/src` est déjà vendu. Sinon, c'est déclaré (précédent D.12d).

### `tools/line_coverage_reference.txt` (G)

C'est une référence « écrite une fois […] on ne la recalcule pas, on la compare ». D.13 a posé l'invariant « plus aucun fichier absent », et sa note le consigne. **Tranché** : retirer les 58 lignes `src/pico8_value.c:*`, rien d'autre, sans recalcul (recette : `grep -v '^src/pico8_value\.c:'`, ordre conservé). Les numéros des fichiers qui existent restent décalés, comme D.13 l'a déclaré (`make e2e-gap` hors suite, non lancé). Rejeté : laisser les 58 lignes, ce qui romprait l'invariant et rouvrirait un point du « Reste de D ».

### `docs/trous-de-portage.md` (G)

- **TROU-3** reste : l'écart (chaînes non internées) a encore six sites marqués. Le premier paragraphe est réécrit sans `pico8_value_equal` ni `PICO8_VALUE_STRING`, ni le parseur `ob()`, supprimé à D.8. Il dit que Lua interne les littéraux, que ce port copie ou alloue ses chaînes (données générées : un tableau distinct par jeton, D.5c), et que deux chaînes égales ne sont donc pas le même pointeur. Le paragraphe « Contourné au cas par cas » perd sa dernière phrase (« `pico8_value_equal` ne fera pas ce qu'on croit »). Il garde « Toute nouvelle comparaison de chaînes doit choisir explicitement » et ses exemples (`pico8_gu_content_ptrs`, `jo:gg`, `db`). La ligne « Fichiers » devient `` `src/pico8_collect.c` ``. Le paragraphe « Hors de TROU-3 : les `qe` » ne change pas.
- **TROU-4** reste (écart dormant, sans site depuis D.8). Une seule phrase change : « `pico8_pi_spr_constructor` rend `pico8_value_ref(NULL)`, donc une valeur fausse » devient un imparfait sans nom du module (« rendait un pointeur nul, donc une valeur fausse »). Rejeté : supprimer l'entrée, parce que l'écart n'est pas refermé (« Une entrée supprimée est une entrée refermée »).

### `docs/glossaire-noms.md` (G)

- `:655` « Emprunt de pointeur » : le sujet `pico8_value_t` disparaît. Son successeur est l'item de rangée, qui a remplacé la valeur à D.18. La règle est réécrite dessus : un `pico8_jb_item_t` ne possède **jamais** ce que pointent `s`, `row` et `ref`, trois membres `const` (vérifié, `pico8_map_tables.h:38-47`). La phrase « `pico8_ptrs_free` ne libère que son tableau » reste.
- `:657` « `pico8_value_ref(NULL)` » : **supprimée**. L'emplissage n'existe plus, et TROU-11, qu'elle expliquait, est sans site depuis D.10a (son texte ne nomme pas le module).
- La phrase d'introduction de la section (« Transversales, et trois trous en dépendent ») n'est pas touchée. Le compte « trois » n'est pas vérifiable ligne à ligne (signalé à `plan-pilot`).

### `docs/plan-3.md` (G)

- **Note « Découvert à D.19c1 »** en fin de ligne D.19c, puis **« Découvert à D.19c2 »**. Elles ne gardent que ce qui change une étape suivante (§7) : le découpage en deux commits, les comptes réels et l'écart aux attendus, l'include direct de `audio.h` (et tout autre gagné), `valeur` = 0 dans `DATA_RESIDUE_GUARD`, `type-tag` 239 / 57 et résidu D 239 / 57 (la seule catégorie restante, pour D.20a/D.20b), la dernière allocation du module partie (« plus aucun `malloc` pour des données » ne dépend plus que de `type-tag`, qui n'alloue pas), et `float_guard` 57. Le reste (chemins, classification, `gcov`, `nm`, préprocesseur, mutations) va dans les messages de commit.
- **Bloc « Reste de D »** (c2) : la puce « `valeur` (980), `type-tag` (243) : étapes après Q12. […] » devient « `type-tag` (239) : D.20a, D.20b. » Les « Restes nommés » (tous typés depuis D.15–D.18 : `pico8_jy_qe` rend `pico8_qe_t`, `pico8_jc_ce` un en-tête d'entité, `pico8_oj` prend `const char *`) et les prototypes à `pico8_value_t` (0) partent. Dans les puces `hp_pose` et `pk`/`pk_is_bool`, l'alternative « étape `valeur` » est retirée : il n'y en a plus, et il reste « phase 2 ». Rien d'autre ne change : ni D.20a/b, ni la Sortie de D, ni les notes livrées. La puce `hp_pose` a en fait été traitée à D.16b2 (« `hp_pose` par position »). Sa clôture reste à la revue de fin de D (signalé, non fait ici).

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer le module est un échec.

- **Interdit** : garder un `pico8_value.h` vide ou renommé ; déplacer une déclaration, un type, un énumérateur ou une fonction du module dans un autre fichier, sous ce nom ou un autre (`typedef`, `#define`, union « genre + contenu », aide qui refabrique une valeur) ; réécrire un commentaire pour qu'il décrive encore le module sous un autre nom ; joindre des lignes comptées. Le commentaire de `geom.h`, dont deux lignes comptées deviennent une ou deux lignes sans le sujet supprimé, n'est pas une jonction : son contenu compté part, et c'est déclaré. Sont aussi interdits : le cast nouveau ; le changement du motif `valeur` ou de toute catégorie dans `tools/data_residue.sh` ; toute modification de `tests/` hors des 9 lignes d'include et du fichier supprimé, de `tests/e2e/`, d'un `*.sha256`, de `tools/` hors `float_guard.txt` (par `make test`) et des 58 retraits de `line_coverage_reference.txt`, du `Makefile` hors sa table, d'un fichier de `src/` hors de son inventaire ; un include ajouté sans erreur de compilation citée (hors `audio.h`).
- **Classification exigée**, au message de chaque commit, pour chaque ligne comptée retirée ou modifiée. Recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`.
  - **c1** : **(a) include retiré** : 32 ; **(a) include remplacé par l'en-tête employé** : 1 (`audio.h:4` → `pico8_fixed.h`) ; **(b)** 0 ; **(c)** 0.
  - **c2** : **(a) fichier supprimé** : 72 (`value.h` 22, `value.c` 23, `test_pico8_value.c` 27) ; **(a) include retiré** : 9 ; **(a) commentaire réécrit, sujet disparu** : 3 (`geom.h:37`, `:38` ; `type_tag.h:10`) ; **(b)** 0 ; **(c)** 0.
- **Mesure exigée**, avant et après chaque commit, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et **par fichier** du diff, en lignes (`--detail`) **et** en occurrences (recette).

| Fichier(s) | `valeur` avant → après c1 | avant → après c2 |
|---|---|---|
| 33 includers de `src/` | 1 / 1 chacun → 0 / 0 | — |
| `pico8_value.h` / `.c` | 22 / 30 ; 23 / 30 (inchangés) | → fichiers supprimés |
| `pico8_geom.h` ; `pico8_type_tag.h` | 2 / 2 ; 1 / 2 (inchangés) | → 0 / 0 ; 0 / 0 |
| `test_pico8_value.c` | 27 / 52 (inchangé) | → fichier supprimé |
| 9 TU à include seul | 1 / 1 chacun (inchangés) | → 0 / 0 |
| **Total** | **117 / 47 / 158 → 84 / 14 / 125** (`src/` 81 / 37 / 97 → 48 / 4 / 64) | **84 / 14 / 125 → 0** |

- **Lignes qui gagnent une catégorie** : attendu **aucune**. `#include "pico8_fixed.h"` et les commentaires réécrits ne touchent aucun des dix motifs. Un include direct gagné (H1 démentie) n'en touche pas non plus.
- **Lignes comptées modifiées, non retirées** : aucune.
- **Δlignes > Δoccurrences** : aucun (chaque ligne retirée porte au moins une occurrence ; `value.h`, `value.c`, `type_tag.h:10` et `test_pico8_value.c` en portent plus).
- **Numéros de ligne** : ceux du **parent** de chaque commit, remesurés, jamais ceux de ce PRD sans remesure.

### Effet sur `make data-residue` (simulé sur une copie de `src/` et des 10 TU, sans build)

| Catégorie | Départ (H-D19ab-1) | Après c1 | Après c2 | Pourquoi |
|---|---|---|---|---|
| `valeur` | 117 / 47 ; 158 occ. | **84 / 14 ; 125** | **0, exact** | 33 includes, puis le module, son test, 9 includes et 3 commentaires |
| `type-tag` | 239 / 57 | 239 / 57 | **239 / 57** | aucune ligne `type-tag` touchée (`type_tag.h:10` n'en est pas une) |
| résidu D | 356 / 97 | **323 / 65** | **239 / 57** | c1 : 32 includers sortent du résidu (seul `bootstrap.c` y reste, par `type-tag`) ; c2 : `value.h`, `value.c`, `geom.h`, `test_pico8_value.c` et 4 des 9 TU sortent (`ga_export_panel_by`, `layout_nil_item_draws_nothing`, `planet_kj_dispatch`, `probe_mm_rnd_order`) |
| `DATA_RESIDUE_GUARD` | `…,liste,ref-audit` | inchangé | **`…,liste,ref-audit,valeur`** | `valeur` = 0 |

Simulation (copie de `src/` et des 9 TU plus `test_pico8_ptrs.c`, éditions de c1 et c2 appliquées par `sed`) : `data_residue.sh src tests` rend `type-tag 83 28` (71 de `src/` + 12 des 9 TU) et plus aucune ligne `valeur`. `--check spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur src tests` rend `0 ligne(s)`, code 0. Avec `/* pico8_value.h */` ajoutée en fin de `tests/test_pico8_ptrs.c`, il rend `valeur: tests/test_pico8_ptrs.c:225:/* pico8_value.h */` et `résidu gardé (spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur) : 1 ligne(s)`, code 1. La garde mordra.

### Empreinte et découpage (G)

| Sous-étape | `src/` | `tests/` | autres | Chemins | Coût estimé |
|---|---|---|---|---|---|
| **D.19c1** | 33 includers (30 `.h`, 3 `.c`) | — | `docs/plan-3.md` | **34** | ≈ 15 min, plafond 30 |
| **D.19c2** | `pico8_value.c` ✗, `pico8_value.h` ✗, `pico8_geom.h`, `pico8_type_tag.h` (4) | `test_pico8_value.c` ✗, 9 TU (10) | `Makefile`, `tools/float_guard.txt`, `tools/line_coverage_reference.txt`, `docs/plan-3.md`, `docs/trous-de-portage.md`, `docs/glossaire-noms.md` (6) | **20** | ≈ 30 min, plafond 50 |

- **Pourquoi deux** : 53 chemins en un seul commit (~40 au plus). La coupe suit la dépendance naturelle : c1 est vert seul, puisque le module et son test compilent encore et que personne d'autre n'en a besoin (H-D19ab-2) ; c2 part d'un arbre où seuls 11 fichiers nomment le module.
- **Replis écrits** (G) : si c1 dépasse son plafond, rien n'est commis, et `plan-pilot` relance c1 en deux moitiés d'includers (ordre alphabétique, 17 puis 16), chacune verte seule. Si c2 dépasse le sien, rien n'est commis, et c2 est relancé en **c2a** (les 9 includes de TU, 10 chemins avec le plan, vert seul : ces TU n'emploient rien du module) puis **c2b** (le reste). Aucun pont, aucune cale dans aucun repli.

### Ce qui ne change pas

- `tests/` hors des 10 fichiers ; `tests/e2e/` ; tout `*.sha256` ; aucun e2e ni TU ajouté ; un seul TU supprimé (`test_pico8_value`).
- Le comportement : le module n'a plus d'appelant, et le préprocesseur montre que chaque `.c` ne perd que ses déclarations. Toute constante `*_HASH`, `identity[]`, `FROZEN_*_TAG`, `VIEW_*`, `TABLE_INDEX_KEY`, `*_KIND` est inchangée.
- `pico8_type_tag.h` (sauf `:10`), `pico8_struct_header_t`, l'étiquette `STRUCT`, `type-tag` : D.20a et D.20b.
- `tools/float_guard.txt` à c1 (réécrit identique : aucun fichier du cœur ne part).

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : les 32 includers retirés et `audio.h` remplacé compilent sous `-std=c99 -Wall -Wextra -Werror` (fermeture plus haut, simulée). Si c'est démenti, include direct déclaré (voir « Includes »).
- **H2** : `-MP` rend inoffensifs les `.d` qui nomment `src/pico8_value.h`. Si c'est démenti (« No rule to make target »), supprimer les `.d` concernés **dans `tmp/`** (non suivi), jamais toucher au `Makefile` hors de sa table ; le déclarer.
- **H3** : `make test` ne réécrit aucun fichier suivi autrement qu'à l'identique à c1, et `tools/float_guard.txt` en `57` à c2.
- **H4** : les e2e n'exécutent aucune ligne de `pico8_value.c` au départ de c2 (D.18c : « branches `FIXED` et `REF` de `pico8_value_tostr`, sans appelant de `src/` »). `L` de c2 est donc vide ; au pire, il ne contient que des lignes de ce fichier.
- **H5** : aucun fichier suivi hors de l'empreinte ne nomme `src/pico8_value.[ch]` ou `test_pico8_value` (vérifié à `b85e94d` : seuls `docs/plan.md`, `docs/plan-2.md`, `docs/plan-3.md`, `docs/prd/`, historiques, et `tools/line_coverage_reference.txt`, traité). Les greps de contrôle passent par `git grep`, jamais par un `grep -r` qui ramasserait `.claude/worktrees/` (ignoré).

## Implementation Chunks

**D.19c1** (un commit, vert seul, vérifié et poussé) :

1. **Départ** : HEAD = graduation de D.19a/b (ou un descendant qui ne touche que `docs/`), arbre propre. Recette : `valeur` 117 / 47 / 158, `type-tag` 239 / 57, résidu 356 / 97 (H-D19ab-1). `git grep -lE "$P" -- src tests` rend exactement les 37 + 10 fichiers de l'inventaire ; sinon remesurer et expliquer, et un reste qui n'est pas un include ou le module est un arrêt, rapporté. **Préprocesseur** (départ) : `cc -std=c99 -E -P` de chaque `src/*.c` et `tests/*.c` dans le scratchpad. **`gcov`** (départ) : `make line-coverage EVENTS="$(ls tests/e2e/*.json)"`, copie de `tmp/line_coverage.txt`.
2. **Édition** : 32 lignes retirées, `audio.h:4` remplacée.
3. **Filets** : `make test`, `make e2e`.
4. **Preuves** : préprocesseur et `gcov` à l'arrivée, comparaison ; M2 ; `nm -u` (règle de lien, mesurée pour c2) ; greps 1 à 3 ; mesure, classification ; note « Découvert à D.19c1 » ; **commit** `refactor(D.19c1): …`.

**D.19c2** (un commit, vert seul, vérifié et poussé) :

1. **Départ** : HEAD = D.19c1, arbre propre ; `valeur` 84 / 14 / 125 ; `nm -u` : seul `test_pico8_value.o` référence `pico8_value_*`. Préprocesseur et `gcov` (départ) comme en c1.
2. **Édition** : suppression des trois fichiers ; 9 includes de TU ; deux commentaires ; `Makefile` (table) ; filtre de `line_coverage_reference.txt` ; TROU-3, TROU-4, glossaire.
3. **Filets** : `make test` (réécrit `float_guard.txt` en `57`), `make e2e`, `make sanitize-net`, balayage ASan+UBSan des TU.
4. **Preuves** : préprocesseur et `gcov` à l'arrivée ; M1 ; greps 4 à 7 ; mesure, classification ; note « Découvert à D.19c2 » et bloc « Reste de D » ; **commit** `refactor(D.19c2): …`.

## Testing Decisions

Un bon test juge ce que le code rend ou fait, pas la présence d'une ligne. D.19c ne retire que du code sans appelant, son test de couche et des includes inutiles. La preuve est donc que rien de ce qui s'exécute ne change : préprocesseur, `nm`, `gcov`, 26 e2e au bit près, `sanitize-net` et ASan+UBSan sur les TU. Deux mutations la complètent : la garde nouvelle mord (M1), et le seul include ajouté est nécessaire (M2). Aucun TU n'est écrit.

### TU touchés

- **Supprimé** : `test_pico8_value` (§6, test de la couche ; table « `test_pico8_value` »).
- **Une ligne d'include retirée**, rien d'autre : `ga_export_panel_by` (§6), `jb_ol`, `je`, `layout_nil_item_draws_nothing` (§6), `nf`, `planet_gh_row_lq` (§6), `planet_kj_dispatch`, `probe_mm_rnd_order`, `qb`. Pour les trois §6, le diff du fichier est exactement la ligne d'include. Aucune preuve rouge n'est exigée, puisque le test n'est pas réécrit (précédent D.12d). Le préprocesseur montre que leur texte compilé ne perd que les déclarations de `pico8_value.h`, qu'ils n'employaient pas.
- Prior art : suppression d'un module avec son test de couche et sa garde (D.8 `ob`/`stash`, D.12d `liste`, D.13 `ref-audit`) ; include direct gagné déclaré (D.12b2 : `economy.h` gagne `pico8_ptrs.h`) ; garde qui mord par ligne plantée (M6 de D.12d, M1 de D.13).

### Preuve (§7)

- **Préprocesseur (c1 et c2)** : pour chaque `src/*.c` et `tests/*.c` présent au départ et à l'arrivée, la sortie de `cc -std=c99 -E -P` est normalisée (`sed -E 's/[0-9]+/N/g' | grep -v '^[[:space:]]*$' | sort`). Deux conditions sont attendues : (i) `comm -13 départ arrivée` **vide** (aucune ligne gagnée) ; (ii) chaque ligne de `comm -23 départ arrivée` figure dans la sortie normalisée de `cc -std=c99 -E -P src/pico8_value.h` (seules les déclarations du module partent). Le tri rend la comparaison insensible au déplacement du texte de `pico8_fixed.h` (fourni plus tôt ou plus tard selon l'ordre des includes), et la normalisation insensible au `__LINE__` des `assert` de `cart.c`, `scroll.c` et `bootstrap.c` (−1). Il faut rapporter le nombre de fichiers dont la sortie change et le nombre total de lignes perdues. Toute autre différence est un arrêt, rapporté.
- **`nm` (fin de c1, départ de c2)** : `nm -A -u tmp/obj/src/*.o tmp/obj/tests/*.o | grep pico8_value_` ne rend que `test_pico8_value.o`, et `nm tmp/obj/src/pico8_value.o` montre les six fonctions définies. C'est la preuve de lien, avant la suppression, qu'aucun autre objet n'en dépend.
- **`gcov` (c1 et c2)** : `make line-coverage EVENTS="$(ls tests/e2e/*.json)"` (pilote instrumenté, arbre `obj-gcov`), au départ puis à l'arrivée de chaque commit. `L` = lignes de `src/` exécutées au départ et plus à l'arrivée. c1 : appariement `n → n − 1` après la ligne retirée dans `cart.c` (`:42`), `scroll.c` (`:11`) et `bootstrap.c` (`:12`), les autres fichiers tels quels (aucun en-tête à code `inline` n'est édité). Attendu : **`L` vide, totaux égaux**. c2 : attendu **`L` vide** (H4), **`L` ⊆ `src/pico8_value.c:*` au pire**, déclaré ; total d'arrivée = total de départ moins les lignes de `pico8_value.c` exécutées au départ (attendu 0). Une ligne de `L` hors de `pico8_value.c` est un comportement perdu : arrêt, rapporté. Rejeté : l'union `gcov` de tous les TU (quelques minutes deux fois), parce que le seul TU qui exécutait le module est supprimé avec lui et que `nm` prouve qu'aucun autre ne le lie.
- **Mutation M1, la garde mord (c2)** : `tests/test_pico8_ptrs.c` est sauvegardé par `cp`, puis la ligne `/* pico8_value.h */` y est ajoutée en fin de fichier. `make data-guard` doit échouer avec `valeur: tests/test_pico8_ptrs.c:<n>:/* pico8_value.h */` et `résidu gardé (spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur) : 1 ligne(s)`. Le fichier est ensuite restauré par `cp` puis `touch`, et `make data-guard` redevient vert.
- **Mutation M2, l'include de `audio.h` est nécessaire (c1)** : `src/pico8_audio.h` est sauvegardé par `cp`, puis sa ligne `#include "pico8_fixed.h"` est retirée. `make tmp/obj/src/pico8_audio.o` doit échouer (`unknown type name 'pico8_fixed_t'`, erreur citée au message). Le fichier est ensuite restauré par `cp` puis `touch`, et l'objet recompile.
- Aucune autre mutation : il n'y a aucune logique nouvelle, et §7 demande des mutations sur ce que `gcov` ne prouve pas, c'est-à-dire ici la garde et l'include (déclaré).

### Filets

- `make test` (dont `data-guard`, `float-guard`, `float-grep`, `constants-check`) et `make e2e` (26 conformes), à chaque commit.
- **`make sanitize-net`** : à c2 (ligne de l'étape ; dernière allocation du module). Attendu : 26 propres. Il est aussi rapporté s'il est lancé à c1.
- **Balayage ASan+UBSan des TU** (c2) : dans une copie jetable du scratchpad, les 78 TU de `UNIT_TESTS` sont compilés avec les flags du `Makefile` plus `-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer`, et lancés comme le `Makefile` avec `ASAN_OPTIONS=detect_leaks=0` (TROU-7). Le `Makefile` n'est pas modifié. Précédents : D.7b, D.18c, D.19b2.
- `make class-coverage-net` : **non** (aucun constructeur touché). `make raylib` : seulement si `third_party/raylib/src` est présent, compilation et lien, **jamais d'exécution**.
- Aucun TU ne rejoue un fichier d'événements ; `make e2e-gap` n'est pas lancé.

### Vérification, budget et tableau « Runs »

**Budget** : c1 ≈ 15 min, **plafond 30** ; c2 ≈ 30 min, **plafond 50** (du départ au commit, filets et preuves compris). Au-delà : repli écrit, et rien d'inachevé n'est commis. La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| recette, `make -s data-residue`, greps | < 1 min |
| `cc -E -P` de ~140 `.c`, départ et arrivée, comparaison | < 1 min |
| `make line-coverage EVENTS=…` (26 e2e), départ et arrivée | < 1 min chacun |
| éditions (c1 : 33 lignes ; c2 : 9 lignes, deux commentaires, `Makefile`, filtre, docs) | ~5 min chacune |
| `make test` (recompilation complète à c1 : tous les en-têtes changent) | 1 à 3 min |
| `make e2e` | ~1 s |
| M1, M2 | secondes |
| `nm -u` | secondes |
| balayage ASan+UBSan (78 TU, c2) | 2 à 5 min |
| `make sanitize-net` (c2) | ~4 min sur machine libre (221 s mesurées en 1b.0), jusqu'à ~8 min sous charge |
| mesure, classification, message, note | ~5 min par commit |

**Jamais l'appli raylib.** Rien de lent : ni union `gcov` des TU, ni `minimize`, ni `class-coverage-net`.

**L'implémenteur tient un tableau « Runs »** par sous-étape, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + état) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | graduation de D.19a/b, avant c1 | 0 | `valeur 117 47` ; `type-tag 239 57` ; résidu 356 / 97 | … |
| 2 | recette `valeur` | idem | 0 | `valeur 117 lignes 47 fichiers 158 occ.` | … |
| 3 | `git grep -lE "$P" -- src tests \| wc -l` | idem | 0 | `47` | … |
| … | … | … | … | … | … |

(c2, lignes 1–3 : `valeur 84 14`, `84 lignes 14 fichiers 125 occ.`, `14`.) Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de c1 ne liste que les 33 includers et `docs/plan-3.md` (34). Celui de c2 ne liste que ses 20 chemins. S'y ajoute seulement un include direct prouvé nécessaire. Ni `tests/e2e/`, ni `*.sha256`.
2. **Greps** 1 à 7 de la « Sortie », exactement.
3. **Includes** : 32 + 9 retirés sans remplaçant, `audio.h` remplacé ; M2 rapporté ; aucun autre include ajouté sans erreur citée.
4. **`Makefile`** : diff = la table, rien d'autre ; `DATA_RESIDUE_GUARD` finit par `,ref-audit,valeur`.
5. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste (c1 : (a) 32 + 1 remplacé ; c2 : (a) 72 + 9 + 3 ; (b) = (c) = 0) ; lignes gagnées « aucune » ; aucun alias ni aucune cale.
6. **Garde** : `make data-guard` vert ; M1 rapporté.
7. **Preuve** : préprocesseur (conditions (i) et (ii)) ; `nm` ; `L` (vide à c1, ⊆ `pico8_value.c` à c2) ; 26 e2e conformes ; `sanitize-net` et balayage ASan+UBSan propres.
8. **Commentaires** : `geom.h` garde son contrat de propriété ; `type_tag.h:10` une ligne pour une ligne ; aucun commentaire ne nomme le module sous un autre nom ; aucun renvoi à D.19 ni au plan.
9. **Docs** : TROU-3 (six sites, `Fichiers` sans `pico8_value.c`), TROU-4 (une phrase), glossaire (`:655` réécrite, `:657` retirée) ; notes « Découvert à D.19c1/c2 » courtes ; bloc « Reste de D » : puce `valeur` fermée, `type-tag (239)`, « étape `valeur` » retirée des puces `hp_pose` et `pk` ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ de c1 n'est pas la graduation de D.19a/b (ou un descendant qui ne touche que `docs/`), propre ; si `valeur` n'y vaut pas 117 / 47 / 158 occ., `type-tag` 239 / 57, résidu D 356 / 97, sans explication ; de même pour c2 (84 / 14 / 125, 239 / 57, 323 / 65) ; si une ligne `valeur` existe hors des 47 fichiers de l'inventaire (sauf écart hérité de D.19a/b, rapporté comme arrêt).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) change au-delà de sa ligne d'include ; si une ligne ne tombe sous aucune décision de ce PRD (arrêt « site sans règle ») ; si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si l'appli raylib est lancée.

**Périmètre et forme**

- `FAILED:` si `pico8_value.c`, `pico8_value.h` ou `test_pico8_value.c` survit, sous ce nom ou un autre, ou si une partie de leur contenu (type, énumérateur, fonction, `pico8_strdup_owned`) est déplacée ailleurs.
- `FAILED:` si un fichier de `src/` gagne une ligne autre que l'include de `audio.h`, un include direct prouvé nécessaire par une erreur citée, ou la réécriture des deux commentaires ; si un includer perd autre chose que sa ligne d'include ; si un TU change au-delà de sa ligne d'include.
- `FAILED:` si le `Makefile` change hors de sa table ; si `tools/` change hors de `float_guard.txt` (réécrit par `make test`, `57`) et des 58 retraits de `line_coverage_reference.txt` ; si `tools/data_residue.sh`, `tools/entity_value_members.sh` ou `tests/test_data_residue.c` change.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit ; si c1 et c2 ne sont pas deux commits distincts, chacun vert seul (hors repli écrit).

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, si `DATA_RESIDUE_GUARD` ne vaut pas `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur` après c2 (ou change à c1), ou si M1 ne fait pas échouer la garde.
- `FAILED:` si `valeur` ne vaut pas 84 / 14 / 125 après c1 et **exactement 0** après c2 ; si `type-tag` ne vaut pas 239 / 57 ou le résidu D 239 / 57 après c2, sans explication.
- `FAILED:` **si un compte baisse par tassement** (cale, alias, renommage, commentaire qui décrit encore le module sous un autre nom, jonction de lignes comptées, déplacement hors `src`/`tests` ou hors `.c`/`.h`) ; (b) ou (c) non nulle.
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification avec numéros du parent ; les lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées (« aucune ») ; Δlignes/Δoccurrences ; les includes retirés, remplacés ou ajoutés (M2 à c1) ; le rapport du préprocesseur ; le rapport `nm` ; le rapport `gcov` (`L`, totaux) ; à c2, M1, le détail des 58 lignes retirées de la référence, `sanitize-net` et le balayage ASan+UBSan, et la table de couverture des 11 cas de `test_pico8_value`. Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère de `garde fichiers 57 en-echec 0` après c2 (ou change à c1).

**Preuve, commit et note**

- `FAILED:` si le préprocesseur montre une ligne gagnée ou une ligne perdue absente de `pico8_value.h` ; si `nm` montre un objet autre que `test_pico8_value.o` qui référence `pico8_value_*` ; si `L` contient une ligne hors de `pico8_value.c` ; si `sanitize-net` ou le balayage ASan+UBSan n'est pas propre.
- `FAILED:` si M1 ou M2 manque, ne rougit pas, ou ne se restaure pas par `cp` (fichier identique au commit).
- `FAILED:` si un plafond est dépassé sans note (et sans repli), ou s'il manque un tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.19c1 » ou « à D.19c2 » ; si le bloc « Reste de D » garde la puce `valeur` ou l'alternative « étape `valeur` » ; si le plan change ailleurs.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`, ou si un renvoi à D.19 ou au plan apparaît dans `src/`, `tests/` ou le `Makefile`.

## Out of Scope

- `type-tag`, `pico8_type_tag.h` (sauf son commentaire `:10`), `pico8_struct_header_t`, l'étiquette `STRUCT`, les `FROZEN_*_TAG` et les `pico8_type_tag_of` des TU : D.20a, D.20b.
- Réécrire les octets de genre déjà littéraux de D.17/D.18a (`hp` `fold_qe`, `lq:206`) ou factoriser les énumérations `*_KIND` des TU : hors du diff.
- `tools/entity_value_members.sh` et `make value-members` (garde vraie, type absent) : leur retrait ou renommage appartient à la revue de fin de D.
- `CLAUDE.md` (« La couche objet Lua (`pico8_value`/`ob`/`class`/`entity`) est en cours de remplacement ») : configuration, pas touchée par un agent ; signalé pour la fin de D.
- La clôture de la puce `hp_pose` du « Reste de D » (traitée à D.16b2) et la phrase « trois trous en dépendent » du glossaire : revue de fin de D.
- Le recalcul des numéros de `tools/line_coverage_reference.txt` ; `make e2e-gap`.
- `docs/plan.md`, `docs/plan-2.md`, `docs/audit-ref-cast-reverse.md`, `docs/prd/done/` (historiques).
- Le nettoyage de `tmp/` et de `.claude/worktrees/` (non suivis).
- `make class-coverage-net`, l'appli raylib, l'audio (hors périmètre), le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Découpage** : deux commits, c1 (33 includes de `src/`, 34 chemins) puis c2 (suppression, 9 includes de TU, garde, docs ; 20 chemins), avec des replis écrits. Rejeté : un commit (53 chemins) ; c1 avec les TU (43 chemins, et les §6 touchés avant la suppression).
  - **Includes** : retirés partout, avec la fermeture comme preuve ; `pico8_audio.h` reçoit `pico8_fixed.h` en direct (M2) ; aucun autre ajout sans erreur citée.
  - **`test_pico8_value`** : supprimé entier (§6) ; ses 11 cas sont des cas de couche, et leur trace dans le jeu est tabulée ; aucun test ajouté.
  - **Commentaires** : `geom.h` garde le contrat de propriété ; `type_tag.h:10` est réécrit une ligne pour une ligne.
  - **`Makefile`** : six lignes retirées et quatre modifiées, rien d'autre.
  - **`tools/`** : `float_guard.txt` par `make test` ; 58 lignes de `line_coverage_reference.txt` retirées (invariant de D.13).
  - **Docs** : TROU-3 réécrit sur ses sites (étude D.14), TROU-4 d'une phrase, glossaire `:655` réécrite sur `pico8_jb_item_t` et `:657` retirée ; plan : deux notes et le « Reste de D ».
  - **Preuve** : préprocesseur trié et normalisé, `nm`, `gcov` e2e, `sanitize-net`, balayage ASan+UBSan, M1 et M2.
- **Surprises de mesure, pour `plan-pilot`** :
  1. **Un seul fichier dépendait vraiment de l'en-tête : `pico8_audio.h`**, qui n'a pas d'autre include et déclare `pico8_rn(pico8_fixed_t)`. Les 41 autres includes sont vides : 12 en-têtes incluent déjà `pico8_fixed.h` en direct (`:4`), les autres le reçoivent par `pico8_struct.h`, `pico8_game_data.h`, `pico8_geom.h`, `pico8_economy.h`, `pico8_bg.h` ou `pico8_ke.h`.
  2. **Quatre en-têtes n'emploient rien de `pico8_fixed.h`** (`cart.h`, `entity.h`, `mouse_reader.h`, `planet_display.h`).
  3. **Le `Makefile` nomme le module sur neuf lignes** (dont deux pour `main_raylib`) : la « règle `test_pico8_value` » du PRD de D.18c en fait trois avec sa recette et sa ligne vide.
  4. **`tools/line_coverage_reference.txt` porte 58 lignes de `pico8_value.c`**. Les laisser romprait l'invariant « aucun fichier absent » que D.13 a posé.
  5. **`test_pico8_value` n'a plus aucun cas « nombre »** (partis à D.15c). Ses 11 cas testent tous des genres ou des comparaisons que plus rien ne produit.
  6. **Le résidu D après D.19c est exactement `type-tag`** (239 / 57) : D.20a/b ferment la phase.
  7. **Le bloc « Reste de D » est en retard sur les étapes** : ses « Restes nommés » (`pico8_jy_qe`, `pico8_jc_ce`, `pico8_oj`…) sont typés depuis D.15–D.18, et `hp_pose` est positionnel depuis D.16b2. Seule la puce `valeur` et l'alternative « étape `valeur` » sont traitées ici ; le reste va à la revue de fin de D.
  8. **`CLAUDE.md` nomme encore `pico8_value`** dans sa description de la couche. Il n'est pas touché (configuration).
- **Pour la suite** : après D.19c, la Sortie de D ne garde que `pico8_type_tag.h` (D.20a, D.20b), et « plus aucun `malloc` pour des données » est atteint pour les trois modules à allocation (`ob`, `stash`, `value`).

## Annexe A — textes du plan recopiés (`docs/plan-3.md` à `b85e94d`, par extraction de lignes)

### Lignes D.19a/b et D.19c (l. 292-293)

> - D.19a/b — TU seulement : empreintes et aides locales en plis typés, constantes inchangées. Compte : ≈ 115. Découpé à la PRD en trois sous-étapes, même PRD, chacune vérifiée et poussée : D.19a (`fr`, `fv`, `g_`, `ga`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` ; plis à octets propres ; → 297), D.19b1 (`hp`, `lq`, `hy`, `js`, `trade`, `ui`, `entity_kind`, `fq`, `mh` ; plis à mot de genre, mots de genre figés en enum local ; → 187), D.19b2 (`jc`, `pj`, `mo`, `iv`, `mz`, `bg`, `ie` ; → 117). Un include part avec le dernier pli du TU ; les 9 TU qui n'ont que l'include attendent D.19c.
> - D.19c — `pico8_value.c/.h` et `test_pico8_value.c` supprimés. Compte : `valeur` = 0, ajouté à `DATA_RESIDUE_GUARD`. `sanitize-net` : oui.

### Fin de la note « Découvert à D.18c » (l. 291, extrait)

> Pour D.19c : `src/` n'a plus que le module (45 lignes), 33 includes et 3 commentaires (`geom.h:37-38`, `type_tag.h:10`) ; `pico8_value_tostr` et `pico8_value_equal` n'ont plus que `test_pico8_value` ; `score_panel.h:12` n'a plus que son include ; TROU-3, TROU-4 et le glossaire (`pico8_value_ref(NULL)`) nomment encore le module.

### Bloc « Reste de D », première puce (extrait)

> - `valeur` (980), `type-tag` (243) : étapes après Q12. Elles emportent les 13 copies privées de `*_truthy(pico8_value_t)` et les includes inutiles de `pico8_value.h` (motif `valeur`). Restes nommés : `pico8_lq_keys_t.items`, `pico8_lq_bg`, `pico8_pj_ed`, `pico8_jc_ce`, `pico8_jy_qe`, `pico8_oj` ; 25 prototypes publics (29 à la fin de D.12, 25 après D.15a) à `pico8_value_t` (96 au début de D ; l'étude disait 62, compte d'une autre méthode).

### §6, premier point

> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.

## Annexe B — reproduire la fermeture (lecture seule, sans build)

Elle est à relancer sur l'arbre gradué (H-D19ab-5). Pour chaque `.c`/`.h` de `src/` et `tests/`, le calcul prend la fermeture des includes entre guillemets (résolus depuis le dossier du fichier, puis `src/`), en retirant `pico8_value.h`. Il signale le fichier si ses noms de `pico8_fixed.h` (`\bpico8_fixed_[a-z0-9_]+\b|\bPICO8_FIXED_[A-Z0-9_]+\b|\bpico8_fixed_t\b`, hors commentaires) n'ont pas `src/pico8_fixed.h` dans cette fermeture, ou si ses types `\bu?int(8|16|32|64)_t\b` n'y ont pas `<stdint.h>`. Attendu : `src/pico8_audio.h` et `src/pico8_audio.c` seuls, plus deux faux positifs antérieurs et inchangés (`tests/test_constants_check.c`, `tests/test_float_grep.c`, noms dans des chaînes de fixture), et `pico8_fixed.h` lui-même. Contrôle d'ordre : aucun includer n'emploie un de ces noms avant sa dernière ligne `#include`.
