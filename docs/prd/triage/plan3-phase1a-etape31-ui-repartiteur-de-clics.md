---
title: "Plan 3, étape 1a.31 : `ui` (le répartiteur de survol et de clics, singleton `ok`) devient une struct fille de `ke` ; les clés construites `bt..\"held\"`/`bt..\"down\"` deviennent `lheld`/`rheld` et `ldown`/`rdown` ; dernière descendante de `ke`, elle fait passer `ke_ks` et `dg` à la forme struct seule"
description: "Le répartiteur `ui` est construit 38 fois sur le filet, toujours par `pico8_ui_new(NULL)` depuis `jl()`. Il passe en struct `pico8_ui_t` (`pico8_ke_t`, 4 défauts `ql`/`fa`/`le`/`hw`, 6 champs écrits `qa`/`ic`/`o_`/`gj`/`lheld`/`rheld`), au descripteur d'une seule méthode (`ev`), sans écrivain. La clé construite de `ui:bh` devient un choix entre deux membres et deux noms de méthode. `le` (échangé en liste à l'init) et `hw` (lu sur la classe par `bv`) sont pris dans la table de la classe, jamais re-parsés, et `hw` est lu par un accesseur. Mesuré : `ui` était la seule instance à atteindre `dg`, et aucune n'atteint `ke_ks`, qui passent donc à la forme struct seule. Sept tests de régression sont réécrits dans leur façon de lire `ok`, puis prouvés rouges."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.31 — `ui` : `ke` (`ui.c:93`), `bt.."held"` → `lheld`/`rheld`, `bt.."down"` → `ldown`/`rdown` (`ui.c:316-352`), `le` réécrit (`:102`), `hw` lu (`probe.c:112`). Tests : `nil_row`, `ga_export_panel_by`, `pause_menu_music`, `planet_gh_row_lq`, `rich_planet_exports`, `tech_screen`, `slipway_drag`.

Les numéros de ligne ont dérivé. À HEAD a95bb03 :
- **`pico8_ui.c`** :
  - la déclaration `g_ui_class = pico8_class_declare(pico8_ke_class(), "ql=58,fa=15,le=o(),hw=b(10,10,118,118)")` est à `:116`, son nom de couverture à `:117` (le `:93` du plan) ;
  - l'échange de `le` (l'`ob_table` de `le=o()` remplacée par une vraie `pico8_list_t` vide dans `g_ui_class->defaults`) est à `:119-127` (le `:102` du plan, vérifié sur 34662a8, le commit qui a posé la ligne) ;
  - l'installation `ev` dans `g_ui_class->defaults` est à `:129` ;
  - `pico8_ui_new` est à `:154`, `pico8_ok`/`pico8_ui_set_ok` à `:173-183`, `pico8_ui_shutdown` (qui libère `g_ok` par `pico8_instance_free`) à `:134` ;
  - `pico8_ui_lj` est à `:187` : il lit `le` (`:199`, `:223`), écrit `qa` (`:210`) et `gj` (`:230`) par `pico8_instance_fields` ;
  - `pico8_ui_ev` est à `:235` (lit `rheld`, `o_`, `ql`) ; `pico8_ui_gw` à `:267` (écrit `ic`, `o_` à `:283-284` et `:301`, lit `qa`, `gj`) ;
  - `pico8_ui_bh` est à `:326-376` (le `:316-352` du plan) : `snprintf(ni, "%sheld", bt)` à `:338`, `snprintf(down_name, "%sdown", bt)` à `:347`, écriture `self[ni]` à `:350` et `:374` ;
  - `pico8_ui_jz` est à `:381` (lit `ic`), `pico8_ui_cj` à `:434` (lit et réécrit `le`).
- **`pico8_probe.c`** : `pico8_probe_ui_hw_call` est à `:104-117` ; il lit `ui_class->defaults` à `:112` (le `:112` du plan) pour `bv`.
- **`pico8_msgbox.c`** : `pico8_pz` appelle `pico8_ui_cj(ok)` à `:500` et écrit `ok.le` par `pico8_instance_fields` à `:506` et `:522`.
- **`pico8_slipway.c`** : `pico8_jc_hq` lit `ok.o_` à `:107-108` (par `pico8_jc_entity_instance`, API à double forme).
- **`pico8_cart.c`** : `pico8__update60` appelle `pico8_ui_lj(mem, pico8_ok())` à `:264` ; `:59-60` asserte que le parent de `ui` est `ke`.
- **`pico8_bootstrap.c`** : `pico8_jl` construit `ok` par `pico8_ui_new(NULL)` à `:50` et le pose à `:54` ; `pico8_dg` est à `:65-105`, avec sa branche instance à `:101`.
- **`pico8_ke.c`** : `pico8_ke_ks` est à `:36-47`, branche instance à `:45-46`.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Clés construites à l'exécution** : `self[bt.."held"]` (`pico8_ui.c:328`) → deux champs `lheld`/`rheld` ; `ldown`/`rdown` côté méthodes. Tout autre site de clé dynamique trouvé est énuméré de même, ou déclenche un arrêt (§7) si l'ensemble des clés n'est pas fini.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe […].
>
> **Méthodes d'une parente convertie** : […] Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante.

`ui` est une **feuille** sous `ke`, et la **dernière descendante non convertie de `ke`** dans `src/` (arbre du §2 ; `pj` a été convertie à 1a.29). Ce commit ferme donc `ke`, comme 1a.28 a fermé `g_`.

Notes du plan qui échoient ici :
- **1a.28** : « restent à double forme : […] `ke_ks`/`dg` (1a.31) ».
- **1a.29** (Further Notes du PRD) : « `dg` reçoit la struct `pj` par `hv`, et `ke_ks` ne reçoit jamais `pj` ».
- **1a.5** et **1a.12** : `ldown` puis `rdown` sont entrés dans l'enum, pour `fq`/`js`/`jo`/`nr` (`ldown`) et `bg` (`rdown`). Les deux méthodes que `bh` construit par leur nom sont **déjà** indexées.
- **1a.6/1a.7** : `je` (`l`/`r`, `ra`) est un `nf` struct ou le substitut statique ; `pico8_ui_je_button_table` asserte déjà `bt ∈ {l, r}`.
- **1a.13** : `slipway_drag` a été réécrit pour lire le `jc` tenu (`qu`/`od`) par l'API à double forme ; sa lecture de `ok.lheld` restait sur la couche « jusqu'à 1a.31 ». Même remarque dans le PRD de 1a.16 pour les lectures de `ok.le`/`ok.o_` de ses cinq tests.

§6 : la ligne nomme **sept** tests de régression. Tous lisent `ok` par `pico8_instance_get_field`, qui asserte sur une struct. Chacun est réécrit **dans sa façon de lire** `ok` seulement, et prouvé rouge avec un bug remis.

§7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` : `ui  38  15/15`, toutes les autres lignes inchangées (`ke  0  0/15  exemptée`) ;
- **preuve des tests par `gcov` et 2 à 3 mutations** ;
- **note « Découvert à 1a.31 »** réduite à ce qui change une étape suivante ;
- un nouveau test dont le rejeu dépasse 30 s va dans `UNIT_TESTS_SLOW` ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.30 ont posé (jusqu'à c1da235), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`).
  - L'enum contient déjà `EV`, `LDOWN`, `RDOWN`, `GV`, `HQ`, `EY`, `HD`, `PH` : **elle ne change pas**.
  - `pico8_entity_get_field` rend d'abord l'entrée du descripteur, sinon le lecteur ; un membre nil est absent.
  - `set_field` NULL : une écriture générique avorte (précédents `nf`, `mh`, `iv`, `pj`).
