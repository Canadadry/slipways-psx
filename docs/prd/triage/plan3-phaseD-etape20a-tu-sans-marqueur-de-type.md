---
title: "Plan 3, étape D.20a : les TU ne lisent plus le marqueur de type — les tests d'étiquette deviennent des tests de NULL ou de classe, le mot d'étiquette plié devient la constante figée `FROZEN_STRUCT_TAG` (ou `0u` pour un pointeur non étiqueté), les tests « une sorte lit UNKNOWN » partent ; aucune constante ne change, aucun fichier de `src/`"
description: "Après D.19c, le résidu D n'est plus que `type-tag` (239 lignes). 168 d'entre elles sont dans 34 TU. D.20a les retire en deux sous-étapes (a1 : famille planète, 12 TU ; a2 : le reste, 22 TU) : 163 lignes sortent, 5 restent pour D.20b (les initialiseurs de fausses entités de `mo` et `ui`, qui doivent écrire l'étiquette tant que `src/` la vérifie). `type-tag` 239 / 57 → 161 / 45 → 76 / 25."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.19c est implémentée** (`plan-pilot`, étape 2b), sur l'arbre commis **`f3d4505`**. Cet arbre est la note de D.19a/b posée sur `07790d1` et ne touche que `docs/` : `src/`, `tests/`, le `Makefile` et `tools/` y sont ceux de `07790d1`. Seuls `grep`, `sed`, `awk` et `sh tools/data_residue.sh` ont tourné. **Rien n'a été compilé ni lancé.** Tout ce que ce PRD suppose de l'issue de D.19c est une hypothèse `H-D19c-n` (plus bas), à vérifier à la graduation de D.19c avant la mise en file.

**Numéros de ligne.** Ceux de `07790d1` pour 29 TU. Pour les cinq TU dont D.19c retire l'include de `pico8_value.h`, écrit **au-dessus** de leurs lignes `type-tag` (`jb_ol:21`, `je:24`, `nf:25`, `planet_gh_row_lq:35`, `qb:31`), l'inventaire donne directement les numéros **après D.19c**, c'est-à-dire décalés de − 1 sous l'include. Les deux sous-étapes touchent des TU disjoints : ce sont donc les numéros du parent de chaque commit. L'implémenteur remesure par contenu et explique tout écart.

Mesuré à `f3d4505` : `valeur` 117 / 47, `type-tag` **239 / 57 ; 373 occ.** (`src/` 71 / 23 / 95, **`tests/` 168 / 34 / 278**), résidu D 356 / 97. **Attendu au départ de D.20a1** (H-D19c-1) : plus de ligne `valeur` ; `type-tag` 239 / 57 ; 373 occ. inchangé ; résidu D **239 / 57**, c'est-à-dire `type-tag` seul.

Le problème, vu du mainteneur du port : la Sortie de D exige que `pico8_type_tag.h` disparaisse. Q13 a été tranchée **A** par l'humain : le marqueur de type (le mot `0x9E5C57C7` en tête de chaque entité) est retiré, on ne garde que le contrôle de classe. D.20b le retire de `src/`. Mais tant que 34 TU le nomment, D.20b devrait toucher 34 TU en plus de `src/`. Les TU le nomment pour quatre raisons :

- **tests d'étiquette** : `pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&` en tête des prédicats `is_X` et des `assert` de classe, ou seul dans un `assert`. Il répond à « est-ce une entité, et non NULL ? ». Après D.12, tout conteneur ne tient que des entités, et un pointeur typé l'est déjà : ce test n'est plus qu'un test de NULL, ou un doublon du test de classe qui le suit ;
- **mot d'étiquette plié** : les `fold_ref` et trois plis de membre écrivent `(uint32_t)pico8_type_tag_of(ref)` dans les empreintes figées. Ce mot vaut `0x9E5C57C7` pour une entité et 0 pour tout autre pointeur ;
- **tests de la couche** : neuf `assert(pico8_type_tag_of(x) == PICO8_TYPE_TAG_UNKNOWN)` vérifient qu'une sorte, un `kw` ou un vecteur n'est pas étiqueté. C'est le §6, premier point : ils partent avec ce qu'ils testent ;
- **includes** : `#include "../src/pico8_type_tag.h"` dans 34 TU.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `f3d4505` (§5, l. 294) :

> - D.20a — `type-tag`, TU seulement (a1/a2). Compte : ≈ 72.

Ligne de l'étude D.14 (§3), recopiée :

> | **D.20a** | TU seuls, `type-tag` : `assert(pico8_type_tag_of(x) == …STRUCT)` en tests de classe ; plis d'étiquette en constante figée `FROZEN_STRUCT_TAG 0x9E5C57C7u` (0 pour NULL et les non-entités, comme aujourd'hui) ; tests « une sorte lit UNKNOWN » supprimés (test de la couche, §6) ; l'étiquette reste dans `src/` | 34 fichiers : **découper** a1 (famille planète, ≈ 80 lignes) / a2 (le reste, ≈ 90) | `type-tag` ≈ 72 (`src/` seul) |

Q13 (étude, §5), tranchée **A** : « Le retirer ; on vérifie seulement la catégorie de l'objet ». Ligne directrice **G1** : « Aucune constante figée ne change dans D.15–D.20. […] Une empreinte qui bouge est une régression, pas une mise à jour. »

Legs reçus, relus ici :

- **D.19a/b** (note, PRD) : « Pour D.20a : `type-tag` `tests/` 168 / 34 ; les `*_KIND` n'en sont pas. » ; « `mo:97-98` portent `PICO8_TYPE_TAG_STRUCT` et restent `type-tag` » (numéros `96-97` à `07790d1`) ; « les lectures d'étiquette des TU (`pico8_type_tag_of` dans `fold_ref`, `FROZEN_*_TAG`, `is_lq`/`is_of_class` de `js`, asserts `PICO8_TYPE_TAG_STRUCT`) : D.20a ». Leur règle des includes s'applique aussi ici : un include part **avec le dernier usage du TU**, dans le même commit.
- **D.19a** : Ma11 (`ga` item pointeur) vert, vert aussi au parent. **D.19b1** : Mb4 (`lq` item pointeur) vert, et l'« étiquette de `fold_ref` `lq` » verte aussi au parent. **D.19b2** : Mc7 (`pj` item pointeur) vert, et `gcov` montre que `fold_item` pointeur n'est jamais exécuté. Autrement dit, **aucun TU ne plie un item à pointeur non NULL**, ce qui compte ici pour les plis d'item (point 4 de « Lecture »).
- **D.19c** (PRD, triage) : « `pico8_type_tag.h` (sauf `:10`), `pico8_struct_header_t`, l'étiquette `STRUCT`, les `FROZEN_*_TAG` et les `pico8_type_tag_of` des TU : D.20a, D.20b » ; `pico8_type_tag.h:10` est réécrit une ligne pour une ligne, donc les numéros de `:45` et `:51` ne bougent pas.
- **Précédents de constante figée** : `FROZEN_LIST_TAG 0x9E5C1157u` (D.12d), `FROZEN_INSTANCE_TAG 0x9E5C1451u` (D.8). Ce sont des `#define` locaux, définis par un littéral et précédés d'un commentaire de piège (« which the frozen hashes fold: any other value moves every *_HASH »). Ils sont recopiés dans chaque TU qui les plie.

### Hypothèses `H-D19c-n` (D.19c non gradée)

Source : le PRD de D.19c (`docs/prd/triage/plan3-phaseD-etape19c-suppression-pico8-value-garde-valeur.md` à `f3d4505`), sa Sortie et son « Effet sur `make data-residue` ». Chacune se vérifie par la commande donnée, sur l'arbre gradué `<g>` :

| Hypothèse | Énoncé | Vérification | Effet si elle tombe |
|---|---|---|---|
| H-D19c-1 | Après D.19c : plus de ligne `valeur` ; `type-tag` **239 / 57** (`src/` 71 / 23 / 95 occ., `tests/` 168 / 34 / 278 occ.) ; résidu D **239 / 57** | `make -s data-residue` ; `sh tools/data_residue.sh src` et `… tests` ; recette des occurrences | départs, arrivées absolues, lignes 1–3 des « Runs », critères `FAILED:` de départ ; les deltas (− 78, − 85) tiennent tant que H-D19c-3 tient |
| H-D19c-2 | `DATA_RESIDUE_GUARD` = `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur` | `grep -n '^DATA_RESIDUE_GUARD' Makefile` | aucune (D.20a ne change pas la garde) ; un écart est rapporté |
| H-D19c-3 | Aucune ligne `type-tag` des TU n'a changé de texte. Seuls les 9 includes de `pico8_value.h` sont retirés, et `test_pico8_value.c` est supprimé. Les numéros sont ceux de l'inventaire : ceux de `07790d1`, − 1 sous l'include dans `jb_ol`, `je`, `nf`, `planet_gh_row_lq`, `qb` | `git diff 07790d1 <g> -- tests \| grep -E '^[-+]' \| grep -E "$T"` → vide ; `git diff --stat 07790d1 <g> -- tests` → 9 TU à une ligne retirée chacun, et `test_pico8_value.c` supprimé | inventaire à remesurer ; une ligne `type-tag` qu'aurait touchée D.19c n'est plus à D.20a (écart rapporté) |
| H-D19c-4 | Aucune ligne `type-tag` de `src/` n'a changé : `pico8_struct.h` inclut toujours `pico8_type_tag.h`, `pico8_struct_header_t` est toujours `{tag, cls}`. `pico8_type_tag.h` garde ses deux `#define` (`UNKNOWN` 0, `STRUCT` `0x9E5C57C7u`) et son `inline` aux mêmes numéros (`:44-51`) | `git diff 07790d1 <g> -- src \| grep -E '^[-+]' \| grep -E "$T"` → vide ; `grep -n 'pico8_type_tag.h' src/pico8_struct.h` ; `grep -n 'return' src/pico8_type_tag.h` → `:45`, `:49`, `:51` | la règle des includes (le compilateur ne voit pas un reste) et l'ensemble `L` sont à refaire |
| H-D19c-5 | Les 34 TU gardent leurs lignes du `Makefile`. Aucun TU n'est créé ; 66 TU au total | `git diff 07790d1 <g> -- Makefile` ne touche que `test_pico8_value`, `DATA_ENGINE_SRCS`, `ENGINE_SRCS`, `main_raylib` et la ligne de `layout_nil_item_draws_nothing` ; `ls tests/test_pico8_*.c \| wc -l` → 66 | empreinte à revoir |
| H-D19c-6 | Aucune constante figée d'un TU ne change, et aucun `FROZEN_STRUCT_TAG` n'existe encore | `git diff 07790d1 <g> -- tests \| grep -E '_HASH\|FROZEN_'` → vide ; `git grep -n FROZEN_STRUCT_TAG` → vide | arrêt, rapporté |
| H-D19c-7 | Les lignes D.20a et D.20b du plan sont celles recopiées ici. La note de D.19c dit « `type-tag` 239 / 57, résidu D 239 / 57 » | `grep -n '^- D.20[ab]' docs/plan-3.md` | Annexe A à reprendre |
| H-D19c-8 | D.19c est faite en deux commits, en ≈ 30 min chacun | journal de D.19c | budget à recalibrer |

