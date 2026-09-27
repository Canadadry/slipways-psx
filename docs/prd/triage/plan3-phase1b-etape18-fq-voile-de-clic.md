---
title: "Plan 3, étape 1b.18 : `fq` (le voile de clic) — cinq membres typés, `m_` écrit par nom (forme 1b.15), `p_` écrit par `ldown` à travers la même aide, `my` en pointeur de fonction non NULL"
description: "`pico8_fq_t` a cinq membres `pico8_value_t` (`fa`, `hd` : `fixe`, jamais nil ; `my` : `fonction`, nil observé ; `p_` : `bool`, nil observé ; `m_` : `bool`, nil observé, lu et écrit par nom). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 195 à 190. L'écrivain par nom de `m_` recopie la forme de 1b.15 sans écart ; `ldown` écrit `p_` par la même aide `static`, et la clé de construction `my` avorte sur une fonction NULL."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.18 — `fq` (`ui_button.h:72`), `m_` écrit par nom. Empreinte 22 (10, 12). Compte : 190.

Cadre : les puces **Nil**, **REF**, **Coexistence** (« Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` ») et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. S'appliquent aussi, sans être redérivées, les notes « Découvert à » de `docs/plan-3.md` :

- **1b.1**, puis **1b.3** à **1b.16** : patron appliqué ; `fixe` à nil `jamais` sans indicateur même quand un homonyme en a un (1b.6) ; nil `stat?` d'homonyme ne décide rien (1b.7) ; **R7 forme « sélecteur local »** — un `pico8_value_t *` local au lecteur est retiré, remplacé par des branches R4 explicites (1b.8, 1b.12) : c'est la forme du lecteur de `fq` ; R5 en aide `static` par genre qui prend la **valeur** (1b.10), une aide partagée entre plusieurs membres d'un même genre nomme les deux dans son message (`lq_take_fixed`, « `lq.c/score` ») ; boucle de clés à une branche par clé et `else` final gardé (1b.5, 1b.10) ;
- **1b.13/1b.15, forme écrivain de R5**, recopiée point par point : une branche `strcmp` par membre `e`, aide `static` par genre au nom de la struct, **nil d'abord** (pose sans condition `<m> = 0` puis `has_<m> = 0`, sort sur `REF NULL` — genre **et** pointeur testés), sinon `assert` « champ sans règle » qui nomme `struct.membre` et le genre, recopie, `has_<m> = 1` en dernière instruction ; nom sans branche → `return 0` sans rien toucher ; **chaque test d'arrêt porte les cas non-REF à union entièrement nulle, construits par `memset` puis `kind`** (1b.13 `write_fo_zero_false`, 1b.14b « `pico8_value_bool(0)` ne remplit qu'une partie de l'union », 1b.15 `zero_fixed`/`zero_string`/`zero_false`) ;
- **1b.14b** : une mutation qui retire l'unique appel d'une aide `static` est arrêtée par `-Werror` ; on la plante sous une forme qui garde l'appel (`if (0)`, clé renommée) ;
- **1b.11, règle R2 des TU lents** : relire les TU de `UNIT_TESTS_SLOW` qui touchent la struct pour y chercher des lectures nil par nom (fait plus bas).

### État de départ

HEAD `f1704bb` (PRD de 1b.17). **1b.17 (`nf`) est en cours de réimplémentation ; précondition : elle est commitée avant que 1b.18 commence.** D'après son PRD, elle ne touche que `src/pico8_mouse_reader.[ch]`, `tests/test_pico8_iv.c`, `tests/test_pico8_nf.c` et le plan : aucun fichier de cette étape. `make value-members` rend alors 195 (H1). Les numéros de ligne cités sont ceux de HEAD et peuvent glisser : chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H14).

### Lignes de recensement (`tools/member_census.txt:253-257`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `fa` | offset | `fixe` | jamais | - | l |
| `hd` | offset | `fixe` | jamais | - | l |
| `my` | offset | `fonction` | observé | lu,stat? | l |
| `p_` | offset | `bool` | observé | lu | l |
| `m_` | offset | `bool` | observé | lu,stat? | **le** |

Cinq membres : 195 − 5 = **190**.

- `m_` a `le` : écrit par nom par `ui:gw` seulement (`false` sur l'ancien `gj`, `true` sur `o_`), lu par nom (`test_hover_loop_on_the_veil`). Même situation que `bg.m_` en 1b.15 : non-pointeur, nil observé → `has_m_` (R2).
- `p_` n'a que `l` : lu par nom par `dg` (`pico8_bootstrap.c`, à chaque frame, sur chaque entité de `hv`) et par les tests ; écrit **directement** par `pico8_fq_ldown` (R6) depuis le résultat de `mv(self,"my")` : `true`, `false` (TU seulement) ou nil (voile nu, `fq()`). Non-pointeur, nil observé `lu` → `has_p_` (R2).
- `my` n'a que `l` : reçu par la clé de construction `my` (`fq(de)` : `de.my`, la fonction de `pj`), lu par nom par `mv(self,"my")` de `ldown` et par les tests ; nil sur un voile nu. Pointeur → pas d'indicateur (R2, NULL = nil).
- `fa` (14), `hd` (1) : posés par le constructeur, jamais nil, jamais écrits ailleurs, lus par nom (`ui:gw`, `find_veil`, `test_defaults_match_the_parsed_spec`, `test_pico8_ie`, `test_struct_defaults`).
- Lignes voisines, **non typées ici** : `pico8_js_t` (même fichier, `my`/`m_`/`fa`/`hd` homonymes, 1b.34) ; `pico8_ui_t.o_`, `gj` (cibles `ref:pico8_fq_t`, `ui` en dernier) ; `mh.p_` (1b.1, fait).

**Revue des TU lents (règle R2 de 1b.11).** `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` ne nomment pas `fq`. `test_pico8_ie` construit un voile (`others[1] = pico8_fq_new(NULL)`) et n'y lit que `hd` par nom, **présent**, fixe 1, puis `mv(…, "hb")` (absent, repli `ke`). Les TU lents qui font tourner `dg` lisent `p_` par nom sur les entités de `hv`, un voile compris s'il y en a un : `p_` a déjà `has_p_`. **Aucune lecture nil par nom qui change une décision** (H4).

### Ce que fait le cart (`pico8/lua.lua`)

- `fq=ke:jr([[fa=14,hd=1,]])` (`:324-326`), `function fq:ldown()self.p_=mv(self,"my") return true end` (`:327`).
- Deux constructions : `fq()` dans `pj:ph` (`:871`, voile nu, `my` nil) et `fq(de)` dans `ix[3].my` (`:985`), avec `de={my=function()ju.c_ = not ju.c_ return true end}` (`:901`).
- `dg` (`:177`) : `if qc.p_ then he(qc) end` — le voile tombe au `dg` suivant un clic dont `my` a rendu vrai. Un voile nu avale le clic (`p_` = nil) et reste.
- `ui:gw` écrit `m_` (`false`/`true`) ; aucune lecture de `m_` sur un `fq` dans le cart (pas de `ev`).

### Le code aujourd'hui

`src/pico8_ui_button.h` (`:68-77`) : `{ pico8_ke_t ke; pico8_value_t fa, hd, my, p_, m_; } pico8_fq_t` (un membre par ligne), contrat au-dessus : « `my` is the closure the construction table carried, which `mv` calls as a field, not a method. Nothing frees the struct: TROU-7. » Inclut déjà `pico8_value.h` (`pico8_value_function_t`). API publique inchangée : `pico8_fq_init`, `_shutdown`, `_ready`, `_default`, `_struct_class`, `pico8_fq_new`, `pico8_fq_ldown`.

`src/pico8_ui_button.c` (partie `fq`, `:376-489`, après tout le code de `js`) :

- **Lecteur** `pico8_fq_field` (`:383-405`) : sélecteur local `const pico8_value_t *member` choisi par cinq `strcmp`, `REF NULL` → `return 0`, sinon copie ; repli `pico8_ke_field`. Pas de table `offsetof` : `grep -rn 'offsetof(pico8_fq_t' src tests` est vide.
- **Écrivain** `pico8_fq_set_field` (`:407-415`) : `strcmp(name, "m_") != 0` → `return 0` ; sinon `fq->m_ = value` (n'importe quel genre), `return 1`.
- **Constructeur** `pico8_fq_struct_new` (`:423-449`) : `malloc`, `pico8_ke_struct_init`, `fa` = fixe 14, `hd` = fixe 1, `my` = `p_` = `m_` = `REF NULL` ; boucle des clés : `my` → `self->my = value` (n'importe quel genre), toute autre clé → `assert(0 && "fq: the only measured key is my; …")` ; `pico8_class_coverage_record("fq")` ; `pico8_ke_struct_pq` ; `PICO8_MEMBER_CENSUS_SEE(self, pico8_fq_t)`.
- **`pico8_fq_ldown`** (`:478-489`) : `self` NULL → `true` ; `mv(self, "my", …)` → `my_result` (REF NULL si `my` absent) ; `self->p_ = my_result` (écriture directe, R6) ; rend `true`.

**Sites par nom, inchangés** : `pico8_dg` (`src/pico8_bootstrap.c`, lit `"p_"`) ; `pico8_ui_gw` (`src/pico8_ui.c`, écrit `"m_"`, lit `"fa"`, `mv(e, "hd", …)`) ; `pico8_mv(self, "my", …)` de `ldown` ; tests : `test_pico8_fq` (tous), `test_pico8_ie` (`hd`), `test_pico8_spec_defaults` (`fa`, `hd` dans `NAMES`, `STRUCT_HASH` ; `my`/`p_`/`m_` n'y sont pas).

**Qui construit un voile** : `src/pico8_menu.c` (`pico8_ix3_my_dispatch` : `pico8_fq_new(pico8_de())`), `src/pico8_score_panel_display.c` (`pj:ph` : `pico8_fq_new(NULL)`) ; la seule fonction `my` que reçoit un voile hors TU est `pico8_pj_de_my_dispatch`, qui rend `pico8_value_bool(1)` (H2).

**Arrêt « nom inconnu »** : `pico8_entity_set_field` (`src/pico8_entity.c`) appelle l'écrivain, puis `PICO8_MEMBER_CENSUS_WRITE`, puis `assert(written && …)`. Inchangé.

### Empreinte remesurée

La ligne dit 22 (10, 12) : majorant des noms courts partagés (`fa`, `hd`, `my`, `p_`, `m_`). **Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_fq_t`) : **6 (3, 3)** — `src/pico8_ui_button.c` (direct), `src/pico8_bootstrap.c` (`p_` par nom), `src/pico8_ui.c` (`m_`, `fa`, `hd` par nom) ; `tests/test_pico8_fq.c`, `test_pico8_ie.c` (`hd`), `test_pico8_spec_defaults.c` (`fa`, `hd`). `menu.c` et `score_panel_display.c` construisent un voile sans lire ses membres ; `test_pico8_bg`, `jc`, `ui`, `pj`, `game_end`, `score_panel_close` le construisent, le comptent ou lisent `me`/`t`/`eb` (repli `ke`), pas ses membres.

