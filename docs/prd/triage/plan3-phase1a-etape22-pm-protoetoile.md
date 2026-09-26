---
title: "Plan 3, étape 1a.22 : `pm` (la protoétoile) devient une struct fille de `hj` avec 2 membres propres (`ng`, `fz`) ; son `pq` passe à la forme struct seule, l'initialisation de `hj` est coupée de son constructeur, les fonctions de `hj` restent à double forme pour `mx`"
description: "La protoétoile `pm` est encore une instance de la couche objet, construite 4 fois sur le filet (3 dans `session_events`, 1 dans `events_recorded_1790198294`) par la seule voie `jo:hu` → `kv[hp.cw]` avec les clés `p`, `ra`, `id`. Elle passe en struct `pico8_pm_t` (`pico8_hj_t` puis `ng`, `fz`), son descripteur reprend les 16 entrées de `hj` sans en ajouter, son `pq` (qui engendre un `ga`) devient struct seule et quitte la classe, et `pico8_hj_struct_init` est coupée du constructeur de `hj` ; les installations `pq`/`mm`/`kj` de `hj->defaults` restent pour `mx` (1a.23)."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.22 — `pm` : `hj` (`planet_family.c:358`), via `kv[hp.cw]` (`planet.c:337,351`).

(À HEAD 1676ec7, les lignes ont glissé avec l'ajout des structs `qh`, `nl`, `os`, `hj` et la reprise de `jo:hu` : la déclaration `g_pm_class = pico8_class_declare(g_hj_class, …)` est à `planet_family.c:818`, son nom de couverture à `:832`, son `pq` (`pico8_pm_pq`) posé à `:833`, son enregistrement nommé `"pm"` à `:834`, `pico8_pm_pq` à `:555`, `pico8_pm_new` à `:960`, le constructeur de `hj` (`pico8_hj_struct_new`) à `:524` ; la construction par `kv[hp.cw]` est `pico8_class_instantiate(target_class, overrides)` dans `pico8_jo_hu`, `planet.c:527` (clés remplies `:517-525`). La ligne ne nomme aucun test de régression.)

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. […]
>
> **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag […] ; seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) […]. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe.
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre […] reste à double forme […]. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante (pour `jo` : 1a.26).
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct : une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec.

Le §5 dit aussi, pour la couche : `pico8_pm_t` commence par `pico8_hj_t` (premier membre), qui commence par `pico8_qh_t`, `pico8_jo_t`, `pico8_ke_t`. `pm` n'a aucune descendante : c'est une feuille.

§6 : aucun test de régression nommé par la ligne ; aucun fichier de régression ne construit un `pm` (mesuré, voir plus bas). Les caractérisations qui lisent un `pm` (`test_pico8_hj`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os`) changent leur façon de le reconnaître, jamais une valeur figée. §7 : un commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (`pm  4  2/15`, `ga  7  3/15`, `hj  5  1/15`, `mx  3  1/15`, toutes les autres lignes inchangées) ; un site découvert couvert par une règle du §5 est ajouté à la ligne 1a.22, avec « découvert à 1a.22 » dans le message ; un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.21 ont posé (jusqu'à 516195e), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`). L'enum vaut `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`, `RDOWN`, `EB`, `HD`, `HB`, `GG`, `GV`, `HK`, `KF`, `CN`, `IB`, `LL`, `LY`, `MM`. `pico8_entity_get_field` sur une struct rend l'entrée du descripteur si le nom est dans l'enum et l'entrée non NULL, sinon le lecteur ; l'écrivain rend 0 pour un nom sans membre, et `pico8_entity_set_field` asserte alors (« a field under no rule of plan-3 section 5 — stop »).
- `pico8_ke_t` (`me`, `t`), `pico8_ke_struct_pq` ; `pico8_jo_t` (31 membres, dont `id`, `p`, `ra`), `pico8_jo_struct_init`, `pico8_jo_pq`, les 11 dispatchers publics de `jo`, écrivain de `jo` à 17 noms.
- `pico8_qh_t`, `pico8_qh_struct_init` (`static`), lecteur/écrivain de `qh` (20 noms), `pico8_qh_ly_dispatch`.
- **1a.19** (`docs/prd/done/plan3-phase1a-etape19-nl-plaque-commerciale.md`, 7719e74) : coupe de `pico8_qh_struct_init` hors du constructeur de `qh` à sa première fille, `static` tant que les filles vivent dans le même fichier ; la méthode propre d'une feuille (`nl:ly`) quitte `defaults` et passe à la forme struct seule, signature typée (`pico8_nl_ly(pico8_nl_t *, …)`), lecture directe des membres.
- **1a.21** (`docs/prd/done/plan3-phase1a-etape21-hj-ascension.md`, 516195e) : `pico8_hj_t` (`pico8_qh_t` puis `rr`, `py`, `ki`, `pn`, `pk`, `fc`, `k_`, `du`, `qo`, `ro`, `qg`, `q_`, `qy`), `g_hj_defaults` (18 défauts rangés à l'offset du membre, hérité quand le nom existe), `g_hj_members`, lecteur `pico8_hj_field` et écrivain `pico8_hj_set_field` (`qy` puis les 20 de `qh` : 21 noms), `static` ; descripteur `g_hj_struct_class` (16 entrées : 14 de `qh`, `KJ` = `pico8_hj_kj_dispatch`, `MM` = `pico8_g__mm`) ; constructeur qui initialise d'un seul tenant (**la coupe `pico8_hj_struct_init` a été renvoyée à cette étape**) ; `pico8_class_set_pq(g_hj_class, pico8_f_)` et les installations `mm`/`kj` de `hj->defaults` gardées pour `pm` et `mx`, avec un commentaire « Remove them at mx's conversion (1a.23), hj's last descendant, not at pm's » ; `pico8_hj_kj` à double forme (`pico8_instance_t *self`).
- Aides partagées déjà à double forme, qu'atteint un `pm` : toutes les fonctions de `jo`, `pico8_qh_ly_dispatch`/`pico8_na`, `pico8_hj_kj`, `pico8_f_` (garde aiguillée sur le tag), `pico8_ez`, `pico8_ei`, `pico8_g__mm`, `pico8_dl`.

### Ce que fait le cart

`pico8/lua.lua` :

```
:86-96     kv:jr : kv[rj.fz]=rj ; __call walks ko=rj, rj.iq, … calling each distinct ko.pq
           (pm: pm.pq, then hj.pq=f_, then qh.pq=jo.pq (inherited, called once), then ke.pq)
:451-456   jo:hu(hp) : if hp.cw then self.p_=true ; self=kv[hp.cw]({p=self.p,ra=self.ra,id=self.id}) …
           … fm:nx(fp) rn(hp.rn or 55) self:kd()
:998-1016  hj=qh:jr([[ … ]]) ; hj.pq=f_ ; hj.mm=g_.mm ; function hj:kj(p) … end
:1017-1030 pm=hj:jr([[ fz="pm", nu=o("jo","np","prober"), hp=3,px=o("protostar",c=13),
             me="ly",s=98, py=0.06,ki=9, pn=8,pk=f, ng=v(1,0), du=2, fc=1,
             k_=o(1,2,4,9,10,7), qo=1.02,ro=-0.0525, qg=0.0525,q_=1.02, ]])
:1030-1035 function pm:pq() ga(ob([[ qo=0.97,ro=-0.1255, qg=0.1255,q_=0.97,
             k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1), iu=-0.009,py=9,ms=1.5, ]],{ra=self.ra+self.ng})) end
```

Points décisifs :

- **Une seule construction d'un `pm`** : `jo:hu` sur une recette dont `cw` vaut `"pm"`, par `kv["pm"]` (côté C : `pico8_class_lookup_named("pm")` puis `pico8_class_instantiate`), clés **`p`, `ra`, `id`** (chacune posée seulement si l'ancienne planète l'a). `pm` n'est pas dans `gx[]` ni construit par `ie:ldown`.
- **Ordre du walk** : `pm:pq` **d'abord** (il construit un `ga` à `ra + ng`, qui s'enregistre dans `hv` **avant** la protoétoile), puis `f_` (`qy`), puis `jo.pq` (`oe`, `lz(gb[3])`), puis `ke.pq` (`lt`).
- **`pm:pq` ne lit que `ra` et `ng`** et n'écrit rien sur la protoétoile ; `ra + ng` est un vecteur neuf : `ng` n'est jamais modifié en place.
- **`me="ly"`** : `dg` appelle `ly` (`qh:ly` → `na`) sur une protoétoile à chaque frame, jamais `kz`. **`pk=f`** : `g_.mm` ne fait pas tourner la vitesse. **`hp=3`** : `jo:pq` court `lz(gb[3])`.
- **`nu` de `pm` ne contient ni `"hj"` ni `"qh"`** : un `pm` entre dans `cg.jo`/`np`/`prober` seulement.
- **Hérités de `hj` sans surcharge** : `rr`=`"ascension"`, `qh`=false, `od`=60, et les trois fonctions `f_`, `mm`, `kj`.
- **`fz="pm"`** est lu par `kv:jr` sur la classe ; sur une instance, il se lit aujourd'hui par la chaîne (présent, `"pm"`).
- **Tables de défaut de `pm`** (`nu`, `px`, `k_`, `ng`) : jamais écrites en place ni réaffectées (grep du cart). Aucun défaut muté.

