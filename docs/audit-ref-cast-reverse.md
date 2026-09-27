# Audit des casts REF en direction inverse (table ordonnée lue comme liste)

Table de classement site par site produite par
[`docs/prd/triage/reverse-direction-ref-cast-audit.md`](prd/triage/reverse-direction-ref-cast-audit.md).
Outillage :
[`src/pico8_ref_audit.h`](../src/pico8_ref_audit.h),
[`tools/ref_audit_report.sh`](../tools/ref_audit_report.sh), cible `make ref-audit`.

## Ce que « direction inverse » veut dire

`pico8_list_t` et `pico8_ob_table_t` ont exactement la même en-tête —
`{tag(4) ; 4 octets de padding ; pointeur(8) ; count(4) ; capacity(4)}` — mais
pas les mêmes éléments : `pico8_value_t` fait 16 octets, `pico8_ob_entry_t`
en fait 32 (une clé de 16 puis une valeur de 16). D'où l'asymétrie entre les
deux sens de confusion :

- **Liste lue comme table** (sens déjà traité, quatre bugs) : parcours à pas
  double, sortie d'allocation, `heap-buffer-overflow` bruyant.
- **Table lue comme liste** (ce document) : parcours à pas moitié, jamais de
  dépassement. `count` et `capacity` se relisent *justes* puisqu'ils sont au
  même offset ; seuls les éléments sont faux —
  `items[0]` = la clé de l'entrée 0 réinterprétée en valeur (un
  `PICO8_OB_KEY_INDEX` valant 0 se relit en `PICO8_VALUE_FIXED`, et le
  pointeur `name` de la clé devient les bits du nombre), `items[1]` = la
  vraie valeur de l'entrée 0, `items[2]` = la clé de l'entrée 1, etc. Aucun
  sanitizer ne voit rien. Le symptôme est un nombre faux ou une ligne vide.

Conséquence directe, et c'est elle qui explique le résultat de cet audit :
**pour une table VIDE, les deux lectures sont indiscernables** (`count` = 0
des deux côtés). Un défaut de ce type ne devient visible qu'à partir de la
première entrée.

## Méthode

Les deux moitiés sont exigées par le PRD, parce que l'audit de pré-Phase 5 a
échoué en n'en utilisant qu'une.

**Statique.** Chaque conversion « valeur REF → `pico8_list_t*` » de `src/`
passe désormais par une seule écriture, `PICO8_REF_AS_LIST` /
`PICO8_REF_AS_LIST_CONST` (`src/pico8_ref_audit.h`), qui en build ordinaire
*est* le cast simple d'avant. L'inventaire devient un `grep` et ne peut plus
être incomplet par oubli. Pour chaque site, les producteurs du champ lu ont
été énumérés par trois recherches distinctes, pas une :

1. les écritures littérales du nom (`set_name(..., "oe", ...)`) ;
2. les écritures à nom dynamique — il y en a 12 dans `src/`, dont la seule
   générique est `pico8_class_instantiate` recopiant une table
   d'`overrides` ; toutes les tables d'`overrides` construites dans `src/`
   ont été listées et aucune ne porte l'un des champs audités hors
   `lp`/`rl` ;
3. **les chaînes de spec de classe**, c'est-à-dire tous les `X=o(...)` que
   `ob()` parse en `pico8_ob_table_t`. C'est cette troisième recherche qui a
   trouvé le seul cas non sûr, et c'est exactement celle que l'audit
   précédent n'avait pas faite.

**Dynamique.** `make ref-audit` compile le moteur dans un quatrième arbre
d'objets (`tmp/obj-refaudit`, `-DPICO8_REF_AUDIT` ; cf. le commentaire du
Makefile sur l'interdiction de partager un objet instrumenté) et produit
huit binaires. Deux canaux :

- **par site** : chaque `PICO8_REF_AS_LIST` compte, pour son `__FILE__`/
  `__LINE__`, les tags réellement reçus (LIST / OB_TABLE / INSTANCE /
  UNKNOWN / NULL) et les vide à la sortie du processus ;
- **à l'entrée des accesseurs** : `pico8_list_add/del/all/foreach/free` et
  `pico8_ob_table_set_*/get_*/remove_index/free` vérifient le tag de leur
  argument. Ce second canal existe parce que le premier ne voit que les
  conversions que l'audit a lui-même trouvées ; celui-ci attrape un objet
  mal typé arrivé par n'importe quel chemin, y compris un chemin écrit
  après cet audit.

