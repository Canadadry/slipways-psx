---
title: "Plan 3, étape 1a.27 : `g_` (la sonde) devient une struct fille de `ke` avec 23 membres propres ; `fv`, sa fille encore instance, garde la classe `g_`, son `pq` et ses 5 installations, et toutes les fonctions de `g_` restent à double forme"
description: "La sonde `g_` est construite 17 fois sur le filet, toujours par `bg:ldown` (`kv.g_`, sans clé). Elle passe en struct `pico8_g__t` (`pico8_ke_t` puis 18 défauts et 5 champs écrits), au descripteur de 5 méthodes (les mêmes pointeurs que les installations). Mesuré : `fv`, fille de `g_` et instance jusqu'à 1a.28, résout `ev`, `mm` et le `pq` (`f_`) dans la classe `g_`. Aucune fonction ne change donc de corps ni de signature, et la classe `g_` n'est pas touchée hors du branchement du constructeur."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.27 — `g_` : `ke` (`probe.c:372`). Tests : `probe_mm_rnd_order`.

Le numéro de ligne tient encore à HEAD 5d4871e. Dans `pico8_probe.c` :
- `pico8_probe_init` est à `:369` ;
- la déclaration `g_probe_class = pico8_class_declare(pico8_ke_class(), …)` est à `:372-375`, son nom de couverture à `:376` ;
- `pico8_class_set_pq(g_probe_class, pico8_f_)` est à `:377` ;
- les 5 installations `hq`/`ey`/`ev`/`kj`/`mm` dans `g_probe_class->defaults` sont à `:378-382`, `register_named("g_")` à `:383` ;
- `pico8_g__class` est à `:402`, `pico8_g__new` à `:406-408` ;
- les fonctions de `g_` : `pico8_bv` `:119` (et son dispatcher `:189`), `pico8_dl` `:195` (`:250`), `pico8_d_` `:258` (`:315`), `pico8_g__mm` `:322`. `pico8_f_`, `pico8_ez`, `pico8_ei` et `pico8_ei_dispatch` sont dans `pico8_particle.c` (`:61`, `:72`, `:88`).

Ailleurs :
- `kv.g_` est `pico8_class_lookup_named("g_")` suivi de `pico8_class_instantiate(cls, NULL)`, dans `pico8_bg_ldown` (`bg.c:229-247`) ;
- `fv` est déclarée contre `pico8_g__class()` dans `pico8_help_init` (`help.c:129`), avec son `pq` et 2 installations (`hd`, `kj`) ;
- `cart.c:92,98` asserte seulement les parents (`fv` → `g_`, `g_` → `ke`).

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. Les descendants non convertis trouvent leurs champs hérités en remontant `parent`, qui reste valide.
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre (dispatcher hérité dans `defaults`, ou appel direct […]) reste à double forme […]. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante.
>
> **Accès directs aux défauts** : […] une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec.

`pico8_g__t` commence par `pico8_ke_t`. `g_` est une **racine sous `ke`** : son seul parent est `ke`, déjà converti. Elle a **une fille non convertie, `fv`** (1a.28). C'est la situation de `nr` à 1a.24 (parente d'une fille restée instance), avec trois différences :
- la parente est `ke`, pas une planète : aucun lecteur ni écrivain d'ancêtre à chaîner hors de `pico8_ke_field` ;
- `g_` a un **`pq` de classe** (`f_`), que `fv` atteint par le walk ;
- **aucune** fonction de `g_` ne change, pas même sa signature (voir « Fonctions de `g_` » ci-dessous).

Notes du plan qui échoient ici :

- **1a.10** : « helpers partagés `bv`/`dl`/`d_` (`probe.c`) » ; **1a.11** : « `et.qs` lu par `dl` » ; **1a.13** : lecteurs à simple forme dans `bv` passés à double forme. Déjà faits : ils restent tels quels.
- **1a.16b** : `pico8_probe_field_truthy` (`probe.c:49`) et le garde de `bv` (`probe.c:128`) déjà sur l'API d'entité.
- **1a.21** : `hj.mm = g_.mm`. Le descripteur de `hj` nomme `pico8_g__mm` (`planet_family.c:499,640`, `planet_family_trade.c:561`). Inchangé.
- **1a.23 / 1a.26** : « `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_` restent à double forme (`g_`, `fv`) ». Le PRD de 1a.26 précise que leur passage à la forme struct seule revient au commit de `fv` (1a.28), pas de `g_`. Mesuré à nouveau ici (voir ci-dessous).

§6 : un test de régression nommé, `probe_mm_rnd_order`. Il est réécrit pour lire la struct, et la preuve rouge est refaite dans ce commit.

§7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` : `g_  17  9/15`, `fv  19  15/15`, toutes les autres lignes inchangées ;
- **preuve des tests par `gcov` et 2 à 3 mutations** ;
- un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.27, avec « découvert à 1a.27 » dans le message ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.26 ont posé (jusqu'à 288e7f6), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`).
  - L'enum contient déjà `HQ`, `EY`, `EV`, `KJ`, `MM` : il ne change pas.
  - `pico8_entity_get_field` rend d'abord l'entrée du descripteur, sinon le lecteur ; un membre nil est absent.
  - L'écrivain rend 0 pour un nom sans membre, et `pico8_entity_set_field` asserte alors.