- `pico8_ke_t`, `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq` (1a.2).
- **REF prises dans la table de la classe, jamais re-parsées** : tableau `{nom, offset, valeur}` rempli une fois à l'init par `pico8_class_get_field`, avec un assert « chaque défaut est une clé de la spec », remis à nil au shutdown (`g_`, `fv`, `pj`).
- **Défaut lu sur la classe** : `bg.pl` (1a.12), `pj.gs`/`pj.i_` (1a.29) : le membre porte la REF de la classe, le lecteur du cart lit l'état statique par un accesseur.
- **Échange d'un défaut à l'init** gardé dans la classe : `ga.by` (1a.26) ; l'état statique est la même REF, lue **après** l'échange.
- **Fermeture d'une parente au commit de sa dernière fille** : `jo` (1a.26), `g_` (1a.28). Forme struct seule = retirer la branche instance et asserter qu'un `self` non NULL est une struct, **sans retypage** de signature ni changement d'appelant.
- Constructeur `static` posé par `pico8_class_set_struct_new`, qui asserte sur toute clé non mesurée, enregistre la couverture et appelle `pico8_ke_struct_pq`.
- **Test qui écrivait un champ par l'API générique** : il écrit le membre directement plutôt que d'élargir l'écrivain (1a.27, 1a.29).

### Ce que fait le cart

`pico8/lua.lua` :

```
:153-155   jl() : hv,cv,cg={},{},{} ; mo,je,ok=qb(),nf(),ui()
:156-161   lt : add(hv,e) ; cv[p] pour p ∈ bm={kj,ev,hd,hk,hb} présents ; cg[t] pour t ∈ e.nu
:168-177   dg : fn=qc[qc.me] ; fn(qc,qc.t) ; qc.p_ → he ; qc.t+=1
:212-216   ui=ke:jr([[ ql=58, fa=15, le=o(), hw=b(10,10,118,118) ]])
:217-224   ui:lj() : local le=self.le ; self.qa=je.ra+mo.ra ; self:gw() ;
                     if #le==0 or gu(le,self.o_) then self:bh("l") end ; self:bh("r") ;
                     if je.l.jn and self.le==le then self:cj() end ; self.gj=self.o_
:225       ui:ev()  : spr(self.rheld and 9 or self.o_ and self.o_.ql or self.ql, je.ra.x, je.ra.y)
:226-236   ui:gw()  : self.ic,self.o_={} ; for e in all(cv.hd) … ft(self.ic,e.fa,e) self.o_=e ;
                     (self.gj or {}).m_=false ; (self.o_ or {}).m_=true ; poke(0x5f80,…)
:237-247   ui:bh(bt): local b,ni=je[bt],bt.."held" ; if b.jn then pw=self:jz(bt.."down",self.qa,je.ra)
                     if pw and type(pw.kn)=="table" then self[ni]=pw.kn end end ;
                     local held=self[ni] ; mv(held,"hq",…) ;
                     if b.hm and held then nw=self:jz("gv",held) mv(held,"ey",nw and nw.kv) self[ni]=nil end
:248-255   ui:jz(rm,…) : for mf=15,0,-1 … mv(h,rm,…) → {kv=h,kn=kn}
:256-257   ui:cj()  : nv(self.le,he) ; self.le={}
:279-287   pz(p,bs) : … ok:cj() ; ok.le=nv(bs,…js(b)) ; if p.ff then ok.le={} end
:550-557   bv       : … or not ui.hw(rp) and {}        -- hw lu sur la CLASSE, appelé comme fonction
:622       jc:hq    : self.re = ok.o_.gv and ok.o_ or {…}
:1187      _update60: dg() ; ok:lj()
```

Points décisifs :
- **Une seule voie de construction** : `ui()` sans table, dans `jl()` (2 par partie : `_init` puis `eu`).
- **Walk `pq`** : `ui` n'a pas de `pq` propre ; seul `ke:pq` (`lt`) court. `ui` a `ev` → `cv.ev` ; `fa=15` le range en dernière couche de `dc("ev")` (le curseur est dessiné par-dessus tout). Pas de `nu`.
- **`me="ph"`** hérité de `ke`, mais `ui` n'a pas de `ph` : `dg` appelle `mv(ok,"ph")`, qui ne trouve rien.
- **Clés construites** : `bt` ne prend que deux valeurs, `"l"` et `"r"` (deux appels littéraux dans `lj`, et `je[bt]` déjà borné à `l`/`r` en 1a.6). L'ensemble des clés est **fini** : `lheld`, `rheld` (champs) et `ldown`, `rdown` (méthodes cherchées sur les entités de `ic`, pas sur `ui`). Pas d'arrêt.
- **`le`** : défaut `o()` partagé par la classe. Il n'est jamais modifié en place (`gu` et `nv` le lisent seulement) ; il est remplacé **sur l'instance** par `cj`/`pz`. Tant qu'aucun `cj` n'a couru, `ok.le` est la table de la classe.
- **`hw`** : jamais lu sur `ok`, seulement sur la classe (`ui.hw(rp)` dans `bv`).

### Emprise réelle de `ui` (mesurée)

**Méthode.**
- Grep de `src/`, `tests/`, `tools/` et du `Makefile` à a95bb03.
- **Sonde** sur une copie `git archive HEAD` jetable du scratchpad, supprimée après usage. Binaires compilés en `-O0 -fno-omit-frame-pointer -no-pie`, appelants résolus par `addr2line`. Elle journalise :
  - chaque `pico8_instance_get_field` sur une instance de la classe `ui` (nom, source : propre, classe, absent ; appelant) ;
  - chaque `pico8_instance_fields` sur une telle instance (appelant) ;
  - chaque `pico8_entity_set_field` sur une telle instance ;
  - chaque instanciation de `ui` (nombre de clés) ;
  - chaque lecture de `hw` par `pico8_probe_ui_hw_call` ;
  - dans `bh` : chaque écriture de `lheld`/`rheld` (tag de la valeur, tag du `kv` gagnant) et chaque remise à nil ; les noms passés à `jz` ;
  - chaque `self` de `cj`/`lj` qui n'est pas le `ok` courant ;
  - **toute instance** (quelle que soit sa classe) reçue par `pico8_dg` dans `hv`, et par `pico8_ke_ks`.
- Rejoués : les 17 fichiers `tests/fixtures/*events.json` et `events_recorded_1790198294.json` (`--from-title` pour les 4 enregistrements), **18 rejeux en sortie 0**.
- **Fil piège** : `make unit`, puis les 8 cibles de `UNIT_TESTS_SLOW`, dans la même copie, même journal, par nom de processus. Sortie 0 les deux fois.

**Constructions d'un `ui`** : 2 par rejeu (10 dans `long_session`), soit **38 sur le filet** (15/15), toutes par `pico8_ui_new(NULL)` depuis `pico8_jl` (`bootstrap.c:50`), **sans clé**. Dans les TU (`make unit` + lents) : 466 constructions, **toutes sans clé** (dont `test_pico8_mo`, `nf`, `je` par `pico8_ui_new(NULL)` directement).

