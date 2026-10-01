---
title: "Plan 3, étape 1a.16 : `js` (le bouton) devient une struct, et `dk`, les clés de capture de fermeture et le harnais `pico8_js_new` suivent"
description: "Le bouton `js` est encore une instance de la couche objet, construit par copie de chaque ligne de `pz()`, par `nv(ix,js)`, par `pj:ph` et par le harnais de 20 tests et des deux pilotes. Il passe en struct fille de `ke`, avec les méthodes `hd` (= `gk`), `ph`, `ev`, `kj`, `ldown` et un membre par clé mesurée des tables de construction (dont `my`, `hb`, `px`/`trade`/`en`, `fg`, `_cr_t`, `_jo_fe_*`). `dk`, qui est ce bouton dans le cart, devient un pointeur de struct. Cinq tests de régression sont réécrits pour lire la struct, puis prouvés rouges."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.16 — `js` (`ui_button.c:116`), `ldown`, construite par les lignes de `pz()` (`msgbox.c:317`). Tests : `nil_row`, `ga_export_panel_by`, `planet_gh_row_lq`, `rich_planet_exports` (`lq`), `pause_menu_music` (`my`) ; harnais `pico8_js_new` de 13 tests.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Clés construites à l'exécution** : […] `ldown`/`rdown` côté méthodes. Tout autre site de clé dynamique trouvé est énuméré de même, ou déclenche un arrêt (§7) si l'ensemble des clés n'est pas fini.
>
> **Défauts mutés sur la classe** : […] tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** […].
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé […].
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe […].

§6 : les cinq tests nommés sont réécrits **dans leur façon de lire** le bouton, dans ce commit, et chacun est prouvé rouge avec son bug d'origine remis localement, puis restauré (voir Testing). §7 : le commit passe `make class-coverage-net` avec une sortie identique à `tools/class_coverage_net_expected.txt` (`js  1286  15/15`). Un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la ligne 1a.16 du plan, avec « découvert à 1a.16 » dans le message de commit.

### Ce que 1a.1 à 1a.15 ont posé (jusqu'à 982cd0e), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t`. L'enum vaut aujourd'hui `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB`. Un nom absent de l'enum, ou dont l'entrée est NULL, est cherché comme champ. Le lecteur rend 1 si le champ n'est pas nil. L'écrivain rend 0 pour un nom sans membre, et `pico8_entity_set_field` asserte alors.
- `pico8_ke_t`, `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq` (à appeler **après** le `pq` propre de la fille).
- `pico8_class_set_struct_new` : `pico8_class_instantiate` délègue au constructeur, qui enregistre la couverture.
- Déjà à double forme : `pico8_entity_get_field`/`pico8_entity_set_field`, `lt`/`he`, `dg`, `dc`, `mv`, la boucle de survol `gw` (`fa`, `m_`, `fg` sur `o_`), `ui:ev` (`ql` sur `o_`), `jz`, `hy:lj` (`mv(e,"hb")`), `jc:hq` (`ok.o_.gv`), le scan `fx` de `mz:pq`, et `gk` (`kq`, `fa`, `ra`, depuis 1a.14). `pico8_instance_get_field` asserte s'il reçoit une struct.
- **Modèles** :
  - champ fonction reçu, pas une méthode : `mh.lh` (1a.4), `fq.my` (1a.5) ;
  - constructeur à clés mesurées qui asserte sur toute autre : `iv` (1a.2), `fq` (1a.5), `jc` (1a.13), `ie` (1a.14) ;
  - fille avec `pq` propre, puis `pico8_ke_struct_pq` : `iv` (1a.2) ;
  - `hd` = `gk` comme méthode `HD` : `ie` (1a.14) ;
  - membre qui tient une struct `jb` : `ie.jb` (1a.14).
- Modèle de test : `test_pico8_ie`, `test_pico8_jc`, `test_pico8_fq`. La caractérisation est figée sur l'ancien code, et lue seulement par l'API à double forme, dans un fils forké juste après `pico8_cart_load()`. La forme est vérifiée après la conversion, et les écritures interdites dans un fils. Les mutations sont notées dans le commit. Pour la réécriture d'un test de régression et sa preuve rouge : `score_panel_close` (1a.5), `planet_kj_dispatch` (1a.8), `slipway_drag` (1a.13).

### Ce que fait le cart

`pico8/lua.lua` :

```
:86-97    kv:jr : js(b) = setmetatable(mw(b), js.ns), puis la marche des pq de la fille vers ke
:258-260  js=ke:jr([[ fa=11, ]])  js.hd=gk
:261-262  function js:pq() self.lq=lq(mv(self,"lq")) self.mb=self.lq.w+2
            self.kq=qz(0,-1,self.mb,11) self.h_=self.h_ and lq(self.h_) self.jb=self.jb and jb(self.jb) end
:263-267  function js:ph() if self.qf then local d=self.qf-self.ra self.ra+=(#d<=1 and d or d*0.4) end end
:268-270  function js:ev() if self.jb and self.m_ then self.jb:ol(127,1) end end
:271-276  function js:kj(p) … self.my … self.m_ … self.lq:ol(p+v(1,2)) … self.h_ … c.w, c.bg … end
:277-279  function js:ldown() rn(self.my and 60) mv(self,"my") return true end
:280-288  function pz(p,bs) … ok.le=nv(bs,function(b) b.ra,b.qf,b.fa=ra,bp,p.ca bp+=p.gq return js(b) end) … end
:1166-1169 fv:pq : for d in all(bo) d.my=eu ; pz(self,bo)
:1181-1182 function eu(nj) dk=nj … nv(ix,js) …
:871-878  pj:ph : js(ob([[fa=15,ra=v(51,100),lq=o(" again? ")]],{my=_init})) ; js(ob([[fa=15,ra=v(39,100),lq=o(o(10,7))]],de))
:980-997  ix[1..4].my, ix[2].hb=function(self) self.lq=lq({…}) end ; ix[4].my : ha[i].my=… puis pz(ha,ha)
:751,885,1330,1334  dk.en, dk.px, dk.trade
```

Points décisifs :

- **Le bouton est la table de construction, copiée.** `js(b)` copie superficiellement `b` (`mw`), puis `js:pq` tourne **avant** `lt` (le `pq` de `ke`). Chaque clé de chaque table de construction devient donc un champ de l'instance. L'ensemble de ces clés est fini et mesuré ci-dessous. C'est la règle des membres, pas une clé dynamique.
- **`dk` est le bouton.** `mv(self,"my")` appelle `my(self)`. Les lignes de `bo` portent `my=eu`, donc `eu(nj)` reçoit le `js` cliqué et fait `dk=nj`. `dk.en`, `dk.trade` et `dk.px` lisent des champs de ce bouton, copiés de la ligne de `bo`. Le harnais `pico8_eu(pico8_js_new(bo[2]))` rejoue exactement cela sans clic.
- **`my` et `hb` sont des champs fonction reçus**, pas des méthodes de classe (modèle `mh.lh`/`fq.my`). `hb` n'existe que sur le bouton de `ix[2]`. Sa présence fait entrer ce seul bouton dans `cv.hb`, et `hy:lj` l'appelle chaque tour. Il écrit `self.lq`.
- **`hd` est `gk`**, une méthode (entrée `HD`, modèle `ie`). `ph`, `ev`, `kj` et `ldown` sont des méthodes de la classe. `me="ph"` fait appeler `js:ph` par `dg` à chaque tick.
- **Aucun défaut de `js` n'est muté.** `fa=11` n'est jamais réaffecté sur la classe. Les lignes (`b`), elles, sont modifiées en place par `pz` (`ra`, `qf`, `fa`), mais ce sont des tables de données (phase D), pas des défauts de `js`.

