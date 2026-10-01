---
title: "Plan 3, étape 1a.28 : `fv` (l'écran d'aide) devient une struct fille de `g_` avec 10 membres propres ; dernière descendante de `g_`, elle ferme la classe `g_` (pq et 5 installations retirés) et fait passer `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_`, `gk`, `na` à la forme struct seule"
description: "L'écran d'aide `fv` est construit 19 fois sur le filet, toujours par `pico8_fv_new(NULL)` (`cart.c:159`). Il passe en struct `pico8_fv_t` (`pico8_g__t` puis 9 défauts propres et `m_`), au descripteur de 6 méthodes. Mesuré : c'était la seule instance qui atteignait le `pq` et les installations de la classe `g_`, et la seule à passer un `self` instance à `f_`, `ez`, `ei`, `g_.mm`, `dl` et `gk`. Ce commit retire donc le `pq` et les installations des classes `g_` et `fv`, et fait passer les fonctions partagées de `g_` à la forme struct seule, en nommant l'étape qui fermera chaque double forme restante."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.28 — `fv` : `g_` (`help.c:129`).

Le numéro de ligne tient encore à HEAD 44a51d5. Dans `pico8_help.c` :
- `pico8_help_init` est à `:117` ;
- la déclaration `g_fv_class = pico8_class_declare(pico8_g__class(), …)` est à `:129-135`, son nom de couverture à `:136` ;
- `pico8_class_set_pq(g_fv_class, pico8_fv_pq)` est à `:137`, les 2 installations `hd`/`kj` à `:138-139` ;
- `pico8_fv_hd_dispatch` (static) est à `:56`, `pico8_fv_pq` à `:61`, `pico8_fv_kj` à `:82`, `pico8_fv_kj_dispatch` à `:104` ;
- `pico8_help_shutdown` est à `:144`, `pico8_fv_new` à `:160`.

Ailleurs :
- `cart.c:91-92` appelle `pico8_help_init()` et asserte que le parent de la classe `fv` est la classe `g_` ; `cart.c:159` construit l'aide, `(void)pico8_fv_new(NULL)` ;
- dans `pico8_probe.c`, `pico8_class_set_pq(g_probe_class, pico8_f_)` et les 5 installations `hq`/`ey`/`ev`/`kj`/`mm` de `g_probe_class->defaults` sont à `:511-518`, sous le commentaire « Unused by a probe […] they serve `fv` » ;
- le constructeur de `g_` est à `:453` ; son lecteur `pico8_g__field` (`:405`) et son écrivain `pico8_g__set_field` (`:422`) sont `static`, joignables par `pico8_g__struct_class()->field`/`->set_field` ;
- les fonctions de `g_` : `pico8_bv` `:119`, `pico8_dl` `:195`, `pico8_d_` `:258`, `pico8_g__mm` `:322` (`pico8_probe.c`) ; `pico8_f_` `:60`, `pico8_ez` `:72`, `pico8_ei` `:90` (`pico8_particle.c`) ;
- `pico8_gk` (`entity_geom.c:39`) et `pico8_na` (`entity_geom.c:62`).

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag […].
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre […] reste à double forme […]. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante.
>
> **Accès directs aux défauts** : […] la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct.

`pico8_fv_t` commence par `pico8_g__t`. `fv` est une **feuille** : elle n'a aucune fille. Elle est aussi **la dernière descendante de `g_`**. C'est la situation de `ga` à 1a.26 (dernière fille de `jo`, qui ferme sa parente), avec trois différences :
- la parente `g_` est une racine sous `ke`, pas une famille de planètes : le lecteur et l'écrivain de `fv` se chaînent sur ceux de `g_`, puis de `ke` ;
- les fonctions de `g_` sont **partagées bien au-delà de `g_`** (`qh` et ses filles, `jc`, `mz`, `hj`/`pm`/`mx`, `ga`, la vue `ie.ke`). Aucun type C commun ne couvre tous leurs `self` : la « forme struct seule » ne peut donc pas être un retypage (voir Implementation Decisions) ;
- le `pq` de `fv` (`pz`) construit 4 boutons `js` qui s'enregistrent dans `hv` **avant** l'aide.

Notes du plan qui échoient ici :

- **1a.27** : « le `pq` (`f_`) et les 5 installations `hq`/`ey`/`ev`/`kj`/`mm` de la classe `g_` restent pour `fv` (qui y résout `f_`, `ev`, `mm`, `nu`), retirés à 1a.28 ; `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_` restent à double forme, fermées à 1a.28 pour celles que plus aucune instance n'atteint ; `test_pico8_hj:390,754` compare encore à l'installation `mm`, à retargeter à 1a.28 ; enum inchangée. »
- **1a.26** : « `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_`, `na`, `gk` restent à double forme (`g_`, `fv`) ». La note de 1a.27 a laissé tomber `na` et `gk` sans nommer d'étape de fermeture : ce PRD la nomme (1a.28, mesuré ci-dessous).
- **1a.25** : « `pico8_na`, `pico8_qh_ly_dispatch`, `bv`, `dl`, `d_` restent à double forme ».
- **1a.14** : « `m_` écrit par `gw` → écrivain » (précédent de `ie`, qui porte `hd`).

§6 : la ligne de l'étape ne nomme **aucun** test de régression. Tous les tests de régression doivent garder un diff vide.

§7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` : `fv  19  15/15`, `g_  17  9/15`, toutes les autres lignes inchangées ;
- **preuve des tests par `gcov` et 2 à 3 mutations** ;
- **note « Découvert à 1a.28 »** réduite à ce qui change une étape suivante, avec l'étape qui fermera chaque double forme laissée ouverte ;
- un nouveau test dont le rejeu dépasse 30 s va dans `UNIT_TESTS_SLOW` ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.27 ont posé (jusqu'à ed688a8), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`).
  - L'enum contient déjà `HQ`, `EY`, `EV`, `KJ`, `MM`, `HD` : il ne change pas.
  - `pico8_entity_get_field` rend d'abord l'entrée du descripteur, sinon le lecteur ; un membre nil est absent.
  - L'écrivain rend 0 pour un nom sans membre, et `pico8_entity_set_field` asserte alors.
