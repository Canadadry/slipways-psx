---
title: "Plan 3, étape 1a.14 : `ie` (l'icône de recherche) devient une struct, `ie.gy`/`ie.id` deviennent un état statique de la classe"
description: "L'icône de recherche `ie` est encore une instance de la couche objet, et le cart mute deux de ses défauts sur la classe (`ie.gy`, le curseur de placement ; `ie.id`, le compteur de noms). Elle passe en struct fille de `ke`, avec ses méthodes `hd` (= `gk`), `hb`, `kj` et `ldown` (l'enum gagne `HD` et `HB`), `kq`/`ra`/`eh` possédés par le module, `gy`/`id` en état statique partagé, et un constructeur qui n'accepte que `ke` et `jb`. `ie:ldown` continue d'instancier la classe-valeur `self.ke` par `pico8_class_instantiate`, et l'enveloppe `ie.ke` (1a.15) reste telle quelle."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.14 — `ie` (`research_icon.c:281`), `gy`/`id` → état statique (`research_icon.c:158,182`, `cart.c:158`), instancie `ie.ke` (`:198`). Tests : aucun.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Défauts mutés sur la classe** : `ie.gy`, `ie.id` (`pico8_research_icon.c:158,182`, `pico8_cart.c:158`) deviennent un état statique partagé de la classe, jamais un champ d'instance. […]
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe. […]
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode) […].

§6 : la ligne ne nomme aucun test de régression, et la mesure n'en trouve aucun qui lise un champ d'`ie` (grep de `"gy"`, `"kh"`, `"eh"`, `"kq"`, `research_icon`, `pico8_ie`, et des lectures de `ok.o_` dans `tests/`). Aucun test de régression n'est réécrit. §7 : le commit passe `make class-coverage-net` avec une sortie identique à `tools/class_coverage_net_expected.txt` (`ie  133  15/15`). Un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la ligne de l'étape dans le commit, avec « découvert à 1a.14 » dans le message.

### Ce que 1a.1 à 1a.13 ont posé (commits 9ce8bfa, 42fdf50, f5cc6d1, 28a08ac, 824725c, ec01e86, 470accc, 9aa4812, 968e059, 3f62fe5, 3cc50d7, e08cfb1), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t` et le descripteur `pico8_struct_class_t` : des méthodes indexées par l'enum (aujourd'hui `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`), un lecteur de champ qui rend 1 si le champ n'est pas nil, et un écrivain de champ. Un nom absent de l'enum, ou dont l'entrée est NULL, est cherché comme champ.
- `pico8_ke_t` en premier membre, avec `pico8_ke_struct_init`, `pico8_ke_field` et `pico8_ke_struct_pq`.
- `pico8_class_set_struct_new` : `pico8_class_instantiate` délègue au constructeur de struct, qui enregistre la couverture.
- `pico8_entity_get_field` et `pico8_entity_set_field` sont à double forme. L'écrivain asserte s'il n'y a pas d'écrivain ou si le nom n'a pas de membre. `pico8_instance_get_field` asserte s'il reçoit une struct.
- Déjà à double forme et atteints par un `ie` : `lt`/`he` (présence des noms `bm`, `nu`), `dg` (`me`, `t`, `p_`, `mv(me)`), `dc` (`fa`, `ra`, `mv(nt)`), `mv`, la boucle de survol `gw` de `ui.c` (`mv(e,"hd",qa,je.ra)`, `fa`, écriture de `m_` sur `o_` et `gj`, lecture de `fg` sur `o_`), `ui:ev` (`ql` sur `o_`), `jz` (`mv(h,"ldown"|"rdown"|"gv")`), la boucle `cv.hb` de `hy:lj` (`economy.c`, `mv(e,"hb")`), `jc:hq` (`ok.o_.gv`, 1a.5), le scan `fx` de `mz:pq` sur `hv` (1a.10), et les lectures `fa`/`ra` de `gk`.
- **Modèles** :
  - Défaut muté → état statique : `nf` (1a.6), dont les tables `l`/`r` ne sont pas des membres et que le lecteur de champ rend depuis l'état du module.
  - Défauts non mutés possédés par le module et pointés par un membre : `pl` de `bg` (1a.12), `nu`/`k_` de `jc` (1a.13), construits depuis le même fragment de spec.
  - Constructeur à clés mesurées qui asserte sur toute autre : `iv` (1a.2), `jc` (1a.13).
  - Classe sans `pq` propre : `fq` (1a.5), dont le constructeur appelle seulement `pico8_ke_struct_pq`.
  - `hd` sur une struct : `fq`/`bg` portent `hd=1`, un membre lu comme champ.
- Modèle de test : `test_pico8_jc` et `test_pico8_bg`. La caractérisation est figée sur l'ancien code et lue seulement par l'API à double forme. Les scénarios tournent dans un fils forké juste après `pico8_cart_load()` (`in_fresh_child`). La vérification de forme s'ajoute après la conversion, et l'écriture interdite est vérifiée dans un fils. Les mutations sont notées dans le commit.

### Ce que fait le cart

`pico8/lua.lua` :

```
:919-925  ie=ke:jr([[ kq=b(0,0,9,8), fa=12, id=0, ra=v(999,999), eh=v(118,120), ]])ie.hd=gk
:926-931  function ie:hb() if not self.kh and oj(self.ke.rr) then
            self.ra,self.kh = ie.gy,true   ie.gy-=v(10,0) end end
:932-935  function ie:ldown(mp) ie.id+=1 return self.ke({id="s"..ie.id}) end
:936-941  function ie:kj(p) if self.kh then palt(3,false) spr(self.ke.s,p.x,p.y)
            if self.m_ then dl(self.ke) self.jb:ol(0,0) end palt(3,true) end end
:209-211  function gk(e,mp,rp) return e.kq((e.fa>=10 and rp or mp)-e.ra) end
:1178-1180 function _init() cu() jl() ie.gy,kc = ie.eh  fv() music(0) end
:1181-1186 function eu(nj) … local gx={qh,os,nr,fr,nl,mx,hj} for i,b in pairs(gx) do ie({ke=b,jb=jb(dr[i])}) end fm:lj() end
:1236-1237 hy:lj : for e in all(cv.hb) do e:hb() end
:156-161  lt : add(hv,e) for p in all(bm) do if (e[p]) ft(cv,p,e) end …   bm = "kj","ev","hd","hk","hb"
```

Points décisifs :