### Sections à revérifier à la graduation de D.19c

1. « État de départ », critères `FAILED:` de départ, lignes 1–3 des « Runs » (H-D19c-1).
2. Inventaire : numéros des cinq TU décalés, texte des 168 lignes (H-D19c-3).
3. Ensemble `L` attendu (H-D19c-4 : `pico8_type_tag.h:45`, `:51`).
4. Budget (H-D19c-8).

### Lecture de la ligne pour D.20a

1. **Test d'étiquette → test de NULL ou de classe** (G, Q13 = A). `pico8_type_tag_of(x) == PICO8_TYPE_TAG_STRUCT` est vrai si et seulement si `x` est non NULL et pointe une entité. Là où `x` est garanti être une entité ou NULL, le test équivaut donc à `x != NULL`. C'est le cas d'un pointeur typé, d'un élément de `hv`, `cv` ou `cg` (que `src/` assert « holds entities only »), ou d'un `held` de `pico8_down_t`. **Trois formes :**
   - **(a1) remplacé** : la moitié d'étiquette devient `x != NULL` quand rien d'autre sur la ligne ne garde le NULL. C'est le cas des prédicats `return pico8_type_tag_of(e) == … &&`, des `if (…tag… &&` et des `assert(…tag… && cls…)`. Un `assert` d'étiquette **seul** devient un test de classe quand aucun ne suit (`ie:693`, `iv:87`, `jb_ol:85-86`), ou `x != NULL` quand la ligne suivante lit déjà la classe (`fm:156`, `hy:656`, `ke_of` de `jc:415` et `ui:833`, `iv:256-257`, `mh:340-341`, qui n'ont pas d'accesseur de classe dans `src/`).
   - **(a2) retiré** : la moitié d'étiquette est retirée sans remplaçant quand la même condition teste déjà `x != NULL` (`jc:119`, `jo:303-335`, `qh:471-495`, `lq:69`…), ou quand `x` est non NULL par contrat (`hy:287`, éléments de `hv`). La ligne peut survivre avec son autre moitié. Un `assert` d'étiquette seul dont la ligne précédente asserte `!= NULL` et la suivante la classe est lui aussi retiré (`bg:320`, `bg:326`).
   - **Piège** : retirer ce test sur un pointeur qui peut viser autre chose qu'une entité ferait lire `->cls` hors de l'objet : un vecteur fait 8 octets, et `cls` est à l'offset 8. C'est pourquoi la **sonde d'étiquette** (« Testing Decisions ») relève, au parent, ce que chaque site voit réellement. Un site de test qui voit un pointeur non NULL **non étiqueté** est un arrêt « site sans règle ».
2. **Mot plié → constante figée** (G, étude). Le mot que les empreintes figées ont plié ne dépend plus du marqueur de `src/` : c'est un octet gelé. Dans chaque TU qui plie une entité, on écrit **`#define FROZEN_STRUCT_TAG 0x9E5C57C7u`**, par un littéral, jamais par `PICO8_TYPE_TAG_STRUCT`, avec le commentaire de piège de `FROZEN_LIST_TAG` (« The tag entity structs carried, which the frozen hashes fold: any other value moves every *_HASH. »). La ligne pliée devient `fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG)` : **classe (c)**, même mot, autre nom, et rien d'autre. Là où le pli ne voit que des pointeurs non étiquetés (vecteur, `qz_box`, rangée, séquence, `lq_item`, `pj_ea`/`pj_ou`), il écrit **`0u`**, le mot que ces pointeurs ont toujours plié : **(a1)**. Aucun identifiant compté n'est remplacé par sa valeur : c'est le résultat d'un calcul, prouvé par la sonde, qui est gelé. **Rejeté** : `#define FROZEN_STRUCT_TAG PICO8_TYPE_TAG_STRUCT` (un alias, qui disparaîtrait à D.20b et ferait bouger les empreintes) ; un en-tête de test partagé (le précédent est par TU).
3. **Plis mêlés** (G) : un TU dont la `fold_ref` reçoit, **selon la sonde**, à la fois des entités et des pointeurs non étiquetés est **coupé selon le type statique de l'appelant**. `fold_ref` garde ses branches (`0xDEAD`, rang dans `hv`, cible jetable `FROZEN_INSTANCE_TAG`) pour l'une des deux familles. Une aide nouvelle, `fold_entity` ou `fold_untagged`, prend l'autre, avec les mêmes branches NULL et `0xEEEE`, et elle seule écrit l'autre mot. L'implémenteur choisit le sens qui change le moins de lignes, et le déclare. Prévision (lecture des types, `f3d4505`) : **`g_`** (`fold_ref_member` de `nu`/`k_`, des séquences, contre `fold_et`, une entité) ; **`ga`** (`fold_ref_member(jo.lq)`, une entité hors de `hv`, contre `lq->ra`, un vecteur, et les items) ; **`pj`** (`pj->ed`, un `mh`, contre `pj_ea()`, `pj_ou()`, les rangées `gs`/`i_`, les vecteurs `cr->hi`/`gq` et les items). Si la sonde ne montre qu'un genre, il n'y a pas de coupe, et le mot est écrit en place. Dans ces trois TU, la classe de la ligne pliée ((c) ou (a1)) dépend du sens choisi ; le total des 19 lignes pliées est fixe.
4. **Items à pointeur** (G, question ouverte 1) : `pico8_jb_item_t.ref` est un `const void *`. `src/` y met soit une rangée (lue par `pico8_lq_cell_row`, `layout.c:54`), soit une entité `lq` ou un vecteur (`score_panel_display.c:224-225`). Aucun TU n'en plie un non NULL (legs D.19a/b1/b2). **Règle** : le pli d'item (`ga:456`, `hp:232`, `lq:131`, `pj:142`) écrit `0u`, le mot d'un pointeur non étiqueté, comme le lit `layout`. Il reste gardé par la sonde : si un TU plie un item à pointeur non NULL, c'est un arrêt, rapporté. Écart déclaré, borné à ces quatre chemins jamais pliés.
5. **Tests de la couche** (§6, premier point) : `assert(pico8_type_tag_of(…) == PICO8_TYPE_TAG_UNKNOWN)` dans `fold_kw` (`hj`, `jo`, `mx`, `nl`, `os`, `pm`, `qh`), `fold_member_vec` (`js:1119`) et `test_kind_reader_serves_the_generated_defaults` (`entity_kind:164`). Ils sont retirés sans remplaçant, puisque le pointeur est déjà typé (`const pico8_kw_t *`, `const pico8_vec_t *`, sorte). Dans `entity_kind`, la boucle `for` qui ne contenait que cet `assert` part avec lui, avec sa variable `i` (orpheline sous `-Werror`, déclarée). Le commentaire `:158` perd la proposition « and pico8_type_tag_of() reads a kind UNKNOWN » : une ligne réécrite, sans nom compté.
6. **Initialiseurs de fausses entités : laissés à D.20b** (G, question ouverte 2). `mo:96`, `mo:97` (`ra_only_t near_e/far_e = {{PICO8_TYPE_TAG_STRUCT, &g_ra_only_class}, NULL}`) et `ui:762` (`g_giver`) écrivent le marqueur dans l'en-tête d'une entité que `src/` vérifie encore (`entity_geom.c:31`, `gd`). Y mettre `FROZEN_STRUCT_TAG` serait un **alias** : la valeur doit être égale au marqueur vivant de `src/`, ce n'est pas un octet gelé d'empreinte. Y mettre 0 rougirait les `assert` de `src/`. Ces trois lignes, et les includes `mo:23` et `ui:54` qui les nourrissent, **suivent leur symbole à D.20b** (règle commune : un TU peut être converti plus tôt que son symbole, jamais plus tard). À D.20b, l'en-tête devient `{&cls}`. **5 lignes restent** : c'est l'écart au « ≈ 72 (`src/` seul) » de l'étude, qui ne comptait pas ces écritures.
7. **`js:1610`** : `o != NULL && o->tag == PICO8_TYPE_TAG_STRUCT && o->cls == pico8_ha_pz_struct_class()` perd sa moitié d'étiquette (a2). Après D.20a, plus aucun TU ne lit le membre `tag` (`git grep -nE '(->|\.)tag\b' -- tests` vide), et D.20b peut le retirer sans toucher un TU.
8. **Includes** (G, règle de D.15b/D.18b/D.12b2/D.19a/b) : chaque TU converti retire `#include "../src/pico8_type_tag.h"` **dans le commit de sa dernière ligne `type-tag`**. Cela fait 12 includes à D.20a1 et 20 à D.20a2 ; `mo` et `ui` gardent le leur (point 6). Aucun autre include n'est touché ni ajouté. **Piège** : `src/pico8_struct.h` inclut `pico8_type_tag.h` (H-D19c-4), donc un TU sans son include **voit encore** `pico8_type_tag_of` et `PICO8_TYPE_TAG_*`. Le compilateur ne signale pas un reste, seule la grep de la Sortie le fait. Vérifié à `f3d4505` : chacun des 32 TU inclut `pico8_struct.h` ou `<string.h>` directement, et ceux qui appellent `memcpy`/`strcmp`/`strlen` incluent `<string.h>`. Aucun ne perd donc une déclaration.
9. **`-Werror`** : partent comme orphelins, déclarés, la boucle et la variable `i` de `entity_kind`, et toute aide que le compilateur montre sans appel. Aucune autre n'est attendue : `rank_in`, `listed`, `fold_ref` et les prédicats `is_X` gardent leurs appelants.
10. **Ce qui ne change pas** : `src/` (aucune ligne), `Makefile`, `tools/`, `tests/e2e/`, tout `*.sha256`, toute constante `*_HASH`/`identity[]`, `FROZEN_LIST_TAG`, `FROZEN_INSTANCE_TAG`, `VIEW_*`, `TABLE_INDEX_KEY`, `*_KIND`, les 32 TU qui ne nomment pas le module, les lignes `mo:23`, `:96`, `:97`, `ui:54`, `:762`.
11. **« Compte : ≈ 72 »** : l'arrivée mesurée ici est **76 / 25** (`src/` 71 / 23 inchangé, `tests/` 5 / 2). L'écart de 4 à « ≈ 72 » est formé des 5 lignes du point 6, moins la ligne de `src/` partie à D.18b (`pico8_js_is_lq`). Le « ≈ » n'est pas un critère ; les deltas exacts le sont (− 78 / − 132 occ. ; − 85 / − 141 occ.).
12. **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; jamais l'appli raylib. Sont interdits : jonction de lignes comptées, alias, renommage pour échapper aux greps, cast nouveau, lecture du premier mot d'une entité par un autre chemin (`memcpy`, `*(const uint32_t *)`), pont, aide laissée sans appel. Les mutations se font dans une copie jetable, ou sont restaurées par `cp` simple puis `touch`. Les binaires sont lancés exactement comme le `Makefile` les lance.

