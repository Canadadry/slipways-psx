---
title: "Plan 3, étape D.15c : `valeur` — `pico8_tostr_fixed` dans `pico8_str.c`, 20 sites d'appel convertis, cas « nombre » de `test_pico8_value` déplacés tels quels dans `test_pico8_str` ; `pico8_value_tostr` garde ses trois appelants à genres mêlés et délègue ses nombres ; aucune constante ne change"
description: "Vingt sites de `src/` écrivent `pico8_value_tostr(pico8_value_fixed(x))` pour formater un nombre, et le formatage à quatre décimales (TROU-22) vit dans le module de la valeur Lua que la phase D doit supprimer. D.15c le déplace dans `pico8_str.c` sous le nom `pico8_tostr_fixed`, à côté de `pico8_sub` (même contrat : l'appelant libère), fait suivre les appelants et les tests dans le même commit : `valeur` baisse de 27 lignes (54 occ.), 906 → 879."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.15b était implémentée** (`plan-pilot`, étape 2b), sur l'arbre commis `289c9a2`, puis **révisé après sa graduation** : D.15b gradée (`33dc521`, VERIFIED ; « docs: D.15b gradée (ij/jc_ce en pointeurs, d_ en entier, jc_hg en chaîne) » **`667fd2f`**). Les hypothèses `H-D15b-n` sont confrontées plus bas à `667fd2f` ; les comptes et **tous les numéros de ligne de ce PRD sont ceux de `667fd2f`**, remesurés par contenu sur `git archive 667fd2f` (parmi les fichiers de l'étape, seuls `galaxy.c`, `probe.c`, `research_icon.c` ont été touchés par D.15b ; seul `research_icon.c` a bougé au-dessus d'un site : `:136` → `:138`). Seuls `grep`, `sed`, `awk`, `git diff 289c9a2 667fd2f` et `sh tools/data_residue.sh` ont tourné. **Rien n'a été compilé ni lancé.** Si le HEAD de départ n'est pas `667fd2f` (ou un descendant qui ne touche que `docs/`), l'implémenteur remesure par contenu et explique l'écart.

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c/.h` disparaissent (D.19c). Or la seule fonction de ce module qui a un vrai contenu, le formatage d'un nombre à quatre décimales arrondies au pair (réécrit en entiers à D2, 13 témoins sur 14 du bloc C de `tools/probe_out.txt`, TROU-22), y est rangée dans la branche `FIXED` de `pico8_value_tostr`. Vingt sites de `src/` fabriquent une valeur rien que pour la lui passer, et ses tests — dont le balayage exhaustif de 1 245 183 entrées et les 13 témoins — vivent dans `test_pico8_value`, le test de la couche, qui sera supprimé avec elle (§6 du plan). Tant que le formatage reste là, supprimer le module supprimerait aussi le seul test qui garde TROU-22.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `289c9a2` (§5, « Phase D — Données », l. 285) :

> - D.15c — `valeur` : `tostr` déplacé dans `pico8_str.c` avec ses tests. `sanitize-net` : oui. Compte : ≈ 880.

Ligne de l'étude D.14 pour cette étape (§3, table des étapes), recopiée :

> | **D.15c** | `pico8_tostr_fixed` dans `pico8_str.c`, 20 sites ; cas « nombre » de `test_pico8_value` déplacés au TU de `pico8_str` (G5) | ≈ 16 fichiers | ≈ 880 |

Textes de l'étude qui fondent l'étape, recopiés : §1.2, ligne (a) — « `pico8_value_tostr(pico8_value_fixed(x))` — 20 sites (`collect`, `economy_score`, `galaxy`, `menu` ×6, `planet_display` ×4, `probe`, `research_display` ×2, `research_icon`, `score_panel` ×3, `score_panel_display`) | toujours un nombre | `char *pico8_tostr_fixed(pico8_fixed_t)` dans `pico8_str.c`, à côté de `pico8_sub` (même convention : l'appelant libère) ; TROU-22 et TROU-13 suivent la fonction (G5) » ; §2 — « `pico8_tostr_fixed` dans `pico8_str.c/.h` (formatage à 4 décimales, arrondi pair ; TROU-22) ; ses cas « nombre » de `test_pico8_value` passent au TU de `pico8_str` (ils portent les 13 témoins de `tools/probe_out.txt`, ce n'est pas un test de la couche) ; ses cas chaîne, booléen et `REF` sont supprimés avec leur genre. » ; « Le motif `valeur` ne doit rien toucher de neuf : `pico8_tostr_fixed` […] n'y correspond pas » ; drapeaux — « `sanitize-net` : D.15c (l'allocation de `tostr` change de module) » ; **G5** — « `pico8_tostr_fixed` va dans `pico8_str.c` (même convention de propriété que `pico8_sub`) ; ses 13 témoins restent testés. »

Règles communes de l'étude (§3), recopiées :

> - **Un TU qui nomme un symbole changé le suit dans le même commit.** Un TU peut être converti plus tôt que son symbole ; jamais plus tard.
> - **Aucune empreinte figée ne change** (G1). […]
> - Aucun `*.sha256` ni e2e ne change.
> - `class-coverage-net` si l'étape touche un constructeur ; `sanitize-net` si elle change une durée de vie ou un propriétaire.

### Hypothèses `H-D15b-n` (écrites pendant D.15b ; verdicts plus bas)

Source : le PRD de D.15b commis à `289c9a2` (`docs/prd/triage/plan3-phaseD-etape15b-valeur-pointeurs-entier-chaine.md`), ses « Sortie », « Inventaire », « Includes », « Pour D.15c ». Rien d'autre de D.15b n'a été lu.

| Hypothèse | Énoncé | Ce qui en dépend ici | Vérification à la graduation |
|---|---|---|---|
| H-D15b-1 | Après D.15b : `valeur` **911 / 101 ; 1030 occ.** (`src/` 436 / 63 / 508 occ., `tests/` 475 / 38 / 522 occ.) ; `type-tag` 243 / 58 ; résidu D **1152 / 110** ; `DATA_RESIDUE_GUARD` inchangé | « État de départ », « Effet sur `make data-residue` », critère `FAILED:` de départ | `make -s data-residue` ; recette ; note « Découvert à D.15b » |
| H-D15b-2 | Dans `galaxy.c`, D.15b ne change que `:53` (`pico8_struct_header_t *nearest;`, en place) ; l'include `:12` est **gardé** (le PRD de D.15b le liste parmi les includes gardés) et `:79` (`tostr`) est intacte au même numéro ; `galaxy.c` vaut **2 / 3 occ.** | la ligne `galaxy.c:12` retirée ici (voir « Includes »), le compte de `galaxy.c`, le passage 101 → 99 fichiers | `git diff 289c9a2 <HEAD gradé> -- src/pico8_galaxy.c` |
| H-D15b-3 | Dans `research_icon.c`, D.15b ajoute trois lignes `g_ie_has_id` (après `:24`, après `:129`, après `:267`) et retire l'`assert` `:300` ; le site `tostr` passe de **`:136` à `:138`**, texte inchangé ; `:91` (`oj`) intacte ; le fichier vaut **2 / 4 occ.** | numéro de ce site (Inventaire, M9), compte de `research_icon.c` | même `git diff` ; `grep -n pico8_value_tostr src/pico8_research_icon.c` |
| H-D15b-4 | Dans `probe.c`, D.15b n'édite qu'en place (`:157`, `:175`, `:180`, `:181`, `:328`, `:335`, `:359`, `:364`) : `:273` (`tostr`) intacte au même numéro ; le fichier vaut **16 / 18 occ.** | numéro et compte de `probe.c` | même `git diff` |
| H-D15b-5 | D.15b ne touche ni `collect.c`, ni `economy_score.c`, ni `menu.c`, ni `planet_display.c`, ni `research_display.c`, ni `score_panel.c`, ni `score_panel_display.c`, ni `pi.c`, ni `pico8_value.c/.h`, ni `pico8_str.c/.h`, ni `test_pico8_value`, ni `test_pico8_str`, ni le `Makefile`, ni `docs/trous-de-portage.md`, ni la ligne `tostr(v)` de `docs/glossaire-noms.md` | tous les numéros non marqués de l'Inventaire ; « `Makefile` et liens » ; « Documentation » | `git diff --stat 289c9a2 <HEAD gradé> -- <ces chemins>` vide |
| H-D15b-6 | La règle d'include de D.15b est appliquée et notée : un **`.c`** qui ne nomme plus rien du module perd son `#include "pico8_value.h"` ; un **en-tête** le garde jusqu'à D.19c | « Includes » (retrait de `galaxy.c:12`) | note « Découvert à D.15b » ; `git show <commit D.15b> -- src/pico8_slipway_display.c` |
| H-D15b-7 | La note « Découvert à D.15b » ne modifie pas la ligne D.15c du plan, ne déplace aucun site `tostr` vers D.15b, et ses « Pour D.15c » redisent `galaxy.c:79`, `probe.c:273`, `research_icon.c:138` (ou leurs numéros réels) | tout le PRD | lecture du plan gradé |
| H-D15b-8 | D.15b est entière à la graduation (un commit, ou b1 + b2 tous deux commis ; pas de repli laissé à moitié) | tout | note de D.15b |
| H-D15b-9 | D.15b n'ajoute aucun `#include "pico8_str.h"`, aucun symbole `pico8_tostr*`, aucun TU ; `tools/` inchangé ; `make e2e` 26 conformes | « Includes », « `Makefile` et liens », filets | `git diff --stat` ; `grep -rn 'pico8_tostr\|pico8_str.h' src tests` |

