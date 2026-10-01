---
title: "Plan 3, étape 1a.16b : préparation de `jo` — accesseur `pico8_instance_fields`, et tout accès à une planète passe à `pico8_entity_get_field`/`set_field`, aucune classe convertie"
description: "Avant que `jo` devienne une struct (1a.17), chaque site du port qui lit ou écrit une planète par la couche à simple forme (`pico8_instance_get_field`, `x->fields`) doit passer à l'API à double forme, sans rien changer à l'exécution. L'étape pose l'accesseur `pico8_instance_fields(x)` qui asserte un tag non-struct, convertit les 114 sites mesurés (94 dans les fichiers nommés par le plan, 20 découverts par le fil piège), et prouve par un fil piège local qu'aucune planète n'atteint plus un chemin à simple forme."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.16b — préparation de `jo`, **aucune classe convertie** : (1) tout accès `x->fields` hors `pico8_class.c` passe par un accesseur `pico8_instance_fields(x)` qui asserte un tag non-struct ; (2) dans `planet.c`, `planet_display.c`, `planet_trade.c`, `planet_family.c`, `planet_family_trade.c`, `economy*.c`, `research_display.c`, `score_panel_display.c`, tout `pico8_instance_get_field`/`set_field`/`x->fields` sur une valeur qui peut être une planète passe à `pico8_entity_get_field`/`set_field` (équivalents exacts sur une instance) ; (3) l'inventaire (grep, puis fil piège local non commité : assert dans l'accesseur et `pico8_instance_get_field` quand la chaîne de `inst->cls` contient `jo`, `make test` + `frames-check`) est versionné dans le PRD. Critère : sortie de `class-coverage-net` et `*.sha256` inchangés ; fil piège muet. Tests : aucun.

Règles du §5 qui fondent l'étape :

> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre (dispatcher hérité dans `defaults`, ou appel direct `jo.kj(self,…)`, `jo.kf(self,…)`) reste à double forme : lecture/écriture par `pico8_entity_get_field`/`set_field`, ou aiguillage sur le tag comme `pico8_ke_ks`. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante (pour `jo` : 1a.26).
>
> **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag (`jo` pour `cg.jo`) […].
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe […].

Les deux dernières règles **ne s'appliquent pas** à ce commit (aucune classe convertie, aucun `->defaults` touché) ; elles sont citées parce que l'inventaire ci-dessous les nourrit pour 1a.17 (voir Further Notes). §7 : un écart port/cart découvert est préservé à l'identique et décrit par un `TROU-N`. Aucun n'est attendu ici : l'étape ne change que des appels à sémantique identique.

### Pourquoi l'étape existe

Dans l'arbre `jo` (`jo` → `qh` → `nl`/`os`/`hj`→`pm`/`mx`/`nr`→`fr`, et `jo` → `ga`), 1a.17 convertit `jo` seul. Après 1a.17, une planète est soit une struct `jo` (1 710 constructions sur le filet), soit une instance d'une fille non convertie, dont la chaîne `cls` passe encore par la `pico8_class_t` de `jo`. Tout site qui peut recevoir une planète recevra donc **les deux formes**. Aujourd'hui, 114 sites du port lisent ou écrivent une planète par la couche à simple forme. `pico8_instance_get_field` asserte sur une struct ; `x->fields` sur une struct lit n'importe quoi. 1a.16b fait tout ce travail **avant** 1a.17, à comportement constant, pour que le diff de 1a.17 ne contienne que la conversion de `jo`.

### Équivalence exacte sur une instance (vérifiée dans le code à 6a7474b)

`pico8_entity_get_field(e, name, out)` : si `pico8_type_tag_of(e) != PICO8_TYPE_TAG_STRUCT`, il **rend** `pico8_instance_get_field(e, name, out)`, sans rien d'autre (`pico8_entity.c:177-179`). `pico8_entity_set_field(e, name, v)` : sur une non-struct, il fait `pico8_ob_table_set_name(((pico8_instance_t *)e)->fields, name, v)` et rien d'autre (`pico8_entity.c:194-196`). D'où, pour tout pointeur qui n'est pas une struct :

| Cas | `pico8_instance_get_field` / `ob_table_set_name(x->fields,…)` | `pico8_entity_get_field` / `set_field` |
|---|---|---|
| champ propre présent | 1, valeur propre | identique (même appel) |
| absent en propre, présent dans un défaut de la chaîne | 1, valeur du défaut le plus proche (la surcharge masque la parente) | identique |
| absent partout | 0, `*out` intact | identique |
| propre = nil (`REF NULL`) écrit par `set_name` | 1, `REF NULL` : le nil propre **masque** le défaut (`pico8_ob_table_set` écrase ou ajoute, ne retire jamais) | identique |
| écriture | toujours dans la table propre, jamais dans un défaut ; écrase en place une entrée de même nom, sinon l'ajoute en fin (ordre d'insertion conservé) | identique (même appel) |
| écriture d'un nil | ajoute ou écrase l'entrée avec `REF NULL`, qui reste **présente** | identique |
| tag `UNKNOWN` (instance mal taguée) | lu comme instance | lu comme instance : même branche |
| receveur NULL | déréférence NULL (crash) | même crash : `pico8_type_tag_of(NULL)` rend `UNKNOWN`, puis même appel. Tous les sites convertis gardent leur garde `!= NULL` existante |

**Où ce n'est pas un équivalent, et ce que fait l'étape** :

- **Lecture propre seule** `pico8_ob_table_get_name(x->fields, …)` : ne remonte pas la chaîne, `pico8_entity_get_field` la remonte. **Pas un équivalent.** Mesuré : aucune n'est atteinte par une planète (`economy.c:48,222,226` sur `hy`, `research.c:154` sur `hp`). Elles passent à l'accesseur seul, jamais à `pico8_entity_get_field`.
- **Accès indexés, parcours, passage de la table** (`pico8_ob_table_get_index`/`set_index`, `positional_values(x->fields)`, `pico8_qj(x->fields,…)`, `pico8_ho_apply(…, x->fields)`) : pas d'équivalent dans l'API d'entité. Mesuré : aucun n'est atteint par une planète (`lq`, `hp`, `hy`). Accesseur seul.
- **Garde `x->fields == NULL`** : pas d'équivalent. Un seul site est atteint par une planète : `particle.c:63` (`pico8_f_`, sur un `ga`). Il prend la forme déjà en place à `probe.c:128` : `pico8_type_tag_of(self) == PICO8_TYPE_TAG_INSTANCE && pico8_instance_fields(self) == NULL`. Sur une instance, c'est la même condition. Sur une struct (jamais vue avant 1a.26), elle rend faux au lieu de lire hors de la struct.
- **Constness** : `pico8_entity_set_field` prend `void *`. Les 26 écritures converties ont toutes un receveur non `const` (vérifié en compilant avec `-Werror`). Aucun cast n'est ajouté.
- **Écritures déjà aiguillées sur le tag** (`ke.c:45-46` dans `pico8_ke_ks`, `bootstrap.c:101` dans `pico8_dg`, garde de `probe.c:128`) : elles sont déjà à double forme au sens du §5 (« ou aiguillage sur le tag comme `pico8_ke_ks` »). Leur branche instance ne reçoit que des instances, par construction, avant comme après 1a.17. Elles passent à l'accesseur seul, sans conversion. Le fil piège les ignore (voir plus bas).

