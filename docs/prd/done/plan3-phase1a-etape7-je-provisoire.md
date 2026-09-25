---
title: "Plan 3, étape 1a.7 : le `je` provisoire, instance sans classe, devient une struct sans parent et sans champ"
description: "Entre `pico8_ui_init` et le premier `jl()`, le slot `je` tient encore une instance de la couche objet construite à la main (tag `INSTANCE`, aucun champ, aucune classe) : elle devient une struct sans parent, à descripteur sans méthode et à lecteur qui ne trouve jamais rien, en stockage statique. La libération « seulement si tagué `INSTANCE` » de `pico8_ui_shutdown` perd alors son objet et disparaît."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.7 — `je` : sans classe (`ui.c:108`), provisoire, remplacée par un `nf` (`bootstrap.c:49`).

Règles du §5 qui s'appliquent à ce commit :

> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe.

§6 : aucun test de régression n'est nommé par l'étape ; aucun ne doit changer. §7 : le commit passe `make class-coverage-net`, sortie identique à `tools/class_coverage_net_expected.txt` ; un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la liste des étapes dans le commit, avec « découvert à 1a.7 » dans le message.

### Ce que 1a.1 à 1a.6 ont posé (commits 9ce8bfa, 42fdf50, f5cc6d1, 28a08ac, 824725c), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT` ; en-tête commun `pico8_struct_header_t` (tag, pointeur vers le descripteur `static const`) ; descripteur `pico8_struct_class_t` (méthodes indexées par l'enum `OL`/`PH`/`KJ`/`EV`/`LDOWN`, lecteur de champ « rend 1 si non nil », écrivain NULL permis).
- `pico8_entity_get_field` : sur une struct, méthode de la table puis lecteur du descripteur ; **asserte** que le lecteur n'est pas NULL (« a struct a generic consumer reaches needs a field reader »). Sur tout le reste, délègue à `pico8_instance_get_field`.
- Racine sans parent déjà convertie : `jb` (en-tête puis ses membres, pas de `pico8_ke_t`).
- 1a.6 : le slot `g_je` (type `pico8_instance_t *`, inchangé) tient le `je` provisoire puis un `nf` struct ; **chaque lecteur de `je` passe déjà par le lecteur à double forme** (`pico8_ui_je_button_table` pour `je[bt]` avec assert `bt` ∈ {`l`,`r`} ; `pico8_ui_field_vec`/`pico8_ui_field_raw` dans `ui.c`, `pico8_probe_field_vec` dans `probe.c:212`, `pico8_msgbox_field_vec` dans `msgbox.c:238`, pour `je.ra`). `pico8_ui_shutdown` ne libère `g_je` que tagué `INSTANCE` : un `nf` struct est oublié (TROU-7). `test_pico8_nf` fige, entre autres, `test_placeholder_je` (après `pico8_jl()` puis `pico8_ui_init()` : `je.l`, `je.r`, `je.ra` absents ; `pico8_ui_lj` sur un `ok` neuf ne pose ni `lheld` ni `rheld`, pose `qa`).

### Ce que fait le cart

`pico8/lua.lua:153-154` : `function jl()hv,cv,cg={},{},{} mo,je,ok=qb(),nf(),ui()end`, et `:1178` : `function _init()cu()jl()…`. **Dans le cart, `je` n'existe pas avant `jl()`** (global nil) et aucun code ne le lit avant : `_init` appelle `jl()` avant toute frame, `cu()` ne lit pas `je`. Le `je` provisoire est un artefact du portage : il rend sûres les lectures `je.ra`/`je[bt]` sur un chemin qui les atteindrait entre `pico8_ui_init` et `pico8_jl` (tests, et `pico8_jl` lui-même quand il construit `ui` faute de `pico8_cart_load`). Même raison que le `mo` provisoire (`entity.c:129`, 1a.9).

### Emprise réelle du `je` provisoire (mesurée sur 824725c)

