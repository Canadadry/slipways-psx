---
title: "Plan 3, étape 1a.35 : `hy` (l'économie) devient une struct racine ; 11 membres, un lecteur, ni méthode ni écrivain, le `fm` provisoire laissé tel quel"
description: "`hy` est construite 19 fois sur le filet (15/15 rejeux), une seule fois par partie, par `pico8_hy_new(NULL)` depuis `eu()`, et rangée dans le slot `fm`, qui tient avant elle le `fm` provisoire sans classe (1a.36). Elle passe en struct `pico8_hy_t` : en-tête, puis `nz`, `ge`, `jk`, `ew`, `invented`, `lo`, `yr` (posés par `pq`) et `po`, `fu`, `happiness`, `hl` (écrits par `lj`). Son descripteur n'a pas de méthode, un lecteur, pas d'écrivain. Ses cinq fonctions (`nx`, `hk`, `lu`, `lj`, `kt`) passent à la forme struct seule par un accesseur typé du slot. Les lecteurs de `fm` hors du module passent au lecteur à double forme. Le slot garde son type, et le code de triche garde sa double forme jusqu'à 1a.36. Les huit tests de régression de la ligne changent leur façon de lire ou d'écrire `fm`, et chacun est prouvé rouge."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.35 — `hy` : racine (`economy.c:60`). Tests : `cheat_code`, `empty_planet_cleared`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`.

Le numéro de ligne est juste. À HEAD 96e74a9 :
- **`pico8_economy.c`** :
  - `g_hy_class` `:16` ; `pico8_class_declare(NULL, NULL)` `:60` ; nom de couverture `:61` ; `pico8_class_set_pq(g_hy_class, pico8_hy_pq)` `:62` ;
  - `field_fixed` `:18-25`, qui lit par `pico8_instance_get_field` ;
  - `hy:pq` (`pico8_hy_pq`, `:30-55`) : parse la spec `nz=100,ge=0,jk=0,ew=1,invented=o(),lo=1,yr=3401`, la copie dans la table de l'instance (`pico8_qj`), puis remplace la table vide `invented` par une liste neuve (TROU-7) ;
  - `pico8_hy_new` `:76` (`pico8_class_instantiate`) ;
  - `nx` `:80`, `hk` `:104`, `lu` `:163`, `lj` `:195`, `kt` `:285` : lisent par `field_fixed` / `pico8_instance_get_field` et écrivent par `pico8_ob_table_set_name(pico8_instance_fields(self), …)`.
- **`pico8_economy.h`** : `pico8_hy_new` et les cinq fonctions typés `pico8_instance_t *` ; `pico8_hy_class()`.
- **`pico8_probe.c`** : le slot `g_fm` `:23` ; le `fm` provisoire construit à la main dans `pico8_probe_init` `:527-530` (tag `INSTANCE`, table vide, `cls` NULL) ; `pico8_probe_shutdown` le libère `:539-542` ; `pico8_fm()` `:557` ; `pico8_probe_set_fm` `:561-567` libère l'occupant précédent par `pico8_instance_free`.
- **`pico8_cart.c`** : `pico8_probe_set_fm(pico8_hy_new(NULL))` `:204`, puis `pico8_hy_lj(pico8_fm())` `:250-251`.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` […] ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Défauts mutés sur la classe** : […] tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** […] devient un état statique partagé de la classe.
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) […].
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […] jusqu'à la dernière étape de 1a. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct […].
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe.
>
> **Ordre** : […] Les racines sans parent dans n'importe quel ordre.

`hy` est une **vraie racine sans fille** (écarts relevés du §2). Elle ne descend pas de `ke` : pas de `lt`, jamais dans `hv`/`cv`/`cg`, jamais atteinte par `mv`, `dc` ou `jz`. Ses précédents :
- `trade` (1a.32), `hp` (1a.33) et `lq` (1a.34) : racine à constructeur posé par `pico8_class_set_struct_new`, lecteur sans écrivain, fonctions et consommateurs retypés ;
- `nf`/`je` (1a.6/1a.7) et `qb`/`mo` (1a.8/1a.9) : une classe convertie dont les instances vont dans un slot qui tient d'abord une instance provisoire sans classe, convertie à l'étape suivante.

§6 : chaque test de régression qui lit ou écrit `fm` est réécrit **dans sa seule façon de lire ou d'écrire l'état**, dans ce commit. Chacun est prouvé rouge avec son bug d'origine remis localement, puis restauré.

§7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` : `hy  19  15/15`, toutes les autres lignes inchangées ;
- preuve des tests par `gcov` et 2 à 3 mutations ;
- note « Découvert à 1a.35 », réduite à ce qui change une étape suivante ;
- un nouveau test dont le rejeu dépasse 30 s va dans `UNIT_TESTS_SLOW` ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.34 ont posé, réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (`methods` indexé par l'enum, `field`, `set_field`) :
  - `pico8_entity_get_field` cherche d'abord le nom dans l'enum, et rend l'entrée si elle est posée ; sinon il passe au lecteur. Un membre nil est absent ;
  - sur tout ce qui n'est pas une struct, il délègue à `pico8_instance_get_field`, avec un résultat identique ;
  - `set_field` NULL : une écriture générique avorte.
- Racine convertie :
  - l'en-tête est le premier membre ;
  - le constructeur est `static`, posé par `pico8_class_set_struct_new` et appelé par `pico8_<classe>_new` ;
  - la couverture est enregistrée par le constructeur ;
  - un assert porteur rejette toute clé non mesurée ;
  - un assert de descripteur protège chaque fonction propre sur un pointeur non NULL (`trade`, `hp`, `lq`).
- Slot qui tient une provisoire puis une convertie (1a.6, `je`) :
  - le type du slot et de son accesseur ne change pas ;
  - le contrat d'en-tête dit qu'il peut tenir une struct ;
  - les lecteurs passent par `pico8_entity_get_field` ;
  - la libération se fait selon le tag.
- Un test qui écrivait par `->fields` écrit le membre de la struct, tag vérifié (1a.6, `test_pico8_iv`).
- **L'enum ne change pas** : `hy` n'est jamais appelée par nom.

### Ce que fait le cart

`pico8/lua.lua` :

```
:1182-1186  eu(nj) : … fm,ju=hy(),pj() … fm:lj()
:1198-1203  hy=kv:jr() ; hy:pq() : qj(self,ob("nz=100,ge=0,jk=0,ew=1,invented=o(),lo=1,yr=3401,"))
:1204-1209  hy:nx(hx) : kx=ov("time compression",12,15) ; self.lo+=hx ; self:lj() ; if self.lo>kx then self.lo-=kx self:hk() end
:1210-1216  hy:hk()   : self.nz+=self.ge ; self.jk+=self.po[r.k]+1 ; self.yr+=1 ; self:lj() ; for e in all(cv.hk) do e:hk(self.yr) end
:1217-1225  hy:lu(jt) : if self.nz<jt then iv({"-no money-",8}) return end ; self.nz-=jt ; if jt~=0 then iv({ne(-jt,10,"$")}) end ; return true
:1226-1235  hy:lj()   : repeat bd() until not bb() ; self.po,self.fu=bx(),ck() ; p.kr=… ; self.ge=… ; self.happiness=bf()
                        self.hl= self.yr>=3426 and "final score" or self.ge<=0 and self.nz<3 and "bankrupt" ; for e in all(cv.hb) do e:hb() end