**Noms lus sur un `ui`** :
- dans la classe `ui` : `ql` (`ev`), `fa` (`dc`), `le` (`lj`, `cj`, avant le premier `cj`), `ev` (`lt`, `mv` de `dc`) ;
- dans `ke` : `me` (`dg`), `t` (`dg`, avant la première écriture) ;
- sur l'instance : `qa`, `ic`, `o_`, `gj`, `le`, `lheld`, `rheld`, `t` ;
- absents : `ra` (`dc`), `ph` (`mv` de `dg`), `p_` (`dg`), `nu`, `kj`, `hd`, `hk`, `hb` (`lt`), `fx` (prédicat de `mz:pq`, `wormhole.c:80`), `o_`/`gj`/`lheld`/`rheld` avant leur première écriture ; dans les TU seulement, par des prédicats qui parcourent `hv` : `fz`, `mb`, `nl`, `hz`, `px`.
- **`hw` n'est jamais lu sur une instance.** Il est lu sur la classe par `bv` : 923 fois sur les 17 premiers rejeux, 5 553 dans `session`, dans les TU par `bg`, `jc`, `mz`, `qb`, `nl`, `js`, `g_`, `ga`, `fr`, `geoharvest_burst`, `nil_row`, `rich_planet_exports`, `playability`, `slipway_drag`, `empty_planet_cleared`.

**Champs écrits sur un `ui`** : aucun par `pico8_entity_set_field` dans les 18 rejeux. Dans les TU : **un seul site**, `test_pico8_jc:431` (`o_`, 2 fois). Toutes les autres écritures passent par `pico8_instance_fields` :
- `qa`, `gj` (`lj`) ; `ic`, `o_` (`gw`) ;
- `lheld`, `rheld` (`bh`, pose et remise à nil) ;
- `le` (`cj` ; `pz`, `msgbox.c:506,522`) ;
- `t` (`dg`, `bootstrap.c:101`).

**Clés construites** (sonde dans `bh`/`jz`) :
- `jz` reçoit `"ldown"` (770 fois sur les rejeux), `"rdown"` (7, tous dans `long_session`) et `"gv"` (352). Aucun autre nom ;
- `lheld` est posé 63 fois sur les 17 premiers rejeux (plus `session`), toujours avec une **struct** (un `jc`, rendu par `jo:ldown`/`nr:ldown`) ; `rheld` 7 fois (`long_session`, un `bg` rendu par `bg:rdown`), 28 fois dans les TU lents, 3 dans `test_pico8_bg`/`qb` ;
- chaque pose est suivie de sa remise à nil au relâchement (`hm`).

**`self` de `lj`/`cj`** : toujours un `ui`. Il n'est pas le `ok` courant 12 fois sur les rejeux : `jl()` reconstruit `ok` pendant un clic dispatché depuis `lj` (`bo[n].my` → `eu`), et la fin de `lj` court encore sur l'ancien (commentaire de `pico8_ui_set_je`). Ce cas doit rester.

**`dg` et `ke_ks`** (tous rejeux et toutes TU) :
- `pico8_dg` reçoit une **instance** dans `hv` **uniquement quand c'est un `ui`** (≈ 170 000 fois sur les rejeux, et dans chaque TU qui lance une partie). Aucune autre instance, d'aucune classe, n'y passe ;
- `pico8_ke_ks` ne reçoit **jamais** d'instance (ses appelants sont `jo:hq`/`jo:kj` et `d_`, tous sur des structs depuis 1a.26/1a.28 ; `test_pico8_g_`/`test_pico8_jo` lui passent des structs).

→ **Après la conversion de `ui`, aucune instance n'atteint `dg` ni `ke_ks`.** Ils passent à la forme struct seule ici. `test_pico8_entity_smoke` instancie encore `ke` et une fille de `ke` sous forme instance (`:253`, `:298`) : il n'appelle ni `dg` ni `ke_ks`, mais il dépend du `pq` de la classe `ke` (`lt`), qui reste donc jusqu'à 1a.42.

**→ Membres de `ui`** :
- les 4 défauts, ordre de la spec : `ql`, `fa`, `le`, `hw` ;
- les 6 champs écrits : `qa`, `ic`, `o_`, `gj`, `lheld`, `rheld`.

**→ Méthodes** : `ev` seule par le descripteur. `lj`, `gw`, `bh`, `jz`, `cj` ne sont jamais cherchées par leur nom (mesuré : aucun `GET` de ces noms) : elles restent des fonctions appelées directement.

**Tests qui touchent `ok`** (grep) :
- **les 7 tests de régression nommés**, par `pico8_instance_get_field(ok, …)` :
  - `built_planet_panel_nil_row:79` (`le`) ; `ga_export_panel_by:36` (`le`) ; `planet_gh_row_lq:45` (`le`) ; `rich_planet_exports:88` (`le`) ; `tech_screen:69` (`le`) ; `pause_menu_music:63,74` (`o_`, `le`) ; `slipway_drag:66` (`lheld`) ;
  - chacun déclare `pico8_instance_t *ok = pico8_ok()` ;
- `test_pico8_jc:431` écrit `o_` par `pico8_entity_set_field(pico8_ok(), …)` ;
- `test_pico8_mo:127`, `test_pico8_nf:256`, `test_pico8_je:103` déclarent `pico8_instance_t *ok = pico8_ui_new(NULL)` puis appellent `pico8_ui_lj(&mem, ok)` ;
- `test_pico8_bg`, `fq`, `fv`, `ie`, `jc`, `js`, `pj` lisent `ok` par l'API à double forme, et appellent `pico8_ui_lj`/`pico8_ui_ev`/`pico8_ui_jz` sur `pico8_ok()` : ils compilent tels quels si `pico8_ok()` rend le type de `self` de ces fonctions ;
- aucun test ne lit `pico8_ui_class()->defaults`.

**Outils** : `tools/float_audit_c1.txt` ne nomme aucune expression de `pico8_ui.c`, `pico8_ke.c` ou `pico8_dg`. `pico8_ui.c` porte 4 `PICO8_REF_AS_LIST_CONST` (`:201`, `:214`, `:405`, `:443`), gardés.

## Solution

