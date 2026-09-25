---
title: "Plan 3, étape 1a.10 : `mz` (le trou de ver) devient une struct, `jj` lu sur la classe"
description: "Le trou de ver `mz` est encore une instance de la couche objet : il passe en struct fille de `ke` avec ses méthodes `hq`, `ev`, `kj`, `kz`, `ey` et `pr` (l'enum gagne `HQ`, `EY`, `KZ`, `PR`). `jj` devient une table possédée par le module et lue sur la classe, l'enveloppe `mz.jj` reste une instance sans classe (1a.11). Les helpers partagés `bv`, `dl`, `d_`, `ks` et la lecture de `s.qu` dans `jo:gv`/`nr:gv` passent à la double forme."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.10 — `mz` (`wormhole.c:270`), `jj` lu sur la classe. Tests : aucun.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Défauts mutés sur la classe** : […] deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** […].
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde qu'une copie en lecture pour les descendants non convertis.

§6 : l'étape ne nomme aucun test de régression. Ceux qui atteignent un `mz` (`playability`, `geoharvest_burst`) ne lisent aucun de ses champs et doivent passer avec un diff vide. §7 : le commit passe `make class-coverage-net` avec une sortie identique à `tools/class_coverage_net_expected.txt`. Un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la liste des étapes dans le commit, avec « découvert à 1a.10 » dans le message.

### Ce que 1a.1 à 1a.9 ont posé (commits 9ce8bfa, 42fdf50, f5cc6d1, 28a08ac, 824725c, ec01e86, 470accc, 9aa4812), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT` ; `pico8_struct_header_t` ; descripteur `pico8_struct_class_t` : méthodes indexées par l'enum (`OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD` aujourd'hui), lecteur de champ qui rend 1 si le champ n'est pas nil, écrivain de champ. Un nom absent de l'enum est cherché comme champ.
- `pico8_ke_t` en premier membre, `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq` : une fille l'appelle une fois, après son propre `pq`, ou seul si elle n'en a pas.
- `pico8_class_set_struct_new` : `pico8_class_instantiate` délègue au constructeur de struct, qui enregistre la couverture.
- `pico8_entity_get_field` / `pico8_entity_set_field` à double forme. L'écrivain asserte s'il n'y a pas d'écrivain ou si le nom n'a pas de membre. `pico8_instance_get_field` asserte s'il reçoit une struct.
- `dg` écrit `t` directement dans `pico8_ke_t` ; `mv` appelle une entrée de la table ; `lt`/`he`, `dc` (`fa`, `ra`), le scan `fx` de `mz:pq` (1a.2) et `pico8_ij` (lecteur `p` de `entity_geom.c`, 1a.8) sont déjà à double forme.
- **Modèle d'une classe avec un `pq` propre : `iv` (1a.2).** Le `pq` de la classe est appelé par le constructeur de struct, puis `pico8_ke_struct_pq` ; `pico8_class_set_pq` disparaît.
- **Modèle des tables par défaut partagées et jamais modifiées : `nu` de `mh` (1a.4, `g_mh_nu`) et `pc` de `qb` (1a.8, `g_qb_pc`).** La table appartient au module, elle est construite à l'init et libérée au shutdown, et le membre est une REF vers elle.
- Modèle de test : `test_pico8_qb` (caractérisation figée sur l'ancien code, lecture seulement par l'API à double forme, rejeux dans un fils forké juste après `pico8_cart_load()`, vérification de forme ajoutée après la conversion, écriture interdite vérifiée dans un fils, mutations notées dans le commit).

### Ce que fait le cart

`pico8/lua.lua` :

```
:376-377  function bg:ldown(mp) return (#(cg.g_ or {})>0 and mz or g_)()end
:552-558  function bv(self,mp,rp)self.ra,self.p=mp,mp/128
            local nq,c=ij(self.p,cg.np)self.et= nq<self.dw and c or
            jp(cg.jc,function(sw) return be(sw.qu.p,sw.re.p,self.p,0.0032)end)[1]or
            not ui.hw(rp)and {}
            if (self.et)self.et.fo=self end
:559-567  function dl(self)if self.me=="kz" then local mp,t=je.ra
            if self.et then t=self.et.qs if (not t)spr(31,…) elseif self.od then t=self.od.."$"end
            io(t or "",…)end end
:568-574  function d_(self)if not self.et and fm:lu(self.od)then
            self:ks(self.dq or "ph")fm:nx(self.fp or 0)rn(self.ik or 54) return true
            else rn(57)self.p_=true end
:752-762  mz=ke:jr([[ nu=o("mz"), me="kz",dw=0, dq="pr", spawn_snd=52,ik=49, oi=0,
            od=3,fp=1, fa=5, kk=o(7,7,6,13,5,1,1,1,0), jj=o(qs="-too far-"), ]])mz.hq=bv
:763      mz.ev=dl
:764-766  function mz:pq()rn(53)self.ox=jp(hv,function(p) return p.fx end)end
:767-771  function mz:hq(...)bv(self,...)local nq
            nq,self.ij=ij(self.p,self.ox)if nq>ov("space folding",0.19,0.27)then
            self.et=mz.jj end end
:772-773  function mz:kz(t)self.oi=ov("space folding",29,35)*(min(t/15,1)+sin(t/40)*0.1)end
:774-775  function mz:ey()self.p_= self.t<15 or not d_(self)end
:776-782  function mz:pr(t)local sr=self:ej(t)/128+0.031
            for pt in all(cg.jo)do if pt.me=="unknown"and (pt.p-self.p):qx()<sr then pt:hh()end end
            self.p_=t>25 end
:783-793  function mz:kj(p) … self.et … self.oi … self.ij.ra … self.t … self.kk[flr(sr/self.oi*8)] …
:794-796  function mz:ej(t) return min(sqrt(t/15),1)*self.oi end
:472-477  function jo:gv(s)if self.me=="ly" then if s.qu==self then self:gh()else return true end end end
:1109-1111 function nr:gv(s) return s.qu~=self and #self.oe<3 end
:245      local nw=self:jz("gv",held)mv(held,"ey",nw and nw.kv)self[ni]=nil   -- ui:bh
```

