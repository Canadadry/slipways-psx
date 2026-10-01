---
title: "Plan 3, étape 1b.15 : `bg` (le fond de carte) — cinq membres typés, et le premier écrivain R5 sur un non-pointeur à indicateur (`m_`, écrit par `gw`)"
description: "`pico8_bg_t` a cinq membres `pico8_value_t` (`fa`, `hd` : `fixe`, jamais nil ; `pl` : `ref:table` ; `mq` : `ref:autre`, nil observé ; `m_` : `bool`, nil observé, lu et écrit par nom). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 207 à 202. C'est la deuxième étape à écrivain par nom : la forme écrivain de R5 fixée en 1b.13 y est recopiée, pour la première fois sur un non-pointeur qui porte un indicateur `has_m_`."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.15 — `bg` (`bg.h:20`), `m_` écrit par nom (`gw`). Empreinte 16 (6, 10). Compte : 202.

Cadre : les puces **Nil**, **REF**, **Coexistence** (en particulier « Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` ») et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. S'appliquent aussi, sans être redérivées, les notes « Découvert à » de `docs/plan-3.md` :

- **1b.3** à **1b.8** : patron appliqué sans écart ; `ref:autre` vers un vecteur → `pico8_vec_t *` emprunté, même pointeur (1b.4) ; forme de la branche R4/R5 à `strcmp` (1b.5) ; `fixe` à nil `jamais` sans indicateur même quand un homonyme en a un (1b.6) ; nil `stat?` d'homonyme ne décide rien quand l'écriture est prouvée (1b.7) ;
- **1b.4b** : correction du recensement ; **`bg.m_` exige désormais `has_m_`** (non-pointeur, lu par nom, nil observé, point (3) de la note) ;
- **1b.9** : **R7 forme `offsetof`** (`members[]`) — chaque entrée d'un membre typé est retirée ; une table vidée est supprimée avec sa boucle ; ses usages deviennent des branches R4 `strcmp` dans l'ordre du typedef et des affectations R3 (pointeurs à NULL, fixes et `has_` à 0, tous écrits) ; preuve mécanique `grep -n 'pico8_value_t \*' … | grep -v 'pico8_value_t \*out_value)'` ;
- **1b.10** : R5 en aide `static` par genre qui prend la **valeur** (`lq_take_vec(pico8_value_t, pico8_vec_t **)`) ; lecture de R5 pour `ref:autre` (REF non NULL de tag `UNKNOWN`) ;
- **1b.11** : **règle R2 des TU lents** — chaque PRD relit les TU de `UNIT_TESTS_SLOW` qui touchent sa struct pour y chercher des lectures nil par nom (fait plus bas) ;
- **1b.12** : `ref:autre` reçu en **argument de méthode** déballé par la lecture R5 de 1b.10 (`pico8_jc_mouse_point_take_ra(mp)`) : c'est le cas de `mq`, écrit par `rdown`/`hq` depuis leur argument `rp` ;
- **1b.13** : **la forme écrivain de R5**, recopiée ici point par point (branche par membre `e`, aide `static` par genre, nil d'abord, `assert` « champ sans règle », nom sans branche → `return 0`), **avec sa règle : chaque test d'arrêt porte un cas non-REF à union entièrement nulle, construit par `memset`** (`write_fo_zero_false`, `tests/test_pico8_jc.c`) ;
- **1b.14a** : R5 sur une aide par valeur dont la signature publique est gardée (`ke_take_string`), les deux cas non-REF à union nulle dans le test d'arrêt.

### État de départ

HEAD `f475ed3` (PRD de 1b.14b). 1b.14b (`ke.t`) est **en cours dans l'arbre de travail** (`src/pico8_ke.[ch]`, `bootstrap.c`, `msgbox.c`, `probe.c`, `wormhole.c`, trois `*_display.c`, `tests/test_pico8_iv.c`, `tests/test_pico8_ui.c`). **Précondition** : 1b.14b est commitée avant que 1b.15 commence. Elle ne touche ni `src/pico8_bg.[ch]` ni `tests/test_pico8_bg.c` ; elle fait passer `pico8_ke_t` (enchâssé en tête de `pico8_bg_t`) à `{ header ; const char *me ; pico8_fixed_t t ; }`, sans effet sur les cinq membres propres de `bg`. Les numéros de ligne cités sont ceux de HEAD et peuvent glisser : chaque site est aussi désigné par son contenu (H9). `make value-members` rend alors 207 (H1).

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H12).

### Lignes de recensement (`tools/member_census.txt:15-19`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `fa` | offset | `fixe` | jamais | - | l |
| `hd` | offset | `fixe` | jamais | - | l |
| `pl` | offset | `ref:table` | jamais | - | l |
| `mq` | offset | `ref:autre` | observé | lu | l |
| `m_` | offset | `bool` | observé | lu,stat? | **le** |

Cinq membres : 207 − 5 = **202**.

- `m_` a `le` : **deuxième membre écrit par nom** d'une étape 1b (après `jc_too_far.fo`), et le **premier non-pointeur** écrit par nom. Son nil `lu` vient de la lecture par nom d'un `bg` neuf (`check_fresh_bg`, `test_pico8_bg`, liste `absent`) ; `stat?` vient des homonymes (`jo`, `ie`, `js`, `fv`). R2 donne `has_m_` (note 1b.4b, point 3), sans lecture nouvelle.
- `mq` n'a que `l` : il n'est écrit que **directement**, par `pico8_bg_rdown` et `pico8_bg_hq` (R6), depuis leur argument `rp`. `test_bg_is_a_struct` fige déjà que l'écrire par nom avorte (`must_abort(built, "mq")`).
- `fa`, `hd`, `pl` : constants depuis le constructeur, jamais nil, lus par nom seulement. `test_bg_is_a_struct` fige que l'écriture par nom de `fa` avorte.
- Lignes voisines, **non typées ici** : `pico8_lq_t.bg` (homonyme, fait en 1b.10) ; `pico8_ui_t.o_`, `gj`, `rheld` (cibles `ref:pico8_bg_t`, 1b.36/`ui` en dernier).

**Revue des TU lents (règle R2 de 1b.11).** Relu : `test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`. Seul `test_pico8_ie` construit un `bg` (`others[0] = pico8_bg_new(NULL)`, dans le test de struct de `ie`) et n'y lit que `hd` par nom, **présent**, fixe 1, puis `mv(…, "hb")` (absent, méthode). Les `"m_"` et `"fa"` des sept autres visent des planètes (listes `jo_members[]`, `refs[]`, `fold_live` : `has(e, "m_")` sur une planète). **Aucune lecture nil par nom d'un membre de `bg`** (H4). Sans effet sur R2 : `m_` a déjà `has_m_`, `mq` et `pl` sont des pointeurs, `fa`/`hd` ne sont jamais nil (seul le constructeur les écrit, à 1).

### Ce que fait le cart (`pico8/lua.lua`)

- Spec de `bg` : `fa=1,hd=1,pl=o(0,0,0,72,…,91)` ; `bg:pq` pave la carte.
- `bg:rdown(mp,rp)` (`:378`) : `self.mq=rp return self`. `bg:hq(mp,rp)` (`:381`) : `mo.v=self.mq-rp self.mq=rp`. `rp` est `je.ra` (`ui:bh`, `:237-241`), le point souris.
- `ui:gw` (`:226-233`) : `(self.gj or {}).m_=false` puis `(self.o_ or {}).m_=true`. **Les seules écritures de `m_`** d'un `bg` : `false` et `true`, jamais nil. Le fond répond toujours au test de survol (`hd=1`), donc il est `o_` dès que rien d'autre n'est survolé.
- Aucune lecture de `m_` sur un `bg` dans le cart : `bg` n'a pas de `ev` (`js:ev` et `:937` lisent le `m_` de leur propre objet).

### Le code aujourd'hui

`src/pico8_bg.h` :

- Typedef (`:18-25`) : `{ pico8_ke_t ke; pico8_value_t fa, hd, pl, mq, m_; } pico8_bg_t`. Contrat au-dessus : « `pl` points at the table the module owns, shared by every `bg` and never modified in place. Nothing frees the struct: TROU-7. » Inclut `pico8_ob.h`, pas `pico8_vec.h`.
- API publique inchangée par l'étape : `pico8_bg_rdown(pico8_bg_t *, pico8_value_t mp, pico8_value_t rp)`, `pico8_bg_hq(…, pico8_value_t mp, pico8_value_t rp)`, `pico8_bg_default`, `pico8_bg_struct_class`, `pico8_bg_new`.

`src/pico8_bg.c` :

- **Lecteur** `pico8_bg_field` (`:57-79`) : table locale `members[]` à `offsetof` (les cinq membres), boucle `strcmp`, lecture `*(const pico8_value_t *)((const char *)bg + offset)`, `REF NULL` → `return 0`, sinon copie ; repli `pico8_ke_field`. C'est la **forme `offsetof` de R7** (1b.9), sur toute la table.
- **Écrivain** `pico8_bg_set_field` (`:81-91`), avec son commentaire « `m_` (the hover loop `gw`): the only name a generic writer ever writes on a `bg`. » : `strcmp(name, "m_") == 0` → `bg->m_ = value` (n'importe quel genre), `return 1` ; sinon `return 0`.
- **Constructeur** `pico8_bg_struct_new` (`:104-123`) : `malloc`, `pico8_ke_struct_init`, puis `fa` = fixe 1, `hd` = fixe 1, `pl` = `pico8_value_ref(g_bg_pl)` (la table du module, **pas** celle de la spec), `mq` = `m_` = `REF NULL` ; `assert` « no construction table was measured » (aucune clé de construction : moitié boucle de R5 vacante) ; `pico8_class_coverage_record("bg")` ; `pq` ; `pico8_ke_struct_pq` ; `PICO8_MEMBER_CENSUS_SEE(self, pico8_bg_t)`.
- **Écritures directes** (R6) : `pico8_bg_rdown` (`self->mq = rp`) et `pico8_bg_hq` (lit `pico8_bg_value_vec(self->mq)`, puis `self->mq = rp`). `pico8_bg_value_vec` (« Nil and wrong-kind both read as the zero vector ») sert aussi pour `rp`.
- `pico8_bg_pq`, `pico8_bg_kj`, `pico8_bg_ldown` ignorent `self` ; `pq` lit `g_bg_pl`, jamais `self->pl`.
- `grep -rn 'offsetof(pico8_bg_t' src tests` : les deux lignes de `members[]`, rien d'autre. Pas de fille.

**Sites par nom, inchangés** :

- écrivain : `pico8_ui_gw` (`src/pico8_ui.c`), `pico8_entity_set_field(gj, "m_", pico8_value_bool(0))` puis `(o_, "m_", pico8_value_bool(1))` ;
- lecteurs en `src` : `pico8_ui_gw` lit `"fa"` (couche de `ic`) et appelle `pico8_mv(e, "hd", …)` (champ fixe, pas méthode) ;
- tests : `test_pico8_bg` (tous les membres), `test_pico8_g_` et `test_pico8_bg` (`the_bg` par `hd`/`fa`), `test_pico8_ie` (`hd`), `test_pico8_spec_defaults` (`fa`, `hd`, `pl` d'un `bg` neuf contre la spec, `STRUCT_HASH`).

**Arrêt « nom inconnu »** : `pico8_entity_set_field` (`src/pico8_entity.c`) appelle l'écrivain, puis `PICO8_MEMBER_CENSUS_WRITE`, puis `assert(written && "entity: a generic write names a field this struct has no member for, …")`. Inchangé.

### Empreinte remesurée

La ligne dit 16 (6, 10) : majorant des noms courts partagés (`fa`, `hd`, `m_`, `pl`). **Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_bg_t`) : **6 (2, 4)** — `src/pico8_bg.c` (direct), `src/pico8_ui.c` (par nom) ; `tests/test_pico8_bg.c`, `test_pico8_g_.c`, `test_pico8_ie.c`, `test_pico8_spec_defaults.c` (par nom). `test_pico8_ui.c` et `test_pico8_jc.c` construisent ou désignent un `bg` sans lire ses membres.

