---
title: "Plan 3, étape 1a.18 : `qh` (le laboratoire) devient une struct fille de `jo`, ses fonctions et celles de `jo` restent à double forme pour ses sept descendantes"
description: "Le laboratoire `qh` est encore une instance de la couche objet, construite 8 fois sur le filet par `ie:ldown` depuis `gx[0]`, alors que sept descendantes non converties (`nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`) héritent de sa `pico8_class_t`, de ses 13 défauts et de ses 4 méthodes. Il passe en struct dont le premier membre est `pico8_jo_t` : 10 membres propres, 4 défauts rangés dans les membres hérités, un descripteur `static const` qui reprend les 11 dispatchers de `jo` et ajoute `HQ`, `EV`, `EY` et `LY` (nouvelle entrée de l'enum), un lecteur et un écrivain chaînés sur ceux de `jo`, et `empty_planet_cleared` relu puis prouvé rouge. Aucune fonction de `qh` ni de `jo` ne passe à la forme struct seule."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.18 — `qh` : `jo` (`planet_family.c:306`), `gx[0]`. Tests : `empty_planet_cleared`.

(À HEAD c825868, la déclaration est bien à `planet_family.c:306`, et `gx[0] = pico8_qh_class()` est à `cart.c:223`.)

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag (`jo` pour `cg.jo`) ; seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe. Les descendants non convertis trouvent leurs champs hérités en remontant `parent`, qui reste valide.
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre (dispatcher hérité dans `defaults`, ou appel direct […]) reste à double forme : lecture/écriture par `pico8_entity_get_field`/`set_field`, ou aiguillage sur le tag comme `pico8_ke_ks`. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante (pour `jo` : 1a.26).
>
> **Accès directs aux défauts** : […] la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct : une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec.

