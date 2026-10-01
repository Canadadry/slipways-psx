---
title: "Plan 3, étape 1a.5 : `fq` (voile de clic) devient une struct, et la boucle de survol de `ui` lit et écrit les deux formes"
description: "Le voile `fq` est encore une instance de la couche objet : il passe en struct fille de `ke` avec sa méthode `ldown`, la boucle de survol de `ui` (non convertie) apprend à lire et écrire une struct, et `score_panel_close` est réécrit pour lire la struct puis prouvé rouge avec le bug `de.my` remis."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.5 — `fq` (`ui_button.c:297`), méthode `ldown`. Tests : `score_panel_close`.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Clés construites à l'exécution** : `self[bt.."held"]` (`pico8_ui.c:328`) → deux champs `lheld`/`rheld` ; `ldown`/`rdown` côté méthodes.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe.

§6 : `score_panel_close` est réécrit pour lire la struct **dans ce commit**, et doit échouer si le bug d'origine est remis : bug remis localement, test rouge, correctif restauré. §7 : chaque commit de 1a passe `make class-coverage-net`, sortie identique à `tools/class_coverage_net_expected.txt`.

### Ce que 1a.1, 1a.2 et 1a.4 ont posé (commits 9ce8bfa, 42fdf50, f5cc6d1), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT` ; en-tête commun (tag, pointeur vers le descripteur `static const` de la classe).
- Descripteur de classe struct (`pico8_struct.h`) : tableau de méthodes indexé par l'enum (`OL`, `PH`, `KJ`, `EV` aujourd'hui) + lecteur de champ par nom (« rend 1 si le champ est non nil ») ; table de noms parallèle (`pico8_method_of`). Un nom absent de l'enum est cherché comme champ.
- `pico8_ke_t` (en-tête, `me`, `t`), `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq` (`lt`), qu'une fille **sans `pq` propre appelle seule** (contrat écrit dans `ke.h`).
- `pico8_class_t.struct_new` : `pico8_class_instantiate` délègue avant toute allocation, tout `pq` et tout comptage ; c'est le constructeur qui enregistre la couverture.
- `pico8_entity_get_field`, lecteur à double forme utilisé par `lt`/`he`, `dc`, `mv`, `dg` et le scan `fx` de `mz:pq`. `dg` écrit `t` directement dans `pico8_ke_t`. `pico8_instance_get_field` asserte ne jamais recevoir de struct.
- Un champ fonction propre à l'instance (`mh.lh`) est un **champ**, pas une entrée d'enum : `mv` l'appelle parce que sa valeur est de genre fonction.
- Modèle de test : `test_pico8_iv`, `test_pico8_mh` (caractérisation figée sur l'ancien code, mutations notées dans le commit).

### Emprise réelle de `fq` (mesurée sur 0fbca60)

**Cart** (`pico8/lua.lua:324-329`) :

```
fq=ke:jr([[ fa=14, hd=1, ]])
function fq:ldown() self.p_=mv(self,"my") return true end
```

Construit par `fq()` (`pj:ph`, `:871`, fin de partie) et `fq(de)` (`ix[3].my`, `:985`, ouverture du panneau des scores), avec `de={my=function() ju.c_=not ju.c_ return true end}` (`:901-903`). `jr` fait une copie superficielle (`mw`) de la table reçue.

`fq` ne dessine rien (ni `kj` ni `ev`) : c'est un voile invisible qui, par `hd=1`, est **toujours survolé**, et, en couche 14, avale tout clic gauche que rien de la couche 15 ne prend. Le filet ne le voit qu'indirectement (clics absorbés, sprite du curseur).

| Aspect | Mesure |
|---|---|
| Constructions | 2 : `score_panel_display.c` (`pj:ph`, `pico8_fq_new(NULL)`, valeur ignorée) et `menu.c` (`ix[3].my`, `pico8_fq_new(pico8_de())`, valeur ignorée). Aucun global ne tient un `fq`. |
| Défauts | `fa`=14, `hd`=1 (valeur fixe, **pas** une fonction). Hérités de `ke` : `me`="ph", `t`=0. Aucun défaut réaffecté ni modifié en place. |
| Clés reçues | aucune (`fq()`) ou `my` seulement (`de`, table du module de `pj`, empruntée, jamais gardée). |
| Champs écrits sur l'instance | `p_` (par `fq:ldown`), `t` (par `dg`), `m_` (par la boucle de survol de `ui`, sur `o_` et sur `gj`). |
| Méthodes appelées par nom | `ldown` via `jz("ldown")` — seule vraie méthode de classe ; `my` via `mv(self,"my")` — **champ** fonction reçu, comme `mh.lh` ; `hd` via `mv(e,"hd",…)` — champ valeur, `mv` rend 1 ; `ph` via `dg` (`me`) — absente, `mv` rend 0. Pas de `pq` propre. |
| Noms lus sur un `fq` et absents | `rdown`, `gv` (`jz`), `ql` (`ui:ev`), `fg` (`ui:gw`), `nu`, `kj`/`ev`/`hk`/`hb` (`lt`/`he`), `fx` (scan de `mz:pq`), `gv` (`jc:hq` sur `ok.o_`). |
| `->defaults` | 1 site : installation de `ldown` dans `pico8_fq_init`. `cart.c` asserte `pico8_fq_class()->parent == pico8_ke_class()` (classe gardée, reste vrai). |
| `bm` / `nu` | `hd` / — → `hv`, `cv.hd`. Aucune entrée `cg`. |
| Consommateurs génériques | `lt`/`he`, `dg` (`me`, `t`, `p_`), `mv` : déjà à double forme. Scan `fx` de `mz:pq` : déjà à double forme. `dc` : jamais atteint (ni `kj` ni `ev`). `economy` (`cv.hk`/`cv.hb`) : non atteint. **Boucle de survol de `ui`** (`pico8_ui_gw`) : lit `fa` par `pico8_instance_get_field` (assert sur une struct), écrit `m_` par `((pico8_instance_t *)…)->fields` sur `gj` et `o_` (**écriture hors bornes** sur une struct), lit `fg` par `pico8_ui_field_raw` (couche). **`pico8_ui_ev`** : lit `ql` sur `o_` par `pico8_ui_field_raw` (couche). **`pico8_ui_jz`** : `mv` seulement (déjà à double forme) ; `out_kv` rend le pointeur tel quel. **`pico8_ui_bh`** : `pw.kn` de `fq:ldown` est un booléen, jamais une table → `lheld` n'est jamais un `fq` ; `jz("gv",…)` ne rend jamais un `fq` (pas de `gv`), donc `ey` ne le reçoit pas. **`jc:hq`** (`slipway.c:105-107`) : lit `gv` sur `ok.o_` par la couche — un `fq` y est dès qu'il est survolé, donc dès qu'il existe ; atteint si un `jc` est tenu à ce moment. `gu(le, o_)` (`ui:lj`) : comparaison d'identité de REF, sans lecture. |
| `pico8_ref_audit.h` | aucune macro ne reçoit un `fq` (les casts de `gw` sur `gj`/`o_` sont des casts nus, remplacés ci-dessous). |
| Tests qui lisent un `fq` | `score_panel_close` : `fq_alive()` lit `e->cls` sur tout `hv` sans tester le tag (note de 1a.3). `game_end` : ne lit pas `fq`, dépend de lui (trois clics absorbés). `geoharvest_burst` : `live_ga` lit `e->cls` sans tag sur `hv` (inoffensif, déjà vrai pour `iv`/`mh`, réécrit en 1a.26). `test_pico8_entity_smoke` : commentaire seulement. |
| Couverture | `fq  3  2/15` (inchangé). |

### Le bug que garde `score_panel_close`

`docs/plan-2.md:422` : « `de.my` rendait nil là où le cart rend `true` (lua.lua:901-903), et le clic qui referme le panneau ouvert par `ix[3]` faisait avorter le port dans `pico8_dg_truthy`. » Le correctif (`fe6473d`, écrasé depuis par le squash de #17) est visible en comparant `77c7f94^` et `77c7f94` : `pico8_pj_de_my_dispatch` (`score_panel_display.c`) finissait par `return pico8_value_ref(NULL);`, il finit par `return pico8_value_bool(1);`.

Mécanisme sur l'ancien code : `fq:ldown` (garde TROU-9 : n'écrit que si `mv` rend 1) stocke REF NULL dans `p_` de l'instance, le champ est donc *présent* et vaut nil, et l'assert de `pico8_dg_truthy` part. Sur une struct, un membre nil est *absent* (contrat du lecteur de champ) : le bug remis ne fait plus avorter `dg`, il laisse le voile en place — `c_` bascule quand même à faux, mais `fq` survit. C'est l'assertion `!fq_alive()` à `CLOSED_FRAME` qui rougit. L'intention du test (« un clic referme le panneau : `c_` faux **et** le voile parti ») est inchangée ; seul le mode d'échec change, ce que le §6 permet (« dans sa façon de lire l'état »).

## Solution

1. **Struct `fq`** : premier membre `pico8_ke_t`, puis `fa`, `hd`, `my`, `p_`, `m_` (`pico8_value_t`). Déclarée dans le module des boutons, publique (les deux appelants et le test la nomment).
2. **Constructeur** (posé comme `struct_new` de la `pico8_class_t` de `fq`), dans l'ordre :
   1. alloue ;
   2. `pico8_ke_struct_init` avec le descripteur de `fq` ;
   3. défauts : `fa`=14, `hd`=1, `my`/`p_`/`m_` nil ;
   4. recopie les clés reçues (table NULL permise) : `my` seulement ; toute autre clé → assertion porteuse de message (arrêt §7) ;
   5. enregistre `"fq"` au crochet de couverture ;
   6. `pico8_ke_struct_pq` seul (`fq` n'a pas de `pq`).
3. **Descripteur de `fq`** : méthode `LDOWN` (nouvelle entrée de l'enum, nom `"ldown"`), lecteur de champ qui connaît `fa`, `hd`, `my`, `p_`, `m_` et délègue à `pico8_ke_field`, **écrivain de champ** qui connaît `m_`.
4. **`fq:ldown` sur la struct** : `self.p_ = mv(self,"my")` littéral — la valeur rendue par `mv`, ou nil si `mv` rend 0, écrite dans le membre ; rend vrai.
5. **Écriture générique de champ, nouvelle dans le noyau** : le descripteur gagne un écrivain de champ par nom ; le module d'entités gagne `pico8_entity_set_field(e, nom, valeur)` — `INSTANCE` → écriture dans `fields` comme aujourd'hui ; `STRUCT` → écrivain du descripteur, qui doit connaître le nom (assert sinon : une struct n'a pas de champ à faire pousser).
6. **Boucle de survol de `ui` (coexistence, `ui` reste une instance jusqu'à 1a.31)**, strictement les sites qui reçoivent une entité de `cv.hd` : lecture de `fa` dans `gw` par `pico8_entity_get_field` ; écritures `m_` sur `gj` et `o_` par `pico8_entity_set_field` ; `pico8_ui_field_raw` (qui sert à lire `fg` et `ql` sur `o_`) passe par `pico8_entity_get_field`. Aucune autre ligne de `ui.c`.
7. **`jc:hq`** : la lecture de `gv` sur `ok.o_` passe par le lecteur à double forme. Rien d'autre dans `slipway.c`.
8. **`pico8_class_t` de `fq`** : même déclaration, même spec, même place, même nom de couverture ; reçoit le constructeur ; perd l'installation `->defaults` de `ldown`.
9. **`score_panel_close`** : `fq_alive()` teste le tag `STRUCT` et le descripteur de `fq` (accesseur public) au lieu de `e->cls` ; le reste du fichier inchangé. Preuve rouge avec `de.my` rendant nil, puis correctif restauré.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `fq` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise réelle de `fq` (constructions, champs, méthodes, `->defaults`, consommateurs génériques, tests) soit mesurée dans le PRD, afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux que la struct `fq` ait exactement les membres de sa chaîne et de ses écritures (`fa`, `hd`, `my`, `p_`, `m_`) après `pico8_ke_t`, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que gardien du filet, je veux que `fa`=14 et `hd`=1 soient égaux aux valeurs que le parseur de spec produit sur la chaîne de spec de `fq`, afin que le voile reste en couche 14, sous les boutons de la couche 15.
5. En tant que gardien du filet, je veux que `hd` reste une valeur (fixe 1), pas une méthode, afin que `lt` enregistre `fq` dans `cv.hd` par présence du champ et que `mv(fq,"hd",…)` rende 1 sans rien appeler.
6. En tant qu'auteur des étapes suivantes, je veux que l'enum ne gagne que `LDOWN`, afin que chaque ajout reste attaché à une étape ; `rdown` viendra avec la première classe qui le porte (`bg`, `nr`).
7. En tant que mainteneur, je veux que `my` soit un champ fonction recopié de la table reçue, pas une entrée d'enum, afin que `mv(self,"my")` appelle la fermeture de `de` exactement comme `mh.lh`.
8. En tant que mainteneur, je veux qu'une clé reçue autre que `my` déclenche une assertion porteuse de message, afin qu'un champ jamais mesuré ne soit pas perdu en silence.
9. En tant que mainteneur, je veux que le constructeur ne garde aucun pointeur sur la table reçue (`de` appartient à `pj`), afin que 1a.29 puisse changer `de` sans toucher `fq`.
10. En tant que gardien du filet, je veux que le constructeur appelle `pico8_ke_struct_pq` une seule fois et rien d'autre, afin que l'enregistrement dans `hv` et `cv.hd` ait lieu au même moment et dans le même ordre que par la marche de chaîne.
11. En tant que gardien du filet, je veux que `lt` enregistre un `fq` dans `hv` et `cv.hd` et nulle part ailleurs, afin que l'ordre de `cv.hd` — donc le dernier `o_` survolé — soit inchangé.
12. En tant que gardien du filet, je veux que `jz("ldown")` trouve `ldown` par la table du descripteur, afin qu'un clic sur le voile soit absorbé comme avant.
13. En tant que gardien du filet, je veux que `jz("rdown")` et `jz("gv")` passent sur un `fq` sans rien trouver, afin que le clic droit et le lâcher d'un glisser traversent le voile comme avant.
14. En tant que gardien du filet, je veux que `fq:ldown` écrive dans `p_` la valeur que rend `my`, ou nil sans `my`, et rende vrai, afin que `fq(de)` se retire au clic et que `fq()` reste en place et avale les clics (`game_end`).
15. En tant que gardien du filet, je veux que `bh` ne range jamais un `fq` dans `lheld` (`kn` booléen), afin qu'aucun `hq`/`ey` ne soit appelé sur le voile.
16. En tant que gardien du filet, je veux que `dg` appelle `me`="ph" sur un `fq` sans rien trouver, incrémente `t` dans la struct `ke` et retire le voile quand `p_` est vrai, afin que la fermeture ait lieu au même tick.
17. En tant que gardien du filet, je veux que la boucle de survol lise `fa` d'une struct par le lecteur à double forme, afin que le voile entre dans `ic[14]` comme avant au lieu de faire partir l'assert de la couche.
18. En tant que gardien de la mémoire, je veux que les écritures `m_` de la boucle de survol sur `gj` et `o_` passent par l'écriture générique, afin qu'aucune écriture `->fields` ne tombe hors bornes dans une struct.
19. En tant que gardien du filet, je veux que `fg` et `ql` lus sur `o_` passent par le lecteur à double forme et soient absents sur un `fq`, afin que le sprite du curseur (repli sur `ui.ql`) et le `poke(0x5f80,…)` soient identiques.
20. En tant que mainteneur, je veux que `pico8_ui_field_raw` garde un résultat identique sur les instances (`ui`, `je`, le `je` sans classe), afin que le changement de lecteur ne déplace rien pour ce qui n'est pas converti.
21. En tant que gardien du filet, je veux que `jc:hq` lise `gv` sur `ok.o_` par le lecteur à double forme, afin qu'un voile apparu pendant un glisser de slipway ne fasse pas avorter le port.
22. En tant qu'auteur des étapes suivantes, je veux une écriture générique de champ par nom dans le noyau, miroir du lecteur, afin que `ui` (`m_`) et les futures filles survolées (`js`, `bg`, `ie`, planètes) n'aient pas besoin de cas par classe dans du code générique.
23. En tant que mainteneur, je veux que l'écriture générique asserte quand une struct ne connaît pas le nom, afin qu'une écriture sur un champ jamais mesuré soit un arrêt et pas une perte silencieuse.
24. En tant que mainteneur, je veux que l'écrivain de champ d'un descripteur puisse être NULL pour une classe qu'aucun écrivain générique n'atteint (`jb`, `iv`, `mh`), avec assert porteur dans le noyau, afin de ne pas écrire de code sans chemin d'exécution.
25. En tant que mainteneur, je veux que la `pico8_class_t` de `fq` reste déclarée à la même place avec son nom de couverture et le constructeur, afin que `KNOWN[]`, le registre et l'assert de `cart.c` sur son `parent` tiennent.
26. En tant que gardien de la couverture, je veux que le constructeur enregistre `"fq"` une fois, jamais `"ke"`, afin que `fq  3  2/15` et `ke  0  0/15` restent identiques.
27. En tant que mainteneur, je veux que l'installation `->defaults` de `ldown` disparaisse, remplacée par la table, afin d'appliquer la règle des accès directs aux défauts.
28. En tant que gardien des tests de régression, je veux que `score_panel_close` lise la struct (tag + descripteur) et plus `e->cls`, afin que le test observe vraiment le voile et ne lise plus des octets de `ke.me`.
29. En tant que gardien des tests de régression, je veux que `score_panel_close` soit prouvé rouge avec `de.my` qui rend nil, puis vert avec le correctif restauré, afin que la réécriture ne l'ait pas vidé de son objet.
30. En tant que gardien des tests de régression, je veux que `game_end`, `geoharvest_burst`, `score_panel_stars` passent avec un diff vide, afin que la conversion ne se valide pas en affaiblissant un test.
31. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
32. En tant que mainteneur de la mémoire, je veux que la struct `fq` ne soit jamais libérée, comme l'instance avant, afin que TROU-7 reste la seule convention de fuite.
33. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés, afin de ne pas laisser de contrat mensonger.

## Implementation Decisions

Pas d'outil d'agent dans cette session pour lancer un agent `guideline` frais : j'ai chargé et appliqué moi-même le skill `guideline` (simplicité, changement chirurgical, hypothèses explicites, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Struct `fq`** : `{ pico8_ke_t ke ; fa ; hd ; my ; p_ ; m_ }`, dans le module des boutons, à côté de la classe `js` qui reste une instance. `m_` est membre parce qu'il est écrit sur l'instance par `ui` (règle des membres), même si rien ne le relit sur un `fq` (`fq` n'a pas de `kj`).
- **Enum** : `LDOWN` ajouté, nom `"ldown"`. Rien d'autre (ni `RDOWN`, ni `HD`, ni `MY`).
- **(G) `my` champ, `hd` champ** : même traitement que `mh.lh` ; `hd=1` est une donnée, et `mv` rend déjà la valeur d'un champ non fonction (contrat posé en 1a.2 pour ce cas précis).
- **(G) `fq:ldown` écrit `p_` sans la garde TROU-9.** Sur la struct, nil dans un membre = champ absent : `self.p_=mv(self,"my")` se traduit exactement. Comportement identique à l'ancien sur tous les chemins non buggés (`my` absent : `p_` reste nil ; `my` rend vrai : `p_` vrai). Les deux `/* TROU-9 */` de `fq_ldown` (`.c` et `.h`) sont retirés ; l'entrée TROU-9 reste (encore citée par `ob.h` et `msgbox.c`).
- **(G) Écrivain de champ dans le descripteur, pas de cas par classe dans `ui.c`.** Rejeté : caster en `fq` dans `gw` (code générique qui nomme une classe) ; `m_` dans `pico8_ke_t` (la chaîne de `ke` n'a pas `m_`, viole la règle des membres). Retenu : le miroir exact du lecteur — `e[k]=v` du cart. L'écrivain de `fq` ne connaît **que `m_`** : c'est le seul nom qu'un code générique écrit sur un `fq` (mesuré ; `t` passe par `pico8_ke_t` dans `dg`, `p_` par `fq:ldown` qui connaît la struct). Pas de miroir complet spéculatif. Contrat à l'en-tête : « rend 1 si `self` a le membre `name` et l'a écrit, 0 sinon ». Les descripteurs de `jb`, `iv`, `mh` n'ont pas d'écrivain (NULL) ; le noyau asserte non NULL avec message quand il atteint une struct.
- **`pico8_entity_set_field`** : dans le module d'entités, à côté de `pico8_entity_get_field`. `INSTANCE` → `pico8_ob_table_set_name(fields, …)` (inchangé) ; `STRUCT` → écrivain, assert qu'il rend 1 (« champ sous aucune règle, §7 »). `dg` n'est pas touché (son écriture de `t` reste telle quelle : changement chirurgical).
- **Boucle de survol de `ui`** — coexistence, minimale et littérale (`ui` reste une instance jusqu'à 1a.31) :
  - `gw` : `fa` lu par `pico8_entity_get_field` ; `(self.gj or {}).m_=false` et `(self.o_ or {}).m_=true` par `pico8_entity_set_field` ; `self.ic`, `self.o_` (écrits sur `ui` lui-même) inchangés ;
  - `pico8_ui_field_raw` passe par `pico8_entity_get_field` : identique pour une instance (le lecteur délègue à la couche), et couvre `fg` (`gw`) et `ql` (`ev`) lus sur `o_` ;
  - `jz`, `bh`, `lj`, `cj` inchangés (`mv` et identité de REF seulement) ;
  - rien d'autre dans `ui.c` : `lheld`/`rheld`, `ldown`/`rdown` en champs de `ui`, `le`, `hw` sont l'affaire de 1a.31.
- **`jc:hq`** : la lecture `ok.o_.gv` passe par le lecteur à double forme (un seul appel remplacé, sur `hovered` seulement). Site découvert par la mesure et couvert par la règle de coexistence (consommateur de `o_`, donc d'une entrée de `cv.hd`) : pas un arrêt. `jc` et ses autres lecteurs sont inchangés.
- **Constructeur** : ordre de la Solution §2. `pico8_fq_new` rend la struct ; les deux appelants ne changent que le type de leur variable (`menu.c`) ou rien (`(void)` dans `score_panel_display.c`). `pico8_fq_ldown` prend la struct.
- **`pico8_class_t` de `fq`** : garde sa chaîne de spec (copie en lecture, `fq` n'a pas de descendant) et son nom de couverture ; reçoit `struct_new` ; perd l'installation `->defaults` de `ldown`. Grep attendu : plus aucun `g_fq_class->defaults`.
- **Identification pour les tests** : accesseur public du descripteur `static const` de `fq` (même rôle que `pico8_fq_class()`, sans logique). `fq_alive()` teste `pico8_type_tag_of(e) == STRUCT` **puis** la comparaison du descripteur de l'en-tête — la lecture sans tag relevée en 1a.3 disparaît.
- **(G) `live_ga` de `geoharvest_burst` laissé tel quel** : il voit désormais aussi des `fq` (et `iv`/`mh` depuis 1a.2/1a.4) ; lecture toujours fausse, jamais déréférencée ; sa réécriture et sa preuve rouge appartiennent à 1a.26.
- **`pj`, `de`, `ju.c_`** : inchangés. `de` reste une table du module de `pj`, lue une fois par le constructeur.
- **Commentaires** (CLAUDE.md) : seulement le contrat de la struct (`fq` jamais libéré, TROU-7 ; `my` est la fermeture reçue, pas une méthode), le contrat de l'écrivain et de `pico8_entity_set_field`, les deux asserts porteurs. Corriger ceux que le changement rend faux : en-tête de `pico8_fq_init` (« wires `ldown` onto its defaults »), le bloc d'en-tête de `score_panel_close` s'il nomme la lecture de `cls`. Tout le reste va dans le message de commit (dont « `jc:hq` : découvert à 1a.5 » et le mode d'échec de la preuve rouge).

## Implementation Chunks

**Un seul commit** (§7 : une classe par commit). Chaque morceau est un point d'arrêt TDD ; chacun laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation de `fq` sur l'ancien code.** Nouveau test (voir Testing), valeurs et empreintes figées **avant** toute modification de `ui_button.c`, `ui.c`, `slipway.c`, `entity.c`, `struct.h`.
2. **Machinerie, sans classe convertie.** `LDOWN` dans l'enum ; écrivain dans le descripteur (NULL pour `jb`/`iv`/`mh`) ; `pico8_entity_set_field` ; `gw`, `pico8_ui_field_raw`, `jc:hq` branchés sur le lecteur/écrivain à double forme ; tests de la couche avec la struct factice. Aucune classe du jeu ne change de forme : filet et couverture trivialement identiques.
3. **`fq` convertie + `score_panel_close` réécrit.** Struct, descripteur, constructeur, `ldown` sur la struct, classe reliée, `->defaults` retiré, accesseur du descripteur ; `fq_alive()` réécrit. Le test du morceau 1 passe **sans changer ses valeurs figées**. Puis preuve rouge (voir Testing), correctif restauré.

## Testing Decisions

- **Un bon test observe le comportement** : appartenance à `hv`/`cv.hd` et son ordre, identité de `ok.o_`/`ok.gj`, contenu de `ic`, ce que rendent `mv` et `jz`, champs lus par `pico8_entity_get_field`, pixels, compte de couverture. Jamais l'agencement mémoire au-delà de « en-tête à l'offset 0 ».
- **Nouveau test `test_pico8_fq`** (dans `make unit`, lié comme `test_pico8_mh`, sur le cart réel chargé). Il lit l'état **uniquement** par l'API à double forme (`pico8_entity_get_field`, `pico8_mv`, `pico8_hv`/`pico8_cv`, `pico8_ok`) pour être le même code avant et après, au type de retour de `pico8_fq_new` près.
  - **Caractérisation, figée sur l'ancien code** :
    1. `fq` construit avec une table `{my = fonction qui compte ses appels et rend vrai}` : relevé par tick (`dg`) de `t`, `p_`, présence dans `hv`/`cv.hd` ; `jz`/`ldown` appelé une fois au tick N : `my` appelé une fois, `p_` vrai, retiré au `dg` suivant ;
    2. `fq` construit sans table : `ldown` rend vrai, `p_` reste absent, toujours présent après K ticks ;
    3. `my` qui rend faux : `p_` faux, voile toujours présent ;
    4. survol réel : après `eu()` sur `bo[2]` et un `fq` construit, un `pico8_ui_lj` (ou `gw` puis `jz`) sur le vrai `ok` : `ok.o_ == fq`, `fq` dans `ic[14]`, `m_` de `fq` vrai, `peek(0x5f80)`, puis au tour suivant après retrait `m_` de `fq` faux (écrit via `gj`) ; `jz("ldown")` rend `fq`, `jz("rdown")` ne le rend pas ; empreinte de `pico8_ui_ev` (sprite du curseur) sur un framebuffer neuf ;
    5. rejeux `score_panel_close_events.json` et `game_end_events.json` pilotés comme les tests de scénario : par frame, présence d'un `fq` dans `hv` (repéré par `cv.hd` et `fa`=14 lu par le lecteur à double forme), `ok.o_ == ce fq`, empreinte d'écran ; replié en une empreinte de séquence + nombre de frames, figés.
  - **Défauts** : `fa`, `hd` égaux aux valeurs parsées de la chaîne de spec de `fq`.
  - **Forme** (ajouté au morceau 3) : `pico8_fq_new` et `pico8_class_instantiate(pico8_fq_class(), …)` rendent un objet tagué `STRUCT`, avec le descripteur de `fq`, présent dans `hv` et `cv.hd`, absent de `cv.kj`/`ev`/`hk`/`hb` ; `mv(fq,"hd")` rend 1 ; `mv(fq,"ph")` et `mv(fq,"rdown")` rendent 0.
  - **Mutations** : l'implémenteur remet localement au moins 6 erreurs plausibles et note dans le commit celles qui sont attrapées : `p_` non écrit par `ldown` ; `my` non recopié ; `hd` absent (le voile sort de `cv.hd`) ; `fa`=15 ; `LDOWN` retiré de la table ; écrivain de `fq` qui ignore `m_` ; `ldown` qui rend faux. **Mutant équivalent à justifier dans le commit** : remettre la garde TROU-9 (n'écrire `p_` que si `mv` rend 1) — équivalent sur la struct, puisque nil dans un membre et membre non écrit sont indiscernables par le lecteur ; ce n'est pas une faiblesse du test. `m_` non écrit est équivalent pour le filet et les rejeux (rien ne relit `fq.m_`), mais doit être attrapé par le scénario 4, qui le lit.
- **Couche d'entités** (dans `test_pico8_entity_smoke`, supprimé en 1a.42) : `pico8_entity_set_field` sur une instance écrit dans `fields` ; sur la struct factice avec écrivain, écrit le membre et le lecteur le relit ; sur un nom inconnu d'une struct, et sur une struct sans écrivain, l'assert part (vérifié par un test de mort de processus si le dépôt en a le moyen, sinon par revue — ne pas inventer de harnais).
- **`score_panel_close`, test de régression nommé** :
  - réécriture limitée à `fq_alive()` (tag puis descripteur) et, si besoin, aux `#include` ; `ju_c_()`, frames, fixture, asserts et messages inchangés ;
  - **preuve rouge**, sur le code final : remettre localement `return pico8_value_ref(NULL);` à la fin de `pico8_pj_de_my_dispatch` → `tmp/test_pico8_score_panel_close` échoue (attendu : assert `!fq_alive()` à `CLOSED_FRAME`, parce que le voile reste) ; restaurer `return pico8_value_bool(1);` → vert. Noter dans le commit la commande, le mode d'échec observé, et qu'il diffère de celui d'origine (abort dans `pico8_dg_truthy`) pour la raison donnée au Problem Statement ;
  - si le bug remis ne rend **pas** le test rouge, c'est un arrêt (§7), pas un test durci.
- **Autres tests de régression** : `game_end` (trois clics absorbés par le voile sans `my`), `score_panel_stars`, `geoharvest_burst` passent avec un diff vide.
- **Couverture** : `make class-coverage-net` code 0, sortie identique à la référence (`fq  3  2/15`, `ke  0  0/15  exemptée …`).
- **Prior art** : `test_pico8_mh` (caractérisation figée, champ fonction appelé par `mv`) ; `test_pico8_iv` ; `test_pico8_score_panel_close` et `test_pico8_game_end` (pilotage de rejeu sans fenêtre) ; `test_pico8_entity_smoke` (struct factice).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `fq` n'est pas `fq  3  2/15`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé de `fq` ne tombe sous aucune règle du §5 — notamment si l'assertion des clés reçues (hors `my`) ou celle de l'écrivain générique (nom inconnu) se déclenche sur un rejeu ou un test : arrêt, pas de membre improvisé.
- `FAILED:` si `score_panel_close` devrait changer d'intention pour passer (frames, fixture, asserts sur `c_` ou sur la présence du voile modifiés), si son diff dépasse la lecture de l'état (`fq_alive()`, includes), ou si le commit ne rapporte pas la preuve rouge (bug `de.my` → nil remis, test rouge, correctif restauré, vert).
- `FAILED:` si `tests/test_pico8_game_end.c`, `tests/test_pico8_geoharvest_burst.c` ou `tests/test_pico8_score_panel_stars.c` a un diff non vide.
- `FAILED:` si une autre classe que `fq` est convertie — en particulier si `ui`, `js`, `jc` ou `pj` reçoivent un constructeur de struct, si `lheld`/`rheld` deviennent des membres, ou si un `->defaults` d'une autre classe bouge.
- `FAILED:` si la `pico8_class_t` de `fq` n'est plus déclarée ou enregistrée à la même place, ou si `ke`/`iv`/`mh`/`jb` changent de forme.
- `FAILED:` si les valeurs figées du test de caractérisation de `fq` ont été changées après la conversion.

Critères de réussite (en plus) :

- grep : plus aucun `g_fq_class->defaults` ; plus aucun `->fields` sur `gj`/`o_` dans `pico8_ui_gw` ; plus aucun `pico8_instance_get_field` sur une entité de `cv.hd` dans `gw` ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN` ;
- la struct `fq` n'a que `pico8_ke_t` et les 5 membres listés ; l'écrivain de `fq` ne connaît que `m_` ;
- dans `ui.c`, le diff se limite à `pico8_ui_field_raw`, la lecture de `fa` et les deux écritures `m_` de `gw` ; dans `slipway.c`, à la lecture de `gv` sur `hovered` ; dans `menu.c`/`score_panel_display.c`, au type du `fq` construit.

## Out of Scope

- `ui` (1a.31) : `lheld`/`rheld`, `ldown`/`rdown` comme clés construites, `le`, `hw`, et toute autre lecture ou écriture de `ui.c` sur `ui` lui-même ou sur `je`/`mo`.
- `js` (1a.16), bien qu'elle partage le module et la boucle de survol ; `jc` (1a.13) au-delà de la lecture de `gv` sur `o_` ; `pj` et `de` (1a.29).
- La réécriture de `live_ga` dans `geoharvest_burst` (1a.26).
- `RDOWN` et toute entrée d'enum qu'aucune struct ne porte encore.
- Remplacer l'écriture de `t` dans `dg` par l'écriture générique.
- La divergence `0x5f80` (voir Further Notes) : un changement de comportement n'appartient pas à ce plan.
- Types C réels (1b) ; libération des entités (TROU-7, Plan 2 §8).
- Lancer l'appli raylib : jamais.

## Further Notes

- Écart relevé en passant, **non corrigé ici** : le cart fait `poke(0x5f80, self.o_ and self.o_.fg or max(peek(0x5f80)-1,0))` ; quand `o_` existe sans `fg` (c'est le cas d'un `fq`), Lua retombe sur `max(peek-1,0)`, le port écrit 0. Rien ne relit `0x5f80` dans `src/` (GPIO) : invisible au filet. À documenter comme trou de portage par une tâche séparée, pas dans ce commit.
- `fq` est invisible : le filet ne le voit que par les clics qu'il absorbe (`game_end`, frames suivant la fin de partie) et par le sprite du curseur. D'où l'importance du scénario de survol dans la caractérisation.
- Le compte de `fq` (3 sur 2 rejeux du filet) vient surtout des fins de partie ; `score_panel_close_events.json` n'est pas dans le filet (pas de `.sha256`), ce qui fait de `score_panel_close` et de `test_pico8_fq` les seuls gardiens du chemin `fq(de)`.
- Mesures faites par grep sur `src/` et `tests/` à 0fbca60, en lisant `pico8/lua.lua:209-257` (`gk`, `ui`), `:324-329` (`fq`), `:870-903` (`pj:ph`, `de`), `:985` (`ix[3]`), et `git show 77c7f94^:src/pico8_score_panel_display.c` pour le bug d'origine.
