---
title: "Plan 3, étape D.15a : `pico8_ov` prend un nom de technologie et deux nombres et rend un nombre, `pico8_cb` prend un entier ; replis morts des six appelants et aide `nil()` de `test_pico8_js` retirés ; `valeur` 1014 → 980"
description: "`pico8_ov` et `pico8_cb` prennent encore des `pico8_value_t`, alors que le cart ne leur passe qu'un nom de technologie littéral, deux nombres littéraux et un vrai/faux. D.15a les type (`const char *`, `pico8_fixed_t`, `int`), fait suivre leurs six et deux appelants de `src/` et les TU `fm` et `js` dans le même commit, sans changer un comportement ni une constante figée : `valeur` passe de 1014 à 980 lignes, exact."
status: needs-triage
---

## Problem Statement

**Écrit sans humain** (`plan-pilot`, `to-prd`), sur l'arbre `a702ce4` (« docs: Q13 tranchée (A, nombre de contrôle retiré) ; étapes D.15 à D.20 reprises de l'étude D.14 »), descendant de `ce41e38` (D.13) et de `dfc057f` (étude D.14). Rien n'était en cours. Seuls `git log`, `grep`, `sed`, `awk`, `sh tools/data_residue.sh` et une simulation des éditions de ce PRD sur une copie de `src/` et `tests/` dans le scratchpad (script Python d'édition de texte, puis `data_residue.sh` et la recette sur la copie) ont tourné. **Rien n'a été compilé ni lancé.**

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c` disparaisse, donc que `valeur` tombe à 0. L'étude D.14 a classé en « (a) mécanique » les API dont chaque argument n'a qu'un genre. Les deux premières de sa liste d'étapes sont :

- **`pico8_ov(px, lk, mc)`** : le cart l'appelle six fois, toujours avec un nom de technologie littéral et deux nombres littéraux (`lua.lua:459`, `:642`, `:768`, `:772`, `:1204`, `:1234`) ; `ov` rend donc toujours un nombre. Le port l'appelle aux six mêmes endroits en emballant les littéraux (`pico8_value_string`, `pico8_value_fixed`), puis chaque appelant relit le résultat par un repli `kind == PICO8_VALUE_FIXED ? … : défaut` qui ne peut jamais prendre sa branche « défaut » ;
- **`pico8_cb(bi)`** : le cart l'appelle avec `true` (`cb(true)`, `lua.lua:956`) ou sans argument (`cb()`, `:982`), et `bi` n'est lu qu'en vérité (`or bi`, `:1461`). Le port passe `pico8_value_bool(1)` et `pico8_value_ref(NULL)`, et les TU `fm` et `js` passent `pico8_value_bool(1)` ou une aide locale `nil()`.

Tant que ces deux signatures prennent des valeurs, 35 lignes `valeur` (dans 9 fichiers) ne peuvent pas partir.

La ligne D.15a du plan, l'extrait de l'étude D.14 qui la fonde (critère, deux lignes de l'inventaire (a), règles communes et ligne D.15a de la table des étapes) sont recopiés tels quels, par extraction de lignes de `a702ce4`, en **Annexe A**. L'essentiel :

> - D.15a — `valeur`, mécanique : `ov` prend des nombres, `cb` un entier. Compte : `valeur` ≈ 979.

### État de départ (mesuré à `a702ce4`)

| Mesure | Valeur |
|---|---|
| `valeur` (`make data-residue`) | **1014 lignes / 105 fichiers / 1151 occ.** (`src/` 518 / 66 / 605 ; `tests/` 496 / 39 / 546) |
| `type-tag` | 243 / 58 |
| résidu D | 1255 lignes / 111 fichiers |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` |
| e2e | 26 dans `tests/e2e/` |

Recette (motif figé de `tools/data_residue.sh` ; `--detail` donne les lignes par fichier, la recette les occurrences) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
for d in "src tests" src tests; do F=$(find $d \( -name '*.c' -o -name '*.h' \) -type f | sort)
  printf '%s\t%s lignes\t%s fichiers\t%s occ.\n' "$d" "$(cat $F | grep -cE "$P")" \
    "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"; done
# par fichier : grep -cE "$P" <f> (lignes) ; grep -oE "$P" <f> | wc -l (occ.)
```

### Numéros de l'étude remesurés

L'étude a été mesurée à `b7f6c02`, avant D.13. D.13 a retiré `#include "pico8_ref_audit.h"` de 12 fichiers, dont trois de cette étape : **`tech_gate.c`, `slipway.c`, `menu.c` sont décalés de −1** ; `tech_gate.h`, `economy.c`, `planet.c`, `wormhole.c` et les TU ne le sont pas.

| Étude (`b7f6c02`) | `a702ce4` |
|---|---|
| `tech_gate.c:45` (`ov`), `:77`, `:106` (`cb`) | `:44`, `:76`, `:105` |
| `slipway.c:271,283-285` | `:270`, `:282-284` |
| `menu.c:177,283` | `:176`, `:282` |
| `tech_gate.h:15`, `:29` ; `economy.c:90-101,196-240` ; `planet.c:435-442` ; `wormhole.c:76-90,103-117` | inchangés |

### Inventaire (numéros de `a702ce4`)

Chaque ligne comptée qui part ou change, avec son remplacement. Classes : **(a)** supprimée sans remplaçant ; **(b)** signature, paramètre ou appel dont le type change ; **(c)** même donnée, autre nom (attendu 0). « occ. » : occurrences du motif sur la ligne.

#### `src/pico8_tech_gate.h` — 4 / 7 → **2 / 2**

| Ligne | Aujourd'hui | Après | Classe | occ. |
|---|---|---|---|---|
| `:12-14` | commentaire « Lua's `A and B or C`, three-way: `mc` comes back only when it is itself truthy […] Not a ternary on oj(px) alone. » | **supprimé** (décision « Commentaires ») | non compté | — |
| `:15` | `pico8_value_t pico8_ov(pico8_value_t px, pico8_value_t lk, pico8_value_t mc);` | `pico8_fixed_t pico8_ov(const char *px, pico8_fixed_t lk, pico8_fixed_t mc);` | (b) | 4 |
| `:29` | `pico8_cb_kept_t pico8_cb(pico8_value_t bi);` | `pico8_cb_kept_t pico8_cb(int bi);` | (b) | 1 |
| `:6`, `:10` | include, `pico8_oj(pico8_value_t px)` | **inchangés** (`oj` : D.16b) | — | — |

#### `src/pico8_tech_gate.c` — 9 / 12 → **7 / 7**

| Ligne | Aujourd'hui | Après | Classe | occ. |
|---|---|---|---|---|
| `:44` | `pico8_value_t pico8_ov(pico8_value_t px, pico8_value_t lk, pico8_value_t mc) {` | `pico8_fixed_t pico8_ov(const char *px, pico8_fixed_t lk, pico8_fixed_t mc) {` | (b) | 4 |
| `:45` | `if (pico8_oj(px) && pico8_tech_gate_truthy(mc)) {` (non comptée) | `if (pico8_oj(pico8_value_string(px))) {` | **gagne `valeur`** (+1 ligne, +1 occ.) | +1 |
| `:46-48` | `return mc;` / `}` / `return lk;` | inchangées | — | — |
| `:76` | `static int pico8_cb_predicate(const pico8_nb_entry_t *t, pico8_value_t bi) {` | `… , int bi) {` | (b) | 1 |
| `:102` | `return affordable \|\| pico8_tech_gate_truthy(bi);` (non comptée) | `return affordable \|\| bi;` | non comptée | — |
| `:105` | `pico8_cb_kept_t pico8_cb(pico8_value_t bi) {` | `pico8_cb_kept_t pico8_cb(int bi) {` | (b) | 1 |
| `:14-22`, `:27`, `:34`, `:98` | `pico8_tech_gate_truthy`, `pico8_oj`, `pico8_oj(pico8_value_string(t->px))` | **inchangés** : `truthy` garde son appelant `oj:31` (B12, D.16b) | — | — |

