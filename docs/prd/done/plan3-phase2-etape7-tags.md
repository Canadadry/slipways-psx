---
title: "Plan 3, étape 2.7 : les étiquettes d'une classe deviennent un masque de bits `tags` dans sa table de classe ; `get_nu`, les membres `nu`, leurs défauts et leurs tables retirés ; `cg` sur un tableau fixe (`nu` 208 → 0, gardé ; `accesseurs-q6` 262 → 237 ; `défauts` 694 → 669 ; pied 1290/88 → 1083/88) — trois commits, jeu identique"
description: "Chaque entité portait, par une copie de pointeur écrite à la construction, la liste de chaînes de sa classe (`nu`), relue par `lt`/`he` via l'accesseur `get_nu` et rangée par `cg` dans un tableau qui grandit par `realloc`/`strdup`. La liste est une constante de classe : elle devient un masque `unsigned tags` de la table de classe, sur un enum de dix étiquettes dans `struct.h` ; `cg` passe à dix seaux fixes ; l'accesseur, les membres, les défauts et les tables partent ; `nu` entre dans la garde."
status: needs-triage
---

## Problem Statement

**Écrit sur `b26fc08`** (branche `claude/plan-pilot-3-7zdft0`, après 2.6 vérifiée : pied `1290 / 88`, garde `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver`). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les essais ont été faits sur une seule copie jetable, d'abord de `65f4581` (avant 2.6), puis rebasée sur `b26fc08` ; 2.6 ne touche aucun fichier de cette étape sauf la ligne de garde du `Makefile` (voir « Recouvrement avec 2.6 »). Tous les numéros de ligne ci-dessous sont ceux de `b26fc08` (identiques à `65f4581` pour chaque fichier touché, `Makefile` compris : la garde reste en `:864`).

Vu du mainteneur du port : dans le cart, `lt(e)` range l'entité dans `cg[t]` pour chaque `t` de `e.nu`, une liste de noms de classe (`nu=o("jo","np","prober")`…). Le port recopie cette forme :

- 13 classes ont un défaut `nu` (`game_data.h:156` à `:418`), un pointeur vers une `pico8_str_seq_t` (`pico8_X_nu_tags`, `game_data.c:132-156`, `:228-247`) ; 13 constructeurs le copient sur l'instance (`self->nu = pico8_X_start.nu`) ; 5 types le portent (`jo`, `mh`, `g_`, `jc`, `mz`) ;
- l'accesseur `get_nu` (`struct.h:104`, un des onze Q6) le rend à `lt`/`he` (`entity.c:202`, `:231`), par 5 lecteurs (`planet.c:79`, `probe.c:399`, `msgbox.c:88`, `slipway.c:367`, `wormhole.c:205`) branchés dans 15 tables ;
- `cg` garde une copie `strdup` de chaque nom vu, dans un tableau qui grandit par `realloc` (`entity.c:23-33`, `:49-90`).

Or la liste est une **constante de classe** : les 13 écritures sur instance sont toutes dans un `*_new`/`*_init` (preuve compilateur ci-dessous), et aucune classe n'a deux listes. Ce sont les 208 lignes `nu` du compteur, un des onze accesseurs, et un champ qu'on écrirait sinon dans 13 archétypes (2.12) pour le voir mourir (plan, « Champs qui meurent avant les archétypes »).

**Pourquoi rien ne change.** Les dix noms du jeu sont fixes. Un masque par table de classe dit exactement ce que disait la liste de l'instance, pour chaque instance (chaque table n'est posée que par un constructeur qui écrit toujours la même liste). L'ordre des seaux de `cg` n'est pas observable (lecture par nom), et `he` sur un seau jamais vu ne fait rien, avant comme après. `pico8_cg` garde son contrat : NULL pour une étiquette qu'aucune entité n'a portée depuis `pico8_entity_init()`, étiquette inconnue comprise. Les empreintes de TU ne bougent pas (mesuré aux trois sous-étapes).

### Texte du plan, recopié (`docs/plan-3.md` à `b26fc08`, ligne 2.7)