- **`gy` et `id` sont mutés sur la classe.** `ie.gy` est posé par `_init` (`ie.gy = ie.eh`, alias du même vecteur, pas une copie), relu et réaffecté par chaque `ie:hb` (`ie.gy -= v(10,0)` rebinde vers un vecteur neuf ; l'ancien, capturé dans `self.ra`, n'est jamais modifié). `ie.id` est incrémenté par chaque `ie:ldown`. Tous deux sont **partagés par toutes les icônes**, et **survivent à `eu()`** (nouvelle partie). Seul `_init` remet `gy` (à `eh`). `id` n'est jamais remis, sauf par la redéclaration de la classe (`pico8_research_icon_init`, donc `pico8_cart_load`). → état statique partagé de la classe, jamais un membre.
- **`kq`, `ra`, `eh` sont des défauts jamais mutés**, ni réaffectés sur la classe, ni modifiés en place. `kq` (la boîte `b(0,0,9,8)`) est lu par `gk`. Le vecteur `ra=v(999,999)` est lu tant que l'icône n'est pas révélée : `gk` rate toujours, et `dc` le passe à `kj`, qui ne dessine rien. `eh` n'est lu que sur la classe, par `_init`. → membres qui pointent vers des objets possédés par le module, comme `pl` de `bg`.
- **`hd` est une méthode** (`ie.hd=gk`), pas la valeur `1` de `fq`/`bg`. Elle est appelée par nom de chaîne (`mv(e,"hd",…)` dans `gw`) et doit rendre `ie` présent dans `cv.hd` au `lt`. **`hb` aussi** (boucle `cv.hb` de `hy:lj`, présence au `lt`). Elles entrent donc dans l'enum. `kj` (`dc`) et `ldown` (`jz`) y sont déjà.
- **`ie` n'a pas de `nu`** et n'hérite que de `ke` (`me="ph"`, `t=0`, `pq=lt`). Il est dans `hv`, `cv.kj`, `cv.hd`, `cv.hb`, jamais dans `cv.ev`/`cv.hk`, ni dans aucun `cg`.
- **`self.ke` est une classe-valeur** (une des sept de `gx[]`), pas une instance. `ie:ldown` l'instancie par `pico8_class_instantiate` avec `{id="s"..ie.id}`. `qh`, `os`, `nr`, `fr`, `nl`, `mx`, `hj` seront convertis plus tard (1a.18 à 1a.25) : la délégation `struct_new` suffira, `ie:ldown` ne change pas alors.
- **`self.ke.rr` et `self.ke.s`**, et `dl(self.ke)`, passent par l'enveloppe `ie.ke` (instance sans classe à pile, `cls` = la classe tenue, `fields` vide). Cette enveloppe est l'objet de 1a.15. Elle reste ici **à l'identique**. Seule change la façon dont `ie` lit sa propre `ke` : par le membre, et non plus par `pico8_instance_get_field`.
- **`self.jb` est déjà une struct `jb`** (1a.1), construite par `eu` et lue par `ie:kj` avec l'assert existant `STRUCT`.

### Emprise réelle de `ie` (mesurée sur e08cfb1)