#### `src/pico8_economy.c` — 8 / 10 → **0 / 0** (le fichier sort de `valeur` ; il reste au résidu D par `type-tag`)

| Ligne | Aujourd'hui | Après | Classe | occ. |
|---|---|---|---|---|
| `:90` | `pico8_value_t kxv;` | supprimée | (a) | 1 |
| `:99-100` | `kxv = pico8_ov(pico8_value_string("time compression"), pico8_value_fixed(pico8_fixed_from_int(12)),` / `pico8_value_fixed(pico8_fixed_from_int(15)));` | `kx = pico8_ov("time compression", pico8_fixed_from_int(12), pico8_fixed_from_int(15));` | (b), (b) | 2 + 1 |
| `:101` | `kx = (kxv.kind == PICO8_VALUE_FIXED) ? kxv.as.fixed : pico8_fixed_from_int(12);` | supprimée (repli mort) | (a) | 1 |
| `:196` | `pico8_value_t skill;` | supprimée | (a) | 1 |
| `:238-239` | `skill = pico8_ov(pico8_value_string("skill implants"), pico8_value_fixed(pico8_fixed_from_int(1)),` / `pico8_value_fixed(PICO8_FIXED_LIT(0x00012666, 1.15)));` | `skill_mult = pico8_ov("skill implants", pico8_fixed_from_int(1), PICO8_FIXED_LIT(0x00012666, 1.15));` | (b), (b) | 2 + 1 |
| `:240` | `skill_mult = (skill.kind == PICO8_VALUE_FIXED) ? skill.as.fixed : pico8_fixed_from_int(1);` | supprimée (repli mort) | (a) | 1 |

#### `src/pico8_planet.c` — 17 / 18 → **13 / 13**

| Ligne | Aujourd'hui | Après | Classe | occ. |
|---|---|---|---|---|
| `:435` | `pico8_value_t fp_val;` | supprimée | (a) | 1 |
| `:440-441` | `fp_val = pico8_ov(pico8_value_string("autoassemblers"), pico8_value_fixed(pico8_fixed_from_int(3)),` / `pico8_value_fixed(pico8_fixed_from_int(2)));` | `fp = pico8_ov("autoassemblers", pico8_fixed_from_int(3), pico8_fixed_from_int(2));` | (b), (b) | 2 + 1 |
| `:442` | `if (fp_val.kind == PICO8_VALUE_FIXED) {` | supprimée (repli mort) | (a) | 1 |
| `:443-444` | `fp = fp_val.as.fixed;` / `}` (non comptées) | supprimées | non comptées | — |

`fp` est initialisé à 2 (`:410`), valeur que `ov` rendait déjà quand « autoassemblers » n'est pas inventé ; le `if` ne pouvait pas être faux.

#### `src/pico8_slipway.c` — 22 / 24 → **18 / 19**

| Ligne | Aujourd'hui | Après | Classe | occ. |
|---|---|---|---|---|
| `:270` | `pico8_value_t ov_result;` | supprimée | (a) | 1 |
| `:282-283` | `ov_result = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(PICO8_FIXED_LIT(0x00008000, 0.5)),` / `pico8_value_fixed(PICO8_FIXED_LIT(0x0001147a, 1.08)));` | `threshold = pico8_ov("space folding", PICO8_FIXED_LIT(0x00008000, 0.5), PICO8_FIXED_LIT(0x0001147a, 1.08));` | (b), (b) | 2 + 1 |
| `:284` | `threshold = (ov_result.kind == PICO8_VALUE_FIXED) ? ov_result.as.fixed : pico8_fixed_from_int(0);` | supprimée (repli mort) | (a) | 1 |

`pico8_jc_ce` garde son retour `pico8_value_t` (D.15b).

#### `src/pico8_wormhole.c` — 21 / 24 → **13 / 14**

| Ligne | Aujourd'hui | Après | Classe | occ. |
|---|---|---|---|---|
| `:76` | `pico8_value_t threshold_val;` (`mz:hq`) | supprimée | (a) | 1 |
| `:88-89` | `threshold_val = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(PICO8_FIXED_LIT(0x000030a3, 0.19)),` / `pico8_value_fixed(PICO8_FIXED_LIT(0x0000451e, 0.27)));` | `threshold = pico8_ov("space folding", PICO8_FIXED_LIT(0x000030a3, 0.19), PICO8_FIXED_LIT(0x0000451e, 0.27));` | (b), (b) | 2 + 1 |
| `:90` | `threshold = (threshold_val.kind == PICO8_VALUE_FIXED) ? threshold_val.as.fixed : pico8_fixed_from_int(0);` | supprimée (repli mort) | (a) | 1 |
| `:103` | `pico8_value_t threshold_val;` (`mz:kz`) | supprimée | (a) | 1 |
| `:115-116` | `threshold_val = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(pico8_fixed_from_int(29)),` / `pico8_value_fixed(pico8_fixed_from_int(35)));` | `threshold = pico8_ov("space folding", pico8_fixed_from_int(29), pico8_fixed_from_int(35));` | (b), (b) | 2 + 1 |
| `:117` | `threshold = (threshold_val.kind == …) ? … : pico8_fixed_from_int(0);` | supprimée (repli mort) | (a) | 1 |

`pico8_ij`, `pico8_mz_value_vec`, `pico8_mz_et` : inchangés (D.15b, D.16b).

#### `src/pico8_menu.c` — 27 / 32 → **25 / 30**

| Ligne | Aujourd'hui | Après | Classe | occ. |
|---|---|---|---|---|
| `:176` | `cb_list = pico8_cb(pico8_value_bool(1));` | `cb_list = pico8_cb(1);` | (b) | 1 |
| `:282` | `cb_list = pico8_cb(pico8_value_ref(NULL));` | `cb_list = pico8_cb(0);` | (b) | 1 |
| `:173-175` | commentaire « `true` is the ignore-cost flag […] The two sites do not merge. » | **inchangé** : il reste vrai (`1` est le drapeau, le piège « ne pas fusionner » tient) | — | — |

#### `tests/test_pico8_fm.c` — 2 / 2 → **0 / 0** (le TU sort de `valeur` ; il reste au résidu D par `type-tag`)

| Ligne | Aujourd'hui | Après | Classe | occ. |
|---|---|---|---|---|
| `:35` | `#include "../src/pico8_value.h"` | supprimée : le TU ne nomme plus rien du module (`pico8_fixed.h` reste inclus directement, `:24`) | (a) | 1 |
| `:88` | `(void)pico8_cb(pico8_value_bool(1));` (`cb_affordable`) | `(void)pico8_cb(1);` | (b) | 1 |

#### `tests/test_pico8_js.c` — 46 / 53 → **44 / 51**

