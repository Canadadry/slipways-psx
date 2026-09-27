---
title: "Plan 3, étape 1b.11 : `ie_ke` (la vue de classe de l'icône de recherche) — quatre membres typés, `rr`/`me` en `const char *` empruntés, `s`/`od` fixes avec `has_`, la recopie depuis la chaîne du genre en aides R5 par genre"
description: "`pico8_ie_ke_t` est la vue par valeur que `ie` construit sur la pile pour lire `rr`, `s`, `me`, `od` sur la chaîne du genre tenu par `self.ke`. Ses quatre membres `pico8_value_t` sont typés selon le patron R1–R9 de 1b.1, et `make value-members` passe de 217 à 213. La ligne de recensement sous-estime les lectures par nom : le seul TU qui lit la vue nil par nom (`test_pico8_ie`) est dans `UNIT_TESTS_SLOW`, que le recensement ne passe pas. `s` reçoit donc un `has_s` que sa ligne ne demande pas. Ni table d'offsets ni écrivain : R7 et la moitié écrivain de R5 restent vacantes."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.11 — `ie_ke` (`research_icon.h:40`). Empreinte 26 (7, 19). Compte : 213.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié), le §6 (tests de régression) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 ». Il est cité ici, pas redécidé. S'appliquent aussi :