### État de départ

| Mesure | À `f3d4505` (D.19c non commencée) | Départ de D.20a1 (H-D19c-1) |
|---|---|---|
| `valeur` | 117 / 47 ; 158 occ. | **0** (gardée) |
| `type-tag` (lignes / fichiers ; occ.) | 239 / 57 ; 373 | **239 / 57 ; 373** (`src/` 71 / 23 / 95, `tests/` 168 / 34 / 278) |
| résidu D | 356 / 97 | **239 / 57** |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | `…,ref-audit,valeur`, inchangé par D.20a |

Recette des occurrences (celle de D.12 à D.19, motif `type-tag` de `tools/data_residue.sh:32`) :

```sh
T='\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'type-tag\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$T")" \
  "$(grep -lE "$T" $F | wc -l)" "$(cat $F | grep -oE "$T" | wc -l)"
for f in $F; do n=$(grep -cE "$T" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$T" $f | wc -l)"; done
```

À `f3d4505`, dans `tests/`, le motif trouve 130 `pico8_type_tag_of`, 105 `PICO8_TYPE_TAG_STRUCT`, 9 `PICO8_TYPE_TAG_UNKNOWN` et 34 `pico8_type_tag.h`, soit 278 occurrences.

### Inventaire (168 lignes de `tests/`)

Classes du message de commit, lues pour ces TU :

- **(a1)** même site, contrôle typé à la place : `x != NULL` à la place de l'étiquette, test de classe là où aucun ne suivait, `0u` (marqué ⁰) dans un pli qui ne voit que des pointeurs non étiquetés ;
- **(a2)** retirée sans remplaçant au site : include ; moitié d'étiquette d'une condition déjà gardée par `!= NULL` ou dont l'argument est non NULL par contrat ; `assert` d'étiquette redondant ; test de la couche `== UNKNOWN` et le commentaire qui le décrit ;
- **(b)** signature d'une aide locale dont le type change : **aucune** (`fold_ref` garde `const void *` ; une aide coupée est nouvelle et n'est pas comptée) ;
- **(c)** même donnée, autre nom : **le mot d'étiquette plié → `FROZEN_STRUCT_TAG`, et rien d'autre** ;
- **(R)** reste, pour D.20b.

Chaque ligne listée hors (R) **sort** de `type-tag`, et aucune n'en gagne. La colonne `Lignes` compte les lignes et `occ.` les occurrences. La répartition (c) / (a1)⁰ des 19 lignes pliées est une **prévision** que la sonde confirme (points 2 à 4 de « Lecture »).

#### D.20a1 — famille planète (12 TU, 78 lignes, 132 occ.)

| TU | Lignes (occ.) | (a1) | (a2) | (c) |
|---|---|---|---|---|
| `fr` | 4 (5) | `:550` | `:40` (incl.) | `:125` (appelée seulement avec NULL : type statique entité), `:494` (`jc.held`, asserté non NULL `jc`) |
| `fv` | 4 (6) | `:167`⁰ (appelants : `fold_ref_member` de vecteurs, `qz_box`, rangée, séquences), `:277`, `:544` | `:35` (incl.) | — |
| `g_` | 7 (12) | `:109`, `:295`, `:424`, `:704`, `:747` | `:39` (incl.) | `:130` (coupe prévue : `fold_ref_member` → `fold_untagged`) |
| `ga` | 4 (6) | `:675` | `:37` (incl.), `:496` (`lq != NULL &&` déjà là) | `:127` (coupe prévue : `:456` items et `:513` `lq->ra` → `fold_untagged`) |
| `hj` | 6 (10) | `:122`, `:547`, `:618` | `:38` (incl.), `:151` (UNKNOWN) | `:137` (`probe->et`) |
| `jo` | 13 (25) | `:539`, `:571` | `:36` (incl.), `:139` (UNKNOWN), `:303`, `:307`, `:311`, `:315`, `:319`, `:323`, `:327`, `:331`, `:335` (`e != NULL &&` déjà là) | — |
| `mx` | 4 (6) | `:640` | `:40` (incl.), `:140` (UNKNOWN) | `:126` (NULL seulement) |
| `nl` | 4 (6) | `:496` | `:36` (incl.), `:135` (UNKNOWN) | `:121` (`probe->et`) |
| `nr` | 5 (7) | `:527`, `:557` | `:38` (incl.) | `:123` (NULL seulement), `:480` (`jc.held`) |
| `os` | 5 (8) | `:120`, `:548` | `:37` (incl.), `:149` (UNKNOWN) | `:135` (`probe->et`) |
| `pm` | 3 (5) | `:531` | `:37` (incl.), `:126` (UNKNOWN) | — |
| `qh` | 19 (36) | `:137`, `:630`, `:671`, `:676`, `:681`, `:686`, `:691`, `:696`, `:700` | `:37` (incl.), `:176` (UNKNOWN), `:471`, `:475`, `:479`, `:483`, `:487`, `:491`, `:495` | `:152` (`probe->et`) |
| **Total** | **78 (132)** | **31** (dont 1⁰) | **36** (12 includes, 7 UNKNOWN, 17 moitiés) | **11** |

Formes :

- prédicat `return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&` → `return e != NULL &&`, la ligne `->cls ==` suivante inchangée ;
- `assert(pico8_type_tag_of(x) == PICO8_TYPE_TAG_STRUCT &&` → `assert(x != NULL &&` ;
- `if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&` → `if (e != NULL &&` ;
- `fnv(fnv(h, 0xEEEEu), (uint32_t)pico8_type_tag_of(ref))` → `fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG)` ou `… 0u)` ;
- `fr:494`, `nr:480` → `h = fnv(h, FROZEN_STRUCT_TAG);` (`jc.held` est asserté non NULL et de classe `jc` deux lignes plus haut).

`FROZEN_STRUCT_TAG` est défini dans `fr`, `g_`, `ga`, `hj`, `mx`, `nl`, `nr`, `os`, `qh`. Lignes non comptées modifiées, attendu ≈ 10 : les `#define` et leurs commentaires, `fold_ref_member` de `g_`, `ga:456`, `ga:513`, l'aide coupée.

#### D.20a2 — autres TU (22 TU, 90 lignes, 146 occ. ; 85 / 141 sortent)