| Ligne | Aujourd'hui | Après | Classe | occ. |
|---|---|---|---|---|
| `:581-584` | `static pico8_value_t nil(void) {` / `return pico8_value_ref(NULL);` / `}` / ligne vide | supprimées : ses trois seuls appels sont ceux de `cb` ; gardée, elle casserait `-Werror` (`unused-function`) | (a), (a) | 1 + 1 |
| `:907` (`test_hb`), `:1735`, `:1750` (`test_cb_writes_every_e_`) | `pico8_cb(nil())` (non comptées : l'aide cache le constructeur) | `pico8_cb(0)` | non comptées | — |
| `:46` | `#include "../src/pico8_value.h"` | **gardé** : 44 autres lignes nomment le module | — | — |

#### Ce qui ne bouge pas

- **Aucun autre fichier de `src/` ou `tests/`** : `git grep -n 'pico8_ov\|pico8_cb('` ne nomme que les 9 fichiers ci-dessus ; aucun TU n'appelle `pico8_ov` ; aucun TU ne plie un argument ni un retour de `ov` ou `cb` (G1 sans objet : aucun pli typé à écrire, aucune constante ne peut bouger).
- `pico8_oj`, `pico8_tech_gate_truthy`, `pico8_jc_ce`, `pico8_ij`, `pico8_mz_*`, `pico8_rn` : étapes suivantes.
- `Makefile`, `tools/`, `tests/e2e/`, tout `*.sha256`, `docs/trous-de-portage.md` (aucune entrée ne nomme `ov` ni `cb`).
- **Docs** : `docs/glossaire-noms.md` (`:186`, `:463`) et `docs/plan-3.md` changent (décisions plus bas).

## Solution

**Une sous-étape, un commit** (D.15a), vert seul, vérifié et poussé :

- `pico8_ov` : `pico8_fixed_t pico8_ov(const char *px, pico8_fixed_t lk, pico8_fixed_t mc)`, corps « `mc` si `oj(px)`, sinon `lk` » ; son commentaire d'en-tête, devenu faux, est supprimé ;
- `pico8_cb`, `pico8_cb_predicate` : `int bi`, lu en vérité C ;
- les six appelants de `ov` écrivent leurs littéraux nus et affectent le résultat directement à leur variable typée (`kx`, `skill_mult`, `fp`, `threshold` ×3) ; leur locale `pico8_value_t` et leur repli `kind == FIXED ? … : défaut` disparaissent ;
- les deux appelants de `cb` passent `1` et `0` ;
- TU : `fm` (appel, include), `js` (trois appels, aide `nil()` retirée) ;
- `glossaire-noms.md` : `ov`, `cb` ; `plan-3.md` : note « Découvert à D.15a » et compte `valeur` du bloc « Reste de D ».

**Sortie de l'étape**, constatée par commande sur le commit :

- `make test` vert, dont `make data-guard` ; `make e2e` : **26 conformes** ;
- `make data-residue` : **`valeur` 980 / 103**, exact (`src/` 488 / 65 / 563 occ. ; `tests/` 492 / 38 / 542 occ. ; 1105 occ. au total) ; `type-tag` 243 / 58 inchangé ; résidu D **1221 / 111** ;
- ces commandes rendent exactement ce qui est écrit (`<parent>` = le parent du commit) :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. ov : une déclaration, une définition, six appels, chacun avec ses littéraux du cart, dans l'ordre (lk, mc)
  git grep -n 'pico8_ov(' -- src tests | wc -l                     # 8
  grep -cF 'pico8_ov("time compression", pico8_fixed_from_int(12), pico8_fixed_from_int(15));' src/pico8_economy.c   # 1
  grep -cF 'pico8_ov("skill implants", pico8_fixed_from_int(1), PICO8_FIXED_LIT(0x00012666, 1.15));' src/pico8_economy.c   # 1
  grep -cF 'pico8_ov("autoassemblers", pico8_fixed_from_int(3), pico8_fixed_from_int(2));' src/pico8_planet.c   # 1
  grep -cF 'pico8_ov("space folding", PICO8_FIXED_LIT(0x00008000, 0.5), PICO8_FIXED_LIT(0x0001147a, 1.08));' src/pico8_slipway.c   # 1
  grep -cF 'pico8_ov("space folding", PICO8_FIXED_LIT(0x000030a3, 0.19), PICO8_FIXED_LIT(0x0000451e, 0.27));' src/pico8_wormhole.c   # 1
  grep -cF 'pico8_ov("space folding", pico8_fixed_from_int(29), pico8_fixed_from_int(35));' src/pico8_wormhole.c   # 1
  # 2. cb : huit lignes, toutes à argument entier
  git grep -nE 'pico8_cb\(' -- src tests | wc -l                   # 8
  git grep -nE 'pico8_cb\((0|1|int bi)\)' -- src tests | wc -l     # 8
  # 3. plus de locale ni de repli de ov ; plus d'aide nil() ; fm et economy sans valeur
  git grep -nE '\b(kxv|fp_val|ov_result|threshold_val)\b' -- src   # vide
  git grep -nE '\bnil\(' -- tests/test_pico8_js.c                  # vide
  git grep -nE "$P" -- src/pico8_economy.c tests/test_pico8_fm.c   # vide
  # 4. truthy de tech_gate : sa définition et son seul appel restant (oj)
  git grep -n 'pico8_tech_gate_truthy' -- src | wc -l              # 2
  # 5. une seule ligne ajoutée porte le motif
  git diff <parent> -- src tests | grep '^+[^+]' | grep -E "$P"    # une ligne : if (pico8_oj(pico8_value_string(px))) {
  # 6. périmètre : 11 chemins
  git diff --stat <parent> | tail -1                               # 11 files changed
  # 7. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep '^valeur' | grep -E 'tech_gate|economy\.c|planet\.c|slipway\.c|wormhole\.c|menu\.c|test_pico8_(fm|js)\.c'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que `pico8_ov` prenne un nom de technologie et deux nombres, afin que sa signature dise ce que le cart lui passe (six appels, toujours un littéral chaîne et deux littéraux nombre).
2. En tant que mainteneur du port, je veux que `pico8_ov` rende un `pico8_fixed_t`, afin que ses appelants n'aient plus à relire un genre qui ne varie jamais.
3. En tant que mainteneur du port, je veux que les six replis `kind == PICO8_VALUE_FIXED ? … : défaut` disparaissent avec leur locale, afin qu'il ne reste pas de branche morte qui fasse croire que `ov` peut rendre autre chose qu'un nombre.
4. En tant que mainteneur du port, je veux que chaque appel de `ov` garde exactement les littéraux du cart, dans l'ordre (`lk`, `mc`), afin qu'aucun seuil ni facteur de jeu ne change.
5. En tant que mainteneur du port, je veux que `pico8_cb` et son prédicat prennent un `int bi`, afin que « ignorer le coût » soit un drapeau C et non une valeur à genre libre.
6. En tant que mainteneur du port, je veux que l'écran des technologies passe `1` et le bouton `ix[2]` passe `0`, afin que les deux sites restent distincts, comme le dit le commentaire de `menu.c`.
7. En tant que mainteneur du port, je veux que `pico8_oj` et `pico8_tech_gate_truthy` restent tels quels, afin que D.15a ne déborde pas sur D.16b (B4, B12).
8. En tant que mainteneur du port, je veux que `ov` appelle `oj` par le même idiome que le prédicat de `cb` (`pico8_oj(pico8_value_string(…))`), afin qu'il n'y ait qu'une seule route vers `oj` et aucune aide nouvelle à retirer plus tard.
9. En tant que relecteur, je veux que la seule ligne qui gagne `valeur` soit déclarée, avec l'étape qui la retirera, afin que 980 au lieu de 979 ne passe pas pour un oubli.
10. En tant que mainteneur des TU, je veux que `test_pico8_fm` et `test_pico8_js` suivent dans le même commit, afin qu'aucun TU ne compile contre une signature disparue.
11. En tant que mainteneur des TU, je veux que l'aide `nil()` de `test_pico8_js` parte avec ses seuls appels, afin que `-Werror` ne casse pas et qu'aucune aide morte ne reste.
12. En tant que mainteneur des TU, je veux que `test_pico8_fm` perde son include de `pico8_value.h` dès qu'il ne nomme plus rien du module, afin que les includes inutiles partent au fil des étapes, comme le prévoit l'étude.
13. En tant que vérificateur, je veux qu'aucune constante figée (`*_HASH`, `identity[]`) ne change, afin que le comportement soit prouvé identique par les TU tels qu'ils sont.
14. En tant que vérificateur, je veux que `make e2e` reste à 26 conformes au bit près, sans `.sha256` ni fichier de `tests/e2e/` touché, afin que le filet juge et ne soit pas ajusté.
15. En tant que vérificateur, je veux un rapport `gcov` (lignes ajoutées ou modifiées exécutées ; `L` vide ou expliqué), afin de constater que chaque site typé est bien parcouru.
16. En tant que vérificateur, je veux des mutations ciblées sur `ov`, `cb` et leurs sites, restaurées par `cp`, afin de savoir que les TU rougiraient si une conversion se trompait de branche ou d'argument.
17. En tant que vérificateur, je veux un balayage ASan+UBSan des TU propre, afin qu'aucune lecture non initialisée n'apparaisse là où une locale a disparu.
18. En tant que relecteur, je veux la classification de chaque ligne retirée ou modifiée ((a)/(b)/(c)), avec les comptes par fichier en lignes et en occurrences, afin de vérifier qu'aucun compte ne baisse par tassement.
19. En tant que relecteur, je veux que la règle des liens tienne (aucun fichier de `DATA_ENGINE_SRCS` ne nomme un symbole défini hors de cette liste), afin que les binaires de données continuent de se lier.
20. En tant que lecteur du glossaire, je veux que `ov` et `cb` y soient décrits avec leurs nouveaux types, afin que « `mc` non faux » ne décrive plus un cas impossible.
21. En tant que `plan-pilot`, je veux une note « Découvert à D.15a » courte et le compte `valeur` du bloc « Reste de D » à jour, afin que D.15b parte des bons comptes.
22. En tant qu'humain, je veux qu'aucun test ne rejoue un `events.json`, que rien ne soit lent, et que l'appli raylib ne soit jamais lancée, afin que la vérification reste dans les règles du dépôt.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Les formes cibles viennent de l'étude D.14 (§1.2, lignes `pico8_ov` et `pico8_cb`), que ce PRD applique.

### Modules

- **Modifiés, `src/`** : `pico8_tech_gate.h`, `pico8_tech_gate.c` (API) ; `pico8_economy.c`, `pico8_planet.c`, `pico8_slipway.c`, `pico8_wormhole.c` (appelants de `ov`) ; `pico8_menu.c` (appelants de `cb`).
- **Modifiés, `tests/`** : `test_pico8_fm.c`, `test_pico8_js.c`.
- **Docs** : `docs/glossaire-noms.md`, `docs/plan-3.md`.
- **Inchangés** : tout le reste, dont le `Makefile`, `tools/`, `docs/trous-de-portage.md`, `pico8_value.c/.h`.

### `pico8_ov` (G)

- **Forme** (étude D.14, §1.2) : `pico8_fixed_t pico8_ov(const char *px, pico8_fixed_t lk, pico8_fixed_t mc)`.
- **Corps** : seule la ligne du test change, `if (pico8_oj(pico8_value_string(px))) {` ; `return mc;` / `return lk;` restent. Le test `pico8_tech_gate_truthy(mc)` part : `mc` est un nombre, et un nombre est toujours vrai en Lua (0 compris ; l'ancien `truthy` rendait déjà 1 pour tout `FIXED`). Comportement identique pour toute valeur de `mc`.
- **`oj` reste à valeur** jusqu'à D.16b (B4 : `il` y passe nil et `false`). `ov` l'appelle donc par `pico8_value_string(px)`, exactement comme le prédicat de `cb` le fait déjà (`tech_gate.c:98`). **Cette ligne gagne `valeur`** (+1 ligne, +1 occ.), d'où **980 et non 979** ; elle part à D.16b, quand `oj` prendra `const char *`.
  - Rejeté : une aide `static` « `oj` sur une chaîne » partagée par `oj`, `ov` et le prédicat de `cb`. Elle devancerait D.16b et ouvrirait une deuxième route vers la même logique ; c'est une cale typée ↔ valeur (interdite).
  - Rejeté : garder `pico8_value_t px`. La déclaration, la définition et les six appels garderaient une occurrence chacun.
  - Rejeté : réécrire le corps en `return … ? mc : lk;`. Hors besoin, et cela change des lignes non comptées sans raison.
- **`px` NULL** : aucun appelant ne le passe (six littéraux). Aucun `assert` n'est ajouté : ce n'est pas un piège qu'on réintroduirait.

### `pico8_cb` (G)

- `pico8_cb_kept_t pico8_cb(int bi)` ; `pico8_cb_predicate(const pico8_nb_entry_t *t, int bi)` ; `return affordable || bi;`. `bi` vaut 1 ou 0. L'ancien `truthy` rendait 1 pour `BOOL 1` et 0 pour `REF NULL`, ce sont les deux seuls genres passés. Comportement identique.
- `menu.c:176` → `pico8_cb(1)`, `:282` → `pico8_cb(0)`. Leur commentaire (`:173-175`) reste : il nomme le drapeau du cart (`true`) et le piège « les deux sites ne fusionnent pas », toujours vrais.
- Rejeté : un `enum` ou deux fonctions (`cb_all`, `cb_affordable`). Ce serait un renommage hors besoin, et le cart n'a qu'un `cb(bi)`.

### Appelants de `ov` (G)

- Chaque appelant **affecte le résultat directement à sa variable typée existante** (`kx`, `skill_mult`, `fp`, `threshold`). La locale `pico8_value_t` (`kxv`, `skill`, `fp_val`, `ov_result`, `threshold_val` ×2) et le repli `kind == FIXED ? … : défaut` sont supprimés : ils ne servaient qu'à relire la valeur, et leur branche « défaut » était morte (`ov` rendait `lk` ou `mc`, tous deux `FIXED`).
- **Littéraux** : ceux du port, recopiés à l'identique, dans le même ordre ; aucune macro, aucune constante nommée (greps 1 de la Sortie). Ils sont conformes au cart (`lua.lua:459`, `:642`, `:768`, `:772`, `:1204`, `:1234`).
- **Mise en page** : un appel typé qui tient sur une ligne de 120 colonnes au plus (la largeur la plus grande de ces fichiers est de 112 à 121) s'écrit sur une ligne ; les six y tiennent (101 à 111 colonnes). L'interdiction de joindre (anti-tassement) vise les lignes qui **gardent** une occurrence comptée. Ici, les deux lignes d'origine de chaque appel perdent toutes leurs occurrences, sont classées (b) une par une et comptées au message.
- Rejeté : garder une locale typée intermédiaire (`pico8_fixed_t kxv = pico8_ov(…)`). Ce serait un double sans lecteur.

### Includes (G)

- `test_pico8_fm.c:35` : retiré. Le TU ne nomme plus rien du module, et `pico8_fixed.h` reste inclus directement (`:24`).
- `test_pico8_js.c:46`, `pico8_tech_gate.h:6`, `pico8_slipway.c:21`, `pico8_menu.c:24` : gardés, puisque d'autres lignes nomment encore le module.
- Aucun include n'est ajouté. Si la compilation dément H1, l'include **direct** manquant est ajouté et déclaré avec l'erreur du compilateur.

### Commentaires (G)

- **`tech_gate.h:12-14` supprimé.** Il portait un piège (« `mc` falsy still yields `lk` […] Not a ternary on oj(px) alone ») que le type rend impossible : un `pico8_fixed_t` est toujours vrai. Il ne porte plus aucune des quatre choses de `CLAUDE.md`. Le sens de `ov` va au glossaire (nom minifié).
  - Rejeté : le réécrire en paraphrase (« `mc` si `px` est inventé, sinon `lk` »), qui répète le code et le glossaire.
- `tech_gate.h:27-28` (`cb` écrit `e_`), `:8-9` (`oj`), `tech_gate.c:73-75` (piège de `e_`) et `menu.c:173-175` restent vrais et ne changent pas.
- Aucun renvoi à D.15a ni au plan dans `src/` ou `tests/`.

### Les TU suivent (G)

- `test_pico8_fm.c:88` → `(void)pico8_cb(1);` ; include `:35` retiré.
- `test_pico8_js.c:907`, `:1735`, `:1750` → `pico8_cb(0)`. L'aide `nil()` (`:581-583` et la ligne vide `:584`) est retirée : ce sont ses trois seuls appels.
- **Aucune constante ne change** : aucun TU ne plie l'argument de `cb`. `test_hb` calcule `affordable` par son propre appel `pico8_cb(0)`, le miroir du site `menu.c:282`. `test_cb_writes_every_e_` lit `e_` et les entrées gardées. Rien ne plie `ov`.
- **Aucun pli typé à écrire** (G1 de l'étude sans objet ici), aucune aide de TU créée ni retypée. Seule `nil()` est retirée.
- Assertion nouvelle : admise seulement pour couvrir une ligne ajoutée non exécutée par les TU, ou pour faire rougir une mutation qui ne rougit rien (trou). Elle va dans un test gardé du module et est listée au message avec la ligne qu'elle couvre. Rien de lent, aucun fichier d'événements.

### Règle des liens (G)

Aucun fichier de `DATA_ENGINE_SRCS` ne doit nommer un symbole défini hors de cette liste. Les fichiers touchés de la liste (`tech_gate.c`, `economy.c`, `planet.c`, `slipway.c`, `wormhole.c`) ne gagnent aucune référence : `pico8_ov`, `pico8_oj`, `pico8_value_string`, `pico8_fixed_from_int` y étaient déjà nommés, et sont définis dans `tech_gate.c`, `value.c`, `fixed.c`, tous dans la liste. `menu.c` n'est pas dans la liste. Preuve : les sept binaires liés sur `DATA_ENGINE_SRCS` (`test_pico8_data_init`, `map_tables`, `jb_ol`, `planet_kj_dispatch`, `bg_kj`, `planet_gh_row_lq`, `ga_export_panel_by`) se lient dans `make test`.

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans changer le type est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs quand la ligne jointe garde une occurrence comptée ; alias (macro ou `typedef` qui renomme `pico8_value_t`, `pico8_value_string` ou `pico8_value_fixed`) ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours pour échapper au motif ; enveloppe ou cale valeur ↔ type (`pico8_ov_value`, `pico8_cb_value`, aide « `oj` sur chaîne », conversion `pico8_value_t` → `pico8_fixed_t` au site) ; cast nouveau, quel qu'il soit (aucun n'est nécessaire) ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; toute modification d'une constante de hachage, de `tests/e2e/`, d'un `*.sha256`, de `tools/`, du `Makefile`.
- **Classification exigée**, au message, pour chaque ligne `valeur` retirée ou modifiée (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`), **avec les numéros du parent remesurés** :
  - **(a) supprimée sans remplaçant** (locale devenue inutile, repli mort, include inutile, aide locale de TU) : attendu **15** (`src/` 12 : `economy.c:90,101,196,240`, `planet.c:435,442`, `slipway.c:270,284`, `wormhole.c:76,90,103,117` ; `tests/` 3 : `fm:35`, `js:581,582`) ;
  - **(b) signature, paramètre ou appel dont le type change** : attendu **20** (`src/` 19 : `tech_gate.h:15,29`, `tech_gate.c:44,76,105`, `economy.c:99,100,238,239`, `planet.c:440,441`, `slipway.c:282,283`, `wormhole.c:88,89,115,116`, `menu.c:176,282` ; `tests/` 1 : `fm:88`) ;
  - **(c) même donnée, autre nom** : attendu **0**.
- **Lignes qui gagnent une catégorie**, par fichier et par catégorie, « aucune » compris : attendu **une seule**, `valeur` à `tech_gate.c:45` (`pico8_oj(pico8_value_string(px))`, retirée à D.16b) ; `type-tag` et les huit catégories gardées : **aucune**.
- **Lignes comptées modifiées, non retirées** : attendu **aucune** (chaque ligne comptée touchée perd toutes ses occurrences).
- **Δlignes > Δoccurrences** : attendu **aucun**, pour aucun fichier (table ci-dessous).
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et **par fichier** du diff, en lignes (`--detail`) **et** en occurrences (recette).

| Fichier | `valeur` avant → après (lignes / occ.) | Δl / Δocc | (a) | (b) | gagnées |
|---|---|---|---|---|---|
| `src/pico8_tech_gate.h` | 4 / 7 → 2 / 2 | −2 / −5 | 0 | 2 | 0 |
| `src/pico8_tech_gate.c` | 9 / 12 → 7 / 7 | −2 / −5 | 0 | 3 | 1 (`:45`) |
| `src/pico8_economy.c` | 8 / 10 → **0 / 0** | −8 / −10 | 4 | 4 | 0 |
| `src/pico8_planet.c` | 17 / 18 → 13 / 13 | −4 / −5 | 2 | 2 | 0 |
| `src/pico8_slipway.c` | 22 / 24 → 18 / 19 | −4 / −5 | 2 | 2 | 0 |
| `src/pico8_wormhole.c` | 21 / 24 → 13 / 14 | −8 / −10 | 4 | 4 | 0 |
| `src/pico8_menu.c` | 27 / 32 → 25 / 30 | −2 / −2 | 0 | 2 | 0 |
| `tests/test_pico8_fm.c` | 2 / 2 → **0 / 0** | −2 / −2 | 1 | 1 | 0 |
| `tests/test_pico8_js.c` | 46 / 53 → 44 / 51 | −2 / −2 | 2 | 0 | 0 |
| **Total** | **1014 / 105 f. / 1151 → 980 / 103 f. / 1105** | **−34 / −46** | **15** | **20** | **1** |

`type-tag` : aucun des 9 fichiers ne change de compte (`economy.c` 3, `planet.c` 1, `slipway.c` 5, `wormhole.c` 1, `menu.c` 1, `fm` 2, `js` 10, `tech_gate.*` 0).

### Effet sur `make data-residue` (simulé sur une copie de `src/` et `tests/`, sans build)

| Catégorie | Départ | Après D.15a | Pourquoi |
|---|---|---|---|
| `valeur` | 1014 / 105 | **980 / 103, exact** (`src/` 488 / 65 ; `tests/` 492 / 38) | 35 lignes retirées, 1 gagnée ; `economy.c` et `test_pico8_fm.c` sortent de la catégorie |
| `type-tag` | 243 / 58 | **243 / 58, exact** | aucune ligne touchée |
| résidu D | 1255 / 111 | **1221 / 111, exact** | 35 lignes distinctes partent, 1 arrive ; aucun fichier ne sort (les deux qui quittent `valeur` gardent `type-tag`) |
| `DATA_RESIDUE_GUARD` | `…,liste,ref-audit` | inchangé | `valeur` ≠ 0 (D.19c) |

Simulation : les éditions de l'Inventaire, appliquées par script à une copie de `src/` et `tests/`, donnent exactement ces comptes, 1105 occurrences, et `--check spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` rend `0 ligne(s)`. Après édition, les numéros des appels typés sont `economy.c:98`, `:234`, `planet.c:439`, `slipway.c:281`, `wormhole.c:87`, `:111`, `menu.c:176`, `:282`, `tech_gate.c:44-45`, `:76`, `:102`, `:105`. Ceux de `tech_gate.h` dépendent du commentaire supprimé (`ov` en `:12`, `cb` en `:26`).

### Documentation (G)

- `docs/glossaire-noms.md:186` (`ov`) : « rend `mc` si `px` est débloqué et `mc` non faux, sinon `lk` » devient « rend `mc` si la technologie `px` est débloquée, sinon `lk` ; `lk` et `mc` sont toujours des nombres (`pico8_fixed_t`), `px` un nom de technologie ».
- `docs/glossaire-noms.md:463` (`cb`) : « `bi` vrai ignore le coût » devient « `bi` (entier) vrai ignore le coût ». Le reste de la ligne est inchangé.
- `docs/plan-3.md` : **note « Découvert à D.15a »** en fin de ligne D.15a, limitée à ce qui change une étape suivante (§7) : `valeur` 980 / 103 (et non ≈ 979 : la ligne `pico8_oj(pico8_value_string(px))` de `ov` part à D.16b avec `oj`) ; `pico8_tech_gate_truthy` n'a plus que `oj` pour appelant (B12, D.16b) ; `economy.c` et `test_pico8_fm` hors `valeur` ; `nil()` de `js` retirée ; tout include direct gagné, s'il y en a. **Bloc « Reste de D »** : « `valeur` (1014) » devient « (980) ». Rien d'autre ne change dans le plan : ni les lignes D.15b–D.20b, ni leurs comptes ≈.
- `docs/trous-de-portage.md` : inchangé (aucun écart en jeu ; TROU-3 reste sur `oj`).

### Ce qui ne change pas

- Le comportement : mêmes seuils, facteurs et durées d'année, mêmes technologies listées, mêmes `e_`, mêmes pixels ; `make e2e` conforme au bit près.
- Toute constante `*_HASH`/`identity[]` de TU ; aucun TU ni e2e ajouté ou supprimé.
- `pico8_oj`, `pico8_tech_gate_truthy`, `pico8_cb_kept_t`, `pico8_nb_e_`, `pico8_gi_nb` ; les API de D.15b et au-delà.
- `tools/float_guard.txt` : `make test` le réécrit à l'identique, puisqu'aucun `.c` n'est ajouté ni retiré. Un diff sur ce fichier est un échec à expliquer.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : sous `-std=c99 -Wall -Wextra -Werror`, le diff compile. Aucune locale ne reste sans usage (chaque locale `pico8_value_t` part avec son repli), `pico8_tech_gate_truthy` garde un appelant (`oj:31`), `nil()` part avec ses trois appels, et `test_pico8_fm` ne demande rien à `pico8_value.h` (`pico8_fixed.h` inclus directement). Si c'est démenti, l'include direct manquant est ajouté et déclaré avec l'erreur.
- **H2** : aucune constante de TU ne change. Les deux fonctions rendent exactement ce qu'elles rendaient pour les seuls arguments que `src/` et les TU leur passent (Décisions `ov`, `cb`).
- **H3** : les lignes ajoutées ou modifiées sont exécutées par les TU. `ov` l'est dans ses deux branches par `test_pico8_hy` (`test_nx` : seuil 12 sans « time compression », 15 avec), `hy:lj` par `test_lj`, `jo:hu` (branche « autoassemblers ») par les TU qui bâtissent une planète (`bg`, `jo`, `hy`…), `jc:ce` par `test_pico8_jc` (`hq_over`), `mz:hq`/`mz:kz` par `test_pico8_mz`, `cb` par `fm` et `js`, `menu.c:282` par `test_hb` (`js`). `menu.c:176` (écran des technologies) l'est par un TU qui ouvre `cr`, sinon par les e2e. Ce qui ne l'est ni par l'un ni par l'autre est justifié au message.
- **H4** : `L` (plus bas) est vide. Seul candidat : `tech_gate.c:19` (`return 0;` de la clause `REF NULL` de `truthy`), qu'un TU atteignait par `cb(nil())`. Il n'est plus atteint que par `oj(nil)` depuis `research_display.c:324` (projet sans prérequis). S'il sort de l'union des TU, il doit rester exécuté par les e2e, et il est expliqué au message.

## Implementation Chunks

Une sous-étape, dans cet ordre, verte seule, vérifiée et poussée :

1. **D0 — départ** : HEAD = `a702ce4` ou un descendant qui ne touche que `docs/`, arbre propre. Recette : `valeur` 1014 / 105 / 1151 occ. (`src/` 518 / 66 / 605), `type-tag` 243 / 58, résidu 1255 / 111 ; `git grep -n 'pico8_ov(\|pico8_cb('` ne nomme que les 9 fichiers de l'Inventaire. Sinon, remesurer et expliquer, avec arrêt si un appelant hors inventaire existe. `gcov` de départ (TU et e2e, plus bas), copié dans le scratchpad.
2. **API** : `tech_gate.h`, `tech_gate.c`.
3. **Appelants** : `economy.c`, `planet.c`, `slipway.c`, `wormhole.c`, `menu.c`.
4. **TU** : `fm`, `js`.
5. **Filets** : `make test`, puis `make e2e`.
6. **Preuves** : `gcov` d'arrivée et `L` ; mutations M1–M7 ; balayage ASan+UBSan des TU ; greps de la Sortie ; mesure et classification ; glossaire ; note du plan ; **commit**.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (seuils, durées d'année, technologies listées, `e_`, pixels), pas la présence d'une ligne. D.15a change deux signatures sans changer une valeur rendue. La preuve : les TU existants et leurs constantes inchangées, les 26 e2e au bit près, `gcov` et des mutations sur ce que `gcov` ne prouve pas (la branche prise, l'argument passé), et le balayage ASan+UBSan pour les locales retirées.

### TU touchés

- `test_pico8_fm` (appel de `cb`, include) et `test_pico8_js` (trois appels de `cb`, aide `nil()`). Aucun autre : aucun TU ne nomme `pico8_ov`.
- §6 : aucun test de régression touché (`layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by` ne nomment ni `ov` ni `cb`).
- **Prior art** : TU suiveurs dans le même commit (D.10b, D.11c, D.12a–c) ; aide locale de TU retirée avec son dernier appel sous `-Werror` (D.12c) ; replis morts retirés avec le type qui les rendait possibles (1b, lectures emballées).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`**, au départ puis à l'arrivée :

- **TU** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`) et lancés comme le `Makefile` les lance (`ASAN_OPTIONS=detect_leaks=0` pour `test_pico8_planet_gh_row_lq`, lié sous ASan) ; union des lignes de `src/` exécutées.
- **e2e** : `make line-coverage EVENTS="$(ls tests/e2e/*.json)"` → `tmp/line_coverage.txt`, copié dans le scratchpad.
- Chaque ligne **ajoutée ou modifiée** de `src/` (numéros d'arrivée de l'Inventaire) est exécutée par l'union des TU, sinon par les e2e, sinon couverte par une assertion ajoutée, sinon justifiée au message.
- `L` = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée. Elles sont appariées par contenu et par fonction, puisque les numéros de `economy.c`, `planet.c`, `slipway.c`, `wormhole.c` se décalent. Les lignes retirées (locales, replis, test `truthy(mc)`) ne sont pas survivantes. **Attendu : vide sur les e2e ; vide ou `{tech_gate.c:19}` sur les TU, expliqué (H4).** Toute autre ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail. Le fichier est sauvegardé par `cp` avant et restauré par `cp` après (jamais `git checkout`/`stash`), puis `touch` pour forcer la recompilation. Chaque TU est lancé exactement comme le `Makefile` le lance (binaire `tmp/test_pico8_<nom>`, `ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan). Chaque mutation doit rougir au moins le TU nommé ; sinon c'est un trou, couvert par une assertion ajoutée ou justifié (mutation équivalente, démontrée).

**Astuces** :
- supprimer un appel, un paramètre ou une entrée peut échouer sous `-Werror` (fonction `static` inutilisée, paramètre inutilisé de `-Wextra`) : neutraliser par `0 ? f(…) : …`, `0 && …`, `1 || …`, ou `(void)bi;` plutôt que supprimer ;
- si un `assert` de `src/` arrête le TU avant l'assertion visée (arrêt sur `SIGABRT` au lieu de la constante), le relancer une fois avec cet `assert` relâché dans la copie mutée, et rapporter les deux résultats.

| # | Mutation (cible) | Attendu rouge |
|---|---|---|
| **M1** | `ov` rend toujours `lk` : `if (0 && pico8_oj(pico8_value_string(px)))` (`tech_gate.c:45`) | `test_pico8_hy` (`test_nx` : `lo` 15 avec « time compression ») |
| **M2** | `ov` rend toujours `mc` : `if (1 \|\| pico8_oj(…))` | `test_pico8_hy` (`test_nx` : l'année tourne après 12 sans la technologie) |
| **M3** | `cb` ignore `bi` : `return affordable \|\| (0 && bi);` (`tech_gate.c:102`) | un TU qui ouvre l'écran des technologies (`cr`), sinon `make e2e` ; sinon trou, couvert par une assertion ajoutée à `test_cb_writes_every_e_` : avec `jk` = 12, `pico8_cb(1)` garde au moins une entrée dont `e_` vaut 0 |
| **M4** | `cb` lit `bi` toujours vrai : `return affordable \|\| (1 \|\| bi);` | `test_pico8_js` (`test_hb`, `HB_HASH`, si l'état de départ tient une technologie de l'ère non finançable) ; sinon trou, couvert par une assertion ajoutée à `test_cb_writes_every_e_` : chaque entrée gardée par `pico8_cb(0)` a `e_` = 1 (le test et le site appellent le même `cb` muté, d'où le risque de trou) |
| **M5** | `menu.c:176` passe `0` | comme M3 |
| **M6** | `menu.c:282` passe `1` | `test_pico8_js` (`test_hb`, `HB_HASH`), sinon `make e2e`, sinon assertion ajoutée (trou) |
| **M7** | un site de `ov` avec `lk` et `mc` intervertis : `economy.c:98` (`15`, `12`) | `test_pico8_hy` (`test_nx`) |

Les cinq autres sites de `ov` ne reçoivent pas de mutation propre (le budget suit la ligne de l'étape). Leurs littéraux sont prouvés par les six greps `-F` de la Sortie, et leur exécution par `gcov`. Rejeté : une mutation par site (six `make` ciblés de plus, sans rien prouver que ces greps ne prouvent déjà).

### Filets

- `make test`, `make e2e` (26 conformes).
- **Balayage ASan+UBSan des TU** : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0` (TROU-7 : les entités ne sont jamais libérées), comme à D.12a–c. Critère : sortie propre de chaque TU.
- `make sanitize-net`, `make class-coverage-net` : non, selon l'étude (aucune durée de vie, aucun propriétaire, aucun constructeur ne change). `make raylib` : non exigé.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 min de temps réel**, D0, filets et preuves compris ; **plafond 35 min**. C'est une étape de 9 fichiers de code, et D.13 (18 chemins, sans `gcov` de TU ni mutation hors garde) a pris quelques minutes. Au-delà du plafond, rien n'est commis et l'implémenteur note ce qui manque. La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| recette, `make -s data-residue`, greps | < 1 min |
| `gcov` de tous les TU, départ et arrivée | quelques minutes chacun |
| `make line-coverage EVENTS=…` (26 e2e), départ et arrivée | < 1 min chacun |
| éditions (9 fichiers, glossaire) | ~5 min |
| `make test` | de l'ordre de la minute (`tech_gate.h` change : recompilation de ses includers) |
| `make e2e` | ~1 s |
| M1–M7 (un `make tmp/test_pico8_<nom>` ciblé et un lancement chacune) | < 5 min |
| balayage ASan+UBSan des TU | quelques minutes |
| mesure, classification, message, note | ~5 min |

**Jamais l'appli raylib.** Rien de lent : ni `minimize`, ni `e2e-gap`, ni rejeu dans un TU.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | `a702ce4`, avant D.15a | 0 | `valeur 1014 105` ; résidu 1255 / 111 | … |
| 2 | recette `valeur` | idem | 0 | `src tests 1014 lignes 105 fichiers 1151 occ.` | … |
| 3 | `gcov` des TU (copie jetable) | idem | 0 | N lignes de `src/` exécutées | … |
| 4 | `make line-coverage EVENTS="…26…"` | idem | 0 | `couverture de lignes: N ligne(s)` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les 11 chemins (`src/pico8_tech_gate.h`, `src/pico8_tech_gate.c`, `src/pico8_economy.c`, `src/pico8_planet.c`, `src/pico8_slipway.c`, `src/pico8_wormhole.c`, `src/pico8_menu.c`, `tests/test_pico8_fm.c`, `tests/test_pico8_js.c`, `docs/glossaire-noms.md`, `docs/plan-3.md`). S'y ajoute seulement un include direct prouvé nécessaire, dans l'un de ces fichiers. Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`.
2. **Greps** 1 à 7 de la « Sortie », exactement.
3. **Formes** : `pico8_ov(const char *, pico8_fixed_t, pico8_fixed_t)` rend `pico8_fixed_t` ; `pico8_cb(int)` et son prédicat `int bi` ; aucun repli ni locale `pico8_value_t` autour d'un appel de `ov` ; littéraux du cart dans l'ordre (`lk`, `mc`).
4. **Casts et cales** : aucun cast nouveau ; aucune aide, macro, `typedef` ou enveloppe nouvelle ; `ov` appelle `oj` par `pico8_value_string(px)` et rien d'autre.
5. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier (table) ; classification présente et juste ((a) 15, (b) 20, (c) 0, numéros du parent) ; une seule ligne gagnée (`tech_gate.c:45`) ; aucune ligne comptée modifiée non retirée ; aucun Δlignes > Δocc.
6. **Liens** : les sept binaires `DATA_ENGINE_SRCS` se lient (`make test`).
7. **Preuve** : `gcov` (lignes ajoutées ou modifiées exécutées ou justifiées ; `L` vide ou `{tech_gate.c:19}` expliqué) ; M1–M7 rapportées, restaurées par `cp` ; balayage ASan+UBSan propre ; 26 e2e conformes.
8. **Commentaires** : `tech_gate.h:12-14` supprimé, rien d'autre ajouté ni reformulé dans `src/`/`tests/` ; aucun renvoi à D.15a ni au plan.
9. **Docs** : glossaire `ov` et `cb` réécrits comme dit ; note « Découvert à D.15a » courte ; « `valeur` (980) » dans « Reste de D » ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `a702ce4` ou un descendant qui ne touche que `docs/`, propre ; si `valeur` n'y vaut pas 1014 / 105 / 1151 occ. sans explication ; si un appelant de `pico8_ov` ou `pico8_cb` existe hors des 9 fichiers de l'Inventaire sans être traité et listé.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change.
- `FAILED:` si un site ne tombe sous aucune décision de ce PRD ni du §5 (arrêt « champ sans règle ») ; si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si le balayage ASan+UBSan des TU n'est pas propre.

**Périmètre et forme**

- `FAILED:` si un chemin hors des 11 est touché (hors include direct prouvé nécessaire) ; si `tools/`, le `Makefile` ou `tools/float_guard.txt` change.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si `pico8_oj`, `pico8_tech_gate_truthy`, `pico8_jc_ce` ou une autre API hors de l'étape change de signature ; si un littéral d'un appel de `ov` change ou change de place.
- `FAILED:` si un cast est ajouté, ou si une aide, une macro, un `typedef` ou une enveloppe apparaît entre valeur et type.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne vaut pas **exactement 980 / 103** (1105 occ.), si `type-tag` ne vaut pas 243 / 58, ou si le résidu D ne vaut pas 1221 / 111, sans explication (un include direct gagné l'explique).
- `FAILED:` **si un compte baisse par tassement** : lignes jointes qui gardent une occurrence, alias, renommage d'un identifiant vivant pour échapper au motif, cale valeur ↔ type, commentaire compté réécrit hors de son sujet ; (c) non nulle ; une ligne gagnée autre que `tech_gate.c:45`.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a)/(b)/(c) avec numéros du parent ; les lignes qui gagnent une catégorie (la ligne de `ov`, et « aucune » pour les autres catégories) ; les lignes comptées modifiées non retirées (« aucune ») ; Δlignes/Δoccurrences par fichier ; les includes retirés, gardés ou ajoutés ; l'aide de TU retirée ; le rapport `gcov` (`L`, lignes ajoutées) ; les mutations ; le balayage ASan+UBSan. Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne ajoutée ou modifiée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` contient autre chose que `tech_gate.c:19` expliqué, ou si `L` des e2e n'est pas vide.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans assertion ajoutée ni justification.
- `FAILED:` si le plafond est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.15a », si le bloc « Reste de D » ne dit pas `valeur` (980), ou si le plan change ailleurs.
- `FAILED:` si un commentaire est ajouté ou reformulé dans `src/`/`tests/` (seule la suppression de `tech_gate.h:12-14` est attendue), ou si un renvoi à D.15a ou au plan apparaît dans `src/`, `tests/` ou le `Makefile`.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- `pico8_oj` et son argument, `pico8_hp_il`, les copies `*_truthy` dont `pico8_tech_gate_truthy` : D.16b (B4, B12).
- `pico8_ij`, `pico8_jc_ce`, `pico8_jc_value_instance`, `pico8_d_`, `pico8_jc_hg_add/has`, `pico8_pf/mz_value_vec`, `g_ie_id` : D.15b.
- `pico8_value_tostr(pico8_value_fixed(x))` (dont les six de `menu.c`) : D.15c.
- `pico8_rn` (dont `planet.c:450,452`, `slipway.c:125,194`, `wormhole.c:64`) : D.16b.
- Les includes de `pico8_value.h` encore utiles, les aides de TU autres que `nil()` de `js`, `test_pico8_value` : D.19.
- `type-tag` : D.20.
- `make sanitize-net`, `make class-coverage-net`, `make raylib`, l'appli raylib, l'audio, le rendu, tout e2e.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Formes** : celles de l'étude (`const char *px`, `pico8_fixed_t lk, mc`, rendu `pico8_fixed_t` ; `int bi`) ; `ov` garde son `if`, seul le test change.
  - **`oj`** : appelé par `pico8_value_string(px)`, idiome déjà présent au prédicat de `cb` ; +1 ligne `valeur` déclarée, retirée à D.16b ; aide `static` partagée rejetée (cale, devance D.16b).
  - **Appelants** : affectation directe à la variable typée ; locales et replis morts retirés ; littéraux inchangés ; appel sur une ligne quand il tient en 120 colonnes, la règle anti-jointure visant les lignes qui gardent une occurrence.
  - **TU** : suivent dans le commit ; `nil()` de `js` retirée ; include de `fm` retiré ; aucune constante, aucun pli.
  - **Commentaire** de `ov` supprimé (piège rendu impossible par le type), sens au glossaire ; `menu.c:173-175` gardé.
  - **Plan** : note courte, « Reste de D » à 980.
  - **Preuve** : `gcov` TU + e2e, sept mutations (deux par fonction, deux sites de `cb`, un site de `ov`), greps `-F` des six appels ; balayage ASan+UBSan ; ni `sanitize-net` ni `class-coverage-net`.
  - **Découpage** : un commit, 11 chemins, ≈ 20 min, sans repli (l'étape est déjà la plus petite de la liste).
- **Surprises de mesure, pour `plan-pilot`** :
  1. **Le compte d'arrivée est 980, pas ≈ 979.** `oj` reste à valeur jusqu'à D.16b, donc `ov` doit lui passer `pico8_value_string(px)`, une ligne `valeur` gagnée. La ligne D.15a et la table de l'étude ne la comptaient pas.
  2. **Les trois appels de `cb` de `test_pico8_js` ne sont pas comptés** : l'aide `nil()` cache le constructeur. `js` ne baisse donc que de 2 lignes, celles de l'aide, que `-Werror` oblige à retirer.
  3. **`pico8_economy.c` et `test_pico8_fm.c` sortent de `valeur`** (105 → 103 fichiers) sans sortir du résidu D, qu'ils gardent par `type-tag`.
  4. **Les six replis des appelants étaient morts** : `ov` rendait `lk` ou `mc`, tous deux `FIXED`. Leurs défauts (12, 1, 0, 0, 0, et « garder `fp` ») n'étaient jamais pris.
  5. **Le commentaire de `ov` et la ligne du glossaire décrivent un cas (« `mc` faux ») que le type rend impossible** : le commentaire est supprimé, la ligne du glossaire réécrite.
  6. **Aucun TU n'appelle `pico8_ov`** ; sa seule route de TU est `test_pico8_hy` (`test_nx`, les deux branches).
  7. Les numéros de l'étude sont décalés de −1 dans `tech_gate.c`, `slipway.c` et `menu.c` (includes retirés par D.13), comme l'étude l'annonçait. Les autres sont exacts.
  8. L'empreinte réelle est de 9 fichiers de code et 2 docs (l'étude disait « ≈ 8 fichiers ») : `pico8_tech_gate.h` et `.c` comptent pour deux.
- **Pour D.15b** : `pico8_jc_ce` rend toujours `pico8_value_t` (`slipway.c:265`) ; `pico8_mz_value_vec` et `pico8_ij` inchangés dans `wormhole.c` ; départ `valeur` 980 / 103.

## Annexe A — textes recopiés (par extraction de lignes à `a702ce4`)

### `docs/plan-3.md`, ligne D.15a (l. 283)

- D.15a — `valeur`, mécanique : `ov` prend des nombres, `cb` un entier. Compte : `valeur` ≈ 979.

### `docs/plan-3.md`, ligne D.15b (l. 284), contexte

- D.15b — `valeur`, mécanique : `ij`/`jc_ce` en pointeurs, `d_` en entier, `jc_hg` en `const char *`, `g_ie_id`. Compte : ≈ 910.

### Étude D.14 (`docs/prd/done/plan3-phaseD-etape14-etude-genres-meles.md`), critère (l. 11)

**Critère (a) / (b).** Une valeur est **(a) mécanique** si elle ne prend qu'un seul genre non nil, et qu'aucun lecteur ne distingue nil de `false` : elle devient son type C, avec `NULL` ou un drapeau `has_x` pour nil (la règle de 1b). Elle est **(b) à genres mêlés** si elle prend deux genres non nil (nombre et `false`, chaîne et nombre…), ou si un lecteur distingue nil de `false`. Les 13 copies privées de `*_truthy(pico8_value_t)` sont classées (b) : elles n'existent que parce que leur argument mêle les genres, et meurent avec lui.

### Étude D.14, §1.2, lignes `ov` et `cb` (l. 34-35)

| `pico8_ov(px, lk, mc)` — `tech_gate.c:45`, `.h:15` ; appelants `economy.c:90-101,196-240`, `planet.c:435-442`, `slipway.c:271,283-285`, `wormhole.c:76-90,103-117` | `px` toujours un littéral chaîne, `lk`/`mc` toujours des littéraux nombre (lua.lua:1467, `ov("space folding",0.5,1.08)`…) ; rend donc un nombre | `pico8_fixed_t pico8_ov(const char *px, pico8_fixed_t lk, pico8_fixed_t mc)` ; les replis `kind == FIXED ? … : défaut` des appelants meurent |
| `pico8_cb(bi)` — `tech_gate.c:77,106`, `.h:29` ; `menu.c:177,283` | `true` ou nil (`cb(true)`, `cb()`), lu en vérité seule (lua.lua:1455-1461) | `int bi` |

### Étude D.14, §3, règles communes (l. 146-150)

Règles communes, pour chaque PRD :
- **Un TU qui nomme un symbole changé le suit dans le même commit.** Un TU peut être converti plus tôt que son symbole ; jamais plus tard.
- **Aucune empreinte figée ne change** (G1). Pendant D.15–D.18, un TU qui plie une valeur désormais typée reproduit les octets de l'ancien pli (octet de genre + contenu) à partir du type C.
- Aucun `*.sha256` ni e2e ne change.
- `class-coverage-net` si l'étape touche un constructeur ; `sanitize-net` si elle change une durée de vie ou un propriétaire.

### Étude D.14, §3, ligne D.15a de la table des étapes (l. 154)

| **D.15a** | `pico8_ov` en nombres, `pico8_cb(int)` ; appelants `economy`, `planet`, `slipway`, `wormhole`, `menu` ; TU `fm`, `js` | ≈ 8 fichiers | `valeur` ≈ 979 |

### Étude D.14, §4, G1 (l. 179)

- **G1. Empreintes figées.** Aucune constante figée ne change dans D.15–D.20. Un pli typé écrit le même octet de genre (`0`/`1`/`2`/`3`, les valeurs de l'actuel `pico8_value_kind_t`, ou les octets propres au TU : `0xB0`, `0xF1`, `0x57`) et le même contenu. Une empreinte qui bouge est une régression, pas une mise à jour.

### Cart (`pico8/lua.lua`), `cb`, `oj`, `ov` (l. 1455-1470) et les six appels de `ov`

```lua
function cb(bi)
return jp(nb,function(t)
t.e_=gi(t)<=fm.jk
return t.lv==fm.ew
and not oj(t.px)
and (t.e_
or bi)
end)
end
function oj(px)
return not px or gu(fm.invented,px)
end
function ov(px,lk,mc)
return oj(px)
and mc or lk
end
```

`:459` `fp=ov("autoassemblers",3,2)` ; `:642` `ov("space folding",0.5,1.08)` ; `:768` `ov("space folding",0.19,0.27)` ; `:772` `ov("space folding",29,35)` ; `:1204` `ov("time compression",12,15)` ; `:1234` `ov("skill implants",1,1.15)` ; appels de `cb` : `:956` `cb(true)`, `:982` `cb()`.