- « Découvert à 1b.3 » : sur un objet sans constructeur de classe, R3 se lit sur ce qui l'initialise (ici le constructeur de vue `pico8_ie_ke_view`) ;
- « à 1b.4b » : la table corrigée fait foi. Elle ne demande aucun `has_` pour `ie_ke` : aucune de ses lignes n'est dans la liste (3) de 1b.4b. Voir la décision R2, qui s'en écarte pour `s` ;
- « à 1b.5 » : la forme R4 d'une branche `strcmp` par membre et la preuve mécanique `grep -n 'pico8_value_t \*' …` ;
- « à 1b.6 » : un lecteur qui **copie** (`value = view->m`) n'est pas un `pico8_X_member`. R7 est vacante quand la struct n'a ni table `offsetof` ni accesseur à pointeur ;
- « à 1b.7 » : sa lecture de R2 (nil `stat?` venu d'un homonyme, sans indicateur quand toute écriture est prouvée non nil) **ne s'applique pas ici**, car la vue est bel et bien écrite nil (voir « Nil, membre par membre ») ;
- « à 1b.8 » : **R5 écrivain toujours vacante**, la première étape à écrivain par nom étant 1b.13. Le sélecteur local au lecteur est retiré au profit de branches R4 explicites ;
- « à 1b.9 » :
  - **R5, boucle de clés par table intermédiaire** : une aide `static` par genre (`hp_take_string(ri, "px", &self->px)`, `hp_take_fixed(ri, "fj", &self->fj, &self->has_fj)`), un appel par membre dans l'ordre de l'ancienne recopie. Chaque aide pose d'abord nil, sort sur clé absente ou `REF NULL`, sinon lance un `assert` « champ sans règle » qui nomme ses membres et le genre de leur ligne, puis recopie `value.as.<champ>` et pose `has_` = 1 ;
  - **R7 forme `offsetof`**, et sa preuve mécanique à deux `grep -v` (`out_value)` et `out)`) ;
- « à 1b.10 » (PRD `docs/prd/triage/plan3-phase1b-etape10-lq-rangee-mise-en-page.md`, précondition ci-dessous) : les aides R5 prennent leur source telle qu'elle vient, sans table à chercher quand il n'y en a pas.

### État de départ

HEAD b082136. **Précondition** : 1b.10 (`lq`) est commitée avant que 1b.11 commence. Elle est en cours d'implémentation (arbre de travail : `layout.h` et `test_pico8_lq.c` modifiés). D'après son PRD, elle touche `layout.[ch]`, `ui_button.c` et `test_pico8_lq.c`, et aucun fichier de cette étape. Les numéros de ligne cités ici (ceux de HEAD) ne devraient donc pas glisser. Chaque site est aussi désigné par son contenu. `make value-members` rend alors 217 (H1). Aujourd'hui, avant 1b.10, il rend 223, et les quatre lignes `pico8_ie_ke_t` (`research_icon.h:40-43`) y figurent : relevé par la lecture statique du script, sans build.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H15).

### Lignes de recensement (`tools/member_census.txt:182-185`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `rr` | offset | `chaine` | observé | stat? | - |
| `s` | offset | `fixe` | observé | stat? | - |
| `me` | offset | `chaine` | observé | stat? | l |
| `od` | offset | `fixe` | observé | stat? | l |

Quatre membres, donc 217 − 4 = **213**, comme le dit la ligne. Aucun n'a la colonne `e` : la vue n'a pas d'écrivain (voir R5). Aucune ligne de la table ne pointe vers `pico8_ie_ke_t` : la vue ne vit que sur la pile. Le membre `ie.ke` (`ref:autre`, 1b.20) tient le **genre** (`pico8_entity_kind_t`), pas la vue.

**Ce que la table ne voit pas.** `make member-census` passe `FRAMES_EVENTS` et `CENSUS_UNIT_TESTS`, qui est « `UNIT_TESTS` … moins `UNIT_TESTS_SLOW` » (Makefile, commentaire de `CENSUS_UNIT_TESTS`). Or le seul TU qui lit la vue par nom hors de `dl`, **`test_pico8_ie`, est dans `UNIT_TESTS_SLOW`**. Son cas `test_ke_view_is_a_struct` (`tests/test_pico8_ie.c:851-903`) lit par nom `rr`, `s`, `me` et `od` sur les sept vues, puis **les quatre, absents, sur la vue d'un `ie` sans `ke`** (`assert(!has(&view, members[k]))`, `has` = `pico8_entity_get_field`). C'est une preuve `lu` pour les quatre membres, et une lecture par nom (`l`) pour `rr` et `s`, que la table ne peut pas contenir. La ligne est « une entrée de la liste et pas un oracle » (1b.0) : ce PRD la complète, sans l'éditer (R8).

### Ce que fait le cart (`pico8/lua.lua`)

- `ie:hb` (`:926-931`) : `if not self.kh and oj(self.ke.rr) then … end`.
- `ie:kj` (`:936-940`) : `spr(self.ke.s,…)`, puis `dl(self.ke)` si `self.m_`.
- `dl` (`:560-567`) lit `self.me=="kz"`, `self.et`, `self.od`.
- `gx={qh,os,nr,fr,nl,mx,hj}` (`:1181-1185`) : un `ie({ke=b,jb=…})` par genre. `self.ke` **est** la classe Lua, et il n'y a pas de vue dans le cart. La vue est un artefact du portage (PRD 1a.15), qui sert à atteindre la chaîne de défauts d'un genre.
- Valeurs lues, mesurées par 1a.15 et figées par `test_ke_view_is_a_struct` : `rr` absent pour `qh`/`os`, chaîne pour les cinq autres ; `s` = 174, 100, 170, 78, 68, 66, 166 ; `od` = 15, 15, 10, 25, 20, 40, 60 ; `me` = `"kz"` pour les sept (hérité de `qh`). Aucune n'est réaffectée sur une classe du cart.

### Le code aujourd'hui

- `src/pico8_research_icon.h:36-44` : le commentaire du typedef (« `self.ke` read as Lua reads a class: no parent, never an entity, and no writer, so a write reaching it stops the run »), puis `{ pico8_struct_header_t header; pico8_value_t rr, s, me, od; }`, un membre par ligne. Le commentaire de module (`:12-18`) dit « a nil or wrong-kind member, or an absent field of `self.ke`, reads as its zero/false default ». Le contrat de `pico8_ie_ke_view` (`:82-84`) dit « By value, one per read: each member copied off the chain of the class `self.ke` holds, nil when absent there or when `self.ke` is ».
- `src/pico8_research_icon.c` :
  - `ie_self_ke_kind` (`:58-67`) : le genre tenu par `self->ke`, ou NULL. **Inchangé** : il lit `ie.ke`, qui est l'affaire de 1b.20 ;
  - lecteur `pico8_ie_ke_view_field` (`:69-89`) : une chaîne `strcmp` qui **copie** le membre dans une locale `pico8_value_t value`, puis `REF NULL` → 0, sinon la valeur. Il n'y a pas de `pico8_value_t *`, pas d'`offsetof` : c'est la forme de `pj_cr` (1b.6) ;
  - descripteur `g_ie_ke_view_struct_class` (`:91-95`) : aucune méthode, `set_field` **NULL** ;
  - constructeur de vue `pico8_ie_ke_view` (`:97-115`) : l'en-tête, les quatre membres à `REF NULL`, puis, si le genre existe, `(void)kind->field("rr", &view.rr)` et de même pour `s`, `me` et `od`, puis `PICO8_MEMBER_CENSUS_SEE(&view, pico8_ie_ke_t)` et `return view`. Aucun `pico8_class_coverage_record` ; la vue n'est pas dans `KNOWN[]` (1a.15) ;
  - `pico8_ie_hb` (`:142-144`) : `rr_val = pico8_ie_ke_view(self).rr;` puis `pico8_oj(rr_val)` ;
  - `pico8_ie_kj` (`:227-236`) : `view = pico8_ie_ke_view(self)`, puis `view.s.kind == FIXED ? ie_to_int(view.s.as.fixed) : 0`, puis `pico8_dl(gfx, mem, (pico8_struct_header_t *)&view)` si `m_` et si le genre existe ;
  - la table `members[]` `offsetof` de `pico8_ie_field` (`:271-278`) et la ligne `:295` sont celles d'**`ie`**, pas de la vue : hors étape (1b.20).
- `src/pico8_probe.c` `pico8_dl` (`:250-300`) lit `me`, `et` et `od` par `pico8_entity_get_field`. **Inchangé** : c'est un site par nom, et `et` rend 0 comme aujourd'hui.
- Lecteurs par nom, **inchangés** : `pico8_dl` ; `test_pico8_ie` (`test_ke_view_is_a_struct`, par `has`/`must_fixed`/`is_string`, et `must_abort(write_od)`).
- `pico8_entity_kind_t.field` (`struct.h:103-111`) « returns 0, leaving *out_value untouched, when `name` is absent from the whole chain ». Pour les sept genres, c'est `pico8_<genre>_default`, qui lit la spec du module (`test_kind_reader_is_the_module_default`).

### Nil, membre par membre

La vue est écrite nil dans deux cas : quand `self.ke` est nil (les quatre membres), et pour `rr` sur `qh`/`os` (genre sans `rr`). Le premier cas n'arrive jamais dans le cart (chaque `ie` reçoit `ke=b`), mais il arrive dans `test_ke_view_is_a_struct` et dans `pico8_ie_hb`/`pico8_ie_kj` sur un `ie` sans `ke`.

- `rr` : `chaine`, nil réel (`qh`, `os`, sans `ke`). C'est un pointeur : NULL = nil.
- `me` : `chaine`, nil seulement sans `ke`. C'est un pointeur : NULL = nil.
- `s` : `fixe`, nil seulement sans `ke`, **lu nil par nom** dans `test_ke_view_is_a_struct` (ligne `-` dans la table, voir plus haut). Non-pointeur lu par nom et nil observé : **`has_s`** (décision R2).
- `od` : `fixe`, nil seulement sans `ke`, lu par nom (`l`) par `dl` et par le test, lu nil par le test. **`has_od`** (R2, sans écart).

### Empreinte remesurée

La ligne dit **26 (7, 19)**. Le grep par noms ne la reproduit pas exactement, car les noms `rr`, `s`, `me` et `od` sont parmi les plus partagés du port. `grep -lE 'pico8_ie_ke_t|"(rr|s|me|od)"'` rend 50 fichiers (16, 34), et en ajoutant `(->|\.)(rr|me|od)\b`, 56 (21, 35). Dans les deux cas, c'est presque tout homonyme (`qh.me`, `nr.od`, `hp.rr`, `jo.s`…).

**Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_ie_ke_t`) : **4 (3, 1)**.

- src : `research_icon.h` (typedef), `research_icon.c` (constructeur de vue, lecteur, `hb`, `kj`), `probe.c` (`dl`, par nom, diff vide) ;
- tests : `test_pico8_ie.c` (par nom, et `view.header` seul en accès direct), diff vide.

`grep -rlE 'pico8_ie_ke_t|ke_view' src tests` rend exactement `research_icon.[ch]` et `test_pico8_ie.c`.

**Fichiers modifiés : 3**, plus `docs/plan-3.md` :

- src (2) : `src/pico8_research_icon.h`, `src/pico8_research_icon.c` ;
- tests (1) : `tests/test_pico8_entity_kind.c`, la caractérisation, en pur ajout (voir Testing Decisions : pourquoi pas `test_pico8_ie`).

**TU de `UNIT_TESTS_SLOW` touché : `test_pico8_ie`**, par compilation et par exécution, **avec un diff vide**. Il déclare `pico8_ie_ke_t view`, n'en lit que `header` directement, et fige par nom les valeurs et l'absence des quatre membres. C'est la preuve d'origine de `has_s`. Aucun autre TU lent ne nomme la vue. **Aucun test de régression (§6)** ne lit la vue.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** (un membre par ligne, ordre et noms gardés, en-tête inchangé) :

   ```c
   pico8_struct_header_t header;
   const char *rr;
   pico8_fixed_t s;   int has_s;
   const char *me;
   pico8_fixed_t od;  int has_od;
   ```

   Le lecteur perd sa locale `pico8_value_t value` au profit de quatre branches R4. Le constructeur de vue perd ses quatre `(void)kind->field(…, &view.m)` au profit de deux aides R5 `static`.
3. Effet mesurable : `make value-members` passe de 217 à **213** et ne liste plus aucune ligne `pico8_ie_ke_t`. Aucun pixel ne bouge.

**Pas de découpage** (G). Le diff tient dans un seul fichier `.c` (une trentaine de lignes), un typedef et une caractérisation. Aucun découpage de repli n'est prévu : deux commits laisseraient un lecteur à deux formes pour quatre membres.

## User Stories

1. En tant que lecteur de `research_icon.c`, je veux les quatre membres de la vue typés, afin que `hb` et `kj` ne déballent plus de `pico8_value_t`.
2. En tant que `dl(self.ke)`, je veux lire par nom `me` = `"kz"` et `od` = le coût du genre, avec le même genre et, pour `me`, le même pointeur qu'en 1a, et `et` absent.
3. En tant que `ie:hb`, je veux `rr` absent pour `qh`/`os` (et sans `ke`), de sorte que `oj` réponde vrai sans condition, et `rr` égal à la chaîne du genre pour les cinq autres.
4. En tant que `ie:kj`, je veux le sprite `s` du genre, et le sprite 0 sans `ke`.
5. En tant que `test_ke_view_is_a_struct`, je veux lire `s` et `od` absents par nom sur la vue d'un `ie` sans `ke`, et non « présent 0 ».
6. En tant que lecteur par nom, je veux 0 avec `*out_value` intact pour un membre nil et pour tout autre nom (`et`, `ke`, `has_s`…).
7. En tant que mainteneur, je veux que `rr` et `me` restent **empruntés** à la spec du genre, jamais copiés ni libérés, parce que la vue vit sur la pile.
8. En tant que mainteneur, je veux qu'un champ de la chaîne d'un genre dont le genre sort de la ligne de recensement arrête l'exécution par un `assert` « champ sans règle », au lieu d'être lu comme 0.
9. En tant que mainteneur, je veux que la vue reste sans écrivain : une écriture par nom (`write_od`) avorte toujours.
10. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
11. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux, sans nouvelle allocation (vue sur la pile, chaînes empruntées).
12. En tant que `plan-pilot`, je veux `make value-members` à 213.
13. En tant que `make float-audit`, je veux que le site C1 `research_icon.c | ie_to_int | x` garde son extrait mot pour mot.
14. En tant qu'auteur de 1b.20 (`ie`, dont `ie.ke` et sa table `offsetof`), je veux trouver la vue typée et `ie_self_ke_kind` inchangé.
15. En tant qu'auteur des étapes suivantes, je veux savoir qu'une lecture par nom faite seulement dans un TU de `UNIT_TESTS_SLOW` compte pour R2, même si la table ne la voit pas.
16. En tant qu'auteur de 1b.13 (`jc_too_far.fo`), je veux savoir que 1b.11 **ne** fixe **pas** la moitié écrivain de R5.
17. En tant que vérificateur, je veux une caractérisation par nom lancée sur l'ancien code, les mutations du §7 déclinées sur la vue, la justification de chaque trou, et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, changement chirurgical, critères vérifiables), et marqués (G).

### Décision R2 : `has_s` et `has_od`, sur preuve `lu` d'un TU lent

R2 donne un indicateur « seulement pour un non-pointeur lu par nom (`l`) à nil `observé` ou `tenu`, sauf preuve du PRD ». La table donne `od` = `l`, nil observé : R2 s'applique sans écart. Elle donne `s` = `-` : R2, lu à la lettre, refuserait `has_s`.

**(G) `has_s` est donné.** La preuve : `test_ke_view_is_a_struct` lit `s` par nom (`must_fixed(&view, "s")`) et le lit absent sur la vue sans `ke` (`!has(&view, "s")`). Ce TU est dans `UNIT_TESTS_SLOW`, que `CENSUS_UNIT_TESTS` exclut par construction : la colonne `par-nom` ne pouvait pas le voir. Sans indicateur, `s` sans `ke` serait lu « présent 0 ». La garantie de **Coexistence** (« présent/absent … lus par nom identiques à 1a ») tomberait, et `test_pico8_ie` rougirait dans `make e2e`. C'est une **lecture nouvelle de R2**, à noter dans « Découvert à 1b.11 » : une lecture par nom faite par un TU hors du recensement (`UNIT_TESTS_SLOW`) compte comme `l`, et un nil qu'il lit compte comme `lu`, quand le PRD cite le site.

- La lecture de R2 de 1b.7 (pas d'indicateur quand le nil n'est que `stat?` et que toute écriture est prouvée non nil) ne s'applique pas : le constructeur de vue écrit nil sur les quatre membres quand `self.ke` est nil.
- Les contrats s'écrivent au format de `trade.h` : `/* 0: \`s\` is nil, and is then never read */`, idem `od`.

### Les membres à pointeur : empruntés à la spec du genre

- `rr`, `me` → `const char *` (`chaine`) : le pointeur de la `STRING` que rend `kind->field`. Ce pointeur appartient à la spec du module du genre et vit jusqu'à son `shutdown` (H6). La vue, sur la pile, ne lui survit pas. Jamais dupliqué ni libéré. Le contrat de `pico8_ie_ke_view` gagne une phrase : « `rr` and `me` are borrowed from the kind's defaults ».
- Pas d'indicateur pour eux (NULL = nil, R2).

### R1–R9 appliquées à `ie_ke` (patron de 1b.1, cité)

| règle | sur `ie_ke` |
|---|---|
| R1 | Voir la Solution. `rr`, `me` → `const char *`. `s`, `od` → `pico8_fixed_t`. Pas de genre mêlé : aucune lecture de 1b.5 ou 1b.7. Noms et ordre gardés. |
| R2 | `has_s` (lecture nouvelle, ci-dessus), `has_od`, chacun juste après son membre. |
| R3 | Le constructeur de vue garde sa forme et son ordre : en-tête (`tag`, `cls`), puis **tous les membres posés nil** par affectation typée (`rr = NULL`, `s = 0`, `has_s = 0`, `me = NULL`, `od = 0`, `has_od = 0` : la vue est sur la pile, rien n'est à zéro d'office), puis, si le genre existe, les quatre aides R5, puis `PICO8_MEMBER_CENSUS_SEE`, puis `return view`. |
| R4 | Le lecteur a une branche `strcmp` par membre, dans l'ordre du typedef, avant le `return 0` final. `rr`, `me` : NULL → `return 0`, sinon `pico8_value_string(ptr)`, même pointeur. `s`, `od` : `has_<m>` = 0 → `return 0`, sinon `pico8_value_fixed`. Nom inconnu → `return 0`. Dans tous les `return 0`, `*out_value` reste intact. La locale `pico8_value_t value` disparaît. |
| R5 | **Écrivain : vacant** (voir plus bas). **Source des clés : la chaîne du genre**, forme de 1b.9 (voir plus bas). |
| R6 | Deux lectures directes, désignées par le compilateur (voir le tableau plus bas). Aucune écriture hors du constructeur de vue. |
| R7 | **Vacante**, comme en 1b.6 : pas de table `offsetof`, pas d'accesseur à pointeur. Le lecteur copie ; sa locale devient des branches R4 explicites (comme le sélecteur local de 1b.8). |
| R8 | `PICO8_MEMBER_CENSUS_SEE(&view, pico8_ie_ke_t)` est gardé. `tools/member_census.txt` n'est pas édité. |
| R9 | 217 → 213, aucune ligne `pico8_ie_ke_t`. `has_s` et `has_od` ne comptent pas. |

### R5, moitié « clés » : la recopie depuis la chaîne du genre (forme 1b.9, source différente)

La vue n'a pas de clés de construction. Sa seule source est `kind->field(name, &out)`, un lecteur de chaîne par nom qui rend 0 sans écrire quand le nom manque. C'est la forme de 1b.9 : une source consultée par nom, puis une recopie membre par membre. **Seule la source change** : `kind->field` au lieu de `pico8_ob_table_get_name(ri, …)`. (G) Ce n'est donc pas une lecture nouvelle de R5 :

- **Deux aides `static`, une par genre**, appelées une fois par membre dans l'ordre d'aujourd'hui (`rr`, `s`, `me`, `od`), seulement si le genre existe :
  - `ie_ke_take_string(kind, "rr", &view.rr)` et de même pour `me` ;
  - `ie_ke_take_fixed(kind, "s", &view.s, &view.has_s)` et de même pour `od`.
- **Chaque aide applique R5 de 1b.1** :
  - elle pose d'abord nil (NULL, ou 0 et `has_` = 0), et lit dans une locale `pico8_value_t` initialisée à `REF NULL` ;
  - nom absent de la chaîne (`kind->field` rend 0), ou `REF NULL` → nil ;
  - genre de la ligne (`STRING` à pointeur non NULL, `FIXED`) → `<m> = value.as.<champ>`, et `has_<m> = 1` s'il existe ;
  - tout autre genre → `assert` « champ sans règle ». Son message nomme les membres de l'aide et leur ligne, au format de `hp`/`lq` : `"ie_ke.rr/me: a chain value neither nil nor a string is off its census row (chaine) -- a field under no rule of plan-3 section 5: stop, do not widen the member"`, et `"ie_ke.s/od: a chain value neither nil nor fixed is off its census row (fixe) -- …"`.
- Le bloc « tous nil » de R3 reste avant l'appel, parce qu'il est seul à poser nil quand le genre manque (les aides ne sont pas appelées). Dans les aides, la pose de nil en tête est la règle de 1b.9, gardée telle quelle.

### R5, moitié écrivain : vacante, confirmée

La note de 1b.8 place le premier écrivain par nom à 1b.13. Pour `ie_ke`, c'est **confirmé** :

- `g_ie_ke_view_struct_class` a `set_field` = NULL (`research_icon.c:91-95`) ;
- aucune ligne de la table n'a `e` ;
- `test_ke_view_is_a_struct` le fige par `must_abort(write_od, &view)`, qui avorte sur l'`assert` de `pico8_entity_set_field` ;
- `grep -rn 'set_field' src/pico8_research_icon.c` ne montre que l'écrivain de **`ie`** (`m_`, 1b.20) ;
- aucun écrivain par nom n'est donc à définir ici.

(G) En ajouter un pour « exercer R5 » serait un écrivain non mesuré : c'est un critère d'échec. **Première candidate : 1b.13** (`jc_too_far.fo`), comme l'écrivent 1b.8, 1b.9 et 1b.10.

### R6 : lectures directes (le compilateur les désigne)

| site | aujourd'hui | après |
|---|---|---|
| `research_icon.c` `hb` (`:142-144`) | `rr_val = pico8_ie_ke_view(self).rr;` puis `pico8_oj(rr_val)` | `rr = pico8_ie_ke_view(self).rr;` puis `pico8_oj(rr != NULL ? pico8_value_string(rr) : pico8_value_ref(NULL))`. Le commentaire « `qh`/`os` never set `rr`, so `rr_val` stays absent… » reste exact, au nom de la locale près |
| `research_icon.c` `kj` (`:228`) | `view.s.kind == FIXED ? ie_to_int(view.s.as.fixed) : 0` | `view.has_s ? ie_to_int(view.s) : 0`. L'appel à `ie_to_int` est gardé (C1 de `float-audit`) |

- `pico8_dl(gfx, mem, (pico8_struct_header_t *)&view)` ne change pas : il lit par nom.
- (G) **Pas d'emballage public** : aucun genre mêlé (1b.5, 1b.7), et aucun fixe à indicateur n'en a (1b.9).
- Aucun site par nom ne change, et aucun n'est remplacé par un accès direct.

### Preuves (toutes vides après l'étape, sauf mention)

- `grep -rn 'offsetof(pico8_ie_ke_t' src/` : vide avant comme après (R7 vacante) ;
- `grep -nE '(view|\))\.(rr|s|me|od)\.(kind|as)|view->(rr|s|me|od)\.(kind|as)' src/pico8_research_icon.c` : rend aujourd'hui 1 ligne (`:228`), vide après ;
- `grep -nE 'kind->field\("(rr|s|me|od)", &view' src/pico8_research_icon.c` : 4 lignes aujourd'hui, vide après ;
- `grep -n 'pico8_value_t \*' src/pico8_research_icon.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : **garde une seule ligne**, `:295` du lecteur de **`ie`** (`value = *(const pico8_value_t *)((const char *)ie + members[i].offset)`), qui relève de 1b.20. C'est déjà le cas aujourd'hui. Aucune ligne ne vise un `pico8_ie_ke_t`.

### Commentaires (règle de `CLAUDE.md`)

- `research_icon.h`, commentaire de module (`:12-14`) : la moitié « … or an absent field of `self.ke`, reads as its zero/false default » reste vraie pour un champ **absent**. Un champ de mauvais genre de `self.ke` arrête désormais l'exécution. La phrase est corrigée en conséquence, et la partie sur les membres d'`ie` reste telle quelle (1b.20).
- Contrats `has_s`, `has_od` au format de `trade.h`. Une phrase d'emprunt ajoutée au contrat de `pico8_ie_ke_view`.
- Le commentaire du typedef (« no writer, so a write reaching it stops the run ») reste exact.
- Aucun autre commentaire. Les aides portent leur contrat par leur `assert`.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 217 sur l'arbre de départ (1b.10 commitée), puis 213.
- **H2** : aucun `pico8_entity_set_field` n'atteint une vue, hors `must_abort(write_od)`.
- **H3** : aucun membre de la vue n'est écrit hors de `pico8_ie_ke_view` (grep `view\.(rr|s|me|od) =` et `->(rr|s|me|od) =` sur `research_icon.c` : seul le constructeur de vue).
- **H4** : pour les sept genres de `gx`, `kind->field` rend `rr` en `STRING` non NULL ou absent, `s` et `od` en `FIXED`, `me` en `STRING` non NULL, et jamais `REF NULL` présent (mesure de 1a.15, `KIND_HASH` de `test_pico8_entity_kind`). Sinon, une aide avorte : c'est l'arrêt « champ sans règle ».
- **H5** : dans `test_pico8_entity_kind`, après `pico8_cart_load()` (qui appelle `pico8_research_icon_init`, `cart.c:92`) et `pico8_jl()`, `pico8_ie_new(NULL)` et `pico8_ie_new({ke = REF genre})` construisent un `ie` sans avorter. `pico8_ie_hb` et `pico8_ie_kj` y sont appelables, avec `pico8_je()` non NULL. Leur enregistrement dans `hv`/`cv` ne gêne pas les cas existants, puisque la caractérisation passe en dernier.
- **H6** : la chaîne que rend `kind->field` est celle de la spec du module, la même à chaque appel (identité par adresse entre deux vues).
- **H7** : `test_pico8_entity_kind` (hors `UNIT_TESTS_SLOW`) reste bien sous 30 s. La caractérisation ne rejoue rien.
- **H8** : aucun rejeu du filet ne survole une icône de recherche assez longtemps pour que la mutation 3 y change un pixel. Si c'est le cas, `make e2e` la rougit aussi, ce qui est un bonus et non un besoin.
- **H9** : le build `-DPICO8_MEMBER_CENSUS` compile sur un `pico8_ie_ke_t` sans `pico8_value_t`, comme sur `pico8_hp_t` après 1b.9. Il n'est pas lancé.
- **H10** : `make float-audit` retrouve `research_icon.c | ie_to_int | x` inchangé.
- **H11** : sous ASan+UBSan, rien de nouveau : la vue est sur la pile et chaque membre est posé avant lecture (R3).
- **H12** : `test_pico8_ie` compile et passe sans diff : `KE_VIEW_HASH`, `DRAW_HASH`, `PLAYTHROUGH_HASH` et `test_ke_view_is_a_struct` inchangés.
- **H13** : `pico8_oj(pico8_value_ref(NULL))` rend vrai (cart `not px`), comme aujourd'hui pour `qh`/`os`.
- **H14** : la mutation 5 (`STRING` NULL passé à `pico8_oj`) fait avorter ou change `kh` pour `qh`/`os`. Les deux sont rouges.
- **H15** : 1b.10, commitée, ne touche aucun des trois fichiers de cette étape.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne fait aucun accès direct à un membre de la vue (seulement `view.header` via `&view`). Elle lit par le **lecteur brut** (`pico8_ie_ke_view_struct_class()->field`, avec un témoin) et par `pico8_entity_get_field`. Elle compile sur les deux formes.
- **(G) Où : `test_pico8_entity_kind`, pas `test_pico8_ie`.** `test_pico8_ie` est lent (`UNIT_TESTS_SLOW`). Y caractériser coûterait un rejeu complet à chaque lancement, soit huit lancements (ancien code, nouveau code, six mutations), ce que le budget de 20 minutes ne permet pas. `test_pico8_entity_kind` est rapide, lie déjà `ENGINE_SRCS`, lit déjà `rr`/`s`/`me`/`od` sur les neuf genres (`test_kind_fields_and_build`), et appartient à `CENSUS_UNIT_TESTS` : un futur recensement y verra la lecture par nom de la vue. Rien n'est ajouté au Makefile. On ajoute seulement `#include "../src/pico8_research_icon.h"` (et `pico8_ui.h` pour `pico8_je`).
- **Caractérisation, figée sur l'ancien code** : `test_ie_ke_view_by_name`, appelée en dernier dans `main`, après `test_kind_reader_is_the_module_default`. Les icônes sont huit `ie` neufs : un par genre de `gx` (`kind_of(0..6)`, overrides `{ke = REF genre}`), plus `bare = pico8_ie_new(NULL)`. `pm`/`ga` sont exclus, car aucun `ie` ne les tient. Le témoin est un fixe `0x1234` posé dans `out` avant chaque lecture.
  1. **Lecture brute** : pour chaque icône, `view = pico8_ie_ke_view(icône)`, puis on lit `rr`, `s`, `me`, `od` et les intrus `""`, `"x"`, `"et"`, `"ke"`, `"header"`, `"has_s"`, `"has_od"`. Le lecteur brut rend 1 avec un genre de la ligne, ou 0 avec `out` égal au témoin. Il ne rend **jamais** 1 avec `REF NULL`. `pico8_entity_get_field(&view, …)` donne le même retour et la même valeur. On plie `(icône, nom, retour, genre, valeur : fixe, ou contenu de la chaîne)` dans une empreinte nouvelle, **`RAW_PRESENCE_HASH`**. On compte aussi **`ABSENT_COUNT`** (lectures à 0 sur les quatre membres, attendu 2 + 4 = 6 : `rr` de `qh`/`os`, et les quatre de `bare`).
  2. **Identité des pointeurs** : pour les sept genres, `rr` (quand il est présent) et `me` lus par nom sont, **par adresse**, la `.as.string` que rend `kind_of(i)->field(name)`. Deux vues successives du même `ie` rendent les mêmes adresses.
  3. **Comportement** : `je.ra` est posé à `v(64,64)` (comme `test_ke_view`). Puis, pour chaque icône : `pico8_ie_hb`, puis `kh` lu par nom (état 0/1/2/3, comme `bool_state`). Pour chaque icône révélée : `pico8_ie_kj(&gfx, &mem, icône, v(0,0))` écran vidé, puis de nouveau avec `m_` = vrai (par `pico8_entity_set_field`, l'écrivain d'`ie`), et l'empreinte du framebuffer à chaque fois. Tout est plié dans **`BEHAVIOR_HASH`**. Cela exerce `hb` (`rr`), `kj` (`s`) et `dl` (`me`, `od` par nom).

  Les trois valeurs sont relevées sur l'arbre de départ et écrites en `#define` avant le typage. La caractérisation est lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite. Le diff de `test_pico8_entity_kind.c` est un pur ajout.
- **Tests existants qui figent déjà beaucoup**, inchangés : `test_kind_fields_and_build` (`KIND_HASH`) ; dans `test_pico8_ie` (lent) : `test_ke_view_is_a_struct` (valeurs, absences, `must_abort(write_od)`), `test_ke_view` (`KE_VIEW_HASH`), `test_draw` (`DRAW_HASH`) et les rejeux.
- **Tests de régression (§6)** : aucun ne lit la vue. Tous ont un diff vide, et il n'y a pas de preuve rouge à refaire.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune lancée par `make tmp/test_pico8_entity_kind && ./tmp/test_pico8_entity_kind` (quelques secondes). Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou. Les rougeurs attendues dans `test_pico8_ie` sont notées, mais pas lancées par mutation (budget).
  1. (§7-1) *indicateur forcé vrai sur un membre à nil observé* : la branche `od` du lecteur ignore `has_od`. `bare` lit `od` `FIXED` 0. Attendu rouge : caractérisation 1 (`RAW_PRESENCE_HASH`, `ABSENT_COUNT`). Hors budget : `test_ke_view_is_a_struct`.
  2. (§7-2) *« présent 0 » là où c'était absent* : le bloc nil de R3 pose `has_s = 1`. `bare` lit `s` `FIXED` 0. Attendu rouge : caractérisation 1. Hors budget : `test_ke_view_is_a_struct`. **Trou justifié** : la même mutation dans la branche « absent » de `ie_ke_take_fixed` est invisible. Aucun des sept genres ne manque de `s` ou d'`od` (H4), et l'aide n'est pas appelée sans genre. La branche est donc un code défensif de R5, sans entrée qui l'atteigne.
  3. (§7-3) *init qui saute un membre typé* : le constructeur de vue n'appelle pas `ie_ke_take_fixed` pour `od` (`has_od` reste 0). Attendu rouge : caractérisation 1 et 3 (`dl` affiche `""` au lieu de `"15$"`). Hors budget : `test_ke_view`, `test_ke_view_is_a_struct`, et un rejeu si H8 tombe.
  4. Propre à l'étape, *chaîne copiée au lieu d'empruntée* : `ie_ke_take_string` pose `strdup(value.as.string)`. Les empreintes par valeur ne bougent pas. Attendu rouge : **caractérisation 2 seule**. C'est ce qui justifie ce cas.
  5. Propre à l'étape, *`rr` NULL lu comme présent dans `hb`* : `pico8_oj(pico8_value_string(rr))`, sans le test de NULL. Attendu rouge : caractérisation 3 (`kh` de `qh`/`os`, ou abandon, H14). Hors budget : les rejeux et `test_ke_view`.
  - **Mutation équivalente, non retenue** : dans `kj`, lire `view.s` sans `has_s`. R3 pose `s = 0` avec `has_s = 0`, et le sprite 0 est déjà la valeur de repli. Le mutant est donc équivalent. Le noter dans le commit.
  - **Trou justifié** : les `assert` « champ sans règle » des deux aides ne sont atteints par aucune entrée (H4). Un `must_abort` qui les viserait demanderait un genre factice, que l'ancien code lirait sans avorter : il ne peut donc pas faire partie de la caractérisation figée. (G) Pas de `must_abort` nouveau, comme en 1b.5, 1b.7, 1b.9 et 1b.10. Le commit note la mutation « `assert` retiré de `ie_ke_take_string` » et l'absence de rouge.
- **`gcov`** : `test_pico8_entity_kind` en `--coverage -O0` (pas de fils forkés dans ce TU, donc pas de piège `__gcov_dump` à poser), puis `gcov` sur `pico8_research_icon.c`. Chaque ligne ajoutée ou modifiée doit être exécutée :
  - les quatre branches du lecteur, présent comme absent ;
  - le bloc nil de R3, les deux chemins genre présent et genre absent ;
  - les deux aides, avec la branche « nom absent » (`rr` de `qh`/`os`) et chaque genre accepté ;
  - `hb` (`rr` NULL et non NULL) ;
  - `kj` (`has_s`).

  Trous attendus et justifiés : la branche d'échec des `assert` R5, et la branche « absent » de `ie_ke_take_fixed` (voir la mutation 2). Aucun autre trou n'est attendu. Le rapport va dans le commit.
- **Prior art** : `test_kind_fields_and_build` (témoin `0x5A5A`, même TU) ; `test_trade_members_by_name` (1b.5, lecteur brut et témoin `0x1234`) ; `test_hp_members_by_name` (1b.9, identité par adresse) ; `test_lq_members_by_name` (1b.10) ; `test_ke_view`/`draw_alone` de `test_pico8_ie` (empreinte du framebuffer, `je.ra` à `v(64,64)`).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `make tmp/test_pico8_entity_kind` puis lancement : quelques secondes, soit huit lancements (ancien code, nouveau code, cinq mutations, `gcov`) en 2 à 3 min ;
- `make tmp/test_pico8_ie` (**compilation seule**, tôt, pour voir le TU lent compiler sur le nouveau typedef) : quelques secondes ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min (c'est lui qui lance `test_pico8_ie`) ;
- `make sanitize-net` : ~4 min seul, le double sous charge ; on ne le lance **jamais** en même temps que `e2e` ;
- `make value-members` : quelques secondes.

Caractérisation, mutations et `gcov` passent par le binaire seul, jamais par `make test`.

**TU de `UNIT_TESTS_SLOW` touché : `test_pico8_ie`**, avec un diff vide. Il est compilé seul tôt, puis lancé une seule fois, par `make e2e`. Aucun autre TU lent ne nomme la vue.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` sans erreur de liste C1, et les `UNIT_TESTS_SLOW` qui passent (`test_pico8_ie` compris) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 213`**, sans aucune ligne `pico8_ie_ke_t`.

`make class-coverage-net` **ne s'applique pas** (§7) : aucun constructeur ne change. `pico8_ie_ke_view` construit une vue sans classe, qui n'appelle pas `pico8_class_coverage_record` et n'est pas dans `KNOWN[]` (1a.15). Le constructeur d'`ie` (`pico8_ie_struct_new`) et son `pico8_class_coverage_record("ie")` ne sont pas touchés. Si le diff touche l'un des trois, la cible redevient obligatoire, avec une sortie identique à `tools/class_coverage_net_expected.txt` (`ie  133  15/15`). `make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si :
  - une aide R5 avorte dans un rejeu ou un TU (H4) ;
  - un membre est ajouté à la vue, sauf `has_s` et `has_od` ;
  - un `set_field`, une méthode de descripteur ou un écrivain de la vue est ajouté ;
  - le lecteur accepte un autre nom que les quatre membres.
- `FAILED:` si un type diffère de la Solution, si un indicateur n'est pas un `int` déclaré juste après son membre, ou si `rr`/`me` ont un indicateur.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un membre nil ou un nom inconnu, lit `s` ou `od` « présent 0 » sur la vue d'un `ie` sans `ke`, ou ne rend pas, pour `rr`/`me`, le pointeur que rend `kind->field`.
- `FAILED:` si une chaîne de la vue est dupliquée ou libérée, si la vue cesse d'être rendue par valeur, ou si une allocation est ajoutée.
- `FAILED:` si `ie_self_ke_kind`, le membre `ie.ke`, la table `members[]` de `pico8_ie_field`, `pico8_ie_set_field`, `pico8_ie_struct_new` ou `pico8_dl` changent (1b.20 ; site par nom).
- `FAILED:` si la signature de `pico8_ie_ke_view` ou de `pico8_ie_ke_view_struct_class` change, ou si le message d'un `assert` existant change (`"ie.ke holds a kind, not an entity or a table"`, `"pico8_research_icon_init() must run before ie:hb writes ie.gy"`, `"ie: unlike self.ke, self.jb holds a built jb struct"`).
- `FAILED:` si une preuve de la section « Preuves » n'est pas vide, ou si la preuve `pico8_value_t \*` garde une autre ligne que `:295` (lecteur d'`ie`).
- `FAILED:` si un extrait C1 de `tools/float_audit_c1.txt`, ou ce fichier, changent.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation, ses `#define` et ses deux `#include` à `test_pico8_entity_kind.c`. Également si `test_pico8_ie.c` a un diff, si une lecture par nom change, ou si une valeur figée existante change (`KIND_HASH`, les `*_HASH` de `test_pico8_ie`).
- `FAILED:` si un test de régression de bug (§6) a un diff non vide.
- `FAILED:` si un site par nom de `src/` change ou est remplacé par un accès direct.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 213, liste encore `pico8_ie_ke_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si le diff touche un constructeur de classe, `pico8_class_coverage_record` ou `KNOWN[]` sans une ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit un membre de la vue directement, ou si ses valeurs (`RAW_PRESENCE_HASH`, `ABSENT_COUNT`, `BEHAVIOR_HASH`) changent après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 5 mutations, chacune avec son test ou la justification de son trou, plus la note sur le mutant équivalent de `kj`, la branche « absent » de `ie_ke_take_fixed` et les `assert` R5 non atteints.
- `FAILED:` s'il manque la note « Découvert à 1b.11 », ou si elle omet la lecture de R2 sur les TU hors recensement, la forme R5 dont la source est un lecteur de chaîne, ou le constat « R5 écrivain toujours vacante ».
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- `ie` et ses huit membres, dont `ie.ke` (`ref:autre` vers le genre), sa table `members[]` et son écrivain `m_` : 1b.20.
- `pico8_entity_kind_t` et la lecture de chaîne des genres (`pico8_<genre>_default`).
- `pico8_dl` et les autres lecteurs par nom de `me`/`od`/`et`.
- Donner une forme à la moitié écrivain de R5 (1b.13).
- Ajouter `test_pico8_ie` au recensement, ou régénérer `tools/member_census.txt` (R8).
- Remplacer une lecture par nom par un accès direct.
- Les fichiers d'événements et `FRAMES_EVENTS`.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `research_icon.c/.h` seuls ; `probe.c` intouché ;
  - tests : caractérisation dans `test_pico8_entity_kind` (rapide, au recensement), pas dans `test_pico8_ie` (lent) ; pas de nouveau TU, pas de changement du Makefile ;
  - un commit, sans découpage de repli ;
  - `has_s` sur preuve `lu` d'un TU lent, contre la lettre de sa ligne (lecture nouvelle de R2) ;
  - `rr`/`me` empruntés à la spec du genre ;
  - recopie depuis `kind->field` par deux aides R5 par genre (forme 1b.9, source différente, pas de lecture nouvelle de R5) ;
  - aucun écrivain, donc R5 écrivain vacante (confirmée) ;
  - R7 vacante (lecteur qui copie, 1b.6) ;
  - aucun emballage public ;
  - aucun `must_abort` nouveau pour les `assert` R5 ;
  - `class-coverage-net` non lancé (aucun constructeur de classe touché).
- **Note « Découvert à 1b.11 »**, à écrire sur la ligne 1b.11 de `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante :
  - « **lecture nouvelle de R2** : `CENSUS_UNIT_TESTS` exclut `UNIT_TESTS_SLOW`, donc une lecture par nom faite seulement par un TU lent (ici `test_pico8_ie`, `test_ke_view_is_a_struct`) manque à la table ; citée par le PRD, elle compte comme `l`, et le nil qu'elle lit comme `lu` (`ie_ke.s` : ligne `-`, `has_s` donné). À vérifier aux étapes qui touchent les structs de `test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` » ;
  - « R5, forme de 1b.9 avec une autre source : un lecteur de chaîne par nom (`kind->field`) au lieu d'une table intermédiaire, deux aides par genre ; le bloc nil de R3 reste le seul à poser nil quand la source manque » ;
  - « R7 vacante (lecteur qui copie, 1b.6) ; **R5 écrivain toujours vacante** (`ie_ke.set_field` NULL, `must_abort(write_od)`) ; première candidate : 1b.13 ».

  Le reste va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le compte 217 → 213 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov` ;
  - les 5 mutations, le mutant équivalent et les trous justifiés ;
  - les valeurs relevées de la caractérisation ;
  - les réponses à H1 à H15 ;
  - l'empreinte remesurée (réelle 4 (3, 1), contre 26 à la ligne, homonymes).
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