Scénarios rejoués : les six `events.json` du dépôt via le pilote headless,
plus `test_pico8_playability`, `test_pico8_slipway_drag`,
`test_pico8_empty_planet_cleared`, `test_pico8_planet_gh_row_lq`,
`test_pico8_data_init`, `test_pico8_planet_kj_dispatch` et
`test_pico8_ga_export_panel_by`.

**Un site qu'aucune des deux méthodes n'atteint est reporté NON COUVERT, pas
sûr.** C'est la distinction que l'audit précédent n'a pas faite.

**Contrôle négatif de l'instrumentation** (pour que « 0 table observée » ne
soit pas une mesure vide) : un programme jetable, compilé avec
`-DPICO8_REF_AUDIT`, passe une `pico8_ob_table_t` non vide dans
`PICO8_REF_AS_LIST_CONST` puis dans `pico8_list_add`. Les deux canaux
répondent —
`SITE ... list=0 ob_table=1` pour le premier,
`MISMATCH pico8_list_add expected=0x9E5C1157 actual=0x9E5C0B7A` pour le
second. Les deux canaux voient donc bien ce qu'ils prétendent voir.

**Limite connue, à savoir avant de croire un NON COUVERT** : la vidange se
fait dans un handler `atexit`, et `abort()` n'exécute pas les handlers
`atexit`. Un binaire qui meurt sur un `assert()` raté (ou sur un rapport de
sanitizer) ne contribue donc **rien du tout**, pas même les sites atteints
avant sa mort, et tous ses sites se relisent en NON COUVERT.
`tools/ref_audit_report.sh` imprime pour cette raison le code de sortie de
chaque binaire : un code non nul rend sa part des chiffres de couverture
sans valeur.

## Résultat

