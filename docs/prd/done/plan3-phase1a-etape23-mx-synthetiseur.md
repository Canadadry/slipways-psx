---
title: "Plan 3, étape 1a.23 : `mx` (le synthétiseur) devient une struct fille de `hj` avec 1 membre propre (`ht`) et sa propre méthode `kf` ; dernière descendante de `hj`, elle fait passer `hj:kj` à la forme struct seule et retire les installations `pq`/`mm`/`kj` de `hj`"
description: "Le synthétiseur `mx` est encore une instance de la couche objet, construite 3 fois sur le filet (toutes dans `events_recorded_1790198294`) par la seule voie `ie:ldown` sur `gx[5]`, avec la clé `id`. Il passe en struct `pico8_mx_t` (`pico8_hj_t` puis `ht`), son descripteur reprend les 16 entrées de `hj` sauf `KF` = `mx:kf`, qui passe à la forme struct seule et quitte `mx->defaults`. Comme `mx` est la dernière descendante non convertie de `hj`, `pico8_hj_kj` passe à la forme struct seule et `pq`/`mm`/`kj` quittent la classe `hj` ; `f_`, `g_.mm`, `ez`, `ei` restent à double forme (atteints par `g_` et `ga`)."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.23 — `mx` : `hj` (`planet_family_trade.c:333`), `gx[5]`.