### Inventaire mesuré (HEAD 6a7474b) — partie (3) de l'étape, versionnée ici

**Méthode.** (a) Grep de `pico8_instance_get_field` et de `->fields`/`.fields` dans tout `src/`, classé par receveur en lisant chaque fonction. (b) Fil piège de mesure, lancé dans une copie jetable (`git archive HEAD`) : `pico8_instance_get_field` et chaque `x->fields` en valeur (réécrit en macro portant `__FILE__`/`__LINE__`) journalisent, sans arrêter, chaque site de `src/` où l'instance reçue a `jo` dans sa chaîne `cls`. `pico8_entity.c` est exclu, `pico8_class.c` n'est pas instrumenté. Puis `make -j4 test` (dont `frames-check`), sortie 0. Une première passe a donné 7 faux positifs dans `research.c` : un pointeur de classe `jo` périmé après `pico8_jo_shutdown()`, réutilisé par l'allocateur pour `hp`. Le piège remet donc son pointeur à NULL au shutdown ; la seconde passe les élimine.

**Résultat.** 114 sites de `src/` distincts atteints par une planète, **tous atteints par `tmp/pico8_replay`**, donc par le filet. Aucun site du grep, dans les fichiers nommés, dont le receveur puisse être une planète n'échappe au piège : le grep et le piège donnent le même ensemble. 94 sites sont dans les fichiers nommés par le plan, 20 sont **découverts** dans `msgbox.c`, `particle.c`, `probe.c`, `wormhole.c`, `wormhole_display.c`, `entity_geom.c`, `ke.c` et `bootstrap.c`. Ils relèvent des règles « Coexistence » et « Méthodes d'une parente convertie » : ce sont des lecteurs génériques, ou des fonctions d'une parente atteintes par une descendante. Ce n'est donc pas un arrêt. Aucune lecture propre seule, aucun accès indexé ni aucune garde `fields == NULL` n'est atteint par une planète, hors `particle.c:63`.

Répartition : 83 lectures → `pico8_entity_get_field` ; 26 écritures → `pico8_entity_set_field` ; 5 restent à l'accesseur seul (3 aiguillages, 1 garde aiguillée, et la garde de `particle.c:63` réécrite en garde aiguillée). Les numéros de ligne sont ceux de HEAD 6a7474b. L'implémenteur s'appuie sur (fonction, receveur, champ), qui ne bouge pas quand une ligne se décale.

#### A. Sites atteints par une planète (114)