| TU | Lignes (occ.) | (a1) | (a2) | (c) | (R) |
|---|---|---|---|---|---|
| `bg` | 6 (11) | `:454`, `:458`, `:512` | `:35` (incl.), `:320`, `:326` (redondants : `held != NULL` avant, classe après) | — | — |
| `entity_kind` | 4 (6) | — | `:28` (incl.), `:150` (`made != NULL &&` déjà là), `:158` (commentaire), `:164` (UNKNOWN) | — | — |
| `fm` | 2 (3) | `:156` → `assert(provisional != NULL);` | `:33` (incl.) | — | — |
| `fq` | 2 (3) | `:267` | `:24` (incl.) | — | — |
| `hp` | 6 (10) | `:135`⁰ (vecteur `ra`, `lq_item`, items), `:283`, `:766`, `:948`, `:960` | `:40` (incl.) | — | — |
| `hy` | 6 (10) | `:652`, `:656` → `pico8_fm() != NULL`, `:667` | `:38` (incl.), `:287` (éléments de `hv`) | `:286` | — |
| `ie` | 4 (7) | `:607`, `:674`, `:693` → `view.header.cls == pico8_ie_ke_view_struct_class()` | `:37` (incl.) | — | — |
| `iv` | 4 (7) | `:87` → `pico8_je() != NULL && pico8_je()->cls == pico8_nf_struct_class()` (il garde le cast en `pico8_nf_t *` qui suit), `:256`, `:257` → `!= NULL` | `:27` (incl.) | — | — |
| `jb_ol` | 3 (5) | `:85`, `:86` → `->header.cls == pico8_jb_struct_class()` | `:20` (incl.) | — | — |
| `jc` | 6 (11) | `:415` (`ke_of` → `e != NULL`), `:875` | `:35` (incl.), `:119`, `:124`, `:983` | — | — |
| `je` | 2 (3) | — | `:22` (incl.), `:138` | — | — |
| `js` | 10 (15) | `:112`, `:190`, `:1070` | `:42` (incl.), `:1019`, `:1119` (UNKNOWN), `:1610` (`o->tag`) | `:213`, `:290`, `:1108` (membres `pico8_lq_t *`, `pico8_jb_t *`, `pico8_jo_t *`, `pico8_hp_t *`) | — |
| `lq` | 5 (8) | `:112`⁰ (items seulement), `:921` | `:38` (incl.), `:69`, `:730` | — | — |
| `mh` | 3 (5) | `:340`, `:341` → `!= NULL` | `:29` (incl.) | — | — |
| `mo` | 4 (5) | — | `:154` | — | `:23` (incl.), `:96`, `:97` |
| `mz` | 5 (9) | `:95`, `:101`, `:137`, `:767` | `:36` (incl.) | — | — |
| `nf` | 2 (3) | `:261` | `:23` (incl.) | — | — |
| `pj` | 4 (6) | `:104`, `:303`, `:199`⁰ (coupe prévue : `pj->ed` → `fold_entity`) | `:38` (incl.) | — | — |
| `planet_gh_row_lq` (§6) | 2 (3) | `:134` → `lq != NULL && lq->header.cls == …` | `:32` (incl.) | — | — |
| `qb` | 3 (5) | `:309`, `:354` | `:29` (incl.) | — | — |
| `trade` | 3 (5) | `:456`, `:457` | `:36` (incl.) | — | — |
| `ui` | 4 (6) | `:200`, `:833` (`ke_of` → `e != NULL`) | — | — | `:54` (incl.), `:762` |
| **Total** | **90 (146)** | **45** (dont 3⁰) | **36** (20 includes, 3 UNKNOWN dont 1 commentaire, 13 moitiés ou redondants) | **4** | **5 (5)** |

`FROZEN_STRUCT_TAG` est défini dans `hy`, `js` et `pj` (pour `fold_entity`). Numéros de `jb_ol`, `je`, `nf`, `planet_gh_row_lq`, `qb` : après D.19c (− 1 par rapport à `07790d1`). Lignes non comptées modifiées, attendu ≈ 12 : `#define` et commentaires, la boucle d'`entity_kind` (3 lignes et `int i;`), `pj:220` et l'aide coupée.

#### Totaux

| Sous-étape | Lignes sorties (occ.) | (a1) | (a2) | dont includes | (b) | (c) | (R) |
|---|---|---|---|---|---|---|---|
| D.20a1 | 78 (132) | 31 | 36 | 12 | 0 | 11 | 0 |
| D.20a2 | 85 (141) | 45 | 36 | 20 | 0 | 4 | 5 lignes / 5 occ. restent |
| **D.20a** | **163 (273)** | **76** | **72** | **32** | **0** | **15** | **5** |

(a1)⁰ + (c) = 19, somme fixe. **Δlignes > Δocc.** : aucun fichier. Chaque ligne sortie porte au moins une occurrence : les lignes pliées et les includes en portent 1, les tests d'étiquette 2. **Lignes comptées modifiées, non retirées** : aucune ; les 5 (R) ne sont pas touchées. **Lignes qui gagnent une catégorie** : aucune. `FROZEN_STRUCT_TAG`, `fold_entity` et `fold_untagged` ne correspondent à aucun des dix motifs.

## Solution

**Deux commits** `refactor(D.20a1)` et `refactor(D.20a2)`, chacun vert seul, vérifié et poussé, dans cet ordre. Ils touchent des TU disjoints et ne dépendent pas l'un de l'autre ; l'ordre suit l'étude. Chacun ne touche que les TU de sa sous-étape dans `tests/` et, pour la note, `docs/plan-3.md`.

1. Chaque TU teste « non NULL » ou « de telle classe » là où il testait l'étiquette.
2. Chaque empreinte plie le même mot qu'avant : `FROZEN_STRUCT_TAG` pour une entité, `0u` pour un pointeur non étiqueté.
3. Les tests « n'est pas étiqueté » sont partis avec la couche qu'ils testaient.
4. Chaque TU converti perd `#include "../src/pico8_type_tag.h"`.
5. Toutes les constantes figées sont identiques ; `src/` est identique.

**Sortie** de chaque sous-étape, constatée par commande :

- `make test` vert (dont `data-guard`, `constants-check`, `float-grep`, `float-guard` ; `tools/float_guard.txt` inchangé) ; `make e2e` **26 conformes** ; `make sanitize-net` et `make class-coverage-net` verts, sans changement de leurs fichiers attendus ; balayage ASan+UBSan des TU de la sous-étape propre (tous les TU à D.20a2) ;
- `make data-residue` :

| Mesure | Départ (H-D19c-1) | Après D.20a1 | Après D.20a2 |
|---|---|---|---|
| `type-tag` | 239 / 57 ; 373 occ. | **161 / 45 ; 241** | **76 / 25 ; 100** |
| `type-tag` `src/` | 71 / 23 ; 95 | 71 / 23 ; 95 | 71 / 23 ; 95 |
| `type-tag` `tests/` | 168 / 34 ; 278 | 90 / 22 ; 146 | **5 / 2 ; 5** |
| résidu D | 239 / 57 | **161 / 45** | **76 / 25** |

  Si le départ réel diffère : départ réel − 78 / − 132 occ., puis − 85 / − 141.
- les commandes suivantes rendent exactement ce qui est écrit (`$U` : les TU de la sous-étape ; `<parent>` : le parent du commit) :

  ```sh
  T='\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h'
  # 1. TU convertis : 0 pour chacun, sauf mo 3 et ui 2 (après D.20a2)
  for t in $U; do printf '%s %s\n' $t "$(grep -cE "$T" tests/test_pico8_$t.c)"; done
  # 2. src/, Makefile, tools/, e2e, sha256 : vide
  git diff --stat <parent> -- src Makefile tools tests/e2e; git diff --name-only <parent> | grep -E '\.sha256$'
  # 3. aucune constante figée changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[|FROZEN_(LIST|INSTANCE)_TAG|VIEW_[A-Z_]+ =|TABLE_INDEX_KEY =|_KIND = )' | grep -vE '^[-+]\s*(/\*|\*)'
  # 4. includes : exactement une ligne '-' par TU converti (pico8_type_tag.h), aucune '+'
  git diff <parent> -- tests | grep -E '^[-+]\s*#include'
  # 5. constante figée : une seule définition, par un littéral, utilisée seulement dans des plis
  grep -hE '#define FROZEN_STRUCT_TAG' tests/*.c | sort -u          # une ligne : #define FROZEN_STRUCT_TAG 0x9E5C57C7u
  grep -nE 'FROZEN_STRUCT_TAG' tests/*.c | grep -vE '#define|fnv\('  # vide
  grep -n '0x9E5C57C7' tests/*.c | grep -v '#define FROZEN_STRUCT_TAG'  # vide
  # 6. plus aucune lecture du marqueur : vide (mo et ui n'écrivent que par initialiseur positionnel)
  grep -nE 'pico8_type_tag_of|PICO8_TYPE_TAG_UNKNOWN|(->|\.)tag\b' tests/*.c
  git diff <parent> -- tests | grep -E '^\+' | grep -E 'memcpy|\*\s*\(\s*const\s+(uint32_t|unsigned)'
  # 7. aucun cast nouveau sur les lignes '+' : vide
  git diff <parent> -- tests | grep -E '^\+' | grep -E '\(\s*(const\s+)?void\s*\*\s*\)|\(\s*(const\s+)?[a-z0-9_]+_t\s*\*\s*\)\s*[^{ ]'
  # 8. comptes
  make -s data-residue; sh tools/data_residue.sh --detail tests | grep '^type-tag'
  ```

  (grep 6 : après D.20a1, les TU de a2 nomment encore `pico8_type_tag_of`. On limite alors la commande à `$U`. grep 7 : un `(const pico8_struct_header_t *)` sur une ligne `+` n'est permis que s'il était déjà sur la ligne `-` correspondante, comme `hy:287`.)

## User Stories

