---
title: "Plan 3, étape 1a.24 : `nr` (le slipgate) devient une struct fille de `qh` avec 2 membres propres (`hz`, `rr`) et ses quatre méthodes `gv`/`ldown`/`hb`/`cn` dans son descripteur ; `fr`, encore instance, les atteint toujours par `nr->defaults`"
description: "Le slipgate `nr` est encore une instance de la couche objet, construit 6 fois sur le filet (5 dans `session_events`, 1 dans `trading_hub_session_events`) par la seule voie `ie:ldown` sur `gx[2]`, avec la clé `id`. Il passe en struct `pico8_nr_t` (`pico8_qh_t` puis `hz`, `rr`), son descripteur reprend les 15 entrées de `qh` avec `GV`/`LDOWN`/`HB`/`CN` = ses propres dispatchers. Comme `fr` (1a.25) descend de `nr` et atteint ces quatre fonctions par `nr->defaults`, elles restent à double forme et installées ; `pico8_qh_struct_init` devient publique."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.24 — `nr` : `qh` (`planet_family_trade.c:312`), `ldown`, `gx[2]`.

(À HEAD a88fe1f, la déclaration `g_nr_class = pico8_class_declare(pico8_qh_class(), …)` est à `planet_family_trade.c:412`, son nom de couverture à `:421`, les quatre installations `gv`/`ldown`/`hb`/`cn` dans `g_nr_class->defaults` à `:422-425`, `pico8_nr_gv` à `:52` (dispatcher `:74`), `pico8_nr_ldown` à `:84` (`:118`), `pico8_nr_hb` à `:125` (`:150`), `pico8_nr_cn` à `:158` (`:217`), `pico8_nr_new` à `:478`. La construction est `ie:ldown` → `pico8_class_instantiate(gx[2], {id})`, `research_icon.c:199`, avec `gx[2] = pico8_nr_class()` posé à `cart.c:225`. Côté `qh` : `pico8_qh_struct_init` (`static`) à `planet_family.c:168`, lecteur `pico8_qh_field` `:110` et écrivain `pico8_qh_set_field` `:125`, tous deux `static`. La ligne ne nomme aucun test de régression.)

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. […]
>
> **Clés construites à l'exécution** : […] `ldown`/`rdown` côté méthodes.
>
> **Champ d'un écrivain générique** : […] seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct […]. Les descendants non convertis trouvent leurs champs hérités en remontant `parent`, qui reste valide.
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre (dispatcher hérité dans `defaults` […]) reste à double forme […]. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante.
>
> **Accès directs aux défauts** : […] la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct : une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec.

`pico8_nr_t` commence par `pico8_qh_t`, qui commence par `pico8_jo_t`, `pico8_ke_t`. `nr` a **une descendante non convertie, `fr`** (1a.25, même fichier) : c'est la différence décisive avec `mx` (1a.23), qui était une feuille.

Note « découvert à 1a.19 » du plan, qui échoit ici : « l'initialisation d'un `pico8_qh_t` (`pico8_qh_struct_init`) est coupée du constructeur de `qh`, `static` tant que les descendantes de `qh` vivent dans le même fichier (publique à la première hors du fichier, `nr`, 1a.24) ».

§6 : aucun test de régression nommé ; aucun fichier de régression ne construit un `nr` (mesuré). Les caractérisations qui lisent un `nr` (`test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`) changent leur façon de le reconnaître, jamais une valeur figée ; `test_pico8_mz` et `test_pico8_bg` appellent `pico8_nr_new` et ne changent que le type de la variable qui le reçoit. §7 : un commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (`nr  6  2/15`, `fr  5  1/15`, `qh  8  3/15`, toutes les autres lignes inchangées) ; **preuve des tests par `gcov` et 2 à 3 mutations** ; un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.24, avec « découvert à 1a.24 » dans le message ; un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.23 ont posé (jusqu'à af03e84), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`). L'enum contient déjà `GV`, `LDOWN`, `HB`, `CN` (et `OL`, `PH`, `KJ`, `EV`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `GG`, `HK`, `KF`, `IB`, `LL`, `LY`, `MM`). `pico8_entity_get_field` sur une struct rend d'abord l'entrée du descripteur quand le nom est une méthode de l'enum (d'où `gv` lu comme fonction à `slipway.c:110`), sinon le lecteur ; un membre nil est absent ; l'écrivain rend 0 pour un nom sans membre et `pico8_entity_set_field` asserte alors.
- `pico8_jo_t` (31 membres, dont `kq`, `oe`, `kw`, `mi`, `id`, `p`, `ra`), les 11 dispatchers publics de `jo`, `pico8_jo_pq`, `pico8_ke_struct_pq`.
- `pico8_qh_t` (10 membres : `hp`, `px`, `dq`, `qh`, `od`, `fp`, `dw`, `s`, `ba`, `et`), `g_qh_defaults` (13), lecteur/écrivain de `qh` (**20 noms** : les 17 de `jo` plus `et`, `ra`, `p`), `static`, joignables par `pico8_qh_struct_class()->field`/`->set_field` ; descripteur de `qh` aux 15 entrées ; `pico8_qh_ly_dispatch`, `pico8_bv_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch` publics.
- **1a.21** (`hj`) : précédent d'une parente convertie dont les méthodes propres restent installées dans `->defaults` et à double forme pour une descendante non convertie (`pq`/`mm`/`kj` gardés pour `mx`, retirés à 1a.23).
- **1a.22** (`pm`) : précédent des clés de construction `p`/`ra`/`id` chacune facultative, rangées dans les membres de `jo`.
- **1a.23** (`mx`, af03e84) : première fille hors du fichier de sa parente (`pico8_hj_struct_init` rendue publique, lecteur/écrivain chaînés par `pico8_hj_struct_class()->field/set_field`), preuve `gcov` avec `__gcov_dump()` avant les `_exit(0)` des fils dans la copie jetable ; **`ke:pq` avant `jo:pq` est un mutant équivalent** (`lt` ne lit que les 5 noms de `bm` et `nu`) — la mutation d'ordre porteuse est « défauts propres copiés avant l'init de la parente ».