| Site (HEAD 6a7474b) | Fichier de la ligne 1a.16b ? | Fonction | Receveur | Champ | Avant → après |
|---|---|---|---|---|---|
| `bootstrap:101` | **découvert** | `pico8_dg` | `qc` | `t` | accesseur seul : branche instance d'un aiguillage déjà à double forme |
| `economy:242` | liste | `pico8_hy_lj` | `p` | `kr` | `pico8_ob_table_set_name(p->fields, …)` → `pico8_entity_set_field(p, …)` |
| `economy_formulas:19` | liste | `field_truthy` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_formulas:27` | liste | `field_vec` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_formulas:35` | liste | `field_ref_table` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_formulas:43` | liste | `field_list` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_score:31` | liste | `field_fixed` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_score:39` | liste | `field_truthy` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_score:47` | liste | `field_ref_table` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_score:55` | liste | `field_list` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_score:110` | liste | `pico8_da` | `pt` | `mi` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_score:145` | liste | `pico8_db` | `pt` | `mi` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_score:185` | liste | `pico8_db` | `oz` | `mi` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_score:240` | liste | `sum_by_mi_visit` | `pt` | `mi` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:28` | liste | `field_truthy` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:36` | liste | `field_list` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:44` | liste | `field_ref_instance` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:85` | liste | `bd_reset_visit` | `pt` | `cc` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:92` | liste | `bd_reset_visit` | `pt` | `cc` | `pico8_ob_table_set_name(pt->fields, …)` → `pico8_entity_set_field(pt, …)` |
| `economy_tiers:107` | liste | `bd_route_visit` | `pt` | `by` | `pico8_ob_table_set_name(pt->fields, …)` → `pico8_entity_set_field(pt, …)` |
| `economy_tiers:149` | liste | `bd_route_visit` | `planet` | `id` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:165` | liste | `bd_route_visit` | `dest` | `cc` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:199` | liste | `bb_visit` | `pt` | `mi` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:210` | liste | `bb_visit` | `pt` | `jy` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:218` | liste | `bb_visit` | `pt` | `mi` | `pico8_ob_table_set_name(pt->fields, …)` → `pico8_entity_set_field(pt, …)` |
| `economy_tiers:240` | liste | `bb_visit` | `pt` | `jh` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:243` | liste | `bb_visit` | `pt` | `kw` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `economy_tiers:269` | liste | `pico8_dh` | `pt` | `me` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `entity_geom:64` | **découvert** | `pico8_na` | `self` | `ra` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `ke:45` | **découvert** | `pico8_ke_ks` | `self` | `me` | accesseur seul : branche instance d'un aiguillage déjà à double forme |
| `ke:46` | **découvert** | `pico8_ke_ks` | `self` | `t` | accesseur seul : branche instance d'un aiguillage déjà à double forme |
| `msgbox:432` | **découvert** | `pico8_pz_step` | `ctx->p` | `ca` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `msgbox:462` | **découvert** | `pico8_pz` | `p` | `ep` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `msgbox:480` | **découvert** | `pico8_pz` | `p` | `eo` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `msgbox:487` | **découvert** | `pico8_pz` | `p` | `hi` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `msgbox:519` | **découvert** | `pico8_pz` | `p` | `ff` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `particle:17` | **découvert** | `pico8_particle_field_fixed` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `particle:25` | **découvert** | `pico8_particle_field_vec` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `particle:33` | **découvert** | `pico8_particle_field_list` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `particle:41` | **découvert** | `pico8_particle_field_table` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `particle:63` | **découvert** | `pico8_f_` | `self` | — | garde `self->fields == NULL` → `pico8_type_tag_of(self) == PICO8_TYPE_TAG_INSTANCE && pico8_instance_fields(self) == NULL` (forme de `probe.c:128`) |
| `particle:69` | **découvert** | `pico8_f_` | `self` | `qy` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:226` | liste | `pico8_jo_pq` | `self` | `oe` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:228` | liste | `pico8_jo_pq` | `self` | `hp` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet:266` | liste | `pico8_jo_lz` | `self` | `hn` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:267` | liste | `pico8_jo_lz` | `self` | `ep` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:276` | liste | `pico8_jo_lz` | `self` | `jy` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:283` | liste | `pico8_jo_lz` | `self` | `kw` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:290` | liste | `pico8_jo_gg` | `self` | `hf` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet:296` | liste | `pico8_jo_gg` | `self` | `me` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet:306` | liste | `pico8_jo_kd` | `self` | `ma` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:313` | liste | `pico8_jo_ll` | `self` | `p_` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:335` | liste | `pico8_jo_hu` | `self` | `p_` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:342` | liste | `pico8_jo_hu` | `self` | `p` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet:345` | liste | `pico8_jo_hu` | `self` | `ra` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet:348` | liste | `pico8_jo_hu` | `self` | `id` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet:367` | liste | `pico8_jo_hu` | `self` | `nk` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:372` | liste | `pico8_jo_hu` | `self` | `fx` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:418` | liste | `pico8_jo_hh` | `self` | `nk` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:423` | liste | `pico8_jo_hh` | `self` | `ep` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet:441` | liste | `pico8_jo_hk` | `self` | `kr` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet:456` | liste | `pico8_jo_hk` | `self` | `ra` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet:475` | liste | `pico8_jo_ldown` | `self` | `me` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:38` | liste | `field_truthy` | `self` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:46` | liste | `field_fixed` | `self` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:54` | liste | `field_list` | `self` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:62` | liste | `field_ref_table` | `self` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:101` | liste | `pico8_jo_hb` | `self` | `jy` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:104` | liste | `pico8_jo_hb` | `self` | `kw` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:114` | liste | `pico8_jo_hb` | `self` | `by` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:120` | liste | `pico8_jo_hb` | `self` | `lq` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet_display:177` | liste | `pico8_jo_gh` | `self` | `px` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:181` | liste | `pico8_jo_gh` | `self` | `mi` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:193` | liste | `pico8_jo_gh` | `self` | `kr` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:357` | liste | `pico8_jo_gv` | `self` | `me` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:495` | liste | `pico8_jo_kj` | `self` | `me` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:515` | liste | `pico8_jo_kj` | `self` | `nk` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:520` | liste | `pico8_jo_kj` | `self` | `s` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:546` | liste | `pico8_jo_kj` | `self` | `lq` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:564` | liste | `pico8_jo_kj` | `self` | `fo` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_display:584` | liste | `pico8_jo_kj` | `self` | `fo` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet_family:39` | liste | `pico8_pf_field_fixed` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family:54` | liste | `pico8_pf_field_vec` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family:81` | liste | `pico8_nl_ly` | `self` | `mi` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family:88` | liste | `pico8_nl_ly` | `self` | `qw` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet_family:109` | liste | `pico8_hj_kj` | `self` | `mi` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family:236` | liste | `pico8_ga_kj` | `self` | `qy` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family:287` | liste | `pico8_ga_kj` | `self` | `p_` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet_family_trade:64` | liste | `pico8_nr_gv` | `self` | `oe` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family_trade:132` | liste | `pico8_nr_hb` | `self` | `oe` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family_trade:136` | liste | `pico8_nr_hb` | `self` | `hz` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family_trade:144` | liste | `pico8_nr_hb` | `self` | `s` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet_family_trade:167` | liste | `pico8_nr_cn` | `self` | `oe` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family_trade:241` | liste | `pico8_mx_kf` | `cp` | `jy` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family_trade:268` | liste | `pico8_mx_kf` | `self` | `ht` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family_trade:278` | liste | `pico8_mx_kf` | `self` | `kw` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_family_trade:285` | liste | `pico8_mx_kf` | `self` | `ht` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet_trade:82` | liste | `pico8_jo_kf` | `self` | `oe` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_trade:92` | liste | `pico8_jo_kf` | `eb_result (cast)` | `fx` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_trade:94` | liste | `pico8_jo_kf` | `self` | `fx` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet_trade:127` | liste | `pico8_jo_cn` | `self` | `jy` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_trade:132` | liste | `pico8_jo_cn` | `self` | `jh` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_trade:172` | liste | `pico8_jo_cn` | `self` | `jy` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet_trade:175` | liste | `pico8_jo_cn` | `self` | `jh` | `pico8_ob_table_set_name(self->fields, …)` → `pico8_entity_set_field(self, …)` |
| `planet_trade:197` | liste | `pico8_jo_cn` | `self` | `ba` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_trade:238` | liste | `pico8_jo_ib` | `self` | `by` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_trade:245` | liste | `pico8_jo_ib` | `self` | `kw` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `planet_trade:257` | liste | `pico8_jo_ib` | `self` | `oe` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `probe:49` | **découvert** | `pico8_probe_field_truthy` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `probe:128` | **découvert** | `pico8_bv` | `self` | — | accesseur seul : garde déjà aiguillée sur le tag (`== PICO8_TYPE_TAG_INSTANCE && …`) |
| `research_display:353` | liste | `pico8_bj_predicate` | `ctx->jo_instance` | `nk` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `wormhole:66` | **découvert** | `pico8_mz_field_vec` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `wormhole:232` | **découvert** | `pico8_mz_pr` | `pt` | `me` | `pico8_instance_get_field` → `pico8_entity_get_field` |
| `wormhole_display:28` | **découvert** | `pico8_mzd_field_vec` | `inst` | `name` (param.) | `pico8_instance_get_field` → `pico8_entity_get_field` |

Les sites de `planet.c`, `planet_display.c`, `planet_trade.c`, `planet_family.c` et `planet_family_trade.c` dont le receveur est `self` sont des méthodes de `jo` ou de ses filles (`nl:ly`, `hj:kj`, `ga:kj`, `nr:*`, `mx:kf`). Tous leurs receveurs sont des planètes, y compris dans une fille : une fille de `jo` est une planète. Les aides `field_*` converties servent aussi à des receveurs qui ne sont pas des planètes. La conversion y est aussi exacte (tableau d'équivalence).

#### B. Sites des fichiers nommés dont le receveur n'est jamais une planète — inchangés

Mesurés muets par le piège, et confirmés en lisant le receveur :

- `planet.c:270,279,328,354,382` : `hp_instance` (`hp`, `i`/`o`/`cw`/`go`/`rn`).
- `planet_trade.c:120` : `t` (`trade`, `qe`). `planet_family_trade.c:186` : `t` (`trade`, `rl`).
- `economy.c:20` (`field_fixed`, n'appelé que sur `self`), `:124`, `:299` : `self` = `hy`.
- `economy_tiers.c:208` : `hn` (`hp`, `dm`). `:301` : `im` (`trade`, `qe`).
- `research_display.c:73,122,132,295,298,317` : `self` = `hp` ; `:346,361` : `b` (`hp`).
- `score_panel_display.c:33` : `fm` (`hy`) ; `:166,181` : `ju` (`pico8_ju()`) ; `:278,311,318` : `self` = `pj`. **Aucun site de `score_panel_display.c` n'est atteint par une planète.** Le fichier ne change qu'à l'accesseur (section C).

Après l'étape, `grep -c pico8_instance_get_field` rend exactement : `planet.c` 5, `planet_display.c` 0, `planet_trade.c` 1, `planet_family.c` 0, `planet_family_trade.c` 1, `economy.c` 3, `economy_formulas.c` 0, `economy_score.c` 0, `economy_tiers.c` 2, `research_display.c` 8, `score_panel_display.c` 6, `msgbox.c` 0, `particle.c` 0, `probe.c` 0, `wormhole.c` 0, `wormhole_display.c` 0, `entity_geom.c` 0.

#### C. Accès `->fields` qui passent à l'accesseur seul (partie 1), receveur jamais une planète — 54 sites

Réécriture mécanique `x->fields` → `pico8_instance_fields(x)` (et `((T *)y)->fields` → `pico8_instance_fields((T *)y)`), même ligne, rien d'autre :

- `cheat.c:25,28,29` (`fm`) ;
- `economy.c:45,48,55,94,99,142,144,146,177,222,226,232,233,252,255,266,297,314` (`self` = `hy`, dont `pico8_qj(…)` et les lectures propres seules `invented`/`po`/`fu`) ;
- `entity.c:195` : branche instance de `pico8_entity_set_field`, `((pico8_instance_t *)e)->fields` → **`pico8_instance_fields(e)`**, cette orthographe exacte, que le fil piège reconnaît ;
- `layout.c:188,201,202` (`lq`) ;
- `menu.c:285` (`fm`), `:342` (`ju`) ;
- `msgbox.c:506,522` (`ok`, `ui`) ;
- `research.c:151,154,177,186` (`hp`) ;
- `research_display.c:313,315,318` (`l`, instance `lq`, accès positionnels) ;
- `score_panel.c:281`, `score_panel_display.c:303,304,305,345` (`pj`), `score_panel_display.c:167` (`ju`) ;
- `slipway.c:118,120,259` (`placeholder`, sans classe) ;
- `ui.c:210,230,283,284,301,350,374,450` (`ui`, dont les clés `ni` construites) ;
- `ui_button.c:30` (`pico8_js_lq_arg_table`, instance `lq`).

S'y ajoutent les 5 sites « accesseur seul » de la table A : `ke.c:45,46`, `bootstrap.c:101`, `probe.c:128` et `particle.c:63`. Au total, 59 appels à l'accesseur hors de sa définition.

#### D. Ce qui garde l'orthographe `fields` — 7 initialisations et 2 commentaires

Ce sont les 7 affectations **du pointeur** `fields` d'une instance sans classe, par le code qui vient de la construire, avec son tag `INSTANCE` posé à la ligne précédente et `cls` NULL. Un accesseur en valeur ne peut pas être une lvalue :

- `menu.c:245`, `menu.c:415` (`wrapper.fields = …`, pile) ;
- `planet_family_trade.c:95` (`empty.fields = …`, pile) ;
- `probe.c:170` (`empty->fields = …`), `probe.c:387` (`g_fm->fields = …`) ;
- `slipway.c:117`, `slipway.c:258` (`placeholder->fields = …`).

Et deux commentaires, inchangés : `entity.h:79`, `menu.c:384`. `tests/` et `tools/` ne sont pas touchés (voir Out of Scope).

#### E. Lectures de planète hors de `src/` — hors périmètre, notées pour 1a.17 et 1a.26

Le piège de mesure les a vues. Le fil piège final les ignore par préfixe de fichier. Ces tests de régression sont réécrits dans le commit de leur classe (§6), jamais ici :

- `built_planet_panel_nil_row:106,119,218` (`me`, `id`, `ep`) ;
- `empty_planet_cleared:55,63` (`me`, `ra`) ;
- `rich_planet_exports:50,59,79` (`id`, `kw`, `jy`) ;
- `slipway_drag:39` (`oe`) ;
- `ga_export_panel_by:80` (`by`, sur un `ga`, 1a.26) ;
- `geoharvest_burst:94` (`qy`, sur un `ga`, 1a.26).

### Preuve de faisabilité (faite en préparant ce PRD, dans une copie jetable)

Toute l'étape a été appliquée à une copie `git archive` de 6a7474b, puis vérifiée :

- l'accesseur, la section C, la table A et les 3 `#include "pico8_entity.h"` manquants (`planet.c`, `research_display.c`, `wormhole_display.c`) ;
- le fil piège final (script ci-dessous) posé par-dessus.