(À HEAD d0e2276, la déclaration `g_mx_class = pico8_class_declare(pico8_hj_class(), …)` est à `planet_family_trade.c:335`, son nom de couverture à `:349`, l'installation `kf` dans `g_mx_class->defaults` à `:350`, `pico8_mx_kf` à `:225`, son dispatcher à `:295`, `pico8_mx_new` à `:388`. La construction est `ie:ldown` → `pico8_class_instantiate(gx[5], {id})`, `research_icon.c:199`, avec `gx[5] = pico8_mx_class()` posé à `cart.c:228`. Côté `hj` : `pico8_hj_kj` à `planet_family.c:406`, `pico8_hj_kj_dispatch` (`static`) à `:419`, `pico8_hj_struct_init` (`static`) à `:519`, les trois installations gardées pour `mx` à `:915-920` ; `pico8_qh_ly_dispatch` (`static`) à `:54`. La ligne ne nomme aucun test de régression.)

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. […]
>
> **Champ d'un écrivain générique** : […] seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]` […] construi[t] ainsi la forme struct sans être touché[…].
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre […] reste à double forme […]. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe […] dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct : une table de défaut […] est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec.

`pico8_mx_t` commence par `pico8_hj_t`, qui commence par `pico8_qh_t`, `pico8_jo_t`, `pico8_ke_t`. `mx` n'a aucune descendante : c'est une feuille, et **la dernière descendante non convertie de `hj`** (`pm` l'est depuis 1a.22).

§6 : aucun test de régression nommé par la ligne ; aucun fichier de régression ne construit un `mx` (mesuré). Les caractérisations qui lisent un `mx` (`test_pico8_hj`, `test_pico8_pm`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`) changent leur façon de le reconnaître ou de nommer la fonction attendue, jamais une valeur figée. §7 : un commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (`mx  3  1/15`, `hj  5  1/15`, `pm  4  2/15`, toutes les autres lignes inchangées) ; **preuve des tests par `gcov` et 2 à 3 mutations** (nouvelle règle à partir de cette étape) ; un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.23, avec « découvert à 1a.23 » dans le message ; un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.22 ont posé (jusqu'à 7b207e1), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`). L'enum vaut `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL`, `LY`, `MM`. Un membre nil est absent ; l'écrivain rend 0 pour un nom sans membre et `pico8_entity_set_field` asserte alors.
- `pico8_jo_t` (31 membres, dont `kw`, `mi`, `id`), les 11 dispatchers publics de `jo`, `pico8_jo_pq`, `pico8_jo_kf` (double forme) ; `pico8_ke_struct_pq`.
- `pico8_qh_t`, lecteur/écrivain de `qh` (20 noms) ; `pico8_qh_ly_dispatch` (`static`, double forme : `nr`/`fr` l'atteignent par `qh->defaults`).
- `pico8_hj_t` (13 membres : `rr`, `py`, `ki`, `pn`, `pk`, `fc`, `k_`, `du`, `qo`, `ro`, `qg`, `q_`, `qy`), `g_hj_defaults` (18), lecteur `pico8_hj_field` et écrivain `pico8_hj_set_field` (21 noms) `static`, joignables par `pico8_hj_struct_class()->field`/`->set_field` ; descripteur de `hj` aux 16 entrées ; `pico8_hj_struct_init(self, cls)` **coupée à 1a.22, `static` « jusqu'à `mx` (1a.23, autre fichier) »**.
- **1a.19** (`nl`, 7719e74) : la méthode propre d'une feuille quitte `defaults`, passe à la forme struct seule, signature typée, lecture directe des membres.
- **1a.21** (`hj`, 516195e) et **1a.22** (`pm`, 7b207e1) : `pq` (`f_`), `mm`, `kj` gardés dans `hj->defaults` et `pico8_hj_kj` à double forme **pour `mx`** ; commentaire en place à `planet_family.c:915-917` : « Remove them at mx's conversion (1a.23), hj's last descendant ».
- **1a.18** : précédent de dispatchers rendus publics pour qu'un descripteur `static const` d'une autre unité de compilation nomme les mêmes pointeurs.

### Ce que fait le cart

`pico8/lua.lua` :

```
:998-1016  hj=qh:jr([[ … qh=f … ]]) ; hj.pq=f_ ; hj.mm=g_.mm ; function hj:kj(p) … end
:1036-1048 mx=hj:jr([[ nu=o("jo","np","prober"), rr="void synthesis",
             hp=5,s=66,px=o("synthesizer",c=13), od=40, ht=o(28,20,21,22),
             py=0.12,ki=9, pn=5,pk=1, fc=0.3, k_=o(1,1,5,5,3,3,3,5,5,1), du=5,
             qo=0.98,ro=-0.07, qg=0.07,q_=0.98, ]])
:1049-1055 function mx:kf(sw) local cp=sw:eb(self) for i in all(cp.jy) do
             if i.gc==0 and gu(self.ht,i.qe) then self.kw.qe,self.ht=i.qe,{} end
           end jo.kf(self,sw) end
:1182      local gx={qh,os,nr,fr,nl,mx,hj}   -- mx = gx[5] en 0-indexé côté C
```

Points décisifs :

- **Une seule construction** : `ie:ldown` sur `gx[5]`, clé **`id`** seule. `mx` n'a pas de `fz` : ni `kv[…]`, ni `lookup_named`.
- **Walk `pq`** : `mx` n'a pas de `pq` ; `hj.pq`=`f_` (`qy`), puis `jo.pq` (`oe`, `lz(gb[5])`, `kw`), puis `ke.pq` (`lt`). Même ordre que `hj`.
- **`mx:kf` est sa seule méthode propre**, surcharge de `jo:kf`, appelée par nom (`mv(…,"kf")`, `slipway.c:177`). Elle lit `self.ht`, écrit **sur l'instance** `self.ht={}` (table neuve ; la table de défaut n'est jamais modifiée en place) et `self.kw.qe` **en place dans le `kw` de l'instance** (écrit par `jo:pq`→`lz` à la construction, jamais celui de la classe), puis appelle `jo.kf`.
- **`ht`** n'a de membre chez aucun ancêtre → membre propre de `mx`, écrit par la méthode propre de la fille (§5).
- **Hérités sans surcharge** : de `hj`, seulement `qh`=false (et les trois fonctions `f_`, `mm`, `kj`) ; de `qh`, `me`="kz", `dq`, `hf`, `fp`, `dw`, `mi`=1, `ba` ; de `jo`, `hs`, `fa`, `kq`, `ma`, `ka`, `dz`, `cq`, `kw`, `gq`, `ca`, `em` ; de `ke`, `t`.
- **`nu` de `mx` ne contient ni `"hj"` ni `"qh"`** : un `mx` entre dans `cg.jo`/`np`/`prober` seulement.
- **Tables de défaut de `mx`** (`nu`, `px`, `ht`, `k_`) : jamais écrites en place ni réaffectées sur la classe (grep du cart). Aucun défaut muté.

### Emprise réelle de `mx` (mesurée sur d0e2276)

**Méthode.** Grep de `src/`, `tests/`, `tools/`, `Makefile`, lecture des fonctions, puis **sonde** sur une copie `git archive HEAD` jetable du scratchpad (supprimée après mesure) : `pico8_entity_get_field`/`set_field`, `pico8_mv`, `pico8_instance_get_field`, `pico8_instance_fields`, `pico8_class_instantiate` deviennent, hors de `pico8_entity.c`/`pico8_class.c`, des macros (`-include`) qui journalisent (opération, nom, fichier:ligne, absent/nil/présent ; clés de construction) chaque accès à une instance dont la classe exacte est `pico8_mx_class()`, journal à tampon ligne. Binaire de couverture sondé relancé sur les 15 rejeux du filet un par un (`--from-title` pour les 4 `RECORDINGS`), tous en sortie 0.

| Aspect | Mesure |
|---|---|
| Constructions d'un `mx` | **une seule** voie : `ie:ldown` (`research_icon.c:199`), clé **`id`** seule. **3 sur le filet, toutes dans `events_recorded_1790198294`** (`--from-title`) ; **0** dans les 14 autres rejeux. `pico8_mx_new` : aucun appelant (`src/`, `tests/`, `tools/`). Hors jeu : `test_pico8_hj` (`test_heirs` : `build(pico8_mx_class())` par l'icône). |
| Défauts propres de `mx` (18) | `nu`, `rr`=`"void synthesis"`, `hp`=5, `s`=66, `px`, `od`=40, **`ht`**=`o(28,20,21,22)`, `py`=0.12, `ki`=9, `pn`=5, `pk`=1, `fc`=0.3, `k_`, `du`=5, `qo`=0.98, `ro`=-0.07, `qg`=0.07, `q_`=0.98. |
| Défauts qui tombent sur un membre hérité (17) | `nu` (`pico8_jo_t`) ; `hp`, `s`, `px`, `od` (`pico8_qh_t`) ; `rr`, `py`, `ki`, `pn`, `pk`, `fc`, `k_`, `du`, `qo`, `ro`, `qg`, `q_` (`pico8_hj_t`). **Le 18e, `ht`, n'a de membre chez aucun ancêtre.** |
| Défauts égaux à ceux de `hj` | `py`, `ki`, `pn`, `pk`, `qo`, `ro`, `qg`, `q_` ont la même valeur que chez `hj` : un « défaut non appliqué » sur eux serait un mutant équivalent. Différents : `nu`, `rr`, `hp` (5/2), `s` (66/166), `px`, `od` (40/60), `fc` (0.3/0.5), `k_`, `du` (5/7.5). |
| Défauts mutés | aucun. |
| Champs écrits sur un `mx` (sonde) | 19 noms : `oe`, `hn`, `ep` (nil), `jy`, `kw`, `ma` (`planet.c:481` ; `entity_geom.c:79` 15 264 fois), `mi` (`economy_tiers.c:218`), `cc`, `by` (`economy_tiers.c:92,107`), `kr` (`economy.c:242`), `lq` (`planet_display.c:120`), `fo` (nil `planet_display.c:572` ; REF `probe.c:185`), `fx` (`planet_trade.c:94`), `m_` (`ui.c:308,313`), `qy` (`particle.c:69`, une fois par `mx`), `et`/`ra`/`p` (`probe.c:179,132,138` : un `mx` est sondé 240 fois), et **`ht`** (`planet_family_trade.c:285`, **2 fois** : `mx:kf` accepte 2 des 3 liaisons). **Tous parmi les 21 noms de `hj` sauf `ht`.** Écritures nil : `ep`, `fo`. |
| Membres propres qui en résultent (1) | après `pico8_hj_t` : **`ht`**. |
| Noms lus sur un `mx` (sonde, 110 sites distincts) | présents : `me`, `t`, `id`, `ra`, `p`, `et`, `oe`, `jy`, `kw` (dont `planet_family_trade.c:278`), `hn`, `kq`, `fa`, `ma`, `s`, `hp`, `hf`, `hs`, `qh` (faux), `mi` (`hj:kj`, 24 996 lectures toutes sites confondus), `lq`, `cc`, `by`, `fx`, `kr`, `ca`, `gq`, `px`, `od`, `dq`, `fp`, `dw`, **`ht`** (4, `planet_family_trade.c:268`), les champs de particules `fc`, `qo`, `qg`, `ro`, `q_`, `k_`, `qy`, `pn`, `du`, `py`, `ki`, `pk`, et `gv` (la fonction, `slipway.c:110`). Absents (aujourd'hui comme demain) : `qw`, `nk`, `fg`, `ql`, `mt`, `qs`, `nl`, `jh`, `p_`, `eo`, `ff`, `hi`, `ik` (`probe.c:302`), et `by`/`cc` avant leur première écriture. **Présent-nil sur l'instance (TROU-9)** : `fo` 773 fois (`planet_display.c:552`), `ep` 2 fois (`msgbox.c:462`) ; ces deux lecteurs testent présence **et** vérité : même branche sur la struct. **Jamais lus par l'API sur le filet** : `rr`, `ba`, `ka`, `dz`, `cq`, `em` (et `nu`, lu par `lt` dans `pico8_entity.c`). |
| Méthodes appelées par nom sur un `mx` | `hd` (`ui.c:296`), `ly` (`bootstrap.c:89`, 24 399 fois après `d_`) et `kz` (même site, 237 fois, **absent** : ni `qh` ni `hj` n'en ont — absent aussi sur la struct), `mm` (`particle.c:83`, 4 500), `gg`, `hb`, `hk`, `ib`, `cn`, `hq` (`ui.c:360`), `ey` (`ui.c:373`), `gv`/`ldown` (`ui.c:411`), **`kf`** (`slipway.c:177`, 3), `kj`/`ev` (`dc`). **Toutes dans l'enum** ; toutes des entrées du descripteur de `hj`, sauf `KF` que `mx` surcharge. |
| Lectures à simple forme | **aucune** : zéro `pico8_instance_get_field` sur un `mx` ; `pico8_instance_fields` seulement aux sites déjà aiguillés (`bootstrap.c:101`, `ke.c:45-46`, `particle.c:63`, `probe.c:128`). |
| `->defaults` / classe de `mx` | trois accès, tous dans `pico8_planet_family_trade_init` : déclaration, `PICO8_CLASS_COVERAGE_NAME` (`:349`), installation `kf` (`:350`). Aucune lecture de `g_mx_class->defaults` ailleurs ; `cart.c:87` n'asserte que `parent`. |
| Fonctions de `hj` et leurs appelants | `pico8_hj_kj` : seul appelant `pico8_hj_kj_dispatch`, atteint par les descripteurs de `hj`/`pm` et, pour `mx`, par `hj->defaults["kj"]`. **Après ce commit, plus aucune instance ne l'atteint → forme struct seule.** `pico8_f_` : aussi `pq` de la classe `g_` (`probe.c:377`) et appelé par `pico8_ga_pq` (`planet_family.c`) ; `pico8_g__mm` : installé sur `g_` (`probe.c:382`) ; `pico8_ez`/`pico8_ei` : partagés avec `ga` → **les quatre restent à double forme** (jusqu'à `ga`, 1a.26, et `g_`, 1a.27). `pico8_qh_ly_dispatch` : `nr`/`fr` l'atteignent par `qh->defaults` → reste à double forme. |
| Tests qui lisent un `mx` ou les installations de `hj` | **`test_pico8_hj`** : `fold_heir` asserte `mx` **instance** (`:543`) et compare `kj`/`mm` à `class_function(pico8_hj_class(), …)` (`:546-547`) ; `test_hj_is_a_struct` compare `KJ`/`MM` du descripteur à `class_function(pico8_hj_class(), …)` (`:759-760`) et attend `mx` `INSTANCE` (`others`, `:738`) ; `test_recorded` reconnaît `mx` par `is_of` (identité de `px`, vaut pour une struct). **`test_pico8_pm`** : `kj`/`mm` comparés à `class_function(pico8_hj_class(), …)` (`:415-416`) ; `mx` attendu `INSTANCE` (`others`, `:697`). **`test_pico8_qh`** : `descendant_of` (`:501`) ne reconnaît `mx` qu'en instance → il disparaîtrait du rejeu `recorded` ; `others` final (`:775`). **`test_pico8_jo`** : `daughter_of` (`:422`). **`test_pico8_nl`** (`:593`), **`test_pico8_os`** (`:620`) : `mx` attendu `INSTANCE`. **`test_pico8_ie`** : `gx()` compare des classes ; ses asserts de `ldown` tolèrent une struct (`:558`, `:564`) → inchangé. `class_function` asserte la présence : **après le retrait des installations de `hj`, les comparaisons à `class_function(pico8_hj_class(), "kj"|"mm")` échoueraient** alors que le pointeur attendu (`pico8_hj_kj_dispatch`, `pico8_g__mm`) ne change pas. |
| Filet | 1 rejeu construit un `mx` : `events_recorded_1790198294` (3, 19 832 frames). Il les dessine (`hj:kj`, `ei` quand `mi>=2`), les fait émettre (4 500 particules), osciller, les relie (`kf` ×3, dont 2 acceptations `ht`/`kw.qe`), les compte (`gg`/`hb`/`ib`), les sonde, affiche leur panneau. |
| Durée | le rejeu seul de `events_recorded_1790198294` prend ~12 s à `-O1` sans repli par frame ; une caractérisation à `-O0` avec empreinte d'écran par frame dépasse 30 s → `UNIT_TESTS_SLOW`, comme `test_pico8_hj`/`test_pico8_pm`. |

## Solution

1. **Enum : inchangée.** `kf` y est déjà (`KF`).
2. **Struct `mx`**, publique dans `planet_family_trade.h` (qui inclut désormais `planet_family.h`) : `pico8_hj_t` en premier membre, puis **`ht`**. Aucun membre ne répète un nom de `hj`, `qh`, `jo` ou `ke`.
3. **Visibilités élargies, sans changement de corps** (dans le module de la famille, déclarées dans `planet_family.h`) : `pico8_hj_struct_init` (renvoyée publique par 1a.22), `pico8_hj_kj_dispatch` et `pico8_qh_ly_dispatch` (pour que le descripteur `static const` de `mx`, dans l'autre fichier, nomme **les mêmes pointeurs** que celui de `hj`). `pico8_hj_field`, `pico8_hj_set_field`, `pico8_qh_struct_init` restent `static` : `mx` les joint par `pico8_hj_struct_class()->field`/`->set_field`, comme `hj` joint ceux de `qh`.
4. **Défauts pris par REF, jamais re-parsés** : `pico8_planet_family_trade_init` déclare `mx` comme aujourd'hui (même spec, même parent, même nom de couverture), pose `pico8_class_set_struct_new(g_mx_class, …)`, **retire** l'installation `kf` de `g_mx_class->defaults`, puis lit **une fois** les 18 défauts de `mx` dans sa classe (`pico8_class_get_field`, assert « chaque défaut est une clé de la spec de `mx` ») vers un état statique `g_mx_defaults`, chaque entrée à l'offset de son membre : `nu` → `hj.qh.jo.nu` ; `hp`, `s`, `px`, `od` → membres de `qh` ; `rr` et les 11 d'émetteur → membres de `hj` ; `ht` → membre propre. `pico8_planet_family_trade_shutdown` remet cet état à nil avant de libérer la classe, sans libérer aucune table.
5. **Constructeur de `mx`**, dans l'ordre du walk du cart (`hj.pq`=`f_`, `jo.pq`, `ke.pq`) :
   1. alloue un `pico8_mx_t` ;
   2. `pico8_hj_struct_init(&self->hj, &g_mx_struct_class)` ;
   3. met `ht` à nil, puis copie les 18 défauts statiques de `mx` (ils écrasent les membres hérités) ;
   4. recopie la clé **`id`** (NULL permise) et **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise ; ne garde aucun pointeur sur la table ;
   5. enregistre `"mx"` au crochet de couverture ;
   6. `pico8_f_` (`qy`), `pico8_jo_pq` (`oe`, `lz(gb[5])`, `kw`), `pico8_ke_struct_pq`.
6. **Descripteur de `mx`** (`static const`) :
   - 16 entrées : **les mêmes pointeurs** que le descripteur de `hj`, sauf **`KF` = le dispatcher de `mx:kf`** ; `PH`, `KZ`, `RDOWN`, `MD`, `OL`, `PR`, `EB` NULL ;
   - **lecteur chaîné** : `ht` (nil = absent), sinon `pico8_hj_struct_class()->field` ;
   - **écrivain chaîné** : `ht`, sinon `pico8_hj_struct_class()->set_field` — **22 noms** : les 21 de `hj` plus `ht` ;
   - accesseur public `pico8_mx_struct_class()`.
7. **`mx:kf` passe à la forme struct seule** (feuille, méthode propre ; précédent `nl:ly`) : `pico8_mx_kf(pico8_mx_t *self, pico8_instance_t *sw)` — `sw` garde son type. Lit `self->ht` et `self->hj.qh.jo.kw` directement, écrit `self->ht` directement (nouvelle table vide, l'ancienne n'est jamais libérée : défaut de la classe ou table déjà remplacée), garde les mêmes gardes de nil ; `cp` (l'autre extrémité, une planète de toute forme) reste lu par `pico8_entity_get_field` ; appelle `pico8_jo_kf((pico8_instance_t *)self, sw)` (double forme, inchangée).
8. **Fermeture de `hj` (règle « Méthodes d'une parente convertie », dernière descendante)** :
   - `pico8_hj_kj` passe à la **forme struct seule** : `pico8_hj_kj(gfx, mem, pico8_hj_t *self, p)`, lit `self->qh.jo.mi` directement (nil ou non fixe → pas de `ei`, comme aujourd'hui), appelle `pico8_ei` et `pico8_jo_kj` (double forme) sur `self` ;
   - **retirés** de `pico8_planet_family_init` : `pico8_class_set_pq(g_hj_class, pico8_f_)`, les installations `"mm"` et `"kj"` de `g_hj_class->defaults`, et leur commentaire ;
   - **restent à double forme** (mesuré, partagés hors de `hj`) : `pico8_f_`, `pico8_g__mm`, `pico8_ez`, `pico8_ei` ; `pico8_qh_ly_dispatch` et les fonctions de `qh`/`jo` (`nr`, `fr`, `ga` encore instances).
9. **`pico8_mx_new`** rend `pico8_mx_t *`, même paramètre (aucun appelant) ; le commentaire « Same accessor and `_new` contract as the six classes » de `planet_family_trade.h` est corrigé pour `mx`.
10. **Tests** : nouveau `test_pico8_mx` (caractérisation, `UNIT_TESTS_SLOW`) ; `test_pico8_hj`, `test_pico8_pm`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os` : changements de forme seulement (voir Testing).
11. **Plan** : à la ligne 1a.23 de `docs/plan-3.md`, ajouter « Découvert à 1a.23 : … » avec au minimum : `ht` (défaut sans membre chez un ancêtre, réécrit sur l'instance par `mx:kf`) → membre de `mx` ; défauts `nu`/`hp`/`s`/`px`/`od`/`rr` et les 11 d'émetteur rangés dans les membres hérités ; descripteur aux 16 entrées de `hj` sauf `KF` = `mx:kf`, lecteur/écrivain chaînés (22 noms) ; `mx:kf` à forme struct seule, retiré de `mx->defaults` ; `pico8_hj_struct_init`, `pico8_hj_kj_dispatch`, `pico8_qh_ly_dispatch` publics ; `pico8_hj_kj` à forme struct seule, `pq`/`mm`/`kj` retirés de la classe `hj` ; `f_`, `g_.mm`, `ez`, `ei` restent à double forme (`g_`, `ga`) ; tests qui reconnaissent `mx` par son descripteur et nomment `kj`/`mm` par le descripteur de `hj`.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `mx` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `nr`, `fr`, `ga`, `g_` à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `mx` mesurée (constructions et clés par rejeu, défauts, champs écrits et par qui, noms lus, méthodes par nom, accès à la classe, tests qui le lisent), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux une struct `mx` dont le premier membre est `pico8_hj_t` et qui n'ajoute que `ht`, nom minifié conservé, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun membre de `mx` ne répète un nom d'un ancêtre, afin qu'un lecteur passant par un descripteur ancêtre voie la valeur du synthétiseur.
5. En tant que gardien du filet, je veux que `hp`=5, `s`=66, `od`=40, `px`, `nu`, `rr`, `fc`=0.3, `du`=5 et `k_` de `mx` écrasent les membres hérités, afin que `jo:pq` coure `lz(gb[5])`, que le synthétiseur émette ses propres particules et n'entre que dans `cg.jo`/`np`/`prober`.
6. En tant que gardien du filet, je veux qu'un synthétiseur garde `qh`=false de `hj` et `me`="kz", `dq`, `fp`, `dw`, `mi`, `ba` de `qh`, afin que ses lectures héritées ne changent pas.
7. En tant que gardien du filet, je veux qu'un synthétiseur construit par `ie:ldown` porte l'`id` que l'icône lui donne, afin que `events_recorded_1790198294` reste identique au pixel près.
8. En tant que mainteneur, je veux qu'une clé de construction autre que `id` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt (§7).
9. En tant que gardien du filet, je veux que le constructeur appelle `f_`, puis `jo:pq`, puis le `pq` de `ke`, afin que `qy`, `oe`, `hn`, `jy`, `kw` existent avant l'enregistrement.
10. En tant que gardien du filet, je veux que `mx:kf`, sur une liaison dont une ligne de `jy` a `gc`=0 et un `qe` de `ht`, écrive `kw.qe` et remplace `ht` par une table vide, puis appelle `jo:kf`, afin que la synthèse se déclenche une seule fois comme dans le cart.
11. En tant que gardien du filet, je veux que `ht` d'un synthétiseur neuf soit la REF de la table de la classe, et qu'après acceptation il soit une table neuve et vide, la table de la classe intacte, afin qu'un autre synthétiseur accepte encore.
12. En tant que gardien du filet, je veux que `dc` appelle `hj:kj`, que `ez` trouve `mm`, que `dg` trouve `ly` et que `mv(…,"kf")` trouve `mx:kf` sur une struct `mx`, afin qu'il se dessine, émette, oscille et synthétise comme avant.
13. En tant que mainteneur, je veux que les 15 autres méthodes d'un `mx` soient les mêmes pointeurs que celles d'un `hj`, afin qu'un synthétiseur et une ascension exécutent le même code.
14. En tant que mainteneur, je veux que `pico8_hj_kj` passe à la forme struct seule et que `pq`/`mm`/`kj` quittent la classe `hj`, afin qu'aucune voie morte ne subsiste une fois la dernière descendante convertie.
15. En tant qu'auteur des étapes 1a.26 et 1a.27, je veux que `f_`, `g_.mm`, `ez` et `ei` restent à double forme, afin que `ga` et `g_`, encore instances, émettent comme avant.
16. En tant qu'auteur de 1a.24 (`nr`), je veux que `pico8_qh_ly_dispatch` reste à double forme et devienne seulement public, afin que `nr`/`fr` l'atteignent encore par `qh->defaults` et que leur descripteur puisse le nommer.
17. En tant que mainteneur, je veux que `nu`, `px`, `ht`, `k_` d'un synthétiseur neuf soient la même REF que celles de `mx->defaults`, et `ba` celle de `qh`, afin qu'aucune table ne soit re-parsée ni dupliquée.
18. En tant que mainteneur, je veux que l'écrivain de `mx` accepte exactement les 21 noms de `hj` plus `ht`, afin qu'un nom hors de cette liste soit un arrêt.
19. En tant que mainteneur, je veux que le lecteur de `mx` délègue à celui de `hj`, afin qu'une fonction de `hj`, `qh`, `jo` ou des particules lise un synthétiseur sans savoir que c'en est un.
20. En tant que gardien de la couverture, je veux que le constructeur enregistre `"mx"` une fois par synthétiseur, afin que `class-coverage-net` soit identique ligne à ligne (`mx 3 1/15`).
21. En tant que vérificateur, je veux une caractérisation de `mx` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`rr`, `ba`, identité des tables, `ht` avant/après `kf`, forme d'une particule).
22. En tant que vérificateur, je veux un rapport `gcov` montrant chaque ligne ajoutée ou modifiée exécutée par la caractérisation, et 2 à 3 mutations ciblées sur ce que `gcov` ne prouve pas, afin de juger la preuve sans monter mon propre banc de mutations.
23. En tant que gardien des caractérisations, je veux que `test_pico8_hj`, `test_pico8_pm`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl` et `test_pico8_os` ne changent que leur façon de reconnaître un `mx` ou de nommer `kj`/`mm`, sans qu'aucune valeur figée ne bouge, afin que 1a.17 à 1a.22 restent des preuves.
24. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
25. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.23, afin que la liste des étapes dise ce que l'étape a réellement touché.
26. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
27. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
28. En tant que développeur, je veux que `make test` reste rapide, afin que le nouveau test à rejeu long tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : le module de la famille de planètes (visibilités, `pico8_hj_kj` struct seule, retrait des trois installations de `hj`) et son voisin `_trade` (struct, défauts, constructeur, descripteur, lecteur/écrivain, `mx:kf`, `pico8_mx_new`). `pico8_struct.h` (enum), `planet`, `particle`, `probe`, `research_icon`, `cart`, `economy*` : aucun changement.
- **`ht` membre propre** : défaut de la chaîne sans membre chez un ancêtre, et champ écrit par la méthode propre de la fille (§5). Pas un défaut muté : `mx:kf` réaffecte l'instance, jamais la table de la classe.
- **Défauts rangés dans les membres hérités quand le nom existe** : même piège qu'à 1a.18-1a.22 — un `mx.fc` propre laisserait `hj.fc`=0.5 visible au lecteur de `hj` que lisent les particules.
- **Descripteur `static const` qui nomme les pointeurs de `hj`** (G) : un initialiseur `static const` ne peut pas relire un autre descripteur ; le test vérifie l'égalité entrée par entrée. Cela exige `pico8_hj_kj_dispatch` et `pico8_qh_ly_dispatch` publics. Précédent : les 11 dispatchers de `jo` rendus publics à 1a.18. Rejeté : un dispatcher local à `_trade` qui appellerait `pico8_hj_kj` — pointeur différent, égalité perdue. Rejeté : déplacer `mx` dans `planet_family.c` — la ligne du plan situe `mx` dans `_trade`, et `nr`/`fr` y vivent.
- **Lecteur et écrivain chaînés par `pico8_hj_struct_class()->field/set_field`** (G) : évite de rendre publics `pico8_hj_field`/`pico8_hj_set_field` ; même forme que `hj` qui chaîne sur `pico8_qh_struct_class()`.
- **`mx:kf` à forme struct seule, retiré de `mx->defaults`** (G) : feuille ; précédent `nl:ly` (1a.19), `pm:pq` (1a.22). `sw` garde son type `pico8_instance_t *` (changement chirurgical ; le jc est déjà une struct et n'est lu que par `pico8_mv`).
- **`pico8_hj_kj` struct seule, installations `pq`/`mm`/`kj` retirées** : règle « Méthodes d'une parente convertie », `mx` étant la dernière descendante de `hj`. Mesuré : aucun autre chemin vers `pico8_hj_kj`. La classe `hj` reste enregistrée (§5, jusqu'à 1a.42) avec ses défauts de spec.
- **`f_`, `g_.mm`, `ez`, `ei` restent à double forme** (mesuré, contrairement à l'hypothèse « fonctions de `hj` ») : ce ne sont pas des fonctions propres de `hj` ; `f_` est aussi le `pq` de `g_` et appelé par `ga:pq`, `g_.mm` est la méthode de `g_`, `ez`/`ei` servent `ga`. Ils passeront à `ga` (1a.26) / `g_` (1a.27).
- **Constructeur `f_` → `jo:pq` → `ke:pq`** : ordre du walk du cart. Comme à 1a.21, `f_` et `jo:pq` sont permutables sans effet observable (mutant équivalent attendu) ; seul `ke:pq` en dernier est porteur (enregistrement dans `hv` après `oe`/`kw`).
- **Clé `id` seule** : seule mesurée (`ie:ldown`), comme `qh`, `os`, `hj`.
- **18 défauts copiés depuis la classe à l'init** (G), scalaires compris. Rejeté : les écrire en dur (deux sources).
- **`pk`=1 et `qh`=false** : membres présents (fixe, booléen faux), pas nil.
- **Sémantique de nil** : `ep`/`fo` présents-nil sur l'instance (TROU-9), relus par deux lecteurs qui testent présence et vérité : même branche. Pas d'écart.
- **Clés dynamiques** : aucune nouvelle.
- **TROU** : aucun écart attendu. Si l'implémenteur en trouve un, règle §7.
- **Commentaires** (CLAUDE.md) :
  - à ajouter : contrat de la struct `mx` (jamais libérée, TROU-7 ; un défaut dont le nom est un membre d'un ancêtre va dans ce membre ; `nu`/`px`/`ht`/`k_` sont les tables de la classe `mx`, empruntées, `ba` celle de `qh` ; `ht` réécrit par `mx:kf` en table neuve jamais libérée) ; contrat public de `pico8_hj_struct_init` (repris de son commentaire actuel) ; assert des clés ; contrat de `pico8_mx_new` ;
  - à retirer : le commentaire « mx, still an instance, reaches these three through `parent` … » avec les trois installations ;
  - à corriger si rendus faux : « Same accessor and `_new` contract as the six classes » (`planet_family_trade.h`), les commentaires de `test_pico8_hj`/`test_pico8_pm` qui disent `mx` instance ou « the functions hj's class still installs for `mx` » ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert (et `make e2e` au dernier). La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_mx` sur l'ancien code** (voir Testing), valeurs figées, dans `UNIT_TESTS_SLOW`. Dans le même morceau, **réécriture neutre** des comparaisons à `class_function(pico8_hj_class(), "kj"|"mm")` de `test_pico8_hj` et `test_pico8_pm` en comparaisons aux entrées `KJ`/`MM` de `pico8_hj_struct_class()` : vertes sur l'ancien code (même pointeur), elles ne dépendront plus des installations. Arrêt ici si `events_recorded_1790198294` ne construit pas exactement 3 `mx` ou si `ht` n'est pas écrit exactement 2 fois : mesure fausse, à corriger dans le PRD.
2. **Visibilités, sans conversion** : `pico8_hj_struct_init`, `pico8_hj_kj_dispatch`, `pico8_qh_ly_dispatch` publics. Suite verte, aucune valeur figée ne bouge.
3. **`mx` convertie** : struct, statiques de défauts, constructeur, descripteur, lecteur/écrivain, accesseur, `pico8_class_set_struct_new`, retrait de l'installation `kf`, `pico8_mx_kf` et `pico8_mx_new` typés ; `test_pico8_hj`, `test_pico8_pm`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os` relus (reconnaissance de `mx`). Toutes les caractérisations passent **sans changer leurs valeurs figées** ; les vérifications de forme s'ajoutent à `test_pico8_mx`.
4. **Fermeture de `hj`** : `pico8_hj_kj` struct seule ; retrait de `set_pq(g_hj_class, pico8_f_)` et des installations `mm`/`kj` avec leur commentaire. Suite et `make e2e` verts, valeurs figées identiques.
5. **Preuves** : rapport `gcov`, 2 à 3 mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.23 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF et des pointeurs de fonction, le contenu de `qy`, `ht` et `kw.qe`. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_mx`** (lié comme `test_pico8_pm`, sur le cart réel, dépendances Makefile sur `events_recorded_1790198294.json` et `empty_planet_cleared_events.json`) — **dans `UNIT_TESTS_SLOW`** (rejeu long, lancé par `make e2e`). Il lit **uniquement** par l'API à double forme et par `pico8_class_get_field(pico8_mx_class()/pico8_hj_class()/pico8_qh_class()/pico8_jo_class(), …)` pour les défauts de spec ; son code est le même avant et après, aux vérifications de forme près. Chaque scénario tourne dans un fils forké. **Toute présence se lit par un `lookup` qui confond nil et absent** (celui de `test_pico8_pm`) : `ep`/`fo` écrits nil restent présents sur l'instance (TROU-9), absents sur la struct.
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction dirigée** (partie lancée comme dans `test_pico8_hj`) : `build(pico8_mx_class())` par l'icône (`ie:ldown`), deux fois. Empreinte de `id`, `me`, `t`, `rr`, `qh`, `hp`, `s`, `od`, `py`, `ki`, `pn`, `pk`, `fc`, `du`, `qo`, `ro`, `qg`, `q_`, `dq`, `fp`, `dw`, `mi`, `hf`, `hs`, `fa`, `ma`, `ca` ; présence de `ht`, des 13 de `hj`, des 10 de `qh` et des 31 de `jo` selon leur état ; absence de `nl`, `qw`, `ng`, `fz`, `et` ; `qy` présent, liste vide, distincte d'un synthétiseur à l'autre ; présence des 16 méthodes et absence de `ph`, `kz`, `rdown`, `md`, `ol`, `pr`, `eb`. Appartenance et rang dans `hv`, `cv.kj`/`ev`/`hd`/`hk`/`hb`, `cg.jo`/`np`/`prober`, **absence de `cg.qh` et `cg.hj`**. **Identité** : `nu`, `px`, `ht`, `k_` = REF de `pico8_class_get_field(pico8_mx_class(), …)` ; `ba` = REF de la classe de `qh` ; `kq`, `em`, `ka`, `dz`, `cq`, `gq` = REF de la classe de `jo` ; `kw` ≠ REF de la classe de `jo` ; `hn` = `gb[5]`. Figée.
    2. **Comportement dirigé** : `dg` en `"kz"` puis `"ly"` → `ra.y` avant/après ; `mv(o,"mm")` sous graine `rnd` fixée → `p`, `v`, `l` de la particule ; `pico8_ez(o, n)` → taille de `qy` ; `kj` via `dc` avec `mi`=1 puis `mi`=2 posé par l'écrivain générique → empreinte d'écran et taille de `qy` ; `gg`, `ib` → résultats ; **`mx:kf`** : un `jc` construit par `pico8_class_lookup_named("jc")` entre une planète et le synthétiseur (recette à reprendre de `test_pico8_jc`/`slipway_drag`), dont l'autre extrémité porte dans `jy` une ligne `gc`=0, `qe`∈`ht` → `kw.qe`, `ht` ≠ REF de la classe et vide, table de la classe intacte (4 éléments), `oe` de `jo:kf` ; puis une seconde liaison → `ht` inchangé. Si la recette du `jc` s'avère hors de portée, la branche d'acceptation reste prouvée par le rejeu (2 acceptations) et `gcov`, et la limite est dite dans le commit. Figés.
    3. **Rejeu `events_recorded_1790198294`** (`--from-title`) : par frame, pour chaque `mx` de `hv` : rang, `id`, `me`, `t`, `ra` x/y, `p`, `mi`, `p_`, `kw.qe`/`kw.mu`, tailles de `oe`/`jy`/`cc`/`by`/`qy`, `m_`, **`ht` = REF de la classe (0/1) et sa taille** ; plus une empreinte de l'écran ; repliés en une empreinte de séquence figée, avec le nombre de `mx` vus (3), le nombre d'acceptations `ht` (2) et le nombre de frames (19 832).
  - **Forme** (ajoutée au morceau 3) : un synthétiseur construit par `ie:ldown` est `STRUCT` avec le descripteur de `mx` ; ses entrées sont **les mêmes** que celles de `pico8_hj_struct_class()` sauf `KF`, différente de celle de `hj` et de `jo` ; les 7 autres NULL ; `pico8_entity_set_field` passe pour les 21 noms de `hj` et `ht`, et asserte (dans un fils) pour `ng`, `fz`, `qw`, `nl` ; une clé de construction autre que `id`, ou positionnelle, asserte ; `pico8_mx_new(NULL)` rend une struct sans `id` mais avec `oe`, `kw` et `qy` ; `pico8_mx_class()->struct_new` posé ; `pico8_class_get_field(pico8_hj_class(), "kj"|"mm")` rend 0 et `pico8_hj_class()->pq` est NULL ; `nr`, `fr`, `ga` restent `INSTANCE`.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (`git archive` du commit candidat, supprimée après usage), compiler `test_pico8_mx` et les objets du moteur avec `--coverage`, le lancer, puis `gcov` sur les deux fichiers de la famille. **Piège** : les scénarios tournent dans des fils terminés par `_exit(0)`, qui saute l'écriture des compteurs ; dans la copie seulement, forcer `-include` d'un en-tête qui redéfinit `_exit(c)` en `(__gcov_dump(), _exit(c))` (vérifié avec gcc 13 : les lignes exécutées dans le fils sont comptées). Les branches d'assert atteintes seulement dans un fils qui avorte (`must_abort`) ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu et justifiées ainsi dans le message de commit. Le rapport liste, pour chaque ligne ajoutée ou modifiée par le diff de `src/`, son compte d'exécution ; une ligne à 0 est soit couverte par un test ajouté, soit justifiée dans le message de commit. Le rapport (résumé : lignes du diff, lignes exécutées, lignes à 0 avec leur justification) est collé dans le message de commit.
  - **2 à 3 mutations**, ciblées sur ce que `gcov` ne prouve pas, chacune rouge sur `test_pico8_mx` (ou sur la caractérisation qui la nomme), notées dans le commit :
    1. **valeur d'un défaut propre** : `fc` (ou `du`) de `mx` rangé dans une copie propre au lieu du membre de `hj` — jamais un défaut égal à celui de `hj` (`py`, `ki`, `pn`, `pk`, `qo`, `ro`, `qg`, `q_` : mutants équivalents) ;
    2. **entrée du descripteur remplacée** : `KF` = `pico8_jo_kf_dispatch` (la synthèse n'a plus lieu : `ht` et `kw.qe` inchangés) ;
    3. **ordre du constructeur** : `pico8_ke_struct_pq` appelé avant `pico8_jo_pq` (enregistrement avant `oe`/`kw`) — pas `f_` après `jo:pq`, équivalent attendu.
- **`test_pico8_hj`** : changements de forme, aucune valeur figée touchée — `fold_heir` accepte pour `mx` une struct au descripteur de `mx` ; ses comparaisons `kj`/`mm` et celles de `test_hj_is_a_struct` visent les entrées de `pico8_hj_struct_class()` (morceau 1) ; `mx` quitte `others`. `HEIRS_HASH`, `RECORDED_HASH`, comptes inchangés ; `is_of` reconnaît déjà la struct.
- **`test_pico8_pm`** : comparaisons `kj`/`mm` aux entrées de `pico8_hj_struct_class()` (morceau 1) ; `mx` quitte `others`. Empreintes inchangées.
- **`test_pico8_qh`** : `descendant_of` reconnaît une struct au descripteur de `mx` ; la boucle finale de `test_qh_is_a_struct` attend `mx` struct. **`test_pico8_jo`** : `daughter_of` idem. **`test_pico8_nl`**, **`test_pico8_os`** : `mx` retiré des descendantes attendues en `INSTANCE`. Aucune valeur figée touchée.
- **Tests existants** : `test_pico8_ie` (diff vide, ses asserts tolèrent une struct), `geoharvest_burst`, `qb`, `ga_export_panel_by`, `jc`, `nf`, `js`, `mz`, `bg`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage` : diff vide. Tous les tests de régression (§6) : diff vide, verts. Aucun ne construit un `mx` : aucune preuve rouge n'est due.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** : `test_pico8_pm` (structure, `lookup`, `must_abort`, rejeu depuis le titre), `test_pico8_hj` (`build` par l'icône, `fold_heir`, rejeu `recorded` avec `mx`), `test_pico8_nl` (feuille dont la méthode propre passe à la forme struct seule), `test_pico8_jc`/`slipway_drag` (construction d'un `jc`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `mx` n'est pas `mx  3  1/15`, `hj` n'est pas `hj  5  1/15`, `pm` n'est pas `pm  4  2/15`, ou si une autre ligne change. Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées/modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations (défaut propre, entrée du descripteur, ordre du constructeur) ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier : si l'assertion des clés ou celle de l'écrivain se déclenche sur un rejeu ou un test ; si un membre propre autre que `ht` est ajouté ; si un membre propre répète un nom d'un ancêtre ; si l'écrivain de `mx` accepte un nom hors des 21 de `hj` plus `ht`, ou si la liste des 21 de `hj`, des 20 de `qh` ou des 17 de `jo` change ; si l'enum change.
- `FAILED:` si une autre classe que `mx` est convertie ou change de forme (`nr`, `fr`, `ga`, `g_`, `ui`, …), ou si le comportement de `hj`, `pm`, `qh`, `nl` ou `os` change (valeur figée de leurs caractérisations, lignes de couverture).
- `FAILED:` si une fonction encore atteignable par une instance devient struct seule ou change de corps : `pico8_f_`, `pico8_g__mm`, `pico8_ez`, `pico8_ei`, `pico8_qh_ly_dispatch`, `pico8_na`, `pico8_qh_field`, `pico8_qh_set_field`, une fonction `pico8_jo_*` ; une installation `->defaults` de `qh`/`jo`/`g_` ou `pico8_class_set_pq(g_jo_class|g_probe_class, …)` retirée ou changée.
- `FAILED:` si `pico8_hj_kj` garde une lecture par `pico8_entity_get_field`/`pico8_instance_get_field` ou un paramètre `pico8_instance_t *`, ou si `pico8_class_set_pq(g_hj_class, …)` ou une installation dans `g_hj_class->defaults` subsiste.
- `FAILED:` si une entrée du descripteur de `mx` autre que `KF` n'est pas le même pointeur que l'entrée correspondante du descripteur de `hj`, ou si `KF` n'est pas le dispatcher de `mx:kf`.
- `FAILED:` si une table de défaut de `mx` est re-parsée depuis la spec, si `nu`/`px`/`ht`/`k_` d'un synthétiseur neuf ne sont pas les REF de `mx->defaults`, ou si `mx:kf` modifie en place la table `ht` de la classe.
- `FAILED:` si le constructeur de `mx` n'appelle pas `f_`, `jo:pq`, `ke:pq` avec `ke:pq` en dernier, ou si `pico8_mx_kf` garde une lecture/écriture de `ht` ou `kw` par `pico8_entity_get_field`/`set_field`, ou reste installé dans `mx->defaults`.
- `FAILED:` si un test de régression (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une valeur figée de `test_pico8_mx` a été changée après la conversion, ou une de `test_pico8_hj`, `test_pico8_pm`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`, `ie`, `geoharvest_burst`, `qb`, `jc`, `nf`, `js`, `mz`, `bg`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo` ; ou si ces six caractérisations changent au-delà de la reconnaissance de `mx`, des listes `others` et de la cible des comparaisons `kj`/`mm`.
- `FAILED:` si `test_pico8_mx` n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- la struct `mx` n'a que `pico8_hj_t` et `ht` ; son descripteur n'a que les 16 méthodes de `hj`, `KF` surchargée ;
- `pico8_hj_struct_init` est publique, appelée par les constructeurs de `hj`, `pm` et `mx` et par eux seuls ;
- grep : aucun `g_hj_class->defaults` ni `pico8_class_set_pq(g_hj_class` ; aucun `g_mx_class->defaults` ; `PICO8_CLASS_COVERAGE_NAME(g_mx_class, "mx")` et `PICO8_CLASS_COVERAGE_NAME(g_hj_class, "hj")` restent ; aucune nouvelle lecture de `->defaults` ;
- hors des deux modules de la famille, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_mx.c` (nouveau), `test_pico8_hj.c`, `test_pico8_pm.c`, `test_pico8_qh.c`, `test_pico8_jo.c`, `test_pico8_nl.c`, `test_pico8_os.c` et le `Makefile` (cible, `UNIT_TESTS`, `UNIT_TESTS_SLOW`) bougent ;
- la ligne 1a.23 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.23 ».

## Out of Scope

- `nr` (1a.24), `fr` (1a.25), `ga` (1a.26), `g_` (1a.27) : leurs structs, leurs `->defaults`.
- Le passage à la forme struct seule de `f_`, `g_.mm`, `ez`, `ei` (partagés avec `g_` et `ga`), de `pico8_qh_ly_dispatch` et des fonctions de `qh`/`jo` (`nr`, `fr`, `ga` encore instances).
- Rendre publics `pico8_hj_field`, `pico8_hj_set_field`, `pico8_qh_struct_init` (le dernier : à `nr`, 1a.24).
- Le type de `sw` dans `pico8_mx_kf` et de `cp` (lu par l'API à double forme).
- La suppression de la `pico8_class_t` de `hj`/`mx` (1a.42).
- Le typage C des membres (1b) ; archétypes et composant « émetteur de particules » (phase 2).
- Libérer un synthétiseur, sa liste `qy` ou les tables `ht` remplacées (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Dernière descendante de `hj`** : c'est l'étape qui solde la dette posée à 1a.21 et reportée à 1a.22. Des « fonctions de `hj` » supposées, seule `hj:kj` l'est vraiment ; `f_` et `g_.mm` sont installées sur `hj` mais appartiennent à tout émetteur (`g_`, `ga`), d'où leur maintien à double forme.
- **Première fille convertie hors du fichier de sa parente** : elle impose les trois visibilités (init de `hj`, dispatchers `kj` de `hj` et `ly` de `qh`) ; `nr` (1a.24) réutilisera `pico8_qh_ly_dispatch` public et devra rendre `pico8_qh_struct_init` publique.
- **Première feuille qui surcharge une méthode de l'enum héritée d'un ancêtre converti** (`KF`) : le descripteur n'est plus une copie exacte de celui de la parente ; le test vérifie les 15 égalités et la différence.
- **Les tests qui nommaient `kj`/`mm` par la classe `hj`** doivent passer au descripteur **avant** le retrait des installations (morceau 1), pour que ce retrait ne touche aucune ligne de test.
- **Premier PRD sous la règle `gcov`** : le piège `_exit` des fils forkés est à régler dans la copie jetable seulement, jamais dans les tests versionnés.
- Mesures : grep sur `src/`, `tests/`, `tools/`, `Makefile` à d0e2276 ; lecture de `pico8/lua.lua:998-1055,1182`, de `pico8_planet_family.c/h`, `pico8_planet_family_trade.c/h`, `pico8_particle.h`, `pico8_probe.c`, `pico8_bootstrap.c`, de `test_pico8_hj.c`, `test_pico8_pm.c`, `test_pico8_qh.c`, `test_pico8_jo.c`, `test_pico8_nl.c`, `test_pico8_os.c`, `test_pico8_ie.c`, et de `tools/class_coverage_net_expected.txt` ; sonde (110 sites distincts de lecture sur un `mx` exact, 19 noms écrits dont `ht` seul hors des 21 de `hj`, écritures nil `ep`/`fo`, 0 `pico8_instance_get_field`, clé `id` à chacune des 3 constructions ; `mx 3` dans `events_recorded_1790198294`, `0` dans les 14 autres ; tous en sortie 0) et essai du contournement `__gcov_dump` sur un programme jouet, dans des copies jetables du scratchpad, supprimées après usage.