### Ce que fait le cart

`pico8/lua.lua` :

```
:1101-1108 nr=qh:jr([[ nu=o("nr","np","prober"), od=10,fp=1, kq=b(-4,-4,5,5),
             s=170, hz=o(170,170,170,168), dw=0.01, rr="slipgates", ]])
:1109-1111 function nr:gv(s) return s.qu~=self and #self.oe<3 end
:1112-1113 function nr:ldown() return self:gv({})and jc({qu=self})end
:1114-1115 function nr:hb()self.s=self.hz[#self.oe+1] end
:1116-1122 function nr:cn(lw) … for w in all(self.oe) … w:eb(self):cn(gf) … end
:1123-1127 fr=nr:jr([[ mt=1, rr="infraspace", od=25, s=78,hz=o(78,78,78,76), ]])
:1182      local gx={qh,os,nr,fr,nl,mx,hj}   -- nr = gx[2] en 0-indexé côté C
```

Points décisifs :

- **Une seule construction** sur le filet : `ie:ldown` sur `gx[2]`, clé **`id`** seule. `nr` n'a pas de `fz` : ni `kv[…]`, ni `lookup_named`.
- **Walk `pq`** : ni `nr` ni `qh` n'ont de `pq` ; `jo.pq` (`oe`, `hn`, `ep`, `jy`, `kw`), puis `ke.pq` (`lt`). Même ordre que `qh`.
- **Quatre méthodes propres**, toutes dans l'enum, toutes des surcharges d'une méthode de `jo` : `gv`, `ldown` (nommée par la ligne), `hb`, `cn`. `fr` n'en surcharge aucune : **une instance `fr` les atteint toutes les quatre par `nr->defaults`**.
- **`nr:hb` écrit `self.s`** sur l'instance : `s` a déjà un membre chez `qh` ; seule écriture d'un nom hors des 20 de l'écrivain de `qh`.
- **`hz`** et **`rr`** n'ont de membre chez aucun ancêtre (`rr` est membre de `nl` et de `hj`, des sœurs) → membres propres de `nr`, que `fr` réutilisera (ses défauts `rr`, `hz` y tomberont à 1a.25).
- **Hérités sans surcharge** : de `qh`, `hp`=1, `px` (la table « laboratory » de `qh`), `me`="kz", `dq`="ly", `hf`=faux, `qh`=1, `mi`=1, `ba` ; de `jo`, `hs`, `fa`, `ma`, `ka`, `dz`, `cq`, `kw`, `gq`, `ca`, `em` ; de `ke`, `t`.
- **`nu` de `nr` ne contient ni `"jo"` ni `"qh"`** : un `nr` entre dans `cg.nr`/`np`/`prober` seulement (`cg("nr")` n'a aucun lecteur dans `src/`).
- **Tables de défaut de `nr`** (`nu`, `kq`, `hz`) : jamais écrites en place ni réaffectées (grep du cart). Aucun défaut muté.

### Emprise réelle de `nr` (mesurée sur a88fe1f)

**Méthode.** Grep de `src/`, `tests/`, `tools/`, `Makefile`, lecture des fonctions, puis **sonde** sur une copie `git archive HEAD` jetable du scratchpad (supprimée après mesure) : `pico8_entity_get_field`/`set_field`, `pico8_mv`, `pico8_instance_get_field`, `pico8_instance_fields`, `pico8_class_instantiate` deviennent, hors de `pico8_entity.c`/`pico8_class.c`, des macros (`-include`) qui journalisent (opération, nom, fichier:ligne, absent/nil/présent ; clés de construction) chaque accès à une instance dont la classe exacte est `pico8_nr_class()` ou `pico8_fr_class()`. Binaire de couverture sondé relancé sur les 15 rejeux du filet un par un (`--from-title` pour les 4 `RECORDINGS`), tous en sortie 0.

| Aspect | Mesure |
|---|---|
| Constructions d'un `nr` | **une seule** voie : `ie:ldown` (`research_icon.c:199`), clé **`id`** seule. **6 sur le filet : 5 dans `session_events`, 1 dans `trading_hub_session_events`** (tous deux `--from-title`) ; 0 dans les 13 autres. Hors jeu : `test_pico8_mz` (`:724`) et `test_pico8_bg` (`:505`) appellent `pico8_nr_new` avec les clés **`p`, `ra`** puis `pico8_nr_gv`. `fr` : 5, toutes dans `session_events`, clé `id`. |
| Défauts propres de `nr` (8) | `nu`, `od`=10, `fp`=1, `kq`=`b(-4,-4,5,5)`, `s`=170, **`hz`**=`o(170,170,170,168)`, `dw`=0.01, **`rr`**=`"slipgates"`. |
| Défauts qui tombent sur un membre hérité (6) | `nu`, `kq` (`pico8_jo_t`) ; `od`, `fp`, `s`, `dw` (`pico8_qh_t`). **`hz` et `rr` n'ont de membre chez aucun ancêtre.** |
| Défauts égaux à ceux de `qh`/`jo` | aucun : `od` 10/15, `fp` 1/2, `s` 170/174, `dw` 0.01/0.025, `kq` différent de celui de `jo`, `nu` différent. Tout « défaut non appliqué » est un mutant non équivalent. |
| Défauts mutés | aucun. |
| Champs écrits sur un `nr` (sonde) | 14 noms : `oe` (`planet.c:401`), `hn`, `ep` (nil, `:442`), `jy`, `kw` (`planet.c:441-458`), `ma` (`planet.c:481` ; `entity_geom.c:79` 28 717 fois), `fx` (`planet_trade.c:94`), `m_` (`ui.c:308,313`), `et`/`ra`/`p` (`probe.c:179,132,138` : 518 sondages), et **`s`** (`planet_family_trade.c:147`, `nr:hb`, 422 fois). **Tous parmi les 20 noms de `qh` sauf `s`**, écrit par la méthode propre `hb` dans le membre `s` de `qh`. Écriture nil : `ep`. Sur `fr` (même mesure, pour 1a.25) : les mêmes plus `fo` (REF `probe.c:185`, nil `planet_display.c:572`). |
| Membres propres qui en résultent (2) | après `pico8_qh_t` : **`hz`**, **`rr`** (ordre de la spec). |
| Noms lus sur un `nr` (sonde) | présents : `me`, `t`, `id`, `ra`, `p`, `et`, `oe` (dont `planet_family_trade.c:67,135,170`), `fa`, `kq`, `ma`, `hf`, `hp`, `dq`, `fp`, `dw`, `od`, `s`, **`hz`** (`planet_family_trade.c:139`, 422), `fx`, et `gv` (la fonction, `slipway.c:110`). Absents (aujourd'hui comme demain) : `fg`, `ql`, `fo`, `lq`, `nk`, `qw`, `kr`, `ik`, `p_`, **`mt`** (`slipway.c:45`, 652 : membre propre de `fr` à 1a.25, jamais de `nr`), **`qu`** (`planet_family_trade.c:63`, 5 : `nr:gv(s)` appelé avec `s` un `nr`), `fx` avant sa première écriture. **Jamais lus par l'API sur le filet** : `rr`, `px`, `qh`, `ba`, `mi`, `hs`, `ka`, `dz`, `cq`, `gq`, `ca`, `em`, `kw` (et `nu`, lu par `lt` dans `pico8_entity.c`). |
| Méthodes appelées par nom sur un `nr` | `hd` (`ui.c:296`), `ly` (`bootstrap.c:89`, 283 350) et `kz` (même site, 512, **absent** — absent aussi sur la struct), `hq` (`ui.c:360`), `ey` (`ui.c:373`), **`gv`** et **`ldown`** (`ui.c:411`, 9 chacune), **`hb`** (`economy.c:277`, 422), `hk` (`economy.c:156`), **`cn`** (`planet_trade.c:305`, 538 ; et `planet_family_trade.c:209`, 35, relais d'un `nr` à l'autre), `kf` (`slipway.c:177-178`, 13) ; `kj`/`ev` par `dc`. **Toutes dans l'enum.** `fr` : mêmes méthodes, mêmes sites (`gv` 11, `ldown` 8, `hb` 175, `cn` 230). |
| Lectures à simple forme | **aucune** : zéro `pico8_instance_get_field` sur un `nr` ; `pico8_instance_fields` seulement aux sites déjà aiguillés (`bootstrap.c:101`, `ke.c:45-46`, `probe.c:128`). Dans `pico8_nr_cn`, `pico8_instance_get_field(t, "rl")` lit le `trade`, pas le `nr`. |
| `->defaults` / classe de `nr` | tous dans `pico8_planet_family_trade_init` : déclaration, `PICO8_CLASS_COVERAGE_NAME` (`:421`), quatre installations (`:422-425`), et la déclaration de `fr` qui prend `g_nr_class` pour parente (`:427`). `cart.c:87` n'asserte que `parent`. `pico8_class_instantiate` ne délègue qu'au `struct_new` de la classe exacte (`class.c:265`) : poser celui de `nr` ne touche pas `fr`. |
| Fonctions de `nr` et leurs appelants | les quatre `pico8_nr_*` : appelées seulement par leurs dispatchers `static` (et `pico8_nr_gv` par `pico8_nr_ldown` et par `test_pico8_mz`/`test_pico8_bg`). Atteintes aujourd'hui par `nr->defaults` pour un `nr` **et pour un `fr`**. **Après ce commit, `fr` les atteint encore → les quatre restent à double forme** (déjà écrites en `pico8_entity_get_field`/`set_field` et `pico8_mv`), et leurs installations restent dans `nr->defaults` jusqu'à 1a.25. |
| Tests qui lisent un `nr` | **`test_pico8_qh`** : `descendant_of` (`:501`) ne reconnaît `nr` qu'en instance → il disparaîtrait des rejeux `session`/`hub` ; boucle finale de `test_qh_is_a_struct` (`:779`, `:854`) attend `nr` `INSTANCE`. **`test_pico8_jo`** : `daughter_of` (`:420-460`), rejeux `session`/`hub`. **`test_pico8_nl`** (`:593`), **`test_pico8_os`** (`:620`), **`test_pico8_hj`** (`:734`), **`test_pico8_pm`** (`:697`), **`test_pico8_mx`** (`:754`) : `nr` dans `others`, attendu `INSTANCE`. **`test_pico8_mz`** (`:710-727`) et **`test_pico8_bg`** (`:495-508`) : `pico8_instance_t *nr = pico8_nr_new(overrides)` avec `p`/`ra`, puis `pico8_nr_gv(nr, …)`. **`test_pico8_ie`** : `gx()` compare des classes, `gx_rr` lit la classe, ses asserts de `ldown` tolèrent une struct (`:558`, `:564`) → inchangé. **`test_pico8_jc`** : rejoue `trading_hub`, ne teste pas la forme d'un `qu` → inchangé. |
| Filet | 2 rejeux construisent un `nr` : `session_events` (5 `nr`, 5 `fr` ; 116 045 frames) et `trading_hub_session_events` (1 `nr`). Ils les sondent, les relient (`ldown`, `kf`), leur font changer de sprite (`hb` → `s`), y font transiter du commerce (`cn` relayé). |
| Durée | `session_events` fait 116 045 frames (`test_pico8_qh`, qui le rejoue déjà, est dans `UNIT_TESTS_SLOW`) : une caractérisation qui le rejoue à `-O0` avec empreinte d'écran dépasse 30 s → `UNIT_TESTS_SLOW`. |

## Solution

1. **Enum : inchangée.** `gv`, `ldown`, `hb`, `cn` y sont déjà.
2. **Struct `nr`**, publique dans `planet_family_trade.h` : `pico8_qh_t` en premier membre, puis **`hz`**, **`rr`**. Aucun membre ne répète un nom de `qh`, `jo` ou `ke`.
3. **Visibilité élargie, sans changement de corps** : `pico8_qh_struct_init` devient publique (déclarée dans `planet_family.h`, son commentaire de contrat déplacé avec elle). `pico8_qh_field`, `pico8_qh_set_field` restent `static` : `nr` les joint par `pico8_qh_struct_class()->field`/`->set_field`, comme `mx` joint ceux de `hj`. Les quatre dispatchers de `nr` restent `static` (`fr`, qui les nommera à 1a.25, vit dans le même fichier).
4. **Défauts pris par REF, jamais re-parsés** : `pico8_planet_family_trade_init` déclare `nr` comme aujourd'hui (même spec, même parent, même nom de couverture), **garde les quatre installations** `gv`/`ldown`/`hb`/`cn` dans `g_nr_class->defaults` (pour `fr`), pose `pico8_class_set_struct_new(g_nr_class, …)`, puis lit **une fois** les 8 défauts de `nr` dans sa classe (`pico8_class_get_field`, assert « chaque défaut est une clé de la spec de `nr` ») vers un état statique `g_nr_defaults`, chaque entrée à l'offset de son membre : `nu`, `kq` → membres de `jo` ; `od`, `fp`, `s`, `dw` → membres de `qh` ; `hz`, `rr` → membres propres. `pico8_planet_family_trade_shutdown` remet cet état à nil avant de libérer la classe, sans libérer aucune table.
5. **Constructeur de `nr`**, dans l'ordre du walk du cart :
   1. alloue un `pico8_nr_t` ;
   2. `pico8_qh_struct_init(&self->qh, &g_nr_struct_class)` ;
   3. met `hz`, `rr` à nil, puis copie les 8 défauts statiques de `nr` (ils écrasent les membres hérités) ;
   4. recopie les clés **`id`**, **`p`**, **`ra`**, chacune facultative, dans les membres de `jo`, et **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise ; ne garde aucun pointeur sur la table ;
   5. enregistre `"nr"` au crochet de couverture ;
   6. `pico8_jo_pq`, puis `pico8_ke_struct_pq`.
6. **Descripteur de `nr`** (`static const`) :
   - 15 entrées : **les mêmes pointeurs** que le descripteur de `qh` pour `HD`, `KJ`, `GG`, `HK`, `KF`, `IB`, `LL`, `HQ`, `EV`, `EY`, `LY` ; **`GV`, `LDOWN`, `HB`, `CN` = les dispatchers de `nr`**, les mêmes pointeurs que ceux installés dans `nr->defaults` ; `OL`, `PH`, `MD`, `KZ`, `PR`, `RDOWN`, `EB`, `MM` NULL ;
   - **lecteur chaîné** : `hz`, `rr` (nil = absent), sinon `pico8_qh_struct_class()->field` ;
   - **écrivain chaîné** : `s` (écrit par `nr:hb`, dans le membre `s` de `qh`), sinon `pico8_qh_struct_class()->set_field` — **21 noms** : les 20 de `qh` plus `s`. `hz`, `rr` refusés (jamais écrits sur une instance) ;
   - accesseur public `pico8_nr_struct_class()`.
7. **Méthodes de `nr` à double forme, inchangées** (règle « Méthodes d'une parente convertie » : `fr` les atteint) : `pico8_nr_gv`, `pico8_nr_ldown`, `pico8_nr_hb`, `pico8_nr_cn` gardent corps et signatures (`pico8_instance_t *`), lisent et écrivent par `pico8_entity_get_field`/`set_field`/`pico8_mv`, déjà valides sur les deux formes. Elles passeront à la forme struct seule à 1a.25.
8. **`pico8_nr_new`** rend `pico8_nr_t *`, même paramètre ; son commentaire dit le contrat réel (`overrides` lu, pas gardé : `id`, `p`, `ra`). `pico8_fr_new` et `pico8_fr_class` inchangés.
9. **Tests** : nouveau `test_pico8_nr` (caractérisation, `UNIT_TESTS_SLOW`) ; `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_mz`, `test_pico8_bg` : changements de forme seulement (voir Testing).
10. **Plan** : à la ligne 1a.24 de `docs/plan-3.md`, ajouter « Découvert à 1a.24 : … » avec au minimum : `hz`, `rr` (défauts sans membre chez un ancêtre) → membres de `nr` ; `s` (écrit par `nr:hb`, `planet_family_trade.c:147`) → accepté par l'écrivain de `nr` avant les 20 noms de `qh` ; défauts `nu`/`kq` (`jo`), `od`/`fp`/`s`/`dw` (`qh`) rangés dans les membres hérités ; descripteur aux 15 entrées de `qh` sauf `GV`/`LDOWN`/`HB`/`CN` = ceux de `nr`, lecteur et écrivain chaînés (21 noms) ; les quatre méthodes et leurs installations dans `nr->defaults` gardées à double forme pour `fr` (1a.25) ; clés de construction `id` (filet) et `p`/`ra` (`test_pico8_mz`, `test_pico8_bg`), chacune facultative ; `pico8_qh_struct_init` publique ; `mt` lu sur un `nr` (`slipway.c:45`) absent aujourd'hui comme demain ; tests qui reconnaissent `nr` par son descripteur.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `nr` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `fr`, `ga`, `g_` à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `nr` mesurée (constructions et clés par rejeu, défauts, champs écrits et par qui, noms lus, méthodes par nom, accès à la classe, tests qui le lisent), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux une struct `nr` dont le premier membre est `pico8_qh_t` et qui n'ajoute que `hz` et `rr`, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun membre de `nr` ne répète un nom d'un ancêtre, afin qu'un lecteur passant par un descripteur ancêtre voie la valeur du slipgate.
5. En tant que gardien du filet, je veux que `od`=10, `fp`=1, `s`=170, `dw`=0.01, `kq` et `nu` de `nr` écrasent les membres hérités, afin que son sprite, sa boîte de survol, sa sonde et ses listes `cg` soient ceux du slipgate.
6. En tant que gardien du filet, je veux qu'un slipgate garde `hp`=1, `px`, `me`="kz", `dq`, `hf`, `qh`=1, `mi`=1 et `ba` de `qh`, afin que ses lectures héritées ne changent pas.
7. En tant que gardien du filet, je veux qu'un slipgate construit par `ie:ldown` porte l'`id` que l'icône lui donne, afin que `session_events` et `trading_hub_session_events` restent identiques au pixel près.
8. En tant que développeur de tests, je veux que `pico8_nr_new` accepte encore `p` et `ra`, afin que `test_pico8_mz` et `test_pico8_bg` gardent leur intention.
9. En tant que mainteneur, je veux qu'une clé de construction autre que `id`, `p`, `ra` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt (§7).
10. En tant que gardien du filet, je veux que le constructeur appelle `jo:pq` puis le `pq` de `ke`, afin que `oe`, `hn`, `jy`, `kw` existent avant l'enregistrement.
11. En tant que gardien du filet, je veux que `mv(…,"ldown")` sur un slipgate de moins de 3 liaisons rende un `jc` dont `qu` est ce slipgate, et faux sinon, afin qu'on tire une route depuis lui comme avant.
12. En tant que gardien du filet, je veux que `mv(…,"gv")` sur un slipgate rende faux quand `s.qu` est lui-même ou qu'il a déjà 3 liaisons, afin que les routes refusées le restent.
13. En tant que gardien du filet, je veux que `hb` écrive `s` = `hz[#oe+1]` sur la struct, afin que le sprite du slipgate suive son nombre de liaisons.
14. En tant que gardien du filet, je veux que `cn` relaie une proposition de commerce à chaque voisin hors de la route, afin que le commerce traverse les slipgates comme avant.
15. En tant que gardien du filet, je veux que `dc`, `dg`, `ui:bh`, `jz` et l'économie trouvent `kj`, `ev`, `hd`, `ly`, `hq`, `ey`, `hk`, `kf` sur une struct `nr`, afin qu'il se dessine, se sonde, se survole et se relie comme avant.
16. En tant que mainteneur, je veux que les 11 entrées non surchargées du descripteur de `nr` soient les mêmes pointeurs que celles de `qh`, et ses 4 surcharges les mêmes pointeurs que les installations de `nr->defaults`, afin qu'un `nr` et un `fr` exécutent le même code.
17. En tant qu'auteur de 1a.25 (`fr`), je veux que les quatre méthodes de `nr` restent installées dans `nr->defaults` et à double forme, afin qu'un `fr`, encore instance, les atteigne et se comporte exactement comme avant.
18. En tant qu'auteur de 1a.25, je veux `hz` et `rr` membres de `nr`, afin que les défauts `hz`, `rr` de `fr` y tombent sans membre en double.
19. En tant que mainteneur, je veux que `nu`, `kq`, `hz` d'un slipgate neuf soient la même REF que celles de `nr->defaults`, et `px`, `ba` celles de `qh`, afin qu'aucune table ne soit re-parsée ni dupliquée.
20. En tant que mainteneur, je veux que l'écrivain de `nr` accepte exactement les 20 noms de `qh` plus `s`, afin qu'un nom hors de cette liste soit un arrêt.
21. En tant que mainteneur, je veux que le lecteur de `nr` délègue à celui de `qh`, afin qu'une fonction de `qh`, `jo` ou de la sonde lise un slipgate sans savoir que c'en est un.
22. En tant que gardien de la couverture, je veux que le constructeur enregistre `"nr"` une fois par slipgate, afin que `class-coverage-net` soit identique ligne à ligne (`nr 6 2/15`).
23. En tant que vérificateur, je veux une caractérisation de `nr` figée sur l'ancien code, qui couvre aussi les `fr` du rejeu `session`, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`rr`, `px`, identité des tables, `ldown` refusé à 3 liaisons).
24. En tant que vérificateur, je veux un rapport `gcov` montrant chaque ligne ajoutée ou modifiée exécutée par la caractérisation, et 2 à 3 mutations ciblées sur ce que `gcov` ne prouve pas, afin de juger la preuve sans monter mon propre banc de mutations.
25. En tant que gardien des caractérisations, je veux que `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_mz` et `test_pico8_bg` ne changent que leur façon de reconnaître ou de recevoir un `nr`, sans qu'aucune valeur figée ne bouge, afin que 1a.12 à 1a.23 restent des preuves.
26. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
27. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.24, afin que la liste des étapes dise ce que l'étape a réellement touché.
28. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
29. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
30. En tant que développeur, je veux que `make test` reste rapide, afin que le nouveau test à rejeu long tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : le module de la famille de planètes (visibilité de `pico8_qh_struct_init` seulement) et son voisin `_trade` (struct, défauts, constructeur, descripteur, lecteur/écrivain, accesseur, `pico8_nr_new`). `pico8_struct.h` (enum), `planet*`, `probe`, `slipway`, `research_icon`, `cart`, `economy*`, `ui` : aucun changement.
- **`hz`, `rr` membres propres** : défauts de la chaîne sans membre chez un ancêtre (§5). `rr` n'est pas remonté dans `qh` : `qh` n'en a pas dans sa spec, et `nl`/`hj` ont chacun le leur ; la phase 2 reclasse.
- **`s` accepté par l'écrivain de `nr`, dans le membre de `qh`** : champ écrit par la méthode propre de la fille (`nr:hb`) dont le nom a déjà un membre chez un ancêtre. Précédent : `ra`/`p` écrits par `bv` et acceptés par l'écrivain de `qh` dans les membres de `jo` (1a.18). Pas d'élargissement de l'écrivain de `qh` : `qh:hb` n'écrit pas `s`.
- **Défauts rangés dans les membres hérités quand le nom existe** : même piège qu'à 1a.18-1a.23 — un `nr.s` propre laisserait `qh.s`=174 visible au lecteur de `qh`, et `hb` écrirait l'un et l'autre lirait l'autre.
- **Méthodes de `nr` à double forme, installations gardées** : règle « Méthodes d'une parente convertie » ; mesuré, `fr` appelle `gv`, `ldown`, `hb`, `cn` par nom. Précédent : `hj` à 1a.21. Rien à réécrire : leurs corps passent déjà par l'API à double forme.
- **Descripteur `static const` qui nomme les pointeurs de `qh` et les dispatchers de `nr`** (G) : les 11 dispatchers hérités sont déjà publics ; les 4 de `nr` sont dans le même fichier. Le test vérifie l'égalité entrée par entrée avec `pico8_qh_struct_class()` et avec `nr->defaults`.
- **Lecteur et écrivain chaînés par `pico8_qh_struct_class()->field/set_field`** (G) : évite de rendre publics `pico8_qh_field`/`pico8_qh_set_field` ; même forme que `mx` sur `hj`.
- **`pico8_qh_struct_init` publique** : seule visibilité à élargir, annoncée par la note de 1a.19. Rejeté : déplacer `nr` dans `planet_family.c` — la ligne du plan situe `nr` dans `_trade`, et `fr`/`mx` y vivent.
- **Clés `id`, `p`, `ra`, chacune facultative** (G) : `id` seule sur le filet ; `p`/`ra` passées par `test_pico8_mz` et `test_pico8_bg` (mesurées), toutes deux des membres de `jo` (précédent `pm`, 1a.22). Rejeté : réécrire ces deux tests pour ne passer que `id` — changement d'intention hors de l'étape, pour un gain nul.
- **`pico8_nr_new` rend `pico8_nr_t *`** (G) : précédent de toutes les classes converties ; `test_pico8_mz`/`test_pico8_bg` changent seulement le type de la variable (`void *`), qui se passe implicitement à `pico8_nr_gv`.
- **Constructeur `jo:pq` → `ke:pq`** : ordre du walk du cart (même que `qh`). Leçon de 1a.23 : l'inversion de ces deux appels est un mutant équivalent ; la mutation d'ordre porteuse est « défauts de `nr` copiés avant `pico8_qh_struct_init` ».
- **8 défauts copiés depuis la classe à l'init** (G), scalaires compris. Rejeté : les écrire en dur (deux sources).
- **Sémantique de nil** : `ep` présent-nil sur l'instance (TROU-9), absent sur la struct ; ses lecteurs testent présence et vérité : même branche. `mt`, `qu` absents sous les deux formes. Pas d'écart.
- **Clés dynamiques** : aucune nouvelle (`ldown` est déjà une entrée de l'enum).
- **TROU** : aucun écart attendu. `pico8_nr_ldown` garde son `/* TROU-6 */`. Si l'implémenteur en trouve un, règle §7.
- **Commentaires** (CLAUDE.md) :
  - à ajouter : contrat de la struct `nr` (jamais libérée, TROU-7 ; un défaut dont le nom est un membre d'un ancêtre va dans ce membre ; `nu`/`kq`/`hz` sont les tables de la classe `nr`, `px`/`ba` celles de `qh`, empruntées) ; contrat public de `pico8_qh_struct_init` (repris de son commentaire actuel) ; assert des clés ; contrat de `pico8_nr_new` ; contrat de l'écrivain (`s`, puis les 20 de `qh`) ;
  - à corriger si rendus faux : « `overrides` is copied, not kept » de `pico8_nr_new` (reste vrai pour `fr`), « Frees the three classes » reste vrai ; commentaires de tests qui disent `nr` instance (`test_pico8_mx` : « `nr`, `fr`, `ga` stay instances », `test_pico8_qh` : liste des structs) ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert (et `make e2e` au dernier). La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_nr` sur l'ancien code** (voir Testing), valeurs figées, dans `UNIT_TESTS_SLOW`. Arrêt ici si `session_events` ne construit pas exactement 5 `nr` et 5 `fr`, ou `trading_hub_session_events` exactement 1 `nr` : mesure fausse, à corriger dans le PRD.
2. **Visibilité, sans conversion** : `pico8_qh_struct_init` publique. Suite verte, aucune valeur figée ne bouge.
3. **`nr` convertie** : struct, statiques de défauts, constructeur, descripteur, lecteur/écrivain, accesseur, `pico8_class_set_struct_new`, `pico8_nr_new` typé ; `test_pico8_qh`, `test_pico8_jo` (reconnaissance de `nr`), `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx` (`nr` quitte `others`), `test_pico8_mz`, `test_pico8_bg` (type de la variable) relus. Toutes les caractérisations passent **sans changer leurs valeurs figées** ; les vérifications de forme s'ajoutent à `test_pico8_nr`.
4. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.24 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF et des pointeurs de fonction, `s` après `hb`, le `jc` rendu par `ldown`. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_nr`** (lié comme `test_pico8_mx`, sur le cart réel, dépendances Makefile sur `session_events.json`, `trading_hub_session_events.json` et `empty_planet_cleared_events.json`) — **dans `UNIT_TESTS_SLOW`**. Il lit **uniquement** par l'API à double forme et par `pico8_class_get_field(pico8_nr_class()/pico8_qh_class()/pico8_jo_class(), …)` pour les défauts de spec ; son code est le même avant et après, aux vérifications de forme près. Chaque scénario tourne dans un fils forké. **Toute présence se lit par un `lookup` qui confond nil et absent** (celui de `test_pico8_mx`) : `ep` écrit nil reste présent sur l'instance (TROU-9), absent sur la struct.
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction dirigée** (partie lancée comme dans `test_pico8_mx`) : `build(pico8_nr_class())` par l'icône (`ie:ldown`), deux fois. Empreinte de `id`, `me`, `t`, `rr`, `hp`, `s`, `od`, `fp`, `dw`, `dq`, `qh`, `mi`, `hf`, `hs`, `fa`, `ma`, `ca` ; présence de `hz`, `rr`, des 10 de `qh` et des 31 de `jo` selon leur état ; absence de `mt`, `qu`, `qw`, `ht`, `qy`, `et` ; présence des 15 méthodes et absence de `ph`, `kz`, `rdown`, `md`, `ol`, `pr`, `eb`, `mm`. Appartenance et rang dans `hv`, `cv.kj`/`ev`/`hd`/`hk`/`hb`, `cg.nr`/`np`/`prober`, **absence de `cg.jo` et `cg.qh`**. **Identité** : `nu`, `kq`, `hz` = REF de `pico8_class_get_field(pico8_nr_class(), …)` ; `px`, `ba` = REF de la classe de `qh` ; `em`, `ka`, `dz`, `cq`, `gq` = REF de la classe de `jo` ; `kw` ≠ REF de la classe de `jo`. Figée.
    2. **Comportement dirigé** : `dg` en `"kz"` puis `"ly"` ; `gv` avec un `s` sans `qu` → vrai, puis avec `oe` à 3 éléments posé par l'écrivain générique → faux ; `ldown` à 0 liaison → un `jc` (struct) dont `qu` est le slipgate, à 3 liaisons → faux ; `hb` avec `oe` de 0 à 3 éléments → `s` = 170, 170, 170, 168 ; `kj` via `dc` → empreinte d'écran. Figés. `cn` relayé est prouvé par les rejeux (573 appels, dont 35 relais d'un `nr` à l'autre) et `gcov` ; une proposition dirigée n'est ajoutée que si la recette (un `trade` et une route) se reprend de `test_pico8_rich_planet_exports` sans nouvel outillage.
    3. **Rejeux `session_events` et `trading_hub_session_events`** (`--from-title`) : par frame, pour chaque `nr` **et chaque `fr`** de `hv` : rang, classe (`nr`/`fr`), `id`, `me`, `t`, `ra` x/y, `s`, `mi`, tailles de `oe`/`jy`, `m_`, présence de `et` ; plus une empreinte de l'écran ; repliés en une empreinte de séquence figée par rejeu, avec le nombre de `nr` vus (5 / 1), de `fr` vus (5 / 0) et le nombre de frames. Les `fr` y sont parce qu'ils atteignent les méthodes de `nr` par `nr->defaults` : ce commit ne doit pas les changer.
  - **Forme** (ajoutée au morceau 3) : un slipgate construit par `ie:ldown` est `STRUCT` avec le descripteur de `nr` ; ses entrées `HD`, `KJ`, `GG`, `HK`, `KF`, `IB`, `LL`, `HQ`, `EV`, `EY`, `LY` sont **les mêmes** que celles de `pico8_qh_struct_class()` ; `GV`, `LDOWN`, `HB`, `CN` sont les fonctions de `class_value(pico8_nr_class(), …)` et diffèrent de celles de `qh` ; les 8 autres NULL ; `pico8_entity_set_field` passe pour les 20 noms de `qh` et `s`, et asserte (dans un fils) pour `hz`, `rr`, `mt`, `qw`, `ht` ; une clé de construction autre que `id`/`p`/`ra`, ou positionnelle, asserte ; `pico8_nr_new(NULL)` rend une struct sans `id` mais avec `oe` et `kw` ; `pico8_nr_class()->struct_new` posé, `pico8_fr_class()->struct_new` NULL ; les 4 installations de `nr->defaults` présentes ; un `fr` construit par l'icône reste `INSTANCE` et `mv(fr,"hb")` écrit `s` = 78.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (`git archive` du commit candidat, supprimée après usage), compiler `test_pico8_nr` et les objets du moteur avec `--coverage -O0`, le lancer, puis `gcov` sur les deux fichiers de la famille. Même piège qu'à 1a.23 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`. Les branches d'assert atteintes seulement dans un fils qui avorte (`must_abort`) ne sont pas comptées : prouvées par le `SIGABRT` attendu et justifiées ainsi. Le rapport (lignes exécutables ajoutées ou modifiées de `src/`, leur compte, lignes à 0 avec justification) est collé dans le message de commit.
  - **2 à 3 mutations**, chacune rouge sur `test_pico8_nr` (ou sur la caractérisation qui la nomme), notées dans le commit :
    1. **valeur d'un défaut propre** : `s` (ou `od`) de `nr` rangé dans une copie hors lecteur au lieu du membre de `qh` ;
    2. **entrée du descripteur remplacée** : `HB` = `pico8_jo_hb_dispatch` (`s` ne suit plus `hz`, `lq` écrit) — ou `LDOWN` = `pico8_jo_ldown_dispatch` (plus de refus à 3 liaisons) ;
    3. **ordre du constructeur** : défauts de `nr` copiés avant `pico8_qh_struct_init` (`qh` les écrase : `s`=174, `od`=15). Pas `ke:pq` avant `jo:pq` (équivalent, 1a.23).
- **`test_pico8_qh`** : `descendant_of` reconnaît une struct au descripteur de `nr` ; la boucle finale de `test_qh_is_a_struct` attend `nr` struct. **`test_pico8_jo`** : `daughter_of` idem. **`test_pico8_nl`**, **`test_pico8_os`**, **`test_pico8_hj`**, **`test_pico8_pm`**, **`test_pico8_mx`** : `nr` retiré de `others`. **`test_pico8_mz`**, **`test_pico8_bg`** : la variable qui reçoit `pico8_nr_new` change de type. Aucune valeur figée touchée.
- **Tests existants** : `test_pico8_ie` (diff vide), `test_pico8_jc`, `geoharvest_burst`, `qb`, `ga_export_panel_by`, `nf`, `js`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage` : diff vide. Tous les tests de régression (§6) : diff vide, verts. Aucun ne construit un `nr` : aucune preuve rouge n'est due.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** : `test_pico8_mx` (structure, `lookup`, `must_abort`, `build` par l'icône, rejeu depuis le titre, fille hors du fichier de sa parente), `test_pico8_qh` (rejeux `session`/`hub`, `descendant_of`), `test_pico8_hj` à 1a.21 (parente convertie dont les installations restent pour une descendante instance), `test_pico8_mz`/`test_pico8_bg` (`pico8_nr_gv`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `nr` n'est pas `nr  6  2/15`, `fr` n'est pas `fr  5  1/15`, `qh` n'est pas `qh  8  3/15`, ou si une autre ligne change. Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées/modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations (défaut propre, entrée du descripteur, ordre du constructeur) ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier : si l'assertion des clés ou celle de l'écrivain se déclenche sur un rejeu ou un test ; si un membre propre autre que `hz` et `rr` est ajouté ; si un membre propre répète un nom d'un ancêtre ; si l'écrivain de `nr` accepte un nom hors des 20 de `qh` plus `s`, ou si la liste des 20 de `qh` ou des 17 de `jo` change ; si l'enum change.
- `FAILED:` si une autre classe que `nr` est convertie ou change de forme (`fr`, `ga`, `g_`, `ui`, …), ou si le comportement de `qh`, `nl`, `os`, `hj`, `pm` ou `mx` change (valeur figée de leurs caractérisations, lignes de couverture).
- `FAILED:` si `pico8_nr_gv`, `pico8_nr_ldown`, `pico8_nr_hb` ou `pico8_nr_cn` change de corps ou de signature, ou si une de leurs quatre installations dans `g_nr_class->defaults` est retirée ou changée (`fr` les atteint encore) ; si une fonction de `qh`/`jo` (`pico8_qh_ly_dispatch`, `pico8_na`, `pico8_qh_field`, `pico8_qh_set_field`, `pico8_jo_*`) change de corps ; si une installation `->defaults` de `qh`/`jo` est retirée ou changée.
- `FAILED:` si une entrée du descripteur de `nr` hors `GV`/`LDOWN`/`HB`/`CN` n'est pas le même pointeur que l'entrée correspondante du descripteur de `qh`, ou si `GV`/`LDOWN`/`HB`/`CN` ne sont pas les mêmes pointeurs que les installations de `nr->defaults`.
- `FAILED:` si une table de défaut de `nr` est re-parsée depuis la spec, ou si `nu`/`kq`/`hz` d'un slipgate neuf ne sont pas les REF de `nr->defaults`.
- `FAILED:` si le constructeur de `nr` n'appelle pas `jo:pq` puis `ke:pq`, ou copie les défauts de `nr` avant `pico8_qh_struct_init`.
- `FAILED:` si un test de régression (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une valeur figée de `test_pico8_nr` a été changée après la conversion, ou une de `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_mz`, `test_pico8_bg`, `ie`, `jc`, `geoharvest_burst`, `qb`, `nf`, `js`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo` ; ou si ces neuf tests changent au-delà de la reconnaissance de `nr`, des listes `others` et du type de la variable qui reçoit `pico8_nr_new`.
- `FAILED:` si `test_pico8_nr` n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- la struct `nr` n'a que `pico8_qh_t`, `hz` et `rr` ; son descripteur n'a que les 15 méthodes de `qh`, `GV`/`LDOWN`/`HB`/`CN` surchargées ;
- `pico8_qh_struct_init` est publique, appelée par les constructeurs de `qh`, `nl`, `os`, `hj` (via `pico8_hj_struct_init`) et `nr`, et par eux seuls ;
- grep : `PICO8_CLASS_COVERAGE_NAME(g_nr_class, "nr")` et les quatre installations dans `g_nr_class->defaults` restent ; `pico8_class_set_struct_new(g_nr_class, …)` présent ; aucun `pico8_class_set_struct_new(g_fr_class` ; aucune nouvelle lecture de `->defaults` ;
- hors des deux modules de la famille, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_nr.c` (nouveau), `test_pico8_qh.c`, `test_pico8_jo.c`, `test_pico8_nl.c`, `test_pico8_os.c`, `test_pico8_hj.c`, `test_pico8_pm.c`, `test_pico8_mx.c`, `test_pico8_mz.c`, `test_pico8_bg.c` et le `Makefile` (cible, `UNIT_TESTS`, `UNIT_TESTS_SLOW`) bougent ;
- la ligne 1a.24 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.24 ».

## Out of Scope

- `fr` (1a.25), `ga` (1a.26), `g_` (1a.27) : leurs structs, leurs `->defaults`. `mt` de `fr`.
- Le passage à la forme struct seule de `pico8_nr_gv`/`ldown`/`hb`/`cn` et le retrait de leurs installations dans `nr->defaults` (à `fr`, 1a.25, dernière descendante de `nr`) ; celui de `pico8_qh_ly_dispatch` et des fonctions de `qh`/`jo` (`fr`, `ga` encore instances).
- Rendre publics `pico8_qh_field`, `pico8_qh_set_field` ou les dispatchers de `nr`.
- Le type de `s` dans `pico8_nr_gv`, de `lw`/`t`/`w` dans `pico8_nr_cn`, et l'instance vide sur la pile de `pico8_nr_ldown`.
- La suppression de la `pico8_class_t` de `nr` (1a.42).
- Le typage C des membres (1b) ; archétypes (phase 2).
- Libérer un slipgate (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Première parente convertie hors du fichier de `qh`, avec une descendante encore instance** : combine les deux situations de 1a.21 (installations gardées pour une descendante) et de 1a.23 (fille hors du fichier de sa parente). La dette posée ici (quatre méthodes à double forme, quatre installations) se solde à 1a.25.
- **Quatre surcharges d'un coup** : le descripteur de `nr` est le premier à surcharger plusieurs entrées de `jo` à la fois ; le test vérifie 11 égalités avec `qh` et 4 égalités avec `nr->defaults`.
- **`nr` hors de `cg.jo`** : son `nu` ne porte pas `"jo"`, d'où l'absence de `gg`/`ib`/`cc`/`by`/`kr`/`mi` écrits par l'économie (mesuré) ; un test qui compterait les `jo` ne le voit pas.
- **Leçon de 1a.23 appliquée** : mutation d'ordre sur la copie des défauts, pas sur `jo:pq`/`ke:pq` ; piège `_exit` des fils réglé dans la copie jetable seulement.
- Mesures : grep sur `src/`, `tests/`, `tools/`, `Makefile` à a88fe1f ; lecture de `pico8/lua.lua:412-416,472-483,539-540,1101-1127,1182`, de `pico8_planet_family.c/h`, `pico8_planet_family_trade.c/h`, `pico8_entity.c`, `pico8_ke.c`, `pico8_class.c`, `pico8_slipway.c`, `pico8_ui.c`, des tests nommés, de `tools/class_coverage_net_expected.txt` et du message de af03e84 ; sonde (6 constructions `nr` et 5 `fr`, clé `id` à chacune ; 14 noms écrits sur un `nr` dont `s` seul hors des 20 de `qh` ; écriture nil `ep` ; 0 `pico8_instance_get_field` ; méthodes par nom toutes dans l'enum ; 15 rejeux en sortie 0) dans une copie jetable du scratchpad, supprimée après usage.