| Aspect | Mesure |
|---|---|
| Constructions | Un seul site en jeu : `pico8_eu` (`cart.c:246-250`), sept par partie, par `pico8_ie_new({ke=gx[i], jb=jb})`. `pico8_ie_new` fait `pico8_class_instantiate(g_ie_class, …)` et n'a pas d'autre appelant, ni dans `src/` ni dans `tests/`. Aucun `fz`, `gx[]`, `lookup_named` ni `kv[]` ne construit d'`ie`. La classe est déclarée par `pico8_research_icon_init` (`research_icon.c:281`), avec l'assert `pico8_ie_class()->parent == pico8_ke_class()` (`cart.c:101`, reste vrai). |
| Défauts | `kq`=boîte `b(0,0,9,8)`, `fa`=12, `id`=0, `ra`=REF `v(999,999)`, `eh`=REF `v(118,120)`, plus les méthodes `hd`, `hb`, `kj`, `ldown`. Hérités de `ke` : `me`=`"ph"`, `t`=0. Défauts mutés sur la classe : `id` (`ie:ldown`) et `gy` (absent du spec, posé par `_init`, puis réaffecté par `ie:hb`). |
| Champs écrits sur l'instance | `ke`, `jb` (table de construction) ; `ra`, `kh` (`ie:hb`) ; `m_` (boucle de survol `gw`, par `pico8_entity_set_field`, sur `o_` et `gj`). Aucun autre nom n'est écrit sur un `ie`, ni dans le cart ni dans `src/`. `ie:hb` asserte aujourd'hui qu'aucun `gy` propre n'existe sur l'instance (`research_icon.c:143`). |
| Méthodes | `hd` (= `gk`), `hb`, `kj`, `ldown`, **installées aujourd'hui par `->defaults`** (`research_icon.c:285-288`). Dans l'enum, `KJ` et `LDOWN` existent déjà. **L'enum gagne `HD` et `HB`.** `me`=`"ph"` fait appeler `mv(ie,"ph")` par `dg` : l'entrée `PH` est NULL sur `ie`, le lecteur rend absent comme aujourd'hui. |
| `->defaults` | 9 sites. Dans `research_icon.c` : `:90` (lecture de `gy`/`id` par `ie_class_field_value`), `:158` (écriture de `gy`), `:182` (écriture de `id`), `:285-288` (4 installations de méthode). Dans `cart.c` : `:157` (lecture de `eh`) et `:158` (écriture de `gy`). Aucun autre `g_ie_class->defaults` ni `pico8_ie_class()->defaults` dans `src/`, `tests/` ou `tools/`. |
| Lecteurs à double forme qui atteignent un `ie` | `lt`/`he` (`kj`, `ev`, `hd`, `hk`, `hb`, `nu`) ; `dg` (`me`, `t`, `p_`, `mv(ie,"ph")`) ; `dc("kj")` (`fa`, `ra`, `mv(ie,"kj",ra)`) ; `dc("ev")` n'atteint pas `ie`. Boucle `gw` (`mv(ie,"hd",qa,je.ra)`, `fa`, `fg`). `ui:ev` (`ql`). `jz` (`mv(ie,"ldown")` rend l'instance neuve, `rdown`/`gv` absents). `hy:lj` (`mv(ie,"hb")`). `jc:hq` (`ok.o_.gv`, absent). `mz:pq` (`fx`, absent). `gk` pour `fa` et `ra`. Tests : `find_bg` (`test_pico8_bg`) et `find_veil` (`test_pico8_fq`) lisent `hd`/`fa` sur chaque entrée de `cv.hd`, par l'API double ; sur un `ie`, `hd` rendra une fonction, donc le filtre `FIXED == 1` l'écarte comme aujourd'hui. |
| **Lecteur à simple forme qui atteint un `ie`** (asserte après conversion) | `gk` (`entity_geom.c:46`) lit `kq` par `pico8_instance_get_field`. `gk` est le `hd` de `ie`, `jo`, `fv` et `js`, et il est appelé sur chaque `ie` de `cv.hd` à chaque frame. **Site découvert** (règle « Coexistence ») : il passe à `pico8_entity_get_field`. Toutes les lectures de `research_icon.c` sur `self` (`kh`, `ke`, `m_`, `jb`) passent aujourd'hui par `pico8_instance_get_field`. |
| Écrivains à simple forme qui atteignent un `ie` | Les écritures `self->fields` de `ie:hb` (`ra`, `kh`), hors bornes sur une struct. Aucun autre. |
| Casts et asserts de tag | `pico8_ie_*_dispatch` castent `self` en `pico8_instance_t *`. `pico8_ie_hd_dispatch` le passe à `gk` en `const pico8_instance_t *` sans le déréférencer (`gk` lit par l'API). `economy.c:277` et `ui_jz` castent sans déréférencer. `dc`/`dg` acceptent déjà les deux tags. L'assert `UNKNOWN` sur la valeur de `ke` (`research_icon.c:56`) et l'assert `STRUCT` sur `jb` (`:247`) restent vrais. Aucune macro de `pico8_ref_audit.h` ne reçoit un `ie`. |
| Clés dynamiques | Aucune sur `ie`. `bt.."down"` est une clé de méthode (`ldown`), déjà dans l'enum. |
| `->cls` lu sur une entrée de `hv` | `test_pico8_geoharvest_burst.c:85` (`live_ga`, sans vérifier le tag). Les `ie` sont dans `hv` dans les 15 rejeux. La lecture reste dans les bornes : la struct `ie` est plus grande que l'en-tête d'instance. Elle n'est jamais égale à la classe `ga`, et jamais déréférencée. Même situation que `bg`/`mz`/`jc`, réécriture prévue en 1a.26. |
| Tests qui atteignent un `ie` | Tout test qui lance `eu()` construit sept `ie`, et sa boucle `gw` les survole. Aucun ne lit un champ d'`ie` en dehors de `find_bg`/`find_veil` (double forme). Aucun test de régression à réécrire. |
| Filet | Les 15 rejeux construisent des `ie` (`ie  133  15/15`) : révélation (`hb`), dessin (`kj`), survol (`gk`, `m_`, `dl(self.ke)`, `jb:ol`) et `ldown` (tous les bâtiments posés depuis la barre de recherche) sont vus par le filet. |
| Couverture | `ie  133  15/15` (inchangé attendu). Les classes que `ie:ldown` instancie (`qh 8`, `os 1`, `nr 6`, `fr 5`, `nl 1`, `mx 3`, `hj 5`) sont inchangées, par le même chemin. |

## Solution

1. **Struct `ie`** (publique dans `pico8_research_icon.h`, comme `pico8_jc_t`) : premier membre `pico8_ke_t`, puis les défauts non mutés de la chaîne `kq`, `fa`, `ra`, `eh`, puis les champs écrits sur instance `ke`, `jb`, `kh`, `m_`. Tous sont des `pico8_value_t` et gardent leur nom minifié. `me`/`t` sont ceux de `pico8_ke_t`. **Ni `gy` ni `id` ne sont des membres.**
2. **État statique partagé de la classe** (module `research_icon`) : `gy` et `id`, en `pico8_value_t`.
   - `pico8_research_icon_init` les remet à nil et à 0, comme le faisait la redéclaration de la classe.
   - `pico8_research_icon_shutdown` remet `gy` à nil (il peut pointer vers `eh`, libéré ensuite).
   - `ie:hb` lit `gy`, le copie dans `self.ra` (même REF), puis rebinde `gy` vers un vecteur neuf `gy - v(10,0)`. L'ancien n'est jamais modifié, et le neuf n'est jamais libéré (TROU-7, comme aujourd'hui).
   - `ie:ldown` incrémente `id` avant de lire `ke`, même si `ke` est absent, comme aujourd'hui.
