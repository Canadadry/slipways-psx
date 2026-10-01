---
title: "Plan 3, étape 1a.19 : `nl` (la plaque commerciale) devient une struct fille de `qh`, sa seule méthode propre `ly` passe à la forme struct seule, les fonctions de `qh` et de `jo` restent à double forme"
description: "La plaque commerciale `nl` est encore une instance de la couche objet, construite une fois sur le filet (`trading_hub_session_events`) par `ie:ldown` depuis `gx[4]`. Elle n'a aucune descendante : elle passe en struct dont le premier membre est `pico8_qh_t`, avec 3 membres propres (`rr`, `nl`, `qw`), 7 défauts rangés dans les membres hérités, un descripteur `static const` qui reprend les 14 entrées de `qh` et remplace `LY` par `nl:ly`, un lecteur et un écrivain chaînés sur ceux de `qh`. L'initialisation d'un `pico8_qh_t` est coupée du constructeur de `qh`. `nl:ly` devient struct seule et son installation `->defaults` migre dans le descripteur ; aucune fonction de `qh` ni de `jo` ne change."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.19 — `nl` : `qh` (`planet_family.c:322`), `gx[4]`.

(À HEAD 7a82ee5, la ligne 322 a glissé avec l'ajout de la struct `qh` : la déclaration `g_nl_class = pico8_class_declare(g_qh_class, …)` est à `planet_family.c:463`, son installation `ly` à `:471`, et `gx[4] = pico8_nl_class()` à `cart.c:227`. La ligne ne nomme aucun test de régression.)

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
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct.

Le §5 dit aussi, pour la couche : `pico8_nl_t` commence par `pico8_qh_t` (premier membre), qui commence par `pico8_jo_t`, qui commence par `pico8_ke_t`.

§6 : aucun test de régression nommé par la ligne ; aucun des 17 fichiers de régression ne construit un `nl` (mesuré, voir plus bas). Les deux caractérisations qui lisent un `nl` (`test_pico8_qh`, `test_pico8_jo`) changent leur façon de le reconnaître, jamais une valeur figée. §7 : un commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (`nl  1  1/15`, toutes les autres lignes inchangées) ; un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.19, avec « découvert à 1a.19 » dans le message ; un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.18 ont posé (jusqu'à 7a82ee5), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`). L'enum vaut `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL`, `LY`. Un nom absent de l'enum, ou dont l'entrée est NULL, est cherché comme champ ; `pico8_mv` rend 0 sur un champ absent. L'écrivain rend 0 pour un nom sans membre, et `pico8_entity_set_field` asserte alors (« a field under no rule of plan-3 section 5 — stop »).
- `pico8_ke_t`, `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq`. `pico8_ke_ks` et `dg` écrivent `me`/`t` dans le `pico8_ke_t` d'une struct.
- **1a.17** : `pico8_jo_t` (31 membres), `pico8_jo_struct_class()`, les 11 dispatchers de `jo` et `pico8_jo_pq` publics, `pico8_jo_struct_init(self, cls)` public (initialise un `pico8_jo_t` alloué avec le descripteur donné, sans `pq` ni `lt`), écrivain de `jo` à 17 noms.
- **1a.18** (le modèle à suivre, `docs/prd/done/plan3-phase1a-etape18-qh-laboratoire.md`, commit 7a82ee5) : `pico8_qh_t` (`pico8_jo_t` puis 10 membres `hp`, `px`, `dq`, `qh`, `od`, `fp`, `dw`, `s`, `ba`, `et`), défauts de `qh` lus une fois par `pico8_class_get_field` dans un état statique `g_qh_defaults` (13 entrées, chacune rangée à l'offset de son membre, `me`/`nu`/`hf`/`mi` dans les membres hérités), descripteur `static const g_qh_struct_class` (11 dispatchers de `jo` + `HQ`=`pico8_bv_dispatch`, `EV`=`pico8_dl_dispatch`, `EY`=`pico8_d__dispatch`, `LY`=`pico8_qh_ly_dispatch`), lecteur chaîné (10 membres puis `pico8_jo_struct_class()->field`), écrivain chaîné à 20 noms (`et`, `ra`, `p`, puis les 17 de `jo`), `pico8_qh_struct_class()` public. **Le constructeur `pico8_qh_struct_new` fait allocation et initialisation d'un seul tenant** (`pico8_jo_struct_init(&self->jo, &g_qh_struct_class)`, 10 membres à nil, 13 défauts copiés) : il n'existe pas encore d'initialisation d'un `pico8_qh_t` séparable.
- Les aides partagées qu'atteint une descendante de `qh` sont déjà à double forme : `pico8_bv`, `pico8_dl`, `pico8_d_`, `pico8_na`, et toutes les fonctions de `jo`.
- Modèles de test : `test_pico8_qh` (caractérisation figée sur l'ancien code, lue par l'API à double forme dans des fils forkés, rejeux depuis le titre en parallèle ; `descendant_of`/`fold_descendant` lisent déjà la première `nl` du rejeu `trading_hub`), `test_pico8_jo` (`daughter_of`/`fold_daughter`, même rejeu).

### Ce que fait le cart

`pico8/lua.lua` :

```
:1057-1066 nl=qh:jr([[ nu=o("jo","np","prober"), rr="trade league",qh=f,nl=1,
             hp=6,s=68,px=o("trading hub",c=13), od=20,qw=15, ba=o(28,48,18,20,21,22), ]])
           function nl:ly(t) na(self,t) if self.mi>=2 then self.qw=8+t%42/6 end end
:546       jo:kj … dv(self.qw) …                      -- lu sur toute planète, absent = 0
:1302      bb : if pt.nl then return min(#jp(pt.jy, gc>0),3) end
:1312-1316 da : pt.qh and #cg.qh or …                 -- qh=f : nl prend la branche gg
:926       ie:hb : oj(self.ke.rr)                      -- rr lu sur la classe, jamais sur l'instance
:1182      eu : gx={qh,os,nr,fr,nl,mx,hj}              -- gx[4] (base 0 côté C) = nl
:932-935   ie:ldown : ie.id+=1 ; return self.ke({id="s"..ie.id})
```

Points décisifs :

- **Une seule construction** : `ie:ldown` sur l'icône qui tient `gx[4]`, clé `id` seule. `nl` n'a pas de `fz`, n'est pas enregistré par nom, n'est pas `kv[hp.cw]`. `pico8_nl_new` n'a aucun appelant (`src/`, `tests/`, `tools/`).
- **Aucune descendante** : rien ne dérive de `nl` (arbre du §2, `grep pico8_class_declare(g_nl_class` vide). Ses fonctions propres peuvent donc passer à la forme struct seule dans ce commit ; celles de `qh` (atteintes par `os`, `hj`, `pm`, `mx`, `nr`, `fr`) et de `jo` (plus `ga`) restent à double forme.
- **`nl:ly`** surcharge `qh.ly` (`na`) : appelle `na` (fonction de `qh`, double forme), puis, si `mi>=2`, écrit `qw`. C'est la seule méthode propre de `nl`, et `qw` le seul champ qu'elle écrit.
- **`nu` de `nl` ne contient pas `"qh"`** : une `nl` n'entre pas dans `cg.qh` (ni `ck`, ni `#cg.qh` de `da`).
- **`hp=6`** : le `pq` de `jo` court `lz(gb[6])` sur chaque `nl` neuve.
- **Tables de défaut de `nl`** (`nu`, `px`, `ba`) : jamais écrites en place ni réaffectées (grep du cart). Aucun défaut muté.

### Emprise réelle de `nl` (mesurée sur 7a82ee5)

**Méthode.** Grep de `src/`, `tests/`, `tools/`, `Makefile`, lecture des fonctions, puis **sonde de mesure** sur une copie `git archive 7a82ee5` jetable (dans le scratchpad, jamais dans le dépôt) : `pico8_entity_get_field`/`set_field`, `pico8_mv`, `pico8_instance_get_field`, `pico8_instance_fields` deviennent, hors de `pico8_entity.c`/`pico8_class.c`, des macros qui journalisent (opération, nom, fichier:ligne, nature de la valeur, « absent » et « écriture nil » à part) chaque accès à une instance dont la classe exacte est `pico8_nl_class()`. Puis le binaire de couverture sur chacun des 15 rejeux du filet, un par un (`--from-title` pour les 4 `RECORDINGS`). Les accès internes à `pico8_entity.c` (`lt`, `he`, `dc`) sont lus dans le code.

| Aspect | Mesure |
|---|---|
| Constructions d'une `nl` | **une seule** voie : `ie:ldown` (`research_icon.c:199`, `pico8_class_instantiate(ke_cls, {id="s"..N})`), clé **`id`** seule. **1 sur le filet**, dans `trading_hub_session_events` (`--from-title`) ; **0** dans les 14 autres rejeux (vérifié rejeu par rejeu). `pico8_nl_new` : aucun appelant. Ni `lookup_named`, ni `kv[]`, ni `fz`. |
| Défauts propres de `nl` (10) | `nu`, `rr`=`"trade league"`, `qh`=false, `nl`=1, `hp`=6, `s`=68, `px`, `od`=20, `qw`=15, `ba`. Hérités de `qh` (non surchargés) : `me`=`"kz"`, `dq`=`"ly"`, `hf`=false, `fp`=2, `dw`=0.025, `mi`=1 ; de `jo` : `hs`, `fa`, `kq`, `ma`, `ka`, `dz`, `cq`, `kw`, `gq`, `ca`, `em` ; de `ke` : `t`. |
| Défauts qui tombent sur un membre hérité (7) | `nu` (`pico8_jo_t`) ; `qh`, `hp`, `s`, `px`, `od`, `ba` (`pico8_qh_t`). Les 3 autres (`rr`, `nl`, `qw`) n'ont de membre ni chez `qh` ni chez `jo`. |
| Défauts mutés | aucun. |
| Champs écrits sur une `nl` (sonde, 111 sites distincts `(op, nom, fichier:ligne)`) | par des fonctions de `jo` : `oe` (`planet.c:401`), `hn`, `ep` (nil), `jy`, `kw` (`lz`, `:441-458`), `ma` (`:481`), `lq` (`planet_display.c:120`), `fx` (`planet_trade.c:94`) ; par les écrivains génériques déjà membres de `jo` : `kr` (`economy.c:242`), `cc`, `by`, `mi` (`economy_tiers.c:92,107,218`), `m_` (`ui.c:308,313`), `ma` (`entity_geom.c:79`) ; par les méthodes de `qh` qu'elle hérite : `ra` (`probe.c:132`), `p` (`:138`), `et` (`:179`) ; par **sa méthode propre `ly`** : **`qw`** (`planet_family.c:220`, 2 537 fois : `mi` atteint 2 sur le filet) ; `me`/`t` dans le `pico8_ke_t` (`ke.c:45-46`, `bootstrap.c:101`, sites déjà aiguillés). Écriture nil mesurée : `ep` seulement (sans défaut dans la chaîne). Atteignables sans être vues, déjà acceptés par l'écrivain de `qh` : `fo` (`bv` sur une cible, `jo:kj`), `jh`, `nk`, `p_`. |
| Membres propres qui en résultent (3) | après `pico8_qh_t` : défauts `rr`, `nl`, `qw` (`qw` étant aussi le seul nom écrit par la méthode propre). |
| Noms lus sur une `nl` | présents : tous les défauts et membres ci-dessus ; `nl` (`economy_tiers.c:28`, pour `bb` `:315`), `qh` faux (`economy_score.c:39`, `da`), `qw` (`planet_display.c:46`, `jo:kj`), `mi` (`planet_family.c:213`, `nl:ly`), `dq` (`probe.c:289`), `fp`, `od`, `dw`, `hp` (`planet.c:403`), `ba` (`planet_trade.c:197`). Absents (rendent « absent » aujourd'hui comme demain) : `fg`, `ql` (`ui.c:54`), `ik` (`probe.c:302`), `mt` (`slipway.c:45`), `fo`, `nk`, `p_`, `jh` (membres nil de `jo`), et `by`/`cc`/`fx`/`lq` avant leur première écriture. `rr` n'est **jamais** lu sur l'instance (seulement sur `gx[4]` par la vue `ie.ke`). |
| Méthodes appelées par nom sur une `nl` | `hq` (`ui.c:360`), `ey` (`ui.c:373`), `gv` (`ui.c:411`, `slipway.c:110`), `hd` (`ui.c:296`), `hk` (`economy.c:156`), `hb` (`economy.c:277`), `gg` (`economy_score.c:95`), `ib` (`economy_tiers.c:104`), `cn` (`planet_trade.c:305`, `planet_family_trade.c:206`), `kf` (`slipway.c:178`), `kj`/`ev` (`dc`), et par `dg` sur `me` (`bootstrap.c:89`) : `kz` (absent, 71 fois) et **`ly`** (3 586 fois → `nl:ly`). Toutes déjà dans l'enum : **aucune entrée nouvelle**. Présentes au `lt` : `kj`, `ev`, `hd`, `hk`, `hb`. |
| Lectures à simple forme | **aucune** : zéro `pico8_instance_get_field` sur une `nl` ; `pico8_instance_fields` seulement aux sites déjà aiguillés (`ke.c:45-46`, `bootstrap.c:101`, `probe.c:128`). |
| `->defaults` de `nl` | une seule installation : `ly` = `pico8_nl_ly_dispatch` (`planet_family.c:471`). Aucune lecture de `pico8_nl_class()->defaults`, aucun `lookup_named("nl")`. |
| Lectures de classe-valeur | la vue `ie.ke` (`research_icon.c:107-110`) lit `rr`/`s`/`me`/`od` sur `gx[4]` par `pico8_class_get_field` : `nl` garde sa `pico8_class_t` et sa spec, rien ne change. `cart.c:84-88` asserte les liens `parent` : inchangés. `test_pico8_ie` lit `gx[4]` comme classe (`:227`). |
| Tests qui lisent une `nl` | `test_pico8_qh` : `descendant_of` (`:501-515`) ne reconnaît une descendante que si elle est `INSTANCE` → **la `nl` disparaîtrait** du rejeu `hub`, dont `required` exige `NL` ; `test_qh_is_a_struct` (`:800-807`) asserte que chaque descendante construite par `pico8_class_instantiate` est une **instance** de sa classe → échouerait pour `nl`. `fold_descendant` réécrit `qw` par `pico8_entity_set_field` (`:547`) : l'écrivain de `nl` doit accepter `qw`. `test_pico8_jo` : `daughter_of` (`:422-440`) ne reconnaît que `qh` en struct et les autres en `INSTANCE` → la `nl` disparaîtrait du rejeu `hub` (`required = {NL}`). `test_pico8_ie` : comparaisons `->cls` déjà gardées par le tag (`:481,558,564`). `test_class_coverage:59` compte `nl` sur une fixture sans `nl` : inchangé. Aucun test de régression (§6) ne construit une `nl`. |
| Filet | 1 rejeu (`trading_hub_session_events`) construit une `nl` (`nl  1  1/15`) : il la dessine (`jo:kj`, avec `dv(self.qw)`), la fait osciller (`na`) et changer de palette (`qw`, `mi>=2`) pendant ~3 600 frames. |

## Solution

1. **Enum** : inchangée (`LY` existe depuis 1a.18).
2. **Préparation dans le module de la famille, sans changement de comportement** : le constructeur de `qh` est coupé en deux, comme celui de `jo` l'a été à 1a.18 :
   - `pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls)` : `pico8_jo_struct_init(&self->jo, cls)`, les 10 membres propres à nil, les 13 défauts statiques de `qh` copiés. Ne fait ni `pq`, ni `lt`, ni couverture, et ne lit aucune clé ;
   - le constructeur de `qh` alloue, appelle `pico8_qh_struct_init(self, &g_qh_struct_class)`, puis fait exactement ce qu'il fait aujourd'hui (clé `id`, couverture `"qh"`, `pico8_jo_pq`, `pico8_ke_struct_pq`) ;
   - **visibilité** (G) : `pico8_qh_struct_init` reste `static` dans le module de la famille, parce que `nl` (et `os`, `hj`, `pm` après elle) vit dans le même fichier. Elle ne devient publique (déclarée dans `planet_family.h`) qu'à la première descendante de `qh` hors de ce fichier (`nr`, 1a.24, `planet_family_trade.c`) — ce sera alors un « découvert à 1a.24 ». Même raisonnement pour `pico8_qh_field`/`pico8_qh_set_field` : `nl` les atteint par `pico8_qh_struct_class()->field`/`->set_field`, comme `qh` atteint ceux de `jo`.
3. **Struct `nl`**, publique dans `planet_family.h` : `pico8_qh_t` en premier membre, puis les 3 membres `pico8_value_t` `rr`, `nl`, `qw`. **Aucun membre ne répète un nom de `qh`, `jo` ou `ke`.**
4. **Défauts pris par REF, jamais re-parsés** : `pico8_planet_family_init` déclare `nl` comme aujourd'hui (même spec, même parent, même nom de couverture), pose `pico8_class_set_struct_new(g_nl_class, …)`, **retire l'installation `ly`** de `g_nl_class->defaults` (elle migre dans le descripteur, règle « Accès directs aux défauts »), puis lit **une fois** les 10 défauts propres de `nl` dans sa `pico8_class_t` (par `pico8_class_get_field`, assert « chaque défaut est une clé de la spec de `nl` ») vers un état statique `g_nl_defaults`, chaque entrée rangée à l'offset de son membre : `nu` → `qh.jo.nu` ; `qh`, `hp`, `s`, `px`, `od`, `ba` → membres de `qh` ; `rr`, `nl`, `qw` → membres propres. `pico8_planet_family_shutdown` remet cet état à nil avant de libérer la classe, sans libérer aucune table.
5. **Constructeur de `nl`**, dans l'ordre (celui de `pico8_class_instantiate` pour une instance `nl`) :
   1. alloue un `pico8_nl_t` ;
   2. `pico8_qh_struct_init(&self->qh, &g_nl_struct_class)` (défauts de `jo`, puis de `qh`, `me` à `"kz"`) ;
   3. met les 3 membres propres à nil, puis copie les 10 défauts statiques de `nl` (ils écrasent `nu`, `qh`, `hp`, `s`, `px`, `od`, `ba` hérités) ;
   4. recopie la clé nommée `id` de la table reçue (NULL permise) dans `qh.jo.id` et **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise ; ne garde aucun pointeur sur la table ;
   5. enregistre `"nl"` au crochet de couverture ;
   6. appelle `pico8_jo_pq` sur la struct (`oe`, puis `hp`=6 → `lz(gb[6])`) ;
   7. appelle `pico8_ke_struct_pq`.
6. **Descripteur de `nl`** (`static const`) :
   - `HD`, `KJ`, `LDOWN`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL` = **les mêmes pointeurs** que les descripteurs de `jo` et de `qh` ; `HQ` = `pico8_bv_dispatch`, `EV` = `pico8_dl_dispatch`, `EY` = `pico8_d__dispatch` = **les mêmes pointeurs** que le descripteur de `qh` et que les installations de `qh->defaults` ; **`LY` = `pico8_nl_ly_dispatch`** (la surcharge remplace l'entrée héritée) ; `PH`, `KZ`, `RDOWN`, `MD`, `OL`, `PR`, `EB` NULL ;
   - **lecteur, chaîné** : les 3 membres propres (nil = absent), sinon `pico8_qh_struct_class()->field` — qui lit les 10 membres de `qh` sur la même adresse (premier membre), puis ceux de `jo`, puis `pico8_ke_field` ;
   - **écrivain, chaîné** : `qw` → membre propre ; tout autre nom → `pico8_qh_struct_class()->set_field` (20 noms, 0 sinon). `nl` accepte donc exactement **21 noms**. `rr` et `nl` ne sont écrits par aucun site : refusés. L'écrivain de `qh` **ne change pas** : sur un `qh`, `qw` reste refusé ;
   - accesseur public `pico8_nl_struct_class()`, comme `pico8_qh_struct_class()`.
7. **`nl:ly` passe à la forme struct seule** (aucune descendante ne l'atteint) : `pico8_nl_ly(pico8_nl_t *self, pico8_fixed_t t)` appelle `pico8_na((pico8_instance_t *)self, t)` (`na` est à `qh` : double forme, inchangée), lit `self->qh.jo.mi` directement — l'assert porteur « `nl:ly` lit `self.mi` sans garde, contrairement à `hj:kj` » est conservé, sur la nature du membre —, et écrit `self->qw` directement. `pico8_nl_ly_dispatch` caste en `pico8_nl_t *`. C'est le traitement des feuilles déjà converties (`jc`, `ie`, `js`).
8. **Fonctions de `qh` et de `jo` : aucune ne change de corps ni de signature.** `pico8_qh_ly_dispatch`, `bv`, `dl`, `d_`, `na` et toutes les fonctions de `jo` restent à double forme ; les 4 installations `->defaults` de `qh` (pour `os`, `hj`, `pm`, `mx`, `nr`, `fr`), les 11 de `jo` et son `pq` de classe restent.
9. **`pico8_nl_new`** rend `pico8_nl_t *`, même paramètre (aucun appelant). `ie:ldown`, `gx[]`, la vue `ie.ke`, `cart.c` : inchangés ; la délégation de `pico8_class_instantiate` suffit.
10. **Tests** : nouveau `test_pico8_nl` (caractérisation) ; `test_pico8_qh` et `test_pico8_jo` : lecture seulement (voir Testing).
11. **Plan** : à la ligne 1a.19 de `docs/plan-3.md`, ajouter « Découvert à 1a.19 : … » avec au minimum : l'initialisation d'un `pico8_qh_t` (`pico8_qh_struct_init`) est coupée du constructeur de `qh`, statique tant que les descendantes sont dans le même fichier ; `qw` (écrit par `nl:ly`, lu par `jo:kj`) → membre de `nl` ; défauts `nu`/`qh`/`hp`/`s`/`px`/`od`/`ba` rangés dans les membres hérités ; l'installation `ly` de `nl->defaults` migre dans le descripteur, `nl:ly` struct seule ; `test_pico8_qh` et `test_pico8_jo` reconnaissent `nl` par son descripteur.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `nl` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga` à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `nl` mesurée (constructions par rejeu, défauts, champs écrits et par qui, noms lus absents, méthodes par nom, `->defaults`, sites qui la reçoivent, tests qui la lisent), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux une struct `nl` dont le premier membre est `pico8_qh_t` et qui n'ajoute que `rr`, `nl`, `qw`, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun membre de `nl` ne répète un nom de `qh`, `jo` ou `ke`, afin qu'un écrivain générique (`bb` sur `mi`) et un lecteur (`da` sur `qh`) ne voient jamais deux copies d'un même champ.
5. En tant que mainteneur, je veux que les défauts `qh`=false, `hp`=6, `s`=68, `od`=20, `px`, `ba` et `nu` de `nl` écrasent les membres hérités, afin qu'une `nl` n'entre pas dans `cg.qh`, que `da` prenne la branche `gg`, et que `jo:pq` coure `lz(gb[6])`.
6. En tant que gardien du filet, je veux qu'une plaque construite par `ie:ldown` depuis `gx[4]` ait son `id` `"sN"` et tous ses défauts, afin que le rejeu `trading_hub` reste identique au pixel près.
7. En tant que mainteneur, je veux qu'une clé de construction autre que `id` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt (§7).
8. En tant que gardien du filet, je veux que `jo:pq` coure sur la struct avant `lt`, afin que `oe`, `hn`, `ep`, `jy`, `kw` existent avant l'enregistrement, comme dans le cart.
9. En tant que gardien du filet, je veux qu'une `nl` neuve entre dans `hv`, `cv.kj`, `cv.ev`, `cv.hd`, `cv.hk`, `cv.hb`, `cg.jo`, `cg.np`, `cg.prober` — et pas `cg.qh` — dans le même ordre relatif, afin que dessin, prix affiché, survol, revenus, `ck` et `da` la voient comme avant.
10. En tant que gardien du filet, je veux que `dg` appelle `nl:ly` sur une plaque en état `"ly"`, afin qu'elle oscille (`na`) et change de palette quand `mi>=2`.
11. En tant que gardien du filet, je veux que les 14 méthodes héritées soient trouvées sur une struct `nl` par les mêmes pointeurs que sur une struct `qh`, afin qu'une plaque et un laboratoire exécutent le même code.
12. En tant qu'auteur des étapes 1a.20 à 1a.26, je veux que les installations `->defaults` de `qh` et de `jo`, le `pq` de classe de `jo` et les corps de `bv`, `dl`, `d_`, `na`, `pico8_qh_ly_dispatch` et des fonctions de `jo` restent à double forme, afin que `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga` trouvent leurs méthodes en remontant `parent`.
13. En tant que mainteneur, je veux que `nl:ly`, que seule une `nl` atteint, lise et écrive ses membres directement, afin que la double forme ne survive pas là où rien ne l'exige.
14. En tant que mainteneur, je veux que l'installation `ly` de `nl->defaults` disparaisse au profit du descripteur, afin qu'il n'y ait qu'une source pour la méthode de `nl` (règle « Accès directs aux défauts »).
15. En tant que mainteneur, je veux que `nu`, `px`, `ba` d'une `nl` soient la même REF que celles de `nl->defaults`, afin qu'aucune table ne soit re-parsée ni dupliquée.
16. En tant que mainteneur, je veux que la vue `ie.ke` continue de lire `rr`/`s`/`me`/`od` sur `gx[4]` par la chaîne de classes, afin que l'icône de la plaque garde nom de techno, sprite et prix.
17. En tant que mainteneur, je veux que l'écrivain de `nl` accepte `qw` et délègue tout le reste à celui de `qh`, afin que la liste des 20 noms de `qh` et des 17 de `jo` reste la seule source et qu'un nom hors des 21 soit un arrêt.
18. En tant que mainteneur, je veux que le lecteur de `nl` délègue à celui de `qh`, afin qu'une fonction de `qh` ou de `jo` lise une plaque sans savoir que c'est une plaque.
19. En tant qu'auteur des étapes suivantes, je veux l'initialisation d'un `pico8_qh_t` séparée de son allocation, afin que chaque fille de `qh` construise sa partie `qh` sans re-parser ni dupliquer ses 13 défauts.
20. En tant que gardien de la couverture, je veux que le constructeur enregistre `"nl"` une fois par plaque, afin que `class-coverage-net` soit identique ligne à ligne.
21. En tant que vérificateur, je veux une caractérisation de `nl` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`bb`, `da`, identité des tables).
22. En tant que gardien des caractérisations, je veux que `test_pico8_qh` et `test_pico8_jo` ne changent que leur façon de reconnaître une `nl`, sans qu'aucune valeur figée ne bouge, afin que 1a.17 et 1a.18 restent des preuves.
23. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
24. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.19, afin que la liste des étapes dise ce que l'étape a réellement touché.
25. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés.
26. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais. J'ai donc appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : module de la famille de planètes seulement (initialisation de `qh` séparée, struct `nl`, statiques, constructeur, descripteur, lecteur, écrivain, accesseur, `pico8_nl_ly` retypée, `pico8_nl_new` typé, installation `ly` retirée). `pico8_struct.h`, le module de la planète, `probe`, `research_icon`, `cart` : aucun changement.
- **Initialisation de `qh` séparée, statique** (G) : la même coupe que `pico8_jo_struct_init` à 1a.18, pour la même raison (construire la partie `qh` d'une fille sans re-parser ni dupliquer ses défauts). Rejeté : l'exporter dès maintenant — aucun appelant hors du fichier avant 1a.24 ; l'API publique ne grandit que quand un appelant l'exige. Rejeté : appeler le constructeur de `qh` puis agrandir — il compte `"qh"` à la couverture (`qh 8` casserait) et enregistre au `lt` avant que ce soit une `nl`.
- **Défauts de `nl` rangés dans les membres hérités quand le nom existe** (`nu`, `qh`, `hp`, `s`, `px`, `od`, `ba`) : même piège qu'à 1a.18 — un `nl.qh` propre laisserait `qh.qh`=1 visible à tout lecteur qui passe par le descripteur de `qh`, et la plaque compterait comme laboratoire dans `da`.
- **`qw` membre de `nl`, pas de `jo`** : écrit par la seule méthode propre de `nl` (règle « seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille ») ; `jo:kj` le lit sur toute planète, où il est absent (lu 0) — même résultat qu'aujourd'hui.
- **`rr` et `nl` membres propres, jamais écrits** : ce sont des défauts de la chaîne (§5, « chaque champ de sa chaîne […] devient un membre »). `rr` n'est lu que sur la classe (vue `ie.ke`), mais le membre est dû.
- **`nl:ly` struct seule, installation retirée** (G) : `nl` n'a aucune descendante ; la règle « Méthodes d'une parente convertie » ne la retient pas, et la règle « Accès directs aux défauts » fait migrer l'installation dans la table de méthodes. `na` qu'elle appelle reste à double forme (fonction de `qh`). Rejeté : garder `nl:ly` à double forme — sans objet, et contraire au traitement des feuilles déjà converties.
- **Lecteur et écrivain chaînés par le descripteur public de `qh`** (`pico8_qh_struct_class()->field`/`->set_field`), sur le modèle de 1a.18 : la liste des 20 noms reste unique.
- **Descripteur `static const` nommant les fonctions** : un initialiseur `static const` ne peut pas relire les entrées d'un autre descripteur ; il nomme donc les mêmes symboles (`pico8_jo_*_dispatch`, `pico8_bv_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch`), et le test vérifie l'égalité des pointeurs avec le descripteur de `qh`.
- **10 défauts copiés depuis la classe à l'init** (G), scalaires compris, comme `jo` et `qh`. Rejeté : les écrire en dur (deux sources).
- **`qh`=false est un membre présent** (booléen faux), pas nil : même lecture que l'instance (`da` lit « présent, faux »).
- **Sémantique de nil** : un membre nil est absent. Mesuré, la seule écriture nil sur une `nl` frappe `ep`, sans défaut dans la chaîne. Pas d'écart. (TROU-9, clé nommée écrite à nil qui reste présente dans une table, ne concerne aucun champ d'une `nl`.)
- **Clés dynamiques, enum** : aucune nouvelle.
- **TROU** : aucun écart attendu. Si l'implémenteur en trouve un, règle §7 (préserver, `TROU-N`, `/* TROU-N */`).
- **Commentaires** (CLAUDE.md) :
  - à ajouter : contrat de la struct `nl` (jamais libérée, TROU-7 ; `nu`/`px`/`ba` sont les tables de la classe, empruntées ; un défaut de `nl` dont le nom est un membre de `qh`/`jo` va dans ce membre) ; contrat de `pico8_qh_struct_init` (ne fait ni `pq` ni `lt` ni couverture) ; contrat de l'écrivain (`qw`, puis les 20 de `qh`) ; assert des clés ;
  - à corriger si rendus faux : le commentaire des `_new` de `planet_family.h` (« yields a class-less instance… ») pour `nl`, le commentaire de `pico8_qh_class()` (« its descendants still inherit ») s'il laisse croire que `nl` en hérite encore à l'exécution ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` et `make frames-check` verts. La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_nl` sur l'ancien code** (voir Testing), valeurs figées. Arrêt ici si le rejeu `trading_hub` ne construit pas exactement une `nl` : mesure fausse, à corriger dans le PRD, pas un test durci.
2. **Préparation sans conversion** : `pico8_qh_struct_init` séparée du constructeur de `qh`. Suite verte, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl` inchangés.
3. **`nl` convertie** : struct, statiques de défauts, constructeur, descripteur, lecteur, écrivain, accesseur, `pico8_class_set_struct_new`, installation `ly` retirée, `pico8_nl_ly` retypée, `pico8_nl_new` typé ; `test_pico8_qh` et `test_pico8_jo` relus (voir Testing). Les trois caractérisations passent **sans changer leurs valeurs figées** ; les vérifications de forme s'ajoutent à `test_pico8_nl`.
4. **Preuves** : mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.19 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF et des pointeurs de fonction. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_nl`** (dans `make unit`, lié comme `test_pico8_qh`, sur le cart réel, dépendance Makefile sur `trading_hub_session_events.json` et la fixture de départ du scénario dirigé). Il lit **uniquement** par l'API à double forme et par `pico8_class_get_field(pico8_nl_class()/pico8_qh_class()/pico8_jo_class(), …)`, qui existent avant et après ; son code est le même avant et après, aux vérifications de forme près. Chaque scénario tourne dans un fils forké ; le rejeu depuis le titre tourne à côté des scénarios dirigés.
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction dirigée** (partie lancée comme dans `test_pico8_qh`) : `ldown` sur l'icône de `cv.hb` dont `ke` est `gx[4]` → la plaque ; deux plaques successives. Empreinte de `id`, `me`, `t`, `rr`, `nl`, `qh`, `hp`, `s`, `od`, `qw`, `dq`, `fp`, `dw`, `mi`, `hf`, `hs`, `fa`, `ma`, `ca`, `gq` ; présence de chacun des 3 membres propres, des 10 de `qh` et des 31 de `jo` ; présence des 15 méthodes (`hd`, `kj`, `ldown`, `hb`, `gg`, `gv`, `hk`, `kf`, `cn`, `ib`, `ll`, `hq`, `ev`, `ey`, `ly`) et absence de `ph`, `kz`, `rdown`, `md`, `ol`, `pr`, `eb` ; `ly` lu sur la plaque ≠ `ly` de `pico8_class_get_field(pico8_qh_class(), "ly")` ; appartenance et rang dans `hv`, `cv.kj`/`ev`/`hd`/`hk`/`hb`, `cg.jo`/`np`/`prober`, **absence de `cg.qh`**. **Identité** : `nu`, `px`, `ba` = REF de `pico8_class_get_field(pico8_nl_class(), …)` (≠ celles de `qh`) ; `kq`, `em`, `ka`, `dz`, `cq`, `gq` = REF de la classe de `jo` ; `kw` ≠ REF de la classe de `jo` (`lz` a couru) ; `hn` = `gb[6]`. Figée.
    2. **Comportement dirigé** : `mv(n,"hq",mp,rp)` puis `mv(n,"ey",nil)` avec crédits → `me`, `t`, `ra`, `et`, crédits ; `dg` en `"ly"` avec `mi`=1 → `ra.y` avant/après, `qw` inchangé (15) ; `mi` posé à 2 puis 3 par `pico8_entity_set_field` (écrivain générique) et `dg` à deux `t` différents → `qw` = `8+t%42/6` ; `dg` en `"kz"` → rien ; `ib` (`bb`, branche `pt.nl`) et `gg`/`da` (branche `qh` faux) sur la plaque posée → résultats. Figés.
    3. **Rejeu du filet** `trading_hub_session_events` (`--from-title`) : par frame, pour chaque `nl` de `hv` : `id`, `me`, `t`, `ra` x/y, `p`, vérité et nature de `et`, `mi`, `qw`, `p_`, `kw.qe`/`kw.mu`, taille de `oe`/`jy`/`cc`/`by`, `m_` ; plus une empreinte de l'écran ; repliés en une empreinte de séquence figée, avec le nombre de `nl` vues (1) et le nombre de frames.
  - **Forme** (ajoutée au morceau 3) : la plaque construite par `ie:ldown` est `STRUCT` avec le descripteur de `nl` ; pour les 14 entrées héritées, l'entrée du descripteur de `nl` est **la même** que celle de `pico8_qh_struct_class()` ; `LY` est `nl:ly` et diffère de celle de `qh` ; `pico8_entity_set_field` passe pour les 21 noms et asserte (dans un fils) pour `rr`, `nl`, `hp`, `id` ; `pico8_entity_set_field(qh_nu, "qw", …)` asserte toujours sur un `qh` nu ; une clé de construction autre que `id`, ou positionnelle, asserte ; `pico8_nl_new(NULL)` rend une struct sans `id` mais avec `oe` ; après le retrait de l'installation, `pico8_class_get_field(pico8_nl_class(), "ly")` remonte la chaîne et rend la `ly` de `qh` (vérifié par le test, pas supposé) : la classe-valeur de `nl` ne porte plus sa méthode, seul le descripteur la porte ; `os`, `hj`, `pm`, `mx`, `nr`, `fr` restent `INSTANCE`.
  - **Mutations** (au moins 12, notées dans le commit, mutants équivalents justifiés) : `LY` du descripteur de `nl` absent, ou égal à celui de `qh` ; `EV`, `HQ` ou `EY` absent ; une entrée de `jo` (`LL`, `KJ`, `IB`) absente ; `nu` de `qh` laissé (la plaque entre dans `cg.qh`) ; `qh`=false non appliqué ; `hp` laissé à 1 (`lz(gb[1])`) ; `qw` dupliqué ou absent ; `mi` dupliqué en membre propre ; `ba` copié au lieu de la REF ; `id` non recopié ; `pico8_jo_pq` omis ; couverture non enregistrée (attrapée par `class-coverage-net`) ; écrivain qui refuse `qw` ou ne délègue pas à `qh` ; lecteur qui ne délègue pas à `qh` ; `nl:ly` qui teste `mi>=1` ; plaque initialisée avec le descripteur de `qh`.
- **`test_pico8_qh`** (caractérisation de 1a.18) : deux changements de lecture, aucune valeur figée touchée — `descendant_of` reconnaît aussi une struct dont le descripteur est celui de `nl` (sinon la plaque disparaîtrait du rejeu `hub` et l'exigence `NL` échouerait) ; la boucle finale de `test_qh_is_a_struct` asserte « `nl` est une struct au descripteur de `nl` » pour `nl` et « instance de sa classe » pour les six autres. `fold_descendant` reste tel quel (l'écrivain de `nl` accepte `qw` ; l'identité `nu`/`px`/`ba` avec `qh` reste fausse pour `nl`). `HUB_DESC_HASH` et toutes les autres empreintes restent identiques.
- **`test_pico8_jo`** (caractérisation de 1a.17) : un changement de lecture — `daughter_of` reconnaît aussi une struct dont le descripteur est celui de `nl` (sinon `required = {NL}` du rejeu `hub` échouerait). `HUB_HASH` et toutes les autres empreintes restent identiques.
- **Tests existants** : `test_pico8_ie` (comparaisons `->cls` déjà gardées par le tag ; `gx[4]` lu comme classe), `jc`, `nf` (rejouent `trading_hub`), `js`, `mz`, `bg`, `bg_kj`, `fq`, `qb`, `iv`, `mh`, `je`, `mo`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage` : diff vide, valeurs figées identiques. Tous les tests de régression (§6) : diff vide, verts. Aucun ne construit une `nl` : aucune preuve rouge n'est due par ce commit.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** : `test_pico8_qh` (construction par `ldown` sur `gx[]`, cycle de vie dirigé, rejeux depuis le titre, forme, mutations), `test_pico8_jo` (galaxie, identité des REF), 1a.18 (coupe de l'initialisation de la parente, descripteur qui reprend les pointeurs de la parente).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `nl` n'est pas `nl  1  1/15`, ou si une autre ligne (`qh  8  3/15`, `jo  1710  15/15`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`…) change. Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier : si l'assertion des clés de construction ou celle de l'écrivain se déclenche sur un rejeu ou un test ; si un membre propre hors de `rr`, `nl`, `qw` est ajouté ; si un membre propre répète un nom de `qh`, `jo` ou `ke` ; si l'écrivain de `nl` accepte un nom hors des 21, ou si la liste des 20 noms de `qh` ou des 17 de `jo` change.
- `FAILED:` si une autre classe que `nl` est convertie ou change de forme (`os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`, `ui`, `lq`, `hy`, `hp`, `pj`, `g_`, `fv`, `trade`…), ou si le comportement de `qh` change (valeur figée de `test_pico8_qh`, ligne `qh` de la couverture).
- `FAILED:` si une fonction de `qh` ou de `jo` atteignable par une descendante non convertie devient struct seule : une des 4 installations `->defaults` de `qh`, une des 11 de `jo`, ou `pico8_class_set_pq(g_jo_class, …)` retirée ou changée de fonction ; un corps de `pico8_qh_ly_dispatch`, `pico8_na`, `pico8_bv`, `pico8_dl`, `pico8_d_` ou d'une fonction `pico8_jo_*` qui caste `self` en struct, lit un membre directement, ou revient à `pico8_instance_get_field`/`pico8_instance_fields` ; une entrée héritée du descripteur de `nl` qui n'est pas le même pointeur que l'entrée correspondante du descripteur de `qh`.
- `FAILED:` si une table de défaut de `nl` est re-parsée depuis la spec (un second `pico8_ob(` ou `pico8_class_declare` sur un fragment de la spec de `nl`), ou si `nu`/`px`/`ba` d'une `nl` neuve ne sont pas les REF de `nl->defaults` ; ou si un `g_nl_class->defaults` subsiste dans `src/`.
- `FAILED:` si un test de régression (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une valeur figée de `test_pico8_nl` a été changée après la conversion, ou une de `test_pico8_qh`, `test_pico8_jo` (empreintes, comptes, frames), `ie`, `jc`, `nf`, `js`, `mz`, `bg`, `bg_kj`, `fq`, `qb`, `iv`, `mh`, `je`, `mo` ; si `test_pico8_qh` change au-delà de `descendant_of` et de la boucle finale de `test_qh_is_a_struct`, ou `test_pico8_jo` au-delà de `daughter_of`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- l'enum est celle de 1a.18, inchangée ;
- la struct `nl` n'a que `pico8_qh_t` et les 3 membres ; le descripteur n'a que les 15 méthodes ; l'écrivain n'accepte que les 21 noms ;
- grep : aucun `g_nl_class->defaults`, aucun `lookup_named("nl")` ; les 4 installations `g_qh_class->defaults` et les 11 de `jo` sont intactes ;
- hors du module de la famille, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_nl.c` (nouveau), `test_pico8_qh.c`, `test_pico8_jo.c` et le `Makefile` (cible et `unit`) bougent ;
- la ligne 1a.19 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.19 ».

## Out of Scope

- Les autres descendantes de `qh` (1a.20 à 1a.25) et `ga` (1a.26) : leurs structs, leurs `->defaults`.
- Le passage à la forme struct seule des fonctions de `qh` et de `jo`, le retrait de leurs installations `->defaults`, le retypage de leurs signatures.
- Rendre `pico8_qh_struct_init`, `pico8_qh_field`, `pico8_qh_set_field` ou `pico8_qh_ly_dispatch` publics : à la première fille de `qh` hors du module (1a.24).
- `ie` et sa vue `ie.ke` (déjà convertis) : aucun changement.
- Le typage C des membres (1b) ; `gb` en données constantes (D) ; archétypes (phase 2).
- Libérer une plaque (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Première feuille d'une parente elle-même fille** : `nl` est la première struct à trois étages sous `ke` (`ke` → `jo` → `qh` → `nl`) et la première fille de `qh` ; la coupe de `pico8_qh_struct_init`, le chaînage par `pico8_qh_struct_class()` et le rangement des défauts dans les membres de deux ancêtres servent de modèle à `os` (1a.20) et `hj` (1a.21).
- **Le filet sert de piège, mais sur un seul rejeu** : `trading_hub` fait traverser à la plaque ~3 600 frames de dessin, 3 586 appels de `ly` et 2 537 écritures de `qw`. Un `LY` manquant (plaque figée, palette fixe) ou un `nu` hérité de `qh` (plaque comptée comme laboratoire) casse `frames-check`. `bb`/`da` sur la plaque et l'identité des tables n'y sont pas toutes visibles : d'où la caractérisation dirigée.
- **`test_pico8_qh` et `test_pico8_jo` doivent être relus** parce qu'ils reconnaissent les descendantes par `->cls` sur une instance : sans relecture, la plaque disparaît de leur rejeu `hub` et leur exigence `NL` échoue. Relire n'est pas refiger.
- Mesures : grep sur `src/`, `tests/`, `tools/`, `Makefile` à 7a82ee5 ; lecture de `pico8/lua.lua:546,926,932-935,1057-1066,1182,1295-1316`, de `pico8_planet_family.c/h`, `pico8_planet.c/h`, `pico8_class.c`, `pico8_entity.c/h`, `pico8_struct.h`, `pico8_research_icon.c`, `pico8_economy_tiers.c`, `pico8_planet_display.c`, de `test_pico8_qh.c`, `test_pico8_jo.c`, `test_pico8_ie.c`, `test_class_coverage.c`, et de `tools/class_coverage_net_expected.txt` ; sonde (111 sites distincts `(opération, nom, fichier:ligne)` sur une `nl` exacte, écritures nil séparées, 0 `pico8_instance_get_field`) et comptes rejeu par rejeu (`nl 1` dans `trading_hub_session_events`, `nl 0` dans les 14 autres, tous en sortie 0) dans une copie `git archive` jetable du scratchpad.