### Sections à revérifier quand D.15b sera gradée (liste d'origine, gardée pour trace ; faite, voir « Sections revérifiées »)

- **Toujours** : « État de départ » (comptes réels, H-D15b-1) ; critères `FAILED:` de départ (HEAD et comptes attendus) ; « Effet sur `make data-residue` » (colonnes « départ » et « après », et la table par fichier) ; grep 10 de la Sortie ; la note attendue « Découvert à D.15c » ; Annexe A (recopier la ligne D.15b et sa note).
- **Si H-D15b-2 tombe** (include de `galaxy.c` déjà retiré, ou `galaxy.c` touché ailleurs) : « Includes » (le retrait passe à D.15b ou reste ici), Inventaire de `galaxy.c`, table (a2), comptes (− 27 peut devenir − 26), passage 101 → 99 fichiers, grep 9.
- **Si H-D15b-3 ou H-D15b-4 tombe** : numéros de `research_icon.c` / `probe.c` dans l'Inventaire, M9, cibles des mutations ; comptes de ces deux fichiers.
- **Si H-D15b-5 tombe** : Inventaire du fichier touché (remesure **par contenu**), empreinte, « `Makefile` et liens » si c'est le `Makefile`.
- **Si H-D15b-6 tombe** (D.15b a gardé l'include de `slipway_display.c`, ou a vidé des en-têtes) : « Includes » (retirer ou garder `galaxy.c:12` selon la règle effectivement appliquée), comptes.
- **Si H-D15b-7, -8 ou -9 tombe** : ne pas mettre ce PRD en file ; le reprendre.

**Verdicts à `667fd2f`** (`git archive`, `git diff 289c9a2 667fd2f`, `sh tools/data_residue.sh`, note « Découvert à D.15b » recopiée en Annexe A) :

| Hypothèse | Verdict |
|---|---|
| H-D15b-1 | **fausse en valeur, sans effet sur le plan** : `valeur` **906 / 101 ; 1025 occ.** (`src/` 436 / 63 / 508 occ. comme prévu ; `tests/` **470 / 38 / 517 occ.** au lieu de 475 / 522) : D.15b a aussi retiré l'aide `fold_value` de `test_pico8_qh` (5 lignes, 5 occ.), restée sans appel ; `type-tag` 243 / 58 ; résidu D **1147 / 110** ; `DATA_RESIDUE_GUARD` inchangé. `test_pico8_qh` n'est pas un fichier de D.15c |
| H-D15b-2 | **tenue** : `galaxy.c` ne change qu'à `:53` (un hunk, en place) ; `:12` include gardé, `:79` intacte ; 2 / 3 occ. |
| H-D15b-3 | **tenue** : `research_icon.c` +3 lignes, `assert` retiré ; site `tostr` à **`:138`** (note de D.15b et grep) ; `:91` intacte ; 2 / 4 occ. |
| H-D15b-4 | **tenue** : `probe.c` édité en place (hunks `:154`, `:172`, `:325`, `:356`, longueurs égales) ; `:273` intacte ; 16 / 18 occ. |
| H-D15b-5 | **tenue** : `git diff --stat 289c9a2 667fd2f` ne touche aucun de ces chemins (`src/` : `economy_tiers.c`, `entity_geom.c/.h`, `galaxy.c`, `planet_family.c`, `probe.c/.h`, `research_icon.c`, `slipway.c/.h`, `slipway_display.c`, `wormhole.c` ; TU `fr`, `g_`, `hj`, `jc`, `mx`, `nl`, `nr`, `os`, `qh` ; `docs/`) ; comptes des sept autres fichiers d'appel identiques à `289c9a2` |
| H-D15b-6 | **tenue** : note « Include retiré : `slipway_display.c` ; en-têtes vidés gardés pour D.19c : `entity_geom.h`, `probe.h`, `slipway.h` » ; `slipway_display.c` ne nomme plus le module |
| H-D15b-7 | **tenue** : ligne D.15c du plan identique ; « Pour D.15c : `tostr` intacts, `galaxy.c:79`, `probe.c:273`, `research_icon.c:138` » |
| H-D15b-8 | **tenue** : un commit (`33dc521`), VERIFIED |
| H-D15b-9 | **tenue** : aucun `pico8_str.h` ni `pico8_tostr*` ajouté (seuls `pico8_str.c:1`, `test_pico8_str.c:1` l'incluent) ; `Makefile`, `tools/` inchangés ; aucun TU ajouté |

Conséquences : aucune décision, ni l'empreinte, ni le découpage (un commit) ne changent. Changent : les comptes de départ (906 / 101 / 1025, `tests/` 470 / 517, résidu D 1147) et donc les comptes d'arrivée (**879 / 99 / 971**, `tests/` 464 / 505, résidu D **1120 / 108**) ; le numéro `research_icon.c:138` n'est plus une hypothèse.

### Sections revérifiées sur `667fd2f`

Refaites : en-tête, verdicts, « Lecture » (compte), « État de départ », Inventaire (numéros : `research_icon.c:138` ; tous les autres remesurés par grep, identiques), « Effet sur `make data-residue` » (départ, arrivée, par fichier), Sortie (compte), tableau « Runs », critères `FAILED:` (départ, comptes), Surprises, Annexe A (ligne D.15b et sa note recopiées). Inchangées et vérifiées : décisions, correspondance des tests (`test_pico8_value.c` et `test_pico8_str.c` identiques à `289c9a2`), includes (`galaxy.c:12` à retirer), `Makefile` (`:407`, `:414` identiques), empreinte (20 chemins), budget.

### Lecture de la ligne pour D.15c

- **« `tostr` déplacé dans `pico8_str.c` »** : l'étude nomme le nouveau symbole `char *pico8_tostr_fixed(pico8_fixed_t)`. **Tranché (G)** : ce nom, déclaré dans `pico8_str.h` à côté de `pico8_sub`, défini dans `pico8_str.c` après lui. C'est un **déplacement vers un module qui a son propre nom**, déclaré comme tel : ce n'est ni un renommage pour échapper aux greps (la fonction d'origine ne disparaît pas, elle garde son nom et ses appelants à genres mêlés) ni un alias. Corps : les lignes de la branche `FIXED` de `pico8_value.c:69-101`, **mot pour mot** (y compris les conversions `(uint32_t)` et `(unsigned long)` déjà présentes, le `/* TROU-22 */` et le commentaire « Round half to even »), la valeur lue devenant le paramètre ; la copie finale `malloc`/`memcpy` écrite comme dans `pico8_sub` (même module, même contrat), pas par une aide publiée.
- **`pico8_value_tostr` ne meurt pas ici** (surprise, déclarée) : l'étude compte 20 sites « toujours un nombre », mais trois autres appelants de `src/` lui passent une valeur à genres mêlés : `research_display.c:57` (`qe`, nil ou nombre : B2, D.17), `pi.c:105` (`a1`, nombre filtré par son genre : B10, D.18c), `score_panel_display.c:196` (`s[i]`, nombre, chaîne ou nil : B11, D.18c). **Tranché (G)** : `pico8_value_tostr` garde son nom, sa signature, ses branches chaîne, booléen et `REF` ; sa branche `FIXED` devient `return pico8_tostr_fixed(v.as.fixed);`. Un seul formatage existe. Ce n'est pas une cale valeur ↔ typé : la fonction existait, elle a trois appelants vivants de `src/`, chacun nommé avec l'étape qui le retire, et elle meurt avec le module (D.19c). Rejetés : garder deux copies du formatage (deux TROU-22 à tenir) ; convertir ces trois appelants maintenant (B2, B10, B11 : hors étape).
- **« avec ses tests »** : **Tranché (G)** : les cas « nombre » passent dans le TU **existant** `tests/test_pico8_str.c` (« le TU de `pico8_str` » de l'étude), pas dans un TU nouveau. Raison : même module, aucune entrée nouvelle dans `UNIT_TESTS`, aucune règle nouvelle. Rejeté : `test_pico8_tostr` (un binaire, une règle et une entrée de plus pour 7 fonctions de test).
- **Cas chaîne et booléen** : l'étude dit « supprimés avec leur genre ». Leur genre ne meurt pas ici (voir ci-dessus). **Tranché (G)** : `test_tostr_string_is_content_passthrough` et `test_tostr_bool` **restent** dans `test_pico8_value`, inchangés ; ils partent avec la fonction (D.19c). Écart à la lettre de l'étude, déclaré. Aucun cas `REF` n'existe aujourd'hui.
- **« TROU-22 et TROU-13 suivent la fonction »** : TROU-22 est le formatage d'un nombre : **il suit** (`/* TROU-22 */` dans `pico8_str.c`, texte de `docs/trous-de-portage.md` réécrit sur `pico8_tostr_fixed`, fichier `src/pico8_str.c`). TROU-13 est la branche `REF` (un champ absent rendu `""`), qui **reste** dans `pico8_value.c` avec son appelant `score_panel_display.c:196` : **il ne suit pas** (écart à l'étude, déclaré ; il sera réécrit par D.18c, qui écrit `""` au site).
- **« Compte : ≈ 880 »** : mesuré à `667fd2f` : **27 lignes, 54 occ. retirées**, aucune gagnée : `valeur` **906 → 879** (1025 → 971 occ.). « ≈ » n'est pas un critère ; « − 27 lignes, − 54 occ., exact, écart expliqué » l'est.
- **Pourquoi 19 sites quittent le motif et non 20** : `planet_display.c:377` enveloppe le résultat dans `pico8_value_string(…)` (`h_item` d'un bouton, B9, D.18b) ; la ligne perd ses deux occurrences `tostr`/`fixed` mais garde la troisième : ligne comptée **modifiée, non retirée**, déclarée.
- **« `sanitize-net` : oui »** : l'allocation de la chaîne change de module (`pico8_str.c` au lieu de `pico8_value.c`) ; chaque appelant libère comme avant. `make sanitize-net` exigé, plus le balayage ASan+UBSan des TU (consigne de l'utilisateur). `class-coverage-net` : **non** (aucun constructeur, aucun `pico8_class_coverage_record`, aucun `KNOWN[]` touché).
- **§6** : `test_pico8_value` est un test de la couche ; il **ne perd pas** son objet ici (la fonction reste), il perd seulement des cas qui n'étaient pas des tests de la couche (l'étude : « ce n'est pas un test de la couche »). Les trois tests de régression (`layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`) ne nomment aucun symbole de l'étape : recompilés, relancés, rien de plus.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent (aucun test ajouté ; le balayage exhaustif, déjà dans `make test`, est déplacé, pas élargi) ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes comptées, alias, renommage pour échapper aux greps, cast nouveau, cale ; mutations restaurées par `cp` simple puis `touch` ; binaires lancés exactement comme le `Makefile` ; **jamais l'appli raylib**.

### État de départ

| Mesure | À `289c9a2` | Départ réel de D.15c (`667fd2f`, D.15b gradée) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 980 / 103 ; 1105 occ. (`src/` 488 / 65 / 563, `tests/` 492 / 38 / 542) | **906 / 101 ; 1025 occ.** (`src/` 436 / 63 / 508, `tests/` 470 / 38 / 517) |
| `type-tag` | 243 / 58 | 243 / 58 |
| résidu D | 1221 / 111 | **1147 / 110** |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |

Recette des occurrences (celle de D.12–D.15b, motif `valeur` de `tools/data_residue.sh:29`) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'valeur\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire (numéros à `667fd2f`, remesurés par contenu ; identiques à `289c9a2` sauf `research_icon.c`)

Classes du message de commit : **(a1)** même site, fonction typée à la place de la valeur (y compris un cas de test déplacé tel quel, un pour un) ; **(a2)** retirée sans remplaçant (include devenu inutile) ; **(b)** signature publique dont le type change : **attendu 0** (`pico8_value_tostr` garde la sienne ; `pico8_tostr_fixed` est nouvelle) ; **(c)** même donnée, autre nom : **attendu 0**.

#### API

| Symbole | Aujourd'hui | Après D.15c |
|---|---|---|
| `pico8_tostr_fixed` (nouveau, `pico8_str.h`, `pico8_str.c`) | — | `char *pico8_tostr_fixed(pico8_fixed_t x);` chaîne nouvellement allouée, terminée par NUL, que l'appelant possède et libère (contrat de `pico8_sub`) |
| `pico8_value_tostr` (`value.h:73`, `value.c:61`) | formate les quatre genres | inchangée pour l'appelant ; `FIXED` délègue à `pico8_tostr_fixed` |

`pico8_str.h` gagne `#include "pico8_fixed.h"` (pour `pico8_fixed_t`). Aucune autre signature ne change.

#### Sites de `src/` (21 lignes, 42 occ. retirées ; 1 ligne modifiée non retirée ; aucune gagnée)

| Fichier | Ligne (occ.) | Remplacement | Classe |
|---|---|---|---|
| `collect.c` | `:85` (2) | `char *num_str = pico8_tostr_fixed(d);` | (a1) |
| | `:90` (1) | commentaire de `ne` : « `d < 0` keeps the '-' `pico8_tostr_fixed` already wrote » (même objet, sa fonction a changé de nom) | (a1) |
| `economy_score.c` | `:223` (2) | `happiness_str = pico8_tostr_fixed(happiness);` | (a1) |
| `galaxy.c` | `:79` (2) | `id_num_str = pico8_tostr_fixed(id);` | (a1) |
| | `:12` (1) | `#include "pico8_value.h"` retiré : le fichier ne nomme plus rien du module (H-D15b-2, -6) | (a2) |
| `menu.c` | `:109`, `:186`, `:220`, `:221`, `:222` (2 chacune) | `gi_str = pico8_tostr_fixed(gi_result);` ; `char *gr_str = pico8_tostr_fixed(pico8_fixed_from_int(gr));` ; `tl_str = pico8_tostr_fixed(tl);` ; `diff1_str = pico8_tostr_fixed(pico8_fixed_sub(pico8_fixed_from_int(gr), qi));` ; `diff2_str = pico8_tostr_fixed(pico8_fixed_sub(ek_tl, qi));` | (a1) |
| `planet_display.c` | `:269`, `:275`, `:276` (2 chacune) | `pico8_tostr_fixed(mu)`, `pico8_tostr_fixed(pico8_fixed_from_int(by_count))`, `pico8_tostr_fixed(mu)` | (a1) |
| | `:377` (3 → 1) | `row->h_item = pico8_value_string(jo_dollar(pico8_tostr_fixed(od)));` — **reste comptée** (`pico8_value_string`, B9, D.18b) | modifiée, non retirée |
| `probe.c` | `:273` (2) | `char *od_str = pico8_tostr_fixed(pb->od);` | (a1) |
| `research_display.c` | `:107`, `:235` (2 chacune) | `pico8_tostr_fixed(self->o)`, `pico8_tostr_fixed(jq)` | (a1) |
| `research_icon.c` | `:138` (2) (`:136` à `289c9a2`) | `id_num_str = pico8_tostr_fixed(new_id);` | (a1) |
| `score_panel.c` | `:200`, `:221` (2 chacune) | `nz_str = pico8_tostr_fixed(fm_nz);`, `happiness_str = pico8_tostr_fixed(fm_happiness);` | (a1) |
| | `:216` (2) | `pico8_tostr_fixed(pico8_fixed_add(po_val, pico8_fixed_from_int(1)));` — **`:215` `plus_num =` reste sur sa ligne** (le tout tiendrait sur une ligne : joindre est interdit) | (a1) |
| `score_panel_display.c` | `:60` (2) | `yr_str = pico8_tostr_fixed(yr);` | (a1) |

Totaux `src/` (lignes / occ. retirées) : `collect.c` 2 / 3, `economy_score.c` 1 / 2, `galaxy.c` 2 / 3, `menu.c` 5 / 10, `planet_display.c` 3 / 8 (dont 2 occ. de `:377`), `probe.c` 1 / 2, `research_display.c` 2 / 4, `research_icon.c` 1 / 2, `score_panel.c` 3 / 6, `score_panel_display.c` 1 / 2 : **21 lignes, 42 occ.**

Hors motif, dans le même commit :

- `pico8_value.c` : `:3` `<stdint.h>` et `:4` `<stdio.h>` retirés (seul le code déplacé s'en servait) ; `#include "pico8_str.h"` ajouté ; `:62` (`char buf[64];`) et `:70-100` partent dans `pico8_str.c` ; `:69` `case PICO8_VALUE_FIXED: {` devient `case PICO8_VALUE_FIXED:` suivi de `return pico8_tostr_fixed(v.as.fixed);` — **ligne comptée modifiée, non retirée** (1 occ. avant, 1 après), déclarée ; `:101` (`}`) part. `pico8_strdup_owned` reste (branches chaîne, booléen, `REF`, défaut). `pico8_value.c` : **23 / 30 occ., inchangé**.
- `pico8_value.h:60-72` : le paragraphe `FIXED` (`:64-67`, hors motif) devient « FIXED is rendered by pico8_tostr_fixed (pico8_str.h). » ; `:68-71` et `/* TROU-13 */` (`:72`) restent ; `:71` et `:73` (comptées) **inchangées**. `pico8_value.h` : **22 / 30 occ., inchangé**.
- `pico8_str.h` : `#include "pico8_fixed.h"` ; prototype de `pico8_tostr_fixed` avec son contrat (propriété ; quatre décimales arrondies au pair, zéros de fin et point nu élagués ; 13 témoins sur 14 du bloc C ; magnitudes sous 0.0001 : TROU-22) — commentaire de contrat C, l'une des quatre sortes de `CLAUDE.md`. **Piège** : ni `double` ni `float` dans ce commentaire ni dans `pico8_str.c` (`make float-grep` lit les commentaires du cœur, `tools/float_grep.sh:78`) ; ni `pico8_value_*` (la ligne gagnerait `valeur`).
- `#include "pico8_str.h"` ajouté dans **les 10 fichiers d'appel** et `pico8_value.c` (aucun ne l'inclut à `667fd2f`, directement ou non), à sa place alphabétique quand le bloc des includes locaux est trié, sinon en fin de bloc.

#### Tests : correspondance case par case (6 lignes, 12 occ. retirées de `test_pico8_value` ; aucune gagnée)

| `test_pico8_value.c` (à `667fd2f`, identique à `289c9a2`) | Devient dans `test_pico8_str.c` | Témoins gardés | Classe |
|---|---|---|---|
| `test_tostr_small_positive_integer` `:69-73` (`:70`, 2 occ.) | même nom, `pico8_tostr_fixed(pico8_fixed_from_int(3))` → `"3"` | `3` → `"3"` | (a1) |
| `test_tostr_zero` `:75-79` (`:76`, 2) | idem, `0` → `"0"` | `"0"` | (a1) |
| `test_tostr_negative_integer` `:81-85` (`:82`, 2) | idem, `-5` → `"-5"` | `"-5"` | (a1) |
| `raw_fixed` `:102-104` | déplacée telle quelle | — | hors motif |
| `assert_tostr_raw` `:106-114` (`:107`, 2) | `char *s = pico8_tostr_fixed(raw_fixed(raw));`, message `fprintf` gardé | — | (a1) |
| `test_tostr_matches_probe_block_c` `:116-130` | déplacée **mot pour mot** : les **13 témoins** du bloc C (`0xfffe8000u` `"-1.5"` … `0x0000aaaau` `"0.6667"`) | 13 | hors motif |
| `test_tostr_below_0_0001_keeps_port_output` `:132-137` (`:133`, 2) | `pico8_tostr_fixed(raw_fixed(0xfffffffau))` → `"-0.0001"` ; `assert` porteur de message TROU-22 gardé mot pour mot | brut −6 | (a1) |
| `test_tostr_exact_ties_round_half_to_even` `:139-142` | déplacée mot pour mot | `0x00000800u` `"0.0312"`, `0x00001800u` `"0.0938"` | hors motif |
| `old_tostr_percent_4f_for_test` `:144-159` (et son commentaire) | déplacée mot pour mot (oracle de test, dans `tests/` : hors `float-grep`) | — | hors motif |
| `check_against_old_tostr` `:161-172` (`:164`, 2) | `char *s = pico8_tostr_fixed(raw);` | — | (a1) |
| `is_exact_4_decimal_tie` `:174-177` | déplacée mot pour mot | — | hors motif |
| `test_tostr_matches_old_percent_4f_exhaustively` `:179-205` | déplacée mot pour mot : `inputs == 1245183`, `ties == 16 * (n_int_parts + 1)` | 1 245 183 entrées, 304 égalités | hors motif |
| appels `:217-219`, `:222-225` de `main` | ajoutés au `main` de `test_pico8_str`, après les cinq `test_sub_*`, dans le même ordre ; le `printf` final de chaque TU inchangé | — | hors motif |
| `test_tostr_string_is_content_passthrough` `:87-91`, `test_tostr_bool` `:93-100` | **restent** dans `test_pico8_value` (voir « Lecture ») | `"hi"`, `"true"`, `"false"` | — |
| `#include <stdint.h>` `:4` | retiré de `test_pico8_value` (plus rien n'y nomme `uint32_t`/`int32_t`) ; ajouté à `test_pico8_str` | — | hors motif |

Sept fonctions de test et cinq aides quittent `test_pico8_value` ; sept fonctions de test et cinq aides entrent dans `test_pico8_str` ; **aucun témoin perdu** : 3 entiers, 13 témoins du bloc C, le brut −6, 2 égalités exactes, 1 245 183 entrées exhaustives. `test_pico8_value` : **33 / 64 occ. → 27 / 52** ; `test_pico8_str` : 0 → 0.

#### Includes (G, règle de D.15b)

Un **`.c`** qui ne nomme plus rien du module perd son `#include "pico8_value.h"` dans ce commit (précédents : `test_pico8_fm.c:35` à D.15a, `slipway_display.c:11` à D.15b, H-D15b-6). Appliqué : **`galaxy.c:12` retiré** (sous H-D15b-2, ses seules lignes restantes sont `:12` et `:79`) ; `collect.c` sort du motif **sans** include à retirer (il reçoit le type par `pico8_collect.h:7`, en-tête gardé jusqu'à D.19c). Aucun autre `.c` de l'étape ne se vide (`menu.c`, `planet_display.c`, `probe.c`, `research_display.c`, `research_icon.c`, `score_panel.c`, `score_panel_display.c`, `economy_score.c` gardent d'autres lignes `valeur`). Aucun include de `pico8_value.h` ajouté ; si la compilation en exige un pour `galaxy.c`, il est remis et déclaré avec l'erreur du compilateur (aucun attendu : `galaxy.c` n'y prend plus que `pico8_fixed.h`, qu'il reçoit par `pico8_vec.h`, `pico8_planet.h` et désormais `pico8_str.h`).

#### `Makefile` et liens

- **Deux lignes de prérequis changent, et elles seules** (exigées par le déplacement) :
  - `Makefile:407` `test_pico8_value` : `src/pico8_str.c` ajouté (la branche `FIXED` de `pico8_value.c` appelle `pico8_tostr_fixed`) ;
  - `Makefile:414` `test_pico8_str` : `src/pico8_fixed.c` ajouté (les trois cas entiers déplacés appellent `pico8_fixed_from_int`, `pico8_fixed.h:23`, fonction non inline). La recette reste sans `-lm` : le code déplacé n'appelle que la libc (`snprintf`, une division). Si l'édition de liens l'exige, `-lm` est ajouté comme à `test_pico8_value`, et déclaré.
- Aucune entrée d'`UNIT_TESTS` ni de liste de sources ne change : `pico8_str.c` est déjà dans `ENGINE_SRCS` (`:212`), donc `CORE_SRCS` et `REPLAY_SRCS`, et dans `DATA_ENGINE_SRCS` (`:271`) ; `pico8_value.c` aussi (`:217`, `:276`).
- **Règle de lien** : aucun fichier de `DATA_ENGINE_SRCS` (`Makefile:229-279`) ne nomme un symbole défini hors de cette liste. `pico8_value.c` (dans la liste) appelle `pico8_tostr_fixed`, défini dans `pico8_str.c` (dans la liste) ; les appelants de la liste (`collect.c`, `economy_score.c`, `planet_display.c`, `probe.c`, `research_display.c`) aussi ; `galaxy.c`, `menu.c`, `research_icon.c`, `score_panel.c`, `score_panel_display.c` n'y sont pas et appellent la liste, ce qui est permis. Vérifié par `awk` sur les règles jointes : **seule** la règle de `test_pico8_value` liait `pico8_value.c` sans `pico8_str.c` ; `test_pico8_layout_nil_item_draws_nothing` (`:561`) lie déjà les deux. Les binaires liés sur `DATA_ENGINE_SRCS` seule (`data_init`, `map_tables`, `jb_ol`, `planet_kj_dispatch`, `bg_kj`), sous ASan (`planet_gh_row_lq`, `:550`) ou avec `planet_family.c` (`ga_export_panel_by`, `:553`) doivent se lier (grep 7).
- `tools/float_guard.txt` : réécrit à l'identique par `make test` (`garde fichiers 58 en-echec 0` : aucun fichier de cœur ajouté ; `pico8_str.c` ne demande aucun registre flottant, comme `pico8_value.c` aujourd'hui). `tools/line_coverage_reference.txt` nomme encore `pico8_value.c:*` et `pico8_str.c:*` à d'anciens numéros : périmé depuis D.12d, **non touché** (hors `make test`).

## Solution

**Un commit** (20 chemins : 14 de `src/`, 2 TU, `Makefile`, `docs/trous-de-portage.md`, `docs/glossaire-noms.md`, `docs/plan-3.md` pour la note), vert seul, vérifié et poussé :

1. `pico8_str.c/.h` gagnent `pico8_tostr_fixed`, corps de la branche `FIXED` déplacé mot pour mot, même contrat de propriété que `pico8_sub`.
2. `pico8_value_tostr` délègue ses nombres à `pico8_tostr_fixed` ; ses branches chaîne, booléen, `REF` et ses trois appelants à genres mêlés ne changent pas.
3. Les 20 sites `pico8_value_tostr(pico8_value_fixed(x))` appellent `pico8_tostr_fixed(x)` ; `galaxy.c` perd son include devenu inutile.
4. Les cas « nombre » de `test_pico8_value` passent tels quels dans `test_pico8_str` ; les cas chaîne et booléen restent.
5. Deux prérequis du `Makefile` suivent ; TROU-22 et le glossaire nomment la nouvelle fonction.

**Sortie de l'étape**, constatée par commande sur le commit :

- `make test` vert (dont `make data-guard`, `float-grep`, `float-guard`, `constants-check`) ; `make e2e` : **26 conformes** ; `make sanitize-net` : **26 propres** ; balayage ASan+UBSan des TU propre ;
- `make data-residue` : `valeur` **906 → 879** lignes (**1025 → 971** occ.), fichiers **101 → 99** (`collect.c` et `galaxy.c` sortent de `valeur` **et** du résidu D : aucune ligne `type-tag`) ; `type-tag` 243 / 58 inchangé ; résidu D **1147 / 110 → 1120 / 108** ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/`, rien dans `tools/` (`float_guard.txt` réécrit à l'identique) ; dans le `Makefile`, les deux lignes de prérequis et elles seules ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. plus aucun tostr d'une valeur fabriquée pour un nombre : vide
  grep -rn 'pico8_value_tostr(pico8_value_fixed' src tests
  # 2. pico8_value_tostr : exactement 4 lignes (définition, pi.c, research_display.c, score_panel_display.c)
  grep -nE 'pico8_value_tostr\(' src/*.c
  # 3. appels et définition de pico8_tostr_fixed : 22 lignes (20 sites, la définition, la délégation de pico8_value.c)
  grep -nE 'pico8_tostr_fixed\(' src/*.c | wc -l
  # 4. le nouveau code ne gagne pas valeur : 0, 0, 0
  grep -cE "$P" src/pico8_str.c src/pico8_str.h tests/test_pico8_str.c
  # 5. témoins : 15 appels (13 du bloc C, 2 égalités) dans test_pico8_str, 0 dans test_pico8_value ;
  #    le balayage exhaustif est dans test_pico8_str seul
  grep -c 'assert_tostr_raw(0x' tests/test_pico8_str.c tests/test_pico8_value.c
  grep -l 'inputs == 1245183' tests/*.c
  # 6. aucun mot flottant dans le code déplacé du cœur : vide
  grep -nE '\b(double|float)\b' src/pico8_str.c src/pico8_str.h
  # 7. règle de lien et les deux TU
  make tmp/test_pico8_str tmp/test_pico8_value tmp/test_pico8_data_init tmp/test_pico8_map_tables \
       tmp/test_pico8_jb_ol tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_bg_kj \
       tmp/test_pico8_planet_gh_row_lq tmp/test_pico8_ga_export_panel_by
  # 8. aucune constante de TU changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 9. includes : exactement '-#include "pico8_value.h"' (galaxy.c) et 11 '+#include "pico8_str.h"'
  git diff <parent> | grep -E '^[-+]\s*#include "(\.\./src/)?pico8_(value|str)\.h"'
  # 10. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^valeur'
  # 11. Makefile : deux lignes '-', deux lignes '+', celles des règles test_pico8_value et test_pico8_str
  git diff <parent> -- Makefile | grep -E '^[-+][^-+]'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que le formatage d'un nombre soit une fonction typée de `pico8_str`, afin qu'il survive à la suppression du module de la valeur Lua.
2. En tant que mainteneur du port, je veux que `pico8_tostr_fixed` rende une chaîne que l'appelant libère, comme `pico8_sub`, afin que les deux fonctions de chaîne du module aient le même contrat.
3. En tant que mainteneur du port, je veux que les vingt sites qui formatent un nombre l'appellent directement, afin qu'aucun ne fabrique une valeur pour la défaire aussitôt.
4. En tant que mainteneur du port, je veux qu'il n'existe qu'un formatage, afin que TROU-22 ne soit tenu qu'à un endroit.
5. En tant que mainteneur du port, je veux que les trois appelants à genres mêlés de `pico8_value_tostr` restent tels quels, chacun nommé avec l'étape qui le convertira, afin que cette étape ne déborde pas sur B2, B10 et B11.
6. En tant que mainteneur du port, je veux que `galaxy.c`, vidé du module, perde son include, afin que les includes inutiles partent au fil des étapes.
7. En tant que gardien des tests, je veux que les 13 témoins du bloc C, le brut −6, les deux égalités exactes, les trois entiers et le balayage exhaustif soient dans le TU de `pico8_str`, afin qu'ils ne partent pas avec le test de la couche.
8. En tant que gardien des tests, je veux une table de correspondance ancienne fonction → nouvelle fonction, afin de prouver qu'aucun cas ni aucun témoin n'est perdu.
9. En tant que gardien des tests, je veux que les cas chaîne et booléen restent là où leur fonction reste, afin qu'aucune fonction encore appelée ne perde ses tests.
10. En tant que gardien des tests, je veux des mutations du formatage qui rougissent `test_pico8_str` et laissent `test_pico8_value` vert, afin de prouver que les tests déplacés mordent et qu'ils ont bien quitté l'ancien TU.
11. En tant que gardien des tests, je veux une mutation de la délégation qui rougisse un TU, afin que la branche `FIXED` de `pico8_value_tostr` reste gardée sans test ajouté.
12. En tant que gardien des filets, je veux `make test`, `make e2e`, `make sanitize-net` et le balayage ASan+UBSan des TU verts, afin de prouver qu'aucun pixel, aucune chaîne ni aucune libération ne change avec le module qui alloue.
13. En tant que gardien du compteur, je veux au message du commit les comptes par catégorie et par fichier en lignes et en occurrences, la classification (a1)/(a2)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris), les lignes comptées modifiées non retirées et chaque Δlignes > Δocc justifié, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
14. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe (`score_panel.c:215-216`), qu'aucun alias, renommage d'évitement, cast nouveau ni cale ne soit ajouté, afin que « − 27 » veuille dire « retiré ».
15. En tant que gardien de la construction, je veux que le `Makefile` ne change que des deux prérequis exigés par le déplacement, et que la règle de lien de `DATA_ENGINE_SRCS` tienne, afin que chaque binaire se lie comme avant.
16. En tant que `plan-pilot`, je veux une étape d'environ un quart d'heure, un repli écrit, et une note « Découvert à D.15c » qui dise les comptes réels et ce que D.17, D.18c et D.19c trouvent, afin que la suite parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)** ; les rejets sont notés à chaque décision. Q12 (oui) et Q13 (A) sont des décisions humaines ; l'étude D.14 (G1–G7) fait foi sauf écart déclaré ici.

### Modules

- **Chaînes** (`pico8_str.c/.h`) : un module profond et minuscule, deux fonctions pures au même contrat (`sub`, `tostr` d'un nombre), testées en isolation par un TU qui ne lie que le module et `pico8_fixed.c`.
- **Valeur** (`pico8_value.c/.h`) : perd son formatage, garde sa forme publique.
- **Appelants** : `collect.c` (`ne`), `economy_score.c`, `galaxy.c` (`bw`), `menu.c`, `planet_display.c`, `probe.c`, `research_display.c`, `research_icon.c` (`ie:ldown`), `score_panel.c`, `score_panel_display.c`.
- **TU** : `test_pico8_str` (reçoit), `test_pico8_value` (cède). **Construction** : `Makefile` (deux prérequis). **Docs** : `trous-de-portage.md` (TROU-22), `glossaire-noms.md` (ligne `tostr(v)`), `plan-3.md` (note).
- Inchangés : `pi.c`, tous les en-têtes hors `pico8_str.h` et `pico8_value.h`, `tools/`, les autres TU (ils prouvent sans suivre), les trois tests §6.

### `pico8_tostr_fixed` (G)

- Déclaration dans `pico8_str.h` après `pico8_sub`, définition dans `pico8_str.c` après `pico8_sub`. `pico8_str.c` gagne `<stdint.h>` et `<stdio.h>`.
- Corps : `char buf[64];` puis les lignes `:71-99` de `pico8_value.c` avec `v.as.fixed` → `x`, puis la copie possédée (`strlen`, `malloc(n + 1)`, `memcpy`), comme `pico8_sub` l'écrit. Pas de `pico8_strdup_owned` publiée (rejeté : API nouvelle pour une ligne) ; pas d'aide statique copiée (rejeté : deux copies d'aide dans deux modules pour un seul appel).
- Aucun cast nouveau : les `(uint32_t)` et `(unsigned long)` sont ceux du corps déplacé, déclarés « déplacés à l'identique » au message.

### `pico8_value_tostr` (G)

- Branche `FIXED` : `return pico8_tostr_fixed(v.as.fixed);`. Le reste de la fonction, son `/* TROU-13 */` et `pico8_strdup_owned` restent. `<stdint.h>`, `<stdio.h>` retirés de `pico8_value.c`.
- Contrat de `pico8_value.h:60-72` : seul le paragraphe du genre `FIXED` est réécrit (renvoi à `pico8_tostr_fixed`) ; les lignes comptées `:71`, `:73` ne bougent pas.

### Appelants (G)

- Chaque site : `pico8_value_tostr(pico8_value_fixed(E))` → `pico8_tostr_fixed(E)`, `E` inchangée, libération inchangée. Aucune ligne jointe ni coupée autrement qu'avant ; `score_panel.c:215-216` reste sur deux lignes.
- `collect.c:90-91` : le commentaire de `ne` nomme la nouvelle fonction (sorte « piège qu'on réintroduirait » : le `-` vient du formatage, seul le `+` est ajouté ici).
- `score_panel_display.c:193-195` (commentaire qui nomme `pico8_value_tostr` pour `s_val`) : reste vrai, inchangé.

### Les TU suivent (G, règle commune de l'étude)

- `test_pico8_str` reçoit les sept fonctions de test et cinq aides de la table, dans l'ordre d'origine, après `test_sub_empty_range_when_j_less_than_i` ; `main` les appelle après les cinq `test_sub_*`. Les noms `test_tostr_*` sont gardés (ils disent ce qu'ils testent ; aucun ne nomme la valeur).
- `test_pico8_value` perd ces fonctions, leurs appels et `<stdint.h>` ; ses constructeurs, égalités, cas chaîne et booléen restent.
- **Constantes** : aucune `*_HASH`/`identity[]` ne change (aucune n'est touchée : l'étape ne change aucune chaîne produite).
- **Assertion nouvelle** : admise seulement pour couvrir une ligne modifiée non exécutée ou pour faire rougir une mutation qui ne rougit rien, dans un test du module, listée au message avec la ligne qu'elle couvre ; rien de lent, aucun fichier d'événements. Attendu : aucune (la délégation est gardée par `test_pico8_pj:763`, voir M7).

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la valeur est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs (`score_panel.c:215-216` nommément) ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours pour échapper au motif (`pico8_value_tostr` garde son nom ; `pico8_tostr_fixed` est un déplacement vers le module `str`, déclaré) ; alias (macro ou `typedef` qui renomme `pico8_value_t` ou une de ses fonctions ; `#define pico8_tostr_fixed …` ; enveloppe `pico8_value_tostr_fixed`) ; cast nouveau ; **cale** (fonction de conversion valeur ↔ typé ajoutée, formatage tenu en double) ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; toute modification d'une constante de hachage, de `tests/e2e/`, d'un `*.sha256`, de `tools/`, d'une ligne du `Makefile` autre que `:407` et `:414`, d'un `#include "pico8_value.h"` autre que `galaxy.c:12`.
- **Classification exigée** au message, pour chaque ligne `valeur` retirée ou modifiée (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) : attendu **(a1) 26** (19 sites, `collect.c:90`, 6 lignes de `test_pico8_value`), **(a2) 1** (`galaxy.c:12`), **(b) 0**, **(c) 0**, et **2 lignes comptées modifiées non retirées** (`planet_display.c:377` : 3 → 1 occ. ; `pico8_value.c:69` : 1 → 1 occ.).
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et **par fichier** du diff : lignes (`--detail`) **et** occurrences (recette).
- **Lignes qui gagnent une catégorie**, par fichier et par catégorie, « aucune » compris. Attendu : **aucune** dans aucune catégorie (grep 4). Toute autre est à justifier.
- **Δlignes > Δoccurrences** : chaque couple justifié. Attendu : aucun (Δocc ≥ Δlignes dans chaque fichier ; `planet_display.c` 3 / 8, `pico8_value.c` 0 / 0).
- **Numéros de ligne** : ceux du **parent du commit**, jamais ceux de ce PRD sans remesure.

### Empreinte et découpage (G)

| Commit | `src/` | `tests/` | autres | Chemins |
|---|---|---|---|---|
| **D.15c** | `str.c`, `str.h`, `value.c`, `value.h`, `collect.c`, `economy_score.c`, `galaxy.c`, `menu.c`, `planet_display.c`, `probe.c`, `research_display.c`, `research_icon.c`, `score_panel.c`, `score_panel_display.c` (14) | `test_pico8_str`, `test_pico8_value` (2) | `Makefile`, `trous-de-portage.md`, `glossaire-noms.md`, `plan-3.md` | 20 |

(L'étude disait ≈ 16 fichiers : elle ne comptait ni `pico8_value.c/.h`, ni le `Makefile`, ni les docs.)

- **Pourquoi un seul commit** : 20 chemins, substitutions mécaniques d'une expression par une autre ; D.15a (11 chemins, gcov et mutations compris) a pris ≈ 7 min entre la mise en file du PRD et le commit, ≈ 12 min jusqu'à la graduation.
- **Repli écrit** (G) : si le plafond est atteint sans commit, rien n'est commis et `plan-pilot` relance en deux commits, même PRD, chacun vert seul : **c1** — `pico8_tostr_fixed`, délégation de `pico8_value_tostr`, tests déplacés, deux prérequis du `Makefile`, TROU-22 et glossaire (`valeur` : − 6 lignes de `test_pico8_value`) ; **c2** — les 20 sites, l'include de `galaxy.c`, `collect.c:90` (− 21). Rejeté comme plan principal : deux coûts fixes pour une étape d'un quart d'heure.

### Effet sur `make data-residue`

| Catégorie | À `289c9a2` | Départ (`667fd2f`) | Après D.15c | Pourquoi |
|---|---|---|---|---|
| `valeur` | 980 / 103 ; 1105 occ. | 906 / 101 ; 1025 occ. | **879 / 99 ; 971 occ.** (− 27, − 54) | Inventaire : `src/` − 21 / − 42, `tests/` − 6 / − 12 |
| `valeur` `src/` | 488 / 65 ; 563 occ. | 436 / 63 ; 508 occ. | **415 / 61 ; 466 occ.** | `collect.c`, `galaxy.c` sortent |
| `valeur` `tests/` | 492 / 38 ; 542 occ. | 470 / 38 ; 517 occ. | **464 / 38 ; 505 occ.** | `test_pico8_value` garde des lignes |
| `type-tag` | 243 / 58 | 243 / 58 | **inchangé** | aucune ligne `type-tag` touchée |
| résidu D | 1221 / 111 | 1147 / 110 | **1120 / 108** | ni `collect.c` ni `galaxy.c` n'ont de ligne `type-tag` |
| `DATA_RESIDUE_GUARD` | 8 catégories | inchangé | inchangé | `valeur` ≠ 0 |

Par fichier (lignes / occ. ; à `289c9a2` → à `667fd2f` → après D.15c) : `collect.c` 2/3 → 2/3 → 0/0 ; `economy_score.c` 3/4 → 3/4 → 2/2 ; `galaxy.c` 3/4 → 2/3 → 0/0 ; `menu.c` 25/30 → 25/30 → 20/20 ; `planet_display.c` 44/51 → 44/51 → 41/43 ; `probe.c` 21/23 → 16/18 → 15/16 ; `research_display.c` 29/37 → 29/37 → 27/33 ; `research_icon.c` 8/11 → 2/4 → 1/2 ; `score_panel.c` 25/32 → 25/32 → 22/26 ; `score_panel_display.c` 25/26 → 25/26 → 24/24 ; `pico8_value.c` 23/30, `pico8_value.h` 22/30 : inchangés ; `test_pico8_value.c` 33/64 → 33/64 → 27/52 ; `pico8_str.c`, `pico8_str.h`, `test_pico8_str.c` : 0/0.

« − 27 », « − 54 occ. », « 879 / 971 », « inchangé » sont des critères (écart = explication chiffrée au message).

### Documentation (G)

- `docs/trous-de-portage.md`, **TROU-22** : « `pico8_value_tostr` écrit un nombre… » → « `pico8_tostr_fixed` écrit un nombre… » ; « Fichier : `src/pico8_value.c`. » → « Fichier : `src/pico8_str.c`. ». **TROU-13** : inchangé (sa fonction et ses fichiers restent vrais).
- `docs/glossaire-noms.md:293` : la ligne `tostr(v)` nomme `pico8_tostr_fixed` (nombre ; `pico8_str.h`) et `pico8_value_tostr` (genres mêlés, jusqu'à D.19c). La convention de la ligne `:655` (« `tostr`, `sub`, `unstash` et `ne` rendent une chaîne possédée par l'appelant ») reste vraie.
- `docs/plan-3.md` : note « Découvert à D.15c » en fin de ligne D.15c, rien d'autre. Elle dit : le commit ; les comptes réels ; les lectures de la ligne (TU existant ; `pico8_value_tostr` gardée et déléguant, avec ses trois appelants et leurs étapes ; cas chaîne et booléen gardés ; TROU-13 ne suit pas) ; les deux prérequis du `Makefile` ; l'include de `galaxy.c` ; **pour D.17** : `research_display.c:57` passe `qe` à `pico8_value_tostr` ; **pour D.18b** : `planet_display.c:377` garde `pico8_value_string` autour du formatage ; **pour D.18c** : `pi.c:105`, `score_panel_display.c:196` ; **pour D.19c** : `test_pico8_value` ne teste plus que la couche (constructeurs, égalité, chaîne, booléen), `Makefile:407` lie `pico8_str.c`.
- Commentaires : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.15c ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, toute ligne du `Makefile` hors `:407` et `:414`, tout `#include "pico8_value.h"` hors `galaxy.c:12` ; aucun e2e ni TU ajouté ou supprimé.
- Toute constante `*_HASH`/`identity[]` de TU.
- Le comportement : mêmes chaînes au caractère près, même propriétaire (l'appelant), mêmes libérations ; `make e2e` conforme au bit près.
- `pico8_value_tostr` pour l'appelant ; `pi.c` ; les copies `truthy` ; `pico8_value_equal`.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : sous `-Wall -Wextra -Werror`, `pico8_value.c` compile sans `<stdint.h>`/`<stdio.h>` (plus de `uint32_t` ni de `snprintf`) ; `pico8_strdup_owned` reste utilisée.
- **H2** : `pico8_str.c` passe `float-grep`, `float-guard` et `-mgeneral-regs-only` comme `pico8_value.c` aujourd'hui (même code, `snprintf` d'entiers).
- **H3** : aucun autre appelant de `pico8_value_tostr(pico8_value_fixed(…))` n'existe hors les 20 de l'Inventaire (grep à `289c9a2` et `667fd2f`, `src/` et `tests/`) ; un oublié se révèle au grep 1 et suit.
- **H4** : le balayage exhaustif prend, sous `--coverage` ou ASan+UBSan, quelques secondes au plus (1 245 183 `malloc`/`free`) : il était déjà dans `make test`, il n'est pas élargi.
- **H5** : la branche `FIXED` de `pico8_value_tostr` est exécutée par l'union des TU (`test_pico8_pj`, par `score_panel_display.c:196` : le total `"14"`, `:742-763`) ; `test_pico8_value` ne l'exécute plus.

## Implementation Chunks

Un seul commit. Ordre de travail :

1. Mesure de départ (`667fd2f` : `valeur` 906 / 101 / 1025 occ., `type-tag` 243 / 58, résidu D 1147 / 110) ; `gcov` de départ.
2. `pico8_str.c/.h` (`pico8_tostr_fixed`) ; `pico8_value.c/.h` (délégation, includes, contrat) ; `Makefile:407`, `:414`.
3. `test_pico8_str`, `test_pico8_value` (déplacement case par case) ; `make tmp/test_pico8_str tmp/test_pico8_value` et lancement.
4. Les 20 sites et leurs includes ; `galaxy.c:12` ; `collect.c:90`.
5. `trous-de-portage.md`, `glossaire-noms.md`.
6. `make test`, `make e2e`, `make sanitize-net`, balayage ASan+UBSan ; `gcov` d'arrivée ; mutations M1–M15 ; mesure, classification ; note « Découvert à D.15c » ; **commit**.

## Testing Decisions

Un bon test juge ce que le code rend (la chaîne d'un nombre, au caractère près), pas où le code se trouve. D.15c ne change aucun résultat : la preuve est que les mêmes témoins passent contre la nouvelle fonction, que les mutations du formatage rougissent le TU qui les a reçus, que les 26 e2e restent au bit près, et que les chaînes allouées ailleurs sont libérées pareil (ASan).

### TU touchés (G)

1. `test_pico8_str` (reçoit), `test_pico8_value` (cède).
2. Prouvent sans suivre : `test_pico8_pj` (`:724` `"1%"` par `score_panel.c:221` ; `:763` total `"14"` par la délégation), `test_pico8_ie` (`:422`, `:427` : `"s1"`, `"s2"` par `research_icon.c:138`), les TU qui démarrent une partie (ids `"p…"` de `galaxy.c:79`), et les constantes des TU qui plient des rangées de menu ou de panneau.
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change.
4. **Prior art** : déplacement de cas entre TU avec absorption prouvée par mutations (D.12d, `test_pico8_list` → `test_pico8_ptrs`, M3a–M3g) ; tests exhaustifs d'hôte contre un oracle (D2) ; TU suiveurs dans le même commit (D.10–D.15b).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`), **les TU qui forkent** (enfants de `must_abort`, qui sortent par `_exit`) compilés en plus avec **`-D_exit=exit`** pour que l'enfant écrive ses compteurs ; lancés comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour `test_pico8_planet_gh_row_lq`) ; union des lignes de `src/` exécutées, départ et arrivée.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée, sinon justifiée au message (déclarations). Attendus : `pico8_str.c` entier par `test_pico8_str` seul (négatif, égalité exacte, retenue à 10000, élagage, point nu : le balayage exhaustif passe par chacun) ; la délégation `pico8_value.c` par `test_pico8_pj` (H5) ; les 20 sites par leurs TU (liste au rapport), sinon les e2e.
- **`L`** = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée, appariées par contenu et fonction ; **les lignes déplacées `pico8_value.c:62`, `:70-100` sont appariées à leurs sœurs de `pico8_str.c`**. **Attendu vide.** Partantes, hors `L` : `pico8_value.c:101` (`}` du bloc). Une ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail : fichier sauvegardé par `cp` avant, restauré par `cp` simple puis `touch` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance ; chacune doit rougir au moins le TU nommé, sinon c'est un trou, couvert par une assertion ou justifié (mutation équivalente, démontrée). Pour M1–M6, **`test_pico8_value` est lancé aussi et doit rester vert** (preuve que les cas « nombre » l'ont quitté).

- **M1** arrondi au-dessus (`r >= 2048u`) → `test_pico8_str` (`test_tostr_exact_ties_round_half_to_even`, exhaustif) ;
- **M2** retenue neutralisée (`if (0 && q == 10000u)`) → `test_pico8_str` (exhaustif) ;
- **M3** élagage des zéros de fin neutralisé → `test_pico8_str` (témoins `"1"`, `"2.5"`, entiers) ;
- **M4** point nu gardé (`if (0 && end == dot)`) → `test_pico8_str` (`"1"`, `"3"`) ;
- **M5** signe perdu (`negative ? "" : ""`) → `test_pico8_str` (`"-1.5"`, `"-32768"`, `"-5"`, brut −6) ;
- **M6** facteur des décimales faussé (`625u` → `624u`) → `test_pico8_str` (témoins du bloc C) ;
- **M7** délégation cassée : la branche `FIXED` de `pico8_value_tostr` rend `pico8_strdup_owned("")` → `test_pico8_pj` (`:763`) ; sinon constantes des TU qui dessinent la barre de menu (`pi.c:105`) ; sinon assertion ajoutée à `test_pico8_value` (ligne gagnée, déclarée) ;
- **M8** `pico8_tostr_fixed` rend une chaîne non terminée à sa taille exacte (`malloc(n)`, `memcpy` de `n`) → balayage ASan des TU rouge (`test_pico8_str`) — preuve que le balayage voit la nouvelle allocation ;
- **M9** `research_icon.c:138` formate `0` au lieu de `new_id` → `test_pico8_ie` (`:422`) ;
- **M10** `score_panel.c:221` formate `0` au lieu de `fm_happiness` → `test_pico8_pj` (`:724`) ;
- **M11** `galaxy.c:79` formate `0` au lieu de `id` → TU qui démarrent une partie (ids de planète, `hg`, constantes), sinon `make e2e` ;
- **M12** `collect.c:85` (`ne`) formate `0` au lieu de `d` → constantes des TU qui plient un nombre signé (`hy`, `pj`, menus), sinon `make e2e`, sinon assertion ;
- **M13** `planet_display.c:377` formate `0` au lieu de `od` → TU `hp`/`jo` s'ils plient `h_item`, sinon `make e2e` ;
- **M14** `menu.c:109` formate `0` au lieu de `gi_result` → TU qui construisent la barre du haut, sinon `make e2e` ;
- **M15** (le test lui-même) un témoin du bloc C faussé dans `test_pico8_str` (`"0.27"` → `"0.28"`) → `test_pico8_str` rouge, avec le message `tostr 0x0000451e: got …` : l'assertion déplacée est vivante.

Cibles (numéros à `667fd2f`) : `pico8_str.c` (nouveau), `pico8_value.c:61-107`, `research_icon.c:138-142`, `score_panel.c:198-225`, `galaxy.c:76-82`, `collect.c:84-100`, `planet_display.c:375-378`, `menu.c:105-112`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes) au commit. Aucun e2e ajouté.
- **`make sanitize-net`** (exigé par la ligne) : 26 propres.
- **Balayage ASan+UBSan des TU** : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.12a–D.15b.
- `make class-coverage-net` : non exigé ; s'il est lancé, résultat rapporté.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 15 min de temps réel**, mesures, `gcov`, mutations et filets compris ; **plafond 30 min** (au-delà : repli c1/c2). Base : D.15a, 11 chemins, ≈ 7 min jusqu'au commit et ≈ 12 min jusqu'à la graduation (horodatages de `84bb8ba`, `48d165e`, `77c916a`) ; D.15c a 20 chemins mais des éditions plus simples et 15 mutations d'un `make` ciblé chacune. La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes (`test_pico8_str` : son balayage exhaustif, < 1 s à `-O0`) |
| `make test` | de l'ordre de la minute (`pico8_str.h` change : recompilation large) |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 1 min |
| `make line-coverage EVENTS="tests/e2e/*.json"` | < 1 min |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes |
| mutations (15 ; un `make` ciblé chacune) | < 6 min |
| balayage ASan+UBSan des TU | quelques minutes |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; rien d'inachevé n'est commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | `667fd2f` (D.15b gradée), avant D.15c | 0 | `valeur 906 101` | … |
| 2 | recette `valeur` | idem | 0 | `valeur 906 lignes 101 fichiers 1025 occ.` | … |
| 3 | `grep -rn 'pico8_value_tostr(pico8_value_fixed' src tests \| wc -l` | idem | 0 | `26` (20 `src/`, 6 `tests/`) | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les 20 chemins de « Empreinte » (plus un TU suiveur listé au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`.
2. **Greps** de la « Sortie », exactement.
3. **Formes** : `pico8_tostr_fixed(pico8_fixed_t)` dans `pico8_str.h/.c`, rend une chaîne possédée ; `pico8_value_tostr` délègue sa branche `FIXED` et garde les trois autres ; un seul formatage dans `src/`.
4. **Correspondance des tests** : chaque ligne de la table « Tests » se retrouve ; 13 + 2 `assert_tostr_raw`, brut −6, trois entiers, exhaustif (`1245183`, `16 * (n_int_parts + 1)`) dans `test_pico8_str` ; `test_pico8_value` garde exactement ses 11 autres fonctions de test (4 constructeurs, 5 égalités, chaîne, booléen).
5. **Casts** : aucun cast nouveau (ceux du corps déplacés à l'identique).
6. **Liens** : grep 7 ; `Makefile` : grep 11.
7. **Constantes** : grep 8 vide.
8. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste ((a1) 26, (a2) 1, (b) 0, (c) 0) ; deux lignes modifiées non retirées listées ; aucune ligne gagnée ; `score_panel.c:215-216` non jointe ; un seul include de `pico8_value.h` retiré (grep 9) ; aucun alias, renommage d'évitement, cale.
9. **Écarts déclarés** : `pico8_value_tostr` gardée et déléguant ; cas chaîne et booléen gardés ; TROU-13 ne suit pas ; TU existant ; deux prérequis du `Makefile` ; `planet_display.c:377` reste comptée ; empreinte 20 chemins et non ≈ 16.
10. **Preuve** : `gcov` (lignes modifiées exécutées ou justifiées ; `L` vide, lignes déplacées appariées), mutations restaurées par `cp` (M1–M6 : `test_pico8_value` vert), `sanitize-net` et balayage ASan+UBSan rapportés.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.15c ni au plan dans `src/` ou `tests/`.
12. **Docs** : TROU-22 et la ligne `tostr(v)` du glossaire réécrits ; note « Découvert à D.15c » en fin de ligne D.15c ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `667fd2f` (ou un descendant qui ne touche que `docs/`), propre ; ou si `valeur` n'y vaut pas 906 / 101 / 1025 occ., `type-tag` 243 / 58, résidu D 1147 / 110, sans explication.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change.
- `FAILED:` si un site ne tombe sous aucune règle de ce PRD ni de l'étude, ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si `make sanitize-net` ou le balayage ASan+UBSan des TU n'est pas propre.

**Périmètre et forme**

- `FAILED:` si `tools/` change (hors `float_guard.txt` réécrit à l'identique), si une ligne du `Makefile` autre que les prérequis de `test_pico8_value` et `test_pico8_str` change, si un `#include "pico8_value.h"` autre que `galaxy.c:12` change, ou si celui-ci reste (sous H-D15b-2) ; si le commit touche un chemin hors « Empreinte » sans le lister comme TU suiveur.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si le formatage existe en deux exemplaires, si `pico8_value_tostr` change de signature ou perd une branche, si `pi.c`, `research_display.c:57` ou `score_panel_display.c:196` changent.
- `FAILED:` si un témoin, un cas ou le balayage exhaustif manque dans `test_pico8_str`, ou si un cas chaîne ou booléen de `test_pico8_value` disparaît.
- `FAILED:` si un cast nouveau est ajouté.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne passe pas du départ à départ **− 27 lignes et − 54 occurrences exactement** (906 / 1025 → 879 / 971), sans explication chiffrée ligne à ligne ; si `type-tag` change.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes (`score_panel.c:215-216`), alias, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, cale, (b) ou (c) non nulle.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées ; la justification de chaque Δlignes > Δoccurrences ; le déplacement déclaré (`pico8_tostr_fixed`, module `str`) ; la table de correspondance des tests ; l'include retiré ; les deux prérequis du `Makefile` ; le rapport `gcov` ; les mutations ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne modifiée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` n'est pas vide.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification ; si, sous M1–M6, `test_pico8_value` rougit (un cas « nombre » y serait resté).
- `FAILED:` si le plafond est dépassé sans note (et sans repli c1/c2), ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.15c », ou si elle omet : les comptes réels, les lectures de la ligne, les trois appelants gardés de `pico8_value_tostr` et leurs étapes, `planet_display.c:377`, les deux prérequis du `Makefile`, l'include retiré.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- Les trois appelants à genres mêlés de `pico8_value_tostr` : `research_display.c:57` (`qe`, D.17), `pi.c:105` (B10, D.18c), `score_panel_display.c:196` (B11, D.18c, avec TROU-13 réécrit au site) ; le `pico8_value_string` de `planet_display.c:377` (B9, D.18b).
- La suppression de `pico8_value_tostr`, de ses cas chaîne et booléen, de `pico8_strdup_owned` et du module : D.19c.
- Le retrait de `malloc` des chaînes formatées (tampon fourni par l'appelant…) : hors plan 3 (`pico8_sub` garde le même contrat).
- `tools/line_coverage_reference.txt` (périmé), `tools/probe*` (le 14ᵉ témoin, TROU-22, reste non sourcé).
- Tout ce que D.15b fait (`ij`, `jc_ce`, `d_`, `hg`, `ie.id`, `pf/mz_value_vec`) ; les copies `truthy` (D.16) ; les includes d'en-têtes (D.19c) ; `type-tag` (D.20).
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **Nom et lieu** : `char *pico8_tostr_fixed(pico8_fixed_t)` dans `pico8_str.c/.h` (nom de l'étude), déplacement déclaré.
  - **`pico8_value_tostr`** : gardée, délègue ses nombres ; trois appelants vivants, chacun avec son étape.
  - **TU** : le TU existant `test_pico8_str` reçoit ; aucun TU nouveau ; cas chaîne et booléen gardés dans `test_pico8_value` (écart à « supprimés avec leur genre », leur genre vit encore).
  - **TROU** : TROU-22 suit, TROU-13 reste (écart à l'étude).
  - **Includes** : `galaxy.c:12` retiré (règle `.c` de D.15b) ; `pico8_str.h` ajouté à 11 `.c`.
  - **`Makefile`** : deux prérequis, rien d'autre.
  - **Drapeaux** : `sanitize-net` (ligne) et balayage ASan+UBSan des TU ; pas de `class-coverage-net`.
  - **Découpage** : un commit (≈ 15 min, plafond 30), repli c1/c2 écrit.
- **Surprises de mesure, pour `plan-pilot`** : (1) `pico8_value_tostr` a trois appelants de plus que les 20 de l'étude (genres mêlés) : elle ne peut pas partir, elle délègue ; (2) `planet_display.c:377` reste comptée (`pico8_value_string` autour) : 19 lignes de sites quittent le motif, pas 20 ; (3) `galaxy.c` se vide (H-D15b-2 tenue) : son include part ici ; (4) `collect.c` sort de `valeur` et du résidu D sans include à retirer (il passe par `pico8_collect.h`) ; (5) le `Makefile` doit changer (l'étude n'en disait rien) : `test_pico8_value` doit lier `pico8_str.c`, `test_pico8_str` doit lier `pico8_fixed.c` ; (6) l'étude dit « ≈ 16 fichiers » : 20 chemins avec `pico8_value.c/.h`, le `Makefile` et les docs ; (7) mesuré à `667fd2f` : − 27 lignes, − 54 occ., 906 → 879 contre ≈ 880 au plan ; (8) `make float-grep` lit les commentaires du cœur : le commentaire de contrat de `pico8_str.h` ne doit pas dire « double » ni « float » ; (9) aucun cas `REF` de `pico8_value_tostr` n'est testé aujourd'hui (TROU-13 n'est gardé que par `test_pico8_pj` et les e2e) : constat pour D.18c, rien à faire ici ; (10) révision après D.15b : ses hypothèses tiennent sauf H-D15b-1 (906 au lieu de 911 : aide `fold_value` de `test_pico8_qh` retirée), sans effet sur l'étape.
- **Pour D.17** : `research_display.c:57` (`pico8_value_tostr(qe)`, `qe` nil → `""`).
- **Pour D.18c** : `pi.c:105`, `score_panel_display.c:196` ; après eux, `pico8_value_tostr` n'a plus d'appelant dans `src/`.
- **Pour D.19c** : `test_pico8_value` ne teste plus que la couche ; `Makefile:407` (`src/pico8_str.c` dans ses prérequis) part avec la règle.

## Annexe A — textes recopiés (`docs/plan-3.md` à `667fd2f`, par extraction de lignes ; depuis `289c9a2`, seule la ligne D.15b a changé)

### Lignes D.15a à D.15c (l. 283-285)

> - D.15a — `valeur`, mécanique : `ov` prend des nombres, `cb` un entier. Compte : `valeur` ≈ 979. Découvert à D.15a : `valeur` 980 / 103 (1105 occ.), et non ≈ 979 : `ov` passe `pico8_value_string(px)` à `oj`, ligne qui part à D.16b avec `oj` ; `pico8_tech_gate_truthy` n'a plus que `oj` pour appelant (B12, D.16b) ; `pico8_economy.c` et `test_pico8_fm` sortent de `valeur` (restent au résidu D par `type-tag`) ; `nil()` de `test_pico8_js` retirée ; aucun include gagné. Résidu D 1221 / 111.
> - D.15b — `valeur`, mécanique : `ij`/`jc_ce` en pointeurs, `d_` en entier, `jc_hg` en `const char *`, `g_ie_id`. Compte : ≈ 910. Découvert à D.15b : un commit (`refactor(D.15b)`) ; `valeur` 980 / 103 (1105 occ.) → **906 / 101 (1025 occ.)**, et non 911 / 1030 : retirer les deux plis `fold_value(h, pico8_d_(…))` de `test_pico8_qh` y laisse `fold_value` sans appel, fatale sous `-Werror` ; elle part (5 lignes, 5 occ. de plus) ; `type-tag` 243 / 58 inchangé ; résidu D 1221 / 111 → 1147 / 110 ; prototypes publics qui nomment `pico8_value_t` : − 5. Lectures : `ij` et `jc_ce` en `pico8_struct_header_t` (et non `void **`) ; G4 en deux moitiés (le cas nombre de `hg` meurt, le cas nil reste : `has(NULL)` → 0, `add(NULL)` refusé, `hg_add_nil`) ; `pf/mz_value_vec` pris ; `fv` hors empreinte (`(void)pico8_d_`). Ligne gagnée : `probe.c:181` `et = pico8_value_ref(c);` (`bv`, `c` en pointeur), rendue par D.16b (B3). Copies `truthy` parties ici : `pico8_jc_truthy`, `pico8_mz_truthy`. Include retiré : `slipway_display.c` ; en-têtes vidés gardés pour D.19c : `entity_geom.h`, `probe.h`, `slipway.h`. Pour D.15c : `tostr` intacts, `galaxy.c:79`, `probe.c:273`, `research_icon.c:138`. Pour D.16 : copies `truthy` restantes 11 (`economy_score`, `economy_tiers`, `layout`, `planet`, `planet_display`, `probe`, `research`, `research_display`, `tech_gate`, `ui_button`, `wormhole_display`) ; `pico8_probe_truthy` n'a plus que `first` (`probe.c:196`).
> - D.15c — `valeur` : `tostr` déplacé dans `pico8_str.c` avec ses tests. `sanitize-net` : oui. Compte : ≈ 880.

Ligne D.15c identique à `289c9a2` (H-D15b-7 tenue).

### §6, extrait (l. 327)

> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.

### §7, extrait (« En D et en 2 »)

> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.

## Annexe B — cart et relevé

- `tostr` est une primitive de PICO-8, pas une fonction du cart : le port la tient pour un nombre depuis D2 (`docs/plan-2.md`, « État de D2 » : 1 245 183 entrées, 304 égalités exactes, 0 différence avec l'ancienne implantation `%.4f`).
- Bloc C de `tools/probe_out.txt` (`## c tostr`, l. 79) : 14 témoins, 13 reproduits ; le 14ᵉ (brut −6 : PICO-8 `"-0"`, port `"-0.0001"`) est TROU-22.
- Lecteurs dans le cart : chaque `..` d'un nombre dans une chaîne affichée (`"p"..id`, `"s"..ie.id`, crédits, scores, années, compteurs) ; tous passent aujourd'hui par l'un des 20 sites ou par `pi.c:105` (`io` d'un nombre).
