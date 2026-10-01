---
title: "Plan 3, étape 1a.8 : `qb` (la caméra) devient une struct, et le slot `mo` se lit et s'écrit sur les deux formes"
description: "La caméra `qb` est encore une instance de la couche objet : elle passe en struct fille de `ke` avec ses méthodes `ph` et `md` (l'enum gagne `MD`), chaque lecteur et écrivain du slot `mo` (`gd`, `ma()`, `ga:pq`, `bg:hq`, `dc`) apprend la forme struct sans toucher le `mo` provisoire, `pico8_entity_shutdown` cesse de libérer une struct, et `planet_kj_dispatch` est réécrit pour poser son espion `md` sur la struct puis prouvé rouge."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.8 — `qb` (`scroll.c:50`). Tests : `planet_kj_dispatch` (`md`).

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe […]. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Défauts mutés sur la classe** : […] deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** […].
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode) […].

§6 : `planet_kj_dispatch` écrit aujourd'hui l'espion `md` dans `mo->fields` : il touche la struct `qb`, il est donc **réécrit dans ce commit** pour poser son espion sur la struct, et doit échouer si le bug d'origine est remis (bug remis localement, test rouge, correctif restauré). §7 : le commit passe `make class-coverage-net`, sortie identique à `tools/class_coverage_net_expected.txt` ; un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la liste des étapes dans le commit, avec « découvert à 1a.8 » dans le message.

