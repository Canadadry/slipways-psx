---
title: "Plan 3, étape 1a.29 : `pj` (le panneau de score) devient une struct fille de `ke` ; ses défauts modifiés en place `ou`/`ea`/`cr` passent dans un état statique partagé de la classe, jamais dans un membre"
description: "Le panneau de score `pj` est construit 19 fois sur le filet, toujours par `pico8_pj_new(NULL)` (`cart.c:205`). Il passe en struct `pico8_pj_t` (`pico8_ke_t`, puis `fa`, `gs`, `i_`, `ed`, `c_`, `gz`), au descripteur de 4 méthodes (`hb`, `kj`, `hk`, `ph`). Ses trois tables modifiées en place (`ou` par `pj:hb`, `ea` par `pj:hk`, `cr` par `cr()`) deviennent un état statique de la classe, avec les mêmes REF que la table de la classe. Les trois sites qui lisent `->defaults` passent par des accesseurs. `game_end` et `score_panel_close` sont réécrits pour lire la struct, puis prouvés rouges."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.29 — `pj` : `ke` (`score_panel.c:348`), défauts mutés en place `ou`/`ea`/`cr` (`score_panel.c:245`, `score_panel_display.c:103-109`, `menu.c:241`). Tests : `game_end`, `score_panel_close`.

Les numéros de ligne ont un peu dérivé. À HEAD a4364b3 :
- **`pico8_score_panel.c`** :
  - la déclaration `g_pj_class = pico8_class_declare(pico8_ke_class(), …)` est à `:350-360`, son nom de couverture à `:361` ;
  - les 4 installations `hb`/`kj`/`hk`/`ph` dans `g_pj_class->defaults` sont à `:363-366` ;
  - `pj_class_field_ref_table` (lecture de `->defaults`) est à `:99` ;
  - `pico8_pj_hb` est à `:179`. Il réécrit `pj.ou[i][1]` à `:238-249` (le `:245` du plan) ;
  - `pico8_pj_kj` est à `:264`. Il lit `pj.i_` et `pj.ou` à `:265-266`, lit `self.gz` à `:278` et l'écrit à `:283` ;
  - les 4 dispatchers (`static`) sont à `:255`, `:292`, `:300`, `:312` ;
  - `pico8_pj_new` est à `:396`, `pico8_ju`/`pico8_pj_set_ju` à `:408-414`.
- **`pico8_score_panel_display.c`** :
  - une seconde copie de `pj_class_field_ref_table` est à `:53` ;
  - `pico8_pj_hk` est à `:79`. Il lit `pj.ea` et `pj.gs` à `:80-81`. Il réécrit `ea[2][1]`, pose ou retire `ea[3]` et pose `ea.lh` à `:101-112` (le `:103-109` du plan). Il construit le `mh` à `:115` ;
  - le `my` de `de` (qui bascule `ju.c_`) est à `:158` ;
  - le `lh` de la boîte des scores (qui lit `ju.c_`) est à `:174` ;
  - `pico8_pj_ph` est à `:274`. Il lit `gz` (`:278`), `c_` (`:311`) et `ed` (`:318`). Il écrit `c_`/`gz`/`fa` (`:303-305`) et `ed` (`:345`).
- **`pico8_menu.c`** :
  - `pj_cr_table` (troisième lecture de `->defaults`) est à `:54` ;
  - `pico8_cr` est à `:150`. Il lit `pj.cr` à `:204`, y écrit `ep` à `:241` (le `:241` du plan), puis l'enveloppe et appelle `pz` à `:243-247` ;
  - le `my` de `ix[3]` (qui écrit `ju.c_`) est à `:334`.
