---
title: "Plan 3, étape 1a.13 : `jc` (le slipway) devient une struct, construite par `lookup_named(\"jc\")`"
description: "Le slipway `jc` est encore une instance de la couche objet, et il est lu et écrit par des consommateurs à simple forme hors de son module (`bv`, `jo:kj`, `bd`, `jc:hq` sur un autre `jc`, le test `slipway_drag`). Il passe en struct fille de `ke`, avec ses méthodes `hq`, `ey`, `eb`, `ev` et `kj` (l'enum gagne `EB`), `nu`/`k_` possédés par le module, et un constructeur qui n'accepte que `qu`. `jo:ldown` et `nr:ldown` continuent de le construire par `lookup_named(\"jc\")`, sans changer."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.13 — `jc` (`slipway.c:392`), instanciée par `lookup_named("jc")` (`planet.c:482,491`, `planet_family_trade.c:104,110`). Tests : aucun.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Défauts mutés sur la classe** : […] deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** […].
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe. […]
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe […].

§6 : la ligne de l'étape ne nomme aucun test de régression, mais la mesure en trouve un qui lit un `jc` : **`slipway_drag`** lit `qu` et `od` sur `ok.lheld` (un `jc`) par `pico8_instance_get_field`, qui assertera sur une struct. Il est réécrit **dans sa façon de lire** (pas dans son intention), avec la preuve rouge du §6 dans ce commit (voir Testing). §7 : le commit passe `make class-coverage-net` avec une sortie identique à `tools/class_coverage_net_expected.txt`. Un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la ligne de l'étape dans le commit, avec « découvert à 1a.13 » dans le message.

