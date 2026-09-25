---
title: "Plan 3, étape 1a.6 : `nf` (lecteur de souris) devient une struct, et `je[bt]` se lit sur les deux formes"
description: "Le lecteur de souris `nf` est encore une instance de la couche objet : il passe en struct fille de `ke` avec sa méthode `ph` et son `pq` propre, ses tables de boutons `l`/`r` (défauts modifiés en place, partagés par tous les `nf`) deviennent l'état statique de la classe, et chaque lecteur de `je` (`je[bt]`, `je.ra`) apprend à lire une struct sans toucher le `je` provisoire."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.6 — `nf` (`mouse_reader.c:35`), clé dynamique `je[bt]` (`ui.c:60`) → `l`/`r`. Tests : aucun.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe […]. Les méthodes appelées par nom de chaîne […] passent par un enum de méthode qui indexe cette table.
>
> **Clés construites à l'exécution** : […] Tout autre site de clé dynamique trouvé est énuméré de même, ou déclenche un arrêt (§7) si l'ensemble des clés n'est pas fini.
>
> **Défauts mutés sur la classe** : […] deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** […] ; la liste des étapes les nomme.
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes […]. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode) […].

§6 : aucun test de régression n'est nommé par l'étape ; aucun ne doit changer. §7 : chaque commit de 1a passe `make class-coverage-net`, sortie identique à `tools/class_coverage_net_expected.txt` ; un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la liste des étapes dans le commit, avec « découvert à 1a.6 » dans le message.

### Ce que 1a.1 à 1a.5 ont posé (commits 9ce8bfa, 42fdf50, f5cc6d1, 28a08ac), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT` ; en-tête commun (tag, pointeur vers le descripteur `static const`).
- Descripteur (`pico8_struct.h`) : tableau de méthodes indexé par l'enum (`OL`, `PH`, `KJ`, `EV`, `LDOWN`), lecteur de champ par nom (« rend 1 si non nil »), écrivain de champ (NULL permis pour une classe qu'aucun écrivain générique n'atteint). Un nom absent de l'enum est cherché comme champ.
- `pico8_ke_t` (en-tête, `me`, `t`), `pico8_ke_struct_init`, `pico8_ke_field`, `pico8_ke_struct_pq` — contrat de `ke.h` : une fille l'appelle **une fois, après son propre `pq`**.
- `pico8_class_t.struct_new` : `pico8_class_instantiate` délègue avant toute allocation ; le constructeur enregistre la couverture.
- `pico8_entity_get_field` / `pico8_entity_set_field`, lecteur et écrivain à double forme ; pour tout ce qui n'est pas tagué `STRUCT`, le lecteur délègue à `pico8_instance_get_field` (résultat identique sur une instance, une instance sans classe ou une `ob_table`). `pico8_instance_get_field` asserte ne jamais recevoir de struct.
- `dg` écrit `t` directement dans `pico8_ke_t` ; `mv` appelle une méthode de la table ou un champ fonction.
- Modèle de test : `test_pico8_fq`, `test_pico8_mh`, `test_pico8_iv` (caractérisation figée sur l'ancien code, lecture par l'API à double forme, mutations notées dans le commit).

### Emprise réelle de `nf` (mesurée sur 58896ec)

**Cart** (`pico8/lua.lua:153-155, 200-208, 212-245`) :

```
function jl()hv,cv,cg={},{},{}
mo,je,ok=qb(),nf(),ui()end
nf=ke:jr([[ l=o(),r=o(),ra=v(0,0), ]])
function nf:pq()poke(0x5f2d,1)end
function nf:ph()self.ra=v(stat(32),stat(33))bu(self.l,1)bu(self.r,6)end
function bu(qn,oa) … qn.on,qn.jn,qn.hm= … end        -- modifie qn EN PLACE
function ui:bh(bt)local b,ni=je[bt],bt.."held" …     -- bt ∈ {"l","r"}
```

Point décisif : `l=o()` et `r=o()` sont parsés **une fois**, sur la classe. `self.l` d'une instance lit donc la table de la classe, que `bu` modifie en place. Tous les `nf` construits (un par `jl()`, 38 sur les 15 rejeux du filet, soit ~2,5 par rejeu : `_init` puis chaque `eu()`) partagent **les mêmes deux tables**. Conséquence observable : le clic qui lance une partie (`eu()` → `jl()`) laisse `l.on` vrai ; au tick suivant le nouveau `nf` lit `ia = l.on = vrai`, donc `l.jn` faux — pas de second `ldown` fantôme dans la nouvelle partie. Des tables par instance changeraient ce comportement. C'est exactement le cas « défaut dont la table pointée est modifiée en place » du §5, non nommé par la liste des étapes : **site découvert à 1a.6, couvert par une règle existante, pas un arrêt**.