**Fichiers modifiés : 3**, plus `docs/plan-3.md` : `src/pico8_bg.h`, `src/pico8_bg.c`, `tests/test_pico8_bg.c` (pur ajout). **Aucun TU de `UNIT_TESTS_SLOW` n'est touché.**

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** : lecture par nom des cinq membres, aller-retour d'écriture par nom de `m_` (dont `false` à union nulle et nil), aller-retour de `mq` par `rdown`/`hq` (voir Testing Decisions).
2. **Typage** : `pico8_fixed_t fa; pico8_fixed_t hd; pico8_ob_table_t *pl; pico8_vec_t *mq; int m_; int has_m_;` (en-tête `ke` en tête, noms et ordre gardés, `has_m_` juste après `m_`).
3. **Lecteur** : cinq branches `strcmp`, la table `members[]` et sa boucle supprimées (R7 de 1b.9).
4. **Écrivain** : la branche `m_` appelle une aide `static` du genre `bool` qui pose d'abord nil, sort sur nil, sinon avorte hors genre, puis recopie et allume `has_m_`. Tout autre nom rend 0 ; la couche entité avorte comme avant.
5. **R6** : `rdown` et `hq` déballent `rp` dans `mq` par une aide `static` du genre `ref:autre` (lecture de 1b.12).
6. **Un test d'arrêt**, écrit rouge sur l'ancien code (qui accepte tout), vert après, avec les cas non-REF à union nulle.
7. Effet mesurable : `make value-members` passe de 207 à **202** et ne liste plus aucune ligne `pico8_bg_t`. Aucun pixel ne bouge.
8. **Note « Découvert à 1b.15 »**.