> - 2.7 — `nu` → masque de bits, nom neuf (Q4) : `tags`, sens du glossaire (`glossaire-noms.md:48`, « étiquettes d'une classe »), enum des dix étiquettes dans le noyau (`struct.h`), dans l'ordre `qh`, `hj`, `nr`, `g_`, `mh`, `jc`, `mz`, `jo`, `np`, `prober`, qui redonne chaque séquence de `game_data.c:132-247` et donc les octets des plis de TU. Le masque vit dans la table de classe que le noyau pointe (règle 1 : aucune écriture sur une instance, preuve compilateur Q3 ; l'étude 2.9 dit s'il passe dans l'archétype). Piège de la règle 3 : le `nu` vide de `ga` (`game_data.c:150`) masque celui de `jo` — masque 0, jamais celui de `jo`. `pico8_cg(const char *)` garde son contrat : NULL pour une étiquette jamais `lt()`ée (`entity.h:33-37`), étiquette inconnue comprise ; son stockage `realloc`/`strdup` (`entity.c:69-89`) devient un tableau fixe.
>   - 2.7a — `lt`/`he` lisent le masque (`entity.c:203`, `:232`) ; `get_nu` retiré de la table (`struct.h:97`) et ses cinq lecteurs (`planet.c:79`, `probe.c:401`, `msgbox.c:88`, `slipway.c:368`, `wormhole.c:206`) avec leurs branchements ; les membres `nu` restent, morts, pour les TU. Compte : `nu` 208 → **180** ; `accesseurs-q6` 262 → **237** ; pied → **1262 / 88**. Preuves : chemin touché (`gcov` de `lt`, `he`, `cg` ; mutations : `ga` reçoit le masque de `jo`, un bit `np` perdu, `cg` non NULL pour une étiquette jamais vue) ; preuve compilateur `const` sur `nu` des cinq types porteurs. `class-coverage-net` : oui. `sanitize-net` : oui. Budget ≈ 50 min.
>   - 2.7b — les membres `nu`, les `.nu` des défauts, les `pico8_X_nu_tags`/`_nu_items` retirés ; les 17 TU plient le masque dans l'ordre de l'enum (octets inchangés). Compte : `nu` 180 → **≈ 30** (commentaires) ; `défauts` 694 → **669** ; pied → **≈ 1113 / 88**. Preuves : empreintes inchangées ; mutation : deux bits de l'enum échangés → TU rouge. `class-coverage-net` : oui (inits touchées). `sanitize-net` : non. Budget ≈ 50 min.
>   - 2.7c — commentaires qui nomment encore `nu` (30 lignes, dont 11 de `planet_family.h`), réécrits selon les quatre destinations de `CLAUDE.md`. Compte : `nu` → **0**, gardé ; pied → **1083 / 88**. Preuves : préprocesseur identique hors `__LINE__`. Budget ≈ 20 min.

Écarts du plan relevés à la mesure (détail en « Surprises ») : 2.7a rend `nu` **175 / 34** et pied **1257 / 88** (le plan : 180, 1262) ; 2.7b rend `nu` **23 / 9** et pied **1106 / 88** (le plan : ≈ 30, ≈ 1113) ; 2.7c rend exactement **1083 / 88**. Les numéros du plan ont un peu glissé (`get_nu` : `struct.h:104` ; lecteurs `probe.c:399`, `slipway.c:367`, `wormhole.c:205` ; stockage `entity.c:49-90`) ; rien d'autre ne diffère. Des 17 TU, seuls `g_` et `fv` plient des noms d'étiquette dans l'ordre ; les 15 autres plient une constante ou comparent un masque (l'ordre de l'enum ne leur importe pas).

Règles communes de la phase (plan, « Preuves communes à chaque ligne ») : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` ni `tests/e2e/` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée. Une catégorie tombée à 0 entre dans `COMPOSITION_RESIDUE_GUARD`, avec une preuve de garde mordante.

### Recouvrement avec 2.6

2.6 (déjà commise, `090ab68`/`51baccb`/`b26fc08`) a touché `src/pico8_pi.c`, `tools/line_coverage_unreachable.txt`, `tools/atteinte_a_prouver.txt`, `tests/test_pico8_hp.c` (commentaire), `docs/plan-3.md` et la ligne `Makefile:864`. **Seul recouvrement : `Makefile:864`**, que 2.7c prolonge de `,nu` (essayé : conflit de rebase attendu sur cette ligne, résolu en ajoutant `,nu` à la forme de 2.6). `docs/plan-3.md` : 2.7 n'écrit qu'au bout de la ligne 2.7. Aucun autre fichier commun.

### État de départ attendu (`b26fc08`, mesuré)

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 262 33`, `défauts 694 52`, **`nu 208 37`**, `void-ptr 11 7`, `ptrs-entité 7 4`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 1290 ligne(s), 88 fichier(s)`** |
| `sh tools/composition_residue.sh --check nu` | 208 lignes, code 1 |
| `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` | `864:COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver` |
| `--champs` | `champs : 287 ; 0 écriture : 15 ; 0 hors constructeur : 27` |
| `make test` ; `make e2e` | vert ; 26 conformes |

## Solution

Trois commits, dans l'ordre du plan, chacun vert seul.

- **2.7a — le masque naît, l'accesseur meurt.** `struct.h` reçoit l'enum `pico8_tag_t` (dix étiquettes, ordre du plan), la macro `PICO8_TAG(t)` et la recherche par nom `pico8_tag_named` ; la table de classe perd `get_nu` et gagne `unsigned tags`. Les 15 tables qui branchaient `get_nu` portent leur masque (celle de `ga` : 0, jamais celui de `jo`). `lt`/`he` parcourent les bits ; `cg` devient dix seaux fixes avec un drapeau `seen`. Les cinq lecteurs `*_get_nu` et deux prototypes partent. Les membres `nu` restent, écrits, lus par les TU seuls. Un TU neuf fixe le contrat NULL de `pico8_cg`. Le glossaire nomme `tags` (Q4 : le nom neuf naît dans le commit qui convertit).
- **2.7b — les listes meurent.** Les 5 membres `nu`, les 13 défauts `.nu`, les 13 copies à la construction, les 13 tables `pico8_X_nu_tags` (et leurs `_nu_items`) et leurs `extern` partent. Les 17 TU lisent le masque de la classe (`…->header.cls->tags`) ; `g_` et `fv` plient les noms dans l'ordre de l'enum ; les empreintes ne bougent pas.
- **2.7c — commentaires, puis garde.** Les 23 lignes de commentaire qui nomment encore `nu` sont réécrites ; `nu` entre dans `COMPOSITION_RESIDUE_GUARD`.

## User Stories

1. En tant qu'auteur des archétypes (2.12), je veux que `nu` n'existe plus sur aucune instance ni dans aucun défaut, afin de ne pas l'écrire dans 13 archétypes pour le voir mourir.
2. En tant que lecteur de `lt`/`he`, je veux lire « chaque bit du masque de la classe → son seau », afin de ne plus suivre un accesseur, une liste de chaînes et un tableau qui grandit.
3. En tant qu'appelant de `pico8_cg`, je veux le même contrat qu'avant (NULL tant qu'aucune entité n'a porté l'étiquette depuis `pico8_entity_init()`, étiquette inconnue comprise), et un TU qui le fixe, afin qu'un seau vide ne remplace jamais NULL en silence.
4. En tant que gardien du piège de la règle 3, je veux que la table de `ga` porte le masque 0, et une mutation qui lui donne celui de `jo` rouge, afin que les bursts n'entrent jamais dans `cg("jo")`.
5. En tant que gardien des empreintes, je veux toutes les constantes `*_HASH` inchangées, afin d'être sûr que chaque classe garde exactement ses étiquettes.
6. En tant que gardien du jeu, je veux `make e2e` à 26 conformes à chaque commit, aucun `*.sha256` touché.
7. En tant que vérificateur, je veux le `gcov` des e2e, l'ensemble `L`, les mutations rejouées au parent et la preuve compilateur `const`, afin de savoir qu'aucun chemin ne perd son test et qu'aucune instance n'écrivait ses étiquettes.
8. En tant que gardien du compteur, je veux `nu` dans la garde, et la preuve qu'une ligne réinjectée rougit `make test`.
9. En tant que lecteur du glossaire, je veux que les entrées `nu`, `cg` et `np` nomment `tags` et `PICO8_TAG_*`, afin de ne pas chercher `pico8_X_nu_tags`.
10. En tant que lecteur du plan, je veux une note « Découvert à 2.7 » courte, afin que 2.8 parte des vrais comptes.

## Implementation Decisions

### Modules

`struct`, `entity`, `planet`, `planet_family`, `planet_family_trade`, `probe`, `help`, `msgbox`, `slipway`, `wormhole`, `game_data` ; TU `jo` (test neuf en 2.7a), puis les 17 TU de 2.7b (`fr`, `fv`, `g_`, `ga`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `mh`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh`, `spec_defaults`) ; `Makefile` (2.7c) ; `docs/glossaire-noms.md` (2.7a) ; `docs/plan-3.md` (note). Ni `tools/composition_residue.sh`, ni `tests/test_composition_residue.c`, ni `tools/class_coverage_net_expected.txt`, ni `docs/trous-de-portage.md` (aucun écart avec le cart).

### 2.7a — ce qui change, fichier par fichier

**`src/pico8_struct.h`**
- Avant `:40` (`typedef struct pico8_struct_class pico8_struct_class_t;`), texte exact essayé :

```c
/* The ten tags `cg` indexes entities under, in the order
 * every class lists its own. */
typedef enum {
    PICO8_TAG_QH,
    PICO8_TAG_HJ,
    PICO8_TAG_NR,
    PICO8_TAG_G_,
    PICO8_TAG_MH,
    PICO8_TAG_JC,
    PICO8_TAG_MZ,
    PICO8_TAG_JO,
    PICO8_TAG_NP,
    PICO8_TAG_PROBER,
    PICO8_TAG_COUNT
} pico8_tag_t;

#define PICO8_TAG(t) (1u << (t))

/* The tag named `name`, or -1 for any other name. */
static inline int pico8_tag_named(const char *name) {
    static const char *const names[PICO8_TAG_COUNT] = {
        [PICO8_TAG_QH] = "qh", [PICO8_TAG_HJ] = "hj", [PICO8_TAG_NR] = "nr",
        [PICO8_TAG_G_] = "g_", [PICO8_TAG_MH] = "mh", [PICO8_TAG_JC] = "jc",
        [PICO8_TAG_MZ] = "mz", [PICO8_TAG_JO] = "jo", [PICO8_TAG_NP] = "np",
        [PICO8_TAG_PROBER] = "prober",
    };
    int i;

    for (i = 0; i < PICO8_TAG_COUNT; i++) {
        if (strcmp(names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}
```

- Après `:96` (`pico8_state_fn states[…];`) : `/* \`PICO8_TAG(t)\` set: lt() registers every instance under tag t. */` puis `unsigned tags;`.
- `:104` (`get_nu`) : retiré.
- **Nom : `pico8_tag_named`, pas `pico8_tag_of`.** Le motif `lien-parent` compte tout `pico8_…_of` (`composition_residue.sh:53`) : `pico8_tag_of` y ajoutait 2 lignes / 1 fichier (mesuré), sans être une lignée. Le compteur n'est pas touché.

**`src/pico8_entity.c`**
- `:23-33` (commentaire « owned copy », `char *tag;`, `g_cg`/`g_cg_count`/`g_cg_capacity`) → texte exact :

```c
/* `seen`: an entity carrying the tag has been lt()'d since
 * pico8_entity_init(); until then pico8_cg() returns NULL for it. */
typedef struct {
    int seen;
    pico8_ptrs_t list;
} pico8_cg_entry_t;

static pico8_cg_entry_t g_cg[PICO8_TAG_COUNT];
```

- `:49-90` (`pico8_entity_strdup`, `pico8_cg_find`, `pico8_cg_find_or_create` et leurs commentaires) : retirés.
- `pico8_entity_shutdown`, `:108-115` → `for (i = 0; i < PICO8_TAG_COUNT; i++) { pico8_ptrs_free(&g_cg[i].list); g_cg[i].seen = 0; }` (liste libérée, drapeau remis ; plus de `free` de nom ni de tableau).
- `pico8_cg`, `:184-185` → `int t = pico8_tag_named(tag);` puis `return t >= 0 && g_cg[t].seen ? &g_cg[t].list : NULL;`.
- `pico8_lt` : `:190` (`const pico8_str_seq_t *tags;`) retiré ; `:200-208` → `for (i = 0; i < PICO8_TAG_COUNT; i++) { if (e->cls->tags & PICO8_TAG(i)) { g_cg[i].seen = 1; pico8_ptrs_add(&g_cg[i].list, e); } }`.
- Commentaire de `pico8_he`, `:213-215` → « `e` must be unregistered under exactly the fields it was registered under: a bm field changed since lt() leaves `e` in a list this never removes it from. » (le masque ne peut plus changer entre `lt` et `he`).
- `pico8_he` : `:218` retiré ; `:229-239` → commentaire « A tag never lt()'d: Lua would error on indexing its bucket, this port deletes from an empty list. » puis `for (i = 0; i < PICO8_TAG_COUNT; i++) { if (e->cls->tags & PICO8_TAG(i)) { pico8_ptrs_del(&g_cg[i].list, e); } }`. Un seau jamais vu est une liste vide (zéro statique ou `pico8_ptrs_free`) : `pico8_ptrs_del` n'y trouve rien, comme l'ancien `bucket == NULL`.

**`src/pico8_entity.h`**, `:32-35` → « The entities registered under `tag`, or NULL if no entity carrying that tag has been lt()'d since pico8_entity_init() -- an unknown tag included. A returned pointer stays valid until the next pico8_entity_init() or pico8_entity_shutdown(). » (contrat C : la mise en garde contre `realloc` n'a plus d'objet).

**Lecteurs retirés** : `planet.c:79-82`, `probe.c:399-402`, `msgbox.c:88-91`, `slipway.c:367-370`, `wormhole.c:205-208` (fonction et ligne vide) ; prototypes `planet.h:116`, `probe.h:63`.

**Tables** : chaque ligne `.get_nu = …,` devient `.tags = …,` à la même place (les initialiseurs désignés ne dépendent pas de l'ordre). Avec `J = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)` :

| Ligne | Table | Masque | Liste du parent |
|---|---|---|---|
| `planet.c:129` | `jo` | `J` | `jo_nu_items` |
| `planet_family.c:52` | `qh` | `PICO8_TAG(PICO8_TAG_QH) \| J` | `qh_nu_items` |
| `:142` | `nl` | `J` | `nl_nu_items` |
| `:202` | `os` | `J` | `os_nu_items` |
| `:280` | `hj` | `PICO8_TAG(PICO8_TAG_HJ) \| J` | `hj_nu_items` |
| `:385` | `pm` | `J` | `pm_nu_items` |
| `:559` | `ga` | `0`, précédé de `/* ga's cart list of tags is empty and replaces jo's: never jo's. */` | `pico8_ga_nu_tags = {0, NULL}` |
| `planet_family_trade.c:183` | `nr` | `PICO8_TAG(PICO8_TAG_NR) \| PICO8_TAG(PICO8_TAG_NP) \| PICO8_TAG(PICO8_TAG_PROBER)` | `nr_nu_items` |
| `:256` | `fr` | idem `nr` | écrite par `pico8_nr_struct_init` |
| `:363` | `mx` | `J` | `mx_nu_items` |
| `msgbox.c:105` | `mh` | `PICO8_TAG(PICO8_TAG_MH)` | `mh_nu_items` |
| `probe.c:436` | `g_` | `PICO8_TAG(PICO8_TAG_G_) \| PICO8_TAG(PICO8_TAG_PROBER)` | `g__nu_items` |
| `help.c:134` | `fv` | idem `g_` | écrite par `pico8_g__struct_init` |
| `slipway.c:398` | `jc` | `PICO8_TAG(PICO8_TAG_JC)` | `jc_nu_items` |
| `wormhole.c:239` | `mz` | `PICO8_TAG(PICO8_TAG_MZ)` | `mz_nu_items` |

Chaque masque est la liste que la dernière écriture du constructeur de la classe pose (vérifié constructeur par constructeur : `nl`, `os`, `pm`, `mx`, `ga` écrasent la liste de leur init parente ; `fr` et `fv` n'écrivent rien après `nr`/`g_`). Les autres tables (22 dans `src/`, 3 de TU) n'ont pas de membre `.tags` : masque 0, comme leur `get_nu` NULL d'avant.

**`tests/test_pico8_jo.c`** : test neuf, avant le commentaire de `test_ll`, et `in_fresh_child(test_cg_null_until_seen);` après `test_new_planet` dans `main`. Texte exact essayé :

```c
/* cg answers NULL for a tag that no entity has carried since
 * pico8_entity_init(), known or not, and keeps a list once emptied. */
static void test_cg_null_until_seen(void) {
    void *a;

    start_title();
    pico8_entity_init();
    assert(pico8_cg("jo") == NULL && pico8_cg("np") == NULL && pico8_cg("bg") == NULL);
    a = new_planet("p902", 5, 6);
    assert(pico8_cg("jo") != NULL && pico8_cg("np") != NULL && pico8_cg("prober") != NULL);
    assert(pico8_cg("qh") == NULL && pico8_cg("bg") == NULL);
    pico8_he(a);
    assert(pico8_cg("jo") != NULL && pico8_cg("jo")->count == 0);
    pico8_entity_init();
    assert(pico8_cg("jo") == NULL);
}
```

Pourquoi : aucun TU ne fixait « NULL pour une étiquette **connue** jamais vue » (`test_pico8_bg.c:216` ne teste qu'une étiquette inconnue) ; sans lui, la mutation M3 est verte (mesuré). Le test passe au parent tel quel (mesuré) : il fixe le contrat, il ne le change pas. Il ne rejoue aucun `events.json`.

**`docs/glossaire-noms.md`**
- `:43` (`cg`) : « entités indexées par étiquette (`tags` de la classe, `PICO8_TAG_*`), d'où `cg("jo")` = toutes les planètes ».
- `:48` (`nu`) : colonne symbole → « `pico8_struct_class_t.tags`, masque de bits sur `pico8_tag_t` (`PICO8_TAG_*`, `pico8_struct.h`) ; `fv` et `fr` portent ceux de `g_` et `nr` » ; sens inchangé.
- `:599` (`np`) : « étiquette (`PICO8_TAG_NP`) portée par toutes les planètes ; … ».

Comptes 2.7a (mesurés, `b26fc08` → essai rebasé) : `nu` 208 / 37 → **175 / 34** ; `accesseurs-q6` 262 / 33 → **237 / 33** ; les autres inchangées ; pied **1290 / 88 → 1257 / 88** ; `--champs` 287 inchangé.

### 2.7b — ce qui change, fichier par fichier (numéros de `b26fc08`)

**`src/`** (83 lignes retirées, 13 lignes vides comprises dans `game_data.c`) :
- `game_data.h` : les 13 membres `const pico8_str_seq_t *nu;` des `pico8_X_start_t` (`:156`, `:168`, `:183`, `:195`, `:272`, `:289`, `:305`, `:318`, `:328`, `:350`, `:371`, `:399`, `:418`) ; les 13 `extern … pico8_X_nu_tags;` (`:454-462`, `:487`, `:489`, `:492`, `:494`) et le commentaire `:488`.
- `game_data.c` : les 12 `static const char *const X_nu_items[]` et les 13 `pico8_X_nu_tags` (`:132-156`, `:228-229`, `:231-232`, `:240-241`, `:246-247`), chaque bloc avec sa ligne vide ; les 13 `.nu = &pico8_X_nu_tags,` des défauts (`:373`, `:385`, `:399`, `:410`, `:490`, `:507`, `:523`, `:536`, `:546`, `:568`, `:589`, `:616`, `:635`). Pas deux lignes vides de suite (vérifié par `cat -s`, aucune autre ligne touchée).
- Les 5 membres : `msgbox.h:30`, `planet.h:27`, `probe.h:42`, `slipway.h:39`, `wormhole.h:30`.
- Les 13 copies à la construction : `msgbox.c:115`, `planet.c:142`, `planet_family.c:80`, `:166`, `:222`, `:311`, `:416`, `:593`, `planet_family_trade.c:204`, `:395`, `probe.c:454`, `slipway.c:409`, `wormhole.c:251` (numéros du parent de 2.7a, que 2.7a ne décale que de −4 dans `planet.c`, `probe.c`, `msgbox.c`, `slipway.c`, `wormhole.c` et de +1 dans `planet_family.c` après `:559`).

**TU** (texte essayé ; `J`, `QH`, `HJ`, `NR`, `G` = masques ci-dessus écrits en `(PICO8_TAG(…) | …)`). Chaque ancien pli de présence `fnv(h, (uint32_t)(jo->nu != NULL))` valait 1 pour toutes ces classes (`ga` compris : pointeur vers une liste vide) ; il devient `fnv(h, 1u)`, comme les autres 1 constants par la forme que ces TU plient déjà.

| TU | Ancien | Nouveau |
|---|---|---|
| `fr`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `ga` | `h = fnv(h, (uint32_t)(jo->nu != NULL));` | `h = fnv(h, 1u);` |
| `ga` | `fnv(h, (uint32_t)(jo->nu == NULL ? -1 : jo->nu->count))` | `fnv(h, (uint32_t)tag_count(jo->header.cls->tags))`, avec `static int tag_count(unsigned tags)` (bits comptés sur `PICO8_TAG_COUNT`) placé avant la première `static uint32_t` |
| `ga` | `assert(jo->nu == &pico8_ga_nu_tags);` | `assert(jo->header.cls->tags == 0);` |
| `fr` | `assert(jo->nu == &pico8_nr_nu_tags);` | `assert(jo->header.cls->tags == NR);` |
| `hj` / `pm` / `mx` / `jo` / `qh` / `nr` | `jo->nu == &pico8_X_nu_tags` dans l'`assert` d'identité | `jo->header.cls->tags == HJ` / `J` / `J` / `J` / `QH` / `NR` |
| `hj`, `nr` / `pm`, `mx` | `assert(jo->nu != &pico8_qh_nu_tags);` / `…hj_nu_tags` | `assert(jo->header.cls->tags != QH);` / `!= HJ` |
| `nl`, `os` | `assert(jo->nu == &pico8_X_nu_tags && jo->nu != &pico8_qh_nu_tags);` | `assert(jo->header.cls->tags == J);` |
| `qh` `:511` (pli) | `fnv(h, (uint32_t)(qh->jo.nu == &pico8_qh_nu_tags))` | `fnv(h, (uint32_t)(qh->jo.header.cls->tags == QH))` (vrai pour `qh` seul, comme avant) ; commentaire `:499` « its tags/`px`/`ba` are qh's own » |
| `jc` `:437`, `:455-456` | variable `nu` et `assert` count/strcmp | variable retirée ; `assert(jc->header.cls->tags == PICO8_TAG(PICO8_TAG_JC));` |
| `jc` `:496` | `…->nu == …->nu` | `…->header.cls->tags == …->header.cls->tags` |
| `mz` `:302`, `:325-326` | idem `jc` | `assert(mz->header.cls->tags == PICO8_TAG(PICO8_TAG_MZ));` |
| `mh` `:292-307` | commentaire « `fa`/`rh`/`nu` », variable `nu`, 5 lignes d'`assert` | commentaire « `fa`/`rh` » ; `assert(box->header.cls->tags == PICO8_TAG(PICO8_TAG_MH));` |
| `g_` `:203-211`, `fv` `:99-108` | `fold_tags(h, const pico8_str_seq_t *)` | `fold_tags(h, unsigned tags)` : parcourt `t` de 0 à `PICO8_TAG_COUNT`, pour chaque bit posé `k++`, `fnv(h, k)`, `fold_string(h, names[t])`, puis `fnv(h, k)` ; `names` en **initialiseurs désignés** `[PICO8_TAG_QH] = "qh", …` (sinon l'échange de deux constantes de l'enum ne rougit rien) |
| `g_` `:176` | `fold_ref_member(h, g_->nu)` | `fold_ref_member(h, &g_->header.cls->tags)` (pointeur non NULL : mêmes octets, `fold_untagged` plie une constante) |
| `g_` `:358-361` | `assert(e->nu != NULL && …)`, `fold_tags(h, e->nu)`, `assert(e->nu == &pico8_g__nu_tags)` | `assert(pr->dq != NULL && em->k_ != NULL)`, `fold_tags(h, e->header.cls->tags)`, `assert(e->header.cls->tags == G)` |
| `g_` `:293-296`, `:651-654`, `:657`, `:679` | `g_->nu == &pico8_g__nu_tags` et commentaires « `g_`'s `nu` » | `g_->header.cls->tags == G` ; « `g_`'s tags » |
| `fv` `:156-158` | branche `ref == &pico8_g__nu_tags` de `fold_ref` | retirée (plus aucun membre ne pointe la liste) |
| `fv` `:331` | `fold_ref_member(h, g_->nu)` | `fold_tags(fnv(fnv(h, 1u), 0x7Au), g_->header.cls->tags)` (le chemin qu'empruntait `fold_ref_member` → `fold_ref`) |
| `fv` `:244-247`, `:306`, `:308` | `g_->nu == …`, `g_->nu != NULL` | `g_->header.cls->tags == G` ; le terme `g_->nu != NULL` retiré |
| `jo_spec_defaults` | 9 `assert(… nu == pico8_X_start.nu);` | retirées (le masque de chaque classe est fixé par son TU) |
| `spec_defaults` `:112`, `:147`, `:173` | `mz->nu == …` dans un `&&` ; `fv->g_.nu == …` ; `g_->nu == …` | terme retiré ; deux lignes retirées |

Comptes 2.7b (mesurés) : `nu` 175 / 34 → **23 / 9** ; `défauts` 694 → **669** (les 13 copies `pico8_X_start.nu` de `src/`, 9 de `jo_spec_defaults`, 3 de `spec_defaults`) ; pied **1257 / 88 → 1106 / 88** ; `--champs` 287 → **282** (les 5 `nu` partent ; `0 écriture : 15 ; 0 hors constructeur : 27` inchangés).

### 2.7c — ce qui change

Les 23 lignes de `--check nu` restantes, toutes des commentaires d'en-tête, plus un commentaire de code. Texte essayé :
- `help.h:17-18` : la fin « , `nu` is `g_`'s. » part (« `rb` at `pico8_fv_rb_row`. */ ») ; `help.h:46` : « The help's class carries `g_`'s tags, so `lt` registers it into `cg.g_` and … ».
- `msgbox.h:27` : la phrase « `nu` points at `pico8_mh_nu_tags`. » part.
- `planet.h:18` : `(\`nu\`, \`kq\`, …` → `(\`kq\`, …`.
- `planet_family.h:27-28`, `:47-48`, `:59`, `:67-68`, `:83-84`, `:95-96` ; `planet_family_trade.h:19`, `:31`, `:40-41` : `` `nu`, `` retiré des listes de membres ; « `nu`, `px`… point at » → « `px`… point at » ; `os` : « `px` points at os's generated data » ; `ga` : « The emitter's `k_`, `kq` and `fz` point at ga's… ».
- `probe.h:36` : « The emitter's `k_` points at `pico8_g__k__seq` (…) » ; `slipway.h:32` : « `k_` points at `pico8_jc_k__seq`; … » ; `wormhole.h:24` : « `kk` points at `pico8_mz_kk_seq`, `jj` at … ».
- `planet_family.c` (commentaire de `ga_new`, `:591-592` au parent) : retiré ; le piège vit désormais au-dessus de `.tags = 0` (2.7a).
- `Makefile:864` → `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu`.

Comptes 2.7c (mesurés) : `nu` → **0**, disparue de la sortie, gardée ; pied **1106 / 88 → 1083 / 88** ; le reste inchangé.

### Disposition mémoire

2.7a : `pico8_struct_class_t` gagne `unsigned tags` et perd un pointeur ; toutes ses instances sont des `static const` à initialiseurs désignés, aucune n'est copiée ni comparée octet à octet. `g_cg` passe d'un tableau `malloc`é de noms `strdup` à dix seaux statiques : propriétaire et durée de vie changent (d'où `sanitize-net`). 2.7b : cinq types d'entité perdent un pointeur ; aucun `offsetof`, `memcpy` ni `sizeof` hors `malloc(sizeof *self)` ne les vise (même relevé qu'en 2.5).

### Récapitulatif des comptes

| Catégorie | `b26fc08` | 2.7a | 2.7b | 2.7c |
|---|---|---|---|---|
| `nu` | 208 / 37 | **175 / 34** | **23 / 9** | **0**, gardée |
| `accesseurs-q6` | 262 / 33 | **237 / 33** | 237 | 237 |
| `défauts` | 694 / 52 | 694 | **669 / 52** | 669 |
| `lien-parent`, `parente`, `void-ptr`, `ptrs-entité`, `inclusion-mutuelle` | 111, 10, 11, 7, 17 paires | inchangées | inchangées | inchangées |
| pied | 1290 / 88 | **1257 / 88** | **1106 / 88** | **1083 / 88** |
| `--champs` | 287 | 287 | **282** | 282 |

Le détail des 33 lignes `nu` qui partent en 2.7a : les 25 lignes `get_nu` (15 branchements, 5 définitions, 2 prototypes, 2 appels dans `entity.c`, 1 membre de table), les 5 `return ((const …)self)->nu;`, les 3 commentaires `.nu` d'`entity.c` (`:24`, `:200`, `:214`) ; 208 − 33 = 175. L'implémenteur recopie `--check nu` du parent dans le message de 2.7a, classé ainsi.

### Anti-tassement

- **Interdit** :
  - garder une liste de noms d'étiquette par classe dans `src/` sous un autre nom, ou un accesseur qui rendrait la liste ou le masque par instance ; le masque ne se lit que par `e->cls->tags` ;
  - garder un membre `nu` renommé sur une instance, ou renommer une ligne pour échapper au motif `nu` (seule la recherche `pico8_tag_named` est nommée pour ne pas tomber dans `lien-parent`, parce qu'elle n'est pas une lignée) ;
  - donner à `ga` un masque non nul, ou à `fr`/`fv` un masque différent de `nr`/`g_` ;
  - un seau de `cg` alloué à la demande, ou `pico8_cg` qui rendrait une liste vide au lieu de NULL ;
  - changer `tools/composition_residue.sh`, `tests/test_composition_residue.c`, `tests/e2e/`, un `*.sha256` ou une constante d'empreinte ;
  - retirer `nu` de la garde.
- Les tables `names` des TU `g_` et `fv` ne sont pas une liste de classe déguisée : c'est le TU qui plie le masque en noms pour garder ses empreintes, pas le jeu qui aiguille.

## Implementation Chunks

Trois commits, ceux du plan ; essais complets bien sous le budget (2.7a ≈ 25 min, 2.7b ≈ 25 min, 2.7c ≈ 10 min, preuves comprises). Budget : 50 / 50 / 20 min.

1. **2.7a** : `struct.h`, `entity.c/.h`, les 15 tables, les 5 lecteurs, le TU `jo`, le glossaire ; preuves ; commit ; push.
2. **2.7b** : `game_data.c/.h`, les 5 membres, les 13 copies, les 17 TU ; preuves ; commit ; push.
3. **2.7c** : les commentaires, la garde ; preuves ; note « Découvert à 2.7 » ; commit ; push.

**Replis écrits (si une sous-étape déborde à 1,5 fois son budget avant son commit).**
- 2.7a en deux : **2.7a1** `struct.h` (enum, `pico8_tag_named`, `tags` à côté de `get_nu`), les 15 masques, `lt`/`he` lisent le masque, `cg` en seaux fixes, TU `jo` neuf — `get_nu` et ses lecteurs restent, branchés mais plus lus ; `nu` inchangé (208), pied ≈ 1287 / 88 (à remesurer) ; **2.7a2** : `get_nu`, ses 5 lecteurs, 15 branchements et 2 prototypes retirés, glossaire → comptes de 2.7a.
- 2.7b en deux : **2.7b1** les 17 TU lisent le masque (les membres existent encore, simplement plus lus par les TU) — `nu` ≈ 110, `défauts` 694 − 12 = 682 (à remesurer) ; **2.7b2** : `src/` → comptes de 2.7b.
- 2.7c n'a pas de repli.

Un repli s'écrit dans la note « Découvert à ». Les comptes finaux ne changent pas.

## Testing Decisions

- **Un TU neuf** (`test_cg_null_until_seen`, TU `jo`) : il fixe le contrat de `pico8_cg` pour une étiquette connue jamais vue, que rien ne fixait. Aucun autre : le comportement ne change pas, et les empreintes des 15 classes (plis de présence, `assert` d'identité du masque, plis de noms de `g_`/`fv`) le prouvent. Les TU ne rejouent jamais un `events.json`. Les e2e partent de l'écran titre (`make e2e`).
- **Prior art** : ensemble `L` de D.20a, 2.3a, 2.4, 2.5 ; règle du parent de D.21 ; preuve compilateur `const` de 2.0 (Q3) ; garde mordante de 2.2 à 2.6.

### Preuves 2.7a (chemin touché, règle 1)

1. `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré.
2. **Preuve compilateur `const` (Q3)**, sur une copie du parent : `const pico8_str_seq_t *nu;` → `const pico8_str_seq_t *const nu;` dans `msgbox.h`, `planet.h`, `probe.h`, `slipway.h`, `wormhole.h`, puis `gcc -std=c99 -fsyntax-only -Isrc -Itests` sur chaque `.c` de `src/` et `tests/`. Mesuré : **13 erreurs** « assignment of read-only member 'nu' », toutes dans un constructeur — `pico8_mh_struct_new` (`msgbox.c:115`), `pico8_jo_struct_init` (`planet.c:142`), `pico8_qh_struct_init` (`planet_family.c:80`), `pico8_nl_struct_new` (`:166`), `pico8_os_struct_new` (`:222`), `pico8_hj_struct_init` (`:311`), `pico8_pm_struct_new` (`:416`), `pico8_ga_struct_new` (`:593`), `pico8_nr_struct_init` (`planet_family_trade.c:204`), `pico8_mx_struct_new` (`:395`), `pico8_g__struct_init` (`probe.c:454`), `pico8_jc_struct_new` (`slipway.c:409`), `pico8_mz_struct_new` (`wormhole.c:251`). Aucune dans `tests/`. Seuls `main_raylib.c`, `main_screencast_viewer.c`, `pico8_raylib_backend.c` ne compilent pas (`raylib.h`), comme sans la mutation. Toute autre erreur : arrêt « champ sans règle ». Et à l'enfant : `e->cls->tags = 0;` ajouté dans `pico8_lt` d'une copie → `error: assignment of member 'tags' in read-only object` (mesuré) ; les tables sont `static const`.
   (`--champs` compte `nu 13 13` : les 13 `.nu = &…` des initialiseurs de fichier de `game_data.c` comptent hors constructeur ; c'est le majorant du relevé par nom, pas une écriture sur instance.)
3. **`gcov` des e2e** (`make line-coverage EVENTS="$(ls tests/e2e/*.json | tr '\n' ' ')"`, copie) au parent puis à l'enfant ; `L` par appariement `difflib` des sources. Mesuré sur `65f4581` (fichiers identiques à `b26fc08`) : **6305 → 6267**. **`L` = 53 lignes exécutables retirées ou réécrites** : `entity.c:50-54`, `:57`, `:59-61`, `:64`, `:69-70`, `:72-73`, `:76-79`, `:82-83`, `:86-89`, `:108`, `:110`, `:112-115`, `:184-185`, `:202-206`, `:231-236` ; `planet.c:79-80`, `probe.c:399-400`, `msgbox.c:88-89`, `slipway.c:367-368`, `wormhole.c:205-206`. Aucune ligne inchangée ne perd sa couverture. **Toutes** les lignes exécutables neuves de `entity.c` (seaux, `cg`, `lt`, `he`, `shutdown`) et de `pico8_tag_named` sont couvertes par les e2e, **sauf** son `return -1;` (étiquette inconnue), que couvrent `test_pico8_bg.c:216` et le TU neuf. Artefact d'appariement à déclarer, pas un ÉCHEC : `difflib` apparie `struct.h:34`/`:37` (`pico8_state_of`) aux lignes de `pico8_tag_named` ; `pico8_state_of` reste couvert.
4. **Mutations**, une à la fois, à l'enfant puis la même au parent sur l'ancienne forme (`make -k unit` puis `make -k e2e`). **ÉCHEC si verte à l'enfant et rouge au parent.** Verte aux deux : déclarée. Restaurer par `cp` puis `touch`. Mesuré :

| # | Mutation enfant ; parent | TU rouges enfant ; parent | e2e conformes enfant ; parent |
|---|---|---|---|
| M1 | `ga` reçoit le masque de `jo` (`.tags = 0` → `J`) ; parent `planet_family.c:593` → `pico8_jo_start.nu` | 3 (`ga hp pm`) ; 4 (`ga hp jo_spec_defaults pm`) | 25 ; 25 |
| M2 | bit `np` perdu dans la table de `jo` (`planet.c:129`) ; parent `jo_nu_items = {"jo", "prober"}`, compte 2 | 11 (`fr g_ hj jc jo mx nl nr os pm qh`) ; 11, les mêmes | 18 ; 18 |
| M3 | `cg` non NULL pour une étiquette connue jamais vue (`t >= 0 && g_cg[t].seen ?` → `t >= 0 ?`) ; parent : `pico8_cg` crée le seau à la lecture pour l'un des dix noms | **avec** le TU neuf : 1 (`jo`) ; 2 (`jo trade`). **Sans** lui : 0 à l'enfant — d'où le TU | 26 ; 26 (aucun appelant du jeu ne distingue NULL d'une liste vide) |
| M4 | `he` ne retire plus de `cg` (`pico8_ptrs_del(&g_cg[i].list, e)` → `(void)0`) ; parent : même ligne dans la boucle `bucket` | 5 (`g_ hp jo mh mz`) ; 5, les mêmes | 24 ; 24 |

5. `make class-coverage-net` : **OK** (« couverture: OK … 26 rejeu(x) »), mesuré. `make sanitize-net` : **26 propre(s)**, mesuré.
6. `--champs` : 287, inchangé. `make composition-residue` : comptes de 2.7a ci-dessus.

### Preuves 2.7b (empreintes, chemin touché)

1. `make test` vert **sans toucher une constante d'empreinte** (c'est la preuve des octets inchangés) ; `make e2e` 26 conformes ; greps `*_HASH`/`FROZEN_`/`*.sha256`/`tests/e2e` vides. Mesuré.
2. `gcov` des e2e : **6267 → 6254** ; `L` = les 13 copies à la construction (numéros de l'arbre 2.7a : `msgbox.c:111`, `planet.c:138`, `planet_family.c:80`, `:166`, `:222`, `:311`, `:416`, `:593`, `planet_family_trade.c:204`, `:395`, `probe.c:450`, `slipway.c:405`, `wormhole.c:247`), toutes couvertes au parent ; aucune ligne inchangée perdue ; aucune ligne exécutable neuve dans `src/`. Mesuré.
3. Mutations :

| # | Mutation enfant ; parent (arbre 2.7a) | TU rouges enfant ; parent | e2e |
|---|---|---|---|
| bM1 | `PICO8_TAG_G_` et `PICO8_TAG_PROBER` échangés dans l'enum ; parent `g__nu_items = {"prober", "g_"}` | 2 (`fv g_`) ; 2 (`fv g_`) | 26 ; 26 |
| bM2 | `PICO8_TAG_QH` et `PICO8_TAG_HJ` échangés | 0 — **mutant équivalent** : aucun masque ne porte les deux, et ni `lt` ni `cg` ne dépendent de l'ordre ; seul l'ordre à l'intérieur d'un masque plié en noms compte | 26 |

4. `make class-coverage-net` : **OK**, mesuré. `sanitize-net` : non exigé.
5. `--champs` : 287 → 282, les 5 lignes `nu` partent, rien d'autre. Mesuré.

### Preuves 2.7c (commentaire, garde)

1. `make test` vert ; `make e2e` 26 conformes. Mesuré.
2. **Préprocesseur** : `gcc -std=c99 -E -P -Isrc -Itests` de chaque `.c` de `src/` et `tests/`, avant et après : seule `planet_family.c` diffère, par le `__LINE__` d'un `assert` (`622` → `620`). Mesuré.
3. **Garde mordante** (copie) : `const struct pico8_str_seq *nu;` réinjecté après `pico8_struct_header_t header;` dans `planet.h` → `make test` code 2, `nu: src/pico8_planet.h:27:…`, `résidu gardé (noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu) : 1 ligne(s)` (mesuré avec la garde de l'essai), `make: *** [Makefile:867: composition-guard] Error 1`. Mesuré.

### Greps de Sortie (sur l'arbre de 2.7c)

1. `sh tools/composition_residue.sh --check nu` → `résidu gardé (nu) : 0 ligne(s)`, code 0.
2. `grep -rnE '\bnu\b|get_nu|_nu_(tags|items)' src tests | grep -v -e test_composition_residue.c -e test_data_residue.c` → **vide**.
3. `grep -rnE 'pico8_cg_find|pico8_entity_strdup|g_cg_(count|capacity)' src` → vide.
4. `grep -h '\.tags = ' src/*.c | wc -l` → **15** (une par table du tableau de 2.7a) ; `grep -n '\.tags = 0' src/*.c` → une seule ligne, dans la table de `ga` (`planet_family.c`).
5. `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → `864:COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu`.
6. `make composition-residue` → pas de ligne `nu`, `accesseurs-q6 237`, `défauts 669`, `lien-parent 111`, pied **`1083 / 88`**.
7. `grep -n 'pico8_tag_of' src tests -r` → vide.

### Budget et tableau « Runs »

Budget : **50 / 50 / 20 min**, preuves comprises. Au-delà de 1,5 fois, repli, sans rien commettre d'incomplet. Jamais l'appli raylib ; `make raylib` n'est pas exigé.

L'implémenteur tient un tableau « Runs », une ligne par lancement, dans chaque message. Voici celui des essais de ce PRD (copie de `65f4581` : `p` parent, `a`/`b`/`c` sous-étapes ; puis rebasée sur `b26fc08` : `p'`, `a'`, `b'`, `c'`) :

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make test` | `p` | 0 | vert | ≈ 8 s |
| 2 | `make composition-residue` ; `--check nu` | `p` | 0 ; 1 | pied 1291 / 89 ; 208 lignes | < 1 s |
| 3 | `nu` rendu `const`, `-fsyntax-only` de tout `src/` + `tests/` | `p` | — | 13 erreurs, toutes en constructeur | ≈ 20 s |
| 4 | `make test` ; `make e2e` | `a` | 0 ; 0 | vert du premier coup ; 26 conformes | ≈ 8 s ; ≈ 1 s |
| 5 | `make composition-residue` | `a` | 0 | `lien-parent 113 28` avec `pico8_tag_of` → renommé `pico8_tag_named` ; puis `nu 176` (un commentaire neuf disait `nu`) → réécrit ; `nu 175 34`, pied 1258 / 89 | < 1 s |
| 6 | `make class-coverage-net` ; `make sanitize-net` | `a` | 0 ; 0 | OK ; 26 propres | ≈ 2 s ; ≈ 3 s |
| 7 | `make line-coverage` 26 e2e | `a` / `p` | 0 | 6267 / 6305 ; `L` = 53 | ≈ 4 s × 2 |
| 8 | M1 à M4, `make -k unit` + `make -k e2e` | `a` / `p` | 2 (M3 sans TU : 0) | tableau 2.7a | ≈ 30 s × 9 |
| 9 | TU neuf `test_pico8_jo` | `a` / `p` | 0 / 0 | passe aux deux | ≈ 5 s |
| 10 | `e->cls->tags = 0;` dans `pico8_lt`, `-fsyntax-only` | `a` | 1 | read-only object | < 1 s |
| 11 | `make test` ; `make e2e` ; greps d'empreinte | `b` | 0 ; 0 ; vide | vert du premier coup ; 26 | ≈ 8 s ; ≈ 1 s |
| 12 | `make composition-residue` ; `class-coverage-net` | `b` | 0 ; 0 | `nu 23 9`, `défauts 669`, pied 1107 / 89 ; OK | < 1 s ; ≈ 2 s |
| 13 | bM1, bM2 | `b` / `a` | 2 ; 0 | tableau 2.7b | ≈ 30 s × 3 |
| 14 | `make line-coverage` | `b` | 0 | 6254 ; `L` = 13 | ≈ 4 s |
| 15 | `--champs` | `p`, `a`, `b` | 0 | 287, 287, 282 | < 1 s |
| 16 | `gcc -E -P` de tout `src/` + `tests/` | `b` / `c` | 0 | seul `__LINE__` de `planet_family.c` | ≈ 10 s |
| 17 | `make test` avec garde `,nu` ; `make e2e` | `c` | 0 ; 0 | vert ; 26 | ≈ 8 s ; ≈ 1 s |
| 18 | `make test`, membre `nu` réinjecté dans `planet.h` | `c` | 2 | `composition-guard` Error 1, 1 ligne | ≈ 8 s |
| 19 | rebase sur `b26fc08` | — | conflit | `Makefile:864`, résolu en `…,atteinte-à-prouver,nu` | — |
| 20 | `make composition-residue` à chaque étape | `p'`…`c'` | 0 | 1290/88 → 1257/88 → 1106/88 → 1083/88 | < 1 s × 4 |
| 21 | `make test` ; `make e2e` | `c'` | 0 ; 0 | vert ; 26 conformes | ≈ 8 s ; ≈ 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `b26fc08` (ou un descendant qui ne touche que `docs/`), ou si les comptes de départ diffèrent de 1290 / 88 et `nu 208 37` sans explication.
- `FAILED:` si, à l'un des trois commits, `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si la preuve compilateur `const` liste une écriture de `nu` hors d'un `*_new`/`*_init`, ou si elle n'est pas dans le message de 2.7a.
- `FAILED:` si `L` contient une ligne inchangée (hors l'artefact `struct.h` déclaré), ou si une ligne exécutable neuve de `entity.c` n'est pas exécutée par les e2e.
- `FAILED:` si une mutation M1 à M4 ou bM1 est verte à l'enfant et rouge au parent, ou si M3 est verte à l'enfant.
- `FAILED:` si la table de `ga` porte un masque non nul, ou si `fr`/`fv` ont un masque différent de `nr`/`g_`.
- `FAILED:` si `pico8_cg` rend autre chose que NULL pour une étiquette qu'aucune entité n'a portée depuis `pico8_entity_init()`.
- `FAILED:` si la garde ne vaut pas `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu`, ou si la ligne réinjectée ne rougit pas `make test`.
- `FAILED:` si `class-coverage-net` n'est pas OK (2.7a, 2.7b) ou si `sanitize-net` n'est pas propre (2.7a).
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit, ou si les comptes d'arrivée d'une sous-étape ne sont pas ceux du récapitulatif, sans explication.
- `FAILED:` en cas de tassement (voir « Anti-tassement »).
- `FAILED:` si un message de commit omet : les sites `--check nu` du parent classés (2.7a : les 33 ; 2.7b : les 152 ; 2.7c : les 23), `L`, les mutations avec leur résultat au parent, la preuve compilateur (2.7a), la garde mordante (2.7c), `class-coverage-net`/`sanitize-net` quand exigés, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.7 » (en 2.7c, avec les comptes des trois sous-étapes et tout repli), ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Le passage du masque dans l'archétype : l'étude 2.9 le tranche (plan).
- `tools/line_coverage_reference.txt` (`make e2e-gap`, hors suite) : jamais remis à jour par une étape de la phase ; les lignes d'`entity.c` y glissent, comme à 2.3a ou 2.5.
- `pico8_state_of` et le reste de `struct.h` ; les autres accesseurs Q6 (2.14).
- Un `assert` sur un nom d'étiquette inconnu dans `pico8_cg` : le contrat dit NULL, et `test_pico8_bg.c:216` le fixe.
- `sanitize-net` pour 2.7b et 2.7c ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. `pico8_tag_named` plutôt que `pico8_tag_of`.** Le second tombait dans `lien-parent` (motif `pico8_…_of`). L'exclusion existe pour `pico8_state_of` (`composition_residue.sh:64-65`), mais toucher le compteur et son TU pour un nom est plus cher que choisir un nom juste.
- **G2. La recherche par nom vit dans `struct.h`**, à côté de `pico8_state_of` qu'elle imite ; `entity.c` est son seul appelant du jeu.
- **G3. Un drapeau `seen` par seau** plutôt qu'un seau alloué à la première écriture : c'est le plus petit état qui garde le contrat NULL de `pico8_cg`, et le tableau reste fixe comme le plan le demande.
- **G4. Un TU neuf pour ce contrat**, dans le TU `jo` (il sait déjà construire une planète seule) : sans lui, M3 est verte.
- **G5. Le piège de `ga`** passe du constructeur (commentaire retiré en 2.7c) à la table (commentaire posé en 2.7a), là où le masque vit.
- **G6. Les plis de présence `jo->nu != NULL` deviennent `fnv(h, 1u)`**, selon le principe de 2.4/2.5 (« chaque TU connaît la constante de sa classe ») ; le seul pli qui porte une information de nombre (`ga`, compte 0) lit le masque.
- **G7. Les `assert` de `jo_spec_defaults`/`spec_defaults` sur `nu` partent** sans remplaçant : le masque n'est plus un défaut d'instance, et chaque TU de classe fixe le sien.
- **G8. Le glossaire change en 2.7a** (Q4 : le nom neuf naît avec la conversion), bien que les membres `nu` morts vivent jusqu'à 2.7b.

### Surprises de mesure, pour `plan-pilot`

1. **2.7a : `nu` 175 / 34, pied 1257, pas 180 / 1262.** Les trois commentaires `.nu` d'`entity.c` partent avec le code qu'ils décrivent ; `entity.c`, `help.c` et `struct.h` quittent la catégorie (`struct.h` reste au pied par ses accesseurs).
2. **2.7b : `nu` 23 / 9, pied 1106, pas ≈ 30 / ≈ 1113.** 2.7c tombe ensuite exactement sur **1083 / 88**.
3. **Seuls `g_` et `fv` plient des noms d'étiquette** ; les 15 autres TU plient une présence (1) ou comparent un masque. L'ordre de l'enum ne compte que dans un même masque : échanger `qh` et `hj` est un mutant équivalent.
4. **Aucun TU ne fixait « NULL pour une étiquette connue jamais vue »** : d'où le test neuf.
5. **`pico8_tag_of` aurait compté en `lien-parent`**, et un commentaire neuf qui dit `nu` compte en `nu`.
6. **`--champs` compte `nu 13 13`** : les initialiseurs de fichier de `game_data.c` passent pour des écritures hors constructeur (majorant du relevé par nom) ; la preuve compilateur tranche : 0 écriture hors constructeur.
7. **Recouvrement avec 2.6 : la seule ligne `Makefile:864`**, conflit de rebase attendu et trivial.

### Questions pour l'humain

Aucune.
