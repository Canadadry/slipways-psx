---
title: "Plan 3, étape 1a.26 : `ga` (l'éclat de particules) devient une struct fille de `jo` avec 11 membres propres ; dernière descendante de `jo`, elle fait passer les fonctions de `jo` à la forme struct seule et retire les 11 installations et le `pq` de la classe `jo`"
description: "L'éclat `ga` est la dernière planète encore instance de la couche objet, construite 7 fois sur le filet par deux voies (`jo:hu` via `kv[hp.cw]`, clés `p`/`ra`/`id` ; `pm:pq`, 9 clés d'émetteur). Il passe en struct `pico8_ga_t` (`pico8_jo_t` puis 11 membres), `by` garde sa table partagée réécrite en liste, `ga:kj` lit toujours le `k_` de la classe (TROU-21). Mesuré : `ga` est la seule instance qui résout encore une fonction dans `jo->defaults` (`hd`, `hb`, `hk`) et dans `ga->defaults` (`kj`, `mm`) ; après lui, les 17 fonctions de `jo` prennent un `self` typé `pico8_jo_t *`, et les installations de `jo` et de `ga` sont retirées."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.26 — `ga` : `jo` (`planet_family.c:378`), `by` réécrit (`:401-406`), `k_` lu (`:271`), via `kv[hp.cw]`. Tests : `ga_export_panel_by`, `geoharvest_burst`.

Les numéros de ligne de la ligne du plan datent d'avant 1a.18. À HEAD c3e89b4, dans `pico8_planet_family.c` :
- la déclaration `g_ga_class = pico8_class_declare(pico8_jo_class(), …)` est à `:938`, son nom de couverture à `:951` et `pico8_class_set_pq(g_ga_class, pico8_ga_pq)` à `:952` ;
- **`by` réécrit** : l'échange ob_table → liste dans `g_ga_class->defaults` est à `:957-969` ;
- les installations `mm`/`kj` sont à `:971-972`, `register_named("ga")` à `:973` ;
- `pico8_ga_pq` est à `:694`, `pico8_ga_mm` à `:709`, `pico8_ga_kj` à `:756` ;
- **`k_` lu** : la lecture de `g_ga_class->defaults["k_"]` dans `ga:kj` est à `:803`, et elle est **TROU-21** ;
- `pico8_ga_kj_dispatch` est à `:822`, `pico8_ga_class`/`pico8_ga_new` à `:1068-1074` ;
- `pico8_pm_pq` appelle `pico8_ga_new` à `:575`.

Côté `jo`, dans `pico8_planet.c` : `pico8_class_set_pq(g_jo_class, pico8_jo_pq)` est à `:309`, les 11 installations dans `g_jo_class->defaults` à `:310-320`, les dispatchers à `:69-161`. Les fonctions sont à `:391-672` (`pq`, `lz`, `gg`, `kd`, `ll`, `hu`, `hh`, `hk`, `ldown`), dans `pico8_planet_display.c` (`hb` `:87`, `gh` `:157`, `gv` `:341`, `fe` `:451`, `kj` `:469`) et dans `pico8_planet_trade.c` (`kf` `:73`, `cn` `:101`, `ib` `:223`). `kv[hp.cw]` est `pico8_class_lookup_named(cw)` suivi de `pico8_class_instantiate`, dans `pico8_jo_hu` (`planet.c:512-530`).

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Défauts mutés sur la classe** : […] Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** (connus : […] `ga.by` […]).
>
> **Champ d'un écrivain générique** : […] seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille.
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre […] reste à double forme […]. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante (pour `jo` : 1a.26).
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde […] que la **même REF** que l'état de la struct.

`pico8_ga_t` commence par `pico8_jo_t`, qui commence par `pico8_ke_t`. `ga` est une **feuille**, la **seule fille de `jo` hors de la branche `qh`**, et la **dernière descendante non convertie de `jo`**. C'est la situation de `fr` à 1a.25, qui a fermé `nr` et `qh`. Deux différences :
- `ga` a des **méthodes propres** (`pq`, `mm`, `kj`), comme `mx` à 1a.23 ;
- la parente qu'il ferme, `jo`, a **17 fonctions** à retyper, réparties dans trois fichiers. Au commit de `mx`, `hj` n'en avait qu'une.

Notes du plan qui échoient ici :

- **1a.17** (PRD de `jo`) : « Fonctions de `jo` inchangées, signatures comprises » ; « Le passage à la forme struct seule des fonctions de `jo`, le retrait de ses 11 installations et de son `pq` de classe, le retypage de leurs signatures (1a.26) » ; « `live_ga` (`geoharvest_burst`) : réécriture en 1a.26 » (déjà reportée de 1a.10 à 1a.16) ; « `ga.by`, `ga.k_` » hors périmètre jusqu'ici.
- **1a.16b** : lectures de `ga` hors de `src/` notées pour 1a.26 : `ga_export_panel_by:80` (`by`) et `geoharvest_burst:94` (`qy`).
- **1a.22** : « Le `ga` engendré par `pm:pq` reste une instance construite par `pico8_ga_new` ».
- **1a.23** : « `f_`, `g_.mm`, `ez`, `ei` restent à double forme (`g_`, `ga`) ».
- **1a.25** : « Après ce commit, la seule planète encore instance est `ga`, dont la conversion fermera `jo` ». Le critère interdisait aussi de changer une fonction `pico8_jo_*`.

§6 : deux tests de régression nommés, `ga_export_panel_by` et `geoharvest_burst`. Ils sont réécrits pour lire la struct, et la preuve rouge est refaite dans ce commit.

§7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` : `ga  7  3/15`, `jo  1710  15/15`, `pm  4  2/15`, toutes les autres lignes inchangées ;
- **preuve des tests par `gcov` et 2 à 3 mutations** ;
- un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.26, avec « découvert à 1a.26 » dans le message ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.25 ont posé (jusqu'à 3cbb057), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` : méthodes indexées par l'enum, `field`, `set_field`.
  - L'enum contient déjà `KJ` et `MM` et les 11 méthodes de `jo`.
  - `pico8_entity_get_field` rend d'abord l'entrée du descripteur, sinon le lecteur ; un membre nil est absent.
  - L'écrivain rend 0 pour un nom sans membre, et `pico8_entity_set_field` asserte alors.
- **`pico8_jo_t`** (31 membres) et `pico8_jo_struct_init` (public). S'y ajoutent :
  - `pico8_jo_struct_class()`, dont le lecteur et l'écrivain (17 noms) sont joignables par `->field`/`->set_field` : c'est le chaînage utilisé par `qh` ;
  - les 11 dispatchers publics, `pico8_jo_pq` public ;
  - `g_jo_defaults`, lu une fois dans la classe (dont `kw`, état partagé modifié en place, 1a.17).
- **1a.22** (`pm`) : `pm:pq` à forme struct seule et retiré de la classe. `test_pico8_pm` suit déjà le `ga` engendré (`is_ga` par la chaîne de `fz`, indépendant de la forme ; `fold_spawned` : `ra`, taille de `qy`, `mo.ma`).
- **1a.23** (`mx`) : précédent d'une **méthode propre qui quitte `->defaults`** (`mx:kf`) et de la **fermeture d'une parente** (`hj`). Les comparaisons aux installations sont retargetées vers le descripteur avant la conversion ; `ke:pq` avant `jo:pq` est un mutant équivalent.
- **1a.25** (`fr`) : fermeture de deux parentes, `self` seul retypé. La preuve `gcov` utilise `__gcov_dump()` avant les `_exit` des fils, dans la copie jetable seulement.
- **1a.17** : `test_pico8_jo`. Son rejeu `session` exige un `ga` (`required = {QH, NR, FR, PM, GA}`) et plie le premier : `ca`, `gq`, `em`, `gg`, `hd`. `daughter_of` le reconnaît encore comme instance.

### Ce que fait le cart

`pico8/lua.lua` :