### Emprise réelle de `js` (mesurée sur 982cd0e)

| Aspect | Mesure |
|---|---|
| Constructions en jeu | `pz` (`msgbox.c:439`, une par ligne, 5 appelants : `fv:pq` sur `bo` `help.c:77`, `jo:gh` `planet_display.c:346`, `jo:fe` `:474`, `cr` sur `pj.cr` `menu.c:246`, `ix[4].my` sur `ha` `menu.c:416`) ; `nv(ix,js)` dans `eu` (`cart.c:184`, 4 boutons, résultat jeté) ; `pj:ph` (`score_panel_display.c:293,298`, résultats jetés). Toutes par `pico8_js_new` → `pico8_class_instantiate(g_js_class, …)`. Aucun `fz`, `gx[]`, `lookup_named` ni `kv[]` ne construit de `js`. |
| Constructions hors jeu (harnais) | `pico8_eu(pico8_js_new(bo[2]))` : **16 appels en ligne dans 15 fichiers de test** (`game_end`, `score_panel_close`, `score_panel_stars`, `rich_planet_exports`, `geoharvest_burst`, `tech_screen`, `pause_menu_music`, `cheat_code`, puis les caractérisations `mh`, `fq` ×2, `mz`, `bg`, `jc`, `ie`, `qb`) et dans `main_class_coverage.c`. **Variable déclarée `pico8_instance_t *difficulty_button`** dans 5 tests (`built_planet_panel_nil_row`, `empty_planet_cleared`, `playability`, `top_menubar`, `shop_tooltip_fillp`) et dans `main.c`. Les « 13 tests » du plan sont les 13 tests de scénario ou de régression. Les 7 caractérisations sont venues depuis. Le compte réel est de 20 fichiers plus les 2 pilotes. |
| Défauts | `fa`=11. Méthodes installées par **`->defaults`** : `hd` (dispatch vers `gk`), `ph`, `ev`, `kj`, `ldown` (`ui_button.c:125-129`), plus `pq` par `pico8_class_set_pq`. Hérités de `ke` : `me`=`"ph"`, `t`=0. Aucun défaut muté. |
| Clés des tables de construction | `bo` : `lq`, `px`, `trade`, `en`, `my` (`eu`). `ha` : `lq`, `my`, et `fg` sur `ha[4]`. `ix` : `ra`, `lq`, `my`, et `hb` sur `ix[2]`. `cr` : `lq`, `h_`, `jb` (si `t.oy`), `my` (fonction, **ou `false`** pour une tech trop chère, `menu.c:134`), `_cr_t` (`menu.c:131`) ; la ligne « invented » : `lq`, `jb`. `jo:fe` : `lq`, `h_` (table ou **`false`**), `_jo_fe_pt`, `_jo_fe_b`, `_jo_fe_od`, `my`, `jb` (`planet_display.c:426-452`). `jo:gh` : `lq`, `h_` (table ou nil). Ligne `ep` préfixée par `pz` : `lq`. `pj:ph` : `fa`, `ra`, `lq`, `my`. Écrites dans chaque ligne par `pz` avant la copie : `ra` (un seul vecteur partagé par toutes les lignes d'un appel), `qf`, `fa` (si `p.ca`, TROU-9). **Aucune clé positionnelle.** |
| Champs écrits sur l'instance | `lq`, `mb`, `kq`, `h_`, `jb` (`js:pq`, par `->fields`) ; `ra` (`js:ph`, par `->fields`) ; `m_` (boucle `gw`, déjà par `pico8_entity_set_field`) ; `lq` (`ix[2].hb`, `menu.c:326`, **par `inst->fields`**) ; `t` (`dg`, dans `pico8_ke_t`). |
| Membres qui en résultent | après `pico8_ke_t` : `fa`, `lq`, `mb`, `kq`, `h_`, `jb`, `ra`, `qf`, `my`, `m_`, `hb`, `fg`, `px`, `trade`, `en`, `_cr_t`, `_jo_fe_pt`, `_jo_fe_b`, `_jo_fe_od` (19). |
| Méthodes appelées par nom | `hd` (`gw`), `ph` (`dg`), `kj`/`ev` (`dc`), `ldown` (`jz`). Présentes au `lt` : `kj`, `ev`, `hd`, et `hb` (champ, `ix[2]` seul). Absentes : `rdown`, `gv` (`jz`, `jc:hq`), `hq`, `ey`, `md`, `hk`, `eb`, `pr`, `kz`, `ol`, `nu`, `fx`, `p_`, `ql`. **L'enum ne gagne rien.** |
| Champs fonction appelés sur un `js` | `my` : `pico8_eu_dispatch` (`cart.c:258`, **caste `self` en `pico8_instance_t *`**), `pico8_cr_row_my_dispatch` (`menu.c:75`, **lit `_cr_t` par `pico8_instance_get_field`**), `pico8_jo_fe_my_dispatch` (`planet_display.c:392-399`, **lit `_jo_fe_pt`/`_b`/`_od` par `pico8_instance_get_field`**), `ix1..4`, `ha2..4`, `pico8_init_dispatch`, `de.my` (ignorent `self`). `hb` : `pico8_ix2_hb_dispatch` (`menu.c:303-329`, **écrit `lq` par `inst->fields`**). |
| `dk` (= le `js` passé à `eu`) | `pico8_eu` fait `pico8_economy_set_dk(nj->fields)` (`cart.c:193`). **Lecteurs à simple forme sur la table de champs** : `en` (`galaxy.c:123`), `trade` (`economy_score.c:174,215`), `px` (`score_panel_display.c:238`). `pico8_eu_dispatch` reçoit le bouton cliqué, le harnais le bouton construit. |
| `->defaults` | 5 sites, tous dans `ui_button.c:125-129` (installations de méthode). Aucun `g_js_class->defaults` ni `pico8_js_class()->defaults` ailleurs (`src/`, `tests/`, `tools/`). `cart.c:96` asserte `pico8_js_class()->parent == pico8_ke_class()` (reste vrai). |
| Lecteurs à double forme qui atteignent un `js` | `lt`/`he` ; `dg` (`me`, `t`, `p_`, `mv(js,"ph")`) ; `dc("kj")`, `dc("ev")` (`fa`, `ra`) ; `gw` (`mv(js,"hd",qa,je.ra)`, `fa`, `fg`, écriture de `m_`) ; `ui:ev` (`ql`) ; `jz` ; `ui:bh` (`kn` vaut `true`, jamais une table : un `js` n'est jamais `lheld`) ; `ui:cj` (`he`) ; `hy:lj` (`mv(js,"hb")`) ; `jc:hq` ; `mz:pq` (`fx`) ; `gk` (`kq`, `fa`, `ra`). Tests : `find_bg`/`find_veil` (lisent `hd`, qui sera une fonction sur un `js`, écartée par le filtre `FIXED == 1`), `test_pico8_ie` (filtre `cv.hb` sur un champ `ke` : absent sur le `js` de `ix[2]`). |
| Lecteurs et écrivains à simple forme qui atteignent un `js` | Dans `ui_button.c`, tout ce qui lit ou écrit `self` (`pico8_ui_field_*`, `pico8_instance_get_field`, `->fields`). Hors du module : `_cr_t` (`menu.c:75`), `_jo_fe_*` (`planet_display.c:392-399`), `ix2_hb` (`menu.c:326`), `dk` (3 lecteurs, et `nj->fields`), `pico8_eu_dispatch` (cast). Tests : les 5 tests nommés (voir plus bas). |
| Casts et `pico8_ref_audit.h` | `pico8_js_lq_arg_table` (`PICO8_REF_AS_LIST_CONST` sur l'argument de `lq()`) reste tel quel : il porte sur la valeur brute de la ligne, pas sur le `js`. Aucune macro d'audit ne reçoit un `js`. `pz_step`, `eu_ix_map` et `pj:ph` ne gardent le résultat que comme REF opaque (`ok.le`, jetés). |
| `->cls` lu sur une entrée de `hv` | `live_ga` (`geoharvest_burst:85`), sans tag. La lecture reste dans les bornes (struct plus grande que l'en-tête d'instance), elle n'est jamais égale à `ga` et n'est jamais déréférencée. Même situation que `bg`/`mz`/`jc`/`ie`. Réécriture prévue en 1a.26. |
| Clés dynamiques | Aucune. `bt.."down"` vise `ldown`, déjà dans l'enum. |
| Filet | Les 15 rejeux construisent des `js` (`js  1286  15/15`). Barre du haut (`ix`), panneaux de planète (`jo:gh`, `jo:fe`), écran de recherche (`cr`), menu pause (`ha`), fin de partie (`pj:ph`) : tous sont vus. `title_click_*` (`test_pico8_replay_from_title`) couvre `fv:pq` → clic → `eu_dispatch`. |

### Les cinq tests de régression nommés

| Test | Ce qu'il lit sur un `js` | Bug d'origine qu'il garde | Preuve rouge (sur le code final) |
|---|---|---|---|
| `built_planet_panel_nil_row` | `lq` de chaque bouton de `ok.le` (`row_first_item_string`, par `pico8_instance_get_field`), puis `lq->fields[1]` (instance `lq`, inchangée). Déclare aussi `pico8_instance_t *difficulty_button`. | 3b8ba2c : `pico8_msgbox_truthy` ne rejetait que `false`, donc un `ep` nil (REF NULL) faisait préfixer une ligne fantôme par `pz`. | Retirer localement la clause REF-NULL de `pico8_msgbox_truthy` (`msgbox.c`). Attendu : `built_rows == BUILT_PANEL_EXPECTED_ROWS` faux (4 lignes). Restaurer, vert. |
| `ga_export_panel_by` | `lq` de chaque bouton (`button_lq`, `pico8_instance_get_field`). | 9183fee : le défaut `by=o()` de `ga` restait une `ob_table` lue comme liste à six sites. | Retirer localement le bloc de remplacement de `by` dans `pico8_ga_init` (`planet_family.c`, « `by=o()` parses to an ob_table… »). Attendu : abort, soit dans `PICO8_REF_AS_LIST_CONST` (« REF read as a list is not one ») pendant `jo:gh`, soit à l'assert de tag du test. Noter lequel. Restaurer, vert. |
| `planet_gh_row_lq` | `lq` de chaque bouton (`button_lq`) ; `lq->cls == pico8_lq_class()`, `lq->fields`, `fs` (instance `lq`, inchangée). | 8361468 : `js:pq` passait la liste `lq` d'une ligne à `lq()` par un cast aveugle en `ob_table`. La boucle de copie parcourait la liste au mauvais pas et débordait. | Remplacer localement le cas `PICO8_TYPE_TAG_LIST` de `pico8_js_lq_arg_table` par le cast aveugle `return (const pico8_ob_table_t *)v.as.ref;`. Attendu : rapport ASan heap-buffer-overflow (la cible est compilée sous ASan), ou à défaut les asserts de champ. Restaurer, vert. |
| `rich_planet_exports` | `lq` de chaque bouton de `ok.le` (`promote_and_open_panel`), puis `lq->fields[1]`. | 50f91b4 : `bb_visit` bornait l'index de production par `po->count` au lieu de `#po`. | Remplacer localement `po_len` par `po->count` dans `bb_visit` (`economy_tiers.c`). Attendu : `mu4 == 3` faux (0). Restaurer, vert. |
| `pause_menu_music` | `my` sur `ok.o_` (`ok_hovered`, puis `pico8_instance_get_field(hovered,"my")`), comparé à `ha[2].my`. | Aucun correctif dans l'historique (`git log -S pico8_ha2_my_dispatch` / `-S hovered_my` : 77c7f94, 4c156f4, b43e6c2 seulement ; c'est un test de scénario du filet, G3 de Plan 2). Bug de référence, nommé par ses propres commentaires : le bouton pressé porte `ha[2].my`, que `ix[4].my` installe **avant** `pz(ha,ha)`, parce que `js(b)` copie `my` à la construction. | Dans `pico8_ix4_my_dispatch` (`menu.c`), déplacer localement l'appel à `pico8_pz` **avant** les quatre installations de `my`. Attendu : `hovered_my` absent à la frame 20. Restaurer, vert. Contre-épreuve, notée aussi : constructeur de `js` qui ne recopie pas `my` → même assert rouge. |

Chaque réécriture ne change que la **façon de lire** : `pico8_entity_get_field` sur le bouton, et le type du pointeur (`const void *` ou le type struct). Les frames, fixtures, asserts et messages restent identiques. Les lectures sur les instances `lq` (`->fields`, `->cls`) restent en place jusqu'à 1a.34. `ok` (`ui`) reste lu par la couche jusqu'à 1a.31.

## Solution

1. **Struct `js`**, publique dans le module des boutons, à côté de `pico8_fq_t`. Premier membre `pico8_ke_t`, puis les 19 membres `pico8_value_t` du tableau d'emprise, noms minifiés conservés (y compris `_cr_t`, `_jo_fe_pt`, `_jo_fe_b`, `_jo_fe_od`, qui sont les noms déjà utilisés par le port).
2. **Valeurs initiales** : `me`=`"ph"`, `t`=0 (par `pico8_ke_struct_init`), `fa`=11, tous les autres nil.
3. **Constructeur** (posé comme `struct_new` de la `pico8_class_t` de `js`), dans l'ordre :
   1. alloue ;
   2. appelle `pico8_ke_struct_init` avec le descripteur de `js` ;
   3. pose les valeurs initiales ;
   4. recopie chaque clé nommée de la table reçue (table NULL permise) dans le membre du même nom, parmi `fa`, `lq`, `h_`, `jb`, `ra`, `qf`, `my`, `hb`, `fg`, `px`, `trade`, `en`, `_cr_t`, `_jo_fe_pt`, `_jo_fe_b`, `_jo_fe_od`, et **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise. Il ne garde aucun pointeur sur la table ;
   5. enregistre `"js"` au crochet de couverture ;
   6. appelle `js:pq` sur la struct ;
   7. appelle `pico8_ke_struct_pq`.
4. **`js:pq` sur la struct**, littéral :
   - la valeur brute de `lq` passe par `pico8_js_lq_arg_table` (inchangé), puis `pico8_lq_new` → membre `lq` ;
   - `mb` = `lq.w + 2` (lu sur l'instance `lq`) ;
   - `kq` = `qz(0,-1,mb,11)` ;
   - `h_` : si vrai au sens Lua, `lq(h_)`, sinon inchangé (`false` reste `false`) ;
   - `jb` : si vrai au sens Lua, `pico8_jb_new`, avec l'assert existant sur la table.
5. **Descripteur de `js`** :
   - méthodes `HD` (dispatch vers `gk`), `PH`, `EV`, `KJ`, `LDOWN` ; `HB` reste NULL, donc `hb` est lu comme champ ;
   - **lecteur** : il connaît les 19 membres (nil = absent), puis délègue à `pico8_ke_field` ;
   - **écrivain** : il n'accepte que `m_` (boucle `gw`) et `lq` (`ix[2].hb`), et rend 0 pour tout autre nom ;
   - **accesseur** public du descripteur, comme `pico8_fq_struct_class()`.
6. **Méthodes sur la struct** : `pico8_js_ph`, `pico8_js_ev`, `pico8_js_kj` et `pico8_js_ldown` prennent `pico8_js_t *`. Elles lisent et écrivent leurs membres directement. `js:ph` rebinde `ra` vers un vecteur neuf (TROU-7, comme aujourd'hui) et ne modifie jamais en place le vecteur partagé posé par `pz`. Les lectures sur les instances `lq` (`w`, `bg`) gardent leurs aides actuelles.
7. **`pico8_js_new`** rend `pico8_js_t *`, avec le même paramètre. `pico8_js_pq` n'est plus un `pq` de classe : la classe perd `pico8_class_set_pq` et les 5 installations `->defaults`, et reçoit le constructeur. Même déclaration, même spec, même place, même nom de couverture.
8. **`eu` et `dk`** (site découvert, règle « Coexistence ») :
   - `pico8_eu` prend `pico8_js_t *nj` ; `pico8_eu_dispatch` caste `self` vers ce type ;
   - `pico8_economy_set_dk` reçoit le bouton lui-même (`dk=nj`, NULL permis), et `pico8_dk()` le rend ;
   - les trois lecteurs (`en`, `trade`, `px`) lisent par `pico8_entity_get_field` sur `dk`, avec les mêmes gardes « absent ou mauvais genre → défaut » qu'aujourd'hui.
9. **Fermetures qui lisent sur leur `self`** (site découvert) : `pico8_cr_row_my_dispatch` lit `_cr_t`, et `pico8_jo_fe_my_dispatch` lit `_jo_fe_pt`/`_b`/`_od`, par `pico8_entity_get_field`. `pico8_ix2_hb_dispatch` écrit `lq` par `pico8_entity_set_field`. Aucune autre ligne de `menu.c` ou de `planet_display.c` ne change.
10. **Appelants du constructeur** : `pz_step` (`msgbox.c`), `pico8_eu_ix_map` (`cart.c`) et `pj:ph` (`score_panel_display.c`) ne changent que le type de la variable qui reçoit le résultat.
11. **Harnais** (changement forcé par le type, minimal) :
    - les 16 appels en ligne `pico8_eu(pico8_js_new(…))` et `main_class_coverage.c` **ne changent pas** ;
    - les 6 déclarations `pico8_instance_t *difficulty_button` (5 tests, `main.c`) deviennent `pico8_js_t *` ;
    - rien d'autre : la duplication de ce harnais, relevée par `warn-review`, **n'est pas factorisée** ici.
12. **Cinq tests de régression** réécrits dans leur façon de lire (tableau ci-dessus), puis preuve rouge.
13. **Plan** : ajouter à la ligne 1a.16 de `docs/plan-3.md` le texte suivant.

    > Découvert à 1a.16 : `dk` est le `js` passé à `eu` (`cart.c:193`), lu par `en` (`galaxy.c:123`), `trade` (`economy_score.c:174,215`), `px` (`score_panel_display.c:238`) → lecteurs à double forme ; clés de construction `px`/`trade`/`en` (`bo`), `fg` (`ha[4]`), `hb` (`ix[2]`, champ fonction, `ix[2].hb` écrit `lq`, `menu.c:326` → écrivain), et les clés de capture `_cr_t` (`menu.c:75,131`) et `_jo_fe_pt`/`_b`/`_od` (`planet_display.c:392-399,450-452`) → membres, lus à double forme ; harnais réel : 20 tests et 2 pilotes, 6 déclarations retypées.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `js` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise de `js` soit mesurée dans le PRD (constructions, clés reçues, champs écrits, méthodes, `->defaults`, lecteurs et écrivains génériques, `dk`, fermetures, tests, harnais, filet), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux que la struct `js` n'ait que `pico8_ke_t` et les 19 membres mesurés, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que gardien du filet, je veux que chaque clé d'une ligne de `pz`, de `ix`, de `bo`, de `pj:ph` et du harnais arrive dans le membre du même nom, afin que `js(b)` reste une copie de `b`.
5. En tant que mainteneur, je veux qu'une clé reçue hors de la liste mesurée déclenche une assertion porteuse de message, afin qu'un champ jamais mesuré soit un arrêt (§7) et pas une perte silencieuse.
6. En tant que gardien du filet, je veux que `js:pq` tourne avant `lt`, et remplace `lq`, `h_` et `jb` par des objets construits en calculant `mb` et `kq`, afin que la largeur et la boîte de chaque bouton soient identiques.
7. En tant que gardien du filet, je veux qu'un `h_` à `false` reste `false`, et qu'un `h_`/`jb` absent reste absent, afin que les lignes de `jo:fe` sans prix ne dessinent pas de pastille.
8. En tant que gardien du filet, je veux que `js:ph` fasse converger `ra` vers `qf` (pas de 0,4, puis saut quand `|d|² ≤ 1`) en rebindant `ra`, afin que l'animation des menus soit identique et que le vecteur partagé de `pz` ne bouge jamais.
9. En tant que gardien du filet, je veux que `js:kj` dessine le cadre, le fond (13, 12 survolé, 5 sans `my`), la ligne de `lq` et la pastille `h_`, afin que chaque bouton soit identique au pixel près.
10. En tant que gardien du filet, je veux que `js:ev` dessine l'infobulle `jb` seulement quand `m_` est vrai, afin que les infobulles des techs et des projets restent identiques.
11. En tant que gardien du filet, je veux que `js:ldown` appelle `rn(60)` si `my` est vrai (sinon `rn(false)`), appelle `my` avec le bouton comme `self` et rende vrai, afin que chaque clic de menu ait le même effet.
12. En tant que gardien du filet, je veux qu'un `my` à `false` (tech trop chère) ne soit ni appelé ni vrai, afin que le bouton reste gris et inerte.
13. En tant que gardien du filet, je veux que `hd` appelle `gk` avec `fa`=11 ou la couche reçue, afin que le survol des boutons soit identique.
14. En tant que gardien du filet, je veux qu'un `js` neuf soit enregistré dans `hv`, `cv.kj`, `cv.ev` et `cv.hd`, dans cet ordre relatif, et dans `cv.hb` seulement pour le bouton de `ix[2]`, afin que l'ordre de dessin, de survol et de mise à jour ne bouge pas.
15. En tant que gardien du filet, je veux que `hy:lj` appelle `ix[2].hb` sur son bouton et que la nouvelle `lq` soit écrite dans le membre, afin que l'icône de recherche de la barre du haut change comme avant.
16. En tant que gardien du filet, je veux que `dk` soit le bouton de difficulté lui-même, et que `en`, `trade` et `px` y soient lus comme avant, afin que la galaxie, les échanges et le score dépendent de la même difficulté.
17. En tant que gardien du filet, je veux que la fermeture de `cr` retrouve sa tech par `_cr_t` sur le bouton, et celle de `jo:fe` sa planète, son projet et son prix par `_jo_fe_*`, afin qu'acheter une tech ou un bâtiment marche comme avant.
18. En tant que gardien du filet, je veux que la boucle `gw` puisse écrire `m_` sur un bouton, afin que le survol colore le bouton et ouvre son infobulle.
19. En tant que mainteneur, je veux que l'écrivain de `js` n'accepte que `m_` et `lq`, afin que toute autre écriture générique sur un bouton soit un arrêt.
20. En tant que mainteneur, je veux que les 5 installations `->defaults` et le `pq` de classe de `js` disparaissent, afin d'appliquer la règle des accès directs aux défauts.
21. En tant que mainteneur, je veux que la `pico8_class_t` de `js` reste déclarée à la même place, avec sa spec, son parent et son nom de couverture, afin que `KNOWN[]` et l'assert de `cart.c` tiennent.
22. En tant que gardien de la couverture, je veux que le constructeur enregistre `"js"` une fois par bouton, jamais `"ke"`, afin que `js  1286  15/15` reste identique.
23. En tant qu'auteur des étapes 1a.30 et 1a.41, je veux que les enveloppes de `pz()` sur `pj.cr` et `g_ha` restent des instances sans classe à pile, et que `pz` continue de lire `p` par la couche, afin que leur conversion soit leur propre étape.
24. En tant qu'auteur de l'étape 1a.34, je veux que les membres `lq` et `h_` tiennent des instances `lq` construites par `pico8_lq_new`, et que `pico8_js_lq_arg_table` reste inchangé, afin que la conversion de `lq` ne touche le bouton qu'à ces points.
25. En tant qu'auteur de l'étape 1a.31, je veux que `ok.le` tienne des REF de structs `js`, que `ui:cj`, `jz` et `bh` traversent déjà à double forme, afin que convertir `ui` ne touche pas le bouton.
26. En tant que mainteneur des tests, je veux que les 16 appels en ligne du harnais compilent sans changement, et que seules les 6 déclarations typées changent de type, afin que la conversion ne réécrive pas 20 fichiers pour rien.
27. En tant que gardien des tests de régression, je veux que `built_planet_panel_nil_row`, `ga_export_panel_by`, `planet_gh_row_lq`, `rich_planet_exports` et `pause_menu_music` ne changent que leur façon de lire le bouton, afin que leur intention reste intacte (§6).
28. En tant que gardien des tests de régression, je veux que chacun de ces cinq tests soit prouvé rouge avec son bug d'origine remis, puis vert, afin que la réécriture ne l'ait pas vidé de son objet.
29. En tant que gardien des tests, je veux que `empty_planet_cleared`, `playability`, `top_menubar` et `shop_tooltip_fillp` ne changent que le type de `difficulty_button`, et que tous les autres tests aient un diff vide, afin que la conversion ne se valide pas en affaiblissant un test.
30. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
31. En tant que vérificateur, je veux une caractérisation de `js` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`dk`, `_cr_t`, `_jo_fe_*`, `my=false`, `h_=false`, `hb`).
32. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.16, afin que la liste des étapes dise ce que l'étape a réellement touché.
33. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés, afin de ne laisser aucun contrat mensonger.

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais. J'ai donc chargé le skill `guideline` et je l'ai appliqué moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Les clés de construction sont des membres, capture de fermeture comprise.** `_cr_t` et `_jo_fe_*` ne sont pas des champs du cart, mais le port les écrit sur l'instance par la copie de `js(b)`. Leur ensemble est fini et nommé à la compilation. La règle des membres s'applique, pas celle des clés dynamiques. (G) Rejeté : une struct de capture séparée par fermeture. Ce serait une réécriture de `menu.c`/`planet_display.c` hors de l'étape, et une abstraction nouvelle.
- **`my` et `hb` sont des champs fonction**, pas des entrées d'enum (modèle `mh.lh`/`fq.my`). `HB` est déjà dans l'enum pour `ie`, mais l'entrée de `js` reste NULL : `hb` est lu comme champ, présent seulement sur le bouton de `ix[2]`. L'enum ne gagne rien.
- **`dk` est le bouton**, pas sa table de champs. C'est le cart littéral (`dk=nj`). (G) Rejeté : garder la ligne de `bo` comme `dk`. Les valeurs seraient les mêmes, mais ce serait un autre objet que celui du cart, et cela cacherait le fait que `dk` est un `js`. Les lecteurs passent par `pico8_entity_get_field`, pour que le morceau de coexistence (voir Chunks) soit vert avant la conversion. `pico8_dk()` prend le type struct au morceau 3.
- **(G) L'écrivain n'accepte que `m_` et `lq`.** Ce sont les deux seuls noms qu'un code générique écrit sur un bouton (mesuré : `gw` ; `ix2_hb`, qui reçoit `void *self`). `ra`, `mb`, `kq`, `h_` et `jb` sont écrits par les méthodes du module, directement dans les membres. Rejeté : caster en `pico8_js_t` dans `menu.c`. Ce serait du code de menu qui connaît l'agencement du bouton.
- **(G) `pico8_eu(pico8_js_t *)`, et `pico8_js_new` rend `pico8_js_t *`** (modèle `fq`/`ie`). Rejeté : rendre `void *` pour ne toucher aucun test. Cela laisserait un `pico8_instance_t *difficulty_button` pointer sur une struct, la confusion de REF que ce plan supprime. Coût mesuré : 6 lignes de déclaration. `pico8_cart.h` inclut l'en-tête des boutons (pas de cycle : cet en-tête n'inclut ni `cart` ni `economy`).
- **Constructeur à clés mesurées** : modèle `iv`/`fq`/`jc`/`ie`. L'ordre est celui de `pico8_class_instantiate` : copie, `js:pq`, puis `pq` de `ke`. Enregistrement de couverture avant `js:pq`, comme `iv`.
- **`pico8_js_lq_arg_table` inchangé**, trois formes comprises : il lit la valeur brute d'une ligne, jamais le bouton. Son cas `INSTANCE` lit `->fields` d'une instance `lq` et changera en 1a.34.
- **Coexistence avec les enveloppes de `pz()`** (1a.30 `pj.cr`, 1a.41 `g_ha`) : `pz` garde sa signature `const pico8_instance_t *p`. Les enveloppes restent des instances à pile (`cls` NULL), lues par la couche. Le bouton ne lit rien sur `p` : `pz_step` n'écrit que dans la ligne `b` avant la construction. Rien ne change dans `pz` au-delà du type de `made`.
- **Coexistence avec `lq`** (1a.34) : les membres `lq` et `h_` tiennent des REF d'instances `lq`. `js:kj` les passe à `pico8_lq_ol_default` en instances. Les lectures de `w`/`bg` restent par la couche. Les tests gardent `lq->fields`/`lq->cls`.
- **Coexistence avec `ui`** (1a.31) : `ok.le`, `o_` et `gj` tiennent des REF de structs `js`. `gw`, `ev`, `jz`, `bh` et `cj` sont déjà à double forme. `ok` lui-même reste lu par la couche, dans les tests aussi.
- **Défauts mutés** : aucun pour `js`. Si l'implémenteur trouve `fa` réaffecté sur la classe, ou une modification en place d'un défaut de `js`, c'est un site découvert (état statique, ligne 1a.16), pas un membre. Le vecteur `ra` partagé entre les boutons d'un même `pz` n'est pas un défaut : c'est une valeur de ligne, jamais modifiée en place (vérifié : `js:ph` rebinde).
- **`live_ga`** (G) : laissé tel quel, réécriture prévue en 1a.26.
- **`test_pico8_entity_smoke`** n'est pas touché : son test de fermeture capturée utilise sa propre classe, pas `js`.
- **Commentaires** (CLAUDE.md) :
  - À ajouter : le contrat de la struct (jamais libérée, TROU-7 ; `my`/`hb` sont des fermetures reçues ; `_cr_t`/`_jo_fe_*` sont les captures des fermetures `my`, pas des champs du cart ; `ra` peut être partagé avec d'autres boutons et n'est jamais écrit au travers) ; le contrat de `pico8_js_new` (table lue, pas gardée, clés mesurées seulement) ; l'assert porteuse des clés reçues ; le contrat de l'écrivain (`m_`, `lq`) ; celui de `pico8_dk()` (le bouton passé à `eu`, NULL avant).
  - À corriger, parce que le changement les rend faux : l'en-tête de `pico8_js_init` (« wires `pq` plus the hd/ph/ev/kj/ldown dispatch adapters onto its defaults ») ; le commentaire de `pico8_economy_set_dk`/`pico8_dk` (« the field table of the difficulty… ») ; le commentaire de `_cr_t` (« read back here off the row once it is an instance ») ; celui de `pz` sur « The `js` instances built just above ».
  - Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation de `js` sur l'ancien code.** Nouveau test (voir Testing), valeurs figées **avant** toute modification de `ui_button.c/h`, `cart.c/h`, `menu.c`, `planet_display.c`, `msgbox.c`, `score_panel_display.c`, `galaxy.c` et `economy_score.c/h`.
2. **Coexistence hors du module.** Toujours avec `js` en instance :
   - `pico8_economy_set_dk` reçoit le bouton (en `const void *` à ce stade), et les trois lecteurs de `dk` passent par `pico8_entity_get_field` ;
   - `_cr_t` et `_jo_fe_*` sont lus à double forme, et `ix2_hb` écrit `lq` par `pico8_entity_set_field` ;
   - les cinq tests de régression lisent le bouton par `pico8_entity_get_field`.

   Tout reste vert, et la caractérisation ne bouge pas.
3. **`js` converti.**
   - Struct, constructeur, `js:pq`/`ph`/`ev`/`kj`/`ldown` sur la struct, descripteur, accesseur, classe reliée, 5 `->defaults` et le `pq` de classe retirés.
   - `pico8_eu(pico8_js_t *)`, `pico8_eu_dispatch`, et `pico8_dk()` typé.
   - Types des trois appelants et des 6 déclarations du harnais ; ligne 1a.16 du plan.

   La caractérisation passe **sans changer ses valeurs figées**, puis la vérification de forme s'ajoute. Enfin les cinq preuves rouges, chaque correctif restauré.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field` et `pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, `ok.le`/`ok.o_`, `pico8_dk()`, les pixels du framebuffer et le compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_js`** (dans `make unit`, lié comme `test_pico8_ie`, sur le cart réel chargé).
  - Il lit **uniquement** par l'API à double forme (`pico8_entity_get_field`, `pico8_entity_set_field`, `pico8_mv`, `pico8_hv`/`pico8_cv`/`pico8_cg`, `pico8_dg`, `pico8_dc`, `pico8__init`, `pico8_eu`, `pico8_js_new`, `pico8_dk()` au travers de `pico8_entity_get_field`). Le code est ainsi le même avant et après, au type de retour de `pico8_js_new` près.
  - Chaque scénario tourne dans un fils forké après `pico8_cart_load()`.
  - Les boutons se trouvent par `ok.le`, et par `cv.hd` filtré sur `fa` = 11 avec un `lq` présent, pour les boutons de `ix`.
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction, un cas par producteur** : titre (`fv:pq` sur `bo`), `eu()` (les 4 de `ix`), pause (`ix[4].my`), recherche (`cr`, avec au moins une tech trop chère et la ligne « invented »), panneau de planète connue (`jo:fe`) et construite (`jo:gh`), fin de partie (`pj:ph`), et le bouton du harnais.
       - Pour chaque bouton : `me`, `t`, `fa`, `mb`, les bornes de `kq` (par `pico8_qz_call` aux coins), `ra` et `qf` (x,y), le genre de `lq`/`h_`/`jb` (tag, `w` de `lq`), le genre de `my` (fonction, `false` ou absent) et l'identité de la fonction, et `hb`, `fg`, `px`, `trade`, `en`, `_cr_t`, `_jo_fe_pt`/`_b`/`_od` (présence et identité).
       - Absents : `m_`, `p_`, `nu`, `ql`.
       - Fonctions : `ph`, `kj`, `ev`, `ldown`, `hd`. Absents : `rdown`, `gv`, `hq`, `ey`, `md`, `hk`.
       - Appartenance à `hv`, `cv.kj`, `cv.ev`, `cv.hd`, `cv.hb`, `cv.hk`, et ordre dans `ok.le`.
       - Le `ra` de deux boutons d'un même `pz` est la même REF.
    2. **`dk`** : après `pico8_eu(pico8_js_new(bo[i]))` pour i=1..4, `dk` est le bouton passé, et ses `en`, `trade`, `px` sont lus. Figé.
    3. **Rejeux** : `title_click_reasonable.json` (depuis le titre), `pause_menu_music_events.json`, `tech_screen_invent_events.json`, `built_planet_panel_events.json`, `game_end_events.json`.
       - Par frame, pour chaque bouton de `ok.le` et de `cv.hb` : `ra`, `m_`, `t`, `w` de `lq`.
       - Le rang d'`ok.o_` dans `ok.le`, et une empreinte de l'écran.
       - Le tout est replié en empreinte de séquence, figée.
       - L'implémenteur confirme que chaque rejeu atteint son producteur, et note dans le commit les producteurs atteints.
    4. **Scénarios dirigés** :
       - `js:ph` sur `qf` loin puis à une distance ≤ 1 (pas de 0,4, puis saut), le vecteur `ra` d'origine inchangé ;
       - `mv(js,"ldown")` avec un `my` qui compte ses appels et reçoit le bouton comme `self` → un appel, rend vrai ; avec `my=false` → aucun appel ;
       - `mv(js_ix2,"hb")` → `lq[1]` vaut 24 ou 37 selon `cb()` ;
       - le `my` d'une ligne de `jo:fe` appelle `hu` sur la bonne planète si le prix passe ;
       - le `my` d'une ligne de `cr` appelle `kt` sur la bonne tech ;
       - `dc("kj")` puis `dc("ev")` sur un framebuffer effacé avec un seul bouton → empreintes pour `my` absent, `my` présent, `m_` vrai, `h_` présent, et `jb` avec `m_`.

       Figés.
  - **Forme** (ajoutée au morceau 3) :
    - chaque bouton de ces producteurs est tagué `STRUCT`, avec le descripteur de `js` ;
    - `pico8_entity_set_field(js,"m_"|"lq",…)` passe ;
    - `pico8_entity_set_field(js,"ra"|"my"|"mb",…)` asserte (dans un fils) ;
    - une construction avec une clé inconnue, ou positionnelle, asserte (dans un fils) ;
    - `mv(js,"hb")` ne trouve rien sur un bouton autre que celui de `ix[2]`.
  - **Mutations** : l'implémenteur remet localement au moins 12 erreurs plausibles et note dans le commit celles qui sont attrapées :
    - `HD`, `PH`, `KJ`, `EV` ou `LDOWN` absent de la table ;
    - `hb` non recopié ;
    - `my` non recopié ;
    - `_cr_t` ou `_jo_fe_b` non recopié ;
    - `fa` initial ≠ 11 ;
    - `mb` sans le +2 ;
    - `h_=false` transformé en `lq` ;
    - `js:pq` appelé après `pico8_ke_struct_pq` ;
    - `js:ph` qui écrit dans le vecteur partagé ;
    - écrivain qui refuse `lq` ;
    - `dk` lu sur la ligne de `bo` au lieu du bouton ;
    - `ldown` qui rend faux.

    Chaque mutant équivalent est justifié. Par exemple, l'ordre couverture/`js:pq` est invisible au test.
- **Tests de régression réécrits** (§6) : `built_planet_panel_nil_row`, `ga_export_panel_by`, `planet_gh_row_lq`, `rich_planet_exports`, `pause_menu_music`.
  - Le diff se limite à la lecture du bouton (`pico8_entity_get_field`, type du pointeur), aux includes, et pour `nil_row` au type de `difficulty_button`.
  - Preuve rouge de chacun sur le code final, par la commande `make tmp/test_pico8_<nom> && ./tmp/test_pico8_<nom>` (`ASAN_OPTIONS=detect_leaks=0` pour `planet_gh_row_lq`), selon le tableau du Problem Statement. Le commit note chaque fois : le bug remis, le mode d'échec observé, la restauration et le vert.
  - Si un bug remis ne rend **pas** son test rouge, c'est un arrêt (§7), pas un test durci.
- **Harnais** : `empty_planet_cleared`, `playability`, `top_menubar`, `shop_tooltip_fillp` et `main.c` ne changent que le type de `difficulty_button`. Les 15 fichiers à appel en ligne et `main_class_coverage.c` ont un diff vide.
- **Tests existants** : `test_pico8_ie`, `test_pico8_jc`, `test_pico8_mz`, `test_pico8_bg`, `test_pico8_bg_kj`, `test_pico8_fq`, `test_pico8_qb`, `test_pico8_iv`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo`, `test_pico8_mh`, `test_pico8_jb_ol`, `test_pico8_class_chain` ont un diff vide, et leurs valeurs figées sont identiques. `tech_screen`, `game_end`, `score_panel_close`, `score_panel_stars`, `cheat_code`, `geoharvest_burst`, `slipway_drag`, `layout_nil_item_draws_nothing` et `test_pico8_replay_from_title` passent avec un diff vide.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED` introduit par l'étape. Les binaires `*_refaudit` compilent, dont `planet_gh_row_lq` et `ga_export_panel_by`.
- **Couverture** : `make class-coverage-net` rend 0, avec une sortie identique à la référence (`js  1286  15/15`, `lq  10718`, `jb  831` inchangés).
- **Prior art** : `test_pico8_ie` (producteurs multiples, rejeux, scénarios dirigés, forme), `test_pico8_fq` (champ fonction `my`, survol `o_`/`m_`), `test_pico8_iv` (constructeur à clés mesurées), `score_panel_close` (1a.5), `planet_kj_dispatch` (1a.8, bug de référence faute d'historique) et `slipway_drag` (1a.13) pour les réécritures et les preuves rouges.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0, ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`, en particulier si `js` n'est pas `js  1286  15/15`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé de `js` ne tombe sous aucune règle du §5. En particulier :
  - si l'assertion des clés reçues ou celle de l'écrivain se déclenche sur un rejeu ou un test ;
  - si l'on trouve une clé de construction hors des 16 mesurées, ou une écriture générique sur un `js` d'un autre nom que `m_`/`lq`.

  C'est un arrêt, pas un membre improvisé.
- `FAILED:` si l'un des cinq tests nommés change autrement que dans sa façon de lire le bouton (plus, pour `nil_row`, le type de `difficulty_button`) : frames, fixtures, asserts ou messages modifiés. Également si le commit ne rapporte pas sa preuve rouge (bug remis, rouge, restauré, vert), ou si un bug remis ne le rend pas rouge.
- `FAILED:` si un autre test de régression (§6) a un diff non vide. Seule exception : `empty_planet_cleared` et `playability`, limités au type de `difficulty_button`.
- `FAILED:` si une autre classe que `js` est convertie ou change de forme. En particulier :
  - si `lq`, `ui`, `jb`, `pj`, `fv`, `hp`, `jo` ou `hy` change de forme ;
  - si les enveloppes de `pz()` (`menu.c`, instances sans classe à pile) changent ;
  - si un `->defaults` d'une autre classe bouge ;
  - si l'enum de méthodes change.
- `FAILED:` si la `pico8_class_t` de `js` n'est plus déclarée à la même place, avec sa spec, son parent et son nom de couverture.
- `FAILED:` si le harnais est refactoré au-delà du changement de type forcé (fonction partagée, suppression de la duplication), ou si l'un des 16 appels en ligne ou `main_class_coverage.c` change.
- `FAILED:` si les valeurs figées de la caractérisation de `js` ont été changées après la conversion, ou celles de `test_pico8_ie`, `jc`, `mz`, `bg`, `bg_kj`, `fq`, `qb`, `iv`, `nf`, `je`, `mo`, `mh`.

Critères de réussite (en plus) :

- grep :
  - plus aucun `g_js_class->defaults` ni `pico8_class_set_pq(g_js_class` ;
  - plus aucun `pico8_instance_get_field` ni `->fields` sur `self` dans les fonctions de `js` de `ui_button.c` (les lectures sur les instances `lq` restent) ;
  - plus aucun `pico8_instance_get_field(inst, "_cr_t"` ni `"_jo_fe_` ;
  - plus aucun `nj->fields` ni `pico8_ob_table_get_name(dk,` ;
- l'enum vaut toujours exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB` ;
- la struct `js` n'a que `pico8_ke_t` et les 19 membres nommés ; son descripteur n'a que `HD`, `PH`, `EV`, `KJ`, `LDOWN` ; son écrivain n'accepte que `m_` et `lq` ;
- hors de `ui_button.c/h`, `cart.c/h`, `menu.c`, `planet_display.c`, `msgbox.c`, `score_panel_display.c`, `galaxy.c`, `economy_score.c/h` et `main.c`, le diff de `src/` est vide. Dans `tests/`, seuls `test_pico8_js.c` (nouveau), les cinq tests nommés, les quatre déclarations du harnais et le `Makefile` (cible et `unit`) bougent ;
- la ligne 1a.16 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.16 ».

## Out of Scope

- Les enveloppes de `pz()` sur `pj.cr` (1a.30) et `g_ha` (1a.41), et la signature de `pz`.
- `lq` (1a.34) : `pico8_js_lq_arg_table`, les lectures `w`/`bg` sur les instances `lq`, et les lectures `lq->fields`/`lq->cls` des tests.
- `ui` (1a.31) : `ok`, `le`, `o_`, `gj`, `lheld`, et les lectures de `ok` par la couche dans les tests.
- `jo` (1a.17), `pj`/`de` (1a.29), `fv` (1a.28), `hp` (1a.33) : producteurs de lignes, inchangés hors des lectures de capture sur le bouton.
- Factoriser le harnais `pico8_eu(pico8_js_new(bo[2]))`, malgré la duplication relevée par `warn-review`.
- La réécriture de `live_ga` dans `geoharvest_burst` (1a.26).
- Le typage C des membres (phase 1b). Les tables `bo`/`ix`/`ha` en données constantes (phase D).
- Libérer les boutons retirés par `ui:cj`, ou les vecteurs `ra` abandonnés (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- Sans `hb` recopié, le bouton de `ix[2]` sortirait de `cv.hb` : l'icône de recherche de la barre du haut ne passerait plus de 37 à 24. Le filet le verrait dès qu'une tech devient abordable.
- Sans `_jo_fe_pt`, l'assert « jo:fe's my callback lost its captured planet » partirait au premier achat de bâtiment. Sans `_cr_t`, le clic sur une tech ne ferait rien, en silence : seuls `tech_screen` et la caractérisation l'attraperaient.
- Sans `dk` = bouton, `pico8_eu` n'aurait plus de `->fields` à passer. Un `dk` lu sur la ligne de `bo` rendrait les mêmes valeurs, et le filet ne le verrait pas. D'où le scénario dirigé 2 et le mutant « `dk` lu sur la ligne ».
- `js:pq` avant `lt` compte : `lt` teste la présence de `hd`/`kj`/`ev`/`hb`, qui ne dépendent pas de `pq`. L'ordre n'est donc pas observable par l'enregistrement, mais il l'est par la couverture et par un `pq` qui lirait l'état de `hv`. Le constructeur suit l'ordre du cart.
- `pz` écrit `ra`, `qf` et `fa` dans les lignes elles-mêmes (`bo`, `ha`, `ix` restent modifiées après l'appel). C'est le cart, et cela ne concerne pas `js`. Le harnais s'appuie dessus : `bo[2]` porte déjà `ra`/`qf`/`fa`/`my` quand `pico8_js_new(bo[2])` le copie, parce que `_init` a construit `fv`.
- Mesures faites par grep sur `src/`, `tests/`, `tools/` et `Makefile` à 982cd0e, en lisant :
  - `pico8/lua.lua:1-16`, `:37-41`, `:85-97`, `:153-165`, `:200-330`, `:405-448`, `:860-1000`, `:1160-1200` ;
  - `pico8_ui_button.c/h`, `pico8_msgbox.c` (`pz`, `pz_step`, `pico8_msgbox_truthy`), `pico8_cart.c/h` (`eu`, `eu_ix_map`, `eu_dispatch`), `pico8_menu.c` (`cr`, `_cr_t`, `ix*`, `ha*`), `pico8_planet_display.c` (`jo:gh`, `jo:fe`, `_jo_fe_*`), `pico8_help.c` (`fv:pq`, `bo`), `pico8_score_panel_display.c` (`pj:ph`, `dk.px`), `pico8_galaxy.c` (`dk.en`), `pico8_economy_score.c/h` (`dk.trade`, `set_dk`), `pico8_economy_tiers.c` (`bb_visit`), `pico8_planet_family.c` (`by` de `ga`), `pico8_entity.c` (`lt`, `he`, `mv`, `dc`, API double), `pico8_struct.h`, `pico8_ke.c/h`, `pico8_ref_audit.h`, `main.c`, `main_class_coverage.c` ;
  - les cinq tests nommés, les 20 fichiers du harnais, `test_pico8_entity_smoke.c`, `test_pico8_ie.c`, `test_pico8_fq.c`, `test_pico8_bg.c`, et `tools/class_coverage_net_expected.txt` ;
  - pour les bugs d'origine : `git show 3b8ba2c`, `9183fee`, `8361468`, `50f91b4`, et `git log -S` sur `pico8_ha2_my_dispatch`/`hovered_my` (sans correctif trouvé pour `pause_menu_music`).