| Aspect | Mesure |
|---|---|
| Construction | 1 site : `pico8_ui_init` (`ui.c:111-114`) : `malloc`, tag `INSTANCE`, `fields` = table vide, `cls` = NULL. Pas de `pico8_class_t`, pas de `pico8_class_instantiate`, pas de `lt` : **jamais dans `hv`/`cv`/`cg`**, jamais atteint par `dg`, `dc`, `gw`, `jz`. |
| Champs | aucun. Aucun code n'écrit sur `je` (mesuré en 1a.6 ; l'unique écriture de test, `test_pico8_iv.c:109`, vise le `nf`, après `jl()`). |
| Méthodes | aucune (`cls` NULL). `mv` n'est jamais appelé sur `je`. |
| `->defaults` | aucun. |
| Noms lus sur lui | `ra` (`ui:lj`, `ui:ev`, `ui:gw`, `ui:bh`, `dl` de `probe.c`, `iv:pq` de `msgbox.c`), `l` et `r` (`je[bt]` dans `bh`, `je.l.jn` dans `lj`). Tous absents : `ra` lu comme vecteur nul ou nil, `je[bt]` rend NULL. |
| Vie | de `pico8_ui_init` au `pico8_ui_set_je` de `pico8_jl`. Dans les binaires (`main.c`, `main_raylib.c`, `main_class_coverage.c`, avec ou sans `--from-title`) : `pico8_cart_load` puis `pico8__init` → `pico8_cu` → `pico8_jl`, **avant la première frame** : aucun rejeu, aucune frame du filet ne le lit. Atteint seulement par les tests : entre `pico8_cart_load` et `pico8__init`, `test_placeholder_je` de `test_pico8_nf`, et `pico8_jl` sans `pico8_cart_load` (`test_pico8_planet_kj_dispatch`, `test_pico8_nf`) où il est construit puis aussitôt remplacé. |
| Libération | `pico8_ui_shutdown` : `pico8_instance_free(g_je)` si tagué `INSTANCE` (donc seulement lui) ; atteint par `pico8_cart_shutdown`, `pico8_bootstrap_shutdown`, et `pico8_ui_init` (qui commence par `pico8_ui_shutdown`). |
| Lecteur NULL | `pico8_ui_je_button_table` appelle `pico8_entity_get_field(g_je, …)` **sans garde NULL** ; sur NULL, `pico8_instance_get_field` déréférence `inst->fields` (crash). Les helpers `field_vec`/`field_raw` gardent NULL. |
| Couverture | **aucun nom** : construit à la main, jamais par `pico8_class_instantiate`, donc jamais compté. Absent de `KNOWN[]` et de `tools/class_coverage_net_expected.txt`. |
| Tests qui le lisent | `test_pico8_nf` (`test_placeholder_je`, valeurs figées en 1a.6). Aucun test ne lit `->fields`/`->cls` de `je` avant `jl()`. |

## Solution