Résultats :

- `make -j4 test` rend 0 (≈ 5 min 30), avec `frames-check` conforme ;
- `make class-coverage-net` rend 0, sortie identique à `tools/class_coverage_net_expected.txt` ;
- le journal du piège est absent.
- **Contre-épreuve** : en remettant `pico8_instance_get_field(self, "hp", …)` dans `pico8_jo_pq`, `make frames-check` s'arrête au premier rejeu (`session_events.json`), sur `fil piege 1a.16b: une planete atteint src/pico8_planet.c:… (get hp)`, et le journal contient exactement cette ligne.

Le diff de `src/` fait ≈ 180 lignes modifiées, plus l'accesseur. Aucune autre ligne ne change.

## Solution

Un seul commit. Aucune classe convertie, aucun comportement changé.

1. **Accesseur** `pico8_ob_table_t *pico8_instance_fields(const pico8_instance_t *inst)`, déclaré dans le module des classes à côté de `pico8_instance_get_field`, et défini dans ce module.
   - Il asserte `pico8_type_tag_of(inst) != PICO8_TYPE_TAG_STRUCT`, avec un message porteur qui renvoie à `pico8_entity_get_field()`/`pico8_entity_set_field()`, sur le modèle de l'assert de `pico8_instance_get_field`.
   - Il rend `inst->fields`, emprunté.
   - Contrat en commentaire : la table propre, jamais la chaîne ; `inst` non NULL ; une struct asserte.
   - `pico8_class.c` garde ses propres `->fields`.
