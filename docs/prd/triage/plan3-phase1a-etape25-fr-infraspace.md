---
title: "Plan 3, étape 1a.25 : `fr` (la porte d'infraspace) devient une struct fille de `nr` avec 1 membre propre (`mt`) ; dernière descendante de `nr` et de `qh`, elle fait passer `nr:gv`/`ldown`/`hb`/`cn` à la forme struct seule et retire les installations de méthodes de `nr` et de `qh`"
description: "La porte d'infraspace `fr` est la dernière planète de la famille `qh` encore instance de la couche objet, construite 5 fois sur le filet (toutes dans `session_events`) par la seule voie `ie:ldown` sur `gx[3]`, avec la clé `id`. Elle passe en struct `pico8_fr_t` (`pico8_nr_t` puis `mt`), son descripteur reprend les 15 entrées de `nr` (mêmes pointeurs). Mesuré : `fr` est la seule instance qui atteint encore `nr->defaults` (`gv`/`ldown`/`hb`/`cn`) et `qh->defaults` (`hq`/`ly`/`ev`/`ey`) ; après elle, les quatre méthodes de `nr` passent à la forme struct seule et ces huit installations sont retirées."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.25 — `fr` : `nr` (`planet_family_trade.c:326`), `gx[3]`.

(À HEAD adc9622, la déclaration `g_fr_class = pico8_class_declare(g_nr_class, …)` est à `planet_family_trade.c:539`, son nom de couverture à `:545`, `pico8_fr_class` à `:599`, `pico8_fr_new` à `:603` (déclarés `planet_family_trade.h:52-56`). La construction est `ie:ldown` → `pico8_class_instantiate(gx[3], {id})`, `research_icon.c:199`, avec `gx[3] = pico8_fr_class()` posé à `cart.c:226`. Côté `nr` : `pico8_nr_gv` `:52` (dispatcher `:74`), `pico8_nr_ldown` `:84` (`:118`), `pico8_nr_hb` `:125` (`:150`), `pico8_nr_cn` `:158` (`:217`), `g_nr_defaults` `:231`, lecteur `pico8_nr_field` `:249`, écrivain `pico8_nr_set_field` `:268`, descripteur `g_nr_struct_class` `:276`, constructeur `pico8_nr_struct_new` `:298`, les quatre installations dans `g_nr_class->defaults` `:528-531`. Côté `qh` : les quatre installations `hq`/`ly`/`ev`/`ey` dans `g_qh_class->defaults`, `planet_family.c:852-855`. La ligne ne nomme aucun test de régression.)

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. […]
>
> **Champ d'un écrivain générique** : […] seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […] jusqu'à la dernière étape de 1a. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct […].
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre (dispatcher hérité dans `defaults` […]) reste à double forme […]. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante.
>
> **Accès directs aux défauts** : […] la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct : une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec.

`pico8_fr_t` commence par `pico8_nr_t`, qui commence par `pico8_qh_t`, `pico8_jo_t`, `pico8_ke_t`. `fr` est une **feuille** et la **dernière descendante non convertie de `nr` et de `qh`** : c'est la situation de `mx` à 1a.23 (fermeture de `hj`), doublée — deux parentes se ferment dans le même commit.

Notes « découvert à » du plan qui échoient ici :