**Fichiers modifiés : 3**, plus `docs/plan-3.md` : `src/pico8_ui_button.h` (typedef `fq`), `src/pico8_ui_button.c` (partie `fq` seule), `tests/test_pico8_fq.c` (pur ajout). **Aucun TU de `UNIT_TESTS_SLOW` n'est touché.**

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** : lecture par nom des cinq membres (voile nu et voile `fq(de)`), aller-retour de `p_` par `ldown`, aller-retour d'écriture par nom de `m_` (dont `false` à union nulle et nil), écrivain brut sur les autres noms (voir Testing Decisions).
2. **Typage** : `pico8_fixed_t fa; pico8_fixed_t hd; pico8_value_function_t my; int p_; int has_p_; int m_; int has_m_;` (en-tête `ke` en tête, noms et ordre gardés, chaque `has_` juste après son membre).
3. **Lecteur** : cinq branches R4 `strcmp` dans l'ordre du typedef, sélecteur local supprimé (R7 de 1b.8).
4. **Écrivain** : la branche `m_` appelle l'aide `static` du genre `bool`, forme 1b.15 exacte. Tout autre nom rend 0 ; la couche entité avorte comme avant.
5. **R6 dans `ldown`** : `p_` reçoit `my_result` par **la même aide `fq_take_bool`**.
6. **Clé `my`** : déballée par une aide `static` du genre `fonction`, `fq_take_function`, qui avorte sur une fonction NULL.
7. **Un test d'arrêt**, rouge sur l'ancien code (qui accepte tout), vert après, avec les cas non-REF à union nulle.
8. Effet mesurable : `make value-members` passe de 195 à **190** et ne liste plus aucune ligne `pico8_fq_t`. Aucun pixel ne bouge.
9. **Note « Découvert à 1b.18 »**.