2. **Partie (1)** : les 54 sites de la section C et les 5 sites « accesseur seul » de la table A passent à l'accesseur, par réécriture mécanique.
3. **Partie (2)** : les 109 autres sites de la table A passent à `pico8_entity_get_field` (83) ou `pico8_entity_set_field` (26).
   - Même receveur, même nom, même valeur.
   - Les signatures des aides (`field_*`, `pico8_pf_field_*`, `pico8_particle_field_*`, `pico8_mz_field_vec`, `pico8_mzd_field_vec`, `pico8_probe_field_truthy`) **ne changent pas** : elles gardent leur paramètre `const pico8_instance_t *`. Le retypage appartient à 1a.17.
   - `#include "pico8_entity.h"` est ajouté là où il manque (`planet.c`, `research_display.c`, `wormhole_display.c`).
4. **Garde de `particle.c:63`** réécrite sous la forme aiguillée de `probe.c:128` (table A).
5. **Fil piège**, local, jamais commité (script ci-dessous) : `make test` et `make class-coverage-net` passent, et le journal reste absent. Puis contre-épreuve, puis retrait.
6. **Test de couche** (voir Testing) : `test_pico8_entity_smoke` gagne deux tests. L'un fixe le contrat de l'accesseur, l'autre fixe l'équivalence instance ↔ API d'entité sur laquelle toute l'étape repose.
7. **Plan** : à la ligne 1a.16b de `docs/plan-3.md`, ajouter :

   > Découvert à 1a.16b : `p` de `pz`/`pz_step` est la planète de `jo:gh`/`jo:fe` (`msgbox.c:432,462,480,487,519`) ; aides de `particle.c` sur un `ga` (`:17-41`, `f_` `:63,69`) ; `pico8_probe_field_truthy` (`probe.c:49`) ; `mz` lit ses extrémités (`wormhole.c:66,232`, `wormhole_display.c:28`) ; `na` = `qh:ly` (`entity_geom.c:64`) → `pico8_entity_get_field`/`set_field` ; `ke_ks` (`ke.c:45-46`), `dg` (`bootstrap.c:101`), `bv` (`probe.c:128`) déjà aiguillés → accesseur seul ; `f_` (`particle.c:63`) → garde aiguillée ; écrivains génériques sur `cg.jo` pour 1a.17 : `kr` (`economy.c:242`), `cc`/`by`/`mi` (`economy_tiers.c:92,107,218`), `t` (`bootstrap.c:101`, membre de `ke`).

### Le fil piège : comment l'implémenteur le lance, et comment le vérificateur le reproduit

Le fil piège ne touche **jamais** l'arbre de travail ni le commit. Il s'applique à une copie jetable, dans le dossier temporaire de session :

```
C=<dossier temporaire>/fil-piege && rm -rf $C && mkdir -p $C
# implémenteur, avant commit : copie de l'arbre de travail tel qu'il sera commité
git ls-files -z | xargs -0 tar -cf - | tar -x -C $C
# vérificateur, après commit : git archive HEAD | tar -x -C $C
cd $C && python3 <le script ci-dessous> && rm -f tmp/fil_piege.log
make -j4 test && make class-coverage-net ; echo "sortie $?"
test ! -e tmp/fil_piege.log && echo "fil piège muet"
```

Critère : la sortie vaut 0, `make class-coverage-net` imprime une sortie identique à `tools/class_coverage_net_expected.txt`, et `tmp/fil_piege.log` n'existe pas. Le journal attrape aussi un piège déclenché dans un fils forké dont l'échec serait masqué. **Contre-épreuve**, obligatoire, dans la même copie : remettre `pico8_instance_get_field(self, "hp", …)` dans `pico8_jo_pq`, puis `make frames-check`. Attendu : arrêt sur `fil piege 1a.16b: une planete atteint src/pico8_planet.c:<n> (get hp)`, et cette seule ligne dans le journal. Sinon, le piège est mal posé, et son silence ne prouve rien. On jette ensuite la copie. Durées mesurées : `make -j4 test` ≈ 5 min 30, `frames-check` seul ≈ 4 min 20.

Ce que fait le piège :

- `pico8_instance_fields` et `pico8_instance_get_field` deviennent, hors de `pico8_class.c`, des macros qui transmettent `__FILE__`/`__LINE__`.
- Un site dont le fichier commence par `src/` et dont l'instance reçue a la `pico8_class_t` de `jo` dans sa chaîne `cls` écrit une ligne au journal, puis `abort()`.
- `tests/` est ignoré (section E).
- `pico8_entity.c` (chemin double forme) et les 5 sites « accesseur seul » (aiguillages) appellent la fonction nue par `(pico8_instance_fields)(…)`, un nom parenthésé que la macro ne capte pas.
- `pico8_planet.c` publie le pointeur de classe `jo` à l'init et le remet à NULL au shutdown : sans cette remise, on obtient des faux positifs par réutilisation d'adresse (mesuré).

Script (prototype éprouvé sur la copie de faisabilité ; chaque `sub` asserte que son texte d'origine existe, et une orthographe différente de l'implémentation le fait échouer : on adapte alors **seulement** la chaîne cherchée, jamais la logique) :