- **Ailleurs** :
  - `cart.c:100` asserte que le parent de la classe `pj` est `ke` ;
  - `cart.c:205` fait `pico8_pj_set_ju(pico8_pj_new(NULL))`.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Défauts mutés sur la classe** : `ie.gy`, `ie.id` […] deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** (connus : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`) […].
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode) […].

`pj` est une **feuille** sous `ke`, sans descendante. Elle ne ferme aucune parente : `ke` garde `ui` (1a.31). C'est la situation de `g_` à 1a.27 (racine sous `ke`), sans fille à servir. La différence de fond est ailleurs : `pj` porte **trois tables partagées par toutes ses instances et modifiées en place**. `pj.ou[i][1]` est réécrit par `pj:hb`. `pj.ea[2][1]`, `pj.ea[3]` et `pj.ea.lh` le sont par `pj:hk`. `pj.cr.ep` l'est par `cr()`.

Notes du plan qui échoient ici :

- **1a.4** : `mh` « construite avec le défaut muté `pj.ea` ». `pico8_mh_new(iw)` reçoit toujours la table partagée, jamais une copie.
- **1a.5** : `score_panel_close` a été réécrit pour lire le `fq` (`fq_alive`). Sa lecture de `ju.c_` passe à la forme struct ici.
- **1a.28** : « restent à double forme : […] `pz` (1a.30, 1a.41), `ke_ks`/`dg` (1a.31) ». Ce commit ne les touche pas. `dg` reçoit une struct `pj` par `hv`, `ke_ks` n'est jamais appelé sur `pj`.

§6 : la ligne nomme **`game_end`** et **`score_panel_close`**. Tous deux lisent `ju.c_` par `pico8_instance_get_field(pico8_ju(), …)`. Ils sont réécrits pour lire la struct, et chacun est prouvé rouge avec un bug remis. Tous les autres tests de régression gardent un diff vide.

§7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` : `pj  19  15/15`, et toutes les autres lignes inchangées ;
- **preuve des tests par `gcov` et 2 à 3 mutations** ;
- **note « Découvert à 1a.29 »** réduite à ce qui change une étape suivante ;
- un nouveau test dont le rejeu dépasse 30 s va dans `UNIT_TESTS_SLOW` ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.28 ont posé (jusqu'à e13f975), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, `field`, `set_field`).
  - L'enum contient déjà `HB`, `KJ`, `HK`, `PH` : **elle ne change pas**.
  - `pico8_entity_get_field` rend d'abord l'entrée du descripteur, sinon le lecteur ; un membre nil est absent.
  - `set_field` NULL est permis pour une classe qu'aucun écrivain générique n'atteint (précédents `nf`, `mh`, `iv`).
- `pico8_ke_t`, `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq` (1a.2). `dg` écrit `t` directement dans le `pico8_ke_t` d'une struct.
- **État statique partagé d'un défaut muté** :
  - `ie.gy`/`ie.id` (1a.14) : statiques du module, sans membre. Le lecteur de `ie` les rend quand on les lit sur une icône ;
  - `nf.l`/`nf.r` (1a.6) : même forme.
- **Défaut lu sur la classe** : `bg.pl` (1a.12), `jo.dz`/`cq`/`ka`/`em` (1a.17). Le membre porte la REF de la classe, et les lecteurs du cart (`bg.pl`, `jo.dz`) lisent l'état statique.
- **REF prises dans la table de la classe, jamais re-parsées** : `g_` (1a.27), `fv` (1a.28). Un tableau `{nom, offset, valeur}` est rempli une fois à l'init par `pico8_class_get_field`, avec un assert « chaque défaut est une clé de la spec », puis remis à nil au shutdown.
- Constructeur `static` posé par `pico8_class_set_struct_new`. Il asserte sur toute clé non mesurée, enregistre la couverture et appelle `pico8_ke_struct_pq` (précédents `g_`, `fv`).
- **Preuve rouge d'un test de régression réécrit** : `score_panel_close` à 1a.5 (`de.my` rendant nil) ; `planet_kj_dispatch` à 1a.8 (bug de référence, faute d'historique).

### Ce que fait le cart

`pico8/lua.lua` :

```
:153-165   lt : add(hv,e) ; cv[p] pour p ∈ bm={kj,ev,hd,hk,hb} présents ; cg[t] pour t ∈ e.nu
:168-177   dg : fn=qc[qc.me] ; fn(qc,qc.t) ; qc.p_ → he ; qc.t+=1
:179-181   ke=kv:jr([[me="ph",t=0]]) ; ke.pq=lt
:188-199   dc(nt) : ft(fl,qc.fa,qc) ; qc[nt](qc,qc.ra)
:815-846   pj=ke:jr([[ fa=10, ea=o(o(42,43,1,fn="br"),o("",64,40,6,0.5,fn="qr")),
                      gs=o(…5 lignes…), cr=o(hi=v(0,11),gq=v(0,11),ca=10),
                      i_=o(…5 lignes…), ou=o(…7 lignes o(f,x,y,c,fn="qr")…), ed=1 ]])
:847-852   pj:hb()  : ou={…7 valeurs de fm…} ; for i,p in pairs(ou) do pj.ou[i][1]=p end ; eg()
:853-857   pj:kj()  : bl(la(pj.i_,pj.ou)) ; if self.gz then self.gz+=0.2 … rk(…) rk(…) end
:858-869   pj:hk(yr): iw,pp=pj.ea,pj.gs[3426-yr] ; iw[2][1],iw[3]="year "..yr,pp or nil ;
                      if pp then local t=90 function iw.lh() … end end ; mh(iw)
:870-900   pj:ph()  : if fm.hl and not self.gz then fq() js(…) js(…,de) self.c_,self.gz,self.fa=true,0,13 end ;
                      if self.c_ and kc~=self.ed then … self.ed=mh(iw) end
:901-903   de={my=function() ju.c_ = not ju.c_ return true end}
:955-964   cr() : … pj.cr.ep="level "..tl.." (…)" ; pz(pj.cr,bs)
:984-985   ix[3].my=function() ju.c_=true fq(de) end
:1182      eu : fm,ju=hy(),pj()
:1214,1236 hy:hk → e:hk(self.yr) pour cv.hk ; hy:lj → e:hb() pour cv.hb
```

Points décisifs :

- **Une seule voie de construction** : `pj()` sans table, une fois par `eu()` (`cart.c:205`).
- **Walk `pq`** : `pj` n'a pas de `pq` propre. Seul `ke:pq` (`lt`) court.
- **Les 5 tables de défaut sont lues sur la classe, jamais sur `self`** : `pj.i_`, `pj.ou` (`kj`, `hb`), `pj.ea`, `pj.gs` (`hk`), `pj.cr` (`cr`). Trois sont modifiées en place : `ou`, `ea`, `cr`. `gs` et `i_` ne sont que lues.
- **Partage entre instances** : une nouvelle partie construit un nouveau `pj` (5 dans `long_session`). Il retrouve `ou`, `ea` et `cr` tels que la partie précédente les a laissés : `ea[3]`, `ea.lh`, `ea[2][1]`, `cr.ep` et les `ou[i][1]`. C'est ce partage que ce commit doit garder.
- **Champs d'instance** : `fa` (défaut 10, réécrit 13 par `ph`), `ed` (défaut 1, réécrit par la boîte `mh`), `c_` (écrit par `ph`, `de.my`, `ix[3].my`), `gz` (écrit par `ph`, avancé par `kj`).
- **`nu` absent** : `pj` n'entre dans aucun `cg`. `lt` l'inscrit dans `cv.kj`, `cv.hk` et `cv.hb` (pas `ev`, pas `hd`).

### Emprise réelle de `pj` (mesurée)

**Méthode.**
- Grep de `src/`, `tests/`, `tools/` et du `Makefile` à a4364b3.
- **Sonde** sur une copie `git archive HEAD` jetable du scratchpad, supprimée après usage. Binaire de couverture compilé en `-O0 -fno-omit-frame-pointer -no-pie`, adresses de retour résolues par `addr2line`. Elle journalise :
  - chaque `pico8_instance_get_field` sur une instance de la classe `pj` (nom, source : propre, classe, absent) ;
  - chaque `pico8_instance_fields` sur une telle instance ;
  - chaque `pico8_entity_set_field` sur une telle instance ;
  - chaque `pico8_mv` sur une telle instance ;
  - chaque instanciation de `pj`, avec le nombre de clés ;
  - chaque lecture de `->defaults` par les trois helpers (`pj_class_field_ref_table` ×2, `pj_cr_table`) ;
  - le `self` reçu par `pico8_pj_kj`, `pico8_pj_ph` et les dispatchers `hb`/`hk`.
- Rejoués : les 17 fichiers `tests/fixtures/*events.json` et `events_recorded_1790198294.json` (`--from-title` pour les 4 enregistrements), **18 rejeux en sortie 0**.
- **Fil piège** : `make unit` dans la même copie, même journal, par nom de processus. Sortie 0.

**Constructions d'un `pj`** : **22 sur les 18 rejeux**, dont **19 sur le filet** (15/15, 5 dans `long_session`) et 3 hors filet (`cheat_code`, `demo`, `score_panel_close`). Toutes par `pico8_pj_new(NULL)` depuis `pico8_eu` (`cart.c:205`), **sans clé**. Dans les TU aussi : aucune instanciation avec une clé.

**Noms lus sur un `pj`** (18 rejeux ; les TU n'ajoutent que des absents lus par des prédicats de test qui parcourent `hv`) :
- dans la classe `pj` :
  - `fa` (`dc`), `ed` (`ph`) ;
  - `kj`, `ph`, `hb`, `hk` (installations, par `mv` et `lt`) ;
- dans `ke` : `me` (`dg`), `t` (`dg`, avant la première écriture) ;
- sur l'instance : `t`, `c_` (`ph`, `de.my`, le `lh` de la boîte des scores), `gz` (`ph`, `kj`), `ed` (`ph`), `fa` (`dc`, après `ph`) ;
- absents : `c_`, `gz` (avant l'ouverture), `ra` (`dc`), `p_` (`dg`), `nu`, `ev`, `hd` (`lt`), `fx` (prédicat de `mz:pq`, `wormhole.c:80`, qui parcourt `hv`) ; dans les TU seulement : `fz`, `ke`, `mb`, `nl`.

**`ou`, `ea`, `cr`, `i_`, `gs` ne sont jamais lus sur une instance.** Toutes leurs lectures passent par les trois helpers de `->defaults` :

| table | lecteur (fonction) | appels |
|---|---|---|
| `ou` | `pico8_pj_kj` | 169 004 |
| `ou` | `pico8_pj_hb` (écrit `ou[i][1]`) | 586 |
| `i_` | `pico8_pj_kj` | 169 004 |
| `ea` | `pico8_pj_hk` (écrit `ea[2][1]`, `ea[3]`, `ea.lh`) | 114 |
| `gs` | `pico8_pj_hk` | 114 |
| `cr` | `pico8_cr` (écrit `cr.ep`) | 45 (`tech_screen_invent`, et en TU : `ga`, `geoharvest`, `jc`, `js`, `nl`, `qb`, `tech`) |

**Champs écrits sur un `pj`** : aucun par `pico8_entity_set_field`, dans les rejeux comme dans les TU. **Aucun écrivain générique n'atteint `pj`.** Toutes les écritures passent par `pico8_instance_fields` :
- `t` (`dg`, `bootstrap.c:101`) ;
- `c_`, `gz`, `fa` (`ph`, `score_panel_display.c:303-305`) ;
- `ed` (`ph`, `:345`) ;
- `gz` (`kj`, `score_panel.c:283`) ;
- `c_` (`de.my`, `score_panel_display.c:167`) ;
- `c_` (`ix[3].my`, `menu.c:341`).

**Méthodes appelées sur un `pj`** :
- `kj` par `dc` (169 004) ;
- `ph` par `dg` (168 997) ;
- `hb` par `hy:lj` (586) ;
- `hk` par `hy:hk` (114).

Le `self` reçu par `pico8_pj_kj`/`pico8_pj_ph` est **toujours un `pj`**, dans les rejeux comme dans les TU. Les dispatchers `hb`/`hk` ignorent `self`.

**→ Membres** :
- les 4 défauts **non modifiés en place** : `fa`, `gs`, `i_`, `ed` (ordre de la spec) ;
- les 2 champs écrits : `c_`, `gz`.

**→ État statique partagé, sans membre** : `ou`, `ea`, `cr` (modifiés en place). `gs` et `i_` sont aussi tenus dans l'état statique, parce que le cart les lit sur la classe (`pj.gs`, `pj.i_`), comme `bg.pl`.

**Tests qui touchent `pj`** (grep) :
- `test_pico8_game_end:44-50` et `test_pico8_score_panel_close:38-47` lisent `c_` par `pico8_instance_get_field(pico8_ju(), …)` ;
- `test_pico8_mh:248-295` :
  - `set_c_` écrit `ju->fields` ;
  - `game_ju` rend `pico8_instance_t *` ;
  - le test écrit `gz` par `ju->fields`, lit `ed` par `pico8_instance_get_field` et appelle `pico8_pj_ph(ju)`.
- `test_pico8_mh:187` appelle `pico8_pj_hk` (signature inchangée) ;
- `test_pico8_fq:386` utilise `pico8_de()` (inchangé) ;
- `test_pico8_js` (`test_pj`), `top_menubar`, `score_panel_stars`, `playability` lisent l'écran ou parcourent `hv`/`cv` par l'API à double forme : inchangés.

Aucun test ne lit `pico8_pj_class()` ni `->defaults` de `pj`.

**Outils** :
- `tools/float_audit_c1.txt` nomme `pico8_pj_hb | fm_lo`, `pico8_pj_hk | yr`, `pico8_pj_ph_map | score_val.as.fixed`, `pico8_eg`, `get_by_value` et `pico8_cr | tl`. Aucune de ces expressions ne change ;
- `pico8_menu.c` porte deux `PICO8_REF_AS_LIST_CONST` (`:167`, `:198`), hors du chemin de `cr` : inchangés.

## Solution

1. **Enum : inchangée.**
2. **Struct `pj`**, publique dans `pico8_score_panel.h` : `pico8_ke_t` en premier membre, puis `fa`, `gs`, `i_`, `ed` (ordre de la spec), puis `c_`, `gz`.
3. **État statique de la classe**, lu **une fois** à l'init dans la table propre de la classe `pj`, jamais re-parsé. Il porte les 7 défauts (`fa`, `ea`, `gs`, `cr`, `i_`, `ou`, `ed`), avec un assert « chaque défaut est une clé de la spec de `pj` ».
   - `ea`, `gs`, `cr`, `i_`, `ou` sont **les mêmes REF** que `g_pj_class->defaults`. Un assert (message porteur) vérifie qu'elles sont des `ob_table`, ce que les helpers vérifiaient à chaque lecture ;
   - `pico8_score_panel_shutdown` remet l'état à nil avant de libérer la classe, sans libérer aucune table.
4. **Accesseurs publics de l'état partagé** (`pico8_score_panel.h`), qui remplacent les trois helpers de `->defaults` :
   - `pico8_pj_ou()`, `pico8_pj_ea()`, `pico8_pj_cr()` rendent `pico8_ob_table_t *`, modifiable : ce sont les tables de la classe, que `hb`, `hk` et `cr()` modifient en place ;
   - `pico8_pj_i_()` et `pico8_pj_gs()` rendent `const pico8_ob_table_t *` ;
   - chacun rend NULL hors init..shutdown.

   Qui les utilise :
   - `pico8_pj_hb` et `pico8_pj_kj` (`ou`, `i_`) ;
   - `pico8_pj_hk` (`ea`, `gs`) ;
   - `pico8_cr` (`cr`) : `pj_cr_table` de `menu.c` est retiré, et l'enveloppe de `pz` (1a.30) reçoit la table de l'accesseur, sans autre changement.

   Les deux `pj_class_field_ref_table` sont retirés.
5. **Constructeur de `pj`** (`static`, posé par `set_struct_new`) :
   1. alloue un `pico8_pj_t` ;
   2. `pico8_ke_struct_init(&self->ke, &<descripteur de pj>)` ;
   3. met `c_`/`gz` à nil et copie `fa`, `gs`, `i_`, `ed` depuis l'état statique ;
   4. **asserte** qu'aucune clé n'est passée (message porteur, arrêt §7 : la seule voie mesurée est `pico8_pj_new(NULL)`) ;
   5. enregistre `"pj"` au crochet de couverture ;
   6. appelle `pico8_ke_struct_pq`.
6. **Descripteur de `pj`** (`static const`) :
   - 4 entrées : `HB`, `KJ`, `HK`, `PH` = les 4 dispatchers existants. Les autres sont NULL ;
   - **lecteur** :
     - les 6 membres, nil = absent ;
     - puis `ou`, `ea`, `cr` rendus depuis l'état statique (précédent `ie.gy`) : lus sur un `pj`, ils répondent la table de la classe, comme la chaîne aujourd'hui ;
     - puis `pico8_ke_field` ;
   - **écrivain : NULL**. Aucun écrivain générique n'atteint `pj` (mesuré). `ph`, `kj`, `de.my` et `ix[3].my` écrivent le membre directement ;
   - accesseur public `pico8_pj_struct_class()`.
7. **Méthodes et fermetures sur la struct** :
   - `pico8_pj_kj(gfx, mem, pico8_pj_t *self)` lit et écrit `self->gz` ;
   - `pico8_pj_ph(pico8_pj_t *self)` lit `gz`/`c_`/`ed` et écrit `c_`/`gz`/`fa`/`ed` dans les membres. Mêmes gardes : présence de `gz`, vérité de `c_`, `ed` comparé à `kc` seulement s'il est une REF ;
   - `pico8_pj_hb` et `pico8_pj_hk` : corps inchangés, sauf la source des tables (accesseurs) ;
   - `pico8_pj_de_my_dispatch`, `pico8_pj_ph_lh_dispatch` (`score_panel_display.c`) et `pico8_ix3_my_dispatch` (`menu.c`) lisent ou écrivent `pico8_ju()->c_` ;
   - les dispatchers ne changent que leur cast interne.
   - **Retirés** de `pico8_score_panel_init` : les 4 installations. La classe `pj` ne garde que ses défauts de spec, et reste enregistrée jusqu'à 1a.42 (`cart.c:100` l'asserte).
8. **Types publics** :
   - `pico8_pj_new` rend `pico8_pj_t *` ;
   - `pico8_ju()` rend `pico8_pj_t *` et `pico8_pj_set_ju` le prend.

   Les commentaires de `pico8_score_panel.h` disent le contrat réel :
   - aucune clé acceptée ;
   - le panneau s'enregistre par `lt` dans `hv`, `cv.kj`, `cv.hk`, `cv.hb`, et dans aucun `cg` ;
   - rien ne le libère (TROU-7) ;
   - les tables de l'état partagé appartiennent à la classe, et `hb`/`hk`/`cr()` les modifient en place.
9. **Tests** : nouveau `test_pico8_pj` (caractérisation dirigée et rejeu court) ; `game_end` et `score_panel_close` réécrits (lecture seulement) et prouvés rouges ; `test_pico8_mh` adapté dans sa forme seulement. Voir Testing Decisions.
10. **Plan** : à la ligne 1a.29 de `docs/plan-3.md`, ajouter « Découvert à 1a.29 : … », réduit à ce qui change une étape suivante (§7) :
    - `ou`/`ea`/`cr` : état statique sans membre, lus par les accesseurs `pico8_pj_ou`/`ea`/`cr`. `cr` arrive à l'enveloppe de `pz` de 1a.30 par `pico8_pj_cr()` ;
    - `gs`/`i_` : lus sur la classe, par l'état statique ;
    - pas d'écrivain (aucun écrivain générique n'atteint `pj`) ;
    - enum inchangée.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `pj` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser l'enveloppe de `pz` (1a.30), `ui` (1a.31) et les racines à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `pj` mesurée (constructions et clés, noms lus, champs écrits, lecteurs de `->defaults`, méthodes appelées, `self` reçus), afin que chaque membre et chaque état statique soit justifié par une mesure.
3. En tant que lecteur du code, je veux une struct `pj` dont le premier membre est `pico8_ke_t` et qui n'ajoute que les 6 noms mesurés, minifiés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux que `ou`, `ea` et `cr` soient un état statique de la classe et jamais un membre (§5, « Défauts mutés sur la classe »), afin qu'une modification en place reste vue par toutes les instances, passées et futures.
5. En tant que joueur qui relance une partie, je veux que le nouveau panneau retrouve la barre du haut (`ou`), la boîte de l'année (`ea`) et l'écran technologie (`cr`) tels que la partie précédente les a laissés, afin que rien ne change entre deux parties par rapport au cart.
6. En tant que mainteneur, je veux que les tables de l'état partagé soient les REF de la table de la classe, lues une fois, jamais re-parsées, afin que leur identité soit celle d'aujourd'hui et que la caractérisation la fige.
7. En tant que mainteneur, je veux qu'aucun site hors de `pico8_class.c` ne lise plus `pico8_pj_class()->defaults`, afin que la règle « Accès directs aux défauts » soit tenue et que 1a.42 n'ait que la spec à supprimer.
8. En tant que mainteneur, je veux des accesseurs publics `pico8_pj_ou`/`ea`/`cr` (modifiables) et `pico8_pj_i_`/`gs` (en lecture), afin que `hb`, `kj`, `hk` et `cr()` lisent la même table sans copier le helper dans trois fichiers.
9. En tant que gardien du filet, je veux que le panneau s'enregistre dans `hv`, `cv.kj`, `cv.hk`, `cv.hb` au même rang qu'avant et dans aucun `cg`, afin que `dc`, `dg`, `hy:hk` et `hy:lj` l'atteignent dans le même ordre.
10. En tant que gardien du filet, je veux que `kj` dessine `i_` puis `ou` et, une fois le panneau ouvert, le rideau à `gz` croissant, afin que l'écran ne change pas d'un pixel.
11. En tant que gardien du filet, je veux qu'à la fin de la partie `ph` ouvre le panneau (`fq`, deux `js`, `c_` vrai, `gz`=0, `fa`=13) puis construise la boîte des scores une fois tant qu'elle est `ed`, afin que `game_end` reste identique.
12. En tant que joueur, je veux que le bouton `ix[3]` ouvre le panneau et qu'un clic le referme (`de.my`), afin que `score_panel_close` reste vrai.
13. En tant que mainteneur, je veux que lire `ou`, `ea` ou `cr` sur un `pj` rende la table partagée (précédent `ie.gy`), afin qu'un lecteur générique voie ce que la chaîne de classes lui rendait.
14. En tant que mainteneur, je veux que le descripteur ait `HB`/`KJ`/`HK`/`PH` et rien d'autre, et pas d'écrivain, afin qu'une écriture générique jamais mesurée soit un arrêt (§7).
15. En tant que mainteneur, je veux qu'une clé de construction passée à `pico8_pj_new` déclenche une assertion porteuse, afin qu'une clé jamais mesurée soit un arrêt.
16. En tant que mainteneur, je veux que la classe `pj` n'ait plus d'installation, afin que rien ne puisse plus y résoudre une méthode.
17. En tant que mainteneur, je veux que `pico8_pj_new`, `pico8_ju`, `pico8_pj_set_ju`, `pico8_pj_kj` et `pico8_pj_ph` prennent ou rendent `pico8_pj_t *`, afin que le type dise ce que la mesure a prouvé : leur `self` est toujours un `pj`.
18. En tant que gardien de la couverture, je veux que le constructeur enregistre `"pj"` une fois par panneau, afin que `class-coverage-net` soit identique ligne à ligne (`pj 19 15/15`).
19. En tant que gardien des tests de régression, je veux que `game_end` et `score_panel_close` ne changent que leur façon de lire `c_`, et qu'ils soient prouvés rouges avec leur bug remis, afin que la réécriture ne les ait pas vidés de leur objet.
20. En tant que vérificateur, je veux une caractérisation de `pj` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas : partage entre deux instances, identité des tables, `ea.lh`, `cr.ep`.
21. En tant que vérificateur, je veux que la caractérisation rejoue `game_end` frame à frame, afin que l'ouverture du panneau et la boîte des scores soient figées pas à pas.
22. En tant que vérificateur, je veux un rapport `gcov` montrant chaque ligne ajoutée ou modifiée exécutée, et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
23. En tant que gardien des caractérisations, je veux que `test_pico8_mh` ne change que sa façon d'obtenir `ju`, d'y poser `c_`/`gz` et d'y lire `ed`, sans valeur figée touchée, afin que 1a.4 reste une preuve.
24. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne des références de couverture ni de `float_audit_c1.txt` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
25. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C, un piège ou un `TROU-N` nu (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
26. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
27. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** :
  - le panneau de score : struct, état partagé, accesseurs, constructeur, descripteur, lecteur, `pico8_pj_new`, `ju`, `kj`, installations retirées ;
  - son affichage : `hk` et `ph` sur les accesseurs et la struct, `de.my` et le `lh` des scores sur `ju->c_` ;
  - le module de menus : `pico8_cr` sur `pico8_pj_cr()`, `ix[3].my` sur `ju->c_`.

  Rien d'autre dans `src/`. `cart.c:205` ne change pas de texte. Pas de changement dans `ke`, `entity`, `bootstrap` (`dg`), `economy` (`hy:hk`/`hy:lj`), `msgbox` (`mh`, `pz`), `ui_button`, `hud`, `pico8_struct.h` (enum).
- **`ou`/`ea`/`cr` : état statique, sans membre** (§5, lecture littérale : « jamais un champ d'instance »). Précédents `ie.gy`/`ie.id`, `nf.l`/`nf.r`.
  - Ce n'est pas la forme de `ga.by` ni de `jo.kw`, qui ont un membre : ces deux champs sont aussi **réécrits sur l'instance** (`economy_tiers.c`, `lz`). Ici, aucun écrivain n'écrit `ou`/`ea`/`cr` sur un `pj` (mesuré), et le cart ne les lit que sur la classe.
- **Le lecteur rend `ou`/`ea`/`cr` depuis l'état statique** (G). Aucun lecteur ne les lit sur un `pj` aujourd'hui, mais la chaîne les rendait. Précédent `ie` : « `gy` and `id` read off an icon answer the shared state ». Rejeté : les déclarer absents. Ce serait une différence avec la chaîne, et la caractérisation lit ces noms sur `ju` pour prouver l'identité.
- **`gs`, `i_` : membres portant la REF de la classe, lus par l'état statique** (G). La règle générale donne un membre à chaque défaut non modifié en place. Le cart lit `pj.gs`/`pj.i_` sur la classe : `hk` n'a d'ailleurs pas de `self` utile. Précédent `bg.pl`. Rejeté : lire `self->i_` dans `kj`. C'est la même valeur, mais cela s'écarte du cart pour rien.
- **REF prises dans la table de la classe, alors que `pj` n'a pas de descendante** (G). La règle « Accès directs aux défauts » ne l'exige que pour une classe à descendants non convertis. On le fait quand même : la classe reste enregistrée jusqu'à 1a.42, et ses tables sont celles que `hb`/`hk`/`cr()` modifient. Garder une seule table par nom évite deux copies qui divergeraient. Précédents `g_`, `fv`. Rejeté : re-parser la spec (identité changée, et deux tables `ou` vivantes).
- **Défauts lus dans la table propre de `pj`, pas par la chaîne** (G) : `pj` ne recouvre aucun défaut de `ke` (`me`, `t`). Le tableau d'état n'y cherche que les 7 noms de sa spec.
- **Pas d'écrivain** (G) : aucune écriture générique mesurée (0 `pico8_entity_set_field` sur un `pj`, rejeux et TU). Précédents `nf`, `mh`, `iv`. Toute écriture générique future avorte donc dans `pico8_entity_set_field`, et c'est l'arrêt voulu.
- **Aucune clé acceptée** (G) : une seule voie, `pico8_pj_new(NULL)`, mesurée dans les rejeux et les TU. Le paramètre reste, pour la forme commune de `set_struct_new`.
- **Constructeur sans ordre à préserver au-delà de « défauts avant `lt` »** : `pj` n'a pas de `pq` propre. Défauts copiés après `ke:pq` est un **mutant équivalent**. `lt` ne lit que les 5 noms de `bm`, servis par le descripteur, et `nu`, absent. On ne l'utilise pas comme mutation (leçon de 1a.23-1a.28).
- **Types des fonctions propres** (G) : `pico8_pj_kj`, `pico8_pj_ph`, `pico8_ju`, `pico8_pj_set_ju`, `pico8_pj_new` passent à `pico8_pj_t *`. Leur `self` est toujours un `pj` (mesuré). Ce n'est pas la situation des fonctions partagées de `g_` à 1a.28, qui recevaient plusieurs formes.
- **`ke_ks`, `dg`, `pz` ne changent pas** : fonctions de `ke` (dernière descendante `ui`, 1a.31) ; `pz` est à double forme jusqu'à 1a.30/1a.41. L'enveloppe de `pico8_cr` reste une instance sans classe sur la pile, qui porte la table `cr` partagée.
- **Tests de régression** :
  - `game_end` et `score_panel_close` lisent `c_` par `pico8_entity_get_field(pico8_ju(), "c_", …)`, qui ne dépend pas de la forme. `ju` est déclaré `const void *`. Rien d'autre ne change (frames, fixture, asserts) ;
  - **preuves rouges** :
    - `score_panel_close` : `pico8_pj_de_my_dispatch` finit par `return pico8_value_ref(NULL);` au lieu de `pico8_value_bool(1)`. C'est le bug d'origine (`docs/plan-2.md:422`), rouge à 1a.5 sur `!fq_alive()` à `CLOSED_FRAME` ;
    - `game_end` n'a pas de bug d'historique (test de scénario). **Bug de référence** (G, précédent `planet_kj_dispatch` 1a.8) : `pico8_pj_ph` n'écrit plus `c_` à l'ouverture. Attendu : l'assert final `ju_c_()`.

    Dans les deux cas : bug remis sur le code final, test rouge, correctif restauré, vert. La commande et le mode d'échec observé sont notés dans le commit. Si le bug remis ne rend pas le test rouge, c'est un arrêt (§7), pas un test durci.
- **`test_pico8_mh`** : `game_ju` rend `pico8_pj_t *`. `set_c_` et la pose de `gz` écrivent le membre directement (pas d'écrivain, précédent 1a.27 : « écriture directe dans le membre, pas élargissement de l'écrivain »). `ed` est lu par `pico8_entity_get_field`. Aucune valeur figée ne bouge.
- **Sémantique de nil** : `c_` et `gz` sont absents avant l'ouverture, sous les deux formes. `fa` et `ed` ne sont jamais nil. TROU-9 n'intervient pas : aucun nil n'est écrit sur un `pj` (mesuré).
- **Clés dynamiques** : aucune. `ea.lh` et `cr.ep` sont des clés nommées fixes dans des tables de données, pas des champs d'entité.
- **TROU** : aucun écart nouveau relevé. `ea.lh` laissé périmé quand `pp` est faux, comme dans le cart, reste tel quel (commentaire existant de `pico8_score_panel_display.h`).
- **Commentaires** (CLAUDE.md) :
  - à ajouter :
    - le contrat de la struct `pj` (jamais libérée, TROU-7 ; `gs`/`i_` sont les tables de la classe, empruntées) ;
    - le contrat des 5 accesseurs (tables de la classe, NULL hors init..shutdown ; `ou`/`ea`/`cr` modifiées en place par `hb`/`hk`/`cr()`, partagées par tous les `pj`) ;
    - l'assert de clé, et celui du tag `ob_table` à l'init (message porteur) ;
  - à corriger ou retirer, rendus faux :
    - `pico8_score_panel.h` sur `pico8_pj_new` (« `overrides` is copied, not kept ») ;
    - « a NULL instance argument is a safe no-op, and an absent or wrong-kind field reads as absent » (à revoir pour la struct) ;
    - les deux commentaires « Reads `pj`'s OWN class-level table », retirés avec leurs helpers ;
    - le commentaire de `pj_cr_table` dans `menu.c`, retiré avec lui ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert (et `make e2e` au dernier). La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_pj` sur l'ancien code** (voir Testing Decisions), valeurs figées. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si le `pj` d'une partie n'a pas `fa`=10 et `ed`=1, ou si `ou`/`ea`/`cr` lus sur lui ne sont pas les REF de la classe : la mesure est fausse et doit être corrigée dans le PRD.
2. **`pj` converti** :
   - struct, état partagé, accesseurs, constructeur, descripteur, lecteur, accesseur du descripteur, `set_struct_new` ;
   - `pico8_pj_new`/`ju` typés, `kj`/`ph`/`hk`/`hb` sur la struct et les accesseurs ;
   - `de.my`, le `lh` des scores, `ix[3].my` et `pico8_cr` sur `ju->c_` et `pico8_pj_cr()` ;
   - les 3 helpers de `->defaults` et les 4 installations retirés ;
   - `test_pico8_mh` adapté.

   Toutes les caractérisations passent **sans changer leurs valeurs figées**. Les vérifications de forme s'ajoutent à `test_pico8_pj`.
3. **Tests de régression** : `game_end` et `score_panel_close` réécrits (lecture de `c_`), puis leurs deux preuves rouges, correctif restauré.
4. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.29 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field`/`pico8_mv`, l'appartenance à `hv`/`cv`/`cg` et son ordre, les pixels, le compte de couverture, l'identité des REF, le contenu des tables partagées. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_pj`** : lié comme `test_pico8_fv`, sur le cart réel, avec une dépendance Makefile sur `tests/fixtures/game_end_events.json`.
  - Il lit **uniquement** par l'API à double forme, et par `pico8_class_get_field(pico8_pj_class(), …)` pour les **défauts de spec**, jamais pour une méthode (les installations disparaissent au morceau 2).
  - Les 4 dispatchers sont `static` : ils sont jugés par leur comportement, appelés par `pico8_mv`.
  - Son code est le même avant et après, aux vérifications de forme près.
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()`, pour l'état de `rnd`.
  - La partie est lancée comme le filet (`pico8__init()`, puis `pico8_eu` sur `bo[2]`).
  - Toute présence se lit par un `lookup` qui confond nil et absent.
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **Construction** :
       - `pico8_ju()` est dans `hv`, à un rang figé ; il est dans `cv.kj`/`cv.hk`/`cv.hb` et pas dans `cv.ev`/`cv.hd` ; il n'est dans aucun `cg` ;
       - `fa`=10, `ed`=1, `me`=`"ph"`, `t` ;
       - absents : `c_`, `gz`, `ra`, `p_`, `nu`, `fx`, `hd`, `ev` ;
       - `kj`/`ph`/`hb`/`hk` présents et de type fonction ;
       - **identité** : `ou`, `ea`, `cr`, `gs`, `i_` lus sur `ju` sont les REF que rend `pico8_class_get_field(pico8_pj_class(), …)` ;
       - empreinte du contenu des 5 tables (lignes, `fn`, positions).
    2. **Partage entre instances** :
       - un second `pico8_pj_new(NULL)` voit les **mêmes REF** `ou`/`ea`/`cr`/`gs`/`i_` que `ju` ;
       - après `hb` (par `pico8_mv(ju, "hb", …)`), `ou[i][1]` lu par le second est la chaîne écrite, et `ea[2][1]`, `ea[3]`, `ea.lh` après `hk` aussi ;
       - à l'inverse, `fa`, `ed`, `c_`, `gz` restent propres à chaque instance : le panneau de `ju` ouvert (`fa`=13, `c_` vrai), le second garde `fa`=10, `ed`=1, `c_`/`gz` absents.

       Figée.
    3. **Vie** :
       - `hb` : empreinte des 7 `ou[i][1]` ;
       - `hk` à 3425 (`gs[1]`), 3400 (pas de message), 3401 : présence de `ea[3]`, de `ea.lh`, valeur de `ea[2][1]`, nombre de `mh` dans `cg.mh`, `kc` ;
       - `pico8_cr()` : `cr.ep`, identité de `cr` ;
       - `kj` ×2 (panneau fermé), empreinte d'écran ;
       - `fm.hl` posé à `"final score"` sur `fm` par `pico8_entity_set_field`, puis `ph` : `c_` vrai, `gz`=0, `fa`=13, un `fq` et deux `js` de `fa`=15 de plus dans `hv`, `ed` = `kc` ;
       - `ph` encore : aucune nouvelle boîte ;
       - `kj` ×3 : `gz` = 0.2, 0.4, 0.6 et empreinte d'écran ;
       - `my` de `pico8_de()` appelé : rend vrai, `c_` faux ; appelé encore : `c_` vrai.

       Figée.
    4. **Rejeu court** : `game_end_events.json` (filet), départ comme le filet, jusqu'à la dernière frame. À chaque frame, on plie :
       - le rang de `ju` dans `hv` et `t` ;
       - la présence et la valeur de `c_`, `gz`, `fa` ;
       - `ed` = `kc` ou non.

       S'y ajoutent une empreinte d'écran finale et l'empreinte de `ou`. Figée.
  - **Forme** (ajoutée au morceau 2) :
    - `ju` est `STRUCT` au descripteur `pico8_pj_struct_class()` ;
    - `HB`/`KJ`/`HK`/`PH` sont non NULL, les autres NULL ; `field` est posé, `set_field` est NULL ;
    - `pico8_entity_set_field(ju, "c_", …)` avorte (dans un fils) ;
    - `pico8_pj_new` avec une clé (nommée ou positionnelle) avorte ;
    - `pico8_pj_class()->struct_new` est posé et son `pq` est NULL. `hb`/`kj`/`hk`/`ph` sont absents de `pico8_pj_class()->defaults` (lecture directe de la table), et les 7 défauts de spec y sont ;
    - `pico8_pj_ou()`, `ea()`, `cr()`, `i_()`, `gs()` sont les REF de la classe ; `ju->gs`/`ju->i_` aussi.
- **Tests de régression (§6)** :
  - `game_end` et `score_panel_close` : réécrits, lecture seulement ; preuves rouges ci-dessus ;
  - tous les autres gardent un diff vide et restent verts.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (copie de l'arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` `test_pico8_pj`, `test_pico8_mh`, `test_pico8_game_end`, `test_pico8_score_panel_close` et `test_pico8_tech_screen` (chemin de `cr`), avec les objets du moteur. Les lancer, puis `gcov` sur `pico8_score_panel.c`, `pico8_score_panel_display.c` et `pico8_menu.c`.
    - Même piège qu'à 1a.23-1a.28 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`.
    - Les branches d'assert atteintes seulement dans un fils qui avorte (`must_abort`) ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu.
    - Le rapport est collé dans le message de commit. Il donne les lignes exécutables ajoutées ou modifiées de `src/` et leur compte : état et reset, accesseurs, constructeur, lecteur (dont `ou`/`ea`/`cr`), `kj`/`ph`/`hk`/`hb`, `de.my`, `lh`, `ix[3].my`, `pico8_cr`. Les lignes à 0 sont données avec leur justification.
  - **2 à 3 mutations**, chacune rouge sur `test_pico8_pj` (ou un autre test nommé), notées dans le commit avec le test qui l'attrape :
    1. **valeur d'un défaut propre** : `fa` non copié, ou copié dans `ed`. Le rang dans `dc` change (couche 10), l'empreinte d'écran et `fa` aussi ;
    2. **entrée du descripteur remplacée** : `PH` = NULL (le panneau ne s'ouvre jamais, `game_end` rouge aussi) ou `HB` ↔ `HK` ;
    3. **état partagé** (à la place de l'ordre du constructeur, mutant équivalent ici) : `ou` (ou `ea`) re-parsé depuis la spec pour chaque instance au lieu de la REF de la classe. L'identité et le partage entre instances (scénario 2) passent au rouge.
- **Tests adaptés** : `test_pico8_mh` (`game_ju`, `set_c_`, pose de `gz`, lecture de `ed`). Aucune valeur figée ne bouge.
- **Tests existants à diff vide** : `test_pico8_fq`, `js`, `top_menubar`, `score_panel_stars`, `playability`, `fv`, `g_`, `bg`, `jc`, `mz`, `mo`, `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, `ga`, `ie`, `qb`, `nf`, `iv`, `je`, `bg_kj`, `jb_ol`, `class_chain`, `entity_smoke`, `class_coverage`, et les autres tests de régression.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED`.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** :
  - `test_pico8_fv`/`test_pico8_g_` : structure, `lookup`, `must_abort`, fils par scénario, rejeu court, forme ;
  - `test_pico8_ie` et `test_pico8_nf` : état statique partagé lu sur l'instance (`gy`/`id`, `l`/`r`) ;
  - `test_pico8_mh` : `hk` et `ph` sur le `ju` d'une partie ;
  - `score_panel_close` (1a.5) et `planet_kj_dispatch` (1a.8) : réécriture et preuve rouge, bug d'origine et bug de référence.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; c'est le seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` (en particulier `pj  19  15/15`). Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations (défaut propre, entrée du descripteur, état partagé) ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier :
  - si l'assertion de clé se déclenche sur un rejeu ou un test hors `must_abort` ;
  - si un membre autre que `fa`, `gs`, `i_`, `ed`, `c_`, `gz` est ajouté, ou si `ou`, `ea` ou `cr` reçoit un membre ;
  - si un écrivain est posé ;
  - si le constructeur accepte une clé ;
  - si l'enum change.
- `FAILED:` si une table de l'état partagé est re-parsée depuis la spec ou copiée par instance, si `ou`/`ea`/`cr`/`gs`/`i_` ne sont pas les REF de `pico8_pj_class()->defaults`, ou si deux `pj` ne voient pas la même table.
- `FAILED:` si un site hors de `pico8_class.c` lit encore `pico8_pj_class()->defaults` ou `cls->defaults` pour `pj` (grep), ou si une installation reste dans la classe `pj`. Également si la classe `pj` cesse d'être enregistrée avec ses défauts de spec.
- `FAILED:` si une autre classe que `pj` est convertie ou change de forme (`ui`, `hp`, `lq`, `hy`, `trade`, l'enveloppe de `pz`…), ou si le comportement d'une autre classe change (valeur figée de sa caractérisation, ligne de couverture).
- `FAILED:` si une entrée du descripteur autre que `HB`/`KJ`/`HK`/`PH` est non NULL, ou si l'une de ces 4 est NULL.
- `FAILED:` si `pico8_ke_ks`, `dg`, `pz`, `mh`, `hy:hk`, `hy:lj` ou l'enveloppe de `pico8_cr` changent au-delà de la source de la table `cr`.
- `FAILED:` si `game_end` ou `score_panel_close` changent au-delà de la lecture de `c_` (frames, fixture, asserts, includes exceptés), ou si le commit ne rapporte pas leurs deux preuves rouges (bug remis, test rouge, correctif restauré, vert).
- `FAILED:` si un autre test de régression (§6) a un diff non vide.
- `FAILED:` si une valeur figée de `test_pico8_pj` a été changée après la conversion, ou une de `test_pico8_mh`, `fq`, `js`, `fv`, `g_` ou d'une autre caractérisation. Également si `test_pico8_mh` change au-delà de `game_ju`, `set_c_`, de la pose de `gz` et de la lecture de `ed`.
- `FAILED:` si une ligne est ajoutée à `tools/float_audit_c1.txt` ou en est retirée, ou si `make float-audit` est rouge.
- `FAILED:` si `test_pico8_pj` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.
- `FAILED:` si la note « Découvert à 1a.29 » manque, ou si elle ne dit pas d'où `cr` arrive à l'enveloppe de 1a.30.

Critères de réussite (en plus) :

- la struct `pj` n'a que `pico8_ke_t` et les 6 membres ; son descripteur a 4 entrées et pas d'écrivain ;
- grep :
  - `PICO8_CLASS_COVERAGE_NAME(g_pj_class, "pj")` reste ;
  - `pico8_class_set_struct_new(g_pj_class, …)` est présent ;
  - aucun `set_name(g_pj_class->defaults, …)` ;
  - aucun `pj_class_field_ref_table` ni `pj_cr_table` ;
  - aucun `->defaults` dans `pico8_score_panel_display.c` ni dans `pico8_menu.c` ;
- le `ju` d'une partie est une struct au descripteur de `pj`, dans `cv.kj`/`cv.hk`/`cv.hb` ;
- la ligne 1a.29 de `docs/plan-3.md` porte la note « Découvert à 1a.29 », et le message de commit dit « découvert à 1a.29 ».

## Out of Scope

- L'enveloppe de `pz()` sur `pj.cr` (1a.30) : elle reste une instance sans classe sur la pile, et ne change que de source de table.
- `ui` (1a.31), et avec lui la fermeture de `ke_ks`/`dg`.
- Les autres classes et instances sans classe : `trade`, `hp`, `lq`, `hy`, `fm`, les `{}`/`et={}` jetables, l'enveloppe de `g_ha` (leurs étapes).
- Supprimer la classe `pj` ou les registres : 1a.42.
- Typer les tables `ou`/`ea`/`cr` (1b, phase D) ou en faire des données `static const` : `ou`/`ea`/`cr` sont modifiées, elles ne le peuvent pas telles quelles.
- Libérer le panneau, la boîte `mh`, les chaînes écrites dans `ou`/`ea`/`cr` (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Premier défaut « modifié en place » sans écriture sur l'instance** : `ie.gy` était réaffecté, `ga.by` et `jo.kw` sont aussi réécrits sur l'instance. `pj` est le cas pur de la règle : trois tables que seul le code du module modifie, par la classe. D'où l'état statique sans membre, et des accesseurs au lieu de trois lectures de `->defaults`.
- **Ce que le filet voit du partage, peu de chose** : dans `long_session`, 5 parties se suivent, et chaque nouveau `pj` hérite des `ou`/`ea`/`cr` de la précédente. Mais `eu` appelle `hy:lj`, donc `pj:hb`, qui réécrit les 7 `ou[i][1]` avant le premier `kj` (mesuré : 22 `hb` depuis `eu`). Une copie par instance de `ou` échapperait donc probablement au filet. Pour `ea`, seul un `ea.lh` périmé qui passe d'une partie à l'autre la trahirait. C'est pourquoi le scénario 2 prouve le partage directement, et pourquoi la mutation 3 le vise.
- **Mesure utile pour 1a.30** : `pico8_cr` (`menu.c:150`) est appelé 45 fois sur les rejeux (écran technologie). Son enveloppe (`menu.c:243-247`) porte la table `cr` partagée, que `pz` lit comme `p` (`hi`, `gq`, `ca`, `ep`).
- **Mesure utile pour 1a.31** : `dg` reçoit la struct `pj` par `hv` (168 997 fois), et `ke_ks` ne reçoit jamais `pj`.
- **Leçons de 1a.23-1a.28 appliquées** :
  - pas de mutation sur un mutant équivalent (l'ordre `lt`/défauts) ;
  - piège `_exit` des fils réglé dans la copie jetable seulement ;
  - pas de membre pour un nom seulement lu et toujours absent (`ra`, `p_`, `nu`, `fx`) ;
  - écriture directe dans le membre depuis un test plutôt qu'un écrivain élargi.
- Mesures :
  - grep sur `src/`, `tests/`, `tools/` et le `Makefile` à a4364b3 ;
  - lecture de `pico8/lua.lua:153-199,815-903,955-985,1182,1210-1238` ;
  - lecture de `pico8_score_panel.c/h`, `pico8_score_panel_display.c/h`, `pico8_menu.c`, `pico8_ke.c/h`, `pico8_struct.h`, `pico8_class.c`, `pico8_entity.c`, `pico8_bootstrap.c` (`dg`), `pico8_cart.c`, `pico8_research_icon.c` (`ie.gy`), `pico8_mouse_reader.c` (`nf.l`), `pico8_bg.c` (`bg.pl`), `pico8_probe.c` (tableau d'état de `g_`), `pico8_planet_family.c` (`ga.by`), des tests nommés, de `tools/class_coverage_net_expected.txt` et `tools/float_audit_c1.txt` ;
  - lecture du PRD de 1a.28 et des messages de e13f975 et ed688a8 ;
  - sonde dans une copie jetable du scratchpad, supprimée après usage : 18 rejeux en sortie 0 ; 22 panneaux (19 sur le filet), tous par `pico8_pj_new(NULL)` ; aucun `pico8_entity_set_field` sur un `pj` ; `ou`/`ea`/`cr`/`gs`/`i_` jamais lus sur une instance ; `self` de `kj`/`ph` toujours un `pj` ;
  - fil piège (`make unit`, même copie) : aucune instanciation avec clé, aucun `self` autre qu'un `pj`, aucune écriture générique ; `cr` atteint aussi par `test_pico8_ga`, `geoharvest_burst`, `jc`, `js`, `nl`, `qb`, `tech_screen`.