### Ce que 1a.1 à 1a.7 ont posé (commits 9ce8bfa, 42fdf50, f5cc6d1, 28a08ac, 824725c, ec01e86), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT` ; en-tête `pico8_struct_header_t` ; descripteur `pico8_struct_class_t` (méthodes indexées par l'enum `OL`/`PH`/`KJ`/`EV`/`LDOWN`, lecteur de champ « rend 1 si non nil », écrivain de champ, NULL permis quand aucun écrivain générique n'atteint la classe). Un nom absent de l'enum est cherché comme champ.
- `pico8_ke_t`, `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq` (une fille l'appelle une fois, après son propre `pq`, ou seul).
- `pico8_class_set_struct_new` : `pico8_class_instantiate` délègue ; le constructeur enregistre la couverture.
- `pico8_entity_get_field` / `pico8_entity_set_field` à double forme (l'écrivain asserte : pas d'écrivain ; nom sans membre). `pico8_instance_get_field` asserte ne jamais recevoir de struct.
- `dg` écrit `t` dans `pico8_ke_t` ; `mv` appelle une méthode de la table.
- **Modèle direct : `nf` (1a.6) et le `je` provisoire (1a.7).** Le slot `g_je` (type `pico8_instance_t *`, inchangé) tient le provisoire puis une struct `nf` ; chaque lecteur passe par le lecteur à double forme ; la libération au shutdown ne visait plus que l'`INSTANCE`. Les valeurs par défaut vecteur de `nf` (`ra`) partent d'un vecteur statique du module (`g_nf_ra`, comme `g_iv_v` en 1a.2), puisque rien ne les modifie en place.
- `pico8_ui_field_vec` (`ui.c`, lu sur `mo` par `ui:lj`) et `pico8_msgbox_field_vec` (`msgbox.c`, lu sur `mo` par `iv:pq`) passent déjà par `pico8_entity_get_field` depuis 1a.6.
- Modèle de test : `test_pico8_nf`, `test_pico8_fq` (caractérisation figée sur l'ancien code, lecture par l'API à double forme, rejeu depuis le titre, forme ajoutée après conversion, mutations notées dans le commit) ; `score_panel_close` en 1a.5 pour la réécriture d'un test de régression et sa preuve rouge.

### Ce que fait le cart

`pico8/lua.lua` :

```
:153-155  function jl()hv,cv,cg={},{},{} mo,je,ok=qb(),nf(),ui()end
:194      if (nt=="kj" and fl[o])mo:md(ci[o])                -- dans dc(nt), avant la boucle par entité
:218      self.qa=je.ra+mo.ra                                 -- ui:lj
:346-348  function gd(e) return abs(e.ra.x-mo.ra.x-64)<=76 and …
:349-350  function ma(p,e)e.ma=max(0,e.ma-0.1) return p+qm(e.ma,e.t*0.2)end
:381      function bg:hq(mp,rp)mo.v=self.mq-rp …
:945      … je.ra+mo.ra-v(0,5)end                             -- iv:pq
:1079     function ga:pq()f_(self)ez(self,140)mo.ma=3 …
:1128-1133 qb=ke:jr([[ v=v(0,0), ma=0, p=v(192,192), ra=v(192,192), pc=o(v(-1,0),v(1,0),v(0,-1),v(0,1)), ]])
:1134-1141 function qb:ph() local ip=v(0,0) for b=0,3 do if btn(b)or btn(b,1) then ip+=self.pc[b+1]*3 end end
            self.p+=self.v  self.v+=(ip-self.v)*0.2
            self.p.x=mid(-64,320,self.p.x) self.p.y=mid(-64,320,self.p.y) self.ra=ma(self.p,self) end
:1142     function qb:md(fy)local cp=self.ra*(fy or 1)camera(cp.x,cp.y)end
```

Points décisifs :

- **Aucun défaut de `qb` n'est réaffecté sur la classe ni modifié en place.** `self.p+=self.v` et `self.v+=…` rebâtissent un vecteur neuf (métaméthode `+`) : le `.x=`/`.y=` qui suit modifie ce vecteur neuf, jamais le `v(192,192)` du défaut. `ra` est réaffecté (`self.ra=ma(…)`), `mo.v` réaffecté par `bg:hq`, `mo.ma`/`e.ma` réaffectés. `pc` n'est que lu. `na` (seule fonction qui modifie un `ra` en place, `:577`) n'est appelée que sur des planètes (`nl:ly` etc.), jamais sur `mo`. **Aucun état statique de classe au sens du §5** : pas de découverte « défaut modifié en place » à cette étape.
- `ma(p,e)` est **partagée** : appelée sur `qb` (`qb:ph`) et sur les planètes (`planet_display.c:493`, `jo` encore instance). Elle lit `ma` et `t` sur `e` et y écrit `ma`.
- `mo` est écrit hors de `qb` : `ga:pq` (`mo.ma=3`) et `bg:hq` (`mo.v=…`), deux classes non converties (1a.26, 1a.12).

### Emprise réelle de `qb` (mesurée sur ec01e86)

| Aspect | Mesure |
|---|---|
| Constructions | 1 site : `pico8_jl` (`bootstrap.c:48`, `pico8_qb_new(NULL)`), rangé par `pico8_entity_set_mo`. Aucune table reçue. Classe absente des registres `fz`/nommé (« Not reachable through `pico8_class_lookup` »), aucun `gx[]`/`kv[]`/`lookup_named`. Déclarée par `pico8_cart_load` (`cart.c:56`) et, à défaut, par `pico8_jl` (`bootstrap.c:38-39`). |
| Défauts | `v`=`v(0,0)`, `ma`=0, `p`=`v(192,192)`, `ra`=`v(192,192)`, `pc`=table de 4 vecteurs (`(-1,0)`,`(1,0)`,`(0,-1)`,`(0,1)`), lue seulement. Hérités de `ke` : `me`="ph", `t`=0. Aucun défaut muté. |
| Champs écrits sur l'instance | `p`, `v`, `ra` (par `qb:ph`, vecteurs neufs sur le tas, TROU-7) ; `ma` (par `ma()` sur `self`, et par `ga:pq` sur `mo`) ; `v` (par `bg:hq` sur `mo`) ; `t` (par `dg`). |
| Méthodes | `ph` (via `dg`, `mv(qc,"ph",t)`), `md` (via `dc("kj")`, `mv(g_mo,"md",ci[o])`, arg0 `fy` résolu `fy or 1` par l'adaptateur) — **toutes deux installées aujourd'hui par `->defaults`** (`scroll.c:54-55`). Pas de `pq` propre : `lt` hérité de `ke`. `md` n'est pas dans l'enum : **l'enum gagne `MD`** (appelée par nom de chaîne depuis `dc`, règle « Méthodes » du §5). |
| Noms lus sur un `qb` et absents | `kj`, `ev`, `hd`, `hk`, `hb`, `nu` (`lt`/`he`) ; `p_` (`dg`) ; `fx` (scan de `mz:pq`). `qb` n'est que dans `hv` (aucun nom `bm`, aucun `nu`). |
| `->defaults` | 2 sites, tous deux dans `pico8_qb_init` : installation de `ph` et de `md`. `cart.c:59` asserte `pico8_qb_class()->parent == pico8_ke_class()` ; `bootstrap.c:38` teste `pico8_qb_class() == NULL` (classe gardée : restent vrais). Aucun autre `g_qb_class->defaults`/`pico8_qb_class()->defaults` dans `src/` ni `tests/`. |
| Consommateurs génériques atteints | `lt` (`pico8_ke_struct_pq`), `he` (jamais : pas de `p_`), `dg` (`me`, `t`, `p_`, `mv("ph")`), `mv`, `dc("kj")` (`mv(g_mo,"md",…)`), scan `fx` de `mz:pq` : **déjà à double forme**. `cv`, `cg`, boucle de survol `gw`, `jz`, `economy` : jamais atteints. |
| Lecteurs du slot `mo` (`g_mo`, qui tient le `mo` provisoire puis un `qb`) | `entity.c:313` `pico8_mv(g_mo,"md",…)` (double forme) ; `ui.c:206` `pico8_ui_field_vec(pico8_mo(),"ra")` (double forme depuis 1a.6) ; `msgbox.c:239` `pico8_msgbox_field_vec(pico8_mo(),"ra")` (double forme depuis 1a.6) ; **`entity_geom.c:54` `pico8_gd` : `pico8_entity_geom_field_vec(pico8_mo(),"ra")` par `pico8_instance_get_field` — assert sur une struct**. `gd` est appelé par `particle.c:108`, `planet.c:447`, `planet_display.c:487`, `slipway_display.c:182` : sur chaque planète dessinée, donc à chaque frame de partie. |
| Lecteurs/écrivains de `qb` hors du slot | **`entity_geom.c:72-83` `pico8_ma(p,e)`** : lit `ma` et `t` par `pico8_entity_geom_field_fixed` (`pico8_instance_get_field` — **assert**) et écrit `ma` par `pico8_ob_table_set_name(e->fields,…)` (**écriture hors bornes** sur une struct). Appelée par `qb:ph` (`scroll.c:126`, sur `qb`) et `planet_display.c:493` (sur `jo`, instance). |
| Écrivains de `mo` | **`planet_family.c:173` `pico8_ga_pq`** : `pico8_ob_table_set_name(mo->fields,"ma",3)` (**hors bornes**) ; **`bg.c:225` `pico8_bg_hq`** : `pico8_ob_table_set_name(mo->fields,"v",…)` (**hors bornes**). |
| Libération de `mo` | **`pico8_entity_shutdown` (`entity.c:107`) fait `pico8_instance_free(g_mo)`** : sur une struct, il libérerait le descripteur `static const` pris pour `fields` (**corruption**). Atteint par **chaque `pico8_jl`** (`pico8_entity_init` commence par `pico8_entity_shutdown`, et `g_mo` y est alors le `qb` du `jl()` précédent : `_init` puis chaque `eu()`), et par `pico8_bootstrap_shutdown` (`planet_kj_dispatch`, `test_pico8_nf`, `test_pico8_je`). Le `mo` provisoire (`entity.c:129-133`, instance sans classe) doit, lui, rester libéré comme aujourd'hui. |
| Clé dynamique | aucune. |
| `pico8_ref_audit.h` | aucune macro ne reçoit un `qb` ni `mo`. |
| Tests qui lisent `mo` / un `qb` | `test_pico8_planet_kj_dispatch` : `pico8_ob_table_set_name(mo->fields,"md",spy_md)` (**hors bornes**), `spy_md` caste `self` en `pico8_instance_t *` pour `pico8_qb_md` ; `pico8_bootstrap_shutdown` en fin (libération). Aussi compilé en variante `_refaudit` (`Makefile:628`). `test_pico8_geoharvest_burst` : `ga:pq` écrit `mo.ma` ; `live_ga` lit `e->cls` sans tag sur chaque entrée de `hv` (un `qb` y est toujours ; lecture dans les bornes, tombe sur `me` du `pico8_ke_t`, jamais égale à la classe `ga`, jamais déréférencée — déjà vrai pour `iv`/`mh`/`fq`/`nf`, réécrit en 1a.26). `test_pico8_je` : `qa` = `mo.ra` du provisoire (avant `jl`). `test_pico8_nf` : rejeu depuis le titre (`mo` réel à chaque frame). `test_pico8_entity_smoke.c:25,279` : commentaires seulement. Aucun test ne lit `->cls` d'un `qb`. |
| Couverture | `qb  38  15/15` (inchangé attendu). |

### Le bug que garde `planet_kj_dispatch`

Le test et le code qu'il garde sont arrivés ensemble dans l'import écrasé `b43e6c2` : l'historique ne conserve pas le correctif d'origine (`git log -S spy_md`, `-S '"md"'` : `b43e6c2`, `327c567` seulement ; aucune entrée dans `docs/trous-de-portage.md`). Ses propres commentaires nomment ce qu'il garde, qui fait foi : dans `dc("kj")`, le cas spécial `mo:md(ci[o])` **tourne vraiment** (sur le vrai `qb`, caméra posée sur `ra`) et **avant** la boucle par entité de la même couche (« Order, never the reverse »). Bug de référence pour la preuve rouge (§6) : le bloc `mo:md` de `pico8_dc` déplacé **après** la boucle par entité de la couche. Contre-épreuve, notée aussi : le bloc supprimé.

## Solution

1. **Struct `qb`** : premier membre `pico8_ke_t`, puis `v`, `ma`, `p`, `ra`, `pc` (`pico8_value_t`, noms minifiés conservés). Déclarée publique dans le module de défilement (comme `pico8_nf_t`).
2. **Valeurs initiales** : `ma` = fixe 0 ; `v`, `p`, `ra` = REF vers trois vecteurs statiques du module, `(0,0)`, `(192,192)`, `(192,192)` (deux vecteurs distincts pour `p` et `ra`), comme `g_nf_ra` — rien ne les modifie en place (mesuré ci-dessus) ; `pc` = REF vers la table des quatre directions, possédée par le module, construite par `pico8_qb_init` et libérée par `pico8_qb_shutdown` (même cycle de vie que les défauts de la classe aujourd'hui), jamais modifiée. Aucun état mutable partagé.
3. **Constructeur** (posé comme `struct_new` de la `pico8_class_t` de `qb`), dans l'ordre : alloue ; `pico8_ke_struct_init` avec le descripteur de `qb` ; valeurs initiales ; table reçue NULL ou vide seulement, toute clé → assertion porteuse de message (arrêt §7) ; enregistre `"qb"` au crochet de couverture ; `pico8_ke_struct_pq` (pas de `pq` propre).
4. **Enum de méthodes** : gagne `MD` (nom `"md"`). C'est le seul ajout.
5. **Descripteur de `qb`** : méthodes `PH` et `MD` ; lecteur de champ qui connaît `v`, `ma`, `p`, `ra`, `pc`, puis délègue à `pico8_ke_field` ; écrivain qui accepte `ma` et `v` seulement (les deux noms qu'un écrivain générique écrit sur `mo` : `ma()`, `ga:pq`, `bg:hq`) et rend 0 pour tout autre nom (l'assert de `pico8_entity_set_field` part).
6. **`qb:ph` et `qb:md` sur la struct** : `pico8_qb_ph`/`pico8_qb_md` prennent la struct. `ph` lit `pc`, `p`, `v` dans les membres, écrit `p`, `v`, `ra` (vecteurs neufs sur le tas, TROU-7) dans les membres, et appelle `pico8_ma(p, self)` (cast vers le type d'entité, comme pour `hv`). `md` lit le membre `ra`. Gardes NULL et adaptateurs (`fy or 1` résolu par l'adaptateur de `md`) conservés. Le helper `pico8_qb_field_vec` disparaît s'il devient inutilisé.
7. **`entity_geom.c`, coexistence** : les deux helpers statiques `pico8_entity_geom_field_vec` et `pico8_entity_geom_field_fixed` passent de `pico8_instance_get_field` à `pico8_entity_get_field` (résultat identique sur toute non-struct) ; l'écriture de `ma` dans `pico8_ma` passe de `pico8_ob_table_set_name(e->fields,…)` à `pico8_entity_set_field` (identique sur une instance). `gd`, `gk`, `na`, la lecture de `kq` : rien d'autre.
8. **Écrivains de `mo`** : `pico8_ga_pq` (`mo.ma=3`) et `pico8_bg_hq` (`mo.v=…`) passent à `pico8_entity_set_field(mo, …)`. Le reste de ces fonctions (écritures sur `self`, qui est un `ga`/`bg` encore instance) inchangé.
9. **`pico8_entity_shutdown`** : libère `g_mo` par `pico8_instance_free` **seulement s'il est tagué `INSTANCE`** (le `mo` provisoire) ; un `qb` struct est oublié, jamais libéré (TROU-7). Le `mo` provisoire de `pico8_entity_init` : inchangé.
10. **`pico8_jl`** : seul le type de la variable du `qb` construit change, et le cast vers le slot (comme `real_je`).
11. **`pico8_class_t` de `qb`** : même déclaration, même spec, même place, même nom de couverture ; reçoit le constructeur ; perd les deux installations `->defaults` (`ph`, `md`). Accesseur public du descripteur (comme `pico8_nf_struct_class()`).
12. **`planet_kj_dispatch`** (§6) : l'espion `md` n'est plus écrit dans `mo->fields` : le test pose sur le `qb` de `jl()` une copie locale du descripteur de `qb` dont l'entrée `MD` est `spy_md`, et rétablit le descripteur d'origine avant les arrêts. `spy_md` caste `self` vers la struct `qb` pour appeler le vrai `pico8_qb_md`. Tout le reste (`spy_kj` sur la planète, sentinelle, asserts `g_order_n == 2`, `'M'` puis `'K'`, caméra 192, pixel, `pico8_mo() == mo`) inchangé. Puis preuve rouge.
13. **Plan** : la ligne 1a.8 de `docs/plan-3.md` gagne « Découvert à 1a.8 : `mo` lu par `gd` (`entity_geom.c:54`), `ma()` partagée qb/jo (`entity_geom.c:72`), écritures `mo.ma` (`planet_family.c:173`, `ga:pq`) et `mo.v` (`bg.c:225`, `bg:hq`), libération de `mo` (`entity.c:107`) ». Aucun défaut muté à ajouter aux connus du §5.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `qb` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise de `qb` (construction, défauts, champs écrits, méthodes, `->defaults`, consommateurs génériques, lecteurs et écrivains de `mo`, libération, tests) soit mesurée dans le PRD, afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux que la struct `qb` n'ait que `pico8_ke_t`, `v`, `ma`, `p`, `ra`, `pc`, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun défaut de `qb` ne devienne un état statique mutable, afin de n'appliquer la règle des défauts mutés que là où le cart mute (il ne mute rien ici).
5. En tant que gardien du filet, je veux qu'un `qb` neuf lise `p`=`ra`=(192,192), `v`=(0,0), `ma`=0 et les quatre directions de `pc` dans l'ordre gauche, droite, haut, bas, afin que la caméra démarre au même endroit et réponde au D-pad dans le même sens.
6. En tant que gardien du filet, je veux que `p` et `ra` d'un `qb` neuf soient deux vecteurs distincts, afin qu'aucune écriture future sur l'un ne touche l'autre.
7. En tant que gardien du filet, je veux que `qb:ph` intègre `p += v`, `v += (ip - v)·0.2`, borne `p` à [-64, 320] et réaffecte `ra = ma(p, self)`, afin que le défilement au D-pad avec inertie soit identique au bit près.
8. En tant que gardien du filet, je veux que `ph` réaffecte `p`, `v`, `ra` à des vecteurs neufs, afin qu'aucun lecteur d'un ancien vecteur ne voie les nouvelles valeurs, comme aujourd'hui.
9. En tant que gardien du filet, je veux que `qb:md(fy)` pose la caméra à `ra·fy`, `fy` absent valant 1, afin que la parallaxe par couche de `dc("kj")` soit identique.
10. En tant que gardien du filet, je veux que `dc("kj")` trouve `md` par la table du descripteur via l'enum `MD`, afin que la caméra soit posée avant chaque couche non vide.
11. En tant que gardien du filet, je veux que `dg` trouve `ph` par la table, incrémente `t` dans `pico8_ke_t` et ne retire jamais un `qb`, afin que la caméra soit lue au même tick et vive jusqu'au `jl()` suivant.
12. En tant que gardien du filet, je veux que `lt` enregistre un `qb` dans `hv` seulement, afin que ni `cv` ni `cg` ne changent.
13. En tant que gardien du filet, je veux que `ma()` lise et écrive `ma` et lise `t` sur les deux formes, afin que la secousse de caméra et le tremblement des planètes décroissent de 0.1 par appel comme avant.
14. En tant que gardien du filet, je veux que `ga:pq` écrive `mo.ma = 3` dans le membre du `qb`, afin que l'éclat de géo-récolte secoue la caméra comme avant.
15. En tant que gardien du filet, je veux que `bg:hq` écrive `mo.v` dans le membre du `qb`, afin que le glisser au clic droit déplace la caméra comme avant.
16. En tant que gardien du filet, je veux que `gd` lise `mo.ra` par le lecteur à double forme, afin que le tri de visibilité des planètes, particules et voies ne fasse pas partir l'assert de la couche.
17. En tant que mainteneur, je veux que ces helpers partagés rendent un résultat identique sur les planètes, les particules, les voies, le `mo` provisoire et les `ob_table`, afin que le changement ne déplace rien de ce qui n'est pas converti.
18. En tant que mainteneur, je veux que l'écrivain de `qb` n'accepte que `ma` et `v`, afin qu'une écriture générique d'un autre nom sur la caméra déclenche l'arrêt du §7 au lieu d'un membre improvisé.
19. En tant que mainteneur, je veux qu'une clé reçue par le constructeur déclenche une assertion porteuse de message, afin qu'un champ jamais mesuré ne soit pas perdu en silence.
20. En tant que gardien de la mémoire, je veux que `pico8_entity_shutdown` ne passe jamais un `qb` struct à `pico8_instance_free`, afin que le second `jl()` (`eu()`), `pico8_bootstrap_shutdown` et tout arrêt ne libèrent pas un pointeur qui n'est pas un `fields`.
21. En tant que gardien de la mémoire, je veux que le `mo` provisoire reste libéré par `pico8_entity_shutdown` comme aujourd'hui, afin de ne rien changer à ce que 1a.9 convertira.
22. En tant que mainteneur de la mémoire, je veux qu'un `qb` struct ne soit jamais libéré, afin que TROU-7 reste la seule convention de fuite.
23. En tant qu'auteur de 1a.9, je veux que le `mo` provisoire reste une instance sans classe construite par `pico8_entity_init`, afin que sa conversion reste une étape à part.
24. En tant que mainteneur, je veux que le contrat de `pico8_mo()` dise qu'il peut rendre un `qb` struct, à lire et écrire par `pico8_entity_get_field`/`pico8_entity_set_field`, afin qu'aucun nouveau site ne réintroduise `->fields` sur `mo`.
25. En tant que mainteneur, je veux que la `pico8_class_t` de `qb` reste déclarée à la même place avec son nom de couverture et son parent, afin que `KNOWN[]`, l'assert de `cart.c` et le test de `bootstrap.c` tiennent.
26. En tant que gardien de la couverture, je veux que le constructeur enregistre `"qb"` une fois, jamais `"ke"`, afin que `qb  38  15/15` et `ke  0  0/15` restent identiques.
27. En tant que mainteneur, je veux que les deux installations `->defaults` (`ph`, `md`) disparaissent, remplacées par la table, afin d'appliquer la règle des accès directs aux défauts.
28. En tant qu'auteur des étapes suivantes, je veux que l'enum ne gagne que `MD`, afin que chaque ajout reste attaché à une étape.
29. En tant que gardien des tests de régression, je veux que `planet_kj_dispatch` pose son espion `md` sur le seul `qb` de `jl()`, par la table de méthodes, afin de garder l'observation de l'ordre `M` puis `K` sans écrire hors d'une struct.
30. En tant que gardien des tests de régression, je veux que `planet_kj_dispatch` échoue quand le bloc `mo:md` de `dc` est déplacé après la boucle par entité, puis repasse quand il est restauré, afin de prouver que la réécriture ne l'a pas vidé de son objet.
31. En tant que gardien des tests de régression, je veux que tous les autres tests de régression passent avec un diff vide (dont `geoharvest_burst`, qui atteint `ga:pq`), afin que la conversion ne se valide pas en affaiblissant un test.
32. En tant que gardien des tests, je veux que `test_pico8_nf`, `test_pico8_je` et `test_pico8_iv` passent sans changer une valeur figée, afin que les caractérisations des étapes précédentes gardent leur objet.
33. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
34. En tant que lecteur du plan, je veux que les sites découverts (`gd`, `ma()`, `ga:pq`, `bg:hq`, libération de `mo`) soient inscrits à la ligne 1a.8, afin que la liste des étapes dise ce que l'étape a réellement touché.
35. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés, afin de ne pas laisser de contrat mensonger.
36. En tant que vérificateur, je veux une caractérisation de la caméra figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit qu'indirectement (secousse, glisser, parallaxe).

## Implementation Decisions

Pas d'outil d'agent dans cette session pour lancer un agent `guideline` frais : j'ai chargé et appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Membres, pas d'état statique de classe.** Les cinq défauts de `qb` deviennent des membres : aucun n'est réaffecté sur la classe ni modifié en place (preuve au Problem Statement : `+` rebâtit, `na` n'atteint jamais `mo`). Si l'implémenteur trouve une modification en place d'un défaut de `qb` (un `->x =`/`->y =` sur le vecteur lu, un `pico8_ob_table_set_*` sur `pc`), c'est la règle des défauts modifiés en place qui s'applique (site découvert, état statique), pas un membre.
- **(G) Stockage des valeurs initiales** : vecteurs statiques du module pour `v`/`p`/`ra`, précédent `g_nf_ra`/`g_iv_v` ; `pc` en table possédée par le module, construite/libérée avec la classe. Rejeté : aller chercher les REF dans `g_qb_class->defaults` au constructeur — c'est un accès `->defaults` que le §5 fait migrer, et la classe retenue n'a pas de descendant qui en aurait besoin. La `pico8_class_t` garde sa spec telle quelle (copie en lecture, jamais lue).
- **Enum** : `MD` ajouté, parce que `dc` appelle `md` par nom de chaîne (règle « Méthodes »). Il ne touche aucun autre descripteur (entrée NULL, le nom retombe sur le lecteur de champ, qui rend absent ; `md` n'est pas un nom `bm`, donc `lt`/`he` inchangés).
- **Écrivain `ma`/`v` seulement** (G) : ce sont les seuls noms qu'un écrivain générique pose sur `mo` (mesuré : `ma()`, `ga:pq`, `bg:hq`). `ph` écrit ses membres directement ; `dg` écrit `t` directement. Rejeté : un écrivain qui accepte tous les membres — du code qu'aucun appelant n'atteint.
- **`ma()` partagée** : changement d'appel seulement dans `entity_geom.c` (deux helpers de lecture, une écriture). Ces helpers servent aussi `gk`, `gd` sur toutes les entités : délégation identique sur toute non-struct. La lecture de `kq` dans `gk` et `na` restent sur `pico8_instance_get_field` : aucun `qb` ne les atteint (G, changement chirurgical).
- **`ga:pq`, `bg:hq`** : seule l'écriture sur `mo` change. `ga`, `bg` restent des instances ; leurs propres écritures sur `self` ne bougent pas.
- **Libération conditionnelle** dans `pico8_entity_shutdown`, exact analogue de celle de `pico8_ui_shutdown` en 1a.6. Conséquence voulue : le `qb` d'un `jl()` précédent, libéré aujourd'hui par le `pico8_entity_init` du `jl()` suivant, est désormais oublié (TROU-7). Rien ne le lit après : `pico8_entity_set_mo` le remplace dans le même `jl()`.
- **Type du slot** : `g_mo` et `pico8_mo()` restent `pico8_instance_t *` (décision de 1a.6 pour `je`) ; `pico8_entity_set_mo` inchangé ; le contrat d'en-tête dit qu'il peut tenir un `qb` struct. Le type disparaît avec la sortie de phase.
- **`planet_kj_dispatch`, réécriture** (G) : descripteur copié dans une variable du test, `MD` remplacé par `spy_md`, posé sur `mo` seul, rétabli avant `pico8_jo_shutdown`/`pico8_bootstrap_shutdown`. C'est l'équivalent exact de l'ancien « champ d'instance qui masque la classe, sur `mo` seul » : l'intention (ordre `M` puis `K`, exactement un de chaque, `md` réel sur le vrai `qb`, caméra 192, pixel) est gardée à l'identique. Rejeté : supprimer `spy_md` et observer la caméra depuis `spy_kj` — perd le compte d'appels `md` (`g_order_n == 2`), donc change l'intention (arrêt §7).
- **Preuve rouge** (§6) : le correctif d'origine n'est pas dans l'historique (import écrasé `b43e6c2`) ; le bug de référence est celui que les commentaires du test nomment, l'ordre inversé dans `dc`. Le commit note la commande, le mode d'échec observé (attendu : `g_order[0] == 'M'` faux), et la contre-épreuve (bloc supprimé : `g_order_n == 2` faux).
- **`test_pico8_entity_smoke`** (G) : commentaires `:25` et `:279` laissés (toujours vrais du test factice ; fichier supprimé en 1a.42).
- **`live_ga` de `geoharvest_burst`** (G) : laissé tel quel ; il voit un `qb` struct à chaque tick, lecture dans les bornes, jamais égale, jamais déréférencée ; réécriture en 1a.26.
- **Commentaires** (CLAUDE.md) : contrat de la struct (`qb` jamais libéré, TROU-7 ; valeurs initiales partagées jamais modifiées en place), contrat de `pico8_mo()`/`pico8_entity_shutdown()`/`pico8_entity_init()` dans `entity.h` (« frees the instance pico8_mo() currently holds » devient faux), assert porteur (clé reçue). Corriger ceux que le changement rend faux : en-tête de `scroll.h` (`pico8_qb_new` rend une instance ; « `overrides` is copied »), `bootstrap.h` (« Frees the `mo`/`ok` instances » : `mo` n'est plus libéré après `jl()`), commentaire de `spy_md` (« Installed as an instance field on `mo` alone »). Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts ; `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation de `qb` sur l'ancien code.** Nouveau test (voir Testing), valeurs figées **avant** toute modification de `scroll.c`, `entity.c`, `entity_geom.c`, `planet_family.c`, `bg.c`, `bootstrap.c`.
2. **Coexistence, sans classe convertie.** Helpers de `entity_geom.c` et écriture de `ma()` à double forme ; écrivains de `ga:pq` et `bg:hq` par `pico8_entity_set_field` ; `pico8_entity_shutdown` ne libère `g_mo` que tagué `INSTANCE`. Aucune classe ne change de forme : filet et couverture trivialement identiques.
3. **`qb` convertie + `planet_kj_dispatch` réécrit.** Struct, valeurs initiales, enum `MD`, descripteur, constructeur, `ph`/`md` sur la struct, classe reliée, `->defaults` retirés, accesseur ; `bootstrap.c` ; `planet_kj_dispatch` ; ligne 1a.8 du plan. La caractérisation passe **sans changer ses valeurs figées**. Puis preuve rouge, correctif restauré.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rend `pico8_entity_get_field` sur `pico8_mo()`, ce que rend `pico8_mv`, la caméra de `gfx` après `md`, appartenance à `hv`/`cv`/`cg`, pixels, compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_qb`** (dans `make unit`, lié comme `test_pico8_nf`, sur le cart réel chargé). Il lit **uniquement** par l'API à double forme (`pico8_entity_get_field`, `pico8_entity_set_field`, `pico8_mv`, `pico8_hv`/`pico8_cv`, `pico8_mo`, `pico8_dg`, `pico8_dc`) et pilote le D-pad par un `pico8_input_t` construit en mémoire ou chargé, pour être le même code avant et après.
  - **Caractérisation, figée sur l'ancien code** :
    1. Après `pico8_jl()` : `mo` dans `hv`, dans aucun `cv` ni `cg` ; `p` = `ra` = (192,192), `v` = (0,0), `ma` = 0, `me` = "ph", `t` = 0 ; `pc` rend une table de 4 vecteurs (-1,0), (1,0), (0,-1), (0,1) ; `p` et `ra` sont deux REF distinctes.
    2. `md` : `pico8_mv(mo,"md",…)` avec `fy` = 1, 0.5, 0 et absent (REF NULL) : caméra (192,192), (96,96), (0,0), (192,192).
    3. Séquence scriptée de ticks `dg` (D-pad : aucun, gauche, droite+haut, bas tenu jusqu'à la borne 320, puis relâché) : par tick `p`, `v`, `ra`, `ma`, `t`, repliés en une empreinte + nombre de ticks, figés ; `p` borné observé.
    4. Secousse : `pico8_entity_set_field(mo,"ma",3)` puis ticks jusqu'à `ma` = 0 : par tick `ma` et `ra - p` (l'offset `qm`), empreinte figée ; nombre de ticks jusqu'à 0 figé.
    5. Second `pico8_jl()` (chemin de `eu()`) : pas de crash, nouveau `mo` distinct lisible comme en 1, l'ancien absent de `hv` ; puis `pico8_bootstrap_shutdown()`, puis `pico8_jl()` de nouveau : pas de crash.
    6. Le `mo` provisoire : après `pico8_entity_init()` seul, `pico8_mv(mo,"md",…)` rend 0 et la caméra ne bouge pas ; `ra`, `p`, `ma` absents.
    7. **Rejeu depuis le titre** de `tests/fixtures/long_session_events.json` (dans `RECORDINGS` et `FRAMES_EVENTS` ; premier D-pad frame 1002, premier clic droit frame 4741), piloté comme les tests de scénario (`--from-title`), sur les N premières frames avec N ≥ 4800 (couvre D-pad et glisser `bg:hq`) : par frame `p`, `v`, `ra`, `ma` de `mo` et caméra, repliés en empreinte de séquence + nombre de frames, figés.
    8. **Rejeu de `geoharvest_burst_events.json`** (comme `test_pico8_geoharvest_burst`) : à la frame où `ga` apparaît, `mo.ma` = 3 ; puis décroissance ; empreinte de `ma` sur la fenêtre, figée.
  - **Forme** (ajouté au morceau 3) : `pico8_mo()` après `jl()` est tagué `STRUCT` avec le descripteur de `qb` (accesseur) ; `pico8_class_instantiate(pico8_qb_class(), NULL)` rend la même forme, présente dans `hv`, absente de tout `cv` ; `pico8_entity_set_field(mo,"p",…)` asserte (vérifié dans un processus enfant, ou omis si le harnais ne le permet pas — noté dans le commit).
  - **Mutations** : l'implémenteur remet localement au moins 8 erreurs plausibles et note dans le commit celles qui sont attrapées : `MD` absent de la table ; `PH` absent ; `pc` dans le désordre ; `p` non borné ; `ra` = `p` sans `ma()` ; `ma()` qui n'écrit plus `ma` ; écrivain qui ignore `v` (glisser sans effet) ; `ga:pq` qui n'écrit plus ; `p` et `ra` aliasés sur le même vecteur statique ; `fy` absent traité comme 0 ; `pico8_ke_struct_pq` omis ; `pico8_entity_shutdown` qui libère la struct (abandon glibc). Mutant équivalent éventuel justifié dans le commit.
- **`planet_kj_dispatch`** : diff limité à l'installation/rétablissement de l'espion, au cast de `spy_md`, à son commentaire et aux includes. **Preuve rouge**, sur le code final : déplacer localement le bloc `mo:md` de `pico8_dc` après la boucle par entité → `tmp/test_pico8_planet_kj_dispatch` rouge ; restaurer → vert ; contre-épreuve bloc supprimé → rouge. La variante `_refaudit` compile et passe. Si le bug remis ne rend **pas** le test rouge, c'est un arrêt (§7), pas un test durci.
- **Autres tests de régression** : tous passent avec un diff vide ; `geoharvest_burst` (écriture `mo.ma` de `ga:pq`), `slipway_drag`, `playability`, `empty_planet_cleared`, `rich_planet_exports` (`gd` sur chaque planète dessinée).
- **`test_pico8_nf`, `test_pico8_je`, `test_pico8_iv`** : inchangés, valeurs figées identiques.
- **Couverture** : `make class-coverage-net` code 0, sortie identique à la référence (`qb  38  15/15`, `ke  0  0/15  exemptée …`).
- **Prior art** : `test_pico8_nf` (rejeu depuis le titre, séquence de ticks, forme ajoutée), `test_pico8_fq` + `score_panel_close` en 1a.5 (réécriture et preuve rouge), `test_pico8_geoharvest_burst` (rejeu non-titre), `test_pico8_replay_from_title`.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `qb` n'est pas `qb  38  15/15`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé de `qb` ne tombe sous aucune règle du §5 — notamment si l'assertion des clés reçues ou celle de l'écrivain générique se déclenche sur un rejeu ou un test, ou si une écriture générique d'un autre nom que `ma`/`v` sur `mo` est trouvée : arrêt, pas de membre improvisé.
- `FAILED:` si un défaut de `qb` devient un état statique mutable sans qu'une modification en place ait été trouvée et nommée à la ligne 1a.8 (ou, inversement, si une modification en place trouvée est traitée en membre).
- `FAILED:` si `planet_kj_dispatch` change d'intention pour passer (asserts sur l'ordre, le compte, la caméra, le pixel ou `pico8_mo() == mo` modifiés ou retirés, `spy_md` supprimé), si son diff dépasse l'installation de l'espion, ou si le commit ne rapporte pas la preuve rouge (bloc `mo:md` déplacé après la boucle, test rouge, correctif restauré, vert).
- `FAILED:` si un autre test de régression de bug (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe que `qb` est convertie ou change de forme — en particulier si le `mo` provisoire (1a.9) cesse d'être une instance sans classe construite par `pico8_entity_init`, ou si `ga`, `bg`, `jo`, `ui`, `nf`, le `je` provisoire, `ke`, `jb`, `iv`, `mh`, `fq` changent de forme, ou si un `->defaults` d'une autre classe bouge.
- `FAILED:` si la `pico8_class_t` de `qb` n'est plus déclarée à la même place avec sa spec, son parent et son nom de couverture.
- `FAILED:` si les valeurs figées de la caractérisation de `qb`, de `test_pico8_nf`, `test_pico8_je` ou `test_pico8_iv` ont été changées après la conversion.

Critères de réussite (en plus) :

- grep : plus aucun `g_qb_class->defaults` ; plus aucun `mo->fields` ni `pico8_mo()->fields` dans `src/` et `tests/` ; plus aucun `pico8_instance_get_field` dans les deux helpers statiques de `entity_geom.c` ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD` ;
- la struct `qb` n'a que `pico8_ke_t`, `v`, `ma`, `p`, `ra`, `pc` ; son descripteur n'a que `PH` et `MD` ; son écrivain n'accepte que `ma` et `v` ;
- hors `scroll.c`/`scroll.h`/`pico8_struct.h`, le diff de `src/` se limite à : les deux helpers et l'écriture de `ma()` dans `entity_geom.c` ; l'écriture sur `mo` dans `pico8_ga_pq` et dans `pico8_bg_hq` ; la libération conditionnelle dans `pico8_entity_shutdown` et les contrats d'`entity.h` ; le type du `qb` construit dans `bootstrap.c` et le contrat de `bootstrap.h` ;
- la ligne 1a.8 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.8 ».

## Out of Scope

- Le `mo` provisoire sans classe (1a.9) : sa construction dans `pico8_entity_init`, sa forme, sa libération, son remplacement ; `planet_kj_dispatch` y sera relu.
- `ga` (1a.26) et `bg` (1a.12) eux-mêmes, dont seules les écritures sur `mo` changent ; `jo` (1a.17), sur lequel `ma()` et `gd` continuent d'opérer comme instance.
- `gk`/`kq`, `na` et tout autre helper de `entity_geom.c` que `qb` n'atteint pas.
- Le typage C des membres (`pico8_vec_t`, fixe) : phase 1b ; le remplacement de la table `pc` par un tableau fixe : phase D.
- La réécriture de `live_ga` dans `geoharvest_burst` (1a.26) et de `test_pico8_entity_smoke` (1a.42).
- Libérer les `qb` remplacés par `jl()` (TROU-7, Plan 2 §8).
- Lancer l'appli raylib : jamais.

## Further Notes

- Sans la libération conditionnelle, **chaque rejeu qui lance une partie** (`_init` puis `eu()` → second `jl()`) corromprait le tas au `pico8_entity_init` : c'est le premier point à faire passer au morceau 2.
- Sans le passage de `gd` au lecteur à double forme, le premier dessin d'une planète après la conversion fait partir l'assert de `pico8_instance_get_field` : le filet entier serait rouge.
- La secousse (`ma`) et le glisser (`v`) ne changent l'écran que par la caméra : le filet les voit, mais seulement sur les frames où ils agissent (géo-récolte, clic droit de `long_session`). D'où les scénarios 4, 7 et 8.
- Mesures faites par grep sur `src/` et `tests/` à ec01e86, en lisant `pico8/lua.lua:153-155`, `:194`, `:218`, `:346-350`, `:381`, `:577`, `:945`, `:1079`, `:1128-1142`, et `pico8_class.c` (`pico8_instance_free`, `pico8_instance_get_field`, délégation `struct_new`), `pico8_entity.c`, `pico8_entity_geom.c`, `pico8_bootstrap.c`, `pico8_cart.c`, les fixtures d'événements (clic droit et D-pad comptés par frame).