### Emprise réelle de `pm` (mesurée sur 1676ec7)

**Méthode.** Grep de `src/`, `tests/`, `tools/`, `Makefile`, lecture des fonctions, puis **sonde de mesure** sur une copie `git archive HEAD` jetable du scratchpad (supprimée après mesure) : `pico8_entity_get_field`/`set_field`, `pico8_mv`, `pico8_instance_get_field`, `pico8_instance_fields`, `pico8_class_instantiate` deviennent, hors de `pico8_entity.c`/`pico8_class.c`, des macros (`-include`) qui journalisent (opération, nom, fichier:ligne, nature de la valeur, « absent » et « écriture nil » à part ; clés de construction) chaque accès à une instance dont la classe exacte est `pico8_pm_class()`, sortie à tampon ligne (un premier essai à tampon plein par unité de compilation entrelaçait les lignes et sous-comptait les constructions : à ne pas refaire). Binaire de couverture sondé relancé sur chacun des 15 rejeux du filet, un par un (`--from-title` pour les 4 `RECORDINGS`), tous en sortie 0. Les accès internes à `pico8_entity.c` (`lt`, `he`, `dc` → `kj`/`ev`) et le walk `pq` de `pico8_class.c` sont lus dans le code.

| Aspect | Mesure |
|---|---|
| Constructions d'un `pm` | **une seule** voie : `jo:hu` (`planet.c:527`), clés **`p`=REF, `ra`=REF, `id`=chaîne**, dans cet ordre, les trois présentes à chaque fois. **4 sur le filet** : **3 dans `session_events`**, **1 dans `events_recorded_1790198294`** (`--from-title` les deux) ; **0** dans les 13 autres rejeux (vérifié rejeu par rejeu). `pico8_pm_new` : aucun appelant (`src/`, `tests/`, `tools/`). Hors jeu : `test_pico8_hj` (`test_heirs` : `p`, `ra`, `id` ; `test_hj_is_a_struct` : `id` seul), `test_pico8_nl`/`test_pico8_os` (`id` seul). |
| Défauts propres de `pm` (18) | `fz`=`"pm"`, `nu`, `hp`=3, `px`, `me`=`"ly"`, `s`=98, `py`=0.06, `ki`=9, `pn`=8, `pk`=false, `ng`=`v(1,0)`, `du`=2, `fc`=1, `k_`, `qo`=1.02, `ro`=-0.0525, `qg`=0.0525, `q_`=1.02. Hérités non surchargés : de `hj` `rr`, `qh`=false, `od`=60 ; de `qh` `dq`, `hf`, `fp`, `dw`, `mi`=1, `ba` ; de `jo` `hs`, `fa`, `kq`, `ma`, `ka`, `dz`, `cq`, `kw`, `gq`, `ca`, `em` ; de `ke` `t`. |
| Défauts qui tombent sur un membre hérité (16) | `me` (`pico8_ke_t`) ; `nu` (`pico8_jo_t`) ; `hp`, `px`, `s` (`pico8_qh_t`) ; `py`, `ki`, `pn`, `pk`, `du`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_` (`pico8_hj_t`). **Les 2 autres, `ng` et `fz`, n'ont de membre chez aucun ancêtre.** |
| Défauts mutés | aucun. |
| Champs écrits sur un `pm` (sonde) | 15 noms : `oe` (`planet.c:401`), `hn` (`:441`), `ep` (nil, `:442`), `jy` (`:451`), `kw` (`:458`), `ma` (`:481`, et `entity_geom.c:79` 48 078 fois), `lq` (`planet_display.c:120`), `fo` (nil, `planet_display.c:572` ; REF, `probe.c:185`, un `pm` cible d'une sonde), `fx` (`planet_trade.c:94`), `kr` (`economy.c:242`), `cc`, `by`, `mi` (`economy_tiers.c:92,107,218`), `m_` (`ui.c:308,313`), `qy` (`particle.c:69`, une fois par `pm`, par `f_`). Écritures nil : `ep` et `fo`. **Tous parmi les 21 noms de `hj`** ; aucun `et`, `ra`, `p`, `p_` écrit (une protoétoile n'est jamais sondée ni détruite sur le filet ; `p`/`ra` viennent des clés de construction). Aucune écriture par `pm:pq`. |
| Membres propres qui en résultent (2) | après `pico8_hj_t` : **`ng`**, **`fz`** — deux défauts de la chaîne, sans membre chez un ancêtre. Aucun champ écrit n'en ajoute. |
| Noms lus sur un `pm` (sonde, 109 sites distincts `(op, nom, fichier:ligne)` lectures et `mv`) | présents : `me`, `t`, `id`, `ra`, `p`, `oe`, `jy`, `kw`, `hn`, `kq`, `fa`, `ma`, `s`, `hp` (`planet.c:403`), `hf` (faux), `hs` (`economy_score.c:31`), `qh` (faux, `economy_score.c:39`, celui de `hj`), `mi` (dont `planet_family.c:420`, `hj:kj`, 143 763 fois), `lq`, `cc`, `by`, `fx`, `kr`, `ca`, `gq`, `px` (`planet_display.c:177`), **`ng`** (`planet_family.c:56`, 4 fois : `pm:pq`), et les champs de particules `fc`, `qo`, `qg`, `ro`, `q_` (`particle.c:17`), `k_` (`:41`), `qy` (`:33`), `pn`, `du`, `py`, `ki`, `t` (`probe.c:41`), `pk` (`probe.c:49`, faux). Absents (aujourd'hui comme demain) : `qw` (`planet_display.c:46`), `nk` (`:506`), `fg`/`ql` (`ui.c:54`), `mt` (`slipway.c:45`), `qs` (`probe.c:219`), `nl` (`economy_tiers.c:28`), `jh` (`:240`), `p_` (`bootstrap.c:92`), `eo`/`ff`/`hi` (`msgbox.c:480,487,519`), et `by`/`cc`/`fx` avant leur première écriture. **Présent-nil sur l'instance (TROU-9)** : `fo` 2 352 fois (`planet_display.c:552`) et `ep` 3 fois (`msgbox.c:462`) ; sur la struct, un membre nil est absent — les deux lecteurs testent présence **et** vérité, même branche dans les deux cas : pas d'écart. **Jamais lus sur le filet** : `fz`, `rr`, `od`, `dq`, `fp`, `dw`, `ba`. |
| Méthodes appelées par nom sur un `pm` | `hd` (`ui.c:296`), `ly` (`bootstrap.c:89`, par `dg` sur `me`="ly", 143 755 fois ; `kz` jamais), `mm` (`particle.c:83`, 48 078), `gg` (`economy_score.c:95`), `hb` (`economy.c:277`), `hk` (`economy.c:156`), `ib` (`economy_tiers.c:104`), `cn` (`planet_trade.c:305`, `planet_family_trade.c:206`), `kf` (`slipway.c:177-178`), `gv`/`ldown` (`ui.c:411`, `slipway.c:110`), `kj`/`ev` (`dc`). **Toutes dans l'enum, toutes des entrées du descripteur de `hj`** ; `pm` n'en a aucune propre. |
| Lectures à simple forme | **aucune** : zéro `pico8_instance_get_field` sur un `pm` ; `pico8_instance_fields` seulement aux sites déjà aiguillés (`bootstrap.c:101`, `particle.c:63`). Côté `jo:hu`, `pico8_jo_kd(working_self)` et `pico8_hy_nx` sont déjà à double forme. |
| `->defaults` / classe de `pm` | trois accès, tous dans `pico8_planet_family_init` : `PICO8_CLASS_COVERAGE_NAME` (`:832`), `pico8_class_set_pq(g_pm_class, pico8_pm_pq)` (`:833`), `pico8_class_register_named("pm", …)` (`:834`). Aucune lecture de `g_pm_class->defaults`. `jo:hu` résout la classe par `lookup_named(cw)`. |
| `pm:pq` | `pico8_pm_pq(pico8_instance_t *)`, lit `ra` et `ng` par `pico8_pf_field_vec` (`entity_get_field`), construit l'overrides (`ra = ra + ng`, tas) et appelle `pico8_ga_new` ; seul appelant : le walk `pq` de `pico8_class_instantiate`. |
| Tests qui lisent un `pm` | **`test_pico8_hj`** : `fold_heir` (`:533-549`) asserte un `pm` **instance** de sa classe ; `test_recorded` le reconnaît par `is_of` (identité de `px`, vaut aussi pour une struct) ; la boucle finale de `test_hj_is_a_struct` (`:782-789`) attend `pm` `INSTANCE`. **`test_pico8_qh`** : `descendant_of` (`:501-529`) ne reconnaît que `nl`/`os`/`hj` en struct → le `pm` disparaîtrait des rejeux `session` (`required = {NR, FR, PM}`) et `recorded` (`{HJ, MX, PM}`) ; la boucle finale de `test_qh_is_a_struct` (`:770`) attend `pm` instance. **`test_pico8_jo`** : `daughter_of` (`:422-451`) → le `pm` disparaîtrait du rejeu `session` (`required = {QH, NR, FR, PM, GA}`). **`test_pico8_nl`** (`:593`) et **`test_pico8_os`** (`:620`) : `pm` attendu `INSTANCE` parmi les « autres descendantes ». `test_pico8_geoharvest_burst`, `test_pico8_qb` (comparaisons à la classe `ga`), `test_pico8_ga_export_panel_by` (`ga` seul) : inchangés. Aucun test de régression (§6) ne construit un `pm`. |
| Filet | 2 rejeux construisent un `pm` : `session_events` (3, 116 045 frames) et `events_recorded_1790198294` (1, 19 832 frames). Ils le dessinent (`hj:kj`, `ei` quand `mi>=2`), le font émettre (`mm`, 48 078 particules), osciller (`ly`/`na`), le relient (`kf`, `cn`), le comptent dans `gg`/`hb`/`ib`, et affichent son panneau (`px`, `jo:gh`). Le `ga` qu'il engendre (couverture `ga 7 3/15`) est dessiné à côté. |

## Solution

1. **Enum : inchangée.** `pm` n'a aucune méthode appelée par nom qui lui soit propre ; son `pq` n'est pas une méthode de l'enum.
2. **Struct `pm`**, publique dans `planet_family.h` : `pico8_hj_t` en premier membre, puis les 2 membres `pico8_value_t` **`ng`**, **`fz`**. **Aucun membre ne répète un nom de `hj`, `qh`, `jo` ou `ke`.**
3. **Coupe `pico8_hj_struct_init(self, cls)`** (renvoyée ici par 1a.21, comme `pico8_qh_struct_init` à 1a.19) : `pico8_qh_struct_init(&self->qh, cls)`, les 13 membres de `hj` à nil, les 18 défauts statiques de `hj`. Ne lit aucune clé, ne court ni `pq` ni `lt`, n'enregistre aucune couverture. **`static`** : `pm` vit dans le même fichier ; elle devient publique à la première descendante hors du fichier (`mx`, 1a.23). Le constructeur de `hj` l'appelle ; son comportement ne change pas.
4. **Défauts pris par REF, jamais re-parsés** : `pico8_planet_family_init` déclare `pm` comme aujourd'hui (même spec, même parent, même nom de couverture, même enregistrement nommé `"pm"`), pose `pico8_class_set_struct_new(g_pm_class, …)`, puis lit **une fois** les 18 défauts de `pm` dans sa `pico8_class_t` (`pico8_class_get_field`, assert « chaque défaut est une clé de la spec de `pm` ») vers un état statique `g_pm_defaults`, chaque entrée rangée à l'offset de son membre : `me` → `hj.qh.jo.ke.me` ; `nu` → `hj.qh.jo.nu` ; `hp`, `px`, `s` → membres de `qh` ; `py`, `ki`, `pn`, `pk`, `du`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_` → membres de `hj` ; `ng`, `fz` → membres propres. `pico8_planet_family_shutdown` remet cet état à nil avant de libérer la classe, sans libérer aucune table.
5. **`pm:pq` passe à la forme struct seule** (feuille : aucune descendante ne l'atteint ; précédent `nl:ly` à 1a.19) : `pico8_pm_pq(pico8_pm_t *self)`, lit `self->hj.qh.jo.ra` et `self->ng` directement (REF nulle → vecteur nul, comme `pico8_pf_value_vec` aujourd'hui), même overrides, même `pico8_ga_new`. **`pico8_class_set_pq(g_pm_class, pico8_pm_pq)` est retiré** : avec le constructeur de struct posé, `pico8_class_instantiate` délègue avant tout walk, et aucune classe ne descend de `pm`.
6. **Constructeur de `pm`**, dans l'ordre du walk du cart (`pm.pq`, `hj.pq`=`f_`, `jo.pq`, `ke.pq`) :
   1. alloue un `pico8_pm_t` ;
   2. `pico8_hj_struct_init(&self->hj, &g_pm_struct_class)` ;
   3. met `ng`, `fz` à nil, puis copie les 18 défauts statiques de `pm` (ils écrasent `me`, `nu`, `hp`, `px`, `s` et les 11 d'émetteur hérités) ;
   4. recopie les clés nommées **`p`, `ra`, `id`** de la table reçue (NULL permise, chacune facultative) dans `hj.qh.jo.p`/`.ra`/`.id` et **asserte** (message porteur, arrêt §7) sur toute autre clé, positionnelle comprise ; ne garde aucun pointeur sur la table ;
   5. enregistre `"pm"` au crochet de couverture ;
   6. appelle **`pico8_pm_pq`** (le `ga` naît et s'enregistre avant la protoétoile) ;
   7. appelle **`pico8_f_`** (écrit `qy` par l'écrivain) ;
   8. appelle `pico8_jo_pq` (`oe`, puis `hp`=3 → `lz(gb[3])`) ;
   9. appelle `pico8_ke_struct_pq`.
7. **Descripteur de `pm`** (`static const`) :
   - les 16 entrées = **les mêmes pointeurs** que le descripteur de `hj` (`KJ` = `pico8_hj_kj_dispatch`, `MM` = `pico8_g__mm`, `LY` = `pico8_qh_ly_dispatch`, les 11 de `jo`, `HQ`/`EV`/`EY`) ; `PH`, `KZ`, `RDOWN`, `MD`, `OL`, `PR`, `EB` NULL ;
   - **lecteur, chaîné** : `ng`, `fz` (nil = absent), sinon `pico8_hj_field` ;
   - **écrivain = `pico8_hj_set_field`, même pointeur** : `pm` accepte exactement les 21 noms de `hj` ; `ng` et `fz`, écrits par aucun site, sont refusés. Les écrivains de `hj` et de `qh` **ne changent pas** ;
   - accesseur public `pico8_pm_struct_class()`, comme `pico8_hj_struct_class()`.
8. **Fonctions de `hj`, de `qh` et de `jo` : aucune ne change de corps ni de signature.** `pico8_hj_kj` reste à double forme, `pico8_class_set_pq(g_hj_class, pico8_f_)` et les installations `mm`/`kj` de `hj->defaults` restent, avec leur commentaire, pour `mx` (1a.23) ; `pico8_qh_struct_init`, `pico8_hj_field`, `pico8_hj_set_field` restent `static`.
9. **`pico8_pm_new`** rend `pico8_pm_t *`, même paramètre (aucun appelant), et quitte le bloc de commentaire des `_new` non convertis. `jo:hu`, `lookup_named`, `ga`, `planet.c`, `cart.c` : inchangés ; la délégation de `pico8_class_instantiate` suffit.
10. **Tests** : nouveau `test_pico8_pm` (caractérisation) ; `test_pico8_hj`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os` : reconnaissance de `pm` par son descripteur (voir Testing).
11. **Plan** : à la ligne 1a.22 de `docs/plan-3.md`, ajouter « Découvert à 1a.22 : … » avec au minimum : `ng` et `fz` (défauts sans membre chez un ancêtre ; `fz` lu seulement sur la classe) → membres de `pm` ; défauts `me`/`nu`/`hp`/`px`/`s` et les 11 d'émetteur rangés dans les membres hérités (`ke`, `jo`, `qh`, `hj`) ; descripteur aux 16 entrées de `hj`, écrivain de `hj` (même pointeur) ; `pm:pq` à forme struct seule, retiré de la classe ; constructeur dans l'ordre `pm:pq` (le `ga` s'enregistre avant), `f_`, `jo:pq`, `ke:pq` ; clés `p`/`ra`/`id` ; `pico8_hj_struct_init` coupée, `static` jusqu'à `mx` ; `test_pico8_hj`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os` reconnaissent `pm` par son descripteur.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `pm` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `mx`, `nr`, `fr`, `ga` à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `pm` mesurée (constructions et clés par rejeu, défauts, champs écrits et par qui, noms lus absents, présents-nil ou jamais lus, méthodes par nom, accès à la classe, tests qui le lisent), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux une struct `pm` dont le premier membre est `pico8_hj_t` et qui n'ajoute que `ng` et `fz`, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux qu'aucun membre de `pm` ne répète un nom de `hj`, `qh`, `jo` ou `ke`, afin qu'un lecteur passant par un descripteur ancêtre voie la valeur de la protoétoile.
5. En tant que gardien du filet, je veux que `me`="ly", `hp`=3, `s`=98, `px`, `nu` et les 11 défauts d'émetteur de `pm` écrasent les membres hérités, afin que `dg` appelle `ly`, que `jo:pq` coure `lz(gb[3])`, que la protoétoile émette ses propres particules et n'entre que dans `cg.jo`/`np`/`prober`.
6. En tant que gardien du filet, je veux qu'une protoétoile garde `rr`, `qh`=false et `od` de `hj`, `dq`/`fp`/`dw`/`mi`/`ba` de `qh`, afin que ses lectures héritées ne changent pas.
7. En tant que gardien du filet, je veux qu'une protoétoile construite par `jo:hu` porte `p`, `ra`, `id` de la planète qu'elle remplace, afin que `session_events` et `events_recorded_1790198294` restent identiques au pixel près.
8. En tant que mainteneur, je veux qu'une clé de construction hors de `p`, `ra`, `id` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt (§7).
9. En tant que gardien du filet, je veux que le constructeur appelle `pm:pq`, puis `f_`, puis `jo:pq`, puis le `pq` de `ke`, afin que le `ga` engendré précède la protoétoile dans `hv` et que `qy`, `oe`, `hn`, `jy`, `kw` existent avant l'enregistrement.
10. En tant que gardien du filet, je veux que le `ga` engendré naisse à `ra + ng` avec les mêmes surcharges qu'aujourd'hui, afin que sa gerbe soit identique.
11. En tant que gardien du filet, je veux que `dc` appelle `hj:kj`, que `ez` trouve `mm` et que `dg` trouve `ly` sur une struct `pm`, afin que la protoétoile se dessine, émette et oscille comme avant.
12. En tant que mainteneur, je veux que les 16 méthodes d'un `pm` soient les mêmes pointeurs que celles d'un `hj`, afin qu'une protoétoile et une ascension exécutent le même code.
13. En tant qu'auteur de l'étape 1a.23, je veux que `pq`, `mm` et `kj` restent installés dans `hj->defaults` et `pico8_hj_kj` à double forme, afin que les synthétiseurs, encore instances, émettent et se dessinent comme avant.
14. En tant qu'auteur de l'étape 1a.23, je veux une `pico8_hj_struct_init` coupée et testée par deux constructeurs, afin que `mx` l'appelle en la rendant seulement publique.
15. En tant que mainteneur, je veux que `pm:pq` passe à la forme struct seule et quitte la classe, afin qu'aucune voie morte ne subsiste pour une feuille convertie.
16. En tant que mainteneur, je veux que `nu`, `px`, `k_`, `ng` d'une protoétoile soient la même REF que celles de `pm->defaults`, et `ba` celle de `qh`, afin qu'aucune table ne soit re-parsée ni dupliquée.
17. En tant que mainteneur, je veux que l'écrivain de `pm` soit celui de `hj`, afin que la liste des 21 noms reste unique et qu'un nom hors d'elle (`ng`, `fz` compris) soit un arrêt.
18. En tant que mainteneur, je veux que le lecteur de `pm` délègue à celui de `hj`, afin qu'une fonction de `hj`, `qh`, `jo` ou des particules lise une protoétoile sans savoir que c'en est une.
19. En tant que mainteneur, je veux que `fz` reste lisible sur une protoétoile (`"pm"`), afin que la lecture d'un défaut de la chaîne ne change pas de résultat.
20. En tant que gardien de la couverture, je veux que le constructeur enregistre `"pm"` une fois par protoétoile, afin que `class-coverage-net` soit identique ligne à ligne (`pm 4 2/15`, `ga 7 3/15`).
21. En tant que vérificateur, je veux une caractérisation de `pm` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`fz`, `rr`, `od`, `ba`, `da`/`bb`, identité des tables, forme d'une particule, `ga` engendré).
22. En tant que gardien des caractérisations, je veux que `test_pico8_hj`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl` et `test_pico8_os` ne changent que leur façon de reconnaître un `pm`, sans qu'aucune valeur figée ne bouge, afin que 1a.17 à 1a.21 restent des preuves.
23. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
24. En tant que lecteur du plan, je veux que les sites découverts soient inscrits à la ligne 1a.22, afin que la liste des étapes dise ce que l'étape a réellement touché.
25. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés.
26. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : module de la famille de planètes seulement (struct `pm`, coupe `pico8_hj_struct_init`, statiques de défauts, constructeur, descripteur, lecteur, accesseur, `pico8_pm_pq` et `pico8_pm_new` typés, retrait du `set_pq` de `pm`, commentaires d'en-tête). `pico8_struct.h` (enum), le module de la planète (`jo:hu`), `particle`, `probe`, `cart`, `planet_family_trade` (`mx`), `economy*` : aucun changement.
- **`ng` et `fz` membres propres** (G) : ce sont des défauts de la chaîne (§5, « chaque champ de sa chaîne […] devient un membre »), sans membre chez un ancêtre. `fz` n'est lu que sur la classe par `kv:jr`/`pico8_class_declare`, mais une instance le lit aujourd'hui par la chaîne ; le membre garde ce résultat. Précédent : `rr` de `nl` et de `hj`, lu seulement sur la classe, a eu son membre. Rejeté : omettre `fz` — une lecture qui rendrait « absent » là où l'instance rend `"pm"`.
- **Défauts rangés dans les membres hérités quand le nom existe** (`me`, `nu`, `hp`, `px`, `s`, les 11 d'émetteur) : même piège qu'à 1a.18-1a.21 — un `pm.py` propre laisserait `hj.py`=0.12 visible au lecteur de `hj` que lisent `g_.mm` et les particules.
- **Aucune entrée d'enum, aucune surcharge** : le descripteur de `pm` nomme les 16 mêmes pointeurs que celui de `hj` (un initialiseur `static const` ne peut pas relire un autre descripteur ; le test vérifie l'égalité).
- **Écrivain de `pm` = `pico8_hj_set_field`, même pointeur** (G) : aucun nom écrit sur un `pm` n'est hors des 21 de `hj` (mesuré : 15). Précédent : `os` a pris le lecteur et l'écrivain de `qh` tels quels. Rejeté : un écrivain chaîné qui n'ajouterait rien.
- **`pm:pq` à forme struct seule, retiré de la classe** (G) : `pm` est une feuille ; la règle « Méthodes d'une parente convertie » ne la retient pas. Précédent : `nl:ly` à 1a.19 (installation retirée, signature typée, membres lus directement). Garder `pico8_class_set_pq(g_pm_class, …)` laisserait une voie que rien n'emprunte (délégation avant le walk).
- **Ordre du constructeur `pm:pq` → `f_` → `jo:pq` → `ke:pq`** : ordre du walk du cart. Ici **non équivalent** à une autre permutation : `pm:pq` avant `ke:pq` place le `ga` avant la protoétoile dans `hv` (ordre de dessin et de parcours) ; le test le fige.
- **Clés `p`, `ra`, `id` chacune facultative** : `jo:hu` ne pose une clé que si l'ancienne planète l'a ; les tests construisent aussi avec `id` seul.
- **Coupe `pico8_hj_struct_init` `static`** : même règle qu'à 1a.19 (publique à la première descendante hors du fichier, `mx`). Rejeté : la rendre publique maintenant — aucun appelant hors du fichier.
- **`pico8_hj_kj`, `f_` et les installations de `hj->defaults` restent** : `mx` (1a.23) les atteint (mesuré à 1a.21 : 24 639 lectures de `mi` à `planet_family.c:420` sur un `mx`). Le commentaire en place (« not at pm's ») reste vrai.
- **18 défauts copiés depuis la classe à l'init** (G), scalaires compris, comme `jo`, `qh`, `nl`, `os`, `hj`. Rejeté : les écrire en dur (deux sources).
- **`pk`=false et `qh`=false sont des membres présents** (booléen faux), pas nil : même lecture que l'instance.
- **Sémantique de nil** : un membre nil est absent. Mesuré : deux écritures nil sur un `pm`, `ep` et `fo`, relues « présent nil » sur l'instance (TROU-9) par `msgbox.c:462` et `planet_display.c:552`, qui testent présence et vérité : même branche. Pas d'écart.
- **Clés dynamiques** : aucune nouvelle.
- **TROU** : aucun écart attendu. Si l'implémenteur en trouve un, règle §7 (préserver, `TROU-N`, `/* TROU-N */`).
- **Commentaires** (CLAUDE.md) :
  - à ajouter : contrat de la struct `pm` (jamais libérée, TROU-7 ; un défaut de `pm` dont le nom est un membre d'un ancêtre va dans ce membre ; `nu`/`px`/`k_`/`ng` sont les tables de la classe `pm`, `ba` celle de `qh`, empruntées) ; contrat de `pico8_hj_struct_init` (sur le modèle de celui de `pico8_qh_struct_init`) ; assert des clés ; contrat de `pico8_pm_new` ;
  - à corriger si rendus faux : le bloc « Each `_new` below … `overrides` is copied, not kept » et « `pm` and `ga` are the only classes here whose spec sets `fz` » de `planet_family.h` (ne couvre plus `pm` comme instance) ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` et `make frames-check` verts. La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_pm` sur l'ancien code** (voir Testing), valeurs figées. Arrêt ici si `session_events` ne construit pas exactement 3 `pm` et `events_recorded_1790198294` exactement 1 : mesure fausse, à corriger dans le PRD, pas un test durci.
2. **Coupe `pico8_hj_struct_init`, sans conversion** : le constructeur de `hj` l'appelle ; suite verte, toutes les caractérisations (`hj`, `qh`, `jo`, `nl`, `os`, `pm`…) inchangées, `frames-check` conforme : preuve que la coupe est neutre.
3. **`pm` convertie** : struct, statiques de défauts, constructeur, descripteur, lecteur, accesseur, `pico8_class_set_struct_new`, retrait du `set_pq`, `pico8_pm_pq` et `pico8_pm_new` typés ; `test_pico8_hj`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os` relus (voir Testing). Toutes les caractérisations passent **sans changer leurs valeurs figées** ; les vérifications de forme s'ajoutent à `test_pico8_pm`.
4. **Preuves** : mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.22 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`/`pico8_da`/`pico8_bb`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF et des pointeurs de fonction, le contenu de `qy`, le `ga` engendré. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_pm`** (dans `make unit`, lié comme `test_pico8_hj`, sur le cart réel, dépendances Makefile sur `session_events.json`, `events_recorded_1790198294.json` et `empty_planet_cleared_events.json`). Il lit **uniquement** par l'API à double forme et par `pico8_class_get_field(pico8_pm_class()/pico8_hj_class()/pico8_qh_class()/pico8_jo_class()/pico8_ga_class(), …)`, qui existent avant et après ; son code est le même avant et après, aux vérifications de forme près. Chaque scénario tourne dans un fils forké ; les rejeux depuis le titre tournent à côté des scénarios dirigés. **Toute présence se lit par un `lookup` qui confond nil et absent** (celui de `test_pico8_hj`) : sur l'instance, `ep` et `fo` écrits nil restent présents (TROU-9), sur la struct ils sont absents ; une empreinte de présence brute changerait à la conversion sans écart réel.
  - **Caractérisation, figée sur l'ancien code** :
    1. **Construction dirigée** (partie lancée comme dans `test_pico8_hj`) : `pico8_class_instantiate(pico8_class_lookup_named("pm"), {p, ra, id})` comme `jo:hu`, deux fois. Empreinte de `id`, `me`, `t`, `fz`, `rr`, `qh`, `hp`, `s`, `od`, `py`, `ki`, `pn`, `pk`, `fc`, `du`, `qo`, `ro`, `qg`, `q_`, `dq`, `fp`, `dw`, `mi`, `hf`, `hs`, `fa`, `ma`, `ca`, `ng` (x, y) ; présence des 2 membres propres, des 13 de `hj`, des 10 de `qh` et des 31 de `jo` selon leur état ; absence de `nl`, `qw`, `ht`, `et` ; `qy` présent, liste vide, distincte d'une protoétoile à l'autre ; présence des 16 méthodes et absence de `ph`, `kz`, `rdown`, `md`, `ol`, `pr`, `eb` ; `kj`, `mm` lus sur la protoétoile = ceux de `pico8_class_get_field(pico8_hj_class(), …)` ; `ly` = celui de `qh`. **Le `ga` engendré** : rang dans `hv` **juste avant** la protoétoile, son `ra` = `ra + ng`, taille de son `qy`, `mo.ma`. Appartenance et rang dans `hv`, `cv.kj`/`ev`/`hd`/`hk`/`hb`, `cg.jo`/`np`/`prober`, **absence de `cg.qh` et `cg.hj`**. **Identité** : `nu`, `px`, `k_`, `ng` = REF de `pico8_class_get_field(pico8_pm_class(), …)` ; `ba` = REF de la classe de `qh` ; `kq`, `em`, `ka`, `dz`, `cq`, `gq` = REF de la classe de `jo` ; `kw` ≠ REF de la classe de `jo` ; `hn` = `gb[3]`. Figée.
    2. **Comportement dirigé** : `dg` en `"ly"` → `ra.y` avant/après ; `mv(o,"mm")` sous une graine `rnd` fixée → `p`, `v`, `l` de la particule (`pk`=false : pas de rotation) ; `pico8_ez(o, n)` sous graine fixée → taille de `qy` ; `kj` via `dc` avec `mi`=1 puis `mi`=2 posé par l'écrivain générique → empreinte de l'écran et taille de `qy` ; `gg`, `ib` (`bb`), `pico8_da` (branche `qh` faux) → résultats ; construction avec `id` seul → `ra` absent, `ga` à `ng`. Figés.
    3. **Rejeux du filet** `session_events` et `events_recorded_1790198294` (`--from-title`) : par frame, pour chaque `pm` de `hv` : rang, `id`, `me`, `t`, `ra` x/y, `p`, `mi`, `p_`, `kw.qe`/`kw.mu`, taille de `oe`/`jy`/`cc`/`by`/`qy`, `m_` ; plus une empreinte de l'écran ; repliés en une empreinte de séquence figée par rejeu, avec le nombre de `pm` vus (3, puis 1) et le nombre de frames (116 045 et 19 832 selon `test_pico8_qh`).
  - **Forme** (ajoutée au morceau 3) : la protoétoile construite par `lookup_named("pm")` est `STRUCT` avec le descripteur de `pm` ; ses 16 entrées sont **les mêmes** que celles de `pico8_hj_struct_class()`, les 7 autres NULL ; `set_field` est celui de `hj`, `field` diffère ; `pico8_entity_set_field` passe pour les 21 noms de `hj` et asserte (dans un fils) pour `ng`, `fz`, `rr`, `py`, `hp`, `id`, `qw`, `ht` ; une clé de construction hors de `p`/`ra`/`id`, ou positionnelle, asserte ; `pico8_pm_new(NULL)` rend une struct sans `id`/`ra` mais avec `oe` et `qy` ; `pico8_pm_class()` n'a plus de `pq` propre ; `mx`, `nr`, `fr`, `ga` restent `INSTANCE`.
  - **Mutations** (au moins 12, notées dans le commit, mutants équivalents justifiés) : une entrée reprise absente (`KJ`, `MM`, `LY`, `EV`) ; `KJ` = celui de `jo` ; `pico8_pm_pq` omis (aucun `ga`) ; `pico8_pm_pq` appelé après `pico8_ke_struct_pq` (le `ga` suit la protoétoile dans `hv`) ; `f_` omis (`qy` absent) ; `pico8_jo_pq` omis ; `me` non appliqué (`kz`, pas d'oscillation) ; `hp` laissé à 2 (`lz(gb[2])`) ; `pk` laissé à 1 (rotation) ; un défaut d'émetteur rangé dans un membre propre au lieu de celui de `hj` ; `ng` copié au lieu de la REF ; `ra` ou `id` non recopié ; lecteur qui ne délègue pas à `hj` ; protoétoile initialisée avec le descripteur de `hj` ; constructeur de struct non posé ; couverture non enregistrée (attrapée par `class-coverage-net` : `pm 0 0/15`) ; installation `kj`/`mm` retirée de `hj->defaults` (attrapée par la partie « descendantes » de `test_pico8_hj` sur `mx` et par `frames-check`). Équivalent attendu : `f_` appelé après `pico8_jo_pq` (aucun des deux ne lit ce que l'autre écrit), comme à 1a.21.
- **`test_pico8_hj`** (caractérisation de 1a.21) : deux changements de forme, aucune valeur figée touchée — `fold_heir` accepte pour `pm` une struct au descripteur de `pm` (l'assert « instance de sa classe » reste pour `mx`) ; la boucle finale de `test_hj_is_a_struct` retire `pm` des classes attendues en `INSTANCE` (ou l'attend en struct). `HEIRS_HASH`, `RECORDED_HASH` et les comptes (5, 1, 3) restent identiques ; `is_of` (identité de `px`) reconnaît déjà la struct.
- **`test_pico8_qh`** : `descendant_of` reconnaît aussi une struct au descripteur de `pm` ; la boucle finale de `test_qh_is_a_struct` asserte « `pm` est une struct au descripteur de `pm` » comme pour `nl`/`os`/`hj`. `fold_descendant` inchangé (`pm` n'a pas `qw`). Toutes les empreintes identiques.
- **`test_pico8_jo`** : `daughter_of` reconnaît aussi une struct au descripteur de `pm`. Toutes les empreintes identiques.
- **`test_pico8_nl`** et **`test_pico8_os`** : `pm` retiré de la liste des descendantes attendues en `INSTANCE`. Aucune valeur figée touchée.
- **Tests existants** : `test_pico8_ie`, `geoharvest_burst`, `qb`, `ga_export_panel_by`, `jc`, `nf`, `js`, `mz`, `bg`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage` : diff vide, valeurs figées identiques. Tous les tests de régression (§6) : diff vide, verts. Aucun ne construit un `pm` : aucune preuve rouge n'est due par ce commit.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** : `test_pico8_hj` (`test_heirs` : construction comme `jo:hu` ; rejeu `recorded` avec `pm`/`mx` ; forme ; mutations), `test_pico8_nl` (feuille dont la méthode propre passe à la forme struct seule), `test_pico8_os` (descripteur et lecteur/écrivain repris tels quels), `test_pico8_qh` (rejeu `session` depuis le titre, `descendant_of`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `pm` n'est pas `pm  4  2/15`, `ga` n'est pas `ga  7  3/15`, `hj` n'est pas `hj  5  1/15`, `mx` n'est pas `mx  3  1/15`, ou si une autre ligne change. Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier : si l'assertion des clés de construction ou celle de l'écrivain se déclenche sur un rejeu ou un test ; si un membre propre hors de `ng`, `fz` est ajouté ; si un membre propre répète un nom de `hj`, `qh`, `jo` ou `ke` ; si l'écrivain de `pm` accepte un nom hors des 21 de `hj`, ou si la liste des 21 de `hj`, des 20 de `qh` ou des 17 de `jo` change ; si l'enum change.
- `FAILED:` si une autre classe que `pm` est convertie ou change de forme (`mx`, `nr`, `fr`, `ga`, `g_`, `ui`, `lq`, `hy`, `hp`, `pj`, `fv`, `trade`…), ou si le comportement de `hj`, `qh`, `nl` ou `os` change (valeur figée de leurs caractérisations, lignes de couverture).
- `FAILED:` si une fonction de `hj`, `qh` ou `jo` atteignable par `mx` devient struct seule : `pico8_class_set_pq(g_hj_class, pico8_f_)` ou une des installations `mm`/`kj` de `hj->defaults` retirée ou changée ; une des installations `->defaults` de `qh`/`jo` ou `pico8_class_set_pq(g_jo_class, …)` retirée ou changée ; un corps de `pico8_hj_kj`, `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_g__mm`, `pico8_qh_ly_dispatch`, `pico8_hj_field`, `pico8_hj_set_field`, `pico8_qh_field`, `pico8_qh_set_field`, `pico8_na` ou d'une fonction `pico8_jo_*` modifié ; la signature de `pico8_hj_kj` changée ; une entrée du descripteur de `pm` qui n'est pas le même pointeur que l'entrée correspondante du descripteur de `hj`.
- `FAILED:` si une table de défaut de `pm` est re-parsée depuis la spec (un second `pico8_ob(` ou `pico8_class_declare` sur un fragment de la spec de `pm`), si `nu`/`px`/`k_`/`ng` d'une protoétoile neuve ne sont pas les REF de `pm->defaults`, ou si `ba` n'est pas la REF de la classe `qh`.
- `FAILED:` si le constructeur de `pm` n'appelle pas `pm:pq`, `f_`, `jo:pq`, `ke:pq` dans cet ordre (le `ga` engendré doit précéder la protoétoile dans `hv`), ou si `pico8_pm_pq` garde une lecture par `pico8_entity_get_field`/`pico8_instance_get_field` ou reste installé par `pico8_class_set_pq`.
- `FAILED:` si un test de régression (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une valeur figée de `test_pico8_pm` a été changée après la conversion, ou une de `test_pico8_hj`, `test_pico8_qh`, `test_pico8_jo`, `test_pico8_nl`, `test_pico8_os` (empreintes, comptes, frames), `ie`, `geoharvest_burst`, `qb`, `jc`, `nf`, `js`, `mz`, `bg`, `bg_kj`, `fq`, `iv`, `mh`, `je`, `mo` ; si `test_pico8_hj` change au-delà de `fold_heir` et de la boucle finale de `test_hj_is_a_struct`, `test_pico8_qh` au-delà de `descendant_of` et de la boucle finale de `test_qh_is_a_struct`, `test_pico8_jo` au-delà de `daughter_of`, `test_pico8_nl`/`test_pico8_os` au-delà de la liste des descendantes attendues en instance.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.

Critères de réussite (en plus) :

- la struct `pm` n'a que `pico8_hj_t`, `ng` et `fz` ; le descripteur n'a que les 16 méthodes de `hj` ; l'écrivain est celui de `hj` ;
- `pico8_hj_struct_init` existe, `static`, appelée par les constructeurs de `hj` et de `pm` et par eux seuls ;
- grep : les trois accès `g_hj_class` de `pico8_planet_family_init` (`set_pq`, `mm`, `kj`) sont intacts ; `pico8_class_register_named("pm", …)` et `PICO8_CLASS_COVERAGE_NAME(g_pm_class, "pm")` restent ; aucun `pico8_class_set_pq(g_pm_class` ; aucune nouvelle lecture de `->defaults` ;
- hors du module de la famille, le diff de `src/` est vide ; dans `tests/`, seuls `test_pico8_pm.c` (nouveau), `test_pico8_hj.c`, `test_pico8_qh.c`, `test_pico8_jo.c`, `test_pico8_nl.c`, `test_pico8_os.c` et le `Makefile` (cible et `unit`) bougent ;
- la ligne 1a.22 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.22 ».

## Out of Scope

- `mx` (1a.23), `nr` (1a.24), `fr` (1a.25), `ga` (1a.26) : leurs structs, leurs `->defaults`, leurs membres propres (`ht`…). Le `ga` engendré par `pm:pq` reste une instance construite par `pico8_ga_new`.
- Rendre `pico8_hj_struct_init`, `pico8_hj_field` ou `pico8_hj_set_field` publics : à `mx` (1a.23, autre fichier).
- Le passage à la forme struct seule de `pico8_hj_kj` et le retrait de `pq`/`mm`/`kj` de `hj->defaults` : au commit de `mx`, dernière descendante de `hj`.
- Le passage à la forme struct seule des fonctions de `qh`, de `jo` et des aides de particules (`f_`, `ez`, `ei`, `g_.mm`, partagées avec `g_` et `ga`).
- `jo:hu` et `kv[hp.cw]` (`lookup_named`) : aucun changement ; `hp` (1a.33).
- Le typage C des membres (1b) ; archétypes et composant « émetteur de particules » (phase 2).
- Libérer une protoétoile, sa liste `qy` ou le vecteur de tas de `pm:pq` (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Première feuille sous `hj`, dernière descendante non** : `pm` n'a pas de fille, mais `hj` en garde une (`mx`) ; les installations de `hj->defaults` survivent donc à ce commit, et le commentaire « not at pm's » posé à 1a.21 le dit déjà.
- **Premier constructeur qui engendre une autre entité avant de s'enregistrer** : `pm:pq` construit un `ga` au milieu du walk. L'ordre du constructeur n'est pas cosmétique ici ; la caractérisation dirigée et les deux rejeux le fixent.
- **Première classe à `fz` convertie** : `fz` reste le nom sous lequel la classe est enregistrée ; le membre ne fait que conserver la lecture sur l'instance.
- **Le filet garde `pm` sur deux rejeux** : `session_events` (3) et `events_recorded_1790198294` (1) ; un `me` non appliqué, un `ly` manquant ou un `ga` mal placé y cassent `frames-check`. `fz`, `rr`, `od`, `ba`, `da`/`bb` et l'identité des tables n'y sont pas visibles : d'où la caractérisation dirigée.
- **`fo`/`ep` présent-nil** : les seules différences instance/struct mesurées sur un `pm` (TROU-9) tombent sur des lecteurs qui ne les distinguent pas ; notées pour que l'implémenteur ne les « corrige » pas.
- Mesures : grep sur `src/`, `tests/`, `tools/`, `Makefile` à 1676ec7 ; lecture de `pico8/lua.lua:86-96,451-456,998-1035`, de `pico8_planet_family.c/h`, `pico8_planet.c` (`jo:hu`), `pico8_class.c` (`instantiate`, registres), `pico8_ke.h`, `pico8_msgbox.c:462`, de `test_pico8_hj.c`, `test_pico8_qh.c`, `test_pico8_jo.c`, `test_pico8_nl.c`, `test_pico8_os.c`, `test_pico8_geoharvest_burst.c`, et de `tools/class_coverage_net_expected.txt` ; sonde (109 sites distincts de lecture/`mv` sur un `pm` exact, 15 noms écrits tous parmi les 21 de `hj`, écritures nil `ep`/`fo`, 0 `pico8_instance_get_field` sur `pm`, clés `p`/`ra`/`id` à chacune des 4 constructions) et comptes rejeu par rejeu (`pm 3` dans `session_events`, `1` dans `events_recorded_1790198294`, `0` dans les 13 autres ; tous en sortie 0) dans une copie `git archive` jetable du scratchpad, supprimée après mesure.
