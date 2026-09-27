---
title: "Plan 3, étape 1b.21 : `iv` (le toast flottant) — sept membres typés, `p_` avec `has_p_`, `at1` en `const char *`, `at2` fixe sans indicateur, clés `ra`/`[1]`/`[2]` par trois aides R5 (1b.10), sélecteur local supprimé (R7 1b.8), pas d'écrivain par nom"
description: "`pico8_iv_t` a sept membres `pico8_value_t` (`fa`, `l` : `fixe`, jamais nil ; `v`, `ra` : `ref:autre`, des vecteurs ; `p_` : `bool`, nil tenu, lu par nom ; `at1` : `chaine` ; `at2` : `fixe`, tous deux jamais lus par nom). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 177 à 170. Seul `p_` reçoit un `has_`. `iv` n'a pas d'écrivain par nom : R5 ne s'exerce que dans la boucle des clés de construction, par trois aides `static` ; un test d'arrêt, rouge sur l'ancien code, fige les clés hors genre, dont les cas à union nulle construits par `memset`."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.21 — `iv` (`msgbox.h:69`). Empreinte 30 (11, 19). Compte : 170.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. S'appliquent aussi, sans être redérivées, les notes « Découvert à » de `docs/plan-3.md` :

- **1b.1**, puis **1b.3** à **1b.19** : patron appliqué ; `ref:autre` vers un vecteur → `pico8_vec_t *` emprunté, même pointeur (1b.4 ; `bg.mq` en 1b.15, `nf.ra` en 1b.17, `qb.v`/`ma` en 1b.19) ; `fixe` à nil `jamais` sans indicateur, même quand un homonyme en a un (1b.6) ; nil `stat?` d'homonyme ne décide rien (1b.7) ; R3 garde un **défaut module** partagé par toutes les instances, distinct de la spec (1b.17 `&g_nf_ra`, 1b.19 `&g_qb_v`) ; `chaine` → `const char *` emprunté, NULL = nil (1b.14a, `ke.me`) ; **lecture de R5 pour `ref:autre`** en boucle de clés : REF non NULL de tag `UNKNOWN`, `assert` sinon (1b.10, `bg_take_vec` en 1b.15) ; **R5 boucle de clés directe** (1b.10) : une branche par clé qui appelle une aide `static` par genre, la clé inconnue garde son `assert` et son message dans l'`else` final ; un fixe sans indicateur reçu par clé : nil avorte (1b.10 `lq.w`, 1b.19 `qb_take_fixed`) ;
- **1b.8, R7 forme sélecteur local** (appliquée sans écart en 1b.12 et 1b.18) : le `const pico8_value_t *member` du lecteur est retiré, remplacé par des branches R4 explicites ; **1b.9/1b.10, R7 forme `offsetof`** : sans objet ici (`grep -rn 'offsetof(pico8_iv_t' src/` est déjà vide), mais sa preuve mécanique `grep -n 'pico8_value_t \*' … | grep -v 'pico8_value_t \*out_value)'` s'applique ;
- **1b.13/1b.15, forme de R5** : chaque aide pose **d'abord** l'état nil, **puis** sort sur `REF NULL` — genre **et** pointeur testés —, sinon `assert` « champ sans règle » qui nomme `struct.membre` et le genre de sa ligne, puis recopie ; un non-pointeur sans indicateur n'a pas d'état nil, son aide commence par l'`assert` et nil y avorte ; **chaque test d'arrêt porte les cas non-REF à union entièrement nulle, construits par `memset` puis `kind`** (1b.13 `write_fo_zero_false`, 1b.14b, 1b.15 `zero_fixed`/`zero_string`/`zero_false`, 1b.18 `zero_function`, 1b.19 `zero_ref`) ; une chaîne à pointeur NULL n'est pas nil (1b.14a : « chaîne NULL refusée ») ; un fixe 0 à union nulle est le témoin accepté (1b.14b) ;
- **1b.14b** : une mutation qui retire l'unique appel d'une aide `static` est arrêtée par `-Werror` ; on la plante sous une forme qui garde l'appel (`if (0)`, clé renommée) ;
- **1b.16** : quand R5 écrivain est vacante, les cas `memset` sont figés comme **écritures par nom avortées** ;
- **1b.11, règle R2 des TU lents** : une lecture par nom faite par un TU de `UNIT_TESTS_SLOW` compte comme `l`, et le nil qu'elle lit comme `lu`. **Relus plus bas : aucun ne lit un membre d'`iv`.**
- **1b.5, 1b.7, 1b.16, lectures de R1 pour genres mêlés** : **sans objet** — chaque ligne d'`iv` n'a qu'un genre.

### État de départ

HEAD `ea1e5fb` (PRD de 1b.20). **1b.20 (`ie`) est en cours d'implémentation.** **Précondition : elle est commitée avant que 1b.21 commence.** Son PRD ne touche ni `src/pico8_msgbox.[ch]`, ni `tests/test_pico8_iv.c`, ni `tests/test_pico8_hy.c`, ni `tools/float_audit_c1.txt` (H9) ; dans `docs/plan-3.md`, 1b.20 écrit sa ligne, 1b.21 la sienne. `make value-members` rend alors 177 (H1). Les numéros de ligne cités sont ceux de HEAD et peuvent glisser : chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H17).

### Lignes de recensement (`tools/member_census.txt:50-56`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `fa` | offset | `fixe` | jamais | - | l |
| `l` | offset | `fixe` | jamais | - | l |
| `v` | offset | `ref:autre` | jamais | - | l |
| `ra` | offset | `ref:autre` | observé | stat? | l |
| `p_` | offset | `bool` | tenu | - | l |
| `at1` | offset | `chaine` | tenu | - | **-** |
| `at2` | offset | `fixe` | tenu | - | **-** |

Sept membres : 177 − 7 = **170**, comme le dit la ligne. Aucune ligne `e` : **`iv` n'a pas d'écrivain par nom** (`g_iv_struct_class.set_field` est `NULL`).

**Revue des TU lents (règle R2 de 1b.11).** `test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` ne nomment ni `pico8_iv`, ni `msgbox`, ni `iv` (`grep -cE 'pico8_iv|msgbox|\biv\b'` à 0 pour les huit). Leurs lectures par nom sur `hv` (`test_pico8_jo.c:497`, `test_pico8_qh.c:644`…) sont filtrées par `daughter_of`/`descendant_of` (filles de `jo`) ; leurs `"p_"` visent `ie`, `jo`, `qh`. Leurs rejeux construisent des `iv` (`jo:hk`, `hy:lu`) et les lisent par les sites de `src/` (`dc`, `dg`), inchangés. **Aucune lecture nil par nom d'un membre d'`iv`** (H4) : la revue est sans effet.

**Décision R2.** `has_p_` seul : `p_` est un non-pointeur lu par nom (`l`) à nil `tenu` — lettre de R2, sans preuve contraire (au contraire : la caractérisation le lit nil par nom sur un `iv` neuf, avant tout `ph`). Aucun autre : `fa`, `l` nil `jamais` (1b.6) ; `v`, `ra`, `at1` pointeurs (NULL = nil) ; `at2` non-pointeur **non lu par nom** (`-`), donc hors de R2 (voir « `at2` » plus bas).

### Ce que fait le cart (`pico8/lua.lua:941-951`)