## User Stories

1. En tant que lecteur de `bg.h`, je veux `fa`/`hd` en `pico8_fixed_t`, `pl` en `pico8_ob_table_t *`, `mq` en `pico8_vec_t *` et `m_` en `int` avec `has_m_`, afin que ce que tient le fond se lise sur le type.
2. En tant que `ui:gw`, je veux écrire `m_ = true` sur le fond survolé et `m_ = false` sur l'ancien `gj`, par nom, comme aujourd'hui.
3. En tant que lecteur par nom de `m_`, je veux 0 avec `*out_value` intact sur un fond neuf, `BOOL 1` après `true`, et `BOOL 0` **présent** après `false`.
4. En tant que mainteneur, je veux qu'écrire nil sur `m_` par nom éteigne `has_m_`, et que la lecture suivante rende 0 avec `*out_value` intact.
5. En tant que mainteneur, je veux qu'un `false` dont l'union entière est à zéro (construit par `memset`) soit lu comme `false` présent, jamais comme nil.
6. En tant que mainteneur, je veux qu'une écriture de `m_` qui n'est ni nil ni un booléen (fixe, fixe 0 à union nulle, chaîne, chaîne NULL à union nulle, fonction, REF de struct, vecteur) arrête l'exécution par un `assert` « champ sans règle » qui nomme `bg.m_` et `bool`.
7. En tant que `ui:bh`, je veux que `rdown` garde `rp` lui-même (même adresse) dans `mq` et rende le fond, et que `hq` déplace `mo.v` de `mq − rp` puis remplace `mq`.
8. En tant que mainteneur, je veux qu'un `rp` qui n'est ni nil ni une REF de vecteur arrête `rdown`/`hq` par l'`assert` « champ sans règle » de `bg.mq`.
9. En tant que `ui:gw` et `the_bg` des tests, je veux lire `fa` = 1 et `hd` = 1 par nom, toujours présents, et `mv(bg, "hd", …)` inchangé.
10. En tant que `test_pico8_spec_defaults`, je veux lire `pl` par nom, même pointeur que `g_bg_pl` pour tous les fonds, afin que `STRUCT_HASH` ne bouge pas.
11. En tant que `test_bg_is_a_struct`, je veux qu'écrire `mq` ou `fa` par nom avorte toujours, par la couche entité.
12. En tant que mainteneur, je veux que l'écrivain brut rende 0 sans rien toucher pour tout nom autre que `m_`.
13. En tant que lecteur de `ke`, je veux que `me` et `t` d'un `bg` restent servis par le repli `pico8_ke_field`.
14. En tant qu'auteur de 1b.16 (`pj.gz`), 1b.23d (`jo.m_`, `jo.kr`), 1b.33 (`fv.m_`) et 1b.34 (`js.m_`), je veux une forme écrite de l'écrivain R5 sur un non-pointeur à `has_`, afin de la recopier sans la redécider.
15. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
16. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
17. En tant que gardien de la couverture, je veux `make class-coverage-net` identique à sa référence (`bg  19  15/15`), puisque le constructeur change.
18. En tant que `plan-pilot`, je veux `make value-members` à 202.
19. En tant que vérificateur, je veux la caractérisation lancée sur l'ancien code, le test d'arrêt rouge puis vert, les mutations du §7 avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées à `bg`