```python
# Fil piège 1a.16b : patch LOCAL, jamais commité. Lancer depuis la racine du dépôt.
# Journal : $PICO8_TRAP_LOG (défaut tmp/fil_piege.log), une ligne par site, puis abort.
import re
def sub(path, old, new, count=1):
    s = open(path).read()
    assert s.count(old) >= 1, (path, old)
    open(path, 'w').write(s.replace(old, new, count if count else s.count(old)))

sub('src/pico8_class.h', 'pico8_ob_table_t *pico8_instance_fields(const pico8_instance_t *inst);',
'''pico8_ob_table_t *pico8_instance_fields(const pico8_instance_t *inst);
extern pico8_class_t *g_trap_jo;
pico8_ob_table_t *pico8_trap_fields(const pico8_instance_t *x, const char *file, int line);
int pico8_trap_get_field(const pico8_instance_t *x, const char *name, pico8_value_t *out, const char *file, int line);
#ifndef PICO8_TRAP_INTERNAL
#define pico8_instance_fields(x) pico8_trap_fields((x), __FILE__, __LINE__)
#define pico8_instance_get_field(x, n, o) pico8_trap_get_field((x), (n), (o), __FILE__, __LINE__)
#endif''')
s = open('src/pico8_class.c').read()
s = '#define PICO8_TRAP_INTERNAL\n' + s + r'''
#include <stdio.h>
pico8_class_t *g_trap_jo = NULL;
static void trap_check(const pico8_instance_t *x, const char *what, const char *name, const char *file, int line) {
    const pico8_class_t *c;
    FILE *f;
    const char *path = getenv("PICO8_TRAP_LOG");
    if (g_trap_jo == NULL || strncmp(file, "src/", 4) != 0 || pico8_type_tag_of(x) != PICO8_TYPE_TAG_INSTANCE) return;
    for (c = x->cls; c != NULL && c != g_trap_jo; c = c->parent) {}
    if (c == NULL) return;
    f = fopen(path ? path : "tmp/fil_piege.log", "a");
    if (f != NULL) { fprintf(f, "%s:%d %s %s\n", file, line, what, name ? name : "-"); fclose(f); }
    fprintf(stderr, "fil piege 1a.16b: une planete atteint %s:%d (%s %s)\n", file, line, what, name ? name : "-");
    abort();
}
pico8_ob_table_t *pico8_trap_fields(const pico8_instance_t *x, const char *file, int line) {
    trap_check(x, "fields", NULL, file, line);
    return pico8_instance_fields(x);
}
int pico8_trap_get_field(const pico8_instance_t *x, const char *name, pico8_value_t *out, const char *file, int line) {
    trap_check(x, "get", name, file, line);
    return pico8_instance_get_field(x, name, out);
}
'''
open('src/pico8_class.c', 'w').write(s)
# Chemins à double forme légitimes : n'y passent que des instances, par construction.
sub('src/pico8_entity.c', 'return pico8_instance_get_field(e, name, out_value);', 'return (pico8_instance_get_field)(e, name, out_value);')
sub('src/pico8_entity.c', 'pico8_instance_fields(e)', '(pico8_instance_fields)(e)')
sub('src/pico8_ke.c', 'pico8_ob_table_set_name(pico8_instance_fields(self), "me"', 'pico8_ob_table_set_name((pico8_instance_fields)(self), "me"')
sub('src/pico8_ke.c', 'pico8_ob_table_set_name(pico8_instance_fields(self), "t"', 'pico8_ob_table_set_name((pico8_instance_fields)(self), "t"')
sub('src/pico8_bootstrap.c', 'pico8_instance_fields(qc)', '(pico8_instance_fields)(qc)')
sub('src/pico8_probe.c', '== PICO8_TYPE_TAG_INSTANCE && pico8_instance_fields(self) == NULL', '== PICO8_TYPE_TAG_INSTANCE && (pico8_instance_fields)(self) == NULL')
sub('src/pico8_particle.c', '== PICO8_TYPE_TAG_INSTANCE && pico8_instance_fields(self) == NULL', '== PICO8_TYPE_TAG_INSTANCE && (pico8_instance_fields)(self) == NULL')
sub('src/pico8_planet.c', '    pico8_class_register_named("jo", g_jo_class);', '    pico8_class_register_named("jo", g_jo_class);\n    g_trap_jo = g_jo_class;')
sub('src/pico8_planet.c', '        g_jo_class = NULL;', '        g_jo_class = NULL;\n        g_trap_jo = NULL;')
print("fil piege pose")
```

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que 1a.16b ne convertisse aucune classe, afin que 1a.17 ne contienne que la conversion de `jo`.
2. En tant qu'auteur de 1a.17, je veux que chaque site du port qui lit ou écrit une planète passe déjà par l'API à double forme, afin qu'une struct `jo` et une instance `qh`… y soient servies de la même façon.
3. En tant qu'auteur de 1a.17, je veux l'inventaire complet des sites atteints par une planète (fonction, receveur, champ), afin de savoir quels champs lisent et écrivent les lecteurs et écrivains génériques, et d'appliquer la règle « Champ d'un écrivain générique ».
4. En tant que gardien du filet, je veux qu'aucun `*.sha256` ni aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
5. En tant que gardien de la couverture, je veux une sortie de `make class-coverage-net` identique à la référence, afin de prouver qu'aucune construction n'a changé.
6. En tant que mainteneur, je veux que tout accès à la table de champs d'une instance hors du module des classes passe par `pico8_instance_fields`, qui asserte sur une struct, afin qu'une struct lue comme instance soit un arrêt net et pas une lecture hors de l'objet.
7. En tant que mainteneur, je veux que l'accesseur ne s'applique qu'à `pico8_instance_t`, afin de ne pas mélanger instances et `ob_table` nues (les tables de données restent lues par `pico8_ob_table_*`).
8. En tant que mainteneur, je veux que les 7 initialisations du pointeur `fields` des instances sans classe restent des affectations directes, afin de ne pas inventer un mutateur pour 7 constructions qui seront converties en 1a.30 à 1a.41.
9. En tant que vérificateur, je veux que chaque conversion soit un équivalent exact sur une instance (même appel sous le capot), afin que l'étape ne puisse rien changer à l'exécution.
10. En tant que vérificateur, je veux que les lectures propres seules, accès indexés et passages de table **ne soient pas** convertis en `pico8_entity_get_field`, afin qu'aucune lecture ne se mette à remonter la chaîne.
11. En tant que vérificateur, je veux que la seule garde `fields == NULL` atteinte par une planète prenne la forme aiguillée déjà en place ailleurs, afin qu'elle reste identique sur une instance.
12. En tant que vérificateur, je veux un fil piège qui s'arrête à la première planète atteignant un chemin à simple forme de `src/`, avec fichier et ligne, afin de prouver que l'inventaire est complet sur tout ce que la suite et le filet exécutent.
13. En tant que vérificateur, je veux que le fil piège journalise dans un fichier, et pas seulement sur stderr, afin qu'un déclenchement dans un fils forké ne soit pas masqué.
14. En tant que vérificateur, je veux une contre-épreuve où le piège se déclenche, afin que son silence prouve quelque chose.
15. En tant que vérificateur, je veux reproduire le fil piège à l'identique sur le commit, dans une copie jetable, afin que la preuve ne dépende pas de l'arbre de travail de l'implémenteur.
16. En tant que mainteneur, je veux que le fil piège ne soit jamais commité, afin que le port ne garde aucun code de mesure.
17. En tant que gardien des tests de régression, je veux que `tests/` ait un diff vide hors du test de couche, afin qu'aucun test de régression ne change (§6).
18. En tant que mainteneur, je veux un test de couche qui fixe le contrat de l'accesseur (table propre rendue, arrêt sur une struct), afin que l'assert porteur ne disparaisse pas en silence.
19. En tant que mainteneur, je veux un test de couche qui fixe l'équivalence instance ↔ API d'entité (champ propre, défaut, absent, nil propre qui masque, écriture nil, écrasement), afin que la prémisse de l'étape soit gardée tant que la couche existe.
20. En tant que lecteur du plan, je veux que les sites découverts hors des fichiers nommés soient inscrits à la ligne 1a.16b, avec « découvert à 1a.16b » dans le message de commit, afin que la liste des étapes dise ce que l'étape a touché.
21. En tant que lecteur du code, je veux que les aides gardent leurs signatures, afin que le diff reste chirurgical et que le retypage soit celui de 1a.17.
22. En tant que lecteur du code, je veux que les seuls commentaires ajoutés soient le contrat de l'accesseur et son assert porteur, afin de respecter la règle de CLAUDE.md.
23. En tant qu'autopilote, je veux qu'un site découvert qu'aucune règle du §5 ne couvre, ou un piège qui reste bruyant après 3 tentatives, soit un arrêt, afin de ne pas improviser.

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais. J'ai donc chargé le skill `guideline` et je l'ai appliqué moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **« Peut être une planète » = une instance dont la chaîne `cls` contient la `pico8_class_t` de `jo`**, filles comprises. C'est exactement ce que teste le fil piège du plan. Une fille est une planète : `nl:ly`, `ga:kj` et `mx:kf` convertissent leur `self`.
- **Portée de la partie (2) étendue aux 20 sites découverts** : le fil piège doit être muet, et chaque site découvert relève d'une règle existante (§7 : ajout à la ligne de l'étape, pas d'arrêt). (G) Rejeté : laisser ces sites à 1a.17. Le piège de 1a.16b ne serait alors jamais muet, et 1a.17 porterait de nouveau un diff hors de `jo`.
- **Aiguillages déjà faits (`ke_ks`, `dg`, garde de `bv`) : accesseur seul, pas de conversion.** Le §5 les compte comme double forme (« ou aiguillage sur le tag comme `pico8_ke_ks` »). Les convertir ajouterait un appel générique à l'intérieur d'une branche déjà typée. Le piège les exempte par nom parenthésé, dans le patch non commité seulement.
- **(G) Garde de `particle.c:63`** : forme aiguillée de `probe.c:128` plutôt qu'accesseur nu. Même condition sur une instance, et pas d'assert sur une struct `ga` à 1a.26. Rejeté : supprimer la garde, qui ne peut jamais être vraie pour une instance de classe. Ce serait un changement de code du cart porté, hors de l'étape.
- **(G) Accesseur : fonction, pas macro ni `static inline`**, déclarée dans l'en-tête des classes, définie dans son module. Il suit le modèle de `pico8_instance_get_field`, et le fil piège peut l'envelopper par une macro. Rejeté : un accesseur qui rend `pico8_ob_table_t **` pour couvrir aussi les 7 initialisations. Ce serait une lvalue déguisée pour 7 sites de construction sans classe, qui disparaissent en 1a.30–1a.41.
- **(G) Assert « non-struct », pas « == INSTANCE »** : c'est le texte du plan. Il est cohérent avec l'assert de `pico8_instance_get_field`, et un pointeur `UNKNOWN` reste servi comme aujourd'hui.
- **Accesseur : `src/` seulement.** `tests/` garde ses `->fields` : ce sont des tests de régression (§6), réécrits dans le commit de leur classe. Un diff là serait un `FAILED`. `tools/` ne contient aucun accès.
- **Aides non retypées** : `const pico8_instance_t *` reste le type des paramètres. Le changement se limite à l'appel intérieur.
- **Pas de `TROU-N`** : aucun écart port/cart n'est touché. Si l'implémenteur en découvre un, il applique la règle §7 (préserver, entrée `TROU-N`, `/* TROU-N */` au site).
- **Commentaires** (CLAUDE.md) : à ajouter, seulement le contrat de l'accesseur et le message de son assert. Aucun commentaire existant n'est rendu faux : `entity.h:79` (« never through `->fields` ») et `menu.c:384` restent vrais. Tout le reste, dont l'inventaire et les résultats du piège, va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert.