- **`pico8_ke_t`** (`me`, `t`), `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq`. `pico8_ke_ks` (`ke.c:36-47`) et `dg` (`bootstrap.c:97-103`) écrivent `me`/`t` directement dans `pico8_ke_t` sur une struct.
- **Racines de `ke` converties** : `bg` (1a.12) est le modèle le plus proche pour la forme du fichier (lecteur par table d'offsets, écrivain, constructeur `static` posé par `pico8_class_set_struct_new`, accesseur public, crochet de couverture). `mz` (1a.10) est l'autre produit de `bg:ldown`.
- **1a.24** (`nr`) : précédent d'une parente convertie dont une fille reste instance. Les installations sont gardées dans `->defaults` pour la fille, **mêmes pointeurs** dans le descripteur, et les fonctions gardent corps et signatures.
- **1a.26** (`ga`) : fonctions de particules atteintes par un `ga` struct. `f_`, `ez`, `ei` marchent déjà sur une struct par l'API d'entité, et la preuve `gcov` utilise `__gcov_dump()` avant les `_exit` des fils, dans la copie jetable seulement.

### Ce que fait le cart

`pico8/lua.lua` :

```
:377      bg:ldown : return (#(cg.g_ or {})>0 and mz or g_)()
:685-707  f_ (self.qy={}), ez (add(self.qy,self:mm())), ei (fa<10 and not gd → rien ; points, puis ez(self,self.fc))
:708-721  g_=ke:jr([[ nu=o("g_","prober"), fx=1, dq="ly", od=0,fp=f, fa=2, dw=0.025,
                      py=0.06,ki=9, pn=3,pk=1, fc=1, k_=o(1,1,5,13,6),
                      qo=1.02,ro=-0.0525, qg=0.0525,q_=1.02, du=2 ]])
:722-726  g_.hq=bv  g_.ey=d_  g_.ev=dl  g_.pq=f_  g_.kj=ei
:727-731  g_:mm : a=(flr(rnd(self.pn))+rnd(0.5))/self.pn+self.t*0.001 ; {p, v=p*py (pa si pk), l=ki}
:1143-1170 fv=g_:jr([[ fa=10, ra, hi, ca, gq, eo, ff, fc=5, k_, qo…q_, pn=5, du=15, py, pk, ki=17, kq, ql, fg, rb ]])
           fv.hd=gk ; fv:pq (pz(self,bo)) ; fv:kj (ei(self) bl(self.rb))
:86-97    kv:jr : walk pq de la feuille à la racine, dédoublonné
```

Points décisifs :

- **Une seule voie de construction dans le jeu** : `bg:ldown`, sans table (`kv.g_()`). Seuls deux tests passent par une autre voie que `bg:ldown`, et un seul passe des clés (voir « Clés »).
- **Walk `pq`** : `f_` (le `pq` de `g_`), puis `ke:pq` (`lt`). Pour un `fv` : `fv:pq`, puis `f_` (hérité de `g_`), puis `lt`.
- **Défauts (18)** : aucun ne tombe sur un membre de `ke` (`me`, `t`). Les 18 sont des membres propres.
- **`nu` de `fv` est celui de `g_`** : `fv` n'a pas de `nu` propre. Un écran d'aide s'enregistre donc sous `cg.g_` et `cg.prober` (TROU-8). C'est aussi pourquoi `bg:ldown`, pendant que l'aide vit, lancerait un `mz`.

### Emprise réelle de `g_` (mesurée)

**Méthode.**
- Grep de `src/`, `tests/`, `tools/` et du `Makefile` à 5d4871e.
- **Sonde** sur une copie `git archive HEAD` jetable du scratchpad, supprimée après usage. Elle journalise, avec les deux adresses de retour résolues par `addr2line` :
  - chaque lecture de champ sur une instance de classe `g_` ou `fv`, avec sa source (propre, classe propriétaire, absent) et, pour une fonction, la classe qui l'installe ;
  - chaque écriture par `pico8_entity_set_field` ;
  - chaque accès à `pico8_instance_fields` ;
  - chaque instanciation, avec ses clés.
- Binaire de couverture relancé sur les 17 fichiers `tests/fixtures/*events.json` et sur `events_recorded_1790198294.json` (`--from-title` pour les 4 enregistrements), tous en sortie 0.
- **Fil piège** sur une seconde copie jetable, supprimée après usage. Il journalise toute lecture `pico8_instance_get_field` et tout `pico8_instance_fields` sur un `g_` qui ne passe pas par l'API d'entité, `ke_ks`, `dg` ou les gardes de `f_`/`bv`, ainsi que tout nom écrit sur un `g_` par `pico8_entity_set_field`. Lancé sur `make unit` et les 8 TU de `UNIT_TESTS_SLOW`, suite verte.

**Constructions d'un `g_`** : **17 sur le filet**, dans 9 rejeux, toutes par `pico8_bg_ldown` (`bg.c:246`), **sans clé** :
- 1 par rejeu dans `session`, `phase4_playthrough`, `built_planet_panel`, `empty_planet_cleared`, `geoharvest_burst` et `trading_hub_session` ;
- 2 dans `slipway_drag`, 4 dans `events_recorded`, 5 dans `long_session`.

Hors filet : 1 dans `demo`.

**Constructions d'un `fv`** : une par rejeu (5 dans `long_session`), par `pico8_fv_new(NULL)` (`cart.c:159`).

**Défauts de `g_`** (18) : `nu`=`o("g_","prober")`, `fx`=1, `dq`=`"ly"`, `od`=0, `fp`=`f`, `fa`=2, `dw`=0.025, `py`=0.06, `ki`=9, `pn`=3, `pk`=1, `fc`=1, `k_`=`o(1,1,5,13,6)`, `qo`=1.02, `ro`=-0.0525, `qg`=0.0525, `q_`=1.02, `du`=2.
- Aucun ne tombe sur un membre de `ke`.
- Deux sont des tables, `nu` et `k_`, lues seulement. Aucune n'est modifiée en place, et aucun défaut n'est réaffecté sur la classe (grep de `g_probe_class->defaults` : les 5 installations seules).

**Champs écrits sur un `g_`** (sonde) :
- `qy` (`f_`, `particle.c:69`, 18) ;
- `ra`, `p`, `et` (`bv`, `probe.c:132,138,179`, 97 chacun) ;
- `p_` (`d_`, `probe.c:311`, 5 dans 3 rejeux) ;
- `me`/`t` directement : `ke_ks` depuis `d_` (13) et `dg` (`t`).

`fo` n'est jamais écrit sur un `g_` : `bv` l'écrit sur sa **cible**. `m_`, `ma`, `oe`, `lq` n'y sont jamais écrits non plus.

**→ 23 membres propres** : les 18 défauts, plus `qy`, `ra`, `p`, `et`, `p_`.

**Noms lus sur un `g_`** :
- présents : `dq`, `du`, `dw` (dont `jo:kj`, `planet_display.c:555`, sur le `fo` d'une planète), `et`, `fa`, `fc`, `fp`, `fx` (`mz:pq`, `wormhole.c:80`), `k_`, `ki`, `me`, `nu`, `od`, `p`, `p_`, `pk`, `pn`, `py`, `q_`, `qg`, `qo`, `qy`, `ra` (dont `wormhole_display.c:29`), `ro`, `t` ;
- absents : `hb`, `hd`, `hk` (`lt`/`he`), `ik` (`d_`), `ly` (`dg`, après `ks("ly")`), `ph` (`dg`), `p_` (`dg`, avant `d_`).

**Méthodes résolues sur un `g_`** : `ev`, `kj` (dont `lt`/`he`), `hq`, `ey`, `mm`, toutes dans `g_->defaults`.

**Méthodes résolues sur un `fv` dans la classe `g_`** (sonde) : `ev` (`lt`, et `dc("ev")` : 636 dans 4 rejeux) et `mm` (3 180, par `ez` dans `ei` depuis `fv:kj`). Son `pq` hérité, `f_`, est appelé par le walk (22). `fv` lit aussi `nu` dans la classe `g_`. `hq`, `ey` et le `kj` de `g_` ne sont pas atteints par `fv` sur le filet, mais ils restent joignables par héritage (`fv` ne les surcharge pas, sauf `kj`).

**Fonctions de `g_` et qui d'autre les atteint** (grep) :
- `pico8_f_` : `pq` de `g_` (`fv` par le walk), `hj`/`pm`/`mx`/`ga` ;
- `pico8_ez`, `pico8_ei` : `ei` est le `kj` de `g_`, appelé aussi par `fv:kj`, `hj:kj` et `ga:kj` ;
- `pico8_g__mm` : `fv` (hérité), et les descripteurs de `hj`/`pm`/`mx` ;
- `pico8_bv` : `HQ` du descripteur de `qh` et de ses filles ;
- `pico8_dl` : `EV` de `qh`, `jc:ev`, l'enveloppe `ie.ke` ;
- `pico8_d_` : `EY` de `qh`, `jc:ey`.

**Aucune n'est atteinte par `g_` seul** : aucune ne change de corps ni de signature dans ce commit. Toutes lisent et écrivent déjà `self` par l'API à double forme, avec gardes aiguillés (`f_` `:63`, `bv` `:128`, `ke_ks`).

**Tests qui lisent un `g_` par l'API d'instance seule** (fil piège) : **un seul**, `test_pico8_probe_mm_rnd_order` (`pico8_instance_get_field(probe, "qy")`).

**Tests qui écrivent sur un `g_` un nom que le jeu n'écrit pas** : **un seul**, `test_pico8_jc:724` (`pico8_entity_set_field(probe, "dw", 0)`).

**Tests qui reconnaissent un `g_` par sa classe** (grep) :
- `test_pico8_bg:389-398` (`test_ldown` : tag `INSTANCE`, `cls == lookup_named("g_")`) ;
- `test_pico8_bg:572-574` (`lheld_kind`, `HELD_G_INSTANCE`).

**Tests qui comparent à une installation de `g_`** : `test_pico8_hj:390,754` (`class_function(pico8_g__class(), "mm")`). L'installation reste : ils sont inchangés.

**Clés de construction** :
- filet : aucune ;
- `test_pico8_probe_mm_rnd_order` : `pn`, `t`, `du`, `py` (par `pico8_g__new`) ;
- `test_pico8_jc` : aucune (`pico8_class_instantiate(lookup_named("g_"), NULL)`) ;
- `test_pico8_bg`, `test_pico8_mz` : par `pico8_bg_ldown`.

**Outils** :
- `tools/float_audit_c1.txt` ne nomme aucun site de `pico8_probe.c` ;
- `tools/ref_audit_report.sh` lance `test_pico8_slipway_drag_refaudit`, qui construit une sonde : inchangé.

**Durée** : `slipway_drag_events` fait 226 frames et construit 2 sondes (l'une valide sa cible, l'autre tombe dans la branche `p_`).

## Solution

1. **Enum : inchangée.**
2. **Struct `g_`**, publique dans `pico8_probe.h` : `pico8_ke_t` en premier membre, puis les 18 défauts dans l'ordre de la spec (`nu`, `fx`, `dq`, `od`, `fp`, `fa`, `dw`, `py`, `ki`, `pn`, `pk`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`, `du`), puis `qy`, `ra`, `p`, `et`, `p_`. Aucun membre ne répète `me` ou `t`.
3. **Défauts pris par REF, jamais re-parsés** : `pico8_probe_init` déclare `g_` comme aujourd'hui (même spec, même parent, même nom de couverture, même `register_named`). Elle **garde** `pico8_class_set_pq(g_probe_class, pico8_f_)` et **les 5 installations**, pour `fv`. Elle pose `pico8_class_set_struct_new(g_probe_class, …)`, puis lit **une fois** les 18 défauts dans la classe vers un état statique, chaque entrée à l'offset de son membre. Un assert porte le message « chaque défaut est une clé de la spec de `g_` ».
   - `nu` et `k_` sont **les mêmes REF** que `g_probe_class->defaults`, donc aussi le `nu` que `fv` résout par héritage.
   - `pico8_probe_shutdown` remet cet état à nil avant de libérer la classe, sans libérer aucune table.
4. **Constructeur de `g_`** (`static`, posé par `set_struct_new`), dans l'ordre du walk du cart :
   1. alloue un `pico8_g__t` ;
   2. `pico8_ke_struct_init(&self->ke, &<descripteur de g_>)` ;
   3. met les 5 membres écrits à nil, puis copie les 18 défauts statiques ;
   4. recopie chaque clé de construction présente parmi les **4 clés mesurées** (`pn`, `du`, `py` → membres propres ; `t` → `ke.t`), chacune facultative. Il **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise, et ne garde aucun pointeur sur la table ;
   5. enregistre `"g_"` au crochet de couverture ;
   6. `pico8_f_` (casté), puis `pico8_ke_struct_pq`.
5. **Descripteur de `g_`** (`static const`) :
   - 5 entrées, **les mêmes pointeurs que les 5 installations** : `HQ` = `pico8_bv_dispatch`, `EY` = `pico8_d__dispatch`, `EV` = `pico8_dl_dispatch`, `KJ` = `pico8_ei_dispatch`, `MM` = `pico8_g__mm`. Les autres sont NULL ;
   - **lecteur** : les 23 membres propres (nil = absent), sinon `pico8_ke_field` ;
   - **écrivain** : `qy`, `ra`, `p`, `et`, `p_`, et rien d'autre (`me`/`t` sont écrits directement par `ke_ks`/`dg`) ;
   - accesseur public `pico8_g__struct_class()`.
6. **Fonctions de `g_` : aucun changement** (règle « Méthodes d'une parente convertie » : `fv` les atteint, et d'autres classes aussi). `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_ei_dispatch`, `pico8_bv`, `pico8_dl`, `pico8_d_`, leurs dispatchers et `pico8_g__mm` gardent corps et signatures. Elles passeront à la forme struct seule, pour celles que plus rien n'atteint sous forme d'instance, au commit de `fv` (1a.28).
7. **`pico8_bg_ldown` : aucun changement.** `kv.g_` construit la struct par délégation (`pico8_class_instantiate` → `struct_new`).
8. **`pico8_g__new`** rend `pico8_g__t *`, même paramètre. Son commentaire dit le contrat réel : `overrides` est lu, pas gardé ; seules `pn`, `t`, `du`, `py` sont acceptées ; appelée avant `pico8_probe_init()`, elle asserte (c'est déjà le cas aujourd'hui, via `pico8_class_instantiate(NULL, …)` : le commentaire actuel, « yields a class-less instance », est faux).
9. **Tests** :
   - nouveau `test_pico8_g_` (caractérisation dirigée, rejeu court, et la fille `fv`) ;
   - `probe_mm_rnd_order` réécrit (§6) ;
   - `test_pico8_bg`, `test_pico8_jc` : changements de forme seulement.

   Voir Testing Decisions.
10. **Plan** : à la ligne 1a.27 de `docs/plan-3.md`, ajouter « Découvert à 1a.27 : … » avec au minimum :
    - 18 défauts (aucun sur un membre de `ke`) et `qy` (`f_`), `ra`/`p`/`et` (`bv`), `p_` (`d_`) → 23 membres de `g_` ;
    - `nu`/`k_` pris par REF dans la classe, partagés avec `fv` ;
    - descripteur aux 5 entrées = les installations (mêmes pointeurs), lecteur (23 membres, puis `ke`), écrivain (5 noms) ;
    - clés de construction : aucune sur le filet, `pn`/`t`/`du`/`py` (`probe_mm_rnd_order`), chacune facultative ;
    - le `pq` (`f_`) et les 5 installations de la classe `g_` gardés pour `fv`, qui résout `ev`, `mm` et `f_` dans la classe `g_` (mesuré) ;
    - aucune fonction de `g_` changée ;
    - `dw` écrit par `test_pico8_jc` dans le membre.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `g_` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `fv` à 1a.28.
2. En tant qu'autopilote, je veux l'emprise de `g_` mesurée (constructions et clés, défauts, champs écrits, noms lus, méthodes résolues, ce que `fv` résout dans la classe `g_`, tests qui lisent un `g_` par l'API d'instance), afin que chaque site touché soit vérifiable et que rien ne soit retiré à tort.
3. En tant que lecteur du code, je veux une struct `g_` dont le premier membre est `pico8_ke_t` et qui n'ajoute que les 23 noms mesurés, minifiés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux que les 18 défauts soient copiés à chaque construction depuis un état lu une fois dans la classe, afin qu'aucune table ne soit re-parsée et que `nu`/`k_` restent les REF de la classe.
5. En tant que gardien du filet, je veux qu'une sonde lancée par `bg:ldown` s'enregistre dans `hv`, `cv.kj`, `cv.ev`, `cg.g_` et `cg.prober`, au même rang qu'avant, afin que `dc`, `dg` et le choix `mz`/`g_` de `bg:ldown` restent identiques.
6. En tant que gardien du filet, je veux que le constructeur appelle `f_` puis `lt`, afin que `qy` existe avant le premier `kj`.
7. En tant que gardien du filet, je veux que `hq` (`bv`) écrive `ra` (la REF de la souris, aliasée), `p`, `et` et le `fo` de la cible comme avant, afin que le glissé d'une sonde trace la même ligne et accroche la même planète ou le même slipway.
8. En tant que gardien du filet, je veux que `ey` (`d_`) fasse basculer la sonde en `ly` par `ks(dq)` quand la cible est absente et que `fm:lu` réussit, ou pose `p_` sinon, afin que la sonde meure ou se valide à la même frame.
9. En tant que gardien du filet, je veux que `ev` (`dl`) affiche le même texte (`qs` de la cible, sprite 31, ou le coût `od$`), afin que l'infobulle de la sonde ne change pas.
10. En tant que gardien du filet, je veux que `kj` (`ei`) et `mm` (`g_:mm`) émettent et peignent les mêmes particules aux mêmes tirages, afin que l'état du générateur reste identique.
11. En tant que gardien du filet, je veux que `mz:pq` lise `fx` et que `jo:kj` lise `dw` sur une sonde comme avant, afin qu'un trou de ver et le cercle d'accroche d'une planète ne changent pas.
12. En tant que mainteneur, je veux les 5 entrées du descripteur égales aux 5 installations (mêmes pointeurs), afin qu'une sonde struct et un `fv` instance exécutent le même code.
13. En tant que mainteneur, je veux que l'écrivain de `g_` accepte `qy`, `ra`, `p`, `et`, `p_` et refuse tout autre nom, afin qu'une écriture jamais mesurée soit un arrêt (§7).
14. En tant que mainteneur, je veux qu'une clé de construction hors de `pn`, `t`, `du`, `py` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt.
15. En tant que gardien de `fv`, je veux que la classe `g_` garde son `pq` (`f_`), ses 5 installations et ses défauts de spec, afin que l'écran d'aide, encore instance, hérite comme avant de `nu`, `ev`, `mm` et du walk `f_`.
16. En tant que mainteneur, je veux que `f_`, `ez`, `ei`, `bv`, `dl`, `d_`, leurs dispatchers et `g_:mm` gardent corps et signature, afin que `fv`, `qh`, `jc`, `hj`, `ga` et l'enveloppe `ie.ke` les appellent comme avant.
17. En tant que mainteneur, je veux que `pico8_g__new` rende la struct et que son commentaire dise son vrai contrat, afin qu'un appelant sache quelles clés passent et ce qui se passe avant l'init.
18. En tant que gardien de la couverture, je veux que le constructeur enregistre `"g_"` une fois par sonde, afin que `class-coverage-net` soit identique ligne à ligne (`g_ 17 9/15`, `fv 19 15/15`).
19. En tant que vérificateur, je veux une caractérisation de `g_` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (clés de `pico8_g__new`, branches de `d_`, identité des tables, sonde et `fv` en même temps).
20. En tant que vérificateur, je veux que la caractérisation lise aussi un `fv`, afin que la règle « descendants non convertis trouvent leurs champs hérités en remontant `parent` » soit prouvée sur la seule fille restante.
21. En tant que vérificateur, je veux que `probe_mm_rnd_order` échoue encore si `g_:mm` tire `rnd(0.5)` avant `rnd(self.pn)`, afin que §6 tienne.
22. En tant que vérificateur, je veux un rapport `gcov` montrant chaque ligne ajoutée ou modifiée exécutée, et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
23. En tant que gardien des caractérisations, je veux que `test_pico8_bg` et `test_pico8_jc` ne changent que leur façon de reconnaître une sonde ou d'y poser `dw`, sans valeur figée touchée, afin que 1a.12 et 1a.13 restent des preuves.
24. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne des références de couverture ni de `float_audit_c1.txt` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
25. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.27, afin que la liste des étapes dise ce que l'étape a réellement touché, y compris ce que 1a.28 doit retirer.
26. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C, un piège ou un `TROU-N` nu (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
27. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
28. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : le module sonde (struct, état des défauts, constructeur, descripteur, lecteur, écrivain, accesseur, `pico8_g__new`) et son en-tête. Rien d'autre dans `src/` : pas de changement dans `particle`, `bg`, `help`, `entity*`, `ke`, `ui`, `wormhole*`, `planet*`, `cart`, `pico8_struct.h` (enum).
- **23 membres propres** : les 18 défauts (règle « chaque champ de sa chaîne », aucun ne répète `me`/`t`) et les 5 champs écrits par les fonctions de `g_` sur elle-même (`f_` : `qy` ; `bv` : `ra`, `p`, `et` ; `d_` : `p_`). Ce ne sont pas des écrivains génériques de `cg.jo` : ils vont dans la struct de `g_`.
  - Homonymes de membres de `hj`/`pm`/`ga` (`fc`, `k_`, `qo`…) : autre branche, aucun partage. C'est la phase 2 (règle 2) qui en fera un composant.
- **`fo`, `m_`, `ma` ne sont pas membres** (mesuré : jamais écrits sur une sonde). Un membre de trop serait sans effet en 1a (§5), mais la mesure est nette et l'écrivain refuse tout nom non mesuré.
- **Classe `g_` gardée entière : `pq`, 5 installations, défauts de spec** (G). Mesuré : `fv` résout `ev`, `mm` et `f_` dans la classe `g_`. `hq`, `ey` et le `kj` de `g_` ne sont pas atteints par `fv` sur le filet, mais ils restent joignables par héritage.
  - Précédent 1a.24 : `nr` a gardé ses 4 installations pour `fr`.
  - Le retrait de ce que plus rien n'atteint revient à 1a.28, au commit de la dernière descendante (précédents 1a.23, 1a.25, 1a.26).
- **Aucune fonction de `g_` ne change**, pas même son type de `self` (G). Toutes sont atteintes par une instance (`fv`) et par d'autres classes. Elles marchent déjà sur une struct : la sonde les appelle déjà sur des structs `qh`, `jc`, `hj`, `ga`, et le fil piège ne trouve aucun accès d'instance seul sur une sonde dans `src/`.
- **Pas d'init publique de `g_`** (G). Contrairement à `hj`/`qh` (précédents 1a.19, 1a.22), rien ne l'exige ici : `fv` reste instance jusqu'à 1a.28, qui coupera l'init s'il en a besoin. Le constructeur peut l'écrire en ligne. Rejeté : couper `pico8_g__struct_init` maintenant (code sans appelant hors du constructeur).
- **Lecteur chaîné sur `pico8_ke_field`, écrivain sans chaînage** (G). `ke` n'a pas d'écrivain : `me`/`t` s'écrivent directement dans `pico8_ke_t` (`ke_ks`, `dg`). Précédent `bg`.
- **Quatre clés, test seulement, chacune facultative** (G). `pn`, `t`, `du`, `py` sont passées par `probe_mm_rnd_order` et elles sont mesurées. Les accepter garde ce test de régression inchangé hors de sa lecture de `qy`. Précédent 1a.24 (`p`/`ra` de `test_pico8_mz`/`bg`). `t` va dans `ke.t` : il est posé **après** `pico8_ke_struct_init`, qui met `t`=0.
  - Rejeté : réécrire le test pour poser les membres après construction. Cela changerait la façon dont il construit, pas seulement sa lecture (§6).
- **Constructeur `f_` → `ke:pq`**, l'ordre du walk. Défauts et clés posés **avant** `f_` et `lt`, comme `mw(ob)` avant la boucle dans le cart.
  - `f_` après `lt` est un **mutant équivalent** : `lt` ne lit que les 5 noms de `bm` et `nu`, jamais `qy`. On ne l'utilise pas comme mutation.
  - La mutation d'ordre porteuse est « défauts copiés après `ke:pq` » : `nu` absent au moment de `lt`, donc aucun `cg`.
- **`pico8_g__new` asserte avant l'init** (G) : même comportement qu'aujourd'hui (`pico8_class_instantiate` asserte sur une classe NULL). Seul le commentaire change.
- **`test_pico8_jc` : `dw` posé dans le membre** (G), pas par l'écrivain. Le jeu n'écrit jamais `dw` sur une sonde, et l'écrivain le refuse. Précédent 1a.26 (`ga_export_panel_by` pose `kw`/`ra`/`gq` dans les membres). La valeur, l'ordre et les empreintes sont inchangés.
- **`test_pico8_bg`** : `test_ldown` attend la struct au descripteur de `g_` au lieu de l'instance de la classe `g_`, et la variable `g_class`, devenue inutile, est retirée. `lheld_kind` reconnaît la sonde par son descripteur. La constante `HELD_G_INSTANCE` peut être renommée (par exemple `HELD_G_STRUCT`) si **sa valeur numérique ne change pas** : elle est pliée dans l'empreinte.
- **`test_pico8_hj:390,754` : diff vide** (G). L'installation `mm` de la classe `g_` reste, donc la comparaison tient. 1a.28 la retargetera s'il retire l'installation.
- **Sémantique de nil** : les membres nil sont absents. `ra`, `p`, `et`, `p_` sont absents avant `bv`/`d_` sous les deux formes. `fp`=`f` est présent (booléen faux, pas nil), `od`=0 est présent. TROU-9 n'intervient pas (aucun nil écrit sur une sonde, mesuré).
- **Clés dynamiques** : aucune.
- **TROU-8** (`prober` sans lecteur) : préservé tel quel, `nu` gardant `"prober"`. Pas de nouvelle entrée `TROU`.
- **Commentaires** (CLAUDE.md) :
  - à ajouter :
    - le contrat de la struct `g_` : jamais libérée (TROU-7) ; `nu`/`k_` sont les tables de la classe `g_`, empruntées et partagées avec `fv` ; `ra` est la REF passée à `bv`, aliasée ;
    - l'assert de clé et celui de l'écrivain (message porteur, arrêt §7) ;
    - le contrat de `pico8_g__new` ;
    - le piège de la classe `g_` : le `pq` et les installations restent pour `fv` jusqu'à 1a.28 ;
  - à corriger ou retirer si rendus faux :
    - `pico8_probe.h` sur `pico8_g__new` (« `overrides` is copied », « yields a class-less instance ») ;
    - `pico8_probe_shutdown` (« never any `g_` instance ») s'il ne dit plus juste ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert (et `make e2e` au dernier). La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_g_` sur l'ancien code** (voir Testing Decisions), valeurs figées. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si une sonde de `bg:ldown` n'a pas `fa`=2, `dw`=0.025, `ki`=9, ou si le `fv` de l'écran titre ne partage pas le `nu` de la classe `g_` : la mesure est fausse et doit être corrigée dans le PRD.
2. **`g_` converti** :
   - struct, état des défauts, constructeur, descripteur, lecteur, écrivain, accesseur, `pico8_class_set_struct_new`, `pico8_g__new` typé ;
   - `probe_mm_rnd_order` réécrit, **preuve rouge** faite (voir Testing) ;
   - `test_pico8_bg` et `test_pico8_jc` adaptés.

   Toutes les caractérisations passent **sans changer leurs valeurs figées**. Les vérifications de forme de `g_` s'ajoutent à `test_pico8_g_`.
3. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.27 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF et des pointeurs de fonction, `et`, `p_`, `qy`, `me`. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_g_`** : lié comme `test_pico8_ga`, sur le cart réel, avec une dépendance Makefile sur `slipway_drag_events.json` et les fixtures qu'il lit.
  - Il lit **uniquement** par l'API à double forme, et par `pico8_class_get_field(pico8_g__class(), …)` pour les défauts de spec.
  - Les méthodes sont comparées aux **fonctions publiques** (`pico8_bv_dispatch`, `pico8_d__dispatch`, `pico8_dl_dispatch`, `pico8_ei_dispatch`, `pico8_g__mm`), qui ne changent pas.
  - Son code est le même avant et après, aux vérifications de forme près.
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()`, pour l'état de `rnd`.
  - Toute présence se lit par un `lookup` qui confond nil et absent.
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **Construction, trois voies** :
       - (a) `pico8_bg_ldown` dans une partie lancée comme dans `test_pico8_bg` (départ `eu(bo[2])`, `cg.g_` vide) ;
       - (b) `pico8_g__new(NULL)` ;
       - (c) `pico8_g__new` avec les 4 clés de `probe_mm_rnd_order`.

       Pour chacune, on prend :
       - l'empreinte des 18 défauts (scalaires, tailles et contenus de `nu`/`k_`), de `me`, `t`, de la taille de `qy` ;
       - l'absence de `ra`, `p`, `et`, `p_`, `fo`, `m_`, `ma`, `hd`, `hk`, `hb`, `ik`, `ly`, `ph` ;
       - les 5 méthodes égales aux fonctions publiques ;
       - le rang dans `hv`, l'appartenance à `cv.kj`/`cv.ev` (et pas à `hd`/`hk`/`hb`), à `cg.g_` et `cg.prober` ;
       - l'**identité** : `nu` et `k_` sont les REF de `g_probe_class->defaults`.

       Pour (c), `pn`, `t`, `du`, `py` valent les clés. Figée.
    2. **Comportement** (partie lancée, sonde de (a)) :
       - une particule rendue par `mv(e,"mm")` (champs `p`, `v`, `l` pliés) ;
       - `hq` en trois points, dont la cible est une planète, un slipway, puis le vide. On plie `et` par genre (planète, `jc`, jetable sans classe, `false`), le `fo` de la cible, l'identité de `ra` avec la REF passée et `p` ;
       - `ev` (`dl`) avec l'empreinte d'écran, cible présente ou non ;
       - `ey` (`d_`) dans ses deux branches, chacune dans un fils : cible présente → `p_` vrai et retour nil ; cible absente avec crédits → `me`=`"ly"`, `t`=0, retour vrai ;
       - `kj` (`ei`) avec l'empreinte d'écran et la taille de `qy` ;
       - quelques pas de `dg` (`t` qui avance, `p_` qui retire la sonde de `hv`/`cg` par `he`).

       Figés.
    3. **Rejeu court `slipway_drag_events`** (226 frames, départ `eu(bo[2])`, 2 sondes) : à chaque frame où une sonde vit, on plie son rang, `me`, `t`, `p_`, le genre de `et`, la taille de `qy` et `ra`. S'y ajoutent la frame de naissance et la frame de mort de chacune, et une empreinte finale. Figée.
    4. **La fille `fv`, encore instance** (après `pico8__init`, écran titre) :
       - tag `INSTANCE`, classe `fv`, `parent` = `pico8_g__class()` ;
       - `nu` est la REF de la classe `g_` ; `fx`=1 et `dq`=`"ly"` hérités ; `fa`=10 et `k_` propres ;
       - `ev` et `mm` égaux à `pico8_dl_dispatch` et `pico8_g__mm` ; `kj`, `hd` propres ;
       - `qy` présent (le walk a appelé `f_` une fois) ;
       - appartenance à `hv`, `cg.g_`, `cg.prober` ;
       - quelques frames de `dc("kj")` avec l'empreinte d'écran (particules de `ei` par `mm` hérité).

       Figée.
  - **Forme** (ajoutée au morceau 2) :
    - une sonde de chaque voie est `STRUCT` au descripteur de `g_` ;
    - ses 5 entrées valent les 5 fonctions et les autres sont NULL ;
    - `cls->field` et `cls->set_field` sont posés ;
    - `pico8_entity_set_field` passe pour `qy`, `ra`, `p`, `et`, `p_` et asserte (dans un fils) pour `dw`, `fx`, `nu`, `k_`, `fo`, `m_`, `me` ;
    - une clé de construction hors des 4, ou positionnelle, asserte ;
    - `pico8_g__class()->struct_new` est posé, `->pq == pico8_f_` ;
    - les 5 installations sont **présentes** dans `g_probe_class->defaults` (lecture directe de la table), mêmes pointeurs que le descripteur ;
    - le `fv` du titre est toujours `INSTANCE`.
- **Test de régression (§6)** :
  - **`probe_mm_rnd_order`** : `pico8_g__new` rend la struct. `qy` est lu par `pico8_entity_get_field` (ou le membre), et `pico8_ez` reçoit la sonde castée. Intention inchangée : la particule émise suit l'ordre de tirage du cart, `rnd(self.pn)` avant `rnd(0.5)`.
    - **Preuve rouge** : dans `pico8_g__mm`, tirer `rnd(0.5)` avant `rnd(pn)` (échanger les deux instructions). Le test doit échouer sur l'assertion de position. Puis restaurer.
  - Tous les autres tests de régression : diff vide, verts.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (`git archive` ou copie de l'arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` `test_pico8_g_`, `test_pico8_probe_mm_rnd_order`, `test_pico8_bg` et `test_pico8_jc`, avec les objets du moteur. Les lancer, puis `gcov` sur `pico8_probe.c`.
    - Même piège qu'à 1a.23-1a.26 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`.
    - Les branches d'assert atteintes seulement dans un fils qui avorte (`must_abort`) ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu et justifiées ainsi.
    - Le rapport est collé dans le message de commit. Il donne les lignes exécutables ajoutées ou modifiées de `src/` et leur compte : constructeur (voie `bg:ldown`, voie clés), lecture des défauts, reset, lecteur, écrivain, accesseur, `pico8_g__new`. Les lignes à 0 sont données avec leur justification.
  - **2 à 3 mutations**, chacune rouge sur `test_pico8_g_` (ou un autre test nommé), notées dans le commit avec le test qui l'attrape :
    1. **valeur d'un défaut propre** : `pn` (ou `ki`, `py`) non copié ou copié dans un autre membre. Les particules changent ;
    2. **entrée du descripteur remplacée** : `MM` = `pico8_ga_mm`, ou `EY` = NULL ;
    3. **ordre du constructeur** : défauts copiés **après** `pico8_ke_struct_pq` (`nu` absent à `lt`, donc aucun `cg`). Pas `f_` après `lt` (équivalent, voir Implementation Decisions).
- **Tests adaptés** : `test_pico8_bg` (`test_ldown`, `lheld_kind`) et `test_pico8_jc` (`dw` dans le membre). Aucune empreinte figée ne bouge.
- **Tests existants à diff vide** : `test_pico8_hj` (compare à l'installation `mm`, gardée), `test_pico8_mz` (`cg.g_` par pointeur), `test_pico8_jo`, `qh`, `nl`, `os`, `pm`, `mx`, `nr`, `fr`, `ga`, `ie`, `qb`, `nf`, `js`, `fq`, `iv`, `mh`, `je`, `mo`, `bg_kj`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage`, et tous les tests de régression autres que `probe_mm_rnd_order`.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** :
  - `test_pico8_ga` : structure, `lookup`, `must_abort`, fils par scénario, rejeu court, formes ;
  - `test_pico8_bg` : `bg:ldown` dans une partie lancée, `lheld_kind` sur un rejeu ;
  - `test_pico8_jc` (`test_bv`) : `hq` d'une sonde sur un slipway ;
  - `test_pico8_nr` : parente convertie dont la fille reste instance (installations gardées, fille lue par la caractérisation).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; c'est le seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` (en particulier `g_  17  9/15`, `fv  19  15/15`). Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations (défaut propre, entrée du descripteur, ordre du constructeur) ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier :
  - si l'assertion de clé ou celle de l'écrivain se déclenche sur un rejeu ou un test ;
  - si un membre autre que les 23 est ajouté, ou si un membre répète `me`/`t` ;
  - si l'écrivain de `g_` accepte autre chose que `qy`, `ra`, `p`, `et`, `p_` ;
  - si le constructeur accepte une clé hors de `pn`, `t`, `du`, `py` ;
  - si l'enum change.
- `FAILED:` si une autre classe que `g_` est convertie ou change de forme (`fv`, `pj`, `ui`, `hp`, `lq`, `hy`, `trade`…), ou si le comportement d'une autre classe change (valeur figée de sa caractérisation, ligne de couverture).
- `FAILED:` si une entrée du descripteur de `g_` n'est pas le même pointeur que l'installation correspondante de `g_probe_class->defaults`, ou si une entrée autre que `HQ`/`EY`/`EV`/`KJ`/`MM` est non NULL.
- `FAILED:` si `pico8_class_set_pq(g_probe_class, pico8_f_)` ou l'une des 5 installations `hq`/`ey`/`ev`/`kj`/`mm` est retirée ou changée. Également si une installation ou le `pq` de `fv` est touché, ou si `pico8_help.c` change.
- `FAILED:` si l'une de ces fonctions change de corps ou de signature : `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_ei_dispatch`, `pico8_bv`, `pico8_dl`, `pico8_d_`, leurs dispatchers, `pico8_g__mm`, `pico8_bg_ldown`, `pico8_ke_ks`, `dg`.
- `FAILED:` si une table de défaut de `g_` est re-parsée depuis la spec, ou si le `nu`/`k_` d'une sonde neuve n'est pas la REF de `g_probe_class->defaults`.
- `FAILED:` si le constructeur de `g_` n'appelle pas `f_` puis `ke:pq`, ou s'il copie les défauts ou pose les clés après l'un d'eux.
- `FAILED:` si `probe_mm_rnd_order` change d'intention (assertion retirée ou affaiblie, paramètres `pn`/`t`/`du`/`py` changés), ou si sa preuve rouge n'est pas notée dans le commit.
- `FAILED:` si un autre test de régression (§6) a un diff non vide.
- `FAILED:` si une valeur figée de `test_pico8_g_` a été changée après la conversion, ou une de `test_pico8_bg`, `jc`, `hj`, `mz`, `jo`, `qh`, `nl`, `os`, `pm`, `mx`, `nr`, `fr`, `ga`, `ie`, `qb`, `nf`, `js`, `fq`, `iv`, `mh`, `je`, `mo`, `bg_kj`. Également si `test_pico8_bg`/`test_pico8_jc` changent au-delà de la reconnaissance d'une sonde par son descripteur et de l'écriture de `dw` dans le membre, ou si la valeur numérique d'une constante pliée change.
- `FAILED:` si une ligne est ajoutée à `tools/float_audit_c1.txt` ou en est retirée, ou si `make float-audit` est rouge.
- `FAILED:` si `test_pico8_g_` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- la struct `g_` n'a que `pico8_ke_t` et les 23 membres ; son descripteur a 5 entrées ;
- grep :
  - `PICO8_CLASS_COVERAGE_NAME(g_probe_class, "g_")` et `pico8_class_register_named("g_", …)` restent ;
  - `pico8_class_set_struct_new(g_probe_class, …)` est présent ;
  - les 5 `set_name(g_probe_class->defaults, …)` et `pico8_class_set_pq(g_probe_class, pico8_f_)` sont inchangés ;
  - le seul autre accès à la classe `g_` est la lecture unique des défauts ;
- le `fv` du titre est toujours une instance dont le `parent` est la classe `g_` ;
- la ligne 1a.27 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.27 ».

## Out of Scope

- `fv` (1a.28) : sa conversion, et avec elle :
  - le retrait du `pq` et des installations de la classe `g_` que plus rien n'atteint ;
  - le passage à la forme struct seule de celles des fonctions de `g_` (`f_`, `ez`, `ei`, `g_:mm`, `bv`, `dl`, `d_`) que plus aucune instance n'atteint ;
  - le retargeting de `test_pico8_hj:390,754`.
- Les classes `pj`, `ui`, `trade`, `hp`, `lq`, `hy` et les instances sans classe (`fm`, `et={}` jetable de `bv`, `{}` de `nr:gv`…) : leurs étapes.
- L'objet `et` jetable construit par `bv` (`probe.c:167-172`) : 1a.38.
- `kv.g_` (`pico8_class_lookup_named`) et les registres de classes : supprimés en 1a.42.
- Corriger TROU-8 (`prober` sans lecteur).
- La mise en composant des champs d'émetteur de `g_`/`fv`/`hj`/`ga` (phase 2, règle 2).
- Le typage C des membres (1b).
- Libérer une sonde, sa liste `qy` ou le `p` de `bv` (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Petit commit** : aucune fonction ne change, à l'inverse de 1a.26. Le diff de `src/` tient dans le module sonde : struct, état, constructeur, descripteur, lecteur, écrivain, accesseur. L'essentiel du travail est la caractérisation, en particulier la lecture de `fv`, la seule fille non convertie.
- **Mesure utile pour 1a.28** : sur le filet, `fv` résout dans la classe `g_` `ev` (`dl`), `mm` (`g_:mm`) et le `pq` (`f_`), ainsi que `nu`. Il ne résout jamais `hq`, `ey`, ni le `kj` de `g_`. `fv:kj` appelle `ei` directement, et `fv:pq` appelle `pz`. Après `fv`, la classe `g_` n'aura plus de descendante instance.
- **`cg.g_` contient l'écran d'aide** : `fv` hérite du `nu` de `g_`. Tant que l'aide vit, `bg:ldown` lancerait un `mz`, pas une sonde. La caractérisation (a) part d'une partie lancée où `cg.g_` est vide, comme `test_pico8_bg`/`test_pico8_mz`.
- **Leçons de 1a.23-1a.26 appliquées** : mutation d'ordre sur ce qui compte (défauts avant `lt`), pas sur un mutant équivalent (`f_`/`lt`). Piège `_exit` des fils réglé dans la copie jetable seulement. Tests qui posent un champ jamais écrit par le jeu : écriture directe dans le membre, pas élargissement de l'écrivain.
- Mesures :
  - grep sur `src/`, `tests/`, `tools/` et le `Makefile` à 5d4871e ;
  - lecture de `pico8/lua.lua:80-97,370-380,680-731,1143-1170` ;
  - lecture de `pico8_probe.c/h`, `pico8_particle.c/h`, `pico8_help.c`, `pico8_bg.c`, `pico8_ke.c/h`, `pico8_struct.h`, `pico8_class.c`, `pico8_entity.c`, `pico8_bootstrap.c`, des tests nommés, de `tools/class_coverage_net_expected.txt`, `tools/float_audit_c1.txt`, `tools/ref_audit_report.sh`, de `docs/trous-de-portage.md` (TROU-6, TROU-7, TROU-8) et de `docs/glossaire-noms.md` (« La sonde `g_` ») ;
  - lecture du PRD et du message de 288e7f6 (1a.26) et du PRD de 1a.24 ;
  - sonde de champs et de méthodes d'un `g_` et d'un `fv`, dans une copie jetable du scratchpad, supprimée après usage : 18 rejeux en sortie 0 ; 17 `g_` sur le filet (9 rejeux), tous par `bg:ldown` sans clé ; `fv` résout `ev`/`mm`/`nu`/`f_` dans la classe `g_` ;
  - fil piège sur `make unit` et `UNIT_TESTS_SLOW`, copie jetable supprimée après usage : seul `probe_mm_rnd_order` lit une sonde par l'API d'instance (`qy`), et seul `test_pico8_jc` y écrit un nom que le jeu n'écrit pas (`dw`).
