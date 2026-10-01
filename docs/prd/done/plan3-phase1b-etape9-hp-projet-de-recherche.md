---
title: "Plan 3, étape 1b.9 : `hp` (le projet de recherche) — dix-sept membres typés, cinq fixes avec `has_`, six chaînes empruntées, `il` ({bool, chaine}) en `const char *` avec `has_il`, et la table `offsetof` `g_hp_members[]` supprimée"
description: "`hp` a dix-sept membres `pico8_value_t`, tous écrits par le seul constructeur à partir de `fd` et lus par nom. Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 240 à 223. `il` reprend la lecture {bool, chaine} de 1b.7, parce que le cart ne peut pas y écrire `true`. `hp` est la première struct de la liste à table d'offsets `members[]` (forme `offsetof` de R7) et à boucle de clés qui passe par une table intermédiaire (`ri`, après `ho`) : ce PRD fixe ces deux formes. Pas d'écrivain par nom : la moitié écrivain de R5 reste vacante."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.9 — `hp` (`research.h:15`), genres mêlés `il`, cible de `jo.hn`/`js._jo_fe_b`. Empreinte 20 (10, 10). Compte : 223.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié), le §6 (tests de régression) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 ». Il est cité ici, pas redécidé. S'appliquent aussi :

- « Découvert à 1b.3 » et « à 1b.4 » : patron appliqué sans écart, R5 et R7 vacantes tant qu'aucune étape n'en a ;
- « à 1b.4b » : la table corrigée (fils forkés) fait foi. Elle exige `has_<m>` pour `hp.fj`, `o`, `dm`, `ii`, `rn`, et note `il` en `bool,chaine` ;
- « à 1b.5 » : **forme R7 d'un accesseur `pico8_X_member`** et sa preuve mécanique `grep -n 'pico8_value_t \*' … | grep -v 'pico8_value_t \*out_value)'` ; **forme R5 de la boucle de clés** (branche par clé, `assert` « champ sans règle » hors genre) ; lecture de R1 pour {fixe, bool}, dont l'**emballage public unique par membre** `pico8_X_<m>` ;
- « à 1b.6 » : un fixe suit son propre nil, pas celui d'un homonyme ;
- « à 1b.7 » : **lecture de R1 pour {bool, chaine}** quand le seul booléen écrit est `false` (`const char *<m>` + `int has_<m>`, NULL présent = `false`, emballage `pico8_X_<m>`). Sa lecture de R2 (nil `stat?` venu d'un homonyme) **ne sert pas ici** : chaque non-pointeur de `hp` à indicateur a une preuve `lu` directe.

### État de départ

HEAD b814117. **Précondition** : 1b.8 (`fm_placeholder`) est commitée avant que 1b.9 commence. Elle est en cours d'implémentation. Elle touche `probe.[ch]`, `cheat.c` et `test_pico8_fm.c`, et aucun fichier de cette étape : les numéros de ligne cités ici (ceux de HEAD) ne devraient pas glisser. Chaque site est désigné aussi par son contenu. `make value-members` rend alors 240.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H15).

### Lignes de recensement (`tools/member_census.txt:157-173`, régénéré par 1b.4b)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `mr` | offset | `ref:table` | jamais | - | l |
| `gp` | offset | `ref:table` | jamais | - | l |
| `fk` | offset | `ref:table` | jamais | - | l |
| `px` | offset | `chaine` | observé | lu | l |
| `lr` | offset | `chaine` | observé | lu | l |
| `fj` | offset | `fixe` | observé | lu | l |
| `ja` | offset | `chaine` | observé | lu | l |
| `i` | offset | `ref:liste` | observé | stat | l |
| `o` | offset | `fixe` | observé | lu | l |
| `il` | offset | **`bool,chaine`** | observé | lu,stat | l |
| `dm` | offset | `fixe` | observé | lu,stat | l |
| `rr` | offset | `chaine` | observé | lu,stat? | l |
| `ii` | offset | `fixe` | observé | lu,stat | l |
| `cw` | offset | `chaine` | observé | lu,stat | l |
| `go` | offset | `chaine` | observé | lu,stat | l |
| `rn` | offset | `fixe` | observé | lu,stat | l |
| `lq` | offset | `ref:table` | observé | stat? | l |

Dix-sept membres, donc 240 − 17 = **223**, comme le dit la ligne. Aucun n'a la colonne `e` : `hp` n'est écrit par nom nulle part (voir R5). Les deux lignes qui pointent vers `hp` restent `pico8_value_t` : `pico8_jo_t hn` (`:74`, 1b.23) et `pico8_js_t _jo_fe_b` (`:251`, 1b.34).

**Ce recensement est exhaustif pour `hp`.** Chaque `hp` est construit par `pico8_research_build_gb` à partir de **tout** `fd`, à chaque chargement, donc à chaque rejeu et dans chaque TU qui charge le cart. Après le constructeur, aucun code n'écrit un membre de `hp` : pas de `set_field`, pas de méthode, et aucune écriture directe hors de `pico8_hp_struct_new` (grep, H3). L'observation de fin de constructeur (`PICO8_MEMBER_CENSUS_SEE`) voit donc chaque valeur que `hp` tiendra jamais.

### Ce que fait le cart (`pico8/lua.lua`)

- `hp=kv:jr(…)` (`:1399-1402`) : `mr=o("px","lr","fj","ja","i","o","il","dm")`, `gp=o("o","dm")`, `fk=o(f,2)`.
- `hp:pq()` (`:1403-1408`) : `ho(gb,hp.mr,hp.gp)(self)`, puis `self.lq={self.px,32,{self.ii,8}}` si `self.ii`, puis `self.i=fi(self.i or "",…)`.
- `ho` (`:17-23`) : `ri[p]=gu(gp,p) and r[ri[i]] or ri[i]`. `o` et `dm` passent par `r`, les autres noms de `mr` sont copiés tels quels.
- `gb` (`:1438-1444`) : `ri.rr=rr`, puis `add(gb,hp(ri))`, pour chaque enregistrement de `fd=ob(unstash(8494))` (`:1391`).
- **`il` ne peut pas valoir `true`.** `il` est `ri[7]`, copié sans passer par `r`. Sa seule source est un enregistrement de `fd`, lu par `ob` (`:55-70`). `ob` ne produit que quatre formes : une chaîne (`sc=='"'`), un appel `x(…)`, **`false`** (`sc=="f"`), ou un nombre. Il n'a aucun littéral `true`. Le port fait de même : `pico8_ob_value_for_token` (`ob.c:238-300`) ne rend que `pico8_value_bool(0)`. Les lecteurs du cart sont `hp:po` (`self.il and oj(self.il)`, `:1423`) et `hp:od` (`:1427`). Ce sont deux tests de vérité, où `false` et nil se valent.

### Le code aujourd'hui

- `src/pico8_research.h:9-32` : le commentaire du typedef, puis `{ header; pico8_value_t mr, gp, fk, px, lr, fj, ja, i, o, il, dm, rr, ii, cw, go, rn, lq; }`, un membre par ligne. Le commentaire de module (`:34-36`) dit « a nil or wrong-kind member reads as its zero/false default rather than crashing ».
- `src/pico8_research.c` :
  - statiques de classe `g_mr`/`g_gp`/`g_fk` (`:22-24`, `pico8_value_t`, **pas des membres**, hors périmètre) ; listes de noms `g_hp_mr_names` (`:26`) et `g_hp_named_keys` (`:27`) ;
  - **table d'offsets `g_hp_members[]`** (`:29-39`) : 17 entrées `{"m", offsetof(pico8_hp_t, m)}` ;
  - accesseur `hp_member` (`:96-104`) : une boucle sur `g_hp_members[]` qui rend `pico8_value_t *` (forme `offsetof`, pas `strcmp`) ;
  - lecteur `pico8_hp_field` (`:106-125`) : il passe par `hp_member`. Un membre nil rend 0, **sauf `lq`**, qui rend alors `pico8_value_function(pico8_hp_lq_dispatch)` : c'est la méthode `lq` de la classe que le membre masque (1a.33) ;
  - descripteur (`:127-131`) : aucune méthode, `set_field` NULL ;
  - constructeur `pico8_hp_struct_new` (`:170-246`) :
    - bloc d'ouverture : les 17 membres à `REF NULL` par `hp_member` (`:183-185`), puis `mr`/`gp`/`fk` = `g_mr`/`g_gp`/`g_fk` (`:186-188`) ;
    - boucle de clés (`:190-200`) : `assert(hp_accepts_key(key) && …)`, puis recopie dans la table locale `ri` ;
    - `pico8_ho_apply(scratch_rc, …, ri)` (`:202-205`) ;
    - **recopie de `ri` dans les membres** par `hp_member` (`:206-211`, les 8 noms de `mr` puis les 5 clés nommées), puis `pico8_ob_table_free(ri)` ;
    - `gb[px]` (`:214-217`) ; `lq` construit si `ii` est vrai (`:219-231`) ; `i` réécrit de la chaîne en liste (`:233-239`) ;
    - `pico8_class_coverage_record("hp")` et `PICO8_MEMBER_CENSUS_SEE` (`:241-244`) ;
  - `pico8_hp_po` (`:372-396`) lit `ja` et `il` ; `pico8_hp_od` (`:398-414`) lit `fj`, `il` et `ii`.
- `src/pico8_research_display.c` : `pico8_hp_jb_join` (`i`, `:62-65`, par `PICO8_REF_AS_LIST_CONST`) ; `pico8_hp_jb` (`ii` `:110`, `o` `:120-121`) ; `pico8_hp_lq` (`i` `:276-277`, `o` `:286`, `px` `:294-295`) ; `pico8_bj_predicate` (`rr` `:322`, `lr` `:333-339`).
- `src/pico8_planet.c` : `pico8_jo_lz` (`:424-450`) pose `self->hn = pico8_value_ref(hp_instance)` et lit `i` (`:438-439`) et `o` (`:446-447`) ; `pico8_jo_hu` (`:507-575`) lit `cw` (`:517-518`), `go` (`:541-551`, branche `FIXED` comprise) et `rn` (`:569-570`).
- `src/pico8_economy_tiers.c:207-214` : `bb` lit `hn->dm` (test de nil, test de vérité, puis `qe`).
- Lecteurs par nom (**inchangés**) :
  - `pico8_mv(b, "lq", …)` (`planet_display.c:414`, `jo:fe`) ;
  - `pico8_entity_get_field(inst, "_jo_fe_b", …)` (`planet_display.c:384`), qui lit un `js`, pas un `hp` ;
  - les tests (`test_pico8_hp`, par `lookup`/`has`/`get_ref`).
- **Cibles REF.** `jo.hn` (`planet.c:434`) et `js._jo_fe_b` (`planet_display.c:438`, recopié par `js(b)`) tiennent `pico8_value_ref(<hp>)`. `pico8_research_free_gb_instances` (`:75-87`) libère chaque `hp` par `free`, sans libérer ses membres (TROU-7).

### Nil, membre par membre

- `mr`, `gp`, `fk` : nil `jamais`. Ce sont des pointeurs : la REF de la classe, la même pour tous les `hp`.
- `px`, `lr`, `ja`, `rr`, `cw`, `go` : `chaine`, nil observé. Ce sont des pointeurs, NULL = nil. `rr` vaut nil quand `fd` n'a pas encore de catégorie (`build_gb`, `ri.rr = REF NULL`).
- `i` : `ref:liste`. C'est un pointeur. Le constructeur l'alloue toujours, il n'est donc jamais NULL après lui. La chaîne brute d'avant `fi` ne passe plus par le membre (voir R3).
- `lq` : `ref:table`. C'est un pointeur. NULL est lu comme la méthode (voir R4).
- `fj`, `o`, `dm`, `ii`, `rn` : `fixe`, nil observé par preuve **`lu` directe** (projets sans cette clé, et le `bare` de `test_hp_is_a_struct`). Ce sont des non-pointeurs lus par nom : **`has_fj`, `has_o`, `has_dm`, `has_ii`, `has_rn`** (R2, note 1b.4b).
- `il` : `bool,chaine`, nil observé (`lu`). Voir la décision R1 ci-dessous.

### Empreinte remesurée

Le grep de la liste rend **20 (10, 10)**, la même valeur que la ligne :

- src : `economy_tiers.c`, `menu.c`, `planet.c`, `planet.h`, `planet_display.c`, `research.c`, `research.h`, `research_display.c`, `research_display.h`, `score_panel_display.c` ;
- tests : `built_planet_panel_nil_row`, `entity_kind`, `ga`, `ga_export_panel_by`, `hp`, `js`, `nl`, `os`, `planet_gh_row_lq`, `rich_planet_exports`.

Ce grep **sur-compte** :

- `menu.c`, `score_panel_display.c` et six des dix tests n'y entrent que par des homonymes lus par nom (`"lq"`, `"px"`, `"o"`, `"i"`) sur d'autres structs ;
- `planet.h`, `research_display.h` et `planet_display.c` nomment `pico8_hp_t` sans lire un membre ;
- `rich_planet_exports` ne fait qu'un cast.

**Fichiers modifiés : 7**, plus `docs/plan-3.md`.

- src (5) : `src/pico8_research.h`, `src/pico8_research.c`, `src/pico8_research_display.c`, `src/pico8_planet.c`, `src/pico8_economy_tiers.c`.
- tests (2) :
  - `tests/test_pico8_hp.c` : la caractérisation, un pur ajout ;
  - `tests/test_pico8_entity_kind.c` : la seule lecture directe d'un membre de `hp` hors de `src/` (`test_every_cw_is_pm_or_ga`, `cw = hp->cw`, `:209-214`), qui change **de forme seulement**. Ce n'est pas un test de régression du §6.

Aucun test de `UNIT_TESTS_SLOW` n'est touché (`ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`). Ils lisent `gb`/`hn` par nom ou par REF, et leur diff est vide. Aucun test de régression (§6) n'est touché : `rich_planet_exports` garde son cast `(pico8_hp_t *)`, et son diff est vide.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** (un membre par ligne, ordre et noms gardés) :

   ```c
   pico8_ob_table_t *mr;
   pico8_ob_table_t *gp;
   pico8_ob_table_t *fk;
   const char *px;
   const char *lr;
   pico8_fixed_t fj;  int has_fj;
   const char *ja;
   pico8_list_t *i;
   pico8_fixed_t o;   int has_o;
   const char *il;    int has_il;
   pico8_fixed_t dm;  int has_dm;
   const char *rr;
   pico8_fixed_t ii;  int has_ii;
   const char *cw;
   const char *go;
   pico8_fixed_t rn;  int has_rn;
   pico8_ob_table_t *lq;
   ```

   La table `g_hp_members[]` et l'accesseur `hp_member` disparaissent. Un emballage public, `pico8_hp_il`, est ajouté.
3. Effet mesurable : `make value-members` passe de 240 à **223** et ne liste plus aucune ligne `pico8_hp_t`. Aucun pixel ne bouge. `jo.hn` et `js._jo_fe_b` tiennent le même pointeur qu'avant.

**Pas de découpage en 1b.9a/b** (G). Côté `src/`, le diff se concentre dans un fichier réécrit (lecteur, constructeur, `po`, `od`) et dans une quinzaine de lignes de lecture directe, que le compilateur désigne. Côté tests, il y a une ligne de forme et une caractérisation. La table `g_hp_members[]` et `hp_member` ne disparaissent qu'une fois toutes leurs entrées retirées. Deux commits laisseraient une table à moitié vide, avec les deux formes de recopie dans le même constructeur. **Découpage de repli**, à ne prendre que si le premier passage dépasse le budget avant les fixes :

- 1b.9a : les onze pointeurs (`mr`, `gp`, `fk`, `px`, `lr`, `ja`, `i`, `rr`, `cw`, `go`, `lq`), compte **229**. `g_hp_members[]` est réduite aux six entrées `fj`, `o`, `il`, `dm`, `ii`, `rn`.
- 1b.9b : `fj`, `o`, `dm`, `ii`, `rn` et `il`, compte **223**. `g_hp_members[]` et `hp_member` sont supprimés.

L'implémenteur écrit alors ce découpage dans la liste du plan, dans le commit de 1b.9a.

## User Stories

1. En tant que lecteur de `research.c`, je veux les dix-sept membres de `hp` typés, afin que le constructeur, `po` et `od` ne déballent plus de `pico8_value_t`.
2. En tant que lecteur de `research_display.c`, `planet.c` et `economy_tiers.c`, je veux lire `i`, `o`, `px`, `rr`, `lr`, `cw`, `go`, `rn`, `ii`, `dm` par leur type C, sans `PICO8_REF_AS_LIST_CONST` sur `i`.
3. En tant que `test_pico8_hp`, je veux lire par nom chaque membre de chacun des 34 projets de `gb` avec le même genre, la même valeur et **le même pointeur** qu'en 1a.
4. En tant que `hp:po` et `hp:od`, je veux `il` lu `BOOL` 0 (présent) sur un projet dont l'enregistrement porte `f`, et non absent, comme en 1a.
5. En tant que `jo:fe`, je veux `mv(b, "lq")` qui rende le membre `lq` d'un projet avec `ii`, et la méthode `lq` des autres.
6. En tant que `jo.hn` et `js._jo_fe_b`, je veux que le projet que je pointe reste à la même adresse, entre dans `gb` sous le même rang et passe `pico8_hp_assert`.
7. En tant que `bare` (`pico8_hp_new(NULL)`), je veux exactement `mr`, `gp`, `fk`, un `i` vide et la méthode `lq`, sans aucun autre membre.
8. En tant que `test_hp_is_a_struct`, je veux que l'écriture par nom, une clé nommée inconnue et une clé positionnelle 9 avortent toujours, avec les mêmes messages.
9. En tant que `test_every_cw_is_pm_or_ga`, je veux lire `cw` en `const char *` et compter les mêmes projets.
10. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
11. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux, sans nouvelle allocation ni libération (TROU-7 préservé pour `i` et `lq`, chaînes toujours empruntées à `fd`).
12. En tant que `plan-pilot`, je veux `make value-members` à 223 et `make class-coverage-net` identique à la référence (`hp  510  15/15`).
13. En tant que `make float-audit`, je veux que les sites C1 `economy_tiers.c | bb_visit | mi` et `research_display.c | pico8_jo_ex | loop_bound` gardent leur extrait mot pour mot.
14. En tant qu'auteur de 1b.23 (`jo`, et sa table `offsetof` enchâssée) et de 1b.32 (`g_`), je veux savoir comment R7 se lit sur une table `members[]` `offsetof`, et ce qui reste à fixer pour une table qui garde des entrées.
15. En tant qu'auteur de 1b.13 (`jc_too_far.fo`), je veux savoir que 1b.9 **ne** fixe **pas** la moitié écrivain de R5, et en quoi le déballage de la recopie `ri` peut lui servir.
16. En tant qu'auteur des étapes suivantes à {bool, pointeur} (`js.h_`/`my` 1b.34, `qh.et` 1b.24, `g_.et` 1b.32, `mz.et` 1b.36), je veux savoir que `hp.il` reprend la lecture de 1b.7 sans l'élargir.
17. En tant que vérificateur, je veux une caractérisation par nom lancée sur l'ancien code, les mutations du §7 déclinées sur `hp`, la justification de chaque trou, et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline`, et marqués (G).

### Décision R1 pour `il` : lecture de 1b.7, appliquée sans écart

La condition de 1b.7 est que le seul booléen écrit soit `false`. Elle tient, par deux preuves :

- **par le cart** : `il` est une copie brute de `ri[7]`, et `ob` n'a pas de littéral `true` (voir « Ce que fait le cart ») ;
- **par le C** : le seul écrivain de `il` est la recopie de `ri` dans le constructeur. `ri` ne tient que ce que `fd` (par `pico8_ob`) ou un appelant de `pico8_hp_new` y met. Les appelants sont `build_gb` (enregistrements de `fd`), `test_hp_is_a_struct` (`NULL`) et `test_pico8_spec_defaults` (`NULL`). Les deux `must_abort` qui passent `BOOL 1` (`build_with_named_key`, `build_with_positional_9`) avortent sur `hp_accepts_key` **avant** toute recopie. Aucun `set_field`, aucune méthode, aucune écriture directe (H3).

**(G) Forme retenue :**

```c
const char *il;
int has_il;   /* 0: `il` is nil; 1 with `il` NULL: `il` is false */
```

| état 1a | `has_il` | `il` | lu par nom |
|---|---|---|---|
| nil (`REF NULL`, clé absente) | 0 | NULL (non lu) | absent, `*out_value` intact |
| `BOOL` 0 | 1 | NULL | `BOOL` 0 |
| `STRING` s | 1 | s, même pointeur (emprunté à `fd`) | `STRING` s |
| `BOOL` 1 | — | — | **impossible** : `assert` « champ sans règle » dans le déballage (R5) |

- **Emballage** (lecture de 1b.5 et 1b.7) : `pico8_value_t pico8_hp_il(const pico8_hp_t *self)`, déclaré dans `research.h`. Il rend `REF NULL` si `self` est NULL ou si `has_il` vaut 0, `pico8_value_bool(0)` si `il` est NULL, `pico8_value_string(il)` sinon. **Ses appelants** sont le lecteur, `pico8_hp_po` et `pico8_hp_od`. Ce sont des lectures directes qui gardent la valeur de 1a, et c'est l'usage prévu par 1b.5 : `pico8_research_truthy(v) && pico8_oj(v)` reste mot pour mot, avec `v = pico8_hp_il(self)`. Son contrat tient en une phrase dans `research.h`.
- Pas de `il_is_bool` : il vaudrait `il == NULL` (1b.7).
- La chaîne n'est jamais possédée ni libérée : elle est empruntée à `fd`, comme `px`.

### Les autres membres à pointeur : même pointeur, jamais possédé

- `mr`, `gp`, `fk` → `pico8_ob_table_t *` (`ref:table`). Le constructeur pose `self->mr = g_mr.as.ref` (idem `gp`, `fk`) : **la même REF** pour tous les projets, celle que rend `pico8_hp_fk()`. Les statiques `g_mr`/`g_gp`/`g_fk` restent `pico8_value_t` (état de classe, pas des membres : hors compte et hors périmètre).
- `px`, `lr`, `ja`, `rr`, `cw`, `go` → `const char *` (`chaine`), le pointeur du `STRING` de `ri`. Ce pointeur est celui de l'enregistrement de `fd`, car `pico8_ob_table_set_*` ne duplique que les noms de clé, pas les valeurs (H13). Jamais libéré.
- `i` → `pico8_list_t *` (`ref:liste`), la liste allouée par le constructeur (TROU-7, jamais libérée).
- `lq` → `pico8_ob_table_t *` (`ref:table`), la table construite si `ii` est présent (TROU-7).

### Cibles REF : `jo.hn` et `js._jo_fe_b` inchangés

Le typage ne touche ni l'allocation ni l'adresse d'un `hp` : un `malloc(sizeof *self)` par construction, rangé dans `gb` (positionnel et `gb[px]`), libéré par `free` dans `pico8_research_free_gb_instances`. `sizeof(pico8_hp_t)` change, mais aucun code ne copie un `hp` par valeur ni ne dépend de sa taille (grep). `jo.hn` (`pico8_jo_lz`, `self->hn = pico8_value_ref(hp_instance)`), `_jo_fe_b` (`pico8_jo_fe_step`) et leurs lecteurs (`field_ref_instance(pt, "hn")`, `pico8_entity_get_field(inst, "_jo_fe_b", …)`) ne changent pas. Ils restent `pico8_value_t` jusqu'à 1b.23 et 1b.34. `pico8_hp_assert` et son message ne changent pas. La caractérisation (cas 4) le vérifie.

### R1–R9 appliquées à `hp` (patron de 1b.1, cité)

| règle | sur `hp` |
|---|---|
| R1 | Voir la Solution. `fj`, `o`, `dm`, `ii`, `rn` → `pico8_fixed_t`. `px`, `lr`, `ja`, `rr`, `cw`, `go` → `const char *`. `i` → `pico8_list_t *`. `mr`, `gp`, `fk`, `lq` → `pico8_ob_table_t *`. `il` : ci-dessus. Noms et ordre gardés, un membre par ligne. |
| R2 | `has_fj`, `has_o`, `has_dm`, `has_ii`, `has_rn`, `has_il`, chacun juste après son membre. Pas d'indicateur pour les pointeurs (NULL = nil), sauf `il` (lecture 1b.7). Chaque preuve de nil d'un non-pointeur est `lu` directe : la lecture de R2 de 1b.7 (homonyme `stat?`) n'est pas invoquée. |
| R3 | Le bloc d'ouverture (`:183-188`) garde sa place. La boucle `hp_member` devient des affectations typées : pointeurs à NULL, fixes à 0 et les six `has_` à 0, posés tous deux. Puis `mr`/`gp`/`fk` = `g_*.as.ref`. **`i` brut** : la chaîne que `ho` laisse dans `ri["i"]` est lue dans une **locale** `pico8_value_t` avant `pico8_ob_table_free(ri)`. `i_src` en est tiré par la même condition qu'aujourd'hui (`STRING` non NULL, sinon `""`), puis `self->i = new_i_heap`. Le membre ne tient jamais la chaîne. Le reste garde son ordre : `gb[px]` (`self->px != NULL`, `set_name(g_gb, self->px, pico8_value_ref(self))`), puis `lq` si `self->has_ii`, avec `inner[1] = pico8_value_fixed(self->ii)` et `outer[1] = pico8_value_string(self->px)` si `px != NULL`, et `self->lq = outer`. Ensuite `i`, `pico8_class_coverage_record`, `PICO8_MEMBER_CENSUS_SEE`. `/* TROU-7 */` est gardé. |
| R4 | Le lecteur a une branche `strcmp` par membre, dans l'ordre du typedef, avant le `return 0` final. `mr`/`gp`/`fk`/`i` : NULL → `return 0`, sinon `pico8_value_ref(ptr)`. Chaînes : NULL → `return 0`, sinon `pico8_value_string(ptr)`, même pointeur. Fixes : `has_<m>` = 0 → `return 0`, sinon `pico8_value_fixed`. `il` : `v = pico8_hp_il(self)`, `return 0` si `v` est `REF NULL`. **`lq` : NULL → `*out_value = pico8_value_function(pico8_hp_lq_dispatch); return 1`** (méthode de classe masquée par le membre, décision de 1a.33 gardée : c'est là que 1a rendait la fonction, pas 0), sinon `pico8_value_ref(lq)`. Nom inconnu → `return 0`. Dans tous les `return 0`, `*out_value` reste intact. Le commentaire du lecteur (« A nil `lq` member falls back to the class's `lq` method… ») reste exact. |
| R5 | **Écrivain : vacant** (voir plus bas). **Boucle de clés : exercée, par une table intermédiaire** (forme fixée ici, voir plus bas). |
| R6 | Lectures directes, désignées par le compilateur (voir le tableau plus bas). Il n'y a aucune écriture de membre hors du constructeur. |
| R7 | **Forme `offsetof` (`members[]`), première de la liste**, voir plus bas. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_hp_t)` est gardé. `tools/member_census.txt` n'est pas édité. |
| R9 | 240 → 223, aucune ligne `pico8_hp_t`. Les six `has_` ne comptent pas. |

### R5, moitié boucle de clés : le déballage de `ri` (forme fixée ici)

Les clés de construction n'arrivent pas directement aux membres, contrairement à `mh` et `trade`. Elles passent par `hp_accepts_key` (`assert` et message **inchangés**), puis par la table locale `ri`, puis par `ho`, qui réécrit `o`/`dm` par `r`. Enfin la recopie `ri` → membres (`:206-211`) les pose. **C'est cette recopie qui devient le déballage R5** (G) :

- **Une aide `static` par genre**, appelée une fois par membre, dans l'ordre d'aujourd'hui (`px`, `lr`, `fj`, `ja`, `o`, `il`, `dm`, puis `rr`, `ii`, `cw`, `go`, `rn`) :
  - `hp_take_string(ri, "px", &self->px)` ;
  - `hp_take_fixed(ri, "fj", &self->fj, &self->has_fj)` ;
  - `hp_take_il(ri, &self->il, &self->has_il)`.

  Les deux boucles sur `g_hp_mr_names`/`g_hp_named_keys` deviennent ces douze appels. `i` n'en fait pas partie : c'est une entrée de `fi`, lue dans une locale (R3). `lq`, `mr`, `gp` et `fk` ne sont pas des clés.
- **Chaque aide applique R5 de 1b.1** :
  - clé absente de `ri`, ou `REF NULL` → nil, soit pointeur NULL, soit `has_<m>` = 0 ;
  - genre de la ligne de recensement → `<m> = value.as.<champ>`, `has_<m> = 1` s'il existe. Pour `il` : `BOOL` 0 → `il = NULL, has_il = 1` ; `STRING` → `il = s, has_il = 1` ;
  - tout autre genre, **`BOOL` 1 compris pour `il`**, ou un `STRING` de pointeur NULL → `assert` « champ sans règle ». Son message nomme les membres de l'aide et leur ligne de recensement, au format de `mh`/`trade`. Exemple : `"hp.fj/o/dm/ii/rn: a key neither nil nor fixed is off its census row (fixe) -- a field under no rule of plan-3 section 5: stop, do not widen the member"`. Un message par aide, puisqu'un littéral ne peut pas porter le nom passé en argument.
- `value_is_nil` est gardée si les aides l'utilisent, et supprimée sinon (`-Werror` sur une `static` inutilisée).

**Ce qui reste pour l'écrivain.** Ces aides déballent une `pico8_value_t` vers un membre typé. Un futur `set_field` pourra les réutiliser, ou écrire les siennes : c'est à trancher par 1b.13.

### R5, moitié écrivain : toujours vacante

`hp` n'a **pas** d'écrivain par nom : `g_hp_struct_class.set_field` vaut NULL (`research.c:130`), le recensement dit `l` sans `e`, et `test_hp_is_a_struct` le fige (`cls->set_field == NULL`, `must_abort(write_px)` sur l'`assert` de `pico8_entity_set_field`). (G) En ajouter un pour « exercer R5 » serait un écrivain non mesuré : c'est un critère d'échec. La moitié écrivain reste à fixer. **Première candidate : 1b.13** (`jc_too_far.fo`), comme l'écrit 1b.8.

### R7, forme `offsetof` (`members[]`) : première application

`g_hp_members[]` est la première table `offsetof` de la liste 1b (1b.5 la laissait à « la première étape qui en a une », et le PRD de 1b.8 la situait à `g_`, 1b.32 : c'est `hp` qui vient d'abord). R7 de 1b.1 s'y lit ainsi :

- l'entrée de chaque membre typé est retirée de `g_hp_members[]`. `hp` n'a pas de fille, donc aucune table `offsetof(pico8_F_t, parent.m)` n'est à toucher. Les 17 membres étant typés, **la table est vide : elle est supprimée, avec son seul client, l'accesseur `hp_member`** (lecteur, bloc d'ouverture, recopie de `ri`) ;
- ses trois appelants prennent une forme typée : branches R4 du lecteur, affectations R3 du bloc d'ouverture, aides R5 de la recopie ;
- `g_hp_mr_names` (vérification de `hp.mr` à l'init, `:280-288`) et `g_hp_named_keys` (`hp_accepts_key`) sont **gardées** : ce sont des listes de noms, pas des offsets, et leurs `assert` ne changent pas ;
- preuves, vides après :
  - `grep -rn 'offsetof(pico8_hp_t' src/` ;
  - `grep -rnw 'hp_member\|g_hp_members' src/` ;
  - `grep -nE '(->|\.)(mr|gp|fk|px|lr|fj|ja|i|o|il|dm|rr|ii|cw|go|rn|lq)\.(kind|as)' src/pico8_research.c src/pico8_research_display.c src/pico8_planet.c src/pico8_economy_tiers.c tests/test_pico8_entity_kind.c`. Il rend aujourd'hui 17 lignes, toutes sur un `hp`. La restriction de fichiers évite les homonymes (`jo`, `lq`…) encore en `pico8_value_t` ;
  - `grep -n 'pico8_value_t \*' src/pico8_research.c | grep -v 'pico8_value_t \*out_value)'` ne garde **qu'une ligne** : le champ `pico8_value_t *value;` du tableau local `defaults[]` de `pico8_research_init`. Il pointe vers les statiques `g_mr`/`g_gp`/`g_fk`, pas vers un membre.
- **Ce qui reste à fixer** : la forme d'une table `offsetof` qui **garde** des entrées après l'étape, ou qui a des filles (`jo` 1b.23a-d, `g_` 1b.32). La preuve `grep offsetof` vide de 1b.1 y suffit sans doute, mais aucune étape ne l'a encore exercée.

### R6 : lectures directes hors du constructeur (le compilateur les désigne)

| site | aujourd'hui | après |
|---|---|---|
| `research.c` `po` | `pico8_research_lookup(pico8_hc(), self->ja, …)` | même appel avec `self->ja != NULL ? pico8_value_string(self->ja) : pico8_value_ref(NULL)`. `pico8_research_lookup` est gardée, puisque `mc` l'appelle encore avec une clé `FIXED` |
| `research.c` `po`, `od` | `pico8_research_truthy(self->il) && pico8_oj(self->il)` | `v = pico8_hp_il(self)`, puis `pico8_research_truthy(v) && pico8_oj(v)` |
| `research.c` `od` | `self->fj.kind == FIXED` → `self->fj.as.fixed` ; `truthy(self->ii)` | `self->has_fj` → `self->fj` ; `self->has_ii` |
| `research_display.c` `jb_join`, `lq` | `self->i.kind == REF && …`, `PICO8_REF_AS_LIST_CONST(self->i.as.ref)` | `self->i == NULL` / `self->i`, sans macro |
| `research_display.c` `jb` | `truthy(self->ii)` ; `!nil(self->o)`, `tostr(self->o)` | `self->has_ii` ; `self->has_o`, `pico8_value_tostr(pico8_value_fixed(self->o))` |
| `research_display.c` `lq` | `pico8_jo_ex(is, self->o, …)` ; `!nil(self->px)` → `list_add(…, self->px)` | `self->has_o ? pico8_value_fixed(self->o) : pico8_value_ref(NULL)` ; `self->px != NULL` → `pico8_value_string(self->px)`. La signature de `pico8_jo_ex` ne change pas |
| `research_display.c` `bj_predicate` | `pico8_oj(b->rr)` ; `b->lr.kind == STRING && …` | `pico8_oj(b->rr != NULL ? pico8_value_string(b->rr) : pico8_value_ref(NULL))` ; `b->lr != NULL`, `b->lr[i]` |
| `planet.c` `jo_lz` | `hp_instance->i…` ; `pico8_jo_present(hp_instance->o)` → `set_name(kw, "qe", hp_instance->o)` | `hp_instance->i` ; `hp_instance->has_o` → `pico8_value_fixed(hp_instance->o)` |
| `planet.c` `jo_hu` | `truthy(cw)`, `cw_val.kind == STRING` → `jo_kv(…)` ; `truthy(go)`, branches `FIXED`/`STRING` ; `truthy(rn)` → `pico8_rn(rn)` | `cw != NULL` → `jo_kv(hp_instance->cw)` ; `go != NULL` → `pico8_ob_table_get_name(pico8_df(), hp_instance->go, …)`. La branche `FIXED` de `go` tombe, car `go` est `chaine` seulement. `has_rn` → `pico8_rn(pico8_value_fixed(hp_instance->rn))` |
| `economy_tiers.c` `bb` | `!(nil(hn->dm)) && economy_truthy(hn->dm)` → `qe = hn->dm` | `hn->has_dm` → `qe = pico8_value_fixed(hn->dm)`. Un fixe présent est vrai en Lua |
| `test_pico8_entity_kind.c:209-214` | `pico8_value_t cw = hp->cw;` REF NULL → `continue`, puis `assert(STRING && strcmp…)` | `const char *cw = hp->cw;` NULL → `continue`, puis `assert(strcmp(cw, "pm") == 0 \|\| strcmp(cw, "ga") == 0)`. Même message imprimé, même compte |

- (G) **Pas d'emballage public pour les fixes à indicateur** (`o`, `dm`, `rn`…). 1b.5 réserve l'emballage aux genres mêlés. Les trois sites qui veulent une `pico8_value_t` « nil ou fixe » l'écrivent en ligne.
- `#include "pico8_ref_audit.h"` est retiré de `research_display.c`, qui n'en a plus d'usage, et de `planet.c` si son seul `PICO8_REF_AS_LIST_CONST` (`:439`) était le dernier (grep avant de retirer).
- Aucun site par nom ne change, et aucun n'est remplacé par un accès direct (`mv(b, "lq")`, `field_ref_instance(pt, "hn")`, `_jo_fe_b`, lectures des tests).
- **Commentaires** (règle de `CLAUDE.md`) :
  - les contrats des `has_` s'écrivent au format de `trade.h` (`/* 0: \`fj\` is nil, and is then never read */`), celui de `has_il` comme ci-dessus, et celui de `pico8_hp_il` en une phrase ;
  - le commentaire de module de `research.h` (`:34-36`) perd sa moitié « a nil or wrong-kind member reads as its zero/false default », qui devient fausse : un mauvais genre est désormais un `assert` à la construction. Il garde « a NULL project argument is a safe no-op » ;
  - le commentaire du typedef (TROU-7, chaînes empruntées) et celui du lecteur (`lq`) restent exacts ;
  - le `/* TROU-7 */` du constructeur est gardé, ainsi que son commentaire sur `i` (« `i` must be rewritten after `ho`… »), qui reste exact ;
  - aucun autre commentaire.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 240 sur l'arbre de départ (1b.8 commitée), puis 223.