Points décisifs :

- **`jj` est lu sur la classe** (`self.et=mz.jj`), jamais sur une instance, et sa table n'est **jamais modifiée en place** : `bv` pose `et.fo=self` sur l'`et` qu'il vient de calculer (planète de `cg.np`, voie de `cg.jc` ou `{}` neuf), et jamais sur `jj`. `mz:hq` remplace `et` par `jj` **après** `bv`, sans rien y écrire ; `dl` ne fait qu'y lire `qs`. **Aucun défaut de `mz` n'est réaffecté sur la classe ni modifié en place** : `oi` est réaffecté sur l'instance (`kz`), `kk` et `nu` sont seulement lus. L'étape n'introduit donc aucun état statique *mutable* et n'ajoute rien à la liste des défauts mutés connus du §5.
- `mz.hq=bv` est redéfini trois instructions plus loin par `function mz:hq` : seule la seconde définition est atteignable (déjà noté dans `wormhole.c`). `mz.ev=dl` : `ev` est le `dl` partagé.
- `bv`, `dl`, `d_` sont **partagés** avec `qh`, `g_`, `jc` et l'enveloppe `ie.ke` (tous encore instances). `ks` (méthode de `ke`) est partagé avec `jo`.
- `me` vaut `"kz"` puis `"pr"` (`d_` appelle `ks(dq)`, avec `dq="pr"`). `dg` appelle `mv(qc, me, t)` : `kz` et `pr` sont donc appelées par nom de chaîne. `hq` et `ey` le sont par `ui:bh` (`mv(held,"hq")`, `mv(held,"ey")`), `kj` et `ev` par `dc`.
- `dw=0` : `nq<self.dw` n'est jamais vrai (une distance n'est jamais négative). Un `mz` n'est donc **jamais** posé en `fo` d'une planète. Il est posé en `fo` d'une voie `jc` (lue seulement en vérité par `slipway_display.c:207-208`, jamais déréférencée) ou d'un `{}` jeté.
- Au relâcher, `ui:bh` appelle `jz(ok,"gv",held)` : si une planète `"ly"` ou un `nr` est survolé, `jo:gv(s)` / `nr:gv(s)` lisent **`s.qu` sur le `mz`**.

### Emprise réelle de `mz` (mesurée sur 9aa4812)

| Aspect | Mesure |
|---|---|
| Constructions | Un seul site en jeu : `pico8_bg_ldown` (`bg.c:184-193`) appelle `pico8_class_lookup_named(count>0 ? "mz" : "g_")`, puis `pico8_class_instantiate(cls, NULL)` et renvoie le résultat en REF. Il passe **par la délégation `struct_new`, sans modification** (`bg` est converti en 1a.12). Hors jeu : `pico8_mz_new(NULL)` dans `test_pico8_iv.c:187`. Classe enregistrée par `pico8_class_register_named("mz", …)` (`wormhole.c:285`), sans `fz`, ni `gx[]` ni `kv[]`. Déclarée par `pico8_wormhole_init` (`cart.c:89`) ; `cart.c:100` asserte `pico8_mz_class()->parent == pico8_ke_class()` (reste vrai). |
| Défauts | `nu`=`o("mz")`, `me`=`"kz"` (masque le `"ph"` de `ke`), `dw`=0, `dq`=`"pr"`, `spawn_snd`=52, `ik`=49, `oi`=0, `od`=3, `fp`=1, `fa`=5, `kk`=table de 9 entiers `7,7,6,13,5,1,1,1,0`, `jj`=table `{qs="-too far-"}`. Hérité de `ke` : `t`=0. Aucun défaut muté. `spawn_snd` n'est lu nulle part (ni dans le cart ni dans `src/`). |
| Champs écrits sur l'instance | `ox` (`mz:pq`, liste sur le tas, TROU-7) ; `ra`, `p`, `et` (`bv`) ; `ij`, `et` (`mz:hq`) ; `oi` (`mz:kz`) ; `p_` (`mz:ey`, `mz:pr`, `d_`) ; `me`, `t` (`ks` via `d_`) ; `t` (`dg`). |
| Méthodes | `pq` (installée par `pico8_class_set_pq`, `wormhole.c:274`) ; `hq`, `ev`(=`dl`), `kz`, `ey`, `pr`, `kj`. Ces six-là sont **installées aujourd'hui par `->defaults`** (`wormhole.c:279-284`). `ej` n'est appelée que directement (`pr`, `kj`). Dans l'enum, seules `KJ` et `EV` existent : **l'enum gagne `HQ`, `EY`, `KZ`, `PR`**, parce que chacune est appelée par nom de chaîne (règle « Méthodes »). |
| `->defaults` | 7 sites, tous dans `pico8_wormhole_init` : 6 installations de méthode (`:279-284`) et 1 lecture, celle de `jj` pour construire l'enveloppe (`:287`). Aucun `g_mz_class->defaults` ni `pico8_mz_class()->defaults` ailleurs dans `src/` ou `tests/`. |
| Noms lus sur un `mz` par un consommateur générique | `lt`/`he` : `kj`, `ev`, `hd`, `hk`, `hb`, `nu`, d'où `hv`, `cv.kj`, `cv.ev` et `cg.mz`. `dg` : `me`, `t`, `p_`, et `mv(me)`. `dc` : `fa`, `ra`, `mv("kj")`, `mv("ev")`. `ui:bh` : `mv(held,"hq")`, `mv(held,"ey")`. `bv` : `dw`. `dl` : `me`, `et`, `od`. `d_` : `et`, `od`, `dq`, `fp`, `ik`. Scan `fx` d'un `mz:pq` suivant : `fx` (absent). `jo:gv`/`nr:gv` : `qu` (absent). |
| Écrivains génériques qui atteignent un `mz` | `bv` : `ra`, `p`, `et` (`probe.c:127-179`, par `self->fields` → **écriture hors bornes** sur une struct ; la garde `self->fields == NULL` lit le descripteur). `d_` : `p_` (`probe.c:311`, **hors bornes**). `ks` : `me`, `t` (`ke.c:40-41`, **hors bornes**). |
| Lecteurs à simple forme qui atteignent un `mz` (**assert** de `pico8_instance_get_field`) | `bv` : `pico8_probe_field_fixed(self,"dw")` (`probe.c:39-44`). `dl` : `me`, `et`, `od` (`probe.c:205,214,236`). `d_` : `et`, `od`, `dq`, `fp`, `ik` (`probe.c:269-301`). `jo:gv` : `s.qu` (`planet_display.c:361`). `nr:gv` : `s.qu` (`planet_family_trade.c:60`). Ceux de `wormhole.c` et `wormhole_display.c`, propres à `mz`, sont réécrits sur les membres. |
| Consommateurs génériques jamais atteints | `cv.hd` (boucle de survol `gw`, `jz`), `cv.hk`/`cv.hb` (`economy`), `cg("mz")` (jamais lu), `ui:cj` (`le` ne tient que des `js`), `planet_display.c:564-573` (`fo.dw` : jamais un `mz`, voir `dw=0`), `slipway_display.c:207-208` (`jc.fo` lu en vérité seulement). |
| Clé dynamique | Aucune. |
| `pico8_ref_audit.h` | Aucune macro ne reçoit un `mz` (seuls `ox`, une liste, et `kk`, une table, passent par des casts). L'assert `ij` INSTANCE de `wormhole_display.c:53` reste vrai : `ij` est une planète ou une sonde `g_`. |
| `->cls` lu sur une entrée de `hv` | `test_pico8_geoharvest_burst.c:85` (`live_ga`, sans vérifier le tag). Le rejeu construit un `mz` (étape 1 du fixture). La lecture reste dans les bornes : un `mz` fait bien plus de 24 octets, et l'octet 16 tombe sur `ke.me`. Elle n'est jamais égale à la classe `ga` et n'est jamais déréférencée. C'était déjà vrai pour `iv`/`mh`/`fq`/`nf`/`qb`, et la réécriture est prévue en 1a.26. `test_pico8_qb.c:390` et `score_panel_close` vérifient le tag. |
| Tests qui lisent un `mz` | `test_pico8_iv.c:177-190` (`test_mz_scan_skips_a_live_toast`) : `pico8_mz_new(NULL)` puis `pico8_instance_get_field(mz,"ox")` → **assert après conversion**. Seul le lecteur change (voir Solution 12). `test_pico8_playability.c` (étape 2, frame 54) et `test_pico8_geoharvest_burst.c` (étape 1) : ils construisent un `mz` par le rejeu, sans lire ses champs. |
| Couverture | `mz  146  9/15` (inchangé attendu). |