- **`pico8_g__t`** (1a.27) : `pico8_ke_t`, puis 18 défauts (`nu`, `fx`, `dq`, `od`, `fp`, `fa`, `dw`, `py`, `ki`, `pn`, `pk`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`, `du`) et 5 champs écrits (`qy`, `ra`, `p`, `et`, `p_`). Ses défauts sont lus une fois dans la classe (`g_g__defaults`) ; `nu`/`k_` sont les REF de la classe. Descripteur `g_g__struct_class` aux 5 entrées = les 5 installations ; lecteur (23 membres, puis `pico8_ke_field`) ; écrivain (5 noms).
- **Chaînage entre fichiers** par l'accesseur du descripteur de la parente (`pico8_hj_struct_class()->field`, précédent `mx` 1a.23) ; init de la parente rendue publique quand la fille vit dans un autre fichier (`pico8_qh_struct_init` 1a.24, `pico8_hj_struct_init` 1a.23).
- **1a.26** (`ga`) : fermeture d'une parente au commit de sa dernière fille. Le `pq` et les installations de la classe parente sont retirés, et ses fonctions passent à la forme struct seule. Les méthodes propres de la feuille (`ga:pq`/`mm`/`kj`) passent aussi à la forme struct seule et quittent sa classe. La preuve `gcov` utilise `__gcov_dump()` avant les `_exit` des fils, dans la copie jetable seulement.
- **1a.27** (`g_`) : `test_pico8_g_` lit déjà le `fv` du titre (`test_fv`, `FV_HASH`), et `test_g__is_a_struct` vérifie les installations de la classe `g_`.

### Ce que fait le cart

`pico8/lua.lua` :

```
:153       jl : hv,cv,cg={},{},{}   (appelé par eu : l'aide quitte les listes au lancement d'une partie)
:209       gk(e,mp,rp) : lit fa, ra, kq
:280-288   pz(p,bs) : lit p.ep, p.eo, p.hi (sinon p.ra), p.ca, p.gq, p.ff ; construit un js par ligne
:685-707   f_ (self.qy={}), ez, ei (fa<10 and not gd → rien ; points, puis ez(self,self.fc))
:708-731   g_=ke:jr(…) ; g_.hq=bv g_.ey=d_ g_.ev=dl g_.pq=f_ g_.kj=ei ; g_:mm
:1143-1170 fv=g_:jr([[ fa=10, ra=v(64,28), hi=v(34,56), ca=11, gq=v(0,11), eo=-1, ff=1, fc=5,
                      k_=o(0,1,1,2,4,9,10,9,4,2,1,1,1), qo=1.02,ro=-0.007, qg=0.007,q_=1.02,
                      pn=5,du=15, py=0.0067,pk=1, ki=17, kq=b(-40,78,40,98), ql=11,fg=5,
                      rb=o(o(192,0,11,16,4,fn="s"), o(172,20,107,2,2,fn="s"),
                           o("need help?",40,110,13,fn="qr"), o("slipways.net/help",40,116,5,fn="qr")) ]])
           fv.hd=gk
           fv:pq : for d in pairs(bo) do d.my=eu end ; pz(self,bo)
           fv:kj(p) : ei(self) ; bl(self.rb)
:86-97     kv:jr : walk pq de la feuille à la racine, dédoublonné
```

Points décisifs :

- **Une seule voie de construction** : `fv()` sans table, une fois par passage à l'écran titre (`cart.c:159`).
- **Walk `pq`** : `fv:pq` (tamponne `my` sur les lignes de `bo`, puis `pz`), puis `f_` (le `pq` de `g_`), puis `ke:pq` (`lt`). Les 4 boutons `js` construits par `pz` s'enregistrent donc dans `hv` **avant** l'aide.
- **Défauts (22)** : 13 recouvrent un défaut de `g_` (`fa`, `ra`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`, `pn`, `du`, `py`, `pk`, `ki`), 9 sont propres (`hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb`). Aucun ne tombe sur un membre de `ke`.
- **`nu` de `fv` est celui de `g_`** : l'aide s'enregistre sous `cg.g_` et `cg.prober` (TROU-8). Tant qu'elle vit, `bg:ldown` lancerait un `mz`.
- **`fv` porte `hd`** (`gk`) : elle est dans `cv.hd`, que la boucle de survol `gw` (`ui.c:289-313`) parcourt sans filtre de classe. Quand la souris est dans `kq` (autour de « need help? / slipways.net/help »), `gw` écrit `m_` sur l'aide, lit son `fa` (index dans `ic`) et son `fg` (`poke 0x5f80`), et `ui:ev` lit son `ql` (sprite du curseur).

### Emprise réelle de `fv` (mesurée)

**Méthode.**
- Grep de `src/`, `tests/`, `tools/` et du `Makefile` à 44a51d5.
- **Sonde** sur une copie `git archive HEAD` jetable du scratchpad, supprimée après usage. Elle journalise, avec les adresses de retour résolues par `addr2line` :
  - chaque lecture `pico8_instance_get_field` sur une instance dont la chaîne contient `fv`, avec sa source (propre, classe propriétaire, absent) ;
  - chaque écriture `pico8_entity_set_field` et chaque `pico8_instance_fields` sur une telle instance ;
  - chaque instanciation de `fv`, avec ses clés ;
  - à l'entrée de `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_`, `na`, `gk`, `gd`, `ma`, `ke_ks`, `pz`, `qh_ly_dispatch`, tout `self` de tag `INSTANCE`, avec sa classe (ou « sans classe ») ; de même pour la cible `et` que `bv` écrit et que `dl` lit.
- Binaire de couverture relancé sur les 17 fichiers `tests/fixtures/*events.json` et sur `events_recorded_1790198294.json` (`--from-title` pour les 4 enregistrements), tous en sortie 0.
- **Fil piège** : les 79 TU de `UNIT_TESTS` (dont les 8 de `UNIT_TESTS_SLOW`), compilés et lancés dans la même copie, tous en sortie 0.

**Constructions d'un `fv`** : **19 sur le filet** (15/15 rejeux, 5 dans `long_session`), et 3 hors filet (`demo`, `cheat_code`, `score_panel_close`). Toutes par `pico8_fv_new(NULL)` (`cart.c:159`), **sans clé**, dans les rejeux comme dans les TU.