```
:448-455  jo:hu : if hp.cw then self.p_=true
                    self=kv[hp.cw]({p=self.p,ra=self.ra,id=self.id})  … self:kd()
:1030-1035 pm:pq : ga(ob([[qo=0.97,ro=-0.1255,qg=0.1255,q_=0.97,
                    k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1),iu=-0.009,py=9,ms=1.5]],{ra=self.ra+self.ng}))
:1067-1078 ga=jo:jr([[ fz="ga", nu=o(), hf=f, by=o(), kq=b(0,0,-1,-1), fa=3, fc=0,
                    qo=0.9,ro=0, qg=0,q_=0.9, k_=o(1,2,4,9,9,10,10,7,7,7,7,7),
                    iu=-0.06,py=6,ms=3, ]])
:1079-1080 ga:pq  : f_(self) ez(self,140) mo.ma=3
:1081-1086 ga:mm  : particule {p, v=p*ms, a=p*iu, l=rnd(py)+py}
:1087-1094 ga:kj  : 3 cercles, couleur self.k_[flr(pt.l/2)] ; ei(self) ; self.p_=not pt
:685-707  f_, ez, ei  (partagés : g_.pq=f_, g_.kj=ei, fv:kj → ei)
:86-97    kv:jr : walk pq de la feuille à la racine, dédoublonné
:500-514  jo:ib : add(ib,a) dans self.by (la table de la classe si l'instance n'en a pas)
```

Points décisifs :

- **Deux voies de construction.**
  - `jo:hu`, quand la recette a `cw`. Clés `p`, `ra`, `id`, chacune recopiée seulement si elle est présente sur l'ancienne planète. La recette « harvest » (`rr="geoharvesting"`) mène à `ga`.
  - `pm:pq`. Clés `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms`, `ra`.
  - `pico8_ga_new(NULL)` n'est appelé que par `test_pico8_ga_export_panel_by`.