1. En tant que mainteneur du port, je veux que plus aucun TU ne lise le marqueur de type, afin que D.20b le retire de `src/` sans toucher un TU au-delà de `mo` et `ui`.
2. En tant que gardien des tests, je veux que chaque test d'étiquette devienne le test qu'il faisait vraiment (non NULL, ou telle classe), afin qu'aucun contrôle ne soit perdu.
3. En tant que gardien des tests, je veux qu'un test d'étiquette seul, sans test de classe à la suite, devienne un test de classe, afin que Q13 = A (« on vérifie seulement la catégorie de l'objet ») soit appliquée à la lettre.
4. En tant que gardien des empreintes, je veux que chaque pli écrive le même mot qu'avant, par une constante figée définie par un littéral, afin qu'aucune empreinte ne dépende du marqueur que D.20b supprime.
5. En tant que gardien des empreintes, je veux que le mot plié pour un pointeur non étiqueté reste 0, afin que les empreintes des vecteurs, rangées et séquences ne bougent pas.
6. En tant que gardien de la mémoire, je veux une sonde au parent qui relève ce que chaque site d'étiquette voit, afin qu'aucun test d'étiquette ne soit retiré devant un pointeur qui pourrait ne pas être une entité.
7. En tant que mainteneur, je veux qu'une `fold_ref` qui reçoit des entités et des non-entités soit coupée selon le type de l'appelant, afin qu'aucune aide ne devine le genre d'un pointeur.
8. En tant que gardien du §6, je veux que les tests « une sorte lit UNKNOWN » partent comme tests de la couche, et que `planet_gh_row_lq` garde sa preuve rouge, afin de ne supprimer aucun test de régression.
9. En tant que gardien du compteur, je veux que les initialiseurs de `mo` et `ui` restent jusqu'à D.20b, afin qu'aucune constante locale ne double le marqueur vivant de `src/`.
10. En tant que mainteneur, je veux qu'un TU perde son include dans le commit de sa dernière ligne `type-tag`, afin d'appliquer la règle de D.19a/b.
11. En tant que gardien de `-Werror`, je veux que la boucle et la variable devenues sans objet partent et soient nommées au message, afin qu'aucun orphelin ne reste.
12. En tant que gardien des tests, je veux des mutations sur chaque constante figée, chaque `0u` et chaque test de classe neuf, avec la règle du parent, afin de prouver que les plis et les contrôles mordent encore.
13. En tant que vérificateur, je veux `gcov` (avec `-D_exit=exit` pour les TU qui forkent) et l'ensemble `L` limité à `pico8_type_tag.h`, afin de prouver qu'aucune ligne de `src/` n'a perdu son seul test.
14. En tant que gardien des filets, je veux `make test`, `make e2e` (26 conformes), `sanitize-net`, `class-coverage-net` et un balayage ASan+UBSan des TU, afin de prouver qu'aucun `->cls` n'est lu hors d'un objet.
15. En tant que gardien du compteur, je veux au message de chaque commit les comptes par catégorie et par fichier, en lignes et occurrences, la classification ligne à ligne avec les numéros du parent, les lignes qui gagnent une catégorie (« aucune » compris), le résultat de la sonde et le sens des coupes, afin qu'aucune baisse ne soit un tassement.
16. En tant que `plan-pilot`, je veux deux sous-étapes d'environ 30 minutes, un plafond de 50 minutes, un repli écrit et une note « Découvert à D.20a1/a2 », afin que D.20b parte d'un état exact.
17. En tant que lecteur du plan, je veux que la dernière note dise ce qui reste pour D.20b (`src/` 71 / 23, `mo` 3, `ui` 2), afin que la dernière étape de D parte de comptes justes.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules, tests et découpage ; ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, orphelins du diff retirés, critères vérifiables) et marqués **(G)**. Q12 (oui) et Q13 (A) sont des décisions humaines, et l'étude D.14 fait foi sauf écart déclaré. Les écarts à l'étude sont : 5 lignes laissées à D.20b ; une sonde exigée ; les plis d'item en `0u` ; des coupes de `fold_ref` dans `g_`, `ga`, `pj`.

### Modules

- **D.20a1** : `test_pico8_{fr,fv,g_,ga,hj,jo,mx,nl,nr,os,pm,qh}.c`.
- **D.20a2** : `test_pico8_{bg,entity_kind,fm,fq,hp,hy,ie,iv,jb_ol,jc,je,js,lq,mh,mo,mz,nf,pj,planet_gh_row_lq,qb,trade,ui}.c`.
- **Docs** : `docs/plan-3.md` (une note par sous-étape), rien d'autre.
- Inchangés : tout `src/`, `Makefile`, `tools/`, `tests/e2e/`, les 32 autres TU, `docs/glossaire-noms.md`, `docs/trous-de-portage.md`.

### Représentation (G)

- `#define FROZEN_STRUCT_TAG 0x9E5C57C7u`, précédé du commentaire de piège, placé à côté des autres `FROZEN_*` ou `*_HASH` du TU.
- Pli d'entité : `fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG)` ; `hy:286` : `h = fnv(h, FROZEN_STRUCT_TAG);` ; `js:290` et `js:1108` gardent leurs octets `0xA2`/`0xB5` devant.
- Pli non étiqueté : `… 0u)` ; `hp:135` : `h = fnv(h, 0u);` ; `lq:112` : `return fnv(h, 0u);`.
- Aide coupée : `static uint32_t fold_untagged(uint32_t h, const void *p)` ou `fold_entity(…)`, avec la même garde NULL (`0xDEAD`) que la `fold_ref` du TU et le même `0xEEEE`, et un commentaire d'une ligne (« A pointer that carried no tag: the frozen hashes fold 0. »).
- Tests : `x != NULL` ; test de classe par l'accesseur de `src/` existant (`pico8_X_struct_class()`), jamais un accesseur nouveau.

### Anti-tassement et message de commit (liant)

