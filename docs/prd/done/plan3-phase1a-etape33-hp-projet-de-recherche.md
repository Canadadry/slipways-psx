---
title: "Plan 3, étape 1a.33 : `hp` (le projet de recherche) devient une struct racine ; les 8 clés `ri[p]` de `ho` énumérées en membres, `mr`/`gp`/`fk` en état statique de la classe, `lq` servi par le lecteur"
description: "`hp` est construite 510 fois sur le filet (15/15 rejeux), 34 par chargement, toujours par `pico8_research_build_gb`. Elle passe en struct `pico8_hp_t` : en-tête, `mr`/`gp`/`fk` (défauts, lus sur la classe par l'état statique), les 8 noms de `hp.mr` écrits par `ho` (`px`, `lr`, `fj`, `ja`, `i`, `o`, `il`, `dm`), les 5 clés nommées de `fd` (`rr`, `ii`, `cw`, `go`, `rn`) et `lq`. Son descripteur n'a ni méthode ni écrivain ; son lecteur rend le membre `lq` s'il est posé, sinon la méthode `lq` de la classe, comme le cart. Les fonctions de `hp` et leurs consommateurs (`jo:pq`/`lz`/`hu`/`fe`, `bj`, `bb`) prennent le type struct. Seul `rich_planet_exports` change, dans son cast."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.33 — `hp` : racine (`research.c:75`), `mr`/`gp`/`fk` lus sur la classe, surcharges `ri[p]`, `p` ∈ `hp.mr` (8 clés, `collect.c:215`).

Pas de « Tests : » dans la ligne. Un seul test de régression passe une `hp` à une fonction du jeu : `rich_planet_exports`, par un cast (voir plus bas). Il change de forme, pas d'intention.

Les numéros de ligne ont dérivé d'une ligne. À HEAD d008307 :
- **`pico8_research.c`** :
  - `g_hp_class = pico8_class_declare(NULL, spec)` à `:75-78`, avec `mr=o("px","lr","fj","ja","i","o","il","dm")`, `gp=o("o","dm")`, `fk=o(f,2)` ; nom de couverture `:79` ; `pico8_class_set_pq` `:80` ; installation de `lq` dans `g_hp_class->defaults` `:81` ;
  - `pico8_hp_new` `:98` (`pico8_class_instantiate`) ;
  - `pico8_hp_pq` `:125-189` :
    - `mr` et `gp` lus dans `g_hp_class->defaults` (`:140`, `:144`, pas `:139,143`) ;
    - `pico8_ho_apply(scratch_rc, mr, &gp_list, pico8_instance_fields(self))` `:151` ;
    - `gb[px]=self` `:154-157` ;
    - `lq` écrit si `ii` (`:165-178`) ;
    - `i` réécrit par `fi` (`:180-186`) ;
  - `pico8_research_free_gb_instances` `:50` (`pico8_instance_free`) ;
  - `pico8_research_build_gb` `:191` construit chaque `hp` (`:212`) après `ri.rr=rr` (`:210`) ;
  - `pico8_hp_po` `:238` (lit `ja` `:247`, `il` `:252`) ; `pico8_hp_od` `:267` (`fj` `:274`, `il` `:277`, `ii` `:281`).
- **`pico8_collect.c:215`** : `pico8_ho_apply` écrit `ri[p]` pour chaque `p` de `mr`. C'est la clé construite à l'exécution de la ligne du plan.
- **`pico8_research_display.c`** :
  - `pico8_hp_jb_join` (`i` `:75`) ;
  - `pico8_hp_jb` (`ii` `:124`, `o` `:134`) ;
  - `pico8_hp_lq` `:278` : garde `pico8_hp_class()` `:290`, lit `i` `:294` et `o` `:297`, `fk` lu dans `pico8_hp_class()->defaults` `:313`, lit `px` `:316` ;
  - `pico8_bj_predicate` : `rr` `:345`, `lr` `:360`, sur chaque élément de `gb`.
  Toutes ces lectures passent par `pico8_instance_get_field`. Les `pico8_instance_fields(l)` de `:312-317` portent sur l'instance `lq` rendue par `pico8_jo_ex`, pas sur l'`hp` (1a.34).
- **`pico8_planet.c`** :
  - `pico8_jo_pq` caste `gb[self.hp]` en `pico8_instance_t *` pour `lz` (`:407`) ;
  - `pico8_jo_lz(pico8_jo_t *, pico8_instance_t *)` `:421` : pose `hn` = l'`hp`, lit `i` `:436` et `o` `:445` ;
  - `pico8_jo_hu(pico8_jo_t *, pico8_instance_t *)` `:484` : lit `cw` `:496` (puis `kv[hp.cw]` par `lookup_named`), `go` `:520`, `rn` `:548`.
- **`pico8_planet_display.c`** :
  - `pico8_jo_fe_step` prend chaque `b` de `bj` en `pico8_instance_t *` (`:407`), appelle `pico8_hp_od` `:410`, `pico8_mv(b, "lq", …)` `:414`, `pico8_hp_jb` `:442` ;
  - `b` est capturé sous `_jo_fe_b` (`:438`) et repris par le `my` de la ligne (`:384`, puis `pico8_jo_hu` `:393`).