3. **Objets possédés par le module** : la boîte `kq` et les vecteurs `ra` (999,999) et `eh` (118,120), construits par `pico8_research_icon_init` depuis le même fragment de spec (après `pico8_pi_init`), et libérés par le shutdown. Le membre de chaque struct est une REF vers l'objet du module : toutes les icônes non révélées partagent le même `ra`, comme elles partageaient le défaut de classe.
4. **Accès pour `_init`** (`cart.c:157-158`) : deux fonctions du module, l'une qui rend `ie.eh` (la REF du module), l'autre qui pose `ie.gy`. `pico8__init` fait `gy = eh` par elles, à la même place, avant `pico8_msgbox_set_kc(NULL)`. Le commentaire existant (alias, pas copie) reste vrai et reste.
5. **Valeurs initiales** : après `pico8_ke_struct_init`, `me`=`"ph"`, `t`=0 ; `fa`=12, `kq`/`ra`/`eh` = REF vers les objets du module ; `ke`, `jb`, `kh`, `m_` à nil.
6. **Constructeur** (posé comme `struct_new` de la `pico8_class_t` d'`ie`), dans l'ordre :
   1. alloue ;
   2. appelle `pico8_ke_struct_init` avec le descripteur d'`ie` ;
   3. pose les valeurs initiales ;
   4. copie les clés nommées `ke` et `jb` de la table reçue dans leurs membres, et **asserte** (message porteur, arrêt §7) sur toute autre clé ;
   5. enregistre `"ie"` au crochet de couverture ;
   6. appelle `pico8_ke_struct_pq` (`ie` n'a pas de `pq` propre, modèle `fq`).
7. **Enum de méthodes** : il gagne `HD` (`"hd"`) et `HB` (`"hb"`), et rien d'autre. Sur `fq` et `bg`, l'entrée `HD` est NULL : `hd` retombe sur leur lecteur de champ, qui rend `1` comme aujourd'hui. Sur tout autre descripteur, `HD`/`HB` sont NULL et le nom est cherché comme champ, donc absent comme aujourd'hui.
8. **Descripteur d'`ie`** : méthodes `HD` (le dispatch vers `gk`), `HB`, `KJ`, `LDOWN`.
   - **Lecteur** : il connaît les 8 membres, rend `gy` et `id` **depuis l'état statique** (modèle `l`/`r` de `nf`, pour qu'une lecture générique sur une icône rende ce que la chaîne rendait), puis délègue à `pico8_ke_field`.
   - **Écrivain** : il n'accepte que `m_` (boucle `gw`) et rend 0 pour tout autre nom. L'assert de `pico8_entity_set_field` part alors.
   - **Accesseur** public du descripteur, comme `pico8_jc_struct_class()`.
9. **Méthodes sur la struct** : `pico8_ie_hb`, `pico8_ie_ldown` et `pico8_ie_kj` prennent la struct `ie`. Elles gardent leurs gardes NULL, leurs asserts porteurs sur l'init, et les TROU existants.
   - Ce qu'elles lisent et écrivent sur `self` (`kh`, `ke`, `m_`, `jb`, `ra`) passe par les membres.
   - `gy` et `id` passent par l'état statique. `ie_class_field_value`/`_fixed`/`_vec` disparaissent. L'assert « pas de `gy` propre » disparaît aussi : la struct n'a pas de membre `gy`, l'invariant est tenu par sa forme. Son contrat passe dans le commentaire de l'état statique.
   - `ie_self_ke_class` lit le membre `ke`, et garde son assert `UNKNOWN` (« `ie.ke` tient une classe »).
   - **L'enveloppe `ie.ke` ne change pas** (1a.15) : `ie_ke_wrapper`, `ie_ke_field` et `g_ie_empty_fields` construisent toujours une instance sans classe à pile, `cls` = la classe tenue, pour lire `rr`/`s` et pour `dl(self.ke)`.
   - `ie:ldown` construit toujours `{id="s"..id}` et appelle `pico8_class_instantiate(ke_cls, …)`. Rien ne change dans l'instanciation de `ie.ke`.
10. **Site hors du module, découvert par la mesure** : `gk` (`entity_geom.c`) lit `kq` par `pico8_entity_get_field`. Ses lectures de `fa`/`ra` sont déjà à double forme. `jo`, `fv` et `js`, encore instances, lisent le même `kq` qu'avant.
11. **`pico8_research_icon_init` / `_shutdown`** : même déclaration, même spec, même parent, même nom de couverture. La classe reçoit le constructeur et perd les 4 installations `->defaults`. Le shutdown libère la classe, puis la boîte et les deux vecteurs du module. Il remet `gy`/`id`, et ne libère jamais un `ie`.
12. **`pico8_ie_new`** rend la struct (comme `pico8_jc_new`), en gardant le même paramètre. `pico8_eu` n'en garde pas le résultat, donc son code ne change pas.
13. **Sans changement** : `pico8_eu`, `ui.c` (`gw`, `bh`, `jz`, `ev`), `economy.c` (`hy:lj`), `dg`, `lt`/`he`/`dc`/`mv`, `dl`, `jb`, les sept classes de `gx[]`.
14. **Plan** : ajouter à la ligne 1a.14 de `docs/plan-3.md` le texte suivant.

    > Découvert à 1a.14 : `hd` (= `gk`) et `hb` entrent dans l'enum (`mv(e,"hd")` dans `gw`, `mv(e,"hb")` dans `hy:lj`, présence au `lt`) ; lecteur à simple forme atteint par un `ie` : `kq` dans `gk` (`entity_geom.c:46`) ; `m_` écrit par `gw` → écrivain.

    Aucun défaut muté ne s'ajoute aux défauts connus du §5 (`gy`, `id` y sont déjà).

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux qu'`ie` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise d'`ie` soit mesurée dans le PRD (constructions, défauts, champs écrits, méthodes, `->defaults`, lecteurs et écrivains génériques, tests, filet), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux que la struct `ie` n'ait que `pico8_ke_t` et les 8 membres `kq`, `fa`, `ra`, `eh`, `ke`, `jb`, `kh`, `m_`, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux que `gy` et `id` soient un état statique partagé du module et jamais des membres, afin d'appliquer la règle des défauts mutés du §5 telle qu'elle nomme `ie`.
5. En tant que gardien du filet, je veux que chaque `ie:hb` qui révèle une icône lui donne la valeur courante de `gy`, puis décale `gy` de 10 px vers la gauche pour **toutes** les icônes, afin que la barre de recherche se remplisse de droite à gauche comme avant.
6. En tant que gardien du filet, je veux que la première icône révélée après `_init` tienne en `ra` la même REF que `eh`, afin que l'alias `ie.gy = ie.eh` soit préservé.
7. En tant que gardien du filet, je veux que `gy` survive à `eu()` et ne soit remis à `eh` que par `_init`, afin qu'un « nouvelle partie » depuis le menu garde le comportement du cart.
8. En tant que gardien du filet, je veux que `id` soit incrémenté par chaque `ie:ldown`, quelle que soit l'icône, et jamais remis par `eu()` ni `_init`, afin que les noms `"s<n>"` des bâtiments posés restent identiques.
9. En tant que gardien du filet, je veux qu'`ie:ldown` instancie toujours la classe-valeur tenue en `ke` par `pico8_class_instantiate`, avec la seule clé `id`, afin que les bâtiments posés soient construits par le même chemin et comptés à l'identique par la couverture.
10. En tant qu'auteur des étapes 1a.18 à 1a.25, je veux qu'`ie:ldown` ignore la forme de ce qu'il instancie, afin que convertir `qh`, `nl`, `os`, `hj`, `mx`, `nr` ou `fr` ne touche pas `research_icon.c`.
11. En tant qu'auteur de 1a.15, je veux que l'enveloppe `ie.ke` (instance sans classe à pile, `cls` non nul) reste construite et lue à l'identique, afin que sa conversion soit sa propre étape.
12. En tant que gardien du filet, je veux qu'une icône non révélée garde `ra` = (999,999), partagé, et ne dessine rien, afin que `gk` la rate et que `dc("kj")` ne la montre pas.
13. En tant que gardien du filet, je veux qu'un `ie` neuf soit enregistré dans `hv`, `cv.kj`, `cv.hd` et `cv.hb`, dans cet ordre relatif aux autres entités, et dans aucun de `cv.ev`/`cv.hk` ni aucun `cg`, afin que l'ordre de dessin, de survol et de révélation ne bouge pas.
14. En tant que gardien du filet, je veux que `mv(ie,"hd",qa,je.ra)` appelle `gk` et teste la boîte `kq` autour de `ra` en coordonnées écran (`fa`=12 ≥ 10), afin que le survol des icônes soit identique.
15. En tant que gardien du filet, je veux que `gk` lise `kq` par l'API double, afin qu'il serve `ie` (struct) et `jo`/`fv`/`js` (instances) sans assert.
16. En tant que gardien du filet, je veux que la boucle `gw` puisse écrire `m_` sur un `ie` survolé puis quitté, afin que l'infobulle (`dl(self.ke)` et `jb:ol`) apparaisse et disparaisse comme avant.
17. En tant que gardien du filet, je veux que `mv(ie,"hb")` depuis `hy:lj` ne révèle une icône qu'une fois (`kh`), et seulement quand `oj(self.ke.rr)` est vrai, afin que la progression de la recherche soit identique.
18. En tant que gardien du filet, je veux que `ie:kj` dessine le sprite `self.ke.s` à `ra`, avec la transparence 3 levée puis remise, afin que le rendu soit identique au pixel près.
19. En tant que gardien du filet, je veux que `dg` appelle `mv(ie,"ph")` sans rien trouver et incrémente `t` dans le `pico8_ke_t`, afin que l'état de chaque icône avance comme avant.
20. En tant que gardien du filet, je veux qu'une table de construction portant une autre clé que `ke` ou `jb` déclenche une assertion porteuse de message, afin qu'un champ jamais mesuré ne soit pas perdu en silence.
21. En tant que mainteneur, je veux que l'écrivain d'`ie` n'accepte que `m_`, afin qu'une écriture générique d'un autre nom (y compris `gy` ou `id`) déclenche l'arrêt du §7 au lieu d'un membre improvisé.
22. En tant que mainteneur, je veux que le lecteur d'`ie` rende `gy` et `id` depuis l'état statique, afin qu'une lecture générique sur une icône rende ce que la chaîne de classes rendait.
23. En tant que mainteneur, je veux que `kq`, `ra` (999,999) et `eh` soient possédés par le module, construits à l'init et libérés au shutdown, afin qu'aucun accès `->defaults` ne subsiste pour `ie`.
24. En tant que mainteneur, je veux que `_init` (`cart.c`) passe par deux fonctions du module pour lire `eh` et poser `gy`, afin que `cart.c` ne touche plus aux défauts d'`ie`.
25. En tant que mainteneur, je veux que `pico8_research_icon_init` remette `gy` à nil et `id` à 0, et que le shutdown remette `gy` à nil, afin qu'un rechargement du cart reparte comme la redéclaration de la classe, sans pointeur pendant vers un `eh` libéré.
26. En tant que gardien de la couverture, je veux que le constructeur enregistre `"ie"` une fois, jamais `"ke"`, afin que `ie  133  15/15` et `ke  0  0/15` restent identiques.
27. En tant que mainteneur, je veux que la `pico8_class_t` d'`ie` reste déclarée à la même place, avec sa spec, son parent et son nom de couverture, afin que `KNOWN[]` et l'assert de `cart.c` tiennent.
28. En tant que mainteneur, je veux que les 9 accès `->defaults` d'`ie` (7 dans `research_icon.c`, 2 dans `cart.c`) disparaissent, afin d'appliquer la règle des accès directs aux défauts.
29. En tant qu'auteur des étapes suivantes, je veux que l'enum ne gagne que `HD` et `HB`, afin que chaque ajout reste attaché à une étape.
30. En tant que gardien du filet, je veux que `hd` sur `fq` et `bg` rende toujours la valeur `1` par leur lecteur de champ, afin que l'ajout de `HD` à l'enum ne change pas leur survol.
31. En tant que gardien des tests, je veux que tous les tests de régression, et `test_pico8_jc`, `test_pico8_mz`, `test_pico8_bg`, `test_pico8_fq`, `test_pico8_qb`, passent avec un diff vide, afin que la conversion ne se valide pas en réécrivant des tests existants.
32. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
33. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.14, afin que la liste des étapes dise ce que l'étape a réellement touché.
34. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés, afin de ne laisser aucun contrat mensonger.
35. En tant que vérificateur, je veux une caractérisation d'`ie` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`id`, alias `gy`/`eh`, `gy` après `eu()`, `m_`).

## Implementation Decisions

Aucun outil de création d'agent n'était disponible dans cette session pour lancer un agent `guideline` frais. J'ai donc chargé le skill `guideline` et je l'ai appliqué moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **État statique : `gy` et `id`, rien d'autre.** Ce sont les deux seuls défauts réaffectés sur la classe (mesuré par grep de `"gy"`, `"id"`, `"eh"`, `"kq"`, `->defaults` dans `src/` et `tests/`). `kq`, `ra` et `eh` ne sont ni réaffectés ni modifiés en place (le seul écrivain en place d'un `ra`, `na`, sert à `qh`). Si l'implémenteur trouve une modification en place de l'un d'eux, c'est la règle des défauts modifiés en place qui s'applique : site découvert, état statique, ligne 1a.14. Ce n'est pas un membre.
- **(G) `gy` et `id` en `pico8_value_t`**, pas en `pico8_vec_t`/`pico8_fixed_t`. Rejeté : le typage C. C'est la phase 1b, et il perdrait l'alias `gy`/`eh` (la première icône révélée tient la REF de `eh` elle-même).
- **(G) Remise à zéro de l'état statique dans `pico8_research_icon_init`**, parce que la redéclaration de la classe remettait aujourd'hui `id`=0 et `gy` absent. `eu()` ne la touche pas, et `_init` ne remet que `gy`. C'est le cart littéral.
- **(G) Le lecteur rend `gy` et `id` depuis l'état statique**, modèle `nf` (`l`/`r`). Aucun consommateur mesuré ne les lit sur une icône. Mais la chaîne de classes les rendait, et la caractérisation (lue par l'API double sur l'ancien code) les lit ainsi. Rejeté : les rendre absents. Cela changerait un champ observable sans raison.
- **`eh` est un membre** (défaut non muté de la chaîne, §5), qui pointe vers le vecteur du module. `_init` le lit « sur la classe » par la fonction du module, comme `pl` de `bg`.
- **(G) Deux fonctions pour `_init`** (lire `eh`, poser `gy`) plutôt qu'une seule « remettre `gy` ». Elles gardent les deux opérations littérales de `ie.gy,kc=ie.eh` dans `cart.c`, là où le cart les fait. Rejeté : déplacer la ligne dans `research_icon.c`. Ce serait plus court, mais cela éloignerait `_init` de son équivalent Lua.
- **Constructeur : seules `ke` et `jb` sont acceptées.** Modèle `iv`/`jc`. Le seul appelant est `eu`.
- **Enum** : `HD` et `HB`, parce que `hd` et `hb` sont appelées par `mv(…,"hd"|"hb")` et doivent être présentes au `lt`. Aucun autre nom ne s'ajoute. `ie` n'a ni `ph`, ni `ev`, ni `hk`, ni `rdown`, ni `gv`.
- **`HD` et `fq`/`bg`** : leur `hd` reste un membre lu comme champ, l'entrée `HD` de leur descripteur reste NULL. Rejeté : leur donner une méthode `HD` qui rend 1. Ce serait toucher deux classes hors de l'étape.
- **Écrivain** : `m_` seul, le seul nom qu'un écrivain générique pose sur un `ie` (mesuré : `gw` sur `o_` et `gj`). `ra` et `kh` sont écrits par `ie:hb`, directement dans les membres. (G) Rejeté : accepter aussi `ra`/`kh`. Aucun écrivain générique ne les écrit, donc une telle écriture serait un site non mesuré, donc un arrêt.
- **Pas de `pq` propre** : le constructeur appelle seulement `pico8_ke_struct_pq`, après l'enregistrement de la couverture (modèle `fq`). L'ordre est celui de `pico8_class_instantiate` : champs copiés, puis `pq` de `ke` (`lt`).
- **`gk` passe à l'API double pour `kq`** : c'est le seul lecteur à simple forme qu'un `ie` atteint (mesuré par grep sur `"kq"`, `"kh"`, `"ke"`, `"jb"`, `"m_"`, `"o_"`, `"gj"`, `cv("hd"|"hb"|"kj")` et `pico8_hv()`). Tout autre lecteur trouvé en cours d'étape est un site découvert (règle « Coexistence ») : il est ajouté à la ligne 1a.14.
- **Coexistence avec l'enveloppe `ie.ke` (1a.15)** : `ie` devient une struct, son membre `ke` tient toujours la **classe-valeur**. L'enveloppe reste une instance sans classe à pile, construite à chaque lecture de `self.ke.<champ>` et pour `dl(self.ke)`. Seul l'accès au membre `ke` change. 1a.15 convertira l'enveloppe sans toucher la struct `ie`.
- **`ie.ke` instancié** (`research_icon.c:198`) : `pico8_class_instantiate` inchangé. Il construit une instance aujourd'hui, et la struct de la classe tenue quand elle sera convertie (délégation `struct_new`, §5). Le résultat part en REF dans `ok.lheld`, quelle que soit sa forme.
- **`live_ga` de `geoharvest_burst`** (G) : laissé tel quel, comme en 1a.10, 1a.12 et 1a.13. La réécriture est prévue en 1a.26.
- **`test_pico8_entity_smoke`** n'est pas touché. Ses fausses structs utilisent des initialiseurs désignés, que l'ajout de `HD`/`HB` ne casse pas.
- **Commentaires** (CLAUDE.md) :
  - À ajouter : le contrat de la struct (jamais libérée, TROU-7 ; `kq`/`ra`/`eh` partagés, jamais modifiés en place ; `ke` tient une classe, pas une instance) ; le contrat de l'état statique (`gy`/`id` partagés par toutes les icônes, `gy` rebindé et jamais écrit au travers, remis par `_init` seulement, `id` jamais remis en jeu) ; le contrat de `pico8_ie_new` (rend une struct, n'accepte que `ke`/`jb`) ; l'assert porteuse des clés reçues ; le contrat de l'écrivain (`m_`).
  - À corriger, parce que le changement les rend faux :
    - l'en-tête de `research_icon.h` (« NULL `self` » vise la struct) ;
    - `pico8_ie_hb` (« writes `ie`'s defaults » devient l'état statique) ;
    - `pico8_ie_ldown` (« Requires pico8_research_icon_init() »… « writes ie.id ») ;
    - le commentaire de `g_ie_empty_fields` s'il ne dit plus vrai ;
    - le commentaire `_init` de `cart.c`, seulement s'il nomme les défauts.
  - Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation d'`ie` sur l'ancien code.** Nouveau test (voir Testing), valeurs figées **avant** toute modification de `research_icon.c/h`, `cart.c`, `entity_geom.c` et `pico8_struct.h`.