### L'enveloppe `mz.jj` : ce qui reste pour 1a.11

`g_mz_jj_wrapper` (`wormhole.c:293-296`) est une instance sans classe (tag `INSTANCE`, `cls` NULL) dont les `fields` empruntent la table `jj`. Elle **reste une instance** dans cette étape, pour trois raisons. `bv` asserte que `et` est une instance (`probe.c:182-184`). `dl` y lit `qs` par `pico8_instance_get_field`. Et la convertir, c'est l'étape 1a.11. Seule change **d'où vient la table empruntée** : aujourd'hui les défauts de la classe (un accès `->defaults`), demain la table `jj` possédée par le module.

## Solution

1. **Struct `mz`** (publique dans le module du trou de ver, comme `pico8_qb_t`) : premier membre `pico8_ke_t`, puis les défauts de la chaîne `nu`, `dw`, `dq`, `spawn_snd`, `ik`, `oi`, `od`, `fp`, `fa`, `kk`, `jj`, puis les champs écrits sur instance `ox`, `ra`, `p`, `et`, `ij`, `p_`. Tous sont des `pico8_value_t` et gardent leur nom minifié. `me` et `t` sont ceux de `pico8_ke_t`.
2. **Tables de classe possédées par le module** : `nu` (`o("mz")`), `kk` (9 entrées) et `jj` (`{qs="-too far-"}`) sont construites par `pico8_wormhole_init` à partir de la même spec, et libérées par `pico8_wormhole_shutdown` **après** l'enveloppe. C'est l'état de classe du module, partagé par tous les `mz` et jamais modifié. `jj` est **lu sur la classe** : `mz:hq` pose en `et` l'enveloppe du module (`pico8_mz_jj_wrapper()`), dont les `fields` empruntent la table `jj` du module. Le membre `jj` de chaque struct est une REF vers cette même table : la règle du §5 (« chaque champ de sa chaîne ») est tenue, et aucun code ne lit ce membre.
3. **Valeurs initiales** : après `pico8_ke_struct_init`, `me` = `"kz"` ; `dw`=0, `dq`=`"pr"`, `spawn_snd`=52, `ik`=49, `oi`=0, `od`=3, `fp`=1, `fa`=5 ; `nu`, `kk`, `jj` = REF vers les tables du module ; `ox`, `ra`, `p`, `et`, `ij`, `p_` = nil.
4. **Constructeur** (posé comme `struct_new` de la `pico8_class_t` de `mz`), dans l'ordre : alloue ; `pico8_ke_struct_init` avec le descripteur de `mz` ; valeurs initiales ; la table reçue doit être NULL ou vide, sinon assertion porteuse de message (arrêt §7) ; enregistre `"mz"` au crochet de couverture ; `mz:pq` (`rn(53)`, puis scan `fx` de `hv` écrit dans le membre `ox`) ; **puis** `pico8_ke_struct_pq`. C'est l'ordre du parcours de classe : le `pq` dérivé d'abord, donc le `mz` neuf n'est pas encore dans `hv` quand il scanne. `pico8_class_set_pq(g_mz_class, …)` disparaît.
5. **Enum de méthodes** : il gagne `HQ` (`"hq"`), `EY` (`"ey"`), `KZ` (`"kz"`), `PR` (`"pr"`), et rien d'autre. Sur tout autre descripteur l'entrée est NULL : le nom retombe sur le lecteur de champ, qui le rend absent comme aujourd'hui. Aucun de ces noms n'est un nom `bm`, donc `lt`/`he` ne changent pas.
6. **Descripteur de `mz`** : méthodes `HQ`, `EV` (le `pico8_dl_dispatch` partagé), `KJ`, `KZ`, `EY`, `PR`. Son lecteur de champ connaît les 17 membres propres, puis délègue à `pico8_ke_field`. Son écrivain n'accepte que `ra`, `p`, `et` (écrits par `bv`) et `p_` (écrit par `d_`), et rend 0 pour tout autre nom (l'assert de `pico8_entity_set_field` part). Un accesseur public du descripteur s'ajoute, comme `pico8_qb_struct_class()`.
7. **Méthodes propres sur la struct** : `pico8_mz_pq`, `pico8_mz_hq`, `pico8_mz_kz`, `pico8_mz_ey`, `pico8_mz_pr`, `pico8_mz_ej` et `pico8_mz_kj` (module d'affichage) prennent la struct `mz`, lisent et écrivent ses membres directement, et gardent leurs gardes NULL, leurs adaptateurs et le TROU-3/TROU-7 existants. `pico8_mz_hq` appelle toujours `pico8_bv` en premier (cast vers le type d'entité, comme pour `hv`). Les helpers statiques de lecture de `wormhole.c` / `wormhole_display.c` que la conversion rend inutiles disparaissent. L'assert `ij` INSTANCE est gardée.
8. **Helpers partagés, coexistence** (`probe.c`, `ke.c`) :
   - `pico8_probe_field_fixed` passe à `pico8_entity_get_field`. Le résultat est identique sur toute non-struct (`g_`, `fm`, `qh`, `jc`).
   - `bv` : la garde devient « `self` NULL, ou instance dont `fields` est NULL ». Les écritures de `ra`, `p` et `et` sur `self` passent à `pico8_entity_set_field`. L'écriture de `fo` sur la cible `et` (toujours une instance, assert gardée) ne change pas.
   - `dl` : les lectures de `me`, `et` et `od` sur `self` passent à `pico8_entity_get_field`. La lecture de `qs` sur `et` (toujours une instance) ne change pas.
   - `d_` : les lectures de `et`, `od`, `dq`, `fp` et `ik` sur `self` passent à `pico8_entity_get_field`, et l'écriture de `p_` à `pico8_entity_set_field`.
   - `pico8_ke_ks` : sur une struct, il écrit directement `me` et `t` dans son `pico8_ke_t` (comme `dg` pour `t`). Sur une instance, rien ne change.