### Ce que 1a.1 à 1a.12 ont posé (commits 9ce8bfa, 42fdf50, f5cc6d1, 28a08ac, 824725c, ec01e86, 470accc, 9aa4812, 968e059, 3f62fe5, 3cc50d7), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, et le descripteur `pico8_struct_class_t` : des méthodes indexées par l'enum (aujourd'hui `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`), un lecteur de champ qui rend 1 si le champ n'est pas nil, et un écrivain de champ. Un nom absent de l'enum, ou dont l'entrée est NULL, est cherché comme champ.
- `pico8_ke_t` en premier membre, avec `pico8_ke_struct_init`, `pico8_ke_field` et `pico8_ke_struct_pq`. `pico8_ke_ks` écrit `me`/`t` dans le `pico8_ke_t` d'une struct.
- `pico8_class_set_struct_new` : `pico8_class_instantiate` délègue au constructeur de struct, qui enregistre la couverture. C'est ce qui fait que `jo:ldown` et `nr:ldown` construiront un `jc` struct sans être touchés.
- `pico8_entity_get_field` et `pico8_entity_set_field` sont à double forme. L'écrivain asserte s'il n'y a pas d'écrivain ou si le nom n'a pas de membre. `pico8_instance_get_field` asserte s'il reçoit une struct.
- Sont déjà à double forme et atteints par un `jc` : `lt`/`he`, `dg` (`me`, `t`, `p_`, `mv(me)`), `dc` (`fa`, `ra`), `mv`, `jz`, `ui:bh` (`mv(held,"hq"|"ey")`), le lecteur de `s.qu` dans `jo:gv`/`nr:gv` (1a.10), `jc:hq` lisant `ok.o_.gv` (1a.5), `d_` (lecture `et`, `od`, `dq`, `fp`, `ik` et écriture de `p_` par l'API double, 1a.10), `dl` (lecture `me`, `et`, `od`, et `et.qs`, 1a.10/1a.11), le scan `fx` de `mz:pq` sur `hv` (1a.10), et `bv` pour ce qu'il lit et écrit **sur `self`** (1a.10).
- **Modèles** : une classe avec un `pq` propre (`iv` 1a.2, `mz` 1a.10, `bg` 1a.12 : le constructeur appelle ce `pq`, puis `pico8_ke_struct_pq`). Un constructeur qui accepte des clés mesurées et asserte sur toute autre (`iv` 1a.2 : `ra`, `[1]`, `[2]`). Des tables partagées possédées par le module (`g_mh_nu` 1a.4, `nu`/`kk` de `mz` 1a.10, `pl` de `bg` 1a.12), construites à l'init et libérées au shutdown. Un lecteur par table d'offsets et un écrivain restreint aux noms mesurés (`mz` 1a.10).
- Modèle de test : `test_pico8_mz` et `test_pico8_bg`. La caractérisation est figée sur l'ancien code et lue seulement par l'API à double forme. Les scénarios qui dépendent de `rnd` tournent dans un fils forké juste après `pico8_cart_load()` (`in_fresh_child`). La vérification de forme s'ajoute après la conversion, et l'écriture interdite est vérifiée dans un fils. Les mutations sont notées dans le commit.

### Ce que fait le cart

`pico8/lua.lua` :

```
:612-619  jc=ke:jr([[ nu=o("jc"), me="kz", fa=2, fp=f,ik=48, dw=0.004,
            k_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1), nh=20, ]])
:620      function jc:pq() rn(50) self.ts,self.hg={},{} end
:622-632  function jc:hq(mp) self.re=ok.o_.gv and ok.o_ or {ra=mp,p=mp/128}
            self.od=dj(self.qu,self.re) self.mt=self.qu.mt or self.re.mt
            self.et=self:ce() if self.et then self.et.fo=self end
            self.fa=self.mt and 2 or 4
:633-640  function jc:ey(re) if not re or not d_(self) then … self.p_=true return end
            self.nh,self.re=1,re self.qu:kf(self) re:kf(self) fm:nx(1)
:641-653  function jc:ce() … {qs="-too far-"} … for w in all(self.qu.oe) do if w:eb(self.qu)==self.re … return w
            … la(jp(cg.np, be…), jp(cg.jc, function(sw) return sw.mt==self.mt and cl(…sw.qu.p,sw.re.p) end))[1]
:654-656  function jc:eb(jo) return jo==self.qu and self.re or self.qu end
:657-660  function jc:ch(f,t) if rnd()<0.08 and self.hg[t.id] then add(self.ts,{…}) end
:661-666  function jc:lg(…) self.qu.ra … self.re.ra … self.t
:667-681  function jc:kj() … self.k_[self.nh] … self.nh+=1 … (self.et or self.fo) … self.mt … self.ts … self.fo=nil
:682-684  function jc:ev() if (self.qu~=self.re) dl(self) end
:414-416  jo:ldown : elseif self.me=="ly" then return jc({qu=self})
:1112-1113 nr:ldown : return self:gv({}) and jc({qu=self})
:465-466  jo:kf(sw) add(self.oe,sw) if (sw:eb(self).fx) self.fx=true
:503-512  jo:cn : for w in all(self.oe) … w:eb(self):cn(lw)
:1049-1056 mx:kf(sw) local cp=sw:eb(self) …        :1117-1119 nr:cn : … w:eb(self):cn(gf)
:606-608  trade:kg(np,jc) rl=la(self.rl,{jc})       :1265-1274 bd : w.hg[e.lp[i+1].id]=true
:548-549  jo:kj : if self.fo then … circfill(…, sqrt(self.fo.dw)*128, 2) … self.fo=nil
:552-558  bv : … jp(cg.jc, function(sw) return be(sw.qu.p,sw.re.p,self.p,0.0032) end)[1] … if (self.et) self.et.fo=self
:1262-1264 bk : sqrt(#(cg.jc or {}))
```

Points décisifs :

- **Aucun défaut de `jc` n'est muté.** `nu` et `k_` sont seulement lus (`lt`/`he`, `jc:kj`). `fp`, `ik`, `dw` ne sont jamais réaffectés. `fa` et `nh` sont réécrits, mais **sur l'instance** (`jc:hq`, `jc:ey`, `jc:kj`) : ce sont des membres, pas un état de classe. `ts` et `hg` sont modifiés en place (`jc:ch`, `jc:kj`, `bd`), mais ce sont des tables **propres à chaque instance** (créées par `jc:pq`), pas des défauts : la règle des défauts modifiés en place ne s'applique pas. L'étape n'introduit donc aucun état statique *mutable*.
- **`k_` est lu sur l'instance** (`self.k_[self.nh]`), contrairement à `jj` de `mz` ou `pl` de `bg`. `jc:kj` lit donc le membre, qui pointe sur la table du module.
- **`fp=f` vaut `false`**, pas nil : le lecteur de champ le rend présent (seul le REF NULL est absent). `d_` le lit comme faux et retombe sur 0, comme aujourd'hui.
- **Construction avec une table** : `jc({qu=self})`. La seule clé reçue est `qu`, depuis `jo:ldown` (planète `ly`) et `nr:ldown`. Le résultat part dans `ok.lheld` par `jz(bt.."down")`.
- **`eb` est appelée par nom de chaîne** (`mv(w,"eb",…)` dans `jo:kf`, `jo:cn`, `nr:cn`, `mx:kf` et `jc:ce`) sur les entrées des listes `oe` des planètes, qui ne contiennent que des `jc`. Elle entre donc dans l'enum. `ce`, `ch`, `lg` et `pq` ne sont appelées que directement et n'y entrent pas.
- **Un `jc` est posé en `fo` d'une planète** par `jc:hq` quand `ce()` rend une planète de `cg.np` sur le trajet. `jo:kj` lit alors **`fo.dw` sur le `jc`** (0.004 : le cercle hachuré de rayon ≈8 px autour de la planète gênante). Un `jc` reçoit lui-même un `fo` de deux écrivains : `bv` (une sonde `g_` instance ou un `mz` struct au-dessus de la voie) et `jc:hq` d'un **autre** `jc` dont `ce()` rend cette voie (croisement ou voie déjà existante).
- **`bd` écrit dans `w.hg`** pour chaque `jc` d'une route commerciale (`e.rl`), en lisant `hg` sur le `jc` puis en écrivant dans sa table.
- **`jc` a `nu=o("jc")`** : il est dans `hv`, `cv.kj`, `cv.ev` et `cg.jc`, et dans aucun de `cv.hd`/`hk`/`hb`. Il n'est donc jamais survolé (`gw`) ni cible de `jz`.

### Emprise réelle de `jc` (mesurée sur 3cc50d7)

| Aspect | Mesure |
|---|---|
| Constructions | Deux sites en jeu, tous deux par `pico8_class_lookup_named("jc")` + `pico8_class_instantiate(cls, {qu=self})` : `pico8_jo_ldown` (`planet.c:483-492`) et `pico8_nr_ldown` (`planet_family_trade.c:104-110`). Aucun `fz`, aucun `gx[]`, aucun `kv[]`. Déclarée par `pico8_jc_init` (`cart.c:82`), avec l'assert `pico8_jc_class()->parent == pico8_ke_class()` (`cart.c:99`, reste vrai). `pico8_jc_new` n'a aucun appelant, ni dans `src/` ni dans `tests/`. Aucun test ne construit de `jc` directement. |
| Défauts | `nu`=table `{"jc"}`, `me`=`"kz"` (surcharge celui de `ke`), `fa`=2, `fp`=`false`, `ik`=48, `dw`=0.004, `k_`=table de 20 entiers `0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1`, `nh`=20. Hérité de `ke` : `t`=0. Aucun défaut muté. |
| Champs écrits sur l'instance | `qu` (table de construction) ; `ts`, `hg` (`jc:pq`) ; `re` (`jc:hq`, `jc:ey`) ; `od`, `mt`, `et` (`jc:hq`) ; `fa` (`jc:hq`) ; `nh` (`jc:ey`, `jc:kj`) ; `p_` (`jc:ey`, et `d_` par `pico8_entity_set_field`) ; `fo` (`jc:kj` le remet à nil ; `bv` et `jc:hq` d'un autre `jc` le posent) ; `me`/`t` (`pico8_ke_ks` depuis `d_`, `dg`). Aucun autre nom n'est écrit sur un `jc` dans le cart ou dans `src/`. |
| Méthodes | `pq` (`pico8_class_set_pq`, `slipway.c:398`), plus `hq`, `ey`, `eb`, `ev` et `kj`, **installées aujourd'hui par `->defaults`** (`slipway.c:399-403`). Dans l'enum, `HQ`, `EY`, `EV` et `KJ` existent déjà : **l'enum gagne seulement `EB`**. `ce`, `ch`, `lg` sont appelées directement. `me`=`"kz"` fait appeler `mv(jc,"kz")` par `dg` : `KZ` est dans l'enum (pour `mz`), mais son entrée est NULL sur `jc`, le lecteur rend absent comme aujourd'hui. Idem pour `"ph"` après `ks`. |
| `->defaults` | 5 sites, tous dans `slipway.c` (`:399-403`), tous des installations de méthode. Aucun `g_jc_class->defaults` ni `pico8_jc_class()->defaults` ailleurs dans `src/` ou `tests/`. |
| Lecteurs à double forme qui atteignent un `jc` | `lt`/`he` (`kj`, `ev`, `hd`, `hk`, `hb`, `nu`) ; `dg` (`me`, `t`, `p_`, `mv(me)`) ; `dc("kj"|"ev")` (`fa`, `ra` absent) ; `mv(jc,"hq"|"ey")` (`ui:bh`) ; `mv(jc,"eb")` (5 sites ci-dessus) ; `jo:gv`/`nr:gv` (`s.qu`, au relâcher par `jz("gv", held)`) ; `d_` (`et`, `od`, `dq` absent, `fp`, `ik`) ; `dl` (`me`, `et`, `od`) et `dl` lisant `et.qs` quand `et` est un `jc` (absent) ; `mz:pq` (`fx` absent) ; `test_pico8_mz` (`get(jc,"qu"|"re")`, `fo` d'un `et` qui est un `jc`, `pico8_cg("jc")`). `pico8_bk` ne fait que compter `cg.jc`. `trade:kg`, `gu(t.rl,w)` et l'ajout dans `oe` ne comparent que l'identité. |
| **Lecteurs à simple forme qui atteignent un `jc`** (assertent après conversion) | (1) `bv` (`probe.c:95-96`) : `sw.qu`/`sw.re` par `pico8_probe_field_instance` → `pico8_instance_get_field`. (2) `jo:kj` (`planet_display.c:569`) : `fo.dw` par `pico8_instance_get_field`, `fo` étant un `jc`. (3) `bd` (`economy_tiers.c:154`) : `w.hg` par `pico8_instance_get_field`, `w` venant d'un `rl`. (4) Tout `slipway.c`/`slipway_display.c` sur `self` et sur `sw` de `cg.jc` (`jc_ce_jc_predicate`), par `pico8_instance_get_field`. (5) `test_pico8_slipway_drag.c:71-85` : `field_instance(held,"qu")`, `field_number(held,"od")`. |
| **Écrivains à simple forme qui atteignent un `jc`** | (1) `bv` (`probe.c:181-186`) : `et.fo=self` par `et_inst->fields`, après un assert `INSTANCE` ; `et` est un `jc` quand la sonde est sur une voie. (2) `jc:hq` (`slipway.c:130-136`) : `et.fo=self`, même assert, `et` étant un `jc` (croisement, voie déjà existante). (3) Toutes les écritures `self->fields` de `slipway.c`/`slipway_display.c` : hors bornes sur une struct. |
| Casts et asserts de tag | `jc:ce` (`slipway.c:268-269`) asserte `INSTANCE` sur les entrées de `qu.oe`, qui seront des `jc` struct. `jo_kf_dispatch`/`mx_kf_dispatch` et `ui.c:357` (TROU-6) castent le `jc` en `pico8_instance_t *` sans le déréférencer ; `mv` lit ensuite le tag. Aucune macro de `pico8_ref_audit.h` ne reçoit un `jc` (`PICO8_REF_AS_LIST*` reçoit `oe`, `ts`). |
| Clés dynamiques | Aucune sur `jc`. `bt.."down"`/`bt.."held"` sont sur `ok` et rendent un `jc` comme valeur, pas comme clé. |
| `->cls` lu sur une entrée de `hv` | `test_pico8_geoharvest_burst.c:85` (`live_ga`, sans vérifier le tag). Si un `jc` précède le `ga` dans `hv`, la lecture reste dans les bornes (la struct `jc` est bien plus grande que l'en-tête d'instance), n'est jamais égale à la classe `ga` et n'est jamais déréférencée. Même situation que `bg`/`mz`, réécriture prévue en 1a.26. `planet_gh_row_lq.c:121` lit `->cls` sur un `lq`, jamais sur un `jc`. |
| Tests qui atteignent un `jc` | `slipway_drag` (lit `qu`/`od`, **à réécrire**). `playability` (construit un slipway, ne lit rien du `jc`). `test_pico8_mz` (lit par l'API double, diff vide). `rich_planet_exports` appelle `pico8_bd` sans slipway. |
| Filet | 6 des 15 rejeux construisent un `jc` (`jc  152  6/15`) : le glisser, le relâcher, `jc:kj` (couleurs, particules `ts`), le cercle `fo.dw` sur la planète gênante et `-too far-` sont vus par le filet. |
| Couverture | `jc  152  6/15` (inchangé attendu). |

## Solution

1. **Struct `jc`** (publique dans `pico8_slipway.h`, comme `pico8_mz_t`/`pico8_bg_t`) : premier membre `pico8_ke_t`, puis les défauts de la chaîne `nu`, `fa`, `fp`, `ik`, `dw`, `k_`, `nh`, puis les champs écrits sur instance `qu`, `ts`, `hg`, `re`, `od`, `mt`, `et`, `fo`, `p_`. Tous sont des `pico8_value_t` et gardent leur nom minifié. `me` et `t` sont ceux de `pico8_ke_t`.
2. **Tables possédées par le module** : `nu` (`{"jc"}`) et `k_` (20 entrées), construites par `pico8_jc_init` à partir du même fragment de spec, et libérées par `pico8_jc_shutdown`. Le membre de chaque struct est une REF vers la table du module. `jc:kj` lit `k_` **par le membre** (le cart le lit sur `self`).
3. **Valeurs initiales** : après `pico8_ke_struct_init`, `me`=`"kz"`, `t`=0 ; `fa`=2, `fp`=`false`, `ik`=48, `dw`= la valeur exacte que `ob()` donne à `0.004`, `nh`=20, `nu`/`k_` = REF vers les tables du module ; tous les champs d'instance à nil.
4. **Constructeur** (posé comme `struct_new` de la `pico8_class_t` de `jc`), dans l'ordre :
   1. alloue ;
   2. appelle `pico8_ke_struct_init` avec le descripteur de `jc` ;
   3. pose les valeurs initiales ;
   4. copie la clé nommée `qu` de la table reçue dans le membre `qu`, et **asserte** (message porteur, arrêt §7) sur toute autre clé ;
   5. enregistre `"jc"` au crochet de couverture ;
   6. appelle `jc:pq` (`rn(50)`, `ts` = liste neuve, `hg` = table neuve) ;
   7. **puis** `pico8_ke_struct_pq`.

   C'est l'ordre du parcours de classe. `pico8_class_set_pq(g_jc_class, …)` disparaît.
5. **Enum de méthodes** : il gagne `EB` (`"eb"`), et rien d'autre. Sur tout autre descripteur l'entrée est NULL, et le nom retombe sur le lecteur de champ, qui le rend absent comme aujourd'hui. `eb` n'est pas un nom `bm`, donc `lt`/`he` ne changent pas.
6. **Descripteur de `jc`** : méthodes `HQ`, `EY`, `EB`, `EV`, `KJ`. Son lecteur de champ connaît les 16 membres propres, puis délègue à `pico8_ke_field`. Son écrivain n'accepte que `fo` (écrit par `bv` et par `jc:hq` d'un autre `jc`) et `p_` (écrit par `d_`), et rend 0 pour tout autre nom : l'assert de `pico8_entity_set_field` part alors. Un accesseur public du descripteur s'ajoute, comme `pico8_mz_struct_class()`.
7. **Méthodes sur la struct** : `pico8_jc_pq`, `_hq`, `_ey`, `_ce`, `_eb`, `_ev` (`slipway.c`) et `pico8_jc_ch`, `_lg`, `_kj` (`slipway_display.c`) prennent la struct `jc`. Elles gardent leurs gardes NULL, leurs adaptateurs, leurs asserts porteurs et les TROU existants.
   - Ce qu'elles lisent et écrivent **sur `self`** passe par les membres.
   - Ce qu'elles lisent **sur une autre entité** (`qu`, `re`, `ok.o_`, les planètes `n` de `cg.np`, les `sw` de `cg.jc`, les `w` de `qu.oe`, `f`/`t` de `ch`) passe par `pico8_entity_get_field`, parce que ces entités sont de l'une ou l'autre forme selon l'étape.
   - `jc:hq` et `jc:ce` construisent **à l'identique** les deux instances sans classe `{ra,p}` et `{qs="-too far-"}` (1a.40, 1a.39).
   - `jc:hq` pose `et.fo=self` par `pico8_entity_set_field`. L'assert sur `et` devient « instance **ou** struct », avec un message qui le dit.
   - `jc:ce` asserte que chaque entrée de `qu.oe` est une struct de `jc`, au lieu d'une instance, et appelle toujours `mv(w,"eb",…)`.
   - `jc:ey` appelle toujours `d_(self)`, `mv(qu,"kf",self)`, `mv(re,"kf",self)` et `fm:nx(1)`.
   - `jc:kj` lit `k_[nh]` par le membre, incrémente `nh` jusqu'à 20, et remet `fo` à nil même quand rien n'est dessiné.
   - Les helpers de lecture que la conversion rend inutiles disparaissent (`pico8_jc_field_value`/`_instance`/`_vec` et leurs pendants `display` pour `self`). Un helper qui lit une autre entité reste, mais par l'API double.
8. **Sites hors du module, découverts par la mesure** (chacun n'est qu'un changement d'appel, pas de logique) :
   - `bv` (`probe.c`) : `pico8_probe_field_instance` lit par `pico8_entity_get_field`. L'écriture de `fo` sur `et` passe à `pico8_entity_set_field`, et l'assert devient « instance ou struct ».
   - `jo:kj` (`planet_display.c`) : `fo.dw` lu par `pico8_entity_get_field`. La lecture et l'écriture de `fo` sur la planète elle-même ne changent pas.
   - `bd` (`economy_tiers.c`) : `w.hg` lu par `pico8_entity_get_field`. L'écriture dans la table `hg` et la tolérance d'un `hg` absent (TROU-15) ne changent pas.
9. **`pico8_jc_init` / `_shutdown`** : même déclaration, même spec, même parent, même nom de couverture, même enregistrement nommé `"jc"`. La classe reçoit le constructeur et perd les 5 installations `->defaults`. Le shutdown libère la classe, puis les tables `nu` et `k_` du module. Il ne libère jamais un `jc`.
10. **`pico8_jc_new`** rend la struct (comme `pico8_mz_new`), en gardant le même paramètre.
11. **Sans changement** : `jo:ldown` et `nr:ldown` (la délégation `struct_new` construit la struct), `ui.c`, `jo:gv`/`nr:gv`, `d_`, `dl`, `jo:kf`/`jo:cn`/`nr:cn`/`mx:kf`, `trade`, `pico8_bk`, `pico8_dj`, `dg`, `lt`/`he`/`dc`/`mv`.
12. **Test de régression `slipway_drag`** : ses deux lectures sur `held` (`qu`, `od`) passent par `pico8_entity_get_field`. Ses assertions, ses frames et son fixture ne changent pas. La preuve rouge du §6 est faite dans le commit.
13. **Plan** : ajouter à la ligne 1a.13 de `docs/plan-3.md` le texte suivant.

    > Tests : `slipway_drag` (lecture `qu`/`od` sur `ok.lheld`). Découvert à 1a.13 : `eb` entre dans l'enum (`mv(w,"eb")` dans `jo:kf`, `jo:cn`, `nr:cn`, `mx:kf`, `jc:ce`) ; lecteurs à simple forme atteints par un `jc` : `sw.qu`/`sw.re` dans `bv` (`probe.c:95-96`), `fo.dw` dans `jo:kj` (`planet_display.c:569`), `w.hg` dans `bd` (`economy_tiers.c:154`) ; `et.fo` écrit sur une cible `jc` par `bv` (`probe.c:185`) et `jc:hq` (`slipway.c:135`) → membre `fo`, écrivain ; `p_` écrit par `d_` → écrivain.

    Aucun défaut muté ne s'ajoute aux défauts connus du §5.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `jc` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise de `jc` soit mesurée dans le PRD (constructions, défauts, champs écrits, méthodes, `->defaults`, lecteurs et écrivains génériques, tests, filet), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux que la struct `jc` n'ait que `pico8_ke_t` et les 16 membres `nu`, `fa`, `fp`, `ik`, `dw`, `k_`, `nh`, `qu`, `ts`, `hg`, `re`, `od`, `mt`, `et`, `fo`, `p_`, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun défaut de `jc` ne devienne un état statique *mutable*, afin de n'appliquer la règle des défauts mutés que là où le cart mute (ici, nulle part).
5. En tant que mainteneur, je veux que `nu` et `k_` soient des tables possédées par le module, construites à l'init et libérées au shutdown, afin qu'aucun accès `->defaults` ne subsiste pour `jc`.
6. En tant que mainteneur, je veux que `ts` et `hg` restent des tables propres à chaque `jc`, créées par `jc:pq`, afin que la modification en place par `ch`, `kj` et `bd` ne touche jamais un autre slipway.
7. En tant que gardien du filet, je veux que `jo:ldown` sur une planète `ly` et `nr:ldown` construisent un `jc` struct par `lookup_named("jc")` sans que leur code change, afin que la délégation `struct_new` soit le seul chemin.
8. En tant que gardien du filet, je veux qu'un `jc` neuf lise `me`=`"kz"`, `t`=0, `fa`=2, `fp`=`false`, `ik`=48, `dw`=0.004, `nh`=20, `k_` dans l'ordre du cart, `qu` = la planète d'origine, `ts` vide et `hg` vide, avec `re`, `od`, `mt`, `et`, `fo` et `p_` absents, afin que son état de départ soit identique.
9. En tant que gardien du filet, je veux que `jc:pq` tire `rn(50)` avant l'enregistrement, afin que l'ordre des effets du parcours de classe soit gardé.
10. En tant que gardien du filet, je veux qu'un `jc` neuf soit enregistré dans `hv`, `cv.kj`, `cv.ev` et `cg.jc`, et dans aucun `cv.hd`/`hk`/`hb`, afin que l'ordre de dessin et de mise à jour ne bouge pas.
11. En tant que gardien du filet, je veux qu'une table de construction portant une autre clé que `qu` déclenche une assertion porteuse de message, afin qu'un champ jamais mesuré ne soit pas perdu en silence.
12. En tant que gardien du filet, je veux que `mv(jc,"hq",…)` pendant un glisser recalcule `re`, `od`, `mt`, `et` et `fa` comme avant, afin que le prix affiché et la couleur de la voie soient identiques.
13. En tant que gardien du filet, je veux que `jc:hq` pose `fo` sur la cible de `ce()`, qu'elle soit une planète instance, un autre `jc` struct ou le `{qs}` sans classe, afin que la gêne s'affiche comme avant.
14. En tant que gardien du filet, je veux que `jo:kj` lise `dw` sur un `jc` posé en `fo` de la planète, afin que le cercle hachuré de la planète gênante garde son rayon.
15. En tant que gardien du filet, je veux que `jc:ce` rende `{qs="-too far-"}` au-delà de la portée, la voie existante de `qu.oe` qui relie déjà les deux mêmes planètes, ou le premier obstacle de `cg.np` puis de `cg.jc`, dans cet ordre, afin que la validation du tracé soit identique.
16. En tant que gardien du filet, je veux que `mv(w,"eb",p)` sur un `jc` struct rende l'autre extrémité, avec la retombée sur `qu` quand `re` est faux, afin que `jo:kf`, `jo:cn`, `nr:cn`, `mx:kf` et `jc:ce` trouvent le même voisin.
17. En tant que gardien du filet, je veux que `jc:ey` sans cible, ou refusé par `d_`, lève seulement `p_`, afin qu'un glisser abandonné disparaisse au `dg` suivant.
18. En tant que gardien du filet, je veux que `jc:ey` réussi pose `nh`=1 et `re`, inscrive le `jc` dans le `oe` des deux planètes et fasse payer par `d_`, afin que la construction coûte et relie comme avant.
19. En tant que gardien du filet, je veux que `d_` puisse écrire `p_` et appeler `ks("ph")` sur un `jc` struct, afin que ni l'abandon ni la réussite ne fasse partir un assert.
20. En tant que gardien du filet, je veux que `jc:kj` dessine les deux passes de `lg`, anime les particules de `ts`, tire `ch` dans les deux sens et remette `fo` à nil, afin que le rendu soit identique au pixel près.
21. En tant que gardien du filet, je veux que `jc:kj` lise `k_[nh]` sur le membre et fasse remonter `nh` jusqu'à 20, afin que l'animation d'apparition garde ses couleurs.
22. En tant que gardien du filet, je veux que `jc:ev` appelle `dl` tant que `qu ≠ re`, afin que le prix et `-too far-` restent affichés près du curseur.
23. En tant que gardien du filet, je veux que `bv` trouve un `jc` struct sous une sonde par `sw.qu`/`sw.re` et lui pose `fo`, afin que la voie survolée par une sonde ou un trou de ver s'allume comme avant.
24. En tant que gardien du filet, je veux que `bd` lise `hg` sur un `jc` struct et y écrive les planètes desservies, afin que les particules de commerce apparaissent sur les mêmes voies.
25. En tant que gardien du filet, je veux que `dg` appelle `mv(jc,"kz")` puis `mv(jc,"ph")` sans rien trouver et incrémente `t` dans le `pico8_ke_t`, afin que l'horloge de `lg` avance comme avant.
26. En tant que mainteneur, je veux que l'écrivain de `jc` n'accepte que `fo` et `p_`, afin qu'une écriture générique d'un autre nom déclenche l'arrêt du §7 au lieu d'un membre improvisé.
27. En tant que gardien de la couverture, je veux que le constructeur enregistre `"jc"` une fois, jamais `"ke"`, afin que `jc  152  6/15` et `ke  0  0/15` restent identiques.
28. En tant que mainteneur, je veux que la `pico8_class_t` de `jc` reste déclarée à la même place, avec sa spec, son parent, son nom de couverture et son enregistrement nommé, afin que `KNOWN[]`, `lookup_named("jc")` et l'assert de `cart.c` tiennent.
29. En tant que mainteneur, je veux que les 5 accès `->defaults` de `slipway.c` disparaissent, afin d'appliquer la règle des accès directs aux défauts.
30. En tant qu'auteur des étapes suivantes, je veux que l'enum ne gagne que `EB`, afin que chaque ajout reste attaché à une étape.
31. En tant qu'auteur de 1a.39 et 1a.40, je veux que `{qs="-too far-"}` et `{ra,p}` restent des instances sans classe construites à l'identique, afin que leur conversion soit leur propre étape.
32. En tant qu'auteur de 1a.17 (`jo`) et 1a.24 (`nr`), je veux que `jo:ldown`/`nr:ldown` ignorent la forme de `jc`, afin que leur conversion ne touche pas `slipway.c`.
33. En tant que gardien des tests de régression, je veux que `slipway_drag` ne change que sa façon de lire `held`, et qu'il redevienne rouge si l'on remet le bug d'origine, afin que la conversion ne l'affaiblisse pas (§6).
34. En tant que gardien des tests, je veux que tous les autres tests de régression et `test_pico8_mz`, `test_pico8_bg`, `test_pico8_fq`, `test_pico8_qb` passent avec un diff vide, afin que la conversion ne se valide pas en réécrivant des tests existants.
35. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
36. En tant que lecteur du plan, je veux que les sites découverts et le test réécrit soient inscrits à la ligne 1a.13, afin que la liste des étapes dise ce que l'étape a réellement touché.
37. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés, afin de ne laisser aucun contrat mensonger.
38. En tant que vérificateur, je veux une caractérisation de `jc` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`hg`, `p_`, `mt`, forme de `et`, `fo` hors écran).

## Implementation Decisions

Aucun outil de création d'agent n'était disponible dans cette session pour lancer un agent `guideline` frais. J'ai donc chargé le skill `guideline` et je l'ai appliqué moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Pas d'état statique mutable ; deux tables de classe en lecture seule** (`nu`, `k_`). La preuve est au Problem Statement. Si l'implémenteur trouve une modification en place de `nu` ou `k_`, c'est la règle des défauts modifiés en place qui s'applique : site découvert, état statique partagé, ligne 1a.13. Ce n'est pas un membre.
- **`ts` et `hg` sont des membres**, pas un état de classe : `jc:pq` les crée par instance. Leur modification en place par `ch`, `kj` et `bd` porte sur la table de l'instance.
- **(G) `k_` lu par le membre**, parce que le cart le lit sur `self`. Rejeté : lire la table du module dans `kj`. Ce serait équivalent aujourd'hui, mais cela inventerait une lecture « sur la classe » que le cart ne fait pas.
- **(G) `fp` vaut `false`, pas nil.** C'est ce que `ob()` produit pour `f`, et le lecteur le rend présent. Rejeté : laisser le membre à nil. `d_` rendrait le même résultat, mais `mv(jc,"fp")` ne rendrait plus rien, ce qui change un champ observable.
- **Constructeur : seule la clé `qu` est acceptée.** Même modèle que `iv` (clés mesurées, assert sur les autres). Les deux appelants sont les deux `ldown` mesurés.
- **Enum** : seul `EB` s'ajoute, parce que `eb` est appelée par `mv(…,"eb")`. `ce`, `ch`, `lg` et `pq` n'y entrent pas.
- **Écrivain** : `fo` et `p_`, les deux seuls noms qu'un écrivain générique pose sur un `jc` (mesuré). Les autres champs sont écrits par les méthodes propres de `jc`, directement dans les membres. (G) Rejeté : accepter aussi `re`/`nh`/`fa`. Aucun écrivain générique ne les écrit, donc une telle écriture serait un site non mesuré, donc un arrêt.
- **(G) Lectures sur les autres entités par l'API double.** Dans `slipway.c`/`slipway_display.c`, tout ce qui n'est pas `self` passe par `pico8_entity_get_field`, y compris `sw` de `cg.jc`. C'est plus simple et plus sûr qu'un cast vers la struct `jc` d'une entrée de liste. Et cela n'aura pas à bouger quand `jo`/`nr` seront convertis.
- **Sites hors du module** : `bv`, `jo:kj` (`fo.dw`), `bd` (`w.hg`). Ce sont les seuls lecteurs et écrivains à simple forme qu'un `jc` atteint (mesuré par grep sur `"qu"`, `"re"`, `"fo"`, `"et"`, `"hg"`, `"eb"`, `"oe"`, `"rl"` et `cg("jc")`). Chacun ne change que d'appel, vers l'API double. Tout autre lecteur trouvé en cours d'étape est un site découvert (règle « Coexistence ») : il est ajouté à la ligne 1a.13.
- **Asserts de tag** : l'assert `INSTANCE` de `bv` et de `jc:hq` sur `et` devient « instance ou struct ». Celui de `jc:ce` sur `qu.oe` devient « struct de `jc` » (seuls des `jc` entrent dans `oe`, par `jo:kf`). Les messages sont réécrits pour dire la nouvelle règle.
- **`pq` dans le constructeur** : modèle `iv`/`mz`/`bg`, avec `jc:pq` puis `pico8_ke_struct_pq`.
- **`jo:ldown`/`nr:ldown` ne sont pas modifiés.** Le résultat de `pico8_class_instantiate` part en REF dans `ok.lheld`, quelle que soit sa forme.
- **Instances sans classe de `jc:hq`/`jc:ce`** : construites à l'identique, hors de l'étape (1a.39, 1a.40).
- **`slipway_drag`** : seule la façon de lire change (`pico8_instance_get_field` → `pico8_entity_get_field` dans `field_instance`/`field_number`). Ses lectures sur `ok`, `fm` et les planètes restent sur instance.
- **`live_ga` de `geoharvest_burst`** (G) : laissé tel quel, comme en 1a.10 et 1a.12. La réécriture est prévue en 1a.26.
- **`test_pico8_entity_smoke`** n'est pas touché (il ne construit aucun `jc`).
- **Commentaires** (CLAUDE.md) :
  - À ajouter : le contrat de la struct (jamais libérée, TROU-7 ; `nu`/`k_` partagées, jamais modifiées en place ; `ts`/`hg` propres à l'instance), le contrat de `pico8_jc_new` (rend une struct, n'accepte que `qu`), l'assert porteuse de la clé reçue, et le contrat de l'écrivain (`fo`, `p_`).
  - À corriger, parce que le changement les rend faux : `slipway.h` « NULL instance argument… absent or wrong-kind field » (les méthodes prennent la struct), le commentaire de `pico8_jc_hq` qui dit que `self.re` « is not always a class instance » (reste vrai, mais la cible `fo` peut maintenant être une struct), celui de `bv` sur `et` « an instance, real target or class-less throwaway », et celui de `slipway_display.h` s'il nomme `pico8_instance_t`.
  - Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation de `jc` sur l'ancien code.** Nouveau test (voir Testing), valeurs figées **avant** toute modification de `slipway*.c/h`, `probe.c`, `planet_display.c`, `economy_tiers.c` et `pico8_struct.h`.
2. **Coexistence hors du module.** `bv`, `jo:kj` (`fo.dw`) et `bd` (`w.hg`) passent à l'API double ; les asserts de `bv` et de `jc:hq` sur `et` acceptent les deux formes ; `slipway_drag` lit `held` par l'API double, avec la preuve rouge du §6. Tout reste vert, et `jc` est encore une instance.
3. **`jc` converti.** Enum `EB`, struct, tables `nu`/`k_` du module, descripteur, constructeur, méthodes sur la struct, classe reliée, `->defaults` retirés, accesseur, ligne 1a.13 du plan. La caractérisation passe **sans changer ses valeurs figées**, et la vérification de forme s'ajoute.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field` et `pico8_mv`, l'appartenance à `hv`/`cv`/`cg`, les listes `oe` des planètes, les crédits de `fm`, `ok.lheld`, les pixels du framebuffer et le compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_jc`** (dans `make unit`, lié comme `test_pico8_mz`/`test_pico8_bg`, sur le cart réel chargé). Il lit **uniquement** par l'API à double forme (`pico8_entity_get_field`, `pico8_entity_set_field`, `pico8_mv`, `pico8_ui_jz`, `pico8_hv`/`pico8_cv`/`pico8_cg`, `pico8_dg`, `pico8_dc`, `pico8_bd`, `pico8_class_instantiate(pico8_class_lookup_named("jc"), {qu=…})`), pour que le code soit le même avant et après. Chaque scénario qui dépend de `rnd` tourne dans un fils forké (`in_fresh_child`).
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction**, par `mv(planète "ly","ldown")` et par `lookup_named("jc")` + `{qu=planète}` :
       - lire `me`, `t`, `fa`, `fp` (genre et valeur), `ik`, `dw` (bits exacts), `nh`, les 20 entrées de `k_`, `nu`, et `qu` identique à la planète ;
       - vérifier que `ts` est une liste vide et `hg` une table vide ;
       - vérifier que `re`, `od`, `mt`, `et`, `fo`, `p_`, `ra`, `hd`, `ql`, `fx` et `dq` sont absents ;
       - vérifier que `hq`, `ey`, `eb`, `ev` et `kj` sont des fonctions, et que `kz`, `ph`, `ldown`, `gv`, `md` sont absents ;
       - vérifier l'appartenance à `hv`, `cv.kj`, `cv.ev`, `cg.jc`, et à aucun de `cv.hd`/`hk`/`hb` ;
       - vérifier que deux `jc` rendent le même `k_` et le même `nu`, mais des `ts`/`hg` distincts.
    2. **Rejeu de `slipway_drag_events.json`** sur toutes ses frames, en suivant `ok.lheld` et chaque `jc` de `cg.jc`. Par frame : `od`, `mt`, `fa`, `nh`, `p_`, `me`, `t`, le genre de `re` (absent, planète, `{ra,p}` sans classe) et celui de `et` (absent, planète, `jc`, `{qs}`), l'identité de `fo`, le nombre de `ts`, les clés de `hg`, puis les `#oe` des planètes et `fm.nz`. Le tout replié en empreinte de séquence. Figé.
    3. **Rejeu de `trading_hub_session_events.json`** (plusieurs voies, des `nr`, un `bd`, du commerce) : même empreinte par frame, plus pour chaque planète l'identité de `fo` et le genre de son porteur (`jc` ou sonde). Figé.
    4. **Rejeu de `phase4_playthrough_events.json`** : même empreinte, plus le résultat de `mv(w,"eb",p)` pour chaque `w` de chaque `oe`. Figé.
    5. **Scénarios dirigés**, sur un monde issu du rejeu 2 dans un fils :
       - `ey(nil)` → `p_` vrai ;
       - `ey(planète)` réussi → `nh`=1, `re`, les `oe`, le coût ;
       - `hq` au-delà de la portée → `et` = `{qs="-too far-"}` avec `fo` = le `jc` ;
       - `hq` sur la même paire qu'une voie existante → `et` = cette voie, et son `fo` est le `jc` ;
       - `eb` avec `re` faux → `qu` ;
       - `dc("kj")` sur un framebuffer effacé, avec seulement ce `jc` puis avec une planète dont `fo` est un `jc` (le cercle `dw`) → empreintes du framebuffer.

       Figés.
    6. **`bv` sur une voie** : une sonde `g_` construite par `lookup_named("g_")`, `mv(g_,"hq")` au milieu d'une voie → `et` est le `jc`, et le `fo` du `jc` est la sonde. Figé.
  - **Forme** (ajoutée au morceau 3) :
    - chaque `jc` de `cg.jc` et celui de `lookup_named` sont tagués `STRUCT` avec le descripteur de `jc` (accesseur) ;
    - `pico8_mv(jc,"hq"|"ey"|"eb"|"ev"|"kj",…)` trouve une méthode, et `mv(jc,"kz"|"ph"|"ldown"|"rdown"|"gv"|"md")` ne trouve rien ;
    - `pico8_entity_set_field(jc,"fo"|"p_",…)` passe ;
    - `pico8_entity_set_field(jc,"re"|"nh"|"fa",…)` asserte (vérifié dans un fils, `SIGABRT`) ;
    - une construction avec `{qu=…, ra=…}` asserte (dans un fils) ;
    - `pico8_mv(bg|fq|mz,"eb")` reste absent.
  - **Mutations** : l'implémenteur remet localement au moins 12 erreurs plausibles et note dans le commit celles qui sont attrapées :
    - `EB`, `HQ`, `EY`, `EV` ou `KJ` absent de la table ;
    - `EB` pointé sur `ey` ;
    - `eb` sans la retombée sur `qu` ;
    - `k_` dans le désordre, ou une entrée fausse ;
    - `nh` initial ≠ 20 ;
    - `dw` ≠ 0.004 ;
    - `fp` à nil ou à 0 au lieu de `false` ;
    - `me` initial `"ph"` ;
    - écrivain qui refuse `fo` ou `p_` ;
    - `ts`/`hg` partagés entre instances ;
    - `jc:pq` omis ou `pico8_ke_struct_pq` omis ;
    - `qu` de la table de construction ignoré ;
    - `kj` qui ne remet plus `fo` à nil ;
    - `bv`, `jo:kj` ou `bd` qui retombe sur une lecture à simple forme (assert).

    Chaque mutant équivalent éventuel est justifié dans le commit. Par exemple : le membre `nu` à une autre table au contenu identique.
- **Test de régression réécrit, `slipway_drag`** (§6) : seules `field_instance` et `field_number` changent d'API. Preuve rouge dans ce commit, sur le code converti :
  - remettre localement le bug d'origine (2d7021f : `ui:bh` passe nil à `ey` au lieu de l'entité trouvée par `jz("gv")`) ;
  - constater le test rouge (`oe_count(origin) == 1` échoue) ;
  - restaurer.

  Le résultat est noté dans le message de commit.