- **Interdit** : joindre des lignes comptées ; renommer un identifiant vivant pour le motif ; alias (`#define` ou constante définie par un nom du module ; `FROZEN_STRUCT_TAG` hors d'un pli) ; cast nouveau ; relire le premier mot d'une entité par un autre chemin ; toucher une constante figée, `src/`, `tests/e2e/`, un `*.sha256`, `tools/`, le `Makefile`, un include autre que celui de `pico8_type_tag.h` des TU convertis, ou les 5 lignes (R).
- **Classification exigée** (recette : `git diff -U0 <parent> -- tests/test_pico8_X.c`, lignes `-` qui portent le motif en regard des `+`). D.20a1 : **(a1) 31, (a2) 36, (b) 0, (c) 11**. D.20a2 : **(a1) 45, (a2) 36, (b) 0, (c) 4, (R) 5**. La répartition (a1)⁰ / (c) peut bouger avec la sonde et le sens des coupes (somme 19). Elle est recomptée au parent, ligne à ligne, avec les numéros du parent.
- **Mesure exigée**, avant et après, par catégorie (`type-tag` ; les neuf gardées à 0 constatées) et par fichier du diff, en lignes (`--detail`) **et** en occurrences.
- **Lignes qui gagnent une catégorie** : attendu aucune.
- **Sonde** : le tableau site → genres vus, et la décision qu'il entraîne.
- **Lignes non comptées modifiées** : leur nombre par fichier.
- **Aides créées, coupées, retirées** et les orphelins.
- **Δlignes > Δocc.** : attendu aucun fichier.

### Empreinte et découpage (G)

| Commit | TU | Chemins | `type-tag` retirées (l. / occ.) | Includes | Coût attendu |
|---|---|---|---|---|---|
| **D.20a1** | 12 (famille planète) | 12 + plan | 78 / 132 | 12 | ≈ 30 min |
| **D.20a2** | 22 (le reste) | 22 + plan | 85 / 141 | 20 | ≈ 35 min |

- **Pourquoi deux** : c'est ce que disent l'étude et le plan (« a1/a2 »). 168 lignes et 34 TU dépassent un plafond de 50 min en un commit, parce que la sonde, `gcov`, `sanitize-net` et `class-coverage-net` s'ajoutent aux éditions. **Rejeté** : trois sous-étapes. Les éditions sont plus mécaniques qu'à D.19a/b (deux formes par ligne), et D.19b1/b2 ont tenu ≈ 10 et ≈ 15 min au commit pour 110 et 70 lignes.
- **Rythme** : sonde ≈ 5 min, éditions ≈ 8–12, mutations ≈ 5, `gcov` ≈ 5, filets (`make test`, `e2e`, `sanitize-net`, `class-coverage-net`, ASan+UBSan) ≈ 6, mesures, classification, message et note ≈ 4. **Budget ≈ 30–35 min jusqu'au commit, plafond 50 min par sous-étape.**
- **Repli écrit** : au plafond sans commit, rien n'est commis, et `plan-pilot` relance la sous-étape coupée en deux, sans pont, sur des TU disjoints. **D.20a1** devient (`jo`, `qh`, `hj`, `pm` : 41 lignes, surtout des prédicats) puis (`fr`, `fv`, `g_`, `ga`, `mx`, `nl`, `nr`, `os` : 37, les plis). **D.20a2** devient (`js`, `hp`, `hy`, `lq`, `pj`, `bg`, `jc`, `mz`, `entity_kind` : 52) puis (les 13 autres : 38, dont les 5 (R)). Les comptes sont recomptés par commit.

### Ce qui ne change pas

- `src/` (aucune ligne), `Makefile`, `tools/` (dont `tools/class_coverage_net_expected.txt` et `tools/float_guard.txt`), `tests/e2e/`, tout `*.sha256`.
- Toute constante `*_HASH`/`identity[]`, `FROZEN_LIST_TAG`, `FROZEN_INSTANCE_TAG`, `VIEW_*`, `TABLE_INDEX_KEY`, `*_KIND`.
- Le comportement : aucun TU ne plie autre chose ni dans un autre ordre. Aucun test n'est ajouté ; les seuls retirés sont les 9 `assert … UNKNOWN`, sauf assertion exigée par une mutation.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : aucun pointeur non étiqueté n'est dans `hv` ; la branche `listed` d'une aide non étiquetée ne sert donc jamais. Elle est gardée si elle est déjà là.
- **H2** : les prédicats `is_X` et `ke_of` ne reçoivent que des entités ou NULL. La sonde le confirme.
- **H3** : `pico8_je()` vaut un `nf` au point `iv:87` (le cast qui suit le suppose). Si le test de classe rougit, on revient à `pico8_je() != NULL`, et l'écart est rapporté.
- **H4** : aucun TU ne plie un item à pointeur non NULL (legs de D.19). Sinon, arrêt.
- **H5** : la boucle d'`entity_kind:163-165` ne contient que l'`assert` ; `kind_of` garde d'autres appelants (`:64`, `:71`).

## Implementation Chunks

**D.20a1** (un commit)

1. Départ : graduation de D.19c, ou un descendant qui ne touche que `docs/` (H-D19c-1 à H-D19c-6). On remesure par contenu les 78 lignes.
2. Sonde d'étiquette au parent pour les 12 TU. On en tire le tableau des sites et les décisions (mot plié, coupes, arrêts).
3. `gcov` de départ, pour l'ensemble `L`.
4. Pour chaque TU, dans cet ordre : `pm`, `mx`, `nl`, `os`, `hj`, `nr`, `fr` (plis simples et `fold_kw`) ; `fv` (`0u`) ; `jo`, `qh` (prédicats et `assert`) ; `g_`, `ga` (coupes). À chaque TU : définition, lignes, include ; `make tmp/test_pico8_<t>` et lancement ; constantes inchangées.
5. Filets, mutations Md, `gcov` d'arrivée, mesures, classification, note ; **commit**, vérification, push.

**D.20a2** (un commit) — même cycle. Ordre : les TU à include et prédicats seuls (`fq`, `nf`, `je`, `qb`, `trade`, `mz`, `fm`), puis ceux qui ont des tests de classe neufs (`ie`, `iv`, `jb_ol`, `mh`), `jc`, `bg`, `entity_kind`, `lq`, `hp`, `hy`, `js`, `pj`, `planet_gh_row_lq` (§6, preuve rouge), `mo`, `ui`. Mutations Me. La note de clôture porte le reste pour D.20b.

## Testing Decisions

Un bon test juge ce que le code rend ou fait, pas la présence d'une ligne. D.20a ne change **aucune ligne de `src/`**. Ce qui est jugé : chaque TU plie **les mêmes mots** qu'avant (ses constantes figées, inchangées, en sont la preuve) ; chaque contrôle réécrit **mord** encore ; aucun test retiré n'était le seul à exécuter une ligne de `src/`.

### Sonde d'étiquette (au parent, avant toute édition)

Elle se fait dans une copie jetable du scratchpad, `Makefile` inchangé. Dans chaque TU de la sous-étape, juste après `#include "../src/pico8_type_tag.h"`, on insère :

```c
#include <stdio.h>
static pico8_type_tag_t tag_probe(int line, const void *r) {
    pico8_type_tag_t t = pico8_type_tag_of(r);
    fprintf(stderr, "TAGPROBE %d %s\n", line, r == NULL ? "NULL" : t != 0 ? "ENTITY" : "UNTAGGED");
    return t;
}
#define pico8_type_tag_of(r) tag_probe(__LINE__ - 7, (r))
```

Le `- 7` corrige le décalage de l'insertion, pour rendre les numéros du parent. On construit et on lance chaque TU comme le `Makefile`, puis `grep TAGPROBE | sort | uniq -c`. Chaque site rend l'ensemble des genres vus, parmi {NULL, ENTITY, UNTAGGED}, ou « jamais atteint ». Décisions :

| Site | Vu | Décision |
|---|---|---|
| test d'étiquette (prédicat, `assert`, `if`) | ENTITY, NULL | forme de « Lecture » 1 |
| test d'étiquette | UNTAGGED | **arrêt « site sans règle »**, rapporté (on lirait `->cls` hors de l'objet) |
| `assert … UNKNOWN` | UNTAGGED | retiré (§6) |
| mot plié | ENTITY seul | `FROZEN_STRUCT_TAG` (c) |
| mot plié | UNTAGGED seul | `0u` (a1) |
| mot plié | jamais atteint | selon le type statique des appelants : entité → `FROZEN_STRUCT_TAG` ; non-entité ou item → `0u` |
| mot plié | ENTITY et UNTAGGED | coupe selon le type statique de l'appelant (« Lecture » 3). Un appelant `const void *` qui voit les deux : **arrêt** |
| pli d'item (`ga:456`, `hp:232`, `lq:131`, `pj:142`) | non NULL | **arrêt** (H4) |

Prévision : `fr:125`, `mx:126`, `nr:123` jamais atteints ; `fr:494`, `nr:480`, `hy:286`, `js` ENTITY ; `fv:167`, `hp:135` UNTAGGED ; `lq:112` jamais atteint ; `g_:130`, `ga:127`, `pj:199` les deux (coupes) ; `hj`, `nl`, `os`, `qh:152` ENTITY ou jamais atteint (`et` peut être dans `hv`) ; aucun test d'étiquette UNTAGGED.

### TU touchés (G)

1. Suivent : les 34 TU de l'inventaire, et eux seuls.
2. **Constantes** : aucune ne change. Une constante qui rougit signale un mot mal reproduit : c'est un bug de ce commit, pas une mise à jour.
3. **§6** : `planet_gh_row_lq` change une garde (`:134`) et son include, pas son oracle. **Preuve rouge** : dans une copie jetable, on remet le bogue d'origine (le pas de copie des positionnels de la rangée d'exportation, `jo:gh`), on constate le TU rouge sous ASan, puis on restaure. `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` ne changent pas autrement.
4. **Assertion nouvelle** : seulement si une mutation exigée ne rougit pas le TU alors que la même mutation au parent le rougit (un chemin perdu).
5. **Prior art** : `FROZEN_LIST_TAG` (D.12d), `FROZEN_INSTANCE_TAG` (D.8) ; les prédicats `is_X` à test de classe de D.11d ; la sonde reprend l'idée de la table de vérité de `pi.c` (D.18c) ; D.19a/b (TU seuls, constantes identiques, sous-étapes disjointes).

### Mutations (règle du parent)

Une à la fois, **dans une copie jetable**, ou dans l'arbre avec le fichier sauvegardé par `cp` et restauré par `cp` simple puis `touch`, jamais `git checkout` ni `stash`. Le TU est lancé exactement comme le `Makefile` le lance. Chacune doit rougir le TU nommé.

**Règle du parent** : une mutation qui ne rougit rien est rejouée **au parent** sur la ligne d'origine. Pour un mot plié, on écrit `(uint32_t)pico8_type_tag_of(ref) + 1u` ; pour un test, on met la même condition fausse. Si le parent reste vert lui aussi, le chemin n'a jamais été plié ou jugé par ce TU : c'est déclaré, et il n'y a rien à ajouter. Si le parent rougit et que l'enfant reste vert, un chemin est perdu : `FAILED:`.

**D.20a1** (exigées : Md1, Md2, Md3)