**Noms lus sur un `fv`** (sonde, 18 rejeux) :
- propres (défauts de `fv`) : `fa`, `ra`, `hi`, `ca`, `gq`, `eo`, `ff`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`, `pn`, `du`, `py`, `pk`, `ki`, `kq`, `rb` ;
- `kj`, `hd` : les installations propres ;
- dans la classe `g_` : `nu`, `ev`, `mm` ;
- sur l'instance : `qy` (`ez`, `ei`), `t` (`dg`, `g_.mm`) ;
- dans `ke` : `me`, `t` (avant le premier `dg`) ;
- absents : `ep` (`pz`), `hk`, `hb` (`lt`), `ph` (`dg`), `p_` (`dg`).

`ql` et `fg` ne sont lus sur aucun rejeu : la souris n'entre jamais dans `kq` sur le filet (640 appels de `gk` sur l'aide, tous faux).

**Champs écrits sur un `fv`** :
- `qy` (`f_`, `particle.c:69`, 22) ;
- `t` directement par `dg` (`bootstrap.c:101`, via l'aiguillage d'instance) ;
- **`m_`, par `gw`** : jamais sur le filet, mais atteignable en jeu par le survol de l'aide (voir « Ce que fait le cart »).

`ra`, `p`, `et` (`bv`) et `p_` (`d_`) ne sont jamais écrits sur un `fv` : `hq` et `ey` ne sont jamais appelés sur lui, parce que `ui:bh` ne tient que ce qu'un `ldown` a rendu, et que `fv` n'a pas de `ldown`.

**→ 10 membres propres** : les 9 défauts propres (`hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb`) et `m_`. Les 13 défauts qui recouvrent ceux de `g_` vont dans les membres de `g_`, jamais dans une seconde copie.

**Méthodes résolues sur un `fv`** : `kj` (propre), `hd` (propre), `ev` (`g_`, `dl`), `mm` (`g_`, `g_.mm`), et le `pq` hérité `f_`. `hq`, `ey` et le `kj` de `g_` sont joignables par héritage mais jamais résolus.

**Qui atteint encore la classe `g_` ou la classe `fv`** : seule une instance `fv`. `g_` est struct depuis 1a.27 et lit son descripteur. Après ce commit, **rien** ne résout le `pq` ni une installation de ces deux classes (grep : les seuls lecteurs restants sont `test_pico8_g_` et `test_pico8_hj:390,754`).

**`self` de tag `INSTANCE` reçus par les fonctions partagées** (sonde, 18 rejeux et 79 TU) :

| fonction | `self` instance | autres |
|---|---|---|
| `f_` | `fv` seul (198) | — |
| `ez`, `ei` | `fv` seul (3 987) | — |
| `g_.mm` | `fv` seul (19 935) | — |
| `dl` | `fv` seul (3 984) | lit `qs` sur une cible `et` sans classe (637) |
| `gk` | `fv` seul (4 019) | — |
| `bv` | aucun | écrit `fo` sur une cible `et` sans classe (640) |
| `d_` | aucun | lit `fm` (instance `hy`) par `hy:lu`/`hy:nx` |
| `na`, `qh_ly_dispatch` | aucun | — |
| `ke_ks` | aucun | — |
| `pz` | `fv` (198) **et** enveloppes sans classe de `menu.c:247,417` (240) | — |
| `gd` | aucun en jeu ; **`test_pico8_mo`** lui passe 2 instances sans classe faites à la main | — |
| `ma` | aucun | — |

→ **Après la conversion de `fv`, aucune instance n'atteint `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_`, `gk` ni `na` comme `self`.** Ce sont eux qui passent à la forme struct seule ici. Restent à double forme :
- `pz`, par les enveloppes de menus (1a.30, 1a.41) ;
- `gd`, par un test qui lui passe une instance sans classe (1a.42, suppression de la couche) ;
- `ke_ks` et `dg`, fonctions de `ke` dont la dernière descendante est `ui` (1a.31) ;
- les paramètres autres que `self` : la cible `et` de `bv`/`dl` (l'`et={}` jetable, 1a.38 ; `{qs="-too far-"}`, 1a.39), et le `fm` que lit `d_` (1a.36).

**Tests qui touchent l'aide** (grep) : **seul `test_pico8_g_`** la nomme :
- `test_fv` la trouve par `INSTANCE && cls == pico8_fv_class()` (`:677`), compare `kj`/`hd` aux installations de la classe `fv` (`:695-696`) et `pico8_g__class()->pq` à `pico8_f_` (`:697`) ;
- `test_g__is_a_struct` lit les installations de la classe `g_` (`installed`, `:782-787`), asserte `pq == pico8_f_` (`:824`) et cherche l'aide comme instance (`:843-851`).

Les autres TU construisent l'aide par `pico8__init()` sans la lire. Aucun ne passe à ces fonctions une instance de `fv` autrement que par le jeu (fil piège).

**Tests qui comparent à une installation de `g_`** : `test_pico8_hj:390,754` (`class_function(pico8_g__class(), "mm")`).

**Outils** : `tools/float_audit_c1.txt` ne nomme aucun site de `pico8_help.c`, `pico8_probe.c`, `pico8_particle.c` ni `pico8_entity_geom.c`. Le seul site d'audit REF touché de près est `PICO8_REF_AS_LIST` dans `pico8_particle_field_list` (`particle.c:34`) : inchangé.

## Solution

1. **Enum : inchangée.**
2. **`pico8_g__struct_init` publique** (`pico8_probe.h`), coupée du constructeur de `g_` : `pico8_ke_struct_init(&self->ke, cls)`, les 5 membres écrits à nil, puis les 18 défauts statiques de `g_`. Le constructeur de `g_` l'appelle, puis pose ses clés, enregistre sa couverture, appelle `f_` puis `ke:pq`, comme aujourd'hui.
3. **Struct `fv`**, publique dans `pico8_help.h` : `pico8_g__t` en premier membre, puis `hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb` (ordre de la spec), puis `m_`.
4. **Défauts pris par REF, jamais re-parsés** : `pico8_help_init` déclare `fv` comme aujourd'hui (même spec, même parent, même nom de couverture). Elle pose `pico8_class_set_struct_new(g_fv_class, …)`, puis lit **une fois** les 22 défauts **dans la table propre de la classe `fv`** vers un état statique, chaque entrée à l'offset de son membre (13 dans `pico8_g__t`, 9 propres). Un assert porte le message « chaque défaut est une clé de la spec de `fv` ».
   - `ra`, `hi`, `gq`, `k_`, `kq`, `rb` sont **les mêmes REF** que `g_fv_class->defaults`.
   - `pico8_help_shutdown` remet cet état à nil avant de libérer la classe, sans libérer aucune table.
5. **Constructeur de `fv`** (`static`, posé par `set_struct_new`), dans l'ordre du walk du cart :
   1. alloue un `pico8_fv_t` ;
   2. `pico8_g__struct_init(&self->g_, &<descripteur de fv>)` ;
   3. met `m_` à nil, puis copie les 22 défauts statiques ;
   4. **asserte** qu'aucune clé n'est passée (message porteur, arrêt §7 : la seule voie mesurée est `pico8_fv_new(NULL)`) ;
   5. enregistre `"fv"` au crochet de couverture ;
   6. `fv:pq` (tampon `my` sur `bo`, puis `pz`), puis `pico8_f_` (casté), puis `pico8_ke_struct_pq`.
6. **Descripteur de `fv`** (`static const`) :
   - 6 entrées : `HQ` = `pico8_bv_dispatch`, `EY` = `pico8_d__dispatch`, `EV` = `pico8_dl_dispatch`, `MM` = `pico8_g__mm` (**les mêmes pointeurs que le descripteur de `g_`**), `KJ` = `pico8_fv_kj_dispatch`, `HD` = le dispatcher `hd` de `fv`. Les autres sont NULL ;
   - **lecteur** : les 10 membres propres (nil = absent), sinon `pico8_g__struct_class()->field` ;
   - **écrivain** : `m_`, sinon `pico8_g__struct_class()->set_field` (`qy`, `ra`, `p`, `et`, `p_`). 6 noms en tout ;
   - accesseur public `pico8_fv_struct_class()`.
7. **Méthodes propres de `fv` à forme struct seule** :
   - `pico8_fv_pq(pico8_fv_t *self)` : même corps. `pz` reçoit `self` casté. Un `self` NULL tamponne encore `bo` sans appeler `pz` (contrat actuel) ;
   - `pico8_fv_kj(…, pico8_fv_t *self, …)` : lit `rb` dans le membre (même garde, même assert de tag), `ei` reçoit `self` casté ;
   - dispatchers `hd`/`kj` : signatures inchangées, cast interne.
   - **Retirés** de `pico8_help_init` : `pico8_class_set_pq(g_fv_class, pico8_fv_pq)` et les installations `hd`/`kj`. La classe `fv` ne garde que ses défauts de spec (précédent `ga` 1a.26).
8. **Fermeture de `g_`** (règle « Méthodes d'une parente convertie », `fv` dernière descendante) :
   - **retirés** de `pico8_probe_init` : `pico8_class_set_pq(g_probe_class, pico8_f_)`, les 5 installations et leur commentaire. La classe `g_` reste enregistrée (§5, jusqu'à 1a.42) avec ses défauts de spec ;
   - **forme struct seule** pour `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_g__mm`, `pico8_bv`, `pico8_dl`, `pico8_d_` :
     - `f_` et `bv` perdent la branche d'instance de leur garde (`INSTANCE && pico8_instance_fields(self) == NULL`) ;
     - chacune asserte, à l'entrée, qu'un `self` non NULL est une struct (message porteur) ;
     - `self` se lit et s'écrit par le descripteur, comme aujourd'hui ;
     - **signatures inchangées** (voir Implementation Decisions) ;
   - leurs paramètres autres que `self` gardent l'API à double forme : la cible `et` de `bv` (écriture de `fo`) et de `dl` (lecture de `qs`), et le `fm` de `d_`.
9. **`gk` et `na` : forme struct seule** au même titre. `gk` : `fv` était la dernière instance à la recevoir (`hd`). `na` : aucune instance ne l'atteint depuis 1a.25, et aucune étape ne la fermait. Même assert d'entrée, corps et signatures inchangés. `pico8_qh_ly_dispatch` ne change pas : il appelle `na`.
10. **`pico8_fv_new`** rend `pico8_fv_t *`, même paramètre. Son commentaire dit le contrat réel : aucune clé acceptée, l'aide s'enregistre par `lt`, rien ne la libère, et elle asserte avant `pico8_help_init()`.
11. **Tests** :
    - nouveau `test_pico8_fv` (caractérisation dirigée et rejeu court) ;
    - `test_pico8_g_` et `test_pico8_hj` : changements de forme seulement.

    Voir Testing Decisions.
12. **Plan** : à la ligne 1a.28 de `docs/plan-3.md`, ajouter « Découvert à 1a.28 : … », réduit à ce qui change une étape suivante (§7) :
    - `m_` (écrit par `gw` sur `cv.hd`) → membre de `fv` ;
    - `pico8_g__struct_init` publique ;
    - le `pq` et les installations des classes `g_` et `fv` retirés ;
    - `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_`, `gk`, `na` struct seules (`self`) ;
    - restent à double forme, avec leur étape : la cible `et` de `bv`/`dl` (1a.38, 1a.39), `fm` dans `d_` (1a.36), `pz` (1a.30, 1a.41), `ke_ks`/`dg` (1a.31), `gd` (1a.42 ; `test_pico8_mo` lui passe une instance sans classe) ;
    - enum inchangée.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `fv` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `pj`, `ui` et les racines à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `fv` mesurée (constructions et clés, défauts, champs écrits, noms lus, méthodes résolues, `self` instance reçus par chaque fonction partagée), afin que chaque fermeture soit prouvée et que rien ne soit fermé à tort.
3. En tant que lecteur du code, je veux une struct `fv` dont le premier membre est `pico8_g__t` et qui n'ajoute que les 10 noms mesurés, minifiés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux que les 13 défauts de `fv` qui recouvrent ceux de `g_` aillent dans les membres de `g_`, jamais dans une seconde copie, afin qu'un lecteur de `g_` voie `fa`=10 et `ki`=17 sur l'aide.
5. En tant que mainteneur, je veux que les 22 défauts soient copiés à chaque construction depuis un état lu une fois dans la table propre de la classe `fv`, afin qu'aucune table ne soit re-parsée et que `ra`, `hi`, `gq`, `k_`, `kq`, `rb` restent les REF de la classe.
6. En tant que gardien du filet, je veux que le constructeur tamponne `bo` et appelle `pz` avant `f_` et `lt`, afin que les 4 boutons de difficulté précèdent l'aide dans `hv` et gardent leur position (`hi` + `gq`×rang, `fa`=`ca`) et que `ok.le` soit vidé (`ff`).
7. En tant que gardien du filet, je veux que l'aide s'enregistre dans `hv`, `cv.kj`, `cv.ev`, `cv.hd`, `cg.g_` et `cg.prober`, au même rang qu'avant, afin que `dc`, `dg`, `gw` et le choix `mz`/`g_` de `bg:ldown` restent identiques.
8. En tant que gardien du filet, je veux que `kj` émette et peigne les mêmes particules (`ei`, `g_.mm` sur les défauts de `fv`) puis dessine `rb`, afin que l'écran titre ne change pas d'un pixel.
9. En tant que joueur, je veux que survoler « need help? » marque l'aide `m_`, change le curseur (`ql`=11) et la palette (`fg`=5) comme avant, sans assertion, afin qu'un geste jamais enregistré ne fasse pas planter le jeu.
10. En tant que mainteneur, je veux les entrées `HQ`/`EY`/`EV`/`MM` du descripteur de `fv` égales à celles du descripteur de `g_` (mêmes pointeurs), afin que l'héritage du cart soit porté tel quel.
11. En tant que mainteneur, je veux que l'écrivain de `fv` accepte `m_` puis les 5 noms de `g_` et refuse tout autre nom, afin qu'une écriture jamais mesurée soit un arrêt (§7).
12. En tant que mainteneur, je veux qu'une clé de construction passée à `pico8_fv_new` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt.
13. En tant que mainteneur, je veux que les classes `g_` et `fv` n'aient plus ni `pq` ni installation, afin que rien ne puisse plus y résoudre une méthode et que 1a.42 n'ait que des défauts de spec à supprimer.
14. En tant que mainteneur, je veux que `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_`, `gk`, `na` refusent un `self` instance par une assertion porteuse, et que `f_`/`bv` perdent leur branche d'instance, afin que la fermeture soit vérifiable et pas seulement déclarée.
15. En tant que mainteneur, je veux que ces fonctions gardent leurs signatures, et l'API à double forme pour la cible `et` et pour `fm`, afin que les descripteurs de `qh`, `jc`, `mz`, `hj`, `pm`, `mx`, `ga`, `g_` et la vue `ie.ke` les appellent sans changement.
16. En tant que lecteur du plan, je veux que chaque double forme laissée ouverte soit nommée avec l'étape qui la fermera, afin que plus aucune dette ne reste sans échéance.
17. En tant que mainteneur, je veux que `pico8_fv_new` rende la struct et que les commentaires de `pico8_help.h` disent le vrai contrat (l'aide hérite du `nu` de `g_` et entre dans `cg.g_`/`cg.prober`, TROU-8), afin qu'un appelant ne soit pas trompé.
18. En tant que gardien de la couverture, je veux que le constructeur enregistre `"fv"` une fois par aide, afin que `class-coverage-net` soit identique ligne à ligne (`fv 19 15/15`, `g_ 17 9/15`).
19. En tant que vérificateur, je veux une caractérisation de `fv` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (survol de l'aide, `m_`, `ql`, `fg`, identité des tables).
20. En tant que vérificateur, je veux que la caractérisation fasse aussi un rejeu court depuis l'écran titre jusqu'à la sortie de l'aide de `hv`, afin que naissance, particules et mort soient figées frame à frame.
21. En tant que vérificateur, je veux un rapport `gcov` montrant chaque ligne ajoutée ou modifiée exécutée, et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
22. En tant que gardien des caractérisations, je veux que `test_pico8_g_` et `test_pico8_hj` ne changent que leur façon de trouver l'aide, de nommer `mm`/`hd`/`kj` et de constater l'absence d'installation, sans valeur figée touchée (`FV_HASH` compris), afin que 1a.21 et 1a.27 restent des preuves.
23. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne des références de couverture ni de `float_audit_c1.txt` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
24. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C, un piège ou un `TROU-N` nu (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
25. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
26. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** :
  - le module d'aide : struct, état des défauts, constructeur, descripteur, lecteur, écrivain, accesseur, `pico8_fv_new`, `fv:pq`/`fv:kj` retypés, installations retirées ;
  - le module sonde : init de `g_` rendue publique, installations et `pq` de la classe retirés, gardes et asserts de `bv`/`dl`/`d_`/`g_.mm` ;
  - le module de particules : gardes et asserts de `f_`/`ez`/`ei` ;
  - la géométrie d'entité : asserts de `gk`/`na`.

  Rien d'autre dans `src/` : pas de changement dans `cart`, `bootstrap`, `ke`, `ui`, `msgbox` (`pz`), `wormhole*`, `planet*`, `slipway*`, `research_icon`, `entity.c`, `pico8_struct.h` (enum).
- **10 membres propres** : les 9 défauts propres, et `m_`.
  - `m_` suit la règle « Champ d'un écrivain générique » : `gw` l'écrit sur un élément de `cv.hd` sans filtre de classe, et `fv` est l'ancêtre le plus haut de sa chaîne qui porte `hd` (`g_` ne le porte pas). Précédents `ie` (1a.14) et `bg` (1a.12).
  - Jamais écrit sur le filet, il l'est en jeu au survol de l'aide. Un écrivain qui le refuserait ferait planter ce geste.
- **Pas de membre pour `ep`, `hk`, `hb`, `ph`** (lus, toujours absents, sous les deux formes).
- **Défauts lus dans la table propre de `fv`, pas par la chaîne** (G) : un nom absent de la spec de `fv` ne doit pas prendre en silence la valeur de `g_`. Les 6 défauts de `g_` que `fv` ne recouvre pas (`nu`, `fx`, `dq`, `od`, `fp`, `dw`) arrivent par `pico8_g__struct_init`.
- **REF prises dans la classe, alors que `fv` n'a pas de descendante** (G). La règle « Accès directs aux défauts » ne l'exige que pour une classe à descendants non convertis. On le fait quand même, pour garder l'identité des tables d'aujourd'hui, que la caractérisation fige (précédent `g_` 1a.27). Rejeté : re-parser la spec (identité changée, sans gain).
- **`pico8_g__struct_init` publique ; lecteur et écrivain de `g_` chaînés par l'accesseur du descripteur** (G). Précédents `pico8_hj_struct_init` et `mx` (1a.23). Rejeté : rendre publics `pico8_g__field`/`pico8_g__set_field` (surface d'API sans besoin).
- **Aucune clé acceptée par le constructeur** (G) : une seule voie, `pico8_fv_new(NULL)`, mesurée dans les rejeux et les TU. Le paramètre reste, pour la forme commune de `set_struct_new`.
- **Constructeur `fv:pq` → `f_` → `ke:pq`**, l'ordre du walk. Défauts posés avant `fv:pq`, qui lit `hi`, `ca`, `gq`, `eo`, `ff` par `pz`.
  - `f_` avant `fv:pq` est un **mutant équivalent** : `pz` ne lit pas `qy`. `f_` après `ke:pq` l'est aussi : `lt` ne lit que les 5 noms de `bm` et `nu`. On ne les utilise pas comme mutations.
  - Les mutations d'ordre porteuses : `fv:pq` après `ke:pq` (l'aide passe devant ses 4 boutons dans `hv`), ou défauts copiés après `fv:pq` (boutons placés sans `hi`/`gq`/`ca`).
- **Forme struct seule des fonctions partagées : assert d'entrée, pas de retypage** (G).
  - Leurs `self` sont des structs de branches différentes (`g_`, `fv`, `qh` et ses filles, `jc`, `mz`, `hj`/`pm`/`mx`, `ga`, la vue `ie.ke`), dont les champs lus (`qy`, `fa`, `ra`, `et`, `od`…) ne sont pas au même offset. Elles doivent donc continuer à lire `self` par le descripteur.
  - Le seul préfixe commun, `pico8_ke_t`, n'est même pas partagé par la vue `ie.ke` de `dl`.
  - La forme struct seule consiste donc à : retirer ce qui ne sert qu'à une instance (la branche `INSTANCE && fields == NULL` de `f_` et `bv`) ; asserter qu'un `self` non NULL est une struct, ce qui rend la fermeture vérifiable (gcov, `SIGABRT`) ; garder les signatures, sans toucher aucun appelant.
  - Rejeté : retyper `self` en `pico8_ke_t *`. Cela ferait changer ~20 casts d'appelants, ne couvrirait pas `dl`, et n'apporterait aucun accès direct. Le typage réel est la phase 1b.
- **`gk` et `na` fermés ici** (G) : c'est une ligne chacun, mesurée sans instance après `fv`, et la note de 1a.26 les laissait ouverts « (`g_`, `fv`) » sans que 1a.27 nomme d'échéance.
  - `gd` et `ma` ne sont pas dans cette liste et restent intacts : `test_pico8_mo` passe à `gd` des instances sans classe faites à la main, et les changer n'est pas l'objet de l'étape.
- **`ke_ks` et `dg` restent aiguillés** (G). Ce sont des fonctions de `ke`, dont la dernière descendante non convertie est `ui` (1a.31). La branche d'instance de `ke_ks` n'est plus atteinte après ce commit (mesuré : aucun `self` instance), mais la règle du §5 fixe sa fermeture au commit de la dernière descendante de `ke`. `dg` parcourt `hv`, qui contient encore `pj` et `ui`.
- **`pz` reste à double forme** : son `p` est encore une enveloppe sans classe (`menu.c:247`, 1a.30 ; `menu.c:417`, 1a.41).
- **`test_pico8_hj:390,754`** : la comparaison passe à `pico8_g__mm` (fonction publique), puisque l'installation disparaît. Même pointeur, donc vert sur l'ancien code comme sur le nouveau.
- **`test_pico8_g_`** : `test_fv` trouve l'aide par un prédicat indépendant de la forme (`nu` = REF de la classe `g_` et `fa` = 10). `kj` est comparé à `pico8_fv_kj_dispatch`, et `hd` est seulement présent (son comportement est figé par `test_pico8_fv`). `pico8_g__class()->pq` passe à NULL. `test_g__is_a_struct` compare les 5 entrées aux fonctions publiques, constate l'absence des 5 installations et du `pq` dans la classe `g_`, et reconnaît l'aide du titre comme struct au descripteur de `fv`. `FV_HASH` et toutes les autres valeurs figées ne bougent pas. Le commentaire d'en-tête (« `fv`, still a class instance ») est corrigé.
- **Sémantique de nil** : `m_` est absent avant le premier survol, sous les deux formes. `ff`=1, `eo`=-1 et `ql`=11 sont présents. TROU-9 n'intervient pas : aucun nil n'est écrit sur l'aide (mesuré).
- **Clés dynamiques** : aucune.
- **TROU-8** (`prober` sans lecteur) : préservé, `nu` étant celui de `g_`. Pas de nouvelle entrée `TROU`.
- **Commentaires** (CLAUDE.md) :
  - à ajouter :
    - le contrat de la struct `fv` : jamais libérée (TROU-7) ; `ra`, `hi`, `gq`, `k_`, `kq`, `rb` sont les tables de la classe `fv`, empruntées et jamais modifiées ; `nu` est celui de `g_` ;
    - l'assert de clé, celui de l'écrivain, et l'assert d'entrée « `self` struct » des 9 fonctions (message porteur) ;
    - le contrat de `pico8_fv_new` et de `pico8_g__struct_init` ;
  - à corriger ou retirer, rendus faux :
    - `pico8_help.h` sur `pico8_fv_class()` (« `fv`'s spec sets no `nu`, so no `fv` instance ever lands in a `cg` list » : faux, l'aide hérite du `nu` de `g_`) ;
    - `pico8_help.h` sur `pico8_fv_new` (« `overrides` is copied ») ;
    - `pico8_particle.h` sur `f_` (« `pico8_class_pq_fn`-shaped, so `g_.pq=f_` […] ports as a direct `pico8_class_set_pq` ») ;
    - `pico8_probe.c` (« Unused by a probe […] they serve `fv` »), retiré avec les installations ;
    - `pico8_probe.h` sur `dl` (« one caller passes a stack instance » : c'est une struct vue) ;
    - le commentaire d'en-tête de `test_pico8_g_` ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert (et `make e2e` au dernier). La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_fv` sur l'ancien code** (voir Testing Decisions), valeurs figées. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si l'aide du titre n'a pas `fa`=10, `ki`=17, `ca`=11, ou si elle ne partage pas le `nu` de la classe `g_` : la mesure est fausse et doit être corrigée dans le PRD.