:1446-1453  hy:kt(t)  : self.jk-=gi(t) ; add(self.invented,t.px) ; if self.ew<4 and #self.invented>=ek[self.ew] then self.ew+=1 end ; self:lj()
:979-980    ix[1].my  : fm.lo=1 ; fm:hk()
lecteurs    pj:hb (:847), pj:ph (:870,885), cr (:953-961), ky (:1347-1354), bf (:1367-1368), cb/oj/gi (:1457-1473)
appelants   fm:lu (:419 jo:gh, :570 d_), fm:nx (:460 jo:hu, :571 d_, :640 jc:ey), fm:kt (:957)
```

Points décisifs :
- **`hy` n'a pas de défaut de classe** : `kv:jr()` sans spec. `pq` écrit les 7 clés de sa spec **dans la table propre de chaque instance**. `invented=o()` est donc une table **par instance**, modifiée en place par `kt` (`add`) : ce n'est pas un défaut de classe modifié en place. La règle « défauts mutés » du §5 **ne s'applique pas**, et `invented` est un membre. Même chose pour `po`/`fu`, remplacés à chaque `lj`, et pour `happiness`/`hl`, écrits par `lj`.
- **`fm` vaut nil dans le cart avant `eu()`**. Le `fm` provisoire du port (une instance vide) n'existe que pour que les lecteurs gardés trouvent un objet. C'est l'étape 1a.36.
- `hl` vaut `false`, pas nil, quand aucune fin n'est atteinte : `A and B or C and D` rend `false`. Le port écrit `BOOL(0)`. Avant le premier `lj`, `hl` est absent.

### Emprise réelle de `hy` et de `fm` (mesurée)

**Méthode.**
- Grep de `src/`, `tests/`, `tools/` et du `Makefile` à 96e74a9 ; lecture du cart aux lignes ci-dessus.
- **Sonde** sur une copie `git archive HEAD` jetable du scratchpad, supprimée après usage :
  - les définitions de `pico8_instance_get_field`, `pico8_instance_fields`, `pico8_instance_free`, `pico8_entity_get_field`, `pico8_entity_set_field`, `pico8_ob_table_get_name`, `pico8_ob_table_set_name`, `pico8_hy_new` et des cinq fonctions de `hy` sont renommées ;
  - un en-tête forcé (`-include`) les remplace par des enveloppes qui portent `__FILE__`/`__LINE__` ;
  - les enveloppes ne journalisent que ce qui touche un objet suivi : le `fm` provisoire (suivi dès `pico8_probe_init`), chaque `hy` (dès `pico8_hy_new`), ou leur table de champs. Le suivi cesse à la libération ;
  - chaque événement est compté par site, nom, genre de la valeur et forme de l'objet (provisoire ou `hy`). S'y ajoutent la forme de `self` à chaque appel d'une fonction de `hy`, et les clés de l'instance après `pq` et à sa libération ;
  - la sortie est vidée à `exit` et `_exit` (fils forkés compris).
- **Rejoués** :
  - les 18 rejeux de `tests/fixtures/*events*.json` hors malformés, `--from-title` pour les 4 enregistrements, plus `events_valid` et `events_valid_ox` : **20 rejeux en sortie 0** ;
  - puis `make -k unit` et les 8 cibles de `UNIT_TESTS_SLOW`, même copie : **tout vert, tout lié**.

**Constructions** :
- toutes par `pico8_hy_new(NULL)` depuis `eu()` (`cart.c:204`), **sans aucune clé**. Jamais par `pico8_class_instantiate` direct, `lookup_named`, `fz` ou `gx[]` : `hy` n'est pas enregistrée par nom ;
- 24 sur les 20 rejeux ; la référence du filet est `hy  19  15/15` ;
- 230 dans les TU ;
- aucun test n'appelle `pico8_hy_new`.

**Clés de l'instance** :
- après `pq` : `ew`, `ge`, `invented` (REF liste), `jk`, `lo`, `nz`, `yr` ;
- à la libération : les mêmes, plus `po`, `fu` (REF table), `happiness` (nombre), `hl` (`BOOL(0)` ou chaîne) ;
- **11 noms, rien d'autre** ; aucune clé d'index.

**Le `fm` provisoire** (sur les 20 rejeux et dans tous les TU) :
- construit une fois par `pico8_probe_init` ;
- libéré par `pico8_probe_set_fm` quand `eu()` pose le premier `hy` (199 fois en TU, 20 sur les rejeux), ou par `pico8_probe_shutdown` (`test_pico8_je`, `test_pico8_nf`) ;
- **une seule lecture, jamais une écriture ni un appel** : `test_pico8_jc.c:111`, par `pico8_entity_get_field(fm,"nz")` (114 fois, repli avant `eu()` de `fold_field`, `:331`), qui rend « absent » ;
- **aucune fonction de `hy` ne reçoit jamais le provisoire, ni NULL** (rejeux et TU).

**Appels des fonctions de `hy`** (tous avec `self` = le `hy` de `fm`) :

| Site | Fonction | 20 rejeux | TU |
|---|---|---|---|
| `cart.c:251` (`eu`) | `lj` | 24 | 230 |
| `probe.c:279` (`d_`) | `lu` (jt > 0 : 253 ; jt = 0 : 13) | 266 | 1 374 |
| `probe.c:301` (`d_`) | `nx` | 261 | 1 348 |
| `planet.c:543` (`jo:hu`) | `nx` | 64 | 367 |
| `planet_display.c:392` (`jo:gh`, `my`) | `lu` (dont jt < 0 : 3 ; refus : 2) | 66 | 367 |
| `slipway.c:183` (`jc:ey`) | `nx` | 98 | 471 |
| `menu.c:269` (`ix[1].my`) | `hk` | 85 | 529 |
| `menu.c:64` (ligne de `cr`, `my`) | `kt` | 27 | 142 |
| `test_pico8_hp.c:397` | `kt` | — | 4 |

**Lectures de `fm` hors de `pico8_economy.c`** :

| Site | Nom | Par | 20 rejeux | Après conversion |
|---|---|---|---|---|
| `tech_gate.c:48` (`oj`) | `invented` | `instance_get_field` | 30 477 | lecteur à double forme |
| `tech_gate.c:81` (`gi`) | `fu` | `instance_get_field` | 12 236 | idem |
| `tech_gate.c:125` / `:132` (`cb`) | `jk` / `ew` | `instance_get_field` | 12 027 chacun | idem |
| `menu.c:149` / `:152` (`cr`) | `ew` / `invented` | `instance_get_field` | 45 chacun | idem |
| `score_panel.c:54,62,70` (`pj:hb`, 3 aides) | `ge`, `nz`, `happiness`, `lo`, `jk`, `yr`, `po` | `instance_get_field` | 588 chacun | idem |
| `score_panel_display.c:33` (`pj:ph`, `st[1]`) | `hl` | `instance_get_field` | 169 006 | idem |
| `economy_score.c:31,47,55` (`ky`, `bf`) | `happiness`, `po`, `invented` | `entity_get_field` (1a.16b) | 600 | inchangé |

**Écritures de `fm` hors de `pico8_economy.c`** :
- `cheat.c:25-29` (`apply`) : `jk`=99, `nz`=200, par `pico8_instance_fields` ; 11 sur les rejeux, 57 en TU, toujours sur un `hy`. Mais `pico8_cheat_step` tourne à chaque frame dès l'écran titre dans `main.c`, `main_raylib.c` et `main_class_coverage.c` (`--from-title`) : **le provisoire est atteignable**, non mesuré ;
- `menu.c:267` (`ix[1].my`) : `lo`=1, par `pico8_instance_fields` ; 85 / 529. `ix` n'existe qu'après `eu()` : toujours sur un `hy`.

**Écritures internes** (par `pico8_instance_fields`) :
- `nx` : `lo` ×2 ;
- `hk` : `nz`, `jk`, `yr` ;
- `lu` : `nz` ;
- `lj` : `po`, `fu` (anciens libérés), `ge`, `happiness`, `hl` ;
- `kt` : `jk`, `ew`, et `invented` modifié en place.

Aucun écrivain générique (`pico8_entity_set_field` dans `src/`) n'atteint `fm`.

**Accès `->defaults`** sur `hy` : aucun (`pq` par `pico8_class_set_pq` seulement).

**Tests qui lisent ou écrivent `fm`** (grep, puis sonde) :
- **régression (§6), les huit de la ligne**, tous sur un `hy`, jamais sur le provisoire :
  - `cheat_code:38` : `instance_get_field` (`jk`, `nz`) ; `:84-85` : écriture par `pico8_fm()->fields` ;
  - `empty_planet_cleared:72`, `playability:87`, `slipway_drag:31` : `instance_get_field` (`nz`) ;
  - `game_end:55` (`yr`) et `:63` (`hl`) : `instance_get_field` ;
  - `geoharvest_burst:67` (`nz`) et `:74` (`invented`) : `instance_get_field` ;
  - `score_panel_stars:114` (`invented`) et `:116` (`happiness`) : écriture par `pico8_fm()->fields` ;
  - `tech_screen:54` (`ew`, `jk`, `yr`) et `:61` (`invented`) : `instance_get_field`.
- **caractérisations** :
  - `test_pico8_ie:648`, `test_pico8_js:514` : `instance_get_field` (`invented`) ;
  - `pico8_entity_set_field(pico8_fm(),"nz",…)` : `test_pico8_fr:361`, `hj:438`, `mx:445`, `mz:521`, `nl:393`, `nr:413`, `os:392`, `qh:390`. Il y a aussi `"hl"` dans `test_pico8_pj:415,468` ;
  - `test_pico8_hp:397` : `pico8_hy_kt(pico8_fm(), t)` ;
  - déjà par `pico8_entity_get_field`, **diff vide** : `test_pico8_jc:111,122,296,331`, `test_pico8_g_:108,579,598`, `test_pico8_mz:107,188`, `test_pico8_js:132,882,885`, `test_pico8_hj:433`, `nl:388`, `os:387`, `qh:385`.
- Aucun test ne lit `pico8_hy_class()`, le tag ou `->cls` de `fm`.

**Outils** :
- `tools/float_audit_c1.txt:14-15` nomment `pico8_hy_hk | k_val.as.fixed` et `pico8_hy_kt | ew` : ces deux extraits doivent rester **à l'identique** ;
- `main_class_coverage.c:23` : `"hy"` dans `KNOWN[]`, inchangé.

**→ Membres de `hy`**, dans cet ordre :
- `nz`, `ge`, `jk`, `ew`, `invented`, `lo`, `yr` : l'ordre de la spec de `pq` ;
- `po`, `fu`, `happiness`, `hl` : l'ordre d'écriture de `lj`.

**→ Descripteur** : aucune méthode. **Lecteur** : les 11 membres (nil = absent). **Écrivain** : aucun.

## Solution

1. **Enum : inchangée.**
2. **Struct `hy`**, publique dans `pico8_economy.h` : `pico8_struct_header_t`, puis les 11 membres `pico8_value_t` ci-dessus.
3. **Constructeur** (`static`, posé par `pico8_class_set_struct_new` dans `pico8_economy_init`, appelé directement par `pico8_hy_new`), dans l'ordre de `kv:jr` puis `hy:pq` :
   1. alloue un `pico8_hy_t` ; pose l'en-tête (tag STRUCT, descripteur de `hy`) ; met les 11 membres à nil ;
   2. **asserte** (message porteur, arrêt §7) que les surcharges sont NULL ou vides ;
   3. `pq` : `nz`=100, `ge`=0, `jk`=0, `ew`=1, `invented` = REF vers une liste neuve vide (TROU-7), `lo`=1, `yr`=3401 ;
   4. enregistre `"hy"` au crochet de couverture.
4. **Descripteur de `hy`** (`static const`) : toutes les méthodes NULL ; lecteur des 11 membres ; **écrivain NULL**. Accesseur public `pico8_hy_struct_class()`.
5. **Fonctions de `hy` sur la struct** : `pico8_hy_new` rend `pico8_hy_t *`. `nx`, `hk`, `lu`, `lj`, `kt` prennent `pico8_hy_t *`, lisent et écrivent les membres, avec les mêmes gardes et la même vérité :
   - fixe attendu, sinon 0 ;
   - NULL = sans effet, et `lu(NULL, jt>0)` échoue.

   Autres règles :
   - chacune asserte (message porteur) qu'un `self` non NULL est une struct au descripteur de `hy` ;
   - `lj` libère encore les anciens `po`/`fu` qu'elle a posés ;
   - `hk` garde sa lecture `po[r.k]` à deux branches ;
   - `field_fixed` disparaît s'il n'a plus d'appelant ;
   - les extraits `k_val.as.fixed` (`hk`) et `ew` (`kt`, variable locale gardée) restent tels quels pour `tools/float_audit_c1.txt`.
6. **Le slot `fm`** (`pico8_probe`) :
   - `pico8_fm()` garde son type `pico8_instance_t *`, comme `pico8_je()` à 1a.6. Son contrat dit : le provisoire (instance) avant `eu()`, puis un `hy` (struct), à lire par `pico8_entity_get_field` ;
   - **nouvel accesseur typé `pico8_fm_hy()`** :
     - NULL si le slot est vide ;
     - le `hy` si le slot tient la struct au descripteur de `hy` ;
     - **assert porteur** sur le provisoire (« a hy function on the provisional fm was never measured; 1a.36 converts it ») ;
   - `pico8_probe_set_fm` libère l'occupant précédent **selon son tag** : le provisoire par `pico8_instance_free`, comme aujourd'hui ; un `hy` par `free` de la struct. Aujourd'hui `pico8_instance_free` libère l'instance et sa table sans suivre les REF imbriquées, donc `invented`/`po`/`fu` restent perdus comme aujourd'hui (TROU-7). `pico8_probe_shutdown` fait de même.
7. **Appelants des fonctions** : `pico8_hy_*(pico8_fm())` → `pico8_hy_*(pico8_fm_hy())`. C'est le cas de `probe.c:272-301` (`d_`, la variable locale `fm` typée `pico8_hy_t *`), `planet.c:543`, `planet_display.c:392`, `slipway.c:183`, `menu.c:64`, `menu.c:261-269` et `cart.c:250-251`. L'assert de `cart.c:250` porte sur `pico8_fm_hy()`. `cart.c:204` passe le `hy` au slot par un cast vers le type du slot.
8. **Lecteurs hors du module** : `pico8_instance_get_field` → `pico8_entity_get_field`, même vérité, même résultat sur le provisoire :
   - `tech_gate.c:48,81,125,132` ;
   - `menu.c:149,152` ;
   - `score_panel.c` : les trois aides `fm_field_*` ;
   - `score_panel_display.c` : `fm_field_value`.

   `economy_score.c` est inchangé.
9. **Écrivains hors du module** :
   - `menu.c:267` (`ix[1].my`) écrit `pico8_fm_hy()->lo`, garde NULL conservée ;
   - `cheat.c` (`apply`) garde **deux formes**, par tag :
     - sur un `hy`, il écrit les membres `jk`/`nz` ;
     - sur le provisoire (écran titre), il écrit sa table comme aujourd'hui. 1a.36 fermera cette branche.
10. **Classe `hy`** : gardée à la même place, constructeur de struct posé, **`pq` retiré** de la classe. `pico8_hy_class()` reste.
11. **Tests** :
    - nouveau `test_pico8_hy` ;
    - les huit tests de régression réécrits dans leur lecture ou écriture, et prouvés rouges (§6) ;
    - les caractérisations qui écrivent ou lisent `fm` par la couche adaptées dans leur seul accès, **valeurs figées inchangées**.

    Voir Testing Decisions.
12. **Plan** : à la ligne 1a.35 de `docs/plan-3.md`, ajouter « Découvert à 1a.35 : … », réduit à ce qui change une étape suivante (§7). A priori :
    - 11 membres, pas de méthode, pas d'écrivain, enum inchangée ;
    - `pico8_fm_hy()` ; `pico8_fm()` garde son type ;
    - restent à double forme jusqu'à 1a.36 : `cheat.c` `apply`, la libération par tag dans `pico8_probe_set_fm`/`shutdown`, et les lecteurs de `fm` par `pico8_entity_get_field`.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `hy` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser le `fm` provisoire à 1a.36.
2. En tant qu'autopilote, je veux l'emprise de `hy` et de `fm` mesurée (constructions, clés, appels par site et forme de `self`, lectures, écritures, tests), afin que chaque membre, l'absence de méthode et d'écrivain, et chaque test touché soient justifiés par une mesure.
3. En tant que lecteur du code, je veux une struct `hy` qui n'a que l'en-tête et les 11 membres, noms minifiés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que mainteneur, je veux que `invented`, `po` et `fu` soient des membres, pas un état statique, parce que ce sont des tables propres à chaque instance et non des défauts de classe. Ainsi la règle « défauts mutés » n'est pas appliquée hors de son cas.
5. En tant que joueur, je veux commencer chaque partie avec 100 crédits, 0 recherche, l'ère 1 et l'année 3401, afin que la partie ne change pas.
6. En tant que joueur, je veux que chaque sonde, colonie ou slipway coûte et fasse avancer le temps comme avant (`lu`, `nx`), afin que l'économie reste identique.
7. En tant que joueur, je veux que passer l'année (`ix[1]`) ajoute les revenus, la recherche et fasse avancer l'année, et que les planètes voient leur `hk`, afin que le tour reste identique.
8. En tant que joueur, je veux qu'une récolte (`lu` négatif) me paie, afin que `geoharvest_burst` reste vrai.
9. En tant que joueur, je veux qu'inventer une technologie dépense la recherche, l'ajoute à `invented` et monte l'ère aux seuils de `ek`, afin que l'écran des technologies reste identique.
10. En tant que joueur, je veux que la partie finisse sur « final score » en 3426, ou sur « bankrupt », afin que le panneau de fin s'ouvre au même moment.
11. En tant que joueur, je veux que le panneau du haut (`pj:hb`) affiche les mêmes crédits, revenus, recherche, bonheur et année, afin que l'écran ne bouge pas.
12. En tant que joueur, je veux que le code de triche mette toujours 99 recherche et 200 crédits, en partie comme sur l'écran titre, afin que `cheat_code` reste vrai.
13. En tant que mainteneur, je veux que le constructeur avorte sur toute clé reçue, afin qu'une surcharge jamais mesurée soit un arrêt (§7).
14. En tant que mainteneur, je veux qu'aucun écrivain ne soit posé, afin qu'une écriture générique jamais mesurée sur `fm` soit un arrêt.
15. En tant que mainteneur, je veux que les cinq fonctions de `hy` parlent `pico8_hy_t *` et assertent le descripteur, afin que le type dise ce que la mesure a prouvé.
16. En tant que mainteneur, je veux un accesseur `pico8_fm_hy()` qui avorte sur le provisoire, afin qu'un appel jamais mesuré sur le provisoire soit détecté au lieu d'être lu de travers.
17. En tant que mainteneur, je veux que tout lecteur de `fm` passe par le lecteur à double forme, afin qu'il rende la même chose sur le provisoire et sur `hy`, aujourd'hui et après 1a.36.
18. En tant qu'auteur de 1a.36, je veux que le provisoire reste une instance sans classe construite par `pico8_probe_init`, et que `pico8_fm()` garde son type, afin que sa conversion reste une étape à part.
19. En tant que gardien de la mémoire, je veux que `pico8_probe_set_fm` et `pico8_probe_shutdown` ne passent jamais un `hy` struct à `pico8_instance_free`, afin qu'aucune nouvelle partie ne libère un pointeur qui n'est pas une instance.
20. En tant que gardien de la couverture, je veux que le constructeur enregistre `"hy"` une fois par partie, afin que `class-coverage-net` reste `hy  19  15/15`.
21. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne des références de couverture ni de `float_audit_c1.txt` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
22. En tant que vérificateur, je veux une caractérisation `test_pico8_hy` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable. Cela couvre aussi ce que le filet voit peu ou pas : « bankrupt », `nx` qui boucle sur 15 avec « time compression », `kt` sans `px`, `ew` à 4, la triche sur le provisoire.
23. En tant que vérificateur, je veux un rapport `gcov` qui montre chaque ligne ajoutée ou modifiée exécutée, et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
24. En tant que gardien des tests de régression, je veux que les huit tests de la ligne ne changent que leur façon de lire ou d'écrire `fm`, chacun prouvé rouge avec son bug d'origine ou de référence, afin que leur objet reste intact (§6).
25. En tant que gardien des caractérisations, je veux que `test_pico8_ie`, `js`, `hp`, `pj`, `qh`, `nl`, `os`, `hj`, `mx`, `nr`, `fr` et `mz` gardent leurs valeurs figées, seul leur accès à `fm` adapté, afin que la conversion ne se valide pas en réécrivant ses témoins.
26. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
27. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C, un piège ou un `TROU-N` nu (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
28. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Pas d'outil de création d'agent dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** :
  - l'économie (`economy`) : struct, constructeur, descripteur, lecteur, cinq fonctions sur la struct ;
  - le slot `fm` (`probe`) : accesseur typé, libération par tag, contrat ; `d_` sur l'accesseur ;
  - appelants : `cart`, `planet`, `planet_display`, `slipway`, `menu` ;
  - lecteurs : `tech_gate`, `menu` (`cr`), `score_panel`, `score_panel_display` ;
  - écrivains : `menu` (`ix[1]`), `cheat`.

  Rien d'autre dans `src/`. Pas de changement dans `entity`, `class`, `pico8_struct.h`, `economy_score`.
- **11 membres, aucun état statique** (G) :
  - la classe n'a pas de défaut ; les 7 clés de `pq` sont écrites sur l'instance, les 4 de `lj` aussi (§5 : champ écrit sur instance → membre) ;
  - `invented` est modifié en place, mais c'est une liste créée pour chaque instance par son `pq`. Deux parties successives ont deux listes distinctes, dans le cart comme dans le port. Un état statique changerait ce que voit la deuxième partie ;
  - rejeté : `invented`/`po`/`fu` en état de classe.
- **Membres initialisés directement par le constructeur** (G), pas par le parse de la spec. Les 7 valeurs sont des entiers exacts, et la caractérisation les fige. `invented` est une liste neuve dès le départ : la table vide parsée puis libérée d'aujourd'hui disparaît, sans effet observable.
- **Pas de méthode dans le descripteur** : aucun site n'appelle une fonction de `hy` par son nom (`mv`, `dc`, `jz`). `hy` n'est dans aucune liste de `cv`. `HK` existe dans l'enum mais reste NULL pour `hy` : `pico8_entity_get_field(fm,"hk")` rend « absent », comme aujourd'hui.
- **Lecteur, pas d'écrivain** (G) :
  - le lecteur sert les 10 lecteurs de `src/` et des tests, qui passent tous par `pico8_entity_get_field` ;
  - aucun écrivain générique n'atteint `fm` (mesuré). `cheat.c` et `menu.c` savent qu'ils écrivent `fm` : ils écrivent le membre par l'accesseur typé ;
  - les caractérisations qui écrivaient par `pico8_entity_set_field(pico8_fm(),…)` écrivent le membre (précédent `test_pico8_iv`, 1a.6) ;
  - rejeté : un écrivain posé pour les seuls tests. Il ouvrirait à toute écriture générique future une porte que le §5 réserve aux écrivains génériques mesurés.
- **Coexistence avec le `fm` provisoire** (G, précédent `je` 1a.6, `mo` 1a.8) :
  - le slot tient une instance, puis une struct ;
  - `pico8_fm()` garde son type ;
  - les lecteurs passent à la double forme ;
  - la libération se fait par tag ;
  - le provisoire n'est **pas touché** : même construction, même libération, même table vide.

  La seule double forme ajoutée côté écriture est celle de `cheat.c`, atteignable sur l'écran titre. Les fonctions de `hy` et `ix[1].my` ne reçoivent jamais le provisoire (mesuré, et `ix` n'existe qu'après `eu()`) : elles passent struct seule par `pico8_fm_hy()`, qui avorte sur le provisoire.
- **Libération d'un `hy` remplacé** (G) : `free` de la struct. C'est l'équivalent exact de `pico8_instance_free` sur l'instance d'aujourd'hui : la table de champs et l'instance sont libérées, les REF imbriquées ne sont pas suivies. Contrat à écrire à la déclaration.
- **Tests de régression : lecture par `pico8_entity_get_field`, écriture par le membre** (G). Le lecteur à double forme rend exactement la valeur d'aujourd'hui, et survivra à 1a.36 sans nouveau diff. L'écriture passe par `pico8_fm_hy()`. `cheat_code:84-85` et `score_panel_stars:114,116` ont lieu après `eu()`.
- **Aucun des huit tests ne lit seulement le provisoire** (mesuré) : aucun ne garde un diff vide. La seule lecture du provisoire dans tout `tests/` est `test_pico8_jc:331`, déjà par `pico8_entity_get_field`, diff vide.
- **Accesseur typé dans le module du slot** (G) : `pico8_fm_hy()` à côté de `pico8_fm()`, dans `pico8_probe`, qui inclut déjà `pico8_economy.h`. Rejeté : caster `pico8_fm()` à chaque site, qui perdrait l'assert sur le provisoire.
- **Constructeur** (précédent `trade`) :
  - couverture enregistrée une fois ;
  - assert de clé porteur : « hy: every measured construction passes no key; a key here would be a field under no rule of plan-3 section 5 -- stop, do not add a member ».
- **Écarts port/cart** :
  - la mesure n'en a trouvé aucun dans `hy` ;
  - `hl` = `BOOL(0)` est le `false` du cart ;
  - le `fm` provisoire est un artefact du port (`fm` vaut nil dans le cart avant `eu()`), déjà nommé provisoire par le plan et traité à 1a.36 ;
  - la triche est un ajout du port (étape 0.1), pas un écart ;
  - si l'implémentation en découvre un, il est préservé et décrit par un `TROU-N` (§7).
- **Commentaires** (CLAUDE.md) :
  - à ajouter :
    - contrat de la struct (rien n'est libéré sauf la struct par `pico8_probe_set_fm`/`shutdown`, `invented` jamais libéré) ;
    - contrat de `pico8_fm()` (provisoire puis `hy`, lire par `pico8_entity_get_field`) et de `pico8_fm_hy()` (NULL, `hy`, ou assert) ;
    - assert de clé, assert de descripteur, assert sur le provisoire ;
  - à corriger ou retirer, rendus faux :
    - `pico8_economy.c:26-29` (« spec's `invented=o()` … swapped ») ;
    - `pico8_economy.h:12-14,21-23` (instance, « frees the previous `fm` ») ;
    - `pico8_probe.h:76-81` (« takes ownership », « frees the previous instance ») ;
    - `pico8_tech_gate.c:50-51` (« pico8_hy_pq … swaps ») ;
    - le message d'assert de `cart.c:250` ;
    - `test_pico8_playability.c:72` (« `hy`'s spec starts `nz` at 100 »), qui reste vrai ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert. La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_hy` sur l'ancien code** (voir Testing Decisions), valeurs figées. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si une construction porte une clé, ou si une fonction de `hy` reçoit le provisoire ou NULL sur un rejeu : la mesure est fausse et doit être corrigée dans le PRD.
2. **`hy` converti** :
   - struct, constructeur, descripteur, lecteur, `set_struct_new`, retrait du `pq` de la classe ; les cinq fonctions sur la struct ;
   - `pico8_fm_hy()`, libération par tag, contrats ;
   - appelants, lecteurs, écrivains (`menu`, `cheat` à double forme) ;
   - les huit tests de régression (accès) ; les accès des caractérisations ; le seul accesseur de `test_pico8_hy` qui lit ou écrit `fm`.

   Toutes les caractérisations passent **sans changer leurs valeurs figées**. Les vérifications de forme s'ajoutent à `test_pico8_hy`.
3. **Preuves** :
   - `make e2e` ;
   - rapport `gcov` ;
   - 2 à 3 mutations ;
   - preuves rouges des huit tests ;
   - `make class-coverage-net`, `make ref-audit`, `make float-audit` ;
   - ligne 1a.35 du plan.

   Puis le commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce qui suit une fonction de `hy`, lu par `pico8_entity_get_field` sur `fm`, les effets visibles (`iv` affichés, `kr` des planètes, `hk`/`hb` appelés sur `cv`), les pixels du panneau du haut, le compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_hy`** : lié comme `test_pico8_trade`/`test_pico8_hp` (cart réel, `ENGINE_SRCS`), avec des dépendances Makefile sur les fixtures qu'il rejoue.
  - Il lit `fm` par `pico8_entity_get_field` (11 noms), et l'écrit (pour préparer un scénario) par **un seul accesseur local**. Cet accesseur est la seule ligne du test que le morceau 2 réécrit (`->fields` → membre par `pico8_fm_hy()`), aux vérifications de forme près.
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()` puis `eu()` sur `bo[2]`, sauf le scénario 7.
  - Toute présence se lit par un `lookup` qui confond nil et absent.
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **Construction** : les 11 noms (7 présents, 4 absents avant `lj` ; après `eu()`, les 11 présents), genre et valeur ; `invented` liste vide ; deux `eu()` de suite donnent deux `invented` distincts.
    2. **`nx`** : `lo` accumulé ; passage de 12 sans « time compression » et de 15 avec (inventé à la main) ; `hk` déclenché une fois, `lo` ramené.
    3. **`hk`** : `nz`+=`ge`, `jk`+=`po[r.k]`+1, `yr`+1 ; les `hk(yr)` de `cv.hk` vus par la liste des planètes (valeur figée).
    4. **`lu`** :
       - `nz < jt` : rend 0, un `iv` « -no money- » ;
       - `jt` = 0 : rend 1, pas d'`iv` ;
       - `jt` > 0 et < 0 : `nz` et texte de l'`iv` ;
       - `lu(NULL, 5)` rend 0.
    5. **`lj`** : `po`/`fu` remplacés (identités changées), `ge`, `happiness`, `kr` des planètes (repli), et `hl` dans ses trois états :
       - « final score » à `yr` = 3426 ;
       - « bankrupt » (`ge` ≤ 0 et `nz` < 3, préparés) ;
       - `BOOL(0)`.
    6. **`kt`** : `jk` -= `gi(t)` ; `px` ajouté, ou rien sans `px` ; `ew` monté aux seuils de `ek` (2, 4, 7, 12) jusqu'à 4, jamais au-delà.
    7. **Triche sur le provisoire** : la séquence complète passée à `pico8_cheat_step` avant `eu()`. `jk`/`nz` sont lus sur le provisoire par `pico8_entity_get_field`, puis ils ont disparu une fois `eu()` passé.
    8. **Rejeux courts** `tech_screen_invent_events.json` et `game_end_events.json` (départ comme le filet, jusqu'à la dernière frame) : à chaque frame, repli des 11 noms de `fm` (compte d'`invented` et dernier élément, identités de `po`/`fu` changées ou non), puis empreinte d'écran finale. Figés.
  - **Forme** (ajoutée au morceau 2) :
    - `fm` après `eu()` est `STRUCT` au descripteur de `hy` (`pico8_hy_struct_class()`) ; toutes les méthodes NULL ; `field` posé, `set_field` NULL ;
    - `pico8_entity_set_field(fm, "nz", …)` avorte (dans un fils) ;
    - `pico8_hy_new` avec une clé `"x"` avorte ;
    - `pico8_hy_lu` sur une struct d'une autre classe (le `ok`) avorte ;
    - `pico8_fm_hy()` sur le provisoire (avant `eu()`) avorte ;
    - `pico8_hy_class()->struct_new` est posé et son `pq` NULL.
- **Tests de régression (§6)**, réécrits dans leur **seule façon de lire ou d'écrire `fm`** :
  - lecture par `pico8_entity_get_field` au lieu de `pico8_instance_get_field` ;
  - écriture par un membre de `pico8_fm_hy()` au lieu de `pico8_fm()->fields`.

  Aucune assertion retirée ni affaiblie, mêmes valeurs attendues, mêmes frames, même fixture. Makefile : dépendances de lien ajustées si nécessaire.

  Preuve rouge de chacun sur le code final, dans une copie jetable, par `make tmp/test_pico8_<nom> && ./tmp/test_pico8_<nom>` (`score_panel_stars` avec 1 et 2). Le commit note le bug remis, l'échec observé, la restauration et le vert. **Si un bug remis ne rend pas son test rouge, c'est un arrêt (§7), pas un test durci.**

  Ceux qui n'ont pas de correctif d'historique (tests de scénario ou de fonction) prennent un **bug de référence** (précédent `planet_kj_dispatch` 1a.8, `game_end` 1a.29, `tech_screen` 1a.31). Ce commit le place **dans le code de `hy` qu'il réécrit** quand le test en lit un effet.

  | Test | Accès réécrit | Bug d'origine | Remis localement | Attendu |
  |---|---|---|---|---|
  | `cheat_code` | `fm_fixed` (`:35-41`) ; écritures `:84-85` | Pas de correctif (test de la fonction, 676d8f3). **Référence** (G) : la triche n'écrit plus sur un `hy` | Dans `apply`, la branche `hy` n'écrit rien (seule la branche du provisoire reste) | `assert_cheat_applied` faux à la frame 23 |
  | `empty_planet_cleared` | `credits_now` (`:69-76`) | 529f355 : `jo`'s `"ll"` non enregistré, une planète vide jamais détruite (preuve de 1a.17/1a.18) | Retirer l'entrée `LL` du descripteur de `jo` | Une assertion du test échoue ; noter laquelle |
  | `playability` | `credits_now` (`:84-91`) | Aucun correctif ; **référence** de 1a.17 : double enregistrement dans `cg.jo` | Constructeur de `jo` qui appelle `pico8_ke_struct_pq` deux fois | `count <= PICO8_PLAYABILITY_MAX_PLANETS` faux |
  | `slipway_drag` | `credits_now` (`:28-35`) | 2d7021f : `ui:bh` passait nil à `ey` (preuve de 1a.17/1a.31) | Remettre nil à la place de l'entité de `jz("gv")` | `oe_count(origin) == 1` faux |
  | `game_end` | `fm_yr_is` (`:52-57`), `fm_hl` (`:59-67`) | Pas de correctif (test de scénario). **Référence** (G) dans `hy:lj` : `yr > 3426` au lieu de `>=` | Comparaison stricte dans `lj` | `assert(end_frame >= 0)` faux (les clics suivants sont absorbés, `yr` reste 3426) |
  | `geoharvest_burst` | `fm_fixed` (`:64-69`), `invented` (`:71-77`) | Pas de correctif (test de scénario). **Référence** (G) dans `hy:lu` : `nz` n'est plus écrit quand `jt < 0` | Écriture de `nz` gardée par `jt > 0` | `fm_fixed("nz") == nz_before + 20` faux à `HARVEST_FRAME` |
  | `score_panel_stars` | écritures `:114`, `:116` | 45b6f79 : les positionnels relus dans l'ordre de stockage, étoiles dessinées 3,1,2,4,5 | Constructeur de `lq` qui remplit `items` dans l'ordre de stockage (mutation 3 de 1a.34) | Rangée d'étoiles fausse pour 1 et 2 étoiles |
  | `tech_screen` | `fm_fixed` (`:51-56`), `invented_count` (`:58-64`) | Pas de correctif (test de scénario). **Référence** (G) dans `hy:kt` : `px` n'est plus ajouté à `invented` | Retirer l'ajout dans `kt` | `invented_count() == 1` faux à la frame 46 |

- **Caractérisations adaptées, valeurs figées inchangées** (précédent `in_gb`, 1a.33 ; `test_pico8_iv`, 1a.6). Chacune est lancée une fois sur l'ancien code (copie `git archive` jetable) avec son accès adapté, sauf les écritures de membre, qui n'existent pas sur l'ancien code. Le commit note qu'elle est verte.
  - Lecture `pico8_instance_get_field` → `pico8_entity_get_field` : `test_pico8_ie:648`, `test_pico8_js:514` (`invent`).
  - Écriture `pico8_entity_set_field(pico8_fm(), …)` → membre de `pico8_fm_hy()` : `test_pico8_fr:361`, `hj:438`, `mx:445`, `mz:521`, `nl:393`, `nr:413`, `os:392`, `qh:390` (`nz`), `pj:415,468` (`hl`).
  - Appel : `test_pico8_hp:397` passe `pico8_fm_hy()`.
  - Si l'une de ces valeurs ne peut pas être rendue à l'identique, c'est l'arrêt « valeur figée changée », pas une nouvelle valeur.
- **Tests existants à diff vide**, verts :
  - `test_pico8_jc` (lit le provisoire et `hy` par `pico8_entity_get_field`), `test_pico8_g_` ;
  - `score_panel_close`, `pause_menu_music`, `top_menubar`, `shop_tooltip_fillp`, `built_planet_panel_nil_row`, `rich_planet_exports`, `ga_export_panel_by`, `planet_gh_row_lq` ;
  - `test_pico8_je`, `test_pico8_nf` (libèrent le provisoire par `pico8_probe_shutdown`) ;
  - toutes les autres.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` les tests suivants, avec les objets du moteur : `test_pico8_hy`, `test_pico8_cheat_code`, `test_pico8_game_end`, `test_pico8_tech_screen`, `test_pico8_geoharvest_burst`, `test_pico8_score_panel_stars`. Les lancer, puis `gcov` sur :
    - `pico8_economy.c`, `pico8_probe.c`, `pico8_cheat.c`, `pico8_menu.c` ;
    - `pico8_tech_gate.c`, `pico8_score_panel.c`, `pico8_score_panel_display.c` ;
    - `pico8_cart.c`, `pico8_planet.c`, `pico8_planet_display.c`, `pico8_slipway.c`.
    - Même piège qu'à 1a.23-1a.34 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`.
    - Les branches d'assert atteintes seulement dans un fils qui avorte ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu.
    - Le rapport est collé dans le message de commit : lignes exécutables ajoutées ou modifiées de `src/` et leur compte. Il couvre :
      - le constructeur et le lecteur ;
      - les cinq fonctions, dont « bankrupt », `lu(NULL)`, `kt` sans `px` ;
      - `pico8_fm_hy()`, la libération d'un `hy` remplacé et du provisoire ;
      - les deux branches de `apply` ;
      - chaque lecteur et appelant réécrit.

      Les lignes à 0 sont données avec leur justification.
  - **2 à 3 mutations**, chacune rouge sur un test nommé, notées dans le commit avec le test qui l'attrape :
    1. **valeur d'un membre initial** : `yr` = 3400. `game_end` (`fm_yr_is(LAST_YEAR - 1)` à la frame 61) et le scénario 1 sont rouges ;
    2. **lecteur** : le nom `jk` rend le membre `ew` (entrée du lecteur remplacée). `tech_screen`, `cheat_code` et le scénario 6 sont rouges ;
    3. **ordre du constructeur** : `invented` laissé nil (liste non créée avant l'enregistrement). `oj`/`kt` ne voient plus de liste ; `tech_screen` et le scénario 1 sont rouges.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED` ; `PICO8_REF_AS_LIST` sur `invented` (`tech_gate.c`, `menu.c`, `economy.c`) gardé.
- **`make float-audit`** : vert, `tools/float_audit_c1.txt` inchangé.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** :
  - `test_pico8_trade`, `test_pico8_hp`, `test_pico8_lq` : structure, `lookup`, `must_abort`, fils par scénario, rejeu court, forme ;
  - `test_pico8_pj` : rejeu court de `game_end` frame à frame ;
  - `test_pico8_je`/`test_pico8_nf` (1a.6/1a.7) : slot à provisoire ;
  - le commit 4158302 (1a.34) : preuves rouges et plis adaptés notés dans le message.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; c'est le seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` (en particulier `hy  19  15/15`). Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier :
  - si un membre autre que les 11 (`nz`, `ge`, `jk`, `ew`, `invented`, `lo`, `yr`, `po`, `fu`, `happiness`, `hl`) est ajouté ;
  - si l'un d'eux devient un état statique de classe ;
  - si le constructeur accepte une clé sans avorter, ou si son assertion se déclenche sur un rejeu ou un test hors `must_abort` ;
  - si un écrivain est posé, ou une entrée de méthode ;
  - si l'enum change.
- `FAILED:` si un site hors de `pico8_class.c` passe un `hy` à `pico8_instance_get_field`, `pico8_instance_fields`, `pico8_instance_free` ou `->fields` (grep dans `src/` et `tests/`), ou si `pico8_economy.c` appelle encore `pico8_class_instantiate` ou `pico8_class_set_pq` pour `hy`.
- `FAILED:` si une fonction de `hy` ou `ix[1].my` accepte le `fm` provisoire sans avorter, ou si `pico8_fm_hy()` rend autre chose que NULL, un `hy`, ou un arrêt.
- `FAILED:` si une autre classe que `hy` est convertie ou change de forme. En particulier :
  - si le `fm` provisoire cesse d'être l'instance sans classe construite par `pico8_probe_init`, ou si `pico8_fm()` change de type (c'est 1a.36) ;
  - si la classe `hy` cesse d'être déclarée et enregistrée à la même place ;
  - si le comportement d'une autre classe change (valeur figée de sa caractérisation, ligne de couverture).
- `FAILED:` si la triche ne s'applique plus au provisoire sur l'écran titre (scénario 7 de `test_pico8_hy`).
- `FAILED:` si l'un des huit tests de régression change au-delà de sa façon de lire ou d'écrire `fm` (assertion retirée ou affaiblie, valeur attendue, frame ou fixture changées). Également si sa preuve rouge manque au message de commit (bug remis, échec observé, correctif restauré, vert), si un bug remis ne le rend pas rouge, ou si un autre test de régression (§6) a un diff non vide.
- `FAILED:` si une valeur figée de `test_pico8_hy` a été changée après la conversion, ou une d'une autre caractérisation (`ie`, `js`, `hp`, `pj`, `qh`, `nl`, `os`, `hj`, `mx`, `nr`, `fr`, `mz`…). Également si les accès adaptés en lecture n'ont pas été vérifiés verts sur l'ancien code (noté dans le commit).
- `FAILED:` si une ligne est ajoutée à `tools/float_audit_c1.txt` ou en est retirée, ou si `make float-audit` est rouge.
- `FAILED:` si `test_pico8_hy` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé (en particulier `hl` = `BOOL(0)` au lieu de `false`, ou `lu(NULL, jt)`), ou si un écart découvert n'a pas son `TROU-N` (entrée et référence au site).
- `FAILED:` si la note « Découvert à 1a.35 » manque à la ligne 1a.35 de `docs/plan-3.md`.

Critères de réussite (en plus) :

- la struct `hy` n'a que l'en-tête et les 11 membres ; son descripteur n'a aucune méthode, un lecteur, pas d'écrivain ;
- grep :
  - `PICO8_CLASS_COVERAGE_NAME(g_hy_class, "hy")` reste ;
  - `pico8_class_set_struct_new(g_hy_class, …)` est présent ;
  - `pico8_class_set_pq(g_hy_class` est absent ;
  - aucun `pico8_instance_get_field`/`pico8_instance_fields` dans `pico8_economy.c`, `pico8_tech_gate.c`, `pico8_score_panel.c`, `pico8_score_panel_display.c`, ni sur `fm` dans `pico8_menu.c` ;
  - `pico8_instance_fields` n'est plus appelé dans `pico8_cheat.c` que dans la branche du provisoire ;
  - `pico8_hy_new` rend `pico8_hy_t *` ; les cinq fonctions prennent `pico8_hy_t *` ;
- le message de commit dit « découvert à 1a.35 ».

## Out of Scope

- Le `fm` provisoire : sa conversion en struct sans parent, le retypage de `pico8_fm()`, la fermeture de la double forme de `cheat.c` et de la libération par tag (1a.36).
- Les autres instances sans classe : `{}` de `nr:gv`, `et={}`, `{qs}`, `re` (1a.37-1a.40), l'enveloppe de `pz` sur `g_ha` (1a.41).
- Les tables `po`/`fu` elles-mêmes (produites par `bx`/`ck`), `ek`, `r`, `nb` : données, phase D.
- Typer les membres (`nz` en fixe, `invented` en liste, `hl` en chaîne ou booléen) : phase 1b.
- Libérer `invented`, `po`, `fu` d'un `hy` remplacé (TROU-7, Plan 2 §8).
- Supprimer la classe `hy`, les registres et `pico8_class.c` : 1a.42.
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Racine, pas de parente à fermer** : aucune fonction partagée ne passe à la forme struct seule ici ; aucune double forme ouverte ne se solde. La double forme ouverte par ce commit (lecteurs de `fm`, `cheat.c`, libération par tag) se ferme à 1a.36.
- **Ce que le filet voit de `hy`** : 19 constructions sur 15 rejeux, environ 169 000 lectures de `hl` (`pj:ph`), 30 000 de `invented` (`oj`), 588 `lj`, 85 `hk`, 27 `kt`, 3 paiements négatifs, 2 fins de partie, 11 triches.
- **Ce qu'il ne voit pas** : « bankrupt », le passage de `nx` avec « time compression », `kt` sans `px`, `ew` à 4, la triche sur le provisoire, `lu(NULL)`. La caractérisation dirigée et les mutations visent ces cas.
- **Pour 1a.36** : grâce au lecteur à double forme, les huit tests de la ligne 1a.35 (« Tests : ceux de 1a.35 ») n'auront à changer, à 1a.36, que si `pico8_fm()` change de type ou si l'accesseur `pico8_fm_hy()` est fusionné avec lui.
