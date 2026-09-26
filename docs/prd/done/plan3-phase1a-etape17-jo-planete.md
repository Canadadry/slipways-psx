---
title: "Plan 3, étape 1a.17 : `jo` (la planète) devient une struct, ses fonctions restent à double forme pour ses neuf filles non converties"
description: "La planète `jo` est encore une instance de la couche objet, construite 90 fois par partie par `bw`, alors que neuf filles non converties (`qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`) héritent de sa `pico8_class_t`, de ses défauts et de ses onze méthodes. Elle passe en struct fille de `ke` : 31 membres mesurés, un descripteur qui réutilise les onze dispatchers installés dans `defaults` (l'enum gagne `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL`), des défauts pris par REF dans `cls->defaults` à l'init (`kw` en état statique partagé), trois asserts de tag élargis, et sept tests de régression relus puis prouvés rouges. Aucune fonction de `jo` ne passe à la forme struct seule avant 1a.26."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.17 — `jo` : `ke` (`planet.c:140`), défauts lus `planet_display.c:184,296,532`, `research_display.c:265-266`. Tests : `nil_row`, `empty_planet_cleared`, `planet_gh_row_lq`, `planet_kj_dispatch`, `playability`, `rich_planet_exports`, `slipway_drag`.

(À HEAD fb67ce9, la déclaration est `planet.c:141`, et la lecture de `research_display.c` est aux lignes 266-267 : `lookup_named("jo")`, puis `->defaults` `"em"`.)

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe […]. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Défauts mutés sur la classe** : […] tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** […].
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag (`jo` pour `cg.jo`) ; seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé […]. Les descendants non convertis trouvent leurs champs hérités en remontant `parent`, qui reste valide.
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre (dispatcher hérité dans `defaults`, ou appel direct `jo.kj(self,…)`, `jo.kf(self,…)`) reste à double forme […]. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante (pour `jo` : 1a.26). La caractérisation de 1a.17 lit au moins une instance de chaque fille non convertie construite par le filet (`qh`, et `pm`/`ga` via `kv[hp.cw]`) : champ hérité de `jo` et résultat d'une méthode de `jo` appelée sur elle.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct : une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec. Le PRD de `jo` tranche `kw` (écrit en place par `mx:kf`, `planet_family_trade.c:280`, si l'instance n'a pas couru `lz`) : défaut modifié en place → état statique partagé, pointé aussi par `defaults`.