- **`pico8_economy_tiers.c`** : `bb` reprend `hn` par `field_ref_instance` (`:207`, `:225`), lit `dm` par `pico8_instance_get_field` (`:209`) et appelle `pico8_hp_po(hn)` (`:227`).
- **`pico8_data_init.c:93-94`** : `pico8_research_init` puis `pico8_research_build_gb(g_fd)`.
- **Tests** : 11 fichiers lisent `pico8_gb()` (`fr`, `mx`, `os`, `ga`, `pm`, `nr`, `jo`, `hj`, `nl`, `qh`, `rich_planet_exports`). Tous prennent une entrée de `gb` en `void *`/REF (comparaison à `hn`, `lookup` par `pico8_entity_get_field`, passage à `pico8_jo_lz`/`hu`), sauf `rich_planet_exports:140`, qui caste en `pico8_instance_t *` pour `pico8_jo_lz`.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` […] ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Clés construites à l'exécution** : […] Tout autre site de clé dynamique trouvé est énuméré de même, ou déclenche un arrêt (§7) si l'ensemble des clés n'est pas fini.
>
> **Défauts mutés sur la classe** : […] deviennent un état statique partagé de la classe, jamais un champ d'instance.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […] jusqu'à la dernière étape de 1a. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct […].
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe […] ou dans la table de méthodes (installation de méthode) […].
>
> **Ordre** : […] Les racines sans parent dans n'importe quel ordre.

`hp` est une **vraie racine sans fille** (écarts relevés du §2). Elle ne descend pas de `ke` : pas de `lt`, jamais dans `hv`/`cv`/`cg`. Ses précédents sont :
- `trade` (1a.32) : racine à lecteur sans écrivain, fonctions retypées ;
- `ui` (1a.31) et `pj` (1a.29) : défauts lus sur la classe, REF prise dans `cls->defaults` à l'init et rendue par un accesseur ;
- `jb` (1a.1) : constructeur posé par `pico8_class_set_struct_new`.

§7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` : `hp  510  15/15`, toutes les autres lignes inchangées ;
- **preuve des tests par `gcov` et 2 à 3 mutations** ;
- **note « Découvert à 1a.33 »**, réduite à ce qui change une étape suivante ;
- un nouveau test dont le rejeu dépasse 30 s va dans `UNIT_TESTS_SLOW` ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.32 ont posé, réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (`methods` indexé par l'enum, `field`, `set_field`).
  - `pico8_entity_get_field` cherche d'abord le nom dans l'enum. S'il y est et que l'entrée du descripteur est posée, il la rend ; sinon il appelle le lecteur. Un membre nil est absent.
  - `set_field` NULL : une écriture générique avorte.
- Racine convertie : l'en-tête en premier membre. Le constructeur `static` est posé par `pico8_class_set_struct_new` et appelé aussi par `pico8_<classe>_new`. Il enregistre la couverture et asserte (message porteur) sur toute clé non mesurée.
- Défaut lu sur la classe : REF prise une fois dans `cls->defaults` à l'init, jamais re-parsée, rendue par un accesseur (`pico8_ui_hw`, `pico8_pj_gs`). Le membre porte la même REF.
- Fonctions propres d'une classe convertie retypées sur la struct quand leur `self` est toujours de cette classe. Elles assertent le descripteur sur un pointeur non NULL (`trade`).
- Champ de fonction servi par le lecteur (`js.hb`, 1a.16) : `pico8_mv` appelle une valeur `FUNCTION` rendue par `pico8_entity_get_field`, qu'elle vienne d'une entrée du descripteur ou du lecteur.

### Ce que fait le cart

`pico8/lua.lua` :

```
:17-23     ho(rc,mr,gp) = function(ri) for i,p in pairs(mr) do ri[p]=gu(gp,p) and r[ri[i]] or ri[i] end ; rc[ri.px]=ri end
:86-88     kv:jr(rj) : ob=setmetatable(mw(ob), rj.ns)       -- les surcharges sont copiées
:1399-1402 hp=kv:jr([[ mr=o("px","lr","fj","ja","i","o","il","dm"), gp=o("o","dm"), fk=o(f,2), ]])
:1403-1408 hp:pq() : ho(gb,hp.mr,hp.gp)(self) ; if self.ii then self.lq={self.px,32,{self.ii,8}} end
                     self.i=fi(self.i or "",function(c) return {qe=r[c]} end)
:1409-1413 hp:lq() : l=jo.ex(self.i,self.o,max(self:po()[1],1),0,1) ; add(l,hp.fk) ; add(l,self.px)
:1414-1421 hp:jb() : lit self.ii, self.i, self.o
:1422-1425 hp:po() : hc[self.ja], self.il
:1426-1433 hp:od() : self.fj, self.il, self.ii
:1435-1440 gb,rr={} ; for ri in all(fd) : chaîne → rr=ri ; sinon ri.rr=rr ; add(gb,hp(ri))
:403-405   jo:pq()  : if self.hp then self:lz(gb[self.hp]) end
:436-441   jo:fe()  : pz(self,nv(bj(self),function(b) … lq=mv(b,"lq"), h_=b:od(), my=…self:hu(b), jb=b:jb() end))
:451-460   jo:hu(hp): hp.cw → kv[hp.cw]{p,ra,id} ; hp.go → self.nk=df[hp.go] ; sinon self:lz(hp) ; rn(hp.rn or 55)
:461-464   jo:lz(hp): self.hn,self.ep=hp ; self.jy=nv(hp.i,mw) ; self.kw={qe=hp.o,mu=0}
:1391      bj(jo)   : jp(gb, b.rr connu et jo.nk.px ∈ b.lr)
bb()                : hn.dm si mi==1 ; hn:po()
```

Points décisifs :
- **Une voie de construction** : `hp(ri)` dans la boucle de `gb`, avec le record de `fd` (positionnels, plus quelques clés nommées), après `ri.rr=rr`.
- **La clé dynamique `ri[p]`** parcourt `hp.mr`, une table constante de 8 chaînes, jamais modifiée. **L'ensemble est fini** : `px`, `lr`, `fj`, `ja`, `i`, `o`, `il`, `dm`. Pour `p` ∈ `gp` (`o`, `dm`), la valeur est `r[ri[i]]`, sinon le positionnel brut. Un positionnel absent donne `ri[p]=nil`, sans effet.
- **`mr`, `gp`, `fk` sont lus sur la classe** (`hp.mr`, `hp.gp`, `hp.fk`), jamais sur `self`. Aucun n'est réaffecté ni modifié en place.
- **`lq` est une collision du cart** : `hp:lq` est une méthode de la classe, mais `hp:pq` écrit `self.lq` (une table) sur les projets à `ii`. `mv(b,"lq")` rend donc la table pour ceux-là, et appelle la méthode pour les autres. L'instance passe avant la classe.
- **Pas d'écriture après `pq`** : aucun site ne pose un champ sur une `hp` construite.

### Emprise réelle de `hp` (mesurée)

**Méthode.**
- Grep de `src/`, `tests/`, `tools/` et du `Makefile` à d008307.
- **Sonde** sur une copie `git archive HEAD` jetable du scratchpad, supprimée après usage. Un en-tête forcé (`-include`) remplace 6 fonctions par des macros qui portent `__FILE__`/`__LINE__` :
  - `pico8_instance_get_field`, `pico8_instance_fields`, `pico8_entity_get_field`, `pico8_entity_set_field`, `pico8_mv`, `pico8_class_instantiate` ;
  - `pico8_class.c` et `pico8_entity.c` gardent les appels internes non instrumentés.

  Elle journalise, pour toute instance de la classe `hp` (pointeur de classe posé par `pico8_research_init`) :
  - chaque construction : clés reçues, nil marqué ; clés après `pq` ;
  - chaque lecture : nom ; source (propre, classe, absent) ; genre de la valeur ;
  - chaque `pico8_instance_fields`, `set_field` et `mv`.
- Rejoués : les 17 `tests/fixtures/*events.json` et `events_recorded_1790198294.json` (`--from-title` pour les 4 enregistrements), **18 rejeux en sortie 0**.
- **Fil piège** : `make unit` puis les 8 cibles de `UNIT_TESTS_SLOW`, même copie, sortie 0. Le journal est écrit par `write` en `O_APPEND` et préfixé du nom du binaire.

**Constructions** :
- **filet** : 34 par chargement du cart, 510 sur 15/15 rejeux, égal à la référence. Toujours par `pico8_hp_new`, appelé par `pico8_research_build_gb` (`research.c:212`), jamais ailleurs. Les TU aussi : 34 par chargement dans 45 binaires.
- **Clés reçues**, toujours parmi les mêmes 8 formes (mêmes 34 records à chaque chargement) :
  - positionnels `[1]`…`[8]` : `[1]`-`[3]` toujours, `[4]`-`[6]` sur les 30 records sans `ii`, `[7]`/`[8]` sur certains. `[7]` peut valoir `false`.
  - nommées : `rr` toujours, **nil sur 20 des 34** (records avant la première chaîne de catégorie de `fd`) ; `ii` sur 4, avec `go` (2) ou `cw`+`rn` (2).
  - **Aucune autre clé**, ni sur le filet ni dans les TU.
- **Clés après `pq`** : les positionnels, plus les clés nommées reçues, plus les noms de `mr` dont le positionnel existe, plus `lq` (les 4 à `ii`) et `i` (tous ; liste de `{qe=…}`, vide pour les 4 à `ii`).

**Lectures sur une `hp`** (rejeux et TU, mêmes sites) :

| Nom | Sites | Source |
|---|---|---|
| `px` | `research.c:172`, `research_display.c:316` | propre |
| `lr` | `research_display.c:360` (`bj`) | propre |
| `fj` | `research.c:274` (`od`) | propre |
| `ja` | `research.c:247` (`po`) | propre |
| `i` | `research.c:180` (`pq`, chaîne ou absent) ; `planet.c:436` (`lz`) ; `research_display.c:75,294` | propre |
| `o` | `planet.c:445` ; `research_display.c:134,297` | propre |
| `il` | `research.c:252,277` | propre ou absent |
| `dm` | `economy_tiers.c:209` (`bb`, sur `hn`) | propre ou absent |
| `rr` | `research_display.c:345` (`bj`) | propre, nil sur 20 records |
| `ii` | `research.c:165,281` ; `research_display.c:124` | propre ou absent |
| `cw` | `planet.c:496` ; `test_pico8_ga.c:105` (`lookup`, `pico8_entity_get_field`) | propre ou absent |
| `go` | `planet.c:520` | propre ou absent ; présent seulement dans `session_events` (2) et les TU `jo`, `nr`, `pm`, `qh` (2 chacun) |
| `rn` | `planet.c:548` | propre ou absent |
| `lq` | `pico8_mv` à `planet_display.c:414` (`jo:fe`) | propre (4 records à `ii`) ou **classe** (méthode) ; 478 appels sur le filet, 360 dans `make unit`, 1 932 dans les TU lentes |

- **Aucun autre nom n'est lu**, dont `mr`, `gp`, `fk` sur l'instance.
- **Aucune lecture par la classe** hors de `lq` par `mv`.

**`pico8_instance_fields` sur une `hp`** : seulement dans `pq` (`research.c:151,154,177,186`). **Aucun positionnel n'est lu** après `pq`, ni dans le cart (seul `ho` lit `ri[i]`), ni dans le port.

**Champs écrits après construction** : **aucun** (0 `pico8_entity_set_field`, 0 `pico8_instance_fields` hors `pq`, rejeux et TU).

**Accès `->defaults`** sur la classe `hp` : `research.c:81` (installation de `lq`), `:140` (`mr`), `:144` (`gp`), `research_display.c:313` (`fk`). Aucun dans les tests.

**Tests qui touchent une `hp`** : voir Problem Statement. Aucun ne lit un positionnel, `mr`, `gp` ou `fk`, et aucun n'appelle `pico8_hp_class()`.

**Outils** :
- `tools/float_audit_c1.txt` ne nomme aucune expression de `pico8_research.c`. Ses lignes `pico8_jo_pq | hp_val.as.fixed` (le **nombre** `jo.hp`, pas la classe) et `pico8_jo_ex | loop_bound` ne changent pas de texte.
- Les `PICO8_REF_AS_LIST_CONST` sur `i` (`research_display.c`, `planet.c`) sont gardés.

**→ Membres de `hp`**, dans cet ordre :
- `mr`, `gp`, `fk` : défauts de la classe, même REF que l'état statique ;
- `px`, `lr`, `fj`, `ja`, `i`, `o`, `il`, `dm` : les 8 clés de `ri[p]`, ordre de `mr` ;
- `rr`, `ii`, `cw`, `go`, `rn` : clés nommées de `fd` ;
- `lq` : écrit par `pq`.

**→ Méthodes par le descripteur** : aucune. **Écrivain** : aucun. **Lecteur** : les 17 membres (nil = absent), avec une règle pour `lq` : le membre s'il n'est pas nil, sinon la méthode `lq` de la classe (valeur `FUNCTION`).

## Solution

1. **Enum : inchangée** (voir décisions : `lq`).
2. **Struct `hp`**, publique dans `pico8_research.h` : `pico8_struct_header_t` en premier membre, puis les 17 membres ci-dessus.
3. **État statique de la classe** : `mr`, `gp`, `fk` sont pris une fois dans `g_hp_class->defaults` à `pico8_research_init`, après la déclaration. Ce sont les REF de la classe, jamais re-parsées. Un assert porteur exige qu'elles soient présentes et des tables. `mr` et `gp` restent `static` dans le module. `fk` est rendu par un accesseur public (`pico8_hp_fk()`) pour `pico8_hp_lq`. Remis à nil par `pico8_research_shutdown`.
4. **Constructeur** (`static`, posé par `pico8_class_set_struct_new` dans `pico8_research_init`, appelé directement par `pico8_hp_new`), dans l'ordre de `hp:pq` :
   1. alloue un `pico8_hp_t`, pose l'en-tête (tag STRUCT, descripteur de `hp`), met tous les membres à nil, puis `mr`/`gp`/`fk` aux REF de l'état statique ;
   2. **asserte** (message porteur, arrêt §7) que les surcharges ne portent que des positionnels 1 à 8 et les clés nommées `rr`, `ii`, `cw`, `go`, `rn` ;
   3. copie les surcharges dans une **table de travail** et y applique `pico8_ho_apply` tel quel (même `mr`, même liste `gp`, `rc` jetable), comme `pq` le fait aujourd'hui sur la table de l'instance ;
   4. lit dans la table de travail les 8 noms de `mr` et les 5 clés nommées, et les range dans leurs membres, **tels quels** (nil compris) ; libère la table de travail. Les positionnels ne sont pas gardés (voir décisions) ;
   5. enregistre `gb[px]=self` si `px` est une chaîne, avec l'assert existant sur `gb` ;
   6. si `ii` est vrai (même vérité qu'aujourd'hui), pose `lq` = `{px, 32, {ii, 8}}` (`px` omis s'il est absent, comme aujourd'hui) ;
   7. réécrit `i` par `fi(i or "")`, avec la même fabrique de `{qe=r[c]}` ;
   8. enregistre `"hp"` au crochet de couverture.

   Overrides NULL : `mr`/`gp`/`fk` posés, `i` = liste vide, tout le reste nil. Le cas est gardé pour la caractérisation.
5. **Descripteur de `hp`** (`static const`) : toutes les méthodes NULL. Le **lecteur** rend les 17 membres (nil = absent), et pour `lq` nil la valeur `FUNCTION` du dispatcher de `pico8_hp_lq` (celui qu'installait `research.c:81`). Rien d'autre. **Écrivain NULL**. Un accesseur public du descripteur (`pico8_hp_struct_class()`), comme `pico8_trade_struct_class()`.
6. **Fonctions de `hp` sur la struct** :
   - `pico8_hp_new` rend `pico8_hp_t *` ;
   - `pico8_hp_po`, `pico8_hp_od`, `pico8_hp_jb` prennent `const pico8_hp_t *`, `pico8_hp_lq` prend `pico8_hp_t *` et rend toujours `pico8_instance_t *` (l'instance `lq`, 1a.34). Mêmes gardes NULL, mêmes vérités, membres lus directement. `pico8_hp_lq` lit `fk` par l'accesseur au lieu de `pico8_hp_class()->defaults` ;
   - chacune asserte (message porteur) qu'un `self` non NULL est une struct au descripteur de `hp` ;
   - `pico8_research_free_gb_instances` libère chaque struct (`free`), et seulement elle, comme `pico8_instance_free` ne libérait que la table de l'instance.
7. **Consommateurs hors du module** :
   - `pico8_jo_lz` et `pico8_jo_hu` prennent `pico8_hp_t *`. Ils lisent `i`/`o` et `cw`/`go`/`rn` directement dans les membres, avec les mêmes vérités (`pico8_jo_truthy`, présence). `hn` reçoit la REF de la struct ;
   - `pico8_jo_pq` passe `target.as.ref` sans cast `pico8_instance_t *` ;
   - `pico8_jo_fe_step` et le `my` de la ligne typent `b` en `pico8_hp_t *`. `pico8_mv` reçoit `b` casté au seul appel, comme les autres structs ;
   - `pico8_bj_predicate` type `b` en `const pico8_hp_t *` et lit `rr`/`lr` dans les membres. Un `rr` nil se lit comme aujourd'hui (`pico8_oj(nil)`) ;
   - `bb` (`economy_tiers.c`) : `hn` typé `pico8_hp_t *`, `dm` lu dans le membre avec la même vérité, `pico8_hp_po(hn)` sans cast `pico8_instance_t *`.
8. **Classe `hp`** : gardée, enregistrée à la même place (emplacement `""` du registre `fz`), constructeur de struct posé. **Le `pq` et l'installation de `lq` sont retirés** de la classe. Plus aucun accès `->defaults` sur `hp` hors de l'init de l'état statique. `pico8_hp_class()` reste.
9. **Tests** :
   - nouveau `test_pico8_hp` ;
   - `rich_planet_exports` : seul le cast de `mine` change, `pico8_instance_t *` → `pico8_hp_t *` ;
   - les 10 autres tests qui lisent `gb` : diff vide.
   Voir Testing Decisions.
10. **Plan** : à la ligne 1a.33 de `docs/plan-3.md`, ajouter « Découvert à 1a.33 : … », réduit à ce qui change une étape suivante (§7). A priori :
    - les 8 clés de `ri[p]` en membres, les positionnels consommés par le constructeur ;
    - `lq` servi par le lecteur (membre, sinon la méthode de la classe), enum inchangée ;
    - `pico8_hp_fk()` ;
    - `pico8_jo_lz`/`hu` typés `pico8_hp_t *` ;
    - `pico8_hp_lq` rend encore une instance `lq` (1a.34).

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `hp` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `lq`, `hy`, `fm` et les instances sans classe à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `hp` mesurée (constructions et clés, noms lus et leur source, écritures, `->defaults`, positionnels), afin que chaque membre, l'absence d'écrivain et l'absence de méthode soient justifiés par une mesure.
3. En tant qu'autopilote, je veux que la clé dynamique `ri[p]` de `ho` soit énumérée en 8 membres nommés d'après `hp.mr`, afin que la règle « Clés construites à l'exécution » soit appliquée à un ensemble fini et prouvé.
4. En tant que lecteur du code, je veux une struct `hp` qui n'a que l'en-tête et les 17 membres, noms minifiés, afin que la traçabilité vers `pico8/lua.lua` tienne.
5. En tant que joueur, je veux que la liste des projets de `jo:fe` (nom, coût, icônes, infobulle) soit identique, afin que le panneau d'une planète ne change pas.
6. En tant que joueur, je veux que les projets à `ii` (récolte, géo-récolte) gardent leur ligne propre (`{px,32,{ii,8}}`) et que les autres gardent la ligne calculée par `hp:lq`, afin que la collision du cart entre champ et méthode `lq` soit préservée.
7. En tant que joueur, je veux qu'installer un projet produise le même effet (`cw` crée l'éclat ou la protoétoile, `go` change le type de planète, sinon `lz`), avec le même son (`rn`), afin que la partie ne change pas.
8. En tant que joueur, je veux que les niveaux et la production d'une planète (`bb` : `dm`, `po`) restent identiques, afin que le score ne bouge pas.
9. En tant que joueur, je veux que le coût d'un projet (`od` : `fj`, `il`, `ii`, nanomatériaux) et ses ressources requises (`jb`) restent identiques.
10. En tant que mainteneur, je veux que `mr`, `gp`, `fk` soient lus sur la classe par un état statique qui porte la REF de la classe, afin que le cart (`hp.mr`, `hp.gp`, `hp.fk`) soit suivi et que `->defaults` ne soit plus lu hors de l'init.
11. En tant que mainteneur, je veux que `pico8_ho_apply` reste inchangé, appliqué à une table de travail, afin que `df` et `nb`, qui le partagent, ne soient pas touchés.
12. En tant que mainteneur, je veux que le constructeur n'accepte que les positionnels 1 à 8 et `rr`/`ii`/`cw`/`go`/`rn`, et avorte sur toute autre clé, afin qu'un record jamais mesuré soit un arrêt (§7).
13. En tant que mainteneur, je veux qu'aucun écrivain ne soit posé, afin qu'une écriture générique jamais mesurée sur une `hp` soit un arrêt.
14. En tant que mainteneur, je veux que `pico8_mv(b,"lq")` rende la table pour un projet à `ii` et appelle `pico8_hp_lq` pour les autres, afin que `jo:fe` reste celui du cart.
15. En tant que mainteneur, je veux que `po`, `od`, `jb`, `lq` et `jo:lz`/`hu` prennent `pico8_hp_t *`, afin que le type dise ce que la mesure a prouvé.
16. En tant que mainteneur, je veux que ces fonctions assertent le descripteur de `hp` sur un pointeur non NULL, afin qu'une valeur de `hn` ou de `gb` qui ne serait pas un projet soit détectée au lieu d'être lue de travers.
17. En tant que mainteneur, je veux que plus aucun site hors de `pico8_class.c` ne passe une `hp` à `pico8_instance_get_field` ou `pico8_instance_fields`, afin que la forme struct ne soit jamais lue par la table d'une instance.
18. En tant que mainteneur, je veux que `gb` garde ses deux vues (positionnelle et `gb[px]`) sur les mêmes pointeurs, et que `shutdown` libère chaque projet une fois, afin que `jo:pq` (`gb[self.hp]`) et `bj` voient les mêmes projets.
19. En tant que gardien de la couverture, je veux que le constructeur enregistre `"hp"` une fois par projet, afin que `class-coverage-net` reste `hp  510  15/15`.
20. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne des références de couverture ni de `float_audit_c1.txt` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
21. En tant que vérificateur, je veux une caractérisation de `hp` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable. Cela vaut aussi pour ce que le filet voit peu ou pas : `go` hors de `session_events`, `dm`, `il` à `false`, `rr` nil, la ligne d'un projet à `ii`.
22. En tant que vérificateur, je veux un rapport `gcov` qui montre chaque ligne ajoutée ou modifiée exécutée, et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
23. En tant que gardien des tests de régression, je veux que `rich_planet_exports` ne change que son cast, avec la preuve rouge du §6, afin que son objet reste intact.
24. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C, un piège ou un `TROU-N` nu (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
25. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
26. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** :
  - le projet `hp` (`research`) : struct, état statique, constructeur, descripteur, lecteur ; `po`/`od` ; libération de `gb` ;
  - son affichage (`research_display`) : `jb`, `lq`, `bj` sur la struct ;
  - `jo:pq`/`lz`/`hu` (`planet.c`, `planet.h`), `jo:fe` (`planet_display.c`), `bb` (`economy_tiers.c`) : types et lecture directe des membres.

  Rien d'autre dans `src/`. Pas de changement dans `collect` (`ho`), `entity`, `pico8_struct.h` (enum), `class`, `data_init`, `cart.c`.
- **Les 8 clés de `ri[p]` sont des membres** (règle « Clés construites à l'exécution »). L'ensemble est `hp.mr`, une table constante de 8 chaînes, lue dans l'état statique. Il est fini et mesuré : aucun record ne produit d'autre nom. `pico8_ho_apply` reste générique, appliqué à une table de travail. Le passage de la table aux membres est une liste explicite des 8 noms, et un assert porteur vérifie qu'elle égale `mr` à l'init (même compte, mêmes chaînes, même ordre). Rejeté : réécrire `ho` pour les structs, parce que `df` et `nb` le partagent.
- **Positionnels non gardés** (G). Dans le cart comme dans le port, les `ri[1..8]` restent sur l'objet après `ho`. Mais aucun site ne les lit ensuite : dans le cart, seul `ho` lit `ri[i]` ; dans le port, seul `pq` passe la table (mesuré, `pico8_instance_fields` seulement dans `pq`). Ce sont les arguments de `ho`, pas des champs lus. Les garder demanderait 8 membres sans nom minifié. Le §5 garde les champs de la chaîne, et ces valeurs restent observables par les 8 noms (le brut pour les 6 hors `gp`, `r[brut]` pour `o`/`dm`). Critère d'arrêt : si un lecteur d'un positionnel d'`hp` est trouvé en cours d'étape, c'est l'arrêt « champ sans règle », pas un membre improvisé.
- **`mr`, `gp`, `fk` : état statique de la classe et membres** (G, précédents `ui.hw`, `pj.gs`/`i_`). Ce sont des défauts de la chaîne, donc des membres. Le cart les lit sur la classe : le code les lit dans l'état statique, qui porte la même REF que `cls->defaults`. Le lecteur les rend aussi, parce que l'instance les trouvait par sa classe. `gp` reste converti en liste à chaque construction, comme aujourd'hui.
- **`lq` servi par le lecteur, pas par l'enum** (G). `mv(b,"lq")` est bien un appel par nom. Mais `pico8_entity_get_field` consulte l'enum **avant** le lecteur : une entrée `LQ` au descripteur ferait passer la méthode avant le champ, et inverserait la collision du cart pour les 4 projets à `ii` (commentaire de `research.c:161-164`, « the cart's, not a porting slip »). Le lecteur reproduit donc la recherche du cart : membre `lq` s'il n'est pas nil, sinon la valeur `FUNCTION` du dispatcher de `hp:lq`. C'est le même mécanisme que `js.hb` (champ de fonction servi par le lecteur). `pico8_entity_get_field(hp, "lq")` rend alors exactement ce qu'il rend aujourd'hui (table ou fonction). Enum inchangée.
- **`rr` nil** (G). Aujourd'hui, 20 projets portent `rr` écrit à nil : c'est une clé présente (TROU-9), là où le cart ne crée pas de clé. Dans la struct, nil et absent ne font qu'un. Le seul lecteur (`bj`) traite déjà les deux pareil (`pico8_oj(nil)`), et aucune lecture de présence de `rr` n'existe (mesuré). Rien d'observable ne change, donc pas de `TROU` nouveau. La caractérisation lit la présence par un `lookup` qui confond nil et absent.
- **Clés acceptées** : positionnels 1 à 8 et `rr`, `ii`, `cw`, `go`, `rn`, chacune facultative. Rejeté : exiger `rr` ou les positionnels 1-3, qui sont toujours présents mais que la caractérisation « overrides NULL » veut pouvoir omettre.
- **Ordre du constructeur** : celui de `hp:pq`. D'abord `ho`, puis `gb[px]`, puis `lq`, puis `i`. `i` doit être réécrit **après** `ho`, qui y range la chaîne brute ; `lq` lit `px`, posé par `ho`.
- **Asserts d'entrée** de `po`/`od`/`jb`/`lq`, `jo:lz`/`hu` (G) : un pointeur non NULL est une struct au descripteur de `hp` (message porteur). Les gardes NULL restent. Le contrat d'en-tête de `pico8_research.h` (« a NULL instance argument is a safe no-op, and an absent or wrong-kind field reads as its zero/false default ») est relu : « instance » devient « projet ».
- **Lectures hors du module** (G) : là où la variable est typée projet (`hn` de `bb`, `b` de `bj`/`jo:fe`, `hp_instance` de `lz`/`hu`), lecture directe du membre avec la même vérité ; pas d'API générique, puisque tous ces sites ne voient que des projets (mesuré). `test_pico8_ga` continue de lire `cw` par `pico8_entity_get_field` (le lecteur répond).
- **Mémoire** : un projet n'est libéré que par `pico8_research_shutdown`, par sa vue positionnelle dans `gb`, jamais par `gb[px]`. Ses tables (`i`, `lq`) ne sont pas libérées, comme aujourd'hui. Les chaînes sont empruntées à `fd`. Contrat à écrire à la déclaration de la struct.
- **Commentaires** (CLAUDE.md) :
  - à ajouter : contrat de la struct (qui libère, emprunts) ; l'assert de clé ; l'assert `mr` = liste des 8 membres ; l'assert de descripteur ; la règle du lecteur pour `lq` (piège : une entrée d'enum l'inverserait) ;
  - à corriger ou retirer, rendus faux : l'en-tête de `pico8_research_init` (« wires "lq" as a PICO8_VALUE_FUNCTION on `hp`'s defaults ») ; ceux de `pico8_research_shutdown` et `pico8_hp_new` (« instance ») ; celui de `pq` (« store the instance, not its bare fields table ») ; le commentaire de la collision `lq`, à reporter au lecteur ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert. La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_hp` sur l'ancien code** (voir Testing Decisions), valeurs figées. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si un record de `fd` porte une clé hors de la liste mesurée : la mesure est fausse et doit être corrigée dans le PRD.
2. **`hp` converti** :
   - struct, état statique, constructeur, descripteur, lecteur, `set_struct_new`, retrait du `pq` et de l'installation de `lq` ;
   - `po`/`od`/`jb`/`lq`/`bj`, libération de `gb` ;
   - `jo:pq`/`lz`/`hu`, `jo:fe`, `bb` ;
   - le cast de `rich_planet_exports`.

   Toutes les caractérisations passent **sans changer leurs valeurs figées**. Les vérifications de forme s'ajoutent à `test_pico8_hp`.
3. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, preuve rouge de `rich_planet_exports`, `make class-coverage-net`, `make ref-audit`, `make float-audit`, ligne 1a.33 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : les valeurs rendues par `pico8_entity_get_field` sur un projet, par `po`/`od`/`jb`/`lq` et par `pico8_mv(b,"lq")`, les champs d'une planète après `lz`/`hu`, le contenu et l'identité de `gb`, les pixels, le compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_hp`** : lié comme `test_pico8_trade` (cart réel, `ENGINE_SRCS`), avec des dépendances Makefile sur les fixtures qu'il rejoue.
  - Il lit les projets **uniquement** par `pico8_entity_get_field`, `pico8_mv` et les fonctions publiques, en passant des `void *` : le même code compile avant et après, aux vérifications de forme près.
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()`, pour l'état de `rnd`.
  - Toute présence se lit par un `lookup` qui confond nil et absent.
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **`gb`** : 34 entrées positionnelles. Pour chacune, dans l'ordre : `px`, `lr`, `fj`, `ja`, `o`, `il` (genre compris : chaîne, `false`, absent), `dm`, `rr`, `ii`, `cw`, `go`, `rn`, et le contenu de `i` (compte, puis le `qe` de chaque élément). Pour `lq` : genre (table ou fonction), et pour une table, ses trois éléments. `mr`, `gp`, `fk` : REF identiques d'un projet à l'autre, contenus lus. Absents : `me`, `nu`, `fa`, `id`, `hp`. Chaque `gb[px]` est le même pointeur que l'entrée positionnelle de même `px`. Nombre d'entrées nommées de `gb`. Empreinte figée.
    2. **Fonctions** : pour chaque projet, `po` (identité de l'entrée de `hc`, ou NULL), `od`, `jb` (contenu, ou NULL pour les 4 à `ii`), `lq` (contenu de l'instance rendue : positionnels, dont `fk` et `px` en queue), `pico8_mv(b,"lq")` (table identique au membre pour les 4 à `ii` ; pour les autres, même contenu que `lq`). Deux passes : état de tech initial, puis après `oj` rendu vrai pour les `il` et `nanomaterials` par l'API du jeu (voir `test_pico8_tech_screen`). NULL → NULL/0.
    3. **Installation** : sur des planètes neuves d'une partie lancée comme le filet :
       - `pico8_jo_lz` avec un projet ordinaire (`hn` = le pointeur, `jy`, `kw.qe`, `ep` nil) ;
       - `pico8_jo_hu` avec un projet ordinaire, un projet à `go` (`nk` = `df[go]`) et un projet à `cw` (la planète construite par `kv[hp.cw]`, son rang dans `hv`) ;
       - `pico8_jo_hu` avec NULL.
       Empreinte figée.
    4. **Rejeux courts** (départ comme le filet, jusqu'à la dernière frame) :
       - `built_planet_panel_events.json` : `jo:fe` (8 `mv`), `lz`, `dm` ;
       - `geoharvest_burst_events.json` : `cw` par un vrai clic.

       À la dernière frame, pour chaque planète de `cg.jo` : le rang dans `gb` de son `hn`, ou -1. S'ajoute une empreinte d'écran finale. Figé.
  - **Forme** (ajoutée au morceau 2) :
    - un projet est `STRUCT` au descripteur de `hp` (`pico8_hp_struct_class()`) ; toutes les méthodes NULL ; `field` posé, `set_field` NULL ;
    - `pico8_entity_set_field(projet, "px", …)` avorte (dans un fils) ;
    - `pico8_hp_new` avec une clé `"x"`, ou un positionnel 9, avorte ;
    - `pico8_hp_od` (et `po`, `jb`, `lq`) sur une struct d'une autre classe (le `ok` de la partie) avorte ;
    - `pico8_hp_class()->struct_new` est posé, son `pq` NULL, et `"lq"` absent de ses défauts ;
    - `pico8_hp_fk()` est la REF lue par `lookup(projet, "fk")`.
- **Tests existants** : `rich_planet_exports` ne change que son cast. Les 10 autres tests qui lisent `gb` et tous les autres tests gardent un **diff vide** et restent verts.
- **Preuve rouge de `rich_planet_exports` (§6)**, dans une copie jetable :
  - remettre le bug d'origine, `#po` qui compte la clé nommée `mc` (`ob_sequence_length` de `bb` remplacé par le compte total des entrées) ;
  - le test réécrit doit être rouge, puis vert une fois le correctif restauré ;
  - le résultat est noté dans le message de commit.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` `test_pico8_hp`, `test_pico8_ga` et `test_pico8_rich_planet_exports`, avec les objets du moteur. Les lancer, puis `gcov` sur `pico8_research.c`, `pico8_research_display.c`, `pico8_planet.c`, `pico8_planet_display.c`, `pico8_economy_tiers.c`.
    - Même piège qu'à 1a.23-1a.32 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`.
    - Les branches d'assert atteintes seulement dans un fils qui avorte ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu.
    - Le rapport est collé dans le message de commit : lignes exécutables ajoutées ou modifiées de `src/` et leur compte. Il couvre :
      - le constructeur, dont les branches `ii`, `px` absent et overrides NULL ;
      - le lecteur, dont `lq` membre et `lq` fonction ;
      - l'état statique ;
      - `po`/`od`/`jb`/`lq`/`bj` ;
      - `jo:pq`/`lz`/`hu`, dont les trois branches `cw`/`go`/`lz` ;
      - `jo:fe`, `bb`, la libération de `gb`.

      Les lignes à 0 sont données avec leur justification.
  - **2 à 3 mutations**, chacune rouge sur `test_pico8_hp` (ou un autre test nommé), notées dans le commit avec le test qui l'attrape :
    1. **valeur d'un défaut propre** : `pico8_hp_fk()` rend la REF de `gp`. La queue de `lq` change ; scénario 2 rouge, rejeu `built_planet_panel` rouge ;
    2. **entrée du descripteur** (le lecteur, seule entrée) : `lq` rend toujours la fonction, même quand le membre est posé. La collision est inversée ; scénario 2 (`mv` sur un projet à `ii`) rouge ;
    3. **ordre du constructeur** : `i` réécrit **avant** `ho`. `i` devient la chaîne brute ; scénarios 1 et 3 rouges.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **`make float-audit`** : vert, `tools/float_audit_c1.txt` inchangé.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** :
  - `test_pico8_trade` et `test_pico8_ui` : structure, `lookup`, `must_abort`, fils par scénario, rejeu court, forme ;
  - `test_pico8_ga` (`harvest_recipe`, `harvest`) : trouver un projet de `gb` par `cw`, installer par `pico8_jo_hu` ;
  - `test_pico8_jo` (bloc `kw`/`lz`) : `pico8_jo_lz` sur une entrée de `gb` ;
  - `test_pico8_tech_screen` : rendre une tech connue par le jeu ;
  - le PRD de 1a.31 (`ui.hw`) et celui de 1a.29 (`pj.gs`) : défaut lu sur la classe par l'état statique.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; c'est le seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` (en particulier `hp  510  15/15`). Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier :
  - si un membre autre que les 17 nommés est ajouté (`mr`, `gp`, `fk`, `px`, `lr`, `fj`, `ja`, `i`, `o`, `il`, `dm`, `rr`, `ii`, `cw`, `go`, `rn`, `lq`) ;
  - si la clé dynamique `ri[p]` n'est pas énumérée comme les 8 noms de `hp.mr`, ou si l'ensemble de ses clés n'est pas prouvé fini (assert d'init `mr` = liste des 8) ;
  - si un lecteur d'un positionnel d'`hp` est découvert et traité autrement que par l'arrêt ;
  - si le constructeur accepte une autre clé sans avorter, ou si son assertion se déclenche sur un rejeu ou un test hors `must_abort` ;
  - si un écrivain est posé, ou une entrée de méthode non NULL ;
  - si l'enum change.
- `FAILED:` si un site hors de `pico8_class.c` passe une `hp` à `pico8_instance_get_field` ou `pico8_instance_fields` (grep dans `src/` et `tests/`), si `pico8_research.c` appelle encore `pico8_class_instantiate` ou `pico8_class_set_pq`, ou si un site hors de `pico8_research_init` lit ou écrit `g_hp_class->defaults`/`pico8_hp_class()->defaults`.
- `FAILED:` si `mr`, `gp` ou `fk` sont re-parsés au lieu d'être pris dans `cls->defaults`, ou si l'état statique et le membre ne portent pas la même REF.
- `FAILED:` si `pico8_mv(b,"lq")` ne rend plus la table pour un projet à `ii`, ou n'appelle plus `pico8_hp_lq` pour les autres (valeur figée de `test_pico8_hp`).
- `FAILED:` si `po`, `od`, `jb`, `lq`, `jo:lz` ou `jo:hu` acceptent un pointeur non NULL qui n'est pas un projet sans avorter, ou changent de comportement sur un projet (valeur figée de `test_pico8_hp`).
- `FAILED:` si `pico8_ho_apply` change, ou si la conversion de `df` ou de `nb` change.
- `FAILED:` si une autre classe que `hp` est convertie ou change de forme (`lq`, `hy`, `fm`, les enveloppes…), si la classe `hp` cesse d'être déclarée et enregistrée à la même place, ou si le comportement d'une autre classe change (valeur figée de sa caractérisation, ligne de couverture).
- `FAILED:` si un test de régression (§6) autre que `rich_planet_exports` a un diff non vide, si `rich_planet_exports` change au-delà du type de son cast, ou si sa preuve rouge (bug remis, test rouge, correctif restauré) manque au message de commit.
- `FAILED:` si une valeur figée de `test_pico8_hp` a été changée après la conversion, ou une d'une autre caractérisation.
- `FAILED:` si une ligne est ajoutée à `tools/float_audit_c1.txt` ou en est retirée, ou si `make float-audit` est rouge.
- `FAILED:` si `test_pico8_hp` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.
- `FAILED:` si la note « Découvert à 1a.33 » manque à la ligne 1a.33 de `docs/plan-3.md`.

Critères de réussite (en plus) :

- la struct `hp` n'a que l'en-tête et les 17 membres ; son descripteur n'a aucune méthode, un lecteur, pas d'écrivain ;
- grep :
  - `PICO8_CLASS_COVERAGE_NAME(g_hp_class, "hp")` reste ;
  - `pico8_class_set_struct_new(g_hp_class, …)` est présent ;
  - `pico8_class_set_pq(g_hp_class` et `->defaults, "lq"` sont absents ;
  - aucun `pico8_instance_get_field` dans `pico8_research.c`, ni sur un projet dans `pico8_research_display.c`, `pico8_planet.c` (`lz`/`hu`), `pico8_economy_tiers.c` (`dm`) ;
  - `pico8_jo_lz`/`pico8_jo_hu` prennent `pico8_hp_t *` ;
- le message de commit dit « découvert à 1a.33 ».

## Out of Scope

- Les autres classes et instances sans classe : `lq` (1a.34, y compris l'instance rendue par `pico8_hp_lq`/`pico8_jo_ex` et ses `pico8_instance_fields(l)`), `hy` (1a.35), `fm` (1a.36), les `{}`/`et={}`/`{qs}`/`re` jetables (1a.37-1a.40), l'enveloppe de `pz` sur `g_ha` (1a.41).
- `ho` lui-même et les autres tables qu'il renomme (`df`, `nb`), `fd`, `hc`, `r`, `gb` comme table.
- Le nombre `jo.hp` (index de `gb` sur une sous-classe de planète) : membre de `jo`, déjà converti.
- Supprimer la classe `hp`, les registres et `pico8_class.c` : 1a.42.
- Typer les membres (`i` en liste, `mr`/`gp`/`fk` en tables) ou `jo.hn` en `pico8_hp_t *` : phase 1b.
- Libérer les tables `i`/`lq` d'un projet (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Racine, pas de parente à fermer** : aucune fonction partagée ne passe à la forme struct seule ici ; aucune double forme ouverte ne se solde.
- **Ce que le filet voit des projets** : la liste de `jo:fe` (478 appels de `mv`), les installations (`cw` 7, `go` 2, `lz` 88), la production (`dm`, `po`), sur 15 rejeux. `go` n'est vu que par `session_events`. La caractérisation dirigée et les mutations visent ce que le filet voit peu : la ligne d'un projet à `ii` et `go`.
- **Écarts avec la ligne du plan** : les numéros `research.c:139,143` sont `:140,144` à HEAD, et `research_display.c:314` est `:313`. `collect.c:215` et `research.c:75` n'ont pas bougé.
- Mesures :
  - grep sur `src/`, `tests/`, `tools/` et le `Makefile` à d008307 ;
  - lecture de `pico8/lua.lua:17-23,86-88,403-464,1391,1399-1440` ;
  - lecture de `pico8_research.c/h`, `pico8_research_display.c`, `pico8_collect.c` (`ho_apply`), `pico8_planet.c` (`jo:pq`/`lz`/`hu`), `pico8_planet_display.c` (`jo:fe`), `pico8_economy_tiers.c` (`bb`), `pico8_data_init.c`, `pico8_entity.c` (`get_field`, `mv`), `pico8_struct.h`, `pico8_class.c/h`, `pico8_ui.c` (`hw`), `pico8_score_panel.c` (`gs`), `pico8_ui_button.c` (`js.hb`), `test_pico8_trade`, `test_pico8_ga`, `test_pico8_jo`, `test_pico8_rich_planet_exports`, de `tools/class_coverage_net_expected.txt` et `tools/float_audit_c1.txt` ;
  - lecture des PRD de 1a.16b et 1a.32, du commit ea1b826 (1a.32) ;
  - sonde dans une copie jetable du scratchpad, supprimée après usage :
    - 18 rejeux en sortie 0 ; 34 projets par chargement, 612 constructions, toutes depuis `build_gb`, toutes aux clés mesurées ;
    - lectures propres ou absentes, sauf `lq` par `mv`, qui trouve la méthode sur la classe ;
    - aucune écriture après `pq`, aucun positionnel lu hors de `pq` ;
  - fil piège, même copie, sortie 0 : `make unit` (37 binaires de test et 2 outils qui chargent le cart), puis les 8 cibles de `UNIT_TESTS_SLOW`. Mêmes sites, mêmes clés, aucune écriture ; `go` atteint par `test_pico8_jo`, `nr`, `pm`, `qh` ; `cw` lu en plus par `test_pico8_ga:105` (`pico8_entity_get_field`).
