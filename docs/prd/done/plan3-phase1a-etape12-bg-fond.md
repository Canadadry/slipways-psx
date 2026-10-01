---
title: "Plan 3, étape 1a.12 : `bg` (le fond) devient une struct, `pl` lu sur la classe"
description: "Le fond `bg` est encore une instance de la couche objet : il passe en struct fille de `ke` avec ses méthodes `kj`, `ldown`, `rdown` (l'enum gagne `RDOWN`) et `hq`. `pl` devient une table possédée par le module et lue sur la classe. `bg:ldown` continue de construire un `mz` struct ou un `g_` instance par `lookup_named`, sans changer."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.12 — `bg` (`bg.c:69`), `pl` lu sur la classe, `ldown`/`rdown`, `lookup_named("mz"|"g_")` (`bg.c:185-191`). Tests : aucun.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Clés construites à l'exécution** : `self[bt.."held"]` (`pico8_ui.c:328`) → deux champs `lheld`/`rheld` ; `ldown`/`rdown` côté méthodes. […]
>
> **Défauts mutés sur la classe** : […] deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** […].
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe. […]
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde qu'une copie en lecture pour les descendants non convertis.

§6 : l'étape ne nomme aucun test de régression. Ceux qui construisent un `bg` (tous ceux qui lancent `eu()`) ne lisent aucun de ses champs et doivent passer avec un diff vide. §7 : le commit passe `make class-coverage-net` avec une sortie identique à `tools/class_coverage_net_expected.txt`. Un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la ligne de l'étape dans le commit, avec « découvert à 1a.12 » dans le message.

### Ce que 1a.1 à 1a.11 ont posé (commits 9ce8bfa, 42fdf50, f5cc6d1, 28a08ac, 824725c, ec01e86, 470accc, 9aa4812, 968e059, 3f62fe5), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, et le descripteur `pico8_struct_class_t` : des méthodes indexées par l'enum (aujourd'hui `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`), un lecteur de champ qui rend 1 si le champ n'est pas nil, et un écrivain de champ. Un nom absent de l'enum, ou dont l'entrée est NULL, est cherché comme champ.
- `pico8_ke_t` en premier membre, avec `pico8_ke_struct_init`, `pico8_ke_field` et `pico8_ke_struct_pq`. Une fille appelle ce dernier une fois, après son propre `pq`.
- `pico8_class_set_struct_new` : `pico8_class_instantiate` délègue au constructeur de struct, qui enregistre la couverture.
- `pico8_entity_get_field` et `pico8_entity_set_field` sont à double forme. L'écrivain asserte s'il n'y a pas d'écrivain ou si le nom n'a pas de membre. `pico8_instance_get_field` asserte s'il reçoit une struct.
- Sont déjà à double forme : `lt`/`he`, `dg` (qui écrit `t` directement dans `pico8_ke_t`), `dc` (`fa`, `ra`), `mv`, la boucle de survol `gw` (1a.5 : `mv(e,"hd")`, `fa`, écritures de `m_` sur `o_`/`gj`, lecture de `fg`), `ui:ev` (lecture de `ql` sur `o_`), `jz`, `jc:hq` (lecture de `o_.gv`, 1a.5), le lecteur de `s.qu` dans `jo:gv`/`nr:gv` (1a.10) et le scan `fx` de `mz:pq`. `bg:hq` écrit `mo.v` par `pico8_entity_set_field` depuis 1a.8.
- **Modèle d'une classe dont un défaut vaut 1 et sert de réponse à `hd` : `fq` (1a.5).** `hd` est un membre fixe que `mv` rend tel quel. `m_` est un membre, et l'écrivain n'accepte que lui.
- **Modèle d'une classe avec un `pq` propre : `iv` (1a.2), `mz` (1a.10).** Le constructeur appelle le `pq` de la classe, puis `pico8_ke_struct_pq`. `pico8_class_set_pq` disparaît.
- **Modèle d'un défaut lu sur la classe et jamais modifié : `jj` de `mz` (1a.10).** Le code lit la valeur du module, jamais celle de `self`. Le membre existe quand même (règle « chaque champ de sa chaîne ») et pointe sur la même table. **Modèle des tables partagées possédées par le module : `g_mh_nu` (1a.4), `g_qb_pc` (1a.8), `nu`/`kk` de `mz` (1a.10).** La table est construite à l'init et libérée au shutdown.
- Modèle de test : `test_pico8_qb` et `test_pico8_mz`. La caractérisation est figée sur l'ancien code et lue seulement par l'API à double forme. Les rejeux tournent dans un fils forké juste après `pico8_cart_load()` (`in_fresh_child`). La vérification de forme s'ajoute après la conversion, et l'écriture interdite est vérifiée dans un fils. Les mutations sont notées dans le commit.

### Ce que fait le cart

`pico8/lua.lua` :