- `iv=ke:jr([[fa=9,l=0,v=v(0,-0.34),]])` ;
- `iv:pq` : `self.ra=self.ra or je.ra+mo.ra-v(0,5)` ;
- `iv:ph` : `self.ra+=self.v`, `self.l+=0.1`, `self.p_=self.l>=5` ;
- `iv:kj(p)` : `dv(self.l) io(self[1],p.x,p.y,self[2],0.5) dv()`.
- Constructions : `iv({ra=self.ra,ne(self.kr,10,"$")})` (`jo:hk`, `:516`), `iv({"-no money-",8})` et `iv({ne(-jt,10,"$")})` (`hy:lu`, `:1218`, `:1223`). `ne(d,c,ku)` rend toujours un `c` non nil quand `c` est passé (`:339-345`) : **dans le cart, `self[2]` est toujours un nombre** et `self[1]` toujours une chaîne.

### Le code aujourd'hui

`src/pico8_msgbox.h:66-77` : `{ pico8_ke_t ke; pico8_value_t fa, l, v, ra, p_, at1, at2; } pico8_iv_t` (un membre par ligne), sous le contrat « `at1`/`at2` are the cart's `self[1]` (the text, a string never freed) and `self[2]` (the forced color). Never freed, nor are the vectors `ra` points at: TROU-7. `v` points at one vector shared by every iv. » API publique inchangée : `pico8_iv_init`/`_shutdown`, `_ready`, `_default`, `pico8_iv_new`, `pico8_iv_pq`, `pico8_iv_ph`, `pico8_iv_kj`.

`src/pico8_msgbox.c`, partie `iv` (`:235-419`) :