`ra`, lui, est **réaffecté** (`self.ra=v(...)`) sur l'instance, jamais modifié en place : c'est un membre ordinaire, défaut `v(0,0)`.

| Aspect | Mesure |
|---|---|
| Constructions | 1 site : `pico8_jl` (`bootstrap.c:49`, `pico8_nf_new(NULL)`), rangé par `pico8_ui_set_je`. Aucune table reçue. Classe absente des registres `fz`/nommé (« Not reachable through `pico8_class_lookup` »), aucun `gx[]`/`kv[]`. |
| Défauts | `l` = table vide, `r` = table vide (modifiées en place → état statique), `ra` = `v(0,0)`. Hérités de `ke` : `me`="ph", `t`=0. |
| Champs écrits sur l'instance | `ra` (par `nf:ph`), `t` (par `dg`). Rien d'autre : aucun code n'écrit sur `je`. |
| Méthodes | `ph` (via `dg`, `mv(qc,"ph",t)`) — installée aujourd'hui par `->defaults` ; `pq` propre (`poke(0x5f2d,1)`), puis `lt` hérité de `ke`, dans cet ordre (marche dérivée → base). |
| Noms lus sur un `nf` et absents | `kj`, `ev`, `hd`, `hk`, `hb`, `nu` (`lt`/`he`) ; `p_` (`dg`) ; `fx` (scan de `mz:pq`). `nf` n'est que dans `hv`. |
| `->defaults` | 1 site : installation de `ph` dans `pico8_nf_init`. `cart.c:60` asserte `pico8_nf_class()->parent == pico8_ke_class()` ; `bootstrap.c:41` teste `pico8_nf_class() == NULL` (classe gardée : restent vrais). |
| `bm` / `nu` | aucun → `hv` seulement ; aucun `cv`, aucun `cg`. |
| Consommateurs génériques atteints | `lt` (`pico8_ke_struct_pq`), `dg` (`me`, `t`, `p_`, `mv("ph")`), `mv`, scan `fx` de `mz:pq` : **déjà à double forme**. `dc`, boucle de survol `gw`, `jz`, `economy` : jamais atteints (pas de `cv`). |
| Lecteurs de `je` (le slot `g_je`, qui tient le `je` provisoire puis un `nf`) | `ui.c` : `pico8_ui_je_button_table` (`je[bt]`, par `pico8_instance_get_field` — **assert sur une struct**) et, par elle, `pico8_ui_je_button_field_bool` (`je[bt].jn`/`.hm` dans `bh`, `je.l.jn` dans `lj`) ; `pico8_ui_field_vec(g_je,"ra")` dans `lj` et `ev` (par `pico8_instance_get_field` — **assert**) ; `pico8_ui_field_raw(g_je,"ra")` dans `gw` et `bh` (déjà à double forme). `probe.c:212` : `pico8_probe_field_vec(pico8_je(),"ra")` (`dl`, par `pico8_instance_get_field` — **assert**). `msgbox.c:238` : `pico8_msgbox_field_vec(pico8_je(),"ra")` (`iv:pq`, idem — **assert**). |
| Libération de `je` | `pico8_ui_shutdown` fait `pico8_instance_free(g_je)` : sur une struct, il libérerait l'octet du descripteur pris pour `fields` (**corruption**). Atteint par `pico8_bootstrap_shutdown` (`test_pico8_planet_kj_dispatch`, après `pico8_jl`), par `pico8_cart_shutdown`, et par `pico8_ui_init` (qui commence par `pico8_ui_shutdown`) si un `nf` est déjà en place. |
| Clé dynamique | `je[bt]` avec `bt` ∈ {`"l"`, `"r"`} : `pico8_ui_bh` n'est appelée qu'avec ces deux littéraux (`ui.c:195,198`, cart `:219-221`), `je.l` est lu en dur (`:221`, `ui.c:200`). Aucun test n'appelle `pico8_ui_bh`. **Ensemble fini de 2 clés.** |
| `pico8_ref_audit.h` | aucune macro ne reçoit un `nf`. |
| Tests qui lisent `je` / un `nf` | `test_pico8_iv.c:107` : `pico8_ob_table_set_name(pico8_je()->fields, "ra", …)` après `pico8_jl()` — **écriture hors bornes** sur une struct. `test_pico8_geoharvest_burst` : `live_ga` lit `e->cls` sans tag sur chaque entrée de `hv` (un `nf` y est toujours ; lecture dans les bornes de la struct, jamais déréférencée, jamais égale à la classe `ga` — déjà vrai pour `iv`/`mh`/`fq`, réécrit en 1a.26). `test_pico8_planet_kj_dispatch` : `pico8_bootstrap_shutdown` après `pico8_jl` (voir libération). `test_pico8_entity_smoke.c:279` : commentaire seulement. Aucun test ne lit `->cls` d'un `nf`. |
| Couverture | `nf  38  15/15` (inchangé). |