- **Tests existants** : `test_pico8_mz`, `test_pico8_bg`, `test_pico8_bg_kj`, `test_pico8_fq`, `test_pico8_qb`, `test_pico8_iv`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo` et `test_pico8_mh` ont un diff vide, et leurs valeurs figées sont identiques. `test_pico8_mz` exerce `bv` sur un `jc` struct.
- **Tests de régression** : tous passent avec un diff vide, sauf `slipway_drag` (façon de lire seulement). En particulier : `playability`, `geoharvest_burst`, `rich_planet_exports`, `empty_planet_cleared`, `game_end`, `tech_screen`, `planet_gh_row_lq`, `ga_export_panel_by`.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED` introduit par l'étape. Les binaires `*_refaudit` compilent, en particulier `test_pico8_slipway_drag_refaudit`.
- **Couverture** : `make class-coverage-net` rend 0, avec une sortie identique à la référence (`jc  152  6/15`, `ke  0  0/15  exemptée …`).
- **Prior art** : `test_pico8_mz` (rejeu `phase4_playthrough`, `cg("jc")`, `bv` sur une voie, `fo`, fils forkés), `test_pico8_bg` (rejeux figés, forme, écriture interdite dans un fils) et `test_pico8_iv` (constructeur à clés mesurées).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0, ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`, en particulier si `jc` n'est pas `jc  152  6/15`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé de `jc` ne tombe sous aucune règle du §5. En particulier : si l'assertion des clés reçues ou celle de l'écrivain générique se déclenche sur un rejeu ou un test ; si l'on trouve une écriture générique sur un `jc` d'un autre nom que `fo` ou `p_` ; ou si la table de construction porte une autre clé que `qu`. C'est un arrêt, pas un membre improvisé.
- `FAILED:` si un défaut de `jc` devient un état statique *mutable* sans qu'une modification en place de `nu`/`k_` ait été trouvée et nommée à la ligne 1a.13, ou si `ts`/`hg` deviennent partagés entre instances.
- `FAILED:` si `jo:ldown` ou `nr:ldown` ne passent plus par `pico8_class_lookup_named("jc")` puis `pico8_class_instantiate`, ou si leur code change.
- `FAILED:` si `slipway_drag` change autrement que dans sa façon de lire `held` (assertions, frames ou fixture), si la preuve rouge du §6 manque au message de commit, ou si un autre test de régression (§6) a un diff non vide ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe que `jc` est convertie ou change de forme. En particulier : si `jo`, `nr`, `g_`, `ui`, `trade`, `ie`, `js`, `pj`, l'instance `{qs="-too far-"}`, l'instance `{ra,p}`, le `{}` de `bv` ou le `{}` de `nr:ldown` change de forme, ou si un `->defaults` d'une autre classe bouge.
- `FAILED:` si la `pico8_class_t` de `jc` n'est plus déclarée à la même place, avec sa spec, son parent, son nom de couverture et son enregistrement nommé.
- `FAILED:` si les valeurs figées de la caractérisation de `jc`, ou celles de `test_pico8_mz`, `test_pico8_bg`, `test_pico8_bg_kj`, `test_pico8_qb`, `test_pico8_fq`, `test_pico8_iv`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo` ou `test_pico8_mh`, ont été changées après la conversion.