1. **Enum : inchangée.**
2. **Struct `ui`**, publique dans `pico8_ui.h` : `pico8_ke_t` en premier membre, puis `ql`, `fa`, `le`, `hw` (ordre de la spec), puis `qa`, `ic`, `o_`, `gj`, `lheld`, `rheld`.
3. **État de la classe**, lu **une fois** à l'init dans la table propre de la classe `ui`, **après** l'échange de `le`, jamais re-parsé : les 4 défauts, avec un assert « chaque défaut est une clé de la spec de `ui` ».
   - `le` est **la même REF** que `g_ui_class->defaults` (la liste vide de l'échange), assertée `LIST` (message porteur) ; `hw` aussi (la boîte `b()`, sans tag) ;
   - l'échange de `le` reste dans `pico8_ui_init` (précédent `ga.by`) : la classe garde ses défauts de spec, et la liste ;
   - `pico8_ui_shutdown` remet l'état à nil avant de libérer la classe.
4. **Accesseur public** `pico8_ui_hw()` : rend `const pico8_qz_box_t *`, la boîte de la classe, NULL hors init..shutdown. `pico8_probe_ui_hw_call` (`probe.c`) le lit au lieu de `ui_class->defaults` : même garde (NULL → « pas dans le panneau »). Plus aucun `->defaults` dans `pico8_probe.c` pour `ui`.
5. **Constructeur de `ui`** (`static`, posé par `set_struct_new`) :
   1. alloue un `pico8_ui_t` ;
   2. `pico8_ke_struct_init(&self->ke, &<descripteur de ui>)` ;
   3. copie `ql`, `fa`, `le`, `hw` depuis l'état, met les 6 champs écrits à nil ;
   4. **asserte** qu'aucune clé n'est passée (message porteur, arrêt §7) ;
   5. enregistre `"ui"` au crochet de couverture ;
   6. appelle `pico8_ke_struct_pq`.
6. **Descripteur de `ui`** (`static const`) :
   - 1 entrée : `EV` = le dispatcher existant. Les autres NULL (en particulier `PH`, `LDOWN`, `RDOWN`, `HD`) ;
   - **lecteur** : les 10 membres, nil = absent, puis `pico8_ke_field` ;
   - **écrivain : NULL** (aucun écrivain générique n'atteint `ui` dans le jeu, mesuré) ;
   - accesseur public `pico8_ui_struct_class()`.
7. **Clés construites** (`bh`) :
   - `bt` reste `const char *`, asserté `"l"` ou `"r"` (message porteur ; même assert que `pico8_ui_je_button_table`) ;
   - `bt` choisit **un pointeur de membre** (`&self->lheld` ou `&self->rheld`) et **un nom de méthode littéral** (`"ldown"` ou `"rdown"`), passé à `jz` tel quel. Les deux `snprintf` et leurs tampons disparaissent ;
   - lecture, pose (`kn` REF) et remise à nil passent par ce pointeur. Rien d'autre ne change dans `bh`.
8. **Fonctions de `ui` sur la struct** :
   - `pico8_ui_lj`, `pico8_ui_ev`, `pico8_ui_gw`, `pico8_ui_bh`, `pico8_ui_jz`, `pico8_ui_cj` prennent `pico8_ui_t *` (`const` pour `ev`), et lisent ou écrivent les membres directement. Même sémantique de nil (REF NULL) ; mêmes gardes `self == NULL` ;
   - `pico8_ui_field_raw`/`pico8_ui_field_vec` restent pour les lectures sur les **autres** entités (`je`, `mo`, `o_.ql`, `o_.fg`) ;
   - `pico8_ui_jz` garde `pico8_instance_t **out_kv` : `kv` est une entité de `ic`, pas un `ui` ;
   - le dispatcher `ev` ne change que son cast interne ;
   - **retirée** de `pico8_ui_init` : l'installation `ev`.
9. **Types publics** : `pico8_ui_new` rend `pico8_ui_t *` ; `pico8_ok()` rend `pico8_ui_t *` et `pico8_ui_set_ok` le prend. `pico8_ui_shutdown` libère `g_ok` par `free` (la struct seule, comme `pico8_instance_free` ne libérait que la table de champs, pas ce qu'elle référençait ; TROU-7 inchangé).
10. **Consommateurs de `ok` hors du module** :
    - `pico8_pz` (`msgbox.c`) écrit `ok->le` directement (deux sites) ; son `p` reste à double forme (1a.41) ;
    - `pico8_jc_hq` (`slipway.c`) : type de `ok` seulement (la lecture de `o_` passe déjà par l'API à double forme) ;
    - `pico8_jl` (`bootstrap.c`) : type de `real_ok`.
11. **Fermeture de `ke`** (dernière descendante) :
    - `pico8_ke_ks` : forme struct seule. Branche instance retirée, assert d'entrée « `self` non NULL est une struct » (message porteur) ; signature inchangée (précédent 1a.28) ;
    - `pico8_dg` : forme struct seule. L'assert de tag de `hv` se resserre à « NULL ou struct » (message porteur), la branche instance de l'écriture de `t` est retirée ; les lectures `me`/`p_`/`t` restent par `pico8_entity_get_field` (le descripteur de la fille peut répondre `p_`) ;
    - le `pq` de la classe `ke` (`lt`) **reste** : `test_pico8_entity_smoke` en dépend (instances de `ke` et d'une fille de test), il part avec ce test et `pico8_class.c` à 1a.42 ;
    - `pico8_lt`, `pico8_he`, `pico8_mv`, `pico8_dc`, `pico8_entity_get_field`/`set_field` ne changent pas : API générique de la couche, encore traversée par des instances sans parent `ke` (`fm`, `hy`, `lq`, `trade`, `hp`, enveloppes), fermée à 1a.42.
12. **Tests** : nouveau `test_pico8_ui` (caractérisation dirigée et rejeux courts) ; 7 tests de régression réécrits (lecture seulement) et prouvés rouges ; `test_pico8_jc`, `mo`, `nf`, `je` adaptés dans leur forme seulement. Voir Testing Decisions.
13. **Plan** : à la ligne 1a.31 de `docs/plan-3.md`, ajouter « Découvert à 1a.31 : … », réduit à ce qui change une étape suivante (§7) :
    - `ke_ks`/`dg` struct seules ; le `pq` de la classe `ke` reste pour `test_pico8_entity_smoke`, retiré à 1a.42 ;
    - `hw` : lu sur la classe par l'accesseur `pico8_ui_hw()` ; `le` : REF de la classe prise après l'échange ;
    - pas d'écrivain ; enum inchangée.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `ui` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `trade`, `hp`, `lq`, `hy` et les instances sans classe à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `ui` mesurée (constructions et clés, noms lus, champs écrits, lecteurs de `->defaults`, noms passés à `jz`, valeurs posées dans `lheld`/`rheld`, instances vues par `dg`/`ke_ks`), afin que chaque membre et chaque fermeture soit justifié par une mesure.
3. En tant que lecteur du code, je veux une struct `ui` dont le premier membre est `pico8_ke_t` et qui n'ajoute que les 10 noms mesurés, minifiés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux que la clé construite `bt.."held"` devienne un choix entre les membres `lheld` et `rheld`, afin qu'aucune écriture par nom calculé ne reste dans `ui` (§5, « Clés construites à l'exécution »).
5. En tant que mainteneur, je veux que `bt.."down"` devienne un choix entre les noms littéraux `"ldown"` et `"rdown"`, déjà indexés par l'enum, afin que la méthode appelée sur les entités survolées soit nommée sans construction de chaîne.
6. En tant que mainteneur, je veux qu'un `bt` autre que `"l"` ou `"r"` déclenche une assertion porteuse, afin qu'une clé hors de l'ensemble fini mesuré soit un arrêt (§7).
7. En tant que joueur, je veux que glisser un slipway d'une planète à l'autre au bouton gauche marche comme avant (pose de `lheld` par `ldown`, `hq` à chaque frame, `ey` sur la cible trouvée par `jz("gv")` au relâchement), afin que `slipway_drag` reste identique.
8. En tant que joueur, je veux que le bouton droit fasse défiler le fond (`bg:rdown` → `rheld`) et que le curseur devienne le sprite 9 tant qu'il est tenu, afin que `long_session` reste identique.
9. En tant que joueur, je veux que le survol surligne l'entité du dessus (`m_`), règle le motif de remplissage (`fg`) et choisisse le sprite du curseur (`o_.ql`, sinon 58), afin que l'écran ne change pas d'un pixel.
10. En tant que joueur, je veux qu'un clic hors d'un menu ouvert le ferme (`cj` quand `je.l.jn` et `le` inchangé), et qu'un clic sur une ligne du menu la déclenche (`gu(le, o_)`), afin que menus, écran technologie et panneaux de planète se comportent comme avant.
11. En tant que mainteneur, je veux que `le` d'un `ui` neuf soit la liste de la classe (même REF), lue une fois après l'échange de l'init, afin que la comparaison d'identité `self.le == le` de `lj` et le premier `cj` voient ce qu'ils voyaient.
12. En tant que mainteneur, je veux que `hw` soit lu sur la classe par un accesseur, et que `pico8_probe.c` ne lise plus `->defaults`, afin que la règle « Accès directs aux défauts » soit tenue pour `ui`.
13. En tant que mainteneur, je veux que `pz` écrive `ok.le` dans le membre, afin qu'aucun site hors de `pico8_class.c` n'écrive plus les champs de `ok` par la table d'une instance.
14. En tant que gardien du filet, je veux que `ok` s'enregistre dans `hv` et `cv.ev` au même rang qu'avant, dans aucun `cg`, et soit dessiné en couche 15 par `dc("ev")`, afin que le curseur reste par-dessus tout.
15. En tant que mainteneur, je veux que le descripteur n'ait que `EV`, et pas d'écrivain, afin qu'une écriture générique jamais mesurée sur `ok` soit un arrêt.
16. En tant que mainteneur, je veux qu'une clé de construction passée à `pico8_ui_new` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt.
17. En tant que mainteneur, je veux que la classe `ui` n'ait plus d'installation, afin que rien ne puisse plus y résoudre `ev`.
18. En tant que mainteneur, je veux que `pico8_ui_new`, `pico8_ok`, `pico8_ui_set_ok` et les fonctions `lj`/`ev`/`gw`/`bh`/`jz`/`cj` prennent ou rendent `pico8_ui_t *`, afin que le type dise ce que la mesure a prouvé : leur `self` est toujours un `ui`.
19. En tant que mainteneur, je veux que `lj` continue de courir sur l'ancien `ok` quand `jl()` le remplace pendant un clic, afin que la reconstruction en cours de frame reste sans use-after-free.
20. En tant que mainteneur, je veux que `ke_ks` et `dg` passent à la forme struct seule, avec une assertion d'entrée, afin que la double forme ouverte à 1a.2 soit soldée au commit de la dernière descendante de `ke` (§5).
21. En tant que mainteneur, je veux que le `pq` de la classe `ke` reste tant que `test_pico8_entity_smoke` vit, afin de ne pas casser un test de la couche avant le commit qui le supprime (§6).
22. En tant que gardien de la couverture, je veux que le constructeur enregistre `"ui"` une fois par `ui`, afin que `class-coverage-net` soit identique ligne à ligne (`ui 38 15/15`).
23. En tant que gardien des tests de régression, je veux que les 7 tests nommés ne changent que leur façon de lire `ok`, et que chacun soit prouvé rouge avec son bug remis, afin que la réécriture ne les ait pas vidés de leur objet.
24. En tant que vérificateur, je veux une caractérisation de `ui` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet voit peu : `rheld`, la liste partagée `le`, `jz` par couches, l'ancien `ok` après `jl()`.
25. En tant que vérificateur, je veux un rapport `gcov` montrant chaque ligne ajoutée ou modifiée exécutée, et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
26. En tant que gardien des caractérisations, je veux que `test_pico8_jc`, `mo`, `nf`, `je` ne changent que leur forme (type de `ok`, écriture directe de `o_`), sans valeur figée touchée, afin que leurs étapes restent prouvées.
27. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne des références de couverture ni de `float_audit_c1.txt` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
28. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C, un piège ou un `TROU-N` nu (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
29. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
30. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** :
  - le répartiteur `ui` : struct, état, accesseur `hw`, constructeur, descripteur, lecteur, fonctions sur la struct, installation `ev` retirée, shutdown ;
  - la sonde (`probe.c`) : `pico8_probe_ui_hw_call` sur l'accesseur ;
  - `pz` (`msgbox.c`) : écriture de `ok->le` ; `jc:hq` (`slipway.c`) et `jl` (`bootstrap.c`) : types ;
  - `ke` (`ke.c`) et `dg` (`bootstrap.c`) : forme struct seule.

  Rien d'autre dans `src/`. `cart.c:264` ne change pas de texte. Pas de changement dans `entity`, `pico8_struct.h` (enum), `mouse_reader` (`je`), `scroll` (`mo`), `ui_button`, `menu`.
- **Clés construites : deux membres et deux littéraux, `bt` gardé en chaîne** (G). Le plan nomme la cible (`lheld`/`rheld`, `ldown`/`rdown`) ; un paramètre `bt` en chaîne asserté garde `lj` et la signature de `bh` tels quels, et l'assert rend l'ensemble fini vérifiable. Rejeté : un enum de bouton (surface d'API nouvelle pour deux appels littéraux) ; `jz` par index de méthode (`pico8_mv` résout déjà le nom par l'enum, et `jz` est aussi appelé avec `"gv"` et par les tests).
- **`le` : membre qui porte la REF de la classe, pas un état statique partagé** (G). Le défaut n'est jamais modifié en place (`gu`, `nv` le lisent), et il n'est jamais lu sur la classe : ce n'est pas un « défaut muté » du §5. Sa REF est prise dans la table de la classe **après** l'échange de l'init, et non re-créée par instance : deux `ui` neufs partagent la même liste vide, comme dans le cart. Rejeté : une liste neuve par instance (identité changée ; invisible au filet, que la caractérisation fige).
- **`hw` : membre + état de la classe lu par accesseur** (G, précédent `pj.gs`/`bg.pl`). Le cart le lit sur la classe (`ui.hw`) ; le membre suit la règle générale « chaque défaut est un membre ». `bv` lit l'accesseur, jamais `ok`. Le membre n'est lu par personne aujourd'hui ; il est gardé pour que le lecteur réponde `hw` sur `ok` comme la chaîne.
- **Pas d'écrivain** (G) : 0 écriture générique sur `ok` dans les 18 rejeux. Le seul site, `test_pico8_jc:431`, pose `o_` pour une caractérisation : il écrit le membre directement (précédents 1a.27, 1a.29). `gw` écrit `m_` sur les entités de `cv.hd`, jamais sur `ok` (`ui` n'a pas `hd`).
- **Aucune clé acceptée** (G) : une seule voie, `pico8_ui_new(NULL)`, dans les rejeux comme dans les 466 constructions des TU.
- **Nil = absent est invisible pour `ui`** : aujourd'hui `o_`, `gj`, `lheld`/`rheld` écrits à nil restent présents à nil dans la table (TROU-9). Tous leurs lecteurs mesurés (`field_raw` de `ui`, `jc:hq` par `pico8_jc_entity_instance`, les tests par des `lookup` qui confondent nil et absent) rendent la même chose pour les deux. Aucun `TROU` nouveau.
- **Ordre du constructeur** : membres avant `ke:pq`. Copier les défauts **après** `lt` est un **mutant équivalent** (`lt` ne lit que `bm`, servi par le descripteur, et `nu`, absent) : on ne l'utilise pas comme mutation (leçon de 1a.23-1a.29).
- **Types des fonctions propres** (G) : leur `self` est toujours un `ui` (mesuré, y compris l'ancien `ok` après `jl()`). Retypage de `self`, comme `pj` à 1a.29 ; pas le cas des fonctions partagées de `g_`/`ke`, qui gardent leur signature.
- **Fermeture de `ke` par assert, sans retypage** (G, précédent 1a.28) : `ke_ks` garde `pico8_instance_t *self` (ses appelants passent des structs castées), `dg` garde son parcours de `hv`. Retirer la branche instance et asserter rend la fermeture vérifiable (`gcov`, `SIGABRT` dans un fils).
- **Le `pq` de la classe `ke` reste** (G) : le retirer casserait `test_pico8_entity_smoke`, test de la couche que §6 supprime avec `pico8_class.c` (1a.42). Aucune instance du jeu ne l'atteint plus après ce commit (mesuré : `DG_INSTANCE` uniquement `ui`).
- **`pz`, `pico8_jc_entity_instance`, l'API générique** ne changent pas de forme : `pz` est à double forme pour son `p` jusqu'à 1a.41.
- **Tests de régression** : chacun remplace `pico8_instance_get_field(ok, …)` par `pico8_entity_get_field(ok, …)`, et le type de `ok` par `const void *` (ou `pico8_ui_t *`). Rien d'autre ne change (frames, fixtures, asserts, messages). **Preuves rouges**, bug remis sur le code final, test rouge, correctif restauré, vert :

  | Test | Lecture réécrite | Bug remis | Attendu |
  |---|---|---|---|
  | `built_planet_panel_nil_row` | `ok.le` (`:79`) | Preuve à deux changements de 1a.17 : (i) le lecteur du descripteur de `jo` rend « présent » un `ep`/`fo`/`nk` nil ; (ii) clause REF-NULL de `pico8_msgbox_truthy` retirée (3b8ba2c). Contrôles (i) seul, (ii) seul → vert. | `built_rows == BUILT_PANEL_EXPECTED_ROWS` faux |
  | `ga_export_panel_by` | `ok.le` (`:36`) | Échange de `by` en liste retiré de l'init de `ga` (9183fee ; preuve de 1a.26). | Abort à l'assertion de tag `LIST` (ou dans `PICO8_REF_AS_LIST_CONST`) ; noter lequel |
  | `planet_gh_row_lq` | `ok.le` (`:45`) | Cas `LIST` de `pico8_js_lq_arg_table` remplacé par le cast aveugle (8361468 ; preuve de 1a.16). | Rapport ASan heap-buffer-overflow, à défaut les asserts de champ |
  | `rich_planet_exports` | `ok.le` (`:88`) | Borne de `bb_visit` par `po->count` au lieu de la longueur de séquence (50f91b4 ; preuve de 1a.16). | `mu4 == 3` faux |
  | `pause_menu_music` | `ok.o_` (`:63`), `ok.le` (`:74`) | Pas de correctif d'historique ; bug de référence de 1a.16 : dans `pico8_ix4_my_dispatch`, `pz` appelé **avant** les quatre installations de `my`. | `hovered_my` absent à la frame 20 |
  | `tech_screen` | `ok.le` (`:69`) | Pas de correctif d'historique (test de scénario). **Bug de référence** (G, précédent `planet_kj_dispatch` 1a.8) **dans `ui`** : `lj` n'appelle `bh("l")` que si `#le == 0` (alternative `gu(le, o_)` retirée), donc un clic sur une ligne d'un menu ouvert est ignoré. | `invented_count() == 1` faux à la frame 46 |
  | `slipway_drag` | `ok.lheld` (`:66`) | 2d7021f, **dans `bh`** : nil passé à `ey` au lieu de l'entité trouvée par `jz("gv")` (preuve de 1a.17). | `oe_count(origin) == 1` faux |

  La commande (`make tmp/test_pico8_<nom> && ./tmp/test_pico8_<nom>`, `ASAN_OPTIONS=detect_leaks=0` pour `planet_gh_row_lq`) et le mode d'échec observé sont notés dans le commit. **Si un bug remis ne rend pas son test rouge, c'est un arrêt (§7), pas un test durci.**
- **Tests adaptés dans leur forme** : `test_pico8_jc:431` écrit `((pico8_ui_t *)pico8_ok())->o_` directement ; `test_pico8_mo`, `nf`, `je` typent `ok` en `pico8_ui_t *`. Aucune valeur figée ne bouge.
- **Commentaires** (CLAUDE.md) :
  - à ajouter : le contrat de la struct `ui` (le `ok` sortant n'est jamais libéré, TROU-7 ; `le` d'un `ui` neuf est la liste de la classe, partagée ; `hw` est la boîte de la classe, empruntée) ; celui de `pico8_ui_hw()` (NULL hors init..shutdown) ; l'assert de `bt`, l'assert de clé, l'assert `LIST` de l'init, les deux asserts de fermeture (`ke_ks`, `dg`) ;
  - à corriger ou retirer, rendus faux : `pico8_ui.h` sur `pico8_ui_new` (« `overrides` is copied, not kept ») ; « Throughout this module a NULL instance […] an absent or wrong-kind field reads as absent » (à revoir pour la struct) ; le commentaire de `pico8_ui_bh` (« concatenated at runtime into the field name ») ; celui de `pico8_probe_ui_hw_call` (« read off the `ui` class defaults ») ; celui de `pico8_ke_ks` et de `pico8_ke.h` s'ils nomment encore une instance ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert (et `make e2e` au dernier). La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_ui` sur l'ancien code** (voir Testing Decisions), valeurs figées. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si le `ok` d'une partie n'a pas `ql`=58 et `fa`=15, si son `le` n'est pas la REF de la classe avant le premier `cj`, ou si `bh` voit un `bt` hors de `l`/`r` : la mesure est fausse et doit être corrigée dans le PRD.
2. **`ui` converti** :
   - struct, état, accesseur `hw`, constructeur, descripteur, lecteur, `set_struct_new`, installation `ev` retirée, shutdown ;
   - `lj`/`ev`/`gw`/`bh`/`jz`/`cj` sur la struct, clés construites remplacées ;
   - `pico8_ui_new`/`ok`/`set_ok` typés ; `pz`, `jc:hq`, `jl`, `pico8_probe_ui_hw_call` adaptés ;
   - `test_pico8_jc`, `mo`, `nf`, `je` adaptés dans leur forme.

   Toutes les caractérisations passent **sans changer leurs valeurs figées**. Les vérifications de forme s'ajoutent à `test_pico8_ui`.
3. **Fermeture de `ke`** : `ke_ks` et `dg` à forme struct seule ; leurs deux vérifications d'abort s'ajoutent à `test_pico8_ui`.
4. **Tests de régression** : les 7 réécrits (lecture de `ok`), puis leurs 7 preuves rouges, correctif restauré.
5. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.31 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF, le contenu des listes. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_ui`** : lié comme `test_pico8_pj`, sur le cart réel, avec des dépendances Makefile sur les fixtures qu'il rejoue.
  - Il lit **uniquement** par l'API à double forme, et par `pico8_class_get_field(pico8_ui_class(), …)` pour les **défauts de spec**, jamais pour `ev` (l'installation disparaît au morceau 2).
  - Il appelle les fonctions publiques `pico8_ui_lj`/`ev`/`gw`/`bh`/`jz`/`cj` sur `pico8_ok()`, sans déclarer le type de `ok` (le même code compile avant et après).
  - Son code est le même avant et après, aux vérifications de forme près.
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()`, pour l'état de `rnd`.
  - La partie est lancée comme le filet (`pico8__init()`, puis `pico8_eu` sur `bo[2]`).
  - Toute présence se lit par un `lookup` qui confond nil et absent.
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **Construction** :
       - `pico8_ok()` est dans `hv`, à un rang figé ; dans `cv.ev`, pas dans `cv.kj`/`hd`/`hk`/`hb` ; dans aucun `cg` ;
       - `ql`=58, `fa`=15, `me`=`"ph"`, `t` ;
       - absents : `qa`, `ic`, `o_`, `gj`, `lheld`, `rheld`, `ra`, `p_`, `nu`, `ph`, `ldown`, `rdown`, `hd`, `fx` ;
       - `ev` présent et de type fonction ;
       - **identité** : `le` et `hw` lus sur `ok` sont les REF que rend `pico8_class_get_field(pico8_ui_class(), …)` ; `le` est une liste vide.
    2. **Partage de `le`** : un second `pico8_ui_new(NULL)` voit la **même REF** `le` ; après `pico8_ui_cj` sur le second, son `le` est une liste neuve vide, et celui de `ok` (et de la classe) est inchangé ; `pz` sur un menu (`ix`) remplace `ok.le` (compte figé), et `ff` le vide.
    3. **Vie** :
       - `lj` sur une partie fraîche : `qa` = `je.ra + mo.ra`, `ic` présent, `gj` = `o_` ; empreinte d'écran de `ev` (sprite 58 à `je.ra`) ;
       - survol d'une planète (position de `je.ra` posée comme `test_pico8_fq`/`fv`) : `o_` = la planète, `m_` vrai sur elle, `gj` au tour suivant, `0x5f80` = son `fg` ;
       - `jz` par couches : entités de `fa` différents sous la souris, la plus haute gagne (`kv` figé) ; `jz("rdown")` sur le fond `bg` (comme `test_pico8_bg:457`) ;
       - bouton droit (`je.r.jn` posé comme `test_pico8_nf`) : `rheld` = le `bg`, curseur sprite 9 (empreinte d'écran), puis relâchement (`hm`) : `rheld` absent ;
       - bouton gauche sur une planète construite : `lheld` = un `jc` ; relâchement sur une autre planète : `ey` reçoit la cible, `lheld` absent ;
       - `cj` : les entités de l'ancien `le` quittent `hv`, `le` est une liste neuve.

       Figée.
    4. **Rejeux courts** (départ comme le filet, jusqu'à la dernière frame) : `slipway_drag_events.json` (`lheld`) et `tech_screen_invent_events.json` (`pz`, `cj`, clic sur une ligne). À chaque frame, on plie : le rang de `ok` dans `hv` et `t` ; la présence et le rang dans `hv` de `o_`, `gj`, `lheld`, `rheld` ; le compte de `le` et son identité (changée ou non depuis la frame précédente) ; le nombre d'entrées de chaque couche de `ic` ; `qa`. S'ajoute une empreinte d'écran finale. Figée. L'implémenteur note dans le commit si un rejeu atteint l'ancien `ok` après `jl()` (mesuré : 12 fois sur les 18 rejeux) ; sinon un scénario dirigé le couvre (clic sur `bo[n]` qui appelle `eu` depuis `lj`).
  - **Forme** (ajoutée aux morceaux 2 et 3) :
    - `ok` est `STRUCT` au descripteur `pico8_ui_struct_class()` ;
    - `EV` est non NULL, les autres NULL ; `field` est posé, `set_field` est NULL ;
    - `pico8_entity_set_field(ok, "o_", …)` avorte (dans un fils) ;
    - `pico8_ui_new` avec une clé (nommée ou positionnelle) avorte ;
    - `pico8_ui_bh(ok, "x")` avorte ;
    - `pico8_ui_class()->struct_new` est posé et son `pq` est NULL ; `ev` est absent de `pico8_ui_class()->defaults` (lecture directe de la table), les 4 défauts de spec y sont ;
    - `pico8_ui_hw()` est la REF de la classe, `ok->hw` aussi ; `ok->le` est celle de la classe sur un `ui` neuf ;
    - `pico8_ke_ks` sur une instance de `ke` (`pico8_class_instantiate(pico8_ke_class(), NULL)`) avorte ; `pico8_dg` avec cette instance dans `hv` avorte (dans des fils).
- **Tests de régression (§6)** : les 7 nommés, réécrits dans leur lecture de `ok` seulement ; preuves rouges du tableau ci-dessus. Tous les autres gardent un diff vide et restent verts.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (copie de l'arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` `test_pico8_ui`, `test_pico8_slipway_drag`, `test_pico8_tech_screen`, `test_pico8_bg`, `test_pico8_fq` et `test_pico8_jc`, avec les objets du moteur. Les lancer, puis `gcov` sur `pico8_ui.c`, `pico8_ke.c`, `pico8_bootstrap.c`, `pico8_probe.c` et `pico8_msgbox.c`.
    - Même piège qu'à 1a.23-1a.29 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`.
    - Les branches d'assert atteintes seulement dans un fils qui avorte (`must_abort`) ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu.
    - Le rapport est collé dans le message de commit : lignes exécutables ajoutées ou modifiées de `src/` et leur compte (état et reset, accesseur, constructeur, lecteur, `lj`/`ev`/`gw`/`bh`/`jz`/`cj`, les deux branches `l`/`r` de `bh`, `pz`, `pico8_probe_ui_hw_call`, `ke_ks`, `dg`). Les lignes à 0 sont données avec leur justification.
  - **2 à 3 mutations**, chacune rouge sur `test_pico8_ui` (ou un autre test nommé), notées dans le commit avec le test qui l'attrape :
    1. **valeur d'un défaut propre** : `ql` non copié (curseur sprite 0) ou `fa` copié dans `ql`. L'empreinte d'écran change ;
    2. **clé construite** (à la place de l'entrée du descripteur, qui n'en a qu'une) : `bt == "l"` choisit `rheld`. Le curseur passe au sprite 9 pendant un glissé, `slipway_drag` et le scénario 3 passent au rouge ; à défaut, `EV` = NULL (le curseur n'est plus dessiné) ;
    3. **état partagé** (à la place de l'ordre du constructeur, mutant équivalent ici) : `le` re-créé par instance au lieu de la REF de la classe. L'identité (scénarios 1 et 2) passe au rouge.
- **Tests existants à diff vide** : `test_pico8_bg`, `fq`, `fv`, `ie`, `js`, `pj`, `g_`, `mz`, `qb`, `iv`, `mh`, `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`, `bg_kj`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage`, et les tests de régression non nommés.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED` (les 4 `PICO8_REF_AS_LIST_CONST` de `pico8_ui.c` restent).
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** :
  - `test_pico8_pj`/`test_pico8_fv` : structure, `lookup`, `must_abort`, fils par scénario, rejeu court, forme ;
  - `test_pico8_fq`, `test_pico8_bg`, `test_pico8_nf` : survol, `jz` par `ldown`/`rdown`, pose de `je.l`/`je.r` ;
  - `test_pico8_jc` et `slipway_drag` : glissé d'un slipway, `ok.lheld` ;
  - 1a.28 : fermeture d'une parente par assert d'entrée, sans retypage ;
  - 1a.16/1a.17/1a.26 : les preuves rouges reprises dans le tableau.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; c'est le seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` (en particulier `ui  38  15/15`). Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations (défaut propre, clé construite ou entrée du descripteur, état partagé) ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier :
  - si `bh` accepte un `bt` autre que `"l"`/`"r"` sans avorter, ou construit encore un nom par concaténation (`snprintf`, `strcat`) ;
  - si l'assertion de clé ou celle de `bt` se déclenche sur un rejeu ou un test hors `must_abort` ;
  - si un membre autre que `ql`, `fa`, `le`, `hw`, `qa`, `ic`, `o_`, `gj`, `lheld`, `rheld` est ajouté ;
  - si un écrivain est posé ;
  - si le constructeur accepte une clé ;
  - si l'enum change.
- `FAILED:` si `le` d'un `ui` neuf n'est pas la REF de `pico8_ui_class()->defaults` (liste re-créée par instance ou re-parsée), ou si `hw` n'est pas la REF de la classe.
- `FAILED:` si un site hors de `pico8_class.c` et de `pico8_ui_init` lit ou écrit encore `pico8_ui_class()->defaults` (grep ; en particulier `pico8_probe.c`), si une installation reste dans la classe `ui`, ou si la classe `ui` cesse d'être enregistrée avec ses défauts de spec.
- `FAILED:` si un site hors de `pico8_class.c` passe `pico8_ok()` (ou un `ui`) à `pico8_instance_fields` ou `pico8_instance_get_field` (grep dans `src/` et `tests/`).
- `FAILED:` si une autre classe que `ui` est convertie ou change de forme (`trade`, `hp`, `lq`, `hy`, `fm`, les enveloppes…), ou si le comportement d'une autre classe change (valeur figée de sa caractérisation, ligne de couverture).
- `FAILED:` si une entrée du descripteur autre que `EV` est non NULL, ou si `EV` est NULL.
- `FAILED:` si `pico8_ke_ks` ou `pico8_dg` acceptent encore une instance, gardent une branche propre à l'instance, changent de signature ou de comportement sur une struct. Également si le `pq` de la classe `ke` est retiré, ou si `pico8_lt`/`he`/`mv`/`dc`/`pico8_entity_*`/`pz` changent de forme.
- `FAILED:` si l'un des 7 tests nommés change au-delà de sa lecture de `ok` (type de `ok`, `pico8_instance_get_field` → `pico8_entity_get_field`, includes) : frames, fixtures, asserts ou messages modifiés. Également si le commit ne rapporte pas ses 7 preuves rouges (bug remis, rouge, restauré, vert), ou si un bug remis ne rend pas son test rouge.
- `FAILED:` si un autre test de régression (§6) a un diff non vide.
- `FAILED:` si une valeur figée de `test_pico8_ui` a été changée après la conversion, ou une de `test_pico8_jc`, `mo`, `nf`, `je`, `bg`, `fq`, `fv`, `pj` ou d'une autre caractérisation. Également si `test_pico8_jc`, `mo`, `nf`, `je` changent au-delà du type de `ok` et de l'écriture directe de `o_`.
- `FAILED:` si une ligne est ajoutée à `tools/float_audit_c1.txt` ou en est retirée, ou si `make float-audit` est rouge.
- `FAILED:` si `test_pico8_ui` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.
- `FAILED:` si la note « Découvert à 1a.31 » manque, ou si elle ne dit pas que le `pq` de la classe `ke` reste jusqu'à 1a.42.

Critères de réussite (en plus) :

- la struct `ui` n'a que `pico8_ke_t` et les 10 membres ; son descripteur a 1 entrée et pas d'écrivain ;
- grep :
  - `PICO8_CLASS_COVERAGE_NAME(g_ui_class, "ui")` reste ;
  - `pico8_class_set_struct_new(g_ui_class, …)` est présent ;
  - aucun `set_name(g_ui_class->defaults, "ev", …)` ;
  - aucun `"%sheld"` ni `"%sdown"` ;
  - aucun `->defaults` dans `pico8_probe.c` ;
  - aucun `pico8_instance_fields` dans `pico8_ui.c`, `pico8_msgbox.c` (sur `ok`) ni `pico8_bootstrap.c`, ni dans `pico8_ke.c` ;
- le `ok` d'une partie est une struct au descripteur de `ui`, dans `hv` et `cv.ev` ;
- la ligne 1a.31 de `docs/plan-3.md` porte la note « Découvert à 1a.31 », et le message de commit dit « découvert à 1a.31 ».

## Out of Scope

- Les autres classes et instances sans classe : `trade` (1a.32), `hp` (1a.33), `lq` (1a.34), `hy` (1a.35), `fm` (1a.36), les `{}`/`et={}`/`{qs}`/`re` jetables (1a.37-1a.40), l'enveloppe de `pz` sur `g_ha` (1a.41).
- Le `p` de `pz` (double forme jusqu'à 1a.41), et les lectures de `je`/`mo` par `ui` (déjà des structs, inchangées).
- Supprimer la classe `ui`, le `pq` de la classe `ke`, les registres et `test_pico8_entity_smoke` : 1a.42.
- Fermer l'API générique (`lt`, `he`, `mv`, `dc`, `pico8_entity_*`) : 1a.42.
- Typer les membres (`lheld` en pointeur d'entité, `le` en `pico8_list_t *`, `hw` en boîte) : phase 1b.
- Libérer `ok`, `le`, `ic`, `qa` réalloués chaque frame (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Dernière descendante de `ke`** : après ce commit, aucune instance du jeu ne descend de `ke`. La double forme posée à 1a.2 dans `ke_ks` et `dg` se solde ici. Ce qui reste de `ke` sous forme de classe (sa `pico8_class_t`, son `pq` `lt`) ne sert plus qu'à `test_pico8_entity_smoke` et aux assertions de parenté de `cart.c`.
- **Ce que le filet voit de `rheld`, peu de chose** : 7 poses dans `long_session` seulement (défilement au bouton droit). Une confusion `lheld`/`rheld` se verrait surtout au curseur (sprite 9) pendant un glissé gauche, c'est pourquoi la mutation 2 la vise et le scénario 3 couvre le bouton droit directement.
- **Ce que le filet voit de `le` partagé, rien** : `jl()` reconstruit `ok`, et le premier menu (`pz`) appelle `cj`, qui remplace `le` sur l'instance. Une liste par instance au lieu de celle de la classe échapperait au filet ; le scénario 2 et la mutation 3 la visent.
- **`hw` et la sonde** : `hw` est une boîte `b()` appelée comme fonction (`pico8_qz_call`). Elle n'a pas de tag : l'accesseur ne peut pas asserter son genre, seulement sa présence dans la spec.
- **Leçons de 1a.23-1a.30 appliquées** :
  - pas de mutation sur un mutant équivalent (ordre `lt`/défauts) ;
  - piège `_exit` des fils réglé dans la copie jetable seulement ;
  - pas de membre pour un nom seulement lu et toujours absent (`ra`, `p_`, `nu`, `ph`, `fx`, `fz`, `mb`, `nl`, `hz`, `px`) ;
  - écriture directe dans le membre depuis un test plutôt qu'un écrivain élargi ;
  - fermeture par assert d'entrée, sans retypage des fonctions partagées.
- Mesures :
  - grep sur `src/`, `tests/`, `tools/` et le `Makefile` à a95bb03 ;
  - lecture de `pico8/lua.lua:150-161,168-181,212-257,279-287,550-557,622,1187` ;
  - lecture de `pico8_ui.c/h`, `pico8_ke.c/h`, `pico8_bootstrap.c`, `pico8_probe.c` (`hw`), `pico8_msgbox.c` (`pz`), `pico8_slipway.c` (`jc:hq`), `pico8_cart.c`, `pico8_struct.h`, `pico8_entity.c`, `pico8_class.c`, des 7 tests nommés et de `test_pico8_jc`/`mo`/`nf`/`je`/`bg`/`fq`/`fv`/`entity_smoke`, de `tools/class_coverage_net_expected.txt` ;
  - lecture des PRD de 1a.16, 1a.17, 1a.26, 1a.28, 1a.29 et de `src/pico8_ui.c` à 34662a8 (`le` à `:102`) ;
  - sonde dans une copie jetable du scratchpad, supprimée après usage : 18 rejeux en sortie 0 ; 38 `ui` sur le filet, tous par `pico8_ui_new(NULL)` ; aucun `pico8_entity_set_field` sur `ok` ; `hw` jamais lu sur une instance ; `jz` ne reçoit que `ldown`, `rdown`, `gv` ; `lheld` toujours un `jc` struct, `rheld` un `bg` struct ; `dg` ne voit que `ui` comme instance, `ke_ks` aucune ;
  - fil piège (`make unit` puis `UNIT_TESTS_SLOW`, même copie, sortie 0) : 466 `ui` sans clé ; une seule écriture générique (`test_pico8_jc:431`, `o_`) ; aucune autre instance dans `dg` ni `ke_ks`.