| règle | sur `bg` |
|---|---|
| R1 | `fa`, `hd` → `pico8_fixed_t` ; `pl` → `pico8_ob_table_t *` (non `const`, R1, même si la table n'est jamais modifiée) ; `mq` → `pico8_vec_t *` (lecture de 1b.4, emprunté, même pointeur) ; `m_` → `int` (0/1). `#include "pico8_vec.h"` ajouté à `bg.h` (précédent : `layout.h` pour `lq.ra`). Noms et ordre gardés. |
| R2 | `int has_m_;` juste après `m_` (note 1b.4b). Aucun autre : `fa`/`hd` nil `jamais` (lecture de 1b.6), `pl`/`mq` pointeurs (NULL = nil). La revue des TU lents n'ajoute rien. |
| R3 | Dans `pico8_bg_struct_new`, à la place des cinq affectations actuelles, même position (après `pico8_ke_struct_init`, avant l'`assert` des clés et `pq`) : `fa = pico8_fixed_from_int(1)`, `hd = pico8_fixed_from_int(1)`, `pl = g_bg_pl` (même REF qu'avant, la table du module, pas celle de la spec), `mq = NULL`, `m_ = 0`, `has_m_ = 0`. Tous écrits : le bloc vient d'un `malloc`. |
| R4 | Cinq branches `strcmp` dans l'ordre du typedef, avant le repli `pico8_ke_field`. `fa`, `hd` : toujours présents, `pico8_value_fixed(…)`. `pl` : NULL → `return 0`, sinon `pico8_value_ref(pl)`. `mq` : NULL → `return 0`, sinon `pico8_value_ref(mq)`. `m_` : `has_m_` = 0 → `return 0`, sinon `pico8_value_bool(m_)`. Dans chaque 0, `*out_value` intact. |
| R5 | **Moitié écrivain**, forme de 1b.13 : voir la section suivante. **Moitié boucle vacante** : aucune clé de construction, l'`assert` « no construction table was measured » reste tel quel. |
| R6 | `rdown` et `hq` : `self->mq = rp` devient `bg_take_vec(rp, &self->mq)` (lecture de 1b.12, argument de méthode, voir plus bas). La lecture de `hq` devient `pico8_bg_value_vec(pico8_value_ref(self->mq))` (G) : une seule fonction lit les deux termes de la soustraction, la règle « nil et hors genre → vecteur nul » reste à un seul endroit. Aucune écriture directe de `fa`, `hd`, `pl`, `m_` hors constructeur. |
| R7 | **Forme `offsetof` de 1b.9, table entière** : les cinq entrées de `members[]` sortent ; la table vide est supprimée avec sa boucle et la lecture `*(const pico8_value_t *)` ; pas d'accesseur `pico8_bg_member` à supprimer ; pas de fille. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_bg_t)` gardé ; `PICO8_MEMBER_CENSUS_WRITE` de `pico8_entity_set_field` inchangé ; `tools/member_census.txt` non édité. |
| R9 | 207 → 202, aucune ligne `pico8_bg_t`. |

### R5, moitié écrivain, sur `m_` : la forme de 1b.13 recopiée

1. **Noms écrits.** Une seule branche, `m_` (le seul membre `e`), `if (strcmp(name, "m_") == 0) { bg_take_bool(value, &bg->m_, &bg->has_m_); return 1; }`. `fa`, `hd`, `pl`, `mq` sont `l` seuls : pas de branche, leur écriture par nom reste « nom inconnu ». Le commentaire au-dessus de l'écrivain reste exact et n'est pas retouché.
2. **Aide `static` du genre `bool`**, au nom de la struct, forme par valeur de 1b.10 : `static void bg_take_bool(pico8_value_t value, int *out, int *has)`.
3. **Comment nil éteint `has_m_`, précisément.** Les deux **premières** instructions de l'aide sont, sans condition, `*out = 0;` puis `*has = 0;`. La troisième est `if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) return;` — **genre et pointeur testés tous deux**. Donc :
   - nil (`REF NULL`) : l'aide sort avec `m_` = 0 et `has_m_` = 0 ; la lecture suivante prend la branche R4 `has_m_ == 0` → `return 0`, `*out_value` intact. Quel que soit l'état d'avant (`true`, `false` ou déjà nil), il n'en reste rien ;
   - `false`, y compris un `false` dont **toute l'union est à zéro** (`memset` puis `kind = BOOL`, donc `as.ref == NULL` sur 64 bits) : le genre n'est pas `REF`, l'aide ne sort pas ; elle recopie, `m_` = 0, `has_m_` = 1 ; lu `BOOL 0` **présent**. C'est ce que protège le test du genre : une aide qui ne testerait que `as.ref == NULL` prendrait ce `false` pour nil ;
   - `has_m_` n'est jamais mis à 1 ailleurs que par la dernière instruction de l'aide, après l'`assert`. Le constructeur le pose à 0 (R3).
4. **Hors genre : « champ sans règle ».** Sinon `assert(value.kind == PICO8_VALUE_BOOL && "bg.m_: a write neither nil nor a boolean is off its census row (bool) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` puis `*out = value.as.boolean;` et `*has = 1;`. Un non-REF à union nulle d'un autre genre (fixe 0, chaîne NULL, fonction NULL) n'est pas nil (genre testé) et tombe dans cet `assert`.
5. **Nom inconnu : inchangé.** L'écrivain rend 0 sans rien toucher pour tout autre nom (`fa`, `hd`, `pl`, `mq`, `me`, `t` compris) ; `pico8_entity_set_field` avorte par son propre `assert(written && …)`, message inchangé. Pas d'`assert` dans l'écrivain.
6. **Crochet** : `PICO8_MEMBER_CENSUS_WRITE` reste dans `pico8_entity_set_field`.

### R6 sur `mq` : aide `bg_take_vec`

`static void bg_take_vec(pico8_value_t value, pico8_vec_t **out)`, forme de `lq_take_vec` (1b.10), source de 1b.12 (argument `rp`) : `*out = NULL;` puis sortie sur `REF NULL` (genre et pointeur testés), sinon `assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN && "bg.mq: an rp neither nil nor a vector ref is off its census row (ref:autre) -- a field under no rule of plan-3 section 5: stop, do not widen the member")`, puis `*out = value.as.ref`. Appelée par `rdown` et par la dernière instruction de `hq`, seulement quand `self != NULL` (garde existante). Signatures publiques gardées (précédent 1b.14a). (G) Pas d'aide partagée avec `layout.c` ni `slipway.c` : l'aide nomme `bg.mq` (règle de 1b.13, point 2).

### Preuves

Toutes vides après l'étape, sauf mention :

- `grep -nE '^\s*pico8_value_t [a-z_]+;' src/pico8_bg.h` : les cinq membres avant ; après, **vide** ;
- `grep -rn 'offsetof(pico8_bg_t' src/` : 2 lignes avant ; après, vide ;
- `grep -nE '(->|\.)(fa|hd|pl|mq|m_)\.(kind|as)' src/pico8_bg.c` : vide avant (la table lit par offset) comme après ;
- `grep -nE 'self->mq = |bg->m_ = ' src/pico8_bg.c` : 3 lignes avant (`rdown`, `hq`, écrivain) ; après, vide (tout passe par les aides) ;
- `grep -n 'pico8_value_t \*' src/pico8_bg.c | grep -v 'pico8_value_t \*out_value)'` : avant, la lecture par offset ; après, **vide** ;
- `grep -c 'off its census row' src/pico8_bg.c` : 0 avant ; **2** après (`bg.m_`, `bg.mq`) ;
- `grep -n 'has_m_' src/pico8_bg.h` : une ligne, juste après `int m_;`.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 207 sur l'arbre de départ (1b.14b commitée), puis 202.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, `m_` d'un `bg` ne reçoit par nom que `BOOL` (0 ou 1, de `gw` et des tests) ou, dans la caractérisation seulement, `REF NULL` ; `mq` ne reçoit que `REF NULL` ou une REF de vecteur (tag `UNKNOWN`). Sinon une aide avorte : arrêt « champ sans règle ».
- **H3** : `pl` n'est jamais NULL dans les rejeux et TU (tout `bg` est construit après `pico8_bg_init`) : la branche NULL de `pl` n'est pas exécutée.
- **H4** : aucun TU de `UNIT_TESTS_SLOW` ne lit un membre de `bg` nil par nom (revue ci-dessus).
- **H5** : `MAP_HASH`, `SECOND_MAP_HASH`, `DRAG_HASH`, `HOVER_HASH`, `DRAW_HASH`, `TITLE_HASH`, `PLAYTHROUGH_HASH` (`test_pico8_bg`), `STRUCT_HASH`/`SPEC_HASH` (`test_pico8_spec_defaults`), les empreintes de `test_pico8_ui`, `test_pico8_g_`, `test_pico8_ie`, `test_pico8_jc` et les 15 du filet ne bougent pas.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile sur une struct sans `pico8_value_t` propre (comme après 1b.3 et 1b.14b). Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : aucune allocation ajoutée ; `pico8_type_tag_of` sur une REF de vecteur lit 4 octets d'un bloc de 8.
- **H8** : `test_pico8_bg` reste sous 30 s avec les deux fonctions ajoutées (hors `UNIT_TESTS_SLOW`). Sa durée actuelle n'est pas connue (deux rejeux dans les fils) : si elle dépasse, l'étape s'arrête et le note, le TU ne change pas de liste.
- **H9** : 1b.14b, commitée, ne touche ni `pico8_bg.[ch]` ni `test_pico8_bg.c` ; les numéros cités glissent peut-être.
- **H10** : `je.ra`, l'`rp` que `ui:bh` passe à `rdown`/`hq`, est `REF NULL` ou un vecteur dont `pico8_type_tag_of` rend `UNKNOWN` (même hypothèse que `lq.ra` en 1b.10 et `jc_mouse_point.ra` en 1b.12).
- **H11** : `make class-coverage-net` rend `bg  19  15/15`, identique à `tools/class_coverage_net_expected.txt` (le diff change le corps du constructeur, pas le nombre d'instanciations).
- **H12** : le lecteur brut `pico8_bg_struct_class()->field` rend `me` et `t` par le repli `pico8_ke_field` (présents, `STRING "ph"` et `FIXED 0`), comme `pico8_entity_get_field`.

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Aucun accès direct à `->fa`/`->m_`… dans les tests : ils compilent sur les deux formes.
- **Caractérisation dirigée, figée sur l'ancien code** (G) : `test_bg_members_by_name`, dans `test_pico8_bg` (rapide, dans `CENSUS_UNIT_TESTS`), lancée par `in_fresh_child` après `test_bg_is_a_struct`. Elle lit par le **lecteur brut** (`pico8_bg_struct_class()->field`, témoin : fixe `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder ; elle écrit par l'**écrivain brut** (`->set_field`) et par `pico8_entity_set_field`.
  1. **Lectures par nom.** `start_game(PLAYTHROUGH)`, `a = the_bg()`, `b = pico8_bg_new(NULL)`. Pour chacun : `fa` → 1, `FIXED 1` ; `hd` → 1, `FIXED 1` ; `pl` → 1, `REF`, même adresse pour `a` et `b`, tag `OB_TABLE`, 15 entrées ; `mq` → 0, témoin intact ; `m_` → 0, témoin intact ; `me` → `STRING "ph"`, `t` → `FIXED 0` (repli `ke`, H12). Intrus `""`, `"x"`, `"M_"`, `"m"`, `"has_m_"`, `"ke"`, `"header"`, `"items"` → 0, témoin intact.
  2. **`mq` par les méthodes** (seul chemin d'écriture). `rp1 = vec_ref(3, 4)` ; `pico8_mv(b, "rdown", 2, vec_ref(0, 0), rp1, &out)` → `out` = `b`, `mq` lu = `rp1` par adresse. `pico8_mv(b, "hq", 2, …, rp2 = vec_ref(5, 9))` → `mq` = `rp2`, `mo.v` = (−2, −5). `pico8_mv(b, "rdown", 2, …, REF NULL)` → `mq` absent (0, témoin intact).
  3. **Aller-retour d'écriture par nom de `m_`**, sur `b` : `pico8_entity_set_field(b, "m_", pico8_value_bool(1))` → `BOOL 1` présent ; écrivain brut `(b, "m_", pico8_value_bool(0))` rend 1 → `BOOL 0` **présent** ; `m_` = `false` à union nulle (`memset`, `kind = BOOL`) → `BOOL 0` présent ; `pico8_entity_set_field(b, "m_", REF NULL)` → **absent** (0, témoin intact) ; `REF NULL` une seconde fois → toujours absent ; `pico8_value_bool(1)` → présent 1. À chaque pas, `fa`, `hd`, `pl`, `mq` inchangés.
  4. **Écrivain brut sur un nom qu'il n'écrit pas** : `(b, n, pico8_value_bool(1))` rend 0 pour `n` ∈ {`"fa"`, `"hd"`, `"pl"`, `"mq"`, `"me"`, `"t"`, `""`, `"m_ "`, `"has_m_"`} ; ensuite `fa`/`hd` = 1, `pl` même adresse, `mq` et `m_` dans l'état d'avant.
  5. **`gw`** : `pico8_ui_lj(&mem, pico8_ok())` sans rien d'autre sous la souris (réglage de `test_hover`) → `a` est `o_`, `m_` de `a` = `BOOL 1`.
  6. On plie les retours, genres, fixes, booléens et comptes lus dans **`BG_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les adresses sont jugées par `assert`.

  Lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite.
- **Test d'arrêt, rouge d'abord** (G) : `test_bg_writer_stops_off_row`, lancé par `in_fresh_child` après la caractérisation. Un nouvel utilitaire `must_abort_value(void *e, void (*write)(void *, pico8_value_t), pico8_value_t v)` — ou, au choix de l'implémenteur, une fonction par cas comme `write_fo_zero_false` : l'important est que chaque cas soit un fils qui doit mourir par `SIGABRT`. `must_abort` existant (qui écrit `pico8_value_bool(1)`) n'est pas modifié. Sur `b = pico8_bg_new(NULL)` après `start_game` :
  - **écrivain, `m_` par nom** : fixe 1 ; **fixe 0 à union entièrement nulle** (`memset(&v, 0, sizeof v)`, `v.kind = PICO8_VALUE_FIXED`) ; **chaîne NULL à union nulle** (`memset`, `kind = STRING`) ; `pico8_value_string("x")` ; une fonction (`get(b, "kj")`) ; REF vers `a` (une struct) ; `vec_ref(1, 1)` ;
  - **R6, `mq` par `rdown`** (`pico8_mv(b, "rdown", 2, vec_ref(0, 0), v, &out)`) : fixe 1 ; **fixe 0 à union nulle** ; **`false` à union nulle** (`memset`, `kind = BOOL` : pour `mq`, c'est un non-REF hors genre) ; REF vers `a` (tag `STRUCT`).

  **Rouge sur l'ancien code** (l'écrivain et `rdown` acceptent tout, le fils sort par `_exit(0)`), vert après : une ligne « Runs » pour chacun des deux passages. Les cas à union nulle sont ceux de la règle de 1b.13. Il n'est pas dans la caractérisation figée, qui ne contient que ce que l'ancien code fait déjà. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants, inchangés, qui figent déjà beaucoup** : `check_fresh_bg` (`fa`/`hd` = 1, `pl` 15 entrées, `mq`/`m_` absents), `test_drag` (`mq` même adresse, `DRAG_HASH`), `test_hover` (`m_` 2 puis 1, `HOVER_HASH`), `test_bg_is_a_struct` (`m_` écrit, `must_abort` sur `mq`/`fa`), `test_replay_from_title`/`test_playthrough` (`m_`, `mq` pliés), `test_struct_defaults` de `test_pico8_spec_defaults`. Aucun test de régression (§6) ne lit ces membres.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_bg && ./tmp/test_pico8_bg` (et `test_pico8_spec_defaults` là où c'est dit). Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : la branche R4 de `m_` ignore `has_m_`. Attendu rouge : `check_fresh_bg` (`test_construction`, `m_` absent), caractérisation 1 et 3.
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre** pour les fixes (`fa`/`hd` n'ont pas d'état absent). Forme la plus proche sur un membre typé : la branche `mq` rend `pico8_value_ref(NULL)` présent quand `mq` est NULL. Attendu rouge : `check_fresh_bg`, caractérisation 1 et 2.
  3. (§7-3) *init qui saute un membre typé*. Pour une mutation déterministe, la copie remplace aussi `malloc` par `calloc`. (a) `fa = 1` omis → `fa` lu 0 → `the_bg()` ne trouve rien : rouge partout dans `test_pico8_bg`. (b) `pl = g_bg_pl` omis → `pl` absent : rouge `check_fresh_bg` et `STRUCT_HASH` de `test_pico8_spec_defaults`. (c) `has_m_ = 0` omis : **trou justifié**, `calloc` pose déjà la bonne valeur, et sous `malloc` une omission est une lecture non initialisée qu'aucun outil du filet ne signale (ASan ne détecte pas les lectures non initialisées) ; la ligne est prouvée exécutée par `gcov`.
  4. Propre à l'étape, *nil jugé sur le pointeur seul* : `bg_take_bool` sort sur `value.as.ref == NULL` sans tester le genre. Attendu rouge : caractérisation 3 (`false` à union nulle lu absent) et test d'arrêt (fixe 0 et chaîne NULL à union nulle ne meurent plus). C'est la mutation qui justifie les cas `memset`.
  5. Propre à l'étape, *nil jugé sur le genre seul* : sortie sur `value.kind == PICO8_VALUE_REF`. Attendu rouge : test d'arrêt (REF de struct et vecteur ne meurent plus).
  6. Propre à l'étape, *nil n'éteint pas la présence* : l'aide teste nil **avant** de poser `*out = 0; *has = 0;`. Attendu rouge : caractérisation 3 (nil après `true` reste présent).
  7. Propre à l'étape, *écrivain qui accepte un autre nom* : une branche `mq` qui rend 1. Attendu rouge : `test_bg_is_a_struct` (`must_abort(built, "mq")`) et caractérisation 4.
  8. Propre à l'étape, *R6 muet* : `rdown` n'appelle pas `bg_take_vec`. Attendu rouge : `test_drag`, `test_hover`, caractérisation 2.
- **`gcov`** : `test_pico8_bg` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23. Chaque ligne modifiée est exécutée, avec deux trous attendus : la branche `pl == NULL` du lecteur (H3), et l'échec des deux `assert` d'aide, exécutés seulement dans les fils de `must_abort`, qui avortent sans vider leur compteur (prouvés par le test d'arrêt et les mutations 4 et 5). Le rapport va dans le commit.
- **Prior art** : `test_too_far_members_by_name` et `test_too_far_writer_stops_off_row` (1b.13, `write_fo_zero_false`), le test d'arrêt de `ks` de 1b.14a (`ks_zero_false`, `ks_zero_fixed` dans `test_pico8_ui`), `test_bg_is_a_struct`, `bool_state` de `test_pico8_bg`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_bg` : quelques secondes à quelques dizaines (H8) ; environ 12 lancements (caractérisation et test d'arrêt sur l'ancien code, nouveau code, 8 mutations, `gcov`), 4 à 5 min ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make class-coverage-net` : 3 à 4 min, dans la même invocation que `make e2e` (arbres distincts, comme en 1b.14a) ;
- `make sanitize-net` : ~4 min seul ; il n'est **jamais** lancé en même temps que `e2e` ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent (aucun n'est touché) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 202`**, sans aucune ligne `pico8_bg_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**, parce que le diff touche le corps du constructeur `pico8_bg_struct_new` (R3), sans toucher `pico8_class_coverage_record("bg")` ni `KNOWN[]` (même lecture du §7 qu'en 1b.10 et 1b.14a). Sortie identique à `tools/class_coverage_net_expected.txt` (`bg  19  15/15`, H11).

`make member-census` ne s'applique pas (R8). On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements. On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si une aide avorte dans un rejeu ou dans un TU hors des fils de `must_abort` (H2, H10), si un membre ou un indicateur autre que `has_m_` est ajouté, si l'écrivain accepte un autre nom que `m_`, ou si un type est élargi (union, pointeur d'en-tête, `m_` tenant un fixe) au lieu d'avorter.
- `FAILED:` si les types ne sont pas exactement `pico8_fixed_t fa, hd`, `pico8_ob_table_t *pl`, `pico8_vec_t *mq`, `int m_` suivi de `int has_m_`, dans cet ordre, après `pico8_ke_t ke`.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur `mq`/`pl` NULL, `has_m_` = 0 ou un nom inconnu, ou ne rend pas le même pointeur pour `pl`/`mq`, ou si `me`/`t` ne passent plus par `pico8_ke_field`.
- `FAILED:` si l'aide de `m_` ne pose pas `m_ = 0` et `has_m_ = 0` **avant** le test de nil, si ce test ne vérifie pas le genre **et** le pointeur, si un `false` à union nulle est lu absent, ou si le message ne nomme pas `bg.m_` et `bool`.
- `FAILED:` si `bg_take_vec` ne vérifie pas `REF` et tag `UNKNOWN`, ou si son message ne nomme pas `bg.mq` et `ref:autre` ; si la signature de `pico8_bg_rdown` ou `pico8_bg_hq` change.
- `FAILED:` si l'écrivain avorte lui-même sur un nom inconnu, ou si le message de l'`assert(written && …)` de `pico8_entity_set_field` change.
- `FAILED:` si un site par nom change (`pico8_ui_gw`, `the_bg` des tests), si `pl` cesse d'être `g_bg_pl`, ou si `ui`, `lq`, `ke`, `jc` ou leurs lecteurs, écrivains ou tables changent.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation, le test d'arrêt, leurs utilitaires, `BG_MEMBERS_HASH` et leurs appels, ou si une valeur figée existante change (`MAP_HASH`, `DRAG_HASH`, `HOVER_HASH`, `DRAW_HASH`, `TITLE_HASH`, `PLAYTHROUGH_HASH`, `STRUCT_HASH`…), ou si un test de régression (§6) change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 202, liste encore `pico8_bg_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit ou écrit un membre directement, ou si `BG_MEMBERS_HASH` change après le typage ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'ancien code et une verte après, ou s'il ne porte pas les cas non-REF à union nulle construits par `memset`.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 8 mutations, chacune avec son test ou la justification de son trou.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.15 », ou le tableau « Runs ».

## Out of Scope

- `ui.o_`, `ui.gj`, `ui.rheld` (cibles `ref:pico8_bg_t`, typées avec `ui`) ; les `m_` homonymes de `jo` (1b.23d), `fv` (1b.33), `js` (1b.34), `ie` (fait).
- `lq.bg` (homonyme, 1b.10, fait) ; `ke.me`/`ke.t` (1b.14a/b).
- Typer `rp`/`mp` dans la signature des méthodes de `bg`, ou remplacer `pico8_bg_value_vec`.
- Lire `self->pl` dans `pq` au lieu de `g_bg_pl`, ou partager la table de la spec.
- Une aide de déballage partagée entre fichiers ; retoucher les aides de 1b.5 à 1b.14.
- Remplacer une lecture ou écriture par nom par un accès direct ; régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - module : `bg.[ch]` seul, un commit, sans découpage ;
  - tests : `test_pico8_bg` étendu (caractérisation + test d'arrêt), sans nouveau TU ;
  - caractérisation dirigée gardée alors que la ligne n'est pas « sans genre » : c'est le premier non-pointeur écrit par nom, et nil n'y est jamais écrit par un rejeu (H2) ;
  - `m_` → `int` + `has_m_` (R1, R2), pas de `<m>_is_bool` (un seul genre) ;
  - `mq` → `pico8_vec_t *` (1b.4), déballé par `bg_take_vec` dans `rdown`/`hq` (1b.12) ; `hq` relit par `pico8_bg_value_vec(pico8_value_ref(self->mq))` ;
  - écrivain : forme 1b.13, nil d'abord, genre et pointeur testés ;
  - test d'arrêt couvrant aussi `rdown` (l'`assert` de `bg_take_vec` est atteignable par un appel public, raison de 1b.13) ;
  - mutation §7-3 rendue déterministe par `calloc` dans la copie ;
  - `class-coverage-net` lancé.
- **Note « Découvert à 1b.15 »**, sur la ligne 1b.15 de `docs/plan-3.md` : « patron 1b.1 appliqué ; R7 forme `offsetof` de 1b.9 sur la table entière (`members[]` supprimée) ; `mq` → `pico8_vec_t *` déballé depuis l'argument `rp` de `rdown`/`hq` (lecture de 1b.12) ; **R5 écrivain, forme 1b.13 recopiée sur un non-pointeur à indicateur** (`bg_take_bool(value, &m_, &has_m_)`) : l'aide pose sans condition `m_ = 0` et `has_m_ = 0`, sort sur `REF NULL` (genre **et** pointeur), sinon `assert` `BOOL`, recopie et `has_m_ = 1` ; nil éteint donc `has_m_` quel que soit l'état d'avant, et un `false` à union nulle reste un `false` présent ; le test d'arrêt porte les non-REF à union nulle hors genre (fixe 0, chaîne NULL ; `false` pour `mq`) ; revue des TU lents sans effet ; empreinte réelle 6 (2, 4). » 1b.16 (`pj.gz`), 1b.23d, 1b.33 et 1b.34 la recopient. Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 207 → 202, le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `BG_MEMBERS_HASH` et les réponses à H1 à H12.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