1. **Struct du `je` provisoire, sans parent** : un seul membre, l'en-tête `pico8_struct_header_t`. Pas de `pico8_ke_t` (§5 « sans parent » ; il n'est pas une entité de `hv`), pas de `ra`/`l`/`r` : l'instance n'en avait pas, et chaque lecteur a déjà un défaut pour leur absence.
2. **Descripteur `static const`** : aucune méthode ; lecteur de champ qui rend 0 (absent) pour tout nom, exactement comme une instance sans classe à table vide ; écrivain NULL (aucun écrivain n'atteint `je`).
3. **Stockage statique** : un unique objet de ce type, défini dans `ui.c`, initialisé à la compilation (tag `STRUCT`, descripteur). `pico8_ui_init` range son adresse dans `g_je` (cast vers le type du slot, comme pour le `nf`) au lieu du `malloc`. Aucune allocation, aucune libération.
4. **`pico8_ui_shutdown`** : la branche « libère `g_je` s'il est tagué `INSTANCE` » ne peut plus être prise (plus aucun `je` n'est une instance) : elle est supprimée ; `g_je` est seulement remis à NULL. `ok` et la classe `ui` inchangés.
5. **Identification pour les tests** : accesseur public du descripteur (comme `pico8_fq_struct_class()`), déclaré dans `ui.h`.
6. **Contrats d'en-tête corrigés** (seuls ceux que ce changement rend faux) : `pico8_je()` (« class-less instance with no fields » → struct provisoire sans champ), `pico8_ui_shutdown()` (plus aucun `je` libéré), `pico8_ui_init()` (« field-less `je` » : reste vrai, préciser struct si la phrase le rend faux), `pico8_bootstrap_shutdown()` dans `bootstrap.h` (« the placeholder `je` » n'est plus libéré : `je` ne l'est jamais).
7. **Rien d'autre** : `pico8_jl`, les lecteurs de `je`, `nf`, `mo` (1a.9), `ui` (1a.31), `KNOWN[]`, le plan (aucun site découvert attendu).

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que le `je` provisoire soit la seule chose convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise du `je` provisoire (construction, champs, méthodes, lecteurs, vie, libération, couverture, tests) soit mesurée dans le PRD, afin que chaque site touché soit vérifiable.
3. En tant que lecteur du plan, je veux que le `je` provisoire devienne une struct sans parent (en-tête seul), afin d'appliquer « converties comme des classes à part entière, sans parent » à la lettre.
4. En tant que mainteneur, je veux que la struct n'ait ni `ra`, ni `l`, ni `r`, afin de ne pas inventer des champs que l'instance n'a jamais eus.
5. En tant que gardien du filet, je veux que toute lecture de champ sur le `je` provisoire rende « absent », afin que `je.ra` se lise comme vecteur nul et `je[bt]` comme NULL, comme aujourd'hui.
6. En tant que gardien du filet, je veux que le `je` provisoire n'ait aucune méthode, afin que `mv(je, …)` rende 0 comme sur une instance sans classe.
7. En tant que gardien du filet, je veux que le `je` provisoire ne soit jamais enregistré dans `hv`/`cv`/`cg`, afin que `dg`, `dc` et la boucle de survol ne le voient toujours pas.
8. En tant que gardien de la mémoire, je veux que le `je` provisoire ne soit ni alloué ni libéré, afin qu'aucun chemin d'arrêt ni `pico8_ui_init` répété ne passe un pointeur statique à `free`.
9. En tant que mainteneur, je veux que la libération conditionnelle de `pico8_ui_shutdown` disparaisse, afin de ne pas laisser une branche morte que ce changement a créée.
10. En tant que mainteneur, je veux que `pico8_ui_shutdown` remette toujours `g_je` à NULL, afin que `pico8_je()` rende NULL après un arrêt comme aujourd'hui.
11. En tant que gardien des tests, je veux que `pico8_ui_init` range de nouveau le `je` provisoire après chaque arrêt, afin que `pico8_cart_load` → `pico8_cart_shutdown` → `pico8_cart_load` dans un même processus lise le même `je`.
12. En tant que gardien des tests, je veux que `pico8_ui_init` appelé après `pico8_jl()` remette le `je` provisoire à la place du `nf`, afin que `test_placeholder_je` de `test_pico8_nf` passe sans changer une valeur figée.
13. En tant que gardien du filet, je veux que le slot `g_je` et `pico8_je()` gardent leur type, afin que `pico8_jl`, `pico8_ui_set_je` et les lecteurs ne soient pas touchés.
14. En tant que gardien de la couverture, je veux que le `je` provisoire ne soit jamais compté, afin que `KNOWN[]`, la référence et la sortie de `make class-coverage-net` restent identiques.
15. En tant que gardien de la couverture, je veux qu'aucun nom (`je` ou autre) ne soit ajouté à `KNOWN[]`, afin de ne pas changer la sortie attendue, ce que le §7 interdit.
16. En tant que mainteneur, je veux que le descripteur soit identifiable par un accesseur public, afin qu'un test prouve la forme sans lire l'agencement mémoire.
17. En tant que gardien des tests de régression, je veux que tous passent avec un diff vide, afin que la conversion ne se valide pas en affaiblissant un test.
18. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
19. En tant qu'auteur de 1a.9, je veux que le `mo` provisoire reste une instance, afin que sa conversion reste une étape à part.
20. En tant qu'auteur de 1a.31, je veux que `ui`, `ok`, `lheld`/`rheld`, `ldown`/`rdown` ne changent pas, afin que leur conversion reste une étape à part.
21. En tant que mainteneur, je veux que l'enum de méthodes reste `OL`, `PH`, `KJ`, `EV`, `LDOWN`, afin que chaque ajout reste attaché à une étape.
22. En tant que lecteur du code, je veux que les contrats d'en-tête de `pico8_je()`, `pico8_ui_shutdown()` et `pico8_bootstrap_shutdown()` disent la vérité après ce commit, afin de ne pas laisser un contrat mensonger (CLAUDE.md).
23. En tant que lecteur du code, je veux qu'aucun commentaire de récit de chantier ne soit ajouté, afin de respecter la règle des quatre destinations.
24. En tant que vérificateur, je veux une caractérisation figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable.