2. **Coexistence hors du module.** `gk` lit `kq` par l'API double. Tout reste vert, et `ie` est encore une instance.
3. **`ie` converti.** Enum `HD`/`HB`, struct, état statique `gy`/`id`, objets du module `kq`/`ra`/`eh`, fonctions `eh`/`gy` pour `_init`, descripteur, constructeur, méthodes sur la struct, classe reliée, `->defaults` retirés (9 sites), accesseur, ligne 1a.14 du plan. La caractérisation passe **sans changer ses valeurs figées**, et la vérification de forme s'ajoute.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field` et `pico8_mv`, l'appartenance à `hv`/`cv`/`cg`, `ok.o_`/`ok.lheld`, les pixels du framebuffer et le compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_ie`** (dans `make unit`, lié comme `test_pico8_jc`/`test_pico8_bg`, sur le cart réel chargé).
  - Il lit **uniquement** par l'API à double forme : `pico8_entity_get_field`, `pico8_entity_set_field`, `pico8_mv`, `pico8_hv`/`pico8_cv`/`pico8_cg`, `pico8_dg`, `pico8_dc`, `pico8__init`, `pico8_eu`, `pico8_class_instantiate(pico8_ie_class(), {ke=…, jb=…})`, et les lectures de `ok` par l'API double. Ainsi le code est le même avant et après.
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()` (`in_fresh_child`).
  - Les icônes se trouvent par `cv.hb` filtré sur la présence d'un champ `ke` qui n'est pas une entité : il n'y a pas de `cg.ie`.
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction** par `eu()` (les sept icônes) et par `pico8_class_instantiate(pico8_ie_class(), {ke=qh, jb=jb neuf})` :
       - lire `me`, `t`, `fa`, les bornes de `kq` (par `pico8_qz_call` sur des points aux coins), `ra` (999,999), `eh` (118,120), `id`, `gy` ;
       - vérifier que `ke` est identique à la classe passée et `jb` au `jb` passé ;
       - vérifier que `ra` et `kq` sont la même REF sur deux icônes neuves ;
       - vérifier que `kh`, `m_`, `p_`, `nu` et `fx` sont absents ;
       - vérifier que `hd`, `hb`, `kj` et `ldown` sont des fonctions, et que `ph`, `ev`, `hk`, `rdown`, `gv`, `hq`, `md` sont absents ;
       - vérifier l'appartenance à `hv`, `cv.kj`, `cv.hd`, `cv.hb`, et à aucun de `cv.ev`/`cv.hk` ;
       - vérifier l'ordre des sept icônes dans `cv.hb` (identité de `ke` = `gx[i]`).
    2. **`_init` puis `eu()`** :
       - la première icône révélée a `ra` identique (même REF) à `eh`, et `kh` vrai ;
       - pour chaque icône, `kh` et `ra` (x,y), puis `gy` (x,y) et `id` sur n'importe quelle icône.
       Figé.
    3. **Rejeu de `events_recorded_1790198294.json`** (depuis le titre ; c'est l'enregistrement de l'étape 0.3, qui construit `qh`, `os`, `nl`, `hj`, `nr`, `fr`, `mx`, donc qui appelle `ie:ldown` sur chaque classe de `gx[]` ; l'implémenteur le confirme par `make class-coverage DIR=…` sur ce seul fichier avant de figer, et choisit sinon le rejeu du filet qui compte `nl`/`os`). Par frame :
       - pour chaque icône de `cv.hb` : `kh`, `ra`, `m_`, `t` ;
       - `gy`, `id` ;
       - le rang d'`ok.o_` dans `cv.hb` (ou absent) ;
       - si `ok.lheld` vient d'`ie:ldown`, son `id` (chaîne `"s<n>"`) et l'identité de sa classe avec le `ke` de l'icône cliquée.
       Le tout est replié en empreinte de séquence. Figé.
    4. **Rejeu de `trading_hub_session_events.json`** et de **`phase4_playthrough_events.json`** : même empreinte. Figés.
    5. **Scénarios dirigés**, sur un monde issu de `_init` + `eu()` dans un fils :
       - `mv(ie,"hb")` deux fois sur une icône révélée → `ra` et `gy` inchangés au second appel ;
       - `mv(ie,"hb")` sur une icône dont `oj(ke.rr)` est faux → reste cachée, `gy` inchangé ;
       - `mv(ie,"ldown")` sur deux icônes différentes → `id` +1 chacune, partagé ; le résultat a `id`=`"s<n>"`, et sa classe est le `ke` de l'icône ;
       - `mv(ie,"hd",qa,rp)` avec `rp` dans puis hors de `kq` autour de `ra` → vrai/faux ; sur une icône cachée → faux ;
       - `pico8_entity_set_field(ie,"m_",vrai)` puis `dc("kj")` sur un framebuffer effacé avec seulement cette icône → empreintes pour cachée, révélée, et révélée avec `m_` ;
       - `eu()` une seconde fois → `gy` et `id` **non** remis ; puis `_init()` → `gy` identique à `eh`, `id` non remis.
       Figés.
  - **Forme** (ajoutée au morceau 3) :
    - chaque icône de `cv.hb` construite par `eu()`, et celle de `pico8_class_instantiate`, est taguée `STRUCT` avec le descripteur d'`ie` (accesseur) ;
    - `pico8_mv(ie,"hd"|"hb"|"kj"|"ldown",…)` trouve une méthode, et `mv(ie,"ph"|"ev"|"hk"|"rdown"|"gv"|"eb")` ne trouve rien ;
    - `pico8_entity_set_field(ie,"m_",…)` passe ;
    - `pico8_entity_set_field(ie,"kh"|"ra"|"gy"|"id",…)` asserte (vérifié dans un fils, `SIGABRT`) ;
    - une construction avec `{ke=…, jb=…, ra=…}` asserte (dans un fils) ;
    - `pico8_entity_get_field(fq|bg,"hd")` rend toujours `1` (fixe), et `pico8_mv(bg|fq|jc|mz,"hb")` reste absent.
  - **Mutations** : l'implémenteur remet localement au moins 12 erreurs plausibles et note dans le commit celles qui sont attrapées :
    - `HD`, `HB`, `KJ` ou `LDOWN` absent de la table ;
    - `HD` pointé sur `hb` ;
    - `gy` en membre par instance (chaque icône son curseur) ;
    - `id` en membre par instance ;
    - `gy` décalé de +10 au lieu de −10 ;
    - `gy` copié au lieu d'aliasé (`ra` de la première icône ≠ REF de `eh`) ;
    - `id` incrémenté après la lecture de `ke`, ou seulement si `ke` est présent ;
    - `gy`/`id` remis par `eu()` ;
    - `fa` initial ≠ 12 ;
    - `ra` initial ≠ (999,999) ;
    - `kq` absent ;
    - écrivain qui refuse `m_` ;
    - `ke` ou `jb` de la table de construction ignoré ;
    - `pico8_ke_struct_pq` omis ;
    - `gk` qui retombe sur une lecture à simple forme de `kq` (assert) ;
    - `HD` posé aussi sur `fq`/`bg`.

    Chaque mutant équivalent éventuel est justifié dans le commit. Par exemple : `ra` initial pointé vers un autre vecteur (999,999) au contenu identique, ou la remise de `gy` au shutdown omise (aucun test ne lit après un shutdown).