| | |
|---|---|
| Sites de cast REF → liste dans `src/` | **47** |
| Sûrs par construction (producteur nommé en commentaire) | **45** |
| Ambigus (plusieurs producteurs, dont un fabriquant une table) | **1** — corrigé chez le producteur |
| Sûrs par le tag lui-même (branche LIST d'un dispatch) | **1** |
| Réellement faux aujourd'hui | **0** |
| Atteints par au moins un scénario | **36** |
| NON COUVERTS | **11** |
| Sites ayant reçu autre chose qu'une liste au rejeu | **0** |
| Incohérences de tag signalées par le canal accesseurs | **0** |

Un 48ᵉ `(pico8_list_t *)` subsiste dans `src/` et n'est **pas** un site de
cast REF : `pico8_la_append` (`src/pico8_collect.c`) caste le `void *ctx` de
`pico8_list_foreach`, qui est le `&t` que `pico8_la` lui passe deux lignes
plus bas. Examiné, commenté sur place, délibérément laissé en cast explicite.

## Le seul cas ambigu : le champ `by`

`by` est lu comme liste sur **six** sites (`pico8_jo_ib` ×2,
`pico8_jo_hb`, puis via les trois `field_list` privés : `pico8_jo_gh`
(`src/pico8_planet_display.c`), `pico8_db` (`src/pico8_economy_score.c`) et
`pico8_dh` (`src/pico8_economy_tiers.c`)). Cinq
de ses producteurs fabriquent une vraie liste ; le sixième, la chaîne de spec
de la classe `ga` (`by=o(),`, `src/pico8_planet_family.c`), fabriquait une
`pico8_ob_table_t`. Les six sites lisaient donc bel et bien une table comme
une liste sur toute instance `ga`.

Ce n'était pas un bug visible, et c'est le point : la table que `ob()`
construit pour `by=o()` est vide (`count=0`, `capacity=0`, `entries=NULL`,
vérifié), et `count` est au même offset dans les deux structs, donc les six
lectures donnaient déjà 0 — la bonne réponse, par le mauvais type. Ce qui
restait était une REF dont le tag annonçait OB_TABLE pendant que tout le
programme la traitait comme une liste : l'invariant non écrit, exactement
la forme qui avait fait classer `js:pq`'s `lq` sûr.

**Chiffrage de l'option « corriger chez le producteur », demandé par le
PRD.** Deux variantes :

- *Gardes site par site* : six sites, chacun un `switch` sur le tag avec ses
  quatre cas et une conversion `pico8_ob_table_from_list` inverse à inventer
  (il n'existe pas de pont table→liste positionnel partagé ; trois copies
  privées de `positional_values` existent déjà dans `src/`, une quatrième
  serait à écrire ou à promouvoir). Environ 60 à 80 lignes réparties sur
  cinq fichiers, plus six occasions de diverger.
- *Correction chez le producteur* : un bloc de ~10 lignes dans
  `pico8_planet_family_init()`, calqué mot pour mot sur ce que
  `pico8_ui_init()` fait déjà pour `le=o()` et `pico8_hy_pq()` pour
  `invented=o()`. Les six sites deviennent sûrs par construction, il n'y a
  plus qu'un producteur à nommer, et le précédent existe déjà deux fois dans
  le port.

La seconde a été retenue. Aucun comportement ne change (voir ci-dessus), et
l'aliasing ne change pas non plus : le défaut de classe est un seul objet
partagé par toutes les instances `ga` avant comme après, ce que le vrai Lua
fait aussi puisque `qj({},X)` copie la référence.

Recensement exhaustif fait à cette occasion : parmi les 93 champs `X=o(...)`
des chaînes de spec du port, **trois seulement** sont aussi lus comme liste
quelque part — `le` (déjà corrigé chez `pico8_ui_init`), `invented` (déjà
corrigé chez `pico8_hy_pq`) et `by`. Le troisième et dernier est maintenant
traité.

## Les 11 sites NON COUVERTS

Aucun scénario du dépôt ne les atteint. Leur classement statique est donné,
mais il n'est adossé à aucune mesure.

| Site | Fonction | Champ | Classement statique |
|---|---|---|---|
| `pico8_collect.c:53` | `pico8_ft_name` | bucket de `rc[nt]` | **Aucun producteur nommable** : cette fonction n'a *aucun appelant* dans `src/` ni `tests/`. Voir « divergence » plus bas. |
| `pico8_economy.c:402` | `pico8_hy_kt` | `invented` | Sûr — `pico8_hy_pq` |
| `pico8_menu.c:209` | `pico8_cr` | `invented` | Sûr — `pico8_hy_pq` |
| `pico8_menu.c:243` | `pico8_cr` | `invented` | Sûr — `pico8_hy_pq` |
| `pico8_planet_family.c:301` | `pico8_ga_kj` | `qy` | Sûr — `pico8_f_` |
| `pico8_planet_family_trade.c:90` | `pico8_nr_gv` | `oe` | Sûr — `pico8_jo_pq` |
| `pico8_planet_family_trade.c:168` | `pico8_nr_hb` | `oe` | Sûr — `pico8_jo_pq` |
| `pico8_planet_family_trade.c:214` | `pico8_nr_cn` | `oe` | Sûr — `pico8_jo_pq` |
| `pico8_planet_family_trade.c:241` | `pico8_nr_cn` | `rl` (d'un `trade`) | Sûr — `pico8_trade_kg` / `pico8_jo_ib` |
| `pico8_planet_family_trade.c:305` | `pico8_mx_kf` | `jy` | Sûr — `pico8_jo_lz` / `pico8_jo_cn` |
| `pico8_slipway_display.c:127` | `pico8_jc_ch` | `ts` | Sûr — `pico8_jc_pq` |

Les trois familles qui ressortent : le popup « techs inventées » du menu
galactique (rien dans les fixtures n'invente de tech), la famille de planètes
`ga`/`nr`/`mx` (rien ne construit ces classes-là), et l'effet de particules
d'un slipway en construction.

## Table complète des 47 sites

`obs.` = tags observés au rejeu. Tous les producteurs cités sont nommés dans
le commentaire posé sur le site lui-même — cette table est un index, la
justification est dans le code.

| Site | Fonction | Champ | Producteur(s) | Classement | obs. |
|---|---|---|---|---|---|
| `pico8_collect.c:25` | `pico8_ft_index` | bucket de `rc[nt]` | la branche `else` de la fonction elle-même ; seul appelant `pico8_ui_gw` | sûr | list=10 |
| `pico8_collect.c:53` | `pico8_ft_name` | bucket de `rc[nt]` | — (fonction morte) | **NON COUVERT** | — |
| `pico8_economy.c:402` | `pico8_hy_kt` | `invented` | `pico8_hy_pq` | sûr | **NON COUVERT** |
| `pico8_economy_formulas.c:55` | `field_list` | `jy` | `pico8_jo_lz`, `pico8_jo_cn` | sûr | list=2 |
| `pico8_economy_score.c:64` | `field_list` | `by`, `invented` | `bd_route_visit` + défaut `ga` ; `pico8_hy_pq` | sûr | list=4395 |
| `pico8_economy_tiers.c:50` | `field_list` | `rl`,`lp`,`jy`,`cc`,`by` | `pico8_trade_kg`, `pico8_jo_ib`, `pico8_jo_lz`, `pico8_jo_cn`, `bd_reset_visit`, `bd_route_visit` | sûr | list=121 |
| `pico8_economy_tiers.c:104` | `bd_reset_visit` | `cc` | `bd_reset_visit` | sûr | list=5197 |
| `pico8_economy_tiers.c:137` | `bd_route_visit` | retour de `jo:ib` | `pico8_jo_ib` | sûr | list=6009 |
| `pico8_economy_tiers.c:193` | `bd_route_visit` | `cc` | `bd_reset_visit` | sûr | list=9 |
| `pico8_economy_tiers.c:247` | `bb_visit` | `jy` | `pico8_jo_lz`, `pico8_jo_cn` | sûr | list=3 |
| `pico8_layout.c:313` | `pico8_lq_ol` | `fs` | `pico8_lq_pq` | sûr | list=8397 |
| `pico8_layout.c:394` | `pico8_jb_ol` | `ls` | `pico8_jb_pq` | sûr | list=24 |
| `pico8_menu.c:209` | `pico8_cr` | `invented` | `pico8_hy_pq` | sûr | **NON COUVERT** |
| `pico8_menu.c:243` | `pico8_cr` | `invented` | `pico8_hy_pq` | sûr | **NON COUVERT** |
| `pico8_particle.c:53` | `pico8_particle_field_list` | `qy` | `pico8_f_` | sûr | list=2701 |
| `pico8_planet.c:382` | `pico8_jo_lz` | `i` (d'un `hp`) | `pico8_hp_pq` | sûr | list=10 |
| `pico8_planet_display.c:78` | `field_list` | `jy`, `by` | `pico8_jo_lz`, `pico8_jo_cn` ; `bd_route_visit` + défaut `ga` | sûr | list=2 |
| `pico8_planet_display.c:130` | `pico8_jo_hb` | `jy` | `pico8_jo_lz`, `pico8_jo_cn` | sûr | list=35 |
| `pico8_planet_display.c:150` | `pico8_jo_hb` | `by` | `bd_route_visit` + défaut `ga` | **ambigu, corrigé chez le producteur** | list=4395 |
| `pico8_planet_display.c:387` | `pico8_jo_gh` | `lq` d'une ligne | `pico8_jo_gh_row` (boucle démarrée à `i=1`, `bs1` sauté) | sûr | list=3 |
| `pico8_planet_family.c:301` | `pico8_ga_kj` | `qy` | `pico8_f_` | sûr | **NON COUVERT** |
| `pico8_planet_family_trade.c:90` | `pico8_nr_gv` | `oe` | `pico8_jo_pq` | sûr | **NON COUVERT** |
| `pico8_planet_family_trade.c:168` | `pico8_nr_hb` | `oe` | `pico8_jo_pq` | sûr | **NON COUVERT** |
| `pico8_planet_family_trade.c:214` | `pico8_nr_cn` | `oe` | `pico8_jo_pq` | sûr | **NON COUVERT** |
| `pico8_planet_family_trade.c:241` | `pico8_nr_cn` | `rl` | `pico8_trade_kg`, `pico8_jo_ib` | sûr | **NON COUVERT** |
| `pico8_planet_family_trade.c:305` | `pico8_mx_kf` | `jy` | `pico8_jo_lz`, `pico8_jo_cn` | sûr | **NON COUVERT** |
| `pico8_planet_trade.c:123` | `pico8_jo_kf` | `oe` | `pico8_jo_pq` | sûr | list=6 |
| `pico8_planet_trade.c:194` | `pico8_jo_cn` | `jy` | `pico8_jo_lz`, `pico8_jo_cn` | sûr | list=18 |
| `pico8_planet_trade.c:326` | `pico8_jo_ib` | `by` | `bd_route_visit` + défaut `ga` | **ambigu, corrigé chez le producteur** | list=5197 |
| `pico8_planet_trade.c:347` | `pico8_jo_ib` | `oe` | `pico8_jo_pq` | sûr | list=6009 |
| `pico8_research_display.c:101` | `pico8_hp_jb_join` | `i` | `pico8_hp_pq` | sûr | list=23 |
| `pico8_research_display.c:334` | `pico8_hp_lq` | `i` | `pico8_hp_pq` | sûr | list=23 |
| `pico8_slipway.c:277` | `pico8_jc_ce` | `oe` (de `qu`) | `pico8_jo_pq` | sûr | list=27 |
| `pico8_slipway_display.c:127` | `pico8_jc_ch` | `ts` | `pico8_jc_pq` | sûr | **NON COUVERT** |
| `pico8_slipway_display.c:219` | `pico8_jc_kj` | `ts` | `pico8_jc_pq` | sûr | list=93 |
| `pico8_tech_gate.c:72` | `pico8_oj` | `invented` | `pico8_hy_pq` | sûr | list=1060 |
| `pico8_trade.c:51` | `pico8_trade_to` | `lp` | `pico8_trade_kg`, `pico8_jo_ib` (classe `trade` sans chaîne de spec) | sûr | list=27 |
| `pico8_trade.c:99` | `pico8_trade_kg` | `lp` | idem | sûr | list=9 |
| `pico8_trade.c:104` | `pico8_trade_kg` | `rl` | idem | sûr | list=9 |
| `pico8_trade.c:154` | `pico8_trade_eq` | `lp` (gauche) | idem | sûr | list=6 |
| `pico8_trade.c:155` | `pico8_trade_eq` | `lp` (droite) | idem | sûr | list=6 |
| `pico8_ui.c:286` | `pico8_ui_lj` | `le` | `pico8_ui_init`, `pico8_ui_cj`, `pico8_pz` | sûr | list=1391 |
| `pico8_ui.c:302` | `pico8_ui_lj` | `le` (capturé avant `gw`/`bh`) | idem | sûr | list=323 |
| `pico8_ui.c:551` | `pico8_ui_jz` | bucket de `ic` | `pico8_ft_index` | sûr | list=108 |
| `pico8_ui.c:602` | `pico8_ui_cj` | `le` | `pico8_ui_init`, `pico8_ui_cj`, `pico8_pz` | sûr | list=69 |
| `pico8_ui_button.c:150` | `pico8_js_lq_arg_table` | argument `lq`/`h_` | — | **sûr par le tag** (branche LIST du `switch`) | list=3 |
| `pico8_wormhole.c:140` | `pico8_mz_hq` | `ox` | `pico8_mz_pq` | sûr | list=194 |

## Divergence relevée, non corrigée (hors périmètre du PRD)

`pico8_ft_name()` (`src/pico8_collect.c`) est le portage fidèle de
`ft(cv,p,e)` / `ft(cg,t,e)` (`pico8/lua.lua:157` et `:160`) et **n'a aucun
appelant**. La raison est que `pico8_lt`/`pico8_he` (`src/pico8_entity.c`)
ne modélisent pas `cv` et `cg` comme des tables Lua de buckets-listes mais
comme des tableaux C statiques de vraies `pico8_list_t`, et y ajoutent /
en retirent directement ; `pico8_dc` fait de même avec son `fl` local
(tableau de 15 `pico8_list_t` de pile) au lieu de `ft(fl,qc.fa,qc)`
(`lua.lua:191`). Les trois des quatre appels réels de `ft()` du cart sont
donc inlinés, et le seul restant (`ft(self.ic,e.fa,e)`, `lua.lua:229`) passe
par `pico8_ft_index`. Aucune différence de comportement n'a été trouvée —
c'est une divergence de *représentation*, déjà documentée dans
`pico8_entity.c`. Elle est signalée ici parce qu'elle a une conséquence pour
cet audit : un site de cast qu'aucune méthode ne pourra jamais atteindre
tant que la fonction reste morte. Conformément au PRD (« toute correction de
fidélité découverte pendant l'audit est signalée, pas traitée »), rien n'a
été changé.

Aucune autre divergence de comportement vis-à-vis de `pico8/lua.lua` n'a été
relevée sur les 47 sites.

## Désambiguïsations heuristiques restantes (user story 11)

Aucune. Le seul reniflage heuristique du port — `pico8_js_lq_arg_table`
comparant `->cls` à `pico8_lq_class()` — avait déjà été remplacé par le
dispatch sur tag lors du correctif du segfault `lq`. Un `grep` sur `->cls`
dans `src/*.c` ne rend plus que `pico8_class.c` (parcours de la chaîne
d'héritage, usage légitime) et les commentaires historiques de
`pico8_ui_button.c`. Rien à supprimer.

## Rejouer l'audit

```
make ref-audit
```

Sortie brute dans `tmp/ref_audit_raw.txt`, rapport dans
`tmp/ref_audit_report.txt`. La cible est un diagnostic, pas un test : elle
n'est volontairement pas dans `make test`, parce qu'elle répond à « qu'est-ce
que les scénarios atteignent » et non à « le port se comporte-t-il bien ».
Le script rejoue les fichiers qu'il reçoit en argument ; `make ref-audit` lui
passe les rejeux du filet (`FRAMES_EVENTS`). Pour étendre la couverture,
ajouter un rejeu au filet ou un binaire au bloc `REFAUDIT_BINS` du Makefile.