2. **`fv` converti** :
   - `pico8_g__struct_init` coupée ;
   - struct, état des défauts, constructeur, descripteur, lecteur, écrivain, accesseur, `pico8_class_set_struct_new`, `pico8_fv_new` typé, `fv:pq`/`fv:kj` retypés, `pq` et installations de la classe `fv` retirés ;
   - `test_pico8_g_` adapté (recherche de l'aide seulement).

   Toutes les caractérisations passent **sans changer leurs valeurs figées**.
3. **Fermeture de `g_`** :
   - `pq` et 5 installations de la classe `g_` retirés ;
   - gardes et asserts de `f_`, `ez`, `ei`, `g_.mm`, `bv`, `dl`, `d_`, `gk`, `na` ;
   - `test_pico8_hj:390,754` et `test_g__is_a_struct` retargetés.

   Les vérifications de forme s'ajoutent à `test_pico8_fv`.
4. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.28 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, les octets de mémoire (`0x5f80`), le compte de couverture, l'identité des REF et des pointeurs de fonction publics, `qy`, `m_`, `t`. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_fv`** : lié comme `test_pico8_g_`, sur le cart réel, avec une dépendance Makefile sur l'enregistrement qu'il rejoue.
  - Il lit **uniquement** par l'API à double forme, et par `pico8_class_get_field(pico8_fv_class()/pico8_g__class(), …)` pour les **défauts de spec**, jamais pour une méthode (les installations disparaissent au morceau 2).
  - `kj`, `ev`, `mm` sont comparés aux **fonctions publiques** (`pico8_fv_kj_dispatch`, `pico8_dl_dispatch`, `pico8_g__mm`). `hd` (dispatcher `static`) est jugé par son comportement.
  - Son code est le même avant et après, aux vérifications de forme près.
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()`, pour l'état de `rnd`.
  - Toute présence se lit par un `lookup` qui confond nil et absent.
  - L'aide est trouvée dans `hv` par un prédicat indépendant de la forme (`nu` = REF de la classe `g_`, `fa` = 10).
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **Construction** (`pico8__init()`, écran titre) :
       - l'empreinte des 22 défauts de `fv` et des 6 hérités de `g_` (scalaires, vecteurs, tailles et contenus de `k_`/`rb`/`kq`), de `me`, `t`, de la taille de `qy` ;
       - l'**identité** : `ra`, `hi`, `gq`, `k_`, `kq`, `rb` sont les REF de `g_fv_class->defaults`, et `nu` celle de `g_probe_class->defaults` ;
       - l'absence de `m_`, `ep`, `p`, `et`, `p_`, `hk`, `hb`, `ph`, `ldown`, `fo`, `ma` ;
       - `kj`, `ev`, `mm` égaux aux fonctions publiques, `hq`/`ey` égaux à `pico8_bv_dispatch`/`pico8_d__dispatch`, `hd` présent ;
       - le rang de l'aide et des 4 boutons de `pz` dans `hv`, l'appartenance à `cv.kj`/`cv.ev`/`cv.hd` (pas `hk`/`hb`), à `cg.g_` et `cg.prober` ;
       - les lignes de `bo` tamponnées `my` (= `pico8_eu_field_value()` du moment), et `ok.le` vide (`ff`).

       Figée.
    2. **Vie** :
       - `dc("kj")` ×3, avec l'empreinte d'écran et la taille de `qy` ;
       - `dc("ev")` (`dl` sort tôt : `me`=`"ph"`), avec l'empreinte d'écran ;
       - `dg` ×3 (`t` qui avance) ;
       - `hd` appelé directement en un point dans `kq` et un point hors de `kq` (vrai/faux, `rp` retenu puisque `fa` ≥ 10) ;
       - **survol** : souris (`je.ra`) posée dans `kq`, puis `gw`. On fige `ok.o_` = l'aide, `m_` vrai, l'octet `0x5f80` (= `fg`), l'entrée `ic[fa]`, le sprite de curseur de `ui:ev` (= `ql`). Puis la souris sort, `gw` → `m_` faux.

       Figée.
    3. **Rejeu court** : un enregistrement de `RECORDINGS`, `--from-title` (le plus court qui quitte le titre), jusqu'à la première frame où l'aide n'est plus dans `hv`. À chaque frame, on plie son rang, `t`, la taille de `qy` et la présence de `m_`. S'y ajoutent la frame de sortie et une empreinte d'écran finale. Figée.
  - **Forme** (ajoutée aux morceaux 2-3) :
    - l'aide est `STRUCT` au descripteur de `fv` ;
    - `HQ`/`EY`/`EV`/`MM` valent ceux de `pico8_g__struct_class()`, `KJ` vaut `pico8_fv_kj_dispatch`, `HD` est non NULL, les autres sont NULL ;
    - `cls->field` et `cls->set_field` sont posés et différents de ceux de `g_` ;
    - `pico8_entity_set_field` passe pour `m_`, `qy`, `ra`, `p`, `et`, `p_` et asserte (dans un fils) pour `fa`, `hi`, `rb`, `ql`, `nu`, `fo`, `me` ;
    - `pico8_fv_new` avec une clé (nommée ou positionnelle) asserte ;
    - `pico8_fv_class()->struct_new` est posé, et son `pq` est NULL. `pq` de la classe `g_` NULL. Les installations `hd`/`kj` sont absentes de `g_fv_class->defaults`, et `hq`/`ey`/`ev`/`kj`/`mm` de `g_probe_class->defaults` (lecture directe de la table) ;
    - **fermeture** : une instance sans classe faite à la main (comme `test_pico8_mo`) passée à `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_g__mm`, `pico8_bv`, `pico8_dl`, `pico8_d_`, `pico8_gk`, `pico8_na` avorte, chacune dans un fils ; un `self` NULL reste un no-op.
- **Test de régression (§6)** : aucun nommé par la ligne. Tous gardent un diff vide et restent verts.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (copie de l'arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` `test_pico8_fv`, `test_pico8_g_`, `test_pico8_hj` et `test_pico8_mo`, avec les objets du moteur. Les lancer, puis `gcov` sur `pico8_help.c`, `pico8_probe.c`, `pico8_particle.c`, `pico8_entity_geom.c`.
    - Même piège qu'à 1a.23-1a.27 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`.
    - Les branches d'assert atteintes seulement dans un fils qui avorte (`must_abort`) ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu et justifiées ainsi.
    - Le rapport est collé dans le message de commit. Il donne les lignes exécutables ajoutées ou modifiées de `src/` et leur compte : constructeur, lecture des défauts, reset, lecteur, écrivain (dont `m_`), accesseur, `pico8_fv_new`, `fv:pq`/`fv:kj`, `pico8_g__struct_init`, gardes et asserts des 9 fonctions. Les lignes à 0 sont données avec leur justification.
  - **2 à 3 mutations**, chacune rouge sur `test_pico8_fv` (ou un autre test nommé), notées dans le commit avec le test qui l'attrape :
    1. **valeur d'un défaut propre** : `fa` (ou `ca`, `ki`) non copié, ou copié dans un autre membre. `fa` resterait à 2 (`g_`) et les particules, le choix `mp`/`rp` de `gk` et l'index de `ic` changeraient ;
    2. **entrée du descripteur remplacée** : `KJ` = `pico8_ei_dispatch` (plus de `rb` dessiné), ou `EV` = NULL (aide hors de `cv.ev`) ;
    3. **ordre du constructeur** : `fv:pq` après `pico8_ke_struct_pq` (l'aide passe devant ses 4 boutons dans `hv`), ou défauts copiés après `fv:pq`. Pas `f_` avant `fv:pq`, ni `f_` après `lt` (équivalents).
- **Tests adaptés** :
  - `test_pico8_g_` (`test_fv`, `test_g__is_a_struct`, en-tête) ;
  - `test_pico8_hj:390,754` (`pico8_g__mm`).

  Aucune valeur figée ne bouge.
- **Tests existants à diff vide** : `test_pico8_bg`, `jc`, `mz`, `mo`, `jo`, `qh`, `nl`, `os`, `pm`, `mx`, `nr`, `fr`, `ga`, `ie`, `qb`, `nf`, `js`, `fq`, `iv`, `mh`, `je`, `bg_kj`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage`, et tous les tests de régression.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** :
  - `test_pico8_g_` : structure, `lookup`, `must_abort`, fils par scénario, lecture de l'aide du titre, rejeu court ;
  - `test_pico8_ga` : fermeture d'une parente (installations absentes, `pq` NULL, méthodes nommées par le descripteur) ;
  - `test_pico8_ie` et `test_pico8_bg` : survol par `gw` (`m_`, `o_`) ;
  - `test_pico8_mo` : instance sans classe faite à la main.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; c'est le seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` (en particulier `fv  19  15/15`, `g_  17  9/15`). Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations (défaut propre, entrée du descripteur, ordre du constructeur) ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier :
  - si l'assertion de clé, celle de l'écrivain ou l'assert « `self` struct » d'une des 9 fonctions se déclenche sur un rejeu ou un test hors `must_abort` ;
  - si un membre propre autre que les 10 est ajouté, si un défaut recouvrant `g_` reçoit un second membre, ou si un membre répète `me`/`t` ;
  - si l'écrivain de `fv` accepte autre chose que `m_`, `qy`, `ra`, `p`, `et`, `p_`, ou refuse `m_` ;
  - si le constructeur accepte une clé ;
  - si l'enum change.
- `FAILED:` si une autre classe que `fv` est convertie ou change de forme (`pj`, `ui`, `hp`, `lq`, `hy`, `trade`…), ou si le comportement d'une autre classe change (valeur figée de sa caractérisation, ligne de couverture).
- `FAILED:` si une entrée `HQ`/`EY`/`EV`/`MM` du descripteur de `fv` n'est pas le même pointeur que celle du descripteur de `g_`, si `KJ` n'est pas `pico8_fv_kj_dispatch`, si `HD` est NULL, ou si une autre entrée est non NULL.
- `FAILED:` si le `pq` ou une installation reste dans la classe `g_` ou dans la classe `fv`, ou si l'une de ces classes cesse d'être enregistrée (registres `fz`/nommé) avec ses défauts de spec.
- `FAILED:` si l'une de `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_g__mm`, `pico8_bv`, `pico8_dl`, `pico8_d_`, `pico8_gk`, `pico8_na` accepte encore un `self` instance, garde une branche propre à l'instance, change de signature, ou change de comportement sur une struct. Également si le traitement de la cible `et` (`bv`, `dl`) ou de `fm` (`d_`) passe à la forme struct seule.
- `FAILED:` si `pico8_ke_ks`, `dg`, `pz`, `gd`, `ma`, `pico8_qh_ly_dispatch` ou `pico8_bg_ldown` change.
- `FAILED:` si une table de défaut de `fv` est re-parsée depuis la spec, si un défaut est lu par la chaîne plutôt que dans la table propre de `fv`, ou si `ra`/`hi`/`gq`/`k_`/`kq`/`rb` d'une aide neuve n'est pas la REF de `g_fv_class->defaults`.
- `FAILED:` si le constructeur de `fv` n'appelle pas `fv:pq`, puis `f_`, puis `ke:pq`, ou s'il copie les défauts après l'un d'eux.
- `FAILED:` si un test de régression (§6) a un diff non vide.
- `FAILED:` si une valeur figée de `test_pico8_fv` a été changée après la conversion, ou une de `test_pico8_g_` (`FV_HASH` compris), `hj`, `bg`, `jc`, `mz`, `mo`, `jo`, `qh`, `nl`, `os`, `pm`, `mx`, `nr`, `fr`, `ga`, `ie`, `qb`, `nf`, `js`, `fq`, `iv`, `mh`, `je`, `bg_kj`. Également si `test_pico8_g_`/`test_pico8_hj` changent au-delà de la recherche de l'aide, de la façon de nommer `mm`/`kj`/`hd` et du constat d'absence des installations et du `pq`.
- `FAILED:` si une ligne est ajoutée à `tools/float_audit_c1.txt` ou en est retirée, ou si `make float-audit` est rouge.
- `FAILED:` si `test_pico8_fv` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.
- `FAILED:` si la note « Découvert à 1a.28 » laisse une double forme ouverte sans nommer l'étape qui la fermera.

Critères de réussite (en plus) :

- la struct `fv` n'a que `pico8_g__t` et les 10 membres ; son descripteur a 6 entrées ;
- grep :
  - `PICO8_CLASS_COVERAGE_NAME(g_fv_class, "fv")` reste ;
  - `pico8_class_set_struct_new(g_fv_class, …)` est présent ;
  - aucun `pico8_class_set_pq(g_fv_class|g_probe_class, …)`, aucun `set_name(g_fv_class->defaults|g_probe_class->defaults, …)` ;
  - `INSTANCE` n'apparaît plus dans `pico8_f_` ni dans `pico8_bv` hors de la construction de l'`et` jetable ;
- l'aide du titre est une struct au descripteur de `fv`, dans `cg.g_` et `cg.prober` ;
- la ligne 1a.28 de `docs/plan-3.md` porte la note « Découvert à 1a.28 », et le message de commit dit « découvert à 1a.28 ».

## Out of Scope

- `pj` (1a.29) et les étapes suivantes.
- Les doubles formes laissées ouvertes, à leurs étapes :
  - la cible `et` de `bv`/`dl` : l'`et={}` jetable (1a.38), `{qs="-too far-"}` (1a.39) ;
  - `fm` lu par `d_` (1a.36) ;
  - `pz` (1a.30, 1a.41) ;
  - `ke_ks`, `dg` (1a.31) ;
  - `gd`, `ma` (1a.42).
- Supprimer les classes `g_` et `fv`, `kv.g_` (`pico8_class_lookup_named`) ou les registres : 1a.42.
- Corriger TROU-8 (`prober` sans lecteur), ou le fait que `bg:ldown` lance un `mz` tant que l'aide vit.
- La mise en composant des champs d'émetteur de `g_`/`fv`/`hj`/`ga` (phase 2, règle 2).
- Le typage C des membres et de `self` des fonctions partagées (1b).
- Libérer l'aide, sa liste `qy`, les boutons de `pz` ou les tables de `bo` (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Fermeture de la dernière famille sous `g_`** : après ce commit, aucune instance ne descend de `g_`, et les classes `g_` et `fv` ne portent plus que leurs défauts de spec. La dette posée à 1a.21-1a.27 se solde ici : `f_`, `g_.mm`, `ez`, `ei`, `bv`, `dl`, `d_` à double forme, le `pq` et les installations de `g_` gardés pour `fv`. Il reste des instances sous `ke` : `pj` (1a.29) et `ui` (1a.31).
- **`hd` de l'aide** : c'est la seule voie par laquelle le jeu écrit sur l'aide un nom qui n'est pas dans sa spec (`m_`). Le filet ne la voit jamais. D'où le scénario de survol dirigé, et `m_` accepté par l'écrivain.
- **Mesure utile pour 1a.29/1a.31** : `ke_ks` ne reçoit plus d'instance après ce commit. `dg` en reçoit encore par `hv` (`pj`, `ui`). Leur fermeture revient à `ui`, dernière descendante de `ke`.
- **Leçons de 1a.23-1a.27 appliquées** : mutation d'ordre sur ce qui compte (`pz` avant `lt`, défauts avant `pz`), pas sur un mutant équivalent (`f_`). Piège `_exit` des fils réglé dans la copie jetable seulement. Pas de membre pour un nom seulement lu et toujours absent.
- Mesures :
  - grep sur `src/`, `tests/`, `tools/` et le `Makefile` à 44a51d5 ;
  - lecture de `pico8/lua.lua:86-97,153,209,280-288,685-731,1143-1170` ;
  - lecture de `pico8_help.c/h`, `pico8_probe.c/h`, `pico8_particle.c/h`, `pico8_entity_geom.c/h`, `pico8_msgbox.c` (`pz`), `pico8_ui.c` (`gw`, `ui:ev`, `ui:bh`), `pico8_ke.c/h`, `pico8_entity.c`, `pico8_research_icon.c` (vue `ie.ke`), `pico8_cart.c`, des tests nommés, de `tools/class_coverage_net_expected.txt` et `tools/float_audit_c1.txt` ;
  - lecture des PRD de 1a.26 et 1a.27 et des messages de 288e7f6 et ed688a8 ;
  - sonde dans une copie jetable du scratchpad, supprimée après usage : 18 rejeux en sortie 0 ; 22 aides (19 sur le filet), toutes par `pico8_fv_new(NULL)` ; `qy` seul écrit par l'API, `t` par `dg` ; `self` instance reçus : `fv` seul pour `f_`/`ez`/`ei`/`g_.mm`/`dl`/`gk`, aucun pour `bv`/`d_`/`na`/`ke_ks`/`ma` ; `pz` reçoit aussi les enveloppes sans classe ;
  - fil piège sur les 79 TU (`UNIT_TESTS`, `UNIT_TESTS_SLOW` compris), même copie : aucun test ne passe à ces fonctions une instance autre que l'aide construite par `pico8__init()`, sauf `test_pico8_mo` pour `gd` (instances sans classe faites à la main) ; seul `test_pico8_g_` nomme l'aide.