- **`g_iv_v`** (`:235`) : `static pico8_vec_t` (0, −0.34), défaut module de `v`, distinct du `v` de `g_iv_spec` (même valeur, autre REF) ;
- **`pico8_iv_value_fixed`** (`:237`), **`pico8_iv_value_vec`** (`:241`) : déballeurs de `pico8_value_t` (mauvais genre → 0 / (0,0)) ; appelants : `ph` (`ra`, `v`, `l`) et `kj` (`l`) ;
- **`pico8_iv_pq`** (`:248-263`) : `if (!pico8_msgbox_truthy(self->ra))` → vecteur neuf du tas `je.ra+mo.ra-(0,5)`, `self->ra = pico8_value_ref(heap_ra)` (TROU-7) ;
- **`pico8_iv_ph`** (`:265-284`) : `self->ra = pico8_value_ref(heap_ra)` (vecteur neuf, TROU-7), `self->l = pico8_value_fixed(new_l)`, `self->p_ = pico8_value_bool(new_l >= 5)` ;
- **`pico8_iv_kj`** (`:294-319`) : `pico8_dv(mem, pico8_iv_value_fixed(self->l))` ; `text_val = self->at1`, `color_val = self->at2` ; texte si `STRING`, sinon `""` ; couleur `pico8_fixed_flr_int(color_val.as.fixed)` si `FIXED`, sinon 0 — **site C1 de `tools/float_audit_c1.txt:31`** : `src/pico8_msgbox.c | pico8_iv_kj | color_val.as.fixed | 1` ;
- **Lecteur** `pico8_iv_field` (`:331-353`) : sélecteur local `const pico8_value_t *member` sur `fa`, `l`, `v`, `ra`, `p_` ; `REF NULL` → 0 ; autre nom → `pico8_ke_field`. **`at1`/`at2` ne sont pas lisibles par nom** (ils tombent dans `pico8_ke_field`, qui rend 0) ;
- **Écrivain** : `NULL` (`:358`) ; `pico8_entity_set_field` sur un `iv` avorte donc par « a struct a generic writer reaches needs a field writer » (`entity.c:197`) ;
- **Constructeur** `pico8_iv_struct_new` (`:361-395`) : `malloc`, `pico8_ke_struct_init`, `fa` = fixe 9, `l` = fixe 0, `v` = `pico8_value_ref(&g_iv_v)`, `ra`/`p_`/`at1`/`at2` = `REF NULL` ; boucle des clés : `ra` nommée → `self->ra = value`, positionnelle 1 → `self->at1 = value`, 2 → `self->at2 = value` (n'importe quel genre), sinon `assert(0 && "iv: every measured key is ra, [1] or [2]; …")` ; `pico8_class_coverage_record("iv")` ; `pico8_iv_pq` ; `pico8_ke_struct_pq` ; `PICO8_MEMBER_CENSUS_SEE(self, pico8_iv_t)`.
- `pico8_msgbox_truthy` et `pico8_msgbox_field_vec` servent aussi `mh:ev` et `pz` : **gardés**.

**Producteurs.** `pico8_hy_lu` (`economy.c:262-282`) : `[1]` = `pico8_value_string("-no money-")` ou le texte `malloc` de `pico8_ne`, `[2]` = `pico8_value_fixed(8 | color)`. `pico8_jo_hk` (`planet.c:634-643`) : `ra` = `self->ra` de la planète (REF de vecteur, seulement si présente), `[1]`, `[2]` idem. TU : `test_pico8_iv` (`build` : `ra` optionnelle, `[1]`, `[2]` ; `generic` : `[1]` seul), `test_pico8_hy` (`iv_descriptor` : `[1]` seul), `test_pico8_ui` et `test_pico8_spec_defaults` (`pico8_iv_new(NULL)`).

**Lecteurs par nom.** `dg` (`bootstrap.c:91`, `p_` sur chaque entité de `hv`, **après** l'appel de la méthode `me` = `ph`) ; `dc` (`entity.c:307`, `fa` ; `:336`, `ra` passé à `kj`) ; `test_pico8_iv` (`field`/`mv` : `ra`, `l`, `t`) ; `test_pico8_spec_defaults` (`STRUCT_HASH`, `NAMES` contient `fa`, `l`, `ra`, `v`).

**Accès directs dans les tests** (ne compileront plus) : `test_pico8_iv.c:264-273` (`test_defaults_match_the_parsed_spec` : `e->v.kind`, `e->v.as.ref`, `e->fa.kind`/`.as.fixed`, `e->l.kind`/`.as.fixed`, identité `->v.as.ref`) ; `test_pico8_hy.c:268-269` (`fold_new_entities` : `fold_value(h, iv->at1, 0)`, `fold_value(h, iv->at2, 0)`) et `:405` (`->at1.as.string`). `test_pico8_iv.c:218` (`f->p_`) vise `fake_toast_t`, pas `iv` : non touché.

### `at2` : pourquoi un fixe sans indicateur (G)

`at2` est `fixe` à nil `tenu`, mais **non lu par nom** (`-`) : R2 ne lui donne pas d'indicateur. Sa seule lecture, `kj`, rend la même couleur pour nil et pour 0 (`REF NULL` → `color = 0` ; `FIXED 0` → `pico8_fixed_flr_int(0)` = 0). Donc `pico8_fixed_t at2`, R3 à 0 quand la clé `[2]` manque, et **une clé `[2]` nil avorte** (1b.10 `lq.w`, 1b.19 `qb_take_fixed`) : aucun producteur ne l'écrit (cart : `ne` rend toujours `c` ; C : `pico8_value_fixed` partout). La caractérisation fige l'égalité d'écran « `[2]` absent » = « `[2]` = 0 » sur l'ancien code : c'est la preuve qui porte la décision.

### Genres mêlés : aucun

Chaque ligne n'a qu'un genre, vérifié côté écrivains : `fa`, `l` que des fixes (init, `ph`) ; `v` que `&g_iv_v` ; `ra` que des REF de vecteur (clé de `jo:hk`, tas de `pq`/`ph`) ; `p_` que des booléens (`ph`) ; `at1` que des chaînes non NULL ; `at2` que des fixes. **Aucune lecture de R1 pour genres mêlés** n'est décidée ici.

### Empreinte remesurée

La ligne dit 30 (11, 19). Les deux greps de la puce **Empreinte**, relancés tels qu'écrits, rendent **30 (10, 20)** : accès direct `msgbox.c`, `test_pico8_hy.c`, `test_pico8_iv.c` ; par nom 28 fichiers (10 de `src/`, 18 de `tests/`), union 30. L'écart de répartition (un fichier) vient des homonymes (`fa`, `l`, `v`, `ra`, `p_`) ; c'est un majorant.

**Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_iv_t`, directement ou par nom sur un `iv`) : **7 (4, 3)** —

- `src/` (4) : `pico8_msgbox.h` (typedef), `pico8_msgbox.c` (direct), `pico8_entity.c` (`dc` : `fa`, `ra`), `pico8_bootstrap.c` (`dg` : `p_`) ;
- `tests/` (3) : `test_pico8_iv` (direct et par nom), `test_pico8_hy` (direct : `at1`, `at2`), `test_pico8_spec_defaults` (`STRUCT_HASH` : `fa`, `l`, `ra`, `v`).
- Hors compte : `economy.c`, `planet.c` (construisent), `test_pico8_ui` (construit un `iv`, n'en lit que `me`/`t`).

**Fichiers modifiés : 5**, plus `docs/plan-3.md` : `src/pico8_msgbox.h`, `src/pico8_msgbox.c`, `tests/test_pico8_iv.c`, `tests/test_pico8_hy.c` (façon de lire seulement), `tools/float_audit_c1.txt` (une ligne, l'extrait du site C1 de `kj`).

**TU de `UNIT_TESTS_SLOW` touchés : aucun par diff.** Aucun ne nomme `iv` ; tous les huit sont reliés contre le nouveau `msgbox.c` (`ENGINE_SRCS`) et exercent `iv` par leurs rejeux (toasts de prix et « -no money- »), une fois, par `make e2e`. **Aucun test de régression (§6)** ne nomme `iv` (`test_pico8_hy` et `test_pico8_iv` ne sont pas de la liste des 17).

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code, par nom**, dans le TU rapide `test_pico8_iv` : `test_defaults_match_the_parsed_spec` réécrite pour lire par nom (même intention), et une nouvelle `test_iv_members_by_name` (voir Testing Decisions). Verte sur l'arbre de départ, avant tout typage.
2. **Typage** (en-tête `ke` en tête, noms et ordre gardés, `has_p_` juste après `p_`) :

   ```c
   pico8_ke_t ke;
   pico8_fixed_t fa;
   pico8_fixed_t l;
   pico8_vec_t *v;
   pico8_vec_t *ra;
   int p_;
   int has_p_; /* 0: `p_` is nil, and `p_` is then never read */
   const char *at1;
   pico8_fixed_t at2;
   ```

3. **Lecteur** : le sélecteur local sort (R7 forme 1b.8) ; cinq branches R4 `strcmp` dans l'ordre du typedef (`fa`, `l`, `v`, `ra`, `p_`), puis le repli `pico8_ke_field`. `at1`/`at2` restent non lisibles par nom.
4. **Écrivain** : **aucun** ; `set_field` reste `NULL` (G, R5 écrivain vacante).
5. **Clés** : trois aides R5 `static`, `iv_take_vec` (`ra`), `iv_take_string` (`[1]`), `iv_take_fixed` (`[2]`) ; l'`assert` des clés inconnues reste tel quel.
6. **R6** : `pq`, `ph`, `kj` écrivent et lisent les membres typés ; `pico8_iv_value_fixed` disparaît, `pico8_iv_value_vec` devient `pico8_iv_vec_or_zero(const pico8_vec_t *)`.
7. **Un test d'arrêt**, rouge sur l'ancien code (qui recopie tout), vert après, avec les cas à union nulle.
8. `tools/float_audit_c1.txt:31` : extrait `color_val.as.fixed` → `self->at2`, rien d'autre.
9. Effet mesurable : `make value-members` passe de 177 à **170** et ne liste plus aucune ligne `pico8_iv_t`. Aucun pixel ne bouge.
10. **Note « Découvert à 1b.21 »**.

**Pas de découpage** (G) : un seul `.c` porte le code (une soixantaine de lignes de diff), empreinte réelle 7.

## User Stories

1. En tant que lecteur de `msgbox.h`, je veux `fa`/`l` en `pico8_fixed_t`, `v`/`ra` en `pico8_vec_t *`, `p_` en `int` suivi de `has_p_`, `at1` en `const char *`, `at2` en `pico8_fixed_t`, afin que ce que tient le toast se lise sur le type.
2. En tant que `dc`, je veux lire par nom `fa` = `FIXED 9` et `ra` (REF, même pointeur que le membre) comme aujourd'hui, pour trier la couche et passer `ra` à `kj`.
3. En tant que `dg`, je veux lire `p_` par nom après `ph` : `BOOL 0` présent tant que `l` < 5, `BOOL 1` à 5, pour retirer le toast de `hv` au même tick qu'aujourd'hui.
4. En tant que lecteur par nom de `p_`, je veux 0 avec `*out_value` intact sur un `iv` neuf (avant tout `ph`), jamais « présent `false` ».
5. En tant que `STRUCT_HASH`, je veux `fa`, `l`, `ra`, `v` d'un `iv` neuf lus par nom comme aujourd'hui, et `v` = même valeur que la spec mais REF distincte (défaut module `&g_iv_v`).
6. En tant que `jo:hk`, je veux construire un toast avec la `ra` de la planète et le relire par nom **même pointeur**, puis `ph` le rebinder sur un vecteur neuf.
7. En tant que `hy:lu`, je veux un toast sans `ra` placé par `pq` sur `je.ra+mo.ra-(0,5)`, vecteur neuf par toast.
8. En tant que `iv:kj`, je veux le même dessin qu'aujourd'hui : texte `at1` (ou `""` quand nil), couleur `flr(at2)` (0 quand `[2]` manque), palette `dv(l)`.
9. En tant que mainteneur, je veux qu'une clé `ra` ni nil ni REF de tag `UNKNOWN` (fixe, fixe 0 à union nulle, `false` à union nulle, chaîne, chaîne à union nulle, fonction à union nulle, REF de table, de liste, d'un `iv`) avorte par l'`assert` qui nomme `iv.ra` et `ref:autre`.
10. En tant que mainteneur, je veux qu'une clé `[1]` ni nil ni chaîne non NULL (fixe, fixe 0 à union nulle, `false` à union nulle, `true`, **chaîne à pointeur NULL**, fonction à union nulle, REF de vecteur, de table) avorte par l'`assert` qui nomme `iv.at1` et `chaine`.
11. En tant que mainteneur, je veux qu'une clé `[2]` qui n'est pas un fixe (**nil**, `false` à union nulle, `true`, chaîne, chaîne à union nulle, fonction à union nulle, REF de vecteur) avorte par l'`assert` qui nomme `iv.at2` et `fixe`, et qu'un fixe 0 à union nulle soit accepté comme la couleur 0.
12. En tant que mainteneur, je veux qu'une clé `ra` nil soit acceptée (NULL, puis `pq` place le toast) comme aujourd'hui, et qu'une clé `[1]` nil laisse `at1` à NULL (dessin `""`).
13. En tant que mainteneur, je veux que toute écriture par nom sur un `iv` (`fa`, `l`, `v`, `ra`, `p_`, `at1`, `at2`, y compris nil et les valeurs à union nulle) avorte comme aujourd'hui, par l'`assert` de la couche entité.
14. En tant que lecteur par nom, je veux `at1`, `at2`, `has_p_`, `ke` et les noms inconnus → 0, témoin intact, et `me`/`t` servis par `pico8_ke_field`.
15. En tant que `test_pico8_hy`, je veux plier `at1`/`at2` d'un toast neuf comme avant (même `LU_HASH`, `HK_HASH`, rejeux), en relisant les membres typés re-emballés.
16. En tant que `make float-audit`, je veux le site C1 de `pico8_iv_kj` retrouvé sous son nouvel extrait, atteint, sans ECART ni NEGATIF.
17. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
18. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
19. En tant que gardien de la couverture, je veux `make class-coverage-net` identique à sa référence (`iv  474  9/15`), puisque le constructeur change.
20. En tant que `plan-pilot`, je veux `make value-members` à 170.
21. En tant qu'auteur des étapes suivantes (`jo.ra`, `g_`, `js`), je veux une forme écrite de R5 sur une clé positionnelle `chaine` et sur une clé positionnelle `fixe` sans indicateur.
22. En tant que vérificateur, je veux la caractérisation lancée sur l'ancien code, le test d'arrêt rouge puis vert, les mutations du §7 avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées à `iv`

| règle | sur `iv` |
|---|---|
| R1 | Voir la Solution. `fa`, `l`, `at2` → `pico8_fixed_t` ; `v`, `ra` → `pico8_vec_t *` (1b.4), non `const` ; `p_` → `int` ; `at1` → `const char *` (1b.14a : la chaîne est empruntée ; le texte `malloc` de `pico8_ne` n'est jamais libéré, TROU-7, comme aujourd'hui). Noms et ordre gardés. Aucun genre mêlé. Aucun `#include` ajouté. |
| R2 | `has_p_` seul, juste après `p_` (ligne `l`, nil `tenu`). Aucun autre (voir « Décision R2 » et « `at2` »). Contrat au format de `mh.has_p_`. |
| R3 | Dans `pico8_iv_struct_new`, à la place des sept affectations, même position (après `pico8_ke_struct_init`, avant la boucle des clés, `pico8_class_coverage_record`, `pico8_iv_pq` et `pico8_ke_struct_pq`) : `fa = pico8_fixed_from_int(9)`, `l = pico8_fixed_from_int(0)`, `v = &g_iv_v`, `ra = NULL`, `p_ = 0`, `has_p_ = 0`, `at1 = NULL`, `at2 = pico8_fixed_from_int(0)`. Tous écrits (bloc de `malloc`). **Défaut module, même REF** (1b.17) : `v` = `&g_iv_v`, partagé, distinct de la spec. |
| R4 | Cinq branches `strcmp` dans l'ordre du typedef, avant `pico8_ke_field` : `fa`, `l` : toujours présents, `pico8_value_fixed` ; `v`, `ra` : NULL → `return 0`, sinon `pico8_value_ref(<m>)`, même pointeur ; `p_` : `has_p_` = 0 → `return 0`, sinon `pico8_value_bool(p_)`. Dans chaque 0, `*out_value` intact. Forme de `pico8_mh_field` du même fichier. |
| R5 | **Écrivain : vacant** (G) — pas de ligne `e`, `set_field` reste `NULL`, rien n'est ajouté. **Boucle de clés** (1b.10) : les trois branches gardent leur test de clé et leur ordre ; `self->ra = value` → `iv_take_vec(value, &self->ra)`, `self->at1 = value` → `iv_take_string(value, &self->at1)`, `self->at2 = value` → `iv_take_fixed(value, &self->at2)` ; l'`else` et son `assert(0 && "iv: every measured key is ra, [1] or [2]; …")` restent tels quels. |
| R6 | `pq` : `!pico8_msgbox_truthy(self->ra)` → `self->ra == NULL` ; `self->ra = pico8_value_ref(heap_ra)` → `self->ra = heap_ra`. `ph` : `new_ra = pico8_vec_add(pico8_iv_vec_or_zero(self->ra), pico8_iv_vec_or_zero(self->v))` ; `self->ra = heap_ra` ; `new_l = pico8_fixed_add(self->l, …0.1)` ; `self->l = new_l` ; `self->p_ = new_l >= pico8_fixed_from_int(5); self->has_p_ = 1;`. `kj` : `pico8_dv(mem, self->l)` ; `text = self->at1 != NULL ? self->at1 : ""` ; `color = pico8_fixed_flr_int(self->at2)` ; les locales `text_val`/`color_val` sortent. `pico8_iv_value_fixed` n'a plus d'appelant : supprimée (sinon `-Werror`). `pico8_iv_value_vec` devient `static pico8_vec_t pico8_iv_vec_or_zero(const pico8_vec_t *p)` (NULL → (0,0), sinon `*p`) : même repli que 1a sur un pointeur que `pq` et R3 ne laissent jamais NULL (G : le repli coûte deux lignes et ne change aucun comportement). Ordre des instructions, `malloc` des vecteurs (TROU-7), constante 0.1, seuil 5 et `pico8_dv_default` inchangés. |
| R7 | **Forme sélecteur local de 1b.8** : `const pico8_value_t *member` et sa recopie `*out_value = *member` sortent avec les cinq `member = &iv->…`. Aucune table `offsetof` (déjà vrai). |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_iv_t)` gardé ; `tools/member_census.txt` non édité. |
| R9 | 177 → 170, aucune ligne `pico8_iv_t`. `has_p_` ne compte pas. |

### R5 : trois aides `static`, une par genre

Toutes par valeur (1b.10), au nom de la struct (`iv_take_*`, forme de `bg_take_vec`), placées avant `pico8_iv_struct_new`. Les aides de `mh` (1b.1, en ligne) ne sont pas retouchées.

1. **`static void iv_take_vec(pico8_value_t value, pico8_vec_t **out)`**, copie de `bg_take_vec` : `*out = NULL;` ; `if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) return;` ; `assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN && "iv.ra: a key neither nil nor a vector ref is off its census row (ref:autre) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` ; `*out = value.as.ref;`. Une clé `ra` nil garde le comportement de 1a (`pq` place le toast).
2. **`static void iv_take_string(pico8_value_t value, const char **out)`** : `*out = NULL;` ; sortie sur `REF NULL` (genre **et** pointeur) ; `assert(value.kind == PICO8_VALUE_STRING && value.as.string != NULL && "iv.at1: a key neither nil nor a string is off its census row (chaine) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` ; `*out = value.as.string;`. La chaîne à pointeur NULL avorte (1b.13 point 3, 1b.14a) : NULL y dirait nil, là où 1a tenait une chaîne présente.
3. **`static void iv_take_fixed(pico8_value_t value, pico8_fixed_t *out)`** (non-pointeur sans indicateur, forme `qb_take_fixed` de 1b.19) : commence par `assert(value.kind == PICO8_VALUE_FIXED && "iv.at2: a key that is not a fixed is off its census row (fixe) and at2 has no presence flag -- a field under no rule of plan-3 section 5: stop, do not widen the member");` ; `*out = value.as.fixed;`. **Nil avorte** ; un fixe 0 à union nulle est accepté.
4. **Nom inconnu : inchangé.**