Critères de réussite (en plus) :

- grep :
  - plus aucun `g_jc_class->defaults` ni `pico8_jc_class()->defaults` dans `src/` et `tests/` ;
  - plus aucun `pico8_class_set_pq(g_jc_class` ;
  - plus aucun `->fields` ni `pico8_instance_get_field` sur `self` dans `slipway.c`/`slipway_display.c` ;
  - plus aucun `pico8_instance_get_field` dans `pico8_probe_field_instance`, sur `fo` dans `planet_display.c`, ni sur `hg` dans `economy_tiers.c` ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB` ;
- la struct `jc` n'a que `pico8_ke_t` et les 16 membres nommés ; son descripteur n'a que `HQ`, `EY`, `EB`, `EV`, `KJ` ; son écrivain n'accepte que `fo` et `p_` ;
- hors de `slipway.c/h`, `slipway_display.c/h`, `pico8_struct.h`, `probe.c`, `planet_display.c` et `economy_tiers.c`, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_jc.c` (nouveau), `test_pico8_slipway_drag.c` (deux helpers) et le `Makefile` (cible et `unit`) bougent ;
- la ligne 1a.13 de `docs/plan-3.md` nomme le test réécrit et les sites découverts, et le message de commit dit « découvert à 1a.13 ».

## Out of Scope