```
:359-362  bg=ke:jr([[ fa=1, hd=1, pl=o(0,0,0,72,73,74,75,88,89,90,91,88,91,89,91), ]])
:363-372  function bg:pq() … mset(x,y,rnd()<0.004 and 122+rnd(5) or bg.pl[flr(vh+rnd(4))]) …
:373-375  function bg:kj() map(…) fillp(…) rectfill(…) fillp() rect(…) end
:376-377  function bg:ldown(mp) return (#(cg.g_ or {})>0 and mz or g_)() end
:378-380  function bg:rdown(mp,rp) self.mq=rp return self end
:381-383  function bg:hq(mp,rp) mo.v=self.mq-rp self.mq=rp end
:1182     function eu(nj) … jl() bg() bw(90) …
:219-223  ui:lj : self:gw() … gu(le,self.o_) … self.gj=self.o_
:225      ui:ev : self.rheld and 9 or self.o_ and self.o_.ql or self.ql
:226-236  ui:gw : for e in all(cv.hd) do if mv(e,"hd",…) then ft(self.ic,e.fa,e) self.o_=e end end
            (self.gj or {}).m_=false (self.o_ or {}).m_=true poke(0x5f80,self.o_ and self.o_.fg or …)
:237-247  ui:bh(bt) : jz(bt.."down",…) → self[bt.."held"] ; mv(held,"hq",…) ; jz("gv",held) ; mv(held,"ey",…)
:472-477  jo:gv(s) … s.qu==self …        :1109-1111 nr:gv(s) return s.qu~=self and #self.oe<3
```

Points décisifs :

- **`pl` est lu sur la classe** (`bg.pl[…]`), jamais sur l'instance, et sa table n'est **jamais modifiée en place** : aucun `pl` n'est écrit ailleurs, ni dans le cart ni dans `src/`. `fa` et `hd` ne sont jamais réaffectés. **Aucun défaut de `bg` n'est muté**. L'étape n'introduit donc aucun état statique *mutable*, et n'ajoute rien à la liste des défauts mutés du §5.
- `bg` n'a pas de `nu`, et `ke` non plus (spec `me="ph",t=0`). Un `bg` est donc enregistré dans `hv`, `cv.kj` et `cv.hd`, et **dans aucun `cg`**.
- `hd=1` est une valeur, pas une méthode : `mv(bg,"hd",…)` rend 1, donc le survol touche toujours. Comme `bg` est le premier de `cv.hd` (construit par `eu()` juste après `jl()`) et que `gw` garde le **dernier** touché, `ok.o_` vaut `bg` chaque fois que rien d'autre n'est survolé. `gw` écrit alors **`m_` sur `bg`** (vrai sur `o_`, faux sur `gj`), lit `fg` (absent), et `ui:ev` y lit `ql` (absent).
- `ldown` et `rdown` sont appelées par nom de chaîne construit (`jz(bt.."down")`). L'ensemble est fini : `bt` ∈ {`l`,`r`}, assert à `ui.c:81-82`. `hq` et `ey` le sont par `ui:bh` (`mv(held,…)`), `kj` par `dc`. **`rdown` n'est dans l'enum que si l'étape l'y met** : aujourd'hui, seul `bg` a un `rdown`.
- `bg:rdown` rend `self`, qui devient `ok.rheld`. Au relâcher, `ui:bh` appelle `jz("gv", bg)`. Chaque `jo`/`nr` survolé lit alors **`s.qu` sur `bg`** (absent) : `nr:gv` rend vrai (`nil ~= self`), puis `mv(bg,"ey",nr)` ne trouve rien. `jz("gv")` appelle aussi `mv(bg,"gv")` sur `bg` lui-même (absent).
- `bg:ldown` construit par `lookup_named` un `mz`, **déjà struct** par la délégation `struct_new` (1a.10), ou un `g_`, **encore instance** (1a.27). Le résultat part en REF dans `ok.lheld`. Les deux formes passent déjà par ce chemin, et il reste tel quel.

### Emprise réelle de `bg` (mesurée sur 3f62fe5)

