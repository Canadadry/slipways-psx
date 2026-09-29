---
title: "Plan 3, étape D.19a/b : les TU plient sans `pico8_value_t` — chaque `fold_value` devient un pli typé par genre aux mêmes octets, les mots de genre nus deviennent une énumération figée locale, les fixtures (`vec_ref`, `mouse_at`, `mp`, `ra_only_t`, `fake_toast_t`) passent en pointeurs et entiers ; aucune constante ne change, aucun fichier de `src/`"
description: "Après D.18c, `src/` n'appelle plus rien du module valeur, mais 28 TU le nomment encore (318 lignes) pour plier leurs empreintes et bâtir leurs fixtures ; D.19a/b les réécrit sur les types C en trois sous-étapes (a : famille planète, 12 TU ; b1 : TU à mots de genre, 9 ; b2 : le reste, 7), constantes figées identiques, includes retirés avec le dernier pli : `valeur` ≈ 435 → 117."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.18c est implémentée** (`plan-pilot`, étape 2b), sur l'arbre commis **`712f68d`** (HEAD de `claude/plan-pilot-3-7zdft0` au départ de D.18c : graduation de D.18b plus le PRD de D.18c mis en file, `docs/` seul), extrait par `git archive 712f68d` dans le scratchpad. Seuls `grep`, `sed`, `awk` et `sh tools/data_residue.sh` ont tourné ; **rien n'a été compilé ni lancé**. Tout ce que ce PRD suppose de l'issue de D.18c est une hypothèse `H-D18c-n` (plus bas), à vérifier à la graduation de D.18c avant la mise en file. **Numéros de ligne : ceux de `712f68d`** ; D.18c ne touche aucun TU (sauf peut-être `test_pico8_pj` après `:744`, H-D18c-2), et les trois sous-étapes de ce PRD touchent des TU disjoints : ce sont donc les numéros du parent de chaque commit. L'implémenteur remesure par contenu et explique tout écart.

Mesuré à `712f68d` (départ de D.18c) : `valeur` 494 / 78 ; 547 occ. (`src/` 140 / 40 / 165, **`tests/` 354 / 38 / 382**), `type-tag` 241 / 57, résidu D 733 / 99. **Attendu au départ de D.19a** (H-D18c-1) : `valeur` **435 / 75 ; 479 occ.** (`src/` 81 / 37 / 97, `tests/` 354 / 38 / 382), `type-tag` **241 / 57**, résidu D **674 / 97**. La prévision `tests/` du PRD de D.18c (354 / 38 / 382, groupe D.19a 138, groupe D.19b 189 / 192 occ., `test_pico8_value` 27 / 52) est **exacte** à `712f68d`, TU par TU.

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c/.h` disparaissent (D.19c). Après D.18c, `src/` n'appelle plus le module ; mais 28 TU le nomment encore, pour trois raisons :

- **plis d'empreinte** : une fonction `fold_value(h, pico8_value_t v[, depth])` par TU, appelée avec `pico8_value_fixed(x)`, `pico8_value_bool(b)`, `pico8_value_string(s)`, `pico8_value_ref(p)` — on fabrique une valeur Lua pour la défaire aussitôt ; et des **mots de genre nus** `(uint32_t)PICO8_VALUE_REF` etc., écrits dans l'empreinte par les aides de membre depuis D.11d ;
- **fixtures** : `vec_ref` (un vecteur alloué rendu comme `pico8_value_ref`, dont chaque appelant lit `.as.ref`), `mouse_at`, les locales `pico8_value_t mp`, les paramètres `hq_over(…, pico8_value_t mp)`, deux fausses entités de TU (`ra_only_t` de `mo`, `fake_toast_t` de `iv`) dont un membre est une valeur ;
- **includes** : `#include "../src/pico8_value.h"` dans 37 TU (dont 9 qui n'en nomment rien d'autre).

Ligne de l'étape, recopiée de `docs/plan-3.md` à `712f68d` (§5, l. 292) :

> - D.19a/b — TU seulement : empreintes et aides locales en plis typés, constantes inchangées. Compte : ≈ 115.

Lignes de l'étude D.14 (§3), recopiées :

> | **D.19a** | TU seuls : aides d'empreinte et aides locales (`fold_value`, `fold_cell`, `fold_kind`, `vec_ref`, `nil()`, `mouse_at`…) en plis typés aux mêmes octets, TU de la famille planète (`fr`, `fv`, `g_`, `ga`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`) | ≈ 12 fichiers | ≈ 260 |
> | **D.19b** | idem, autres TU (`bg`, `hp`, `hy`, `ie`, `iv`, `jc`, `js`, `lq`, `mo`, `mz`, `pj`, `trade`, `ui`…) | ≈ 20 fichiers | ≈ 115 |

Lignes directrices de l'étude (§4), en extrait : **G1** « Aucune constante figée ne change dans D.15–D.20. Un pli typé écrit le même octet de genre (`0`/`1`/`2`/`3`, les valeurs de l'actuel `pico8_value_kind_t`, ou les octets propres au TU : `0xB0`, `0xF1`, `0x57`) et le même contenu. Une empreinte qui bouge est une régression, pas une mise à jour. » ; **G2** nil et `false` confondus seulement là où aucun lecteur ne les sépare ; **G4** un cas de TU d'un genre que `src/` ne produit pas est supprimé avec le type qui le permettait.

Legs reçus, relus ici :

- **D.15b** : « Include retiré : `slipway_display.c` ; en-têtes vidés gardés pour D.19c ». **D.18b** : « include `pico8_value.h` de `menu.c` […] retirés ». **D.12b2** (TU, catégorie `liste`) : « 13 TU perdent `#include "pico8_list.h"` » avec leur dernier usage. C'est la règle des includes discutée plus bas.
- **D.16a** : « Pour D.19a : `fold_value` déjà absente de `mx`, `os`, `pm`, `hj`, `nl` (et `qh` depuis D.15b), `fold_ref` absente de `pm`. » — confirmé à `712f68d`.
- **D.17** : « Pour D.19b : `test_pico8_trade` (`fold_qe` des membres, `fold_ref`, octets `PICO8_VALUE_*`) et `hp` (`fold_qe` écrit en octets 0 / 3). »
- **D.18a** : « Pour D.19b : `test_pico8_lq` plie encore par `fold_value` (`fold_cell`, `fold_tuple`, `cell_value_kind`), 30 lignes ; `test_pico8_js` par son `fold_value`, 39. » — 28 et 16 après D.18b.
- **D.18b** : « Pour D.19b : `test_pico8_js` garde 16 lignes / 17 occ. (`fold_value`, `fold_js_fn`, `fold_kind`, include), `test_pico8_lq` 28. Pour D.19c : `ui_button.h:13` n'a plus que son include. »
- **D.18c** (PRD, « Prévision pour D.19a/b et D.19c ») : deux lectures des includes, « le PRD de D.19a tranche » — tranché ici (**« Lecture »**, point 6).

### Hypothèses `H-D18c-n` (D.18c non gradée)

Source : le PRD de D.18c (`docs/prd/triage/plan3-phaseD-etape18c-b10-b11-articles-score-et-ou-types.md` à `712f68d`), sa Sortie, son « Effet sur `make data-residue` » et sa « Prévision ». À vérifier contre la note « Découvert à D.18c » et `git diff --stat 712f68d <graduation> -- src tests Makefile tools`.

| Hypothèse | Énoncé | Effet si elle tombe |
|---|---|---|
| H-D18c-1 | Après D.18c : `valeur` **435 / 75 ; 479 occ.** (`src/` 81 / 37 / 97, `tests/` 354 / 38 / 382) ; `type-tag` **241 / 57** ; résidu D **674 / 97** ; `DATA_RESIDUE_GUARD` inchangé | départs, arrivées absolues, critères `FAILED:` de départ, lignes 1–3 du tableau « Runs » à remettre ; les deltas de D.19a/b (− 138, − 110, − 70) ne dépendent pas de D.18c tant que H-D18c-2 tient |
| H-D18c-2 | D.18c ne touche **aucun TU**, sauf peut-être `test_pico8_pj.c` (assertions dans `test_ph_rows_show_ky`, `:744` et après, lignes hors motif) : les 17 lignes `valeur` de `pj` (`:41`–`:521`) gardent numéros et texte | inventaire de `pj` à remesurer ; si D.18c a touché une ligne comptée d'un TU, elle n'est plus à D.19 (écart rapporté) |
| H-D18c-3 | D.18c ne change dans `pico8_value.c/.h` que les deux marqueurs `/* TROU-13 */` : l'énumération reste `FIXED` = 0, `STRING` = 1, `BOOL` = 2, `REF` = 3 (`pico8_value.h:14-19`) et `pico8_value_bool` normalise `b ? 1 : 0` (`pico8_value.c:25`) | les mots de genre figés et la normalisation de `fold_bool` sont à redériver |
| H-D18c-4 | Aucune constante de TU ne change à D.18c (sa grep 7 vide) ; les items d'`ou` gardent les octets de `pico8_pj_item_of` | les constantes de `pj`/`fm` qui servent de preuve ici seraient autres (arrêt, rapporté) |
| H-D18c-5 | Les 30 en-têtes et 3 `.c` de `src/` gardent leur include de `pico8_value.h` (D.19c) ; `Makefile` et `tools/` inchangés ; aucun TU créé ni supprimé | la remarque « le compilateur ne voit pas un reste » change ; les règles de lien de la Sortie restent |
| H-D18c-6 | D.18c en un commit, ≈ 15 min de la mise en file au commit | budget à recalibrer sur le temps réel |
| H-D18c-7 | La ligne D.19a/b du plan reste mot pour mot celle recopiée ci-dessus ; la note de D.18c porte la prévision (≈ 117 / ≈ 145) et laisse la lecture des includes à ce PRD | « Lecture » et Annexe A à refaire |
| H-D18c-8 | `pico8_value_tostr` et `pico8_value_equal` n'ont plus que `test_pico8_value` ; `test_pico8_value` garde 27 lignes / 52 occ. | prévision de D.19c à corriger |

### Sections à revérifier à la graduation de D.18c

1. « État de départ » et critères `FAILED:` de départ (H-D18c-1) ; lignes 1–3 du tableau « Runs » de chaque sous-étape.
2. Inventaire de `test_pico8_pj` (H-D18c-2) : numéros, totaux, cibles des mutations Mc4–Mc7.
3. « Mots de genre figés » (H-D18c-3) : valeurs 0 / 1 / 2 / 3 et normalisation du booléen.
4. « Effet sur `make data-residue` » : arrivées absolues (H-D18c-1).
5. Budget (H-D18c-6).
6. « Prévision pour D.19c » (H-D18c-5, H-D18c-8) et Annexe A (H-D18c-7).

### Deux familles d'octets (lu dans chaque TU, pas supposé)

Les TU ne plient **pas** tous une valeur de la même façon. Lu à `712f68d` :