1. **Tests de couche d'abord** (voir Testing) : l'accesseur n'existe pas, la cible ne compile pas (rouge). Puis l'accesseur, et la cible devient verte.
2. **Partie (1)**, mécanique : section C, et les 5 sites « accesseur seul » de la table A (dont la garde de `particle.c:63`). Le nombre de lignes ne change pas.
3. **Partie (2)** : les 109 conversions de la table A, fichier par fichier, puis les 3 `#include`. Suite verte.
4. **Fil piège** dans une copie jetable : muet, contre-épreuve rouge, copie jetée. Puis `make class-coverage-net` sur l'arbre de travail, comparé à la référence, et la ligne 1a.16b du plan. Commit, avec « découvert à 1a.16b » et le résultat du piège (commande, sortie 0, journal absent, ligne de la contre-épreuve) dans le message.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent les lectures, ce qu'une écriture laisse lire ensuite, l'arrêt sur une struct. Pas l'agencement mémoire.
- **« Tests : aucun » du plan** : au sens du §5, aucun **test de régression** n'est réécrit. Deux tests **de couche** s'ajoutent à `test_pico8_entity_smoke` (§6 : il sera supprimé avec le code qu'il teste, en 1a.42). Ils coûtent peu et fixent la prémisse de l'étape. (G) Rejeté : ne rien ajouter. L'équivalence ne serait alors gardée que par la lecture du code, et l'assert de l'accesseur par rien.
  1. **Contrat de l'accesseur** :
     - sur une instance de classe, `pico8_instance_fields(inst)` rend le même pointeur que `inst->fields`, et une écriture par lui se relit par `pico8_instance_get_field` ;
     - sur une instance sans classe à pile (tag posé à la main), même chose ;
     - sur une fausse struct du fichier (tag `STRUCT`), l'appel meurt d'un assert, dans un fils (`dies_of_an_assert`, déjà présent).
  2. **Équivalence sur une instance** : une classe parente avec défaut `a=1`, une fille qui surcharge `a=2` et ajoute `b=3`, une instance de la fille. Pour chaque nom, `pico8_entity_get_field` et `pico8_instance_get_field` rendent le même code et la même valeur (vérifiés par `kind` et `as`). Cas couverts :
     - `a` (surcharge, 2), `b` (défaut, 3), `z` (absent, 0, `out` intact) ;
     - après `pico8_entity_set_field(inst,"b",REF NULL)` : présent, `REF NULL`, le défaut est masqué, et le nombre d'entrées propres a augmenté de 1 ;
     - après une seconde écriture sur `b` : écrasée en place, nombre d'entrées inchangé ;
     - `pico8_entity_set_field` et `pico8_ob_table_set_name(pico8_instance_fields(inst),…)` sur deux instances jumelles laissent des tables propres identiques (mêmes clés, même ordre, mêmes valeurs).
  - Prior art : `test_set_field_on_both_forms` et `dies_of_an_assert` dans le même fichier.
- **Preuve d'équivalence de l'étape** : les deux tests de couche, la suite complète (dont les 7 tests de régression qui lisent des planètes, en section E), `frames-check` (15 rejeux, 114 sites atteints par `tmp/pico8_replay`), `class-coverage-net` identique, et le fil piège muet avec sa contre-épreuve. Le filet ne voit que l'écran (§3). Ici, le risque d'une valeur non affichée est nul par construction, puisque chaque conversion est le même appel sous le capot. Les tests de couche fixent ce « même appel ».
- **Tests existants** : diff vide pour tout `tests/` hors `test_pico8_entity_smoke.c`. Les valeurs figées des caractérisations (`test_pico8_js`, `ie`, `jc`, `mz`, `bg`, `bg_kj`, `fq`, `qb`, `iv`, `nf`, `je`, `mo`, `mh`) restent inchangées.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`. Les binaires `*_refaudit` compilent.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` (dont `frames-check`) est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0, ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`. Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si une classe est convertie ou change de forme. En particulier :
  - si une nouvelle struct, un nouveau descripteur ou un nouveau `pico8_class_set_struct_new` apparaît ;
  - si un `->defaults` bouge ;
  - si l'enum de méthodes change ;
  - si une signature publique autre que l'ajout de `pico8_instance_fields` change.
- `FAILED:` si le fil piège, reproduit par le vérificateur selon la procédure ci-dessus sur une copie `git archive HEAD`, se déclenche : `make test` ou `make class-coverage-net` échoue sous le piège, ou `tmp/fil_piege.log` existe. C'est une planète atteinte par un chemin à simple forme que l'inventaire ne couvre pas. `FAILED:` aussi si la contre-épreuve ne le fait **pas** se déclencher, ou si le message de commit ne rapporte pas le fil piège (commande, résultat, contre-épreuve).
- `FAILED:` si un test de régression (§6) a un diff non vide, ou si un fichier de `tests/` autre que `test_pico8_entity_smoke.c` (et `Makefile`, s'il le faut) change.
- `FAILED:` si le fil piège est commité : macro, `g_trap_jo`, `PICO8_TRAP_INTERNAL`, journal, ou nom parenthésé `(pico8_instance_fields)`.
- `FAILED:` si une lecture propre seule, un accès indexé, un parcours ou un passage de table (section C) devient `pico8_entity_get_field`/`set_field`. Ce ne sont pas des équivalents.
- `FAILED:` si un site découvert en cours d'étape n'est couvert par aucune règle du §5 et reçoit pourtant un traitement improvisé : c'est un arrêt, pas un correctif.

Critères de réussite (en plus) :

- `grep -n -- '->fields\|\.fields\b' src/*.c src/*.h | grep -v '^src/pico8_class.c'` ne rend que les 7 initialisations et les 2 commentaires de la section D ;
- `grep -c pico8_instance_get_field` par fichier vaut les comptes de la section B ;
- `grep -rn 'pico8_instance_fields' src/` rend 61 lignes (59 appels, plus la déclaration et la définition) ;
- `pico8_entity.c` écrit `pico8_instance_fields(e)` dans la branche instance de `pico8_entity_set_field` ;
- `git diff --stat` ne touche que :
  - dans `src/` : les fichiers des tables A et C, et le module des classes ;
  - dans `tests/` : `tests/test_pico8_entity_smoke.c` ;
  - dans `docs/` : `docs/plan-3.md` (ligne 1a.16b) ;
- le message de commit dit « découvert à 1a.16b ».

## Out of Scope

- La conversion de `jo` (1a.17) : struct, descripteur, constructeur, les dispatchers de `planet.c` qui castent `self` en `pico8_instance_t *`, les `g_jo_class->defaults` (`planet.c:161-171`), et les défauts lus `planet_display.c:184,296,532` et `research_display.c:265-266`.
- Le retypage des aides `field_*` et des signatures en `const void *` ou en type struct.
- La règle « Champ d'un écrivain générique » (membres `kr`, `cc`, `by`, `mi`… de `jo`) : c'est 1a.17 qui l'applique. L'inventaire lui sert d'entrée.
- `kw` écrit en place par `mx:kf` (`planet_family_trade.c:280`) : tranché par le PRD de 1a.17 (§5, « Accès directs aux défauts »).
- `tests/` : les lectures de planète de la section E et tous les `->fields` des tests (1a.17, 1a.26, et les étapes de `lq`/`hy`/`ui`).
- Les 7 initialisations de `fields` des instances sans classe (1a.30 à 1a.41).
- Lancer l'appli raylib : jamais. L'audio : hors périmètre.

## Further Notes

- **Écrivains génériques sur `cg.jo`** relevés par le piège, pour 1a.17 : `hy:lj` écrit `kr` sur chaque `jo` (`economy.c:242`) ; `bd` écrit `cc` et `by` (`economy_tiers.c:92,107`) ; `bb` écrit `mi` (`economy_tiers.c:218`) ; `dg` écrit `t` (membre de `pico8_ke_t`). Écritures des méthodes propres d'une fille : `qw` (`nl:ly`), `p_` (`ga:kj`), `s` (`nr:hb`), `ht` (`mx:kf`), `qy` (`f_` sur `ga`).
- **Lecteurs génériques de planète hors de l'arbre `jo`** : `pz` lit `ca`/`ep`/`eo`/`hi`/`ff` sur la planète passée par `jo:gh`/`jo:fe`, et `mz` lit `p`/`me` sur ses extrémités. Après 1a.17, ils reçoivent une struct `jo` et doivent trouver ces noms par le lecteur du descripteur de `jo`.
- Le piège de mesure a été lancé deux fois. La première passe a donné 7 faux positifs (`research.c`) par réutilisation d'adresse d'une `pico8_class_t` libérée. C'est la raison de la remise à NULL dans le script : ne pas la retirer.
- Mesures : grep sur `src/`, `tests/`, `tools/` à 6a7474b, et lecture de `pico8_entity.c:173-204`, `pico8_class.c:313-333`, `pico8_class.h`, `pico8_ob.c:88-126`, `pico8_type_tag.h`, et des fonctions de la table A. Piège de mesure et preuve de faisabilité dans des copies `git archive` jetables, jamais dans le dépôt.