## User Stories

1. En tant que lecteur de `ui_button.h`, je veux `fa`/`hd` en `pico8_fixed_t`, `my` en `pico8_value_function_t`, `p_` et `m_` en `int` suivis de `has_p_`/`has_m_`, afin que ce que tient le voile se lise sur le type.
2. En tant que `ui:gw`, je veux écrire `m_ = true` sur le voile survolé et `m_ = false` sur l'ancien `gj`, par nom, comme aujourd'hui.
3. En tant que lecteur par nom de `m_`, je veux 0 avec `*out_value` intact sur un voile neuf, `BOOL 1` après `true`, `BOOL 0` **présent** après `false`.
4. En tant que mainteneur, je veux qu'écrire nil sur `m_` par nom éteigne `has_m_`, quel que soit l'état d'avant, et que la lecture suivante rende 0 avec `*out_value` intact.
5. En tant que mainteneur, je veux qu'un `false` dont toute l'union est à zéro (`memset`) soit lu `false` présent, jamais nil, qu'il arrive par `m_` ou par le retour de `my`.
6. En tant que mainteneur, je veux qu'une écriture de `m_` ni nil ni booléenne (fixe, fixe 0 à union nulle, chaîne, chaîne NULL à union nulle, fonction, REF de struct, REF de table) avorte par l'`assert` « champ sans règle » qui nomme `fq.m_/p_` et `bool`.
7. En tant que `dg`, je veux lire `p_` absent sur un voile nu cliqué (le voile reste), `BOOL 1` après un `my` vrai (le voile tombe au `dg` suivant), `BOOL 0` présent après un `my` faux (le voile reste).
8. En tant que mainteneur, je veux qu'un `my` qui rend nil après avoir rendu vrai éteigne `has_p_`.
9. En tant que mainteneur, je veux qu'un `my` qui rend autre chose que nil ou un booléen arrête `ldown` par le même `assert`.
10. En tant que `fq(de)`, je veux que `my` lu par nom soit la même fonction que `de.my`, rendue `FUNCTION`, et que `mv(self,"my")` l'appelle.
11. En tant que `fq()`, je veux `my` absent (0, témoin intact), et `ldown` qui rend quand même `true`.
12. En tant que mainteneur, je veux qu'une clé de construction `my` nil donne `my` absent, et qu'une clé `my` ni nil ni fonction non NULL (fixe, fixe 0 à union nulle, `false` à union nulle, **fonction NULL à union nulle**, chaîne, REF de struct) avorte par l'`assert` « champ sans règle » de `fq.my`.
13. En tant que `ui:gw`, `find_veil`, `test_pico8_ie` et `test_struct_defaults`, je veux lire `fa` = 14 et `hd` = 1 par nom, toujours présents, et `mv(fq, "hd", …)` rendre `FIXED 1`.
14. En tant que mainteneur, je veux que l'écrivain brut rende 0 sans rien toucher pour tout nom autre que `m_`, et qu'écrire `p_`, `my`, `fa` ou `hd` par nom avorte toujours, par la couche entité.
15. En tant que lecteur de `ke`, je veux que `me` et `t` d'un voile restent servis par le repli `pico8_ke_field`.
16. En tant qu'auteur de 1b.23d (`jo.m_`), 1b.33 (`fv.m_`) et 1b.34 (`js.m_`, `js.my`), je veux une deuxième application sans écart de la forme 1b.15, et une forme écrite du genre `fonction` en boucle de clés.
17. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
18. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
19. En tant que gardien de la couverture, je veux `make class-coverage-net` identique à sa référence (`fq  3  2/15`), puisque le constructeur change.
20. En tant que `plan-pilot`, je veux `make value-members` à 190.
21. En tant que vérificateur, je veux la caractérisation lancée sur l'ancien code, le test d'arrêt rouge puis vert, les mutations du §7 avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées à `fq`