9. **`jo:gv` et `nr:gv`** : seule la lecture de `s.qu` sur l'argument passe à `pico8_entity_get_field`. Le reste de ces fonctions (lectures sur `self`, qui est une planète encore instance) ne change pas.
10. **`pico8_wormhole_init` / `_shutdown`** : même déclaration, même spec, même parent, même nom de couverture, même `pico8_class_register_named("mz", …)`. La classe reçoit le constructeur et perd les 6 installations `->defaults` et la lecture `->defaults` de `jj`. L'enveloppe `mz.jj` est construite comme aujourd'hui (tag `INSTANCE`, `cls` NULL) sur la table `jj` du module. Le shutdown libère la classe, puis l'enveloppe par un `free` simple, puis les trois tables du module. Il ne libère jamais un `mz`.
11. **`pico8_mz_new`** rend la struct (comme `pico8_qb_new`). `pico8_bg_ldown` ne change pas.
12. **`test_pico8_iv`** : dans `test_mz_scan_skips_a_live_toast`, le type de la variable et le lecteur (`pico8_instance_get_field` → `pico8_entity_get_field`) changent, et seulement eux. Les deux assertions sur `ox` ne changent pas.
13. **Plan** : ajouter à la ligne 1a.10 de `docs/plan-3.md` le texte « Découvert à 1a.10 : helpers partagés `bv`/`dl`/`d_` (`probe.c`) et `ks` (`ke.c`), `s.qu` lu par `jo:gv` (`planet_display.c:361`) et `nr:gv` (`planet_family_trade.c:60`) ». Aucun défaut muté ne s'ajoute aux défauts connus du §5.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `mz` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise de `mz` soit mesurée dans le PRD (constructions, défauts, champs écrits, méthodes, `->defaults`, lecteurs et écrivains génériques, tests), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux que la struct `mz` n'ait que `pico8_ke_t` et les 17 membres nommés, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun défaut de `mz` ne devienne un état statique *mutable*, afin de n'appliquer la règle des défauts mutés que là où le cart mute (il ne mute rien ici).
5. En tant que mainteneur, je veux que `nu`, `kk` et `jj` soient des tables possédées par le module, construites à l'init et libérées au shutdown, afin qu'aucun accès `->defaults` ne subsiste pour `mz`.
6. En tant que gardien du filet, je veux que `mz:hq` pose en `et` l'enveloppe `jj` du module quand la cible la plus proche est trop loin, afin que « -too far- » s'affiche exactement comme avant.
7. En tant qu'auteur de 1a.11, je veux que l'enveloppe `mz.jj` reste une instance sans classe, sur la table `jj` du module, afin que sa conversion reste une étape à part.
8. En tant que gardien du filet, je veux qu'un `mz` neuf lise `me`=`"kz"`, `t`=0, `dw`=0, `dq`=`"pr"`, `spawn_snd`=52, `ik`=49, `oi`=0, `od`=3, `fp`=1, `fa`=5, `kk` dans l'ordre 7,7,6,13,5,1,1,1,0 et `jj.qs`=`"-too far-"`, afin que son comportement de départ soit identique.
9. En tant que gardien du filet, je veux que `bg:ldown`, quand une sonde `g_` existe, construise un `mz` struct par `lookup_named("mz")` sans que `bg.c` change, afin que la délégation `struct_new` fasse ce que le §5 promet.
10. En tant que gardien du filet, je veux qu'un `mz` neuf soit enregistré dans `hv`, `cv.kj`, `cv.ev` et `cg.mz`, et dans aucun `cv.hd`/`hk`/`hb`, afin que l'ordre de dessin et de mise à jour ne bouge pas.
11. En tant que gardien du filet, je veux que `mz:pq` scanne `hv` pour `fx` **avant** que le `mz` neuf n'y entre, afin que `ox` contienne les mêmes entités, dans le même ordre.
12. En tant que gardien du filet, je veux que `bv` écrive `ra`, `p` et `et` dans les membres du `mz` et pose `fo` sur la cible trouvée, afin que la voie survolée s'allume comme avant.
13. En tant que gardien du filet, je veux que `mz:hq` écrive `ij` (l'ancre la plus proche parmi `ox`), afin que le trait vers l'ancre soit tracé au même endroit.
14. En tant que gardien du filet, je veux que `dg` appelle `kz`, puis `pr` après le `ks("pr")`, par la table via l'enum, afin que l'anneau grandisse et que le dévoilement se fasse aux mêmes ticks.
15. En tant que gardien du filet, je veux que `mz:ey` mette `p_` à vrai avant le tick 15 sans appeler `d_`, afin que le court-circuit du `or` soit gardé (ni son ni débit précoces).
16. En tant que gardien du filet, je veux qu'après le tick 15, `d_` débite `od` crédits, fasse `ks("pr")` et `nx(fp)`, joue `ik`, ou bien refuse (`rn(57)`, `p_`), afin que le coût et le refus du trou de ver soient identiques.
17. En tant que gardien du filet, je veux que `ks` écrive `me` et `t` dans le `pico8_ke_t` d'une struct et dans les `fields` d'une instance, afin que `jo` et `mz` changent d'état comme avant.
18. En tant que gardien du filet, je veux que `mz:pr` révèle chaque planète `"unknown"` dans le rayon `ej(t)/128+0.031` et mette `p_` à vrai à `t>25`, afin que la carte se dévoile identiquement.
19. En tant que gardien du filet, je veux que `mz:kj` dessine l'anneau `oi` (sans `et`), le trait vers `ij.ra` et le sprite 7 en phase `kz`, puis les anneaux colorés par `kk` en phase `pr`, afin que le rendu soit identique au pixel près.
20. En tant que gardien du filet, je veux que `dl` affiche `qs` de `et`, sinon le sprite 31, sinon `od.."$"`, afin que l'étiquette du curseur soit identique.
21. En tant que gardien du filet, je veux que relâcher un `mz` au-dessus d'une planète `"ly"` ou d'un `nr` lise `s.qu` par le lecteur à double forme, afin que `jz("gv",held)` ne fasse pas partir l'assert de la couche.
22. En tant que mainteneur, je veux que chaque helper partagé (`bv`, `dl`, `d_`, `ks`, `pico8_probe_field_fixed`, `jo:gv`, `nr:gv`) rende un résultat identique sur `qh`, `g_`, `jc`, `jo`, `nr`, `fm` et l'enveloppe `ie.ke`, afin que rien de non converti ne bouge.
23. En tant que mainteneur, je veux que l'écrivain de `mz` n'accepte que `ra`, `p`, `et` et `p_`, afin qu'une écriture générique d'un autre nom déclenche l'arrêt du §7 au lieu d'un membre improvisé.
24. En tant que mainteneur, je veux qu'une clé reçue par le constructeur déclenche une assertion porteuse de message, afin qu'un champ jamais mesuré ne soit pas perdu en silence.
25. En tant que gardien de la couverture, je veux que le constructeur enregistre `"mz"` une fois, jamais `"ke"`, afin que `mz  146  9/15` et `ke  0  0/15` restent identiques.
26. En tant que mainteneur, je veux que la `pico8_class_t` de `mz` reste déclarée à la même place, avec sa spec, son parent, son nom de couverture et son enregistrement nommé, afin que `KNOWN[]`, l'assert de `cart.c` et `lookup_named` tiennent.
27. En tant que mainteneur, je veux que les 7 accès `->defaults` de `wormhole.c` disparaissent, afin d'appliquer la règle des accès directs aux défauts.
28. En tant qu'auteur des étapes suivantes, je veux que l'enum ne gagne que `HQ`, `EY`, `KZ`, `PR`, afin que chaque ajout reste attaché à une étape.
29. En tant que gardien de la mémoire, je veux que le shutdown libère la classe, puis l'enveloppe par un `free` simple, puis les tables du module, jamais un `mz`, afin que TROU-7 reste la seule convention de fuite.
30. En tant que gardien des tests, je veux que `test_pico8_iv` ne change que son lecteur de `ox`, avec les mêmes assertions, afin que sa caractérisation garde son objet.
31. En tant que gardien des tests de régression, je veux que tous passent avec un diff vide (en particulier `playability` et `geoharvest_burst`, qui construisent un `mz`), afin que la conversion ne se valide pas en affaiblissant un test.
32. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
33. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.10, afin que la liste des étapes dise ce que l'étape a réellement touché.
34. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés, afin de ne laisser aucun contrat mensonger.
35. En tant que vérificateur, je veux une caractérisation du trou de ver figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (crédits débités, planètes révélées, `ox`, `ij`).

## Implementation Decisions

Aucun outil d'agent n'était disponible dans cette session pour lancer un agent `guideline` frais. J'ai chargé le skill `guideline` et je l'ai appliqué moi-même (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Pas d'état statique mutable ; trois tables de classe en lecture seule.** La preuve est au Problem Statement : `jj` n'est jamais modifiée en place, `kk` et `nu` sont seulement lues, et `oi` est réaffecté sur l'instance. Si l'implémenteur trouve une modification en place (un `pico8_ob_table_set_*` sur `jj`, `kk` ou `nu`, ou un `fo` posé sur l'enveloppe), c'est la règle des défauts modifiés en place qui s'applique (site découvert, état statique partagé, ligne 1a.10), pas un membre.
- **(G) `jj` : lu sur la classe, et aussi membre.** Le texte de l'étape impose la lecture sur la classe : `mz:hq` prend l'enveloppe du module, jamais `self.jj`. Le §5 impose que chaque défaut de la chaîne soit un membre, donc `jj` en est un, sur la même table. Le coût est une REF que rien ne lit. Rejeté : pas de membre `jj`. C'est plus court, mais c'est un défaut de la chaîne qui ne tombe plus sous la règle des membres, ce qui ouvre un risque d'arrêt §7. Même raisonnement pour `spawn_snd`, que personne ne lit.
- **(G) Tables du module** : même mécanisme que `g_mh_nu`/`g_qb_pc`. Elles sont parsées par `pico8_ob` depuis les fragments de la spec, et `jj` est la table sur laquelle l'enveloppe est construite. Rejeté : continuer d'emprunter les tables de `g_mz_class->defaults`, car c'est un accès `->defaults` que le §5 fait migrer, et `mz` n'a aucun descendant qui en aurait besoin. La `pico8_class_t` garde sa spec telle quelle, comme copie en lecture jamais lue.
- **Enum** : `HQ`, `EY`, `KZ`, `PR` s'ajoutent, parce que chacune est appelée par nom de chaîne (`dg` via `me`, `ui:bh`). `ej` et `pq` n'y entrent pas : elles ne sont appelées que directement.
- **`pq` dans le constructeur** : c'est le modèle `iv`, et l'ordre (`mz:pq`, puis `lt`) est celui du parcours de classe. Un `lt` placé avant le scan ferait entrer le `mz` neuf dans son propre `ox` s'il avait `fx`. Il n'en a pas, mais l'ordre des effets (`rn(53)`, puis l'enregistrement) est gardé tel quel.
- **Écrivain `ra`/`p`/`et`/`p_` seulement** (G) : ce sont les seuls noms qu'un écrivain générique pose sur un `mz` (mesuré : `bv`, `d_`). `ks` écrit `me`/`t` directement dans `pico8_ke_t`, qui est commun à toute struct descendante de `ke`, et c'est l'analogue exact de `dg`. Rejeté : faire passer `ks` par `pico8_entity_set_field`, qui obligerait chaque futur écrivain de fille à accepter `me`/`t`.
- **Helpers partagés** : seul l'appel change (lecteur ou écrivain à double forme) sur les sites qui reçoivent `self` ou `s`. Les lectures et écritures sur les cibles (`et.fo`, `et.qs`, `sw.qu`/`sw.re`) et sur `self` dans `jo:gv`/`nr:gv` ne changent pas (G, changement chirurgical) : aucun `mz` ne les atteint.
- **Garde de `bv`** (G) : `self->fields == NULL` n'a de sens que pour une instance. Sur une struct, `fields` lirait le descripteur. La garde devient donc conditionnelle au tag : le comportement est identique sur toute instance, et une struct n'est jamais écartée.
- **`bg:ldown` n'est pas touché** : `lookup_named` + `pico8_class_instantiate` construit la struct par délégation. C'est la vérification que le §5 attend de cette étape.
- **`live_ga` de `geoharvest_burst`** (G) : laissé tel quel. La lecture reste dans les bornes, n'est jamais égale et n'est jamais déréférencée. La réécriture est prévue en 1a.26.
- **`test_pico8_entity_smoke`** : il n'est pas touché (il ne construit aucun `mz`).
- **Commentaires** (CLAUDE.md) : ajouter le contrat de la struct (jamais libérée, TROU-7 ; `nu`/`kk`/`jj` partagées, jamais modifiées en place), le contrat de `pico8_mz_new` (rend une struct, table reçue vide seulement) et l'assert porteuse (clé reçue). À corriger parce que le changement les rend faux : `wormhole.h:17` (« Frees the `mz` class and the `jj` wrapper » : libère aussi les tables), `wormhole.h:25-27` (« `overrides` is copied »), `wormhole.h:29-34` (« `fields` are borrowed from the class defaults » : ils viennent désormais de la table `jj` du module), `wormhole.c:299-300` (« owned by the class defaults »), `wormhole.c:275-278` (le piège `mz.hq=bv` reste vrai, mais il porte désormais sur l'entrée `HQ` de la table), et l'en-tête de `wormhole_display.h` (« `self` » est une struct). Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation de `mz` sur l'ancien code.** Nouveau test (voir Testing), valeurs figées **avant** toute modification de `wormhole*.c`, `probe.c`, `ke.c`, `planet_display.c`, `planet_family_trade.c`, `pico8_struct.h`.
2. **Coexistence, sans classe convertie.** `bv`, `dl`, `d_`, `pico8_probe_field_fixed`, `ks`, `jo:gv` et `nr:gv` passent à la double forme. Aucune classe ne change de forme : le filet et la couverture sont trivialement identiques.
3. **`mz` converti.** Enum, struct, tables du module, descripteur, constructeur, méthodes sur la struct, classe reliée, `->defaults` retirés, enveloppe sur la table du module, accesseur, `test_pico8_iv` (lecteur), ligne 1a.10 du plan. La caractérisation passe **sans changer ses valeurs figées**.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field` et `pico8_mv`, l'appartenance à `hv`/`cv`/`cg`, les crédits de `fm`, les `me` des planètes, les pixels du framebuffer, le compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_mz`** (dans `make unit`, lié comme `test_pico8_qb`, sur le cart réel chargé). Il lit **uniquement** par l'API à double forme (`pico8_entity_get_field`, `pico8_entity_set_field`, `pico8_mv`, `pico8_hv`/`pico8_cv`/`pico8_cg`, `pico8_dg`, `pico8_dc`, `pico8_class_instantiate(pico8_class_lookup_named("mz"), NULL)`), pour que le code soit le même avant et après. Chaque scénario dépendant de l'ordre `rnd` tourne dans un fils forké juste après `pico8_cart_load()` (modèle `in_fresh_child` de `test_pico8_qb`).
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction** (partie lancée par `eu()`, sonde posée) : par `lookup_named("mz")`, `pico8_mz_new(NULL)` et `pico8_bg_ldown` (avec `cg.g_` non vide). Lire `me`, `t`, `dw`, `dq`, `spawn_snd`, `ik`, `oi`, `od`, `fp`, `fa`, les 9 entrées de `kk`, `jj.qs`, `nu[1]` ; vérifier que `ra`, `p`, `et`, `ij`, `p_`, `fx` et `qu` sont absents et que `ox` est présent. Le `mz` est dans `hv`, `cv.kj`, `cv.ev` et `cg.mz`, et dans aucun de `cv.hd`/`hk`/`hb`. Avec `cg.g_` vide, `bg:ldown` ne construit pas de `mz`.
    2. **`ox`** : son compte et, pour chaque entrée, son rang dans `hv`, repliés en empreinte. Un `mz` construit après un autre `mz` vivant ne met pas le premier dans son `ox`.
    3. **`hq`** en quatre positions scriptées (sur une voie `jc` si une existe dans la partie, loin de toute ancre, dans `ui.hw`, et dans le vide proche d'une ancre). Pour chacune : `ra`, `p`, le genre de `et` (voie, enveloppe `jj` identique à `pico8_mz_jj_wrapper()`, `false`, `{}` sans classe), `qs` de `et`, le rang de `ij` dans `hv`, et le `fo` de la voie égal au `mz`. Empreinte figée.
    4. **Ticks `kz`** : de `t`=0 à 20 par `dg`, `oi` par tick, avec `p_` absent. Empreinte + nombre figés.
    5. **`ey`** dans trois cas. À `t`<15 : `p_` vrai, crédits inchangés. À `t`≥15 avec `et` posé : `p_` vrai, crédits inchangés. À `t`≥15 sans `et`, avec crédits suffisants : `me`=`"pr"`, `t`=0, `p_` faux, crédits − `od`. Figer aussi le cas de crédits insuffisants.
    6. **Ticks `pr`** jusqu'à la sortie de `hv` : par tick, le nombre de planètes passées de `"unknown"` à autre chose, `p_`, et l'appartenance à `hv`/`cv`/`cg`. Empreinte figée, et tick de sortie figé.
    7. **Dessin** : `dc("kj")` puis `dc("ev")` sur un framebuffer effacé, en phase `kz` sans `et`, en phase `kz` avec l'enveloppe `jj` (texte « -too far- »), et en phase `pr` à `t`=4, 9 et 20. Empreinte du framebuffer par cas, figée.
    8. **Rejeu de `phase4_playthrough_events.json`** (non-titre, piloté comme `test_pico8_playability`) sur toutes ses frames : par frame et pour chaque `mz` de `cg.mz`, `me`, `t`, `oi`, `ra`, `p`, le genre de `et` et `p_`, plus les crédits de `fm`, repliés en empreinte de séquence + nombre de frames. Figés.
    9. **Rejeu de `geoharvest_burst_events.json`** (non-titre, comme `test_pico8_geoharvest_burst`) sur les frames 0 à 40 : même empreinte. Figée.
    10. **`gv` sur un `mz`** : `pico8_jo_gv(planète "ly", mz)` rend vrai, et `pico8_nr_gv(nr, mz)` rend `#oe<3`, si la partie scriptée en fournit un ; sinon le cas est noté comme omis dans le commit.
  - **Forme** (ajoutée au morceau 3) : le `mz` rendu par `lookup_named`/`bg:ldown` est tagué `STRUCT` avec le descripteur de `mz` (accesseur) ; `pico8_mv(mz,"hq"|"ey"|"kz"|"pr"|"kj"|"ev",…)` trouve une méthode ; `pico8_mz_jj_wrapper()` est tagué `INSTANCE` avec `cls` NULL, et ses `fields` sont la table REF du membre `jj` ; `pico8_entity_set_field(mz,"oi",…)` asserte (vérifié dans un fils, `SIGABRT`).
  - **Mutations** : l'implémenteur remet localement au moins 10 erreurs plausibles et note dans le commit celles qui sont attrapées : `HQ`, `KZ`, `PR` ou `EY` absent de la table ; `EV` pointé sur autre chose que `dl` ; `kk` dans le désordre ; `jj` pris sur une table neuve plutôt que sur celle de l'enveloppe ; `lt` appelé avant le scan `ox` ; `pico8_ke_struct_pq` omis ; `me` initial laissé à `"ph"` ; `dw` ≠ 0 ; `ks` qui n'écrit plus `t` sur une struct ; `d_` qui n'écrit plus `p_` ; écrivain qui ignore `et` ; `ey` qui appelle `d_` avant le tick 15 ; `ej` sans le `min(…,1)`. Chaque mutant équivalent éventuel est justifié dans le commit (par exemple `spawn_snd` ou `jj` membre à une valeur fausse : aucun code ne les lit, donc équivalents par construction, ce qui se dit plutôt que de se tester).
- **`test_pico8_iv`** : son diff se limite au type et au lecteur de `test_mz_scan_skips_a_live_toast`, et ses valeurs figées sont identiques.
- **Tests de régression** : tous passent avec un diff vide, en particulier `playability` (étape 2 : `mz` sur p27, `pr` qui le révèle), `geoharvest_burst` (sonde `mz` qui révèle p27), `slipway_drag`, `empty_planet_cleared`, `game_end` et `tech_screen`.
- **`test_pico8_nf`, `test_pico8_je`, `test_pico8_iv`, `test_pico8_qb`, `test_pico8_mo`, `test_pico8_fq`, `test_pico8_mh`** : valeurs figées identiques.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence (`mz  146  9/15`, `ke  0  0/15  exemptée …`).
- **Prior art** : `test_pico8_qb` (fils forkés, forme ajoutée, écriture interdite dans un fils), `test_pico8_iv` (classe avec `pq` propre, scan `fx`), `test_pico8_playability` et `test_pico8_geoharvest_burst` (pilotage non-titre), `test_pico8_mh` (table `nu` du module).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`, en particulier si `mz` n'est pas `mz  146  9/15`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé de `mz` ne tombe sous aucune règle du §5. En particulier : si l'assertion des clés reçues ou celle de l'écrivain générique se déclenche sur un rejeu ou un test, ou si l'on trouve une écriture générique sur un `mz` d'un autre nom que `ra`/`p`/`et`/`p_` (hors `me`/`t` par `ks`). C'est un arrêt, pas un membre improvisé.
- `FAILED:` si un défaut de `mz` devient un état statique *mutable* sans qu'une modification en place ait été trouvée et nommée à la ligne 1a.10, ou si, inversement, une modification en place trouvée est traitée en membre.
- `FAILED:` si `mz:hq` lit `jj` sur l'instance (`self.jj`, ou le membre) au lieu de la classe, ou si l'enveloppe `mz.jj` n'emprunte plus la même table que le membre `jj`.
- `FAILED:` si un test de régression de bug (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe que `mz` est convertie ou change de forme. En particulier : si l'enveloppe `mz.jj` (1a.11) cesse d'être une instance sans classe ; si `bg`, `g_`, `qh`, `jo`, `nr`, `jc`, `ie`, `ui`, `qb`, `nf`, `ke`, `jb`, `iv`, `mh`, `fq` ou un provisoire changent de forme ; si `bg.c` est modifié ; si un `->defaults` d'une autre classe bouge.
- `FAILED:` si la `pico8_class_t` de `mz` n'est plus déclarée à la même place avec sa spec, son parent, son nom de couverture et son enregistrement nommé `"mz"`.
- `FAILED:` si les valeurs figées de la caractérisation de `mz`, ou celles de `test_pico8_iv`, `test_pico8_qb`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo`, `test_pico8_fq` ou `test_pico8_mh`, ont été changées après la conversion.

Critères de réussite (en plus) :

- grep : plus aucun `g_mz_class->defaults` ni `pico8_mz_class()->defaults` dans `src/` et `tests/` ; plus aucun `pico8_class_set_pq(g_mz_class` ; plus aucun `pico8_instance_get_field` ni `->fields` sur `self` dans `pico8_bv`, `pico8_dl`, `pico8_d_`, ni sur `s` dans `pico8_jo_gv`/`pico8_nr_gv` ; plus aucun `pico8_instance_get_field` dans `pico8_probe_field_fixed`.
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR` ;
- la struct `mz` n'a que `pico8_ke_t` et `nu`, `dw`, `dq`, `spawn_snd`, `ik`, `oi`, `od`, `fp`, `fa`, `kk`, `jj`, `ox`, `ra`, `p`, `et`, `ij`, `p_` ; son descripteur n'a que `HQ`, `EV`, `KJ`, `KZ`, `EY`, `PR` ; son écrivain n'accepte que `ra`, `p`, `et`, `p_`.
- hors `wormhole.c`/`.h`, `wormhole_display.c`/`.h` et `pico8_struct.h`, le diff de `src/` se limite à : `pico8_probe_field_fixed`, `pico8_bv`, `pico8_dl` et `pico8_d_` dans `probe.c` ; `pico8_ke_ks` dans `ke.c` (et son contrat dans `ke.h`) ; la lecture de `s.qu` dans `pico8_jo_gv` et dans `pico8_nr_gv`.
- la ligne 1a.10 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.10 ».

## Out of Scope

- L'enveloppe `mz.jj` elle-même (1a.11) : sa forme, son tag, sa libération.
- `bg` (1a.12), y compris `bg:ldown` et `lookup_named("g_")` ; `g_` (1a.27) ; `qh`, `jo`, `nr`, `jc`, dont seuls les helpers partagés et la lecture de `s.qu` changent d'appel.
- `bv`'s `{}` jeté (1a.38) et les cibles `et`/`fo` sur les voies et planètes.
- `planet_display.c:564-573` (`fo.dw`) : aucun `mz` n'y arrive (`dw=0`).
- Le typage C des membres : phase 1b. Le remplacement des tables `nu`/`kk`/`jj` par des tableaux fixes : phase D.
- La réécriture de `live_ga` dans `geoharvest_burst` (1a.26).
- L'audio (`rn(53)`, `rn(57)`, `ik`, `spawn_snd`) : appels gardés tels quels, hors périmètre.
- Libérer les `mz` retirés de `hv` (TROU-7, Plan 2 §8).
- Lancer l'appli raylib : jamais.

## Further Notes

- Sans la double forme de `bv`, le premier `hq` d'un `mz` struct écrirait par `self->fields` dans le descripteur `static const` (segfault ou corruption). Sans celle de `pico8_probe_field_fixed`, il ferait partir l'assert à la lecture de `dw`. Les deux sont atteints dès la frame 54 de `phase4_playthrough`, donc le filet serait rouge : c'est le premier point du morceau 2.
- `jo:gv` ne lit `s.qu` que sur une planète `"ly"` survolée au relâcher. `phase4_playthrough` relâche son `mz` sur une p27 encore `"unknown"` et ne l'atteint pas. Le filet peut donc ne pas le voir, d'où le scénario 10.
- Le trou de ver n'agit sur l'écran que par l'anneau, le trait, l'étiquette et le dévoilement : le débit de crédits et `ox`/`ij` hors du trait ne sont vus qu'indirectement. D'où les scénarios 2, 3, 5 et 6.
- Mesures faites par grep sur `src/` et `tests/` à 9aa4812, en lisant `pico8/lua.lua:245`, `:376-377`, `:472-477`, `:552-574`, `:752-796`, `:1109-1111`, ainsi que `pico8_wormhole.c`, `pico8_wormhole_display.c`, `pico8_probe.c`, `pico8_ke.c`, `pico8_entity.c`, `pico8_bootstrap.c` (`dg`), `pico8_ui.c` (`bh`, `jz`), `pico8_bg.c` (`ldown`), `pico8_class.c` (délégation), `pico8_planet_display.c`, `pico8_planet_family_trade.c`, `pico8_slipway_display.c`, `tools/class_coverage_net_expected.txt` et les en-têtes des tests `playability` et `geoharvest_burst`.