### Site C1 de `float-audit` (G)

`kj` appelle `pico8_fixed_flr_int(self->at2)` sans condition. L'extrait du site change : `tools/float_audit_c1.txt:31` devient `src/pico8_msgbox.c | pico8_iv_kj | self->at2 | 1`, **seule ligne modifiée du fichier**. Précédent : 1a.26 (`288e7f6`) a réécrit l'extrait `fixed_of(self->mi)` de `pico8_jo_kj` dans son commit de conversion. Sans cette ligne, `float_audit.sh` sort en 5 (« entrée sans site ») et le site passe en D. Écarté : garder une locale `pico8_value_t color_val = pico8_value_fixed(self->at2)` pour conserver l'extrait mot pour mot — un emballage mort écrit pour un outil. Dans les rejeux, `at2` est toujours présent (producteurs) : le nombre d'appels du site ne change pas (H13).

### Commentaires (règle de `CLAUDE.md`)

- Contrat du typedef (`msgbox.h`) : « `at1`/`at2` are the cart's `self[1]` (the text, a string never freed) and `self[2]` (the forced color). » garde son sens ; on ajoute, contrat C que le nom ne dit pas : « `at1` NULL is nil ; a missing `[2]` leaves `at2` at 0, the color `kj` gave nil. » Le reste (« Never freed, nor are the vectors `ra` points at: TROU-7. `v` points at one vector shared by every iv. ») reste.
- `has_p_` : « 0: `p_` is nil, and `p_` is then never read », format de `mh.has_p_`.
- Contrat de `pico8_iv_new` (« `overrides` is read, not kept: `ra`, `[1]` and `[2]` only… ») : exact, inchangé.
- Commentaire de module de `msgbox.h` (« an absent or wrong-kind field reads as its zero/false default ») : il couvre aussi `mh` et les lecteurs `pico8_msgbox_field_vec` d'autres entités ; **inchangé** (G, 1b.1 l'avait gardé pour `mh`).
- Aucun autre commentaire ; les aides portent leur contrat par leur `assert`.