§6 : chaque test de régression nommé qui lit une struct `jo` est réécrit **dans sa façon de lire l'état** seulement, dans ce commit, et prouvé rouge avec son bug d'origine remis localement, puis restauré. §7 : un commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (`jo  1710  15/15`, et `qh 8`, `nl 1`, `os 1`, `hj 5`, `pm 4`, `mx 3`, `nr 6`, `fr 5`, `ga 7` inchangés) ; un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.17, avec « découvert à 1a.17 » dans le message ; un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.16b ont posé (jusqu'à fb67ce9), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t`. L'enum vaut `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB`. Un nom absent de l'enum, ou dont l'entrée est NULL, est cherché comme champ. Le lecteur rend 1 si le membre n'est pas nil. L'écrivain rend 0 pour un nom sans membre, et `pico8_entity_set_field` asserte alors (« a field under no rule of plan-3 section 5 — stop »).
- `pico8_ke_t`, `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq` (après le `pq` propre). `pico8_ke_ks` et `dg` écrivent déjà `me`/`t` dans le `pico8_ke_t` d'une struct.
- `pico8_class_set_struct_new` : `pico8_class_instantiate` délègue au constructeur **de la classe instanciée seulement** ; une fille non convertie (`struct_new` NULL) est construite comme aujourd'hui, et sa marche des `pq` passe par le `pq` de la `pico8_class_t` de `jo`.
- **1a.16b** : tous les sites de `src/` atteints par une planète (114, fil piège) lisent et écrivent par `pico8_entity_get_field`/`set_field`, ou sont déjà aiguillés sur le tag (`ke_ks`, `dg`, garde de `bv`, garde de `f_`). L'accesseur `pico8_instance_fields` et `pico8_instance_get_field` assertent sur une struct. Les fonctions de `jo` (`planet.c`, `planet_display.c`, `planet_trade.c`) sont donc **déjà à double forme** au sens du §5.
- Modèles : constructeur à clés mesurées qui asserte sur toute autre (`iv`, `fq`, `jc`, `ie`, `js`) ; `pq` propre puis `pico8_ke_struct_pq` (`iv`, `js`) ; `hd` = `gk` comme méthode `HD` (`ie`, `js`) ; table de classe lue sur la classe (`bg.pl`, `mz.jj`) ; copie locale d'un descripteur pour poser un espion (`planet_kj_dispatch`, 1a.8) ; caractérisation figée sur l'ancien code, lue seulement par l'API à double forme dans des fils forkés (`test_pico8_js`, `test_pico8_ie`, `test_pico8_jc`).

### Ce que fait le cart

`pico8/lua.lua` :

```
:384-402  jo=ke:jr([[ nu=o("jo","np","prober"), me="unknown", hf=1,hs=1, fa=6, kq=b(-7,-7,7,7), ma=0,
            ka=o(2,3,9,12), dz=o(…), cq=o(12,36,24,25), kw=o(mu=0), gq=v(0,11),ca=9, em=o(f,3,0,-1), ]])  jo.hd=gk
:403-406  jo:pq   self.oe={} ; if self.hp then self:lz(gb[self.hp]) end
:407-411  jo:hh   nk (jo.ir()/df.ll/jo.oc()), ep, ks(nk.me or "known"), kd
:412-417  jo:ldown  known → fe ; ly → jc({qu=self})
:424-448  jo:gh   … jo.dz[self.mi] … gu(jo.cq,self.kw.qe) …
:451-460  jo:hu   cw → self.p_=true ; self=kv[hp.cw]({p,ra,id}) | go → nk=df[hp.go] | ks("ly"),lz,fx=true
:461-464  jo:lz   hn,ep ; jy=nv(hp.i,mw) ; kw={qe=hp.o,mu=0}
:465-471  jo:kf, jo:kd (ma=1.25), jo:ll (p_=t>60)
:472-514  jo:gv, jo:cn, jo:ib
:515-540  jo:hk, jo.ex (… add(l,jo.em) …), jo:hb (self.lq=…)
:541-551  jo:kj   … jo.ka[self.mi] … self.fo=nil
:744-751  bw(n) : jo({id="p"..id,p=p,ra=p*128}) ; jo.oc,jo.ir=gn(…),jf(…)
:905-1100 qh=jo:jr, ga=jo:jr ; hj:kj → jo.kj(self,p) ; mx:kf → jo.kf(self,sw)
:1244-1296 bx, bd (pt.cc={}, pt.by=pt:ib()), bb (pt.mi=…, pt.kw.mu=… en place, add(pt.jy,…))
```

Points décisifs :

- **Une seule construction d'un `jo` nu** : `bw`, 90 planètes par partie (`id`, `p`, `ra`). Tout le reste construit une **fille** : `kv[hp.cw]` dans `jo:hu` (`lookup_named(cw)`), `gx[]` au travers de `ie.ke`, `ga(…)` dans `pm:pq`.
- **`jo.dz`, `jo.cq`, `jo.ka`, `jo.em` sont lus sur la classe**, jamais sur `self`. `jo.oc`/`jo.ir` sont réaffectés sur la classe par `bw` : déjà un état statique du module (`g_jo_oc`/`g_jo_ir`).
- **`kw` est la seule table de défaut modifiée en place** (grep du cart : `pt.kw.mu=` dans `bb`, `self.kw.qe=` dans `mx:kf`), et seulement sur un objet qui n'a pas couru `lz`. Aucune autre table de défaut (`nu`, `kq`, `ka`, `dz`, `cq`, `gq`, `em`) n'est écrite.
- **`dg` appelle `qc[qc.me]`** : un `jo` dont `me` vaut `"ll"` (entrée `df` `o("ll",45,0,me="ll")`) fait tourner `jo:ll`. C'est le correctif 529f355 que garde `empty_planet_cleared`.
- **Toutes les fonctions de `jo` sont atteintes par une fille** : `qh`, `nl`, `os` héritent des onze ; `nr` surcharge `gv`/`ldown`/`hb`/`cn` mais hérite du reste ; `hj`/`pm` héritent de tout sauf `kj`, et `hj:kj` appelle `jo.kj` ; `mx` surcharge `kf` et appelle `jo.kf` ; `ga` surcharge `kj` et hérite du reste ; le `pq` de `jo` tourne dans la marche de chaque fille. **Aucune fonction de `jo` ne peut passer à la forme struct seule dans ce commit.**

### Emprise réelle de `jo` (mesurée sur fb67ce9)

**Méthode.** Grep de `src/`, `tests/`, `tools/`, lecture des fonctions, puis **sonde de mesure** posée sur une copie `git archive fb67ce9` jetable (jamais dans le dépôt) : `pico8_entity_get_field`/`set_field`, `pico8_mv`, `pico8_instance_get_field`, `pico8_instance_fields` et `pico8_class_instantiate` deviennent, hors de leurs modules, des macros qui journalisent (opération, nom, classe exacte de l'instance, fichier:ligne) chaque accès à une instance dont la chaîne `cls` contient `jo`. Puis `make -j8 test` (sortie 0, `frames-check` 15 conformes) et `make class-coverage-net` (sortie 0, identique). Les accès internes à `pico8_entity.c` (`lt`, `he`, `dc`) ne sont pas journalisés ; ils sont lus dans le code. Le binaire de couverture a ensuite été lancé rejeu par rejeu pour savoir lequel construit quelle fille.

| Aspect | Mesure |
|---|---|
| Constructions d'un `jo` nu | `bw` (`galaxy.c:113`, `pico8_jo_new`, clés **`id`, `p`, `ra`** — seules clés vues), 1 710 sur le filet (90 × 19 parties). Harnais : `pico8_jo_new(NULL)` dans `planet_gh_row_lq` et `planet_kj_dispatch`. `lookup_named("jo")` ne construit rien (`research_display.c:266` lit `em`). Aucun `gx[]`, `kv[]`, `ie.ke` ni `fz` ne construit un `jo` nu. |
| Constructions de filles (inchangées) | `kv[hp.cw]` (`planet.c:339,353`), `gx[]` via `ie.ke`, `ga` par `pm:pq`. Par rejeu du filet : `session_events` → `qh 4`, `nr 5`, `fr 5`, `pm 3`, `ga 3` ; `events_recorded_1790198294` → `hj 5`, `mx 3`, `pm 1`, `ga 3` ; `trading_hub_session` → `nl 1`, `nr 1` ; `long_session` → `os 1`, `qh 2` ; `empty_planet_cleared` → `qh 2` ; `geoharvest_burst` → `ga 1`. Les quatre premiers sont des `RECORDINGS` (`--from-title`). |
| Défauts de `jo` (14) | `nu`, `me`=`"unknown"`, `hf`=1, `hs`=1, `fa`=6, `kq`, `ma`=0, `ka`, `dz`, `cq`, `kw`, `gq`, `ca`=9, `em`. Hérités de `ke` : `t`=0 (`me` surchargé). |
| Défauts mutés | `kw` (table modifiée en place, voir plus haut). `jo.oc`/`jo.ir` : déjà statiques. Aucun autre. |
| Champs écrits sur un `jo` nu (mesurés, 17) | par les fonctions de `jo` : `oe` (`pq`), `hn`, `ep`, `jy`, `kw` (`lz`), `nk`, `ep` (`hh`, `hu`), `fx` (`hu`, `kf`), `p_` (`ll`, `hu`), `ma` (`kd`), `jh`, `jy` (`cn`), `lq` (`hb`), `fo` (`kj`, nil) ; par des **écrivains génériques** sans filtre de classe : `kr` (`hy:lj` sur `cg.jo`, `economy.c:242`), `cc`, `by` (`bd`, `economy_tiers.c:92,107`), `mi` (`bb`, `:218`), `m_` (boucle de survol `gw` sur `cv.hd`, `ui.c:308,313`), `ma` (`ma()`, `entity_geom.c:79`, partagée `qb`/`jo`), `fo` (`bv`, `probe.c:185`, et `jc:hq`, `slipway.c:140`, sur `et`). Et `me`/`t` dans le `pico8_ke_t` (`ke_ks`, `dg`). Écritures nil mesurées : `ep`, `fo`, `nk`, jamais sur un membre qui a un défaut. |
| Membres qui en résultent (31) | après `pico8_ke_t` : défauts `nu`, `hf`, `hs`, `fa`, `kq`, `ma`, `ka`, `dz`, `cq`, `kw`, `gq`, `ca`, `em` ; clés de construction `id`, `p`, `ra` ; écrits par les fonctions de `jo` `oe`, `hn`, `ep`, `jy`, `nk`, `fx`, `lq`, `fo`, `jh`, `p_` ; écrivains génériques `kr`, `cc`, `by`, `mi`, `m_`. |
| Noms lus sur un `jo` nu sans membre (rendent « absent ») | `hp` (`jo:pq`), `px`, `qh`, `s`, `qw` (`gh`, `kj`, `economy_score`), `eo`, `ff`, `hi` (`pz`), `fg`, `ql` (`ui.c:54`), `mt` (`slipway.c:45`), `nl` (`economy_tiers.c:28`), `qs` (`dl`, `probe.c:219`). Même résultat qu'aujourd'hui : absents de toute la chaîne de `jo`. |
| Méthodes appelées par nom sur un `jo` nu | `hd` (`gw`), `kj` (`dc`), `hk` (`hy:hk`, `cv.hk`), `hb` (`hy:lj`, `cv.hb`), `gg` (`economy_score.c:95`), `ib` (`bd`, `economy_tiers.c:104`), `cn` (`planet_trade.c:305`, `planet_family_trade.c:206`), `kf` (`jc:pq`, `slipway.c:177`), `gv`, `ldown`, `rdown` (`jz`, `ui.c:411` ; `gv` aussi `slipway.c:110`), et par `dg` sur `me` : `ll`, et `unknown`/`known`/`ly` (absents). Présentes au `lt` : `kj`, `hd`, `hk`, `hb` ; absente : `ev`. **Hors enum aujourd'hui : `gg`, `gv`, `hk`, `kf`, `cn`, `ib`, `ll` → l'enum gagne 7 entrées.** |
| `->defaults` | `planet.c:162-172` : 11 installations (`hd`, `gg`, `hb`, `gv`, `hk`, `ldown`, `kf`, `cn`, `ib`, `kj`, `ll`), plus `pico8_class_set_pq`. Lectures de classe : `planet_display.c:184` (`dz`), `:296` (`cq`), `:532` (`ka`), `research_display.c:266-267` (`em`). Aucun `->defaults` de `jo` dans `tests/` ni `tools/`. |
| Appels directs depuis une fille | `pico8_jo_kj` (`hj:kj`, `planet_family.c:113`), `pico8_jo_kf` (`mx:kf`, `planet_family_trade.c:292`). |
| Sites à simple forme qui recevront un `jo` struct | Aucun accès de champ (1a.16b). **Trois asserts de tag** « INSTANCE seulement » atteints par le filet : `pico8_ij` (`entity_geom.c:98`, `bw` sur `cg.jo`, `bv` sur `cg.np`), `pico8_eg_gg_pred` (`score_panel.c:134`, « cg("jo") only ever holds instances »), `pico8_mzd_ra_of_ij` (`wormhole_display.c:49`, `mz.ij` = planète la plus proche). |
| Lectures de classe-valeur qui remontent jusqu'à `jo` | `research_icon.c:107-110` (`rr`/`s`/`me`/`od` sur une classe de `gx[]`, toujours une fille) : inchangées, `defaults` de `jo` gardés. |
| Tests | Lectures à simple forme d'un `jo` nu : `built_planet_panel_nil_row:106,119,218` (`me`, `id`, `ep`), `empty_planet_cleared:55,63` (`me`, `ra`, sur `cg.jo` **et** sur le `qh` construit), `rich_planet_exports:50,59,79` (`id`, `kw`, `jy`), `slipway_drag:39` (`oe`). Écritures `->fields` : `planet_gh_row_lq:102,107,110` (`kw`, `ra`, `gq`), `planet_kj_dispatch:104,109` (`ra`, espion `kj`), `rich_planet_exports:74` (`mi`). `playability` ne lit que le compte de `cg.jo` et `fm`. Les caractérisations `test_pico8_bg`, `fq`, `ie`, `jc`, `js`, `mz` lisent déjà les planètes par l'API double (`jc.c:698` écrit `fo`, accepté). `geoharvest_burst:85` lit `->cls` sur chaque entrée de `hv` sans tag : lecture dans les bornes d'une struct `jo` (plus grande que l'en-tête d'instance), jamais égale à `ga`, jamais déréférencée — même situation qu'avec `bg`/`mz`/`jc`/`ie`/`js`, réécriture en 1a.26. `ga_export_panel_by` et `geoharvest_burst` ne lisent qu'un `ga` (fille, instance). |
| Filet | Les 15 rejeux construisent des `jo` (`jo  1710  15/15`). |

### Les sept tests de régression nommés

| Test | Ce qu'il lit ou écrit sur un `jo` | Bug d'origine | Preuve rouge (code final) |
|---|---|---|---|
| `built_planet_panel_nil_row` | `me`, `id`, `ep` sur `cg.jo` par `pico8_instance_get_field` | 3b8ba2c : `pico8_msgbox_truthy` ne rejetait que `false` ; un `ep` nil faisait préfixer une ligne fantôme | ~~Retirer la clause REF-NULL de `pico8_msgbox_truthy`~~ : cette preuve à un seul changement ne peut pas échouer sur une struct `jo` (le `ep` nil de p27 est un membre nil, donc absent : `msgbox.c:462` ne passe jamais le nil à `pico8_msgbox_truthy`). Remplacée (décision coordinateur, option b) par une preuve à deux changements locaux : (i) le lecteur du descripteur de `jo` rend « présent » un membre nil parmi ceux que le port écrit à nil (`ep`, `fo`, `nk`, sémantique de l'instance) ; (ii) clause REF-NULL de `pico8_msgbox_truthy` retirée → `built_rows == BUILT_PANEL_EXPECTED_ROWS` faux. Contrôles : (i) seul, (ii) seul → vert. (i) étendu à tout membre nil fait partir plus tôt l'assert de `pico8_dg_truthy` (`p_` jamais écrit lu présent-nil), pas l'assert du test. |
| `empty_planet_cleared` | `me`, `ra` sur `cg.jo` (structs) et sur le `qh` posé (instance) | 529f355 : `jo`'s `"ll"` non enregistré, une planète vide n'était jamais détruite | Retirer l'entrée `LL` du descripteur de `jo` → une assertion du test échoue (noter laquelle) |
| `planet_gh_row_lq` | construit un `jo`, écrit `kw`/`ra`/`gq` par `->fields`, appelle `jo:gh` | 8361468 : cast aveugle de la liste `lq` dans `js:pq` | Cas `LIST` de `pico8_js_lq_arg_table` remplacé par le cast aveugle → ASan heap-buffer-overflow |
| `planet_kj_dispatch` | construit un `jo`, écrit `ra`, pose l'espion `kj` par `->fields` | ordre `mo:md` avant la boucle par entité de `dc("kj")` (1a.8) | Bloc `mo:md` de `pico8_dc` déplacé après la boucle → `g_order[0] == 'M'` faux |
| `playability` | rien : compte de `cg.jo`, `fm.nz` | aucun correctif dans l'historique ; bug de référence nommé par ses commentaires : double enregistrement dans `cg.jo` (« 90 is a hard ceiling ») | Constructeur de `jo` qui appelle `pico8_ke_struct_pq` deux fois → `count <= PICO8_PLAYABILITY_MAX_PLANETS` faux |
| `rich_planet_exports` | `id`, `kw`, `jy` lus, `mi` écrit par `->fields` | 50f91b4 : `bb_visit` bornait par `po->count` au lieu de `#po` | `po_len` → `po->count` → `mu4 == 3` faux |
| `slipway_drag` | `oe` sur `cg.jo` | 2d7021f : `ui:bh` passait nil à `ey` | Remettre nil à la place de l'entité de `jz("gv")` → `oe_count(origin) == 1` faux |

Aucun des sept ne lit **seulement** une fille. `playability` garde un **diff vide** (il ne lit aucun champ de planète) ; sa preuve rouge est faite quand même. Hors de la liste, `ga_export_panel_by` et `geoharvest_burst` ne lisent qu'un `ga` et gardent un diff vide.

## Solution

1. **Enum** : ajouter `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL` (noms `"gg"`, `"gv"`, `"hk"`, `"kf"`, `"cn"`, `"ib"`, `"ll"`). Pour toute struct déjà convertie, l'entrée est NULL et le nom est cherché comme champ : comportement inchangé.
2. **Struct `jo`**, publique dans le module de la planète : `pico8_ke_t`, puis les 31 membres `pico8_value_t` du tableau, noms minifiés conservés.
3. **Défauts pris par REF, jamais re-parsés** : `pico8_jo_init` déclare la classe comme aujourd'hui (même spec, même parent, même nom de couverture, mêmes 11 installations, même `pq`, même `register_named("jo")`), puis lit **une fois** les 14 défauts dans la `pico8_class_t` de `jo` (par `pico8_class_get_field`) vers l'état statique du module. `pico8_jo_shutdown` remet cet état à NULL/nil avant de libérer la classe ; il ne libère aucune de ces tables (elles appartiennent à `defaults`).
4. **`kw` : état statique partagé.** Le membre `kw` d'un `jo` neuf est la REF de la table `kw` de `defaults`, la même pour tous les `jo` et toutes les filles qui n'ont pas couru `lz`. Une écriture en place (`bb`, `mx:kf`) la voit partout, comme dans le cart. Aucune copie par struct.
5. **Lectures sur la classe** : `jo.dz`, `jo.cq`, `jo.ka`, `jo.em` passent par des accesseurs du module qui rendent la REF statique (NULL avant l'init). `planet_display.c` (`dz`, `cq`, `ka`) et `research_display.c` (`em`) perdent leur `->defaults` et leur `lookup_named("jo")`, et gardent leur garde « absent → comme aujourd'hui ».
6. **Constructeur** (posé par `pico8_class_set_struct_new` sur la classe de `jo`), dans l'ordre :
   1. alloue ;
   2. `pico8_ke_struct_init` avec le descripteur de `jo` ;
   3. copie les 14 défauts statiques dans leurs membres (`me` écrase `"ph"`), les autres membres à nil ;
   4. recopie chaque clé nommée de la table reçue (NULL permise) parmi `id`, `p`, `ra`, et **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise ; ne garde aucun pointeur sur la table ;
   5. enregistre `"jo"` au crochet de couverture ;
   6. appelle `pico8_jo_pq` sur la struct (inchangé, à double forme : `oe`, puis `hp` absent) ;
   7. appelle `pico8_ke_struct_pq`.
7. **Descripteur de `jo`** :
   - méthodes `HD`, `KJ`, `LDOWN`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL` = **exactement les mêmes fonctions** que les 11 dispatchers installés dans `defaults` (`pico8_jo_*_dispatch`) ; `EV`, `PH`, `RDOWN`, `HQ`, `EY`, `MD`, `KZ`, `PR`, `EB`, `OL` NULL ;
   - **lecteur** : les 31 membres (nil = absent), puis `pico8_ke_field` ;
   - **écrivain** : n'accepte que les 17 noms mesurés `by`, `cc`, `ep`, `fo`, `fx`, `hn`, `jh`, `jy`, `kr`, `kw`, `lq`, `m_`, `ma`, `mi`, `nk`, `oe`, `p_` ; rend 0 pour tout autre ;
   - accesseur public du descripteur, comme `pico8_js_struct_class()`.
8. **Fonctions de `jo` : aucune ne change de corps ni de signature.** Elles sont déjà à double forme (1a.16b) et restent atteintes par des filles jusqu'à 1a.26. Les 11 installations `->defaults` et `pico8_class_set_pq(g_jo_class, pico8_jo_pq)` **restent**, pour les filles : ce sont les mêmes fonctions que le descripteur.
9. **`pico8_jo_new`** rend `pico8_jo_t *`, même paramètre. `bw` (`galaxy.c`) ne change que le type de la variable jetée.
10. **Trois asserts de tag élargis** à `INSTANCE || STRUCT`, sur le modèle de `probe.c:182` : `pico8_ij`, `pico8_eg_gg_pred`, `pico8_mzd_ra_of_ij`. Messages mis à jour pour dire « entité » au lieu d'« instance ». Rien d'autre dans ces fonctions.
11. **Tests de régression** (tableau) : lectures par `pico8_entity_get_field` ; `rich_planet_exports` écrit `mi` par `pico8_entity_set_field` (écrivain générique réel) ; `planet_gh_row_lq` et `planet_kj_dispatch` prennent un `pico8_jo_t *` et écrivent `ra`/`gq`/`kw` dans les membres ; `planet_kj_dispatch` pose son espion `kj` par une copie locale du descripteur de `jo` dont l'entrée `KJ` est `spy_kj`, rétablie avant la fin (modèle de son propre espion `md`). Types de pointeurs ajustés (`const void *` ou `pico8_jo_t *`), includes. Rien d'autre.
12. **Plan** : à la ligne 1a.17 de `docs/plan-3.md`, ajouter une phrase « Découvert à 1a.17 : … » qui nomme les sites effectivement découverts (au minimum : l'enum gagne `GG`/`GV`/`HK`/`KF`/`CN`/`IB`/`LL` — `ll` appelé par `dg` sur `me`, les autres par `mv`/`jz` ; asserts de tag `ij` (`entity_geom.c:98`), `eg_gg_pred` (`score_panel.c:134`), `mz.ij` (`wormhole_display.c:49`) élargis ; `m_` (`gw`), `ma` (`ma()`), `fo` (`bv`, `jc:hq`) écrivains génériques → membres ; `jo.dz`/`cq`/`ka`/`em` lus sur la classe, REF prise à l'init ; `kw` état statique partagé ; lecture `em` à `research_display.c:266-267`).

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `jo` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `qh`…`ga` à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `jo` mesurée (constructions, défauts, champs écrits, écrivains génériques, méthodes par nom, `->defaults`, appels directs, asserts de tag, tests, filet), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux une struct `jo` qui n'a que `pico8_ke_t` et les 31 membres mesurés, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que gardien du filet, je veux que les 90 planètes de `bw` soient construites avec leurs `id`, `p`, `ra` et les défauts de `jo`, afin que la galaxie soit identique au pixel près.
5. En tant que mainteneur, je veux qu'une clé de construction hors `id`/`p`/`ra` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt (§7) et pas une perte silencieuse.
6. En tant que gardien du filet, je veux que `jo:pq` tourne avant `lt` sur la struct, afin que `oe` existe avant tout enregistrement, comme dans le cart.
7. En tant que gardien du filet, je veux qu'un `jo` neuf entre dans `hv`, `cv.kj`, `cv.hd`, `cv.hk`, `cv.hb`, `cg.jo`, `cg.np` et `cg.prober`, dans le même ordre relatif, et jamais dans `cv.ev`, afin que dessin, survol, revenus et recherche parcourent les planètes dans le même ordre.
8. En tant que gardien du filet, je veux que `dg` appelle `jo:ll` sur une planète dont `me` vaut `"ll"`, et rien pour `"unknown"`, `"known"` ou `"ly"`, afin que les planètes vides disparaissent comme depuis 529f355.
9. En tant que gardien du filet, je veux que `gg`, `ib`, `cn`, `kf`, `gv`, `hk` soient trouvés par nom sur une struct `jo`, afin que score, routes, échanges, slipways, drag et revenus marchent comme avant.
10. En tant que mainteneur, je veux que le descripteur de `jo` pointe sur les mêmes dispatchers que ceux installés dans `defaults`, afin qu'une struct `jo` et une fille héritière exécutent exactement le même code.
11. En tant qu'auteur des étapes 1a.18 à 1a.26, je veux que toutes les fonctions de `jo` restent à double forme et que ses 11 installations `->defaults` et son `pq` de classe restent en place, afin que chaque fille non convertie trouve ses méthodes héritées en remontant `parent`.
12. En tant qu'auteur de 1a.26, je veux que le passage à la forme struct seule des fonctions de `jo` soit laissé à ce commit-là, afin de ne rien casser pour `ga` avant.
13. En tant que mainteneur, je veux que les tables de défaut de `jo` soient prises dans `cls->defaults` à l'init, jamais re-parsées, afin qu'une struct et une fille lisent la même table.
14. En tant que gardien du filet, je veux que `kw` soit un état statique partagé, pointé par `defaults` et par chaque `jo` qui n'a pas couru `lz`, afin qu'une écriture en place de `bb` ou `mx:kf` se voie partout, comme dans le cart.
15. En tant que mainteneur, je veux que `jo.dz`, `jo.cq`, `jo.ka`, `jo.em` soient lus sur la classe par des accesseurs du module, afin qu'aucun `->defaults` de `jo` ne subsiste hors des installations destinées aux filles.
16. En tant que gardien du filet, je veux que les écrivains génériques (`hy:lj` → `kr`, `bd` → `cc`/`by`, `bb` → `mi`, `gw` → `m_`, `ma()` → `ma`, `bv`/`jc:hq` → `fo`) écrivent dans des membres de `jo`, afin d'appliquer la règle « Champ d'un écrivain générique ».
17. En tant que mainteneur, je veux que l'écrivain de `jo` n'accepte que les 17 noms mesurés, afin que toute autre écriture générique sur une planète soit un arrêt.
18. En tant que gardien du filet, je veux que `ij`, le prédicat `gg` du score et `mz.ij` acceptent une struct `jo`, afin que la génération de galaxie, la fin de partie et le dessin des trous de ver ne meurent pas sur un assert.
19. En tant que gardien de la couverture, je veux que le constructeur enregistre `"jo"` une fois par planète, et que les filles gardent leur propre compte, afin que `class-coverage-net` soit identique ligne à ligne.
20. En tant que vérificateur, je veux une caractérisation de `jo` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas.
21. En tant que vérificateur, je veux que cette caractérisation lise au moins une instance de chaque fille construite par le filet (champ hérité de `jo` et résultat d'une méthode de `jo`), afin de prouver que l'héritage des filles survit à la conversion de leur parente.
22. En tant que vérificateur, je veux que la caractérisation fige l'identité des REF de défaut (`kw`, `kq`, `em`…) entre une planète, une fille et la classe, afin que la règle « même REF » soit vérifiée et pas seulement écrite.
23. En tant que gardien des tests de régression, je veux que les sept tests nommés ne changent que leur façon de lire ou de poser l'état, afin que leur intention reste intacte (§6).
24. En tant que gardien des tests de régression, je veux que chacun des sept soit prouvé rouge avec son bug d'origine remis, puis vert, afin que la réécriture ne l'ait pas vidé de son objet.
25. En tant que gardien des tests, je veux que `playability`, `ga_export_panel_by`, `geoharvest_burst` et tous les autres tests aient un diff vide, afin que la conversion ne se valide pas en affaiblissant un test.
26. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
27. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.17, afin que la liste des étapes dise ce que l'étape a réellement touché.
28. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés, afin de ne laisser aucun contrat mensonger.
29. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais. J'ai donc chargé le skill `guideline` et je l'ai appliqué moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : module de la planète (struct, statiques, constructeur, descripteur, accesseurs, `pico8_jo_new`) ; en-tête des structs (enum) ; génération de galaxie (type de retour) ; affichage de planète et de recherche (lectures sur la classe) ; géométrie des entités, panneau de score, affichage du trou de ver (trois asserts). Aucun autre module de `src/`.
- **Fonctions de `jo` inchangées, signatures comprises** (G). Elles reçoivent une struct `jo` ou une fille, et leurs corps ne passent que par l'API d'entité. Rejeté : les retyper en `pico8_jo_t *`, ce qui mentirait pour une fille ; les retyper en `void *`, ce qui toucherait ~15 signatures, les dispatchers et leurs appelants pour aucun comportement. Le retypage appartient à 1a.26, quand elles deviennent struct seule.
- **Le descripteur réutilise les dispatchers existants** plutôt que des fonctions struct seules : c'est ce qui rend « même code pour la struct et la fille » vérifiable par une égalité de pointeurs.
- **Les 11 installations `->defaults` et le `pq` de classe restent** : ce sont les méthodes des filles (§5, « Accès directs aux défauts » : ce que la classe retenue garde pour ses descendants non convertis). Elles disparaissent en 1a.26.
- **14 défauts copiés depuis la classe à l'init** (G), scalaires compris. Rejeté : écrire les scalaires en dur, qui ferait deux sources pour la même valeur. La struct n'a aucun re-parsage de spec.
- **`kw` : état statique partagé** (tranché par le plan). Rejeté : une copie par struct, qui divergerait du cart dès qu'une écriture en place frappe le défaut.
- **Lectures sur la classe par accesseurs dédiés** (G) (`dz`, `cq`, `ka`, `em`) : quatre fonctions de une ligne qui rendent la REF statique. Rejeté : lire le membre d'une planète, qui ne serait pas le cart (`jo.dz`, pas `self.dz`) ; un accesseur générique par nom, sans usage au-delà de quatre noms.
- **Membres `dz`, `cq`, `ka`, `em` gardés** (règle des membres, modèle `bg.pl`) : REF vers la même table, jamais lus par le code de `jo`.
- **Écrivain à 17 noms** (G) : exactement les écritures mesurées sur un `jo` nu. `ra`, `gq`, `id`, `p` n'y sont pas : aucun code du port ne les écrit sur un `jo` nu. Les deux tests qui posent `ra`/`gq`/`kw` écrivent les membres de leur `pico8_jo_t` directement. Rejeté : ouvrir l'écrivain pour les tests.
- **Sémantique de nil** : un membre nil est absent. Sur une instance, un nil écrit masquait le défaut ; mesuré, aucune écriture nil ne frappe un membre à défaut (`ep`, `fo`, `nk` n'en ont pas). Pas d'écart.
- **Asserts de tag élargis, pas supprimés** (G) : même forme que `probe.c:182`. Rejeté : les retirer, ce qui perdrait la garde contre une table nue.
- **Clés dynamiques** : aucune nouvelle. `dg` appelle `qc[qc.me]` sur un ensemble fini de noms (`me` ∈ défauts, `ks` du port, `df.me`) : `ll` entre dans l'enum, les autres valeurs de `me` n'ont pas de membre et rendent absent.
- **Coexistence avec `ui`, `lq`, `js`, `jc`, `hy`** : aucun changement. Les membres `lq`, `m_`, `fo` tiennent des REF comme aujourd'hui.
- **`live_ga`** (`geoharvest_burst`) : laissé tel quel, réécriture en 1a.26.
- **TROU** : aucun écart attendu. Si l'implémenteur en trouve un, règle §7 (préserver, `TROU-N`, `/* TROU-N */`).
- **Commentaires** (CLAUDE.md) :
  - à ajouter : le contrat de la struct (jamais libérée, TROU-7 ; `kw` peut être la table partagée des défauts ; `dz`/`cq`/`ka`/`em` sont des REF vers les tables de la classe) ; celui de `pico8_jo_new` (table lue, pas gardée, `id`/`p`/`ra` seulement) ; l'assert des clés ; le contrat de l'écrivain ; celui des accesseurs de classe (NULL avant l'init, table empruntée) ; une ligne sur les installations `->defaults` qui restent pour les filles jusqu'à 1a.26 (piège qu'on réintroduirait en les retirant) ;
  - à corriger si rendus faux : le commentaire de `pico8_jo_new`/`pico8_jo_class` dans `planet.h`, celui de `research_display.h` sur `lookup_named("jo")`, les messages des trois asserts ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` et `make frames-check` verts. L'ordre est fait pour échouer tôt : la caractérisation (filles comprises) est figée sur l'ancien code **avant toute modification de `src/`**, et tout ce qui peut précéder la conversion la précède.

1. **Caractérisation `test_pico8_jo` sur l'ancien code** (voir Testing), valeurs figées, filles comprises. Arrêt ici si un rejeu ne construit pas la fille attendue : c'est une mesure fausse, à corriger dans le PRD, pas un test durci.
2. **Coexistence sans conversion** (`jo` encore instance) :
   - enum `GG`…`LL` ajoutée (aucune struct ne la remplit encore) ;
   - accesseurs `dz`/`cq`/`ka`/`em` et état statique des 14 défauts pris à l'init ; `planet_display.c` et `research_display.c` y passent ;
   - trois asserts de tag élargis ;
   - lectures des tests nommés passées à `pico8_entity_get_field`, et `mi` de `rich_planet_exports` à `pico8_entity_set_field` (équivalents exacts sur une instance).
   Suite verte, caractérisation inchangée.
3. **`jo` converti** : struct, constructeur, descripteur, écrivain, accesseur du descripteur, `pico8_class_set_struct_new`, `pico8_jo_new` typé, `bw` ; `planet_gh_row_lq` et `planet_kj_dispatch` (types, membres, espion par descripteur). La caractérisation passe **sans changer ses valeurs figées** ; les vérifications de forme s'y ajoutent.
4. **Preuves** : sept preuves rouges, mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.17 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_jo`** (dans `make unit`, lié comme `test_pico8_js`, sur le cart réel). Il lit **uniquement** par l'API à double forme et par `pico8_class_get_field(pico8_jo_class(), …)`, qui existe avant et après ; son code est le même avant et après, au type de retour de `pico8_jo_new` près. Chaque scénario tourne dans un fils forké.
  - **Caractérisation, figée sur l'ancien code** :
    1. **Galaxie** (après `pico8_eu(pico8_js_new(bo[2]))`) : pour chacune des 90 planètes, empreinte de `me`, `t`, `id`, `p`, `ra`, `hf`, `hs`, `fa`, `ma`, `ca`, `gq`, présence de chacun des 31 noms, des 11 méthodes, et absence de `ev`, `ph`, `ly`, `rdown`, `hq`, `ey`, `md` ; appartenance et rang dans `hv`, `cv.kj`/`ev`/`hd`/`hk`/`hb`, `cg.jo`/`np`/`prober`. **Identité** : la REF de `nu`, `kq`, `kw`, `gq`, `em` lue sur la planète est celle de `pico8_class_get_field(pico8_jo_class(), …)`. Figée.
    2. **Rejeux** `built_planet_panel_events`, `empty_planet_cleared_events`, `slipway_drag_events`, `phase4_playthrough_events` : par frame, pour chaque entrée de `cg.jo`, `me`, `t`, `ma`, `p_`, `fx`, `mi`, `kr`, `kw.qe`/`kw.mu`, taille de `oe`/`jy`/`by`/`cc`, présence de `fo`/`nk`/`lq`/`m_`, `ep` ; plus une empreinte de l'écran ; repliés en une empreinte de séquence figée. L'implémenteur note dans le commit les états atteints (`known`, `ly`, `ll`, `kf`, `cn`, `ib`).
    3. **Filles construites par le filet** (règle « Méthodes d'une parente convertie ») : `session_events` (`qh`, `nr`, `fr`, `pm`, `ga`), `events_recorded_1790198294` (`hj`, `mx`), `trading_hub_session` (`nl`), `long_session` (`os`), tous depuis le titre (`--from-title`). Pour au moins une instance de **chaque** fille, à sa première frame d'existence puis en fin de rejeu : champ hérité de `jo` (`ca`, `gq` x/y, identité de la REF `em` avec celle de la classe de `jo`) et résultat de méthodes de `jo` appelées sur elle (`mv(d,"gg")` ; `mv(d,"hd",pt,pt)` en `d.ra` et en `d.ra+(8,8)`). Figés. `pm` et `ga` sont obligatoires (plan) ; les sept autres sont exigés parce que le filet les construit.
    4. **Scénarios dirigés** : une planète neuve de `pico8_jo_new` (clés `id`/`p`/`ra`) — mêmes lectures que 1 ; `dg` sur une planète forcée à `me="ll"` et `t>60` → `p_` vrai et sortie de `hv` ; `kw` d'une planète qui n'a pas couru `lz` = la REF de la classe, et une écriture en place de `mu` sur elle se lit sur une autre planète neuve et sur la classe (état partagé) ; après `pico8_jo_lz`, `kw` n'est plus cette REF.
  - **Forme** (ajoutée au morceau 3) : chaque `jo` de `bw` et de `pico8_jo_new` est `STRUCT` avec le descripteur de `jo` ; pour chacune des 11 méthodes, la fonction lue sur la struct est **la même** que `pico8_class_get_field(pico8_jo_class(), nom)` ; `pico8_entity_set_field` passe pour les 17 noms et asserte (dans un fils) pour `ra`, `id`, `hp` ; une clé de construction inconnue ou positionnelle asserte ; les filles restent `INSTANCE`.
  - **Mutations** (au moins 12, notées dans le commit, mutants équivalents justifiés) : `LL`, `GG`, `IB`, `CN`, `KF`, `HK` ou `HD` absent du descripteur ; `me` initial laissé à `"ph"` ; `kw` re-parsé ou copié au lieu de la REF de la classe ; `ra` non recopié ; `jo_pq` après `pico8_ke_struct_pq` ; couverture non enregistrée (attrapée par `class-coverage-net`) ; écrivain qui refuse `m_` ou `kr` ; accesseur `em` qui re-parse ; un dispatcher de fille (installation `->defaults`) retiré.
- **Tests de régression réécrits** (§6) : les six premiers du tableau, diff limité à la lecture ou à la pose de l'état, aux types et aux includes ; `playability` diff vide. Preuve rouge des **sept** sur le code final, par `make tmp/test_pico8_<nom> && ./tmp/test_pico8_<nom>` (`ASAN_OPTIONS=detect_leaks=0` pour `planet_gh_row_lq`), selon le tableau ; le commit note le bug remis, l'échec observé, la restauration et le vert. Si un bug remis ne rend **pas** son test rouge, c'est un arrêt (§7), pas un test durci.
- **Tests existants** : `test_pico8_js`, `ie`, `jc`, `mz`, `bg`, `bg_kj`, `fq`, `qb`, `iv`, `nf`, `je`, `mo`, `mh`, `jb_ol`, `class_chain`, `entity_smoke` : diff vide, valeurs figées identiques. `ga_export_panel_by`, `geoharvest_burst`, `tech_screen`, `game_end`, `score_panel_close`, `score_panel_stars`, `cheat_code`, `top_menubar`, `shop_tooltip_fillp`, `layout_nil_item_draws_nothing`, `probe_mm_rnd_order`, `test_pico8_replay_from_title` : diff vide, verts.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED` ; les binaires `*_refaudit` compilent.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** : `test_pico8_js` (producteurs, rejeux, scénarios, forme, mutations), `test_pico8_ie` (instances de toutes les filles via `gx[]`, lecture de classes-valeurs), `test_pico8_jc`/`test_pico8_mz` (planètes lues à double forme, `fo`, `oe`), 1a.8 (espion par copie de descripteur), 1a.13 et 1a.16 (réécritures et preuves rouges).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `jo` n'est pas `jo  1710  15/15`, ou si une ligne `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga` change. Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier : si l'assertion des clés de construction ou celle de l'écrivain se déclenche sur un rejeu ou un test ; si un membre hors des 31 est ajouté ; si un nom hors des 17 est accepté par l'écrivain.
- `FAILED:` si une fille de `jo` (`qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`) est convertie ou change de forme, ou si une autre classe que `jo` est convertie (`ui`, `lq`, `hy`, `hp`, `pj`, `g_`, `fv`, `trade`…).
- `FAILED:` si une fonction de `jo` atteignable par une fille non convertie devient struct seule : une des 11 installations `->defaults` de `jo` ou `pico8_class_set_pq(g_jo_class, …)` retirée ou changée de fonction ; un corps de `pico8_jo_*` (dont `pq`, `lz`, `kd`, `hu`, `hh`, `fe`, `gh`, `gv`, `kj`, `kf`, `cn`, `ib`, `hb`, `hk`, `gg`, `ldown`, `ll`) qui caste `self` en `pico8_jo_t *`, lit un membre directement, ou revient à `pico8_instance_get_field`/`pico8_instance_fields` ; une entrée du descripteur qui n'est pas le même pointeur que l'installation correspondante.
- `FAILED:` si une table de défaut de `jo` est re-parsée depuis la spec (un second `pico8_ob(` ou `pico8_class_declare` sur un fragment de la spec de `jo`), ou si `kw` d'un `jo` neuf n'est pas la REF de `defaults`.
- `FAILED:` si l'un des sept tests nommés change autrement que dans sa façon de lire ou de poser l'état (frames, fixtures, asserts, messages, seuils modifiés) ; si `playability` a un diff non vide ; si le commit ne rapporte pas les sept preuves rouges (bug remis, rouge, restauré, vert) ; ou si un bug remis ne rend pas son test rouge.
- `FAILED:` si un autre test de régression (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si les valeurs figées de `test_pico8_jo` ont été changées après la conversion, ou celles de `test_pico8_js`, `ie`, `jc`, `mz`, `bg`, `bg_kj`, `fq`, `qb`, `iv`, `nf`, `je`, `mo`, `mh` ; ou si la caractérisation ne lit pas au moins un `qh`, un `pm` et un `ga` construits par un rejeu du filet (champ hérité et méthode de `jo`).
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- grep : plus aucun `pico8_jo_class()->defaults` ni `lookup_named("jo")` hors du module de la planète ; les seuls `g_jo_class->defaults` restants sont les 11 installations ;
- l'enum vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL` ;
- la struct `jo` n'a que `pico8_ke_t` et les 31 membres ; le descripteur n'a que les 11 méthodes ; l'écrivain n'accepte que les 17 noms ;
- hors du module de la planète, `pico8_struct.h`, `galaxy.c`, `planet_display.c`, `research_display.c(.h)`, `entity_geom.c`, `score_panel.c`, `wormhole_display.c`, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_jo.c` (nouveau), les six tests réécrits et le `Makefile` (cible et `unit`) bougent ;
- la ligne 1a.17 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.17 ».

## Out of Scope

- Toute fille de `jo` (1a.18 à 1a.26) : leurs structs, leurs `->defaults` (`qh`, `nl`, `hj`, `nr`, `mx`, `ga`), `ga.by`, `ga.k_`, `live_ga`.
- Le passage à la forme struct seule des fonctions de `jo`, le retrait de ses 11 installations et de son `pq` de classe, le retypage de leurs signatures (1a.26).
- `ui` (1a.31), `lq` (1a.34), `hy` (1a.35), `hp` (1a.33), `trade` (1a.32) : lectures `lq->fields`/`ok` des tests comprises.
- Le typage C des membres (1b) ; les tables `df`, `gb` en données constantes (D).
- Libérer une planète (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Pourquoi l'étape est surtout additive** : 1a.16b a déjà rendu chaque fonction de `jo` à double forme, et toutes sont atteintes par une fille. La conversion n'ajoute donc qu'une struct, un constructeur, un descripteur branché sur les dispatchers existants, et les quatre lectures de classe ; le reste du diff est dans les tests.
- **Le filet sert de piège** : 1 710 structs `jo` le traversent, et `pico8_instance_get_field`/`pico8_instance_fields` assertent sur une struct. Un site à simple forme oublié casse `frames-check`. Les trois asserts de tag relevés ne sont pas des accès de champ, et le fil piège de 1a.16b ne pouvait pas les voir : c'est pourquoi ils sont nommés ici.
- **Sans `LL` dans le descripteur**, les planètes vides ne disparaissent plus : c'est exactement 529f355, d'où la preuve rouge d'`empty_planet_cleared`.
- **`kw` partagé** : dans le filet, aucune écriture en place ne frappe la table de défaut (chaque planète qui atteint `bb` ou `mx:kf` a couru `lz`) ; le partage est structurel, pas observé à l'écran. D'où le scénario dirigé 4.
- **Réutilisation d'adresse** : la sonde de 1a.16b avait vu des faux positifs par réutilisation d'une `pico8_class_t` libérée. L'état statique des défauts est remis à NULL au shutdown pour la même raison.
- Mesures : grep sur `src/`, `tests/`, `tools/`, `Makefile` à fb67ce9 ; lecture de `pico8/lua.lua:5-13,80-97,173-196,209-211,346-356,384-551,735-751,900-1110,1244-1300,1364-1390`, de `pico8_planet*.c`, `pico8_class.c/h`, `pico8_entity.c/h`, `pico8_struct.h`, `pico8_ke.c/h`, `pico8_bootstrap.c`, `pico8_entity_geom.c`, `pico8_galaxy.c`, `pico8_research_display.c`, `pico8_score_panel.c`, `pico8_wormhole_display.c`, des sept tests nommés et de `tools/class_coverage_net_expected.txt` ; sonde de mesure et comptes par rejeu dans une copie `git archive` jetable (`make -j8 test` sortie 0, `class-coverage-net` sortie 0, 143 sites distincts `(opération, nom, fichier:ligne)` sur un `jo` nu dans `src/`).