- `{qs="-too far-"}` (1a.39) et `re={ra,p}` (1a.40) : construits à l'identique, en instances sans classe.
- Le `{}` de `bv` (1a.38) et le `{}` passé à `nr:gv` par `nr:ldown` (1a.37).
- `jo` (1a.17), `nr` (1a.24), `mx` (1a.23) : leurs lectures de `oe`, `fo`, `mt`, `p`, `ra`, `id`, `jy` restent sur instance, et leurs `kf`/`cn`/`gv`/`ldown` ne changent pas.
- `trade` (1a.32) et ses listes `rl`/`lp`. `hy` (`fm`), `ui` (`ok`, `lheld`/`rheld`, 1a.31). `g_` (1a.27), même si `bv` lui pose `fo` sur un `jc`.
- La réécriture de `live_ga` dans `geoharvest_burst` (1a.26).
- Le typage C des membres (phase 1b). Le remplacement de `nu`/`k_` par des tableaux fixes (phase D).
- Libérer les `jc` abandonnés ou des parties précédentes (TROU-7, Plan 2 §8).
- L'audio (`rn(50)`, `rn(-1)` restent appelés tels quels) : hors périmètre.
- Lancer l'appli raylib : jamais.

## Further Notes

- Sans `EB` dans l'enum, `mv(jc,"eb")` retomberait sur le lecteur de champ et rendrait absent. `jo:kf` ne propagerait plus `fx`, et `jo:cn`/`nr:cn` ne trouveraient plus de voisin : plus de routes commerciales, et le filet serait rouge sur les rejeux qui construisent une voie.
- Sans la conversion de `planet_display.c` (`fo.dw`), le premier glisser dont le tracé touche une planète ferait partir l'assert de `pico8_instance_get_field`. Sans celle de `bd`, c'est la première route commerciale qui le ferait. Sans celle de `bv`, c'est la première sonde au-dessus d'une voie. Et sans le relâchement des asserts de tag, c'est le premier `fo` posé sur un `jc`.
- Sans `fo` dans l'écrivain, une sonde ou un tracé croisant une voie ferait partir l'assert de `pico8_entity_set_field`. Sans `p_`, c'est le premier glisser abandonné.
- `test_pico8_mz` exerce déjà `bv` sur une voie (`ET_JC`, `check_fo_is`). Il devient un garde supplémentaire de la conversion, sans changer.
- Mesures faites par grep sur `src/`, `tests/`, `tools/` et `Makefile` à 3cc50d7, en lisant :
  - `pico8/lua.lua:403-416`, `:465-512`, `:548-558`, `:606-684`, `:1049-1056`, `:1108-1119`, `:1262-1274` ;
  - `pico8_slipway.c/h`, `pico8_slipway_display.c/h`, `pico8_planet.c` (`ldown`, `kf`), `pico8_planet_trade.c` (`kf`, `cn`), `pico8_planet_family_trade.c` (`nr:gv`, `nr:ldown`, `nr:cn`, `mx:kf`), `pico8_planet_display.c` (`gv`, `kj`), `pico8_probe.c` (`bv`, `dl`, `d_`), `pico8_economy_tiers.c` (`bd`), `pico8_economy_formulas.c` (`bk`), `pico8_trade.c`, `pico8_ui.c` (`bh`, `jz`), `pico8_entity.c` (`lt`, `mv`, `dc`, API double), `pico8_bootstrap.c` (`dg`), `pico8_class.c` (délégation, `instance_get_field`), `pico8_struct.h`, `pico8_ke.h`, `pico8_wormhole.c` (modèle) ;
  - `tests/test_pico8_slipway_drag.c`, `tests/test_pico8_mz.c`, `tests/test_pico8_geoharvest_burst.c`, et `tools/class_coverage_net_expected.txt`.
