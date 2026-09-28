---
title: "Plan 3, étape 1b.33 : `fv`, l'aide de l'écran titre. Ses 10 membres propres typés, `m_` en booléen plus `has_m_` ; `g_fv_defaults`, `g_fv_members` et le dernier `offsetof` de `help.c` supprimés"
description: "Après 1b.32, `pico8_fv_t` garde 10 membres `pico8_value_t` : 9 défauts de spec lus par nom (`hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb`) et `m_`, écrit par `gw` sur `cv.hd`. 1b.33 les type selon R1–R9 de 1b.1. `m_` (« sans genre » levé par 1b.4b : `bool`, nil `lu`) prend `has_m_` (R2) et l'écrivain de 1b.13/1b.15 ; les 9 défauts passent en `static` ; `g_fv_defaults` et `g_fv_members` se vident et sont supprimées avec leurs boucles. `make value-members` passe de 72 à 62."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.33 — `fv` (`help.h:21`), **`m_` sans genre** (écrit par `gw` sur `cv.hd`). Empreinte 9 (4, 5). Compte : 62.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, aucun `*.sha256` modifié), §6, §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèles : le PRD de 1b.32 (`plan3-phase1b-etape32-g-sonde.md`, la parente, qui fixe ce dont 1b.33 hérite) et celui de 1b.31 (`plan3-phase1b-etape31-fr-porte-d-infraspace.md`, dernière table vidée d'un fichier). Notes « Découvert à » **citées, pas redérivées** ; elles lient ce PRD :

- **1b.1** : patron R1–R9 ; s'en écarter est une lecture nouvelle, notée.
- **1b.4b** : « sans genre » levé : **`fv.m_` - → `bool` (vu par `test_hover`, pas par la boucle sentinelle)** ; **R2 exige désormais `has_<m>` pour 1b.33 `fv.m_`** ; hors étape, **`fv.ca` n'a plus de preuve de nil** ; la boucle sentinelle `written[]` de `fv` est arrêtée par `PICO8_MEMBER_CENSUS_STOP()`.
- **1b.7** : homonymes — un nil `stat?` venu d'un homonyme ne donne pas d'indicateur quand toute écriture sur le typedef est non nil.
- **1b.9/1b.20** : R7 forme `offsetof` — une table vidée est supprimée avec son seul client (`g_fv_members` → branches R4 `strcmp`), `<stddef.h>` retiré quand plus aucun `offsetof` (1b.20, 1b.31).
- **1b.11** : R2 des TU lents — relus plus bas.
- **1b.13** (forme écrivain de R5) et **1b.15** (sa copie sur un non-pointeur à indicateur, `bg_take_bool`) : une branche par membre `e`, dans l'ordre du typedef ; aide `static` par genre, forme par valeur, **jamais partagée entre fichiers** ; l'aide pose **d'abord** l'état nil (`m_ = 0`, `has_m_ = 0`), puis sort sur `REF NULL` (genre **et** pointeur testés), sinon `assert` « champ sans règle » qui nomme `fv.m_`, le genre de la ligne et l'arrêt ; nil éteint `has_m_` quel que soit l'état d'avant ; un `false` à union nulle est un `false` présent ; `has_m_ = 1` en dernier ; nom sans branche → `return 0`, l'arrêt reste l'`assert(written && …)` de `pico8_entity_set_field` ; **test d'arrêt rouge d'abord**, avec les cas non-REF à union nulle construits par `memset` (1b.14b).
- **1b.14a** : R7 sur une table de défauts — chaque défaut typé est déballé **une fois, à l'init**, dans `static g_fv_<m>` (même pointeur pour un pointeur), remis à nil dans `pico8_fv_defaults_reset`, posé dans le constructeur après la chaîne d'init, avant les clés et tout `pq`.
- **1b.14b** : une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` (`-Werror`).
- **1b.20/1b.23c** : l'`assert` de lecture d'une méthode sur un membre typé (ici celui de `pico8_fv_kj` sur `rb`) passe à l'écriture, c'est-à-dire au déballage.
- **1b.23a1** : plus de trois défauts dans un fichier → une aide `static` de lecture de spec par genre et par fichier, message aux noms de ses membres ; valeurs figées = valeurs **observées** ; `kq` → `pico8_qz_box_t *` (défaut `b()`, lecture 1b.10 de `ref:autre`, forme `jo_default_box`).
- **1b.23a2** : un membre typé sort des `written[]` des TU ; son écriture par nom est figée par la caractérisation de l'étape (fonction distincte, hash propre).
- **1b.23b** : le grep §6 couvre `(cast)x->m`, pas seulement `fv\.m` ; un test de régression qui change dans la seule forme de son écriture porte sa preuve rouge.
- **1b.23d** : un `written[]` vidé est supprimé avec sa boucle ; les variables encore utilisées restent (`-Werror`).
- **1b.24** : le crochet `PICO8_MEMBER_CENSUS_STOP()` d'une boucle supprimée est **gardé seul** ; preuve de typedef par `awk`.
- **1b.29** : un test de régression **sur le chemin** du membre typé, sans diff, est prouvé rouge (bug remis, puis restauré).
- **1b.30/1b.31** : caractérisation dans le TU rapide de la classe.
- **1b.7 (homonymes)** : les lectures par nom de `m_`, `kq`, `gq`, `ca`, `ql`, `fg`… sur `bg`, `ie`, `js`, `jo`, `ui` ne comptent pas.

### État de départ

HEAD `16278a5` (PRD de 1b.32) au moment de l'écriture ; 1b.32 est implémentée **en parallèle**. **Précondition : 1b.32 (`g_`) est commitée avant que 1b.33 commence (H1)** ; `make value-members` rend alors **72**. Numéros de ligne indicatifs (arbre `16278a5`, **avant** 1b.32) ; chaque site est aussi désigné par son contenu. **Rien n'a été compilé ni lancé pour ce PRD** : les faits viennent de `grep` et de la lecture ; tout fait d'exécution, et toute forme du code après 1b.32, est une hypothèse (H1–H20). L'arbre de travail de 1b.32, non commité, lu sans le modifier à l'écriture de ce PRD, a déjà la forme de `help.c` que suppose H2 (`g_fv_defaults` à 9 entrées, 13 `static`, `help_default_fixed/vec/table` définies juste avant `pico8_help_init`, `written[] = {"m_"}`) ; seul le commit fait foi.

### Lignes de recensement (`tools/member_census.txt:31-40`)

| membre | accès | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|---|
| `hi` | offset | `ref:autre` | jamais | - | l | `pico8_vec_t *` (`v(34,56)` de la spec) | non (pointeur) |
| `ca` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non (plus de preuve de nil, 1b.4b) |
| `gq` | offset | `ref:autre` | jamais | - | l | `pico8_vec_t *` (`v(0,11)` de la spec) | non (pointeur) |
| `eo` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `ff` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `kq` | offset | `ref:autre` | jamais | - | l | `pico8_qz_box_t *` (`b(-40,78,40,98)` de la spec) | non (pointeur) |
| `ql` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `fg` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `rb` | offset | `ref:table` | observé | stat | l | `pico8_ob_table_t *` (table de la spec) | non (pointeur ; le `stat` est le test NULL de `pico8_fv_kj`) |
| `m_` | offset | `bool` | observé | lu,stat? | **le** | `int` (0/1) | **`int has_m_`** (R2, 1b.4b) |

Compte : 72 − 10 = **62**. Aucune ligne `pico8_fv_t` ne reste. Les 10 lignes `pico8_js_t` et suivantes (1b.34–1b.37) ne bougent pas.

### Le cart (`pico8/lua.lua`)

- `fv=g_:jr` (`:1143-1166`) : `fa=10`, `ra=v(64,28)`, **`hi=v(34,56)`, `ca=11`, `gq=v(0,11)`, `eo=-1`, `ff=1`**, `fc=5`, `k_=o(…)`, `qo`, `ro`, `qg`, `q_`, `pn=5`, `du=15`, `py`, `pk=1`, `ki=17`, **`kq=b(-40,78,40,98)`, `ql=11`, `fg=5`, `rb=o(4 lignes)`** ; `fv.hd=gk` ; `fv:pq` = `pz(self,bo)` ; `fv:kj(p)` = `ei(self) bl(self.rb)`. Aucune écriture de ces 9 défauts.
- `pz` (`:280-287`) lit `p.ep`, `p.eo`, `p.hi or p.ra`, `p.ca`, `p.gq`, `p.ff` : les 4 boutons de difficulté sont posés à `hi + k·gq`, `fa = ca`.
- `gk` (`:209-210`) lit `e.kq`, `e.fa`, `e.ra`. `ui:ev` (`:225`) lit `o_.ql` ; `ui:gw` (`:226-236`) écrit **`(self.gj or {}).m_=false`** puis **`(self.o_ or {}).m_=true`** sur tout élément de `cv.hd` survolé, et lit `o_.fg`. **Seuls `true` et `false` sont écrits dans `m_` ; nil jamais** (tenu avant le premier survol). Le cart ne relit jamais `fv.m_` : seul le TU le lit (`test_hover`, `test_replay`, `absent[]`).

### Le code (après 1b.32, forme supposée — H2)

**`src/pico8_help.h`** : typedef `pico8_fv_t` (`:19-31`), `pico8_g__t g_` puis 10 `pico8_value_t` ; commentaire TROU-7 au-dessus (« `ra`, `hi`, `gq`, `k_`, `kq`, `rb` are the tables of `fv`'s class, borrowed and never modified ; `nu` is `g_`'s ») qui reste vrai. N'inclut pas `pico8_geom.h` (`pico8_qz_box_t`, `geom.h:33`) ; aucune de ses inclusions ne l'amène (fait, grep).

**`src/pico8_help.c`** :

- `g_fv_defaults[]` : après 1b.32, **9 entrées propres** (`hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb`), `offsetof(pico8_fv_t, <m>)` ; ses 3 clients : la boucle de `pico8_fv_defaults_reset`, la boucle de recopie de `pico8_fv_struct_new` (après `pico8_g__struct_init` et `m_ = REF NULL`), la boucle de lecture + `assert` « fv: every default of the struct is a key of fv's spec » de `pico8_help_init`.
- 13 `static g_fv_<membre de g_>` (`fa`, `fc`, `qo`, `ro`, `qg`, `q_`, `pn`, `du`, `py`, `pk`, `ki`, `ra`, `k_`), aides `help_default_fixed`, `help_default_vec`, `help_default_table`, poses `self->g_.<m> = g_fv_<m>` après la boucle (1b.32).
- `g_fv_members[]` (`:142-150`, 10 entrées : les 9 et `m_`) et son seul client, le lecteur `pico8_fv_field` (REF NULL → 0, repli `pico8_g__struct_class()->field`).
- Écrivain `pico8_fv_set_field` (`:168-175`) : `m_` recopié **quel que soit le genre**, puis délégation à l'écrivain typé de `g_`.
- **Seul accès direct** hors tables : `pico8_fv_kj` (`:91-95`), `self->rb.kind == REF && self->rb.as.ref != NULL`, `assert` de tag `OB_TABLE`, puis `rb_table = self->rb.as.ref` (R6).
- `offsetof(pico8_fv_t` : 19 occurrences après 1b.32 (9 + 10), **les dernières du fichier** ; `size_t i` dans `pico8_fv_defaults_reset`, `pico8_fv_struct_new`, `pico8_help_init`, `pico8_fv_field`.

**Sites par nom, inchangés** (grep `"(hi|ca|gq|eo|ff|kq|ql|fg|rb|m_)"` sur `src/`) : `pico8_pz` (`msgbox.c:480-569` : `ca`, `gq`, `eo`, `hi`, `ff`), `pico8_gk` (`entity_geom.c:48` : `kq`), `pico8_ui_ev`/`pico8_ui_gw` (`ui.c:349`, `:408`, `:413`, `:417` : `ql`, écritures de `m_` en `pico8_value_bool(0/1)`, `fg`). Les autres lignes (`bg.c`, `research_icon.c`, `menu.c`, `score_panel.c`, `planet*.c`, `ui_button.c`) portent des homonymes.

**Tests** :

- Accès direct à un des 10 membres d'un `fv` : **aucun** (grep `(->|\.)(hi|ca|gq|eo|ff|kq|ql|fg|rb|m_)\b` hors chaînes sur les fichiers qui nomment `pico8_fv_`, et `\(pico8_fv_t \*\)` sur `tests/` : vide).
- `tests/test_pico8_fv.c` (TU de la classe, rapide, `CENSUS_UNIT_TESTS`) : `own[]`, `inherited[]`, `tables[]` (`ra`, `hi`, `gq`, `k_`, `kq`, `rb` = `class_ref(pico8_fv_default, …)`), `absent[]` ∋ `m_` ; `test_hover`/`test_replay` lisent `m_` par `bool_state` ; `test_fv_is_a_struct` : `written[]` (après 1b.32 : `{"m_"}`) après `PICO8_MEMBER_CENSUS_STOP()`, qui écrit `pico8_value_fixed(i + 1)` et relit `FIXED` — **à supprimer** : un fixe dans `m_` avorte après typage.
- `tests/test_pico8_spec_defaults.c` (`test_struct_defaults`, `STRUCT_HASH`) : un `pico8_fv_new(NULL)` comparé à la spec par `same_value` (identité des REF `hi`, `gq`, `kq`, `rb`). **Inchangé.**
- `tests/test_pico8_g_.c` (`test_fv`, `FV_HASH`, et la caractérisation de 1b.32) : membres de `g_` sur un `fv`, `write_m_` sur une **sonde** (`:806`, homonyme). **Inchangé.**
- `tests/test_pico8_ui.c` : `pico8_fv_new(NULL)` comme porteur de `fo`/cible refusée d'`et`, lecture de `me` seulement. **Inchangé.**

### Revue des TU lents (R2 de 1b.11)

`grep -nE 'pico8_fv_|is_help|the_help|"fv"|"(hi|eo|ff|ql|fg|rb|kq|m_)"'` sur `test_pico8_{ie,jo,qh,os,hj,pm,mx,nr}.c` : lectures de `kq` (`ie`, `jo`, arbre `qh`) et de `m_` (`ie`) sur leurs propres classes ; les parcours de `hv` (`test_pico8_jo.c:497-515`) filtrent par `daughter_of`. **Aucune lecture nil par nom d'un membre propre de `fv` : aucun indicateur de plus que `has_m_`** (H13).

## Solution

1. **Caractérisation figée sur l'ancien code** (arbre 1b.32 commité), dans le TU rapide `test_pico8_fv`, par nom seul ; verte avant tout typage.
2. **Test d'arrêt rouge d'abord** (écrivain de `m_`) dans le même TU.
3. **Commit 1b.33** : les 10 membres typés, `has_m_` juste après `m_` ; `g_fv_defaults` et `g_fv_members` supprimées avec leurs boucles et l'`assert` de lecture de `pico8_help_init` ; 9 `static g_fv_<m>` ; lecteur à 10 branches `strcmp` ; écrivain `m_` par une aide ; `pico8_fv_kj` lit `rb` directement ; plus aucun `offsetof` dans `help.c` ; `written[]` de `test_pico8_fv` supprimé (crochet `STOP` gardé seul) ; `make value-members` rend **62**.
4. Aucun pixel ne bouge. Note « Découvert à 1b.33 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `help.h`, je veux `pico8_fv_t` sans `pico8_value_t`, chaque membre au type de sa ligne, `has_m_` juste après `m_`.
2. En tant que `pz`, je veux lire sur le `fv` du titre `hi` (même vecteur que la spec), `gq`, `ca` 11, `eo` -1, `ff` 1 comme avant : les 4 boutons au même endroit, `ok.le` vidé.
3. En tant que `gk`, je veux `kq` au même pointeur que la boîte de la spec.
4. En tant que `ui:ev`/`ui:gw`, je veux `ql` 11 et `fg` 5, et écrire `m_` = `true`/`false` relu `BOOL` 1 / `BOOL` 0 présent.
5. En tant que `fv:kj`, je veux dessiner les 4 lignes de `rb`, le même tableau que la spec.
6. En tant que lecteur par nom (TU), je veux `m_` absent sur un `fv` neuf, et nil qui éteint `m_` quel que soit l'état d'avant.
7. En tant que gardien de `m_`, je veux que tout genre hors `bool` et nil avorte, union nulle comprise.
8. En tant que mainteneur, je veux `g_fv_defaults`, `g_fv_members` et tout `offsetof` absents de `help.c`.
9. En tant que test de régression `slipway_drag`, qui clique un bouton de difficulté posé par `pz` depuis `hi`/`gq`/`ca`, je veux rester vert sans diff, et rouge quand mon bug revient.
10. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
11. En tant que `plan-pilot`, je veux `make value-members` à 62 sans ligne `pico8_fv_t`.
12. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, la preuve rouge §6, les mutations et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle : les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` — hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables — et marquées (G).

### `m_` : `int m_` + `int has_m_`, forme de `bg.m_` (1b.15) sans écart (G)

États : nil (tenu à l'init, lu par nom : `absent[]` de `test_built`, `bool_state` = 0 de `test_replay`) ; `true` (écrit par `gw` sur `o_`) ; `false` (écrit par `gw` sur `gj`, lu `BOOL` 0 présent par `test_hover`). Ligne `bool` seule (1b.4b), écrivain unique `pico8_ui_gw` en `pico8_value_bool(0/1)` (H3). **Pas une lecture nouvelle** : R1 `bool` → `int`, R2 `has_m_` (non-pointeur, `l`, nil `lu`), R5 de 1b.15 recopiée ; même forme que `bg.m_`, `ie.m_`, `fq.m_`, `jo.m_`.

- `has_m_` = 0 : nil, `m_` à 0 et non lu ; sinon `m_` ∈ {0, 1}.
- Pas d'emballage public (aucune lecture directe hors le lecteur, précédent `bg.m_`).
- Commentaire de contrat court sur `has_m_`, forme de `research_icon.h:38` (`/* 0: `m_` is nil, and is then never read */`).

### R1–R9 appliquées

| règle | sur `fv` |
|---|---|
| R1 | `pico8_vec_t *hi`, `pico8_fixed_t ca`, `pico8_vec_t *gq`, `pico8_fixed_t eo`, `pico8_fixed_t ff`, `pico8_qz_box_t *kq`, `pico8_fixed_t ql`, `pico8_fixed_t fg`, `pico8_ob_table_t *rb`, `int m_`, `int has_m_` ; noms et places gardés, `g_` premier membre. `#include "pico8_geom.h"` ajouté à `help.h` (seul `#include` ajouté). |
| R2 | `has_m_` seul. `ca` sans indicateur (plus de preuve de nil, 1b.4b) ; `eo`, `ff`, `ql`, `fg` `jamais` ; `hi`, `gq`, `kq`, `rb` pointeurs (NULL = nil, jamais tenu). |
| R3 | `pico8_fv_struct_new` : `self->m_ = 0; self->has_m_ = 0;` à la place de `m_ = REF NULL` ; puis, après les 13 poses de `g_` (1b.32), les 9 poses `self-><m> = g_fv_<m>` dans l'ordre du typedef, à la place de la boucle ; avant l'`assert` sur `overrides`, `pq` (donc `pz`, qui lit `hi`/`gq`/`ca`/`eo`/`ff`), `f_` et `lt`. `size_t i` retiré. |
| R4 | `pico8_fv_field` : 10 branches `strcmp` dans l'ordre du typedef, puis `return pico8_g__struct_class()->field(self, name, out_value);`. Fixes : toujours `FIXED`. `hi`, `gq`, `kq`, `rb` : NULL → `return 0`, sinon REF au même pointeur. `m_` : `has_m_` = 0 → `return 0`, sinon `pico8_value_bool(m_)`. `*out_value` intact sur 0. |
| R5 | Écrivain : `if (strcmp(name, "m_") == 0) { help_take_bool(value, &fv->m_, &fv->has_m_); return 1; }`, puis la délégation à `g_` inchangée. Aucune clé de construction (l'`assert` sur `overrides` reste, texte inchangé). |
| R6 | `pico8_fv_kj` : `if (self != NULL && self->rb != NULL) { rb_table = self->rb; }` ; l'`assert` de tag passe au déballage (`help_default_table`, 1b.20/1b.23c). Aucun autre accès direct dans `src/`. Aucun dans `tests/`. |
| R7 | `g_fv_defaults` vidée → supprimée avec ses 3 boucles et l'`assert` « fv: every default … » ; `g_fv_members` vidée → supprimée avec la boucle du lecteur ; **plus aucun `offsetof` dans `help.c`**, `#include <stddef.h>` retiré (précédents 1b.20, 1b.31 ; H5). `pico8_fv_defaults_reset` gardée (elle remet à nil les 22 `static`), appelée par `pico8_help_shutdown`. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_fv_t)` gardé ; `tools/member_census.txt` non édité. |
| R9 | 72 → 62. |

### Défauts déballés (patron `static`, R7 de 1b.14a, forme de 1b.23a1) (G)

- **9 `static`** dans `help.c`, à côté des 13 de 1b.32 : `g_fv_hi`, `g_fv_gq` (`pico8_vec_t *`), `g_fv_kq` (`pico8_qz_box_t *`), `g_fv_rb` (`pico8_ob_table_t *`), `g_fv_ca`, `g_fv_eo`, `g_fv_ff`, `g_fv_ql`, `g_fv_fg` (`pico8_fixed_t`). Déballés dans `pico8_help_init` juste après le `pico8_ob(…)` de la spec, à la place de la boucle `g_fv_defaults`, par `found = pico8_fv_default("<m>", &v); g_fv_<m> = help_default_<genre>(found, v);`. **Pointeurs de spec** pour `hi`, `gq`, `kq`, `rb` (identité figée par `tables[]` de `test_built` et `STRUCT_HASH` de `test_pico8_spec_defaults`).
- **Aides** : les trois de 1b.32 sont réutilisées, **messages élargis** aux noms nouveaux (`help_default_fixed` : `fv.fa/fc/qo/ro/qg/q_/pn/du/py/pk/ki/ca/eo/ff/ql/fg … (fixe)` ; `help_default_vec` : `fv.ra/hi/gq … a vector v() (ref:autre)` ; `help_default_table` : `fv.k_/rb … a table (ref:table)`) ; **une aide nouvelle**, `help_default_box` (forme `jo_default_box` : REF non NULL, tag `UNKNOWN`, message `"fv.kq: a default of the struct is a key of fv's spec and a box b() (ref:autre)"`).
- Remis à 0/NULL dans `pico8_fv_defaults_reset`, qui ne garde plus de boucle.
- Le commentaire au-dessus de `g_fv_defaults` (« fv's … spec defaults … ») disparaît avec la table ; aucun commentaire nouveau hors contrat (règle de `CLAUDE.md`).
- **Piège noté** : `ca` et `ql` valent tous deux 11 ; un échange des deux poses est un mutant équivalent pour les valeurs (voir Mutations).

### Aide d'écriture (forme 1b.13/1b.15), `static` dans `help.c` (G)

```c
static void help_take_bool(pico8_value_t value, int *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_BOOL &&
           "fv.m_: a write neither nil nor a boolean is off its census row (bool) -- a field under no rule of plan-3 "
           "section 5: stop, do not widen the member");
    *out = value.as.boolean;
    *has = 1;
}
```

Nom au préfixe de fichier, comme `help_default_*` de 1b.32 (lecture 1b.23b : une aide par genre et par fichier ; `help.c` n'a qu'une struct). Copie de `bg_take_bool`, jamais partagée avec `bg.c`.

### Interaction avec la parente `g_` (1b.32)

- 1b.33 ne touche ni `probe.[ch]`, ni les 13 `static g_fv_<membre de g_>`, ni leurs poses, ni la délégation de `pico8_fv_set_field`/`pico8_fv_field` à `g_`.
- L'ensemble des noms qu'accepte l'écrivain de `fv` ne change pas : `m_` puis les 5 de `g_`.

### Tests touchés (G)

- **`tests/test_pico8_fv.c`** seul :
  - ajouts : caractérisation, test d'arrêt, `FV_MEMBERS_HASH`, aides de test (valeur hors ligne globale, forme `g_off_row_value` de `test_pico8_ui`) ;
  - `test_fv_is_a_struct` : `written[]` (`{"m_"}`) vidé → supprimé avec sa boucle ; `PICO8_MEMBER_CENSUS_STOP()` **gardé seul** à sa place (1b.24) ; `size_t i` retiré s'il ne sert plus (`-Werror`), `v` gardé (`pico8_fv_default("hd", &v)`) ; commentaire de tête (« it takes exactly `m_` and `g_`'s 5 written names ») réécrit s'il ne dit plus vrai.
- Aucun autre fichier de `tests/`.

### Revue des tests de régression, mécanique

**Liste** (reprise de 1b.30–1b.32) : les tests nommés au §6 (les 17 de régression) et dans les lignes 1a : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `js`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, plus `tests/test_class_coverage.c`. Tous existent.

**Greps faits pour ce PRD** (`M = hi|ca|gq|eo|ff|kq|ql|fg|rb|m_`) :

- accès direct : `grep -cE 'pico8_fv_t|pico8_fv_|->(M)\b'` → **0 partout** sauf `planet_gh_row_lq:120` (`planet->gq`, un `jo` : homonyme) ;
- par nom `"(M)"` : homonymes seuls — `bg` 17 (`m_` du `bg`), `entity_kind` 19 (`m_`/`kq` d'`ie`), `js` 27 (`m_`/`kq` des boutons), `jo`/`hj`/`nl`/`os`/`pm`/`qh` 7–9 (`kq`/`gq`/`ca` de `jo`), `jc` 1 (`absent[]` d'un `jc`), `jo_spec_defaults` 1 (`NAMES[]` de l'arbre `jo`) ;
- chemin : `pico8__init` (qui construit le `fv` du titre, donc `pz` lit ses 5 défauts par nom) dans tous les tests de la liste sauf `layout_nil_item_draws_nothing`, `planet_kj_dispatch`, `probe_mm_rnd_order`, `mo`, `entity_kind`, `jo_spec_defaults` ; en-têtes des rejeux : tous passent le titre par `pico8_eu` sur `bo[2]` (« no title screen ») **sauf `slipway_drag`** (rejeu depuis le titre, premier clic en (76, 71) à la frame 0, sur un bouton de `pz`) et `ga_export_panel_by` (`boot`, sans rejeu, `pico8__init` seul).

| # | test | accès / chemin | diff ? | preuve rouge |
|---|---|---|---|---|
| R-a | `slipway_drag` | rejeu **depuis le titre** : le clic de la frame 0 vise un bouton que `pz` a posé à `hi + k·gq` du `fv` (branches R4 `hi`/`gq`/`ca`/`eo`/`ff` typées) ; `gw` y écrit `m_` sur l'élément survolé : **sur le chemin** (H14) | non | **Obligatoire** (1b.29). Bug d'origine **2d7021f** remis (nil à la place de l'entité de `jz("gv")` dans `ui:bh`, même preuve que R-d de 1b.32) → rouge (`oe_count(origin) == 1`) ; restauré → vert. |
| — | `js` (TU de 1a.16) | boutons de `pz` et rejeu `title_click_reasonable` : lisent les `js`, pas le `fv` ; `m_`/`kq` des boutons (homonymes) | non | aucune (TU de classe, sans diff) ; vert dans `make test`. |
| — | les 11 rejeux du §6 qui passent le titre par `eu(bo[2])` (`built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `score_panel_close`, `rich_planet_exports`, `pause_menu_music`) | construisent le `fv` du titre (R3, R4 via `pz`), puis `eu(bo[2])` : ni lecture ni écriture d'un membre de `fv` dans leur scénario ; valeurs figées par `test_pico8_fv`, `test_pico8_spec_defaults` et les frames titre du filet | non | aucune ; verts sans modification (H15, précédent 1b.32). |
| — | `ga_export_panel_by`, `planet_gh_row_lq` | `pico8__init` sans rejeu du titre, planètes lues par nom (`planet->gq` de `planet_gh_row_lq:120` : un `jo`) | non | aucune. |
| — | `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `mz`, `nl`, `os`, `pm`, `qh` | homonymes seuls | non | aucune. |
| — | `layout_nil_item_draws_nothing`, `planet_kj_dispatch`, `probe_mm_rnd_order`, `mo`, `test_class_coverage` | aucun | non | aucune. |

### Preuves (sortie attendue sur l'arbre final)

**Sortie de `fv`** :

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_fv_t;/{printf "%s", b}' src/pico8_help.h | grep -c pico8_value_t` : **0** ;
- `make value-members` : **62**, `grep -c pico8_fv_t` : **0**, aucune autre ligne ne bouge (seules les 10 lignes `pico8_fv_t` disparaissent) ;
- `grep -rnw 'g_fv_members\|g_fv_defaults' src/` : **vide** ; `grep -c 'offsetof' src/pico8_help.c` : **0** (19 avant) ; `grep -rn 'offsetof(pico8_fv_t' src/` : **vide** ; `grep -n 'stddef' src/pico8_help.c` : **vide** (H5).

**Lecteur, écrivain, aides** :

- `grep -nE '(->|\.)(hi|ca|gq|eo|ff|kq|ql|fg|rb|m_)\.(kind|as)' src/pico8_help.c` : **vide** ;
- `grep -n 'pico8_value_t \*' src/pico8_help.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : **vide** (le `const pico8_value_t *member` du lecteur et le `*(pico8_value_t *)` de la recopie avant) ;
- `grep -c 'off its census row' src/pico8_help.c` : **1** (`help_take_bool`) ;
- `grep -n 'help_default_box\|help_take_bool' src/pico8_help.c` : chacune définie et appelée une fois (`help_take_bool` par l'écrivain) ;
- `grep -n 'has_m_' src/pico8_help.h` : juste après `m_` ; `git diff -U0 src/pico8_help.h | grep '^+#include'` : `pico8_geom.h` seul ;
- `grep -n 'self->rb' src/pico8_help.c` : la pose R3 et la lecture de `pico8_fv_kj`, rien d'autre.

**Périmètre du diff** :

- `git diff --stat HEAD -- src/ ':!src/pico8_help.h' ':!src/pico8_help.c' tools/ Makefile` : **vide** ;
- dans `tests/`, seul `test_pico8_fv.c` change (ajouts, `written[]` et sa boucle supprimés, commentaire).

### Empreinte et découpage

Ligne : 9 (4, 5). **Remesurée pour ce PRD** avec la formule du plan (fichiers qui nomment `pico8_fv_t` et accèdent à un des 10 noms, ∪ lignes `_field`/`_truthy` portant `"m"`) : **10 (4, 6)** — `help.c` ; `entity_geom.c`, `msgbox.c`, `ui.c` ; `test_pico8_{bg,entity_kind,fq,ie,jo,js}` —, majorant dû aux homonymes (`m_`, `kq`, `ca`, `gq`). **Fichiers modifiés attendus : 3** — `src/pico8_help.h`, `src/pico8_help.c`, `tests/test_pico8_fv.c` — plus `docs/plan-3.md`. Empreinte réelle comptée au commit (H17).

**Découpage (G) : aucun, un seul commit.** Écartés : (a) défauts puis `m_` — les deux moitiés partagent `g_fv_members` et le lecteur, la table ne tomberait qu'au second commit, deux `make e2e` et deux `class-coverage-net` hors budget ; (b) rien à séparer côté fille : `fv` n'en a pas.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.32 commitée avant le départ ; `make value-members` rend 72 au départ (aucune ligne `pico8_g__t`, 10 lignes `pico8_fv_t`), 62 à la fin.
- **H2** : l'arbre 1b.32 a la forme que fixe son PRD : `g_fv_defaults` à 9 entrées propres avec sa boucle de recopie, sa boucle de lecture + `assert` et sa remise à nil ; 13 `static g_fv_<m>` et aides `help_default_fixed/vec/table` ; `g_fv_members`, `pico8_fv_field`, `pico8_fv_set_field` intacts ; `written[]` de `test_pico8_fv` = `{"m_"}`. **Si 1b.32 a pris une autre forme** (noms d'aides, table déjà réduite autrement, `written[]` déjà supprimé), l'implémenteur applique ce PRD à la forme trouvée — mêmes types, même fin (tables et `offsetof` supprimés, 62) — et le note au commit ; ce n'est pas un arrêt. Idem pour une note « Découvert à 1b.32 » qui changerait une règle citée ici : elle prime, et l'écart est noté.
- **H3** : `m_` d'un `fv` ne reçoit que `BOOL` (`pico8_ui_gw`) ou nil, dans les 15 rejeux et les TU hors fils de `must_abort` ; aucun autre écrivain par nom de `m_` sur un `fv` (la boucle sentinelle disparaît). Démentie : arrêt « champ sans règle », sans élargir le membre.
- **H4** : aucun site de `src/` ne lit un des 10 membres directement hors `pico8_fv_kj` (`rb`) ; le compilateur le confirme.
- **H5** : `-Werror` muet après retrait de `<stddef.h>` et des `size_t i` (`size_t`, `NULL` viennent de `<stdlib.h>`/`<string.h>`) ; sinon `<stddef.h>` reste et c'est noté.
- **H6** : tout fichier qui inclut `help.h` compile avec `pico8_geom.h` ajouté (il n'inclut que `pico8_fixed.h` et `pico8_vec.h`).
- **H7** : `test_pico8_fv` reste sous 30 s avec deux fonctions de plus (chacune démarre le titre par `start_title`, sans rejouer tout le rejeu). Démentie : on allège la caractérisation, on ne déplace pas le TU, le `Makefile` ne change pas.
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_fv_t` sans `pico8_value_t` ; `-fsyntax-only -DPICO8_MEMBER_CENSUS` sur `help.c` et `test_pico8_fv.c` suffit si le build complet n'est pas lancé (précédents 1b.29–1b.31).
- **H9** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (l'écrivain recopie tout) ; un cas qui avorte déjà est retiré du « rouge d'abord » et noté.
- **H10** : valeurs **observées** sur le `fv` du titre et sur un `pico8_fv_new(NULL)` : `hi`, `gq`, `kq`, `rb` = pointeurs de la spec ; `ca` 11, `eo` -1, `ff` 1, `ql` 11, `fg` 5 en `FIXED` ; `m_` absent.
- **H11** : `make sanitize-net` propre (aucune allocation nouvelle ; tout membre est posé avant `pz`).
- **H12** : aucune valeur figée ne bouge : `BUILT`/`LIFE`/`HOVER`/`REPLAY_HASH` de `test_pico8_fv`, `FV_HASH` et `G__MEMBERS_HASH` de `test_pico8_g_`, `STRUCT_HASH`/`SPEC_HASH` de `test_pico8_spec_defaults`, les hash de `test_pico8_ui` et de `test_pico8_js`, et les 15 empreintes du filet.
- **H13** : revue des TU lents confirmée par l'implémenteur (grep ci-dessus) : aucune lecture nil par nom d'un membre propre de `fv`.
- **H14** : dans `slipway_drag`, le clic de la frame 0 tombe sur un bouton de `pz` (donc dépend de `hi`/`gq`) et la preuve 2d7021f rougit comme en 1b.32. Si le rejeu ne passe pas par ce bouton, la preuve est faite quand même (deux lancements de quelques secondes) et l'écart noté.
- **H15** : aucun test de régression ne dépend de la représentation des 10 membres ; rien ne dépend de `sizeof(pico8_fv_t)` (`g_` reste premier).
- **H16** : déplacer l'`assert` de tag de `rb` au déballage ne change aucun comportement (`rb` n'a pas d'écrivain, sa seule source est la spec).
- **H17** : empreinte réelle comptée au commit.
- **H18** : le crochet `STOP` gardé seul dans `test_fv_is_a_struct` laisse la table régénérée identique (non lancé).
- **H19** : `must_abort(write_fa/hi/rb/ql/nu/fo/me)` de `test_fv_is_a_struct` avortent toujours (noms sans branche d'écrivain, `pico8_entity_set_field` avorte).
- **H20** : `make class-coverage-net` identique à la référence (le compte d'instanciations de `fv` ne change pas).

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 10 membres directement : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_fv`**, TU de la classe, rapide, dans `CENSUS_UNIT_TESTS`, lié par `ENGINE_SRCS` ; il a `start_title`, `the_help`, `lookup`, `get`, `get_ref`, `class_ref`, `bool_state`, `mouse_at`, `fold_value`, `fold_vec`, `fold_box`, `must_abort`, `in_fresh_child`, et inclut `pico8_geom.h`, `pico8_ui.h`.
- **Caractérisation dirigée `test_fv_members_by_name`**, par `in_fresh_child` après `test_fv_is_a_struct`, hash **`FV_MEMBERS_HASH`**. Lecture par le **lecteur brut** (`pico8_fv_struct_class()->field`, témoin `pico8_value_fixed(0x1234)`) **et** par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur) ; adresses jugées par `assert`, jamais pliées.
  1. **Neufs** : le `fv` du titre (`the_help()`) et un `pico8_fv_new(NULL)`. Les 10 noms pliés (présence, genre, valeur ; vecteurs et boîte par contenu). `assert` : `hi`, `gq`, `kq`, `rb` au pointeur de `class_ref(pico8_fv_default, …)` ; `ca` 11, `eo` -1, `ff` 1, `ql` 11, `fg` 5 ; `m_` absent, témoin intact (H10).
  2. **Écrivain par nom**, sur les deux `fv`, par `pico8_entity_set_field` :

     | écriture de `m_` | lecture attendue |
     |---|---|
     | `true` | `BOOL` 1 |
     | `false` | `BOOL` 0 présent |
     | **`false` à union nulle** (`memset` puis `kind`) | `BOOL` 0 présent |
     | `REF NULL` après `true` | absent, témoin intact |
     | `REF NULL` après `false` ; `REF NULL` sur nil | absent |

     `cls->set_field` rend 1 pour `m_` ; il rend 0, sans rien toucher (valeur relue identique), pour `hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb` (écriture d'un fixe 9) et pour `""`, `"M_"`, `"m_ "`, `"m"`, `"has_m_"`.
  3. **`gw`** : `mouse_at(64, 112)` puis `pico8_ui_lj` → `m_` `BOOL` 1 ; `mouse_at(64, 10)` puis `pico8_ui_lj` → `BOOL` 0 présent (les deux lecteurs).
  4. **Intrus** sur le lecteur : `""`, `"h"`, `"hi "`, `"HI"`, `"m__"`, `"has_m_"`, `"rb_"`, `"kq2"` → 0, témoin intact.
  5. Tout plié dans **`FV_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ (valeur imprimée par `check`), figé ensuite.
  - `pz` (boutons à `hi + k·gq`, `fa` = `ca`), `gk` (`kq`), `kj` (`rb` à l'écran), `ui:ev` (`ql`) et `gw` (`fg` à `0x5f80`) sont déjà figés par `test_built`, `test_life`, `test_hover`, `test_replay` : la caractérisation ne les recopie pas.
- **Test d'arrêt `test_fv_writer_stops_off_row`**, rouge d'abord, par `in_fresh_child` après la caractérisation ; `must_abort` existant, valeur hors ligne dans une globale ; cas à union nulle par `memset` puis `kind` :

  | cible | nom | valeurs hors genre |
  |---|---|---|
  | le `fv` du titre et un `pico8_fv_new(NULL)` | `m_` | fixe 1, **fixe 0 union nulle**, `"x"`, **chaîne NULL** (union nulle), **fonction NULL** (union nulle), REF `ob_table`, REF vecteur, REF `pico8_hv()` (liste), REF d'une sonde neuve (`pico8_g__new(NULL)`), REF de l'autre `fv` |

  **Exclus** (dans le genre) : `true`, `false`, `false` union nulle, nil. Rouge sur l'ancien code (H9), vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_fv && ./tmp/test_pico8_fv`, notées dans le commit avec leur test rouge ou la justification du trou ; un appel unique d'aide `static` se retire sous `if (0)` (1b.14b) :
  1. **(§7-1) indicateur forcé vrai** : le lecteur ignore `has_m_` (nil lu `BOOL` 0 présent) → rouge : caractérisation 1, `test_built` (`absent[]` ∋ `m_`), `test_replay` (`REPLAY_HASH`).
  2. **(§7-2) « présent 0 » là où c'était absent** : sans objet dans sa lettre (aucun fixe typé de `fv` ne peut être absent). **Forme voisine** : la branche `m_` rend `pico8_value_fixed(0)` quand `has_m_` = 0 → rouge : `test_built` (`absent[]`), caractérisation 1.
  3. **(§7-3) init qui saute un membre typé** : **sans objet dans sa lettre** (plus aucune boucle de recopie dans `help.c`) ; forme « pose une autre valeur » (précédents 1b.14a, 1b.24, 1b.32) : (a) `self->eo = g_fv_ff` → rouge : `test_built` (`BUILT_HASH`), caractérisation 1 ; (b) `self->hi = g_fv_gq` → rouge : `test_built` (`tables[]`, `qf` des 4 boutons), caractérisation 1 ; (c) `self->has_m_ = 1` en R3 → rouge : caractérisation 1, `test_built` ; (d) `g_fv_ql` déballé depuis `"fg"` → rouge : `test_hover` (sprite de curseur 11), caractérisation 1 ; (e) `self->kq = NULL` → rouge : `test_life` (`hd` vrai en (64, 112)), caractérisation 1.
  4. *Écrivain de `m_` muet* (`if (0) help_take_bool(…)`, `return 1` gardé) → rouge : `test_hover` (`bool_state` = 2), caractérisation 2 et 3.
  5. *`has_m_ = 1` retiré de l'aide* → rouge : `test_hover`, caractérisation 2.
  6. *Nil jugé sur le pointeur seul* (`value.as.ref == NULL` sans le genre) → rouge : caractérisation 2 (`false` union nulle lu absent), test d'arrêt (fixe 0, chaîne NULL, fonction NULL à union nulle).
  7. *Genre non vérifié* (condition de l'`assert` suivie de `|| 1`) → rouge : test d'arrêt.
  8. *Lecteur : branche `ql` qui rend `fg`* → rouge : `test_hover` (sprite), caractérisation 1.
  9. *R6 : `pico8_fv_kj` ne prend jamais `rb`* (garde `if (0 && …)`) → rouge : `test_life` (`LIFE_HASH`, écran de `kj`).
  10. *Lecteur : branche `rb` renommée* (`"rb "`, repli sur `g_` → 0) → rouge : `test_built` (`own[]`, `tables[]`), caractérisation 1.
  - **Mutants équivalents, notés** : `self->ca = g_fv_ql` (les deux valent 11) ; `*has = 1` avant l'`assert` de l'aide (fils qui avorte, état jamais relu) ; remise à 0/NULL retirée de `pico8_fv_defaults_reset` (l'init redéballe avant toute lecture).
- **`gcov`** : `test_pico8_fv` en `--coverage -O0`, piège `__gcov_dump` des fils (1a.23), sur `help.c`. Lignes attendues exécutées : les 9 déballages et leurs remises à nil, les 9 poses R3 et celles de `m_`/`has_m_`, les 10 branches R4 (deux états de `m_`), la branche `m_` de l'écrivain, `help_take_bool` (nil, `true`, `false`), `help_default_box`, la lecture de `rb` de `pico8_fv_kj`. **Trous attendus** : les `return 0` NULL de `hi`, `gq`, `kq`, `rb` (jamais NULL : aucun écrivain, un nil écrit directement par un test ne serait pas une preuve, 1b.31) ; les échecs d'`assert` des aides (fils de `must_abort`, prouvés par le test d'arrêt et les mutations 6–7 ; spec littérale pour les aides de lecture). Rapport dans le commit.
- **Prior art** : `test_bg_members_by_name`/test d'arrêt de `test_pico8_bg` (1b.15, `bg_take_bool`) ; `test_g__members_by_name`/`test_g__writer_stops_off_row` de `test_pico8_g_` (1b.32) ; `test_nr_members_by_name` et `test_fr_mt_by_name` de `test_pico8_fr` (1b.30, 1b.31) ; `jo_default_box`, `family_trade_default_box`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer.

- `./tmp/test_pico8_fv` : ≤ 30 s (H7) ; environ 17 lancements (départ, arrêt rouge, final, 14 mutations, `gcov`) ≈ 8 min. Si un lancement dépasse 20 s, on joue d'abord 1, 2, 3a, 3c, 4, 6, 7, 9, et on note les autres non lancées.
- §6 R-a (`slipway_drag`) : deux lancements, quelques secondes.
- `make test` une fois ; `make e2e` une fois, **dans la même invocation** que `make class-coverage-net` ; `make sanitize-net` une fois, jamais en même temps que `e2e` ; `make value-members`.
- Aucun TU lent n'est modifié ni lancé seul (aucune mutation n'en dépend) : `make e2e` les lance.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-a bug remis / §6 R-a restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, les **8** TU de `UNIT_TESTS_SLOW` verts ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **62**, aucune ligne `pico8_fv_t` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — requis, `pico8_fv_struct_new` change (sans toucher `pico8_class_coverage_record` ni `KNOWN[]`).

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements (`trading_hub_session_events.json` et `slipway_drag_events.json` sont seulement **lus**).

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet**

- `FAILED:` si 1b.32 n'est pas commitée au départ, ou si `make value-members` ne rend pas 72 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements.

**Types, tables, écrivain**

- `FAILED:` (« champ sans règle ») si `help_take_bool` ou une aide de lecture de spec avorte dans un rejeu ou un TU hors des fils de `must_abort` (H3), ou si un type est élargi au lieu d'avorter (fixe ou autre genre accepté dans `m_`, `pico8_value_t` gardé).
- `FAILED:` si l'un des 10 membres n'a pas le type retenu à sa place, si `has_m_` manque ou n'est pas juste après `m_`, ou si un autre `has_` est ajouté (`has_ca`, `has_rb`…).
- `FAILED:` si nil et `false` de `m_` ne se lisent plus par nom exactement comme en 1a (absent ; `BOOL` 0 présent).
- `FAILED:` si `pico8_fv_t` garde un `pico8_value_t`, si `g_fv_defaults`, `g_fv_members` ou un `offsetof` existe encore dans `help.c`.
- `FAILED:` si `hi`, `gq`, `kq`, `rb` ne sont pas les **pointeurs de la spec** de `fv`, ou si un `static` nouveau n'est pas remis à nil dans `pico8_fv_defaults_reset`.
- `FAILED:` si l'écrivain de `fv` n'accepte plus exactement `m_` puis les 5 noms de `g_`, ou si le message de l'`assert` sur `overrides` change.
- `FAILED:` si l'aide viole la forme de 1b.13/1b.15 (état nil d'abord ; genre **et** pointeur testés ; union nulle hors genre qui avorte ; message qui nomme `fv.m_` et le genre ; `has_m_` posé en dernier), ou si elle est partagée entre fichiers ou rendue publique.
- `FAILED:` si `help.h` gagne un autre `#include` que `pico8_geom.h`, ou si un membre, une aide ou un `static` de `g_` (1b.32) change.

**Diff et preuves**

- `FAILED:` si un site par nom change (`pz`, `pz_step`, `gk`, `ui_ev`, `ui_gw`…), ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si un autre fichier de `src/` que `help.h`, `help.c` change, ou `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS`, `UNIT_TESTS_SLOW`.
- `FAILED:` si un autre test que `test_pico8_fv.c` change, si son diff fait autre chose que ce que liste « Tests touchés », ou si une valeur figée existante change (H12).
- `FAILED:` si un test de régression (§6) a un diff, ou s'il manque les deux lignes « Runs » (rouge bug remis, vert restauré) de R-a.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas l'attendu, si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 62 ou si un autre typedef gagne ou perd une ligne, ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change (le `STOP` de `test_fv_is_a_struct` gardé seul), ou si `tools/member_census.txt` est édité à la main.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, lit ou écrit un des 10 membres directement, ne couvre pas les deux `fv`, les 10 noms, les trois états de `m_` (dont `false` à union nulle), nil après `true` et après `false`, `gw`, les noms sans branche et les intrus ; ou si `FV_MEMBERS_HASH` change après.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il n'a pas les cas union nulle `memset` (fixe 0, chaîne NULL, fonction NULL).
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 10 (avec 3a–3e), chacune avec son test ou la justification de son trou, plus les mutants équivalents.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.33 ».

## Out of Scope

- Les membres de `g_` portés par `fv` (1b.32, faits) : ni leurs types, ni leurs 13 `static`, ni leurs aides d'écriture.
- Les homonymes : `bg.m_` (1b.15), `ie.m_`/`kq` (1b.20), `fq.m_` (1b.18), `jo.m_`/`kq`/`gq`/`ca` (1b.23), `ha_pz.hi`/`gq`/`ca` (1b.4), `js.m_`/`kq` (1b.34), `ui.ql`/`o_`/`gj` (1b.37).
- `pz`, `pz_step`, `gk`, `ui_ev`, `ui_gw` eux-mêmes (sites par nom) ; libérer la spec de `fv` (TROU-7).
- Déplacer un TU entre listes ; régénérer `tools/member_census.txt`.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** : modules `help.h` (typedef, `has_m_`, `#include "pico8_geom.h"`) et `help.c` (lecteur, écrivain, `help_take_bool`, `help_default_box`, 9 `static`, messages élargis, `pico8_fv_kj`, `<stddef.h>`) ; `m_` → `int` + `has_m_` (forme `bg.m_`), `hi`/`gq` → `pico8_vec_t *`, `kq` → `pico8_qz_box_t *`, `rb` → `pico8_ob_table_t *`, `ca`/`eo`/`ff`/`ql`/`fg` → `pico8_fixed_t` ; tests dans `test_pico8_fv` seul, `written[]` supprimé ; pas de découpage ; §6 : R-a (`slipway_drag`) obligatoire.
- **Note « Découvert à 1b.33 »** (à écrire sur la ligne ; le reste va dans le commit) :

  > patron 1b.1 appliqué ; « sans genre » levé par 1b.4b : **`m_` → `int` + `has_m_`, écrivain de 1b.15 sans écart** (`help_take_bool`) ; `hi`, `gq` → `pico8_vec_t *`, `kq` → `pico8_qz_box_t *`, `rb` → `pico8_ob_table_t *` (pointeurs de spec), `ca`, `eo`, `ff`, `ql`, `fg` → `pico8_fixed_t`, aucun autre indicateur (`ca` sans preuve de nil depuis 1b.4b, `rb` pointeur) ; R7 de 1b.14a : 9 `static` de plus, `help_default_box` ajoutée ; **`g_fv_defaults` et `g_fv_members` supprimées, plus aucun `offsetof` ni `<stddef.h>` dans `help.c`** ; R6 : l'`assert` de tag de `rb` de `pico8_fv_kj` passe au déballage ; boucle sentinelle de `fv` supprimée (`STOP` gardé seul) ; §6 : `slipway_drag` sur le chemin (bouton de `pz` posé depuis `hi`/`gq`), prouvé rouge sans diff ; empreinte réelle mesurée au commit.
- Le message de commit porte : compte 72 → 62, résumé « Runs », `gcov`, mutations et trous, `FV_MEMBERS_HASH`, réponses à H1–H20, et la forme de 1b.32 effectivement trouvée (H2).
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