- **Md1** : `FROZEN_STRUCT_TAG` → `0x9E5C57C8u`, dans chaque TU qui la définit (9 mutations). Rouge attendu : `fr`, `nr` (par `:494`/`:480`, toujours pliés) ; ailleurs, rouge ou règle du parent.
- **Md2** : `fv:167` `0u` → `1u`. Rouge attendu : `fv` (membres `ra`/`hi`/`gq` non NULL de l'aide) ; sinon règle du parent.
- **Md3** : un test de classe réécrit, `qh:671` (classe `nl` → `os`), puis `jo:571` → `qh`, puis `g_:747` → `jo` : chacun rougit son TU. Cela prouve que la ligne réécrite s'exécute encore.
- **Md4** : l'octet `0xEEEE` → `0xEEEF` de l'aide coupée de `g_`, puis de `ga` : rouge, ou règle du parent.
- **Md5** : un `x != NULL` de prédicat → `1` (`jo:539`, `qh:630`) : vert attendu, et vert au parent (le NULL n'y arrive pas) ; rapporté.

**D.20a2** (exigées : Me1, Me2, Me3, Me5)

- **Me1** : `FROZEN_STRUCT_TAG` → `0x9E5C57C8u` dans `hy` (rouge attendu : `:286` plie chaque entité neuve de `hv`), `js` (rouge ou règle du parent) et `pj` (`fold_entity`, rouge ou règle du parent).
- **Me2** : `0u` → `1u` dans `hp:135`, `lq:112` et l'aide non étiquetée de `pj`. Vert probable pour `lq` (jamais atteint), vert aussi au parent ; rouge attendu pour `hp` (`ra` d'un `lq`) ; sinon règle du parent.
- **Me3** : chaque test de classe neuf, tourné vers une autre classe : `ie:693` (→ `pico8_ie_struct_class()`), `iv:87` (→ `pico8_je_placeholder_struct_class()`), `jb_ol:85` (→ `pico8_lq_struct_class()`). Chacun rougit son TU.
- **Me4** : `ke_of` de `jc:415` et `ui:833`, `e != NULL` → `e == NULL`. Rouge attendu (l'`assert` saute) : cela prouve que l'aide s'exécute.
- **Me5** (§6) : la preuve rouge de `planet_gh_row_lq`.

**Si une mutation exigée ne rougit pas son TU sans que la règle du parent l'explique : `FAILED:`.**

### `gcov` et ensemble `L`

Dans une copie jetable du scratchpad, tous les TU sont compilés en `--coverage` (flags du `Makefile` plus `GCOV_CFLAGS`). **Les TU qui forkent** (`in_fresh_child`, sortie par `_exit` : `bg`, `fm`, `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `hy`, `ie`, `jc`, `jo`, `js`, `lq`, `mx`, `mz`, `nl`, `nr`, `os`, `pj`, `pm`, `qb`, `qh`, `trade`, `ui`) sont compilés en plus avec **`-D_exit=exit`**, sinon leurs compteurs sont perdus. Ils sont lancés comme le `Makefile` : depuis la racine, avec `ASAN_OPTIONS=detect_leaks=0` pour `planet_gh_row_lq`. On fait l'union des lignes de `src/` exécutées, au départ et à l'arrivée de chaque sous-étape.

- **`L`** = lignes de `src/` exécutées au départ et plus à l'arrivée (`src/` est identique, donc l'appariement se fait par numéro). Attendu : **au plus `pico8_type_tag.h:45`** (retour NULL) **et `:51`** (retour UNKNOWN d'un pointeur non étiqueté). Seuls les TU passaient à `pico8_type_tag_of` un NULL non gardé ou un non-entité ; justification : « le marqueur disparaît à D.20b ». **Toute autre ligne de `L` est un comportement perdu** (par exemple une ligne que seul un `assert … UNKNOWN` retiré atteignait) : arrêt, rapporté.
- Aucune ligne de `src/` n'est ajoutée ni modifiée, donc il n'y a pas de couverture de lignes neuves à prouver.

### Filets

- `make test`, `make e2e` (**26 conformes**), à chaque sous-étape. Aucun e2e n'est ajouté.
- **`make sanitize-net`** et **`make class-coverage-net`**, à chaque sous-étape : verts, attendus inchangés. `src/` ne change pas : ils prouvent que les TU convertis n'ont rien changé à ce que `src/` fait.
- **Balayage ASan+UBSan des TU** : dans une copie jetable, les TU de la sous-étape (tous les TU à D.20a2) sont compilés avec les flags du `Makefile` plus `-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer`, et lancés comme le `Makefile` avec `ASAN_OPTIONS=detect_leaks=0`. **C'est la garde du piège de « Lecture » 1** : un `->cls` lu sur un non-entité serait un débordement.
- Aucun TU ne rejoue un fichier d'événements ; rien de lent.

### Vérification, budget et tableau « Runs »

**Budget : ≈ 30–35 min de temps réel par sous-étape jusqu'au commit**, sonde, mesures, mutations, `gcov` et filets compris ; **plafond 50 min** (au-delà, repli). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| sonde (12 ou 22 TU) | < 5 min |
| `make tmp/test_pico8_<t>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute |
| `make e2e` | ~1 s |
| `make sanitize-net`, `make class-coverage-net` | quelques minutes |
| `gcov` départ + arrivée | ≈ 5 min |
| mutations (a1 : ≈ 17 ; a2 : ≈ 12), chacune un `make` ciblé | < 6 min |
| balayage ASan+UBSan | 2 à 5 min |
| mesures, classification, greps | < 3 min |

**Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** par sous-étape, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + état) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | graduation de D.19c, avant D.20a1 | 0 | `type-tag 239 57` ; résidu 239 / 57 (H-D19c-1) | … |
| 2 | recette `type-tag` | idem | 0 | `type-tag 239 lignes 57 fichiers 373 occ.` | … |
| 3 | `for t in fr fv g_ ga hj jo mx nl nr os pm qh; do grep -cE "$T" tests/test_pico8_$t.c; done` | idem | 0 | `4 4 7 4 6 13 4 4 5 5 3 19` | … |
| 4 | sonde | copie jetable du parent | 0 | tableau des sites | … |
| … | … | … | … | … | … |

Pour D.20a2, la ligne 3 est `bg entity_kind fm fq hp hy ie iv jb_ol jc je js lq mh mo mz nf pj planet_gh_row_lq qb trade ui` → `6 4 2 2 6 6 4 4 3 6 2 10 5 3 4 5 2 4 2 3 3 4`, avec un départ à 161 / 45 / 241. Le vérificateur réutilise le tableau et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les TU de la sous-étape et `docs/plan-3.md`.
2. **Greps** de la Sortie, exactement.
3. **Formes** : plus aucun `pico8_type_tag_of` ni `PICO8_TYPE_TAG_*` dans les TU convertis ; `FROZEN_STRUCT_TAG` défini par le littéral, seulement dans des plis ; tests de classe par les accesseurs existants ; aides coupées à une famille chacune.
4. **Sonde** : présente, avec ses décisions appliquées ; aucun test d'étiquette UNTAGGED retiré.
5. **Constantes** : grep 3 vide ; mutations rapportées.
6. **Anti-tassement** : comptes avant/après en lignes ET en occurrences, par catégorie et par fichier ; classification juste, avec numéros du parent ; (c) exactement les mots pliés ; includes (grep 4) ; les 5 (R) intactes.
7. **Preuve** : `L` ⊆ {`pico8_type_tag.h:45`, `:51`} ; mutations restaurées ; règle du parent appliquée ; `sanitize-net`, `class-coverage-net`, ASan+UBSan rapportés ; preuve rouge du §6.
8. **Commentaires** : les quatre sortes de `CLAUDE.md` ; le commentaire de piège de `FROZEN_STRUCT_TAG` ; aucun renvoi à D.20 ni au plan ; aucun commentaire neuf ne nomme `pico8_type_tag_of` ni `PICO8_TYPE_TAG_*`.
9. **Docs** : note « Découvert à D.20a… » en fin de ligne D.20a, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ de D.20a1 n'est pas la graduation de D.19c (ou un descendant qui ne touche que `docs/`), propre ; ou si `type-tag` n'y vaut pas 239 / 57 / 373 occ. et le résidu D 239 / 57, sans explication. De même pour D.20a2 (161 / 45 / 241, résidu 161 / 45), sauf écart hérité expliqué.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 perd son oracle ou sa preuve rouge ; si un site ne tombe sous aucune règle de ce PRD ; si le balayage ASan+UBSan, `sanitize-net` ou `class-coverage-net` n'est pas propre.

**Périmètre et forme**

- `FAILED:` si une ligne de `src/`, du `Makefile` ou de `tools/` change ; si un include change hors de ceux de `pico8_type_tag.h` des TU convertis ; si une des 5 lignes (R) change.
- `FAILED:` si une grep de la Sortie ne rend pas ce qui y est écrit.
- `FAILED:` si `FROZEN_STRUCT_TAG` est défini autrement que par `0x9E5C57C7u`, ou utilisé hors d'un pli ; si un test d'étiquette est retiré devant un site que la sonde montre UNTAGGED ; si une aide devine le genre d'un pointeur `const void *`.
- `FAILED:` si un cast nouveau est ajouté, si des lignes comptées sont jointes, si un identifiant compté est remplacé par un nom défini à partir d'un nom du module, ou si le premier mot d'une entité est relu par un autre chemin.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `type-tag` ne baisse pas exactement de **78 / 132 occ.** (D.20a1) puis **85 / 141** (D.20a2) depuis le départ réel, sans explication chiffrée ligne à ligne.
- `FAILED:` **si un compte baisse par tassement** : lignes comptées jointes, alias, renommage, commentaire compté réécrit hors de son sujet, pont.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : les comptes avant/après par catégorie et par fichier, **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c)/(R) ligne à ligne, avec numéros du parent ; les lignes qui gagnent une catégorie (« aucune » compris) ; Δlignes > Δocc. (attendu : aucune) ; le tableau de la sonde ; le sens de chaque coupe ; les aides créées ou retirées et les orphelins ; les mutations et la règle du parent ; `L` ; le balayage et les filets ; la preuve que `src/` est identique ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH`, `identity[]`, `FROZEN_LIST_TAG`, `FROZEN_INSTANCE_TAG`, `VIEW_*`, `TABLE_INDEX_KEY` ou `*_KIND` change.
- `FAILED:` si une mutation exigée manque, ne se restaure pas, ou ne rougit pas son TU sans que la règle du parent l'explique.
- `FAILED:` si `L` contient une ligne hors de `pico8_type_tag.h:45` et `:51`.
- `FAILED:` si le plafond est dépassé sans note (et sans repli), ou s'il manque un tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.20a1/a2 », ou si la dernière omet les comptes réels et le reste pour D.20b.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- Tout `src/` : le membre `tag`, ses 17 écritures, ses 23 tests, les 15 includes, `pico8_type_tag.h`, le commentaire `pico8_struct.h:125` (« reads it UNKNOWN ») : D.20b.
- `mo:23`, `:96`, `:97`, `ui:54`, `:762` : D.20b (« Lecture » 6).
- Mettre `type-tag` dans `DATA_RESIDUE_GUARD` : D.20b.
- Réécrire les octets déjà littéraux, unifier les aides de pli entre TU, un en-tête de test partagé.
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Découpage** : deux commits D.20a1 / D.20a2 (12 / 22 TU), ≈ 30–35 min chacun, plafond 50 ; un repli par moitiés est écrit.
  - **Tests** : étiquette → `x != NULL` ou test de classe ; tests UNKNOWN retirés (§6) ; sonde au parent comme garde.
  - **Plis** : `FROZEN_STRUCT_TAG` (littéral) pour une entité, `0u` pour un pointeur non étiqueté ; coupe selon le type de l'appelant ; items en `0u`.
  - **Includes** : retirés avec la dernière ligne du TU ; `mo` et `ui` gardent le leur.
  - **Reste** : 5 lignes pour D.20b, déclarées.
- **Questions ouvertes (pour l'humain ; les défauts ci-dessus s'appliquent s'il ne répond pas)** :
  1. *Items à pointeur.* Un élément de rangée peut pointer une rangée, une entité ou un vecteur, sans dire lequel. Aucun test n'en plie un. Défaut : ils plient « non étiqueté » (0), comme les lit `layout`, et la sonde arrête tout si un test en plie un un jour. L'autre choix serait d'ajouter à l'élément un drapeau « c'est une entité » dans `src/`, hors de D.20a.
  2. *Fausses entités de `mo` et `ui`.* Elles doivent porter le marqueur tant que `src/` le vérifie. Défaut : leurs 5 lignes attendent D.20b, qui les réécrit en `{&cls}`. L'autre choix, une constante locale égale au marqueur, serait un alias, donc refusé.
  3. *Tests de classe neufs.* `iv` et `mh` n'ont pas d'accesseur de classe dans `src/`. Défaut : on y teste « non NULL », et la liste d'appartenance qui suit fait le reste. En ajouter un toucherait `src/`.
- **Surprises de mesure, pour `plan-pilot`** :
  1. **Arrivée 76 et non ≈ 72** : 5 lignes de `tests/` restent (`mo`, `ui`) et `src/` est à 71 (`pico8_js_is_lq` parti à D.18b).
  2. **Trois `fold_ref` mêlent entités et non-entités** (`g_`, `ga`, `pj`) : coupes prévues.
  3. **Trois `fold_ref` ne sont appelées qu'avec NULL** (`fr`, `mx`, `nr`) : leur mot d'étiquette n'a jamais été plié.
  4. **`fv` ne plie que des non-entités** : son mot est 0.
  5. **Dans `entity_kind`, la boucle d'`assert` UNKNOWN part entière.**
  6. **`js:1610` est le seul TU qui lit le membre `tag`.**
  7. **Le compilateur ne voit pas un reste** : `pico8_struct.h` inclut `pico8_type_tag.h` jusqu'à D.20b, et seule la grep garde.
- **Pour D.20b** (reste mesuré à `f3d4505`, à reconfirmer à la graduation de D.20a2) : `type-tag` **76 / 25 ; 100 occ.** Dans `src/`, 71 lignes / 23 fichiers / 95 occ. :
  - 15 includes ;
  - `pico8_type_tag.h` (12 lignes comptées) ;
  - 17 écritures (`header.tag = …` et initialiseurs `{PICO8_TYPE_TAG_STRUCT, &cls}` de `economy`, `entity`, `ke`, `layout` ×2, `menu`, `planet_family_trade`, `probe` ×2, `research`, `research_icon`, `score_panel`, `slipway` ×2, `trade`, `ui`, `wormhole`) ;
  - 23 `assert`/tests `pico8_type_tag_of`, à rendre en tests de classe, ou à retirer si le pointeur est typé ;
  - le membre et 3 commentaires de `pico8_struct.h` (`:43`, `:44`, `:46`, `:125`).

  Dans `tests/`, `mo:23`, `:96`, `:97` et `ui:54`, `:762` (initialiseurs `{&cls}`, includes retirés). Les `FROZEN_STRUCT_TAG` des TU restent : ce sont des octets d'empreinte. `class-coverage-net` : oui (en-tête de tous les constructeurs). Avec `type-tag` = 0 dans `DATA_RESIDUE_GUARD`, **le résidu D est vide : c'est la Sortie de D.** L'ensemble `L` de D.20b contiendra tout `pico8_type_tag.h`, supprimé.

## Amendement (sonde D.20a1)

Sonde d'étiquette lancée au parent (8a42cba) sur les 12 TU de D.20a1. Tous passent. Les nombres entre parenthèses comptent les appels observés.

| Site | Vu | Décision |
|---|---|---|
| `fr:494` (1), `fr:550` (2), `fv:277` (4), `fv:544` (1), `g_:295` (7), `g_:424` (4), `g_:704` (3), `g_:747` (1), `ga:496` (3), `ga:675` (3), `hj:547` (2), `hj:618` (2), `jo:303`–`:335`, `jo:539` (92), `jo:571` (1), `mx:640` (2), `nl:496` (2), `nr:480` (1), `nr:527` (2), `nr:557` (1), `os:548` (2), `pm:531` (2), `qh:137` (1), `qh:471`–`:495`, `qh:630` (2), `qh:671`–`:700` | ENTITY seul | formes de « Lecture » 1 et 2, comme prévu |
| **`g_:109`** (`is_bv_target`) | **ENTITY 8, UNTAGGED 2** | test d'étiquette qui voit un pointeur non étiqueté : **arrêt « site sans règle »**, levé par la coupe ci-dessous |
| `g_:130` | UNTAGGED 2 seul (les séquences `g_->nu`, `em->k_`, via `fold_ref_member`) | coupe : ces deux appels passent à `fold_untagged` (`0u`) ; `:130` garde la famille entité, donc (c) |
| `ga:127` | **jamais atteint** (et non « les deux genres ») : `:456` et `:513` jamais appelés, `:589` NULL seul | coupe selon le type statique, comme prévu : items et `lq->ra` → `fold_untagged` ; `jo.lq` reste dans `fold_ref`, donc (c) |
| `fv:167` | UNTAGGED 4 | `0u` |
| `fr:125`, `mx:126`, `nr:123`, `hj:137`, `nl:121`, `os:135`, `qh:152` | jamais atteints | type statique : entité, donc `FROZEN_STRUCT_TAG` |
| `hj:122`, `os:120` (`is_bv_target`) | **jamais atteints** (non prévu) | forme (a1) |
| `hj:151`, `jo:139`, `mx:140`, `nl:135`, `os:149`, `pm:126`, `qh:176` | UNTAGGED | tests de la couche, retirés |

**H1 vérifiée pour `g_`** par une sonde au parent dans `fold_ref_member` : les deux appels non NULL ont `rank_in(hv) = -1`, `is_bv_target` faux et une étiquette à 0. L'empreinte qu'ils écrivent est égale à `fnv(fnv(fnv(h, 1u), 0xEEEEu), 0u)`. `fold_untagged` écrit exactement ces octets, sans appeler `is_bv_target`.

**Sonde refaite après la coupe** : `is_bv_target` de `g_` voit ENTITY 8, et rien d'autre. `fold_untagged` voit UNTAGGED 2, et `fold_ref` ENTITY 3.

**Grep 7 de la Sortie.** Pour D.20a1, sa sortie vide exclut la ligne `+    assert(fv != NULL && ((const pico8_struct_header_t *)fv)->cls == cls);`. Ce cast était déjà sur la ligne `-` (`fv:544`). La grep devient :

```sh
git diff <parent> -- tests | grep -E '^\+' | grep -E '\(\s*(const\s+)?void\s*\*\s*\)|\(\s*(const\s+)?[a-z0-9_]+_t\s*\*\s*\)\s*[^{ ]' | grep -vF 'assert(fv != NULL && ((const pico8_struct_header_t *)fv)->cls == cls);'
```

## Annexe A — textes recopiés (`f3d4505`, par extraction de lignes)

### Lignes D.20a et D.20b (`docs/plan-3.md`, l. 294-295)

> - D.20a — `type-tag`, TU seulement (a1/a2). Compte : ≈ 72.
> - D.20b — `type-tag`, `src/` : membre d'étiquette retiré (Q13 = A), `assert` d'étiquette → contrôles de classe, `pico8_type_tag.h` supprimé. Compte : `type-tag` = 0, gardé ; résidu D vide, Sortie de D atteinte. `class-coverage-net` : oui.

### `src/pico8_type_tag.h:27-52` (extrait)

```c
#define PICO8_TYPE_TAG_UNKNOWN 0x00000000u
#define PICO8_TYPE_TAG_STRUCT 0x9E5C57C7u
static inline pico8_type_tag_t pico8_type_tag_of(const void *ref) {
    pico8_type_tag_t tag;
    if (ref == NULL) {
        return PICO8_TYPE_TAG_UNKNOWN;          /* :45 */
    }
    memcpy(&tag, ref, sizeof tag);
    if (tag == PICO8_TYPE_TAG_STRUCT) {
        return tag;                             /* :49 */
    }
    return PICO8_TYPE_TAG_UNKNOWN;              /* :51 */
}
```

### Modèle de constante figée en place (`tests/test_pico8_fr.c:48-50`)

```c
/* The tag `jo:ib`'s list carried as a tagged list, which the frozen hashes
 * fold: any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u
```

## Annexe B — reproduire (lecture seule, sans build)

```sh
T='\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h'
sh tools/data_residue.sh; sh tools/data_residue.sh src; sh tools/data_residue.sh --detail tests | grep '^type-tag'
grep -nE "$T" tests/*.c                                                   # les 168 lignes
grep -ohE "$T" tests/*.c | sort | uniq -c                                 # 130 / 105 / 9 / 34
grep -nE 'fold_ref(_member)?\(' tests/test_pico8_{fr,fv,g_,ga,hj,hp,lq,mx,nl,nr,os,pj,qh,js}.c   # appelants des plis
grep -nE '^\s*(const )?[a-z0-9_ ]+\*\s*(et|ra|hi|gq|k_|kq|rb|nu|gs|i_|ed|lq|held)\s*;' src/*.h  # types des membres pliés
grep -nE '\.ref = |is_ref = 1' src/*.c                                    # producteurs d'items à pointeur
grep -nE '(->|\.)tag\b' tests/*.c                                         # js:1610 seul
grep -nE "$T" src/*.c src/*.h | grep -vE '#include|pico8_type_tag.h:|tag = PICO8|\{PICO8_TYPE_TAG_STRUCT|pico8_type_tag_of\('   # struct.h:44, :46
```