- **Walk `pq`** : `ga:pq`, puis `jo:pq`, puis `ke:pq`. Les clés de construction sont posées **avant** `ga:pq` (`mw(ob)` avant la boucle), si bien que les `ms`/`iu`/`py` de `pm` pilotent les 140 particules d'`ez`.
- **Défauts (15)**.
  - 5 tombent sur un membre de `jo` : `nu`=`o()` (vide : `ga` n'entre dans aucun `cg`), `hf`=`f`, `by`=`o()`, `kq`, `fa`=3.
  - 10 n'ont de membre chez aucun ancêtre : `fz`, `fc`, `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms`. Ce sont des homonymes de membres de `hj`/`pm`, mais dans une autre branche.
- **`qy`** est écrit par `f_` (`pq` propre de `ga`) : c'est un membre propre, précédent `hj` à 1a.21.
- **`by` est une table de défaut modifiée en place** (`jo:ib` fait `add(ib,a)` sur `self.by`). Le port la réécrit à l'init en liste (`ob()` produit une ob_table, alors que les six lecteurs de `by` castent en liste). C'est l'objet de `ga_export_panel_by`.
- **TROU-21** : `ga:kj` lit `self.k_`, donc pour un `ga` de protoétoile la palette de `pm`. Le port lit le `k_` **de la classe `ga`**. `ei` lit, lui, le `k_` de l'instance. Il faut préserver cet écart, pas le corriger.

### Emprise réelle de `ga` (mesurée)

**Méthode.**
- Grep de `src/`, `tests/`, `tools/` et du `Makefile` à c3e89b4.
- **Sonde** sur une copie `git archive c3e89b4` jetable du scratchpad, supprimée après usage. Elle journalise :
  - chaque résolution d'une **valeur fonction** dans `defaults` (classe de départ, propriétaire, nom) ;
  - chaque lecture et écriture de champ sur une instance de classe `ga`, avec sa source (propre, classe, absent) ;
  - chaque accès à `pico8_instance_fields` sur un `ga` ;
  - chaque `pico8_class_instantiate` de `ga`, avec ses clés.
- Binaire de couverture relancé sur les 17 fichiers `tests/fixtures/*events.json` et sur `events_recorded_1790198294.json` (`--from-title` pour les 4 enregistrements), tous en sortie 0.

**Constructions d'un `ga`** : **7 sur le filet**.

| Rejeu | Via `pm:pq` | Via `jo:hu` | Total |
|---|---|---|---|
| `session_events` | 3 | 0 | 3 |
| `events_recorded_1790198294` | 1 | 2 | 3 |
| `geoharvest_burst` | 0 | 1 | 1 |
| Autres rejeux | 0 | 0 | 0 |

Clés : `p,ra,id` par `jo:hu`, `qo,ro,qg,q_,k_,iu,py,ms,ra` par `pm:pq`. Aucune autre clé, aucune clé positionnelle.

**Défauts de `ga`** (15) : `fz`=`"ga"`, `nu`=`o()`, `hf`=`f`, `by`=`o()` (liste vide après l'échange), `kq`=`b(0,0,-1,-1)`, `fa`=3, `fc`=0, `qo`=0.9, `ro`=0, `qg`=0, `q_`=0.9, `k_`=`o(1,2,4,9,9,10,10,7,7,7,7,7)`, `iu`=-0.06, `py`=6, `ms`=3.
- Ceux qui tombent sur un membre hérité (5, tous de `jo`) : `nu`, `hf`, `by`, `kq`, `fa`.
- Ceux qui en résultent comme membres propres : `fz`, `fc`, `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms` (10), plus `qy` (écrit par `f_`). **11 membres.**

**Champs écrits sur un `ga`** (sonde) :
- `qy` (`f_`, 7) ;
- `oe` (`jo:pq`, 7) ;
- `p_` (`ga:kj`, 751) ;
- `ma` (`jo:kd` après `jo:hu`, 3) ;
- `lq` (`jo:hb`, 11) ;
- `me`/`t` par l'accesseur direct (`ke_ks`, `dg`).

Hors `qy`, tous figurent parmi les 17 noms de l'écrivain de `jo`.

**Noms lus sur un `ga`** : `ra`, `qy`, `fa`, `fc`, `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms`, `kq`, `me`, `t`, `p_`, `nu`, `by`, `kw`. Trois noms sont lus absents : `hp` (`jo:pq`), `jy` (`jo:hb`), `kr` (`jo:hk`). S'y ajoutent `ev` et `unknown` (le `me` de `ga` est `"unknown"` : `dg` ne trouve aucune méthode).

**Méthodes résolues sur un `ga`** (sonde, 3 rejeux) :
- dans `ga->defaults` : `mm` 980, `kj` 765 ;
- dans `jo->defaults` : `hd` 758, `hb` 25, `hk` 16.

**Qui atteint `jo->defaults` et `ga->defaults`** : **seule la classe de départ `ga`**, sur tout le filet. Les autres résolutions de fonctions viennent de `lq`, `ui`, `pj`, `g_`, `fv`, `hy`, `hp` : aucune dans `jo` ni dans `ga`. Le `pq` de classe de `jo` n'est atteint que par le walk d'un `ga` (seule descendante instance).

**Fonctions de `jo` (17, avec `self`)** et leurs appelants :
- dispatchers (`planet.c`) ;
- `pq` : constructeurs de `jo`/`qh`/`nl`/`os`/`hj`/`pm`/`mx`/`nr`/`fr`, plus le walk de la classe ;
- `lz` : `jo:pq`, `jo:hu`, `test_pico8_jo:635`, `rich_planet_exports:140` ;
- `kd` : `hu`, `hh`, `kf` ;
- `gg` : `score_panel.c:137`, `jo:kj` ;
- `hh` : `wormhole.c:237` ;
- `hu` : `planet_display.c:396` ;
- `fe` : `ldown`, `gv` ;
- `gh` : `gv`, `ga_export_panel_by:119,144`, `planet_gh_row_lq:113`, `rich_planet_exports:86` ;
- `gv` : `test_pico8_mz:719`, `test_pico8_bg:499` (des `void *`) ;
- `kj` : `hj:kj`, `planet_kj_dispatch:53` ;
- `kf` : `mx:kf`.

**Après ce commit, chaque `self` qu'elles reçoivent est une struct qui commence par `pico8_jo_t`.**

**Noms lus sur `self` par les fonctions de `jo` qui ne sont pas des membres de `jo`/`ke`** : `hp` (`pq`), `px` (`gh`), `s` (`kj`), `ba` (`cn`), tous des membres de `qh`. Ils se lisent par le lecteur du descripteur.

**Fonctions partagées qui restent à double forme** (atteintes par `g_`/`fv`, instances jusqu'à 1a.27/1a.28, ou par des classes de `ke`) :
- `pico8_f_` (`pq` de `g_probe_class`) ;
- `pico8_ez` (par `ei`) ;
- `pico8_ei` (`g_.kj`, `fv:kj`) ;
- `pico8_g__mm` (`g_probe_class`) ;
- `pico8_bv`/`dl`/`d_` et leurs dispatchers (`g_probe_class`) ;
- `pico8_na`, `pico8_qh_ly_dispatch` (1a.25) ;
- `pico8_gk` (`hd`, partagé avec `ie`, `fv`…), `pico8_gd`, `ma()`, `pico8_ke_ks`, `dg` ;
- les écrivains et lecteurs génériques de `cg.jo` (`economy*.c`, `research_display.c`, `score_panel*.c`, `msgbox.c`).

Ce ne sont pas des fonctions de `jo` : leur corps ne change pas.

**`->defaults` et classe de `ga`** : tout est dans `pico8_planet_family_init`, sauf la lecture de `k_` dans `ga:kj`. `cart.c:84` asserte seulement que le `parent` de `ga` est `jo`.

**Tests qui lisent un `ga` ou comparent aux installations retirées** :
- **`geoharvest_burst`** (régression) : `live_ga` (`e->cls == pico8_class_lookup("ga")`) et `particle_count` (`pico8_instance_get_field(ga,"qy")`).
- **`ga_export_panel_by`** (régression) : `pico8_ga_new(NULL)`, écrit `kw`/`ra`/`gq` dans `planet->fields`, lit `by` par `pico8_instance_get_field`, `pico8_jo_gh(planet)`.
- **`test_pico8_qb`** : `ga_alive` (`:385-393`), qui exige le tag instance.
- **`test_pico8_jo`** : `daughter_of` (`:422-469`, `ga` par sa classe) ; `fold_planet` (`:302-307`) et `test_jo_is_a_struct` (`:706-710`) comparent aux installations de la classe `jo`.
- **`test_pico8_pm`** (`:691`, `:765-772`) et **`test_pico8_mx`** (`:754`) : `ga` dans `others`, attendu `INSTANCE`.
- **`test_pico8_mx`** (`:399`, `:786-787`), **`test_pico8_nr`** (`:368`, `:719-720`), **`test_pico8_hj`** (`:389`) : `class_function(pico8_jo_class()/pico8_qh_class(), <méthode de jo>)`, qui asserte la présence.
- **Appels de fonctions de `jo` avec un cast d'instance** : `planet_gh_row_lq:113`, `rich_planet_exports:43-86,106-140`, `planet_kj_dispatch:53` (régression, voir Implementation Decisions).

**Outils** :
- `tools/float_audit_c1.txt` nomme 5 sites C1 de `pico8_ga_kj` et 4 de `pico8_jo_kj` par **l'extrait textuel** de l'argument (`field_fixed(self, "mi")`, `s_val.as.fixed`, `k_entry.as.fixed`…) : un extrait réécrit doit être réécrit à l'identique dans le fichier.
- `tools/ref_audit_report.sh` lance `test_pico8_ga_export_panel_by_refaudit`.

**Durée** : `geoharvest_burst_events` fait 293 frames ; `session_events` (116 045 frames) est déjà rejoué par `test_pico8_jo`/`pm` (`UNIT_TESTS_SLOW`).

## Solution

1. **Enum : inchangée.**
2. **Struct `ga`**, publique dans le `.h` de la famille : `pico8_jo_t` en premier membre, puis `fz`, `fc`, `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms`, `qy`. Aucun membre ne répète un nom de `jo` ou `ke`.
3. **Défauts pris par REF, jamais re-parsés** : `pico8_planet_family_init` déclare `ga` comme aujourd'hui (même spec, même parent, même nom de couverture, même `register_named`), **garde l'échange de `by`** (ob_table → liste, ob_table libérée), pose `pico8_class_set_struct_new(g_ga_class, …)`, puis lit **une fois**, **après l'échange**, les 15 défauts vers un état statique `g_ga_defaults`, chaque entrée à l'offset de son membre (`nu`, `hf`, `by`, `kq`, `fa` → membres de `jo`). Le shutdown remet cet état à nil avant de libérer les classes, sans libérer aucune table.
   - **`by`** : l'entrée statique **est** l'état partagé de la classe (§5, défaut modifié en place). C'est la même REF que `ga->defaults["by"]`, et chaque `ga` neuf la reçoit dans `jo.by`. Même traitement que `kw` de `jo` à 1a.17.
4. **Constructeur de `ga`**, dans l'ordre du walk du cart :
   1. alloue un `pico8_ga_t` ;
   2. `pico8_jo_struct_init(&self->jo, &g_ga_struct_class)` ;
   3. met les 11 membres propres à nil, puis copie les 15 défauts statiques (ils écrasent ceux de `jo`) ;
   4. recopie chaque clé de construction présente parmi les **11 clés mesurées** (`p`, `ra`, `id` → membres de `jo` ; `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms` → membres propres). Il **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise, et ne garde aucun pointeur sur la table ;
   5. enregistre `"ga"` au crochet de couverture ;
   6. `pico8_ga_pq` (**après** les clés), puis `pico8_jo_pq`, puis `pico8_ke_struct_pq`.
5. **Descripteur de `ga`** (`static const`) :
   - 12 entrées : les 11 de `jo` (**mêmes pointeurs**) sauf `KJ` = le dispatcher de `ga:kj`, plus `MM` = `pico8_ga_mm` ; les autres sont NULL ;
   - **lecteur chaîné** : les 11 membres propres (nil = absent), sinon le lecteur de `jo` (`pico8_jo_struct_class()->field`) ;
   - **écrivain chaîné** : `qy` dans le membre propre, sinon l'écrivain de `jo`. Il accepte 18 noms et refuse les 10 autres membres propres ;
   - accesseur public `pico8_ga_struct_class()`.
6. **Méthodes propres de `ga` à forme struct seule** :
   - `pico8_ga_pq(pico8_ga_t *self)` : `f_`/`ez` reçoivent `self` casté, et `mo.ma` passe inchangé par l'API d'entité (`mo` n'est pas `self`) ;
   - `pico8_ga_mm` : même signature de méthode, lit `ms`/`iu`/`py` dans les membres ;
   - `pico8_ga_kj(…, pico8_ga_t *self, …)` : lit `qy` et écrit `p_` directement. **La couleur reste lue dans le `k_` de `g_ga_defaults`**, la REF de la classe, jamais dans `self->k_`. Un `/* TROU-21 */` nu est posé au site. `ei` reçoit `self` casté.
   - **retirés** : `pico8_class_set_pq(g_ga_class, …)` et les installations `mm`/`kj` de `g_ga_class->defaults` (précédents `pm:pq` 1a.22, `mx:kf` 1a.23). La classe `ga` ne garde que ses défauts de spec (`by` échangé).
7. **Fermeture de `jo`** (règle « Méthodes d'une parente convertie », `ga` dernière descendante) :
   - **retirés** de `pico8_jo_init` : `pico8_class_set_pq(g_jo_class, pico8_jo_pq)` et les 11 installations `hd`/`gg`/`hb`/`gv`/`hk`/`ldown`/`kf`/`cn`/`ib`/`kj`/`ll`. La classe `jo` reste enregistrée (§5, jusqu'à 1a.42) avec ses défauts de spec ;
   - **les 17 fonctions de `jo`** (`pq`, `lz`, `gg`, `kd`, `ll`, `hu`, `hh`, `hk`, `ldown`, `hb`, `gh`, `gv`, `fe`, `kj`, `kf`, `cn`, `ib`) prennent `self` typé `pico8_jo_t *` (`const` là où il l'est aujourd'hui) :
     - les membres de `jo` et de `ke` se lisent et s'écrivent **directement** (`self->kw`, `self->ke.me`…) ;
     - un nom qui n'est membre ni de `jo` ni de `ke` (`hp`, `px`, `s`, `ba`) se lit par le lecteur du descripteur (`pico8_entity_get_field(self, …)`) ;
     - **mêmes gardes**, même sémantique de nil (membre nil = absent) ;
   - les paramètres autres que `self` (`hp_instance`, `s`, `sw`, `lw`, le `p` du `pt` de `fe`/`hu`) **gardent leur type et l'API à double forme** ;
   - les dispatchers castent vers `pico8_jo_t *` ; leurs signatures ne changent pas ;
   - `pico8_jo_hu` construit toujours par `pico8_class_lookup_named` + `pico8_class_instantiate` (`kv[hp.cw]`, supprimé en 1a.42) ;
   - les appelants hors de `jo` (`score_panel.c`, `wormhole.c`, les constructeurs de la famille, `hj:kj`, `mx:kf`) passent leur struct avec un cast ou `&x->…jo`, **sans autre changement**.
8. **`pico8_ga_new`** rend `pico8_ga_t *`, même paramètre. Son commentaire dit le contrat réel : `overrides` est lu, pas gardé, et seules les 11 clés mesurées sont acceptées.
9. **Tests** :
   - nouveau `test_pico8_ga` (caractérisation dirigée et rejeu court) ;
   - `ga_export_panel_by` et `geoharvest_burst` réécrits (§6) ;
   - `test_pico8_jo`, `test_pico8_qb`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_nr`, `test_pico8_hj` : changements de forme seulement ;
   - `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch` : type de pointeur seulement.

   Voir Testing Decisions.
10. **Plan** : à la ligne 1a.26 de `docs/plan-3.md`, ajouter « Découvert à 1a.26 : … » avec au minimum :
    - 10 défauts sans membre chez un ancêtre et `qy` (`f_`) → membres de `ga` ;
    - `nu`/`hf`/`by`/`kq`/`fa` rangés dans les membres de `jo` ;
    - `by` : état statique = même REF que la classe, après l'échange en liste ;
    - `k_` de `ga:kj` lu dans l'état statique (TROU-21) ;
    - descripteur aux 11 entrées de `jo` sauf `KJ`, plus `MM` ; lecteur et écrivain chaînés (`qy`, puis `jo` ; 18 noms) ;
    - 11 clés de construction (deux voies) ; constructeur `ga:pq`, `jo:pq`, `ke:pq`, clés avant `ga:pq` ;
    - `ga:pq`/`mm`/`kj` struct seules, retirées de la classe ;
    - 17 fonctions de `jo` retypées `pico8_jo_t *`, `pq` et 11 installations retirés de la classe `jo` ;
    - `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_`, `na`, `gk` restent à double forme (`g_`, `fv`) ;
    - les tests reconnaissent `ga` par son descripteur.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `ga` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `g_`, `fv`, `pj`, `ui` à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `ga` mesurée (constructions et clés par voie, défauts, champs écrits, noms lus, méthodes résolues, qui atteint encore `jo->defaults`/`ga->defaults`), afin que chaque site touché et chaque retrait soit vérifiable.
3. En tant que lecteur du code, je veux une struct `ga` dont le premier membre est `pico8_jo_t` et qui n'ajoute que les 11 noms mesurés, minifiés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun membre de `ga` ne répète un nom de `jo` ou `ke`, afin que le lecteur de `jo` voie `fa`=3, `hf`=faux et le `nu` vide d'un éclat.
5. En tant que gardien du filet, je veux qu'un éclat n'entre dans aucun `cg` (son `nu` est vide) et soit enregistré dans `hv` au même rang, afin que `dg`, `dc` et le survol le traitent comme avant.
6. En tant que gardien du filet, je veux qu'un éclat construit par la récolte (`jo:hu`) reçoive `p`, `ra`, `id` de la planète qu'il remplace, afin que `geoharvest_burst` et `events_recorded` restent identiques au pixel près.
7. En tant que gardien du filet, je veux qu'un éclat engendré par une protoétoile reçoive `ra`=`ra+ng` et ses 8 clés d'émetteur **avant** `ga:pq`, afin que ses 140 particules initiales suivent `py`=9, `ms`=1.5, `iu`=-0.009.
8. En tant que mainteneur, je veux qu'une clé de construction hors des 11 mesurées déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt (§7).
9. En tant que gardien du filet, je veux que le constructeur appelle `ga:pq`, puis `jo:pq`, puis `ke:pq`, afin que `qy` soit rempli, `mo.ma` mis à 3 et `oe` créé avant l'enregistrement.
10. En tant que gardien du filet, je veux que `ga:mm` rende une particule `{p, v=p*ms, a=p*iu, l=rnd(py)+py}` aux mêmes tirages, afin que l'état du générateur reste identique.
11. En tant que gardien du filet, je veux que `ga:kj` trace ses trois cercles dans la couleur du `k_` **de la classe** (TROU-21), et que `ei` peigne ses points dans celle de l'instance, afin que l'écart connu soit préservé au pixel près.
12. En tant que gardien du filet, je veux que `p_` suive le troisième emplacement de `qy`, afin que l'éclat meure quand il reste moins de 3 particules.
13. En tant que mainteneur, je veux que `by` d'un éclat soit la liste partagée de la classe, réécrite une fois à l'init, afin que l'ajout en place de `jo:ib` et le panneau « exports: » se comportent comme avant.
14. En tant que mainteneur, je veux que `nu`, `kq`, `by`, `k_` d'un éclat neuf soient les REF de `ga->defaults`, `kw`/`em`/`ka`/`dz`/`cq`/`gq` celles de `jo`, et le `k_` d'un éclat de protoétoile la table de `pm:pq`, afin qu'aucune table ne soit re-parsée ni dupliquée.
15. En tant que mainteneur, je veux les 12 entrées du descripteur : 10 mêmes pointeurs que `jo`, plus `KJ`/`MM` propres, afin qu'un éclat exécute `jo:hd`/`hb`/`hk` comme avant.
16. En tant que mainteneur, je veux que l'écrivain de `ga` accepte `qy` et les 17 noms de `jo` et refuse ses 10 autres membres, afin qu'un nom hors des 18 soit un arrêt.
17. En tant que mainteneur, je veux que `ga:pq`, `ga:mm` et `ga:kj` lisent et écrivent `self` directement, et qu'ils quittent la classe `ga`, afin qu'aucune installation n'y survive sans lecteur.
18. En tant que mainteneur, je veux que les 17 fonctions de `jo` prennent un `pico8_jo_t *`, maintenant que toute planète est une struct qui commence par `jo`, afin que la dette posée à 1a.16b et 1a.17 soit soldée.
19. En tant que mainteneur, je veux que ces fonctions gardent leurs gardes et l'API à double forme pour tout ce qui n'est pas `self`, et lisent par le descripteur les noms de `qh` (`hp`, `px`, `s`, `ba`), afin qu'une recette, un `jc`, un `mz`, un `bg` ou une fille de `qh` se lisent comme avant.
20. En tant que mainteneur, je veux que le `pq` de classe et les 11 installations de `jo` soient retirés, afin que la classe `jo` ne garde que ses défauts de spec.
21. En tant que mainteneur, je veux que `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_`, `na`, `gk`, `gd`, `ma()`, `ke_ks` gardent corps et signature, afin que `g_` et `fv`, encore instances, émettent et se sondent comme avant.
22. En tant que gardien de la couverture, je veux que le constructeur enregistre `"ga"` une fois par éclat, afin que `class-coverage-net` soit identique ligne à ligne (`ga 7 3/15`).
23. En tant que vérificateur, je veux une caractérisation de `ga` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (voie `pico8_ga_new(NULL)`, palette TROU-21, identité des tables, `hb`/`hk` sur un éclat).
24. En tant que vérificateur, je veux que `ga_export_panel_by` échoue encore si l'échange de `by` en liste est retiré, et que `geoharvest_burst` échoue encore si la récolte ne laisse pas un éclat qui meurt à la frame attendue, afin que §6 tienne.
25. En tant que vérificateur, je veux un rapport `gcov` montrant chaque ligne ajoutée ou modifiée exécutée, et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
26. En tant que gardien des caractérisations, je veux que `test_pico8_jo`, `qb`, `pm`, `mx`, `nr`, `hj` ne changent que leur façon de reconnaître un `ga` ou de nommer une méthode de `jo`, sans valeur figée touchée, afin que 1a.8 à 1a.25 restent des preuves.
27. En tant que gardien des tests de régression, je veux que `planet_gh_row_lq`, `rich_planet_exports` et `planet_kj_dispatch` ne changent que le type de pointeur passé à une fonction de `jo`, afin que leur intention reste intacte.
28. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne des références de couverture ni aucune entrée ajoutée ou retirée de `float_audit_c1.txt` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
29. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.26, afin que la liste des étapes dise ce que l'étape a réellement touché.
30. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C, un piège ou un `TROU-N` nu (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
31. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
32. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** :
  - la famille de planètes : struct, défauts, constructeur, descripteur, méthodes propres de `ga`, `pico8_ga_new`, appel de `pm:pq`, casts vers `jo:pq`/`jo:kj` ;
  - son module `_trade` : casts vers `jo:pq`/`jo:kf` ;
  - le module planète : init de `jo`, dispatchers, 9 fonctions ;
  - `planet_display` : 5 fonctions ;
  - `planet_trade` : 3 fonctions ;
  - `score_panel` et `wormhole` : un cast chacun.

  `pico8_struct.h` (enum), `particle`, `probe`, `help`, `entity*`, `economy*`, `research*`, `msgbox`, `cart` : aucun changement.
- **11 membres propres** : `fz`, `fc`, `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms` sont des défauts de la chaîne sans membre chez un ancêtre (§5) ; `qy` est écrit par le `pq` propre (`f_`), précédent `hj`. Ils ne remontent pas dans `jo` : `ga` est la seule fille de `jo` à les avoir hors de `hj`, et la phase 2 en fera un composant (§5, phase 2, règle 2).
- **Défauts rangés dans les membres de `jo` quand le nom existe** : même piège qu'à 1a.18-1a.25. Un `ga.fa` propre laisserait `jo.fa`=6 visible à `ei`, à `gd` et au survol.
- **`by` : état statique partagé, même REF que la classe, lue après l'échange** (§5, défaut modifié en place ; précédent `kw` de `jo`). L'échange ob_table → liste reste à l'init. Rejetés : une liste neuve par éclat (cela changerait l'ajout en place de `jo:ib`) et une liste statique hors de la classe (deux REF).
- **TROU-21 préservé à l'identique** : `ga:kj` lit le `k_` de `g_ga_defaults` (la REF de la classe), pas `self->k_`. Un `/* TROU-21 */` nu est posé au site (G) : l'entrée existe déjà dans `docs/trous-de-portage.md` et nomme ce fichier. Il n'y a aucune nouvelle entrée `TROU`.
- **Descripteur `static const`** : les 10 entrées de `jo` héritées par nom (le cart n'en surcharge aucune), `KJ` propre, `MM` propre. `ga` n'a ni `HQ`/`EV`/`EY`/`LY` : la sonde les lit absents (`ev`).
- **Écrivain chaîné, pas le même pointeur que `jo`** (G) : `f_` écrit `qy` sur un éclat (mesuré), précédent `hj`. Rejeté : mettre `qy` dans `jo` (ce n'est pas un écrivain générique de `cg.jo`).
- **Onze clés, deux voies, chacune facultative** (G) : mesurées par la sonde. `pico8_ga_new(NULL)` (test) passe sans clé. Les clés sont posées avant `ga:pq`, comme dans le cart (`mw(ob)` avant le walk) : c'est la différence avec `pm`, dont le `pq` propre ne lit rien de ses défauts d'émetteur.
- **Constructeur `ga:pq` → `jo:pq` → `ke:pq`**, l'ordre du walk du cart. `ke:pq` avant `jo:pq` est un mutant équivalent (1a.23). Les mutations d'ordre porteuses sont « clés posées après `ga:pq` » et « défauts copiés après `ga:pq` ».
- **Méthodes propres de `ga` retirées de la classe** : seule la classe `ga` les résout (mesuré) et elle n'a pas de descendante. Précédents `pm:pq`, `mx:kf`.
- **Fermeture de `jo` : `pq` et 11 installations retirés** : la sonde montre que seul `ga` résout une fonction dans `jo->defaults` et atteint le `pq` de classe de `jo`.
- **Les 17 fonctions de `jo` retypées `pico8_jo_t *`** : c'est la règle du §5 (« pour `jo` : 1a.26 »), et le PRD de 1a.17 le renvoyait ici explicitement (« le retypage de leurs signatures (1a.26) »). Le type ne ment plus pour une fille : toute planète commence par `pico8_jo_t`.
- **Noms de `qh` lus par le descripteur** (G) : `hp`, `px`, `s`, `ba` ne sont pas des membres de `jo`. Les lire par `pico8_entity_get_field(self, …)` reste exact sur toute fille. Rejeté : caster vers `pico8_qh_t *` (faux pour un `jo` nu ou un `ga`).
- **`self` seul retypé** (G) : `hp_instance` (une recette `hp`, instance jusqu'à 1a.33), le `s` de `gv` (un `jc`, un `mz`, un `bg`), le `sw` de `kf`, le `lw` de `cn` et les voisins gardent leur type et l'API à double forme. C'est le changement chirurgical, précédent 1a.25.
- **Dispatchers de `jo` : signatures inchangées**, cast interne vers `pico8_jo_t *`. Les descripteurs des filles les nomment déjà.
- **Tests de régression à diff de type seulement** (G) : `planet_gh_row_lq` (`(const pico8_instance_t *)planet` → la struct), `rich_planet_exports` (`planet_by_id` et `promote_and_open_panel` typés sur la struct, ou cast au site d'appel) et `planet_kj_dispatch` (`(pico8_instance_t *)self` → `(pico8_jo_t *)self`). Ils ne compileraient plus sous `-Werror`, et ne sont pas nommés par la ligne du plan. Le changement porte sur la façon de passer l'état, pas sur ce qu'on lit ou asserte (§6). Leur preuve rouge date de 1a.17 : elle n'est pas refaite. Le vérificateur contrôle que leur diff se limite à ces types et casts.
- **`ga_export_panel_by` : écritures directes des membres** (G). Le test posait `kw`, `ra`, `gq` dans `planet->fields`. Il les pose dans `planet->jo.kw`/`ra`/`gq`, car l'écrivain de `ga` (celui de `jo`) refuse `ra` et `gq`, qui ne sont pas écrits par le jeu sur une planète. `by` est toujours lu **par l'interface de champ** (`pico8_entity_get_field`), comme son commentaire le demande. L'assertion de tag `LIST` et les deux assertions de panneau sont inchangées.
- **`geoharvest_burst`** : `live_ga` reconnaît un éclat par son descripteur (`pico8_ga_struct_class()`, après vérification du tag) et `particle_count` lit `qy` par `pico8_entity_get_field`. Frames, assertions et messages sont inchangés.
- **`float_audit_c1.txt`** (G) : si le retypage réécrit l'argument d'un `pico8_fixed_flr_int` listé (par exemple `field_fixed(self, "mi")` dans `pico8_jo_kj`), l'extrait de la ligne correspondante est réécrit **dans le même commit**, sans ajout ni retrait de ligne, et `make float-audit` reste vert. Pour garder le diff minimal, l'implémenteur peut aussi laisser l'argument textuellement identique.
- **`test_pico8_jo` : la branche instance de `daughter_of` disparaît** (G). Plus aucune fille n'est instance : `ga` est reconnu par son descripteur, et le tableau `classes`, devenu inutile (erreur `-Werror`), est retiré.
- **Sémantique de nil** : les membres nil sont absents ; `hp`, `jy`, `kr` sont lus absents sur un éclat sous les deux formes, et `p_`, `lq`, `ma` sont présents une fois écrits. TROU-9 n'intervient pas (aucun nil écrit sur un éclat, mesuré).
- **Clés dynamiques** : aucune.
- **Commentaires** (CLAUDE.md) :
  - à ajouter :
    - le contrat de la struct `ga` : jamais libérée (TROU-7) ; un défaut dont le nom est un membre de `jo` va dans ce membre ; `nu`/`kq`/`by`/`k_` sont les tables de la classe `ga`, empruntées, et `by` est la liste partagée qu'`ib` étend en place ; le `k_` d'un éclat de protoétoile est celui de `pm:pq` ;
    - l'assert de clé ;
    - le contrat de `pico8_ga_new` ;
    - `/* TROU-21 */` au site de couleur ;
  - à corriger ou retirer si rendus faux :
    - `pico8_planet.c:305-308` (« they stay until jo's last daughter is a struct (plan-3 1a.26) ») ;
    - le commentaire de `pico8_ga_new` (« `overrides` is copied ») ;
    - le commentaire de `pico8_jo_pq` (« the effective pq of a daughter that has none of its own ») ;
    - celui des dispatchers (« The dispatchers jo's descriptor and its class `defaults` hold ») ;
    - le commentaire de `pico8_planet_family_shutdown` s'il ne compte plus bien ;
    - dans les tests : « `ga` stays an instance » (`test_pico8_pm`, `test_pico8_mx`), l'en-tête de `test_pico8_jo` (« Its unconverted daughters »), « the daughters other than `qh` stay instances » ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert (et `make e2e` au dernier). La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_ga` sur l'ancien code** (voir Testing Decisions), valeurs figées. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si un éclat de récolte n'a pas `fa`=3, `ms`=3, `hf` faux, ou si un éclat de protoétoile n'a pas `ms`=1.5 : la mesure est fausse et doit être corrigée dans le PRD.
2. **Tests retargetés, verts sur l'ancien code** : chaque comparaison à `class_function`/`class_value`/`pico8_class_get_field(pico8_jo_class() ou pico8_qh_class(), <méthode de jo>)` vise l'entrée correspondante de `pico8_jo_struct_class()`. C'est le même pointeur sur l'ancien code, donc aucune empreinte ne bouge.
   - Fichiers : `test_pico8_mx:399`, `test_pico8_nr:368`, `test_pico8_hj:389`, `test_pico8_jo` `fold_planet:302-307`.
   - Les assertions devenues tautologiques sont retirées.
   - Suite verte.
3. **`ga` converti** :
   - struct, statiques de défauts, constructeur, descripteur, lecteur, écrivain, accesseur, `pico8_class_set_struct_new`, `pico8_ga_new` typé ;
   - `ga:pq`/`mm`/`kj` struct seules et retirées de la classe ; `pm:pq` adapté ;
   - `ga_export_panel_by` et `geoharvest_burst` réécrits, **preuves rouges** faites (voir Testing) ;
   - `test_pico8_qb` (`ga_alive` par descripteur), `test_pico8_jo` (`daughter_of`), `test_pico8_pm`/`mx` (`ga` quitte `others` ; le tableau vide et sa boucle disparaissent).

   Toutes les caractérisations passent **sans changer leurs valeurs figées**. Les vérifications de forme de `ga` s'ajoutent à `test_pico8_ga`.
4. **Fermeture de `jo`, classe** : `pq` et 11 installations retirés de `pico8_jo_init`.
   - `test_jo_is_a_struct` (`:706-710`) attend les 11 noms **absents** de `jo->defaults` (lecture directe de la table) et `pico8_jo_class()->pq == NULL`.
   - `test_pico8_mx:786-787` et `test_pico8_nr:719-720` attendent la méthode absente de toute la chaîne de classes.
   - Valeurs figées identiques.
5. **Fermeture de `jo`, fonctions** : les 9 fonctions de `pico8_planet.c`, puis les 5 de `planet_display`, puis les 3 de `planet_trade`, retypées `pico8_jo_t *` avec lecture directe des membres. Les appelants sont adaptés par cast.
   - `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch` : type de pointeur seulement.
   - `float_audit_c1.txt` : extraits réécrits si besoin.
   - `make test` vert après chaque fichier.
6. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.26 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF et des pointeurs de fonction, `p_`, `qy`, `mo.ma`, le panneau de `jo:gh`. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_ga`** : lié comme `test_pico8_fr`, sur le cart réel, avec une dépendance Makefile sur `geoharvest_burst_events.json` et les fixtures qu'il lit.
  - Il lit **uniquement** par l'API à double forme et par `pico8_class_get_field(pico8_ga_class()/pico8_jo_class()/pico8_pm_class(), …)` pour les **défauts de spec**, jamais pour une méthode (les installations disparaissent aux morceaux 3-4).
  - Il nomme les méthodes de `jo` par `pico8_jo_struct_class()`.
  - Son code est le même avant et après, aux vérifications de forme près.
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()`, pour l'état de `rnd`.
  - Toute présence se lit par un `lookup` qui confond nil et absent.
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **Construction, trois voies**, partie lancée comme dans `test_pico8_fr` :
       - (a) une planète construite par `pico8_jo_new` (`id`/`p`/`ra`) puis `pico8_jo_hu` avec la recette de `gb` dont `cw` = `"ga"`. L'éclat est le nouvel élément de `hv` ;
       - (b) `kv["pm"]`/`pico8_pm_new` ; l'éclat est l'élément qui précède la protoétoile dans `hv` ;
       - (c) `pico8_ga_new(NULL)`.

       Pour chacune, on prend :
       - l'empreinte de `fz`, `fa`, `fc`, `qo`, `ro`, `qg`, `q_`, `iu`, `py`, `ms`, `hf`, `me`, `t`, `id`, `ra`, `p`, de la taille de `k_`, `nu`, `by`, `qy` et de `mo.ma` ;
       - la présence des 11 membres propres et des 31 de `jo` selon leur état ;
       - l'absence de `hp`, `jy`, `kr`, `qu`, `ht`, `et`, `ng`, `du` ;
       - les 12 méthodes présentes, **les 10 héritées égales aux entrées de `pico8_jo_struct_class()`**, `kj`/`mm` différentes, les autres absentes ;
       - le rang dans `hv`, l'absence de tout `cg` (`jo`, `np`, `prober`, `hj`, `qh`) ;
       - l'**identité** :
         - `nu`, `kq`, `by`, `k_` sont les REF de la classe `ga` pour (a) et (c) ;
         - le `k_` de (b) n'est pas celui de la classe ;
         - `by` est la même REF pour les trois éclats et `pico8_type_tag_of(by)` vaut `LIST` ;
         - `kw`, `em`, `ka`, `dz`, `cq`, `gq` sont les REF de la classe `jo`.

       Figée.
    2. **Comportement** :
       - une particule rendue par `mv(e,"mm")` (champs `p`, `v`, `a`, `l` pliés) ;
       - `dc("kj")` sur chaque voie, avec l'empreinte d'écran, dont **la couleur des cercles d'un éclat de protoétoile, prise dans le `k_` de la classe** (TROU-21), et les points d'`ei` dans celle de l'instance ;
       - `p_` après `kj` avec `qy` réduit en place à 3, puis 2, puis 0 éléments : faux, vrai, vrai ;
       - `hd` en deux points ; `gg` (faux) ;
       - `hb` via `mv` : `lq` construit, puis `by` étendu de deux entrées en place, puis `hb` (le panneau reflète la liste partagée) ;
       - `hk` (sans `kr` : rien).

       Figés.
    3. **Rejeu court `geoharvest_burst_events`** (293 frames, départ `eu(bo[2])` comme le test de régression) : à chaque frame où un éclat vit, on plie son rang, la taille de `qy`, `p_`, `t`, `me`, `ra` et `mo.ma`. S'y ajoutent la frame de naissance, la frame de mort et une empreinte finale. Figée.
  - **Pas de rejeu long dans `test_pico8_ga`** (G). Les 6 éclats de `session_events`/`events_recorded` sont couverts par `frames-check` (pixels), par `test_pico8_jo` (premier `ga` de `session` plié) et par `test_pico8_pm` (`fold_spawned`). Un rejeu de 116 045 frames de plus ne prouverait que ce que la caractérisation dirigée fige déjà pour la voie `pm`.
  - **Forme** (ajoutée aux morceaux 3-4) :
    - un éclat de chaque voie est `STRUCT` au descripteur de `ga` ;
    - ses entrées valent celles de `jo` sauf `KJ`/`MM`, les autres sont NULL ;
    - `cls->field` et `cls->set_field` sont propres ;
    - le lecteur de `jo` voit sur un éclat les 5 défauts rangés dans ses membres (`nu`, `hf`, `by`, `kq`, `fa`) ;
    - `pico8_entity_set_field` passe pour les 18 noms et asserte (dans un fils) pour `fz`, `k_`, `ms`, `py`, `hp`, `ra`, `id`, `ht` ;
    - une clé de construction hors des 11, ou positionnelle, asserte ;
    - `pico8_ga_class()->pq == NULL` et `->struct_new` est posé ;
    - `mm`/`kj` sont absents de `ga->defaults`, les 11 méthodes de `jo` absentes de `jo->defaults`, `pico8_jo_class()->pq == NULL` (lecture directe de la table).
- **Tests de régression (§6)** :
  - **`ga_export_panel_by`** : `pico8_ga_new(NULL)` rend la struct ; `kw`/`ra`/`gq` posés dans les membres ; `by` lu par `pico8_entity_get_field` ; `pico8_jo_gh(&planet->jo)`. Intention inchangée : le contrat « `by` est une vraie liste » est asserté sur la REF, entre « 0/2 » et « 2/2 ».
    - **Preuve rouge** : retirer localement l'échange de `by` à l'init (la classe garde l'ob_table, et l'état statique aussi). Le test doit échouer sur l'assertion de tag. Puis restaurer.
  - **`geoharvest_burst`** : `live_ga` par descripteur, `qy` par `pico8_entity_get_field`. Intention inchangée : la récolte laisse un éclat de ≥ 3 particules qui meurt à la frame 289.
    - **Preuve rouge** : faire suivre `p_` au **dernier emplacement non vide** au lieu du troisième, ou le faire devenir vrai seulement quand `qy` est vide (la « correction » que le commentaire du site interdit). L'éclat doit être encore vivant à la frame 289, et le test rouge.
    - Si la mesure montre que ce mutant est équivalent sur ce rejeu, la preuve rouge devient « `ga:pq` ne remplit pas `qy` » (assertion de la frame 205). Le mutant retenu est noté dans le commit.
    - Puis restaurer.
  - **`planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`** : diff limité au type de pointeur passé à une fonction de `jo`. Pas de preuve rouge due (inchangés dans ce qu'ils lisent et assertent, prouvés à 1a.17).
  - Tous les autres tests de régression : diff vide, verts.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (`git archive` ou copie de l'arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` `test_pico8_ga`, `test_pico8_jo`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_nr`, `test_pico8_hj` et les 5 tests de régression touchés, avec les objets du moteur. Les lancer, puis `gcov` sur la famille, `planet`, `planet_display`, `planet_trade`, `score_panel` et `wormhole`.
    - Même piège qu'à 1a.23-1a.25 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`.
    - Les branches d'assert atteintes seulement dans un fils qui avorte (`must_abort`) ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu et justifiées ainsi.
    - Le rapport est collé dans le message de commit. Il donne les lignes exécutables ajoutées ou modifiées de `src/` et leur compte, en particulier :
      - chaque fonction de `jo` retypée ;
      - le constructeur de `ga` par voie (clés `p`/`id` et les 8 clés d'émetteur) ;
      - `ga:kj` avec `qy` à 3 et à moins de 3 ;
      - les lignes à 0, avec leur justification.
  - **2 à 3 mutations**, chacune rouge sur `test_pico8_ga` (ou un autre test nommé), notées dans le commit avec le test qui l'attrape :
    1. **valeur d'un défaut propre** : `ms` (ou `iu`) de `ga` non copié, ou copié hors du membre lu par `ga:mm` : les particules changent ;
    2. **entrée du descripteur remplacée** : `MM` = `pico8_g__mm`, ou `KJ` = `pico8_jo_kj_dispatch` ;
    3. **ordre du constructeur** : clés de construction posées **après** `pico8_ga_pq` (les 140 particules d'un éclat de protoétoile suivent alors `py`=6, `ms`=3). Pas `ke:pq` avant `jo:pq` (équivalent, 1a.23).

    Mutation facultative, utile : `ga:kj` lit `self->k_` (TROU-21 corrigé en douce). Elle doit être rouge sur la couleur des cercles d'un éclat de protoétoile.
- **Tests retargetés ou reconnus par descripteur** :
  - `test_pico8_jo` : `daughter_of`, `fold_planet` ; `test_jo_is_a_struct` attend les installations absentes et le `pq` de classe NULL, et son commentaire est corrigé ;
  - `test_pico8_qb` : `ga_alive` ;
  - `test_pico8_pm`, `test_pico8_mx` : `others` retiré ; `mx:399`, `mx:786-787` ;
  - `test_pico8_nr` : `:368`, `:719-720` ;
  - `test_pico8_hj` : `:389`.

  Aucune empreinte figée ne bouge : `session`/`recorded`/`hub`/`long` de `test_pico8_jo`, les empreintes de `pm`, `mx`, `nr`, `hj`, `qb`.
- **Tests existants à diff vide** : `test_pico8_qh`, `nl`, `os`, `fr`, `ie`, `mz` et `bg` (ils passent des `void *` à `pico8_jo_gv`), `jc`, `nf`, `js`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage`.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** :
  - `test_pico8_fr` : structure, `lookup`, `must_abort`, construction dirigée, lecture directe des `->defaults` pour les formes ;
  - `test_pico8_mx` à 1a.23 : méthode propre retirée de la classe, tests retargetés vers le descripteur ;
  - `test_pico8_pm` : `fold_spawned`, reconnaissance d'un éclat par `fz` ;
  - `test_pico8_geoharvest_burst` : départ `eu(bo[2])` et frames du rejeu court.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; c'est le seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` (en particulier `ga  7  3/15`, `jo  1710  15/15`, `pm  4  2/15`). Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations (défaut propre, entrée du descripteur, ordre du constructeur) ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier :
  - si l'assertion de clé ou celle de l'écrivain se déclenche sur un rejeu ou un test ;
  - si un membre propre autre que les 11 est ajouté, ou si un membre propre répète un nom de `jo`/`ke` ;
  - si l'écrivain de `ga` accepte autre chose que `qy` et les 17 noms de `jo`, ou si la liste des 17 noms de `jo` change ;
  - si l'enum change.
- `FAILED:` si une autre classe que `ga` est convertie ou change de forme (`g_`, `fv`, `pj`, `ui`, `hp`, `lq`, …), ou si le comportement de `jo` ou d'une autre fille change (valeur figée de leurs caractérisations, lignes de couverture).
- `FAILED:` si une entrée héritée du descripteur de `ga` n'est pas le même pointeur que l'entrée correspondante de `pico8_jo_struct_class()`, ou si `KJ`/`MM` ne sont pas les méthodes propres de `ga`.
- `FAILED:` dans chacun des cas suivants :
  - `pico8_class_set_pq(g_jo_class, …)`, une installation dans `g_jo_class->defaults`, `pico8_class_set_pq(g_ga_class, …)` ou une installation `mm`/`kj` dans `g_ga_class->defaults` subsiste ;
  - l'échange de `by` en liste est retiré ;
  - une installation `->defaults` de `g_`/`g_probe_class`, `fv`, `ui`, `pj`, `hp` ou `lq` est retirée ou changée.
- `FAILED:` dans chacun des cas suivants :
  - une des 17 fonctions de `jo` garde un `self` typé `pico8_instance_t *`/`void *` ;
  - l'une d'elles lit ou écrit par l'API à double forme un membre de `jo`/`ke` sur `self` ;
  - l'une d'elles change une garde ;
  - un paramètre autre que `self` change de type.
- `FAILED:` si une fonction encore atteinte par une instance change de corps ou devient struct seule : `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_g__mm`, `pico8_bv`/`dl`/`d_` et leurs dispatchers, `pico8_na`, `pico8_qh_ly_dispatch`, `pico8_gk`, `pico8_gd`, `pico8_ke_ks`.
- `FAILED:` si `ga:kj` lit la couleur ailleurs que dans le `k_` de la classe (état statique, même REF que `ga->defaults`) : TROU-21 corrigé au lieu d'être préservé.
- `FAILED:` si une table de défaut de `ga` est re-parsée depuis la spec ; si `by` d'un éclat neuf n'est pas la REF de `ga->defaults["by"]` (liste) ; si `nu`/`kq`/`k_` d'un éclat de récolte ne sont pas celles de `ga->defaults`.
- `FAILED:` si le constructeur de `ga` n'appelle pas `ga:pq`, puis `jo:pq`, puis `ke:pq`, ou s'il pose les clés ou copie les défauts après `ga:pq`.
- `FAILED:` si `ga_export_panel_by` ou `geoharvest_burst` changent d'intention (une assertion de comportement retirée ou affaiblie, une frame changée), ou si leur preuve rouge n'est pas notée dans le commit.
- `FAILED:` si `planet_gh_row_lq`, `rich_planet_exports` ou `planet_kj_dispatch` ont un diff au-delà du type de pointeur passé à une fonction de `jo`, ou si un autre test de régression (§6) a un diff non vide.
- `FAILED:` si une valeur figée de `test_pico8_ga` a été changée après la conversion, ou une de `test_pico8_jo`, `pm`, `mx`, `nr`, `hj`, `qh`, `nl`, `os`, `fr`, `qb`, `ie`, `mz`, `bg`, `jc`, `nf`, `js`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo`. Également si ces caractérisations changent au-delà de la reconnaissance de `ga`, des listes `others`, des comparaisons de méthodes retargetées vers un descripteur et des assertions de forme sur les installations.
- `FAILED:` si une ligne est ajoutée à `tools/float_audit_c1.txt` ou en est retirée, ou si `make float-audit` est rouge.
- `FAILED:` si `test_pico8_ga` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- la struct `ga` n'a que `pico8_jo_t` et les 11 membres ; son descripteur a 12 entrées ;
- grep :
  - `PICO8_CLASS_COVERAGE_NAME(g_ga_class, "ga")` et `pico8_class_register_named("ga", …)` restent ;
  - `pico8_class_set_struct_new(g_ga_class, …)` est présent ;
  - aucun `set_name(g_jo_class->defaults, …)` ni `set_name(g_ga_class->defaults, "mm"|"kj", …)` ;
  - le seul accès restant à `g_ga_class->defaults` hors lecture unique des défauts est l'échange de `by` ;
  - `/* TROU-21 */` est présent au site de couleur de `ga:kj` ;
- aucune fonction `pico8_jo_*` avec `self` n'a encore `pico8_instance_t *self` dans son en-tête ;
- la ligne 1a.26 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.26 ».

## Out of Scope

- `g_` (1a.27 ; ses fonctions `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_` passent à la forme struct seule au commit de sa dernière descendante, `fv`, 1a.28), `pj`, `ui`, `hp`, `lq`, `hy`, et leurs `->defaults`.
- Les écrivains et lecteurs génériques de `cg.jo` hors des fonctions de `jo` (`economy*.c`, `research_display.c`, `score_panel*.c`, `msgbox.c`) : ils restent sur l'API d'entité, valide sur toute struct. Leur typage relève de la phase 1b.
- Le type des paramètres autres que `self` des fonctions de `jo` (`hp_instance` : `hp` en 1a.33 ; `s`, `sw`, `lw`).
- `kv[hp.cw]` (`pico8_class_lookup_named`) et les registres de classes : supprimés en 1a.42.
- Corriger TROU-21 ou TROU-20.
- La mise en composant des champs d'émetteur de `ga`/`hj` (phase 2, règle 2).
- La suppression des `pico8_class_t` de `ga` et `jo` (1a.42).
- Le typage C des membres (1b).
- Libérer un éclat ou sa liste `qy` (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Fermeture de la dernière famille de planètes** : après ce commit, aucune instance de la couche objet ne descend de `jo`, et la classe `jo` ne porte plus que ses défauts de spec. La dette posée à 1a.16b-1a.17 (17 fonctions à double forme, 11 installations et un `pq` de classe gardés pour les filles) se solde ici. Il reste des instances sous `ke` : `g_`, `fv`, `pj`, `ui`.
- **Taille** : le morceau 5 est le plus gros diff du commit (trois fichiers, environ 60 sites d'accès sur `self`) mais le plus mécanique. D'où son découpage par fichier, `make test` vert entre chaque. L'ordre des morceaux 3 → 4 → 5 permet de s'arrêter proprement si la conversion de `ga` révèle un écart avant qu'on touche aux signatures de `jo`.
- **Nouveau cas par rapport à 1a.25** : les clés de construction d'une voie (`pm:pq`) pilotent le `pq` propre (`ga:mm` pendant `ez(140)`), d'où l'ordre « clés avant `ga:pq` » et la mutation qui le vérifie.
- **Mesure utile pour 1a.27/1a.28** : `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_` sont atteints par `g_` **et par `fv`** (fille de `g_` : `fv:kj` → `ei` ; `g_probe_class->defaults` hérités). Leur passage à la forme struct seule revient donc au commit de `fv` (1a.28), pas de `g_`.
- **Leçons de 1a.23-1a.25 appliquées** : comparaisons de méthodes retargetées vers le descripteur avant la conversion (vertes sur l'ancien code), mutation d'ordre sur ce qui compte (clés, défauts), pas sur `jo:pq`/`ke:pq`, et piège `_exit` des fils réglé dans la copie jetable seulement.
- Mesures :
  - grep sur `src/`, `tests/`, `tools/` et le `Makefile` à c3e89b4 ;
  - lecture de `pico8/lua.lua:86-97,448-455,500-514,685-707,727-731,1030-1094,1166-1170` ;
  - lecture de `pico8_planet*.c/h`, `pico8_particle.c`, `pico8_probe.c`, `pico8_help.c`, `pico8_class.c`, `pico8_entity.c`, des tests nommés, de `tools/class_coverage_net_expected.txt`, `tools/float_audit_c1.txt`, de `docs/trous-de-portage.md` (TROU-20, TROU-21) ;
  - lecture du PRD et du message de 3cbb057 (1a.25) et des PRD de 1a.16b, 1a.17, 1a.22, 1a.23 ;
  - sonde de résolution de fonctions et de champs d'un `ga`, dans une copie jetable du scratchpad, supprimée après usage : 18 rejeux en sortie 0 ; 7 `ga` (3 `session`, 3 `recorded`, 1 `geoharvest`) ; seule la classe `ga` atteint `jo->defaults` et `ga->defaults`.