**Famille « octet propre »** (`fr`, `fv`, `g_`, `ga`, `jo`, `nr`, `jc`, `pj`, `js`) — `fold_value` sans mot de genre :

```c
case PICO8_VALUE_BOOL:   return fnv(fnv(h, 0xB0u), (uint32_t)v.as.boolean);   /* 0 ou 1 : pico8_value_bool normalise */
case PICO8_VALUE_FIXED:  return fnv(fnv(h, 0xF1u), (uint32_t)v.as.fixed);
case PICO8_VALUE_STRING: return fnv_str(fnv(h, 0x57u), v.as.string);
case PICO8_VALUE_REF:    return fold_ref(h, v.as.ref);   /* 0xDEAD pour NULL ; 0x4E + rang dans hv ; [0xEEEE + étiquette figée] ; 0xEEEE + étiquette */
default:                 return fnv(h, 0xF0u);           /* inatteignable */
```

(`jo` a le cas `REF` écrit en ligne, sans `fold_ref` ; il n'a que des appelants `BOOL`.)

**Famille « mot de genre »** (`hp`, `lq`, `hy` pour `fold_value` ; `js`, `trade`, `ui`, `entity_kind`, `fq`, `mh` pour les aides de membre) — le mot de genre d'abord, puis le contenu :

```c
h = fnv(h, (uint32_t)v.kind);                    /* FIXED 0, STRING 1, BOOL 2, REF 3 */
FIXED  : fnv(h, (uint32_t)v.as.fixed)
BOOL   : fnv(h, (uint32_t)v.as.boolean)
STRING : chaque octet, puis 0xFF                 /* hy : par son fold_string */
REF    : NULL -> fnv(h, 0xDEAD) ; sinon fnv(h, étiquette) ; hp seul, depth > 0 : puis rang dans hv, rang dans gb
```

Les aides de membre de la seconde famille écrivent le mot de genre **nu** : `fnv(fnv(h, (uint32_t)PICO8_VALUE_FIXED), (uint32_t)v)` (`hp:189`, `hy:152`, `lq:244`, `entity_kind:123`, `fq:83`, `mh:92`, `ui:1036`, `trade:102`), `fold_kind(h, PICO8_VALUE_REF)` (`js`, huit appels) : aucune `pico8_value_t` n'y passe, seul le nom de l'énumérateur est compté. **42 lignes** sont de cette sorte.

Octets déjà écrits en littéraux par des étapes précédentes, **laissés tels quels** : `fnv(fnv(h, 0xB0u), 1u)` (`fr:487`, `nr:473`, D.15b), `pico8_jo_gg(…) ? 1u : 0u` après `0xB0u` (`ga:590`, `jo:367`, D.16a), `fold_qe` de `hp` en `0u` / `3u` (D.17), `fnv(fnv(h, 3u), 0xDEADu)` / `fnv(fnv(h, 2u), 0u)` de `lq:206` (D.18a), `fold_fixed`/`fold_bool`/`fold_string` de `js` (`:237-247`, octets `0xF1`/`0xB0`/`0x57`).

### Lecture de la ligne pour D.19a/b

1. **Pli typé par genre** (G, G1) : dans chaque TU qui a un `fold_value`, celui-ci **disparaît** et, **à sa place**, une aide `static` par genre encore plié : `fold_fixed(h, pico8_fixed_t)`, `fold_bool(h, int)`, `fold_string(h, const char *)`, et `fold_ref` — **les octets exacts de sa branche**, rien d'autre. Chaque appel `fold_value(x, pico8_value_K(y)[, depth])` devient `fold_K(x, y)` ; un `pico8_value_ref(p)` va à la `fold_ref` du TU. Une aide existante de même nom et **mêmes octets** est réutilisée (`js`) ; une aide de même nom aux octets différents ne l'est pas (`jc` : `fold_fixed(h, has, f)` et `fold_string` ont une garde `0xDEAD` ; `hy` : `fold_string` ne plie pas de mot de genre — le site l'écrit devant). **Rejeté** : réutiliser les aides de membre (`fold_fixed_member(h, 1, v)`…) : elles sont définies après une partie des sites (`hp:165` avant `:176`, `lq:120-207` avant `:243`), il faudrait déplacer du code ou déclarer d'avance ; et un membre « absent » n'a pas de sens pour un élément de liste.
2. **Normalisation du booléen** (piège) : `pico8_value_bool(b)` écrit `b ? 1 : 0`. `fold_bool` des TU qui en avaient besoin écrit donc **`b ? 1u : 0u`** (même forme que `ga:590`), pas `(uint32_t)b` : `gv(…).stop` et `hd(…)` sont des `int` qu'aucun contrat ne borne à 0/1. Là où `fold_bool` existe déjà (`js`, `(uint32_t)b`), elle n'est appelée qu'avec 0 (`:1097`) : inchangée.
3. **Mots de genre figés** (G, modèle déclaré) : un identifiant compté remplacé par sa valeur est un alias (D.7c : « les trois octets de sorte de clé […] ne deviennent pas des littéraux `0u`/`1u` ») ; la sortie de D.8 est « une constante locale nommée […], octet gelé d'une énumération supprimée, (c) déclarées », et le modèle en place pour un mot de genre est `enum { FUNCTION_KIND = 4 };` de `hp:110` et `js:1084` (« The kind the name reader gave […], frozen in […]; the enum entry is gone »), comme `FROZEN_LIST_TAG` pour une étiquette. Donc, dans chaque TU de la seconde famille, une **énumération locale** des mots qu'il plie : **`enum { FIXED_KIND = 0, STRING_KIND = 1, BOOL_KIND = 2, REF_KIND = 3 };`** (seulement ceux que le TU plie ; dans `hp` et `js`, ils rejoignent l'énumération de `FUNCTION_KIND`, commentaire réécrit pour les cinq), commentaire de piège : « The kind words the frozen hashes fold before a value: any other value moves them. » Chaque `(uint32_t)PICO8_VALUE_K` devient `(uint32_t)K_KIND` (le cast était là) : **classe (c)**, déclarée ligne à ligne. Les littéraux antérieurs (`hp` `fold_qe`, `lq:206`) ne sont pas réécrits (lignes non comptées, hors du diff nécessaire). **Rejeté** : littéraux `3u` (alias, D.7c) ; `#define REF_KIND PICO8_VALUE_REF` (alias, le nom compté survivrait sous un autre nom) ; un en-tête de test partagé (fichier nouveau, dépendances du `Makefile`, et le précédent est par TU : `FROZEN_*_TAG` est recopié dans douze TU).
4. **Fixtures en types C** (G, G2) : `vec_ref` garde son nom et rend **`pico8_vec_t *`** (alloué, jamais libéré, comme aujourd'hui) ; chaque `vec_ref(…).as.ref` perd `.as.ref` ; `vec_ref_fixed` (`jc`, `mz`), `mouse_at` (`g_`) idem ; les locales `pico8_value_t mp` / `p` / `ra` / `rp` / `m1` / `m2` deviennent `pico8_vec_t *` ; `hq_over` (`jc`), `hq` (`mz`, `bg`), `fold_held` (`qh`), `with_ra` (`lq`) prennent `pico8_vec_t *` / `const pico8_vec_t *` ; `fold_particle` (`g_`, `ga`, `hj`, `mx`, `pm`) prend `const pico8_qy_t *q` et son `assert(pt.kind == REF && ref != NULL)` devient `assert(q != NULL)` (aucun octet n'était plié du genre). `ra_only_t.ra` (`mo`) devient `const pico8_vec_t *ra` (NULL au départ, comme le `{PICO8_VALUE_REF, {0}}` d'aujourd'hui, que `get_ra` lisait NULL) ; `fake_toast_t.p_` (`iv`) devient `int p_` (1 posé par `ph`, 0 pour le nil de départ : G2, le seul lecteur est le `get_p_` du TU, qui confond déjà nil et faux). `jo:496` `pico8_value_t hp` devient `pico8_hp_t *hp` ; `jc:600-603` perdent l'aller-retour `pico8_value_ref(…).as.ref`. Les casts `(void *)item->ref` (`ga:461`, `hp:231`, `hp:304`, `lq:131`, `pj:130`) disparaissent (`fold_ref` prend `const void *`).
5. **Aides retypées ou renommées** : `item_value` (`pj`) et `tuple_item_value` (`lq`) rendaient une valeur à plier ; elles deviennent **`fold_item(h, item)`** et **`fold_tuple_item(h, item)`** (même ordre de priorité : chaîne, faux, pointeur, nombre) — le contrat change (valeur → pli), aucun des deux noms ne correspond à un motif : ce n'est pas un renommage d'évitement ; déclaré. `cell_value_kind` (`lq`) garde son nom et rend `uint32_t` (le mot figé). `fold_kind` (`js`) prend `uint32_t kind`.
6. **Includes : ils partent avec le dernier pli du TU** (G) — **≈ 117 et non ≈ 145.** La règle appliquée depuis D.15b est « un `.c` qui ne nomme plus rien du module perd son include » : `slipway_display.c` (D.15b), `menu.c` (D.18b), et, pour les TU, les 13 includes de `pico8_list.h` retirés à D.12b2 avec leur dernier usage. La seconde lecture du PRD de D.18c invoquait « le précédent de D.18b : includes `pico8_value.h` des TU gardés » : mais à D.18b ces TU nommaient encore le module (leurs plis) — garder l'include n'était pas une exception à la règle, c'en était l'application. Garder ici un include dans un `.c` qui ne nomme plus rien créerait exactement l'état que la règle interdit, et renverrait 28 fichiers de plus à D.19c. Seuls les **en-têtes** attendent D.19c (un include d'en-tête nourrit ses inclueurs ; D.15b). Chaque TU plié retire donc son include **dans le commit de son dernier pli** (a2) : 12 à D.19a, 9 à D.19b1, 7 à D.19b2.
7. **Les 9 TU à include seul attendent D.19c** (G) : `ga_export_panel_by`, `jb_ol`, `je`, `layout_nil_item_draws_nothing`, `nf`, `planet_gh_row_lq`, `planet_kj_dispatch`, `probe_mm_rnd_order`, `qb`. Ils ne nomment déjà rien du module ; la règle dit qu'ils devront perdre leur include, mais **aucun pli ne les fait entrer dans l'empreinte de D.19a/b**, trois d'entre eux sont des tests de régression du §6 (`ga_export_panel_by`, `planet_gh_row_lq`, `layout_nil_item_draws_nothing`), et l'étude les range à D.19c (« les includes restants » ; « `ga_export_panel_by` n'a qu'un include (D.19c) »), qui touche de toute façon la ligne du `Makefile` de `layout_nil_item_draws_nothing`. Écart déclaré à la règle, borné à ces 9 lignes. **Rejeté** : les retirer ici (empreinte + 9 chemins, dont trois §6, pour 9 lignes que D.19c retire avec le reste).
8. **`pico8_fixed_t` reste fourni** : chaque TU plié inclut un autre en-tête qui inclut `pico8_fixed.h` (grep à `712f68d` : `pico8_fixed.h` direct dans 13 TU, `pico8_economy*.h` pour les autres). **Piège** : les 30 en-têtes de `src/` incluent encore `pico8_value.h` (H-D18c-5), donc un TU sans son include **voit encore** `pico8_value_t` : le compilateur ne signale pas un reste oublié ; seule la grep de la Sortie le fait.
9. **`-Werror`** : toute aide `static` sans appel est fatale. Partent avec `fold_value` : ses déclarations anticipées (`fv:88`, `pj:118`) et deux déclarations anticipées de `fold_ref` qui ne servaient qu'à elle (`jc:146`, `js:204`) — orphelins du diff, déclarés ; `pj:119` reste (`fold_item` l'appelle avant la définition). Aucune autre aide ne devient orpheline à `712f68d` (`listed`, `rank_in`, `fold_ref`, `pico8_type_tag_of` gardent des appelants dans chaque TU) ; si le compilateur en montre une, elle part et est déclarée.
10. **`type-tag` baisse de 2** (constat, pas un but) : les branches `REF` non NULL de `jo:132-139` et `hy:109-117` n'ont aucun appelant (`jo` ne plie que des booléens ; `hy` ne passe que `pico8_value_ref(NULL)`) et partent avec `fold_value`, dont leurs lignes `pico8_type_tag_of` `jo:138` et `hy:113`. Celles de `hp:131` et `lq:105` **se déplacent** dans la `fold_ref` du TU (une ligne retirée, une gagnée : lignes « qui gagnent une catégorie », déclarées). `mo:97-98` portent `PICO8_TYPE_TAG_STRUCT` et restent `type-tag`.
11. **Ce qui ne change pas** : `src/` (aucune ligne), `Makefile`, `tools/`, `test_pico8_value.c` (test de la couche, supprimé entier à D.19c avec le module, §6), les 9 TU à include seul, toute constante `*_HASH` / `identity[]`, `FROZEN_LIST_TAG`, `FROZEN_INSTANCE_TAG`, `VIEW_*`, `TABLE_INDEX_KEY`, les octets littéraux cités plus haut.
12. **« Compte : ≈ 115 »** : mesuré ici **117** (`src/` 81 inchangé ; `tests/` 36 = 9 includes seuls + `test_pico8_value` 27). L'écart de 2 est l'include de `test_pico8_value` plus un de la base : le « ≈ » n'est pas un critère ; les deltas exacts le sont (− 138 / − 110 / − 70 lignes).
13. **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; jamais l'appli raylib ; interdits : jonction de lignes comptées (sauf contrefactuel déclaré), alias, renommage pour échapper aux greps, cast nouveau, cale valeur ↔ typé (une union locale « genre + contenu », une aide qui refabrique une valeur), pont, aide laissée sans appel ; mutations dans une copie jetable ou restaurées par `cp` simple puis `touch` ; binaires lancés exactement comme le `Makefile`.

### État de départ

| Mesure | À `712f68d` (D.18c non commencée) | Départ de D.19a (H-D18c-1) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 494 / 78 ; 547 occ. | **435 / 75 ; 479 occ.** (`src/` 81 / 37 / 97, `tests/` 354 / 38 / 382) |
| `type-tag` | 241 / 57 (dont `tests/` 170) | **241 / 57** |
| résidu D | 733 / 99 | **674 / 97** |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |

Recette des occurrences (celle de D.12–D.18c, motif `valeur` de `tools/data_residue.sh:29`) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'valeur\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire (354 lignes de `tests/`, `712f68d`)

Classes du message de commit, lues pour des TU : **(a1)** même site, type C à la place de la valeur (appel de pli typé, locale ou membre typé, `return v;` d'une fixture) ; **(a2)** retirée sans remplaçant au site (include, définition et cas de `fold_value`, déclaration anticipée, moitié d'une condition dont le test de genre n'a plus d'objet) ; **(b)** signature d'une aide locale au TU ou membre d'un type local au TU dont le type change (l'« API » du TU) ; **(c)** même donnée, autre nom : **le mot de genre nu `PICO8_VALUE_K` → `K_KIND`** (énumération figée locale, voir « Lecture » 3), **et rien d'autre**. Chaque ligne listée **sort** de `valeur` ; aucune n'en gagne. `occ.` = lignes, sauf mention.

#### D.19a — famille planète (12 TU, 138 lignes, 138 occ.)

| TU | Lignes | (a1) | (a2) | (b) | (c) | Aides typées créées |
|---|---|---|---|---|---|---|
| `fr` | 15 | `:186`, `:435`, `:499`, `:500`, `:505`, `:506`, `:513`, `:518` (8) | `:42`, `:129`, `:131`, `:133`, `:135`, `:137` (6) | `:183` | — | `fold_bool` |
| `fv` | 16 | `:96`, `:107`, `:124`, `:126`, `:128`, `:132`, `:135`, `:237` (8) | `:38`, `:88`, `:159`, `:161`, `:163`, `:165`, `:167` (7) | `:234` | — | `fold_fixed`, `fold_bool`, `fold_string` (à la place de `:88`) |
| `g_` | 16 | `:209`, `:220`, `:259`, `:413`, `:514`, `:545`, `:546` (7) | `:42`, `:134`, `:136`, `:138`, `:140`, `:142` (6) | `:256`, `:410`, `:507` | — | `fold_fixed`, `fold_string` |
| `ga` | 28 | `:191`, `:439`, `:457`, `:459`, `:461`, `:463`, `:467`, `:470`, `:485`, `:487`, `:489`, `:510`, `:514`, `:518`, `:523`, `:563`, `:564`, `:565`, `:588`, `:589` (20) | `:39`, `:141`, `:143`, `:145`, `:147`, `:149` (6) | `:188`, `:436` | — | `fold_fixed`, `fold_bool`, `fold_string` ; `REF` → `fold_ref` (`:461`, `:518`) |
| `hj` | 8 | `:192`, `:229`, `:496`, `:525`, `:571` (5) | `:40` | `:189`, `:226` | — | — |
| `jo` | 12 | `:189`, `:370`, `:372`, `:496`, `:512` (5) | `:38`, `:123`, `:125`, `:127`, `:129`, `:131` (6) | `:186` | — | `fold_bool` |
| `mx` | 7 | `:166`, `:203`, `:488`, `:515` (4) | `:42` | `:163`, `:200` | — | — |
| `nl` | 4 | `:185`, `:426` (2) | `:38` | `:182` | — | — |
| `nr` | 17 | `:186`, `:453`, `:485`, `:486`, `:491`, `:492`, `:499`, `:504`, `:523`, `:524` (10) | `:40`, `:127`, `:129`, `:131`, `:133`, `:135` (6) | `:183` | — | `fold_bool` |
| `os` | 4 | `:221`, `:490` (2) | `:39` | `:218` | — | — |
| `pm` | 6 | `:152`, `:189`, `:512` (3) | `:39` | `:149`, `:186` | — | — |
| `qh` | 5 | `:201`, `:403` (2) | `:39` | `:198`, `:379` | — | — |
| **Total** | **138** | **76** | **43** (12 includes) | **19** | **0** | |

Formes : `:NNN` de `vec_ref` (b) → `static pico8_vec_t *vec_ref(…)` ; la ligne `return pico8_value_ref(v);` (a1) → `return v;`. `fold_value(h, pico8_value_bool(E))` → `fold_bool(h, E)` ; `fold_value(x, pico8_value_fixed(E))` → `fold_fixed(x, E)` ; `…string…` → `fold_string` ; `fold_value(x, pico8_value_ref(E))` → `fold_ref(x, E)`. `fold_particle(h, pico8_value_ref(cls_of(a)->mm(a)))` → `fold_particle(h, cls_of(a)->mm(a))`. `pico8_value_t mp;` → `pico8_vec_t *mp;`. `jo:496`/`:512` → `pico8_hp_t *hp;` / `hp = pico8_gb_find("");`. `nr:523-524` → `pico8_vec_t *p = vec_ref(…)`. `qh:379` → `fold_held(uint32_t h, void *e, const pico8_vec_t *mp)`. Lignes non comptées modifiées (attendu ≈ 80) : les `.as.ref` des appelants de `vec_ref`/`mp`/`mouse_at`, `q = pt.as.ref;` des `fold_particle` (retirée), le corps `REF` de `jo:132-139` (retiré, avec `jo:138` `type-tag`).

#### D.19b1 — TU à mots de genre (9 TU, 110 lignes, 112 occ.)

| TU | Lignes (occ.) | (a1) | (a2) | (b) | (c) | Aides / énumérateurs |
|---|---|---|---|---|---|---|
| `hp` | 34 | `:165`, `:215`, `:225`, `:228`, `:231`, `:233`, `:249`, `:252`, `:264`, `:266`, `:268`, `:292`, `:295`, `:298`, `:304`, `:306`, `:556`, `:631` (18) | `:43`, `:114`, `:118`, `:120`, `:122`, `:127` (6) | `:553` | `:160`, `:181`, `:189`, `:193`, `:205`, `:242`, `:301`, `:464`, `:623` (9) | `fold_fixed`, `fold_bool`, `fold_string` (octets puis `0xFF`), `fold_ref(h, ref, depth)` (tag, puis `depth > 0` : rang `hv`, rang `gb`) ; les 4 mots joints à `FUNCTION_KIND` (`:110`) |
| `lq` | 28 | `:120`, `:125`, `:128`, `:131`, `:133`, `:151`, `:154`, `:199`, `:201`, `:207`, `:294` (11) | `:41`, `:88`, `:92`, `:94`, `:96`, `:101` (6) | `:123` (→ `fold_tuple_item`), `:160`, `:291`, `:386` (4) | `:141`, `:163`, `:165`, `:167`, `:169`, `:244`, `:249` (7) | `fold_fixed`, `fold_bool`, `fold_string`, `fold_ref` (sans `depth` : les deux branches de `lq` rendaient l'étiquette) ; 4 mots |
| `hy` | 15 (16) | `:138`, `:308` (2 occ.), `:309` (3) | `:41`, `:100`, `:103`, `:105`, `:107`, `:109` (6) | — | `:133`, `:152`, `:161`, `:165`, `:180`, `:182` (6) | `fold_fixed` (mot + nombre) ; `:308` écrite au site : `iv->at1 != NULL ? fold_string(fnv(h, (uint32_t)STRING_KIND), iv->at1) : fnv(fnv(h, (uint32_t)REF_KIND), 0xDEADu)` ; 4 mots |
| `js` | 16 (17) | `:1097` (2 occ. : `fold_bool(fnv(fnv(h, 1u), (uint32_t)BOOL_KIND), 0)` ; porte aussi un mot (c), compté (a1)) | `:45`, `:206`, `:208`, `:210`, `:212`, `:214` (6) | `:1105` (`fold_kind(uint32_t h, uint32_t kind)`) | `:1112`, `:1137`, `:1141`, `:1152`, `:1154`, `:1162`, `:1165`, `:1171` (8) | aucune (réutilise `fold_bool`) ; 4 mots joints à `FUNCTION_KIND` (`:1084`) |
| `trade` | 4 | — | `:39` | — | `:88`, `:100`, `:102` (3) | `REF_KIND`, `BOOL_KIND`, `FIXED_KIND` |
| `ui` | 4 | — | `:57` | — | `:969`, `:1004`, `:1036` (3) | `STRING_KIND`, `FIXED_KIND` |
| `entity_kind` | 3 | — | `:30` | — | `:118`, `:123` (2) | `STRING_KIND`, `FIXED_KIND` |
| `fq` | 3 | — | `:27` | — | `:77`, `:83` (2) | `BOOL_KIND`, `FIXED_KIND` |
| `mh` | 3 | — | `:31` | — | `:92`, `:94` (2) | `FIXED_KIND`, `BOOL_KIND` |
| **Total** | **110 (112)** | **33** | **29** (9 includes) | **6** | **42** | |

Formes : `hp` — `:165`, `:215`, `:225`, `:252`, `:266` → `fold_string` ; `:228`, `:268` → `fold_bool` ; `:231`, `:298`, `:304` → `fold_ref(…, depth|3|2)` (casts `(void *)` partis) ; `:233`, `:249`, `:264`, `:292`, `:295`, `:306`, `:631` → `fold_fixed` (le `depth` passé à `fold_value` n'était lu que par `REF` : il tombe) ; le `pico8_type_tag_of` de `:131` se déplace dans `fold_ref`. `lq` — `tuple_item_value` → `fold_tuple_item(uint32_t h, const pico8_jb_item_t *item)` (`:123` b ; `:125`, `:128`, `:131`, `:133` a1), son appelant `:148` (non compté) devient `h = fold_tuple_item(h, &t->items[i]);` ; `cell_value_kind` rend `uint32_t` (`:160` b ; `:163-169` c) ; `with_ra(pico8_lq_keys_t k, pico8_vec_t *ra)` (`:386` b, `:387` non compté `k.ra = ra;`) ; le `pico8_type_tag_of` de `:105` se déplace dans `fold_ref`. `hy` — la branche `REF` non NULL (`:109-117`, dont `:113` `type-tag`) part sans remplaçant. Lignes non comptées modifiées : les commentaires qui nomment `fold_value` (`hp:142`, `:174`, `:236`, `:272`, `:300`, `:614` ; `lq:136`, `:203`, `:241` ; `hy:147` ; `js:234`), réécrits pour dire « the bytes the frozen hashes fold » sans nommer une fonction disparue ni `pico8_value_*` ; les deux énumérations `FUNCTION_KIND` (`hp:110`, `js:1084`) ; `lq:712` (`fnv(h, cell_value_kind(…))`, le cast `(uint32_t)` y devient inutile et reste).

#### D.19b2 — autres TU (7 TU, 70 lignes, 71 occ.)

| TU | Lignes (occ.) | (a1) | (a2) | (b) | (c) | Aides typées créées |
|---|---|---|---|---|---|---|
| `jc` | 26 | `:261`, `:309`, `:372`, `:411`, `:417`, `:600`, `:601`, `:602`, `:603`, `:604`, `:605`, `:606`, `:607`, `:757`, `:758`, `:1001` (16) | `:38`, `:149`, `:151`, `:153`, `:155`, `:157` (6) | `:362`, `:408`, `:414`, `:421` (4) | — | `fold_bool` (`jc` a déjà `fold_fixed`/`fold_string`, à garde `0xDEAD` : non réutilisées, non utiles) ; `REF` → `fold_ref` |
| `pj` | 17 | `:124`, `:127`, `:130`, `:132`, `:146`, `:149`, `:517`, `:519`, `:521` (9) | `:41`, `:118`, `:171`, `:173`, `:175`, `:177`, `:179` (7) | `:122` (→ `fold_item`) | — | `fold_fixed`, `fold_bool`, `fold_string` ; `REF` → `fold_ref` |
| `mo` | 7 | `:88`, `:97`, `:98`, `:103`, `:104` (5) | `:25` | `:82` | — | — |
| `iv` | 6 | `:186`, `:193`, `:208` (3) | `:28`, `:194` (2) | `:175` | — | — |
| `mz` | 6 (7) | `:256`, `:262` (2) | `:39` | `:253`, `:259`, `:265` (2 occ.) (3) | — | — |
| `bg` | 5 | `:175`, `:274` (2) | `:38` | `:172`, `:264` (2) | — | — |
| `ie` | 3 | `:240` | `:40` | `:237` | — | — |
| **Total** | **70 (71)** | **38** | **19** (7 includes) | **13** | **0** | |

Formes : `jc` — `:600-603` `assert(pico8_jc_eb(old, origin) == target);` … ; `:604-607` `h = fold_ref(h, pico8_jc_eb(…));` ; `:309` idem ; `:261` `fold_bool(h, 1)` ; `hq_over(void *jc, void *hovered, pico8_vec_t *mp)` (`:362`, `:421`), `:372` `hq_over(jc, re, mp);`, `:1001` `hq_over(jc, NULL, NULL);` ; `m1`/`m2` en `pico8_vec_t *` ; `:146` (déclaration anticipée de `fold_ref`, non comptée) part. `pj` — `item_value` → `fold_item(uint32_t h, const pico8_jb_item_t *item)` (même priorité ; ses appelants `:143`, `:300`, non comptés, deviennent `fold_item(h, …)`). `mo` — `const pico8_vec_t *ra;` (`:82`), `return e->ra;` (`:88`), `{{PICO8_TYPE_TAG_STRUCT, &g_ra_only_class}, NULL}` (`:97-98`, restent `type-tag`), `near_e.ra = &near_ra;` (`:103-104`). `iv` — `int p_;` (`:175`), `f->p_ = 1;` (`:186`), `f->p_ = 0;` (`:208`), `return f->p_ != 0;` (`:193`) ; `:194` (a2) est la seconde moitié d'une même condition. `mz` — `hq(void *mz, pico8_vec_t *mp, pico8_vec_t *rp)`, `:266` non comptée (`->hq(mz, mp, rp)`). `bg` — `hq(void *bg, pico8_vec_t *rp)`, `pico8_vec_t *rp;`.

**Contrefactuel `iv:193-194` (anti-tassement).** La condition `!(p_ est REF NULL) && !(p_ est BOOL 0)` porte le motif sur ses deux lignes ; en entier, elle devient `p_ != 0`. Écrite sur deux lignes (`return f->p_ != 0 &&` / `1;`), les deux lignes comptées sortiraient aussi : aucune ligne comptée ne reste dans aucune forme ; ce n'est pas une jonction de lignes comptées (deux sortent, une ligne hors motif entre). Déclaré au message.

#### D.19c — reste après D.19b2 (hors de ce PRD)

`test_pico8_value.c` (27 lignes / 52 occ., test de la couche), les 9 includes seuls (liste au point 7) ; `src/` : 81 lignes (45 du module, 33 includes, 3 commentaires, H-D18c-1).

#### Totaux

| Sous-étape | Lignes (occ.) | (a1) | (a2) | dont includes | (b) | (c) |
|---|---|---|---|---|---|---|
| D.19a | 138 (138) | 76 | 43 | 12 | 19 | 0 |
| D.19b1 | 110 (112) | 33 | 29 | 9 | 6 | 42 |
| D.19b2 | 70 (71) | 38 | 19 | 7 | 13 | 0 |
| **D.19a/b** | **318 (321)** | **147** | **91** | **28** | **38** | **42** |

**Δlignes > Δocc.** : attendu **aucun** fichier (`js`, `hy`, `mz` : Δocc. = Δlignes + 1 ; les autres égaux). **Lignes comptées modifiées, non retirées** : aucune en `valeur` ; en `type-tag`, `mo:97`, `:98` (modifiées, restent). **Lignes qui gagnent une catégorie** : `valeur` aucune ; `type-tag` deux (déplacements de `hp:131` et `lq:105` dans leur `fold_ref`), aucune autre ; les huit catégories gardées à 0 : aucune (aucun nom neuf ne touche un motif : `fold_*`, `*_KIND`, `vec_ref`, `fold_item`, `fold_tuple_item` — ni `_default_`, ni `_take_`, ni `field`, ni `PICO8_REF_`).

## Solution

**Trois commits** `refactor(D.19a)`, `refactor(D.19b1)`, `refactor(D.19b2)`, chacun vert seul, vérifié et poussé, dans cet ordre (sans dépendance entre eux : TU disjoints ; l'ordre ne fait que poser d'abord la forme la plus répétée). Chacun ne touche que `tests/` (les TU de sa sous-étape) et, pour la note, `docs/plan-3.md`.

1. Chaque TU plie ses nombres, booléens, chaînes et pointeurs par une aide typée aux octets de l'ancien `fold_value`, ou écrit son mot de genre par l'énumérateur figé de son énumération locale.
2. Les fixtures passent des vecteurs, des entités et des entiers, plus des valeurs.
3. Chaque TU plié perd `#include "../src/pico8_value.h"`.
4. Toutes les constantes figées sont identiques ; `src/` est identique.

**Sortie** de chaque sous-étape, constatée par commande :

- `make test` vert (dont `data-guard`, `constants-check`, `float-grep`, `float-guard` ; `tools/float_guard.txt` inchangé) ; `make e2e` **26 conformes** ; balayage ASan+UBSan des TU de la sous-étape propre (tous les TU à D.19b2) ;
- `make data-residue` :

| Mesure | Départ (H-D18c-1) | Après D.19a | Après D.19b1 | Après D.19b2 |
|---|---|---|---|---|
| `valeur` | 435 / 75 ; 479 occ. | **297 / 63 ; 341** | **187 / 54 ; 229** | **117 / 47 ; 158** |
| `valeur` `src/` | 81 / 37 ; 97 | 81 / 37 ; 97 | 81 / 37 ; 97 | 81 / 37 ; 97 |
| `valeur` `tests/` | 354 / 38 ; 382 | 216 / 26 ; 244 | 106 / 17 ; 132 | **36 / 10 ; 61** |
| `type-tag` | 241 / 57 | **240 / 57** (`jo:138`) | **239 / 57** (`hy:113`) | 239 / 57 |
| résidu D | 674 / 97 | **535 / 97** | **424 / 97** | **356 / 97** (`mo:97-98` restent `type-tag`) |

  Aucun fichier ne sort du résidu D (tous les TU pliés portent des lignes `type-tag`). Si le départ réel diffère : départ réel − 138 / − 138 occ., − 110 / − 112, − 70 / − 71.
- ces commandes rendent exactement ce qui est écrit (`$T` : les TU de la sous-étape ; `<parent>` : le parent du commit) :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. TU pliés : 0 pour chacun
  for t in $T; do printf '%s %s\n' $t "$(grep -cE "$P" tests/test_pico8_$t.c)"; done
  # 2. src/, Makefile, tools/, e2e, sha256 : vide
  git diff --stat <parent> -- src Makefile tools tests/e2e; git diff --name-only <parent> | grep -E '\.sha256$'
  # 3. aucune constante figée changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[|FROZEN_[A-Z_]*_TAG|VIEW_[A-Z_]+ =|TABLE_INDEX_KEY =)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 4. includes : exactement une ligne '-' par TU plié (pico8_value.h), aucune '+'
  git diff <parent> -- tests | grep -E '^[-+]\s*#include'
  # 5. mots de genre figés : chaque énumérateur a sa valeur, aucun n'est défini par un nom du module
  grep -hoE '\b(FIXED|STRING|BOOL|REF)_KIND = [0-9]+' tests/*.c | sort | uniq -c   # FIXED 0, STRING 1, BOOL 2, REF 3 seulement
  grep -nE '_KIND\s*=\s*PICO8_|define\s+[A-Z_]+\s+\(?PICO8_VALUE' tests/*.c           # vide
  # 6. aucun cast nouveau sur les lignes '+' : vide
  git diff <parent> -- tests | grep -E '^\+' | grep -E '\(\s*(const\s+)?void\s*\*\s*\)|\(\s*(const\s+)?[a-z0-9_]+_t\s*\*\s*\)\s*[^{ ]'
  # 7. aucune cale : ni pico8_value_t, ni union locale « genre + contenu » dans les TU pliés : vide
  grep -nE 'union\s*\{' $(for t in $T; do echo tests/test_pico8_$t.c; done) | grep -v 'pico8_lq_cell'
  # 8. aucun fold_value restant : vide
  grep -nw 'fold_value' $(for t in $T; do echo tests/test_pico8_$t.c; done)
  # 9. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^(valeur|type-tag)'
  ```

  (grep 5, après D.19b1 : les quatre valeurs attendues, et rien d'autre ; avant, `FUNCTION_KIND = 4` n'est pas dans ce motif. grep 6 : les lignes `+` peuvent garder un `(uint32_t)` sur un énumérateur — motif exclu, il était là.)

## User Stories

1. En tant que mainteneur du port, je veux que les TU plient un nombre, un booléen, une chaîne ou un pointeur sans fabriquer une valeur Lua, afin que D.19c puisse supprimer le module sans toucher un pli.
2. En tant que gardien des tests, je veux que chaque pli typé écrive exactement les octets de la branche de `fold_value` qu'il remplace, afin que toutes les constantes figées restent égales (G1).
3. En tant que gardien des tests, je veux que `fold_bool` écrive `b ? 1u : 0u` là où `pico8_value_bool` normalisait, afin qu'aucune empreinte ne dépende d'un `int` non borné.
4. En tant que gardien du compteur, je veux que les mots de genre nus soient écrits par une énumération figée locale (sur le modèle de `FUNCTION_KIND`, `FROZEN_LIST_TAG`, `TABLE_INDEX_KEY`), déclarée (c), afin qu'aucun identifiant compté ne soit remplacé par sa valeur.
5. En tant que lecteur d'un TU, je veux que `vec_ref` rende un `pico8_vec_t *`, afin que ses appelants cessent de lire `.as.ref`.
6. En tant que lecteur d'un TU, je veux que `fold_particle` prenne une particule, `hq_over`/`hq`/`fold_held` un vecteur, afin que les signatures locales disent ce qu'elles reçoivent.
7. En tant que mainteneur, je veux que les fausses entités de `mo` et `iv` portent un pointeur et un entier, afin qu'aucun membre de struct de test ne soit une valeur.
8. En tant que mainteneur, je veux que `item_value` et `tuple_item_value` deviennent des plis d'item, afin qu'aucune aide ne rende une valeur à plier.
9. En tant que mainteneur, je veux qu'un TU qui ne nomme plus rien du module perde son include dans le commit de son dernier pli, afin d'appliquer aux TU la règle de D.15b, D.18b et D.12b2.
10. En tant que mainteneur, je veux que les 9 TU à include seul (dont trois §6) attendent D.19c, afin que D.19a/b ne touche que les fichiers dont elle réécrit un pli.
11. En tant que gardien de `-Werror`, je veux que chaque aide ou déclaration anticipée devenue sans objet parte et soit nommée au message, afin qu'aucune ne reste orpheline.
12. En tant que gardien des tests, je veux des mutations ciblées qui changent un octet de chaque pli typé (ou un énumérateur figé) et rougissent le TU, afin de prouver que les plis mordent encore.
13. En tant que gardien des tests, je veux une règle pour une mutation qui ne rougit rien — rejouée sur la branche de `fold_value` au parent —, afin de distinguer un chemin jamais plié d'un chemin perdu.
14. En tant que gardien des filets, je veux `make test`, `make e2e` et un balayage ASan+UBSan des TU touchés (tous à la fin), afin de prouver qu'aucune fixture retypée ne lit hors de sa mémoire.
15. En tant que gardien du compteur, je veux au message de chaque commit les comptes par catégorie et par fichier, en lignes et occurrences, la classification (a1)/(a2)/(b)/(c) ligne à ligne avec les numéros du parent, les lignes qui gagnent une catégorie (« aucune » compris), les Δlignes > Δocc. (attendu aucun), le contrefactuel de `iv`, afin qu'aucune baisse ne soit un tassement.
16. En tant que vérificateur, je veux la preuve qu'aucune ligne de `src/` ne change, afin de ne pas exiger de `gcov` de `src/`.
17. En tant que `plan-pilot`, je veux trois sous-étapes d'environ 20 à 25 minutes, un plafond de 50 minutes chacune, un repli écrit et une note « Découvert à D.19… » par sous-étape, afin que D.19c parte d'un état exact.
18. En tant que lecteur du plan, je veux que la note finale dise ce qui reste pour D.19c (9 includes, `test_pico8_value`, `src/` 81) et pour D.20a (`type-tag` `tests/` 168), afin que les étapes suivantes partent de comptes justes.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules, tests et découpage ; ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, orphelins du diff retirés, critères vérifiables) et marqués **(G)** ; les rejets sont notés. Q12 (oui) et Q13 (A) sont des décisions humaines ; l'étude D.14 (G1, G2, G4, lignes D.19a/D.19b) fait foi sauf écart déclaré : trois sous-étapes et non deux (b coupée en b1/b2) ; 9 includes seuls laissés à D.19c ; `type-tag` − 2 ; (c) = 42 (mots de genre figés).

### Modules

- **TU de la famille planète** (D.19a) : `test_pico8_{fr,fv,g_,ga,hj,jo,mx,nl,nr,os,pm,qh}.c`.
- **TU à mots de genre** (D.19b1) : `test_pico8_{hp,lq,hy,js,trade,ui,entity_kind,fq,mh}.c`.
- **Autres TU pliés** (D.19b2) : `test_pico8_{jc,pj,mo,iv,mz,bg,ie}.c`.
- **Docs** : `docs/plan-3.md` (note par sous-étape), rien d'autre.
- Inchangés : tout `src/`, `Makefile`, `tools/`, `test_pico8_value.c`, les 9 TU à include seul, `tests/e2e/`, `docs/glossaire-noms.md`, `docs/trous-de-portage.md` (aucun nom minifié ne change de sens, aucun écart port/cart touché).

### Représentation (G)

- **Famille « octet propre »** : `static uint32_t fold_fixed(uint32_t h, pico8_fixed_t x) { return fnv(fnv(h, 0xF1u), (uint32_t)x); }`, `fold_bool(h, int b)` → `fnv(fnv(h, 0xB0u), b ? 1u : 0u)`, `fold_string(h, const char *s)` → `fnv_str(fnv(h, 0x57u), s)` ; `REF` : la `fold_ref(h, const void *)` du TU. Commentaire (piège) au-dessus du groupe : « The words the frozen hashes fold before a value — 0xF1 a number, 0xB0 a boolean, 0x57 a string: any other moves every *_HASH. » (précédent : `js:234-236`).
- **Famille « mot de genre »** : `fold_fixed(h, x)` → `fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)x)` ; `fold_bool` → `fnv(fnv(h, (uint32_t)BOOL_KIND), b ? 1u : 0u)` ; `fold_string` → mot `STRING_KIND`, chaque octet, `0xFF` ; `fold_ref` → mot `REF_KIND`, `0xDEAD` si NULL, sinon l'étiquette (`hp` : puis, si `depth > 0`, rang dans `hv` et `gb_rank`). Énumération locale des mots (voir « Lecture » 3).
- **Fixtures** : `static pico8_vec_t *vec_ref(…)` (contrat inchangé : allouée, jamais libérée — les TU tournent sous `detect_leaks=0` dans le balayage) ; `mouse_at` idem ; `fold_particle(uint32_t h, const pico8_qy_t *q)` ; `fold_held(…, const pico8_vec_t *mp)` ; `hq_over(…, pico8_vec_t *mp)` ; `hq` de `mz`/`bg` en `pico8_vec_t *` ; `with_ra(…, pico8_vec_t *ra)` ; `ra_only_t.ra` en `const pico8_vec_t *` ; `fake_toast_t.p_` en `int`.
- **Piège de forme** : garder un `fold_value` qui prend une petite union locale (genre + contenu), ou une aide qui rend une struct « valeur » sous un autre nom, serait une cale : interdit. Un `fold_K` ne fait que ce que faisait **une** branche.
- **Piège de motif** : un commentaire ou un nom neuf qui écrit `pico8_value_…` ou `PICO8_VALUE_…` (« as pico8_value_bool normalised ») **regagne** une ligne `valeur` : les commentaires disent « the frozen hashes », jamais le nom du module.

### Anti-tassement et message de commit (liant)

- **Interdit** : joindre des lignes comptées (seul `iv:193-194` passe de deux à une, contrefactuel déclaré) ; renommer un identifiant vivant pour échapper au motif (`vec_ref`, `mouse_at`, `cell_value_kind`, `fold_kind` gardent leur nom ; `item_value` → `fold_item` et `tuple_item_value` → `fold_tuple_item` changent de contrat, déclarés) ; alias (`#define`/`typedef`/énumérateur défini par un nom du module ; littéral à la place d'un `PICO8_VALUE_K`) ; cast nouveau ; cale ou pont ; toucher une constante figée, `src/`, `tests/e2e/`, un `*.sha256`, `tools/`, le `Makefile`, un include autre que celui de `pico8_value.h` des TU pliés.
- **Classification exigée** (recette : `git diff -U0 <parent> -- tests/test_pico8_X.c`, lignes `-` qui portent le motif en regard des `+`) : D.19a **(a1) 76, (a2) 43, (b) 19, (c) 0** ; D.19b1 **(a1) 33, (a2) 29, (b) 6, (c) 42** ; D.19b2 **(a1) 38, (a2) 19, (b) 13, (c) 0**, recomptée au parent, ligne à ligne (numéros du parent). Chaque (c) est listée avec son énumérateur.
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et par fichier du diff, en lignes (`--detail`) **et** occurrences.
- **Lignes qui gagnent une catégorie**, par fichier et catégorie, « aucune » compris : attendu `type-tag` `hp` 1 et `lq` 1 (déplacements, à D.19b1), aucune autre.
- **Lignes non comptées modifiées** : nombre par fichier au message (attendu ≈ 80 à D.19a, ≈ 40 à D.19b1, ≈ 60 à D.19b2 : `.as.ref`, appelants de `hq_over`/`hq`, commentaires qui nommaient `fold_value`, énumérations `FUNCTION_KIND`).
- **Aides retirées, retypées, créées**, par TU, avec leurs appelants.
- **Δlignes > Δocc.** : attendu aucun fichier.
- **Numéros de ligne** : ceux du parent.

### Empreinte et découpage (G)

| Commit | TU (`tests/test_pico8_*.c`) | Chemins | `valeur` retirées (l. / occ.) | Includes | Aides créées | Coût attendu |
|---|---|---|---|---|---|---|
| **D.19a** | `fr`, `fv`, `g_`, `ga`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` | 12 + plan | 138 / 138 | 12 | `fold_bool` (5 TU), `fold_fixed`/`fold_string` (3) | ≈ 25 min |
| **D.19b1** | `hp`, `lq`, `hy`, `js`, `trade`, `ui`, `entity_kind`, `fq`, `mh` | 9 + plan | 110 / 112 | 9 | trois TU réécrivent `fold_value` ; six ne changent que des mots | ≈ 25 min |
| **D.19b2** | `jc`, `pj`, `mo`, `iv`, `mz`, `bg`, `ie` | 7 + plan | 70 / 71 | 7 | `fold_bool` (2), `fold_fixed`/`fold_string` (1), `fold_item` | ≈ 20 min |

- **Pourquoi trois et non deux** (G) : la ligne dit « D.19a/b » et l'étude met 16 TU dans b ; D.11d (même nature : TU seuls, plis figés) a tenu 9 à 12 TU par sous-étape, gradées en ≈ 21–27 min pour un budget de ≈ 50 ; 16 TU de deux formes différentes dépasseraient ce rythme. b est coupée **par forme** : b1 introduit les mots de genre figés (toutes les lignes (c)), b2 reprend la forme de D.19a. **Rejeté** : un commit unique (28 TU, un plafond qui ne se tient pas) ; D.19b en un commit (16 TU, deux modèles) ; quatre sous-étapes (coût fixe d'un commit pour 35 lignes).
- **Rythme mesuré** : D.18a ≈ 10 min de la mise en file au commit, D.18b ≈ 15 (H-D18c-6 pour D.18c) ; D.11d1–d4 ≈ 21–27 min pour 9–12 TU et des centaines d'appels. Ici, par sous-étape : éditions ≈ 8–12 (mécaniques, un TU à la fois, chaque TU construit et lancé aussitôt) ; mutations ≈ 5 ; balayage ASan+UBSan ≈ 3 (tous les TU à b2 : ≈ 5) ; `make test`, `make e2e` ≈ 2 ; mesures, classification, message, note ≈ 3. **Budget ≈ 25 min jusqu'au commit, plafond 50 min par sous-étape.**
- **Repli écrit** (G) : au plafond sans commit, rien n'est commis et `plan-pilot` relance la sous-étape coupée en deux, sans pont, TU disjoints : **D.19a** → a1 (`fr`, `fv`, `g_`, `ga`, `jo`, `nr` : les six `fold_value`, 94 lignes) / a2 (`hj`, `mx`, `nl`, `os`, `pm`, `qh` : fixtures seules, 44) ; **D.19b1** → (`hp`, `lq`, `hy` : 77) / (`js`, `trade`, `ui`, `entity_kind`, `fq`, `mh` : 33) ; **D.19b2** → (`jc`, `pj` : 43) / (`mo`, `iv`, `mz`, `bg`, `ie` : 27). Comptes recomptés par commit.

### Effet sur `make data-residue`

Voir le tableau de la Sortie. Par fichier : chaque TU plié passe de ses lignes (inventaire) à **0 / 0** ; `test_pico8_value.c` 27 / 52 et les 9 includes seuls inchangés ; `type-tag` `jo` 14 → 13, `hy` 7 → 6, `hp` 6 → 6, `lq` 5 → 5, `mo` 4 → 4. « − 138 / − 138 », « − 110 / − 112 », « − 70 / − 71 », « `type-tag` − 1, − 1, 0 » (depuis le départ réel) sont des critères (écart = explication chiffrée ligne à ligne au message).

### Prévision pour D.19c (après D.19b2)

- `valeur` **117 lignes / 47 fichiers / 158 occ.** : `src/` 81 / 37 / 97 (le module 45 lignes, 33 includes — 3 `.c` et 30 en-têtes —, 3 commentaires : `geom.h:37-38`, `type_tag.h:10`, H-D18c-1) ; `tests/` 36 / 10 / 61 : `test_pico8_value.c` 27 / 52 et les includes de `ga_export_panel_by`, `jb_ol`, `je`, `layout_nil_item_draws_nothing`, `nf`, `planet_gh_row_lq`, `planet_kj_dispatch`, `probe_mm_rnd_order`, `qb`.
- **Aucun TU hors `test_pico8_value` ne nomme plus rien du module** ; mais les en-têtes de `src/` l'incluent encore : D.19c retire les includes d'en-tête en vérifiant que chaque TU garde `pico8_fixed_t` par un autre chemin (il le garde, grep de ce PRD).
- `type-tag` 239 / 57 (`tests/` 168) : les énumérations `*_KIND` de ce PRD n'en font pas partie ; les `FROZEN_*_TAG` et les `pico8_type_tag_of` des `fold_ref` sont à D.20a.

### Documentation (G)

- `docs/plan-3.md` : une note « Découvert à D.19a » (puis « à D.19b1 », « à D.19b2 ») en fin de ligne D.19a/b, rien d'autre. Elle dit : le commit ; les comptes réels et l'écart aux attendus ; les lectures (deux familles d'octets ; `fold_K` par genre à la place de `fold_value` ; `fold_bool` normalisée ; énumérations `*_KIND` figées, (c) ; `vec_ref` en `pico8_vec_t *` ; `fold_item`/`fold_tuple_item` ; includes retirés avec le dernier pli, 9 laissés à D.19c) ; les assertions ajoutées s'il y en a ; `type-tag` − 2 ; à la dernière : **pour D.19c** la prévision ci-dessus, **pour D.20a** `type-tag` `tests/` 168.
- Commentaires des TU : quatre sortes de `CLAUDE.md` ; le commentaire de piège des aides typées et de l'énumération figée ; les commentaires qui nommaient `fold_value` réécrits sur les octets figés ; aucun renvoi à D.19 ni au plan.

### Ce qui ne change pas

- `src/` (aucune ligne), `Makefile`, `tools/`, `tests/e2e/`, tout `*.sha256`, `tools/class_coverage_net_expected.txt`, `tools/float_guard.txt`.
- Toute constante `*_HASH` / `identity[]`, `FROZEN_LIST_TAG`, `FROZEN_INSTANCE_TAG`, `VIEW_*`, `TABLE_INDEX_KEY`, `FUNCTION_KIND = 4` ; les octets déjà littéraux (`fr:487`, `nr:473`, `ga:590`, `jo:367`, `hp` `fold_qe`, `lq:206`, `js:237-247`).
- `test_pico8_value.c`, les 9 TU à include seul, `test_pico8_fm` et les autres TU qui ne nomment pas le module.
- Le comportement : aucun TU ne plie autre chose ni dans un autre ordre ; aucun test ajouté ou supprimé, sauf assertion exigée par une mutation (voir « Testing Decisions »).

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : sous `-Wall -Wextra -Werror`, `pico8_vec_t *` passe où `pico8_vec_t *` / `const pico8_vec_t *` sont attendus (`hq`, `hd`, `bv`, `gk`, `kj`, `ldown`, `rdown`, clés `p`/`ra`) ; `struct pico8_qy *` (retour de `mm`) passe à `const pico8_qy_t *` ; `pico8_jc_eb` rend `pico8_struct_header_t *`, comparé et plié tel quel.
- **H2** : aucune aide typée créée n'est sans appel (sinon elle n'est pas créée) ; les orphelins sont ceux de « Lecture » 9.
- **H3** : `vec_ref` n'est jamais appelée pour autre chose que lire `.as.ref` ; si un site lit `.kind` d'un `vec_ref`, il est à classer (arrêt « site sans règle » s'il n'entre dans aucune).
- **H4** : aucun site ne passe une chaîne NULL à `fold_value(…, pico8_value_string(…))` (tous gardés ou littéraux) : `fold_string` n'a pas de garde, comme la branche qu'elle remplace.
- **H5** : `gv(…).stop` et `hd(…)` rendent 0 ou 1 dans ces TU (la normalisation de `fold_bool` est alors sans effet observable ; elle reste, parce que `pico8_value_bool` la faisait).

## Implementation Chunks

**D.19a** (un commit)

1. Mesure de départ (graduation de D.18c ou un descendant `docs/` seul : H-D18c-1) ; remesure par contenu des 138 lignes.
2. Pour chaque TU, dans cet ordre (`fr`, `nr`, `jo` : `fold_bool` seul ; `fv`, `g_`, `ga` : trois aides ; `hj`, `mx`, `nl`, `os`, `pm`, `qh` : fixtures) : aides typées à la place de `fold_value`, appels, fixtures, include ; `make tmp/test_pico8_<t>` et lancement ; constante inchangée.
3. `make test`, `make e2e`, balayage ASan+UBSan des 12 TU ; mutations Ma1–Ma13 ; mesures, classification ; note ; **commit**, vérification, push.

**D.19b1** (un commit) — même cycle ; ordre : `trade`, `ui`, `entity_kind`, `fq`, `mh` (mots seuls), `js`, `hy`, `lq`, `hp` ; mutations Mb1–Mb6.

**D.19b2** (un commit) — même cycle ; ordre : `ie`, `bg`, `mz`, `mo`, `iv`, `pj`, `jc` ; balayage ASan+UBSan de **tous** les TU ; mutations Mc1–Mc10 ; note de clôture avec les prévisions pour D.19c et D.20a.

## Testing Decisions

Un bon test juge ce que le code rend ou fait, pas la présence d'une ligne. D.19a/b ne change **aucune ligne de `src/`** : ce qui est jugé, c'est que chaque TU plie **les mêmes octets** qu'avant (ses constantes figées, inchangées, en sont la preuve) et que chaque pli typé **mord** (une mutation d'un de ses octets rougit le TU). Aucun `gcov` de `src/` n'est exigé : la grep 2 de la Sortie prouve que `src/` est identique au parent.

### TU touchés (G)

1. Suivent : les 28 TU de l'inventaire, et eux seuls.
2. **Constantes** : aucune ne change ; une constante qui rougit est un octet mal reproduit (bug de ce commit, pas une mise à jour).
3. **Assertion nouvelle** : seulement si une mutation exigée ne rougit pas le TU alors que la **même** mutation sur la branche de `fold_value` au parent le rougit (chemin perdu) — cas qui devrait être impossible si les octets sont reproduits ; `FAILED:` s'il se produit sans explication.
4. **Prior art** : `fold_fixed`/`fold_bool`/`fold_string` de `test_pico8_js` (D.11d) ; `fold_qe` de `test_pico8_hp` (D.17) ; énumérations figées `FUNCTION_KIND`, `VIEW_*`, `TABLE_INDEX_KEY` (D.8), `FROZEN_LIST_TAG` (D.12b) ; `fold_ib` des sept TU (D.12b2) ; D.11d (TU seuls, constantes identiques, quatre sous-étapes).

### Mutations

Une à la fois, **dans une copie jetable** (ou fichier sauvegardé par `cp`, restauré par `cp` simple puis `touch`, jamais `git checkout`/`stash`) ; le TU lancé exactement comme le `Makefile` le lance. Chacune doit rougir le TU nommé. **Règle d'équivalence** : une mutation qui ne rougit rien est rejouée **au parent** sur la branche correspondante de `fold_value` (même octet, ex. `0xB0u` → `0xB1u` dans `case PICO8_VALUE_BOOL`) ou sur le mot `(uint32_t)PICO8_VALUE_K` (+ 8) : si le parent reste vert aussi, le chemin n'a jamais été plié par ce TU (déclaré, rien à ajouter) ; si le parent rougit et l'enfant non, un chemin est perdu : `FAILED:`.

**D.19a** (exigées : Ma1–Ma10)

- **Ma1** `fr` : `fold_bool` plie `0xB1u` → `fr` rouge (`LIFE_HASH`). **Ma2** `nr` : idem → `nr` (`LIFE_HASH`). **Ma3** `jo` : idem → `jo` (`DAUGHTERS_HASH`).
- **Ma4** `ga` `fold_bool`, **Ma5** `ga` `fold_fixed` (`0xF2u`), **Ma6** `ga` `fold_string` (`0x58u`) → `ga` (`BUILT_HASH` ou `LIFE_HASH`) ; **Ma7** `ga:518` `fold_ref(…, lq->ra)` → `fold_ref(…, NULL)` → `ga`.
- **Ma8** `fv` `fold_string`, **Ma9** `fv` `fold_fixed` → `fv` ; **Ma10** `g_` `fold_string` et `fold_fixed` (deux mutations) → `g_` (`BUILT_HASH`).
- **Ma11** `ga:461` (item `is_ref` d'un tuple) → `fold_ref(h, NULL)` ; **Ma12** `fv` `fold_bool` (item `is_false` d'une rangée) : règle d'équivalence si rien ne rougit.
- **Ma13** (fixture, identité) `qh:414` : passer un `vec_ref(vec_xy(7680, 7680))` neuf à `hq` au lieu de `mp` → l'`assert(qh->jo.ra == mp)` de `fold_held` rougit `qh` ; prouve que `vec_ref` rend bien le pointeur que l'entité garde.
- **Ma14** (équivalence attendue, rapportée) `fr` : `b ? 1u : 0u` → `(uint32_t)b` → vert attendu (H5).

**D.19b1** (exigées : Mb1–Mb4)

- **Mb1** : chaque énumérateur `*_KIND` défini dans chaque TU, un à la fois, décalé de + 8 → le TU rouge. 27 mutations : `hp` 4 (`GB_HASH`, `INSTALL_HASH`, `DEFAULTS_HASH`…), `lq` 4 (`CONSTRUCTIONS_HASH`…), `hy` 4, `js` 4 (`JS_MEMBERS_HASH`), `trade` 3, `ui` 2 (`KE_ME_HASH`, `KE_T_HASH`), `entity_kind` 2 (`KIND_HASH`), `fq` 2, `mh` 2 ; règle d'équivalence pour un énumérateur qui ne rougit rien (ex. `BOOL_KIND` de `hy`, plié seulement pour un `hl` faux).
- **Mb2** `hp` `fold_ref` : `depth <= 0` → `depth < 0` → `hp` rouge.
- **Mb3** `hp` `fold_string` : `0xFFu` final → `0xFEu` → `hp` rouge ; **Mb4** `lq` `fold_tuple_item` : branche pointeur passée à `fold_ref(h, NULL)` → `lq` rouge (tuples `ld`/`jb` à pointeur, `:604`) sinon équivalence.
- **Mb5** `hy:308` : `0xDEADu` → `0xDEAEu` → `hy` ou équivalence ; **Mb6** `lq` `cell_value_kind` : `ROW` → `STRING_KIND` → `lq` rouge (`:712`).

**D.19b2** (exigées : Mc1–Mc6)

- **Mc1** `jc` `fold_bool` (`0xB1u`) → `jc` (pli de `hg`). **Mc2** `jc:604` : `pico8_jc_eb(old, origin)` → `(old, target)` → `jc` (`EB_HASH`). **Mc3** `jc:1001` : `hq_over(jc, NULL, NULL)` → `vec_ref(0, 0)` → `jc` rouge (l'`assert(… mouse_point_ra(re) == NULL)` qui suit).
- **Mc4** `pj` `fold_item`, branche faux (`0xB1u`) ; **Mc5** branche nombre (`0xF2u`) ; **Mc6** branche chaîne (`0x58u`) → `pj` (`BUILT_HASH`, `SHARED_HASH` ou `LIFE_HASH` : les items d'`ou` passent par `fold_item`, le `false` initial compris).
- **Mc7** `pj` `fold_item`, branche pointeur → `fold_ref(h, NULL)` → `pj` ou équivalence.
- **Mc8** `mo` : `ra_only_get_ra` rend NULL → `mo` rouge (`test_gd_on_placeholder`). **Mc9** `iv` : `fake_toast_get_p_` rend 0 → `iv` rouge (`test_dg_drives_a_ke_struct`). **Mc10** `bg` : `hq(bg, vec_ref(10, 20))` au lieu de `rp` → `bg` rouge (`assert(bg->mq == rp)`).

**Si une mutation exigée (Ma1–Ma10, Mb1–Mb4, Mc1–Mc6) ne rougit pas son TU sans que la règle d'équivalence l'explique : `FAILED:`.**

### Filets

- `make test`, `make e2e` (26 conformes), à chaque sous-étape (bon marché ; `src/` inchangé, ils doivent l'être).
- **Balayage ASan+UBSan des TU** : dans une copie jetable du scratchpad, les TU de la sous-étape (tous les TU à D.19b2) compilés avec les flags du `Makefile` plus `-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer`, lancés comme le `Makefile` avec `ASAN_OPTIONS=detect_leaks=0` (`vec_ref` ne libère pas, par contrat) ; `Makefile` inchangé. Précédents : D.7b, D.18c.
- `make sanitize-net`, `make class-coverage-net` : **non exigés** (aucun fichier de `src/` ne change, aucun constructeur) ; s'ils sont lancés, rapportés.
- Aucun TU ne rejoue un fichier d'événements ; rien de lent.

### Vérification, budget et tableau « Runs »

**Budget : environ 25 min de temps réel par sous-étape jusqu'au commit**, mesures, mutations et filets compris ; **plafond 50 min** (au-delà : repli de la sous-étape). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<t>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute |
| `make e2e` | ~1 s |
| mutations (a : 14 ; b1 : ≈ 32 ; b2 : 10), chacune un `make` ciblé | < 6 min par sous-étape |
| balayage ASan+UBSan (TU de la sous-étape ; tous à b2) | 2 à 5 min |
| `make data-residue`, recette, classification, greps | < 3 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; rien d'inachevé n'est commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** par sous-étape, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + état) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | graduation de D.18c, avant D.19a | 0 | `valeur 435 75`, `type-tag 241 57` (H-D18c-1) | … |
| 2 | recette `valeur` | idem | 0 | `valeur 435 lignes 75 fichiers 479 occ.` | … |
| 3 | `for t in fr fv g_ ga hj jo mx nl nr os pm qh; do grep -cE "$P" tests/test_pico8_$t.c; done` | idem | 0 | `15 16 16 28 8 12 7 4 17 4 6 5` | … |
| … | … | … | … | … | … |

(D.19b1, ligne 3 : `hp lq hy js trade ui entity_kind fq mh` → `34 28 15 16 4 4 3 3 3` ; D.19b2 : `jc pj mo iv mz bg ie` → `26 17 7 6 6 5 3`.) Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les TU de la sous-étape et `docs/plan-3.md`. Ni `src/`, ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`, ni un §6, ni `test_pico8_value.c`, ni un des 9 TU à include seul.
2. **Greps** de la Sortie, exactement.
3. **Formes** : aucun `fold_value` ; une aide `fold_K` par genre plié, qui fait ce que faisait une branche ; `fold_bool` normalisée là où elle est créée ; énumérateurs `*_KIND` = 0 / 1 / 2 / 3, définis par des littéraux ; `vec_ref` en `pico8_vec_t *` ; `fold_particle`, `hq_over`, `hq`, `fold_held`, `with_ra`, `ra_only_t`, `fake_toast_t` typés ; `fold_item`/`fold_tuple_item`.
4. **Casts** : grep 6 vide ; les cinq `(void *)item->ref` partis.
5. **Constantes** : grep 3 vide ; mutations rapportées.
6. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente, juste, avec numéros du parent ; (c) exactement les mots de genre ; contrefactuel `iv` ; includes (grep 4).
7. **Écarts déclarés** : trois sous-étapes ; 9 includes à D.19c ; `type-tag` − 2 et deux lignes déplacées ; (c) non nulle à b1 ; `item_value`/`tuple_item_value` renommées ; orphelins retirés.
8. **Preuve** : mutations restaurées (fichiers identiques au commit), règle d'équivalence appliquée et rapportée, balayage ASan+UBSan rapporté, `src/` identique.
9. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.19 ni au plan ; aucun commentaire neuf ne nomme `pico8_value_*` ni `fold_value`.
10. **Docs** : note « Découvert à D.19… » en fin de ligne D.19a/b, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ de D.19a n'est pas la graduation de D.18c (ou un descendant qui ne touche que `docs/`), propre ; ou si `valeur` n'y vaut pas 435 / 75 / 479 occ., `type-tag` 241 / 57, résidu D 674 / 97, sans explication (valeurs remises à la graduation de D.18c) ; de même pour D.19b1 (297 / 63 / 341, 240 / 57, 535 / 97) et D.19b2 (187 / 54 / 229, 239 / 57, 424 / 97), sauf écart hérité expliqué.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change ; si un site ne tombe sous aucune règle de ce PRD (arrêt « site sans règle ») ; si le balayage ASan+UBSan n'est pas propre.

**Périmètre et forme**

- `FAILED:` si une ligne de `src/`, du `Makefile` ou de `tools/` change ; si un include change hors de ceux de `pico8_value.h` des TU pliés ; si le commit touche un chemin hors de son empreinte.
- `FAILED:` si une grep de la Sortie ne rend pas ce qui y est écrit.
- `FAILED:` si un `fold_value` survit dans un TU plié, si une aide typée fait plus qu'une branche de l'ancien pli, ou si une union locale « genre + contenu », une aide qui refabrique une valeur, un pont apparaît.
- `FAILED:` si un cast nouveau est ajouté, si des lignes comptées sont jointes (hors `iv:193-194`, déclaré), si un identifiant compté est remplacé par sa valeur ou par un nom défini à partir d'un nom du module.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne baisse pas de **138 / 138 occ.** (D.19a), **110 / 112** (D.19b1), **70 / 71** (D.19b2) exactement depuis le départ réel, sans explication chiffrée ligne à ligne ; si `type-tag` ne baisse pas de 1, 1, 0.
- `FAILED:` **si un compte baisse par tassement** : lignes comptées jointes, alias, renommage d'un identifiant vivant pour le motif, commentaire compté réécrit hors de son sujet, cale ou pont, (c) hors des mots de genre.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) ligne à ligne avec numéros du parent ; les lignes qui gagnent une catégorie (« aucune » compris) ; la justification de chaque Δlignes > Δoccurrences (attendu : aucune) ; le contrefactuel `iv` (à b2) ; les aides créées, retypées, retirées et les orphelins ; la liste des (c) et leur énumérateur (à b1) ; les mutations et la règle d'équivalence ; le balayage ASan+UBSan ; la preuve que `src/` est identique ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH`, `identity[]`, `FROZEN_*_TAG`, `VIEW_*`, `TABLE_INDEX_KEY` ou `FUNCTION_KIND` change.
- `FAILED:` si une mutation manque, ne se restaure pas, ou ne rougit rien sans que la règle d'équivalence l'explique ; **si une mutation exigée ne rougit pas son TU** alors que la même, au parent, le rougissait.
- `FAILED:` si le plafond est dépassé sans note (et sans repli), ou s'il manque un tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.19… » d'une sous-étape, ou si la dernière omet les comptes réels, les lectures, la prévision pour D.19c et le compte `type-tag` pour D.20a.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`, ou nomme `pico8_value_*` ou `fold_value`.

## Out of Scope

- `test_pico8_value.c`, `pico8_value.c/.h`, les includes des en-têtes et des 3 `.c` de `src/`, les 9 TU à include seul, le `Makefile`, TROU-3, TROU-4, le glossaire (`:294`, `:657`) : D.19c.
- Les lectures d'étiquette des TU (`pico8_type_tag_of` dans `fold_ref`, `FROZEN_*_TAG`, `is_lq`/`is_of_class` de `js`, asserts `PICO8_TYPE_TAG_STRUCT`) : D.20a.
- Réécrire les octets déjà littéraux (`hp` `fold_qe`, `lq:206`, `fr:487`, `nr:473`, `ga:590`, `jo:367`) sur les énumérateurs neufs : non nécessaire, hors du diff ; à une étape qui en aurait besoin.
- Unifier les aides de membre (`fold_*_member`) avec les aides typées : hors de la ligne.
- Toute ligne de `src/` ; tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Découpage** : trois commits D.19a / D.19b1 / D.19b2 (12 / 9 / 7 TU), ≈ 25 min chacun, plafond 50 ; repli par moitiés écrit, sans pont.
  - **Plis** : une aide par genre, à la place de `fold_value`, octets de sa branche ; réutilisation seulement à octets identiques (`js`) ; `fold_bool` normalisée.
  - **Mots de genre** : énumération locale `FIXED_KIND`…`REF_KIND` = 0…3, modèle `FUNCTION_KIND` / `FROZEN_LIST_TAG` / `TABLE_INDEX_KEY`, (c) déclarées.
  - **Fixtures** : pointeurs et entiers ; `vec_ref` garde son nom ; `item_value`/`tuple_item_value` deviennent des plis.
  - **Includes** : retirés avec le dernier pli du TU (≈ 117) ; les 9 includes seuls à D.19c.
  - **Tests** : aucun ajouté ni retiré ; mutations par octet et par énumérateur, règle d'équivalence au parent ; balayage ASan+UBSan ; pas de `gcov`, `src/` identique.
- **Surprises de mesure, pour `plan-pilot`** : (1) **deux familles d'octets** et non une : `fold_value` à octet propre (`0xF1`/`0xB0`/`0x57`, `fold_ref` à `0xDEAD`/`0x4E`/`0xEEEE`) dans `fr`, `fv`, `g_`, `ga`, `jo`, `nr`, `jc`, `pj`, `js`, et à **mot de genre** (0 / 1 / 2 / 3 puis contenu, chaîne terminée par `0xFF`) dans `hp`, `lq`, `hy` ; « BOOL 2 + 0/1 » et « la forme `0xB0` » ne sont pas la même chose ; (2) **42 lignes n'ont aucune valeur** : ce sont des mots de genre nus `(uint32_t)PICO8_VALUE_K` des aides de membre (D.11d) ; sans énumération figée, les retirer serait un alias : (c) = 42 à D.19b1, là où les étapes récentes visaient (c) = 0 ; (3) **`type-tag` baisse de 2** (`jo:138`, `hy:113` : branches `REF` jamais appelées) et **deux lignes `type-tag` se déplacent** (`hp:131`, `lq:105`) ; D.20a part de `tests/` 168 et non 170 ; (4) **cinq casts `(void *)` disparaissent** avec `pico8_value_ref` ; (5) le compilateur ne voit pas un reste de `pico8_value_t` dans un TU sans include, les en-têtes de `src/` l'incluant encore : la grep est la seule garde jusqu'à D.19c ; (6) **`iv:193-194`** : une condition sur deux lignes comptées devient une ligne (contrefactuel écrit) ; (7) parmi les 9 includes seuls, **trois sont des §6** ; (8) aucune collision de nom sauf `jc` (`fold_fixed`/`fold_string` à garde `0xDEAD`) et `hy` (`fold_string` sans mot de genre), non réutilisées ; (9) la prévision `tests/` de D.18c est **exacte** à `712f68d` ; l'arrivée est **117**, le « ≈ 115 » du plan à 2 près ; (10) des octets de genre ont déjà été écrits en **littéraux** par D.17 (`hp` `fold_qe` : `0u`/`3u`) et D.18a (`lq:206` : `3u`/`2u`) en remplaçant des appels de `fold_value` — accepté alors parce qu'aucun identifiant compté n'était remplacé ; ils côtoieront désormais `FIXED_KIND`/`REF_KIND` dans le même fichier, incohérence de forme signalée, non corrigée ici.
- **Pour D.19c** : voir « Prévision pour D.19c » ; ajouter : les TU n'ont plus besoin de `pico8_value.h` que par les en-têtes ; `test_pico8_layout_nil_item_draws_nothing` (§6) garde `src/pico8_value.c` dans sa ligne du `Makefile`.
- **Pour D.20a** : `type-tag` `tests/` 168 ; les énumérations `*_KIND` ne sont pas des étiquettes.

## Annexe A — textes recopiés (`712f68d`, par extraction de lignes)

### Lignes D.19a/b et D.19c (`docs/plan-3.md`, l. 292-293)

> - D.19a/b — TU seulement : empreintes et aides locales en plis typés, constantes inchangées. Compte : ≈ 115.
> - D.19c — `pico8_value.c/.h` et `test_pico8_value.c` supprimés. Compte : `valeur` = 0, ajouté à `DATA_RESIDUE_GUARD`. `sanitize-net` : oui.

(La ligne D.18c, `- D.18c — B10, B11, \`ou[]\`. \`sanitize-net\` : oui. Compte : ≈ 435.`, portera sa note « Découvert à D.18c » : à réextraire à la graduation, H-D18c-7.)

### Précédent D.7c et sortie D.8 (`docs/plan-3.md`, l. 255-256, extraits)

> Arbitrage du pilote : les trois octets de sorte de clé (`fold_names` de `test_pico8_hp`, `fold_qe_map` de `test_pico8_hy`, `fold_hg` de `test_pico8_jc`) ne deviennent pas des littéraux `0u`/`1u` (un identifiant compté remplacé par sa valeur est un alias) […]

> Les octets de sorte de clé de `hp` et `hy` sont pliés par une constante locale nommée (`VIEW_INDEX_KEY`, existante ; `TABLE_INDEX_KEY`, nouvelle), octet gelé d'une énumération supprimée, (c) déclarées ; aucune constante `*_HASH` ne change.

### `pico8_value.h:14-19` et `pico8_value.c:22-27` (`712f68d`)

```c
typedef enum {
    PICO8_VALUE_FIXED,   /* a PICO-8 fixed-point number */
    PICO8_VALUE_STRING,  /* a C string (not owned by the value; see below) */
    PICO8_VALUE_BOOL,    /* a boolean (0 or 1) */
    PICO8_VALUE_REF,     /* an opaque pointer/reference (e.g. an entity) */
} pico8_value_kind_t;

pico8_value_t pico8_value_bool(int b) {
    pico8_value_t v;
    v.kind = PICO8_VALUE_BOOL;
    v.as.boolean = b ? 1 : 0;
    return v;
}
```

### Modèle de mot figé en place (`test_pico8_hp.c:107-110`, `712f68d`)

```c
/* The kind the name reader gave `lq` when it fell back to the method,
 * frozen in GB_HASH; the enum entry is gone.
 * Changing it reddens them. */
enum { FUNCTION_KIND = 4 };
```

## Annexe B — reproduire

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
T='\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h'
sh tools/data_residue.sh --detail tests | grep -E '^(valeur|type-tag)'
for t in fr fv g_ ga hj jo mx nl nr os pm qh hp lq jc pj js hy mo iv mz bg trade ui entity_kind fq ie mh; do
  echo "== $t"; grep -nE "$P" tests/test_pico8_$t.c; done
grep -nE "$P" tests/*.c | grep -E "$T"                                  # mo:97-98 seules
grep -n 'fold_value(uint32_t' -A30 tests/test_pico8_{hp,lq,hy,jo}.c | grep -E "$T"   # hp:131, lq:105, hy:113, jo:138
grep -rnwE 'REF_KIND|FIXED_KIND|STRING_KIND|BOOL_KIND' src tests          # vide à 712f68d
grep -oE '#include "\.\./src/[a-z0-9_]+\.h"' tests/test_pico8_fr.c         # et suivants : pico8_fixed.h fourni par un autre en-tête
grep -n 'static uint32_t fold_\(fixed\|bool\|string\)' tests/test_pico8_{jc,js,hy}.c  # collisions de noms
```