### Le `je` provisoire et `nf` : comment ils coexistent

`pico8_ui_init` construit dans `g_je` une instance sans classe, sans champ (`ui.c:108`), que `pico8_jl` remplace par un `nf` (`bootstrap.c:49`). Cette instance provisoire est l'étape 1a.7 et **reste une instance dans ce commit**. Le slot `g_je` tient donc, selon le moment, une instance (`INSTANCE`, aucun champ : `je[bt]` absent, `je.ra` lu comme vecteur nul) ou une struct `nf`. Chaque lecteur de `je` passe par le lecteur à double forme, qui rend exactement le résultat actuel sur l'instance provisoire ; `pico8_ui_shutdown` ne libère `g_je` que s'il est tagué `INSTANCE`. Le type du slot et de `pico8_je()` ne change pas (`pico8_instance_t *`, comme `hv` qui tient déjà des structs) ; son contrat d'en-tête dit qu'il peut être un `nf` struct, à lire seulement par `pico8_entity_get_field`.

## Solution

1. **Struct `nf`** : premier membre `pico8_ke_t`, puis `ra` (`pico8_value_t`). Rien d'autre : `l`/`r` ne sont pas des membres. Déclarée publique dans le module du lecteur de souris.
2. **État statique de la classe** : deux `pico8_value_t` statiques `l`, `r` (REF vers une table de boutons), créés vides par `pico8_nf_init`, partagés par tous les `nf`, remis à neuf à chaque `pico8_nf_init` (comme le faisait la reconstruction de la classe), libérés par `pico8_nf_shutdown` (même propriétaire, même moment qu'avant : les défauts de la classe).
3. **Constructeur** (posé comme `struct_new` de la `pico8_class_t` de `nf`), dans l'ordre :
   1. alloue ;
   2. `pico8_ke_struct_init` avec le descripteur de `nf` ;
   3. `ra` = le `v(0,0)` du défaut parsé (même REF qu'aujourd'hui) ;
   4. table reçue : NULL ou vide seulement ; toute clé → assertion porteuse de message (arrêt §7) ;
   5. enregistre `"nf"` au crochet de couverture ;
   6. `pq` propre de `nf` (`poke(0x5f2d,1)` sur la mémoire de dispatch, no-op si NULL) ;
   7. `pico8_ke_struct_pq`.
4. **Descripteur de `nf`** : méthode `PH` seule ; lecteur de champ qui connaît `ra` (membre), `l` et `r` (état statique), puis délègue à `pico8_ke_field` ; écrivain NULL (aucun écrivain générique n'atteint `nf`).
5. **`nf:ph` sur la struct** : `self.ra = v(stat(32),stat(33))` (nouveau vecteur sur le tas, TROU-7), puis `bu` sur la table statique `l` (masque 1) puis `r` (masque 6). Garde NULL conservée.
6. **Clé dynamique `je[bt]`** : `pico8_ui_je_button_table` lit `bt` sur `g_je` par `pico8_entity_get_field` et asserte d'abord que `bt` est `"l"` ou `"r"` (ensemble énuméré, §5). Sur un `nf`, le lecteur rend la table statique ; sur le `je` provisoire, rien, comme aujourd'hui.
7. **Lecteurs de `je.ra`** : `pico8_ui_field_vec`, `pico8_probe_field_vec`, `pico8_msgbox_field_vec` passent de `pico8_instance_get_field` à `pico8_entity_get_field` — identique sur tout ce qui n'est pas une struct (`mo`, `ok`, planètes, `ob_table`).
8. **`pico8_ui_shutdown`** : libère `g_je` par `pico8_instance_free` seulement s'il est tagué `INSTANCE` ; un `nf` struct est oublié, jamais libéré (TROU-7). `g_ok` inchangé.
9. **`pico8_jl`** : range le `nf` construit dans `g_je` (cast vers le type du slot, comme `pico8_ke_struct_pq` le fait pour `hv`). `real_mo`, `real_ok` inchangés.
10. **`pico8_class_t` de `nf`** : même déclaration, même spec, même place, même nom de couverture ; reçoit le constructeur ; perd l'installation `->defaults` de `ph` et son `pico8_class_set_pq` (le constructeur fait le `pq`).
11. **`test_pico8_iv`** : la ligne 107 écrit `ra` dans le membre de la struct `nf` (tag vérifié), au lieu de `->fields` ; aucune valeur figée ne change.
12. **Plan** : la ligne 1a.6 de `docs/plan-3.md` gagne « `l`/`r` défauts modifiés en place → état statique, découvert à 1a.6 », et le §5 « Défauts mutés » ajoute `nf.l`, `nf.r` aux connus.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `nf` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise de `nf` (construction, défauts, champs écrits, méthodes, `->defaults`, consommateurs génériques, lecteurs de `je`, tests) soit mesurée dans le PRD, afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux que la struct `nf` n'ait que `pico8_ke_t` et `ra`, nom minifié conservé, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que gardien du filet, je veux que `l` et `r` soient l'état statique partagé de la classe, afin que tous les `nf` successifs lisent et modifient les deux mêmes tables, comme dans le cart.
5. En tant que gardien du filet, je veux qu'un bouton tenu pendant un `jl()` ne produise pas de `jn` sur le nouveau `nf`, afin que le clic qui lance une partie ne soit pas rejoué dans la nouvelle partie.
6. En tant que gardien du filet, je veux que `pico8_nf_init` remette `l`/`r` à neuf, afin que la reconstruction de la classe par `pico8_cart_load` garde son effet d'aujourd'hui.
7. En tant que mainteneur, je veux que `l`/`r` ne soient jamais des membres d'instance, afin d'appliquer la règle des défauts modifiés en place et pas d'en inventer une.
8. En tant que gardien du filet, je veux que `ra` d'un `nf` neuf vaille le `v(0,0)` du défaut, afin que `je.ra` lu avant le premier `ph` (`iv:pq`, `ui:lj`) soit identique.
9. En tant que gardien du filet, je veux que `nf:ph` réaffecte `ra` à un vecteur neuf `(stat(32),stat(33))`, afin que le curseur, `qa` et les toasts suivent la souris comme avant.
10. En tant que gardien du filet, je veux que `ph` applique `bu` à `l` avec le masque 1 puis à `r` avec le masque 6, afin que `on`/`jn`/`hm` des deux boutons soient identiques.
11. En tant que gardien du filet, je veux que le constructeur fasse `poke(0x5f2d,1)` puis `lt`, dans cet ordre, afin de reproduire la marche des `pq` dérivée → base.
12. En tant que gardien du filet, je veux que `lt` enregistre un `nf` dans `hv` seulement, afin que ni `cv` ni `cg` ne changent.
13. En tant que gardien du filet, je veux que `dg` trouve `ph` par la table du descripteur, afin que la souris soit lue au même tick.
14. En tant que gardien du filet, je veux que `dg` incrémente `t` dans `pico8_ke_t` et ne retire jamais un `nf` (`p_` absent), afin que `je` vive jusqu'au `jl()` suivant.
15. En tant qu'auteur des étapes suivantes, je veux que l'enum ne gagne rien (`PH` existe), afin que chaque ajout reste attaché à une étape.
16. En tant que mainteneur, je veux qu'une clé reçue par le constructeur déclenche une assertion porteuse de message, afin qu'un champ jamais mesuré ne soit pas perdu en silence.
17. En tant que mainteneur, je veux que `je[bt]` asserte `bt` ∈ {`l`,`r`}, afin que l'ensemble de clés énuméré par ce PRD soit vérifié à l'exécution.
18. En tant que gardien du filet, je veux que `je[bt]` rende la table statique sur un `nf` et rien sur le `je` provisoire, afin que `bh` et `lj` se comportent comme avant, avant et après `jl()`.
19. En tant que gardien du filet, je veux que les trois lecteurs de `je.ra` (`ui`, `probe`, `msgbox`) passent par le lecteur à double forme, afin qu'un `nf` struct ne fasse pas partir l'assert de la couche.
20. En tant que mainteneur, je veux que ces trois lecteurs rendent un résultat identique sur `mo`, `ok`, les planètes et le `je` provisoire, afin que le changement ne déplace rien de ce qui n'est pas converti.
21. En tant que gardien de la mémoire, je veux que `pico8_ui_shutdown` ne passe jamais un `nf` struct à `pico8_instance_free`, afin qu'aucun arrêt ni `pico8_ui_init` après `jl()` ne libère un pointeur qui n'est pas un `fields`.
22. En tant que gardien de la mémoire, je veux que le `je` provisoire reste libéré par `pico8_ui_shutdown` comme aujourd'hui, afin de ne rien changer à ce que 1a.7 convertira.
23. En tant que mainteneur de la mémoire, je veux qu'un `nf` struct ne soit jamais libéré, afin que TROU-7 reste la seule convention de fuite.
24. En tant qu'auteur de 1a.7, je veux que le `je` provisoire reste une instance sans classe construite par `pico8_ui_init`, afin que sa conversion reste une étape à part.
25. En tant que mainteneur, je veux que le contrat de `pico8_je()` dise qu'il peut rendre un `nf` struct, à lire par `pico8_entity_get_field`, afin qu'aucun nouveau lecteur ne réintroduise `pico8_instance_get_field` sur `je`.
26. En tant que mainteneur, je veux que la `pico8_class_t` de `nf` reste déclarée à la même place avec son nom de couverture et son parent, afin que `KNOWN[]`, l'assert de `cart.c` et le test de `bootstrap.c` tiennent.
27. En tant que gardien de la couverture, je veux que le constructeur enregistre `"nf"` une fois, jamais `"ke"`, afin que `nf  38  15/15` et `ke  0  0/15` restent identiques.
28. En tant que mainteneur, je veux que l'installation `->defaults` de `ph` disparaisse, remplacée par la table, afin d'appliquer la règle des accès directs aux défauts.
29. En tant que gardien des tests, je veux que `test_pico8_iv` place la souris dans le membre `ra` du `nf` sans changer une valeur figée, afin que la caractérisation de 1a.2 garde son objet.
30. En tant que gardien des tests de régression, je veux que tous les tests de régression passent avec un diff vide (dont `planet_kj_dispatch`, `pause_menu_music`, `geoharvest_burst`, `game_end`), afin que la conversion ne se valide pas en affaiblissant un test.
31. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
32. En tant que lecteur du plan, je veux que la découverte `nf.l`/`nf.r` soit inscrite à la ligne 1a.6 et dans les connus du §5, afin que la liste des étapes nomme ses défauts modifiés en place, comme le §5 l'exige.
33. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés, afin de ne pas laisser de contrat mensonger.

## Implementation Decisions

Pas d'outil d'agent dans cette session pour lancer un agent `guideline` frais : j'ai chargé et appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **`l`/`r` en état statique, pas en membres.** La ligne de l'étape dit « `je[bt]` → `l`/`r` » ; la règle « défaut dont la table pointée est modifiée en place » du §5 impose que ces deux noms soient servis par l'état statique de la classe. Les deux lectures sont compatibles : la clé dynamique est énumérée en deux noms fixes, et ces deux noms se résolvent sur l'état partagé. Membres par instance rejetés : changement de comportement au `jl()` (User Story 5).
- **(G) Propriété des tables statiques** : créées vides par `pico8_nf_init`, libérées par `pico8_nf_shutdown` — le cycle de vie exact des défauts de la classe aujourd'hui. Les tables `l`/`r` de la spec restent dans la `pico8_class_t` retenue, jamais lues ni modifiées (copie en lecture ; `nf` n'a pas de descendant). Rejeté : aliaser les tables des défauts (la classe retenue garderait un état mutable, contraire au « copie en lecture » du §5).
- **Tables de boutons** : restent des `ob_table` (`pico8_bu` inchangé, `on`/`jn`/`hm` lus comme aujourd'hui). Leur remplacement est l'affaire de la phase D.
- **`ra`** : membre, initialisé à la REF du `v(0,0)` parsé dans la spec retenue (même pointeur que l'ancienne lecture de défaut ; rien ne le modifie en place).
- **Enum** : inchangé (`PH` existe déjà). Le `pq` n'est pas une entrée d'enum : il n'est appelé que par le constructeur, comme pour `iv`/`mh`.
- **(G) Écrivain NULL** : aucun code de `src/` n'écrit un champ sur `je` par un chemin générique (mesuré). L'écriture de `test_pico8_iv` passe par le membre de la struct publique, pas par un écrivain ajouté pour un test.
- **Clé dynamique `je[bt]`** : le seul site est `pico8_ui_je_button_table` ; l'assert `bt` ∈ {`l`,`r`} y est posé, avec un message qui cite le §5. Ensemble fini de 2 clés : pas d'arrêt. `lheld`/`rheld`, `ldown`/`rdown` (clés construites sur `ui`) restent à 1a.31.
- **Lecteurs de `je.ra`** : changement d'appel seulement (`pico8_instance_get_field` → `pico8_entity_get_field`) dans `pico8_ui_field_vec`, `pico8_probe_field_vec`, `pico8_msgbox_field_vec`. Ce sont des helpers partagés avec d'autres receveurs (non-structs) : résultat identique pour eux par délégation. Rien d'autre dans `probe.c`/`msgbox.c`.
- **`ui.c`, strictement** : `pico8_ui_je_button_table` (lecteur + assert), `pico8_ui_field_vec` (lecteur), `pico8_ui_shutdown` (libération conditionnelle de `g_je`), contrat d'en-tête de `pico8_je()`/`pico8_ui_set_je()`/`pico8_ui_shutdown()` dans `ui.h`. Le `je` provisoire construit par `pico8_ui_init`, `ok`, `ui` : inchangés.
- **`bootstrap.c`** : seul le type de la variable du `nf` construit et le cast vers le slot changent ; en-tête `bootstrap.h` (« Frees the mo/je/ok instances ») corrigé si le changement le rend faux.
- **Constructeur** : ordre de la Solution §3. `pico8_nf_new` rend la struct ; `pico8_nf_ph` prend la struct ; `pico8_nf_pq` (public, non appelé ailleurs) prend la struct ou disparaît au profit du constructeur — l'implémenteur garde la forme qui ne laisse pas de code mort.
- **`pico8_class_t` de `nf`** : garde sa spec, son parent, son nom de couverture ; reçoit `struct_new` ; perd `->defaults` de `ph` et le `set_pq`. Grep attendu : plus aucun `g_nf_class->defaults`.
- **Identification pour les tests** : accesseur public du descripteur `static const` de `nf` (comme `pico8_fq_struct_class()`).
- **(G) `live_ga` de `geoharvest_burst` laissé tel quel** : il voit désormais un `nf` struct à chaque tick ; lecture dans les bornes (le champ `cls` d'une instance tombe sur `me` du `pico8_ke_t`), jamais égale à la classe `ga`, jamais déréférencée. Sa réécriture appartient à 1a.26.
- **(G) Commentaire de `test_pico8_entity_smoke.c:279`** (« la forme de `js`/`nf`/`qb` ») : laissé, il décrit une marche de classe que le test factice reproduit toujours ; ce fichier est supprimé en 1a.42.
- **Commentaires** (CLAUDE.md) : contrat de la struct (`nf` jamais libéré, TROU-7 ; `l`/`r` partagés, remis à neuf par `pico8_nf_init`), contrat de `pico8_je()`, asserts porteurs (clé reçue, `bt`). Corriger ceux que le changement rend faux : en-tête de `pico8_nf_init` (« `nf` carries its own `pq` on top of the inherited `lt` » — reste vrai du comportement, pas du mécanisme), de `pico8_nf_ph` (« `self.l`/`self.r` mutated in place » → état de la classe), de `pico8_ui_shutdown`/`pico8_bootstrap_shutdown` (« frees je »). Tout le reste va dans le message de commit (dont « `nf.l`/`nf.r` : découvert à 1a.6 »).

## Implementation Chunks

**Un seul commit** (§7 : une classe par commit). Chaque morceau est un point d'arrêt TDD ; chacun laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation de `nf` sur l'ancien code.** Nouveau test (voir Testing), valeurs et empreintes figées **avant** toute modification de `mouse_reader.c`, `ui.c`, `probe.c`, `msgbox.c`, `bootstrap.c`.
2. **Coexistence, sans classe convertie.** Les trois lecteurs de `je.ra` et `je[bt]` passent par `pico8_entity_get_field` (+ assert `bt`) ; `pico8_ui_shutdown` ne libère `g_je` que tagué `INSTANCE`. Aucune classe ne change de forme : filet et couverture trivialement identiques.
3. **`nf` convertie.** Struct, état statique, descripteur, constructeur, `ph` sur la struct, classe reliée, `->defaults` et `set_pq` retirés, accesseur du descripteur ; `bootstrap.c` ; `test_pico8_iv:107` ; ligne 1a.6 et §5 du plan. Le test du morceau 1 passe **sans changer ses valeurs figées**.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rend `pico8_entity_get_field` sur `pico8_je()` (`ra`, `l`, `r`, puis `on`/`jn`/`hm` des tables), appartenance à `hv`/`cv`/`cg`, `peek(0x5f2d)`, ce que rend `mv`, pixels, compte de couverture. Jamais l'agencement mémoire au-delà de « en-tête à l'offset 0 ».
- **Nouveau test `test_pico8_nf`** (dans `make unit`, lié comme `test_pico8_fq`, sur le cart réel chargé). Il lit l'état **uniquement** par l'API à double forme (`pico8_entity_get_field`, `pico8_mv`, `pico8_hv`/`pico8_cv`, `pico8_je`, `pico8_dg`) et pilote la souris par un `pico8_input_t` construit en mémoire ou chargé, pour être le même code avant et après.
  - **Caractérisation, figée sur l'ancien code** :
    1. `pico8_jl()` avec la mémoire de dispatch à zéro : `peek(0x5f2d)` vaut 1 ; `je` dans `hv`, dans aucun `cv` ni `cg` ; `je.ra` = (0,0) avant tout tick ; `je.l`, `je.r` présents, sans `on`/`jn`/`hm`.
    2. Séquence scriptée de ticks `dg` (position et boutons variés : gauche seul, droit, milieu, gauche+droit, relâchers) : par tick, `ra` et `on`/`jn`/`hm` de `l` et de `r`, repliés en une empreinte + nombre de ticks, figés.
    3. **Partage entre `nf`** : bouton gauche tenu, tick ; `pico8_jl()` (nouveau `nf`) ; tick bouton toujours tenu → `je.l.jn` faux, `je.l.on` vrai ; la REF de `je.l` est la même avant et après `jl()`.
    4. **Remise à neuf** : après `pico8_nf_init()` puis `pico8_jl()`, `je.l` n'a plus de `on`.
    5. **Le `je` provisoire** : après `pico8_ui_init()` seul (avant `jl`), `pico8_entity_get_field(pico8_je(),"l")` et `"ra"` rendent 0 ; un `pico8_ui_lj` sur un `ok` construit ne dispatche rien (empreinte de `ok` ou absence de `lheld`).
    6. **Rejeu depuis le titre** d'un enregistrement de `RECORDINGS` qui est aussi dans `FRAMES_EVENTS` (il traverse le clic de lancement de partie, donc un `jl()` bouton tenu), piloté comme les tests de scénario (`--from-title`) : par frame, `je.ra`, `on`/`jn`/`hm` de `l` et `r`, empreinte d'écran ; repliés en une empreinte de séquence + nombre de frames, figés (on peut se limiter aux N premières frames si elles contiennent le lancement).
  - **Forme** (ajouté au morceau 3) : `pico8_nf_new(NULL)` et `pico8_class_instantiate(pico8_nf_class(), NULL)` rendent un objet tagué `STRUCT` avec le descripteur de `nf`, présent dans `hv`, absent de tout `cv` ; `mv(je,"ph")` appelle la méthode ; `mv(je,"kj")`, `mv(je,"p_")` rendent 0 ; deux `nf` successifs rendent la même REF pour `l` et pour `r`.
  - **Mutations** : l'implémenteur remet localement au moins 6 erreurs plausibles et note dans le commit celles qui sont attrapées : `l`/`r` en membres par instance (tables neuves à chaque construction) ; `l` et `r` confondus (même table) ; masques 1 et 6 inversés ; `ra` non réaffecté par `ph` ; `poke(0x5f2d,1)` omis ; `pico8_ke_struct_pq` omis (le `nf` sort de `hv`) ; `PH` absent de la table ; état statique non remis à neuf par `pico8_nf_init`. **Mutant équivalent à justifier dans le commit** : `poke` après `lt` au lieu d'avant — rien entre les deux n'observe l'autre (`lt` ne lit pas `0x5f2d`, `poke` ne lit pas `hv`) ; ce n'est pas une faiblesse du test.
- **`test_pico8_iv`** : seule la mise en place de la souris change (membre `ra` du `nf` au lieu de `->fields`, include du module) ; ses valeurs figées sont identiques avant et après.
- **Tests de régression** : aucun n'est nommé par l'étape ; tous passent avec un diff vide. `planet_kj_dispatch` exerce `pico8_bootstrap_shutdown` après `jl()` (libération conditionnelle), `pause_menu_music` le `je.l.jn` de `lj`, tous les rejeux de scénario `je[bt]` et `je.ra`.
- **Couverture** : `make class-coverage-net` code 0, sortie identique à la référence (`nf  38  15/15`, `ke  0  0/15  exemptée …`).
- **Prior art** : `test_pico8_fq` (caractérisation figée, rejeu piloté, forme ajoutée après conversion) ; `test_pico8_mh`, `test_pico8_iv` ; `test_pico8_replay_from_title` (rejeu depuis le titre).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `nf` n'est pas `nf  38  15/15`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé de `nf` ne tombe sous aucune règle du §5 — notamment si l'assertion des clés reçues, celle de `bt` ∉ {`l`,`r`}, ou celle de l'écrivain générique se déclenche sur un rejeu ou un test, ou si un autre nom de clé construite sur `je` est trouvé sans ensemble fini : arrêt, pas de membre improvisé.
- `FAILED:` si `l` ou `r` devient un membre de la struct `nf` (règle des défauts modifiés en place violée).
- `FAILED:` si un test de régression de bug (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe que `nf` est convertie — en particulier si le `je` provisoire (1a.7), `mo`/`qb` (1a.8, 1a.9) ou `ui` (1a.31) change de forme, si `lheld`/`rheld`/`ldown`/`rdown` changent de traitement, ou si un `->defaults` d'une autre classe bouge.
- `FAILED:` si la `pico8_class_t` de `nf` n'est plus déclarée à la même place avec son parent et son nom de couverture, ou si `ke`/`jb`/`iv`/`mh`/`fq` changent de forme.
- `FAILED:` si les valeurs figées du test de caractérisation de `nf` ou de `test_pico8_iv` ont été changées après la conversion.

Critères de réussite (en plus) :

- grep : plus aucun `g_nf_class->defaults` ; plus aucun `pico8_instance_get_field` sur `g_je`/`pico8_je()` dans `src/` ; plus aucun `->fields` sur `pico8_je()` dans `tests/` ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN` ;
- la struct `nf` n'a que `pico8_ke_t` et `ra` ; son descripteur n'a que `PH` et un écrivain NULL ;
- dans `ui.c`, le diff se limite à `pico8_ui_je_button_table`, `pico8_ui_field_vec` et `pico8_ui_shutdown` ; dans `probe.c`/`msgbox.c`, à l'appel du lecteur de leur helper `field_vec` ; dans `bootstrap.c`, au type du `nf` construit ;
- la ligne 1a.6 de `docs/plan-3.md` et les connus du §5 nomment `nf.l`/`nf.r`, et le message de commit dit « découvert à 1a.6 ».

## Out of Scope

- Le `je` provisoire sans classe (1a.7) : sa construction dans `pico8_ui_init`, sa forme, son remplacement.
- `qb`/`mo` (1a.8, 1a.9), bien que construits dans le même `jl()` et lus par les mêmes helpers (`mo.ra`).
- `ui` (1a.31) : `lheld`/`rheld`, `ldown`/`rdown`, `le`, `hw`, `qa`, et toute écriture de `ui.c` sur `ui` lui-même.
- Le remplacement des tables de boutons `ob_table` par des structs (phase D) et le typage C de `ra` (phase 1b).
- La réécriture de `live_ga` dans `geoharvest_burst` (1a.26) et de `test_pico8_entity_smoke` (1a.42).
- Libérer les `nf` remplacés par `jl()` (TROU-7, Plan 2 §8).
- Lancer l'appli raylib : jamais.

## Further Notes

- Le partage de `l`/`r` entre `nf` successifs est le point le plus fragile de l'étape et le filet ne le garde qu'indirectement (un `ldown` fantôme au premier tick d'une partie ne changerait l'écran que s'il tombait sur un bouton). D'où les scénarios 3, 4 et 6 de la caractérisation.
- `pico8_ui_init` commence par `pico8_ui_shutdown` : sans la libération conditionnelle, tout `pico8_cart_load` après un `jl()` dans le même processus corromprait le tas. Aucun binaire actuel ne le fait (un processus par rejeu), mais `test_pico8_planet_kj_dispatch` atteint le même chemin par `pico8_bootstrap_shutdown`.
- Mesures faites par grep sur `src/` et `tests/` à 58896ec, en lisant `pico8/lua.lua:153-155` (`jl`), `:200-208` (`nf`, `bu`), `:212-245` (`ui`), `:561` (`dl`), `:945` (`iv:pq`), et `pico8_class_instantiate`/`pico8_instance_get_field` (`class.c`) pour la résolution des défauts sur la classe.