§6 : `empty_planet_cleared` lit une struct `qh` (le laboratoire posé, `cg.qh[1]`) ; il est réécrit **dans sa façon de lire l'état** seulement, dans ce commit, et prouvé rouge avec son bug d'origine (529f355) remis localement, puis restauré. §7 : un commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (`qh  8  3/15`, toutes les autres lignes inchangées) ; un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.18, avec « découvert à 1a.18 » dans le message ; un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.17 ont posé (jusqu'à c825868), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`). L'enum vaut `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL`. Un nom absent de l'enum, ou dont l'entrée est NULL, est cherché comme champ ; `pico8_mv` rend 0 sur un champ absent. L'écrivain rend 0 pour un nom sans membre, et `pico8_entity_set_field` asserte alors (« a field under no rule of plan-3 section 5 — stop »).
- `pico8_ke_t`, `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq`. `pico8_ke_ks` et `dg` écrivent `me`/`t` dans le `pico8_ke_t` d'une struct. `lt` n'inscrit une struct dans `cv.kj`/`ev`/`hd`/`hk`/`hb` que si son descripteur a l'entrée, et dans `cg` selon le `nu` lu par l'API.
- **1a.17** : `pico8_jo_t` (`pico8_ke_t` puis 31 membres), `pico8_jo_struct_class()` (11 méthodes = les 11 dispatchers installés dans `defaults` de `jo` ; lecteur 31 membres puis `pico8_ke_field` ; écrivain à 17 noms `by`, `cc`, `ep`, `fo`, `fx`, `hn`, `jh`, `jy`, `kr`, `kw`, `lq`, `m_`, `ma`, `mi`, `nk`, `oe`, `p_`), défauts de `jo` pris par REF dans `cls->defaults` à l'init (`g_jo_defaults`), `kw` état statique partagé. Les fonctions de `jo` prennent un `pico8_instance_t *` et ne lisent que par l'API d'entité ; les 11 installations `->defaults` et `pico8_class_set_pq(g_jo_class, pico8_jo_pq)` restent pour les filles. `pico8_jo_pq` et les 11 dispatchers sont **statiques** dans le module de la planète, et le constructeur de `jo` fait allocation, initialisation et `pq` d'un seul tenant.
- Les aides partagées qu'atteint un `qh` sont déjà à double forme : `pico8_bv` (garde aiguillée `probe.c:128`, assert élargi `:182`), `pico8_dl`, `pico8_d_` (`ke_ks` aiguillé), `pico8_na` (1a.16b). Les asserts de tag « instance seulement » qu'atteint une planète sont élargis (`ij`, `eg_gg_pred`, `mz.ij`, `dc`, `dg`, `bv`, `jc:hq`).
- Modèles : constructeur à clés mesurées qui asserte sur toute autre (`jo`, `js`, `ie`) ; caractérisation figée sur l'ancien code, lue seulement par l'API à double forme dans des fils forkés, rejeux depuis le titre lancés côte à côte (`test_pico8_jo`) ; descendantes lues par la caractérisation de leur parente (`fold_daughter` de `test_pico8_jo`).

### Ce que fait le cart

`pico8/lua.lua` :

```
:905-918  qh=jo:jr([[ nu=o("qh","jo","np","prober"), hp=1,px=o("laboratory",c=13),
            me="kz",dq="ly", hf=f,qh=1, od=15,fp=2, dw=0.025, mi=1, s=174,
            ba=o(28,18,20,21,22,48), ]])  qh.hq=bv  qh.ly=na  qh.ev=dl  qh.ey=d_
:552-576  bv (self.ra,self.p=mp,mp/128 ; self.et=… ; et.fo=self), dl (me=="kz" → et.qs / od.."$"),
          d_ (not et and fm:lu(od) → ks(dq or "ph"), nx(fp) ; sinon p_=true)
:577-579  na(self,t)  self.ra.y+=sin(t*0.005)*0.06        -- en place, sur la REF de ra
:1182-1186 eu : gx={qh,os,nr,fr,nl,mx,hj} ; ie({ke=b,jb=…})   -- ie:ldown → self.ke({id="s"..ie.id})
:1252-1256 ck : for l in all(cg.qh) … l.jy[#l.jy].qe
:1312-1316 da : pt.qh and #cg.qh or …
:237-247  ui:bh : mv(held,"hq",self.qa,je.ra) ; mv(held,"ey",nw and nw.kv)
:998-1110 hj, nl, os, nr = qh:jr ; pm, mx = hj:jr ; fr = nr:jr
```

Points décisifs :

- **Une seule construction d'un `qh` nu** : `ie:ldown` sur l'icône qui tient `gx[0]` (clé `id` seule). `qh` n'est pas enregistré par nom, n'est ni `kv[hp.cw]` ni `fz`.
- **Cycle de vie** : tenu par `ok.lheld` en état `"kz"` (`hq` = `bv` à chaque frame, `ev` = `dl` dessine le prix), relâché (`ey` = `d_`) → `ks("ly")` et paiement, ou `p_=true` et retrait. En état `"ly"`, `dg` appelle `qc[qc.me]`, c'est-à-dire `qh.ly` = `na`, qui écrit `ra.y` **en place** dans le vecteur que `bv` a pris dans `ui.qa`. Aucun site textuel n'appelle `:ly(` : c'est exactement la forme du bug 529f355 (`ll` de `jo`), transposée à `qh`.
- **`qh` a `hp=1`** : le `pq` de `jo` (seul `pq` de sa chaîne sous celui de `ke`) court `lz(gb[1])` sur chaque `qh` neuf (`hn`, `ep`, `jy`, `kw`) ; le `kw` partagé de `jo` n'est donc jamais celui d'un `qh` après construction.
- **Tables de défaut de `qh`** (`nu`, `px`, `ba`) : jamais écrites en place ni réaffectées (grep du cart). Aucun défaut muté.
- **Toutes les fonctions atteignables par `qh` le sont aussi par une descendante non convertie** : `os` hérite des quatre méthodes de `qh` ; `nl` surcharge `ly` (`nl:ly` appelle `na`) et hérite de `hq`/`ev`/`ey` ; `hj`, `pm`, `mx`, `nr`, `fr` héritent des quatre ; toutes héritent des 11 de `jo`. **Aucune fonction de `qh` ni de `jo` ne peut passer à la forme struct seule dans ce commit.**

### Emprise réelle de `qh` (mesurée sur c825868)

**Méthode.** Grep de `src/`, `tests/`, `tools/`, `Makefile`, lecture des fonctions, puis **sonde de mesure** sur une copie `git archive c825868` jetable (jamais dans le dépôt) : `pico8_entity_get_field`/`set_field`, `pico8_mv`, `pico8_instance_get_field`, `pico8_instance_fields` et `pico8_class_instantiate` deviennent, hors de leurs modules, des macros qui journalisent (opération, nom, classe exacte, fichier:ligne, et « écriture nil » à part) chaque accès à une instance dont la chaîne `cls` contient `qh`. Puis `make -j8 test` (sortie 0, `frames-check` 15 conformes) et `make class-coverage-net` (sortie 0, identique). Enfin le binaire de couverture, rejeu par rejeu du filet (`--from-title` pour les `RECORDINGS`). Les accès internes à `pico8_entity.c` (`lt`, `he`, `dc`) sont lus dans le code.

| Aspect | Mesure |
|---|---|
| Constructions d'un `qh` nu | **une seule** : `ie:ldown` (`research_icon.c:199`, `pico8_class_instantiate(ke_cls, {id="s"..N})`), clé **`id`** seule vue. 8 sur le filet : `session_events` 4, `long_session_events` 2, `empty_planet_cleared_events` 2 (les deux premiers sont des `RECORDINGS`, `--from-title`) ; 0 dans les 12 autres rejeux. `pico8_qh_new` n'a **aucun appelant** (`src/`, `tests/`, `tools/`). Ni `lookup_named`, ni `kv[]`, ni `fz` ne construit un `qh`. |
| Constructions des descendantes (inchangées) | par rejeu : `session_events` → `nr 5`, `fr 5`, `pm 3` ; `events_recorded_1790198294` → `hj 5`, `mx 3`, `pm 1` ; `trading_hub_session_events` → `nl 1`, `nr 1` ; `long_session_events` → `os 1`. (Et `ga` : `session 3`, `events_recorded 3`, `geoharvest_burst 1`.) |
| Défauts propres de `qh` (13) | `nu`, `hp`=1, `px`, `me`=`"kz"`, `dq`=`"ly"`, `hf`=false, `qh`=1, `od`=15, `fp`=2, `dw`=0.025, `mi`=1, `s`=174, `ba`. Hérités de `jo` (non surchargés) : `hs`, `fa`, `kq`, `ma`, `ka`, `dz`, `cq`, `kw`, `gq`, `ca`, `em` ; de `ke` : `t`. |
| Défauts qui tombent sur un membre hérité | `me` (`pico8_ke_t`), `nu`, `hf`, `mi` (`pico8_jo_t`). Les 9 autres n'ont pas de membre chez `jo`. |
| Défauts mutés | aucun (ni réaffectation de `qh.*` après l'init, ni écriture en place de `nu`/`px`/`ba`). |
| Champs écrits sur un `qh` (mesurés) | par des fonctions de `jo` : `oe` (`pq`, `planet.c:398`), `hn`, `ep` (nil), `jy`, `kw` (`lz`, `:438-455`), `ma` (`kd`, `:478`), `lq` (`hb`, `planet_display.c:120`), `fo` (nil, `kj`, `:572`), `fx` (`planet_trade.c:94`) ; par les écrivains génériques déjà membres de `jo` : `kr` (`economy.c:242`), `cc`, `by`, `mi` (`economy_tiers.c:92,107,218`), `m_` (`ui.c:308,313`), `ma` (`entity_geom.c:79`), `fo` (`probe.c:185`) ; par **les méthodes propres de `qh`** : `ra` (`bv`, `probe.c:132`), `p` (`bv`, `:138`), `et` (`bv`, `:179`), `p_` (`d_`, `:311`, déjà chez `jo`) ; `me`/`t` dans le `pico8_ke_t` (`ke_ks` depuis `d_`, `dg`). Écritures nil mesurées : `ep`, `fo` seulement, jamais sur un membre qui a un défaut. Atteignables sans être vues sur le filet, déjà dans l'écrivain de `jo` : `jh` (`jo:cn`), `nk` (`jo:hh`/`hu`, qui exigent `me="unknown"`/`"known"`, états qu'un `qh` n'a jamais). |
| Membres propres qui en résultent (10) | après `pico8_jo_t` : défauts `hp`, `px`, `dq`, `qh`, `od`, `fp`, `dw`, `s`, `ba` ; écrit par la méthode propre `hq` (`bv`) : `et`. `ra` et `p` sont déjà membres de `jo` (clés de construction de `bw`). |
| Noms lus sur un `qh` sans membre (rendent « absent ») | `qs` (`dl` sur un `et` qui est un `qh`, `probe.c:219`), `ik` (`d_`, `:302`), `qw` (`planet_display.c:46`), `nl` (`economy_tiers.c:28`), `eo`, `hi`, `ff` (`msgbox.c:480,487,519`), `fg`, `ql` (`ui.c:54`), `mt` (`slipway.c:45`), `ke` (`test_pico8_ie`). Même résultat qu'aujourd'hui : absents de toute la chaîne `qh`→`jo`→`ke`. |
| Méthodes appelées par nom sur un `qh` | `hq` (`ui.c:360`), `ey` (`ui.c:373`), `gv`, `ldown` (`jz`, `ui.c:411` ; `gv` aussi `slipway.c:110`), `hd` (`ui.c:296`), `hk` (`economy.c:156`), `hb` (`economy.c:277`), `gg` (`economy_score.c:95`), `ib` (`economy_tiers.c:104`), `cn` (`planet_trade.c:305`, `planet_family_trade.c:206`), `kf` (`slipway.c:178`), `ev`/`kj` (`dc`), et par `dg` sur `me` : **`kz`** (absent, entrée de l'enum sans méthode chez `qh`) et **`ly`** (présent sur l'instance, **hors enum aujourd'hui** → l'enum gagne `LY`). Présentes au `lt` : `kj`, `ev`, `hd`, `hk`, `hb` — `ev` est propre à `qh` (`jo` ne l'a pas). |
| `->defaults` de `qh` | `planet_family.c:318-321` : 4 installations (`hq` = `pico8_bv_dispatch`, `ly` = `pico8_qh_ly_dispatch`, `ev` = `pico8_dl_dispatch`, `ey` = `pico8_d__dispatch`). Aucune lecture de `pico8_qh_class()->defaults`, aucun `lookup_named("qh")`, dans `src/`, `tests/`, `tools/`. |
| Lectures de classe-valeur | la vue `ie.ke` (`research_icon.c:107-110`) lit `rr`/`s`/`me`/`od` sur `gx[0]` par `pico8_class_get_field` : `qh` garde sa `pico8_class_t` et ses `defaults`, rien ne change. `cart.c:84-88` asserte les liens `parent` : inchangés. |
| Sites qui reçoivent un `qh` | tous par l'API d'entité (aucun `iget` journalisé sur un `qh`) : `dg` (`bootstrap.c`), `hy:lj`/`hk`/`hb` (`economy.c`), `ck` sur `cg.qh` (`economy_formulas.c:160`, lit `jy`) et `bx`/`p` (`:19-43`), `da` et `gg` (`economy_score.c:39-145`, lit `qh`, `#cg.qh`), `bd`/`bb` (`economy_tiers.c`), `gk`/`ij`/`na`/`ma()` (`entity_geom.c`), `pz` (`msgbox.c`), fonctions de `jo` (`planet.c`, `planet_display.c`, `planet_trade.c`), `bv`/`dl`/`d_` (`probe.c`), `jc:pq`/`gv` (`slipway.c`), `slipway_display.c:30`, survol et `bh` (`ui.c`, `lheld` tient la struct : TROU-6 inchangé), `mz` (`wormhole.c:80,232`). Accès à `->fields` : seulement aux sites déjà aiguillés (`ke.c:45-46`, `bootstrap.c:101`, `probe.c:128`). |
| Tests qui lisent un `qh` | `empty_planet_cleared` (`me`, `ra` du laboratoire par `pico8_entity_get_field`, mais tenu dans un `pico8_instance_t *lab` casté, `:215-220`) ; `test_pico8_jo` (`daughter_of` ne reconnaît une fille que si elle est `INSTANCE`, `:428` ; `test_jo_is_a_struct` asserte que `cg.qh[1]` est une **instance** de `pico8_qh_class()`, `:714-715`) ; `test_pico8_ie` (l'objet rendu par `ldown` sur `gx[0]`, lu par l'API ; la comparaison `->cls == ke` est déjà gardée par le tag, `:481,558,564`). Lectures de `->cls` sur une entrée de `hv` sans tag (`geoharvest_burst:85`) ou gardées (`test_pico8_bg:572`, `test_pico8_qb:390`) : dans les bornes d'une struct `qh`, jamais égales, jamais déréférencées. |
| Filet | 3 rejeux construisent un `qh` (`qh  8  3/15`) : ils le dessinent tenu (`dl` via `cv.ev`), posé (`jo:kj`) et le font osciller (`ly`). |

## Solution

1. **Enum** : ajouter `LY` (nom `"ly"`). Pour toute struct déjà convertie (`jo` comprise), l'entrée est NULL et `"ly"` est cherché comme champ : absent, comportement inchangé (un `jo` en état `"ly"` n'a pas de méthode `ly`, comme dans le cart).
2. **Préparation dans le module de la planète, sans changement de comportement** :
   - le constructeur de `jo` est coupé en deux : une fonction publique d'initialisation d'un `pico8_jo_t` déjà alloué, qui reçoit le descripteur à poser (`pico8_ke_struct_init`, les 31 membres à nil, les 14 défauts statiques de `jo`), et le constructeur de `jo` qui alloue, l'appelle avec le descripteur de `jo`, puis fait exactement ce qu'il fait aujourd'hui (clés, couverture, `pq`, `pico8_ke_struct_pq`) ;
   - `pico8_jo_pq` devient public (même corps, même signature `pico8_instance_t *`) : c'est le `pq` effectif de `qh` ;
   - les 11 dispatchers de `jo` deviennent publics (déclarés dans l'en-tête de la planète, corps inchangés), sur le modèle de `pico8_bv_dispatch`/`dl`/`d_` déjà publics dans `probe.h`, pour que le descripteur de `qh` soit une table `static const` qui les nomme.
3. **Struct `qh`**, publique dans le module de la famille : `pico8_jo_t` en premier membre, puis les 10 membres `pico8_value_t` `hp`, `px`, `dq`, `qh`, `od`, `fp`, `dw`, `s`, `ba`, `et`. **Aucun membre ne répète un nom de `jo` ou de `ke`.**
4. **Défauts pris par REF, jamais re-parsés** : `pico8_planet_family_init` déclare `qh` comme aujourd'hui (même spec, même parent, même nom de couverture, mêmes 4 installations), pose `pico8_class_set_struct_new(g_qh_class, …)`, puis lit **une fois** les 13 défauts propres de `qh` dans sa `pico8_class_t` (par `pico8_class_get_field`) vers un état statique du module. `pico8_planet_family_shutdown` remet cet état à nil avant de libérer la classe, sans libérer aucune table.
5. **Constructeur de `qh`**, dans l'ordre (celui de `pico8_class_instantiate` pour une instance `qh`) :
   1. alloue un `pico8_qh_t` ;
   2. initialise la partie `jo` avec **le descripteur de `qh`** (défauts de `jo`, `me` à `"unknown"`) ;
   3. met les 10 membres propres à nil, puis copie les 13 défauts statiques de `qh` : `me` dans le `pico8_ke_t` (`"kz"`), `nu`, `hf`, `mi` dans les membres de `jo`, les 9 autres dans les membres propres ;
   4. recopie la clé nommée `id` de la table reçue (NULL permise) et **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise ; ne garde aucun pointeur sur la table ;
   5. enregistre `"qh"` au crochet de couverture ;
   6. appelle `pico8_jo_pq` sur la struct (`oe`, puis `hp`=1 → `lz(gb[1])`) ;
   7. appelle `pico8_ke_struct_pq`.
6. **Descripteur de `qh`** (`static const`) :
   - `HD`, `KJ`, `LDOWN`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL` = **les mêmes pointeurs** que le descripteur de `jo` ; `HQ` = `pico8_bv_dispatch`, `EV` = `pico8_dl_dispatch`, `EY` = `pico8_d__dispatch`, `LY` = `pico8_qh_ly_dispatch` = **les mêmes pointeurs** que les 4 installations de `qh->defaults` ; `PH`, `KZ`, `RDOWN`, `MD`, `OL`, `PR`, `EB` NULL ;
   - **lecteur, chaîné** : les 10 membres propres (nil = absent), sinon `pico8_jo_struct_class()->field` — qui lit les 31 membres de `jo` sur la même adresse (premier membre), puis `pico8_ke_field`. C'est la chaîne `pico8_ke_field` de 1a.2, un étage plus haut ;
   - **écrivain, chaîné** : `et` → membre propre ; `ra`, `p` → membres de `jo` du `pico8_qh_t` ; tout autre nom → `pico8_jo_struct_class()->set_field`, qui n'accepte que ses 17 noms et rend 0 sinon. `qh` accepte donc exactement **20 noms** : les 17 de `jo` (écrivains génériques de `cg.jo` et fonctions de `jo`, atteignables sur un `qh`) plus `ra`, `p`, `et`, écrits par sa méthode propre `hq`. L'écrivain de `jo` **ne change pas** : sur un `jo` nu, `ra`/`p`/`et` restent refusés (assert de 1a.17) ;
   - accesseur public du descripteur, comme `pico8_jo_struct_class()`.
7. **Fonctions de `qh` et de `jo` : aucune ne change de corps ni de signature.** `pico8_qh_ly_dispatch` (→ `na`), `bv`, `dl`, `d_`, `na` et toutes les fonctions de `jo` restent à double forme. Les 4 installations `->defaults` de `qh` restent (ses descendantes), les 11 de `jo` et son `pq` de classe aussi.
8. **`pico8_qh_new`** rend `pico8_qh_t *`, même paramètre (aucun appelant). `ie:ldown` n'est pas touché : il reçoit la struct par la délégation de `pico8_class_instantiate` et la rend en REF.
9. **Tests** : `empty_planet_cleared` (réécriture minimale, voir Testing) ; `test_pico8_jo` : lecture seulement (voir Testing).
10. **Plan** : à la ligne 1a.18 de `docs/plan-3.md`, ajouter « Découvert à 1a.18 : … » avec au minimum : l'enum gagne `LY` (`dg` appelle `qc[qc.me]`, `me="ly"` après `d_`) ; les 11 dispatchers de `jo`, `pico8_jo_pq` et l'initialisation d'un `pico8_jo_t` deviennent publics (descripteur `static const` de la fille, `pq` effectif) ; `et` (écrit par `bv` = `qh.hq`) → membre de `qh`, `ra`/`p` écrits par `bv` → acceptés par l'écrivain de `qh` sur les membres de `jo` ; défauts `me`/`nu`/`hf`/`mi` rangés dans les membres hérités ; `test_pico8_jo` reconnaît une fille convertie par son descripteur.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `qh` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `nl`…`fr` et `ga` à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `qh` mesurée (constructions, défauts, champs écrits et par qui, noms lus absents, méthodes par nom, `->defaults`, sites qui reçoivent un `qh`, tests, rejeux qui le construisent), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux une struct `qh` dont le premier membre est `pico8_jo_t` et qui n'ajoute que les 10 membres mesurés, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun membre de `qh` ne répète un nom de `jo` ou de `ke`, afin qu'un écrivain générique (`bb` sur `mi`) et un lecteur ne voient jamais deux copies d'un même champ.
5. En tant que gardien du filet, je veux qu'un laboratoire construit par `ie:ldown` depuis `gx[0]` ait son `id` `"sN"`, les défauts de `qh` et ceux de `jo` qu'il hérite, afin que la boutique et le dessin soient identiques au pixel près.
6. En tant que mainteneur, je veux qu'une clé de construction autre que `id` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt (§7).
7. En tant que gardien du filet, je veux que `jo:pq` coure sur la struct avant `lt`, avec `lz(gb[1])` puisque `hp`=1, afin que `oe`, `hn`, `jy`, `kw` existent avant tout enregistrement, comme dans le cart.
8. En tant que gardien du filet, je veux qu'un `qh` neuf entre dans `hv`, `cv.kj`, `cv.ev`, `cv.hd`, `cv.hk`, `cv.hb`, `cg.qh`, `cg.jo`, `cg.np`, `cg.prober`, dans le même ordre relatif, afin que dessin, prix affiché, survol, revenus, `ck` et `da` voient les laboratoires comme avant.
9. En tant que gardien du filet, je veux que `hq` (`bv`), `ey` (`d_`) et `ev` (`dl`) soient trouvés par nom sur une struct `qh`, afin que prise, prix et pose marchent comme avant.
10. En tant que gardien du filet, je veux que `dg` appelle `qh.ly` (`na`) sur un laboratoire en état `"ly"` et rien en état `"kz"`, afin que le laboratoire posé oscille, sans refaire le bug 529f355 sur `ly`.
11. En tant que gardien du filet, je veux que les 11 méthodes de `jo` soient trouvées sur une struct `qh` par les mêmes pointeurs que sur une struct `jo`, afin qu'un laboratoire et une planète exécutent le même code.
12. En tant qu'auteur des étapes 1a.19 à 1a.25, je veux que les 4 installations `->defaults` de `qh`, les 11 de `jo`, le `pq` de classe de `jo` et les corps de `bv`, `dl`, `d_`, `na`, `pico8_qh_ly_dispatch` restent en place et à double forme, afin que chaque descendante non convertie trouve ses méthodes en remontant `parent`.
13. En tant que mainteneur, je veux que les tables `nu`, `px`, `ba` d'un `qh` soient la même REF que celles de `qh->defaults`, afin qu'une struct `qh` et une descendante qui hérite lisent la même table.
14. En tant que mainteneur, je veux que la vue `ie.ke` continue de lire `rr`/`s`/`me`/`od` sur `gx[0]` par la chaîne de classes, afin que les icônes de recherche gardent sprite et prix.
15. En tant que mainteneur, je veux que l'écrivain de `qh` accepte `et`, `ra`, `p` et délègue tout le reste à celui de `jo`, afin que la liste des 17 noms de `jo` reste la seule source pour les écrivains génériques et qu'un nom hors des 20 soit un arrêt.
16. En tant que mainteneur, je veux que le lecteur de `qh` délègue à celui de `jo` puis de `ke`, afin qu'une fonction de `jo` lise un laboratoire sans savoir que c'est un laboratoire.
17. En tant que gardien de la couverture, je veux que le constructeur enregistre `"qh"` une fois par laboratoire, et que `jo` et les descendantes gardent leur compte, afin que `class-coverage-net` soit identique ligne à ligne.
18. En tant que vérificateur, je veux une caractérisation de `qh` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas.
19. En tant que vérificateur, je veux que cette caractérisation lise au moins une instance de chaque descendante de `qh` construite par le filet (champ hérité de `qh` et résultat d'une méthode de `qh`), afin de prouver que l'héritage survit à la conversion de leur parente.
20. En tant que gardien des tests de régression, je veux qu'`empty_planet_cleared` ne change que sa façon de tenir le laboratoire lu, et qu'il soit prouvé rouge avec le bug 529f355 remis, afin que la réécriture ne l'ait pas vidé.
21. En tant que gardien des caractérisations, je veux que `test_pico8_jo` ne change que sa façon de reconnaître une fille (et sa vérification de forme de `qh`), sans qu'aucune valeur figée ne bouge, afin que 1a.17 reste une preuve.
22. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
23. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.18, afin que la liste des étapes dise ce que l'étape a réellement touché.
24. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés.
25. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais. J'ai donc chargé le skill `guideline` et je l'ai appliqué moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : en-tête des structs (enum `LY`) ; module de la planète (initialisation d'un `pico8_jo_t` séparée de l'allocation, `pico8_jo_pq` et 11 dispatchers publics, aucun corps changé) ; module de la famille de planètes (struct `qh`, statiques, constructeur, descripteur, lecteur, écrivain, accesseur, `pico8_qh_new`). Aucun autre module de `src/`.
- **Descripteur `static const`, dispatchers de `jo` rendus publics** (G). Le §5 dit « table `static const` […] une surcharge remplace l'entrée héritée » ; une table `static const` ne peut nommer que des symboles visibles, et `probe.h` exporte déjà `bv`/`dl`/`d_` pour la même raison. Rejeté : copier le descripteur de `jo` à l'init dans une table mutable, qui s'écarte du texte du plan et ouvre la table à l'écriture ; ajouter des fonctions struct seules, interdit tant qu'une descendante existe.
- **Initialisation de `jo` séparée de son allocation** (G) : c'est le seul moyen de construire la partie `jo` d'une fille sans re-parser ni dupliquer les 14 défauts. Le constructeur de `jo` garde exactement son comportement (sa caractérisation et ses mutations de 1a.17 le prouvent). Rejeté : appeler le constructeur de `jo` puis agrandir (`realloc`) — il enregistre la planète au `lt` avant qu'elle soit un `qh`.
- **Défauts de `qh` rangés dans les membres hérités quand le nom existe** (`me`, `nu`, `hf`, `mi`) : sinon l'écrivain générique de `jo` (`bb` → `mi`) écrirait `jo.mi` pendant que le lecteur de `qh` rendrait un `qh.mi` périmé. C'est le piège principal de la première fille d'une parente convertie.
- **`ra`, `p`, `et` acceptés par l'écrivain de `qh`, pas par celui de `jo`** : ils ne sont écrits que par `bv`, méthode propre de `qh` (`qh.hq=bv`) — règle « seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille ». `et` devient membre propre ; `ra`/`p` sont déjà des membres de `jo` (clés de `bw`) : aucun membre ajouté, seule la permission d'écriture est propre à `qh`. Élargir l'écrivain de `jo` casserait l'assert de forme de 1a.17 (`write_ra` doit échouer sur un `jo` nu).
- **Lecteur et écrivain chaînés par le descripteur public de `jo`** (`pico8_jo_struct_class()->field`/`->set_field`), sur le modèle de la chute vers `pico8_ke_field` : aucune fonction statique de `jo` n'est exposée pour cela, et la liste des 17 noms reste unique.
- **13 défauts copiés depuis la classe à l'init** (G), scalaires compris, comme `jo`. Rejeté : les écrire en dur (deux sources).
- **`hf`=false** est un membre présent (booléen faux), pas nil : même lecture que l'instance.
- **Sémantique de nil** : un membre nil est absent. Mesuré, les seules écritures nil sur un `qh` frappent `ep` et `fo`, sans défaut dans la chaîne. Pas d'écart.
- **`ly` par l'enum, pas par un champ fonction** : le §5 fait passer par l'enum toute méthode appelée par nom de chaîne ; `dg` appelle `qc[qc.me]` sur l'ensemble fini des états. `kz` est déjà dans l'enum (entrée NULL chez `qh`, donc absent, comme sur l'instance).
- **Clés dynamiques** : aucune nouvelle.
- **`ie:ldown`, `gx[]`, la vue `ie.ke`, `cart.c`** : inchangés ; la délégation de `pico8_class_instantiate` suffit.
- **`na` écrit `ra.y` en place** dans le vecteur pris par `bv` dans `ui.qa` (même aliasing que le cart) : inchangé, la struct garde la REF comme l'instance gardait son champ.
- **TROU** : aucun écart attendu. Si l'implémenteur en trouve un, règle §7 (préserver, `TROU-N`, `/* TROU-N */`).
- **Commentaires** (CLAUDE.md) :
  - à ajouter : contrat de la struct `qh` (jamais libérée, TROU-7 ; `nu`/`px`/`ba` sont les tables de la classe, empruntées) ; piège « un défaut de `qh` dont le nom est un membre de `jo`/`ke` va dans ce membre » ; contrat de l'écrivain (`et`, `ra`, `p`, puis les 17 de `jo`) ; assert des clés ; contrat de la fonction d'initialisation d'un `pico8_jo_t` (ne fait ni `pq` ni `lt`) ;
  - à corriger si rendus faux : le commentaire des `_new` de `planet_family.h` (« yields a class-less instance »), celui de `pico8_jo_t` dans `planet.h` s'il dit que seules les filles instances atteignent les fonctions de `jo` ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` et `make frames-check` verts. La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_qh` sur l'ancien code** (voir Testing), valeurs figées, descendantes comprises. Arrêt ici si un rejeu ne construit pas le `qh` ou la descendante attendue : mesure fausse, à corriger dans le PRD, pas un test durci.
2. **Préparation sans conversion** : enum `LY` ; initialisation d'un `pico8_jo_t` séparée de son allocation ; `pico8_jo_pq` et les 11 dispatchers publics. Suite verte, `test_pico8_jo` et `test_pico8_qh` inchangés.
3. **`qh` converti** : struct, statiques de défauts, constructeur, descripteur, lecteur, écrivain, accesseur, `pico8_class_set_struct_new`, `pico8_qh_new` typé ; `empty_planet_cleared` et `test_pico8_jo` relus (voir Testing). Les deux caractérisations passent **sans changer leurs valeurs figées** ; les vérifications de forme s'ajoutent à `test_pico8_qh`.
4. **Preuves** : preuve rouge d'`empty_planet_cleared`, mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.18 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF et des pointeurs de fonction. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_qh`** (dans `make unit`, lié comme `test_pico8_jo`, sur le cart réel). Il lit **uniquement** par l'API à double forme et par `pico8_class_get_field(pico8_qh_class(), …)` / `pico8_class_get_field(pico8_jo_class(), …)`, qui existent avant et après ; son code est le même avant et après, aux vérifications de forme près. Chaque scénario tourne dans un fils forké ; les rejeux depuis le titre tournent côte à côte.
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction dirigée** (partie lancée par `pico8_eu(pico8_js_new(bo[2]))`) : `ldown` sur l'icône qui tient `gx[0]` → le laboratoire. Empreinte de `id`, `me`, `t`, `hp`, `dq`, `qh`, `od`, `fp`, `dw`, `mi`, `s`, `hf`, `hs`, `fa`, `ma`, `ca`, `gq` ; présence de chacun des 10 membres propres et des 31 de `jo` ; présence des 15 méthodes (`hd`, `kj`, `ldown`, `hb`, `gg`, `gv`, `hk`, `kf`, `cn`, `ib`, `ll`, `hq`, `ev`, `ey`, `ly`) et absence de `ph`, `kz`, `rdown`, `md`, `ol`, `pr`, `eb` ; appartenance et rang dans `hv`, `cv.kj`/`ev`/`hd`/`hk`/`hb`, `cg.qh`/`jo`/`np`/`prober`. **Identité** : `nu`, `px`, `ba` = REF de `pico8_class_get_field(pico8_qh_class(), …)` ; `kq`, `em`, `ka`, `dz`, `cq`, `gq` = REF de la classe de `jo` ; `kw` ≠ REF de la classe de `jo` (`lz` a couru) ; `hn` = `gb[1]`. Figée.
    2. **Cycle de vie dirigé** : `mv(q,"hq",mp,rp)` à un point vide puis sur une planète → `ra` (la REF passée), `p`, `et` (vrai/faux/entité), `fo` de la cible ; `mv(q,"ey",nil)` avec et sans crédits → `me`, `t`, `p_`, crédits ; `dg` sur un laboratoire en `"ly"` → `ra.y` avant/après ; `dg` sur un laboratoire en `"kz"` → `ra` inchangé. Figés.
    3. **Rejeux du filet qui construisent un `qh`** : `empty_planet_cleared_events` (direct), `session_events` et `long_session_events` (`--from-title`). Par frame, pour chaque entrée de `cg.qh` : `id`, `me`, `t`, `ra` x/y, `p`, vérité et nature de `et`, `mi`, `p_`, `kw.qe`/`kw.mu`, taille de `oe`/`jy`, `m_` ; plus `#cg.qh` et une empreinte de l'écran ; repliés en une empreinte de séquence figée, avec le nombre de `qh` vus (4, 2, 2).
    4. **Descendantes construites par le filet** (règle « Méthodes d'une parente convertie ») : `session_events` (`nr`, `fr`, `pm`), `events_recorded_1790198294` (`hj`, `mx`, `pm`), `trading_hub_session_events` (`nl`, `nr`), `long_session_events` (`os`), depuis le titre. Pour la première instance de **chaque** descendante (`nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`), à sa première frame puis en fin de rejeu : un champ hérité de `qh` qu'elle ne surcharge pas (`dq`, `fp` ou `dw`, `ba` pour `os`, `hf`) avec l'identité des REF de table avec la classe de `qh` ; `hq`, `ev`, `ey` lus sur elle = les fonctions de `pico8_class_get_field(pico8_qh_class(), …)` ; `ly` lu sur elle (celui de `qh`, ou `nl:ly` pour `nl`) appelé, et `ra.y` avant/après. Figés. Les sept sont exigés parce que le filet les construit.
  - **Forme** (ajoutée au morceau 3) : chaque laboratoire construit par `ie:ldown` est `STRUCT` avec le descripteur de `qh` ; pour les 11 méthodes de `jo`, l'entrée du descripteur de `qh` est **la même** que celle de `pico8_jo_struct_class()` ; pour `hq`, `ly`, `ev`, `ey`, elle est **la même** que `pico8_class_get_field(pico8_qh_class(), nom)` ; `pico8_entity_set_field` passe pour les 20 noms et asserte (dans un fils) pour `hp`, `id`, `s`, `od` ; `pico8_entity_set_field(jo_nu, "et", …)` asserte toujours sur un `jo` nu ; une clé de construction autre que `id`, ou positionnelle, asserte ; les descendantes restent `INSTANCE`.
  - **Mutations** (au moins 12, notées dans le commit, mutants équivalents justifiés) : `LY`, `EV`, `HQ` ou `EY` absent du descripteur de `qh` ; une entrée de `jo` (`LL`, `GG`, `KJ`) absente du descripteur de `qh` ; `me` initial laissé à `"unknown"` ; `mi` dupliqué en membre propre ; `ba` re-parsé ou copié au lieu de la REF de la classe ; `id` non recopié ; `pico8_jo_pq` omis ou après `pico8_ke_struct_pq` ; couverture non enregistrée (attrapée par `class-coverage-net`) ; écrivain qui refuse `et` ou ne délègue pas à `jo` (`mi`) ; lecteur qui ne délègue pas à `jo` ; une installation `->defaults` de `qh` retirée (attrapée par les descendantes).
- **`empty_planet_cleared`** (§6, test nommé) : il lit une struct `qh` (le laboratoire posé). Seule réécriture : le laboratoire est tenu en `const void *` au lieu d'un `pico8_instance_t *` casté (`:215,220`) ; ses lectures passent déjà par `pico8_entity_get_field`. Frames, fixture, asserts, messages, seuils : inchangés. **Preuve rouge sur le code final** avec le bug d'origine (529f355, « `ll` de `jo` non enregistré ») : retirer l'entrée `LL` du descripteur de `jo` (le vide est un `jo` nu) → l'assertion `qh->count == 1` échoue (noter laquelle) ; restaurer → vert. Par `make tmp/test_pico8_empty_planet_cleared && ./tmp/test_pico8_empty_planet_cleared`. Si le bug remis ne rend **pas** le test rouge, c'est un arrêt (§7), pas un test durci.
- **`test_pico8_jo`** (caractérisation de 1a.17) : deux changements de lecture, aucune valeur figée touchée — `daughter_of` reconnaît aussi une struct dont le descripteur est celui de `qh` (sinon les laboratoires disparaîtraient de `replay_daughters` et l'exigence `QH` échouerait) ; la dernière vérification de forme de `test_jo_is_a_struct` devient « `cg.qh[1]` est une struct au descripteur de `qh` » (elle disait « instance de `pico8_qh_class()` »). Toutes les empreintes (`SESSION_HASH`, `LONG_HASH`, `EMPTY`, …) restent identiques.
- **Tests existants** : `test_pico8_ie` (la comparaison `->cls == ke` est déjà gardée par le tag ; valeurs figées identiques), `js`, `jc`, `mz`, `bg`, `bg_kj`, `fq`, `qb`, `iv`, `nf`, `je`, `mo`, `mh`, `jb_ol`, `class_chain`, `entity_smoke` : diff vide, valeurs figées identiques. Les six autres tests nommés à 1a.17 (`nil_row`, `planet_gh_row_lq`, `planet_kj_dispatch`, `playability`, `rich_planet_exports`, `slipway_drag`), `ga_export_panel_by`, `geoharvest_burst`, et tous les autres : diff vide, verts.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** : `test_pico8_jo` (galaxie, rejeux, filles depuis le titre en parallèle, identité des REF, forme, mutations), `test_pico8_ie` (construction par `ldown` sur `gx[]`), 1a.17 (preuve rouge d'`empty_planet_cleared` par l'entrée `LL`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `qh` n'est pas `qh  8  3/15`, ou si une autre ligne (`jo  1710  15/15`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`…) change. Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier : si l'assertion des clés de construction ou celle de l'écrivain se déclenche sur un rejeu ou un test ; si un membre propre hors des 10 est ajouté ; si un membre propre répète un nom de `jo` ou de `ke` ; si l'écrivain de `qh` accepte un nom hors des 20, ou si la liste des 17 noms de `jo` change.
- `FAILED:` si une autre classe que `qh` est convertie ou change de forme (`nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`, `ui`, `lq`, `hy`, `hp`, `pj`, `g_`, `fv`, `trade`…).
- `FAILED:` si une fonction de `qh` ou de `jo` atteignable par une descendante non convertie devient struct seule : une des 4 installations `->defaults` de `qh`, une des 11 de `jo`, ou `pico8_class_set_pq(g_jo_class, …)` retirée ou changée de fonction ; un corps de `pico8_qh_ly_dispatch`, `pico8_na`, `pico8_bv`, `pico8_dl`, `pico8_d_` ou d'une fonction `pico8_jo_*` qui caste `self` en `pico8_qh_t *`/`pico8_jo_t *`, lit un membre directement, ou revient à `pico8_instance_get_field`/`pico8_instance_fields` ; une entrée du descripteur de `qh` qui n'est pas le même pointeur que l'entrée de `jo` ou l'installation de `qh` correspondante.
- `FAILED:` si une table de défaut de `qh` est re-parsée depuis la spec (un second `pico8_ob(` ou `pico8_class_declare` sur un fragment de la spec de `qh`), ou si `nu`/`px`/`ba` d'un `qh` neuf ne sont pas les REF de `qh->defaults`.
- `FAILED:` si `empty_planet_cleared` change autrement que dans sa façon de tenir ou lire l'état (frames, fixture, asserts, messages, seuils modifiés) ; si le commit ne rapporte pas sa preuve rouge (bug remis, rouge, restauré, vert) ; ou si le bug remis ne le rend pas rouge.
- `FAILED:` si un autre test de régression (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une valeur figée de `test_pico8_qh` a été changée après la conversion, ou une de `test_pico8_jo` (empreintes, comptes, frames), `ie`, `js`, `jc`, `mz`, `bg`, `bg_kj`, `fq`, `qb`, `iv`, `nf`, `je`, `mo`, `mh` ; si `test_pico8_jo` change au-delà de `daughter_of` et de la vérification de forme de `qh` ; ou si la caractérisation ne lit pas au moins une instance de chacune de `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr` construite par un rejeu du filet (champ hérité et méthode de `qh`).
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- l'enum vaut exactement celle de 1a.17 plus `LY` ;
- la struct `qh` n'a que `pico8_jo_t` et les 10 membres ; le descripteur n'a que les 15 méthodes ; l'écrivain n'accepte que les 20 noms ;
- grep : les seuls `g_qh_class->defaults` restants sont les 4 installations ; aucun `lookup_named("qh")` ;
- hors du module de la famille, de celui de la planète (préparation seulement, aucun corps de fonction de `jo` changé) et de `pico8_struct.h`, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_qh.c` (nouveau), `test_pico8_empty_planet_cleared.c`, `test_pico8_jo.c` et le `Makefile` (cible et `unit`) bougent ;
- la ligne 1a.18 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.18 ».

## Out of Scope

- Les descendantes de `qh` (1a.19 à 1a.25) : leurs structs, leurs `->defaults` (`nl.ly`, `hj.kj`/`mm`/`pq`, `pm.pq`, `nr`, `mx`, `fr`) ; `ga` (1a.26).
- Le passage à la forme struct seule des fonctions de `qh` et de `jo`, le retrait de leurs installations `->defaults`, le retypage de leurs signatures (dernière descendante : 1a.25 pour `qh`, 1a.26 pour `jo`).
- `ie` et sa vue `ie.ke` (déjà convertis) : aucun changement.
- `ui` (1a.31), `lq` (1a.34), `hy` (1a.35), `hp` (1a.33), `trade` (1a.32).
- Le typage C des membres (1b) ; `gb` en données constantes (D).
- Libérer un laboratoire (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Première fille d'une parente convertie** : 1a.2 à 1a.17 n'ont converti que des filles de `ke`, qui n'a aucune méthode. `qh` est la première à hériter d'un descripteur rempli ; les choix de cette étape (initialisation de la parente séparée de son allocation, dispatchers publics, lecteur/écrivain chaînés par le descripteur public de la parente, défauts rangés dans les membres hérités) servent de modèle à 1a.19-1a.26.
- **Le filet sert de piège** : 8 laboratoires le traversent, tenus (`cv.ev`, `hq`), posés (`ey`, `ly`) et refusés (`p_`). `pico8_instance_get_field`/`pico8_instance_fields` assertent sur une struct : un site à simple forme oublié casse `frames-check`. La sonde n'en a trouvé aucun.
- **Sans `LY`**, un laboratoire posé ne bouge plus : l'écart est de 0,06 px par frame, invisible pour `empty_planet_cleared` (tolérance 0,5 px sur 6 frames) mais visible au filet sur `session_events`/`long_session_events` et dans la caractérisation (scénario 2).
- **`test_pico8_jo` doit être relu** parce qu'il reconnaît les filles par `->cls` sur une instance : après conversion, les laboratoires n'y seraient plus vus. Relire n'est pas refiger : ses empreintes doivent rester celles de 1a.17.
- Mesures : grep sur `src/`, `tests/`, `tools/`, `Makefile` à c825868 ; lecture de `pico8/lua.lua:237-251,384-579,905-918,998-1110,1182-1186,1244-1316`, de `pico8_planet*.c/h`, `pico8_class.c/h`, `pico8_entity.c/h`, `pico8_struct.h`, `pico8_ke.c`, `pico8_probe.c/h`, `pico8_entity_geom.c`, `pico8_research_icon.c`, `pico8_ui.c`, `pico8_bootstrap.c`, de `test_pico8_empty_planet_cleared.c`, `test_pico8_jo.c`, `test_pico8_ie.c`, `test_pico8_bg.c`, `test_pico8_geoharvest_burst.c` et de `tools/class_coverage_net_expected.txt` ; sonde de mesure (160 sites distincts `(opération, nom, fichier:ligne)` sur un `qh` exact, écritures nil séparées) et comptes par rejeu dans une copie `git archive` jetable (`make -j8 test` sortie 0, 15 conformes, `class-coverage-net` sortie 0).