- **H2** : aucun `pico8_entity_set_field` n'atteint un `hp`, hors `must_abort(write_px)` (recensement `l` sans `e`).
- **H3** : aucun membre de `hp` n'est écrit hors de `pico8_hp_struct_new` (grep `->(m) =` sur les fichiers qui nomment `pico8_hp_t` : seul le constructeur). Le recensement de fin de constructeur est donc exhaustif.
- **H4** : dans les 15 rejeux et les TU, `il` ne tient jamais `BOOL` 1, et aucune chaîne de `hp` n'a un pointeur NULL. **Au moins un projet de `gb` a `il` = `BOOL` 0 présent** (c'est d'où vient `bool` dans la ligne). Sinon, la mutation 4 n'est rougie que par la caractérisation, et c'est noté.
- **H5** : après `ho`, `o` et `dm` sont toujours `FIXED` ou absents : la recherche dans `r` réussit pour chaque enregistrement (ligne `fixe` seulement). Et `px`, `lr`, `ja`, `rr`, `cw`, `go` ne tiennent jamais `BOOL` 0 (pas de `f` à leur place dans `fd`). Si l'une de ces deux conditions tombe, une aide R5 avorte au chargement : c'est l'arrêt « champ sans règle ».
- **H6** : les 15 empreintes et les valeurs figées ne bougent pas : `GB_HASH`, `DEFAULTS_HASH`, `FUNCTIONS_HASH`, `FUNCTIONS_INVENTED_HASH`, `INSTALL_HASH`, `BUILT_REPLAY_HASH`, `GEOHARVEST_REPLAY_HASH` (`test_pico8_hp`), la ligne `[cw]` de `test_pico8_entity_kind`, et toutes les empreintes des TU qui lisent `gb`/`hn` par nom.
- **H7** : `test_pico8_hp` (dans `UNIT_TESTS`, hors `UNIT_TESTS_SLOW`) reste sous 30 s avec la caractérisation, qui rejoue au plus une fixture de plus (cas 4). Sa durée n'est pas mesurée ici.
- **H8** : `make class-coverage-net` rend `hp  510  15/15`, identique à la référence.
- **H9** : `make float-audit` (dans `make e2e`) retrouve ses deux clés C1 voisines (`economy_tiers.c | bb_visit | mi`, `research_display.c | pico8_jo_ex | loop_bound`) avec leur extrait inchangé. La disparition de lectures `.as.fixed` non listées (`od`) n'y est pas une erreur.
- **H10** : le build `-DPICO8_MEMBER_CENSUS` compile encore sur un `pico8_hp_t` sans `pico8_value_t`. Il n'est pas lancé.
- **H11** : la branche `dm` de `bb` (`economy_tiers.c:208-214`) est atteinte par un TU (rejeu `BUILT` ou `GEOHARVEST` de `test_pico8_hp`, sinon `test_pico8_rich_planet_exports`). Sinon, c'est un trou `gcov` justifié par le rejeu du filet (`make e2e`).
- **H12** : le rejeu `BUILT` passe par au moins une frame où un `js` de `hv` porte `_jo_fe_b` (panneau de construction ouvert). Sinon, le cas 4b de la caractérisation se réduit à `jo.hn`, et l'identité de `_jo_fe_b` reste couverte par `test_pico8_js` (`copied`, diff vide). C'est noté dans le commit.
- **H13** : `pico8_ob_table_set_index`/`set_name` copient la `pico8_value_t` sans dupliquer la chaîne (seul le nom de clé est dupliqué, `ob.c:124`). Le pointeur d'un membre chaîne est donc celui de l'enregistrement de `fd`.
- **H14** : sous ASan+UBSan, rien de nouveau (`detect_leaks=0`, `i`/`lq` toujours perdus, chaînes empruntées).
- **H15** : `test_pico8_rich_planet_exports` compile et passe sans diff (cast `(pico8_hp_t *)` seul).

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne fait aucun accès `->m` à un `hp`. Elle lit par le **lecteur brut** (`pico8_hp_struct_class()->field`, avec un témoin) et par `pico8_entity_get_field`/`pico8_mv`, et compile sur les deux formes.
- **Caractérisation, figée sur l'ancien code** : `test_hp_members_by_name`, dans `test_pico8_hp`, lancée par `in_fresh_child` après `test_hp_is_a_struct`. Pas de nouveau TU. Le témoin est un fixe `0x1234` posé dans `out` avant chaque lecture. Les noms lus sont les 17 membres, plus les intrus `""`, `"x"`, `"hp"`, `"me"`, `"has_ii"`, `"has_il"`, `"il_is_bool"`.
  1. **`bare`** (`pico8_hp_new(NULL)`) :
     - `mr`, `gp`, `fk` : lecteur à 1, `REF`, mêmes pointeurs que `project(1)`, et `fk` égal à `pico8_hp_fk().as.ref` ;
     - `i` : `REF` d'étiquette `LIST`, `count` 0 ;
     - `lq` : lecteur à 1, `FUNCTION` ;
     - les douze autres membres et chaque intrus : lecteur à 0, `out` égal au témoin.
  2. **Les 34 projets de `gb`** : pour chaque membre, le lecteur brut rend 1 avec un genre de la ligne de recensement, ou 0 avec `out` intact. Il ne rend **jamais** 1 avec `REF NULL`, ni `BOOL` 1 pour `il`. On plie `(k, membre, retour, genre)` dans une empreinte nouvelle, **`RAW_PRESENCE_HASH`**. On compte aussi les trois états de `il` : `IL_STRING_COUNT`, **`IL_FALSE_COUNT`** (H4, attendu ≥ 1) et `IL_ABSENT_COUNT`. Les quatre valeurs sont relevées sur l'arbre de départ et écrites en `#define` avant le typage. Elles distinguent « présent faux » d'« absent » et « présent 0 » d'« absent », ce que `lookup` (qui filtre `REF NULL`) et `GB_HASH` ne font qu'en partie.
  3. **Identité des pointeurs** : on apparie le k-ième enregistrement non chaîne de `pico8_fd()` au projet `k`, dans l'ordre de `build_gb`.
     - `px`, `lr`, `ja` et `il` quand c'est une chaîne sont lus par nom et comparés, **par adresse**, aux positionnels 1, 2, 4 et 7 de l'enregistrement. `rr`, `cw`, `go` le sont à ses clés nommées.
     - Deux lectures successives de `i`, `lq` (membre), `mr`, `gp`, `fk` rendent le même pointeur.
     - Pour un projet avec `ii`, `mv(b, "lq")` rend le pointeur du membre.
  4. **Cibles REF** :
     - (a) `pico8_jo_lz(new_planet(…), b)` pour un projet `b` ordinaire, puis `hn` lu par nom est `b`, avec `gb_rank(b)` inchangé ;
     - (b) sur le rejeu `BUILT` (frames jouées jusqu'au premier `js` de `hv` qui porte `_jo_fe_b`, H12), `_jo_fe_b` lu par nom est un projet de `gb` (`gb_rank ≥ 1`), et `pico8_hp_od` sur lui rend la même valeur que sur `project(gb_rank)`.

  Elle est lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite. Le diff de `test_pico8_hp.c` est un pur ajout.
- **Tests existants qui figent déjà beaucoup**, inchangés : `test_gb` (`GB_HASH`, genres et valeurs par nom des 34 projets, `mr`/`gp`/`fk` partagés, `gb[px]`), `test_functions` (`po`, `od`, `jb`, `lq`, `mv(b,"lq")`, avant et après invention des `il`), `test_install` (`lz`/`hu` : `cw`, `go`, ordinaire, `NULL`), les deux rejeux, `test_hp_is_a_struct`, `test_rebuild_and_shutdown`, et `test_every_cw_is_pm_or_ga` (forme seule).
- **Tests de régression (§6)** : aucun n'est touché. `rich_planet_exports` a un diff vide (H15). Il n'y a pas de preuve rouge à refaire.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune lancée par `make tmp/test_pico8_hp && ./tmp/test_pico8_hp`. Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : le lecteur ignore `has_ii`. Attendu rouge : caractérisation 1 et 2, `test_hp_is_a_struct` (`!has(bare, own[n])`), `GB_HASH`, `INSTALL_HASH` (`project_with(NULL)` change).
  2. (§7-2) *« présent 0 » là où c'était absent* : `hp_take_fixed` pose `has_<m> = 1` sur une clé absente ou nil. C'est la branche R5 `REF NULL` de la boucle de clés. Attendu rouge : caractérisation 1 et 2, `test_hp_is_a_struct`, `GB_HASH`, `FUNCTIONS_HASH` (`lq` construit pour tous, `od`).
  3. (§7-3) *init qui saute un membre typé* : la recopie de `ri` saute `dm`, donc `has_dm` reste à 0. Attendu rouge : caractérisation 2, `GB_HASH`, et un rejeu si H11 tient.
  4. Propre à l'étape, *faux devenu nil, invisible dans `po`/`od`* : `pico8_hp_il` rend `REF NULL` quand `il` est NULL. `FUNCTIONS_HASH` ne bouge pas, car `po`/`od` ne distinguent pas `false` de nil. Attendu rouge : caractérisation 2 (`IL_FALSE_COUNT`, `RAW_PRESENCE_HASH`), `GB_HASH` si H4 tient.
  5. Propre à l'étape, *méthode `lq` perdue* : la branche `lq` du lecteur rend 0 sur NULL. Attendu rouge : caractérisation 1, `test_hp_is_a_struct` (`lookup(bare,"lq")` `FUNCTION`), `fold_functions` (`assert(pico8_mv(b,"lq"…))`), et les rejeux (`assert(0 && "hp must expose \"lq\"…")` de `jo:fe`).
  6. Propre à l'étape, *chaîne copiée au lieu d'empruntée* : `hp_take_string` pose `strdup(value.as.string)`. `GB_HASH` ne bouge pas, puisque le contenu est le même. Attendu rouge : **caractérisation 3 seule**. C'est ce qui justifie ce cas.
  - **Trou justifié** : les `assert` « champ sans règle » des trois aides R5 ne sont atteints par aucune entrée, ni `fd`, ni un TU (H4, H5). Un `must_abort` qui les viserait (par exemple positionnel 7 = `BOOL` 1) serait rouge sur l'ancien code, qui accepte cette clé. Il ne peut donc pas faire partie de la caractérisation figée. (G) Ce PRD n'en ajoute pas, comme 1b.5 et 1b.7. Le commit note la mutation « `assert` retiré de `hp_take_il` » et l'absence de rouge.
- **`gcov`** : `test_pico8_hp` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, puis `gcov` sur `pico8_research.c`, `pico8_research_display.c`, `pico8_planet.c` et `pico8_economy_tiers.c`. Chaque ligne ajoutée ou modifiée doit être exécutée : les 17 branches du lecteur, présent comme absent, et les deux branches de `lq` ; les trois aides R5 (absent et chaque genre accepté) ; le bloc d'ouverture ; `po`, `od`, `jb`, `lq`, `bj_predicate` ; les branches `cw`, `go`, ordinaire et `rn` présent/absent de `jo_hu` ; `jo_lz`. Trous attendus et justifiés :
  - la branche `self == NULL` de `pico8_hp_il` : ses appelants ne passent jamais NULL, et la caractérisation, qui doit compiler sur l'ancien code, ne peut pas l'appeler ;
  - les `return 0` sur `i == NULL` (lecteur), `self->i == NULL` (`jb_join`, `lq`) et `mr`/`gp`/`fk` NULL : inatteignables, puisque le constructeur alloue toujours `i` et ne tourne qu'après `pico8_research_init` ;
  - la branche d'échec des `assert` R5 (voir le trou ci-dessus) ;
  - la branche `dm` de `bb`, si H11 tombe.

  Aucun autre trou n'est attendu. Le rapport va dans le commit.
- **Prior art** : `test_gb`/`test_functions`/`test_hp_is_a_struct` (`test_pico8_hp`, 1a.33), `test_trade_members_by_name` (1b.5, lecteur brut et témoin `0x1234`), `test_hy_members_by_name` (1b.7, les trois états de `hl`), `test_fm_members_by_name` (1b.8).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make sanitize-net` : ~4 min seul, le double sous charge ; on ne le lance **jamais** en même temps que `e2e` ;
- `make class-coverage-net` : 3 à 4 min ;
- `make value-members` : quelques secondes.

Caractérisation, mutations et `gcov` passent par un binaire seul (`make tmp/test_pico8_hp`), jamais par `make test`.

**TU de `UNIT_TESTS_SLOW` touchés : aucun.** Rien à compiler en avance. Les TU lents qui lient `research.c`/`planet.c` (`jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ne nomment aucun membre de `hp`, donc une erreur de compilation n'y est pas attendue. `make e2e` les construit et les lance.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` sans erreur de liste C1, et les `UNIT_TESTS_SLOW` qui passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 223`**, sans aucune ligne `pico8_hp_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**. Le diff touche le corps du constructeur `pico8_hp_struct_new` (R3, R5), même si `pico8_class_coverage_record("hp")` et `KNOWN[]` ne bougent pas. C'est la même lecture du §7 qu'en 1b.5 et 1b.7. La sortie doit être identique à `tools/class_coverage_net_expected.txt` (`hp  510  15/15`).

`make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si :
  - une aide R5 avorte dans un rejeu ou un TU (H4, H5) ;
  - `il` tient `BOOL` 1, ou une chaîne de `hp` a un pointeur NULL ;
  - un membre est ajouté à `hp`, sauf les six `has_` ;
  - un `set_field`, une méthode de descripteur ou une clé de construction acceptée est ajouté ;
  - le lecteur accepte un autre nom que les 17 membres.
- `FAILED:` si un type diffère de la Solution, si un indicateur n'est pas un `int` déclaré juste après son membre, ou si `il_is_bool` ou un `has_` de pointeur (autre que `has_il`) existe.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un membre nil ou un nom inconnu, ne rend pas la méthode pour `lq` NULL, ne rend pas le même pointeur pour les chaînes, `i`, `lq`, `mr`/`gp`/`fk`, ou lit absent un `il` `false`.
- `FAILED:` si `g_hp_members[]` ou `hp_member` survivent, si `pico8_hp_il` manque à `research.h`, ou si une preuve de R7 n'est pas vide (ou garde autre chose que la ligne `defaults[]` de `pico8_research_init`).
- `FAILED:` si une chaîne de `hp` est dupliquée, si une libération est ajoutée pour `i`, `lq`, une chaîne ou `il` (TROU-7), si l'allocation ou la libération d'un `hp` change, ou si un autre écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si `jo.hn`, `js._jo_fe_b`, leurs écrivains (`pico8_jo_lz`, `pico8_jo_fe_step`) ou leurs lecteurs changent, ou si `pico8_hp_assert` change.
- `FAILED:` si le message d'un `assert` existant change (`hp: every measured record of fd passes only positionals 1-8…`, `hp:pq registers itself in gb…`, `hp: every gb entry, every planet's hn…`, `hp: mr, gp, fk are tables…`, `hp: the members ho fills are exactly hp.mr's names…`, `hp must expose "lq"…`).
- `FAILED:` si `g_mr`/`g_gp`/`g_fk`, `pico8_hp_fk`, `pico8_hp_default`, `pico8_jo_ex` ou `pico8_ho_apply` changent de type ou de signature.
- `FAILED:` si un extrait C1 de `tools/float_audit_c1.txt` ou ce fichier changent.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation à `test_pico8_hp.c` (et ses `#define`) et changer la forme de `test_every_cw_is_pm_or_ga`. Également si une lecture par nom change, ou si une valeur figée existante change (`*_HASH` de `test_pico8_hp`, empreintes des autres TU).
- `FAILED:` si un test de régression de bug (§6) a un diff non vide.
- `FAILED:` si un site par nom de `src/` change ou est remplacé par un accès direct.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 223 (ou 229 puis 223 en découpage de repli), liste encore `pico8_hp_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si `make class-coverage-net` n'a pas de ligne « Runs » identique à la référence, ou si cette référence, les exemptions ou `KNOWN[]` changent.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit un membre directement, ou si ses valeurs (`RAW_PRESENCE_HASH`, les trois comptes de `il`) changent après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 6 mutations, chacune avec son test ou la justification de son trou, plus la note sur les `assert` R5 non atteints.
- `FAILED:` s'il manque la note « Découvert à 1b.9 », ou si elle omet la forme R7 `offsetof`, la forme R5 de la boucle de clés par table intermédiaire, ou le constat « R5 écrivain toujours vacante ».
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- `jo.hn` (1b.23) et `js._jo_fe_b` (1b.34), et leurs tables `offsetof`.
- Les statiques de classe `g_mr`/`g_gp`/`g_fk`, `pico8_hp_fk()` et le type des entrées de `gb`.
- Libérer `i`, `lq` ou les chaînes (TROU-7).
- Donner une forme à la moitié écrivain de R5 (1b.13), ou à une table `offsetof` qui garde des entrées ou a des filles (1b.23, 1b.32).
- La forme {bool, pointeur} quand `true` peut être écrit (`qh.et`, `g_.et`, `mz.et`, `js.my`).
- Remplacer une lecture par nom par un accès direct.
- Régénérer `tools/member_census.txt` (R8).
- Les fichiers d'événements et `FRAMES_EVENTS`.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `research.c/.h` réécrits, plus les lectures directes de `research_display.c`, `planet.c` et `economy_tiers.c` ;
  - tests : `test_pico8_hp` étendu d'une caractérisation au lecteur brut, sans nouveau TU, et `test_pico8_entity_kind` retouché dans sa seule forme ;
  - un commit, avec le découpage de repli écrit (229 puis 223) ;
  - `il` en `const char *` + `has_il`, où NULL présent vaut `false` (lecture 1b.7, cart sans `true`) ;
  - un emballage public `pico8_hp_il`, appelé aussi par `po` et `od` ;
  - aucun emballage pour les fixes à indicateur ;
  - cinq `has_` sur preuve `lu` directe ;
  - `lq` NULL lu comme la méthode ;
  - boucle de clés : trois aides R5 par genre sur `ri` ;
  - aucun `set_field`, donc R5 écrivain vacante ;
  - `g_hp_members[]` et `hp_member` supprimés ;
  - aucun `must_abort` nouveau pour les `assert` R5 ;
  - `class-coverage-net` lancé.
- **Note « Découvert à 1b.9 »**, à écrire sur la ligne 1b.9 de `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante :
  - « lecture de R1 {bool, chaine} de 1b.7 appliquée à `il` sans écart (preuve : `ob` n'a pas de littéral `true`, et `hp` n'est écrit que par son constructeur, depuis `fd`) ; emballage `pico8_hp_il` appelé aussi par les lectures directes (`po`, `od`) » ;
  - « **R7, forme `offsetof`** (`members[]`), première de la liste : entrées retirées, table vide supprimée avec son accesseur (`g_hp_members[]`, `hp_member`) ; listes de noms sans offset gardées ; preuves `grep -rn 'offsetof(pico8_hp_t' src/` et `grep -rnw 'hp_member\|g_hp_members' src/` vides. Reste à fixer : une table qui garde des entrées ou a des filles (`jo`, `g_`) » ;
  - « **R5, boucle de clés par table intermédiaire** (`ri` après `ho`) : la recopie devient une aide `static` par genre, un appel par membre, clé absente = nil, `assert` « champ sans règle » par aide, qui nomme ses membres. **R5 écrivain toujours vacante** (`hp.set_field` NULL) ; première candidate : 1b.13 » ;
  - « R4 : un membre qui masque une méthode (`lq`) rend la méthode quand il est NULL, pas 0 ».

  Le reste va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le compte 240 → 223 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov` ;
  - les 6 mutations et le trou justifié des `assert` R5 ;
  - les valeurs relevées de la caractérisation (`RAW_PRESENCE_HASH`, comptes de `il`) ;
  - les réponses à H1 à H15.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
