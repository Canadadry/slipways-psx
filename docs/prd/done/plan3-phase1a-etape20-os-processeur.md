---
title: "Plan 3, étape 1a.20 : `os` (le processeur) devient une struct fille de `qh` sans membre propre ni méthode propre ; les fonctions de `qh` et de `jo` restent à double forme"
description: "Le processeur `os` est encore une instance de la couche objet, construite une fois sur le filet (`long_session_events`) par `ie:ldown` depuis `gx[1]`. Il n'a aucune descendante, aucune méthode propre et aucun champ que seule sa méthode écrirait : il passe en struct `pico8_os_t` réduite à `pico8_qh_t`, ses 7 défauts rangés dans les membres hérités (`hs` dans celui de `jo`), un descripteur `static const` aux 15 entrées de `qh` et au lecteur/écrivain de `qh`, sans rien changer à `qh` ni à `jo`."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.20 — `os` : `qh` (`planet_family.c:331`), `gx[1]`.

(À HEAD c868719, la ligne 331 a glissé avec l'ajout des structs `qh` et `nl` : la déclaration `g_os_class = pico8_class_declare(g_qh_class, …)` est à `planet_family.c:587`, son nom de couverture à `:594`, `pico8_os_new` à `:732`, et `gx[1] = pico8_os_class()` à `cart.c:224`. La ligne ne nomme aucun test de régression.)

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag (`jo` pour `cg.jo`) ; seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe.
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre […] reste à double forme […]. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante (pour `jo` : 1a.26).
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe […] ; une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec.

Le §5 dit aussi, pour la couche : `pico8_os_t` commence par `pico8_qh_t` (premier membre), qui commence par `pico8_jo_t`, qui commence par `pico8_ke_t`.

§6 : aucun test de régression nommé par la ligne ; aucun des fichiers de régression ne construit un `os` (mesuré, voir plus bas). Les trois caractérisations qui peuvent lire un `os` (`test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`) changent leur façon de le reconnaître, jamais une valeur figée — avec une exception mécanique dans `test_pico8_qh`, tranchée plus bas. §7 : un commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (`os  1  1/15`, toutes les autres lignes inchangées) ; un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.20, avec « découvert à 1a.20 » dans le message ; un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.19 ont posé (jusqu'à c868719), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`). L'enum vaut `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL`, `LY`. Un nom absent de l'enum, ou dont l'entrée est NULL, est cherché comme champ ; `pico8_mv` rend 0 sur un champ absent. L'écrivain rend 0 pour un nom sans membre, et `pico8_entity_set_field` asserte alors (« a field under no rule of plan-3 section 5 — stop »).
- `pico8_ke_t`, `pico8_ke_struct_pq` ; `pico8_jo_t` (31 membres, dont `hs`), `pico8_jo_struct_init`, `pico8_jo_pq`, les 11 dispatchers publics de `jo`, écrivain de `jo` à 17 noms.
- **1a.18** : `pico8_qh_t` (`pico8_jo_t` puis `hp`, `px`, `dq`, `qh`, `od`, `fp`, `dw`, `s`, `ba`, `et`), `g_qh_defaults` (13 défauts lus par `pico8_class_get_field`), descripteur `static const g_qh_struct_class` (11 dispatchers de `jo` + `HQ`=`pico8_bv_dispatch`, `EV`=`pico8_dl_dispatch`, `EY`=`pico8_d__dispatch`, `LY`=`pico8_qh_ly_dispatch`), lecteur `pico8_qh_field` et écrivain `pico8_qh_set_field` (20 noms : `et`, `ra`, `p` puis les 17 de `jo`), tous deux `static`, atteignables de l'extérieur par `pico8_qh_struct_class()->field`/`->set_field`.
- **1a.19** (le modèle à suivre, `docs/prd/done/plan3-phase1a-etape19-nl-plaque-commerciale.md`, commit c868719) : `pico8_qh_struct_init(self, cls)` `static`, coupée du constructeur de `qh` (jo avec le descripteur donné, 10 membres à nil, 13 défauts copiés ; ni `pq`, ni `lt`, ni couverture, ni clé) ; `pico8_nl_t` ; `g_nl_defaults` rangé à l'offset du membre hérité quand le nom existe ; descripteur de `nl` qui nomme les mêmes 14 fonctions que celui de `qh` et remplace `LY` ; lecteur et écrivain chaînés ; constructeur (alloue, `pico8_qh_struct_init`, membres propres à nil, défauts propres, clé `id` seule sinon assert, couverture, `pico8_jo_pq`, `pico8_ke_struct_pq`) ; `pico8_nl_new` typé ; `test_pico8_nl` (caractérisation figée sur l'ancien code, rejeu depuis le titre, forme, mutations) ; `test_pico8_qh`/`test_pico8_jo` reconnaissent `nl` par son descripteur.
- Les aides partagées qu'atteint une descendante de `qh` sont déjà à double forme : `pico8_bv`, `pico8_dl`, `pico8_d_`, `pico8_na`, `pico8_qh_ly_dispatch`, et toutes les fonctions de `jo`.

### Ce que fait le cart

`pico8/lua.lua` :

```
:385-388   jo=ke:jr([[ nu=o("jo","np","prober"), me="unknown", hf=1,hs=1, … ]])
:1095-1100 os=qh:jr([[ nu=o("jo","np","prober"), hp=4,px=o("processor",c=13),
             qh=f,s=100, od=15, hs=0.5, ]])          -- aucune fonction os:… ne suit
:1182      eu : gx={qh,os,nr,fr,nl,mx,hj}              -- gx[1] (base 0 côté C) = os
:1302-1311 bb : if pt.nl … ; if not pt.hf then … jy … end
:1312-1316 da : pt.qh and #cg.qh or pt:gg() and es[pt.mi] or 0
:1325-1331 db : for e in all(pt.by) … if oz.hf then total+=dk.trade*pt.hs*(…)
:932-935   ie:ldown : ie.id+=1 ; return self.ke({id="s"..ie.id})
```

Points décisifs :

- **Une seule construction** : `ie:ldown` sur l'icône qui tient `gx[1]`, clé `id` seule. `os` n'a pas de `fz`, n'est pas enregistré par nom, n'est pas `kv[hp.cw]`. `pico8_os_new` n'a aucun appelant (`src/`, `tests/`, `tools/`).
- **Aucune descendante, aucune méthode propre** : rien ne dérive de `os` (arbre du §2, `grep pico8_class_declare(g_os_class` vide) et le cart ne définit aucune fonction `os:…`. Toutes ses méthodes sont celles de `qh` et de `jo`, qui restent à double forme (atteintes par `hj`, `pm`, `mx`, `nr`, `fr`, `ga`).
- **Tous les défauts de `os` tombent sur un membre hérité** : `nu`, `hs` (`jo`) ; `hp`, `px`, `qh`, `s`, `od` (`qh`). `os` n'a donc **aucun membre propre**.
- **`hs=0.5`** surcharge le défaut de `jo` (`hs=1`) : c'est le seul défaut d'un `os` qui soit un défaut de `jo` et non de `qh`, et il n'est lu que par `db` quand un export de l'`os` va vers une planète dont `hf` est vrai.
- **`nu` de `os` ne contient pas `"qh"`** : un `os` n'entre pas dans `cg.qh`. **`qh=f`** : `da` prend la branche `gg`. **`hp=4`** : le `pq` de `jo` court `lz(gb[4])`.
- **`od=15` est égal au défaut de `qh`** : la surcharge ne change pas la valeur.
- **`ba` n'est pas surchargé** : un `os` porte la table `ba` de la classe `qh`.
- **Tables de défaut de `os`** (`nu`, `px`) : jamais écrites en place ni réaffectées (grep du cart). Aucun défaut muté.

### Emprise réelle de `os` (mesurée sur c868719)

**Méthode.** Grep de `src/`, `tests/`, `tools/`, `Makefile`, lecture des fonctions, puis **sonde de mesure** sur une copie `git archive c868719` jetable du scratchpad (supprimée après mesure) : `pico8_entity_get_field`/`set_field`, `pico8_mv`, `pico8_instance_get_field`, `pico8_instance_fields` deviennent, hors de `pico8_entity.c`/`pico8_class.c`, des macros qui journalisent (opération, nom, fichier:ligne, nature de la valeur, « absent » et « écriture nil » à part) chaque accès à une instance dont la classe exacte est `pico8_os_class()`. Binaire de couverture relancé sur chacun des 15 rejeux du filet, un par un (`--from-title` pour les 4 `RECORDINGS`), puis la sonde sur le rejeu qui construit un `os`.

| Aspect | Mesure |
|---|---|
| Constructions d'un `os` | **une seule** voie : `ie:ldown` (`research_icon.c:199`, `pico8_class_instantiate(ke_cls, {id="s"..N})`), clé **`id`** seule (`id` lu « str » à `slipway_display.c:30`). **1 sur le filet**, dans **`long_session_events`** (`--from-title`, sortie 0 ; `qh 2` sur le même rejeu) — la mesure des PRD précédents est confirmée. **0** dans les 14 autres rejeux (vérifié rejeu par rejeu, tous en sortie 0). `pico8_os_new` : aucun appelant. Ni `lookup_named`, ni `kv[]`, ni `fz`. |
| Défauts propres de `os` (7) | `nu`, `hp`=4, `px`, `qh`=false, `s`=100, `od`=15, `hs`=0.5. Hérités de `qh` (non surchargés) : `me`=`"kz"`, `dq`=`"ly"`, `hf`=false, `fp`=2, `dw`=0.025, `mi`=1, `ba` ; de `jo` : `fa`, `kq`, `ma`, `ka`, `dz`, `cq`, `kw`, `gq`, `ca`, `em` ; de `ke` : `t`. |
| Défauts qui tombent sur un membre hérité | **les 7** : `nu`, `hs` (`pico8_jo_t`) ; `hp`, `px`, `qh`, `s`, `od` (`pico8_qh_t`). |
| Défauts mutés | aucun. |
| Champs écrits sur un `os` (sonde) | 15 noms, tous déjà acceptés par l'écrivain de `qh` : par des fonctions de `jo` : `oe` (`planet.c:401`), `hn` (`:441`), `ep` (nil, `:442`), `jy` (`:451`), `kw` (`:458`), `ma` (`:481`), `lq` (`planet_display.c:120`), `fx` (`planet_trade.c:94`) ; par les écrivains génériques déjà membres de `jo` : `kr` (`economy.c:242`), `cc`, `by` (`economy_tiers.c:92,107`), `m_` (`ui.c:308,313`), `ma` (`entity_geom.c:79`) ; par les méthodes de `qh` qu'il hérite : `ra` (`probe.c:132`), `p` (`:138`), `et` (`:179`, faux ou REF) ; `me`/`t` dans le `pico8_ke_t` (sites déjà aiguillés). Écriture nil : `ep` seulement. Atteignables sans être vus sur le filet, déjà acceptés : `mi` (`economy_tiers.c:218`, amélioration), `fo`, `jh`, `nk`, `p_`. **Aucun nom écrit hors des 20 de `qh`** : aucun membre propre n'est dû. |
| Membres propres qui en résultent | **aucun** : `pico8_os_t` n'a que `pico8_qh_t`. |
| Noms lus sur un `os` (sonde, 98 sites distincts `(op, nom, fichier:ligne)`) | présents : `me`, `t`, `ra`, `p`, `oe`, `jy`, `kw`, `hn`, `kq`, `fa`, `ma`, `s`, `od`, `dw`, `dq`, `fp`, `hp`, `hf` (faux), `qh` (faux, `economy_score.c:39`, `da`), `mi`, `id`, `lq`, `cc`, `by`, `et`, `fx`. Absents (rendent « absent » aujourd'hui comme demain) : `qw` (`planet_display.c:46`, `jo:kj`, 1 330 fois), `fg`, `ql` (`ui.c:54`), `ik` (`probe.c:302`), `mt` (`slipway.c:45`), `fo`, `nk`, `p_`, `jh`, et `by`/`cc`/`fx`/`lq` avant leur première écriture. **Jamais lus sur le filet** (chemins atteignables non exercés, d'où la caractérisation dirigée) : **`hs`** (`db`, `economy_score.c:167`, seulement si un export de l'`os` va vers une planète `hf`), `nl` (`bb`, `economy_tiers.c:28`, l'`os` du rejeu sort plus tôt), `px`, `ba`. |
| Méthodes appelées par nom sur un `os` | `hq` (`ui.c:360`, 147), `ey` (`ui.c:373`), `gv` (`ui.c:411`, `slipway.c:110`), `ldown` (`ui.c:411`), `hd` (`ui.c:296`), `hb` (`economy.c:277`), `gg` (`economy_score.c:95`), `ib` (`economy_tiers.c:104`), `cn` (`planet_trade.c:305`), `kf` (`slipway.c:177-178`), `kj`/`ev` (`dc`), et par `dg` sur `me` (`bootstrap.c:89`) : `kz` (absent, 146 fois) et **`ly`** (1 183 fois → `qh:ly` = `na`). Toutes déjà dans l'enum : **aucune entrée nouvelle**. |
| Lectures à simple forme | **aucune** : zéro `pico8_instance_get_field` sur un `os` ; `pico8_instance_fields` seulement aux sites déjà aiguillés (`ke.c:45-46`, `bootstrap.c:101`, `probe.c:128`). |
| `->defaults` de `os` | **aucun** site : ni installation, ni lecture de `g_os_class->defaults`, ni `lookup_named("os")`. |
| Lectures de classe-valeur | la vue `ie.ke` lit `rr` (absent)/`s`/`me`/`od` sur `gx[1]` par `pico8_class_get_field` : `os` garde sa `pico8_class_t` et sa spec. `cart.c` asserte les liens `parent` : inchangés. `test_pico8_ie` lit `gx[1]` comme classe (`:224`, `:642`). |
| Tests qui lisent un `os` | **`test_pico8_qh`** : `descendant_of` (`:501-520`) ne reconnaît que `nl` en struct → **l'`os` disparaîtrait** du rejeu `long`, dont `required = {OS}` ; `fold_descendant` (`:525-557`) **réécrit `qw`** par `pico8_entity_set_field(d, "qw", had_qw ? qw : nil)` (`:556`) : sur un `os` struct, l'écrivain de `qh` refuse `qw` → **assertion** ; la boucle finale de `test_qh_is_a_struct` (`:813-825`) attend un `os` **instance**. **`test_pico8_jo`** : `daughter_of` (`:422-446`) → l'`os` disparaîtrait du rejeu `long` (`required = {OS}`, `:540`) ; `fold_daughter` ne fait que lire et appeler `gg`/`hd`. **`test_pico8_nl`** : `test_nl_is_a_struct` (`:593`) attend `os` instance parmi les « autres descendantes ». `test_pico8_ie` (comparaisons `->cls` déjà gardées par le tag, `:481,558,564`), `test_pico8_bg`, `test_pico8_qb` (rejouent `long`, ne reconnaissent pas `os`) : inchangés. Aucun test de régression (§6) ne construit un `os`. |
| Filet | 1 rejeu (`long_session_events`) construit un `os` (`os  1  1/15`) : il le dessine (`jo:kj`), le fait osciller (`na`, 1 183 appels de `ly`), le relie (`kf`, `cn`), le compte dans `gg`/`hb`/`ib` pendant ~1 330 frames. |

## Solution

1. **Enum** : inchangée (`LY` existe depuis 1a.18 ; `os` n'a aucune méthode propre).
2. **Struct `os`**, publique dans `planet_family.h` : `pico8_qh_t` seul membre. Aucun membre propre : les 7 défauts de `os` ont tous un membre hérité, et aucun site n'écrit sur un `os` un nom hors des 20 de `qh`.
3. **Défauts pris par REF, jamais re-parsés** : `pico8_planet_family_init` déclare `os` comme aujourd'hui (même spec, même parent, même nom de couverture), pose `pico8_class_set_struct_new(g_os_class, …)`, puis lit **une fois** les 7 défauts propres de `os` dans sa `pico8_class_t` (par `pico8_class_get_field`, assert « chaque défaut est une clé de la spec de `os` ») vers un état statique `g_os_defaults`, chaque entrée rangée à l'offset de son membre : `nu` → `qh.jo.nu`, **`hs` → `qh.jo.hs`** ; `hp`, `px`, `qh`, `s`, `od` → membres de `qh`. `pico8_planet_family_shutdown` remet cet état à nil avant de libérer la classe, sans libérer aucune table.
4. **Constructeur de `os`**, dans l'ordre (celui de 1a.19) :
   1. alloue un `pico8_os_t` ;
   2. `pico8_qh_struct_init(&self->qh, &g_os_struct_class)` (défauts de `jo`, puis de `qh` — `ba` de `qh` compris —, `me` à `"kz"`) ;
   3. copie les 7 défauts statiques de `os` (ils écrasent `nu`, `hs`, `hp`, `px`, `qh`, `s`, `od` hérités) ;
   4. recopie la clé nommée `id` de la table reçue (NULL permise) dans `qh.jo.id` et **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise ; ne garde aucun pointeur sur la table ;
   5. enregistre `"os"` au crochet de couverture ;
   6. appelle `pico8_jo_pq` sur la struct (`oe`, puis `hp`=4 → `lz(gb[4])`) ;
   7. appelle `pico8_ke_struct_pq`.
5. **Descripteur de `os`** (`static const`, distinct de celui de `qh`) :
   - les 15 entrées = **les mêmes pointeurs** que le descripteur de `qh` : `HD`, `KJ`, `LDOWN`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL` (dispatchers de `jo`), `HQ` = `pico8_bv_dispatch`, `EV` = `pico8_dl_dispatch`, `EY` = `pico8_d__dispatch`, **`LY` = `pico8_qh_ly_dispatch`** (aucune surcharge) ; `PH`, `KZ`, `RDOWN`, `MD`, `OL`, `PR`, `EB` NULL ;
   - **lecteur et écrivain** : les fonctions mêmes de `qh` (`pico8_qh_field`, `pico8_qh_set_field`, `static` du même fichier), puisqu'il n'y a aucun membre propre à consulter d'abord. `os` accepte donc exactement les **20 noms** de `qh` ; l'écrivain de `qh` ne change pas ;
   - accesseur public `pico8_os_struct_class()`, comme `pico8_nl_struct_class()`.
6. **Fonctions de `qh` et de `jo` : aucune ne change de corps ni de signature.** `pico8_qh_struct_init` reste `static` (os vit dans le même fichier) ; `pico8_qh_ly_dispatch`, `bv`, `dl`, `d_`, `na` et toutes les fonctions de `jo` restent à double forme ; les 4 installations `->defaults` de `qh`, les 11 de `jo` et son `pq` de classe restent.
7. **`pico8_os_new`** rend `pico8_os_t *`, même paramètre (aucun appelant), et passe sous le bloc de commentaire des `_new` convertis. `ie:ldown`, `gx[]`, la vue `ie.ke`, `cart.c` : inchangés ; la délégation de `pico8_class_instantiate` suffit.
8. **Tests** : nouveau `test_pico8_os` (caractérisation) ; `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl` : reconnaissance de `os` par son descripteur (voir Testing), plus une garde d'écriture dans `test_pico8_qh` (voir Implementation Decisions).
9. **Plan** : à la ligne 1a.20 de `docs/plan-3.md`, ajouter « Découvert à 1a.20 : … » avec au minimum : `os` n'a ni membre ni méthode propre (`pico8_os_t` = `pico8_qh_t`, descripteur aux 15 entrées et au lecteur/écrivain de `qh`) ; défauts `nu`/`hs` rangés dans les membres de `jo`, `hp`/`px`/`qh`/`s`/`od` dans ceux de `qh` ; `hs` lu seulement par `db`, jamais sur le filet ; `test_pico8_qh` ne réécrit `qw` que s'il était présent ; `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl` reconnaissent `os` par son descripteur.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `os` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `hj`, `pm`, `mx`, `nr`, `fr`, `ga` à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `os` mesurée (constructions par rejeu, défauts, champs écrits et par qui, noms lus absents ou jamais lus, méthodes par nom, `->defaults`, tests qui le lisent), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux une struct `os` dont le seul membre est `pico8_qh_t`, afin que la forme dise d'emblée que le processeur n'ajoute rien au laboratoire que des valeurs de défaut.
4. En tant que mainteneur, je veux qu'aucun défaut de `os` n'ait de copie propre, afin qu'un lecteur qui passe par le descripteur de `qh` ou de `jo` voie la valeur du processeur et non celle de la parente.
5. En tant que mainteneur, je veux que `hs`=0.5 écrase le `hs` de `jo` (1) dans le membre de `jo`, afin que `db` rende pour un processeur exportateur la moitié du revenu d'une planète ordinaire, comme dans le cart.
6. En tant que mainteneur, je veux que `qh`=false, `hp`=4, `s`=100, `px` et `nu` de `os` écrasent les membres hérités, afin qu'un processeur n'entre pas dans `cg.qh`, que `da` prenne la branche `gg`, que `jo:pq` coure `lz(gb[4])` et que le sprite et le nom affichés soient ceux du processeur.
7. En tant que mainteneur, je veux que `ba` d'un `os` soit la REF de la table `ba` de `qh`, afin qu'aucune table ne soit dupliquée pour un défaut que `os` ne surcharge pas.
8. En tant que gardien du filet, je veux qu'un processeur construit par `ie:ldown` depuis `gx[1]` ait son `id` `"sN"` et tous ses défauts, afin que le rejeu `long_session` reste identique au pixel près.
9. En tant que mainteneur, je veux qu'une clé de construction autre que `id` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt (§7).
10. En tant que gardien du filet, je veux que `jo:pq` coure sur la struct avant `lt`, afin que `oe`, `hn`, `ep`, `jy`, `kw` existent avant l'enregistrement, comme dans le cart.
11. En tant que gardien du filet, je veux qu'un `os` neuf entre dans `hv`, `cv.kj`, `cv.ev`, `cv.hd`, `cv.hk`, `cv.hb`, `cg.jo`, `cg.np`, `cg.prober` — et pas `cg.qh` — dans le même ordre relatif, afin que dessin, survol, revenus, `gg` et `da` le voient comme avant.
12. En tant que gardien du filet, je veux que `dg` appelle la `ly` de `qh` sur un processeur en état `"ly"`, afin qu'il oscille (`na`) comme un laboratoire.
13. En tant que gardien du filet, je veux que les 15 méthodes d'un `os` soient les mêmes pointeurs que celles d'un `qh`, afin qu'un processeur et un laboratoire exécutent le même code.
14. En tant qu'auteur des étapes 1a.21 à 1a.26, je veux que les installations `->defaults` de `qh` et de `jo`, le `pq` de classe de `jo` et les corps de `bv`, `dl`, `d_`, `na`, `pico8_qh_ly_dispatch` et des fonctions de `jo` restent à double forme, afin que `hj`, `pm`, `mx`, `nr`, `fr`, `ga` trouvent leurs méthodes en remontant `parent`.
15. En tant que mainteneur, je veux que le lecteur et l'écrivain de `os` soient ceux de `qh`, afin que la liste des 20 noms reste la seule source et qu'un nom hors de cette liste (`qw` compris) soit un arrêt.
16. En tant que mainteneur, je veux que la vue `ie.ke` continue de lire `s`/`me`/`od` sur `gx[1]` par la chaîne de classes, afin que l'icône du processeur garde sprite et prix.
17. En tant que gardien de la couverture, je veux que le constructeur enregistre `"os"` une fois par processeur, afin que `class-coverage-net` soit identique ligne à ligne.
18. En tant que vérificateur, je veux une caractérisation de `os` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit jamais (`hs` dans `db`, `bb`, `da`, identité des tables).
19. En tant que gardien des caractérisations, je veux que `test_pico8_qh`, `test_pico8_jo` et `test_pico8_nl` ne changent que leur façon de reconnaître un `os` (et, pour `test_pico8_qh`, ne réécrivent plus un `qw` qui n'existait pas), sans qu'aucune valeur figée ne bouge, afin que 1a.17 à 1a.19 restent des preuves.
20. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
21. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.20, afin que la liste des étapes dise ce que l'étape a réellement touché.
22. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés.
23. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais. J'ai donc appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : module de la famille de planètes seulement (struct `os`, statiques de défauts, constructeur, descripteur, accesseur, `pico8_os_new` typé, commentaires d'en-tête). `pico8_struct.h`, le module de la planète, `probe`, `research_icon`, `cart`, `economy*` : aucun changement.
- **`pico8_os_t` réduit à `pico8_qh_t`** : §5 exige la struct de la parente en premier membre et un membre par champ de la chaîne ; tous les champs de la chaîne de `os` ont déjà leur membre chez `qh`, `jo` ou `ke`. Rejeté : réutiliser `pico8_qh_t` sans nouveau type — le constructeur, `pico8_os_new` et les tests nomment la classe, et 1a.21+ garderont le même motif.
- **Descripteur distinct, lecteur/écrivain partagés** (G) : un descripteur propre est dû (§5, « une table `static const` par classe », et c'est lui qui distingue un processeur d'un laboratoire pour les tests et pour `class-coverage`). Ses champs `field`/`set_field` pointent sur les fonctions de `qh` elles-mêmes : une fonction de chaînage qui ne consulterait aucun membre avant de déléguer n'ajouterait rien. Rejeté : une fonction `pico8_os_field` qui délègue immédiatement — code mort de plus, même comportement.
- **Descripteur `static const` nommant les fonctions** : un initialiseur `static const` ne peut pas relire les entrées d'un autre descripteur ; il nomme donc les mêmes symboles (dispatchers de `jo`, `pico8_bv_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch`, `pico8_qh_ly_dispatch`, `pico8_qh_field`, `pico8_qh_set_field`), et le test vérifie l'égalité des pointeurs avec `pico8_qh_struct_class()`.
- **`hs` dans le membre de `jo`** : même piège qu'à 1a.18 et 1a.19 — un membre propre `hs` laisserait `jo.hs`=1 visible à `db` (qui lit par le descripteur, lequel remonte à `jo`), et le filet ne le verrait pas (`hs` n'y est jamais lu). La caractérisation dirigée est la seule garde.
- **7 défauts copiés depuis la classe à l'init** (G), scalaires compris, comme `jo`, `qh` et `nl`. Rejeté : les écrire en dur (deux sources). `od`=15 est copié bien qu'égal à celui de `qh` : c'est un défaut de la spec de `os`, et la liste suit la spec.
- **`qh`=false est un membre présent** (booléen faux), pas nil : même lecture que l'instance (`da` lit « présent, faux »).
- **Aucune méthode propre, aucune fonction passée à la forme struct seule** : `os` n'en a pas. La règle « Méthodes d'une parente convertie » garde toutes celles de `qh` et de `jo` à double forme jusqu'à leur dernière descendante.
- **`pico8_qh_struct_init` reste `static`** : `os` vit dans le même fichier que `qh` (la décision de 1a.19 reste valable jusqu'à `nr`, 1a.24).
- **`test_pico8_qh` : garde d'écriture sur `qw`** (G). `fold_descendant` remet `qw` après avoir appelé `ly`, en écrivant nil quand il était absent. Sur une instance, cette écriture ajoute une clé nil (TROU-9) que tout lecteur voit comme absente (le `lookup` du test traite nil comme absent ; `jo:kj` lit `qw` par `field_fixed`, qui rend 0 pour une REF comme pour un absent). Sur un `os` struct, l'écrivain de `qh` refuse `qw` et `pico8_entity_set_field` asserte. Décision : n'écrire `qw` que s'il était présent (`had_qw`). Cette garde est posée **sur l'ancien code, avant la conversion** (morceau 1), et `test_pico8_qh` doit y passer avec toutes ses valeurs figées inchangées : c'est la preuve que la garde est neutre. Rejeté : faire accepter `qw` par l'écrivain de `os` — champ qu'aucun site du jeu n'écrit sur un `os`, donc sans règle du §5 (arrêt) ; rejeté : sauter `fold_descendant` pour `os` — change `LONG_DESC_HASH`.
- **Sémantique de nil** : un membre nil est absent. Mesuré, la seule écriture nil sur un `os` frappe `ep`, sans défaut dans la chaîne. Pas d'écart.
- **Clés dynamiques, enum** : aucune nouvelle.
- **TROU** : aucun écart attendu. Si l'implémenteur en trouve un, règle §7 (préserver, `TROU-N`, `/* TROU-N */`).
- **Commentaires** (CLAUDE.md) :
  - à ajouter : contrat de la struct `os` (jamais libérée, TROU-7 ; aucun membre propre, chaque défaut de `os` vit dans le membre de `qh` ou de `jo` du même nom ; `nu`/`px` sont les tables de la classe `os`, `ba` celle de `qh`, empruntées) ; contrat de `pico8_os_new` (comme `pico8_nl_new`) ; assert des clés ;
  - à corriger si rendus faux : le bloc « Each `_new` below … `overrides` is copied, not kept » de `planet_family.h`, qui ne couvre plus `os` ; le commentaire de `pico8_qh_class()` (« its unconverted descendants still inherit ») s'il laisse croire que `os` en hérite encore à l'exécution ; le commentaire de `fold_descendant` s'il décrit la remise de `qw` ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` et `make frames-check` verts. La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Sur l'ancien code** : garde `had_qw` dans `fold_descendant` de `test_pico8_qh` (suite verte, toutes les valeurs figées inchangées, `LONG_DESC_HASH` compris) ; puis caractérisation `test_pico8_os` (voir Testing), valeurs figées. Arrêt ici si le rejeu `long_session` ne construit pas exactement un `os` : mesure fausse, à corriger dans le PRD, pas un test durci.
2. **`os` convertie** : struct, statiques de défauts, constructeur, descripteur, accesseur, `pico8_class_set_struct_new`, `pico8_os_new` typé ; `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl` relus (voir Testing). Les quatre caractérisations passent **sans changer leurs valeurs figées** ; les vérifications de forme s'ajoutent à `test_pico8_os`.
3. **Preuves** : mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.20 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`/`pico8_db`/`pico8_da`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF et des pointeurs de fonction. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_os`** (dans `make unit`, lié comme `test_pico8_nl`, sur le cart réel, dépendance Makefile sur `long_session_events.json` et la fixture de départ du scénario dirigé `empty_planet_cleared_events.json`). Il lit **uniquement** par l'API à double forme et par `pico8_class_get_field(pico8_os_class()/pico8_qh_class()/pico8_jo_class(), …)`, qui existent avant et après ; son code est le même avant et après, aux vérifications de forme près. Chaque scénario tourne dans un fils forké ; le rejeu depuis le titre tourne à côté des scénarios dirigés.
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction dirigée** (partie lancée comme dans `test_pico8_nl`) : `ldown` sur l'icône de `cv.hb` dont `ke` est `gx[1]` → le processeur ; deux processeurs successifs. Empreinte de `id`, `me`, `t`, `qh`, `hp`, `s`, `od`, **`hs`**, `dq`, `fp`, `dw`, `mi`, `hf`, `fa`, `ma`, `ca`, `gq` ; présence des 10 membres de `qh` et des 31 de `jo` ; absence de `rr`, `nl`, `qw` ; présence des 15 méthodes (`hd`, `kj`, `ldown`, `hb`, `gg`, `gv`, `hk`, `kf`, `cn`, `ib`, `ll`, `hq`, `ev`, `ey`, `ly`) et absence de `ph`, `kz`, `rdown`, `md`, `ol`, `pr`, `eb` ; **`ly` lu sur le processeur = `ly` de `pico8_class_get_field(pico8_qh_class(), "ly")`** ; appartenance et rang dans `hv`, `cv.kj`/`ev`/`hd`/`hk`/`hb`, `cg.jo`/`np`/`prober`, **absence de `cg.qh`**. **Identité** : `nu`, `px` = REF de `pico8_class_get_field(pico8_os_class(), …)` (≠ celles de `qh`) ; **`ba` = REF de la classe de `qh`** ; `kq`, `em`, `ka`, `dz`, `cq`, `gq` = REF de la classe de `jo` ; `kw` ≠ REF de la classe de `jo` (`lz` a couru) ; `hn` = `gb[4]`. Figée.
    2. **Comportement dirigé** : `mv(o,"hq",mp,rp)` puis `mv(o,"ey",nil)` avec crédits → `me`, `t`, `ra`, `et`, crédits ; `dg` en `"ly"` → `ra.y` avant/après ; `dg` en `"kz"` → rien ; `ib` (`bb`, branches `pt.nl` absent et `pt.hf` faux) et `gg`/`pico8_da` (branche `qh` faux) sur le processeur posé → résultats ; **`pico8_db` sur un processeur dont `by` contient un échange vers une planète `hf`** (construit comme le jeu le construit, par exemple par une liaison `kf` entre le processeur et une planète nue) → résultat, où `hs`=0.5 entre ; la même mesure sur une planète nue au même échange, pour que l'empreinte distingue 0.5 de 1. Figés.
    3. **Rejeu du filet** `long_session_events` (`--from-title`) : par frame, pour chaque `os` de `hv` : `id`, `me`, `t`, `ra` x/y, `p`, vérité et nature de `et`, `mi`, `p_`, `kw.qe`/`kw.mu`, taille de `oe`/`jy`/`cc`/`by`, `m_`, `hs` ; plus une empreinte de l'écran ; repliés en une empreinte de séquence figée, avec le nombre d'`os` vus (1) et le nombre de frames (27 273 selon `test_pico8_qh`).
  - **Forme** (ajoutée au morceau 2) : le processeur construit par `ie:ldown` est `STRUCT` avec le descripteur de `os`, **distinct** de celui de `qh` ; pour les 15 entrées de méthode, l'entrée du descripteur de `os` est **la même** que celle de `pico8_qh_struct_class()` ; `field`/`set_field` sont ceux de `pico8_qh_struct_class()` ; `pico8_entity_set_field` passe pour les 20 noms de `qh` et asserte (dans un fils) pour `qw`, `rr`, `hs`, `hp`, `id` ; une clé de construction autre que `id`, ou positionnelle, asserte ; `pico8_os_new(NULL)` rend une struct sans `id` mais avec `oe` ; `hj`, `pm`, `mx`, `nr`, `fr` restent `INSTANCE`.
  - **Mutations** (au moins 12, notées dans le commit, mutants équivalents justifiés) : descripteur de `os` = celui de `qh` (processeur initialisé avec `&g_qh_struct_class`) ; `LY`, `EV`, `HQ` ou `EY` absent ; une entrée de `jo` (`LL`, `KJ`, `IB`) absente ; `hs` non appliqué (attrapé par `db` dirigé seulement) ; `hs` rangé dans un membre propre ajouté ; `nu` de `qh` laissé (le processeur entre dans `cg.qh`) ; `qh`=false non appliqué ; `hp` laissé à 1 (`lz(gb[1])`) ; `s` laissé à 174 ; `px` laissé à celui de `qh` ; `ba` copié au lieu de la REF ; `id` non recopié ; `pico8_jo_pq` omis ; couverture non enregistrée (attrapée par `class-coverage-net` : `os 0 0/15`) ; constructeur de struct non posé ; écrivain de `os` qui accepte `qw`. Équivalent attendu : `od` non appliqué (15 = défaut de `qh`) — à noter comme tel, pas à « réparer » par un test.
- **`test_pico8_qh`** (caractérisation de 1a.18) : trois changements, aucune valeur figée touchée — la garde `had_qw` de `fold_descendant` (morceau 1, sur l'ancien code) ; `descendant_of` reconnaît aussi une struct dont le descripteur est celui de `os` (sinon l'`os` disparaîtrait du rejeu `long` et l'exigence `OS` échouerait) ; la boucle finale de `test_qh_is_a_struct` asserte « `os` est une struct au descripteur de `os` » comme pour `nl`, « instance de sa classe » pour les cinq autres. `LONG_DESC_HASH` et toutes les autres empreintes restent identiques.
- **`test_pico8_jo`** (caractérisation de 1a.17) : un changement de lecture — `daughter_of` reconnaît aussi une struct dont le descripteur est celui de `os`. Toutes les empreintes restent identiques.
- **`test_pico8_nl`** (caractérisation de 1a.19) : un changement de forme — `test_nl_is_a_struct` retire `os` de la liste des descendantes attendues en `INSTANCE` (ou l'attend en struct au descripteur de `os`). Aucune valeur figée touchée.
- **Tests existants** : `test_pico8_ie` (comparaisons `->cls` déjà gardées par le tag ; `gx[1]` lu comme classe), `bg`, `qb` (rejouent `long`), `jc`, `nf`, `js`, `mz`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage` : diff vide, valeurs figées identiques. Tous les tests de régression (§6) : diff vide, verts. Aucun ne construit un `os` : aucune preuve rouge n'est due par ce commit.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** : `test_pico8_nl` et le commit c868719 (construction par `ldown` sur `gx[]`, rangement des défauts dans les membres hérités, descripteur qui reprend les pointeurs de `qh`, forme, mutations), `test_pico8_qh` (rejeu `long` depuis le titre), `test_pico8_jo` (identité des REF de `jo`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `os` n'est pas `os  1  1/15`, ou si une autre ligne (`qh  8  3/15`, `nl  1  1/15`, `jo  1710  15/15`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`…) change. Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier : si l'assertion des clés de construction ou celle de l'écrivain se déclenche sur un rejeu ou un test ; si `pico8_os_t` reçoit un membre propre ; si l'écrivain de `os` accepte un nom hors des 20 de `qh` (`qw` compris), ou si la liste des 20 noms de `qh` ou des 17 de `jo` change.
- `FAILED:` si une autre classe que `os` est convertie ou change de forme (`hj`, `pm`, `mx`, `nr`, `fr`, `ga`, `ui`, `lq`, `hy`, `hp`, `pj`, `g_`, `fv`, `trade`…), ou si le comportement de `qh` ou de `nl` change (valeur figée de `test_pico8_qh`/`test_pico8_nl`, lignes `qh`/`nl` de la couverture).
- `FAILED:` si une fonction de `qh` ou de `jo` devient struct seule : une des 4 installations `->defaults` de `qh`, une des 11 de `jo`, ou `pico8_class_set_pq(g_jo_class, …)` retirée ou changée de fonction ; un corps de `pico8_qh_ly_dispatch`, `pico8_qh_field`, `pico8_qh_set_field`, `pico8_na`, `pico8_bv`, `pico8_dl`, `pico8_d_` ou d'une fonction `pico8_jo_*` qui caste `self` en struct `os`/`qh`, lit un membre directement là où il passait par l'API, ou revient à `pico8_instance_get_field`/`pico8_instance_fields` ; une entrée du descripteur de `os` qui n'est pas le même pointeur que l'entrée correspondante du descripteur de `qh`.
- `FAILED:` si une table de défaut de `os` est re-parsée depuis la spec (un second `pico8_ob(` ou `pico8_class_declare` sur un fragment de la spec de `os`), si `nu`/`px` d'un `os` neuf ne sont pas les REF de la classe `os`, ou si `ba` n'est pas la REF de la classe `qh`.
- `FAILED:` si un test de régression (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une valeur figée de `test_pico8_os` a été changée après la conversion, ou une de `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl` (empreintes, comptes, frames), `ie`, `bg`, `qb`, `jc`, `nf`, `js`, `mz`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo` ; si `test_pico8_qh` change au-delà de la garde `had_qw`, de `descendant_of` et de la boucle finale de `test_qh_is_a_struct`, `test_pico8_jo` au-delà de `daughter_of`, ou `test_pico8_nl` au-delà de la liste des descendantes attendues en instance.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- l'enum est celle de 1a.18, inchangée ;
- `pico8_os_t` n'a que `pico8_qh_t` ; le descripteur n'a que les 15 méthodes, toutes égales à celles de `qh`, et le lecteur/écrivain de `qh` ;
- grep : aucun `g_os_class->defaults`, aucun `lookup_named("os")` ; les 4 installations `g_qh_class->defaults` et les 11 de `jo` sont intactes ;
- hors du module de la famille, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_os.c` (nouveau), `test_pico8_qh.c`, `test_pico8_jo.c`, `test_pico8_nl.c` et le `Makefile` (cible et `unit`) bougent ;
- la ligne 1a.20 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.20 ».

## Out of Scope

- Les autres descendantes de `qh` (1a.21 à 1a.25) et `ga` (1a.26) : leurs structs, leurs `->defaults`.
- Le passage à la forme struct seule des fonctions de `qh` et de `jo`, le retrait de leurs installations `->defaults`, le retypage de leurs signatures.
- Rendre `pico8_qh_struct_init`, `pico8_qh_field`, `pico8_qh_set_field` ou `pico8_qh_ly_dispatch` publics : à la première fille de `qh` hors du module (1a.24).
- `ie` et sa vue `ie.ke` (déjà convertis) : aucun changement.
- Le typage C des membres (1b) ; `gb` en données constantes (D) ; archétypes (phase 2 — `os` y deviendra un pur archétype de `qh`).
- Libérer un processeur (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Première fille sans rien de propre** : `os` est la première conversion dont la struct n'ajoute aucun membre et dont le descripteur ne surcharge aucune entrée. Toute sa différence avec un laboratoire tient dans 7 valeurs de défaut, dont une (`hs`) appartient à `jo`. Le motif (descripteur distinct, lecteur/écrivain de la parente) resservira si une autre classe s'avère sans membre propre.
- **Le filet ne garde pas `hs`** : `db` ne lit `pt.hs` que pour un export vers une planète `hf`, et le processeur de `long_session` n'en fait aucun (sonde : 0 lecture de `hs`). Un `hs` resté à 1 passerait `frames-check` ; seule la mesure dirigée de `db` l'attrape.
- **`test_pico8_qh` écrivait un champ que le jeu n'écrit pas** : la remise de `qw` à nil dans `fold_descendant` était neutre sur une instance (clé nil = absente pour tout lecteur) et devient un arrêt sur une struct sans `qw`. La garde est prouvée neutre en passant sur l'ancien code avant la conversion ; elle ne relâche aucune vérification.
- Mesures : grep sur `src/`, `tests/`, `tools/`, `Makefile` à c868719 ; lecture de `pico8/lua.lua:385-388,932-935,1095-1100,1182,1276-1331`, de `pico8_planet_family.c/h`, `pico8_planet.c/h`, `pico8_entity.c/h`, `pico8_economy_score.c`, `pico8_economy_tiers.c`, `pico8_planet_display.c`, de `test_pico8_qh.c`, `test_pico8_jo.c`, `test_pico8_nl.c`, `test_pico8_ie.c`, `test_pico8_bg.c`, `test_pico8_qb.c`, et de `tools/class_coverage_net_expected.txt` ; sonde (98 sites distincts `(opération, nom, fichier:ligne)` sur un `os` exact, 15 noms écrits, écriture nil `ep` seule, 0 `pico8_instance_get_field`, 0 lecture de `hs`) et comptes rejeu par rejeu (`os 1` dans `long_session_events`, `os 0` dans les 14 autres, tous en sortie 0) dans une copie `git archive` jetable du scratchpad, supprimée après mesure.