| règle | sur `fq` |
|---|---|
| R1 | `fa`, `hd` → `pico8_fixed_t` ; `my` → `pico8_value_function_t` (précédent `mh.lh`, 1b.1) ; `p_`, `m_` → `int` (0/1). Noms et ordre gardés. Aucun include ajouté. |
| R2 | `int has_p_;` juste après `p_`, `int has_m_;` juste après `m_`. Aucun autre : `fa`/`hd` nil `jamais` (1b.6), `my` pointeur (NULL = nil). La revue des TU lents n'ajoute rien. |
| R3 | Dans `pico8_fq_struct_new`, à la place des cinq affectations actuelles, même position (après `pico8_ke_struct_init`, avant la boucle des clés et `pico8_ke_struct_pq`) : `fa = pico8_fixed_from_int(14)`, `hd = pico8_fixed_from_int(1)`, `my = NULL`, `p_ = 0`, `has_p_ = 0`, `m_ = 0`, `has_m_ = 0`. Tous écrits : le bloc vient d'un `malloc`. |
| R4 | Cinq branches `strcmp` dans l'ordre du typedef, avant le repli `pico8_ke_field`. `fa`, `hd` : toujours présents, `pico8_value_fixed(…)`. `my` : NULL → `return 0`, sinon `pico8_value_function(my)`, même pointeur. `p_` : `has_p_` = 0 → `return 0`, sinon `pico8_value_bool(p_)`. `m_` : idem avec `has_m_`. Dans chaque 0, `*out_value` intact. |
| R5 | **Moitié écrivain** : forme 1b.15 (section suivante). **Moitié boucle** : la branche `my` appelle `fq_take_function` ; l'`else` final et son message restent tels quels. |
| R6 | `ldown` : `self->p_ = my_result` devient `fq_take_bool(my_result, &self->p_, &self->has_p_)`. Le reste de `ldown` est inchangé (garde `self` NULL, `mv(self,"my")`, `REF NULL` quand `my` est absent, `return true`). Aucune écriture directe de `fa`, `hd`, `my`, `m_` hors constructeur. |
| R7 | **Forme 1b.8/1b.12, sélecteur local** : `const pico8_value_t *member` et la copie commune sont retirés, remplacés par les branches R4. Aucune table `offsetof`, aucun `pico8_fq_member`. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_fq_t)` gardé ; `PICO8_MEMBER_CENSUS_WRITE` inchangé ; `tools/member_census.txt` non édité. |
| R9 | 195 → 190, aucune ligne `pico8_fq_t` ; les lignes `pico8_js_t` du même fichier restent. |

### R5, moitié écrivain, et R6 de `p_` : une aide `fq_take_bool`

1. **Noms écrits.** Une seule branche, `m_` (le seul membre `e`) : `if (strcmp(name, "m_") == 0) { fq_take_bool(value, &fq->m_, &fq->has_m_); return 1; } return 0;`. `fa`, `hd`, `my`, `p_` sont `l` seuls : pas de branche, leur écriture par nom reste « nom inconnu ».
2. **Aide `static` du genre `bool`**, au nom de la struct, forme par valeur : `static void fq_take_bool(pico8_value_t value, int *out, int *has)`. Une par genre (1b.13, point 2) : `m_` (écrivain) et `p_` (`ldown`) la partagent. (G) Son message nomme les deux membres, comme `lq_take_fixed` (« `lq.c/score` »).
3. **Nil d'abord, exactement comme `bg_take_bool`** : `*out = 0;` puis `*has = 0;` sans condition ; puis `if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) return;` — genre **et** pointeur. Nil éteint donc l'indicateur quel que soit l'état d'avant ; un `false` à union nulle n'est pas nil (genre testé), il est recopié, présent.
4. **Hors genre** : `assert(value.kind == PICO8_VALUE_BOOL && "fq.m_/p_: a write neither nil nor a boolean is off its census row (bool) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` puis `*out = value.as.boolean;` et `*has = 1;` en dernière instruction. Personne d'autre ne met `has_m_`/`has_p_` à 1.
5. **Nom inconnu : inchangé.** Pas d'`assert` dans l'écrivain ; `pico8_entity_set_field` avorte par son propre `assert(written && …)`, message inchangé.

### R5, moitié boucle, sur `my` : aide `fq_take_function`

`static void fq_take_function(pico8_value_t value, pico8_value_function_t *out)` : `*out = NULL;` puis sortie sur `REF NULL` (genre et pointeur) ; sinon `assert(value.kind == PICO8_VALUE_FUNCTION && value.as.function != NULL && "fq.my: a key neither nil nor a non-NULL function is off its census row (fonction) -- a field under no rule of plan-3 section 5: stop, do not widen the member")`, puis `*out = value.as.function`.

**Pourquoi la fonction NULL avorte** (G) : c'est la règle « union nulle » de 1b.13 (point 3) lue pour un pointeur de fonction. Un `FUNCTION` dont le pointeur est NULL est un non-REF à union nulle ; recopié, il donnerait `my` NULL, lu **absent**, alors que l'ancien code le lisait **présent** (`FUNCTION`) et que `mv` l'aurait appelé : la Coexistence exige « présent/absent identiques à 1a ». Aucun producteur ne l'écrit (H2), donc avorter ne change aucun chemin réel. Ce n'est pas une lecture nouvelle d'une règle. `mh.lh` (1b.1), antérieur à la règle, recopie une fonction NULL en nil : **non retouché** (hors périmètre), signalé dans la note.

### Preuves

Toutes sur l'arbre final, sauf mention. `FQ` désigne `sed -n '/^static pico8_value_t pico8_fq_ldown_dispatch/,$p' src/pico8_ui_button.c` (la partie `fq` est en fin de fichier ; le code de `js` au-dessus garde ses `pico8_value_t`).

- `make value-members | grep -c pico8_fq_t` : 5 avant ; **0** après ;
- `grep -rn 'offsetof(pico8_fq_t' src/` : vide avant comme après ;
- `FQ | grep -n 'pico8_value_t \*' | grep -v 'pico8_value_t \*out_value)'` : 1 ligne avant (`const pico8_value_t *member;`) ; **vide** après ;
- `FQ | grep -nE '(self|fq)->(my|p_|m_) = [^N0]'` : 6 lignes avant (écrivain, trois `REF NULL` de l'init, clé `my`, `ldown`) ; **vide** après (tout passe par les aides, R3 écrit `NULL`/`0`) ;
- `FQ | grep -nE '(->|\.)(fa|hd|my|p_|m_)\.(kind|as)'` : vide après ;
- `grep -c 'off its census row' src/pico8_ui_button.c` : 0 avant ; **2** après (`fq.m_/p_`, `fq.my`) ;
- `grep -n 'has_p_\|has_m_' src/pico8_ui_button.h` : 0 avant ; **2** après, chacune juste après son membre dans le typedef `pico8_fq_t` ;
- `git diff --stat` : seuls `src/pico8_ui_button.h`, `src/pico8_ui_button.c`, `tests/test_pico8_fq.c`, `docs/plan-3.md` ; le diff de `ui_button.c` ne touche aucune ligne au-dessus de `pico8_fq_ldown_dispatch`.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 195 sur l'arbre de départ (1b.17 commitée), puis 190.
- **H2** : dans les 15 rejeux et dans les TU hors fils de `must_abort`, `m_` d'un voile ne reçoit par nom que `BOOL` (de `gw` et des tests) ou, dans la caractérisation, `REF NULL` ; `my_result` de `ldown` n'est que `BOOL` (`pico8_pj_de_my_dispatch` rend `true`, `counting_my` des TU `true`/`false`) ou `REF NULL` ; la clé `my` n'est qu'une `FUNCTION` non NULL (`pico8_de()`, `counting_my`) ou, dans la caractérisation, `REF NULL`. Sinon une aide avorte : arrêt « champ sans règle ».
- **H3** : `pico8_ob_table_set_name(t, "my", pico8_value_ref(NULL))` garde l'entrée (`pico8_ob_table_set` ne retire rien), donc la clé nil atteint la boucle du constructeur (lu dans `src/pico8_ob.c`).
- **H4** : aucun TU de `UNIT_TESTS_SLOW` ne lit un membre de `fq` nil par nom d'une façon qui change R2 (revue ci-dessus).
- **H5** : `MY_TRUE_SEQUENCE_HASH`, `BARE_SEQUENCE_HASH`, `MY_FALSE_SEQUENCE_HASH`, `HOVER_SEQUENCE_HASH`, `HOVER_CURSOR_HASH`, `SCORE_PANEL_CLOSE_HASH`, `GAME_END_HASH` (`test_pico8_fq`), `STRUCT_HASH`/`SPEC_HASH` (`test_pico8_spec_defaults`), les empreintes de `test_pico8_ie`, `ui`, `bg`, `jc`, `pj`, `game_end`, `score_panel_close` et les 15 du filet ne bougent pas.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile sur une struct sans `pico8_value_t` propre. Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : aucune allocation ajoutée ni retirée.
- **H8** : `test_pico8_fq` reste sous 30 s avec les deux fonctions ajoutées (deux rejeux courts de 40 et 74 frames ; durée actuelle inconnue). Sinon l'étape s'arrête et le note ; le TU ne change pas de liste.
- **H9** : 1b.17, commitée, ne touche ni `pico8_ui_button.[ch]` ni `test_pico8_fq.c`.
- **H10** : `make class-coverage-net` rend `fq  3  2/15`, identique à `tools/class_coverage_net_expected.txt` (le corps du constructeur change, pas le nombre d'instanciations).
- **H11** : le lecteur brut `pico8_fq_struct_class()->field` rend `me` (`STRING "ph"`) et `t` (`FIXED 0`) par le repli `pico8_ke_field`, comme `pico8_entity_get_field`.
- **H12** : `test_pico8_fq`, appelé sans `in_fresh_child`, tolère deux fonctions de plus **en fin de `main`** : les tests existants gardent leur ordre et leurs valeurs figées ; les nouvelles appellent `pico8_jl()` d'abord.
- **H13** : les fils du test d'arrêt, créés par `fork` dans `test_pico8_fq` (TU de `CENSUS_UNIT_TESTS`), ne vident rien au recensement : ils meurent par `SIGABRT`, ou sur l'ancien code sortent par `_exit(0)` (1b.4b).
- **H14** : `pico8_mv(veil, "ldown", …)` sur un voile dont `my` rend une valeur hors genre atteint `fq_take_bool` dans le fils et y avorte, sans autre `assert` avant (`mv` recopie le retour tel quel).

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Aucun accès direct à `->fa`/`->m_`… dans les tests : ils compilent sur les deux formes.
- **Caractérisation dirigée, figée sur l'ancien code** (G) : `test_fq_members_by_name`, dans `test_pico8_fq` (rapide, dans `CENSUS_UNIT_TESTS`), appelée en fin de `main`. Elle lit par le **lecteur brut** (`pico8_fq_struct_class()->field`, témoin : fixe `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder ; elle écrit par l'**écrivain brut** (`->set_field`) et par `pico8_entity_set_field`.
  1. **Lectures par nom.** `pico8_jl()`, `bare = pico8_fq_new(NULL)`, `built = pico8_fq_new(pico8_de())`, `nil_key` = voile construit avec une table dont `my` = `REF NULL` (H3). Pour chacun : `fa` → `FIXED 14` ; `hd` → `FIXED 1` ; `my` → absent (0, témoin intact) pour `bare` et `nil_key`, `FUNCTION` de même pointeur que `de.my` (lu dans `pico8_de()`) pour `built` ; `p_`, `m_` → absents ; `me` → `STRING "ph"`, `t` → `FIXED 0` (H11). Intrus `""`, `"x"`, `"M_"`, `"P_"`, `"m"`, `"p"`, `"has_m_"`, `"has_p_"`, `"ke"`, `"header"` → 0, témoin intact.
  2. **`p_` par `ldown`** (seul chemin d'écriture). `bare` : `ldown` rend `true`, `p_` absent. Voile dont `my` est une fonction scriptée (tableau statique de retours, compteur d'appels) : `BOOL 1` → `p_` = `BOOL 1` présent ; `REF NULL` → **absent** ; `false` à union nulle (`memset`, `kind = BOOL`) → `BOOL 0` présent ; `BOOL 0` → `BOOL 0` présent ; `BOOL 1` → présent 1. À chaque pas, `ldown` rend `true`, un appel de `my` de plus, et `fa`, `hd`, `my`, `m_` inchangés.
  3. **Aller-retour d'écriture par nom de `m_`**, sur `bare` (forme 1b.15) : `pico8_entity_set_field(…, "m_", true)` → `BOOL 1` ; écrivain brut `false` rend 1 → `BOOL 0` **présent** ; `false` à union nulle → `BOOL 0` présent ; `REF NULL` → **absent** ; `REF NULL` encore → absent ; `true` → présent 1 ; `REF NULL` → absent ; `false` à union nulle → présent 0. À chaque pas, `fa`, `hd`, `my`, `p_` inchangés.
  4. **Écrivain brut sur un nom qu'il n'écrit pas** : `(bare, n, pico8_value_bool(1))` rend 0 pour `n` ∈ {`"fa"`, `"hd"`, `"my"`, `"p_"`, `"me"`, `"t"`, `""`, `"m_ "`, `"has_m_"`, `"has_p_"`} ; ensuite l'état lu est celui d'avant. Puis, par un utilitaire `must_abort(e, name)` ajouté au fichier (forme de `test_pico8_bg`, `fork` + `SIGABRT`) : `pico8_entity_set_field` de `"p_"`, `"my"`, `"fa"`, `"hd"` avorte — **déjà vrai sur l'ancien code** (« nom inconnu »), d'où sa place ici.
  5. `gw` n'est pas recopié : `test_hover_loop_on_the_veil` fige déjà `m_` `true` puis `false` par `gw`.
  6. On plie les retours, genres, fixes, booléens, compteurs d'appels et comptes lus dans **`FQ_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les pointeurs (`my`) sont jugés par `assert`.

  Lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite.
- **Test d'arrêt, rouge d'abord** (G) : `test_fq_writer_stops_off_row`, appelé après la caractérisation, avec un utilitaire `must_abort_value(void *e, void (*write)(void *, pico8_value_t), pico8_value_t v)` (forme de `test_pico8_bg`) ou une fonction par cas : chaque cas est un fils qui doit mourir par `SIGABRT`. Valeurs à union nulle construites par `memset(&v, 0, sizeof v)` puis `v.kind = …` : `zero_fixed`, `zero_string`, `zero_false`, `zero_function`.
  - **`m_` par nom** (écrivain) : fixe 1 ; `zero_fixed` ; `zero_string` ; `pico8_value_string("x")` ; `pico8_value_function(counting_my)` ; `zero_function` ; REF vers un autre voile (struct) ; `pico8_value_ref(pico8_de())` (table).
  - **`p_` par `ldown`** (R6 ; le fils construit un voile dont `my` rend `v`, puis `ldown`, H14) : fixe 1 ; `zero_fixed` ; `zero_string` ; `pico8_value_string("x")` ; REF vers un voile.
  - **`my` par clé de construction** (le fils construit un voile avec une table dont `my` = `v`) : fixe 1 ; `zero_fixed` ; `zero_false` ; **`zero_function`** (la fonction NULL) ; `pico8_value_string("x")` ; REF vers un voile.

  **Rouge sur l'ancien code** (l'écrivain, `ldown` et la boucle recopient tout, le fils sort par `_exit(0)`), vert après : une ligne « Runs » pour chacun des deux passages. Il n'est pas dans la caractérisation figée, qui ne contient que ce que l'ancien code fait déjà.
- **Tests existants, inchangés, qui figent déjà beaucoup** : `test_ldown_with_a_true_my_removes_the_veil`, `test_bare_veil_swallows_the_click_and_stays`, `test_false_my_keeps_the_veil` (`p_` absent, vrai, faux ; `dg`), `test_hover_loop_on_the_veil` (`m_` par `gw`), `test_defaults_match_the_parsed_spec` (`fa`, `hd`, `my` absent), `test_fq_is_a_struct_registered_under_hd` (`hd` par `mv`, `my` `FUNCTION`), les deux rejeux ; `test_struct_defaults` (`STRUCT_HASH`) ; `test_pico8_ie` (`hd`). Aucun test de régression (§6) ne lit ces membres ; `test_pico8_game_end` et `test_pico8_score_panel_close` ne font que compter les voiles.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_fq && ./tmp/test_pico8_fq` (et `test_pico8_spec_defaults` là où c'est dit). Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : (a) la branche R4 de `m_` ignore `has_m_` → rouge : caractérisation 1 et 3 ; (b) celle de `p_` ignore `has_p_` → rouge : `test_bare_veil_swallows_the_click_and_stays` (`p_` absent après clic), caractérisation 1 et 2.
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre** pour les fixes (`fa`/`hd` n'ont pas d'état absent). Forme la plus proche sur un membre typé : la branche `my` rend `pico8_value_function(NULL)` présent quand `my` est NULL → rouge : `test_defaults_match_the_parsed_spec` (`my` absent), caractérisation 1, et `ldown` d'un voile nu appelle un pointeur NULL (le TU meurt).
  3. (§7-3) *init ou boucle qui saute un membre typé*, `malloc` remplacé par `calloc` dans la copie pour un résultat déterministe : (a) `fa = 14` omis → rouge `test_defaults_match_the_parsed_spec`, `find_veil` des rejeux ; (b) `hd = 1` omis → rouge `test_fq_is_a_struct_registered_under_hd` (`mv(…, "hd")` = 1) ; (c) **boucle qui saute la clé `my`** : planté en gardant l'appel (`if (0) fq_take_function(…)`, 1b.14b, sinon `-Werror` l'arrête) → rouge `test_ldown_with_a_true_my_removes_the_veil` (`g_my_calls == 1`), `test_fq_is_a_struct_registered_under_hd` ; (d) `has_m_ = 0` ou `has_p_ = 0` omis : **trou justifié**, `calloc` pose déjà 0, et sous `malloc` l'omission est une lecture non initialisée qu'aucun outil du filet ne signale (ASan ne voit pas les lectures non initialisées) ; les lignes sont prouvées exécutées par `gcov`.
  4. Propre à l'étape, *nil jugé sur le pointeur seul* dans `fq_take_bool` → rouge : caractérisation 2 et 3 (`false` à union nulle lu absent), test d'arrêt (`zero_fixed`, `zero_string`, `zero_function` ne meurent plus).
  5. Propre à l'étape, *nil jugé sur le genre seul* (`kind == REF`) → rouge : test d'arrêt (REF de voile, REF de table ne meurent plus).
  6. Propre à l'étape, *nil n'éteint pas la présence* : l'aide teste nil **avant** `*out = 0; *has = 0;` → rouge : caractérisation 2 (`my` nil après vrai) et 3 (nil après `true`).
  7. Propre à l'étape, *écrivain qui accepte un autre nom* : une branche `p_` qui rend 1 → rouge : caractérisation 4 (écrivain brut `"p_"` rend 1, `must_abort(p_)` ne meurt plus).
  8. Propre à l'étape, *R6 muet* : `ldown` n'appelle plus `fq_take_bool` (l'aide reste appelée par l'écrivain, pas de `-Werror`) → rouge : `test_ldown_with_a_true_my_removes_the_veil` (le voile ne tombe pas), `test_false_my_keeps_the_veil`, caractérisation 2.
  9. Propre à l'étape, *fonction NULL acceptée* : `&& value.as.function != NULL` retiré de `fq_take_function` → rouge : test d'arrêt (`my` = `zero_function` ne meurt plus). C'est la mutation qui justifie ce cas.
- **`gcov`** : `test_pico8_fq` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23. Chaque ligne modifiée est exécutée (la sortie nil de `fq_take_function` l'est par le voile `nil_key`, H3) ; trou attendu : l'échec des deux `assert` d'aide, exécutés seulement dans les fils de `must_abort_value`, qui avortent sans vider leur compteur (prouvés par le test d'arrêt et les mutations 4, 5 et 9). Le rapport va dans le commit.
- **Prior art** : `test_bg_members_by_name` et `test_bg_writer_stops_off_row` (1b.15, `must_abort_value`, `zero_*`), `test_too_far_writer_stops_off_row` (1b.13, `write_fo_zero_false`), `counting_my`/`veil_with_my` de `test_pico8_fq`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_fq` : quelques secondes (H8) ; environ 15 lancements (caractérisation et test d'arrêt sur l'ancien code, nouveau code, 9 mutations dont 1 et 3 en plusieurs formes, `gcov`), 3 à 5 min ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make class-coverage-net` : 3 à 4 min, dans la même invocation que `make e2e` (arbres distincts, comme en 1b.14a et 1b.15) ;
- `make sanitize-net` : ~4 min seul ; il n'est **jamais** lancé en même temps que `e2e` ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent (aucun n'est touché) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 190`**, sans aucune ligne `pico8_fq_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**, parce que le diff touche le corps du constructeur `pico8_fq_struct_new` (R3, clé `my`), sans toucher `pico8_class_coverage_record("fq")` ni `KNOWN[]`. Sortie identique à `tools/class_coverage_net_expected.txt` (`fq  3  2/15`, H10).

`make member-census` ne s'applique pas (R8). On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements. On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si une aide avorte dans un rejeu ou dans un TU hors des fils de `must_abort`/`must_abort_value` (H2), si un membre ou un indicateur autre que `has_p_`/`has_m_` est ajouté, si l'écrivain accepte un autre nom que `m_`, ou si un type est élargi (union, `p_`/`m_` tenant un fixe, `my` tenant autre chose qu'une fonction) au lieu d'avorter.
- `FAILED:` si les types ne sont pas exactement `pico8_fixed_t fa`, `pico8_fixed_t hd`, `pico8_value_function_t my`, `int p_`, `int has_p_`, `int m_`, `int has_m_`, dans cet ordre, après `pico8_ke_t ke`.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur `my` NULL, `has_p_` = 0, `has_m_` = 0 ou un nom inconnu, ne rend pas le même pointeur pour `my`, ou si `me`/`t` ne passent plus par `pico8_ke_field`.
- `FAILED:` si `fq_take_bool` n'est pas l'unique aide qui écrit `m_` et `p_`, ne pose pas `*out = 0` et `*has = 0` **avant** le test de nil, si ce test ne vérifie pas le genre **et** le pointeur, si un `false` à union nulle est lu absent, ou si le message ne nomme pas `fq.m_`, `p_` et `bool`.
- `FAILED:` si `fq_take_function` accepte une fonction NULL, ne vérifie pas le genre `FUNCTION`, ou si son message ne nomme pas `fq.my` et `fonction` ; si l'`else` de la boucle des clés ou son message change.
- `FAILED:` si `ldown` change autrement que par l'appel de `fq_take_bool` (garde `self` NULL, `mv(self,"my")`, `REF NULL` quand `my` est absent, `return true`), ou si la signature d'une fonction publique de `fq` change.
- `FAILED:` si l'écrivain avorte lui-même sur un nom inconnu, ou si le message de l'`assert(written && …)` de `pico8_entity_set_field` change.
- `FAILED:` si un site par nom change (`pico8_dg`, `pico8_ui_gw`, `mv(self,"my")`, les tests), si le code de `js` (au-dessus de `pico8_fq_ldown_dispatch`) change, ou si `ui`, `menu`, `score_panel_display`, `bootstrap`, `mh` ou leurs lecteurs, écrivains ou tables changent.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation, le test d'arrêt, leurs utilitaires, `FQ_MEMBERS_HASH`, les includes nécessaires au `fork` et leurs appels en fin de `main`, ou si une valeur figée existante change (`MY_TRUE_SEQUENCE_HASH`, `BARE_SEQUENCE_HASH`, `MY_FALSE_SEQUENCE_HASH`, `HOVER_SEQUENCE_HASH`, `HOVER_CURSOR_HASH`, `SCORE_PANEL_CLOSE_HASH`, `GAME_END_HASH`, `STRUCT_HASH`…), ou si un test de régression (§6) change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 190, liste encore `pico8_fq_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit ou écrit un membre directement, ou si `FQ_MEMBERS_HASH` change après le typage ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'ancien code et une verte après, ou s'il ne porte pas les cas non-REF à union nulle construits par `memset` (dont `zero_function` sur la clé `my`).
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 9 mutations, chacune avec son test ou la justification de son trou.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.18 », ou le tableau « Runs ».
- `FAILED:` si le budget d'environ 20 minutes est dépassé sans que le journal note ce qui n'a pas été lancé.

## Out of Scope

- `pico8_js_t` et ses homonymes `my`, `m_`, `fa`, `hd` (1b.34), même fichier ; son écrivain, sa table `g_js_members[]` et `pico8_js_member`.
- `ui.o_`, `ui.gj` (cibles `ref:pico8_fq_t`, typées avec `ui`) ; les `m_` de `jo` (1b.23d), `fv` (1b.33).
- Aligner `mh.lh` (1b.1) sur le rejet de la fonction NULL.
- Typer le retour de `my` ou la signature de `pico8_fq_ldown` ; remplacer `mv(self,"my")` ou la lecture `"p_"` de `dg` par un accès direct.
- Une aide de déballage partagée entre fichiers ; retoucher les aides de 1b.5 à 1b.17.
- Régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - module : partie `fq` de `ui_button.[ch]` seule, un commit, sans découpage ;
  - tests : `test_pico8_fq` étendu (caractérisation + test d'arrêt, en fin de `main`), sans nouveau TU ;
  - caractérisation dirigée gardée alors que la ligne n'est pas « sans genre » : un écrivain par nom, un écrivain direct et une clé, et nil n'est écrit par nom dans aucun rejeu (H2) ;
  - `p_`, `m_` → `int` + `has_` (R1, R2), pas de `<m>_is_bool` (un seul genre) ; `my` → `pico8_value_function_t` sans indicateur ;
  - une seule aide `fq_take_bool` pour `m_` (écrivain) et `p_` (`ldown`), message aux deux noms (précédent `lq_take_fixed`) ;
  - `fq_take_function` rejette la fonction NULL (règle union nulle de 1b.13) ;
  - test d'arrêt couvrant les trois chemins d'écriture (écrivain, `ldown`, clé) : chacun a un `assert` atteignable par un appel public (raison de 1b.13) ;
  - mutation §7-3 rendue déterministe par `calloc` dans la copie ; `class-coverage-net` lancé.
- **Note « Découvert à 1b.18 »** proposée, sur la ligne 1b.18 de `docs/plan-3.md` : « patron 1b.1 appliqué ; R7 forme sélecteur local (1b.8) ; **R5 écrivain de 1b.15 recopiée sans écart** sur `m_` ; `p_`, écrit directement par `ldown` depuis `mv(self,"my")`, passe par la même aide `fq_take_bool` (une aide par genre et par struct, message aux deux noms comme `lq_take_fixed`) ; **genre `fonction` en boucle de clés** : `fq_take_function` avorte sur un `FUNCTION` à pointeur NULL (règle union nulle de 1b.13 : NULL y dirait nil, là où 1a lisait présent), cas `zero_function` au test d'arrêt ; `mh.lh` (1b.1) accepte encore la fonction NULL en nil, non retouché ; revue des TU lents sans effet ; empreinte réelle 6 (3, 3), 3 fichiers modifiés. » 1b.34 (`js.my`, `js.m_`) la recopie. Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 195 → 190, le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `FQ_MEMBERS_HASH` et les réponses à H1 à H14.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