- **Tests de régression** : aucun n'est réécrit. Tous passent avec un diff vide, en particulier `playability`, `tech_screen`, `cheat_code`, `game_end`, `geoharvest_burst`, `empty_planet_cleared`, `slipway_drag`, `score_panel_close`.
- **Tests existants** : `test_pico8_jc`, `test_pico8_mz`, `test_pico8_bg`, `test_pico8_bg_kj`, `test_pico8_fq`, `test_pico8_qb`, `test_pico8_iv`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo`, `test_pico8_mh` et `test_pico8_jb_ol` ont un diff vide, et leurs valeurs figées sont identiques. `find_bg` et `find_veil` exercent la nouvelle méthode `hd` d'`ie` sans changer.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED` introduit par l'étape. Les binaires `*_refaudit` compilent.
- **Couverture** : `make class-coverage-net` rend 0, avec une sortie identique à la référence (`ie  133  15/15`, et `qh`, `os`, `nr`, `fr`, `nl`, `mx`, `hj` inchangés).
- **Prior art** : `test_pico8_jc` (rejeux depuis le titre, empreintes par frame, scénarios dirigés, fils forkés), `test_pico8_nf` (état statique partagé lu par le lecteur), `test_pico8_bg` (forme, écriture interdite dans un fils, survol `o_`/`m_`) et `test_pico8_iv` (constructeur à clés mesurées).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0, ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`, en particulier si `ie` n'est pas `ie  133  15/15`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé d'`ie` ne tombe sous aucune règle du §5. En particulier :
  - si l'assertion des clés reçues ou celle de l'écrivain générique se déclenche sur un rejeu ou un test ;
  - si l'on trouve une écriture générique sur un `ie` d'un autre nom que `m_` ;
  - si la table de construction porte une autre clé que `ke` ou `jb`.

  C'est un arrêt, pas un membre improvisé.
- `FAILED:` si `gy` ou `id` devient un membre de la struct `ie`, ou si leur état n'est pas partagé par toutes les icônes. Également si un autre défaut d'`ie` devient un état statique sans qu'une réaffectation ou une modification en place ait été trouvée et nommée à la ligne 1a.14.
- `FAILED:` si un test de régression (§6) a un diff non vide ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe qu'`ie` est convertie ou change de forme. En particulier :
  - si l'enveloppe `ie.ke` (`ie_ke_wrapper`, instance sans classe à `cls` non nul) change de forme ou cesse d'être construite (1a.15) ;
  - si `jo`, `fv`, `js`, `qh`, `nl`, `os`, `hj`, `mx`, `nr`, `fr`, `jb`, `ui`, `hy` ou `pj` change de forme ;
  - si un `->defaults` d'une autre classe bouge ;
  - si `fq` ou `bg` reçoit une méthode `HD`.
- `FAILED:` si `ie:ldown` n'instancie plus la classe tenue en `ke` par `pico8_class_instantiate`, ou si `pico8_eu` change autrement que par le type de retour ignoré de `pico8_ie_new`.
- `FAILED:` si la `pico8_class_t` d'`ie` n'est plus déclarée à la même place, avec sa spec, son parent et son nom de couverture.
- `FAILED:` si les valeurs figées de la caractérisation d'`ie` ont été changées après la conversion. Idem pour celles de `test_pico8_jc`, `test_pico8_mz`, `test_pico8_bg`, `test_pico8_bg_kj`, `test_pico8_qb`, `test_pico8_fq`, `test_pico8_iv`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo`, `test_pico8_mh`.