## Implementation Decisions

Pas d'outil d'agent dans cette session pour lancer un agent `guideline` frais : j'ai chargé et appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Forme retenue : en-tête seul, sans parent, sans champ.** Les lecteurs du slot attendent des champs à la `nf` (`ra`, `l`, `r`), mais tous **tolèrent leur absence** (vecteur nul, nil, NULL) et c'est ce que l'instance leur donne aujourd'hui. Une struct sans champ dont le lecteur ne trouve rien est l'équivalent exact.
  - Rejeté : une struct « à la `nf` » avec `ra`/`l`/`r` à nil — des membres que l'instance n'a jamais eus, sans règle du §5 qui les demande.
  - Rejeté : un vrai `nf` construit dans `pico8_ui_init` — il s'enregistrerait dans `hv` par `lt`, compterait `nf` au crochet (`nf  38` bougerait : FAILED), et ferait `poke(0x5f2d,1)` plus tôt.
  - Rejeté : `je` = NULL avant `jl()`, bien que plus proche du cart (nil) — `pico8_ui_je_button_table` n'a pas de garde NULL (crash dans `pico8_instance_get_field`), `test_placeholder_je` figé en 1a.6 échouerait, et la ligne de l'étape dit « provisoire, remplacée », pas « supprimée ».
- **(G) Lecteur qui rend 0 pour tout nom, sans assert.** C'est exactement le comportement d'une instance sans classe à table vide. Un assert sur les noms hors {`ra`,`l`,`r`} a été envisagé comme fil-piège ; rejeté : le `je` provisoire n'est jamais dans `hv`, aucun consommateur générique ne l'atteint, et les seuls lecteurs sont nommés ci-dessus — pas de champ à protéger, l'assert ne garderait rien. Le lecteur ne peut pas être NULL : `pico8_entity_get_field` l'asserte.
- **(G) Stockage statique plutôt que `malloc`.** L'objet n'a aucun état mutable (tag et descripteur fixés à la compilation, aucun écrivain) : un exemplaire unique est indiscernable d'un exemplaire par `pico8_ui_init`. Aucun code ne compare l'identité de `je` à travers un `pico8_ui_init` (mesuré : `pico8_ui_set_je` compare au `je` courant ; `test_pico8_nf` compare `first_je`, pris après `pico8__init`, donc un `nf`). Conséquence : plus rien à libérer. Rejeté : `malloc` + libération « si descripteur provisoire » — garde une branche et un `free` pour un objet sans état.
- **Pas de constructeur, pas de couverture.** « Classe à part entière » = type struct + descripteur `static const` ; la construction est l'initialiseur statique. Le crochet de couverture n'est jamais appelé : le `je` provisoire n'a jamais été compté (construit à la main, hors `pico8_class_instantiate`), n'a pas de `pico8_class_t` à garder (§5 « Héritage entre formes » sans objet), et un nom compté hors `KNOWN[]` ferait échouer l'outil (règles 1a.0) — l'ajouter changerait la sortie attendue, ce que §7 interdit. **Cette étape n'ajoute aucun nom de couverture.**
- **`pico8_ui_shutdown`** : après ce commit, `g_je` est soit le `je` provisoire statique, soit un `nf` struct, soit NULL — jamais une instance. La branche `INSTANCE` est supprimée (orphelin créé par ce changement) ; `pico8_type_tag_of`/`pico8_instance_free` restent utilisés ailleurs dans `ui.c` (`g_ok`) ou non, l'implémenteur retire seulement ce qui devient inutilisé. Contrat : « `je` n'est jamais libéré ».
- **Type du slot** : inchangé (`pico8_instance_t *`), décision de 1a.6 ; le contrat de `pico8_je()` dit déjà « à lire seulement par `pico8_entity_get_field` ». Le type disparaît avec la phase (1a.42 et après), pas ici.
- **Emplacement** : type et objet dans `ui.c` (privés), accesseur du descripteur dans `ui.h`. Nom : rattaché à `je` (le glossaire mappe `je` → `pico8_je`), par exemple « placeholder » comme le disent déjà les commentaires du module ; pas de nom minifié du cart à conserver (le cart n'a pas de valeur pour `je` avant `jl()`).
- **Commentaires** (CLAUDE.md) : un contrat C sur l'objet (statique, jamais libéré, aucun champ) ; corrections de `ui.h` et `bootstrap.h` listées dans la Solution §6. Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts ; `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation sur l'ancien code.** Nouveau test (voir Testing), valeurs figées **avant** toute modification de `ui.c`.
2. **Conversion.** Struct, descripteur, objet statique, `pico8_ui_init`, `pico8_ui_shutdown`, accesseur, contrats d'en-tête ; test de forme ajouté. La caractérisation passe **sans changer ses valeurs figées**.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rend `pico8_entity_get_field` sur `pico8_je()`, ce que rend `pico8_mv`, appartenance à `hv`/`cv`/`cg`, ce que `pico8_ui_lj` pose sur un `ok`, NULL ou non-NULL de `pico8_je()`. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_je`** (dans `make unit`, lié comme `test_pico8_nf`, sur le cart réel chargé ; lecture **uniquement** par l'API à double forme, pour être le même code avant et après).
  - **Caractérisation, figée sur l'ancien code** :
    1. Après `pico8_cart_load()` et **avant** `pico8__init()` : `pico8_je()` non NULL ; `pico8_entity_get_field` rend 0 pour `ra`, `l`, `r`, `me`, `t`, `ph` ; `pico8_mv(je,"ph",…)` et `pico8_mv(je,"kj",…)` rendent 0 ; `je` absent de `hv` et de chaque `cv`/`cg` (listes vides ou non contenantes, selon ce que rend l'API avant `pico8_entity_init`).
    2. Même point : `pico8_ui_lj` sur un `ok` neuf (`pico8_ui_new(NULL)`) ne pose ni `lheld` ni `rheld`, et pose `qa` = `(0,0)` + `mo.ra` tel que lu (valeur figée).
    3. Cycle : `pico8_ui_shutdown()` → `pico8_je()` NULL ; `pico8_ui_init()` → de nouveau lisible comme en 1 ; deux `pico8_ui_init()` de suite, puis `pico8_cart_shutdown()` → `pico8_cart_load()` : lisible comme en 1, sans crash.
    4. Remplacement : `pico8__init()` (donc `jl()`) → `pico8_je()` rend `ra` présent (le `v(0,0)` du `nf`) et `l`/`r` présents ; `pico8_ui_init()` ensuite → de nouveau le provisoire (lectures de 1).
    5. `pico8_bootstrap_shutdown()` après `pico8_jl()` sans `pico8_cart_load()` (chemin de `planet_kj_dispatch`), puis `pico8_jl()` de nouveau : pas de crash, `je` lisible comme un `nf`.
  - **Forme** (ajouté au morceau 2) : après `pico8_cart_load()`, `pico8_je()` est tagué `STRUCT` avec le descripteur du `je` provisoire (accesseur) ; après `pico8_jl()`, avec celui de `nf` ; après `pico8_ui_init()`, de nouveau le provisoire, à la même adresse (stockage statique).
  - **Mutations** : l'implémenteur remet localement au moins 5 erreurs plausibles et note dans le commit celles qui sont attrapées : lecteur qui rend un vecteur non nul pour `ra` ; lecteur qui rend une table pour `l` ; `PH` (ou `KJ`) posé dans la table ; `pico8_ui_init` qui ne range pas le provisoire (`g_je` NULL : crash de `je[bt]` ou `pico8_je()` NULL) ; `pico8_ui_shutdown` qui passe l'objet statique à `pico8_instance_free`/`free` (abandon glibc) ; `pico8_ui_shutdown` qui ne remet pas `g_je` à NULL ; provisoire enregistré par `lt` dans `hv`. **Mutants équivalents à justifier dans le commit** : `malloc` par `pico8_ui_init` au lieu du statique sans libération (fuite seulement, rien ne compare l'identité à travers `pico8_ui_init` — sauf l'assertion de même adresse du test de forme, qui est une assertion de forme ajoutée après conversion, pas de caractérisation) ; lecteur avec assert sur les noms hors {`ra`,`l`,`r`} (aucun lecteur n'en demande d'autre).
- **`test_pico8_nf`** : inchangé ; `test_placeholder_je` passe avec un diff vide (il atteint `pico8_ui_init` après `pico8_jl()`).
- **Tests de régression** : aucun n'est nommé par l'étape ; tous passent avec un diff vide.
- **Couverture** : `make class-coverage-net` code 0, sortie identique à la référence.
- **Prior art** : `test_pico8_nf` (surtout `test_placeholder_je`), `test_pico8_fq`, `test_pico8_mh` (caractérisation figée puis forme ajoutée) ; `jb` (`layout.h`) pour une racine sans parent.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié, ou si le `je` provisoire appelle le crochet de couverture.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 — notamment si un lecteur de `je` avant `jl()` demande un nom hors {`ra`,`l`,`r`} et en attend une valeur, ou si une écriture sur le `je` provisoire est trouvée : arrêt, pas de membre improvisé.
- `FAILED:` si la struct du `je` provisoire contient autre chose que l'en-tête (en particulier un `pico8_ke_t`, `ra`, `l` ou `r`), ou si son descripteur a une méthode.
- `FAILED:` si un test de régression de bug (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe est convertie ou change de forme — en particulier le `mo` provisoire (1a.9), `qb` (1a.8), `ui`/`ok` (1a.31), `nf`, `ke`, `jb`, `iv`, `mh`, `fq` — ou si un `->defaults` bouge.
- `FAILED:` si les valeurs figées de la caractérisation du `je` provisoire, de `test_pico8_nf` ou de `test_pico8_iv` ont été changées après la conversion.

Critères de réussite (en plus) :

- grep : plus aucun `malloc`/`pico8_instance_free` de `g_je` dans `src/pico8_ui.c`, plus aucun `PICO8_TYPE_TAG_INSTANCE` écrit dans `pico8_ui_init` ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN` ;
- dans `ui.c`, le diff se limite au type/objet/descripteur du provisoire, à `pico8_ui_init` (rangement dans `g_je`), à `pico8_ui_shutdown` et à l'accesseur ; hors `ui.c`/`ui.h`, seul `bootstrap.h` (commentaire) change dans `src/` ;
- les contrats d'en-tête de `pico8_je()`, `pico8_ui_shutdown()` et `pico8_bootstrap_shutdown()` ne mentionnent plus de `je` libéré ni d'instance sans classe.

## Out of Scope

- Le `mo` provisoire (1a.9) et `qb` (1a.8), bien que construits de la même façon et lus par les mêmes helpers.
- `ui`/`ok` (1a.31) et toute écriture de `ui.c` sur eux ; `lheld`/`rheld`, `ldown`/`rdown`.
- Le type du slot `g_je` et de `pico8_je()` (`pico8_instance_t *`) : disparaît avec la sortie de phase.
- Ajouter une garde NULL à `pico8_ui_je_button_table`, ou supprimer le `je` provisoire au profit de nil : changement de comportement hors de la ligne de l'étape.
- Libérer les `nf` remplacés par `jl()` (TROU-7, Plan 2 §8).
- Lancer l'appli raylib : jamais.

## Further Notes

- Le filet ne voit jamais le `je` provisoire (remplacé par `jl()` dans `pico8__init`, avant la première frame de tout rejeu, avec ou sans `--from-title`). Toute la garde de l'étape est donc la caractérisation et `test_placeholder_je` ; la couverture est inchangée par construction.
- Mesures faites par grep sur `src/` et `tests/` à 824725c, en lisant `pico8/lua.lua:153-154` (`jl`) et `:1178` (`_init`), `pico8_ui.c` (`pico8_ui_init`, `pico8_ui_shutdown`, lecteurs), `pico8_entity.c` (`pico8_entity_get_field`), `pico8_class.c` (`pico8_instance_get_field`, pas de garde NULL), `pico8_bootstrap.c`, `pico8_cart.c`, les trois `main*.c`.
