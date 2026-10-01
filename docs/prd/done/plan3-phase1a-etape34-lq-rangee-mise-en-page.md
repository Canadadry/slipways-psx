---
title: "Plan 3, étape 1a.34 : `lq` (la rangée mise en page) devient une struct racine ; ses positionnels en un membre liste, `c`/`bg`/`score`/`ra`/`fs`/`w` en membres, `ol` par le descripteur"
description: "`lq` est construite 10 718 fois sur le filet (15/15 rejeux), par six sites : `jo.ex`, `jb:pq`, `js:pq` (`lq` et `h_`), `ix[2].hb`, `pj:ph`. Elle passe en struct `pico8_lq_t` : en-tête, un membre liste `items` (les `self[1..n]` du cart, dans l'ordre d'index croissant que `lq:pq` parcourt), puis `c`, `bg`, `score`, `ra` (clés nommées reçues), `fs`, `w` (écrits par `pq`). Son descripteur a `OL`, un lecteur, pas d'écrivain. `hp:lq` ajoute ses deux entrées au membre liste sans toucher `fs`/`w` ; `js:pq` recopie une `lq` par une vue en table. Cinq tests de régression changent leur façon de lire la rangée : les quatre de la ligne du plan, plus `built_planet_panel_nil_row`."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.34 — `lq` : racine (`layout.c:354`), champs positionnels → membre liste. Tests : `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`, `rich_planet_exports`.