Critères de réussite (en plus) :

- grep :
  - plus aucun `g_ie_class->defaults` ni `pico8_ie_class()->defaults` dans `src/` et `tests/` ;
  - plus aucun `->fields` ni `pico8_instance_get_field` sur `self` dans `research_icon.c` (l'enveloppe `ie.ke` garde sa propre lecture `pico8_instance_get_field(&wrapper, …)`) ;
  - plus aucun `pico8_instance_get_field` sur `kq` dans `entity_geom.c` ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB` ;
- la struct `ie` n'a que `pico8_ke_t` et les 8 membres nommés ; son descripteur n'a que `HD`, `HB`, `KJ`, `LDOWN` ; son écrivain n'accepte que `m_` ;
- hors de `research_icon.c/h`, `cart.c`, `entity_geom.c` et `pico8_struct.h`, le diff de `src/` est vide. Dans `tests/`, seuls `test_pico8_ie.c` (nouveau) et le `Makefile` (cible et `unit`) bougent ;
- la ligne 1a.14 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.14 ».

## Out of Scope

- L'enveloppe `ie.ke` (1a.15) : elle reste une instance sans classe à pile, et `rr`/`s`/`dl(self.ke)` restent lus au travers.
- Les sept classes de `gx[]` (`qh` 1a.18, `nl` 1a.19, `os` 1a.20, `hj` 1a.21, `mx` 1a.23, `nr` 1a.24, `fr` 1a.25) : `ie:ldown` les instancie sans changer, et la délégation `struct_new` fera le reste le moment venu.
- `jo` (1a.17), `fv` (1a.28), `js` (1a.16) : ils partagent `gk` comme `hd`, et restent des instances. Seule la lecture de `kq` dans `gk` change d'API.
- `ui` (`ok`, `o_`, `gj`, `lheld`, 1a.31), `hy` (`fm`, `hy:lj`, 1a.35), `pj` (`cv.hb`, 1a.29), `jb` (déjà struct).
- La réécriture de `live_ga` dans `geoharvest_burst` (1a.26).
- Le typage C de `gy`, `id` et des membres (phase 1b). Le remplacement de `kq`/`ra`/`eh` par des constantes (phase D).
- Libérer les `ie` des parties précédentes, ou les vecteurs `gy` abandonnés (TROU-7, Plan 2 §8).
- L'audio : hors périmètre.
- Lancer l'appli raylib : jamais.

## Further Notes

- Sans `HD` dans l'enum, `lt` ne mettrait plus `ie` dans `cv.hd`. Les icônes ne seraient plus survolables : plus d'infobulle, plus de `ldown`, plus aucun bâtiment posé depuis la barre de recherche. Le filet serait rouge sur les 15 rejeux. Sans `HB`, `hy:lj` ne révélerait plus rien et `dc("kj")` ne dessinerait aucune icône.
- Sans la conversion de `gk`, le premier survol de la barre de recherche après la conversion ferait partir l'assert de `pico8_instance_get_field`. C'est-à-dire la première frame de jeu, puisque `gw` teste chaque entité de `cv.hd` à chaque frame.
- Sans `m_` dans l'écrivain, le premier survol d'une icône ferait partir l'assert de `pico8_entity_set_field`.
- Si `gy` devenait un membre, chaque icône aurait son propre curseur : toutes les icônes révélées s'empileraient à (118,120). Si `id` devenait un membre, deux bâtiments posés depuis deux icônes différentes porteraient le même nom `"s1"`.
- `gy` n'est remis que par `_init`, jamais par `eu()`. Le chemin « nouvelle partie » du cart passe par `_init` (score → `pico8_init_dispatch`), puis `fv` → `eu`. Un `eu()` sans `_init` laisserait les icônes décalées. C'est le comportement du cart, préservé tel quel et figé par le scénario dirigé.
- Mesures faites par grep sur `src/`, `tests/`, `tools/` et `Makefile` à e08cfb1, en lisant :
  - `pico8/lua.lua:1-9`, `:150-211`, `:555-575`, `:900-945`, `:1170-1195`, `:1236-1237` ;
  - `pico8_research_icon.c/h`, `pico8_cart.c` (`_init`, `eu`, déclarations), `pico8_entity.c` (`lt`, `he`, `mv`, `dc`, API double), `pico8_entity_geom.c` (`gk`), `pico8_ui.c` (`gw`, `bh`, `jz`, `ev`), `pico8_economy.c` (`hy:lj`), `pico8_bootstrap.c` (`dg`), `pico8_wormhole.c` (`mz:pq`), `pico8_slipway.c` (`jc:hq`), `pico8_struct.h`, `pico8_ke.c/h`, `pico8_bg.c` et `pico8_ui_button.c` (`fq`, modèles `hd=1`), `pico8_mouse_reader.c/h` (modèle état statique) ;
  - `tests/test_pico8_bg.c` (`find_bg`), `tests/test_pico8_fq.c` (`find_veil`), `tests/test_pico8_jc.c` (modèle), `tests/test_pico8_geoharvest_burst.c`, `tests/test_pico8_entity_smoke.c`, et `tools/class_coverage_net_expected.txt`.