- 1a.24 : « `pico8_nr_gv`/`ldown`/`hb`/`cn` et leurs quatre installations dans `nr->defaults` gardées à double forme pour `fr` (1a.25), dernière descendante de `nr` » ; « `mt` lu sur un `nr` (`slipway.c:45`) absent aujourd'hui comme demain » (`mt` est le membre propre de `fr`) ; « `hz` et `rr` … membres de `nr` » (les défauts `hz`, `rr` de `fr` y tombent).
- 1a.23 : « `pico8_qh_ly_dispatch` aussi (nr/fr l'atteignent par `qh->defaults`) ».

§6 : aucun test de régression nommé ; aucun fichier de régression ne construit un `fr` (mesuré à 1a.24 : les seuls lecteurs de `fr` sont les caractérisations). §7 : un commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (`fr  5  1/15`, `nr  6  2/15`, `qh  8  3/15`, toutes les autres lignes inchangées) ; **preuve des tests par `gcov` et 2 à 3 mutations** ; un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.25, avec « découvert à 1a.25 » dans le message ; un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.24 ont posé (jusqu'à 243426b), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`). L'enum contient déjà les 15 méthodes de `nr` ; `pico8_entity_get_field` sur une struct rend d'abord l'entrée du descripteur quand le nom est une méthode de l'enum, sinon le lecteur ; un membre nil est absent ; l'écrivain rend 0 pour un nom sans membre et `pico8_entity_set_field` asserte alors.
- `pico8_nr_t` (`pico8_qh_t`, `hz`, `rr`), `g_nr_defaults` (8), lecteur `pico8_nr_field` (`hz`, `rr`, puis `qh`), écrivain `pico8_nr_set_field` (**21 noms** : `s`, puis les 20 de `qh`), descripteur `g_nr_struct_class` (15 entrées : celles de `qh` sauf `GV`/`LDOWN`/`HB`/`CN` = les dispatchers de `nr`), tous `static` dans le même fichier que `fr` ; `pico8_nr_struct_class()`, `pico8_nr_new` → `pico8_nr_t *` ; `pico8_qh_struct_init` publique.
- **1a.22** (`pm`) : précédent d'une fille dont l'écrivain est **le même pointeur** que celui de sa parente (aucun nom propre écrit) et d'une initialisation de parente coupée du constructeur de la parente (`pico8_hj_struct_init`, `static` tant que la fille vit dans le même fichier).
- **1a.23** (`mx`, af03e84) : précédent de la **fermeture d'une parente** au commit de sa dernière descendante — `pico8_hj_kj` passe à la forme struct seule, `pq`/`mm`/`kj` retirés de `hj->defaults` ; les caractérisations qui comparaient à `class_function(pico8_hj_class(), …)` visent les entrées du descripteur (vertes sur l'ancien code, même pointeur), celles devenues tautologiques sont retirées. Preuve `gcov` avec `__gcov_dump()` avant les `_exit(0)` des fils dans la copie jetable ; **`ke:pq` avant `jo:pq` est un mutant équivalent**, la mutation d'ordre porteuse est « défauts propres copiés avant l'init de la parente ».
- **1a.24** (`nr`) : `test_pico8_nr`, dont le rejeu `session_events` **suit déjà chaque `fr` frame par frame** (rang, genre, `id`, `me`, `t`, `ra`, `s`, `mi`, tailles de `oe`/`jy`, `m_`, `et`, écran) avec une empreinte figée (0x1FB92F93, 5 `nr`, 5 `fr`) ; `gate_kind` reconnaît un `fr` par la REF de son `hz` (`class_ref(pico8_fr_class(), "hz")`), quelle que soit sa forme.

### Ce que fait le cart

`pico8/lua.lua` :

```
:1101-1108 nr=qh:jr([[ nu=o("nr","np","prober"), od=10,fp=1, kq=b(-4,-4,5,5),
             s=170, hz=o(170,170,170,168), dw=0.01, rr="slipgates", ]])
:1109-1122 nr:gv, nr:ldown, nr:hb, nr:cn
:1123-1127 fr=nr:jr([[ mt=1, rr="infraspace", od=25, s=78,hz=o(78,78,78,76), ]])
:624-625   (jc) self.mt=self.qu.mt or self.re.mt
:916       qh.ly=na
:1182      local gx={qh,os,nr,fr,nl,mx,hj}   -- fr = gx[3] en 0-indexé côté C
```

Points décisifs :

- **Une seule construction** : `ie:ldown` sur `gx[3]`, clé **`id`** seule. `fr` n'a pas de `fz` ; `pico8_fr_new` n'a **aucun appelant** dans `src/` ni `tests/` (grep).
- **Walk `pq`** : ni `fr`, ni `nr`, ni `qh` n'ont de `pq` ; `jo.pq`, puis `ke.pq`. Même ordre que `nr`.
- **Aucune méthode propre** : `fr` atteint les 15 méthodes de `nr` (dont `gv`/`ldown`/`hb`/`cn` de `nr`) — son descripteur est celui de `nr`, entrée pour entrée.
- **Défauts propres (5)** : `mt`=1, `rr`=`"infraspace"`, `od`=25, `s`=78, `hz`=`o(78,78,78,76)`. **`mt` n'a de membre chez aucun ancêtre** (celui de `jc` est une autre classe) → membre propre de `fr`. `rr`, `hz` tombent sur les membres de `nr` ; `od`, `s` sur ceux de `qh`.
- **Hérités de `nr` sans surcharge** : `nu` (d'où `fr` dans `cg.nr`/`np`/`prober`, ni `cg.jo` ni `cg.qh`), `fp`=1, `kq`, `dw`=0.01 ; de `qh`, `hp`, `px`, `me`="kz", `dq`, `hf`, `qh`, `mi`, `ba` ; de `jo`, `hs`, `fa`, `ma`, `ka`, `dz`, `cq`, `kw`, `gq`, `ca`, `em` ; de `ke`, `t`.
- **`mt` est lu, jamais écrit, sur une planète** : `jc:hq` lit `qu.mt`/`re.mt` (`slipway.c:130-131`, par `pico8_jc_entity_value`, double forme) et écrit `mt` sur **le `jc`** (membre de `jc`). C'est ce qui rend une route d'infraspace différente d'une route de slipgate (`lua.lua:631,650,672`).
- **Tables de défaut de `fr`** (`hz`) : jamais écrites en place ni réaffectées. Aucun défaut muté.

### Emprise réelle de `fr` (mesurée)

**Méthode.** Grep de `src/`, `tests/`, `tools/`, `Makefile` à adc9622 ; mesures de sonde de 1a.24 (a88fe1f, `fr` inchangé depuis) pour les champs lus/écrits sur un `fr` ; **sonde propre à cette étape** sur une copie `git archive adc9622` jetable du scratchpad (supprimée après usage) : `pico8_class_get_field` journalise chaque résolution d'une **valeur fonction** dans `defaults` (classe de départ, classe propriétaire, nom), binaire de couverture relancé sur `session_events` et `trading_hub_session_events` (`--from-title`), tous deux en sortie 0.

| Aspect | Mesure |
|---|---|
| Constructions d'un `fr` | **une seule** voie : `ie:ldown` (`research_icon.c:199`), clé **`id`** seule. **5 sur le filet, toutes dans `session_events`** ; 0 dans `trading_hub_session_events` ni ailleurs (couverture `fr 5 1/15`). `pico8_fr_new` : aucun appelant. |
| Défauts propres de `fr` (5) | `mt`=1, `rr`=`"infraspace"`, `od`=25, `s`=78, **`hz`**=`o(78,78,78,76)`. |
| Défauts qui tombent sur un membre hérité (4) | `rr`, `hz` (`pico8_nr_t`) ; `od`, `s` (`pico8_qh_t`). **`mt` n'a de membre chez aucun ancêtre.** |
| Défauts égaux à ceux de `nr` | aucun : `rr`, `od` 25/10, `s` 78/170, `hz` différent. Tout « défaut non appliqué » est un mutant non équivalent. |
| Champs écrits sur un `fr` (sonde 1a.24) | ceux d'un `nr` (`oe`, `hn`, `ep` nil, `jy`, `kw`, `ma`, `fx`, `m_`, `et`, `ra`, `p`, et `s` par `nr:hb`) **plus `fo`** (REF `probe.c:185`, nil `planet_display.c:572`) : **tous parmi les 21 noms de l'écrivain de `nr`**. `mt`, `hz`, `rr` jamais écrits. |
| Membre propre qui en résulte (1) | après `pico8_nr_t` : **`mt`**. |
| Noms lus sur un `fr` | ceux d'un `nr` plus **`mt`** (`slipway.c:130-131`, présent, 1). |
| Méthodes appelées par nom sur un `fr` | les mêmes sites que `nr` (`hd`, `ly`, `kz` absent, `hq`, `ey`, `gv`, `ldown`, `hb`, `hk`, `cn`, `kf` ; `kj`/`ev` par `dc`), toutes dans l'enum. |
| **Qui atteint `nr->defaults` et `qh->defaults`** (sonde de cette étape) | `session_events` : **seule la classe de départ `fr`** y résout une fonction — dans `nr` : `gv` 293, `cn` 230, `hb` 180, `ldown` 8 ; dans `qh` : `ev` 175 585, `ly` 175 128, `hq` 452, `ey` 5 ; dans `jo` : `kj`, `hd`, `hk`, `kf`. `trading_hub_session_events` : **aucune** résolution dans `nr` ni `qh`. Les autres fonctions résolues viennent de `lq`, `ui`, `pj`, `g_`, `fv`, `ga` (→ `jo`), `hp` : aucune dans `nr` ni `qh`. |
| Autres chemins vers ces installations | grep : `pico8_class_get_field` hors `pico8_class.c` et des boucles d'init n'est appelé que par la vue `ie.ke` (`research_icon.c:107-110`), sur `rr`/`s`/`me`/`od` — des **défauts de spec**, pas des méthodes : inchangés. Aucune lecture de `->defaults` de `nr`/`qh` hors de leur init. Descendantes de `qh` : `nl`, `os`, `hj` (→ `pm`, `mx`), `nr` (→ `fr`) — toutes struct après ce commit. |
| Fonctions de `nr` et leurs appelants | `pico8_nr_gv` : son dispatcher, `pico8_nr_ldown`, `test_pico8_mz:726`, `test_pico8_bg:507` (qui lui passent le `void *` rendu par `pico8_nr_new`) ; `pico8_nr_ldown`/`hb`/`cn` : leurs dispatchers seuls. **Après ce commit, seuls des `pico8_nr_t` (un `nr`, ou la partie `nr` d'un `fr`) les atteignent.** |
| Fonctions de `qh` | `pico8_qh_ly_dispatch` → `pico8_na` (`entity_geom.c:62`), aussi appelée par `pico8_nl_ly` ; `pico8_bv_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch` sont **partagés avec `g_`** (installés dans `g_probe_class->defaults`, `probe.c:378-380`), encore instance jusqu'à 1a.27. |
| `->defaults` / classe de `fr` | tous dans `pico8_planet_family_trade_init` (déclaration, `PICO8_CLASS_COVERAGE_NAME`). `cart.c:87` n'asserte que le `parent` de `nr` et `mx`. |
| Tests qui lisent un `fr` ou comparent aux installations retirées | **`test_pico8_qh`** : `descendant_of` (`:501`) ne reconnaît `fr` qu'en instance ; la boucle finale de `test_qh_is_a_struct` (`:783`, `:863`) attend `fr` `INSTANCE` ; `class_value(pico8_qh_class(), …)` sur `hq`/`ev`/`ey`/`ly` (`:339`, `:562`, `:798`). **`test_pico8_jo`** : `daughter_of` (`:420-460`). **`test_pico8_nl`** (`:338`, `:343`, `:593`, `:619`), **`test_pico8_os`** (`:338`, `:620`), **`test_pico8_hj`** (`:386`, `:734`), **`test_pico8_pm`** (`:418`, `:697`), **`test_pico8_mx`** (`:754`) : `fr` dans `others` attendu `INSTANCE`, et/ou comparaison d'une méthode de `qh` à `class_value`/`class_function(pico8_qh_class(), …)`. **`test_pico8_nr`** : `class_function(pico8_nr_class(), …)` (`:367`, `:719`), installations présentes dans `nr->defaults` (`:720`), `fr` instance et `struct_new` NULL (`:740`, `:761-764`). **`test_pico8_ie`** : `gx()` compare des classes, ses asserts de `ldown` tolèrent une struct → inchangé. **`test_pico8_mz`**, **`test_pico8_bg`** : passent un `void *` à `pico8_nr_gv` → inchangés. |
| Durée | `session_events` fait 116 045 frames ; le rejeu qui suit les `fr` existe déjà dans `test_pico8_nr` (`UNIT_TESTS_SLOW`). |

## Solution

1. **Enum : inchangée.**
2. **Struct `fr`**, publique dans `planet_family_trade.h` : `pico8_nr_t` en premier membre, puis **`mt`**. Aucun membre ne répète un nom de `nr`, `qh`, `jo` ou `ke`.
3. **Initialisation d'un `pico8_nr_t` coupée du constructeur de `nr`** : `pico8_nr_struct_init(pico8_nr_t *, const pico8_struct_class_t *)`, **`static`** (`fr` vit dans le même fichier), sans changement de comportement : `pico8_qh_struct_init`, `hz`/`rr` à nil, les 8 défauts de `nr`. Ne lit aucune clé, ne lance ni `pq` ni `lt`, n'enregistre aucune couverture. `pico8_nr_struct_new` l'appelle.
4. **Défauts pris par REF, jamais re-parsés** : `pico8_planet_family_trade_init` déclare `fr` comme aujourd'hui (même spec, même parent, même nom de couverture), pose `pico8_class_set_struct_new(g_fr_class, …)`, puis lit **une fois** les 5 défauts de `fr` dans sa classe (`pico8_class_get_field`, assert « chaque défaut est une clé de la spec de `fr` ») vers un état statique `g_fr_defaults`, chaque entrée à l'offset de son membre : `rr`, `hz` → membres de `nr` ; `od`, `s` → membres de `qh` ; `mt` → membre propre. `pico8_planet_family_trade_shutdown` remet cet état à nil avant de libérer les classes, sans libérer aucune table.
5. **Constructeur de `fr`**, dans l'ordre du walk du cart :
   1. alloue un `pico8_fr_t` ;
   2. `pico8_nr_struct_init(&self->nr, &g_fr_struct_class)` ;
   3. met `mt` à nil, puis copie les 5 défauts statiques de `fr` (ils écrasent les membres hérités) ;
   4. recopie la clé **`id`**, facultative, dans le membre de `jo`, et **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise ; ne garde aucun pointeur sur la table ;
   5. enregistre `"fr"` au crochet de couverture ;
   6. `pico8_jo_pq`, puis `pico8_ke_struct_pq`.
6. **Descripteur de `fr`** (`static const`) :
   - 15 entrées : **les mêmes pointeurs** que le descripteur de `nr` (dont `GV`/`LDOWN`/`HB`/`CN` = les dispatchers de `nr`) ; les 8 autres NULL ;
   - **lecteur chaîné** : `mt` (nil = absent), sinon le lecteur de `nr` ;
   - **écrivain : le même pointeur que celui de `nr`** (21 noms) — `fr` n'écrit aucun nom propre ; `mt` refusé ;
   - accesseur public `pico8_fr_struct_class()`.
7. **Fermeture de `nr`** (règle « Méthodes d'une parente convertie », `fr` dernière descendante) :
   - `pico8_nr_gv(const pico8_nr_t *self, const pico8_instance_t *s)`, `pico8_nr_ldown(pico8_nr_t *self)`, `pico8_nr_hb(pico8_nr_t *self)`, `pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw)` passent à la **forme struct seule** : sur `self`, `oe` est lu dans `self->qh.jo.oe`, `hz` dans `self->hz`, et `hb` écrit `self->qh.s` directement ; **mêmes gardes** (nil, genre, `hz[#oe+1]` absent ou non fixe → 0). Tout ce qui n'est pas `self` garde l'API à double forme : `s.qu` (`pico8_entity_get_field`), les voisins `w` de `cn` (`pico8_mv`), le `trade` `t` (`pico8_instance_get_field(t, "rl")`), le `jc` construit par `ldown`, l'instance vide sur la pile de `ldown` (1a.37). Les quatre dispatchers castent vers `pico8_nr_t *` et restent `static`.
   - **retirées** de `pico8_planet_family_trade_init` : les quatre installations `gv`/`ldown`/`hb`/`cn` dans `g_nr_class->defaults`. Une recherche de ces noms sur la classe `nr` tombe désormais sur ceux de `jo`.
   - l'écrivain de `nr` garde `s` (21 noms) (G).
8. **Fermeture de `qh`** (même règle, `fr` dernière descendante de `qh`) : **retirées** de `pico8_planet_family_init` les quatre installations `hq`/`ly`/`ev`/`ey` dans `g_qh_class->defaults`. Le descripteur de `qh` et ceux de ses filles nomment déjà ces fonctions (`pico8_bv_dispatch`, `pico8_qh_ly_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch`, publics). Corps inchangés : `pico8_qh_ly_dispatch`, `pico8_na` (G, voir Implementation Decisions), `pico8_bv`/`dl`/`d_` (atteints par `g_`, instance). La classe `qh` reste enregistrée (§5, jusqu'à 1a.42) avec ses défauts de spec.
9. **`pico8_fr_new`** rend `pico8_fr_t *`, même paramètre ; son commentaire dit le contrat réel (`overrides` lu, pas gardé : `id` seule). `pico8_nr_new` inchangé.
10. **Tests** : nouveau `test_pico8_fr` (caractérisation dirigée) ; `test_pico8_nr`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx` : changements de forme seulement (voir Testing).
11. **Plan** : à la ligne 1a.25 de `docs/plan-3.md`, ajouter « Découvert à 1a.25 : … » avec au minimum : `mt` (défaut sans membre chez un ancêtre) → membre de `fr` ; défauts `rr`/`hz` (`nr`), `od`/`s` (`qh`) rangés dans les membres hérités ; descripteur aux 15 entrées de `nr` (mêmes pointeurs), lecteur chaîné (`mt`, puis `nr`), écrivain de `nr` (même pointeur, 21 noms, `mt` refusé) ; `pico8_nr_struct_init` coupée du constructeur de `nr` (`static`) ; constructeur `jo:pq` puis `ke:pq`, clé `id` seule ; `pico8_nr_gv`/`ldown`/`hb`/`cn` struct seules, leurs installations retirées de `nr->defaults` ; `hq`/`ly`/`ev`/`ey` retirés de `qh->defaults` (seul `fr` les atteignait, mesuré) ; `pico8_na`, `bv`, `dl`, `d_` restent à double forme ; tests qui reconnaissent `fr` par son descripteur et nomment les méthodes de `qh`/`nr` par leur descripteur.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `fr` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `ga`, `g_`, `ui` à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `fr` mesurée (constructions et clés, défauts, champs écrits, noms lus, méthodes par nom, et qui atteint encore `nr->defaults`/`qh->defaults`), afin que chaque site touché et chaque retrait soit vérifiable.
3. En tant que lecteur du code, je veux une struct `fr` dont le premier membre est `pico8_nr_t` et qui n'ajoute que `mt`, nom minifié conservé, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun membre de `fr` ne répète un nom d'un ancêtre, afin qu'un lecteur passant par le descripteur de `nr` ou de `qh` voie la valeur de la porte d'infraspace.
5. En tant que gardien du filet, je veux que `rr`=`"infraspace"`, `od`=25, `s`=78 et `hz`=`o(78,78,78,76)` écrasent les membres hérités de `nr` et `qh`, afin que son sprite, sa sonde et son libellé soient ceux de la porte d'infraspace.
6. En tant que gardien du filet, je veux qu'une porte d'infraspace garde `nu`, `fp`=1, `kq`, `dw`=0.01 de `nr` et `hp`, `px`, `me`, `dq`, `hf`, `qh`, `mi`, `ba` de `qh`, afin que ses lectures héritées ne changent pas.
7. En tant que gardien du filet, je veux que `mt`=1 soit lu sur une porte d'infraspace et absent sur un slipgate, afin qu'une route tirée depuis elle reste une route d'infraspace (`jc.mt`) au pixel près.
8. En tant que gardien du filet, je veux qu'une porte construite par `ie:ldown` porte l'`id` que l'icône lui donne, afin que `session_events` reste identique au pixel près.
9. En tant que mainteneur, je veux qu'une clé de construction autre que `id` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt (§7).
10. En tant que gardien du filet, je veux que le constructeur appelle `jo:pq` puis le `pq` de `ke`, afin que `oe`, `hn`, `jy`, `kw` existent avant l'enregistrement.
11. En tant que gardien du filet, je veux que `hb` sur une porte d'infraspace écrive `s` = 78, 78, 78, 76 selon ses 0 à 3 liaisons, afin que son sprite suive son nombre de liaisons.
12. En tant que gardien du filet, je veux que `gv`/`ldown` d'une porte d'infraspace refusent à 3 liaisons et rendent sinon un `jc` dont `qu` est la porte, afin qu'on tire une route depuis elle comme avant.
13. En tant que gardien du filet, je veux que `cn` relaie une proposition de commerce à travers une porte d'infraspace comme à travers un slipgate, afin que le commerce ne change pas.
14. En tant que mainteneur, je veux que les 15 entrées du descripteur de `fr` soient les mêmes pointeurs que celles de `nr`, afin qu'un `fr` et un `nr` exécutent le même code.
15. En tant que mainteneur, je veux que l'écrivain de `fr` soit celui de `nr` et refuse `mt`, afin qu'un nom hors des 21 soit un arrêt.
16. En tant que mainteneur, je veux que le lecteur de `fr` délègue à celui de `nr`, afin qu'une fonction de `nr`, `qh`, `jo` ou de la sonde lise une porte d'infraspace sans savoir que c'en est une.
17. En tant que mainteneur, je veux que `hz` d'une porte neuve soit la REF de `fr->defaults`, `nu`/`kq` celles de `nr->defaults`, `px`/`ba` celles de `qh`, afin qu'aucune table ne soit re-parsée ni dupliquée.
18. En tant que mainteneur, je veux que `pico8_nr_gv`/`ldown`/`hb`/`cn` lisent et écrivent `self` directement, maintenant qu'aucune instance ne les atteint, afin que la dette posée à 1a.24 soit soldée.
19. En tant que mainteneur, je veux que ces quatre fonctions gardent exactement leurs gardes, et l'API à double forme pour tout ce qui n'est pas `self`, afin qu'un `jc`, un `mz`, un `bg` ou l'instance vide de `ldown` passés en argument se lisent comme avant.
20. En tant que mainteneur, je veux que les installations `gv`/`ldown`/`hb`/`cn` de `nr` et `hq`/`ly`/`ev`/`ey` de `qh` soient retirées, afin qu'aucune classe ne garde une installation que plus aucune instance n'atteint (§5, « Accès directs aux défauts »).
21. En tant que mainteneur, je veux que les fonctions encore atteintes par une instance (`bv`, `dl`, `d_` par `g_` ; les fonctions de `jo` par `ga`) gardent corps et installations chez leurs autres classes, afin que `g_` et `ga` ne changent pas.
22. En tant que gardien de la couverture, je veux que le constructeur enregistre `"fr"` une fois par porte, afin que `class-coverage-net` soit identique ligne à ligne (`fr 5 1/15`).
23. En tant que vérificateur, je veux une caractérisation de `fr` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`rr`, `mt` d'une porte hors écran, identité des tables, refus à 3 liaisons).
24. En tant que vérificateur, je veux que l'empreinte figée du rejeu `session_events` de `test_pico8_nr`, qui suit chaque `fr` frame par frame, ne bouge pas, afin que la fermeture de `nr` soit prouvée sur les 5 portes du filet.
25. En tant que vérificateur, je veux un rapport `gcov` montrant chaque ligne ajoutée ou modifiée exécutée par les caractérisations, et 2 à 3 mutations ciblées sur ce que `gcov` ne prouve pas, afin de juger la preuve sans monter mon propre banc de mutations.
26. En tant que gardien des caractérisations, je veux que `test_pico8_nr`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm` et `test_pico8_mx` ne changent que leur façon de reconnaître un `fr` ou de nommer une méthode de `qh`/`nr`, sans qu'aucune valeur figée ne bouge, afin que 1a.18 à 1a.24 restent des preuves.
27. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
28. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.25, afin que la liste des étapes dise ce que l'étape a réellement touché.
29. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
30. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
31. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : le module `_trade` de la famille de planètes (struct `fr`, défauts, constructeur, descripteur, lecteur, accesseur, `pico8_fr_new`, `pico8_nr_struct_init`, fermeture de `nr`) et le module de la famille (retrait des quatre installations de `qh` seulement). `pico8_struct.h` (enum), `entity_geom`, `probe`, `slipway`, `research_icon`, `cart`, `economy*`, `ui`, `planet*` : aucun changement.
- **`mt` membre propre de `fr`** : défaut de la chaîne sans membre chez un ancêtre (§5). Pas remonté dans `nr` : `nr` ne l'a pas dans sa spec ; la phase 2 reclasse.
- **Défauts rangés dans les membres hérités quand le nom existe** : même piège qu'à 1a.18-1a.24 — un `fr.s` propre laisserait `qh.s`=170 visible au lecteur de `qh` et à `nr:hb`, qui écrit `qh.s`.
- **`pico8_nr_struct_init` coupée, `static`** (G) : précédent `pico8_hj_struct_init` (1a.22, `static` tant que la fille vit dans le même fichier). Rejeté : dupliquer l'init de `nr` dans le constructeur de `fr` (deux listes à tenir).
- **Descripteur `static const` qui nomme les mêmes pointeurs que celui de `nr`** : `fr` ne surcharge rien ; les 4 dispatchers de `nr` sont dans le même fichier, les 11 autres déjà publics.
- **Écrivain de `fr` = même pointeur que celui de `nr`** (G) : aucun nom propre écrit (mesuré) ; précédent `pm` (1a.22). `mt` refusé comme `hz`/`rr`.
- **Clé `id` seule** (G) : seule mesurée ; `pico8_fr_new` n'a aucun appelant. Précédent `mx` (1a.23).
- **Fermeture de `nr` : forme struct seule, installations retirées** : règle « Méthodes d'une parente convertie », `fr` dernière descendante ; précédent `hj` à 1a.23. Mesuré : `fr` est la seule classe qui résout une fonction dans `nr->defaults`.
- **`self` seul passe en `pico8_nr_t *`** (G) : `s` de `gv` (un `jc`, un `nr`, un `mz`, un `bg` ou l'instance vide), `lw`/`t`/`w` de `cn` gardent leur type et l'API à double forme — changement chirurgical ; `{}` de `ldown` est l'étape 1a.37. `test_pico8_mz` et `test_pico8_bg` passent un `void *` : diff vide.
- **L'écrivain de `nr` garde `s`** (G) : `nr:hb` n'y passe plus (écriture directe), mais le nom reste un champ écrit par la méthode propre de `nr` (§5) et `test_nr_is_a_struct` l'écrit ; le retirer changerait un contrat posé à 1a.24 sans gain. Précédent : l'écrivain de `mx` garde `ht` alors que `mx:kf` l'écrit directement.
- **Fermeture de `qh` : installations `hq`/`ly`/`ev`/`ey` retirées** (G) : la règle du §5 s'applique à `qh` comme à `nr`, `fr` étant aussi la dernière descendante non convertie de `qh` (mesuré : seule classe qui résout une fonction dans `qh->defaults`) ; laisser des installations mortes contredirait « Accès directs aux défauts ». Rejeté : les garder jusqu'à 1a.42 (dette sans lecteur, et `class_value(pico8_qh_class(), "ly")` resterait une référence trompeuse dans six caractérisations).
- **`pico8_na` reste à double forme, corps inchangé** (G) : c'est une fonction libre du module de géométrie (`na` du cart, affectée à `qh.ly` et appelée par `nl:ly`), qui lit `ra` par l'API à double forme, déjà valide sur une struct. La passer en `pico8_qh_t *` ferait dépendre `entity_geom` de la famille de planètes, pour aucun gain en 1a (la phase 1b type ses membres). `pico8_qh_ly_dispatch` inchangé. `bv`/`dl`/`d_` restent à double forme (`g_`, 1a.27).
- **Constructeur `jo:pq` → `ke:pq`** : ordre du walk du cart. `ke:pq` avant `jo:pq` est un mutant équivalent (1a.23) ; la mutation d'ordre porteuse est « défauts de `fr` copiés avant `pico8_nr_struct_init` ».
- **5 défauts copiés depuis la classe à l'init** (G), scalaires compris. Rejeté : les écrire en dur (deux sources).
- **Sémantique de nil** : `ep`/`fo` écrits nil sont présents-nil sur l'instance (TROU-9), absents sur la struct ; leurs lecteurs testent présence et vérité : même branche (constat de 1a.23-1a.24). `mt` présent sur `fr`, absent sur `nr` sous les deux formes. Pas d'écart.
- **Clés dynamiques** : aucune.
- **TROU** : aucun écart attendu. `pico8_nr_ldown` garde son `/* TROU-6 */`. Si l'implémenteur en trouve un, règle §7.
- **Commentaires** (CLAUDE.md) :
  - à ajouter : contrat de la struct `fr` (jamais libérée, TROU-7 ; un défaut dont le nom est un membre d'un ancêtre va dans ce membre ; `hz` est la table de la classe `fr`, `nu`/`kq` celles de `nr`, `px`/`ba` celles de `qh`, empruntées) ; contrat de `pico8_nr_struct_init` (repris de celui de `pico8_qh_struct_init`/`pico8_hj_struct_init`) ; assert de la clé ; contrat de `pico8_fr_new` ;
  - à corriger ou retirer si rendus faux : le commentaire de `pico8_nr_gv` (« `nr:ldown` passes a stack instance » reste vrai), celui de `pico8_fr_new` (« `overrides` is copied »), « Frees the three classes » (reste vrai) ; commentaires de tests qui disent `fr` instance ou « nr's class still installs » (`test_pico8_nr` en tête et au-dessus de `test_nr_is_a_struct`, `test_pico8_qh` liste des structs, `test_pico8_mx`/`pm` « stay instances ») ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert (et `make e2e` au dernier). La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_fr` sur l'ancien code** (voir Testing), valeurs figées. Durée mesurée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si une porte construite par l'icône n'a pas `mt`=1, `od`=25, `s`=78 : mesure fausse, à corriger dans le PRD.
2. **Tests relus, verts sur l'ancien code** : dans `test_pico8_qh`, `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_nr`, chaque comparaison d'une méthode `hq`/`ly`/`ev`/`ey` à `class_value`/`class_function(pico8_qh_class(), …)` vise l'entrée correspondante de `pico8_qh_struct_class()`, et chaque comparaison de `gv`/`ldown`/`hb`/`cn` à `class_function(pico8_nr_class(), …)` vise l'entrée de `pico8_nr_struct_class()` — même pointeur sur l'ancien code, donc aucune empreinte ne bouge. Les assertions devenues tautologiques (le descripteur comparé à lui-même) sont retirées. Suite verte.
3. **`fr` convertie** : `pico8_nr_struct_init`, struct, statiques de défauts, constructeur, descripteur, lecteur, accesseur, `pico8_class_set_struct_new`, `pico8_fr_new` typé ; `test_pico8_qh`, `test_pico8_jo` (reconnaissance de `fr`), `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx` (`fr` quitte `others`, le tableau disparaît s'il devient vide), `test_pico8_nr` (`fr` struct, `struct_new` posé). Toutes les caractérisations passent **sans changer leurs valeurs figées** ; les vérifications de forme s'ajoutent à `test_pico8_fr`.
4. **Fermeture de `nr` et de `qh`** : quatre fonctions de `nr` struct seules, huit installations retirées ; `test_nr_is_a_struct` attend les installations absentes (une recherche sur la classe `nr` tombe sur `jo`), `test_nl_is_a_struct` n'attend plus `ly` sur les classes `nl`/`qh`. Valeurs figées identiques.
5. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.25 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF et des pointeurs de fonction, `s` après `hb`, le `jc` rendu par `ldown` et son `mt`. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_fr`** (lié comme `test_pico8_nr`, sur le cart réel, dépendance Makefile sur `empty_planet_cleared_events.json` et les fixtures qu'il lit). Il lit **uniquement** par l'API à double forme et par `pico8_class_get_field(pico8_fr_class()/pico8_nr_class()/pico8_qh_class()/pico8_jo_class(), …)` pour les **défauts de spec** (jamais pour une méthode : les installations disparaissent au morceau 4) ; son code est le même avant et après, aux vérifications de forme près. Chaque scénario tourne dans un fils forké. **Toute présence se lit par un `lookup` qui confond nil et absent** (celui de `test_pico8_nr`).
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **Construction** (partie lancée comme dans `test_pico8_nr`) : `build(pico8_fr_class())` par l'icône (`ie:ldown`), deux fois. Empreinte de `id`, `me`, `t`, `rr`, `mt`, `hp`, `s`, `od`, `fp`, `dw`, `dq`, `qh`, `mi`, `hf`, `hs`, `fa`, `ma`, `ca`, `gq`, taille de `hz` ; présence de `mt`, `hz`, `rr`, des 10 de `qh` et des 31 de `jo` selon leur état ; absence de `qu`, `qw`, `ht`, `qy`, `et` ; les 15 méthodes présentes et **égales une à une à celles d'un slipgate construit dans la même partie**, les 8 autres absentes. Appartenance et rang dans `hv`, `cv`, `cg.nr`/`np`/`prober`, absence de `cg.jo` et `cg.qh`. **Identité** : `hz` = REF de `pico8_class_get_field(pico8_fr_class(), "hz")` et ≠ celle de `nr` ; `nu`, `kq` = REF de la classe `nr` ; `px`, `ba` = REF de la classe `qh` ; `em`, `ka`, `dz`, `cq`, `gq` = REF de la classe `jo` ; `kw` ≠ REF de la classe `jo`. Figée.
    2. **Comportement** : `dg` en `"kz"` puis `"ly"` ; `gv` sans `qu` → vrai, contre un `s` dont `qu` est la porte → faux, avec `oe` à 3 éléments posé par l'écrivain générique → faux ; `ldown` à 0 liaison → un `jc` (struct) dont `qu` est la porte, à 3 liaisons → faux ; `hb` avec `oe` de 0 à 3 éléments → `s` = 78, 78, 78, 76 ; `kj` via `dc` → empreinte d'écran ; **`mt` d'une route** : si la recette du `jc` réel de `test_pico8_mx` (`lookup_named("jc")`, `jc:ey` vers une autre planète) se reprend sans nouvel outillage, le `jc` tiré depuis la porte porte `mt`=1 et celui tiré depuis un slipgate ne l'a pas ; sinon `mt` n'est prouvé que par sa lecture directe et par `frames-check`. Figés. `cn` relayé est prouvé par le rejeu de `test_pico8_nr` et `gcov`.
  - **Pas de rejeu dans `test_pico8_fr`** (G) : le rejeu `session_events` de `test_pico8_nr` suit déjà chaque `fr` frame par frame avec une empreinte figée sur le code où `fr` était instance ; il garde les 5 portes du filet à travers la conversion et la fermeture de `nr`. Un second rejeu de 116 045 frames ne prouverait que `od`/`rr`/`mt` de plus, que la caractérisation dirigée fige.
  - **Forme** (ajoutée au morceau 3-4) : une porte construite par `ie:ldown` est `STRUCT` au descripteur de `fr` ; ses 15 entrées sont **les mêmes** que celles de `pico8_nr_struct_class()`, les 8 autres NULL ; `cls->set_field == pico8_nr_struct_class()->set_field`, `cls->field` différent ; le lecteur de `nr` voit sur une porte les 4 défauts rangés dans un membre hérité (`rr`, `hz`, `od`, `s`), pas `mt` ; `pico8_entity_set_field` passe pour les 21 noms et asserte (dans un fils) pour `mt`, `hz`, `rr`, `qw`, `ht` ; une clé de construction autre que `id`, ou positionnelle, asserte ; `pico8_fr_new(NULL)` rend une struct sans `id` mais avec `oe` et `kw`, `mt` écrit nil lu absent ; `pico8_fr_class()->struct_new` posé ; **`gv`/`ldown`/`hb`/`cn` absents de `pico8_nr_class()->defaults`** et **`hq`/`ly`/`ev`/`ey` absents de `pico8_qh_class()->defaults`** (lecture directe de la table, pas par la chaîne).
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (`git archive` ou copie de l'arbre du commit candidat, supprimée après usage), compiler `test_pico8_fr`, `test_pico8_nr` et les objets du moteur avec `--coverage -O0`, lancer les deux, puis `gcov` sur les deux fichiers de la famille. Même piège qu'à 1a.23 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`. Les branches d'assert atteintes seulement dans un fils qui avorte (`must_abort`) ne sont pas comptées : prouvées par le `SIGABRT` attendu et justifiées ainsi. Le rapport (lignes exécutables ajoutées ou modifiées de `src/`, leur compte — en particulier les quatre fonctions de `nr` sur un `fr` et sur un `nr`, `cn` compris —, lignes à 0 avec justification) est collé dans le message de commit.
  - **2 à 3 mutations**, chacune rouge sur `test_pico8_fr` ou `test_pico8_nr`, notées dans le commit avec le test qui l'attrape :
    1. **valeur d'un défaut propre** : `od` (ou `s`) de `fr` rangé dans une copie hors lecteur au lieu du membre de `qh` — ou `mt` non copié ;
    2. **entrée du descripteur remplacée** : `HB` de `fr` = `pico8_jo_hb_dispatch` (`s` ne suit plus `hz`) ;
    3. **ordre du constructeur** : défauts de `fr` copiés avant `pico8_nr_struct_init` (`nr` les écrase : `s`=170, `od`=10, `hz` de `nr`, et `gate_kind` de `test_pico8_nr` ne reconnaît plus les `fr` du rejeu). Pas `ke:pq` avant `jo:pq` (équivalent, 1a.23).
- **`test_pico8_nr`** : `class_function(pico8_nr_class(), …)` → entrées de `pico8_nr_struct_class()` (morceau 2) ; `test_nr_is_a_struct` attend `fr` struct au descripteur de `fr`, `struct_new` posé sur les deux classes, les quatre installations **absentes** de `nr->defaults` ; son `hb` d'un `fr` rend toujours 78. Les empreintes `built`, `life`, `session`, `hub` ne bougent pas.
- **`test_pico8_qh`** : `descendant_of` reconnaît une struct au descripteur de `fr` ; la boucle finale de `test_qh_is_a_struct` attend `fr` struct ; les comparaisons `hq`/`ev`/`ey`/`ly` à la classe `qh` visent le descripteur de `qh` (celles qui deviennent tautologiques sont retirées). **`test_pico8_jo`** : `daughter_of` idem. **`test_pico8_nl`** : comparaisons à la classe `qh` → descripteur de `qh`, l'assertion `:619` (le `ly` de la classe `nl` est celui de la classe `qh`) devient « `ly` absent des deux classes », `others` vide retiré. **`test_pico8_os`**, **`test_pico8_hj`**, **`test_pico8_pm`** : comparaisons à la classe `qh` → descripteur de `qh` ; `fr` retiré de `others`. **`test_pico8_mx`** : `fr` retiré de `others`. Aucune valeur figée touchée.
- **Tests existants** : `test_pico8_ie` (diff vide), `test_pico8_mz`, `test_pico8_bg`, `test_pico8_jc`, `geoharvest_burst`, `qb`, `ga_export_panel_by`, `nf`, `js`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage` : diff vide. Tous les tests de régression (§6) : diff vide, verts. Aucun ne construit un `fr` : aucune preuve rouge n'est due.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** : `test_pico8_nr` (structure, `lookup`, `must_abort`, `build` par l'icône, `gate_kind`, rejeu qui suit les `fr`), `test_pico8_mx` à 1a.23 (fermeture d'une parente, tests retargetés vers le descripteur, `jc` réel), `test_pico8_pm` à 1a.22 (écrivain de la parente, même pointeur).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `fr` n'est pas `fr  5  1/15`, `nr` n'est pas `nr  6  2/15`, `qh` n'est pas `qh  8  3/15`, ou si une autre ligne change. Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées/modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations (défaut propre, entrée du descripteur, ordre du constructeur) ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier : si l'assertion de clé ou celle de l'écrivain se déclenche sur un rejeu ou un test ; si un membre propre autre que `mt` est ajouté ; si un membre propre répète un nom d'un ancêtre ; si l'écrivain de `fr` n'est pas celui de `nr`, ou si la liste des 21 noms de `nr`, des 20 de `qh` ou des 17 de `jo` change ; si l'enum change.
- `FAILED:` si une autre classe que `fr` est convertie ou change de forme (`ga`, `g_`, `ui`, …), ou si le comportement de `qh`, `nl`, `os`, `hj`, `pm`, `mx` ou `nr` change (valeur figée de leurs caractérisations, lignes de couverture).
- `FAILED:` si une entrée du descripteur de `fr` n'est pas le même pointeur que l'entrée correspondante du descripteur de `nr`.
- `FAILED:` si une installation `gv`/`ldown`/`hb`/`cn` reste dans `g_nr_class->defaults` ou une installation `hq`/`ly`/`ev`/`ey` dans `g_qh_class->defaults` ; si une installation `->defaults` de `jo`, `g_` (`g_probe_class`), `ga` ou `ui` est retirée ou changée.
- `FAILED:` si `pico8_nr_gv`/`ldown`/`hb`/`cn` lisent ou écrivent encore `self` par l'API à double forme, changent une garde, ou changent le type d'un paramètre autre que `self` ; si une fonction encore atteignable par une instance change de corps ou devient struct seule : `pico8_na`, `pico8_qh_ly_dispatch`, `pico8_bv`/`pico8_dl`/`pico8_d_` et leurs dispatchers, `pico8_f_`, `pico8_g__mm`, `pico8_ez`, `pico8_ei`, une fonction `pico8_jo_*`.
- `FAILED:` si une table de défaut de `fr` est re-parsée depuis la spec, ou si `hz` d'une porte neuve n'est pas la REF de `fr->defaults` et `nu`/`kq` celles de `nr->defaults`.
- `FAILED:` si le constructeur de `fr` n'appelle pas `jo:pq` puis `ke:pq`, ou copie les défauts de `fr` avant `pico8_nr_struct_init`.
- `FAILED:` si un test de régression (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une valeur figée de `test_pico8_fr` a été changée après la conversion, ou une de `test_pico8_nr` (dont `session` 0x1FB92F93, 5 `nr`, 5 `fr`, et `hub` 0x65DBE61E), `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `ie`, `mz`, `bg`, `jc`, `geoharvest_burst`, `qb`, `nf`, `js`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo` ; ou si ces huit caractérisations changent au-delà de la reconnaissance de `fr`, des listes `others`, des comparaisons de méthodes retargetées vers un descripteur et des assertions de forme sur les installations.
- `FAILED:` si `test_pico8_fr` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- la struct `fr` n'a que `pico8_nr_t` et `mt` ; son descripteur a les 15 entrées de celui de `nr` ;
- `pico8_nr_struct_init` est `static`, appelée par les constructeurs de `nr` et de `fr`, et par eux seuls ;
- grep : `PICO8_CLASS_COVERAGE_NAME(g_fr_class, "fr")` reste ; `pico8_class_set_struct_new(g_fr_class, …)` présent ; aucune installation `set_name(g_nr_class->defaults, …)` ni `set_name(g_qh_class->defaults, …)` ; aucune nouvelle lecture de `->defaults` ;
- hors des deux modules de la famille, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_fr.c` (nouveau), `test_pico8_nr.c`, `test_pico8_qh.c`, `test_pico8_jo.c`, `test_pico8_nl.c`, `test_pico8_os.c`, `test_pico8_hj.c`, `test_pico8_pm.c`, `test_pico8_mx.c` et le `Makefile` (cible, `UNIT_TESTS`, éventuellement `UNIT_TESTS_SLOW`) bougent ;
- la ligne 1a.25 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.25 ».

## Out of Scope

- `ga` (1a.26, dernière descendante de `jo` : fermeture de `jo`), `g_` (1a.27 : `bv`/`dl`/`d_`, `f_`, `g_.mm` à forme struct seule), `ui`, et leurs `->defaults`.
- Passer `pico8_na` ou `pico8_qh_ly_dispatch` à la forme struct seule ; rendre publics `pico8_nr_field`, `pico8_nr_set_field`, `pico8_nr_struct_init` ou les dispatchers de `nr`.
- Le type de `s` dans `pico8_nr_gv`, de `lw`/`t`/`w` dans `pico8_nr_cn`, et l'instance vide sur la pile de `pico8_nr_ldown` (1a.37).
- Retirer `s` de l'écrivain de `nr`.
- La suppression des `pico8_class_t` de `fr`, `nr`, `qh` (1a.42).
- Le typage C des membres (1b) ; archétypes (phase 2).
- Libérer une porte (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Double fermeture** : `fr` est à la fois la dernière descendante de `nr` et de `qh` ; c'est le premier commit qui ferme deux parentes. La dette posée à 1a.24 (quatre méthodes à double forme, quatre installations) et celle de 1a.18 (installations de `qh` gardées pour ses descendantes) se soldent ici. Après ce commit, la seule planète encore instance est `ga`, dont la conversion fermera `jo`.
- **Aucune méthode propre** : comme `os` (1a.20) pour `qh`, `fr` ne diffère de `nr` que par ses défauts et `mt` ; tout son comportement est celui de `nr`, d'où la caractérisation dirigée qui compare ses 15 méthodes à celles d'un slipgate.
- **`mt` hors écran** : `mt` ne se voit qu'à travers le `jc` (couleur et épaisseur de la route, `lua.lua:631,672`) ; la caractérisation dirigée le lit sur la porte, le filet sur la route.
- **Leçons de 1a.23-1a.24 appliquées** : comparaisons de méthodes retargetées vers le descripteur avant la conversion (vertes sur l'ancien code) ; mutation d'ordre sur la copie des défauts, pas sur `jo:pq`/`ke:pq` ; piège `_exit` des fils réglé dans la copie jetable seulement.
- Mesures : grep sur `src/`, `tests/`, `tools/`, `Makefile` à adc9622 ; lecture de `pico8/lua.lua:577,624-672,916,1101-1127,1182`, de `pico8_planet_family.c`, `pico8_planet_family_trade.c/h`, `pico8_entity_geom.c`, `pico8_research_icon.c`, `pico8_class.c`, `pico8_slipway.c`, des tests nommés, de `tools/class_coverage_net_expected.txt`, du PRD et du message de 243426b (1a.24) et de af03e84 (1a.23) ; sonde de résolution de fonctions dans `defaults` (`session_events` et `trading_hub_session_events` en sortie 0 ; seule `fr` atteint `nr` et `qh`) dans une copie jetable du scratchpad, supprimée après usage.