Le numéro de ligne a dérivé. À HEAD c452539 :
- **`pico8_layout.c`** :
  - `g_lq_class = pico8_class_declare(NULL, NULL)` à `:394` ; nom de couverture `:395` ; `pico8_class_set_pq(g_lq_class, pico8_lq_pq)` `:396` ; installation de `ol` dans `g_lq_class->defaults` `:397` ;
  - `pico8_lq_new` `:422` (`pico8_class_instantiate`) ;
  - `lq:mn` (`pico8_lq_mn`, `:84-158`) lit `c` sur l'instance `:137` ;
  - `lq:pq` (`pico8_lq_pq`, `:187-203`) : parcourt les positionnels par `pico8_layout_positional_values(pico8_instance_fields(self))` `:188` (ordre d'index croissant), écrit `fs` `:201` et `w` `:202` dans la table de l'instance ;
  - `lq:ol` (`pico8_lq_ol`, `:228-253`) lit `w` `:239` et `fs` `:244` ;
  - `jb:pq` construit une `lq` par ligne (`pico8_jb_pq_step`, `:278`) ; `jb:ol` appelle `pico8_lq_ol` sur chaque entrée de `ls` `:326` ;
  - `pico8_lq_ol_dispatch` `:334` (le pont `mv(…,"ol")`).
- **`pico8_layout.h`** : `pico8_lq_new` et `pico8_lq_ol`/`pico8_lq_ol_default` typés `pico8_instance_t *` ; `pico8_lq_class()`.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` […] ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe […]. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, …) passent par un enum de méthode qui indexe cette table.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […] jusqu'à la dernière étape de 1a. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct […].
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : […] dans la table de méthodes (installation de méthode) […].
>
> **Ordre** : […] Les racines sans parent dans n'importe quel ordre.

« Champs positionnels → membre liste » : c'est le traitement de `mh` à 1a.4 (`pico8_mh_t.items`, un `pico8_list_t`), que le PRD de 1a.4 rattachait déjà à cette ligne.

`lq` est une **vraie racine sans fille** (écarts relevés du §2). Elle ne descend pas de `ke` : pas de `lt`, jamais dans `hv`/`cv`/`cg`. Ses précédents :
- `jb` (1a.1), sa voisine de module : racine à constructeur posé par `pico8_class_set_struct_new`, méthode `OL` par le descripteur, positionnels reçus ;
- `mh` (1a.4) : positionnels en un membre liste ;
- `hp` (1a.33) et `trade` (1a.32) : racine à lecteur sans écrivain, fonctions et consommateurs retypés.

§6 : chaque test de régression qui lit une `lq` est réécrit **dans sa seule façon de lire l'état**, et prouvé rouge avec son bug d'origine remis localement, dans ce commit.

§7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` : `lq  10718  15/15`, toutes les autres lignes inchangées ;
- preuve des tests par `gcov` et 2 à 3 mutations ;
- note « Découvert à 1a.34 », réduite à ce qui change une étape suivante ;
- un nouveau test dont le rejeu dépasse 30 s va dans `UNIT_TESTS_SLOW` ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.33 ont posé, réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (`methods` indexé par l'enum, `field`, `set_field`).
  - `pico8_entity_get_field` cherche d'abord le nom dans l'enum ; entrée posée → rendue ; sinon le lecteur. Un membre nil est absent.
  - `set_field` NULL : une écriture générique avorte.
  - `pico8_mv` sur une struct passe par l'entrée du descripteur.
- Racine convertie : en-tête en premier membre ; constructeur `static` posé par `pico8_class_set_struct_new` et appelé par `pico8_<classe>_new` ; couverture enregistrée par le constructeur ; assert porteur sur toute clé non mesurée.
- `PICO8_METHOD_OL` existe (1a.1, `jb`) : **l'enum ne change pas**.
- Fonctions propres retypées sur la struct, assert de descripteur sur un pointeur non NULL (`trade`, `hp`).
- Caractérisations qui pliaient la forme instance : le pli est adapté pour rendre la même valeur figée sur la forme struct (1a.33, `test_pico8_js` et `in_gb`).

### Ce que fait le cart

`pico8/lua.lua` :

```
:15-16     mw(o) = qj({},o)                       -- copie superficielle de toutes les clés propres
:86-88     kv:jr(rj) : __call → ob=setmetatable(mw(ob),rj.ns) ; puis les pq de la chaîne
:289-294   lq=kv:jr() ; lq:pq() : self.fs=nv(self,function(f) f,fw,fs=self:mn(f) … return {fn=f,d=v(x,0)} end) ; self.w=w-fs
:295-299   lq:ol(p,a) : if a then p-=v(a*self.w,0) end ; for f in all(self.fs) do f.fn(p+f.d) end
:300-312   lq:mn(f)   : table → {qe}→{qe,5} ; f,w,qw,qv=f[1..4] ; chaîne → couleur self.c or 6 ; sinon sprite si f vrai
:313       pi.ld=lq.ol                             -- appel non lié : oo(pi[e.fn],e) passe la lq en self
:314-315   jb:pq : self.ls=nv(self,function(ln) return lq(type(ln)=="string" and {ln} or ln) end)
:261-262   js:pq : self.lq=lq(mv(self,"lq")) ; self.mb=self.lq.w+2 ; self.h_=self.h_ and lq(self.h_)
:273-275   js:kj : self.lq:ol(p+v(1,2)) ; c=self.h_ : c.w, c.bg or 2, c:ol(…)
:530-538   jo.ex(…)   : l={…} ; si jq>5 : add(l,tostr(jq)) add(l,{false,-4}) l.c=13 ; … ; return lq(l)
:539       jo:hb()    : self.lq=jo.ex(…)
:548       jo:kj(p)   : mv(self.lq,"ol",p+v(0,7),0.5)
:885-891   pj:ph      : st[3]=… ; st[1]=… ; st[i]=… ; return {lq(st),st.ra,0.5,fn="ld"}
:982       ix[2].hb   : self.lq=lq({…})
:1409-1413 hp:lq()    : l=jo.ex(…) ; add(l,hp.fk) ; add(l,self.px)   -- après pq : fs et w ne les voient pas
```

Points décisifs :
- **Les positionnels sont les données de la rangée** : `lq:pq` les parcourt (`nv`, donc `all`) pour construire `fs` ; rien d'autre ne les lit dans le cart, sauf la recopie `mw` quand une `lq` est repassée à `lq()`.
- **Une `lq` repassée à `lq()`** : `js:pq` fait `lq(mv(self,"lq"))`. Quand la ligne vient de `jo:fe`, `mv(b,"lq")` a appelé `hp:lq`, qui rend une `lq` déjà construite. `mw` en copie **toutes** les clés propres (positionnels, `fs`, `w`, `c`…), puis `pq` réécrit `fs` et `w`.
- **`hp:lq` ajoute deux positionnels après `pq`** : `lq:ol` de cette `lq`-là ne les dessine pas, mais la copie de `js:pq` les voit. C'est le cart, pas un écart du port (commentaire de `pico8_research_display.h:12-15`).
- **`ol` est une méthode de la classe**, appelée par `mv(self.lq,"ol",…)` (`jo:kj`), par `self.lq:ol`/`c:ol` (`js:kj`), par `l:ol` (`jb:ol`) et, non lié, par `pi.ld` (`bl`).
- **`c` est lu par `mn` pendant `pq`** : la clé doit être copiée avant `pq`.

### Emprise réelle de `lq` (mesurée)

**Méthode.**
- Grep de `src/`, `tests/`, `tools/` et du `Makefile` à c452539 ; lecture du cart aux lignes ci-dessus.
- **Sonde** sur une copie `git archive HEAD` jetable du scratchpad, supprimée après usage. Un en-tête forcé (`-include`) remplace par des macros qui portent `__FILE__`/`__LINE__` : `pico8_lq_new`, `pico8_instance_get_field`, `pico8_instance_fields`, `pico8_entity_get_field`, `pico8_entity_set_field`, `pico8_mv`, `pico8_lq_ol`, `pico8_lq_ol_default`, et `pico8_type_tag_of` (journalisé seulement depuis `tests/`). `pico8_class.c` et `pico8_entity.c` gardent leurs appels internes non instrumentés. Elle journalise, pour toute instance de la classe `lq` : chaque construction (appelant ; clés reçues avec leur genre ; positionnels : nombre, index maximal, genres ; un nommé stocké avant un positionnel ; clés après `pq` ; source = table d'une `lq` existante) ; chaque lecture, écriture, `mv`, `ol`, `pico8_instance_fields` et prise de tag.
- Rejoués : les 17 `tests/fixtures/*events.json` et `events_recorded_1790198294.json` (`--from-title` pour les 4 enregistrements), **18 rejeux en sortie 0**.
- **Fil piège** : `make -k unit` puis les 8 cibles de `UNIT_TESTS_SLOW`, même copie. Tous les binaires qui se lient sont verts. Les binaires qui ne lient pas `pico8_class.c`/`pico8_entity.c` ne se lient pas avec la sonde. Parmi eux, seul `test_pico8_layout_nil_item_draws_nothing` touche une `lq` : lu à la main.

**Constructions** (toutes par `pico8_lq_new`, jamais par `pico8_class_instantiate` direct, `lookup_named`, `fz` ou `gx[]`) :

| Appelant | Cart | 18 rejeux | TU |
|---|---|---|---|
| `research_display.c:275` (`jo.ex` : `jo:hb`, `hp:lq`) | `:538` | 6 691 | 29 319 |
| `ui_button.c:246` (`js:pq`, `lq`) | `:261` | 1 313 | 7 262 |
| `layout.c:278` (`jb:pq`) | `:315` | 1 501 | 9 573 |
| `ui_button.c:259` (`js:pq`, `h_`) | `:262` | 702 | 3 380 |
| `menu.c:306` (`ix[2].hb`) | `:982` | 586 | 3 191 |
| `score_panel_display.c:243` (`pj:ph`) | `:891` | 28 | 182 |

Total 10 821 sur les 18 rejeux ; la référence du filet (15 rejeux) est `lq  10718  15/15`.

**Clés reçues** (filet et TU, mêmes formes) :
- **positionnels** : de 1 à 11 ; genres chaîne, nombre et table. Un `nil` explicite n'apparaît que dans `layout_nil_item_draws_nothing` (lu), un nombre 0 dans son contrôle.
- **à trou** : seulement la rangée « score 0 » de `pj:ph` (`o("",score=0,…,c=5)` puis `st[3]`, `st[1]`) : index 1 et 3, pas de 2 (4 sur les rejeux, 26 dans les TU).
- **nommées**, et rien d'autre :
  - `c` (nombre ; `js:pq` `lq`/`h_`, `jb:pq`, `pj:ph`) ;
  - `bg` (nombre ou `false` ; `h_` seulement) ;
  - `score` (nombre) et `ra` (REF vers un vecteur non tagué) : `pj:ph` seulement ;
  - `fs` (liste) et `w` (nombre) : seulement quand la source est une `lq` (462 sur les rejeux, 2 223 dans les TU, toutes depuis `hp:lq` par `ui_button.c:30`).
- **Surcharges NULL** (`lq(nil)`) : jamais sur le filet. Dans les TU seulement (`gh_row_lq`, `ga_export_panel_by`, `js`) : le premier rang de `jo:gh` d'une planète nue, sans `px` ni `mi`.
- **Nommé stocké avant un positionnel** (ordre de la table) :
  - les `lq` de `pj:ph` (`st[3]`/`st[i]` écrits après `score`/`ra`/`c`) ;
  - les tables de `lq` repassées par `js:pq` (`fs`/`w` avant les deux positionnels de `hp:lq`).
  - Jamais pour `jo.ex` sur le filet ni dans les TU. La branche `jq>5` de `jo.ex`, qui pose `c=13` entre deux positionnels, **n'est jamais atteinte** : chemin non exercé.
- **Après `pq`** : les clés reçues, plus `fs` (liste, une entrée par positionnel) et `w` (nombre).

**Lectures, écritures et appels sur une `lq`** :

| Site | Nom | 18 rejeux | Genre |
|---|---|---|---|
| `layout.c:137` (`mn`) | `c` | 4 340 | nombre ou absent |
| `layout.c:239` / `:244` (`ol`) | `w` / `fs` | 1 701 315 chacun | toujours posés |
| `ui_button.c:250` (`js:pq`) | `w` | 1 313 | nombre |
| `ui_button.c:50` via `:341` (`js:kj`, `h_`) | `w` | 132 744 | nombre |
| `ui_button.c:346` (`js:kj`, `h_`) | `bg` | 132 744 | nombre, `false` ou absent |
| `planet_display.c:541` (`jo:kj`) | `mv(…,"ol")` | 648 719 | argc 2 |
| `ui_button.c:336` / `:354` (`js:kj`) | `pico8_lq_ol_default` | 880 414 / 132 744 | — |
| `pi.c:143` (`ld`, `bl`) | `pico8_lq_ol` | 5 236 | — |
| `layout.c:326` (`jb:ol`) | `pico8_lq_ol` | lu | — |

- **`pico8_instance_fields` sur une `lq`** hors de `pq` :
  - `research_display.c:308,310,313` (`hp:lq` : compte les positionnels, ajoute `fk` puis `px`), 462 sur les rejeux ;
  - `ui_button.c:30` (`pico8_js_lq_arg_table`, cas `INSTANCE` : la table de la `lq` passée comme surcharges), 462. Le cas `INSTANCE` n'a jamais reçu autre chose qu'une `lq`.
- **Aucun** `pico8_entity_get_field` ni `pico8_entity_set_field` sur une `lq` dans `src/`. `score` et `ra` ne sont **jamais lus**.
- Aucun champ écrit après construction, hors des deux positionnels de `hp:lq`.

**Accès `->defaults`** sur la classe `lq` : `layout.c:397` (installation de `ol`). Aucun dans les tests.

**Asserts de forme dans `src/`** : `planet_display.c:539-540` (« jo.lq must be an lq instance », tag `INSTANCE`) ; `ui_button.c:28-38` (aiguillage par tag).

**Tests qui lisent une `lq`** (grep, puis sonde) :
- **régression (§6)** :
  - `layout_nil_item_draws_nothing:111,116` : `pico8_lq_new`, `pico8_lq_ol_default` sur `pico8_instance_t *` ;
  - `planet_gh_row_lq:65,122,135-142,145` : `lq->cls == pico8_lq_class()`, `lq->fields` aux index 1 à 5, `fs` par `pico8_instance_get_field` ;
  - `ga_export_panel_by:54` : `lq->fields` aux index 1 et 2 ;
  - `rich_planet_exports:96` : `->fields` à l'index 1 ;
  - **`built_planet_panel_nil_row:96-98`** : `lq->fields` à l'index 1. **Absent de la ligne du plan** (voir décisions).
- **caractérisations** :
  - `test_pico8_js:132,143` (`w` par `pico8_entity_get_field`), `:243` (tag `INSTANCE`, `cls == pico8_lq_class()`), `:245` (`->fields`), `:265-266` (tag plié dans `fold_built`), `:793` (`->fields[1]`), `:954` (tag `INSTANCE` de `h_`) ;
  - `test_pico8_pj:103` (`w`, `c` par `lookup`), `:522` (tag), `:525,551` (`pico8_instance_fields`) ;
  - `test_pico8_ga:464` (tag), `:465` (`fold_table(lq->fields)` : clés et valeurs dans l'ordre de la table) ;
  - `test_pico8_hp:212` (`fold_lq_instance` : `fold_value` de la table entière, profondeur 4 ; sur `pico8_hp_lq(b)` et `mv(b,"lq")`).
- **Aucune autre** : `test_pico8_jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr` nomment `"lq"` dans leurs listes de membres de planète, mais ne prennent jamais le tag d'une `lq` présente (sonde : 0 prise de tag hors `js`, `pj`, `ga`).
- `test_pico8_jb_ol` construit des `lq` par `jb` et ne les lit pas.

**Outils** :
- `tools/float_audit_c1.txt:27-28` nomment `pico8_lq_ol_item_draw` (`item->color`, `item->sprite_id`) : la fonction ne change pas.
- `ref-audit` : `PICO8_REF_AS_LIST_CONST` sur `fs` (`layout.c:245`) gardé.

**→ Membres de `lq`**, dans cet ordre :
- `items` : un `pico8_list_t`, les positionnels reçus **dans l'ordre d'index croissant** (celui de `pico8_layout_positional_values`, que `pq` parcourt), trous resserrés ;
- `c`, `bg`, `score`, `ra` : clés nommées reçues ;
- `fs`, `w` : écrits par `pq`.

**→ Méthodes par le descripteur** : `OL` = `pico8_lq_ol_dispatch`. **Lecteur** : les 6 membres nommés (nil = absent) ; `items` n'est pas lu par nom. **Écrivain** : aucun.

## Solution

1. **Enum : inchangée** (`OL` existe).
2. **Struct `lq`**, publique dans `pico8_layout.h` à côté de `pico8_jb_t` : `pico8_struct_header_t`, puis `items` (`pico8_list_t`), puis `c`, `bg`, `score`, `ra`, `fs`, `w` (`pico8_value_t`).
3. **Constructeur** (`static`, posé par `pico8_class_set_struct_new` dans `pico8_layout_init`, appelé directement par `pico8_lq_new`), dans l'ordre de `kv:jr` puis `lq:pq` :
   1. alloue un `pico8_lq_t` ; pose l'en-tête (tag STRUCT, descripteur de `lq`) ; met les 6 membres à nil et `items` à vide ;
   2. **asserte** (message porteur, arrêt §7) que les surcharges ne portent que des positionnels et les clés nommées `c`, `bg`, `score`, `ra`, `fs`, `w` ;
   3. copie les clés nommées reçues dans leurs membres (la copie `mw`), et remplit `items` par `pico8_layout_positional_values(overrides)` ;
   4. `pq` : construit `fs` par `pico8_nv` sur `items`, avec `self` pour la couleur `c`, et pose `w`, comme aujourd'hui (un `fs`/`w` reçu est réécrit, comme dans le cart) ;
   5. enregistre `"lq"` au crochet de couverture.

   Surcharges NULL : `items` vide, `fs` liste vide, `w` = 0, le reste nil, comme aujourd'hui.
4. **Descripteur de `lq`** (`static const`) : `OL` = `pico8_lq_ol_dispatch` ; les autres méthodes NULL. **Lecteur** : `c`, `bg`, `score`, `ra`, `fs`, `w` (nil = absent). **Écrivain NULL**. Accesseur public `pico8_lq_struct_class()`, comme `pico8_hp_struct_class()`.
5. **Vue en table** : une fonction publique du module rend une **table neuve** qui reproduit la table de l'instance d'aujourd'hui, **clés dans le même ordre** :
   - les `k` premiers éléments de `items` aux index 1..k, où `k` est le nombre d'entrées de `fs` (les positionnels vus par `pq`) ;
   - puis `c`, `bg`, `score`, `ra` non nils ;
   - puis `fs`, `w` ;
   - puis les éléments ajoutés après `pq` aux index k+1…

   Les valeurs sont partagées, pas copiées en profondeur. L'appelant libère la table, pas son contenu. C'est la copie `mw` d'une `lq`, pour `js:pq`. Les caractérisations qui pliaient la table de l'instance la plient à sa place (voir Testing).
6. **Fonctions de `lq` sur la struct** :
   - `pico8_lq_new` rend `pico8_lq_t *` ;
   - `pico8_lq_ol` et `pico8_lq_ol_default` prennent `const pico8_lq_t *`, lisent `w` et `fs` dans les membres. Mêmes gardes : NULL, `fs` non liste ;
   - `pico8_lq_mn` lit `c` dans le membre, même vérité ;
   - `pico8_lq_ol` asserte (message porteur) qu'un `self` non NULL est une struct au descripteur de `lq` ;
   - `jb:pq`/`jb:ol` : type de l'élément seulement.
7. **Consommateurs hors du module** :
   - `pico8_jo_ex` et `pico8_hp_lq` rendent `pico8_lq_t *`. `hp:lq` ajoute `fk` puis `px` à la fin de `items`, avec les mêmes gardes nil, **sans toucher `fs` ni `w`** ;
   - `pico8_jo_hb` : type de `lq_result` ;
   - `jo:kj` (`planet_display.c:539`) : l'assert de forme exige une struct au descripteur de `lq` ; `pico8_mv(…,"ol")` inchangé ;
   - `js:pq` (`ui_button.c`) :
     - `pico8_js_lq_arg_table` remplace son cas `INSTANCE` par le cas d'une struct au descripteur de `lq`, qui rend la vue en table. Une autre struct tombe dans l'assert existant ;
     - `mb` lit `new_lq->w` ;
   - `js:kj` : `h_` typé `const pico8_lq_t *`, `w` et `bg` lus dans les membres, mêmes vérités ;
   - `ix[2].hb` (`menu.c`) et `pj:ph` (`score_panel_display.c`) : type seulement ;
   - `pi.c` (`ld`) : `a1` casté en `const pico8_lq_t *`.
8. **Classe `lq`** : gardée, enregistrée à la même place (emplacement `""`), constructeur de struct posé. **Le `pq` et l'installation de `ol` sont retirés** de la classe. Plus aucun accès `->defaults` sur `lq`. `pico8_lq_class()` reste.
9. **Tests** : nouveau `test_pico8_lq` ; les cinq tests de régression réécrits dans leur lecture (§6) ; `test_pico8_js`, `pj`, `ga`, `hp` adaptés dans leur pli, **valeurs figées inchangées**. Voir Testing Decisions.
10. **`TROU-25`** (voir décisions) : la rangée à trou de `pj:ph`.
11. **Plan** : à la ligne 1a.34 de `docs/plan-3.md`, ajouter « Découvert à 1a.34 : … », réduit à ce qui change une étape suivante (§7). A priori :
    - positionnels en `items`, ordre d'index croissant, trous resserrés ;
    - `c`/`bg`/`score`/`ra`/`fs`/`w` membres ;
    - `OL` par le descripteur, enum inchangée, pas d'écrivain ;
    - vue en table pour la copie `mw` de `js:pq` ;
    - `built_planet_panel_nil_row` réécrit aussi ;
    - `TROU-25`.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `lq` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `hy`, `fm` et les instances sans classe à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `lq` mesurée (constructions par appelant, clés reçues, noms lus, `mv`, `pico8_instance_fields`, prises de tag des tests), afin que chaque membre, l'entrée `OL`, l'absence d'écrivain et chaque test touché soient justifiés par une mesure.
3. En tant que lecteur du code, je veux les positionnels d'une `lq` dans un membre liste `items`, comme `mh`, afin que la ligne du plan (« champs positionnels → membre liste ») soit tenue par un précédent.
4. En tant que lecteur du code, je veux une struct `lq` qui n'a que l'en-tête, `items` et les 6 membres nommés, noms minifiés, afin que la traçabilité vers `pico8/lua.lua` tienne.
5. En tant que joueur, je veux que chaque rangée d'icônes (panneau de planète, boutons, bulles, menu, score, icônes flottantes au-dessus des planètes) soit dessinée pixel pour pixel comme avant, afin que la partie ne change pas.
6. En tant que joueur, je veux que la largeur d'un bouton (`mb`, tirée de `w`) et la pastille `h_` (`w`, `bg`) restent identiques, afin que le panneau d'une planète ne bouge pas.
7. En tant que joueur, je veux que la ligne d'un projet de `jo:fe` montre toujours le coût `fk` et le nom `px` que `hp:lq` ajoute, afin que la copie de `js:pq` voie ce que le cart voit.
8. En tant que joueur, je veux que les icônes flottantes de `jo:hb` ne dessinent pas ces deux ajouts (absents de `fs`), afin que la collision du cart entre `pq` et `add` soit préservée.
9. En tant que joueur, je veux que la rangée des étoiles du score reste dans l'ordre d'index, même écrite dans le désordre (`st[3]` avant `st[1..5]`), afin que `score_panel_stars` reste vert.
10. En tant que mainteneur, je veux que le constructeur n'accepte que des positionnels et `c`/`bg`/`score`/`ra`/`fs`/`w`, et avorte sur toute autre clé, afin qu'une surcharge jamais mesurée soit un arrêt (§7).
11. En tant que mainteneur, je veux qu'aucun écrivain ne soit posé, afin qu'une écriture générique jamais mesurée sur une `lq` soit un arrêt.
12. En tant que mainteneur, je veux `ol` dans le descripteur, afin que `mv(self.lq,"ol",…)` de `jo:kj` passe par l'enum et que `->defaults` ne soit plus touché.
13. En tant que mainteneur, je veux que `pico8_lq_ol`, `pico8_jo_ex`, `pico8_hp_lq` et `pico8_lq_new` parlent `pico8_lq_t *`, afin que le type dise ce que la mesure a prouvé.
14. En tant que mainteneur, je veux que `pico8_lq_ol` asserte le descripteur de `lq` sur un pointeur non NULL, afin qu'une REF qui ne serait pas une rangée (dans `pi.ld`, `jb.ls`, `js.lq`) soit détectée au lieu d'être lue de travers.
15. En tant que mainteneur, je veux qu'une `lq` repassée à `lq()` soit copiée par une vue en table qui garde l'ordre des clés d'aujourd'hui, afin que `js:pq` reste la copie `mw` du cart.
16. En tant que mainteneur, je veux que plus aucun site hors de `pico8_class.c` ne passe une `lq` à `pico8_instance_get_field`, `pico8_instance_fields` ou `->fields`, afin que la forme struct ne soit jamais lue par la table d'une instance.
17. En tant que gardien de la couverture, je veux que le constructeur enregistre `"lq"` une fois par rangée, afin que `class-coverage-net` reste `lq  10718  15/15`.
18. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne des références de couverture ni de `float_audit_c1.txt` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
19. En tant que vérificateur, je veux une caractérisation `test_pico8_lq` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable. Cela vaut aussi pour ce que le filet voit peu ou pas : `jo.ex` à `jq>5`, `lq(nil)`, l'item nil, `bg` à `false`, la rangée à trou.
20. En tant que vérificateur, je veux un rapport `gcov` qui montre chaque ligne ajoutée ou modifiée exécutée, et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
21. En tant que gardien des tests de régression, je veux que `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`, `rich_planet_exports` et `built_planet_panel_nil_row` ne changent que leur façon de lire la rangée, chacun prouvé rouge avec son bug d'origine, afin que leur objet reste intact (§6).
22. En tant que gardien des caractérisations, je veux que `test_pico8_js`, `pj`, `ga` et `hp` gardent leurs valeurs figées, seul leur pli adapté à la forme struct, afin que la conversion ne se valide pas en réécrivant ses témoins.
23. En tant qu'autopilote, je veux que l'écart port/cart découvert (rangée à trou) soit préservé et décrit par `TROU-25`, afin que la conversion ne corrige rien en passant.
24. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C, un piège ou un `TROU-N` nu (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
25. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain : modules attendus, modules à tester, découpage. J'ai appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** :
  - la rangée (`layout`) : struct, constructeur, descripteur, lecteur, vue en table, `mn`/`pq`/`ol` sur la struct, `jb` retypé ;
  - `jo.ex` et `hp:lq` (`research_display`), `jo:hb`/`jo:kj` (`planet_display`) ;
  - `js:pq`/`js:kj` (`ui_button`), `ix[2].hb` (`menu`), `pj:ph` (`score_panel_display`), `ld` (`pi`).

  Rien d'autre dans `src/`. Pas de changement dans `entity`, `class`, `pico8_struct.h` (enum), `collect` (`nv`).
- **`items` : ordre d'index croissant, trous resserrés** (G). C'est exactement la suite que `pq` parcourt aujourd'hui (`pico8_layout_positional_values`), donc `fs` et `w` sont identiques par construction. Rejeté : l'ordre de stockage (celui de `mh`), qui inverserait la rangée des étoiles, écrite `st[3]` avant `st[1..5]`. Le resserrement ne change l'index que de la rangée « score 0 » (index 1 et 3). Aucun site ne lit cette rangée par index : seuls `pq`, `ol` et `pi.ld` la touchent (mesuré). Un `nil` explicite reçu reste un élément, comme aujourd'hui (`layout_nil_item_draws_nothing`).
- **`c`, `bg`, `score`, `ra` membres** (G). Ce sont des champs écrits sur l'instance par la copie `mw`, donc des membres (§5), même `score` et `ra`, jamais lus (précédent : les clés de construction de `js`). `bg` peut valoir `false` : il est rangé tel quel. Le lecteur de `js:kj` n'en garde que le nombre, comme aujourd'hui.
- **`fs`, `w` membres, et acceptés en surcharge** (G). `pq` les écrit. Une `lq` repassée par `js:pq` les apporte, et `pq` les réécrit, comme `mw` puis `pq` dans le cart. Rejeté : les omettre de la vue en table. Ils ne sont pas observables par le rendu, mais le seraient par l'ordre des clés, que plient deux caractérisations.
- **Vue en table plutôt qu'un constructeur de copie** (G). `js:pq` reçoit, selon la ligne, une table, une liste ou une `lq`. La vue en table garde **un seul chemin**, `pico8_lq_new(table)`, qui est la copie `mw` du cart. Elle sert aussi de lecture fidèle aux plis de `test_pico8_ga` et `test_pico8_hp`, qui hachent l'ordre des clés.
  - Ordre : `k` premiers positionnels, puis nommés, puis `fs`/`w`, puis les ajouts. Il reproduit l'ordre de la table de l'instance pour toutes les tables où aucun nommé n'était stocké entre deux positionnels reçus. Sonde : c'est le cas de toutes les `lq` de `jo.ex` (filet et TU), les seules qu'on recopie ou qu'on plie par ordre.
  - Mémoire : la vue est libérée par `js:pq` après `pico8_lq_new`, qui en a copié les valeurs. Le cas `LIST` voisin reste tel quel (TROU-7).
- **`hp:lq` écrit `items` directement** (G) : `pico8_list_add` sur le membre, gardes nil inchangées. Pas d'accesseur pour deux appels. Le compte des positionnels devient `items.count`, égal pour une rangée de `jo.ex`, contiguë par construction.
- **`OL` par le descripteur**, pas par le lecteur. Contrairement au `lq` de `hp` (1a.33), aucune `lq` ne porte de champ `ol` qui devrait passer avant la méthode (mesuré : clés reçues). `pi.ld` appelle `pico8_lq_ol` directement, comme aujourd'hui.
- **Assert de `pico8_lq_ol`** (G) : un pointeur non NULL est une struct au descripteur de `lq` (message porteur). La garde NULL reste. `pico8_lq_mn` (statique, appelé par `pq`) n'en a pas besoin.
- **`pico8_js_lq_arg_table`** (G) : le cas `INSTANCE` ne recevait que des `lq` (mesuré, 462/462). Il devient le cas d'une struct au descripteur de `lq`. Une instance, ou une struct d'une autre classe, tombe dans l'assert existant, avec son message ajusté (« untagged or unmeasured REF »).
- **`built_planet_panel_nil_row` réécrit aussi** (G). Il lit `lq->fields[1]` (`:98`), qui ne veut plus rien dire sur une struct. La ligne du plan ne le nomme pas, mais le §6 vaut pour tout test de régression qui lit la classe convertie. Même traitement que les quatre nommés : lecture de `items[0]`, preuve rouge (voir Testing). Ce n'est pas un changement d'intention. Précédent : 1a.26 a réécrit trois tests hors de sa ligne.
- **`TROU-25` : une rangée à trou** (G). La rangée « score 0 » de `pj:ph` porte les index 1 et 3.
  - Le port en dessine les deux : `positional_values` prend toutes les clés d'index, et `st[3]` est la chaîne vide de TROU-13.
  - Le cart parcourt `all(self)`, qui s'arrête à la bordure `#t`. Pour une table construite `{[1]}` puis complétée en `[3]`, c'est 1 dans l'implémentation Lua 5.2, non vérifié sur PICO-8.
  - L'écart change `w`, donc le centrage (`a=0.5`) de la rangée, atteinte par le filet (`game_end`, 4 constructions).
  - Il est préexistant, pas créé par ce commit : le membre `items` le **préserve** (trou resserré, les deux éléments gardés).
  - Nouvelle entrée `TROU-25` dans `docs/trous-de-portage.md` (source : `lua.lua:885-891`, `pico8_layout.c` `positional_values`, `pico8_score_panel_display.c`), « non vérifié sur PICO-8 » comme TROU-20/22, référencée par `/* TROU-25 */` au site du resserrement.
  - Si le vérificateur juge que ce n'est pas un écart, l'entrée est la seule chose à retirer. Le code ne change pas.
- **Constructeur** (précédent `jb`) : couverture enregistrée une fois ; assert de clé porteur (« a named key other than c/bg/score/ra/fs/w would be a field under no rule of plan-3 section 5 -- stop, do not add a member »).
- **Mémoire** : une `lq` n'est jamais libérée, ni sa liste `items`, ni `fs` et ses éléments (TROU-7, comme l'instance). Les valeurs de `items` sont empruntées à l'appelant (sous-tables partagées, comme `mw`). Contrat à écrire à la déclaration de la struct.
- **Commentaires** (CLAUDE.md) :
  - à ajouter :
    - contrat de la struct : `items` = `self[1..n]` du cart en ordre d'index, emprunts, rien n'est libéré ;
    - assert de clé ; assert de descripteur ;
    - contrat de la vue en table : table neuve, valeurs partagées, l'appelant libère la table seule, ordre des clés ;
    - `/* TROU-25 */` ;
  - à corriger ou retirer, rendus faux :
    - `pico8_layout.c:390-391` (« "ol" is wired onto lq's defaults ») ;
    - `pico8_research_display.h:12-15,25` (« fresh `lq` instance », « this whole fields table ») ;
    - `pico8_ui_button.c:46` (« An `lq` instance's field ») ;
    - `planet_display.c:540` (message d'assert) ;
    - l'en-tête de `pico8_layout.h` sur les deux classes, qui reste vrai (pas de `ke`) ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert. La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_lq` sur l'ancien code** (voir Testing Decisions), valeurs figées. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si une construction porte une clé nommée hors de `c`/`bg`/`score`/`ra`/`fs`/`w` : la mesure est fausse et doit être corrigée dans le PRD.
2. **`lq` converti** :
   - struct, constructeur, descripteur, lecteur, vue en table, `set_struct_new`, retrait du `pq` et de l'installation de `ol` ;
   - `mn`/`pq`/`ol`, `jb` ;
   - `jo.ex`/`hp:lq`, `jo:hb`/`jo:kj`, `js:pq`/`js:kj`, `ix[2].hb`, `pj:ph`, `ld` ;
   - les cinq tests de régression (lecture) ; les plis de `test_pico8_js`, `pj`, `ga`, `hp`, et le seul accesseur de lecture de `test_pico8_lq` ;
   - `TROU-25`.

   Toutes les caractérisations passent **sans changer leurs valeurs figées**. Les vérifications de forme s'ajoutent à `test_pico8_lq`.
3. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, preuves rouges des cinq tests de régression, `make class-coverage-net`, `make ref-audit`, `make float-audit`, ligne 1a.34 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : les pixels dessinés par `ol` (direct, par `mv(…,"ol")`, par `pi.ld`, par `jb:ol`), `w` et `fs` (compte) lus par `pico8_entity_get_field`, les positionnels dans l'ordre, `mb` d'un bouton, le compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_lq`** : lié comme `test_pico8_hp` (cart réel, `ENGINE_SRCS`), avec des dépendances Makefile sur les fixtures qu'il rejoue.
  - Il lit les rangées par `pico8_entity_get_field` (`w`, `fs`, `c`, `bg`, `score`, `ra`), par les pixels et par **un seul accesseur local** des positionnels (compte, i-ième). Cet accesseur est la seule ligne du test que le morceau 2 réécrit (`->fields` → `items`), aux vérifications de forme près.
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()`.
  - Toute présence se lit par un `lookup` qui confond nil et absent.
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **Constructions directes**, chacune dessinée par `ol` sur un écran effacé à `a` = 0, 0.5 et 1. Pour chacune : empreinte d'écran, `w`, compte de `fs`, positionnels, `c`/`bg`/`score`/`ra`. Les cas :
       - chaîne seule ; chaîne avec `c` ; nombre (sprite) ; table `{spr,w,qw,qv}` ; table `{qe=…}` ;
       - `false` ; `nil` explicite ; mélange ;
       - surcharges NULL ;
       - rangée à trou (index 1 et 3, comme « score 0 ») ;
       - index écrits dans le désordre (3 puis 1, 2, 4, 5, comme les étoiles) ;
       - `bg` nombre et `false` ; `score`/`ra`.
    2. **`jo.ex`** appelé directement, dont `jq>5` (chemin non exercé : `c=13` entre deux positionnels) : empreinte d'écran, `w`, `c`, positionnels.
    3. **`hp:lq` et la copie** : pour trois projets de `gb`, `pico8_hp_lq(b)` (positionnels, `w`, compte de `fs` inchangés par les deux ajouts), puis un bouton `pico8_js_new` dont `lq` est cette rangée (positionnels de la copie, dont `fk`/`px`, son `w`, `mb`, empreinte de `js:kj`).
    4. **Appels** : `pico8_mv(lq,"ol",…)` dessine comme `pico8_lq_ol` ; une entrée `{lq,ra,0.5,fn="ld"}` passée à `pico8_bl_default` dessine comme `pico8_lq_ol(…,0.5)`.
    5. **Rejeu court** `built_planet_panel_events.json` (départ comme le filet, jusqu'à la dernière frame) : pour chaque bouton de `ok.le`, `w`, `c`, compte et genres des positionnels de son `lq` et de son `h_`, et `bg` ; empreinte d'écran finale. Figé.
  - **Forme** (ajoutée au morceau 2) :
    - une rangée est `STRUCT` au descripteur de `lq` (`pico8_lq_struct_class()`) ; `OL` posé, toutes les autres méthodes NULL ; `field` posé, `set_field` NULL ;
    - `pico8_entity_set_field(lq, "w", …)` avorte (dans un fils) ;
    - `pico8_lq_new` avec une clé `"x"` avorte ;
    - `pico8_lq_ol` sur une struct d'une autre classe (le `ok` de la partie) avorte ;
    - `pico8_lq_class()->struct_new` est posé, son `pq` NULL, et `"ol"` absent de ses défauts ;
    - la vue en table d'une rangée de `hp:lq` a ses clés dans l'ordre « positionnels de `pq`, `fs`, `w`, ajouts ».
- **Tests de régression (§6)**, réécrits dans leur **seule façon de lire la rangée** : type `const pico8_lq_t *`, `items` au lieu de `->fields`, descripteur au lieu de `cls`. Aucune assertion retirée ni affaiblie, mêmes valeurs attendues. Makefile : dépendances de lien ajustées si `pico8_layout.c` en réclame. Preuve rouge de chacun sur le code final, dans une copie jetable, par `make tmp/test_pico8_<nom> && ./tmp/test_pico8_<nom>` (`ASAN_OPTIONS=detect_leaks=0` pour `planet_gh_row_lq`). Le commit note le bug remis, l'échec observé, la restauration et le vert. **Si un bug remis ne rend pas son test rouge, c'est un arrêt (§7), pas un test durci.**

  | Test | Lecture réécrite | Bug d'origine | Remis localement | Attendu |
  |---|---|---|---|---|
  | `layout_nil_item_draws_nothing` | `pico8_lq_t *line` (`:109-116`) | 3b8ba2c : `pico8_layout_truthy` ne rejetait que `false` ; un item nil devenait le sprite #0 | Retirer la clause REF-NULL de `pico8_layout_truthy` | `[nil]` : `hits == 0` faux (sprite #0 dessiné en `(DRAW_X, DRAW_Y+7)`). **Première preuve de ce test dans le plan 3** |
  | `planet_gh_row_lq` | `button_lq` → `const pico8_lq_t *` ; descripteur de `lq` (`:122`) ; `items` aux index 1-5 (`:65,135-142`) ; compte de `fs` par le membre ou `pico8_entity_get_field` (`:145`) | 8361468 : cast aveugle de la liste `lq` d'une ligne en `ob_table` dans `js:pq` | Cas `LIST` de `pico8_js_lq_arg_table` remplacé par `return (const pico8_ob_table_t *)v.as.ref;` | Rapport ASan heap-buffer-overflow ; à défaut, l'assert de clé du constructeur, ou les asserts de champ. Noter lequel |
  | `ga_export_panel_by` | `button_lq` et `lq_index_string` sur `items` (`:43-58`) | 9183fee : `by=o()` de `ga` restait une `ob_table` lue comme liste | Retirer l'échange de `by` en liste dans l'init de `ga` (`planet_family.c:1085-1098`) | Abort dans `PICO8_REF_AS_LIST_CONST` pendant `jo:gh`, ou à l'assertion de tag `LIST` du test. Noter lequel |
  | `rich_planet_exports` | premier positionnel par `items` (`:92-97`) | 50f91b4 : `bb_visit` bornait l'index de production par le compte total de `po` (clé `mc` comptée) | `po_len` → `po->count` dans `bb_visit` (`economy_tiers.c:230`) | `mu4 == 3` faux |
  | `built_planet_panel_nil_row` | `row_first_item_string` sur `items` (`:84-102`) | 3b8ba2c (moitié msgbox) : un `ep` nil faisait préfixer une ligne fantôme | Preuve à deux changements de 1a.17/1a.31 : (i) le lecteur du descripteur de `jo` rend « présent » un `ep`/`fo`/`nk` nil ; (ii) clause REF-NULL de `pico8_msgbox_truthy` retirée. Contrôles (i) seul, (ii) seul → vert | `built_rows == BUILT_PANEL_EXPECTED_ROWS` faux |

- **Caractérisations adaptées, valeurs figées inchangées** (précédent `in_gb`, 1a.33). Chacune est lancée une fois sur l'ancien code (copie `git archive` jetable) avec son pli adapté, verte, et le commit le note.
  - `test_pico8_js` :
    - `fold_built` plie le tag `INSTANCE` pour une struct au descripteur de `lq` (`:265`) ;
    - `fold_lq` lit `items` avec l'index `i+1` (`:245-251`) et vérifie le descripteur au lieu de `cls` (`:243`) ;
    - `lq_first` lit `items[0]` (`:793`) ;
    - la vérification de forme de `h_` (`:954`) exige le descripteur de `lq`.
  - `test_pico8_pj` : `fold_lq` (`:518-531`) et `fold_cr_le` (`:551`) lisent `items`, index `i+1` ; assert de forme au descripteur (`:522`).
  - `test_pico8_ga` : `fold_lq` plie la vue en table de la rangée (`:461-466`), même `fold_table`, puis la libère.
  - `test_pico8_hp` : `fold_lq_instance` plie la vue en table (`:208-213`), même `fold_value`, puis la libère.
  - Si l'une de ces valeurs ne peut pas être rendue à l'identique, c'est l'arrêt « valeur figée changée », pas une nouvelle valeur.
- **Tests existants à diff vide**, verts :
  - `test_pico8_jb_ol`, `score_panel_stars`, `game_end`, `score_panel_close`, `pause_menu_music`, `tech_screen`, `top_menubar`, `shop_tooltip_fillp` ;
  - les caractérisations de planète (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`) ;
  - toutes les autres.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` `test_pico8_lq`, `test_pico8_js`, `test_pico8_hp`, `test_pico8_layout_nil_item_draws_nothing` et `test_pico8_score_panel_stars`, avec les objets du moteur. Les lancer, puis `gcov` sur `pico8_layout.c`, `pico8_research_display.c`, `pico8_planet_display.c`, `pico8_ui_button.c`, `pico8_menu.c`, `pico8_score_panel_display.c`, `pico8_pi.c`.
    - Même piège qu'à 1a.23-1a.33 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`.
    - Les branches d'assert atteintes seulement dans un fils qui avorte ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu.
    - Le rapport est collé dans le message de commit : lignes exécutables ajoutées ou modifiées de `src/` et leur compte. Il couvre :
      - le constructeur, dont `c`/`bg`/`score`/`ra`/`fs`/`w` reçus et les surcharges NULL ;
      - le lecteur ; la vue en table, dont les ajouts après `pq` ;
      - `mn`/`pq`/`ol` ; `jo.ex` dont `jq>5` ; `hp:lq` ;
      - `js:pq`/`js:kj` (copie, `h_`, `bg`) ; `ix[2].hb`, `pj:ph`, `ld`.

      Les lignes à 0 sont données avec leur justification.
  - **2 à 3 mutations**, chacune rouge sur un test nommé, notées dans le commit avec le test qui l'attrape :
    1. **entrée du descripteur** : `OL` retiré, ou remplacé par le dispatcher de `jb`. `mv(lq,"ol")` ne dessine plus la rangée ; scénario 4 de `test_pico8_lq` rouge (et l'assert de `jo:kj`) ;
    2. **ordre du constructeur** : `pq` avant la copie de `c`. Le texte coloré (`c=13`) passe en couleur 6 ; scénario 1 rouge ;
    3. **forme du membre** : `items` rempli dans l'ordre de stockage au lieu de l'ordre d'index. La rangée des étoiles s'inverse ; `score_panel_stars` et le scénario 1 (désordre) rouges.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`. Les binaires `*_refaudit` compilent, dont `planet_gh_row_lq` et `ga_export_panel_by`.
- **`make float-audit`** : vert, `tools/float_audit_c1.txt` inchangé.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** :
  - `test_pico8_hp` et `test_pico8_trade` : structure, `lookup`, `must_abort`, fils par scénario, rejeu court, forme ;
  - `test_pico8_layout_nil_item_draws_nothing` : construire une `lq` réelle et chercher des pixels ;
  - `test_pico8_jb_ol` : `ol` sur un écran effacé ;
  - `test_pico8_mh` (1a.4) : positionnels en membre liste ;
  - le commit 75360a7 (1a.33) : pli adapté sans changer la valeur figée (`in_gb`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; c'est le seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` (en particulier `lq  10718  15/15`). Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier :
  - si un membre autre que `items`, `c`, `bg`, `score`, `ra`, `fs`, `w` est ajouté ;
  - si les positionnels ne sont pas un membre liste, ou pas dans l'ordre d'index croissant ;
  - si le constructeur accepte une autre clé nommée sans avorter, ou si son assertion se déclenche sur un rejeu ou un test hors `must_abort` ;
  - si un écrivain est posé, ou une entrée de méthode autre que `OL` ;
  - si l'enum change.
- `FAILED:` si un site hors de `pico8_class.c` passe une `lq` à `pico8_instance_get_field`, `pico8_instance_fields` ou `->fields` (grep dans `src/` et `tests/`), si `pico8_layout.c` appelle encore `pico8_class_instantiate` ou `pico8_class_set_pq` pour `lq`, ou si un site lit ou écrit `g_lq_class->defaults`/`pico8_lq_class()->defaults` (sauf la vérification de forme du test).
- `FAILED:` si `hp:lq` recalcule `fs` ou `w` après ses deux ajouts, ou si la copie de `js:pq` ne les voit plus (valeur figée de `test_pico8_lq`, scénario 3).
- `FAILED:` si `pico8_lq_ol` accepte un pointeur non NULL qui n'est pas une rangée sans avorter.
- `FAILED:` si une autre classe que `lq` est convertie ou change de forme (`hy`, `fm`, les enveloppes, `jb`…), si la classe `lq` cesse d'être déclarée et enregistrée à la même place, ou si le comportement d'une autre classe change (valeur figée de sa caractérisation, ligne de couverture).
- `FAILED:` si l'un des cinq tests de régression (`layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`, `rich_planet_exports`, `built_planet_panel_nil_row`) change au-delà de sa façon de lire la rangée (assertion retirée ou affaiblie, valeur attendue changée), si sa preuve rouge (bug remis, échec observé, correctif restauré, vert) manque au message de commit, ou si un bug remis ne le rend pas rouge. Également si un autre test de régression (§6) a un diff non vide.
- `FAILED:` si une valeur figée de `test_pico8_lq` a été changée après la conversion, ou une de `test_pico8_js`, `pj`, `ga`, `hp` ou d'une autre caractérisation ; ou si les plis adaptés de ces quatre tests n'ont pas été vérifiés verts sur l'ancien code (noté dans le commit).
- `FAILED:` si une ligne est ajoutée à `tools/float_audit_c1.txt` ou en est retirée, ou si `make float-audit` est rouge.
- `FAILED:` si `test_pico8_lq` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé : en particulier si la rangée à trou de `pj:ph` ne dessine plus ses deux éléments, ou si `TROU-25` manque (entrée et référence au site).
- `FAILED:` si la note « Découvert à 1a.34 » manque à la ligne 1a.34 de `docs/plan-3.md`.

Critères de réussite (en plus) :

- la struct `lq` n'a que l'en-tête, `items` et les 6 membres ; son descripteur a `OL`, un lecteur, pas d'écrivain ;
- grep :
  - `PICO8_CLASS_COVERAGE_NAME(g_lq_class, "lq")` reste ;
  - `pico8_class_set_struct_new(g_lq_class, …)` est présent ;
  - `pico8_class_set_pq(g_lq_class` et `->defaults, "ol"` sont absents ;
  - aucun `pico8_instance_get_field`/`pico8_instance_fields` dans `pico8_layout.c`, ni sur une rangée dans `pico8_research_display.c` et `pico8_ui_button.c` ;
  - `pico8_lq_new`, `pico8_jo_ex`, `pico8_hp_lq` rendent `pico8_lq_t *` ;
- le message de commit dit « découvert à 1a.34 ».

## Out of Scope

- Les autres classes et instances sans classe : `hy` (1a.35), `fm` (1a.36), les `{}`/`et={}`/`{qs}`/`re` jetables (1a.37-1a.40), l'enveloppe de `pz` sur `g_ha` (1a.41). `jb` (déjà struct) ne change que le type de ses éléments.
- `lq` du projet `hp` (la table `{px,32,{ii,8}}` des 4 projets à `ii`) : membre de `hp`, inchangé.
- Les tables de ligne `{lq=…,h_=…}` de `pz`, les listes `lq` de `jo:gh` : ce sont des surcharges, pas des rangées.
- Typer les membres (`fs` en liste, `c` en nombre, `bg` en nombre ou booléen) : phase 1b.
- Corriger TROU-25 ou TROU-13 ; libérer une rangée (TROU-7, Plan 2 §8).
- Supprimer la classe `lq`, les registres et `pico8_class.c` : 1a.42.
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Racine, pas de parente à fermer** : aucune fonction partagée ne passe à la forme struct seule ici ; aucune double forme ouverte ne se solde.
- **Ce que le filet voit des rangées** : 10 718 constructions sur 15 rejeux, 1,7 million d'`ol`, 649 000 `mv(…,"ol")` depuis `jo:kj`, 462 copies d'une rangée de `hp:lq`, 4 rangées à trou. Il ne voit pas : `jo.ex` à `jq>5`, `lq(nil)`, un item nil. La caractérisation dirigée et les mutations visent ces cas.
- **Écarts avec la ligne du plan** :
  - `layout.c:354` est `:394` à HEAD (déclaration de `g_lq_class`) ;
  - la ligne ne nomme pas `built_planet_panel_nil_row`, qui lit aussi une rangée (décision ci-dessus).
- Mesures :
  - grep sur `src/`, `tests/`, `tools/` et le `Makefile` à c452539 ;
  - lecture de `pico8/lua.lua:15-16,86-88,261-315,530-548,885-891,982,1409-1413` ;
  - lecture de `pico8_layout.c/h`, `pico8_research_display.c/h` (`jo.ex`, `hp:lq`), `pico8_planet_display.c` (`jo:hb`, `jo:kj`), `pico8_ui_button.c` (`js:pq`, `js:kj`, `pico8_js_lq_arg_table`), `pico8_menu.c` (`ix[2].hb`), `pico8_score_panel_display.c` (`pj:ph`), `pico8_pi.c` (`ld`), `pico8_struct.h`, `pico8_type_tag.h` ;
  - lecture des tests nommés et de `test_pico8_js`, `pj`, `ga`, `hp` ;
  - lecture des PRD de 1a.1, 1a.4, 1a.16, 1a.17, 1a.26, 1a.31, 1a.33 et du commit 75360a7 ; `git log -S` sur `test_pico8_layout_nil_item_draws_nothing` (3b8ba2c) ;
  - sonde dans une copie jetable du scratchpad, supprimée après usage :
    - 18 rejeux en sortie 0 ; 10 821 constructions, toutes par les six appelants ci-dessus, toutes aux clés mesurées ;
    - lectures `c`/`w`/`fs`/`bg` seulement ; aucune écriture après `pq` hors des deux ajouts de `hp:lq` ;
    - `make -k unit` et `UNIT_TESTS_SLOW` : 52 907 constructions, mêmes six appelants, mêmes clés ; prises de tag d'une rangée seulement dans `test_pico8_js`, `pj`, `ga`.