| Aspect | Mesure |
|---|---|
| Constructions | Un seul site en jeu : `pico8_eu` (`cart.c:203`, `(void)pico8_bg_new(NULL)`), une fois par partie lancée. Aucun registre `fz` (la spec n'a pas de `fz`) ni nommé, aucun `gx[]`, aucun `kv[]`. Déclarée par `pico8_bg_init` (`cart.c:70`). `cart.c:97` asserte `pico8_bg_class()->parent == pico8_ke_class()` (reste vrai). Aucun test ne construit de `bg` directement : ils passent tous par `eu()`. |
| Défauts | `fa`=1, `hd`=1, `pl`=table de 15 entiers `0,0,0,72,73,74,75,88,89,90,91,88,91,89,91`. Hérités de `ke` : `me`=`"ph"`, `t`=0. Aucun défaut muté. |
| Champs écrits sur l'instance | `mq` (`bg:rdown` `bg.c:200`, `bg:hq` `bg.c:228`, par `self->fields`) ; `m_` (`gw`, `ui.c:308,313`, par `pico8_entity_set_field`) ; `t` (`dg`). |
| Méthodes | `pq` (`pico8_class_set_pq`, `bg.c:72`), plus `kj`, `ldown`, `rdown` et `hq`, **installées aujourd'hui par `->defaults`** (`bg.c:73-76`). Dans l'enum, `KJ`, `LDOWN` et `HQ` existent déjà : **l'enum gagne seulement `RDOWN`**. `pq` n'y entre pas, car elle n'est appelée que directement. |
| `->defaults` | 5 sites, tous dans `bg.c` : 4 installations de méthode (`:73-76`) et 1 lecture de `pl` (`:115`, `bg:pq`). Aucun `g_bg_class->defaults` ni `pico8_bg_class()->defaults` ailleurs dans `src/` ou `tests/`. |
| Noms lus sur un `bg` par un consommateur générique | `lt`/`he` : `kj`, `ev`, `hd`, `hk`, `hb`, `nu`. `dg` : `me`, `t`, `p_`, et `mv("ph")` (absent). `dc("kj")` : `fa`, `ra` (absent), `mv("kj")`. `gw` : `mv("hd")`, `fa`, `fg` (absent). `ui:ev` : `ql` (absent). `jz` : `mv("ldown")`, `mv("rdown")`, `mv("gv")` (absent). `ui:bh` : `mv(held,"hq")`, `mv(held,"ey")` (absent). `jo:gv`/`nr:gv` : `qu` (absent). `jc:hq` : `o_.gv` (absent). `mz:pq` : `fx` (absent). **Tous passent déjà par l'API à double forme.** |
| Écrivains génériques qui atteignent un `bg` | `gw` : `m_` (`ui.c:308,313`), déjà par `pico8_entity_set_field`. Aucun autre : `bv` ne pose `fo` que sur une planète, une voie ou un `{}`, et `jc:hq` ne prend `o_` comme `re` que si `o_.gv` est vrai. |
| Lecteurs à simple forme qui atteignent un `bg` | Seulement ceux de `bg.c` : `pico8_bg_field_vec(self,"mq")` (`:218`, `pico8_instance_get_field`), qui **assertera après conversion**, et les écritures `self->fields` de `mq` (`:200,228`), qui seraient **hors bornes** sur une struct. Ils sont réécrits sur les membres. |
| Consommateurs génériques jamais atteints | `cv.ev`/`hk`/`hb` (`economy`), `cg` (`bg` n'a pas de `nu`), `ij` (listes `cg.np`/`ox` : `bg` n'a pas de `fx`), `bv`/`dl`/`d_`, `ui:cj` (`le` ne tient que des `js`). |
| Clé dynamique | `bt.."down"` → `ldown`/`rdown`, ensemble fini (règle « Clés construites à l'exécution »). `bt.."held"` est sur `ok` (1a.31), pas sur `bg`. |
| `pico8_ref_audit.h` / casts | Aucune macro ne reçoit un `bg`. `ui.c:357` (TROU-6) caste `rheld` en pointeur d'entité sans le déréférencer ; `mv` lit ensuite le tag. |
| `->cls` lu sur une entrée de `hv` | `test_pico8_geoharvest_burst.c:85` (`live_ga`, sans vérifier le tag). `bg` précède tout `ga` dans `hv` et y est donc lu à chaque appel. La lecture reste dans les bornes : un `bg` fait bien plus de 24 octets, et l'octet 16 tombe sur `ke.me`. Elle n'est jamais égale à la classe `ga` et n'est jamais déréférencée. C'était déjà vrai pour `iv`/`mh`/`fq`/`nf`/`qb`/`mz`, et la réécriture est prévue en 1a.26. `test_pico8_qb.c:390`, `score_panel_close` et les tests de forme vérifient le tag. |
| Tests qui atteignent un `bg` | Tous ceux qui lancent `eu()`, sans lire ses champs. `test_pico8_fq.c:267` appelle `jz("rdown")` (qui touche `bg`) et `:398` vérifie que `mv(fq,"rdown")` est absent : le résultat reste identique avec `RDOWN` dans l'enum, car l'entrée est NULL sur `fq` et le lecteur rend absent. `test_pico8_mz.c:345,351,829-830` appelle `pico8_bg_ldown(NULL, …)`. `test_pico8_bg_kj.c:35,57` appelle `pico8_bg_kj(&gfx,&mem,NULL)`. `test_pico8_qb` rejoue `long_session` jusqu'à la frame 4800, ce qui passe par `bg:hq` (glissé droit à partir de la frame 4741). |
| Filet | `long_session_events.json` a 513 frames au bouton droit (7 glissés, frames 4741-11700) : `bg:rdown`/`bg:hq` sont vus par le filet. Les 15 rejeux construisent un `bg` (`bg  19  15/15`). |
| Couverture | `bg  19  15/15` (inchangé attendu). |

## Solution

1. **Struct `bg`** (publique dans le module du fond, comme `pico8_qb_t`/`pico8_mz_t`) : premier membre `pico8_ke_t`, puis les défauts de la chaîne `fa`, `hd`, `pl`, puis les champs écrits sur instance `mq`, `m_`. Tous sont des `pico8_value_t` et gardent leur nom minifié. `me` et `t` sont ceux de `pico8_ke_t`.
2. **Table de classe possédée par le module** : `pl` (15 entrées) est construite par `pico8_bg_init` à partir du même fragment de spec, et libérée par `pico8_bg_shutdown`. **`pl` est lu sur la classe** : `bg:pq` indexe la table du module, jamais `self->pl`. Le membre `pl` de chaque struct est une REF vers cette même table, ce qui tient la règle du §5 ; aucun code ne lit ce membre.
3. **Valeurs initiales** : après `pico8_ke_struct_init`, `me` = `"ph"` et `t`=0 (ceux de `ke`) ; `fa`=1, `hd`=1 ; `pl` = REF vers la table du module ; `mq`, `m_` = nil.
4. **Constructeur** (posé comme `struct_new` de la `pico8_class_t` de `bg`), dans l'ordre :
   1. alloue ;
   2. appelle `pico8_ke_struct_init` avec le descripteur de `bg` ;
   3. pose les valeurs initiales ;
   4. asserte que la table reçue est NULL ou vide, avec un message (arrêt §7) ;
   5. enregistre `"bg"` au crochet de couverture ;
   6. appelle `bg:pq` (qui peint les 32×32 tuiles avec le `mem` du contexte de dispatch) ;
   7. **puis** `pico8_ke_struct_pq`.

   C'est l'ordre du parcours de classe, qui consomme `rnd` dans le même ordre. `pico8_class_set_pq(g_bg_class, …)` disparaît.
5. **Enum de méthodes** : il gagne `RDOWN` (`"rdown"`), et rien d'autre. Sur tout autre descripteur, l'entrée est NULL : le nom retombe sur le lecteur de champ, qui le rend absent comme aujourd'hui. `rdown` n'est pas un nom `bm`, donc `lt`/`he` ne changent pas.
6. **Descripteur de `bg`** : méthodes `KJ`, `LDOWN`, `RDOWN`, `HQ`. Son lecteur de champ connaît les 5 membres propres, puis délègue à `pico8_ke_field`. Son écrivain n'accepte que `m_` (écrit par `gw`), et rend 0 pour tout autre nom : l'assert de `pico8_entity_set_field` part alors. Un accesseur public du descripteur s'ajoute, comme `pico8_qb_struct_class()`.
7. **Méthodes sur la struct** : `pico8_bg_pq`, `pico8_bg_kj`, `pico8_bg_ldown`, `pico8_bg_rdown` et `pico8_bg_hq` prennent la struct `bg` et gardent leurs gardes NULL (un `self` NULL reste un no-op sûr, ce que `test_pico8_mz` et `test_pico8_bg_kj` utilisent), leurs adaptateurs et le TROU-7 existant.
   - `rdown` écrit `rp` tel quel dans le membre `mq` et rend `self` en REF.
   - `hq` lit le membre `mq`, dont le nil ou le mauvais genre se lit comme le vecteur nul, comme aujourd'hui. Il écrit `mo.v` par `pico8_entity_set_field` (inchangé), puis `rp` dans `mq`.
   - `pq` lit la table `pl` du module. Un index hors 1..15 peint la tuile 0, comme aujourd'hui.
   - `ldown` garde **à l'identique** `pico8_cg("g_")`, `pico8_class_lookup_named(count > 0 ? "mz" : "g_")`, `pico8_class_instantiate(cls, NULL)` et le retour en REF. Il construit un `mz` struct ou un `g_` instance selon la forme de chacune.
   - Les helpers de lecture de `bg.c` que la conversion rend inutiles disparaissent (`pico8_bg_field_vec`).
8. **`pico8_bg_init` / `_shutdown`** : même déclaration, même spec, même parent, même nom de couverture. La classe reçoit le constructeur et perd les 4 installations `->defaults` et la lecture `->defaults` de `pl`. Le shutdown libère la classe, puis la table `pl` du module. Il ne libère jamais un `bg`.
9. **`pico8_bg_new`** rend la struct (comme `pico8_qb_new`). `cart.c:203` ne change pas.
10. **Hors du module** : aucun changement de code dans `src/` en dehors de `bg.c`, `bg.h` et `pico8_struct.h` (enum). Chaque consommateur générique est déjà à double forme (voir l'emprise). Les signatures prennent désormais la struct, et les appels existants avec `NULL` (`test_pico8_mz`, `test_pico8_bg_kj`) compilent sans changement.
11. **Plan** : ajouter à la ligne 1a.12 de `docs/plan-3.md` le texte « Découvert à 1a.12 : `m_` écrit sur `bg` par la boucle de survol `gw` (`ui.c:308,313`) → membre ; `rdown` entre dans l'enum. » Aucun défaut muté ne s'ajoute aux défauts connus du §5.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `bg` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise de `bg` soit mesurée dans le PRD (constructions, défauts, champs écrits, méthodes, `->defaults`, lecteurs et écrivains génériques, tests, filet), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux que la struct `bg` n'ait que `pico8_ke_t` et les 5 membres `fa`, `hd`, `pl`, `mq`, `m_`, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun défaut de `bg` ne devienne un état statique *mutable*, afin de n'appliquer la règle des défauts mutés que là où le cart mute (ici, nulle part).
5. En tant que mainteneur, je veux que `pl` soit une table possédée par le module, construite à l'init et libérée au shutdown, afin qu'aucun accès `->defaults` ne subsiste pour `bg`.
6. En tant que gardien du filet, je veux que `bg:pq` indexe la table `pl` du module (lue sur la classe), afin que le champ d'étoiles soit peint tuile pour tuile comme avant.
7. En tant que gardien du filet, je veux que `bg:pq` s'exécute avant l'enregistrement dans `hv` et consomme `rnd` dans le même ordre, afin que la carte et tout tirage ultérieur restent identiques.
8. En tant que gardien du filet, je veux qu'un `bg` neuf lise `me`=`"ph"`, `t`=0, `fa`=1, `hd`=1 et `pl` dans l'ordre `0,0,0,72,73,74,75,88,89,90,91,88,91,89,91`, avec `mq` et `m_` absents, afin que son état de départ soit identique.
9. En tant que gardien du filet, je veux qu'un `bg` neuf soit enregistré dans `hv`, `cv.kj` et `cv.hd`, et dans aucun `cv.ev`/`hk`/`hb` ni `cg`, afin que l'ordre de dessin et de survol ne bouge pas.
10. En tant que gardien du filet, je veux que `mv(bg,"hd",…)` rende la valeur 1, afin que le fond réponde toujours au survol et devienne `ok.o_` quand rien d'autre n'est survolé.
11. En tant que gardien du filet, je veux que `gw` puisse écrire `m_` vrai puis faux sur un `bg` struct, afin que la boucle de survol ne fasse pas partir l'assert de l'écrivain.
12. En tant que gardien du filet, je veux que `jz("ldown")` trouve `bg:ldown` par la table via l'enum, afin qu'un clic gauche dans le vide lance une sonde ou un trou de ver comme avant.
13. En tant que gardien du filet, je veux que `bg:ldown` construise un `g_` (instance) quand `cg.g_` est vide et un `mz` (struct) sinon, par `lookup_named`, afin que les deux formes passent par le même chemin sans que `ldown` les distingue.
14. En tant que gardien du filet, je veux que `jz("rdown")` trouve `bg:rdown` par la nouvelle entrée `RDOWN`, qui rend `bg` en REF, afin que `ok.rheld` tienne le fond pendant un glissé droit.
15. En tant que gardien du filet, je veux que `bg:rdown` range `rp` tel quel dans `mq`, afin que le premier `hq` du glissé calcule le même delta.
16. En tant que gardien du filet, je veux que `bg:hq` écrive `mo.v = mq - rp` puis `mq = rp`, afin que la caméra glisse identiquement (`long_session`, frames 4741 et suivantes).
17. En tant que gardien du filet, je veux qu'un `hq` sans `mq` préalable lise le vecteur nul, afin que le comportement existant sur champ absent soit gardé.
18. En tant que gardien du filet, je veux qu'au relâcher d'un glissé droit, `jz("gv", bg)` lise `s.qu` sur `bg` par le lecteur à double forme et que `mv(bg,"ey")` ne trouve rien, afin que rien ne se passe, comme avant.
19. En tant que gardien du filet, je veux que `dc("kj")` dessine le fond par `bg:kj` au calque 1, afin que le rendu soit identique au pixel près.
20. En tant que gardien du filet, je veux que `dg` appelle `mv(bg,"ph")` sans rien trouver et incrémente `t` dans le `pico8_ke_t`, afin que le fond ne change jamais d'état.
21. En tant que mainteneur, je veux que l'écrivain de `bg` n'accepte que `m_`, afin qu'une écriture générique d'un autre nom déclenche l'arrêt du §7 au lieu d'un membre improvisé.
22. En tant que mainteneur, je veux qu'une clé reçue par le constructeur déclenche une assertion porteuse de message, afin qu'un champ jamais mesuré ne soit pas perdu en silence.
23. En tant que gardien de la couverture, je veux que le constructeur enregistre `"bg"` une fois, jamais `"ke"`, afin que `bg  19  15/15` et `ke  0  0/15` restent identiques.
24. En tant que mainteneur, je veux que la `pico8_class_t` de `bg` reste déclarée à la même place, avec sa spec, son parent et son nom de couverture, afin que `KNOWN[]` et l'assert de `cart.c` tiennent.
25. En tant que mainteneur, je veux que les 5 accès `->defaults` de `bg.c` disparaissent, afin d'appliquer la règle des accès directs aux défauts.
26. En tant qu'auteur des étapes suivantes, je veux que l'enum ne gagne que `RDOWN`, afin que chaque ajout reste attaché à une étape.
27. En tant qu'auteur de 1a.24 (`nr`, `ldown`) et 1a.31 (`ui`), je veux que `ldown`/`rdown` soient des entrées d'enum ordinaires, afin que la clé construite `bt.."down"` reste un ensemble fini.
28. En tant qu'auteur de 1a.27 (`g_`), je veux que `bg:ldown` n'ait aucune connaissance de la forme de `g_`, afin que la conversion de `g_` passe par la délégation `struct_new` sans toucher `bg.c`.
29. En tant que gardien de la mémoire, je veux que le shutdown libère la classe puis la table `pl`, jamais un `bg`, afin que TROU-7 reste la seule convention de fuite.
30. En tant que gardien des tests, je veux que `test_pico8_bg_kj`, `test_pico8_mz` et `test_pico8_fq` passent avec un diff vide, afin que la conversion ne se valide pas en réécrivant des tests existants.
31. En tant que gardien des tests de régression, je veux que tous passent avec un diff vide, afin que la conversion ne se valide pas en affaiblissant un test.
32. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
33. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.12, afin que la liste des étapes dise ce que l'étape a réellement touché.
34. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés, afin de ne laisser aucun contrat mensonger.
35. En tant que vérificateur, je veux une caractérisation du fond figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`mq`, `m_`, `mo.v` hors écran, forme du résultat de `ldown`).

## Implementation Decisions

Aucun outil de création d'agent n'était disponible dans cette session pour lancer un agent `guideline` frais. J'ai donc chargé le skill `guideline` et je l'ai appliqué moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Pas d'état statique mutable ; une table de classe en lecture seule.** La preuve est au Problem Statement : `pl` n'est ni réaffecté ni modifié en place, et `fa`/`hd` non plus. Si l'implémenteur trouve une modification en place (un `pico8_ob_table_set_*` sur `pl`), c'est la règle des défauts modifiés en place qui s'applique : site découvert, état statique partagé, ligne 1a.12. Ce n'est pas un membre.
- **(G) `pl` : lu sur la classe, et aussi membre.** Le texte de l'étape impose la lecture sur la classe, et le §5 impose un membre par défaut de la chaîne. Même arbitrage que `jj` en 1a.10 : le coût est une REF que rien ne lit. Rejeté : pas de membre `pl`. Ce serait un défaut de la chaîne hors de la règle des membres, donc un risque d'arrêt §7.
- **(G) Table du module** : même mécanisme que `g_qb_pc`, parsée par `pico8_ob` depuis le fragment de la spec. Rejeté : continuer de lire `g_bg_class->defaults`, parce que c'est un accès `->defaults` que le §5 fait migrer, et que `bg` n'a aucun descendant. La `pico8_class_t` garde sa spec telle quelle, comme copie en lecture jamais lue.
- **`m_` est un membre, et l'écrivain l'accepte.** C'est un champ écrit sur l'instance (par `gw`), donc un membre par la règle du §5. Il n'est pas nommé par la ligne de l'étape, d'où la mention « découvert à 1a.12 ». Même traitement que `fq` (1a.5). Aucun code ne lit `m_` sur un `bg` : le membre existe pour tenir l'écriture.
- **`mq` est écrit directement** par `rdown`/`hq`, qui sont les méthodes propres de `bg`. Aucun écrivain générique ne l'écrit, donc l'écrivain ne l'accepte pas (G) : une écriture générique de `mq` serait un site non mesuré, donc un arrêt.
- **Enum** : seul `RDOWN` s'ajoute, parce que `rdown` est appelé par nom de chaîne (`jz(bt.."down")`). `pq` n'y entre pas : elle n'est appelée que directement. `hd` reste un champ (valeur 1) et n'entre pas dans l'enum. Les `hd` fonctions de `js`/`jo`/`ie`/`fv` sont des instances, jusqu'à leurs étapes.
- **`pq` dans le constructeur** : modèle `iv`/`mz`, dans l'ordre du parcours de classe (`bg:pq`, puis `lt`). Placer `lt` avant `pq` ne changerait rien d'observable aujourd'hui (`pq` ne lit pas `hv` et `lt` ne tire pas `rnd`), mais l'ordre des effets est gardé tel quel.
- **`bg:ldown` n'est pas modifié dans sa logique.** `lookup_named` et `pico8_class_instantiate` construisent la forme de chaque classe par délégation. C'est exactement ce que la ligne de l'étape demande de garder pour `mz` (struct) et `g_` (instance). Seul le type de `self` change.
- **(G) Aucun changement hors du module et de l'enum.** Tous les consommateurs génériques qui atteignent un `bg` sont déjà à double forme (mesuré). Réécrire l'un d'eux serait hors de l'étape.
- **`live_ga` de `geoharvest_burst`** (G) : laissé tel quel, comme en 1a.10. La lecture reste dans les bornes, n'est jamais égale et n'est jamais déréférencée. La réécriture est prévue en 1a.26.
- **`test_pico8_entity_smoke`** : il n'est pas touché (il ne construit aucun `bg`).
- **Commentaires** (CLAUDE.md) :
  - À ajouter : le contrat de la struct (jamais libérée, TROU-7 ; `pl` partagée, jamais modifiée en place), le contrat de `pico8_bg_new` (rend une struct, table reçue vide seulement), l'assert porteuse (clé reçue), et le contrat de l'écrivain (`m_` seulement).
  - À corriger, parce que le changement les rend faux : `bg.h` « Frees the `bg` class, never any `bg` instance » (libère aussi `pl`), « `overrides` is copied, not kept » et « `pl` is read off the class defaults » (il vient désormais de la table du module), ainsi que le commentaire de `pico8_bg_field_vec`, qui disparaît avec le helper.
  - Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation de `bg` sur l'ancien code.** Nouveau test (voir Testing), valeurs figées **avant** toute modification de `bg.c`, `bg.h` et `pico8_struct.h`.
2. **`bg` converti.** Enum `RDOWN`, struct, table `pl` du module, descripteur, constructeur, méthodes sur la struct, classe reliée, `->defaults` retirés, accesseur, ligne 1a.12 du plan. La caractérisation passe **sans changer ses valeurs figées**, et la vérification de forme s'ajoute.

Il n'y a pas de morceau « coexistence » : il n'y a rien à rendre double hors du module.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field` et `pico8_mv`, l'appartenance à `hv`/`cv`/`cg`, la carte (`mget`), `mo.v`, `ok.o_`/`ok.rheld`/`ok.lheld`, les pixels du framebuffer et le compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_bg`** (dans `make unit`, lié comme `test_pico8_qb`/`test_pico8_mz`, sur le cart réel chargé). Il lit **uniquement** par l'API à double forme (`pico8_entity_get_field`, `pico8_entity_set_field`, `pico8_mv`, `pico8_ui_jz`, `pico8_ui_lj`, `pico8_hv`/`pico8_cv`/`pico8_cg`, `pico8_dg`, `pico8_dc`, `pico8_bg_new`, `pico8_class_instantiate(pico8_bg_class(), NULL)`), pour que le code soit le même avant et après. Le `bg` de `eu()` se trouve comme le voile de `test_pico8_fq` : l'entité de `cv.hd` dont `hd` et `fa` valent 1. Chaque scénario qui dépend de l'ordre `rnd` tourne dans un fils forké juste après `pico8_cart_load()` (`in_fresh_child`).
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction** (partie lancée par `eu()`), sur le `bg` de `eu()`, sur `pico8_bg_new(NULL)` et sur `pico8_class_instantiate(pico8_bg_class(), NULL)` :
       - lire `me`, `t`, `fa`, `hd` et les 15 entrées de `pl` ;
       - vérifier que `mq`, `m_`, `ra`, `p_`, `nu`, `fg`, `ql`, `qu`, `gv`, `ey`, `fx` et `ph` sont absents ;
       - vérifier que `kj`, `ldown`, `rdown` et `hq` sont des fonctions ;
       - vérifier l'appartenance à `hv`, `cv.kj` et `cv.hd`, et à aucun de `cv.ev`/`hk`/`hb` ;
       - vérifier que `cg("bg")` est NULL et que deux `bg` rendent le même `pl`.
    2. **Carte** : dans un fils frais, `eu()` puis empreinte des 32×32 `mget`, avec le nombre de tuiles 122..126 et de tuiles 0. Puis un second `pico8_bg_new(NULL)`, et la nouvelle empreinte. Toutes les tuiles sont dans `pl` ∪ {0} ∪ 122..126. Figées.
    3. **`rdown`/`hq`** :
       - `mv(bg,"hq",…)` sans `mq` : lire `mo.v` (le vecteur nul moins `rp`) ;
       - `mv(bg,"rdown",mp,rp1)` rend `bg` en REF, et `mq` est la même REF que `rp1` ;
       - trois `hq` successifs avec `rp2`, `rp3`, `rp4` : `mo.v` et `mq` après chacun.
       Empreinte figée.
    4. **`ldown`** : `mv(bg,"ldown",…)` avec `cg.g_` vide rend un `g_` (tag `INSTANCE`, dans `cg.g_`) ; un second appel rend un `mz` (tag `STRUCT`, dans `cg.mz`, `me`=`"kz"`). Les comptes de `cg.g_`/`cg.mz` avant et après sont figés.
    5. **Survol et boutons** (partie lancée, `je.ra` dans le vide) :
       - `pico8_ui_lj` : `ok.o_` est le `bg`, `m_` du `bg` est vrai, le `bg` est au calque 1 de `ok.ic`, et `0x5f80` est lu ;
       - `jz("ldown")` et `jz("rdown")` rendent `bg` comme `kv` ;
       - un `fq` construit ensuite prend `o_`, puis `ui_lj` repasse `m_` du `bg` à faux par `gj`.
       Empreinte figée.
    6. **`gv` sur un `bg`** : `pico8_jo_gv(planète "ly", bg)` rend nil sans appeler `gh`, et `pico8_nr_gv(nr, bg)` rend `#oe<3` si la partie scriptée fournit un `nr`. Sinon, le cas est noté comme omis dans le commit.
    7. **Dessin** : `dc("kj")` sur un framebuffer effacé, avec seulement la carte du `bg` (`mo` à sa position initiale). Empreinte du framebuffer, figée.
    8. **Rejeu de `long_session_events.json`** (depuis le titre, piloté comme `test_replay_from_title` de `test_pico8_qb`) jusqu'à la fin du deuxième glissé droit (frame 4984) : par frame, pour le `bg` courant, `mq` (vecteur ou absent), `m_`, `t`, puis `ok.rheld == bg`, `ok.o_ == bg` et `mo.v`, repliés en empreinte de séquence et nombre de frames. Figés.
    9. **Rejeu de `phase4_playthrough_events.json`** (non-titre, comme `test_pico8_playability`) sur toutes ses frames : par frame, `ok.o_ == bg`, `m_` du `bg`, et le genre de `ok.lheld` (absent, `g_` instance, `mz` struct, autre), repliés en empreinte. Figés.
  - **Forme** (ajoutée au morceau 2) :
    - le `bg` de `eu()` et ceux de `pico8_bg_new`/`pico8_class_instantiate` sont tagués `STRUCT` avec le descripteur de `bg` (accesseur) ;
    - `pico8_mv(bg,"kj"|"ldown"|"rdown"|"hq",…)` trouve une méthode, et `mv(bg,"ph"|"ey"|"gv"|"md"|"ol"|"ev")` ne trouve rien ;
    - `pico8_entity_set_field(bg,"m_",…)` passe ;
    - `pico8_entity_set_field(bg,"mq",…)` et `(bg,"fa",…)` assertent (vérifié dans un fils, `SIGABRT`) ;
    - `pico8_mv(fq,"rdown")` reste absent.
  - **Mutations** : l'implémenteur remet localement au moins 10 erreurs plausibles et note dans le commit celles qui sont attrapées :
    - `RDOWN`, `LDOWN`, `HQ` ou `KJ` absent de la table ;
    - `RDOWN` pointé sur `ldown` ;
    - `pl` dans le désordre, ou une entrée fausse ;
    - `fa` ≠ 1 ;
    - `hd` absent ;
    - écrivain qui refuse `m_` ;
    - `rdown` qui rend nil au lieu de `self` ;
    - `hq` qui n'écrit plus `mq` ;
    - `hq` qui fait `rp - mq` ;
    - `ldown` aux branches inversées ;
    - `pico8_ke_struct_pq` omis ;
    - `bg:pq` omis ;
    - `me` initial différent de `"ph"`.

    Chaque mutant équivalent éventuel est justifié dans le commit. Par exemple : `lt` avant `pq` (ni l'un ni l'autre n'observe l'effet de l'autre) ; `pq` qui lit `self->pl` au lieu de la table du module (même table, par construction) ; membre `pl` à une valeur fausse (aucun code ne le lit). Ces cas se disent plutôt que de se tester.
- **Tests existants** : `test_pico8_bg_kj`, `test_pico8_mz`, `test_pico8_fq`, `test_pico8_qb`, `test_pico8_iv`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo` et `test_pico8_mh` ont un diff vide, et leurs valeurs figées sont identiques.
- **Tests de régression** : tous passent avec un diff vide, en particulier `playability`, `slipway_drag`, `geoharvest_burst`, `pause_menu_music`, `score_panel_close`, `empty_planet_cleared`, `game_end` et `tech_screen`.
- **Couverture** : `make class-coverage-net` rend 0, avec une sortie identique à la référence (`bg  19  15/15`, `ke  0  0/15  exemptée …`).
- **Prior art** : `test_pico8_fq` (voile `hd=1`, boucle de survol, `m_`, `jz("ldown"|"rdown")`), `test_pico8_qb` (rejeu `long_session` depuis le titre, `mo.v` par `bg:hq`, fils forkés, écriture interdite dans un fils) et `test_pico8_mz` (`pq` propre, table du module lue sur la classe, `bg:ldown`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0, ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`, en particulier si `bg` n'est pas `bg  19  15/15`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé de `bg` ne tombe sous aucune règle du §5. En particulier : si l'assertion des clés reçues ou celle de l'écrivain générique se déclenche sur un rejeu ou un test ; si l'on trouve une écriture générique sur un `bg` d'un autre nom que `m_` ; ou si la clé construite `bt.."down"` prend une valeur hors de {`ldown`, `rdown`}. C'est un arrêt, pas un membre improvisé.
- `FAILED:` si un défaut de `bg` devient un état statique *mutable* sans qu'une modification en place ait été trouvée et nommée à la ligne 1a.12, ou si, inversement, une modification en place trouvée est traitée en membre.
- `FAILED:` si `bg:pq` lit `pl` sur l'instance (`self->pl`, ou par le lecteur de champ) au lieu de la table du module, ou si le membre `pl` ne désigne pas la même table.
- `FAILED:` si `bg:ldown` ne passe plus par `pico8_class_lookup_named("mz"|"g_")` puis `pico8_class_instantiate`, ou s'il distingue la forme de ce qu'il construit.
- `FAILED:` si un test de régression de bug (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe que `bg` est convertie ou change de forme. En particulier : si `g_`, `ui`, `jo`, `jc`, `ie`, `js`, `nr`, `pj` ou un provisoire change de forme, ou si un `->defaults` d'une autre classe bouge.
- `FAILED:` si la `pico8_class_t` de `bg` n'est plus déclarée à la même place, avec sa spec, son parent et son nom de couverture.
- `FAILED:` si les valeurs figées de la caractérisation de `bg`, ou celles de `test_pico8_mz`, `test_pico8_qb`, `test_pico8_fq`, `test_pico8_iv`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo`, `test_pico8_mh` ou `test_pico8_bg_kj`, ont été changées après la conversion.

Critères de réussite (en plus) :

- grep : plus aucun `g_bg_class->defaults` ni `pico8_bg_class()->defaults` dans `src/` et `tests/`, plus aucun `pico8_class_set_pq(g_bg_class`, et plus aucun `pico8_instance_get_field` ni `->fields` dans `bg.c` ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN` ;
- la struct `bg` n'a que `pico8_ke_t` et `fa`, `hd`, `pl`, `mq`, `m_` ; son descripteur n'a que `KJ`, `LDOWN`, `RDOWN`, `HQ` ; son écrivain n'accepte que `m_` ;
- hors `bg.c`/`bg.h` et `pico8_struct.h`, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_bg.c` (nouveau) et le `Makefile` (cible et `unit`) bougent ;
- la ligne 1a.12 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.12 ».

## Out of Scope

- `g_` (1a.27) : sa forme, même si `bg:ldown` le construit. `mz` et l'enveloppe `mz.jj` sont déjà convertis.
- `ui` (1a.31), y compris `bt.."held"` → `lheld`/`rheld` sur `ok` et la boucle `gw`, déjà à double forme.
- `jo`, `nr`, `jc`, `fq`, `js`, `ie` : leurs lectures de `qu`, `gv` et `m_` sur un `bg` passent déjà par l'API à double forme.
- La réécriture de `live_ga` dans `geoharvest_burst` (1a.26).
- La lecture de `fg` sur `o_` dans `gw` : quand `o_` n'a pas de `fg` (c'est le cas de `bg`), le port poke 0 là où le cart retomberait sur `max(peek(0x5f80)-1,0)`. L'écart est constaté pendant la mesure, invisible à l'écran (rien ne relit `0x5f80`) et **n'est pas touché ici** : aucun changement de comportement dans ce plan (§8). Il est à signaler en dehors de l'étape.
- Le typage C des membres : phase 1b. Le remplacement de la table `pl` par un tableau fixe : phase D.
- Libérer les `bg` des parties précédentes (TROU-7, Plan 2 §8).
- L'audio : hors périmètre.
- Lancer l'appli raylib : jamais.

## Further Notes

- Sans conversion des écritures `self->fields` de `rdown`/`hq`, le premier glissé droit écrirait dans le descripteur `static const` (segfault ou corruption). `long_session` l'atteint à la frame 4741, donc le filet et `test_pico8_qb` le verraient.
- Sans `m_` dans l'écrivain, l'assert de `pico8_entity_set_field` partirait dès la première frame d'une partie où rien n'est survolé, c'est-à-dire dans presque tous les rejeux.
- Sans `RDOWN` dans l'enum, `mv(bg,"rdown")` retomberait sur le lecteur de champ et rendrait absent : le glissé droit ne ferait plus rien, et le filet (`long_session`) serait rouge.
- Le `bg` d'une partie précédente reste dans `hv` jusqu'au `jl()` suivant, qui reconstruit les listes. Rien ne le libère (TROU-7).
- Mesures faites par grep sur `src/`, `tests/`, `tools/` et `Makefile` à 3f62fe5, en lisant `pico8/lua.lua:219-247`, `:359-383`, `:472-477`, `:1109-1111`, `:1182`, ainsi que `pico8_bg.c`/`.h`, `pico8_cart.c`, `pico8_ui.c` (`lj`, `ev`, `gw`, `bh`, `jz`), `pico8_entity.c` (`lt`, `he`, `mv`, `dc`), `pico8_bootstrap.c` (`dg`), `pico8_ke.c`, `pico8_struct.h`, `pico8_class.c` (délégation, `lookup_named`), `pico8_slipway.c` (`jc:hq`), `pico8_planet_display.c:361`, `pico8_planet_family_trade.c:60`, `pico8_ui_button.c` (`fq`), les fixtures de `FRAMES_EVENTS` (frames au bouton droit) et `tools/class_coverage_net_expected.txt`.