### Preuves

Toutes sur l'arbre final, sauf mention. `F` = `src/pico8_msgbox.c`.

- `make value-members | grep -c pico8_iv_t` : 7 avant ; **0** après ; total **170** ;
- `grep -n 'const pico8_value_t \*member' F` : 1 avant (`:333`) ; **vide** après ;
- `grep -n 'pico8_value_t \*' F | grep -v 'pico8_value_t \*out_value)'` : 1 avant (`:333`) ; **vide** après ;
- `grep -rn 'offsetof(pico8_iv_t' src/ tests/` : vide avant et après ;
- `grep -nE 'self->(fa|l|v|ra|p_|at1|at2) = (pico8_value_|value;)' F` : 14 lignes avant (init 7, boucle 3, `pq` 1, `ph` 3) ; **vide** après ;
- `grep -c 'pico8_iv_value_fixed\|pico8_iv_value_vec' F` : 5 avant ; **0** après ;
- `grep -c 'text_val\|color_val' F` : 8 avant ; **0** après ;
- `grep -c 'off its census row' F` : 2 avant (`mh.rh`, `mh.lh`) ; **5** après (+ `iv.ra`, `iv.at1`, `iv.at2`) ;
- `grep -c 'has_p_' src/pico8_msgbox.h` : 1 avant (`mh`) ; **2** après, la nouvelle juste après `int p_;` de `pico8_iv_t` ;
- `grep -nE '(->|\.)(fa|l|v|ra|at1|at2)\.(kind|as)' tests/test_pico8_iv.c tests/test_pico8_hy.c` : 7 lignes avant (`iv.c:264-268,270,273`, `hy.c:405`) ; **vide** après ;
- `grep -n 'pico8_iv_kj' tools/float_audit_c1.txt` : `color_val.as.fixed` avant ; `self->at2` après ; `git diff tools/float_audit_c1.txt` : une ligne `-`, une ligne `+` ;
- `git diff --stat` : seuls les cinq fichiers listés plus haut et `docs/plan-3.md` ; dans `F`, aucune ligne de la partie `mh` ni de `pz` ne change.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 177 sur l'arbre de départ (1b.20 commitée), puis 170.
- **H2** : dans les 15 rejeux et dans les TU hors fils de `must_abort*`, la clé `ra` d'un `iv` n'est qu'une REF de vecteur (tag `UNKNOWN`) ; `[1]` qu'une `STRING` non NULL ; `[2]` qu'un `FIXED`. Sinon une aide avorte : arrêt « champ sans règle ».
- **H3** : `pico8_type_tag_of` rend `UNKNOWN` sur la `ra` d'une planète (vecteur du tas, `jo.ra` non typé à cette étape) et sur les vecteurs de `build` ; une REF de genre ou de boîte en clé `ra` n'avorterait donc pas (limite connue de `ref:autre`, trou justifié).
- **H4** : aucun TU lent ne lit un membre d'`iv` par nom (revue ci-dessus).
- **H5** : ne bougent pas : `NO_MONEY_SEQUENCE_HASH`, `NO_MONEY_FIRST_DRAW_HASH`, `PRICE_SEQUENCE_HASH`, `PRICE_FIRST_DRAW_HASH` et les comptes de ticks (51) de `test_pico8_iv` ; `CONSTRUCTION_HASH`, `LU_HASH`, `HK_HASH`, `TECH_REPLAY_HASH`, `GAME_END_REPLAY_HASH` et les autres de `test_pico8_hy` ; `STRUCT_HASH`/`SPEC_HASH`/`READY_HASH` de `test_pico8_spec_defaults` ; les empreintes de `test_pico8_ui` ; les 15 du filet.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile sur `pico8_iv_t` sans `pico8_value_t`. Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : aucune allocation ajoutée ni retirée (les `malloc` de `pq`/`ph` restent, TROU-7).
- **H8** : `test_pico8_iv` reste sous 10 s avec les deux fonctions ajoutées (une cinquantaine de fils `fork` courts et 50 `ph`), bien sous 30 s ; sinon l'étape le note, le TU ne change pas de liste.
- **H9** : 1b.20, commitée, ne touche ni `msgbox.[ch]`, ni `test_pico8_iv.c`, ni `test_pico8_hy.c`, ni `tools/float_audit_c1.txt`.
- **H10** : `make class-coverage-net` rend `iv  474  9/15`, identique à `tools/class_coverage_net_expected.txt`.
- **H11** : `pico8_entity_set_field` sur un `iv` avorte par l'`assert(header->cls->set_field != NULL && …)` (`entity.c:197`), pour tout nom et toute valeur, sur l'ancien comme le nouveau code ; les fils meurent par `SIGABRT`.
- **H12** : `pico8_ob_table_set_index(t, 2, pico8_value_ref(NULL))` et `pico8_ob_table_set_name(t, "ra", pico8_value_ref(NULL))` gardent l'entrée (`pico8_ob_table_set` ne retire rien, même lecture qu'en 1b.18 et 1b.20), donc une clé nil atteint la boucle.
- **H13** : `make float-audit` retrouve le site `pico8_iv_kj | self->at2 | 1`, en C1, atteint par les rejeux, sans ECART ni NEGATIF (couleurs 8, 10, 11… positives entières), et sort 0 ; aucun autre site ne change.
- **H14** : les fils du test d'arrêt, créés par `fork` dans `test_pico8_iv` (TU de `CENSUS_UNIT_TESTS`), ne vident rien au recensement : ils meurent par `SIGABRT`, ou sur l'ancien code sortent par `_exit(0)` (1b.4b).
- **H15** : dans `test_pico8_hy`, chaque `iv` plié par `fold_new_entities` porte un `[1]` chaîne et un `[2]` fixe (seuls producteurs : `hy:lu`, `jo:hk` ; l'`iv` à `[1]` seul d'`iv_descriptor` est construit avant `from`) : re-emballer `at1` en `pico8_value_string` et `at2` en `pico8_value_fixed` rend exactement les `pico8_value_t` d'avant, donc les mêmes empreintes.
- **H16** : `pico8_iv_kj` sur un `iv` à `[1]` seul et sur un `iv` à `[2]` = 0 donne la même empreinte d'écran sur l'ancien code (couleur 0 dans les deux cas).
- **H17** : 50 appels de `pico8_iv_ph` sur un `iv` neuf portent `l` à 5 exactement (50 × `0x1999` + arrondi) et `p_` à `true` au même appel que `dg` le voit aujourd'hui (51 ticks de `live`) ; la caractérisation plie `l` et `p_` à chaque appel, quel qu'en soit le rang.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** Aucun accès direct à `->fa`/`->p_`… dans les tests nouveaux : ils compilent sur les deux formes. Seules exceptions, imposées parce que `at1`/`at2` ne sont pas lisibles par nom : les trois lignes de `test_pico8_hy` qui les lisent déjà directement, réécrites pour relire les membres typés (voir plus bas).
- **(G) Où : `test_pico8_iv`.** Rapide, dans `CENSUS_UNIT_TESTS`, lie `ENGINE_SRCS`, construit déjà des `iv` après `pico8_cart_load()`/`pico8__init()`. On y ajoute `#define _POSIX_C_SOURCE 200809L` en tête, `<signal.h>`, `<sys/types.h>`, `<sys/wait.h>`, `<unistd.h>`. Rien au Makefile. Pas de nouveau TU.
- **Caractérisation, figée sur l'ancien code, par nom** :
  0. **`test_defaults_match_the_parsed_spec` réécrite par nom**, même intention : `field(e, "v")` → REF, valeur égale au `v` de `pico8_iv_default`, REF **différente** de celle de la spec (défaut module) ; `field_fixed(e, "fa")`, `field_fixed(e, "l")` égaux aux défauts parsés ; identité de `v` entre deux `iv` par les REF lues par nom ; les lectures `e->ke.me`/`e->ke.t` (membres de `ke`, déjà typés) restent. Même nom de fonction.
  1. **`test_iv_members_by_name`**, appelée en fin de `main`. Lit par le **lecteur brut** (`((const pico8_struct_header_t *)e)->cls->field`, témoin : fixe `WITNESS` = `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder. `fresh_world()` d'abord.
     - **Lectures sur `iv` neufs** : `a` = `build(&(250,260), "+12$", 11)` ; `b` = `build(NULL, "x", 7)` ; `n` = `pico8_iv_new(NULL)` ; `r` = `iv` à clé `ra` = `REF NULL`, `[1]` = `"r"`, `[2]` = 3 ; `s` = `iv` à `[1]` = `REF NULL`, `[2]` = 5 (H12 ; dessiné plus bas comme `n`). Pour chacun : `fa` → `FIXED 9` ; `l` → `FIXED 0` ; `v` → REF, pointeur **identique** entre les cinq, valeur (0, −0.34 lit) ; `ra` → `a` : REF **même pointeur** que la clé ; `b`, `n`, `r`, `s` : REF d'un vecteur neuf (`je.ra+mo.ra-(0,5)`), pointeurs deux à deux distincts ; `p_` → **absent** ; `me` → `STRING "ph"`, `t` → `FIXED 0` (repli). Intrus `"at1"`, `"at2"`, `"has_p_"`, `"ke"`, `"header"`, `"member"`, `""`, `"P_"`, `"ra "` → 0, témoin intact.
     - **`ph` pas à pas**, sur `b` : 50 appels de `pico8_iv_ph` ; à chaque appel `ra` → REF d'un **nouveau** pointeur (≠ précédent), valeur = précédente + `v` ; `l` → précédent + 0.1 lit ; `p_` → `BOOL` **présent** (0 puis 1 quand `l` ≥ 5) ; `fa`, `v` inchangés. Rang du premier `p_` = 1 plié (H17).
     - **`kj`** (écran vidé, `p` = (64,64)) : `a`, `b` ; `n` (`at1` et `at2` nil → texte `""`, couleur 0) ; `s` (`at1` nil par clé, couleur 5) ; `c` = `iv` à `[1]` = `"c"` seul ; `z` = `iv` à `[1]` = `"c"`, `[2]` = fixe 0 **construit par `memset` puis `kind = FIXED`** (témoin accepté) ; `assert(hash(c) == hash(z))` (H16) ; chaque empreinte pliée ; `l` d'un `iv` après 20 `ph` dessiné aussi (bloc de palette).
     - **Écritures par nom, toutes avortées** (1b.16 : R5 écrivain vacante), par un utilitaire `must_abort_write(e, name, value)` (`fork` + `SIGABRT`, forme de `must_abort_value` de `test_pico8_bg`) : sur `a`, pour chaque nom de `{"fa", "l", "v", "ra", "p_", "at1", "at2"}`, la valeur `pico8_value_bool(1)` ; puis sur `"p_"` et `"ra"` : `REF NULL`, `zero_fixed`, `zero_false`, `zero_string`, `zero_function` (union nulle par `memset(&v, 0, sizeof v)` puis `v.kind = …`). Déjà vrai sur l'ancien code (H11), d'où sa place ici.
     - On plie retours, genres, fixes, booléens, vecteurs lus, identités comparées (0/1), empreintes d'écran et rangs dans **`IV_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les pointeurs sont jugés par `assert`, jamais pliés.

  Lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite.
- **Test d'arrêt, rouge d'abord** (G) : `test_iv_keys_stop_off_row`, appelé après la caractérisation, avec un utilitaire `must_abort_key(key_kind, name_or_index, pico8_value_t v)` : le fils construit une table à la seule clé donnée (plus `[1]` = `"k"` quand la clé testée n'est pas `[1]`), appelle `pico8_iv_new`, puis `_exit(0)` ; il doit mourir par `SIGABRT`. Valeurs à union nulle : `zero_fixed`, `zero_false`, `zero_string`, `zero_function`.
  - **clé `ra`** : `pico8_value_fixed(1)` ; `zero_fixed` ; `zero_false` ; `zero_string` ; `zero_function` ; `pico8_value_string("x")` ; REF d'une table (`pico8_ob_table_new()`) ; REF de `hv` (liste) ; REF d'un `iv`.
  - **clé `[1]`** : `pico8_value_fixed(1)` ; `zero_fixed` ; `zero_false` ; `pico8_value_bool(1)` ; `zero_string` ; `zero_function` ; REF d'un vecteur du tas ; REF d'une table.
  - **clé `[2]`** : `REF NULL` ; `zero_false` ; `pico8_value_bool(1)` ; `pico8_value_string("8")` ; `zero_string` ; `zero_function` ; REF d'un vecteur.

  **Rouge sur l'ancien code** (la boucle recopie tout, `pq` ne déréférence pas une `ra` vraie, le fils sort par `_exit(0)`), vert après : une ligne « Runs » pour chacun des deux passages. Il n'est pas dans la caractérisation figée, qui ne contient que ce que l'ancien code fait déjà. Le fixe 0 à union nulle sur `[2]` est le témoin accepté : il est dans la caractérisation (`kj`, `z`), pas ici.
- **`test_pico8_hy`, façon de lire seulement** (dans le commit de typage, forcément : les membres ne sont pas lisibles par nom) : `fold_value(h, iv->at1, 0)` → `fold_value(h, iv->at1 != NULL ? pico8_value_string(iv->at1) : pico8_value_ref(NULL), 0)` ; `fold_value(h, iv->at2, 0)` → `fold_value(h, pico8_value_fixed(iv->at2), 0)` (H15) ; `:405` `->at1.as.string` → `->at1`. Aucune valeur figée ne change.
- **Tests existants, inchangés, qui figent déjà beaucoup** : `test_no_money_toast_lives_as_before`, `test_price_toast_lives_as_before` (`ra`, `l`, `t`, écran à chaque tick, sortie de `hv` au 51e), `test_mz_scan_skips_a_live_toast`, `test_iv_is_a_struct_registered_under_kj` ; `test_struct_defaults` (`STRUCT_HASH`) ; `test_pico8_ui` (`me`/`t` d'un `iv(NULL)`).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_iv && ./tmp/test_pico8_iv` (et `tmp/test_pico8_hy` pour celles qui le nomment). Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : la branche R4 de `p_` ignore `has_p_` → rouge : caractérisation (lectures sur `iv` neufs, `p_` lu `BOOL 0` présent).
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre** — les fixes lisibles, `fa` et `l`, ne sont jamais absents ; `at2` n'est pas lisible par nom. Forme la plus proche, jouée : R3 pose `has_p_ = 1` → rouge : caractérisation (`p_` présent sur un `iv` neuf).
  3. (§7-3) *init ou boucle qui saute un membre typé*, `malloc` → `calloc` dans la copie : (a) **boucle** : `if (0) iv_take_vec(…)` → rouge : caractérisation (`ra` de `a` ≠ clé), `PRICE_SEQUENCE_HASH` ; (b) `if (0) iv_take_string(…)` → rouge : caractérisation (`kj`), `NO_MONEY_FIRST_DRAW_HASH`, `PRICE_FIRST_DRAW_HASH`, `test_pico8_hy` (`:405`) ; (c) `if (0) iv_take_fixed(…)` → rouge : `PRICE_FIRST_DRAW_HASH` (couleur 11 → 0), caractérisation (`kj` de `a`) ; (d) `fa = 9` omis → rouge : caractérisation, `first_draw != blank` de `live` ; (e) `v = &g_iv_v` omis → rouge : caractérisation (`v` absent), `PRICE_SEQUENCE_HASH` ; (f) `l = 0`, `ra = NULL`, `p_ = 0`, `has_p_ = 0`, `at1 = NULL`, `at2 = 0` omis : **trou justifié**, `calloc` pose déjà l'état nil ou 0, et sous `malloc` l'omission est une lecture non initialisée qu'aucun outil du filet ne signale à coup sûr ; les lignes sont prouvées exécutées par `gcov`.
  4. *nil jugé sur le pointeur seul* dans `iv_take_string` (`value.as.ref == NULL` sans le genre) → rouge : test d'arrêt (`[1]` = `zero_fixed`, `zero_false`, `zero_function`).
  5. *chaîne NULL acceptée* dans `iv_take_string` → rouge : test d'arrêt (`[1]` = `zero_string`).
  6. *nil jugé sur le genre seul* (`kind == REF`) dans `iv_take_vec` → rouge : test d'arrêt (`ra` = REF de table, de liste, d'`iv`).
  7. *tag non vérifié* dans `iv_take_vec` → rouge : test d'arrêt (mêmes cas).
  8. *nil accepté* dans `iv_take_fixed` (`REF NULL` → 0) → rouge : test d'arrêt (`[2]` = `REF NULL`).
  9. *R6 muet sur `has_p_`* : `ph` ne pose plus `has_p_ = 1` → rouge : caractérisation (`ph` pas à pas), `live` (`ticks < 1000`), les quatre empreintes de toast.
  10. *R6 muet sur `ra` dans `pq`* : `if (0)` autour de `self->ra = heap_ra` → rouge : caractérisation (`ra` absent sur `b`, `n`, `r`, `s`), `NO_MONEY_SEQUENCE_HASH`.
  11. *`pq` teste la mauvaise condition* (`self->ra != NULL`) → rouge : caractérisation (`ra` de `a` remplacée, `b` sans `ra`), les quatre empreintes.
  - **Mutants équivalents, non retenus, notés dans le commit** : `pico8_iv_vec_or_zero` qui déréférence sans tester NULL, et les branches R4 de `v`/`ra` qui ne testent pas NULL (R3 et `pq` ne laissent jamais `v` ni `ra` NULL après construction) ; `kj` qui lit `self->at1` sans repli `""` n'est **pas** équivalent (`n`, `iv(NULL)` de `test_pico8_ui`/`spec_defaults` ; plantage ou dessin différent selon `pico8_io`) : noté si joué.
  - **Trou justifié** : une REF de genre ou de boîte en clé `ra` passe `iv_take_vec` (même tag `UNKNOWN`, H3) ; l'ancien code avait la même limite (et pire : toute REF non NULL).
- **`gcov`** : `test_pico8_iv` en `--coverage -O0` ; ses seuls fils sont ceux de `must_abort_write`/`must_abort_key`, qui avortent sans vider leur compteur (pas de piège `__gcov_dump` à poser). Chaque ligne modifiée est exécutée : les cinq branches R4 (présent, et `p_` absent), les trois aides en recopie, `iv_take_vec`/`iv_take_string` en sortie nil (`r`, `s`), R3, `pq` (avec et sans `ra`), `ph`, `kj` (`at1` NULL et non NULL). **Trous attendus, justifiés** : la branche d'échec des trois `assert` d'aide (exécutée seulement dans les fils, prouvée par le test d'arrêt et les mutations 4 à 8) ; les `return 0` de `v`/`ra` NULL dans le lecteur et le NULL de `pico8_iv_vec_or_zero` (inatteignables, mutants équivalents ci-dessus). Le rapport va dans le commit.
- **Prior art** : `test_bg_members_by_name`/`test_bg_writer_stops_off_row` (1b.15, `must_abort_value`, `zero_*`), `test_pico8_qb` (1b.19, `qb_take_fixed`, `zero_ref`), `test_pico8_fq` (1b.18, `zero_function`), `test_ie_members_by_name` (1b.20, témoin, lecteur brut), `test_pico8_mh` (1b.1, même fichier source).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_iv` : quelques secondes (H8) ; environ 20 lancements (caractérisation et test d'arrêt sur l'ancien code, nouveau code, 11 mutations dont la 3 en plusieurs formes, `gcov`), 2 à 4 min. Si un lancement dépasse 20 s, l'implémenteur joue d'abord les mutations §7 (1 à 3) et 5, 8, 9, et note les autres comme non lancées ;
- `./tmp/test_pico8_hy` : une fois après le typage, et pour la mutation 3b ; il rejoue deux fichiers d'événements (ordre de 10 à 30 s, à noter) ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min (frames-check, `float-audit`, les huit `UNIT_TESTS_SLOW`) ;
- `make class-coverage-net` : 3 à 4 min, dans la même invocation que `make e2e` (arbres distincts, comme en 1b.14a, 1b.15, 1b.18, 1b.20) ;
- `make sanitize-net` : ~4 min seul ; **jamais** lancé en même temps que `e2e` ;
- `make value-members` : quelques secondes.

**TU de `UNIT_TESTS_SLOW` touchés : aucun par diff** ; les huit (`test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) sont reliés contre le nouveau `msgbox.c` et lancés une fois, par `make e2e`. Aucun n'est lancé seul.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` sortie 0 (site `pico8_iv_kj | self->at2 | 1` en C1, atteint), et les huit `UNIT_TESTS_SLOW` qui passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 170`**, sans aucune ligne `pico8_iv_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**, parce que le diff touche le corps du constructeur `pico8_iv_struct_new` (R3, R5), sans toucher `pico8_class_coverage_record("iv")` ni `KNOWN[]`. Sortie identique à `tools/class_coverage_net_expected.txt` (`iv  474  9/15`, H10).

`make member-census` ne s'applique pas (R8). On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements. On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si une aide avorte dans un rejeu ou dans un TU hors des fils de `must_abort_*` (H2), si un indicateur autre que `has_p_` est ajouté, si un écrivain par nom est ajouté à `iv`, si la boucle accepte une autre clé que `ra`, `[1]`, `[2]`, ou si un type est élargi (union, `void *`, `pico8_value_t`) au lieu d'avorter.
- `FAILED:` si les types ne sont pas exactement, dans cet ordre après `pico8_ke_t ke` : `pico8_fixed_t fa`, `pico8_fixed_t l`, `pico8_vec_t *v`, `pico8_vec_t *ra`, `int p_`, `int has_p_`, `const char *at1`, `pico8_fixed_t at2`.
- `FAILED:` si `v` d'un `iv` neuf n'est pas `&g_iv_v`, partagé, ou si une clé `ra` n'est pas gardée même pointeur.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur `p_` nil ou un nom inconnu, rend `p_` « présent `false` » quand il est nil, rend `at1`/`at2` lisibles par nom, ou si `me`/`t` changent de chemin.
- `FAILED:` si `iv_take_vec`/`iv_take_string` ne posent pas `*out = NULL` **avant** le test de nil, si ce test ne vérifie pas le genre **et** le pointeur, si `iv_take_string` accepte une chaîne NULL, si `iv_take_vec` ne vérifie pas le tag `UNKNOWN`, si `iv_take_fixed` accepte nil, ou si un message ne nomme pas `iv.<membre>` et le genre de sa ligne.
- `FAILED:` si l'`assert` des clés inconnues (« iv: every measured key is ra, [1] or [2]… ») ou son message change, ou si un message d'`assert` de `pico8_entity_set_field` change.
- `FAILED:` si `pq`, `ph` ou `kj` changent autrement que par R6 (ordre des instructions, `malloc` des vecteurs et TROU-7, constante 0.1, seuil 5, `pico8_dv_default` gardés), ou si la signature d'une fonction publique d'`iv` change.
- `FAILED:` si la partie `mh` de `msgbox.[ch]`, `pz`, `pico8_msgbox_truthy` ou `pico8_msgbox_field_vec` changent, ou si un site par nom change (`dc`, `dg`), ou si `entity.c`, `bootstrap.c`, `economy.c`, `planet.c` changent.
- `FAILED:` si `tools/float_audit_c1.txt` change autrement que par la seule ligne `pico8_iv_kj` (`color_val.as.fixed` → `self->at2`), ou si `make float-audit` ne sort pas 0.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff de `test_pico8_hy.c` fait autre chose que les trois lectures décrites ; si le diff de `test_pico8_iv.c` fait autre chose que réécrire `test_defaults_match_the_parsed_spec` par nom et ajouter la caractérisation, le test d'arrêt, leurs utilitaires, `IV_MEMBERS_HASH`, les `#include`/`#define` nommés et leurs appels en fin de `main` ; si une valeur figée existante change (H5) ; ou si un test de régression (§6) change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 170, liste encore `pico8_iv_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` s'il manque la ligne « Runs » de `make e2e` montrant les huit `UNIT_TESTS_SLOW` verts.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation (réécriture de `test_defaults_match_the_parsed_spec` comprise) n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit ou écrit un membre d'`iv` directement, ou si `IV_MEMBERS_HASH` change après le typage ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'ancien code et une verte après, ou s'il ne porte pas les cas à union nulle construits par `memset` (`zero_fixed`, `zero_false`, `zero_string`, `zero_function` sur `ra` et `[1]` ; `zero_false`, `zero_string`, `zero_function` sur `[2]`).
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 11 mutations, chacune avec son test ou la justification de son trou, plus les mutants équivalents et le trou `ref:autre` de `ra`.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.21 », ou le tableau « Runs ».
- `FAILED:` si le budget d'environ 20 minutes est dépassé sans que le journal note ce qui n'a pas été lancé.

## Out of Scope

- La partie `mh` de `msgbox.[ch]` (1b.1), `pz`, `kc`, et leurs aides en ligne.
- Les homonymes `fa`, `l`, `v`, `ra`, `p_` de `jo`, `qb`, `nf`, `bg`, `js`, `g_`… (étapes suivantes) ; `jo.ra`, qui produit la clé `ra`, reste `pico8_value_t` jusqu'à 1b.23b.
- Rendre `at1`/`at2` lisibles par nom ; ajouter un écrivain par nom à `iv`.
- Libérer le texte `at1` ou les vecteurs de `ra` (TROU-7).
- Distinguer un vecteur d'une autre REF de tag `UNKNOWN`.
- Remplacer une lecture par nom (`dc`, `dg`) par un accès direct.
- Le commentaire de module de `msgbox.h`.
- Régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `msgbox.[ch]` (partie `iv`) seulement, plus une ligne de `tools/float_audit_c1.txt` ; un commit, sans découpage ;
  - tests : caractérisation et test d'arrêt dans `test_pico8_iv` (rapide, au recensement) ; `test_defaults_match_the_parsed_spec` réécrite par nom avant le typage ; `test_pico8_hy` touché dans sa façon de lire `at1`/`at2` seulement, dans le commit de typage ; pas de nouveau TU, pas de changement du Makefile ;
  - `has_p_` seul (lettre de R2) ; **`at2` fixe sans indicateur**, justifié par la non-lecture par nom et l'égalité d'écran nil/0, clé `[2]` nil qui avorte (1b.10, 1b.19) ;
  - `at1` → `const char *` (1b.14a), chaîne NULL refusée en clé ;
  - **R5 écrivain vacante, décidée explicitement** : `iv` n'a pas de ligne `e`, `set_field` reste `NULL` ; R5 ne s'exerce que dans la boucle de clés (1b.10, trois aides) ; les cas `memset` sont figés comme écritures par nom avortées (1b.16) et comme clés avortées (test d'arrêt) ;
  - R7 forme sélecteur local de 1b.8 ;
  - extrait C1 de `kj` réécrit dans `tools/float_audit_c1.txt` (précédent 1a.26) plutôt qu'un emballage mort ;
  - mutation §7-2 sans objet dans sa lettre, jouée sous sa forme la plus proche ; §7-3 rendue déterministe par `calloc` ; `class-coverage-net` lancé.
- **Note « Découvert à 1b.21 »** proposée, sur la ligne 1b.21 de `docs/plan-3.md` : « patron 1b.1 appliqué ; R7 forme sélecteur local (1b.8) ; `has_p_` seul ; **R2 : un fixe à nil `tenu` jamais lu par nom (`at2`) n'a pas d'indicateur**, sa seule lecture directe rendant nil et 0 à l'identique (preuve d'écran de la caractérisation), et sa clé nil avorte (1b.10, 1b.19) ; `at1` → `const char *`, chaîne NULL refusée en clé ; **R5 écrivain vacante** (pas de ligne `e`) ; boucle de clés 1b.10 sur deux clés positionnelles (`iv_take_string`, `iv_take_fixed`) et `ra` (`iv_take_vec`) ; **un site C1 de `float-audit` dont l'extrait change avec le typage est réécrit dans `tools/float_audit_c1.txt`, seule ligne touchée (précédent 1a.26)** ; revue des TU lents sans effet ; empreinte réelle 7 (4, 3), 5 fichiers modifiés. » Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 177 → 170, le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `IV_MEMBERS_HASH` et les réponses à H1 à H17.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
